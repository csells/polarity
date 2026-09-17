#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/engine.h"
#include "../src/city.h"

int main(void){
 assert(ROOM_W==92);
 for(unsigned room=0;room<ROOMS;room++){
  load_room(room);
  unsigned signal=0,power=0,letters=0,exits=0,flags=0;
  for(unsigned y=0;y<ROOM_H;y++)for(unsigned x=0;x<ROOM_W;x++){
   unsigned c=world[y*ROOM_W+x];
   signal+=c=='u';power+=c=='v';letters+=c=='l';exits+=c=='E';
   if(c=='c'){
    assert(y==15&&(x==20||x==44||x==68));flags++;
    State s;init_state(&s,room);s.checkpoint=x;s.relays=3;s.letter=1;respawn(&s,room);
    for(unsigned f=0;f<120;f++)assert(!(step(&s,room,0)&EV_DIE));
   }
   if(room==0)assert(c!='^'&&c!='a'&&c!='b'&&c!='h'&&c!='m');
  }
  assert(signal==1&&power==1&&letters==1&&exits==1&&flags==3);
 }
 for(unsigned checkpoint=0;checkpoint<3;checkpoint++){
  City before={0},after={0};uint8_t bytes[SAVE_SIZE];
  before.checkpoint[0]=(unsigned[]){20,44,68}[checkpoint];before.relays[0]=3;
  city_encode(&before,bytes);assert(city_decode(&after,bytes)==1);
  assert(!memcmp(&before,&after,sizeof(City)));
 }
 puts("PASS: all 18 Advance layouts, safe checkpoint respawns, hazard-free tutorial, and all three checkpoint save positions.");
}
