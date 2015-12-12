// license:???
// copyright-holders:Jarek Burczynski
/*
**
** File: ymf262.c - software implementation of YMF262
**                  FM sound generator type OPL3
**
** Copyright Jarek Burczynski
**
** Version 0.2
**

Revision History:

03-03-2003: initial release
 - thanks to Olivier Galibert and Chris Hardy for YMF262 and YAC512 chips
 - thanks to Stiletto for the datasheets

   Features as listed in 4MF262A6 data sheet:
    1. Registers are compatible with YM3812 (OPL2) FM sound source.
    2. Up to six sounds can be used as four-operator melody sounds for variety.
    3. 18 simultaneous melody sounds, or 15 melody sounds with 5 rhythm sounds (with two operators).
    4. 6 four-operator melody sounds and 6 two-operator melody sounds, or 6 four-operator melody
       sounds, 3 two-operator melody sounds and 5 rhythm sounds (with four operators).
    5. 8 selectable waveforms.
    6. 4-channel sound output.
    7. YMF262 compabile DAC (YAC512) is available.
    8. LFO for vibrato and tremolo effedts.
    9. 2 programable timers.
   10. Shorter register access time compared with YM3812.
   11. 5V single supply silicon gate CMOS process.
   12. 24 Pin SOP Package (YMF262-M), 48 Pin SQFP Package (YMF262-S).


differences between OPL2 and OPL3 not documented in Yamaha datahasheets:
- sinus table is a little different: the negative part is off by one...

- in order to enable selection of four different waveforms on OPL2
  one must set bit 5 in register 0x01(test).
  on OPL3 this bit is ignored and 4-waveform select works *always*.
  (Don't confuse this with OPL3's 8-waveform select.)

- Envelope Generator: all 15 x rates take zero time on OPL3
  (on OPL2 15 0 and 15 1 rates take some time while 15 2 and 15 3 rates
  take zero time)

- channel calculations: output of operator 1 is in perfect sync with
  output of operator 2 on OPL3; on OPL and OPL2 output of operator 1
  is always delayed by one sample compared to output of operator 2


differences between OPL2 and OPL3 shown in datasheets:
- YMF262 does not support CSM mode


*/

#include "emu.h"
#include "ymf262.h"



/* output final shift */
#if (OPL3_SAMPLE_BITS==16)
	#define FINAL_SH    (0)
	#define MAXOUT      (+32767)
	#define MINOUT      (-32768)
#else
	#define FINAL_SH    (8)
	#define MAXOUT      (+127)
	#define MINOUT      (-128)
#endif


#define FREQ_SH         16  /* 16.16 fixed point (frequency calculations) */
#define EG_SH           16  /* 16.16 fixed point (EG timing)              */
#define LFO_SH          24  /*  8.24 fixed point (LFO calculations)       */
#define TIMER_SH        16  /* 16.16 fixed point (timers calculations)    */

#define FREQ_MASK       ((1<<FREQ_SH)-1)

/* envelope output entries */
#define ENV_BITS        10
#define ENV_LEN         (1<<ENV_BITS)
#define ENV_STEP        (128.0/ENV_LEN)

#define MAX_ATT_INDEX   ((1<<(ENV_BITS-1))-1) /*511*/
#define MIN_ATT_INDEX   (0)

/* sinwave entries */
#define SIN_BITS        10
#define SIN_LEN         (1<<SIN_BITS)
#define SIN_MASK        (SIN_LEN-1)

#define TL_RES_LEN      (256)   /* 8 bits addressing (real chip) */



/* register number to channel number , slot offset */
#define SLOT1 0
#define SLOT2 1

/* Envelope Generator phases */

#define EG_ATT          4
#define EG_DEC          3
#define EG_SUS          2
#define EG_REL          1
#define EG_OFF          0


/* save output as raw 16-bit sample */

/*#define SAVE_SAMPLE*/

#ifdef SAVE_SAMPLE
static FILE *sample[1];
	#if 1   /*save to MONO file */
		#define SAVE_ALL_CHANNELS \
		{   signed int pom = a; \
			fputc((unsigned short)pom&0xff,sample[0]); \
			fputc(((unsigned short)pom>>8)&0xff,sample[0]); \
		}
	#else   /*save to STEREO file */
		#define SAVE_ALL_CHANNELS \
		{   signed int pom = a; \
			fputc((unsigned short)pom&0xff,sample[0]); \
			fputc(((unsigned short)pom>>8)&0xff,sample[0]); \
			pom = b; \
			fputc((unsigned short)pom&0xff,sample[0]); \
			fputc(((unsigned short)pom>>8)&0xff,sample[0]); \
		}
	#endif
#endif

#define LOG_CYM_FILE 0
static FILE * cymfile = nullptr;





#define OPL3_TYPE_YMF262 (0)    /* 36 operators, 8 waveforms */


struct OPL3_SLOT
{
	UINT32  ar;         /* attack rate: AR<<2           */
	UINT32  dr;         /* decay rate:  DR<<2           */
	UINT32  rr;         /* release rate:RR<<2           */
	UINT8   KSR;        /* key scale rate               */
	UINT8   ksl;        /* keyscale level               */
	UINT8   ksr;        /* key scale rate: kcode>>KSR   */
	UINT8   mul;        /* multiple: mul_tab[ML]        */

	/* Phase Generator */
	UINT32  Cnt;        /* frequency counter            */
	UINT32  Incr;       /* frequency counter step       */
	UINT8   FB;         /* feedback shift value         */
	INT32   *connect;   /* slot output pointer          */
	INT32   op1_out[2]; /* slot1 output for feedback    */
	UINT8   CON;        /* connection (algorithm) type  */

	/* Envelope Generator */
	UINT8   eg_type;    /* percussive/non-percussive mode */
	UINT8   state;      /* phase type                   */
	UINT32  TL;         /* total level: TL << 2         */
	INT32   TLL;        /* adjusted now TL              */
	INT32   volume;     /* envelope counter             */
	UINT32  sl;         /* sustain level: sl_tab[SL]    */

	UINT32  eg_m_ar;    /* (attack state)               */
	UINT8   eg_sh_ar;   /* (attack state)               */
	UINT8   eg_sel_ar;  /* (attack state)               */
	UINT32  eg_m_dr;    /* (decay state)                */
	UINT8   eg_sh_dr;   /* (decay state)                */
	UINT8   eg_sel_dr;  /* (decay state)                */
	UINT32  eg_m_rr;    /* (release state)              */
	UINT8   eg_sh_rr;   /* (release state)              */
	UINT8   eg_sel_rr;  /* (release state)              */

	UINT32  key;        /* 0 = KEY OFF, >0 = KEY ON     */

	/* LFO */
	UINT32  AMmask;     /* LFO Amplitude Modulation enable mask */
	UINT8   vib;        /* LFO Phase Modulation enable flag (active high)*/

	/* waveform select */
	UINT8   waveform_number;
	unsigned int wavetable;

//unsigned char reserved[128-84];//speedup: pump up the struct size to power of 2
unsigned char reserved[128-100];//speedup: pump up the struct size to power of 2

};

struct OPL3_CH
{
	OPL3_SLOT SLOT[2];

	UINT32  block_fnum; /* block+fnum                   */
	UINT32  fc;         /* Freq. Increment base         */
	UINT32  ksl_base;   /* KeyScaleLevel Base step      */
	UINT8   kcode;      /* key code (for key scaling)   */

	/*
	   there are 12 2-operator channels which can be combined in pairs
	   to form six 4-operator channel, they are:
	    0 and 3,
	    1 and 4,
	    2 and 5,
	    9 and 12,
	    10 and 13,
	    11 and 14
	*/
	UINT8   extended;   /* set to 1 if this channel forms up a 4op channel with another channel(only used by first of pair of channels, ie 0,1,2 and 9,10,11) */

unsigned char reserved[512-272];//speedup:pump up the struct size to power of 2

};

/* OPL3 state */
struct OPL3
{
	OPL3_CH P_CH[18];               /* OPL3 chips have 18 channels  */

	UINT32  pan[18*4];              /* channels output masks (0xffffffff = enable); 4 masks per one channel */
	UINT32  pan_ctrl_value[18];     /* output control values 1 per one channel (1 value contains 4 masks) */

	signed int chanout[18];
	signed int phase_modulation;        /* phase modulation input (SLOT 2) */
	signed int phase_modulation2;   /* phase modulation input (SLOT 3 in 4 operator channels) */

	UINT32  eg_cnt;                 /* global envelope generator counter    */
	UINT32  eg_timer;               /* global envelope generator counter works at frequency = chipclock/288 (288=8*36) */
	UINT32  eg_timer_add;           /* step of eg_timer                     */
	UINT32  eg_timer_overflow;      /* envelope generator timer overlfows every 1 sample (on real chip) */

	UINT32  fn_tab[1024];           /* fnumber->increment counter   */

	/* LFO */
	UINT32  LFO_AM;
	INT32   LFO_PM;

	UINT8   lfo_am_depth;
	UINT8   lfo_pm_depth_range;
	UINT32  lfo_am_cnt;
	UINT32  lfo_am_inc;
	UINT32  lfo_pm_cnt;
	UINT32  lfo_pm_inc;

	UINT32  noise_rng;              /* 23 bit noise shift register  */
	UINT32  noise_p;                /* current noise 'phase'        */
	UINT32  noise_f;                /* current noise period         */

	UINT8   OPL3_mode;              /* OPL3 extension enable flag   */

	UINT8   rhythm;                 /* Rhythm mode                  */

	int     T[2];                   /* timer counters               */
	UINT8   st[2];                  /* timer enable                 */

	UINT32  address;                /* address register             */
	UINT8   status;                 /* status flag                  */
	UINT8   statusmask;             /* status mask                  */

	UINT8   nts;                    /* NTS (note select)            */

	/* external event callback handlers */
	OPL3_TIMERHANDLER  timer_handler;/* TIMER handler                */
	void *TimerParam;                   /* TIMER parameter              */
	OPL3_IRQHANDLER    IRQHandler;  /* IRQ handler                  */
	void *IRQParam;                 /* IRQ parameter                */
	OPL3_UPDATEHANDLER UpdateHandler;/* stream update handler       */
	void *UpdateParam;              /* stream update parameter      */

	UINT8 type;                     /* chip type                    */
	int clock;                      /* master clock  (Hz)           */
	int rate;                       /* sampling rate (Hz)           */
	double freqbase;                /* frequency base               */
	attotime TimerBase;         /* Timer base time (==sampling time)*/
	device_t *device;
};



/* mapping of register number (offset) to slot number used by the emulator */
static const int slot_array[32]=
{
		0, 2, 4, 1, 3, 5,-1,-1,
		6, 8,10, 7, 9,11,-1,-1,
	12,14,16,13,15,17,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1
};

/* key scale level */
/* table is 3dB/octave , DV converts this into 6dB/octave */
/* 0.1875 is bit 0 weight of the envelope counter (volume) expressed in the 'decibel' scale */
#define DV (0.1875/2.0)
static const double ksl_tab[8*16]=
{
	/* OCT 0 */
		0.000/DV, 0.000/DV, 0.000/DV, 0.000/DV,
		0.000/DV, 0.000/DV, 0.000/DV, 0.000/DV,
		0.000/DV, 0.000/DV, 0.000/DV, 0.000/DV,
		0.000/DV, 0.000/DV, 0.000/DV, 0.000/DV,
	/* OCT 1 */
		0.000/DV, 0.000/DV, 0.000/DV, 0.000/DV,
		0.000/DV, 0.000/DV, 0.000/DV, 0.000/DV,
		0.000/DV, 0.750/DV, 1.125/DV, 1.500/DV,
		1.875/DV, 2.250/DV, 2.625/DV, 3.000/DV,
	/* OCT 2 */
		0.000/DV, 0.000/DV, 0.000/DV, 0.000/DV,
		0.000/DV, 1.125/DV, 1.875/DV, 2.625/DV,
		3.000/DV, 3.750/DV, 4.125/DV, 4.500/DV,
		4.875/DV, 5.250/DV, 5.625/DV, 6.000/DV,
	/* OCT 3 */
		0.000/DV, 0.000/DV, 0.000/DV, 1.875/DV,
		3.000/DV, 4.125/DV, 4.875/DV, 5.625/DV,
		6.000/DV, 6.750/DV, 7.125/DV, 7.500/DV,
		7.875/DV, 8.250/DV, 8.625/DV, 9.000/DV,
	/* OCT 4 */
		0.000/DV, 0.000/DV, 3.000/DV, 4.875/DV,
		6.000/DV, 7.125/DV, 7.875/DV, 8.625/DV,
		9.000/DV, 9.750/DV,10.125/DV,10.500/DV,
		10.875/DV,11.250/DV,11.625/DV,12.000/DV,
	/* OCT 5 */
		0.000/DV, 3.000/DV, 6.000/DV, 7.875/DV,
		9.000/DV,10.125/DV,10.875/DV,11.625/DV,
		12.000/DV,12.750/DV,13.125/DV,13.500/DV,
		13.875/DV,14.250/DV,14.625/DV,15.000/DV,
	/* OCT 6 */
		0.000/DV, 6.000/DV, 9.000/DV,10.875/DV,
		12.000/DV,13.125/DV,13.875/DV,14.625/DV,
		15.000/DV,15.750/DV,16.125/DV,16.500/DV,
		16.875/DV,17.250/DV,17.625/DV,18.000/DV,
	/* OCT 7 */
		0.000/DV, 9.000/DV,12.000/DV,13.875/DV,
		15.000/DV,16.125/DV,16.875/DV,17.625/DV,
		18.000/DV,18.750/DV,19.125/DV,19.500/DV,
		19.875/DV,20.250/DV,20.625/DV,21.000/DV
};
#undef DV

