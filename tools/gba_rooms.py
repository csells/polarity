"""Advance route compositions, with a full-screen pause between each challenge.

The underlying jump/dash vocabulary is shared with Color. Advance adds three
64px transition courts per route and makes the first lesson a roof walk. These
are authored into collision data, not stretched by the camera or emulator.
"""
from pathlib import Path
import rooms as original

W,H=92,18
rooms=[]
for n,source in enumerate(original.rooms):
    g=[[] for _ in range(H)]
    for x in range(80):
        for y in range(H):g[y].append(source[y][x])
        if x in (22,42,62):
            for y in range(H):g[y].extend(list('####' if y in (2,17) else '    '))
    # Each district's new courts rehearse its own mechanic. They follow a
    # checkpoint, with a safe landing after the new feature.
    for act,x in enumerate((23,47,71)):
        region=n//3
        if region==0:
            if n==0:
                g[16][x:x+3]=list('###')
            elif n==1:
                g[14][x:x+3]=list('###')
            else:
                g[15][x:x+2]=list('##');g[13][x+2:x+4]=list('##')
        elif region==1:
            g[14][x:x+4]=list('####')
            if act==1:g[13][x+1]='m'
        elif region==2:
            for y in range(9,16):g[y][x+1]='r'
            g[11][x+2:x+4]=list('##')
        elif region==3:
            g[16][x+1]='j';g[10][x+2:x+4]=list('##')
        elif region==4:
            for y in range(8,16):g[y][x:x+2]=list('ww')
            g[12][x+2:x+4]=list('##')
        else:
            g[13][x:x+4]=list('####')
            # A pulse behind the upper court is optional to cross; the court
            # offers a clear sheltered place to observe the timing.
            for y in range(8,13):g[y][x+2]='h'
    # The first route's early steps read as complete roofs, with no lethal pits.
    if n==0:
        for x in range(3,19):
            surfaces=[y for y in range(4,17) if g[y][x]=='#']
            if surfaces:
                for y in range(min(surfaces)+1,17):
                    if g[y][x]==' ':g[y][x]='#'
    # Later crossings ask players to use the district mechanic instead of
    # walking along the floor underneath it. The checkpoint plazas stay safe.
    if n>=3:
        for act,x in enumerate((23,47,71)):
            if n%3==0 and act<2:continue  # Teach, rehearse, then test.
            width=4 if n%3<2 else 6
            for xx in range(x,x+width):
                if g[16][xx]==' ':g[16][xx]='^'
            if region==1:
                g[13][x+1]='m'  # A guarded gantry above the furnace floor.
            elif region==2 and n%3==2:
                for y in (9,10):
                    if g[y][x+3]==' ':g[y][x+3]='a' if act%2==0 else 'b'
            elif region==3 and n%3==2:
                for y in (8,9):
                    if g[y][x+3]==' ':g[y][x+3]='b' if act%2==0 else 'a'
            elif region==4 and n%3==2:
                for y in (10,11):
                    if g[y][x+3]==' ':g[y][x+3]='a' if act%2==0 else 'b'
            elif region==5:
                for y in (14,15):
                    if g[y][x+2]==' ':g[y][x+2]='h'
    assert all(len(row)==W for row in g)
    flat=''.join(map(''.join,g))
    for c in 'uvlE':assert flat.count(c)==1,(n,c)
    assert flat.count('c')==3
    assert flat.count('m')<=8,(n,'Too many sentries for the engine')
    rooms.append(g)

out=['#include <stdint.h>']
for i,g in enumerate(rooms):
    flat=''.join(map(''.join,g));rle=[];last=flat[0];count=0
    for c in flat:
        if c==last and count<255:count+=1
        else:rle.extend([count,ord(last)]);last=c;count=1
    rle.extend([count,ord(last),0])
    out.append(f'const uint8_t gba_room_{i}[]={{'+','.join(map(str,rle))+'};')
out.append('const uint8_t *const levels[18]={'+','.join(f'gba_room_{i}' for i in range(18))+'};')
out.append('const char *const names[18]={'+','.join('"'+n+'"' for n in original.names)+'};')
out.append('const char *const locations[6]={'+','.join('"'+n+'"' for n in original.locations)+'};')
Path('src/gba/generated/levels.c').write_text('\n'.join(out)+'\n')
print('Advance routes: 18 rooms, 1472px wide, 54 new transition courts',flush=True)
