const {chromium}=require('playwright'),assert=require('node:assert/strict'),fs=require('node:fs');
const {symbols,boot}=require('./gba-core.cjs');
const url=process.env.POLARITY_URL||'http://127.0.0.1:8792';
async function download(page){
 if(!await page.locator('.diagnostics').evaluate(el=>el.open))await page.locator('.diagnostics summary').click();
 const [file]=await Promise.all([page.waitForEvent('download',{timeout:3000}),page.locator('#download-diagnostics').click()]);
 return JSON.parse(fs.readFileSync(await file.path(),'utf8'));
}
(async()=>{
 const browser=await chromium.launch({channel:'chrome',headless:true,args:['--disable-gpu']});
 try{
  const context=await browser.newContext({acceptDownloads:true}),p=await context.newPage();
  await p.goto(url);await p.waitForFunction(()=>window.polarity);await p.click('#play');
  await p.waitForFunction(s=>polarity.read(s.game_mode)===4,symbols);
  await p.keyboard.press('z',{delay:30});await p.waitForFunction(s=>polarity.read(s.game_mode)===5,symbols);
  await p.keyboard.press('z',{delay:30});await p.waitForFunction(s=>polarity.read(s.game_mode)===1,symbols);
  await p.waitForTimeout(200);
  // Exercise actual tab focus loss. Automation keeps visibility "visible", so
  // dispatch freeze/resume separately to cover those browser event handlers.
  const cdp=await context.newCDPSession(p);
  await cdp.send('Emulation.setFocusEmulationEnabled',{enabled:false});
  await p.evaluate(()=>{const get=navigator.getGamepads.bind(navigator);window.inputPolls=0;navigator.getGamepads=()=>{inputPolls++;return get();};});
  const other=await context.newPage();await other.goto('about:blank');await other.bringToFront();
  await p.waitForFunction(()=>!document.hasFocus()&&polarity.paused);
  const stopped=await p.evaluate(()=>polarity.frames);
  const stoppedPolls=await p.evaluate(()=>inputPolls);
  await p.evaluate(()=>document.dispatchEvent(new Event('freeze')));
  await new Promise(resolve=>setTimeout(resolve,6000));
  assert.equal(await p.evaluate(()=>polarity.frames),stopped,'Background emulation must stop');
  assert.equal(await p.evaluate(()=>inputPolls),stoppedPolls,'Background input polling must stop too');
  await p.evaluate(()=>document.dispatchEvent(new Event('resume')));
  await p.bringToFront();
  const initial=await download(p);
  assert.equal(initial.format,'polarity-crash-report');
  const session=initial.sessions[0];
  assert.ok(session.events.some(e=>e.type==='freeze'),'Capture background lifecycle before suspension');
  assert.ok(session.snapshot?.stateBase64,'Include a restorable native emulator snapshot');
  assert.match(session.cartridge.sha256,/^[a-f0-9]{64}$/);
  // An exception must retain the healthy snapshot without querying a dead core.
  await p.evaluate(()=>{polarity.core._mgbawasm_run_frame=()=>{throw Error('Background crash fixture');};});
  await p.keyboard.press('Enter');
  await p.locator('#recovery').waitFor({state:'visible'});
  const crash=await download(p),crashed=crash.sessions.find(s=>s.error?.message==='Background crash fixture');
  assert.ok(crashed?.snapshot?.stateBase64);
  await p.reload();await p.waitForFunction(()=>window.polarity);
  const after=await download(p);
  assert.ok(after.sessions.some(s=>s.id===crashed.id&&s.error?.message==='Background crash fixture'),'Reload must retain the crash report');
  // The downloaded snapshot must actually load in the shipped mGBA core.
  const g=await boot(),state=Buffer.from(crashed.snapshot.stateBase64,'base64');
  assert.equal(state.length,g.m._mgbawasm_state_size());g.m.HEAPU8.set(state,g.state);
  assert.equal(g.m._mgbawasm_state_load(g.state),1);g.run(0,3);assert.equal(g.read('game_mode'),1);
  // A killed renderer has no opportunity to execute our JavaScript handler.
  // Its pre-freeze record must still be downloadable from a replacement page.
  const beforeKill=after.sessions[0].id;
  await cdp.send('Emulation.setFocusEmulationEnabled',{enabled:false});
  await other.bringToFront();await p.waitForFunction(()=>!document.hasFocus());
  const killed=p.waitForEvent('crash');
  cdp.send('Page.crash').catch(()=>{});await killed;
  const reopened=await context.newPage();await reopened.goto(url);await reopened.waitForFunction(()=>window.polarity);
  const recovered=await download(reopened),last=recovered.sessions.find(s=>s.id===beforeKill);
  assert.ok(last?.events.some(e=>e.type==='blur'),'Persist the last background event before a renderer crash');
  assert.ok(last.snapshot.stateBase64,'Keep the snapshot even when no error handler ran');
  assert.equal(last.error,null,'Do not invent a JavaScript crash stack for a browser process failure');
  assert.ok(recovered.sessions.length<=3);
  assert.ok(JSON.stringify(recovered).length<3*1024*1024,'Diagnostic storage must stay bounded');
  const restricted=await browser.newContext({acceptDownloads:true}),r=await restricted.newPage();
  await r.addInitScript(()=>{localStorage.setItem('polarity-last-error',JSON.stringify({message:'Existing error from older player'}));const set=Storage.prototype.setItem;Storage.prototype.setItem=function(k,v){if(k==='polarity-diagnostics-v1')throw new DOMException('Diagnostic quota fixture','QuotaExceededError');return set.call(this,k,v);};});
  await r.goto(url);await r.waitForFunction(()=>window.polarity);
  await r.evaluate(()=>{console.warn('Diagnostic console fixture');Promise.reject(new Error('Diagnostic rejection fixture'));});
  await r.waitForTimeout(100);
  const live=await download(r);
  assert.match(live.storageError,/Diagnostic quota fixture/);
  assert.equal(live.previousErrorSummary.message,'Existing error from older player');
  assert.equal(live.sessions[0].error.type,'unhandled-rejection');
  assert.ok(live.sessions[0].error.stack.includes('Diagnostic rejection fixture'));
  assert.ok(live.sessions[0].events.some(e=>e.type==='console-warn'&&e.message.includes('Diagnostic console fixture')));
  assert.match(await r.locator('#diagnostics-status').textContent(),/cannot be saved/);
  console.log('PASS: lifecycle history, cartridge identity, usable emulator snapshot, crash stack, download and reload persistence.');
  console.log('PASS: a real killed browser renderer retains its background report; diagnostic history stays bounded.');
  console.log('PASS: console/rejection evidence and live downloads remain available when diagnostic storage is full.');
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exit(1)});
