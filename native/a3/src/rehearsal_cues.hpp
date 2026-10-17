#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace roomtone::rehearsal {
// Owner-selected frame positions only. The ORIGINAL per-song queue owns
// continuity; no second playback clock, recording, database or .rtk extension.
// Always reuse the one original 48 kHz song transport for actual seeking.
class Cues {
public:
 static constexpr std::size_t MaxCues=12;
 static constexpr std::uint64_t DuplicateWindowFrames=4800; // within 100 ms
 bool add(std::uint64_t frame,std::uint64_t songFrames) {
  if(songFrames==0||frame>=songFrames||frames_.size()>=MaxCues)return false;
  if(std::any_of(frames_.begin(),frames_.end(),[&](std::uint64_t at){
    return at>frame?at-frame<=DuplicateWindowFrames:frame-at<=DuplicateWindowFrames;
  }))return false;
  frames_.insert(std::lower_bound(frames_.begin(),frames_.end(),frame),frame);
  return true;
 }
 std::size_t size()const noexcept{return frames_.size();}
 const std::vector<std::uint64_t>& items()const noexcept{return frames_;}
 std::optional<std::uint64_t> at(std::size_t index,std::uint64_t songFrames)const noexcept{
  if(index>=frames_.size()||frames_[index]>=songFrames||!songFrames)return std::nullopt;
  return frames_[index];
 }
 bool remove(std::size_t index) {
  if(index>=frames_.size())return false;
  frames_.erase(frames_.begin()+static_cast<std::ptrdiff_t>(index));
  return true;
 }
 std::size_t clear()noexcept{
  const auto removed=frames_.size();frames_.clear();return removed;
 }
 void restore(const std::vector<std::uint64_t>& saved,std::uint64_t songFrames){
  frames_.clear();
  for(auto frame:saved)add(frame,songFrames);
 }
private:
 std::vector<std::uint64_t> frames_;
};
}
