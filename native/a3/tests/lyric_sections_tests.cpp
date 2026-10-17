#include "../src/lyric_sections.hpp"
#include <iostream>
#include <stdexcept>
using namespace roomtone;
static void need(bool result,const char* reason){if(!result)throw std::runtime_error(reason);}
int main(){
 constexpr std::uint64_t sec=48000;
 Handoff source;source.audioName="original.wav";source.title="Source owned";
 source.offsetMs=125;
 source.lines={{0,"Get ready"},{1000,"First"},{1000,"Duplicate cue"},
               {4000,"Second"},{8000,"Chorus"},{14000,"Last"}};
 const auto section=lyric_sections::choose(source,1,4,20*sec);
 need(section&&section->startFrame==1125*48&&section->endFrame==8125*48,
     "select arbitrary authored verse-to-chorus bounds with lyric offset");
 need(section->startLine==1&&section->endLine==4,"retain exact original lyric indices");
 const auto toEnd=lyric_sections::choose(source,4,source.lines.size(),20*sec);
 need(toEnd&&toEnd->endFrame==20*sec&&toEnd->startFrame==8125*48,
     "song-end cue is exact PCM end, not invented extra lyric");
 need(!lyric_sections::choose(source,1,2,20*sec),
     "simultaneous/tied authoritative cues cannot make zero loop");
 need(!lyric_sections::choose(source,2,1,20*sec),
     "nonincreasing source indices rejected");
 need(!lyric_sections::choose(source,2,0,20*sec),
     "reverse bounds rejected");
 need(!lyric_sections::choose(source,source.lines.size(),source.lines.size(),20*sec),
     "end cannot be a start cue");
 need(!lyric_sections::choose(source,0,100,20*sec),
     "out of range end cue rejected");
 need(!lyric_sections::choose(source,0,1,0),"no original song rejected");
 source.offsetMs=-30000;
 need(!lyric_sections::choose(source,1,4,20*sec),
     "negative offset collapses cues and refuses false A-B");
 source.offsetMs=30000;
 need(!lyric_sections::choose(source,4,source.lines.size(),20*sec),
     "offset beyond original song duration refuses inverted A-B");
 source.offsetMs=-900;
 source.lines={{850,"Clamped"},{1100,"Just after"},{1500,"Clearly later"}};
 need(!lyric_sections::choose(source,0,1,2*sec),
     "exactly 250 ms violates original >250ms A-B contract");
 need(lyric_sections::choose(source,0,2,2*sec).has_value(),
     "valid >250 ms source interval after negative offset");
 need(!lyric_sections::marker(10,false,100),
     "owner must explicitly set marker before jump");
 need(lyric_sections::marker(10,true,100).value()==10,
     "jump uses frame-exact selected marker");
 need(!lyric_sections::marker(101,true,100),
     "stale marker outside song refuses seek");
 need(lyric_sections::marker(100,true,100).has_value(),
     "exact EOF is a valid paused cue, not autoplay");
 std::cout<<"PASS original authored lyric-cue A-B section bounds, signed offsets, collapsed timestamps, end-of-song and paused marker seek policy.\n";
}
