"""Build GBA VRAM banks from original artwork and authored 16px terrain.

BG0/1/2 share charblock 0 (16KiB); the 128-color painted backdrop occupies
charblocks 1..3 up to 0xe000. Screenblocks 28..31 occupy the final 8KiB.
OBJ art has an independent 32KiB budget. No bitmap-mode CPU framebuffer.
"""
from pathlib import Path
import ast, struct, math
root=Path('src/gba/generated');root.mkdir(parents=True,exist_ok=True)
packed=Path('assets/gba/packed')
font=ast.literal_eval(next(n.value for n in ast.parse(Path('tools/make_assets.py').read_text()).body if isinstance(n,ast.Assign) and any(isinstance(t,ast.Name) and t.id=='font' for t in n.targets)))
def canvas(w,h,c=0):return [[c]*w for _ in range(h)]
def rect(g,x,y,w,h,c):
 for yy in range(max(0,y),min(len(g),y+h)):
  for xx in range(max(0,x),min(len(g[0]),x+w)):g[yy][xx]=c
def line(g,x,y,x2,y2,c):
 n=max(abs(x2-x),abs(y2-y))
 for i in range(n+1):rect(g,round(x+(x2-x)*i/max(1,n)),round(y+(y2-y)*i/max(1,n)),1,1,c)
def disk(g,x,y,r,c):
 for yy in range(y-r,y+r+1):
  for xx in range(x-r,x+r+1):
   if (xx-x)**2+(yy-y)**2<=r*r:rect(g,xx,yy,1,1,c)
