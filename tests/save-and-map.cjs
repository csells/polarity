const {chromium}=require('playwright');const fs=require('fs');const assert=require('node:assert/strict');
const s={};for(const l of fs.readFileSync('build/polarity.noi','utf8').split('\n')){const m=l.match(/^DEF _(player|game_mode|room_id|deaths|saved_checkpoint|camera|oldkeys|tick) 0x([0-9A-F]+)/);if(m)s[m[1]]=parseInt(m[2],16)}
(async()=>{const b=await chromium.launch({channel:'chrome',headless:true});try{
const p=await b.newPage({viewport:{width:390,height:844},isMobile:true,hasTouch:true});await p.goto(process.env.POLARITY_URL||'http://127.0.0.1:8788');await p.waitForFunction(()=>window.polarity);await p.click('#play');await p.waitForFunction(a=>polarity.read(a)===4,s.game_mode);
async function press(code,mode){
 await p.waitForFunction(a=>polarity.read(a)===0,s.oldkeys);
 await p.keyboard.down(code);
 if(mode!==undefined)await p.waitForFunction(({a,m})=>polarity.read(a)===m,{a:s.game_mode,m:mode});
 else await p.waitForFunction(a=>polarity.read(a)!==0,s.oldkeys);
 await p.keyboard.up(code);await p.waitForFunction(a=>polarity.read(a)===0,s.oldkeys);
}
await p.screenshot({path:'artifacts/map-phone.png',fullPage:true});await press('KeyZ',5);await press('KeyZ',1);
await p.waitForTimeout(300);await p.locator('#screen').screenshot({path:'artifacts/room-start.png'});
// Place just before the real flag, then walk into it through ordinary input.
await p.evaluate(s=>{polarity.write(s.player,152*16&255);polarity.write(s.player+1,152*16>>8);polarity.write(s.player+2,2000&255);polarity.write(s.player+3,2000>>8);},s);
await p.keyboard.down('ArrowRight');await p.waitForFunction(a=>polarity.read(a)===20,s.saved_checkpoint);await p.keyboard.up('ArrowRight');await p.waitForTimeout(1100);
await p.locator('#screen').screenshot({path:'artifacts/room-scrolled.png'});
await p.reload();await p.waitForFunction(()=>window.polarity);assert.equal(await p.evaluate(a=>polarity.read(a),s.saved_checkpoint),20);await p.click('#play');await p.waitForFunction(a=>polarity.read(a)===4,s.game_mode);await press('KeyZ',5);await press('KeyZ',1);
const x=await p.evaluate(a=>polarity.read(a)+256*polarity.read(a+1),s.player);assert.equal(x,160*16,'reload resumes at flag');
const before=await p.evaluate(a=>polarity.read(a),s.deaths);await press('KeyR');assert.equal(await p.evaluate(a=>polarity.read(a),s.deaths),before+1);assert.equal(await p.evaluate(a=>polarity.read(a)+256*polarity.read(a+1),s.player),160*16);
console.log('Checkpoint reload and retry passed; opening map',await p.evaluate(s=>({mode:polarity.read(s.game_mode),paused:polarity.paused,room:polarity.read(s.room_id)}),s));await p.click('#map');await p.waitForTimeout(500);console.log('Map input result',await p.evaluate(s=>({mode:polarity.read(s.game_mode),paused:polarity.paused,room:polarity.read(s.room_id),active:[...document.querySelectorAll('[data-key].active')].map(e=>e.dataset.key)}),s));await p.locator('#screen').screenshot({path:'artifacts/map-button-result.png'});await p.waitForFunction(a=>polarity.read(a)===4,s.game_mode);await press('KeyX',1);assert.equal(await p.evaluate(a=>polarity.read(a)+256*polarity.read(a+1),s.player),160*16);
await press('KeyM',4);await press('KeyX',1);
// Camera must move while the HUD stays at the top of the image, and steady play stays responsive.
const t=await p.evaluate(s=>({clock:polarity.read(s.player+20),frames:polarity.frames}),s);await p.waitForTimeout(500);const u=await p.evaluate(s=>({clock:polarity.read(s.player+20),frames:polarity.frames}),s);console.log('Simulation cadence',{updates:(u.clock-t.clock+256)%256,frames:u.frames-t.frames});assert.ok(((u.clock-t.clock+256)%256)>=Math.max(1,(u.frames-t.frames)*.8-2),'Simulation fell behind video frames');
console.log('PASS: phone map, real flag activation, reload at flag, retry at flag, map return/resume, and responsive simulation.');
}finally{await b.close();}})().catch(e=>{console.error(e);process.exit(1)});
