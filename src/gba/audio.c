#include "hardware.h"
#include "engine.h"
extern const signed char *const music_left[6],*const music_right[6];
static unsigned region=99,block=0,chime,chime_tick;
static void stream(void){
 REG16(0x102)=0;REG16(0x106)=0;REG32(0xc4)=0;REG32(0xd0)=0;
 REG16(0x82)=0x9a0e; /* A left, B right, timer 0, reset both FIFOs */
 REG32(0xbc)=(uint32_t)music_left[region];REG32(0xc0)=0x040000a0;
 REG32(0xc8)=(uint32_t)music_right[region];REG32(0xcc)=0x040000a4;
 REG32(0xc4)=0xb6400000;REG32(0xd0)=0xb6400000;
 REG16(0x100)=65536-1024;REG16(0x104)=0;REG16(0x106)=0xc4;REG16(0x102)=0x80;block=0;
}
void audio_init(void){REG16(0x84)=0x80;REG16(0x80)=0x3377;REG16(0x88)=0x200;}
void audio_region(unsigned n){if(n==region)return;region=n;stream();}
void audio_irq(void){if(++block==8)stream();}
static void tone(unsigned freq,unsigned envelope){REG16(0x68)=envelope;REG16(0x6c)=0x8000|freq;}
void audio_tick(void){
 if(!chime)return;
 if(chime_tick%7==0){
  static const unsigned notes[5]={1547,1670,1750,1796,1859};
  tone(notes[(chime_tick/7)%5],0x7180);
 }
 if(++chime_tick>=chime){chime=0;chime_tick=0;}
}
void audio_fx(unsigned event,int x){
 REG16(0x80)=0x0077|(x<175?0xb000:0)|(x>65?0x0b00:0);
 if(event==EV_DASH){
  REG16(0x60)=0x0023;REG16(0x62)=0x8240;REG16(0x64)=0x8000|1450;
  REG16(0x78)=0x5140;REG16(0x7c)=0xc020;
 }else if(event==EV_JUMP){
  REG16(0x60)=0x0013;REG16(0x62)=0x6140;REG16(0x64)=0x8000|1250;
 }else if(event==EV_DIE){
  REG16(0x60)=0x003b;REG16(0x62)=0x8240;REG16(0x64)=0x8000|950;
  REG16(0x78)=0x6240;REG16(0x7c)=0xc035;chime=0;
 }else{
  chime=event==EV_EXIT?35:event==EV_RELAY?28:14;chime_tick=0;
  tone(1547,0x7180);
 }
}
