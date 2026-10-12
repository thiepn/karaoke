"""U21 genuine Chromium local-HTTP offline/network recovery, focus & AX checks.

Real 127.0.0.1 ephemeral HTTP server with no-store source responses; Chromium
network toggled using BrowserContext.set_offline. NO external origin, account,
uploads, hardware, microphone, user data, real Android or OS background claim.
"""
from pathlib import Path
from http.server import ThreadingHTTPServer, SimpleHTTPRequestHandler
from threading import Thread
from urllib.parse import urlsplit
from playwright.sync_api import sync_playwright, Error as PlaywrightError
import hashlib

ROOT=Path(__file__).resolve().parents[1]
HTML=(ROOT/"index.html").read_text(encoding="utf-8")
for name in ("u14","u18","u19"):
    assert HTML.count(f'src="{name}.js"')==1
OUT=ROOT/"u21-evidence"
OUT.mkdir(exist_ok=True)
assert not list(OUT.glob("u21-*.png")) and not (OUT/"sha256.txt").exists()
counts=0
actual_hidden=0
actual_blur=0
offline_reload_unavailable=0

class LocalSource(SimpleHTTPRequestHandler):
    def __init__(self,*args,**kwargs):
        super().__init__(*args,directory=str(ROOT),**kwargs)
    def log_message(self,*args):
        pass
    def end_headers(self):
        # Browser fixture policy, NOT the remote application's hosting policy.
        self.send_header("Cache-Control","no-store")
        super().end_headers()

server=ThreadingHTTPServer(("127.0.0.1",0),LocalSource)
thread=Thread(target=server.serve_forever,daemon=True)
thread.start()
origin=f"http://127.0.0.1:{server.server_address[1]}"

def check(value,label,width):
    global counts
    assert value,f"U21 {width}px: {label}"
    counts+=1

def view_state(page):
    return page.evaluate("""()=>({
      view:document.querySelector('.shell')?.dataset.rtCurrentView,
      active:document.activeElement?.id||'',
      fullscreen:document.querySelector('#stage')?.classList.contains('focus-mode'),
      scrollLocked:document.body.classList.contains('in-focus'),
      dialog:document.querySelector('#stage')?.getAttribute('role'),
      modal:document.querySelector('#stage')?.getAttribute('aria-modal'),
      headerInert:document.querySelector('.rt-header')?.inert,
      sidecarInert:document.querySelector('.rt-stage-sidecar')?.inert,
      muteParent:document.querySelector('#rtMobileMute')?.parentElement.id||'',
      muteCount:document.querySelectorAll('#rtMobileMute').length,
      hasFocus:document.hasFocus(),visibility:document.visibilityState,
      online:navigator.onLine,search:!!document.querySelector('#rtSongSearch'),
      overflow:document.documentElement.scrollWidth>innerWidth+2
    })""")

