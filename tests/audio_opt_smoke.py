"""ROOMTONE A1 local UI qualification. Synthetic browser only, NOT real microphone H1 acceptance."""
from pathlib import Path
from playwright.sync_api import sync_playwright
import json,hashlib

root=Path(__file__).resolve().parent.parent
html=(root/'index.html').read_text()
assert html.count('src="audio-opt.js"')==1
assert html.count('href="audio-opt.css"')==1
assets={name:(root/name).read_text() for name in
        ('u2.css','u3.css','u4.css','audio-opt.css','u2.js','u3.js','u4.js','audio-opt.js')}
out=root/'audio-opt-evidence';out.mkdir(exist_ok=True)
checks=0
with sync_playwright() as pw:
 browser=pw.chromium.launch(executable_path='/usr/bin/chromium' if Path('/usr/bin/chromium').exists() else None,headless=True,args=['--no-sandbox'])
 for width,height in ((1440,900),(390,844)):
  context=browser.new_context(viewport={'width':width,'height':height},accept_downloads=True)
  page=context.new_page(); errors=[]
  page.on('pageerror',lambda err:errors.append(str(err)))
  page.evaluate("""()=>{const store=new Map();Object.defineProperty(window,'localStorage',{configurable:true,value:{
    getItem:k=>store.get(k)||null,setItem:(k,v)=>store.set(k,String(v)),removeItem:k=>store.delete(k)
  }})}""")
  page.set_content(html,wait_until='load')
  for name in ('u2.css','u3.css','u4.css','audio-opt.css'):page.add_style_tag(content=assets[name])
  for name in ('u2.js','u3.js','u4.js','audio-opt.js'):page.add_script_tag(content=assets[name])
  def ok(condition,label):
   global checks
   assert condition,str(width)+': '+label
   checks+=1
  page.locator('[data-rt-view="setup"]').click()
  ok(page.locator('#rtAoGuide').is_visible(),'calibration shown in Setup')
  ok(page.locator('#rtAoLatency').input_value()=='not-tested','no invented hardware measurement')
  ok(page.locator('#monitorBtn').inner_text().startswith('Enable'),'monitor muted at start')
  page.locator('#rtAoNatural').click()
  ok(page.locator('#reverb').input_value()=='17' and page.locator('#echo').input_value()=='0','natural short-room sound')
  page.locator('#rtAoHandheld').click()
  ok(page.locator('#reverb').input_value()=='14' and page.locator('#micGain').input_value()=='100','handheld starting sound')
  page.locator('#rtAoSave').click()
  page.locator('#rtAoNatural').click()
  page.locator('#rtAoLoad').click()
  ok(page.locator('#reverb').input_value()=='14','microphone sound restored')
  ok(page.locator('#monitorBtn').inner_text().startswith('Enable'),'loading profile never unmutes')
  # A visibly-live monitor blocks any profile application.
  page.locator('#monitorBtn').evaluate("e=>{e.textContent='Mute voice in speakers'}")
  page.locator('#rtAoNatural').click()
  ok(page.locator('#reverb').input_value()=='14','live-level guard refuses profile')
  page.locator('#monitorBtn').evaluate("e=>{e.textContent='Enable voice in speakers'}")
  page.locator('#inputSelect').evaluate("""e=>{e.add(new Option('Synthetic second mic','fake-mic'));e.value='fake-mic';e.dispatchEvent(new Event('change',{bubbles:true}))}""")
  ok(page.locator('#rtAoLatency').input_value()=='not-tested','device change resets observations')
  page.locator('#rtAoLoad').click()
  ok('No sound profile' in page.locator('#rtAoStatus').inner_text(),'profiles isolated per device')
  page.locator('#rtAoOpenLab').click()
  ok(page.locator('#rtU3Lab').get_attribute('open') is not None,'calibration tools revealed')
  page.locator('#rtAoLatency').select_option('comfortable')
  page.locator('[data-ao-check="raw"]').check()
  with page.expect_download() as d:page.locator('#rtAoReport').click()
  report=json.loads(Path(d.value.path()).read_text())
  ok(report['kind']=='user-observations-not-automated-measurement','reported not measured')
  ok(report['hardwareQualifiedByApplication'] is False,'never certifies hardware')
  ok(report['allChecksUserMarked'] is False,'incomplete H1 acknowledged')
  ok(report['checks']['raw'] and not report['checks']['record'],'manual detail exact')
  ok(report['perceivedDelay']=='comfortable','qualitative latency field')
  ok(page.evaluate('document.documentElement.scrollWidth<=innerWidth+2'),'no horizontal overflow')
  ok(not errors,'no script errors: '+str(errors))
  page.evaluate("window.scrollTo({top:0,behavior:'instant'})")
  page.screenshot(path=str(out/('setup-'+str(width)+'.png')),full_page=True)
  print('PASS A1',width,'x',height,'cumulative assertions',checks,flush=True)
  context.close()
 browser.close()
shots=sorted(out.glob('setup-*.png'))
assert len(shots)==2
(out/'sha256.txt').write_text(''.join(hashlib.sha256(f.read_bytes()).hexdigest()+'  '+f.name+'\n' for f in shots))
print('PASS A1',checks,'synthetic assertions; 2 original screenshots hashed; H1 physical tests outstanding')
