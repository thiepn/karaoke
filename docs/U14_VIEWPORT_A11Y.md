# ROOMTONE U14 — Viewport-Level Accessibility & Density Hardening

## Qualified U13 baseline and original screenshot custody
- Parent is U13 draft PR #16 exact qualified head d989d869d4ed4071469bb8ba2e4c9d54d91d7977.
- U13 exact-head push CI 38179573420 and PR CI 38179577841 both succeeded, including all inherited U3–U13 Chromium assertions.
- Original artifact 11716869418: ZIP SHA256 9c775070d47e7c744d4c78befdd0b911882177d6d4aaa01dacd510b21964c798.
- Original artifact 11716798011: ZIP SHA256 ad38d150cf09a9422220263c5939c4c29f86d9900265932fcae318bfc5c48086.
- Both actual archives independently read: SHA256 and ZIP CRC passed. Each contains 110 PNGs total, including **101 per-image hash manifest-listed** PNGs (U4 12, U5 15, U6 16, U7 16, U11 14, U12 16, U13 12) and **nine separate U3 PNGs not accompanied by individual U3 hashes**. Do not describe all 110 as individually hash-manifested. The ZIP SHA256 protects all members.
- Inspected full-size original Stage, Songs, and viewport-only Focus screenshots at 350/390/768/1440 widths. The existing responsive layouts are coherent. No speculative global visual redesign is justified by this evidence.

## Actual independently reproduced keyboard defect
The original Stage focus mode is a fixed, 100dvh visual surface, but the original setFocusMode only toggles CSS classes and aria-pressed. No focus containment was present.
The initial isolated U14 diagnostic commit f94a3c5dcc372e281e2828bc88452bcef52ee306 and failed push Actions run 38180395313 independently reproduced:
- Chromium 1440px, fullscreen true; after six successful Tab keypresses, **Tab #6** moved actual keyboard focus to original #rtAddSong outside Stage while the Stage remained fullscreen.
- This is a functional keyboard accessibility defect. The intentionally failing initial diagnostic is retained as evidence, not mislabeled as a qualified acceptance run.

## U14 source-only remediation
An additive isolated u14.js focus guard:
- Activates only while the original Stage has focus-mode and its workspace is visible.
- Cycles keyboard Tab and Shift+Tab through currently visible enabled Stage controls. It does not replace any existing control or event handler.
- Redirects programmatic focus wandering behind the fullscreen Stage back to its original Focus/Exit control.
- Adds role=dialog / aria-modal=true **only during** fullscreen; restores the exact prior attribute values after exit.
- Leaves original Escape emergency mute priority, Focus/Exit handler, global navigation, Queue/Mixer, original audio nodes and local-data behavior untouched.
- Keeps the existing focus-pane CSS and internal scroll geometry identical to U13. No new settings, credentials or production access.

## Qualification plan and objective limits
- New tests/u14_smoke.py checks native headless Chromium keyboard containment at 350x740, 390x844, 768x1024 and 1440x900, 45 forward + 45 reverse Tab keypresses at each viewport, original Escape close, original focused controls, semantic lifecycle, and no JS errors.
- Eight original U14 screenshots: Stage **viewport-only** and Focus **viewport-only** at each of four widths. Each PNG has an independently checkable sha256.txt manifest. These are browser screenshots, not human/device acceptance.
- Dedicated read-only U14 workflow pins the exact PR/push source SHA; verifies parent ancestry and diff whitelist; byte-for-byte original index after removal of one u14.js link; unmodified U2–U7/U11–U13 styles, original U2–U7 JS, U8 evidence/validator; runs full inherited U3–U13 browser suites; confirms HU7 statuses and production NO_GO.
- No test contacts a production origin, grants mic permissions, touches physical microphones/audio hardware, changes live storage or exercises release dispatch.

## Unchanged human/release gates
- HU7-01 through HU7-07: NOT_TESTED.
- HU7-08 (explicit owner decision): NOT_GIVEN.
- Actual TalkBack, NVDA, real mobile/physical keyboard and owner visual acceptance: NOT_TESTED.
- Production: NO_GO. Main remains the baseline branch, earlier drafts and separate audio/hardware drafts stay unmerged.
- No merge, deployment, secrets, DNS, Cloudflare or production data change.

## Proposed next phase
**U15 — Modal Focus Robustness & Real UI Structure Qualification**, NOT STARTED.
Audit the exact U14 browser evidence and test focus lifecycle including navigation to Lyrics studio, hidden/disabled control order, and synthetic screen-reader semantics. Implement only independently reproduced defects in a U14-child draft. Human gates remain open; release NO_GO.
