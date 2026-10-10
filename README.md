# ROOMTONE Karaoke

A standalone, local-first karaoke studio that runs in a browser. Built for a HyperX QuadCast microphone and Creative Pebble V3 speakers, but can be used with other audio devices supported by Chrome or Edge.

## Run locally

- Open `index.html` in current Chrome or Edge.
- Allow microphone access when prompted. If microphone access is blocked when opening a local file, serve the folder over localhost (for example, `python -m http.server 8765`) and open `http://localhost:8765`.
- Select your microphone and speakers in the application, load a locally stored music file, and optionally add an `.lrc` lyrics file.
- Start with **voice speaker monitoring OFF** and the **physical speaker volume low**. Press **Esc** to mute monitoring.

The app supports local song queues, synced lyrics and an editor, live vocal effects, A–B practice loops, local practice-session metadata, and processed-voice or local voice+track recording. Uploaded song files and microphone audio are processed locally and are not uploaded to an app backend.

## Release status

**v1.0 release candidate with H2 playback-control correction; not hardware-qualified.**

- Source: the preserved v1.0 standalone app plus H2-001.
- H2-001: the stage Play/Pause handler no longer forwards the click event as a forced-play argument.
- Previously reported headless Chromium regression: 12/12 assertions passing for the corrected candidate. This is not proof of real audio-device quality.
- The five physical QuadCast/Pebble V3 checks have **not been independently completed**: raw microphone input, speaker routing/default mute, safe dry monitoring, reverb/echo stability, and downloaded recording playback.
- The interface includes **Release check / QuadCast + Pebble V3** with an **Download device report** action.

**Safety:** Using speakers with a live microphone can create loud acoustic feedback. A software limiter cannot guarantee prevention. Do not treat displayed browser audio buffer timings as measured microphone-to-speaker latency. If feedback occurs, immediately mute with Esc and reduce physical speaker volume.

## Hosting

This repository contains `index.html`; uploading it to GitHub **does not automatically publish a website**. GitHub Pages or another static host can be configured separately. Browser microphone access on a public host normally requires HTTPS.

## Files

- `index.html` — single-file ROOMTONE H2 candidate.

No cloud backend, account, telemetry, or externally hosted media catalog is required.
