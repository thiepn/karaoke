#include "../src/lyric_timing.hpp"
#include "../src/session_export.hpp"
#include <stdexcept>
#include <iostream>
static void need(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(){
 using namespace roomtone;
 Handoff base;base.audioName="canonical.wav";base.title="Source lyrics";
 base.offsetMs=-200;base.lines={{1000,"Intro"},{2000,"Chorus"},{3000,"Outro"}};
 const auto original=base;
 constexpr std::uint64_t dur=10000ULL*48;
 const auto early=lyric_timing::nudge(base,1,-100,dur);
 need(early&&early->lines[1].milliseconds==1900,"nudge one line early");
 need(early->lines[0].milliseconds==1000&&early->lines[2].milliseconds==3000&&
      early->lines[1].text=="Chorus"&&early->audioName=="canonical.wav",
      "unchanged original source line identity and neighbors");
 need(base.lines[1].milliseconds==2000,"original source remains untouched until commit");
 const auto later=lyric_timing::nudge(base,1,100,dur);
 need(later&&later->lines[1].milliseconds==2100,"nudge one line later");
 need(!lyric_timing::nudge(base,3,-100,dur),"invalid source index");
 need(!lyric_timing::nudge(base,0,101,dur),"only bounded authorized 100ms step");
 need(!lyric_timing::setSourceMs(base,1,999,dur),"cannot overtake previous authored cue");
 need(!lyric_timing::setSourceMs(base,1,3001,dur),"cannot overtake following authored cue");
 need(!lyric_timing::setSourceMs(base,1,-1,dur),"no negative source milliseconds");
 need(!lyric_timing::setSourceMs(base,1,720001,dur),"canonical 12-minute limit");
 need(!lyric_timing::setSourceMs(base,1,0,0),"missing song refuses change");
 need(!lyric_timing::setSourceMs(base,1,2000,dur),"no-op cannot claim mutation");
 const auto tapped=lyric_timing::alignToFrame(base,1,2350ULL*48,dur);
 need(tapped&&tapped->lines[1].milliseconds==2550,
     "owner playhead aligns lyric after original signed offset");
 need(!lyric_timing::alignToFrame(base,1,250ULL*48,dur),
     "playhead alignment cannot violate source order");
 need(!lyric_timing::alignToFrame(base,1,dur,dur),
     "tapping at exact song EOF refused");
 Handoff positive=base;positive.offsetMs=500;
 need(!lyric_timing::alignToFrame(positive,0,100ULL*48,dur),
      "positive lyric offset cannot invent a negative source time");
 Handoff edge=base;edge.lines={{0,"First"},{100,"Next"}};
 edge.offsetMs=0;
 need(!lyric_timing::nudge(edge,0,-100,dur),"zero lower boundary");
 need(lyric_timing::setSourceMs(edge,0,100,dur).has_value(),
      "tied original timestamps remain legal");
 need(!lyric_timing::setSourceMs(edge,0,101,dur),
      "cannot reverse equal authored timestamps");
 Handoff end=base;end.offsetMs=0;end.lines={{719800,"Ending"}};
 need(!lyric_timing::nudge(end,0,100,100ULL*48),
      "out of actual original track duration refused");
 // Export the exact changed line through EXISTING canonical .rtk V1.
 const auto encoded=session_export::encode(*tapped);
 const auto parsed=parseHandoff(encoded);
 need(parsed.lines.size()==3&&parsed.lines[1].milliseconds==2550&&
      parsed.lines[1].text=="Chorus"&&parsed.audioName=="canonical.wav",
      "explicit original .rtk export preserves edit with no new fields");
 need(original.lines[1].milliseconds==2000,"undo restores intact source-owned previous snapshot");
 std::cout<<"PASS owner-only individual lyric timing, monotonic bounds, offsets, tap position, undo source, canonical .rtk export. No audio hardware opened.\n";
}
