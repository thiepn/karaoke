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

## A3/A4 local practice session export (owner-initiated)

Use **Practice → Save local practice session (Ctrl+S)** from the native karaoke window, or **save PATH.rtk** in the console, to explicitly create a **new** `.rtk` file with the original audio **filename only**, title, current frozen/queued song position, bounded lyric offset, original timed lyrics and active A–B loop. Uses the same `ROOMTONE_HANDOFF_V1` schema and parser as the browser handoff; native source records, audio bytes, absolute audio paths, recordings, endpoint identifiers and credentials are never exported. A plain whole-song repeat is not represented as an A–B loop because that field does not exist in the original format. Canceling exports nothing. Existing files are never overwritten: select a new filename if the path exists. GUI/console export silences mic/music and joins the existing callback before collecting a snapshot; a previously ready device can be reopened **MUTED and PAUSED**, while a previously disconnected/interrupted device stays offline. This is an explicit local action, not autosave or background progress synchronization.

Re-import the `.rtk` with the same original local instrumental to resume. Canonical serialization is checked by the original parser before storage. Portable regression tests validate roundtrip, UTF-8, timeline, lyric and loop identity and invalid-source rejection. Actual Windows Save dialog user journeys, filesystems, hardware and screen readers still need owner qualification.

### Software-only export acceptance status

Exact-head CI now exercises the canonical original .rtk serializer on Windows/Linux, checks saved UTF-8 and original timestamp/offset/loop contracts, and uses a genuine Windows temporary file with `CREATE_NEW` to verify file bytes, refuse an existing destination, and retain its contents unchanged. GUI export is restricted to .rtk filenames and confirms imports with a metadata preview before overwriting local in-memory lyric/practice state. A sub-millisecond A–B loop just over 250 ms is rounded outward to fit the existing integer-millisecond handoff contract. These checks do not simulate live QuadCast/Pebble I/O, actual user file-dialog clicks, feedback or latency. A4 physical release gate remains NO GO.

## A3/A4 native owner-selected song queue (software-only draft)

The Windows-native **Songs** menu now supports a bounded **volatile, in-memory queue** of up to 24 explicitly selected local songs using the existing **one** miniaudio/WASAPI mixer and existing offline MP3/WAV/FLAC decoder. **Ctrl+A** adds another local audio file without playing it; **Ctrl+Q** opens a keyboard-driven, native list of filenames (not private full paths); **Ctrl+N** and **Ctrl+P** select the next/previous queued song manually. There is deliberately **no autoplay**, automatic background decoding, server, library database, native browser playlist mirroring, or recording takeover. Explicit **Change song (Ctrl+O)** starts a fresh one-song queue, clearing the former listening context.

Switching songs first mutes the microphone and backing music. The new local file is decoded outside the callback; decode failure leaves the current original PCM, lyrics, practice session and queue index intact while audio stays muted/paused. After successful decoding, the old native WASAPI callback is stopped and joined, PCM is replaced in the same engine, and **previous song's lyrics, handoff, A–B markers, repeat and seek state are cleared**. The newly selected song starts with monitoring MUTED and music PAUSED and never silently reconnects an already interrupted/unavailable device. Manual Reconnect remains necessary after hardware failure. Local file paths exist only in volatile process memory; UI shows basenames and queue ordinal, no network access or disk catalog is created. The current track's original filename is still the sole authority for explicit .rtk export/import matching.

The device-selection modal now honors owner-window-local **F8 mute**, like the timed-lyrics and song-queue dialogs. It is **not** a system-global shortcut or certified feedback protection; physical microphone mute/speaker power remain essential for real acoustic feedback.

Automated Linux/Windows queue unit tests cover bounded lists, duplicate path handling, owner-only commit and rollback on invalid selection. Synthetic Win32 tests cover menu commands, queue chooser cancellation, F8 modal dispatch and non-global quit behavior; CI cannot establish actual sound, physical device recovery, installed screen-reader behavior or measured latency. **A4 hardware acceptance remains NO GO.**

## A3/A4 in-process per-song practice continuity and owner queue management

The original WASAPI decoder, mixer and callback remain the only native audio authority. The Songs menu now supports explicit Move up, Move down, Remove INACTIVE queued song and Clear pending entries (keeping the currently loaded source), all without touching original audio/lyric files. Removal and clearing need explicit owner confirmation; the current PCM source cannot be removed. The native keyboard-accessible modal has action-specific headings, Escape cancellation and focused F8 mute. Reordering adjusts the active queue index without decoding/replacing PCM.

