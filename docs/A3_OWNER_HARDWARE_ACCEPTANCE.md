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