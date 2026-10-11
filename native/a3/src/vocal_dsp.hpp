#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

// Real-time processing: no allocation, mutex, logging or sleeps in process().
namespace roomtone {
struct Stereo { float left; float right; };
class VocalDsp {
public:
    static constexpr unsigned kSampleRate = 48000;
    static constexpr std::size_t kCapacity = 4096;
    explicit VocalDsp(float wet = 0.12f, float gain = 0.18f) noexcept
      : wet_(std::clamp(wet, 0.0f, 0.25f)),
        gain_(std::clamp(gain, 0.0f, 0.35f)) {}
    void reset() noexcept {
        for (auto& delay : delays_) delay.fill(0.0f);
        positions_.fill(0);
        damped_.fill(0.0f);
        previousInput_ = 0.0f;
        highpass_ = 0.0f;
    }
    Stereo process(float input) noexcept {
        if (!std::isfinite(input)) input = 0.0f;
        input = std::clamp(input, -1.0f, 1.0f);
        // One-pole DC/high-pass filter with no lookahead or dry delay.
        const float hp = input - previousInput_ + 0.99f * highpass_;
        previousInput_ = input;
        highpass_ = hp;
        // Four restrained feedback lines with different L/R time constants.
        // Wet returns are parallel: the dry voice is never delayed by the effect.
        const float a = tap(0, hp, 1637);
        const float c = tap(1, hp, 2399);
        const float d = tap(2, hp, 1823);
        const float e = tap(3, hp, 2741);
        return {limit((hp + wet_ * (a + c) * 0.5f) * gain_),
                limit((hp + wet_ * (d + e) * 0.5f) * gain_)};
    }
private:
    float tap(std::size_t which, float x, std::size_t length) noexcept {
        auto& buffer = delays_[which];
        auto& pos = positions_[which];
        const float old = buffer[pos];
        damped_[which] = old * 0.35f + damped_[which] * 0.65f;
        buffer[pos] = x * 0.24f + damped_[which] * 0.26f;
        pos = (pos + 1) % length;
        return old;
    }
    static float limit(float x) noexcept {
        return std::clamp(std::isfinite(x) ? x : 0.0f, -0.35f, 0.35f);
    }
    std::array<std::array<float,kCapacity>,4> delays_{};
    std::array<std::size_t,4> positions_{};
    std::array<float,4> damped_{};
    float previousInput_{0.0f};
    float highpass_{0.0f};
    float wet_;
    float gain_;
};
}
