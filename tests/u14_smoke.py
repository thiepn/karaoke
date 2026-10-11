"""ROOMTONE U14: synthetic Chromium keyboard/AX/modal and Songs regression.

No real screen reader, microphone, deployed origin or production user data.
Original UI/JS/CSS stays intact except the isolated U14 accessibility layer.
"""
from pathlib import Path
from playwright.sync_api import sync_playwright
import hashlib
import io
import math
import struct
import wave

ROOT = Path(__file__).resolve().parents[1]
HTML = (ROOT / "index.html").read_text(encoding="utf-8")
assert HTML.count('src="u14.js"') == 1
assert 'u14.css' not in HTML
for n in ("u2","u3","u4","u5","u6","u7","u11","u12","u13"):
    assert HTML.count(f'href="{n}.css"') == 1
for n in ("u2","u3","u4","u5","u6","u7"):
    assert HTML.count(f'src="{n}.js"') == 1
CSS = {n:(ROOT/f"{n}.css").read_text(encoding="utf-8")
       for n in ("u2","u3","u4","u5","u6","u7","u11","u12","u13")}
JS = {n:(ROOT/f"{n}.js").read_text(encoding="utf-8")
      for n in ("u2","u3","u4","u5","u6","u7","u14")}
OUT = ROOT/"u14-evidence"
OUT.mkdir(exist_ok=True)
assert not list(OUT.glob("u14-*.png")), "stale U14 screenshot found"
buf=io.BytesIO()
with wave.open(buf,"wb") as w:
    w.setnchannels(1);w.setsampwidth(2);w.setframerate(22050)
    w.writeframes(b"".join(struct.pack("<h",int(420*math.sin(
        2*math.pi*250*i/22050))) for i in range(22050)))
WAV=buf.getvalue()
tests=0

def verify(ok, msg, width):
    global tests
    assert ok, f"U14 {width}px: {msg}"
    tests+=1

