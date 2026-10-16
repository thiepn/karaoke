// ROOMTONE A3: minimal Windows-native experimental microphone monitor.
// This is a standalone WASAPI full-duplex executable, NOT a replacement or
// wrapped version of the web application. No accounts, file access or uploads.
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "karaoke_mix.hpp"
#include "transport.hpp"
#include "handoff.hpp"
#include "desktop_ui.hpp"
#include "device_recovery.hpp"
#include <algorithm>
#include <cstdint>
#include <sstream>
#include <utility>
#include <vector>
#include <limits>
#include <filesystem>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#endif
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

struct Options {
    int input = -1;   // -1 uses Windows default
    int output = -1;
    unsigned period = 240;  // requested five milliseconds at 48 kHz
    float wet = 0.12f;
    float gain = 0.18f;
    float echo=0.12f,feedback=0.15f,musicGain=0.2f;
    unsigned echoMs=280;
    std::string song,handoffFile;
    bool list = false,probeSong=false,pickSong=false,guided=false,probeHandoff=false,pickHandoff=false,gui=false;
};
static int readInteger(const std::string& s) {
    std::size_t count=0;
    const int n=std::stoi(s,&count);
    if(count!=s.size())throw std::invalid_argument("invalid integer: "+s);
    return n;
}
static float readFloat(const std::string& s) {
    std::size_t count=0;
    const float x=std::stof(s,&count);
    if(count!=s.size()||!std::isfinite(x))throw std::invalid_argument("invalid floating point number: "+s);
    return x;
}
static Options parse(int argc,char** argv) {
    Options opt;
    for(int i=1;i<argc;++i) {
        const std::string key=argv[i];
        if(key=="--list"){opt.list=true;continue;}
        if(key=="--gui"){opt.gui=true;continue;}
        if(key=="--console"){opt.gui=false;continue;}
        if(key=="--probe-song"){opt.probeSong=true;continue;}
        if(key=="--pick-song"){opt.pickSong=true;continue;}
        if(key=="--setup"){opt.guided=true;continue;}
        if(key=="--probe-handoff"){opt.probeHandoff=true;continue;}
        if(key=="--pick-handoff"){opt.pickHandoff=true;continue;}
        if(key=="--help"||key=="-h"){
            std::cout<<"ROOMTONE A3 Windows WASAPI proof-of-concept\n"
                     <<"  --list  list device indices (0-based)\n"
                     <<"  --input N --output N  select indices from --list\n"
                     <<"  --period 120|240|480  requested buffer frames at 48000 Hz\n"
                     <<"  --wet 0..0.25  short parallel reverb, default 0.12\n"
                     <<"  --gain 0..0.35  output level, default 0.18\n"
                     <<"  --song PATH  local MP3/WAV/FLAC instrumental (12-minute limit)\n"
                     <<"  --probe-song  decode local track without opening devices\n"
                     <<"  --pick-song  Windows file chooser (Unicode paths)\n"
                     <<"  --handoff PATH  import offline browser lyrics/position (.rtk)\n"
                     <<"  --pick-handoff  choose downloaded .rtk in Windows file picker\n"
                     <<"  --probe-handoff  validate .rtk WITHOUT opening devices\n"
                     <<"  --setup  guided mic/speaker device selection\n"
                     <<"  --gui  in-process Win32 karaoke view\n"
                     <<"  --console  original command-line controls\n"
                     <<"  --echo 0..0.35 --feedback 0..0.35 --echo-ms 80..550\n"
                     <<"  --music 0..0.50  backing track volume\n"
                     <<"Starts MIC MUTED and SONG PAUSED. Commands m/p/r, status,\n"
                     <<"seek SECONDS (+/- relative), loop on/off, reconnect,\n"
                     <<"wet/gain/echo/feedback/echo-ms/music NUMBER; q to quit.\n";
            std::exit(0);
        }
        if(i+1>=argc)throw std::invalid_argument("missing value for "+key);
        const std::string value=argv[++i];
        if(key=="--input") opt.input=readInteger(value);
        else if(key=="--output") opt.output=readInteger(value);
        else if(key=="--period") opt.period=static_cast<unsigned>(readInteger(value));
        else if(key=="--wet") opt.wet=readFloat(value);
        else if(key=="--gain") opt.gain=readFloat(value);
        else if(key=="--song") opt.song=value;
        else if(key=="--handoff") opt.handoffFile=value;
        else if(key=="--echo") opt.echo=readFloat(value);
        else if(key=="--feedback") opt.feedback=readFloat(value);
        else if(key=="--echo-ms") {const int n=readInteger(value);if(n<0)throw std::invalid_argument("negative echo-ms");opt.echoMs=static_cast<unsigned>(n);}
        else if(key=="--music") opt.musicGain=readFloat(value);
        else throw std::invalid_argument("unknown argument: "+key);
    }
    if(opt.input < -1 || opt.output < -1)throw std::invalid_argument("device index must be -1 or greater");
    if(opt.period!=120 && opt.period!=240 && opt.period!=480)
        throw std::invalid_argument("period must be 120, 240 or 480 frames");
    if(opt.wet<0||opt.wet>0.25f||opt.gain<0||opt.gain>0.35f||
       opt.echo<0||opt.echo>0.35f||opt.feedback<0||opt.feedback>0.35f||
       opt.echoMs<80||opt.echoMs>550||opt.musicGain<0||opt.musicGain>0.50f)
        throw std::invalid_argument("audio levels outside safe test range");
    if(opt.probeHandoff&&(opt.handoffFile.empty()||opt.pickHandoff||opt.pickSong||opt.guided))throw std::invalid_argument("--probe-handoff requires --handoff PATH without interactive flags");
    if(opt.probeSong&&(opt.song.empty()||opt.pickSong||opt.guided))
        throw std::invalid_argument("--probe-song needs --song PATH without --setup/--pick-song");
    return opt;
}

