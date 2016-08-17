// license:BSD-3-Clause
// copyright-holders:Brad Oliver, Eric Smith, Juergen Buchmueller
/*****************************************************************************
 *
 *  POKEY chip emulator 4.6
 *
 *  Based on original info found in Ron Fries' Pokey emulator,
 *  with additions by Brad Oliver, Eric Smith and Juergen Buchmueller.
 *  paddle (a/d conversion) details from the Atari 400/800 Hardware Manual.
 *  Polynome algorithms according to info supplied by Perry McFarlane.
 *
 *****************************************************************************/

#pragma once

#ifndef __POKEY_H__
#define __POKEY_H__

#include "machine/rescap.h"

/*
 *  ATARI Pokey (CO12294) pin-out
 *
                 +-----------------+
        VSS      |  1           40 |  D2
        D3       |  2           39 |  D1
        D4       |  3           38 |  D0
        D5       |  4           37 |  AUD
        D6       |  5           36 |  A0
        D7       |  6           35 |  A1
        PHI2     |  7           34 |  A2
        P6       |  8           33 |  A3
        P7       |  9           32 |  R / /W
        P4       | 10           31 |  CS1
        P5       | 11           30 |  /CS0
        P2       | 12           29 |  IRQ
        P3       | 13           28 |  SOD
        P0       | 14           27 |  ACLK
        P1       | 15           26 |  BCLK
        /KR2     | 16           25 |  /KR1
        VCC      | 17           24 |  SID
        /K5      | 18           23 |  /K0
        /K4      | 19           22 |  /K1
        /K3      | 20           21 |  /K2
                 +-----------------+
 *
 */

/* CONSTANT DEFINITIONS */

/* exact 1.79 MHz clock freq (of the Atari 800 that is) */
#define FREQ_17_EXACT   1789790

//**************************************************************************
//  CALLBACK HANDLERS
//**************************************************************************

typedef device_delegate<UINT8 (UINT8 k543210)> pokey_kb_cb_delegate;
typedef device_delegate<void (int mask)> pokey_int_cb_delegate;

