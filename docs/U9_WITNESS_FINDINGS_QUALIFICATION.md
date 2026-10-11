# ROOMTONE U9 — Witness Integrity & Residual Visual Qualification

## Baseline reverified before editing
- Originally requested U8 SHA 329d72227cb0d0f7125ef4c84c748a1629f4bfbe:
  exact-head push 38168257192 PASS and PR 38168280588 PASS.
- U8 draft PR #11 **subsequently advanced by two commits** to exact head
  fe9208b20ed8136c895946754ae258962280c025, ahead without divergence.
  Latest exact-head push 38168441668 and PR 38168445797 both PASS.
- Current U8 changes since the old SHA are limited to its evidence-format
  validator and tests. They explicitly reject raw external evidence paths.
  U9 stacks on this *newer qualified U8 head*, not the obsolete original.
- U1 through U8 (main, U1, U2, U3, U4, U5, U6, U7, current U8) ancestry
  verified linear. No ancestor branch has been modified by U9.
- Original visual archive is U7 exact head
  1c7dd8f10d2f2fcccf1ac9809f1f9047dbb4666a;
  U7 push/PR workflows and retained image manifests were qualified earlier.
  U9 produces no new screenshots and makes no claim of new visual acceptance.

## Owner human findings search
Check available conversation claims and original PR #8, #9, #10 and #11
merged discussions/reviews. There are no genuine HU7-01–HU7-08 observed
device/owner outcomes. This finding is restricted to accessible evidence:
an unreported private test cannot be treated as either PASS or FAIL.
The committed U8 blank evidence template remains unchanged.

| Gate | Description | Observed outcome |
| --- | --- | --- |
| HU7-01 | Android Chrome portrait, Stage safety rail and fullscreen | NOT_TESTED |
| HU7-02 | Android Chrome Queue/Mixer sheet and focus recovery | NOT_TESTED |
| HU7-03 | Android Chrome Songs filtering and synthetic LRC | NOT_TESTED |
| HU7-04 | Samsung Internet mobile Stage and safety | NOT_TESTED |
| HU7-05 | Desktop human visual review | NOT_TESTED |
| HU7-06 | Actual external keyboard traversal | NOT_TESTED |
| HU7-07 | Actual TalkBack / NVDA screen reader | NOT_TESTED |
| HU7-08 | Explicit owner visual/release decision | NOT_GIVEN |

## Reproduced source-only defects and narrow corrections
1. **Malformed status causes uncontrolled traceback.** U8's
   'if status not in allowed' uses Python set membership before verifying
   type. Status '[]' or '{}' is unhashable and raises TypeError, which U8
   CLI does not catch; it exits with a traceback instead of a controlled
   REJECTED_INTAKE response. U9 checks type first and raises ValueError.
   The regression tries list, dict, nested list/dict, null and integer.
2. **Mutable scenario permits mislabeled gate.** U8 only checked that a
   scenario description was 20+ characters. A private intake could relabel
   HU7-01 as a different, easier test and still appear structurally valid.
   U9 pins each of the eight scenarios to the existing U8 blank template's
   *exact* original wording. Observations belong only in the observation
   field; the template itself and all statuses remain unchanged.
   Synthetic test cases demonstrate rejection of HU7-01 and HU7-08
   substitute descriptions.

These are **intake integrity** defects, not proof of a user-facing visual
failure. Do not edit karaoke player, existing Stage/CSS/HTML, permissions,
LRC/audio, microphone, recording, production data, screenshots or tests of
the original musical hardware. The validator still explicitly returns
human_truth_verified=false and release=NO_GO even for structurally complete
synthetic examples. These checks cannot verify that a user genuinely
performed a test or approved anything.

## Automated qualification
New U9 exact-head workflow:
- require ancestry from exact latest U8 qualified SHA;
- permit only validator, matching tests, this document and U9 workflow;
- require U8 template, app index, CSS, JS and older UI sources byte-for-byte
  unchanged relative to the pinned U8 SHA;
- execute original U8 intake tests, including new malformed-status and
  scenario-substitution cases; assert the blank template still yields
  claimed_observed=0 / human_truth_verified=false / release=NO_GO.
The workflow makes no screenshot, artifact or release dispatch and has
read-only GitHub permissions.

## Residual visual qualification decision
NO NEW VERIFIED HUMAN FINDINGS. No additional UI defect can be inferred
from the absence of owner testimony. U7's 16 original 1440/768/390px browser
screenshots and automated geometry checks remain retained in U7 artifacts,
but they cannot substitute for actual physical-device, external keyboard,
assistive technology, or owner visual acceptance. U9 **does not close**
any real-device gate.

## Next owner-led test (one at a time)
HU7-01 remains the immediate task. Use **exact U7 UI SHA**
1c7dd8f10d2f2fcccf1ac9809f1f9047dbb4666a in an isolated, owner-controlled
local test origin, not live production. In Android Chrome portrait visually
verify that Queue/Mixer remains in flow above Stage, navigation Mute is
visible and does not cover Play/Next, and fullscreen/scroll presentation is
legible. Do not enable microphone or recording. Supply an actual
PASS/FAIL/NOT_TESTED account and any specific overlap/focus observations;
time, device and browser if available. Then and only then proceed HU7-02.

## Release / change gates
U9 human gate closure BLOCKED. Retain U1–U8 and hardware/audio drafts.
No merges, deployment, release dispatch, Cloudflare/DNS/secret changes,
preview enablement, physical microphone test or protected/production data
access. Main remains untouched. **PRODUCTION NO_GO.**

## Suggested U10
U10 — Owner Witness Triage & Visual Evidence Reconciliation.
Status NOT STARTED. Verify U9 exact-head source-only tests and check real
HU7-01–HU7-08 testimony. Repair only newly demonstrated source defects
in a separate draft. Preserve NO_GO until independent owner approval.
