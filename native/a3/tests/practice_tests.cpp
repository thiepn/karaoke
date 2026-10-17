#include "../src/practice.hpp"
#include <iostream>
#include <stdexcept>
static void need(bool b,const char* label){if(!b)throw std::runtime_error(label);}
int main(){
 using namespace roomtone::practice;
 need(validAB(0,12001,240000),"start at zero permitted");
 need(validAB(48000,48000+12001,240000),"valid loop");
 need(!validAB(48000,60000,240000),"exact 250ms is insufficient");
 need(!validAB(48000,47999,240000),"backward loop refused");
 need(!validAB(48000,48000,240000),"zero-length loop refused");
 need(!validAB(240001,240002,240000),"start outside song refused");
 need(!validAB(0,240001,240000),"end beyond song refused");
 need(markAt(300000,240000)==240000,"offline position clamps at EOF");
 need(adjustOffset(29970,100)==30000,"positive offset clamp");
 need(adjustOffset(-29980,-100)==-30000,"negative offset clamp");
 need(adjustOffset(-150,100)==-50,"negative offset nudge");
 need(adjustOffset(300,-100)==200,"positive offset nudge");
 std::cout<<"PASS source-compatible native A-B selection, quarter-second rule and lyric offset limits.\n";
}