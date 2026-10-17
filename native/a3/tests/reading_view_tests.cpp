#include "../src/reading_view.hpp"
#include <iostream>
#include <stdexcept>
static void need(bool p,const char* err){if(!p)throw std::runtime_error(err);}
int main(){
 using namespace roomtone::reading_view;
 for(int w:{570,620,860,1440})for(int h:{640,650,768,900,1200}){
  const auto a=layout(w,h);
  for(const auto& rect:{a.title,a.previous,a.current,a.next,a.seekLabel,
        a.seekBar,a.status,a.play,a.reconnect,a.mute,a.notice})
   need(within(rect,w,h),"focus control outside native client bounds");
  need(a.current.y+a.current.height<=a.next.y,"large lyric never overlaps upcoming line");
  need(a.next.y+a.next.height<=a.seekLabel.y,"upcoming line never covers seek");
  need(a.seekBar.y+a.seekBar.height<=a.status.y,"seek never covers safety status");
  need(a.status.y+a.status.height<=a.play.y,"safety status above transport");
  need(a.play.y+a.play.height<=a.notice.y,"transport above warning");
  need(a.mute.width>=160&&a.current.height>=288,"readable source lyric and panic mute");
 }
 need(fontSize(false,-2)==30&&fontSize(false,9)==46,
      "bounded normal font sizes");
 need(fontSize(true,0)==38&&fontSize(true,1)==48&&fontSize(true,2)==58,
      "bounded focused font sizes");
 need(fontSize(true,2)>fontSize(false,2),
      "focus mode genuinely enlarges original native lyric");
 std::cout<<"PASS bounded native large-lyric reading geometry and focus safety surfaces at 570-1440 px, 640-1200 px. No Windows audio opened.\n";
}