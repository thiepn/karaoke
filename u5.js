/* ROOMTONE U5 — presentation only. The original player, queue and mixer remain authoritative. */
(()=>{
  'use strict';
  const $=id=>document.getElementById(id);
  const stage=$('stage'),view=$('rtStagePanel'),songs=$('rtSongsContent');
  const source=$('trackQueue'),player=$('songPlayer');
  if(!stage||!view||!songs||!source||!player||!$('rtU2SongList'))return;
  const el=(tag,cls,text)=>{const n=document.createElement(tag);if(cls)n.className=cls;if(text!==undefined)n.textContent=text;return n;};
  const meta=el('div','rt-u5-stage-meta');
  meta.append(el('span','','01 / AFTER HOURS · PERFORMANCE'));
  const state=el('span','');state.id='rtU5PlayState';meta.append(state);stage.prepend(meta);
  const lyrics=$('lyricView'),current=$('lyricCurrent'),title=$('stageTitle');
  const cue=el('span','rt-u5-cue');cue.id='rtU5LyricCue';lyrics.append(cue);
  const add=el('button','rt-u5-empty-add','+ Add your first song');
  add.type='button';add.id='rtU5EmptyAdd';add.addEventListener('click',()=>$('rtAddSong')?.click());lyrics.append(add);
  function refreshStage(){
    const hasSong=!!$('stagePlay')&&!$('stagePlay').disabled;
    const playing=hasSong&&!player.paused&&!player.ended;
    const next=hasSong?(playing?'PLAYING':player.currentTime>0?'PAUSED':'READY'):'AWAITING TRACK';
    if(state.textContent!==next)state.textContent=next;
    const hasLyrics=!current?.classList.contains('placeholder')&&!!current?.textContent?.trim();
    const cueText=hasLyrics?'SYNCHRONIZED LYRICS':hasSong?'ADD .LRC TO FOLLOW ALONG':'LOCAL AUDIO · NO UPLOAD';
    if(cue.textContent!==cueText)cue.textContent=cueText;
    add.hidden=hasSong;
    stage.classList.toggle('rt-u5-with-lyrics',hasLyrics);
  }
  for(const ev of ['play','pause','ended','loadedmetadata','emptied','timeupdate'])player.addEventListener(ev,refreshStage);
  if(current)new MutationObserver(refreshStage).observe(current,{childList:true,subtree:true,characterData:true,attributes:true,attributeFilter:['class']});
  if(title)new MutationObserver(refreshStage).observe(title,{childList:true,subtree:true,characterData:true});
  if($('stagePlay'))new MutationObserver(refreshStage).observe($('stagePlay'),{attributes:true,attributeFilter:['disabled']});
  refreshStage();

  const library=songs.querySelector('.rt-u2-console'),toolbar=library?.querySelector('.rt-u2-toolbar');
  if(library&&toolbar){
    const row=el('div','rt-u5-filter-row');row.id='rtU5LibraryFilters';
    const group=el('div','rt-u5-filter-group');group.setAttribute('role','group');group.setAttribute('aria-label','Filter songs by lyric availability');
    const count=el('span','rt-u5-filter-count');count.id='rtU5FilteredCount';
    let filter='all';const buttons=[];
    const apply=()=>{
      const items=[...source.querySelectorAll('.queue-item')];
      const rendered=[...$('rtU2SongList').querySelectorAll('.rt-u2-song-row')];
      let visible=0;
      for(const item of rendered){
        const index=Number(item.querySelector('.rt-u2-song-number')?.textContent)-1;
        const sourceRow=Number.isInteger(index)?items[index]:null;
        const ready=!!sourceRow?.querySelector('.queue-lyric-indicator')?.textContent?.trim();
        const show=filter==='all'||(filter==='ready'&&ready)||(filter==='missing'&&!ready);
        item.hidden=!show;
        if(show)visible++;
      }
      const summary=visible===1?'1 song shown':visible+' songs shown';
      if(count.textContent!==summary)count.textContent=summary;
    };
    for(const [value,label] of [['all','All songs'],['ready','Lyrics ready'],['missing','Needs lyrics']]){
      const button=el('button','rt-u5-filter',label);button.type='button';
      button.dataset.rtU5Filter=value;button.setAttribute('aria-pressed',String(value===filter));
      button.addEventListener('click',()=>{
        filter=value;
        for(const item of buttons)item.setAttribute('aria-pressed',String(item===button));
        apply();
      });
      buttons.push(button);group.append(button);
    }
    row.append(group,count);toolbar.after(row);
    new MutationObserver(apply).observe($('rtU2SongList'),{childList:true});
    new MutationObserver(apply).observe(source,{childList:true,subtree:true,characterData:true,attributes:true,attributeFilter:['class']});
    apply();
  }

  const side=view.querySelector('.rt-stage-sidecar');
  const queuePanel=side?.querySelector('.rt-side-queue'),mixPanel=side?.querySelector('.rt-side-mix');
  if(side&&queuePanel&&mixPanel){
    queuePanel.id='rtU5QueueSheet';mixPanel.id='rtU5MixSheet';
    const mobile=window.matchMedia('(max-width: 680px)');
    const dock=el('div','rt-u5-mobile-dock');dock.setAttribute('role','group');dock.setAttribute('aria-label','Stage queue and mix panels');
    const backdrop=el('button','rt-u5-sheet-backdrop');backdrop.type='button';backdrop.hidden=true;backdrop.setAttribute('aria-label','Close stage panel');view.append(backdrop);
    let open=null;const triggers=new Map();
    const queueButton=el('button','','Queue');queueButton.type='button';queueButton.setAttribute('aria-controls','rtU5QueueSheet');queueButton.setAttribute('aria-expanded','false');
    const count=el('span','','00');count.id='rtU5DockQueueCount';queueButton.append(count);
    const mixButton=el('button','','Mixer');mixButton.type='button';mixButton.setAttribute('aria-controls','rtU5MixSheet');mixButton.setAttribute('aria-expanded','false');mixButton.append(el('span','','OPEN'));
    triggers.set('queue',queueButton);triggers.set('mix',mixButton);dock.append(queueButton,mixButton);view.append(dock);
    function setOpen(kind,restoreFocus=false){
      if(kind&&!mobile.matches)return;
      const was=open;open=kind;
      if(kind)document.body.dataset.rtU5Sheet=kind;
      else delete document.body.dataset.rtU5Sheet;
      backdrop.hidden=!kind;
      for(const [key,button] of triggers)button.setAttribute('aria-expanded',String(key===kind));
      if(kind){
        queuePanel.setAttribute('aria-hidden',String(kind!=='queue'));
        mixPanel.setAttribute('aria-hidden',String(kind!=='mix'));
        const panel=kind==='queue'?queuePanel:mixPanel;
        panel.querySelector('.rt-u5-sheet-close')?.focus({preventScroll:true});
      }else{
        queuePanel.removeAttribute('aria-hidden');mixPanel.removeAttribute('aria-hidden');
        if(restoreFocus&&was)triggers.get(was)?.focus({preventScroll:true});
      }
    }
    queueButton.addEventListener('click',()=>setOpen(open==='queue'?null:'queue',open==='queue'));
    mixButton.addEventListener('click',()=>setOpen(open==='mix'?null:'mix',open==='mix'));
    backdrop.addEventListener('click',()=>setOpen(null,true));
    for(const [kind,panel] of [['queue',queuePanel],['mix',mixPanel]]){
      const close=el('button','rt-u5-sheet-close','×');close.type='button';
      close.setAttribute('aria-label','Close '+(kind==='queue'?'queue':'quick mix')+' panel');
      panel.querySelector('.rt-side-heading')?.append(close);
      close.addEventListener('click',()=>setOpen(null,true));
    }
    const syncCount=()=>{
      const formatted=String(source.querySelectorAll('.queue-item').length).padStart(2,'0');
      if(count.textContent!==formatted)count.textContent=formatted;
    };
    new MutationObserver(syncCount).observe(source,{childList:true});syncCount();
    mobile.addEventListener('change',()=>setOpen(null,false));
    for(const nav of document.querySelectorAll('[data-rt-view]'))nav.addEventListener('click',()=>setOpen(null,false),true);
    $('stageFocus')?.addEventListener('click',()=>setOpen(null,false),true);
    document.addEventListener('keydown',event=>{
      if(event.key.toLowerCase()!=='f'||event.ctrlKey||event.altKey||event.metaKey||event.repeat||view.hidden)return;
      if(document.activeElement?.closest('input,textarea,select,button,[contenteditable="true"],[role="slider"]'))return;
      event.preventDefault();$('stageFocus')?.click();
    });
    window.addEventListener('pagehide',()=>{delete document.body.dataset.rtU5Sheet;});
  }
})();
