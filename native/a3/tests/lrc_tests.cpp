#include "../src/lrc.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
using namespace std::string_literals;
static void need(bool valid,const char* label){if(!valid)throw std::runtime_error(label);}
static bool rejects(const std::string& text){
 try{(void)roomtone::parseLrc(text,"instrumental.wav","Instrumental");return false;}
 catch(const std::exception&){return true;}
}
int main(){
 auto doc=roomtone::parseLrc(
  "\xef\xbb\xbf[ti:Nacht und Licht]\r\n[ar:Unknown]\n"
  "[offset:-200]\n[00:10.00][00:15.500]One line\n"
  "[00:13.5]Second line\n[00:00.050]Intro\n",
  "instrumental.wav","Fallback");
 need(doc.audioName=="instrumental.wav"&&doc.title=="Nacht und Licht","original file and title");
 need(doc.lines.size()==4&&doc.lines[0].milliseconds==0,"negative offset clamped");
 need(doc.lines[1].milliseconds==9800&&doc.lines[1].text=="One line","centisecond line");
 need(doc.lines[2].milliseconds==13300&&doc.lines[3].milliseconds==15300,
      "fractional timestamps, multi-tag sorting");
 need(!doc.loop&&doc.offsetMs==0,"source stamps applied once without double offset");
 need(roomtone::lyricIndexAt(doc,10000)==1,"original handoff lyric lookup");
 need(roomtone::lyricJumpMs(doc,11000,true,30000)==13300,"original native lyric jump");
 need(roomtone::parseLrc("[01:04.123]Unicode \xE2\x9C\x93\n","song.mp3","Fallback")
       .lines[0].milliseconds==64123,"UTF-8 millisecond timestamp");
 need(roomtone::parseLrc("[00:02.5]Line\n[offset:250]\n[00:03]Next\n",
                         "x.wav","Default").lines[1].milliseconds==3250,
      "in-file offset updates match source browser semantics");
 for(const auto& x:{"[00:60.1]Bad\n","[00:01.]Bad\n","[00:01.ab]Bad\n",
                    "[13:00.001]Beyond\n","[00:01.2]Hi\n[00:03]Bad\tTab\n",
                    "[ti:Metadata only]","[offset:90000]\n[00:01]X\n"})
  need(rejects(x),"invalid LRC rejected");
 need(rejects(std::string("[00:01]X\n")+std::string(1048576,'x')),"oversized input");
 need(rejects("[00:01]"+std::string(2001,'a')),"long lyric rejected");
 need(rejects("[00:01]\xC0\xAF\n"),"overlong UTF-8 rejected");
 need(rejects("[00:01]\xED\xA0\x80\n"),"UTF-8 surrogate rejected");
 need(rejects("[00:01]A\0B\n"s),"embedded NUL rejected");
 std::string many;
 for(int i=0;i<501;++i)many+="[00:01]Line\n";
 need(rejects(many),"native lyric count bounded");
 bool unsafe=false;
 try{(void)roomtone::parseLrc("[00:01]Lyrics\n","../music.wav","Default");}
 catch(const std::exception&){unsafe=true;}
 need(unsafe,"audio path identity refused");
 std::cout<<"PASS native direct LRC UTF-8, offsets, multi-timestamps, source lyric lookup, bounds and invalid data.\n";
}
