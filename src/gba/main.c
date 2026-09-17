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
unsigned game_mode,room_id,selected,camera,deaths,simulation_frames;
static unsigned previous_keys,region=99,notice,timer,first_restoration,shake;
static uint16_t foreground[1024],overlay[1024],skyline[228];
static int trail_x[5],trail_y[5];
static const char *radio_text;
static const int map_x[6]={28,80,80,148,148,208},map_y[6]={76,48,103,48,103,76};
static const char *const region_sub[6]={"FOLLOW THE SIGNAL","WAKE THE NIGHT CREW","GIVE THE ROOTS WATER","CARRY EVERYONE HIGHER","FIND A WAY THROUGH THE STORM","BRING THE LAST LIGHT HOME"};
static const char *const instructions[6]={"A JUMP   B DASH / FLIP","STOMP OR DASH THROUGH BOTS","UP / DOWN CLIMB   A LET GO","SPRINGS LAUNCH - AIM YOUR DASH","RIDE THE WIND THEN DASH OUT","DIM COILS SAFE - BRIGHT HURTS"};
static void irq(void){unsigned flags=REG16(0x202);if(flags&1)frame_tick++;if(flags&16)audio_irq();*(volatile uint16_t *)0x03007ff8|=flags;REG16(0x202)=flags;}
void copy16(volatile uint16_t *dst,const void *src,unsigned count){REG32(0xd4)=(uint32_t)src;REG32(0xd8)=(uint32_t)dst;REG32(0xdc)=0x80000000|count;}
static void sprite(unsigned i,int x,int y,unsigned tile,unsigned bank,unsigned size,unsigned flip){
 if(x<-(int)(8<<size)||x>239||y<-(int)(8<<size)||y>159){objects[i].a=0x0200;return;}
 objects[i].a=y&255;objects[i].b=(x&511)|(size<<14)|(flip?0x1000:0);objects[i].c=tile|(bank<<12);}
