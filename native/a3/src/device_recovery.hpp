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
// Never use a new same-name WASAPI endpoint without explicit owner selection.
// Tokens are private, ephemeral bytes of the WASAPI endpoint ID, not persisted.
inline bool acceptsEndpointId(const std::string& pinned,const std::string& observed){
 return !observed.empty()&&(pinned.empty()||pinned==observed);
}
// Index returned to the Win32 chooser. -1 is an explicit Windows default;
// -2 means a missing/ambiguous previously chosen endpoint, requiring owner input.
inline int chooserIndex(const std::string& selected,
                        const std::vector<std::string>& available){
 const Match match=resolve(selected,available);
 return match.state==State::Default?-1:match.state==State::Unique?match.index:-2;
}
// The user saw a particular endpoint at dialog opening. A hotplug during
// the modal window cannot silently authorize another device under the same
// display name or a reordered index. Call after fresh re-enumeration.
inline bool retainsCandidate(int choice,
 const std::vector<std::string>& beforeNames,
 const std::vector<std::string>& beforeIds,
 const std::vector<std::string>& afterNames,
 const std::vector<std::string>& afterIds){
 if(choice==-1)return true; // explicitly selected Windows default
 if(choice<0||static_cast<std::size_t>(choice)>=beforeNames.size()||
    beforeNames.size()!=beforeIds.size()||afterNames.size()!=afterIds.size())return false;
 const auto matching=resolve(beforeNames[static_cast<std::size_t>(choice)],afterNames);
 if(matching.state!=State::Unique||
    matching.index<0||static_cast<std::size_t>(matching.index)>=afterIds.size())return false;
 const auto& id=beforeIds[static_cast<std::size_t>(choice)];
 return !id.empty()&&id==afterIds[static_cast<std::size_t>(matching.index)]&&
        resolve(beforeNames[static_cast<std::size_t>(choice)],beforeNames).state==State::Unique;
}
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
// Software-only owner authorization gate for the EXISTING WASAPI duplex device.
// Neither hotplug, an offline chooser cancellation, nor a rejected candidate
// is permission to restart capture/playback.
enum class DriverState { Ready, Interrupted, OwnerSuspended, Offline };
inline DriverState state(bool active,bool interrupted,bool suspended) noexcept {
 if(suspended)return DriverState::OwnerSuspended;
 if(interrupted)return DriverState::Interrupted;
 return active?DriverState::Ready:DriverState::Offline;
}
enum class DeviceChoice { Canceled, Rejected, Revalidated };
inline bool reopenAfterChooser(bool previouslyReady,DeviceChoice decision) noexcept {
 return decision==DeviceChoice::Revalidated ||
        (decision==DeviceChoice::Canceled&&previouslyReady);
}
inline bool ownerMayReconnect(bool explicitlyApproved) noexcept{
 return explicitlyApproved;
}
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
