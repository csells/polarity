const {chromium}=require('playwright'),assert=require('node:assert/strict'),fs=require('node:fs');
const url=process.env.POLARITY_URL||'http://127.0.0.1:8792';
(async()=>{
 const browser=await chromium.launch({channel:'chrome',headless:true,args:['--disable-gpu']});
 try{
  const p=await browser.newPage({acceptDownloads:true});
  await p.goto(url);await p.waitForFunction(()=>window.polarity);await p.click('#play');
  await p.click('#pause');await p.waitForTimeout(6500);
  assert.equal(await p.evaluate(()=>JSON.parse(localStorage.getItem('polarity-diagnostics-v1'))[0].stall||null),null,'Normal pause is not a stall');
  await p.click('#pause');await p.waitForTimeout(1200);
  await p.evaluate(()=>polarity.freeze());
  await p.waitForFunction(()=>JSON.parse(localStorage.getItem('polarity-diagnostics-v1'))[0].stall,{},{timeout:8000});
  assert.match(await p.locator('#diagnostics-status').textContent(),/stopped advancing/);
  // A report for a suspected stalled core must not call into it again.
  await p.evaluate(()=>{polarity.core._mgbawasm_state_save=()=>{throw Error('Report reentered stalled core');};});
  await p.locator('.diagnostics summary').click();
  const [file]=await Promise.all([p.waitForEvent('download'),p.click('#download-diagnostics')]);
  const report=JSON.parse(fs.readFileSync(await file.path(),'utf8')),s=report.sessions[0];
  assert.equal(s.stall.type,'frame-progress-stopped');assert.ok(s.stall.durationMs>=5000);
  assert.ok(s.snapshot.stateBase64);assert.equal(s.error,null);
  assert.ok(!s.events.some(e=>e.type==='snapshot-unavailable'));
  assert.ok(s.events.some(e=>e.type==='frame-progress-stopped'&&e.expectedToAdvance===true));
  await p.reload();await p.waitForFunction(()=>window.polarity);
  assert.ok(await p.evaluate(id=>JSON.parse(localStorage.getItem('polarity-diagnostics-v1')).some(s=>s.id===id&&s.stall),s.id));
  console.log('PASS: normal pause is not a freeze; stopped game loop is detected without an exception; download preserves healthy snapshot and stall history across reload.');
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exit(1)});
