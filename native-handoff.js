/* Explicit offline browser -> WASAPI handoff; no localhost relay. */
(()=>{
 'use strict';
 const $=id=>document.getElementById(id);
 const container=$('rtSongsContent'),player=$('songPlayer');
 if(!container||!player||$('rtNativeHandoff'))return;
 const block=document.createElement('section');
 block.id='rtNativeHandoff';block.className='card rt-native-handoff';
 block.setAttribute('aria-label','Windows native karaoke handoff');
 block.innerHTML=[
  '<div class="card-head"><h2>Windows native karaoke</h2><span class="tag">LOCAL / MANUAL</span></div>',
  '<p class="help">Prepare the selected local song, timed lyrics, loop and playback position for the existing Windows WASAPI program. No server, audio upload or automatic executable launch.</p>',
  '<p id="rtHandoffTrack" class="help">Choose a local song first.</p>',
  '<div class="rt-handoff-actions"><button id="rtHandoffPrepare" type="button" class="btn" disabled>Stop browser audio &amp; download session</button>',
  '<button id="rtHandoffReturn" type="button" class="btn" hidden>Return to browser audio</button></div>',
  '<p id="rtHandoffState" role="status" aria-live="polite" class="help">Browser owns playback. No handoff prepared.</p>',
  '<p class="help">Open the Windows native application, choose the SAME original instrumental and the downloaded .rtk file. Lyrics are transferred; audio bytes are not. Browser and native playback do not synchronize automatically.</p>'
 ].join('');
 container.append(block);
 const prepare=$('rtHandoffPrepare'),returnBtn=$('rtHandoffReturn');
 const state=$('rtHandoffState'),trackLabel=$('rtHandoffTrack');
 let locked=false,busy=false;
 const track=()=>typeof queue!=='undefined'&&typeof activeTrack!=='undefined'?queue[activeTrack]:null;
 const lockedMessage='Native handoff active: browser playback and microphone monitoring disabled. Return to browser audio explicitly before using ROOMTONE again.';
 const refresh=()=>{
  const song=track();
  trackLabel.textContent=song?'Selected: '+(song.title||song.file.name)+' · '+(song.lyrics?.length||0)+' lyric lines':'Choose a local song first.';
  prepare.disabled=!song||locked||busy||!!recordSession;
  returnBtn.hidden=!locked;
 };
 const finiteMillis=(seconds,label)=>{
  if(!Number.isFinite(seconds)||seconds<0||seconds>720)
   throw new Error(label+' must be 0–720 seconds');
  return Math.round(seconds*1000);
 };
 const build=song=>{
  if(!song?.file?.name)throw new Error('Choose a local song first');
  const encoder=new TextEncoder();
  const title=song.title||song.file.name;
  if(encoder.encode(song.file.name).length>255||encoder.encode(title).length>255)
   throw new Error('Song title or filename exceeds native limit');
  const lines=song.lyrics||[];
  if(lines.length>500)throw new Error('Too many lyric lines (maximum 500)');
  const offset=Number(song.offset)||0;
  if(!Number.isInteger(offset)||Math.abs(offset)>30000)throw new Error('Lyric offset out of bounds');
  const loop=song.loop||{};
  const valid=!!loop.enabled&&Number.isFinite(loop.a)&&Number.isFinite(loop.b)&&loop.b>loop.a+.25;
  const loopStart=valid?finiteMillis(loop.a,'Loop start'):0;
  const loopEnd=valid?finiteMillis(loop.b,'Loop end'):0;
  const current=finiteMillis(Number(player.currentTime)||0,'Song position');
  const values=[
   'ROOMTONE_HANDOFF_V1','audio\t'+encodeURIComponent(song.file.name),
   'title\t'+encodeURIComponent(title),'start-ms\t'+current,'offset-ms\t'+offset,
   'loop\t'+(valid?'1':'0'),'loop-start-ms\t'+loopStart,'loop-end-ms\t'+loopEnd
  ];
  for(const line of [...lines].sort((a,b)=>a.time-b.time)){
   const ms=finiteMillis(line.time,'Lyric timestamp');
   if(encoder.encode(line.text||'').length>2000)throw new Error('Lyric line too long');
   values.push('lyric\t'+ms+'\t'+encodeURIComponent(line.text||''));
  }
  const text=values.join('\n')+'\n';
  if(encoder.encode(text).length>1048576)throw new Error('Handoff too large');
  return text;
 };
 const cancelBrowserStart=e=>{
  if(!locked)return;
  e.preventDefault();e.stopImmediatePropagation();
  if(!player.paused)player.pause();
  if(typeof monitorMute==='function')monitorMute();
  state.textContent=lockedMessage;
 };
 for(const id of ['startBtn','monitorBtn','recordBtn','recordAndPlay','stagePlay','songRestart']){
  $(id)?.addEventListener('click',cancelBrowserStart,true);
 }
 player.addEventListener('play',()=>{
  if(locked){player.pause();state.textContent=lockedMessage;}
 });
 prepare.addEventListener('click',async()=>{
  if(busy||locked)return;
  if(recordSession){state.textContent='Stop and save the active recording before handoff.';refresh();return;}
  const song=track();if(!song){refresh();return;}
  busy=true;refresh();
  try{
   const data=build(song);
   player.pause();
   if(typeof monitorMute==='function')monitorMute();
   if(typeof stopMic==='function')await stopMic();
   const blob=new Blob([data],{type:'text/plain;charset=utf-8'});
   const url=URL.createObjectURL(blob),a=document.createElement('a');
   a.href=url;
   a.download=(song.file.name.replace(/\.[^.]+$/,'').replace(/[<>:"/\\|?*\u0000-\u001f]/g,'_').slice(0,100)||'roomtone')+'.rtk';
   document.body.append(a);a.click();a.remove();
   setTimeout(()=>URL.revokeObjectURL(url),30000);
   locked=true;
   state.textContent='Browser audio stopped and locked. Offline session downloaded. Open the native Windows app and select the matching song + session file; nothing was uploaded or auto-played.';
  }catch(e){
   state.textContent='Handoff not completed: '+(e?.message||e)+'. Browser music will not restart automatically.';
  }finally{busy=false;refresh();}
 });
 returnBtn.addEventListener('click',()=>{
  if(busy)return;
  locked=false;
  state.textContent='Browser audio ownership restored. Microphone monitoring stays muted until manually enabled.';
  refresh();
 });
 const observer=new MutationObserver(refresh);
 observer.observe($('trackQueue')||container,{childList:true,subtree:true});
 refresh();
})();
