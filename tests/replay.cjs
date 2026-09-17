const {chromium}=require('playwright');const fs=require('fs');
(async()=>{
const b=await chromium.launch({channel:'chrome',headless:true});const p=await b.newPage({viewport:{width:390,height:844},hasTouch:true});const errors=[];p.on('pageerror',e=>errors.push(e.message));
await p.goto('http://127.0.0.1:8787',{waitUntil:'domcontentloaded'});await p.waitForFunction(()=>window.polarity);await p.click('#play');await p.waitForTimeout(1200);await p.evaluate(()=>polarity.freeze());
const symbols={};for(const l of fs.readFileSync('build/polarity.noi','utf8').split('\n')){const m=l.match(/^DEF _(player|room_id|game_mode|deaths|tick|joypad) 0x([0-9A-F]+)/);if(m)symbols[m[1]]=parseInt(m[2],16)}
console.log('Boot',await p.evaluate(s=>({mode:polarity.read(s.game_mode),room:polarity.read(s.room_id),ticks:polarity.read(s.tick)}),symbols));
console.log('Debug API',await p.evaluate(()=>({breakpoint:polarity.core._emulator_set_breakpoint.length,clear:polarity.core._emulator_clear_breakpoints.length})));
await p.screenshot({path:'artifacts/mobile-game.png',fullPage:true,timeout:10000});
for(let room=0;room<8;room++){
const route=JSON.parse(fs.readFileSync(`artifacts/route-${room}.json`));
const result=await p.evaluate(({s,room,route})=>{
 const q=polarity; q.release();
 const c=q.core,e=q.emu;
 function nextInput(){
  let left=false;
  for(let i=0;i<1000000;i++){
   const pc=c._emulator_get_PC(e);if(pc!==s.joypad)left=true;if(left&&pc===s.joypad)return;
   const ev=c._emulator_run_until_f64(e,c._emulator_get_ticks_f64(e)+1);if(ev&16)throw Error('Invalid opcode');
  }
  throw Error('Input polling not reached');
 }
 nextInput();
 // Initialize a room fixture at its real starting position. Progression must happen through its exit.
 q.write(s.room_id,room);q.write(s.game_mode,1);
 const init=[0,1,64,7,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0];init.forEach((v,i)=>q.write(s.player+i,v));
 const before=q.read(s.deaths);const keyNames=['left','right','up','down','A','B'];let moves=0;
 for(const keys of route){for(let n=0;n<6;n++){
  keyNames.forEach((k,i)=>q.setKey(k,!!(keys&(1<<i))));nextInput();moves++;
  if(q.read(s.room_id)!==room||q.read(s.deaths)!==before)break;
 }if(q.read(s.room_id)!==room||q.read(s.deaths)!==before)break;}
 q.release();q.render();
 return {room:room+1,next:q.read(s.room_id),deaths:q.read(s.deaths)-before,moves,mode:q.read(s.game_mode),x:q.read(s.player)+256*q.read(s.player+1),y:q.read(s.player+2)+256*q.read(s.player+3)};
},{s:symbols,room,route});console.log('Replay',result);
if(result.next!==room+1||result.deaths)throw Error('ROM replay failed in room '+(room+1));
}
await p.screenshot({path:'artifacts/victory.png',timeout:10000});console.log('Browser errors:',errors);await b.close();if(errors.length)process.exit(1);
})().catch(e=>{console.error(e);process.exit(1)});
