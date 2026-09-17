"""Six original 16-second PCM arrangements. No external samples or music assets."""
from pathlib import Path
import math,random,hashlib
p=Path('src/gba/generated');p.mkdir(parents=True,exist_ok=True)
signature=hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
stamp=p/'music.sha256'
rebuild=not stamp.exists() or stamp.read_text()!=signature
rate=16384;length=262144
motif=[0,7,12,9,7,4,2,7,0,7,11,14,12,7,4,2,4,9,12,16,14,12,9,7,2,7,11,14,12,9,7,0]
chords=[(0,4,7),(9,12,16),(5,9,12),(7,11,14)]
assembly=['.section .rodata',' .balign 4','.global music_tracks','music_tracks:']
assembly += [' .word music_'+str(i) for i in range(6)]
for area in range(6):
 filename=p/f'music-{area}.raw'
 if rebuild or not filename.exists():
  mix=[0.0]*length;rng=random.Random(400+area);root=[48,45,53,55,50,48][area]
  def note(start,duration,midi,volume,voice):
   freq=440*2**((midi-69)/12);n=int(duration*rate);begin=int(start*rate)
   for j in range(n):
    t=j/rate;phase=2*math.pi*freq*t;env=min(1,t/.006)*min(1,(duration-t)/.08)
    if voice=='bell':v=(math.sin(phase)+.3*math.sin(phase*2.01)*math.exp(-t*6)+.12*math.sin(phase*3))*math.exp(-t*3)
    elif voice=='bass':v=(math.sin(phase)+.15*math.sin(phase*2))*math.exp(-t*3)
    elif voice=='pad':v=(math.sin(phase)+.22*math.sin(phase*1.002)+.15*math.sin(phase*2))*min(1,t/.12)
    else:v=(math.sin(phase)+.3*math.sin(phase*2)+.12*math.sin(phase*3))*math.exp(-t*5)
    mix[(begin+j)%length]+=v*env*volume
  for beat in range(32):
   chord=chords[(beat//8)%4];time=beat*.5
   note(time,.45,root+12+motif[beat],.19,'bell' if area in (0,2,4) else 'pluck')
   note(time+.25,.32,root+12+chord[beat%3],.065,'bell')
   note(time,.45,root-12+chord[0],.16,'bass')
   if beat%4==0:
    for n in chord:note(time,1.98,root+n,.036,'pad')
   # Soft brushed drums, region-specific rhythmic accents.
   for j in range(2200):
    t=j/rate;noise=rng.uniform(-1,1)
    kick=math.sin(2*math.pi*(55*t+1.1*(1-math.exp(-t*30))))*math.exp(-t*32)
    v=kick*.17 if beat%2==0 else noise*math.exp(-t*45)*.045
    mix[int(time*rate)+j]+=v
   if area in (1,3,5):
    for j in range(550):mix[int((time+.25)*rate)+j]+=rng.uniform(-1,1)*.032*math.exp(-j/110)
  # Gentle cross-channel-ready mono mastering, leaving headroom for live PSG effects.
  raw=bytes((max(-120,min(120,round(math.tanh(v)*108)))&255) for v in mix)
  filename.write_bytes(raw)
 assembly += [' .balign 4',f'music_{area}:',f' .incbin "{filename}"',' .space 32']
 print(f'[{area+1}/6] Soundtrack: 16s PCM arrangement ready',flush=True)
(p/'music.s').write_text('\n'.join(assembly)+'\n')

stamp.write_text(signature)
