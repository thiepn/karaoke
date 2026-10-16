#include "../src/desktop_ui.hpp"
#include <iostream>
#include <stdexcept>
int main(){
#ifdef _WIN32
 try{
  roomtone::desktop::Window window;
  unsigned snapshots=0,playEvents=0,muteEvents=0,seekEvents=0,lastSeek=0;
  window.syntheticTest=true;
  window.controller.snapshot=[&]{
   ++snapshots;
   roomtone::desktop::Snapshot state;
   state.title=L"TEST -- OFFLINE";
   state.current=L"First line";
   state.next=L"Second line";
   state.hasSong=true;state.deviceActive=true;state.durationSeconds=120;
   return state;
  };
  window.controller.action=[&](roomtone::desktop::Action action){
   if(action==roomtone::desktop::Play)++playEvents;
   if(action==roomtone::desktop::EmergencyMute)++muteEvents;
  };
  window.controller.seekSeconds=[&](unsigned sec){++seekEvents;lastSeek=sec;};
  // The nested modal chooser must NEVER post WM_QUIT for the entire app.
  {
   roomtone::desktop::DeviceChooser picker;
   const std::vector<std::wstring> input{L"Test input"},output{L"Test output"};
   picker.inputs=&input;picker.outputs=&output;picker.syntheticCancel=true;
   if(picker.run().has_value())throw std::runtime_error("Synthetic chooser cancellation accepted");
   MSG pending{};
   if(PeekMessageW(&pending,nullptr,WM_QUIT,WM_QUIT,PM_NOREMOVE))
    throw std::runtime_error("Modal chooser incorrectly quit app's message loop");
  }
  const int result=window.run();
  if(result!=0||snapshots==0||playEvents!=1||muteEvents!=1||seekEvents!=1||lastSeek!=43)
   throw std::runtime_error("in-process control/seek message, modal lifetime and focus");
  if(roomtone::desktop::wide("Nacht \xC3\xBC""ber")!=L"Nacht \xFC""ber")
   throw std::runtime_error("UTF-8 lyric display decoding");
  std::cout<<"PASS Windows synthetic native lyric surface, accessible seek, modal cancel, action events, UTF-8 and close lifecycle. No WASAPI device opened.\n";
  return 0;
 }catch(const std::exception& e){
  std::cerr<<"Native UI smoke FAILED: "<<e.what()<<"\n";
  return 1;
 }
#else
 return 0;
#endif
}
