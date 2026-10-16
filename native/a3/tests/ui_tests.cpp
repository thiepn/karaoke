#include "../src/desktop_ui.hpp"
#include <iostream>
#include <stdexcept>
int main(){
#ifdef _WIN32
 try{
  roomtone::desktop::Window window;
  unsigned snapshots=0,playEvents=0;
  window.syntheticTest=true;
  window.controller.snapshot=[&]{
   ++snapshots;
   roomtone::desktop::Snapshot state;
   state.title=L"TEST -- OFFLINE";
   state.current=L"First line";
   state.next=L"Second line";
   state.hasSong=true;state.deviceActive=true;
   return state;
  };
  window.controller.action=[&](roomtone::desktop::Action action){
   if(action==roomtone::desktop::Play)++playEvents;
  };
  const int result=window.run();
  if(result!=0||snapshots==0||playEvents!=1)
   throw std::runtime_error("in-process control message / UI lifetime");
  if(roomtone::desktop::wide("Nacht \xC3\xBCber")!=L"Nacht \xFC""ber")
   throw std::runtime_error("UTF-8 lyric display decoding");
  std::cout<<"PASS Windows synthetic native lyric surface, button event, UTF-8 and close lifecycle. No WASAPI device opened.\n";
  return 0;
 }catch(const std::exception& e){
  std::cerr<<"Native UI smoke FAILED: "<<e.what()<<"\n";
  return 1;
 }
#else
 return 0;
#endif
}
