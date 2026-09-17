#include "hardware.h"
#include "generated/assets.h"
#include "engine.h"
#include "city.h"
#include "story.h"
#include <string.h>
/* Retained by the linker: emulators and flash carts identify battery SRAM. */
const char save_type[] __attribute__((used))="SRAM_V113";
State player;City city;
Obj objects[128];
volatile unsigned frame_tick;
unsigned game_mode,room_id,selected,camera,camera_y,deaths,simulation_frames,render_overruns;
static unsigned previous_keys,region=99,scene=99,notice,timer,first_restoration,shake,room_time,landing,entry_latch;
static int old_ground;
static uint16_t foreground[1024],overlay[1024],decoration[1024];
static int trail_x[5],trail_y[5];
static const char *radio_text;
static const int map_x[6]={28,82,82,156,156,215},map_y[6]={76,47,105,47,105,76};
static const char *const instructions[6]={"A JUMP   B DASH / FLIP","STOMP OR DASH THROUGH BOTS","UP / DOWN CLIMB   A LET GO","SPRINGS LAUNCH - AIM YOUR DASH","RIDE THE WIND THEN DASH OUT","DIM COILS SAFE - BRIGHT HURTS"};
static void irq(void){unsigned flags=REG16(0x202);if(flags&1)frame_tick++;if(flags&16)audio_irq();*(volatile uint16_t *)0x03007ff8|=flags;REG16(0x202)=flags;}
void copy16(volatile uint16_t *dst,const void *src,unsigned count){REG32(0xd4)=(uint32_t)src;REG32(0xd8)=(uint32_t)dst;REG32(0xdc)=0x80000000|count;}
static HOT void sprite(unsigned i,int x,int y,unsigned tile,unsigned bank,unsigned size,unsigned flip){
 if(x<-(int)(8<<size)||x>239||y<-(int)(8<<size)||y>159){objects[i].a=0x0200;return;}
 objects[i].a=y&255;objects[i].b=(x&511)|(size<<14)|(flip?0x1000:0);objects[i].c=tile|(bank<<12);}
