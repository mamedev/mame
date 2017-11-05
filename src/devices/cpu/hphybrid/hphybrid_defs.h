#ifndef MAME_CPU_HPHYBRID_HPHYBRID_DEFS_H
#define MAME_CPU_HPHYBRID_HPHYBRID_DEFS_H

// Addresses of memory mapped registers
#define HP_REG_A_ADDR       0x0000
#define HP_REG_B_ADDR       0x0001
#define HP_REG_P_ADDR       0x0002
#define HP_REG_R_ADDR       0x0003
#define HP_REG_R4_ADDR      0x0004
#define HP_REG_R5_ADDR      0x0005
#define HP_REG_R6_ADDR      0x0006
#define HP_REG_R7_ADDR      0x0007
#define HP_REG_IV_ADDR      0x0008
#define HP_REG_PA_ADDR      0x0009
#define HP_REG_W_ADDR       0x000A
#define HP_REG_DMAPA_ADDR   0x000B
#define HP_REG_DMAMA_ADDR   0x000C
#define HP_REG_DMAC_ADDR    0x000D
#define HP_REG_C_ADDR       0x000e
#define HP_REG_D_ADDR       0x000f
#define HP_REG_AR2_ADDR     0x0010
#define HP_REG_SE_ADDR      0x0014
#define HP_REG_R25_ADDR     0x0015
#define HP_REG_R26_ADDR     0x0016
#define HP_REG_R27_ADDR     0x0017
#define HP_REG_R32_ADDR     0x001a
#define HP_REG_R33_ADDR     0x001b
#define HP_REG_R34_ADDR     0x001c
#define HP_REG_R35_ADDR     0x001d
#define HP_REG_R36_ADDR     0x001e
#define HP_REG_R37_ADDR     0x001f
#define HP_REG_LAST_ADDR    0x001f
#define HP_REG_AR1_ADDR     0xfff8

#define HP_REG_IV_MASK      0xfff0
#define HP_REG_PA_MASK      0x000f

#endif // MAME_CPU_HPHYBRID_HPHYBRID_DEFS_H
