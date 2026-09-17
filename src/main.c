#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include "engine.h"
#include "city.h"
#include "story.h"
#include "music.h"
#include "art.h"
State player;
City city;
uint8_t room_id=0,game_mode=0,oldkeys=0,tick=0,selected=0;
uint8_t message_timer=0,save_migrated=0,first_restoration=0,room_area=0;
const char *message;
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
const palette_color_t hero_pal[]={RGB(0,0,0),RGB(4,8,12),RGB(11,31,27),RGB(31,31,28),RGB(0,0,0),RGB(5,7,10),RGB(31,20,8),RGB(31,31,28)};
const palette_color_t scenery[]={
 RGB(3,4,8),RGB(8,12,17),RGB(11,22,24),RGB(5,9,14),
 RGB(5,3,5),RGB(15,7,6),RGB(27,16,9),RGB(9,5,7),
 RGB(2,6,6),RGB(6,14,11),RGB(18,25,12),RGB(3,10,9),
 RGB(5,5,11),RGB(11,12,21),RGB(24,22,30),RGB(8,8,16),
 RGB(2,6,10),RGB(7,14,21),RGB(18,28,30),RGB(4,10,15),
 RGB(6,2,9),RGB(13,7,20),RGB(27,16,29),RGB(9,4,14)
};

