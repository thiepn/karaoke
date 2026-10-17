#pragma once
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <commctrl.h>
#include <dbt.h>
#include <algorithm>
#include <functional>
#include <string>
#include <vector>
#include <utility>
#include <optional>
#include <stdexcept>
#include <cstddef>
#include <cstdint>

namespace roomtone::desktop {
// All Win32 window/UI callbacks run on the application's main/control thread.
// The existing WASAPI callback remains the only realtime audio engine.
inline std::wstring wide(const std::string& value) {
 if(value.empty())return {};
 const int len=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,value.data(),
                                  static_cast<int>(value.size()),nullptr,0);
 if(len<=0)return L"[text encoding unavailable]";
 std::wstring text(static_cast<std::size_t>(len),L'\0');
 MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,value.data(),
                     static_cast<int>(value.size()),text.data(),len);
 return text;
}
enum Action {
 Play=101,Mic,Back,Forward,Restart,Repeat,NextLine,
 ReverbDown,ReverbUp,EchoDown,EchoUp,MusicDown,MusicUp,
 ChooseSong,ChooseSession,ChooseDevices,Reconnect,EmergencyMute,GainDown,GainUp,
  PreviousLine,BrowseLyrics,MarkLoopA,MarkLoopB,ClearAB,ToggleAB,OffsetEarlier,OffsetLater,OffsetReset,SaveSession,AddQueueSong,ShowQueue,NextQueueSong,PreviousQueueSong,
  RemoveQueuedSong,MoveQueuedUp,MoveQueuedDown,ClearPendingQueue,
  FeedbackDown,FeedbackUp,DelayShorter,DelayLonger,DryVoice,RestoreEffects,ShowEffects,ShowRecovery,
  ChooseLyricSection,SeekLoopA,SeekLoopB
};
struct Snapshot {
 std::wstring title,previous,current,next,input,output,notice;
 std::uint64_t loopA=0,loopB=0;
 int lyricOffsetMs=0;
 bool loopMarked=false,loopEnabled=false;
 unsigned positionSeconds=0,durationSeconds=0;
 std::size_t queueIndex=0,queueTotal=0;
 float reverb=0,echo=0,music=0,gain=0,feedback=0;
 unsigned delayMs=280;
 bool playing=false,microphone=false,repeat=false,interrupted=false,deviceActive=false,hasSong=false,hasLyrics=false;
};
struct Controller {
 std::function<Snapshot()> snapshot;
 std::function<void(Action)> action;
 std::function<void(unsigned)> seekSeconds; // only on owner UI thread
  std::function<void()> maintenance; // timer drains driver events on owner thread
  std::function<void()> deviceChanged; // Windows device notification signals topology change
};
inline HFONT font(int size,bool bold=false) {
 return CreateFontW(-size,0,0,0,bold?FW_SEMIBOLD:FW_NORMAL,FALSE,FALSE,FALSE,
                    DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
                    CLEARTYPE_QUALITY,DEFAULT_PITCH|FF_DONTCARE,L"Segoe UI");
}
// Update only on state change: periodic lyric refreshes must not flood native
// accessibility clients with duplicate text-change notifications.
inline void setText(HWND handle,const std::wstring& value){
 if(!handle)return;
 const int length=GetWindowTextLengthW(handle);
 std::wstring previous(static_cast<std::size_t>(std::max(0,length))+1,L'\0');
 const int read=GetWindowTextW(handle,previous.data(),static_cast<int>(previous.size()));
 previous.resize(static_cast<std::size_t>(std::max(0,read)));
 if(previous!=value)SetWindowTextW(handle,value.c_str());
}
inline std::wstring clock(unsigned seconds) {
 return std::to_wstring(seconds/60)+L":"+(seconds%60<10?L"0":L"")+std::to_wstring(seconds%60);
}
struct Window {
 Controller controller;
 HWND hwnd=nullptr,title=nullptr,previous=nullptr,current=nullptr,next=nullptr,
      status=nullptr,device=nullptr,notice=nullptr,seekLabel=nullptr,seekBar=nullptr;
 HWND buttons[22]{};
 HFONT minorFont=nullptr,regular=nullptr,large=nullptr,heading=nullptr;
 bool ready=false;
 bool syntheticTest=false;
 bool trackingSeek=false;
 void applyFont(HWND h,HFONT f){if(h)SendMessageW(h,WM_SETFONT,reinterpret_cast<WPARAM>(f),TRUE);}
 HWND label(const wchar_t* value,DWORD flags,HFONT f) {
  auto h=CreateWindowExW(0,L"STATIC",value,WS_CHILD|WS_VISIBLE|flags,0,0,100,25,
                         hwnd,nullptr,GetModuleHandleW(nullptr),nullptr);
  applyFont(h,f);return h;
 }
 HWND button(Action id,const wchar_t* text){
  auto h=CreateWindowExW(0,L"BUTTON",text,WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_PUSHBUTTON,
                 0,0,100,40,hwnd,reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
                 GetModuleHandleW(nullptr),nullptr);
  applyFont(h,regular);return h;
 }
 // Compact native layout remains usable on 1366x768 Windows desktops.
 // Taller windows grow the primary lyric region; all transport controls stay visible.
 void layout(){
  if(!hwnd||!title)return;
  RECT bounds{};GetClientRect(hwnd,&bounds);
  const int w=std::max(570L,bounds.right-bounds.left);
  const int height=std::max(640L,bounds.bottom-bounds.top);
  const int lyricExtra=std::clamp(height-650,0,140);
  const int pad=20,gap=8;
  MoveWindow(title,pad,8,w-2*pad,27,TRUE);
  MoveWindow(previous,pad,40,w-2*pad,22,TRUE);
  MoveWindow(current,pad,66,w-2*pad,58+lyricExtra,TRUE);
  MoveWindow(next,pad,125+lyricExtra,w-2*pad,24,TRUE);
  MoveWindow(seekLabel,pad,151+lyricExtra,w-2*pad,18,TRUE);
  MoveWindow(seekBar,pad,172+lyricExtra,w-2*pad,26,TRUE);
  MoveWindow(status,pad,201+lyricExtra,w-2*pad,30,TRUE);
  const int bw=(w-2*pad-gap*3)/4;
  for(int i=0;i<16;++i){
   const int x=pad+(i%4)*(bw+gap),y=237+lyricExtra+(i/4)*39;
   MoveWindow(buttons[i],x,y,bw,34,TRUE);
  }
  const int half=(w-2*pad-gap)/2;
  MoveWindow(buttons[16],pad,395+lyricExtra,half,32,TRUE);
  MoveWindow(buttons[20],pad+half+gap,395+lyricExtra,half,32,TRUE);
  MoveWindow(buttons[21],pad,434+lyricExtra,w-2*pad,32,TRUE);
  MoveWindow(buttons[17],pad,473+lyricExtra,w-2*pad,34,TRUE);
  MoveWindow(buttons[18],pad,513+lyricExtra,half,31,TRUE);
  MoveWindow(buttons[19],pad+half+gap,513+lyricExtra,half,31,TRUE);
  MoveWindow(device,pad,551+lyricExtra,w-2*pad,30,TRUE);
  MoveWindow(notice,pad,585+lyricExtra,w-2*pad,56,TRUE);
 }
 void render() {
  if(!controller.snapshot)return;
  const auto s=controller.snapshot();
  setText(title,s.title.empty()?L"ROOMTONE / NATIVE KARAOKE":s.title);
  std::wstring practiceNote=L"Position (slider arrows / Home / End) · Practice menu";
  if(s.loopMarked){
   practiceNote=L"A "+clock(static_cast<unsigned>(s.loopA/48000))+
      L" · B "+clock(static_cast<unsigned>(s.loopB/48000))+
      (s.loopEnabled?L" · A-B LOOP ON":L" · A-B LOOP OFF");
  }
  if(s.hasLyrics)practiceNote+=L" · offset "+
      std::to_wstring(s.lyricOffsetMs)+L"ms";
  setText(seekLabel,practiceNote);
  setText(previous,s.previous.empty()?L" ":s.previous);
  setText(current,s.current.empty()?L"No synchronized lyrics loaded":s.current);
  setText(next,s.next.empty()?L" ":s.next);
  if(seekBar){
   const int limit=static_cast<int>(std::min<unsigned>(s.durationSeconds,720));
   SendMessageW(seekBar,TBM_SETRANGEMAX,FALSE,limit);
   if(!trackingSeek)
    SendMessageW(seekBar,TBM_SETPOS,TRUE,std::min<unsigned>(s.positionSeconds,limit));
   EnableWindow(seekBar,s.hasSong); // owner queues offline seeks across reconnect
  }
  auto state=s.interrupted?L"INTERRUPTED":s.deviceActive?L"READY":L"DEVICE STOPPED";
  setText(status,clock(s.positionSeconds)+L" / "+clock(s.durationSeconds)
      +L"  |  "+state+L"  |  Voice "+(s.microphone?L"ON":L"MUTED")
      +L"  |  Music "+(s.playing?L"PLAYING":L"PAUSED")+
       (s.queueTotal?L"  |  Song "+std::to_wstring(s.queueIndex+1)+L"/"+std::to_wstring(s.queueTotal):L""));
  setText(buttons[0],s.playing?L"Pause music":L"Play music");
  setText(buttons[1],s.microphone?L"MUTE VOICE (F9)":L"Enable voice (F9)");
  setText(buttons[5],s.repeat?L"Repeat ON":L"Repeat OFF");
  setText(buttons[7],L"Reverb - ("+std::to_wstring(static_cast<int>(s.reverb*100))+L"%)");
  setText(buttons[8],L"Reverb +");
  setText(buttons[9],L"Echo - ("+std::to_wstring(static_cast<int>(s.echo*100))+L"%)");
  setText(buttons[10],L"Echo +");
  setText(buttons[11],L"Music - ("+std::to_wstring(static_cast<int>(s.music*100))+L"%)");
  setText(buttons[12],L"Music +");
  setText(buttons[18],L"Vocal gain - ("+std::to_wstring(static_cast<int>(s.gain*100))+L"%)");
  setText(buttons[19],L"Vocal gain +");
  setText(device,L"Input: "+s.input+L"  |  Output: "+s.output);
  setText(notice,s.notice.empty()?L"Local WASAPI | No network | Buffer size is NOT measured mic-to-speaker latency":s.notice);
  EnableWindow(buttons[0],s.hasSong&&s.deviceActive&&!s.interrupted);
  EnableWindow(buttons[1],s.deviceActive&&!s.interrupted);
  for(int i:{2,3,4,5})EnableWindow(buttons[i],s.hasSong);
   EnableWindow(buttons[6],s.hasSong&&s.hasLyrics);
   EnableWindow(buttons[20],s.hasSong&&s.hasLyrics);
   EnableWindow(buttons[21],s.hasSong&&s.hasLyrics);
 }
 void notify(Action action){
  if(controller.action)controller.action(action);
  render();
 }
 static LRESULT CALLBACK proc(HWND h,UINT message,WPARAM w,LPARAM l){
  auto* self=reinterpret_cast<Window*>(GetWindowLongPtrW(h,GWLP_USERDATA));
  if(message==WM_NCCREATE){
   auto* created=reinterpret_cast<CREATESTRUCTW*>(l);
   self=static_cast<Window*>(created->lpCreateParams);
   SetWindowLongPtrW(h,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(self));
   self->hwnd=h;
   return TRUE;
  }
  if(!self)return DefWindowProcW(h,message,w,l);
  switch(message){
   case WM_CREATE:{
    self->minorFont=font(17);self->regular=font(17);self->large=font(30,true);
    self->heading=font(25,true);
   HMENU menubar=CreateMenu(),practiceMenu=CreatePopupMenu(),songMenu=CreatePopupMenu(),effectsMenu=CreatePopupMenu(),recoveryMenu=CreatePopupMenu();
   if(menubar&&practiceMenu){
    const struct Entry {Action action;const wchar_t* title;} entries[]={
     {MarkLoopA,L"Mark loop start A\tCtrl+1"},
     {MarkLoopB,L"Mark loop end B\tCtrl+2"},
     {ClearAB,L"Clear A-B loop\tCtrl+3"},
     {ToggleAB,L"Toggle A-B loop\tCtrl+4"},
     {OffsetEarlier,L"Lyrics 100 ms earlier\tCtrl+["},
     {OffsetLater,L"Lyrics 100 ms later\tCtrl+]"},
     {OffsetReset,L"Reset lyric offset\tCtrl+0"},
     {SaveSession,L"Save local practice session\tCtrl+S"},
     {ChooseLyricSection,L"Select A-B from authored lyric lines\tCtrl+7"},
     {SeekLoopA,L"Pause and seek to marker A\tCtrl+8"},
     {SeekLoopB,L"Pause and seek to marker B\tCtrl+9"}};
    for(const auto& entry:entries)AppendMenuW(practiceMenu,MF_STRING,
       static_cast<UINT_PTR>(entry.action),entry.title);
    AppendMenuW(menubar,MF_POPUP,reinterpret_cast<UINT_PTR>(practiceMenu),L"&Practice");
    if(songMenu){
     AppendMenuW(songMenu,MF_STRING,AddQueueSong,L"Add local song to queue\tCtrl+A");
     AppendMenuW(songMenu,MF_STRING,ShowQueue,L"Browse owner song queue\tCtrl+Q");
     AppendMenuW(songMenu,MF_STRING,NextQueueSong,L"Next queued song\tCtrl+N");
     AppendMenuW(songMenu,MF_STRING,PreviousQueueSong,L"Previous queued song\tCtrl+P");
     AppendMenuW(songMenu,MF_SEPARATOR,0,nullptr);
     AppendMenuW(songMenu,MF_STRING,MoveQueuedUp,L"Move queued entry up");
     AppendMenuW(songMenu,MF_STRING,MoveQueuedDown,L"Move queued entry down");
     AppendMenuW(songMenu,MF_STRING,RemoveQueuedSong,L"Remove inactive queued entry");
     AppendMenuW(songMenu,MF_STRING,ClearPendingQueue,L"Clear pending songs (keep current)");
     AppendMenuW(menubar,MF_POPUP,reinterpret_cast<UINT_PTR>(songMenu),L"&Songs");
    }
    if(effectsMenu){
     AppendMenuW(effectsMenu,MF_STRING,FeedbackDown,L"Echo feedback -2%"); 
     AppendMenuW(effectsMenu,MF_STRING,FeedbackUp,L"Echo feedback +2%");
     AppendMenuW(effectsMenu,MF_STRING,DelayShorter,L"Echo delay -20 ms");
     AppendMenuW(effectsMenu,MF_STRING,DelayLonger,L"Echo delay +20 ms");
     AppendMenuW(effectsMenu,MF_SEPARATOR,0,nullptr);
     AppendMenuW(effectsMenu,MF_STRING,DryVoice,L"Dry vocal comparison (keep output gains)\tCtrl+5");
     AppendMenuW(effectsMenu,MF_STRING,RestoreEffects,L"Restore original startup effects\tCtrl+6");
     AppendMenuW(effectsMenu,MF_STRING,ShowEffects,L"Current DSP levels / bounds\tCtrl+E");
     AppendMenuW(menubar,MF_POPUP,reinterpret_cast<UINT_PTR>(effectsMenu),L"&Effects");
    }
    if(recoveryMenu){
     AppendMenuW(recoveryMenu,MF_STRING,ShowRecovery,L"Device health / recovery steps\tCtrl+H");
     AppendMenuW(recoveryMenu,MF_STRING,ChooseDevices,L"Choose microphone / output\tCtrl+D");
     AppendMenuW(recoveryMenu,MF_STRING,Reconnect,L"Owner reconnect (muted)\tCtrl+R");
     AppendMenuW(recoveryMenu,MF_STRING,EmergencyMute,L"VOICE MUTE (focused window)\tF8");
     AppendMenuW(menubar,MF_POPUP,reinterpret_cast<UINT_PTR>(recoveryMenu),L"&Audio");
    }
    SetMenu(h,menubar);
   }else{
    if(practiceMenu)DestroyMenu(practiceMenu);
    if(songMenu)DestroyMenu(songMenu);
    if(effectsMenu)DestroyMenu(effectsMenu);
    if(recoveryMenu)DestroyMenu(recoveryMenu);
    if(menubar)DestroyMenu(menubar);
   }
    self->title=self->label(L"ROOMTONE",SS_LEFT,self->heading);
    self->previous=self->label(L"",SS_CENTER,self->minorFont);
    self->current=self->label(L"",SS_CENTER|SS_CENTERIMAGE,self->large);
    self->next=self->label(L"",SS_CENTER,self->minorFont);
    self->seekLabel=self->label(L"Song position | focus slider, use arrow keys / Home / End to seek",SS_LEFT,self->minorFont);
    self->seekBar=CreateWindowExW(0,TRACKBAR_CLASSW,L"Song position",WS_CHILD|WS_VISIBLE|WS_TABSTOP|TBS_HORZ|TBS_NOTICKS,
        0,0,100,30,h,nullptr,GetModuleHandleW(nullptr),nullptr);
    self->status=self->label(L"Voice MUTED · Music PAUSED",SS_CENTER,self->regular);
    const std::pair<Action,const wchar_t*> labels[]={
     {Play,L"Play / Pause"},{Mic,L"Enable voice"},{Back,L"-10 seconds"},
     {Forward,L"+10 seconds"},{Restart,L"Start song"},{Repeat,L"Repeat OFF"},
     {NextLine,L"Next lyric (Ctrl+Right)"},{ReverbDown,L"Reverb -"},{ReverbUp,L"Reverb +"},
     {EchoDown,L"Echo -"},{EchoUp,L"Echo +"},{MusicDown,L"Music -"},
     {MusicUp,L"Music +"},{ChooseSong,L"Change song"},{ChooseSession,L"Import .rtk / .lrc"},
     {ChooseDevices,L"Select devices"},{Reconnect,L"Reconnect (muted)"},
     {EmergencyMute,L"MUTE VOICE NOW (F8)"},
     {GainDown,L"Vocal gain -"},{GainUp,L"Vocal gain +"},
      {PreviousLine,L"Previous lyric (Ctrl+Left)"},
      {BrowseLyrics,L"Browse all timed lyrics (Ctrl+L)"}
    };
    for(int i=0;i<22;++i)self->buttons[i]=self->button(labels[i].first,labels[i].second);
    self->device=self->label(L"Devices: Windows defaults",SS_LEFT,self->minorFont);
    self->notice=self->label(L"No physical latency measurements available",SS_LEFT,self->minorFont);
    self->layout();
    SetTimer(h,1,120,nullptr);
    self->ready=true;self->render();
    if(self->syntheticTest){
        SendMessageW(self->seekBar,TBM_SETPOS,TRUE,43);
        PostMessageW(h,WM_HSCROLL,MAKEWPARAM(TB_ENDTRACK,0),reinterpret_cast<LPARAM>(self->seekBar));
        PostMessageW(h,WM_DEVICECHANGE,DBT_DEVNODES_CHANGED,0);
        PostMessageW(h,WM_TIMER,1,0);
        PostMessageW(h,WM_COMMAND,MAKEWPARAM(Action::PreviousLine,BN_CLICKED),0);
        PostMessageW(h,WM_COMMAND,MAKEWPARAM(Action::BrowseLyrics,BN_CLICKED),0);
        PostMessageW(h,WM_COMMAND,MAKEWPARAM(Action::MarkLoopA,0),0);
        PostMessageW(h,WM_COMMAND,MAKEWPARAM(Action::OffsetLater,0),0);
        PostMessageW(h,WM_COMMAND,MAKEWPARAM(Action::SaveSession,0),0);
        PostMessageW(h,WM_COMMAND,MAKEWPARAM(Action::AddQueueSong,0),0);
        PostMessageW(h,WM_COMMAND,MAKEWPARAM(Action::ShowQueue,0),0);
        PostMessageW(h,WM_COMMAND,MAKEWPARAM(Action::NextQueueSong,0),0);
        PostMessageW(h,WM_COMMAND,MAKEWPARAM(Action::PreviousQueueSong,0),0);
        PostMessageW(h,WM_COMMAND,MAKEWPARAM(Action::MoveQueuedUp,0),0);
        PostMessageW(h,WM_COMMAND,MAKEWPARAM(Action::MoveQueuedDown,0),0);
        PostMessageW(h,WM_COMMAND,MAKEWPARAM(Action::RemoveQueuedSong,0),0);
        PostMessageW(h,WM_COMMAND,MAKEWPARAM(Action::ClearPendingQueue,0),0);
        PostMessageW(h,WM_COMMAND,MAKEWPARAM(Action::FeedbackDown,0),0);
        PostMessageW(h,WM_COMMAND,MAKEWPARAM(Action::FeedbackUp,0),0);
        PostMessageW(h,WM_COMMAND,MAKEWPARAM(Action::DelayShorter,0),0);
        PostMessageW(h,WM_COMMAND,MAKEWPARAM(Action::DelayLonger,0),0);
        PostMessageW(h,WM_COMMAND,MAKEWPARAM(Action::DryVoice,0),0);
        PostMessageW(h,WM_COMMAND,MAKEWPARAM(Action::RestoreEffects,0),0);
        PostMessageW(h,WM_COMMAND,MAKEWPARAM(Action::ShowEffects,0),0);
        PostMessageW(h,WM_COMMAND,MAKEWPARAM(Action::ShowRecovery,0),0);
        PostMessageW(h,WM_COMMAND,MAKEWPARAM(Action::ChooseLyricSection,0),0);
        PostMessageW(h,WM_COMMAND,MAKEWPARAM(Action::SeekLoopA,0),0);
        PostMessageW(h,WM_COMMAND,MAKEWPARAM(Action::SeekLoopB,0),0);
        PostMessageW(h,WM_COMMAND,MAKEWPARAM(Action::Play,BN_CLICKED),0);
        PostMessageW(h,WM_COMMAND,MAKEWPARAM(Action::EmergencyMute,BN_CLICKED),0);
        PostMessageW(h,WM_CLOSE,0,0);
    }
    return 0;
   }
   case WM_GETMINMAXINFO:{
     auto* info=reinterpret_cast<MINMAXINFO*>(l);
     info->ptMinTrackSize.x=620;info->ptMinTrackSize.y=700;
     return 0;
   }
   case WM_SIZE:self->layout();return 0;
   case WM_HSCROLL:
    if(reinterpret_cast<HWND>(l)==self->seekBar){
     const auto part=LOWORD(w);
     if(part==TB_THUMBTRACK){self->trackingSeek=true;return 0;}
     if(part==TB_ENDTRACK||part==TB_THUMBPOSITION||part==TB_LINEUP||
        part==TB_LINEDOWN||part==TB_PAGEUP||part==TB_PAGEDOWN||
        part==TB_TOP||part==TB_BOTTOM){
        const auto position=static_cast<unsigned>(SendMessageW(self->seekBar,TBM_GETPOS,0,0));
        self->trackingSeek=false;
        if(self->controller.seekSeconds)self->controller.seekSeconds(position);
        self->render();
     }
     return 0;
    }
    break;
   case WM_TIMER:
     if(w==1){if(self->controller.maintenance)self->controller.maintenance();self->render();}
     return 0;
    case WM_DEVICECHANGE:
     if(w==DBT_DEVNODES_CHANGED||w==DBT_DEVICEARRIVAL||w==DBT_DEVICEREMOVECOMPLETE){
      if(self->controller.deviceChanged)self->controller.deviceChanged();
     }
     return DefWindowProcW(h,message,w,l);
   case WM_KEYDOWN:
    // Local to the focused karaoke window; never intercept other apps' F keys.
    if(w==VK_F8)self->notify(EmergencyMute);
    else if(w==VK_F9)self->notify(Mic);
    else if(w==VK_F10)self->notify(Play);
    return 0;
   case WM_COMMAND:
    if(HIWORD(w)==BN_CLICKED&&LOWORD(w)>=101&&LOWORD(w)<=149)
     self->notify(static_cast<Action>(LOWORD(w)));
    return 0;
   case WM_CLOSE:
    // Never leave monitoring running behind a closed window.
    DestroyWindow(h);return 0;
   case WM_DESTROY:
    KillTimer(h,1);
    PostQuitMessage(0);return 0;
  }
  return DefWindowProcW(h,message,w,l);
 }
 int run(){
  INITCOMMONCONTROLSEX common{};common.dwSize=sizeof(common);common.dwICC=ICC_BAR_CLASSES;
  if(!InitCommonControlsEx(&common))throw std::runtime_error("Windows seek slider unavailable");
  const wchar_t* name=L"ROOMTONE Native Karaoke A3";
  WNDCLASSEXW cls{};
  cls.cbSize=sizeof(cls);cls.hInstance=GetModuleHandleW(nullptr);
  cls.lpfnWndProc=&proc;cls.lpszClassName=name;
  cls.hCursor=LoadCursorW(nullptr,MAKEINTRESOURCEW(32512));
  cls.hbrBackground=reinterpret_cast<HBRUSH>(COLOR_WINDOW+1);
  RegisterClassExW(&cls);
  hwnd=CreateWindowExW(0,name,L"ROOMTONE | Native Karaoke",
       WS_OVERLAPPEDWINDOW|WS_VISIBLE,CW_USEDEFAULT,CW_USEDEFAULT,860,745,
       nullptr,nullptr,cls.hInstance,this);
  if(!hwnd)throw std::runtime_error("Unable to open Windows native karaoke interface");
  MSG m{};
  while(GetMessageW(&m,nullptr,0,0)>0){
   // Non-global keys work even while a child button has keyboard focus.
   if((m.hwnd==hwnd||IsChild(hwnd,m.hwnd))&&m.message==WM_KEYDOWN){
    if(m.wParam==VK_F8){notify(EmergencyMute);continue;}
    if(m.wParam==VK_F9){notify(Mic);continue;}
    if(m.wParam==VK_F10){notify(Play);continue;}
     if(GetKeyState(VK_CONTROL)<0){
      Action shortcut{};bool recognized=true;
      switch(m.wParam){
       case VK_LEFT:shortcut=PreviousLine;break;
       case VK_RIGHT:shortcut=NextLine;break;
       case 'O':shortcut=ChooseSong;break;
       case 'I':shortcut=ChooseSession;break;
       case 'D':shortcut=ChooseDevices;break;
       case 'L':shortcut=BrowseLyrics;break;
       case 'R':shortcut=Reconnect;break;
       case '1':shortcut=MarkLoopA;break;
       case '2':shortcut=MarkLoopB;break;
       case '3':shortcut=ClearAB;break;
       case '4':shortcut=ToggleAB;break;
       case '0':shortcut=OffsetReset;break;
       case 'S':shortcut=SaveSession;break;
        case 'A':shortcut=AddQueueSong;break;
        case 'Q':shortcut=ShowQueue;break;
        case 'N':shortcut=NextQueueSong;break;
        case 'P':shortcut=PreviousQueueSong;break;
        case '5':shortcut=DryVoice;break;
        case '6':shortcut=RestoreEffects;break;
        case '7':shortcut=ChooseLyricSection;break;
        case '8':shortcut=SeekLoopA;break;
        case '9':shortcut=SeekLoopB;break;
        case 'E':shortcut=ShowEffects;break;
        case 'H':shortcut=ShowRecovery;break;
       case VK_OEM_4:shortcut=OffsetEarlier;break;
       case VK_OEM_6:shortcut=OffsetLater;break;
       default:recognized=false;break;
      }
      if(recognized){notify(shortcut);continue;}
     }
   }
   if(!IsDialogMessageW(hwnd,&m)){TranslateMessage(&m);DispatchMessageW(&m);}
  }
  DeleteObject(minorFont);DeleteObject(regular);DeleteObject(large);DeleteObject(heading);
  return static_cast<int>(m.wParam);
 }
};
// Modal owner-only native device chooser. Names shown come from a fresh WASAPI
// enumeration, and -1 explicitly means Windows default device.
struct DeviceChooser{
 HWND hwnd=nullptr,input=nullptr,output=nullptr;
 HFONT text=nullptr;
 const std::vector<std::wstring>* inputs=nullptr,*outputs=nullptr;
 int inputChoice=-1,outputChoice=-1;
 int initiallySelectedInput=-1,initiallySelectedOutput=-1; // -2 means missing/ambiguous
 HWND owner=nullptr;
 bool approved=false,syntheticCancel=false,syntheticMute=false;
  std::function<void()> emergencyMute; // focused chooser only
 static LRESULT CALLBACK proc(HWND h,UINT msg,WPARAM w,LPARAM l){
  auto* self=reinterpret_cast<DeviceChooser*>(GetWindowLongPtrW(h,GWLP_USERDATA));
  if(msg==WM_NCCREATE){
   self=static_cast<DeviceChooser*>(reinterpret_cast<CREATESTRUCTW*>(l)->lpCreateParams);
   self->hwnd=h;SetWindowLongPtrW(h,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(self));return TRUE;
  }
  if(!self)return DefWindowProcW(h,msg,w,l);
  if(msg==WM_CREATE){
   self->text=font(18);
   auto create=[&](const wchar_t* cls,const wchar_t* title,DWORD st,int x,int y,int wd,int ht,int id)->HWND{
    HWND c=CreateWindowW(cls,title,WS_CHILD|WS_VISIBLE|st,x,y,wd,ht,h,
           reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),GetModuleHandleW(nullptr),nullptr);
    SendMessageW(c,WM_SETFONT,reinterpret_cast<WPARAM>(self->text),TRUE);return c;
   };
   create(L"STATIC",L"Microphone input",0,20,20,460,30,0);
   self->input=create(L"COMBOBOX",L"",CBS_DROPDOWNLIST|WS_VSCROLL|WS_TABSTOP,20,52,460,240,1);
   create(L"STATIC",L"Speaker output",0,20,101,460,30,0);
   self->output=create(L"COMBOBOX",L"",CBS_DROPDOWNLIST|WS_VSCROLL|WS_TABSTOP,20,133,460,240,2);
   for(auto combo:{self->input,self->output})
    SendMessageW(combo,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(L"Windows default device"));
   for(const auto& value:*self->inputs)
    SendMessageW(self->input,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(value.c_str()));
   for(const auto& value:*self->outputs)
    SendMessageW(self->output,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(value.c_str()));
   // Index zero is the explicit default, never silently preselect it after
   // a prior named endpoint disappeared. CB_SETCURSEL(-1) means no choice.
   SendMessageW(self->input,CB_SETCURSEL,
    static_cast<WPARAM>(self->initiallySelectedInput+1),0);
   SendMessageW(self->output,CB_SETCURSEL,
    static_cast<WPARAM>(self->initiallySelectedOutput+1),0);
   create(L"BUTTON",L"Use devices",BS_DEFPUSHBUTTON|WS_TABSTOP,205,200,135,38,101);
   create(L"BUTTON",L"Cancel",BS_PUSHBUTTON|WS_TABSTOP,350,200,130,38,102);
   if(self->syntheticMute)PostMessageW(self->input,WM_KEYDOWN,VK_F8,0);
    if(self->syntheticCancel)PostMessageW(h,WM_CLOSE,0,0);
   return 0;
  }
  if(msg==WM_COMMAND){
   if(LOWORD(w)==101){
    const int in=static_cast<int>(SendMessageW(self->input,CB_GETCURSEL,0,0));
    const int out=static_cast<int>(SendMessageW(self->output,CB_GETCURSEL,0,0));
    if(in==CB_ERR||out==CB_ERR){
     MessageBoxW(h,L"A selected device is disconnected or ambiguous. Choose a microphone and speaker explicitly.",
       L"ROOMTONE | Device selection required",MB_OK|MB_ICONWARNING);
     return 0;
    }
    self->inputChoice=in-1;self->outputChoice=out-1;
    self->approved=true;DestroyWindow(h);return 0;
   }
   if(LOWORD(w)==102){DestroyWindow(h);return 0;}
  }
  if(msg==WM_CLOSE){DestroyWindow(h);return 0;}
  if(msg==WM_DESTROY){DeleteObject(self->text);self->hwnd=nullptr;return 0;}
  return DefWindowProcW(h,msg,w,l);
 }
 std::optional<std::pair<int,int>> run(){
  WNDCLASSEXW cls{};cls.cbSize=sizeof(cls);cls.lpfnWndProc=proc;
  cls.hInstance=GetModuleHandleW(nullptr);cls.lpszClassName=L"ROOMTONE Device Selection";
  cls.hbrBackground=reinterpret_cast<HBRUSH>(COLOR_WINDOW+1);cls.hCursor=LoadCursorW(nullptr,MAKEINTRESOURCEW(32512));
  RegisterClassExW(&cls);
  const bool disableOwner=owner&&IsWindow(owner)&&IsWindowEnabled(owner);
  const HWND priorFocus=disableOwner?GetFocus():nullptr;
  if(disableOwner)EnableWindow(owner,FALSE);
  hwnd=CreateWindowExW(WS_EX_DLGMODALFRAME,cls.lpszClassName,L"ROOMTONE | Audio devices",
      WS_CAPTION|WS_SYSMENU|WS_VISIBLE,CW_USEDEFAULT,CW_USEDEFAULT,520,290,
      owner,nullptr,cls.hInstance,this);
  if(!hwnd){
   if(disableOwner)EnableWindow(owner,TRUE);
   throw std::runtime_error("Windows device selector failed to open");
  }
  MSG msg{};
  while(hwnd&&IsWindow(hwnd)){
   const auto result=GetMessageW(&msg,nullptr,0,0);
   if(result<=0){
    if(result==0)PostQuitMessage(static_cast<int>(msg.wParam));
    break;
   }
   if(msg.message==WM_KEYDOWN&&msg.wParam==VK_F8&&
       (msg.hwnd==hwnd||IsChild(hwnd,msg.hwnd))){
      if(emergencyMute)emergencyMute();
      continue;
    }
    if(!IsDialogMessageW(hwnd,&msg)){TranslateMessage(&msg);DispatchMessageW(&msg);}
  }
  if(disableOwner&&IsWindow(owner)){
   EnableWindow(owner,TRUE);SetForegroundWindow(owner);
   if(priorFocus&&IsWindow(priorFocus)&&(priorFocus==owner||IsChild(owner,priorFocus)))
    SetFocus(priorFocus);
  }
  if(!approved)return std::nullopt;
  return std::make_pair(inputChoice,outputChoice);
 }
};
// Owner-only accessible list: timed source lines remain in the native .rtk.
// The dialog never starts playback, changes microphone state, or reads any files.
struct LyricChooser {
 HWND hwnd=nullptr,list=nullptr,owner=nullptr;
 HFONT text=nullptr;
 const std::vector<std::wstring>* rows=nullptr;
 std::size_t current=0;
 std::wstring dialogTitle=L"ROOMTONE | Timed lyrics";
 std::wstring hintText=L"Choose a timed line. Arrow keys move; Enter jumps. Escape cancels.";
 std::wstring confirmText=L"Jump to line";
 std::optional<std::size_t> chosen;
 std::function<void()> emergencyMute; // focused modal only, never system-wide
 bool syntheticCancel=false,syntheticMute=false,syntheticAccept=false;
 static LRESULT CALLBACK proc(HWND h,UINT msg,WPARAM w,LPARAM l){
  auto* self=reinterpret_cast<LyricChooser*>(GetWindowLongPtrW(h,GWLP_USERDATA));
  if(msg==WM_NCCREATE){
   self=static_cast<LyricChooser*>(reinterpret_cast<CREATESTRUCTW*>(l)->lpCreateParams);
   self->hwnd=h;SetWindowLongPtrW(h,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(self));return TRUE;
  }
  if(!self)return DefWindowProcW(h,msg,w,l);
  switch(msg){
   case WM_CREATE:{
    self->text=font(18);
    HWND hint=CreateWindowW(L"STATIC",self->hintText.c_str(),
       WS_CHILD|WS_VISIBLE,16,12,686,26,h,nullptr,GetModuleHandleW(nullptr),nullptr);
    SendMessageW(hint,WM_SETFONT,reinterpret_cast<WPARAM>(self->text),TRUE);
    self->list=CreateWindowExW(WS_EX_CLIENTEDGE,L"LISTBOX",L"",
       WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_VSCROLL|LBS_NOTIFY|LBS_NOINTEGRALHEIGHT,
       16,44,686,355,h,reinterpret_cast<HMENU>(201),GetModuleHandleW(nullptr),nullptr);
    SendMessageW(self->list,WM_SETFONT,reinterpret_cast<WPARAM>(self->text),TRUE);
    for(const auto& line:*self->rows)
     SendMessageW(self->list,LB_ADDSTRING,0,reinterpret_cast<LPARAM>(line.c_str()));
    SendMessageW(self->list,LB_SETCURSEL,static_cast<WPARAM>(
     std::min(self->current,self->rows->size()-1)),0);
    auto createButton=[&](const wchar_t* name,int x,int id,DWORD style){
     HWND b=CreateWindowW(L"BUTTON",name,WS_CHILD|WS_VISIBLE|WS_TABSTOP|style,
       x,412,150,37,h,reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),GetModuleHandleW(nullptr),nullptr);
     SendMessageW(b,WM_SETFONT,reinterpret_cast<WPARAM>(self->text),TRUE);
    };
    createButton(self->confirmText.c_str(),398,301,BS_DEFPUSHBUTTON);
    createButton(L"Cancel",558,302,BS_PUSHBUTTON);
    SetFocus(self->list);
    if(self->syntheticMute)PostMessageW(self->list,WM_KEYDOWN,VK_F8,0);
    if(self->syntheticCancel)PostMessageW(h,WM_CLOSE,0,0);
    if(self->syntheticAccept)PostMessageW(h,WM_COMMAND,MAKEWPARAM(301,BN_CLICKED),0);
    return 0;
   }
   case WM_COMMAND:
    if(LOWORD(w)==301||(LOWORD(w)==201&&HIWORD(w)==LBN_DBLCLK)){
     const auto selected=SendMessageW(self->list,LB_GETCURSEL,0,0);
     if(selected!=LB_ERR&&static_cast<std::size_t>(selected)<self->rows->size()){
      self->chosen=static_cast<std::size_t>(selected);DestroyWindow(h);
     }
     return 0;
    }
    if(LOWORD(w)==302){DestroyWindow(h);return 0;}
    break;
   case WM_CLOSE:DestroyWindow(h);return 0;
   case WM_DESTROY:DeleteObject(self->text);self->hwnd=nullptr;return 0;
  }
  return DefWindowProcW(h,msg,w,l);
 }
 std::optional<std::size_t> run(){
  if(!rows||rows->empty())return std::nullopt;
  WNDCLASSEXW cls{};cls.cbSize=sizeof(cls);cls.lpfnWndProc=proc;
  cls.hInstance=GetModuleHandleW(nullptr);cls.lpszClassName=L"ROOMTONE Timed Lyrics Picker";
  cls.hbrBackground=reinterpret_cast<HBRUSH>(COLOR_WINDOW+1);
  cls.hCursor=LoadCursorW(nullptr,MAKEINTRESOURCEW(32512));
  RegisterClassExW(&cls);
  const bool disableOwner=owner&&IsWindow(owner)&&IsWindowEnabled(owner);
  const HWND priorFocus=disableOwner?GetFocus():nullptr;
  if(disableOwner)EnableWindow(owner,FALSE);
  hwnd=CreateWindowExW(WS_EX_DLGMODALFRAME,cls.lpszClassName,dialogTitle.c_str(),
    WS_CAPTION|WS_SYSMENU|WS_VISIBLE,CW_USEDEFAULT,CW_USEDEFAULT,735,505,
    owner,nullptr,cls.hInstance,this);
  if(!hwnd){if(disableOwner)EnableWindow(owner,TRUE);
   throw std::runtime_error("Cannot create native lyric navigator");}
  MSG msg{};
  while(hwnd&&IsWindow(hwnd)){
   const auto result=GetMessageW(&msg,nullptr,0,0);
   if(result<=0){if(result==0)PostQuitMessage(static_cast<int>(msg.wParam));break;}
   // A live lyric modal must never obstruct the owner-local F8 emergency mute.
   if(msg.message==WM_KEYDOWN&&msg.wParam==VK_F8&&
      (msg.hwnd==hwnd||IsChild(hwnd,msg.hwnd))){
    if(emergencyMute)emergencyMute();
    continue;
   }
   if(!IsDialogMessageW(hwnd,&msg)){TranslateMessage(&msg);DispatchMessageW(&msg);}
  }
  if(disableOwner&&IsWindow(owner)){
   EnableWindow(owner,TRUE);SetForegroundWindow(owner);
   if(priorFocus&&IsWindow(priorFocus)&&(priorFocus==owner||IsChild(owner,priorFocus)))
    SetFocus(priorFocus);
  }
  return chosen;
 }
};
inline std::optional<std::size_t> selectLyric(const std::vector<std::wstring>& rows,
 std::size_t current,HWND owner=nullptr,std::function<void()> emergencyMute={}){
 LyricChooser picker;picker.rows=&rows;picker.current=current;picker.owner=owner;
 picker.emergencyMute=std::move(emergencyMute);
 return picker.run();
}
inline std::optional<std::size_t> selectQueuedSong(
 const std::vector<std::wstring>& rows,std::size_t current,HWND owner,
 std::function<void()> emergencyMute={},
 std::wstring title=L"ROOMTONE | Local song queue",
 std::wstring hint=L"Choose an owner-selected song. Enter selects; Esc cancels. No automatic playback.",
 std::wstring confirm=L"Select song"){
 LyricChooser picker;picker.rows=&rows;picker.current=current;picker.owner=owner;
 picker.dialogTitle=std::move(title);
 picker.hintText=std::move(hint);
 picker.confirmText=std::move(confirm);
 picker.emergencyMute=std::move(emergencyMute);
 return picker.run();
}
inline std::optional<std::pair<int,int>> selectDevices(
 const std::vector<std::wstring>& inputs,const std::vector<std::wstring>& outputs,
 HWND owner=nullptr,int initialInput=-1,int initialOutput=-1,
 std::function<void()> emergencyMute={}){
 DeviceChooser picker;picker.inputs=&inputs;picker.outputs=&outputs;picker.owner=owner;
 picker.initiallySelectedInput=initialInput;picker.initiallySelectedOutput=initialOutput;
 picker.emergencyMute=std::move(emergencyMute);
 return picker.run();
}
}
#endif
