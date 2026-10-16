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
