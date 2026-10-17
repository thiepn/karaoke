#include "../src/rehearsal_cues.hpp"
#include <iostream>
#include <stdexcept>
static void need(bool p,const char* error){if(!p)throw std::runtime_error(error);}
int main(){
 using roomtone::rehearsal::Cues;
 Cues cues;
 constexpr std::uint64_t second=48000;
 need(!cues.add(0,0)&&!cues.add(10*second,10*second),"no source or EOF marker");
 need(cues.add(5*second,80*second),"first owner position");
 need(cues.add(2*second,80*second)&&cues.add(12*second,80*second),"unordered bookmarks sorted");
 need(cues.at(0,80*second)==2*second&&cues.at(2,80*second)==12*second,
      "frame-exact original positions");
 need(!cues.add(5*second+4800,80*second),"duplicate within 100ms rejected");
 need(!cues.add(5*second-4500,80*second),"backward near-duplicate rejected");
 need(cues.add(5*second+4801,80*second),"distinct nearby segment allowed");
 need(!cues.at(3,5*second),"stale index outside shortened song rejected");
 need(!cues.at(99,80*second),"invalid browse index no seek");
 need(cues.remove(1)&&cues.size()==3,"specific bookmark removed");
 need(!cues.remove(9),"invalid removal retains all");
 auto saved=cues.items();
 Cues other;
 other.restore(saved,80*second);
 need(other.items()==saved,"source-owned queue roundtrip in memory");
 other.restore({80*second,7*second,7*second,9*second,2*second,0},10*second);
 need(other.size()==4&&other.at(0,10*second)==0&&
      other.at(3,10*second)==9*second,"restore rejects EOF and duplicate, sorts valid frames");
 other.restore(saved,2*second);
 need(other.size()==0,"shortened replacement file cannot adopt stale cue positions");
 other.restore({},80*second);
 need(other.size()==0,"explicit new source clears old cues");
 Cues cap;
 for(std::size_t i=0;i<Cues::MaxCues;++i)
  need(cap.add((i+1)*second,30*second),"bounded 12 rehearsal positions");
 need(!cap.add(14*second,30*second)&&cap.size()==Cues::MaxCues,"bounded memory");
 need(cap.clear()==Cues::MaxCues&&cap.clear()==0,"explicit clear is idempotent");
 std::cout<<"PASS native original-source rehearsal cues: 12 slots, precision, duplicate bounds, queue snapshot and replacement invalidation.\n";
}
