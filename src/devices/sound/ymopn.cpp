// license:GPL-2.0+
// copyright-holders:Jarek Burczynski,Tatsuyuki Satoh
//
/*
**
** File: fm.c -- software implementation of Yamaha FM sound generator
**
** Copyright Jarek Burczynski (bujar at mame dot net)
** Copyright Tatsuyuki Satoh , MultiArcadeMachineEmulator development
**
** Version 1.4.2 (final beta)
**
** History:
**
** 2006-2008 Eke-Eke (Genesis Plus GX), MAME backport by R. Belmont.
**  - implemented PG overflow, aka "detune bug" (Ariel, Comix Zone, Shaq Fu, Spiderman,...), credits to Nemesis
**  - fixed SSG-EG support, credits to Nemesis and additional fixes from Alone Coder
**  - modified EG rates and frequency, tested by Nemesis on real hardware
**  - implemented LFO phase update for CH3 special mode (Warlock birds, Alladin bug sound)
**  - fixed Attack Rate update (Batman & Robin intro)
**  - fixed attenuation level at the start of Substain (Gynoug explosions)
**  - fixed EG decay->substain transition to handle special cases, like SL=0 and Decay rate is very slow (Mega Turrican tracks 03,09...)
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

//**********************************************************************
//    comment of hiro-shi(Hiromitsu Shioya)
//    YM2610(B) = OPN-B
//    YM2610  : PSG:3ch FM:4ch ADPCM(18.5KHz):6ch DeltaT ADPCM:1ch
//    YM2610B : PSG:3ch FM:6ch ADPCM(18.5KHz):6ch DeltaT ADPCM:1ch
//**********************************************************************

#include "emu.h"

#define YM2610B_WARNING
#include "fm.h"
#include "ymdeltat.h"
#include "2203intf.h"
#include "2608intf.h"
#include "2610intf.h"


// samples are computed to 16 bits internally; scale by 1/32768 to get 1.0
constexpr stream_buffer::sample_t sample_scale = 1.0 / 32768.0;

// base output clock rate is /72
constexpr int CLOCK_DIVIDER = 72;


constexpr int FREQ_SHIFT = 16; // 16.16 fixed point (frequency calculations)
constexpr int EG_SHIFT = 16;   // 16.16 fixed point (envelope generator timing)
constexpr int LFO_SHIFT = 24;  //  8.24 fixed point (LFO calculations)

constexpr u32 FREQ_MASK = (1 << FREQ_SHIFT) - 1;

constexpr int ENV_BITS = 10;
constexpr u32 ENV_LEN = 1 << ENV_BITS;
constexpr double ENV_STEP = 128.0 / double(ENV_LEN);

constexpr s32 MAX_ATT_INDEX = ENV_LEN - 1; // 1023
constexpr s32 MIN_ATT_INDEX = 0;           // 0

enum : u8
{
	EG_ATT = 4,
	EG_DEC = 3,
	EG_SUS = 2,
	EG_REL = 1,
	EG_OFF = 0
};

constexpr int SIN_BITS = 10;
constexpr u32 SIN_LEN = 1 << SIN_BITS;
constexpr u32 SIN_MASK = SIN_LEN - 1;

// bit0 = Right enable , bit1 = Left enable
#define OUTD_RIGHT  1
#define OUTD_LEFT   2
#define OUTD_CENTER 3

constexpr u8 RATE_STEPS = 8;


constexpr u32 EG_TIMER_OVERFLOW = 3 << EG_SHIFT;


//***************************************************************************
//  STATIC FIXED TABLES
//***************************************************************************

//
// sustain level table (3dB per step)
// bit0, bit1, bit2, bit3, bit4, bit5, bit6
// 1,    2,    4,    8,    16,   32,   64   (value)
// 0.75, 1.5,  3,    6,    12,   24,   48   (dB)
//
// 0 - 15: 0, 3, 6, 9,12,15,18,21,24,27,30,33,36,39,42,93 (dB)
//
#define SC(db) u32(db * (4.0 / ENV_STEP))
static const u32 s_sl_table[16] =
{
	SC( 0), SC( 1), SC( 2), SC( 3), SC( 4), SC( 5), SC( 6), SC( 7),
	SC( 8), SC( 9), SC(10), SC(11), SC(12), SC(13), SC(14), SC(31)
};
#undef SC

//
//
//
static const u8 s_eg_inc[19*RATE_STEPS] =
{
	//cycle:0 1  2 3  4 5  6 7

	/* 0 */ 0,1, 0,1, 0,1, 0,1, // rates 00..11 0 (increment by 0 or 1)
	/* 1 */ 0,1, 0,1, 1,1, 0,1, // rates 00..11 1
	/* 2 */ 0,1, 1,1, 0,1, 1,1, // rates 00..11 2
	/* 3 */ 0,1, 1,1, 1,1, 1,1, // rates 00..11 3

	/* 4 */ 1,1, 1,1, 1,1, 1,1, // rate 12 0 (increment by 1)
	/* 5 */ 1,1, 1,2, 1,1, 1,2, // rate 12 1
	/* 6 */ 1,2, 1,2, 1,2, 1,2, // rate 12 2
	/* 7 */ 1,2, 2,2, 1,2, 2,2, // rate 12 3

	/* 8 */ 2,2, 2,2, 2,2, 2,2, // rate 13 0 (increment by 2)
	/* 9 */ 2,2, 2,4, 2,2, 2,4, // rate 13 1
	/*10 */ 2,4, 2,4, 2,4, 2,4, // rate 13 2
	/*11 */ 2,4, 4,4, 2,4, 4,4, // rate 13 3

	/*12 */ 4,4, 4,4, 4,4, 4,4, // rate 14 0 (increment by 4)
	/*13 */ 4,4, 4,8, 4,4, 4,8, // rate 14 1
	/*14 */ 4,8, 4,8, 4,8, 4,8, // rate 14 2
	/*15 */ 4,8, 8,8, 4,8, 8,8, // rate 14 3

	/*16 */ 8,8, 8,8, 8,8, 8,8, // rates 15 0, 15 1, 15 2, 15 3 (increment by 8)
	/*17 */ 16,16,16,16,16,16,16,16, // rates 15 2, 15 3 for attack
	/*18 */ 0,0, 0,0, 0,0, 0,0, // infinity rates for attack and decay(s)
};


//
// note that there is no O(17) in this table - it's directly in the code
//
#define O(a) (a*RATE_STEPS)
static const u8 s_eg_rate_select[32+64+32] =
{
	// Envelope Generator rates (32 + 64 rates + 32 RKS)
	// 32 infinite time rates
	O(18),O(18),O(18),O(18),O(18),O(18),O(18),O(18),
	O(18),O(18),O(18),O(18),O(18),O(18),O(18),O(18),
	O(18),O(18),O(18),O(18),O(18),O(18),O(18),O(18),
	O(18),O(18),O(18),O(18),O(18),O(18),O(18),O(18),

	// rates 00-11
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
	O( 0),O( 1),O( 2),O( 3),
	O( 0),O( 1),O( 2),O( 3),

	// rate 12
	O( 4),O( 5),O( 6),O( 7),

	// rate 13
	O( 8),O( 9),O(10),O(11),

	// rate 14
	O(12),O(13),O(14),O(15),

	// rate 15
	O(16),O(16),O(16),O(16),

	// 32 dummy rates (same as 15 3)
	O(16),O(16),O(16),O(16),O(16),O(16),O(16),O(16),
	O(16),O(16),O(16),O(16),O(16),O(16),O(16),O(16),
	O(16),O(16),O(16),O(16),O(16),O(16),O(16),O(16),
	O(16),O(16),O(16),O(16),O(16),O(16),O(16),O(16)
};
#undef O

//
// rate  0,    1,    2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
// shift 11,  10,  9,  8,  7,  6,  5,  4,  3,  2, 1,  0,  0,  0,  0,  0
// mask  2047, 1023, 511, 255, 127, 63, 31, 15, 7,  3, 1,  0,  0,  0,  0,  0
//
static const u8 s_eg_rate_shift[32+64+32] =
{
	// Envelope Generator counter shifts (32 + 64 rates + 32 RKS)

	// 32 infinite time rates
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,

	// rates 00-11
	11, 11, 11, 11,
	10, 10, 10, 10,
	 9,  9,  9,  9,
	 8,  8,  8,  8,
	 7,  7,  7,  7,
	 6,  6,  6,  6,
	 5,  5,  5,  5,
	 4,  4,  4,  4,
	 3,  3,  3,  3,
	 2,  2,  2,  2,
	 1,  1,  1,  1,
	 0,  0,  0,  0,

	// rate 12
	0, 0, 0, 0,

	// rate 13
	0, 0, 0, 0,

	// rate 14
	0, 0, 0, 0,

	// rate 15
	0, 0, 0, 0,

	// 32 dummy rates (same as 15 3)
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};


//
// this is YM2151 and YM2612 phase increment data (in 10.10 fixed point format)
//
static const s8 s_detune_table[8*32] =
{
// FD=0
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
// FD=1
	0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2,
	2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 8, 8, 8, 8,
// FD=2
	1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5,
	5, 6, 6, 7, 8, 8, 9,10,11,12,13,14,16,16,16,16,
// FD=3
	2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7,
	8 , 8, 9,10,11,12,13,14,16,17,19,20,22,22,22,22,
// FD=4
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
// FD=5
	0, 0, 0, 0,-1,-1,-1,-1,-1,-1,-1,-1,-2,-2,-2,-2,
	-2,-3,-3,-3,-4,-4,-4,-5,-5,-6,-6,-7,-8,-8,-8,-8,
// FD=6
	-1,-1,-1,-1,-2,-2,-2, -2, -2, -3, -3, -3, -4, -4, -4, -5,
	-5,-6,-6,-7,-8,-8,-9,-10,-11,-12,-13,-14,-16,-16,-16,-16,
// FD=7
	-2,-2,-2, -2, -2, -3, -3, -3, -4, -4, -4, -5, -5, -6, -6, -7,
	-8,-8,-9,-10,-11,-12,-13,-14,-16,-17,-19,-20,-22,-22,-22,-22
};


//
// OPN key frequency number -> key code follow table
// fnum higher 4bit -> keycode lower 2bit
//
static const u8 s_opn_fktable[16] = {0,0,0,0,0,0,0,1,2,3,3,3,3,3,3,3};


//
// 8 LFO speed parameters
// each raw value represents number of samples that one LFO level will last for
//
static const u32 s_lfo_samples_per_step[8] = {108, 77, 71, 67, 62, 44, 8, 5};


//
// There are 4 different LFO AM depths available, they are:
// 0 dB, 1.4 dB, 5.9 dB, 11.8 dB
// Here is how it is generated (in EG steps):
//
// 11.8 dB = 0, 2, 4, 6, 8, 10,12,14,16...126,126,124,122,120,118,....4,2,0
//  5.9 dB = 0, 1, 2, 3, 4, 5, 6, 7, 8....63, 63, 62, 61, 60, 59,.....2,1,0
//  1.4 dB = 0, 0, 0, 0, 1, 1, 1, 1, 2,...15, 15, 15, 15, 14, 14,.....0,0,0
//
// (1.4 dB is losing precision as you can see)
//
// It's implemented as generator from 0..126 with step 2 then a shift
// right N times, where N is:
//   8 for 0 dB
//   3 for 1.4 dB
//   1 for 5.9 dB
//   0 for 11.8 dB
//
static const u8 s_lfo_ams_depth_shift[4] = {8, 3, 1, 0};


//
// There are 8 different LFO PM depths available, they are:
// 0, 3.4, 6.7, 10, 14, 20, 40, 80 (cents)
//
// Modulation level at each depth depends on F-NUMBER bits: 4,5,6,7,8,9,10
// (bits 8,9,10 = FNUM MSB from OCT/FNUM register)
//
// Here we store only first quarter (positive one) of full waveform.
// Full table (lfo_pm_table) containing all 128 waveforms is build
// at run (init) time.
//
// One value in table below represents 4 (four) basic LFO steps
// (1 PM step = 4 AM steps).
//
// For example:
// at LFO SPEED=0 (which is 108 samples per basic LFO step)
// one value from "lfo_pm_output" table lasts for 432 consecutive
// samples (4*108=432) and one full LFO waveform cycle lasts for 13824
// samples (32*432=13824; 32 because we store only a quarter of whole
// 		waveform in the table below)
//
static const u8 s_lfo_pm_output[7*8][8] =
{
	// 7 bits meaningful (of F-NUMBER), 8 LFO output levels per one depth (out of 32), 8 LFO depths
	// FNUM BIT 4: 000 0001xxxx
	/* DEPTH 0 */ {0,   0,   0,   0,   0,   0,   0,   0},
	/* DEPTH 1 */ {0,   0,   0,   0,   0,   0,   0,   0},
	/* DEPTH 2 */ {0,   0,   0,   0,   0,   0,   0,   0},
	/* DEPTH 3 */ {0,   0,   0,   0,   0,   0,   0,   0},
	/* DEPTH 4 */ {0,   0,   0,   0,   0,   0,   0,   0},
	/* DEPTH 5 */ {0,   0,   0,   0,   0,   0,   0,   0},
	/* DEPTH 6 */ {0,   0,   0,   0,   0,   0,   0,   0},
	/* DEPTH 7 */ {0,   0,   0,   0,   1,   1,   1,   1},

	// FNUM BIT 5: 000 0010xxxx
	/* DEPTH 0 */ {0,   0,   0,   0,   0,   0,   0,   0},
	/* DEPTH 1 */ {0,   0,   0,   0,   0,   0,   0,   0},
	/* DEPTH 2 */ {0,   0,   0,   0,   0,   0,   0,   0},
	/* DEPTH 3 */ {0,   0,   0,   0,   0,   0,   0,   0},
	/* DEPTH 4 */ {0,   0,   0,   0,   0,   0,   0,   0},
	/* DEPTH 5 */ {0,   0,   0,   0,   0,   0,   0,   0},
	/* DEPTH 6 */ {0,   0,   0,   0,   1,   1,   1,   1},
	/* DEPTH 7 */ {0,   0,   1,   1,   2,   2,   2,   3},

	// FNUM BIT 6: 000 0100xxxx
	/* DEPTH 0 */ {0,   0,   0,   0,   0,   0,   0,   0},
	/* DEPTH 1 */ {0,   0,   0,   0,   0,   0,   0,   0},
	/* DEPTH 2 */ {0,   0,   0,   0,   0,   0,   0,   0},
	/* DEPTH 3 */ {0,   0,   0,   0,   0,   0,   0,   0},
	/* DEPTH 4 */ {0,   0,   0,   0,   0,   0,   0,   1},
	/* DEPTH 5 */ {0,   0,   0,   0,   1,   1,   1,   1},
	/* DEPTH 6 */ {0,   0,   1,   1,   2,   2,   2,   3},
	/* DEPTH 7 */ {0,   0,   2,   3,   4,   4,   5,   6},

	// FNUM BIT 7: 000 1000xxxx
	/* DEPTH 0 */ {0,   0,   0,   0,   0,   0,   0,   0},
	/* DEPTH 1 */ {0,   0,   0,   0,   0,   0,   0,   0},
	/* DEPTH 2 */ {0,   0,   0,   0,   0,   0,   1,   1},
	/* DEPTH 3 */ {0,   0,   0,   0,   1,   1,   1,   1},
	/* DEPTH 4 */ {0,   0,   0,   1,   1,   1,   1,   2},
	/* DEPTH 5 */ {0,   0,   1,   1,   2,   2,   2,   3},
	/* DEPTH 6 */ {0,   0,   2,   3,   4,   4,   5,   6},
	/* DEPTH 7 */ {0,   0,   4,   6,   8,   8, 0xa, 0xc},

	// FNUM BIT 8: 001 0000xxxx
	/* DEPTH 0 */ {0,   0,   0,   0,   0,   0,   0,   0},
	/* DEPTH 1 */ {0,   0,   0,   0,   1,   1,   1,   1},
	/* DEPTH 2 */ {0,   0,   0,   1,   1,   1,   2,   2},
	/* DEPTH 3 */ {0,   0,   1,   1,   2,   2,   3,   3},
	/* DEPTH 4 */ {0,   0,   1,   2,   2,   2,   3,   4},
	/* DEPTH 5 */ {0,   0,   2,   3,   4,   4,   5,   6},
	/* DEPTH 6 */ {0,   0,   4,   6,   8,   8, 0xa, 0xc},
	/* DEPTH 7 */ {0,   0,   8, 0xc,0x10,0x10,0x14,0x18},

	// FNUM BIT 9: 010 0000xxxx
	/* DEPTH 0 */ {0,   0,   0,   0,   0,   0,   0,   0},
	/* DEPTH 1 */ {0,   0,   0,   0,   2,   2,   2,   2},
	/* DEPTH 2 */ {0,   0,   0,   2,   2,   2,   4,   4},
	/* DEPTH 3 */ {0,   0,   2,   2,   4,   4,   6,   6},
	/* DEPTH 4 */ {0,   0,   2,   4,   4,   4,   6,   8},
	/* DEPTH 5 */ {0,   0,   4,   6,   8,   8, 0xa, 0xc},
	/* DEPTH 6 */ {0,   0,   8, 0xc,0x10,0x10,0x14,0x18},
	/* DEPTH 7 */ {0,   0,0x10,0x18,0x20,0x20,0x28,0x30},

	// FNUM BIT10: 100 0000xxxx
	/* DEPTH 0 */ {0,   0,   0,   0,   0,   0,   0,   0},
	/* DEPTH 1 */ {0,   0,   0,   0,   4,   4,   4,   4},
	/* DEPTH 2 */ {0,   0,   0,   4,   4,   4,   8,   8},
	/* DEPTH 3 */ {0,   0,   4,   4,   8,   8, 0xc, 0xc},
	/* DEPTH 4 */ {0,   0,   4,   8,   8,   8, 0xc,0x10},
	/* DEPTH 5 */ {0,   0,   8, 0xc,0x10,0x10,0x14,0x18},
	/* DEPTH 6 */ {0,   0,0x10,0x18,0x20,0x20,0x28,0x30},
	/* DEPTH 7 */ {0,   0,0x20,0x30,0x40,0x40,0x50,0x60},
};



