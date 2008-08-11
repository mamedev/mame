/***************************************************************************

 h8priv.h : Private constants and other definitions for the H8/3002 emulator.

****************************************************************************/

#pragma once

#ifndef __H8PRIV_H__
#define __H8PRIV_H__

typedef struct _h83002_state h83002_state;
struct _h83002_state
{
	// main CPU stuff
	UINT32 h8err;
	UINT32 regs[8];
	UINT32 pc, ppc;
	UINT32 h8_IRQrequestH, h8_IRQrequestL;

	UINT8  ccr;
	UINT8  h8nflag, h8vflag, h8cflag, h8zflag, h8iflag, h8hflag;
	UINT8  h8uflag, h8uiflag;

	int (*irq_cb)(int);

	// H8/3002 onboard peripherals stuff

	UINT8 per_regs[256];

	UINT16 h8TCNT0, h8TCNT1, h8TCNT2, h8TCNT3, h8TCNT4;
	UINT8 h8TSTR;

	emu_timer *timer[5];

	int cpu_number;

};

extern h83002_state h8;

UINT8 h8_register_read8(UINT32 address);
UINT8 h8_3007_register_read8(UINT32 address);
UINT8 h8_3007_register1_read8(UINT32 address);
void h8_register_write8(UINT32 address, UINT8 val);
void h8_3007_register_write8(UINT32 address, UINT8 val);
void h8_3007_register1_write8(UINT32 address, UINT8 val);

void h8_itu_init(void);
void h8_3007_itu_init(void);
void h8_itu_reset(void);
UINT8 h8_itu_read8(UINT8 reg);
UINT8 h8_3007_itu_read8(UINT8 reg);
void h8_itu_write8(UINT8 reg, UINT8 val);
void h8_3007_itu_write8(UINT8 reg, UINT8 val);

#endif /* __H8PRIV_H__ */
