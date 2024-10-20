// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo, Jarek Burczynski, Aaron Giles, Jonathan Gevaryahu, Couriersud
#ifndef MAME_SOUND_TMS5110_H
#define MAME_SOUND_TMS5110_H

#pragma once


/* HACK: if defined, uses impossibly perfect 'straight line' interpolation */
#undef TMS5110_PERFECT_INTERPOLATION_HACK

/* clock rate = 80 * output sample rate,     */
/* usually 640000 for 8000 Hz sample rate or */
/* usually 800000 for 10000 Hz sample rate.  */

class tms5110_device : public device_t, public device_sound_interface
{
public:
	/* TMS5110 commands */
													/* CTL8  CTL4  CTL2  CTL1  |   PDC's  */
													/* (MSB)             (LSB) | required */
	static constexpr uint8_t CMD_RESET        = 0;  /*    0     0     0     x  |     1    */
	static constexpr uint8_t CMD_LOAD_ADDRESS = 2;  /*    0     0     1     x  |     2    */
	static constexpr uint8_t CMD_OUTPUT       = 4;  /*    0     1     0     x  |     3    */
	static constexpr uint8_t CMD_SPKSLOW      = 6;  /*    0     1     1     x  |     1    */
	/* Note: TMS5110_CMD_SPKSLOW is undocumented on the datasheets, it only appears
	   on the patents. It might not actually work properly on some of the real
	   chips as manufactured. Acts the same as CMD_SPEAK, but makes the
	   interpolator take two A cycles wherever it would normally only take one,
	   effectively making speech of any given word take 1.5x as long as normal. */
	static constexpr uint8_t CMD_READ_BIT    =  8;  /*    1     0     0     x  |     1    */
	static constexpr uint8_t CMD_SPEAK       = 10;  /*    1     0     1     x  |     1    */
	static constexpr uint8_t CMD_READ_BRANCH = 12;  /*    1     1     0     x  |     1    */
	static constexpr uint8_t CMD_TEST_TALK   = 14;  /*    1     1     1     x  |     3    */

	tms5110_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto m0() { return m_m0_cb.bind(); }
	auto m1() { return m_m1_cb.bind(); }
	auto addr() { return m_addr_cb.bind(); }
	auto data() { return m_data_cb.bind(); }
	auto romclk() { return m_romclk_cb.bind(); }

	void ctl_w(uint8_t data);
	uint8_t ctl_r();
	void pdc_w(int state);

	// this is only used by cvs.cpp
	// it is not related at all to the speech generation and conflicts with the new ROM controller interface.
	int romclk_hack_r();

protected:
	tms5110_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int variant);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	TIMER_CALLBACK_MEMBER(romclk_hack_toggle);

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	uint8_t TALK_STATUS() const { return m_SPEN | m_TALKD; }

	uint8_t m_SPEN;             /* set on speak command, cleared on stop command or reset command */
	uint8_t m_TALK;             /* set on SPEN & RESETL4(pc12->pc0 transition), cleared on stop command or reset command */
	uint8_t m_TALKD;            /* TALK(TCON) value, latched every RESETL4 */
	sound_stream *m_stream;

private:
	void new_int_write(uint8_t rc, uint8_t m0, uint8_t m1, uint8_t addr);
	void new_int_write_addr(uint8_t addr);
	uint8_t new_int_read();
	void register_for_save_states();
	int read_bits(int count);
	void perform_dummy_read();
	int32_t lattice_filter();
	void process(int16_t *buffer, unsigned int size);
	void PDC_set(int data);
	void parse_frame();

	uint8_t &OLD_FRAME_SILENCE_FLAG() { return m_OLDE; } // 1 if E=0, 0 otherwise.
	uint8_t &OLD_FRAME_UNVOICED_FLAG() { return m_OLDP; } // 1 if P=0 (unvoiced), 0 if voiced

	bool NEW_FRAME_STOP_FLAG() const { return m_new_frame_energy_idx == 0x0F; } // 1 if this is a stop (Energy = 0x0F) frame
	bool NEW_FRAME_SILENCE_FLAG() const { return m_new_frame_energy_idx == 0; } // ditto as above
	bool NEW_FRAME_UNVOICED_FLAG() const { return m_new_frame_pitch_idx == 0; } // ditto as above

	// internal state
	/* table */
	optional_region_ptr<uint8_t> m_table;

	/* coefficient tables */
	const int m_variant;                /* Variant of the 5110 - see tms5110.h */

	/* coefficient tables */
	const struct tms5100_coeffs *m_coeff;

	/* these contain global status bits */
	uint8_t m_PDC;
	uint8_t m_CTL_pins;
	uint8_t m_state;

	/* Rom interface */
	uint32_t m_address;
	uint8_t  m_next_is_address;
	uint8_t  m_schedule_dummy_read;
	uint8_t  m_addr_bit;
	/* read byte */
	uint8_t  m_CTL_buffer;

	/* callbacks */
	devcb_write_line   m_m0_cb;      // the M0 line
	devcb_write_line   m_m1_cb;      // the M1 line
	devcb_write8       m_addr_cb;    // Write to ADD1,2,4,8 - 4 address bits
	devcb_read_line    m_data_cb;    // Read one bit from ADD8/Data - voice data
	// On a real chip rom_clk is running all the time
	// Here, we only use it to properly emulate the protocol.
	// Do not rely on it to be a timed signal.
	devcb_write_line   m_romclk_cb;  // rom clock - Only used to drive the data lines

	/* these contain data describing the current and previous voice frames */
	uint8_t m_OLDE;
	uint8_t m_OLDP;

	uint8_t m_new_frame_energy_idx;
	uint8_t m_new_frame_pitch_idx;
	uint8_t m_new_frame_k_idx[10];


	/* these are all used to contain the current state of the sound generation */
