#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "../src/city.h"
int main(void){
 uint8_t b[32]={0};City c={0},d={0};
 assert(city_unlocked(&c,0)&&!city_unlocked(&c,1));
 c.progress[0]=3;assert(city_unlocked(&c,1)&&city_unlocked(&c,2));assert(!city_unlocked(&c,5));
 c.progress[1]=c.progress[2]=c.progress[3]=c.progress[4]=3;assert(city_unlocked(&c,5));
 assert(city_neighbor(0,DOWN)==2);assert(city_neighbor(1,RIGHT)==3);assert(city_neighbor(3,DOWN)==4);assert(city_neighbor(5,UP)==3);
 c.selected=2;c.checkpoint[2]=40;c.relays[2]=1;city_take_letter(&c,17);city_encode(&c,b);assert(city_decode(&d,b)==1);assert(!memcmp(&c,&d,sizeof c));assert(city_has_letter(&d,17)&&!city_has_letter(&d,16));
 b[10]^=1;assert(!city_decode(&d,b));
 memset(b,0,sizeof b);b[0]=0x50;b[1]=2;b[2]=1;b[3]=3;b[4]=1;b[10]=40;b[15]=37+1+3+1+40;
 assert(city_decode(&d,b)==2);assert(d.progress[0]==3&&d.progress[1]==1&&d.selected==1);assert(d.checkpoint[1]==0&&d.relays[1]==0);
 puts("City topology, unlocks, letters, save round trip, corruption rejection and legacy migration passed.");
}
