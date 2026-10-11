/* ROOMTONE U19 — recover an existing modal when the application switches
 * workspaces and hides Stage. Source-only Chromium reproduction: a synthetic
 * original Studio nav click changed views but left fullscreen Stage, body
 * scroll lock and background inertness active, with focus on BODY.
 *
 * Preserve U14's modal/inert/focus manager, U18's Lyrics Studio handler,
 * original U1 navigation, safety Mute, playback, microphone, storage and data.
 * No new route dispatch, no intercepted event or duplicate original handler.
 */
(()=>{
  'use strict';
  const stage=document.getElementById('stage');
  const stagePanel=document.getElementById('rtStagePanel');
  const focusButton=document.getElementById('stageFocus');
  if(!stage||!stagePanel||!focusButton)return;

  // The original switchView hides Stage before U14 can observe a close.
  // React only to that *actual* hidden transition; normal fullscreen toggles
  // and the existing U18 Lyrics studio path retain their own lifecycle.
  const recoverHiddenModal=()=>{
    if(!stagePanel.hidden ||
       !stage.classList.contains('focus-mode') ||
       !document.body.classList.contains('in-focus'))return;

    // Reuse the application's original close control. U14 then releases
    // background inertness and restores its previous ARIA attributes.
    focusButton.click();

    // The view's original switchView has already selected and revealed its
    // destination. Restore heading focus only after modal cleanup settles.
    requestAnimationFrame(()=>{
      if(stage.classList.contains('focus-mode') ||
         document.body.classList.contains('in-focus') ||
         !stagePanel.hidden)return;
      const destination=[...document.querySelectorAll('.rt-view[data-rt-panel]')]
        .find(panel=>!panel.hidden && panel.id!=='rtStagePanel');
      const heading=destination?.querySelector('h1');
      if(!heading || heading.closest('[inert],[hidden]') ||
         heading.getClientRects().length===0)return;
      heading.setAttribute('tabindex','-1');
      heading.focus({preventScroll:true});
    });
  };
  new MutationObserver(recoverHiddenModal).observe(stagePanel,{
    attributes:true,attributeFilter:['hidden']
  });
  recoverHiddenModal();
})();
