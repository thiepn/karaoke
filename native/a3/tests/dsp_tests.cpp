#include "../src/karaoke_mix.hpp"
#include "../src/transport.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>
static void require(bool b,const char* msg){if(!b)throw std::runtime_error(msg);}
int main(){
 using roomtone::VocalDsp;
 VocalDsp dry(0,0.2f);
 auto x=dry.process(0.5f);
 require(x.left>0.09f&&std::abs(x.left-x.right)<1e-7f,"dry sample must be immediate");
 VocalDsp wet(0.25f,0.2f);double tail=0;
 for(int i=0;i<8000;++i){
  auto s=wet.process(i==0?0.8f:0.0f);
  require(std::isfinite(s.left)&&std::isfinite(s.right),"wet finite");
  require(std::abs(s.left)<=0.35001f&&std::abs(s.right)<=0.35001f,"wet bound");
  if(i>1400)tail+=std::abs(s.left-s.right);
 }
 require(tail>0.00001,"reverb tail");
 VocalDsp echo(0,0.35f);echo.setParameters(0,0.35f,0.35f,0.3f,80);
 double repeats=0;
 for(int i=0;i<8400;++i){
  auto s=echo.process(i==0?1.0f:0.0f);
  if(i>3800)repeats+=std::abs(s.left);
  require(std::isfinite(s.left)&&std::abs(s.left)<=0.35001f,"echo finite/bounded");
 }
 require(repeats>0.001,"echo repeats");
 VocalDsp safety(0.25f,0.35f);
 for(float v:{NAN,INFINITY,-INFINITY,100.f,-100.f}){
  auto s=safety.process(v);
  require(std::isfinite(s.left)&&std::abs(s.left)<=0.35001f,"invalid input");
 }
 safety.reset();require(safety.process(0).left==0,"reset");
 std::vector<float> song(9600);
 for(std::size_t i=0;i<song.size()/2;++i){song[2*i]=0.4f;song[2*i+1]=-0.2f;}
 roomtone::KaraokeMix mixer(std::move(song),0,0.2f);
 require(mixer.hasSong()&&mixer.songFrames()==4800,"song length");
 auto silent=mixer.render(0.5f,false,false,0.4f);
 require(silent.left==0&&silent.right==0&&mixer.position()==0,"start muted");
 auto mixed=mixer.render(0.4f,true,true,0.4f);
 for(int i=0;i<1200;++i)mixed=mixer.render(0.4f,true,true,0.4f);
 require(mixed.left>0.1f&&mixed.right<0.1f,"stereo live mix");
 const auto before=mixer.position();
 for(int i=0;i<1200;++i)mixer.render(0,false,false,0.4f);
 require(mixer.position()<before+1200,"pause fails");
 mixer.rewind();require(mixer.position()==0,"rewind fails");
 for(int i=0;i<5500;++i)mixer.render(0,false,true,0.4f);
 require(mixer.songEnded(),"song end not detected");
 for(int i=0;i<4000;++i){
  auto s=mixer.render(2,true,false,10);
  require(std::isfinite(s.left)&&std::abs(s.left)<=0.700001f,"mixer clipping guard");
 }
 // Seeking is clamped; only backing transport moves, vocal processing persists.
 mixer.seekFrames(2400);
 require(mixer.position()==2400&&!mixer.songEnded(),"middle seek");
 mixer.seekFrames(999999);
 require(mixer.position()==mixer.songFrames()&&mixer.songEnded(),"end seek clamp");
 mixer.rewind();
 require(mixer.position()==0&&!mixer.songEnded(),"seek back from EOF");
 auto restart=mixer.render(0,false,true,0.4f);
 require(std::isfinite(restart.left)&&std::isfinite(restart.right),"restart sample");
 require(roomtone::parseSeekFrames("1.5",0,100000)==72000,"absolute seek");
 require(roomtone::parseSeekFrames("+0.25",48000,100000)==60000,"forward relative seek");
 require(roomtone::parseSeekFrames("-3",48000,100000)==0,"backward clamp");
 require(roomtone::parseSeekFrames("99",0,100000)==100000,"EOF clamp");
 for(const char* bad:{"nan","inf","12garbage","", "-infinity"}) {
  bool rejected=false;try{(void)roomtone::parseSeekFrames(bad,0,100000);}
  catch(const std::exception&){rejected=true;}
  require(rejected,"seek invalid input rejected");
 }
 std::vector<float> replacement(2000,0.2f);
 mixer.replaceSong(std::move(replacement));
 require(mixer.songFrames()==1000&&mixer.position()==0,"replace local track");
 for(int i=0;i<1100;++i)mixer.render(0,false,true,0.2f);
 require(mixer.songEnded(),"replacement EOF");
 mixer.replaceSong({});
 require(!mixer.hasSong()&&mixer.position()==0,"clear song");
 roomtone::KaraokeMix empty;
 require(!empty.hasSong()&&empty.render(0,false,true,0).left==0,"empty song");
 std::cout<<"PASS native DSP: dry immediate, bounded reverb and echo, mute, stereo music+voice, pause, rewind, EOF, invalid input, output ceiling\n";
 std::cout<<"NOT_TESTED actual microphone/speaker latency, acoustic feedback and hardware quality\n";
}
