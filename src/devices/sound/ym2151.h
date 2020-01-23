// license:GPL-2.0+
// copyright-holders:Jarek Burczynski,Ernesto Corvi
/*
** File: ym2151.h - header file for software implementation of YM2151
**                                            FM Operator Type-M(OPM)
**
** (c) 1997-2002 Jarek Burczynski (s0246@poczta.onet.pl, bujar@mame.net)
** Some of the optimizing ideas by Tatsuyuki Satoh
**
** Version 2.150 final beta May, 11th 2002
**
**
** I would like to thank following people for making this project possible:
**
** Beauty Planets - for making a lot of real YM2151 samples and providing
** additional informations about the chip. Also for the time spent making
** the samples and the speed of replying to my endless requests.
**
** Shigeharu Isoda - for general help, for taking time to scan his YM2151
** Japanese Manual first of all, and answering MANY of my questions.
**
** Nao - for giving me some info about YM2151 and pointing me to Shigeharu.
** Also for creating fmemu (which I still use to test the emulator).
**
** Aaron Giles and Chris Hardy - they made some samples of one of my favourite
** arcade games so I could compare it to my emulator.
**
** Bryan McPhail and Tim (powerjaw) - for making some samples.
**
** Ishmair - for the datasheet and motivation.
*/

#ifndef MAME_SOUND_YM2151_H
#define MAME_SOUND_YM2151_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> ym2151_device

