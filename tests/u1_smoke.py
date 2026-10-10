"""U1 UI regression smoke, uses Playwright Chromium. Install: pip install playwright.
Run: python tests/u1_smoke.py from repository root.
No microphone/speaker hardware passes are implied by these synthetic browser checks.
"""
from pathlib import Path
import math, wave, struct, io
from playwright.sync_api import sync_playwright
ROOT=Path(__file__).resolve().parent.parent
buf=io.BytesIO()
with wave.open(buf,'wb') as wav:
    wav.setnchannels(1);wav.setsampwidth(2);wav.setframerate(22050)
    wav.writeframes(b''.join(struct.pack('<h',int(2700*math.sin(2*math.pi*440*i/22050))) for i in range(22050*5)))
SAMPLE=buf.getvalue()
with sync_playwright() as p:
    browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox','--autoplay-policy=no-user-gesture-required'])
    for w,h in [(1440,900),(390,844),(768,1024)]:
        context=browser.new_context(viewport={'width':w,'height':h},accept_downloads=True)
        page=context.new_page(); errors=[]
        page.on('pageerror',lambda e:errors.append(str(e)))
        page.set_content(ROOT.joinpath('index.html').read_text(),wait_until='load');page.wait_for_timeout(250)
        assert page.locator('#rtStagePanel').is_visible(),f'Stage hidden {w}'
        assert page.locator('#rtSongsPanel').is_hidden()
        assert page.locator('#rtStudioPanel').is_hidden()
        assert page.locator('#rtSetupPanel').is_hidden()
        assert page.locator('#rtStagePanel #stage').count()==1
        assert page.locator('#rtStagePanel #rtQuickMusic').count()==1
        assert page.locator('#rtStagePanel #rtQueueMirror').count()==1
        assert page.locator('body').evaluate('(e)=>document.documentElement.scrollWidth<=innerWidth+2'),f'horizontal overflow {w}'
        page.screenshot(path=str(ROOT/f'u1-tested-{w}.png'),full_page=True)
        page.locator('[data-rt-view="setup"]').click()
        assert page.locator('#rtSetupPanel').is_visible()
        assert page.locator('#inputSelect').is_visible()
        assert page.locator('#exportDiagnostics').count()==1
        page.locator('[data-rt-view="studio"]').click()
        assert page.locator('#rtStudioPanel').is_visible()
        assert page.locator('#reverb').is_visible()
        assert page.locator('#recordBtn').count()==1
        page.locator('[data-rt-view="songs"]').click()
        assert page.locator('#rtSongsPanel').is_visible()
        assert page.locator('#songFile').count()==1
        assert page.locator('#lyricsStudio').is_visible()
        page.locator('[data-rt-view="stage"]').click()
        # controls must remain coupled with original ones
        page.locator('#rtQuickMusic').fill('42')
        assert page.locator('#musicVolume').input_value()=='42'
        assert page.locator('#rtQuickMusicValue').inner_text()=='42%'
        page.locator('#rtQuickMicGain').fill('115')
        assert page.locator('#micGain').input_value()=='115'
        page.select_option('#rtQuickPreset','studio')
        assert 'active' in (page.locator('[data-preset="studio"]').get_attribute('class') or '')
        page.locator('#rtAddSong').click()
        page.locator('#songFile').set_input_files({'name':'U1 sample.wav','mimeType':'audio/wav','buffer':SAMPLE})
        page.wait_for_timeout(450)
        assert page.locator('#trackQueue li').count()==1
        assert page.locator('#rtQueueMirror li').count()==1
        assert page.locator('#rtQueueCount').inner_text()=='01'
        assert page.locator('#stagePlay').is_enabled()
        # Use actual audio HTML element and reproduce fixed H2 Play/Pause handler.
        page.locator('#stagePlay').click()
        page.wait_for_timeout(150)
        playing=page.locator('#songPlayer').evaluate('(e)=>!e.paused')
        assert playing,'Player failed to start after Play click'
        page.locator('#stagePlay').click()
        assert page.locator('#songPlayer').evaluate('(e)=>e.paused'),'Playback did not pause'
        # Load matching timed lyrics in Songs, then exercise Stage line rendering.
        page.locator('[data-rt-view="songs"]').click()
        page.locator('#lrcFile').set_input_files({'name':'U1 sample.lrc','mimeType':'text/plain','buffer':b'[00:00.00]First verse\n[00:02.50]Second verse\n'})
        page.wait_for_timeout(200)
        page.locator('[data-rt-view="stage"]').click()
        assert 'First verse' in page.locator('#lyricCurrent').inner_text(),page.locator('#lyricCurrent').inner_text()
        # source control edits from Songs must be reflected in Stage
        page.locator('[data-rt-view="songs"]').click()
        page.locator('#musicVolume').fill('64')
        page.locator('[data-rt-view="stage"]').click()
        assert page.locator('#rtQuickMusic').input_value()=='64'
        # Lyrics studio: stage link must navigate to proper hidden panel
        page.locator('#jumpToStudio').click()
        assert page.locator('#lyricsStudio').is_visible()
        page.locator('[data-rt-view="stage"]').click()
        page.locator('#stageFocus').click()
        assert 'focus-mode' in page.locator('#stage').get_attribute('class')
        page.locator('#stageFocus').click()
        assert 'focus-mode' not in page.locator('#stage').get_attribute('class')
        if w<600:assert page.locator('#rtMobileMute').is_visible()
        else:assert page.locator('#rtMobileMute').is_hidden()
        # H1 diagnostic export still works after moving Setup nodes.
        page.locator('[data-rt-view="setup"]').click()
        page.locator('#hardwareQa > summary').click()
        with page.expect_download() as download_info:
            page.locator('#exportDiagnostics').click()
        download=download_info.value
        assert download.suggested_filename=='roomtone-device-report.json'
        assert not errors, f'Page errors at {w}: {errors}'
        print('PASS',w,'x',h,'stage/song/studio/setup navigation, queue, input mirrors, effects, H2 playback, focus, no overflow, no JS exceptions')
        context.close()
    browser.close()

