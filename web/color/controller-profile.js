/* Device-local controller bindings, learned from the browser's actual reports. */
(function(root){
 const actions=['up','down','left','right','A','B','start','select'];
 const storageKey='polarity-controller-profiles-v1';
 const deviceKey=p=>JSON.stringify([p.id,p.mapping,p.buttons.length,p.axes.length]);
 const neutral=p=>({buttons:p.buttons.map(b=>b.pressed),axes:Array.from(p.axes)});
 function capture(p,base){
  const button=p.buttons.findIndex((b,i)=>b.pressed&&!base.buttons[i]);
  if(button>=0)return {type:'button',index:button};
  for(let i=0;i<p.axes.length;i++){
   const value=p.axes[i],rest=base.axes[i];
   if(Math.abs(value-rest)<.5)continue;
   if(Math.abs(rest)>1&&Math.abs(value)<=1)return {type:'hat',index:i,value};
   if(Math.abs(rest)<=1&&Math.abs(value)<=1)return {type:'axis',index:i,rest,sign:value>rest?1:-1};
  }
  return null;
 }
 function matches(p,b){
  if(b.type==='button')return !!p.buttons[b.index]?.pressed;
  const value=p.axes[b.index];if(!Number.isFinite(value))return false;
  if(b.type==='axis')return (value-b.rest)*b.sign>.3;
  // HID hats encode eight compass positions from -1 through +1, with an out-of-range neutral.
  if(Math.abs(value)>1)return false;
  const target=Math.round((b.value+1)*3.5),sector=Math.round((value+1)*3.5);
  const distance=Math.abs(target-sector);return Math.min(distance,8-distance)<=1;
 }
 function released(p,base){return p.buttons.every(b=>!b.pressed)&&p.axes.every((v,i)=>Math.abs(v-base.axes[i])<.15);}
 function valid(p,profile){return profile&&actions.every(a=>{
  const b=profile[a];if(!b||!Number.isInteger(b.index)||b.index<0)return false;
  if(b.type==='button')return b.index<p.buttons.length;
  if(b.index>=p.axes.length)return false;
  return b.type==='axis'?Number.isFinite(b.rest)&&Math.abs(b.rest)<=1&&[1,-1].includes(b.sign):b.type==='hat'&&Number.isFinite(b.value)&&Math.abs(b.value)<=1;
 });}
 const api={actions,deviceKey,neutral,capture,matches,released,valid,storageKey};
 if(typeof module!=='undefined'&&module.exports)module.exports=api;else root.ControllerProfile=api;
})(globalThis);
