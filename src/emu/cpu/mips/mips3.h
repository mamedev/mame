// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    mips3.h

    Interface file for the universal machine language-based
    MIPS III/IV emulator.

***************************************************************************/

#pragma once

#ifndef __MIPS3_H__
#define __MIPS3_H__



/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	MIPS3_PC = 1,
	MIPS3_R0,
	MIPS3_R1,
	MIPS3_R2,
	MIPS3_R3,
	MIPS3_R4,
	MIPS3_R5,
	MIPS3_R6,
	MIPS3_R7,
	MIPS3_R8,
	MIPS3_R9,
	MIPS3_R10,
	MIPS3_R11,
	MIPS3_R12,
	MIPS3_R13,
	MIPS3_R14,
	MIPS3_R15,
	MIPS3_R16,
	MIPS3_R17,
	MIPS3_R18,
	MIPS3_R19,
	MIPS3_R20,
	MIPS3_R21,
	MIPS3_R22,
	MIPS3_R23,
	MIPS3_R24,
	MIPS3_R25,
	MIPS3_R26,
	MIPS3_R27,
	MIPS3_R28,
	MIPS3_R29,
	MIPS3_R30,
	MIPS3_R31,
	MIPS3_HI,
	MIPS3_LO,
	MIPS3_FPR0,
	MIPS3_FPS0,
	MIPS3_FPD0,
	MIPS3_FPR1,
	MIPS3_FPS1,
	MIPS3_FPD1,
	MIPS3_FPR2,
	MIPS3_FPS2,
	MIPS3_FPD2,
	MIPS3_FPR3,
	MIPS3_FPS3,
	MIPS3_FPD3,
	MIPS3_FPR4,
	MIPS3_FPS4,
	MIPS3_FPD4,
	MIPS3_FPR5,
	MIPS3_FPS5,
	MIPS3_FPD5,
	MIPS3_FPR6,
	MIPS3_FPS6,
	MIPS3_FPD6,
	MIPS3_FPR7,
	MIPS3_FPS7,
	MIPS3_FPD7,
	MIPS3_FPR8,
	MIPS3_FPS8,
	MIPS3_FPD8,
	MIPS3_FPR9,
	MIPS3_FPS9,
	MIPS3_FPD9,
	MIPS3_FPR10,
	MIPS3_FPS10,
	MIPS3_FPD10,
	MIPS3_FPR11,
	MIPS3_FPS11,
	MIPS3_FPD11,
	MIPS3_FPR12,
	MIPS3_FPS12,
	MIPS3_FPD12,
	MIPS3_FPR13,
	MIPS3_FPS13,
	MIPS3_FPD13,
	MIPS3_FPR14,
	MIPS3_FPS14,
	MIPS3_FPD14,
	MIPS3_FPR15,
	MIPS3_FPS15,
	MIPS3_FPD15,
	MIPS3_FPR16,
	MIPS3_FPS16,
	MIPS3_FPD16,
	MIPS3_FPR17,
	MIPS3_FPS17,
	MIPS3_FPD17,
	MIPS3_FPR18,
	MIPS3_FPS18,
	MIPS3_FPD18,
	MIPS3_FPR19,
	MIPS3_FPS19,
	MIPS3_FPD19,
	MIPS3_FPR20,
	MIPS3_FPS20,
	MIPS3_FPD20,
	MIPS3_FPR21,
	MIPS3_FPS21,
	MIPS3_FPD21,
	MIPS3_FPR22,
	MIPS3_FPS22,
	MIPS3_FPD22,
	MIPS3_FPR23,
	MIPS3_FPS23,
	MIPS3_FPD23,
	MIPS3_FPR24,
	MIPS3_FPS24,
	MIPS3_FPD24,
	MIPS3_FPR25,
	MIPS3_FPS25,
	MIPS3_FPD25,
	MIPS3_FPR26,
	MIPS3_FPS26,
	MIPS3_FPD26,
	MIPS3_FPR27,
	MIPS3_FPS27,
	MIPS3_FPD27,
	MIPS3_FPR28,
	MIPS3_FPS28,
	MIPS3_FPD28,
	MIPS3_FPR29,
	MIPS3_FPS29,
	MIPS3_FPD29,
	MIPS3_FPR30,
	MIPS3_FPS30,
	MIPS3_FPD30,
	MIPS3_FPR31,
	MIPS3_FPS31,
	MIPS3_FPD31,
	MIPS3_CCR1_31,
	MIPS3_SR,
	MIPS3_EPC,
	MIPS3_CAUSE,
	MIPS3_COUNT,
	MIPS3_COMPARE,
	MIPS3_INDEX,
	MIPS3_RANDOM,
	MIPS3_ENTRYHI,
	MIPS3_ENTRYLO0,
	MIPS3_ENTRYLO1,
	MIPS3_PAGEMASK,
	MIPS3_WIRED,
	MIPS3_BADVADDR
};

