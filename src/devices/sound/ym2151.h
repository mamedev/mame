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

#pragma once

#ifndef __YM2151_H__
#define __YM2151_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_YM2151_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, YM2151, _clock)

#define MCFG_YM2151_IRQ_HANDLER(_devcb) \
	devcb = &ym2151_device::set_irq_handler(*device, DEVCB_##_devcb);
#define MCFG_YM2151_PORT_WRITE_HANDLER(_devcb) \
	devcb = &ym2151_device::set_port_write_handler(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> ym2151_device

class ym2151_device :   public device_t,
						public device_sound_interface
{
public:
	// construction/destruction
	ym2151_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object) { return downcast<ym2151_device &>(device).m_irqhandler.set_callback(object); }
	template<class _Object> static devcb_base &set_port_write_handler(device_t &device, _Object object) { return downcast<ym2151_device &>(device).m_portwritehandler.set_callback(object); }

	// read/write
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_READ8_MEMBER( status_r );
	DECLARE_WRITE8_MEMBER( register_w );
	DECLARE_WRITE8_MEMBER( data_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void device_post_load() override;

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
		TL_TAB_LEN = 13*2*TL_RES_LEN,

		SIN_BITS = 10,
		SIN_LEN = 1 << SIN_BITS,
		SIN_MASK = SIN_LEN - 1
	};

	int tl_tab[TL_TAB_LEN];
	unsigned int sin_tab[SIN_LEN];
	UINT32 d1l_tab[16];

	static const UINT8 eg_inc[19*RATE_STEPS];
	static const UINT8 eg_rate_select[32+64+32];
	static const UINT8 eg_rate_shift[32+64+32];
	static const UINT32 dt2_tab[4];
	static const UINT8 dt1_tab[4*32];
	static const UINT16 phaseinc_rom[768];
	static const UINT8 lfo_noise_waveform[256];

	/* struct describing a single operator */
	struct YM2151Operator
	{
		UINT32      phase;                  /* accumulated operator phase */
		UINT32      freq;                   /* operator frequency count */
		INT32       dt1;                    /* current DT1 (detune 1 phase inc/decrement) value */
		UINT32      mul;                    /* frequency count multiply */
		UINT32      dt1_i;                  /* DT1 index * 32 */
		UINT32      dt2;                    /* current DT2 (detune 2) value */

		signed int *connect;                /* operator output 'direction' */

		/* only M1 (operator 0) is filled with this data: */
		signed int *mem_connect;            /* where to put the delayed sample (MEM) */
		INT32       mem_value;              /* delayed sample (MEM) value */

		/* channel specific data; note: each operator number 0 contains channel specific data */
		UINT32      fb_shift;               /* feedback shift value for operators 0 in each channel */
		INT32       fb_out_curr;            /* operator feedback value (used only by operators 0) */
		INT32       fb_out_prev;            /* previous feedback value (used only by operators 0) */
		UINT32      kc;                     /* channel KC (copied to all operators) */
		UINT32      kc_i;                   /* just for speedup */
		UINT32      pms;                    /* channel PMS */
		UINT32      ams;                    /* channel AMS */
		/* end of channel specific data */

		UINT32      AMmask;                 /* LFO Amplitude Modulation enable mask */
		UINT32      state;                  /* Envelope state: 4-attack(AR) 3-decay(D1R) 2-sustain(D2R) 1-release(RR) 0-off */
		UINT8       eg_sh_ar;               /*  (attack state) */
		UINT8       eg_sel_ar;              /*  (attack state) */
		UINT32      tl;                     /* Total attenuation Level */
		INT32       volume;                 /* current envelope attenuation level */
		UINT8       eg_sh_d1r;              /*  (decay state) */
		UINT8       eg_sel_d1r;             /*  (decay state) */
		UINT32      d1l;                    /* envelope switches to sustain state after reaching this level */
		UINT8       eg_sh_d2r;              /*  (sustain state) */
		UINT8       eg_sel_d2r;             /*  (sustain state) */
		UINT8       eg_sh_rr;               /*  (release state) */
		UINT8       eg_sel_rr;              /*  (release state) */

		UINT32      key;                    /* 0=last key was KEY OFF, 1=last key was KEY ON */

		UINT32      ks;                     /* key scale    */
		UINT32      ar;                     /* attack rate  */
		UINT32      d1r;                    /* decay rate   */
		UINT32      d2r;                    /* sustain rate */
		UINT32      rr;                     /* release rate */

		UINT32      reserved0;              /**/
		UINT32      reserved1;              /**/

		void key_on(UINT32 key_set, UINT32 eg_cnt);
		void key_off(UINT32 key_set);
	};

	signed int chanout[8];
	signed int m2,c1,c2; /* Phase Modulation input for operators 2,3,4 */
	signed int mem;     /* one sample delay memory */

	YM2151Operator  oper[32];           /* the 32 operators */

	UINT32      pan[16];                /* channels output masks (0xffffffff = enable) */

	UINT32      eg_cnt;                 /* global envelope generator counter */
	UINT32      eg_timer;               /* global envelope generator counter works at frequency = chipclock/64/3 */
	UINT32      eg_timer_add;           /* step of eg_timer */
	UINT32      eg_timer_overflow;      /* envelope generator timer overlfows every 3 samples (on real chip) */

	UINT32      lfo_phase;              /* accumulated LFO phase (0 to 255) */
	UINT32      lfo_timer;              /* LFO timer                        */
	UINT32      lfo_timer_add;          /* step of lfo_timer                */
	UINT32      lfo_overflow;           /* LFO generates new output when lfo_timer reaches this value */
	UINT32      lfo_counter;            /* LFO phase increment counter      */
	UINT32      lfo_counter_add;        /* step of lfo_counter              */
	UINT8       lfo_wsel;               /* LFO waveform (0-saw, 1-square, 2-triangle, 3-random noise) */
	UINT8       amd;                    /* LFO Amplitude Modulation Depth   */
	INT8        pmd;                    /* LFO Phase Modulation Depth       */
	UINT32      lfa;                    /* LFO current AM output            */
	INT32       lfp;                    /* LFO current PM output            */

	UINT8       test;                   /* TEST register */
	UINT8       ct;                     /* output control pins (bit1-CT2, bit0-CT1) */

	UINT32      noise;                  /* noise enable/period register (bit 7 - noise enable, bits 4-0 - noise period */
	UINT32      noise_rng;              /* 17 bit noise shift register */
	UINT32      noise_p;                /* current noise 'phase'*/
	UINT32      noise_f;                /* current noise period */

	UINT32      csm_req;                /* CSM  KEY ON / KEY OFF sequence request */

	UINT32      irq_enable;             /* IRQ enable for timer B (bit 3) and timer A (bit 2); bit 7 - CSM mode (keyon to all slots, everytime timer A overflows) */
	UINT32      status;                 /* chip status (BUSY, IRQ Flags) */
	UINT8       connect[8];             /* channels connections */

	emu_timer   *timer_A, *timer_A_irq_off;
	emu_timer   *timer_B, *timer_B_irq_off;

	attotime    timer_A_time[1024];     /* timer A times for MAME */
	attotime    timer_B_time[256];      /* timer B times for MAME */
	int         irqlinestate;

	UINT32      timer_A_index;          /* timer A index */
	UINT32      timer_B_index;          /* timer B index */
	UINT32      timer_A_index_old;      /* timer A previous index */
	UINT32      timer_B_index_old;      /* timer B previous index */

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
	UINT32      freq[11*768];           /* 11 octaves, 768 'cents' per octave */

	/*  Frequency deltas for DT1. These deltas alter operator frequency
	*   after it has been taken from frequency-deltas table.
	*/
	INT32       dt1_freq[8*32];         /* 8 DT1 levels, 32 KC values */

	UINT32      noise_tab[32];          /* 17bit Noise Generator periods */

	// internal state
	sound_stream *         m_stream;
	UINT8                  m_lastreg;
	devcb_write_line       m_irqhandler;
	devcb_write8           m_portwritehandler;

	void init_tables();
	void envelope_KONKOFF(YM2151Operator * op, int v);
	void set_connect(YM2151Operator *om1, int cha, int v);
	void advance();
	void advance_eg();
	void write_reg(int r, int v);
	void chan_calc(unsigned int chan);
	void chan7_calc();
	int op_calc(YM2151Operator * OP, unsigned int env, signed int pm);
	int op_calc1(YM2151Operator * OP, unsigned int env, signed int pm);
	void refresh_EG(YM2151Operator * op);
};


// device type definition
extern const device_type YM2151;


#endif /* __2151INTF_H__ */
