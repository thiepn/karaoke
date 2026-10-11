# ROOMTONE U13 — Responsive Focus & Song Browser Evidence Hardening

## Exact U12 custody and archived original images
- Source: U12 draft PR #15 at exact head 171c2d481b6ba96b96316567d638a02b3d0d5484, both push 38178513158 and pull-request 38178516231 successful.
- U12 original push artifact 11716527384 ZIP SHA256 b5238056a8bb587efb13f948e6f6ac833741d4f2b34acd8b4b477969fdcdc813.
- U12 original PR artifact 11716447564 ZIP SHA256 e29c08a75975c6c1c968c7d5aff4807e16edbd9bd14e055ee194635c2bbb8f57.
- Both original archives inspected independently in container: exact ZIP SHA256, full ZIP CRC, U12 16 PNGs, U11 14 PNGs, U7 16, U6 16, U5 15, U4 12; every per-phase SHA256 manifest was checked.
- Original screenshots reviewed individually and in a 350/390/768/1440 contact grid: Stage, Songs, Studio and focus. No owner or native device test was conducted.

## Independently demonstrated defects / evidence interpretation
1. **Duplicated mobile library totals** in 350/390px U12 original Songs screenshots. Both "1 song shown" (the live filter result) and "1 song" (the unfiltered library total) appear above the same active row. At <=680px, preserve the useful visible filtered count and make the duplicate total visually clipped but available in the original DOM and nonvisual tree. The live status remains accessible; no filter/search, song selection or count JS is modified. At desktop width, both originally available counts remain visible.
2. **Focus full-page screenshot interpretation gap**: U12 fullscreen focus *full_page=True* images include the underlying document below the viewport. This is expected from capturing the whole underlying document and is NOT by itself evidence of a visible fixed overlay defect. The U12 source already fixed the transformed containing block. U13 captures separate *viewport-only* screenshots using full_page=False and checks that the Stage occupies the entire viewport, blocks footer hit-testing, and that the root scroll is locked while the original Stage retains internal overflow and touch-sized safety controls.
3. **Potential background scroll during focus**: U12 confined focus to the fixed 100dvh Stage but only applied body overflow:hidden; root html overflow remains unrestricted. U13 adds html focus-only overflow:hidden and overscroll containment. The ordinary library and Stage remain scrollable when focus is exited. Browser assertions verify the actual computed root style and restoration; no custom fullscreen JS is introduced.

## Files in this strictly isolated U12-child draft
- index.html: exactly one new stylesheet link after u12.css; removing it yields the U12 exact-parent HTML.
- u13.css: focus-only root scroll lock, unchanged internally scrolling fullscreen Stage, and mobile visually clipped duplicate total counter.
- tests/u13_smoke.py: source-pinned synthetic Chromium 1440, 768, 390, 350 viewport checks; **12 original PNGs**, Stage and Songs as full-page reference images but **Focus as viewport-only** images. Controls, same original Mute, original counter DOM, dynamic lyrics filters, no overflow or JS errors, root scroll/restoration, Stage topmost hit-testing. Uses generated disposable local WAV, never microphone permission or real audio.
- .github/workflows/u13-ui.yml: exact-head ancestry, whitelist, inherited U3–U7/U11/U12 browser suites, new U13, original per-file screenshot hashes and artifact retention with read-only contents permission.
- docs/U13_BROWSER_EVIDENCE.md: this evidence and release boundary.

No original U2–U7, U11 or U12 CSS/JS altered, no modifications to playback, recording, monitoring, microphone, original click handlers, queue, library storage, LRC parser, lyrics editor, old artifact goldens, or evidence validator. No deliberate deployed origin access, Cloudflare/DNS, merge, release dispatch or production data operations.

## Human and release gates: preserved, not waived
HU7-01–HU7-07: NOT_TESTED.
HU7-08 explicit approval: NOT_GIVEN.
This browser-only source phase is not physical Android/Samsung, native audio, keyboard hardware, assistive tech, owner acceptance or release approval. Prior U1–U12 and separate hardware/audio drafts remain untouched and unmerged.
**PRODUCTION NO_GO.**

## Next
U14 — Viewport-Level Accessibility & Density Hardening. NOT STARTED.
Inspect U13 exact-head original viewport screenshots and all complete CI runs, repair only demonstrated defects on a fresh stacked draft, preserve protected audio/data/human gates and NO_GO.
