# A3 real-hardware owner acceptance (NOT TESTED)

1. Start on Windows with wired headphones. Confirm song-picker cancel does not open audio hardware. Select intended microphone/output devices and verify startup voice-muted/song-paused.
2. Test QuadCast dry vocals (wet 0, echo 0) and compare subjective consonant timing to QuadCast direct headphone monitoring and browser A1.
3. Test wet room and echo separately, multiple echo timings, bounded gain/music balance and 5-minute stability. Requested 120/240/480 frames are NOT end-to-end delay measurements.
4. Test pause/resume, absolute/relative seek, loop, EOF, and new song selection using open/load while mic is muted. Verify new song restarts with mic and music muted.
5. Disconnect devices, then run reconnect. Log wrong/missing devices, driver failures and whether song position survives. Recovery is not guaranteed by CI.
6. Finally try Pebble V3 USB/AUX only at very low physical volume and QuadCast cardioid pickup away. For ringing/feedback physically mute QuadCast or power off speakers at once; no active feedback detector exists.
7. Report actual round-trip measurement method or subjective delay, driver settings, crackles, feedback, mic/track levels and real error logs. Browser recordings must be qualified in browser independently. Native prototype has no recording.

Remaining release blockers: physical hardware acceptance; native GUI UX validation; trustworthy browser/native transport architecture; signed binary/distribution decision. Release NO GO.


Native GUI physical checks: initial song and device dialogs; current/next lyrics under genuine playback; F8 mute-only; F9 confirmation; F10 music control; source-matched offline .rtk reload; no auto unmute after device recovery; same one WASAPI callback; visual accessibility. This has not been physically tested on QuadCast/Pebble V3.


## A3 recovery and slider acceptance — owner only / NOT TESTED

- Focus the native Song position slider by Tab. Check mouse drag/release, arrow keys, Page Up/Down and Home/End, and verify imported timed lyrics follow the selected position. Seeking must never enable microphone monitoring automatically.
- Open Select devices, cancel, and confirm the karaoke window remains running with the previously focused control usable. Test multiple input/output list changes while the chooser is open.
- Disconnect the chosen QuadCast/Pebble device. An interrupted state must visibly block F9 microphone monitoring, mute all native playback and require explicit reconnect; never silently substitute another device.
- Test missing/duplicate device names and explicit Windows-default selections. Different physical endpoints that share a unique display name remain an unsupported identity guarantee; collect real endpoint/driver evidence.
- While a local song is paused or playing, queue a seek/restart, then force a device interruption and reconnect. Verify the intended position survives while music and microphone remain PAUSED/MUTED. If recovery fails, record error message and whether the original device was available.
- Test USB and AUX on Pebble V3 with wired headphones first, very low volume and cardioid QuadCast aimed away from the speaker. Physical microphone mute or speaker power-off remains mandatory for acoustic feedback.

All physical recovery, measured latency and acoustic safety gates remain **NOT TESTED**. Release NO GO.
## A3 additional owner verification required (NOT TESTED)

- Disconnect and reconnect the selected QuadCast/Pebble. Confirm the UI explicitly requires Reconnect and does not automatically resume audio or the microphone; the song/lyrics position should survive.
- Replace the selected input/output with a different Windows endpoint using the same display name, or observe an endpoint ID reallocation. Verify that reconnect refuses it until manually choosing devices again. This is OS endpoint identity, not physical device serial-number validation.
- While offline, seek with slider/10-second controls and previous/next lyric controls, then reconnect. Verify pending seek, restart, lyric-offset, repeat and effect state without any automatic playback.
- Confirm owner-thread cleanup on real USB removal and rapid hotplug while the native UI and modal picker are open. The current CI cannot simulate actual device driver callbacks or real hardware.

No QuadCast/Pebble latency or acoustic feedback measurements are claimed. Real device hot-unplug and hardware identity acceptance are **NOT TESTED**.

## Additional A3 owner acceptance: compact screen, modal transport and endpoint ID continuity — NOT TESTED

