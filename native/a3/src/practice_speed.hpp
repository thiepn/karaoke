#pragma once
#include <algorithm>
#include <cmath>

namespace roomtone::practice_speed {
// Bounded resampling of only predecoded ORIGINAL backing PCM.
// IMPORTANT: resampling changes the instrumental's musical pitch/key.
// It is NOT time-stretching or latency compensation. Microphone/DSP stays 48kHz.
inline constexpr float Minimum=0.60f;
inline constexpr float Maximum=1.20f;
inline constexpr float Normal=1.00f;
inline float safe(float rate) noexcept {
 return std::isfinite(rate)?std::clamp(rate,Minimum,Maximum):Normal;
}
// UI steps use integer tenths to avoid drifting 0.1-float additions.
inline float step(float rate,int tenths) noexcept {
 const int at=static_cast<int>(std::lround(safe(rate)*10));
 return static_cast<float>(std::clamp(at+tenths,6,12))/10.0f;
}
inline bool standard(float rate) noexcept {
 return std::abs(safe(rate)-Normal)<0.001f;
}
}
