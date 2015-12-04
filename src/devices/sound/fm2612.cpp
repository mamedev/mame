// license:???
// copyright-holders:Jarek Burczynski,Tatsuyuki Satoh
/*
**
** File: fm2612.c -- software implementation of Yamaha YM2612 FM sound generator
** Split from fm.c to keep 2612 fixes from infecting other OPN chips
**
** Copyright Jarek Burczynski (bujar at mame dot net)
** Copyright Tatsuyuki Satoh , MultiArcadeMachineEmulator development
**
** Version 1.5.1 (Genesis Plus GX ym2612.c rev. 368)
**
*/

/*
** History:
**
** 2006~2009  Eke-Eke (Genesis Plus GX):
** Huge thanks to Nemesis, lot of those fixes came from his tests on Sega Genesis hardware
** More informations at http://gendev.spritesmind.net/forum/viewtopic.php?t=386
**
**  TODO:
**
**  - core documentation
**  - BUSY flag support
**
**  CHANGELOG:
**
** 07-30-2014 dink (FB Alpha project):
**  - fixed missing dac channel on savestate load
**
** xx-xx-xxxx
**  - fixed LFO implementation:
**      .added support for CH3 special mode: fixes various sound effects (birds in Warlock, bug sound in Aladdin...)
**      .inverted LFO AM waveform: fixes Spider-Man & Venom : Separation Anxiety (intro), California Games (surfing event)
**      .improved LFO timing accuracy: now updated AFTER sample output, like EG/PG updates, and without any precision loss anymore.
**  - improved internal timers emulation
**  - adjusted lowest EG rates increment values
**  - fixed Attack Rate not being updated in some specific cases (Batman & Robin intro)
**  - fixed EG behavior when Attack Rate is maximal
**  - fixed EG behavior when SL=0 (Mega Turrican tracks 03,09...) or/and Key ON occurs at minimal attenuation
**  - implemented EG output immediate changes on register writes
**  - fixed YM2612 initial values (after the reset): fixes missing intro in B.O.B
**  - implemented Detune overflow (Ariel, Comix Zone, Shaq Fu, Spiderman & many other games using GEMS sound engine)
**  - implemented accurate CSM mode emulation
**  - implemented accurate SSG-EG emulation (Asterix, Beavis&Butthead, Bubba'n Stix & many other games)
**  - implemented accurate address/data ports behavior
**
** 06-23-2007 Zsolt Vasvari:
**  - changed the timing not to require the use of floating point calculations
**
** 03-08-2003 Jarek Burczynski:
**  - fixed YM2608 initial values (after the reset)
**  - fixed flag and irqmask handling (YM2608)
**  - fixed BUFRDY flag handling (YM2608)
**
** 14-06-2003 Jarek Burczynski:
**  - implemented all of the YM2608 status register flags
**  - implemented support for external memory read/write via YM2608
**  - implemented support for deltat memory limit register in YM2608 emulation
**
** 22-05-2003 Jarek Burczynski:
**  - fixed LFO PM calculations (copy&paste bugfix)
**
** 08-05-2003 Jarek Burczynski:
**  - fixed SSG support
**
** 22-04-2003 Jarek Burczynski:
**  - implemented 100% correct LFO generator (verified on real YM2610 and YM2608)
**
** 15-04-2003 Jarek Burczynski:
**  - added support for YM2608's register 0x110 - status mask
**
** 01-12-2002 Jarek Burczynski:
**  - fixed register addressing in YM2608, YM2610, YM2610B chips. (verified on real YM2608)
**    The addressing patch used for early Neo-Geo games can be removed now.
**
** 26-11-2002 Jarek Burczynski, Nicola Salmoria:
**  - recreated YM2608 ADPCM ROM using data from real YM2608's output which leads to:
**  - added emulation of YM2608 drums.
**  - output of YM2608 is two times lower now - same as YM2610 (verified on real YM2608)
**
** 16-08-2002 Jarek Burczynski:
**  - binary exact Envelope Generator (verified on real YM2203);
**    identical to YM2151
**  - corrected 'off by one' error in feedback calculations (when feedback is off)
**  - corrected connection (algorithm) calculation (verified on real YM2203 and YM2610)
**
** 18-12-2001 Jarek Burczynski:
**  - added SSG-EG support (verified on real YM2203)
**
** 12-08-2001 Jarek Burczynski:
**  - corrected sin_tab and tl_tab data (verified on real chip)
**  - corrected feedback calculations (verified on real chip)
**  - corrected phase generator calculations (verified on real chip)
**  - corrected envelope generator calculations (verified on real chip)
**  - corrected FM volume level (YM2610 and YM2610B).
**  - changed YMxxxUpdateOne() functions (YM2203, YM2608, YM2610, YM2610B, YM2612) :
**    this was needed to calculate YM2610 FM channels output correctly.
**    (Each FM channel is calculated as in other chips, but the output of the channel
**    gets shifted right by one *before* sending to accumulator. That was impossible to do
**    with previous implementation).
**
** 23-07-2001 Jarek Burczynski, Nicola Salmoria:
**  - corrected YM2610 ADPCM type A algorithm and tables (verified on real chip)
**
** 11-06-2001 Jarek Burczynski:
**  - corrected end of sample bug in ADPCMA_calc_cha().
**    Real YM2610 checks for equality between current and end addresses (only 20 LSB bits).
**
** 08-12-98 hiro-shi:
** rename ADPCMA -> ADPCMB, ADPCMB -> ADPCMA
** move ROM limit check.(CALC_CH? -> 2610Write1/2)
** test program (ADPCMB_TEST)
** move ADPCM A/B end check.
** ADPCMB repeat flag(no check)
** change ADPCM volume rate (8->16) (32->48).
**
** 09-12-98 hiro-shi:
** change ADPCM volume. (8->16, 48->64)
** replace ym2610 ch0/3 (YM-2610B)
** change ADPCM_SHIFT (10->8) missing bank change 0x4000-0xffff.
** add ADPCM_SHIFT_MASK
** change ADPCMA_DECODE_MIN/MAX.
*/




/************************************************************************/
/*    comment of hiro-shi(Hiromitsu Shioya)                             */
/*    YM2610(B) = OPN-B                                                 */
/*    YM2610  : PSG:3ch FM:4ch ADPCM(18.5KHz):6ch DeltaT ADPCM:1ch      */
/*    YM2610B : PSG:3ch FM:6ch ADPCM(18.5KHz):6ch DeltaT ADPCM:1ch      */
/************************************************************************/

#include "emu.h"
#include "fm.h"

/* shared function building option */
#define BUILD_OPN (BUILD_YM2203||BUILD_YM2608||BUILD_YM2610||BUILD_YM2610B||BUILD_YM2612||BUILD_YM3438)
#define BUILD_OPN_PRESCALER (BUILD_YM2203||BUILD_YM2608)


/* globals */
#define TYPE_SSG    0x01    /* SSG support          */
#define TYPE_LFOPAN 0x02    /* OPN type LFO and PAN */
#define TYPE_6CH    0x04    /* FM 6CH / 3CH         */
#define TYPE_DAC    0x08    /* YM2612's DAC device  */
#define TYPE_ADPCM  0x10    /* two ADPCM units      */
#define TYPE_2610   0x20    /* bogus flag to differentiate 2608 from 2610 */


#define TYPE_YM2203 (TYPE_SSG)
#define TYPE_YM2608 (TYPE_SSG |TYPE_LFOPAN |TYPE_6CH |TYPE_ADPCM)
#define TYPE_YM2610 (TYPE_SSG |TYPE_LFOPAN |TYPE_6CH |TYPE_ADPCM |TYPE_2610)
#define TYPE_YM2612 (TYPE_DAC |TYPE_LFOPAN |TYPE_6CH)


/* globals */
#define FREQ_SH         16  /* 16.16 fixed point (frequency calculations) */
#define EG_SH           16  /* 16.16 fixed point (envelope generator timing) */
#define LFO_SH          24  /*  8.24 fixed point (LFO calculations)       */
#define TIMER_SH        16  /* 16.16 fixed point (timers calculations)    */

#define FREQ_MASK       ((1<<FREQ_SH)-1)

#define MAXOUT    (+32767)
#define MINOUT    (-32768)

/* envelope generator */
#define ENV_BITS        10
#define ENV_LEN         (1<<ENV_BITS)
#define ENV_STEP        (128.0/ENV_LEN)

#define MAX_ATT_INDEX   (ENV_LEN-1) /* 1023 */
#define MIN_ATT_INDEX   (0)         /* 0 */

#define EG_ATT          4
#define EG_DEC          3
#define EG_SUS          2
#define EG_REL          1
#define EG_OFF          0

/* operator unit */
#define SIN_BITS        10
#define SIN_LEN         (1<<SIN_BITS)
#define SIN_MASK        (SIN_LEN-1)

#define TL_RES_LEN      (256) /* 8 bits addressing (real chip) */

/*  TL_TAB_LEN is calculated as:
*   13 - sinus amplitude bits     (Y axis)
*   2  - sinus sign bit           (Y axis)
*   TL_RES_LEN - sinus resolution (X axis)
*/
#define TL_TAB_LEN (13*2*TL_RES_LEN)
static signed int tl_tab[TL_TAB_LEN];

#define ENV_QUIET       (TL_TAB_LEN>>3)

/* sin waveform table in 'decibel' scale */
static unsigned int sin_tab[SIN_LEN];

/* sustain level table (3dB per step) */
/* bit0, bit1, bit2, bit3, bit4, bit5, bit6 */
/* 1,    2,    4,    8,    16,   32,   64   (value)*/
/* 0.75, 1.5,  3,    6,    12,   24,   48   (dB)*/

/* 0 - 15: 0, 3, 6, 9,12,15,18,21,24,27,30,33,36,39,42,93 (dB)*/
/* attenuation value (10 bits) = (SL << 2) << 3 */
#define SC(db) (UINT32) ( db * (4.0/ENV_STEP) )
static const UINT32 sl_table[16]={
	SC( 0),SC( 1),SC( 2),SC(3 ),SC(4 ),SC(5 ),SC(6 ),SC( 7),
	SC( 8),SC( 9),SC(10),SC(11),SC(12),SC(13),SC(14),SC(31)
};
#undef SC


#define RATE_STEPS (8)
static const UINT8 eg_inc[19*RATE_STEPS]={
/*cycle:0 1  2 3  4 5  6 7*/

/* 0 */ 0,1, 0,1, 0,1, 0,1, /* rates 00..11 0 (increment by 0 or 1) */
/* 1 */ 0,1, 0,1, 1,1, 0,1, /* rates 00..11 1 */
/* 2 */ 0,1, 1,1, 0,1, 1,1, /* rates 00..11 2 */
/* 3 */ 0,1, 1,1, 1,1, 1,1, /* rates 00..11 3 */

/* 4 */ 1,1, 1,1, 1,1, 1,1, /* rate 12 0 (increment by 1) */
/* 5 */ 1,1, 1,2, 1,1, 1,2, /* rate 12 1 */
/* 6 */ 1,2, 1,2, 1,2, 1,2, /* rate 12 2 */
/* 7 */ 1,2, 2,2, 1,2, 2,2, /* rate 12 3 */

/* 8 */ 2,2, 2,2, 2,2, 2,2, /* rate 13 0 (increment by 2) */
/* 9 */ 2,2, 2,4, 2,2, 2,4, /* rate 13 1 */
/*10 */ 2,4, 2,4, 2,4, 2,4, /* rate 13 2 */
/*11 */ 2,4, 4,4, 2,4, 4,4, /* rate 13 3 */

/*12 */ 4,4, 4,4, 4,4, 4,4, /* rate 14 0 (increment by 4) */
/*13 */ 4,4, 4,8, 4,4, 4,8, /* rate 14 1 */
/*14 */ 4,8, 4,8, 4,8, 4,8, /* rate 14 2 */
/*15 */ 4,8, 8,8, 4,8, 8,8, /* rate 14 3 */

/*16 */ 8,8, 8,8, 8,8, 8,8, /* rates 15 0, 15 1, 15 2, 15 3 (increment by 8) */
/*17 */ 16,16,16,16,16,16,16,16, /* rates 15 2, 15 3 for attack */
/*18 */ 0,0, 0,0, 0,0, 0,0, /* infinity rates for attack and decay(s) */
};


#define O(a) (a*RATE_STEPS)

/*note that there is no O(17) in this table - it's directly in the code */
static const UINT8 eg_rate_select2612[32+64+32]={  /* Envelope Generator rates (32 + 64 rates + 32 RKS) */
/* 32 infinite time rates (same as Rate 0) */
O(18),O(18),O(18),O(18),O(18),O(18),O(18),O(18),
O(18),O(18),O(18),O(18),O(18),O(18),O(18),O(18),
O(18),O(18),O(18),O(18),O(18),O(18),O(18),O(18),
O(18),O(18),O(18),O(18),O(18),O(18),O(18),O(18),

/* rates 00-11 */
/*
O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),
*/
O(18),O(18),O( 0),O( 0),
O( 0),O( 0),O( 2),O( 2),   // Nemesis's tests

O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),