1. On Windows at 1366×768 and 125–200% DPI, check full native window visibility including MUTE NOW, gain and status; run Tab/Shift+Tab and a screen reader (NVDA or Narrator). Synthetic sizing does not demonstrate actual DPI or screen-reader usability.
2. Import a source-matched local .rtk with long multilingual lyrics. Browse timed lyric list using Ctrl+L, arrows, Enter, double click and Cancel. Confirm offsets, current/previous/next progression and transport preservation while paused, playing, and offline.
3. With the lyrics modal open, press focused F8; confirm microphone monitoring immediately mutes without closing the dialog. Physically mute the mic or power speakers off if any feedback occurs; F8 alone is not a verified emergency safety guarantee.
4. Select named microphone and speaker, reopen the device modal and verify the original names remain selected. Disconnect one during the modal: the former name must not silently select Windows default. Change a same-name endpoint ID and confirm no audio resume until deliberate selection. Test rapid USB replug, default routing, and recovery of song position and DSP state.
5. Run a measured microphone-to-headphones loopback check, then only after safe wired-headphone acceptance test Pebble at low output; log method, driver and endpoint IDs without committing private machine identifiers.

Physical QuadCast/Pebble behavior, genuine endpoint re-enumeration, acoustic feedback and measured latency remain NOT TESTED. No native release authorization.

## Software-only A3/A4 direct .lrc workflow (automated source coverage; hardware NOT TESTED)

- Export a .lrc file from existing browser Lyrics Studio, select a local instrumental in the native Windows UI, and import the .lrc directly; verify live current/next lyric timestamps, selection, centisecond precision, Unicode and offset behavior.
- Import another .lrc while offline and while song is paused; ensure original song/position/A–B loop remain and mic/music never auto-enable. A canceled/invalid import must keep the former lyric session.
- Reopen the original .rtk and verify exact original-audio filename checking, A–B loop restoration and browser-handoff transport semantics are unchanged.
- These are native Windows owner workflows not physically qualified by Chromium or synthetic Win32 CI. A4 hardware acceptance remains NO GO.

## A3/A4 native practice owner checks — NOT TESTED on hardware

- Start with monitoring muted and wired headphones. From a loaded instrumental, mark A (Ctrl+1), seek or play at least 0.25 seconds onward and mark B (Ctrl+2). Toggle A-B repeat (Ctrl+4) while music paused; start music separately and verify actual loop with song fade-in and no microphone auto-enable.
- Mark invalid B at or before A; the old valid state should not be silently replaced. Clear A-B (Ctrl+3) and confirm native Repeat reverts to whole-song semantics. Test song replacement and .rtk A-B restoration.
- Import timed .rtk or .lrc and adjust lyric offset by ±100 ms, then reset to the imported offset. Ensure this shifts display/navigation but never alters audio samples or writes files. Test offline marker edits and reconnect at saved position with mic/music muted.
- Keyboard practice actions and Windows 1366×768 menu accessibility need human testing; physical QuadCast/Pebble validation and real latency remain NO GO.

## A3/A4 owner session export acceptance (NOT TESTED physically)

1. Load a local instrumental and matching UTF-8 .lrc or .rtk. Seek while offline and/or playing, nudge lyric offset and enable A–B. Choose Practice → Save local practice session (Ctrl+S), select a NEW `.rtk` filename. Confirm microphone/song are muted/paused on return, with no automatic recording.
2. Open the saved text file privately: it must contain `ROOMTONE_HANDOFF_V1` and only source audio filename, title, line timestamps/text, bounded offset and active A–B/position. It must not include audio bytes, full local paths, recordings, device endpoint IDs or secrets. Test Unicode filenames and lyrics.
3. Quit and re-open with the original song and the saved .rtk; confirm the source-owned lyrics, practice offset, A–B loop and saved position. A plain whole-song repeat is deliberately not exported as an A–B loop.
4. Cancel the native Save dialog, select an existing destination, or unplug the output while exporting: ensure no overwrite, no silent fallback, no automatic mic/music activation, and previous session remains usable. Repeat from an already offline state; it must not attempt to reconnect automatically.
5. Validate 1366x768, high DPI, Ctrl+S and NVDA/Narrator accessibility separately. CI tests only synthetic Win32 actions and portable format; no physical hardware/latency claims.

