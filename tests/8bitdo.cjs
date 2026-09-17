const {symbols}=require('./gba-core.cjs');
const {chromium}=require('playwright');const assert=require('node:assert/strict');
(async()=>{
 const browser=await chromium.launch({channel:'chrome',headless:true});
 try{
 for(const kind of ['axes','hat']){
  const context=await browser.newContext();const page=await context.newPage();const errors=[];
  page.on('pageerror',e=>errors.push(e.message));
  await page.addInitScript(({kind})=>{
   window.rawIdle=kind==='hat'?[0,0,3.285714]:[0,0];
   window.rawAxes=rawIdle.slice();window.rawButtons=[];
   Object.defineProperty(navigator,'getGamepads',{value:()=>[null,null,{id:'8BitDo SN30 test '+kind,index:2,mapping:'',connected:true,axes:rawAxes,buttons:Array.from({length:12},(_,i)=>({pressed:rawButtons.includes(i),value:rawButtons.includes(i)?1:0}))}]});
  },{kind});
  const input=async(buttons=[],axes=null)=>page.evaluate(a=>{rawButtons=a.buttons;rawAxes=a.axes||rawIdle.slice();return new Promise(r=>requestAnimationFrame(()=>requestAnimationFrame(r)));},{buttons,axes});
  await page.goto(process.env.POLARITY_URL||'http://127.0.0.1:8788',{waitUntil:'domcontentloaded'});await page.waitForFunction(()=>window.polarity);
  await input();assert.match(await page.locator('#controller-status').textContent(),/Set up controller/);
  await page.click('.controller-help summary');await page.click('#controller-configure');await input();await page.keyboard.press('Enter');assert.equal(await page.evaluate(()=>polarity.playing),false);
  const directions=kind==='axes'?[[0,-1],[0,1],[-1,0],[1,0]]:[[0,0,-1],[0,0,1/7],[0,0,5/7],[0,0,-3/7]];
  for(let i=0;i<8;i++){
   assert.match(await page.locator('#controller-setup-prompt').textContent(),new RegExp((i+1)+'/8'));
   if(i<4)await input([],directions[i]);else await input([[1],[0],[7],[6]][i-4]);
   await input();
  }
  assert.match(await page.locator('#controller-setup-prompt').textContent(),/saved/);
  await input([7]);await page.waitForFunction(()=>polarity.playing);await input();await page.waitForTimeout(400);
  await input([1]);await page.waitForFunction(a=>polarity.read(a)===5,symbols.game_mode);await input();await page.waitForTimeout(100);await input([1]);await page.waitForFunction(a=>polarity.read(a)===1,symbols.game_mode);await input();
  await input([1],directions[3]);for(const k of ['A','right'])assert(await page.locator(`[data-key="${k}"].active`).count());
  await input([0],kind==='axes'?[1,-1]:[0,0,-5/7]);for(const k of ['B','up','right'])assert(await page.locator(`[data-key="${k}"].active`).count());
  await input();assert.equal(await page.locator('[data-key].active').count(),0);
  await input([7]);assert(await page.evaluate(()=>polarity.paused));await input();await input([7]);assert.equal(await page.evaluate(()=>polarity.paused),false);await input();
  const before=await page.evaluate(a=>polarity.read(a),symbols.deaths);await input([6]);await page.waitForFunction(({a,n})=>polarity.read(a)===n+1,{a:symbols.deaths,n:before});await input();
  await page.reload({waitUntil:'domcontentloaded'});await page.waitForFunction(()=>window.polarity);await input();assert.match(await page.locator('#controller-status').textContent(),/Custom layout ready/);
  await input([7]);await page.waitForFunction(()=>polarity.playing);await input();
  assert.deepEqual(errors,[]);
  console.log(`PASS: raw 8BitDo ${kind} reports: setup, jump, diagonal dash, Start, Select, neutral release, persisted layout and reload`);
  await context.close();
 }
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exit(1)});
