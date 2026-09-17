#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include "engine.h"
#include "art.h"
State player;
uint8_t room_id=0,game_mode=0,oldkeys=0,tick=0,selected=0;
uint8_t progress[LOCATIONS],saved_checkpoint[LOCATIONS],notice=0;
uint16_t deaths=0,camera=0;
uint8_t map[360],attrs[360],column[18],colors[18];
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
const palette_color_t scenery[]={
 RGB(3,4,8),RGB(8,12,17),RGB(11,22,24),RGB(5,9,14),
 RGB(5,3,5),RGB(15,7,6),RGB(27,16,9),RGB(9,5,7),
 RGB(2,6,6),RGB(6,14,11),RGB(18,25,12),RGB(3,10,9),
 RGB(5,5,11),RGB(11,12,21),RGB(24,22,30),RGB(8,8,16),
 RGB(2,6,10),RGB(7,14,21),RGB(18,28,30),RGB(4,10,15),
 RGB(6,2,9),RGB(13,7,20),RGB(27,16,29),RGB(9,4,14)
};
void text_at(uint8_t x,uint8_t y,const char *s){uint8_t c;while(*s&&x<20){c=*s++;set_bkg_tiles(x++,y,1,1,&c);}}
void win_text(uint8_t x,uint8_t y,const char *s){uint8_t c;while(*s&&x<20){c=*s++;set_win_tiles(x++,y,1,1,&c);}}
void sound(uint8_t kind){
 NR10_REG=0;NR11_REG=0x80;NR12_REG=0xA1;
 if(kind==EV_JUMP){NR13_REG=0x90;NR14_REG=0x86;}
 else if(kind==EV_DASH){NR10_REG=0x16;NR13_REG=0x20;NR14_REG=0x87;}
 else if(kind==EV_DIE){NR10_REG=0x3c;NR13_REG=0x80;NR14_REG=0x83;}
 else{NR13_REG=0xB0;NR14_REG=0x87;}
}