/* rate 12 */
O( 4),O( 5),O( 6),O( 7),

/* rate 13 */
O( 8),O( 9),O(10),O(11),

/* rate 14 */
O(12),O(13),O(14),O(15),

/* rate 15 */
O(16),O(16),O(16),O(16),

/* 32 dummy rates (same as 15 3) */
O(16),O(16),O(16),O(16),O(16),O(16),O(16),O(16),
O(16),O(16),O(16),O(16),O(16),O(16),O(16),O(16),
O(16),O(16),O(16),O(16),O(16),O(16),O(16),O(16),
O(16),O(16),O(16),O(16),O(16),O(16),O(16),O(16)

};
#undef O

/*rate  0,    1,    2,   3,   4,   5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15*/
/*shift 11,   10,   9,   8,   7,   6,  5,  4,  3,  2, 1,  0,  0,  0,  0,  0 */
/*mask  2047, 1023, 511, 255, 127, 63, 31, 15, 7,  3, 1,  0,  0,  0,  0,  0 */

#define O(a) (a*1)
static const UINT8 eg_rate_shift[32+64+32]={  /* Envelope Generator counter shifts (32 + 64 rates + 32 RKS) */
/* 32 infinite time rates */
/* O(0),O(0),O(0),O(0),O(0),O(0),O(0),O(0),
O(0),O(0),O(0),O(0),O(0),O(0),O(0),O(0),
O(0),O(0),O(0),O(0),O(0),O(0),O(0),O(0),
O(0),O(0),O(0),O(0),O(0),O(0),O(0),O(0), */

/* fixed (should be the same as rate 0, even if it makes no difference since increment value is 0 for these rates) */
O(11),O(11),O(11),O(11),O(11),O(11),O(11),O(11),
O(11),O(11),O(11),O(11),O(11),O(11),O(11),O(11),
O(11),O(11),O(11),O(11),O(11),O(11),O(11),O(11),
O(11),O(11),O(11),O(11),O(11),O(11),O(11),O(11),

/* rates 00-11 */
O(11),O(11),O(11),O(11),
O(10),O(10),O(10),O(10),
O( 9),O( 9),O( 9),O( 9),
O( 8),O( 8),O( 8),O( 8),
O( 7),O( 7),O( 7),O( 7),
O( 6),O( 6),O( 6),O( 6),
O( 5),O( 5),O( 5),O( 5),
O( 4),O( 4),O( 4),O( 4),
O( 3),O( 3),O( 3),O( 3),
O( 2),O( 2),O( 2),O( 2),
O( 1),O( 1),O( 1),O( 1),
O( 0),O( 0),O( 0),O( 0),

/* rate 12 */
O( 0),O( 0),O( 0),O( 0),

/* rate 13 */
O( 0),O( 0),O( 0),O( 0),

/* rate 14 */
O( 0),O( 0),O( 0),O( 0),

/* rate 15 */
O( 0),O( 0),O( 0),O( 0),

/* 32 dummy rates (same as 15 3) */
O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),
O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),
O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),
O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),O( 0)

};
#undef O

static const UINT8 dt_tab[4 * 32]={
/* this is YM2151 and YM2612 phase increment data (in 10.10 fixed point format)*/
/* FD=0 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* FD=1 */
	0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2,
	2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 8, 8, 8, 8,
/* FD=2 */
	1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5,
	5, 6, 6, 7, 8, 8, 9,10,11,12,13,14,16,16,16,16,
/* FD=3 */
	2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7,
	8 , 8, 9,10,11,12,13,14,16,17,19,20,22,22,22,22
};


/* OPN key frequency number -> key code follow table */
/* fnum higher 4bit -> keycode lower 2bit */
static const UINT8 opn_fktable[16] = {0,0,0,0,0,0,0,1,2,3,3,3,3,3,3,3};


/* 8 LFO speed parameters */
/* each value represents number of samples that one LFO level will last for */
static const UINT32 lfo_samples_per_step[8] = {108, 77, 71, 67, 62, 44, 8, 5};



/*There are 4 different LFO AM depths available, they are:
  0 dB, 1.4 dB, 5.9 dB, 11.8 dB
  Here is how it is generated (in EG steps):

  11.8 dB = 0, 2, 4, 6, 8, 10,12,14,16...126,126,124,122,120,118,....4,2,0
   5.9 dB = 0, 1, 2, 3, 4, 5, 6, 7, 8....63, 63, 62, 61, 60, 59,.....2,1,0
   1.4 dB = 0, 0, 0, 0, 1, 1, 1, 1, 2,...15, 15, 15, 15, 14, 14,.....0,0,0

  (1.4 dB is losing precision as you can see)

  It's implemented as generator from 0..126 with step 2 then a shift
  right N times, where N is:
    8 for 0 dB
    3 for 1.4 dB
    1 for 5.9 dB
    0 for 11.8 dB
*/
static const UINT8 lfo_ams_depth_shift[4] = {8, 3, 1, 0};



/*There are 8 different LFO PM depths available, they are:
  0, 3.4, 6.7, 10, 14, 20, 40, 80 (cents)

  Modulation level at each depth depends on F-NUMBER bits: 4,5,6,7,8,9,10
  (bits 8,9,10 = FNUM MSB from OCT/FNUM register)

  Here we store only first quarter (positive one) of full waveform.
  Full table (lfo_pm_table) containing all 128 waveforms is build
  at run (init) time.

  One value in table below represents 4 (four) basic LFO steps
  (1 PM step = 4 AM steps).

  For example:
   at LFO SPEED=0 (which is 108 samples per basic LFO step)
   one value from "lfo_pm_output" table lasts for 432 consecutive
   samples (4*108=432) and one full LFO waveform cycle lasts for 13824
   samples (32*432=13824; 32 because we store only a quarter of whole
            waveform in the table below)
*/
static const UINT8 lfo_pm_output[7*8][8]={ /* 7 bits meaningful (of F-NUMBER), 8 LFO output levels per one depth (out of 32), 8 LFO depths */
/* FNUM BIT 4: 000 0001xxxx */
/* DEPTH 0 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 1 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 2 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 3 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 4 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 5 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 6 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 7 */ {0,   0,   0,   0,   1,   1,   1,   1},

/* FNUM BIT 5: 000 0010xxxx */
/* DEPTH 0 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 1 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 2 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 3 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 4 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 5 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 6 */ {0,   0,   0,   0,   1,   1,   1,   1},
/* DEPTH 7 */ {0,   0,   1,   1,   2,   2,   2,   3},

/* FNUM BIT 6: 000 0100xxxx */
/* DEPTH 0 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 1 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 2 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 3 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 4 */ {0,   0,   0,   0,   0,   0,   0,   1},
/* DEPTH 5 */ {0,   0,   0,   0,   1,   1,   1,   1},
/* DEPTH 6 */ {0,   0,   1,   1,   2,   2,   2,   3},
/* DEPTH 7 */ {0,   0,   2,   3,   4,   4,   5,   6},

/* FNUM BIT 7: 000 1000xxxx */
/* DEPTH 0 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 1 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 2 */ {0,   0,   0,   0,   0,   0,   1,   1},
/* DEPTH 3 */ {0,   0,   0,   0,   1,   1,   1,   1},
/* DEPTH 4 */ {0,   0,   0,   1,   1,   1,   1,   2},
/* DEPTH 5 */ {0,   0,   1,   1,   2,   2,   2,   3},
/* DEPTH 6 */ {0,   0,   2,   3,   4,   4,   5,   6},
/* DEPTH 7 */ {0,   0,   4,   6,   8,   8, 0xa, 0xc},

/* FNUM BIT 8: 001 0000xxxx */
/* DEPTH 0 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 1 */ {0,   0,   0,   0,   1,   1,   1,   1},
/* DEPTH 2 */ {0,   0,   0,   1,   1,   1,   2,   2},
/* DEPTH 3 */ {0,   0,   1,   1,   2,   2,   3,   3},
/* DEPTH 4 */ {0,   0,   1,   2,   2,   2,   3,   4},
/* DEPTH 5 */ {0,   0,   2,   3,   4,   4,   5,   6},
/* DEPTH 6 */ {0,   0,   4,   6,   8,   8, 0xa, 0xc},
/* DEPTH 7 */ {0,   0,   8, 0xc,0x10,0x10,0x14,0x18},

/* FNUM BIT 9: 010 0000xxxx */
/* DEPTH 0 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 1 */ {0,   0,   0,   0,   2,   2,   2,   2},
/* DEPTH 2 */ {0,   0,   0,   2,   2,   2,   4,   4},
/* DEPTH 3 */ {0,   0,   2,   2,   4,   4,   6,   6},
/* DEPTH 4 */ {0,   0,   2,   4,   4,   4,   6,   8},
/* DEPTH 5 */ {0,   0,   4,   6,   8,   8, 0xa, 0xc},
/* DEPTH 6 */ {0,   0,   8, 0xc,0x10,0x10,0x14,0x18},
/* DEPTH 7 */ {0,   0,0x10,0x18,0x20,0x20,0x28,0x30},

/* FNUM BIT10: 100 0000xxxx */
/* DEPTH 0 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 1 */ {0,   0,   0,   0,   4,   4,   4,   4},
/* DEPTH 2 */ {0,   0,   0,   4,   4,   4,   8,   8},
/* DEPTH 3 */ {0,   0,   4,   4,   8,   8, 0xc, 0xc},
/* DEPTH 4 */ {0,   0,   4,   8,   8,   8, 0xc,0x10},
/* DEPTH 5 */ {0,   0,   8, 0xc,0x10,0x10,0x14,0x18},
/* DEPTH 6 */ {0,   0,0x10,0x18,0x20,0x20,0x28,0x30},
/* DEPTH 7 */ {0,   0,0x20,0x30,0x40,0x40,0x50,0x60},

};

/* all 128 LFO PM waveforms */
static INT32 lfo_pm_table[128*8*32]; /* 128 combinations of 7 bits meaningful (of F-NUMBER), 8 LFO depths, 32 LFO output levels per one depth */

/* register number to channel number , slot offset */
#define OPN_CHAN(N) (N&3)
#define OPN_SLOT(N) ((N>>2)&3)

/* slot number */
#define SLOT1 0
#define SLOT2 2
#define SLOT3 1
#define SLOT4 3

/* bit0 = Right enable , bit1 = Left enable */
#define OUTD_RIGHT  1
#define OUTD_LEFT   2
#define OUTD_CENTER 3


/* save output as raw 16-bit sample */
/* #define SAVE_SAMPLE */

#ifdef SAVE_SAMPLE
static FILE *sample[1];
	#if 1   /*save to MONO file */
		#define SAVE_ALL_CHANNELS \
		{   signed int pom = lt; \
			fputc((unsigned short)pom&0xff,sample[0]); \
			fputc(((unsigned short)pom>>8)&0xff,sample[0]); \
		}
	#else   /*save to STEREO file */
		#define SAVE_ALL_CHANNELS \
		{   signed int pom = lt; \
			fputc((unsigned short)pom&0xff,sample[0]); \
			fputc(((unsigned short)pom>>8)&0xff,sample[0]); \
			pom = rt; \
			fputc((unsigned short)pom&0xff,sample[0]); \
			fputc(((unsigned short)pom>>8)&0xff,sample[0]); \
		}
	#endif
#endif


/* struct describing a single operator (SLOT) */
struct fm2612_FM_SLOT
{
	INT32   *DT;        /* detune          :dt_tab[DT] */
	UINT8   KSR;        /* key scale rate  :3-KSR */
	UINT32  ar;         /* attack rate  */
	UINT32  d1r;        /* decay rate   */
	UINT32  d2r;        /* sustain rate */
	UINT32  rr;         /* release rate */
	UINT8   ksr;        /* key scale rate  :kcode>>(3-KSR) */
	UINT32  mul;        /* multiple        :ML_TABLE[ML] */

	/* Phase Generator */
	UINT32  phase;      /* phase counter */
	INT32   Incr;       /* phase step */

	/* Envelope Generator */
	UINT8   state;      /* phase type */
	UINT32  tl;         /* total level: TL << 3 */
	INT32   volume;     /* envelope counter */
	UINT32  sl;         /* sustain level:sl_table[SL] */
	UINT32  vol_out;    /* current output from EG circuit (without AM from LFO) */

