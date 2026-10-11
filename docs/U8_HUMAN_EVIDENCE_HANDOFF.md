# ROOMTONE U8 — Human Findings Intake and Owner Visual Gate Closure

**Current decision: NO_GO.** This phase is an intake mechanism and release
blocker, not acceptance. No actual HU7-01–HU7-08 owner/device/assistive-tech
observations were located at the time this draft was created. Never translate
headless Chromium checks, synthetic fixtures, screenshots, or developer
commentary into actual human PASS/FAIL observations.

## Verifiable technical baseline
- U7 UI draft PR #10: exactly
  1c7dd8f10d2f2fcccf1ac9809f1f9047dbb4666a,
  stacked on U6 draft PR #9
  d29d4ce16d22710a2274fc93af9ab19b3e6b98b1.
- Exact-head U7 push CI 38122148928 PASS; screenshot artifact 11695334143
  ZIP SHA-256 efd090047191cc9d55de30a4cc4efedfbf9604c0d6bdedb79e9b970a7aac5ae9.
- Exact-head U7 PR CI 38122151357 PASS; screenshot artifact 11695687410
  ZIP SHA-256 85c4dfae4b51ee58e07c779390cf1a6f77ed6a1dd0164b0036a04c7b8c4ba3e7.
- Both ZIP CRCs and all original 16 U7, 16 U6, 15 U5 and 12 U4 PNG
  manifests passed independent verification. No screenshot is a human witness.
- All U1–U7 parent-child merge bases verified; main untouched.
- U8 must change only intake documents, a local stdlib validator and its
  source tests / CI. No UI, JS player code, CSS, audio, data or config changes.

## Human gate register — NOT TESTED / NOT GIVEN
| ID | Exact human observation | Current status |
| --- | --- | --- |
| HU7-01 | Android Chrome portrait: Stage, Queue/Mixer row, navigation Mute, unobstructed Play/Next, scrolling and focus | NOT_TESTED |
| HU7-02 | Android Chrome: actual Queue/Mixer open-close and focus return | NOT_TESTED |
| HU7-03 | Android Chrome: Songs filters/search with safe local WAV/LRC only | NOT_TESTED |
| HU7-04 | Samsung Internet: real Stage/Queue/Mixer/Mute visual and focus | NOT_TESTED |
| HU7-05 | Desktop Chrome: actual Stage/fullscreen/library presentation | NOT_TESTED |
| HU7-06 | Physical keyboard: Tab, Shift+Tab, Enter/Space, Escape, focus return | NOT_TESTED |
| HU7-07 | Actual TalkBack or NVDA: headings, live statuses, current song, pressed filters, sheet focus | NOT_TESTED |
| HU7-08 | Separate, explicit owner visual/release approval | NOT_GIVEN |

## Intake contract
A checked-in blank source template lives at
docs/evidence/U8_HU7_ACCEPTANCE.template.json.
It names every gate and pins the U7 source SHA. It is **NOT** a completed
witness record. Keep it unchanged, with all statuses open. A private,
off-repository copy may capture real observations when the owner performs
each test. Avoid putting personal photos, microphone recordings, device
identifiers, network addresses, account data or raw private screenshots
in public GitHub PRs, comments or Actions artifacts.

Local *format-only* validation:

    python scripts/u8_validate_evidence.py docs/evidence/U8_HU7_ACCEPTANCE.template.json
    python -m unittest discover -s tests -p 'u8_evidence_intake_test.py' -v

To evaluate a genuine witness record, work with a separate local JSON copy:

    python scripts/u8_validate_evidence.py /private/path/owner-observations.json

The validator rejects ambiguous status names, missing attestation fields,
duplicate IDs, release GO, wrong tested SHA, unqualified timestamps and
malformed private evidence pointers. It **cannot establish human honesty,
screen-reader behavior, genuine device execution, permission safety or
production readiness**. Unit tests use clearly synthetic records. Even a
structurally complete record never changes NO_GO automatically.

PASS or FAIL is valid only when the real owner supplies an actual observation,
its test ID, observed result, time with UTC offset, device/browser, observer
identifier and a description. Screenshots are optional and should remain
private. If referenced, store only a private local reference and SHA-256;
do not publish the image into the source PR. For HU7-08 the only positive
human decision is explicit APPROVED by the owner, not generic PASS. A
negative release decision is REJECTED. Any uncertainty stays NOT_GIVEN.

## One-test-at-a-time human handoff: HU7-01
**Do not advance automatically.** The exact U7 draft is not deployed; testing
the production site would not assess these U7 source corrections. On an
authorized local computer with the repository present, check out exact U7:

    git fetch origin feat/u7-owner-visual-handoff
    git switch --detach 1c7dd8f10d2f2fcccf1ac9809f1f9047dbb4666a
    git rev-parse HEAD
    python -m http.server 8765 --bind 0.0.0.0

On Android Chrome on the same trusted private LAN, open the computer's
LAN address on port 8765. Review the Stage in portrait without granting
microphone access, opening recording devices or uploading real audio.
Queue/Mixer must appear above the Stage, navigation Mute must remain
visible without obstructing Play/Next/practice, and empty-lyrics focus
must remain legible. Scroll and report the observed outcome.

This HTTP LAN route is a **non-secure test origin**. It is not a hosted HTTPS,
storage recovery, microphone permission or native audio acceptance. Shut
down the disposable HTTP server after testing. If this test setup is not
possible or not executed, retain HU7-01 = NOT_TESTED.

Record only actual **PASS or FAIL** and specific overlap/focus observations;
do not infer from generated PNGs. Move to HU7-02 only after HU7-01 has been
recorded.

## Source and release protection
All U1–U7 drafts, hardware and audio work, original data, local and
production recordings, DNS, Cloudflare settings, secrets, releases and
production deployments are out of scope. This is an isolated U7-child draft
only. No merge, workflow dispatch, physical microphone test, live monitoring,
or production-origin contact is authorized. Even with passing CI and real
visual acceptance, release needs a separately authorized plan because main
may auto-deploy as stacked drafts are merged.

## Next phase
U9 — Witness Findings Assessment & Residual Visual Defect Qualification.
**NOT STARTED**. Remain blocked on genuine HU7-01–HU7-08 evidence; do not
mark U8 human gates closed based on this source-only intake infrastructure.
