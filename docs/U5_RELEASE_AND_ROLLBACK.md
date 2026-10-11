# ROOMTONE U5 — Owner-controlled release and rollback runbook

**PREPARATION ONLY — no release approval and no hardware pass.** The only authorized public destination is **https://karaoke.thiepn.dev**. Never publish or announce a provider-generated hostname as the finished deployment. Any test deployment or release requires a separate explicit owner instruction.

## Independently inspected baseline — 2026-10-11

| Item | Current verified value |
|---|---|
| GitHub repository | `thiepn/karaoke` |
| Production main | `9b1d39f6110baf581f4598d45b2fba1258db18c0` |
| Current successful Cloudflare Pages **production** deployment ID | `8fef111f-5aab-4bdd-941c-72abecc0ebef` |
| Current deployed commit | `9b1d39f6110baf581f4598d45b2fba1258db18c0` |
| Cloudflare Pages project | `roomtone-karaoke`, linked to `thiepn/karaoke`, production branch `main` |
| Cloudflare Pages automatic production deployments | **ENABLED** on `main` |
| Preview deployment setting | `none`; skipped preview records are not successful releases |
| Verified custom domain | `karaoke.thiepn.dev`, active |
| U1 PR #1 | `4ef9ed958b94c49ff283c7b8a090efcbaa20f46e`, draft, base `main` |
| U2 PR #2 | `8418201fe02cb1cf485670858651a904df7fc2e5`, draft, base U1 |
| U3 PR #3 | `dbb5e26687d4faa04c14a307ab7fbce3616e0fb6`, draft, base U2 |
| U4 PR #4 | `56399f8c47030eb8540fffd0dbf71b5f62635872`, draft, base U3 |
| U4 push CI | [38099117158](https://github.com/thiepn/karaoke/actions/runs/38099117158) — successful at exact U4 SHA |
| U4 PR CI | [38099119786](https://github.com/thiepn/karaoke/actions/runs/38099119786) — successful at exact U4 SHA |
| Original retained evidence | Push artifact `11686614550`; PR artifact `11686399958`. Both original ZIPs have 21 PNGs + 12-image hash manifest, integrity checked against successful log ZIP digests. |
| H1 physical QuadCast/Pebble checks | **NOT TESTED — mandatory release blocker** |
| Release approval | **NOT GIVEN** |

This runbook must be revalidated against the **current** GitHub branch tips, Actions jobs, Cloudflare settings and actual production deployment before any future cutover. No credential, billing or DNS changes are needed to perform U5 preparation.

## Source and persistence invariants

U5 changes only release-preparation docs, a read-only source gate and an isolated GitHub Actions check. It must **not** modify the validated U4 application (`index.html`, `u2.js/css`, `u3.js/css`, `u4.js/css`) or its regression scripts. Verify against the exact U4 ancestor.

The application stores sound presets at `roomtone:user-sound:v1`, timed lyric drafts under `roomtone:track-draft:v1:` and practice history under `roomtone:practice-history:v1`. Audio media is loaded from local files and **not retained in localStorage**. Preserve these schemas across the release, avoid clearing browser storage, and check existing settings/lyrics/history on **karaoke.thiepn.dev** after an approved promotion. Do not copy or export private user storage to CI.

## Release gates (all required)

1. H1-A..E genuinely passed on the owner’s Windows QuadCast + Pebble V3 using the exact candidate commit, evidence retained privately.
2. U1–U4 stacked ancestry reconfirmed; owner reviews and signs off all relevant diffs and unresolved issues. Review U5 docs as separate draft.
3. Exact target SHA requalified via source checks and comprehensive Chromium at desktop/tablet/mobile. No masked failures or changed goldens.
4. Human keyboard/screen-reader review, recording output review, fail-safe mute, mobile usability, and same-origin storage/recovery acceptance completed. All unresolved release blockers documented.
5. Backup/recovery plan and known-good deployment ID verified; production settings, limits and release protection eligibility inspected read-only.
6. Explicit owner authorization specifies **what qualified commit** may go live and confirms **karaoke.thiepn.dev only**. No approval is implied by a green workflow, PR review or an on-screen checklist.

## Safe promotion — SINGLE production update (owner-approved future operation)

**Warning:** Cloudflare currently automatically deploys **each push to `main`**. Thus **do NOT sequentially merge PRs #1 → #2 → #3 → #4 into `main`**. That would produce multiple unapproved intermediate live versions.

The safe default is:
1. Review stacked changes in dependency order: U1, U2, U3, U4, then U5; leave all drafts unmerged until full acceptance.
2. After owner authorization, prepare a separate **aggregate release PR** comparing the **final qualified U5 descendant** directly to `main` (or one explicitly owner-authorized exact-head fast-forward). Revalidate the entire combined diff, existing `main` HEAD, and one-time production delivery mechanism. Do not let earlier draft PRs trigger intermediate main pushes.
3. Have the owner explicitly authorize **one** main update at the qualified aggregate SHA. Automatic Git→Pages deployment will then run from `main`; inspect the actual build result and exact deployed commit. Do not change DNS or add any other domain.
4. On **https://karaoke.thiepn.dev**, verify HTTP/HTTPS load, U5 UI/scripts present, default mic monitor OFF, prior same-origin settings, local song import, live lyric timing, recording review, emergency mute, and Windows hardware behavior. Check logs and reload/interruption recovery. Confirm that no private clips were sent to a backend.
5. Mark release **accepted** only after owner reviews this evidence. If acceptance fails, execute the owner-authorized rollback immediately.

A Pages-managed internal hostname may still exist even with a custom domain; it must never be treated as a user-facing target. If the owner requires technical exclusion of all provider hostnames, verify that control separately before approving a release; do not assert that canonical links alone make those hostnames inaccessible.

## Rollback — prepare now, EXECUTE ONLY on explicit owner approval

**Known-good point:** Cloudflare Pages production deployment ID `8fef111f-5aab-4bdd-941c-72abecc0ebef` serves commit `9b1d39f6110baf581f4598d45b2fba1258db18c0`.

1. Stop further release actions; capture failing commit, error timestamps and sanitized evidence. If acoustic feedback occurs, **Esc/mute or stop microphone immediately**. Do not use the UI during a hazardous audio condition.
2. Reverify the known-good deployment remains successful and rollback-eligible. Obtain owner approval for the Cloudflare Pages **Rollback to previous successful production deployment** action targeting the verified known-good ID. No DNS changes.
3. Since Git-integrated `main` auto deploys, plan a separately authorized **source reconciliation** (controlled revert PR or owner-approved recovery commit). Otherwise a later main push could silently reintroduce the broken version. Never force-push or rewrite `main` without explicit authorization.
4. Verify **karaoke.thiepn.dev** serves the expected known-good version with real browser checks, while preserving browser localStorage and user files. Only then resume release work.
5. If Pages rollback is unavailable, treat this as **BLOCKED**; request owner approval for a single Git-based restoration of the known-good commit and verify the resulting deployment. Do not claim successful rollback merely because an API accepted a request.

No production promotion, Pages rollback, preview enabling, merges or configuration/DNS changes are part of U5.
