"""U4 release candidate headless Chromium: local synthetic source, no H1 device acceptance.
Run python tests/u4_smoke.py. The mocked MediaRecorder validates UI and browser
Blob export only, never acoustic feedback, microphone hardware or genuine recording.
"""
from pathlib import Path
import hashlib,io,struct,wave,math
from playwright.sync_api import sync_playwright
ROOT=Path(__file__).resolve().parent.parent
HTML=(ROOT/'index.html').read_text()
for a in ('u2','u3','u4'):
 assert HTML.count(f'href="{a}.css"')==1 and HTML.count(f'src="{a}.js"')==1
ASSETS={name:(ROOT/name).read_text() for name in ('u2.css','u3.css','u4.css','u2.js','u3.js','u4.js')}
OUT=ROOT/'u4-evidence';OUT.mkdir(exist_ok=True)
b=io.BytesIO()
with wave.open(b,'wb') as wav:
 wav.setnchannels(1);wav.setsampwidth(2);wav.setframerate(22050)
 wav.writeframes(b''.join(struct.pack('<h',int(700*math.sin(2*math.pi*330*i/22050))) for i in range(22050*4)))
WAV=b.getvalue()
checks=0
with sync_playwright() as p:
 browser=p.chromium.launch(executable_path='/usr/bin/chromium' if Path('/usr/bin/chromium').exists() else None,headless=True,args=['--no-sandbox','--autoplay-policy=no-user-gesture-required'])
 for width,height in [(1440,900),(768,1024),(390,844)]:
  ctx=browser.new_context(viewport={'width':width,'height':height},accept_downloads=True,reduced_motion='reduce' if width==390 else 'no-preference')
  page=ctx.new_page();errors=[];page.on('pageerror',lambda e:errors.append(str(e)))
  page.evaluate("""()=>{const d=new Map();Object.defineProperty(window,'localStorage',{configurable:true,value:{getItem(k){return d.get(k)||null},setItem(k,v){d.set(k,String(v))},removeItem(k){d.delete(k)}}})}""")
  page.set_content(HTML,wait_until='load')
  for n in ('u2.css','u3.css','u4.css'):page.add_style_tag(content=ASSETS[n])
  for n in ('u2.js','u3.js','u4.js'):page.add_script_tag(content=ASSETS[n])
  page.wait_for_timeout(100)
  def ok(value,message):
   global checks
   assert value,f'{width}px: {message}'
   checks+=1
  ok(page.locator('#rtStagePanel').is_visible(),'stage default')
  ok(page.locator('#stagePanic').count()==1 and page.locator('#rtMobileMute').count()==1,'panic controls singleton')
  page.keyboard.press('Tab')
  ok(page.locator('.rt-u4-skip').evaluate('(e)=>e===document.activeElement'),'skip in first tab position')
  page.keyboard.press('Enter')
  ok(page.evaluate("document.activeElement.closest('#rtStagePanel')!==null"),'skip focuses visible heading')
  page.locator('[data-rt-view="songs"]').click()
  page.locator('#songFile').set_input_files([
   {'name':'First U4.wav','mimeType':'audio/wav','buffer':WAV},
   {'name':'Second U4.wav','mimeType':'audio/wav','buffer':WAV}])
  page.wait_for_timeout(425)
  ok(page.locator('#trackQueue .queue-item').count()==2,'file imports')
  ok(page.locator('.rt-u2-song-select').count()==2,'mirrored queue')
  page.locator('.rt-u2-song-select').nth(1).focus();page.keyboard.press('Enter');page.wait_for_timeout(90)
  ok(page.locator('.rt-u2-song-select').nth(1).evaluate('(e)=>e===document.activeElement'),'song keyboard focus preserved')
  ok(page.locator('.rt-u2-song-select').nth(1).get_attribute('aria-current')=='true','active song current semantics')
  ok('Second U4' in page.locator('#rtU4TrackAnnouncement').inner_text(),'selected song announced')
  page.locator('#rtSongSearch').fill('first')
  ok(page.locator('.rt-u2-song-select').count()==1,'search filters')
  page.locator('#rtSongSearch').fill('')
  ok(page.locator('.rt-u2-song-select').count()==2,'filter clearing')
  page.locator('#rtU3OpenTracks').click()
  page.locator('#trackQueue .queue-item').nth(1).locator('.queue-move').first.focus()
  page.keyboard.press('Enter');page.wait_for_timeout(90)
  ok('Second U4' in page.locator('#trackQueue .queue-item').first.locator('.queue-play').inner_text(),'original reorder')
  ok(page.locator('#trackQueue .queue-item').first.locator('.queue-play').evaluate('(e)=>e===document.activeElement'),'reorder restores focus')
  page.locator('#trackQueue .queue-item').nth(1).locator('.queue-remove').focus()
  page.keyboard.press('Enter');page.wait_for_timeout(90)
  ok(page.locator('#trackQueue .queue-item').count()==1,'original remove')
  ok(page.locator('#trackQueue .queue-play').first.evaluate('(e)=>e===document.activeElement'),'remove preserves focus')
  page.locator('#lrcFile').set_input_files({'name':'Second U4.lrc','mimeType':'text/plain','buffer':b'[00:00.00]U4 lyrics\n[00:02.00]Next line\n'})
  page.locator('#rtU3OpenLyrics').click();page.wait_for_timeout(60)
  ok(page.locator('#songTitleEdit').evaluate('(e)=>e===document.activeElement'),'lyrics open and focus')
  ok(page.locator('#lyricEditor').is_visible(),'real LRC editor available')
  page.screenshot(path=str(OUT/f'u4-songs-{width}.png'),full_page=True)
  page.locator('[data-rt-view="stage"]').click()
  page.locator('#rtQueueMirror .rt-queue-choice').first.focus()
  page.keyboard.press('Enter');page.wait_for_timeout(90)
  ok(page.locator('#rtQueueMirror .rt-queue-choice').first.evaluate('(e)=>e===document.activeElement'),'stage queue focus preserved')
  ok(page.locator('#rtQueueMirror .rt-queue-choice').first.get_attribute('aria-current')=='true','stage current semantics')
  page.locator('#stagePlay').click();page.wait_for_timeout(180)
  ok(page.locator('#songPlayer').evaluate('(e)=>!e.paused'),'H2 play works')
  page.locator('#stagePlay').click()
  ok(page.locator('#songPlayer').evaluate('(e)=>e.paused'),'H2 pause works')
  page.screenshot(path=str(OUT/f'u4-stage-{width}.png'),full_page=True)
  page.locator('#jumpToStudio').focus();page.keyboard.press('Enter');page.wait_for_timeout(100)
  ok(page.locator('#rtSongsPanel').is_visible(),'stage jumps to lyrics')
  ok(page.locator('#rtU3LyricsTools').get_attribute('open') is not None,'editor disclosure opened')
  ok(page.locator('#songTitleEdit').evaluate('(e)=>e===document.activeElement'),'stage to lyrics restores focus')
  page.locator('[data-rt-view="studio"]').click()
  page.locator('#rtU2Mic').fill('125')
  page.locator('#rtU2Music').fill('32')
  ok(page.locator('#micGain').input_value()=='125' and page.locator('#musicVolume').input_value()=='32','original audio settings synchronized')
  page.locator('#rtU3OpenEffects').click()
  page.locator('#saveSound').click()
  page.locator('#rtU2Mic').fill('170')
  page.locator('#loadSound').click()
  ok(page.locator('#rtU2Mic').input_value()=='125','save/load working')
  page.evaluate("document.querySelector('#takePanel').classList.remove('hidden')")
  page.wait_for_timeout(65)
  ok('No playable recording available' in page.locator('#rtU3RecordState').inner_text(),'not claiming an unsaved take')
  page.evaluate("document.querySelector('#takePanel').classList.add('hidden')")
  page.evaluate("""()=>{
    class FakeMediaRecorder{
      constructor(stream,opts){this.state='inactive';this.mimeType='audio/webm';this.listeners={};}
      static isTypeSupported(t){return t.startsWith('audio/webm');}
      start(){this.state='recording'}
      stop(){this.state='inactive';queueMicrotask(()=>{this.ondataavailable?.({data:new Blob(['synthetic U4 take'],{type:this.mimeType})});this.onstop?.();for(const f of this.listeners.stop||[])f();});}
      addEventListener(n,fn){(this.listeners[n]??=[]).push(fn)}
    }
    window.MediaRecorder=FakeMediaRecorder;ctx={};nodes={recordDest:{stream:{}}};setRecordingUi(false);
  }""")
  page.locator('#recordBtn').click()
  ok(page.locator('#recordStopBtn').is_enabled(),'mock lifecycle recording started')
  page.locator('#recordStopBtn').focus();page.keyboard.press('Enter');page.wait_for_timeout(130)
  ok(page.locator('#takePanel').is_visible(),'mock lifecycle take visible')
  ok(page.locator('#takeDownload').get_attribute('href').startswith('blob:'),'local blob URL')
  ok(page.locator('#takeDownload').evaluate('(e)=>e===document.activeElement'),'keyboard stop focuses playable take')
  with page.expect_download() as d:page.locator('#takeDownload').click()
  ok(d.value.suggested_filename.startswith('roomtone-vocal-'),'original download name')
  ok(d.value.failure() is None,'mock local download completes')
  page.screenshot(path=str(OUT/f'u4-studio-{width}.png'),full_page=True)
  page.locator('[data-rt-view="setup"]').click()
  ok(page.locator('#inputSelect').is_visible() and page.locator('#outputSelect').is_visible(),'device selectors visible')
  ok(page.locator('#rtSetupContent .notice').is_visible(),'hardware warning present')
  page.locator('#rtU2GotoQa').click()
  ok(page.locator('#hardwareQa').get_attribute('open') is not None,'H1 checklist remains')
  page.screenshot(path=str(OUT/f'u4-setup-{width}.png'),full_page=True)
  ok(page.evaluate('document.documentElement.scrollWidth<=innerWidth+2'),'no horizontal overflow')
  ok(not errors,f'no page exceptions {errors}')
  ctx.close()
  print(f'PASS U4 {width}x{height}; cumulative {checks} assertions',flush=True)
 browser.close()
files=sorted(OUT.glob('u4-*.png'))
assert len(files)==12
(OUT/'sha256.txt').write_text(''.join(f'{hashlib.sha256(f.read_bytes()).hexdigest()}  {f.name}\n' for f in files))
print(f'PASS U4 {checks} assertions; {len(files)} original PNGs hashed',flush=True)
