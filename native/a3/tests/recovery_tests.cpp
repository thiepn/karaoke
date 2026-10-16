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
 std::cout<<"PASS native recovery policy: duplicate/missing identity refusal, reordering, default, pending seek/rewind and clamped resume\n";
}