with sync_playwright() as playwright:
    browser=playwright.chromium.launch(
        executable_path="/usr/bin/chromium" if Path("/usr/bin/chromium").exists() else None,
        headless=True,args=["--no-sandbox","--autoplay-policy=no-user-gesture-required"])
    for width,height in ((1440,900),(768,1024),(390,844),(350,740)):
        ctx=browser.new_context(viewport={"width":width,"height":height},
                                reduced_motion="reduce" if width<=390 else "no-preference")
        page=ctx.new_page()
        page_errors=[]
        page.on("pageerror",lambda error:page_errors.append(str(error)))
        page.evaluate("""()=>{
          const data=new Map();
          Object.defineProperty(window,'localStorage',{configurable:true,value:{
            getItem:k=>data.has(k)?data.get(k):null,
            setItem:(k,v)=>data.set(k,String(v)),
            removeItem:k=>data.delete(k),clear:()=>data.clear()
          }});
        }""")
        page.set_content(HTML,wait_until="load")
        for name,style in CSS.items():page.add_style_tag(content=style)
        for name in ("u2","u3","u4","u5","u6","u7","u14"):
            page.add_script_tag(content=JS[name])
        page.wait_for_timeout(140)
        cdp=ctx.new_cdp_session(page)

        verify(page.locator("#stage").get_attribute("role") is None,
               "default Stage is not a modal dialog",width)
        verify(not page.locator(".rt-header").evaluate("(e)=>e.inert"),
               "navigation initially accessible",width)
        verify(page.locator("#rtMobileMute").count()==1,
               "same emergency mute retained",width)
        verify(page.locator("#stagePanic").count()==1,
               "inline emergency mute retained",width)
        page.locator("#songFile").set_input_files({
            "name":"U14 Accessibility Fixture.wav","mimeType":"audio/wav","buffer":WAV})
        page.wait_for_timeout(120)
        verify(page.locator("#stagePlay").is_enabled(),
               "synthetic track is ready",width)
        page.screenshot(path=str(OUT/f"u14-stage-{width}.png"),full_page=True)
        verify(page.evaluate("document.documentElement.scrollWidth<=innerWidth+2"),
               "Stage no horizontal overflow",width)

        page.locator("#stageFocus").click()
        page.wait_for_function("""()=>{
          const s=document.querySelector('#stage');
          return s.getAttribute('role')==='dialog' &&
                 s.getAttribute('aria-modal')==='true';
        }""")
        verify(page.locator("#stage").get_attribute("aria-labelledby")=="stageTitle",
               "modal announced by actual active song title",width)
        verify(page.locator(".rt-header").evaluate("(e)=>e.inert"),
               "unrelated navigation excluded from focus/accessibility",width)
        verify(page.locator(".rt-intro").count()==0 or
               page.locator(".rt-header").evaluate("(e)=>e.inert"),
               "original header remains inert during focus",width)
        verify(page.locator(".rt-stage-sidecar").evaluate("(e)=>e.inert"),
               "background queue/mixer inaccessible",width)
        verify(page.evaluate("""()=>{
          const stage=document.querySelector('#stage'), r=stage.getBoundingClientRect();
          const c=getComputedStyle(stage);
          return c.position==='fixed' && c.overflowY==='auto' &&
            Math.abs(r.top)<1 && Math.abs(r.left)<1 &&
            Math.abs(r.width-innerWidth)<2 && Math.abs(r.height-innerHeight)<2;
        }"""),"original fullscreen Stage fills viewport",width)
        verify(page.evaluate("""()=>{
          const el=document.elementFromPoint(innerWidth/2,innerHeight-2);
          return !!el?.closest('#stage');
        }"""),"no background hit target at viewport bottom",width)

        ax=cdp.send("Accessibility.getFullAXTree")["nodes"]
        exposed=[n for n in ax if not n.get("ignored")]
        verify(any(n.get("role",{}).get("value")=="dialog" for n in exposed),
               "Chromium AX tree exposes a dialog",width)
        verify(not any(n.get("role",{}).get("value")=="button" and
                       n.get("name",{}).get("value")=="Songs" for n in exposed),
               "Chromium AX tree excludes inert background Songs button",width)
        if width<=680:
            verify(page.locator("#rtMobileMute").is_visible(),
                   "existing Mute visible in mobile focus",width)
            verify(page.locator("#rtMobileMute").evaluate(
                   "(e)=>e.parentElement.id==='stage'"),
                   "original emergency Mute moved inside modal Stage",width)
            verify(page.locator("#rtMobileMute").evaluate("""e=>{
                const r=e.getBoundingClientRect();return r.top>=0 &&
                  r.left>=0 && r.right<=innerWidth+1 && r.bottom<=innerHeight+1;
            }"""),"Mute not clipped by viewport",width)
        page.screenshot(path=str(OUT/f"u14-focus-viewport-{width}.png"),full_page=False)

        # Explicitly reproduce focus cycling. Never let Tab leave the dialog.
        edge=page.locator("#stage").evaluate("""e=>{
          const xs=[...e.querySelectorAll(
            'button:not([disabled]),a[href],input:not([disabled]),' +
            'select:not([disabled]),textarea:not([disabled]),' +
            '[tabindex]:not([tabindex="-1"])'
          )].filter(x=>!x.inert && x.getClientRects().length &&
                    getComputedStyle(x).visibility!=='hidden');
          xs[xs.length-1].focus({preventScroll:true});
          return {first:xs[0].id,last:xs[xs.length-1].id};
        }""")
        page.keyboard.press("Tab")
        verify(page.evaluate("(id)=>document.activeElement?.id===id",edge["first"]),
               "Tab at last Stage control wraps to first",width)
        page.keyboard.press("Shift+Tab")
        verify(page.evaluate("(id)=>document.activeElement?.id===id",edge["last"]),
               "Shift+Tab at first Stage control wraps to last",width)
        verify(page.locator(".rt-header").evaluate("(e)=>e.inert"),
               "Tab never reenabled background content",width)
        verify(page.locator("#stage").evaluate("""e=>{
          e.scrollTop=e.scrollHeight;
          const b=e.querySelector('.stage-bottom').getBoundingClientRect();
          return b.bottom<=innerHeight+6&&b.top>=-5;
        }"""),"practice and transport scroll to reachability",width)
        page.screenshot(path=str(OUT/f"u14-focus-scrolled-{width}.png"),full_page=False)

        # Existing Escape handler closes focus; U14 must not intercept mute safety.
        page.keyboard.press("Escape")
        page.wait_for_function("!document.body.classList.contains('in-focus')")
        verify(page.locator("#stage").get_attribute("aria-modal") is None,
               "aria-modal removed on close",width)
        verify(page.locator("#stage").get_attribute("role") is None,
               "original Stage semantics restored on close",width)
        verify(not page.locator(".rt-header").evaluate("(e)=>e.inert"),
               "outside navigation reenabled",width)
        verify(not page.locator(".rt-stage-sidecar").evaluate("(e)=>e.inert"),
               "original queue/mixer reenabled",width)
        verify(page.evaluate("getComputedStyle(document.documentElement).overflow!=='hidden'"),
               "root scroll restored after focus",width)
        if width<=680:
            verify(page.locator("#rtMobileMute").evaluate(
                   "(e)=>e.parentElement.classList.contains('rt-navigation')"),
                   "safety Mute returns to original navigation",width)

        page.locator('[data-rt-view="songs"]').click()
        verify(page.locator("#rtSongsContent").is_visible(),
               "Songs workspace keyboard-accessible after focus",width)
        verify(page.locator("#rtU2SongList .rt-u2-song-row").count()==1,
               "original playable row retained",width)
        page.locator("#rtSongSearch").focus()
        page.keyboard.type("Accessibility Fixture")
        page.wait_for_timeout(50)
        verify(page.locator("#rtU2SongList .rt-u2-song-row:visible").count()==1,
               "text search still resolves matching original song",width)
        page.keyboard.press("Tab")
        verify(page.evaluate("document.activeElement?.id==='rtU2AddTracks'"),
               "library Tab keyboard traversal unchanged",width)
        page.locator("#rtSongSearch").fill("missing-U14-phrase")
        page.wait_for_timeout(50)
        verify(page.locator("#rtU2SongList .rt-u2-empty").is_visible(),
               "no-match message available",width)
        verify(page.locator("#rtU2SongStatus").get_attribute("aria-live")=="polite",
               "live screen-reader Songs status preserved",width)
        verify(page.evaluate("document.documentElement.scrollWidth<=innerWidth+2"),
               "Songs has no horizontal overflow",width)
        page.screenshot(path=str(OUT/f"u14-songs-{width}.png"),full_page=False)
        verify(not page_errors,"no page JavaScript error: "+repr(page_errors),width)
        print(f"U14 PASS synthetic {width}x{height}: cumulative {tests}",flush=True)
        ctx.close()
    browser.close()

images=sorted(OUT.glob("u14-*.png"))
assert len(images)==16,f"Expected 16 original U14 PNGs; got {len(images)}"
(OUT/"sha256.txt").write_text(
    "".join(f"{hashlib.sha256(x.read_bytes()).hexdigest()}  {x.name}\n"
            for x in images),encoding="utf-8")
print(f"U14 PASS {tests} synthetic assertions, 16 new screenshots",flush=True)
