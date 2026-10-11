#include "../src/vocal_dsp.hpp"
#include <stdexcept>
#include <string>
static void require(bool value, const char* msg) { if(!value) throw std::runtime_error(msg); }
#include <cmath>
#include <iostream>
int main() {
    using roomtone::VocalDsp;
    VocalDsp dry(0.0f,0.2f);
    const auto immediate = dry.process(0.5f);
    require(immediate.left > 0.09f && immediate.right > 0.09f,"DSP assertion failed");
    require(std::abs(immediate.left-immediate.right)<1e-7f,"DSP assertion failed");
    std::cout<<"PASS immediate direct vocal sample (no DSP lookahead)\n";
    VocalDsp wet(0.25f,0.2f);
    double tail=0;
    for(int i=0;i<8000;++i) {
        auto s=wet.process(i==0 ? 0.8f:0.0f);
        require(std::isfinite(s.left) && std::isfinite(s.right),"DSP assertion failed");
        require(std::abs(s.left)<=0.350001f && std::abs(s.right)<=0.350001f,"DSP assertion failed");
        if(i>1400)tail+=std::abs(s.left-s.right);
    }
    require(tail > 0.00001,"DSP assertion failed");
    std::cout<<"PASS bounded stereo wet reflections\n";
    VocalDsp safety(0.25f,0.35f);
    const float input[]={NAN,INFINITY,-INFINITY,100.f,-100.f};
    for(float x:input) {
        auto y=safety.process(x);
        require(std::isfinite(y.left)&&std::isfinite(y.right),"DSP assertion failed");
        require(std::abs(y.left)<=0.350001f&&std::abs(y.right)<=0.350001f,"DSP assertion failed");
    }
    safety.reset();
    auto zero=safety.process(0.0f);
    require(zero.left==0.0f&&zero.right==0.0f,"DSP assertion failed");
    std::cout<<"PASS invalid-input containment, output ceiling and reset\n";
    std::cout<<"Hardware latency, Windows audio routing and acoustic feedback NOT measured.\n";
}
