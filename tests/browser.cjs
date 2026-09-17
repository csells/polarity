const {chromium}=require('playwright');
const fs=require('fs');
(async()=>{
 const browser=await chromium.launch({channel:'chrome',headless:true});
 const page=await browser.newPage({viewport:{width:1360,height:1100},deviceScaleFactor:1});
 const errors=[];page.on('pageerror',e=>errors.push(e.message));
 await page.goto('http://127.0.0.1:8787',{waitUntil:'domcontentloaded'});console.log('Page loaded');await page.waitForFunction(()=>window.polarity,{timeout:15000});
 await page.screenshot({path:'artifacts/desktop.png',fullPage:true,timeout:10000});console.log('Desktop captured');
 await page.click('#play');await page.waitForTimeout(800);
 await page.evaluate(()=>polarity.freeze());
 await page.screenshot({path:'artifacts/game.png'});
 console.log('Runtime:',await page.evaluate(()=>({frames:polarity.frames,playing:polarity.playing,canvas:document.querySelector('canvas').getBoundingClientRect().toJSON()})));
 // Expose the emulator symbols only to the test harness.
 const symbols={};for(const line of fs.readFileSync('build/polarity.noi','utf8').split('\n')){const m=line.match(/^DEF _(player|room_id|game_mode|deaths|tick) 0x([0-9A-F]+)/);if(m)symbols[m[1]]=parseInt(m[2],16)}
 console.log('Symbols:',symbols);fs.writeFileSync('artifacts/symbols.json',JSON.stringify(symbols));
 console.log('State:',await page.evaluate(s=>Object.fromEntries(Object.entries(s).map(([k,v])=>[k,Array.from({length:k==='player'?22:2},(_,i)=>polarity.read(v+i))])),symbols));
 await page.setViewportSize({width:390,height:844});await page.screenshot({path:'artifacts/mobile.png',fullPage:true});
 console.log('Mobile overflow:',await page.evaluate(()=>document.documentElement.scrollWidth>innerWidth));
 console.log('Errors:',errors);
 await browser.close();if(errors.length)process.exit(1);
})().catch(e=>{console.error(e);process.exit(1)});