	UINT8   eg_sh_ar;   /*  (attack state) */
	UINT8   eg_sel_ar;  /*  (attack state) */
	UINT8   eg_sh_d1r;  /*  (decay state) */
	UINT8   eg_sel_d1r; /*  (decay state) */
	UINT8   eg_sh_d2r;  /*  (sustain state) */
	UINT8   eg_sel_d2r; /*  (sustain state) */
	UINT8   eg_sh_rr;   /*  (release state) */
	UINT8   eg_sel_rr;  /*  (release state) */

	UINT8   ssg;        /* SSG-EG waveform */
	UINT8   ssgn;       /* SSG-EG negated output */

	UINT8   key;        /* 0=last key was KEY OFF, 1=KEY ON */

	/* LFO */
	UINT32  AMmask;     /* AM enable flag */

};

struct fm2612_FM_CH
{
	fm2612_FM_SLOT SLOT[4];    /* four SLOTs (operators) */

	UINT8   ALGO;       /* algorithm */
	UINT8   FB;         /* feedback shift */
	INT32   op1_out[2]; /* op1 output for feedback */

	INT32   *connect1;  /* SLOT1 output pointer */
	INT32   *connect3;  /* SLOT3 output pointer */
	INT32   *connect2;  /* SLOT2 output pointer */
	INT32   *connect4;  /* SLOT4 output pointer */

	INT32   *mem_connect;/* where to put the delayed sample (MEM) */
	INT32   mem_value;  /* delayed sample (MEM) value */

	INT32   pms;        /* channel PMS */
	UINT8   ams;        /* channel AMS */

	UINT32  fc;         /* fnum,blk:adjusted to sample rate */
	UINT8   kcode;      /* key code:                        */
	UINT32  block_fnum; /* current blk/fnum value for this slot (can be different betweeen slots of one channel in 3slot mode) */
};


struct fm2612_FM_ST
{
	device_t *device;
	void *      param;              /* this chip parameter  */
	double      freqbase;           /* frequency base       */
	int         timer_prescaler;    /* timer prescaler      */
	UINT8       irq;                /* interrupt level      */
	UINT8       irqmask;            /* irq mask             */
#if FM_BUSY_FLAG_SUPPORT
	TIME_TYPE   busy_expiry_time;   /* expiry time of the busy status */
#endif
	UINT32      clock;              /* master clock  (Hz)   */
	UINT32      rate;               /* sampling rate (Hz)   */
	UINT16      address;            /* address register     */
	UINT8       status;             /* status flag          */
	UINT32      mode;               /* mode  CSM / 3SLOT    */
	UINT8       fn_h;               /* freq latch           */
	UINT8       prescaler_sel;      /* prescaler selector   */
	INT32       TA;                 /* timer a              */
	INT32       TAC;                /* timer a counter      */
	UINT8       TB;                 /* timer b              */
	INT32       TBC;                /* timer b counter      */
	/* local time tables */
	INT32       dt_tab[8][32];      /* DeTune table         */
	/* Extention Timer and IRQ handler */
	FM_TIMERHANDLER timer_handler;
	FM_IRQHANDLER   IRQ_Handler;
	const ssg_callbacks *SSG;
};



/***********************************************************/
/* OPN unit                                                */
/***********************************************************/

/* OPN 3slot struct */
struct fm2612_FM_3SLOT
{
	UINT32  fc[3];          /* fnum3,blk3: calculated */
	UINT8   fn_h;           /* freq3 latch */
	UINT8   kcode[3];       /* key code */
	UINT32  block_fnum[3];  /* current fnum value for this slot (can be different betweeen slots of one channel in 3slot mode) */
	UINT8   key_csm;        /* CSM mode Key-ON flag */
};

/* OPN/A/B common state */
struct fm2612_FM_OPN
{
	UINT8   type;           /* chip type */
	fm2612_FM_ST   ST;             /* general state */
	fm2612_FM_3SLOT SL3;           /* 3 slot mode state */
	fm2612_FM_CH   *P_CH;          /* pointer of CH */
	unsigned int pan[6*2];  /* fm channels output masks (0xffffffff = enable) */

	UINT32  eg_cnt;         /* global envelope generator counter */
	UINT32  eg_timer;       /* global envelope generator counter works at frequency = chipclock/144/3 */
	UINT32  eg_timer_add;   /* step of eg_timer */
	UINT32  eg_timer_overflow;/* envelope generator timer overlfows every 3 samples (on real chip) */


	/* there are 2048 FNUMs that can be generated using FNUM/BLK registers
	   but LFO works with one more bit of a precision so we really need 4096 elements */
	UINT32  fn_table[4096]; /* fnumber->increment counter */
	UINT32 fn_max;    /* maximal phase increment (used for phase overflow) */

	/* LFO */
	UINT8   lfo_cnt;            /* current LFO phase (out of 128) */
	UINT32  lfo_timer;          /* current LFO phase runs at LFO frequency */
	UINT32  lfo_timer_add;      /* step of lfo_timer */
	UINT32  lfo_timer_overflow; /* LFO timer overflows every N samples (depends on LFO frequency) */
	UINT32  LFO_AM;             /* current LFO AM step */
	UINT32  LFO_PM;             /* current LFO PM step */

	INT32   m2,c1,c2;       /* Phase Modulation input for operators 2,3,4 */
	INT32   mem;            /* one sample delay memory */
	INT32   out_fm[8];      /* outputs of working channels */

};

/* here's the virtual YM2612 */
struct YM2612
{
	UINT8       REGS[512];          /* registers            */
	fm2612_FM_OPN      OPN;                /* OPN state            */
	fm2612_FM_CH       CH[6];              /* channel state        */
	UINT8       addr_A1;            /* address line A1      */

	/* dac output (YM2612) */
	int         dacen;
	INT32       dacout;
	device_t    *device;
};

/* log output level */
#define LOG_ERR  3      /* ERROR       */
#define LOG_WAR  2      /* WARNING     */
#define LOG_INF  1      /* INFORMATION */
#define LOG_LEVEL LOG_INF

#ifndef __RAINE__
#define LOG(d,n,x) do { if( (n)>=LOG_LEVEL ) d->logerror x; } while (0)
#endif

/* limitter */
#define Limit(val, max,min) { \
	if ( val > max )      val = max; \
	else if ( val < min ) val = min; \
}


/* status set and IRQ handling */
INLINE void FM_STATUS_SET(fm2612_FM_ST *ST,int flag)
{
	/* set status flag */
	ST->status |= flag;
	if ( !(ST->irq) && (ST->status & ST->irqmask) )
	{
		ST->irq = 1;
		/* callback user interrupt handler (IRQ is OFF to ON) */
		if(ST->IRQ_Handler) (ST->IRQ_Handler)(ST->param,1);
	}
}

/* status reset and IRQ handling */
INLINE void FM_STATUS_RESET(fm2612_FM_ST *ST,int flag)
{
	/* reset status flag */
	ST->status &=~flag;
	if ( (ST->irq) && !(ST->status & ST->irqmask) )
	{
		ST->irq = 0;
		/* callback user interrupt handler (IRQ is ON to OFF) */
		if(ST->IRQ_Handler) (ST->IRQ_Handler)(ST->param,0);
	}
}

/* IRQ mask set */
INLINE void FM_IRQMASK_SET(fm2612_FM_ST *ST,int flag)
{
	ST->irqmask = flag;
	/* IRQ handling check */
	FM_STATUS_SET(ST,0);
	FM_STATUS_RESET(ST,0);
}

INLINE void FM_KEYON(fm2612_FM_OPN *OPN, fm2612_FM_CH *CH , int s )
{
	fm2612_FM_SLOT *SLOT = &CH->SLOT[s];

	if( !SLOT->key && !OPN->SL3.key_csm)
	{
		/* restart Phase Generator */
		SLOT->phase = 0;

		/* reset SSG-EG inversion flag */
		SLOT->ssgn = 0;

		if ((SLOT->ar + SLOT->ksr) < 94 /*32+62*/)
		{
			SLOT->state = (SLOT->volume <= MIN_ATT_INDEX) ? ((SLOT->sl == MIN_ATT_INDEX) ? EG_SUS : EG_DEC) : EG_ATT;
		}
		else
		{
			/* force attenuation level to 0 */
			SLOT->volume = MIN_ATT_INDEX;

			/* directly switch to Decay (or Sustain) */
			SLOT->state = (SLOT->sl == MIN_ATT_INDEX) ? EG_SUS : EG_DEC;
		}

		/* recalculate EG output */
		if ((SLOT->ssg&0x08) && (SLOT->ssgn ^ (SLOT->ssg&0x04)))
			SLOT->vol_out = ((UINT32)(0x200 - SLOT->volume) & MAX_ATT_INDEX) + SLOT->tl;
		else
			SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
	}

	SLOT->key = 1;
}

INLINE void FM_KEYOFF(fm2612_FM_OPN *OPN, fm2612_FM_CH *CH , int s )
{
	fm2612_FM_SLOT *SLOT = &CH->SLOT[s];

	if (SLOT->key && !OPN->SL3.key_csm)
	{
		if (SLOT->state>EG_REL)
		{
			SLOT->state = EG_REL; /* phase -> Release */

			/* SSG-EG specific update */
			if (SLOT->ssg&0x08)
			{
				/* convert EG attenuation level */
				if (SLOT->ssgn ^ (SLOT->ssg&0x04))
						SLOT->volume = (0x200 - SLOT->volume);

				/* force EG attenuation level */
				if (SLOT->volume >= 0x200)
				{
					SLOT->volume = MAX_ATT_INDEX;
					SLOT->state  = EG_OFF;
				}

				/* recalculate EG output */
				SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
			}
		}
	}

	SLOT->key = 0;
}

INLINE void FM_KEYON_CSM(fm2612_FM_OPN *OPN, fm2612_FM_CH *CH , int s )
{
	fm2612_FM_SLOT *SLOT = &CH->SLOT[s];

	if( !SLOT->key && !OPN->SL3.key_csm)
	{
		/* restart Phase Generator */
		SLOT->phase = 0;

		/* reset SSG-EG inversion flag */
		SLOT->ssgn = 0;

		if ((SLOT->ar + SLOT->ksr) < 94 /*32+62*/)
		{
			SLOT->state = (SLOT->volume <= MIN_ATT_INDEX) ? ((SLOT->sl == MIN_ATT_INDEX) ? EG_SUS : EG_DEC) : EG_ATT;
		}
		else
		{
			/* force attenuation level to 0 */
			SLOT->volume = MIN_ATT_INDEX;

			/* directly switch to Decay (or Sustain) */
			SLOT->state = (SLOT->sl == MIN_ATT_INDEX) ? EG_SUS : EG_DEC;
		}

		/* recalculate EG output */
		if ((SLOT->ssg&0x08) && (SLOT->ssgn ^ (SLOT->ssg&0x04)))
			SLOT->vol_out = ((UINT32)(0x200 - SLOT->volume) & MAX_ATT_INDEX) + SLOT->tl;
		else
			SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
	}
}

INLINE void FM_KEYOFF_CSM(fm2612_FM_CH *CH , int s )
{
	fm2612_FM_SLOT *SLOT = &CH->SLOT[s];
	if (!SLOT->key)
	{
		if (SLOT->state>EG_REL)
		{
			SLOT->state = EG_REL; /* phase -> Release */

			/* SSG-EG specific update */
			if (SLOT->ssg&0x08)
			{
				/* convert EG attenuation level */
				if (SLOT->ssgn ^ (SLOT->ssg&0x04))
					SLOT->volume = (0x200 - SLOT->volume);

				/* force EG attenuation level */
				if (SLOT->volume >= 0x200)
				{
					SLOT->volume = MAX_ATT_INDEX;
					SLOT->state  = EG_OFF;
				}

				/* recalculate EG output */
				SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
			}
		}
	}
}

