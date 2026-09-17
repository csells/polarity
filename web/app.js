'use strict';
const $=s=>document.querySelector(s);
const canvas=$('#screen'),ctx=canvas.getContext('2d',{alpha:false});
const keyNames=['left','right','up','down','A','B','start','select'];
const held=new Map();
let core,emu,romPtr,audio,audioNext=0,muted=false,playing=false,paused=false,lastTime=0,raf,frames=0;
let gain;
const keymap={ArrowLeft:'left',KeyA:'left',ArrowRight:'right',KeyD:'right',ArrowUp:'up',KeyW:'up',ArrowDown:'down',KeyS:'down',KeyZ:'A',Space:'A',KeyX:'B',ShiftLeft:'B',ShiftRight:'B',Enter:'start',KeyR:'select'};
function setKey(key,on,source='api'){
 if(!emu)return;
 if(!held.has(key))held.set(key,new Set());
 const set=held.get(key);if(on)set.add(source);else set.delete(source);
 core['_set_joyp_'+key](emu,set.size?1:0);
 document.querySelectorAll(`[data-key="${key}"]`).forEach(b=>b.classList.toggle('active',set.size>0));
}
function release(){for(const key of keyNames){held.set(key,new Set());if(emu)core['_set_joyp_'+key](emu,0);}document.querySelectorAll('.active').forEach(e=>e.classList.remove('active'));}
function render(){const ptr=core._get_frame_buffer_ptr(emu);ctx.putImageData(new ImageData(new Uint8ClampedArray(core.HEAPU8.buffer,ptr,160*144*4),160,144),0,0);}
function audioBuffer(){
 if(!audio||audio.state!=='running'||muted)return;
 const now=audio.currentTime;if(audioNext<now||audioNext>now+.18)audioNext=now+.025;
 const data=new Uint8Array(core.HEAPU8.buffer,core._get_audio_buffer_ptr(emu),core._get_audio_buffer_capacity(emu));
 const b=audio.createBuffer(2,1024,audio.sampleRate);
 for(let c=0;c<2;c++){const out=b.getChannelData(c);for(let i=0;i<1024;i++)out[i]=(data[i*2+c]-128)/128;}
 const src=audio.createBufferSource();src.buffer=b;src.connect(gain);src.start(audioNext);audioNext+=1024/audio.sampleRate;
}
function advance(ticks,sound=true){const until=Math.floor(core._emulator_get_ticks_f64(emu)+ticks);let loops=0;while(core._emulator_get_ticks_f64(emu)<until){const ev=core._emulator_run_until_f64(emu,until);if(ev&1){frames++;}if((ev&2)&&sound)audioBuffer();if(ev&16)throw Error('Invalid opcode in cartridge');if(ev&4)break;if(++loops>10000)throw Error('Emulator failed to advance');}render();}
function loop(now){raf=requestAnimationFrame(loop);if(!playing||paused){lastTime=now;return;}const dt=lastTime?Math.min((now-lastTime)/1000,.05):0;lastTime=now;advance(dt*4194304);pollGamepad();}
function pulse(key){setKey(key,true,'pulse');setTimeout(()=>setKey(key,false,'pulse'),100);}
async function start(){
 if(!emu||playing)return;
 try{audio=new (window.AudioContext||window.webkitAudioContext)({sampleRate:48000});gain=audio.createGain();gain.gain.value=.18;gain.connect(audio.destination);await audio.resume();}
 catch(error){console.warn('Sound unavailable:',error);$('#sound span').textContent='NO AUDIO';}
 playing=true;$('#play').hidden=true;$('#loading').hidden=true;lastTime=0;pulse('start');
}
function setPaused(value){if(!playing)return;paused=value;release();$('#pause').firstChild.textContent=value?'▶':'Ⅱ';$('#pause span').textContent=value?'RESUME':'PAUSE';$('#status').innerHTML=value?'Ⅱ PAUSED':'<i></i> NATIVE GBC · 160 × 144';if(audio){if(value)audio.suspend();else if(!muted)audio.resume().catch(console.error);}lastTime=0;}
$('#play').addEventListener('click',start);
window.addEventListener('keydown',e=>{const key=keymap[e.code];if(!key)return;e.preventDefault();if(e.repeat)return;if(!playing){start();return;}if(key==='start'){setPaused(!paused);return;}if(!paused)setKey(key,true,e.code);});
window.addEventListener('keyup',e=>{const key=keymap[e.code];if(key){e.preventDefault();setKey(key,false,e.code);}});
const pointers=new Map();
function pointKey(x,y){const e=document.elementFromPoint(x,y);return e?.closest('[data-key]')?.dataset.key;}
for(const b of document.querySelectorAll('.action [data-key]')){
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
window.addEventListener('gamepaddisconnected',release);
$('#pause').addEventListener('click',()=>setPaused(!paused));
$('#retry').addEventListener('click',()=>{if(playing){setPaused(false);pulse('select');}});
$('#sound').addEventListener('click',()=>{muted=!muted;$('#sound').setAttribute('aria-pressed',String(muted));$('#sound span').textContent=muted?'SOUND OFF':'SOUND ON';if(gain)gain.gain.value=muted?0:.18;if(audio&&!muted&&!paused)audio.resume().catch(console.error);});
$('#fullscreen').addEventListener('click',async()=>{try{if(document.fullscreenElement)await document.exitFullscreen();else if($('.play-section').requestFullscreen)await $('.play-section').requestFullscreen();else{$('.play-section').scrollIntoView({block:'start',behavior:'smooth'});$('#status').textContent='TIP: ROTATE YOUR PHONE TO LANDSCAPE';}}catch(e){$('#status').textContent='FULLSCREEN UNAVAILABLE ON THIS BROWSER';}});
window.addEventListener('blur',()=>{release();setPaused(true);});
document.addEventListener('visibilitychange',()=>{if(document.hidden){release();setPaused(true);}});
function pollGamepad(){const pad=navigator.getGamepads?.()[0];if(!pad)return;const buttons={A:0,B:1,select:8,left:14,right:15,up:12,down:13};for(const [key,n]of Object.entries(buttons)){let on=pad.buttons[n]?.pressed||false;if(key==='left')on ||=pad.axes[0]<-.4;if(key==='right')on ||=pad.axes[0]>.4;if(key==='up')on ||=pad.axes[1]<-.4;if(key==='down')on ||=pad.axes[1]>.4;setKey(key,on,'gamepad');}}
(async()=>{try{
 const result=await Promise.all([Binjgb({locateFile:p=>'vendor/'+p}),fetch('polarity.gbc')]);core=result[0];if(!result[1].ok)throw Error('Cartridge download failed');const rom=new Uint8Array(await result[1].arrayBuffer());romPtr=core._malloc(rom.length);core.HEAPU8.set(rom,romPtr);emu=core._emulator_new_simple(romPtr,rom.length,48000,1024,0);if(!emu)throw Error('Invalid cartridge');const joy=core._joypad_new();core._emulator_set_default_joypad_callback(emu,joy);advance(4194304/2,false);$('#loading').hidden=true;$('#play').hidden=false;raf=requestAnimationFrame(loop);
 // Deterministic harness operates the same emulator and cartridge as the UI.
 window.polarity={get core(){return core},get emu(){return emu},setKey,release,advance,render,get frames(){return frames},get playing(){return playing},get paused(){return paused},freeze(){cancelAnimationFrame(raf)},read(addr){return core._emulator_read_mem(emu,addr)},write(addr,v){core._emulator_write_mem(emu,addr,v)}};
 }catch(e){$('#load-status').textContent='COULD NOT LOAD — PLEASE RELOAD';console.error(e);$('#status').textContent=e.message;}})();
