# ROOMTONE A3 — Experimental Windows-native karaoke

Unmerged Windows WASAPI console prototype. One full-duplex callback mixes live microphone capture and a predecoded local stereo backing track. The original immediate dry vocal DSP now provides separately adjustable short-room reflections and repeating echo. This is not a replacement for, nor a bridge to, browser ROOMTONE.

- Microphone starts MUTED; backing track starts PAUSED.
- Original WASAPI shared-mode 48 kHz duplex device remains the single processing engine. Buffers of 120/240/480 frames are only requested sizes, NOT measured latency.
- Local MP3, WAV or FLAC song is predecoded through pinned miniaudio before opening devices, bounded to 12 minutes, with no allocation, I/O or mutex in callback.
- Live settings: vocal gain, reverb wet level, echo level/delay/feedback, backing-track volume. Local stereo playback: play/pause/rewind. Output is hard-bounded, not feedback protected.
- Audio is never uploaded, saved, or silently recorded. Existing browser recording and A1 routing remain separate and unchanged.

## Run

Windows 10/11, Visual Studio C++ tools, CMake and Git required for initial pinned miniaudio FetchContent.

    git fetch origin feat/a3-native-audio-prototype
    git switch --detach <exact commit>
    cmake -S native/a3 -B build/a3 -A x64
    cmake --build build/a3 --config Release
    ctest --test-dir build/a3 -C Release --output-on-failure
    .\build\a3\Release\roomtone_a3_native.exe --list
    .\build\a3\Release\roomtone_a3_native.exe --song "C:\Music\backing.mp3" --gain 0.10 --wet 0.12 --echo 0.15 --echo-ms 280 --music 0.2

Leave --input and --output unset to use system defaults, or supply their indices from --list.

- Command m + Enter: toggle vocal monitoring (default OFF).
- Command p + Enter: toggle song playback (default PAUSED).
- Command r + Enter: rewind and play. Command status: show approximate callback-position.
- Commands wet 0.15 / gain 0.10 / echo 0.18 / feedback 0.12 / echo-ms 280 / music 0.20: adjust immediately on next processing block, within guarded limits.
- Command q: quit.

You can check decoding without opening the speaker or microphone device:

    .\build\a3\Release\roomtone_a3_native.exe --probe-song --song "C:\Music\backing.mp3"

## What this does NOT provide

No browser queue/lyrics sync, external audio/YouTube interception, automatic cross-process bridge, GUI, direct microphone recordings or guaranteed karaoke latency. Browser ROOMTONE continues to own its existing local recording UI. The native prototype provides a separately loaded local song only.

## Physical owner testing — NOT TESTED

First try wired headphones and compare immediate dry audio (--wet 0 --echo 0) against QuadCast direct monitoring. Avoid hearing both simultaneously. Then test reverb and echo separately, backing balance, pause/rewind and 5-minute stability.

Only after headphone safety is confirmed, try Pebble V3 over USB/AUX at very low physical volume. Place speakers away from the QuadCast cardioid pickup and physically mute QuadCast or power down speakers if feedback occurs. The command-line toggle is NOT an emergency mute; there is no automatic acoustic feedback detector. Actual microphone-to-speaker latency must be measured on real hardware, not inferred from buffer size or synthetic CI.

Linux/Windows CI covers DSP and mixer invariants, Windows native compilation and offline WAV decoder fixture. It does not prove low-latency operation on your real devices.