// Decode local backing tracks entirely before audio device startup.
// All potentially blocking file access and allocations are outside callback.
static std::vector<float> loadSong(const std::string& path,const std::wstring& widePath={}) {
    if(path.empty()&&widePath.empty())return {};
    ma_decoder_config config=ma_decoder_config_init(ma_format_f32,2,roomtone::VocalDsp::kSampleRate);
    ma_decoder decoder{};
    const ma_result decoderStatus=widePath.empty()
        ? ma_decoder_init_file(path.c_str(),&config,&decoder)
        : ma_decoder_init_file_w(widePath.c_str(),&config,&decoder);
    if(decoderStatus!=MA_SUCCESS)
        throw std::runtime_error("Could not decode selected local instrumental file");
    constexpr ma_uint64 maxFrames=static_cast<ma_uint64>(roomtone::VocalDsp::kSampleRate)*60*12;
    ma_uint64 frames=0;
    const ma_result sizeStatus=ma_decoder_get_length_in_pcm_frames(&decoder,&frames);
    if(sizeStatus!=MA_SUCCESS||frames==0||frames>maxFrames){
        ma_decoder_uninit(&decoder);
        throw std::runtime_error("Song is empty, duration unknown or longer than 12 minutes");
    }
    std::vector<float> samples(static_cast<std::size_t>(frames)*2);
    ma_uint64 read=0;
    const ma_result result=ma_decoder_read_pcm_frames(&decoder,samples.data(),frames,&read);
    ma_decoder_uninit(&decoder);
    if((result!=MA_SUCCESS&&result!=MA_AT_END)||read==0)
        throw std::runtime_error("Could not read backing track frames");
    samples.resize(static_cast<std::size_t>(read)*2);
    for(float& v:samples)if(!std::isfinite(v))v=0;
    return samples;
}
#ifdef _WIN32
// Optional native file dialog. Does not open a server or expose local file paths
// to the browser; uses Unicode Windows filesystem APIs.
static std::wstring chooseSongFile() {
    wchar_t file[32768]={};
    OPENFILENAMEW dialog{};
    dialog.lStructSize=sizeof(dialog);
    dialog.lpstrFile=file;
    dialog.nMaxFile=static_cast<DWORD>(sizeof(file)/sizeof(file[0]));
    dialog.lpstrFilter=L"Audio files (*.wav;*.mp3;*.flac)\0*.wav;*.mp3;*.flac\0All files (*.*)\0*.*\0";
    dialog.Flags=OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
    if(GetOpenFileNameW(&dialog))return file;
    const DWORD error=CommDlgExtendedError();
    if(error!=0)throw std::runtime_error("Windows file chooser failed (code "+std::to_string(error)+")");
    return {}; // Cancelled by user.
}
static std::wstring chooseHandoffFile(){
 wchar_t filename[32768]={};
 OPENFILENAMEW d{};
 d.lStructSize=sizeof(d);d.lpstrFile=filename;
 d.nMaxFile=static_cast<DWORD>(sizeof(filename)/sizeof(filename[0]));
 d.lpstrFilter=L"ROOMTONE sessions (*.rtk)\0*.rtk\0All files\0*.*\0";
 d.Flags=OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
 if(GetOpenFileNameW(&d))return filename;
 const DWORD err=CommDlgExtendedError();
 if(err)throw std::runtime_error("Could not open native session picker (error "+std::to_string(err)+")");
 return {}; // User cancelled, no session imported.
}
static int chooseDeviceIndex(const char* label,ma_device_info* list,ma_uint32 count){
    for(;;){
        std::cout<<label<<" device (-1 or Enter = Windows default):\n";
        for(ma_uint32 i=0;i<count;++i)std::cout<<"  "<<i<<" : "<<list[i].name<<(list[i].isDefault?" [default]":"")<<"\n";
        std::cout<<"Selection: "<<std::flush;
        std::string line;
        if(!std::getline(std::cin,line))throw std::runtime_error("Guided setup cancelled");
        if(line.empty()||line=="-1")return -1;
        try{
            const int index=readInteger(line);
            if(index>=0&&index<static_cast<int>(count))return index;
        }catch(const std::exception&){}
        std::cout<<"Invalid device choice. Select a listed index.\n";
    }
}
#endif

struct Runtime {
    Runtime(const Options& o,std::vector<float> song)
        : mixer(std::move(song),o.wet,o.gain),wet(o.wet),gain(o.gain),
          echo(o.echo),feedback(o.feedback),echoMs(o.echoMs),musicGain(o.musicGain){}
    roomtone::KaraokeMix mixer;
    std::atomic<bool> monitoring{false},musicPlaying{false},rewindRequest{false},repeat{false},interrupted{false};
    std::atomic<std::uint64_t> seekRequest{std::numeric_limits<std::uint64_t>::max()};
    std::atomic<float> wet,gain,echo,feedback,musicGain;
    std::atomic<unsigned> echoMs;
    std::atomic<std::uint64_t> songPosition{0};
    roomtone::Handoff session;
    bool abLoop=false;
    std::size_t abStart=0,abEnd=0;
};
// On driver stop/reroute, fail closed. Never restart/uninit a device from
// a driver callback; the user can explicitly request reconnect on CLI thread.
static void onNotification(const ma_device_notification* event) {
    if(!event||!event->pDevice)return;
    auto* rt=static_cast<Runtime*>(event->pDevice->pUserData);
    if(!rt)return;
    if(event->type==ma_device_notification_type_stopped||
       event->type==ma_device_notification_type_rerouted) {
        rt->monitoring.store(false,std::memory_order_relaxed);
        rt->musicPlaying.store(false,std::memory_order_relaxed);
        rt->interrupted.store(true,std::memory_order_relaxed);
    }
}
static void callback(ma_device* device, void* output, const void* input, ma_uint32 frames) {
    auto* rt=static_cast<Runtime*>(device->pUserData);
    if(!output||!rt)return;
    auto* out=static_cast<float*>(output);
    const auto* in=static_cast<const float*>(input);
    // A rerouted/stopped driver must emit silence even if it delivers a final
    // callback before the UI owner drains/uninitializes the old device.
    if(rt->interrupted.load(std::memory_order_relaxed)){
        std::fill_n(out,static_cast<std::size_t>(frames)*2,0.0f);
        return;
    }
    if(rt->rewindRequest.exchange(false,std::memory_order_relaxed))rt->mixer.rewind();
    constexpr auto kNoSeek=std::numeric_limits<std::uint64_t>::max();
    const auto seek=rt->seekRequest.exchange(kNoSeek,std::memory_order_relaxed);
    if(seek!=kNoSeek)rt->mixer.seekFrames(static_cast<std::size_t>(seek));
    rt->mixer.vocal.setParameters(rt->wet.load(std::memory_order_relaxed),
        rt->gain.load(std::memory_order_relaxed),rt->echo.load(std::memory_order_relaxed),
        rt->feedback.load(std::memory_order_relaxed),rt->echoMs.load(std::memory_order_relaxed));
    const bool monitor=in&&rt->monitoring.load(std::memory_order_relaxed);
    const bool play=rt->musicPlaying.load(std::memory_order_relaxed);
    const float level=rt->musicGain.load(std::memory_order_relaxed);
    for(ma_uint32 i=0;i<frames;++i) {
        if(play&&rt->repeat.load(std::memory_order_relaxed)){
            if(rt->abLoop&&rt->mixer.position()>=rt->abEnd)rt->mixer.seekFrames(rt->abStart);
            else if(rt->mixer.songEnded())rt->mixer.rewind();
        }
        const auto result=rt->mixer.render(in?in[i]:0.0f,monitor,play,level);
        out[i*2]=result.left;out[i*2+1]=result.right;
    }
    rt->songPosition.store(rt->mixer.position(),std::memory_order_relaxed);
    if(rt->mixer.songEnded()&&!rt->repeat.load(std::memory_order_relaxed)&&!rt->rewindRequest.load(std::memory_order_relaxed))
        rt->musicPlaying.store(false,std::memory_order_relaxed);
}
static bool changeLevel(Runtime& rt,const std::string& key,const std::string& value) {
    if(key=="echo-ms"){
        const int ms=readInteger(value);
        if(ms<80||ms>550)throw std::invalid_argument("echo-ms requires 80..550");
        rt.echoMs.store(static_cast<unsigned>(ms));return true;
    }
    const float v=readFloat(value);
    std::atomic<float>* target=nullptr;float maximum=0;
    if(key=="wet"){target=&rt.wet;maximum=0.25f;}
    else if(key=="gain"){target=&rt.gain;maximum=0.35f;}
    else if(key=="echo"){target=&rt.echo;maximum=0.35f;}
    else if(key=="feedback"){target=&rt.feedback;maximum=0.35f;}
    else if(key=="music"){target=&rt.musicGain;maximum=0.50f;}
    if(!target)return false;
    if(v<0||v>maximum)throw std::invalid_argument("level outside permitted range");
    target->store(v);return true;
}

