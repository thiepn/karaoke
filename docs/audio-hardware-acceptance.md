# ROOMTONE A1 — First-use calibration and H1 hardware qualification

A1 is stacked on the unmerged U4 AFTER HOURS draft and is **not a release**. This adds local, explicit microphone sound profiles, a first-use checklist, subjective latency feedback and a manual JSON report. It does not change capture, DSP, media recording, playback, routing or feedback safety.

## Genuine device acceptance (manual owner actions)
1. Use Chrome/Edge, connect the real QuadCast or handheld microphone through USB, and Creative Pebble V3 through USB/AUX rather than Bluetooth. Speakers low; monitoring OFF.
2. In Setup, select the real input and speaker output. Run the existing **Measure room noise** and **Measure voice** checks. Ensure raw peaks do not clip; reduce physical mic gain if they do.
3. Enable **Dry-only sound check** before live monitoring, point speakers away from the microphone, and explicitly enable monitoring at the default 8%. Press **Esc** immediately if ringing develops. Verify default mute on reload, device loss and recovery.
4. Sing a short phrase or clap and listen for timing displacement. Mark the **subjective** experience (comfortable / noticeable / unusable). This is NOT a calibrated number. To measure actual round-trip milliseconds, use a separately instrumented synchronized acoustic impulse/loopback with known processing and capture offsets.
5. Switch back to wet short-room reverb, then test moderate echo. Ensure no feedback, self-growing repeats or unexpectedly hot output.
6. Record a real vocal-only and local voice+track take; download and play both. Check timing, sound, clipping and file validity. Browser synthetic MediaRecorder tests are not physical capture proof.
7. Independently complete the original five H1 checklist gates and its downloadable report, then export A1's manual observations. Neither the JSON form nor an automated check makes a hardware certification.

## Storage and failure behavior
- Profiles use localStorage keys based on the currently selected browser microphone device ID. Changing devices never automatically loads profiles. Devices may receive different IDs when browser permissions change.
- Applying profiles while monitoring appears active is blocked; presets do not request microphone permission or enable monitoring.
- A1 input gain loads are conservatively capped at 140% to avoid large surprise changes, though actual physical gain still needs user adjustment.
- Built-in overload mute, Esc emergency mute, fade-in, current reverb/audio graph, all recordings and locked U4 visuals remain in place.
- Do not merge, publish, change Cloudflare/DNS, or treat hardware as qualified based on synthetic CI.
