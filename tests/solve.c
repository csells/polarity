#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/engine.h"
#define MAX 1600000
#define HASH 8388608
typedef struct { State s; int parent; unsigned char action; } Node;
static Node nodes[MAX];static unsigned int seen[HASH];
static unsigned char actions[]={0,RIGHT,LEFT,JUMP,RIGHT|JUMP,LEFT|JUMP,RIGHT|DASH,LEFT|DASH,UP|DASH,UP|RIGHT|DASH,UP|LEFT|DASH,DOWN|DASH};
unsigned int hash(State *s){
 unsigned int h=2166136261u;int v[]={s->x/24,s->y/24,s->vx/8,s->vy/8,s->phase,s->charge,s->coyote>0,s->buffer>0,s->dash,s->prev,s->lock};
 for(int i=0;i<11;i++)h=(h^(unsigned int)v[i])*16777619u;return h?h:1;
}
int main(){
 for(int r=0;r<ROOMS;r++){
  memset(seen,0,sizeof(seen));init_state(&nodes[0].s,r);nodes[0].parent=-1;int end=1,found=-1;
  for(int n=0;n<end&&end<MAX-20&&found<0;n++){
   for(int a=0;a<12;a++){
    State s=nodes[n].s;int ev=0;
    for(int f=0;f<6;f++){ev=step(&s,r,actions[a]);if(ev&(EV_DIE|EV_EXIT))break;}
    if(ev&EV_DIE)continue;
    unsigned int h=hash(&s),j=h&(HASH-1);while(seen[j]&&seen[j]!=h)j=(j+1)&(HASH-1);if(seen[j]&&!(ev&EV_EXIT))continue;seen[j]=h;
    nodes[end]=(Node){s,n,actions[a]};if(ev&EV_EXIT){found=end;break;}end++;
   }
  }
  if(found<0){printf("Room %d FAILED after %d states\n",r+1,end);fflush(stdout);return 1;}
  unsigned char path[2000];int count=0;for(int n=found;nodes[n].parent>=0;n=nodes[n].parent)path[count++]=nodes[n].action;
  printf("Room %d solved: %d frames, %d explored states\n",r+1,count*6,end);fflush(stdout);
  char fn[64];snprintf(fn,sizeof(fn),"artifacts/route-%d.json",r);FILE *fp=fopen(fn,"w");fputc('[',fp);for(int i=count-1;i>=0;i--)fprintf(fp,"%s%d",i==count-1?"":",",path[i]);fputs("]\n",fp);fclose(fp);
 }
}
