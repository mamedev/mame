// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo, Aaron Giles, Jonathan Gevaryahu, Raphael Nabet, Couriersud, Michael Zapf
#pragma once

#ifndef __TMS5220_H__
#define __TMS5220_H__

#include "emu.h"
#include "machine/spchrom.h"

/* HACK: if defined, uses impossibly perfect 'straight line' interpolation */
#undef PERFECT_INTERPOLATION_HACK

#define FIFO_SIZE 16

/* clock rate = 80 * output sample rate,     */
/* usually 640000 for 8000 Hz sample rate or */
/* usually 800000 for 10000 Hz sample rate.  */

/* IRQ callback function, active low, i.e. state=0 */
#define MCFG_TMS52XX_IRQ_HANDLER(_devcb) \
	devcb = &tms5220_device::set_irq_handler(*device, DEVCB_##_devcb);

/* Ready callback function, active low, i.e. state=0 */
#define MCFG_TMS52XX_READYQ_HANDLER(_devcb) \
	devcb = &tms5220_device::set_readyq_handler(*device, DEVCB_##_devcb);

#define MCFG_TMS52XX_SPEECHROM(_tag) \
	tms5220_device::set_speechrom_tag(*device, _tag);

class tms5220_device : public device_t,
									public device_sound_interface
{
public:
	tms5220_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tms5220_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// static configuration helpers
	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object) { return downcast<tms5220_device &>(device).m_irq_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_readyq_handler(device_t &device, _Object object) { return downcast<tms5220_device &>(device).m_readyq_handler.set_callback(object); }
	static void set_speechrom_tag(device_t &device, const char *_tag) { downcast<tms5220_device &>(device).m_speechrom_tag = _tag; }

	/* Control lines - once written to will switch interface into
	 * "true" timing behaviour.
	 */

	/* all lines with suffix q are active low! */

	WRITE_LINE_MEMBER( rsq_w );
	WRITE_LINE_MEMBER( wsq_w );

	DECLARE_WRITE8_MEMBER( data_w );
	DECLARE_READ8_MEMBER( status_r );

	READ_LINE_MEMBER( readyq_r );
	READ_LINE_MEMBER( intq_r );

	double time_to_ready();

	void set_frequency(int frequency);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

	void set_variant(int variant);

private:
	void register_for_save_states();
	void data_write(int data);
	void update_fifo_status_and_ints();
	int extract_bits(int count);
	int status_read();
	int ready_read();
	int cycles_to_ready();
	int int_read();
	void process(INT16 *buffer, unsigned int size);
	INT32 lattice_filter();
	void process_command(unsigned char cmd);
	void parse_frame();
	void set_interrupt_state(int state);
	void update_ready_state();

	// internal state

	/* coefficient tables */
	int m_variant;                /* Variant of the 5xxx - see tms5110r.h */

	/* coefficient tables */
	const struct tms5100_coeffs *m_coeff;

	/* these contain data that describes the 128-bit data FIFO */
	UINT8 m_fifo[FIFO_SIZE];
	UINT8 m_fifo_head;
	UINT8 m_fifo_tail;
	UINT8 m_fifo_count;
	UINT8 m_fifo_bits_taken;


	/* these contain global status bits */
	UINT8 m_previous_TALK_STATUS;      /* this is the OLD value of TALK_STATUS (i.e. previous value of m_SPEN|m_TALKD), needed for generating interrupts on a falling TALK_STATUS edge */
	UINT8 m_SPEN;             /* set on speak(or speak external and BL falling edge) command, cleared on stop command, reset command, or buffer out */
	UINT8 m_DDIS;             /* If 1, DDIS is 1, i.e. Speak External command in progress, writes go to FIFO. */
	UINT8 m_TALK;             /* set on SPEN & RESETL4(pc12->pc0 transition), cleared on stop command or reset command */
#define TALK_STATUS (m_SPEN|m_TALKD)
	UINT8 m_TALKD;            /* TALK(TCON) value, latched every RESETL4 */
	UINT8 m_buffer_low;       /* If 1, FIFO has less than 8 bytes in it */
	UINT8 m_buffer_empty;     /* If 1, FIFO is empty */
	UINT8 m_irq_pin;          /* state of the IRQ pin (output) */
	UINT8 m_ready_pin;        /* state of the READY pin (output) */

	/* these contain data describing the current and previous voice frames */
#define OLD_FRAME_SILENCE_FLAG m_OLDE // 1 if E=0, 0 otherwise.
#define OLD_FRAME_UNVOICED_FLAG m_OLDP // 1 if P=0 (unvoiced), 0 if voiced
	UINT8 m_OLDE;
	UINT8 m_OLDP;

#define NEW_FRAME_STOP_FLAG (m_new_frame_energy_idx == 0xF) // 1 if this is a stop (Energy = 0xF) frame
#define NEW_FRAME_SILENCE_FLAG (m_new_frame_energy_idx == 0) // ditto as above
#define NEW_FRAME_UNVOICED_FLAG (m_new_frame_pitch_idx == 0) // ditto as above
	UINT8 m_new_frame_energy_idx;
	UINT8 m_new_frame_pitch_idx;
	UINT8 m_new_frame_k_idx[10];


	/* these are all used to contain the current state of the sound generation */
#ifndef PERFECT_INTERPOLATION_HACK
	INT16 m_current_energy;
	INT16 m_current_pitch;
	INT16 m_current_k[10];
#else
	UINT8 m_old_frame_energy_idx;
	UINT8 m_old_frame_pitch_idx;
	UINT8 m_old_frame_k_idx[10];
	UINT8 m_old_zpar;
	UINT8 m_old_uv_zpar;

	INT32 m_current_energy;
	INT32 m_current_pitch;
	INT32 m_current_k[10];
#endif

	UINT16 m_previous_energy; /* needed for lattice filter to match patent */

	UINT8 m_subcycle;         /* contains the current subcycle for a given PC: 0 is A' (only used on SPKSLOW mode on 51xx), 1 is A, 2 is B */
	UINT8 m_subc_reload;      /* contains 1 for normal speech, 0 when SPKSLOW is active */
	UINT8 m_PC;               /* current parameter counter (what param is being interpolated), ranges from 0 to 12 */
	/* NOTE: the interpolation period counts 1,2,3,4,5,6,7,0 for divide by 8,8,8,4,4,2,2,1 */
	UINT8 m_IP;               /* the current interpolation period */
	UINT8 m_inhibit;          /* If 1, interpolation is inhibited until the DIV1 period */
	UINT8 m_uv_zpar;          /* If 1, zero k5 thru k10 coefficients */
	UINT8 m_zpar;             /* If 1, zero ALL parameters. */
	UINT8 m_pitch_zero;       /* circuit 412; pitch is forced to zero under certain circumstances */
	UINT8 m_c_variant_rate;    /* only relevant for tms5220C's multi frame rate feature; is the actual 4 bit value written on a 0x2* or 0x0* command */
	UINT16 m_pitch_count;     /* pitch counter; provides chirp rom address */

	INT32 m_u[11];
	INT32 m_x[10];

	UINT16 m_RNG;             /* the random noise generator configuration is: 1 + x + x^3 + x^4 + x^13 TODO: no it isn't */
	INT16 m_excitation_data;

	/* R Nabet : These have been added to emulate speech Roms */
	UINT8 m_schedule_dummy_read;          /* set after each load address, so that next read operation is preceded by a dummy read */
	UINT8 m_data_register;                /* data register, used by read command */
	UINT8 m_RDB_flag;                 /* whether we should read data register or status register */

	/* The TMS52xx has two different ways of providing output data: the
	   analog speaker pin (which was usually used) and the Digital I/O pin.
	   The internal DAC used to feed the analog pin is only 8 bits, and has the
	   funny clipping/clamping logic, while the digital pin gives full 10 bit
	   resolution of the output data.
	   TODO: add a way to set/reset this other than the FORCE_DIGITAL define
	 */
	UINT8 m_digital_select;

	/* io_ready: page 3 of the datasheet specifies that READY will be asserted until
	 * data is available or processed by the system.
	 */
	UINT8 m_io_ready;

	/* flag for "true" timing involving rs/ws */
	UINT8 m_true_timing;

	/* rsws - state, rs bit 1, ws bit 0 */
	UINT8 m_rs_ws;
	UINT8 m_read_latch;
	UINT8 m_write_latch;

	sound_stream *m_stream;
	int m_clock;
	emu_timer *m_timer_io_ready;

	/* callbacks */
	devcb_write_line m_irq_handler;
	devcb_write_line m_readyq_handler;
	const char *m_speechrom_tag;
	speechrom_device *m_speechrom;
};

extern const device_type TMS5220;

class tms5220c_device : public tms5220_device
{
public:
	tms5220c_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type TMS5220C;

class cd2501e_device : public tms5220_device
{
public:
	cd2501e_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type CD2501E;

class tms5200_device : public tms5220_device
{
public:
	tms5200_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type TMS5200;

class cd2501ecd_device : public tms5220_device
{
public:
	cd2501ecd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type CD2501ECD;

#endif
