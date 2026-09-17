const fs=require('node:fs'),zlib=require('node:zlib'),path=require('node:path');
const root=path.resolve(__dirname,'..');const create=require('../web/vendor/mgba/mgba.js');
const symbols=Object.fromEntries(fs.readFileSync(root+'/build/polarity-gba.sym','utf8').trim().split('\n').map(l=>{const [a,,n]=l.split(/\s+/);return[n,parseInt(a,16)];}));
function png(bytes,w=240,h=160){function crc(b){let c=0xffffffff;for(const v of b){c^=v;for(let n=0;n<8;n++)c=c&1?(c>>>1)^0xedb88320:c>>>1;}return(c^0xffffffff)>>>0;}function chunk(t,b){const h=Buffer.alloc(8),c=Buffer.alloc(4);h.writeUInt32BE(b.length);h.write(t,4);c.writeUInt32BE(crc(Buffer.concat([Buffer.from(t),b])));return Buffer.concat([h,b,c]);}const header=Buffer.alloc(13);header.writeUInt32BE(w);header.writeUInt32BE(h,4);header[8]=8;header[9]=6;const rows=[];for(let y=0;y<h;y++)rows.push(Buffer.from([0]),Buffer.from(bytes.slice(y*w*4,(y+1)*w*4)));return Buffer.concat([Buffer.from('89504e470d0a1a0a','hex'),chunk('IHDR',header),chunk('IDAT',zlib.deflateSync(Buffer.concat(rows))),chunk('IEND',Buffer.alloc(0))]);}
async function boot(save){const m=await create();m._mgbawasm_init();m._mgbawasm_set_log_level(0);const rom=fs.readFileSync(root+'/web/polarity.gba');const ptr=m._malloc(rom.length);m.HEAPU8.set(rom,ptr);if(!m._mgbawasm_load(ptr,rom.length,0,0,0,0,1))throw Error('Cartridge failed');m._free(ptr);m._mgbawasm_set_idle_optimization(1);if(save){for(let i=0;i<8;i++)m._mgbawasm_run_frame();const p=m._malloc(save.length);m.HEAPU8.set(save,p);m._mgbawasm_sram_load(p,save.length);m._free(p);m._mgbawasm_reset();}const size=m._mgbawasm_state_size(),state=m._malloc(size),audio=m._malloc(32768);let current;
 function snapshot(){m._mgbawasm_state_save(state);current=new DataView(m.HEAPU8.buffer,state,size);return current;}
 function offset(n){const a=typeof n==='number'?n:symbols[n];return a>=0x03000000?0x19000+a-0x03000000:0x21000+a-0x02000000;}
 function read(n,bytes=4){return current[bytes===1?'getUint8':bytes===2?'getUint16':'getUint32'](offset(n),true);}
 function run(keys=0,n=1){m._mgbawasm_set_keys(keys);for(let i=0;i<n;i++){m._mgbawasm_run_frame();m._mgbawasm_read_audio(audio,8192);}snapshot();}
 function tap(keys){run(0,2);const mode=read("game_mode");run(keys);if((keys&9)&&mode!==1){for(let i=0;i<7&&read("game_mode")===mode;i++)run(keys);}m._mgbawasm_set_keys(0);}
 function capture(name){fs.writeFileSync(root+'/artifacts/'+name+'.png',png(m.HEAPU8.slice(m._mgbawasm_video_ptr(),m._mgbawasm_video_ptr()+240*160*4)));}
 function sram(){const n=m._mgbawasm_sram_save();return m.HEAPU8.slice(m._mgbawasm_sram_ptr(),m._mgbawasm_sram_ptr()+n);}
 run(0,8);return {m,read,run,tap,capture,sram,snapshot,symbols,state,audio,offset};}
module.exports={boot,png,symbols};
