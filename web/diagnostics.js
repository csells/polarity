'use strict';
// Local, bounded flight recorder. No network uploads or unrelated browser data.
class PolarityDiagnostics {
 constructor({status,capture}){
  this.key='polarity-diagnostics-v1';this.status=status;this.capture=capture;
  this.cached=[];this.storageError=null;this.lastSample=0;this.lastSnapshot=0;
  this.progress=null;this.stallActive=false;this.watchTimer=null;
  this.current={schema:1,id:crypto.randomUUID(),startedAt:new Date().toISOString(),
   environment:{browser:navigator.userAgent,platform:navigator.platform,touchPoints:navigator.maxTouchPoints,
    hardwareConcurrency:navigator.hardwareConcurrency,deviceMemoryGiB:navigator.deviceMemory||null,
    screen:[screen.width,screen.height],pixelRatio:devicePixelRatio,page:location.origin+location.pathname},
   navigation:{type:performance.getEntriesByType('navigation')[0]?.type||'unknown',wasDiscarded:typeof document.wasDiscarded==='boolean'?document.wasDiscarded:null},
   playerBuild:document.querySelector('meta[name="polarity-build"]')?.content||'local-source',
   emulator:'@wasm-gaming/mgba-wasm 0.1.1',cartridge:null,events:[],snapshot:null,error:null};
  this.cached=this.read();
  this.event('page-start');this.persist();
  for(const name of ['blur','focus','pagehide','pageshow'])window.addEventListener(name,e=>this.checkpoint(name,{persisted:e.persisted===true}));
  for(const name of ['visibilitychange','freeze','resume'])document.addEventListener(name,()=>this.checkpoint(name));
  window.addEventListener('error',e=>{if(e.error||e.message)this.failure(e.error||new Error(e.message),'window-error');});
  window.addEventListener('unhandledrejection',e=>this.failure(e.reason,'unhandled-rejection'));
  for(const level of ['warn','error']){
   const original=console[level].bind(console);
   console[level]=(...args)=>{
    original(...args);
    const message=args.map(value=>{
     if(value instanceof Error)return value.stack||value.message;
     if(typeof value==='string')return value;
     try{return JSON.stringify(value);}catch{return String(value);}
    }).join(' ').slice(0,2000);
    this.event('console-'+level,{message});this.persist();
   };
  }
  document.querySelector('#download-diagnostics').addEventListener('click',()=>this.download());
  document.querySelector('#download-crash').addEventListener('click',()=>this.download());
  // Independent of the emulation loop, so a lost loop timer is observable.
  // A synchronous hang blocking the whole page still requires reopening it.
  for(const name of ['blur','focus','pagehide','pageshow'])window.addEventListener(name,()=>this.watch());
  document.addEventListener('visibilitychange',()=>this.watch());
  document.addEventListener('freeze',()=>{clearTimeout(this.watchTimer);this.progress=null;});
  document.addEventListener('resume',()=>this.watch());
  this.watch();
 }
 watch(){
  clearTimeout(this.watchTimer);this.progress=null;
  if(document.hidden||!document.hasFocus())return;
  const check=()=>{
   if(document.hidden||!document.hasFocus())return;
   this.checkProgress(performance.now());this.watchTimer=setTimeout(check,1000);
  };
  this.watchTimer=setTimeout(check,1000);
 }
 checkProgress(now){
  const status=this.status();
  if(!status.expectedToAdvance){this.progress=null;return;}
  if(!this.progress||this.progress.frames!==status.frames){
   this.progress={frames:status.frames,since:now};
   if(this.stallActive&&status.frames!==this.current.stall.frames){this.stallActive=false;this.current.stall.resumedAt=new Date().toISOString();this.event('frame-progress-resumed');this.persist();}
   return;
  }
  const durationMs=Math.round(now-this.progress.since);
  if(durationMs<5000||this.stallActive)return;
  this.stallActive=true;
  this.current.stall={time:new Date().toISOString(),type:'frame-progress-stopped',durationMs,frames:status.frames};
  this.event('frame-progress-stopped',{durationMs});this.persist();
 }
 static base64(bytes){let s='';for(let i=0;i<bytes.length;i+=8192)s+=String.fromCharCode(...bytes.subarray(i,i+8192));return btoa(s);}
 read(){
  try{
   const raw=localStorage.getItem(this.key);if(!raw)return [];
   if(raw.length>3*1024*1024)throw Error('Stored diagnostic data exceeds its size limit');
   const records=JSON.parse(raw);if(!Array.isArray(records))throw Error('Invalid diagnostic history');
   return records.filter(s=>s?.schema===1&&typeof s.id==='string'&&Array.isArray(s.events)).slice(0,3);
  }catch(error){this.storageError=String(error.message||error);return this.cached;}
 }
 event(type,detail={}){
  this.current.events.push({time:new Date().toISOString(),elapsedMs:Math.round(performance.now()),type,
   visibility:document.visibilityState,focused:document.hasFocus(),...this.status(),...detail});
  if(this.current.events.length>80)this.current.events.shift();
  this.current.updatedAt=new Date().toISOString();
 }
 records(){
  const others=this.read().filter(s=>s.id!==this.current.id);
  // Keep the latest failure across ordinary subsequent reloads, plus one recent
  // session in case the renderer was killed without delivering an error event.
  const failure=others.find(s=>s.error||s.stall),recent=others.filter(s=>s!==failure);
  return [this.current,...(failure?[failure]:[]),...recent].slice(0,3);
 }
 persist(){
  this.cached=this.records();
  try{localStorage.setItem(this.key,JSON.stringify(this.cached));this.storageError=null;}
  catch(error){this.storageError=String(error.message||error);}
  const previous=this.cached.find(s=>s.id!==this.current.id);
  document.querySelector('#diagnostics-status').textContent=this.storageError?
   'Reports cannot be saved in this browser. Download one before reloading.':
   this.stallActive?'The game stopped advancing. Download a report before reloading.':
   previous?'A report from your previous visit is available, including its last recorded game state.':
   'Recording locally. Download a report if the game freezes or closes.';
 }
 checkpoint(type,detail={}){
  this.event(type,detail);
  // Never enter the core after a fatal error; retain its last known healthy state.
  if(!this.current.error&&!this.stallActive){
   try{
    const snapshot=this.capture();
    if(snapshot){
     if(snapshot.state.length>1024*1024)throw Error('Emulator snapshot exceeds its size limit');
     this.current.snapshot={capturedAt:new Date().toISOString(),reason:type,frames:this.status().frames,
      stateBase64:PolarityDiagnostics.base64(snapshot.state),sramBase64:snapshot.sram||null};
     this.lastSnapshot=performance.now();
    }
   }catch(error){this.event('snapshot-unavailable',{message:String(error.message||error).slice(0,500)});}
  }
  this.persist();
 }
 tick(now){
  if(now-this.lastSample<5000)return;this.lastSample=now;
  if(now-this.lastSnapshot>=30000)this.checkpoint('heartbeat');
  else this.event('sample');
 }
 failure(error,type='emulator-error'){
  const message=String(error?.message||error).slice(0,1000),stack=String(error?.stack||'').slice(0,3000);
  this.current.error={time:new Date().toISOString(),type,message,stack};
  this.event(type,{message,stack});this.persist();
 }
 report(){
  let previousErrorSummary=null;
  try{const raw=localStorage.getItem('polarity-last-error');if(raw&&raw.length<=16384)previousErrorSummary=JSON.parse(raw);}
  catch(error){previousErrorSummary={unavailable:String(error.message||error)};}
  return {format:'polarity-crash-report',schema:1,exportedAt:new Date().toISOString(),storageError:this.storageError,
   note:'Contains game state, browser/device details and recent game events. A browser process crash may have no error stack; use the last recorded snapshot and lifecycle events.',
   previousErrorSummary,sessions:this.records()};
 }
 download(){
  this.checkpoint('report-download');
  const url=URL.createObjectURL(new Blob([JSON.stringify(this.report(),null,2)],{type:'application/json'}));
  const link=document.createElement('a');link.href=url;link.download='polarity-crash-report-'+new Date().toISOString().replace(/[:.]/g,'-')+'.json';
  document.body.append(link);link.click();link.remove();setTimeout(()=>URL.revokeObjectURL(url),10000);
 }
}
