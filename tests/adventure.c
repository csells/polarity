#include <assert.h>
#include <stdio.h>
#include "../src/engine.h"
static void at(State *s,int x,int y){s->x=x*16;s->y=y*16;s->vx=s->vy=0;s->prev=0;}
int main(void){
 State s;uint8_t ev;
 load_room(0);init_state(&s,0);at(&s,160,120);assert(step(&s,0,0)&EV_CHECKPOINT);assert(s.checkpoint==20);respawn(&s,0);assert(s.x==160*16&&s.charge);
 at(&s,320,120);assert(step(&s,0,0)&EV_CHECKPOINT);respawn(&s,0);assert(s.x==320*16);
 at(&s,480,120);assert(step(&s,0,0)&EV_CHECKPOINT);respawn(&s,0);assert(s.x==480*16);
 load_room(6);init_state(&s,6);at(&s,64,90);step(&s,6,UP);assert(s.vy<0&&s.charge);step(&s,6,JUMP|RIGHT);assert(s.vy< -40);
 load_room(9);init_state(&s,9);at(&s,64,120);ev=step(&s,9,0);assert(ev&EV_SPRING);assert(s.vy== -88&&s.boost);
 load_room(12);init_state(&s,12);at(&s,56,88);step(&s,12,0);assert(s.vy<0&&s.charge);
 load_room(15);init_state(&s,15);at(&s,80,100);assert(!(step(&s,15,0)&EV_DIE));s.clock=63;at(&s,80,100);assert(step(&s,15,0)&EV_DIE);
 load_room(3);init_state(&s,3);at(&s,enemy_x(0,1),115);assert(step(&s,3,0)&EV_DIE);
 init_state(&s,3);at(&s,enemy_x(0,1)-3,115);ev=step(&s,3,RIGHT|DASH);assert(!(ev&EV_DIE));assert(s.defeated&1);
 puts("Checkpoints, rope climbing/jumping, springs, updrafts, pulsed coils and dash attacks passed.");
}
