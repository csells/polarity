const {chromium}=require('playwright');const fs=require('fs');const assert=require('node:assert/strict');
const base=process.env.GAME_URL||'http://127.0.0.1:8788';
const symbols={};for(const l of fs.readFileSync('build/polarity.noi','utf8').split('\n')){const m=l.match(/^DEF _(player|room_id|game_mode|deaths|tick|joypad|progress|selected|camera|saved_checkpoint) 0x([0-9A-F]+)/);if(m)symbols[m[1]]=parseInt(m[2],16)}
(async()=>{
const b=await chromium.launch({channel:'chrome',headless:true});try{
const p=await b.newPage({viewport:{width:390,height:844},hasTouch:true});const errors=[];p.on('pageerror',e=>errors.push(e.message));
await p.goto(base,{waitUntil:'domcontentloaded'});await p.waitForFunction(()=>window.polarity);await p.evaluate(()=>{polarity.freeze();document.querySelector('#play').hidden=true;});
await p.evaluate(s=>{
 const q=polarity,c=q.core,e=q.emu;
 window.nextInput=()=>{let left=false;for(let i=0;i<1000000;i++){const pc=c._emulator_get_PC(e);if(pc!==s.joypad)left=true;if(left&&pc===s.joypad)return;const ev=c._emulator_run_until_f64(e,c._emulator_get_ticks_f64(e)+1);if(ev&16)throw Error('Invalid opcode');}throw Error('Input polling not reached');};
 window.tap=key=>{q.release();nextInput();q.setKey(key,true);nextInput();q.release();};
 nextInput();tap('start');q.render();
},symbols);
assert.equal(await p.evaluate(s=>polarity.read(s.game_mode),symbols),4);
await p.screenshot({path:'artifacts/world-map.png',fullPage:true});
// A locked destination cannot be entered.
await p.evaluate(()=>{tap('right');tap('A');});assert.equal(await p.evaluate(s=>polarity.read(s.game_mode),symbols),4);
await p.evaluate(()=>{tap('left');tap('A');tap('A');});
for(let room=0;room<18;room++){
 if(room&&room%3===0){await p.evaluate(()=>{tap('right');tap('A');tap('A');});}
 const route=JSON.parse(fs.readFileSync(`artifacts/route-${room}.json`));
 const result=await p.evaluate(({s,room,route})=>{
  const q=polarity;const before=q.read(s.deaths);const keyNames=['left','right','up','down','A','B'];let moves=0,maxCamera=0;
  if(q.read(s.room_id)!==room||q.read(s.game_mode)!==1)throw Error('Wrong room start '+room);
  for(const keys of route){for(let n=0;n<6;n++){
   keyNames.forEach((k,i)=>q.setKey(k,!!(keys&(1<<i))));nextInput();moves++;
   maxCamera=Math.max(maxCamera,q.read(s.camera)+256*q.read(s.camera+1));
   if(q.read(s.room_id)!==room||q.read(s.game_mode)!==1||q.read(s.deaths)!==before)break;
  }if(q.read(s.room_id)!==room||q.read(s.game_mode)!==1||q.read(s.deaths)!==before)break;}
  q.release();q.render();q.save();
  return {room:room+1,next:q.read(s.room_id),deaths:q.read(s.deaths)-before,moves,mode:q.read(s.game_mode),maxCamera,progress:q.read(s.progress+Math.floor(room/3)),x:q.read(s.player)+256*q.read(s.player+1),y:q.read(s.player+2)+256*q.read(s.player+3)};
 },{s:symbols,room,route});console.log('ROM replay',result);
 assert.equal(result.deaths,0,'Unexpected death in room '+(room+1));assert.ok(result.maxCamera>=470,'Camera did not follow entire room');
 assert.equal(result.progress,room%3+1);assert.equal(result.mode,room%3===2?4:1);
 if(room%3!==2)assert.equal(result.next,room+1);
 if(room===0)await p.screenshot({path:'artifacts/mobile-adventure.png',fullPage:true});
}
await p.screenshot({path:'artifacts/all-locations-complete.png',fullPage:true});
await p.reload();await p.waitForFunction(()=>window.polarity);
const saved=await p.evaluate(s=>Array.from({length:6},(_,i)=>polarity.read(s.progress+i)),symbols);assert.deepEqual(saved,[3,3,3,3,3,3]);
assert.equal(errors.length,0,errors.join('\n'));console.log('All 18 real ROM routes, branching unlocks, four-screen scrolling and save/reload passed.');
}finally{await b.close();}
})().catch(e=>{console.error(e);process.exit(1)});