const uint8_t scenery_tiles[6]={T_BRICK,T_METAL,T_MOSS,T_RIVET,T_GLASS,T_CORE};
const uint8_t city_icons[6]={T_ROOF,T_GEAR,T_TREE,T_CLOUD,T_DISH,T_HEART};
void text_at(uint8_t x,uint8_t y,const char *s){uint8_t c;while(*s&&x<20){c=*s++;set_bkg_tiles(x++,y,1,1,&c);}}
void win_text(uint8_t x,uint8_t y,const char *s){uint8_t c;while(*s&&x<20){c=*s++;set_win_tiles(x++,y,1,1,&c);}}
void cell(uint8_t x,uint8_t y,uint8_t t,uint8_t a){VBK_REG=1;set_bkg_tiles(x,y,1,1,&a);VBK_REG=0;set_bkg_tiles(x,y,1,1,&t);}
void sound(uint8_t kind){
 NR10_REG=0;NR11_REG=0x80;NR12_REG=0xA1;
 if(kind==EV_JUMP){NR13_REG=0x90;NR14_REG=0x86;}
 else if(kind==EV_DASH){NR10_REG=0x16;NR13_REG=0x20;NR14_REG=0x87;}
 else if(kind==EV_DIE){NR10_REG=0x3c;NR13_REG=0x80;NR14_REG=0x83;}
 else{NR13_REG=0xB0;NR14_REG=0x87;}
}
void save_progress(void){
 volatile uint8_t *ram=(volatile uint8_t *)0xA000;uint8_t i,b[SAVE_SIZE];city.selected=selected;city_encode(&city,b);
 ENABLE_RAM;ram[0]=0;for(i=1;i<SAVE_SIZE;i++)ram[i]=b[i];ram[0]=b[0];DISABLE_RAM;
}
void load_progress(void){volatile uint8_t *ram=(volatile uint8_t *)0xA000;uint8_t i,b[SAVE_SIZE];ENABLE_RAM;for(i=0;i<SAVE_SIZE;i++)b[i]=ram[i];DISABLE_RAM;save_migrated=city_decode(&city,b)==2;selected=city.selected;}
void remember_room(void){uint8_t n=room_id/3;if(city.progress[n]==room_id%3){city.checkpoint[n]=player.checkpoint;city.relays[n]=player.relays;}if(player.letter)city_take_letter(&city,room_id);save_progress();}
uint8_t letter_count(void){uint8_t n,i;for(i=0,n=0;i<ROOMS;i++)n+=city_has_letter(&city,i);return n;}
void clear_screen(void){uint16_t i;DISPLAY_OFF;HIDE_WIN;move_bkg(0,0);for(i=0;i<360;i++){map[i]=0;attrs[i]=0;}VBK_REG=1;set_bkg_tiles(0,0,20,18,attrs);VBK_REG=0;set_bkg_tiles(0,0,20,18,map);HIDE_SPRITES;for(i=0;i<16;i++)move_sprite(i,0,0);}
void hud_start(void) NONBANKED {if(game_mode==1||game_mode==2)SHOW_WIN;}
void hud_end(void) NONBANKED {HIDE_WIN;}
void radio_message(const char *line){message=line;message_timer=150;}
void hud(void){
 char line[21]="1-1 CYAN + LINK0/2  ";
 line[0]='1'+room_id/3;line[2]='1'+room_id%3;
 if(player.phase){line[4]='A';line[5]='M';line[6]='B';line[7]='R';}
 line[9]=player.charge?'+':'-';line[15]='0'+((player.relays&1)!=0)+((player.relays&2)!=0);
 win_text(0,0,line);win_text(0,1,"                    ");
 if(message_timer)win_text(0,1,message);
 else if(room_id==0)win_text(0,1,player.relays==3?"A THEN UP+B: GO HIGH":!(player.relays&1)?"ARROWS MOVE / A JUMP":"B DASH TURNS AMBER");
 else win_text(0,1,names[room_id]);
}
void tile_art(uint8_t c,uint8_t x,uint8_t y,uint8_t *t,uint8_t *a){
 uint8_t area=room_area,on=city.progress[area]==3||player.relays==3;
 *t=0;*a=6;
 switch(c){
 case '#':*t=scenery_tiles[area];*a=1;break;case '^':*t=2;*a=2;break;
 case 'a':*t=3;*a=3;break;case 'b':*t=4;*a=4;break;
 case 'E':*t=5;*a=player.relays==3?5:6;break;
 case 'o':*t=6;*a=5;break;case 'r':*t=10;*a=4;break;
 case 'j':*t=11;*a=5;break;case 'c':*t=12;*a=player.checkpoint>=x?3:5;break;
 case 'w':*t=13;*a=3;break;case 'h':*t=14;*a=7;break;
 case 'u':*t=player.relays&1?T_LINKED:T_SOCKET;*a=3;break;
 case 'v':*t=player.relays&2?T_LINKED:T_SOCKET;*a=4;break;
 case 'l':*t=player.letter?0:T_LETTER;*a=5;break;
 case '=':case '+':*t=player.relays&(c=='='?1:2)?T_BRIDGE_ON:T_BRIDGE_OFF;*a=c=='='?3:4;break;
 default:
  if(area==0){if(y>=12&&(x&3)>1)*t=on?T_WINDOW_LIT:8;else if(((x^(y*3))&31)==0)*t=7;}
  else if(area==1){if(y==5||y==13)*t=T_PIPES;else if((x&7)==0&&y>8)*t=T_CHAIN;}
  else if(area==2){if(((x*3+y)&7)==0&&y>5)*t=T_LEAVES;else if(y>13&&(x&3)==0)*t=on?T_TREE:T_LEAVES;}
  else if(area==3){if(y==6&&(x&7)<3)*t=T_CLOUD;else if((x&7)==0&&y>8)*t=T_CHAIN;}
  else if(area==4){if(((x*7+y*11)&31)==0)*t=T_STARS;else if(y>12&&(x&7)==0)*t=T_DISH;}
  else{if(y==7||y==12)*t=T_CABLE;else if((x&7)==0&&y>7)*t=on?T_WINDOW_LIT:8;}
  if(on)*a=1;break;
 }
}
void draw_column(uint8_t x){uint8_t y;for(y=0;y<18;y++)tile_art(x<ROOM_W?world[(uint16_t)y*ROOM_W+x]:'#',x,y,&column[y],&colors[y]);VBK_REG=1;set_bkg_tiles(x&31,0,1,18,colors);VBK_REG=0;set_bkg_tiles(x&31,0,1,18,column);}
void refresh_objects(void){uint8_t x,y,c,t,a;for(x=camera/8;x<camera/8+21&&x<ROOM_W;x++)for(y=3;y<17;y++){c=world[(uint16_t)y*ROOM_W+x];if(c=='u'||c=='v'||c=='l'||c=='='||c=='+'||c=='E'||c=='c'){tile_art(c,x,y,&t,&a);cell(x&31,y,t,a);}}}
void update_camera(void){int16_t next=player.x/16-72;uint8_t before=camera/8,after;if(next<0)next=0;if(next>ROOM_W*8-160)next=ROOM_W*8-160;camera=next;after=camera/8;while(before<after){before++;draw_column(before+20);}while(before>after){before--;draw_column(before);}move_bkg(camera,0);}
void draw_room(void){
 uint8_t i;clear_screen();set_bkg_palette(1,1,&scenery[(room_id/3)*4]);
 camera=player.x/16>72?player.x/16-72:0;if(camera>ROOM_W*8-160)camera=ROOM_W*8-160;
 for(i=0;i<32;i++)draw_column(camera/8+i);
 VBK_REG=1;set_win_tiles(0,0,20,2,attrs);VBK_REG=0;set_win_tiles(0,0,20,2,map);move_win(7,0);hud();SHOW_WIN;move_bkg(camera,0);
 set_sprite_tile(0,0);set_sprite_tile(1,1);set_sprite_tile(2,2);for(i=0;i<8;i++){set_sprite_tile(3+i,3);set_sprite_prop(3+i,1);}
 set_sprite_tile(12,8);set_sprite_tile(13,9);set_sprite_prop(12,1);set_sprite_prop(13,1);SHOW_SPRITES;DISPLAY_ON;
}
void route_line(uint8_t from,uint8_t to){int8_t x=city_x[from],y=city_y[from],dx=city_x[to]>x?1:-1;uint8_t lit=city.progress[from]==3;while(x!=city_x[to]){x+=dx;if(y<city_y[to])y++;else if(y>city_y[to])y--;cell(x,y,T_CABLE,lit?3:6);}while(y!=city_y[to]){y+=y<city_y[to]?1:-1;cell(x,y,T_CABLE,lit?3:6);}}
void world_map(void){
 uint8_t i,x,y,n=letter_count();char status[21]="0/6 AWAKE MAIL 00/18";char label[4];clear_screen();game_mode=4;
 text_at(3,0,"THE CITY OF LUMEN");status[0]='0'+city_restored(&city);status[15]='0'+n/10;status[16]='0'+n%10;text_at(0,2,status);
 route_line(0,1);route_line(0,2);route_line(1,3);route_line(2,4);route_line(3,5);route_line(4,5);
 for(i=0;i<6;i++){x=city_x[i];y=city_y[i];cell(x,y,city_icons[i],city.progress[i]==3?5:city_unlocked(&city,i)?3:6);cell(x,y+1,city.progress[i]==3?T_WINDOW_LIT:8,city.progress[i]==3?5:6);label[0]='1'+i;label[1]=0;text_at(x,y+2,label);if(selected==i)text_at(x,y-1,"+");}
 text_at(0,14,locations[selected]);label[0]='0'+city.progress[selected];label[1]='/';label[2]='3';label[3]=0;text_at(0,15,"DELIVERIES");text_at(11,15,label);
 if(city_unlocked(&city,selected))text_at(0,17,"A CHOOSE / B RESUME");
 else text_at(0,17,selected==5?"NEEDS LIFTS AND WIND":selected==3?"RESTORE THE FOUNDRY":selected==4?"RESTORE THE GARDENS":"RESTORE THE POSTS");
 DISPLAY_ON;
}
const char * const lessons[6][3]={
 {"MARA IS ON THE RADIO","ONE SPARK LEFT","TAKE IT TO THE POSTS"},
 {"BOTS GUARD THE WORKS","STOMP OR DASH THEM","RECONNECT BOTH LINKS"},
 {"UP / DOWN ON ROPES","JUMP TO LET GO","WATER NEEDS A ROUTE"},
 {"SPRINGS LAUNCH YOU","STEER THEN DASH","GET THE CARGO MOVING"},
 {"ARROWS LIFT YOU UP","DASH OUT TO A LEDGE","GIVE THE CITY A SKY"},
 {"DIM COILS ARE SAFE","BRIGHT COILS HURT","BRING THE LINKS HOME"}
};
void briefing(void){uint8_t i,a;char label[4];clear_screen();game_mode=5;text_at(0,1,locations[selected]);text_at(0,3,"LEFT / RIGHT: ROOM");for(i=0;i<3;i++){a=i<=city.progress[selected]?3:6;cell(5+i*4,5,city_has_letter(&city,selected*3+i)?T_LETTER:6,a);label[0]='1'+i;label[1]=0;text_at(5+i*4,6,label);if(i==room_id%3)text_at(5+i*4,4,"+");}text_at(0,8,names[room_id]);for(i=0;i<3;i++)text_at(0,10+i*2,lessons[selected][i]);if(city_has_letter(&city,room_id))text_at(0,16,"SELECT READ LETTER");text_at(0,17,"A GO / B MAP");DISPLAY_ON;}
void letter_screen(void){uint8_t i;clear_screen();game_mode=9;cell(9,2,T_LETTER,5);text_at(4,4,"LOST AND FOUND");for(i=0;i<3;i++)text_at(0,7+i*2,letters[room_id][i]);text_at(0,17,"A / B BACK");DISPLAY_ON;}
void enter_room(uint8_t resume){
 room_area=room_id/3;load_room(room_id);init_state(&player,room_id);message_timer=0;
 if(resume&&city.progress[room_id/3]==room_id%3){player.checkpoint=city.checkpoint[room_id/3];player.relays=city.relays[room_id/3];respawn(&player,room_id);}
 player.letter=city_has_letter(&city,room_id);draw_room();game_mode=1;save_progress();
}
void delivery(void){uint8_t i;clear_screen();game_mode=6;text_at(4,1,"DELIVERY MADE");cell(9,4,T_PORTRAIT,room_id/3%2?4:3);text_at(0,6,people[room_id/3]);for(i=0;i<3;i++)text_at(0,9+i*2,deliveries[room_id][i]);text_at(0,16,player.letter?"LOST LETTER RETURNED":"A LETTER WAITS HERE");text_at(0,17,"A CONTINUE");DISPLAY_ON;sound(EV_EXIT);}
void celebration(void){uint8_t i;clear_screen();game_mode=7;text_at(2,1,"A DISTRICT AWAKES");text_at(0,3,locations[selected]);for(i=1;i<19;i++){cell(i,8,scenery_tiles[selected],1);cell(i,7,T_WINDOW_LIT,5);if(i%3==0)cell(i,6,city_icons[selected],5);}for(i=0;i<3;i++)text_at(0,11+i*2,restoration[selected][i]);text_at(0,17,"A RETURN TO THE CITY");for(i=0;i<6;i++){set_sprite_tile(i,2);set_sprite_prop(i,i&1);}SHOW_SPRITES;DISPLAY_ON;}
void homecoming(void){uint8_t i,n=letter_count();char count[21]="MAIL DELIVERED 00/18";clear_screen();game_mode=10;text_at(4,1,"HOME AT LAST");for(i=0;i<6;i++)cell(2+i*3,5,city_icons[i],5);text_at(0,8,"SIX LIGHTS. ONE CITY");text_at(0,10,"YOU BROUGHT US HOME.");count[15]='0'+n/10;count[16]='0'+n%10;text_at(0,13,count);text_at(0,15,n==18?"EVERY LETTER FOUND!":"LETTERS STILL WAIT");text_at(0,17,"A EXPLORE AGAIN");DISPLAY_ON;}
void title(void){clear_screen();text_at(4,3,"P O L A R I T Y");text_at(4,5,"STORM COURIER");text_at(1,8,"THE CITY WENT DARK");text_at(2,10,"YOU HAVE ONE SPARK");text_at(2,13,"PRESS A OR START");text_at(0,16,save_migrated?"OLD ROUTES REMEMBERED":"BRING THE LIGHT HOME");DISPLAY_ON;}
void main(void){
 uint8_t keys,pressed,input,ev,i,flash=0,old_relays,old_letter;int16_t sx;
 DISPLAY_OFF;cpu_fast();SPRITES_8x8;LCDC_REG|=LCDCF_WIN9C00;
 add_VBL(hud_start);add_LCD(hud_end);LYC_REG=16;STAT_REG=STATF_LYC;set_interrupts(VBL_IFLAG|LCD_IFLAG);
 set_bkg_data(0,TILE_COUNT,tiles);set_sprite_data(0,SPRITE_COUNT,sprites);set_bkg_palette(0,8,palettes);set_sprite_palette(0,2,hero_pal);
 NR52_REG=0x80;NR50_REG=0x77;NR51_REG=0xFF;music_init();load_progress();SHOW_BKG;title();
 while(1){
  wait_vbl_done();tick++;keys=joypad();pressed=keys&~oldkeys;oldkeys=keys;music_update(game_mode==1?room_id/3:selected,city_restored(&city));
  if(game_mode==0){if(pressed&(J_A|J_START))world_map();continue;}
  if((pressed&J_SELECT)&&(keys&J_UP)){if(game_mode==1||game_mode==2){selected=room_id/3;remember_room();}world_map();continue;}
  if(game_mode==4){
   input=pressed&J_LEFT?LEFT:pressed&J_RIGHT?RIGHT:pressed&J_UP?UP:pressed&J_DOWN?DOWN:0;
   if(input){selected=city_neighbor(selected,input);world_map();}
   if((pressed&(J_A|J_START))&&city_unlocked(&city,selected)){room_id=selected*3+(city.progress[selected]<3?city.progress[selected]:0);briefing();}
   if((pressed&J_B)&&city_unlocked(&city,selected)){room_id=selected*3+(city.progress[selected]<3?city.progress[selected]:0);enter_room(1);}continue;
  }
  if(game_mode==5){
   if((pressed&J_LEFT)&&room_id%3){room_id--;briefing();}if((pressed&J_RIGHT)&&room_id%3<2&&room_id%3<city.progress[selected]){room_id++;briefing();}
   if(pressed&(J_A|J_START))enter_room(1);else if(pressed&J_B)world_map();else if((pressed&J_SELECT)&&city_has_letter(&city,room_id))letter_screen();continue;
  }
  if(game_mode==9){if(pressed&(J_A|J_B|J_START))briefing();continue;}
  if(game_mode==6){if(pressed&(J_A|J_START)){if(room_id%3==2){if(first_restoration)celebration();else world_map();}else{room_id++;enter_room(0);}}continue;}
  if(game_mode==7){for(i=0;i<6;i++)move_sprite(i,24+((tick+i*23)%128),45+((tick/2+i*7)%27));if(pressed&(J_A|J_START)){if(city_restored(&city)==6)homecoming();else world_map();}continue;}
  if(game_mode==10){if(pressed&(J_A|J_START))world_map();continue;}
  if(game_mode==2){if(pressed&(J_A|J_START)){game_mode=1;draw_room();}else if(pressed&J_B){remember_room();world_map();}continue;}
  if(pressed&J_START){game_mode=2;win_text(0,1,"A RESUME / B MAP    ");continue;}
  if(pressed&J_SELECT){deaths++;respawn(&player,room_id);draw_room();}
  input=0;if(keys&J_LEFT)input|=LEFT;if(keys&J_RIGHT)input|=RIGHT;if(keys&J_UP)input|=UP;if(keys&J_DOWN)input|=DOWN;if(keys&J_A)input|=JUMP;if(keys&J_B)input|=DASH;
  old_relays=player.relays;old_letter=player.letter;ev=step(&player,room_id,input);
  if(player.relays!=old_relays){radio_message(radio_lines[room_id][(player.relays^old_relays)&1?0:1]);remember_room();refresh_objects();sound(EV_RELAY);}
  if(player.letter!=old_letter){radio_message("LOST LETTER FOUND!");remember_room();refresh_objects();sound(EV_REFILL);}
  if(ev&EV_DIE){deaths++;sound(EV_DIE);respawn(&player,room_id);draw_room();flash=8;}
  else if(ev&EV_EXIT){selected=room_id/3;first_restoration=city.progress[selected]<3;if(city.progress[selected]<room_id%3+1){city.progress[selected]=room_id%3+1;city.checkpoint[selected]=0;city.relays[selected]=0;}save_progress();delivery();continue;}
  else if(ev&EV_CHECKPOINT){remember_room();refresh_objects();if(!message_timer)radio_message("CHECKPOINT SAVED");sound(EV_REFILL);}
  else if(ev&(EV_JUMP|EV_DASH|EV_REFILL|EV_SPRING))sound((ev&EV_DASH)?EV_DASH:((ev&EV_JUMP)?EV_JUMP:EV_REFILL));
  if(message_timer)message_timer--;
  if(!(tick&15)){i=tile(room_id,player.x/16+3,player.y/16+5);if((i=='u'&&player.phase)|| (i=='v'&&!player.phase))radio_message(room_id==0?(i=='u'?"B DASH TURNS CYAN":"B DASH TURNS AMBER"):(i=='u'?"SIGNAL NEEDS CYAN":"POWER NEEDS AMBER"));if(i=='E'&&player.relays!=3)radio_message("CONNECT BOTH SOCKETS");hud();}
  update_camera();
  if(player.clock&64){set_bkg_palette_entry(7,2,RGB(31,12,13));set_bkg_palette_entry(7,3,RGB(31,20,10));}else{set_bkg_palette_entry(7,2,RGB(8,6,9));set_bkg_palette_entry(7,3,RGB(10,8,12));}
  set_sprite_prop(0,(player.face?0:S_FLIPX)|player.phase);set_sprite_prop(1,(player.face?0:S_FLIPX)|player.phase);
  i=player.ground?(player.vx?4+((player.clock>>3)&1):1):6;if(tile(room_id,player.x/16+3,player.y/16+5)=='r'&&(input&(UP|DOWN)))i=7;set_sprite_tile(1,i);
  sx=player.x/16-camera;if(flash)flash--;
  if(flash&1){move_sprite(0,0,0);move_sprite(1,0,0);}else{move_sprite(0,sx+7,player.y/16+16);move_sprite(1,sx+7,player.y/16+24);}
  if(player.dash)move_sprite(2,sx+7-player.vx/8,player.y/16+19-player.vy/8);else move_sprite(2,0,0);
  for(i=0;i<enemy_count;i++){sx=enemy_x(i,player.clock)-camera;if(sx> -8&&sx<160&&!(player.defeated&(1u<<i)))move_sprite(3+i,sx+8,enemies[i].y+16);else move_sprite(3+i,0,0);}
  sx=600-camera;if(sx>=0&&sx<156){move_sprite(12,sx+8,137);move_sprite(13,sx+8,145);}else{move_sprite(12,0,0);move_sprite(13,0,0);}
 }
}