/* OPN Mode Register Write */
INLINE void set_timers(fm2612_FM_OPN *OPN, fm2612_FM_ST *ST, void *n, int v)
{
	/* b7 = CSM MODE */
	/* b6 = 3 slot mode */
	/* b5 = reset b */
	/* b4 = reset a */
	/* b3 = timer enable b */
	/* b2 = timer enable a */
	/* b1 = load b */
	/* b0 = load a */

	if ((OPN->ST.mode ^ v) & 0xC0)
	{
		/* phase increment need to be recalculated */
		OPN->P_CH[2].SLOT[SLOT1].Incr=-1;

		/* CSM mode disabled and CSM key ON active*/
		if (((v & 0xC0) != 0x80) && OPN->SL3.key_csm)
		{
			/* CSM Mode Key OFF (verified by Nemesis on real hardware) */
			FM_KEYOFF_CSM(&OPN->P_CH[2],SLOT1);
			FM_KEYOFF_CSM(&OPN->P_CH[2],SLOT2);
			FM_KEYOFF_CSM(&OPN->P_CH[2],SLOT3);
			FM_KEYOFF_CSM(&OPN->P_CH[2],SLOT4);
			OPN->SL3.key_csm = 0;
		}
	}

	/* reload Timers */
	if ((v&1) && !(ST->mode&1))
	{
		ST->TAC = (1024-ST->TA);
		/* External timer handler */
		if (ST->timer_handler) (ST->timer_handler)(n,0,ST->TAC * ST->timer_prescaler,ST->clock);
	}
	else if (!(v & 1))
	{
		if( ST->TAC != 0 )
		{
			ST->TAC = 0;
			if (ST->timer_handler) (ST->timer_handler)(n,0,0,ST->clock);
		}
	}

	if ((v&2) && !(ST->mode&2))
	{
		ST->TBC = ( 256-ST->TB)<<4;
		/* External timer handler */
		if (ST->timer_handler) (ST->timer_handler)(n,1,ST->TBC * ST->timer_prescaler,ST->clock);
	}
	else if (!(v & 2))
	{
		if( ST->TBC != 0 )
		{
			ST->TBC = 0;
			if (ST->timer_handler) (ST->timer_handler)(n,1,0,ST->clock);
		}
	}

	/* reset Timers flags */
	ST->status &= (~v >> 4);

	/* if IRQ should be lowered now, do so */
	if ( (ST->irq) && !(ST->status & ST->irqmask) )
	{
		ST->irq = 0;
		/* callback user interrupt handler (IRQ is ON to OFF) */
		if(ST->IRQ_Handler) (ST->IRQ_Handler)(ST->param, 0);
	}
	ST->mode = v;
}


/* Timer A Overflow */
INLINE void TimerAOver(fm2612_FM_ST *ST)
{
	/* set status (if enabled) */
	if(ST->mode & 0x04) FM_STATUS_SET(ST,0x01);
	/* clear or reload the counter */
	ST->TAC = (1024-ST->TA);
	if (ST->timer_handler) (ST->timer_handler)(ST->param,0,ST->TAC * ST->timer_prescaler,ST->clock);
}
/* Timer B Overflow */
INLINE void TimerBOver(fm2612_FM_ST *ST)
{
	/* set status (if enabled) */
	if(ST->mode & 0x08) FM_STATUS_SET(ST,0x02);
	/* clear or reload the counter */
	ST->TBC = ( 256-ST->TB)<<4;
	if (ST->timer_handler) (ST->timer_handler)(ST->param,1,ST->TBC * ST->timer_prescaler,ST->clock);
}


#if FM_INTERNAL_TIMER
/* ----- internal timer mode , update timer */

/* ---------- calculate timer A ---------- */
	#define INTERNAL_TIMER_A(ST,CSM_CH)                 \
	{                                                   \
		if( ST->TAC &&  (ST->timer_handler==0) )        \
			if( (ST->TAC -= (int)(ST->freqbase*4096)) <= 0 )    \
			{                                           \
				TimerAOver( ST );                       \
				/* CSM mode total level latch and auto key on */    \
				if( ST->mode & 0x80 )                   \
					CSMKeyControll( CSM_CH );           \
			}                                           \
	}
/* ---------- calculate timer B ---------- */
	#define INTERNAL_TIMER_B(ST,step)                       \
	{                                                       \
		if( ST->TBC && (ST->timer_handler==0) )             \
			if( (ST->TBC -= (int)(ST->freqbase*4096*step)) <= 0 )   \
				TimerBOver( ST );                           \
	}
#else /* FM_INTERNAL_TIMER */
/* external timer mode */
#define INTERNAL_TIMER_A(ST,CSM_CH)
#define INTERNAL_TIMER_B(ST,step)
#endif /* FM_INTERNAL_TIMER */



#if FM_BUSY_FLAG_SUPPORT
#define FM_BUSY_CLEAR(ST) ((ST)->busy_expiry_time = UNDEFINED_TIME)
INLINE UINT8 FM_STATUS_FLAG(fm2612_FM_ST *ST)
{
	if( COMPARE_TIMES(ST->busy_expiry_time, UNDEFINED_TIME) != 0 )
	{
		if (COMPARE_TIMES(ST->busy_expiry_time, FM_GET_TIME_NOW(&ST->device->machine())) > 0)
			return ST->status | 0x80;   /* with busy */
		/* expire */
		FM_BUSY_CLEAR(ST);
	}
	return ST->status;
}
#if 0
INLINE void FM_BUSY_SET(fm2612_FM_ST *ST,int busyclock )
{
	TIME_TYPE expiry_period = MULTIPLY_TIME_BY_INT(attotime::from_hz(ST->clock), busyclock * ST->timer_prescaler);
	ST->busy_expiry_time = ADD_TIMES(FM_GET_TIME_NOW(&ST->device->machine()), expiry_period);
}
#endif
#else
#define FM_STATUS_FLAG(ST) ((ST)->status)
#define FM_BUSY_SET(ST,bclock) {}
#define FM_BUSY_CLEAR(ST) {}
#endif


/* set algorithm connection */
static void setup_connection(fm2612_FM_OPN *OPN, fm2612_FM_CH *CH, int ch)
{
	INT32 *carrier = &OPN->out_fm[ch];

	INT32 **om1 = &CH->connect1;
	INT32 **om2 = &CH->connect3;
	INT32 **oc1 = &CH->connect2;

	INT32 **memc = &CH->mem_connect;

	switch( CH->ALGO )
	{
	case 0:
		/* M1---C1---MEM---M2---C2---OUT */
		*om1 = &OPN->c1;
		*oc1 = &OPN->mem;
		*om2 = &OPN->c2;
		*memc= &OPN->m2;
		break;
	case 1:
		/* M1------+-MEM---M2---C2---OUT */
		/*      C1-+                     */
		*om1 = &OPN->mem;
		*oc1 = &OPN->mem;
		*om2 = &OPN->c2;
		*memc= &OPN->m2;
		break;
	case 2:
		/* M1-----------------+-C2---OUT */
		/*      C1---MEM---M2-+          */
		*om1 = &OPN->c2;
		*oc1 = &OPN->mem;
		*om2 = &OPN->c2;
		*memc= &OPN->m2;
		break;
	case 3:
		/* M1---C1---MEM------+-C2---OUT */
		/*                 M2-+          */
		*om1 = &OPN->c1;
		*oc1 = &OPN->mem;
		*om2 = &OPN->c2;
		*memc= &OPN->c2;
		break;
	case 4:
		/* M1---C1-+-OUT */
		/* M2---C2-+     */
		/* MEM: not used */
		*om1 = &OPN->c1;
		*oc1 = carrier;
		*om2 = &OPN->c2;
		*memc= &OPN->mem;   /* store it anywhere where it will not be used */
		break;
	case 5:
		/*    +----C1----+     */
		/* M1-+-MEM---M2-+-OUT */
		/*    +----C2----+     */
		*om1 = nullptr;   /* special mark */
		*oc1 = carrier;
		*om2 = carrier;
		*memc= &OPN->m2;
		break;
	case 6:
		/* M1---C1-+     */
		/*      M2-+-OUT */
		/*      C2-+     */
		/* MEM: not used */
		*om1 = &OPN->c1;
		*oc1 = carrier;
		*om2 = carrier;
		*memc= &OPN->mem;   /* store it anywhere where it will not be used */
		break;
	case 7:
		/* M1-+     */
		/* C1-+-OUT */
		/* M2-+     */
		/* C2-+     */
		/* MEM: not used*/
		*om1 = carrier;
		*oc1 = carrier;
		*om2 = carrier;
		*memc= &OPN->mem;   /* store it anywhere where it will not be used */
		break;
	}

	CH->connect4 = carrier;
}

/* set detune & multiple */
INLINE void set_det_mul(fm2612_FM_ST *ST,fm2612_FM_CH *CH,fm2612_FM_SLOT *SLOT,int v)
{
	SLOT->mul = (v&0x0f)? (v&0x0f)*2 : 1;
	SLOT->DT  = ST->dt_tab[(v>>4)&7];
	CH->SLOT[SLOT1].Incr=-1;
}

/* set total level */
INLINE void set_tl(fm2612_FM_CH *CH,fm2612_FM_SLOT *SLOT , int v)
{
	SLOT->tl = (v&0x7f)<<(ENV_BITS-7); /* 7bit TL */

	/* recalculate EG output */
	if ((SLOT->ssg&0x08) && (SLOT->ssgn ^ (SLOT->ssg&0x04)) && (SLOT->state > EG_REL))
		SLOT->vol_out = ((UINT32)(0x200 - SLOT->volume) & MAX_ATT_INDEX) + SLOT->tl;
	else
		SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
}

/* set attack rate & key scale  */
INLINE void set_ar_ksr(UINT8 type, fm2612_FM_CH *CH,fm2612_FM_SLOT *SLOT,int v)
{
	UINT8 old_KSR = SLOT->KSR;

	SLOT->ar = (v&0x1f) ? 32 + ((v&0x1f)<<1) : 0;

	SLOT->KSR = 3-(v>>6);
	if (SLOT->KSR != old_KSR)
	{
		CH->SLOT[SLOT1].Incr=-1;
	}

	/* Even if it seems unnecessary, in some odd case, KSR and KC are modified   */
	/* and could result in SLOT->kc remaining unchanged.                              */
	/* In such case, AR values would not be recalculated despite SLOT->ar has changed */
	/* This actually fixes the intro of "The Adventures of Batman & Robin" (Eke-Eke) */
	if ((SLOT->ar + SLOT->ksr) < 94 /*32+62*/)
	{
		SLOT->eg_sh_ar  = eg_rate_shift [SLOT->ar  + SLOT->ksr ];
		SLOT->eg_sel_ar = eg_rate_select2612[SLOT->ar  + SLOT->ksr ];
	}
	else
	{
		SLOT->eg_sh_ar  = 0;
		SLOT->eg_sel_ar = 18*RATE_STEPS;    /* verified by Nemesis on real hardware */
	}
}

/* set decay rate */
INLINE void set_dr(UINT8 type, fm2612_FM_SLOT *SLOT,int v)
{
	SLOT->d1r = (v&0x1f) ? 32 + ((v&0x1f)<<1) : 0;

	SLOT->eg_sh_d1r = eg_rate_shift [SLOT->d1r + SLOT->ksr];
	SLOT->eg_sel_d1r= eg_rate_select2612[SLOT->d1r + SLOT->ksr];
}

/* set sustain rate */
INLINE void set_sr(UINT8 type, fm2612_FM_SLOT *SLOT,int v)
{
	SLOT->d2r = (v&0x1f) ? 32 + ((v&0x1f)<<1) : 0;

	SLOT->eg_sh_d2r = eg_rate_shift [SLOT->d2r + SLOT->ksr];
	SLOT->eg_sel_d2r= eg_rate_select2612[SLOT->d2r + SLOT->ksr];
}

/* set release rate */
INLINE void set_sl_rr(UINT8 type, fm2612_FM_SLOT *SLOT,int v)
{
	SLOT->sl = sl_table[ v>>4 ];

	/* check EG state changes */
	if ((SLOT->state == EG_DEC) && (SLOT->volume >= (INT32)(SLOT->sl)))
		SLOT->state = EG_SUS;

	SLOT->rr  = 34 + ((v&0x0f)<<2);

	SLOT->eg_sh_rr  = eg_rate_shift [SLOT->rr  + SLOT->ksr];
	SLOT->eg_sel_rr = eg_rate_select2612[SLOT->rr  + SLOT->ksr];
}

/* advance LFO to next sample */
INLINE void advance_lfo(fm2612_FM_OPN *OPN)
{
	if (OPN->lfo_timer_overflow)   /* LFO enabled ? */
	{
		/* increment LFO timer */
		OPN->lfo_timer +=  OPN->lfo_timer_add;

		/* when LFO is enabled, one level will last for 108, 77, 71, 67, 62, 44, 8 or 5 samples */
		while (OPN->lfo_timer >= OPN->lfo_timer_overflow)
		{
			OPN->lfo_timer -= OPN->lfo_timer_overflow;

			/* There are 128 LFO steps */
			OPN->lfo_cnt = ( OPN->lfo_cnt + 1 ) & 127;

			/* triangle (inverted) */
			/* AM: from 126 to 0 step -2, 0 to 126 step +2 */
			if (OPN->lfo_cnt<64)
				OPN->LFO_AM = (OPN->lfo_cnt ^ 63) << 1;
			else
				OPN->LFO_AM = (OPN->lfo_cnt & 63) << 1;

			/* PM works with 4 times slower clock */
			OPN->LFO_PM = OPN->lfo_cnt >> 2;
		}
	}
}

