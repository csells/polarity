const assert=require('node:assert/strict');
const {boot}=require('./gba-core.cjs');

(async()=>{
 const g=await boot(),failures=[];
 g.tap(8);g.tap(1);g.tap(1);g.run(0,30);
 function stationary(label){
  const poses=new Set(),cameras=new Set(),positions=new Set();
  for(let i=0;i<120;i++){
   g.run();
   poses.add([g.read('objects',2),g.read(g.symbols.objects+2,2),g.read(g.symbols.objects+4,2)].join(','));
   cameras.add([g.read('camera'),g.read('camera_y')].join(','));
   positions.add(g.read(g.symbols.player+2,2));
  }
  // Exercise the existing fractional gravity cycle, rather than pausing physics.
  assert.ok(positions.size>1,'Physics must continue running while standing');
  for(const [name,values] of [['courier pose',poses],['landscape scroll',cameras]]){
   try{assert.equal(values.size,1,`${label}: ${name} must stay still without input`);}
   catch(e){failures.push(e.message);}
  }
  console.log(`${label}: ${poses.size} courier poses, ${cameras.size} camera positions`);
 }
 stationary('Starting floor');
 // Reach the raised roof through normal play, where the vertical camera is free.
 g.run(17,30);g.run(0,90);
 assert.ok(g.read('camera_y')>0&&g.read('camera_y')<144,'Roof must exercise unclamped camera tracking');
 stationary('Raised roof');
 // Airborne motion retains the GBA renderer's half-logical-pixel precision.
 g.run(1);
 for(let i=0;i<8;i++){
  g.run(1);
  const y=g.read(g.symbols.player+2,2),vy=g.read(g.symbols.player+6,2);
  assert.ok(vy&0x8000,'Courier must be ascending');
  assert.equal((g.read('objects',2)&255)+g.read('camera_y')+8,Math.floor(y/8));
 }
 assert.deepEqual(failures,[]);
 console.log('PASS: idle courier and landscape remain stable; airborne subpixel movement is preserved.');
})().catch(e=>{console.error(e);process.exit(1)});
