# ROOMTONE U15 — Modal Focus Robustness and Browser-Accessible UI Structure

## Immutable qualified parent
- Exact U14 draft PR #17 head: e752702557c63a51b75a7972fc26d239f8a3dbd3, untouched and unmerged.
- Exact U14 push CI 38180583111 and PR CI 38180587859: both successful.
- U14 push artifact 11718085473 ZIP SHA-256 af5a2b20a2491b5ac792d77e01ec9c58f4f6ab0d177deeee04eb8952c2aec155.
- U14 PR artifact 11717895848 ZIP SHA-256 efbebcae213187b381718ac30bde933fd31f7bf75f21560cc4c221f3e5c29fca.
- Original downloads independently audited offline: both exact ZIP digests; ZIP member CRC; 118 PNGs/archive, of which 109 are individually listed and verified against 8 per-phase original SHA-256 manifests, and 9 U3 images lack their own manifest. Do not conflate protected-by-ZIP and individually manifest-listed images.
- Original U14 Stage and Focus viewport screenshots examined at 350, 390, 768 and 1440 CSS px; no independently justified broad redesign. Existing U15 docs branch and competing U14 draft PR #18 intentionally untouched.

## Reproduced defect before remediation
- Separate U15 initial diagnostic commit f8e35fd5afaa759427f9b9f5982897514eed689d on new isolated branch and GitHub Actions push 38181358758.
- Real Chromium synthetic test at 1440px, with original U14 code and fullscreen true, failed: "background header removed from modal interaction". Header.inert was false while the fixed Stage had role=dialog and aria-modal=true. Original U14 script trapped Tab but did not make background content inert to the browser accessibility tree.
- Source inspection identified missing external Focus-shortcut opener restoration and cross-view heading focus as further potential lifecycle problems. Their qualification is explicitly performed by new U15 Chromium assertions (not asserted as independently reproduced before execution).

## Remediation boundaries
U15 is a child of exact U14 PR #17, not the divergent existing U15 documents branch nor competing U14 PR #18.
Exactly five allowed changed files relative to U14:
1. index.html: one new u15.js script include after u14.js, absolutely no other original HTML/inline player markup or events modified.
2. u15.js: narrow addon only. During original fullscreen focus, temporarily makes element siblings of the Stage and its ancestors inert, saves and restores prior inert state, and preserves original single Stage and emergency Mute. Original U14 Tab trap and role=dialog remain authoritative. On exit via shortcut, tries to return to the prior visible external trigger. On the original Lyrics Studio action, closes focus through its original button before original U2 navigation and places keyboard focus on the Songs heading after actual navigation. Handles programmatic original cross-workspace navigation by leaving fullscreen before the navigation handler. All original handlers remain intact; no preventDefault/stopPropagation.
3. tests/u15_smoke.py: synthetic 350/390/768/1440 Chromium tests, accessible dialog role/name via get_by_role + aria_snapshot, background inertness and re-enable, real Tab/Shift+Tab traversal excluding disabled/hidden controls, original Escape, F shortcut origin recovery, original Lyrics Studio route and heading focus, cross-view root unlock, no JS error, 12 original viewport PNGs with SHA256 manifest.
4. .github/workflows/u15-modal.yml: exact-source pinned read-only push/PR CI, strict five-file source whitelist, byte equality of original audio, microphone, storage, U2-U7/U14 JavaScript and all previous CSS/evidence, inherited U3–U14 browser regressions and U15, per-phase original screenshot hashes, gate truth and artifact retention.
5. docs/U15_MODAL_LIFECYCLE.md: this report.

## Safety, release and truthful labels
- Original audio graph/DSP, microphone devices/monitoring, player, recordings, local song/lyrics storage, original handlers and prod data untouched.
- U1–U14 drafts and audio/hardware branches preserved unmerged; main unchanged.
- Real NVDA/TalkBack, physical keyboard and actual Android Chrome/Samsung owner acceptance remain NOT_TESTED. Chromium accessibility API role and DOM inert checks are synthetic browser verification, not an assistive-technology human witness.
- HU7-01 through HU7-07: NOT_TESTED. HU7-08 owner release decision: NOT_GIVEN.
- Production release: NO_GO. No releases, deployment, merge, credentials, DNS/Cloudflare, secrets, microphone permission, production data access or physical audio test.

## Suggested next phase
U16 — Focus Exit Resilience & Screen Reader Semantic Reconciliation (NOT STARTED).
Independently audit exact U15 screenshots, accessibility snapshots, source and CI; investigate event ordering, mobile panel ownership, keyboard Escape/Enter and actual fallback semantics, with a new separate draft only for independently reproduced defects. Never substitute synthetic observations for physical owner review.
