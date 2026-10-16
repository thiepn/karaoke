#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <string>

namespace roomtone {
// Parse in the control thread ONLY: bare seconds are absolute, +/- relative.
// Never call this from the realtime audio callback.
inline std::size_t parseSeekFrames(const std::string& argument,
                                  std::size_t current,
                                  std::size_t duration,
                                  unsigned sampleRate=48000) {
    if(argument.empty()||!sampleRate)throw std::invalid_argument("seek requires seconds");
    std::size_t consumed=0;
    const double seconds=std::stod(argument,&consumed);
    if(consumed!=argument.size()||!std::isfinite(seconds))
        throw std::invalid_argument("seek requires finite seconds");
    const bool relative=argument[0]=='+'||argument[0]=='-';
    if(!relative&&seconds<0)throw std::invalid_argument("absolute seek must be >= 0");
    const long double delta=static_cast<long double>(seconds)*sampleRate;
    const long double proposed=(relative?static_cast<long double>(current):0)+delta;
    const long double clipped=std::clamp(proposed,0.0L,static_cast<long double>(duration));
    return static_cast<std::size_t>(clipped);
}
}
