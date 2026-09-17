#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include "engine.h"
#include "art.h"
State player;
uint8_t room_id=0,game_mode=0,oldkeys=0,tick=0;
uint16_t deaths=0;
uint8_t map[360],attrs[360];
const palette_color_t palettes[]={
 RGB(3,4,8),RGB(5,8,12),RGB(24,29,26),RGB(11,15,20),
 RGB(3,4,8),RGB(8,12,17),RGB(11,22,24),RGB(5,9,14),
 RGB(3,4,8),RGB(12,5,9),RGB(31,12,13),RGB(22,7,12),
 RGB(3,4,8),RGB(5,11,13),RGB(11,31,27),RGB(8,20,21),
 RGB(3,4,8),RGB(13,8,4),RGB(31,24,10),RGB(29,15,7),
 RGB(3,4,8),RGB(13,19,6),RGB(26,31,15),RGB(31,31,24),
 RGB(3,4,8),RGB(5,7,11),RGB(8,12,17),RGB(11,17,20),
 RGB(3,4,8),RGB(8,12,17),RGB(31,31,29),RGB(11,31,27)
};
const palette_color_t hero_pal[]={RGB(0,0,0),RGB(4,8,12),RGB(26,31,15),RGB(31,31,28),RGB(0,0,0),RGB(5,7,10),RGB(31,20,8),RGB(31,31,28)};
void text_at(uint8_t x,uint8_t y,const char *s){uint8_t c;while(*s&&x<20){c=*s++;set_bkg_tiles(x++,y,1,1,&c);}}
void sound(uint8_t kind){
 NR10_REG=0;NR11_REG=0x80;NR12_REG=0xA1;
 if(kind==EV_JUMP){NR13_REG=0x90;NR14_REG=0x86;}
 else if(kind==EV_DASH){NR10_REG=0x16;NR13_REG=0x20;NR14_REG=0x87;}
 else if(kind==EV_DIE){NR10_REG=0x3c;NR13_REG=0x80;NR14_REG=0x83;}
 else{NR13_REG=0xB0;NR14_REG=0x87;}
}
void clear_screen(void){uint16_t i;DISPLAY_OFF;for(i=0;i<360;i++){map[i]=0;attrs[i]=0;}VBK_REG=1;set_bkg_tiles(0,0,20,18,attrs);VBK_REG=0;set_bkg_tiles(0,0,20,18,map);HIDE_SPRITES;}
void hud(void){
 uint8_t n=room_id+'1';char d[4];
 text_at(1,0,"P");set_bkg_tiles(3,0,1,1,&n);text_at(4,0,"/8");
 text_at(7,0,player.phase?"AMBER":" CYAN");
 text_at(14,0,player.charge?"DASH +":"DASH -");
 d[0]='0'+(deaths/100)%10;d[1]='0'+(deaths/10)%10;d[2]='0'+deaths%10;d[3]=0;
 text_at(1,1,"RETRY");text_at(7,1,d);
}
void draw_room(void){
 uint16_t i;uint8_t t,a,x,y;clear_screen();
 for(i=40;i<360;i++){
  x=i%20;y=i/20;a=6;t=0;
  switch(levels[room_id][i]){
   case '#':t=1;a=1;break;
   case '^':t=2;a=2;break;
   case 'a':t=3;a=3;break;
   case 'b':t=4;a=4;break;
   case 'E':t=5;a=5;break;
   case 'o':t=6;a=5;break;
   default:if(y>=12&&((x*7)%5)>1)t=8;else if((x*13+y*7)%29==0)t=7;break;
  }
  map[i]=t;attrs[i]=a;
 }
 VBK_REG=1;set_bkg_tiles(0,0,20,18,attrs);VBK_REG=0;set_bkg_tiles(0,0,20,18,map);
 hud();set_sprite_tile(0,0);set_sprite_tile(1,1);set_sprite_tile(2,2);SHOW_SPRITES;DISPLAY_ON;
}
void title(void){clear_screen();
 text_at(4,3,"P O L A R I T Y");text_at(4,5,"STORM COURIER");
 text_at(2,8,"A CITY GONE DARK");text_at(2,10,"ONE LAST SPARK.");
 text_at(3,13,"PRESS A OR START");text_at(2,16,"8 ROOMS / RETRY +");DISPLAY_ON;
}
void finish(void){clear_screen();text_at(3,3,"SIGNAL RESTORED");text_at(2,6,"THE CITY IS AWAKE.");text_at(2,8,"YOU WERE THE SPARK.");text_at(3,12,"ALL 8 ROOMS CLEAR");text_at(3,15,"START TO RUN AGAIN");DISPLAY_ON;game_mode=3;sound(EV_EXIT);}
void main(void){
 uint8_t keys,pressed,input,ev,flash=0;
 DISPLAY_OFF;cpu_fast();SPRITES_8x8;
 set_bkg_data(0,TILE_COUNT,tiles);set_sprite_data(0,3,sprites);
 set_bkg_palette(0,8,palettes);set_sprite_palette(0,2,hero_pal);
 NR52_REG=0x80;NR50_REG=0x77;NR51_REG=0xFF;
 SHOW_BKG;title();
 while(1){
  wait_vbl_done();tick++;keys=joypad();pressed=keys&~oldkeys;oldkeys=keys;
  if(game_mode==0||game_mode==3){if(pressed&(J_A|J_START)){room_id=0;deaths=0;init_state(&player,0);draw_room();game_mode=1;}continue;}
  if(pressed&J_START){if(game_mode==1){game_mode=2;text_at(7,1,"PAUSED");}else{game_mode=1;draw_room();}continue;}
  if(game_mode==2)continue;
  if(pressed&J_SELECT){deaths++;init_state(&player,room_id);draw_room();}
  input=0;if(keys&J_LEFT)input|=LEFT;if(keys&J_RIGHT)input|=RIGHT;if(keys&J_UP)input|=UP;if(keys&J_DOWN)input|=DOWN;if(keys&J_A)input|=JUMP;if(keys&J_B)input|=DASH;
  ev=step(&player,room_id,input);
  if(ev&EV_DIE){deaths++;sound(EV_DIE);init_state(&player,room_id);flash=8;hud();}
  else if(ev&EV_EXIT){room_id++;if(room_id==ROOMS){finish();continue;}init_state(&player,room_id);draw_room();sound(EV_EXIT);}
  else if(ev&(EV_JUMP|EV_DASH|EV_REFILL)){sound((ev&EV_DASH)?EV_DASH:((ev&EV_JUMP)?EV_JUMP:EV_REFILL));hud();}
  if(!(tick&15))hud();
  set_sprite_prop(0,(player.face?0:S_FLIPX)|player.phase);set_sprite_prop(1,(player.face?0:S_FLIPX)|player.phase);
  if(flash){flash--;if(flash&1){move_sprite(0,0,0);move_sprite(1,0,0);continue;}}
  move_sprite(0,player.x/16+7,player.y/16+16);move_sprite(1,player.x/16+7,player.y/16+24);
  if(player.dash){move_sprite(2,player.x/16+7-(player.vx/8),player.y/16+19-(player.vy/8));}else move_sprite(2,0,0);
 }
}
