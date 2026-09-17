const assert=require('node:assert/strict'),fs=require('node:fs');
const {boot}=require('./gba-core.cjs');
(async()=>{
 const tracks=[];let rate;
 for(const dash of [false,true]){
  const g=await boot();g.tap(8);g.tap(1);g.tap(1);g.run(0,30);
  rate=g.m._mgbawasm_sample_rate();const chunks=[];
  for(let frame=0;frame<90;frame++){
   g.m._mgbawasm_set_keys(dash&&frame===0?2:0);g.m._mgbawasm_run_frame();
   const n=g.m._mgbawasm_read_audio(g.audio,8192);
   chunks.push(Buffer.from(g.m.HEAPU8.slice(g.audio,g.audio+n*4)));
  }
  tracks.push(Buffer.concat(chunks));
 }
 assert.equal(tracks[0].length,tracks[1].length);
 const pcm=Buffer.alloc(tracks[0].length);let peak=0,last=0;
 // Identical native music timelines cancel, leaving just the dash effect.
 for(let i=0;i<pcm.length;i+=2){
  const delta=tracks[1].readInt16LE(i)-tracks[0].readInt16LE(i);
  peak=Math.max(peak,Math.abs(delta));if(delta)last=i;
  assert.ok(Math.abs(delta)<32768,'Dash effect must not clip');pcm.writeInt16LE(delta,i);
 }
 assert.ok(peak>1000,'Dash must be audible');
 const duration=last/(rate*4);assert.ok(duration>.04&&duration<.25,`Dash should finish promptly, lasted ${duration}s`);
 const h=Buffer.alloc(44);h.write('RIFF');h.writeUInt32LE(36+pcm.length,4);h.write('WAVEfmt ',8);h.writeUInt32LE(16,16);h.writeUInt16LE(1,20);h.writeUInt16LE(2,22);h.writeUInt32LE(rate,24);h.writeUInt32LE(rate*4,28);h.writeUInt16LE(4,32);h.writeUInt16LE(16,34);h.write('data',36);h.writeUInt32LE(pcm.length,40);
 fs.writeFileSync('artifacts/dash-effect.wav',Buffer.concat([h,pcm]));
 console.log(`PASS: native dash effect is audible, unclipped, and ends after ${(duration*1000).toFixed(0)}ms.`);
})().catch(e=>{console.error(e);process.exit(1)});
