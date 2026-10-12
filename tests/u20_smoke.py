"""U20 synthetic Chromium browser lifecycle & history probes.

Every action runs against original qualified U19 application source. No device,
microphone, storage fixture beyond a fresh in-memory localStorage replacement,
real owner observation, or production/browser release gate is exercised.
Same-document browser History API probes are NOT an app router specification.
"""
from pathlib import Path
from playwright.sync_api import sync_playwright
import hashlib

ROOT=Path(__file__).resolve().parent.parent
HTML=(ROOT/"index.html").read_text(encoding="utf-8")
assert HTML.count('src="u19.js"')==1
CSS={s:(ROOT/f"{s}.css").read_text(encoding="utf-8")
     for s in ("u2","u3","u4","u5","u6","u7","u11","u12","u13")}
JS={s:(ROOT/f"{s}.js").read_text(encoding="utf-8")
    for s in ("u2","u3","u4","u5","u6","u7","u14","u18","u19")}
OUT=ROOT/"u20-evidence"
OUT.mkdir(exist_ok=True)
assert not list(OUT.glob("u20-*.png")) and not (OUT/"sha256.txt").exists()
checks=0
visibility_hidden_probes=0

def ok(truth,label,width):
    global checks
    assert truth, f"U20 {width}px: {label}"
    checks+=1

