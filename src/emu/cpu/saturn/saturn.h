/*****************************************************************************
 *
 *   cpustate->h
 *   portable saturn emulator interface
 *   (hp calculators)
 *
 *   Copyright Peter Trauner, all rights reserved.
 *
 *   Modified by Antoine Mine'
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     peter.trauner@jk.uni-linz.ac.at
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************/
/*
Calculator        Release Date          Chip Version     Analog/Digital IC
HP71B (early)     02/01/84              1LF2              -
HP71B (later)     ??/??/??              1LK7              -
HP18C             06/01/86              1LK7              -
HP28C             01/05/87              1LK7              -
HP17B             01/04/88              1LT8             Lewis
HP19B             01/04/88              1LT8             Lewis
HP27S             01/04/88              1LT8             Lewis
HP28S             01/04/88              1LT8             Lewis
HP48SX            03/16/91              1LT8             Clarke
HP48S             04/02/91              1LT8             Clarke
HP48GX            06/01/93              1LT8             Yorke
HP48G             06/01/93              1LT8             Yorke
HP38G             09/??/95              1LT8             Yorke
*/
/* 4 bit processor
   20 address lines */

#pragma once

#ifndef __SATURN_H__
#define _SATURN_H


#define SATURN_INT_NONE	0
#define SATURN_INT_IRQ	1
#define SATURN_INT_NMI	2

struct saturn_cpu_core
{
	void (*out)(device_t*,int);
	int (*in)(device_t*);
	void (*reset)(device_t*);
	void (*config)(device_t*,int v);
	void (*unconfig)(device_t*,int v);
	int (*id)(device_t*);
	void (*crc)(device_t*,int addr, int data);
	void (*rsi)(device_t*);
};

enum
{
	SATURN_A=1, SATURN_B, SATURN_C, SATURN_D,
	SATURN_R0, SATURN_R1, SATURN_R2, SATURN_R3, SATURN_R4,
	SATURN_RSTK0, SATURN_RSTK1, SATURN_RSTK2, SATURN_RSTK3,
	SATURN_RSTK4, SATURN_RSTK5, SATURN_RSTK6, SATURN_RSTK7,
	SATURN_PC, SATURN_D0, SATURN_D1,

	SATURN_P,
	SATURN_OUT,
	SATURN_CARRY,
	SATURN_ST,
	SATURN_HST,

	SATURN_IRQ_STATE,
	SATURN_SLEEPING,
};

#define SATURN_IRQ_LINE 0
#define SATURN_NMI_LINE 1
#define SATURN_WAKEUP_LINE 2

CPU_DISASSEMBLE( saturn );

DECLARE_LEGACY_CPU_DEVICE(SATURN, saturn);

#endif /* __SATURN_H__ */
