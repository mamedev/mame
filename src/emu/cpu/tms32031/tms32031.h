/***************************************************************************

    tms32031.h
    Interface file for the portable TMS32C031 emulator.
    Written by Aaron Giles

***************************************************************************/

#pragma once

#ifndef __TMS32031_H__
#define __TMS32031_H__


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*tms32031_xf_func)(device_t *device, UINT8 val);
typedef void (*tms32031_iack_func)(device_t *device, UINT8 val, offs_t address);


typedef struct _tms32031_config tms32031_config;
struct _tms32031_config
{
	UINT32				bootoffset;
	tms32031_xf_func	xf0_w;
	tms32031_xf_func	xf1_w;
	tms32031_iack_func	iack_w;
};


/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	TMS32031_PC=1,
	TMS32031_R0,TMS32031_R1,TMS32031_R2,TMS32031_R3,
	TMS32031_R4,TMS32031_R5,TMS32031_R6,TMS32031_R7,
	TMS32031_R0F,TMS32031_R1F,TMS32031_R2F,TMS32031_R3F,
	TMS32031_R4F,TMS32031_R5F,TMS32031_R6F,TMS32031_R7F,
	TMS32031_AR0,TMS32031_AR1,TMS32031_AR2,TMS32031_AR3,
	TMS32031_AR4,TMS32031_AR5,TMS32031_AR6,TMS32031_AR7,
	TMS32031_DP,TMS32031_IR0,TMS32031_IR1,TMS32031_BK,
	TMS32031_SP,TMS32031_ST,TMS32031_IE,TMS32031_IF,
	TMS32031_IOF,TMS32031_RS,TMS32031_RE,TMS32031_RC
};


/***************************************************************************
    INTERRUPT CONSTANTS
***************************************************************************/

#define TMS32031_IRQ0		0		/* IRQ0 */
#define TMS32031_IRQ1		1		/* IRQ1 */
#define TMS32031_IRQ2		2		/* IRQ2 */
#define TMS32031_IRQ3		3		/* IRQ3 */
#define TMS32031_XINT0		4		/* serial 0 transmit interrupt */
#define TMS32031_RINT0		5		/* serial 0 receive interrupt */
#define TMS32031_XINT1		6		/* serial 1 transmit interrupt */
#define TMS32031_RINT1		7		/* serial 1 receive interrupt */
#define TMS32031_TINT0		8		/* timer 0 interrupt */
#define TMS32031_TINT1		9		/* timer 1 interrupt */
#define TMS32031_DINT		10		/* DMA interrupt */
#define TMS32031_DINT0		10		/* DMA 0 interrupt (32032 only) */
#define TMS32031_DINT1		11		/* DMA 1 interrupt (32032 only) */


/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

DECLARE_LEGACY_CPU_DEVICE(TMS32031, tms32031);
DECLARE_LEGACY_CPU_DEVICE(TMS32032, tms32032);

extern float convert_tms3203x_fp_to_float(UINT32 floatdata);
extern double convert_tms3203x_fp_to_double(UINT32 floatdata);
extern UINT32 convert_float_to_tms3203x_fp(float fval);
extern UINT32 convert_double_to_tms3203x_fp(double dval);

#endif /* __TMS32031_H__ */
