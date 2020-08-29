// license:GPL-2.0+
// copyright-holders:Jarek Burczynski,Tatsuyuki Satoh,Aaron Giles
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
	// m_freqbase is x.1, so only shift by one less
//	return (val * 32 * m_freqbase) << (FREQ_SHIFT - 11);
	return (val * m_freqbase) << (FREQ_SHIFT - 6);
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
	// assuming that SIN_BITS + FREQ_SHIFT > 20
	// also m_freqbase is x.1, so need to take off one more
	return (s_detune_table[fd * 32 + index] * m_freqbase) << (SIN_BITS + FREQ_SHIFT - 20 - 1);
}

inline u32 ymopn_device_base::lfo_step(u8 index) const
{
	// m_freqbase is x.1, so shift by 1 less
	return (m_freqbase << (LFO_SHIFT - 1)) / s_lfo_samples_per_step[index];
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
#define SLOT_NAME(x) x, "slot." #x
void opn_slot_t::save(int index)
{
	m_opn.save_item(SLOT_NAME(m_detune), index);
	m_opn.save_item(SLOT_NAME(m_ksr_shift), index);
	m_opn.save_item(SLOT_NAME(m_attack_rate), index);
	m_opn.save_item(SLOT_NAME(m_decay_rate), index);
	m_opn.save_item(SLOT_NAME(m_sustain_rate), index);
	m_opn.save_item(SLOT_NAME(m_release_rate), index);
	m_opn.save_item(SLOT_NAME(m_ksr), index);
	m_opn.save_item(SLOT_NAME(m_multiply), index);
	m_opn.save_item(SLOT_NAME(m_phase), index);
	m_opn.save_item(SLOT_NAME(m_phase_step), index);
	m_opn.save_item(SLOT_NAME(m_eg_state), index);
	m_opn.save_item(SLOT_NAME(m_total_level), index);
	m_opn.save_item(SLOT_NAME(m_volume), index);
	m_opn.save_item(SLOT_NAME(m_sustain_level), index);
	m_opn.save_item(SLOT_NAME(m_envelope_volume), index);
	m_opn.save_item(SLOT_NAME(m_attack_shift), index);
	m_opn.save_item(SLOT_NAME(m_attack_select), index);
	m_opn.save_item(SLOT_NAME(m_decay_shift), index);
	m_opn.save_item(SLOT_NAME(m_decay_select), index);
	m_opn.save_item(SLOT_NAME(m_sustain_shift), index);
	m_opn.save_item(SLOT_NAME(m_sustain_select), index);
	m_opn.save_item(SLOT_NAME(m_release_shift), index);
	m_opn.save_item(SLOT_NAME(m_release_select), index);
	m_opn.save_item(SLOT_NAME(m_ssg), index);
	m_opn.save_item(SLOT_NAME(m_ssg_state), index);
	m_opn.save_item(SLOT_NAME(m_key), index);
	m_opn.save_item(SLOT_NAME(m_am_mask), index);
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
	m_disabled(false),
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
#define CHANNEL_NAME(x) x, "channel." #x
void opn_channel_t::save(int index)
{
	m_slot1.save(index * 4 + 0);
	m_slot2.save(index * 4 + 1);
	m_slot3.save(index * 4 + 2);
	m_slot4.save(index * 4 + 3);

	m_opn.save_item(CHANNEL_NAME(m_algorithm), index);
	m_opn.save_item(CHANNEL_NAME(m_fb_shift), index);
	m_opn.save_item(CHANNEL_NAME(m_op1_out), index);

	m_opn.save_item(CHANNEL_NAME(m_mem_value), index);

	m_opn.save_item(CHANNEL_NAME(m_pm_shift), index);
	m_opn.save_item(CHANNEL_NAME(m_am_shift), index);

	m_opn.save_item(CHANNEL_NAME(m_fc), index);
	m_opn.save_item(CHANNEL_NAME(m_kcode), index);
	m_opn.save_item(CHANNEL_NAME(m_block_fnum), index);

	m_opn.save_item(CHANNEL_NAME(m_pan), index);
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
	setup_connection();
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

	// if disabled, do nothing more
	if (m_disabled)
		return;

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
// OPN ADPCM CHANNEL
//*********************************************************

opn_adpcm_channel_t::opn_adpcm_channel_t(ymopn_device_base &opn) :
	m_opn(opn),
	m_total_level(0x3f),
	m_instrument_level(0x1f),
	m_flag(0),
	m_curbyte(0),
	m_curaddress(0),
	m_curfrac(0),
	m_start(0),
	m_end(0),
	m_adpcm_acc(0),
	m_adpcm_step(0),
	m_addrshift(8+1),
	m_pan(3),
	m_step_divisor(1)
{
}

#define ADPCM_NAME(x) x, "adpcm." #x
void opn_adpcm_channel_t::save(int index)
{
	m_opn.save_item(ADPCM_NAME(m_total_level), index);
	m_opn.save_item(ADPCM_NAME(m_instrument_level), index);
	m_opn.save_item(ADPCM_NAME(m_flag), index);
	m_opn.save_item(ADPCM_NAME(m_curbyte), index);
	m_opn.save_item(ADPCM_NAME(m_curaddress), index);
	m_opn.save_item(ADPCM_NAME(m_curfrac), index);
	m_opn.save_item(ADPCM_NAME(m_start), index);
	m_opn.save_item(ADPCM_NAME(m_end), index);
	m_opn.save_item(ADPCM_NAME(m_adpcm_acc), index);
	m_opn.save_item(ADPCM_NAME(m_adpcm_step), index);
	m_opn.save_item(ADPCM_NAME(m_pan), index);
}

void opn_adpcm_channel_t::reset()
{
	m_total_level = 0x3f;
	m_instrument_level = 0x1f;
	m_flag = 0;
	m_curbyte = 0;
	m_curaddress = 0;
	m_curfrac = 0;
	m_start = 0;
	m_end = 0;
	m_adpcm_acc = 0;
	m_adpcm_step = 0;
	m_pan = 3;
}

void opn_adpcm_channel_t::keyonoff(bool on)
{
	if (on)
	{
		m_curaddress = start_address();
		m_curfrac = 0;
		m_adpcm_acc = 0;
		m_adpcm_step = 0;
		m_flag = 1;
	}
	else
		m_flag = 0;
}

void opn_adpcm_channel_t::set_volume_pan(u8 value)
{
	m_instrument_level = ~value & 0x1f;
	m_pan = value >> 6;
}

bool opn_adpcm_channel_t::update(u32 freqbase)
{
	// usual ADPCM table (16 * 1.1^N)
	static constexpr u16 s_steps[49] =
	{
		 16,  17,   19,   21,   23,   25,   28,
		 31,  34,   37,   41,   45,   50,   55,
		 60,  66,   73,   80,   88,   97,  107,
		118, 130,  143,  157,  173,  190,  209,
		230, 253,  279,  307,  337,  371,  408,
		449, 494,  544,  598,  658,  724,  796,
		876, 963, 1060, 1166, 1282, 1411, 1552
	};
	static constexpr s8 s_step_inc[8] = { -1, -1, -1, -1, 2, 5, 7, 9 };

	if (m_flag == 0)
	{
		m_adpcm_acc = 0;
		return false;
	}

	// freqbase is x.1, so shift by 1 less
	for (m_curfrac += (freqbase << (FRAC_SHIFT - 1)) / (3 * m_step_divisor); m_curfrac >= FRAC_ONE; m_curfrac -= FRAC_ONE)
	{
		// YM2610 checks lower 20 bits only, the 4 MSB bits are sample bank
		if ((((m_curaddress ^ end_address()) >> 1) & 0xfffff) == 0)
		{
			m_flag = 0;
			return true;
		}

		int nibble = m_curaddress & 1;
		if (nibble == 0)
			m_curbyte = m_opn.adpcm_read(m_curaddress >> 1);
		u8 data = u8(m_curbyte << (4 * nibble)) >> 4;
		m_curaddress++;

		// the 12-bit accumulator wraps on the ym2610 and ym2608 (like the msm5205), it does not saturate (like the msm5218)
		s32 delta = (2 * (data & 7) + 1) * s_steps[m_adpcm_step] / 8;
		if (BIT(data, 3))
			delta = -delta;
		m_adpcm_acc = (m_adpcm_acc + delta) & 0xfff;

		// extend 12-bit signed int
		if (m_adpcm_acc & 0x800)
			m_adpcm_acc |= ~0xfff;

		// adjust ADPCM step
		m_adpcm_step += s_step_inc[data & 7];
		if (m_adpcm_step > 48)
			m_adpcm_step = 48;
		else if (m_adpcm_step < 0)
			m_adpcm_step = 0;
	}
	return false;
}



//*********************************************************
// OPN DELTAT CHANNEL
//*********************************************************

opn_deltat_channel_t::opn_deltat_channel_t(ymopn_device_base &opn) :
	m_opn(opn),
	m_curaddress(0),
	m_curfrac(0),
	m_start(0),
	m_limit(~0),
	m_end(0),
	m_delta(0),
	m_volume(0),
	m_adpcm_acc(0),
	m_prev_acc(0),
	m_adpcm_step(127),
	m_output(0),
	m_curbyte(0),
	m_cpudata(0),
	m_portstate(0),
	m_control2(0),
	m_addrshift(5),
	m_memread(0),
	m_status(0)
{
}

#define DELTAT_NAME(x) x, "deltat." #x
void opn_deltat_channel_t::save(int index)
{
	m_opn.save_item(DELTAT_NAME(m_curaddress), index);
	m_opn.save_item(DELTAT_NAME(m_curfrac), index);
	m_opn.save_item(DELTAT_NAME(m_start), index);
	m_opn.save_item(DELTAT_NAME(m_limit), index);
	m_opn.save_item(DELTAT_NAME(m_end), index);
	m_opn.save_item(DELTAT_NAME(m_delta), index);
	m_opn.save_item(DELTAT_NAME(m_volume), index);
	m_opn.save_item(DELTAT_NAME(m_adpcm_acc), index);
	m_opn.save_item(DELTAT_NAME(m_prev_acc), index);
	m_opn.save_item(DELTAT_NAME(m_adpcm_step), index);
	m_opn.save_item(DELTAT_NAME(m_output), index);
	m_opn.save_item(DELTAT_NAME(m_curbyte), index);
	m_opn.save_item(DELTAT_NAME(m_cpudata), index);
	m_opn.save_item(DELTAT_NAME(m_portstate), index);
	m_opn.save_item(DELTAT_NAME(m_control2), index);
	m_opn.save_item(DELTAT_NAME(m_addrshift), index);
	m_opn.save_item(DELTAT_NAME(m_memread), index);
	m_opn.save_item(DELTAT_NAME(m_status), index);
}

void opn_deltat_channel_t::status_set_reset(u8 set, u8 reset)
{
	u8 prev = m_status;
	m_status = (m_status & ~reset) | set;
	if (m_status != prev)
		m_opn.deltat_status_change(m_status);
}

void opn_deltat_channel_t::reset()
{
	m_curaddress = 0;
	m_curfrac = 0;
	m_start = 0;
	m_limit = ~0;
	m_end = 0;
	m_delta = 0;
	m_volume = 0;
	m_adpcm_acc = 0;
	m_prev_acc = 0;
	m_adpcm_step = DELTA_DEFAULT;
	m_output = 0;
	m_portstate = (m_opn.type() == YM2610) ? 0x20 : 0x00;
	m_control2 = (m_opn.type() == YM2610) ? 0xc1 : 0xc0;
	m_addrshift = 5;
	m_memread = 0;
	status_set_reset(0, 0xff);
}

void opn_deltat_channel_t::set_portstate(u8 value)
{
	//
	// START:
	//     Accessing *external* memory is started when START bit (D7) is set to "1", so
	//     you must set all conditions needed for recording/playback before starting.
	//     If you access *CPU-managed* memory, recording/playback starts after
	//     read/write of ADPCM data register $08.
	//
	// REC:
	//     0 = ADPCM synthesis (playback)
	//     1 = ADPCM analysis (record)
	//
	// MEMDATA:
	//     0 = processor (*CPU-managed*) memory (means: using register $08)
	//     1 = external memory (using start/end/limit registers to access memory: RAM or ROM)
	//
	// SPOFF:
	//     controls output pin that should disable the speaker while ADPCM analysis
	//
	// RESET and REPEAT only work with external memory.
	//
	// some examples:
	// value:   START, REC, MEMDAT, REPEAT, SPOFF, x,x,RESET   meaning:
	//   C8     1      1    0       0       1      0 0 0       Analysis (recording) from AUDIO to CPU (to reg $08), sample rate in PRESCALER register
	//   E8     1      1    1       0       1      0 0 0       Analysis (recording) from AUDIO to EXT.MEMORY,       sample rate in PRESCALER register
	//   80     1      0    0       0       0      0 0 0       Synthesis (playing) from CPU (from reg $08) to AUDIO,sample rate in DELTA-N register
	//   a0     1      0    1       0       0      0 0 0       Synthesis (playing) from EXT.MEMORY to AUDIO,        sample rate in DELTA-N register
	//
	//   60     0      1    1       0       0      0 0 0       External memory write via ADPCM data register $08
	//   20     0      0    1       0       0      0 0 0       External memory read via ADPCM data register $08
	//
	m_portstate = value & 0xf1;  // start, rec, memory mode, repeat flag copy, reset(bit0)
	if (BIT(m_portstate, 7)) // START,REC,MEMDATA,REPEAT,SPOFF,--,--,RESET
	{
		status_set_reset(STATUS_BUSY);

		// start ADPCM
		m_curfrac  = 0;
		m_adpcm_acc = 0;
		m_prev_acc = 0;
		m_output = 0;
		m_adpcm_step = DELTA_DEFAULT;
		m_curbyte = 0;
	}

	if (BIT(m_portstate, 5)) // do we access external memory?
	{
		m_curaddress = start_address();
		m_memread = 2;    // two dummy reads needed before accesing external memory via register $08
	}
	else // we access CPU memory (ADPCM data register $08) so we only reset m_curaddress here
		m_curaddress = 0;

	if (BIT(m_portstate, 0))
	{
		m_portstate = 0x00;
		status_set_reset(STATUS_BRDY, STATUS_BUSY);
	}
}

void opn_deltat_channel_t::set_pan_control2(u8 value, u8 shift_override)
{
	m_control2 = value;

	// low 2 bits:
	//   0-DRAM x1, 1-ROM, 2-DRAM x8, 3-ROM (3 is bad setting - not allowed by the manual)
	//
	// final shift value depends on chip type and memory type selected:
	//		8 for YM2610 (ROM only),
	//		5 for ROM for Y8950 and YM2608,
	//		5 for x8bit DRAMs for Y8950 and YM2608,
	//		2 for x1bit DRAMs for Y8950 and YM2608.
	m_addrshift = ((value & 3) == 0) ? 2 : 5;
	if (shift_override != 0)
		m_addrshift = shift_override;

	// add 1 to the address shift since we work in nibbles
	m_addrshift++;
}

void opn_deltat_channel_t::write_data(u8 value)
{
	// external memory write
	if ((m_portstate & 0xe0) == 0x60)
	{
		// clear out dummy reads and set start address
		if (m_memread != 0)
		{
			m_curaddress = start_address();
			m_memread = 0;
		}

		// before end?
		if (m_curaddress != end_address())
		{
			m_opn.deltat_write(m_curaddress, value);
			m_curaddress += 2;

			// reset BRDY bit in status register, which means we are processing the write
			status_set_reset(0, STATUS_BRDY);

			// should go high again after 10 master clock cycles (Y8950) but this is not simulated
			status_set_reset(STATUS_BRDY);
		}
		else
			status_set_reset(STATUS_EOS);
	}

	// ADPCM synthesis from CPU
	else if ((m_portstate & 0xe0) == 0x80)
	{
		m_cpudata = value;
		status_set_reset(0, STATUS_BRDY);
	}
}

u8 opn_deltat_channel_t::read_data()
{
	u8 result = 0;

	// external memory read
	if ((m_portstate & 0xe0) == 0x20)
	{
		// two dummy reads
		if (m_memread != 0)
		{
			m_curaddress = start_address();
			m_memread--;
		}

		// before end?
		else if (m_curaddress != end_address())
		{
			result = m_opn.deltat_read(m_curaddress);
			m_curaddress += 2;

			// reset BRDY bit in status register, which means we are reading the memory now
			status_set_reset(0, STATUS_BRDY);

			// should go high again after 10 master clock cycles (Y8950) but this is not simulated
			status_set_reset(STATUS_BRDY);
		}
		else
			status_set_reset(STATUS_EOS);
	}
	return result;
}

bool opn_deltat_channel_t::update(u32 freqbase)
{
	// Forecast to next Forecast (rate = *8)
	// 1/8 , 3/8 , 5/8 , 7/8 , 9/8 , 11/8 , 13/8 , 15/8
	static constexpr s8 s_decode_b1[16] =
	{
		 1,   3,   5,   7,   9,  11,  13,  15,
		-1,  -3,  -5,  -7,  -9, -11, -13, -15,
	};

	// delta to next delta (rate= *64)
	// 0.9 , 0.9 , 0.9 , 0.9 , 1.2 , 1.6 , 2.0 , 2.4
	static constexpr s16 s_decode_b2[16] =
	{
		57,  57,  57,  57, 77, 102, 128, 153,
		57,  57,  57,  57, 77, 102, 128, 153
	};

	// only process if active
	if (!BIT(m_portstate, 7))
		return false;

	// determine which mode we are operating in
	bool external_memory;
	if ((m_portstate & 0xe0) == 0xa0)
		external_memory = true;
	else if ((m_portstate & 0xe0) == 0x80)
		external_memory = false;
	else
		return false;

	// process any samples once we cross the 1.0 fractional boundary
	// freqbase is x.1, so shift one less
	for (m_curfrac += (m_delta * freqbase) >> 1; m_curfrac >= FRAC_ONE; m_curfrac -= FRAC_ONE)
	{
		// in external memory mode, check the end address and process
		if (external_memory)
		{
			// wrap at the limit address
			if (m_curaddress == limit_address())
				m_curaddress = 0;

			// handle the sample end, either repeating or stopping
			if (m_curaddress == end_address())
			{
				if (BIT(m_portstate, 4))
				{
					// repeat start
					m_curaddress = start_address();
					m_adpcm_acc = m_prev_acc = 0;
					m_adpcm_step = DELTA_DEFAULT;
				}
				else
				{
					status_set_reset(STATUS_EOS, STATUS_BUSY);
					m_portstate = 0;
					m_output = 0;
					m_prev_acc = 0;
					return true;
				}
			}
		}

		// determine which nibble (0 = low, 1 = high)
		int nibble = m_curaddress & 1;

		// in external memory mode, fetch the data when we first need it
		if (external_memory && nibble == 0)
			m_curbyte = m_opn.deltat_read(m_curaddress >> 1);

		// extract the nibble
		u8 data = u8(m_curbyte << (4 * nibble)) >> 4;

		// in CPU-driven mode, after we consume the upper nibble, copy the next byte and request more
		if (!external_memory && nibble == 1)
		{
			m_curbyte = m_cpudata;
			status_set_reset(STATUS_BRDY, 0);
		}

		// YM2610 address register is 24 bits wide.
		// The "+1" is there because we use 1 bit more for nibble calculations.
		m_curaddress++;
		m_curaddress &= (1 << 25) - 1;

		// remember previous value for interpolation
		m_prev_acc = m_adpcm_acc;

		// forecast to next forecast
		m_adpcm_acc += s_decode_b1[data] * m_adpcm_step / 8;
		if (m_adpcm_acc > 32767)
			m_adpcm_acc = 32767;
		else if (m_adpcm_acc < -32768)
			m_adpcm_acc = -32768;

		// delta to next delta
		m_adpcm_step = (m_adpcm_step * s_decode_b2[data]) / 64;
		if (m_adpcm_step > DELTA_MAX)
			m_adpcm_step = DELTA_MAX;
		else if (m_adpcm_step < DELTA_MIN)
			m_adpcm_step = DELTA_MIN;
	}

	// interpolate between samples
	m_output = (m_prev_acc * s32(FRAC_ONE - m_curfrac) + m_adpcm_acc * s32(m_curfrac)) >> FRAC_SHIFT;
	return false;
}



//*********************************************************
// OPN BASE DEVICE CLASS
//*********************************************************

//-------------------------------------------------
//  ymopn_device_base - constructor
//-------------------------------------------------

ymopn_device_base::ymopn_device_base(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type) :
	ay8910_device(mconfig, type, tag, owner, clock, PSG_TYPE_YM, (type == YM2203) ? 3 : 1, (type == YM2610 || type == YM2610B) ? 0 : 2),
	m_adpcm_status(0),
	m_eg_count(0),
	m_eg_timer(0),
	m_lfo_am(0),
	m_lfo_pm(0),
	m_lfo_count(0),
	m_lfo_step(0),
	m_prescaler_sel(0),
	m_freqbase(CLOCK_DIVIDER * 2),
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

	// allocate the appropriate number of ADPCM channels
	int adpcm = (type == YM2203) ? 0 : 6;
	for (int index = 0; index < adpcm; index++)
		m_adpcm_channel.push_back(std::make_unique<opn_adpcm_channel_t>(*this));

	// allocate the deltat channel
	bool deltat = (type != YM2203);
	if (deltat)
		m_deltat_channel = std::make_unique<opn_deltat_channel_t>(*this);
}

//-------------------------------------------------
//  status_set_reset - set/reset bits in the status
//  register, signaling IRQs according to the mask
//-------------------------------------------------

void ymopn_device_base::status_set_reset(u8 set, u8 reset)
{
	m_status = (m_status | set) & ~reset;
	bool irq = ((m_status & m_irqmask) != 0);
	if (irq != m_irq)
	{
		m_irq = irq;
		synchronize(TIMER_IRQ_SYNC, m_irq);
	}
}


//-------------------------------------------------
//  adpcm_read - read a byte of ADPCM data
//-------------------------------------------------

u8 ymopn_device_base::adpcm_read(offs_t offset)
{
	// default implementation is empty
	return 0;
}

u8 ym2608_device::adpcm_read(offs_t offset)
{
	// ADPCM reads from the internal ROM
	return m_internal->as_u8(offset % m_internal->bytes());
}

u8 ym2610_device::adpcm_read(offs_t offset)
{
	// read from space 0
	return space(0).read_byte(offset);
}


//-------------------------------------------------
//  deltat_read - read a byte of delta-T ADPCM data
//-------------------------------------------------

u8 ymopn_device_base::deltat_read(offs_t offset)
{
	// default implementation is empty
	return 0;
}

u8 ym2608_device::deltat_read(offs_t offset)
{
	return space(0).read_byte(offset);
}

u8 ym2610_device::deltat_read(offs_t offset)
{
	return space(1).read_byte(offset);
}


//-------------------------------------------------
//  deltat_write - write a byte of delta-T ADPCM
//  data
//-------------------------------------------------

void ymopn_device_base::deltat_write(offs_t offset, u8 data)
{
	// default implementation is empty
}

void ym2608_device::deltat_write(offs_t offset, u8 data)
{
	space(0).write_byte(offset, data);
}


//-------------------------------------------------
//  deltat_status_change - process a status change
//  from the delta-T ADPCM unit
//-------------------------------------------------

void ymopn_device_base::deltat_status_change(u8 newstatus)
{
	// default implementation is empty
}

void ym2608_device::deltat_status_change(u8 newstatus)
{
	u8 set = 0;
	u8 reset = 0;

	// map EOS flag to the status register
	if ((newstatus & opn_deltat_channel_t::STATUS_EOS) != 0)
		set |= STATUS_DELTAT_EOS_2608;
	else
		reset |= STATUS_DELTAT_EOS_2608;

	// map BRDY flag to the status register
	if ((newstatus & opn_deltat_channel_t::STATUS_BRDY) != 0)
		set |= STATUS_DELTAT_BRDY_2608;
	else
		reset |= STATUS_DELTAT_BRDY_2608;

	// pass the bits through the regular status/IRQ mechanism
	status_set_reset(set, reset);
}


//-------------------------------------------------
//  set_irqmask - set the IRQ mask
//-------------------------------------------------

void ymopn_device_base::set_irqmask(u8 flag)
{
	m_irqmask = flag;
	status_set_reset();
}


//-------------------------------------------------
//  status - return the current status
//-------------------------------------------------

u8 ymopn_device_base::status()
{
	u8 status = m_status;
	if (machine().time() < m_busy_expiry_time)
		status |= STATUS_BUSY;
	return status;
}


//-------------------------------------------------
//  busy_set - set the busy flag expiration time
//-------------------------------------------------

void ymopn_device_base::busy_set()
{
	attotime expiry_period = attotime::from_hz(clock()) * m_timer_prescaler;
	m_busy_expiry_time = machine().time() + expiry_period;
}


//-------------------------------------------------
//  timer_a_period - return the period of timer A,
//  in attotime
//-------------------------------------------------

attotime ymopn_device_base::timer_a_period() const
{
	return ((1024 - m_timer_a_value) * m_timer_prescaler) * attotime::from_hz(clock());
}


//-------------------------------------------------
//  timer_b_period - return the period of timer B,
//  in attotime
//-------------------------------------------------

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
		status_set_reset(0, STATUS_TIMERB);

	// reset Timer a flag
	if (BIT(value, 4))
		status_set_reset(0, STATUS_TIMERA);

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
//  write_adpcm - handle writes to ADPCM registers
//-------------------------------------------------

void ymopn_device_base::write_adpcm(u8 reg, u8 value)
{
	// determine channel
	u8 chnum = reg & 7;
	if (chnum >= m_adpcm_channel.size())
		return;
	opn_adpcm_channel_t &chan = *m_adpcm_channel[chnum];

	switch (reg & 0xf8)
	{
		case 0x00:
			if (chnum == 0)			// DM,--,C5,C4,C3,C2,C1,C0
			{
				for (int chnum = 0; chnum < m_adpcm_channel.size(); chnum++)
					if (BIT(value, chnum))
						m_adpcm_channel[chnum]->keyonoff(BIT(~value, 7));
			}
			else if (chnum == 1)	// B0-5 = TL
			{
				for (auto &chan : m_adpcm_channel)
					chan->set_tl(~value & 0x3f);
			}
			break;

		case 0x08:
			chan.set_volume_pan(value);
			break;

		case 0x10:
			chan.set_start_byte(value, 0);
			break;

		case 0x18:
			chan.set_start_byte(value, 8);
			break;

		case 0x20:
			chan.set_end_byte(value, 0);
			break;

		case 0x28:
			chan.set_end_byte(value, 8);
			break;
	}
}


//-------------------------------------------------
//  write_deltat - handle writes to DeltaT
//  registers
//-------------------------------------------------

void ymopn_device_base::write_deltat(u8 reg, u8 value)
{
	// determine channel
	if (!m_deltat_channel)
		return;
	opn_deltat_channel_t &chan = *m_deltat_channel;

	switch (reg)
	{
		case 0x00:
			// YM2610 always uses external memory and has no record support
			if (type() == YM2610 || type() == YM2610B)
				value = (value | 0x20) & ~0x40;
			chan.set_portstate(value);
			break;

		case 0x01:  // L,R,-,-,SAMPLE,DA/AD,RAMTYPE,ROM
			// YM2610 always uses ROM as an external memory and doesn't tave ROM/RAM memory flag bit
			// it also has a fixed address shift of 8, which overrides the normal ones
			if (type() == YM2610 || type() == YM2610B)
				chan.set_pan_control2(value | 0x01, 8);
			else
				chan.set_pan_control2(value);
			break;

		case 0x02:  // start address low
			chan.set_start_byte(value, 0);
			break;

		case 0x03:  // start address high
			chan.set_start_byte(value, 8);
			break;

		case 0x04:  // end address low
			chan.set_end_byte(value, 0);
			break;

		case 0x05:  // end address high
			chan.set_end_byte(value, 8);
			break;

		case 0x06:  // prescale low/high (for record -- not supported)
		case 0x07:
			break;

		case 0x08:  // data write
			chan.write_data(value);
			break;

		case 0x09:  // delta-N low
			chan.set_delta_byte(value, 0);
			break;

		case 0x0a:  // delta-N high
			chan.set_delta_byte(value, 8);
			break;

		case 0x0b:  // output level control
			chan.set_volume(value);
			break;

		case 0x0c:  // mimit address low
			chan.set_limit_byte(value, 0);
			break;

		case 0x0d:  // limit address high
			chan.set_limit_byte(value, 8);
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

	m_freqbase = (CLOCK_DIVIDER * 2) / (s_opn_pres[sel] * pre_divider);
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
				status_set_reset(STATUS_TIMERA);
			m_timer_a->adjust(timer_a_period());
			break;

		case TIMER_B:
			if (BIT(m_mode, 3))
				status_set_reset(STATUS_TIMERB);
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

		case TIMER_WRITE_ADPCM:
			m_stream->update();
			write_adpcm(param >> 8, param & 0xff);
			break;

		case TIMER_WRITE_DELTAT:
			m_stream->update();
			write_deltat(param >> 8, param & 0xff);
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
	for (auto &chan : m_channel)
		chan->refresh_fc_eg();

	// buffering
	for (int sampindex = 0; sampindex < outputs[0].samples(); sampindex++)
	{
		// advance the LFO
		advance_lfo();

		// advance envelope generator
		// freqbase is x.1, so shift by one less
		m_eg_timer += m_freqbase << (EG_SHIFT - 1);
		while (m_eg_timer >= EG_TIMER_OVERFLOW)
		{
			m_eg_timer -= EG_TIMER_OVERFLOW;
			m_eg_count++;

			for (auto &chan : m_channel)
				chan->advance_eg(m_eg_count);
		}

		// calculate FM
		s32 lsum = 0, rsum = 0;
		for (auto &chan : m_channel)
		{
			chan->update(m_lfo_am, m_lfo_pm);
			lsum += chan->output_l();
			rsum += chan->output_r();
		}

		// calculate delta-T ADPCM
		if (m_deltat_channel)
		{
			auto &chan = *m_deltat_channel;
			if (chan.update(m_freqbase))
				m_adpcm_status |= 0x80;
			lsum += chan.output_l();
			rsum += chan.output_r();
		}

		// calculate ADPCM
		for (int chnum = 0; chnum < m_adpcm_channel.size(); chnum++)
		{
			auto &chan = *m_adpcm_channel[chnum];
			if (chan.update(m_freqbase))
				m_adpcm_status |= 1 << chnum;
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

#define YM_NAME(x) x, "ym." #x
void ymopn_device_base::device_start()
{
	ay8910_device::device_start();

	m_irq_handler.resolve();

	m_timer_a = timer_alloc(TIMER_A);
	m_timer_b = timer_alloc(TIMER_B);

	m_stream = &stream_alloc_ex(0, (type() == YM2203) ? 1 : 2, clock() / CLOCK_DIVIDER);

	init_tables();

	save_item(YM_NAME(m_3slot_state.m_fc));
	save_item(YM_NAME(m_3slot_state.m_fn_h));
	save_item(YM_NAME(m_3slot_state.m_kcode));
	save_item(YM_NAME(m_3slot_state.m_block_fnum));
	for (int chnum = 0; chnum < m_channel.size(); chnum++)
		m_channel[chnum]->save(chnum);
	for (int chnum = 0; chnum < m_adpcm_channel.size(); chnum++)
		m_adpcm_channel[chnum]->save(chnum);
	if (m_deltat_channel)
		m_deltat_channel->save(0);

	save_item(YM_NAME(m_eg_count));
	save_item(YM_NAME(m_eg_timer));

	save_item(YM_NAME(m_lfo_am));
	save_item(YM_NAME(m_lfo_pm));
	save_item(YM_NAME(m_lfo_count));
	save_item(YM_NAME(m_lfo_step));

	save_item(YM_NAME(m_prescaler_sel));
	save_item(YM_NAME(m_freqbase));
	save_item(YM_NAME(m_timer_prescaler));

	save_item(YM_NAME(m_busy_expiry_time));
	save_item(YM_NAME(m_irq));
	save_item(YM_NAME(m_irqmask));
	save_item(YM_NAME(m_status));
	save_item(YM_NAME(m_adpcm_status));
	save_item(YM_NAME(m_mode));
	save_item(YM_NAME(m_fn_h));

	save_item(YM_NAME(m_timer_a_value));
	save_item(YM_NAME(m_timer_b_value));
}

void ym2203_device::device_start()
{
	ymopn_device_base::device_start();

	save_item(YM_NAME(m_address));
}

void ym2608_device::device_start()
{
	ymopn_device_base::device_start();

	save_item(YM_NAME(m_address));
	save_item(YM_NAME(m_2608_irqmask));
	save_item(YM_NAME(m_2608_flagmask));
}

void ym2610_device::device_start()
{
	ymopn_device_base::device_start();

	save_item(YM_NAME(m_address));
	save_item(YM_NAME(m_2610_flagmask));

	if (!has_configured_map(0) && !has_configured_map(1))
	{
		if (m_adpcm_a_region)
			space(0).install_rom(0, m_adpcm_a_region->bytes() - 1, m_adpcm_a_region->base());

		if (m_adpcm_b_region)
			space(1).install_rom(0, m_adpcm_b_region->bytes() - 1, m_adpcm_b_region->base());
		else if (m_adpcm_a_region)
			space(1).install_rom(0, m_adpcm_a_region->bytes() - 1, m_adpcm_a_region->base());
	}
}



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
	// reset prescaler
	set_prescale(2);

	// reset SSG section
	ay8910_device::device_reset();

	m_busy_expiry_time = attotime::zero;
	m_mode = 0;
	m_timer_a_value = 0;
	m_timer_b_value = 0;

	m_eg_timer = 0;
	m_eg_count = 0;
	m_adpcm_status = 0;

	for (auto &chan : m_channel)
		chan->reset();
	for (auto &chan : m_adpcm_channel)
		chan->reset();
	if (m_deltat_channel)
		m_deltat_channel->reset();
}

void ym2203_device::device_reset()
{
	// status clear
	set_irqmask(0x03);
	set_mode(0x30); // mode 0, timer reset
	status_set_reset(0, 0xff);
	ymopn_device_base::device_reset();

	// reset operator paramater
	for (int regnum = 0xb2; regnum >= 0x30; regnum--)
		write_reg(regnum, 0);
	for (int regnum = 0x26; regnum >= 0x20; regnum--)
		write_reg(regnum, 0);
}

void ym2608_device::device_reset()
{
	// register 0x29 - default value after reset is:
	// enable only 3 FM channels and enable all the status flags
	irq_mask_write(0x1f);

	// register 0x110: default value is 1 for D4, D3, D2, 0 for the rest
	irq_flag_write(0x1c);

	set_mode(0x30); // mode 0, timer reset
	status_set_reset(0, 0xff);
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

	// configure ADPCM percussion sounds
	static const u32 s_rom_addresses[2*6] =
	{
		0x0000, 0x01bf, // bass drum
		0x01c0, 0x043f, // snare drum
		0x0440, 0x1b7f, // top cymbal
		0x1b80, 0x1cff, // high hat
		0x1d00, 0x1f7f, // tom tom
		0x1f80, 0x1fff  // rim shot
	};
	for (int chnum = 0; chnum < 6; chnum++)
	{
		auto &chan = adpcm(chnum);
		chan.set_start(s_rom_addresses[chnum * 2]);
		chan.set_end(s_rom_addresses[chnum * 2 + 1]);

		// channels 4 and 5 work with a slower clock
		chan.set_step_divisor((chnum <= 3) ? 1 : 2);
	}
}

void ym2610_device::device_reset()
{
	// status clear
	set_irqmask(0x03);
	set_mode(0x30); // mode 0, timer reset
	status_set_reset(0, 0xff);
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

	// initialize delta-T ADPCM address shift
	write_deltat(0x01, 0xc0);

	m_2610_flagmask = 0xbf;
}


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
	u8 result = 0;
	switch (offset & 1)
	{
		case 0:	// status port
			result = status();
			break;

		case 1: // data port (only SSG)
			if (m_address < 16)
				result = ay8910_read_ym();
			break;
	}
	return result;
}

u8 ym2608_device::read(offs_t offset)
{
	u8 result = 0;
	switch (offset & 3)
	{
		case 0: // status 0 : YM2203 compatible
			// BUSY:x:x:x:x:x:FLAGB:FLAGA
			result = status() & 0x83;
			break;

		case 1: // status 0, ID
			if (m_address < 16)
				result = ay8910_read_ym();
			else if (m_address == 0xff)
				result = 1;  // ID code
			break;

		case 2: // status 1 : status 0 + ADPCM status
			// BUSY:x:PCMBUSY:ZERO:BRDY:EOS:FLAGB:FLAGA
			result = status() & (m_2608_flagmask | 0x80);
			if ((deltat().status() & opn_deltat_channel_t::STATUS_BUSY) != 0)
				result |= 0x20;
			break;

		case 3:
			if (m_address == 0x08)
				result = deltat().read_data();
			else if (m_address == 0x0f)
			{
				logerror("YM2608 A/D conversion is accessed but not implemented !\n");
				result = 0x80; // 2's complement PCM data - result from A/D conversion
			}
			break;
	}
	return result;
}

u8 ym2610_device::read(offs_t offset)
{
	u8 result = 0;
	switch (offset & 3)
	{
		case 0: // status 0 : YM2203 compatible
			// BUSY:x:x:x:x:x:FLAGB:FLAGA
			result = status() & 0x83;
			break;

		case 1: // status 0, ID
			if (m_address < 16)
				result = ay8910_read_ym();
			else if (m_address == 0xff)
				result = 1;  // ID code
			break;

		case 2: // status 1 : ADPCM status
			// ADPCM STATUS (arrived End Address)
			// B,--,A5,A4,A3,A2,A1,A0
			// B     = ADPCM-B(DELTA-T) arrived end address
			// A0-A5 = ADPCM-A          arrived end address
			result = m_adpcm_status & m_2610_flagmask;
			break;

		case 3:
			break;
	}
	return result;
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ym2203_device::write(offs_t offset, u8 value)
{
	switch (offset & 1)
	{
		case 0:	// address port
			m_address = value;

			// write register to SSG emulator
			if (m_address < 16)
				ay8910_write_ym(0, m_address);

			// prescaler select : 2d,2e,2f
			if (m_address >= 0x2d && m_address <= 0x2f)
				prescaler_w(m_address);
			break;

		case 1: // data port
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
			break;
	}
}

void ym2608_device::write(offs_t offset, u8 value)
{
	switch (offset & 3)
	{
		case 0:	// address port 0
			m_address = value;

			// write register to SSG emulator
			if (m_address < 16)
				ay8910_write_ym(0, m_address);

			// prescaler select : 2d,2e,2f
			if (m_address >= 0x2d && m_address <= 0x2f)
				prescaler_w(m_address);
			break;

		case 1: // data port 0
			if (BIT(m_address, 8))
				break; // verified on real YM2608

			switch (m_address & 0xf0)
			{
				case 0x00:  // 0x00-0x0f : SSG section
					ay8910_write_ym(1, value);
					break;

				case 0x10:
					synchronize(TIMER_WRITE_ADPCM, value | ((m_address & 0xf) << 8));
					break;

				case 0x20:  // 0x20-0x2f : Mode section
					if (m_address == 0x29)
						irq_mask_write(value);
					else
						synchronize(TIMER_WRITE_MODE, value | (m_address << 8));
					break;

				default:    // 0x30-0xff : OPN section
					synchronize(TIMER_WRITE_REG, value | (m_address << 8));
					break;
			}
			busy_set();
			break;

		case 2: // address port 1
			m_address = value | 0x100;
			break;

		case 3: // data port 1
			if (!BIT(m_address, 8))
				break; // verified on real YM2608

			switch (m_address & 0xf0)
			{
				case 0x00:  // DELTAT port
					if (m_address < 0x10e)
						synchronize(TIMER_WRITE_DELTAT, value | ((m_address & 0xf) << 8));
					else if (m_address == 0x10e)
						logerror("YM2608: write to DAC data (unimplemented) value=%02x\n", value);
					break;

				case 0x10:  // IRQ flag control
					if (m_address == 0x110)
						irq_flag_write(value);
					break;

				default:    // 0x30-0xff : OPN section
					synchronize(TIMER_WRITE_REG, value | (m_address << 8));
					break;
			}
			busy_set();
			break;
	}
}

void ym2610_device::write(offs_t offset, u8 value)
{
	switch (offset & 3)
	{
		case 0:	// address port 0
			m_address = value;

			// write register to SSG emulator
			if (m_address < 16)
				ay8910_write_ym(0, m_address);
			break;

		case 1: // data port 0
			if (BIT(m_address, 8))
				break; // verified on real YM2608

			switch (m_address & 0xf0)
			{
				case 0x00:  // 0x00-0x0f : SSG section
					ay8910_write_ym(1, value);
					break;

				case 0x10:
					if (m_address < 0x1c)
						synchronize(TIMER_WRITE_DELTAT, value | ((m_address & 0xf) << 8));
					else if (m_address == 0x1c)
					{
						m_2610_flagmask = ~value;
						m_adpcm_status &= ~value;
					}
					break;

				case 0x20:  // 0x20-0x2f : Mode section
					synchronize(TIMER_WRITE_MODE, value | (m_address << 8));
					break;

				default:    // 0x30-0xff : OPN section
					synchronize(TIMER_WRITE_REG, value | (m_address << 8));
					break;
			}
			busy_set();
			break;

		case 2: // address port 1
			m_address = value | 0x100;
			break;

		case 3: // data port 1
			if (!BIT(m_address, 8))
				break; // verified on real YM2608

			switch (m_address & 0xf0)
			{
				case 0x00:  // DELTAT port
				case 0x10:  // DELTAT port
				case 0x20:  // DELTAT port
					synchronize(TIMER_WRITE_ADPCM, value | ((m_address & 0x3f) << 8));
					break;

				default:    // 0x30-0xff : OPN section
					synchronize(TIMER_WRITE_REG, value | (m_address << 8));
					break;
			}
			busy_set();
			break;
	}
}


















/* flag enable control 0x110 */
void ym2608_device::irq_flag_write(u8 value)
{
	if (BIT(value, 7))
	{
		// reset IRQ flag; don't touch BUFRDY flag otherwise we'd
		// have to call ymdeltat module to set the flag back
		status_set_reset(0, 0xf7);
	}
	else
	{
		// set status flag mask
		m_2608_flagmask = ~(value & 0x1f);
		set_irqmask(m_2608_irqmask & m_2608_flagmask);
	}
}

/* compatible mode & IRQ enable control 0x29 */
void ym2608_device::irq_mask_write(u8 value)
{
	// SCH,xx,xxx,EN_ZERO,EN_BRDY,EN_EOS,EN_TB,EN_TA

	// extend 3ch. enable/disable
	for (int chnum = 3; chnum < 6; chnum++)
		channel(chnum).set_disabled(BIT(~value, 7));

	// IRQ MASK store and set
	m_2608_irqmask = value & 0x1f;
	set_irqmask(m_2608_irqmask & m_2608_flagmask);
}

ym2203_device::ym2203_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymopn_device_base(mconfig, tag, owner, clock, YM2203)
{
}

ym2608_device::ym2608_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymopn_device_base(mconfig, tag, owner, clock, YM2608),
	device_rom_interface(mconfig, *this),
	m_internal(*this, "internal")
{
}

ROM_START( ym2608 )
	ROM_REGION( 0x2000, "internal", 0 )
	//
	// While this rom was dumped by output analysis, not decap, it was tested
	// by playing it back into the chip as an external adpcm sample and produced
	// an identical dac result. a decap would be nice to verify things 100%,
	// but there is currently no reason to think this rom dump is incorrect.
	//
	// offset 0:
	//    Source: 01BD.ROM
	//     Length: 448 / 0x000001C0
	// offset 0x1C0:
	//     Source: 02SD.ROM
	//     Length: 640 / 0x00000280
	// offset 0x440:
	//     Source: 04TOP.ROM
	//     Length: 5952 / 0x00001740
	// offset 0x1B80:
	//     Source: 08HH.ROM
	//     Length: 384 / 0x00000180
	// offset 0x1D00
	//     Source: 10TOM.ROM
	//     Length: 640 / 0x00000280
	// offset 0x1F80
	//     Source: 20RIM.ROM
	//     Length: 128 / 0x00000080
	//
	ROM_LOAD16_WORD( "ym2608_adpcm_rom.bin", 0x0000, 0x2000, CRC(23c9e0d8) SHA1(50b6c3e288eaa12ad275d4f323267bb72b0445df) )
ROM_END

const tiny_rom_entry *ym2608_device::device_rom_region() const
{
	return ROM_NAME( ym2608 );
}

void ym2608_device::rom_bank_updated()
{
	stream_update();
}

ym2610_device::ym2610_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type) :
	ymopn_device_base(mconfig, tag, owner, clock, type),
	device_memory_interface(mconfig, *this),
	m_adpcm_a_config("adpcm-a", ENDIANNESS_LITTLE, 8, 24, 0),
	m_adpcm_b_config("adpcm-b", ENDIANNESS_LITTLE, 8, 24, 0),
	m_adpcm_b_region_name("^" + std::string(basetag()) + ".deltat"),
	m_adpcm_a_region(*this, DEVICE_SELF),
	m_adpcm_b_region(*this, m_adpcm_b_region_name.c_str())
{
	channel(0).set_disabled(type == YM2610);
	channel(3).set_disabled(type == YM2610);
}

ym2610b_device::ym2610b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ym2610_device(mconfig, tag, owner, clock, YM2610B)
{
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector ym2610_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(0, &m_adpcm_a_config),
		std::make_pair(1, &m_adpcm_b_config)
	};
}

DEFINE_DEVICE_TYPE(YM2203, ym2203_device, "ym2203", "YM2203 OPN")
DEFINE_DEVICE_TYPE(YM2608, ym2608_device, "ym2608", "YM2608 OPN")
DEFINE_DEVICE_TYPE(YM2610, ym2610_device, "ym2610", "YM2610 OPNB")
DEFINE_DEVICE_TYPE(YM2610B, ym2610b_device, "ym2610b", "YM2610B OPNB")
