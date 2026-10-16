#pragma once
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <filesystem>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
namespace roomtone {
struct HandoffLine {std::uint32_t milliseconds;std::string text;};
struct Handoff {
 std::string audioName,title;
 std::uint32_t startMs=0;
 int offsetMs=0;
 bool loop=false;
 std::uint32_t loopStartMs=0,loopEndMs=0;
 std::vector<HandoffLine> lines;
};
inline unsigned decodeHex(char c){
 if(c>='0'&&c<='9')return static_cast<unsigned>(c-'0');
 if(c>='A'&&c<='F')return static_cast<unsigned>(c-'A'+10);
 if(c>='a'&&c<='f')return static_cast<unsigned>(c-'a'+10);
 throw std::invalid_argument("bad percent-encoded character");
}
inline std::string percentDecode(const std::string& encoded){
 if(encoded.size()>24000)throw std::invalid_argument("encoded value too large");
 std::string out;out.reserve(encoded.size());
 for(std::size_t i=0;i<encoded.size();++i){
  unsigned char c=static_cast<unsigned char>(encoded[i]);
  if(c=='%'){
   if(i+2>=encoded.size())throw std::invalid_argument("truncated percent encoding");
   c=static_cast<unsigned char>((decodeHex(encoded[++i])<<4)|decodeHex(encoded[++i]));
  }else if(c>127 || c<' ' || c=='\x7f'){
   throw std::invalid_argument("unencoded control or non-ASCII character");
  }
  if(c==0||c==10||c==13||c==27||c=='\x7f')
   throw std::invalid_argument("unsafe control in lyric/metadata");
  out.push_back(static_cast<char>(c));
 }
 return out;
}
inline std::uint32_t parseMillis(const std::string& str,std::uint32_t max=720000){
 if(str.empty()||str.size()>10)throw std::invalid_argument("invalid milliseconds");
 std::uint64_t n=0;
 for(char c:str){
  if(c<'0'||c>'9')throw std::invalid_argument("milliseconds must be unsigned digits");
  n=n*10+static_cast<unsigned>(c-'0');
  if(n>max)throw std::invalid_argument("milliseconds out of supported song bounds");
 }
 return static_cast<std::uint32_t>(n);
}
inline int parseOffset(const std::string& str){
 if(str.empty()||str.size()>8)throw std::invalid_argument("bad lyric offset");
 std::size_t i=0;bool neg=false;
 if(str[0]=='-'){neg=true;i=1;}else if(str[0]=='+')i=1;
 if(i==str.size())throw std::invalid_argument("bad lyric offset");
 int n=0;
 for(;i<str.size();++i){
  if(str[i]<'0'||str[i]>'9')throw std::invalid_argument("bad lyric offset");
  n=n*10+str[i]-'0';
  if(n>30000)throw std::invalid_argument("lyric offset out of range");
 }
 return neg?-n:n;
}
inline Handoff parseHandoff(const std::string& data){
 if(data.size()>1024*1024)throw std::invalid_argument("session file exceeds 1 MiB");
 const auto first=data.find('\n');
 std::string magic=first==std::string::npos?std::string{}:data.substr(0,first);
 if(!magic.empty()&&magic.back()=='\r')magic.pop_back();
 if(magic.size()>=3&&magic.substr(0,3)=="\xef\xbb\xbf")magic.erase(0,3);
 if(first==std::string::npos||magic!="ROOMTONE_HANDOFF_V1")
  throw std::invalid_argument("wrong session format");
 Handoff h;
 bool audio=false,title=false,start=false,offset=false,loop=false,a=false,b=false;
 std::uint32_t prevTime=0;
 for(std::size_t cursor=first+1;cursor<data.size();){
  const auto end=data.find('\n',cursor);
  auto line=data.substr(cursor,(end==std::string::npos?data.size():end)-cursor);
  cursor=end==std::string::npos?data.size():end+1;
  if(!line.empty()&&line.back()=='\r')line.pop_back();
  if(line.empty())continue;
  auto tab=line.find('\t');
  if(tab==std::string::npos)throw std::invalid_argument("malformed session record");
  const auto key=line.substr(0,tab),value=line.substr(tab+1);
  if(key=="audio"&&!audio){h.audioName=percentDecode(value);audio=true;}
  else if(key=="title"&&!title){h.title=percentDecode(value);title=true;}
  else if(key=="start-ms"&&!start){h.startMs=parseMillis(value);start=true;}
  else if(key=="offset-ms"&&!offset){h.offsetMs=parseOffset(value);offset=true;}
  else if(key=="loop"&&!loop){
   if(value!="0"&&value!="1")throw std::invalid_argument("invalid repeat flag");
   h.loop=value=="1";loop=true;
  }else if(key=="loop-start-ms"&&!a){h.loopStartMs=parseMillis(value);a=true;}
  else if(key=="loop-end-ms"&&!b){h.loopEndMs=parseMillis(value);b=true;}
  else if(key=="lyric"){
   if(h.lines.size()>=500)throw std::invalid_argument("too many lyrics");
   const auto mid=value.find('\t');
   if(mid==std::string::npos)throw std::invalid_argument("bad lyric record");
   auto ms=parseMillis(value.substr(0,mid));
   if(!h.lines.empty()&&ms<prevTime)throw std::invalid_argument("lyrics must be sorted");
   prevTime=ms;
   auto decoded=percentDecode(value.substr(mid+1));
   if(decoded.size()>2000)throw std::invalid_argument("lyric line too long");
   h.lines.push_back({ms,std::move(decoded)});
  }else throw std::invalid_argument("unknown or duplicate session field: "+key);
 }
 if(!(audio&&title&&start&&offset&&loop&&a&&b))
  throw std::invalid_argument("missing required session metadata");
 if(h.audioName.empty()||h.audioName.size()>255||h.audioName=="."||h.audioName==".."||
    h.audioName.find('/')!=std::string::npos||h.audioName.find('\\')!=std::string::npos||
    h.audioName.find(':')!=std::string::npos)
  throw std::invalid_argument("audio must be filename only, never a file path");
 if(h.title.size()>255)throw std::invalid_argument("title too long");
 if(h.loop&&(h.loopEndMs<=h.loopStartMs+250))
  throw std::invalid_argument("invalid loop interval");
 return h;
}
inline Handoff loadHandoff(const std::filesystem::path& path){
 std::ifstream file(path,std::ios::binary|std::ios::ate);
 if(!file)throw std::runtime_error("cannot open local handoff file");
 const auto size=file.tellg();
 if(size<0||size>1024*1024)throw std::runtime_error("local handoff exceeds 1 MiB");
 std::string data(static_cast<std::size_t>(size),'\0');
 file.seekg(0);
 if(!file.read(data.data(),static_cast<std::streamsize>(data.size())))
  throw std::runtime_error("cannot read session file");
 return parseHandoff(data);
}
inline Handoff loadHandoff(const std::string& path){return loadHandoff(std::filesystem::u8path(path));}
inline std::size_t lyricIndexAt(const Handoff& h,std::uint32_t positionMs){
 if(h.lines.empty())return h.lines.size();
 const std::int64_t adjusted=static_cast<std::int64_t>(positionMs)-h.offsetMs;
 auto it=std::upper_bound(h.lines.begin(),h.lines.end(),adjusted,
   [](std::int64_t at,const HandoffLine& l){return at<l.milliseconds;});
 return it==h.lines.begin()?h.lines.size():static_cast<std::size_t>((it-h.lines.begin())-1);
}
}
