#ifndef CITY_H
#define CITY_H
#include "engine.h"
#define SAVE_SIZE 32
typedef struct { uint8_t progress[6],checkpoint[6],relays[6],letters[3],selected; } City;
extern const uint8_t city_x[6],city_y[6];
uint8_t city_neighbor(uint8_t location,uint8_t direction);
uint8_t city_unlocked(const City *c,uint8_t location);
uint8_t city_restored(const City *c);
uint8_t city_has_letter(const City *c,uint8_t room);
void city_take_letter(City *c,uint8_t room);
void city_encode(const City *c,uint8_t *bytes);
uint8_t city_decode(City *c,const uint8_t *bytes);
#endif
