#include "../src/session_export.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
static void require(bool condition,const char* reason){if(!condition)throw std::runtime_error(reason);}
static bool fails(const roomtone::Handoff& h){
 try{(void)roomtone::session_export::encode(h);return false;}
 catch(const std::exception&){return true;}
}
int main(){
 using namespace roomtone;
 Handoff source=parseHandoff(
  "ROOMTONE_HANDOFF_V1\n"
  "audio\tQuelle.wav\n"
  "title\tPr%C3%BCfung%20%26%20Licht\n"
  "start-ms\t1000\n"
  "offset-ms\t-100\n"
  "loop\t1\nloop-start-ms\t1000\nloop-end-ms\t3000\n"
  "lyric\t1000\tZeile%20eins\n"
  "lyric\t2000\t%C3%84pfel%20%26%20Licht\n");
 source.offsetMs=275;
 auto h=session_export::snapshot(source,"Neue Datei.wav","Other",144000,240000,
     true,48000,192000,true);
 const auto serialized=session_export::encode(h);
 const auto parsed=parseHandoff(serialized);
 require(parsed.audioName=="Neue Datei.wav","new explicit source filename");
 require(parsed.title==source.title,"original title and Unicode preserved");
 require(parsed.startMs==3000&&parsed.offsetMs==275,"position and offset");
 require(parsed.loop&&parsed.loopStartMs==1000&&parsed.loopEndMs==4000,"A-B preserved");
 require(parsed.lines.size()==2&&parsed.lines[1].text==source.lines[1].text,
     "source lyric timestamps and multibyte UTF-8");
 require(session_export::encode(parsed)==serialized,"roundtrip format canonical");
 auto noLoop=session_export::snapshot(source,"Neue Datei.wav","Fallback",999999,240000,
      true,0,0,false);
 require(noLoop.startMs==5000&&!noLoop.loop&&noLoop.loopStartMs==0&&
         noLoop.loopEndMs==0,"EOF clamp and whole-track repeat not falsely A-B");
 auto blank=session_export::snapshot(Handoff{},"No lyrics.flac","My instrumental",
                                     0,48000,false,0,0,false);
 require(blank.lines.empty()&&blank.title=="My instrumental","song-only resumable");
 require(fails(Handoff{}),"missing audio filename refused");
 Handoff altered=h;altered.audioName="../secret.wav";
 require(fails(altered),"unsafe audio file path refused");
 altered=h;altered.lines[0].text="\x1b[31munsafe";
 require(fails(altered),"unsafe terminal escape refused");
 altered=h;altered.lines[0].text="\xC0\xAF";
 require(fails(altered),"invalid UTF-8 rejected");
 altered=h;altered.lines[1].milliseconds=10;
 require(fails(altered),"unsorted lyric timestamps rejected");
 altered=h;altered.offsetMs=30001;
 require(fails(altered),"out of range offset rejected");
 altered=h;altered.loopEndMs=1001;
 require(fails(altered),"invalid A-B bounds rejected");
 altered=h;altered.lines.resize(501);
 require(fails(altered),"too many lyric entries rejected");
 require(session_export::percentEncode("A + B ~!")=="A%20%2B%20B%20~%21",
         "URI-compatible percent encoding");
 std::cout<<"PASS local owner-selected practice session serialization, UTF-8, roundtrip, recovery, bounds and privacy.\n";
}
