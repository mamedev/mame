/*****************************************************************************
 *
 *   saturn.h
 *   portable saturn emulator interface
 *   (hp calculators)
 *
 *   Copyright (c) 2000 Peter Trauner, all rights reserved.
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

#ifndef _SATURN_H
#define _SATURN_H

#include "cpuintrf.h"

#define SATURN_INT_NONE	0
#define SATURN_INT_IRQ	1
#define SATURN_INT_NMI	2

typedef struct
{
	void (*out)(int);
	int (*in)(void);
	void (*reset)(void);
	void (*config)(int v);
	void (*unconfig)(int v);
	int (*id)(void);
	void (*crc)(int addr, int data);
} SATURN_CONFIG;

#ifdef MAME_DEBUG
unsigned saturn_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
#endif /* MAME_DEBUG */

void saturn_get_info(UINT32 state, cpuinfo *info);

#endif

