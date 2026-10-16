// ROOMTONE A3: minimal Windows-native experimental microphone monitor.
// This is a standalone WASAPI full-duplex executable, NOT a replacement or
// wrapped version of the web application. No accounts, file access or uploads.
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "karaoke_mix.hpp"
#include "transport.hpp"
#include <algorithm>
#include <cstdint>
#include <sstream>
#include <utility>
#include <vector>
#include <limits>
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
    std::string song;
    bool list = false,probeSong=false,pickSong=false,guided=false;
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
        if(key=="--probe-song"){opt.probeSong=true;continue;}
        if(key=="--pick-song"){opt.pickSong=true;continue;}
        if(key=="--setup"){opt.guided=true;continue;}
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
                     <<"  --setup  guided mic/speaker device selection\n"
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
        if(play&&rt->repeat.load(std::memory_order_relaxed)&&rt->mixer.songEnded())rt->mixer.rewind();
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
        if(argc==1)opt.guided=true; // Double-click opens guided setup.
        std::wstring chosenSong;
        if(!opt.list&&!opt.probeSong&&(opt.pickSong||(opt.guided&&opt.song.empty()))) {
            chosenSong=chooseSongFile();
            if(chosenSong.empty()){
                std::cout<<"Song picker cancelled. No hardware was activated.\n";
                return 0;
            }
        }
        // All decode happens before starting the WASAPI stream.
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
        if(opt.guided) {
            try{
                opt.input=chooseDeviceIndex("Microphone",inputs,inCount);
                opt.output=chooseDeviceIndex("Speaker",outputs,outCount);
            }catch(...){ma_context_uninit(&context);throw;}
        }
        if(opt.input>=static_cast<int>(inCount)||opt.output>=static_cast<int>(outCount)){
            std::cerr<<"Invalid device index. Run --list first.\n";
            ma_context_uninit(&context);return 4;
        }
        if(opt.input>=0)std::cout<<"Input: "<<inputs[opt.input].name<<"\n";
        if(opt.output>=0)std::cout<<"Output: "<<outputs[opt.output].name<<"\n";
        // Remember exact names, not volatile enumeration indexes. Reconnection
        // refuses to silently attach to a different named device.
        const std::string inputName=opt.input>=0?inputs[opt.input].name:"";
        const std::string outputName=opt.output>=0?outputs[opt.output].name:"";
        Runtime rt(opt,std::move(song));
        ma_device device{};
        bool deviceActive=false;
        const auto startDevice=[&]()->bool{
            ma_device_info *newOutputs=nullptr,*newInputs=nullptr;
            ma_uint32 newOutCount=0,newInCount=0;
            if(ma_context_get_devices(&context,&newOutputs,&newOutCount,&newInputs,&newInCount)!=MA_SUCCESS){
                std::cout<<"Cannot enumerate WASAPI devices; reconnect again later.\n";return false;
            }
            const auto resolve=[](const std::string& name,ma_device_info* devices,ma_uint32 count)->int{
                if(name.empty())return -1;
                int match=-2;
                for(ma_uint32 i=0;i<count;++i)if(name==devices[i].name){
                    if(match!=-2)return -2; // Duplicate names are ambiguous.
                    match=static_cast<int>(i);
                }
                return match;
            };
            const int capture=resolve(inputName,newInputs,newInCount);
            const int playback=resolve(outputName,newOutputs,newOutCount);
            if((!inputName.empty()&&capture<0)||(!outputName.empty()&&playback<0)){
                std::cout<<"Selected device is missing or ambiguous. Reconnect it or restart guided setup.\n";
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
                std::cout<<"WASAPI init failed; try reconnect or a larger --period value.\n";
                return false;
            }
            if(ma_device_start(&device)!=MA_SUCCESS){
                ma_device_uninit(&device);
                std::cout<<"WASAPI start failed; driver may be unavailable.\n";return false;
            }
            deviceActive=true;
            rt.interrupted.store(false);
            return true;
        };
        if(!startDevice()){
            ma_context_uninit(&context);
            return 5;
        }
        std::cout<<"\nRUNNING MIC MUTED + SONG PAUSED. Requested "<<opt.period<<" frames at 48000 Hz (NOT measured latency).\n"
                 <<"Predecoded backing frames: "<<rt.mixer.songFrames()<<"\n"
                 <<"HEADPHONES FIRST; NO acoustic feedback detection. Physical microphone mute is fastest.\n"
                 <<"Commands m, p, r, seek SECONDS (+/- relative), loop on/off, reconnect,\n"
                 <<"status, wet/gain/echo/feedback/echo-ms/music NUMBER, q.\n";
        std::string command;
        while(std::getline(std::cin,command)){
            try {
                std::istringstream in(command);
                std::string op,value,extra;
                in>>op;if(op.empty())continue;
                if(op=="q"||op=="quit")break;
                if(op=="reconnect"){
                    rt.monitoring.store(false);rt.musicPlaying.store(false);
                    if(deviceActive){ma_device_uninit(&device);deviceActive=false;}
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