A manual queue switch mutes voice/music, decodes a newly selected local song, and only after successful decode joins the original callback and freezes pending seek/restart. The departing entry keeps its own lyrics, imported lyric offset, current frame position, repeat/A–B state, title and practice start marker in volatile memory. On revisiting that original full pathname, only its corresponding practice is restored. The restored music and microphone remain PAUSED/MUTED; an interrupted device stays offline until the owner manually reconnects. Explicit Change Song still resets the entire queue.

To avoid stale practice after the original audio file changes under the same pathname, the queue retains a source-version pair: file size and OS last-modified timestamp. It verifies the version before/after decoding and refuses to attach an old practice snapshot if either changed. This is a best-effort version check, NOT cryptographic authentication: hostile replacement that preserves both metadata fields is outside its guarantee. Paths that share a basename retain separate practice slots keyed to the full owner-selected local path; private absolute paths never leave process memory, appear in the displayed queue or enter the existing .rtk handoff. No database, cloud, recording engine, automatic playback, background autosave or network service was added.

Native CTest exercises queue preservation, ownership, reordering, removal, clear and changed-file invalidation; Windows synthetic UI exercises owner menu commands, focused F8, cancel and keyboard handling. Real mic/speaker tests, actual UI ergonomics, physical USB interruptions, feedback and driver timing remain NOT_TESTED: A4 NO GO.

## A3/A4 original DSP completion and manual audio recovery (software-only)

The native window's **Effects** menu now exposes the original, formerly console-only VocalDsp **echo feedback +/-2 percentage points** (source bounds 0-35%) and **echo delay +/-20 ms** (source bounds 80-550 ms). These directly update the original realtime callback's existing bounded atomic parameters; no second mixing thread, invented audio engine, hardware latency measurements, cloud preset storage or recording logic was introduced. The prior reverb, echo amount, vocal gain and music-level buttons remain unchanged.

**Dry vocal comparison (Ctrl+5)** explicitly zeroes reverb, echo and feedback without raising vocal gain or changing music volume; **Restore initial startup effects (Ctrl+6)** reapplies the original argument-selected bounded settings, *first muting microphone and pausing backing music* to avoid an unexpected live increase. **Current original DSP levels (Ctrl+E)** displays reverb, echo, feedback, delay, voice gain and music level with their bounded ranges; both paths are muted/paused before opening this modal to avoid hiding the window's local F8 mute key. Increasing feedback while the microphone is live first mutes monitoring. Settings are session-only original runtime parameters, not saved in another store or asserted to be calibrated for QuadCast/Pebble hardware.

The new **Audio** menu provides **Device health / recovery steps (Ctrl+H)**, owner-only Choose devices (Ctrl+D), manual Reconnect (Ctrl+R), and F8 mute. The diagnostic dialog displays the current READY / INTERRUPTED / OFFLINE state, selected input/output display names, source playhead and last failure message, with explicit steps to physically mute, check cables and Windows audio selection, reauthorize any changed endpoint and reconnect separately. It shows neither private WASAPI endpoint IDs nor audio/recording bytes and never initiates a reconnect itself. The GUI's local .rtk/.lrc import now respects a previously offline/interrupted driver: imports can update lyrics and position offline, but may reopen devices **only if the driver was already active and not interrupted**. No automatic fallback, audio unmute, account or network relay.

The device guide is NOT physical acceptance. Native synthetic UI and portable parameter unit tests cannot prove actual latency, feedback, real USB unplug/replug, screen-reader operation or the Windows dialog keyboard experience. **A4 remains NO GO.**

## Original lyric-cue rehearsal sections (software-only A3)