#define MIPS3_MAX_FASTRAM       4
#define MIPS3_MAX_HOTSPOTS      16

enum
{
	CPUINFO_INT_MIPS3_DRC_OPTIONS = CPUINFO_INT_CPU_SPECIFIC,

	CPUINFO_INT_MIPS3_FASTRAM_SELECT,
	CPUINFO_INT_MIPS3_FASTRAM_START,
	CPUINFO_INT_MIPS3_FASTRAM_END,
	CPUINFO_INT_MIPS3_FASTRAM_READONLY,

	CPUINFO_INT_MIPS3_HOTSPOT_SELECT,
	CPUINFO_INT_MIPS3_HOTSPOT_PC,
	CPUINFO_INT_MIPS3_HOTSPOT_OPCODE,
	CPUINFO_INT_MIPS3_HOTSPOT_CYCLES,

	CPUINFO_PTR_MIPS3_FASTRAM_BASE = CPUINFO_PTR_CPU_SPECIFIC
};



/***************************************************************************
    INTERRUPT CONSTANTS
***************************************************************************/

#define MIPS3_IRQ0      0       /* IRQ0 */
#define MIPS3_IRQ1      1       /* IRQ1 */
#define MIPS3_IRQ2      2       /* IRQ2 */
#define MIPS3_IRQ3      3       /* IRQ3 */
#define MIPS3_IRQ4      4       /* IRQ4 */
#define MIPS3_IRQ5      5       /* IRQ5 */



/***************************************************************************
    STRUCTURES
***************************************************************************/

struct mips3_config
{
	size_t      icache;                         /* code cache size */
	size_t      dcache;                         /* data cache size */
	UINT32      system_clock;                   /* system clock rate */
};



/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

void mips3drc_set_options(device_t *device, UINT32 options);
void mips3drc_add_fastram(device_t *device, offs_t start, offs_t end, UINT8 readonly, void *base);
void mips3drc_add_hotspot(device_t *device, offs_t pc, UINT32 opcode, UINT32 cycles);

DECLARE_LEGACY_CPU_DEVICE(VR4300BE_INT, vr4300be_int);
DECLARE_LEGACY_CPU_DEVICE(VR4300LE_INT, vr4300le_int);
DECLARE_LEGACY_CPU_DEVICE(VR4310BE_INT, vr4310be_int);
DECLARE_LEGACY_CPU_DEVICE(VR4310LE_INT, vr4310le_int);

DECLARE_LEGACY_CPU_DEVICE(R4600BE_INT, r4600be_int);
DECLARE_LEGACY_CPU_DEVICE(R4600LE_INT, r4600le_int);

DECLARE_LEGACY_CPU_DEVICE(R4650BE_INT, r4650be_int);
DECLARE_LEGACY_CPU_DEVICE(R4650LE_INT, r4650le_int);

DECLARE_LEGACY_CPU_DEVICE(R4700BE_INT, r4700be_int);
DECLARE_LEGACY_CPU_DEVICE(R4700LE_INT, r4700le_int);

