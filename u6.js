/* ROOMTONE U6 — repair observable list presentation, not track or audio data.
 * U5 remains the owner of filtering; U2 remains the owner of the local queue.
 */
(()=>{
  'use strict';
  const $=id=>document.getElementById(id);
  const list=$('rtU2SongList'),source=$('trackQueue'),filters=$('rtU5LibraryFilters');
  const count=$('rtU5FilteredCount'),status=$('rtU2SongStatus');
  if(!list||!source||!filters||!count||!status)return;

  // The U2 label contains an inherited "N. " prefix from the source queue
  // while the redesigned list already renders N in a separate number column.
  // Remove only the exact queue-ordinal prefix; never alter the original queue.
  function tidyLabels(){
    for(const row of list.querySelectorAll('.rt-u2-song-row')){
      const number=Number(row.querySelector('.rt-u2-song-number')?.textContent);
      const choose=row.querySelector('.rt-u2-song-select');
      if(!Number.isInteger(number)||number<1||!choose)continue;
      const prefix=new RegExp('^\\s*'+number+'\\.\\s+');
      const before=choose.textContent||'';
      const after=before.replace(prefix,'');
      if(after!==before){
        choose.textContent=after;
        choose.setAttribute('aria-label','Play '+after);
      }
    }
  }

  // Stage mirrors have the same double-ordinal defect: "02 2. Song".
  const stageMirror=$('rtQueueMirror'),now=$('rtU2SongNow');
  function tidyStageMirror(){
    if(stageMirror){
      for(const button of stageMirror.querySelectorAll('.rt-queue-choice')){
        const ordinal=Number(button.querySelector('.rt-queue-index')?.textContent);
        const label=button.lastElementChild;
        if(!Number.isInteger(ordinal)||ordinal<1||!label||label.classList.contains('rt-queue-index'))continue;
        const cleaned=(label.textContent||'').replace(new RegExp('^\\s*'+ordinal+'\\.\\s+'),'');
        if(cleaned!==label.textContent){label.textContent=cleaned;button.title=cleaned;}
      }
    }
    if(now){
      const rows=[...source.querySelectorAll('.queue-item')];
      const index=rows.findIndex(li=>li.classList.contains('current'))+1;
      if(index>0){
        const cleaned=(now.textContent||'').replace(new RegExp('^\\s*'+index+'\\.\\s+'),'');
        if(now.textContent!==cleaned)now.textContent=cleaned;
      }
    }
  }
  if(stageMirror)new MutationObserver(tidyStageMirror).observe(stageMirror,{childList:true});

  const empty=document.createElement('p');
  empty.className='rt-u6-filter-empty';
  empty.id='rtU6FilterEmpty';
  empty.setAttribute('role','status');
  empty.hidden=true;
  list.after(empty);

  function repair(){
    tidyLabels();
    tidyStageMirror();
    const rows=[...list.querySelectorAll('.rt-u2-song-row')];
    const visible=rows.filter(row=>!row.hidden).length;
    const total=source.querySelectorAll('.queue-item').length;
    const selected=filters.querySelector('[data-rt-u5-filter][aria-pressed="true"]')?.dataset.rtU5Filter||'all';
    const query=$('rtSongSearch')?.value.trim()||'';
    const summary=visible===1?'1 song shown':visible+' songs shown';
    const suffix=selected==='ready'?'with synchronized lyrics':selected==='missing'?'without synchronized lyrics':'in your library';
    // Keep U5's count, U2's existing polite status, and the actual visible rows aligned.
    if(count.textContent!==summary)count.textContent=summary;
    const message=summary+' '+suffix+' · '+total+' total';
    if(status.textContent!==message)status.textContent=message;
    const filteredOut=rows.length>0&&visible===0;
    empty.hidden=!filteredOut;
    if(filteredOut){
      const reason=selected==='ready'
        ?'No matching songs have timed lyrics. Select a track and attach an .lrc file.'
        :selected==='missing'
        ?'Every matching song already has lyrics.'
        :'No matching songs. Try clearing the search.';
      if(empty.textContent!==reason)empty.textContent=reason;
    }
    // When search itself found no rows, U2 already renders its own empty state.
    // Do not append a second explanation.
    if(rows.length===0)empty.hidden=true;
  }

  // U5 registers its observers before U6, so its filter is applied first.
  // Listen only for structural replacement, never loop on cosmetic text updates.
  new MutationObserver(repair).observe(list,{childList:true});
  new MutationObserver(repair).observe(source,{childList:true,subtree:true,attributes:true,attributeFilter:['class']});
  filters.addEventListener('click',event=>{
    if(event.target.closest('[data-rt-u5-filter]'))queueMicrotask(repair);
  });
  $('rtSongSearch')?.addEventListener('input',()=>queueMicrotask(repair));
  repair();

  // The Escape safety shortcut must still mute monitoring in the original app.
  // In addition, close any open mobile sheet and return focus to its trigger.
  document.addEventListener('keydown',event=>{
    if(event.key!=='Escape'||event.repeat)return;
    const kind=document.body.dataset.rtU5Sheet;
    if(kind!=='queue'&&kind!=='mix')return;
    const sheet=$(kind==='queue'?'rtU5QueueSheet':'rtU5MixSheet');
    queueMicrotask(()=>sheet?.querySelector('.rt-u5-sheet-close')?.click());
  });
})();
