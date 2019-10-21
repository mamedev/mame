// license:BSD-3-Clause
// copyright-holders:Brad Oliver, Eric Smith, Juergen Buchmueller
/*****************************************************************************
 *
 *  POKEY chip emulator 4.6
 *
 *  Based on original info found in Ron Fries' Pokey emulator,
 *  with additions by Brad Oliver, Eric Smith and Juergen Buchmueller.
 *  paddle (a/d conversion) details from the Atari 400/800 Hardware Manual.
 *  Polynomial algorithms according to info supplied by Perry McFarlane.
 *
 *****************************************************************************/

#ifndef MAME_SOUND_POKEY_H
#define MAME_SOUND_POKEY_H

#pragma once

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

//**************************************************************************
//  CALLBACK HANDLERS
//**************************************************************************

#define POKEY_KEYBOARD_CB_MEMBER(_name) uint8_t _name(uint8_t k543210)
#define POKEY_INTERRUPT_CB_MEMBER(_name) void _name(int mask)

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
	/* CONSTANT DEFINITIONS */

	/* exact 1.79 MHz clock freq (of the Atari 800 that is) */
	static constexpr unsigned FREQ_17_EXACT = 1789790;

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
	pokey_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <unsigned N> auto pot_r() { return m_pot_r_cb[N].bind(); }
	auto allpot_r() { return m_allpot_r_cb.bind(); }
	auto serin_r() { return m_serin_r_cb.bind(); }
	auto serout_w() { return m_serout_w_cb.bind(); }

	/* k543210 = k5 ... k0 returns bit0: kr1, bit1: kr2 */
	/* all are, in contrast to actual hardware, ACTIVE_HIGH */
	typedef device_delegate<uint8_t (uint8_t k543210)> kb_cb_delegate;
	void set_keyboard_callback(kb_cb_delegate callback) { m_keyboard_r = callback; }
	template <class FunctionClass> void set_keyboard_callback(const char *devname, uint8_t (FunctionClass::*callback)(uint8_t), const char *name)
	{
		set_keyboard_callback(kb_cb_delegate(callback, name, devname, static_cast<FunctionClass *>(nullptr)));
	}
	template <class FunctionClass> void set_keyboard_callback(uint8_t (FunctionClass::*callback)(uint8_t), const char *name)
	{
		set_keyboard_callback(kb_cb_delegate(callback, name, nullptr, static_cast<FunctionClass *>(nullptr)));
	}

	typedef device_delegate<void (int mask)> int_cb_delegate;
	void set_interrupt_callback(int_cb_delegate callback) { m_irq_f = callback; }
	template <class FunctionClass> void set_interrupt_callback(const char *devname, void (FunctionClass::*callback)(int), const char *name)
	{
		set_interrupt_callback(int_cb_delegate(callback, name, devname, static_cast<FunctionClass *>(nullptr)));
	}
	template <class FunctionClass> void set_interrupt_callback(void (FunctionClass::*callback)(int), const char *name)
	{
		set_interrupt_callback(int_cb_delegate(callback, name, nullptr, static_cast<FunctionClass *>(nullptr)));
	}

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	DECLARE_WRITE_LINE_MEMBER( sid_w ); // pin 24
	void serin_ready(int after);

	// analog output configuration
	void set_output_rc(double r, double c, double v)
	{
		m_output_type = pokey_device::RC_LOWPASS;
		m_r_pullup = r;
		m_cap = c;
		m_v_ref = v;
	}

	/* C ignored, please see pokey.c */
	void set_output_opamp(double r, double c, double v)
	{
		m_output_type = pokey_device::OPAMP_C_TO_GROUND;
		m_r_pullup = r;
		m_cap = c;
		m_v_ref = v;
	}

	void set_output_opamp_low_pass(double r, double c, double v)
	{
		m_output_type = pokey_device::OPAMP_LOW_PASS;
		m_r_pullup = r;
		m_cap = c;
		m_v_ref = v;
	}

	void set_output_discrete()
	{
		m_output_type = pokey_device::DISCRETE_VAR_R;
	}

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

	//virtual uint32_t execute_min_cycles() const { return 114; }
	// other internal states
	int m_icount;

private:

	class pokey_channel
	{
	public:
		pokey_channel();
		pokey_device *m_parent;
		uint8_t m_INTMask;
		uint8_t m_AUDF;           /* AUDFx (D200, D202, D204, D206) */
		uint8_t m_AUDC;           /* AUDCx (D201, D203, D205, D207) */
		int32_t m_borrow_cnt;     /* borrow counter */
		int32_t m_counter;        /* channel counter */
		uint8_t m_output;         /* channel output signal (1 active, 0 inactive) */
		uint8_t m_filter_sample;  /* high-pass filter sample */

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

	static constexpr int POKEY_CHANNELS = 4;

	void step_one_clock();
	void step_keyboard();
	void step_pot();

	void poly_init_4_5(uint32_t *poly, int size, int xorbit, int invert);
	void poly_init_9_17(uint32_t *poly, int size);
	void vol_init();

	inline void process_channel(int ch);
	void pokey_potgo(void);
	char *audc2str(int val);
	char *audctl2str(int val);

	void write_internal(offs_t offset, uint8_t data);

	// internal state
	sound_stream* m_stream;

	pokey_channel m_channel[POKEY_CHANNELS];

	uint32_t m_out_raw;         /* raw output */
	bool m_old_raw_inval;       /* true: recalc m_out_raw required */
	double m_out_filter;        /* filtered output */

	int32_t m_clock_cnt[3];     /* clock counters */
	uint32_t m_p4;              /* poly4 index */
	uint32_t m_p5;              /* poly5 index */
	uint32_t m_p9;              /* poly9 index */
	uint32_t m_p17;             /* poly17 index */

	devcb_read8 m_pot_r_cb[8];
	devcb_read8 m_allpot_r_cb;
	devcb_read8 m_serin_r_cb;
	devcb_write8 m_serout_w_cb;

	kb_cb_delegate m_keyboard_r;
	int_cb_delegate m_irq_f;

	uint8_t m_POTx[8];        /* POTx   (R/D200-D207) */
	uint8_t m_AUDCTL;         /* AUDCTL (W/D208) */
	uint8_t m_ALLPOT;         /* ALLPOT (R/D208) */
	uint8_t m_KBCODE;         /* KBCODE (R/D209) */
	uint8_t m_SERIN;          /* SERIN  (R/D20D) */
	uint8_t m_SEROUT;         /* SEROUT (W/D20D) */
	uint8_t m_IRQST;          /* IRQST  (R/D20E) */
	uint8_t m_IRQEN;          /* IRQEN  (W/D20E) */
	uint8_t m_SKSTAT;         /* SKSTAT (R/D20F) */
	uint8_t m_SKCTL;          /* SKCTL  (W/D20F) */

	uint8_t m_pot_counter;
	uint8_t m_kbd_cnt;
	uint8_t m_kbd_latch;
	uint8_t m_kbd_state;

	attotime m_clock_period;

	uint32_t m_poly4[0x0f];
	uint32_t m_poly5[0x1f];
	uint32_t m_poly9[0x1ff];
	uint32_t m_poly17[0x1ffff];
	uint32_t m_voltab[0x10000];

	output_type m_output_type;
	double m_r_pullup;
	double m_cap;
	double m_v_ref;
};


// device type definition
DECLARE_DEVICE_TYPE(POKEY, pokey_device)

#endif // MAME_SOUND_POKEY_H
