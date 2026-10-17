#pragma once
#include "handoff.hpp"
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace roomtone {
// Browser Songs/Lyrics Studio's standard LRC grammar: [mm:ss.xxx] lyric,
// repeated timestamps on one line, [ti:...] and [offset:+/-ms].
// The browser already clamps negative offset-adjusted positions to zero.
// No network, file paths, recording bytes, or identity are placed in a session.
inline bool validUtf8(const std::string& value) noexcept {
 for(std::size_t i=0;i<value.size();){
  const unsigned char c=static_cast<unsigned char>(value[i]);
  if(c<0x80){++i;continue;}
  unsigned length=0;std::uint32_t point=0;
  if(c>=0xC2&&c<=0xDF){length=2;point=c&0x1F;}
  else if(c>=0xE0&&c<=0xEF){length=3;point=c&0x0F;}
  else if(c>=0xF0&&c<=0xF4){length=4;point=c&0x07;}
  else return false;
  if(i+length>value.size())return false;
  for(unsigned j=1;j<length;++j){
   const unsigned char b=static_cast<unsigned char>(value[i+j]);
   if((b&0xC0)!=0x80)return false;
   point=(point<<6)|(b&0x3F);
  }
  if((length==2&&point<0x80)||(length==3&&(point<0x800||(point>=0xD800&&point<=0xDFFF)))||
     (length==4&&(point<0x10000||point>0x10FFFF)))return false;
  i+=length;
 }
 return true;
}
inline std::string trimLrc(const std::string& source) {
 const auto first=source.find_first_not_of(" \t");
 if(first==std::string::npos)return {};
 const auto last=source.find_last_not_of(" \t");
 return source.substr(first,last-first+1);
}
inline bool timestampLrc(const std::string& token,std::uint32_t& millis) {
 const auto colon=token.find(':');
 // Non-time metadata tags [ar:], [al:], [by:] are intentionally ignored.
 if(colon==std::string::npos||token.empty()||token[0]<'0'||token[0]>'9')return false;
 const auto minute=token.substr(0,colon);
 const auto rest=token.substr(colon+1);
 const auto dot=rest.find('.');
 const auto seconds=rest.substr(0,dot);
 const auto fraction=dot==std::string::npos?std::string{}:rest.substr(dot+1);
 if(minute.empty()||minute.size()>3||seconds.size()!=2||
    (dot!=std::string::npos&&(fraction.empty()||fraction.size()>3)))
  throw std::invalid_argument("invalid LRC timestamp");
 const auto digits=[](const std::string& part){
  return std::all_of(part.begin(),part.end(),[](char c){return c>='0'&&c<='9';});
 };
 if(!digits(minute)||!digits(seconds)||!digits(fraction))
  throw std::invalid_argument("invalid LRC timestamp digits");
 const auto mm=std::stoul(minute),ss=std::stoul(seconds);
 if(ss>59)throw std::invalid_argument("invalid LRC seconds");
 std::uint32_t part=0;
 if(!fraction.empty()){
  part=static_cast<std::uint32_t>(std::stoul(fraction));
  if(fraction.size()==1)part*=100;
  else if(fraction.size()==2)part*=10;
 }
 const auto total=static_cast<std::uint64_t>(mm)*60000+ss*1000+part;
 if(total>720000)throw std::invalid_argument("LRC timestamp exceeds 12 minutes");
 millis=static_cast<std::uint32_t>(total);
 return true;
}
inline Handoff parseLrc(const std::string& data,const std::string& audioName,
                        const std::string& fallbackTitle) {
 if(data.size()>1024*1024)throw std::invalid_argument("LRC exceeds 1 MiB");
 if(!validUtf8(data))throw std::invalid_argument("LRC requires valid UTF-8");
 if(audioName.empty()||audioName.size()>255||audioName=="."||audioName==".."||
    audioName.find_first_of("/\\:")!=std::string::npos||
    fallbackTitle.size()>255)
  throw std::invalid_argument("LRC local audio filename/title is invalid");
 Handoff result;
 result.audioName=audioName;result.title=fallbackTitle;
 int currentOffset=0;
 std::size_t cursor=data.compare(0,3,"\xef\xbb\xbf")==0?3:0;
 while(cursor<data.size()){
  const auto end=data.find('\n',cursor);
  auto line=data.substr(cursor,(end==std::string::npos?data.size():end)-cursor);
  cursor=end==std::string::npos?data.size():end+1;
  if(!line.empty()&&line.back()=='\r')line.pop_back();
  if(line.size()>8192)throw std::invalid_argument("LRC source line too long");
  if(line.find('\0')!=std::string::npos)throw std::invalid_argument("LRC contains NUL");
  line=trimLrc(line);
  if(line.empty())continue;
  // Metadata is independent of the instrumental. [offset] follows the
  // browser parser: each following timestamp is adjusted immediately.
  if(line.compare(0,4,"[ti:")==0&&line.back()==']'){
   auto title=trimLrc(line.substr(4,line.size()-5));
   if(title.size()>255||title.find_first_of("\r\n\t\x1b")!=std::string::npos)
    throw std::invalid_argument("unsafe LRC title");
   if(!title.empty())result.title=std::move(title);
   continue;
  }
  if(line.compare(0,8,"[offset:")==0&&line.back()==']'){
   currentOffset=parseOffset(trimLrc(line.substr(8,line.size()-9)));
   continue;
  }
  std::vector<std::uint32_t> stamps;
  std::size_t pos=0;
  while(pos<line.size()&&line[pos]=='['){
   const auto close=line.find(']',pos+1);
   if(close==std::string::npos)break;
   const auto tag=line.substr(pos+1,close-pos-1);
   std::uint32_t ms=0;
   if(!timestampLrc(tag,ms))break;
   const auto adjusted=std::clamp<std::int64_t>(
     static_cast<std::int64_t>(ms)+currentOffset,0,720000);
   stamps.push_back(static_cast<std::uint32_t>(adjusted));
   pos=close+1;
  }
  if(stamps.empty())continue; // Other standard LRC metadata / plain notes.
  auto content=trimLrc(line.substr(pos));
  if(content.size()>2000)throw std::invalid_argument("LRC lyric exceeds 2000 bytes");
  for(unsigned char c:content)if(c<0x20||c==0x7F)
    throw std::invalid_argument("unsafe control character in LRC lyric");
  if(result.lines.size()+stamps.size()>500)
   throw std::invalid_argument("LRC exceeds native 500-line limit");
  for(auto ms:stamps)result.lines.push_back({ms,content});
 }
 if(result.lines.empty())throw std::invalid_argument("LRC has no timed lyric lines");
 std::stable_sort(result.lines.begin(),result.lines.end(),
   [](const HandoffLine& a,const HandoffLine& b){return a.milliseconds<b.milliseconds;});
 return result;
}
inline Handoff loadLrc(const std::filesystem::path& path,const std::string& audioName,
                       const std::string& fallbackTitle){
 std::ifstream input(path,std::ios::binary|std::ios::ate);
 if(!input)throw std::runtime_error("Cannot open local LRC file");
 const auto size=input.tellg();
 if(size<0||size>1024*1024)throw std::runtime_error("LRC file exceeds 1 MiB");
 std::string data(static_cast<std::size_t>(size),'\0');
 input.seekg(0);
 if(!input.read(data.data(),static_cast<std::streamsize>(data.size())))
  throw std::runtime_error("Could not read local LRC file");
 return parseLrc(data,audioName,fallbackTitle);
}
}
