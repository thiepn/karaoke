#pragma once
#include <algorithm>
#include <cstdint>
namespace roomtone::practice {
// Native practice uses the same 48-kHz transport and >250 ms A-B validity
// threshold as the browser Songs editor. No files, audio buffers or threads.
inline constexpr std::uint64_t MinLoopFrames=12000;
inline bool validAB(std::uint64_t start,std::uint64_t end,
                    std::uint64_t duration) noexcept{
 return start<=duration&&end<=duration&&end>start&&
        end-start>MinLoopFrames;
}
inline std::uint64_t markAt(std::uint64_t published,std::uint64_t duration) noexcept{
 return std::min(published,duration);
}
inline int adjustOffset(int original,int delta) noexcept{
 return std::clamp(original+delta,-30000,30000);
}
}