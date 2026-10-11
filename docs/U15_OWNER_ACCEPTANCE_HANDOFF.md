# ROOMTONE U15 — Owner Android, Samsung Internet, Keyboard & Accessibility Handoff

## Scope and evidence truth
This is a **human-executed** plan, not an executed acceptance report. All HU7-01 through HU7-07 remain **NOT_TESTED** and HU7-08 remains **NOT_GIVEN** until an actual owner reports an actual observation. CI screenshots, Chromium AX trees, and automated keypresses are not human device/assistive-technology observations. Production stays **NO_GO**.

The preferred source candidate is the isolated U15 draft stacked on U14 PR #18 at **U14 parent SHA** `194862723f2c89d72ad32b0f8591476095197e50`. The U15 candidate must be identified by its **own complete exact qualified head SHA** from its passing push + PR CI; DO NOT use the branch name alone or production. PR #17, PR #18, U13, main and audio/hardware branches remain unmerged, unchanged, and independent.

## Safe offline/local environment — owner, not ChatGPT, executes
1. On an **owner-controlled** desktop with a separate checkout of `thiepn/karaoke`, verify the GitHub U15 draft head is exactly the qualified SHA recorded in that PR. Do not use `main`, the public deployment, or an unqualified PR merge commit.
2. Export a **source-only disposable copy** with `git archive <FULL_QUALIFIED_U15_SHA> | tar -xf - -C <EMPTY_REVIEW_DIRECTORY>`. Check that the directory was originally empty and contains no .git directory, private files, personal WAV/LRC, credentials, or recordings. Do **not** expose the working repository or user home directory over HTTP.
3. If testing on a trusted personal Wi-Fi LAN, run `python -m http.server 8765 --bind 0.0.0.0` **from that disposable directory only**, with appropriate firewall restrictions, then open `http://<YOUR_COMPUTER_PRIVATE_LAN_IP>:8765` on the Android device. Do not publish/reverse-proxy this service or modify production Cloudflare/DNS. Stop with Ctrl+C after the session. A LAN HTTP origin is **not HTTPS**, can lack browser secure-context features, and is not valid for release, microphone, origin/storage or permission acceptance.
4. Alternative for a computer-only keyboard/NVDA test: use `http://127.0.0.1:8765` from the computer. Do not grant microphone permissions, start monitoring or record voice. For song search, prepare only a disposable locally generated audio fixture and optionally a synthetic LRC; keep it out of GitHub PR/CI and dispose of it after testing.
5. Record the full tested SHA, local date/time with UTC offset, actual device model, OS, browser/version, orientation, observer identity and observed PASS/FAIL description **privately**. Screenshots are optional and private; avoid screenshots with user data or network identifiers. Never commit raw screenshots, recording files, addresses or observations to source control.

**The original U8 owner template is pinned to the U7 exact source SHA and is intentionally immutable.** Do not replace its SHA or scenario labels, and do not claim its validator accepts U15 witness evidence. Maintain U15 observations in a separate private record with U15's exact SHA, for owner review only. A technically valid record is not proof that a human performed the test.

## Exact one-at-a-time owner acceptance sequence

| Original ID | Real device / tool | What the owner actually observes | Outcome now |
|---|---|---|---|
| HU7-01 | Android Chrome, portrait | Stage scrolling; Queue/Mixer row above Stage; Mute in navigation then in fullscreen; no transport overlap, clipping or obscured Play/Next; visible lyrics/practice | NOT_TESTED |
| HU7-02 | Android Chrome | Queue open/close, Mixer open/close, real focus recovery, no overlay trapping/obscuring actions; keep monitoring OFF | NOT_TESTED |
| HU7-03 | Android Chrome | Songs search, lyric-ready/needs-lyrics filters, empty result and clearing search, synthetic local WAV and optionally LRC; test only local files | NOT_TESTED |
| HU7-04 | Samsung Internet | Repeat Stage, fullscreen and scroll, Queue/Mixer, original Mute visibility/placement, no viewport clipping | NOT_TESTED |
| HU7-05 | Desktop Chrome | Actual 1440px Stage and Songs, fullscreen with lyrics placeholder and later synthetic LRC, queue actions, contrast and scrolling | NOT_TESTED |
| HU7-06 | Physical keyboard | Tab and Shift+Tab repeatedly while fullscreen; none may focus background queue/headers; Enter/Space toggles expected button, Escape closes focus, focus resumes outside. **Actual monitoring mute remains hardware-untested** | NOT_TESTED |
| HU7-07 | Real TalkBack on Android and/or NVDA desktop | Fullscreen Stage announced as named dialog, background nav not traversable, Mute identifiable, heading/lyric/status and Songs search/filter status announced correctly, focus restored on exit | NOT_TESTED |
| HU7-08 | Owner explicit decision | Separate aesthetic and release-readiness assessment, with dated evidence and exact source SHA; approval is never inferred from preceding PASS reports | NOT_GIVEN |

**Execute one scenario at a time.** Do not mark subsequent scenarios complete because earlier ones passed. For any FAIL, report the exact control, visible label, keypress/gesture, attempted and actual behavior, and viewport. Do not mark PARTIAL as PASS; unresolved or unsupported test capability stays NOT_TESTED.

### First actual owner prompt — HU7-01 only
On the exact qualified U15 source copy, open Android Chrome on the user's real phone in portrait without granting microphone access. Locate Stage. Scroll from Queue/Mixer controls to lyrics and Play/Next/practice; inspect the original emergency Mute in the navigation. Open focus view, scroll the Stage internally from top to bottom, then exit. Report **PASS** only if the same original Mute remains reachable, no controls are obscured, and scrolling/lyrics are usable. Otherwise report **FAIL** with the observed obstruction; if not performed, retain **NOT_TESTED**.

Record locally:
- Tested full commit SHA, device/Android/browser version, portrait viewport if known;
- local time with UTC offset, observer, HU7-01 result (PASS/FAIL/NOT_TESTED);
- one-sentence observed result and (for failure) the exact location/control; optional private screenshot identifier and hash.

**Stop after HU7-01**. The assistant must not turn a proposed procedure into a human observation or proceed to HU7-02 before the owner supplies the real result.

## Separate unresolved acceptance
Real microphone, monitor loudspeaker, acoustic feedback, real recording, hardware latency, actual Samsung Internet, real TalkBack/NVDA and deployed-origin behavior have not been tested by synthetic CI. A clean accessibility tree does not certify actual screen-reader usability. Release is prohibited until independently authorized human and production gates are addressed. No merge/deploy, DNS/Cloudflare, secrets, production data or real microphone use is authorized by this document.
