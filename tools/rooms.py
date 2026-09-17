"""Authored room compositions. Coordinates are 8px tiles; each room has four acts.

Every room contains a cyan signal socket (u), amber power socket (v), a lost
letter (l), three safe checkpoint plazas, and a delivery beacon (E).
= and + are bridges restored by signal and power respectively.
"""
W,H=80,18
locations=['SPARK DISTRICT','IRON FOUNDRY','HANGING GARDENS','CLOUD WORKS','WIND OBSERVATORY','STORM HEART']
rooms=[];names=[];intent=[]
def begin(name,idea):
 g=[list(' '*W) for _ in range(H)]
 g[2]=list('#'*W);g[17]=list('#'*W)
 for y in range(3,17):g[y][0]=g[y][-1]='#'
 rooms.append(g);names.append(name);intent.append(idea);return g
def row(g,x,y,n,c='#'):
 for i in range(n):g[y][x+i]=c
def wall(g,x,y,n,c='#'):
 for i in range(n):g[y+i][x]=c
def platforms(g,*items):
 for x,y,n in items:row(g,x,y,n)
def spikes(g,x,n):row(g,x,16,n,'^')
def ropes(g,*items):
 for x,top,bottom in items:wall(g,x,top,bottom-top+1,'r')
def wind(g,x,top,n):
 for y in range(top,16):row(g,x,y,n,'w')
def put(g,x,y,c):
 assert g[y][x] in ' w',(names[-1],x,y,g[y][x],c)
 g[y][x]=c

g=begin('THE FIRST DELIVERY','Safe move/jump lesson; cyan signal builds a bridge, amber dash supplies power. No lethal hazards.')
platforms(g,(7,15,3),(10,14,3),(26,15,3),(45,15,4),(50,13,4),(55,11,4),(63,12,4),(69,14,5))
put(g,15,15,'u');put(g,35,15,'v');row(g,24,13,11,'=');row(g,58,11,5,'+');put(g,61,9,'l')

g=begin('SIGNAL ALLEY','Climb to the signal post, cross the restored cable, then flip safely before the amber gate.')
platforms(g,(5,15,3),(10,13,5),(26,14,3),(32,11,6),(45,15,4),(50,12,6),(66,14,3),(72,12,4))
put(g,12,12,'u');row(g,15,13,4,'=');spikes(g,29,2);wall(g,35,3,8,'b');put(g,53,11,'v');spikes(g,46,2);row(g,64,12,5,'+');put(g,72,9,'l');put(g,68,10,'o')

g=begin('ROOFTOP CIRCUIT','A rooftop ascent and a low return channel combine wall jumps, both gate colors and a power bridge.')
platforms(g,(4,15,4),(9,13,4),(14,11,5),(24,14,3),(28,11,6),(34,8,4),(44,15,3),(48,13,4),(53,10,5),(64,14,3),(68,11,6))
spikes(g,8,6);wall(g,17,11,6);put(g,30,10,'u');put(g,35,6,'l');wall(g,47,3,12,'b');put(g,55,9,'v');row(g,64,10,4,'+');put(g,65,8,'o');spikes(g,68,5)

g=begin('CLOCK IN','Meet one isolated sentry, then use low gantries to choose between stomping and dashing.')
platforms(g,(9,14,5),(25,12,5),(33,15,5),(46,13,5),(54,10,4),(65,14,4),(71,12,4))
put(g,8,15,'m');put(g,15,15,'u');put(g,28,10,'l');put(g,35,13,'m');put(g,48,12,'v');put(g,56,8,'m');row(g,63,12,7,'+');spikes(g,67,4)

g=begin('ASSEMBLY LINE','Two factory floors: take the upper signal line, drop through the machine bays, restore the freight bridge.')
platforms(g,(4,15,3),(8,12,3),(12,9,6),(25,14,4),(30,11,4),(35,8,4),(44,10,5),(51,13,7),(65,15,3),(70,12,5))
wall(g,17,9,8);put(g,14,8,'u');spikes(g,8,8);put(g,26,12,'m');put(g,36,6,'l');put(g,54,12,'v');put(g,46,8,'m');wall(g,49,3,10,'b');row(g,64,11,6,'+');put(g,66,9,'o')

