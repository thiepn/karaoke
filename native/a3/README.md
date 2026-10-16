# ROOMTONE A3 — Windows-native local karaoke prototype

UNMERGED draft PR #7. Extends ORIGINAL miniaudio/WASAPI duplex engine and preserves the browser ROOMTONE app, A1 routing, local recordings and U21 UI.

## Native experimental usage

Double-click the Windows EXE with no arguments: a Unicode Windows song picker opens, then a microphone/speaker chooser. Canceling the picker exits without opening audio hardware. Local WAV/MP3/FLAC is decoded before audio startup (maximum 12 minutes). Voice starts MUTED and song starts PAUSED.

Options:
- --setup: guided microphone and speaker selection
- --pick-song: native Windows file dialog
- --song PATH: path to local instrumental
- --list: device enumeration
- --probe-song --song PATH: offline decode without opening WASAPI
- --period 120|240|480: requested callback frames, NOT measured latency

Commands (press Enter):
- m toggles voice, p toggles backing track, r rewinds/plays, q quits
- seek 30 = absolute seconds; seek +10 or seek -5 = relative; loop on/off = repeating track
- wet, gain, echo, feedback, echo-ms, music = existing bounded live vocal/music adjustments, e.g. echo 0.15
- open = pick another song; load C:\Music\Song.mp3 = load another local song (spaces supported)
- reconnect = explicitly reopen selected devices and preserve song position, always with voice and music muted
- status = callback-reported song position, repeat and device state

Song replacement: both paths mute, decoding happens outside real-time audio, the original WASAPI device is stopped and its callback joined, PCM replaced, and the SAME engine reopened MUTED and PAUSED. Canceled/invalid files retain previous song. On reroute/driver-stop notifications the app mutes; recovery after real device failures is UNVERIFIED.

## Build

Requires Windows 10/11, Visual Studio C++ tools, CMake and Git for pinned miniaudio revision.

    cmake -S native/a3 -B build/a3 -A x64
    cmake --build build/a3 --config Release
    ctest --test-dir build/a3 -C Release --output-on-failure

EXE: build/a3/Release/roomtone_a3_native.exe

## Scope and physical safety

This is NOT a browser-native control bridge. Webpages cannot safely launch/control arbitrary WASAPI processes without a designed authorized local transport layer. Browser songs, lyrics, YouTube playback and recordings do not synchronize with this prototype; original browser recording is unchanged.

Physical testing is outstanding: QuadCast/Pebble latency, real device hot-unplug, Windows file-picker clicks, singing audio quality and acoustic feedback. Start with wired headphones, not simultaneous QuadCast direct monitoring. Test Pebble V3 only over USB/AUX at low volume; point the cardioid pickup away and physically mute or turn speakers off if feedback occurs. CLI Enter is NOT an emergency mute. Buffer period is NOT measured latency.

See docs/A3_OWNER_HARDWARE_ACCEPTANCE.md. Do not release or merge without owner evidence.


## Browser-to-native offline handoff (no relay)

In ROOMTONE Songs, choose a LOCAL song and optionally edit its lyric lines and A–B loop. Select **Stop browser audio & download session**. The browser pauses music, mutes/stops browser microphone capture and downloads an `.rtk` text file. Browser playback stays locked until **Return to browser audio** is explicitly selected. Existing local recordings are not silently exported or erased; handoff refuses to interrupt an active recording.

On Windows, launch the A3 executable without arguments. Choose the same original song in the native dialog. Answer `y` to import a browser session and pick the downloaded `.rtk` file, or use `--song PATH --handoff PATH` or `--pick-song --pick-handoff`. The native program refuses a session whose audio filename does not exactly match the selected instrumental. It retains the browser-start position, lyric offset, timed lines and optional A–B loop, then starts **MUTED and PAUSED**. Commands `lyrics` and `next-line` use callback-reported song time for native console lyric navigation; no background relay, web controller or browser lyric auto-sync exists. `--probe-handoff --handoff file.rtk` validates locally before opening any audio device.

The `.rtk` carries no audio bytes, native filesystem path, recordings or credentials. It is user-triggered and must be transferred manually. Do not assume Windows effects or clock timing has been physically measured. The browser and native programs are separate; only one should own live audio at a time.
