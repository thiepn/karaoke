# ROOMTONE A3 — Experimental Windows-native audio

This is a **standalone Windows WASAPI console prototype**, not a replacement for the browser app. No hardware round-trip latency has been measured, and neither reliability nor speaker feedback safety is certified.

The implementation uses C++17, miniaudio 0.11.25 pinned to commit 9634bedb5b5a2ca38c1ee7108a9358a4e233f14d, WASAPI shared-mode capture/playback within one duplex callback, floating-point 48kHz mono capture and stereo output. Requested buffer periods: 120/240/480 frames (not guarantees). Dry voice has no intentional DSP delay; optional parallel short reverb and bounded gain use no memory allocation or locks in the callback. Microphone monitoring starts **MUTED** and is only enabled by typing m then Enter; physical microphone mute remains essential. No songs or recordings yet.

## Build with Windows 10/11, CMake, Git and Visual Studio C++ tools

Use a new clone or a clean checkout, avoiding production:

    git fetch origin feat/a3-native-audio-prototype
    git switch --detach <EXACT_A3_HEAD_SHA>
    cmake -S native/a3 -B build/a3 -A x64
    cmake --build build/a3 --config Release
    ctest --test-dir build/a3 -C Release --output-on-failure
    .\build\a3\Release\roomtone_a3_native.exe --list

Run with the 0-based indexes from the device list:

    .\build\a3\Release\roomtone_a3_native.exe --input N --output M --period 240 --wet 0 --gain 0.12

You need network access during the first CMake configure to fetch the pinned open-source miniaudio source. A GitHub Actions artifact, if the build passes, is UNSIGNED and not a production release.

## Physical QuadCast and Pebble V3 acceptance

1. Close browser ROOMTONE and all other monitoring programs. Connect QuadCast by USB. **Use wired headphones initially**, not speakers. Test QuadCast direct headphone monitoring as a subjective near-immediate reference, while avoiding doubled direct and processed monitoring.
2. Run --list, choose devices, then run the dry --wet 0 test. Confirm it starts muted. Type m and Enter to enable; sing consonant-heavy phrases and a short scale. Compare audible delay and naturalness with browser ROOMTONE at similar volume. Type m and Enter to mute.
3. Repeat with --wet 0.12 for restrained short room reverb and optionally --wet 0.20. Log any crackling, dropouts, distortions, feedback, and listening comfort for five minutes.
4. On errors try --period 480 or Windows default input/output. Never equate requested 120/240/480 frames with measured end-to-end latency; device resampling and clock differences may add buffering.
5. If headphones pass and you choose to try Pebble V3 loudspeakers, ensure USB or AUX, not Bluetooth; keep their physical volume very low and point them away from QuadCast cardioid pickup. Try --gain 0.08. **This prototype has no acoustic feedback detector. CLI mute requires Enter and is not an emergency stop. Use the QuadCast physical mute, or turn off the speaker, immediately if ringing begins.**
6. Optionally estimate acoustic transit time with a separate phone recording of a gentle click near the mic and the click reproduced through speakers, repeated 10+ times. Use measured time difference and subtract the difference in sound travel distances / 343 m/s. Label this as a geometrically corrected acoustic estimate, not a calibrated electrical driver round-trip measurement.
7. Record exact tested hardware, Windows audio settings, CLI parameters, expected and actual sound, median and range of measured acoustic time differences (if feasible), and whether A3 genuinely feels better than the browser. **No native performance claim until such real evidence is collected.**

| Gate | Status |
|---|---|
| Exact-head native Windows compilation and DSP test | Await CI |
| Microphone and speaker enumeration on actual hardware | Not tested |
| QuadCast-to-Pebble latency and singing comfort | Not tested |
| Uncontrolled feedback, crackles, dropouts | Not tested |
| Comparative karaoke experience | Not tested |
| Production deployment or merge | Forbidden without owner authorization |
