const {symbols}=require('./gba-core.cjs');
const {chromium}=require('playwright');
const assert=require('node:assert/strict');
(async()=>{
 const browser=await chromium.launch({channel:'chrome',headless:true});
 try {
 const page=await browser.newPage();const errors=[];page.on('pageerror',e=>{errors.push(e.message);console.error('Browser error:',e.message)});page.on('console',m=>{if(m.type()==='error')console.error('Browser console:',m.text())});
 await page.addInitScript(()=>{
  window.testPads=[];
  Object.defineProperty(navigator,'getGamepads',{value:()=>window.testPads});
  // Browsers may leave controller-initiated audio suspended until a page tap.
  class LockedAudio extends AudioContext {resume(){return new Promise(()=>{});}}
  window.AudioContext=LockedAudio;
  window.padInput=(buttons=[],axes=[0,0],index=2)=>{
   window.testPads[index]={id:'Standard test controller',index,connected:true,mapping:'standard',axes,buttons:Array.from({length:17},(_,i)=>({pressed:buttons.includes(i),value:buttons.includes(i)?1:0}))};
  };
 });
 await page.goto(process.env.POLARITY_URL||'http://127.0.0.1:8788',{waitUntil:'domcontentloaded'});
 await page.waitForFunction(()=>window.polarity);
 const input=async(buttons=[],axes=[0,0],index=2)=>{await page.evaluate(a=>{padInput(...a);return new Promise(resolve=>requestAnimationFrame(()=>requestAnimationFrame(resolve)));},[buttons,axes,index]);};
 await input([0]);
 await page.waitForFunction(()=>polarity.playing,{timeout:3000});
 console.log('PASS: controller at slot 2 starts game even with suspended audio');
 await input();await page.waitForTimeout(400);
 await input([0]);await page.waitForFunction(a=>polarity.read(a)===5,symbols.game_mode);await input();await page.waitForTimeout(100);await input([0]);await page.waitForFunction(a=>polarity.read(a)===1,symbols.game_mode);await input();
 assert.match(await page.locator('#controller-status').textContent(),/connected/i);
 await input([0,15]);
 await page.waitForFunction(()=>document.querySelector('[data-key="right"]').classList.contains('active')&&document.querySelector('[data-key="A"]').classList.contains('active'));
 await input([2],[.8,-.8]);
 for(const k of ['right','up','B'])assert(await page.locator(`[data-key="${k}"].active`).count());
 await input([],[.1,-.1]);assert.equal(await page.locator('[data-key].active').count(),0);
 console.log('PASS: jump + D-pad, diagonal analog dash, west-button dash, dead zone and release');
 await input([9]);assert(await page.evaluate(()=>polarity.paused));await page.waitForTimeout(150);assert(await page.evaluate(()=>polarity.paused));
 await input();await input([9]);assert.equal(await page.evaluate(()=>polarity.paused),false);
 await input();const retries=await page.evaluate(a=>polarity.read(a),symbols.deaths);await input([8]);await page.waitForFunction(({a,n})=>polarity.read(a)===n+1,{a:symbols.deaths,n:retries});await input();
 console.log('PASS: Start pauses once per press and resumes while paused; Back retries');
 await page.keyboard.down('ArrowLeft');await input();assert(await page.locator('[data-key="left"].active').count());await page.keyboard.up('ArrowLeft');
 await input([15]);await page.evaluate(()=>{testPads[2]=null;});await page.waitForFunction(()=>polarity.paused);assert(await page.evaluate(()=>polarity.paused));assert.equal(await page.locator('[data-key].active').count(),0);
 await input();await input([9]);assert.equal(await page.evaluate(()=>polarity.paused),false);await input();
 console.log('PASS: keyboard coexists; disconnect releases inputs and pauses; reconnect resumes');
 await page.evaluate(()=>window.dispatchEvent(new Event('blur')));await input([9]);assert(await page.evaluate(()=>polarity.paused));
 await page.evaluate(()=>window.dispatchEvent(new Event('focus')));await input([9]);assert(await page.evaluate(()=>polarity.paused));await input();await input([9]);assert.equal(await page.evaluate(()=>polarity.paused),false);
 console.log('PASS: background controller input cannot unpause the game');
 assert.deepEqual(errors,[]);console.log('PASS: no browser runtime errors');
 } finally {await browser.close();}
})().catch(e=>{console.error(e);process.exit(1)});
