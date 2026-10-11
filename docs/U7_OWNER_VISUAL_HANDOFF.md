# ROOMTONE U7 — Owner Visual Review & Real-Device Evidence Handoff

## Exact qualification prerequisites
- U6 PR #9 remains draft, head d29d4ce16d22710a2274fc93af9ab19b3e6b98b1.
- U6 exact-head push 38105058935 and PR 38105060906 both succeeded.
- U6 original artifacts 11688799814 and 11688824749 each retain:
  16 original U6 PNGs with matching SHA-256 manifests; 15 U5 and 12 U4 PNGs,
  also verified. ZIP SHA-256/CRC independently verified before branching U7.
- U1–U6 ancestry linear without divergence; main is not a U7 release target.

## Screenshot-grounded defect and scope of correction
The original U6 Android-size (390x844) Stage screenshot visibly places the fixed
Queue/Mixer/Mute rail across the playback/progress/practice region. This is a real
rendered-browser visual defect; it is NOT evidence about real phones.

U7 reuses exactly the existing Queue and Mixer buttons and moves their dock
immediately after the Stage section heading in normal document flow, sticky
below the actual measured header height at narrow widths. The same original emergency mute moves into the sticky navigation so that it
does not cover the Next button. During fullscreen focus mode it repositions
inside the fullscreen Stage. Its existing click handler, Escape shortcut and
the original inline emergency mute remain unchanged. U7 does not change microphone permissions, audio DSP,
playback/recording, safety shortcuts, user data, LRC, or saved storage.

The first U7 Chromium screenshot demonstrated a second obstruction: the fixed
emergency button covered Next. The second corrective commit puts it in the
sticky navigation. Re-check the revised screenshots; do not claim real-device
acceptance or shortcut reliability without owner observations.

## Evidence classification
- **Chromium synthetic acceptance**: checks DOM layout, original controls,
  keyboard Escape, focus, song loading and viewport overflow; does not establish
  physical handset or assistive-technology experience.
- **Original CI screenshot**: browser evidence; not an independently approved
  visual golden or a human observation.
- **Owner/human acceptance**: NOT TESTED. Must be explicitly supplied by an
  actual device user and have device, browser/version, time and specific result.
- **Production/hosted-origin acceptance**: NOT TESTED. No user data accessed.

## Required owner witness sequence (execute ONE item at a time)
All outcomes initially **NOT TESTED**. If a real test cannot be performed,
retain NOT TESTED rather than guessing. Store screenshots privately without
sensitive files or live microphone input.

| ID | Real test | Result |
| --- | --- | --- |
| HU7-01 | Android Chrome: Stage at actual mobile width, inspect Queue/Mixer row above lyrics and red navigation Mute visibility while scrolling | NOT TESTED |
| HU7-02 | Android Chrome: open Queue then Mixer, close, return focus, ensure original controls work | NOT TESTED |
| HU7-03 | Android Chrome: Songs filters/search with synthetic local song and matching LRC (no microphone) | NOT TESTED |
| HU7-04 | Samsung Internet: repeat Stage, Queue, Mixer, safety visibility | NOT TESTED |
| HU7-05 | Desktop Chrome: 1440 px lyric layout, real queue actions, fullscreen focus and contrast | NOT TESTED |
| HU7-06 | Real keyboard: Tab/Shift+Tab, Enter/Space, Escape sheet-close + emergency mute, focus return | NOT TESTED |
| HU7-07 | Real TalkBack/NVDA: status announcements, headings, selected songs, pressed filter, sheet semantics | NOT TESTED |
| HU7-08 | Explicit owner aesthetic acceptance with evidence and timestamp | NOT GIVEN |

## Local-only mobile test environment — without production deployment
The owner can check out exactly the U7 draft commit locally on a computer,
confirm its full SHA using git rev-parse HEAD, and start a disposable static
Python HTTP server on a *trusted private LAN*:
  python -m http.server 8765 --bind 0.0.0.0
Then open http://COMPUTER_LAN_IP:8765 on Android Chrome using the same trusted
Wi-Fi. Stop the server after testing. Do NOT grant microphone permissions,
start live monitoring or upload recordings. Plain HTTP over LAN is not a
secure context and **cannot** certify permission, audio, HTTPS/origin/storage
or production runtime behavior. UI-only observations still count as real
device/browser evidence when accurately reported.

Alternative: USB remote port forwarding to Android Chrome localhost can provide
a local secure context for UI, but remains NOT a deployed-origin acceptance.

## Owner handoff — Test HU7-01 ONLY
Open the exact U7 local build in Android Chrome, with Stage active. Confirm
the Queue and Mixer buttons appear in their own row **above** the Stage, not
over the transport/progress or lyric text. Slowly scroll between lyrics,
playback/practice and bottom of the page. Check that the red Mute button in the navigation is visible and does not
cover Next, playback or practice controls; inspect focus mode too.
Do not enable the microphone. Record:
- Android device model, Chrome version (if known) and portrait orientation;
- PASS or FAIL and the observed behavior in one sentence;
- screenshot or visible overlap description if FAIL, and approximate time.

Stop here. Do not mark HU7-02 or later tests until the owner reports HU7-01.

## Release and phase gates
**NO_GO** until independently approved. No merge, deployment, release dispatch,
DNS, Cloudflare settings, preview activation, production access or credentials.
U7 is a draft UI-only child of U6, not a prerequisite to audio research.
U3–U7 screenshot files are not physical/human witness artifacts.

## Next phase
**U8 — Human Findings Remediation & Owner Visual Gate Closure**:
Not started. Only begin acceptance closure after genuine human observations
are supplied. Otherwise maintain the open U7 checklist, source-only draft,
and explicit production NO_GO.
