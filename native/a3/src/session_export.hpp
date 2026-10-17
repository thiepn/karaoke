#pragma once
#include "handoff.hpp"
#include "lrc.hpp"
#include "practice.hpp"
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace roomtone::session_export {
// The browser's ROOMTONE_HANDOFF_V1 text schema is the sole authority.
// Every export is a user-requested, local, bounded, filename-only snapshot.
inline std::string percentEncode(const std::string& input){
 if(!validUtf8(input))throw std::invalid_argument("session text must be valid UTF-8");
 const char digits[]="0123456789ABCDEF";
 std::string result;result.reserve(input.size());
 for(unsigned char c:input){
  if(c==0||c==10||c==13||c==27||c==127)
   throw std::invalid_argument("session field has unsafe control");
  if((c>='a'&&c<='z')||(c>='A'&&c<='Z')||
     (c>='0'&&c<='9')||c=='-'||c=='_'||c=='.'||c=='~')result.push_back(static_cast<char>(c));
  else{result.push_back('%');result.push_back(digits[c>>4]);result.push_back(digits[c&15]);}
 }
 return result;
}
inline std::string encode(const Handoff& h){
 std::string data="ROOMTONE_HANDOFF_V1\n";
 const auto field=[&data](const char* name,const std::string& value){
  data+=name;data+='\t';data+=value;data+='\n';
 };
 field("audio",percentEncode(h.audioName));
 field("title",percentEncode(h.title));
 field("start-ms",std::to_string(h.startMs));
 field("offset-ms",std::to_string(h.offsetMs));
 field("loop",h.loop?"1":"0");
 field("loop-start-ms",std::to_string(h.loopStartMs));
 field("loop-end-ms",std::to_string(h.loopEndMs));
 for(const auto& line:h.lines){
  field("lyric",std::to_string(line.milliseconds)+"\t"+percentEncode(line.text));
 }
 if(data.size()>1024*1024)throw std::invalid_argument("session exceeds 1 MiB");
 // Validate the exact bytes against the existing canonical reader before
 // any attempt to write. Never silently normalize the original contract.
 (void)parseHandoff(data);
 return data;
}
inline Handoff snapshot(const Handoff& source,const std::string& audioFilename,
 const std::string& displayedTitle,std::uint64_t requestedFrames,
 std::uint64_t durationFrames,bool repeatEnabled,
 std::uint64_t loopStartFrames,std::uint64_t loopEndFrames,bool validRange){
 Handoff h=source; // original timed lyric IDs, text and timestamp order kept
 h.audioName=audioFilename;
 if(h.title.empty())h.title=displayedTitle.empty()?audioFilename:displayedTitle;
 constexpr std::uint64_t rate=48;
 h.startMs=static_cast<std::uint32_t>(
  std::min<std::uint64_t>(std::min(requestedFrames,durationFrames)/rate,720000));
 const bool useLoop=repeatEnabled&&validRange&&
  practice::validAB(loopStartFrames,loopEndFrames,durationFrames);
 h.loop=useLoop;
 if(useLoop){
  h.loopStartMs=static_cast<std::uint32_t>(loopStartFrames/rate);
  h.loopEndMs=static_cast<std::uint32_t>(loopEndFrames/rate);
 }else{
  // Preserve whole-track repeat as ROOMTONE_HANDOFF_V1 loop semantics.
  // The source contract has no separate whole-track repeat field, so
  // never misrepresent it as a user-marked A-B loop.
  h.loopStartMs=0;h.loopEndMs=0;
 }
 (void)encode(h); // fail before storage if user state violates the format
 return h;
}
}