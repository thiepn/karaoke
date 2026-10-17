#pragma once
#include "handoff.hpp"
#include "practice.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace roomtone::song_queue {
// A queued file's practice belongs ONLY to this user-picked full local path.
// Its source-owned .rtk content is held in volatile RAM. Nothing is auto-saved,
// serialized to a new store, fetched, synced or handed to any other app.
struct Practice {
 Handoff session;
 std::string displayTitle;
 std::uint64_t frames=0,loopA=0,loopB=0;
 int importedOffset=0;
 bool hasSession=false,repeat=false,loopValid=false,startMarked=false;
};
class Queue {
public:
 static constexpr std::size_t MaxSongs=24;
 explicit Queue(const std::filesystem::path& current={}){
  if(!current.empty())reset(current);
 }
 bool reset(const std::filesystem::path& file){
  if(file.empty()||file.filename().empty())return false;
  items_={{file,std::nullopt}};current_=0;return true;
 }
 bool add(const std::filesystem::path& file){
  if(file.empty()||file.filename().empty()||items_.size()>=MaxSongs)return false;
  if(std::any_of(items_.begin(),items_.end(),[&](const Entry& e){return e.file==file;}))
   return false;
  items_.push_back({file,std::nullopt});return true;
 }
 std::size_t size()const noexcept{return items_.size();}
 std::size_t currentIndex()const noexcept{return current_;}
 const std::filesystem::path& at(std::size_t index)const{return items_.at(index).file;}
 const std::filesystem::path& current()const{return at(current_);}
 std::optional<std::size_t> next()const noexcept{
  if(items_.empty()||current_+1>=items_.size())return std::nullopt;
  return current_+1;
 }
 std::optional<std::size_t> previous()const noexcept{
  if(items_.empty()||current_==0)return std::nullopt;
  return current_-1;
 }
 bool canSelect(std::size_t index)const noexcept{return index<items_.size();}
 // Read/copy a queued snapshot BEFORE stopping/replacing PCM, since
 // switching the owner may update the selected entry's saved state.
 std::optional<Practice> practice(std::size_t index)const{
  return items_.at(index).saved;
 }
 bool rememberCurrent(const Practice& data){
  if(items_.empty())return false;
  // Source owner names are checked against the original local filename;
  // another path with the same basename never inherits this saved state.
  if(data.hasSession&&data.session.audioName!=items_[current_].file.filename().u8string())
   return false;
  items_[current_].saved=data;return true;
 }
 // Requires successful local decode and stopped callback by the caller.
 bool commit(std::size_t index)noexcept{
  if(!canSelect(index))return false;
  current_=index;return true;
 }
 // Explicit queue-management only; don't implicitly switch PCM when deleting.
 bool remove(std::size_t index){
  if(index>=items_.size()||index==current_)return false;
  items_.erase(items_.begin()+static_cast<std::ptrdiff_t>(index));
  if(index<current_)--current_;
  return true;
 }
 bool move(std::size_t index,int delta){
  if(index>=items_.size()||(delta!=-1&&delta!=1))return false;
  const auto dest=static_cast<std::int64_t>(index)+delta;
  if(dest<0||dest>=static_cast<std::int64_t>(items_.size()))return false;
  const auto other=static_cast<std::size_t>(dest);
  std::swap(items_[index],items_[other]);
  if(current_==index)current_=other;
  else if(current_==other)current_=index;
  return true;
 }
 std::size_t clearPending(){
  if(items_.empty())return 0;
  const auto removed=items_.size()-1;
  auto active=std::move(items_[current_]);
  items_.clear();items_.push_back(std::move(active));current_=0;
  return removed;
 }
private:
 struct Entry{
  std::filesystem::path file;
  std::optional<Practice> saved;
 };
 std::vector<Entry> items_;
 std::size_t current_=0;
};
}