static HOT void hide_sprites(void){for(unsigned i=0;i<128;i++)objects[i].a=0x0200;}
static HOT void text(unsigned x,unsigned y,const char *s,unsigned bank){while(*s&&x<30){overlay[y*32+x++]=(unsigned char)*s++|(bank<<12);}}
static HOT void center(unsigned y,const char *s){unsigned n=0;while(s[n])n++;text(n<30?(30-n)/2:0,y,s,8);}
static HOT void panel(unsigned y,unsigned h){for(unsigned yy=y;yy<y+h;yy++)for(unsigned x=0;x<30;x++)overlay[yy*32+x]=1|8<<12;}
static void number(unsigned x,unsigned y,unsigned n){char s[4];s[0]='0'+n/10%10;s[1]='0'+n%10;s[2]=0;text(x,y,s,8);}
static unsigned mail_count(void){unsigned n=0;for(unsigned i=0;i<18;i++)n+=city_has_letter(&city,i);return n;}
static void save(void){uint8_t bytes[SAVE_SIZE];city.selected=selected;city_encode(&city,bytes);bytes[0]=0x41;bytes[1]=2;volatile uint8_t *s=(volatile uint8_t *)0x0e000000;s[0]=0;for(unsigned i=1;i<SAVE_SIZE;i++)s[i]=bytes[i];s[0]=bytes[0];}
static void remember(void){unsigned n=room_id/3;if(city.progress[n]==room_id%3){city.checkpoint[n]=player.checkpoint;city.relays[n]=player.relays;}if(player.letter)city_take_letter(&city,room_id);save();}
static void load(void){uint8_t bytes[SAVE_SIZE];volatile uint8_t *s=(volatile uint8_t *)0x0e000000;for(unsigned i=0;i<SAVE_SIZE;i++)bytes[i]=s[i];if(bytes[0]==0x41&&bytes[1]==2){bytes[0]=0x50;bytes[1]=3;city_decode(&city,bytes);}selected=city.selected;}
static void scene_art(unsigned n){
 if(scene==n)return;
 REG16(0)=0x80;scene=n;
 copy16((volatile uint16_t *)0x06004000,backgrounds[n],BACKGROUND_BYTES/2);
 copy16(BG_PALETTE,region_palettes[n],256);
 for(unsigned y=0;y<32;y++)for(unsigned x=0;x<32;x++)MAP(28)[y*32+x]=(y%20)*32+x;
}
static void region_art(unsigned n){REG16(0)=0x80;scene_art(n);copy16(VRAM,terrain_data[n],8192);region=n;audio_region(n);}
static void clear_ui(void){memset(overlay,0,sizeof(overlay));memset(foreground,0,sizeof(foreground));memset(decoration,0,sizeof(decoration));hide_sprites();camera=0;camera_y=0;notice=0;}
static void title(void){clear_ui();game_mode=0;region_art(0);
 center(2,"N I G H T S H I F T");
 for(unsigned y=0;y<4;y++)for(unsigned x=0;x<28;x++)overlay[(5+y)*32+x+1]=(400+y*28+x)|8<<12;
 center(10,"S T O R M   C O U R I E R");
 sprite(0,104,88,11*16,0,2,0);
 panel(16,4);center(16,"THE STORM CUT EVERY CONNECTION.");center(18,"PRESS A OR START");
}
static void atlas(void){clear_ui();game_mode=4;scene_art(6);audio_region(selected);region=selected;
 panel(0,2);text(1,0,"LUMEN",8);text(11,0,"AWAKE",8);number(17,0,city_restored(&city));text(21,0,"MAIL",8);number(27,0,mail_count());
 for(unsigned i=0;i<6;i++){
  unsigned bank=city.progress[i]==3?11:city_unlocked(&city,i)?9:13;
  sprite(i,map_x[i]-8,map_y[i]-8,FX_TILE+7*4,bank,1,0);
 }
 sprite(7,map_x[selected]-8,map_y[selected]-22,FX_TILE+4*4,8,1,0);
 panel(16,4);center(16,locations[selected]);
 if(city_unlocked(&city,selected)){text(7,17,"DELIVERIES",8);number(18,17,city.progress[selected]);center(19,"A ROUTES   B CONTINUE");}
 else center(18,"FOLLOW THE CONNECTING RAILWAY");
}
static void briefing(void){clear_ui();game_mode=5;region_art(selected);
 panel(0,3);center(1,locations[selected]);
 sprite(0,10,32,PORTRAIT_TILE+selected*64,2+selected,3,0);
 text(11,5,people[selected],8);text(11,7,"DELIVERY",8);number(22,7,room_id%3+1);
 for(unsigned i=0;i<3;i++){
  text(13+i*5,10,i==room_id%3?"+":".",i==room_id%3?10:8);
  if(city_has_letter(&city,selected*3+i))sprite(2+i,98+i*40,68,FX_TILE+5*4,9,1,0);
 }
 panel(12,8);center(12,names[room_id]);center(14,instructions[selected]);
 center(16,city_has_letter(&city,room_id)?"SELECT: READ YOUR LETTER":"FIND THE LETTER OFF THE PATH");
 center(18,"LEFT / RIGHT ROUTE   A GO");center(19,"B CITY MAP");
}
static void letter_screen(void){clear_ui();game_mode=9;panel(2,16);center(3,"L O S T   A N D   F O U N D");sprite(0,112,42,FX_TILE+5*4,9,1,0);for(unsigned i=0;i<3;i++)center(9+i*2,letters[room_id][i]);center(17,"A / B BACK");}
/* Physics stays in the validated logical units. The presentation uses two
 * screen pixels per logical pixel, 16px metatiles, and a two-axis camera. */
