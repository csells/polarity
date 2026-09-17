"""Six original 32-second stereo scores, sharing Lumen's four-note theme.

Generated offline at 16,384Hz for the GBA's two hardware Direct Sound FIFOs.
Each district has its own orchestration, register, rhythm and second phrase.
No downloaded samples or music; deterministic synthesis with looped reverb.
"""
from pathlib import Path
import math,random,hashlib
p=Path('src/gba/generated');p.mkdir(parents=True,exist_ok=True)
signature=hashlib.sha256(Path(__file__).read_bytes()).hexdigest();stamp=p/'music.sha256'
rebuild=not stamp.exists() or stamp.read_text()!=signature
rate=16384;length=524288
# A question answered in the second half: signal rising, power returning home.
melody=[0,7,12,11,9,7,4,2, 0,4,7,9,12,11,7,2,
        4,7,12,14,16,14,12,9, 7,11,14,12,9,7,4,0]
chords=[(0,4,7),(9,12,16),(5,9,12),(7,11,14),
        (0,4,7),(5,9,12),(9,12,16),(7,11,14)]
assembly=['.section .rodata',' .balign 4','.global music_left','music_left:']
assembly+=[' .word music_l_'+str(i) for i in range(6)]
assembly+=['.global music_right','music_right:']+[' .word music_r_'+str(i) for i in range(6)]
for area in range(6):
 files=[p/f'music-{area}-{side}.raw' for side in ('l','r')]
 if rebuild or not all(f.exists() for f in files):
  mix=[[0.0]*length for _ in range(2)];rng=random.Random(400+area);root=[48,45,53,55,50,48][area]
  def note(start,duration,midi,volume,voice,pan=0):
   freq=440*2**((midi-69)/12);begin=int(start*rate);n=int(duration*rate)
   left=math.sqrt((1-pan)/2)*volume;right=math.sqrt((1+pan)/2)*volume
   for j in range(n):
    t=j/rate;phase=2*math.pi*freq*t
    env=min(1,t/.009)*min(1,(duration-t)/.08)
    if voice=='bell':v=(math.sin(phase)+.34*math.sin(phase*2.005)*math.exp(-t*4)+.12*math.sin(phase*3))*math.exp(-t*2.8)
    elif voice=='bass':v=(math.sin(phase)+.22*math.sin(phase*2))*math.exp(-t*2.4)
    elif voice=='strings':v=(math.sin(phase)+.23*math.sin(phase*2)+.12*math.sin(phase*3)+.08*math.sin(phase*4))*min(1,t/.15)*(.88+.12*math.sin(t*31))
    elif voice=='flute':v=(math.sin(phase+.012*math.sin(t*29))+.09*math.sin(phase*2))*(.85+.15*math.sin(t*3))
    else:v=(math.sin(phase)+.4*math.sin(phase*2)+.2*math.sin(phase*3)+.08*math.sin(phase*5))*math.exp(-t*4)
    v*=env;at=(begin+j)%length;mix[0][at]+=v*left;mix[1][at]+=v*right
  voice=['bell','pluck','flute','strings','bell','pluck'][area]
  for bar in range(16):
   t=bar*2;chord=chords[bar%8];second=bar>=8
   for k,n in enumerate(chord):note(t,2.25,root+n,.040 if area!=5 else .031,'strings',(-.55,0,.55)[k])
   for beat in range(4):
    at=t+beat*.5
    note(at,.43,root-12+chord[0],.16 if area in (1,5) else .12,'bass',-.08)
    if beat in (0,2) or second:
     idx=((bar%8)*4+beat)%32
     note(at,.70 if voice in ('flute','strings') else .58,root+12+melody[idx],.20,voice,-.18)
    # The answering voice enters only after the first eight bars.
    if second and beat in (1,3):note(at+.25,.38,root+24+chord[(bar+beat)%3],.075,'bell',.48)
    if area in (0,2,4):note(at+.25,.34,root+12+chord[beat%3],.050,'pluck',.35)
    if area in (1,3,5) or second:
     for j in range(1800):
      q=j/rate;noise=rng.uniform(-1,1)
      kick=math.sin(2*math.pi*(49*q+1.2*(1-math.exp(-q*35))))*math.exp(-q*35)
      v=kick*.14 if beat%2==0 else noise*math.exp(-q*43)*.045
      atj=int(at*rate)+j;mix[0][atj]+=v*.8;mix[1][atj]+=v*.7
     if area in (1,5):
      for j in range(650):
       v=rng.uniform(-1,1)*.025*math.exp(-j/140);atj=int((at+.25)*rate)+j;mix[1][atj]+=v
   if bar%4==3:print(f'[{area+1}/6] Score: {bar+1}/16 bars synthesized',flush=True)
  # Three quiet cross-channel reflections wrap at the loop boundary.
  dry=[v[:] for v in mix]
  for delay,gain in ((.113,.12),(.227,.07),(.349,.04)):
   offset=int(delay*rate)
   for i in range(length):
    mix[0][i]+=dry[1][(i-offset)%length]*gain;mix[1][i]+=dry[0][(i-offset)%length]*gain
  for side,f in enumerate(files):f.write_bytes(bytes(max(-120,min(120,round(math.tanh(v)*110)))&255 for v in mix[side]))
 for side,f in zip(('l','r'),files):assembly+=[' .balign 4',f'music_{side}_{area}:',f' .incbin "{f}"',' .space 32']
 print(f'[{area+1}/6] 32-second stereo score ready',flush=True)
(p/'music.s').write_text('\n'.join(assembly)+'\n');stamp.write_text(signature)