g=begin('THE NIGHT CREW','A compact machine climb opens a signal bridge; a guarded power socket feeds the final furnace crossing.')
platforms(g,(5,13,4),(11,15,3),(15,10,4),(24,15,3),(28,13,3),(33,10,5),(45,14,4),(51,11,3),(55,8,3),(64,15,3),(69,12,6))
put(g,7,11,'m');put(g,16,8,'l');put(g,35,9,'u');row(g,27,10,5,'=');spikes(g,30,8);put(g,46,12,'m');put(g,56,7,'v');wall(g,53,3,8,'b');row(g,65,11,4,'+');spikes(g,68,7);put(g,71,9,'m')

g=begin('ROOT ACCESS','A safe first rope leads up to the greenhouse signal. Later ropes cross thorn beds and restore a canopy bridge.')
ropes(g,(7,6,15),(28,5,15),(49,4,15),(67,6,15));platforms(g,(10,10,8),(31,9,7),(52,8,6),(70,10,5))
put(g,13,9,'u');spikes(g,11,6);wall(g,17,10,7);put(g,35,7,'l');spikes(g,32,6);wall(g,37,9,8);put(g,54,7,'v');wall(g,57,8,9);spikes(g,52,5);row(g,64,9,5,'+');put(g,71,8,'o')

g=begin('THE WATER PATH','Follow irrigation terraces up and down, switching between ropes and restored signal ledges.')
ropes(g,(5,10,15),(15,4,12),(32,6,15),(45,8,15),(56,4,12),(73,4,15));platforms(g,(7,13,4),(12,9,6),(25,14,4),(34,10,4),(46,11,4),(53,7,5),(65,14,4),(69,11,3))
put(g,16,8,'u');row(g,25,10,7,'=');spikes(g,8,4);spikes(g,34,4);wall(g,37,10,7);put(g,55,6,'v');put(g,54,4,'l');wall(g,49,12,5);row(g,64,8,5,'+');spikes(g,67,7);wall(g,74,8,9)

g=begin('SEEDS IN THE SKY','A rising greenhouse wall becomes a rope-to-dash canopy traverse, with a letter above the return path.')
platforms(g,(5,15,3),(10,12,3),(15,9,4),(24,12,5),(31,7,7),(44,14,4),(50,10,3),(55,6,3),(65,13,3),(71,9,4))
ropes(g,(8,8,15),(16,4,10),(28,5,13),(47,5,15),(54,4,11),(68,5,15));put(g,17,8,'u');wall(g,18,9,8);spikes(g,11,6);put(g,35,5,'l');put(g,33,5,'m');put(g,56,5,'v');spikes(g,51,6);wall(g,57,6,11);row(g,64,9,7,'+');wall(g,73,3,6,'a');put(g,66,7,'o')

g=begin('LIFT OFF','One spring in a safe bay teaches steering; the delivery climbs successive freight docks.')
put(g,8,16,'j');platforms(g,(12,10,6),(25,15,4),(31,8,7),(45,13,5),(53,7,5),(65,12,4),(72,9,3))
put(g,14,9,'u');wall(g,17,10,7);put(g,27,14,'j');put(g,34,6,'l');put(g,47,12,'j');put(g,55,6,'v');wall(g,57,7,10);row(g,64,9,6,'+');spikes(g,66,7);put(g,69,7,'o')

g=begin('FREIGHT HOP','Launch from raised docks, descend to the power bay, and take a spring back up through the freight ceiling.')
platforms(g,(5,15,4),(11,12,4),(16,8,3),(24,10,6),(33,14,5),(44,15,4),(49,11,5),(55,8,3),(65,15,3),(70,10,5))
put(g,6,14,'j');put(g,17,7,'u');wall(g,18,8,9);put(g,26,8,'m');put(g,35,13,'j');put(g,51,10,'v');put(g,56,6,'l');wall(g,52,3,7,'b');put(g,66,14,'j');row(g,64,8,5,'+');spikes(g,69,6)

g=begin('THE CLOUD ENGINE','Connect a high signal manifold before descending through alternating launch bays to power the main lift.')
platforms(g,(4,14,5),(12,8,6),(25,12,4),(32,6,6),(45,15,3),(51,10,7),(65,13,5),(72,7,3))
put(g,6,13,'j');put(g,15,7,'u');wall(g,17,8,9);put(g,27,11,'j');put(g,35,4,'l');wall(g,34,3,3,'b');put(g,46,14,'j');put(g,54,9,'v');put(g,56,8,'m');wall(g,57,10,7);put(g,67,12,'j');row(g,64,7,5,'+');spikes(g,69,5);put(g,70,6,'o')