#define POKEY_KEYBOARD_CB_MEMBER(_name) UINT8 _name(UINT8 k543210)
#define POKEY_INTERRUPT_CB_MEMBER(_name) void _name(int mask)


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_POKEY_POT0_R_CB(_devcb) \
	devcb = &pokey_device::set_pot0_r_callback(*device, DEVCB_##_devcb);

#define MCFG_POKEY_POT1_R_CB(_devcb) \
	devcb = &pokey_device::set_pot1_r_callback(*device, DEVCB_##_devcb);

#define MCFG_POKEY_POT2_R_CB(_devcb) \
	devcb = &pokey_device::set_pot2_r_callback(*device, DEVCB_##_devcb);

#define MCFG_POKEY_POT3_R_CB(_devcb) \
	devcb = &pokey_device::set_pot3_r_callback(*device, DEVCB_##_devcb);

#define MCFG_POKEY_POT4_R_CB(_devcb) \
	devcb = &pokey_device::set_pot4_r_callback(*device, DEVCB_##_devcb);

#define MCFG_POKEY_POT5_R_CB(_devcb) \
	devcb = &pokey_device::set_pot5_r_callback(*device, DEVCB_##_devcb);

#define MCFG_POKEY_POT6_R_CB(_devcb) \
	devcb = &pokey_device::set_pot6_r_callback(*device, DEVCB_##_devcb);

#define MCFG_POKEY_POT7_R_CB(_devcb) \
	devcb = &pokey_device::set_pot7_r_callback(*device, DEVCB_##_devcb);

#define MCFG_POKEY_ALLPOT_R_CB(_devcb) \
	devcb = &pokey_device::set_allpot_r_callback(*device, DEVCB_##_devcb);

#define MCFG_POKEY_SERIN_R_CB(_devcb) \
	devcb = &pokey_device::set_serin_r_callback(*device, DEVCB_##_devcb);

#define MCFG_POKEY_SEROUT_W_CB(_devcb) \
	devcb = &pokey_device::set_serout_w_callback(*device, DEVCB_##_devcb);

/* k543210 = k5 ... k0 returns bit0: kr1, bit1: kr2 */
/* all are, in contrast to actual hardware, ACTIVE_HIGH */
#define MCFG_POKEY_KEYBOARD_CB(_class, _method) \
	pokey_device::set_keyboard_callback(*device, pokey_kb_cb_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_POKEY_INTERRUPT_CB(_class, _method) \
	pokey_device::set_interrupt_callback(*device, pokey_int_cb_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));


#define MCFG_POKEY_OUTPUT_RC(_R, _C, _V) \
	(downcast<pokey_device *>(device))->m_output_type = pokey_device::RC_LOWPASS; \
	(downcast<pokey_device *>(device))->m_r_pullup = (_R); \
	(downcast<pokey_device *>(device))->m_cap = (_C); \
	(downcast<pokey_device *>(device))->m_v_ref = (_V);

/* C ignored, please see pokey.c */

#define MCFG_POKEY_OUTPUT_OPAMP(_R, _C, _V) \
	(downcast<pokey_device *>(device))->m_output_type = pokey_device::OPAMP_C_TO_GROUND; \
	(downcast<pokey_device *>(device))->m_r_pullup = (_R); \
	(downcast<pokey_device *>(device))->m_cap = (_C); \
	(downcast<pokey_device *>(device))->m_v_ref = (_V);

#define MCFG_POKEY_OUTPUT_OPAMP_LOW_PASS(_R, _C, _V) \
	(downcast<pokey_device *>(device))->m_output_type = pokey_device::OPAMP_LOW_PASS; \
	(downcast<pokey_device *>(device))->m_r_pullup = (_R); \
	(downcast<pokey_device *>(device))->m_cap = (_C); \
	(downcast<pokey_device *>(device))->m_v_ref = (_V);

#define MCFG_POKEY_OUTPUT_DISCRETE() \
	(downcast<pokey_device *>(device))->m_output_type = pokey_device::DISCRETE_VAR_R;

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pokey_device

class pokey_device : public device_t,
						public device_sound_interface,
						public device_execute_interface,
						public device_state_interface
{
public:

	enum
	{
		POK_KEY_BREAK = 0x30,
		POK_KEY_SHIFT = 0x20,
		POK_KEY_CTRL  = 0x00
	};

	enum
	{
		/* POKEY WRITE LOGICALS */
		AUDF1_C  =   0x00,
		AUDC1_C  =   0x01,
		AUDF2_C  =   0x02,
		AUDC2_C  =   0x03,
		AUDF3_C  =   0x04,
		AUDC3_C  =   0x05,
		AUDF4_C  =   0x06,
		AUDC4_C  =   0x07,
		AUDCTL_C =   0x08,
		STIMER_C =   0x09,
		SKREST_C =   0x0A,
		POTGO_C  =   0x0B,
		SEROUT_C =   0x0D,
		IRQEN_C  =   0x0E,
		SKCTL_C  =   0x0F
	};

	enum
	{
		/* POKEY READ LOGICALS */
		POT0_C   =  0x00,
		POT1_C   =  0x01,
		POT2_C   =  0x02,
		POT3_C   =  0x03,
		POT4_C   =  0x04,
		POT5_C   =  0x05,
		POT6_C   =  0x06,
		POT7_C   =  0x07,
		ALLPOT_C =  0x08,
		KBCODE_C =  0x09,
		RANDOM_C =  0x0A,
		SERIN_C  =  0x0D,
		IRQST_C  =  0x0E,
		SKSTAT_C =  0x0F
	};

	enum /* sync-operations */
	{
		SYNC_NOOP       = 11,
		SYNC_SET_IRQST  = 12,
		SYNC_POT        = 13,
		SYNC_WRITE      = 14
	};

	enum output_type
	{
		LEGACY_LINEAR = 0,
		RC_LOWPASS,
		OPAMP_C_TO_GROUND,
		OPAMP_LOW_PASS,
		DISCRETE_VAR_R
	};

	// construction/destruction
	pokey_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_pot0_r_callback(device_t &device, _Object object) { return downcast<pokey_device &>(device).m_pot0_r_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pot1_r_callback(device_t &device, _Object object) { return downcast<pokey_device &>(device).m_pot1_r_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pot2_r_callback(device_t &device, _Object object) { return downcast<pokey_device &>(device).m_pot2_r_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pot3_r_callback(device_t &device, _Object object) { return downcast<pokey_device &>(device).m_pot3_r_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pot4_r_callback(device_t &device, _Object object) { return downcast<pokey_device &>(device).m_pot4_r_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pot5_r_callback(device_t &device, _Object object) { return downcast<pokey_device &>(device).m_pot5_r_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pot6_r_callback(device_t &device, _Object object) { return downcast<pokey_device &>(device).m_pot6_r_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pot7_r_callback(device_t &device, _Object object) { return downcast<pokey_device &>(device).m_pot7_r_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_allpot_r_callback(device_t &device, _Object object) { return downcast<pokey_device &>(device).m_allpot_r_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_serin_r_callback(device_t &device, _Object object) { return downcast<pokey_device &>(device).m_serin_r_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_serout_w_callback(device_t &device, _Object object) { return downcast<pokey_device &>(device).m_serout_w_cb.set_callback(object); }

	static void set_keyboard_callback(device_t &device, pokey_kb_cb_delegate callback) { downcast<pokey_device &>(device).m_keyboard_r = callback; }
	static void set_interrupt_callback(device_t &device, pokey_int_cb_delegate callback) { downcast<pokey_device &>(device).m_irq_f = callback; }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	UINT8 read(offs_t offset);
	void  write(offs_t offset, UINT8 data);

	DECLARE_WRITE_LINE_MEMBER( sid_w ); // pin 24
	void serin_ready(int after);

	// analog output configuration

	output_type  m_output_type;
	double m_r_pullup;
	double m_cap;
	double m_v_ref;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;
	virtual void device_clock_changed() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	virtual void execute_run() override;

	//virtual UINT32 execute_min_cycles() const { return 114; }
	// other internal states
	int m_icount;

private:


	class pokey_channel
	{
	public:
		pokey_channel();
		pokey_device *m_parent;
		UINT8 m_INTMask;
		UINT8 m_AUDF;           /* AUDFx (D200, D202, D204, D206) */
		UINT8 m_AUDC;           /* AUDCx (D201, D203, D205, D207) */
		INT32 m_borrow_cnt;     /* borrow counter */
		INT32 m_counter;        /* channel counter */
		UINT8 m_output;         /* channel output signal (1 active, 0 inactive) */
		UINT8 m_filter_sample;  /* high-pass filter sample */
		UINT8 m_div2;           /* division by 2 */

		inline void sample(void)            { m_filter_sample = m_output; }
		inline void reset_channel(void)     { m_counter = m_AUDF ^ 0xff; }

		inline void inc_chan()
		{
			m_counter = (m_counter + 1) & 0xff;
			if (m_counter == 0 && m_borrow_cnt == 0)
			{
				m_borrow_cnt = 3;
				if (m_parent->m_IRQEN & m_INTMask)
				{
					/* Exposed state has changed: This should only be updated after a resync ... */
					m_parent->synchronize(SYNC_SET_IRQST, m_INTMask);
				}
			}
		}

		inline int check_borrow()
		{
			if (m_borrow_cnt > 0)
			{
				m_borrow_cnt--;
				return (m_borrow_cnt == 0);
			}
			return 0;
		}
	};

	static const int POKEY_CHANNELS = 4;

	UINT32 step_one_clock();
	void step_keyboard();
	void step_pot();

	void poly_init_4_5(UINT32 *poly, int size, int xorbit, int invert);
	void poly_init_9_17(UINT32 *poly, int size);
	void vol_init();

	inline void process_channel(int ch);
	void pokey_potgo(void);
	char *audc2str(int val);
	char *audctl2str(int val);

	void write_internal(offs_t offset, UINT8 data);

	// internal state
	sound_stream* m_stream;

	pokey_channel m_channel[POKEY_CHANNELS];

	UINT32 m_output;        /* raw output */
	double m_out_filter;    /* filtered output */

	INT32 m_clock_cnt[3];       /* clock counters */
	UINT32 m_p4;              /* poly4 index */
	UINT32 m_p5;              /* poly5 index */
	UINT32 m_p9;              /* poly9 index */
	UINT32 m_p17;             /* poly17 index */

	devcb_read8 m_pot0_r_cb;
	devcb_read8 m_pot1_r_cb;
	devcb_read8 m_pot2_r_cb;
	devcb_read8 m_pot3_r_cb;
	devcb_read8 m_pot4_r_cb;
	devcb_read8 m_pot5_r_cb;
	devcb_read8 m_pot6_r_cb;
	devcb_read8 m_pot7_r_cb;
	devcb_read8 m_allpot_r_cb;
	devcb_read8 m_serin_r_cb;
	devcb_write8 m_serout_w_cb;

	pokey_kb_cb_delegate m_keyboard_r;
	pokey_int_cb_delegate m_irq_f;

	UINT8 m_POTx[8];        /* POTx   (R/D200-D207) */
	UINT8 m_AUDCTL;         /* AUDCTL (W/D208) */
	UINT8 m_ALLPOT;         /* ALLPOT (R/D208) */
	UINT8 m_KBCODE;         /* KBCODE (R/D209) */
	UINT8 m_SERIN;          /* SERIN  (R/D20D) */
	UINT8 m_SEROUT;         /* SEROUT (W/D20D) */
	UINT8 m_IRQST;          /* IRQST  (R/D20E) */
	UINT8 m_IRQEN;          /* IRQEN  (W/D20E) */
	UINT8 m_SKSTAT;         /* SKSTAT (R/D20F) */
	UINT8 m_SKCTL;          /* SKCTL  (W/D20F) */

	UINT8 m_pot_counter;
	UINT8 m_kbd_cnt;
	UINT8 m_kbd_latch;
	UINT8 m_kbd_state;

	attotime m_clock_period;

	UINT32 m_poly4[0x0f];
	UINT32 m_poly5[0x1f];
	UINT32 m_poly9[0x1ff];
	UINT32 m_poly17[0x1ffff];
	UINT32 m_voltab[0x10000];
};


// device type definition
extern const device_type POKEY;

#endif  /* __POKEY_H__ */
