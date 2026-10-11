/* ROOMTONE U7 — screenshot-grounded positioning fix; audio controls untouched.
 * The bottom Queue/Mixer bar visibly crossed Stage transport on mobile.
 * Reparent the EXISTING dock into normal Stage flow. The independent original
 * emergency mute stays accessible in every workspace and is never reparented.
 */
(()=>{
  'use strict';
  const view=document.getElementById('rtStagePanel');
  const dock=view?.querySelector('.rt-u5-mobile-dock');
  const heading=view?.querySelector('.rt-view-title');
  const header=document.querySelector('.rt-header');
  const panic=document.getElementById('rtMobileMute');
  if(!view||!dock||!heading||!header||!panic)return;

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
  // Keep the actual safety button globally reachable, including Songs and Setup.
  panic.setAttribute('data-rt-u7-safety','preserved');
})();
