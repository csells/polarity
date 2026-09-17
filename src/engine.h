#ifndef ENGINE_H
#define ENGINE_H
#include <stdint.h>
#define LEFT 1
#define RIGHT 2
#define UP 4
#define DOWN 8
#define JUMP 16
#define DASH 32
#define LOCATIONS 6
#define ROOMS 18
#define ROOM_W 80
#define ROOM_H 18
#define EV_JUMP 1
#define EV_DASH 2
#define EV_DIE 4
#define EV_EXIT 8
#define EV_REFILL 16
#define EV_LAND 32
#define EV_CHECKPOINT 64
#define EV_SPRING 128
typedef struct { int16_t x,y,vx,vy; uint8_t phase,charge,ground,coyote,buffer,dash,prev,face,wall,lock; uint16_t defeated; uint8_t clock,checkpoint,boost; } State;
typedef struct { uint16_t x; uint8_t y; } Enemy;
extern uint8_t world[ROOM_W*ROOM_H],enemy_count;
extern Enemy enemies[8];
extern const uint8_t * const levels[ROOMS];
extern const char * const names[ROOMS];
extern const char * const locations[LOCATIONS];
void load_room(uint8_t room);
uint8_t tile(uint8_t room,int16_t x,int16_t y);
int16_t enemy_x(uint8_t n,uint8_t clock);
void init_state(State *s,uint8_t room);
void respawn(State *s,uint8_t room);
uint8_t step(State *s,uint8_t room,uint8_t keys);
#endif
