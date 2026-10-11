/* ROOMTONE U7 — screenshot-grounded positioning fix; audio controls untouched.
 * The bottom Queue/Mixer bar visibly crossed Stage transport on mobile.
 * Reparent the EXISTING dock into normal Stage flow. The same original
 * emergency mute is repositioned without replacing its click listener.
 */
(()=>{
  'use strict';
  const view=document.getElementById('rtStagePanel');
  const dock=view?.querySelector('.rt-u5-mobile-dock');
  const heading=view?.querySelector('.rt-view-title');
  const header=document.querySelector('.rt-header');
  const navigation=header?.querySelector('.rt-navigation');
  const stage=document.getElementById('stage');
  const panic=document.getElementById('rtMobileMute');
  if(!view||!dock||!heading||!header||!navigation||!stage||!panic)return;

  // Retain the U5 real queue/mixer buttons and their event listeners.
  heading.insertAdjacentElement('afterend',dock);
  dock.classList.add('rt-u7-stage-dock');
  dock.setAttribute('aria-label','Open the stage queue or mixer');
  dock.setAttribute('data-rt-u7-layout','in-flow');
  const updateTop=()=>{
    const offset=Math.ceil(header.getBoundingClientRect().height)+6;
    dock.style.setProperty('--rt-u7-sticky-top',offset+'px');
  };
  updateTop();
  if(typeof ResizeObserver==='function'){
    const observer=new ResizeObserver(updateTop);
    observer.observe(header);
  }else{
    window.addEventListener('resize',updateTop,{passive:true});
  }
  // Screenshot defect: fixed mute obscured Next at 390px. Move the same button
  // into the sticky navigation. DOM reparenting preserves its original listeners.
  panic.textContent='Mute';
  panic.title='Emergency mute live microphone monitoring (Escape)';
  panic.setAttribute('aria-keyshortcuts','Escape');
  panic.setAttribute('data-rt-u7-safety','preserved');
  const syncPanicPlacement=()=>{
    const focusing=stage.classList.contains('focus-mode');
    const target=focusing?stage:navigation;
    if(panic.parentElement!==target){
      if(focusing)stage.prepend(panic);
      else navigation.append(panic);
    }
    panic.dataset.rtU7Placement=focusing?'focus':'navigation';
  };
  new MutationObserver(syncPanicPlacement).observe(stage,{attributes:true,attributeFilter:['class']});
  syncPanicPlacement();
})();
