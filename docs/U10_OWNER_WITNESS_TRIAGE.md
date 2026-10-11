# ROOMTONE U10 — Owner Witness Triage & Visual Evidence Reconciliation

## Exact-head verification
- U9 draft PR #12 head: \`cdf1d4fac581a176a44e28f5c8b3dffac44c3678\`.
- U9 exact-head push \`38168773351\` and PR \`38168794354\`: both successful (one job each, no failed steps).
- U1→U2→U3→U4→U5→U6→U7→U8→U9 ancestry inspected as a linear stack, no divergent merge bases.
- U8 parent at \`fe9208b20ed8136c895946754ae258962280c025\` is unchanged by U10.
- Main remains \`9b1d39f6110baf581f4598d45b2fba1258db18c0\`.
- PRs #8–#12 returned zero human witness comments/reviews; available GitHub issue records contained only draft PR issues, not external owner evidence. The existing tracked blank template is still unaltered.

## Human evidence state: unchanged, zero actual owner submissions

| Gate | Required actual evidence | Present? | Truthful status |
| --- | --- | --- | --- |
| HU7-01 | Real Android Chrome Stage/mute/transport and scroll | No | NOT_TESTED |
| HU7-02 | Real Android Chrome Queue/Mixer open-close and focus | No | NOT_TESTED |
| HU7-03 | Real Android Chrome Songs filters/LRC | No | NOT_TESTED |
| HU7-04 | Real Samsung Internet interaction | No | NOT_TESTED |
| HU7-05 | Owner desktop review of karaoke UI | No | NOT_TESTED |
| HU7-06 | Actual physical keyboard navigation | No | NOT_TESTED |
| HU7-07 | Actual TalkBack/NVDA assistance | No | NOT_TESTED |
| HU7-08 | Explicit independent owner approval | No | NOT_GIVEN |

An unknown private owner observation is **not** assumed absent in the world;
it is *unverified in accessible records*. Browser-generated PNGs are not
owner-witness evidence. The existing U8 blank intake template and all human
status values remain unchanged, including original U7 code being tested.

## Reproduced remaining integrity defect (not a visual defect)
The qualified U9 evidence validator parses a timezone-aware timestamp but
does **not compare it to current time**. A synthetic structurally complete
report dated \`9999-12-31T23:59:59+00:00\` would be classified as a claimed
observation, despite being chronologically impossible. This is a genuine
intake-integrity defect verified by reviewing the exact U9 source path; it
does not establish any real user observation.

U10 makes **only** a narrow chronological guard:
- Reject an \`observed_at\` more than 10 minutes ahead of the current UTC clock.
- Permit at most 10 minutes of modest device clock skew.
- Check the timestamp in UTC, with offsets preserved.
- Two new **synthetic** regression tests assert rejection of year 9999,
  and permitted small clock skew still returns
  \`human_truth_verified=false\`, \`release=NO_GO\`.
- All previous U9 validation tests continue unchanged, and a source-only
  CI guard checks the template, song app, stage/mobile UI, audio source,
  and previous docs byte-for-byte against the qualified U9 exact SHA.

This is **not a cryptographically trusted timestamp** and is not an
authentication mechanism. A submitter can still type a plausible date.
Validation never certifies a human, never advances any gate and never
generates a deployment or release authorization.

## Visual evidence reconciliation
Prior U7 original Chromium screenshots cover the 1440/768/390px UI,
inherited Stage, Songs and focus states, and demonstrate the earlier fixes.
No *new* genuine defect report or owner screenshot was supplied, so U10
does **not** modify \`index.html\`, \`u7.css\`, \`u7.js\`, \`u6.css\`,
\`u6.js\`, recording, DSP, hardware settings, song persistence or
screenshot goldens. A zero-observation environment cannot prove there
are no residual real-device UI problems; it can only retain their
NOT_TESTED status.

## Required next owner action
**One test at a time: HU7-01.**
Open exact U7 source \`1c7dd8f10d2f2fcccf1ac9809f1f9047dbb4666a\`
in an isolated owner-controlled static local UI preview (not production)
on Android Chrome. Inspect Stage Queue/Mixer placement, persistent Mute
visibility, unobstructed Play/Next/practice and focus mode. Do not
grant microphone permissions or test real audio. Record a witnessed
PASS or FAIL, owner-reported time with offset, device/browser and
specific observed behavior. If not done, retain NOT_TESTED. Continue
to HU7-02 only when HU7-01 is actually reported.

## Source & deployment safety
U10 is a new draft stacked directly on U9 and scoped strictly to:
- \`scripts/u8_validate_evidence.py\`
- \`tests/u8_evidence_intake_test.py\`
- \`docs/U10_OWNER_WITNESS_TRIAGE.md\`
- \`.github/workflows/u10-evidence.yml\`

No prior draft branch, U8 template, visual, audio, hardware, secrets,
Cloudflare/DNS, production data, production deployment, microphone
hardware, recordings, release or original code is modified. CI uses
read-only repository permissions, Python stdlib tests only and no
privileged workflows.

**PRODUCTION NO_GO. All eight human gates remain open.**

## Next phase
**U11 — Owner Witness Intake & Real-Device Findings Response.**
Not started. Only genuine human HU7 findings may support closing
human gates or modifying UI based on field observations. Otherwise
retain the NO_GO decision and prepare one-at-a-time owner handoff.
