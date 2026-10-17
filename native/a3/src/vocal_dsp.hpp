#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
namespace roomtone {
struct Stereo { float left; float right; };
class VocalDsp {
public:
 static constexpr unsigned kSampleRate=48000;
 static constexpr std::size_t kCapacity=4096,kEchoCapacity=48000;
 explicit VocalDsp(float wet=0.12f,float gain=0.18f) noexcept { setParameters(wet,gain,0,0.15f,280); }
 void setParameters(float wet,float gain,float echo,float feedback,unsigned echoMs) noexcept {
  wet_=std::clamp(std::isfinite(wet)?wet:0.0f,0.0f,0.25f);
  gain_=std::clamp(std::isfinite(gain)?gain:0.0f,0.0f,0.35f);
  echo_=std::clamp(std::isfinite(echo)?echo:0.0f,0.0f,0.35f);
  feedback_=std::clamp(std::isfinite(feedback)?feedback:0.0f,0.0f,0.35f);
  echoFrames_=std::clamp(static_cast<std::size_t>(echoMs)*kSampleRate/1000,std::size_t{3840},std::size_t{26400});
 }
 void reset() noexcept {
  for(auto& d:delays_)d.fill(0);
  positions_.fill(0);damped_.fill(0);
  echoBuffer_.fill(0);echoPosition_=0;previousInput_=0;highpass_=0;
 }
 Stereo process(float input) noexcept {
  if(!std::isfinite(input))input=0;
  input=std::clamp(input,-1.0f,1.0f);
  const float hp=input-previousInput_+0.99f*highpass_;
  previousInput_=input;highpass_=hp;
  const float a=tap(0,hp,1637),c=tap(1,hp,2399);
  const float d=tap(2,hp,1823),e=tap(3,hp,2741);
  const std::size_t read=(echoPosition_+kEchoCapacity-echoFrames_)%kEchoCapacity;
  const float repeated=echoBuffer_[read];
  echoBuffer_[echoPosition_]=std::clamp(hp*0.30f+repeated*feedback_,-1.0f,1.0f);
  echoPosition_=(echoPosition_+1)%kEchoCapacity;
  return {limit((hp+wet_*(a+c)*0.5f+echo_*repeated)*gain_),
          limit((hp+wet_*(d+e)*0.5f+echo_*repeated)*gain_)};
 }
private:
 float tap(std::size_t which,float x,std::size_t length) noexcept {
  auto& buffer=delays_[which];auto& pos=positions_[which];
  const float old=buffer[pos];
  damped_[which]=old*0.35f+damped_[which]*0.65f;
  buffer[pos]=x*0.24f+damped_[which]*0.26f;
  pos=(pos+1)%length;return old;
 }
 static float limit(float x) noexcept {return std::clamp(std::isfinite(x)?x:0.0f,-0.35f,0.35f);}
 std::array<std::array<float,kCapacity>,4> delays_{};
 std::array<std::size_t,4> positions_{};
 std::array<float,4> damped_{};
 std::array<float,kEchoCapacity> echoBuffer_{};
 std::size_t echoPosition_=0,echoFrames_=13440;
 float previousInput_=0,highpass_=0,wet_=0.12f,gain_=0.18f,echo_=0,feedback_=0.15f;
};
}
