#include <stddef.h>
#include <stdint.h>
#include "engine.h"
HOT void *memset(void *dst,int c,size_t n){
 unsigned char *p=dst;
 while(n&&((uintptr_t)p&3)){*p++=c;n--;}
 uint32_t fill=(unsigned char)c;fill|=fill<<8;fill|=fill<<16;
 uint32_t *words=(uint32_t *)p;
 while(n>=4){*words++=fill;n-=4;}
 p=(unsigned char *)words;while(n--)*p++=c;
 return dst;
}
void *memcpy(void *dst,const void *src,size_t n){unsigned char *d=dst;const unsigned char *s=src;while(n--)*d++=*s++;return dst;}