g=begin('READ THE WIND','A sheltered updraft leads to the signal vane. Ride a wider current to the power station, then dash out to land.')
wind(g,7,5,3);wind(g,28,4,4);wind(g,47,4,3);wind(g,66,5,3);platforms(g,(12,10,6),(34,8,4),(52,9,6),(72,9,3))
put(g,14,9,'u');wall(g,17,10,7);put(g,36,6,'l');spikes(g,33,5);put(g,54,8,'v');wall(g,57,9,8);row(g,64,8,8,'+');put(g,73,7,'o')

g=begin('CROSSWINDS','Offset air shafts require committing to a landing. The signal builds a sheltered ledge between currents.')
wind(g,4,7,3);wind(g,15,4,3);wind(g,25,6,3);wind(g,36,4,2);wind(g,44,5,3);wind(g,55,4,3);wind(g,68,4,3)
platforms(g,(8,12,5),(14,8,5),(29,11,5),(34,7,4),(49,13,4),(54,8,4),(65,12,3),(72,7,3))
put(g,16,7,'u');put(g,36,5,'l');row(g,29,8,5,'=');spikes(g,8,5);wall(g,18,8,9);put(g,56,7,'v');spikes(g,49,6);wall(g,57,8,9);row(g,63,9,5,'+');wall(g,73,3,4,'a')

g=begin('THE WEATHER EYE','Climb the observatory terraces, use a rope to leave the wind, then thread the charge gates around the telescope.')
wind(g,7,4,3);wind(g,31,4,3);wind(g,45,6,3);wind(g,64,4,3);platforms(g,(12,8,6),(25,14,4),(34,10,4),(49,12,4),(54,7,4),(68,11,3),(73,6,2))
put(g,14,7,'u');wall(g,17,8,9);ropes(g,(27,6,15),(53,4,13));put(g,35,8,'m');put(g,26,4,'l');put(g,56,6,'v');wall(g,55,3,4,'b');wall(g,57,7,10);row(g,65,7,6,'+');spikes(g,68,6);put(g,72,5,'o')

g=begin('THE QUIET PULSE','Observe a single coil from safety, restore the timing signal, and cross the now-visible reactor rhythm.')
platforms(g,(6,14,5),(14,12,4),(25,15,3),(30,11,5),(45,13,5),(54,10,4),(65,14,4),(71,11,4))
wall(g,10,8,9,'h');put(g,16,11,'u');row(g,25,11,5,'=');wall(g,34,7,10,'h');put(g,32,8,'l');put(g,56,9,'v');wall(g,48,9,8,'h');row(g,64,11,5,'+');wall(g,72,7,10,'h')

g=begin('BORROWED LIGHT','All restored districts help: a freight spring, greenhouse rope, and observatory current bridge the broken reactor.')
platforms(g,(5,15,4),(12,9,6),(25,13,4),(34,8,4),(46,14,4),(54,7,4),(65,12,4),(72,8,3))
put(g,6,14,'j');put(g,15,8,'u');wall(g,17,9,8);ropes(g,(28,4,14));put(g,35,6,'l');wall(g,36,3,5,'b');wind(g,48,4,4);put(g,56,6,'v');wall(g,57,7,10);row(g,64,8,6,'+');spikes(g,66,7);wall(g,70,4,8,'h');put(g,73,6,'o')

g=begin('A LIGHT TO COME HOME','A final tour of the repaired network: signal climb, freight launch, wind power and the last delivery home.')
platforms(g,(4,15,3),(10,12,4),(15,8,4),(25,14,4),(32,7,6),(44,13,4),(53,9,5),(65,14,3),(71,10,4))
ropes(g,(7,5,15),(16,4,9));put(g,17,7,'u');wall(g,18,8,9);put(g,26,13,'j');put(g,35,5,'l');wall(g,34,3,4,'b');wind(g,47,4,4);put(g,55,8,'v');wall(g,57,9,8);row(g,64,10,6,'+');wall(g,67,5,12,'h');wall(g,73,3,7,'a');put(g,70,7,'o')

for g in rooms:
 for x in [20,40,60]:
  for xx in range(x-1,x+3):
   for y in range(12,17):g[y][xx]=' '
  g[15][x]='c'
 g[15][77]='E'
for name,g in zip(names,rooms):
 flat=''.join(map(''.join,g))
 for c in 'uvlE':assert flat.count(c)==1,(name,c)
 assert flat.count('c')==3
