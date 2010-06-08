/*****************************************************************************
 *
 *  POKEY chip emulator 4.3
 *  Copyright Nicola Salmoria and the MAME Team
 *
 *  Based on original info found in Ron Fries' Pokey emulator,
 *  with additions by Brad Oliver, Eric Smith and Juergen Buchmueller.
 *  paddle (a/d conversion) details from the Atari 400/800 Hardware Manual.
 *  Polynome algorithms according to info supplied by Perry McFarlane.
 *
 *  This code is subject to the MAME license, which besides other
 *  things means it is distributed as is, no warranties whatsoever.
 *  For more details read mame.txt that comes with MAME.
 *
 *****************************************************************************/

#pragma once

#ifndef __POKEY_H__
#define __POKEY_H__

#include "devlegcy.h"

/* CONSTANT DEFINITIONS */

/* POKEY WRITE LOGICALS */
#define AUDF1_C     0x00
#define AUDC1_C     0x01
#define AUDF2_C     0x02
#define AUDC2_C     0x03
#define AUDF3_C     0x04
#define AUDC3_C     0x05
#define AUDF4_C     0x06
#define AUDC4_C     0x07
#define AUDCTL_C    0x08
#define STIMER_C    0x09
#define SKREST_C    0x0A
#define POTGO_C     0x0B
#define SEROUT_C    0x0D
#define IRQEN_C     0x0E
#define SKCTL_C     0x0F

/* POKEY READ LOGICALS */
#define POT0_C      0x00
#define POT1_C      0x01
#define POT2_C      0x02
#define POT3_C      0x03
#define POT4_C      0x04
#define POT5_C      0x05
#define POT6_C      0x06
#define POT7_C      0x07
#define ALLPOT_C    0x08
#define KBCODE_C    0x09
#define RANDOM_C    0x0A
#define SERIN_C     0x0D
#define IRQST_C     0x0E
#define SKSTAT_C    0x0F

/* exact 1.79 MHz clock freq (of the Atari 800 that is) */
#define FREQ_17_EXACT   1789790


/*****************************************************************************
 * pot0_r to pot7_r:
 *  Handlers for reading the pot values. Some Atari games use
 *  ALLPOT to return dipswitch settings and other things.
 * serin_r, serout_w, interrupt_cb:
 *  New function pointers for serial input/output and a interrupt callback.
 *****************************************************************************/

typedef struct _pokey_interface pokey_interface;
struct _pokey_interface
{
	devcb_read8 pot_r[8];
	devcb_read8 allpot_r;
	devcb_read8 serin_r;
	devcb_write8 serout_w;
	void (*interrupt_cb)(running_device *device, int mask);
};


READ8_DEVICE_HANDLER( pokey_r );
WRITE8_DEVICE_HANDLER( pokey_w );

/* fix me: eventually this should be a single device with pokey subdevices */
READ8_HANDLER( quad_pokey_r );
WRITE8_HANDLER( quad_pokey_w );

void pokey_serin_ready (running_device *device, int after);
void pokey_break_w (running_device *device, int shift);
void pokey_kbcode_w (running_device *device, int kbcode, int make);

DECLARE_LEGACY_SOUND_DEVICE(POKEY, pokey);

#endif	/* __POKEY_H__ */
