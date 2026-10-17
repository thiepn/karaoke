#include "../src/signal_diagnostics.hpp"
#include <stdexcept>
#include <iostream>
#include <limits>
static void need(bool v,const char* msg){if(!v)throw std::runtime_error(msg);}
int main(){
 using namespace roomtone::signal_diagnostics;
 need(sample(0.0f)==0&&sample(-0.5f)==0.5f&&sample(5.0f)==1,
      "finite amplitudes bounded for display");
 need(sample(std::numeric_limits<float>::quiet_NaN())==0,"nan never published");
 need(sample(-std::numeric_limits<float>::infinity())==0,"inf never published");
 need(percentage(-0.245f)==25&&percentage(1.2f)==100,"amplitude display rounded");
 need(!inputNearClipping(0.97f)&&inputNearClipping(-0.99f),
      "input clipping warning is original amplitude observation");
 need(!outputAtLimiter(0.68f)&&outputAtLimiter(-0.7f),
      "0.7 original native output limiter observation");
 Levels meter;
 need(meter.capture()==0&&meter.output()==0,"no invented idle activity");
 meter.publish(0.63f,-0.32f);
 need(meter.capture()==0.63f&&meter.output()==0.32f,
      "callback peaks visible without recording or microphone monitor");
 meter.publish(std::numeric_limits<float>::quiet_NaN(),1.5f);
 need(meter.capture()==0&&meter.output()==1.0f,
      "invalid samples fail closed and output normalized for display");
 meter.clear();
 need(meter.capture()==0&&meter.output()==0,
      "interrupted/stopped driver never exposes previous signal as live");
 std::cout<<"PASS realtime-safe original callback peak observations, finite bounds, near-clipping/limiter status and offline zero. No WASAPI devices opened.\n";
}
