#pragma once
#include <algorithm>
#include <atomic>
#include <cmath>
namespace roomtone::signal_diagnostics {
// Bounded buffer peaks from the ORIGINAL WASAPI callback. Not sound pressure,
// output device latency, acoustic feedback detection, or a recording.
inline float sample(float value) noexcept {
 return std::isfinite(value)?std::min(std::abs(value),1.0f):0.0f;
}
inline int percentage(float peak) noexcept {
 return static_cast<int>(std::lround(sample(peak)*100.0f));
}
inline bool inputNearClipping(float peak) noexcept {return sample(peak)>=0.98f;}
inline bool outputAtLimiter(float peak) noexcept {return sample(peak)>=0.699f;}
class Levels {
 std::atomic<float> capture_{0},output_{0};
public:
 void publish(float capturePeak,float outputPeak)noexcept {
  capture_.store(sample(capturePeak),std::memory_order_relaxed);
  output_.store(sample(outputPeak),std::memory_order_relaxed);
 }
 void clear()noexcept{publish(0,0);}
 float capture()const noexcept{return capture_.load(std::memory_order_relaxed);}
 float output()const noexcept{return output_.load(std::memory_order_relaxed);}
};
}
