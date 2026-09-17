#include <gb/gb.h>
#include "music.h"
/* An original sixteen-note courier theme. The city adds bass, then percussion. */
static uint8_t beat=0,frame=0;
static const uint16_t notes[12]={1547,1602,1651,1673,1715,1751,1783,1798,1825,1850,1872,1881};
static const uint8_t tune[6][16]={
 {0,4,7,9,7,4,2,4,0,4,7,11,9,7,4,2},
 {0,0,4,7,4,2,0,4,7,7,9,7,4,2,4,0},
 {4,7,9,11,9,7,4,2,4,7,9,7,4,2,0,2},
 {0,4,7,11,9,7,4,7,2,5,9,11,9,7,4,2},
 {7,9,11,9,7,4,2,4,5,9,11,9,7,5,4,2},
 {0,2,4,7,4,2,0,2,4,7,9,11,9,7,4,0}
};
void music_init(void){uint8_t i;volatile uint8_t *wave=(volatile uint8_t *)0xff30;NR30_REG=0;for(i=0;i<16;i++)wave[i]=i<8?0x33:0xcc;}
void music_update(uint8_t area,uint8_t restored){
 uint16_t f;if(++frame<14)return;frame=0;beat=(beat+1)&31;f=notes[tune[area][beat>>1]];
 /* Soft square lead leaves channel one free for jump, dash and delivery cues. */
 if(!(beat&1)){NR21_REG=0x80;NR22_REG=0x51;NR23_REG=f;NR24_REG=0x80|(f>>8);}
 if(restored&&!(beat&7)){f=notes[(beat&16)?5:0];NR30_REG=0x80;NR31_REG=0;NR32_REG=0x60;NR33_REG=f;NR34_REG=0x80|(f>>8);}
 if(restored>=3&&(beat&3)==2){NR41_REG=0x3f;NR42_REG=0x21;NR43_REG=0x34;NR44_REG=0xc0;}
}
