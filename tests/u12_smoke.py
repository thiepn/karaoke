"""ROOMTONE U12 — synthetic visual regression of UI-only After Hours polish.

No production origin, microphone permission, physical hardware, owner witness
or audio DSP test. Existing buttons and controls must retain their original
handlers; screenshots are Chromium evidence, never human acceptance.
"""
from pathlib import Path
from playwright.sync_api import sync_playwright
import hashlib
import io
import math
import struct
import wave

ROOT = Path(__file__).resolve().parent.parent
HTML = (ROOT / "index.html").read_text(encoding="utf-8")
assert HTML.count('href="u11.css"') == 1
assert HTML.count('href="u12.css"') == 1
assert HTML.count('src="u11.js"') == 0
for n in ("u2", "u3", "u4", "u5", "u6", "u7"):
    assert HTML.count(f'href="{n}.css"') == 1
    assert HTML.count(f'src="{n}.js"') == 1
ASSETS = {f"{n}.css": (ROOT / f"{n}.css").read_text(encoding="utf-8")
          for n in ("u2", "u3", "u4", "u5", "u6", "u7", "u11", "u12")}
SCRIPTS = {f"{n}.js": (ROOT / f"{n}.js").read_text(encoding="utf-8")
           for n in ("u2", "u3", "u4", "u5", "u6", "u7")}

buf = io.BytesIO()
with wave.open(buf, "wb") as wav:
    wav.setnchannels(1)
    wav.setsampwidth(2)
    wav.setframerate(22050)
    wav.writeframes(b"".join(
        struct.pack("<h", int(520 * math.sin(2 * math.pi * 300 * i / 22050)))
        for i in range(22050 * 2)))
AUDIO = buf.getvalue()

OUT = ROOT / "u12-evidence"
OUT.mkdir(exist_ok=True)
assert not list(OUT.glob("u12-*.png")), "Unexpected stale U12 screenshots"
count = 0

def check(truth, message, width):
    global count
    assert truth, f"U12 {width}px: {message}"
    count += 1

