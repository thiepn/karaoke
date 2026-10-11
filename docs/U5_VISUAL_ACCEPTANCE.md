# ROOMTONE U5 — AFTER HOURS Visual Acceptance

## Source and branch
Stacked directly on U4 PR #4 at exact head
56399f8c47030eb8540fffd0dbf71b5f62635872. This source-only UI track
is distinct from hardware-documentation PR #6 and audio A1/A3 drafts.

## Delivered
- Immersive lyric-first stage with enlarged timed lyric typography, true playback
  state, first-song import affordance and polished focus layout.
- Existing local song queue now filters all / synchronized lyrics / needs lyrics
  using original LRC metadata rather than duplicating storage.
- Desktop/tablet stage queue and mixer become opt-in mobile sheets on narrow screens,
  keeping the emergency mute visible, close focus restored and original mixer active.
- Studio and Setup retain recording, detailed effects, calibration and safety controls.
- Exact-head Chromium test generates 15 original desktop/tablet/mobile screenshots
  with SHA-256 and retains independent U3/U4 screenshots without rewriting goldens.
- No recording engine, microphone permissions, DSP, storage, or upload changes.

## Evidence gates
| Gate | Status |
| --- | --- |
| U4 ancestry and unchanged audio source | Exact-head CI required |
| U3/U4 browser regression | Exact-head CI required |
| Stage, LRC, song filters, mixer and focus | Exact-head CI required |
| Desktop/tablet/mobile original screenshot inspection | Await artifact review |
| Human screenshot acceptance | NOT TESTED |
| Physical phone / assistive technology | NOT TESTED |
| Real QuadCast/Pebble, acoustic feedback or round-trip latency | NOT TESTED; out of scope |
| Hosted HTTPS and persisted-data acceptance | NOT TESTED |
| Owner production approval | NOT GIVEN |

## Release boundary
Draft only; no merging, deploying, preview enablement, release workflow dispatch,
DNS/Cloudflare/settings edits, production data or credentials. The stated future
destination is karaoke.thiepn.dev. Cloudflare Pages production monitors main:
never sequentially merge stacked drafts without an explicitly approved release
strategy that prevents unintended multiple production deployments.

## Next phase
U6 — Original Screenshot Remediation & Human UX Acceptance. Review the real U5
screenshots, correct reproduced layout/accessibility defects on a new stacked
source-only draft, exact-head CI, then obtain genuine owner device evidence.

## Screenshot-driven U5 correction
Initial U5 Chromium screenshots showed the previous full-width fixed emergency
mute obscuring Stage transport and Songs controls while a second U5 dock floated
above it. The corrective commit places Queue/Mixer and the still-visible panic
mute side-by-side in one restrained fixed bottom rail at narrow widths. An
original Chromium DOM-geometry test checks that the two rails do not overlap.
Visual inspection is not a substitute for owner acceptance on a physical phone.
