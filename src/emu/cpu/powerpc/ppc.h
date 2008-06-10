/***************************************************************************

    ppc.h
    Interface file for the portable MIPS III/IV emulator.
    Written by Aaron Giles

***************************************************************************/

#ifndef __PPC_H__
#define __PPC_H__

#include "cpuintrf.h"


/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	PPC_PC = 1,
	PPC_R0,
	PPC_R1,
	PPC_R2,
	PPC_R3,
	PPC_R4,
	PPC_R5,
	PPC_R6,
	PPC_R7,
	PPC_R8,
	PPC_R9,
	PPC_R10,
	PPC_R11,
	PPC_R12,
	PPC_R13,
	PPC_R14,
	PPC_R15,
	PPC_R16,
	PPC_R17,
	PPC_R18,
	PPC_R19,
	PPC_R20,
	PPC_R21,
	PPC_R22,
	PPC_R23,
	PPC_R24,
	PPC_R25,
	PPC_R26,
	PPC_R27,
	PPC_R28,
	PPC_R29,
	PPC_R30,
	PPC_R31,
	PPC_CR,
	PPC_LR,
	PPC_CTR,
	PPC_XER,
	PPC_MSR,
	PPC_SRR0,
	PPC_SRR1,
	PPC_SPRG0,
	PPC_SPRG1,
	PPC_SPRG2,
	PPC_SPRG3,
	PPC_SDR1,
	PPC_EXIER,
	PPC_EXISR,
	PPC_EVPR,
	PPC_IOCR,
	PPC_TBL,
	PPC_TBH,
	PPC_DEC
};


#define PPC_MAX_FASTRAM			4
#define PPC_MAX_HOTSPOTS		16


enum
{
	CPUINFO_INT_PPC_DRC_OPTIONS = CPUINFO_INT_CPU_SPECIFIC,

	CPUINFO_INT_PPC_FASTRAM_SELECT,
	CPUINFO_INT_PPC_FASTRAM_START,
	CPUINFO_INT_PPC_FASTRAM_END,
	CPUINFO_INT_PPC_FASTRAM_READONLY,

	CPUINFO_INT_PPC_HOTSPOT_SELECT,
	CPUINFO_INT_PPC_HOTSPOT_PC,
	CPUINFO_INT_PPC_HOTSPOT_OPCODE,
	CPUINFO_INT_PPC_HOTSPOT_CYCLES,

	CPUINFO_INT_PPC_RX_DATA,

	CPUINFO_PTR_PPC_FASTRAM_BASE = CPUINFO_PTR_CPU_SPECIFIC,

	CPUINFO_PTR_SPU_RX_HANDLER,
	CPUINFO_PTR_SPU_TX_HANDLER,

	CPUINFO_PTR_CONTEXT			/* temporary */
};



/***************************************************************************
    INTERRUPT CONSTANTS
***************************************************************************/

#define PPC_IRQ				0		/* external IRQ */
#define PPC_IRQ_LINE_0		0		/* (4XX) external IRQ0 */
#define PPC_IRQ_LINE_1		1		/* (4XX) external IRQ1 */
#define PPC_IRQ_LINE_2		2		/* (4XX) external IRQ2 */
#define PPC_IRQ_LINE_3		3		/* (4XX) external IRQ3 */
#define PPC_IRQ_LINE_4		4		/* (4XX) external IRQ4 */



/***************************************************************************
    STRUCTURES
***************************************************************************/

typedef struct _powerpc_config powerpc_config;
struct _powerpc_config
{
	UINT32		bus_frequency;
};



/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

#if (HAS_PPC403GA)
void ppc403ga_get_info(UINT32 state, cpuinfo *info);
#endif

#if (HAS_PPC403GCX)
void ppc403gcx_get_info(UINT32 state, cpuinfo *info);
#endif

#if (HAS_PPC601)
void ppc601_get_info(UINT32 state, cpuinfo *info);
#endif

#if (HAS_PPC602)
void ppc602_get_info(UINT32 state, cpuinfo *info);
#endif

#if (HAS_PPC603)
void ppc603_get_info(UINT32 state, cpuinfo *info);
#endif

#if (HAS_PPC603E)
void ppc603e_get_info(UINT32 state, cpuinfo *info);
#endif

#if (HAS_PPC603R)
void ppc603r_get_info(UINT32 state, cpuinfo *info);
#endif

#if (HAS_PPC604)
void ppc604_get_info(UINT32 state, cpuinfo *info);
#endif

#if (HAS_MPC8240)
void mpc8240_get_info(UINT32 state, cpuinfo *info);
#endif



/***************************************************************************
    COMPILER-SPECIFIC OPTIONS
***************************************************************************/

#define PPCDRC_STRICT_VERIFY		0x0001			/* verify all instructions */
#define PPCDRC_FLUSH_PC				0x0002			/* flush the PC value before each memory access */
#define PPCDRC_ACCURATE_SINGLES		0x0004			/* do excessive rounding to make single-precision results "accurate" */

#define PPCDRC_COMPATIBLE_OPTIONS	(PPCDRC_STRICT_VERIFY | PPCDRC_FLUSH_PC | PPCDRC_ACCURATE_SINGLES)
#define PPCDRC_FASTEST_OPTIONS		(0)

#endif	/* __PPC_H__ */
