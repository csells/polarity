#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/engine.h"
static void at(State *s,int x,int y){s->x=x*16;s->y=y*16;s->vx=s->vy=0;s->prev=0;}
static void empty(State *s){memset(world,' ',sizeof world);for(int x=0;x<ROOM_W;x++)world[17*ROOM_W+x]='#';enemy_count=0;init_state(s,0);}
int main(void){
 State s;uint8_t ev;
 empty(&s);world[15*ROOM_W+20]='c';at(&s,160,120);assert(step(&s,0,0)&EV_CHECKPOINT);assert(s.checkpoint==20);respawn(&s,0);assert(s.x==160*16&&s.charge);
 empty(&s);world[11*ROOM_W+8]='r';at(&s,64,88);step(&s,0,UP);assert(s.vy<0&&s.charge);step(&s,0,JUMP|RIGHT);assert(s.vy< -40);
 empty(&s);world[16*ROOM_W+8]='j';at(&s,64,120);ev=step(&s,0,0);assert(ev&EV_SPRING);assert(s.vy== -88&&s.boost);
 empty(&s);world[11*ROOM_W+7]='w';at(&s,56,88);step(&s,0,0);assert(s.vy<0&&s.charge);
 empty(&s);world[12*ROOM_W+10]='h';at(&s,80,96);assert(!(step(&s,0,0)&EV_DIE));s.clock=63;at(&s,80,96);assert(step(&s,0,0)&EV_DIE);
 empty(&s);enemies[0].x=80;enemies[0].y=121;enemy_count=1;at(&s,enemy_x(0,1),115);assert(step(&s,0,0)&EV_DIE);
 init_state(&s,0);at(&s,enemy_x(0,1)-3,115);ev=step(&s,0,RIGHT|DASH);assert(!(ev&EV_DIE));assert(s.defeated&1);
 empty(&s);world[15*ROOM_W+8]='u';at(&s,64,120);s.phase=1;assert(!(step(&s,0,0)&EV_RELAY));assert(s.relays==0);s.phase=0;assert(step(&s,0,0)&EV_RELAY);assert(s.relays==1);
 world[15*ROOM_W+8]='v';s.phase=1;assert(step(&s,0,0)&EV_RELAY);assert(s.relays==3);
 world[15*ROOM_W+8]='l';step(&s,0,0);assert(s.letter);s.checkpoint=20;respawn(&s,0);assert(s.letter&&s.relays==3&&s.x==160*16);
 world[15*ROOM_W+20]='E';assert(step(&s,0,0)&EV_EXIT);s.relays=1;assert(!(step(&s,0,0)&EV_EXIT));
 empty(&s);world[16*ROOM_W+8]='=';at(&s,64,117);s.relays=1;for(int i=0;i<8;i++)step(&s,0,0);assert(s.y/16==117);s.relays=0;for(int i=0;i<12;i++)step(&s,0,0);assert(s.y/16==125);
 load_room(0);for(int i=0;i<ROOM_W*ROOM_H;i++)assert(world[i]!='^'&&world[i]!='h'&&world[i]!='a'&&world[i]!='b');assert(enemy_count==0);
 puts("Mechanics, nonlethal first lesson, charge sockets, persistent relays/letters, powered bridges and locked exit passed.");
}
