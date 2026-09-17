#include "engine.h"
const char *names[ROOMS]={"FIRST SPARK","CROSS CURRENT","AIR MAIL","WALL SIGNAL","LIVE WIRE","DOUBLE SWITCH","HIGH VOLTAGE","STORM HEART"};
const uint8_t spawn_y[ROOMS]={116,116,116,116,116,116,116,116};
uint8_t tile(uint8_t room,int16_t x,int16_t y){
 if(x<0||x>=160||y<16) return '#';
 if(y>=144) return '^';
 return levels[room][(y/8)*20+x/8];
}
static uint8_t solid(uint8_t room,int16_t x,int16_t y){return tile(room,x,y)=='#';}
static uint8_t blocked(uint8_t room,int16_t x,int16_t y){
 return solid(room,x,y)||solid(room,x+5,y)||solid(room,x,y+10)||solid(room,x+5,y+10)||solid(room,x,y+5)||solid(room,x+5,y+5);
}
void init_state(State *s,uint8_t room){
 s->x=16*16;s->y=spawn_y[room]*16;s->vx=0;s->vy=0;s->phase=0;s->charge=1;s->ground=0;s->coyote=0;s->buffer=0;s->dash=0;s->prev=0;s->face=1;s->wall=0;s->lock=0;s->gems=0;
}
uint8_t step(State *s,uint8_t room,uint8_t keys){
 uint8_t ev=0,press=keys&~s->prev,i,t; int16_t px,py,nx,ny,delta; int8_t dir;
 s->prev=keys; px=s->x/16;py=s->y/16;
 if(press&JUMP)s->buffer=7;else if(s->buffer)s->buffer--;
 if(s->ground){s->coyote=6;s->charge=1;}else if(s->coyote)s->coyote--;
 s->wall=blocked(room,px-1,py)?1:(blocked(room,px+1,py)?2:0);
 if(s->buffer&&(s->coyote||s->wall)){
  s->vy=-58;s->buffer=0;s->ground=0;s->coyote=0;s->dash=0;ev|=EV_JUMP;
  if(s->wall&&!solid(room,px+2,py+11)){s->vx=s->wall==1?32:-32;s->lock=9;s->charge=1;}
 }
 if((press&DASH)&&s->charge){
  s->phase^=1;s->charge=0;s->dash=12;s->lock=0;ev|=EV_DASH;
  dir=(keys&RIGHT)?1:((keys&LEFT)?-1:0);
  s->vx=dir*64;s->vy=(keys&UP)?-64:((keys&DOWN)?64:0);
  if(!s->vx&&!s->vy)s->vx=s->face?64:-64;
  if(s->vx&&s->vy){s->vx=s->vx>0?45:-45;s->vy=s->vy>0?45:-45;}
 }
 if(s->dash){s->dash--;}
 else {
  if(s->lock)s->lock--;else {
   dir=(keys&RIGHT)?1:((keys&LEFT)?-1:0);
   if(dir){s->vx+=dir*5;if(s->vx>28)s->vx=28;if(s->vx< -28)s->vx=-28;s->face=dir>0;}
   else{if(s->vx>0){s->vx-=6;if(s->vx<0)s->vx=0;}if(s->vx<0){s->vx+=6;if(s->vx>0)s->vx=0;}}
  }
  if(!(keys&JUMP)&&s->vy< -24)s->vy=-24;
  s->vy+=3;if(s->vy>56)s->vy=56;
  if(s->wall&&s->vy>14&&(keys&(LEFT|RIGHT)))s->vy=14;
 }
 /* Move one pixel at a time: dash cannot tunnel through a wall. */
 nx=s->x+s->vx;ny=s->y+s->vy;delta=nx/16-px;
 dir=delta>0?1:-1;
 while(delta){if(blocked(room,px+dir,py)){nx=px*16;s->vx=0;break;}px+=dir;delta-=dir;}
 s->x=nx;delta=ny/16-py;dir=delta>0?1:-1;s->ground=0;
 while(delta){if(blocked(room,px,py+dir)){ny=py*16;s->vy=0;if(dir>0){s->ground=1;s->charge=1;}break;}py+=dir;delta-=dir;}
 s->y=ny;
 if(py>132)return ev|EV_DIE;
 /* Sample interior of the player, giving spikes a small forgiving margin. */
 for(i=0;i<4;i++){
  int16_t xx=px+1+(i&1)*3, yy=py+2+(i>>1)*7;
  t=tile(room,xx,yy);
  if((t=='^'&&(yy&7)>2)||(t=='a'&&s->phase!=0)||(t=='b'&&s->phase!=1))return ev|EV_DIE;
  if(t=='E')return ev|EV_EXIT;
  if(t=='o'){
   /* Refills are repeatable, but touching one only restores a spent dash. */
   if(!s->charge){s->charge=1;ev|=EV_REFILL;}
  }
 }
 return ev;
}
