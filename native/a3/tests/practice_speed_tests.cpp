#include "../src/karaoke_mix.hpp"
#include "../src/practice_speed.hpp"
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

static void need(bool ok,const char* what){if(!ok)throw std::runtime_error(what);}
int main(){
 using roomtone::KaraokeMix;
 namespace speed=roomtone::practice_speed;
 need(speed::safe(NAN)==1&&speed::safe(INFINITY)==1&&speed::safe(-INFINITY)==1,
      "invalid rate resets to original 1x");
 need(speed::safe(-10)==speed::Minimum&&speed::safe(10)==speed::Maximum,
      "bounded rate rejects out of-range effective transport");
 need(speed::step(1,-1)==.9f&&speed::step(1,1)==1.1f,"owner +/-0.1 speed steps");
 need(speed::step(.6f,-1)==.6f&&speed::step(1.2f,1)==1.2f,"UI rate bounds");
 need(speed::standard(1)&&!speed::standard(.8f),"normal mode indicated");
 std::vector<float> pcm(8000*2);
 for(std::size_t i=0;i<8000;++i){
   pcm[2*i]=static_cast<float>(i)/16000.0f;
   pcm[2*i+1]=-static_cast<float>(i)/16000.0f;
 }
 KaraokeMix mix(std::move(pcm),0,0);
 need(mix.hasSong()&&mix.songFrames()==8000,"original decoded song sample ownership");
 mix.seekFrames(2000);
 for(int i=0;i<10;++i)mix.render(0,false,true,0.5f,1.0f);
 need(mix.position()==2010,"original 1x frame count unchanged");
 mix.seekFrames(2000);
 for(int i=0;i<10;++i)mix.render(0,false,true,0.5f,.6f);
 need(mix.position()==2006,"slow rehearsal advances six original frames per ten output frames");
 mix.seekFrames(2000);
 for(int i=0;i<10;++i)mix.render(0,false,true,0.5f,1.2f);
 need(mix.position()==2012,"fast rehearsal advances twelve original frames per ten output frames");
 mix.seekFrames(2000);
 for(int i=0;i<2000;++i)mix.render(0,false,true,0.5f,1.0f);
 const auto before=mix.position();
 auto sample=mix.render(0,false,true,0.5f,.6f);
 need(std::isfinite(sample.left)&&std::isfinite(sample.right)&&
      std::abs(sample.left+sample.right)<0.0001f,
      "linear-interpolated stereo remains symmetric and finite");
 need(mix.position()==before,"fractional frame progresses without rounding forward");
 mix.render(0,false,true,0.5f,.6f);
 need(mix.position()==before+1,"fractional source progress eventually crosses next frame");
 const auto paused=mix.position();
 for(int i=0;i<1000;++i)mix.render(0,false,false,0.5f,.6f);
 need(mix.position()<=paused+960,"muted/paused song stops after existing finite fade");
 mix.seekFrames(mix.songFrames()-2);
 for(int i=0;i<10;++i)mix.render(0,false,true,0.5f,1.2f);
 need(mix.songEnded()&&mix.position()==mix.songFrames(),"fast rate cannot read after EOF");
 mix.seekFrames(mix.songFrames()-1);
 for(int i=0;i<10;++i)mix.render(0,false,true,0.5f,.6f);
 need(mix.songEnded(),"slow rate handles last sample via final-frame interpolation");
 mix.rewind();need(mix.position()==0&&!mix.songEnded(),"original restart works at any tempo");
 mix.replaceSong(std::vector<float>(100*2,0.2f));
 for(int i=0;i<200;++i)mix.render(0,false,true,0.4f,NAN);
 need(mix.songEnded(),"invalid callback rate uses finite original 1x fallback");
 // Fractional backing speed never resamples or retimes the microphone path.
 std::vector<float> drySource(4000*2,0.0f);
 KaraokeMix slowVoice(drySource,0,0.2f),fastVoice(drySource,0,0.2f);
 for(int i=0;i<1300;++i){
  const auto slow=slowVoice.render(.5f,true,false,0.0f,.6f);
  const auto fast=fastVoice.render(.5f,true,false,0.0f,1.2f);
  need(std::abs(slow.left-fast.left)<0.0000001f&&
       std::abs(slow.right-fast.right)<0.0000001f,
       "backing speed must not alter original 48kHz live vocal DSP");
 }
 mix.replaceSong({});
 need(!mix.hasSong()&&mix.position()==0,"source replacement clears transport");
 std::cout<<"PASS original WASAPI backing PCM-rate resampling, stereo interpolation, exact 1x, speed bounds, seek/EOF/replacement, paused fade; NOT pitch preserving.\n";
}
