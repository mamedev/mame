/*****************************************************************************
 *
 *  POKEY chip emulator 4.3
 *  Copyright Nicola Salmoria and the MAME Team
 *
 *  Based on original info found in Ron Fries' Pokey emulator,
 *  with additions by Brad Oliver, Eric Smith and Juergen Buchmueller.
 *  paddle (a/d conversion) details from the Atari 400/800 Hardware Manual.
 *  Polynome algorithms according to info supplied by Perry McFarlane.
 *
 *  This code is subject to the MAME license, which besides other
 *  things means it is distributed as is, no warranties whatsoever.
 *  For more details read mame.txt that comes with MAME.
 *
 *****************************************************************************/

#pragma once

#ifndef __POKEY_H__
#define __POKEY_H__

#include "devlegcy.h"

/* CONSTANT DEFINITIONS */

/* exact 1.79 MHz clock freq (of the Atari 800 that is) */
#define FREQ_17_EXACT   1789790


/*****************************************************************************
 * pot0_r to pot7_r:
 *  Handlers for reading the pot values. Some Atari games use
 *  ALLPOT to return dipswitch settings and other things.
 * serin_r, serout_w, interrupt_cb:
 *  New function pointers for serial input/output and a interrupt callback.
 *****************************************************************************/

class pokeyn_device;

typedef struct _pokey_interface pokey_interface;
struct _pokey_interface
{
	devcb_read8 pot_r[8];
	devcb_read8 allpot_r;
	devcb_read8 serin_r;
	devcb_write8 serout_w;
	void (*interrupt_cb)(pokeyn_device *device, int mask);
	/* offset = k0 ... k5 , bit0: kr1, bit1: kr2 */
	/* all are, in contrast to actual hardware, ACTIVE_HIGH */
	devcb_read8 kbd_r;
};


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pokey_device

class pokeyn_device : public device_t,
					  public device_sound_interface,
					  public device_execute_interface,
					  public device_execute_state
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
		SYNC_NOOP 		= 11,
		SYNC_SET_IRQST 	= 12,
		SYNC_POT		= 13,
		SYNC_WRITE		= 14
	};

	// construction/destruction
	pokeyn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	UINT8 read(offs_t offset);
	void  write(offs_t offset, UINT8 data);

	void serin_ready(int after);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_post_load();
	virtual void device_clock_changed();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

	virtual void execute_run();

	// configuration state
	pokey_interface m_intf;

	// other internal states
    int m_icount;

private:


    class pokey_channel
    {
    public:
    	pokey_channel();
    	pokeyn_device *m_parent;
    	UINT8 m_INTMask;
    	UINT8 m_AUDF;           /* AUDFx (D200, D202, D204, D206) */
    	UINT8 m_AUDC;			/* AUDCx (D201, D203, D205, D207) */
    	INT32 m_borrow_cnt;		/* borrow counter */
    	INT32 m_counter;		/* channel counter */
    	UINT32 m_volume;		/* channel volume - derived */
    	UINT8 m_output;			/* channel output signal (1 active, 0 inactive) */
    	UINT8 m_filter_sample;  /* high-pass filter sample */
    	UINT8 m_div2;			/* division by 2 */

    	inline void sample(void)			{ m_filter_sample = m_output; }
    	inline void reset_channel(void)		{ m_counter = m_AUDF ^ 0xff; }

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
	inline void process_channel(int ch);
	void pokey_potgo(void);
	char *audc2str(int val);
	char *audctl2str(int val);

	void write_internal(offs_t offset, UINT8 data);

	// internal state
	sound_stream* m_stream;

	pokey_channel m_channel[POKEY_CHANNELS];

	UINT32 m_output;

	INT32 m_clock_cnt[3];		/* clock counters */
	UINT32 m_p4;              /* poly4 index */
	UINT32 m_p5;              /* poly5 index */
	UINT32 m_p9;              /* poly9 index */
	UINT32 m_p17;             /* poly17 index */
	UINT32 m_clockmult;		/* clock multiplier */

	devcb_resolved_read8 m_pot_r[8];
	devcb_resolved_read8 m_allpot_r;
	devcb_resolved_read8 m_serin_r;
	devcb_resolved_write8 m_serout_w;
	devcb_resolved_read8 m_kbd_r;

	void (*m_interrupt_cb)(pokeyn_device *device, int mask);

	UINT8 m_POTx[8];		/* POTx   (R/D200-D207) */
	UINT8 m_AUDCTL;			/* AUDCTL (W/D208) */
	UINT8 m_ALLPOT;			/* ALLPOT (R/D208) */
	UINT8 m_KBCODE;			/* KBCODE (R/D209) */
	UINT8 m_SERIN;			/* SERIN  (R/D20D) */
	UINT8 m_SEROUT;			/* SEROUT (W/D20D) */
	UINT8 m_IRQST;			/* IRQST  (R/D20E) */
	UINT8 m_IRQEN;			/* IRQEN  (W/D20E) */
	UINT8 m_SKSTAT;			/* SKSTAT (R/D20F) */
	UINT8 m_SKCTL;			/* SKCTL  (W/D20F) */

	UINT8 m_pot_counter;
	UINT8 m_kbd_cnt;
	UINT8 m_kbd_latch;
	UINT8 m_kbd_state;

	attotime m_clock_period;

	UINT32 m_poly4[0x0f];
	UINT32 m_poly5[0x1f];
	UINT32 m_poly9[0x1ff];
	UINT32 m_poly17[0x1ffff];
};


// device type definition
extern const device_type POKEYN;


/* fix me: eventually this should be a single device with pokey subdevices */
READ8_HANDLER( quad_pokeyn_r );
WRITE8_HANDLER( quad_pokeyn_w );


#endif	/* __POKEY_H__ */
