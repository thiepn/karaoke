/* ROOMTONE U14 — fullscreen keyboard accessibility only.
   Proven U13 defect: on desktop, Tab #6 from fullscreen focus reached
   #rtAddSong behind the fixed performance Stage. Do not replace any
   original click handlers, safety keyboard handlers, recording, audio,
   queue or storage. */
(()=>{
  'use strict';
  const stage=document.getElementById('stage');
  const view=document.getElementById('rtStagePanel');
  const focusButton=document.getElementById('stageFocus');
  if(!stage||!view||!focusButton)return;
  const initialRole=stage.getAttribute('role');
  const initialModal=stage.getAttribute('aria-modal');
  let modal=false;

  const isActive=()=>stage.classList.contains('focus-mode')&&!view.hidden;
  const visibleFocusables=()=>{
    const selector='a[href],button:not([disabled]),input:not([disabled]),select:not([disabled]),textarea:not([disabled]),[tabindex]:not([tabindex="-1"])';
    return [...stage.querySelectorAll(selector)].filter(element=>{
      if(element.closest('[inert],[hidden]'))return false;
      const style=getComputedStyle(element);
      return style.visibility!=='hidden'&&style.display!=='none'&&element.getClientRects().length>0;
    });
  };
  const restoreAttribute=(name,value)=>{
    if(value===null)stage.removeAttribute(name);
    else stage.setAttribute(name,value);
  };
  const sync=()=>{
    const now=isActive();
    if(now===modal)return;
    modal=now;
    if(now){
      stage.setAttribute('role','dialog');
      stage.setAttribute('aria-modal','true');
      // Original focus button stays the activation and exit control.
      // If a shortcut activated focus from outside Stage, move focus
      // into it so the modal keyboard cycle has a valid starting point.
      if(!stage.contains(document.activeElement))
        focusButton.focus({preventScroll:true});
    }else{
      restoreAttribute('role',initialRole);
      restoreAttribute('aria-modal',initialModal);
    }
  };
  new MutationObserver(sync).observe(stage,{attributes:true,attributeFilter:['class']});
  new MutationObserver(sync).observe(view,{attributes:true,attributeFilter:['hidden']});
  sync();

  document.addEventListener('keydown',event=>{
    if(event.key!=='Tab'||event.altKey||event.ctrlKey||event.metaKey||!isActive())return;
    const controls=visibleFocusables();
    if(!controls.length)return;
    const first=controls[0],last=controls[controls.length-1];
    const active=document.activeElement;
    if(!stage.contains(active)){
      event.preventDefault();
      (event.shiftKey?last:first).focus();
    }else if(event.shiftKey&&active===first){
      event.preventDefault();
      last.focus();
    }else if(!event.shiftKey&&active===last){
      event.preventDefault();
      first.focus();
    }
  },true);
  // Programmatic focus and assistive navigation cannot strand keyboard
  // users behind the modal. This is focus-only; no audio/controls touched.
  document.addEventListener('focusin',event=>{
    if(isActive()&&!stage.contains(event.target))
      focusButton.focus({preventScroll:true});
  });
})();
