from pathlib import Path
import sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from rooms import rooms,names,W,H
assert len(rooms)==18
for i,g in enumerate(rooms):
 assert len(names[i])<=20
 flat=''.join(map(''.join,g));assert len(flat)==W*H
 for c in 'uvlE':assert flat.count(c)==1,(i,c)
 assert flat.count('c')==3
 for x in [20,40,60]:assert g[15][x]=='c' and all(g[y][x]==' ' for y in range(12,15))
for a in range(6):
 for i in range(3):
  for j in range(i+1,3):
   # Compare authored structural geometry, excluding the shared roof/floor/checkpoint zones.
   left={(x,y,c) for y,row in enumerate(rooms[a*3+i]) for x,c in enumerate(row) if 3<=y<17 and 1<x<79 and c=='#'}
   right={(x,y,c) for y,row in enumerate(rooms[a*3+j]) for x,c in enumerate(row) if 3<=y<17 and 1<x<79 and c=='#'}
   overlap=len(left&right)/len(left|right)
   assert overlap<.6,(a,i,j,overlap)
print('18 rooms: unique platform compositions, intact checkpoint plazas, one of each socket/letter, readable titles.')
