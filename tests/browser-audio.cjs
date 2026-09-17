const {chromium}=require('playwright'),assert=require('node:assert/strict');
(async()=>{
 const b=await chromium.launch({channel:'chrome',headless:true,args:['--disable-gpu']});
 try{
  const p=await b.newPage();
  await p.addInitScript(()=>{
   // A device audio clock can stall before the browser reports suspension.
   // No source finishes in this fixture; the player must bound its own queue.
   window.audioProbe={live:0,peak:0,created:0};
   window.AudioContext=class{
    constructor(){this.state='running';this.currentTime=0;this.destination={};}
    createGain(){return {gain:{value:1},connect(){}};}
    addEventListener(){} resume(){this.state='running';return Promise.resolve();} suspend(){this.state='suspended';return Promise.resolve();}
    createBuffer(channels,count){return {getChannelData(){return new Float32Array(count);}};}
    createBufferSource(){let connected=false;audioProbe.created++;return {start(){},stop(){},connect(){connected=true;audioProbe.live++;audioProbe.peak=Math.max(audioProbe.peak,audioProbe.live);},disconnect(){if(connected){connected=false;audioProbe.live--;}}};}
   };
  });
  await p.goto(process.env.POLARITY_URL||'http://127.0.0.1:8792');await p.waitForFunction(()=>window.polarity);await p.click('#play');
  const before=await p.evaluate(()=>{polarity.freeze();polarity.advance(70224*1200);return audioProbe;});
  assert.ok(before.created>=1200,'Exercise twenty seconds of real emulator audio production');
  assert.ok(before.peak<=12,`Stalled output retained ${before.peak} audio sources`);
  await p.click('#pause');assert.equal(await p.evaluate(()=>audioProbe.live),0,'Pausing releases all queued audio');
  console.log('PASS: stalled audio output cannot grow the source queue; pause releases every queued source.');
 }finally{await b.close();}
})().catch(e=>{console.error(e);process.exit(1)});