/* changed from INLINE to static here to work around gcc 4.2.1 codegen bug */
static void advance_eg_channel(fm2612_FM_OPN *OPN, fm2612_FM_SLOT *SLOT)
{
	unsigned int out;
	unsigned int i = 4; /* four operators per channel */

	do
	{
		switch(SLOT->state)
		{
			case EG_ATT:    /* attack phase */
			if (!(OPN->eg_cnt & ((1<<SLOT->eg_sh_ar)-1)))
			{
					/* update attenuation level */
					SLOT->volume += (~SLOT->volume * (eg_inc[SLOT->eg_sel_ar + ((OPN->eg_cnt>>SLOT->eg_sh_ar)&7)]))>>4;

					/* check phase transition*/
					if (SLOT->volume <= MIN_ATT_INDEX)
					{
						SLOT->volume = MIN_ATT_INDEX;
						SLOT->state = (SLOT->sl == MIN_ATT_INDEX) ? EG_SUS : EG_DEC; /* special case where SL=0 */
					}

					/* recalculate EG output */
					if ((SLOT->ssg&0x08) && (SLOT->ssgn ^ (SLOT->ssg&0x04)))  /* SSG-EG Output Inversion */
					SLOT->vol_out = ((UINT32)(0x200 - SLOT->volume) & MAX_ATT_INDEX) + SLOT->tl;
					else
						SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
			}
			break;

			case EG_DEC:  /* decay phase */
			if (!(OPN->eg_cnt & ((1<<SLOT->eg_sh_d1r)-1)))
			{
					/* SSG EG type */
					if (SLOT->ssg&0x08)
					{
						/* update attenuation level */
						if (SLOT->volume < 0x200)
					{
						SLOT->volume += 4 * eg_inc[SLOT->eg_sel_d1r + ((OPN->eg_cnt>>SLOT->eg_sh_d1r)&7)];

						/* recalculate EG output */
						if (SLOT->ssgn ^ (SLOT->ssg&0x04))   /* SSG-EG Output Inversion */
							SLOT->vol_out = ((UINT32)(0x200 - SLOT->volume) & MAX_ATT_INDEX) + SLOT->tl;
						else
							SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
					}

					}
					else
					{
					/* update attenuation level */
					SLOT->volume += eg_inc[SLOT->eg_sel_d1r + ((OPN->eg_cnt>>SLOT->eg_sh_d1r)&7)];

					/* recalculate EG output */
					SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
					}

					/* check phase transition*/
					if (SLOT->volume >= (INT32)(SLOT->sl))
						SLOT->state = EG_SUS;
			}
			break;

			case EG_SUS:  /* sustain phase */
			if (!(OPN->eg_cnt & ((1<<SLOT->eg_sh_d2r)-1)))
			{
					/* SSG EG type */
					if (SLOT->ssg&0x08)
					{
					/* update attenuation level */
					if (SLOT->volume < 0x200)
					{
						SLOT->volume += 4 * eg_inc[SLOT->eg_sel_d2r + ((OPN->eg_cnt>>SLOT->eg_sh_d2r)&7)];

						/* recalculate EG output */
						if (SLOT->ssgn ^ (SLOT->ssg&0x04))   /* SSG-EG Output Inversion */
							SLOT->vol_out = ((UINT32)(0x200 - SLOT->volume) & MAX_ATT_INDEX) + SLOT->tl;
						else
							SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
					}
					}
					else
					{
						/* update attenuation level */
						SLOT->volume += eg_inc[SLOT->eg_sel_d2r + ((OPN->eg_cnt>>SLOT->eg_sh_d2r)&7)];

						/* check phase transition*/
						if ( SLOT->volume >= MAX_ATT_INDEX )
							SLOT->volume = MAX_ATT_INDEX;
						/* do not change SLOT->state (verified on real chip) */

						/* recalculate EG output */
						SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
					}
			}
			break;

			case EG_REL:  /* release phase */
			if (!(OPN->eg_cnt & ((1<<SLOT->eg_sh_rr)-1)))
			{
					/* SSG EG type */
					if (SLOT->ssg&0x08)
					{
						/* update attenuation level */
						if (SLOT->volume < 0x200)
							SLOT->volume += 4 * eg_inc[SLOT->eg_sel_rr + ((OPN->eg_cnt>>SLOT->eg_sh_rr)&7)];
					/* check phase transition */
					if (SLOT->volume >= 0x200)
					{
						SLOT->volume = MAX_ATT_INDEX;
						SLOT->state = EG_OFF;
					}
					}
					else
					{
						/* update attenuation level */
						SLOT->volume += eg_inc[SLOT->eg_sel_rr + ((OPN->eg_cnt>>SLOT->eg_sh_rr)&7)];

						/* check phase transition*/
						if (SLOT->volume >= MAX_ATT_INDEX)
						{
							SLOT->volume = MAX_ATT_INDEX;
							SLOT->state = EG_OFF;
						}
					}

					/* recalculate EG output */
					SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;

			}
			break;
		}

		out = ((UINT32)SLOT->volume);

				/* negate output (changes come from alternate bit, init comes from attack bit) */
		if ((SLOT->ssg&0x08) && (SLOT->ssgn&2) && (SLOT->state > EG_REL))
			out ^= MAX_ATT_INDEX;

		/* we need to store the result here because we are going to change ssgn
		    in next instruction */
		SLOT->vol_out = out + SLOT->tl;

		SLOT++;
		i--;
	}while (i);

}

/* SSG-EG update process */
/* The behavior is based upon Nemesis tests on real hardware */
/* This is actually executed before each samples */
static void update_ssg_eg_channel(fm2612_FM_SLOT *SLOT)
{
	unsigned int i = 4; /* four operators per channel */

	do
	{
		/* detect SSG-EG transition */
		/* this is not required during release phase as the attenuation has been forced to MAX and output invert flag is not used */
		/* if an Attack Phase is programmed, inversion can occur on each sample */
		if ((SLOT->ssg & 0x08) && (SLOT->volume >= 0x200) && (SLOT->state > EG_REL))
		{
			if (SLOT->ssg & 0x01)  /* bit 0 = hold SSG-EG */
			{
				/* set inversion flag */
					if (SLOT->ssg & 0x02)
						SLOT->ssgn = 4;

				/* force attenuation level during decay phases */
				if ((SLOT->state != EG_ATT) && !(SLOT->ssgn ^ (SLOT->ssg & 0x04)))
					SLOT->volume  = MAX_ATT_INDEX;
			}
			else  /* loop SSG-EG */
			{
				/* toggle output inversion flag or reset Phase Generator */
					if (SLOT->ssg & 0x02)
						SLOT->ssgn ^= 4;
					else
						SLOT->phase = 0;

				/* same as Key ON */
				if (SLOT->state != EG_ATT)
				{
					if ((SLOT->ar + SLOT->ksr) < 94 /*32+62*/)
					{
						SLOT->state = (SLOT->volume <= MIN_ATT_INDEX) ? ((SLOT->sl == MIN_ATT_INDEX) ? EG_SUS : EG_DEC) : EG_ATT;
					}
					else
					{
						/* Attack Rate is maximal: directly switch to Decay or Substain */
						SLOT->volume = MIN_ATT_INDEX;
						SLOT->state = (SLOT->sl == MIN_ATT_INDEX) ? EG_SUS : EG_DEC;
					}
				}
			}

			/* recalculate EG output */
			if (SLOT->ssgn ^ (SLOT->ssg&0x04))
				SLOT->vol_out = ((UINT32)(0x200 - SLOT->volume) & MAX_ATT_INDEX) + SLOT->tl;
			else
				SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
		}

		/* next slot */
		SLOT++;
		i--;
	} while (i);
}


INLINE void update_phase_lfo_slot(fm2612_FM_OPN *OPN, fm2612_FM_SLOT *SLOT, INT32 pms, UINT32 block_fnum)
{
	UINT32 fnum_lfo   = ((block_fnum & 0x7f0) >> 4) * 32 * 8;
	INT32  lfo_fn_table_index_offset = lfo_pm_table[ fnum_lfo + pms + OPN->LFO_PM ];

	block_fnum = block_fnum*2 + lfo_fn_table_index_offset;

	if (lfo_fn_table_index_offset)    /* LFO phase modulation active */
	{
		UINT8 blk = (block_fnum&0x7000) >> 12;
		UINT32 fn  = block_fnum & 0xfff;

		/* recalculate keyscale code */
		int kc = (blk<<2) | opn_fktable[(fn >> 7) & 0xf];

		/* recalculate (frequency) phase increment counter */
		int fc = (OPN->fn_table[fn]>>(7-blk)) + SLOT->DT[kc];

		/* (frequency) phase overflow (credits to Nemesis) */
		if (fc < 0) fc += OPN->fn_max;

		/* update phase */
		SLOT->phase += (fc * SLOT->mul) >> 1;
	}
	else    /* LFO phase modulation  = zero */
	{
		SLOT->phase += SLOT->Incr;
	}
}

INLINE void update_phase_lfo_channel(fm2612_FM_OPN *OPN, fm2612_FM_CH *CH)
{
	UINT32 block_fnum = CH->block_fnum;

	UINT32 fnum_lfo  = ((block_fnum & 0x7f0) >> 4) * 32 * 8;
	INT32  lfo_fn_table_index_offset = lfo_pm_table[ fnum_lfo + CH->pms + OPN->LFO_PM ];

	block_fnum = block_fnum*2 + lfo_fn_table_index_offset;

	if (lfo_fn_table_index_offset)    /* LFO phase modulation active */
	{
		UINT8 blk = (block_fnum&0x7000) >> 12;
		UINT32 fn  = block_fnum & 0xfff;

		/* recalculate keyscale code */
		int kc = (blk<<2) | opn_fktable[(fn >> 7) & 0xf];

		/* recalculate (frequency) phase increment counter */
		int fc = (OPN->fn_table[fn]>>(7-blk));

		/* (frequency) phase overflow (credits to Nemesis) */
		int finc = fc + CH->SLOT[SLOT1].DT[kc];
		if (finc < 0) finc += OPN->fn_max;
		CH->SLOT[SLOT1].phase += (finc*CH->SLOT[SLOT1].mul) >> 1;

		finc = fc + CH->SLOT[SLOT2].DT[kc];
		if (finc < 0) finc += OPN->fn_max;
		CH->SLOT[SLOT2].phase += (finc*CH->SLOT[SLOT2].mul) >> 1;

		finc = fc + CH->SLOT[SLOT3].DT[kc];
		if (finc < 0) finc += OPN->fn_max;
		CH->SLOT[SLOT3].phase += (finc*CH->SLOT[SLOT3].mul) >> 1;

		finc = fc + CH->SLOT[SLOT4].DT[kc];
		if (finc < 0) finc += OPN->fn_max;
		CH->SLOT[SLOT4].phase += (finc*CH->SLOT[SLOT4].mul) >> 1;
	}
	else    /* LFO phase modulation  = zero */
	{
			CH->SLOT[SLOT1].phase += CH->SLOT[SLOT1].Incr;
			CH->SLOT[SLOT2].phase += CH->SLOT[SLOT2].Incr;
			CH->SLOT[SLOT3].phase += CH->SLOT[SLOT3].Incr;
			CH->SLOT[SLOT4].phase += CH->SLOT[SLOT4].Incr;
	}
}

/* update phase increment and envelope generator */
INLINE void refresh_fc_eg_slot(fm2612_FM_OPN *OPN, fm2612_FM_SLOT *SLOT , int fc , int kc )
{
	int ksr = kc >> SLOT->KSR;

	fc += SLOT->DT[kc];

	/* detects frequency overflow (credits to Nemesis) */
	if (fc < 0) fc += OPN->fn_max;

	/* (frequency) phase increment counter */
	SLOT->Incr = (fc * SLOT->mul) >> 1;

	if( SLOT->ksr != ksr )
	{
		SLOT->ksr = ksr;

		/* calculate envelope generator rates */
		if ((SLOT->ar + SLOT->ksr) < 32+62)
		{
			SLOT->eg_sh_ar  = eg_rate_shift [SLOT->ar  + SLOT->ksr ];
			SLOT->eg_sel_ar = eg_rate_select2612[SLOT->ar  + SLOT->ksr ];
		}
		else
		{
			SLOT->eg_sh_ar  = 0;
			SLOT->eg_sel_ar = 18*RATE_STEPS; /* verified by Nemesis on real hardware (Attack phase is blocked) */
		}

		SLOT->eg_sh_d1r = eg_rate_shift [SLOT->d1r + SLOT->ksr];
		SLOT->eg_sh_d2r = eg_rate_shift [SLOT->d2r + SLOT->ksr];
		SLOT->eg_sh_rr  = eg_rate_shift [SLOT->rr  + SLOT->ksr];

		SLOT->eg_sel_d1r= eg_rate_select2612[SLOT->d1r + SLOT->ksr];
		SLOT->eg_sel_d2r= eg_rate_select2612[SLOT->d2r + SLOT->ksr];
		SLOT->eg_sel_rr = eg_rate_select2612[SLOT->rr  + SLOT->ksr];
	}
}

