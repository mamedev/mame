// license:GPL-2.0+
// copyright-holders:Jarek Burczynski,Ernesto Corvi
#pragma once

#ifndef __YM2413_H__
#define __YM2413_H__

#include "emu.h"

class ym2413_device : public device_t,
						public device_sound_interface
{
public:
	ym2413_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE8_MEMBER( register_port_w );
	DECLARE_WRITE8_MEMBER( data_port_w );

	void _ym2413_update_request();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	struct OPLL_SLOT
	{
		UINT32  ar;         /* attack rate: AR<<2           */
		UINT32  dr;         /* decay rate:  DR<<2           */
		UINT32  rr;         /* release rate:RR<<2           */
		UINT8   KSR;        /* key scale rate               */
		UINT8   ksl;        /* keyscale level               */
		UINT8   ksr;        /* key scale rate: kcode>>KSR   */
		UINT8   mul;        /* multiple: mul_tab[ML]        */

		/* Phase Generator */
		UINT32  phase;      /* frequency counter            */
		UINT32  freq;       /* frequency counter step       */
		UINT8   fb_shift;   /* feedback shift value         */
		INT32   op1_out[2]; /* slot1 output for feedback    */

		/* Envelope Generator */
		UINT8   eg_type;    /* percussive/nonpercussive mode*/
		UINT8   state;      /* phase type                   */
		UINT32  TL;         /* total level: TL << 2         */
		INT32   TLL;        /* adjusted now TL              */
		INT32   volume;     /* envelope counter             */
		UINT32  sl;         /* sustain level: sl_tab[SL]    */

		UINT8   eg_sh_dp;   /* (dump state)                 */
		UINT8   eg_sel_dp;  /* (dump state)                 */
		UINT8   eg_sh_ar;   /* (attack state)               */
		UINT8   eg_sel_ar;  /* (attack state)               */
		UINT8   eg_sh_dr;   /* (decay state)                */
		UINT8   eg_sel_dr;  /* (decay state)                */
		UINT8   eg_sh_rr;   /* (release state for non-perc.)*/
		UINT8   eg_sel_rr;  /* (release state for non-perc.)*/
		UINT8   eg_sh_rs;   /* (release state for perc.mode)*/
		UINT8   eg_sel_rs;  /* (release state for perc.mode)*/

		UINT32  key;        /* 0 = KEY OFF, >0 = KEY ON     */

		/* LFO */
		UINT32  AMmask;     /* LFO Amplitude Modulation enable mask */
		UINT8   vib;        /* LFO Phase Modulation enable flag (active high)*/

		/* waveform select */
		unsigned int wavetable;
	};

	struct OPLL_CH
	{
		OPLL_SLOT SLOT[2];
		/* phase generator state */
		UINT32  block_fnum; /* block+fnum                   */
		UINT32  fc;         /* Freq. freqement base         */
		UINT32  ksl_base;   /* KeyScaleLevel Base step      */
		UINT8   kcode;      /* key code (for key scaling)   */
		UINT8   sus;        /* sus on/off (release speed in percussive mode)*/
	};

	enum {
		RATE_STEPS = (8),

		/* sinwave entries */
		SIN_BITS =       10,
		SIN_LEN  =       (1<<SIN_BITS),
		SIN_MASK =       (SIN_LEN-1),

		TL_RES_LEN =     (256),   /* 8 bits addressing (real chip) */

		/*  TL_TAB_LEN is calculated as:
		 *   11 - sinus amplitude bits     (Y axis)
		 *   2  - sinus sign bit           (Y axis)
		 *   TL_RES_LEN - sinus resolution (X axis)
		 */
		TL_TAB_LEN = (11*2*TL_RES_LEN),

		LFO_AM_TAB_ELEMENTS = 210

	};

	static const double ksl_tab[8*16];
	static const UINT32 ksl_shift[4];
	static const UINT32 sl_tab[16];
	static const unsigned char eg_inc[15*RATE_STEPS];
	static const unsigned char eg_rate_select[16+64+16];
	static const unsigned char eg_rate_shift[16+64+16];
	static const UINT8 mul_tab[16];
	static const UINT8 lfo_am_table[LFO_AM_TAB_ELEMENTS];
	static const INT8 lfo_pm_table[8*8];
	static const unsigned char table[19][8];

	int tl_tab[TL_TAB_LEN];

	/* sin waveform table in 'decibel' scale */
	/* two waveforms on OPLL type chips */
	unsigned int sin_tab[SIN_LEN * 2];


	OPLL_CH P_CH[9];                /* OPLL chips have 9 channels*/
	UINT8   instvol_r[9];           /* instrument/volume (or volume/volume in percussive mode)*/

	UINT32  eg_cnt;                 /* global envelope generator counter    */
	UINT32  eg_timer;               /* global envelope generator counter works at frequency = chipclock/72 */
	UINT32  eg_timer_add;           /* step of eg_timer                     */
	UINT32  eg_timer_overflow;      /* envelope generator timer overlfows every 1 sample (on real chip) */

	UINT8   rhythm;                 /* Rhythm mode                  */

	/* LFO */
	UINT32  LFO_AM;
	INT32   LFO_PM;
	UINT32  lfo_am_cnt;
	UINT32  lfo_am_inc;
	UINT32  lfo_pm_cnt;
	UINT32  lfo_pm_inc;

	UINT32  noise_rng;              /* 23 bit noise shift register  */
	UINT32  noise_p;                /* current noise 'phase'        */
	UINT32  noise_f;                /* current noise period         */


/* instrument settings */
/*
    0-user instrument
    1-15 - fixed instruments
    16 -bass drum settings
    17,18 - other percussion instruments
*/
	UINT8 inst_tab[19][8];

	UINT32  fn_tab[1024];           /* fnumber->increment counter   */

	UINT8 address;                  /* address register             */

	signed int output[2];

	// internal state
	sound_stream *  m_stream;

	int limit( int val, int max, int min );
	void advance_lfo();
	void advance();
	int op_calc(UINT32 phase, unsigned int env, signed int pm, unsigned int wave_tab);
	int op_calc1(UINT32 phase, unsigned int env, signed int pm, unsigned int wave_tab);
	void chan_calc( OPLL_CH *CH );
	void rhythm_calc( OPLL_CH *CH, unsigned int noise );
	void key_on(OPLL_SLOT *SLOT, UINT32 key_set);
	void key_off(OPLL_SLOT *SLOT, UINT32 key_clr);
	void calc_fcslot(OPLL_CH *CH, OPLL_SLOT *SLOT);
	void set_mul(int slot,int v);
	void set_ksl_tl(int chan,int v);
	void set_ksl_wave_fb(int chan,int v);
	void set_ar_dr(int slot,int v);
	void set_sl_rr(int slot,int v);
	void load_instrument(UINT32 chan, UINT32 slot, UINT8* inst );
	void update_instrument_zero( UINT8 r );
	void write_reg(int r, int v);

};

extern const device_type YM2413;

#endif /*__YM2413_H__*/
