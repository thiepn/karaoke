"""U18 positive browser gate against qualified U16 parent, synthetic Chromium only.

First diagnostic run intentionally tests existing modal -> Lyrics Studio route
without altering application source. Expected positive behavior: destination
Songs heading owns keyboard focus after original cross-view navigation.
Do not reinterpret failed browser assertions as human owner observations.
"""
from pathlib import Path
from playwright.sync_api import sync_playwright
import hashlib

ROOT=Path(__file__).resolve().parents[1]
HTML=(ROOT/"index.html").read_text(encoding="utf-8")
CSS={n:(ROOT/f"{n}.css").read_text() for n in
     ("u2","u3","u4","u5","u6","u7","u11","u12","u13")}
JS={n:(ROOT/f"{n}.js").read_text() for n in
    ("u2","u3","u4","u5","u6","u7","u14")}
if (ROOT/"u18.js").exists():
    JS["u18"]=(ROOT/"u18.js").read_text()
OUT=ROOT/"u18-evidence"
OUT.mkdir(exist_ok=True)
assert not list(OUT.glob("u18-*.png")), "Stale diagnostic PNGs prohibited"
checks=0

def verify(value,why,width):
    global checks
    assert value, f"U18 {width}px: {why}"
    checks+=1

with sync_playwright() as pw:
    browser=pw.chromium.launch(
        executable_path="/usr/bin/chromium" if Path("/usr/bin/chromium").exists() else None,
        headless=True,args=["--no-sandbox"])
    for width,height in ((1440,900),(768,1024),(390,844),(350,740)):
        ctx=browser.new_context(viewport={"width":width,"height":height},
            reduced_motion="reduce" if width<400 else "no-preference")
        page=ctx.new_page()
        errors=[]
        page.on("pageerror",lambda error:errors.append(str(error)))
        page.evaluate("""()=>{
          const data=new Map();
          Object.defineProperty(window,"localStorage",{configurable:true,value:{
            getItem:k=>data.has(k)?data.get(k):null,
            setItem:(k,v)=>data.set(k,String(v)),
            removeItem:k=>data.delete(k),clear:()=>data.clear()}});
        }""")
        page.set_content(HTML,wait_until="load")
        for src in CSS.values():page.add_style_tag(content=src)
        for name in ("u2","u3","u4","u5","u6","u7","u14","u18"):
            if name in JS:page.add_script_tag(content=JS[name])
        page.wait_for_timeout(130)
        verify(page.locator("#rtStagePanel").is_visible(),"Stage initially visible",width)
        verify(not page.locator(".rt-header").evaluate("(e)=>e.inert"),
               "header available before fullscreen",width)
        verify(page.locator("#rtMobileMute").count()==1,
               "original single emergency Mute retained",width)
        page.locator("#stageFocus").click()
        page.wait_for_function("""()=>{
          const e=document.querySelector('#stage');
          return e.getAttribute('role')==='dialog'&&
                 document.querySelector('.rt-header').inert;
        }""")
        verify(page.locator("#stage").get_attribute("aria-labelledby")=="stageTitle",
               "original PR18 accessible dialog naming",width)
        verify(page.locator(".rt-stage-sidecar").evaluate("(e)=>e.inert"),
               "queue/mixer excluded from modal",width)
        ax=ctx.new_cdp_session(page).send("Accessibility.getFullAXTree")["nodes"]
        visible=[n for n in ax if not n.get("ignored")]
        verify(any(n.get("role",{}).get("value")=="dialog" for n in visible),
               "dialog exposed in actual Chromium AX",width)
        verify(not any(n.get("role",{}).get("value")=="button" and
                       n.get("name",{}).get("value")=="Songs" for n in visible),
               "inert Songs nav excluded from AX",width)
        page.screenshot(path=str(OUT/f"u18-before-lyrics-route-{width}.png"),full_page=False)

        # Original click handler switches the view; expected keyboard-focus
        # destination is the actual existing Songs heading.
        page.locator("#jumpToStudio").click()
        page.wait_for_timeout(130)
        diagnostic=page.evaluate("""()=>{
          const h=document.querySelector('#rtSongsHeading');
          return {
            activeId:document.activeElement?.id||'',
            activeTag:document.activeElement?.tagName||'',
            songsVisible:!document.querySelector('#rtSongsPanel').hidden,
            stageHidden:document.querySelector('#rtStagePanel').hidden,
            stageFocused:document.querySelector('#stage').classList.contains('focus-mode'),
            bodyLocked:document.body.classList.contains('in-focus'),
            headerInert:document.querySelector('.rt-header').inert,
            sidecarInert:document.querySelector('.rt-stage-sidecar').inert,
            headingFocusable:!h.closest('[inert],[hidden]'),
            emergencyMuteParent:document.querySelector('#rtMobileMute').parentElement.id
          };
        }""")
        page.screenshot(path=str(OUT/f"u18-after-lyrics-route-{width}.png"),full_page=False)
        print(f"U18 ORIGINAL ROUTE STATE {width}: {diagnostic}",flush=True)
        verify(diagnostic["songsVisible"] and diagnostic["stageHidden"],
               "existing Lyrics studio action navigates to Songs",width)
        verify(not diagnostic["stageFocused"] and not diagnostic["bodyLocked"],
               "modal and page scroll lock ended",width)
        verify(not diagnostic["headerInert"] and not diagnostic["sidecarInert"],
               "navigation and queue/mixer accessible again",width)
        verify(diagnostic["activeId"]=="rtSongsHeading",
               "destination Songs heading focused after Lyrics studio route: "+repr(diagnostic),width)
        verify(not errors,"no page JS errors "+repr(errors),width)
        print(f"U18 PASS synthetic Chromium {width}x{height}: cumulative={checks}",flush=True)
        ctx.close()
    browser.close()

png=sorted(OUT.glob("u18-*.png"))
assert len(png)==8,f"8 U18 original viewport PNGs expected, got {len(png)}"
(OUT/"sha256.txt").write_text("".join(
    f"{hashlib.sha256(p.read_bytes()).hexdigest()}  {p.name}\n" for p in png),
    encoding="utf-8")
print(f"U18 PASS {checks} synthetic browser assertions; 8 original PNGs",flush=True)
