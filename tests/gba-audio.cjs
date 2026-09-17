const assert=require('node:assert/strict'),fs=require('node:fs'),crypto=require('node:crypto');const {boot}=require('./gba-core.cjs');
(async()=>{const hashes=new Set(),preview=[];
for(let region=0;region<6;region++){
 const save=new Uint8Array(32768);save.fill(255);save.fill(0,0,32);save[0]=65;save[1]=2;save[2]=region;save.fill(3,3,9);save[31]=37+region+18;
 const g=await boot(save);g.tap(8);g.tap(2);g.run(0,3);g.capture('gba-location-'+region);assert.equal(g.read('room_id'),region*3);
 const samples=[];const seconds=region===0?34:3,frames=Math.ceil(seconds*59.7275);
 for(let f=0;f<frames;f++){g.m._mgbawasm_set_keys(0);g.m._mgbawasm_run_frame();const n=g.m._mgbawasm_read_audio(g.audio,8192);samples.push(Buffer.from(g.m.HEAPU8.slice(g.audio,g.audio+n*4)));}
 const data=Buffer.concat(samples);let sum=0,peak=0;for(let i=0;i<data.length;i+=2){const v=data.readInt16LE(i);sum+=v*v;peak=Math.max(peak,Math.abs(v));}const rms=Math.sqrt(sum/(data.length/2));assert.ok(rms>300&&peak<32767,`Audio silent or clipped: ${rms}/${peak}`);
 if(region===0){let tail=0,n=0;for(let i=data.length-16384*4;i<data.length;i+=2){tail+=data.readInt16LE(i)**2;n++;}assert.ok(Math.sqrt(tail/n)>300,'Music failed after the 32-second DMA loop');}
 let stereo=0;for(let i=0;i<data.length;i+=4)stereo+=Math.abs(data.readInt16LE(i)-data.readInt16LE(i+2));assert.ok(stereo/(data.length/4)>20,'Stereo channels must differ');
 hashes.add(crypto.createHash('sha256').update(data.subarray(0,16384*4)).digest('hex'));preview.push(data.subarray(0,16384*4*2));console.log(`[${region+1}/6] PCM audio RMS ${rms.toFixed(0)}, peak ${peak}, ${seconds}s including ${region===0?'loop boundary':'regional arrangement'}`);
}
assert.equal(hashes.size,6);const pcm=Buffer.concat(preview),h=Buffer.alloc(44);h.write('RIFF');h.writeUInt32LE(36+pcm.length,4);h.write('WAVEfmt ',8);h.writeUInt32LE(16,16);h.writeUInt16LE(1,20);h.writeUInt16LE(2,22);h.writeUInt32LE(16384,24);h.writeUInt32LE(65536,28);h.writeUInt16LE(4,32);h.writeUInt16LE(16,34);h.write('data',36);h.writeUInt32LE(pcm.length,40);fs.writeFileSync('artifacts/gba-soundtrack.wav',Buffer.concat([h,pcm]));console.log('PASS: six distinct native PCM arrangements, no clipping, sustained playback across DMA loop.');
})().catch(e=>{console.error(e);process.exit(1)});