//***************************************************************************
//  STATIC RUNTIME TABLES
//***************************************************************************

//
//  TL_TAB_LEN is calculated as:
//    13 - sinus amplitude bits     (Y axis)
//     2 - sinus sign bit           (Y axis)
//    TL_RES_LEN - sinus resolution (X axis)
//
constexpr u32 TL_RES_LEN = 256;   // 8 bits addressing (real chip)
constexpr u32 TL_TAB_LEN = 13 * 2 * TL_RES_LEN;
constexpr u32 ENV_QUIET = TL_TAB_LEN / 8;

static s32 *s_tl_table = nullptr;

// sin waveform table in 'decibel' scale
static u32 *s_sin_table = nullptr;

// all 128 LFO PM waveforms
static s32 *s_lfo_pm_table = nullptr;



//***************************************************************************
//  INLINE HELPERS
//***************************************************************************

inline u32 ymopn_device_base::fn_value(u32 val) const
{
	// FREQ_SHIFT-10 because chip works with 10.10 fixed point, while we use 16.16
//	return (val * 32 * m_freqbase) << (FREQ_SHIFT - 10);
	return (val * m_freqbase) << (FREQ_SHIFT - 5);
}

inline u32 ymopn_device_base::fn_max() const
{
	return fn_value(4096);
}

// detects frequency overflow (credits to Nemesis)
inline u32 ymopn_device_base::fc_fix(s32 fc) const
{
	return (fc < 0) ? (fc + fn_max()) : fc;
}

// lookup the detune value, which depends on m_freqbase
inline s32 ymopn_device_base::detune(u8 fd, u8 index) const
{
	// scale factor is: SIN_LEN * (1 << FREQ_SHIFT) / (1 << 20)
	// assumine that SIN_BITS + FREQ_SHIFT > 20, so:
	return (s_detune_table[fd * 32 + index] * m_freqbase) << (SIN_BITS + FREQ_SHIFT - 20);
}

inline u32 ymopn_device_base::lfo_step(u8 index) const
{
	return (m_freqbase << LFO_SHIFT) / s_lfo_samples_per_step[index];
}



//***************************************************************************
//  OPN 3-SLOT STATE
//***************************************************************************

opn_3slot_t::opn_3slot_t() :
	m_fc{0, 0, 0},
	m_fn_h(0),
	m_kcode{0, 0, 0},
	m_block_fnum{0, 0, 0}
{
}

void opn_3slot_t::set_fnum(ymopn_device_base &opn, u8 chnum, u8 value)
{
	u32 fn = ((m_fn_h & 7) << 8) + value;
	u8 blk = m_fn_h >> 3;

	// keyscale code
	m_kcode[chnum] = (blk << 2) | s_opn_fktable[fn >> 7];
	// phase increment counter
	m_fc[chnum] = opn.fn_value(fn * 2) >> (7 - blk);
	// store fnum in clear form for LFO PM calculations
	m_block_fnum[chnum] = (blk << 11) | fn;
}



//***************************************************************************
//  OPN SLOT
//***************************************************************************

// constructor
opn_slot_t::opn_slot_t(ymopn_device_base &opn) :
	m_opn(opn),
	m_detune(0),
	m_ksr_shift(0),
	m_attack_rate(0),
	m_decay_rate(0),
	m_sustain_rate(0),
	m_release_rate(0),
	m_ksr(0),
	m_multiply(0),
	m_phase(0),
	m_phase_step(0),
	m_eg_state(EG_OFF),
	m_total_level(0),
	m_volume(MAX_ATT_INDEX),
	m_sustain_level(0),
	m_envelope_volume(MAX_ATT_INDEX),
	m_attack_shift(0),
	m_attack_select(0),
	m_decay_shift(0),
	m_decay_select(0),
	m_sustain_shift(0),
	m_sustain_select(0),
	m_release_shift(0),
	m_release_select(0),
	m_ssg(0),
	m_ssg_state(0),
	m_key(0),
	m_am_mask(0)
{
}

// register for save states
void opn_slot_t::save(int index)
{
	m_opn.save_item(NAME(m_detune), index);
	m_opn.save_item(NAME(m_ksr_shift), index);
	m_opn.save_item(NAME(m_attack_rate), index);
	m_opn.save_item(NAME(m_decay_rate), index);
	m_opn.save_item(NAME(m_sustain_rate), index);
	m_opn.save_item(NAME(m_release_rate), index);
	m_opn.save_item(NAME(m_ksr), index);
	m_opn.save_item(NAME(m_multiply), index);
	m_opn.save_item(NAME(m_phase), index);
	m_opn.save_item(NAME(m_phase_step), index);
	m_opn.save_item(NAME(m_eg_state), index);
	m_opn.save_item(NAME(m_total_level), index);
	m_opn.save_item(NAME(m_volume), index);
	m_opn.save_item(NAME(m_sustain_level), index);
	m_opn.save_item(NAME(m_envelope_volume), index);
	m_opn.save_item(NAME(m_attack_shift), index);
	m_opn.save_item(NAME(m_attack_select), index);
	m_opn.save_item(NAME(m_decay_shift), index);
	m_opn.save_item(NAME(m_decay_select), index);
	m_opn.save_item(NAME(m_sustain_shift), index);
	m_opn.save_item(NAME(m_sustain_select), index);
	m_opn.save_item(NAME(m_release_shift), index);
	m_opn.save_item(NAME(m_release_select), index);
	m_opn.save_item(NAME(m_ssg), index);
	m_opn.save_item(NAME(m_ssg_state), index);
	m_opn.save_item(NAME(m_key), index);
	m_opn.save_item(NAME(m_am_mask), index);
}

// reset the state
void opn_slot_t::reset()
{
	m_ssg = 0;
	m_ssg_state = 0;
	m_eg_state = EG_OFF;
	m_volume = MAX_ATT_INDEX;
	m_envelope_volume = MAX_ATT_INDEX;
}

// process a key on signal
void opn_slot_t::keyonoff(bool on)
{
	if (on && m_key == 0)
	{
		m_key = 1;
		m_phase = 0;        // restart Phase Generator
		m_ssg_state = BIT(m_ssg, 2) << 1;
		m_eg_state = EG_ATT;
	}
	else if (!on && m_key != 0)
	{
		m_key = 0;
		if (m_eg_state > EG_REL)
			m_eg_state = EG_REL;// phase -> Release
	}
}

// set detune & multiply
void opn_slot_t::set_det_mul(u8 value)
{
	m_multiply = ((value & 0x0f) != 0) ? 2 * (value & 0x0f) : 1;
	m_detune = (value >> 4) & 7;
}

// set attack rate & key scale
bool opn_slot_t::set_ar_ksr(u8 value)
{
	// refresh attack rate
	m_attack_rate = ((value & 0x1f) != 0) ? 32 + 2 * (value & 0x1f) : 0;
	if ((m_attack_rate + m_ksr) < 32 + 62)
	{
		m_attack_shift = s_eg_rate_shift[m_attack_rate + m_ksr];
		m_attack_select = s_eg_rate_select[m_attack_rate + m_ksr];
	}
	else
	{
		m_attack_shift = 0;
		m_attack_select = 17 * RATE_STEPS;
	}

	u8 prev_ksr = m_ksr_shift;
	m_ksr_shift = 3 - (value >> 6);
	return (m_ksr_shift != prev_ksr);
}

// set decay rate
void opn_slot_t::set_dr_am(u8 value)
{
	m_decay_rate = ((value & 0x1f) != 0) ? 32 + 2 * (value & 0x1f) : 0;
	m_decay_shift = s_eg_rate_shift[m_decay_rate + m_ksr];
	m_decay_select = s_eg_rate_select[m_decay_rate + m_ksr];

	if (m_opn.type() != YM2203)
		m_am_mask = BIT(value, 7) ? ~0 : 0;
}

// set sustain rate
void opn_slot_t::set_sr(u8 value)
{
	m_sustain_rate = ((value & 0x1f) != 0) ? 32 + 2 * (value & 0x1f) : 0;
	m_sustain_shift = s_eg_rate_shift[m_sustain_rate + m_ksr];
	m_sustain_select = s_eg_rate_select[m_sustain_rate + m_ksr];
}

// set release rate
void opn_slot_t::set_sl_rr(int value)
{
	m_sustain_level = s_sl_table[value >> 4];
	m_release_rate = 34 + 4 * (value & 0x0f);
	m_release_shift = s_eg_rate_shift[m_release_rate + m_ksr];
	m_release_select = s_eg_rate_select[m_release_rate + m_ksr];
}

// set total level
void opn_slot_t::set_tl(u8 value)
{
	m_total_level = (value & 0x7f) << (ENV_BITS - 7); // 7bit TL
}

// set the envelope shape
void opn_slot_t::set_ssg(u8 value)
{
	// SSG-EG envelope shapes :
	//
	// E AtAlH
	// 1 0 0 0  \\\\
	//
	// 1 0 0 1  \___
	//
	// 1 0 1 0  \/\/
	//           ___
	// 1 0 1 1  \
	//
	// 1 1 0 0  ////
	//           ___
	// 1 1 0 1  /
	//
	// 1 1 1 0  /\/\
	//
	// 1 1 1 1  /___
	//
	// E = SSG-EG enable
	//
	// The shapes are generated using Attack, Decay and Sustain phases.
	//
	// Each single character in the diagrams above represents this whole
	// sequence:
	//
	//   - when KEY-ON = 1, normal Attack phase is generated (*without* any
	//     difference when compared to normal mode),
	//
	//   - later, when envelope level reaches minimum level (max volume),
	//     the EG switches to Decay phase (which works with bigger steps
	//     when compared to normal mode - see below),
	//
	//   - later when envelope level passes the SL level,
	//     the EG swithes to Sustain phase (which works with bigger steps
	//     when compared to normal mode - see below),
	//
	//   - finally when envelope level reaches maximum level (min volume),
	//     the EG switches to Attack phase again (depends on actual waveform).
	//
	// Important is that when switch to Attack phase occurs, the phase counter
	// of that operator will be zeroed-out (as in normal KEY-ON) but not always.
	// (I havent found the rule for that - perhaps only when the output level is low)
	//
	// The difference (when compared to normal Envelope Generator mode) is
	// that the resolution in Decay and Sustain phases is 4 times lower;
	// this results in only 256 steps instead of normal 1024.
	// In other words:
	//  * when SSG-EG is disabled, the step inside of the EG is one,
	//  * when SSG-EG is enabled, the step is four (in Decay and Sustain phases).
	//
	// Times between the level changes are the same in both modes.
	//
	// Important:
	// Decay 1 Level (so called SL) is compared to actual SSG-EG output, so
	// it is the same in both SSG and no-SSG modes, with this exception:
	//
	// when the SSG-EG is enabled and is generating raising levels
	// (when the EG output is inverted) the SL will be found at wrong level !!!
	// For example, when SL=02:
	// 	0 -6 = -6dB in non-inverted EG output
	// 	96-6 = -90dB in inverted EG output
	// Which means that EG compares its level to SL as usual, and that the
	// output is simply inverted afterall.
	//
	// The Yamaha's manuals say that AR should be set to 0x1f (max speed).
	// That is not necessary, but then EG will be generating Attack phase.
	//
	m_ssg = value & 0x0f;
	m_ssg_state = BIT(value, 2) << 1; // bit 1 in ssgn = attack
}

// update phase increment and envelope generator
void opn_slot_t::refresh_fc_eg(int fc, int kc)
{
	// (frequency) phase increment counter
	fc = m_opn.fc_fix(fc + m_opn.detune(m_detune, kc));
	m_phase_step = (fc * m_multiply) >> 1;

	int ksr = kc >> m_ksr_shift;
	if (m_ksr != ksr)
	{
		m_ksr = ksr;

		// calculate envelope generator rates
		if ((m_attack_rate + m_ksr) < 32 + 62)
		{
			m_attack_shift  = s_eg_rate_shift[m_attack_rate + m_ksr];
			m_attack_select = s_eg_rate_select[m_attack_rate + m_ksr];
		}
		else
		{
			m_attack_shift  = 0;
			m_attack_select = 17 * RATE_STEPS;
		}

		m_decay_shift = s_eg_rate_shift[m_decay_rate + m_ksr];
		m_sustain_shift = s_eg_rate_shift[m_sustain_rate + m_ksr];
		m_release_shift = s_eg_rate_shift[m_release_rate + m_ksr];

		m_decay_select = s_eg_rate_select[m_decay_rate + m_ksr];
		m_sustain_select = s_eg_rate_select[m_sustain_rate + m_ksr];
		m_release_select = s_eg_rate_select[m_release_rate + m_ksr];
	}
}