with sync_playwright() as pw:
    browser=pw.chromium.launch(
        executable_path="/usr/bin/chromium" if Path("/usr/bin/chromium").exists() else None,
        headless=True,args=["--no-sandbox"])
    for width,height in ((1440,900),(768,1024),(390,844),(350,740)):
        ctx=browser.new_context(viewport={"width":width,"height":height},
            reduced_motion="reduce" if width<=390 else "no-preference")
        page=ctx.new_page()
        errors=[]
        page.on("pageerror",lambda e:errors.append(str(e)))
        page.evaluate("""()=>{
          const m=new Map();
          Object.defineProperty(window,'localStorage',{configurable:true,value:{
            getItem:k=>m.has(k)?m.get(k):null,setItem:(k,v)=>m.set(k,String(v)),
            removeItem:k=>m.delete(k),clear:()=>m.clear()
          }});
        }""")
        page.set_content(HTML,wait_until="load")
        for style in CSS.values():page.add_style_tag(content=style)
        for script in ("u2","u3","u4","u5","u6","u7","u14","u18","u19"):
            page.add_script_tag(content=JS[script])
        page.wait_for_timeout(80)

        def state():
            return page.evaluate("""()=>({
                view:document.querySelector(".shell").dataset.rtCurrentView,
                dialog:document.querySelector("#stage").getAttribute("role"),
                label:document.querySelector("#stage").getAttribute("aria-labelledby"),
                modal:document.querySelector("#stage").getAttribute("aria-modal"),
                focus:document.querySelector("#stage").classList.contains("focus-mode"),
                bodyLock:document.body.classList.contains("in-focus"),
                headerInert:document.querySelector(".rt-header").inert,
                sidecarInert:document.querySelector(".rt-stage-sidecar").inert,
                activeId:document.activeElement?.id||"",
                muteParent:document.querySelector("#rtMobileMute").parentElement.id,
                stageHidden:document.getElementById("rtStagePanel").hidden,
                visibility:document.visibilityState,
                hash:location.hash,
                scrollWidth:document.documentElement.scrollWidth
            })""")

        def opened():
            v=state()
            ok(v["view"]=="stage" and v["focus"] and v["bodyLock"],
               f"visible fullscreen Stage with scroll lock: {v}",width)
            ok(v["dialog"]=="dialog" and v["modal"]=="true" and v["label"]=="stageTitle",
               f"named modal semantics: {v}",width)
            ok(v["headerInert"] and v["sidecarInert"],
               f"background inaccessible while fullscreen: {v}",width)
            ok(page.locator("#rtMobileMute").count()==1,
               "exactly one original emergency Mute exists",width)
            ok(v["muteParent"]=="stage","emergency Mute reparented into fullscreen Stage",width)
            ax=ctx.new_cdp_session(page).send("Accessibility.getFullAXTree")["nodes"]
            visible=[n for n in ax if not n.get("ignored")]
            ok(any(n.get("role",{}).get("value")=="dialog" for n in visible),
               "Chromium AX tree contains Stage dialog",width)
            ok(not any(n.get("role",{}).get("value")=="button" and
                       n.get("name",{}).get("value")=="Songs" for n in visible),
               "Chromium AX tree excludes inert background navigation",width)

        def closed():
            v=state()
            ok(not v["focus"] and not v["bodyLock"],
               f"fullscreen and body lock released: {v}",width)
            ok(v["dialog"] is None and v["modal"] is None,
               f"original ARIA semantics restored: {v}",width)
            ok(not v["headerInert"] and not v["sidecarInert"],
               f"background inert flags restored: {v}",width)
            ok(page.locator("#rtMobileMute").count()==1,
               "emergency mute preserved",width)
            ok(page.locator("#rtMobileMute").evaluate(
               "(e)=>e.parentElement.classList.contains('rt-navigation')"),
               "emergency Mute returned to the original nav",width)

        ok(state()["view"]=="stage","initial Stage workspace",width)
        page.screenshot(path=str(OUT/f"u20-stage-{width}.png"),full_page=False)
        for cycle in range(3):
            page.locator("#stageFocus").click()
            page.wait_for_function("""()=>document.querySelector('#stage')
                                      .getAttribute('aria-modal')==='true'""")
            opened()
            if cycle==0:
                page.screenshot(path=str(OUT/f"u20-fullscreen-{width}.png"),full_page=False)
                # Same-document history traversal tests browser-native
                # popstate and hash state, not unsupported app URL routing.
                page.evaluate("""()=>{
                  history.pushState({u20:'synthetic'},'', '#u20-history-probe');
                }""")
                ok(state()["hash"]=="#u20-history-probe",
                   "native same-document pushState updates URL hash",width)
                page.evaluate("history.back()")
                page.wait_for_function("location.hash===''")
                opened()
                ok(state()["hash"]=="","browser Back returns prior hash",width)
                page.evaluate("history.forward()")
                page.wait_for_function("location.hash==='#u20-history-probe'")
                opened()
                page.evaluate("history.replaceState({},'','#u20-reset')")
                ok(state()["hash"]=="#u20-reset",
                   "same-document history state replaced using valid hash",width)

                # Browser page visibility: activate another real Chromium tab;
                # distinguish actual hidden state from synthetic event dispatch.
                other=ctx.new_page()
                other.set_content("<title>synthetic U20 tab</title>",wait_until="load")
                other.bring_to_front()
                hidden=page.evaluate("document.visibilityState==='hidden'")
                if hidden:
                    visibility_hidden_probes+=1
                    opened()
                else:
                    print(f"U20 NOT_TESTED actual background visibility at {width}; "
                          "headless Chromium did not report hidden",flush=True)
                page.bring_to_front()
                other.close()
                page.wait_for_timeout(40)
                opened()
                # Synthetic visibilitychange alone is never reported as an
                # actual OS tab/background acceptance observation.
                page.evaluate("document.dispatchEvent(new Event('visibilitychange'))")
                opened()
                page.screenshot(path=str(OUT/f"u20-return-{width}.png"),full_page=False)

            page.locator("#stageFocus").focus()
            for direction in ("Tab","Shift+Tab"):
                for i in range(6):
                    page.keyboard.press(direction)
                    ok(page.evaluate("!!document.activeElement?.closest('#stage')"),
                       f"cycle {cycle} {direction} {i} remains inside Stage",width)
            if cycle==1:
                page.locator("#stageFocus").focus()
                page.keyboard.press("Escape")
            else:
                page.locator("#stageFocus").click()
            page.wait_for_function("""()=>!document.body.classList.contains('in-focus')
                                      && !document.querySelector('.rt-header').inert""")
            closed()

        # Native real workspace routing, no mocked app listeners.
        page.locator('[data-rt-view="songs"]').click()
        ok(state()["view"]=="songs" and not page.locator("#rtSongsPanel").is_hidden(),
           "Songs visible after completed modal cycles",width)
        ok(page.evaluate("document.activeElement?.id==='rtSongsHeading'"),
           "Songs receives keyboard heading focus",width)
        page.locator('[data-rt-view="studio"]').click()
        ok(state()["view"]=="studio" and not page.locator("#rtStudioPanel").is_hidden(),
           "Studio visible after Songs",width)
        ok(page.evaluate("document.activeElement?.id==='rtStudioHeading'"),
           "Studio receives keyboard heading focus",width)
        page.locator('[data-rt-view="stage"]').click()
        ok(state()["view"]=="stage","Stage re-entered after Studio",width)

        # Actual original UI keyboard shortcut F on an external heading.
        heading=page.locator("#rtStagePanel .rt-view-title h1")
        heading.evaluate("e=>{e.tabIndex=-1;e.focus({preventScroll:true})}")
        page.keyboard.press("f")
        page.wait_for_function("document.body.classList.contains('in-focus')")
        opened()
        page.keyboard.press("Escape")
        page.wait_for_function("!document.body.classList.contains('in-focus')")
        closed()
        ok(page.evaluate("""()=>!!document.activeElement?.closest(
                  '#rtStagePanel .rt-view-title')"""),
           "external F-key origin restored on Escape",width)

        # U19 original cross-view repair must remain active for programmatic
        # Studio route and all original sidecar/AX cleanup.
        page.locator("#stageFocus").click()
        opened()
        page.locator('[data-rt-view="studio"]').evaluate("e=>e.click()")
        page.wait_for_function("""()=>!document.body.classList.contains('in-focus')
                                    && !document.querySelector('.rt-header').inert""")
        closed()
        ok(state()["view"]=="studio" and state()["activeId"]=="rtStudioHeading",
           "programmatic fullscreen -> Studio route restores destination focus",width)
        page.screenshot(path=str(OUT/f"u20-studio-{width}.png"),full_page=False)
        ok(state()["scrollWidth"] <= width+2,
           "no horizontal overflow after repeated modal transitions",width)
        ok(not errors,"no JavaScript page errors: "+repr(errors),width)
        print(f"U20 PASS {width}x{height} synthetic Chromium cumulative={checks}",flush=True)
        ctx.close()
    browser.close()

pngs=sorted(OUT.glob("u20-*.png"))
assert len(pngs)==16,f"expected 16 original U20 screenshots, got {len(pngs)}"
(OUT/"sha256.txt").write_text("".join(
    f"{hashlib.sha256(p.read_bytes()).hexdigest()}  {p.name}\n" for p in pngs),
    encoding="utf-8")
print(f"U20 PASS {checks} synthetic Chromium checks and {len(pngs)} PNGs; "
      f"actual hidden-tab probes {visibility_hidden_probes}/4",flush=True)
