#ifndef M68000__HEADER
#define M68000__HEADER

#include "cpuintrf.h"

enum
{
	/* NOTE: M68K_SP fetches the current SP, be it USP, ISP, or MSP */
	M68K_PC=1, M68K_SP, M68K_ISP, M68K_USP, M68K_MSP, M68K_SR, M68K_VBR,
	M68K_SFC, M68K_DFC, M68K_CACR, M68K_CAAR, M68K_PREF_ADDR, M68K_PREF_DATA,
	M68K_D0, M68K_D1, M68K_D2, M68K_D3, M68K_D4, M68K_D5, M68K_D6, M68K_D7,
	M68K_A0, M68K_A1, M68K_A2, M68K_A3, M68K_A4, M68K_A5, M68K_A6, M68K_A7
};

enum
{
	CPUINFO_PTR_M68K_RESET_CALLBACK = CPUINFO_PTR_CPU_SPECIFIC,
	CPUINFO_PTR_M68K_CMPILD_CALLBACK,
	CPUINFO_PTR_M68K_RTE_CALLBACK,
	CPUINFO_PTR_M68K_TAS_CALLBACK
};

extern int m68k_ICount;

/* Redirect memory calls */

struct m68k_memory_interface
{
	offs_t		opcode_xor;						// Address Calculation
	UINT8		(*read8)(offs_t);				// Normal read 8 bit
	UINT16	(*read16)(offs_t);				// Normal read 16 bit
	UINT32	(*read32)(offs_t);				// Normal read 32 bit
	void		(*write8)(offs_t, UINT8);		// Write 8 bit
	void		(*write16)(offs_t, UINT16);	// Write 16 bit
	void		(*write32)(offs_t, UINT32);	// Write 32 bit
	void		(*changepc)(offs_t);			// Change PC

    // For Encrypted Stuff

	UINT8		(*read8pc)(offs_t);				// PC Relative read 8 bit
	UINT16	(*read16pc)(offs_t);			// PC Relative read 16 bit
	UINT32	(*read32pc)(offs_t);			// PC Relative read 32 bit

	UINT16	(*read16d)(offs_t);				// Direct read 16 bit
	UINT32	(*read32d)(offs_t);				// Direct read 32 bit
};

struct m68k_encryption_interface
{
	UINT8		(*read8pc)(offs_t);				// PC Relative read 8 bit
	UINT16	(*read16pc)(offs_t);			// PC Relative read 16 bit
	UINT32	(*read32pc)(offs_t);			// PC Relative read 32 bit

	UINT16	(*read16d)(offs_t);				// Direct read 16 bit
	UINT32	(*read32d)(offs_t);				// Direct read 32 bit
};

/* The MAME API for MC68000 */

#define MC68000_IRQ_1    1
#define MC68000_IRQ_2    2
#define MC68000_IRQ_3    3
#define MC68000_IRQ_4    4
#define MC68000_IRQ_5    5
#define MC68000_IRQ_6    6
#define MC68000_IRQ_7    7

#define MC68000_INT_ACK_AUTOVECTOR    -1
#define MC68000_INT_ACK_SPURIOUS      -2

void m68000_get_info(UINT32 state, cpuinfo *info);
extern void m68000_memory_interface_set(int Entry,void * memory_routine);

/****************************************************************************
 * M68008 section
 ****************************************************************************/
#if HAS_M68008
#define MC68008_IRQ_1					MC68000_IRQ_1
#define MC68008_IRQ_2					MC68000_IRQ_2
#define MC68008_IRQ_3					MC68000_IRQ_3
#define MC68008_IRQ_4					MC68000_IRQ_4
#define MC68008_IRQ_5					MC68000_IRQ_5
#define MC68008_IRQ_6					MC68000_IRQ_6
#define MC68008_IRQ_7					MC68000_IRQ_7
#define MC68008_INT_ACK_AUTOVECTOR		MC68000_INT_ACK_AUTOVECTOR
#define MC68008_INT_ACK_SPURIOUS		MC68000_INT_ACK_SPURIOUS

void m68008_get_info(UINT32 state, cpuinfo *info);
#endif

/****************************************************************************
 * M68010 section
 ****************************************************************************/
#if HAS_M68010
#define MC68010_IRQ_1					MC68000_IRQ_1
#define MC68010_IRQ_2					MC68000_IRQ_2
#define MC68010_IRQ_3					MC68000_IRQ_3
#define MC68010_IRQ_4					MC68000_IRQ_4
#define MC68010_IRQ_5					MC68000_IRQ_5
#define MC68010_IRQ_6					MC68000_IRQ_6
#define MC68010_IRQ_7					MC68000_IRQ_7
#define MC68010_INT_ACK_AUTOVECTOR		MC68000_INT_ACK_AUTOVECTOR
#define MC68010_INT_ACK_SPURIOUS		MC68000_INT_ACK_SPURIOUS

void m68010_get_info(UINT32 state, cpuinfo *info);
#endif

/****************************************************************************
 * M68EC020 section
 ****************************************************************************/
#if HAS_M68EC020
#define MC68EC020_IRQ_1					MC68000_IRQ_1
#define MC68EC020_IRQ_2					MC68000_IRQ_2
#define MC68EC020_IRQ_3					MC68000_IRQ_3
#define MC68EC020_IRQ_4					MC68000_IRQ_4
#define MC68EC020_IRQ_5					MC68000_IRQ_5
#define MC68EC020_IRQ_6					MC68000_IRQ_6
#define MC68EC020_IRQ_7					MC68000_IRQ_7
#define MC68EC020_INT_ACK_AUTOVECTOR	MC68000_INT_ACK_AUTOVECTOR
#define MC68EC020_INT_ACK_SPURIOUS		MC68000_INT_ACK_SPURIOUS

void m68ec020_get_info(UINT32 state, cpuinfo *info);
#endif

/****************************************************************************
 * M68020 section
 ****************************************************************************/
#if HAS_M68020
#define MC68020_IRQ_1					MC68000_IRQ_1
#define MC68020_IRQ_2					MC68000_IRQ_2
#define MC68020_IRQ_3					MC68000_IRQ_3
#define MC68020_IRQ_4					MC68000_IRQ_4
#define MC68020_IRQ_5					MC68000_IRQ_5
#define MC68020_IRQ_6					MC68000_IRQ_6
#define MC68020_IRQ_7					MC68000_IRQ_7
#define MC68020_INT_ACK_AUTOVECTOR		MC68000_INT_ACK_AUTOVECTOR
#define MC68020_INT_ACK_SPURIOUS		MC68000_INT_ACK_SPURIOUS

void m68020_get_info(UINT32 state, cpuinfo *info);
#endif

/****************************************************************************
 * M68040 section
 ****************************************************************************/
#if HAS_M68040
#define MC68040_IRQ_1					MC68000_IRQ_1
#define MC68040_IRQ_2					MC68000_IRQ_2
#define MC68040_IRQ_3					MC68000_IRQ_3
#define MC68040_IRQ_4					MC68000_IRQ_4
#define MC68040_IRQ_5					MC68000_IRQ_5
#define MC68040_IRQ_6					MC68000_IRQ_6
#define MC68040_IRQ_7					MC68000_IRQ_7
#define MC68040_INT_ACK_AUTOVECTOR		MC68000_INT_ACK_AUTOVECTOR
#define MC68040_INT_ACK_SPURIOUS		MC68000_INT_ACK_SPURIOUS

void m68040_get_info(UINT32 state, cpuinfo *info);
#endif

// C Core header
#include "m68kmame.h"

#endif /* M68000__HEADER */
