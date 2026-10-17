#pragma once
#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace roomtone::song_queue {
// Owner-provided local files only. This is a volatile UI/navigation list,
// not a second decoder, playback engine, persisted library, or background sync.
class Queue {
public:
 static constexpr std::size_t MaxSongs=24;
 explicit Queue(const std::filesystem::path& current={}){
  if(!current.empty())reset(current);
 }
 bool reset(const std::filesystem::path& file){
  if(file.empty()||file.filename().empty())return false;
  items_={file};current_=0;return true;
 }
 bool add(const std::filesystem::path& file){
  if(file.empty()||file.filename().empty()||items_.size()>=MaxSongs)return false;
  if(std::find(items_.begin(),items_.end(),file)!=items_.end())return false;
  items_.push_back(file);return true;
 }
 std::size_t size()const noexcept{return items_.size();}
 std::size_t currentIndex()const noexcept{return current_;}
 const std::filesystem::path& at(std::size_t index)const{return items_.at(index);}
 const std::filesystem::path& current()const{return items_.at(current_);}
 std::optional<std::size_t> next()const noexcept{
  if(items_.empty()||current_+1>=items_.size())return std::nullopt;
  return current_+1;
 }
 std::optional<std::size_t> previous()const noexcept{
  if(items_.empty()||current_==0)return std::nullopt;
  return current_-1;
 }
 bool canSelect(std::size_t index)const noexcept{return index<items_.size();}
 // The calling owner thread must decode first and stop/join WASAPI before
 // committing; a failed decode leaves both queue selection and mixer intact.
 bool commit(std::size_t index)noexcept{
  if(!canSelect(index))return false;
  current_=index;return true;
 }
private:
 std::vector<std::filesystem::path> items_;
 std::size_t current_=0;
};
}
