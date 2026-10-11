"""ROOMTONE U15 — independently reconcile PR #17 stress gate on PR #18.

Synthetic Chromium only, no hardware, deployed app, personal local files or human witness.
Existing playback/microphone/audio/storage and U13/U14 sources remain unmodified.
"""
from pathlib import Path
import hashlib
from playwright.sync_api import sync_playwright

ROOT=Path(__file__).resolve().parents[1]
HTML=(ROOT/"index.html").read_text(encoding="utf-8")
assert HTML.count('src="u14.js"')==1
assert "u15.js" not in HTML and "u15.css" not in HTML
STYLES={n:(ROOT/f"{n}.css").read_text(encoding="utf-8")
        for n in ("u2","u3","u4","u5","u6","u7","u11","u12","u13")}
SCRIPTS={n:(ROOT/f"{n}.js").read_text(encoding="utf-8")
         for n in ("u2","u3","u4","u5","u6","u7","u14")}
OUT=ROOT/"u15-evidence"
OUT.mkdir(exist_ok=True)
assert not list(OUT.glob("u15-*.png")), "U15 cannot reuse stale screenshots"
checks=0

def require(value,label,width):
    global checks
    assert value, f"U15 {width}px: {label}"
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
          const store=new Map();
          Object.defineProperty(window,"localStorage",{configurable:true,value:{
            getItem:k=>store.has(k)?store.get(k):null,
            setItem:(k,v)=>store.set(k,String(v)),
            removeItem:k=>store.delete(k),clear:()=>store.clear()
          }});
        }""")
        page.set_content(HTML,wait_until="load")
        for s in STYLES.values():page.add_style_tag(content=s)
        for key in ("u2","u3","u4","u5","u6","u7","u14"):
            page.add_script_tag(content=SCRIPTS[key])
        page.wait_for_timeout(120)
        require(page.locator("#stage").get_attribute("aria-modal") is None,
                "non-focus Stage has original semantics",width)
        require(not page.locator(".rt-header").evaluate("(e)=>e.inert"),
                "header initially accessible",width)
        require(page.locator("#rtMobileMute").count()==1,
                "original emergency Mute remains unique",width)

        page.locator("#stageFocus").click()
        page.wait_for_function("""()=>{
          const e=document.querySelector("#stage");
          return e.getAttribute("role")==="dialog" &&
                 e.getAttribute("aria-modal")==="true" &&
                 document.querySelector(".rt-header").inert;
        }""")
        require(page.locator("#stage").get_attribute("aria-labelledby")=="stageTitle",
                "dialog has dynamic Stage heading",width)
        require(page.locator(".rt-stage-sidecar").evaluate("(e)=>e.inert"),
                "queue and mixer excluded from modal focus",width)
        require(page.evaluate("""()=>{
          const s=document.querySelector("#stage"),r=s.getBoundingClientRect();
          return Math.abs(r.top)<1 && Math.abs(r.left)<1 &&
                 Math.abs(r.width-innerWidth)<2 && Math.abs(r.height-innerHeight)<2;
        }"""),"fullscreen occupies viewport",width)
        if width<=680:
            require(page.locator("#rtMobileMute").evaluate(
                    "(e)=>e.parentElement.id==='stage'"),
                    "same emergency Mute moved inside Stage",width)
            require(page.locator("#rtMobileMute").is_visible(),
                    "mobile emergency Mute visible during focus",width)

        ax=context.new_cdp_session(page).send("Accessibility.getFullAXTree")["nodes"]
        exposed=[n for n in ax if not n.get("ignored")]
        dialog=[n for n in exposed if n.get("role",{}).get("value")=="dialog"]
        require(len(dialog)==1,"one modal dialog exposed in Chromium AX tree",width)
        require(bool(dialog[0].get("name",{}).get("value","").strip()),
                "modal accessible name is nonempty",width)
        require(not any(n.get("role",{}).get("value")=="button" and
                        n.get("name",{}).get("value")=="Songs" for n in exposed),
                "underlying Songs nav excluded from AX tree",width)

        # PR #17's proven diagnostic used 45 forward and 45 backward Tabs.
        # Qualify those exact stress patterns on preferred PR #18 without
        # copying or weakening either original U14 implementation.
        for direction in ("Tab","Shift+Tab"):
            page.locator("#stageFocus").focus()
            visited=set()
            for index in range(45):
                page.keyboard.press(direction)
                record=page.evaluate("""()=>({
                  inStage:!!document.activeElement?.closest("#stage"),
                  id:document.activeElement?.id || "",
                  tag:document.activeElement?.tagName || "",
                  focusedTitle:document.activeElement?.getAttribute("aria-label") || ""
                })""")
                require(record["inStage"],
                        f"{direction} #{index+1} escaped to {record}",width)
                visited.add((record["id"],record["tag"],record["focusedTitle"]))
            require(len(visited)>=3,
                    f"{direction} genuinely traversed multiple Stage controls",width)

        # Programmatic attempt to move into inert header must not strand focus.
        page.locator('.rt-header [data-rt-view="songs"]').evaluate("(e)=>e.focus()")
        require(page.evaluate("!!document.activeElement?.closest('#stage')"),
                "background programmatic focus cannot escape modal",width)
        page.locator("#stage").evaluate("(e)=>e.scrollTop=0")
        page.screenshot(path=str(OUT/f"u15-focus-{width}.png"),full_page=False)
        require(page.locator("#stage").evaluate("""e=>{
          e.scrollTop=e.scrollHeight;
          const b=e.querySelector('.stage-bottom').getBoundingClientRect();
          return b.top>=-5 && b.bottom<=innerHeight+6;
        }"""),"transport remains reachable after stress navigation",width)
        page.screenshot(path=str(OUT/f"u15-scrolled-{width}.png"),full_page=False)

        page.keyboard.press("Escape")
        page.wait_for_function("""()=>{
          const e=document.querySelector("#stage");
          return !document.body.classList.contains("in-focus") &&
                 e.getAttribute("role")===null &&
                 e.getAttribute("aria-modal")===null &&
                 !document.querySelector(".rt-header").inert;
        }""")
        require(not page.locator(".rt-stage-sidecar").evaluate("(e)=>e.inert"),
                "sidecar fully restored on exit",width)
        require(page.evaluate("getComputedStyle(document.documentElement).overflow!=='hidden'"),
                "document scroll unlocked on exit",width)
        if width<=680:
            require(page.locator("#rtMobileMute").evaluate(
                    "(e)=>e.parentElement.classList.contains('rt-navigation')"),
                    "Mute returns to navigation on exit",width)
        page.locator('.rt-header [data-rt-view="songs"]').focus()
        require(page.evaluate("document.activeElement?.textContent?.trim()==='Songs'"),
                "keyboard can focus original Songs nav again",width)
        page.locator('.rt-header [data-rt-view="songs"]').click()
        require(page.locator("#rtSongSearch").is_visible(),
                "original searchable local Songs view restored",width)
        require(not errors,"no uncaught page errors "+repr(errors),width)
        print(f"U15 PASS {width}x{height}: {checks} cumulative assertions",flush=True)
        context.close()
    browser.close()

images=sorted(OUT.glob("u15-*.png"))
assert len(images)==8,f"expected 8 original U15 PNGs, found {len(images)}"
(OUT/"sha256.txt").write_text("".join(
    f"{hashlib.sha256(path.read_bytes()).hexdigest()}  {path.name}\n" for path in images
),encoding="utf-8")
print(f"U15 PASS {checks} synthetic assertions, 8 SHA256-manifested original PNGs",flush=True)
