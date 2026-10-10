"""ROOMTONE U2 responsive browser smoke. No physical microphone/speaker acceptance implied."""
from pathlib import Path
import io, math, struct, wave, shutil
from playwright.sync_api import sync_playwright

ROOT=Path(__file__).resolve().parent.parent
def sample():
    data=io.BytesIO()
    with wave.open(data,'wb') as wav:
        wav.setnchannels(1);wav.setsampwidth(2);wav.setframerate(22050)
        wav.writeframes(b''.join(struct.pack('<h',int(1000*math.sin(2*math.pi*440*i/22050))) for i in range(22050*4)))
    return data.getvalue()

def main():
    html=(ROOT/'index.html').read_text()
    css=(ROOT/'u2.css').read_text()
    js=(ROOT/'u2.js').read_text()
    assert html.count('<link rel="stylesheet" href="u2.css">')==1
    assert html.count('<script defer src="u2.js"></script>')==1
    checks=0
    with sync_playwright() as p:
        binary=shutil.which('chromium') or shutil.which('google-chrome')
        launch={'executable_path':binary} if binary else {}
        browser=p.chromium.launch(headless=True,args=['--no-sandbox','--autoplay-policy=no-user-gesture-required'],**launch)
        for width,height in ((1440,900),(768,1024),(390,844)):
            context=browser.new_context(viewport={'width':width,'height':height},accept_downloads=True)
            page=context.new_page();errors=[]
            page.on('pageerror',lambda e:errors.append(str(e)))
            # set_content runs against about:blank; localStorage needs a shim here only.
            page.evaluate("""()=>{const values=new Map();Object.defineProperty(window,'localStorage',
               {configurable:true,value:{setItem(k,v){values.set(k,String(v))},
                 getItem(k){return values.get(k)??null},removeItem(k){values.delete(k)}}})}""")
            page.set_content(html,wait_until='load')
            page.add_style_tag(content=css);page.add_script_tag(content=js)
            page.locator('[data-rt-view="songs"]').click()
            assert page.locator('#rtSongSearch').is_visible();checks+=1
            assert page.locator('#rtU2SongCount').inner_text()=='0 songs';checks+=1
            assert page.locator('#songFile').count()==1 and page.locator('#lrcFile').count()==1;checks+=1
            page.locator('#songFile').set_input_files({'name':'Smoke.wav','mimeType':'audio/wav','buffer':sample()})
            page.wait_for_timeout(400)
            assert page.locator('#rtU2SongCount').inner_text()=='1 song';checks+=1
            page.locator('#rtSongSearch').fill('nonmatching')
            assert page.locator('#rtU2SongList .rt-u2-song-row').count()==0;checks+=1
            page.locator('#rtSongSearch').fill('smoke')
            assert page.locator('#rtU2SongList .rt-u2-song-row').count()==1;checks+=1
            page.locator('#rtU2SongList .rt-u2-song-select').click()
            assert page.locator('#stageTitle').inner_text().strip()!='No song loaded';checks+=1
            page.locator('#lrcFile').set_input_files({'name':'Smoke.lrc','mimeType':'text/plain','buffer':b'[00:00.00]First line\n[00:02.00]Second line\n'})
            assert page.locator('#lyricsStudio').count()==1;checks+=1
            page.locator('[data-rt-view="studio"]').click()
            assert page.locator('#rtU2Mic').is_visible() and page.locator('#recordBtn').count()==1;checks+=1
            page.locator('#rtU2Mic').fill('130')
            assert page.locator('#micGain').input_value()=='130';checks+=1
            page.locator('#rtU2Music').fill('47')
            assert page.locator('#musicVolume').input_value()=='47';checks+=1
            page.locator('[data-rt-u2-preset="studio"]').click()
            assert page.locator('[data-rt-u2-preset="studio"]').get_attribute('aria-pressed')=='true';checks+=1
            page.locator('#saveSound').click();page.locator('#rtU2Mic').fill('175');page.locator('#loadSound').click()
            assert page.locator('#rtU2Mic').input_value()=='130';checks+=1
            page.locator('#resetSound').click()
            assert page.locator('#rtU2Mic').input_value()=='100';checks+=1
            page.locator('[data-rt-view="setup"]').click()
            assert page.locator('#rtU2StartMic').is_enabled();checks+=1
            page.locator('#rtU2GotoQa').click()
            assert page.locator('#hardwareQa').get_attribute('open') is not None;checks+=1
            page.locator('#hardwareQa input[data-qa="input"]').check()
            page.wait_for_timeout(100)
            assert '1 of 5' in page.locator('#rtU2QaStatus').inner_text();checks+=1
            page.locator('[data-rt-view="stage"]').click()
            page.locator('#stagePlay').click();page.wait_for_timeout(140)
            assert page.locator('#songPlayer').evaluate('(e)=>!e.paused');checks+=1
            page.locator('#stagePlay').click()
            assert page.locator('#songPlayer').evaluate('(e)=>e.paused');checks+=1
            assert page.evaluate('document.documentElement.scrollWidth<=innerWidth+2');checks+=1
            assert not errors,errors;checks+=1
            print('PASS',width,'x',height,'Songs Studio Setup playback no overflow no pageerrors')
            context.close()
        browser.close()
    print('TOTAL',checks,'browser assertions across 3 viewports')

if __name__=='__main__':main()
