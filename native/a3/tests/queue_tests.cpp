#include "../src/song_queue.hpp"
#include <filesystem>
#include <fstream>
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
 roomtone::song_queue::Practice owner;
 owner.frames=144000;owner.loopA=48000;owner.loopB=96000;
 owner.repeat=true;owner.loopValid=true;owner.startMarked=true;
 owner.hasSession=true;owner.session.audioName="one.wav";
 owner.session.title="Owner source";
 owner.session.offsetMs=-250;
 owner.session.lines={{1000,"First"},{1500,"Second"}};
 need(list.rememberCurrent(owner),"capture current path-owned practice");
 need(!list.practice(1),"queued neighbor has no inherited practice");
 need(list.commit(1),"switch to another owner path");
 need(!list.practice(1),"same queue no cross-song content");
 roomtone::song_queue::Practice wrong=owner;
 need(!list.rememberCurrent(wrong),"mismatched audio identity not cached");
 need(list.commit(0),"return to original");
 auto recovered=list.practice(0);
 need(recovered&&recovered->frames==144000&&recovered->session.lines.size()==2&&
      recovered->session.offsetMs==-250&&recovered->repeat,"original source state recovered");
 need(list.move(0,1)&&list.currentIndex()==1,"current entry reorder tracks identity");
 need(list.at(1).filename()=="one.wav"&&list.practice(1)->frames==144000,
      "reorder retains original source snapshot");
 need(!list.remove(1)&&list.currentIndex()==1,"active source cannot be removed");
 need(list.remove(0)&&list.currentIndex()==0,"earlier inactive removal updates current index");
 need(list.at(0).filename()=="one.wav"&&list.practice(0)->session.audioName=="one.wav",
      "remove cannot transfer practice ownership");
 need(!list.move(0,-1)&&!list.move(0,5),"invalid reorder rejected");
 const auto removed=list.clearPending();
 need(removed==Queue::MaxSongs-2&&list.size()==1&&list.currentIndex()==0,
      "clear pending keeps only active original source");
 need(list.practice(0)->session.lines[0].text=="First","clear pending retains active state");
 need(list.reset("fresh.wav")&&!list.practice(0),"explicit new queue destroys prior local practice");
 // A previously authorized FULL pathname can now point to changed audio.
 // Refuse its stale lyric/practice cache using observed version evidence.
 const auto fixture=std::filesystem::temp_directory_path()/"roomtone-queue-version-fixture.wav";
 if(std::filesystem::exists(fixture))throw std::runtime_error("refusing existing test fixture");
 try{
  {std::ofstream f(fixture,std::ios::binary);f<<"original pcm"; }
  roomtone::song_queue::Queue owned(fixture);
  roomtone::song_queue::Practice history;
  history.hasSession=true;history.session.audioName=fixture.filename().u8string();
  history.frames=48000;history.session.lines={{1000,"Source lyric"}};
  need(owned.rememberCurrent(history),"bound session cache");
  const auto original=roomtone::song_queue::observe(fixture);
  need(original.has_value()&&owned.verifiedPractice(0,*original).has_value(),
       "original observed file version authorizes its own snapshot");
  {std::ofstream f(fixture,std::ios::binary|std::ios::trunc);f<<"replacement audio data"; }
  const auto replaced=roomtone::song_queue::observe(fixture);
  need(replaced.has_value()&&!original->same(*replaced),"source metadata changed");
  need(!owned.verifiedPractice(0,*replaced),"stale practice cannot attach to replacement");
  need(owned.practice(0).has_value(),"owner cache untouched until explicit queue reset");
  need(owned.commitVerified(0,*replaced),"new decoded source can be explicitly bound");
  need(!owned.verifiedPractice(0,*replaced),
       "new source version permanently invalidates stale original practice");
  roomtone::song_queue::Practice updated;
  updated.hasSession=true;updated.session.audioName=fixture.filename().u8string();
  updated.session.lines={{2300,"New source lyric"}};updated.frames=96000;
  need(owned.rememberCurrent(updated),"store independently reviewed new source practice");
  need(owned.verifiedPractice(0,*replaced)->frames==96000,
       "new source practice available on later revisits");
  need(owned.commitVerified(0,*replaced)&&owned.verifiedPractice(0,*replaced)->frames==96000,
       "unchanged source version preserves correct new snapshot on further revisit");
 }catch(...){std::filesystem::remove(fixture);throw;}
 std::filesystem::remove(fixture);
 std::cout<<"PASS source-owned volatile native queue, owner transitions, bounds, duplicate rejection, no file writes.\n";
}