static void hide_sprites(void){for(unsigned i=0;i<128;i++)objects[i].a=0x0200;}
static void text(unsigned x,unsigned y,const char *s,unsigned bank){while(*s&&x<30){overlay[y*32+x++]=(unsigned char)*s++|(bank<<12);}}
static void center(unsigned y,const char *s){unsigned n=0;while(s[n])n++;text(n<30?(30-n)/2:0,y,s,8);}
static void panel(unsigned y,unsigned h){for(unsigned yy=y;yy<y+h;yy++)for(unsigned x=0;x<30;x++)overlay[yy*32+x]=1|8<<12;}
static void number(unsigned x,unsigned y,unsigned n){char s[4];s[0]='0'+n/10%10;s[1]='0'+n%10;s[2]=0;text(x,y,s,8);}
static unsigned mail_count(void){unsigned n=0;for(unsigned i=0;i<18;i++)n+=city_has_letter(&city,i);return n;}
static void save(void){uint8_t bytes[SAVE_SIZE];city.selected=selected;city_encode(&city,bytes);bytes[0]=0x41;bytes[1]=1;volatile uint8_t *s=(volatile uint8_t *)0x0e000000;s[0]=0;for(unsigned i=1;i<SAVE_SIZE;i++)s[i]=bytes[i];s[0]=bytes[0];}
static void remember(void){unsigned n=room_id/3;if(city.progress[n]==room_id%3){city.checkpoint[n]=player.checkpoint;city.relays[n]=player.relays;}if(player.letter)city_take_letter(&city,room_id);save();}
static void load(void){uint8_t bytes[SAVE_SIZE];volatile uint8_t *s=(volatile uint8_t *)0x0e000000;for(unsigned i=0;i<SAVE_SIZE;i++)bytes[i]=s[i];if(bytes[0]==0x41&&bytes[1]==1){bytes[0]=0x50;bytes[1]=3;city_decode(&city,bytes);}selected=city.selected;}
static void region_art(unsigned n){
 if(region==n)return;
 REG16(0)=0;BG_PALETTE[0]=0;region=n;copy16(VRAM,backgrounds[n],BACKGROUND_BYTES/2);copy16(BG_PALETTE,region_palettes[n],256);
 copy16(MAP(28),scenery_maps[n][1],1024);copy16(MAP(29),scenery_maps[n][2],1024);
 for(unsigned y=0;y<228;y++){unsigned t=y<160?1+y*14/160:15;skyline[y]=region_palettes[n][t];}
 audio_region(n);
}
static void clear_ui(void){memset(overlay,0,sizeof(overlay));memset(foreground,0,sizeof(foreground));hide_sprites();camera=0;notice=0;}
static void title(void){clear_ui();game_mode=0;region_art(0);
 panel(2,1);center(2,"NIGHTSHIFT PRESENTS");for(unsigned y=0;y<4;y++)for(unsigned x=0;x<28;x++)overlay[(4+y)*32+x+1]=(896+y*28+x)|8<<12;
 center(9,"S T O R M   C O U R I E R");center(12,"ONE SPARK. EVERYONE CONNECTED.");center(15,"PRESS A OR START");center(18,"A GAME BOY ADVANCE ADVENTURE");
 sprite(0,112,88,0,0,1,0);
}
static void route(unsigned a,unsigned b){int x=map_x[a]/8,y=map_y[a]/8,tx=map_x[b]/8,ty=map_y[b]/8;while(x!=tx){x+=tx>x?1:-1;if(y<ty)y++;else if(y>ty)y--;foreground[y*32+x]=151|(city.progress[a]==3?4:3)<<12;}while(y!=ty){y+=ty>y?1:-1;foreground[y*32+x]=152|(city.progress[a]==3?4:3)<<12;}}
static void atlas(void){clear_ui();game_mode=4;region_art(selected);panel(0,3);center(0,"L U M E N   /   C O U R I E R");text(1,2,"AWAKE",8);number(7,2,city_restored(&city));text(18,2,"LETTERS",8);number(26,2,mail_count());
 route(0,1);route(0,2);route(1,3);route(2,4);route(3,5);route(4,5);
 for(unsigned i=0;i<6;i++){sprite(i,map_x[i]-16,map_y[i]-16,ICON_TILE+i*16,city.progress[i]==3?2+i:city_unlocked(&city,i)?8:9,2,0);}
 sprite(7,map_x[selected]-8,map_y[selected]-28,21*4,0,1,0);
 panel(16,4);center(16,locations[selected]);text(8,17,"DELIVERIES",8);number(19,17,city.progress[selected]);center(18,city_unlocked(&city,selected)?"A CHOOSE ROOM   B RESUME":"RESTORE THE CONNECTED DISTRICTS");
}
static void briefing(void){clear_ui();game_mode=5;region_art(selected);panel(0,3);center(1,locations[selected]);sprite(0,104,28,ICON_TILE+selected*16,2+selected,2,0);
 for(unsigned i=0;i<3;i++){char s[2]={'1'+i,0};text(9+i*5,9,s,8);if(i==room_id%3)text(9+i*5,8,"+",4);if(city_has_letter(&city,selected*3+i))sprite(2+i,68+i*40,80,22*4,1,1,0);}
 panel(11,9);center(11,names[room_id]);center(7,region_sub[selected]);center(13,instructions[selected]);center(15,city_has_letter(&city,room_id)?"SELECT: READ YOUR LETTER":"ONE LOST LETTER ON EVERY ROUTE");center(17,"LEFT / RIGHT CHOOSE   A GO");center(19,"B BACK TO THE CITY");
}
static void letter_screen(void){clear_ui();game_mode=9;panel(2,16);center(3,"L O S T   A N D   F O U N D");sprite(0,112,42,22*4,1,1,0);for(unsigned i=0;i<3;i++)center(9+i*2,letters[room_id][i]);center(17,"A / B BACK");}
static uint16_t art(unsigned char c,unsigned x,unsigned y){unsigned t=0,p=1;switch(c){
 case '#': t=(y>0&&world[(y-1)*ROOM_W+x]!='#')?(region==2?133:(region==1||region==3)?132:128):129;break;
 case '^':t=134;p=6;break;case 'a':t=135;p=4;break;case 'b':t=136;p=5;break;
 case 'u':t=player.relays&1?139:137;p=4;break;case 'v':t=player.relays&2?140:138;p=5;break;
 case '=':case '+':t=player.relays&(c=='='?1:2)?141:142;p=c=='='?4:5;break;
 case 'l':t=player.letter?0:143;p=5;break;case 'c':t=144;p=player.checkpoint>=x?4:7;break;
 case 'j':t=145;p=7;break;case 'r':t=146;p=5;break;case 'w':t=147;p=4;break;case 'h':t=148;p=(player.clock&64)?6:3;break;
 case 'o':t=149;p=7;break;case 'E':t=150;p=player.relays==3?7:3;break;
 }return t?t|(p<<12):0;}
