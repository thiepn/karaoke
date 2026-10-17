"""A1R: genuine local-HTTP Chromium route tests with fake media; NOT physical latency."""
from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from threading import Thread
from playwright.sync_api import sync_playwright

root=Path(__file__).resolve().parent.parent
class Quiet(SimpleHTTPRequestHandler):
 def log_message(self,*args):pass
handler=partial(Quiet,directory=str(root))
server=ThreadingHTTPServer(('127.0.0.1',0),handler)
thread=Thread(target=server.serve_forever,daemon=True)
thread.start()
url=f'http://127.0.0.1:{server.server_port}/index.html'
checks=0
try:
 with sync_playwright() as pw:
  browser=pw.chromium.launch(headless=True,args=[
   '--no-sandbox','--use-fake-ui-for-media-stream',
   '--use-fake-device-for-media-stream','--autoplay-policy=no-user-gesture-required'])
  for width in (1280,390):
   context=browser.new_context(viewport={'width':width,'height':850},permissions=['microphone'])
   page=context.new_page()
   errors=[]
   page.on('pageerror',lambda e:errors.append(str(e)))
   # Simulate a browser that does not implement AudioContext.setSinkId.
   # The HTMLMediaElement fallback remains available for explicit output.
   page.add_init_script("""(() => {
    Object.defineProperty(AudioContext.prototype,'setSinkId',{value:undefined,configurable:true});
    Object.defineProperty(HTMLMediaElement.prototype,'setSinkId',{value:async function(id){this.__testSink=id},configurable:true});
   })()""")
   page.goto(url)
   page.evaluate("() => startMic()")
   # The route string can settle BEFORE async output gain gates switch.
   # Wait for original source-owned routing AND gain policy together;
   # otherwise a fast runner samples a transient state as an A1 failure.
   page.wait_for_function("() => engine?.route === 'default-direct' && engine.direct && engine.directGate.gain.value > .99 && engine.compatGate.gain.value < .01")
   result=page.evaluate("""() => ({
    route:engine.route, direct:engine.direct, fallback:ui.voicePlayback.paused,
    record:engine.recordDest===nodes.recordDest, monitor:speakerOn,
    directGain:engine.directGate.gain.value, compatGain:engine.compatGate.gain.value,
    capture:stream.getAudioTracks()[0].getSettings().echoCancellation
   })""")
   assert result['direct'] and result['fallback'] and result['record'] and not result['monitor'],result
   assert result['directGain']==1 and result['compatGain']==0,result
   checks+=2
   # The explicit route uses the media-element sink without rebuilding or
   # affecting the recording destination; voice must remain muted.
   page.evaluate("""() => {
    const select=ui.outputSelect;
    select.add(new Option('Synthetic Pebble','test-pebble'));
    select.value='test-pebble';
    select.dispatchEvent(new Event('change',{bubbles:true}));
   }""")
   page.wait_for_function("() => engine?.route === 'media-element' && engine.compatGate.gain.value > .99 && engine.directGate.gain.value < .01")
   result=page.evaluate("""() => ({
    route:engine.route, direct:engine.direct, paused:ui.voicePlayback.paused,
    sink:ui.voicePlayback.__testSink, record:engine.recordDest===nodes.recordDest,
    monitor:speakerOn, directGain:engine.directGate.gain.value,
    compatGain:engine.compatGate.gain.value
   })""")
   assert result['sink']=='test-pebble' and not result['direct'] and not result['paused'],result
   assert result['record'] and not result['monitor'] and result['directGain']==0 and result['compatGain']==1,result
   checks+=2
   page.evaluate("""() => {
    ui.outputSelect.value='';
    ui.outputSelect.dispatchEvent(new Event('change',{bubbles:true}));
   }""")
   page.wait_for_function("() => engine?.route === 'default-direct' && ui.voicePlayback.paused && engine.directGate.gain.value > .99 && engine.compatGate.gain.value < .01")
   assert page.evaluate("() => !speakerOn && engine.direct && engine.directGate.gain.value===1")
   checks+=1
   # Failure to select an unsupported device must not silently switch audio
   # or enable monitoring; the previous default output must remain active.
   page.evaluate("""() => {
    Object.defineProperty(HTMLMediaElement.prototype,'setSinkId',{value:undefined,configurable:true});
    ui.outputSelect.value='test-pebble';
    ui.outputSelect.dispatchEvent(new Event('change',{bubbles:true}));
   }""")
   page.wait_for_function("() => ui.outputSelect.value === ''")
   assert page.evaluate("() => engine.route==='default-direct' && !speakerOn")
   checks+=1
   assert not errors,errors
   checks+=1
   context.close()
  browser.close()
finally:
 server.shutdown()
 server.server_close()
print('PASS A1R',checks,'fake-device route assertions; real QuadCast/Pebble latency NOT TESTED')
