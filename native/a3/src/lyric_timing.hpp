#pragma once
#include "handoff.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace roomtone::lyric_timing {
// Source-owned, per-line correction for existing original .rtk/.lrc lines.
// Return a copy ONLY if the adjustment keeps source index, lyric text,
// monotonic canonical .rtk timestamps, and original 12-minute bounds intact.
// No audio callback, file writes, new sync clock, or guessing lyric content.
inline std::optional<Handoff> setSourceMs(const Handoff& original,
 std::size_t index,std::int64_t sourceMs,std::uint64_t durationFrames) {
 if(index>=original.lines.size()||durationFrames==0||sourceMs<0||
    sourceMs>720000)return std::nullopt;
 const std::int64_t observed=sourceMs+original.offsetMs;
 if(observed<0||
    static_cast<std::uint64_t>(observed)*48>durationFrames)
   return std::nullopt;
 if(index>0&&sourceMs<original.lines[index-1].milliseconds)return std::nullopt;
 if(index+1<original.lines.size()&&
    sourceMs>original.lines[index+1].milliseconds)return std::nullopt;
 if(sourceMs==original.lines[index].milliseconds)return std::nullopt;
 Handoff changed=original;
 changed.lines[index].milliseconds=static_cast<std::uint32_t>(sourceMs);
 return changed;
}
inline std::optional<Handoff> nudge(const Handoff& original,
 std::size_t index,int deltaMs,std::uint64_t durationFrames) {
 if(index>=original.lines.size()||(deltaMs!=-100&&deltaMs!=100))
   return std::nullopt;
 return setSourceMs(original,index,
    static_cast<std::int64_t>(original.lines[index].milliseconds)+deltaMs,
    durationFrames);
}
inline std::optional<Handoff> alignToFrame(const Handoff& original,
 std::size_t index,std::uint64_t ownerFrame,std::uint64_t durationFrames) {
 if(!durationFrames||ownerFrame>=durationFrames)return std::nullopt;
 const auto sourceMs=static_cast<std::int64_t>(ownerFrame/48)-
                     static_cast<std::int64_t>(original.offsetMs);
 return setSourceMs(original,index,sourceMs,durationFrames);
}
}
