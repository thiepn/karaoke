# ROOMTONE U11 — After Hours UI Iteration

## Intent and qualified source
The owner explicitly opted out of HU7-01 real-device testing and requested more
UI development instead. This is not an approval, failed test or waiver.
The existing HU7-01–HU7-07 statuses stay NOT_TESTED; HU7-08 NOT_GIVEN.
**Production NO_GO.**

U11 stacks on qualified U10 draft PR #13 head:
323100e83340ccfae2bb4c599d10705fc8499d21.
U10 push CI 38173738655 and PR CI 38173757635 both passed at that
exact head. Every U1–U10 ancestor is still draft and unmerged. Main stays
9b1d39f6110baf581f4598d45b2fba1258db18c0.

## Screenshot-grounded U7 problems selected for polish
Inspected original U7 screenshot evidence (not human owner testimony):
- 390px Songs shows the search followed by four narrow, heavily line-wrapped
  action buttons crowded on one row. The original controls are retained
  verbatim; U11 makes a two-column/two-row layout with 49px minimum height.
- 390px Studio shows four vocal presets in an uneven three-across plus
  one orphaned row. U11 uses two equal columns, giving each preset a
  minimum 48px target.
- Stage and sidecar surfaces use many competing outlines with relatively
  little differentiation for the actual vocal line, current track and
  primary Play control. U11 applies a restrained flat late-night palette,
  centered lyric focus area, readable track/status and a clearer Play action
  without moving transport, Queue/Mixer, safety buttons or sliders.

## Technical scope
Exactly five files relative to U10:
1. index.html: **only one** new stylesheet link after u7.css; all other
   markup, inline player logic, audio DSP, microphone and local storage
   bytes are identical to the U10 parent.
2. u11.css: additive, responsive visual treatment for Stage, Songs,
   Studio and Setup, intentionally keeping the original U7 empty-LRC
   focus cue/safety mute geometry authoritative.
3. tests/u11_smoke.py: synthetic Chromium 1440/768/390/350px original
   visual evidence plus geometry, touch-target, no-overlap and
   original-control-preservation assertions. A generated local WAV is
   used to populate existing queue; no production data, microphone
   permissions, recorded vocals or physical monitoring.
4. .github/workflows/u11-ui.yml: read-only exact-head ancestry and strict
   source scope, inherited U3–U7 browser tests and U11 test, original
   PNG SHA-256 verification, 30-day artifact retention.
5. docs/U11_VISUAL_ITERATION.md: this audit/evidence and custody note.

No u11.js, replacement audio logic or new settings schema. U2–U7 CSS/JS,
original U8–U10 witness validator, and the original blank evidence
template remain exactly unchanged.

## Acceptance scope and truthful labels
Passing Playwright means only synthetic browser geometry, keyboard,
interaction surfaces and locally generated screenshots were checked.
It is not physical Android Chrome, Samsung Internet, desktop owner
visual approval, external keyboard, TalkBack/NVDA, audio hardware,
native monitoring or production-origin acceptance.
Original screenshots are not approved screenshot goldens and should
not be silently made into them.

HU7-01–HU7-07: NOT_TESTED.
HU7-08: NOT_GIVEN.
Physical/owner acceptance: NOT TESTED.
Production release: NO_GO.

## Next phase
**U12 — Stage & Library Density Refinement and Visual Artifact Audit.**
Status NOT STARTED. Use U11 original rendered PNGs and exact-head CI
to identify genuine visual problems and fix only confirmed issues.
Preserve every human gate, all earlier drafts, no merge/deployment
or production financial/audio data access.
