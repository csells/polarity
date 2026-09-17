const {chromium}=require('playwright'),assert=require('node:assert/strict');
const {symbols}=require('./gba-core.cjs');
(async()=>{
 const b=await chromium.launch({channel:'chrome',headless:true,args:['--disable-gpu']});
 try{
  const p=await b.newPage();
  await p.addInitScript(()=>{
   if(sessionStorage.getItem('recovery-fixture-seeded'))return;
   sessionStorage.setItem('recovery-fixture-seeded','1');
   const save=new Uint8Array(32768);save[0]=65;save[1]=2;save[9]=20;save[16]=1;save[31]=58;
   localStorage.setItem('polarity-advance-save-v2',btoa(String.fromCharCode(...save)));
  });
  await p.goto(process.env.POLARITY_URL||'http://127.0.0.1:8792');
  await p.waitForFunction(()=>window.polarity);await p.click('#play');
  await p.waitForFunction(s=>polarity.read(s.game_mode)===4,symbols);
  await p.keyboard.press('x',{delay:40});await p.waitForFunction(s=>polarity.read(s.game_mode)===1,symbols);
  assert.equal(await p.evaluate(s=>polarity.read(s.player+21),symbols),20);
  await p.evaluate(()=>{window.failedFrames=0;polarity.core._mgbawasm_run_frame=()=>{failedFrames++;polarity.core._mgbawasm_set_keys=()=>{throw Error('Core unavailable after failure');};throw Error('Injected emulator failure');};});
  await p.locator('#recovery').waitFor({state:'visible',timeout:3000});
  await p.waitForTimeout(200);assert.equal(await p.evaluate(()=>failedFrames),1,'Stop the failing timer instead of throwing every 8ms');
  assert.match(await p.locator('#error-report').textContent(),/Injected emulator failure/);
  // The focused recovery button must work with a keyboard as well as a tap.
  await Promise.all([p.waitForEvent('load',{timeout:3000}),p.keyboard.press('Enter')]);
  await p.waitForFunction(()=>window.polarity);
  assert.equal(await p.evaluate(s=>polarity.read(s.city+6),symbols),20,'Reload keeps the saved checkpoint');
  console.log('PASS: runtime errors stop the loop, expose a useful report, and reload preserves Advance progress.');
 }finally{await b.close();}
})().catch(e=>{console.error(e);process.exit(1)});
