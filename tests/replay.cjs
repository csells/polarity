const {chromium}=require('playwright');const fs=require('fs');const assert=require('node:assert/strict');
const base=process.env.GAME_URL||'http://127.0.0.1:8788';
const symbols={};for(const l of fs.readFileSync('build/polarity.noi','utf8').split('\n')){const m=l.match(/^DEF _(player|room_id|game_mode|deaths|tick|joypad|progress|selected|camera|saved_checkpoint|city) 0x([0-9A-F]+)/);if(m)symbols[m[1]]=parseInt(m[2],16)}
symbols.progress=symbols.city;symbols.saved_checkpoint=symbols.city+6;
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
 if(room&&room%3===0){await p.evaluate(({s,target})=>{
 const neighbors=[[0,1,1,2],[0,3,1,2],[0,4,1,2],[1,5,3,4],[2,5,3,4],[3,5,3,4]],keys=['left','right','up','down'];
 const start=polarity.read(s.selected),queue=[[start,[]]],seen=new Set([start]);
 while(queue.length){const [n,path]=queue.shift();if(n===target){path.forEach(tap);break;}neighbors[n].forEach((v,i)=>{if(!seen.has(v)){seen.add(v);queue.push([v,[...path,keys[i]]]);}});}
 tap('A');tap('A');
 },{s:symbols,target:Math.floor(room/3)});}
 const route=JSON.parse(fs.readFileSync(`artifacts/route-letter-${room}.json`));
 const result=await p.evaluate(({s,room,route})=>{
  const q=polarity;const before=q.read(s.deaths),begin=q.core._emulator_get_ticks_f64(q.emu);const keyNames=['left','right','up','down','A','B'];let moves=0,maxCamera=0;
  if(q.read(s.room_id)!==room||q.read(s.game_mode)!==1)throw Error('Wrong room start '+room);
  for(const keys of route){for(let n=0;n<6;n++){
   keyNames.forEach((k,i)=>q.setKey(k,!!(keys&(1<<i))));nextInput();moves++;
   maxCamera=Math.max(maxCamera,q.read(s.camera)+256*q.read(s.camera+1));
   if(q.read(s.room_id)!==room||q.read(s.game_mode)!==1||q.read(s.deaths)!==before)break;
  }if(q.read(s.room_id)!==room||q.read(s.game_mode)!==1||q.read(s.deaths)!==before)break;}
  q.release();q.render();q.save();
  return {room:room+1,next:q.read(s.room_id),deaths:q.read(s.deaths)-before,moves,mode:q.read(s.game_mode),maxCamera,relays:q.read(s.player+23),letter:q.read(s.player+24),videoFrames:Math.round((q.core._emulator_get_ticks_f64(q.emu)-begin)/70224),progress:q.read(s.progress+Math.floor(room/3)),x:q.read(s.player)+256*q.read(s.player+1),y:q.read(s.player+2)+256*q.read(s.player+3)};
 },{s:symbols,room,route});console.log('ROM replay',result);
 assert.equal(result.deaths,0,'Unexpected death in room '+(room+1));assert.ok(result.videoFrames<result.moves*1.2+30,'Rendering slowed the simulation in room '+(room+1));assert.ok(result.maxCamera>=470,'Camera did not follow entire room');
 assert.equal(result.progress,room%3+1);assert.equal(result.mode,6);assert.equal(result.relays,3);assert.equal(result.letter,1);
 await p.evaluate(()=>{polarity.advance(70224*2,false);polarity.render();});
 if(room%3===2)await p.locator('#screen').screenshot({path:`artifacts/delivery-${Math.floor(room/3)}.png`});
 await p.evaluate(()=>tap('A'));
 if(room%3===2){assert.equal(await p.evaluate(s=>polarity.read(s.game_mode),symbols),7);await p.evaluate(()=>{polarity.advance(70224*2,false);polarity.render();});await p.locator('#screen').screenshot({path:`artifacts/restoration-${Math.floor(room/3)}.png`});await p.evaluate(()=>tap('A'));if(room===17){assert.equal(await p.evaluate(s=>polarity.read(s.game_mode),symbols),10);await p.evaluate(()=>{polarity.advance(70224*2,false);polarity.render();});await p.locator('#screen').screenshot({path:'artifacts/homecoming.png'});await p.evaluate(()=>tap('A'));}}
 if(room===0)await p.screenshot({path:'artifacts/mobile-adventure.png',fullPage:true});
}
await p.evaluate(()=>{polarity.advance(70224*2,false);polarity.render();});
await p.screenshot({path:'artifacts/all-locations-complete.png',fullPage:true});
await p.evaluate(()=>{tap('A');tap('right');tap('right');tap('select');polarity.advance(70224*2,false);polarity.render();});
assert.equal(await p.evaluate(s=>polarity.read(s.game_mode),symbols),9);await p.locator('#screen').screenshot({path:'artifacts/last-letter.png'});

await p.reload();await p.waitForFunction(()=>window.polarity);
const saved=await p.evaluate(s=>Array.from({length:6},(_,i)=>polarity.read(s.progress+i)),symbols);assert.deepEqual(saved,[3,3,3,3,3,3]);assert.deepEqual(await p.evaluate(s=>[polarity.read(s.city+18),polarity.read(s.city+19),polarity.read(s.city+20)],symbols),[255,255,3]);
assert.equal(errors.length,0,errors.join('\n'));console.log('All 18 real ROM routes and letters, both charge links, branching unlocks, resident deliveries, restoration, homecoming and save/reload passed.');
}finally{await b.close();}
})().catch(e=>{console.error(e);process.exit(1)});
