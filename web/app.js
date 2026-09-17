'use strict';
const $=s=>document.querySelector(s);
const canvas=$('#screen'),ctx=canvas.getContext('2d',{alpha:false});
const keyNames=['left','right','up','down','A','B','start','select','L','R'];
const gbaKeys={A:1,B:2,select:4,start:8,right:16,left:32,up:64,down:128,R:256,L:512};
let emulatedTicks=0,frameRemainder=0,audioPtr,statePtr;
let desiredMask=0,queuedMask=0,inputFlushPending=false;
const inputQueue=[];
function flushInput(){if(desiredMask!==queuedMask){inputQueue.push(desiredMask);queuedMask=desiredMask;}inputFlushPending=false;}
function updateKeys(){
 desiredMask=0;for(const [k,v] of held)if(v.size)desiredMask|=gbaKeys[k];
 // Coalesce a direction+button event, but preserve press/release edges between
 // events so a quick tap cannot disappear between emulated input polls.
 if(!inputFlushPending){inputFlushPending=true;queueMicrotask(flushInput);}
}
function snapshot(){if(!core._mgbawasm_state_save(statePtr))throw Error('Could not capture emulator state');return core.HEAPU8.slice(statePtr,statePtr+core._mgbawasm_state_size());}
function memoryOffset(addr){if(addr>=0x02000000&&addr<0x02040000)return 0x21000+addr-0x02000000;if(addr>=0x03000000&&addr<0x03008000)return 0x19000+addr-0x03000000;throw Error('Unsupported debug address');}
const held=new Map(),pulses=new Map();
let core,emu,romPtr,audio,audioNext=0,muted=false,playing=false,paused=false,lastTime=0,clockTimer,frames=0;
let gain,frameImage,frameMemory,framePointer,failed=false,emulatorOperation='startup';
const audioSources=new Set(),maxAudioSources=12;
function clearAudioQueue(){
 for(const src of audioSources){src.onended=null;src.stop();src.disconnect();}
 audioSources.clear();audioNext=0;
}
function failGame(error){
 diagnostics.failure(error);
 if(failed)return;failed=true;paused=true;clearTimeout(clockTimer);
 console.error('Polarity stopped:',error);release();cartridgeSave();
 clearAudioQueue();if(audio)audio.suspend().catch(console.error);
 const report=JSON.stringify({time:new Date().toISOString(),message:String(error?.message||error),stack:String(error?.stack||'').slice(0,1600),frames,wasmBytes:core?.HEAPU8?.length,audioState:audio?.state,browser:navigator.userAgent,screen:[innerWidth,innerHeight]},null,2);
 $('#error-report').textContent=report;
 try{localStorage.setItem('polarity-last-error',report);}catch(storageError){console.warn('Error report could not be stored:',storageError);}
 $('#recovery').hidden=false;$('#status').textContent='GAME STOPPED';$('#reload-game').focus();
}
let controllerDevice=null,controllerSetup=null,controllerProfiles={};
try{const saved=JSON.parse(localStorage.getItem(ControllerProfile.storageKey)||'{}');if(saved&&typeof saved==='object'&&!Array.isArray(saved))controllerProfiles=saved;}catch(error){console.warn('Saved controller layouts unavailable:',error);}
let pageActive=!document.hidden,controllerId=null,controllerButtons=new Set(),gamepadBlocked=false,controllerStartHeld=false,controllerResync=false;
const keymap={ArrowLeft:'left',KeyA:'left',ArrowRight:'right',KeyD:'right',ArrowUp:'up',KeyW:'up',ArrowDown:'down',KeyS:'down',KeyZ:'A',Space:'A',KeyX:'B',ShiftLeft:'B',ShiftRight:'B',Enter:'start',KeyR:'select',KeyQ:'L',KeyE:'R'};
const saveKey='polarity-advance-save-v2';
let lastSave='',saveTime=0;
function cartridgeSave(load=false){
 if(!emu)return;
 try{
  if(load){
   const saved=localStorage.getItem(saveKey);if(!saved)return;
   const bytes=Uint8Array.from(atob(saved),c=>c.charCodeAt(0));
   if(bytes.length!==32768||bytes[0]!==0x41||bytes[1]!==2)throw Error('Invalid Advance save');
   const ptr=core._malloc(bytes.length);try{core.HEAPU8.set(bytes,ptr);if(!core._mgbawasm_sram_load(ptr,bytes.length))throw Error('Could not load SRAM');}finally{core._free(ptr);}
   lastSave=saved;
  }else{
   const size=core._mgbawasm_sram_save(),ptr=core._mgbawasm_sram_ptr();if(!size)return;
   const bytes=core.HEAPU8.subarray(ptr,ptr+size);if(bytes[0]!==0x41)return;
   const saved=btoa(String.fromCharCode(...bytes));
   if(saved!==lastSave){localStorage.setItem(saveKey,saved);lastSave=saved;$('#save-status').textContent='Progress saved here · checkpoints, deliveries & letters';}
  }
 }catch(error){console.warn('Progress save unavailable:',error);$('#save-status').textContent='Progress could not be saved. Keep this tab open to continue.';}
}
function openMap(){if(!playing||controllerSetup)return;setPaused(false);pulse('L');}
window.addEventListener('pagehide',()=>cartridgeSave());
document.addEventListener('visibilitychange',()=>{if(document.hidden)cartridgeSave();});
function setKey(key,on,source='api'){
 if(!emu||failed)return;
 if(!held.has(key))held.set(key,new Set());
 const set=held.get(key);if(on)set.add(source);else set.delete(source);
 updateKeys();
 document.querySelectorAll(`[data-key="${key}"]`).forEach(b=>b.classList.toggle('active',set.size>0));
}
function release(){pulses.clear();for(const key of keyNames)held.set(key,new Set());desiredMask=queuedMask=0;inputQueue.length=0;inputQueue.push(0);if(emu&&!failed)core._mgbawasm_set_keys(0);document.querySelectorAll('.active').forEach(e=>e.classList.remove('active'));}
function render(){
 const ptr=core._mgbawasm_video_ptr(),memory=core.HEAPU8.buffer;
 if(memory!==frameMemory||ptr!==framePointer){frameMemory=memory;framePointer=ptr;frameImage=new ImageData(new Uint8ClampedArray(memory,ptr,240*160*4),240,160);}
 ctx.putImageData(frameImage,0,0);
}
function audioBuffer(sound=true){
 let count;while((count=core._mgbawasm_read_audio(audioPtr,2048))>0){
  if(!sound||!audio||audio.state!=='running'||muted)continue;
  const now=audio.currentTime;if(audioNext<now||audioNext>now+.15)audioNext=now+.018;
  const data=core.HEAP16.subarray(audioPtr/2,audioPtr/2+count*2),rate=core._mgbawasm_sample_rate();
  const b=audio.createBuffer(2,count,rate);
  for(let c=0;c<2;c++){const out=b.getChannelData(c);for(let i=0;i<count;i++)out[i]=data[i*2+c]/32768;}
  // Some output devices stall before AudioContext reports suspension. Bound
  // pending sources even then, and explicitly detach completed native nodes.
  while(audioSources.size>=maxAudioSources){const old=audioSources.values().next().value;old.onended=null;old.stop();old.disconnect();audioSources.delete(old);}
  const src=audio.createBufferSource();src.buffer=b;src.connect(gain);
  src.onended=()=>{src.disconnect();audioSources.delete(src);};
  src.start(audioNext);audioSources.add(src);audioNext+=count/rate;
 }
}
function advance(ticks,sound=true){
 const before=frames;frameRemainder+=ticks;
 while(frameRemainder>=70224){
  emulatorOperation='input';
  flushInput();core._mgbawasm_set_keys(inputQueue.length?inputQueue.shift():desiredMask);
  emulatorOperation='frame';core._mgbawasm_run_frame();frames++;emulatedTicks+=70224;frameRemainder-=70224;
  emulatorOperation='audio';audioBuffer(sound);
  for(const [key,until] of pulses)if(emulatedTicks>=until){setKey(key,false,'pulse');pulses.delete(key);}
 }
 if(frames!==before){emulatorOperation='render';render();}emulatorOperation='idle';
}
// Advance simulation independently of throttled/occluded animation callbacks.
// Hidden pages pause explicitly; a visible page retains native input cadence.
function loop(now){
 if(failed||document.hidden||!pageActive)return;
 try{
  pollGamepad();
  if(!playing||paused||controllerSetup)lastTime=now;
  else{const dt=lastTime?Math.min((now-lastTime)/1000,.1):0;lastTime=now;advance(dt*4194304);if(now-saveTime>1000){saveTime=now;cartridgeSave();}}
  diagnostics.tick(now);clockTimer=setTimeout(()=>loop(performance.now()),8);
 }catch(error){failGame(error);}
}
function wakeLoop(){clearTimeout(clockTimer);lastTime=0;if(emu&&!failed&&!document.hidden&&pageActive)clockTimer=setTimeout(()=>loop(performance.now()),8);}
function pulse(key){setKey(key,true,'pulse');pulses.set(key,emulatedTicks+280896);}
function updateSoundButton(){
 $('#sound').setAttribute('aria-pressed',String(muted));
 $('#sound span').textContent=muted?'SOUND OFF':audio&&audio.state!=='running'&&!paused?'ENABLE SOUND':'SOUND ON';
}
function start(){
 if(!emu||playing||controllerSetup)return;
 // Gamepad buttons do not always unlock browser audio. Never block play on resume().
 playing=true;$('#play').hidden=true;$('#loading').hidden=true;lastTime=0;pulse('start');
 try{
  audio=new (window.AudioContext||window.webkitAudioContext)({sampleRate:48000});
  gain=audio.createGain();gain.gain.value=muted?0:.65;gain.connect(audio.destination);
  audio.addEventListener('statechange',()=>{updateSoundButton();diagnostics.event('audio-statechange');diagnostics.persist();});
  audio.resume().catch(error=>{console.warn('Sound unavailable:',error);updateSoundButton();});
  updateSoundButton();
 }catch(error){console.warn('Sound unavailable:',error);$('#sound span').textContent='NO AUDIO';}
}
function setPaused(value){if(!playing||failed)return;paused=value;release();clearAudioQueue();$('#pause').firstChild.textContent=value?'▶':'Ⅱ';$('#pause span').textContent=value?'RESUME':'PAUSE';$('#status').innerHTML=value?'Ⅱ PAUSED':'<i></i> NATIVE GBA · 240 × 160';if(audio){if(value)audio.suspend().catch(console.error);else if(!muted)audio.resume().catch(console.error);}lastTime=0;}
$('#reload-game').addEventListener('click',()=>location.reload());
$('#copy-error').addEventListener('click',async()=>{try{await navigator.clipboard.writeText($('#error-report').textContent);$('#copy-error').textContent='Report copied';}catch(error){$('#error-details').open=true;$('#copy-error').textContent='Select and copy the report below';}});
$('#play').addEventListener('click',start);
window.addEventListener('keydown',e=>{if(failed)return;if(e.code==='KeyM'){e.preventDefault();if(!e.repeat)openMap();return;}const key=keymap[e.code];if(!key)return;e.preventDefault();if(e.repeat||controllerSetup)return;if(!playing){start();return;}if(key==='start'){setPaused(!paused);return;}if(!paused)setKey(key,true,e.code);});
window.addEventListener('keyup',e=>{const key=keymap[e.code];if(key){e.preventDefault();setKey(key,false,e.code);}});
const pointers=new Map();
function pointKey(x,y){const e=document.elementFromPoint(x,y);return e?.closest('[data-key]')?.dataset.key;}
for(const b of document.querySelectorAll('.action [data-key], .shoulder-dash')){
 b.addEventListener('contextmenu',e=>e.preventDefault());
 b.addEventListener('pointerdown',e=>{e.preventDefault();if(!playing){start();return;}if(paused)return;b.setPointerCapture(e.pointerId);pointers.set(e.pointerId,b.dataset.key);setKey(b.dataset.key,true,e.pointerId);});
 b.addEventListener('pointermove',e=>{if(!pointers.has(e.pointerId))return;const old=pointers.get(e.pointerId);if(!['left','right','up','down'].includes(old))return;const k=pointKey(e.clientX,e.clientY);if(k&&['left','right','up','down'].includes(k)&&k!==old){setKey(old,false,e.pointerId);setKey(k,true,e.pointerId);pointers.set(e.pointerId,k);}});
 const end=e=>{const k=pointers.get(e.pointerId);if(k)setKey(k,false,e.pointerId);pointers.delete(e.pointerId);};
 b.addEventListener('pointerup',end);b.addEventListener('pointercancel',end);b.addEventListener('lostpointercapture',end);
}
// A thumb can aim diagonally across the entire D-pad, including its corners.
const pad=$('#dpad');
function padMove(e){
 const rect=pad.getBoundingClientRect(),x=(e.clientX-rect.left)/rect.width*2-1,y=(e.clientY-rect.top)/rect.height*2-1;
 const next=[];if(x<-.25)next.push('left');if(x>.25)next.push('right');if(y<-.25)next.push('up');if(y>.25)next.push('down');
 for(const k of ['left','right','up','down'])setKey(k,next.includes(k),'pad'+e.pointerId);
}
pad.addEventListener('pointerdown',e=>{e.preventDefault();if(!playing){start();return;}if(paused)return;pad.setPointerCapture(e.pointerId);pointers.set(e.pointerId,'pad');padMove(e);});
pad.addEventListener('pointermove',e=>{if(pointers.get(e.pointerId)==='pad')padMove(e);});
function endPad(e){for(const k of ['left','right','up','down'])setKey(k,false,'pad'+e.pointerId);pointers.delete(e.pointerId);}
for(const type of ['pointerup','pointercancel','lostpointercapture'])pad.addEventListener(type,endPad);
pad.addEventListener('contextmenu',e=>e.preventDefault());
window.addEventListener('gamepaddisconnected',()=>pollGamepad());
$('#map').addEventListener('click',openMap);
$('#shoulder-map').addEventListener('click',openMap);
$('#pause').addEventListener('click',()=>setPaused(!paused));
$('#retry').addEventListener('click',()=>{if(playing){setPaused(false);pulse('select');}});
$('#sound').addEventListener('click',()=>{
 if(audio&&audio.state!=='running'&&!muted&&!paused){audio.resume().then(updateSoundButton).catch(console.error);return;}
 muted=!muted;clearAudioQueue();if(gain)gain.gain.value=muted?0:.65;
 if(audio&&!muted&&!paused)audio.resume().catch(console.error);
 updateSoundButton();
});
$('#fullscreen').addEventListener('click',async()=>{try{if(document.fullscreenElement)await document.exitFullscreen();else if($('.play-section').requestFullscreen)await $('.play-section').requestFullscreen();else{$('.play-section').scrollIntoView({block:'start',behavior:'smooth'});$('#status').textContent='TIP: ROTATE YOUR PHONE TO LANDSCAPE';}}catch(e){$('#status').textContent='FULLSCREEN UNAVAILABLE ON THIS BROWSER';}});
window.addEventListener('blur',()=>{pageActive=false;controllerResync=true;release();setPaused(true);clearTimeout(clockTimer);});
window.addEventListener('focus',()=>{pageActive=!document.hidden;wakeLoop();});
document.addEventListener('visibilitychange',()=>{pageActive=!document.hidden&&document.hasFocus();if(document.hidden){controllerResync=true;release();setPaused(true);clearTimeout(clockTimer);}else wakeLoop();});
document.addEventListener('freeze',()=>{controllerResync=true;setPaused(true);clearTimeout(clockTimer);});
document.addEventListener('resume',()=>{pageActive=!document.hidden&&document.hasFocus();wakeLoop();});
window.addEventListener('pageshow',()=>{pageActive=!document.hidden&&document.hasFocus();wakeLoop();});
function controllerMessage(message){const el=$('#controller-status');if(el.textContent!==message)el.textContent=message;}
function customProfile(p){const value=controllerProfiles[ControllerProfile.deviceKey(p)];return ControllerProfile.valid(p,value)?value:null;}
function saveProfiles(){
 try{localStorage.setItem(ControllerProfile.storageKey,JSON.stringify(controllerProfiles));return true;}
 catch(error){console.warn('Could not save controller layout:',error);return false;}
}
const setupLabels=['UP','DOWN','LEFT','RIGHT','JUMP (SNES B)','DASH (SNES Y or A)','PAUSE (START)','RETRY (SELECT)'];
function setupPrompt(message){$('#controller-setup-prompt').textContent=message;}
function endControllerSetup(message){controllerSetup=null;$('#controller-cancel').hidden=true;$('#controller-configure').disabled=false;setupPrompt(message);controllerButtons=new Set();}
$('#controller-configure').addEventListener('click',()=>{
 if(!controllerDevice){setupPrompt('Connect your controller and press a button first.');return;}
 setPaused(true);release();
 controllerSetup={device:ControllerProfile.deviceKey(controllerDevice),base:ControllerProfile.neutral(controllerDevice),step:0,waiting:true,bindings:{}};
 $('#controller-cancel').hidden=false;$('#controller-configure').disabled=true;
 setupPrompt('Release all controls to begin.');
});
$('#controller-cancel').addEventListener('click',()=>endControllerSetup('Setup cancelled. Your previous layout is unchanged.'));
$('#controller-reset').addEventListener('click',()=>{
 if(!controllerDevice)return;
 delete controllerProfiles[ControllerProfile.deviceKey(controllerDevice)];const saved=saveProfiles();
 setPaused(true);release();endControllerSetup(saved?'Custom layout cleared.': 'Layout cleared for this visit; browser storage is unavailable.');
});
function learnController(p){
 const setup=controllerSetup;
 if(!pageActive)return;
 if(setup.waiting){
  if(!ControllerProfile.released(p,setup.base))return;
  setup.base=ControllerProfile.neutral(p);setup.waiting=false;
  if(setup.step===ControllerProfile.actions.length){
   controllerProfiles[setup.device]=setup.bindings;const saved=saveProfiles();
   endControllerSetup(saved?'Layout saved on this PC. Press START to play.':'Layout ready for this visit. Press START to play. Browser storage is unavailable.');return;
  }
  setupPrompt((setup.step+1)+'/8 — Press '+setupLabels[setup.step]);return;
 }
 const binding=ControllerProfile.capture(p,setup.base);if(!binding)return;
 if(Object.values(setup.bindings).some(b=>JSON.stringify(b)===JSON.stringify(binding))){setupPrompt('That control is already assigned. Release it and choose another.');return;}
 setup.bindings[ControllerProfile.actions[setup.step]]=binding;setup.step++;setup.waiting=true;setupPrompt('Release the control to continue.');
}
function readController(p,profile){
 if(profile)return Object.fromEntries(ControllerProfile.actions.map(key=>[key,ControllerProfile.matches(p,profile[key])]));
 if(p.mapping!=='standard')return null;
 const pressed=i=>!!p.buttons[i]?.pressed,x=p.axes[0]||0,y=p.axes[1]||0;
 return {left:pressed(14)||x<-.3,right:pressed(15)||x>.3,up:pressed(12)||y<-.3,down:pressed(13)||y>.3,A:pressed(0),B:pressed(1)||pressed(2),start:pressed(9),select:pressed(8),L:pressed(4),R:pressed(5)};
}
function pollGamepad(){
 if(gamepadBlocked||!pageActive||document.hidden)return;
 if(!navigator.getGamepads){controllerMessage('Controller input is unavailable in this browser.');return;}
 let pads;
 try{pads=Array.from(navigator.getGamepads()).filter(p=>p&&p.connected);}
 catch(error){gamepadBlocked=true;controllerMessage('Controller access is blocked by this browser.');console.warn('Gamepad access:',error);return;}
 // Browsers can leave holes in this array after devices are disconnected.
 const supported=pads.filter(p=>p.mapping==='standard'||customProfile(p));
 const identity=p=>p.index+':'+p.id;
 const controller=pads.find(p=>identity(p)===controllerId)||supported[0]||pads[0];
 controllerDevice=controller||null;
 const nextId=controller?identity(controller):null;
 if(nextId!==controllerId){
  for(const key of ['left','right','up','down','A','B','L','R'])setKey(key,false,'gamepad');
  if(controllerId&&playing)setPaused(true);
  controllerId=nextId;controllerButtons=new Set();
  if(controllerSetup)endControllerSetup('Controller changed or disconnected. Connect it and restart setup.');
 }
 if(!controller){
  controllerResync=false;
  controllerMessage('Connect a controller, then press a button.');
  return;
 }
 if(controllerSetup&&ControllerProfile.deviceKey(controller)!==controllerSetup.device)endControllerSetup('Controller mode changed. Restart setup for this mode.');
 if(controllerSetup){learnController(controller);return;}
 const profile=customProfile(controller),keys=readController(controller,profile);
 if(!keys){controllerMessage('Controller detected — choose Set up controller below.');return;}
 const snes=/8bitdo|sn30|sf30|sfc30/i.test(controller.id);
 controllerMessage(profile?'Controller connected · Custom layout ready':snes?'8BitDo connected · B: jump · Y / A: dash · START: pause':'Controller connected · Start / Options to pause');
 const buttons=new Set(Object.keys(keys).filter(key=>keys[key]));
 // Polling sleeps in the background. Treat buttons already held on return as
 // existing holds, not new presses that could accidentally resume the game.
 if(controllerResync){controllerButtons=buttons;controllerResync=false;}
 if(controllerStartHeld&&buttons.size===0)controllerStartHeld=false;
 const pressed=key=>buttons.has(key)&&!controllerButtons.has(key);
 let transition=false;
 if(pageActive){
  if(!playing&&(pressed('A')||pressed('start'))){start();controllerStartHeld=true;transition=true;}
  else if(playing&&pressed('start')){setPaused(!paused);transition=true;}
  else if(playing&&!paused&&pressed('select')){if(keys.up)pulse('L');else pulse('select');transition=true;}
 }
 controllerButtons=buttons;
 delete keys.start;delete keys.select;
 // Opposing D-pad / stick directions cancel, so a drifting stick cannot pick a side.
 if(keys.left&&keys.right)keys.left=keys.right=false;
 if(keys.up&&keys.down)keys.up=keys.down=false;
 const enabled=pageActive&&playing&&!paused&&!transition&&!controllerStartHeld;
 for(const [key,on]of Object.entries(keys))setKey(key,enabled&&on,'gamepad');
}
const diagnostics=new PolarityDiagnostics({
 status:()=>({frames,playing,paused,failed,emulatorOperation,frameRemainder,inputMask:desiredMask,queuedInputs:inputQueue.length,viewport:[innerWidth,innerHeight],
  wasmBytes:core?.HEAPU8?.length||0,jsHeapBytes:performance.memory?.usedJSHeapSize||null,
  audioState:audio?.state||'not-created',audioTime:audio?.currentTime||0,queuedAudio:audioSources.size,
  controller:controllerDevice?{mapping:controllerDevice.mapping,buttons:controllerDevice.buttons.length,axes:controllerDevice.axes.length}:null}),
 capture:()=>emu&&!failed?{state:snapshot(),sram:lastSave}:null
});
(async()=>{try{
 const result=await Promise.all([createMgbaModule({locateFile:p=>'vendor/mgba/'+p}),fetch('polarity.gba')]);core=result[0];
 if(!result[1].ok)throw Error('Cartridge download failed');
 const rom=new Uint8Array(await result[1].arrayBuffer());
 const hash=await crypto.subtle.digest('SHA-256',rom);
 diagnostics.current.cartridge={bytes:rom.length,sha256:Array.from(new Uint8Array(hash),b=>b.toString(16).padStart(2,'0')).join('')};
 romPtr=core._malloc(rom.length);core.HEAPU8.set(rom,romPtr);
 core._mgbawasm_init();core._mgbawasm_set_log_level(0);
 if(!core._mgbawasm_load(romPtr,rom.length,0,0,0,0,1))throw Error('Invalid Advance cartridge');
 core._free(romPtr);emu=1;core._mgbawasm_set_idle_optimization(1);
 audioPtr=core._malloc(8192);statePtr=core._malloc(core._mgbawasm_state_size());
 // Let mGBA detect SRAM from the cartridge before restoring it, then reboot.
 advance(70224*8,false);cartridgeSave(true);core._mgbawasm_reset();advance(70224*8,false);$('#loading').hidden=true;$('#play').hidden=false;diagnostics.checkpoint('cartridge-ready');wakeLoop();
 window.polarity={get core(){return core},get emu(){return emu},setKey,release,advance,render,save:cartridgeSave,snapshot,get frames(){return frames},get playing(){return playing},get paused(){return paused},freeze(){clearTimeout(clockTimer)},read(addr){return snapshot()[memoryOffset(addr)]},write(addr,v){const b=snapshot();b[memoryOffset(addr)]=v;core.HEAPU8.set(b,statePtr);if(!core._mgbawasm_state_load(statePtr))throw Error('State write failed');}};
 }catch(e){diagnostics.failure(e,'startup-error');$('#load-status').textContent='COULD NOT LOAD — PLEASE RELOAD';console.error(e);$('#status').textContent=e.message;}})();