/* Battery-backed save, versioned and checksummed. Each branch remembers its flag. */
void save_progress(void){
 volatile uint8_t *ram=(volatile uint8_t *)0xA000;uint8_t i,sum=37;
 ENABLE_RAM;ram[0]=0;
 for(i=0;i<LOCATIONS;i++){ram[3+i]=progress[i];sum+=progress[i];ram[9+i]=saved_checkpoint[i];sum+=saved_checkpoint[i];}
 ram[1]=2;ram[2]=selected;sum+=selected;ram[15]=sum;ram[0]=0x50;DISABLE_RAM;
}
void load_progress(void){
 volatile uint8_t *ram=(volatile uint8_t *)0xA000;uint8_t i,sum=37,ok=1;
 ENABLE_RAM;if(ram[0]!=0x50||ram[1]!=2||ram[2]>=LOCATIONS)ok=0;
 sum+=ram[2];for(i=0;i<LOCATIONS;i++){sum+=ram[3+i];sum+=ram[9+i];if(ram[3+i]>3||(ram[9+i]!=0&&ram[9+i]!=20&&ram[9+i]!=40&&ram[9+i]!=60))ok=0;}
 if(sum!=ram[15])ok=0;
 if(ok){selected=ram[2];for(i=0;i<LOCATIONS;i++){progress[i]=ram[3+i];saved_checkpoint[i]=ram[9+i];}}
 DISABLE_RAM;
}
uint8_t unlocked(uint8_t n){
 if(n==0)return 1;if(n<3)return progress[0]==3;if(n==3)return progress[1]==3;if(n==4)return progress[2]==3;return progress[3]==3&&progress[4]==3;
}
void clear_screen(void){uint16_t i;DISPLAY_OFF;HIDE_WIN;move_bkg(0,0);for(i=0;i<360;i++){map[i]=0;attrs[i]=0;}VBK_REG=1;set_bkg_tiles(0,0,20,18,attrs);VBK_REG=0;set_bkg_tiles(0,0,20,18,map);HIDE_SPRITES;for(i=0;i<12;i++)move_sprite(i,0,0);}
/* The Game Boy window has no height: hide it after the two HUD scanlines. */
void hud_start(void) NONBANKED {if(game_mode==1||game_mode==2)SHOW_WIN;}
void hud_end(void) NONBANKED {HIDE_WIN;}
void hud(void){
 char line[22]="1-1 CYAN DASH+ F0   ";
 line[0]='1'+room_id/3;line[2]='1'+room_id%3;
 if(player.phase){line[4]='A';line[5]='M';line[6]='B';line[7]='R';}
 line[13]=player.charge?'+':'-';line[16]='0'+player.checkpoint/20;
 win_text(0,0,line);win_text(0,1,names[room_id]);
}
void draw_column(uint8_t x){
 uint8_t y,t,a,c;uint16_t pos=x;
 for(y=0;y<18;y++,pos+=ROOM_W){c=x<ROOM_W?world[pos]:'#';a=6;t=0;
 switch(c){
 case '#':t=1;a=1;break;case '^':t=2;a=2;break;
 case 'a':t=3;a=3;break;case 'b':t=4;a=4;break;
 case 'E':t=5;a=5;break;case 'o':t=6;a=5;break;
 case 'r':t=10;a=4;break;case 'j':t=11;a=5;break;
 case 'c':t=12;a=5;break;case 'w':t=13;a=3;break;
 case 'h':t=14;a=7;break;
 default:if(y>=12&&((x*7)%5)>1)t=8;else if((x*13+y*7)%29==0)t=7;break;
 }column[y]=t;colors[y]=a;}
 VBK_REG=1;set_bkg_tiles(x&31,0,1,18,colors);VBK_REG=0;set_bkg_tiles(x&31,0,1,18,column);
}
void update_camera(void){
 int16_t next=player.x/16-72;uint8_t before=camera/8,after;
 if(next<0)next=0;if(next>ROOM_W*8-160)next=ROOM_W*8-160;
 camera=next;after=camera/8;
 while(before<after){before++;draw_column(before+20);}
 while(before>after){before--;draw_column(before);}
 move_bkg(camera,0);
}
void draw_room(void){
 uint8_t i;clear_screen();set_bkg_palette(1,1,&scenery[(room_id/3)*4]);
 camera=player.x/16>72?player.x/16-72:0;if(camera>ROOM_W*8-160)camera=ROOM_W*8-160;
 for(i=0;i<32;i++)draw_column(camera/8+i);
 VBK_REG=1;set_win_tiles(0,0,20,2,attrs);VBK_REG=0;set_win_tiles(0,0,20,2,map);
 move_win(7,0);hud();SHOW_WIN;move_bkg(camera,0);
 set_sprite_tile(0,0);set_sprite_tile(1,1);set_sprite_tile(2,2);
 for(i=0;i<8;i++){set_sprite_tile(3+i,3);set_sprite_prop(3+i,1);}
 SHOW_SPRITES;DISPLAY_ON;
}
void world_map(void){
 uint8_t i,x,y,t,a;char label[4];clear_screen();game_mode=4;
 text_at(4,0,"COURIER ATLAS");
 text_at(1,2,notice==2?"CITY POWER RESTORED":notice==1?"LOCATION COMPLETE!":"CHOOSE YOUR ROUTE");
 /* Branching routes: 1 -> 2 -> 4 and 1 -> 3 -> 5, then 6. */
 text_at(4,5,"---");text_at(8,5,"---");text_at(3,6,"/");text_at(3,7,"/");text_at(4,8,"---");text_at(8,8,"---");text_at(12,6,"/");text_at(12,7,"/");text_at(13,8,"---");
 for(i=0;i<6;i++){
  x=i==0?2:(i==1||i==2)?7:(i==3||i==4)?11:16;y=(i==2||i==4||i==5)?8:5;
  t=progress[i]==3?5:unlocked(i)?6:14;a=selected==i?5:unlocked(i)?3:2;
  VBK_REG=1;set_bkg_tiles(x,y,1,1,&a);VBK_REG=0;set_bkg_tiles(x,y,1,1,&t);
  label[0]='1'+i;label[1]=0;text_at(x,y+1,label);
  if(selected==i)text_at(x,y-1,"+");
 }
 text_at(1,11,locations[selected]);label[0]='0'+progress[selected];label[1]='/';label[2]='3';label[3]=0;text_at(1,12,"ROOMS");text_at(7,12,label);
 if(unlocked(selected)){
  text_at(1,13,progress[selected]==3?"A REPLAY / B RESUME":"A ENTER / B RESUME");
  text_at(1,15,"D-PAD CHOOSE GROUP");
 }else{text_at(1,13,selected==5?"CLEAR GROUPS 4 AND 5":selected==3?"CLEAR GROUP 2 FIRST":selected==4?"CLEAR GROUP 3 FIRST":"CLEAR GROUP 1 FIRST");text_at(1,15,"D-PAD EXPLORE MAP");}
 text_at(1,17,"PROGRESS AUTO-SAVED");DISPLAY_ON;
}
const char * const lessons[6][4]={
 {"DASH FLIPS COLOR","MATCH THE LIT GATES","FLAGS SAVE PROGRESS","RESTORE THE GRID"},
 {"SENTRIES ON PATROL","JUMP ON OR DASH THEM","LAND TO CHARGE AGAIN","RESTART THE FOUNDRY"},
 {"UP / DOWN ON ROPES","JUMP AWAY TO LET GO","CLIMB ABOVE THORNS","WAKE THE GARDENS"},
 {"GREEN SPRINGS LAUNCH","STEER IN THE AIR","DASH TO STEER","POWER THE CLOUDS"},
 {"ARROWS ARE UPDRAFTS","RIDE INTO THE SKY","DASH TO A LANDING","ALIGN THE WIND ARRAY"},
 {"COILS PULSE ON / OFF","DIM SAFE / RED HURTS","WAIT THEN DASH","DELIVER THE SPARK"}
};
void briefing(void){uint8_t i;clear_screen();game_mode=5;text_at(1,2,locations[selected]);text_at(1,4,names[room_id]);for(i=0;i<4;i++)text_at(0,7+i*2,lessons[selected][i]);text_at(2,17,"A GO / B BACK");DISPLAY_ON;}
void enter_room(uint8_t resume){
 load_room(room_id);init_state(&player,room_id);
 if(resume){player.checkpoint=saved_checkpoint[room_id/3];respawn(&player,room_id);}
 draw_room();game_mode=1;save_progress();
}
void title(void){clear_screen();text_at(4,3,"P O L A R I T Y");text_at(4,5,"STORM COURIER");text_at(2,8,"SIX LOST LOCATIONS");text_at(2,10,"ONE LAST SPARK.");text_at(3,13,"PRESS A OR START");text_at(1,16,"18 ROOMS / 6 GROUPS");DISPLAY_ON;}
void main(void){
 uint8_t keys,pressed,input,ev,i,flash=0;int16_t sx;
 DISPLAY_OFF;cpu_fast();SPRITES_8x8;LCDC_REG|=LCDCF_WIN9C00;
 add_VBL(hud_start);add_LCD(hud_end);LYC_REG=16;STAT_REG=STATF_LYC;set_interrupts(VBL_IFLAG|LCD_IFLAG);set_bkg_data(0,TILE_COUNT,tiles);set_sprite_data(0,4,sprites);
 set_bkg_palette(0,8,palettes);set_sprite_palette(0,2,hero_pal);NR52_REG=0x80;NR50_REG=0x77;NR51_REG=0xFF;
 load_progress();SHOW_BKG;title();
 while(1){
  wait_vbl_done();tick++;keys=joypad();pressed=keys&~oldkeys;oldkeys=keys;
  if(game_mode==0){if(pressed&(J_A|J_START))world_map();continue;}
  if(game_mode==4){
   if(pressed&(J_RIGHT|J_DOWN)){selected=(selected+1)%LOCATIONS;notice=0;world_map();}
   if(pressed&(J_LEFT|J_UP)){selected=selected?selected-1:5;notice=0;world_map();}
   if((pressed&(J_A|J_START))&&unlocked(selected)){room_id=selected*3+(progress[selected]<3?progress[selected]:0);briefing();}
   if((pressed&J_B)&&unlocked(selected)){room_id=selected*3+(progress[selected]<3?progress[selected]:0);enter_room(progress[selected]<3);}
   continue;
  }
  if(game_mode==5){if(pressed&(J_A|J_START))enter_room(progress[selected]<3);else if(pressed&J_B)world_map();continue;}
  if(game_mode==2){if(pressed&(J_A|J_START)){game_mode=1;draw_room();}else if(pressed&J_B){notice=0;world_map();}continue;}
  if(pressed&J_START){game_mode=2;win_text(0,1,"A RESUME / B MAP    ");continue;}
  if((pressed&J_SELECT)&&(keys&J_UP)){selected=room_id/3;notice=0;save_progress();world_map();continue;}
  if(pressed&J_SELECT){deaths++;respawn(&player,room_id);draw_room();}
  input=0;if(keys&J_LEFT)input|=LEFT;if(keys&J_RIGHT)input|=RIGHT;if(keys&J_UP)input|=UP;if(keys&J_DOWN)input|=DOWN;if(keys&J_A)input|=JUMP;if(keys&J_B)input|=DASH;
  ev=step(&player,room_id,input);
  if(ev&EV_DIE){deaths++;sound(EV_DIE);respawn(&player,room_id);draw_room();flash=8;}
  else if(ev&EV_EXIT){
   selected=room_id/3;if(progress[selected]<room_id%3+1)progress[selected]=room_id%3+1;saved_checkpoint[selected]=0;save_progress();sound(EV_EXIT);
   if(room_id%3==2){notice=1;for(i=0;i<6&&progress[i]==3;i++);if(i==6)notice=2;world_map();continue;}
   room_id++;enter_room(0);continue;
  }
  else if(ev&EV_CHECKPOINT){if(progress[room_id/3]==room_id%3){saved_checkpoint[room_id/3]=player.checkpoint;save_progress();}sound(EV_REFILL);hud();}
  else if(ev&(EV_JUMP|EV_DASH|EV_REFILL|EV_SPRING)){sound((ev&EV_DASH)?EV_DASH:((ev&EV_JUMP)?EV_JUMP:EV_REFILL));hud();}
  update_camera();if(!(tick&15))hud();
  /* Coil brightness is synchronized with the collision clock, including after retries. */
  if(player.clock&64){set_bkg_palette_entry(7,2,RGB(31,12,13));set_bkg_palette_entry(7,3,RGB(31,20,10));}else{set_bkg_palette_entry(7,2,RGB(8,6,9));set_bkg_palette_entry(7,3,RGB(10,8,12));}
  set_sprite_prop(0,(player.face?0:S_FLIPX)|player.phase);set_sprite_prop(1,(player.face?0:S_FLIPX)|player.phase);
  sx=player.x/16-camera;
  if(flash){flash--;if(flash&1){move_sprite(0,0,0);move_sprite(1,0,0);}else{move_sprite(0,sx+7,player.y/16+16);move_sprite(1,sx+7,player.y/16+24);}}
  else{move_sprite(0,sx+7,player.y/16+16);move_sprite(1,sx+7,player.y/16+24);}
  if(player.dash)move_sprite(2,sx+7-player.vx/8,player.y/16+19-player.vy/8);else move_sprite(2,0,0);
  for(i=0;i<enemy_count;i++){sx=enemy_x(i,player.clock)-camera;if(sx> -8&&sx<160&&!(player.defeated&(1u<<i)))move_sprite(3+i,sx+8,enemies[i].y+16);else move_sprite(3+i,0,0);}
 }
}
