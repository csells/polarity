const assert=require('node:assert/strict');
const {boot}=require('./gba-core.cjs');
(async()=>{
 const g=await boot();g.tap(8);g.tap(1);g.tap(1);
 assert.equal(g.read('game_mode'),1);
 const before=g.read('simulation_frames');g.run(1,20);
 assert.equal(g.read('simulation_frames'),before,'Holding menu confirm must not advance a new room');
 assert.equal(g.read(g.symbols.player+20,1),0,'Hazards begin with gameplay input');
 g.run(16,10);
 assert.equal(g.read('simulation_frames')-before,10);
 assert.ok(g.read(g.symbols.player,2)>256,'Direction releases the entry latch immediately');
 g.tap(8);g.run(0,3);assert.equal(g.read('game_mode'),2);
 g.tap(1);g.run(0,3);assert.equal(g.read('game_mode'),1);
 for(let y=7;y<13;y++)for(let x=0;x<30;x++)
  assert.equal(g.read(g.symbols.overlay+(y*32+x)*2,2),0,'Native pause panel must clear on resume');
 console.log('PASS: held menu confirmation does not consume room timing; movement starts at native cadence.');
 console.log('PASS: native pause/resume removes the pause panel.');
})().catch(e=>{console.error(e);process.exit(1)});
