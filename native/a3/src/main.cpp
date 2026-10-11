// ROOMTONE A3: minimal Windows-native experimental microphone monitor.
// This is a standalone WASAPI full-duplex executable, NOT a replacement or
// wrapped version of the web application. No accounts, file access or uploads.
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "vocal_dsp.hpp"
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
    bool list = false;
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
        if(key=="--help"||key=="-h"){
            std::cout<<"ROOMTONE A3 Windows WASAPI proof-of-concept\n"
                     <<"  --list  list device indices (0-based)\n"
                     <<"  --input N --output N  select indices from --list\n"
                     <<"  --period 120|240|480  requested buffer frames at 48000 Hz\n"
                     <<"  --wet 0..0.25  short parallel reverb, default 0.12\n"
                     <<"  --gain 0..0.35  output level, default 0.18\n"
                     <<"Starts MUTED. Type m then Enter to toggle; q then Enter to quit.\n";
            std::exit(0);
        }
        if(i+1>=argc)throw std::invalid_argument("missing value for "+key);
        const std::string value=argv[++i];
        if(key=="--input") opt.input=readInteger(value);
        else if(key=="--output") opt.output=readInteger(value);
        else if(key=="--period") opt.period=static_cast<unsigned>(readInteger(value));
        else if(key=="--wet") opt.wet=readFloat(value);
        else if(key=="--gain") opt.gain=readFloat(value);
        else throw std::invalid_argument("unknown argument: "+key);
    }
    if(opt.input < -1 || opt.output < -1)throw std::invalid_argument("device index must be -1 or greater");
    if(opt.period!=120 && opt.period!=240 && opt.period!=480)
        throw std::invalid_argument("period must be 120, 240 or 480 frames");
    if(opt.wet<0||opt.wet>0.25f||opt.gain<0||opt.gain>0.35f)
        throw std::invalid_argument("wet/gain out of safe test range");
    return opt;
}

struct Runtime {
    explicit Runtime(const Options& o): dsp(o.wet,o.gain) {}
    roomtone::VocalDsp dsp;
    std::atomic<bool> monitoring{false};
    float envelope{0.0f}; // audio thread only
};

static void callback(ma_device* device, void* output, const void* input, ma_uint32 frames) {
    auto* rt=static_cast<Runtime*>(device->pUserData);
    if(!output||!rt)return;
    auto* out=static_cast<float*>(output);
    const auto* in=static_cast<const float*>(input);
    for(ma_uint32 i=0;i<frames;i++) {
        const bool active=rt->monitoring.load(std::memory_order_relaxed);
        const float desired=active ? 1.0f : 0.0f;
        // 20ms full-scale ramp to prevent abrupt button-induced loud transients.
        if(rt->envelope<desired)rt->envelope=std::min(desired,rt->envelope+1.0f/960.0f);
        if(rt->envelope>desired)rt->envelope=std::max(desired,rt->envelope-1.0f/960.0f);
        auto voice=rt->dsp.process(in ? in[i] : 0.0f);
        // An absent capture input is never treated as permission to monitor.
        const float amplitude=(in ? rt->envelope : 0.0f);
        out[i*2]   =voice.left * amplitude;
        out[i*2+1] =voice.right * amplitude;
    }
}

int main(int argc,char** argv) {
#ifdef _WIN32
    try {
        const Options opt=parse(argc,argv);
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
        if(opt.input>=static_cast<int>(inCount)||opt.output>=static_cast<int>(outCount)){
            std::cerr<<"Invalid device index. Run --list first.\n";
            ma_context_uninit(&context);return 4;
        }
        if(opt.input>=0)std::cout<<"Input: "<<inputs[opt.input].name<<"\n";
        if(opt.output>=0)std::cout<<"Output: "<<outputs[opt.output].name<<"\n";
        // Duplex mode synchronizes capture/render through one low-latency callback.
        ma_device_config config=ma_device_config_init(ma_device_type_duplex);
        config.capture.format=ma_format_f32; config.capture.channels=1;
        config.playback.format=ma_format_f32;config.playback.channels=2;
        config.sampleRate=roomtone::VocalDsp::kSampleRate;
        config.periodSizeInFrames=opt.period;
        config.performanceProfile=ma_performance_profile_low_latency;
        if(opt.input>=0)config.capture.pDeviceID=&inputs[opt.input].id;
        if(opt.output>=0)config.playback.pDeviceID=&outputs[opt.output].id;
        Runtime rt(opt);
        config.pUserData=&rt;
        config.dataCallback=callback;
        ma_device device{};
        if(ma_device_init(&context,&config,&device)!=MA_SUCCESS){
            std::cerr<<"WASAPI duplex initialization failed; try --period 480, default devices, or matching 48kHz Windows device formats.\n";
            ma_context_uninit(&context);return 5;
        }
        // No audio can reach speakers until the user enters 'm' explicitly.
        if(ma_device_start(&device)!=MA_SUCCESS){
            std::cerr<<"WASAPI duplex start failed; device may be busy or unavailable.\n";
            ma_device_uninit(&device);ma_context_uninit(&context);return 6;
        }
        std::cout<<"\nRUNNING MUTED. Requested "<<opt.period<<" frames at 48000 Hz (not measured round-trip latency).\n"
                 <<"HEADPHONES FIRST. If trying speakers, start with their physical volume very low and microphone aimed away.\n"
                 <<"Acoustic feedback is possible; there is NO automated feedback detector.\n"
                 <<"Type m + Enter to toggle monitoring; q + Enter to stop. Hardware microphone mute is fastest.\n";
        std::string command;
        while(std::getline(std::cin,command)){
            if(command=="q"||command=="quit")break;
            if(command=="m"||command=="mute"){
                const bool next=!rt.monitoring.load();
                rt.monitoring.store(next);
                std::cout<<(next?"MONITOR ON — stop immediately if ringing occurs.":"MONITOR MUTED.")<<"\n";
            }
        }
        rt.monitoring.store(false);
        ma_device_uninit(&device);
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