/* 0 / 3.0 / 1.5 / 6.0 dB/OCT */
static const UINT32 ksl_shift[4] = { 31, 1, 2, 0 };


/* sustain level table (3dB per step) */
/* 0 - 15: 0, 3, 6, 9,12,15,18,21,24,27,30,33,36,39,42,93 (dB)*/
#define SC(db) (UINT32) ( db * (2.0/ENV_STEP) )
static const UINT32 sl_tab[16]={
	SC( 0),SC( 1),SC( 2),SC(3 ),SC(4 ),SC(5 ),SC(6 ),SC( 7),
	SC( 8),SC( 9),SC(10),SC(11),SC(12),SC(13),SC(14),SC(31)
};
#undef SC


#define RATE_STEPS (8)
static const unsigned char eg_inc[15*RATE_STEPS]={
/*cycle:0 1  2 3  4 5  6 7*/

/* 0 */ 0,1, 0,1, 0,1, 0,1, /* rates 00..12 0 (increment by 0 or 1) */
/* 1 */ 0,1, 0,1, 1,1, 0,1, /* rates 00..12 1 */
/* 2 */ 0,1, 1,1, 0,1, 1,1, /* rates 00..12 2 */
/* 3 */ 0,1, 1,1, 1,1, 1,1, /* rates 00..12 3 */

/* 4 */ 1,1, 1,1, 1,1, 1,1, /* rate 13 0 (increment by 1) */
/* 5 */ 1,1, 1,2, 1,1, 1,2, /* rate 13 1 */
/* 6 */ 1,2, 1,2, 1,2, 1,2, /* rate 13 2 */
/* 7 */ 1,2, 2,2, 1,2, 2,2, /* rate 13 3 */

/* 8 */ 2,2, 2,2, 2,2, 2,2, /* rate 14 0 (increment by 2) */
/* 9 */ 2,2, 2,4, 2,2, 2,4, /* rate 14 1 */
/*10 */ 2,4, 2,4, 2,4, 2,4, /* rate 14 2 */
/*11 */ 2,4, 4,4, 2,4, 4,4, /* rate 14 3 */

/*12 */ 4,4, 4,4, 4,4, 4,4, /* rates 15 0, 15 1, 15 2, 15 3 for decay */
/*13 */ 8,8, 8,8, 8,8, 8,8, /* rates 15 0, 15 1, 15 2, 15 3 for attack (zero time) */
/*14 */ 0,0, 0,0, 0,0, 0,0, /* infinity rates for attack and decay(s) */
};


#define O(a) (a*RATE_STEPS)

/* note that there is no O(13) in this table - it's directly in the code */
static const unsigned char eg_rate_select[16+64+16]={   /* Envelope Generator rates (16 + 64 rates + 16 RKS) */
/* 16 infinite time rates */
O(14),O(14),O(14),O(14),O(14),O(14),O(14),O(14),
O(14),O(14),O(14),O(14),O(14),O(14),O(14),O(14),

/* rates 00-12 */
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
O( 0),O( 1),O( 2),O( 3),

/* rate 13 */
O( 4),O( 5),O( 6),O( 7),

/* rate 14 */
O( 8),O( 9),O(10),O(11),

/* rate 15 */
O(12),O(12),O(12),O(12),

/* 16 dummy rates (same as 15 3) */
O(12),O(12),O(12),O(12),O(12),O(12),O(12),O(12),
O(12),O(12),O(12),O(12),O(12),O(12),O(12),O(12),

};
#undef O

/*rate  0,    1,    2,    3,   4,   5,   6,  7,  8,  9,  10, 11, 12, 13, 14, 15 */
/*shift 12,   11,   10,   9,   8,   7,   6,  5,  4,  3,  2,  1,  0,  0,  0,  0  */
/*mask  4095, 2047, 1023, 511, 255, 127, 63, 31, 15, 7,  3,  1,  0,  0,  0,  0  */

#define O(a) (a*1)
static const unsigned char eg_rate_shift[16+64+16]={    /* Envelope Generator counter shifts (16 + 64 rates + 16 RKS) */
/* 16 infinite time rates */
O(0),O(0),O(0),O(0),O(0),O(0),O(0),O(0),
O(0),O(0),O(0),O(0),O(0),O(0),O(0),O(0),

/* rates 00-12 */
O(12),O(12),O(12),O(12),
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

/* rate 13 */
O( 0),O( 0),O( 0),O( 0),

/* rate 14 */
O( 0),O( 0),O( 0),O( 0),

/* rate 15 */
O( 0),O( 0),O( 0),O( 0),

/* 16 dummy rates (same as 15 3) */
O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),
O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),

};
#undef O


/* multiple table */
#define ML 2
static const UINT8 mul_tab[16]= {
/* 1/2, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,10,12,12,15,15 */
	ML/2, 1*ML, 2*ML, 3*ML, 4*ML, 5*ML, 6*ML, 7*ML,
	8*ML, 9*ML,10*ML,10*ML,12*ML,12*ML,15*ML,15*ML
};
#undef ML

/*  TL_TAB_LEN is calculated as:

*   (12+1)=13 - sinus amplitude bits     (Y axis)
*   additional 1: to compensate for calculations of negative part of waveform
*   (if we don't add it then the greatest possible _negative_ value would be -2
*   and we really need -1 for waveform #7)
*   2  - sinus sign bit           (Y axis)
*   TL_RES_LEN - sinus resolution (X axis)
*/
#define TL_TAB_LEN (13*2*TL_RES_LEN)
static signed int tl_tab[TL_TAB_LEN];

#define ENV_QUIET       (TL_TAB_LEN>>4)

/* sin waveform table in 'decibel' scale */
/* there are eight waveforms on OPL3 chips */
static unsigned int sin_tab[SIN_LEN * 8];


/* LFO Amplitude Modulation table (verified on real YM3812)
   27 output levels (triangle waveform); 1 level takes one of: 192, 256 or 448 samples

   Length: 210 elements.

    Each of the elements has to be repeated
    exactly 64 times (on 64 consecutive samples).
    The whole table takes: 64 * 210 = 13440 samples.

    When AM = 1 data is used directly
    When AM = 0 data is divided by 4 before being used (losing precision is important)
*/

#define LFO_AM_TAB_ELEMENTS 210

static const UINT8 lfo_am_table[LFO_AM_TAB_ELEMENTS] = {
0,0,0,0,0,0,0,
1,1,1,1,
2,2,2,2,
3,3,3,3,
4,4,4,4,
5,5,5,5,
6,6,6,6,
7,7,7,7,
8,8,8,8,
9,9,9,9,
10,10,10,10,
11,11,11,11,
12,12,12,12,
13,13,13,13,
14,14,14,14,
15,15,15,15,
16,16,16,16,
17,17,17,17,
18,18,18,18,
19,19,19,19,
20,20,20,20,
21,21,21,21,
22,22,22,22,
23,23,23,23,
24,24,24,24,
25,25,25,25,
26,26,26,
25,25,25,25,
24,24,24,24,
23,23,23,23,
22,22,22,22,
21,21,21,21,
20,20,20,20,
19,19,19,19,
18,18,18,18,
17,17,17,17,
16,16,16,16,
15,15,15,15,
14,14,14,14,
13,13,13,13,
12,12,12,12,
11,11,11,11,
10,10,10,10,
9,9,9,9,
8,8,8,8,
7,7,7,7,
6,6,6,6,
5,5,5,5,
4,4,4,4,
3,3,3,3,
2,2,2,2,
1,1,1,1
};

/* LFO Phase Modulation table (verified on real YM3812) */
static const INT8 lfo_pm_table[8*8*2] = {
/* FNUM2/FNUM = 00 0xxxxxxx (0x0000) */
0, 0, 0, 0, 0, 0, 0, 0, /*LFO PM depth = 0*/
0, 0, 0, 0, 0, 0, 0, 0, /*LFO PM depth = 1*/

/* FNUM2/FNUM = 00 1xxxxxxx (0x0080) */
0, 0, 0, 0, 0, 0, 0, 0, /*LFO PM depth = 0*/
1, 0, 0, 0,-1, 0, 0, 0, /*LFO PM depth = 1*/

/* FNUM2/FNUM = 01 0xxxxxxx (0x0100) */
1, 0, 0, 0,-1, 0, 0, 0, /*LFO PM depth = 0*/
2, 1, 0,-1,-2,-1, 0, 1, /*LFO PM depth = 1*/

/* FNUM2/FNUM = 01 1xxxxxxx (0x0180) */
1, 0, 0, 0,-1, 0, 0, 0, /*LFO PM depth = 0*/
3, 1, 0,-1,-3,-1, 0, 1, /*LFO PM depth = 1*/

/* FNUM2/FNUM = 10 0xxxxxxx (0x0200) */
2, 1, 0,-1,-2,-1, 0, 1, /*LFO PM depth = 0*/
4, 2, 0,-2,-4,-2, 0, 2, /*LFO PM depth = 1*/

/* FNUM2/FNUM = 10 1xxxxxxx (0x0280) */
2, 1, 0,-1,-2,-1, 0, 1, /*LFO PM depth = 0*/
5, 2, 0,-2,-5,-2, 0, 2, /*LFO PM depth = 1*/

/* FNUM2/FNUM = 11 0xxxxxxx (0x0300) */
3, 1, 0,-1,-3,-1, 0, 1, /*LFO PM depth = 0*/
6, 3, 0,-3,-6,-3, 0, 3, /*LFO PM depth = 1*/

/* FNUM2/FNUM = 11 1xxxxxxx (0x0380) */
3, 1, 0,-1,-3,-1, 0, 1, /*LFO PM depth = 0*/
7, 3, 0,-3,-7,-3, 0, 3  /*LFO PM depth = 1*/
};


/* lock level of common table */
static int num_lock = 0;

/* work table */
#define SLOT7_1 (&chip->P_CH[7].SLOT[SLOT1])
#define SLOT7_2 (&chip->P_CH[7].SLOT[SLOT2])
#define SLOT8_1 (&chip->P_CH[8].SLOT[SLOT1])
#define SLOT8_2 (&chip->P_CH[8].SLOT[SLOT2])





static inline int limit( int val, int max, int min ) {
	if ( val > max )
		val = max;
	else if ( val < min )
		val = min;

	return val;
}


/* status set and IRQ handling */
static inline void OPL3_STATUS_SET(OPL3 *chip,int flag)
{
	/* set status flag masking out disabled IRQs */
	chip->status |= (flag & chip->statusmask);
	if(!(chip->status & 0x80))
	{
		if(chip->status & 0x7f)
		{   /* IRQ on */
			chip->status |= 0x80;
			/* callback user interrupt handler (IRQ is OFF to ON) */
			if(chip->IRQHandler) (chip->IRQHandler)(chip->IRQParam,1);
		}
	}
}

/* status reset and IRQ handling */
static inline void OPL3_STATUS_RESET(OPL3 *chip,int flag)
{
	/* reset status flag */
	chip->status &= ~flag;
	if(chip->status & 0x80)
	{
		if (!(chip->status & 0x7f))
		{
			chip->status &= 0x7f;
			/* callback user interrupt handler (IRQ is ON to OFF) */
			if(chip->IRQHandler) (chip->IRQHandler)(chip->IRQParam,0);
		}
	}
}

/* IRQ mask set */
static inline void OPL3_STATUSMASK_SET(OPL3 *chip,int flag)
{
	chip->statusmask = flag;
	/* IRQ handling check */
	OPL3_STATUS_SET(chip,0);
	OPL3_STATUS_RESET(chip,0);
}


