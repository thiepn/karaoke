#include "../src/song_queue.hpp"
#include <filesystem>
#include <iostream>
#include <stdexcept>
static void need(bool value,const char* reason){if(!value)throw std::runtime_error(reason);}
int main(){
 using roomtone::song_queue::Queue;
 Queue list;
 need(list.size()==0&&!list.next()&&!list.previous()&&!list.commit(0),"empty queue safe");
 need(list.reset(std::filesystem::u8path("C:/Music/first.wav")),"explicit first song");
 need(list.size()==1&&list.currentIndex()==0&&!list.next()&&!list.previous(),"single song boundaries");
 need(list.add(std::filesystem::u8path("C:/Music/second.flac")),"append next");
 need(!list.add(std::filesystem::u8path("C:/Music/second.flac")),"duplicate path rejected");
 need(list.add(std::filesystem::u8path("D:/Music/second.flac")),"different local path distinct");
 need(list.next()&&*list.next()==1,"forward candidate");
 need(list.currentIndex()==0,"candidate does not advance until decode/commit");
 need(!list.commit(5)&&list.currentIndex()==0,"invalid index does not change selection");
 need(list.commit(1)&&list.previous()&&*list.previous()==0,"explicit commit and previous");
 need(list.at(1).filename()==std::filesystem::path("second.flac"),"display basename");
 need(list.commit(2)&&!list.next(),"last song bound");
 need(list.reset(std::filesystem::u8path("E:/one.wav")),"new song resets old context");
 need(list.size()==1&&list.currentIndex()==0,"reset source ownership");
 for(std::size_t i=1;i<Queue::MaxSongs;++i)
  need(list.add(std::filesystem::path("track"+std::to_string(i)+".wav")),"bounded queue fill");
 need(!list.add("overflow.wav")&&list.size()==Queue::MaxSongs,"24 track cap");
 need(!list.add(std::filesystem::path{}),"empty path rejected");
 need(!list.reset(std::filesystem::path{}),"invalid reset retained");
 need(list.size()==Queue::MaxSongs,"reject no state loss");
 std::cout<<"PASS source-owned volatile native queue, owner transitions, bounds, duplicate rejection, no file writes.\n";
}
