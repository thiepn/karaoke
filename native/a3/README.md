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


## In-process Windows karaoke view (experimental; NOT released)

The SAME ROOMTONE A3 executable now supports an in-process Win32 lyrics and transport window. Running the program with no arguments opens the song picker, optional user-mediated local .rtk import prompt and microphone/speaker selector. It then starts the existing one WASAPI engine with VOICE MUTED and SONG PAUSED. Use --gui --song PATH --handoff SESSION_PATH for explicit GUI mode or --console --song PATH for original command-line mode.

The window displays imported timed lyric lines (previous/current/next) from native callback-reported position, song title, audio/driver status and separate effects levels. Controls include play/pause, restart, +/-10-second seeking, next lyric, repeat, reverb, echo and music levels, load a new local song, import a matching .rtk, choose devices, manual reconnect and explicit MUTE NOW.

All local file decoding and device lifecycle changes occur on the Windows owner/UI thread outside the realtime audio callback. Choosing another song clears previous source-owned lyrics. Reconnecting never unmutes the microphone or restarts music automatically. Enabling monitoring requires an explicit feedback confirmation. F8 (mute only), F9 (toggle monitoring with confirmation), and F10 (music toggle) are window-local keys, not system-global hotkeys. Physical QuadCast mute or turning off the speaker remains the reliable feedback emergency action.

No localhost listener, network service, new audio engine, connected-provider integration, shell execution, credentials or new recording engine exists. Original browser recording and A1 Web Audio continue independently. Browser queue/music/lyrics are NOT live synchronized with native view: only the user-triggered offline .rtk snapshot transfers position and timed lyric data. Real hardware latency and Windows physical device recovery remain NOT TESTED.

Owner qualification: first wired headphones, confirm initially muted/paused, test all lyric and transport controls; only later use Pebble V3 via USB/AUX at low volume. Confirm window layout, F8 panic mute, song replacement clearing lyrics, matching .rtk imports, manual selected-device recovery, 5-minute dropout performance, and physical QuadCast-to-Pebble dry voice delay. CI Win32 message/compile checks do not prove singing quality or acoustic feedback safety.


## A3 device recovery and accessible seek controls

- The one native Win32 karaoke window now has a focusable **Song position** slider between timed lyrics and transport controls. Click or use arrow keys, Page Up/Down, Home/End to seek within the existing WASAPI engine. Seeking never unmutes the microphone or restarts paused music, and is disabled while audio is interrupted.
- Canceling the device selector no longer posts a thread-wide quit event. The karaoke window is temporarily disabled while the modal chooser is open, then re-enabled and focus restored.
- USB/WASAPI device labels are snapshotted before opening selection. A named device is accepted only if unique, missing/duplicated/stale names are refused, and Windows default is chosen only when explicitly selected. Names are not stable endpoint IDs: a physically different device sharing the same unique name cannot be distinguished from its name alone. Physical device identity qualification remains outstanding.
- Reconnect stops the old native callback, preserves its latest song position including a pending seek/restart, then starts the same engine MUTED and PAUSED. Driver reroute/stop blocks monitoring pending explicit owner recovery. No automatic background retry or additional audio engine was introduced.
- Startup interruptions fail closed instead of clearing the interruption flag after the driver event. Missing or ambiguous device selection and failure to open/start WASAPI are shown as actionable window notices.
- Timed lyric labels and button captions update only when their value changes to reduce redundant accessibility text-change signals. F8/F9/F10 remain window-local. Physical QuadCast mute or speaker power is still the dependable emergency action.

CI tests the Win32 message/slider/selector lifecycle synthetically, offline decoding, portable C++ recovery policies and Chromium handoff. It does not test actual QuadCast/Pebble hardware, USB reconnect behavior, acoustic feedback or round-trip singing latency. Start with wired headphones.
## A3 owner-thread hotplug recovery and endpoint identity (unreleased)

After native Windows device topology notifications or miniaudio stop/reroute signals, the UI owner thread enumerates endpoints and safely drains/uninitializes an interrupted WASAPI callback before freezing requested seek/rewind and reported song position. Missing/duplicate selected names stop the device, rather than switching audio silently. A successful explicitly selected WASAPI input/output is now pinned to its OS endpoint ID in process memory; a different endpoint with the same human-readable name is refused on later reconnect until the owner uses **Select devices** again. The token is never logged, persisted, sent to the browser or exported. The OS may reassign an endpoint ID on a hardware reconnect: a changed ID requires explicit reselection. Pinning an OS endpoint ID does **not** prove physical device serial identity. Explicit Windows defaults remain Windows-controlled.

