"""ROOMTONE U13 - synthetic browser evidence, not owner/device acceptance.

Focus screenshots are viewport-only. U12 full-page focus captures showed a
background document below the viewport and cannot establish a visible leak.
No live origin, production storage, microphone permissions, or physical audio.
"""
from pathlib import Path
from playwright.sync_api import sync_playwright
import hashlib, io, math, struct, wave

ROOT = Path(__file__).resolve().parent.parent
HTML = (ROOT / "index.html").read_text(encoding="utf-8")
for name in ("u2", "u3", "u4", "u5", "u6", "u7", "u11", "u12", "u13"):
    assert HTML.count(f'href="{name}.css"') == 1, name
for name in ("u2", "u3", "u4", "u5", "u6", "u7"):
    assert HTML.count(f'src="{name}.js"') == 1, name
assert 'src="u13.js"' not in HTML
ASSETS = {f"{name}.css": (ROOT / f"{name}.css").read_text(encoding="utf-8")
          for name in ("u2","u3","u4","u5","u6","u7","u11","u12","u13")}
SCRIPTS = {f"{name}.js": (ROOT / f"{name}.js").read_text(encoding="utf-8")
           for name in ("u2","u3","u4","u5","u6","u7")}
OUT = ROOT / "u13-evidence"
OUT.mkdir(exist_ok=True)
assert not list(OUT.glob("u13-*.png")), "Unexpected stale U13 evidence"
audio_bytes = io.BytesIO()
with wave.open(audio_bytes, "wb") as wav:
    wav.setnchannels(1); wav.setsampwidth(2); wav.setframerate(22050)
    wav.writeframes(b"".join(struct.pack("<h",int(475*math.sin(2*math.pi*320*i/22050)))
                             for i in range(22050*2)))
AUDIO = audio_bytes.getvalue()
checks = 0

def verify(ok, description, width):
    global checks
    assert ok, f"U13 {width}px: {description}"
    checks += 1

