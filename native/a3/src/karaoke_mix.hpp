#pragma once
#include "vocal_dsp.hpp"
#include "practice_speed.hpp"
#include <cmath>
#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>
namespace roomtone {
// Local track PCM is predecoded before the audio device starts.
// The render callback performs no allocation, locks or disk I/O.
class KaraokeMix {
public:
 explicit KaraokeMix(std::vector<float> song={},float wet=0.12f,float gain=0.18f)
  : vocal(wet,gain),song_(std::move(song)) {}
 VocalDsp vocal;
 bool hasSong()const noexcept {return !song_.empty();}
 bool songEnded()const noexcept {return hasSong()&&position_>=static_cast<double>(songFrames());}
 std::size_t songFrames()const noexcept {return song_.size()/2;}
 std::size_t position()const noexcept {
  return static_cast<std::size_t>(std::min(position_,static_cast<double>(songFrames())));
 }
 // Called only on the audio callback thread, never concurrently with render().
 // A 20ms fade-in after seeking avoids a hard discontinuity in backing music.
 void seekFrames(std::size_t frames) noexcept {
  position_=std::min(frames,songFrames());
  musicEnvelope_=0;
 }
 void rewind() noexcept {seekFrames(0);}
 // Only replace after the audio device/callback is fully uninitialized.
 void replaceSong(std::vector<float> next) noexcept {song_=std::move(next);seekFrames(0);}
 // Playback-rate changes ONLY original backing track read positions. Uses
 // bounded linear interpolation with no allocation, IO or locks in callback.
 // At 1.0x the original integer-index stereo PCM path is exact.
 Stereo render(float microphone,bool voiceOn,bool musicOn,float musicGain,
               float playbackRate=practice_speed::Normal) noexcept {
  const float targetVoice=voiceOn?1.0f:0.0f;
  const float targetMusic=musicOn&&hasSong()&&!songEnded()?1.0f:0.0f;
  voiceEnvelope_=ramp(voiceEnvelope_,targetVoice);
  musicEnvelope_=ramp(musicEnvelope_,targetMusic);
  const Stereo voice=vocal.process(microphone);
  float left=0,right=0;
  if(musicEnvelope_>0&&!songEnded()){
   const auto index=static_cast<std::size_t>(position_);
   const auto next=std::min(index+1,songFrames()-1);
   const float fraction=static_cast<float>(position_-static_cast<double>(index));
   left=song_[2*index]+(song_[2*next]-song_[2*index])*fraction;
   right=song_[2*index+1]+(song_[2*next+1]-song_[2*index+1])*fraction;
   position_=std::min(position_+static_cast<double>(practice_speed::safe(playbackRate)),
                      static_cast<double>(songFrames()));
  }
  const float level=std::clamp(std::isfinite(musicGain)?musicGain:0.0f,0.0f,0.5f);
  return {bound(voice.left*voiceEnvelope_+left*level*musicEnvelope_),
          bound(voice.right*voiceEnvelope_+right*level*musicEnvelope_)};
 }
private:
 static float ramp(float current,float target) noexcept {
  constexpr float step=1.0f/960.0f;
  return current+std::clamp(target-current,-step,step);
 }
 static float bound(float x) noexcept {
  return std::clamp(std::isfinite(x)?x:0.0f,-0.7f,0.7f);
 }
 std::vector<float> song_;
 double position_=0;
 float voiceEnvelope_=0,musicEnvelope_=0;
};
}
