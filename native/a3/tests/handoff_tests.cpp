#include "../src/handoff.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
static void need(bool c,const char *why){if(!c)throw std::runtime_error(why);}
static bool rejects(const std::string& s){
 try{(void)roomtone::parseHandoff(s);return false;}catch(const std::exception&){return true;}
}
int main(){
 const std::string doc=
  "ROOMTONE_HANDOFF_V1\n"
  "audio\tNacht%20und%20Licht.wav\n"
  "title\tNacht%20%26%20Licht\n"
  "start-ms\t12500\n"
  "offset-ms\t-200\n"
  "loop\t1\n"
  "loop-start-ms\t10000\n"
  "loop-end-ms\t22000\n"
  "lyric\t10000\tFirst%20verse\n"
  "lyric\t13000\tSecond%20verse\n";
 const auto h=roomtone::parseHandoff(doc);
 need(h.audioName=="Nacht und Licht.wav"&&h.title=="Nacht & Licht","metadata decoding");
 need(h.startMs==12500&&h.loop&&h.loopStartMs==10000&&h.loopEndMs==22000,"timeline");
 need(roomtone::lyricIndexAt(h,9000)==h.lines.size(),"before first");
 need(roomtone::lyricIndexAt(h,9700)==h.lines.size(),"negative offset before first");
 need(roomtone::lyricIndexAt(h,9800)==0,"offset-adjusted line");
 need(roomtone::lyricIndexAt(h,12800)==1,"second line");
 need(rejects("invalid\n"+doc),"magic required");
 need(rejects(doc+"evil\t123\n"),"unknown key");
 need(rejects(doc+"start-ms\t1\n"),"duplicate key");
 need(rejects(std::string(doc).replace(doc.find("Nacht%20und%20Licht.wav"),21,"..%2Fpasswd")),"path traversal");
 need(rejects(std::string(doc).replace(doc.find("First%20verse"),13,"%1B[0m")),"terminal escape");
 need(rejects(std::string(doc).replace(doc.find("lyric\t13000"),11,"lyric\t9000")),"unsorted lyrics");
 need(rejects(std::string(doc).replace(doc.find("12500"),5,"1.5")),"fractional milliseconds");
 need(rejects(std::string(doc).replace(doc.find("loop-end-ms\t22000"),18,"loop-end-ms\t10000")),"bad loop");
 need(rejects(doc+std::string(1048576,'A')),"oversized input");
 std::cout<<"PASS local user-mediated session handoff metadata, lyric offset/timing, validation and traversal/terminal escape protections\n";
}
