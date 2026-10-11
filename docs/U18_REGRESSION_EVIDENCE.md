# ROOMTONE U18 — Actual Modal Route Focus Reproduction & Minimal Recovery

## Custody and branch isolation

- Parent is exact qualified U16 [draft PR #21](https://github.com/thiepn/karaoke/pull/21) head `91b3379e43847f36073bbc31a8317ee7b72cdb32`; inherited U16 exact-head push CI 38182168435 and PR CI 38182191250 **SUCCESS**, 15 original U8 plus 11 U16 unit tests per run, and no private human record.
- U15 original source archive push 11718026758 ZIP SHA-256 `d90312a065d9d9be2ab5ac6394fd81245a094ad07c48a2ee382985d20d16b0b3`; PR 11718610505 ZIP SHA-256 `80d418c2011818446a11d8e846fce10126fce4a8e479e79530a5275546b00ae0`. Independently downloaded during U18; all CRCs clean, **134 original PNGs per archive** and all nine available per-phase SHA-256 manifests valid (125 PNG manifest entries plus original 9 U3 images).
- PR #17 U14 keyboard-only `e752702557c63a51b75a7972fc26d239f8a3dbd3` and preferred PR #18 modal/AX `194862723f2c89d72ad32b0f8591476095197e50` remain **distinct draft descendants of U13**; U15 #19 `df3dc97134492974f210c6f6b96f20877dd441d4` and U16 #21 follow preferred PR #18. Competing U15 [PR #20](https://github.com/thiepn/karaoke/pull/20) is a separate PR #17-child at `754b37ab84caeaab89ed2b69cbe458eb13c01ebd`. No cross-branch merging or cherry-picking.
- Production `main` remains `9b1d39f6110baf581f4598d45b2fba1258db18c0`.

## Evidence-based source defect

Independent U18 test-only diagnostic head `d2d31b87d0f38b50c89d6242a553221d743fd09f` intentionally ran a positive Chromium browser assertion **before making any UI source change**:

- Original [U18 push run 38184768879](https://github.com/thiepn/karaoke/actions/runs/38184768879) **FAILED** on the new U18 browser gate. At **1440x900**, original log reports a valid Lyrics studio route out of fullscreen (`songsVisible=True`, `stageHidden=True`, `stageFocused=False`, `bodyLocked=False`, `headerInert=False`, `sidecarInert=False`) **but `document.activeElement` was BODY**.
- Original failure: `AssertionError: U18 1440px: destination Songs heading focused after Lyrics studio route`. This is browser/DOM evidence, not an Android/keyboard/screen-reader observation.
- The cause is scoped: original U1 `switchView('songs')` is called without the `focusHeading=true` option by the target `#jumpToStudio` click listener; it hides Stage and exposes Songs. U14 modal exits on `body.in-focus` mutation and correctly restores inert background but does not focus the destination Songs heading. The original activated button is then hidden, leaving focus on BODY.
- Competing PR #20 independently investigated a similar route defect **on a different U14 source line**; its patches were **not copied, merged or substituted** into the preferred PR #18 line.

## Minimal isolated remediation (only after diagnosis)

- The **only existing file edit** is adding exactly one deferred `u18.js` reference at the bottom of `index.html`, after the inherited `u14.js`. Removing that single line restores byte-exact parent HTML.
- New `u18.js` observes the authentic original `#jumpToStudio` click during fullscreen, then in the next animation frame focuses the **existing** `#rtSongsHeading` with `preventScroll:true` **only if** Songs is really visible and fullscreen and body lock are closed; it does not replay the action, cancel the click, invoke playback, move content, write storage or replace original routing.
- No edits to U1–U15 original UI event handlers, recording/audio, microphone, DSP, keys, storage, lyrics, original CSS, original U8 acceptance template/validator or U16 private evidence contract.
- Original U18 diagnostic captures at 1440px show actual before/after route DOM. New 1440/768/390/350px browser tests gate keyboard focus, modal semantics, original mobile Mute, scroll restoration and Songs navigation, with original viewport PNG evidence and SHA-256 manifests, followed by inherited U3–U7/U11–U15 suites and human NO_GO checks.

## Release and human truth

The new UI patch is a narrow **synthetic-browser** correction only. It does not certify physical Android Chrome, Samsung Internet, actual external keyboard, TalkBack/NVDA, a microphone, monitoring/feedback, local sensitive audio or deployed origin. HU7-01–HU7-07 **NOT_TESTED**, HU7-08 **NOT_GIVEN**, production **NO_GO**. No merge, deploy, release, DNS/Cloudflare, credentials or production/private data.

## Following U19

**U19 — Cross-View Focus Regression and Independent Acceptance Readiness**, **NOT STARTED**. Reverify U18 exact qualified source and original artifacts, add only independently evidenced regression/recovery work. Human approval is a separate non-automated gate.