### Native session export and confirmation follow-up (manual, NOT TESTED)

Inspect the native import confirmation summary (original filename, title, lyric count, offset and .rtk resume vs .lrc retention). Decline and verify previous source lyrics/position remain intact and both audio paths stay muted. Save using Ctrl+S with an invalid extension; it must be rejected, never save audio bytes. Select an already-existing .rtk name; Windows CREATE_NEW must refuse overwriting it. A valid new .rtk must reopen with the correct original song and preserve any barely-over-250 ms A–B loop after millisecond quantization. Portable and synthetic Windows tests do not replace human UI acceptance.

## Native song queue owner acceptance (NOT TESTED on physical hardware)

1. With wired headphones, a low physical output volume and microphone monitoring MUTED, use Songs → Add local song (Ctrl+A) to choose one or more other local audio tracks; confirm no song starts playing or attempts a second capture engine. View the queued filenames in Ctrl+Q, navigate with arrows, Enter and Escape, and verify F8 mutes while the queue or device dialog has focus.
2. Test Ctrl+N/Ctrl+P with two different files, one invalid/unavailable queued source, and multiple files with identical basenames in different directories. A failed decode must retain the original PCM/song index/lyrics but remain muted. Successful switching must clear old lyrics and A–B/repeat/seek, show the correct original song filename, and remain voice MUTED and song PAUSED.
3. While interrupted or unplugged, select a queued song: decoding and song selection can complete offline, but the driver must not reconnect/unmute without explicit owner action. On manual Reconnect, confirm the selected song opens paused and muted with its correct source ID.
4. Check 1366×768 high-DPI native window and screen reader focus in Songs menu/queue selector. Ensure queue basenames are visible and private absolute paths never appear. The queue is intentionally forgotten at application exit; a previously saved .rtk is independent of the volatile queue.
5. Test USB hotplug during device and queue modals and manually verify QuadCast/Pebble vocal timing, feedback and 5-minute stability. None of this is evidenced by synthetic CI. Release stays NO GO.

## Per-song native practice and queue management — owner acceptance (NOT_TESTED)

1. Using wired headphones with physical microphone muted, add two source songs and give each distinct .rtk/.lrc lyrics, offset and A–B position. Manually switch between them, ensuring only the correct source-owned lyrics, offsets, playhead and loop return. Both playback paths must remain muted/paused after each change.
2. Add same-named files from different directories: verify practice never crosses distinct local full paths. Replace a selected audio file on disk with different content/size/modified timestamp, then revisit; stale practice must NOT be restored.
3. Use Songs > Move entry up/down, Remove inactive entry and Clear pending. Verify index updates, current PCM remaining loaded, cancel handling, explicit destructive-in-memory confirmation, no disk files deleted and no automatic microphone activation.
4. Select a now-unavailable queued file and confirm it fails before replacing the currently loaded PCM, saved lyrics or queue index. Restore the source and retry explicitly.
5. Repeat with real USB unplug/reroute and check the interrupted device is never revived without owner Reconnect. Verify focused F8 across queued-song/lyric/device dialogs, keyboard navigation, installed screen reader and high-DPI 1366×768 controls.
6. Measure actual QuadCast/Pebble latency, acoustic feedback behavior, extended dropouts and browser U21 recording by hand. Synthetic CI cannot qualify these; A4 physical gate remains NO GO.

## Effects and recovery owner acceptance — NOT_TESTED on physical hardware