// updat the phase LFO
void opn_slot_t::update_phase_lfo(s32 lfo_pm, s32 pm_shift, u32 block_fnum)
{
	if (pm_shift != 0)
	{
		u32 fnum_lfo = ((block_fnum & 0x7f0) >> 4) * 32 * 8;
		s32 lfo_fn_table_index_offset = s_lfo_pm_table[fnum_lfo + pm_shift + lfo_pm];

		if (lfo_fn_table_index_offset != 0)    // LFO phase modulation active
		{
			block_fnum = block_fnum * 2 + lfo_fn_table_index_offset;

			u8 blk = (block_fnum & 0x7000) >> 12;
			u32 fn = block_fnum & 0xfff;

			// keyscale code
			int kc = (blk << 2) | s_opn_fktable[fn >> 8];

			// phase increment counter
			int fc = m_opn.fc_fix((m_opn.fn_value(fn) >> (7 - blk)) + m_opn.detune(m_detune, kc));

			// update phase and return
			m_phase += (fc * m_multiply) >> 1;
			return;
		}
	}

	// LFO phase modulation  = zero
	m_phase += m_phase_step;
}

// advance the envelope generator
void opn_slot_t::advance_eg(u32 eg_cnt)
{
	// reset SSG-EG swap flag
	u8 swap_flag = 0;

	switch (m_eg_state)
	{
		case EG_ATT:        // attack phase
			if ((eg_cnt & ((1 << m_attack_shift) - 1)) == 0)
			{
				m_volume += (~m_volume * s_eg_inc[m_attack_select + ((eg_cnt >> m_attack_shift) & 7)]) >> 4;
				if (m_volume <= MIN_ATT_INDEX)
				{
					m_volume = MIN_ATT_INDEX;
					m_eg_state = EG_DEC;
				}
			}
			break;

		case EG_DEC:    // decay phase
			if ((eg_cnt & ((1 << m_decay_shift) - 1)) == 0)
			{
				// SSG EG type envelope selected?
				int scale = BIT(m_ssg, 3) ? 4 : 1;
				m_volume += scale * s_eg_inc[m_decay_select + ((eg_cnt >> m_decay_shift) & 7)];

				if (m_volume >= s32(m_sustain_level))
					m_eg_state = EG_SUS;
			}
			break;

		case EG_SUS:    // sustain phase
			if ((eg_cnt & ((1 << m_sustain_shift) - 1)) == 0)
			{
				// SSG EG type envelope selected?
				int scale = BIT(m_ssg, 3) ? 4 : 1;
				m_volume += scale * s_eg_inc[m_sustain_select + ((eg_cnt >> m_sustain_shift) & 7)];

				// special behavior for SSG EG
				if (BIT(m_ssg, 3))
				{
					if (m_volume >= ENV_QUIET)
					{
						m_volume = MAX_ATT_INDEX;

						if (BIT(m_ssg, 0)) // bit 0 = hold
						{
							if (BIT(m_ssg_state, 0))   // have we swapped once ???
								; // yes, so do nothing, just hold current level
							else
								swap_flag = (m_ssg & 2) | 1; // bit 1 = alternate
						}
						else
						{
							// same as KEY-ON operation
							// restart of the Phase Generator should be here
							m_phase = 0;

							// phase -> Attack
							m_volume = 511;
							m_eg_state = EG_ATT;

							swap_flag = (m_ssg & 2) | 0; // bit 1 = alternate
						}
					}
				}
				else
				{
					if (m_volume >= MAX_ATT_INDEX)
					{
						m_volume = MAX_ATT_INDEX;
						// do not change m_eg_state (verified on real chip)
					}
				}
			}
			break;

		case EG_REL:    // release phase
			if ((eg_cnt & ((1 << m_release_shift) - 1)) == 0)
			{
				// SSG-EG affects Release phase also (Nemesis)
				m_volume += s_eg_inc[m_release_select + ((eg_cnt >> m_release_shift) & 7)];

				if (m_volume >= MAX_ATT_INDEX)
				{
					m_volume = MAX_ATT_INDEX;
					m_eg_state = EG_OFF;
				}
			}
			break;
	}

	u32 out = u32(m_volume);

	// negate output (changes come from alternate bit, init comes from attack bit)
	if (BIT(m_ssg, 3) && BIT(m_ssg_state, 1) && m_eg_state > EG_REL)
		out ^= MAX_ATT_INDEX;
	m_ssg_state ^= swap_flag;
	m_envelope_volume = out;
}



//***************************************************************************
//  OPN CHANNEL
//***************************************************************************

// constructor
opn_channel_t::opn_channel_t(ymopn_device_base &opn) :
	m_opn(opn),
	m_refresh(true),
	m_slot1(opn),
	m_slot2(opn),
	m_slot3(opn),
	m_slot4(opn),
	m_algorithm(0),
	m_fb_shift(0),
	m_op1_out{0,0},
	m_connect1(nullptr),
	m_connect3(nullptr),
	m_connect2(nullptr),
	m_connect4(nullptr),
	m_mem_connect(nullptr),
	m_mem_value(0),
	m_pm_shift(0),
	m_am_shift(0),
	m_fc(0),
	m_kcode(0),
	m_block_fnum(0),
	m_m2(0),
	m_c1(0),
	m_c2(0),
	m_mem(0),
	m_out_fm(0),
	m_pan(3),
	m_3slot(nullptr)
{
}

// register for save states
void opn_channel_t::save(int index)
{
	m_slot1.save(index * 4 + 0);
	m_slot2.save(index * 4 + 1);
	m_slot3.save(index * 4 + 2);
	m_slot4.save(index * 4 + 3);

	m_opn.save_item(NAME(m_algorithm), index);
	m_opn.save_item(NAME(m_fb_shift), index);
	m_opn.save_item(NAME(m_op1_out), index);

	m_opn.save_item(NAME(m_mem_value), index);

	m_opn.save_item(NAME(m_pm_shift), index);
	m_opn.save_item(NAME(m_am_shift), index);

	m_opn.save_item(NAME(m_fc), index);
	m_opn.save_item(NAME(m_kcode), index);
	m_opn.save_item(NAME(m_block_fnum), index);

	m_opn.save_item(NAME(m_pan), index);
}

// recover after loading a save state
void opn_channel_t::post_load()
{
	setup_connection();
}

// reset our state and all our owned slots
void opn_channel_t::reset()
{
	m_fc = 0;
	m_slot1.reset();
	m_slot2.reset();
	m_slot3.reset();
	m_slot4.reset();
}

// configure for 3-slot mode
void opn_channel_t::set_three_slot_mode(opn_3slot_t *state)
{
	m_3slot = state;
}

// set the values from fnum
void opn_channel_t::set_fnum(u8 upper, u8 value)
{
	u32 fn = ((upper & 7) << 8) + value;
	u8 blk = upper >> 3;

	// keyscale code
	m_kcode = (blk << 2) | s_opn_fktable[fn >> 7];

	// phase increment counter
	m_fc = m_opn.fn_value(fn * 2) >> (7 - blk);

	// store fnum in clear form for LFO PM calculations
	m_block_fnum = (blk << 11) | fn;
	force_refresh();
}

// set the LFO shift and panning values
void opn_channel_t::set_lfo_shift_pan(u8 value)
{
	// b0-2 PMS
	m_pm_shift = (value & 7) * 32; // CH->pm_shift = PM depth * 32 (index in s_lfo_pm_table)

	// b4-5 AMS
	m_am_shift = s_lfo_ams_depth_shift[(value >> 4) & 3];

	// PAN :  b7 = L, b6 = R
	m_pan = value >> 6;
}

// set algorithm and feedback values
void opn_channel_t::set_algorithm_feedback(u8 value)
{
	u8 feedback = (value >> 3) & 7;
	m_algorithm = value & 7;
	m_fb_shift = (feedback != 0) ? feedback + 6 : 0;
	setup_connection();
}

// refresh the parameters if needed
void opn_channel_t::refresh_fc_eg()
{
	// skip if nothing to do
	if (!m_refresh)
		return;

	if (m_3slot != nullptr)
	{
		m_slot1.refresh_fc_eg(m_3slot->m_fc[1], m_3slot->m_kcode[1]);
		m_slot2.refresh_fc_eg(m_3slot->m_fc[2], m_3slot->m_kcode[2]);
		m_slot3.refresh_fc_eg(m_3slot->m_fc[0], m_3slot->m_kcode[0]);
	}
	else
	{
		m_slot1.refresh_fc_eg(m_fc, m_kcode);
		m_slot2.refresh_fc_eg(m_fc, m_kcode);
		m_slot3.refresh_fc_eg(m_fc, m_kcode);
	}
	m_slot4.refresh_fc_eg(m_fc, m_kcode);
	m_refresh = false;
}

// CSM key control
void opn_channel_t::csm_key_control()
{
	// all key on then off (only for operators which were OFF!)
	if (!m_slot1.key())
	{
		m_slot1.keyonoff(1);
		m_slot1.keyonoff(0);
	}
	if (!m_slot2.key())
	{
		m_slot2.keyonoff(1);
		m_slot2.keyonoff(0);
	}
	if (!m_slot3.key())
	{
		m_slot3.keyonoff(1);
		m_slot3.keyonoff(0);
	}
	if (!m_slot4.key())
	{
		m_slot4.keyonoff(1);
		m_slot4.keyonoff(0);
	}
}

// advance the envelope generator in all our slots
void opn_channel_t::advance_eg(u32 eg_cnt)
{
	m_slot1.advance_eg(eg_cnt);
	m_slot2.advance_eg(eg_cnt);
	m_slot3.advance_eg(eg_cnt);
	m_slot4.advance_eg(eg_cnt);
}

// compute the operator result
inline s32 opn_channel_t::op_calc(u32 phase, u32 env, s32 pm)
{
	u32 p = (env << 3) + s_sin_table[(s32((phase & ~FREQ_MASK) + pm) >> FREQ_SHIFT) & SIN_MASK];
	return (p < TL_TAB_LEN) ? s_tl_table[p] : 0;
}

// update the channel state with the given LFO parameters
void opn_channel_t::update(u32 lfo_am, u32 lfo_pm)
{
	// restore delayed sample (MEM) value to m2 or c2
	*m_mem_connect = m_mem_value;
	m_m2 = m_c1 = m_c2 = m_mem = m_out_fm = 0;

	u32 am = lfo_am >> m_am_shift;
	u32 eg_out = m_slot1.volume(am);
	{
		s32 out = m_op1_out[0] + m_op1_out[1];
		m_op1_out[0] = m_op1_out[1];
		m_op1_out[1] = 0;

		// algorithm 5? (special case)
		if (m_connect1 == nullptr)
			m_mem = m_c1 = m_c2 = m_op1_out[0];
		else
			*m_connect1 += m_op1_out[0];

		if (eg_out < ENV_QUIET)
		{
			if (m_fb_shift == 0)
				out = 0;
			m_op1_out[1] = op_calc(m_slot1.phase(), eg_out, out << m_fb_shift);
		}
	}

	eg_out = m_slot3.volume(am);
	if (eg_out < ENV_QUIET)        // SLOT 3
		*m_connect3 += op_calc(m_slot3.phase(), eg_out, m_m2 << 15);

	eg_out = m_slot2.volume(am);
	if (eg_out < ENV_QUIET)        // SLOT 2
		*m_connect2 += op_calc(m_slot2.phase(), eg_out, m_c1 << 15);

	eg_out = m_slot4.volume(am);
	if (eg_out < ENV_QUIET)        // SLOT 4
		*m_connect4 += op_calc(m_slot4.phase(), eg_out, m_c2 << 15);

	// store current MEM
	m_mem_value = m_mem;

	// update phase counters AFTER output calculations
	if (m_3slot != nullptr)
	{
		m_slot1.update_phase_lfo(lfo_pm, m_pm_shift, m_3slot->m_block_fnum[1]);
		m_slot2.update_phase_lfo(lfo_pm, m_pm_shift, m_3slot->m_block_fnum[2]);
		m_slot3.update_phase_lfo(lfo_pm, m_pm_shift, m_3slot->m_block_fnum[0]);
	}
	else
	{
		m_slot1.update_phase_lfo(lfo_pm, m_pm_shift, m_block_fnum);
		m_slot2.update_phase_lfo(lfo_pm, m_pm_shift, m_block_fnum);
		m_slot3.update_phase_lfo(lfo_pm, m_pm_shift, m_block_fnum);
	}
	m_slot4.update_phase_lfo(lfo_pm, m_pm_shift, m_block_fnum);
}

// helper to set the 5 connections
inline void opn_channel_t::set_connections(s32 *c1, s32 *c2, s32 *c3, s32 *c4, s32 *mem)
{
	m_connect1 = c1;
	m_connect2 = c2;
	m_connect3 = c3;
	m_connect4 = c4;
	m_mem_connect = mem;
}

// set the connections based on the algorithm
void opn_channel_t::setup_connection()
{
	switch (m_algorithm)
	{
		case 0:
			// M1---C1---MEM---M2---C2---OUT
			set_connections(&m_c1, &m_mem, &m_c2, &m_out_fm, &m_m2);
			break;

		case 1:
			// M1------+-MEM---M2---C2---OUT
			//      C1-+
			set_connections(&m_mem, &m_mem, &m_c2, &m_out_fm, &m_m2);
			break;

		case 2:
			// M1-----------------+-C2---OUT
			//      C1---MEM---M2-+
			set_connections(&m_c2, &m_mem, &m_c2, &m_out_fm, &m_m2);
			break;

		case 3:
			// M1---C1---MEM------+-C2---OUT
			//                 M2-+
			set_connections(&m_c1, &m_mem, &m_c2, &m_out_fm, &m_c2);
			break;

		case 4:
			// M1---C1-+-OUT
			// M2---C2-+
			// MEM: not used
			set_connections(&m_c1, &m_out_fm, &m_c2, &m_out_fm, &m_mem);
			break;

		case 5:
			//    +----C1----+
			// M1-+-MEM---M2-+-OUT
			//    +----C2----+
			set_connections(nullptr, &m_out_fm, &m_out_fm, &m_out_fm, &m_m2);
			break;

		case 6:
			// M1---C1-+
			//      M2-+-OUT
			//      C2-+
			// MEM: not used
			set_connections(&m_c1, &m_out_fm, &m_out_fm, &m_out_fm, &m_mem);
			break;

		case 7:
			// M1-+
			// C1-+-OUT
			// M2-+
			// C2-+
			// MEM: not used
			set_connections(&m_out_fm, &m_out_fm, &m_out_fm, &m_out_fm, &m_mem);
			break;
	}
}



//*********************************************************
// OPN base device class
//*********************************************************