/* update phase increment counters */
/* Changed from INLINE to static to work around gcc 4.2.1 codegen bug */
static void refresh_fc_eg_chan(fm2612_FM_OPN *OPN, fm2612_FM_CH *CH )
{
	if( CH->SLOT[SLOT1].Incr==-1)
	{
		int fc = CH->fc;
		int kc = CH->kcode;
		refresh_fc_eg_slot(OPN, &CH->SLOT[SLOT1] , fc , kc );
		refresh_fc_eg_slot(OPN, &CH->SLOT[SLOT2] , fc , kc );
		refresh_fc_eg_slot(OPN, &CH->SLOT[SLOT3] , fc , kc );
		refresh_fc_eg_slot(OPN, &CH->SLOT[SLOT4] , fc , kc );
	}
}

#define volume_calc(OP) ((OP)->vol_out + (AM & (OP)->AMmask))

INLINE signed int op_calc(UINT32 phase, unsigned int env, signed int pm)
{
	UINT32 p;

	p = (env<<3) + sin_tab[ ( ((signed int)((phase & ~FREQ_MASK) + (pm<<15))) >> FREQ_SH ) & SIN_MASK ];

	if (p >= TL_TAB_LEN)
	return 0;
	return tl_tab[p];
}

INLINE signed int op_calc1(UINT32 phase, unsigned int env, signed int pm)
{
	UINT32 p;

	p = (env<<3) + sin_tab[ ( ((signed int)((phase & ~FREQ_MASK) + pm      )) >> FREQ_SH ) & SIN_MASK ];

	if (p >= TL_TAB_LEN)
	return 0;
	return tl_tab[p];
}

INLINE void chan_calc(YM2612 *F2612, fm2612_FM_OPN *OPN, fm2612_FM_CH *CH)
{
	UINT32 AM = OPN->LFO_AM >> CH->ams;
	unsigned int eg_out = volume_calc(&CH->SLOT[SLOT1]);

	OPN->m2 = OPN->c1 = OPN->c2 = OPN->mem = 0;

	*CH->mem_connect = CH->mem_value;  /* restore delayed sample (MEM) value to m2 or c2 */

	{
	INT32 out = CH->op1_out[0] + CH->op1_out[1];
	CH->op1_out[0] = CH->op1_out[1];

	if( !CH->connect1 )
	{
		/* algorithm 5  */
		OPN->mem = OPN->c1 = OPN->c2 = CH->op1_out[0];
	}
	else
	{
		/* other algorithms */
		*CH->connect1 += CH->op1_out[0];
	}


	CH->op1_out[1] = 0;
	if( eg_out < ENV_QUIET )  /* SLOT 1 */
	{
		if (!CH->FB)
		out=0;

		CH->op1_out[1] = op_calc1(CH->SLOT[SLOT1].phase, eg_out, (out<<CH->FB) );
	}
	}

	eg_out = volume_calc(&CH->SLOT[SLOT3]);
	if( eg_out < ENV_QUIET )    /* SLOT 3 */
	*CH->connect3 += op_calc(CH->SLOT[SLOT3].phase, eg_out, OPN->m2);

	eg_out = volume_calc(&CH->SLOT[SLOT2]);
	if( eg_out < ENV_QUIET )    /* SLOT 2 */
	*CH->connect2 += op_calc(CH->SLOT[SLOT2].phase, eg_out, OPN->c1);

	eg_out = volume_calc(&CH->SLOT[SLOT4]);
	if( eg_out < ENV_QUIET )    /* SLOT 4 */
	*CH->connect4 += op_calc(CH->SLOT[SLOT4].phase, eg_out, OPN->c2);


	/* store current MEM */
	CH->mem_value = OPN->mem;

	/* update phase counters AFTER output calculations */
	if(CH->pms)
	{
	/* add support for 3 slot mode */
	if ((OPN->ST.mode & 0xC0) && (CH == &F2612->CH[2]))
	{
		update_phase_lfo_slot(OPN, &CH->SLOT[SLOT1], CH->pms, OPN->SL3.block_fnum[1]);
		update_phase_lfo_slot(OPN, &CH->SLOT[SLOT2], CH->pms, OPN->SL3.block_fnum[2]);
		update_phase_lfo_slot(OPN, &CH->SLOT[SLOT3], CH->pms, OPN->SL3.block_fnum[0]);
		update_phase_lfo_slot(OPN, &CH->SLOT[SLOT4], CH->pms, CH->block_fnum);
	}
	else update_phase_lfo_channel(OPN, CH);
	}
	else  /* no LFO phase modulation */
	{
	CH->SLOT[SLOT1].phase += CH->SLOT[SLOT1].Incr;
	CH->SLOT[SLOT2].phase += CH->SLOT[SLOT2].Incr;
	CH->SLOT[SLOT3].phase += CH->SLOT[SLOT3].Incr;
	CH->SLOT[SLOT4].phase += CH->SLOT[SLOT4].Incr;
	}
}

static void FMCloseTable( void )
{
#ifdef SAVE_SAMPLE
	fclose(sample[0]);
#endif
	return;
}


/* CSM Key Controll */
INLINE void CSMKeyControll(fm2612_FM_OPN *OPN, fm2612_FM_CH *CH)
{
	/* all key ON (verified by Nemesis on real hardware) */
	FM_KEYON_CSM(OPN,CH,SLOT1);
	FM_KEYON_CSM(OPN,CH,SLOT2);
	FM_KEYON_CSM(OPN,CH,SLOT3);
	FM_KEYON_CSM(OPN,CH,SLOT4);
	OPN->SL3.key_csm = 1;
}

#ifdef __SAVE_H__
/* FM channel save , internal state only */
static void FMsave_state_channel(device_t *device,fm2612_FM_CH *CH,int num_ch)
{
	int slot , ch;

	for(ch=0;ch<num_ch;ch++,CH++)
	{
		/* channel */
		device->save_item(NAME(CH->op1_out), ch);
		device->save_item(NAME(CH->fc), ch);
		/* slots */
		for(slot=0;slot<4;slot++)
		{
			fm2612_FM_SLOT *SLOT = &CH->SLOT[slot];
			device->save_item(NAME(SLOT->phase), ch * 4 + slot);
			device->save_item(NAME(SLOT->state), ch * 4 + slot);
			device->save_item(NAME(SLOT->volume), ch * 4 + slot);
		}
	}
}

static void FMsave_state_st(device_t *device,fm2612_FM_ST *ST)
{
#if FM_BUSY_FLAG_SUPPORT
	device->save_item(NAME(ST->busy_expiry_time) );
#endif
	device->save_item(NAME(ST->address) );
	device->save_item(NAME(ST->irq)     );
	device->save_item(NAME(ST->irqmask) );
	device->save_item(NAME(ST->status)  );
	device->save_item(NAME(ST->mode)    );
	device->save_item(NAME(ST->prescaler_sel) );
	device->save_item(NAME(ST->fn_h) );
	device->save_item(NAME(ST->TA)   );
	device->save_item(NAME(ST->TAC)  );
	device->save_item(NAME(ST->TB)  );
	device->save_item(NAME(ST->TBC)  );
}
#endif /* _STATE_H */

#if BUILD_OPN
/* write a OPN mode register 0x20-0x2f */
static void OPNWriteMode(fm2612_FM_OPN *OPN, int r, int v)
{
	UINT8 c;
	fm2612_FM_CH *CH;

	switch(r)
	{
	case 0x21:  /* Test */
		break;
	case 0x22:  /* LFO FREQ (YM2608/YM2610/YM2610B/YM2612) */
		if (v&8) /* LFO enabled ? */
		{
			OPN->lfo_timer_overflow = lfo_samples_per_step[v&7] << LFO_SH;
		}
		else
		{
			/* hold LFO waveform in reset state */
			OPN->lfo_timer_overflow = 0;
			OPN->lfo_timer = 0;
			OPN->lfo_cnt   = 0;
			OPN->LFO_PM    = 0;
			OPN->LFO_AM    = 126;
		}
		break;
	case 0x24:  /* timer A High 8*/
		OPN->ST.TA = (OPN->ST.TA & 0x03)|(((int)v)<<2);
		break;
	case 0x25:  /* timer A Low 2*/
		OPN->ST.TA = (OPN->ST.TA & 0x3fc)|(v&3);
		break;
	case 0x26:  /* timer B */
		OPN->ST.TB = v;
		break;
	case 0x27:  /* mode, timer control */
		set_timers( OPN, &(OPN->ST),OPN->ST.param,v );
		break;
	case 0x28:  /* key on / off */
		c = v & 0x03;
		if( c == 3 ) break;
		if( (v&0x04) && (OPN->type & TYPE_6CH) ) c+=3;
		CH = OPN->P_CH;
		CH = &CH[c];
		if(v&0x10) FM_KEYON(OPN,CH,SLOT1); else FM_KEYOFF(OPN,CH,SLOT1);
		if(v&0x20) FM_KEYON(OPN,CH,SLOT2); else FM_KEYOFF(OPN,CH,SLOT2);
		if(v&0x40) FM_KEYON(OPN,CH,SLOT3); else FM_KEYOFF(OPN,CH,SLOT3);
		if(v&0x80) FM_KEYON(OPN,CH,SLOT4); else FM_KEYOFF(OPN,CH,SLOT4);
		break;
	}
}

/* write a OPN register (0x30-0xff) */
static void OPNWriteReg(fm2612_FM_OPN *OPN, int r, int v)
{
	fm2612_FM_CH *CH;
	fm2612_FM_SLOT *SLOT;

	UINT8 c = OPN_CHAN(r);

	if (c == 3) return; /* 0xX3,0xX7,0xXB,0xXF */

	if (r >= 0x100) c+=3;

	CH = OPN->P_CH;
	CH = &CH[c];

	SLOT = &(CH->SLOT[OPN_SLOT(r)]);

	switch( r & 0xf0 ) {
	case 0x30:  /* DET , MUL */
		set_det_mul(&OPN->ST,CH,SLOT,v);
		break;

	case 0x40:  /* TL */
		set_tl(CH,SLOT,v);
		break;

	case 0x50:  /* KS, AR */
		set_ar_ksr(OPN->type,CH,SLOT,v);
		break;

	case 0x60:  /* bit7 = AM ENABLE, DR */
		set_dr(OPN->type, SLOT,v);

		if(OPN->type & TYPE_LFOPAN) /* YM2608/2610/2610B/2612 */
		{
			SLOT->AMmask = (v&0x80) ? ~0 : 0;
		}
		break;

	case 0x70:  /*     SR */
		set_sr(OPN->type,SLOT,v);
		break;

	case 0x80:  /* SL, RR */
		set_sl_rr(OPN->type,SLOT,v);
		break;

	case 0x90:  /* SSG-EG */
		SLOT->ssg  =  v&0x0f;

			/* recalculate EG output */
		if ((SLOT->ssg&0x08) && (SLOT->ssgn ^ (SLOT->ssg&0x04)) && (SLOT->state > EG_REL))
			SLOT->vol_out = ((UINT32)(0x200 - SLOT->volume) & MAX_ATT_INDEX) + SLOT->tl;
		else
			SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;

		/* SSG-EG envelope shapes :

		E AtAlH
		1 0 0 0  \\\\

		1 0 0 1  \___

		1 0 1 0  \/\/
		          ___
		1 0 1 1  \

		1 1 0 0  ////
		          ___
		1 1 0 1  /

		1 1 1 0  /\/\

		1 1 1 1  /___


		E = SSG-EG enable


		The shapes are generated using Attack, Decay and Sustain phases.

		Each single character in the diagrams above represents this whole
		sequence:

		- when KEY-ON = 1, normal Attack phase is generated (*without* any
		  difference when compared to normal mode),

		- later, when envelope level reaches minimum level (max volume),
		  the EG switches to Decay phase (which works with bigger steps
		  when compared to normal mode - see below),

		- later when envelope level passes the SL level,
		  the EG swithes to Sustain phase (which works with bigger steps
		  when compared to normal mode - see below),

		- finally when envelope level reaches maximum level (min volume),
		  the EG switches to Attack phase again (depends on actual waveform).

		Important is that when switch to Attack phase occurs, the phase counter
		of that operator will be zeroed-out (as in normal KEY-ON) but not always.
		(I havent found the rule for that - perhaps only when the output level is low)

		The difference (when compared to normal Envelope Generator mode) is
		that the resolution in Decay and Sustain phases is 4 times lower;
		this results in only 256 steps instead of normal 1024.
		In other words:
		when SSG-EG is disabled, the step inside of the EG is one,
		when SSG-EG is enabled, the step is four (in Decay and Sustain phases).

		Times between the level changes are the same in both modes.


		Important:
		Decay 1 Level (so called SL) is compared to actual SSG-EG output, so
		it is the same in both SSG and no-SSG modes, with this exception:

		when the SSG-EG is enabled and is generating raising levels
		(when the EG output is inverted) the SL will be found at wrong level !!!
		For example, when SL=02:
		    0 -6 = -6dB in non-inverted EG output
		    96-6 = -90dB in inverted EG output
		Which means that EG compares its level to SL as usual, and that the
		output is simply inverted afterall.


		The Yamaha's manuals say that AR should be set to 0x1f (max speed).
		That is not necessary, but then EG will be generating Attack phase.

		*/


		break;

	case 0xa0:
		switch( OPN_SLOT(r) )
		{
		case 0:     /* 0xa0-0xa2 : FNUM1 */
			{
				UINT32 fn = (((UINT32)( (OPN->ST.fn_h)&7))<<8) + v;
				UINT8 blk = OPN->ST.fn_h>>3;
				/* keyscale code */
				CH->kcode = (blk<<2) | opn_fktable[(fn >> 7) & 0xf];
				/* phase increment counter */
				CH->fc = OPN->fn_table[fn*2]>>(7-blk);

				/* store fnum in clear form for LFO PM calculations */
				CH->block_fnum = (blk<<11) | fn;

				CH->SLOT[SLOT1].Incr=-1;
			}
			break;
		case 1:     /* 0xa4-0xa6 : FNUM2,BLK */
			OPN->ST.fn_h = v&0x3f;
			break;
		case 2:     /* 0xa8-0xaa : 3CH FNUM1 */
			if(r < 0x100)
			{
				UINT32 fn = (((UINT32)(OPN->SL3.fn_h&7))<<8) + v;
				UINT8 blk = OPN->SL3.fn_h>>3;
				/* keyscale code */
				OPN->SL3.kcode[c]= (blk<<2) | opn_fktable[(fn >> 7) & 0xf];
				/* phase increment counter */
				OPN->SL3.fc[c] = OPN->fn_table[fn*2]>>(7-blk);
				OPN->SL3.block_fnum[c] = (blk<<11) | fn;
				(OPN->P_CH)[2].SLOT[SLOT1].Incr=-1;
			}
			break;
		case 3:     /* 0xac-0xae : 3CH FNUM2,BLK */
			if(r < 0x100)
				OPN->SL3.fn_h = v&0x3f;
			break;
		}
		break;

	case 0xb0:
		switch( OPN_SLOT(r) )
		{
		case 0:     /* 0xb0-0xb2 : FB,ALGO */
			{
				int feedback = (v>>3)&7;
				CH->ALGO = v&7;
				CH->FB   = feedback ? feedback+6 : 0;
				setup_connection( OPN, CH, c );
			}
			break;
		case 1:     /* 0xb4-0xb6 : L , R , AMS , PMS (YM2612/YM2610B/YM2610/YM2608) */
			if( OPN->type & TYPE_LFOPAN)
			{
				/* b0-2 PMS */
				CH->pms = (v & 7) * 32; /* CH->pms = PM depth * 32 (index in lfo_pm_table) */

				/* b4-5 AMS */
				CH->ams = lfo_ams_depth_shift[(v>>4) & 0x03];

				/* PAN :  b7 = L, b6 = R */
				OPN->pan[ c*2   ] = (v & 0x80) ? ~0 : 0;
				OPN->pan[ c*2+1 ] = (v & 0x40) ? ~0 : 0;

			}
			break;
		}
		break;
	}
}