- Begin with wired headphones, low output volume and physically muted QuadCast. Under Effects, adjust original feedback/echo delay, review the read-only values (Ctrl+E), activate Dry vocal comparison (Ctrl+5) and Restore startup effects (Ctrl+6). Verify no unexpected rise in voice/music volume, that Dry keeps current gain/music values, and that Restore leaves music PAUSED and microphone MUTED. Actual acoustic risk and audio quality still require human listening.
- With a valid original instrumental and .rtk/.lrc lyrics selected, disconnect the chosen USB input/output, import a local lyric session while offline, and confirm no automatic WASAPI restart. Use Audio > Device health (Ctrl+H) to inspect the actual state, then reconnect with Ctrl+R only after owner inspection and device reauthorization. Verify the offline-position seek survives manual recovery.
- Check that both Effects/Audio menus and keyboard shortcuts work at 1366x768 and with NVDA/Narrator, that modal diagnostics do not conceal active voice monitoring and no private endpoint IDs or absolute audio paths appear.
- Real QuadCast/Pebble input-to-speaker latency, feedback behavior, minimum practical 5-minute stability, hardware unplug/replug, recording U21 browser continuity and driver notifications remain NOT_TESTED / A4 NO GO.

## Lyric-based authored rehearsal sections — owner acceptance (NOT_TESTED)

1. With the microphone physically muted, headphones connected and original song loaded, import matching source-owned .rtk or .lrc lines. Use Practice > Select A-B from authored lyric lines (Ctrl+7), choose A and a later cue/true song end in the native two-stage list, then confirm. Verify original cue text, source indices and signed offset are reflected; no audio starts and repeat remains OFF.
2. Press Ctrl+8 to jump to the exact A frame, Ctrl+9 to jump to the B frame. Verify microphone/music remain MUTED/PAUSED after either jump; explicitly use the original toggle to repeat if desired. Save the .rtk manually and reload the original source to check the authored A-B range persists through the pre-existing session format.
3. Repeat with duplicated timestamps, adjacent cues <=250 ms, offsets moving cues outside the original audio, empty handoff, and cancellation at each of the two modal stages and confirmation. Invalid or canceled selection must leave the previous A-B range untouched and cannot invent song timestamps.
4. Check F8, keyboard focus/Enter/Escape, Win32 dialog ownership, 1366x768/high-DPI and NVDA/Narrator with real files. Test USB interruption mid-dialog; manual reconnect only, with no automatic microphone or music activation.
5. Real speaker feedback, sampled audio latency, continuous recording, and USB hotplug remain NOT_TESTED. These must be performed separately with actual owner hardware; software CI does not qualify A4 release.

## Owner acceptance — reading-first native lyric focus (NOT_TESTED)

1. With microphone physically muted, original local song and matching source-backed lyrics, use View > Large-lyric focus (Ctrl+F) and adjust current lyric with Ctrl++ / Ctrl+-. Confirm original lyric content and previous/next context remain correct at changing song positions; no new audio source, speech synthesis, recording, playback or automatic monitoring appears.
2. Confirm the original **MUTE VOICE NOW (F8)** remains visible, reachable and functional, with Play/Pause, Reconnect, transport, current device/voice status and notices also visible. Test F8 with focus on seek, play, reconnect and the mute button. Hiding previous controls must not trap focus; menu and keyboard shortcuts still work.
3. Restore the original full native controls by toggling Ctrl+F. Test 620x700, 1366x768, larger monitors, 125-200% Windows display scaling, very long Unicode lyric text, keyboard Tab order, NVDA/Narrator and high-contrast themes. The source-focused lyric must not cover safety warnings.
4. Real QuadCast/Pebble mic-to-speaker latency, feedback, monitoring in a room, USB unplug/replug and screen-reader usability remain NOT_TESTED. None of those can be signed off by the geometry and synthetic Win32 checks: **A4 physical NO GO**.

## Owner-held duplex suspension and approval-based recovery (NOT_TESTED)

