#include "city.h"
#include <string.h>
const uint8_t city_x[6]={2,7,7,12,12,17},city_y[6]={8,4,11,4,11,8};
/* left, right, up, down follow the drawn streets, including vertical branch choices. */
static const uint8_t neighbors[6][4]={{0,1,1,2},{0,3,1,2},{0,4,1,2},{1,5,3,4},{2,5,3,4},{3,5,3,4}};
uint8_t city_neighbor(uint8_t n,uint8_t direction){return neighbors[n][direction==LEFT?0:direction==RIGHT?1:direction==UP?2:3];}
uint8_t city_unlocked(const City *c,uint8_t n){if(n==0)return 1;if(n<3)return c->progress[0]==3;if(n==3)return c->progress[1]==3;if(n==4)return c->progress[2]==3;return c->progress[3]==3&&c->progress[4]==3;}
uint8_t city_restored(const City *c){uint8_t i,n=0;for(i=0;i<6;i++)if(c->progress[i]==3)n++;return n;}
uint8_t city_has_letter(const City *c,uint8_t room){return (c->letters[room/8]>>(room&7))&1;}
void city_take_letter(City *c,uint8_t room){c->letters[room/8]|=1u<<(room&7);}
void city_encode(const City *c,uint8_t *b){uint8_t i,sum=37;memset(b,0,SAVE_SIZE);b[0]=0x50;b[1]=3;b[2]=c->selected;for(i=0;i<6;i++){b[3+i]=c->progress[i];b[9+i]=c->checkpoint[i];b[16+i]=c->relays[i];}for(i=0;i<3;i++)b[22+i]=c->letters[i];for(i=2;i<31;i++)sum+=b[i];b[31]=sum;}
uint8_t city_decode(City *c,const uint8_t *b){
 uint8_t i,sum=37;if(b[0]!=0x50||(b[1]!=2&&b[1]!=3)||b[2]>=6)return 0;
 for(i=0;i<6;i++)if(b[3+i]>3||(b[9+i]!=0&&b[9+i]!=20&&b[9+i]!=40&&b[9+i]!=60))return 0;
 if(b[1]==2){for(i=2;i<15;i++)sum+=b[i];if(sum!=b[15])return 0;}
 else{for(i=2;i<31;i++)sum+=b[i];if(sum!=b[31])return 0;for(i=0;i<6;i++)if(b[16+i]>3)return 0;if(b[24]&252)return 0;}
 memset(c,0,sizeof(*c));c->selected=b[2];for(i=0;i<6;i++){c->progress[i]=b[3+i];if(b[1]==3){c->checkpoint[i]=b[9+i];c->relays[i]=b[16+i];}}
 if(b[1]==3){for(i=0;i<3;i++)c->letters[i]=b[22+i];}
 return b[1]==2?2:1;
}
