/***************************************************************************

    ADSP2100.h
    Interface file for the portable Analog ADSP-2100 emulator.
    Written by Aaron Giles

***************************************************************************/

#ifndef _ADSP2100_H
#define _ADSP2100_H

#include "cpuintrf.h"


/***************************************************************************
    GLOBAL CONSTANTS
***************************************************************************/

/* transmit and receive data callbacks types */
typedef INT32 (*RX_CALLBACK)(int port);
typedef void  (*TX_CALLBACK)(int port, INT32 data);
typedef void  (*ADSP2100_TIMER_CALLBACK)(int enable);


/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	ADSP2100_PC=1,
	ADSP2100_AX0, ADSP2100_AX1, ADSP2100_AY0, ADSP2100_AY1, ADSP2100_AR, ADSP2100_AF,
	ADSP2100_MX0, ADSP2100_MX1, ADSP2100_MY0, ADSP2100_MY1, ADSP2100_MR0, ADSP2100_MR1, ADSP2100_MR2, ADSP2100_MF,
	ADSP2100_SI, ADSP2100_SE, ADSP2100_SB, ADSP2100_SR0, ADSP2100_SR1,
	ADSP2100_I0, ADSP2100_I1, ADSP2100_I2, ADSP2100_I3, ADSP2100_I4, ADSP2100_I5, ADSP2100_I6, ADSP2100_I7,
	ADSP2100_L0, ADSP2100_L1, ADSP2100_L2, ADSP2100_L3, ADSP2100_L4, ADSP2100_L5, ADSP2100_L6, ADSP2100_L7,
	ADSP2100_M0, ADSP2100_M1, ADSP2100_M2, ADSP2100_M3, ADSP2100_M4, ADSP2100_M5, ADSP2100_M6, ADSP2100_M7,
	ADSP2100_PX, ADSP2100_CNTR, ADSP2100_ASTAT, ADSP2100_SSTAT, ADSP2100_MSTAT,
	ADSP2100_PCSP, ADSP2100_CNTRSP, ADSP2100_STATSP, ADSP2100_LOOPSP,
	ADSP2100_IMASK, ADSP2100_ICNTL, ADSP2100_IRQSTATE0, ADSP2100_IRQSTATE1, ADSP2100_IRQSTATE2, ADSP2100_IRQSTATE3,
	ADSP2100_FLAGIN, ADSP2100_FLAGOUT, ADSP2100_FL0, ADSP2100_FL1, ADSP2100_FL2,
	ADSP2100_AX0_SEC, ADSP2100_AX1_SEC, ADSP2100_AY0_SEC, ADSP2100_AY1_SEC, ADSP2100_AR_SEC, ADSP2100_AF_SEC,
	ADSP2100_MX0_SEC, ADSP2100_MX1_SEC, ADSP2100_MY0_SEC, ADSP2100_MY1_SEC, ADSP2100_MR0_SEC, ADSP2100_MR1_SEC, ADSP2100_MR2_SEC, ADSP2100_MF_SEC,
	ADSP2100_SI_SEC, ADSP2100_SE_SEC, ADSP2100_SB_SEC, ADSP2100_SR0_SEC, ADSP2100_SR1_SEC
};

enum
{
	CPUINFO_PTR_ADSP2100_RX_HANDLER = CPUINFO_PTR_CPU_SPECIFIC,
	CPUINFO_PTR_ADSP2100_TX_HANDLER,
	CPUINFO_PTR_ADSP2100_TIMER_HANDLER
};


/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

#define ADSP2100_IRQ0		0		/* IRQ0 */
#define ADSP2100_SPORT1_RX	0		/* SPORT1 receive IRQ */
#define ADSP2100_IRQ1		1		/* IRQ1 */
#define ADSP2100_SPORT1_TX	1		/* SPORT1 transmit IRQ */
#define ADSP2100_IRQ2		2		/* IRQ2 */
#define ADSP2100_IRQ3		3		/* IRQ3 */

#if (HAS_ADSP2100)
extern void adsp2100_get_info(UINT32 state, cpuinfo *info);
#endif

/**************************************************************************
 * ADSP2101 section
 **************************************************************************/

#define ADSP2101_IRQ0		0		/* IRQ0 */
#define ADSP2101_SPORT1_RX	0		/* SPORT1 receive IRQ */
#define ADSP2101_IRQ1		1		/* IRQ1 */
#define ADSP2101_SPORT1_TX	1		/* SPORT1 transmit IRQ */
#define ADSP2101_IRQ2		2		/* IRQ2 */
#define ADSP2101_SPORT0_RX	3		/* SPORT0 receive IRQ */
#define ADSP2101_SPORT0_TX	4		/* SPORT0 transmit IRQ */
#define ADSP2101_TIMER		5		/* internal timer IRQ */