try:
  with sync_playwright() as pw:
    browser=pw.chromium.launch(
        executable_path="/usr/bin/chromium" if Path("/usr/bin/chromium").exists() else None,
        headless=True,args=["--no-sandbox"])
    for width,height in ((1440,900),(768,1024),(390,844),(350,740)):
      ctx=browser.new_context(viewport={"width":width,"height":height},
          reduced_motion="reduce" if width<=390 else "no-preference",
          service_workers="block")
      page=ctx.new_page()
      page_errors=[]
      requested=[]
      page.on("pageerror",lambda exc:page_errors.append(str(exc)))
      page.on("request",lambda req:requested.append(req.url))
      url=origin+"/index.html"
      response=page.goto(url,wait_until="load")
      check(response is not None and response.status==200,
            "original HTML loaded from actual local HTTP server",width)
      page.wait_for_function("""()=>document.querySelector('.shell')
                                  ?.dataset.rtCurrentView==='stage'""")
      styles=page.evaluate("""()=>[...document.styleSheets]
             .filter(s=>s.href?.includes('127.0.0.1')).map(s=>s.href)""")
      check(all(any("/"+n+".css" in u for u in styles)
          for n in ("u2","u3","u4","u5","u6","u7","u11","u12","u13")),
          "all nine original modular stylesheets load through HTTP",width)
      check(all(page.evaluate("""(s)=>[...document.scripts]
          .some(e=>e.src?.endsWith('/'+s+'.js'))""",n)
          for n in ("u2","u3","u4","u5","u6","u7","u14","u18","u19")),
          "all nine original app scripts linked to HTTP source",width)
      check(not any(urlsplit(u).netloc not in
          (urlsplit(origin).netloc,"") for u in requested),
          "no remote application resource requests",width)
      check(view_state(page)["muteCount"]==1,
          "original safety Mute is unique",width)
      check(not page_errors,"no boot JavaScript errors: "+repr(page_errors),width)
      page.screenshot(path=str(OUT/f"u21-http-online-{width}.png"),
          full_page=False)

      # Synthetic isolated test sentinel, NOT an existing app user key.
      page.evaluate("""()=>localStorage.setItem(
         'roomtone:u21:synthetic-ephemeral','source-browser-check')""")
      ctx.set_offline(True)
      page.wait_for_function("navigator.onLine===false")
      check(view_state(page)["online"] is False,
            "genuine browser context reports offline",width)
      request_result=page.evaluate("""async()=>{
        try {
          const response=await fetch('/u19.js',{cache:'no-store'});
          return {ok:true,status:response.status};
        }catch(e){return {ok:false,name:e.name};}
      }""")
      check(request_result["ok"] is False,
            "fresh network fetch correctly blocked by actual offline mode",width)

      # The already-loaded app must remain interactive when disconnected.
      page.locator("#stageFocus").click()
      page.wait_for_function("""()=>document.getElementById('stage')
          .getAttribute('aria-modal')==='true'""")
      s=view_state(page)
      check(s["fullscreen"] and s["scrollLocked"] and
            s["headerInert"] and s["sidecarInert"] and
            s["dialog"]=="dialog" and s["modal"]=="true",
            "offline Stage modal works and background remains inert",width)
      check(s["muteParent"]=="stage",
            "original emergency Mute remains in offline fullscreen",width)
      ax=ctx.new_cdp_session(page).send("Accessibility.getFullAXTree")["nodes"]
      nodes=[n for n in ax if not n.get("ignored")]
      check(any(n.get("role",{}).get("value")=="dialog" for n in nodes),
            "actual Chromium accessibility tree has offline Stage dialog",width)
      page.screenshot(path=str(OUT/f"u21-http-offline-modal-{width}.png"),
          full_page=False)
      page.locator("#stageFocus").focus()
      for direction in ("Tab","Shift+Tab"):
        for i in range(8):
          page.keyboard.press(direction)
          check(page.evaluate(
              "!!document.activeElement?.closest('#stage')"),
              f"offline {direction} {i} focus contained inside Stage",width)
      page.keyboard.press("Escape")
      page.wait_for_function("!document.body.classList.contains('in-focus')")
      s=view_state(page)
      check(not s["fullscreen"] and not s["headerInert"]
          and not s["sidecarInert"] and s["dialog"] is None,
          "offline Escape releases all original modal state",width)
      check(page.locator("#rtMobileMute").evaluate(
          "(e)=>e.parentElement.classList.contains('rt-navigation')"),
          "original Mute restored offline",width)

      page.locator('[data-rt-view="songs"]').click()
      s=view_state(page)
      check(s["view"]=="songs" and s["active"]=="rtSongsHeading" and s["search"],
            "offline Songs heading and search remain available",width)
      page.screenshot(path=str(OUT/f"u21-http-offline-songs-{width}.png"),
          full_page=False)
      page.locator('[data-rt-view="studio"]').click()
      s=view_state(page)
      check(s["view"]=="studio" and s["active"]=="rtStudioHeading",
            "offline Studio navigation and keyboard heading work",width)
      page.locator('[data-rt-view="stage"]').click()
      check(view_state(page)["view"]=="stage",
            "offline Stage reentry available without network",width)

      # Real Chromium tab focus is observed; headless visibility might remain
      # visible and must not be fabricated as OS background testing.
      before=view_state(page)
      other=ctx.new_page()
      other.goto("about:blank")
      other.bring_to_front()
      after=view_state(page)
      if after["visibility"]=="hidden":
        actual_hidden+=1
      else:
        print(f"U21 NOT_TESTED actual page-hidden state at {width}px "
              f"(reported {after['visibility']})",flush=True)
      if before["hasFocus"] and not after["hasFocus"]:
        actual_blur+=1
      else:
        print(f"U21 NOT_TESTED actual page-blur transition at {width}px "
              f"(before={before['hasFocus']}, after={after['hasFocus']})",
              flush=True)
      other.close()
      page.bring_to_front()
      check(view_state(page)["view"]=="stage",
            "Stage preserved after headless tab switch",width)

      # Check offline **reload** separately from already-loaded offline use:
      # With no service-worker cache and no-store test HTTP responses, failure
      # is an explicit limitation, not a newly invented app acceptance gate.
      try:
        reload=page.reload(wait_until="domcontentloaded",timeout=7000)
        if reload and reload.ok:
          print(f"U21 offline reload OBSERVED success at {width}px",flush=True)
        else:
          offline_reload_unavailable+=1
          print(f"U21 OFFLINE_RELOAD_UNAVAILABLE at {width}px "
                "(no installed offline navigation cache)",flush=True)
      except PlaywrightError as error:
        offline_reload_unavailable+=1
        print(f"U21 OFFLINE_RELOAD_UNAVAILABLE at {width}px "
              f"(browser network error: {str(error)[:130]})",flush=True)

      ctx.set_offline(False)
      response=page.goto(url,wait_until="load")
      check(response is not None and response.status==200,
            "original local HTML reinitializes after reconnect",width)
      page.wait_for_function("""()=>document.querySelector('.shell')
                                 ?.dataset.rtCurrentView==='stage'""")
      check(page.evaluate("""()=>localStorage.getItem(
             'roomtone:u21:synthetic-ephemeral')""")==
             "source-browser-check",
            "synthetic ephemeral storage persists through network recovery",width)
      check(view_state(page)["muteCount"]==1 and
            not view_state(page)["scrollLocked"] and
            not view_state(page)["headerInert"],
            "safe unlocked Stage after network return",width)
      page.locator("#stageFocus").click()
      page.wait_for_function("document.body.classList.contains('in-focus')")
      page.keyboard.press("Escape")
      page.wait_for_function("!document.body.classList.contains('in-focus')")
      check(view_state(page)["dialog"] is None and
            not view_state(page)["sidecarInert"],
            "Stage focus works again after offline recovery",width)
      check(not view_state(page)["overflow"],
            "no horizontal overflow after offline recovery",width)
      check(not page_errors,
            "no page errors during offline lifecycle: "+repr(page_errors),width)
      page.screenshot(path=str(OUT/f"u21-http-restored-{width}.png"),
          full_page=False)
      print(f"U21 PASS {width}x{height}, cumulative={counts}",flush=True)
      ctx.close()
    browser.close()
finally:
  server.shutdown()
  server.server_close()
  thread.join(timeout=5)

png=sorted(OUT.glob("u21-*.png"))
assert len(png)==16,f"expected 16 original U21 PNGs, saw {len(png)}"
(OUT/"sha256.txt").write_text("".join(
    f"{hashlib.sha256(p.read_bytes()).hexdigest()}  {p.name}\n" for p in png),
    encoding="utf-8")
print(f"U21 PASS {counts} local-HTTP/offline Chromium assertions "
      f"and 16 original PNGs; real tab-hidden observed {actual_hidden}/4, "
      f"tab-blur observed {actual_blur}/4; offline reload unavailable "
      f"{offline_reload_unavailable}/4",flush=True)
