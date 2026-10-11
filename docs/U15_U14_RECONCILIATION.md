# ROOMTONE U15 — Independent U14 Draft Reconciliation & Owner-Custody Handoff

## Baseline and draft independence
- U13 [draft PR #16](https://github.com/thiepn/karaoke/pull/16) exact head \`d989d869d4ed4071469bb8ba2e4c9d54d91d7977\`, unmerged. This is the **same exact parent** of both competing U14 draft PRs. No U14 draft was merged, closed, rebased or modified during reconciliation.
- [U14 keyboard-only PR #17](https://github.com/thiepn/karaoke/pull/17) current exact head \`e752702557c63a51b75a7972fc26d239f8a3dbd3\`, two ahead / zero behind U13; original U2–U13 application/audio/queue/acceptance data untouched. Narrow changes: one \`u14.js\` ref plus script, focused synthetic browser test, workflow, findings doc.
- [U14 modal/AX PR #18](https://github.com/thiepn/karaoke/pull/18) current qualified exact head \`194862723f2c89d72ad32b0f8591476095197e50\`, five ahead / zero behind U13. Only five U14 source files; original app player/audio/data and older draft contents remain unchanged.
- U15 itself is a **separate evidence-only child of qualified PR #18**, not a merge/rebase of PR #17. It changes only the U15 test, read-only CI workflow, this reconciliation doc, and the private owner-run handoff doc. U14 application code and all earlier code remain byte-for-byte equal to PR #18.

## Independent original failure and both fixes
The earliest PR #17 diagnostic at \`f94a3c5dcc372e281e2828bc88452bcef52ee306\` intentionally ran a positive focus-containment gate against **unfixed U13**:
- [Push 38180395313](https://github.com/thiepn/karaoke/actions/runs/38180395313) FAILURE, and [PR 38180408357](https://github.com/thiepn/karaoke/actions/runs/38180408357) FAILURE.
- Both original failure logs read: \`AssertionError: 1440px: Tab #6 escaped Stage to {'inside': False, 'tag': 'BUTTON', 'id': 'rtAddSong', 'stage': True}\`.
- This is a reproducible browser keyboard defect, **not a physical owner observation**.

PR #17 later passed at its current \`e752...\` head:
- [Push CI 38180583111](https://github.com/thiepn/karaoke/actions/runs/38180583111) SUCCESS, [PR CI 38180587859](https://github.com/thiepn/karaoke/actions/runs/38180587859) SUCCESS.
- Original logs show **392 synthetic assertions per run**, including 45 Tab and 45 Shift+Tab keypresses at **each** of 1440, 768, 390 and 350 CSS px, and 8 source-origin viewport screenshot PNGs.

PR #18 was already qualified at \`194862...\`:
- [Push CI 38180402124](https://github.com/thiepn/karaoke/actions/runs/38180402124) SUCCESS; [PR CI 38180416978](https://github.com/thiepn/karaoke/actions/runs/38180416978) SUCCESS, inherited suite plus **132 synthetic U14 assertions** at four widths.
- Its Stage is temporarily a **named** dialog with \`aria-modal=true\`; it marks sibling branches of the ancestor chain \`inert\` during focus, verifies background removal from Chromium's accessibility tree, and preserves the original Mute/Play/Escape handlers and restores prior attributes/inert on exit.
- PR #17 uses only a keyboard focus trap and dialog role without native sibling inert or a separately verified dialog name/AX-tree exclusion. Consequently an apparent successful Tab loop does not independently establish background screen-reader isolation.

## Candidate decision
**Preferred candidate: PR #18 source unchanged, plus PR #17 repeated-keyboard test coverage in isolated U15.**

Criteria:
1. Functional reproduction/correction: both PR #17 and #18 qualify normal keyboard focus; PR #18 also proves browser AX exclusion of background UI and preservation of the original keyboard/audio surface.
2. Risk of duplicate competing U14 changes: **no merging or cherry-picking**. Reuse the **test behavior** of PR #17 in a new pure U15 synthetic test on PR #18; retain both existing U14 PRs intact.
3. Maintainability: native \`inert\` + named dialog expresses the browser accessibility boundary, while a focus trap supplies keyboard fallback. PR #18 is moderately more complex; synthetic evidence does not establish cross-browser or real-screen-reader behavior.
4. Explicit gaps: hardware keyboard, TalkBack, NVDA, Samsung Internet, Android browser and owner visual signoff remain **NOT_TESTED**. The script-level focus restoration and semantic announcement must be independently assessed by the owner; do not infer production readiness.
5. PR #18 is not approved for merge/release. The preferred candidate is a **source/evidence recommendation** only; U15's passing CI qualifies test custody, not owner acceptance.

## Independent original artifact custody
All ZIP archives listed below were downloaded from their original Actions artifact endpoints. SHA-256 was computed directly over ZIP bytes; \`ZipFile.testzip()\` returned no CRC failures. Every per-phase \`sha256.txt\` entry was recomputed over the archived original PNG bytes and matched, without editing or substituting the image.

| Draft | Workflow | Artifact | Verified ZIP SHA-256 |
|---|---|---|---|
| PR #18 push | 38180402124 | 11716774143 | \`0c9eb3fbf11b10c5d892605d4d69fd1ceea76cf4a56aee00ecccc862d10582ad\` |
| PR #18 PR | 38180416978 | 11718150028 | \`a02128249f8fe1e1e1485675cd15e269d891ea9b426d8865d0e2298a9a6d4433\` |
| PR #17 push | 38180583111 | 11718085473 | \`af5a2b20a2491b5ac792d77e01ec9c58f4f6ab0d177deeee04eb8952c2aec155\` |
| PR #17 PR | 38180587859 | 11717895848 | \`efbebcae213187b381718ac30bde933fd31f7bf75f21560cc4c221f3e5c29fca\` |

PR #18 has **16 U14**, PR #17 has **8 U14** PNGs per artifact; both contain inherited **12 U13 + 16 U12 + 14 U11 + 16 U7 + 16 U6 + 15 U5 + 12 U4** originals, each with matching original PNG SHA-256 manifests. Also retained: 9 original U3 PNGs. U14 original viewports and Songs, Stage, scroll and focus screenshots were visually inspected across 350/390/768/1440 CSS px; no new independently demonstrated styling defect justifies source edits.

## U15 reproducible qualification
- **No UI/JS/CSS source changes** on U15. The CI whitelist is restricted to new U15 test, U15 workflow and two documentation files.
- Inherit the full original U3–U7, U11, U12, U13, U14 suites; test PR #18 as-is at U15 exact SHA.
- Add 45 repeated **forward** and 45 repeated **reverse** Tab cycles on each of four viewport sizes, retaining the original PR #17 positive acceptance intent. Check focus never leaves Stage, that traversal is nontrivial, background programmatic focus cannot land, Chromium AX dialog is named and nav is excluded, original Mute reparenting and focus exit restores nav.
- Retain **8 original U15 viewport PNGs** and SHA-256 manifest, and byte-verify all inherited screenshot manifests.
- Confirm U8 immutable blank template still yields 7 NOT_TESTED and one NOT_GIVEN; validator continues to report production NO_GO.
- Do not claim an exact qualified U15 head/run/artifact until **both new exact-head CI runs and original ZIP/image verification** actually complete.

## Human acceptance and production boundaries
The detailed one-test-at-a-time procedure is \`docs/U15_OWNER_ACCEPTANCE_HANDOFF.md\`. Start with real **HU7-01**, and do not record any observation until an actual owner supplies it. The original U8 evidence template is pinned to U7 and cannot be silently repurposed for U15. Do not expose private data, LAN addresses, recordings or personal images in GitHub.

HU7-01–HU7-07 **NOT_TESTED**; HU7-08 **NOT_GIVEN**; real Android Chrome, Samsung Internet, physical keyboard, TalkBack/NVDA, owner visuals and physical microphone/hardware all remain **NOT_TESTED**. **PRODUCTION NO_GO**. No merge, deployment, release, DNS/Cloudflare, credentials, personal/production data or microphone tests.

## Next phase
U16 — Owner Acceptance Evidence Intake & Source-Level Recovery Gate. **NOT STARTED**. Verify the exact qualified U15 source, original CI/artifact custody and first genuine owner test before any acceptance claim or further remedial code.
