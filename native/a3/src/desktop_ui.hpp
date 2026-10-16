#pragma once
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <algorithm>
#include <functional>
#include <string>
#include <vector>
#include <utility>
#include <optional>
#include <stdexcept>

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
 ChooseSong,ChooseSession,ChooseDevices,Reconnect
};
struct Snapshot {
 std::wstring title,previous,current,next,input,output,notice;
 unsigned positionSeconds=0,durationSeconds=0;
 float reverb=0,echo=0,music=0;
 bool playing=false,microphone=false,repeat=false,interrupted=false,deviceActive=false,hasSong=false;
};
struct Controller {
 std::function<Snapshot()> snapshot;
 std::function<void(Action)> action;
};
inline HFONT font(int size,bool bold=false) {
 return CreateFontW(-size,0,0,0,bold?FW_SEMIBOLD:FW_NORMAL,FALSE,FALSE,FALSE,
                    DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
                    CLEARTYPE_QUALITY,DEFAULT_PITCH|FF_DONTCARE,L"Segoe UI");
}
inline void setText(HWND handle,const std::wstring& value){
 if(handle)SetWindowTextW(handle,value.c_str());
}
inline std::wstring clock(unsigned seconds) {
 return std::to_wstring(seconds/60)+L":"+(seconds%60<10?L"0":L"")+std::to_wstring(seconds%60);
}
struct Window {
 Controller controller;
 HWND hwnd=nullptr,title=nullptr,previous=nullptr,current=nullptr,next=nullptr,
      status=nullptr,device=nullptr,notice=nullptr;
 HWND buttons[17]{};
 HFONT minorFont=nullptr,regular=nullptr,large=nullptr,heading=nullptr;
 bool ready=false;
 bool syntheticTest=false;
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
 void layout(){
  RECT bounds{};GetClientRect(hwnd,&bounds);
  const int w=std::max(570L,bounds.right-bounds.left);
  const int pad=24;
  MoveWindow(title,pad,15,w-2*pad,40,TRUE);
  MoveWindow(previous,pad,70,w-2*pad,34,TRUE);
  MoveWindow(current,pad,109,w-2*pad,96,TRUE);
  MoveWindow(next,pad,208,w-2*pad,35,TRUE);
  MoveWindow(status,pad,256,w-2*pad,44,TRUE);
  const int gap=9,columns=4,bw=(w-2*pad-gap*(columns-1))/columns;
  for(int i=0;i<16;++i){
   const int x=pad+(i%columns)*(bw+gap),y=308+(i/columns)*48;
   MoveWindow(buttons[i],x,y,bw,40,TRUE);
  }
  MoveWindow(buttons[16],pad,503,w-2*pad,40,TRUE);
  MoveWindow(device,pad,558,w-2*pad,38,TRUE);
  MoveWindow(notice,pad,606,w-2*pad,44,TRUE);
 }
 void render() {
  if(!controller.snapshot)return;
  const auto s=controller.snapshot();
  setText(title,s.title.empty()?L"ROOMTONE / NATIVE KARAOKE":s.title);
  setText(previous,s.previous.empty()?L" ":s.previous);
  setText(current,s.current.empty()?L"No synchronized lyrics loaded":s.current);
  setText(next,s.next.empty()?L" ":s.next);
  auto state=s.interrupted?L"INTERRUPTED":s.deviceActive?L"READY":L"DEVICE STOPPED";
  setText(status,clock(s.positionSeconds)+L" / "+clock(s.durationSeconds)
      +L"  |  "+state+L"  |  Voice "+(s.microphone?L"ON":L"MUTED")
      +L"  |  Music "+(s.playing?L"PLAYING":L"PAUSED"));
  setText(buttons[0],s.playing?L"Pause music":L"Play music");
  setText(buttons[1],s.microphone?L"MUTE VOICE (F9)":L"Enable voice (F9)");
  setText(buttons[5],s.repeat?L"Repeat ON":L"Repeat OFF");
  setText(buttons[7],L"Reverb - ("+std::to_wstring(static_cast<int>(s.reverb*100))+L"%)");
  setText(buttons[8],L"Reverb +");
  setText(buttons[9],L"Echo - ("+std::to_wstring(static_cast<int>(s.echo*100))+L"%)");
  setText(buttons[10],L"Echo +");
  setText(buttons[11],L"Music - ("+std::to_wstring(static_cast<int>(s.music*100))+L"%)");
  setText(buttons[12],L"Music +");
  setText(device,L"Input: "+s.input+L"  |  Output: "+s.output);
  setText(notice,s.notice.empty()?L"Local WASAPI | No network | Buffer size is NOT measured mic-to-speaker latency":s.notice);
  EnableWindow(buttons[0],s.hasSong&&s.deviceActive&&!s.interrupted);
  EnableWindow(buttons[1],s.deviceActive&&!s.interrupted);
  for(int i:{2,3,4,5,6})EnableWindow(buttons[i],s.hasSong&&s.deviceActive&&!s.interrupted);
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
    self->title=self->label(L"ROOMTONE",SS_LEFT,self->heading);
    self->previous=self->label(L"",SS_CENTER,self->minorFont);
    self->current=self->label(L"",SS_CENTER|SS_CENTERIMAGE,self->large);
    self->next=self->label(L"",SS_CENTER,self->minorFont);
    self->status=self->label(L"Voice MUTED · Music PAUSED",SS_CENTER,self->regular);
    const std::pair<Action,const wchar_t*> labels[]={
     {Play,L"Play / Pause"},{Mic,L"Enable voice"},{Back,L"-10 seconds"},
     {Forward,L"+10 seconds"},{Restart,L"Start song"},{Repeat,L"Repeat OFF"},
     {NextLine,L"Next lyric"},{ReverbDown,L"Reverb -"},{ReverbUp,L"Reverb +"},
     {EchoDown,L"Echo -"},{EchoUp,L"Echo +"},{MusicDown,L"Music -"},
     {MusicUp,L"Music +"},{ChooseSong,L"Change song"},{ChooseSession,L"Import .rtk"},
     {ChooseDevices,L"Select devices"},{Reconnect,L"Reconnect (muted)"}
    };
    for(int i=0;i<17;++i)self->buttons[i]=self->button(labels[i].first,labels[i].second);
    self->device=self->label(L"Devices: Windows defaults",SS_LEFT,self->minorFont);
    self->notice=self->label(L"No physical latency measurements available",SS_LEFT,self->minorFont);
    self->layout();
    SetTimer(h,1,120,nullptr);
    self->ready=true;self->render();
    if(self->syntheticTest){
        PostMessageW(h,WM_COMMAND,MAKEWPARAM(Action::Play,BN_CLICKED),0);
        PostMessageW(h,WM_CLOSE,0,0);
    }
    return 0;
   }
   case WM_GETMINMAXINFO:{
     auto* info=reinterpret_cast<MINMAXINFO*>(l);
     info->ptMinTrackSize.x=620;info->ptMinTrackSize.y=725;
     return 0;
   }
   case WM_SIZE:self->layout();return 0;
   case WM_TIMER:if(w==1)self->render();return 0;
   case WM_KEYDOWN:
    // Local to the focused karaoke window; never intercept other apps' F keys.
    if(w==VK_F9)self->notify(Mic);
    else if(w==VK_F10)self->notify(Play);
    return 0;
   case WM_COMMAND:
    if(HIWORD(w)==BN_CLICKED&&LOWORD(w)>=101&&LOWORD(w)<=117)
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
  const wchar_t* name=L"ROOMTONE Native Karaoke A3";
  WNDCLASSEXW cls{};
  cls.cbSize=sizeof(cls);cls.hInstance=GetModuleHandleW(nullptr);
  cls.lpfnWndProc=&proc;cls.lpszClassName=name;
  cls.hCursor=LoadCursorW(nullptr,MAKEINTRESOURCEW(32512));
  cls.hbrBackground=reinterpret_cast<HBRUSH>(COLOR_WINDOW+1);
  RegisterClassExW(&cls);
  hwnd=CreateWindowExW(0,name,L"ROOMTONE | Native Karaoke",
       WS_OVERLAPPEDWINDOW|WS_VISIBLE,CW_USEDEFAULT,CW_USEDEFAULT,920,750,
       nullptr,nullptr,cls.hInstance,this);
  if(!hwnd)throw std::runtime_error("Unable to open Windows native karaoke interface");
  MSG m{};
  while(GetMessageW(&m,nullptr,0,0)>0){
   // Non-global keys work even while a child button has keyboard focus.
   if((m.hwnd==hwnd||IsChild(hwnd,m.hwnd))&&m.message==WM_KEYDOWN){
    if(m.wParam==VK_F9){notify(Mic);continue;}
    if(m.wParam==VK_F10){notify(Play);continue;}
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
 bool approved=false;
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
   SendMessageW(self->input,CB_SETCURSEL,0,0);
   SendMessageW(self->output,CB_SETCURSEL,0,0);
   create(L"BUTTON",L"Use devices",BS_DEFPUSHBUTTON|WS_TABSTOP,205,200,135,38,101);
   create(L"BUTTON",L"Cancel",BS_PUSHBUTTON|WS_TABSTOP,350,200,130,38,102);
   return 0;
  }
  if(msg==WM_COMMAND){
   if(LOWORD(w)==101){
    self->inputChoice=static_cast<int>(SendMessageW(self->input,CB_GETCURSEL,0,0))-1;
    self->outputChoice=static_cast<int>(SendMessageW(self->output,CB_GETCURSEL,0,0))-1;
    self->approved=true;DestroyWindow(h);return 0;
   }
   if(LOWORD(w)==102){DestroyWindow(h);return 0;}
  }
  if(msg==WM_CLOSE){DestroyWindow(h);return 0;}
  if(msg==WM_DESTROY){DeleteObject(self->text);PostQuitMessage(0);return 0;}
  return DefWindowProcW(h,msg,w,l);
 }
 std::optional<std::pair<int,int>> run(){
  WNDCLASSEXW cls{};cls.cbSize=sizeof(cls);cls.lpfnWndProc=proc;
  cls.hInstance=GetModuleHandleW(nullptr);cls.lpszClassName=L"ROOMTONE Device Selection";
  cls.hbrBackground=reinterpret_cast<HBRUSH>(COLOR_WINDOW+1);cls.hCursor=LoadCursorW(nullptr,MAKEINTRESOURCEW(32512));
  RegisterClassExW(&cls);
  hwnd=CreateWindowExW(WS_EX_DLGMODALFRAME,cls.lpszClassName,L"ROOMTONE | Audio devices",
      WS_CAPTION|WS_SYSMENU|WS_VISIBLE,CW_USEDEFAULT,CW_USEDEFAULT,520,290,
      nullptr,nullptr,cls.hInstance,this);
  if(!hwnd)throw std::runtime_error("Windows device selector failed to open");
  MSG msg{};
  while(GetMessageW(&msg,nullptr,0,0)>0)
   if(!IsDialogMessageW(hwnd,&msg)){TranslateMessage(&msg);DispatchMessageW(&msg);}
  if(!approved)return std::nullopt;
  return std::make_pair(inputChoice,outputChoice);
 }
};
inline std::optional<std::pair<int,int>> selectDevices(
 const std::vector<std::wstring>& inputs,const std::vector<std::wstring>& outputs){
 DeviceChooser picker;picker.inputs=&inputs;picker.outputs=&outputs;
 return picker.run();
}
}
#endif
