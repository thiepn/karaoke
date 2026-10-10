/* ROOMTONE U2 — workspace enhancement. No audio nodes, saved-key schemas or DSP routines altered. */
(()=>{
  'use strict';
  const $=id=>document.getElementById(id);
  const make=(tag,className,text)=>{const e=document.createElement(tag);if(className)e.className=className;if(text!==undefined)e.textContent=text;return e};
  const button=(label,cls,handler)=>{const b=make('button',`rt-u2-action ${cls||''}`,label);b.type='button';b.addEventListener('click',handler);return b};
  const invoke=(id)=>$(id)?.click();
  const songs=$('rtSongsContent'),studio=$('rtStudioContent'),setup=$('rtSetupContent');
  if(!songs||!studio||!setup||!$('trackQueue'))return;

  // SONGS — discovery list mirrors actual queue controls instead of maintaining its own track store.
  const songConsole=make('section','rt-u2-console');songConsole.setAttribute('aria-label','Song browser');
  songConsole.innerHTML=`<div class="rt-u2-head"><div><p class="rt-u2-eyebrow">01 / LOCAL MUSIC</p><h2>Find your next song.</h2></div><p>Build your queue, search by title and attach synchronized lyrics. Audio stays on this device.</p></div><div class="rt-u2-toolbar"><label class="rt-u2-search-wrap" for="rtSongSearch"><span aria-hidden="true">⌕</span><input type="search" id="rtSongSearch" placeholder="Filter your setlist" autocomplete="off"></label><button type="button" class="rt-u2-action primary" id="rtU2AddTracks">+ Audio files</button><button type="button" class="rt-u2-action" id="rtU2AddLyrics">+ LRC lyrics</button></div><div class="rt-u2-strip"><strong id="rtU2SongCount">0 songs</strong><span id="rtU2SongNow">Nothing playing yet</span></div><ol id="rtU2SongList" class="rt-u2-song-list" aria-label="Searchable song queue"></ol><p class="rt-u2-status" id="rtU2SongStatus" role="status" aria-live="polite">Import an audio file to begin.</p>`;
  songs.prepend(songConsole);
  $('rtU2AddTracks').addEventListener('click',()=>invoke('songFile'));
  $('rtU2AddLyrics').addEventListener('click',()=>invoke('lrcFile'));
  const search=$('rtSongSearch'),sourceQueue=$('trackQueue'),list=$('rtU2SongList');
  function renderSongs(){
    const items=Array.from(sourceQueue.querySelectorAll('.queue-item'));
    const phrase=search.value.trim().toLocaleLowerCase();
    $('rtU2SongCount').textContent=`${items.length} ${items.length===1?'song':'songs'}`;
    const current=items.find(li=>li.classList.contains('current'));
    $('rtU2SongNow').textContent=current?.querySelector('.queue-play')?.textContent?.trim()||'Nothing playing yet';
    list.replaceChildren();let shown=0;
    items.forEach((li,i)=>{
      const original=li.querySelector('.queue-play'); const name=original?.textContent?.trim()||`Song ${i+1}`;
      if(phrase&&!name.toLocaleLowerCase().includes(phrase))return;
      shown++;
      const row=make('li',`rt-u2-song-row${li.classList.contains('current')?' active':''}`);
      const number=make('span','rt-u2-song-number',String(i+1).padStart(2,'0'));
      const select=make('button','rt-u2-song-select',name);select.type='button';select.disabled=!original||original.disabled;
      select.setAttribute('aria-label',`Play ${name}`);select.addEventListener('click',()=>original?.click());
      row.append(number,select);
      if(li.classList.contains('current'))row.append(make('span','rt-u2-song-state','Current'));
      list.append(row);
    });
    if(!shown)list.append(make('li','rt-u2-empty',items.length?'No songs match your search. Clear the filter to see everything.':'Your setlist is empty. Add audio files to get started.'));
    $('rtU2SongStatus').textContent=`${shown} ${shown===1?'song':'songs'} shown · original queue order preserved`;
  }
  search.addEventListener('input',renderSongs);
  new MutationObserver(renderSongs).observe(sourceQueue,{childList:true,subtree:true,attributes:true,attributeFilter:['class','disabled']});
  renderSongs();

  // STUDIO — control surfaces delegate to the existing input handlers and preset buttons.
  const studioConsole=make('section','rt-u2-console');studioConsole.setAttribute('aria-label','Live mixing desk');
  studioConsole.innerHTML=`<div class="rt-u2-head"><div><p class="rt-u2-eyebrow">02 / LIVE MIX</p><h2>Shape your sound.</h2></div><p>A simple starting point for a balanced vocal. Full reverb, echo and recording controls remain below.</p></div><div class="rt-u2-presets" id="rtU2Presets" role="group" aria-label="Vocal presets"></div><div class="rt-u2-faders"><div class="rt-u2-fader"><label for="rtU2Mic">Microphone gain <output id="rtU2MicOut">100%</output></label><input id="rtU2Mic" type="range" min="20" max="220" step="5" value="100"></div><div class="rt-u2-fader"><label for="rtU2Music">Backing volume <output id="rtU2MusicOut">70%</output></label><input id="rtU2Music" type="range" min="0" max="100" step="1" value="70"></div></div><div class="rt-u2-strip"><strong id="rtU2MicState">Mic off</strong><span id="rtU2TakeState">Recording ready when the microphone starts</span><button type="button" class="rt-u2-action danger" id="rtU2Mute">Mute speaker monitoring · Esc</button></div>`;
  studio.prepend(studioConsole);
  function link(mirror,original,out){
    const m=$(mirror),s=$(original),o=$(out);if(!s)return;
    const sync=()=>{m.value=s.value;o.textContent=`${s.value}%`};
    m.addEventListener('input',()=>{s.value=m.value;s.dispatchEvent(new Event('input',{bubbles:true}))});
    s.addEventListener('input',sync);s.addEventListener('change',sync);sync();
    // Original save/load/reset writes settings directly. Synchronize those paths.
    for(const id of ['loadSound','resetSound','saveSound'])$(id)?.addEventListener('click',()=>queueMicrotask(sync));
  }
  link('rtU2Mic','micGain','rtU2MicOut');link('rtU2Music','musicVolume','rtU2MusicOut');
  const options=['clean','studio','karaoke','hall'];
  for(const name of options){
    const p=make('button','rt-u2-preset',name[0].toUpperCase()+name.slice(1));p.type='button';p.dataset.rtU2Preset=name;p.setAttribute('aria-pressed','false');
    p.addEventListener('click',()=>document.querySelector(`[data-preset="${name}"]`)?.click());$('rtU2Presets').append(p);
  }
  const presetRoot=document.querySelector('.presets');
  function syncPresets(){const active=document.querySelector('[data-preset].active')?.dataset.preset;
    document.querySelectorAll('[data-rt-u2-preset]').forEach(b=>b.setAttribute('aria-pressed',String(b.dataset.rtU2Preset===active)));
  }
  if(presetRoot)new MutationObserver(syncPresets).observe(presetRoot,{subtree:true,attributes:true,attributeFilter:['class']});syncPresets();
  function syncStudioState(){
    $('rtU2MicState').textContent=$('stageVoiceState')?.textContent||'Microphone off';
    const panel=$('takePanel');const clock=$('recordClock')?.textContent||'00:00';
    $('rtU2TakeState').textContent=panel&&!panel.classList.contains('hidden')?'Take ready — review and download below':$('recordStopBtn')&&!$('recordStopBtn').disabled?`Recording · ${clock}`:'Record voice alone or with your local song';
  }
  const voice=$('stageVoiceState'),take=$('takePanel'),recordClock=$('recordClock'),recordStop=$('recordStopBtn');
  if(voice)new MutationObserver(syncStudioState).observe(voice,{childList:true,subtree:true,characterData:true});
  if(take)new MutationObserver(syncStudioState).observe(take,{attributes:true,attributeFilter:['class']});
  if(recordClock)new MutationObserver(syncStudioState).observe(recordClock,{childList:true,characterData:true,subtree:true});
  if(recordStop)new MutationObserver(syncStudioState).observe(recordStop,{attributes:true,attributeFilter:['disabled']});
  $('rtU2Mute').addEventListener('click',()=>invoke('stagePanic'));syncStudioState();

  // SETUP — guided checklist, real device selectors/meters and release QA remain in their existing DOM.
  const setupConsole=make('section','rt-u2-console');setupConsole.setAttribute('aria-label','Audio setup guide');
  setupConsole.innerHTML=`<div class="rt-u2-head"><div><p class="rt-u2-eyebrow">03 / SAFE START</p><h2>Connect. Check. Sing.</h2></div><p>Start with low physical speaker volume. Voice monitoring remains off until you request it.</p></div><div class="rt-u2-steps"><div class="rt-u2-step"><span class="rt-u2-step-num">01 — INPUT</span><strong>Connect your microphone</strong><p id="rtU2InputState">Select a microphone below; the raw meter confirms signal.</p><button id="rtU2StartMic" type="button" class="rt-u2-action primary">Start microphone</button></div><div class="rt-u2-step"><span class="rt-u2-step-num">02 — OUTPUT</span><strong>Select speakers</strong><p id="rtU2OutputState">Choose the actual speaker output and keep its volume low.</p><button id="rtU2GotoOutput" type="button" class="rt-u2-action">Jump to output device</button></div><div class="rt-u2-step"><span class="rt-u2-step-num">03 — VERIFY</span><strong>Check before monitoring</strong><p>Confirm five real hardware checks. Browser tests cannot detect physical acoustic feedback.</p><button id="rtU2GotoQa" type="button" class="rt-u2-action">Open hardware checklist</button></div></div><p class="rt-u2-qa-status" id="rtU2QaStatus" role="status" aria-live="polite">Hardware qualification: not yet verified</p>`;
  setup.prepend(setupConsole);
  $('rtU2StartMic').addEventListener('click',()=>invoke('startBtn'));
  function scrollToElement(id){const el=$(id);if(!el)return; if(el.tagName==='DETAILS')el.open=true;el.scrollIntoView({block:'start',behavior:'instant'});el.focus?.({preventScroll:true})}
  $('rtU2GotoOutput').addEventListener('click',()=>scrollToElement('outputSelect'));
  $('rtU2GotoQa').addEventListener('click',()=>scrollToElement('hardwareQa'));
  function syncSetup(){
    const input=$('inputSelect'),output=$('outputSelect');
    $('rtU2InputState').textContent=`Input: ${input?.selectedOptions?.[0]?.textContent||'System default'} · ${$('startBtn')?.disabled?'Connected':'Not started'}`;
    $('rtU2OutputState').textContent=`Output: ${output?.selectedOptions?.[0]?.textContent||'System default'}. Keep volume low before enabling monitoring.`;
    $('rtU2StartMic').disabled=!!$('startBtn')?.disabled;
    $('rtU2QaStatus').textContent=$('qaStatus')?.textContent||'0 of 5 hardware checks verified';
  }
  for(const id of ['inputSelect','outputSelect'])$(id)?.addEventListener('change',syncSetup);
  for(const id of ['startBtn','stopBtn'])if($(id))new MutationObserver(syncSetup).observe($(id),{attributes:true,attributeFilter:['disabled']});
  if($('qaStatus'))new MutationObserver(syncSetup).observe($('qaStatus'),{childList:true,subtree:true,characterData:true});syncSetup();
})();
