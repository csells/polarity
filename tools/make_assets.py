from pathlib import Path
import json
# Each route is four screens wide. Safe flag plazas separate its four acts.
W,H=80,18
rooms=[]
names=[]
locations=['SPARK DISTRICT','IRON FOUNDRY','HANGING GARDENS','CLOUD WORKS','WIND OBSERVATORY','STORM HEART']
titles=[['ROOFTOP POST','SWITCH STREET','SIGNAL BRIDGE'],['SENTRY YARD','FURNACE WALK','NIGHT SHIFT'],['ROOTS AND ROPES','CANOPY CROSSING','GREEN ASCENT'],['FIRST LAUNCH','BOUNCE DEPOT','CLOUD EXPRESS'],['TAILWIND TRAIL','UPDRAFT ARRAY','EYE OF THE WIND'],['PULSE CHAMBER','STORM RELAY','THE LAST SIGNAL']]
def new():
 g=[list(' '*W) for _ in range(H)]
 g[2]=list('#'*W);g[17]=list('#'*W)
 for y in range(3,17):g[y][0]=g[y][-1]='#'
 return g
def line(g,x,y,n,c='#'):
 for i in range(n):g[y][x+i]=c
def gate(g,x,y,n,c):
 for i in range(n):g[y+i][x]=c
def put(g,x,y,c):g[y][x]=c
def platform(g,x,y,n):line(g,x,y,n)
def spikes(g,x,n):line(g,x,16,n,'^')
def rope(g,x,top=5,bottom=15):gate(g,x,top,bottom-top+1,'r')
def wind(g,x,n):
 for y in range(5,16):line(g,x,y,n,'w')
# Hand-authored routes: introduce, vary, combine each location's central idea.
for area in range(6):
 for stage in range(3):
  g=new()
  if area==0:
   if stage==0:
    spikes(g,9,2);platform(g,13,14,4);platform(g,25,15,4);spikes(g,30,3);platform(g,35,14,3);gate(g,49,3,14,'b');platform(g,53,14,4)
   elif stage==1:
    gate(g,10,3,14,'b');platform(g,14,14,3);spikes(g,26,4);platform(g,25,14,3);put(g,30,12,'o');platform(g,33,14,4);gate(g,48,3,14,'a');spikes(g,52,3)
   else:
    platform(g,6,14,4);spikes(g,10,6);put(g,12,11,'o');platform(g,16,14,3);gate(g,27,3,14,'b');platform(g,31,14,4);spikes(g,45,8);platform(g,44,14,3);put(g,49,11,'o');platform(g,53,13,5)
  elif area==1:
   put(g,10,15,'m');platform(g,13,13,4);put(g,28,15,'m');platform(g,31,14,4);put(g,49,15,'m');platform(g,52,13,5)
   if stage>=1:
    spikes(g,16,2);gate(g,35,3,14,'b');platform(g,25,12,3);put(g,54,11,'m')
   if stage==2:
    put(g,6,15,'m');spikes(g,45,2);gate(g,53,3,10,'a');put(g,49,11,'o')
  elif area==2:
   rope(g,8);platform(g,10,10,6);spikes(g,11,6);rope(g,28);platform(g,30,9,6);spikes(g,31,7);rope(g,46);platform(g,49,8,8);spikes(g,49,8)
   if stage>=1:
    rope(g,14,5,10);rope(g,35,4,11);gate(g,33,3,6,'b');put(g,51,6,'o')
   if stage==2:
    put(g,25,15,'m');gate(g,51,3,5,'a');rope(g,53,4,9)
  elif area==3:
   put(g,8,16,'j');platform(g,12,9,6);spikes(g,12,6);put(g,26,16,'j');platform(g,30,8,7);spikes(g,30,8);put(g,44,16,'j');platform(g,49,8,8);spikes(g,49,8)
   if stage>=1:
    gate(g,33,3,5,'b');put(g,30,6,'o');put(g,53,6,'m')
   if stage==2:
    rope(g,35,4,9);gate(g,52,3,5,'a');put(g,47,6,'o')
  elif area==4:
   wind(g,7,3);platform(g,12,9,6);spikes(g,12,6);wind(g,25,4);platform(g,32,8,6);spikes(g,32,6);wind(g,43,4);platform(g,51,7,6);spikes(g,50,7)
   if stage>=1:
    gate(g,34,3,5,'b');rope(g,30,4,10);put(g,47,7,'o')
   if stage==2:
    put(g,14,7,'m');gate(g,53,3,4,'a');put(g,48,16,'j')
  else:
   gate(g,10,11,6,'h');platform(g,13,13,5);gate(g,27,10,7,'h');platform(g,31,13,6);gate(g,47,9,8,'h');platform(g,51,12,6)
   if stage>=1:
    put(g,7,16,'j');put(g,34,11,'m');rope(g,44,5,15);gate(g,54,3,9,'b')
   if stage==2:
    wind(g,25,3);gate(g,34,3,8,'a');put(g,31,7,'o');spikes(g,49,5);put(g,48,7,'o')
  # Tall retaining walls make climbing / launching meaningful, not optional scenery.
  if area in [2,3,4]:
   for x,y in [(17,10),(37,9),(57,8)]:gate(g,x,y,17-y,'#')
  # The fourth act resolves the route's central mechanic in a final delivery.
  if area==0:
   platform(g,65,14,3);spikes(g,68,6);put(g,70,11,'o');gate(g,73,3,10,'b' if stage!=1 else 'a')
  elif area==1:
   platform(g,64,13,4);put(g,70,15,'m');spikes(g,73,2)
   if stage:put(g,66,11,'m');gate(g,74,3,12,'b')
  elif area==2:
   rope(g,64+stage,4,15);platform(g,69,7,6);spikes(g,68,7);gate(g,74,7,10,'#');put(g,71,5,'o')
  elif area==3:
   put(g,64+stage,16,'j');platform(g,70,8,5);gate(g,74,8,9,'#');spikes(g,69,5);put(g,71,6,'o')
  elif area==4:
   wind(g,63+stage,4);platform(g,71,8,4);gate(g,74,8,9,'#');spikes(g,70,4);put(g,71,6,'o')
  else:
   gate(g,66,8,9,'h');gate(g,72,10,7,'h');platform(g,68,12-stage,3);put(g,69,9-stage,'o')
  # Safe arrivals, checkpoint plazas and exit landings are shared visual landmarks.
  for x in [20,40,60]:
   for xx in range(x-1,x+3):
    for y in range(12,17):g[y][xx]=' '
   put(g,x,15,'c')
  put(g,77,15,'E')
  rooms.append(g);names.append(titles[area][stage])
