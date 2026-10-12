# ROOMTONE U20 — Modal Lifecycle, Browser Navigation and Visibility Diagnostics

## Qualified source custody

U20 is an **additive, source-only browser qualification** on the unchanged source of open U19 draft PR #23 exact head `c9690d35a1087e2c4a967dfc5cc953f097ddbff2`. Previous PRs #17–#23 remain separate open drafts, with PR #18 the preferred U14 line. Production main remains `9b1d39f6110baf581f4598d45b2fba1258db18c0`. No original app HTML, CSS, JS, audio/player/monitor, data, screenshots, or original U8/U16 human-evidence template is altered by this diagnostic.

Original U19 push CI [38186011564](https://github.com/thiepn/karaoke/actions/runs/38186011564), job 114608363029, and PR CI [38186036779](https://github.com/thiepn/karaoke/actions/runs/38186036779), job 114608441483, both **SUCCESS** on the exact U19 head, with every step passed, 306 synthetic Chromium assertions per run across 1440/768/390/350 widths and all inherited U3–U18 suites passed.

Original U19 artifacts downloaded again and independently verified:
- Push [11719990761](https://github.com/thiepn/karaoke/actions/runs/38186011564/artifacts/11719990761), ZIP SHA256 `945a5225f03c040fd7b6df3ad5a8ad145f983f38c89f338508f4c1cce4ae47c5`.
- PR [11719094859](https://github.com/thiepn/karaoke/actions/runs/38186036779/artifacts/11719094859), ZIP SHA256 `23199c691b6cf700f930dc9008c9faeb73cd1b8ba04732c49b39cf02da31b87d`.
- Each original archive has **158 PNGs**, clean CRC, **11 original SHA256 manifests and all 149 per-PNG hashes passed**.

The initial U19 reproduced source defect on uncorrected U18 remains independently preserved as [failing original CI 38185861483](https://github.com/thiepn/karaoke/actions/runs/38185861483): Studio was visible while the prior Stage remained a locked/inert modal with document focus on BODY. The U19 correction remains unchanged.

## New diagnostic scope and limits

The added `tests/u20_smoke.py` drives genuine headless Chromium at 1440x900, 768x1024, 390x844 and 350x740 with ephemeral localStorage and without real microphone/audio, account or production origin. It tests:

- **Three successive fullscreen entry/exit cycles per viewport**, with Stage keyboard/focus containment and one Escape exit, named modal role, aria-modal/label restoration and emergency Mute reparenting.
- **Chromium CDP accessibility tree**, including modal dialog presence and background Songs nav excluded from modal access.
- **Same-document browser history**: native `pushState`, Back, Forward and `replaceState` while Stage is modal. This is *not* app view-router coverage: ROOMTONE does not promise browser-history-driven internal workspace routing, so no such claim or new behavior is invented.
- **Browser visibility**: activate another real Chromium tab and report whether headless Chromium actually marks the Stage page hidden. If it does not, the hidden-tab observation is recorded as NOT_TESTED rather than fabricated. A synthetic `visibilitychange` event is separately tested and labeled as synthetic.
- **Keyboard F/Escape** from the external Stage heading, native Songs → Studio → Stage navigation and the original U19 programmatic modal → Studio recovery. Check destination heading focus, root unlocking and no horizontal overflow.
- **Original screenshot custody**: create 16 new U20 viewport PNGs and per-PNG SHA256 manifest; rerun inherited U3–U19 suites and original screenshot manifests; verify original U8/U16 blank human templates always remain NO_GO.

These are computer-simulated browser checks, not real Android Chrome, Samsung Internet, physical keyboard, TalkBack/NVDA, audio hardware or personal media acceptance.

Only `tests/u20_smoke.py`, `.github/workflows/u20-ui.yml` and this documentation are permitted to differ from the verified U19 head unless a **new independently reproduced application defect** justifies a separately documented minimal fix. Workflow uses an exact-SHA checkout, bytewise comparison of **every prior tracked U19 file**, read-only scope validation and original screenshot hashes.

## Decision gates

**U20 actual browser result is pending CI verification.** If diagnostics pass without a new genuine defect, do **not** patch application source. Do not interpret a missing actual tab-hide state or unimplemented browser-history app routing as an application bug.

Human HU7-01–HU7-07 **NOT_TESTED** and HU7-08 **NOT_GIVEN**. Original U8/U16 templates and validators remain immutable; no private owner records are used. **Production NO_GO**. No merge, deployment, release, DNS/Cloudflare, secrets, production or device data.

## Next

U21 — Extended Offline & Focus Resilience Qualification, **NOT STARTED**, constrained by independent defect evidence and human release gates. Do not perform unbounded source-free testing or invent closure from synthetic browser evidence.