/* advance LFO to next sample */
static inline void advance_lfo(OPL3 *chip)
{
	UINT8 tmp;

	/* LFO */
	chip->lfo_am_cnt += chip->lfo_am_inc;
	if (chip->lfo_am_cnt >= ((UINT32)LFO_AM_TAB_ELEMENTS<<LFO_SH) ) /* lfo_am_table is 210 elements long */
		chip->lfo_am_cnt -= ((UINT32)LFO_AM_TAB_ELEMENTS<<LFO_SH);

	tmp = lfo_am_table[ chip->lfo_am_cnt >> LFO_SH ];

	if (chip->lfo_am_depth)
		chip->LFO_AM = tmp;
	else
		chip->LFO_AM = tmp>>2;

	chip->lfo_pm_cnt += chip->lfo_pm_inc;
	chip->LFO_PM = ((chip->lfo_pm_cnt>>LFO_SH) & 7) | chip->lfo_pm_depth_range;
}

/* advance to next sample */
static inline void advance(OPL3 *chip)
{
	OPL3_CH *CH;
	OPL3_SLOT *op;
	int i;

	chip->eg_timer += chip->eg_timer_add;

	while (chip->eg_timer >= chip->eg_timer_overflow)
	{
		chip->eg_timer -= chip->eg_timer_overflow;

		chip->eg_cnt++;

		for (i=0; i<9*2*2; i++)
		{
			CH  = &chip->P_CH[i/2];
			op  = &CH->SLOT[i&1];
#if 1
			/* Envelope Generator */
			switch(op->state)
			{
			case EG_ATT:    /* attack phase */
//              if ( !(chip->eg_cnt & ((1<<op->eg_sh_ar)-1) ) )
				if ( !(chip->eg_cnt & op->eg_m_ar) )
				{
					op->volume += (~op->volume *
												(eg_inc[op->eg_sel_ar + ((chip->eg_cnt>>op->eg_sh_ar)&7)])
												) >>3;

					if (op->volume <= MIN_ATT_INDEX)
					{
						op->volume = MIN_ATT_INDEX;
						op->state = EG_DEC;
					}

				}
			break;

			case EG_DEC:    /* decay phase */
//              if ( !(chip->eg_cnt & ((1<<op->eg_sh_dr)-1) ) )
				if ( !(chip->eg_cnt & op->eg_m_dr) )
				{
					op->volume += eg_inc[op->eg_sel_dr + ((chip->eg_cnt>>op->eg_sh_dr)&7)];

					if ( op->volume >= op->sl )
						op->state = EG_SUS;

				}
			break;

			case EG_SUS:    /* sustain phase */

				/* this is important behaviour:
				one can change percusive/non-percussive modes on the fly and
				the chip will remain in sustain phase - verified on real YM3812 */

				if(op->eg_type)     /* non-percussive mode */
				{
									/* do nothing */
				}
				else                /* percussive mode */
				{
					/* during sustain phase chip adds Release Rate (in percussive mode) */
//                  if ( !(chip->eg_cnt & ((1<<op->eg_sh_rr)-1) ) )
					if ( !(chip->eg_cnt & op->eg_m_rr) )
					{
						op->volume += eg_inc[op->eg_sel_rr + ((chip->eg_cnt>>op->eg_sh_rr)&7)];

						if ( op->volume >= MAX_ATT_INDEX )
							op->volume = MAX_ATT_INDEX;
					}
					/* else do nothing in sustain phase */
				}
			break;

			case EG_REL:    /* release phase */
//              if ( !(chip->eg_cnt & ((1<<op->eg_sh_rr)-1) ) )
				if ( !(chip->eg_cnt & op->eg_m_rr) )
				{
					op->volume += eg_inc[op->eg_sel_rr + ((chip->eg_cnt>>op->eg_sh_rr)&7)];

					if ( op->volume >= MAX_ATT_INDEX )
					{
						op->volume = MAX_ATT_INDEX;
						op->state = EG_OFF;
					}

				}
			break;

			default:
			break;
			}
#endif
		}
	}

	for (i=0; i<9*2*2; i++)
	{
		CH  = &chip->P_CH[i/2];
		op  = &CH->SLOT[i&1];

		/* Phase Generator */
		if(op->vib)
		{
			UINT8 block;
			unsigned int block_fnum = CH->block_fnum;

			unsigned int fnum_lfo   = (block_fnum&0x0380) >> 7;

			signed int lfo_fn_table_index_offset = lfo_pm_table[chip->LFO_PM + 16*fnum_lfo ];

			if (lfo_fn_table_index_offset)  /* LFO phase modulation active */
			{
				block_fnum += lfo_fn_table_index_offset;
				block = (block_fnum&0x1c00) >> 10;
				op->Cnt += (chip->fn_tab[block_fnum&0x03ff] >> (7-block)) * op->mul;
			}
			else    /* LFO phase modulation  = zero */
			{
				op->Cnt += op->Incr;
			}
		}
		else    /* LFO phase modulation disabled for this operator */
		{
			op->Cnt += op->Incr;
		}
	}

	/*  The Noise Generator of the YM3812 is 23-bit shift register.
	*   Period is equal to 2^23-2 samples.
	*   Register works at sampling frequency of the chip, so output
	*   can change on every sample.
	*
	*   Output of the register and input to the bit 22 is:
	*   bit0 XOR bit14 XOR bit15 XOR bit22
	*
	*   Simply use bit 22 as the noise output.
	*/

	chip->noise_p += chip->noise_f;
	i = chip->noise_p >> FREQ_SH;       /* number of events (shifts of the shift register) */
	chip->noise_p &= FREQ_MASK;
	while (i)
	{
		/*
		UINT32 j;
		j = ( (chip->noise_rng) ^ (chip->noise_rng>>14) ^ (chip->noise_rng>>15) ^ (chip->noise_rng>>22) ) & 1;
		chip->noise_rng = (j<<22) | (chip->noise_rng>>1);
		*/

		/*
		    Instead of doing all the logic operations above, we
		    use a trick here (and use bit 0 as the noise output).
		    The difference is only that the noise bit changes one
		    step ahead. This doesn't matter since we don't know
		    what is real state of the noise_rng after the reset.
		*/

		if (chip->noise_rng & 1) chip->noise_rng ^= 0x800302;
		chip->noise_rng >>= 1;

		i--;
	}
}


static inline signed int op_calc(UINT32 phase, unsigned int env, signed int pm, unsigned int wave_tab)
{
	UINT32 p;

	p = (env<<4) + sin_tab[wave_tab + ((((signed int)((phase & ~FREQ_MASK) + (pm<<16))) >> FREQ_SH ) & SIN_MASK) ];

	if (p >= TL_TAB_LEN)
		return 0;
	return tl_tab[p];
}

static inline signed int op_calc1(UINT32 phase, unsigned int env, signed int pm, unsigned int wave_tab)
{
	UINT32 p;

	p = (env<<4) + sin_tab[wave_tab + ((((signed int)((phase & ~FREQ_MASK) + pm))>>FREQ_SH) & SIN_MASK)];

	if (p >= TL_TAB_LEN)
		return 0;
	return tl_tab[p];
}


#define volume_calc(OP) ((OP)->TLL + ((UINT32)(OP)->volume) + (chip->LFO_AM & (OP)->AMmask))

/* calculate output of a standard 2 operator channel
 (or 1st part of a 4-op channel) */
static inline void chan_calc( OPL3 *chip, OPL3_CH *CH )
{
	OPL3_SLOT *SLOT;
	unsigned int env;
	signed int out;

	chip->phase_modulation = 0;
	chip->phase_modulation2= 0;

	/* SLOT 1 */
	SLOT = &CH->SLOT[SLOT1];
	env  = volume_calc(SLOT);
	out  = SLOT->op1_out[0] + SLOT->op1_out[1];
	SLOT->op1_out[0] = SLOT->op1_out[1];
	SLOT->op1_out[1] = 0;
	if( env < ENV_QUIET )
	{
		if (!SLOT->FB)
			out = 0;
		SLOT->op1_out[1] = op_calc1(SLOT->Cnt, env, (out<<SLOT->FB), SLOT->wavetable );
	}
	*SLOT->connect += SLOT->op1_out[1];
//logerror("out0=%5i vol0=%4i ", SLOT->op1_out[1], env );

	/* SLOT 2 */
	SLOT++;
	env = volume_calc(SLOT);
	if( env < ENV_QUIET )
		*SLOT->connect += op_calc(SLOT->Cnt, env, chip->phase_modulation, SLOT->wavetable);

//logerror("out1=%5i vol1=%4i\n", op_calc(SLOT->Cnt, env, chip->phase_modulation, SLOT->wavetable), env );

}

/* calculate output of a 2nd part of 4-op channel */
static inline void chan_calc_ext( OPL3 *chip, OPL3_CH *CH )
{
	OPL3_SLOT *SLOT;
	unsigned int env;

	chip->phase_modulation = 0;

	/* SLOT 1 */
	SLOT = &CH->SLOT[SLOT1];
	env  = volume_calc(SLOT);
	if( env < ENV_QUIET )
		*SLOT->connect += op_calc(SLOT->Cnt, env, chip->phase_modulation2, SLOT->wavetable );

	/* SLOT 2 */
	SLOT++;
	env = volume_calc(SLOT);
	if( env < ENV_QUIET )
		*SLOT->connect += op_calc(SLOT->Cnt, env, chip->phase_modulation, SLOT->wavetable);

}

/*
    operators used in the rhythm sounds generation process:

    Envelope Generator:

channel  operator  register number   Bass  High  Snare Tom  Top
/ slot   number    TL ARDR SLRR Wave Drum  Hat   Drum  Tom  Cymbal
 6 / 0   12        50  70   90   f0  +
 6 / 1   15        53  73   93   f3  +
 7 / 0   13        51  71   91   f1        +
 7 / 1   16        54  74   94   f4              +
 8 / 0   14        52  72   92   f2                    +
 8 / 1   17        55  75   95   f5                          +

    Phase Generator:

channel  operator  register number   Bass  High  Snare Tom  Top
/ slot   number    MULTIPLE          Drum  Hat   Drum  Tom  Cymbal
 6 / 0   12        30                +
 6 / 1   15        33                +
 7 / 0   13        31                      +     +           +
 7 / 1   16        34                -----  n o t  u s e d -----
 8 / 0   14        32                                  +
 8 / 1   17        35                      +                 +

channel  operator  register number   Bass  High  Snare Tom  Top
number   number    BLK/FNUM2 FNUM    Drum  Hat   Drum  Tom  Cymbal
   6     12,15     B6        A6      +

   7     13,16     B7        A7            +     +           +

   8     14,17     B8        A8            +           +     +

*/

/* calculate rhythm */

