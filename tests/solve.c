#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/engine.h"
#define MAX 4000000
#define HASH 8388608
typedef struct { State s; int parent,cost,score; unsigned char action; } Node;
static int want_letter,timed;
static Node nodes[MAX];static unsigned int seen[HASH];static int heap[MAX],hn;
static unsigned char actions[]={RIGHT,RIGHT|JUMP,RIGHT|DASH,UP|RIGHT|DASH,UP|DASH,UP,UP|RIGHT,0,JUMP,LEFT,LEFT|JUMP,LEFT|DASH,UP|LEFT|DASH,DOWN};
unsigned int hash(State *s){
 unsigned int h=2166136261u;int v[]={s->x/24,s->y/24,s->vx/8,s->vy/8,s->phase,s->charge,s->coyote>0,s->buffer>0,s->dash,s->prev,s->lock,timed?s->clock/8:0,s->defeated,s->boost,s->relays,want_letter?s->letter:0};
 for(int i=0;i<16;i++)h=(h^(unsigned int)v[i])*16777619u;return h?h:1;
}
void push(int n){int i=++hn;while(i>1&&nodes[heap[i/2]].score>nodes[n].score){heap[i]=heap[i/2];i/=2;}heap[i]=n;}
int pop(void){int n=heap[1],v=heap[hn--],i=1,j;while((j=i*2)<=hn){if(j<hn&&nodes[heap[j+1]].score<nodes[heap[j]].score)j++;if(nodes[v].score<=nodes[heap[j]].score)break;heap[i]=heap[j];i=j;}heap[i]=v;return n;}
static int distance(int x,int y,int xx,int yy){return abs(x-xx)+abs(y-yy)*2;}
static int path(int x,int y,int mask){
 int best=100000;
 if(!mask)return distance(x,y,(ROOM_W-3)*8,125);
 for(int i=0;i<3;i++)if(mask&(1<<i)){
  Enemy p=i==2?letter_point:relay_points[i];int d=distance(x,y,p.x,p.y)+path(p.x,p.y,mask&~(1<<i));if(d<best)best=d;
 }
 return best;
}
int main(int argc,char **argv){
 want_letter=argc>2;
 int first=argc>1?atoi(argv[1]):0,last=argc>1?first+1:ROOMS;
 for(int r=first;r<last;r++){
  memset(seen,0,sizeof(seen));load_room(r);timed=enemy_count>0;for(int i=0;i<ROOM_W*ROOM_H;i++)if(world[i]=='h')timed=1;init_state(&nodes[0].s,r);nodes[0].parent=-1;nodes[0].cost=0;nodes[0].score=0;hn=0;push(0);int end=1,found=-1,best=0;
  while(hn&&end<MAX-20&&found<0){int n=pop();
   for(int a=0;a<(int)sizeof(actions);a++){
    State s=nodes[n].s;int ev=0;
    for(int f=0;f<6;f++){ev=step(&s,r,actions[a]);if(ev&(EV_DIE|EV_EXIT))break;}
    if(ev&EV_DIE)continue;if((ev&EV_EXIT)&&want_letter&&!s.letter)continue;
    unsigned int h=hash(&s),j=h&(HASH-1);while(seen[j]&&seen[j]!=h)j=(j+1)&(HASH-1);if(seen[j]&&!(ev&EV_EXIT))continue;seen[j]=h;
    int cost=nodes[n].cost+6,remain=path(s.x/16,s.y/16,(3&~s.relays)|((want_letter&&!s.letter)?4:0));
    nodes[end]=(Node){s,n,cost,cost+remain*2,actions[a]};if(s.x/16>best)best=s.x/16;
    if(ev&EV_EXIT){found=end;break;}push(end++);
   }
  }
  if(found<0){printf("Room %d FAILED after %d states, furthest x=%d\n",r+1,end,best);fflush(stdout);return 1;}
  unsigned char path[4000];int count=0;for(int n=found;nodes[n].parent>=0;n=nodes[n].parent)path[count++]=nodes[n].action;
  printf("[%d/%d] %s: solved in %d frames, %d states\n",r+1,ROOMS,names[r],count*6,end);fflush(stdout);
  char fn[64];
#ifdef POLARITY_ADVANCE
  snprintf(fn,sizeof(fn),want_letter?"artifacts/gba-route-letter-%d.json":"artifacts/gba-route-%d.json",r);
#else
  snprintf(fn,sizeof(fn),want_letter?"artifacts/route-letter-%d.json":"artifacts/route-%d.json",r);
#endif
  FILE *fp=fopen(fn,"w");fputc('[',fp);for(int i=count-1;i>=0;i--)fprintf(fp,"%s%d",i==count-1?"":",",path[i]);fputs("]\n",fp);fclose(fp);
 }
}
