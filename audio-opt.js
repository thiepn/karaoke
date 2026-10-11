/* ROOMTONE A1: additive, local-only calibration and microphone-specific vocal profiles.
   Does not obtain audio, enable monitoring or change the DSP/recording graph. */
(()=>{
  'use strict';
  const $=id=>document.getElementById(id);
  const setup=$('rtSetupContent'),input=$('inputSelect'),monitor=$('monitorBtn');
  if(!setup||!input||!monitor||$('rtAoGuide'))return;
  const root=document.createElement('section');
  root.id='rtAoGuide';root.className='rt-ao-guide card';
  root.setAttribute('aria-label','Microphone first-use calibration');
  root.innerHTML='<div><span class="smallcaps">AUDIO / FIRST-USE</span><h2>Calibrate your microphone</h2>'+
    '<p class="rt-ao-help">Profiles and calibration remain on this device. Speaker monitoring always requires your explicit action.</p></div>'+
    '<p id="rtAoDevice" class="rt-ao-help"></p>'+
    '<div class="rt-ao-actions"><button type="button" id="rtAoNatural" class="btn">Natural voice</button>'+
    '<button type="button" id="rtAoHandheld" class="btn">Handheld starting sound</button>'+
    '<button type="button" id="rtAoSave" class="btn">Save for this microphone</button>'+
    '<button type="button" id="rtAoLoad" class="btn">Load this microphone</button></div>'+
    '<div class="rt-ao-checks">'+
    '<label><input type="checkbox" data-ao-check="raw"> Raw mic signal checked without clipping</label>'+
    '<label><input type="checkbox" data-ao-check="routing"> Correct speaker routing; monitor starts muted</label>'+
    '<label><input type="checkbox" data-ao-check="dry"> Dry live voice at low level without feedback</label>'+
    '<label><input type="checkbox" data-ao-check="wet"> Short reverb / echo without unstable repeats</label>'+
    '<label><input type="checkbox" data-ao-check="record"> Real downloaded recording plays with expected sound</label></div>'+
    '<label class="rt-ao-select" for="rtAoLatency">Perceived live-voice delay — your observation, NOT measured milliseconds'+
    '<select id="rtAoLatency"><option value="not-tested">Not tested</option>'+
    '<option value="comfortable">Comfortable</option><option value="noticeable">Noticeable but tolerable</option>'+
    '<option value="unusable">Distracting or unusable</option></select></label>'+
    '<p class="rt-ao-help">Use USB/AUX speakers, never Bluetooth, and start with monitoring OFF and volume low. Press Esc if feedback occurs. The browser buffer estimate is not microphone-to-speaker latency.</p>'+
    '<div class="rt-ao-actions"><button type="button" id="rtAoOpenLab" class="btn">Open sound check</button>'+
    '<button type="button" id="rtAoReport" class="btn">Download manual report</button></div>'+
    '<p id="rtAoStatus" role="status" aria-live="polite">No physical device measurements have been independently verified.</p>';
  const lab=$('rtU3Lab');
  if(lab&&lab.parentNode===setup)setup.insertBefore(root,lab);
  else setup.append(root);
  const status=$('rtAoStatus');
  const fields=['micGain','dynamics','voiceTone','reverb','roomSize','reverbTone','echo','echoFeedback','clarity'];
  const numeric={micGain:[20,140],voiceTone:[0,100],reverb:[0,100],reverbTone:[0,100],echo:[0,100],echoFeedback:[0,35]};
  const natural={micGain:100,dynamics:'balanced',voiceTone:55,reverb:17,roomSize:'studio',reverbTone:45,echo:0,echoFeedback:0,clarity:false};
  const handheld={micGain:100,dynamics:'balanced',voiceTone:50,reverb:14,roomSize:'studio',reverbTone:45,echo:0,echoFeedback:0,clarity:false};
  const device=()=>({id:input.value||'default',label:input.selectedOptions[0]?.textContent?.trim()||'System default microphone'});
  const key=()=> 'roomtone:audio-opt:mic:v1:'+device().id;
  const monitoring=()=> /^Mute voice/i.test(monitor.textContent||'');
  const message=s=>{status.textContent=s;};
  const deviceText=()=>{$('rtAoDevice').textContent='Selected input: '+device().label+'. Profiles require manual loading; they never automatically enable monitoring.';};
  function apply(values){
    if(monitoring()){message('Mute voice in speakers first, then apply a profile to avoid unexpected changes in level.');return false;}
    for(const field of fields){
      const el=$(field),v=values[field];if(!el||v===undefined)continue;
      if(el.type==='checkbox'){if(typeof v!=='boolean')continue;el.checked=v;}
      else if(el.tagName==='SELECT'){if(typeof v!=='string'||![...el.options].some(o=>o.value===v))continue;el.value=v;}
      else {if(typeof v!=='number'||!Number.isFinite(v))continue;const [min,max]=numeric[field];el.value=String(Math.min(max,Math.max(min,v)));}
      el.dispatchEvent(new Event('input',{bubbles:true}));el.dispatchEvent(new Event('change',{bubbles:true}));
    }
    return true;
  }
  function snapshot(){return Object.fromEntries(fields.map(f=>{const e=$(f);return [f,e.type==='checkbox'?e.checked:e.tagName==='SELECT'?e.value:Number(e.value)];}));}
  $('rtAoNatural').addEventListener('click',()=>{if(apply(natural))message('Natural short-room vocal sound selected. Compare with dry voice.');});
  $('rtAoHandheld').addEventListener('click',()=>{if(apply(handheld))message('Handheld starting sound selected. Check raw input level before raising gain.');});
  $('rtAoSave').addEventListener('click',()=>{
    try{localStorage.setItem(key(),JSON.stringify({version:1,values:snapshot()}));message('Saved locally for this microphone. Live monitoring unchanged.');}
    catch(e){message('Profile could not be saved: '+String(e.message||e));}
  });
  $('rtAoLoad').addEventListener('click',()=>{
    try{
      const profile=JSON.parse(localStorage.getItem(key())||'null');
      if(!profile||profile.version!==1||!profile.values||typeof profile.values!=='object'){message('No sound profile saved for this microphone.');return;}
      if(apply(profile.values))message('Profile loaded. Speaker monitoring remains muted.');
    }catch(e){message('Saved profile is invalid: '+String(e.message||e));}
  });
  input.addEventListener('change',()=>{
    deviceText();root.querySelectorAll('[data-ao-check]').forEach(e=>e.checked=false);
    $('rtAoLatency').value='not-tested';
    message('Microphone changed. Repeat the physical checks; no profile was automatically applied.');
  });
  deviceText();
  $('rtAoOpenLab').addEventListener('click',()=>{
    $('rtU3Lab')?.setAttribute('open','');
    const target=$('noiseTest')||$('rtU3Lab');
    target?.scrollIntoView({behavior:'instant',block:'start'});
    message('Start microphone, measure room noise and voice, then test dry monitoring at a low volume.');
  });
  $('rtAoReport').addEventListener('click',()=>{
    const names=['raw','routing','dry','wet','record'];
    const checks=Object.fromEntries(names.map(n=>[n,!!root.querySelector('[data-ao-check="'+n+'"]')?.checked]));
    const delay=$('rtAoLatency').value;
    const report={
      schema:'roomtone-manual-device-check/v1',
      kind:'user-observations-not-automated-measurement',
      time:new Date().toISOString(),
      microphone:device().label,
      speaker:$('outputSelect')?.selectedOptions[0]?.textContent?.trim()||'System default',
      echoCancellation:!!$('aec')?.checked,monitoringAtExport:monitoring(),
      checks,perceivedDelay:delay,
      allChecksUserMarked:Object.values(checks).every(Boolean)&&delay!=='not-tested',
      hardwareQualifiedByApplication:false,
      warning:'A user-entered report is not independently verified or measured microphone-to-speaker latency.'
    };
    const url=URL.createObjectURL(new Blob([JSON.stringify(report,null,2)+'\n'],{type:'application/json'}));
    const anchor=document.createElement('a');anchor.href=url;anchor.download='roomtone-manual-device-check.json';
    document.body.append(anchor);anchor.click();anchor.remove();
    setTimeout(()=>URL.revokeObjectURL(url),2000);
    message('User-reported observations downloaded. Independent physical hardware qualification remains pending.');
  });
})();