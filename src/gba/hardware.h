#ifndef GBA_H
#define GBA_H
#include <stdint.h>
#define REG16(a) (*(volatile uint16_t *)(0x04000000+(a)))
#define REG32(a) (*(volatile uint32_t *)(0x04000000+(a)))
#define RGB(r,g,b) ((r)|((g)<<5)|((b)<<10))
#define BG_PALETTE ((volatile uint16_t *)0x05000000)
#define OBJ_PALETTE ((volatile uint16_t *)0x05000200)
#define VRAM ((volatile uint16_t *)0x06000000)
#define MAP(n) ((volatile uint16_t *)(0x06000000+(n)*2048))
#define KEY_A 1
#define KEY_B 2
#define KEY_SELECT 4
#define KEY_START 8
#define KEY_RIGHT 16
#define KEY_LEFT 32
#define KEY_UP 64
#define KEY_DOWN 128
#define KEY_R 256
#define KEY_L 512
typedef struct {uint16_t a,b,c,pad;} Obj;
extern Obj objects[128];
void copy16(volatile uint16_t *dst,const void *src,unsigned count);
void audio_init(void);
void audio_tick(void);
void audio_region(unsigned region);
void audio_irq(void);
void audio_fx(unsigned event,int x);
#endif
