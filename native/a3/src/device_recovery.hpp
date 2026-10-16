#pragma once
#include <string>
#include <vector>
#include <atomic>
#include <algorithm>
#include <cstdint>
#include <limits>

namespace roomtone::device_policy {
enum class State { Default, Unique, Missing, Ambiguous };
struct Match { State state; int index; };
inline Match resolve(const std::string& name,const std::vector<std::string>& available){
 if(name.empty())return {State::Default,-1};
 int found=-1;
 for(std::size_t i=0;i<available.size();++i)if(name==available[i]){
  if(found>=0)return {State::Ambiguous,-1};
  found=static_cast<int>(i);
 }
 return found<0?Match{State::Missing,-1}:Match{State::Unique,found};
}
// Explicit owner device identity; reject same-name hardware rather than silently
// routing voice or music to an arbitrary replacement on reconnect.
struct Selection {
 std::string inputName,outputName;
 bool inputDefault=true,outputDefault=true;
};
inline bool resolvePair(const Selection& selection,
                        const std::vector<std::string>& inputs,
                        const std::vector<std::string>& outputs,
                        int& inputIndex,int& outputIndex) {
 const Match in=resolve(selection.inputDefault?std::string{}:selection.inputName,inputs);
 const Match out=resolve(selection.outputDefault?std::string{}:selection.outputName,outputs);
 if((in.state!=State::Unique&&in.state!=State::Default)||
    (out.state!=State::Unique&&out.state!=State::Default))return false;
 inputIndex=in.index;outputIndex=out.index;
 return true;
}
inline bool selectedName(int selected,const std::vector<std::string>& visible,std::string& result){
 if(selected==-1){result.clear();return true;}
 if(selected<0||static_cast<std::size_t>(selected)>=visible.size())return false;
 const auto match=resolve(visible[static_cast<std::size_t>(selected)],visible);
 if(match.state!=State::Unique||match.index!=selected)return false;
 result=visible[static_cast<std::size_t>(selected)];
 return true;
}
}
namespace roomtone::recovery {
inline constexpr std::uint64_t NoSeek=std::numeric_limits<std::uint64_t>::max();
struct Frozen {std::uint64_t frames;bool pendingSeek;bool pendingRewind;};
// Call ONLY after a successfully opened device has been uninitialized and
// its callback is joined; never call on the audio render thread.
// Pending user commands take priority over the last callback snapshot.
// Keep the pending seek for the first buffer of the next device instance.
inline Frozen freeze(std::atomic<std::uint64_t>& requestedSeek,
                     std::atomic<bool>& requestedRewind,
                     std::atomic<std::uint64_t>& publishedPosition,
                     std::uint64_t songFrames) noexcept {
 const auto seek=requestedSeek.exchange(NoSeek,std::memory_order_relaxed);
 const bool rewind=requestedRewind.exchange(false,std::memory_order_relaxed);
 const auto current=publishedPosition.load(std::memory_order_relaxed);
 const auto wanted=rewind?std::uint64_t{0}:seek==NoSeek?current:seek;
 const auto clipped=std::min(wanted,songFrames);
 publishedPosition.store(clipped,std::memory_order_relaxed);
 requestedSeek.store(clipped,std::memory_order_relaxed);
 return {clipped,seek!=NoSeek,rewind};
}
}
