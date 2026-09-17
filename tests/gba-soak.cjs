// Real wall-clock playback: exercise audio scheduling, saves and native frames.
const {chromium,webkit}=require('playwright'),assert=require('node:assert/strict');
const {symbols}=require('./gba-core.cjs');
(async()=>{
 const useWebKit=process.env.SOAK_BROWSER==='webkit';
 const browser=await (useWebKit?webkit:chromium).launch(useWebKit?{headless:true}:{channel:'chrome',headless:true,args:['--disable-gpu']});
 try{
  const page=await browser.newPage(),errors=[];
  page.on('pageerror',e=>errors.push(e.message));page.on('crash',()=>errors.push('Browser page crashed'));
  await page.goto(process.env.POLARITY_URL||'http://127.0.0.1:8791');
  await page.waitForFunction(()=>window.polarity);await page.click('#play');
  await page.waitForFunction(s=>polarity.read(s.game_mode)===4,symbols);
  await page.keyboard.press('z',{delay:40});await page.waitForFunction(s=>polarity.read(s.game_mode)===5,symbols);
  await page.keyboard.press('z',{delay:40});await page.waitForFunction(s=>polarity.read(s.game_mode)===1,symbols);
  const duration=Number(process.env.SOAK_SECONDS||600),start=Date.now();let previous=0,lastSample=start;
  console.log(`Starting ${duration}s real-time game/audio/save soak`);
  while(Date.now()-start<duration*1000){
   await page.keyboard.press('z',{delay:150});await page.keyboard.press('x',{delay:40});
   await page.waitForTimeout(1000);await page.keyboard.press('r');
   await page.waitForTimeout(Math.max(0,Math.min(28000,duration*1000-(Date.now()-start))));
   const status=await page.evaluate(s=>({frames:polarity.frames,mode:polarity.read(s.game_mode),wasmMiB:polarity.core.HEAPU8.length/1048576,heapMiB:performance.memory?.usedJSHeapSize/1048576,audioState:audio?.state,audioTime:audio?.currentTime,queuedAudio:typeof audioSources==='undefined'?null:audioSources.size,saved:!!localStorage.getItem('polarity-advance-save-v2')}),symbols);
   const sampleTime=Date.now();
   assert.ok(status.frames>previous+Math.min(500,(sampleTime-lastSample)*.02),'Emulator stopped advancing');previous=status.frames;lastSample=sampleTime;
   assert.equal(status.mode,1,'Cartridge left gameplay unexpectedly');assert.deepEqual(errors,[]);
   assert.equal(status.audioState,'running','Endurance test must exercise real audio playback');
   assert.ok(status.queuedAudio===null||status.queuedAudio<=12,'Audio queue must remain bounded');
   console.log(`[${Math.round((Date.now()-start)/1000)}/${duration}s] ${JSON.stringify(status)}`);
  }
  console.log('PASS: sustained real-time emulation, audio, input and saving without a browser error or cartridge freeze.');
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exit(1)});