with sync_playwright() as pw:
    browser = pw.chromium.launch(
        executable_path="/usr/bin/chromium" if Path("/usr/bin/chromium").exists() else None,
        headless=True, args=["--no-sandbox", "--autoplay-policy=no-user-gesture-required"])
    for width, height in ((1440,900),(768,1024),(390,844),(350,740)):
        context=browser.new_context(viewport={"width":width,"height":height},
                                    reduced_motion="reduce" if width<=390 else "no-preference")
        page=context.new_page()
        errors=[]
        page.on("pageerror", lambda err: errors.append(str(err)))
        page.evaluate("""()=>{
          const cache=new Map();
          Object.defineProperty(window,'localStorage',{configurable:true,value:{
            getItem:k=>cache.has(k)?cache.get(k):null,
            setItem:(k,v)=>cache.set(k,String(v)),
            removeItem:k=>cache.delete(k),
            clear:()=>cache.clear()
          }});
        }""")
        page.set_content(HTML, wait_until="load")
        for name in ("u2","u3","u4","u5","u6","u7","u11","u12","u13"):
            page.add_style_tag(content=ASSETS[f"{name}.css"])
        for name in ("u2","u3","u4","u5","u6","u7"):
            page.add_script_tag(content=SCRIPTS[f"{name}.js"])
        page.wait_for_timeout(140)

        def screenshot(which, fullpage):
            page.evaluate("document.activeElement?.blur()")
            page.wait_for_timeout(75)
            page.screenshot(path=str(OUT/f"u13-{which}-{width}.png"),
                            full_page=fullpage)

        verify(page.locator("#rtMobileMute").count()==1, "original emergency mute unique", width)
        verify(page.locator("#stagePanic").count()==1, "original inline emergency mute unique", width)
        verify(page.locator("#stagePlay").count()==1 and page.locator("#stageNext").count()==1,
               "original player buttons preserved", width)
        verify(page.evaluate("getComputedStyle(document.documentElement).overflow!=='hidden'"),
               "root scroll not locked outside focus",width)
        page.locator("#songFile").set_input_files({
            "name":"U13 Visual Fixture.wav","mimeType":"audio/wav","buffer":AUDIO})
        page.wait_for_timeout(100)
        verify(page.locator("#stagePlay").is_enabled(), "local synthetic WAV enables original player",width)
        verify(page.locator(".rt-u7-stage-dock button").count()==2, "original queue/mixer controls retained",width)
        if width<=680:
            verify(page.locator("#rtMobileMute").is_visible(),"original mobile emergency mute visible",width)
            verify(page.locator("#rtMobileMute").evaluate(
                "(e)=>e.parentElement.classList.contains('rt-navigation')"),
                "mobile mute remains in navigation",width)
            verify(page.evaluate("""()=>{
                const a=document.querySelector('#rtMobileMute').getBoundingClientRect();
                const b=document.querySelector('#stagePlay').getBoundingClientRect();
                const c=document.querySelector('#stageNext').getBoundingClientRect();
                const hit=(p,q)=>p.left<q.right&&p.right>q.left&&p.top<q.bottom&&p.bottom>q.top;
                return !hit(a,b)&&!hit(a,c);
            }"""),"mute does not cover Play or Next",width)
        screenshot("stage",True)

        page.locator('[data-rt-view="songs"]').click()
        verify(page.locator("#rtSongsContent").is_visible(),"original library view works",width)
        verify(page.locator("#rtU2SongStatus").get_attribute("aria-live")=="polite",
               "original live announcement survives U12 and U13",width)
        verify(page.locator("#rtU2SongCount").inner_text()=="1 song",
               "original total song count retained in DOM",width)
        verify(page.locator("#rtU5FilteredCount").inner_text()=="1 song shown",
               "filter result count still visible",width)
        if width<=680:
            verify(page.locator("#rtU2SongCount").evaluate(
                "(e)=>getComputedStyle(e).clipPath.includes('inset')"),
                "redundant total count visually clipped on mobile",width)
            verify(page.locator("#rtU5FilteredCount").is_visible(),
                "only filtered count remains visually prominent",width)
        else:
            verify(page.locator("#rtU2SongCount").evaluate(
                "(e)=>!getComputedStyle(e).clipPath.includes('inset')"),
                "desktop original total count still visible",width)
        # Exercise the existing U5 filter controls with a locally generated
        # synthetic file without LRC; no interaction handler is replaced.
        filters=page.locator("#rtU5LibraryFilters .rt-u5-filter")
        verify(filters.count()==3, "all original lyric filter buttons retained",width)
        filters.nth(1).click()
        verify(page.locator("#rtU5FilteredCount").inner_text()=="0 songs shown",
               "lyrics-ready filter recomputes live count",width)
        filters.nth(2).click()
        verify(page.locator("#rtU5FilteredCount").inner_text()=="1 song shown",
               "needs-lyrics filter shows synthetic song",width)
        filters.nth(0).click()
        verify(page.locator("#rtU2SongList .rt-u2-song-row").first.is_visible(),
               "re-select all restores clickable original row",width)
        verify(page.evaluate("document.documentElement.scrollWidth<=innerWidth+2"),
               "no library horizontal overflow",width)
        page.evaluate("window.scrollTo({top:0,behavior:'instant'})")
        screenshot("songs",True)

        page.locator('[data-rt-view="stage"]').click()
        page.locator("#stageFocus").click()
        page.wait_for_timeout(120)
        verify(page.locator("#stage").evaluate(
            "(e)=>e.classList.contains('focus-mode')"),"native focus mode active",width)
        verify(page.evaluate("getComputedStyle(document.documentElement).overflow==='hidden'"),
               "root scroll locked only while focus Stage is active",width)
        verify(page.evaluate("""()=>{
            const e=document.querySelector('#stage'), r=e.getBoundingClientRect(), c=getComputedStyle(e);
            return c.position==='fixed' && c.overflowY==='auto' &&
                   Number(c.zIndex)>=3000 && Math.abs(r.left)<1 &&
                   Math.abs(r.top)<1 && Math.abs(r.width-innerWidth)<2 &&
                   Math.abs(r.height-innerHeight)<2;
        }"""),"fullscreen Stage fills exact viewport and scrolls internally",width)
        verify(page.evaluate("""()=>{
            const el=document.elementFromPoint(innerWidth/2,innerHeight-2);
            return el&&!!el.closest('#stage');
        }"""),"viewport bottom is inside Stage not underlying footer",width)
        verify(page.locator("#rtMobileMute").count()==1,
               "original emergency mute retained in fullscreen",width)
        if width<=680:
            verify(page.locator("#rtMobileMute").is_visible(),
                   "mobile focus retains visible original Mute",width)
            verify(page.locator("#rtMobileMute").evaluate(
                "(e)=>e.parentElement.id==='stage'"),
                "mobile original Mute reparented to Stage",width)
            verify(page.locator("#stage").evaluate("""e=>{
                e.scrollTop=e.scrollHeight;
                const bottom=e.querySelector('.stage-bottom').getBoundingClientRect();
                return bottom.top>=-5 && bottom.bottom<=innerHeight+6;
            }"""),"practice and transport reachable in internal focus scroll",width)
            page.locator("#stage").evaluate("(e)=>e.scrollTop=0")
        verify(page.evaluate("""()=>{
            const current=document.querySelector('#lyricCurrent').getBoundingClientRect();
            const cue=document.querySelector('#rtU5LyricCue').getBoundingClientRect();
            return current.bottom+6<=cue.top;
        }"""),"fullscreen empty-lyrics placeholder clears cue",width)
        # CRITICAL U13 change: browser viewport screenshot, not full_page.
        # A fullPage image additionally captures otherwise-hidden document
        # below the viewport and cannot prove a visual fullscreen leak.
        screenshot("focus-viewport",False)

        page.locator("#stageFocus").click()
        verify(not page.locator("#stage").evaluate(
            "(e)=>e.classList.contains('focus-mode')"),"original focus exit works",width)
        verify(page.evaluate("getComputedStyle(document.documentElement).overflow!=='hidden'"),
               "root scroll restored after leaving focus",width)
        if width<=680:
            verify(page.locator("#rtMobileMute").evaluate(
                "(e)=>e.parentElement.classList.contains('rt-navigation')"),
                "same original emergency mute returns to header",width)
        verify(page.evaluate("document.documentElement.scrollWidth<=innerWidth+2"),
               "no horizontal overflow after focus exit",width)
        verify(not errors,"no JavaScript errors: "+repr(errors),width)
        print(f"U13 PASS synthetic {width}x{height}: cumulative {checks}",flush=True)
        context.close()
    browser.close()

images=sorted(OUT.glob("u13-*.png"))
assert len(images)==12, f"expected 12 original U13 PNGs, got {len(images)}"
(OUT/"sha256.txt").write_text(
    "".join(f"{hashlib.sha256(path.read_bytes()).hexdigest()}  {path.name}\n" for path in images),
    encoding="utf-8")
print(f"U13 PASS {checks} synthetic assertions; 12 original PNGs + SHA256 manifest",flush=True)
