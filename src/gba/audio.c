#include "hardware.h"
extern const signed char *const music_tracks[6];
static unsigned region=99,block=0;
static void stream(void){
 REG16(0x102)=0;REG16(0x106)=0;REG32(0xc4)=0;
 REG16(0x82)=0x0b0e; /* direct sound A stereo, FIFO reset, PSG full */
 REG32(0xbc)=(uint32_t)music_tracks[region];REG32(0xc0)=0x040000a0;
 REG32(0xc4)=0xb6400000; /* DMA1: FIFO special timing, word, repeat, fixed destination */
 REG16(0x100)=65536-1024;REG16(0x104)=0;REG16(0x106)=0xc4;REG16(0x102)=0x80;block=0;
}
void audio_init(void){REG16(0x84)=0x80;REG16(0x80)=0x2277;REG16(0x88)=0x200;}
void audio_region(unsigned n){if(n==region)return;region=n;stream();}
void audio_irq(void){if(++block==4)stream();}
void audio_fx(unsigned event,int x){
 unsigned freq=event==2?1600:event==1?1350:event==4?550:1850;
 REG16(0x80)=0x77|(x<160?0x2000:0)|(x>80?0x0200:0);
 REG16(0x68)=event==4?0x8240:0x9140;REG16(0x6c)=0x8000|freq;
}
