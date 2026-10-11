# ROOMTONE U14 — Viewport-Level Accessibility & Density Hardening

## Qualified U13 baseline, independently inspected
- U13 PR #16 remains draft at exact `d989d869d4ed4071469bb8ba2e4c9d54d91d7977`, based on U12 `171c2d481b6ba96b96316567d638a02b3d0d5484`.
- U13 exact-head push 38179573420 and PR 38179577841: SUCCESS; all inherited and U13 browser steps passed, 144 synthetic U13 assertions.
- U13 original push artifact 11716869418: archive SHA-256 `9c775070d47e7c744d4c78befdd0b911882177d6d4aaa01dacd510b21964c798`.
- U13 original PR artifact 11716798011: archive SHA-256 `ad38d150cf09a9422220263c5939c4c29f86d9900265932fcae318bfc5c48086`.
- Both exact ZIP bytes independently inspected: SHA-256 verified, ZIP CRC passed, and every original PNG in the per-phase SHA-256 manifests matched: U13 12, U12 16, U11 14, U7 16, U6 16, U5 15 and U4 12 (101 each). Original U13 Stage/Songs/fullscreen-viewport captures at 350, 390, 768, 1440 CSS px were visually inspected.

## Independently identified accessibility defect in U13 source
- Existing `setFocusMode(on)` toggles `#stage.focus-mode`, `body.in-focus`, label and `aria-pressed`. U12/U13 fix full-viewport position, background scroll and visual coverage.
- But the original Stage remains an ordinary `<section>` and none of its ancestor-chain sibling regions is made inert; no `aria-modal`, dialog semantics or Tab/Shift+Tab containment is established. The visual overlay cannot protect keyboard or screen-reader navigation from hidden underlying page controls.
- U14 corrects *only* this source-observable access boundary, using `u14.js` attached after existing U7 controller. A body-class observer synchronizes the original focus mode, marks unrelated sibling regions inert, temporarily adds dialog/aria-modal/aria-labelledby semantics to the original Stage, and traps Tab/Shift+Tab within visible Stage controls. On exit it restores only attributes/inert states U14 set, and preserves existing Escape emergency mute and native Stage focus behavior. No player, microphone or library handlers are replaced.
- The implementation uses existing `#stageFocus`, `#stageTitle`, `#stagePanic` and existing U7 `#rtMobileMute` without replacing, duplicating or changing their event listeners.

## U14 five-file source-only change set
- `index.html`: exactly one `<script defer src="u14.js"></script>` after `u7.js`; removing it yields the exact unchanged U13 source.
- `u14.js`: isolated modal semantics, inert ancestor siblings and keyboard cycling only during inherited fullscreen.
- `tests/u14_smoke.py`: synthetic Chromium 350/390/768/1440 browser checks and actual Chromium CDP accessibility-tree inspection; original viewports and keyboard forward/backward focus wrap, root scroll, safety Mute placement, exit restoration, Songs typed search/live status and no horizontal overflow. Captures 16 original PNGs, SHA-256 manifest.
- `.github/workflows/u14-ui.yml`: exact U13 source ancestry and five-file allowlist, read-only permissions, inherited U3-U7/U11/U12/U13 Chromium suites, new U14 tests, SHA-256 image manifests and retained original unapproved screenshots.
- `docs/U14_ACCESSIBILITY_EVIDENCE.md`: this source justification and owner gate.

## Prohibitions and unresolved acceptance
No U1–U13 code alterations except the added script reference. No audio, DSP, recording, microphone, queue, LRC, user storage, saved data, screenshots goldens, deployed origins, protected resources, DNS/Cloudflare, credentials, merge or release operation. Browser CDP inspection is synthetic accessibility evidence, **not real TalkBack, NVDA, Samsung Internet, hardware keyboard or owner visual approval**. These remain NOT_TESTED.

HU7-01–HU7-07 **NOT_TESTED**; HU7-08 **NOT_GIVEN**; protected audio/hardware gates unchanged; production **NO_GO**.

## Next phase, conditional
U15 — Independent Modal Accessibility Regression & Owner-Witness Handoff. **NOT STARTED**. Reverify U14 exact-head runs and independent screenshot custody first; fix only demonstrated defects on a new U14-child draft, then prioritize genuine owner real-device observations without treating machine-generated evidence as human approval.