ymopn_device_base::ymopn_device_base(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type) :
	ay8910_device(mconfig, type, tag, owner, clock, PSG_TYPE_YM, (type == YM2203) ? 3 : 1, (type == YM2610 || type == YM2610B) ? 0 : 2),
	m_eg_count(0),
	m_eg_timer(0),
	m_lfo_am(0),
	m_lfo_pm(0),
	m_lfo_count(0),
	m_lfo_step(0),
	m_prescaler_sel(0),
	m_freqbase(CLOCK_DIVIDER),
	m_timer_prescaler(0),
	m_busy_expiry_time(attotime::zero),
	m_irq(0),
	m_irqmask(0),
	m_status(0),
	m_mode(0),
	m_fn_h(0),
	m_timer_a(nullptr),
	m_timer_a_value(0),
	m_timer_b(nullptr),
	m_timer_b_value(0),
	m_stream(nullptr),
	m_irq_handler(*this)
{
	// allocate the appropriate number of channels
	int channels = (type == YM2203) ? 3 : 6;
	for (int index = 0; index < channels; index++)
		m_channel.push_back(std::make_unique<opn_channel_t>(*this));
}

// status set and IRQ handling
void ymopn_device_base::set_reset_status(u8 set, u8 reset)
{
	m_status = (m_status | set) & ~reset;
	bool irq = ((m_status & m_irqmask) != 0);
	if (irq != m_irq)
	{
		m_irq = irq;
		synchronize(TIMER_IRQ_SYNC, m_irq);
	}
}

// IRQ mask set
void ymopn_device_base::set_irqmask(u8 flag)
{
	m_irqmask = flag;
	set_reset_status();
}

u8 ymopn_device_base::status()
{
	u8 status = m_status;
	if (machine().time() < m_busy_expiry_time)
		status |= 0x80;
	return status;
}

void ymopn_device_base::busy_set()
{
	attotime expiry_period = attotime::from_hz(clock()) * m_timer_prescaler;
	m_busy_expiry_time = machine().time() + expiry_period;
}

// return the period of timer A, in attotime
attotime ymopn_device_base::timer_a_period() const
{
	return ((1024 - m_timer_a_value) * m_timer_prescaler) * attotime::from_hz(clock());
}

// return the period of timer B, in attotime
attotime ymopn_device_base::timer_b_period() const
{
	return ((256 - m_timer_b_value) * 16 * m_timer_prescaler) * attotime::from_hz(clock());
}


//-------------------------------------------------
//  set_mode - handle writes to the mode register
//-------------------------------------------------

void ymopn_device_base::set_mode(u8 value)
{
	// b7 = CSM MODE
	// b6 = 3 slot mode
	// b5 = reset b
	// b4 = reset a
	// b3 = timer enable b
	// b2 = timer enable a
	// b1 = load b
	// b0 = load a
	m_mode = value;

	// notify the relevant channel of being in 3-slot mode
	three_slot_channel().set_three_slot_mode(BIT(value, 6) ? &m_3slot_state : nullptr);

	// reset Timer b flag
	if (BIT(value, 5))
		set_reset_status(0, 0x02);

	// reset Timer a flag
	if (BIT(value, 4))
		set_reset_status(0, 0x01);

	// load b
	if (BIT(value, 1))
	{
		if (!m_timer_b->enabled())
			m_timer_b->adjust(timer_b_period());
	}
	else
		m_timer_b->enable(false);

	// load a
	if (BIT(value, 0))
	{
		if (!m_timer_a->enabled())
			m_timer_a->adjust(timer_a_period());
	}
	else
		m_timer_a->enable(false);
}


//-------------------------------------------------
//  advance_lfo - step the LFO forward one step
//-------------------------------------------------

void ymopn_device_base::advance_lfo()
{
	if (m_lfo_step != 0)   // LFO enabled ?
	{
		m_lfo_count += m_lfo_step;

		u8 pos = (m_lfo_count >> LFO_SHIFT) & 0x7f;
		m_lfo_am = (pos < 64) ? (pos * 2) : (126 - (pos & 63) * 2);
		m_lfo_pm = pos >> 2;
	}
	else
	{
		m_lfo_am = 0;
		m_lfo_pm = 0;
	}
}


//-------------------------------------------------
//  write_mode - handle writes to OPN mode
//  registers (0x20-0x2f)
//-------------------------------------------------

void ymopn_device_base::write_mode(u8 reg, u8 value)
{
	switch (reg)
	{
		case 0x21:  // Test
			break;

		case 0x22:  // LFO FREQ (YM2608/YM2610/YM2610B/YM2612)
			if (type() != YM2203)
				m_lfo_step = BIT(value, 3) ? lfo_step(value & 7) : 0;
			break;

		case 0x24:  // timer A High 8
			m_timer_a_value = (m_timer_a_value & 0x03) | (value << 2);
			break;

		case 0x25:  // timer A Low 2
			m_timer_a_value = (m_timer_a_value & 0x3fc) | (value & 0x03);
			break;

		case 0x26:  // timer B
			m_timer_b_value = value;
			break;

		case 0x27:  // mode, timer control
			set_mode(value);
			break;

		case 0x28:  // key on / off
			if ((value & 3) != 3)
			{
				opn_channel_t &chan = channel(value & 3, BIT(value, 2));
				chan.slot1().keyonoff(BIT(value, 4));
				chan.slot2().keyonoff(BIT(value, 5));
				chan.slot3().keyonoff(BIT(value, 6));
				chan.slot4().keyonoff(BIT(value, 7));
			}
			break;
	}
}


//-------------------------------------------------
//  write_reg - handle writes to OPN registers
//  (0x30-0xff)
//-------------------------------------------------

void ymopn_device_base::write_reg(u16 reg, u8 value)
{
	// determine channel
	u8 chnum = reg & 3;
	if (chnum == 3)
		return; // 0xX3,0xX7,0xXB,0xXF
	opn_channel_t &chan = channel(chnum, reg >= 0x100);

	// determine slot
	u8 slotnum = (reg >> 2) & 3;
	opn_slot_t &slot = chan.slot(slotnum);

	switch (reg & 0xf0)
	{
		case 0x30:  // DET , MUL
			slot.set_det_mul(value);
			chan.force_refresh();
			break;

		case 0x40:  // TL
			slot.set_tl(value);
			break;

		case 0x50:  // KS, AR
			if (slot.set_ar_ksr(value))
				chan.force_refresh();
			break;

		case 0x60:  // bit7 = AM ENABLE, DR
			slot.set_dr_am(value);
			break;

		case 0x70:  //     SR
			slot.set_sr(value);
			break;

		case 0x80:  // SL, RR
			slot.set_sl_rr(value);
			break;

		case 0x90:  // SSG-EG
			slot.set_ssg(value);
			break;

		case 0xa0:
			switch (slotnum)
			{
				case 0:     // 0xa0-0xa2 : FNUM1
					chan.set_fnum(m_fn_h, value);
					break;

				case 1:     // 0xa4-0xa6 : FNUM2,BLK
					m_fn_h = value & 0x3f;
					break;

				case 2:     // 0xa8-0xaa : 3CH FNUM1
					if (reg < 0x100)
					{
						m_3slot_state.set_fnum(*this, chnum, value);
						three_slot_channel().force_refresh();
					}
					break;

				case 3:     // 0xac-0xae : 3CH FNUM2,BLK
					if (reg < 0x100)
						m_3slot_state.m_fn_h = value & 0x3f;
					break;
			}
			break;

		case 0xb0:
			switch (slotnum)
			{
				case 0:     // 0xb0-0xb2 : FB,ALGO
					chan.set_algorithm_feedback(value);
					break;

				case 1:     // 0xb4-0xb6 : L , R , AMS , PMS (YM2612/YM2610B/YM2610/YM2608)
					if (type() != YM2203)
						chan.set_lfo_shift_pan(value);
					break;
			}
		break;
	}
}


//-------------------------------------------------
//  prescaler_w - handle writes to prescaler
//  registers
//-------------------------------------------------

//	prescaler circuit (best guess to verified chip behaviour)
//
//                    +--------------+  +-sel2-+
//                    |              +--|in20  |
//              +---+ |  +-sel1-+       |      |
//     M-CLK -+-|1/2|-+--|in10  | +---+ |   out|--INT_CLOCK
//            | +---+    |   out|-|1/3|-|in21  |
//            +----------|in11  | +---+ +------+
//                       +------+
//
//	reg.2d : sel2 = in21 (select sel2)
//	reg.2e : sel1 = in11 (select sel1)
//	reg.2f : sel1 = in10 , sel2 = in20 (clear selector)
//	reset  : sel1 = in11 , sel2 = in21 (clear both)
void ymopn_device_base::prescaler_w(u8 addr)
{
	switch (addr)
	{
		case 0x2d:  // divider sel : select 1/1 for 1/3line
			m_prescaler_sel |= 0x02;
			break;

		case 0x2e:  // divider sel , select 1/3line for output
			m_prescaler_sel |= 0x01;
			break;

		case 0x2f:  // divider sel , clear both selector to 1/2,1/2
			m_prescaler_sel = 0;
			break;
	}

	// update prescaler
	set_prescale(m_prescaler_sel);
}


//-------------------------------------------------
//  set_prescale - directly configure prescale
//  values
//-------------------------------------------------

void ymopn_device_base::set_prescale(u8 sel)
{
	static const u8 s_opn_pres[4] = { 2*12, 2*12, 6*12, 3*12 };
	static const u8 s_ssg_pres[4] = {    1,    1,    4,    2 };
	const u8 pre_divider = (type() == YM2203) ? 1 : 2;

	sel &= 3;
	m_prescaler_sel = sel;

	m_freqbase = CLOCK_DIVIDER / (s_opn_pres[sel] * pre_divider);
	m_timer_prescaler = s_opn_pres[sel] * pre_divider;
	ay_set_clock(clock() * 2 / (s_ssg_pres[sel] * pre_divider));
}


//-------------------------------------------------
//  device_timer - handle timer callbacks
//-------------------------------------------------

void ymopn_device_base::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_A:
			m_stream->update();
			if (BIT(m_mode, 7))
				channel(2).csm_key_control();
			if (BIT(m_mode, 2))
				set_reset_status(0x01);
			m_timer_a->adjust(timer_a_period());
			break;

		case TIMER_B:
			if (BIT(m_mode, 3))
				set_reset_status(0x02);
			m_timer_b->adjust(timer_b_period());
			break;

		case TIMER_IRQ_SYNC:
			if (!m_irq_handler.isnull())
				m_irq_handler(param);
			break;

		case TIMER_WRITE_REG:
			m_stream->update();
			write_reg(param >> 8, param & 0xff);
			break;

		case TIMER_WRITE_MODE:
			m_stream->update();
			write_mode(param >> 8, param & 0xff);
			break;
	}
}


//-------------------------------------------------
//  sound_stream_update_ex - generate samples
//-------------------------------------------------

void ymopn_device_base::sound_stream_update_ex(sound_stream &stream, std::vector<read_stream_view> &inputs, std::vector<write_stream_view> &outputs)
{
	// if this is not our stream, pass it on
	if (&stream != m_stream)
	{
		ay8910_device::sound_stream_update_ex(stream, inputs, outputs);
		return;
	}

	// refresh PG and EG
	for (int index = 0; index < m_channel.size(); index++)
		channel(index).refresh_fc_eg();

	// buffering
	for (int sampindex = 0; sampindex < outputs[0].samples(); sampindex++)
	{
		// advance envelope generator
		m_eg_timer += m_freqbase << EG_SHIFT;
		while (m_eg_timer >= EG_TIMER_OVERFLOW)
		{
			m_eg_timer -= EG_TIMER_OVERFLOW;
			m_eg_count++;

			for (int index = 0; index < m_channel.size(); index++)
				channel(index).advance_eg(m_eg_count);
		}

		// calculate FM
		s32 lsum = 0, rsum = 0;
		for (int index = 0; index < m_channel.size(); index++)
		{
			auto &chan = channel(index);
			chan.update(m_lfo_am, m_lfo_pm);
			lsum += chan.output_l();
			rsum += chan.output_r();
		}

		// clamp (does the chip do this?)
		if (lsum < -32768)
			lsum = -32768;
		if (lsum > 32767)
			lsum = 32767;
		if (rsum < -32768)
			rsum = -32768;
		if (rsum > 32767)
			rsum = 32767;

		// buffering
		outputs[0].put(sampindex, lsum * sample_scale);
		if (outputs.size() > 1)
			outputs[1].put(sampindex, rsum * sample_scale);
	}
}


//-------------------------------------------------
//  init_tables - derive runtime tables
//-------------------------------------------------

