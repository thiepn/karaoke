# ROOMTONE U5 — Owner-run Windows hardware acceptance

**Current status: NOT TESTED. Release is BLOCKED.** This checklist is a test protocol, not evidence that any hardware check passed. Nothing in U5 deploys the application. Use the exact U4 candidate `56399f8c47030eb8540fffd0dbf71b5f62635872` (the U5 documentation branch has identical application files).

## 0. Prepare safely (5 minutes)

1. Connect **HyperX QuadCast** by USB. Select **cardioid**, point the front toward your mouth, and place it about **10–15 cm** away. Aim the **Creative Pebble V3** away from the microphone; start its physical and Windows volumes low. Prefer USB/wired output to Bluetooth for latency testing.
2. On Windows, check **Settings → System → Sound**: microphone input = QuadCast; speaker output = Pebble V3 (or the actual connection it uses). Permit microphone access for desktop apps / Chrome or Edge. Close virtual audio routes such as Voicemeeter while establishing a baseline.
3. Use a separate, **local** copy of the precise qualified source, not the live URL:

   ```powershell
   git clone https://github.com/thiepn/karaoke.git
   cd karaoke
   git checkout 56399f8c47030eb8540fffd0dbf71b5f62635872
   git rev-parse HEAD
   py -3 -m http.server 8765 --bind 127.0.0.1
   ```

   Open **http://127.0.0.1:8765** in current Chrome or Edge and grant microphone permission only there. Stop the local server with Ctrl+C when done. Do not upload personal recordings to GitHub.
4. Open **Setup** and **Studio**. Confirm **voice speaker monitoring is OFF** before allowing microphone input to start. Keep the **Esc emergency mute** in reach. Never raise volume to overcome feedback.

**Important:** Browser localStorage is origin-specific. Testing at `127.0.0.1` cannot prove persistence of existing settings at `karaoke.thiepn.dev`. A separate same-origin, owner-approved release smoke is still required.

## 1. Five mandatory H1 checks

Record each test below as **PASS**, **FAIL**, or **NOT TESTED**. None may be marked PASS from screenshots, simulated microphones or headless Chromium.

| ID | Owner action | PASS requires | Evidence to keep privately | Result |
|---|---|---|---|---|
| **H1-A** | Setup → select QuadCast → **Start microphone**. Speak normally at 10–15 cm, with voice monitoring muted. | VOICE meter follows speech; normal singing does not clip; no permission/device error. | Photo or screenshot of meter with no sensitive device identifiers; input type and browser version. | NOT TESTED |
| **H1-B** | Import a **local** WAV/MP3 via Songs; select Pebble V3 in Setup or Windows Sound; play/pause. Reload and restart mic. | Backing track clearly reaches Pebble V3; microphone-to-speakers starts OFF on each connection and after reload. | Output route, volume setting and whether monitoring was OFF. | NOT TESTED |
| **H1-C** | With volume low, use **Compare dry voice**, then deliberately enable voice in speakers; speak briefly and press **Esc**. | Dry voice is comfortable, without feedback, dangerous gain, clipping or unusable echo/delay; Esc immediately mutes monitoring. | Short notes describing perceived delay and Esc result; optional external timing measurement if actually measured. | NOT TESTED |
| **H1-D** | At safe volume, switch between **Clean** and **Studio**, then apply moderate reverb/echo and stop speaking. | Effects sound stable and natural; no rising echo repetitions, ringing, whistling or persistent acoustic feedback. | Preset names and observations before/after speaking. | NOT TESTED |
| **H1-E** | Songs → load a **local** backing track; Studio → **Recording includes: Voice + local backing track** → **Record + play song** → **Stop take** → review and **Download vocal take**. | Downloaded file plays outside the app with **both** voice and music, sensible synchronization and no truncation. Record a short separate **Processed voice only** take to verify that option too. | Private downloaded clips, playable format/size/duration, voice/music audibility, timing notes. | NOT TESTED |

For **H1-C or H1-D**: If screeching, feedback or rising repeats occur, press Esc and/or stop the mic immediately; lower physical speaker volume. **FAIL** the test; do not repeat at a higher volume. You can use playback or recording with speaker monitoring OFF but must not claim H1 monitoring passed.

For **H1-E**: Protected or cross-origin YouTube/tab sound is **not** part of a downloadable mix. A local backing file is mandatory for testing the full recording path. If the voice is early/late in downloaded output, record the observed offset and test the **Recorded take: track/voice timing** control separately; this slider does not change live speaker latency.

## 2. Short evidence record (copy privately for each run)

```text
Candidate commit: 56399f8c47030eb8540fffd0dbf71b5f62635872
When tested / time zone: [not tested]
Windows edition and version: [not tested]
Chrome or Edge version: [not tested]
Input: HyperX QuadCast / cardioid / connection details: [not tested]
Output: Creative Pebble V3 / USB or wired / Windows route: [not tested]
Starting speaker volume, software monitoring level: [not tested]
H1-A Raw input: NOT TESTED / FAIL / PASS — notes:
H1-B Output, monitoring-off on reconnect/reload: NOT TESTED / FAIL / PASS — notes:
H1-C Dry monitor, Esc mute, perceived delay: NOT TESTED / FAIL / PASS — notes:
H1-D Reverb/echo stability: NOT TESTED / FAIL / PASS — notes:
H1-E Voice + local music recording and voice-only take: NOT TESTED / FAIL / PASS — notes:
Device report saved privately: YES / NO
Private recording files verified outside browser: YES / NO
Unexpected errors, discontinuities, feedback: [not tested]
Owner explicit sign-off: NOT GIVEN
```

Use **Setup → Release check / QuadCast + Pebble V3 → Download device report** once the five checks are genuinely run. The on-screen checkboxes are user-entered claims, **not** independently authenticated evidence. Before sharing a report, remove full device IDs or personally identifying labels. Do not commit raw microphone audio, diagnostic identifiers, private settings, or personally identifying screenshots to this public repository.

## 3. Decision

- **PASS / technically ready for owner release review:** all H1-A..E PASS with private evidence, no unresolved high-severity defect, U4 source and CI remain qualified, actual Windows Chrome/Edge smoke and keyboard/screen-reader checks pass, and settings/recovery are separately tested on the intended origin when approved.
- **HOLD:** any NOT TESTED item, missing evidence, unresolved high-impact accessibility issue, unknown source commit, or pending owner approval.
- **FAIL / stop:** unsafe feedback, failed emergency mute, clipped mic in normal use, wrong speaker routing, recording without expected voice/music, corrupted file, or reproducible data loss.
- **RELEASE:** requires a separate **explicit owner instruction identifying the qualified commit and production target**. Passing H1 is necessary but never itself permission to merge, deploy or modify DNS.

Do not present localhost tests, automated Chromium results or simulated MediaRecorder as hardware qualification.
