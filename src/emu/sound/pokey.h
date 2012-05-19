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

/* POKEY WRITE LOGICALS */
#define AUDF1_C     0x00
#define AUDC1_C     0x01
#define AUDF2_C     0x02
#define AUDC2_C     0x03
#define AUDF3_C     0x04
#define AUDC3_C     0x05
#define AUDF4_C     0x06
#define AUDC4_C     0x07
#define AUDCTL_C    0x08
#define STIMER_C    0x09
#define SKREST_C    0x0A
#define POTGO_C     0x0B
#define SEROUT_C    0x0D
#define IRQEN_C     0x0E
#define SKCTL_C     0x0F

/* POKEY READ LOGICALS */
#define POT0_C      0x00
#define POT1_C      0x01
#define POT2_C      0x02
#define POT3_C      0x03
#define POT4_C      0x04
#define POT5_C      0x05
#define POT6_C      0x06
#define POT7_C      0x07
#define ALLPOT_C    0x08
#define KBCODE_C    0x09
#define RANDOM_C    0x0A
#define SERIN_C     0x0D
#define IRQST_C     0x0E
#define SKSTAT_C    0x0F

/* exact 1.79 MHz clock freq (of the Atari 800 that is) */
#define FREQ_17_EXACT   1789790


/*****************************************************************************
 * pot0_r to pot7_r:
 *  Handlers for reading the pot values. Some Atari games use
 *  ALLPOT to return dipswitch settings and other things.
 * serin_r, serout_w, interrupt_cb:
 *  New function pointers for serial input/output and a interrupt callback.
 *****************************************************************************/

typedef struct _pokey_interface pokey_interface;
struct _pokey_interface
{
	devcb_read8 pot_r[8];
	devcb_read8 allpot_r;
	devcb_read8 serin_r;
	devcb_write8 serout_w;
	void (*interrupt_cb)(device_t *device, int mask);
};


READ8_DEVICE_HANDLER( pokey_r );
WRITE8_DEVICE_HANDLER( pokey_w );

/* fix me: eventually this should be a single device with pokey subdevices */
READ8_HANDLER( quad_pokey_r );
WRITE8_HANDLER( quad_pokey_w );

void pokey_serin_ready (device_t *device, int after);
void pokey_break_w (device_t *device, int shift);
void pokey_kbcode_w (device_t *device, int kbcode, int make);

DECLARE_LEGACY_SOUND_DEVICE(POKEY, pokey);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> pokey_device

class pokeyn_device : public device_t,
						public device_sound_interface
{
public:
	// construction/destruction
	pokeyn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_post_load();
	virtual void device_clock_changed();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

	// configuration state
	pokey_interface m_intf;

private:

	void poly_init_4_5(UINT32 *poly, int size, int xorbit, int invert);
	void poly_init_9_17(UINT32 *poly, int size);
	inline void process_channel(int ch);
	inline void reset_channel(int ch);
	inline void inc_chan(int ch);
	inline int check_borrow(int ch);
	void pokey_potgo(void);
	void write_cmd(int offset, UINT8 data);



	// internal state
	sound_stream* m_stream;

	INT32 m_clock_cnt[3];		/* clock counters */
	INT32 m_borrow_cnt[4];	/* borrow counters */

	INT32 m_counter[4];		/* channel counter */
	INT32 m_divisor[4];		/* channel divisor (modulo value) */
	UINT32 m_volume[4];		/* channel volume - derived */
	UINT8 m_output[4];		/* channel output signal (1 active, 0 inactive) */
	UINT8 m_filter_sample[4];  /* hi-pass filter sample */
	UINT32 m_p4;              /* poly4 index */
	UINT32 m_p5;              /* poly5 index */
	UINT32 m_p9;              /* poly9 index */
	UINT32 m_p17;             /* poly17 index */
	UINT32 m_r9;				/* rand9 index */
	UINT32 m_r17;             /* rand17 index */
	UINT32 m_clockmult;		/* clock multiplier */
	emu_timer *m_timer[3];	/* timers for channel 1,2 and 4 events */
	attotime m_timer_period[3];	/* computed periods for these timers */
	int m_timer_param[3];		/* computed parameters for these timers */
	emu_timer *m_rtimer;     /* timer for calculating the random offset */
	emu_timer *m_ptimer[8];	/* pot timers */
	devcb_resolved_read8 m_pot_r[8];
	devcb_resolved_read8 m_allpot_r;
	devcb_resolved_read8 m_serin_r;
	devcb_resolved_write8 m_serout_w;
	void (*m_interrupt_cb)(device_t *device, int mask);
	UINT8 m_AUDF[4];          /* AUDFx (D200, D202, D204, D206) */
	UINT8 m_AUDC[4];			/* AUDCx (D201, D203, D205, D207) */
	UINT8 m_POTx[8];			/* POTx   (R/D200-D207) */
	UINT8 m_AUDCTL;			/* AUDCTL (W/D208) */
	UINT8 m_ALLPOT;			/* ALLPOT (R/D208) */
	UINT8 m_KBCODE;			/* KBCODE (R/D209) */
	UINT8 m_RANDOM;			/* RANDOM (R/D20A) */
	UINT8 m_SERIN;			/* SERIN  (R/D20D) */
	UINT8 m_SEROUT;			/* SEROUT (W/D20D) */
	UINT8 m_IRQST;			/* IRQST  (R/D20E) */
	UINT8 m_IRQEN;			/* IRQEN  (W/D20E) */
	UINT8 m_SKSTAT;			/* SKSTAT (R/D20F) */
	UINT8 m_SKCTL;			/* SKCTL  (W/D20F) */
	attotime m_clock_period;
	attotime m_ad_time_fast;
	attotime m_ad_time_slow;

	UINT32 m_poly4[0x0f];
	UINT32 m_poly5[0x1f];
	UINT32 m_poly9[0x1ff];
	UINT32 m_poly17[0x1ffff];
};


// device type definition
extern const device_type POKEYN;


#endif	/* __POKEY_H__ */
