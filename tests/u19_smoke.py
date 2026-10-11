"""ROOMTONE U19: original source Chromium regression, no physical owner/device tests.

Start as a POSITIVE diagnostic on unchanged U18. Any failure must be reproduced
before introducing a separate, minimal U19 JS accessibility layer. All checks
are synthetic: no microphone, monitoring, production origin or personal data.
"""
from pathlib import Path
import hashlib
from playwright.sync_api import sync_playwright

ROOT=Path(__file__).resolve().parents[1]
HTML=(ROOT/"index.html").read_text(encoding="utf-8")
assert HTML.count('src="u14.js"')==1
assert HTML.count('src="u18.js"')==1
STYLES={name:(ROOT/f"{name}.css").read_text(encoding="utf-8") for name in
        ("u2","u3","u4","u5","u6","u7","u11","u12","u13")}
JS={name:(ROOT/f"{name}.js").read_text(encoding="utf-8") for name in
    ("u2","u3","u4","u5","u6","u7","u14","u18")}
if (ROOT/"u19.js").exists():
    assert HTML.count('src="u19.js"')==1
    JS["u19"]=(ROOT/"u19.js").read_text(encoding="utf-8")
else:
    assert 'src="u19.js"' not in HTML
OUT=ROOT/"u19-evidence"
OUT.mkdir(exist_ok=True)
assert not list(OUT.glob("u19-*.png")), "Stale U19 PNGs forbidden"
assert not (ROOT/"u19-evidence"/"sha256.txt").exists(), "Stale U19 manifest forbidden"
assert len(STYLES)==9 and len(JS)>=8

checks=0
def require(truth,label,width):
    global checks
    assert truth, f"U19 {width}px: {label}"
    checks+=1

