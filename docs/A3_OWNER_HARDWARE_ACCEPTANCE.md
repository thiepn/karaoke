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