#ifndef TMS5110_PERFECT_INTERPOLATION_HACK
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
	uint16_t m_pitch_count;     /* pitch counter; provides chirp rom address */

	int32_t m_u[11];
	int32_t m_x[10];

	uint16_t m_RNG;             /* the random noise generator configuration is: 1 + x + x^3 + x^4 + x^13 TODO: no it isn't */
	int16_t m_excitation_data;

	/* The TMS51xx has two different ways of providing output data: the
	   analog speaker pins (which were usually used) and the Digital I/O pin.
	   The internal DAC used to feed the analog pins is only 8 bits, and has the
	   funny clipping/clamping logic, while the digital pin gives full 10 bit
	   resolution of the output data.
	   TODO: add a way to set/reset this other than the FORCE_DIGITAL define
	 */
	uint8_t m_digital_select;

	int32_t m_speech_rom_bitnum;

	uint8_t m_romclk_hack_timer_started;
	uint8_t m_romclk_hack_state;

	emu_timer *m_romclk_hack_timer;
};


class tms5100_device : public tms5110_device
{
public:
	tms5100_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class tmc0281_device : public tms5110_device
{
public:
	tmc0281_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class tms5100a_device : public tms5110_device
{
public:
	tms5100a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class tmc0281d_device : public tms5110_device
{
public:
	tmc0281d_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class cd2801_device : public tms5110_device
{
public:
	cd2801_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class cd2802_device : public tms5110_device
{
public:
	cd2802_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class tms5110a_device : public tms5110_device
{
public:
	tms5110a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class m58817_device : public tms5110_device
{
public:
	m58817_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t status_r();
};


DECLARE_DEVICE_TYPE(TMS5110,  tms5110_device)
DECLARE_DEVICE_TYPE(TMS5100,  tms5100_device)
DECLARE_DEVICE_TYPE(TMC0281,  tmc0281_device)
DECLARE_DEVICE_TYPE(TMS5100A, tms5100a_device)
DECLARE_DEVICE_TYPE(TMC0281D, tmc0281d_device)
DECLARE_DEVICE_TYPE(CD2801,   cd2801_device)
DECLARE_DEVICE_TYPE(CD2802,   cd2802_device)
DECLARE_DEVICE_TYPE(TMS5110A, tms5110a_device)
DECLARE_DEVICE_TYPE(M58817,   m58817_device)



/* PROM controlled TMS5110 interface */

class tmsprom_device : public device_t
{
public:
	tmsprom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_region(const char *region) { m_prom.set_tag(region); }
	void set_rom_size(uint32_t rom_size) { m_rom_size = rom_size; }
	void set_pdc_bit(uint8_t pdc_bit) { m_pdc_bit = pdc_bit; }
	void set_ctl1_bit(uint8_t ctl1_bit) { m_ctl1_bit = ctl1_bit; }
	void set_ctl2_bit(uint8_t ctl2_bit) { m_ctl2_bit = ctl2_bit; }
	void set_ctl4_bit(uint8_t ctl4_bit) { m_ctl4_bit = ctl4_bit; }
	void set_ctl8_bit(uint8_t ctl8_bit) { m_ctl8_bit = ctl8_bit; }
	void set_reset_bit(uint8_t reset_bit) { m_reset_bit = reset_bit; }
	void set_stop_bit(uint8_t stop_bit) { m_stop_bit = stop_bit; }
	auto pdc() { return m_pdc_cb.bind(); }
	auto ctl() { return m_ctl_cb.bind(); }

	void m0_w(int state);
	int data_r();

	/* offset is rom # */
	void rom_csq_w(offs_t offset, uint8_t data);
	void bit_w(uint8_t data);
	void enable_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_romclk);

private:
	void register_for_save_states();
	void update_prom_cnt();

	// internal state
	/* Rom interface */
	uint32_t m_address;
	/* ctl lines */
	uint8_t  m_m0;
	uint8_t  m_enable;
	uint32_t  m_base_address;
	uint8_t  m_bit;

	int m_prom_cnt;

	required_region_ptr<uint8_t> m_rom;
	required_region_ptr<uint8_t> m_prom;
	uint32_t m_rom_size;                /* individual rom_size */
	uint8_t m_pdc_bit;                  /* bit # of pdc line */
	/* virtual bit 8: constant 0, virtual bit 9:constant 1 */
	uint8_t m_ctl1_bit;                 /* bit # of ctl1 line */
	uint8_t m_ctl2_bit;                 /* bit # of ctl2 line */
	uint8_t m_ctl4_bit;                 /* bit # of ctl4 line */
	uint8_t m_ctl8_bit;                 /* bit # of ctl8 line */
	uint8_t m_reset_bit;                /* bit # of rom reset */
	uint8_t m_stop_bit;                 /* bit # of stop */
	devcb_write_line m_pdc_cb;      /* tms pdc func */
	devcb_write8 m_ctl_cb;          /* tms ctl func */

	emu_timer *m_romclk_timer;
};

DECLARE_DEVICE_TYPE(TMSPROM, tmsprom_device)

#endif // MAME_SOUND_TMS5110_H
