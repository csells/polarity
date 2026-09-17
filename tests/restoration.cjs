const {chromium}=require('playwright');const fs=require('fs');const assert=require('node:assert/strict');
const s={};for(const l of fs.readFileSync('build/polarity.noi','utf8').split('\n')){const m=l.match(/^DEF _(player|game_mode|room_id|city|selected|oldkeys) 0x([0-9A-F]+)/);if(m)s[m[1]]=parseInt(m[2],16)}
function save(version,progress,selected=0,cp=0){const b=new Uint8Array(8192);b[0]=0x50;b[1]=version;b[2]=selected;progress.forEach((v,i)=>b[3+i]=v);b[9+selected]=cp;let sum=37;for(let i=2;i<(version===2?15:31);i++)sum+=b[i];b[version===2?15:31]=sum&255;return Buffer.from(b).toString('base64');}
(async()=>{const b=await chromium.launch({channel:'chrome',headless:true});try{
const context=await b.newContext();await context.addInitScript(v=>localStorage.setItem('polarity-adventure-save-v2',v),save(2,[3,1,0,0,0,0],1,40));const p=await context.newPage();await p.goto('http://127.0.0.1:8788');await p.waitForFunction(()=>window.polarity);
assert.deepEqual(await p.evaluate(s=>({progress:[...Array(6)].map((_,i)=>polarity.read(s.city+i)),cp:polarity.read(s.city+7),relays:polarity.read(s.city+13),selected:polarity.read(s.selected)}),s),{progress:[3,1,0,0,0,0],cp:0,relays:0,selected:1});await context.close();console.log('PASS: legacy completed deliveries retained; obsolete geometry checkpoint reset safely.');
for(let area=0;area<6;area++){
 const c=await b.newContext({viewport:{width:1100,height:900}});await c.addInitScript(v=>localStorage.setItem('polarity-adventure-save-v2',v),save(3,[3,3,3,3,3,3],area));const p=await c.newPage();const errors=[];p.on('pageerror',e=>errors.push(e.message));await p.goto('http://127.0.0.1:8788');await p.waitForFunction(()=>window.polarity);
 await p.evaluate(()=>{polarity.freeze();document.querySelector('#play').hidden=true;window.tap=key=>{polarity.release();polarity.advance(70224*8,false);polarity.setKey(key,true);polarity.advance(70224*8,false);polarity.release();polarity.advance(70224*8,false);};tap('start');});
 assert.equal(await p.evaluate(a=>polarity.read(a),s.game_mode),4);
 if(area===0)await p.locator('#screen').screenshot({path:'artifacts/city-restored.png'});
 await p.evaluate(()=>tap('A'));assert.equal(await p.evaluate(a=>polarity.read(a),s.game_mode),5);
 await p.evaluate(()=>{tap('right');tap('right');});assert.equal(await p.evaluate(a=>polarity.read(a),s.room_id),area*3+2);
 await p.locator('#screen').screenshot({path:`artifacts/room-chooser-${area}.png`});
 await p.evaluate(()=>{tap('A');polarity.advance(70224*60,false);});assert.equal(await p.evaluate(a=>polarity.read(a),s.game_mode),1);
 await p.locator('#screen').screenshot({path:`artifacts/location-${area}.png`});
 const music=await p.evaluate(()=>{polarity.advance(70224*180,false);const q=polarity,c=q.core;const samples=c.HEAPU8.slice(c._get_audio_buffer_ptr(q.emu),c._get_audio_buffer_ptr(q.emu)+c._get_audio_buffer_capacity(q.emu));return {bass:!!(q.read(0xff1a)&128),levels:new Set(samples).size};});assert(music.bass&&music.levels>3,'Music did not produce layered audio');
 assert.deepEqual(errors,[]);console.log(`PASS: district ${area+1}, spatial map, room replay selection, scenery, layered music.`);await c.close();
}
}finally{await b.close();}})().catch(e=>{console.error(e);process.exit(1)});
