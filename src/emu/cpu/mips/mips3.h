/***************************************************************************

    mips3.h

    Interface file for the universal machine language-based
    MIPS III/IV emulator.

    Copyright Aaron Giles
    Released for general non-commercial use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

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

#define MIPS3_MAX_FASTRAM		4
#define MIPS3_MAX_HOTSPOTS		16

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

#define MIPS3_IRQ0		0		/* IRQ0 */
#define MIPS3_IRQ1		1		/* IRQ1 */
#define MIPS3_IRQ2		2		/* IRQ2 */
#define MIPS3_IRQ3		3		/* IRQ3 */
#define MIPS3_IRQ4		4		/* IRQ4 */
#define MIPS3_IRQ5		5		/* IRQ5 */



/***************************************************************************
    STRUCTURES
***************************************************************************/

struct mips3_config
{
	size_t		icache;							/* code cache size */
	size_t		dcache;							/* data cache size */
	UINT32		system_clock;					/* system clock rate */
};



/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

void mips3drc_set_options(device_t *device, UINT32 options);
void mips3drc_add_fastram(device_t *device, offs_t start, offs_t end, UINT8 readonly, void *base);
void mips3drc_add_hotspot(device_t *device, offs_t pc, UINT32 opcode, UINT32 cycles);

DECLARE_LEGACY_CPU_DEVICE(VR4300BE, vr4300be);
DECLARE_LEGACY_CPU_DEVICE(VR4300LE, vr4300le);
DECLARE_LEGACY_CPU_DEVICE(VR4310BE, vr4310be);
DECLARE_LEGACY_CPU_DEVICE(VR4310LE, vr4310le);

DECLARE_LEGACY_CPU_DEVICE(R4600BE, r4600be);
DECLARE_LEGACY_CPU_DEVICE(R4600LE, r4600le);

DECLARE_LEGACY_CPU_DEVICE(R4650BE, r4650be);
DECLARE_LEGACY_CPU_DEVICE(R4650LE, r4650le);

DECLARE_LEGACY_CPU_DEVICE(R4700BE, r4700be);
DECLARE_LEGACY_CPU_DEVICE(R4700LE, r4700le);

DECLARE_LEGACY_CPU_DEVICE(R5000BE, r5000be);
DECLARE_LEGACY_CPU_DEVICE(R5000LE, r5000le);

DECLARE_LEGACY_CPU_DEVICE(QED5271BE, qed5271be);
DECLARE_LEGACY_CPU_DEVICE(QED5271LE, qed5271le);

DECLARE_LEGACY_CPU_DEVICE(RM7000BE, rm7000be);
DECLARE_LEGACY_CPU_DEVICE(RM7000LE, rm7000le);



/***************************************************************************
    COMPILER-SPECIFIC OPTIONS
***************************************************************************/

/* fix me -- how do we make this work?? */
#define MIPS3DRC_STRICT_VERIFY		0x0001			/* verify all instructions */
#define MIPS3DRC_STRICT_COP1		0x0002			/* validate all COP1 instructions */
#define MIPS3DRC_STRICT_COP2		0x0004			/* validate all COP2 instructions */
#define MIPS3DRC_FLUSH_PC			0x0008			/* flush the PC value before each memory access */
#define MIPS3DRC_CHECK_OVERFLOWS	0x0010			/* actually check overflows on add/sub instructions */

#define MIPS3DRC_COMPATIBLE_OPTIONS	(MIPS3DRC_STRICT_VERIFY | MIPS3DRC_STRICT_COP1 | MIPS3DRC_STRICT_COP2 | MIPS3DRC_FLUSH_PC)
#define MIPS3DRC_FASTEST_OPTIONS	(0)



#endif /* __MIPS3_H__ */