static HOT uint16_t art(unsigned char c,unsigned x,unsigned y){unsigned t=0,p=9;switch(c){
 case '#':
  if(y==2)t=T_CEILING;
  else if(y>0&&world[(y-1)*ROOM_W+x]!='#'){
   t=region==2?T_MOSS:(region==1||region==3)?T_STEEL:T_ROOF;
   if(t==T_ROOF){if(x&&world[y*ROOM_W+x-1]!='#')t=T_ROOF_LEFT;else if(x+1<ROOM_W&&world[y*ROOM_W+x+1]!='#')t=T_ROOF_RIGHT;}
  }else{t=(x%5==2&&y%3==0)?T_WINDOW:T_WALL;if(t==T_WINDOW)p=15;}
  break;
 case '^':t=T_SPIKES;p=12;break;case 'a':t=T_GATE_CYAN;p=10;break;case 'b':t=T_GATE_AMBER;p=11;break;
 case 'u':t=player.relays&1?T_SIGNAL_ON:T_SIGNAL_OFF;p=10;break;
 case 'v':t=player.relays&2?T_POWER_ON:T_POWER_OFF;p=11;break;
 case '=':case '+':t=player.relays&(c=='='?1:2)?T_BRIDGE_ON:T_BRIDGE_OFF;p=c=='='?10:11;break;
 case 'l':t=player.letter?0:T_LETTER;p=11;break;
 case 'c':t=T_FLAG;p=player.checkpoint>=x?10:13;break;
 case 'j':t=T_SPRING;p=13;break;case 'r':t=T_ROPE;p=11;break;case 'w':t=T_WIND;p=10;break;
 case 'h':t=T_COIL;p=(player.clock&64)?12:14;break;
 case 'o':t=T_REFILL;p=13;break;case 'E':t=T_EXIT;p=player.relays==3?13:14;break;
 }return t?t|(p<<12):0;
}
static int clamp(int n,int lo,int hi){return n<lo?lo:n>hi?hi:n;}
static unsigned supports_courier(unsigned t){return t=='#'||(t=='='&&(player.relays&1))||(t=='+'&&(player.relays&2));}
static int courier_y(void){
 // Whole-pixel collision leaves fractional gravity motion while resting on a floor.
 // Anchor the drawing to that floor; retain half-pixel precision in the air.
 int x=player.x/16,y=player.y/16;
 if(player.vy>=0&&(supports_courier(tile(room_id,x,y+11))||supports_courier(tile(room_id,x+5,y+11))))return y*2;
 return player.y/8;
}
static HOT void follow_camera(unsigned immediate){
 int tx=clamp(player.x/8-(player.face?88:144),0,ROOM_W*16-240);
 int ty=clamp(courier_y()-82,0,ROOM_H*16-144);
 if(immediate){camera=tx;camera_y=ty;return;}
 int dx=tx-(int)camera,dy=ty-(int)camera_y;
 camera+=clamp(dx/4+(dx>0)-(dx<0),-8,8);
 camera_y+=clamp(dy/4+(dy>0)-(dy<0),-8,8);
}
static HOT void hud(void){
 panel(0,1);text(1,0,player.phase?"AMBER":"CYAN",player.phase?11:10);
 text(8,0,player.charge?"DASH +":"DASH .",8);text(16,0,player.relays&1?"S+":"S.",10);text(20,0,player.relays&2?"P+":"P.",11);text(25,0,player.letter?"MAIL+":"MAIL.",8);
 if(notice){panel(18,2);center(18,radio_text);}
 else if(room_id==0){
  int x=player.x/16;panel(18,2);
  center(18,x<78?"A: JUMP. HOLD FOR MORE HEIGHT.":!(player.relays&1)?"TOUCH THE DIAMOND WHILE CYAN.":!(player.relays&2)?"B: DASH TURNS YOUR COAT AMBER.":!player.letter?"JUMP + UP/B: FIND THE LETTER.":"BOTH LIGHTS ON. MARA IS AHEAD.");
 }else if(room_time<180){panel(18,2);center(18,names[room_id]);}
}
static HOT void room_render(void){
 memset(overlay,0,32*sizeof(*overlay));memset(overlay+18*32,0,64*sizeof(*overlay));follow_camera(0);
 unsigned first=camera/16,top=camera_y/16;
 for(unsigned row=0;row<11;row++)for(unsigned col=0;col<16;col++){
  unsigned x=first+col,y=top+row,xx=x*2,yy=y*2;
  uint16_t a=(x<ROOM_W&&y<ROOM_H)?art(world[y*ROOM_W+x],x,y):0;
  unsigned prop=0;
  if(x>1&&x<ROOM_W-2&&y>3&&y<ROOM_H-1&&world[y*ROOM_W+x]==' '){
   if(world[(y+1)*ROOM_W+x]=='#'&&x%7==3)prop=region==2?T_PLANT:region==1?T_PIPE:T_FINIAL;
   if(world[(y+1)*ROOM_W+x]=='#'&&x%13==5)prop=T_SKYLIGHT;
   if(world[(y-1)*ROOM_W+x]=='#'&&x%9==4)prop=T_LAMP;
  }
  for(unsigned dy=0;dy<2;dy++)for(unsigned dx=0;dx<2;dx++){
   unsigned at=((yy+dy)&31)*32+((xx+dx)&31),sub=dy*2+dx;
   foreground[at]=a?a+sub:0;
   decoration[at]=prop?(prop+sub)|(15<<12):0;
  }
 }
 hide_sprites();
 unsigned frame=player.dash?7:!(player.ground||(player.coyote&&player.vy>=0))?(player.wall&&player.vy>0?8:player.vy<0?5:6):landing?9:player.vx?1+(player.clock/5)%4:0;
 if(tile(room_id,player.x/16+3,player.y/16+5)=='r'&&player.vy)frame=8;
 int px=player.x/8-(int)camera-10,py=courier_y()-(int)camera_y-8;
 sprite(0,px,py,frame*16,player.phase,2,!player.face);
 for(unsigned i=4;i>0;i--){trail_x[i]=trail_x[i-1];trail_y[i]=trail_y[i-1];}trail_x[0]=player.x/8;trail_y[0]=courier_y();
 if(player.dash)for(unsigned i=1;i<5;i++){
  sprite(24+i,trail_x[i]-(int)camera-10,trail_y[i]-(int)camera_y-8,7*16,player.phase,2,!player.face);
  objects[24+i].a|=0x0400;objects[24+i].c|=1<<10;
 }
 for(unsigned i=0;i<enemy_count;i++)if(!(player.defeated&(1u<<i))){
  sprite(2+i,enemy_x(i,player.clock)*2-(int)camera-1,enemies[i].y*2-(int)camera_y,FX_TILE+6*4,10,1,0);
 }
 sprite(12,(ROOM_W-4)*16-4-(int)camera,240-(int)camera_y,NPC_TILE+region*16,2+region,2,0);
 // The waiting neighbor waves their signal lantern when the route is live.
 if(player.relays==3)for(unsigned i=0;i<4;i++)sprite(14+i,(ROOM_W-3)*16-(int)camera+(int)((frame_tick+i*7)%19)-9,232-(int)camera_y+(frame_tick/3+i*11)%27,FX_TILE+(i%4)*4,8+i%2,1,0);
 // Region-specific motes: rain, embers, pollen, cloud wisps, stars, static.
 for(unsigned i=0;i<8;i++){
  int x=((i*47+frame_tick/(region==4?2:3))%260)-16;
  int y=12+(i*31+frame_tick/(region==2?10:3))%136;
  unsigned weather=region==0?8:region==1||region==5?9:region==3?11:10;
  sprite(48+i,x,y,FX_TILE+weather*4,region==1?9:13,1,0);objects[48+i].c|=2<<10;
 }
 if(shake){shake--;for(unsigned i=0;i<8;i++)sprite(64+i,px+(int)((i*19+frame_tick*3)%60)-20,py+(int)((i*11+frame_tick*2)%44)-12,FX_TILE+(i%4)*4,8+player.phase,1,0);}
 hud();
}
static void enter_room(unsigned resume){
 clear_ui();region_art(room_id/3);load_room(room_id);init_state(&player,room_id);
 if(resume&&city.progress[room_id/3]==room_id%3){player.checkpoint=city.checkpoint[room_id/3];player.relays=city.relays[room_id/3];respawn(&player,room_id);}
 player.letter=city_has_letter(&city,room_id);game_mode=1;entry_latch=previous_keys&(KEY_A|KEY_B|KEY_START);room_time=0;landing=0;old_ground=0;follow_camera(1);
 for(unsigned i=0;i<5;i++){trail_x[i]=player.x/8;trail_y[i]=courier_y();}
 room_render();save();
}
static void delivery(void){clear_ui();game_mode=6;panel(1,3);center(2,"D E L I V E R Y   M A D E");sprite(0,88,22,PORTRAIT_TILE+(room_id/3)*64,2+room_id/3,3,0);panel(11,9);center(11,people[room_id/3]);for(unsigned i=0;i<3;i++)center(12+i*2,deliveries[room_id][i]);center(18,player.letter?"A LETTER FINDS ITS WAY HOME":"A LOST LETTER STILL WAITS");center(19,"A CONTINUE");audio_fx(EV_EXIT,120);}
static void celebration(void){clear_ui();game_mode=7;timer=0;panel(1,3);center(2,"A NEIGHBORHOOD AWAKES");sprite(0,88,27,PORTRAIT_TILE+selected*64,2+selected,3,0);panel(10,10);center(10,locations[selected]);for(unsigned i=0;i<3;i++)center(12+i*2,restoration[selected][i]);center(19,"A RETURN TO THE CITY");}
static void homecoming(void){clear_ui();game_mode=10;region_art(0);panel(0,4);center(1,"SIX LIGHTS. ONE CITY.");center(3,"YOU BROUGHT US HOME.");for(unsigned i=0;i<6;i++)sprite(i,18+i*35,56,NPC_TILE+i*16,2+i,2,0);panel(12,8);center(12,"FOR EVERY GENERATION");center(14,"THAT KEPT A LIGHT ON.");text(5,16,"LETTERS DELIVERED",8);number(23,16,mail_count());center(18,mail_count()==18?"EVERY LETTER FOUND. THANK YOU!":"THE CITY STILL HAS STORIES");center(19,"A EXPLORE AGAIN");}
static void update(unsigned keys){unsigned pressed=keys&~previous_keys;previous_keys=keys;audio_tick();
 if(game_mode==0){if(pressed&(KEY_A|KEY_START))atlas();return;}
 if((pressed&KEY_L)||((pressed&KEY_SELECT)&&(keys&KEY_UP))){if(game_mode==1||game_mode==2){selected=room_id/3;remember();}atlas();return;}
 if(game_mode==4){unsigned dir=pressed&KEY_LEFT?LEFT:pressed&KEY_RIGHT?RIGHT:pressed&KEY_UP?UP:pressed&KEY_DOWN?DOWN:0;if(dir){selected=city_neighbor(selected,dir);atlas();}if(city_unlocked(&city,selected)){if(pressed&(KEY_A|KEY_START)){room_id=selected*3+(city.progress[selected]<3?city.progress[selected]:0);briefing();}else if(pressed&KEY_B){room_id=selected*3+(city.progress[selected]<3?city.progress[selected]:0);enter_room(1);}}return;}
 if(game_mode==5){if((pressed&KEY_LEFT)&&room_id%3){room_id--;briefing();}else if((pressed&KEY_RIGHT)&&room_id%3<2&&room_id%3<city.progress[selected]){room_id++;briefing();}if(pressed&(KEY_A|KEY_START))enter_room(1);else if(pressed&KEY_B)atlas();else if((pressed&KEY_SELECT)&&city_has_letter(&city,room_id))letter_screen();return;}
 if(game_mode==9){if(pressed&(KEY_A|KEY_B|KEY_START))briefing();return;}
 if(game_mode==6){if(pressed&(KEY_A|KEY_START)){if(room_id%3==2){if(first_restoration)celebration();else atlas();}else{room_id++;enter_room(1);}}return;}
 if(game_mode==7||game_mode==10){timer++;for(unsigned i=0;i<24;i++)sprite(30+i,(i*39+timer)%256-8,25+(i*13+timer/2)%60,FX_TILE+(i%4)*4,8+i%2,1,0);if(pressed&(KEY_A|KEY_START)){if(game_mode==7&&city_restored(&city)==6)homecoming();else atlas();}return;}
 if(game_mode==2){if(pressed&(KEY_A|KEY_START)){game_mode=1;memset(overlay,0,sizeof(overlay));room_render();}else if(pressed&KEY_B){remember();atlas();}return;}
 if(pressed&KEY_START){game_mode=2;panel(7,6);center(8,"TAKE A BREATHER");center(10,"A / START RESUME");center(12,"B / L CITY MAP");return;}
 if(pressed&KEY_SELECT){deaths++;respawn(&player,room_id);follow_camera(1);}
 // A held menu-confirm press must not start the new room's hazard clock.
 if(entry_latch&&(keys&entry_latch)&&!(keys&(KEY_LEFT|KEY_RIGHT|KEY_UP|KEY_DOWN))){room_render();return;}
 entry_latch=0;
 unsigned input=0;if(keys&KEY_LEFT)input|=LEFT;if(keys&KEY_RIGHT)input|=RIGHT;if(keys&KEY_UP)input|=UP;if(keys&KEY_DOWN)input|=DOWN;if(keys&KEY_A)input|=JUMP;if(keys&(KEY_B|KEY_R))input|=DASH;
 unsigned old_relays=player.relays,old_letter=player.letter;unsigned ev=step(&player,room_id,input);simulation_frames++;room_time++;
 unsigned grounded=player.ground||(player.coyote&&player.vy>=0);
 if(grounded&&!old_ground)landing=5;else if(landing)landing--;old_ground=grounded;
 if(old_relays!=player.relays){radio_text=radio_lines[room_id][(old_relays^player.relays)&1?0:1];notice=150;shake=22;remember();audio_fx(EV_RELAY,player.x/8-camera);}
 if(old_letter!=player.letter){radio_text="A LETTER FOUND ITS COURIER!";notice=150;shake=18;remember();audio_fx(EV_REFILL,120);}
 if(ev&EV_DIE){deaths++;respawn(&player,room_id);follow_camera(1);audio_fx(EV_DIE,120);shake=8;}
 else if(ev&EV_EXIT){selected=room_id/3;first_restoration=city.progress[selected]<3;if(city.progress[selected]<room_id%3+1){city.progress[selected]=room_id%3+1;city.checkpoint[selected]=0;city.relays[selected]=0;}save();delivery();return;}
 else if(ev&EV_CHECKPOINT){remember();if(!notice){radio_text="CHECKPOINT - YOUR LIGHT IS SAFE";notice=100;}audio_fx(EV_REFILL,120);}
 else if(ev&(EV_JUMP|EV_DASH|EV_SPRING))audio_fx(ev&EV_DASH?EV_DASH:EV_JUMP,player.x/8-camera);
 if(notice){notice--;}
 unsigned c=tile(room_id,player.x/16+3,player.y/16+5);if((c=='u'&&player.phase)||(c=='v'&&!player.phase)){radio_text=c=='u'?"DASH TO CYAN: SIGNAL CONNECTS":"DASH TO AMBER: POWER CONNECTS";notice=30;}if(c=='E'&&player.relays!=3){radio_text="RECONNECT SIGNAL AND POWER";notice=30;}
 room_render();
}
static HOT void present(void){
 copy16(MAP(30),foreground,1024);copy16(MAP(31),overlay,1024);copy16(MAP(29),decoration,1024);
 copy16((volatile uint16_t *)0x07000000,objects,512);
 REG16(0)=0x1f40;
 REG16(0x10)=0;REG16(0x12)=0;
 REG16(0x14)=camera&255;REG16(0x16)=camera_y&255;
 REG16(0x18)=camera&255;REG16(0x1a)=camera_y&255;
 REG16(0x1c)=game_mode==1?camera*16/(ROOM_W*16-240):8;REG16(0x1e)=0;
 // Reserve the blend unit for translucent dash afterimages. Palette shading
 // keeps scenery quiet and the city visibly warms as circuits come online.
 unsigned light=game_mode==1?((player.relays==3)?27:22):32;
 for(unsigned i=0;i<128;i++){unsigned c=region_palettes[scene][i];BG_PALETTE[i]=RGB((c&31)*light/32,((c>>5)&31)*light/32,((c>>10)&31)*light/32);}
 REG16(0x50)=0x3f40;REG16(0x52)=0x0b05;
}
int main(void){
 REG16(0)=0x80;REG16(0x204)=0x4317; /* cartridge prefetch, conservative SRAM wait */
 REG16(8)=31<<8;REG16(10)=(30<<8)|1;REG16(12)=(29<<8)|2;REG16(14)=(28<<8)|(1<<2)|0x80|3;
 copy16(VRAM,terrain_data[0],8192);
 copy16((volatile uint16_t *)0x06010000,sprite_data,sizeof(sprite_data)/2);copy16(OBJ_PALETTE,sprite_palette,256);
 *(void (**)(void))0x03007ffc=irq;REG16(4)=8;REG16(0x200)=1|16;REG16(0x208)=1;audio_init();load();title();
 unsigned seen=frame_tick;
 while(1){while(frame_tick==seen)__asm__ volatile("swi 0x020000":::"memory");seen=frame_tick;unsigned active=game_mode==1;present();update((~REG16(0x130))&1023);if(active&&game_mode==1&&frame_tick!=seen)render_overruns++;}
}
