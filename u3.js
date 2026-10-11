/* ROOMTONE U3 — presentation/keyboard improvements; audio and storage engine untouched. */
(()=>{
  'use strict';
  const $=id=>document.getElementById(id);
  const songs=$('rtSongsContent'),studio=$('rtStudioContent'),setup=$('rtSetupContent');
  if(!songs||!studio||!setup||!$('rtU2SongList'))return;
  const create=(tag,cls,txt)=>{const n=document.createElement(tag);if(cls)n.className=cls;if(txt!==undefined)n.textContent=txt;return n};
  const makeDisclosure=(node,id,title,description,open=false)=>{
    if(!node)return null;
    const details=create('details','rt-u3-disclosure');details.id=id;details.open=open;
    const summary=create('summary','rt-u3-summary');
    const headings=create('span','rt-u3-summary-copy');
    headings.append(create('strong','',title),create('small','',description));
    summary.append(headings,create('span','rt-u3-chevron','+'));
    const inner=create('div','rt-u3-disclosure-body');
    node.replaceWith(details);inner.append(node);details.append(summary,inner);
    return details;
  };
  const toggleAndReveal=(panel,target,focusId)=>{
    if(!panel)return;
    panel.open=true;
    // A click on the original editor jump first reveals its collapsed ancestor.
    requestAnimationFrame(()=>{
      const elem=$(focusId)||target||panel;
      if(elem){elem.scrollIntoView({block:'start',behavior:'instant'});if(elem===panel)elem.focus();else if(!elem.matches('input,button,select,textarea,[tabindex]')){elem.tabIndex=-1;elem.focus({preventScroll:true})}else elem.focus({preventScroll:true});}
    });
  };
  // Songs: preserve original backing-track card and lyrics editor but show one task at a time.
  const songCard=Array.from(songs.children).find(n=>n.classList.contains('card'));
  const legacyTracks=makeDisclosure(songCard,'rtU3TrackTools','Track controls','Playback speed, queue order, timing and recording source settings');
  const lyricsPanel=makeDisclosure($('lyricsStudio'),'rtU3LyricsTools','Lyrics editor & practice','Edit timed lines, export LRC and review local practice sessions');
  if(legacyTracks){const button=create('button','rt-u2-action','Track options');button.type='button';button.id='rtU3OpenTracks';button.setAttribute('aria-controls',legacyTracks.id);$('rtU2AddTracks')?.after(button);button.addEventListener('click',()=>toggleAndReveal(legacyTracks,legacyTracks));}
  if(lyricsPanel){
    const button=create('button','rt-u2-action','Edit lyrics');button.type='button';button.id='rtU3OpenLyrics';button.setAttribute('aria-controls',lyricsPanel.id);$('rtU2AddLyrics')?.after(button);
    button.addEventListener('click',()=>toggleAndReveal(lyricsPanel,lyricsPanel,'songTitleEdit'));
    $('jumpToStudio')?.addEventListener('click',()=>{lyricsPanel.open=true;},{capture:true});
    // Timed lyric editor remains a genuine editable form; expose its change announcements.
    $('lyricEditor')?.setAttribute('aria-label','Timed lyrics, editable line list');
    $('bulkLyrics')?.setAttribute('aria-describedby','editorStatus');
    $('replaceLyrics')?.setAttribute('aria-describedby','editorStatus');
  }
  $('rtSongSearch')?.setAttribute('aria-label','Filter local song queue');
  $('rtU2SongList')?.setAttribute('aria-live','off'); // changes are already announced by #rtU2SongStatus
  // Studio: high-frequency mix first; advanced effects are available without a second dashboard.
  const effects=Array.from(studio.children).find(n=>n.classList.contains('card'));
  const effectPanel=makeDisclosure(effects,'rtU3Effects','Detailed vocal processing','Reverb, echo, dynamics and sound calibration');
  const mixConsole=studio.querySelector('.rt-u2-console');
  if(effectPanel&&mixConsole){
    const action=create('button','rt-u2-action','Advanced effects');action.id='rtU3OpenEffects';action.type='button';action.setAttribute('aria-controls',effectPanel.id);
    mixConsole.querySelector('.rt-u2-strip')?.append(action);
    action.addEventListener('click',()=>toggleAndReveal(effectPanel,effectPanel));
  }
  const record=studio.querySelector('.rt-studio-record');
  if(record){
    record.classList.add('rt-u3-record');
    const intro=create('div','rt-u3-record-intro');intro.innerHTML='<div><span class="rt-u3-kicker">YOUR SESSION / RECORDING</span><h2>Keep the good takes.</h2><p>Your processed microphone can be recorded alone or combined with a local track. Monitoring may stay muted.</p></div><span class="rt-u3-record-safe">LOCAL RECORDING</span>';
    const oldH=record.querySelector(':scope > h2');oldH?.remove();record.prepend(intro);
    const recordingControls=record.querySelector('.record-box .monitor');if(recordingControls)recordingControls.classList.add('rt-u3-record-buttons');
    const take=$('takePanel');
    if(take){
      const head=create('div','rt-u3-take-head');head.innerHTML='<strong>Review your take</strong><p>Listen locally, then download your recording. No upload is performed.</p>';
      take.prepend(head);take.setAttribute('role','region');take.setAttribute('aria-label','Completed recording playback and download');
      $('takePlayer')?.setAttribute('aria-label','Recorded vocal playback');
      $('takeDownload')?.setAttribute('aria-label','Download this recorded vocal take');
    }
    for(const [id,label] of [['recordBtn','Start a processed vocal recording'],['recordAndPlay','Record voice and play selected backing track'],['recordStopBtn','Stop recording and create a downloadable take']])$(id)?.setAttribute('aria-label',label);
    const status=create('p','rt-u3-record-state','Microphone permission and connection are required before recording.');
    status.id='rtU3RecordState';status.setAttribute('role','status');status.setAttribute('aria-live','polite');
    record.querySelector('.record-box')?.append(status);
    const refresh=()=>{let message;
      if(take&&!take.classList.contains('hidden'))message='Take ready. Preview or download using the controls above.';
      else if($('recordStopBtn')&&!$('recordStopBtn').disabled)message='Recording in progress. Stop the take when finished.';
      else message=$('recordBtn')?.disabled?'Start the microphone from Setup to enable recording.':'Ready. Choose voice only or voice with your local track.';
      if(status.textContent!==message)status.textContent=message;
    };
    for(const [el,attrs] of [[take,['class']],[$('recordBtn'),['disabled']],[$('recordStopBtn'),['disabled']]])if(el)new MutationObserver(refresh).observe(el,{attributes:true,attributeFilter:attrs});
    refresh();
  }
  // Setup: safety guidance and device selectors stay visible; optional help and latency tools collapse.
  const about=setup.querySelector('.intro');
  makeDisclosure(about,'rtU3About','About this studio','Local operation, privacy and getting started');
  const lab=setup.querySelector('section.lab');
  const labPanel=makeDisclosure(lab,'rtU3Lab','Calibration and latency','Detailed measurements and diagnostic controls');
  if(labPanel){const openLab=create('button','rt-u2-action','Audio diagnostics');openLab.id='rtU3OpenLab';openLab.type='button';openLab.setAttribute('aria-controls',labPanel.id);$('rtU2GotoQa')?.after(openLab);openLab.addEventListener('click',()=>toggleAndReveal(labPanel,labPanel));}
  for(const id of ['inputSelect','outputSelect'])$(id)?.setAttribute('aria-label',id==='inputSelect'?'Microphone input device':'Speaker output device');
  // Keyboard navigation: these are ordinary native buttons, with additional arrow-key support.
  const nav=[...document.querySelectorAll('.rt-navigation [data-rt-view]')];
  nav.forEach((btn,i)=>{
    btn.addEventListener('keydown',event=>{
      let next=-1;
      if(event.key==='ArrowRight'||event.key==='ArrowDown')next=(i+1)%nav.length;
      else if(event.key==='ArrowLeft'||event.key==='ArrowUp')next=(i+nav.length-1)%nav.length;
      else if(event.key==='Home')next=0;
      else if(event.key==='End')next=nav.length-1;
      if(next>=0){event.preventDefault();nav[next].focus();}
    });
  });
  // Screen reader focus target: use the actual workspace heading instead of an off-screen control.
  for(const id of ['rtSongsHeading','rtStudioHeading','rtSetupHeading'])$(id)?.setAttribute('tabindex','-1');
})();