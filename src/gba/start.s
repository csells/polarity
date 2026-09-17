.syntax unified
.cpu arm7tdmi
.arm
.section .init,"ax"
.global _start
_start:
 b boot
 .space 188
boot:
 msr cpsr_c, #0xd2
 ldr sp, =0x03007fa0
 msr cpsr_c, #0xdf
 ldr sp, =0x03007e00
 ldr r0, =__iwram_load
 ldr r1, =__iwram_start
 ldr r2, =__iwram_end
4: cmp r1,r2
 ldrlo r3,[r0],#4
 strlo r3,[r1],#4
 blo 4b
 ldr r0, =__data_load
 ldr r1, =__data_start
 ldr r2, =__data_end
1: cmp r1,r2
 ldrlo r3,[r0],#4
 strlo r3,[r1],#4
 blo 1b
 ldr r1, =__bss_start
 ldr r2, =__bss_end
 mov r3,#0
2: cmp r1,r2
 strlo r3,[r1],#4
 blo 2b
 msr cpsr_c, #0x1f
 bl main
3: b 3b