static inline void chan_calc_rhythm( OPL3 *chip, OPL3_CH *CH, unsigned int noise )
{
	OPL3_SLOT *SLOT;
	signed int *chanout = chip->chanout;
	signed int out;
	unsigned int env;


	/* Bass Drum (verified on real YM3812):
	  - depends on the channel 6 'connect' register:
	      when connect = 0 it works the same as in normal (non-rhythm) mode (op1->op2->out)
	      when connect = 1 _only_ operator 2 is present on output (op2->out), operator 1 is ignored
	  - output sample always is multiplied by 2
	*/

	chip->phase_modulation = 0;

	/* SLOT 1 */
	SLOT = &CH[6].SLOT[SLOT1];
	env = volume_calc(SLOT);

	out = SLOT->op1_out[0] + SLOT->op1_out[1];
	SLOT->op1_out[0] = SLOT->op1_out[1];

	if (!SLOT->CON)
		chip->phase_modulation = SLOT->op1_out[0];
	//else ignore output of operator 1

	SLOT->op1_out[1] = 0;
	if( env < ENV_QUIET )
	{
		if (!SLOT->FB)
			out = 0;
		SLOT->op1_out[1] = op_calc1(SLOT->Cnt, env, (out<<SLOT->FB), SLOT->wavetable );
	}

	/* SLOT 2 */
	SLOT++;
	env = volume_calc(SLOT);
	if( env < ENV_QUIET )
		chanout[6] += op_calc(SLOT->Cnt, env, chip->phase_modulation, SLOT->wavetable) * 2;


	/* Phase generation is based on: */
	// HH  (13) channel 7->slot 1 combined with channel 8->slot 2 (same combination as TOP CYMBAL but different output phases)
	// SD  (16) channel 7->slot 1
	// TOM (14) channel 8->slot 1
	// TOP (17) channel 7->slot 1 combined with channel 8->slot 2 (same combination as HIGH HAT but different output phases)

	/* Envelope generation based on: */
	// HH  channel 7->slot1
	// SD  channel 7->slot2
	// TOM channel 8->slot1
	// TOP channel 8->slot2


	/* The following formulas can be well optimized.
	   I leave them in direct form for now (in case I've missed something).
	*/

	/* High Hat (verified on real YM3812) */
	env = volume_calc(SLOT7_1);
	if( env < ENV_QUIET )
	{
		/* high hat phase generation:
		    phase = d0 or 234 (based on frequency only)
		    phase = 34 or 2d0 (based on noise)
		*/

		/* base frequency derived from operator 1 in channel 7 */
		unsigned char bit7 = ((SLOT7_1->Cnt>>FREQ_SH)>>7)&1;
		unsigned char bit3 = ((SLOT7_1->Cnt>>FREQ_SH)>>3)&1;
		unsigned char bit2 = ((SLOT7_1->Cnt>>FREQ_SH)>>2)&1;

		unsigned char res1 = (bit2 ^ bit7) | bit3;

		/* when res1 = 0 phase = 0x000 | 0xd0; */
		/* when res1 = 1 phase = 0x200 | (0xd0>>2); */
		UINT32 phase = res1 ? (0x200|(0xd0>>2)) : 0xd0;

		/* enable gate based on frequency of operator 2 in channel 8 */
		unsigned char bit5e= ((SLOT8_2->Cnt>>FREQ_SH)>>5)&1;
		unsigned char bit3e= ((SLOT8_2->Cnt>>FREQ_SH)>>3)&1;

		unsigned char res2 = (bit3e ^ bit5e);

		/* when res2 = 0 pass the phase from calculation above (res1); */
		/* when res2 = 1 phase = 0x200 | (0xd0>>2); */
		if (res2)
			phase = (0x200|(0xd0>>2));


		/* when phase & 0x200 is set and noise=1 then phase = 0x200|0xd0 */
		/* when phase & 0x200 is set and noise=0 then phase = 0x200|(0xd0>>2), ie no change */
		if (phase&0x200)
		{
			if (noise)
				phase = 0x200|0xd0;
		}
		else
		/* when phase & 0x200 is clear and noise=1 then phase = 0xd0>>2 */
		/* when phase & 0x200 is clear and noise=0 then phase = 0xd0, ie no change */
		{
			if (noise)
				phase = 0xd0>>2;
		}

		chanout[7] += op_calc(phase<<FREQ_SH, env, 0, SLOT7_1->wavetable) * 2;
	}

	/* Snare Drum (verified on real YM3812) */
	env = volume_calc(SLOT7_2);
	if( env < ENV_QUIET )
	{
		/* base frequency derived from operator 1 in channel 7 */
		unsigned char bit8 = ((SLOT7_1->Cnt>>FREQ_SH)>>8)&1;

		/* when bit8 = 0 phase = 0x100; */
		/* when bit8 = 1 phase = 0x200; */
		UINT32 phase = bit8 ? 0x200 : 0x100;

		/* Noise bit XOR'es phase by 0x100 */
		/* when noisebit = 0 pass the phase from calculation above */
		/* when noisebit = 1 phase ^= 0x100; */
		/* in other words: phase ^= (noisebit<<8); */
		if (noise)
			phase ^= 0x100;

		chanout[7] += op_calc(phase<<FREQ_SH, env, 0, SLOT7_2->wavetable) * 2;
	}

	/* Tom Tom (verified on real YM3812) */
	env = volume_calc(SLOT8_1);
	if( env < ENV_QUIET )
		chanout[8] += op_calc(SLOT8_1->Cnt, env, 0, SLOT8_1->wavetable) * 2;

	/* Top Cymbal (verified on real YM3812) */
	env = volume_calc(SLOT8_2);
	if( env < ENV_QUIET )
	{
		/* base frequency derived from operator 1 in channel 7 */
		unsigned char bit7 = ((SLOT7_1->Cnt>>FREQ_SH)>>7)&1;
		unsigned char bit3 = ((SLOT7_1->Cnt>>FREQ_SH)>>3)&1;
		unsigned char bit2 = ((SLOT7_1->Cnt>>FREQ_SH)>>2)&1;

		unsigned char res1 = (bit2 ^ bit7) | bit3;

		/* when res1 = 0 phase = 0x000 | 0x100; */
		/* when res1 = 1 phase = 0x200 | 0x100; */
		UINT32 phase = res1 ? 0x300 : 0x100;

		/* enable gate based on frequency of operator 2 in channel 8 */
		unsigned char bit5e= ((SLOT8_2->Cnt>>FREQ_SH)>>5)&1;
		unsigned char bit3e= ((SLOT8_2->Cnt>>FREQ_SH)>>3)&1;

		unsigned char res2 = (bit3e ^ bit5e);
		/* when res2 = 0 pass the phase from calculation above (res1); */
		/* when res2 = 1 phase = 0x200 | 0x100; */
		if (res2)
			phase = 0x300;

		chanout[8] += op_calc(phase<<FREQ_SH, env, 0, SLOT8_2->wavetable) * 2;
	}

}


/* generic table initialize */
static int init_tables(void)
{
	signed int i,x;
	signed int n;
	double o,m;


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
		n <<= 1;        /* 12 bits here (as in real chip) */
		tl_tab[ x*2 + 0 ] = n;
		tl_tab[ x*2 + 1 ] = ~tl_tab[ x*2 + 0 ]; /* this *is* different from OPL2 (verified on real YMF262) */

		for (i=1; i<13; i++)
		{
			tl_tab[ x*2+0 + i*2*TL_RES_LEN ] =  tl_tab[ x*2+0 ]>>i;
			tl_tab[ x*2+1 + i*2*TL_RES_LEN ] = ~tl_tab[ x*2+0 + i*2*TL_RES_LEN ];  /* this *is* different from OPL2 (verified on real YMF262) */
		}
	#if 0
			logerror("tl %04i", x*2);
			for (i=0; i<13; i++)
				logerror(", [%02i] %5i", i*2, tl_tab[ x*2 +0 + i*2*TL_RES_LEN ] ); /* positive */
			logerror("\n");

			logerror("tl %04i", x*2);
			for (i=0; i<13; i++)
				logerror(", [%02i] %5i", i*2, tl_tab[ x*2 +1 + i*2*TL_RES_LEN ] ); /* negative */
			logerror("\n");
	#endif
	}

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
		if (n&1)                        /* round to nearest */
			n = (n>>1)+1;
		else
			n = n>>1;

		sin_tab[ i ] = n*2 + (m>=0.0? 0: 1 );

		/*logerror("YMF262.C: sin [%4i (hex=%03x)]= %4i (tl_tab value=%5i)\n", i, i, sin_tab[i], tl_tab[sin_tab[i]] );*/
	}

	for (i=0; i<SIN_LEN; i++)
	{
		/* these 'pictures' represent _two_ cycles */
		/* waveform 1:  __      __     */
		/*             /  \____/  \____*/
		/* output only first half of the sinus waveform (positive one) */

		if (i & (1<<(SIN_BITS-1)) )
			sin_tab[1*SIN_LEN+i] = TL_TAB_LEN;
		else
			sin_tab[1*SIN_LEN+i] = sin_tab[i];

		/* waveform 2:  __  __  __  __ */
		/*             /  \/  \/  \/  \*/
		/* abs(sin) */

		sin_tab[2*SIN_LEN+i] = sin_tab[i & (SIN_MASK>>1) ];

		/* waveform 3:  _   _   _   _  */
		/*             / |_/ |_/ |_/ |_*/
		/* abs(output only first quarter of the sinus waveform) */

		if (i & (1<<(SIN_BITS-2)) )
			sin_tab[3*SIN_LEN+i] = TL_TAB_LEN;
		else
			sin_tab[3*SIN_LEN+i] = sin_tab[i & (SIN_MASK>>2)];

		/* waveform 4:                 */
		/*             /\  ____/\  ____*/
		/*               \/      \/    */
		/* output whole sinus waveform in half the cycle(step=2) and output 0 on the other half of cycle */

		if (i & (1<<(SIN_BITS-1)) )
			sin_tab[4*SIN_LEN+i] = TL_TAB_LEN;
		else
			sin_tab[4*SIN_LEN+i] = sin_tab[i*2];

		/* waveform 5:                 */
		/*             /\/\____/\/\____*/
		/*                             */
		/* output abs(whole sinus) waveform in half the cycle(step=2) and output 0 on the other half of cycle */

		if (i & (1<<(SIN_BITS-1)) )
			sin_tab[5*SIN_LEN+i] = TL_TAB_LEN;
		else
			sin_tab[5*SIN_LEN+i] = sin_tab[(i*2) & (SIN_MASK>>1) ];

		/* waveform 6: ____    ____    */
		/*                             */
		/*                 ____    ____*/
		/* output maximum in half the cycle and output minimum on the other half of cycle */

		if (i & (1<<(SIN_BITS-1)) )
			sin_tab[6*SIN_LEN+i] = 1;   /* negative */
		else
			sin_tab[6*SIN_LEN+i] = 0;   /* positive */

		/* waveform 7:                 */
		/*             |\____  |\____  */
		/*                   \|      \|*/
		/* output sawtooth waveform    */

		if (i & (1<<(SIN_BITS-1)) )
			x = ((SIN_LEN-1)-i)*16 + 1; /* negative: from 8177 to 1 */
		else
			x = i*16;   /*positive: from 0 to 8176 */

		if (x > TL_TAB_LEN)
			x = TL_TAB_LEN; /* clip to the allowed range */

		sin_tab[7*SIN_LEN+i] = x;

		//logerror("YMF262.C: sin1[%4i]= %4i (tl_tab value=%5i)\n", i, sin_tab[1*SIN_LEN+i], tl_tab[sin_tab[1*SIN_LEN+i]] );
		//logerror("YMF262.C: sin2[%4i]= %4i (tl_tab value=%5i)\n", i, sin_tab[2*SIN_LEN+i], tl_tab[sin_tab[2*SIN_LEN+i]] );
		//logerror("YMF262.C: sin3[%4i]= %4i (tl_tab value=%5i)\n", i, sin_tab[3*SIN_LEN+i], tl_tab[sin_tab[3*SIN_LEN+i]] );
		//logerror("YMF262.C: sin4[%4i]= %4i (tl_tab value=%5i)\n", i, sin_tab[4*SIN_LEN+i], tl_tab[sin_tab[4*SIN_LEN+i]] );
		//logerror("YMF262.C: sin5[%4i]= %4i (tl_tab value=%5i)\n", i, sin_tab[5*SIN_LEN+i], tl_tab[sin_tab[5*SIN_LEN+i]] );
		//logerror("YMF262.C: sin6[%4i]= %4i (tl_tab value=%5i)\n", i, sin_tab[6*SIN_LEN+i], tl_tab[sin_tab[6*SIN_LEN+i]] );
		//logerror("YMF262.C: sin7[%4i]= %4i (tl_tab value=%5i)\n", i, sin_tab[7*SIN_LEN+i], tl_tab[sin_tab[7*SIN_LEN+i]] );
	}
	/*logerror("YMF262.C: ENV_QUIET= %08x (dec*8=%i)\n", ENV_QUIET, ENV_QUIET*8 );*/

#ifdef SAVE_SAMPLE
	sample[0]=fopen("sampsum.pcm","wb");
#endif

	return 1;
}

static void OPLCloseTable( void )
{
#ifdef SAVE_SAMPLE
	fclose(sample[0]);
#endif
}



