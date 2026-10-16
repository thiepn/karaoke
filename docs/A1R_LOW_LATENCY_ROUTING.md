# ROOMTONE A1R — Low-latency browser routing

Implemented on the existing A1 draft PR; no changes to native A3 or later stacked UI drafts.

## User-visible behavior
- System default output goes straight to AudioContext.destination even on browsers without AudioContext.setSinkId. This avoids the MediaStreamDestination -> HTMLMediaElement extra playback layer; it does **not** guarantee any particular end-to-end latency.
- An explicitly selected speaker prefers AudioContext.setSinkId. If unavailable or rejected and HTMLMediaElement.setSinkId is supported, the existing fallback is used.
- Speaker changes are muted first, with no simultaneous output paths. The same Web Audio context and independent recording destination stay intact.
- Route and capture/buffer estimates are shown separately from the **unmeasured** microphone-to-speaker delay. getUserMedia latency uses only an advisory ideal constraint.
- Existing dry/echo/reverb settings and microphone profiles remain unchanged, and monitoring is never auto-enabled.

## Physical owner test, not satisfied by CI
1. In Windows, choose Pebble V3 (USB or AUX, NOT Bluetooth) as system default, QuadCast on USB. Disable Windows "Listen to this device".
2. Set system-default output inside ROOMTONE. Connect mic; check Setup shows "direct AudioContext / Windows default output".
3. Test dry monitoring first, with physical speaker volume low and QuadCast cardioid pickup oriented away. Immediately use physical mute or Esc for feedback.
4. Compare the same consonant-heavy phrase with dry, short reverb and echo; document subjective delay and whether the issue is capture delay or intentional repeats.
5. If needed select an explicit Pebble V3 device, observe context-sink vs media-element fallback, then switch to default while monitoring remains muted.
6. Check local voice-only and voice+song recording, reconnection, physical USB disconnect and playback through actual hardware.
7. Compare browser audio with the existing A3 WASAPI test only through owner-controlled hardware qualification; never equate requested callback period or browser buffer values to measured round-trip time.

Unmeasured: actual voice latency, speaker acoustic feedback, output driver buffering, physical reverb quality and Windows-specific dropouts.
