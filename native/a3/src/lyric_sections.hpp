#pragma once
#include "handoff.hpp"
#include "practice.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace roomtone::lyric_sections {
// A/B is derived ONLY from imported source timestamp IDs. This is NOT a new
// lyric editor, song clock, learning-progress engine or waveform source.
struct Range {
 std::uint64_t startFrame=0,endFrame=0;
 std::size_t startLine=0,endLine=0; // endLine==lines.size(): song end
};
// Index validation and signed source lyric offset are resolved on the owner
// control thread, never in WASAPI callback. Tied/collapsed cues fail closed.
inline std::optional<Range> choose(const Handoff& source,
 std::size_t first,std::size_t last,std::uint64_t songFrames) noexcept {
 if(songFrames==0||first>=source.lines.size()||last<=first||
    last>source.lines.size())return std::nullopt;
 constexpr std::uint64_t FramesPerMs=48;
 const auto adjusted=[&](std::size_t index)->std::uint64_t{
  const std::int64_t at=static_cast<std::int64_t>(source.lines[index].milliseconds)
     +static_cast<std::int64_t>(source.offsetMs);
  if(at<=0)return 0;
  return std::min<std::uint64_t>(static_cast<std::uint64_t>(at)*FramesPerMs,songFrames);
 };
 const auto a=adjusted(first);
 const auto b=last==source.lines.size()?songFrames:adjusted(last);
 if(!practice::validAB(a,b,songFrames))return std::nullopt;
 return Range{a,b,first,last};
}
inline std::optional<std::uint64_t> marker(std::uint64_t selected,
 bool marked,std::uint64_t duration) noexcept {
 if(!marked||duration==0||selected>duration)return std::nullopt;
 return selected;
}
}
