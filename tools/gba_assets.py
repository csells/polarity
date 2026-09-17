"""Original indexed pixel art, laid out for GBA hardware tiles and sprites."""
from pathlib import Path
import math,random,ast
root=Path('src/gba/generated');root.mkdir(parents=True,exist_ok=True)
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
def tile8(g,x,y):return [row[x:x+8] for row in g[y:y+8]]
def tiles(g):return sum((pack(tile8(g,x,y)) for y in range(0,len(g),8) for x in range(0,len(g[0]),8)),[])
def color(h):h=h.lstrip('#');r,g,b=[int(h[i:i+2],16)//8 for i in (0,2,4)];return r|g<<5|b<<10
def blend(a,b,t):return tuple(round(x*(1-t)+y*t) for x,y in zip(a,b))
def pal(colors):return [color(c) for c in colors]+[0]*(16-len(colors))
# Ink / edge / face / bevel / glint are consistent in all districts.
themes=[('111d37','283653','405477','66849e','a3b9ba','e1cc9b'),('261c2c','4d303b','75424c','a3685b','e0a17c','ffe0a5'),('102c30','204541','356d54','65a276','a0d69d','e2edbc'),('20283e','364765','536d8e','87a4be','bdcdd2','ffe6bd'),('16213e','2a3e62','416691','689abc','a9dbe5','e5efdb'),('231b3b','403058','684774','98709c','d2a4c6','ffe4c4')]
base=[canvas(8,8) for _ in range(192)]
for ch,glyph in font.items():
 if ord(ch)>=128:continue
 base[ord(ch)]=canvas(8,8,2)
 for y,row in enumerate(glyph):
  for x,b in enumerate(row):
   if b=='1':base[ord(ch)][y][x+1]=1
base[32]=canvas(8,8,2)
# UI blank and divider.
base[1]=canvas(8,8,2);base[2]=canvas(8,8,3)
for tile in range(128,134):
 g=canvas(8,8,3);rect(g,0,0,8,1,4);rect(g,0,1,8,1,5);rect(g,0,7,8,1,2)
 if tile!=128:rect(g,0,0,8,2,3)
 for y in [3,6]:line(g,0,y,7,y,2)
 rect(g,3 if tile&1 else 6,3,1,3,2);rect(g,1,2,2,1,4)
 if tile==130:rect(g,0,0,1,8,5)
 if tile==131:rect(g,7,0,1,8,2)
 if tile==132: # metal deck
  g=canvas(8,8,3);rect(g,0,0,8,2,5);rect(g,0,6,8,2,2);rect(g,1,3,1,1,6);rect(g,6,3,1,1,6)
 if tile==133: # lush lip
  rect(g,0,0,8,2,5);rect(g,2,2,2,2,4);rect(g,6,2,1,3,4)
 base[tile]=g
# Object tiles: color and shape both communicate function.
for id in range(134,160):
 g=canvas(8,8)
 if id==134: # thorns
  for x in (1,5):line(g,x,1,x-2,7,2);line(g,x,1,x+2,7,1)
 elif id in (135,136):
  rect(g,2,0,4,8,2);rect(g,3,0,2,8,1)
  for y in (1,5):line(g,1,y,6,y+2,3)
 elif id in (137,138,139,140):
  disk(g,4,4,3,2);disk(g,4,4,2,1);rect(g,3,3,2,2,4 if id>=139 else 3)
  if id&1:rect(g,3,1,2,6,1);rect(g,1,3,6,2,1)
  else:line(g,1,4,4,1,1);line(g,4,1,6,4,1)
 elif id in (141,142):
  rect(g,0,2,8,2,1 if id==141 else 2);rect(g,0,4,8,2,2 if id==141 else 0)
  for x in (1,5):rect(g,x,3,2,1,3)
 elif id==143:
  rect(g,0,1,8,6,2);rect(g,1,2,6,4,1);line(g,0,1,4,4,3);line(g,4,4,7,1,3)
 elif id==144:
  rect(g,1,0,1,8,1);rect(g,2,1,5,3,3);rect(g,0,7,4,1,2)
 elif id==145:
  rect(g,0,1,8,2,1)
  for y in (3,5):line(g,2,y,5,y+1,3)
  rect(g,0,7,8,1,2)
 elif id==146:
  rect(g,3,0,2,8,1)
  for y in (1,5):line(g,2,y,5,y+1,2)
 elif id==147:
  line(g,3,2,3,6,2);line(g,1,3,3,1,1);line(g,3,1,5,3,1)
 elif id==148:
  for y in range(8):rect(g,2+(y%3),y,2,1,1 if y&1 else 3)
 elif id==149:
  disk(g,4,4,3,2);line(g,4,1,6,4,1);line(g,6,4,4,6,1);line(g,4,6,1,4,1);line(g,1,4,4,1,1)
 elif id==150:
  rect(g,1,0,6,8,2);rect(g,2,1,4,6,3);rect(g,3,2,2,4,1)
 elif id==151:rect(g,0,3,8,2,1)
 elif id==152:rect(g,3,0,2,8,1)
 else:disk(g,4,4,1,1)
 base[id]=g
logo=canvas(224,32)
for n,ch in enumerate('POLARITY'):
 for y,row in enumerate(font[ch]):
  for x,c in enumerate(row):
   if c=='1':
    rect(logo,16+n*24+x*4+1,y*4+3,4,4,2);rect(logo,16+n*24+x*4,y*4+1,4,4,1 if n<5 else 3)
logo_tiles=tiles(logo)
assets=[];maps=[];palettes=[]
for area in range(6):
 rng=random.Random(110+area);ink,shadow,face,edge,light,warm=themes[area]
 p=[0]*256
 sky0=[(12,20,43),(29,16,38),(10,30,40),(18,30,57),(9,22,50),(23,14,42)][area]
 sky1=[(79,69,99),(153,74,73),(75,129,118),(132,151,167),(60,102,158),(103,66,122)][area]
 colors=['%02x%02x%02x'%blend(sky0,sky1,i/15) for i in range(16)]
 p[0:16]=pal(colors);p[16:32]=pal(['000000',ink,shadow,face,edge,light,warm])
 p[32:48]=pal(['000000',shadow,face,edge,light,warm]);p[48:64]=pal(['000000',ink,shadow,face,edge,light,warm])
 p[64:80]=pal(['000000','86f6e7','277b93','d1fff1','eafff7']);p[80:96]=pal(['000000','ffd28b','ae6446','fff0bf','fff9df'])
 p[96:112]=pal(['000000','ff8298','76394e','ffced0','ffefcf']);p[112:128]=pal(['000000','d9efbd','5e908c','ffffff','93f6cc'])
 p[128:144]=pal(['000000','e6eedf','121e35','ffc789','71e5dd','ffc789'])
 palettes.append(p)
 sky=canvas(256,160)
 for y in range(160):rect(sky,0,y,256,1,min(15,1+y*14//160))
 # The moon and stars stay in the far layer; nearer structures scroll faster.
 far=canvas(256,160);near=canvas(256,160)
 for _ in range(28):
  x,y=rng.randrange(256),rng.randrange(8,91);rect(far,x,y,1,1,4)
 disk(far,192,37,16,3);disk(far,186,33,15,0)
 for x in range(-8,256,24):
  top=rng.randrange(60,115);w=rng.randrange(14,24)
  rect(far,x,top,w,160-top,1);rect(far,x,top,w,2,2)
  for yy in range(top+6,148,9):
   for xx in range(x+3,x+w-2,6):rect(far,xx,yy,2,3,3 if rng.random()<.25 else 2)
  line(far,x+w//2,top,x+w//2,top-8,2)
 if area==2:
  for x in (16,74,148,218):disk(far,x,80,24,2);disk(far,x-8,74,16,3);rect(far,x,80,3,64,1)
 elif area==3:
  for x,y in ((28,54),(122,30),(212,75)):
   disk(far,x,y,10,3);disk(far,x+13,y+3,8,3);rect(far,x-12,y+4,35,3,3)
 elif area==4:
  line(far,113,118,130,65,2);line(far,146,118,130,65,2);line(far,112,119,147,119,3)
  for a in range(180):
   t=a*math.pi/180;rect(far,130+int(math.cos(t)*19),62+int(math.sin(t)*11),1,1,4)
 elif area==5:
  for y in (45,55,65):
   for x in range(256):rect(far,x,y+(x//12%2)*3,1,1,2)
 for x in range(-8,256,48):
  h=rng.randrange(22,49);rect(near,x,160-h,32,h,1);rect(near,x-2,160-h,36,3,3)
  for yy in range(166-h,156,8):
   for xx in range(x+4,x+29,8):rect(near,xx,yy,3,4,4)
  if area==1:rect(near,x+7,124-h,9,36,2);rect(near,x+5,122-h,13,3,3)
  if area==2:
   for yy in range(110,150,8):disk(near,x+35,yy,3,3);line(near,x+35,105,x+35,160,2)
  if area in (3,4):line(near,x,146-h,x+40,142-h,3)
 # Deduplicate all backgrounds into one region-specific character set.
 data=sum((pack(t) for t in base),[]);lookup={};layer_maps=[]
 for img,bank in ((sky,0),(far,2),(near,3)):
  m=[0]*1024
  for y in range(20):
   for x in range(32):
    bits=tuple(pack(tile8(img,x*8,y*8)))
    if not any(bits):idx=0
    elif bits in lookup:idx=lookup[bits]
    else:idx=len(data)//32;lookup[bits]=idx;data.extend(bits)
    m[y*32+x]=idx|bank<<12
  layer_maps.append(m)
 assert len(data)//32<896,(area,len(data)//32)
 data.extend([0]*(896*32-len(data)));data.extend(logo_tiles)
 assets.append(data);maps.append(layer_maps)
 print(f'[{area+1}/6] GBA scenery: {len(lookup)} unique background tiles',flush=True)
# Animated, sixteen-pixel courier. The six-pixel collision core stays unchanged.
sprites=[]
for f in range(10):
 g=canvas(16,16);bob=1 if f in (2,4) else 0
 rect(g,5,1+bob,7,6,1);rect(g,6,0+bob,5,2,3);rect(g,5,2+bob,8,3,3);rect(g,6,2+bob,5,1,4)
 rect(g,7,5+bob,5,3,5);rect(g,10,5+bob,1,2,1);rect(g,12,5+bob,1,1,6)
 rect(g,5,8+bob,7,5,2);rect(g,7,8+bob,4,4,3);rect(g,5,9+bob,2,3,7)
 rect(g,3,8+bob,3,2,6);rect(g,1,7+(f&1),3,2,6);rect(g,0,6+(f&1),2,2,6)
 rect(g,5,12,3,3,1);rect(g,9,12,3,3,1)
 if f in (1,3):rect(g,3,14,4,2,4);rect(g,10,13,4,2,4)
 elif f in (5,7):rect(g,3,11,3,3,4);rect(g,11,11,3,3,4)
 else:rect(g,5,14,3,2,4);rect(g,10,14,3,2,4)
 if f==7:rect(g,0,10,4,2,3)
 sprites.append(g)
for area in range(6):
 g=canvas(16,16);rect(g,4,1,8,6,1);rect(g,5,3,7,6,5);rect(g,10,5,1,1,1);rect(g,4,9,9,5,3);rect(g,5,14,3,2,1);rect(g,10,14,3,2,1)
 if area==0:rect(g,3,1,10,3,6)
 if area==1:rect(g,3,1,10,4,6);rect(g,5,7,7,2,2)
 if area==2:rect(g,2,1,12,3,2);rect(g,7,0,5,1,6)
 if area==3:rect(g,5,3,8,2,4)
 if area==4:rect(g,4,1,8,3,4);rect(g,6,6,6,2,4)
 if area==5:rect(g,4,0,8,4,4);rect(g,6,9,4,5,6)
 sprites.append(g)
# Sentry, spark, particle, map cursor, and letter.
g=canvas(16,16);rect(g,3,5,10,8,1);rect(g,4,4,8,7,2);rect(g,5,5,6,2,6);rect(g,5,6,2,1,4);rect(g,9,6,2,1,4);rect(g,2,12,4,3,3);rect(g,10,12,4,3,3);sprites.append(g)
for f in range(4):
 g=canvas(16,16);r=4-f;disk(g,8,8,r,3);disk(g,8,8,max(1,r-2),4);sprites.append(g)
g=canvas(16,16);line(g,4,3,8,7,4);line(g,8,7,12,3,4);sprites.append(g)
g=canvas(16,16);rect(g,2,4,12,8,6);rect(g,3,5,10,6,4);line(g,2,4,8,9,3);line(g,8,9,13,4,3);sprites.append(g)
# 32px portraits are drawn from each resident, then framed with district trim.
portraits=[]
for area in range(6):
 g=canvas(32,32,1);rect(g,1,1,30,30,2);s=sprites[10+area]
 for y in range(15):
  for x in range(16):
   if s[y][x]:rect(g,x*2,y*2+2,2,2,s[y][x])
 portraits.append(g)
objpal=[]
for accent,shade,scarf in [('7df4e5','318a9c','ffb87d'),('ffcf88','ba7652','82eadc')]+[(themes[i][4],themes[i][2],themes[i][5]) for i in range(6)]:
 objpal+=pal(['000000','11182a',shade,accent,'f5f5d8','e9b899',scarf,'73566c'])
objpal+=pal(['000000','15243a','354e65','54768b','8db4bf','b6d0ca','e2c78c','58647c'])
objpal+=pal(['000000','152037','263249','37455b','4d5f73','667586','7a8794','465063'])
# map emblems are bespoke small architectural vignettes (4x4 hardware tiles).
icons=[]
for a in range(6):
 g=canvas(32,32);disk(g,16,18,13,2);rect(g,3,26,26,3,1)
 if a==0:
  rect(g,7,11,18,15,3)
  for y in range(8):rect(g,6+y,10-y,20-y*2,1,4)
  rect(g,14,18,5,8,6);rect(g,8,15,3,4,4);rect(g,21,15,3,4,4)
 elif a==1:
  rect(g,5,14,23,12,3);rect(g,8,3,5,14,2);rect(g,20,7,4,10,2)
  for x in (7,14,21):rect(g,x,17,4,5,6)
 elif a==2:
  rect(g,14,12,4,15,3)
  for x,y,r in ((9,13,6),(21,13,7),(15,7,7)):disk(g,x,y,r,4)
  rect(g,13,13,4,3,6)
 elif a==3:
  rect(g,7,16,20,8,3);rect(g,6,24,22,2,6);line(g,8,17,15,5,4);line(g,25,17,18,5,4);disk(g,16,5,4,6)
 elif a==4:
  rect(g,12,12,8,14,3);line(g,6,7,24,19,6);line(g,6,7,12,3,4);line(g,24,19,28,12,4);disk(g,14,9,4,4)
 else:
  disk(g,16,15,11,3);disk(g,16,15,8,6);disk(g,16,15,5,4);rect(g,14,5,4,20,4);rect(g,6,13,20,4,4)
 icons.append(g)
# C data has explicit sizes for direct DMA transfer, no runtime decompression.
out=['#include <stdint.h>']
def arr(name,data,typ='uint8_t'):out.append(f'const {typ} {name}[{len(data)}]={{'+','.join(map(str,data))+'};')
for a in range(6):
 arr(f'background_{a}',assets[a]);arr(f'palette_{a}',palettes[a],'uint16_t')
 for l in range(3):arr(f'map_{a}_{l}',maps[a][l],'uint16_t')
arr('sprite_data',sum((tiles(g) for g in sprites+portraits+icons),[]));arr('sprite_palette',objpal,'uint16_t')
out+=['const uint8_t *const backgrounds[6]={'+','.join(f'background_{a}' for a in range(6))+'};','const uint16_t *const region_palettes[6]={'+','.join(f'palette_{a}' for a in range(6))+'};','const uint16_t *const scenery_maps[6][3]={'+','.join('{'+','.join(f'map_{a}_{l}' for l in range(3))+'}' for a in range(6))+'};']
(root/'assets.c').write_text('\n'.join(out)+'\n')
(root/'assets.h').write_text(f'''#include <stdint.h>\nextern const uint8_t *const backgrounds[6];\nextern const uint16_t *const region_palettes[6];\nextern const uint16_t *const scenery_maps[6][3];\nextern const uint8_t sprite_data[{(len(sprites)*4+12*16)*32}];\nextern const uint16_t sprite_palette[160];\n#define BACKGROUND_BYTES {len(assets[0])}\n#define PORTRAIT_TILE {len(sprites)*4}\n#define ICON_TILE {len(sprites)*4+6*16}\n''')
print(f'Art built: {len(sprites)} animated/object frames, six portraits, six map landmarks')
