/*****************************************************************************

    h6280.h Portable Hu6280 emulator interface

    Copyright Bryan McPhail, mish@tendril.co.uk

    This source code is based (with permission!) on the 6502 emulator by
    Juergen Buchmueller.  It is released as part of the Mame emulator project.
    Let me know if you intend to use this code in any other project.

******************************************************************************/

#pragma once

#ifndef __H6280_H__
#define __H6280_H__

#include "cpuintrf.h"

enum
{
	H6280_PC=1, H6280_S, H6280_P, H6280_A, H6280_X, H6280_Y,
	H6280_IRQ_MASK, H6280_TIMER_STATE,
	H6280_NMI_STATE, H6280_IRQ1_STATE, H6280_IRQ2_STATE, H6280_IRQT_STATE,
	H6280_M1, H6280_M2, H6280_M3, H6280_M4,
	H6280_M5, H6280_M6, H6280_M7, H6280_M8
};

#define LAZY_FLAGS  0

#define H6280_RESET_VEC	0xfffe
#define H6280_NMI_VEC	0xfffc
#define H6280_TIMER_VEC	0xfffa
#define H6280_IRQ1_VEC	0xfff8
#define H6280_IRQ2_VEC	0xfff6			/* Aka BRK vector */

CPU_GET_INFO( h6280 );


READ8_HANDLER( h6280_irq_status_r );
WRITE8_HANDLER( h6280_irq_status_w );

READ8_HANDLER( h6280_timer_r );
WRITE8_HANDLER( h6280_timer_w );

/* functions for use by the PSG and joypad port only! */
UINT8 h6280io_get_buffer(void);
void h6280io_set_buffer(UINT8);

CPU_DISASSEMBLE( h6280 );

#endif /* __H6280_H__ */
