/* ROOMTONE U4 — repair accessibility around dynamic DOM replacements.
 * Presentation only. No microphone, DSP, MediaRecorder, file contents, or saved-data writes.
 */
(()=>{
  'use strict';
  const $=id=>document.getElementById(id);
  const main=$('rtWorkspace');
  if(!main||!$('rtQueueMirror')||!$('rtU2SongList'))return;
  const shell=document.querySelector('.shell');
  if(shell){
    const skip=document.createElement('a');
    skip.href='#rtWorkspace';skip.className='rt-u4-skip';skip.textContent='Skip to karaoke workspace';
    shell.prepend(skip);main.tabIndex=-1;
    skip.addEventListener('click',event=>{
      event.preventDefault();
      const active=main.querySelector('.rt-view:not([hidden]) h1');
      if(active)active.tabIndex=-1;
      (active||main).focus({preventScroll:false});
    });
  }
  function recoverMirroredFocus(listId,selector){
    const list=$(listId);
    if(!list)return;
    list.addEventListener('click',event=>{
      const target=event.target.closest(selector);
      if(!target||!list.contains(target)||event.detail!==0||document.activeElement!==target)return;
      const buttons=[...list.querySelectorAll(selector)];
      const index=buttons.indexOf(target);
      if(index<0)return;
      requestAnimationFrame(()=>{
        if(!list.isConnected||list.closest('[hidden]'))return;
        if(document.activeElement!==document.body&&document.activeElement!==target)return;
        const replacement=list.querySelectorAll(selector)[index];
        if(replacement&&!replacement.disabled)replacement.focus({preventScroll:true});
        else if(listId==='rtU2SongList')$('rtSongSearch')?.focus({preventScroll:true});
        else $('rtAddSong')?.focus({preventScroll:true});
      });
    },true);
  }
  recoverMirroredFocus('rtU2SongList','.rt-u2-song-select');
  recoverMirroredFocus('rtQueueMirror','.rt-queue-choice');
  const original=$('trackQueue');
  original?.addEventListener('click',event=>{
    const control=event.target.closest('button');
    if(!control||!original.contains(control)||event.detail!==0||document.activeElement!==control)return;
    const item=control.closest('.queue-item');
    const oldIndex=[...original.querySelectorAll('.queue-item')].indexOf(item);
    if(oldIndex<0)return;
    const kind=control.classList.contains('queue-move')?'move':control.classList.contains('queue-remove')?'remove':control.classList.contains('queue-play')?'play':null;
    if(!kind)return;
    const isUp=control.textContent?.trim()==='↑';
    const dest=kind==='move'?oldIndex+(isUp?-1:1):oldIndex;
    const cls=kind==='move'?'.queue-move':'.queue-play';
    requestAnimationFrame(()=>{
      if(original.closest('[hidden]')||original.closest('details:not([open])'))return;
      if(document.activeElement!==document.body&&document.activeElement!==control)return;
      const items=[...original.querySelectorAll('.queue-item')];
      const row=items[Math.min(Math.max(0,dest),items.length-1)];
      const candidate=kind==='move'?[...row?.querySelectorAll(cls)||[]].find(b=>b.textContent?.trim()===(isUp?'↑':'↓')):row?.querySelector(cls);
      const safe=candidate&&!candidate.disabled?candidate:row?.querySelector('.queue-play:not(:disabled)');
      (safe||$('rtU3OpenTracks')||$('rtU2AddTracks'))?.focus({preventScroll:true});
    });
  },true);
  const announce=document.createElement('p');
  announce.id='rtU4TrackAnnouncement';announce.className='rt-u4-visually-hidden';
  announce.setAttribute('role','status');announce.setAttribute('aria-live','polite');
  main.prepend(announce);
  let lastSong='';
  const title=$('stageTitle');
  function announceSong(){
    const song=title?.textContent?.trim()||'';
    if(!song||song===lastSong)return;
    lastSong=song;
    if(song!=='No song loaded')announce.textContent='Selected song: '+song;
    else announce.textContent='No song selected';
  }
  if(title)new MutationObserver(announceSong).observe(title,{childList:true,subtree:true,characterData:true});
  announceSong();
  const songList=$('rtU2SongList'),mirror=$('rtQueueMirror');
  function indicateCurrent(container,sel){
    if(!container)return;
    for(const row of container.querySelectorAll('li')){
      const isCurrent=row.classList.contains('active')||row.classList.contains('current');
      const b=row.querySelector(sel);if(!b)continue;
      if(isCurrent)b.setAttribute('aria-current','true');else b.removeAttribute('aria-current');
    }
  }
  for(const [list,sel] of [[songList,'.rt-u2-song-select'],[mirror,'.rt-queue-choice']]){
    if(!list)continue;
    const sync=()=>indicateCurrent(list,sel);
    new MutationObserver(sync).observe(list,{childList:true});sync();
  }
  $('jumpToStudio')?.addEventListener('click',()=>{
    requestAnimationFrame(()=>{
      const editor=$('songTitleEdit');
      if($('rtSongsPanel')?.hidden||!editor||!editor.getClientRects().length)return;
      if(document.activeElement===document.body||document.activeElement===$('jumpToStudio'))editor.focus({preventScroll:true});
    });
  });
  const stop=$('recordStopBtn'),take=$('takePanel'),download=$('takeDownload');
  const state=$('rtU3RecordState');
  let pendingKeyboardTake=false;
  stop?.addEventListener('click',event=>{
    if(event.detail===0&&document.activeElement===stop)pendingKeyboardTake=true;
  },true);
  for(const id of ['recordBtn','recordAndPlay'])$(id)?.addEventListener('click',()=>{pendingKeyboardTake=false;},true);
  const reviewReady=()=>!!take&&!take.classList.contains('hidden')&&!!download?.getAttribute('href')&&!!$('takePlayer')?.getAttribute('src');
  function syncReview(){
    if(take&&!take.classList.contains('hidden')&&!reviewReady()&&state){
      state.textContent='No playable recording available yet. Finish a recording to review or download it.';
    }
    if(pendingKeyboardTake&&reviewReady()){
      pendingKeyboardTake=false;
      requestAnimationFrame(()=>{
        if($('rtStudioPanel')?.hidden)return;
        if(document.activeElement===document.body||document.activeElement===stop)download.focus({preventScroll:true});
      });
    }
  }
  if(take)new MutationObserver(syncReview).observe(take,{attributes:true,attributeFilter:['class']});
  if(download)new MutationObserver(syncReview).observe(download,{attributes:true,attributeFilter:['href']});
  if(state)new MutationObserver(()=>{if(!reviewReady()&&take&&!take.classList.contains('hidden')&&state.textContent.startsWith('Take ready.'))syncReview();}).observe(state,{childList:true});
})();
