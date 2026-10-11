# ROOMTONE U6 — Visual Evidence Remediation & Human UX Acceptance Handoff

## Source authority and boundaries
U6 is a source-only UI branch directly stacked on U5 PR #8 at exact parent
9e5c71088a8b078c990dce4083c6957be4b300ea. U1–U5 drafts and audio /
hardware drafts remain unchanged. Original playback, microphone, Web Audio,
MediaRecorder, LRC timing, all song data, queue, and local storage keys remain
inherited from U5. Only two asset references are added to index.html.

## Reproduced issues and corrections
1. Duplicated song numbers: U2 queue mirror displayed the original "2. Title"
   in a row already rendering "02". U6 strips the exact ordinal prefix
   from the mirrored button text ONLY. Original source queue titles and
   playback delegation are unchanged.
2. Conflicting count: U5 filter found one song while inherited U2 status
   reported two. U6 reconciles polite status and filter count to actual
   visible rows after U2/U5 render, including search and new LRC attachments.
3. Invisible empty results: U6 shows an accessible hint when all imported
   songs fail the selected lyrics filter, without duplicating U2 search empty.
4. Mobile recovery: Escape closes the visual panel and restores trigger focus
   WITHOUT suppressing the original Escape emergency mute behavior.
5. Visual polish: modest song row legibility and touch target improvements.

### Further original-screenshot reconciliation
The retained desktop Stage screenshot also shows double ordinals (02 / 2. Song)
in its queue mirror and the library current-song summary. U6 corrects those
presentation labels, preserving the actual authoritative queue title.
The first U6 workflow exposed an overly strict source-title assertion, which
was corrected to verify the existing song identity and preserve source queue
content rather than assume an uncontracted ordinal format.

## Automated evidence
- Exact-head checkout, known U5 ancestor and source-only six-file diff guard.
- Inherited U3/U4/U5 Playwright browser regressions unchanged.
- U6 synthetic local WAV and LRC Chromium checks at 1440x900, 768x1024, 390x844.
  Filtering, labels, live status, playback, mobile queue/Escape/focus, studio,
  setup, overflow, page exceptions covered.
- 16 newly rendered U6 screenshots, original SHA-256 manifest; U3/U4/U5
  screenshot evidence retained. No goldens rewritten or human acceptance
  inferred from screenshot creation.

## Human evidence handoff (all untested)
| ID | Scenario | Status |
| --- | --- | --- |
| HV-01 | Human visual inspection: desktop Stage/lyrics | NOT TESTED |
| HV-02 | Human visual inspection: tablet workspace | NOT TESTED |
| HV-03 | Real Android Chrome: Stage, Songs, mobile sheet | NOT TESTED |
| HV-04 | Real Samsung Internet: same flows | NOT TESTED |
| HV-05 | Real keyboard Tab, filters, queue, Escape focus | NOT TESTED |
| HV-06 | Screen reader active song and filter announcements | NOT TESTED |
| HV-07 | Hosted-origin refresh/retention and safe recovery | NOT TESTED |
| HV-08 | Explicit owner visual signoff | NOT GIVEN |

Record real PASS/FAIL only after actually executing each scenario. Capture
device/browser, exact observations, timestamps, and independently attributable
witness evidence. Browser CI and synthetic screenshots are not human witnesses.

## Release controls
NO_GO. No merge, deploy, release dispatch, DNS/Cloudflare settings, production
data, secrets, preview activation or physical microphone tests. Production
Cloudflare Pages monitors main, so sequentially merging the draft stack would
cause intermediate deployments without a separate authorized release plan.

## Next phase
U7 — Owner Visual Review & Real-Device Evidence Handoff. Start from qualified
U6 draft head, record genuine real-device observations, fix only reproduced UI
defects as a separate draft, and preserve NO_GO pending owner release approval.
