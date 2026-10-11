"""ROOMTONE U14: fullscreen keyboard containment in synthetic Chromium only.

Positive acceptance gate: all sequential Tab/Shift+Tab focus remains in
fullscreen Stage, with no microphone, production, or physical owner test.
"""
from pathlib import Path
import hashlib
from playwright.sync_api import sync_playwright

ROOT=Path(__file__).resolve().parent.parent
HTML=(ROOT/'index.html').read_text(encoding='utf-8')
ASSETS={f'{n}.css':(ROOT/f'{n}.css').read_text()
        for n in ('u2','u3','u4','u5','u6','u7','u11','u12','u13')}
SCRIPTS={f'{n}.js':(ROOT/f'{n}.js').read_text()
         for n in ('u2','u3','u4','u5','u6','u7')}
if (ROOT/'u14.js').exists():
    SCRIPTS['u14.js']=(ROOT/'u14.js').read_text()
OUT=ROOT/'u14-evidence'
OUT.mkdir(exist_ok=True)
assert not list(OUT.glob('u14-*.png')), 'Stale U14 screenshot evidence'
checks=0
with sync_playwright() as pw:
    browser=pw.chromium.launch(executable_path='/usr/bin/chromium' if Path('/usr/bin/chromium').exists() else None,
                               headless=True,args=['--no-sandbox'])
    for width,height in ((1440,900),(768,1024),(390,844),(350,740)):
        c=browser.new_context(viewport={'width':width,'height':height},
                              reduced_motion='reduce' if width<=390 else 'no-preference')
        p=c.new_page()
        errors=[]
        p.on('pageerror',lambda error:errors.append(str(error)))
        p.evaluate("""()=>{const cache=new Map();Object.defineProperty(window,'localStorage',{
          configurable:true,value:{getItem:k=>cache.get(k)||null,setItem:(k,v)=>cache.set(k,String(v)),
          removeItem:k=>cache.delete(k),clear:()=>cache.clear()}})}""")
        p.set_content(HTML,wait_until='load')
        for n in ('u2','u3','u4','u5','u6','u7','u11','u12','u13'):
            p.add_style_tag(content=ASSETS[f'{n}.css'])
        for n in ('u2','u3','u4','u5','u6','u7'):
            p.add_script_tag(content=SCRIPTS[f'{n}.js'])
        if 'u14.js' in SCRIPTS:p.add_script_tag(content=SCRIPTS['u14.js'])
        p.wait_for_timeout(120)
        def ok(value,label):
            global checks
            assert value, f'{width}px: {label}'
            checks+=1
        p.screenshot(path=str(OUT/f'u14-stage-viewport-{width}.png'),full_page=False)
        p.locator('#stageFocus').click()
        p.wait_for_timeout(60)
        ok(p.locator('#stage').evaluate("e=>e.classList.contains('focus-mode')"),
           'original fullscreen opened')
        ok(p.locator('#stageFocus').get_attribute('aria-pressed')=='true',
           'original focus toggle unchanged')
        for direction in ('Tab','Shift+Tab'):
            p.locator('#stageFocus').focus()
            for i in range(45):
                p.keyboard.press(direction)
                state=p.evaluate("""()=>({
                    inside:!!document.activeElement?.closest('#stage'),
                    tag:document.activeElement?.tagName,
                    id:document.activeElement?.id,
                    stage:document.querySelector('#stage').classList.contains('focus-mode')
                })""")
                ok(state['inside'],f'{direction} #{i} escaped Stage to {state}')
        if 'u14.js' in SCRIPTS:
            ok(p.locator('#stage').get_attribute('role')=='dialog',
               'fullscreen semantics announced as dialog')
            ok(p.locator('#stage').get_attribute('aria-modal')=='true',
               'fullscreen semantics are modal only when focus is active')
        p.screenshot(path=str(OUT/f'u14-focus-viewport-{width}.png'),full_page=False)
        p.locator('#stageFocus').focus()
        p.keyboard.press('Escape')
        ok(not p.locator('#stage').evaluate("e=>e.classList.contains('focus-mode')"),
           'original Escape leaves fullscreen')
        if 'u14.js' in SCRIPTS:
            ok(p.locator('#stage').get_attribute('role') is None,
               'original Stage semantics restored on exit')
            ok(p.locator('#stage').get_attribute('aria-modal') is None,
               'aria-modal restored on exit')
        ok(not errors,'no page errors '+repr(errors))
        print(f'U14 PASS synthetic Chromium {width}x{height} cumulative={checks}',flush=True)
        c.close()
    browser.close()
images=sorted(OUT.glob('u14-*.png'))
assert len(images)==8, f'Expected 8 U14 viewport PNGs, got {len(images)}'
(OUT/'sha256.txt').write_text(''.join(f'{hashlib.sha256(path.read_bytes()).hexdigest()}  {path.name}\n' for path in images))
print(f'U14 PASS {checks} keyboard containment and recovery assertions; 8 viewport PNGs with SHA256',flush=True)
