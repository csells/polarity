#include "engine.h"
uint8_t world[ROOM_W*ROOM_H],enemy_count;
Enemy enemies[8];
void load_room(uint8_t room){
 const uint8_t *p=levels[room];uint16_t i=0;uint8_t n,t;
 enemy_count=0;
 while((n=*p++)){t=*p++;while(n--){world[i]=t;if(t=='m'){if(enemy_count<8){enemies[enemy_count].x=(i%ROOM_W)*8;enemies[enemy_count++].y=(i/ROOM_W)*8+1;}world[i]=' ';}i++;}}
}
int16_t enemy_x(uint8_t n,uint8_t clock){uint8_t t=(clock+n*31)&63;return enemies[n].x+(t<32?t:63-t)/2-8;}
uint8_t tile(uint8_t room,int16_t x,int16_t y){
 (void)room;if(x<0||x>=ROOM_W*8||y<24)return '#';if(y>=144)return '^';
 return world[(y/8)*ROOM_W+x/8];
}
static uint8_t solid(uint8_t room,int16_t x,int16_t y){return tile(room,x,y)=='#';}
static uint8_t blocked(uint8_t room,int16_t x,int16_t y){
 return solid(room,x,y)||solid(room,x+5,y)||solid(room,x,y+10)||solid(room,x+5,y+10)||solid(room,x,y+5)||solid(room,x+5,y+5);
}
void init_state(State *s,uint8_t room){
 (void)room;
 s->x=16*16;s->y=125*16;s->vx=0;s->vy=0;s->phase=0;s->charge=1;s->ground=0;s->coyote=0;s->buffer=0;s->dash=0;s->prev=JUMP|DASH;s->face=1;s->wall=0;s->lock=0;s->defeated=0;s->clock=0;s->checkpoint=0;s->boost=0;
}
void respawn(State *s,uint8_t room){uint8_t cp=s->checkpoint;init_state(s,room);s->checkpoint=cp;if(cp)s->x=cp*128;}
uint8_t step(State *s,uint8_t room,uint8_t keys){
 uint8_t ev=0,press=keys&~s->prev,i,t; int16_t px,py,nx,ny,delta; int8_t dir;
 s->clock++;s->prev=keys; px=s->x/16;py=s->y/16;
 if(press&JUMP)s->buffer=7;else if(s->buffer)s->buffer--;
 if(s->ground){s->coyote=6;s->charge=1;}else if(s->coyote)s->coyote--;
 s->wall=blocked(room,px-1,py)?1:(blocked(room,px+1,py)?2:0);
 if(s->buffer&&(s->coyote||s->wall||tile(room,px+3,py+5)=='r')){
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
  if(s->boost)s->boost--;
  if(!s->boost&&!(keys&JUMP)&&s->vy< -24)s->vy=-24;
  s->vy+=3;if(s->vy>56)s->vy=56;
  if(s->wall&&s->vy>14&&(keys&(LEFT|RIGHT)))s->vy=14;
  t=tile(room,px+3,py+5);
  if(t=='r'&&(keys&(UP|DOWN))&&!(keys&JUMP)){s->vy=(keys&UP)?-24:24;s->charge=1;}
  if(t=='w'){s->vy-=7;if(s->vy< -40)s->vy=-40;s->charge=1;}

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
  if(t=='h'&&(s->clock&64))return ev|EV_DIE;
  if(t=='E')return ev|EV_EXIT;
  if(t=='c'&&xx/8>s->checkpoint){s->checkpoint=xx/8;ev|=EV_CHECKPOINT;}
  if(t=='j'&&s->vy>=0){s->vy=-88;s->boost=24;s->charge=1;ev|=EV_SPRING;}

  if(t=='o'){
   /* Refills are repeatable, but touching one only restores a spent dash. */
   if(!s->charge){s->charge=1;ev|=EV_REFILL;}
  }
 }
 for(i=0;i<enemy_count;i++){
  if(s->defeated&(1u<<i))continue;
  nx=enemy_x(i,s->clock);ny=enemies[i].y;
  if(px+5>=nx&&px<nx+8&&py+10>=ny&&py<ny+8){
   if(s->dash||(s->vy>0&&py+7<ny)){s->defeated|=1u<<i;s->charge=1;s->vy=-40;ev|=EV_REFILL;}
   else return ev|EV_DIE;
  }
 }
 return ev;
}