void ymopn_device_base::init_tables()
{
	if (s_tl_table != nullptr)
		return;

	// build the total level table
	s_tl_table = new s32[TL_TAB_LEN];
	for (int x = 0; x < TL_RES_LEN; x++)
	{
		double m = double(1 << 16) / pow(2, (x + 1) * (ENV_STEP/4.0) / 8.0);

		// we never reach (1 << 16) here due to the (x+1)
		// result fits within 16 bits at maximum

		s32 n = int(floor(m));  // 16 bits here
		n >>= 4;                // 12 bits here
		n = (n >> 1) + (n & 1); // 11 bits here (rounded)
		n <<= 2;                // 13 bits here (as in real chip)

		for (int i = 0; i < 13; i++)
		{
			s_tl_table[(i * TL_RES_LEN + x) * 2 + 0] = n >> i;
			s_tl_table[(i * TL_RES_LEN + x) * 2 + 1] = -(n >> i);
		}
	}

	// build the sin table
	s_sin_table = new u32[SIN_LEN];
	for (int i = 0; i < SIN_LEN; i++)
	{
		// non-standard sinus -- verified against the real chip
		double m = sin(((i * 2) + 1) * M_PI / SIN_LEN);

		// we never reach zero here due to ((i*2)+1)
		double o = 8 * log(1.0 / fabs(m)) / log(2.0);  // convert to 'decibels'
		o /= (ENV_STEP/4);

		s32 n = int(2.0 * o);
		n = (n >> 1) + (n & 1);			// round to nearest

		s_sin_table[i] = (n * 2) + (m >= 0.0 ? 0 : 1);
	}

	// build the LFO PM modulation table
	// 128 combinations of 7 bits meaningful (of F-NUMBER), 8 LFO depths, 32 LFO output levels per one depth
	s_lfo_pm_table = new s32[128*8*32];
	for (int offset_depth = 0; offset_depth < 8; offset_depth++) // 8 PM depths
		for (u8 fnum = 0; fnum < 128; fnum++) // 7 bits meaningful of F-NUMBER
			for (u8 step = 0; step < 8; step++)
			{
				u8 value = 0;
				for (u32 bit_tmp = 0; bit_tmp < 7; bit_tmp++) // 7 bits
				{
					if (BIT(fnum, bit_tmp)) // only if bit "bit_tmp" is set
					{
						u32 offset_fnum_bit = bit_tmp * 8;
						value += s_lfo_pm_output[offset_fnum_bit + offset_depth][step];
					}
				}
				s_lfo_pm_table[(fnum * 32 * 8) + (offset_depth * 32) +  step    +  0] = value;
				s_lfo_pm_table[(fnum * 32 * 8) + (offset_depth * 32) + (step^7) +  8] = value;
				s_lfo_pm_table[(fnum * 32 * 8) + (offset_depth * 32) +  step    + 16] = -value;
				s_lfo_pm_table[(fnum * 32 * 8) + (offset_depth * 32) + (step^7) + 24] = -value;
			}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ymopn_device_base::device_start()
{
	ay8910_device::device_start();

	m_irq_handler.resolve();

	m_timer_a = timer_alloc(TIMER_A);
	m_timer_b = timer_alloc(TIMER_B);

	m_stream = &stream_alloc_ex(0, 1, clock() / CLOCK_DIVIDER);

	init_tables();

	save_item(NAME(m_3slot_state.m_fc));
	save_item(NAME(m_3slot_state.m_fn_h));
	save_item(NAME(m_3slot_state.m_kcode));
	save_item(NAME(m_3slot_state.m_block_fnum));
	for (int chnum = 0; chnum < m_channel.size(); chnum++)
		channel(chnum).save(chnum);

	save_item(NAME(m_eg_count));
	save_item(NAME(m_eg_timer));

	save_item(NAME(m_lfo_am));
	save_item(NAME(m_lfo_pm));
	save_item(NAME(m_lfo_count));
	save_item(NAME(m_lfo_step));

	save_item(NAME(m_prescaler_sel));
	save_item(NAME(m_freqbase));
	save_item(NAME(m_timer_prescaler));

	save_item(NAME(m_busy_expiry_time));
	save_item(NAME(m_irq));
	save_item(NAME(m_irqmask));
	save_item(NAME(m_status));
	save_item(m_mode, "m_ym_mode");		// PSG also saves 'm_mode'
	save_item(NAME(m_fn_h));

	save_item(NAME(m_timer_a_value));
	save_item(NAME(m_timer_b_value));
}

void ym2203_device::device_start()
{
	ymopn_device_base::device_start();

	save_item(NAME(m_address));
	save_item(m_regs, "m_ym_regs");		// PSG also saves 'm_regs'
}

#if 0
void ym2608_device::device_start()
{
	ymopn_device_base::device_start();

	/* DELTA-T */
	F2608->deltaT.read_byte = ExternalReadByte;
	F2608->deltaT.write_byte = ExternalWriteByte;

	/*F2608->deltaT.write_time = 20.0 / clock;*/    /* a single byte write takes 20 cycles of main clock */
	/*F2608->deltaT.read_time  = 18.0 / clock;*/    /* a single byte read takes 18 cycles of main clock */

	F2608->deltaT.status_set_handler = YM2608_deltat_status_set;
	F2608->deltaT.status_reset_handler = YM2608_deltat_status_reset;
	F2608->deltaT.status_change_which_chip = F2608;
	F2608->deltaT.status_change_EOS_bit = 0x04; /* status flag: set bit2 on End Of Sample */
	F2608->deltaT.status_change_BRDY_bit = 0x08;    /* status flag: set bit3 on BRDY */
	F2608->deltaT.status_change_ZERO_bit = 0x10;    /* status flag: set bit4 if silence continues for more than 290 milliseconds while recording the ADPCM */

	/* ADPCM Rhythm */
	F2608->read_byte = InternalReadByte;

	Init_ADPCMATable();

	save_item(NAME(m_address));
	save_item(NAME(m_regs));
	/* address register1 */
	save_item(NAME(F2608->addr_A1));
	/* rhythm(ADPCMA) */
	FMsave_state_adpcma(device,F2608->adpcm);
	/* Delta-T ADPCM unit */
	F2608->deltaT.savestate(device);
}
#endif


//-------------------------------------------------
//  device_post_load - device-specific refresh
//  after state loading
//-------------------------------------------------

void ymopn_device_base::device_post_load()
{
}


//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void ymopn_device_base::device_stop()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ymopn_device_base::device_reset()
{
	// reset SSG section
	ay8910_device::device_reset();

	m_busy_expiry_time = attotime::zero;
	m_mode = 0;
	m_timer_a_value = 0;
	m_timer_b_value = 0;

	m_eg_timer = 0;
	m_eg_count = 0;
	for (int chnum = 0; chnum < m_channel.size(); chnum++)
		channel(chnum).reset();
}

void ym2203_device::device_reset()
{
	// reset prescaler
	set_prescale(2);

	// status clear
	set_irqmask(0x03);
	set_mode(0x30); // mode 0, timer reset
	set_reset_status(0, 0xff);
	ymopn_device_base::device_reset();

	// reset operator paramater
	for (int regnum = 0xb2; regnum >= 0x30; regnum--)
		write_reg(regnum, 0);
	for (int regnum = 0x26; regnum >= 0x20; regnum--)
		write_reg(regnum, 0);
}

#if 0
void ym2608_device::device_reset()
{
	// reset prescaler
	prescaler_w(0, 2);

	F2608->deltaT.freqbase = OPN->ST.freqbase;

	/* register 0x29 - default value after reset is:
	    enable only 3 FM channels and enable all the status flags */
	YM2608IRQMaskWrite(OPN, F2608, 0x1f );  /* default value for D4-D0 is 1 */

	/* register 0x10, A1=1 - default value is 1 for D4, D3, D2, 0 for the rest */
	YM2608IRQFlagWrite(OPN, F2608, 0x1c );  /* default: enable timer A and B, disable EOS, BRDY and ZERO */

	set_mode(0x30); // mode 0 , timer reset
	set_reset_status(0, 0xff);
	ymopn_device_base::device_reset();

	// reset operator paramater
	for (int regnum = 0xb6; regnum >= 0xb4; regnum--)
	{
		write_reg(regnum + 0x000, 0xc0);
		write_reg(regnum + 0x100, 0xc0);
	}
	for (int regnum = 0xb2; regnum >= 0x30; regnum--)
	{
		write_reg(regnum + 0x000, 0);
		write_reg(regnum + 0x100, 0);
	}
	for (int regnum = 0x26; regnum >= 0x20; regnum--)
		write_reg(regnum, 0);

	/* ADPCM - percussion sounds */
	for( i = 0; i < 6; i++ )
	{
		if (i<=3)   /* channels 0,1,2,3 */
			F2608->adpcm[i].step      = (uint32_t)((float)(1<<ADPCM_SHIFT)*((float)F2608->OPN.ST.freqbase)/3.0f);
		else        /* channels 4 and 5 work with slower clock */
			F2608->adpcm[i].step      = (uint32_t)((float)(1<<ADPCM_SHIFT)*((float)F2608->OPN.ST.freqbase)/6.0f);

		F2608->adpcm[i].start     = YM2608_ADPCM_ROM_addr[i*2];
		F2608->adpcm[i].end       = YM2608_ADPCM_ROM_addr[i*2+1];

		F2608->adpcm[i].now_addr  = 0;
		F2608->adpcm[i].now_step  = 0;
		/* F2608->adpcm[i].delta     = 21866; */
		F2608->adpcm[i].vol_mul   = 0;
		F2608->adpcm[i].pan       = &OPN->out_adpcm[OUTD_CENTER]; /* default center */
		F2608->adpcm[i].flagMask  = 0;
		F2608->adpcm[i].flag      = 0;
		F2608->adpcm[i].adpcm_acc = 0;
		F2608->adpcm[i].adpcm_step= 0;
		F2608->adpcm[i].adpcm_out = 0;
	}
	F2608->adpcmTL = 0x3f;

	F2608->adpcm_arrivedEndAddress = 0; /* not used */

	/* DELTA-T unit */
	DELTAT->freqbase = OPN->ST.freqbase;
	DELTAT->output_pointer = OPN->out_delta;
	DELTAT->portshift = 5;      /* always 5bits shift */ /* ASG */
	DELTAT->output_range = 1<<23;
	DELTAT->ADPCM_Reset(OUTD_CENTER,YM_DELTAT::EMULATION_MODE_NORMAL,F2608->device);
}
#endif


//-------------------------------------------------
//  device_clock_changed - device-specific clock
//  changed event
//-------------------------------------------------

void ymopn_device_base::device_clock_changed()
{
	m_stream->set_sample_rate(clock() / CLOCK_DIVIDER);
}


//-------------------------------------------------
//  read - handle a read from the register
//  interface
//-------------------------------------------------

u8 ym2203_device::read(offs_t offset)
{
	// status port
	if ((offset & 1) == 0)
		return status();

	// data port (only SSG)
	else if (m_address < 16)
		return ay8910_read_ym();

	return 0;
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ym2203_device::write(offs_t offset, u8 value)
{
	// address port
	if ((offset & 1) == 0)
	{
		m_address = value;

		// Write register to SSG emulator
		if (m_address < 16)
			ay8910_write_ym(0, m_address);

		// prescaler select : 2d,2e,2f
		if (m_address >= 0x2d && m_address <= 0x2f)
			prescaler_w(m_address);
	}

	// data port
	else
	{
		m_regs[m_address] = value;
		switch (m_address & 0xf0)
		{
			case 0x00:  // 0x00-0x0f : SSG section
				ay8910_write_ym(1, value);
				break;

			case 0x20:  // 0x20-0x2f : Mode section
				synchronize(TIMER_WRITE_MODE, value | (m_address << 8));
				break;

			default:    // 0x30-0xff : OPN section
				synchronize(TIMER_WRITE_REG, value | (m_address << 8));
				break;
		}
		busy_set();
	}
}
































































#if 0


namespace {
//*** YM2610 ADPCM defines ***
constexpr unsigned ADPCM_SHIFT          = 16;  // frequency step rate
constexpr unsigned ADPCMA_ADDRESS_SHIFT = 8;   // adpcm A address shift

// speedup purposes only
static int jedi_table[ 49*16 ];

// ADPCM type A channel struct
struct ADPCM_CH
{
	u8       flag;           // port state
	u8       flagMask;       // arrived flag mask
	u8       now_data;       // current ROM data
	u32      now_addr;       // current ROM address
	u32      now_step;
	u32      step;
	u32      start;          // sample data start address
	u32      end;            // sample data end address
	u8       IL;             // Instrument Level
	s32       adpcm_acc;      // accumulator
	s32       adpcm_step;     // step
	s32       adpcm_out;      // (speedup) hiro-shi!!
	s8        vol_mul;        // volume in "0.75dB" steps
	u8       vol_shift;      // volume in "-6dB" steps
	s32       *pan;           // &out_adpcm[OPN_xxxx]
};

// here's the virtual YM2610
struct ym2610_state
{
	u8       REGS[512];          // registers
	FM_OPN      OPN;                // OPN state
	opn_channel_t       CH[6];              // channel state
	u8       addr_A1;            // address line A1

	// ADPCM-A unit
	FM_READBYTE   read_byte;
	u8       adpcmTL;            // adpcmA total level
	ADPCM_CH    adpcm[6];           // adpcm channels
	u32      adpcmreg[0x30];     // registers
	u8       adpcm_arrivedEndAddress;
	YM_DELTAT   deltaT;             // Delta-T ADPCM unit

	u8       flagmask;           // YM2608 only
	u8       irqmask;            // YM2608 only

	device_t    *device;

	// different from the usual ADPCM table
	static constexpr int step_inc[8] = { -1*16, -1*16, -1*16, -1*16, 2*16, 5*16, 7*16, 9*16 };

	// ADPCM A (Non control type) : calculate one channel output
	inline void ADPCMA_calc_chan( ADPCM_CH *ch )
	{
		u32 step;
		u8  data;


		ch->now_step += ch->step;
		if ( ch->now_step >= (1<<ADPCM_SHIFT) )
		{
			step = ch->now_step >> ADPCM_SHIFT;
			ch->now_step &= (1<<ADPCM_SHIFT)-1;
			do{
				// end check
				// 11-06-2001 JB: corrected comparison. Was > instead of ==
				// YM2610 checks lower 20 bits only, the 4 MSB bits are sample bank
				// Here we use 1<<21 to compensate for nibble calculations

				if (   (ch->now_addr & ((1<<21)-1)) == ((ch->end<<1) & ((1<<21)-1))    )
				{
					ch->flag = 0;
					adpcm_arrivedEndAddress |= ch->flagMask;
					return;
				}
				if ( ch->now_addr&1 )
					data = ch->now_data & 0x0f;
				else
				{
					ch->now_data = read_byte(device, ch->now_addr>>1);
					data = (ch->now_data >> 4) & 0x0f;
				}

				ch->now_addr++;

				ch->adpcm_acc += jedi_table[ch->adpcm_step + data];

				// the 12-bit accumulator wraps on the ym2610 and ym2608 (like the msm5205), it does not saturate (like the msm5218)
				ch->adpcm_acc &= 0xfff;

				// extend 12-bit signed int
				if (ch->adpcm_acc & 0x800)
					ch->adpcm_acc |= ~0xfff;

				ch->adpcm_step += step_inc[data & 7];
				if (ch->adpcm_step > 48*16)
					ch->adpcm_step = 48*16;
				else if (ch->adpcm_step < 0*16)
					ch->adpcm_step = 0*16;

			}while(--step);

			// calc pcm * volume data
			ch->adpcm_out = ((ch->adpcm_acc * ch->vol_mul) >> ch->vol_shift) & ~3;  // multiply, shift and mask out 2 LSB bits
		}

		// output for work of output channels (out_adpcm[OPNxxxx])
		*(ch->pan) += ch->adpcm_out;
	}

	// ADPCM type A Write
	void FM_ADPCMAWrite(int r,int v)
	{
		u8 c = r&0x07;

		adpcmreg[r] = v&0xff; // stock data
		switch( r )
		{
		case 0x00: // DM,--,C5,C4,C3,C2,C1,C0
			if( !(v&0x80) )
			{
				// KEY ON
				for( c = 0; c < 6; c++ )
				{
					if( (v>>c)&1 )
					{
						//*** start adpcm ***
						adpcm[c].step      = (u32)((float)(1<<ADPCM_SHIFT)*((float)CLOCK_DIVIDER)/3.0f);
						adpcm[c].now_addr  = adpcm[c].start<<1;
						adpcm[c].now_step  = 0;
						adpcm[c].adpcm_acc = 0;
						adpcm[c].adpcm_step= 0;
						adpcm[c].adpcm_out = 0;
						adpcm[c].flag      = 1;
					}
				}
			}
			else
			{
				// KEY OFF
				for( c = 0; c < 6; c++ )
					if( (v>>c)&1 )
						adpcm[c].flag = 0;
			}
			break;
		case 0x01:  // B0-5 = TL
			adpcmTL = (v & 0x3f) ^ 0x3f;
			for( c = 0; c < 6; c++ )
			{
				int volume = adpcmTL + adpcm[c].IL;

				if ( volume >= 63 ) // This is correct, 63 = quiet
				{
					adpcm[c].vol_mul   = 0;
					adpcm[c].vol_shift = 0;
				}
				else
				{
					adpcm[c].vol_mul   = 15 - (volume & 7);     // so called 0.75 dB
					adpcm[c].vol_shift =  1 + (volume >> 3);    // Yamaha engineers used the approximation: each -6 dB is close to divide by two (shift right)
				}

				// calc pcm * volume data
				adpcm[c].adpcm_out = ((adpcm[c].adpcm_acc * adpcm[c].vol_mul) >> adpcm[c].vol_shift) & ~3;  // multiply, shift and mask out low 2 bits
			}
			break;
		default:
			c = r&0x07;
			if( c >= 0x06 ) return;
			switch( r&0x38 )
			{
			case 0x08:  // B7=L,B6=R, B4-0=IL
			{
				int volume;

				adpcm[c].IL = (v & 0x1f) ^ 0x1f;

				volume = adpcmTL + adpcm[c].IL;

				if ( volume >= 63 ) // This is correct, 63 = quiet
				{
					adpcm[c].vol_mul   = 0;
					adpcm[c].vol_shift = 0;
				}
				else
				{
					adpcm[c].vol_mul   = 15 - (volume & 7);     // so called 0.75 dB
					adpcm[c].vol_shift =  1 + (volume >> 3);    // Yamaha engineers used the approximation: each -6 dB is close to divide by two (shift right)
				}

				adpcm[c].pan    = &OPN.out_adpcm[(v>>6)&0x03];

				// calc pcm * volume data
				adpcm[c].adpcm_out = ((adpcm[c].adpcm_acc * adpcm[c].vol_mul) >> adpcm[c].vol_shift) & ~3;  // multiply, shift and mask out low 2 bits
			}
				break;
			case 0x10:
			case 0x18:
				adpcm[c].start  = ( (adpcmreg[0x18 + c]*0x0100 | adpcmreg[0x10 + c]) << ADPCMA_ADDRESS_SHIFT);
				break;
			case 0x20:
			case 0x28:
				adpcm[c].end    = ( (adpcmreg[0x28 + c]*0x0100 | adpcmreg[0x20 + c]) << ADPCMA_ADDRESS_SHIFT);
				adpcm[c].end   += (1<<ADPCMA_ADDRESS_SHIFT) - 1;
				break;
			}
		}
	}

};

constexpr int ym2610_state::step_inc[8];

// here is the virtual YM2608
typedef ym2610_state ym2608_state;


// Algorithm and tables verified on real YM2608 and YM2610

// usual ADPCM table (16 * 1.1^N)
constexpr int steps[49] =
{
		16,  17,   19,   21,   23,   25,   28,
		31,  34,   37,   41,   45,   50,   55,
		60,  66,   73,   80,   88,   97,  107,
	118, 130,  143,  157,  173,  190,  209,
	230, 253,  279,  307,  337,  371,  408,
	449, 494,  544,  598,  658,  724,  796,
	876, 963, 1060, 1166, 1282, 1411, 1552
};


void Init_ADPCMATable()
{
	int step, nib;

	for (step = 0; step < 49; step++)
	{
		// loop over all nibbles and compute the difference
		for (nib = 0; nib < 16; nib++)
		{
			int value = (2*(nib & 0x07) + 1) * steps[step] / 8;
			jedi_table[step*16 + nib] = (nib&0x08) ? -value : value;
		}
	}
}

// FM channel save , internal state only
void FMsave_state_adpcma(device_t *device,ADPCM_CH *adpcm)
{
	int ch;

	for(ch=0;ch<6;ch++,adpcm++)
	{
		device->save_item(NAME(adpcm->flag), ch);
		device->save_item(NAME(adpcm->now_data), ch);
		device->save_item(NAME(adpcm->now_addr), ch);
		device->save_item(NAME(adpcm->now_step), ch);
		device->save_item(NAME(adpcm->adpcm_acc), ch);
		device->save_item(NAME(adpcm->adpcm_step), ch);
		device->save_item(NAME(adpcm->adpcm_out), ch);
	}
}
} // anonymous namespace


//***************************************************************************
//      YM2608 local section
//***************************************************************************



static const u32 YM2608_ADPCM_ROM_addr[2*6] = {
0x0000, 0x01bf, // bass drum
0x01c0, 0x043f, // snare drum
0x0440, 0x1b7f, // top cymbal
0x1b80, 0x1cff, // high hat
0x1d00, 0x1f7f, // tom tom
0x1f80, 0x1fff  // rim shot
};


// flag enable control 0x110
static inline void YM2608IRQFlagWrite(FM_OPN *OPN, ym2608_state *F2608, int v)
{
	if( v & 0x80 )
	{   // Reset IRQ flag
		set_reset_status(0, 0xf7); // don't touch BUFRDY flag otherwise we'd have to call ymdeltat module to set the flag back
	}
	else
	{   // Set status flag mask
		F2608->flagmask = (~(v&0x1f));
		OPN->ST.set_irqmask(F2608->irqmask & F2608->flagmask);
	}
}

// compatible mode & IRQ enable control 0x29
static inline void YM2608IRQMaskWrite(FM_OPN *OPN, ym2608_state *F2608, int v)
{
	// SCH,xx,xxx,EN_ZERO,EN_BRDY,EN_EOS,EN_TB,EN_TA

	// extend 3ch. enable/disable
	if(v&0x80)
		OPN->type |= TYPE_6CH;  // OPNA mode - 6 FM channels
	else
		OPN->type &= ~TYPE_6CH; // OPN mode - 3 FM channels

	// IRQ MASK store and set
	F2608->irqmask = v&0x1f;
	OPN->ST.set_irqmask(F2608->irqmask & F2608->flagmask);
}

// Generate samples for one of the YM2608s
void ym2608_update_one(void *chip, std::vector<write_stream_view> &buffer, int length)
{
	ym2608_state *F2608 = (ym2608_state *)chip;
	FM_OPN *OPN   = &F2608->OPN;
	YM_DELTAT *DELTAT = &F2608->deltaT;
	int i,j;
	opn_channel_t   *cch[6];
	s32 *out_fm = OPN->m_out_fm;

	cch[0]   = &F2608->CH[0];
	cch[1]   = &F2608->CH[1];
	cch[2]   = &F2608->CH[2];
	cch[3]   = &F2608->CH[3];
	cch[4]   = &F2608->CH[4];
	cch[5]   = &F2608->CH[5];

	// refresh PG and EG
	cch[0].refresh_fc_eg();
	cch[1].refresh_fc_eg();
	cch[2].refresh_fc_eg();
	cch[3].refresh_fc_eg();
	cch[4].refresh_fc_eg();
	cch[5].refresh_fc_eg();


	// buffering
	for(i=0; i < length ; i++)
	{
		OPN.advance_lfo();

		// clear output acc.
		OPN->out_adpcm[OUTD_LEFT] = OPN->out_adpcm[OUTD_RIGHT] = OPN->out_adpcm[OUTD_CENTER] = 0;
		OPN->out_delta[OUTD_LEFT] = OPN->out_delta[OUTD_RIGHT] = OPN->out_delta[OUTD_CENTER] = 0;
		// clear outputs
		out_fm[0] = 0;
		out_fm[1] = 0;
		out_fm[2] = 0;
		out_fm[3] = 0;
		out_fm[4] = 0;
		out_fm[5] = 0;

		// calculate FM
		chan_calc(OPN, cch[0], 0 );
		chan_calc(OPN, cch[1], 1 );
		chan_calc(OPN, cch[2], 2 );
		chan_calc(OPN, cch[3], 3 );
		chan_calc(OPN, cch[4], 4 );
		chan_calc(OPN, cch[5], 5 );

		// deltaT ADPCM
		if( DELTAT->portstate&0x80 )
			DELTAT->ADPCM_CALC();

		// ADPCMA
		for( j = 0; j < 6; j++ )
		{
			if( F2608->adpcm[j].flag )
				F2608->ADPCMA_calc_chan( &F2608->adpcm[j]);
		}

		// advance envelope generator
		OPN->m_eg_timer += EG_TIMER_ADD;
		while (OPN->m_eg_timer >= EG_TIMER_OVERFLOW)
		{
			OPN->m_eg_timer -= EG_TIMER_OVERFLOW;
			OPN->eg_cnt++;

			cch[0].advance_eg_channel(OPN->eg_cnt);
			cch[1].advance_eg_channel(OPN->eg_cnt);
			cch[2].advance_eg_channel(OPN->eg_cnt);
			cch[3].advance_eg_channel(OPN->eg_cnt);
			cch[4].advance_eg_channel(OPN->eg_cnt);
			cch[5].advance_eg_channel(OPN->eg_cnt);
		}

		// buffering
		{
			int lt,rt;

			lt =  OPN->out_adpcm[OUTD_LEFT]  + OPN->out_adpcm[OUTD_CENTER];
			rt =  OPN->out_adpcm[OUTD_RIGHT] + OPN->out_adpcm[OUTD_CENTER];
			lt += (OPN->out_delta[OUTD_LEFT]  + OPN->out_delta[OUTD_CENTER])>>9;
			rt += (OPN->out_delta[OUTD_RIGHT] + OPN->out_delta[OUTD_CENTER])>>9;
			lt += ((out_fm[0]>>1) & OPN->pan[0]);   // shift right verified on real YM2608
			rt += ((out_fm[0]>>1) & OPN->pan[1]);
			lt += ((out_fm[1]>>1) & OPN->pan[2]);
			rt += ((out_fm[1]>>1) & OPN->pan[3]);
			lt += ((out_fm[2]>>1) & OPN->pan[4]);
			rt += ((out_fm[2]>>1) & OPN->pan[5]);
			lt += ((out_fm[3]>>1) & OPN->pan[6]);
			rt += ((out_fm[3]>>1) & OPN->pan[7]);
			lt += ((out_fm[4]>>1) & OPN->pan[8]);
			rt += ((out_fm[4]>>1) & OPN->pan[9]);
			lt += ((out_fm[5]>>1) & OPN->pan[10]);
			rt += ((out_fm[5]>>1) & OPN->pan[11]);

			Limit( lt, MAXOUT, MINOUT );
			Limit( rt, MAXOUT, MINOUT );
			// buffering
			buffer[0].put(i, lt * sample_scale);
			buffer[1].put(i, rt * sample_scale);
		}
	}

	// check IRQ for DELTA-T EOS
	set_reset_status();

}
void ym2608_postload(void *chip)
{
	if (chip)
	{
		ym2608_state *F2608 = (ym2608_state *)chip;
		int r;

		// prescaler
		F2608->OPN.prescaler_w(1,2);
		// IRQ mask / mode
		YM2608IRQMaskWrite(&F2608->OPN, F2608, F2608->REGS[0x29]);
		// SSG registers
		for(r=0;r<16;r++)
		{
			(*F2608->OPN.ST.SSG->write)(F2608->OPN.ST.device,0,r);
			(*F2608->OPN.ST.SSG->write)(F2608->OPN.ST.device,1,F2608->REGS[r]);
		}

		// OPN registers
		// DT / MULTI , TL , KS / AR , AMON / DR , SR , SL / RR , SSG-EG
		for(r=0x30;r<0x9e;r++)
			if((r&3) != 3)
			{
				F2608->OPN.write_reg(r,F2608->REGS[r]);
				F2608->OPN.write_reg(r|0x100,F2608->REGS[r|0x100]);
			}
		// FB / CONNECT , L / R / AMS / PMS
		for(r=0xb0;r<0xb6;r++)
			if((r&3) != 3)
			{
				F2608->OPN.write_reg(r,F2608->REGS[r]);
				F2608->OPN.write_reg(r|0x100,F2608->REGS[r|0x100]);
			}
		// FM channels
		//FM_channel_postload(F2608->CH,6);
		// rhythm(ADPCMA)
		F2608->FM_ADPCMAWrite(1,F2608->REGS[0x111]);
		for( r=0x08 ; r<0x0c ; r++)
			F2608->FM_ADPCMAWrite(r,F2608->REGS[r+0x110]);
		// Delta-T ADPCM unit
		F2608->deltaT.postload( &F2608->REGS[0x100] );
	}
}

static void YM2608_save_state(ym2608_state *F2608, device_t *device)
{
	device->save_item(NAME(F2608->REGS));
	FMsave_state_st(device,&F2608->OPN.ST);
	FMsave_state_channel(device,F2608->CH,6);
	// 3slots
	device->save_item(NAME(F2608->OPN.SL3.fc));
	device->save_item(NAME(F2608->OPN.SL3.fn_h));
	device->save_item(NAME(F2608->OPN.SL3.kcode));
	// address register1
	device->save_item(NAME(F2608->addr_A1));
	// rhythm(ADPCMA)
	FMsave_state_adpcma(device,F2608->adpcm);
	// Delta-T ADPCM unit
	F2608->deltaT.savestate(device);
}

static void YM2608_deltat_status_set(void *chip, u8 changebits)
{
	ym2608_state *F2608 = (ym2608_state *)chip;
	F2608->OPN.ST->set_status(changebits);
}
static void YM2608_deltat_status_reset(void *chip, u8 changebits)
{
	ym2608_state *F2608 = (ym2608_state *)chip;
	F2608->OPN.ST->reset_status(changebits);
}
// YM2608(OPNA)
void * ym2608_init(device_t *device, int clock,
	FM_READBYTE InternalReadByte,
	FM_READBYTE ExternalReadByte, FM_WRITEBYTE ExternalWriteByte,
	FM_TIMERHANDLER timer_handler,FM_IRQHANDLER IRQHandler, const ssg_callbacks *ssg)
{
	ym2608_state *F2608;

	// allocate extend state space
	F2608 = auto_alloc_clear(device->machine(), <ym2608_state>());
	// allocate total level table (128kb space)
	if( !init_tables() )
	{
		auto_free( device->machine(), F2608 );
		return nullptr;
	}

	F2608->device = device;
	F2608->OPN.type = TYPE_YM2608;
	F2608->OPN.P_CH = F2608->CH;
	F2608->OPN.ST.device = device;
	F2608->OPN.ST.clock = clock;

	// External handlers
	F2608->OPN.ST.timer_handler = timer_handler;
	F2608->OPN.ST.IRQ_Handler   = IRQHandler;
	F2608->OPN.ST.SSG           = ssg;

	// DELTA-T
	F2608->deltaT.read_byte = ExternalReadByte;
	F2608->deltaT.write_byte = ExternalWriteByte;

	//F2608->deltaT.write_time = 20.0 / clock;    // a single byte write takes 20 cycles of main clock
	//F2608->deltaT.read_time  = 18.0 / clock;    // a single byte read takes 18 cycles of main clock

	F2608->deltaT.status_set_handler = YM2608_deltat_status_set;
	F2608->deltaT.status_reset_handler = YM2608_deltat_status_reset;
	F2608->deltaT.status_change_which_chip = F2608;
	F2608->deltaT.status_change_EOS_bit = 0x04; // status flag: set bit2 on End Of Sample
	F2608->deltaT.status_change_BRDY_bit = 0x08;    // status flag: set bit3 on BRDY
	F2608->deltaT.status_change_ZERO_bit = 0x10;    // status flag: set bit4 if silence continues for more than 290 milliseconds while recording the ADPCM

	// ADPCM Rhythm
	F2608->read_byte = InternalReadByte;

	Init_ADPCMATable();

	YM2608_save_state(F2608, device);
	return F2608;
}

void ym2608_clock_changed(void *chip, int clock)
{
	ym2608_state *F2608 = (ym2608_state *)chip;

	F2608->OPN.ST.clock = clock;
}

// shut down emulator
void ym2608_shutdown(void *chip)
{
	ym2608_state *F2608 = (ym2608_state *)chip;

	auto_free(F2608->OPN.ST.device->machine(), F2608);
}

// reset one of chips
void ym2608_reset_chip(void *chip)
{
	int i;
	ym2608_state *F2608 = (ym2608_state *)chip;
	FM_OPN *OPN   = &F2608->OPN;
	YM_DELTAT *DELTAT = &F2608->deltaT;

	// Reset Prescaler
	OPN->prescaler_w(0, 2);
	// reset SSG section
	(*OPN->ST.SSG->reset)(OPN->ST.device);

	// status clear
	OPN->ST.busy_clear();

	// register 0x29 - default value after reset is:
	// enable only 3 FM channels and enable all the status flags
	YM2608IRQMaskWrite(OPN, F2608, 0x1f );  // default value for D4-D0 is 1

	// register 0x10, A1=1 - default value is 1 for D4, D3, D2, 0 for the rest
	YM2608IRQFlagWrite(OPN, F2608, 0x1c );  // default: enable timer A and B, disable EOS, BRDY and ZERO

	OPN->write_mode(0x27,0x30);    // mode 0 , timer reset

	OPN->ST.reset_status(0xff);
	OPN->ST.reset();
	OPN->reset();

	// reset OPerator paramater
	for(i = 0xb6 ; i >= 0xb4 ; i-- )
	{
		OPN->write_reg(i      ,0xc0);
		OPN->write_reg(i|0x100,0xc0);
	}
	for(i = 0xb2 ; i >= 0x30 ; i-- )
	{
		OPN->write_reg(i      ,0);
		OPN->write_reg(i|0x100,0);
	}
	for(i = 0x26 ; i >= 0x20 ; i-- ) OPN->write_reg(i,0);

	// ADPCM - percussion sounds
	for( i = 0; i < 6; i++ )
	{
		if (i<=3)   // channels 0,1,2,3
			F2608->adpcm[i].step      = (u32)((float)(1<<ADPCM_SHIFT)*((float)F2608->CLOCK_DIVIDER)/3.0f);
		else        // channels 4 and 5 work with slower clock
			F2608->adpcm[i].step      = (u32)((float)(1<<ADPCM_SHIFT)*((float)F2608->CLOCK_DIVIDER)/6.0f);

		F2608->adpcm[i].start     = YM2608_ADPCM_ROM_addr[i*2];
		F2608->adpcm[i].end       = YM2608_ADPCM_ROM_addr[i*2+1];

		F2608->adpcm[i].now_addr  = 0;
		F2608->adpcm[i].now_step  = 0;
		// F2608->adpcm[i].delta     = 21866;
		F2608->adpcm[i].vol_mul   = 0;
		F2608->adpcm[i].pan       = &OPN->out_adpcm[OUTD_CENTER]; // default center
		F2608->adpcm[i].flagMask  = 0;
		F2608->adpcm[i].flag      = 0;
		F2608->adpcm[i].adpcm_acc = 0;
		F2608->adpcm[i].adpcm_step= 0;
		F2608->adpcm[i].adpcm_out = 0;
	}
	F2608->adpcmTL = 0x3f;

	F2608->adpcm_arrivedEndAddress = 0; // not used

	// DELTA-T unit
	DELTAT->output_pointer = OPN->out_delta;
	DELTAT->portshift = 5;      // always 5bits shift  // ASG
	DELTAT->output_range = 1<<23;
	DELTAT->ADPCM_Reset(OUTD_CENTER,YM_DELTAT::EMULATION_MODE_NORMAL,F2608->device);
}

// YM2608 write
// n = number
// a = address
// v = value
int ym2608_write(void *chip, int a,u8 v)
{
	ym2608_state *F2608 = (ym2608_state *)chip;
	FM_OPN *OPN   = &F2608->OPN;
	int addr;

	v &= 0xff;  //adjust to 8 bit bus


	switch(a&3)
	{
	case 0: // address port 0
		OPN->ST.address = v;
		F2608->addr_A1 = 0;

		// Write register to SSG emulator
		if( v < 16 ) (*OPN->ST.SSG->write)(OPN->ST.device,0,v);
		// prescaler selecter : 2d,2e,2f
		if( v >= 0x2d && v <= 0x2f )
		{
			OPN->prescaler_w(v, 2);
		}
		break;

	case 1: // data port 0
		if (F2608->addr_A1 != 0)
			break;  // verified on real YM2608

		addr = OPN->ST.address;
		F2608->REGS[addr] = v;
		switch(addr & 0xf0)
		{
		case 0x00:  // SSG section
			// Write data to SSG emulator
			(*OPN->ST.SSG->write)(OPN->ST.device,a,v);
			break;
		case 0x10:  // 0x10-0x1f : Rhythm section
			ym2608_device::update_request(OPN->ST.device);
			F2608->FM_ADPCMAWrite(addr-0x10,v);
			break;
		case 0x20:  // Mode Register
			switch(addr)
			{
			case 0x29:  // SCH,xx,xxx,EN_ZERO,EN_BRDY,EN_EOS,EN_TB,EN_TA
				YM2608IRQMaskWrite(OPN, F2608, v);
				break;
			default:
				ym2608_device::update_request(OPN->ST.device);
				OPN->write_mode(addr,v);
			}
			break;
		default:    // OPN section
			ym2608_device::update_request(OPN->ST.device);
			OPN->write_reg(addr,v);
		}
		break;

	case 2: // address port 1
		OPN->ST.address = v;
		F2608->addr_A1 = 1;
		break;

	case 3: // data port 1
		if (F2608->addr_A1 != 1)
			break;  // verified on real YM2608

		addr = OPN->ST.address;
		F2608->REGS[addr | 0x100] = v;
		ym2608_device::update_request(OPN->ST.device);
		switch( addr & 0xf0 )
		{
		case 0x00:  // DELTAT PORT
			switch( addr )
			{
			case 0x0e:  // DAC data
				F2608->device->logerror("YM2608: write to DAC data (unimplemented) value=%02x\n",v);
				break;
			default:
				// 0x00-0x0d
				F2608->deltaT.ADPCM_Write(addr,v);
			}
			break;
		case 0x10:  // IRQ Flag control
			if( addr == 0x10 )
			{
				YM2608IRQFlagWrite(OPN, F2608, v);
			}
			break;
		default:
			OPN->write_reg(addr | 0x100,v);
		}
	}
	return OPN->ST.irq;
}

u8 ym2608_read(void *chip,int a)
{
	ym2608_state *F2608 = (ym2608_state *)chip;
	int addr = F2608->OPN.ST.address;
	u8 ret = 0;

	switch( a&3 )
	{
	case 0: // status 0 : YM2203 compatible
		// BUSY:x:x:x:x:x:FLAGB:FLAGA
		ret = F2608->OPN.ST.status() & 0x83;
		break;

	case 1: // status 0, ID
		if( addr < 16 ) ret = (*F2608->OPN.ST.SSG->read)(F2608->OPN.ST.device);
		else if(addr == 0xff) ret = 0x01; // ID code
		break;

	case 2: // status 1 : status 0 + ADPCM status
		// BUSY : x : PCMBUSY : ZERO : BRDY : EOS : FLAGB : FLAGA
		ret = (F2608->OPN.ST.status() & (F2608->flagmask|0x80)) | ((F2608->deltaT.PCM_BSY & 1)<<5) ;
		break;

	case 3:
		if(addr == 0x08)
		{
			ret = F2608->deltaT.ADPCM_Read();
		}
		else
		{
			if(addr == 0x0f)
			{
				F2608->device->logerror("YM2608 A/D conversion is accessed but not implemented !\n");
				ret = 0x80; // 2's complement PCM data - result from A/D conversion
			}
		}
		break;
	}
	return ret;
}

int ym2608_timer_over(void *chip,int c)
{
	ym2608_state *F2608 = (ym2608_state *)chip;

	switch(c)
	{
#if 0
	case 2:
		{   // BUFRDY flag
			F2608->deltaT.BRDY_callback();
		}
		break;
#endif
	case 1:
		{   // Timer B
			F2608->OPN.ST.timer_b_over();
		}
		break;
	case 0:
		{   // Timer A
			ym2608_device::update_request(F2608->OPN.ST.device);
			// timer update
			F2608->OPN.ST.timer_a_over();
			// CSM mode key,TL controll
			if( F2608->OPN.ST.mode & 0x80 )
			{   // CSM mode total level latch and auto key on
				F2608->CH[2].csm_key_control();
			}
		}
		break;
	default:
		break;
	}

	return F2608->OPN.ST.irq;
}



// YM2610(OPNB)

// Generate samples for one of the YM2610s
void ym2610_update_one(void *chip, std::vector<write_stream_view> &buffer, int length)
{
	ym2610_state *F2610 = (ym2610_state *)chip;
	FM_OPN *OPN   = &F2610->OPN;
	YM_DELTAT *DELTAT = &F2610->deltaT;
	int i,j;
	opn_channel_t   *cch[4];
	s32 *out_fm = OPN->m_out_fm;

	cch[0] = &F2610->CH[1];
	cch[1] = &F2610->CH[2];
	cch[2] = &F2610->CH[4];
	cch[3] = &F2610->CH[5];

#ifdef YM2610B_WARNING
#define FM_KEY_IS(SLOT) ((SLOT)->key)
#define FM_MSG_YM2610B "YM2610-%p.CH%d is playing,Check whether the type of the chip is YM2610B\n"
	// Check YM2610B warning message
	if( FM_KEY_IS(&F2610->CH[0].SLOT[3]) )
		LOG(F2610->device,LOG_WAR,(FM_MSG_YM2610B,F2610->OPN.ST.device,0));
	if( FM_KEY_IS(&F2610->CH[3].SLOT[3]) )
		LOG(F2610->device,LOG_WAR,(FM_MSG_YM2610B,F2610->OPN.ST.device,3));
#endif

	// refresh PG and EG
	cch[0].refresh_fc_eg();
	cch[1].refresh_fc_eg();
	cch[2].refresh_fc_eg();
	cch[3].refresh_fc_eg();

	// buffering
	for(i=0; i < length ; i++)
	{
		OPN.advance_lfo();

		// clear output acc.
		OPN->out_adpcm[OUTD_LEFT] = OPN->out_adpcm[OUTD_RIGHT] = OPN->out_adpcm[OUTD_CENTER] = 0;
		OPN->out_delta[OUTD_LEFT] = OPN->out_delta[OUTD_RIGHT] = OPN->out_delta[OUTD_CENTER] = 0;
		// clear outputs
		out_fm[1] = 0;
		out_fm[2] = 0;
		out_fm[4] = 0;
		out_fm[5] = 0;

		// advance envelope generator
		OPN->m_eg_timer += EG_TIMER_ADD;
		while (OPN->m_eg_timer >= EG_TIMER_OVERFLOW)
		{
			OPN->m_eg_timer -= EG_TIMER_OVERFLOW;
			OPN->eg_cnt++;

			cch[0].advance_eg_channel(OPN->eg_cnt);
			cch[1].advance_eg_channel(OPN->eg_cnt);
			cch[2].advance_eg_channel(OPN->eg_cnt);
			cch[3].advance_eg_channel(OPN->eg_cnt);
		}

		// calculate FM
		chan_calc(OPN, cch[0], 1 ); //remapped to 1
		chan_calc(OPN, cch[1], 2 ); //remapped to 2
		chan_calc(OPN, cch[2], 4 ); //remapped to 4
		chan_calc(OPN, cch[3], 5 ); //remapped to 5

		// deltaT ADPCM
		if( DELTAT->portstate&0x80 )
			DELTAT->ADPCM_CALC();

		// ADPCMA
		for( j = 0; j < 6; j++ )
		{
			if( F2610->adpcm[j].flag )
				F2610->ADPCMA_calc_chan(&F2610->adpcm[j]);
		}

		// buffering
		{
			int lt,rt;

			lt =  OPN->out_adpcm[OUTD_LEFT]  + OPN->out_adpcm[OUTD_CENTER];
			rt =  OPN->out_adpcm[OUTD_RIGHT] + OPN->out_adpcm[OUTD_CENTER];
			lt += (OPN->out_delta[OUTD_LEFT]  + OPN->out_delta[OUTD_CENTER])>>9;
			rt += (OPN->out_delta[OUTD_RIGHT] + OPN->out_delta[OUTD_CENTER])>>9;


			lt += ((out_fm[1]>>1) & OPN->pan[2]);   // the shift right was verified on real chip
			rt += ((out_fm[1]>>1) & OPN->pan[3]);
			lt += ((out_fm[2]>>1) & OPN->pan[4]);
			rt += ((out_fm[2]>>1) & OPN->pan[5]);

			lt += ((out_fm[4]>>1) & OPN->pan[8]);
			rt += ((out_fm[4]>>1) & OPN->pan[9]);
			lt += ((out_fm[5]>>1) & OPN->pan[10]);
			rt += ((out_fm[5]>>1) & OPN->pan[11]);

			Limit( lt, MAXOUT, MINOUT );
			Limit( rt, MAXOUT, MINOUT );

			// buffering
			buffer[0].put(i, lt * sample_scale);
			buffer[1].put(i, rt * sample_scale);
		}
	}
}

// Generate samples for one of the YM2610Bs
void ym2610b_update_one(void *chip, std::vector<write_stream_view> &buffer, int length)
{
	ym2610_state *F2610 = (ym2610_state *)chip;
	FM_OPN *OPN   = &F2610->OPN;
	YM_DELTAT *DELTAT = &F2610->deltaT;
	int i,j;
	opn_channel_t   *cch[6];
	s32 *out_fm = OPN->m_out_fm;

	cch[0] = &F2610->CH[0];
	cch[1] = &F2610->CH[1];
	cch[2] = &F2610->CH[2];
	cch[3] = &F2610->CH[3];
	cch[4] = &F2610->CH[4];
	cch[5] = &F2610->CH[5];

	// refresh PG and EG
	cch[0].refresh_fc_eg();
	cch[1].refresh_fc_eg();
	cch[2].refresh_fc_eg();
	cch[3].refresh_fc_eg();
	cch[4].refresh_fc_eg();
	cch[5].refresh_fc_eg();

	// buffering
	for(i=0; i < length ; i++)
	{
		OPN.advance_lfo();

		// clear output acc.
		OPN->out_adpcm[OUTD_LEFT] = OPN->out_adpcm[OUTD_RIGHT] = OPN->out_adpcm[OUTD_CENTER] = 0;
		OPN->out_delta[OUTD_LEFT] = OPN->out_delta[OUTD_RIGHT] = OPN->out_delta[OUTD_CENTER] = 0;
		// clear outputs
		out_fm[0] = 0;
		out_fm[1] = 0;
		out_fm[2] = 0;
		out_fm[3] = 0;
		out_fm[4] = 0;
		out_fm[5] = 0;

		// advance envelope generator
		OPN->m_eg_timer += EG_TIMER_ADD;
		while (OPN->m_eg_timer >= EG_TIMER_OVERFLOW)
		{
			OPN->m_eg_timer -= EG_TIMER_OVERFLOW;
			OPN->eg_cnt++;

			cch[0].advance_eg_channel(OPN->eg_cnt);
			cch[1].advance_eg_channel(OPN->eg_cnt);
			cch[2].advance_eg_channel(OPN->eg_cnt);
			cch[3].advance_eg_channel(OPN->eg_cnt);
			cch[4].advance_eg_channel(OPN->eg_cnt);
			cch[5].advance_eg_channel(OPN->eg_cnt);
		}

		// calculate FM
		chan_calc(OPN, cch[0], 0 );
		chan_calc(OPN, cch[1], 1 );
		chan_calc(OPN, cch[2], 2 );
		chan_calc(OPN, cch[3], 3 );
		chan_calc(OPN, cch[4], 4 );
		chan_calc(OPN, cch[5], 5 );

		// deltaT ADPCM
		if( DELTAT->portstate&0x80 )
			DELTAT->ADPCM_CALC();

		// ADPCMA
		for( j = 0; j < 6; j++ )
		{
			if( F2610->adpcm[j].flag )
				F2610->ADPCMA_calc_chan(&F2610->adpcm[j]);
		}

		// buffering
		{
			int lt,rt;

			lt =  OPN->out_adpcm[OUTD_LEFT]  + OPN->out_adpcm[OUTD_CENTER];
			rt =  OPN->out_adpcm[OUTD_RIGHT] + OPN->out_adpcm[OUTD_CENTER];
			lt += (OPN->out_delta[OUTD_LEFT]  + OPN->out_delta[OUTD_CENTER])>>9;
			rt += (OPN->out_delta[OUTD_RIGHT] + OPN->out_delta[OUTD_CENTER])>>9;

			lt += ((out_fm[0]>>1) & OPN->pan[0]);   // the shift right is verified on YM2610
			rt += ((out_fm[0]>>1) & OPN->pan[1]);
			lt += ((out_fm[1]>>1) & OPN->pan[2]);
			rt += ((out_fm[1]>>1) & OPN->pan[3]);
			lt += ((out_fm[2]>>1) & OPN->pan[4]);
			rt += ((out_fm[2]>>1) & OPN->pan[5]);
			lt += ((out_fm[3]>>1) & OPN->pan[6]);
			rt += ((out_fm[3]>>1) & OPN->pan[7]);
			lt += ((out_fm[4]>>1) & OPN->pan[8]);
			rt += ((out_fm[4]>>1) & OPN->pan[9]);
			lt += ((out_fm[5]>>1) & OPN->pan[10]);
			rt += ((out_fm[5]>>1) & OPN->pan[11]);

			Limit( lt, MAXOUT, MINOUT );
			Limit( rt, MAXOUT, MINOUT );

			// buffering
			buffer[0].put(i, lt * sample_scale);
			buffer[1].put(i, rt * sample_scale);
		}
	}
}


void ym2610_postload(void *chip)
{
	if (chip)
	{
		ym2610_state *F2610 = (ym2610_state *)chip;
		int r;

		// SSG registers
		for(r=0;r<16;r++)
		{
			(*F2610->OPN.ST.SSG->write)(F2610->OPN.ST.device,0,r);
			(*F2610->OPN.ST.SSG->write)(F2610->OPN.ST.device,1,F2610->REGS[r]);
		}

		// OPN registers
		// DT / MULTI , TL , KS / AR , AMON / DR , SR , SL / RR , SSG-EG
		for(r=0x30;r<0x9e;r++)
			if((r&3) != 3)
			{
				F2610->OPN.write_reg(r,F2610->REGS[r]);
				F2610->OPN.write_reg(r|0x100,F2610->REGS[r|0x100]);
			}
		// FB / CONNECT , L / R / AMS / PMS
		for(r=0xb0;r<0xb6;r++)
			if((r&3) != 3)
			{
				F2610->OPN.write_reg(r,F2610->REGS[r]);
				F2610->OPN.write_reg(r|0x100,F2610->REGS[r|0x100]);
			}
		// FM channels
		//FM_channel_postload(F2610->CH,6);

		// rhythm(ADPCMA)
		F2610->FM_ADPCMAWrite(1,F2610->REGS[0x101]);
		for( r=0 ; r<6 ; r++)
		{
			F2610->FM_ADPCMAWrite(r+0x08,F2610->REGS[r+0x108]);
			F2610->FM_ADPCMAWrite(r+0x10,F2610->REGS[r+0x110]);
			F2610->FM_ADPCMAWrite(r+0x18,F2610->REGS[r+0x118]);
			F2610->FM_ADPCMAWrite(r+0x20,F2610->REGS[r+0x120]);
			F2610->FM_ADPCMAWrite(r+0x28,F2610->REGS[r+0x128]);
		}
		// Delta-T ADPCM unit
		F2610->deltaT.postload( &F2610->REGS[0x010] );
	}
}

static void YM2610_save_state(ym2610_state *F2610, device_t *device)
{
	device->save_item(NAME(F2610->REGS));
	FMsave_state_st(device,&F2610->OPN.ST);
	FMsave_state_channel(device,F2610->CH,6);
	// 3slots
	device->save_item(NAME(F2610->OPN.SL3.fc));
	device->save_item(NAME(F2610->OPN.SL3.fn_h));
	device->save_item(NAME(F2610->OPN.SL3.kcode));
	// address register1
	device->save_item(NAME(F2610->addr_A1));

	device->save_item(NAME(F2610->adpcm_arrivedEndAddress));
	// rhythm(ADPCMA)
	FMsave_state_adpcma(device,F2610->adpcm);
	// Delta-T ADPCM unit
	F2610->deltaT.savestate(device);
}

static void YM2610_deltat_status_set(void *chip, u8 changebits)
{
	ym2610_state *F2610 = (ym2610_state *)chip;
	F2610->adpcm_arrivedEndAddress |= changebits;
}
static void YM2610_deltat_status_reset(void *chip, u8 changebits)
{
	ym2610_state *F2610 = (ym2610_state *)chip;
	F2610->adpcm_arrivedEndAddress &= (~changebits);
}

void *ym2610_init(device_t *device, int clock,
	FM_READBYTE adpcm_a_read_byte, FM_READBYTE adpcm_b_read_byte,
	FM_TIMERHANDLER timer_handler,FM_IRQHANDLER IRQHandler, const ssg_callbacks *ssg)
{
	ym2610_state *F2610;

	// allocate extend state space
	F2610 = auto_alloc_clear(device->machine(), <ym2610_state>());
	// allocate total level table (128kb space)
	if( !init_tables() )
	{
		auto_free( device->machine(), F2610 );
		return nullptr;
	}

	F2610->device = device;
	// FM
	F2610->OPN.type = TYPE_YM2610;
	F2610->OPN.P_CH = F2610->CH;
	F2610->OPN.ST.device = device;
	F2610->OPN.ST.clock = clock;
	// Extend handler
	F2610->OPN.ST.timer_handler = timer_handler;
	F2610->OPN.ST.IRQ_Handler   = IRQHandler;
	F2610->OPN.ST.SSG           = ssg;
	// ADPCM
	F2610->read_byte = adpcm_a_read_byte;
	// DELTA-T
	F2610->deltaT.read_byte = adpcm_b_read_byte;
	F2610->deltaT.write_byte = nullptr;

	F2610->deltaT.status_set_handler = YM2610_deltat_status_set;
	F2610->deltaT.status_reset_handler = YM2610_deltat_status_reset;
	F2610->deltaT.status_change_which_chip = F2610;
	F2610->deltaT.status_change_EOS_bit = 0x80; // status flag: set bit7 on End Of Sample

	Init_ADPCMATable();
	YM2610_save_state(F2610, device);
	return F2610;
}

void ym2610_clock_changed(void *chip, int clock)
{
	ym2610_state *F2610 = (ym2610_state *)chip;

	F2610->OPN.ST.clock = clock;
}

// shut down emulator
void ym2610_shutdown(void *chip)
{
	ym2610_state *F2610 = (ym2610_state *)chip;

	auto_free(F2610->OPN.ST.device->machine(), F2610);
}

// reset one of chip
void ym2610_reset_chip(void *chip)
{
	int i;
	ym2610_state *F2610 = (ym2610_state *)chip;
	FM_OPN *OPN   = &F2610->OPN;
	YM_DELTAT *DELTAT = &F2610->deltaT;

	device_t* dev = F2610->OPN.ST.device;
	std::string name(dev->tag());

	// Reset Prescaler
	OPN->set_prescale(6*24, 4*2); // OPN 1/6 , SSG 1/4
	// reset SSG section
	(*OPN->ST.SSG->reset)(OPN->ST.device);
	// status clear
	OPN->ST.set_irqmask(0x03);
	OPN->ST.busy_clear();
	OPN->write_mode(0x27,0x30); // mode 0 , timer reset

	OPN->ST.reset_status(0xff);
	OPN->ST.reset();
	OPN->reset_channels();

	// reset OPerator paramater
	for(i = 0xb6 ; i >= 0xb4 ; i-- )
	{
		OPN->write_reg(i      ,0xc0);
		OPN->write_reg(i|0x100,0xc0);
	}
	for(i = 0xb2 ; i >= 0x30 ; i-- )
	{
		OPN->write_reg(i      ,0);
		OPN->write_reg(i|0x100,0);
	}
	for(i = 0x26 ; i >= 0x20 ; i-- ) OPN->write_reg(i,0);
	//*** ADPCM work initial ***
	for( i = 0; i < 6 ; i++ )
	{
		F2610->adpcm[i].step      = (u32)((float)(1<<ADPCM_SHIFT)*((float)F2610->CLOCK_DIVIDER)/3.0f);
		F2610->adpcm[i].now_addr  = 0;
		F2610->adpcm[i].now_step  = 0;
		F2610->adpcm[i].start     = 0;
		F2610->adpcm[i].end       = 0;
		// F2610->adpcm[i].delta     = 21866;
		F2610->adpcm[i].vol_mul   = 0;
		F2610->adpcm[i].pan       = &OPN->out_adpcm[OUTD_CENTER]; // default center
		F2610->adpcm[i].flagMask  = 1<<i;
		F2610->adpcm[i].flag      = 0;
		F2610->adpcm[i].adpcm_acc = 0;
		F2610->adpcm[i].adpcm_step= 0;
		F2610->adpcm[i].adpcm_out = 0;
	}
	F2610->adpcmTL = 0x3f;

	F2610->adpcm_arrivedEndAddress = 0;

	// DELTA-T unit
	DELTAT->output_pointer = OPN->out_delta;
	DELTAT->portshift = 8;      // allways 8bits shift
	DELTAT->output_range = 1<<23;
	DELTAT->ADPCM_Reset(OUTD_CENTER,YM_DELTAT::EMULATION_MODE_YM2610,F2610->device);
}

// YM2610 write
// n = number
// a = address
// v = value
int ym2610_write(void *chip, int a, u8 v)
{
	ym2610_state *F2610 = (ym2610_state *)chip;
	FM_OPN *OPN   = &F2610->OPN;
	int addr;
	int ch;

	v &= 0xff;  // adjust to 8 bit bus

	switch( a&3 )
	{
	case 0: // address port 0
		OPN->ST.address = v;
		F2610->addr_A1 = 0;

		// Write register to SSG emulator
		if( v < 16 ) (*OPN->ST.SSG->write)(OPN->ST.device,0,v);
		break;

	case 1: // data port 0
		if (F2610->addr_A1 != 0)
			break;  // verified on real YM2608

		addr = OPN->ST.address;
		F2610->REGS[addr] = v;
		switch(addr & 0xf0)
		{
		case 0x00:  // SSG section
			// Write data to SSG emulator
			(*OPN->ST.SSG->write)(OPN->ST.device,a,v);
			break;
		case 0x10: // DeltaT ADPCM
			ym2610_device::update_request(OPN->ST.device);

			switch(addr)
			{
			case 0x10:  // control 1
			case 0x11:  // control 2
			case 0x12:  // start address L
			case 0x13:  // start address H
			case 0x14:  // stop address L
			case 0x15:  // stop address H

			case 0x19:  // delta-n L
			case 0x1a:  // delta-n H
			case 0x1b:  // volume
				{
					F2610->deltaT.ADPCM_Write(addr-0x10,v);
				}
				break;

			case 0x1c: //  FLAG CONTROL : Extend Status Clear/Mask
				{
					u8 statusmask = ~v;
					// set arrived flag mask
					for(ch=0;ch<6;ch++)
						F2610->adpcm[ch].flagMask = statusmask&(1<<ch);

					F2610->deltaT.status_change_EOS_bit = statusmask & 0x80;    // status flag: set bit7 on End Of Sample

					// clear arrived flag
					F2610->adpcm_arrivedEndAddress &= statusmask;
				}
				break;

			default:
				F2610->device->logerror("YM2610: write to unknown deltat register %02x val=%02x\n",addr,v);
				break;
			}

			break;
		case 0x20:  // Mode Register
			ym2610_device::update_request(OPN->ST.device);
			OPN->write_mode(addr,v);
			break;
		default:    // OPN section
			ym2610_device::update_request(OPN->ST.device);
			// write register
			OPN->write_reg(addr,v);
		}
		break;

	case 2: // address port 1
		OPN->ST.address = v;
		F2610->addr_A1 = 1;
		break;

	case 3: // data port 1
		if (F2610->addr_A1 != 1)
			break;  // verified on real YM2608

		ym2610_device::update_request(OPN->ST.device);
		addr = OPN->ST.address;
		F2610->REGS[addr | 0x100] = v;
		if( addr < 0x30 )
			// 100-12f : ADPCM A section
			F2610->FM_ADPCMAWrite(addr,v);
		else
			OPN->write_reg(addr | 0x100,v);
	}
	return OPN->ST.irq;
}

u8 ym2610_read(void *chip,int a)
{
	ym2610_state *F2610 = (ym2610_state *)chip;
	int addr = F2610->OPN.ST.address;
	u8 ret = 0;

	switch( a&3)
	{
	case 0: // status 0 : YM2203 compatible
		ret = F2610->OPN.ST.status() & 0x83;
		break;
	case 1: // data 0
		if( addr < 16 ) ret = (*F2610->OPN.ST.SSG->read)(F2610->OPN.ST.device);
		if( addr == 0xff ) ret = 0x01;
		break;
	case 2: // status 1 : ADPCM status
		// ADPCM STATUS (arrived End Address)
		// B,--,A5,A4,A3,A2,A1,A0
		// B     = ADPCM-B(DELTA-T) arrived end address
		// A0-A5 = ADPCM-A          arrived end address
		ret = F2610->adpcm_arrivedEndAddress;
		break;
	case 3:
		ret = 0;
		break;
	}
	return ret;
}

int ym2610_timer_over(void *chip,int c)
{
	ym2610_state *F2610 = (ym2610_state *)chip;

	if( c )
	{   // Timer B
		F2610->OPN.ST.timer_b_over();
	}
	else
	{   // Timer A
		ym2610_device::update_request(F2610->OPN.ST.device);
		// timer update
		F2610->OPN.ST.timer_a_over();
		// CSM mode key,TL controll
		if( F2610->OPN.ST.mode & 0x80 )
		{   // CSM mode total level latch and auto key on
			F2610->CH[2].csm_key_control();
		}
	}
	return F2610->OPN.ST.irq;
}
#endif

ym2203_device::ym2203_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymopn_device_base(mconfig, tag, owner, clock, YM2203)
{
}

DEFINE_DEVICE_TYPE(YM2203, ym2203_device, "ym2203", "YM2203 OPN")