def pack(g):return [g[y][x]|g[y][x+1]<<4 for y in range(len(g)) for x in range(0,len(g[0]),2)]
def tiles(g):return sum((pack([row[x:x+8] for row in g[y:y+8]]) for y in range(0,len(g),8) for x in range(0,len(g[0]),8)),[])
def color(h):r,g,b=[int(h[i:i+2],16)//8 for i in (0,2,4)];return r|g<<5|b<<10
def pal(colors):return list(map(color,colors))+[0]*(16-len(colors))
def readpal(name):return list(struct.unpack('<16H',(packed/(name+'.pal')).read_bytes()))
base=[canvas(8,8) for _ in range(128)]
for ch,glyph in font.items():
 if ord(ch)>=128:continue
 base[ord(ch)]=canvas(8,8,2)
 for y,row in enumerate(glyph):
  for x,b in enumerate(row):
   if b=='1':base[ord(ch)][y][x+1]=1
base[1]=canvas(8,8,2)
base[2]=canvas(8,8,3)
# The foreground has strong silhouettes and one-pixel highlights. Backdrop
# values are kept quieter by hardware alpha blending in the native renderer.
meta=[];ids={}
def add(name,g):ids[name]=128+len(meta)*4;meta.append(g)
for kind in range(8):
 g=canvas(16,16,2)
 # Staggered slate courses with broken highlights, not uniform checkerboard.
 for y in (1,6,11):
  for x in range(-5+(y%2)*7,16,13):
   rect(g,x,y,12,4,3+(x+y)%2);line(g,x+1,y,x+10,y,5);rect(g,x+2,y+2,3,1,4)
 if kind<4:
  rect(g,0,0,16,3,6);rect(g,0,0,16,1,8);rect(g,0,3,16,2,1)
  if kind==1:rect(g,0,0,2,16,6);rect(g,2,4,1,12,1)
  if kind==2:rect(g,14,0,2,16,1)
  if kind==3:
   for x in range(0,16,4):rect(g,x,2,2,4,7);rect(g,x+1,5,1,2,6)
 elif kind==5:
  rect(g,0,0,16,16,2);rect(g,0,0,16,2,5);rect(g,0,14,16,2,1)
  for x in (2,12):rect(g,x,4,2,2,7);rect(g,x,11,2,2,4)
  line(g,4,3,10,12,3)
 elif kind==6:
  rect(g,3,3,10,13,1);rect(g,4,4,8,12,9);rect(g,7,3,2,13,2);rect(g,3,8,10,2,2)
 elif kind==7:
  rect(g,0,0,16,2,1);rect(g,0,2,16,1,5)
 add(['ROOF','ROOF_LEFT','ROOF_RIGHT','MOSS','WALL','STEEL','WINDOW','CEILING'][kind],g)
g=canvas(16,16)
for x in (3,11):
 for y in range(3,16):rect(g,x-(y-3)//4,y,1+(y-3)//2,1,2)
 line(g,x,3,x-3,15,1);line(g,x,4,x+2,14,3)
add('SPIKES',g)
for name in ('GATE_CYAN','GATE_AMBER'):
 g=canvas(16,16);rect(g,4,0,8,16,2);rect(g,6,0,4,16,3);rect(g,7,0,2,16,1)
 for y in (2,10):
  if name.endswith('CYAN'):line(g,3,y+3,8,y,1);line(g,8,y,12,y+3,1)
  else:rect(g,2,y+1,12,2,1)
 add(name,g)
for name in ('SIGNAL_OFF','POWER_OFF','SIGNAL_ON','POWER_ON'):
 g=canvas(16,16);disk(g,8,8,7,2);disk(g,8,7,6,3);disk(g,8,7,4,2);line(g,3,3,6,1,1)
 if 'SIGNAL' in name:
  line(g,4,7,8,3,1);line(g,8,3,12,7,1);line(g,4,7,8,11,1);line(g,8,11,12,7,1)
 else:rect(g,6,3,4,9,1);rect(g,3,6,10,3,1)
 if name.endswith('ON'):disk(g,8,7,2,4)
 rect(g,6,14,4,2,3);add(name,g)
for name in ('BRIDGE_ON','BRIDGE_OFF'):
 g=canvas(16,16);on=name.endswith('_ON')
 rect(g,0,3,16,2,1 if on else 2)
 for x in (1,9):rect(g,x,6,5,1,3 if on else 2)
 if on:
  rect(g,0,5,16,5,2);line(g,0,10,7,5,3);line(g,8,5,15,10,3)
 add(name,g)
g=canvas(16,16);rect(g,1,3,14,10,2);rect(g,2,4,12,8,1);line(g,1,3,8,9,3);line(g,8,9,14,3,3);add('LETTER',g)
g=canvas(16,16);rect(g,3,1,2,15,2);rect(g,3,1,1,14,1);rect(g,5,2,9,6,3);line(g,6,3,11,5,1);rect(g,1,15,7,1,1);add('FLAG',g)
g=canvas(16,16);rect(g,1,2,14,3,1);rect(g,0,14,16,2,2)
for y in (5,8,11):line(g,4,y,11,y+2,3);line(g,11,y+2,4,y+3,1)
add('SPRING',g)
g=canvas(16,16);rect(g,7,0,3,16,2)
for y in range(0,16,4):line(g,6,y,10,y+3,1);rect(g,6,y,1,2,3)
add('ROPE',g)
g=canvas(16,16);line(g,7,5,7,12,2);line(g,4,7,7,4,3);line(g,7,4,10,7,3);add('WIND',g)
g=canvas(16,16);rect(g,5,0,6,16,2)
for y in range(0,16,4):line(g,4,y,11,y+3,1);line(g,11,y+3,4,y+4,3)
add('COIL',g)
g=canvas(16,16);disk(g,8,8,6,2);line(g,8,2,13,8,1);line(g,13,8,8,14,3);line(g,8,14,3,8,3);line(g,3,8,8,2,1);disk(g,8,8,2,4);add('REFILL',g)
g=canvas(16,16);rect(g,0,0,16,16,2);rect(g,2,1,12,14,3);rect(g,4,3,8,12,2);rect(g,6,4,4,8,1);add('EXIT',g)
# Architectural props: lamps, roof finials, drainpipes, skylights and plant pots.
g=canvas(16,16);line(g,8,0,8,15,2);rect(g,3,3,10,9,2);rect(g,5,4,6,6,9);line(g,3,3,8,0,6);line(g,8,0,13,3,6);rect(g,7,4,1,6,6);add('LAMP',g)
g=canvas(16,16);line(g,7,3,7,15,6);disk(g,7,2,2,8);line(g,1,13,7,6,5);line(g,7,6,14,13,5);add('FINIAL',g)
g=canvas(16,16);rect(g,5,0,6,16,1);rect(g,6,0,3,16,5);rect(g,6,0,1,16,7);rect(g,4,6,8,3,6);add('PIPE',g)
g=canvas(16,16);rect(g,2,10,12,6,5);rect(g,3,12,10,4,3)
for x,y in ((4,6),(9,3),(12,7),(6,9)):disk(g,x,y,3,7);line(g,x,y,8,12,6)
add('PLANT',g)
g=canvas(16,16);rect(g,0,5,16,11,2);line(g,0,5,6,0,8);line(g,6,0,15,5,8);rect(g,3,5,10,8,9);rect(g,7,3,2,13,5);rect(g,0,13,16,3,6);add('SKYLIGHT',g)
g=canvas(16,16);rect(g,1,1,14,15,2);rect(g,2,2,12,13,6);rect(g,4,4,8,8,1);line(g,4,4,11,11,5);line(g,11,4,4,11,5);add('CRATE',g)
# One 16px map marker, with clear locked/open/restored palette states.
g=canvas(16,16);disk(g,8,8,6,2);disk(g,8,8,4,3);disk(g,8,8,2,1);add('MARKER',g)
logo=canvas(224,32)
for n,ch in enumerate('POLARITY'):
 for y,row in enumerate(font[ch]):
  for x,c in enumerate(row):
   if c=='1':rect(logo,16+n*24+x*4+1,y*4+3,4,4,2);rect(logo,16+n*24+x*4,y*4+1,4,4,1 if n<5 else 3)
terrain_palettes=[
 ['101b2b','111b29','203348','344d62','486b7d','698ea0','a8c4c0','769773','e5e2bc','ffd38a'],
 ['171521','1d1a27','3b2935','663e48','97604f','bb8163','e4b686','8b995e','ffe2a0','ffbd69'],
 ['0d2028','0f2026','20403d','335b4d','557b58','84aa71','bcd49b','649455','e5edbe','ffdf9e'],
 ['16233c','172239','2d435c','4b667d','7695a3','9cbcbf','dbe3ca','8f9977','fff1c3','ffd999'],
 ['171d34','192139','2e3c60','4b6188','6c86af','9ab2cc','c4d5d9','678994','e8efd0','ffe1a4'],
 ['1b192d','211b32','39304e','5a486f','856789','ac8eab','d5bad0','849796','ffefdd','ffcf8c']]
terrain_palettes=[pal(['000000']+p[1:]) for p in terrain_palettes]
# Utility palette uses the same role indices for cyan and amber shapes.
utility=[pal(['000000','c3fff3','194c62','48b9bd','f4fff4']),pal(['000000','ffe3a9','633b38','df9d56','fff8d7']),pal(['000000','ffd3c2','553047','e76c7c','fff1d3']),pal(['000000','e3ffc7','2a544f','82b78a','ffffff'])]
ui=pal(['000000','f4efda','0b192b','eabe85','75d8d6'])
data=sum((pack(g) for g in base),[])+sum((tiles(g) for g in meta),[])
assert len(data)<=400*32
data += [0]*(400*32-len(data));data+=tiles(logo)
assert len(data)==16384
# OBJ data: 16 courier poses, six residents, six independently sampled portraits.
obj=list((packed/'courier.tiles').read_bytes());npc_start=len(obj)//32
for i in range(6):obj+=list((packed/f'neighbor-{i}.tiles').read_bytes()[:512])
portrait_start=len(obj)//32
for i in range(6):obj+=list((packed/f'neighbor-{i}.tiles').read_bytes()[512:])
fx_start=len(obj)//32
fx=[]
for f in range(12):
 g=canvas(16,16)
 if f<4:
  r=4-f;disk(g,8,8,r,3);disk(g,8,8,max(1,r-2),1)
 elif f==4:line(g,3,4,8,9,1);line(g,8,9,13,4,1)
 elif f==5:g=meta[list(ids).index('LETTER')]
 elif f==6:
  for x in (2,10):rect(g,x,11,5,3,2)
  rect(g,3,3,10,9,2);rect(g,4,2,8,7,3);rect(g,5,4,6,2,1);rect(g,7,8,2,3,4)
 elif f==7:g=meta[list(ids).index('MARKER')]
 elif f==8:line(g,8,5,6,10,3)
 elif f==9:rect(g,7,7,1,2,1);rect(g,8,6,1,1,3)
 elif f==10:rect(g,7,7,1,1,1)
 else:line(g,4,8,9,7,3)
 fx.append(g)
obj+=sum((tiles(g) for g in fx),[])
objpal=readpal('courier')+readpal('courier-amber')
for bank in range(2):
 for i in range(1,16):
  c=objpal[bank*16+i];r,g,b=c&31,(c>>5)&31,(c>>10)&31
  if max(r,g,b)>6:r=min(31,r*5//4+1);g=min(31,g*5//4+1);b=min(31,b*5//4+1)
  objpal[bank*16+i]=r|(g<<5)|(b<<10)
for i in range(6):objpal+=readpal(f'neighbor-{i}')
objpal+=sum(utility,[])+ui+pal(['000000','778594','233345','45566a','617588'])
objpal += [0]*(256-len(objpal));assert len(obj)<=32768
out=['#include <stdint.h>']
def arr(name,data,typ='uint8_t'):out.append(f'const {typ} {name}[{len(data)}]={{'+','.join(map(str,data))+'};')
for area in range(6):
 painted=list((packed/f'terrain-{area}.tiles').read_bytes())
 assert len(painted)==256
 regional=data[:]
 for name in ('ROOF','ROOF_LEFT','ROOF_RIGHT','MOSS','STEEL'):
  at=ids[name]*32;regional[at:at+128]=painted[:128]
 for name in ('WALL','CEILING'):
  at=ids[name]*32;regional[at:at+128]=painted[128:]
 arr(f'terrain_{area}',regional)
arr('sprite_data',obj);arr('sprite_palette',objpal,'uint16_t')
for i in range(7):
 arr(f'background_{i}',(packed/f'scene-{i}.tiles').read_bytes())
 p=list(struct.unpack('<128H',(packed/f'scene-{i}.pal').read_bytes()))+ui+readpal(f'terrain-{min(i,5)}')+sum(utility,[])+terrain_palettes[min(i,5)]*2
 assert len(p)==256;arr(f'palette_{i}',p,'uint16_t')
out+=['const uint8_t *const terrain_data[6]={'+','.join(f'terrain_{i}' for i in range(6))+'};','const uint8_t *const backgrounds[7]={'+','.join(f'background_{i}' for i in range(7))+'};','const uint16_t *const region_palettes[7]={'+','.join(f'palette_{i}' for i in range(7))+'};']
(root/'assets.c').write_text('\n'.join(out)+'\n')
(root/'assets.h').write_text('#include <stdint.h>\nextern const uint8_t *const backgrounds[7];\nextern const uint16_t *const region_palettes[7];\nextern const uint8_t *const terrain_data[6];\nextern const uint8_t sprite_data['+str(len(obj))+'];\nextern const uint16_t sprite_palette[256];\n#define BACKGROUND_BYTES 40960\n#define NPC_TILE '+str(npc_start)+'\n#define PORTRAIT_TILE '+str(portrait_start)+'\n#define FX_TILE '+str(fx_start)+'\n'+''.join(f'#define T_{name} {idx}\n' for name,idx in ids.items()))
print(f'Art built: seven 128-color scenes, {len(meta)} metatiles, 16 courier poses, six residents and portraits; OBJ {len(obj)}/32768 bytes',flush=True)
