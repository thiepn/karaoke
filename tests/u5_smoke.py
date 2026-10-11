"""U5 original Chromium UI evidence; synthetic local tracks only, no physical audio claims."""
from pathlib import Path
import hashlib,io,math,struct,wave
from playwright.sync_api import sync_playwright

ROOT=Path(__file__).resolve().parent.parent
HTML=(ROOT/'index.html').read_text()
for n in ('u2','u3','u4','u5'):
    assert HTML.count(f'href="{n}.css"')==1 and HTML.count(f'src="{n}.js"')==1
ASSETS={f'{n}.{ext}':(ROOT/f'{n}.{ext}').read_text() for n in ('u2','u3','u4','u5') for ext in ('css','js')}
OUT=ROOT/'u5-evidence'
OUT.mkdir(exist_ok=True)
sound=io.BytesIO()
with wave.open(sound,'wb') as w:
    w.setnchannels(1);w.setsampwidth(2);w.setframerate(22050)
    w.writeframes(b''.join(struct.pack('<h',int(600*math.sin(2*math.pi*300*i/22050))) for i in range(22050*3)))
WAV=sound.getvalue()
checks=0
with sync_playwright() as p:
    browser=p.chromium.launch(executable_path='/usr/bin/chromium' if Path('/usr/bin/chromium').exists() else None,
                              headless=True,args=['--no-sandbox','--autoplay-policy=no-user-gesture-required'])
    for width,height in [(1440,900),(768,1024),(390,844)]:
        ctx=browser.new_context(viewport={'width':width,'height':height},reduced_motion='reduce' if width==390 else 'no-preference')
        page=ctx.new_page()
        errors=[]
        page.on('pageerror',lambda e:errors.append(str(e)))
        page.evaluate("""()=>{const m=new Map();Object.defineProperty(window,'localStorage',
          {configurable:true,value:{getItem:k=>m.get(k)||null,setItem:(k,v)=>m.set(k,String(v)),
          removeItem:k=>m.delete(k)}})}""")
        page.set_content(HTML,wait_until='load')
        for name in ('u2','u3','u4','u5'): page.add_style_tag(content=ASSETS[f'{name}.css'])
        for name in ('u2','u3','u4','u5'): page.add_script_tag(content=ASSETS[f'{name}.js'])
        page.wait_for_timeout(100)
        def ok(condition,message):
            global checks
            assert condition,f'{width}px: {message}'
            checks+=1
        def shot(view):
            page.evaluate("document.activeElement?.blur();window.scrollTo({top:0,behavior:'instant'})")
            page.wait_for_timeout(85)
            page.screenshot(path=str(OUT/f'u5-{view}-{width}.png'),full_page=True)
        ok(page.locator('.rt-u5-stage-meta').count()==1,'new stage header is unique')
        ok(page.locator('#rtU5PlayState').inner_text()=='AWAITING TRACK','truthful empty state')
        ok(page.locator('#rtU5EmptyAdd').is_visible(),'stage offers an import')
        ok(page.locator('#stagePanic').count()==1 and page.locator('#rtMobileMute').count()==1,'emergency controls retained')
        ok(page.locator('#rtU5LibraryFilters button').count()==3,'three song filters')
        page.locator('[data-rt-view="songs"]').click()
        page.locator('#songFile').set_input_files([
          {'name':'First U5.wav','mimeType':'audio/wav','buffer':WAV},
          {'name':'Second U5.wav','mimeType':'audio/wav','buffer':WAV}])
        page.wait_for_timeout(380)
        ok(page.locator('#trackQueue .queue-item').count()==2,'original local tracks imported')
        ok(page.locator('#rtU5FilteredCount').inner_text()=='2 songs shown','song count')
        page.locator('[data-rt-u5-filter="ready"]').click()
        ok(page.locator('.rt-u2-song-row:visible').count()==0,'zero tracks with lyrics initially')
        page.locator('[data-rt-u5-filter="missing"]').click()
        ok(page.locator('.rt-u2-song-row:visible').count()==2,'tracks missing lyrics')
        page.locator('[data-rt-u5-filter="all"]').click()
        ok(page.locator('.rt-u2-song-row:visible').count()==2,'all tracks restored')
        page.locator('#rtSongSearch').fill('second')
        ok(page.locator('.rt-u2-song-row:visible').count()==1,'search coexists with filters')
        page.locator('#rtSongSearch').fill('')
        page.locator('.rt-u2-song-select').nth(1).click()
        page.locator('#lrcFile').set_input_files({'name':'Second U5.lrc','mimeType':'text/plain','buffer':b'[00:00.00]The first line\n[00:01.00]Sing the next line\n'})
        page.wait_for_timeout(125)
        page.locator('[data-rt-u5-filter="ready"]').click()
        ok(page.locator('.rt-u2-song-row:visible').count()==1,'LRC metadata drives filter')
        ok('Second U5' in page.locator('.rt-u2-song-row:visible').first.inner_text(),'correct ready track')
        ok(page.locator('[data-rt-u5-filter="ready"]').get_attribute('aria-pressed')=='true','filter pressed semantics')
        shot('songs')
        page.locator('[data-rt-view="stage"]').click()
        ok(page.locator('#rtU5EmptyAdd').is_hidden(),'empty import button hidden when loaded')
        ok(page.locator('#rtU5LyricCue').inner_text()=='SYNCHRONIZED LYRICS','actual synced lyric cue')
        ok('The first line' in page.locator('#lyricCurrent').inner_text(),'original lyric engine active')
        ok(page.locator('#rtU5DockQueueCount').inner_text()=='02','queue count comes from source')
        page.locator('#stagePlay').click()
        page.wait_for_timeout(150)
        ok(page.locator('#rtU5PlayState').inner_text()=='PLAYING','player status reflects play')
        page.locator('#stagePlay').click()
        ok(page.locator('#songPlayer').evaluate('(e)=>e.paused'),'original player pauses')
        shot('stage')
        if width==390:
            ok(page.locator('.rt-stage-sidecar').is_hidden(),'mobile sidecar hidden until requested')
            page.locator('.rt-u5-mobile-dock button').first.click()
            ok(page.locator('.rt-side-queue').is_visible(),'queue sheet opens')
            ok(page.locator('.rt-u5-mobile-dock button').first.get_attribute('aria-expanded')=='true','expanded semantics')
            ok(page.locator('.rt-side-mix').is_hidden(),'unselected mix sheet remains hidden')
            ok(page.locator('#rtMobileMute').is_visible(),'emergency mute remains visible')
            page.locator('.rt-side-queue .rt-u5-sheet-close').click()
            ok(page.locator('.rt-stage-sidecar').is_hidden(),'queue sheet closes')
            ok(page.locator('.rt-u5-mobile-dock button').first.evaluate('(e)=>e===document.activeElement'),'focus returns to trigger')
            page.locator('.rt-u5-mobile-dock button').nth(1).click()
            ok(page.locator('.rt-side-mix').is_visible(),'mix sheet uses real original controls')
            page.locator('#rtQuickMusic').fill('33')
            ok(page.locator('#musicVolume').input_value()=='33','mobile mix synchronized')
            page.locator('.rt-side-mix .rt-u5-sheet-close').click()
            ok(page.locator('.rt-stage-sidecar').is_hidden(),'mix sheet closes')
        else:
            ok(page.locator('.rt-stage-sidecar').is_visible(),'desktop/tablet sidecar shown')
        page.locator('#stageFocus').click()
        ok(page.locator('#stage').evaluate("(e)=>e.classList.contains('focus-mode')"),'focus uses original UI')
        shot('focus')
        page.locator('#stageFocus').click()
        ok(not page.locator('#stage').evaluate("(e)=>e.classList.contains('focus-mode')"),'focus can exit')
        page.locator('[data-rt-view="studio"]').click()
        ok(page.locator('#rtU2Mic').is_visible(),'studio remains functional')
        page.locator('#rtU2Mic').fill('120')
        ok(page.locator('#micGain').input_value()=='120','original mic slider receives change')
        ok(page.locator('#rtU3Effects').count()==1,'advanced controls preserved')
        shot('studio')
        page.locator('[data-rt-view="setup"]').click()
        ok(page.locator('#inputSelect').is_visible() and page.locator('#outputSelect').is_visible(),'setup selectors visible')
        ok(page.locator('#rtSetupContent .notice').is_visible(),'hardware caution intact')
        shot('setup')
        ok(page.evaluate('document.documentElement.scrollWidth<=innerWidth+2'),'no viewport overflow')
        ok(not errors,f'no page errors {errors}')
        ctx.close()
        print(f'PASS U5 {width}x{height}; cumulative {checks}',flush=True)
    browser.close()
shots=sorted(OUT.glob('u5-*.png'))
assert len(shots)==15
(OUT/'sha256.txt').write_text(''.join(f'{hashlib.sha256(f.read_bytes()).hexdigest()}  {f.name}\n' for f in shots))
print(f'PASS U5 {checks} assertions, 15 original PNGs hashed',flush=True)