with sync_playwright() as pw:
    browser=pw.chromium.launch(
        executable_path="/usr/bin/chromium" if Path("/usr/bin/chromium").exists() else None,
        headless=True,args=["--no-sandbox"])
    for width,height in ((1440,900),(768,1024),(390,844),(350,740)):
        context=browser.new_context(
            viewport={"width":width,"height":height},
            reduced_motion="reduce" if width<=390 else "no-preference")
        page=context.new_page()
        errors=[]
        page.on("pageerror",lambda error:errors.append(str(error)))
        page.evaluate("""()=>{
          const values=new Map();
          Object.defineProperty(window,"localStorage",{configurable:true,value:{
            getItem:k=>values.has(k)?values.get(k):null,
            setItem:(k,v)=>values.set(k,String(v)),removeItem:k=>values.delete(k),
            clear:()=>values.clear()}});
        }""")
        page.set_content(HTML,wait_until="load")
        for style in STYLES.values():page.add_style_tag(content=style)
        for name in ("u2","u3","u4","u5","u6","u7","u14","u18","u19"):
            if name in JS:page.add_script_tag(content=JS[name])
        page.wait_for_timeout(120)

        def state():
            return page.evaluate("""()=>({
              view:document.querySelector(".shell").dataset.rtCurrentView,
              focus:document.querySelector("#stage").classList.contains("focus-mode"),
              locked:document.body.classList.contains("in-focus"),
              stageHidden:document.getElementById("rtStagePanel").hidden,
              headerInert:document.querySelector(".rt-header").inert,
              sidecarInert:document.querySelector(".rt-stage-sidecar").inert,
              role:document.querySelector("#stage").getAttribute("role"),
              ariaModal:document.querySelector("#stage").getAttribute("aria-modal"),
              focusId:document.activeElement?.id||"",
              focusTag:document.activeElement?.tagName||"",
              muteParent:document.getElementById("rtMobileMute").parentElement.id
            })""")
        def open_focus():
            page.locator("#stageFocus").click()
            page.wait_for_function("""()=>{
              const stage=document.getElementById("stage");
              return stage.getAttribute("aria-modal")==="true" &&
                     document.querySelector(".rt-header").inert;
            }""")
        def closed(destination,heading):
            page.wait_for_timeout(100)
            v=state()
            require(v["view"]==destination,
                    f"{destination} route visible after modal exit: {v}",width)
            require(not v["focus"] and not v["locked"],
                    f"{destination} route releases Stage focus/root lock: {v}",width)
            require(not v["headerInert"] and not v["sidecarInert"],
                    f"{destination} route restores navigation and sidecar: {v}",width)
            require(v["role"] is None and v["ariaModal"] is None,
                    f"{destination} route clears modal dialog semantics: {v}",width)
            require(v["focusId"]==heading,
                    f"{destination} route gives destination heading keyboard focus: {v}",width)
            require(page.evaluate(
                    "getComputedStyle(document.documentElement).overflow!=='hidden'"),
                    "root document scroll restored on modal exit",width)
            if width<=680:
                require(page.locator("#rtMobileMute").evaluate(
                    "(e)=>e.parentElement.classList.contains('rt-navigation')"),
                    "emergency Mute returns to its original navigation home",width)

        require(state()["view"]=="stage","Stage initially visible",width)
        require(page.locator("#rtMobileMute").count()==1,
                "unique original emergency Mute",width)
        page.screenshot(path=str(OUT/f"u19-stage-{width}.png"),full_page=False)
        open_focus()
        require(state()["role"]=="dialog" and state()["focus"],
                "Stage modal semantic activated",width)
        require(state()["sidecarInert"],"background mixer is inert",width)
        require(page.evaluate("""()=>{
          const r=document.getElementById('stage').getBoundingClientRect();
          return Math.abs(r.top)<2 && Math.abs(r.left)<2 &&
            Math.abs(r.width-innerWidth)<2 && Math.abs(r.height-innerHeight)<2;
        }"""),"fullscreen viewport coverage",width)
        ax=context.new_cdp_session(page).send("Accessibility.getFullAXTree")["nodes"]
        exposed=[n for n in ax if not n.get("ignored")]
        require(any(n.get("role",{}).get("value")=="dialog" for n in exposed),
                "Chromium AX tree exposes modal dialog",width)
        require(not any(n.get("role",{}).get("value")=="button" and
                        n.get("name",{}).get("value")=="Songs" for n in exposed),
                "Chromium AX tree excludes background workspace nav",width)
        page.screenshot(path=str(OUT/f"u19-focus-{width}.png"),full_page=False)
        page.locator("#stageFocus").focus()
        for direction in ("Tab","Shift+Tab"):
            for i in range(20):
                page.keyboard.press(direction)
                require(page.evaluate("!!document.activeElement?.closest('#stage')"),
                        f"{direction} #{i+1} never escapes fullscreen",width)
        page.keyboard.press("Escape")
        page.wait_for_function("!document.body.classList.contains('in-focus')")
        require(not state()["headerInert"] and not state()["sidecarInert"],
                "Escape restores original accessibility hierarchy",width)
        require(state()["role"] is None,"Escape restores original Stage role",width)

        # Keyboard-triggered F from a real external heading, then Escape must
        # put focus back on that heading rather than body or hidden Stage.
        heading=page.locator("#rtStagePanel .rt-view-title h1")
        heading.evaluate("e=>{e.tabIndex=-1;e.focus({preventScroll:true})}")
        page.keyboard.press("f")
        page.wait_for_function("document.body.classList.contains('in-focus')")
        page.keyboard.press("Escape")
        page.wait_for_function("!document.body.classList.contains('in-focus')")
        require(page.evaluate("""()=>!!document.activeElement?.closest(
                      '#rtStagePanel .rt-view-title')"""),
                "keyboard F/Escape restores original external heading",width)

        # Inherited U18 witnessed real route must still pass.
        open_focus()
        page.locator("#jumpToStudio").click()
        closed("songs","rtSongsHeading")
        require(page.locator("#rtSongsContent").is_visible(),
                "Songs workspace remains readable after Lyrics studio route",width)
        page.screenshot(path=str(OUT/f"u19-songs-{width}.png"),full_page=False)

        # Route to Stage via the ORIGINAL navigation (no mocked app handlers).
        page.locator('[data-rt-view="stage"]').click()
        require(state()["view"]=="stage","normal Stage navigation restored",width)

        # POSITIVE REGRESSION DIAGNOSTIC: a SPA workspace transition may be
        # programmatically dispatched by an integration/keyboard command.
        # The native inert attribute prevents pointer navigation meanwhile,
        # but the original switchView still runs on a trusted app listener.
        # It must never leave a hidden Stage as active modal or lock the page.
        open_focus()
        page.locator('[data-rt-view="studio"]').evaluate("(button)=>button.click()")
        closed("studio","rtStudioHeading")
        require(page.locator("#rtStudioContent").is_visible(),
                "Studio original workspace visible",width)
        page.screenshot(path=str(OUT/f"u19-studio-{width}.png"),full_page=False)

        # Repeat the same hidden-background transition to Songs after another
        # fullscreen entry; assert that modal/inert cleanup remains idempotent.
        page.locator('[data-rt-view="stage"]').click()
        require(state()["view"]=="stage","Stage re-entered",width)
        open_focus()
        page.locator('[data-rt-view="songs"]').evaluate("(button)=>button.click()")
        closed("songs","rtSongsHeading")
        require(page.locator("#rtSongSearch").is_visible(),
                "Songs search interactive after repeated workspace transitions",width)
        require(page.evaluate("document.documentElement.scrollWidth<=innerWidth+2"),
                "no horizontal overflow in Songs after route",width)
        require(not errors,"no page errors: "+repr(errors),width)
        print(f"U19 PASS {width}x{height}; cumulative={checks}",flush=True)
        context.close()
    browser.close()

pictures=sorted(OUT.glob("u19-*.png"))
assert len(pictures)==16,f"expected 16 original U19 screenshots, got {len(pictures)}"
(OUT/"sha256.txt").write_text("".join(
    f"{hashlib.sha256(p.read_bytes()).hexdigest()}  {p.name}\n" for p in pictures
),encoding="utf-8")
print(f"U19 PASS {checks} synthetic Chromium assertions, 16 PNGs with SHA256",flush=True)