#if (HAS_ADSP2101)
extern void adsp2101_get_info(UINT32 state, cpuinfo *info);
#endif

#if (HAS_ADSP2104)
/**************************************************************************
 * ADSP2104 section
 **************************************************************************/

#define ADSP2104_IRQ0		0		/* IRQ0 */
#define ADSP2104_SPORT1_RX	0		/* SPORT1 receive IRQ */
#define ADSP2104_IRQ1		1		/* IRQ1 */
#define ADSP2104_SPORT1_TX	1		/* SPORT1 transmit IRQ */
#define ADSP2104_IRQ2		2		/* IRQ2 */
#define ADSP2104_SPORT0_RX	3		/* SPORT0 receive IRQ */
#define ADSP2104_SPORT0_TX	4		/* SPORT0 transmit IRQ */
#define ADSP2104_TIMER		5		/* internal timer IRQ */

extern void adsp2104_get_info(UINT32 state, cpuinfo *info);
extern void adsp2104_load_boot_data(UINT8 *srcdata, UINT32 *dstdata);
#endif

#if (HAS_ADSP2105)
/**************************************************************************
 * ADSP2105 section
 **************************************************************************/

#define ADSP2105_IRQ0		0		/* IRQ0 */
#define ADSP2105_SPORT1_RX	0		/* SPORT1 receive IRQ */
#define ADSP2105_IRQ1		1		/* IRQ1 */
#define ADSP2105_SPORT1_TX	1		/* SPORT1 transmit IRQ */
#define ADSP2105_IRQ2		2		/* IRQ2 */
#define ADSP2105_TIMER		5		/* internal timer IRQ */

extern void adsp2105_get_info(UINT32 state, cpuinfo *info);
extern void adsp2105_load_boot_data(UINT8 *srcdata, UINT32 *dstdata);
#endif

#if (HAS_ADSP2115)
/**************************************************************************
 * ADSP2115 section
 **************************************************************************/

#define ADSP2115_IRQ0		0		/* IRQ0 */
#define ADSP2115_SPORT1_RX	0		/* SPORT1 receive IRQ */
#define ADSP2115_IRQ1		1		/* IRQ1 */
#define ADSP2115_SPORT1_TX	1		/* SPORT1 transmit IRQ */
#define ADSP2115_IRQ2		2		/* IRQ2 */
#define ADSP2115_SPORT0_RX	3		/* SPORT0 receive IRQ */
#define ADSP2115_SPORT0_TX	4		/* SPORT0 transmit IRQ */
#define ADSP2115_TIMER		5		/* internal timer IRQ */

extern void adsp2115_get_info(UINT32 state, cpuinfo *info);
extern void adsp2115_load_boot_data(UINT8 *srcdata, UINT32 *dstdata);
#endif

#if (HAS_ADSP2181)
/**************************************************************************
 * ADSP2181 section
 **************************************************************************/

#define ADSP2181_IRQ0		0		/* IRQ0 */
#define ADSP2181_SPORT1_RX	0		/* SPORT1 receive IRQ */
#define ADSP2181_IRQ1		1		/* IRQ1 */
#define ADSP2181_SPORT1_TX	1		/* SPORT1 transmit IRQ */
#define ADSP2181_IRQ2		2		/* IRQ2 */
#define ADSP2181_SPORT0_RX	3		/* SPORT0 receive IRQ */
#define ADSP2181_SPORT0_TX	4		/* SPORT0 transmit IRQ */
#define ADSP2181_TIMER		5		/* internal timer IRQ */
#define ADSP2181_IRQE		6		/* IRQE */
#define ADSP2181_IRQL1		7		/* IRQL1 */
#define ADSP2181_IRQL2		8		/* IRQL2 */

extern void adsp2181_get_info(UINT32 state, cpuinfo *info);
extern void adsp2181_load_boot_data(UINT8 *srcdata, UINT32 *dstdata);
extern void adsp2181_idma_addr_w(UINT16 data);
extern UINT16 adsp2181_idma_addr_r(void);
extern void adsp2181_idma_data_w(UINT16 data);
extern UINT16 adsp2181_idma_data_r(void);
#endif

#endif /* _ADSP2100_H */