The native **Practice** menu adds **Select A-B from authored lyric lines (Ctrl+7)**, **Pause and seek to A (Ctrl+8)** and **Pause and seek to B (Ctrl+9)**. After manually importing matching original .rtk or .lrc timed lyrics, the owner can select any start line and a later original source line (or the instrumental's actual song end) using the existing keyboard-accessible native list. Each list shows the original lyric text, 1-based authored index and adjusted clock time. The two pickers and final explicit confirmation run **with microphone MUTED and music PAUSED**; rejecting/canceling either selection or choosing invalid/collapsed timestamps retains the previously defined A-B markers.

Native section bounds come only from the original handoff timestamps, the signed user-adjusted lyric offset and the existing 48-kHz track duration. Duplicate/tied timecodes, ranges <=250ms and out-of-bounds ranges fail closed under the existing greater-than-250ms practice policy. Confirming a valid section replaces only the single original A/B range; it does NOT enable repeat, start music, enable monitoring, edit original lyrics, write a file or create a second progress engine. The existing A-B, native transport, .rtk session export and volatile per-song queue continuity carry the result forward as before. Marker navigation queues a frame-exact seek in the original transport, with both audio paths first paused/muted; it does not auto-resume.

Portable source tests exercise authored indices, changed/signed lyric offsets, ties, source/EOF boundaries, the >250ms threshold and safe marker navigation. Synthetic native Windows UI tests exercise the new Practice menu commands and a real Win32 LISTBOX selection/cancel path; this is not real QuadCast/Pebble, screen-reader or driver qualification. Physical A4 remains NO GO.

## Native reading-first lyric focus mode (A3; synthetic acceptance only)

The SAME original Windows-native karaoke window has a **View → Large-lyric focus view (Ctrl+F)**. It enlarges the **current original source-owned lyric** and retains the previous/next line, seek slider, explicit music Play/Pause, owner Reconnect, current device/voice/music/queue status, notices, and a continuously visible **MUTE VOICE NOW (F8)**. Dense effect and management buttons are temporarily hidden, but existing native menu commands and local keyboard shortcuts remain available. This is **not** fullscreen and does not duplicate the lyric clock, playback engine, database, account, recording or storage.

**View → Enlarge / Reduce current lyric (Ctrl++ / Ctrl+-)** adjust only the displayed Windows font in three bounded sizes, including in normal mode. Toggling focus back restores all existing controls and the normal layout. If a formerly focused button becomes hidden, keyboard focus moves to the still-visible seek slider rather than trapping Tab/F8. Changes are in-memory window presentation only; music, voice and song progress are not changed just by toggling readability. The original source of the visible lyric remains the same imported .rtk/.lrc and 48-kHz transport. Existing UI text updates continue only on actual value changes, avoiding unnecessary screen-reader notifications.

Portable geometry tests cover client dimensions from 570x600 through 1440x1200 and source-font bounds. Synthetic real Win32 acceptance resizes the focused view to a 620x700 outer window and checks current lyric, transport, safety notices and panic mute fit the client while dense controls stay hidden. These tests **cannot prove** actual NVDA/Narrator reading order, long-line readability, Windows system magnifier / font scaling, physical USB routing, or actual acoustics. A4 remains NO GO.

## Native owner-held WASAPI suspension and guarded recovery (software-only)

The existing Audio menu now offers **Stop duplex audio; retain playhead (F7 or Ctrl+U)**. This is deliberately different from F8, which mutes microphone monitoring only. F7 mutes **both** paths, stops/joins the ORIGINAL WASAPI duplex callback, releases its device resources, resets the original vocal DSP's echo buffer, freezes any outstanding owner seek/rewind and keeps the current original audio PCM, lyrics, 48-kHz position, A–B markers, queue and per-song local practice **in memory**. The visible native status displays **OWNER SUSPENDED**. USB notifications may show devices again, but never automatically reopen a suspended duplex path. This control is local to the focused native window, not a system-wide hotkey and not a physical emergency mute. Existing original browser A1/U21 recording paths remain separate.

**Owner reconnect (Ctrl+R)** now presents a visible, explicit YES/NO preview with selected microphone, selected speaker, current local playhead, physical-mute warning and endpoint identity policy. Default is **No**. Cancel leaves the driver as it is and keeps monitoring/music muted; Yes joins/stops any previous callback and asks the **one original WASAPI engine** to reenumerate and reconnect, reusing the original private in-memory endpoint identity pins and refusing an unapproved same-name replacement. Successful reconnection starts **MUTED/PAUSED**, not automatically singing or recording. Real driver correctness is NOT hardware qualified.

**Choose devices (Ctrl+D)** now distinguishes three outcomes after its original fresh post-dialog WASAPI enumeration: (1) a freshly revalidated owner-approved endpoint pair authorizes muted reopening and resets endpoint pins, (2) canceled selection may reopen the previous device ONLY when it was actually READY before the picker; cancel while OFFLINE/INTERRUPTED/OWNER SUSPENDED **never** reopens it, and (3) rejected, missing, ambiguous or changed endpoint candidates leave the driver OFFLINE with the earlier owner identity intact until another explicit action. The original song, lyrics, pending seek and queue are preserved in each case. No automatic fallback, cloud service, background reconnection or production data write.

Portable policy tests cover suspension states, choice authorization, canceled/offline refusal and revalidated pair behavior. Synthetic Windows GUI tests exercise Audio suspension commands and focused-slider F7 handling without opening real WASAPI. Chromium browser routing CI was additionally made to wait until asynchronous original gain gates have settled before asserting the stable direct-route state (no browser production logic changed). **A4 physical NO GO**.

## Owner-local live microphone/output signal check (A3, NOT acoustic qualification)

The native **Audio > Live local signal check (Ctrl+M)** opens a dedicated Windows diagnostics view backed solely by numeric per-buffer peaks published by the original 48-kHz WASAPI duplex callback. No second capture pipeline, recording, waveform files, network service, persisted meters or browser A1/U21 changes are introduced.

The read-only panel reports **input-capture peak** and **duplex-output peak** as bounded percentages of *digital full-scale sample amplitude*, alongside original WASAPI **READY / INTERRUPTED / OFFLINE / OWNER SUSPENDED** state, voice monitoring and music status. It flags input samples near digital clipping (98%+) or original output limiter activity (69.9%+). Input activity can exist even when monitoring is muted because the already-selected duplex capture remains open; the panel **mutes microphone monitoring before appearing** and never enables it. Owner can press the modal's visible **MUTE VOICE (F8)** or **STOP DUPLEX (F7)** at any point, including when its controls have keyboard focus. Stopping the driver clears the stored transient numeric peaks to zero, leaving original source/playhead/lyrics/queue unchanged. Closing the diagnostics panel never restarts capture or unmutes voice.

**Digital peaks are NOT sound pressure, calibrated loudness, acoustic feedback detection, device output identity, input-to-speaker latency or a recording.** No real external microphone/USB device was accessed during CI. Portable CTest covers finite/bounded observations, near-clipping and output-limiter thresholds, callback ownership and zeroing. Synthetic Win32 CTest checks the actual owner-local modal, numeric text, focused F7/F8 and no global app quit. Physical A4 remains NO GO.

## Native per-song rehearsal bookmarks — implementation-only, no A4 qualification

The original Windows **Practice** menu now supports up to **12 local rehearsal bookmarks per original song**, distinct from the single A–B loop: **Ctrl+B** marks the current 48-kHz frame; **Ctrl+J** opens the original keyboard-accessible native list to **pause and jump**; **Ctrl+K** removes a selected bookmark after an explicit default-No confirmation; Practice → **Clear this song's rehearsal bookmarks** also requires confirmation. The chooser displays local source time and, where original timed lyrics were imported, the contemporaneous original lyric text. There are **no audio bytes, absolute paths, recordings, online services or new persistent files**.

Positions remain sorted, reject invalid/EOF frames, reject a near-duplicate within 100 ms and cap memory at 12. Marking does not change the audio output, and honors an owner seek/restart pending in the original callback. Browsing, jumping, removing and clearing mute the microphone and pause music before entering a modal, and jumping requests the ORIGINAL transport to move to an exact bookmarked frame without auto-starting playback or the microphone. Canceling retains the current cue list and original source playback state.

All bookmarks are held **solely** in the existing queue-owned per-song **volatile practice** snapshot. Switching to a different queued audio file clears the active marker list, and returning to the previous owner-picked full path restores its markers only if the previous source version still matches; a changed same-path audio file does not inherit stale bookmarks. Explicit Change song resets that queue as before. Existing .rtk export intentionally does **not** serialize bookmark arrays: the canonical original session schema and original browser A1/U21 recording remain untouched. All saved original lyrics, A–B ranges, DSP, focus view and WASAPI routing retain their original engines. Synthetic CTest and Windows UI evidence does **not** qualify real output, feedback, manual accessibility or physical A4 release.

## Owner-operated variable backing speed — original 48 kHz mixer (A3 software only)

The native **Backing speed** menu now offers **Slower (Ctrl+T)**, **Faster (Ctrl+Y)**, **Original 100% (Ctrl+G)**, and an explicit pitch/rate explanation. Playback rate is bounded **60–120% in 10% UI steps**; default 100%. The original console exposes `speed 0.80` or `speed 1.20` within the same strict bounds. Speed changes first **MUTE microphone monitoring and PAUSE backing music** to avoid surprise playback/pitch changes. The owner must explicitly restart each path. The controls do not reopen a suspended or interrupted driver.

This uses **linear interpolation over the original decoded stereo instrumental** inside the *existing single miniaudio/WASAPI callback*, not a second audio engine, stream, timer or decoder. This is **not pitch-preserving time stretching**: slowing a track lowers its *musical pitch*, speeding it raises pitch, and the result is not suitable when maintaining the original song key is required. The original microphone capture, vocal DSP, effects and output render sample rate remain exactly **48 kHz** and are **NOT retimed**. The callback still has no allocations, locks, disk reads, network or recording writes. The original song position, lyric timestamps, A–B ranges and section bookmarks remain in **source frames**, not wall-clock playback seconds; at 60% speed a 60-second original section takes about 100 seconds in real time. None of these computed durations are claims of measured hardware latency.

Speed is kept only in the same **queue-owned volatile per-song practice snapshot** that already owns bookmarks, lyrics, A–B markers and position. Moving to another verified original path restores only that path's selected speed, and decoding a modified song invalidates all prior cached practice including speed. Explicit New Song starts at 100%. The existing canonical `.rtk` serialization and browser A1/U21 recording authority are unchanged; exporting an `.rtk` does **not** include native speed. No persistent preset library, browser bridge, microphone resampling, cloud account or migrations were added.

Portable tests cover original exact 1.0x behavior, bounded 0.6–1.2 speed, stereo interpolation, EOF, seek, paused fade, changed file/queue ownership, NaN/Inf rejection and **unaltered live vocal DSP at different backing speeds**. Real audio quality, interpolation artifacts, subjective timing, singing in tune, hardware USB routing, NVDA/Narrator and screen scaling remain NOT_TESTED; **A4 remains NO_GO**.

CLI `open`/`load` also resets rate to **100%** on an explicitly new original source. Speed is never allowed to bleed across unrelated tracks even in console mode.

## Native per-line lyric timing correction (source-owned; A3 software only)

The original Win32 **Practice** menu now corrects a **single imported timed lyric line** without changing the global offset, source text, playback, or existing A–B/bookmark frames:

- **Ctrl+Shift+Left**: select one original lyric and move its source timestamp **100 ms earlier**.
- **Ctrl+Shift+Right**: select one original lyric and move its source timestamp **100 ms later**.
- **Ctrl+Shift+Enter**: select one original lyric and **align its timestamp to the owner playhead captured at command time**. The original signed global offset is subtracted when converting that original song frame to the canonical source timestamp.
- **Ctrl+Z**: undo the **most recent** individual timestamp correction for this source. Undo is one step, not a permanent history timeline.

Each operation first mutes microphone monitoring and pauses music before opening the existing keyboard-accessible Win32 lyric list. Select one source line, review its old/new source timestamps in milliseconds, and explicitly confirm **Yes** (default is **No**). Escape or a rejected/invalid correction changes nothing. The policy rejects source timestamps outside 0–720000 ms, after original song end, negative offset-adjusted times, a cue overtaking its source neighbors, or no-op changes. Equal-timestamp adjacent original lines remain legal per existing canonical V1 schema; the original ordered line IDs and texts are never sorted or silently rewritten.

Confirmed corrections alter only the **local in-memory original `Handoff.lines[index].milliseconds`** and become immediately visible in original previous/current/next timed lyrics, browsing and source cue-based section selection. The original source filename, global lyric offset, playback position, queued bookmarks and existing A–B frame markers do not change. No automatic microphone/music playback, WASAPI restart or data migration occurs. The one-level undo is held in the **same per-song volatile queue practice state**; switching to a different owner-selected song preserves each original source's separate undo and edits, while a changed source version or a fresh .rtk/.lrc import invalidates obsolete undo. **Ctrl+S** is still the only explicit route to export corrected lyrics into a **new, non-overwriting local .rtk file** using the unchanged canonical `ROOMTONE_HANDOFF_V1` format. No edits to source LRC files, no cloud store, no recording transfer.

Automated portable tests cover timestamp monotonicity, signed offsets, source duration bounds, original-line preservation and canonical .rtk roundtrip, with synthetic Win32 menu-command and existing list-modal tests. Real Windows owner interactions, screen-reader behavior and audio latency are NOT_TESTED. **A4 physical NO GO**.
