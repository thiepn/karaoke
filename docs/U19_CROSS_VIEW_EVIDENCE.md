# ROOMTONE U19 — Cross-View Modal Focus Regression and Owner NO_GO

## Pinned inherited evidence

Qualified parent: [U18 draft PR #22](https://github.com/thiepn/karaoke/pull/22), exact SHA `fe43b5535aec962e2347a6994c6bf4e08b0af00c`. U18 push CI [38184898120](https://github.com/thiepn/karaoke/actions/runs/38184898120) and PR CI [38184918285](https://github.com/thiepn/karaoke/actions/runs/38184918285) both SUCCESS; all steps passed. U18's original [failing CI 38184768879](https://github.com/thiepn/karaoke/actions/runs/38184768879) proved the distinct Lyrics Studio destination-heading focus defect subsequently corrected by U18.

Original U18 artifact verification: push artifact 11718579644 ZIP SHA-256 `818c0f98cf5d8ba42a077cc3115a6595400e477adc4a0289fd80ad9602b90543`; PR artifact 11719212630 ZIP SHA-256 `573de309d6b7fbbb9a52e8d2c3fe0727fe58826f8aeac9b0011d72ce251e6ec4`. Both downloaded afresh; hashes and CRC pass. Each has 142 original PNGs; all 133 per-image hashes across 10 manifests pass.

All previous PRs #17–#22 are independent OPEN DRAFT, unmerged. Preferred U14 source #18 has U15 #19, U16 #21, U18 #22 descendants; competing #20 is a separate PR #17 child. Production main last verified `9b1d39f6110baf581f4598d45b2fba1258db18c0`.

## Actual U19 source regression

Unmodified qualified U18 source was tested with new source-only U19 test/workflow at diagnostic exact head `c1ddc4839cee2550587b04713b5b2ad5d414314b`. Original [CI 38185861483](https://github.com/thiepn/karaoke/actions/runs/38185861483) job 114607918191 failed at 1440 CSS px:

`U19 1440px: studio route releases Stage focus/root lock: {'view': 'studio', 'focus': True, 'locked': True, 'stageHidden': True, 'headerInert': True, 'sidecarInert': True, 'role': 'dialog', 'ariaModal': 'true', 'focusId': '', 'focusTag': 'BODY', 'muteParent': 'stage'}`

The real existing U1 view switch hid Stage without clearing its fullscreen focus or root scroll lock. U14 observed body focus classes, but not the Stage view's hidden attribute, so the visible Studio was excluded from accessibility and the keyboard active element fell to BODY. This diagnostic uses a synthetic programmatic click on the original inert background nav, representing an application-driven workspace transition; it is not physical keyboard or owner evidence.

## Minimal source-scoped correction

The separate `u19.js` observes only visibility changes of the original `#rtStagePanel`. If a genuine view transition hides Stage while it remains in focus mode and the body remains locked, the addon closes through the *existing Focus button*. After inherited U14 modal and inert cleanup, a frame callback focuses the new visible workspace's existing heading. No new routing event, original handler replacement, audio, monitor, mic, lyrics, queue, safety, data, CSS or owner evidence changes. Normal Escape and the existing U18 Lyrics Studio route remain unchanged.

Only five files may differ from qualified U18: new `tests/u19_smoke.py`, `.github/workflows/u19-ui.yml`, `docs/U19_CROSS_VIEW_EVIDENCE.md`, `u19.js` and exactly one `u19.js` defer script line in `index.html`. Older code, U8/U16 evidence templates and validators, screenshot goldens and other drafts must remain byte-identical.

## Browser and original screenshot evidence contract

U19 checks four synthetic Chromium viewports: 1440x900, 768x1024, 390x844, 350x740. Assertions cover modal AX-tree semantics, background inertness, 20 forward and reverse Tabs each, F/Escape keyboard focus return, U18 Lyrics Studio to Songs heading, programmatic Stage to Studio and Songs transitions, restoration of Mute/navigation/scroll/ARIA, repeatability and horizontal overflow. It captures 16 original PNGs and a SHA-256 manifest per run, reruns inherited U3–U7/U11–U15/U18 suites and verifies original per-PNG manifests.

Do not claim U19 CI qualification until both exact-head push and PR runs and independently downloaded original ZIP SHA-256/CRC/PNG manifests pass.

## Owner and release boundaries

HU7-01–HU7-07 remain NOT_TESTED; HU7-08 NOT_GIVEN; production NO_GO. No real Android, Samsung Internet, physical keyboard, screen reader, microphone, audio hardware, user data, release or deployment was tested. The U8 and U16 blank witness templates remain immutable and are not evidence that any real human observed this source. No merging, Cloudflare/DNS, secrets or protected data.

Next: U20 — Modal Lifecycle & Browser Navigation Resilience; NOT STARTED. Extend source-only regression to view changes, browser history and document visibility when independently justified, preserving all real-world acceptance gates.