static void OPL3_initalize(OPL3 *chip)
{
	int i;

	/* frequency base */
	chip->freqbase  = (chip->rate) ? ((double)chip->clock / (8.0*36)) / chip->rate  : 0;
#if 0
	chip->rate = (double)chip->clock / (8.0*36);
	chip->freqbase  = 1.0;
#endif

	/* logerror("YMF262: freqbase=%f\n", chip->freqbase); */

	/* Timer base time */
	chip->TimerBase = attotime::from_hz(chip->clock) * (8*36);

	/* make fnumber -> increment counter table */
	for( i=0 ; i < 1024 ; i++ )
	{
		/* opn phase increment counter = 20bit */
		chip->fn_tab[i] = (UINT32)( (double)i * 64 * chip->freqbase * (1<<(FREQ_SH-10)) ); /* -10 because chip works with 10.10 fixed point, while we use 16.16 */
#if 0
		logerror("YMF262.C: fn_tab[%4i] = %08x (dec=%8i)\n",
					i, chip->fn_tab[i]>>6, chip->fn_tab[i]>>6 );
#endif
	}

#if 0
	for( i=0 ; i < 16 ; i++ )
	{
		logerror("YMF262.C: sl_tab[%i] = %08x\n",
			i, sl_tab[i] );
	}
	for( i=0 ; i < 8 ; i++ )
	{
		int j;
		logerror("YMF262.C: ksl_tab[oct=%2i] =",i);
		for (j=0; j<16; j++)
		{
			logerror("%08x ", static_cast<UINT32>(ksl_tab[i*16+j]) );
		}
		logerror("\n");
	}
#endif


	/* Amplitude modulation: 27 output levels (triangle waveform); 1 level takes one of: 192, 256 or 448 samples */
	/* One entry from LFO_AM_TABLE lasts for 64 samples */
	chip->lfo_am_inc = (1.0 / 64.0 ) * (1<<LFO_SH) * chip->freqbase;

	/* Vibrato: 8 output levels (triangle waveform); 1 level takes 1024 samples */
	chip->lfo_pm_inc = (1.0 / 1024.0) * (1<<LFO_SH) * chip->freqbase;

	/*logerror ("chip->lfo_am_inc = %8x ; chip->lfo_pm_inc = %8x\n", chip->lfo_am_inc, chip->lfo_pm_inc);*/

	/* Noise generator: a step takes 1 sample */
	chip->noise_f = (1.0 / 1.0) * (1<<FREQ_SH) * chip->freqbase;

	chip->eg_timer_add  = (1<<EG_SH)  * chip->freqbase;
	chip->eg_timer_overflow = ( 1 ) * (1<<EG_SH);
	/*logerror("YMF262init eg_timer_add=%8x eg_timer_overflow=%8x\n", chip->eg_timer_add, chip->eg_timer_overflow);*/

}

static inline void FM_KEYON(OPL3_SLOT *SLOT, UINT32 key_set)
{
	if( !SLOT->key )
	{
		/* restart Phase Generator */
		SLOT->Cnt = 0;
		/* phase -> Attack */
		SLOT->state = EG_ATT;
	}
	SLOT->key |= key_set;
}

static inline void FM_KEYOFF(OPL3_SLOT *SLOT, UINT32 key_clr)
{
	if( SLOT->key )
	{
		SLOT->key &= key_clr;

		if( !SLOT->key )
		{
			/* phase -> Release */
			if (SLOT->state>EG_REL)
				SLOT->state = EG_REL;
		}
	}
}

/* update phase increment counter of operator (also update the EG rates if necessary) */
static inline void CALC_FCSLOT(OPL3_CH *CH,OPL3_SLOT *SLOT)
{
	int ksr;

	/* (frequency) phase increment counter */
	SLOT->Incr = CH->fc * SLOT->mul;
	ksr = CH->kcode >> SLOT->KSR;

	if( SLOT->ksr != ksr )
	{
		SLOT->ksr = ksr;

		/* calculate envelope generator rates */
		if ((SLOT->ar + SLOT->ksr) < 16+60)
		{
			SLOT->eg_sh_ar  = eg_rate_shift [SLOT->ar + SLOT->ksr ];
			SLOT->eg_m_ar   = (1<<SLOT->eg_sh_ar)-1;
			SLOT->eg_sel_ar = eg_rate_select[SLOT->ar + SLOT->ksr ];
		}
		else
		{
			SLOT->eg_sh_ar  = 0;
			SLOT->eg_m_ar   = (1<<SLOT->eg_sh_ar)-1;
			SLOT->eg_sel_ar = 13*RATE_STEPS;
		}
		SLOT->eg_sh_dr  = eg_rate_shift [SLOT->dr + SLOT->ksr ];
		SLOT->eg_m_dr   = (1<<SLOT->eg_sh_dr)-1;
		SLOT->eg_sel_dr = eg_rate_select[SLOT->dr + SLOT->ksr ];
		SLOT->eg_sh_rr  = eg_rate_shift [SLOT->rr + SLOT->ksr ];
		SLOT->eg_m_rr   = (1<<SLOT->eg_sh_rr)-1;
		SLOT->eg_sel_rr = eg_rate_select[SLOT->rr + SLOT->ksr ];
	}
}

/* set multi,am,vib,EG-TYP,KSR,mul */
static inline void set_mul(OPL3 *chip,int slot,int v)
{
	OPL3_CH   *CH   = &chip->P_CH[slot/2];
	OPL3_SLOT *SLOT = &CH->SLOT[slot&1];

	SLOT->mul     = mul_tab[v&0x0f];
	SLOT->KSR     = (v&0x10) ? 0 : 2;
	SLOT->eg_type = (v&0x20);
	SLOT->vib     = (v&0x40);
	SLOT->AMmask  = (v&0x80) ? ~0 : 0;

	if (chip->OPL3_mode & 1)
	{
		int chan_no = slot/2;

		/* in OPL3 mode */
		//DO THIS:
		//if this is one of the slots of 1st channel forming up a 4-op channel
		//do normal operation
		//else normal 2 operator function
		//OR THIS:
		//if this is one of the slots of 2nd channel forming up a 4-op channel
		//update it using channel data of 1st channel of a pair
		//else normal 2 operator function
		switch(chan_no)
		{
		case 0: case 1: case 2:
		case 9: case 10: case 11:
			if (CH->extended)
			{
				/* normal */
				CALC_FCSLOT(CH,SLOT);
			}
			else
			{
				/* normal */
				CALC_FCSLOT(CH,SLOT);
			}
		break;
		case 3: case 4: case 5:
		case 12: case 13: case 14:
			if ((CH-3)->extended)
			{
				/* update this SLOT using frequency data for 1st channel of a pair */
				CALC_FCSLOT(CH-3,SLOT);
			}
			else
			{
				/* normal */
				CALC_FCSLOT(CH,SLOT);
			}
		break;
		default:
				/* normal */
				CALC_FCSLOT(CH,SLOT);
		break;
		}
	}
	else
	{
		/* in OPL2 mode */
		CALC_FCSLOT(CH,SLOT);
	}
}

/* set ksl & tl */
static inline void set_ksl_tl(OPL3 *chip,int slot,int v)
{
	OPL3_CH   *CH   = &chip->P_CH[slot/2];
	OPL3_SLOT *SLOT = &CH->SLOT[slot&1];

	SLOT->ksl = ksl_shift[v >> 6];
	SLOT->TL  = (v&0x3f)<<(ENV_BITS-1-7); /* 7 bits TL (bit 6 = always 0) */

	if (chip->OPL3_mode & 1)
	{
		int chan_no = slot/2;

		/* in OPL3 mode */
		//DO THIS:
		//if this is one of the slots of 1st channel forming up a 4-op channel
		//do normal operation
		//else normal 2 operator function
		//OR THIS:
		//if this is one of the slots of 2nd channel forming up a 4-op channel
		//update it using channel data of 1st channel of a pair
		//else normal 2 operator function
		switch(chan_no)
		{
		case 0: case 1: case 2:
		case 9: case 10: case 11:
			if (CH->extended)
			{
				/* normal */
				SLOT->TLL = SLOT->TL + (CH->ksl_base>>SLOT->ksl);
			}
			else
			{
				/* normal */
				SLOT->TLL = SLOT->TL + (CH->ksl_base>>SLOT->ksl);
			}
		break;
		case 3: case 4: case 5:
		case 12: case 13: case 14:
			if ((CH-3)->extended)
			{
				/* update this SLOT using frequency data for 1st channel of a pair */
				SLOT->TLL = SLOT->TL + ((CH-3)->ksl_base>>SLOT->ksl);
			}
			else
			{
				/* normal */
				SLOT->TLL = SLOT->TL + (CH->ksl_base>>SLOT->ksl);
			}
		break;
		default:
				/* normal */
				SLOT->TLL = SLOT->TL + (CH->ksl_base>>SLOT->ksl);
		break;
		}
	}
	else
	{
		/* in OPL2 mode */
		SLOT->TLL = SLOT->TL + (CH->ksl_base>>SLOT->ksl);
	}

}

/* set attack rate & decay rate  */
static inline void set_ar_dr(OPL3 *chip,int slot,int v)
{
	OPL3_CH   *CH   = &chip->P_CH[slot/2];
	OPL3_SLOT *SLOT = &CH->SLOT[slot&1];

	SLOT->ar = (v>>4)  ? 16 + ((v>>4)  <<2) : 0;

	if ((SLOT->ar + SLOT->ksr) < 16+60) /* verified on real YMF262 - all 15 x rates take "zero" time */
	{
		SLOT->eg_sh_ar  = eg_rate_shift [SLOT->ar + SLOT->ksr ];
		SLOT->eg_m_ar   = (1<<SLOT->eg_sh_ar)-1;
		SLOT->eg_sel_ar = eg_rate_select[SLOT->ar + SLOT->ksr ];
	}
	else
	{
		SLOT->eg_sh_ar  = 0;
		SLOT->eg_m_ar   = (1<<SLOT->eg_sh_ar)-1;
		SLOT->eg_sel_ar = 13*RATE_STEPS;
	}

	SLOT->dr    = (v&0x0f)? 16 + ((v&0x0f)<<2) : 0;
	SLOT->eg_sh_dr  = eg_rate_shift [SLOT->dr + SLOT->ksr ];
	SLOT->eg_m_dr   = (1<<SLOT->eg_sh_dr)-1;
	SLOT->eg_sel_dr = eg_rate_select[SLOT->dr + SLOT->ksr ];
}

/* set sustain level & release rate */
static inline void set_sl_rr(OPL3 *chip,int slot,int v)
{
	OPL3_CH   *CH   = &chip->P_CH[slot/2];
	OPL3_SLOT *SLOT = &CH->SLOT[slot&1];

	SLOT->sl  = sl_tab[ v>>4 ];

	SLOT->rr  = (v&0x0f)? 16 + ((v&0x0f)<<2) : 0;
	SLOT->eg_sh_rr  = eg_rate_shift [SLOT->rr + SLOT->ksr ];
	SLOT->eg_m_rr   = (1<<SLOT->eg_sh_rr)-1;
	SLOT->eg_sel_rr = eg_rate_select[SLOT->rr + SLOT->ksr ];
}


static void update_channels(OPL3 *chip, OPL3_CH *CH)
{
	/* update channel passed as a parameter and a channel at CH+=3; */
	if (CH->extended)
	{   /* we've just switched to combined 4 operator mode */

	}
	else
	{   /* we've just switched to normal 2 operator mode */

	}

}

