// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo, Aaron Giles, Jonathan Gevaryahu, Raphael Nabet, Couriersud, Michael Zapf
#ifndef MAME_SOUND_TMS5220_H
#define MAME_SOUND_TMS5220_H

#pragma once

#include "machine/spchrom.h"

/* HACK: if defined, uses impossibly perfect 'straight line' interpolation */
#undef TMS5220_PERFECT_INTERPOLATION_HACK

/* clock rate = 80 * output sample rate,     */
/* usually 640000 for 8000 Hz sample rate or */
/* usually 800000 for 10000 Hz sample rate.  */

/* IRQ callback function, active low, i.e. state=0 */
#define MCFG_TMS52XX_IRQ_HANDLER(_devcb) \
	devcb = &downcast<tms5220_device &>(*device).set_irq_handler(DEVCB_##_devcb);

/* Ready callback function, active low, i.e. state=0 */
#define MCFG_TMS52XX_READYQ_HANDLER(_devcb) \
	devcb = &downcast<tms5220_device &>(*device).set_readyq_handler(DEVCB_##_devcb);

/* old VSM handler, remove me! */
#define MCFG_TMS52XX_SPEECHROM(_tag) \
	downcast<tms5220_device &>(*device).set_speechrom_tag(_tag);

/* new VSM handler */
#define MCFG_TMS52XX_M0_CB(_devcb) \
	devcb = &downcast<tms5220_device &>(*device).set_m0_callback(DEVCB_##_devcb);

#define MCFG_TMS52XX_M1_CB(_devcb) \
	devcb = &downcast<tms5220_device &>(*device).set_m1_callback(DEVCB_##_devcb);

#define MCFG_TMS52XX_ADDR_CB(_devcb) \
	devcb = &downcast<tms5220_device &>(*device).set_addr_callback(DEVCB_##_devcb);

#define MCFG_TMS52XX_DATA_CB(_devcb) \
	devcb = &downcast<tms5220_device &>(*device).set_data_callback(DEVCB_##_devcb);

#define MCFG_TMS52XX_ROMCLK_CB(_devcb) \
	devcb = &downcast<tms5220_device &>(device).set_romclk_callback(DEVCB_##_devcb);

class tms5220_device : public device_t, public device_sound_interface
{
public:
	enum
	{
		RS=2,
		WS=1
	};

	tms5220_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	template <class Object> devcb_base &set_irq_handler(Object &&cb) { return m_irq_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_readyq_handler(Object &&cb) { return m_readyq_handler.set_callback(std::forward<Object>(cb)); }
	// old VSM support, remove me!
	void set_speechrom_tag(const char *_tag) { m_speechrom_tag = _tag; }
	// new VSM support
	template <class Object> devcb_base &set_m0_callback(Object &&cb) { return m_m0_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_m1_callback(Object &&cb) { return m_m1_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_addr_callback(Object &&cb) { return m_addr_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_data_callback(Object &&cb) { return m_data_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_romclk_callback(Object &&cb) { return m_romclk_cb.set_callback(std::forward<Object>(cb)); }

	// Control lines - once written to will switch interface into * "true" timing behaviour.

	// all lines with suffix q are active low!

	WRITE_LINE_MEMBER( rsq_w );
	WRITE_LINE_MEMBER( wsq_w );

	DECLARE_WRITE8_MEMBER( combined_rsq_wsq_w );
	/* this combined_rsq_wsq_w hack is necessary for specific systems such as
	the TI 99/8 since the 5220c and cd2501ecd do specific things if both lines
	go active or inactive at slightly different times by separate write_line
	writes, which causes the chip to incorrectly reset itself on the 99/8,
	where the writes are supposed to happen simultaneously;
	/RS is bit 1, /WS is bit 0
	Note this is a hack and probably can be removed later, once the 'real'
	line handlers above defer by at least 4 clock cycles before taking effect */
	DECLARE_WRITE8_MEMBER( data_w );
	DECLARE_READ8_MEMBER( status_r );

	void data_w(uint8_t data);
	uint8_t status_r();

	READ_LINE_MEMBER( readyq_r );
	READ_LINE_MEMBER( intq_r );

	attotime time_to_ready();

protected:
	tms5220_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int variant);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_clock_changed() override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	static constexpr unsigned FIFO_SIZE = 16;

	// 51xx and VSM related
	void new_int_write(uint8_t rc, uint8_t m0, uint8_t m1, uint8_t addr);
	void new_int_write_addr(uint8_t addr);
	uint8_t new_int_read();
	void perform_dummy_read();
	// 52xx or common
	void register_for_save_states();
	void data_write(int data);
	void update_fifo_status_and_ints();
	int extract_bits(int count);
	int status_read();
	int ready_read();
	int cycles_to_ready();
	int int_read();
	void process(int16_t *buffer, unsigned int size);
	int32_t lattice_filter();
	void process_command(unsigned char cmd);
	void parse_frame();
	void set_interrupt_state(int state);
	void update_ready_state();

	uint8_t TALK_STATUS() const { return m_SPEN | m_TALKD; }
	uint8_t &OLD_FRAME_SILENCE_FLAG() { return m_OLDE; } // 1 if E=0, 0 otherwise.
	uint8_t &OLD_FRAME_UNVOICED_FLAG() { return m_OLDP; } // 1 if P=0 (unvoiced), 0 if voiced
	bool NEW_FRAME_STOP_FLAG() const { return m_new_frame_energy_idx == 0x0F; } // 1 if this is a stop (Energy = 0xF) frame
	bool NEW_FRAME_SILENCE_FLAG() const { return m_new_frame_energy_idx == 0; } // ditto as above
	bool NEW_FRAME_UNVOICED_FLAG() const { return m_new_frame_pitch_idx == 0; } // ditto as above

	// internal state

	/* coefficient tables */
	const int m_variant;                /* Variant of the 5xxx - see tms5110r.h */

	/* coefficient tables */
	const struct tms5100_coeffs *m_coeff;

	/* these contain global status bits */
	uint8_t m_PDC;
	uint8_t m_CTL_pins;
	uint8_t m_state;

	/* New VSM interface */
	uint32_t m_address;
	uint8_t  m_next_is_address;
	uint8_t  m_schedule_dummy_read;
	uint8_t  m_addr_bit;
	/* read byte */
	uint8_t  m_CTL_buffer;

	/* Old VSM interface; R Nabet : These have been added to emulate speech Roms */
	//uint8_t m_schedule_dummy_read;          /* set after each load address, so that next read operation is preceded by a dummy read */
	uint8_t m_data_register;                /* data register, used by read command */
	uint8_t m_RDB_flag;                 /* whether we should read data register or status register */

	/* these contain data that describes the 128-bit data FIFO */
	uint8_t m_fifo[FIFO_SIZE];
	uint8_t m_fifo_head;
	uint8_t m_fifo_tail;
	uint8_t m_fifo_count;
	uint8_t m_fifo_bits_taken;


	/* these contain global status bits */
	uint8_t m_previous_TALK_STATUS;      /* this is the OLD value of TALK_STATUS (i.e. previous value of m_SPEN|m_TALKD), needed for generating interrupts on a falling TALK_STATUS edge */
	uint8_t m_SPEN;             /* set on speak(or speak external and BL falling edge) command, cleared on stop command, reset command, or buffer out */
	uint8_t m_DDIS;             /* If 1, DDIS is 1, i.e. Speak External command in progress, writes go to FIFO. */
	uint8_t m_TALK;             /* set on SPEN & RESETL4(pc12->pc0 transition), cleared on stop command or reset command */
	uint8_t m_TALKD;            /* TALK(TCON) value, latched every RESETL4 */
	uint8_t m_buffer_low;       /* If 1, FIFO has less than 8 bytes in it */
	uint8_t m_buffer_empty;     /* If 1, FIFO is empty */
	uint8_t m_irq_pin;          /* state of the IRQ pin (output) */
	uint8_t m_ready_pin;        /* state of the READY pin (output) */

	/* these contain data describing the current and previous voice frames */
	uint8_t m_OLDE;
	uint8_t m_OLDP;

	uint8_t m_new_frame_energy_idx;
	uint8_t m_new_frame_pitch_idx;
	uint8_t m_new_frame_k_idx[10];


	/* these are all used to contain the current state of the sound generation */
#ifndef TMS5220_PERFECT_INTERPOLATION_HACK
	int16_t m_current_energy;
	int16_t m_current_pitch;
	int16_t m_current_k[10];
#else
	uint8_t m_old_frame_energy_idx;
	uint8_t m_old_frame_pitch_idx;
	uint8_t m_old_frame_k_idx[10];
	uint8_t m_old_zpar;
	uint8_t m_old_uv_zpar;

	int32_t m_current_energy;
	int32_t m_current_pitch;
	int32_t m_current_k[10];
#endif

	uint16_t m_previous_energy; /* needed for lattice filter to match patent */

	uint8_t m_subcycle;         /* contains the current subcycle for a given PC: 0 is A' (only used on SPKSLOW mode on 51xx), 1 is A, 2 is B */
	uint8_t m_subc_reload;      /* contains 1 for normal speech, 0 when SPKSLOW is active */
	uint8_t m_PC;               /* current parameter counter (what param is being interpolated), ranges from 0 to 12 */
	/* NOTE: the interpolation period counts 1,2,3,4,5,6,7,0 for divide by 8,8,8,4,4,2,2,1 */
	uint8_t m_IP;               /* the current interpolation period */
	uint8_t m_inhibit;          /* If 1, interpolation is inhibited until the DIV1 period */
	uint8_t m_uv_zpar;          /* If 1, zero k5 thru k10 coefficients */
	uint8_t m_zpar;             /* If 1, zero ALL parameters. */
	uint8_t m_pitch_zero;       /* circuit 412; pitch is forced to zero under certain circumstances */
	uint8_t m_c_variant_rate;    /* only relevant for tms5220C's multi frame rate feature; is the actual 4 bit value written on a 0x2* or 0x0* command */
	uint16_t m_pitch_count;     /* pitch counter; provides chirp rom address */

	int32_t m_u[11];
	int32_t m_x[10];

	uint16_t m_RNG;             /* the random noise generator configuration is: 1 + x + x^3 + x^4 + x^13 TODO: no it isn't */
	int16_t m_excitation_data;

	/* The TMS52xx has two different ways of providing output data: the
	   analog speaker pin (which was usually used) and the Digital I/O pin.
	   The internal DAC used to feed the analog pin is only 8 bits, and has the
	   funny clipping/clamping logic, while the digital pin gives full 10 bit
	   resolution of the output data.
	   TODO: add a way to set/reset this other than the FORCE_DIGITAL define
	 */
	uint8_t m_digital_select;

	/* io_ready: page 3 of the datasheet specifies that READY will be asserted until
	 * data is available or processed by the system.
	 */
	uint8_t m_io_ready;

	/* flag for "true" timing involving rs/ws */
	uint8_t m_true_timing;

	/* rsws - state, rs bit 1, ws bit 0 */
	uint8_t m_rs_ws;
	uint8_t m_read_latch;
	uint8_t m_write_latch;

	sound_stream *m_stream;
	emu_timer *m_timer_io_ready;

	/* callbacks */
	devcb_write_line m_irq_handler;
	devcb_write_line m_readyq_handler;
	// next 2 lines are old speechrom handler, remove me!
	const char *m_speechrom_tag;
	speechrom_device *m_speechrom;
	// next lines are new speechrom handler
	devcb_write_line   m_m0_cb;      // the M0 line
	devcb_write_line   m_m1_cb;      // the M1 line
	devcb_write8       m_addr_cb;    // Write to ADD1,2,4,8 - 4 address bits
	devcb_read_line    m_data_cb;    // Read one bit from ADD8/Data - voice data
	// On a real chip rom_clk is running all the time
	// Here, we only use it to properly emulate the protocol.
	// Do not rely on it to be a timed signal.
	devcb_write_line   m_romclk_cb;  // rom clock - Only used to drive the data lines
};


class tms5220c_device : public tms5220_device
{
public:
	tms5220c_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class cd2501e_device : public tms5220_device
{
public:
	cd2501e_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class tms5200_device : public tms5220_device
{
public:
	tms5200_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class cd2501ecd_device : public tms5220_device
{
public:
	cd2501ecd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


DECLARE_DEVICE_TYPE(TMS5220,   tms5220_device)
DECLARE_DEVICE_TYPE(TMS5220C,  tms5220c_device)
DECLARE_DEVICE_TYPE(CD2501E,   cd2501e_device)
DECLARE_DEVICE_TYPE(TMS5200,   tms5200_device)
DECLARE_DEVICE_TYPE(CD2501ECD, cd2501ecd_device)

#endif // MAME_SOUND_TMS5220_H
