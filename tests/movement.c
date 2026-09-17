#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "../src/engine.h"
static uint32_t h=2166136261u;
static void hash(int v){h=(h^(uint32_t)v)*16777619u;}
int main(void){
 State s;
 for(int scenario=0;scenario<6;scenario++){
  memset(world,' ',sizeof(world));
  for(int x=0;x<ROOM_W;x++){world[2*ROOM_W+x]='#';world[17*ROOM_W+x]='#';}
  for(int y=3;y<17;y++){world[y*ROOM_W]='#';world[y*ROOM_W+18]='#';}
  enemy_count=0;
  if(scenario==3)world[16*ROOM_W+8]='j';
  if(scenario==4)for(int y=4;y<17;y++)world[y*ROOM_W+8]='r';
  if(scenario==5)for(int y=4;y<17;y++)for(int x=7;x<10;x++)world[y*ROOM_W+x]='w';
  init_state(&s,0);if(scenario>=3)s.x=64*16;
  for(int f=0;f<240;f++){
   uint8_t k=f%100<65?RIGHT:LEFT;
   if(f%43<12)k|=JUMP;
   if(f%51==20)k|=DASH;
   if((scenario==1&&f%51==20)||scenario==4)k|=UP;
   if(scenario==2&&f%51==20)k=UP|DASH;
   uint8_t e=step(&s,0,k);
   hash(e);hash(s.x);hash(s.y);hash(s.vx);hash(s.vy);hash(s.phase);hash(s.charge);hash(s.ground);hash(s.coyote);hash(s.buffer);hash(s.dash);hash(s.wall);hash(s.lock);hash(s.boost);
   if(e&EV_DIE)respawn(&s,0);
  }
 }
 printf("Movement fingerprint: %08x\n",h);
assert(h==0xee887670u);
}