/* write a value v to register r on OPL chip */
static void OPL3WriteReg(OPL3 *chip, int r, int v)
{
	OPL3_CH *CH;
	signed int *chanout = chip->chanout;
	unsigned int ch_offset = 0;
	int slot;
	int block_fnum;



	if (LOG_CYM_FILE && (cymfile) && ((r&255)!=0) && (r!=255) )
	{
		if (r>0xff)
			fputc( (unsigned char)0xff, cymfile );/*mark writes to second register set*/

		fputc( (unsigned char)r&0xff, cymfile );
		fputc( (unsigned char)v, cymfile );
	}

	if(r&0x100)
	{
		switch(r)
		{
		case 0x101: /* test register */
			return;

		case 0x104: /* 6 channels enable */
			{
				UINT8 prev;

				CH = &chip->P_CH[0];    /* channel 0 */
				prev = CH->extended;
				CH->extended = (v>>0) & 1;
				if(prev != CH->extended)
					update_channels(chip, CH);
				CH++;                   /* channel 1 */
				prev = CH->extended;
				CH->extended = (v>>1) & 1;
				if(prev != CH->extended)
					update_channels(chip, CH);
				CH++;                   /* channel 2 */
				prev = CH->extended;
				CH->extended = (v>>2) & 1;
				if(prev != CH->extended)
					update_channels(chip, CH);


				CH = &chip->P_CH[9];    /* channel 9 */
				prev = CH->extended;
				CH->extended = (v>>3) & 1;
				if(prev != CH->extended)
					update_channels(chip, CH);
				CH++;                   /* channel 10 */
				prev = CH->extended;
				CH->extended = (v>>4) & 1;
				if(prev != CH->extended)
					update_channels(chip, CH);
				CH++;                   /* channel 11 */
				prev = CH->extended;
				CH->extended = (v>>5) & 1;
				if(prev != CH->extended)
					update_channels(chip, CH);

			}
			return;

		case 0x105: /* OPL3 extensions enable register */

			chip->OPL3_mode = v&0x01;   /* OPL3 mode when bit0=1 otherwise it is OPL2 mode */

			/* following behaviour was tested on real YMF262,
			switching OPL3/OPL2 modes on the fly:
			 - does not change the waveform previously selected (unless when ....)
			 - does not update CH.A, CH.B, CH.C and CH.D output selectors (registers c0-c8) (unless when ....)
			 - does not disable channels 9-17 on OPL3->OPL2 switch
			 - does not switch 4 operator channels back to 2 operator channels
			*/

			return;

		default:
			if (r < 0x120)
				chip->device->logerror("YMF262: write to unknown register (set#2): %03x value=%02x\n",r,v);
		break;
		}

		ch_offset = 9;  /* register page #2 starts from channel 9 (counting from 0) */
	}

	/* adjust bus to 8 bits */
	r &= 0xff;
	v &= 0xff;


	switch(r&0xe0)
	{
	case 0x00:  /* 00-1f:control */
		switch(r&0x1f)
		{
		case 0x01:  /* test register */
		break;
		case 0x02:  /* Timer 1 */
			chip->T[0] = (256-v)*4;
		break;
		case 0x03:  /* Timer 2 */
			chip->T[1] = (256-v)*16;
		break;
		case 0x04:  /* IRQ clear / mask and Timer enable */
			if(v&0x80)
			{   /* IRQ flags clear */
				OPL3_STATUS_RESET(chip,0x60);
			}
			else
			{   /* set IRQ mask ,timer enable */
				UINT8 st1 = v & 1;
				UINT8 st2 = (v>>1) & 1;

				/* IRQRST,T1MSK,t2MSK,x,x,x,ST2,ST1 */
				OPL3_STATUS_RESET(chip, v & 0x60);
				OPL3_STATUSMASK_SET(chip, (~v) & 0x60 );

				/* timer 2 */
				if(chip->st[1] != st2)
				{
					attotime period = st2 ? chip->TimerBase * chip->T[1] : attotime::zero;
					chip->st[1] = st2;
					if (chip->timer_handler) (chip->timer_handler)(chip->TimerParam,1,period);
				}
				/* timer 1 */
				if(chip->st[0] != st1)
				{
					attotime period = st1 ? chip->TimerBase * chip->T[0] : attotime::zero;
					chip->st[0] = st1;
					if (chip->timer_handler) (chip->timer_handler)(chip->TimerParam,0,period);
				}
			}
		break;
		case 0x08:  /* x,NTS,x,x, x,x,x,x */
			chip->nts = v;
		break;

		default:
			chip->device->logerror("YMF262: write to unknown register: %02x value=%02x\n",r,v);
		break;
		}
		break;
	case 0x20:  /* am ON, vib ON, ksr, eg_type, mul */
		slot = slot_array[r&0x1f];
		if(slot < 0) return;
		set_mul(chip, slot + ch_offset*2, v);
	break;
	case 0x40:
		slot = slot_array[r&0x1f];
		if(slot < 0) return;
		set_ksl_tl(chip, slot + ch_offset*2, v);
	break;
	case 0x60:
		slot = slot_array[r&0x1f];
		if(slot < 0) return;
		set_ar_dr(chip, slot + ch_offset*2, v);
	break;
	case 0x80:
		slot = slot_array[r&0x1f];
		if(slot < 0) return;
		set_sl_rr(chip, slot + ch_offset*2, v);
	break;
	case 0xa0:
		if (r == 0xbd)          /* am depth, vibrato depth, r,bd,sd,tom,tc,hh */
		{
			if (ch_offset != 0) /* 0xbd register is present in set #1 only */
				return;

			chip->lfo_am_depth = v & 0x80;
			chip->lfo_pm_depth_range = (v&0x40) ? 8 : 0;

			chip->rhythm = v&0x3f;

			if(chip->rhythm&0x20)
			{
				/* BD key on/off */
				if(v&0x10)
				{
					FM_KEYON (&chip->P_CH[6].SLOT[SLOT1], 2);
					FM_KEYON (&chip->P_CH[6].SLOT[SLOT2], 2);
				}
				else
				{
					FM_KEYOFF(&chip->P_CH[6].SLOT[SLOT1],~2);
					FM_KEYOFF(&chip->P_CH[6].SLOT[SLOT2],~2);
				}
				/* HH key on/off */
				if(v&0x01) FM_KEYON (&chip->P_CH[7].SLOT[SLOT1], 2);
				else       FM_KEYOFF(&chip->P_CH[7].SLOT[SLOT1],~2);
				/* SD key on/off */
				if(v&0x08) FM_KEYON (&chip->P_CH[7].SLOT[SLOT2], 2);
				else       FM_KEYOFF(&chip->P_CH[7].SLOT[SLOT2],~2);
				/* TOM key on/off */
				if(v&0x04) FM_KEYON (&chip->P_CH[8].SLOT[SLOT1], 2);
				else       FM_KEYOFF(&chip->P_CH[8].SLOT[SLOT1],~2);
				/* TOP-CY key on/off */
				if(v&0x02) FM_KEYON (&chip->P_CH[8].SLOT[SLOT2], 2);
				else       FM_KEYOFF(&chip->P_CH[8].SLOT[SLOT2],~2);
			}
			else
			{
				/* BD key off */
				FM_KEYOFF(&chip->P_CH[6].SLOT[SLOT1],~2);
				FM_KEYOFF(&chip->P_CH[6].SLOT[SLOT2],~2);
				/* HH key off */
				FM_KEYOFF(&chip->P_CH[7].SLOT[SLOT1],~2);
				/* SD key off */
				FM_KEYOFF(&chip->P_CH[7].SLOT[SLOT2],~2);
				/* TOM key off */
				FM_KEYOFF(&chip->P_CH[8].SLOT[SLOT1],~2);
				/* TOP-CY off */
				FM_KEYOFF(&chip->P_CH[8].SLOT[SLOT2],~2);
			}
			return;
		}

		/* keyon,block,fnum */
		if( (r&0x0f) > 8) return;
		CH = &chip->P_CH[(r&0x0f) + ch_offset];

		if(!(r&0x10))
		{   /* a0-a8 */
			block_fnum  = (CH->block_fnum&0x1f00) | v;
		}
		else
		{   /* b0-b8 */
			block_fnum = ((v&0x1f)<<8) | (CH->block_fnum&0xff);

			if (chip->OPL3_mode & 1)
			{
				int chan_no = (r&0x0f) + ch_offset;

				/* in OPL3 mode */
				//DO THIS:
				//if this is 1st channel forming up a 4-op channel
				//ALSO keyon/off slots of 2nd channel forming up 4-op channel
				//else normal 2 operator function keyon/off
				//OR THIS:
				//if this is 2nd channel forming up 4-op channel just do nothing
				//else normal 2 operator function keyon/off
				switch(chan_no)
				{
				case 0: case 1: case 2:
				case 9: case 10: case 11:
					if (CH->extended)
					{
						//if this is 1st channel forming up a 4-op channel
						//ALSO keyon/off slots of 2nd channel forming up 4-op channel
						if(v&0x20)
						{
							FM_KEYON (&CH->SLOT[SLOT1], 1);
							FM_KEYON (&CH->SLOT[SLOT2], 1);
							FM_KEYON (&(CH+3)->SLOT[SLOT1], 1);
							FM_KEYON (&(CH+3)->SLOT[SLOT2], 1);
						}
						else
						{
							FM_KEYOFF(&CH->SLOT[SLOT1],~1);
							FM_KEYOFF(&CH->SLOT[SLOT2],~1);
							FM_KEYOFF(&(CH+3)->SLOT[SLOT1],~1);
							FM_KEYOFF(&(CH+3)->SLOT[SLOT2],~1);
						}
					}
					else
					{
						//else normal 2 operator function keyon/off
						if(v&0x20)
						{
							FM_KEYON (&CH->SLOT[SLOT1], 1);
							FM_KEYON (&CH->SLOT[SLOT2], 1);
						}
						else
						{
							FM_KEYOFF(&CH->SLOT[SLOT1],~1);
							FM_KEYOFF(&CH->SLOT[SLOT2],~1);
						}
					}
				break;

				case 3: case 4: case 5:
				case 12: case 13: case 14:
					if ((CH-3)->extended)
					{
						//if this is 2nd channel forming up 4-op channel just do nothing
					}
					else
					{
						//else normal 2 operator function keyon/off
						if(v&0x20)
						{
							FM_KEYON (&CH->SLOT[SLOT1], 1);
							FM_KEYON (&CH->SLOT[SLOT2], 1);
						}
						else
						{
							FM_KEYOFF(&CH->SLOT[SLOT1],~1);
							FM_KEYOFF(&CH->SLOT[SLOT2],~1);
						}
					}
				break;

				default:
					if(v&0x20)
					{
						FM_KEYON (&CH->SLOT[SLOT1], 1);
						FM_KEYON (&CH->SLOT[SLOT2], 1);
					}
					else
					{
						FM_KEYOFF(&CH->SLOT[SLOT1],~1);
						FM_KEYOFF(&CH->SLOT[SLOT2],~1);
					}
				break;
				}
			}
			else
			{
				if(v&0x20)
				{
					FM_KEYON (&CH->SLOT[SLOT1], 1);
					FM_KEYON (&CH->SLOT[SLOT2], 1);
				}
				else
				{
					FM_KEYOFF(&CH->SLOT[SLOT1],~1);
					FM_KEYOFF(&CH->SLOT[SLOT2],~1);
				}
			}
		}
		/* update */
		if(CH->block_fnum != block_fnum)
		{
			UINT8 block  = block_fnum >> 10;

			CH->block_fnum = block_fnum;

			CH->ksl_base = static_cast<UINT32>(ksl_tab[block_fnum>>6]);
			CH->fc       = chip->fn_tab[block_fnum&0x03ff] >> (7-block);

			/* BLK 2,1,0 bits -> bits 3,2,1 of kcode */
			CH->kcode    = (CH->block_fnum&0x1c00)>>9;

			/* the info below is actually opposite to what is stated in the Manuals (verifed on real YMF262) */
			/* if notesel == 0 -> lsb of kcode is bit 10 (MSB) of fnum  */
			/* if notesel == 1 -> lsb of kcode is bit 9 (MSB-1) of fnum */
			if (chip->nts&0x40)
				CH->kcode |= (CH->block_fnum&0x100)>>8; /* notesel == 1 */
			else
				CH->kcode |= (CH->block_fnum&0x200)>>9; /* notesel == 0 */

			if (chip->OPL3_mode & 1)
			{
				int chan_no = (r&0x0f) + ch_offset;
				/* in OPL3 mode */
				//DO THIS:
				//if this is 1st channel forming up a 4-op channel
				//ALSO update slots of 2nd channel forming up 4-op channel
				//else normal 2 operator function keyon/off
				//OR THIS:
				//if this is 2nd channel forming up 4-op channel just do nothing
				//else normal 2 operator function keyon/off
				switch(chan_no)
				{
				case 0: case 1: case 2:
				case 9: case 10: case 11:
					if (CH->extended)
					{
						//if this is 1st channel forming up a 4-op channel
						//ALSO update slots of 2nd channel forming up 4-op channel

						/* refresh Total Level in FOUR SLOTs of this channel and channel+3 using data from THIS channel */
						CH->SLOT[SLOT1].TLL = CH->SLOT[SLOT1].TL + (CH->ksl_base>>CH->SLOT[SLOT1].ksl);
						CH->SLOT[SLOT2].TLL = CH->SLOT[SLOT2].TL + (CH->ksl_base>>CH->SLOT[SLOT2].ksl);
						(CH+3)->SLOT[SLOT1].TLL = (CH+3)->SLOT[SLOT1].TL + (CH->ksl_base>>(CH+3)->SLOT[SLOT1].ksl);
						(CH+3)->SLOT[SLOT2].TLL = (CH+3)->SLOT[SLOT2].TL + (CH->ksl_base>>(CH+3)->SLOT[SLOT2].ksl);

						/* refresh frequency counter in FOUR SLOTs of this channel and channel+3 using data from THIS channel */
						CALC_FCSLOT(CH,&CH->SLOT[SLOT1]);
						CALC_FCSLOT(CH,&CH->SLOT[SLOT2]);
						CALC_FCSLOT(CH,&(CH+3)->SLOT[SLOT1]);
						CALC_FCSLOT(CH,&(CH+3)->SLOT[SLOT2]);
					}
					else
					{
						//else normal 2 operator function
						/* refresh Total Level in both SLOTs of this channel */
						CH->SLOT[SLOT1].TLL = CH->SLOT[SLOT1].TL + (CH->ksl_base>>CH->SLOT[SLOT1].ksl);
						CH->SLOT[SLOT2].TLL = CH->SLOT[SLOT2].TL + (CH->ksl_base>>CH->SLOT[SLOT2].ksl);

						/* refresh frequency counter in both SLOTs of this channel */
						CALC_FCSLOT(CH,&CH->SLOT[SLOT1]);
						CALC_FCSLOT(CH,&CH->SLOT[SLOT2]);
					}
				break;

				case 3: case 4: case 5:
				case 12: case 13: case 14:
					if ((CH-3)->extended)
					{
						//if this is 2nd channel forming up 4-op channel just do nothing
					}
					else
					{
						//else normal 2 operator function
						/* refresh Total Level in both SLOTs of this channel */
						CH->SLOT[SLOT1].TLL = CH->SLOT[SLOT1].TL + (CH->ksl_base>>CH->SLOT[SLOT1].ksl);
						CH->SLOT[SLOT2].TLL = CH->SLOT[SLOT2].TL + (CH->ksl_base>>CH->SLOT[SLOT2].ksl);

						/* refresh frequency counter in both SLOTs of this channel */
						CALC_FCSLOT(CH,&CH->SLOT[SLOT1]);
						CALC_FCSLOT(CH,&CH->SLOT[SLOT2]);
					}
				break;

				default:
					/* refresh Total Level in both SLOTs of this channel */
					CH->SLOT[SLOT1].TLL = CH->SLOT[SLOT1].TL + (CH->ksl_base>>CH->SLOT[SLOT1].ksl);
					CH->SLOT[SLOT2].TLL = CH->SLOT[SLOT2].TL + (CH->ksl_base>>CH->SLOT[SLOT2].ksl);

					/* refresh frequency counter in both SLOTs of this channel */
					CALC_FCSLOT(CH,&CH->SLOT[SLOT1]);
					CALC_FCSLOT(CH,&CH->SLOT[SLOT2]);
				break;
				}
			}
			else
			{
				/* in OPL2 mode */

				/* refresh Total Level in both SLOTs of this channel */
				CH->SLOT[SLOT1].TLL = CH->SLOT[SLOT1].TL + (CH->ksl_base>>CH->SLOT[SLOT1].ksl);
				CH->SLOT[SLOT2].TLL = CH->SLOT[SLOT2].TL + (CH->ksl_base>>CH->SLOT[SLOT2].ksl);

				/* refresh frequency counter in both SLOTs of this channel */
				CALC_FCSLOT(CH,&CH->SLOT[SLOT1]);
				CALC_FCSLOT(CH,&CH->SLOT[SLOT2]);
			}
		}
	break;

	case 0xc0:
		/* CH.D, CH.C, CH.B, CH.A, FB(3bits), C */
		if( (r&0xf) > 8) return;

		CH = &chip->P_CH[(r&0xf) + ch_offset];

		if( chip->OPL3_mode & 1 )
		{
			int base = ((r&0xf) + ch_offset) * 4;

			/* OPL3 mode */
			chip->pan[ base    ] = (v & 0x10) ? ~0 : 0; /* ch.A */
			chip->pan[ base +1 ] = (v & 0x20) ? ~0 : 0; /* ch.B */
			chip->pan[ base +2 ] = (v & 0x40) ? ~0 : 0; /* ch.C */
			chip->pan[ base +3 ] = (v & 0x80) ? ~0 : 0; /* ch.D */
		}
		else
		{
			int base = ((r&0xf) + ch_offset) * 4;

			/* OPL2 mode - always enabled */
			chip->pan[ base    ] = ~0;      /* ch.A */
			chip->pan[ base +1 ] = ~0;      /* ch.B */
			chip->pan[ base +2 ] = ~0;      /* ch.C */
			chip->pan[ base +3 ] = ~0;      /* ch.D */
		}

		chip->pan_ctrl_value[ (r&0xf) + ch_offset ] = v;    /* store control value for OPL3/OPL2 mode switching on the fly */

		CH->SLOT[SLOT1].FB  = (v>>1)&7 ? ((v>>1)&7) + 7 : 0;
		CH->SLOT[SLOT1].CON = v&1;

		if( chip->OPL3_mode & 1 )
		{
			int chan_no = (r&0x0f) + ch_offset;

			switch(chan_no)
			{
			case 0: case 1: case 2:
			case 9: case 10: case 11:
				if (CH->extended)
				{
					UINT8 conn = (CH->SLOT[SLOT1].CON<<1) | ((CH+3)->SLOT[SLOT1].CON<<0);
					switch(conn)
					{
					case 0:
						/* 1 -> 2 -> 3 -> 4 - out */

						CH->SLOT[SLOT1].connect = &chip->phase_modulation;
						CH->SLOT[SLOT2].connect = &chip->phase_modulation2;
						(CH+3)->SLOT[SLOT1].connect = &chip->phase_modulation;
						(CH+3)->SLOT[SLOT2].connect = &chanout[ chan_no + 3 ];
					break;
					case 1:
						/* 1 -> 2 -\
						   3 -> 4 -+- out */

						CH->SLOT[SLOT1].connect = &chip->phase_modulation;
						CH->SLOT[SLOT2].connect = &chanout[ chan_no ];
						(CH+3)->SLOT[SLOT1].connect = &chip->phase_modulation;
						(CH+3)->SLOT[SLOT2].connect = &chanout[ chan_no + 3 ];
					break;
					case 2:
						/* 1 -----------\
						   2 -> 3 -> 4 -+- out */

						CH->SLOT[SLOT1].connect = &chanout[ chan_no ];
						CH->SLOT[SLOT2].connect = &chip->phase_modulation2;
						(CH+3)->SLOT[SLOT1].connect = &chip->phase_modulation;
						(CH+3)->SLOT[SLOT2].connect = &chanout[ chan_no + 3 ];
					break;
					case 3:
						/* 1 ------\
						   2 -> 3 -+- out
						   4 ------/     */
						CH->SLOT[SLOT1].connect = &chanout[ chan_no ];
						CH->SLOT[SLOT2].connect = &chip->phase_modulation2;
						(CH+3)->SLOT[SLOT1].connect = &chanout[ chan_no + 3 ];
						(CH+3)->SLOT[SLOT2].connect = &chanout[ chan_no + 3 ];
					break;
					}
				}
				else
				{
					/* 2 operators mode */
					CH->SLOT[SLOT1].connect = CH->SLOT[SLOT1].CON ? &chanout[(r&0xf)+ch_offset] : &chip->phase_modulation;
					CH->SLOT[SLOT2].connect = &chanout[(r&0xf)+ch_offset];
				}
			break;

			case 3: case 4: case 5:
			case 12: case 13: case 14:
				if ((CH-3)->extended)
				{
					UINT8 conn = ((CH-3)->SLOT[SLOT1].CON<<1) | (CH->SLOT[SLOT1].CON<<0);
					switch(conn)
					{
					case 0:
						/* 1 -> 2 -> 3 -> 4 - out */

						(CH-3)->SLOT[SLOT1].connect = &chip->phase_modulation;
						(CH-3)->SLOT[SLOT2].connect = &chip->phase_modulation2;
						CH->SLOT[SLOT1].connect = &chip->phase_modulation;
						CH->SLOT[SLOT2].connect = &chanout[ chan_no ];
					break;
					case 1:
						/* 1 -> 2 -\
						   3 -> 4 -+- out */

						(CH-3)->SLOT[SLOT1].connect = &chip->phase_modulation;
						(CH-3)->SLOT[SLOT2].connect = &chanout[ chan_no - 3 ];
						CH->SLOT[SLOT1].connect = &chip->phase_modulation;
						CH->SLOT[SLOT2].connect = &chanout[ chan_no ];
					break;
					case 2:
						/* 1 -----------\
						   2 -> 3 -> 4 -+- out */

						(CH-3)->SLOT[SLOT1].connect = &chanout[ chan_no - 3 ];
						(CH-3)->SLOT[SLOT2].connect = &chip->phase_modulation2;
						CH->SLOT[SLOT1].connect = &chip->phase_modulation;
						CH->SLOT[SLOT2].connect = &chanout[ chan_no ];
					break;
					case 3:
						/* 1 ------\
						   2 -> 3 -+- out
						   4 ------/     */
						(CH-3)->SLOT[SLOT1].connect = &chanout[ chan_no - 3 ];
						(CH-3)->SLOT[SLOT2].connect = &chip->phase_modulation2;
						CH->SLOT[SLOT1].connect = &chanout[ chan_no ];
						CH->SLOT[SLOT2].connect = &chanout[ chan_no ];
					break;
					}
				}
				else
				{
					/* 2 operators mode */
					CH->SLOT[SLOT1].connect = CH->SLOT[SLOT1].CON ? &chanout[(r&0xf)+ch_offset] : &chip->phase_modulation;
					CH->SLOT[SLOT2].connect = &chanout[(r&0xf)+ch_offset];
				}
			break;

			default:
					/* 2 operators mode */
					CH->SLOT[SLOT1].connect = CH->SLOT[SLOT1].CON ? &chanout[(r&0xf)+ch_offset] : &chip->phase_modulation;
					CH->SLOT[SLOT2].connect = &chanout[(r&0xf)+ch_offset];
			break;
			}
		}
		else
		{
			/* OPL2 mode - always 2 operators mode */
			CH->SLOT[SLOT1].connect = CH->SLOT[SLOT1].CON ? &chanout[(r&0xf)+ch_offset] : &chip->phase_modulation;
			CH->SLOT[SLOT2].connect = &chanout[(r&0xf)+ch_offset];
		}
	break;

	case 0xe0: /* waveform select */
		slot = slot_array[r&0x1f];
		if(slot < 0) return;

		slot += ch_offset*2;

		CH = &chip->P_CH[slot/2];


		/* store 3-bit value written regardless of current OPL2 or OPL3 mode... (verified on real YMF262) */
		v &= 7;
		CH->SLOT[slot&1].waveform_number = v;

		/* ... but select only waveforms 0-3 in OPL2 mode */
		if( !(chip->OPL3_mode & 1) )
		{
			v &= 3; /* we're in OPL2 mode */
		}
		CH->SLOT[slot&1].wavetable = v * SIN_LEN;
	break;
	}
}

