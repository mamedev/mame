// license:GPL-2.0+
// copyright-holders:Jarek Burczynski,Ernesto Corvi
#ifndef MAME_SOUND_YM2413_H
#define MAME_SOUND_YM2413_H

#pragma once


class ym2413_device : public device_t, public device_sound_interface
{
public:
	ym2413_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(offs_t offset, u8 data);

	void register_port_w(u8 data);
	void data_port_w(u8 data);

protected:
	ym2413_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_clock_changed() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	uint8_t m_inst_table[19][8];

private:
	struct OPLL_SLOT
	{
		uint32_t  ar;         /* attack rate: AR<<2           */
		uint32_t  dr;         /* decay rate:  DR<<2           */
		uint32_t  rr;         /* release rate:RR<<2           */
		uint8_t   KSR;        /* key scale rate               */
		uint8_t   ksl;        /* keyscale level               */
		uint8_t   ksr;        /* key scale rate: kcode>>KSR   */
		uint8_t   mul;        /* multiple: mul_tab[ML]        */

		/* Phase Generator */
		uint32_t  phase;      /* frequency counter            */
		uint32_t  freq;       /* frequency counter step       */
		uint8_t   fb_shift;   /* feedback shift value         */
		int32_t   op1_out[2]; /* slot1 output for feedback    */

		/* Envelope Generator */
		uint8_t   eg_type;    /* percussive/nonpercussive mode*/
		uint8_t   state;      /* phase type                   */
		uint32_t  TL;         /* total level: TL << 2         */
		int32_t   TLL;        /* adjusted now TL              */
		int32_t   volume;     /* envelope counter             */
		uint32_t  sl;         /* sustain level: sl_tab[SL]    */

		uint8_t   eg_sh_dp;   /* (dump state)                 */
		uint8_t   eg_sel_dp;  /* (dump state)                 */
		uint8_t   eg_sh_ar;   /* (attack state)               */
		uint8_t   eg_sel_ar;  /* (attack state)               */
		uint8_t   eg_sh_dr;   /* (decay state)                */
		uint8_t   eg_sel_dr;  /* (decay state)                */
		uint8_t   eg_sh_rr;   /* (release state for non-perc.)*/
		uint8_t   eg_sel_rr;  /* (release state for non-perc.)*/
		uint8_t   eg_sh_rs;   /* (release state for perc.mode)*/
		uint8_t   eg_sel_rs;  /* (release state for perc.mode)*/

		uint32_t  key;        /* 0 = KEY OFF, >0 = KEY ON     */

		/* LFO */
		uint32_t  AMmask;     /* LFO Amplitude Modulation enable mask */
		uint8_t   vib;        /* LFO Phase Modulation enable flag (active high)*/

		/* waveform select */
		unsigned int wavetable;
	};

	struct OPLL_CH
	{
		OPLL_SLOT SLOT[2];
		/* phase generator state */
		uint32_t  block_fnum; /* block+fnum                   */
		uint32_t  fc;         /* Freq. freqement base         */
		uint32_t  ksl_base;   /* KeyScaleLevel Base step      */
		uint8_t   kcode;      /* key code (for key scaling)   */
		uint8_t   sus;        /* sus on/off (release speed in percussive mode)*/
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
	static const uint32_t ksl_shift[4];
	static const uint32_t sl_tab[16];
	static const uint8_t eg_inc[15*RATE_STEPS];
	static const uint8_t eg_rate_select[16+64+16];
	static const uint8_t eg_rate_shift[16+64+16];
	static const uint8_t mul_tab[16];
	static const uint8_t lfo_am_table[LFO_AM_TAB_ELEMENTS];
	static const int8_t lfo_pm_table[8*8];
	static const uint8_t table[19][8];

	int tl_tab[TL_TAB_LEN];

	/* sin waveform table in 'decibel' scale */
	/* two waveforms on OPLL type chips */
	unsigned int sin_tab[SIN_LEN * 2];


	OPLL_CH P_CH[9];                /* OPLL chips have 9 channels*/
	uint8_t   instvol_r[9];           /* instrument/volume (or volume/volume in percussive mode)*/

	uint32_t  eg_cnt;                 /* global envelope generator counter    */
	uint32_t  eg_timer;               /* global envelope generator counter works at frequency = chipclock/72 */
	uint32_t  eg_timer_add;           /* step of eg_timer                     */
	uint32_t  eg_timer_overflow;      /* envelope generator timer overflows every 1 sample (on real chip) */

	uint8_t   rhythm;                 /* Rhythm mode                  */

	/* LFO */
	uint32_t  LFO_AM;
	int32_t   LFO_PM;
	uint32_t  lfo_am_cnt;
	uint32_t  lfo_am_inc;
	uint32_t  lfo_pm_cnt;
	uint32_t  lfo_pm_inc;

	uint32_t  noise_rng;              /* 23 bit noise shift register  */
	uint32_t  noise_p;                /* current noise 'phase'        */
	uint32_t  noise_f;                /* current noise period         */


/* instrument settings */
/*
    0-user instrument
    1-15 - fixed instruments
    16 -bass drum settings
    17,18 - other percussion instruments
*/
	uint8_t inst_tab[19][8];

	uint32_t  fn_tab[1024];           /* fnumber->increment counter   */

	uint8_t address;                  /* address register             */

	signed int output[2];

	// internal state
	sound_stream *  m_stream;

	int limit( int val, int max, int min );
	void advance_lfo();
	void advance();
	int op_calc(uint32_t phase, unsigned int env, signed int pm, unsigned int wave_tab);
	int op_calc1(uint32_t phase, unsigned int env, signed int pm, unsigned int wave_tab);
	void chan_calc( OPLL_CH *CH );
	void rhythm_calc( OPLL_CH *CH, unsigned int noise );
	void key_on(OPLL_SLOT *SLOT, uint32_t key_set);
	void key_off(OPLL_SLOT *SLOT, uint32_t key_clr);
	void calc_fcslot(OPLL_CH *CH, OPLL_SLOT *SLOT);
	void set_mul(int slot,int v);
	void set_ksl_tl(int chan,int v);
	void set_ksl_wave_fb(int chan,int v);
	void set_ar_dr(int slot,int v);
	void set_sl_rr(int slot,int v);
	void load_instrument(uint32_t chan, uint32_t slot, uint8_t* inst );
	void update_instrument_zero( uint8_t r );
	void write_reg(int r, int v);

};

DECLARE_DEVICE_TYPE(YM2413, ym2413_device)

class vrc7snd_device : public ym2413_device
{
public:
	vrc7snd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

private:
	static const uint8_t vrc7_table[19][8];
};

DECLARE_DEVICE_TYPE(VRC7, vrc7snd_device)

#endif // MAME_SOUND_YM2413_H