int main(int argc,char** argv) {
#ifdef _WIN32
    try {
        Options opt=parse(argc,argv);
        if(argc==1){opt.guided=true;opt.gui=true;} // Double-click uses Win32 UI.
        if(opt.probeHandoff){
            const auto session=roomtone::loadHandoff(opt.handoffFile);
            std::cout<<"OFFLINE_SESSION_OK audio="<<session.audioName<<" lyrics="<<session.lines.size()
                     <<" start-ms="<<session.startMs<<"; NO audio devices opened.\n";
            return 0;
        }
        std::wstring chosenSong;
        if(!opt.list&&!opt.probeSong&&(opt.pickSong||(opt.guided&&opt.song.empty()))) {
            chosenSong=chooseSongFile();
            if(chosenSong.empty()){
                std::cout<<"Song picker cancelled. No hardware was activated.\n";
                return 0;
            }
        }
        roomtone::Handoff imported;
        bool hasHandoff=false;
        if(!opt.handoffFile.empty()&&!opt.list&&!opt.probeSong){
            imported=roomtone::loadHandoff(opt.handoffFile);hasHandoff=true;
        }
        if(!opt.list&&!opt.probeSong&&(opt.pickHandoff||(opt.guided&&!hasHandoff))){
            bool choose=opt.pickHandoff;
            if(!choose&&opt.guided){
                if(opt.gui){
                    choose=MessageBoxW(nullptr,
                         L"Import a downloaded ROOMTONE .rtk session with lyrics and timing?",
                         L"ROOMTONE local handoff",MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2)==IDYES;
                }else{
                    std::cout<<"Import downloaded browser .rtk session? y/N: "<<std::flush;
                    std::string answer;
                    if(!std::getline(std::cin,answer))return 0;
                    choose=answer=="y"||answer=="Y";
                }
            }
            if(choose){
                const auto selected=chooseHandoffFile();
                if(!selected.empty()){
                    imported=roomtone::loadHandoff(std::filesystem::path(selected));
                    hasHandoff=true;
                }
            }
        }
        if(hasHandoff){
            if(opt.song.empty()&&chosenSong.empty())
                throw std::invalid_argument("ROOMTONE session requires original local audio file");
            const std::string filename=chosenSong.empty()
              ?std::filesystem::u8path(opt.song).filename().u8string()
              :std::filesystem::path(chosenSong).filename().u8string();
            if(filename!=imported.audioName)
                throw std::invalid_argument("Selected local instrumental filename does not match original ROOMTONE session");
        }
        // Decode before WASAPI; .rtk never contains audio bytes or disk path.
        auto song=opt.list?std::vector<float>{}:loadSong(opt.song,chosenSong);
        if(opt.probeSong){
            std::cout<<"DECODED_LOCAL_FRAMES "<<song.size()/2<<" AT_48000_STEREO; no audio device opened.\n";
            return 0;
        }
        const ma_backend backend[]={ma_backend_wasapi};
        ma_context context{};
        if(ma_context_init(backend,1,nullptr,&context)!=MA_SUCCESS) {
            std::cerr<<"Could not initialize Windows WASAPI backend.\n";return 2;
        }
        ma_device_info* outputs=nullptr;
        ma_device_info* inputs=nullptr;
        ma_uint32 outCount=0,inCount=0;
        if(ma_context_get_devices(&context,&outputs,&outCount,&inputs,&inCount)!=MA_SUCCESS){
            std::cerr<<"Audio device enumeration failed.\n";
            ma_context_uninit(&context);return 3;
        }
        auto printDevices=[](const char* kind,ma_device_info* infos,ma_uint32 count) {
            std::cout<<kind<<" devices (0-based; default is the Windows-selected device):\n";
            for(ma_uint32 i=0;i<count;++i)std::cout<<"  "<<i<<"  "<<infos[i].name<<(infos[i].isDefault?" [default]":"")<<"\n";
        };
        if(opt.list) {
            printDevices("Capture",inputs,inCount);
            printDevices("Playback",outputs,outCount);
            ma_context_uninit(&context);return 0;
        }
        // Snapshot the chooser labels before any modal user interaction.
        // Enumerated miniaudio device pointers may become stale after USB changes.
        std::vector<std::string> initialInputs,initialOutputs;
        for(ma_uint32 i=0;i<inCount;++i)initialInputs.emplace_back(inputs[i].name);
        for(ma_uint32 i=0;i<outCount;++i)initialOutputs.emplace_back(outputs[i].name);
        if(opt.guided){
            try{
                if(opt.gui){
                    std::vector<std::wstring> microphones,speakers;
                    for(const auto& name:initialInputs)microphones.push_back(roomtone::desktop::wide(name));
                    for(const auto& name:initialOutputs)speakers.push_back(roomtone::desktop::wide(name));
                    const auto choice=roomtone::desktop::selectDevices(microphones,speakers);
                    if(!choice){ma_context_uninit(&context);return 0;}
                    opt.input=choice->first;opt.output=choice->second;
                }else{
                    opt.input=chooseDeviceIndex("Microphone",inputs,inCount);
                    opt.output=chooseDeviceIndex("Speaker",outputs,outCount);
                }
            }catch(...){ma_context_uninit(&context);throw;}
        }
        std::string inputName,outputName;
        // This WASAPI endpoint token is private, in-memory and never serialized.
        // It identifies the selected OS endpoint, not the physical USB unit.
        std::string pinnedInputId,pinnedOutputId;
        const auto endpointId=[](const ma_device_info& info)->std::string{
            const auto& bytes=info.id.wasapi;
            std::size_t length=0;
            while(length<sizeof(bytes)/sizeof(bytes[0])&&bytes[length]!=0)++length;
            return std::string(reinterpret_cast<const char*>(bytes),length*sizeof(bytes[0]));
        };
        if(!roomtone::device_policy::selectedName(opt.input,initialInputs,inputName)||
           !roomtone::device_policy::selectedName(opt.output,initialOutputs,outputName)){
            std::cerr<<"Device selection is missing, stale or ambiguous. Select a unique named device or Windows default.\n";
            ma_context_uninit(&context);return 4;
        }
        if(!inputName.empty())std::cout<<"Input: "<<inputName<<"\n";
        if(!outputName.empty())std::cout<<"Output: "<<outputName<<"\n";
        // A unique name selects the first OS endpoint. Once successfully started, its
        // WASAPI endpoint ID is pinned until the owner deliberately re-selects devices.
        // System defaults remain explicitly OS-owned. Same-name new endpoints fail closed.
        std::string selectedSongFile=chosenSong.empty()
             ?std::filesystem::u8path(opt.song).filename().u8string()
             :std::filesystem::path(chosenSong).filename().u8string();
        std::wstring displayTitle=hasHandoff?roomtone::desktop::wide(imported.title)
             :roomtone::desktop::wide(selectedSongFile);
        Runtime rt(opt,std::move(song));
        if(hasHandoff){
            rt.session=std::move(imported);
            rt.seekRequest.store(std::min<std::uint64_t>(rt.session.startMs*48ULL,rt.mixer.songFrames()));
            rt.repeat.store(rt.session.loop);
            if(rt.session.loop){
                rt.abStart=std::min<std::size_t>(rt.session.loopStartMs*48ULL,rt.mixer.songFrames());
                rt.abEnd=std::min<std::size_t>(rt.session.loopEndMs*48ULL,rt.mixer.songFrames());
                rt.abLoop=rt.abEnd>rt.abStart+12000;
            }
        }
        ma_device device{};
        bool deviceActive=false;
        bool deviceTopologyDirty=false;
        std::wstring lastDeviceIssue;
        const auto startDevice=[&]()->bool{
            lastDeviceIssue.clear();
            // Device output always starts in an explicitly muted state.
            rt.monitoring.store(false);rt.musicPlaying.store(false);
            ma_device_info *newOutputs=nullptr,*newInputs=nullptr;
            ma_uint32 newOutCount=0,newInCount=0;
            if(ma_context_get_devices(&context,&newOutputs,&newOutCount,&newInputs,&newInCount)!=MA_SUCCESS){
                lastDeviceIssue=L"Windows WASAPI enumeration failed. Reconnect USB device or reopen this app.";
                std::cout<<"Cannot enumerate WASAPI devices; reconnect again later.\n";return false;
            }
            std::vector<std::string> captures,playbacks;
            for(ma_uint32 i=0;i<newInCount;++i)captures.emplace_back(newInputs[i].name);
            for(ma_uint32 i=0;i<newOutCount;++i)playbacks.emplace_back(newOutputs[i].name);
            const auto captured=roomtone::device_policy::resolve(inputName,captures);
            const auto rendered=roomtone::device_policy::resolve(outputName,playbacks);
            if((!inputName.empty()&&captured.state!=roomtone::device_policy::State::Unique)||
               (!outputName.empty()&&rendered.state!=roomtone::device_policy::State::Unique)){
                const auto failure=[](roomtone::device_policy::State state,const wchar_t* role)->std::wstring{
                    if(state==roomtone::device_policy::State::Missing)
                        return std::wstring(role)+L" was disconnected or renamed.";
                    if(state==roomtone::device_policy::State::Ambiguous)
                        return std::wstring(role)+L" name is duplicated; choose a unique device.";
                    return {};
                };
                lastDeviceIssue=failure(captured.state,L"Microphone ")+L" "+failure(rendered.state,L"Speaker ");
                if(lastDeviceIssue.empty())lastDeviceIssue=L"Selected device missing or ambiguous.";
                std::cout<<"Selected device is missing or ambiguous. Audio remains muted. Reconnect it or select devices again.\n";
                return false;
            }
            const int capture=captured.index,playback=rendered.index;
            const auto currentInput=capture>=0?endpointId(newInputs[capture]):std::string{};
            const auto currentOutput=playback>=0?endpointId(newOutputs[playback]):std::string{};
            if((capture>=0&&!roomtone::device_policy::acceptsEndpointId(pinnedInputId,currentInput))||
               (playback>=0&&!roomtone::device_policy::acceptsEndpointId(pinnedOutputId,currentOutput))){
                lastDeviceIssue=L"WASAPI endpoint identity changed or unavailable. Select devices again to authorize this endpoint.";
                std::cout<<"Endpoint identity mismatch; cannot silently use a same-name replacement.\n";
                return false;
            }
            ma_device_config config=ma_device_config_init(ma_device_type_duplex);
            config.capture.format=ma_format_f32;config.capture.channels=1;
            config.playback.format=ma_format_f32;config.playback.channels=2;
            config.sampleRate=roomtone::VocalDsp::kSampleRate;
            config.periodSizeInFrames=opt.period;
            config.performanceProfile=ma_performance_profile_low_latency;
            if(capture>=0)config.capture.pDeviceID=&newInputs[capture].id;
            if(playback>=0)config.playback.pDeviceID=&newOutputs[playback].id;
            config.pUserData=&rt;
            config.dataCallback=callback;
            config.notificationCallback=onNotification;
            if(ma_device_init(&context,&config,&device)!=MA_SUCCESS){
                lastDeviceIssue=L"WASAPI could not initialize capture/playback. Check USB and 48 kHz device setup.";
                std::cout<<"WASAPI init failed; try reconnect or a larger --period value.\n";
                return false;
            }
            // Clear an OLD interruption before starting, not afterward.
            // Otherwise a fresh stopped/rerouted notification during startup
            // could be accidentally masked and re-enable unsafe monitoring.
            rt.interrupted.store(false);
            if(ma_device_start(&device)!=MA_SUCCESS){
                rt.interrupted.store(true);
                ma_device_uninit(&device);
                lastDeviceIssue=L"WASAPI failed to start. Device may be busy or unavailable.";
                std::cout<<"WASAPI start failed; driver may be unavailable.\n";return false;
            }
            deviceActive=true;
            if(rt.interrupted.load()){
                rt.monitoring.store(false);rt.musicPlaying.store(false);
                ma_device_uninit(&device);deviceActive=false;
                lastDeviceIssue=L"Audio was interrupted during device startup. Reconnect only after checking Windows sound.";
                return false;
            }
            // The successful user-selected WASAPI endpoint is now pinned.
            // A later hotplug cannot reuse its display name with a new ID.
            if(capture>=0&&pinnedInputId.empty())pinnedInputId=currentInput;
            if(playback>=0&&pinnedOutputId.empty())pinnedOutputId=currentOutput;
            return true;
        };
        if(!startDevice()&&!opt.gui){
            ma_context_uninit(&context);return 5;
        }
        if(opt.gui){
            using roomtone::desktop::Action;
            using roomtone::desktop::Snapshot;
            std::wstring notice=lastDeviceIssue.empty()
                ?L"Start with wired headphones at low volume. F9 (focused window) toggles mic."
                :lastDeviceIssue+L" Voice and music muted.";
            auto safeStop=[&](){
                rt.monitoring.store(false);rt.musicPlaying.store(false);
                if(deviceActive){ma_device_uninit(&device);deviceActive=false;}
                // Callback has stopped. Freeze the latest owner-requested transport
                // (including an unconsumed seek/restart), not stale UI position.
                roomtone::recovery::freeze(
                    rt.seekRequest,rt.rewindRequest,rt.songPosition,rt.mixer.songFrames());
            };
            auto safeRestart=[&](){
                rt.mixer.vocal.reset();
                if(startDevice())notice=L"Audio ready. Voice MUTED, music PAUSED. Press Play when ready.";
                else notice=lastDeviceIssue.empty()
                    ?L"Audio unavailable. Voice and music remain muted."
                    :lastDeviceIssue+L" Voice and music remain muted.";
            };
            roomtone::desktop::Window window;
            window.controller.deviceChanged=[&]{deviceTopologyDirty=true;};
            window.controller.maintenance=[&]{
                // WASAPI notifications and WM_DEVICECHANGE only signal.
                // Joining/uninitializing stays on this owner/control thread.
                if(deviceActive&&rt.interrupted.load()){
                    safeStop();rt.mixer.vocal.reset();
                    notice=L"WASAPI interrupted. Position saved; reconnect explicitly (both channels muted).";
                }
                if(!deviceTopologyDirty)return;
                deviceTopologyDirty=false;
                ma_device_info *outs=nullptr,*ins=nullptr;
                ma_uint32 no=0,ni=0;
                if(ma_context_get_devices(&context,&outs,&no,&ins,&ni)!=MA_SUCCESS){
                    rt.monitoring.store(false);rt.musicPlaying.store(false);
                    if(deviceActive){rt.interrupted.store(true);safeStop();}
                    notice=L"Windows device list unavailable. Reconnect explicitly; both channels muted.";
                    return;
                }
                std::vector<std::string> captureNames,playbackNames;
                for(ma_uint32 i=0;i<ni;++i)captureNames.emplace_back(ins[i].name);
                for(ma_uint32 i=0;i<no;++i)playbackNames.emplace_back(outs[i].name);
                const auto capture=roomtone::device_policy::resolve(inputName,captureNames);
                const auto playback=roomtone::device_policy::resolve(outputName,playbackNames);
                const bool missing=(capture.state==roomtone::device_policy::State::Missing||
                    capture.state==roomtone::device_policy::State::Ambiguous||
                    playback.state==roomtone::device_policy::State::Missing||
                    playback.state==roomtone::device_policy::State::Ambiguous||
                    (capture.index>=0&&!pinnedInputId.empty()&&
                     !roomtone::device_policy::acceptsEndpointId(pinnedInputId,endpointId(ins[capture.index])))||
                    (playback.index>=0&&!pinnedOutputId.empty()&&
                     !roomtone::device_policy::acceptsEndpointId(pinnedOutputId,endpointId(outs[playback.index]))));
                if(missing){
                    rt.monitoring.store(false);rt.musicPlaying.store(false);
                    if(deviceActive){rt.interrupted.store(true);safeStop();}
                    notice=L"Named microphone/speaker missing or ambiguous after device change. No silent fallback. Select devices or reconnect.";
                }else if(!deviceActive){
                    notice=L"Devices visible again. Press Reconnect to reopen voice-MUTED and song-PAUSED.";
                }
            };
            window.controller.snapshot=[&]()->Snapshot{
                Snapshot view;
                view.title=displayTitle.empty()?L"ROOMTONE / NATIVE KARAOKE":displayTitle;
                view.positionSeconds=static_cast<unsigned>(rt.songPosition.load()/48000);
                view.durationSeconds=static_cast<unsigned>(rt.mixer.songFrames()/48000);
                view.playing=rt.musicPlaying.load();view.microphone=rt.monitoring.load();
                view.repeat=rt.repeat.load();view.interrupted=rt.interrupted.load();
                view.deviceActive=deviceActive;view.hasSong=rt.mixer.hasSong();
                view.hasLyrics=hasHandoff&&!rt.session.lines.empty();
                view.reverb=rt.wet.load();view.echo=rt.echo.load();view.music=rt.musicGain.load();view.gain=rt.gain.load();
                view.input=inputName.empty()?L"Windows default":roomtone::desktop::wide(inputName);
                view.output=outputName.empty()?L"Windows default":roomtone::desktop::wide(outputName);
                view.notice=rt.interrupted.load()&&deviceActive
                    ?L"Audio interrupted. Reconnect manually (voice and song stay muted)."
                    :notice;
                if(hasHandoff&&!rt.session.lines.empty()){
                    const auto ms=static_cast<std::uint32_t>(
                        std::min<std::uint64_t>(rt.songPosition.load()/48,720000));
                    const auto at=roomtone::lyricIndexAt(rt.session,ms);
                    if(at==rt.session.lines.size()){
                        view.current=L"Get ready...";
                        view.next=roomtone::desktop::wide(rt.session.lines.front().text);
                    }else{
                        view.current=roomtone::desktop::wide(rt.session.lines[at].text);
                        if(at>0)view.previous=roomtone::desktop::wide(rt.session.lines[at-1].text);
                        if(at+1<rt.session.lines.size())
                            view.next=roomtone::desktop::wide(rt.session.lines[at+1].text);
                    }
                }else view.current=L"Import a matching .rtk for synchronized lyrics";
                return view;
            };
            window.controller.seekSeconds=[&](unsigned seconds){
                if(!rt.mixer.hasSong()){
                    notice=L"Cannot seek without a loaded local song.";return;
                }
                const auto at=std::min<std::uint64_t>(
                    static_cast<std::uint64_t>(seconds)*48000ULL,rt.mixer.songFrames());
                rt.rewindRequest.store(false);
                rt.seekRequest.store(at);
                if(!deviceActive||rt.interrupted.load())rt.songPosition.store(at);
                notice=(!deviceActive||rt.interrupted.load())
                    ?L"Offline seek saved; reconnect stays voice-MUTED and music-PAUSED."
                    :L"Song seek queued in the original WASAPI engine.";
            };
            window.controller.action=[&](Action action){
                try{
                    const bool ready=deviceActive&&!rt.interrupted.load();
                    const auto level=[&](std::atomic<float>& target,float change,float maximum){
                        const float current=target.load();
                        target.store(std::clamp(std::round((current+change)*100.0f)/100.0f,0.0f,maximum));
                    };
                    switch(action){
                    case Action::EmergencyMute:
                        rt.monitoring.store(false);
                        notice=L"VOICE MUTED immediately. Use physical mute/power for acoustic feedback.";
                        break;
                    case Action::Mic:
                        if(!ready){notice=L"Reconnect audio before enabling the microphone.";break;}
                        if(!rt.monitoring.load()){
                            if(MessageBoxW(window.hwnd,
                               L"Enable microphone monitoring?\nBegin with wired headphones and very low volume.\n"
                               L"Speakers can cause dangerous feedback. Use the physical QuadCast mute immediately.",
                               L"ROOMTONE - microphone safety",MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2)!=IDYES)
                               break;
                            rt.monitoring.store(true);
                            notice=L"MIC ON. Physically mute the QuadCast if feedback begins.";
                        }else{rt.monitoring.store(false);notice=L"Microphone muted.";}
                        break;
                    case Action::Play:
                        if(!ready||!rt.mixer.hasSong()){notice=L"Reconnect audio / load instrumental.";break;}
                        if(rt.songPosition.load()>=rt.mixer.songFrames())rt.rewindRequest.store(true);
                        rt.musicPlaying.store(!rt.musicPlaying.load());break;
                    case Action::Restart:
                        if(rt.mixer.hasSong()){
                            // While offline, an older frozen seek must not override restart
                            // on the first callback of the reconnected device.
                            if(ready){
                                rt.seekRequest.store(roomtone::recovery::NoSeek);
                                rt.rewindRequest.store(true);
                                rt.musicPlaying.store(true);
                            }else{
                                rt.rewindRequest.store(false);
                                rt.seekRequest.store(0);
                                rt.songPosition.store(0);
                                notice=L"Restart queued offline; reconnect stays muted and paused.";
                            }
                        }break;
                    case Action::Back:case Action::Forward:
                        if(rt.mixer.hasSong()){
                            const auto current=static_cast<std::size_t>(rt.songPosition.load());
                            const auto destination=roomtone::parseSeekFrames(
                                action==Action::Back?"-10":"+10",current,rt.mixer.songFrames());
                            rt.rewindRequest.store(false);rt.seekRequest.store(destination);
                            if(!ready)rt.songPosition.store(destination);
                        }break;
                    case Action::NextLine:case Action::PreviousLine:
                        if(rt.mixer.hasSong()&&hasHandoff){
                            const auto ms=static_cast<std::uint32_t>(
                                std::min<std::uint64_t>(rt.songPosition.load()/48,720000));
                            const auto limit=static_cast<std::uint32_t>(
                                std::min<std::uint64_t>(rt.mixer.songFrames()/48,720000));
                            const auto target=roomtone::lyricJumpMs(
                                rt.session,ms,action==Action::NextLine,limit);
                            if(target){
                                rt.rewindRequest.store(false);
                                rt.seekRequest.store(static_cast<std::uint64_t>(*target)*48);
                                if(!ready)rt.songPosition.store(static_cast<std::uint64_t>(*target)*48);
                            }
                        }break;
                    case Action::BrowseLyrics:{
                        if(!rt.mixer.hasSong()||!hasHandoff||rt.session.lines.empty()){
                            notice=L"Load matching .rtk lyrics before browsing.";break;
                        }
                        // Source-owned timestamps; never access audio callback or browser.
                        std::vector<std::wstring> rows;
                        rows.reserve(rt.session.lines.size());
                        const auto limit=static_cast<std::int64_t>(
                            std::min<std::uint64_t>(rt.mixer.songFrames()/48,720000));
                        for(const auto& lyric:rt.session.lines){
                            const auto at=std::clamp<std::int64_t>(
                                static_cast<std::int64_t>(lyric.milliseconds)+rt.session.offsetMs,0,limit);
                            std::wstring line=roomtone::desktop::clock(static_cast<unsigned>(at/1000));
                            line+=L"  ";
                            line+=roomtone::desktop::wide(lyric.text);
                            if(line.size()>300)line.resize(300);
                            rows.push_back(std::move(line));
                        }
                        const auto ms=static_cast<std::uint32_t>(
                            std::min<std::uint64_t>(rt.songPosition.load()/48,720000));
                        const auto current=roomtone::lyricIndexAt(rt.session,ms);
                        const auto chosen=roomtone::desktop::selectLyric(rows,
                            current==rt.session.lines.size()?0:current,window.hwnd);
                        if(!chosen){notice=L"Lyric navigation canceled; transport unchanged.";break;}
                        const auto target=std::clamp<std::int64_t>(
                            static_cast<std::int64_t>(rt.session.lines[*chosen].milliseconds)
                                +rt.session.offsetMs,0,limit);
                        rt.rewindRequest.store(false);
                        rt.seekRequest.store(static_cast<std::uint64_t>(target)*48);
                        if(!ready)rt.songPosition.store(static_cast<std::uint64_t>(target)*48);
                        notice=L"Selected lyric seek queued without changing mic or music state.";
                        break;
                    }
                    case Action::Repeat:rt.repeat.store(!rt.repeat.load());break;
                    case Action::ReverbDown:level(rt.wet,-0.02f,0.25f);break;
                    case Action::ReverbUp:level(rt.wet,0.02f,0.25f);break;
                    case Action::EchoDown:level(rt.echo,-0.02f,0.35f);break;
                    case Action::EchoUp:level(rt.echo,0.02f,0.35f);break;
                    case Action::MusicDown:level(rt.musicGain,-0.03f,0.5f);break;
                    case Action::MusicUp:level(rt.musicGain,0.03f,0.5f);break;
                    case Action::GainDown:level(rt.gain,-0.02f,0.35f);break;
                    case Action::GainUp:level(rt.gain,0.02f,0.35f);break;
                    case Action::Reconnect:safeStop();safeRestart();break;
                    case Action::ChooseSong:{
                        rt.monitoring.store(false);rt.musicPlaying.store(false);
                        const auto chosen=chooseSongFile();
                        if(chosen.empty()){notice=L"Song selection canceled; microphone and music muted.";break;}
                        auto next=loadSong("",chosen);
                        if(next.empty())throw std::runtime_error("No decoded song frames");
                        safeStop();
                        rt.seekRequest.store(std::numeric_limits<std::uint64_t>::max());
                        rt.rewindRequest.store(false);
                        rt.mixer.replaceSong(std::move(next));
                        rt.songPosition.store(0);
                        rt.session=roomtone::Handoff{};hasHandoff=false;
                        rt.abLoop=false;rt.abStart=rt.abEnd=0;rt.repeat.store(false);
                        selectedSongFile=std::filesystem::path(chosen).filename().u8string();
                        displayTitle=roomtone::desktop::wide(selectedSongFile);
                        safeRestart();
                        notice=L"Song replaced; previous source-owned lyrics cleared. Import matching .rtk.";
                        break;
                    }
                    case Action::ChooseSession:{
                        rt.monitoring.store(false);rt.musicPlaying.store(false);
                        const auto path=chooseHandoffFile();
                        if(path.empty()){notice=L"Session selection canceled; microphone and music muted.";break;}
                        const auto loaded=roomtone::loadHandoff(std::filesystem::path(path));
                        if(loaded.audioName!=selectedSongFile)
                            throw std::runtime_error("Session filename must match selected original instrumental");
                        safeStop();
                        rt.session=loaded;hasHandoff=true;rt.repeat.store(loaded.loop);
                        rt.abStart=std::min<std::size_t>(loaded.loopStartMs*48ULL,rt.mixer.songFrames());
                        rt.abEnd=std::min<std::size_t>(loaded.loopEndMs*48ULL,rt.mixer.songFrames());
                        rt.abLoop=loaded.loop&&rt.abEnd>rt.abStart+12000;
                        rt.seekRequest.store(std::min<std::uint64_t>(loaded.startMs*48ULL,rt.mixer.songFrames()));
                        displayTitle=roomtone::desktop::wide(loaded.title);
                        safeRestart();
                        notice=L"Matching local session imported. Voice MUTED; music PAUSED.";
                        break;
                    }
                    case Action::ChooseDevices:{
                        safeStop();
                        ma_device_info *outs=nullptr,*ins=nullptr;
                        ma_uint32 no=0,ni=0;
                        if(ma_context_get_devices(&context,&outs,&no,&ins,&ni)!=MA_SUCCESS)
                            throw std::runtime_error("Cannot enumerate devices");
                        // Copy labels before modal loop, never keep raw enumeration
                        // pointers across owner interaction or possible hotplug.
                        std::vector<std::string> candidateInputs,candidateOutputs;
                        std::vector<std::string> candidateInputIds,candidateOutputIds;
                        for(ma_uint32 i=0;i<ni;++i){
                            candidateInputs.emplace_back(ins[i].name);
                            candidateInputIds.push_back(endpointId(ins[i]));
                        }
                        for(ma_uint32 i=0;i<no;++i){
                            candidateOutputs.emplace_back(outs[i].name);
                            candidateOutputIds.push_back(endpointId(outs[i]));
                        }
                        std::vector<std::wstring> inputLabels,outputLabels;
                        for(const auto& name:candidateInputs)inputLabels.push_back(roomtone::desktop::wide(name));
                        for(const auto& name:candidateOutputs)outputLabels.push_back(roomtone::desktop::wide(name));
                        const auto choice=roomtone::desktop::selectDevices(inputLabels,outputLabels,window.hwnd,
                            roomtone::device_policy::chooserIndex(inputName,candidateInputs),
                            roomtone::device_policy::chooserIndex(outputName,candidateOutputs));
                        if(choice){
                            std::string nextInput=inputName,nextOutput=outputName;
                            if(!roomtone::device_policy::selectedName(choice->first,candidateInputs,nextInput)||
                               !roomtone::device_policy::selectedName(choice->second,candidateOutputs,nextOutput)){
                                notice=L"Device changed while selecting or name is ambiguous. Previous choice retained; voice muted.";
                                safeRestart();break;
                            }
                            // The chooser can remain open during physical USB changes.
                            // Verify both snapshot IDs against a NEW enumeration before
                            // changing the user's device selection or endpoint pins.
                            ma_device_info *verifiedOuts=nullptr,*verifiedIns=nullptr;
                            ma_uint32 verifiedNo=0,verifiedNi=0;
                            if(ma_context_get_devices(&context,&verifiedOuts,&verifiedNo,
                                                     &verifiedIns,&verifiedNi)!=MA_SUCCESS){
                                notice=L"Cannot validate devices after chooser. Previous endpoints retained; muted.";
                                break;
                            }
                            std::vector<std::string> afterInputs,afterOutputs,afterInputIds,afterOutputIds;
                            for(ma_uint32 i=0;i<verifiedNi;++i){
                                afterInputs.emplace_back(verifiedIns[i].name);
                                afterInputIds.push_back(endpointId(verifiedIns[i]));
                            }
                            for(ma_uint32 i=0;i<verifiedNo;++i){
                                afterOutputs.emplace_back(verifiedOuts[i].name);
                                afterOutputIds.push_back(endpointId(verifiedOuts[i]));
                            }
                            if(!roomtone::device_policy::retainsCandidate(
                                  choice->first,candidateInputs,candidateInputIds,afterInputs,afterInputIds)||
                               !roomtone::device_policy::retainsCandidate(
                                  choice->second,candidateOutputs,candidateOutputIds,afterOutputs,afterOutputIds)){
                                notice=L"Device list or endpoint ID changed during selection. Previous endpoint retained; muted. Reopen selector.";
                                break;
                            }
                            inputName=std::move(nextInput);
                            outputName=std::move(nextOutput);
                            // Only an explicitly approved and freshly revalidated
                            // selection may authorize different WASAPI endpoint IDs.
                            pinnedInputId.clear();pinnedOutputId.clear();
                        }
                        safeRestart();
                        if(!choice)notice=L"Device chooser canceled; previous selection restored, both outputs muted.";
                        break;
                    }
                    }
                }catch(const std::exception& e){
                    rt.monitoring.store(false);rt.musicPlaying.store(false);
                    notice=L"Operation rejected: "+roomtone::desktop::wide(e.what())+L". Monitoring muted.";
                }
            };
            const int result=window.run();
            safeStop();ma_context_uninit(&context);
            return result;
        }
        std::cout<<"\nRUNNING MIC MUTED + SONG PAUSED. Requested "<<opt.period<<" frames at 48000 Hz (NOT measured latency).\n"
                 <<"Predecoded backing frames: "<<rt.mixer.songFrames()<<"\n"
                 <<"HEADPHONES FIRST; NO acoustic feedback detection. Physical microphone mute is fastest.\n"
                 <<"Commands m, p, r, open, load PATH, seek SECONDS (+/- relative),\n"
                 <<"loop on/off, reconnect,\n"
                 <<"open/load, lyrics, next-line (from handoff), status, live effects, q.\n";
        std::string command;
        while(std::getline(std::cin,command)){
            try {
                std::istringstream in(command);
                std::string op,value,extra;
                in>>op;if(op.empty())continue;
                if(op=="q"||op=="quit")break;
                if(op=="open"||op=="load"){
                    rt.monitoring.store(false);rt.musicPlaying.store(false);
                    std::vector<float> next;
                    if(op=="open"){
                        const auto chosen=chooseSongFile();
                        if(chosen.empty()){std::cout<<"Selection cancelled; previous song retained, mic and music muted.\n";continue;}
                        next=loadSong("",chosen);
                    }else{
                        std::string path;std::getline(in,path);
                        const auto first=path.find_first_not_of(" \t");
                        if(first==std::string::npos)throw std::invalid_argument("load requires a local path");
                        path.erase(0,first);
                        if(path.size()>=2&&path.front()=='"'&&path.back()=='"')path=path.substr(1,path.size()-2);
                        next=loadSong(path);
                    }
                    if(next.empty())throw std::runtime_error("Selected file has no audio");
                    // Stop and join the real-time thread before replacing audio memory.
                    if(deviceActive){ma_device_uninit(&device);deviceActive=false;}
                    rt.seekRequest.store(std::numeric_limits<std::uint64_t>::max());
                    rt.rewindRequest.store(false);
                    rt.mixer.replaceSong(std::move(next));
                    rt.session=roomtone::Handoff{};rt.abLoop=false;rt.repeat.store(false);hasHandoff=false;
                    rt.mixer.vocal.reset();
                    rt.songPosition.store(0);
                    if(startDevice())std::cout<<"New song loaded: MIC MUTED + SONG PAUSED. Press p to play.\n";
                    else std::cout<<"New song loaded, but device unavailable. Retry reconnect.\n";
                }else if(op=="reconnect"){
                    rt.monitoring.store(false);rt.musicPlaying.store(false);
                    if(deviceActive){ma_device_uninit(&device);deviceActive=false;}
                    roomtone::recovery::freeze(
                        rt.seekRequest,rt.rewindRequest,rt.songPosition,rt.mixer.songFrames());
                    // Callback is fully stopped; clear stale reflections.
                    rt.mixer.vocal.reset();
                    if(startDevice())std::cout<<"Reconnected MUTED + PAUSED; song position preserved.\n";
                    else std::cout<<"Reconnect failed; voice and music remain muted. Retry after checking devices.\n";
                }else if(op=="seek"){
                    if(!rt.mixer.hasSong()){std::cout<<"No song loaded\n";continue;}
                    in>>value;if(value.empty()||(in>>extra))throw std::invalid_argument("seek requires one seconds value");
                    const auto seek=roomtone::parseSeekFrames(value,
                        static_cast<std::size_t>(rt.songPosition.load()),rt.mixer.songFrames());
                    rt.seekRequest.store(static_cast<std::uint64_t>(seek));
                    std::cout<<"Seek queued to "<<seek/48000.0<<"s; voice monitoring unchanged.\n";
                }else if(op=="loop"){
                    in>>value;
                    if((value!="on"&&value!="off")||(in>>extra))
                        throw std::invalid_argument("loop requires on or off");
                    rt.repeat.store(value=="on");
                    std::cout<<"Repeat "<<value<<"\n";
                }else if(op=="m"||op=="mute"){
                    if(!deviceActive||rt.interrupted.load()){
                        std::cout<<"Audio device unavailable; reconnect before monitoring.\n";continue;
                    }
                    const bool next=!rt.monitoring.load();
                    rt.monitoring.store(next);
                    std::cout<<(next?"MIC ON — stop immediately for acoustic feedback":"MIC MUTED")<<"\n";
                }else if(op=="p"||op=="play"){
                    if(!deviceActive||rt.interrupted.load()){
                        std::cout<<"Audio device unavailable; reconnect before playback.\n";continue;
                    }
                    if(!rt.mixer.hasSong()){std::cout<<"No song loaded\n";continue;}
                    const bool next=!rt.musicPlaying.load();
                    if(next&&rt.songPosition.load()>=rt.mixer.songFrames())rt.rewindRequest.store(true);
                    rt.musicPlaying.store(next);
                    std::cout<<(next?"SONG PLAYING":"SONG PAUSED")<<"\n";
                }else if(op=="r"||op=="restart"){
                    if(!deviceActive||rt.interrupted.load()){
                        std::cout<<"Audio device unavailable; reconnect before playback.\n";continue;
                    }
                    if(!rt.mixer.hasSong()){std::cout<<"No song loaded\n";continue;}
                    rt.rewindRequest.store(true);rt.musicPlaying.store(true);
                    std::cout<<"SONG RESTART REQUESTED\n";
                }else if(op=="lyrics"||op=="next-line"){
                    if(!hasHandoff||rt.session.lines.empty()){
                        std::cout<<"No imported timed lyrics. Choose --handoff session.rtk\n";continue;
                    }
                    const auto ms=static_cast<std::uint32_t>(
                        std::min<std::uint64_t>(rt.songPosition.load()/48,720000));
                    const auto index=roomtone::lyricIndexAt(rt.session,ms);
                    if(op=="next-line"){
                        const auto next=index==rt.session.lines.size()?0:index+1;
                        if(next>=rt.session.lines.size()){std::cout<<"No later lyric\n";continue;}
                        const auto target=std::clamp<std::int64_t>(
                            static_cast<std::int64_t>(rt.session.lines[next].milliseconds)+rt.session.offsetMs,
                            0,static_cast<std::int64_t>(rt.mixer.songFrames()/48));
                        rt.seekRequest.store(static_cast<std::uint64_t>(target)*48);
                        std::cout<<"Next lyric seek queued at "<<target/1000.0<<" seconds\n";
                    }else{
                        std::cout<<"Song "<<ms/1000.0<<"s | ";
                        if(index==rt.session.lines.size())std::cout<<"Get ready...";
                        else std::cout<<rt.session.lines[index].text;
                        if(index+1<rt.session.lines.size())
                            std::cout<<" | NEXT: "<<rt.session.lines[index+1].text;
                        std::cout<<"\n";
                    }
                }else if(op=="status"){
                    std::cout<<"Mic "<<(rt.monitoring.load()?"ON":"MUTED")
                             <<"; song "<<(rt.musicPlaying.load()?"PLAYING":"PAUSED")
                             <<"; position "<<rt.songPosition.load()/48000<<"/"<<rt.mixer.songFrames()/48000
                             <<" seconds; repeat="<<(rt.repeat.load()?"on":"off")
                             <<"; driver="<<(rt.interrupted.load()?"INTERRUPTED":"running")
                             <<" (callback-reported; NOT synchronized with browser)\n";
                }else{
                    in>>value;
                    if(value.empty()||(in>>extra))
                        throw std::invalid_argument("expected one numeric argument");
                    if(!changeLevel(rt,op,value))throw std::invalid_argument("unknown command");
                    std::cout<<op<<" -> "<<value<<" (applies on next callback)\n";
                }
            }catch(const std::exception& e){
                std::cout<<"Rejected command: "<<e.what()<<"\n";
            }
        }
        rt.monitoring.store(false);rt.musicPlaying.store(false);
        if(deviceActive)ma_device_uninit(&device);
        ma_context_uninit(&context);
        std::cout<<"Stopped. No audio was saved.\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"Invalid configuration: "<<e.what()<<"\n";
        return 1;
    }
#else
    (void)argc;(void)argv;
    std::cerr<<"This experimental executable requires Windows WASAPI.\n";
    return 1;
#endif
}
