#include "../src/device_recovery.hpp"
#include <iostream>
#include <stdexcept>
#include <atomic>
#include <vector>
static void need(bool p,const char* reason){if(!p)throw std::runtime_error(reason);}
int main(){
 using namespace roomtone::device_policy;
 std::vector<std::string> first={"USB QuadCast","Built-in microphone","USB QuadCast"};
 need(resolve("",first).state==State::Default,"default explicit");
 need(resolve("USB QuadCast",first).state==State::Ambiguous,"duplicate name fail closed");
 need(resolve("Bluetooth mic",first).state==State::Missing,"disconnected name fail closed");
 need(resolve("Built-in microphone",first).index==1,"unique index");
 Selection pair;
 pair.inputDefault=false;pair.inputName="Built-in microphone";
 pair.outputDefault=false;pair.outputName="USB QuadCast";
 int inIndex=999,outIndex=999;
 need(!resolvePair(pair,first,first,inIndex,outIndex),"ambiguous output must fail closed");
 pair.outputName="Built-in microphone";
 need(resolvePair(pair,first,first,inIndex,outIndex)&&inIndex==1&&outIndex==1,"unique pair resolves");
 pair.inputDefault=true;pair.outputDefault=true;
 need(resolvePair(pair,first,first,inIndex,outIndex)&&inIndex==-1&&outIndex==-1,"explicit default pair");
 pair.inputDefault=false;pair.inputName="Bluetooth mic";
 need(!resolvePair(pair,first,first,inIndex,outIndex),"missing input pair fails");
 std::string chosen="prior";
 need(!selectedName(0,first,chosen)&&chosen=="prior","ambiguous choice cannot replace previous");
 need(selectedName(-1,first,chosen)&&chosen.empty(),"explicit system default");
 std::vector<std::string> now={"Built-in microphone","USB QuadCast"};
 need(selectedName(1,now,chosen)&&chosen=="USB QuadCast","unique user choice");
 need(resolve(chosen,{"USB QuadCast"}).index==0,"permuted index resolved from name");
 need(resolve(chosen,{"Another speaker"}).state==State::Missing,"no fallback to unrelated device");
 need(chooserIndex("",first)==-1,"default remains explicitly default");
 need(chooserIndex("Built-in microphone",first)==1,"previous selection remains highlighted");
 need(chooserIndex("USB QuadCast",first)==-2,"ambiguous previous choice is not default");
 need(chooserIndex("Unplugged",first)==-2,"missing selected device requires fresh owner choice");
 const std::vector<std::string> beforeNames{"Mic A","Speaker B"};
 const std::vector<std::string> beforeIds{"endpoint-A","endpoint-B"};
 need(retainsCandidate(0,beforeNames,beforeIds,{"Speaker B","Mic A"},{"endpoint-B","endpoint-A"}),"reordering retains actual endpoint");
 need(!retainsCandidate(0,beforeNames,beforeIds,{"Mic A","Speaker B"},{"endpoint-C","endpoint-B"}),"same name replaced during picker rejected");
 need(!retainsCandidate(0,beforeNames,beforeIds,{"Mic A","Mic A"},{"endpoint-A","endpoint-A"}),"duplicated label during picker rejected");
 need(!retainsCandidate(0,beforeNames,beforeIds,{"Speaker B"},{"endpoint-B"}),"unplug during picker rejected");
 need(!retainsCandidate(9,beforeNames,beforeIds,beforeNames,beforeIds),"stale index rejected");
 need(retainsCandidate(-1,beforeNames,beforeIds,{},{}),"explicit default remains OS-owned");
 need(acceptsEndpointId("","wasapi:device-a"),"initial explicit selection pins fresh endpoint");
 need(acceptsEndpointId("wasapi:device-a","wasapi:device-a"),"original endpoint reconnect accepted");
 need(!acceptsEndpointId("wasapi:device-a","wasapi:device-b"),"same-name replacement endpoint refused");
 need(!acceptsEndpointId("wasapi:device-a",""),"missing endpoint token refused");
 need(!acceptsEndpointId("",""),"unidentified explicit endpoint refused");
 need(!acceptsEndpointId(std::string("a\0b",3),std::string("a\0c",3)),"binary endpoint identity exact");
 need(!selectedName(2,now,chosen)&&chosen=="USB QuadCast","stale selection rejected");
 using namespace roomtone::recovery;
 std::atomic<std::uint64_t> seek{NoSeek},position{10000};
 std::atomic<bool> rewind{false};
 const auto resume=freeze(seek,rewind,position,48000);
 need(resume.frames==10000&&seek.load()==10000&&!resume.pendingSeek,"resume callback position");
 position.store(10000);seek.store(30000);
 const auto requested=freeze(seek,rewind,position,48000);
 need(requested.frames==30000&&requested.pendingSeek&&position.load()==30000,"pending user seek wins");
 position.store(35000);seek.store(45000);rewind.store(true);
 const auto reset=freeze(seek,rewind,position,48000);
 need(reset.frames==0&&reset.pendingRewind&&!rewind.load(),"pending restart wins");
 position.store(10000);seek.store(800000);
 const auto clamped=freeze(seek,rewind,position,48000);
 need(clamped.frames==48000&&seek.load()==48000,"seek bounded to current song");
 using roomtone::recovery::DeviceChoice;
 using roomtone::recovery::DriverState;
 need(roomtone::recovery::state(true,false,false)==DriverState::Ready,
      "valid running audio has ready status");
 need(roomtone::recovery::state(true,true,false)==DriverState::Interrupted,
      "interrupted callback cannot advertise ready");
 need(roomtone::recovery::state(false,false,true)==DriverState::OwnerSuspended,
      "owner suspension differs from missing hardware");
 need(roomtone::recovery::state(false,true,true)==DriverState::OwnerSuspended,
      "owner suspension stays explicit despite old driver issue");
 need(roomtone::recovery::state(false,false,false)==DriverState::Offline,
      "offline is not a trusted ready device");
 for(bool ready:{false,true}){
  need(roomtone::recovery::reopenAfterChooser(ready,DeviceChoice::Revalidated),
       "newly verified owner selection authorizes muted opening");
  need(!roomtone::recovery::reopenAfterChooser(ready,DeviceChoice::Rejected),
       "stale/ambiguous device selection never restarts either old/new endpoint");
  need(roomtone::recovery::reopenAfterChooser(ready,DeviceChoice::Canceled)==ready,
       "cancel only reopens an actual previously ready route");
 }
 need(!roomtone::recovery::ownerMayReconnect(false)&&
      roomtone::recovery::ownerMayReconnect(true),
      "reconnect requires affirmative local owner consent");
 std::cout<<"PASS owner-approved device recovery, deliberate suspension, cancellation and endpoint fail-closed policy\\n";
 std::cout<<"PASS native recovery policy: duplicate/missing identity refusal, reordering, default, pending seek/rewind and clamped resume\n";
}