1. Start on owner Windows hardware with headphones at low volume and a physically muted microphone. Play original instrumental, seek near a timed lyric, mark A–B and queue a second source. Trigger **Audio > Stop duplex audio (F7 / Ctrl+U)** with the seek slider focused. Verify immediate microphone/music mute, actual driver release, owner-suspended status and preservation of the song, lyrics, last pending seek and queue. F7 is a focused native window shortcut, not a system-wide mute or a substitute for physical hardware safety.
2. With the driver SUSPENDED, cancel **Ctrl+R** confirmation and verify there is **no WASAPI open**, no microphone/music output and no song-source change. After physically checking speaker/mic safety, approve Ctrl+R; verify correct original endpoints, preserved position, MUTED/PAUSED recovery and *no* switch to a newly connected same-name device without renewed approval.
3. While already OFFLINE or INTERRUPTED, open Audio > Choose devices and cancel; confirm no auto-reconnection. Reject an ambiguous/stale selected endpoint (including simulated unplug/replug during the chooser); confirm previous endpoint and PCM remain attached in memory but the driver stays offline. Only a newly selected, freshly validated input/output pair may restart muted after explicit owner selection.
4. Repeat F7 and F8 with keyboard focus on play, seek, visible buttons and the native Audio menu. Inspect 1366x768, high DPI, NVDA/Narrator and physical USB reconnection. Check actual microphone/speaker absence while held, real position preservation, audible echo reset, input-to-speaker latency, feedback safety and sustained run.
5. These physical/assistive/driver checks are still **NOT_TESTED**. Synthetic CI proves only source-level policies and simulated Win32 controls. **A4 remains NO GO**.

## A4 physical acceptance: live signal diagnostics — NOT_TESTED

1. Physically mute the microphone; connect headphones at low volume, then explicitly open **Audio > Live local signal check (Ctrl+M)**. Confirm the existing WASAPI input level follows physical capture even when software monitoring remains MUTED. Ensure no voice is routed to speakers, no duplicate WASAPI stream opens and no PCM is persisted. The output meter must reflect only actual original callback output.
2. Test quiet input, near-full-scale input, empty backing song, music playing, and the existing 0.7 limiter. Confirm percentages are digital sample amplitudes, not false dB, acoustic feedback readings or output-device latency.
3. With keyboard focus on each modal control, test **F8** microphone mute and **F7** owner-held duplex stop. Verify the driver actually releases, the status changes to SUSPENDED, numeric readings return to zero, and closing the panel cannot reconnect or re-enable voice. Explicit owner-approved recovery should restore audio MUTED/PAUSED.
4. Repeat with a real USB unplug/replug, failed WASAPI initialization, high-DPI and 1366×768 Windows, keyboard navigation, NVDA/Narrator and closing the native window. Check no stale 'live' meter readings after a driver stop. Physical signal processing, actual feedback/latency, prolonged stability and screen-reader experience all remain **NOT_TESTED/NO_GO** until manually witnessed.

## Rehearsal bookmarks — physical/manual acceptance (NOT_TESTED)

1. With an original local instrumental and physically muted microphone, mark three distinct rehearsal bookmarks (Ctrl+B), including one after an offline seek. Confirm exact owner frame is used, sorted and that adding a duplicate within 100 ms or more than 12 cues is refused.
2. Browse Ctrl+J with keyboard, Enter, Escape, and F8. Validate native source lyric labels, exact paused/muted seek, no hidden monitoring, no automatic repeat or device restart. Remove Ctrl+K and clear via Practice menu with default-No confirmation; cancellation leaves the original cues intact.
3. Switch to another queued local file and mark unrelated bookmarks; return and verify per-song ownership. Replace a queued song on disk (changed size/mtime) and ensure stale practice and bookmarks are not transferred to its replacement. Explicit Change Song should discard prior transient queue bookmarks.
4. Repeat with suspended/interrupted WASAPI and manual reconnect, Win32 620x700, high-DPI 1366x768, Narrator/NVDA, long Unicode lyrics and real QuadCast/Pebble device. Verify output remains MUTED/PAUSED across dialogs. Measure physical latency, speaker feedback and long-run stability separately. **A4 stays NOT_TESTED/NO GO**.