static TIMER_CALLBACK( cymfile_callback )
{
	if (cymfile)
	{
		fputc( (unsigned char)0, cymfile );
	}
}

/* lock/unlock for common table */
static int OPL3_LockTable(device_t *device)
{
	num_lock++;
	if(num_lock>1) return 0;

	/* first time */

	if( !init_tables() )
	{
		num_lock--;
		return -1;
	}

	if (LOG_CYM_FILE)
	{
		cymfile = fopen("ymf262_.cym","wb");
		if (cymfile)
			device->machine().scheduler().timer_pulse ( attotime::from_hz(110), FUNC(cymfile_callback)); /*110 Hz pulse timer*/
		else
			device->logerror("Could not create ymf262_.cym file\n");
	}

	return 0;
}

static void OPL3_UnLockTable(void)
{
	if(num_lock) num_lock--;
	if(num_lock) return;

	/* last time */
	OPLCloseTable();

	if (LOG_CYM_FILE)
		fclose (cymfile);
	cymfile = nullptr;
}

static void OPL3ResetChip(OPL3 *chip)
{
	int c,s;

	chip->eg_timer = 0;
	chip->eg_cnt   = 0;

	chip->noise_rng = 1;    /* noise shift register */
	chip->nts       = 0;    /* note split */
	OPL3_STATUS_RESET(chip,0x60);

	/* reset with register write */
	OPL3WriteReg(chip,0x01,0); /* test register */
	OPL3WriteReg(chip,0x02,0); /* Timer1 */
	OPL3WriteReg(chip,0x03,0); /* Timer2 */
	OPL3WriteReg(chip,0x04,0); /* IRQ mask clear */


//FIX IT  registers 101, 104 and 105


//FIX IT (dont change CH.D, CH.C, CH.B and CH.A in C0-C8 registers)
	for(c = 0xff ; c >= 0x20 ; c-- )
		OPL3WriteReg(chip,c,0);
//FIX IT (dont change CH.D, CH.C, CH.B and CH.A in C0-C8 registers)
	for(c = 0x1ff ; c >= 0x120 ; c-- )
		OPL3WriteReg(chip,c,0);



	/* reset operator parameters */
	for( c = 0 ; c < 9*2 ; c++ )
	{
		OPL3_CH *CH = &chip->P_CH[c];
		for(s = 0 ; s < 2 ; s++ )
		{
			CH->SLOT[s].state     = EG_OFF;
			CH->SLOT[s].volume    = MAX_ATT_INDEX;
		}
	}
}

