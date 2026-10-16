"""Real local-HTTP Chromium proof of explicit offline handoff; no hardware latency."""
from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from threading import Thread
from playwright.sync_api import sync_playwright
root=Path(__file__).resolve().parent.parent
class Quiet(SimpleHTTPRequestHandler):
 def log_message(self,*args):pass
server=ThreadingHTTPServer(('127.0.0.1',0),partial(Quiet,directory=str(root)))
Thread(target=server.serve_forever,daemon=True).start()
checks=0
try:
 with sync_playwright() as pw:
  browser=pw.chromium.launch(headless=True,args=['--no-sandbox','--autoplay-policy=no-user-gesture-required'])
  for width in (1280,390):
   context=browser.new_context(viewport={'width':width,'height':860},accept_downloads=True)
   page=context.new_page();errors=[]
   page.on('pageerror',lambda err:errors.append(str(err)))
   page.goto(f'http://127.0.0.1:{server.server_port}/index.html')
   page.locator('#rtNativeHandoff').wait_for()
   assert page.locator('#rtHandoffPrepare').is_disabled()
   checks+=1
   page.evaluate("""() => {
     const f=new File([new Uint8Array(10)],'karaoke.wav',{type:'audio/wav'});
     queue.push({file:f,url:URL.createObjectURL(f),
       title:'Practice Set',offset:-200,lyrics:[{time:1.25,text:'First line'},{time:2.5,text:'Second line'}],
       loop:{a:1,b:3,enabled:true}});
     selectTrack(0);
   }""")
   page.wait_for_function("() => !document.getElementById('rtHandoffPrepare').disabled")
   checks+=1
   page.locator('[data-rt-view="songs"]').first.click()
   with page.expect_download() as received:
    page.locator('#rtHandoffPrepare').click()
   download=received.value
   file=Path(download.path())
   text=file.read_text(encoding='utf-8')
   assert download.suggested_filename=='karaoke.rtk',download.suggested_filename
   assert text.startswith('ROOMTONE_HANDOFF_V1\n'),text[:150]
   assert 'audio\tkaraoke.wav\n' in text
   assert 'title\tPractice%20Set\n' in text
   assert 'offset-ms\t-200\n' in text
   assert 'loop\t1\nloop-start-ms\t1000\nloop-end-ms\t3000\n' in text
   assert 'lyric\t1250\tFirst%20line\nlyric\t2500\tSecond%20line\n' in text
   assert 'songUrl' not in text and 'blob:' not in text
   checks+=7
   page.wait_for_function("() => !document.getElementById('rtHandoffReturn').hidden")
   assert page.evaluate("() => songPlayer.paused && !speakerOn && !nodes")
   checks+=1
   # Programmatic browser playback must be paused again under native ownership.
   page.evaluate("""() => {
      const p=document.getElementById('songPlayer');
      p.dispatchEvent(new Event('play'));
   }""")
   assert page.evaluate("() => songPlayer.paused")
   checks+=1
   page.locator('#rtHandoffReturn').click()
   page.wait_for_function("() => document.getElementById('rtHandoffReturn').hidden")
   assert page.locator('#rtHandoffPrepare').is_enabled()
   assert not errors,errors
   checks+=2
   context.close()
  browser.close()
finally:
 server.shutdown();server.server_close()
print('PASS offline ROOMTONE browser handoff',checks,'desktop/mobile assertions; real physical audio NOT TESTED')