with sync_playwright() as p:
    browser = p.chromium.launch(
        executable_path="/usr/bin/chromium" if Path("/usr/bin/chromium").exists() else None,
        headless=True,
        args=["--no-sandbox", "--autoplay-policy=no-user-gesture-required"],
    )
    for width, height in ((1440, 900), (768, 1024), (390, 844), (350, 740)):
        context = browser.new_context(
            viewport={"width": width, "height": height},
            reduced_motion="reduce" if width <= 390 else "no-preference",
        )
        page = context.new_page()
        errors = []
        page.on("pageerror", lambda error: errors.append(str(error)))
        page.evaluate("""()=>{
          const items=new Map();
          Object.defineProperty(window,'localStorage',{
            configurable:true,
            value:{
              getItem:k=>items.has(k)?items.get(k):null,
              setItem:(k,v)=>items.set(k,String(v)),
              removeItem:k=>items.delete(k),
              clear:()=>items.clear()
            }
          });
        }""")
        page.set_content(HTML, wait_until="load")
        for n in ("u2", "u3", "u4", "u5", "u6", "u7", "u11", "u12"):
            page.add_style_tag(content=ASSETS[f"{n}.css"])
        for n in ("u2", "u3", "u4", "u5", "u6", "u7"):
            page.add_script_tag(content=SCRIPTS[f"{n}.js"])
        page.wait_for_timeout(120)

        def shot(view):
            page.evaluate("document.activeElement?.blur();window.scrollTo({top:0,behavior:'instant'})")
            page.wait_for_timeout(100)
            page.screenshot(path=str(OUT / f"u12-{view}-{width}.png"), full_page=True)

        def no_horizontal_overflow():
            return page.evaluate("document.documentElement.scrollWidth<=innerWidth+2")

        check(page.locator("#stagePlay").count() == 1, "original play retained", width)
        check(page.locator("#stageNext").count() == 1, "original next retained", width)
        check(page.locator("#rtMobileMute").count() == 1, "original emergency mute retained", width)
        check(page.locator("#stagePanic").count() == 1, "original inline safety mute retained", width)
        check(page.locator("#rtU5PlayState").count() == 1, "original semantic play state", width)
        check(page.locator(".rt-u7-stage-dock button").count() == 2, "real U7 buttons reused", width)
        check(page.locator("body").evaluate(
            "(e)=>getComputedStyle(e).backgroundColor==='rgb(11, 13, 19)'"),
            "U12 visual stylesheet takes precedence after U7", width)
        check(no_horizontal_overflow(), "no Stage horizontal overflow", width)
        page.locator("#songFile").set_input_files({
            "name": "U12 Layout Fixture.wav", "mimeType": "audio/wav", "buffer": AUDIO
        })
        page.wait_for_timeout(120)
        check(page.locator("#stagePlay").is_enabled(), "synthetic local song loads original player", width)
        check(page.locator("#rtU5PlayState").inner_text() in
              ("READY", "PLAYING", "PAUSED"), "live state is original, not hard-coded", width)
        check(page.locator("#stagePanic").is_visible(), "original Stage safety mute visible", width)
        if width <= 680:
            check(page.locator(".rt-u7-stage-dock").is_visible(), "in-flow mobile dock", width)
            check(page.locator("#rtMobileMute").is_visible(), "global navigation mute", width)
            check(page.locator(".rt-u7-stage-dock").evaluate(
                "(e)=>getComputedStyle(e).position==='sticky'"),
                "sticky dock remains in-flow", width)
            check(page.evaluate("""()=>{
              const m=document.querySelector('#rtMobileMute').getBoundingClientRect();
              const a=document.querySelector('#stagePlay').getBoundingClientRect();
              const b=document.querySelector('#stageNext').getBoundingClientRect();
              const hit=(p,q)=>p.left<q.right&&p.right>q.left&&p.top<q.bottom&&p.bottom>q.top;
              return !hit(m,a)&&!hit(m,b);
            }"""), "emergency mute never overlaps Stage Play/Next", width)
            check(page.locator("#rtMobileMute").evaluate(
                "(e)=>e.parentElement.classList.contains('rt-navigation')"),
                "original navigation emergency mute placement", width)
        else:
            check(page.locator(".rt-u7-stage-dock").is_hidden(),
                  "desktop retains existing side panels", width)
        if width <= 680:
            check(page.locator("#lyricView").evaluate(
                "(e)=>e.getBoundingClientRect().height<=260"),
                "empty lyric presentation uses compact mobile height", width)
        shot("stage")

        page.locator('[data-rt-view="songs"]').click()
        check(page.locator("#rtSongsContent").is_visible(), "Songs workspace remains functional", width)
        buttons = page.locator("#rtSongsContent .rt-u2-toolbar > .rt-u2-action")
        check(buttons.count() >= 4, "all original song actions present", width)
        check(page.locator("#rtSongSearch").is_visible(), "search remains reachable", width)
        if width <= 680:
            check(page.locator("#rtSongsContent .rt-u2-toolbar").evaluate(
                "(e)=>getComputedStyle(e).display==='grid'"),
                "cramped U7 mobile toolbar upgraded to grid", width)
            boxes = buttons.evaluate_all("""els=>els.slice(0,4).map(el=>{
              const b=el.getBoundingClientRect();
              return {x:b.x,y:b.y,width:b.width,height:b.height,right:b.right}
            })""")
            check(len(boxes) == 4 and abs(boxes[0]["y"] - boxes[1]["y"]) < 2
                  and abs(boxes[2]["y"] - boxes[3]["y"]) < 2
                  and boxes[2]["y"] > boxes[0]["y"] + 30,
                  "four original song actions in two balanced rows", width)
            check(all(box["width"] > 80 and box["height"] >= 44
                      and box["right"] <= width for box in boxes),
                  "mobile song actions are touch-sized and not clipped", width)
        check(page.locator("#rtU2SongStatus").evaluate(
            "(e)=>e.getAttribute('role')==='status' && getComputedStyle(e).clipPath.includes('inset')"),
            "duplicate library status is visually compact yet aria-live remains", width)
        if width <= 680:
            check(page.locator("#rtU2SongNow").is_hidden(),
                "small-screen duplicate current title removed from summary only", width)
            check(page.locator("#rtU2SongList .rt-u2-song-row").first.is_visible(),
                "actual selectable song row remains visible", width)
        check(no_horizontal_overflow(), "no Songs overflow", width)
        shot("songs")

        page.locator('[data-rt-view="studio"]').click()
        check(page.locator("#rtStudioContent").is_visible(), "Studio workspace remains functional", width)
        preset = page.locator("#rtU2Presets .rt-u2-preset")
        check(preset.count() == 4, "all original vocal presets retained", width)
        check(page.locator("#rtU2Mute").is_visible(), "original Studio emergency mute exists", width)
        if width <= 680:
            check(page.locator("#rtU2Presets").evaluate(
                "(e)=>getComputedStyle(e).display==='grid'"),
                "Studio presets use balanced 2x2 grid", width)
            rect = preset.evaluate_all("""els=>els.map(e=>{
              const b=e.getBoundingClientRect();
              return {y:b.y,width:b.width,height:b.height}
            })""")
            check(abs(rect[0]["y"]-rect[1]["y"])<2 and
                  abs(rect[2]["y"]-rect[3]["y"])<2 and
                  rect[2]["y"]>rect[0]["y"]+30,
                  "four Studio presets form two complete rows", width)
            check(all(x["width"]>80 and x["height"]>=44 for x in rect),
                  "all Studio presets keep touch-safe geometry", width)
        check(no_horizontal_overflow(), "no Studio overflow", width)
        shot("studio")

        page.locator('[data-rt-view="stage"]').click()
        page.locator("#stageFocus").click()
        page.wait_for_timeout(80)
        check(page.locator("#stage").evaluate(
            "(e)=>e.classList.contains('focus-mode')"),
            "original focus mode retained", width)
        check(page.evaluate("""()=>{
          const lyric=document.querySelector('#lyricCurrent').getBoundingClientRect();
          const cue=document.querySelector('#rtU5LyricCue').getBoundingClientRect();
          return lyric.bottom+6<=cue.top;
        }"""), "focus placeholder does not collide with U7 cue", width)
        check(page.locator(".rt-stage-view").evaluate(
            "(e)=>getComputedStyle(e).transform==='none'"),
            "focus removes animated parent containing block without changing regular view motion", width)
        check(page.locator("#stage").evaluate("""(e)=>{
            const css=getComputedStyle(e);
            const rect=e.getBoundingClientRect();
            return css.position==='fixed' && Number(css.zIndex)>=3000 &&
                   css.overflowY==='auto' && css.overscrollBehavior==='contain' &&
                   rect.left===0 && rect.top===0 && rect.right<=innerWidth+1 &&
                   rect.bottom<=innerHeight+1;
        }"""), "fullscreen Stage is isolated and scrollable", width)
        if width <= 680:
            check(page.locator("#rtMobileMute").is_visible(), "focus emergency mute visible", width)
            check(page.locator("#stage").evaluate("""e=>{
                e.scrollTop=e.scrollHeight;
                const bottom=e.querySelector('.stage-bottom').getBoundingClientRect();
                return bottom.bottom<=innerHeight+5 && bottom.top>=-5;
            }"""), "mobile practice/transport reachable inside focus scrollport", width)
            page.locator("#stage").evaluate("(e)=>e.scrollTop=0")
        shot("focus")
        page.locator("#stageFocus").click()
        check(no_horizontal_overflow(), "no overflow after focus exit", width)
        check(not errors, "no JavaScript errors: " + repr(errors), width)
        print(f"U12 PASS synthetic Chromium {width}x{height}, cumulative {count}", flush=True)
        context.close()
    browser.close()

images = sorted(OUT.glob("u12-*.png"))
assert len(images) == 16, f"Expected exactly 16 original U12 PNGs, got {len(images)}"
(OUT / "sha256.txt").write_text(
    "".join(f"{hashlib.sha256(path.read_bytes()).hexdigest()}  {path.name}\n"
            for path in images),
    encoding="utf-8"
)
print(f"U12 PASS {count} synthetic assertions; 16 original PNGs and SHA256 manifest", flush=True)
