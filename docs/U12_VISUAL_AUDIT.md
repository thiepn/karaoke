# ROOMTONE U12 — Stage & Library Density Refinement / Screenshot Audit

## Exact U11 starting point and original visual custody
- U11 draft PR #14 feat/u11-after-hours-ui-polish at c2f6ac9d68e0c26ad5d5aa19fa194c13d7a43bde, stacked directly on qualified U10 and left draft/unmerged.
- Exact-head push 38176369756 and PR 38176387596 both successful.
- Original U11 push artifact 11715853371: sha256:f5bd3625b902cd4c5c793cf80518ed9e461035ab3060b5a2e03658f5fbad6ef6.
- Original U11 PR artifact 11715319723: sha256:2b39316eb87fc23e2294553e71b69819d4a48a574770f8d07e16450948c8d212.
- Both archives independently verified: ZIP digest and CRC, 14 original U11 PNGs and their per-PNG SHA-256; inherited 16 U7, 16 U6, 15 U5, 12 U4 PNGs each also verified.
- Reviewed full-page original captures at 350, 390, 768 and 1440 CSS px. These are Chromium captures, NOT owner approval or device certification.

## Screenshot-grounded defects and U12 corrections
1. At 350 and 390px, no-LRC placeholder occupies an unnecessarily large Stage area, pushing practice/loop/timing/safety below first-screen view. U12 reduces the empty-lyrics placeholder footprint only at <=680px, tightens gaps while retaining transport targets. No active synchronized lyric type, audio or events change.
2. At 390px focus, the capture clips the practice-loop panel while showing the underlying footer; at 1440px focus, the underlying mixer/footer appears below the Stage. A later inline source rule reduces the fullscreen Stage z-index to 100. U12 uses an isolated fixed 100dvh fullscreen Stage with z-index 3000 and internal scrolling to keep the original playback/practice and same original emergency Mute available; no new control handlers.
3. Songs on 350/390/1440px repeats the current-song title above the actual active row and repeats prose count below. U12 retains the playable list, counters, live announcements and filters, visually hides only duplicate summary text on smaller screens, keeps the role=status message accessible via screen-reader-only clipping, and reduces redundant spacing.

## Exact source scope — U12 child of U11 only
- index.html: one new stylesheet link to u12.css; removing the link yields U11 HTML byte-for-byte.
- u12.css: presentation-only Stage/focus layering, scroll geometry, compact empty lyric placeholder, library density.
- tests/u12_smoke.py: synthetic Chromium at 1440x900, 768x1024, 390x844 and 350x740, four original PNGs per viewport (Stage/Songs/Studio/focus), checks Stage/Queue/Mute, lyric/cue clearance, focus z-index and scroll, library status/live region, no overlap/overflow/JS errors. Local generated WAV only.
- .github/workflows/u12-ui.yml: exact-head ancestry and five-file source guard, inherited U3–U7 and U11 browser checks, U12 checks, all original per-PNG hashes, artifacts retained.
- docs/U12_VISUAL_AUDIT.md: this original evidence and custody policy.

No JS or modifications to original U2–U7/U11 CSS and JS, microphone, gain, monitoring, recording, playback, queue engine, local persistence, lyric editor, prod data, screenshot goldens or the original acceptance template.

## Unchanged human, hardware and release gates
HU7-01 through HU7-07 = NOT_TESTED.
HU7-08 explicit owner approval = NOT_GIVEN.
All physical Android, Samsung Internet, actual keyboard, TalkBack/NVDA, hardware/audio and deployed-origin acceptance remain NOT_TESTED. Original screenshots are not human observations.
All U1–U11 and separate audio/hardware drafts remain open/unmerged; main unchanged. No deploy, releases, secrets, DNS/Cloudflare, production access or recording/hardware tests. PRODUCTION NO_GO.

## Proposed U13
U13 — Responsive Focus & Song Browser Browser-Evidence Hardening.
NOT STARTED. Reverify exact U12 head and workflows, ZIP CRC/digests/original 16 U12 images, assess responsive rendered visuals, repair only independently reproduced source bugs on a separate draft, preserve human gates and NO_GO.