/* initialize time tables */
static void init_timetables(fm2612_FM_OPN *OPN, double freqbase)
{
	int i,d;
	double rate;

	/* DeTune table */
	for (d = 0;d <= 3;d++)
	{
		for (i = 0;i <= 31;i++)
		{
			rate = ((double)dt_tab[d*32 + i]) * freqbase * (1<<(FREQ_SH-10)); /* -10 because chip works with 10.10 fixed point, while we use 16.16 */
			OPN->ST.dt_tab[d][i]   = (INT32) rate;
			OPN->ST.dt_tab[d+4][i] = -OPN->ST.dt_tab[d][i];
		}
	}

	/* there are 2048 FNUMs that can be generated using FNUM/BLK registers
	but LFO works with one more bit of a precision so we really need 4096 elements */
	/* calculate fnumber -> increment counter table */
	for(i = 0; i < 4096; i++)
	{
		/* freq table for octave 7 */
		/* OPN phase increment counter = 20bit */
		/* the correct formula is : F-Number = (144 * fnote * 2^20 / M) / 2^(B-1) */
		/* where sample clock is  M/144 */
		/* this means the increment value for one clock sample is FNUM * 2^(B-1) = FNUM * 64 for octave 7 */
		/* we also need to handle the ratio between the chip frequency and the emulated frequency (can be 1.0)  */
		OPN->fn_table[i] = (UINT32)( (double)i * 32 * freqbase * (1<<(FREQ_SH-10)) ); /* -10 because chip works with 10.10 fixed point, while we use 16.16 */
	}

	/* maximal frequency is required for Phase overflow calculation, register size is 17 bits (Nemesis) */
	OPN->fn_max = (UINT32)( (double)0x20000 * freqbase * (1<<(FREQ_SH-10)) );
}

/* prescaler set (and make time tables) */
static void OPNSetPres(fm2612_FM_OPN *OPN, int pres, int timer_prescaler, int SSGpres)
{
	/* frequency base */
	OPN->ST.freqbase = (OPN->ST.rate) ? ((double)OPN->ST.clock / OPN->ST.rate) / pres : 0;

	/* EG is updated every 3 samples */
	OPN->eg_timer_add  = (UINT32)((1<<EG_SH) * OPN->ST.freqbase);
	OPN->eg_timer_overflow = ( 3 ) * (1<<EG_SH);

	/* LFO timer increment (every samples) */
	OPN->lfo_timer_add  = (UINT32)((1<<LFO_SH) * OPN->ST.freqbase);

	/* Timer base time */
	OPN->ST.timer_prescaler = timer_prescaler;

	/* SSG part  prescaler set */
	if( SSGpres ) (*OPN->ST.SSG->set_clock)( OPN->ST.param, OPN->ST.clock * 2 / SSGpres );

	/* make time tables */
	init_timetables(OPN, OPN->ST.freqbase);
}

static void reset_channels(fm2612_FM_ST *ST , fm2612_FM_CH *CH , int num)
{
	int c,s;

	for( c = 0 ; c < num ; c++ )
	{
		CH[c].fc = 0;
		for(s = 0 ; s < 4 ; s++ )
		{
			CH[c].SLOT[s].ssg = 0;
			CH[c].SLOT[s].ssgn = 0;
			CH[c].SLOT[s].state= EG_OFF;
			CH[c].SLOT[s].volume = MAX_ATT_INDEX;
			CH[c].SLOT[s].vol_out= MAX_ATT_INDEX;
		}
	}
}

/* initialize generic tables */
static void init_tables(void)
{
	signed int i,x;
	signed int n;
	double o,m;

	/* build Linear Power Table */
	for (x=0; x<TL_RES_LEN; x++)
	{
		m = (1<<16) / pow(2, (x+1) * (ENV_STEP/4.0) / 8.0);
		m = floor(m);

		/* we never reach (1<<16) here due to the (x+1) */
		/* result fits within 16 bits at maximum */

		n = (int)m;     /* 16 bits here */
		n >>= 4;        /* 12 bits here */
		if (n&1)        /* round to nearest */
			n = (n>>1)+1;
		else
			n = n>>1;
						/* 11 bits here (rounded) */
		n <<= 2;        /* 13 bits here (as in real chip) */


		/* 14 bits (with sign bit) */
		tl_tab[ x*2 + 0 ] = n;
		tl_tab[ x*2 + 1 ] = -tl_tab[ x*2 + 0 ];

		/* one entry in the 'Power' table use the following format, xxxxxyyyyyyyys with:            */
		/*        s = sign bit                                                                      */
		/* yyyyyyyy = 8-bits decimal part (0-TL_RES_LEN)                                            */
		/* xxxxx    = 5-bits integer 'shift' value (0-31) but, since Power table output is 13 bits, */
		/*            any value above 13 (included) would be discarded.                             */
		for (i=1; i<13; i++)
		{
			tl_tab[ x*2+0 + i*2*TL_RES_LEN ] =  tl_tab[ x*2+0 ]>>i;
			tl_tab[ x*2+1 + i*2*TL_RES_LEN ] = -tl_tab[ x*2+0 + i*2*TL_RES_LEN ];
		}
	}

	/* build Logarithmic Sinus table */
	for (i=0; i<SIN_LEN; i++)
	{
		/* non-standard sinus */
		m = sin( ((i*2)+1) * M_PI / SIN_LEN ); /* checked against the real chip */
		/* we never reach zero here due to ((i*2)+1) */

		if (m>0.0)
			o = 8*log(1.0/m)/log(2.0);  /* convert to 'decibels' */
		else
			o = 8*log(-1.0/m)/log(2.0); /* convert to 'decibels' */

		o = o / (ENV_STEP/4);

		n = (int)(2.0*o);
		if (n&1)            /* round to nearest */
			n = (n>>1)+1;
		else
			n = n>>1;

		/* 13-bits (8.5) value is formatted for above 'Power' table */
		sin_tab[ i ] = n*2 + (m>=0.0? 0: 1 );
	}

	/* build LFO PM modulation table */
	for(i = 0; i < 8; i++) /* 8 PM depths */
	{
		UINT8 fnum;
		for (fnum=0; fnum<128; fnum++) /* 7 bits meaningful of F-NUMBER */
		{
			UINT8 value;
			UINT8 step;
			UINT32 offset_depth = i;
			UINT32 offset_fnum_bit;
			UINT32 bit_tmp;

			for (step=0; step<8; step++)
			{
				value = 0;
				for (bit_tmp=0; bit_tmp<7; bit_tmp++) /* 7 bits */
				{
					if (fnum & (1<<bit_tmp)) /* only if bit "bit_tmp" is set */
					{
						offset_fnum_bit = bit_tmp * 8;
						value += lfo_pm_output[offset_fnum_bit + offset_depth][step];
					}
				}
				/* 32 steps for LFO PM (sinus) */
				lfo_pm_table[(fnum*32*8) + (i*32) + step   + 0] = value;
				lfo_pm_table[(fnum*32*8) + (i*32) +(step^7)+ 8] = value;
				lfo_pm_table[(fnum*32*8) + (i*32) + step   +16] = -value;
				lfo_pm_table[(fnum*32*8) + (i*32) +(step^7)+24] = -value;
			}

		}
	}

#ifdef SAVE_SAMPLE
	sample[0]=fopen("sampsum.pcm","wb");
#endif
}

#endif /* BUILD_OPN */

#if (BUILD_YM2612||BUILD_YM3438)
/*******************************************************************************/
/*      YM2612 local section                                                   */
/*******************************************************************************/