class ym2151_device :   public device_t,
						public device_sound_interface,
						public device_memory_interface
{
public:
	// construction/destruction
	ym2151_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto irq_handler() { return m_irqhandler.bind(); }
	auto port_write_handler() { return m_portwritehandler.bind(); }

	// read/write
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	u8 status_r();
	void register_w(u8 data);
	void data_w(u8 data);

	DECLARE_WRITE_LINE_MEMBER(reset_w);

	// overall control registers
	void test_w(u8 data);
	void keyonoff_w(u8 data);
	void noise_w(u8 data);
	void timer_a_hi_w(u8 data);
	void timer_a_lo_w(u8 data);
	void timer_b_w(u8 data);
	void timer_ctrl_w(u8 data);
	void lfo_freq_w(u8 data);
	void lfo_pmd_amd_w(u8 data);
	void lfo_wave_ct_w(u8 data);

	// each channel registers
	void rl_fb_connect_w(offs_t offset, u8 data);
	void keycode_w(offs_t offset, u8 data);
	void keyfrac_w(offs_t offset, u8 data);
	void pms_ams_w(offs_t offset, u8 data);

	// each operator registers
	void dt1_mul_w(offs_t offset, u8 data);
	void tl_w(offs_t offset, u8 data);
	void ks_ar_w(offs_t offset, u8 data);
	void am_d1r_w(offs_t offset, u8 data);
	void dt2_d2r_w(offs_t offset, u8 data);
	void dl1_rr_w(offs_t offset, u8 data);

	void map(address_map &map);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void device_post_load() override;
	virtual void device_clock_changed() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	address_space_config        m_reg_config;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	enum {
		TIMER_IRQ_A_OFF,
		TIMER_IRQ_B_OFF,
		TIMER_A,
		TIMER_B
	};

	enum {
		RATE_STEPS = 8,
		TL_RES_LEN = 256, /* 8 bits addressing (real chip) */

		/*  TL_TAB_LEN is calculated as:
		 *   13 - sinus amplitude bits     (Y axis)
		 *   2  - sinus sign bit           (Y axis)
		 *   TL_RES_LEN - sinus resolution (X axis)
		 */
		TL_TAB_LEN = 13 * 2 * TL_RES_LEN,

		SIN_BITS = 10,
		SIN_LEN = 1 << SIN_BITS,
		SIN_MASK = SIN_LEN - 1
	};

	int tl_tab[TL_TAB_LEN];
	u32 sin_tab[SIN_LEN];
	u32 d1l_tab[16];

	static const u8 eg_inc[19 * RATE_STEPS];
	static const u8 eg_rate_select[32 + 64 + 32];
	static const u8 eg_rate_shift[32 + 64 + 32];
	static const u32 dt2_tab[4];
	static const u8 dt1_tab[4 * 32];
	static const u16 phaseinc_rom[768];
	static const u8 lfo_noise_waveform[256];

	/* struct describing a single operator */
	struct YM2151Operator
	{
		u32      phase;                  /* accumulated operator phase */
		u32      freq;                   /* operator frequency count */
		s32      dt1;                    /* current DT1 (detune 1 phase inc/decrement) value */
		u32      mul;                    /* frequency count multiply */
		u32      dt1_i;                  /* DT1 index * 32 */
		u32      dt2;                    /* current DT2 (detune 2) value */

		s32     *connect;                /* operator output 'direction' */

		/* only M1 (operator 0) is filled with this data: */
		s32     *mem_connect;            /* where to put the delayed sample (MEM) */
		s32      mem_value;              /* delayed sample (MEM) value */

		/* channel specific data; note: each operator number 0 contains channel specific data */
		u32      fb_shift;               /* feedback shift value for operators 0 in each channel */
		s32      fb_out_curr;            /* operator feedback value (used only by operators 0) */
		s32      fb_out_prev;            /* previous feedback value (used only by operators 0) */
		u32      kc;                     /* channel KC (copied to all operators) */
		u32      kc_i;                   /* just for speedup */
		u32      pms;                    /* channel PMS */
		u32      ams;                    /* channel AMS */
		/* end of channel specific data */

		u32      AMmask;                 /* LFO Amplitude Modulation enable mask */
		u32      state;                  /* Envelope state: 4-attack(AR) 3-decay(D1R) 2-sustain(D2R) 1-release(RR) 0-off */
		u8       eg_sh_ar;               /*  (attack state) */
		u8       eg_sel_ar;              /*  (attack state) */
		u32      tl;                     /* Total attenuation Level */
		s32      volume;                 /* current envelope attenuation level */
		u8       eg_sh_d1r;              /*  (decay state) */
		u8       eg_sel_d1r;             /*  (decay state) */
		u32      d1l;                    /* envelope switches to sustain state after reaching this level */
		u8       eg_sh_d2r;              /*  (sustain state) */
		u8       eg_sel_d2r;             /*  (sustain state) */
		u8       eg_sh_rr;               /*  (release state) */
		u8       eg_sel_rr;              /*  (release state) */

		u32      key;                    /* 0=last key was KEY OFF, 1=last key was KEY ON */

		u32      ks;                     /* key scale    */
		u32      ar;                     /* attack rate  */
		u32      d1r;                    /* decay rate   */
		u32      d2r;                    /* sustain rate */
		u32      rr;                     /* release rate */

		u32      reserved0;              /**/
		u32      reserved1;              /**/

		void key_on(u32 key_set, u32 eg_cnt);
		void key_off(u32 key_set);
	};

	s32 chanout[8];
	s32 m2,c1,c2; /* Phase Modulation input for operators 2,3,4 */
	s32 mem;     /* one sample delay memory */

	YM2151Operator  oper[32];           /* the 32 operators */

	u32      pan[16];                /* channels output masks (0xffffffff = enable) */

	u32      eg_cnt;                 /* global envelope generator counter */
	u32      eg_timer;               /* global envelope generator counter works at frequency = chipclock/64/3 */
	u32      eg_timer_add;           /* step of eg_timer */
	u32      eg_timer_overflow;      /* envelope generator timer overflows every 3 samples (on real chip) */

	u32      lfo_phase;              /* accumulated LFO phase (0 to 255) */
	u32      lfo_timer;              /* LFO timer                        */
	u32      lfo_timer_add;          /* step of lfo_timer                */
	u32      lfo_overflow;           /* LFO generates new output when lfo_timer reaches this value */
	u32      lfo_counter;            /* LFO phase increment counter      */
	u32      lfo_counter_add;        /* step of lfo_counter              */
	u8       lfo_wsel;               /* LFO waveform (0-saw, 1-square, 2-triangle, 3-random noise) */
	u8       amd;                    /* LFO Amplitude Modulation Depth   */
	s8       pmd;                    /* LFO Phase Modulation Depth       */
	u32      lfa;                    /* LFO current AM output            */
	s32      lfp;                    /* LFO current PM output            */

	u8       test;                   /* TEST register */
	u8       ct;                     /* output control pins (bit1-CT2, bit0-CT1) */

	u32      noise;                  /* noise enable/period register (bit 7 - noise enable, bits 4-0 - noise period */
	u32      noise_rng;              /* 17 bit noise shift register */
	u32      noise_p;                /* current noise 'phase'*/
	u32      noise_f;                /* current noise period */

	u32      csm_req;                /* CSM  KEY ON / KEY OFF sequence request */

	u32      irq_enable;             /* IRQ enable for timer B (bit 3) and timer A (bit 2); bit 7 - CSM mode (keyon to all slots, everytime timer A overflows) */
	u32      status;                 /* chip status (BUSY, IRQ Flags) */
	u8       connect[8];             /* channels connections */

	emu_timer   *timer_A, *timer_A_irq_off;
	emu_timer   *timer_B, *timer_B_irq_off;

	attotime    timer_A_time[1024];     /* timer A times for MAME */
	attotime    timer_B_time[256];      /* timer B times for MAME */
	int         irqlinestate;

	u32      timer_A_index;          /* timer A index */
	u32      timer_B_index;          /* timer B index */
	u32      timer_A_index_old;      /* timer A previous index */
	u32      timer_B_index_old;      /* timer B previous index */

	/*  Frequency-deltas to get the closest frequency possible.
	*   There are 11 octaves because of DT2 (max 950 cents over base frequency)
	*   and LFO phase modulation (max 800 cents below AND over base frequency)
	*   Summary:   octave  explanation
	*              0       note code - LFO PM
	*              1       note code
	*              2       note code
	*              3       note code
	*              4       note code
	*              5       note code
	*              6       note code
	*              7       note code
	*              8       note code
	*              9       note code + DT2 + LFO PM
	*              10      note code + DT2 + LFO PM
	*/
	u32      freq[11 * 768];         /* 11 octaves, 768 'cents' per octave */

	/*  Frequency deltas for DT1. These deltas alter operator frequency
	*   after it has been taken from frequency-deltas table.
	*/
	s32      dt1_freq[8 * 32];       /* 8 DT1 levels, 32 KC values */

	u32      noise_tab[32];          /* 17bit Noise Generator periods */

	// internal state
	sound_stream *         m_stream;
	u8                     m_lastreg;
	devcb_write_line       m_irqhandler;
	devcb_write8           m_portwritehandler;
	bool                   m_reset_active;

	void init_tables();
	void calculate_timers();
	void envelope_KONKOFF(YM2151Operator *op, int v);
	void set_connect(YM2151Operator *om1, int cha, int v);
	void advance();
	void advance_eg();
	void chan_calc(u32 chan);
	void chan7_calc();
	int op_calc(YM2151Operator *OP, u32 env, s32 pm);
	int op_calc1(YM2151Operator *OP, u32 env, s32 pm);
	void refresh_EG(YM2151Operator *op);
};


// device type definition
DECLARE_DEVICE_TYPE(YM2151, ym2151_device)


#endif // MAME_SOUND_YM2151_H
