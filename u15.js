/* ROOMTONE U15 — modal accessibility and focus lifecycle hardening.
   U14 remains the original focus trapping implementation. This addon only
   manages background inertness and restoration/route focus; no audio, DSP,
   microphone, local files, storage or original event handler is changed. */
(()=>{
  'use strict';
  const stage=document.getElementById('stage');
  const view=document.getElementById('rtStagePanel');
  const focusButton=document.getElementById('stageFocus');
  if(!stage||!view||!focusButton)return;
  const marked=new Map();
  let applied=false;
  let returnTarget=null;
  let routeToSongs=false;

  const focusMode=()=>stage.classList.contains('focus-mode');
  const modalActive=()=>focusMode()&&!view.hidden;

  // Save the original state. Do not strip a pre-existing inert attribute on
  // exit, and never mark the Stage or any of its ancestor containers inert.
  const markBackground=()=>{
    for(let node=stage;node&&node!==document.body;node=node.parentElement){
      const parent=node.parentElement;
      if(!parent)break;
      for(const sibling of parent.children){
        if(sibling===node||sibling.tagName==='SCRIPT'||sibling.tagName==='STYLE')continue;
        if(!marked.has(sibling))marked.set(sibling,sibling.inert);
        sibling.inert=true;
      }
    }
  };
  const restoreBackground=()=>{
    for(const [element,previous] of marked){
      if(element.isConnected)element.inert=previous;
    }
    marked.clear();
  };
  const canRestore=(target)=>target?.isConnected &&
    !target.closest('[inert],[hidden]') &&
    getComputedStyle(target).visibility!=='hidden' &&
    getComputedStyle(target).display!=='none' &&
    target.getClientRects().length>0;
  const moveToSongs=()=>{
    const heading=document.getElementById('rtSongsHeading');
    if(heading && canRestore(heading)){
      heading.setAttribute('tabindex','-1');
      heading.focus({preventScroll:true});
    }
  };
  const sync=()=>{
    const now=modalActive();
    if(now===applied)return;
    applied=now;
    if(now){
      markBackground();
    }else{
      restoreBackground();
      const target=returnTarget;
      const songs=routeToSongs;
      returnTarget=null;
      routeToSongs=false;
      // Original U14 focus observer and U7 Mute reparenting run on the same
      // class mutation. Restore only after those updates have settled.
      queueMicrotask(()=>{
        if(modalActive())return;
        if(songs)moveToSongs();
        else if(canRestore(target))target.focus({preventScroll:true});
      });
    }
  };
  new MutationObserver(sync).observe(stage,{attributes:true,attributeFilter:['class']});
  new MutationObserver(sync).observe(view,{attributes:true,attributeFilter:['hidden']});
  sync();

  document.addEventListener('click',event=>{
    const target=event.target;
    if(!(target instanceof Element))return;
    // Store the origin of keyboard F or another programmatic Focus-button
    // activation when focus started outside the Stage.
    if(target.closest('#stageFocus') && !focusMode() && !view.hidden){
      const origin=document.activeElement;
      returnTarget=origin instanceof HTMLElement && !stage.contains(origin) ? origin : null;
    }

    if(!modalActive())return;
    if(target.closest('#jumpToStudio')){
      routeToSongs=true;
      returnTarget=null;
      // Exit via the same original button before U2's capture listener
      // reveals Songs. Do not suppress or replace its original navigation.
      focusButton.click();
      return;
    }
    const nav=target.closest('[data-rt-view]');
    if(nav && nav.dataset.rtView!=='stage'){
      routeToSongs=false;
      returnTarget=null;
      // Prevent stale focus-mode/body.in-focus after any workspace switch.
      focusButton.click();
    }
  },true);
})();