# RLE data keeps all eighteen long rooms in the cartridge's fixed ROM bank.
packed=[]
for g in rooms:
 flat=''.join(''.join(r) for r in g);runs=[];last=flat[0];n=0
 for c in flat:
  if c!=last or n==255:runs.extend([n,ord(last)]);last=c;n=0
  n+=1
 runs.extend([n,ord(last),0]);packed.append(runs)
code='#include "engine.h"\n'
for i,data in enumerate(packed):code+='static const uint8_t room_%d[]={%s};\n'%(i,','.join(map(str,data)))
code+='const uint8_t * const levels[ROOMS]={'+','.join('room_'+str(i) for i in range(len(rooms)))+'};\n'
code+='const char * const names[ROOMS]={'+','.join('"'+n+'"' for n in names)+'};\n'
code+='const char * const locations[LOCATIONS]={'+','.join('"'+n+'"' for n in locations)+'};\n'
Path('src/levels.c').write_text(code)
Path('tools/levels.json').write_text(json.dumps([''.join(''.join(r) for r in g) for g in rooms]))
# 2bpp pixel artwork, authored at native resolution.
patterns=[
['00000000']*8,
['22222222','33333333','31111113','31111113','31111113','31111113','31111113','33333333'],
['00020000','00023000','00223000','00223300','02223300','02223330','22223333','33333333'],
['00122100','00022000','00122100','00022000','00122100','00022000','00122100','00022000'],
['00133100','00033000','00133100','00033000','00133100','00033000','00133100','00033000'],
['00022000','00233200','02322320','23222232','23222232','02322320','00233200','00022000'],
['00020000','00232000','02333200','23333320','02333200','00232000','00020000','00000000'],
['00000000','00000000','00010000','00121000','00010000','00000000','00000000','00000000'],
['00000000','00000000','11111111','10000001','10200101','10000001','10010001','11111111'],
['00000000','00000000','00000000','00000000','00000000','00000000','00000000','11111111'],
]
patterns += [
['00022000','00033000','00022000','00322300','00022000','00033000','00022000','00322300'],
['22222222','03333330','00022000','00200200','02000020','00200200','00022000','33333333'],
['00222220','00233320','00233200','00222000','00200000','00200000','00200000','02220000'],
['00020000','00222000','02020200','00020000','00020000','00000000','00020000','00000000'],
['00011000','00122100','00033000','00122100','00033000','00122100','00011000','00011000'],
]
font={
'A':['01110','10001','10001','11111','10001','10001','10001'],
'B':['11110','10001','10001','11110','10001','10001','11110'],
'C':['01111','10000','10000','10000','10000','10000','01111'],
'D':['11110','10001','10001','10001','10001','10001','11110'],
'E':['11111','10000','10000','11110','10000','10000','11111'],
'F':['11111','10000','10000','11110','10000','10000','10000'],
'G':['01111','10000','10000','10111','10001','10001','01111'],
'H':['10001','10001','10001','11111','10001','10001','10001'],
'I':['111','010','010','010','010','010','111'],
'J':['00111','00010','00010','00010','10010','10010','01100'],
'K':['10001','10010','10100','11000','10100','10010','10001'],
'L':['10000','10000','10000','10000','10000','10000','11111'],
'M':['10001','11011','10101','10101','10001','10001','10001'],
'N':['10001','11001','10101','10011','10001','10001','10001'],
'O':['01110','10001','10001','10001','10001','10001','01110'],
'P':['11110','10001','10001','11110','10000','10000','10000'],
'Q':['01110','10001','10001','10001','10101','10010','01101'],
'R':['11110','10001','10001','11110','10100','10010','10001'],
'S':['01111','10000','10000','01110','00001','00001','11110'],
'T':['11111','00100','00100','00100','00100','00100','00100'],
'U':['10001','10001','10001','10001','10001','10001','01110'],
'V':['10001','10001','10001','10001','10001','01010','00100'],
'W':['10001','10001','10001','10101','10101','10101','01010'],
'X':['10001','10001','01010','00100','01010','10001','10001'],
'Y':['10001','10001','01010','00100','00100','00100','00100'],
'Z':['11111','00001','00010','00100','01000','10000','11111'],
'0':['01110','10001','10011','10101','11001','10001','01110'],
'1':['010','110','010','010','010','010','111'],
'2':['01110','10001','00001','00010','00100','01000','11111'],
'3':['11110','00001','00001','01110','00001','00001','11110'],
'4':['10010','10010','10010','11111','00010','00010','00010'],
'5':['11111','10000','10000','11110','00001','00001','11110'],
'6':['01110','10000','10000','11110','10001','10001','01110'],
'7':['11111','00001','00010','00100','01000','01000','01000'],
'8':['01110','10001','10001','01110','10001','10001','01110'],
'9':['01110','10001','10001','01111','00001','00001','01110'],
'-':['00000','00000','00000','11111','00000','00000','00000'],
'/':['00001','00001','00010','00100','01000','10000','10000'],
'+':['00000','00100','00100','11111','00100','00100','00000'],
'.':['000','000','000','000','000','010','010'],
'!':['010','010','010','010','010','000','010']}
while len(patterns)<32:patterns.append(['00000000']*8)
for code in range(32,91):
 rows=font.get(chr(code),['00000']*7)
 patterns.append(['0'+r.replace('1','2').ljust(7,'0') for r in rows]+['00000000'])
def pack(rows):
 out=[]
 for row in rows:
  out += [sum((int(v)&1)<<(7-i) for i,v in enumerate(row)),sum(((int(v)>>1)&1)<<(7-i) for i,v in enumerate(row))]
 return out
sprites=[['00111100','01222210','12222221','12233331','12230301','12233331','01222210','00122100'],['01222210','01222210','00111100','00100100','01100110','00000000','00000000','00000000'],['00000000','00010000','00122000','01233210','00122000','00010000','00000000','00000000']]
sprites.append(['00111100','01233210','12300321','12333321','01222210','00111100','01100110','01000010'])
Path('src/art.h').write_text('const unsigned char tiles[]={'+','.join(str(n) for p in patterns for n in pack(p))+'};\nconst unsigned char sprites[]={'+','.join(str(n) for p in sprites for n in pack(p))+'};\n#define TILE_COUNT '+str(len(patterns))+'\n')
print('Generated 18 scrolling rooms,',len(patterns),'background tiles and 4 sprite tiles')
