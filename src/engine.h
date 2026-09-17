#ifndef ENGINE_H
#define ENGINE_H
#include <stdint.h>
#define LEFT 1
#define RIGHT 2
#define UP 4
#define DOWN 8
#define JUMP 16
#define DASH 32
#define ROOMS 8
#define EV_JUMP 1
#define EV_DASH 2
#define EV_DIE 4
#define EV_EXIT 8
#define EV_REFILL 16
#define EV_LAND 32
typedef struct { int16_t x,y,vx,vy; uint8_t phase,charge,ground,coyote,buffer,dash,prev,face,wall,lock; uint16_t gems; } State;
extern const char levels[ROOMS][361];
extern const char *names[ROOMS];
extern const uint8_t spawn_y[ROOMS];
uint8_t tile(uint8_t room,int16_t x,int16_t y);
void init_state(State *s,uint8_t room);
uint8_t step(State *s,uint8_t room,uint8_t keys);
#endif