/* Create one of virtual YMF262 */
/* 'clock' is chip clock in Hz  */
/* 'rate'  is sampling rate  */
static OPL3 *OPL3Create(device_t *device, int clock, int rate, int type)
{
	OPL3 *chip;

	if (OPL3_LockTable(device) == -1) return nullptr;

	/* allocate memory block */
	chip = auto_alloc_clear(device->machine(), OPL3);

	chip->device = device;
	chip->type  = type;
	chip->clock = clock;
	chip->rate  = rate;

	/* init global tables */
	OPL3_initalize(chip);

	/* reset chip */
	OPL3ResetChip(chip);
	return chip;
}

/* Destroy one of virtual YMF262 */
static void OPL3Destroy(OPL3 *chip)
{
	OPL3_UnLockTable();
	auto_free(chip->device->machine(), chip);
}


/* Optional handlers */

static void OPL3SetTimerHandler(OPL3 *chip,OPL3_TIMERHANDLER timer_handler,void *param)
{
	chip->timer_handler   = timer_handler;
	chip->TimerParam = param;
}
static void OPL3SetIRQHandler(OPL3 *chip,OPL3_IRQHANDLER IRQHandler,void *param)
{
	chip->IRQHandler     = IRQHandler;
	chip->IRQParam = param;
}
static void OPL3SetUpdateHandler(OPL3 *chip,OPL3_UPDATEHANDLER UpdateHandler,void *param)
{
	chip->UpdateHandler = UpdateHandler;
	chip->UpdateParam = param;
}

/* YMF262 I/O interface */
static int OPL3Write(OPL3 *chip, int a, int v)
{
	/* data bus is 8 bits */
	v &= 0xff;

	switch(a&3)
	{
	case 0: /* address port 0 (register set #1) */
		chip->address = v;
	break;

	case 1: /* data port - ignore A1 */
	case 3: /* data port - ignore A1 */
		if(chip->UpdateHandler) chip->UpdateHandler(chip->UpdateParam,0);
		OPL3WriteReg(chip,chip->address,v);
	break;

	case 2: /* address port 1 (register set #2) */

		/* verified on real YMF262:
		 in OPL3 mode:
		   address line A1 is stored during *address* write and ignored during *data* write.

		 in OPL2 mode:
		   register set#2 writes go to register set#1 (ignoring A1)
		   verified on registers from set#2: 0x01, 0x04, 0x20-0xef
		   The only exception is register 0x05.
		*/
		if( chip->OPL3_mode & 1 )
		{
			/* OPL3 mode */
				chip->address = v | 0x100;
		}
		else
		{
			/* in OPL2 mode the only accessible in set #2 is register 0x05 */
			if( v==5 )
				chip->address = v | 0x100;
			else
				chip->address = v;  /* verified range: 0x01, 0x04, 0x20-0xef(set #2 becomes set #1 in opl2 mode) */
		}
	break;
	}

	return chip->status>>7;
}

static unsigned char OPL3Read(OPL3 *chip,int a)
{
	if( a==0 )
	{
		/* status port */
		return chip->status;
	}

	return 0x00;    /* verified on real YMF262 */
}



static int OPL3TimerOver(OPL3 *chip,int c)
{
	if( c )
	{   /* Timer B */
		OPL3_STATUS_SET(chip,0x20);
	}
	else
	{   /* Timer A */
		OPL3_STATUS_SET(chip,0x40);
	}
	/* reload timer */
	if (chip->timer_handler) (chip->timer_handler)(chip->TimerParam,c,chip->TimerBase * chip->T[c]);
	return chip->status>>7;
}




void * ymf262_init(device_t *device, int clock, int rate)
{
	return OPL3Create(device,clock,rate,OPL3_TYPE_YMF262);
}

void ymf262_shutdown(void *chip)
{
	OPL3Destroy((OPL3 *)chip);
}
void ymf262_reset_chip(void *chip)
{
	OPL3ResetChip((OPL3 *)chip);
}

int ymf262_write(void *chip, int a, int v)
{
	return OPL3Write((OPL3 *)chip, a, v);
}

unsigned char ymf262_read(void *chip, int a)
{
	/* Note on status register: */

	/* YM3526(OPL) and YM3812(OPL2) return bit2 and bit1 in HIGH state */

	/* YMF262(OPL3) always returns bit2 and bit1 in LOW state */
	/* which can be used to identify the chip */

	/* YMF278(OPL4) returns bit2 in LOW and bit1 in HIGH state ??? info from manual - not verified */

	return OPL3Read((OPL3 *)chip, a);
}
int ymf262_timer_over(void *chip, int c)
{
	return OPL3TimerOver((OPL3 *)chip, c);
}

void ymf262_set_timer_handler(void *chip, OPL3_TIMERHANDLER timer_handler, void *param)
{
	OPL3SetTimerHandler((OPL3 *)chip, timer_handler, param);
}
void ymf262_set_irq_handler(void *chip,OPL3_IRQHANDLER IRQHandler,void *param)
{
	OPL3SetIRQHandler((OPL3 *)chip, IRQHandler, param);
}
void ymf262_set_update_handler(void *chip,OPL3_UPDATEHANDLER UpdateHandler,void *param)
{
	OPL3SetUpdateHandler((OPL3 *)chip, UpdateHandler, param);
}


/*
** Generate samples for one of the YMF262's
**
** 'which' is the virtual YMF262 number
** '**buffers' is table of 4 pointers to the buffers: CH.A, CH.B, CH.C and CH.D
** 'length' is the number of samples that should be generated
*/
void ymf262_update_one(void *_chip, OPL3SAMPLE **buffers, int length)
{
	int i;
	OPL3        *chip  = (OPL3 *)_chip;
	signed int *chanout = chip->chanout;
	UINT8       rhythm = chip->rhythm&0x20;

	OPL3SAMPLE  *ch_a = buffers[0];
	OPL3SAMPLE  *ch_b = buffers[1];
	OPL3SAMPLE  *ch_c = buffers[2];
	OPL3SAMPLE  *ch_d = buffers[3];

	for( i=0; i < length ; i++ )
	{
		int a,b,c,d;


		advance_lfo(chip);

		/* clear channel outputs */
		memset(chip->chanout, 0, sizeof(chip->chanout));

#if 1
	/* register set #1 */
		chan_calc(chip, &chip->P_CH[0]);            /* extended 4op ch#0 part 1 or 2op ch#0 */
		if (chip->P_CH[0].extended)
			chan_calc_ext(chip, &chip->P_CH[3]);    /* extended 4op ch#0 part 2 */
		else
			chan_calc(chip, &chip->P_CH[3]);        /* standard 2op ch#3 */


		chan_calc(chip, &chip->P_CH[1]);            /* extended 4op ch#1 part 1 or 2op ch#1 */
		if (chip->P_CH[1].extended)
			chan_calc_ext(chip, &chip->P_CH[4]);    /* extended 4op ch#1 part 2 */
		else
			chan_calc(chip, &chip->P_CH[4]);        /* standard 2op ch#4 */


		chan_calc(chip, &chip->P_CH[2]);            /* extended 4op ch#2 part 1 or 2op ch#2 */
		if (chip->P_CH[2].extended)
			chan_calc_ext(chip, &chip->P_CH[5]);    /* extended 4op ch#2 part 2 */
		else
			chan_calc(chip, &chip->P_CH[5]);        /* standard 2op ch#5 */


		if(!rhythm)
		{
			chan_calc(chip, &chip->P_CH[6]);
			chan_calc(chip, &chip->P_CH[7]);
			chan_calc(chip, &chip->P_CH[8]);
		}
		else        /* Rhythm part */
		{
			chan_calc_rhythm(chip, &chip->P_CH[0], (chip->noise_rng>>0)&1 );
		}

	/* register set #2 */
		chan_calc(chip, &chip->P_CH[ 9]);
		if (chip->P_CH[9].extended)
			chan_calc_ext(chip, &chip->P_CH[12]);
		else
			chan_calc(chip, &chip->P_CH[12]);


		chan_calc(chip, &chip->P_CH[10]);
		if (chip->P_CH[10].extended)
			chan_calc_ext(chip, &chip->P_CH[13]);
		else
			chan_calc(chip, &chip->P_CH[13]);


		chan_calc(chip, &chip->P_CH[11]);
		if (chip->P_CH[11].extended)
			chan_calc_ext(chip, &chip->P_CH[14]);
		else
			chan_calc(chip, &chip->P_CH[14]);


		/* channels 15,16,17 are fixed 2-operator channels only */
		chan_calc(chip, &chip->P_CH[15]);
		chan_calc(chip, &chip->P_CH[16]);
		chan_calc(chip, &chip->P_CH[17]);
#endif

		/* accumulator register set #1 */
		a =  chanout[0] & chip->pan[0];
		b =  chanout[0] & chip->pan[1];
		c =  chanout[0] & chip->pan[2];
		d =  chanout[0] & chip->pan[3];
#if 1
		a += chanout[1] & chip->pan[4];
		b += chanout[1] & chip->pan[5];
		c += chanout[1] & chip->pan[6];
		d += chanout[1] & chip->pan[7];
		a += chanout[2] & chip->pan[8];
		b += chanout[2] & chip->pan[9];
		c += chanout[2] & chip->pan[10];
		d += chanout[2] & chip->pan[11];

		a += chanout[3] & chip->pan[12];
		b += chanout[3] & chip->pan[13];
		c += chanout[3] & chip->pan[14];
		d += chanout[3] & chip->pan[15];
		a += chanout[4] & chip->pan[16];
		b += chanout[4] & chip->pan[17];
		c += chanout[4] & chip->pan[18];
		d += chanout[4] & chip->pan[19];
		a += chanout[5] & chip->pan[20];
		b += chanout[5] & chip->pan[21];
		c += chanout[5] & chip->pan[22];
		d += chanout[5] & chip->pan[23];

		a += chanout[6] & chip->pan[24];
		b += chanout[6] & chip->pan[25];
		c += chanout[6] & chip->pan[26];
		d += chanout[6] & chip->pan[27];
		a += chanout[7] & chip->pan[28];
		b += chanout[7] & chip->pan[29];
		c += chanout[7] & chip->pan[30];
		d += chanout[7] & chip->pan[31];
		a += chanout[8] & chip->pan[32];
		b += chanout[8] & chip->pan[33];
		c += chanout[8] & chip->pan[34];
		d += chanout[8] & chip->pan[35];

		/* accumulator register set #2 */
		a += chanout[9] & chip->pan[36];
		b += chanout[9] & chip->pan[37];
		c += chanout[9] & chip->pan[38];
		d += chanout[9] & chip->pan[39];
		a += chanout[10] & chip->pan[40];
		b += chanout[10] & chip->pan[41];
		c += chanout[10] & chip->pan[42];
		d += chanout[10] & chip->pan[43];
		a += chanout[11] & chip->pan[44];
		b += chanout[11] & chip->pan[45];
		c += chanout[11] & chip->pan[46];
		d += chanout[11] & chip->pan[47];

		a += chanout[12] & chip->pan[48];
		b += chanout[12] & chip->pan[49];
		c += chanout[12] & chip->pan[50];
		d += chanout[12] & chip->pan[51];
		a += chanout[13] & chip->pan[52];
		b += chanout[13] & chip->pan[53];
		c += chanout[13] & chip->pan[54];
		d += chanout[13] & chip->pan[55];
		a += chanout[14] & chip->pan[56];
		b += chanout[14] & chip->pan[57];
		c += chanout[14] & chip->pan[58];
		d += chanout[14] & chip->pan[59];

		a += chanout[15] & chip->pan[60];
		b += chanout[15] & chip->pan[61];
		c += chanout[15] & chip->pan[62];
		d += chanout[15] & chip->pan[63];
		a += chanout[16] & chip->pan[64];
		b += chanout[16] & chip->pan[65];
		c += chanout[16] & chip->pan[66];
		d += chanout[16] & chip->pan[67];
		a += chanout[17] & chip->pan[68];
		b += chanout[17] & chip->pan[69];
		c += chanout[17] & chip->pan[70];
		d += chanout[17] & chip->pan[71];
#endif
		a >>= FINAL_SH;
		b >>= FINAL_SH;
		c >>= FINAL_SH;
		d >>= FINAL_SH;

		/* limit check */
		a = limit( a , MAXOUT, MINOUT );
		b = limit( b , MAXOUT, MINOUT );
		c = limit( c , MAXOUT, MINOUT );
		d = limit( d , MAXOUT, MINOUT );

		#ifdef SAVE_SAMPLE
		if (which==0)
		{
			SAVE_ALL_CHANNELS
		}
		#endif

		/* store to sound buffer */
		ch_a[i] = a;
		ch_b[i] = b;
		ch_c[i] = c;
		ch_d[i] = d;

		advance(chip);
	}

}