Song slider, +/-10-second seek, restart and timed previous/next lyric commands can queue local position while devices are offline. After a deliberate Reconnect the song remains PAUSED and the microphone MUTED; imported lyrics/offset, repeat and effect settings survive. Ctrl+Left/Right navigates imported timed lyrics; Ctrl+O/I/D/R opens song/session/device selection and reconnect. Shortcuts only affect the focused native window. No alternate audio engine, background retry, browser transport takeover or network connection was added.

## A3 native owner controls: selected devices, all-lyrics and compact viewport

The native Win32 device selector now opens on the currently selected input and output, not the Windows defaults. A disconnected or duplicate-name previous selection starts with **no valid selection**, and the owner must explicitly select a valid input/output. The chooser copies human-readable labels AND WASAPI endpoint IDs, then re-enumerates when the modal closes: a name match is not sufficient when an endpoint was replaced while the picker was open. The original endpoint authorization stays unchanged on failure, and both audio channels remain muted/stopped. Explicit Windows-default mode remains OS-owned, while named mode pins an endpoint ID only in memory. This is not a hardware serial-number identity claim.

The native window now uses a compact layout to expose controls on smaller Windows screens. With a matching user-imported .rtk session, **Browse all timed lyrics (Ctrl+L)** shows an accessible native list of timestamped lyrics, selection and cancellation. It seeks within the original song clock using the import offset and does not start music or microphone monitoring. A focused lyrics modal continues to honor F8 as a local emergency voice mute. These changes reuse the existing WASAPI mixer, source-owned session parser and control-thread transport. No network, persistence, duplicate audio engine or microphone recording was added. Screen-reader semantics, physical DPI scaling and genuine on-hardware latency remain NOT TESTED.

## A3/A4 source-owned standalone LRC import (software-only, unreleased)

Native Windows **Import .rtk / .lrc** supports the original browser Lyrics Studio's exported timestamped .lrc files directly, without requiring a browser handoff. The existing user-mediated .rtk preserves its full loop/offset/position contract. Standard LRC imports accept [mm:ss], [mm:ss.x], [mm:ss.xx], [mm:ss.xxx], multiple tags, [ti:] and [offset:] metadata following browser source behavior, sort timestamps and use the same native lyric display, previous/next control and Ctrl+L browser. Import retains an already selected song and, for lyrics-only replacement, its current position and A–B/repeat state. The microphone and music are muted/paused while importing and do not restart automatically. Imported lyrics are local-only and never written to a new storage engine.

CLI: **--lrc PATH --song SONG** loads matching local lyrics; **--probe-lrc --lrc PATH** parses without opening Windows audio, and **--pick-handoff** lets the owner choose .rtk or .lrc. Files must be valid UTF-8 with at most 500 timed lyric lines, 2000 bytes per line and 1 MiB total; unsafe control sequences and invalid timestamps fail closed. Native .lrc import does not verify that the lyrics belong to the instrumental: the owner must choose the correct source file. A filename-qualified .rtk still enforces exact audio filename identity. No recording, network relay, credentials or extra audio engine is introduced.

## A3/A4 native practice controls (software implemented; hardware acceptance NO GO)

The original Windows WASAPI transport now has a focused **Practice** menu (Alt+P). The owner can **Mark A (Ctrl+1)** at the current callback-reported/offline position, **Mark B (Ctrl+2)** at least 250 ms later, **Toggle A-B repeat (Ctrl+4)** without changing voice/mic consent, and **Clear A-B (Ctrl+3)**. Marking never turns music on, starts microphone monitoring, or enables a loop automatically. The original Repeat control still toggles whole-song repeat when no A-B range is marked. The practice menu also offers **lyrics 100 ms earlier (Ctrl+[)**, **100 ms later (Ctrl+])** and **restore imported offset (Ctrl+0)**; offsets are limited to ±30000 ms. The current lyric offset and A-B state appear in the native position label. Edits are deliberately session-only and are not written into original local files or the .rtk handoff.

The two A-B bounds/validity flag are atomically shared with the original realtime callback; the owner publishes disabled validity before changing frame bounds and republishes the completed valid range afterward. Imported .rtk loop metadata continues to populate the same transport. Native Linux/Windows policy tests cover loop bounds and offset clamping; Win32 synthetic tests exercise menu action dispatch. Physical audio dropout, timing continuity, mouse/keyboard UX and loop recording are NOT TESTED with real devices.