/* Generate samples for one of the YM2612s */
void ym2612_update_one(void *chip, FMSAMPLE **buffer, int length)
{
	YM2612 *F2612 = (YM2612 *)chip;
	fm2612_FM_OPN *OPN   = &F2612->OPN;
	INT32 *out_fm = OPN->out_fm;
	int i;
	FMSAMPLE  *bufL,*bufR;
	fm2612_FM_CH   *cch[6];
	int lt,rt;

	/* set bufer */
	bufL = buffer[0];
	bufR = buffer[1];

	cch[0]   = &F2612->CH[0];
	cch[1]   = &F2612->CH[1];
	cch[2]   = &F2612->CH[2];
	cch[3]   = &F2612->CH[3];
	cch[4]   = &F2612->CH[4];
	cch[5]   = &F2612->CH[5];

	/* refresh PG and EG */
	refresh_fc_eg_chan( OPN, cch[0] );
	refresh_fc_eg_chan( OPN, cch[1] );
	if( (OPN->ST.mode & 0xc0) )
	{
		/* 3SLOT MODE */
		if( cch[2]->SLOT[SLOT1].Incr==-1)
		{
			refresh_fc_eg_slot(OPN, &cch[2]->SLOT[SLOT1] , OPN->SL3.fc[1] , OPN->SL3.kcode[1] );
			refresh_fc_eg_slot(OPN, &cch[2]->SLOT[SLOT2] , OPN->SL3.fc[2] , OPN->SL3.kcode[2] );
			refresh_fc_eg_slot(OPN, &cch[2]->SLOT[SLOT3] , OPN->SL3.fc[0] , OPN->SL3.kcode[0] );
			refresh_fc_eg_slot(OPN, &cch[2]->SLOT[SLOT4] , cch[2]->fc , cch[2]->kcode );
		}
	}else refresh_fc_eg_chan( OPN, cch[2] );
	refresh_fc_eg_chan( OPN, cch[3] );
	refresh_fc_eg_chan( OPN, cch[4] );
	refresh_fc_eg_chan( OPN, cch[5] );

	/* buffering */
	for(i=0; i < length ; i++)
	{
		/* clear outputs */
		out_fm[0] = 0;
		out_fm[1] = 0;
		out_fm[2] = 0;
		out_fm[3] = 0;
		out_fm[4] = 0;
		out_fm[5] = 0;

		/* update SSG-EG output */
		update_ssg_eg_channel(&cch[0]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[1]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[2]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[3]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[4]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[5]->SLOT[SLOT1]);

		/* calculate FM */
		chan_calc(F2612, OPN, cch[0]);
		chan_calc(F2612, OPN, cch[1]);
		chan_calc(F2612, OPN, cch[2]);
		chan_calc(F2612, OPN, cch[3]);
		chan_calc(F2612, OPN, cch[4]);
		if( F2612->dacen )
			*cch[5]->connect4 += F2612->dacout;
		else
			chan_calc(F2612, OPN, cch[5]);

		/* advance LFO */
		advance_lfo(OPN);

		/* advance envelope generator */
		OPN->eg_timer += OPN->eg_timer_add;
		while (OPN->eg_timer >= OPN->eg_timer_overflow)
		{
			OPN->eg_timer -= OPN->eg_timer_overflow;
			OPN->eg_cnt++;

			advance_eg_channel(OPN, &cch[0]->SLOT[SLOT1]);
			advance_eg_channel(OPN, &cch[1]->SLOT[SLOT1]);
			advance_eg_channel(OPN, &cch[2]->SLOT[SLOT1]);
			advance_eg_channel(OPN, &cch[3]->SLOT[SLOT1]);
			advance_eg_channel(OPN, &cch[4]->SLOT[SLOT1]);
			advance_eg_channel(OPN, &cch[5]->SLOT[SLOT1]);
		}

		if (out_fm[0] > 8191) out_fm[0] = 8191;
		else if (out_fm[0] < -8192) out_fm[0] = -8192;
		if (out_fm[1] > 8191) out_fm[1] = 8191;
		else if (out_fm[1] < -8192) out_fm[1] = -8192;
		if (out_fm[2] > 8191) out_fm[2] = 8191;
		else if (out_fm[2] < -8192) out_fm[2] = -8192;
		if (out_fm[3] > 8191) out_fm[3] = 8191;
		else if (out_fm[3] < -8192) out_fm[3] = -8192;
		if (out_fm[4] > 8191) out_fm[4] = 8191;
		else if (out_fm[4] < -8192) out_fm[4] = -8192;
		if (out_fm[5] > 8191) out_fm[5] = 8191;
		else if (out_fm[5] < -8192) out_fm[5] = -8192;

		/* 6-channels mixing  */
		lt  = ((out_fm[0]>>0) & OPN->pan[0]);
		rt  = ((out_fm[0]>>0) & OPN->pan[1]);
		lt += ((out_fm[1]>>0) & OPN->pan[2]);
		rt += ((out_fm[1]>>0) & OPN->pan[3]);
		lt += ((out_fm[2]>>0) & OPN->pan[4]);
		rt += ((out_fm[2]>>0) & OPN->pan[5]);
		lt += ((out_fm[3]>>0) & OPN->pan[6]);
		rt += ((out_fm[3]>>0) & OPN->pan[7]);
		lt += ((out_fm[4]>>0) & OPN->pan[8]);
		rt += ((out_fm[4]>>0) & OPN->pan[9]);
		lt += ((out_fm[5]>>0) & OPN->pan[10]);
		rt += ((out_fm[5]>>0) & OPN->pan[11]);

//      Limit( lt, MAXOUT, MINOUT );
//      Limit( rt, MAXOUT, MINOUT );

		#ifdef SAVE_SAMPLE
			SAVE_ALL_CHANNELS
		#endif

		/* buffering */
		bufL[i] = lt;
		bufR[i] = rt;

		/* CSM mode: if CSM Key ON has occurred, CSM Key OFF need to be sent       */
		/* only if Timer A does not overflow again (i.e CSM Key ON not set again) */
		OPN->SL3.key_csm <<= 1;

		/* timer A control */
		INTERNAL_TIMER_A( &OPN->ST , cch[2] )

		/* CSM Mode Key ON still disabled */
		/* CSM Mode Key OFF (verified by Nemesis on real hardware) */
		FM_KEYOFF_CSM(cch[2],SLOT1);
		FM_KEYOFF_CSM(cch[2],SLOT2);
		FM_KEYOFF_CSM(cch[2],SLOT3);
		FM_KEYOFF_CSM(cch[2],SLOT4);
		OPN->SL3.key_csm = 0;
	}

	/* timer B control */
	INTERNAL_TIMER_B(&OPN->ST,length)
}

#ifdef __SAVE_H__
void ym2612_postload(void *chip)
{
	if (chip)
	{
		YM2612 *F2612 = (YM2612 *)chip;
		int r;

		/* DAC data & port */
		F2612->dacout = ((int)F2612->REGS[0x2a] - 0x80) << 6;   /* level unknown */
		F2612->dacen  = F2612->REGS[0x2b] & 0x80;
		/* OPN registers */
		/* DT / MULTI , TL , KS / AR , AMON / DR , SR , SL / RR , SSG-EG */
		for(r=0x30;r<0x9e;r++)
			if((r&3) != 3)
			{
				OPNWriteReg(&F2612->OPN,r,F2612->REGS[r]);
				OPNWriteReg(&F2612->OPN,r|0x100,F2612->REGS[r|0x100]);
			}
		/* FB / CONNECT , L / R / AMS / PMS */
		for(r=0xb0;r<0xb6;r++)
			if((r&3) != 3)
			{
				OPNWriteReg(&F2612->OPN,r,F2612->REGS[r]);
				OPNWriteReg(&F2612->OPN,r|0x100,F2612->REGS[r|0x100]);
			}
		/* channels */
		/*FM_channel_postload(F2612->CH,6);*/
	}
}

static void YM2612_save_state(YM2612 *F2612, device_t *device)
{
	device->save_item(NAME(F2612->REGS));
	FMsave_state_st(device,&F2612->OPN.ST);
	FMsave_state_channel(device,F2612->CH,6);
	/* 3slots */
	device->save_item(NAME(F2612->OPN.SL3.fc));
	device->save_item(NAME(F2612->OPN.SL3.fn_h));
	device->save_item(NAME(F2612->OPN.SL3.kcode));
	/* address register1 */
	device->save_item(NAME(F2612->addr_A1));
}
#endif /* _STATE_H */

/* initialize YM2612 emulator(s) */
void * ym2612_init(void *param, device_t *device, int clock, int rate,
				FM_TIMERHANDLER timer_handler,FM_IRQHANDLER IRQHandler)
{
	YM2612 *F2612;

	/* allocate extend state space */
	F2612 = auto_alloc_clear(device->machine(), YM2612);
	/* allocate total level table (128kb space) */
	init_tables();

	F2612->device = device;
	F2612->OPN.ST.param = param;
	F2612->OPN.type = TYPE_YM2612;
	F2612->OPN.P_CH = F2612->CH;
	F2612->OPN.ST.device = device;
	F2612->OPN.ST.clock = clock;
	F2612->OPN.ST.rate = rate;
	/* F2612->OPN.ST.irq = 0; */
	/* F2612->OPN.ST.status = 0; */
	/* Extend handler */
	F2612->OPN.ST.timer_handler = timer_handler;
	F2612->OPN.ST.IRQ_Handler   = IRQHandler;

#ifdef __SAVE_H__
	YM2612_save_state(F2612, device);
#endif
	return F2612;
}

/* shut down emulator */
void ym2612_shutdown(void *chip)
{
	YM2612 *F2612 = (YM2612 *)chip;

	FMCloseTable();
	auto_free(F2612->OPN.ST.device->machine(), F2612);
}

/* reset one of chip */
void ym2612_reset_chip(void *chip)
{
	int i;
	YM2612 *F2612 = (YM2612 *)chip;
	fm2612_FM_OPN *OPN   = &F2612->OPN;

	OPNSetPres( OPN, 6*24, 6*24, 0);
	/* status clear */
	FM_IRQMASK_SET(&OPN->ST,0x03);
	FM_BUSY_CLEAR(&OPN->ST);
	OPNWriteMode(OPN,0x27,0x30); /* mode 0 , timer reset */

	OPN->eg_timer = 0;
	OPN->eg_cnt   = 0;

	OPN->lfo_timer = 0;
	OPN->lfo_cnt   = 0;
	OPN->LFO_AM    = 126;
	OPN->LFO_PM    = 0;

	OPN->ST.status = 0;
	OPN->ST.mode = 0;

	OPNWriteMode(OPN,0x27,0x30);
	OPNWriteMode(OPN,0x26,0x00);
	OPNWriteMode(OPN,0x25,0x00);
	OPNWriteMode(OPN,0x24,0x00);

	reset_channels( &OPN->ST , &F2612->CH[0] , 6 );

	for(i = 0xb6 ; i >= 0xb4 ; i-- )
	{
		OPNWriteReg(OPN,i      ,0xc0);
		OPNWriteReg(OPN,i|0x100,0xc0);
	}
	for(i = 0xb2 ; i >= 0x30 ; i-- )
	{
		OPNWriteReg(OPN,i      ,0);
		OPNWriteReg(OPN,i|0x100,0);
	}

	/* DAC mode clear */
	F2612->dacen = 0;
	F2612->dacout = 0;
}

/* YM2612 write */
/* n = number  */
/* a = address */
/* v = value   */
int ym2612_write(void *chip, int a, UINT8 v)
{
	YM2612 *F2612 = (YM2612 *)chip;
	int addr;

	v &= 0xff;  /* adjust to 8 bit bus */

	switch( a&3)
	{
	case 0: /* address port 0 */
		F2612->OPN.ST.address = v;
		F2612->addr_A1 = 0;
		break;

	case 1: /* data port 0    */
		if (F2612->addr_A1 != 0)
			break;  /* verified on real YM2608 */

		addr = F2612->OPN.ST.address;
		F2612->REGS[addr] = v;
		switch( addr & 0xf0 )
		{
		case 0x20:  /* 0x20-0x2f Mode */
			switch( addr )
			{
			case 0x2a:  /* DAC data (YM2612) */
				ym2612_update_req(F2612->OPN.ST.param);
				F2612->dacout = ((int)v - 0x80) << 6;   /* level unknown */
				break;
			case 0x2b:  /* DAC Sel  (YM2612) */
				/* b7 = dac enable */
				F2612->dacen = v & 0x80;
				break;
			default:    /* OPN section */
				ym2612_update_req(F2612->OPN.ST.param);
				/* write register */
				OPNWriteMode(&(F2612->OPN),addr,v);
			}
			break;
		default:    /* 0x30-0xff OPN section */
			ym2612_update_req(F2612->OPN.ST.param);
			/* write register */
			OPNWriteReg(&(F2612->OPN),addr,v);
		}
		break;

	case 2: /* address port 1 */
		F2612->OPN.ST.address = v;
		F2612->addr_A1 = 1;
		break;

	case 3: /* data port 1    */
		if (F2612->addr_A1 != 1)
			break;  /* verified on real YM2608 */

		addr = F2612->OPN.ST.address;
		F2612->REGS[addr | 0x100] = v;
		ym2612_update_req(F2612->OPN.ST.param);
		OPNWriteReg(&(F2612->OPN),addr | 0x100,v);
		break;
	}
	return F2612->OPN.ST.irq;
}

UINT8 ym2612_read(void *chip,int a)
{
	YM2612 *F2612 = (YM2612 *)chip;

	switch( a&3)
	{
	case 0: /* status 0 */
		return FM_STATUS_FLAG(&F2612->OPN.ST);
	case 1:
	case 2:
	case 3:
		LOG(F2612->device,LOG_WAR,("YM2612 #%p:A=%d read unmapped area\n",F2612->OPN.ST.param,a));
		return FM_STATUS_FLAG(&F2612->OPN.ST);
	}
	return 0;
}

int ym2612_timer_over(void *chip,int c)
{
	YM2612 *F2612 = (YM2612 *)chip;

	if( c )
	{   /* Timer B */
		TimerBOver( &(F2612->OPN.ST) );
	}
	else
	{   /* Timer A */
		ym2612_update_req(F2612->OPN.ST.param);
		/* timer update */
		TimerAOver( &(F2612->OPN.ST) );
		/* CSM mode key,TL controll */
		if ((F2612->OPN.ST.mode & 0xc0) == 0x80)
		{   /* CSM mode total level latch and auto key on */
			CSMKeyControll( &F2612->OPN, &(F2612->CH[2]) );
		}
	}
	return F2612->OPN.ST.irq;
}

#endif /* (BUILD_YM2612||BUILD_YM3238) */
