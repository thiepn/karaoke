# ROOMTONE U21 — Actual Local-HTTP Offline & Focus Resilience

## Pinned inheritance and original source custody

This is an **isolated, source-only test phase**, not a real-world/production release or a new feature. Parent is qualified U20 draft PR #24 exact SHA `66b9406e247c13dfc2def70c22a2f765bb1b353f` on preferred U14 PR #18 descendant line. Prior PRs #17–#24 remain open, draft, separate and unmerged, including unrelated competing PR #20. Production `main` stays at `9b1d39f6110baf581f4598d45b2fba1258db18c0`.

- Original U20 [push CI 38214416253](https://github.com/thiepn/karaoke/actions/runs/38214416253), job 114695692894, and [PR CI 38214419781](https://github.com/thiepn/karaoke/actions/runs/38214419781), job 114695702973: **SUCCESS, all original job steps**, including 548 synthetic Chromium assertions per run and all inherited U3–U19 suites.
- Original push [artifact 11732110253](https://github.com/thiepn/karaoke/actions/runs/38214416253/artifacts/11732110253), ZIP SHA256 `8180037563f8f6e95c0bc23f8081d659838ad09e9a2131eed6e3b6b62ebe89ba`. Original PR [artifact 11731166249](https://github.com/thiepn/karaoke/actions/runs/38214419781/artifacts/11731166249), ZIP SHA256 `872ccf03b02820e3960edeedf609ce6fa947acdb5ebdb1a6aa1af400ad6ed0fe`.
- Both artifacts independently freshly downloaded; ZIP hashes match the original CI logs, CRC clean, **174 original PNGs**, and **165 original per-PNG SHA256 entries across 12 manifests** matched per ZIP. No "recreated" original screenshots substituted.
- U19's original deliberately failing [run 38185861483](https://github.com/thiepn/karaoke/actions/runs/38185861483) proved an inherited source modal lock/hidden Stage defect fixed in original U19.
- U20 original failure [38214281757](https://github.com/thiepn/karaoke/actions/runs/38214281757) was a synthetic `about:blank` invalid URL history-replace **test harness issue**, not an app bug.
- U20 explicitly **NOT TESTED** actual OS background-tab hidden state: the headless tab did not report hidden at any of four viewport widths. No synthesis or automatic approval.

## U21 execution and limitations

Unlike inherited `page.set_content` tests, new `tests/u21_offline_focus.py` serves the **original unmodified ROOMTONE source** from a disposable ephemeral `127.0.0.1` port and uses actual Playwright Chromium `BrowserContext.set_offline(True)`. All new browser contexts are fresh; origin requests are confined to this local server, whose temporary test headers use `Cache-Control: no-store`. No user browser, real server, secrets or production data are involved.

Across **1440x900, 768x1024, 390x844, 350x740**, test:
1. Genuine local HTTP 200, all original modular stylesheets and scripts and no remote app resource requests.
2. Actual browser network-offline state and blocked new `fetch` requests.
3. On already loaded app, Stage modal ARIA, Chromium AX tree, Mute ownership, Tab and Shift+Tab containment, Escape and background inert release.
4. Offline Songs/Studio/Stage navigation with correct actual heading keyboard focus.
5. Actual second-tab switch and `document.hasFocus()/visibilityState`, recorded as **NOT TESTED** if headless Chromium does not genuinely report transitions. Never equate dispatched events or a synthetic hidden flag with real OS suspend/resume.
6. **Offline fresh reload** is separately observed: with no service worker and the test's no-store policy, failure is transparently classified as **OFFLINE_RELOAD_UNAVAILABLE**, not a new user-specified requirement or success. "Works offline after loading" only supports in-session offline operation; not an unsupported guarantee of persistent reload after disconnect.
7. Real network restoration and a fresh HTTP navigation, restoration of safe modal/scroll/inert state and persistence of one clearly **synthetic** isolated localStorage sentinel. No personal user records or microphone/recording operations.

Each CI run creates 16 original new U21 viewport PNGs and a SHA256 manifest; inherited browser suites U3–U20 run from their original unmodified source, and all inherited per-phase SHA256 screenshot manifests are verified.

**Allowlist:** only new `tests/u21_offline_focus.py`, `.github/workflows/u21-ui.yml`, and this file. The GitHub Action performs byte-exact checks for **every tracked inherited U20 file**, including original U8/U16 witness templates, app JS/HTML/CSS, audio/media logic, screenshot goldens and previous test scripts. A new application edit is justified **only** if a new source defect is reproduced and isolated; unrelated functionality must never be changed.

## Acceptance and release

HU7-01–HU7-07 **NOT_TESTED**, HU7-08 **NOT_GIVEN**, production **NO_GO**. Real Android Chrome, Samsung Internet, physical keyboard, TalkBack/NVDA, OS visibility lifecycle, microphone/audio hardware, deployed origin, personal media and private owner testimony **NOT TESTED**. Do not merge, deploy, create resources, change Cloudflare/DNS, access credentials or modify production/private data.

**Exact-head push and PR CI and original ZIP re-verification are pending** until independently completed. Do not report PASS based solely on fixture creation.

## Following phase

**U22 — Targeted Real-World Acceptance Gap & Source Regression Decision**: NOT STARTED. Determine whether additional source work is justified by actual reproduced faults, rather than opening unlimited documentation-only phases; preserve all human gates unless genuine device witness reports exist.