static void hud(void){panel(0,2);text(0,0,player.phase?"AMBER":"CYAN ",player.phase?5:4);text(6,0,player.charge?"DASH+":"DASH-",8);text(13,0,"LINK",8);text(18,0,player.relays&1?"S":"-",4);text(20,0,player.relays&2?"P":"-",5);text(23,0,"MAIL",8);text(28,0,player.letter?"+":"-",5);
 if(notice)text(0,1,radio_text,8);
 else if(room_id==0){int x=player.x/16;text(0,1,x<95?"A: JUMP - HOLD TO GO HIGHER":player.relays==3?"JUMP THEN UP+B: REACH THE MAIL":!(player.relays&1)?"CYAN CONNECTS THE SIGNAL SOCKET":"B: DASH FLIPS CYAN / AMBER",8);}
 else text(0,1,names[room_id],8);
}
static void room_render(void){
 memset(overlay,0,sizeof(overlay));
 int target=player.x/16-104;if(target<0)target=0;if(target>ROOM_W*8-240)target=ROOM_W*8-240;camera=target;
 unsigned first=camera/8;
 for(unsigned y=0;y<20;y++)for(unsigned col=0;col<32;col++){unsigned x=first+col;foreground[y*32+(x&31)]=(y>=2&&x<ROOM_W)?art(world[(y-2)*ROOM_W+x],x,y-2):0;}
 hide_sprites();unsigned frame=player.dash?7:!player.ground?(player.vy<0?5:6):player.vx?1+(player.clock/6)%4:0;
 if(tile(room_id,player.x/16+3,player.y/16+5)=='r'&&player.vy)frame=8;
 int px=player.x/16-camera-5,py=player.y/16+12;sprite(0,px,py,frame*4,player.phase,1,!player.face);
 for(unsigned i=4;i>0;i--){trail_x[i]=trail_x[i-1];trail_y[i]=trail_y[i-1];}trail_x[0]=player.x/16;trail_y[0]=player.y/16;
 if(player.dash)for(unsigned i=1;i<5;i++)sprite(24+i,trail_x[i]-camera-2,trail_y[i]+14,(17+i%4)*4,player.phase,1,0);
 for(unsigned i=0;i<enemy_count;i++)if(!(player.defeated&(1u<<i)))sprite(2+i,enemy_x(i,player.clock)-camera-4,enemies[i].y+12,16*4,1,1,0);
 sprite(12,604-camera,136,(10+region)*4,2+region,1,0);
 if(player.relays==3){for(unsigned i=0;i<4;i++)sprite(14+i,615-camera+(int)((frame_tick+i*7)%19)-9,122+(frame_tick/3+i*11)%27,20*4,player.phase,1,0);}
 // Weather sits behind the courier; no collision or input effects.
 for(unsigned i=0;i<12;i++){int x=((i*47+frame_tick/(region==4?1:3))%260)-16;int y=30+(i*31+frame_tick/(region==2?8:3))%124;sprite(48+i,x,y,20*4,region==1?1:0,1,0);objects[48+i].c|=2<<10;}
 if(shake){shake--;for(unsigned i=0;i<8;i++)sprite(64+i,px+(int)((i*19+frame_tick*3)%60)-24,py+(int)((i*11+frame_tick*2)%44)-22,(18+i%3)*4,player.phase,1,0);}
 hud();
}
static void enter_room(unsigned resume){clear_ui();region_art(room_id/3);load_room(room_id);init_state(&player,room_id);if(resume&&city.progress[room_id/3]==room_id%3){player.checkpoint=city.checkpoint[room_id/3];player.relays=city.relays[room_id/3];respawn(&player,room_id);}player.letter=city_has_letter(&city,room_id);game_mode=1;for(unsigned i=0;i<5;i++){trail_x[i]=player.x/16;trail_y[i]=player.y/16;}room_render();save();}
static void delivery(void){clear_ui();game_mode=6;panel(1,3);center(2,"D E L I V E R Y   M A D E");sprite(0,104,40,PORTRAIT_TILE+(room_id/3)*16,2+room_id/3,2,0);panel(10,10);center(10,people[room_id/3]);for(unsigned i=0;i<3;i++)center(12+i*2,deliveries[room_id][i]);center(18,player.letter?"A LETTER FINDS ITS WAY HOME":"A LOST LETTER STILL WAITS");center(19,"A CONTINUE");audio_fx(EV_EXIT,120);}
static void celebration(void){clear_ui();game_mode=7;timer=0;panel(1,3);center(2,"A NEIGHBORHOOD AWAKES");sprite(0,104,42,ICON_TILE+selected*16,2+selected,2,0);panel(10,10);center(10,locations[selected]);for(unsigned i=0;i<3;i++)center(12+i*2,restoration[selected][i]);center(19,"A RETURN TO THE CITY");}
static void homecoming(void){clear_ui();game_mode=10;region_art(0);panel(0,4);center(1,"SIX LIGHTS. ONE CITY.");center(3,"YOU BROUGHT US HOME.");for(unsigned i=0;i<6;i++)sprite(i,18+i*35,56,ICON_TILE+i*16,2+i,2,0);panel(12,8);center(12,"FOR EVERY GENERATION");center(14,"THAT KEPT A LIGHT ON.");text(5,16,"LETTERS DELIVERED",8);number(23,16,mail_count());center(18,mail_count()==18?"EVERY LETTER FOUND. THANK YOU!":"THE CITY STILL HAS STORIES");center(19,"A EXPLORE AGAIN");}
static void update(unsigned keys){unsigned pressed=keys&~previous_keys;previous_keys=keys;
 if(game_mode==0){if(pressed&(KEY_A|KEY_START))atlas();return;}
 if((pressed&KEY_L)||((pressed&KEY_SELECT)&&(keys&KEY_UP))){if(game_mode==1||game_mode==2){selected=room_id/3;remember();}atlas();return;}
 if(game_mode==4){unsigned dir=pressed&KEY_LEFT?LEFT:pressed&KEY_RIGHT?RIGHT:pressed&KEY_UP?UP:pressed&KEY_DOWN?DOWN:0;if(dir){selected=city_neighbor(selected,dir);atlas();}if(city_unlocked(&city,selected)){if(pressed&(KEY_A|KEY_START)){room_id=selected*3+(city.progress[selected]<3?city.progress[selected]:0);briefing();}else if(pressed&KEY_B){room_id=selected*3+(city.progress[selected]<3?city.progress[selected]:0);enter_room(1);}}return;}
 if(game_mode==5){if((pressed&KEY_LEFT)&&room_id%3){room_id--;briefing();}else if((pressed&KEY_RIGHT)&&room_id%3<2&&room_id%3<city.progress[selected]){room_id++;briefing();}if(pressed&(KEY_A|KEY_START))enter_room(1);else if(pressed&KEY_B)atlas();else if((pressed&KEY_SELECT)&&city_has_letter(&city,room_id))letter_screen();return;}
 if(game_mode==9){if(pressed&(KEY_A|KEY_B|KEY_START))briefing();return;}
 if(game_mode==6){if(pressed&(KEY_A|KEY_START)){if(room_id%3==2){if(first_restoration)celebration();else atlas();}else{room_id++;enter_room(1);}}return;}
 if(game_mode==7||game_mode==10){timer++;for(unsigned i=0;i<24;i++)sprite(30+i,(i*39+timer)%256-8,25+(i*13+timer/2)%60,(18+i%3)*4,i%2,1,0);if(pressed&(KEY_A|KEY_START)){if(game_mode==7&&city_restored(&city)==6)homecoming();else atlas();}return;}
 if(game_mode==2){if(pressed&(KEY_A|KEY_START)){game_mode=1;room_render();}else if(pressed&KEY_B){remember();atlas();}return;}
 if(pressed&KEY_START){game_mode=2;panel(7,6);center(8,"TAKE A BREATHER");center(10,"A / START RESUME");center(12,"B / L CITY MAP");return;}
 if(pressed&KEY_SELECT){deaths++;respawn(&player,room_id);}
 unsigned input=0;if(keys&KEY_LEFT)input|=LEFT;if(keys&KEY_RIGHT)input|=RIGHT;if(keys&KEY_UP)input|=UP;if(keys&KEY_DOWN)input|=DOWN;if(keys&KEY_A)input|=JUMP;if(keys&(KEY_B|KEY_R))input|=DASH;
 unsigned old_relays=player.relays,old_letter=player.letter;unsigned ev=step(&player,room_id,input);simulation_frames++;
 if(old_relays!=player.relays){radio_text=radio_lines[room_id][(old_relays^player.relays)&1?0:1];notice=150;shake=22;remember();audio_fx(EV_RELAY,player.x/16-camera);}
 if(old_letter!=player.letter){radio_text="A LETTER FOUND ITS COURIER!";notice=150;shake=18;remember();audio_fx(EV_REFILL,120);}
 if(ev&EV_DIE){deaths++;respawn(&player,room_id);audio_fx(EV_DIE,120);shake=8;}
 else if(ev&EV_EXIT){selected=room_id/3;first_restoration=city.progress[selected]<3;if(city.progress[selected]<room_id%3+1){city.progress[selected]=room_id%3+1;city.checkpoint[selected]=0;city.relays[selected]=0;}save();delivery();return;}
 else if(ev&EV_CHECKPOINT){remember();if(!notice){radio_text="CHECKPOINT - YOUR LIGHT IS SAFE";notice=100;}audio_fx(EV_REFILL,120);}
 else if(ev&(EV_JUMP|EV_DASH|EV_SPRING))audio_fx(ev&EV_DASH?EV_DASH:EV_JUMP,player.x/16-camera);
 if(notice){notice--;}
 unsigned c=tile(room_id,player.x/16+3,player.y/16+5);if((c=='u'&&player.phase)||(c=='v'&&!player.phase)){radio_text=c=='u'?"DASH TO CYAN: SIGNAL CONNECTS":"DASH TO AMBER: POWER CONNECTS";notice=30;}if(c=='E'&&player.relays!=3){radio_text="RECONNECT SIGNAL AND POWER";notice=30;}
 room_render();
}
static void present(void){
 unsigned awake=city.progress[region]==3||(game_mode==1&&player.relays==3);
 BG_PALETTE[35]=awake?region_palettes[region][22]:region_palettes[region][35];
 BG_PALETTE[52]=awake?RGB(31,25,16):region_palettes[region][52];
 REG32(0xb8)=0;BG_PALETTE[0]=skyline[0];REG32(0xb0)=(uint32_t)(skyline+1);REG32(0xb4)=0x05000000;REG32(0xb8)=0xa2400001;
 copy16(MAP(30),foreground,1024);copy16(MAP(31),overlay,1024);copy16((volatile uint16_t *)0x07000000,objects,512);
 REG16(0)=0x1f40;REG16(0x10)=0;REG16(0x12)=0;REG16(0x14)=camera&255;REG16(0x16)=0;REG16(0x18)=(camera/2+frame_tick/180)&255;REG16(0x1a)=0;REG16(0x1c)=camera/4;REG16(0x1e)=0;
}
int main(void){
 REG16(0)=0x80;REG16(0x204)=0x4317; /* cartridge prefetch, conservative SRAM wait */
 REG16(8)=31<<8;REG16(10)=(30<<8)|1;REG16(12)=(29<<8)|2;REG16(14)=(28<<8)|3;
 copy16((volatile uint16_t *)0x06010000,sprite_data,sizeof(sprite_data)/2);copy16(OBJ_PALETTE,sprite_palette,160);
 *(void (**)(void))0x03007ffc=irq;REG16(4)=8;REG16(0x200)=1|16;REG16(0x208)=1;audio_init();load();title();
 unsigned seen=frame_tick;
 while(1){while(frame_tick==seen)__asm__ volatile("swi 0x020000":::"memory");seen=frame_tick;present();update((~REG16(0x130))&1023);}
}
