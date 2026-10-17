#pragma once
#include <algorithm>
namespace roomtone::reading_view {
// Pure geometry for the existing native Win32 window. No second renderer,
// lyric clock, IO, sound engine, owner state persistence, or OS API.
struct Rect {int x,y,width,height;};
struct Focus {
 Rect title,previous,current,next,seekLabel,seekBar,status,play,reconnect,mute,notice;
};
inline int fontSize(bool focused,int step) noexcept {
 return (focused?38:30)+std::clamp(step,0,2)*(focused?10:8);
}
inline Focus layout(int actualWidth,int actualHeight) noexcept{
 const int w=std::max(570,actualWidth),h=std::max(640,actualHeight);
 const int pad=20,content=w-2*pad,third=(content-16)/3;
 return {
  {pad,8,content,34}, {pad,48,content,36},
  {pad,92,content,h-352}, // lyric region never obscures navigation/status
  {pad,h-248,content,35},
  {pad,h-209,content,19}, {pad,h-186,content,30},
  {pad,h-148,content,34},
  {pad,h-107,third,39},
  {pad+third+8,h-107,third,39},
  {pad+2*(third+8),h-107,content-2*(third+8),39},
  {pad,h-60,content,52}
 };
}
inline bool within(Rect area,int width,int height) noexcept{
 return area.width>0&&area.height>0&&area.x>=0&&area.y>=0&&
  area.x+area.width<=std::max(570,width)&&
  area.y+area.height<=std::max(640,height);
}
}