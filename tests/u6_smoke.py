"""U6 UI-only regression. Synthetic audio/LRC files; no microphone or network use."""
from pathlib import Path
from playwright.sync_api import sync_playwright
import hashlib, io, math, struct, wave

ROOT=Path(__file__).resolve().parent.parent
HTML=(ROOT/'index.html').read_text()
for name in ('u2','u3','u4','u5','u6'):
    assert HTML.count(f'href="{name}.css"')==1, name
    assert HTML.count(f'src="{name}.js"')==1, name
ASSETS={f'{n}.{suffix}':(ROOT/f'{n}.{suffix}').read_text()
        for n in ('u2','u3','u4','u5','u6') for suffix in ('css','js')}
EVIDENCE=ROOT/'u6-evidence'
EVIDENCE.mkdir(exist_ok=True)
buf=io.BytesIO()
with wave.open(buf,'wb') as wav:
    wav.setnchannels(1);wav.setsampwidth(2);wav.setframerate(22050)
    wav.writeframes(b''.join(struct.pack('<h',int(650*math.sin(2*math.pi*300*i/22050)))
                             for i in range(22050*4)))
AUDIO=buf.getvalue()
assertions=0
with sync_playwright() as p:
    browser=p.chromium.launch(executable_path='/usr/bin/chromium' if Path('/usr/bin/chromium').exists() else None,
        headless=True,args=['--no-sandbox','--autoplay-policy=no-user-gesture-required'])
    for width,height in ((1440,900),(768,1024),(390,844)):
        context=browser.new_context(viewport={'width':width,'height':height},reduced_motion='reduce' if width==390 else 'no-preference')
        page=context.new_page()
        errors=[]
        page.on('pageerror',lambda e:errors.append(str(e)))
        page.evaluate("""()=>{const map=new Map();Object.defineProperty(window,'localStorage',{configurable:true,
          value:{getItem:key=>map.get(key)||null,setItem:(key,value)=>map.set(key,String(value)),removeItem:key=>map.delete(key)}})}""")
        page.set_content(HTML,wait_until='load')
        for name in ('u2','u3','u4','u5','u6'):
            page.add_style_tag(content=ASSETS[f'{name}.css'])
        for name in ('u2','u3','u4','u5','u6'):
            page.add_script_tag(content=ASSETS[f'{name}.js'])
        page.wait_for_timeout(100)
        def check(value,message):
            global assertions
            assert value,f'{width}px: {message}'
            assertions+=1
        def screenshot(name):
            page.evaluate("document.activeElement?.blur();window.scrollTo({top:0,behavior:'instant'})")
            page.wait_for_timeout(90)
            page.screenshot(path=str(EVIDENCE/f'u6-{name}-{width}.png'),full_page=True)
        check(page.locator('#rtU6FilterEmpty').count()==1,'single U6 no-results region')
        check(page.locator('#stagePanic').count()==1 and page.locator('#rtMobileMute').count()==1,'original emergency controls exist')
        page.locator('[data-rt-view="songs"]').click()
        page.locator('#songFile').set_input_files([
            {'name':'First U6.wav','mimeType':'audio/wav','buffer':AUDIO},
            {'name':'Second U6.wav','mimeType':'audio/wav','buffer':AUDIO}])
        page.wait_for_timeout(420)
        check(page.locator('.rt-u2-song-row').count()==2,'real local songs imported')
        check(page.locator('.rt-u2-song-select').first.inner_text()=='First U6','first ordinal prefix removed')
        check(page.locator('.rt-u2-song-select').nth(1).inner_text()=='Second U6','second ordinal prefix removed')
        check(page.locator('#trackQueue .queue-play').nth(1).inner_text().startswith('2. Second U6'),'original queue text unchanged')
        check(page.locator('#rtU2SongStatus').inner_text().startswith('2 songs shown'),'initial legacy status truthful')
        page.locator('[data-rt-u5-filter="ready"]').click()
        check(page.locator('.rt-u2-song-row:visible').count()==0,'zero ready when no LRC')
        check(page.locator('#rtU6FilterEmpty').is_visible(),'filtered empty explanation visible')
        check('0 songs shown' in page.locator('#rtU2SongStatus').inner_text(),'legacy status aligned on empty result')
        check(page.locator('#rtU5FilteredCount').inner_text()=='0 songs shown','filter count aligned on empty result')
        page.locator('[data-rt-u5-filter="missing"]').click()
        check(page.locator('.rt-u2-song-row:visible').count()==2,'missing lyrics filter shows both')
        check(page.locator('#rtU6FilterEmpty').is_hidden(),'nonempty results hide extra hint')
        page.locator('[data-rt-u5-filter="all"]').click()
        page.locator('#rtSongSearch').fill('second')
        check(page.locator('.rt-u2-song-row:visible').count()==1,'search still works')
        check(page.locator('#rtU2SongStatus').inner_text().startswith('1 song shown'),'status responds to search')
        page.locator('#rtSongSearch').fill('')
        page.locator('.rt-u2-song-select').nth(1).click()
        page.locator('#lrcFile').set_input_files({'name':'Second U6.lrc','mimeType':'text/plain',
            'buffer':b'[00:00.00]The first U6 line\n[00:01.00]A second U6 line\n'})
        page.wait_for_timeout(150)
        page.locator('[data-rt-u5-filter="ready"]').click()
        check(page.locator('.rt-u2-song-row:visible').count()==1,'LRC ready result')
        check(page.locator('.rt-u2-song-row:visible .rt-u2-song-select').first.inner_text()=='Second U6','ready title clean')
        check(page.locator('#rtU2SongStatus').inner_text().startswith('1 song shown'),'status corrected after LRC')
        check(page.locator('#rtU5FilteredCount').inner_text()=='1 song shown','count consistent after LRC')
        check(page.locator('#rtU6FilterEmpty').is_hidden(),'ready result not empty')
        check(page.locator('.rt-u2-song-row:visible .rt-u2-song-select').first.get_attribute('aria-current')=='true','current semantics preserved')
        screenshot('songs')
        page.locator('[data-rt-view="stage"]').click()
        check('The first U6 line' in page.locator('#lyricCurrent').inner_text(),'synchronized lyric remains active')
        check(page.locator('#rtU5DockQueueCount').inner_text()=='02','queue count untouched')
        page.locator('#stagePlay').click()
        page.wait_for_timeout(110)
        check(page.locator('#songPlayer').evaluate('(e)=>!e.paused'),'original playback works')
        page.locator('#stagePlay').click()
        screenshot('stage')
        if width==390:
            check(page.locator('.rt-stage-sidecar').is_hidden(),'mobile panel collapsed')
            page.locator('.rt-u5-mobile-dock button').first.click()
            check(page.locator('.rt-side-queue').is_visible(),'queue sheet opens')
            check(page.locator('#rtMobileMute').is_visible(),'panic remains visible with sheet')
            screenshot('sheet')
            page.keyboard.press('Escape')
            page.wait_for_timeout(90)
            check(page.locator('.rt-stage-sidecar').is_hidden(),'Escape also closes sheet')
            check(page.locator('.rt-u5-mobile-dock button').first.evaluate('(e)=>e===document.activeElement'),'Escape restores trigger focus')
            page.locator('.rt-u5-mobile-dock button').nth(1).click()
            check(page.locator('.rt-side-mix').is_visible(),'original mixer sheet opens')
            page.locator('#rtQuickMusic').fill('35')
            check(page.locator('#musicVolume').input_value()=='35','real mixer responds')
            page.locator('.rt-side-mix .rt-u5-sheet-close').click()
            check(page.evaluate("""()=>{const a=document.querySelector('.rt-u5-mobile-dock').getBoundingClientRect();
              const b=document.querySelector('#rtMobileMute').getBoundingClientRect();return a.right+3<=b.left;}"""),'fixed mobile dock does not overlap mute')
        page.locator('#stageFocus').click()
        check(page.locator('#stage').evaluate("(e)=>e.classList.contains('focus-mode')"),'immersive stage active')
        screenshot('focus')
        page.locator('#stageFocus').click()
        page.locator('[data-rt-view="studio"]').click()
        check(page.locator('#rtU2Mic').is_visible(),'original studio present')
        check(page.locator('#rtU3Effects').count()==1,'advanced effects preserved')
        screenshot('studio')
        page.locator('[data-rt-view="setup"]').click()
        check(page.locator('#inputSelect').is_visible() and page.locator('#outputSelect').is_visible(),'setup retained')
        check(page.locator('#rtSetupContent .notice').is_visible(),'hardware safety notice retained')
        screenshot('setup')
        check(page.evaluate('document.documentElement.scrollWidth <= innerWidth + 2'),'no horizontal overflow')
        check(not errors,'no uncaught JS errors: '+repr(errors))
        context.close()
        print(f'PASS U6 {width}x{height}; cumulative {assertions}',flush=True)
    browser.close()
shots=sorted(EVIDENCE.glob('u6-*.png'))
assert len(shots)==16,f'Expected 16 original screenshots, received {len(shots)}'
(EVIDENCE/'sha256.txt').write_text(''.join(
    f'{hashlib.sha256(path.read_bytes()).hexdigest()}  {path.name}\n' for path in shots))
print(f'PASS U6 {assertions} assertions, {len(shots)} original PNGs SHA-256 hashed',flush=True)