DECLARE_LEGACY_CPU_DEVICE(R5000BE_INT, r5000be_int);
DECLARE_LEGACY_CPU_DEVICE(R5000LE_INT, r5000le_int);

DECLARE_LEGACY_CPU_DEVICE(QED5271BE_INT, qed5271be_int);
DECLARE_LEGACY_CPU_DEVICE(QED5271LE_INT, qed5271le_int);

DECLARE_LEGACY_CPU_DEVICE(RM7000BE_INT, rm7000be_int);
DECLARE_LEGACY_CPU_DEVICE(RM7000LE_INT, rm7000le_int);

DECLARE_LEGACY_CPU_DEVICE(VR4300BE_DRC, vr4300be_drc);
DECLARE_LEGACY_CPU_DEVICE(VR4300LE_DRC, vr4300le_drc);
DECLARE_LEGACY_CPU_DEVICE(VR4310BE_DRC, vr4310be_drc);
DECLARE_LEGACY_CPU_DEVICE(VR4310LE_DRC, vr4310le_drc);

DECLARE_LEGACY_CPU_DEVICE(R4600BE_DRC, r4600be_drc);
DECLARE_LEGACY_CPU_DEVICE(R4600LE_DRC, r4600le_drc);

DECLARE_LEGACY_CPU_DEVICE(R4650BE_DRC, r4650be_drc);
DECLARE_LEGACY_CPU_DEVICE(R4650LE_DRC, r4650le_drc);

DECLARE_LEGACY_CPU_DEVICE(R4700BE_DRC, r4700be_drc);
DECLARE_LEGACY_CPU_DEVICE(R4700LE_DRC, r4700le_drc);

DECLARE_LEGACY_CPU_DEVICE(R5000BE_DRC, r5000be_drc);
DECLARE_LEGACY_CPU_DEVICE(R5000LE_DRC, r5000le_drc);

DECLARE_LEGACY_CPU_DEVICE(QED5271BE_DRC, qed5271be_drc);
DECLARE_LEGACY_CPU_DEVICE(QED5271LE_DRC, qed5271le_drc);

DECLARE_LEGACY_CPU_DEVICE(RM7000BE_DRC, rm7000be_drc);
DECLARE_LEGACY_CPU_DEVICE(RM7000LE_DRC, rm7000le_drc);

extern const device_type VR4300BE;
extern const device_type VR4300LE;
extern const device_type VR4310BE;
extern const device_type VR4310LE;
extern const device_type R4600BE;
extern const device_type R4600LE;
extern const device_type R4650BE;
extern const device_type R4650LE;
extern const device_type R4700BE;
extern const device_type R4700LE;
extern const device_type R5000BE;
extern const device_type R5000LE;
extern const device_type QED5271BE;
extern const device_type QED5271LE;
extern const device_type RM7000BE;
extern const device_type RM7000LE;


/***************************************************************************
    COMPILER-SPECIFIC OPTIONS
***************************************************************************/

/* fix me -- how do we make this work?? */
#define MIPS3DRC_STRICT_VERIFY      0x0001          /* verify all instructions */
#define MIPS3DRC_STRICT_COP1        0x0002          /* validate all COP1 instructions */
#define MIPS3DRC_STRICT_COP2        0x0004          /* validate all COP2 instructions */
#define MIPS3DRC_FLUSH_PC           0x0008          /* flush the PC value before each memory access */
#define MIPS3DRC_CHECK_OVERFLOWS    0x0010          /* actually check overflows on add/sub instructions */

#define MIPS3DRC_COMPATIBLE_OPTIONS (MIPS3DRC_STRICT_VERIFY | MIPS3DRC_STRICT_COP1 | MIPS3DRC_STRICT_COP2 | MIPS3DRC_FLUSH_PC)
#define MIPS3DRC_FASTEST_OPTIONS    (0)



#endif /* __MIPS3_H__ */
