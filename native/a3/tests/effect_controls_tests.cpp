#include "../src/effect_controls.hpp"
#include <stdexcept>
#include <iostream>
static void need(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
int main(){
 using namespace roomtone::effects;
 need(nudge(0.15f,2,0.35f)==0.17f,"2% feedback up");
 need(nudge(0.0f,-2,0.35f)==0,"feedback lower bound");
 need(nudge(0.35f,2,0.35f)==0.35f,"feedback upper bound");
 need(nudge(0.01f,-2,0.35f)==0,"feedback clamp");
 need(nudgeDelay(280,-20)==260,"shorter echo delay");
 need(nudgeDelay(80,-20)==80,"delay lower bound");
 need(nudgeDelay(550,20)==550,"delay upper bound");
 Settings current{0.23f,0.32f,0.25f,0.30f,0.48f,500};
 auto dry=dryVoice(current);
 need(dry.wet==0&&dry.echo==0&&dry.feedback==0,"dry effect components off");
 need(dry.gain==current.gain&&dry.music==current.music&&dry.delayMs==500,
   "dry voice must not amplify or interrupt existing music level");
 Settings initial{0.14f,0.20f,0.10f,0.08f,0.25f,320};
 auto original=clamp(initial);
 need(original.wet==0.14f&&original.delayMs==320&&original.music==0.25f,
      "exact original argument-defined initial settings preserved");
 Settings unsafe{1,1,1,1,1,900};
 auto bounded=clamp(unsafe);
 need(bounded.wet==0.25f&&bounded.gain==0.35f&&bounded.echo==0.35f&&
      bounded.feedback==0.35f&&bounded.music==0.50f&&bounded.delayMs==550,
      "existing source DSP limits reused");
 std::cout<<"PASS original native VocalDsp feedback/delay, dry comparison, initial-setting reset and bounded levels.\n";
}
