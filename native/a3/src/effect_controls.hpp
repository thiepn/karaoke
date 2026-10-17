#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace roomtone::effects {
// Pure control-thread parameter policy for the EXISTING VocalDsp.
// No second engine, saved preset library, DSP allocation or sound claims.
struct Settings {
 float wet=0.12f,gain=0.18f,echo=0.12f,feedback=0.15f,music=0.20f;
 unsigned delayMs=280;
};
inline float nudge(float from,int hundredths,float maximum) noexcept {
 const int current=static_cast<int>(std::lround(std::clamp(from,0.0f,maximum)*100));
 const int ceiling=static_cast<int>(std::lround(maximum*100));
 return static_cast<float>(std::clamp(current+hundredths,0,ceiling))/100.0f;
}
inline unsigned nudgeDelay(unsigned from,int stepMs) noexcept {
 const int current=static_cast<int>(std::clamp(from,80u,550u));
 return static_cast<unsigned>(std::clamp(current+stepMs,80,550));
}
inline Settings dryVoice(Settings existing) noexcept{
 // Reverb/echo feedback are explicitly zero, while the owner's actual
 // music and vocal output gain stay UNCHANGED (no surprise amplification).
 existing.wet=0;existing.echo=0;existing.feedback=0;
 return existing;
}
inline Settings clamp(Settings s) noexcept{
 s.wet=std::clamp(s.wet,0.0f,0.25f);
 s.gain=std::clamp(s.gain,0.0f,0.35f);
 s.echo=std::clamp(s.echo,0.0f,0.35f);
 s.feedback=std::clamp(s.feedback,0.0f,0.35f);
 s.music=std::clamp(s.music,0.0f,0.50f);
 s.delayMs=std::clamp(s.delayMs,80u,550u);
 return s;
}
}