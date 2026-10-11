/* ROOMTONE U14: fixed fullscreen Stage is a modal interaction surface.
 * The inherited U13 focus mode already handles layout, Escape/muting and
 * scroll. This layer manages only accessibility exposure and focus.
 * No audio, recording, microphone, song library or persistence interaction.
 */
(() => {
  'use strict';
  const stage = document.getElementById('stage');
  const focusButton = document.getElementById('stageFocus');
  if (!stage || !focusButton) return;

  const original = Object.fromEntries(
    ['role', 'aria-modal', 'aria-labelledby'].map(name => [name, stage.getAttribute(name)]));
  const modifiedInert = new Set();
  let active = false;
  let returnFocus = null;
  const isOpen = () => document.body.classList.contains('in-focus') &&
                        stage.classList.contains('focus-mode');

  const eligible = () => [...stage.querySelectorAll(
    'button:not([disabled]), a[href], input:not([disabled]), select:not([disabled]), ' +
    'textarea:not([disabled]), [tabindex]:not([tabindex="-1"])'
  )].filter(el => !el.inert && el.getClientRects().length > 0 &&
                   getComputedStyle(el).visibility !== 'hidden');

  const restoreAttributes = () => {
    for (const [name, value] of Object.entries(original)) {
      if (value === null) stage.removeAttribute(name);
      else stage.setAttribute(name, value);
    }
  };

  const enter = () => {
    if (active || !isOpen()) return;
    active = true;
    returnFocus = document.activeElement instanceof HTMLElement
      ? document.activeElement : null;
    stage.setAttribute('role', 'dialog');
    stage.setAttribute('aria-modal', 'true');
    stage.setAttribute('aria-labelledby', 'stageTitle');

    // Inert only siblings on the Stage-to-body ancestor chain. Never inert
    // the Stage itself or replace original DOM nodes or handlers.
    let child = stage;
    while (child && child !== document.body) {
      const parent = child.parentElement;
      if (!parent) break;
      for (const sibling of parent.children) {
        if (sibling !== child && sibling instanceof HTMLElement && !sibling.inert) {
          sibling.inert = true;
          modifiedInert.add(sibling);
        }
      }
      child = parent;
    }
    if (!stage.contains(document.activeElement))
      (focusButton.getClientRects().length ? focusButton : eligible()[0])
        ?.focus({preventScroll: true});
  };

  const exit = () => {
    if (!active) return;
    active = false;
    for (const node of modifiedInert) node.inert = false;
    modifiedInert.clear();
    restoreAttributes();
    // Do not force focus away from a still-visible Stage button. If focus
    // entered from outside the Stage (e.g. shortcut), restore its origin.
    if (returnFocus && returnFocus.isConnected && !stage.contains(returnFocus))
      returnFocus.focus({preventScroll: true});
    returnFocus = null;
  };

  new MutationObserver(() => { if (isOpen()) enter(); else exit(); })
    .observe(document.body, {attributes:true,attributeFilter:['class']});
  // Keep keyboard focus inside the visible fullscreen surface. Native inert
  // hides background controls from AT; wrapping guards Tab/Shift+Tab.
  document.addEventListener('keydown', event => {
    if (!active || event.key !== 'Tab' || event.altKey || event.ctrlKey || event.metaKey) return;
    const items = eligible();
    if (!items.length) return;
    const focused = document.activeElement;
    if (!stage.contains(focused)) {
      event.preventDefault();
      items[0].focus({preventScroll:true});
    } else if (event.shiftKey && focused === items[0]) {
      event.preventDefault();
      items[items.length-1].focus({preventScroll:true});
    } else if (!event.shiftKey && focused === items[items.length-1]) {
      event.preventDefault();
      items[0].focus({preventScroll:true});
    }
  }, true);
  document.addEventListener('focusin', event => {
    if (active && !stage.contains(event.target))
      (eligible()[0] || focusButton).focus({preventScroll:true});
  });
  if (isOpen()) enter();
})();
