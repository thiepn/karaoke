# ROOMTONE U16 — Private Owner Witness Intake & Recovery Gate

## Exact source custody and unresolved human truth

This U16 source-only child of qualified U15 PR #19 is **not** owner acceptance. Candidate app source is the exact verified U15 SHA `df3dc97134492974f210c6f6b96f20877dd441d4`. U16 changes no app code; its additions are a local private witness format checker, blank U16-specific template, negative unit tests, read-only CI and this document. U14 PR #17 and #18 are separate unchanged drafts; PR #18 is the preferred modal/AX source underneath U15. The separate PR #20 is a competing U15 diagnostic based on PR #17, not merged into this source line.

Requested inherited original U15 proof:

- [U15 push CI 38181296489](https://github.com/thiepn/karaoke/actions/runs/38181296489) SUCCESS at exact `df3dc97134492974f210c6f6b96f20877dd441d4`, [PR CI 38181298942](https://github.com/thiepn/karaoke/actions/runs/38181298942) SUCCESS, all jobs and steps.
- [Push artifact 11718026758](https://github.com/thiepn/karaoke/actions/runs/38181296489/artifacts/11718026758) verified ZIP SHA256 `d90312a065d9d9be2ab5ac6394fd81245a094ad07c48a2ee382985d20d16b0b3`.
- [PR artifact 11718610505](https://github.com/thiepn/karaoke/actions/runs/38181298942/artifacts/11718610505) verified ZIP SHA256 `80d418c2011818446a11d8e846fce10126fce4a8e479e79530a5275546b00ae0`.
- Both ZIPs downloaded in U16 verification: independent exact SHA256, clean ZIP CRC, all per-image PNG SHA256 manifests matched. Each contains 134 original PNGs: U15 8, U14 16, U13 12, U12 16, U11 14, U7 16, U6 16, U5 15, U4 12 and nine U3 originals. New U15 350/390/768/1440 focus and scrolled viewport PNGs inspected, not human observations.
- PRs #10–#19: accessible original issue comments contained **no owner observation**. Relevant U14/U15 PR reviews contained no approval; the committed U8 template remains empty. An observation not available in these checked sources cannot be classified as a PASS or FAIL. **Seven HU7 cases NOT_TESTED and HU7-08 NOT_GIVEN; production NO_GO.**

## Scope and provenance of the distinct private U16 intake

The original `docs/evidence/U8_HU7_ACCEPTANCE.template.json` and `scripts/u8_validate_evidence.py` are immutable in this draft. They are pinned to *U7*, not U15/U16, and must never be edited, used to imply new owner testimony, or silently relabeled.

New `docs/evidence/U16_HU7_LOCAL_INTAKE.template.json` is a **blank public source template** explicitly pinned to the qualified `df3...` U15 app candidate, with the original eight U8 scenario descriptions copied *exactly*. Keep this blank checked-in file unmodified. Its direct observation attestations are all false, the 7 test statuses are NOT_TESTED, owner approval is NOT_GIVEN, and release_gate=NO_GO.

When (and only when) a real owner actually performs HU7-01:
1. Privately copy the U16 template to an owner-controlled location **outside GitHub/repo/Actions**, e.g. `~/private-roomtone/hu7-01.json`. No real screenshot or observer name belongs in source control.
2. Populate **only** the HU7-01 `status` (PASS or FAIL) and `observer`, `observed_at` (ISO-8601 with UTC offset), `device`, `browser`, `observation`, `attests_direct_observation=true`. Include an optional opaque private screenshot pointer like `owner-local:visual001` **paired** with its lowercase SHA256; do not include paths, URLs, photos or recording bytes.
3. Preserve the exact source pin and the remaining HU7-02–HU7-07 NOT_TESTED / HU7-08 NOT_GIVEN. The new validator deliberately rejects premature results for later tests, changed scenarios, claimed approval, invented future timestamps, malformed fields, raw paths/URLs and altered NO_GO.
4. On the owner's own computer, validate the **local file**:
   `python scripts/u16_validate_private_witness.py ~/private-roomtone/hu7-01.json`
5. The output says `VALID_SHAPE_UNVERIFIED` and **human_truth_verified=false**, **owner_approval_verified=false**, **release=NO_GO**, even when an owner claims PASS. It does **not** print the owner name, observation text, device details or screenshot pointer. Only human review of the original private record can establish truth. Do not commit the record, upload it, paste personal content into PR comments or copy it into this repository.
6. Invalid input returns `REJECTED_INTAKE`. Correct the private report if appropriate; never modify the original U8 template or "repair" an invalid observation by inventing testimony. If the owner reports a genuine UI FAIL, retain NO_GO and initiate a separate source-level defect triage with exact reproduced steps before considering a code change.

The U16 template's `candidate_source_sha` intentionally pins the **application source tested** at U15, even though the owner may operate U16's added local validator from another directory. The owner must record which exact checkout of the UI they actually opened; never assume a production URL or a PR merge commit is equivalent.

### HU7-01: First actual owner test (do not proceed to HU7-02)

Use the U15 qualified exact SHA `df3dc97134492974f210c6f6b96f20877dd441d4` for a disposable **source-only** local UI copy, not `main` or a deployed website. From an owner-controlled checkout, export only that SHA into a new empty private review folder:

```bash
mkdir -p roomtone-owner-review
git archive df3dc97134492974f210c6f6b96f20877dd441d4 | tar -xf - -C roomtone-owner-review
cd roomtone-owner-review
python -m http.server 8765 --bind 0.0.0.0
```

Only on a trusted, firewall-restricted private LAN, open `http://<desktop-private-LAN-IP>:8765` in the owner's Android Chrome. This is plain HTTP, not a production HTTPS or permission/audio acceptance origin; do not enable the microphone, monitoring or recording. Do not serve a checkout containing credentials, recordings or personal files. Stop the server when finished.

On the **real** Android device in portrait: inspect the Queue/Mixer row, navigation emergency Mute, lyrics and Play/Next/practice clearance. Enter fullscreen focus, verify the original Mute is visible and usable, scroll the internal Stage to the bottom, and exit. Check no overlap or control is unreachable. Record a real PASS or FAIL with observed behavior, tested full SHA, observer, device/browser/version and local timestamp with timezone. If not performed, record **NOT_TESTED**, not a synthetic result.

No physical tests were executed by U16 CI. HU7-02 Android sheets, HU7-03 Songs+LRC, HU7-04 Samsung Internet, HU7-05 desktop visual, HU7-06 physical keyboard, HU7-07 TalkBack/NVDA and HU7-08 explicit owner approval all remain open until separately performed and evidenced. **Stop after HU7-01**.

## Source recovery and release boundaries

The current source code cannot prove the private identity or veracity of a witness. The validator is *format-only* and works offline using Python's standard library. Negative test fixtures must be labeled synthetic and never copied into owner records. Its CI exercises only blank templates and synthetic data; no real private JSON is uploaded into CI, checked in, logged, or retained as artifacts. Its whitelist forbids application UI/player/audio/DSP, credentials, deployment, Cloudflare/DNS, production data, original U8 acceptance and prior branches.

Any observed HU7 failure requires first-class owner testimony and a new independent reproduction before altering application code. Do not turn a locally well-formed document or clean automation screenshot into a release authorization.

**Production NO_GO. HU7-01–HU7-07 NOT_TESTED; HU7-08 NOT_GIVEN.**

## Next

**U17 — First Owner HU7-01 Observation Triage & Recovery Decision**: NOT STARTED. Revalidate exact U16 source and determine whether any genuine private owner HU7-01 observation is available, without fetching other private data, creating fictional results, merging or deploying. If none is available, remain blocked, with source-only follow-up permitted only for independently reproduced defects.
