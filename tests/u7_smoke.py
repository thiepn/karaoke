"""U7 original screenshot-grounded mobile dock regression; synthetic files only.
No hardware, production origin, mic permission, recording or human acceptance.
"""
from pathlib import Path
from playwright.sync_api import sync_playwright
import hashlib,io,math,struct,wave

ROOT=Path(__file__).resolve().parent.parent
HTML=(ROOT/'index.html').read_text()
for n in ('u2','u3','u4','u5','u6','u7'):
    assert HTML.count(f'href="{n}.css"')==1 and HTML.count(f'src="{n}.js"')==1,n
ASSETS={f'{n}.{ext}':(ROOT/f'{n}.{ext}').read_text()
        for n in ('u2','u3','u4','u5','u6','u7') for ext in ('css','js')}
OUT=ROOT/'u7-evidence'
OUT.mkdir(exist_ok=True)
wav_data=io.BytesIO()
with wave.open(wav_data,'wb') as wav:
    wav.setnchannels(1);wav.setsampwidth(2);wav.setframerate(22050)
    wav.writeframes(b''.join(struct.pack('<h',int(550*math.sin(2*math.pi*275*i/22050)))
                             for i in range(22050*3)))
AUDIO=wav_data.getvalue()
checks=0
with sync_playwright() as p:
    browser=p.chromium.launch(executable_path='/usr/bin/chromium' if Path('/usr/bin/chromium').exists() else None,
        headless=True,args=['--no-sandbox','--autoplay-policy=no-user-gesture-required'])
    for width,height in ((1440,900),(768,1024),(390,844)):
        context=browser.new_context(viewport={'width':width,'height':height},
            reduced_motion='reduce' if width==390 else 'no-preference')
        page=context.new_page()
        errors=[]
        page.on('pageerror',lambda err:errors.append(str(err)))
        page.evaluate("""()=>{const cache=new Map();Object.defineProperty(window,'localStorage',
          {configurable:true,value:{getItem:k=>cache.get(k)||null,setItem:(k,v)=>cache.set(k,String(v)),
          removeItem:k=>cache.delete(k)}})}""")
        page.set_content(HTML,wait_until='load')
        for name in ('u2','u3','u4','u5','u6','u7'):
            page.add_style_tag(content=ASSETS[f'{name}.css'])
        for name in ('u2','u3','u4','u5','u6','u7'):
            page.add_script_tag(content=ASSETS[f'{name}.js'])
        page.wait_for_timeout(120)
        def ok(value,message):
            global checks
            assert value,f'{width}px: {message}'
            checks+=1
        def shot(name):
            page.evaluate("document.activeElement?.blur();window.scrollTo({top:0,behavior:'instant'})")
            page.wait_for_timeout(90)
            page.screenshot(path=str(OUT/f'u7-{name}-{width}.png'),full_page=True)
        ok(page.locator('.rt-u7-stage-dock').count()==1,'same original U5 dock repositioned once')
        ok(page.locator('.rt-u7-stage-dock button').count()==2,'original queue and mixer controls preserved')
        ok(page.locator('#rtMobileMute').count()==1,'global emergency mute remains one')
        ok(page.locator('#stagePanic').count()==1,'original inline safety mute preserved')
        ok(page.locator('.rt-u7-stage-dock').get_attribute('data-rt-u7-layout')=='in-flow','UI correction marker')
        ok(page.locator('.rt-u7-stage-dock').evaluate('(e)=>e.previousElementSibling.classList.contains("rt-view-title")'),'dock placed after heading before stage')
        if width==390:
            ok(page.locator('.rt-u7-stage-dock').is_visible(),'mobile stage dock visible')
            ok(page.locator('.rt-u7-stage-dock').evaluate('(e)=>getComputedStyle(e).position==="sticky"'),'dock is sticky, not fixed')
            ok(page.evaluate("""()=>{
                const dock=document.querySelector('.rt-u7-stage-dock').getBoundingClientRect();
                const stage=document.querySelector('.rt-stage-layout').getBoundingClientRect();
                return dock.bottom<=stage.top+1 && dock.top>=0 && dock.bottom<innerHeight;
            }"""),'queue/mixer dock occupies dedicated flow space before stage')
            ok(page.locator('#rtMobileMute').is_visible(),'emergency mute still globally visible')
        else:
            ok(page.locator('.rt-u7-stage-dock').is_hidden(),'desktop retains sidecar without mobile dock')

        page.locator('#songFile').set_input_files({'name':'U7 Witness Sample.wav','mimeType':'audio/wav','buffer':AUDIO})
        page.wait_for_timeout(160)
        ok(page.locator('#trackQueue .queue-item').count()==1,'local track is in original queue')
        ok(page.locator('#stagePlay').is_enabled(),'original player ready')
        shot('stage')
        if width==390:
            page.locator('.rt-u7-stage-dock button').first.click()
            ok(page.locator('.rt-side-queue').is_visible(),'original queue sheet works after dock relocation')
            ok(page.locator('#rtMobileMute').is_visible(),'emergency mute visible with sheet')
            shot('queue')
            page.keyboard.press('Escape')
            page.wait_for_timeout(90)
            ok(page.locator('.rt-stage-sidecar').is_hidden(),'Escape closes sheet')
            ok(page.locator('.rt-u7-stage-dock button').first.evaluate('(e)=>document.activeElement===e'),'Escape restores original queue focus')
            page.locator('.rt-u7-stage-dock button').nth(1).click()
            ok(page.locator('.rt-side-mix').is_visible(),'original mixer sheet opens')
            page.locator('#rtQuickMusic').fill('36')
            ok(page.locator('#musicVolume').input_value()=='36','backing volume delegates to original input')
            page.locator('.rt-side-mix .rt-u5-sheet-close').click()
        page.locator('#stageFocus').click()
        ok(page.locator('#stage').evaluate('(e)=>e.classList.contains("focus-mode")'),'native focus mode still works')
        if width==390:
            ok(page.locator('.rt-u7-stage-dock').is_hidden(),'dock absent from fullscreen performance')
            ok(page.locator('#rtMobileMute').is_visible(),'panic remains visible in focus')
        shot('focus')
        page.locator('#stageFocus').click()
        page.locator('[data-rt-view="songs"]').click()
        ok(page.locator('#rtSongsContent').is_visible(),'songs accessible')
        ok(page.locator('#rtU6FilterEmpty').count()==1,'prior U6 filter fix preserved')
        if width==390:ok(page.locator('#rtMobileMute').is_visible(),'global panic available in Songs')
        shot('songs')
        page.locator('[data-rt-view="studio"]').click()
        ok(page.locator('#rtU2Mic').is_visible(),'original studio mixer present')
        if width==390:ok(page.locator('#rtMobileMute').is_visible(),'global panic available in Studio')
        shot('studio')
        page.locator('[data-rt-view="setup"]').click()
        ok(page.locator('#rtSetupContent .notice').is_visible(),'original hardware warning retained')
        if width==390:ok(page.locator('#rtMobileMute').is_visible(),'global panic available in Setup')
        shot('setup')
        ok(page.evaluate('document.documentElement.scrollWidth<=innerWidth+2'),'no horizontal overflow')
        ok(not errors,'no JavaScript exceptions: '+repr(errors))
        print(f'U7 PASS viewport {width}x{height}, cumulative {checks}',flush=True)
        context.close()
    browser.close()
images=sorted(OUT.glob('u7-*.png'))
assert len(images)==16,f'expected 16 original PNGs, found {len(images)}'
(OUT/'sha256.txt').write_text(''.join(f'{hashlib.sha256(p.read_bytes()).hexdigest()}  {p.name}\n' for p in images))
print(f'U7 PASS {checks} assertions; 16 original screenshot hashes recorded',flush=True)
