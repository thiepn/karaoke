"""U15 synthetic Chromium: focus lifecycle and structural accessibility.

Not a human assistive-technology, keyboard device, microphone or release test.
"""
from pathlib import Path
from playwright.sync_api import sync_playwright
import hashlib

ROOT=Path(__file__).resolve().parent.parent
HTML=(ROOT/"index.html").read_text(encoding="utf-8")
CSS={n:(ROOT/f"{n}.css").read_text(encoding="utf-8") for n in
     ("u2","u3","u4","u5","u6","u7","u11","u12","u13")}
JS={n:(ROOT/f"{n}.js").read_text(encoding="utf-8") for n in
    ("u2","u3","u4","u5","u6","u7","u14")}
if (ROOT/"u15.js").exists(): JS["u15"]=(ROOT/"u15.js").read_text()
OUT=ROOT/"u15-evidence"
OUT.mkdir(exist_ok=True)
assert not list(OUT.glob("u15-*.png")), "stale U15 PNGs are disallowed"
checks=0
def verify(truth,message,width):
    global checks
    assert truth,f"U15 {width}px: {message}"
    checks+=1

with sync_playwright() as pw:
    browser=pw.chromium.launch(executable_path="/usr/bin/chromium" if Path("/usr/bin/chromium").exists() else None,
        headless=True,args=["--no-sandbox"])
    for width,height in ((1440,900),(768,1024),(390,844),(350,740)):
        c=browser.new_context(viewport={"width":width,"height":height},
            reduced_motion="reduce" if width<=390 else "no-preference")
        page=c.new_page()
        errors=[]
        page.on("pageerror",lambda err:errors.append(str(err)))
        page.evaluate("""()=>{
          const cache=new Map();
          Object.defineProperty(window,'localStorage',{configurable:true,value:{
            getItem:k=>cache.has(k)?cache.get(k):null,
            setItem:(k,v)=>cache.set(k,String(v)),
            removeItem:k=>cache.delete(k),clear:()=>cache.clear()
          }});
        }""")
        page.set_content(HTML,wait_until="load")
        for n,s in CSS.items():page.add_style_tag(content=s)
        for n,s in JS.items():page.add_script_tag(content=s)
        page.wait_for_timeout(100)
        verify(page.locator("#stage").get_attribute("role") is None,"Stage original semantics preserved outside focus",width)
        verify(page.locator(".rt-header").evaluate("e=>!e.inert"),"header interactive before focus",width)
        verify(page.locator("#rtMobileMute").count()==1,"original emergency Mute unique",width)
        page.screenshot(path=str(OUT/f"u15-stage-viewport-{width}.png"),full_page=False)

        # Enter via the original Focus button. Accessible dialog name is real
        # and background content should be inert both for keyboard and AT.
        page.locator("#stageFocus").click()
        page.wait_for_timeout(90)
        verify(page.get_by_role("dialog",name="Karaoke performance stage").count()==1,
               "focus Stage has named accessible dialog role",width)
        verify(page.locator("#stage").get_attribute("aria-modal")=="true",
               "modal attribute only during focus",width)
        verify(page.locator(".rt-header").evaluate("e=>e.inert"),
               "background header removed from modal interaction",width)
        verify(page.locator(".rt-stage-sidecar").evaluate("e=>e.inert"),
               "background queue and mixer absent from modal accessibility",width)
        verify(page.locator("#rtStagePanel .rt-view-title").evaluate("e=>e.inert"),
               "heading behind modal inert",width)
        verify(page.locator("#stage").evaluate("""e=>{
            const r=e.getBoundingClientRect();
            return Math.abs(r.top)<2&&Math.abs(r.left)<2&&
                   Math.abs(r.width-innerWidth)<2&&Math.abs(r.height-innerHeight)<2;
        }"""),"dialog occupies actual full viewport",width)
        snapshot=page.get_by_role("dialog").aria_snapshot()
        verify("Karaoke performance stage" in snapshot and "Exit focus" in snapshot,
               "synthetic accessibility snapshot has dialog and real exit control",width)
        page.locator("#stageFocus").focus()
        for direction in ("Tab","Shift+Tab"):
            for i in range(12):
                page.keyboard.press(direction)
                verify(page.evaluate("""()=>{
                    const e=document.activeElement;
                    return !!e.closest('#stage')&&!e.disabled&&
                           getComputedStyle(e).visibility!=='hidden'&&
                           e.getClientRects().length>0;
                }"""),f"{direction} #{i} stays in visible enabled Stage",width)
        page.screenshot(path=str(OUT/f"u15-focus-viewport-{width}.png"),full_page=False)
        page.locator("#stageFocus").focus()
        page.keyboard.press("Escape")
        page.wait_for_timeout(60)
        verify(not page.locator("#stage").evaluate("e=>e.classList.contains('focus-mode')"),
               "Escape preserves original fullscreen exit",width)
        verify(page.locator("#stage").get_attribute("role") is None,
               "Stage original role restored",width)
        verify(page.locator(".rt-header").evaluate("e=>!e.inert"),
               "header restored after focus exit",width)
        verify(page.locator(".rt-stage-sidecar").evaluate("e=>!e.inert"),
               "queue/mixer restored after focus exit",width)

        # Keyboard F from original visible Stage heading, then Escape.
        # U14 moved focus into modal but did not restore external trigger.
        heading=page.locator("#rtStagePanel .rt-view-title h1")
        heading.evaluate("e=>{e.tabIndex=-1;e.focus({preventScroll:true})}")
        verify(page.evaluate("document.activeElement?.closest('#rtStagePanel .rt-view-title')!==null"),
               "external Stage heading focus established",width)
        page.keyboard.press("f")
        page.wait_for_timeout(60)
        verify(page.locator("#stage").evaluate("e=>e.classList.contains('focus-mode')"),
               "existing F shortcut enters Stage",width)
        page.keyboard.press("Escape")
        page.wait_for_timeout(70)
        verify(page.evaluate("document.activeElement?.closest('#rtStagePanel .rt-view-title')!==null"),
               "Escape restores pre-modal heading after keyboard F",width)

        # From modal Stage, original Lyrics studio action switches to Songs.
        page.locator("#stageFocus").click()
        page.locator("#jumpToStudio").click()
        page.wait_for_timeout(90)
        verify(page.locator("#rtSongsPanel").is_visible(),
               "original Lyrics Studio route changes view to Songs",width)
        verify(not page.locator("#stage").evaluate("e=>e.classList.contains('focus-mode')"),
               "route clears fullscreen state",width)
        verify(not page.locator("body").evaluate("e=>e.classList.contains('in-focus')"),
               "route releases focus body lock",width)
        verify(page.evaluate("document.activeElement?.id==='rtSongsHeading'"),
               "destination Songs heading receives keyboard focus",width)
        verify(page.locator(".rt-header").evaluate("e=>!e.inert"),
               "navigation becomes interactive in Songs view",width)
        page.screenshot(path=str(OUT/f"u15-songs-viewport-{width}.png"),full_page=False)

        # Enter focus again and change workspace via original nav event.
        page.locator('[data-rt-view="stage"]').click()
        page.locator("#stageFocus").click()
        page.locator('[data-rt-view="studio"]').evaluate("e=>e.click()")
        page.wait_for_timeout(75)
        verify(page.locator("#rtStudioPanel").is_visible(),
               "original navigation can reach Studio",width)
        verify(not page.locator("#stage").evaluate("e=>e.classList.contains('focus-mode')"),
               "switching workspace cancels fullscreen Stage",width)
        verify(page.evaluate("getComputedStyle(document.documentElement).overflow!=='hidden'"),
               "cross-view transition restores root scroll",width)
        verify(page.locator(".rt-header").evaluate("e=>!e.inert"),
               "cross-view transition releases inert background",width)
        verify(not errors,"no Chromium JS errors: "+repr(errors),width)
        print(f"U15 PASS {width}x{height}, cumulative={checks}",flush=True)
        c.close()
    browser.close()
png=sorted(OUT.glob("u15-*.png"))
assert len(png)==12,f"Expected 12 U15 viewport images; got {len(png)}"
(OUT/"sha256.txt").write_text("".join(f"{hashlib.sha256(p.read_bytes()).hexdigest()}  {p.name}\n" for p in png),
 encoding="utf-8")
print(f"U15 PASS {checks} synthetic assertions; 12 viewport PNGs with SHA256",flush=True)
