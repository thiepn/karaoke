#include "../src/desktop_ui.hpp"
#include "../src/session_io_win.hpp"
#include <fstream>
#include <iterator>
#include <iostream>
#include <stdexcept>
int main(){
#ifdef _WIN32
 try{
  roomtone::desktop::Window window;
  unsigned snapshots=0,playEvents=0,muteEvents=0,seekEvents=0,lastSeek=0,topologyEvents=0,tickEvents=0,previousEvents=0,browseEvents=0,modalMuteEvents=0,deviceModalMuteEvents=0,queueModalMuteEvents=0,loopEvents=0,offsetEvents=0,saveEvents=0,queueActionEvents=0,manageQueueEvents=0;
  window.syntheticTest=true;
  bool compactLayout=false;
  window.controller.snapshot=[&]{
   if(window.hwnd&&window.notice){
    RECT bounds{},last{};GetClientRect(window.hwnd,&bounds);
    GetWindowRect(window.notice,&last);
    POINT endpoint{last.right,last.bottom};
    ScreenToClient(window.hwnd,&endpoint);
    compactLayout=endpoint.x<=bounds.right&&endpoint.y<=bounds.bottom;
   }
   ++snapshots;
   roomtone::desktop::Snapshot state;
   state.title=L"TEST -- OFFLINE";
   state.current=L"First line";
   state.next=L"Second line";
   state.hasSong=true;state.hasLyrics=true;state.deviceActive=true;state.durationSeconds=120;
   return state;
  };
  window.controller.action=[&](roomtone::desktop::Action action){
   if(action==roomtone::desktop::Play)++playEvents;
   if(action==roomtone::desktop::EmergencyMute)++muteEvents;
   if(action==roomtone::desktop::PreviousLine)++previousEvents;
   if(action==roomtone::desktop::BrowseLyrics)++browseEvents;
   if(action==roomtone::desktop::MarkLoopA)++loopEvents;
   if(action==roomtone::desktop::OffsetLater)++offsetEvents;
   if(action==roomtone::desktop::SaveSession)++saveEvents;
   if(action==roomtone::desktop::AddQueueSong||action==roomtone::desktop::ShowQueue||
      action==roomtone::desktop::NextQueueSong||action==roomtone::desktop::PreviousQueueSong)++queueActionEvents;
   if(action==roomtone::desktop::RemoveQueuedSong||action==roomtone::desktop::MoveQueuedUp||
      action==roomtone::desktop::MoveQueuedDown||action==roomtone::desktop::ClearPendingQueue)++manageQueueEvents;
  };
  window.controller.seekSeconds=[&](unsigned sec){++seekEvents;lastSeek=sec;};
  window.controller.deviceChanged=[&]{++topologyEvents;};
  window.controller.maintenance=[&]{++tickEvents;};
  // The nested modal chooser must NEVER post WM_QUIT for the entire app.
  {
   roomtone::desktop::DeviceChooser picker;
   const std::vector<std::wstring> input{L"Test input"},output{L"Test output"};
   picker.inputs=&input;picker.outputs=&output;
   picker.initiallySelectedInput=0;picker.initiallySelectedOutput=-2;
   picker.syntheticMute=true;picker.syntheticCancel=true;
   picker.emergencyMute=[&]{++deviceModalMuteEvents;};
   if(picker.run().has_value())throw std::runtime_error("Synthetic chooser cancellation accepted");
   MSG pending{};
   if(PeekMessageW(&pending,nullptr,WM_QUIT,WM_QUIT,PM_NOREMOVE))
    throw std::runtime_error("Modal chooser incorrectly quit app's message loop");
  }
  // Lyric chooser cancellation never transfers transport and never quits app.
  {
   roomtone::desktop::LyricChooser chooser;
   std::vector<std::wstring> rows{L"0:10 Verse one",L"0:24 Chorus"};
   chooser.rows=&rows;chooser.current=1;chooser.syntheticMute=true;chooser.syntheticCancel=true;
   chooser.emergencyMute=[&]{++modalMuteEvents;};
   if(chooser.run().has_value())throw std::runtime_error("Cancelled lyric jump was accepted");
   MSG pending{};
   if(PeekMessageW(&pending,nullptr,WM_QUIT,WM_QUIT,PM_NOREMOVE))
    throw std::runtime_error("Lyric chooser incorrectly posted process-wide quit");
  }
  // No audio hardware: exercise actual Windows CREATE_NEW semantics in an
  // isolated OS temporary directory. Never overwrite an unrelated file.
  {
   wchar_t buffer[MAX_PATH]={};
   const DWORD length=GetTempPathW(MAX_PATH,buffer);
   if(!length||length>=MAX_PATH)throw std::runtime_error("Windows temp path unavailable");
   const auto file=std::filesystem::path(buffer)/
    (L"roomtone-a3-io-test-"+std::to_wstring(GetCurrentProcessId())+L".rtk");
   if(std::filesystem::exists(file))
    throw std::runtime_error("Refusing to overwrite existing test fixture");
   roomtone::Handoff session;
   session.audioName="original.wav";session.title="Source owned";
   session.startMs=1450;
   try{
    roomtone::local_file::saveNew(file,session);
    std::ifstream input(file,std::ios::binary);
    const std::string data((std::istreambuf_iterator<char>(input)),std::istreambuf_iterator<char>());
    auto decoded=roomtone::parseHandoff(data);
    if(decoded.audioName!=session.audioName||decoded.startMs!=1450)
     throw std::runtime_error("Saved local session did not roundtrip");
    bool refused=false;
    try{roomtone::local_file::saveNew(file,session);}
    catch(const std::exception&){refused=true;}
    if(!refused)throw std::runtime_error("Existing .rtk overwritten");
    std::ifstream reread(file,std::ios::binary);
    const std::string retained((std::istreambuf_iterator<char>(reread)),std::istreambuf_iterator<char>());
    if(retained!=data)throw std::runtime_error("Previously saved bytes changed on collision");
   }catch(...){DeleteFileW(file.c_str());throw;}
   if(!DeleteFileW(file.c_str()))throw std::runtime_error("Failed to remove temp session");
  }
  // Owner-selected queue reuses native focused list and never starts audio.
  {
   roomtone::desktop::LyricChooser picker;
   std::vector<std::wstring> localSongs{L"1. Original",L"2. Second"};
   picker.rows=&localSongs;picker.current=0;
   picker.dialogTitle=L"ROOMTONE | Local song queue";
   picker.hintText=L"Choose an owner-selected local song. Enter selects; Esc cancels.";
   picker.confirmText=L"Select song";
   picker.syntheticMute=true;picker.syntheticCancel=true;
   picker.emergencyMute=[&]{++queueModalMuteEvents;};
   if(picker.run().has_value())throw std::runtime_error("Queue cancel changed current song");
  }
  const int result=window.run();
  if(result!=0||snapshots==0||playEvents!=1||muteEvents!=1||previousEvents!=1||browseEvents!=1||loopEvents!=1||offsetEvents!=1||saveEvents!=1||queueActionEvents!=4||manageQueueEvents!=4||deviceModalMuteEvents!=1||queueModalMuteEvents!=1||modalMuteEvents!=1||!compactLayout||topologyEvents<1||tickEvents==0||seekEvents!=1||lastSeek!=43)
   throw std::runtime_error("in-process control/seek message, modal lifetime and focus");
  if(roomtone::desktop::wide("Nacht \xC3\xBC""ber")!=L"Nacht \xFC""ber")
   throw std::runtime_error("UTF-8 lyric display decoding");
  std::cout<<"PASS Windows synthetic native lyric/queue UI, source-management actions, focused F8, accessibility and modal cancellation; WASAPI not opened.\n";
  return 0;
 }catch(const std::exception& e){
  std::cerr<<"Native UI smoke FAILED: "<<e.what()<<"\n";
  return 1;
 }
#else
 return 0;
#endif
}
