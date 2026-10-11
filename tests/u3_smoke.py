"""ROOMTONE U3: browser-only workspace, A11y and safe-delegation regression.
Synthetic WAV/LRC and fake UI take state are *not* hardware or recording acceptance.
Run python tests/u3_smoke.py from repo root (or local roomtone-u3 directory).
"""
from pathlib import Path
from playwright.sync_api import sync_playwright
import math,io,wave,struct
ROOT=Path(__file__).resolve().parent
if not (ROOT/'u2.js').exists(): ROOT=ROOT.parent
HTML=(ROOT/'index.html').read_text()
ASSETS={name:(ROOT/name).read_text() for name in ('u2.css','u3.css','u2.js','u3.js')}
assert HTML.count('href="u3.css"')==1
assert HTML.count('src="u3.js"')==1
wavebuf=io.BytesIO()
with wave.open(wavebuf,'wb') as wav:
 wav.setnchannels(1);wav.setsampwidth(2);wav.setframerate(22050)
 wav.writeframes(b''.join(struct.pack('<h',int(1200*math.sin(2*math.pi*440*i/22050))) for i in range(22050*4)))
WAV=wavebuf.getvalue(); n=0
with sync_playwright() as playwright:
 browser=playwright.chromium.launch(executable_path='/usr/bin/chromium' if Path('/usr/bin/chromium').exists() else None,headless=True,args=['--no-sandbox','--autoplay-policy=no-user-gesture-required'])
 for w,h in [(1440,900),(768,1024),(390,844)]:
  context=browser.new_context(viewport={'width':w,'height':h},accept_downloads=True,reduced_motion='reduce' if w==390 else 'no-preference')
  page=context.new_page(); errors=[];page.on('pageerror',lambda e:errors.append(str(e)))
  page.evaluate('''()=>{const d=new Map();Object.defineProperty(window,'localStorage',{configurable:true,value:{setItem(k,v){d.set(k,String(v))},getItem(k){return d.get(k)??null},removeItem(k){d.delete(k)}}})}''')
  page.set_content(HTML,wait_until='load')
  for name in ('u2.css','u3.css'):page.add_style_tag(content=ASSETS[name])
  for name in ('u2.js','u3.js'):page.add_script_tag(content=ASSETS[name])
  page.wait_for_timeout(160)
  def ok(cond,what):
   global n
   assert cond,f'{w}px: {what}'
   n+=1
  ok(page.locator('#rtStagePanel').is_visible(),'Stage remains first view')
  ok(page.locator('#stagePanic').count()==1 and page.locator('#rtMobileMute').count()==1,'emergency mute retained')
  ok(page.evaluate('document.documentElement.scrollWidth<=innerWidth+2'),'no overflow before navigation')
  # Keyboard navigation, no accidental trigger when only focus changes.
  page.locator('[data-rt-view="stage"]').focus()
  page.keyboard.press('ArrowRight')
  ok(page.locator('[data-rt-view="songs"]').evaluate('(e)=>document.activeElement===e'),'nav arrow focus')
  ok(page.locator('#rtStagePanel').is_visible(),'focus alone does not change current panel')
  page.keyboard.press('Enter')
  ok(page.locator('#rtSongsPanel').is_visible(),'nav Enter activates Songs')
  ok(page.locator('#rtSongsHeading').evaluate('(e)=>document.activeElement===e'),'workspace heading receives focus')
  ok(page.locator('#rtU3TrackTools').get_attribute('open') is None,'advanced track tools collapsed')
  ok(page.locator('#rtU3LyricsTools').get_attribute('open') is None,'lyrics tools collapsed')
  ok(page.locator('#songFile').count()==1 and page.locator('#bulkLyrics').count()==1,'original file and LRC editors retained')
  page.locator('#songFile').set_input_files({'name':'U3_TRACK.wav','mimeType':'audio/wav','buffer':WAV});page.wait_for_timeout(380)
  ok(page.locator('#rtU2SongList .rt-u2-song-row').count()==1,'song imported')
  page.locator('#rtSongSearch').fill('unknown');ok(page.locator('.rt-u2-empty').is_visible(),'empty filter feedback')
  page.locator('#rtSongSearch').fill('track');ok(page.locator('#rtU2SongList .rt-u2-song-row').count()==1,'filter restores song')
  page.locator('#rtU3OpenLyrics').click()
  # U3 schedules focus via requestAnimationFrame; wait for the actual result,
  # rather than racing the next statement on a busy Chromium runner.
  page.wait_for_function("document.activeElement === document.getElementById('songTitleEdit')",timeout=2000)
  ok(page.locator('#rtU3LyricsTools').get_attribute('open') is not None,'lyrics tools open via dedicated button')
  ok(page.locator('#songTitleEdit').is_visible(),'song title editing visible')
  ok(page.locator('#songTitleEdit').evaluate('(e)=>document.activeElement===e'),'edit lyrics moves focus to title')
  page.locator('#lrcFile').set_input_files({'name':'U3_TRACK.lrc','mimeType':'text/plain','buffer':b'[00:00.00]U3 first line\n[00:02.00]U3 next line\n'});page.wait_for_timeout(150)
  ok(page.locator('#lyricEditor').is_visible(),'real lyric editor still present')
  ok(page.locator('#lyricEditor').get_attribute('aria-label')=='Timed lyrics, editable line list','lyrics accessible label')
  page.locator('#rtU3OpenTracks').click()
  ok(page.locator('#rtU3TrackTools').get_attribute('open') is not None,'advanced track settings open')
  ok(page.locator('#songSpeed').is_visible(),'song speed accessible')
  page.screenshot(path=str(ROOT/f'u3-songs-{w}.png'),full_page=True)
  page.locator('[data-rt-view="studio"]').click()
  ok(page.locator('#rtU3Effects').get_attribute('open') is None,'advanced vocal processing collapsed')
  ok(page.locator('#rtU2Mic').is_visible() and page.locator('#rtU2Music').is_visible(),'quick mixer visible')
  ok(page.locator('#recordBtn').count()==1 and page.locator('#takePanel').count()==1,'recording controls singletons')
  ok(page.locator('#recordBtn').is_disabled(),'mic not started: record disabled')
  page.locator('#rtU2Mic').fill('120')
  ok(page.locator('#micGain').input_value()=='120','gain mirror')
  page.locator('#rtU2Music').fill('35')
  ok(page.locator('#musicVolume').input_value()=='35','backing mirror')
  page.locator('[data-rt-u2-preset="studio"]').click()
  ok(page.locator('[data-rt-u2-preset="studio"]').get_attribute('aria-pressed')=='true','preset activation')
  page.locator('#rtU3OpenEffects').click()
  page.locator('#saveSound').click()
  page.locator('#rtU2Mic').fill('170')
  page.locator('#loadSound').click()
  ok(page.locator('#rtU2Mic').input_value()=='120','save/load preserved')
  ok(page.locator('#reverb').is_visible(),'advanced reverb visible')
  ok(page.locator('#rtU3Effects').get_attribute('open') is not None,'effects disclosure opens')
  # Simulated render state: tests review presentation only, not an actual recording.
  page.evaluate('''()=>{const panel=document.querySelector('#takePanel');panel.classList.remove('hidden')}''')
  page.wait_for_timeout(60)
  ok(page.locator('#rtU3RecordState').inner_text().startswith('Take ready.'),'take ready announcement')
  ok(page.locator('#takePlayer').get_attribute('aria-label')=='Recorded vocal playback','take player accessible')
  ok(page.locator('#takeDownload').count()==1,'download link retained')
  page.screenshot(path=str(ROOT/f'u3-studio-{w}.png'),full_page=True)
  page.evaluate('''()=>{document.querySelector('#takePanel').classList.add('hidden')}''')
  page.locator('[data-rt-view="setup"]').click()
  ok(page.locator('#rtU3About').get_attribute('open') is None,'about collapsed')
  ok(page.locator('#rtU3Lab').get_attribute('open') is None,'lab collapsed')
  ok(page.locator('#inputSelect').is_visible() and page.locator('#outputSelect').is_visible(),'real device routing remains visible')
  ok(page.locator('#rtSetupContent .notice').is_visible(),'feedback warning visible')
  page.locator('#rtU3OpenLab').click()
  ok(page.locator('#rtU3Lab').get_attribute('open') is not None,'calibration expands')
  page.locator('#rtU2GotoQa').click()
  ok(page.locator('#hardwareQa').get_attribute('open') is not None,'H1 checklist accessible')
  page.screenshot(path=str(ROOT/f'u3-setup-{w}.png'),full_page=True)
  page.locator('[data-rt-view="stage"]').click()
  page.locator('#stagePlay').click();page.wait_for_timeout(130)
  ok(page.locator('#songPlayer').evaluate('(e)=>!e.paused'),'H2 Play starts')
  page.locator('#stagePlay').click()
  ok(page.locator('#songPlayer').evaluate('(e)=>e.paused'),'H2 Pause stops')
  page.locator('#jumpToStudio').click();page.wait_for_timeout(60)
  ok(page.locator('#rtU3LyricsTools').get_attribute('open') is not None,'Stage lyrics jump expands hidden editor')
  ok(page.locator('#lyricsStudio').is_visible(),'Stage lyrics jump navigates Songs')
  ok(page.evaluate('document.documentElement.scrollWidth<=innerWidth+2'),'no horizontal overflow after interactions')
  ok(not errors,f'JS errors {errors}')
  context.close()
  print('PASS',w,'x',h,'assertions cumulative',n)
 browser.close()
print('TOTAL U3 assertions',n)