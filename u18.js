/* ROOMTONE U18: restore keyboard focus when the original Lyrics studio route
 * closes fullscreen Stage. The original route, audio, mic, queue, recording,
 * navigation listeners and U14 modal lifecycle stay unchanged.
 *
 * Proven against unchanged U16: original click routes correctly to Songs but
 * leaves document.activeElement on BODY (U18 push 38184768879).
 */
(()=>{
  'use strict';
  const stage=document.getElementById('stage');
  const action=document.getElementById('jumpToStudio');
  const songs=document.getElementById('rtSongsPanel');
  const heading=document.getElementById('rtSongsHeading');
  if(!stage||!action||!songs||!heading)return;
  // Observe only the genuine existing Stage action, without replacing,
  // cancelling, redispatching or intercepting any original click handler.
  document.addEventListener('click',event=>{
    if(event.target!==action && !action.contains(event.target))return;
    if(!stage.classList.contains('focus-mode'))return;
    // U1 switches view and the original Stage handler exits focus later in
    // this click dispatch; U14 releases inertness in its mutation microtask.
    requestAnimationFrame(()=>{
      if(songs.hidden || stage.classList.contains('focus-mode') ||
         document.body.classList.contains('in-focus') ||
         heading.closest('[inert],[hidden]'))return;
      heading.setAttribute('tabindex','-1');
      heading.focus({preventScroll:true});
    });
  },true);
})();
