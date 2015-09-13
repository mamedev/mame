// license:???
// copyright-holders:Frank Palazzolo, Jarek Burczynski, Aaron Giles, Jonathan Gevaryahu, Couriersud
#pragma once

#ifndef __TMS5110_H__
#define __TMS5110_H__

#include "emu.h"

/* HACK: if defined, uses impossibly perfect 'straight line' interpolation */
#undef PERFECT_INTERPOLATION_HACK

/* TMS5110 commands */
										/* CTL8  CTL4  CTL2  CTL1  |   PDC's  */
										/* (MSB)             (LSB) | required */
#define TMS5110_CMD_RESET        (0) /*    0     0     0     x  |     1    */
#define TMS5110_CMD_LOAD_ADDRESS (2) /*    0     0     1     x  |     2    */
#define TMS5110_CMD_OUTPUT       (4) /*    0     1     0     x  |     3    */
#define TMS5110_CMD_SPKSLOW      (6) /*    0     1     1     x  |     1    | Note: this command is undocumented on the datasheets, it only appears on the patents. It might not actually work properly on some of the real chips as manufactured. Acts the same as CMD_SPEAK, but makes the interpolator take three A cycles whereever it would normally only take one, effectively making speech of any given word take twice as long as normal. */
#define TMS5110_CMD_READ_BIT     (8) /*    1     0     0     x  |     1    */
#define TMS5110_CMD_SPEAK       (10) /*    1     0     1     x  |     1    */
#define TMS5110_CMD_READ_BRANCH (12) /*    1     1     0     x  |     1    */
#define TMS5110_CMD_TEST_TALK   (14) /*    1     1     1     x  |     3    */

/* clock rate = 80 * output sample rate,     */
/* usually 640000 for 8000 Hz sample rate or */
/* usually 800000 for 10000 Hz sample rate.  */

#define MCFG_TMS5110_M0_CB(_devcb) \
	devcb = &tms5110_device::set_m0_callback(*device, DEVCB_##_devcb);

#define MCFG_TMS5110_M1_CB(_devcb) \
	devcb = &tms5110_device::set_m1_callback(*device, DEVCB_##_devcb);

#define MCFG_TMS5110_ADDR_CB(_devcb) \
	devcb = &tms5110_device::set_addr_callback(*device, DEVCB_##_devcb);

#define MCFG_TMS5110_DATA_CB(_devcb) \
	devcb = &tms5110_device::set_data_callback(*device, DEVCB_##_devcb);

#define MCFG_TMS5110_ROMCLK_CB(_devcb) \
	devcb = &tms5110_device::set_romclk_callback(*device, DEVCB_##_devcb);


class tms5110_device : public device_t,
						public device_sound_interface
{
public:
	tms5110_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tms5110_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	template<class _Object> static devcb_base &set_m0_callback(device_t &device, _Object object) { return downcast<tms5110_device &>(device).m_m0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_m1_callback(device_t &device, _Object object) { return downcast<tms5110_device &>(device).m_m1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_addr_callback(device_t &device, _Object object) { return downcast<tms5110_device &>(device).m_addr_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_data_callback(device_t &device, _Object object) { return downcast<tms5110_device &>(device).m_data_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_romclk_callback(device_t &device, _Object object) { return downcast<tms5110_device &>(device).m_romclk_cb.set_callback(object); }

	DECLARE_WRITE8_MEMBER( ctl_w );
	DECLARE_READ8_MEMBER( ctl_r );
	DECLARE_WRITE_LINE_MEMBER( pdc_w );

	/* this is only used by cvs.c
	 * it is not related at all to the speech generation
	 * and conflicts with the new rom controller interface.
	 */
	DECLARE_READ8_MEMBER( romclk_hack_r );

	void set_frequency(int frequency);

	int _speech_rom_read_bit();
	void _speech_rom_set_addr(int addr);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

	void set_variant(int variant);

	UINT8 m_SPEN;             /* set on speak command, cleared on stop command or reset command */
	UINT8 m_TALK;             /* set on SPEN & RESETL4(pc12->pc0 transition), cleared on stop command or reset command */
#define TALK_STATUS (m_SPEN|m_TALKD)
	UINT8 m_TALKD;            /* TALK(TCON) value, latched every RESETL4 */
	sound_stream *m_stream;

private:
	void new_int_write(UINT8 rc, UINT8 m0, UINT8 m1, UINT8 addr);
	void new_int_write_addr(UINT8 addr);
	UINT8 new_int_read();
	void register_for_save_states();
	int extract_bits(int count);
	void perform_dummy_read();
	INT32 lattice_filter();
	void process(INT16 *buffer, unsigned int size);
	void PDC_set(int data);
	void parse_frame();

	// internal state
	/* coefficient tables */
	int m_variant;                /* Variant of the 5110 - see tms5110.h */

	/* coefficient tables */
	const struct tms5100_coeffs *m_coeff;

	/* these contain global status bits */
	UINT8 m_PDC;
	UINT8 m_CTL_pins;
	UINT8 m_state;

	/* Rom interface */
	UINT32 m_address;
	UINT8  m_next_is_address;
	UINT8  m_schedule_dummy_read;
	UINT8  m_addr_bit;
	/* read byte */
	UINT8  m_CTL_buffer;

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
	UINT16 m_pitch_count;     /* pitch counter; provides chirp rom address */

	INT32 m_u[11];
	INT32 m_x[10];

	UINT16 m_RNG;             /* the random noise generator configuration is: 1 + x + x^3 + x^4 + x^13 TODO: no it isn't */
	INT16 m_excitation_data;

	/* The TMS51xx has two different ways of providing output data: the
	   analog speaker pins (which were usually used) and the Digital I/O pin.
	   The internal DAC used to feed the analog pins is only 8 bits, and has the
	   funny clipping/clamping logic, while the digital pin gives full 10 bit
	   resolution of the output data.
	   TODO: add a way to set/reset this other than the FORCE_DIGITAL define
	 */
	UINT8 m_digital_select;

	INT32 m_speech_rom_bitnum;

	UINT8 m_romclk_hack_timer_started;
	UINT8 m_romclk_hack_state;

	emu_timer *m_romclk_hack_timer;
	const UINT8 *m_table;
};

extern const device_type TMS5110;

class tms5100_device : public tms5110_device
{
public:
	tms5100_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type TMS5100;

class tmc0281_device : public tms5110_device
{
public:
	tmc0281_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type TMC0281;

class tms5100a_device : public tms5110_device
{
public:
	tms5100a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type TMS5100A;

class tmc0281d_device : public tms5110_device
{
public:
	tmc0281d_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type TMC0281D;

class cd2801_device : public tms5110_device
{
public:
	cd2801_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type CD2801;

class cd2802_device : public tms5110_device
{
public:
	cd2802_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type CD2802;

class tms5110a_device : public tms5110_device
{
public:
	tms5110a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type TMS5110A;

class m58817_device : public tms5110_device
{
public:
	m58817_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( status_r );

protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type M58817;



/* PROM controlled TMS5110 interface */

class tmsprom_device : public device_t
{
public:
	tmsprom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void set_region(device_t &device, const char *region) { downcast<tmsprom_device &>(device).m_prom_region = region; }
	static void set_rom_size(device_t &device, UINT32 rom_size) { downcast<tmsprom_device &>(device).m_rom_size = rom_size; }
	static void set_pdc_bit(device_t &device, UINT8 pdc_bit) { downcast<tmsprom_device &>(device).m_pdc_bit = pdc_bit; }
	static void set_ctl1_bit(device_t &device, UINT8 ctl1_bit) { downcast<tmsprom_device &>(device).m_ctl1_bit = ctl1_bit; }
	static void set_ctl2_bit(device_t &device, UINT8 ctl2_bit) { downcast<tmsprom_device &>(device).m_ctl2_bit = ctl2_bit; }
	static void set_ctl4_bit(device_t &device, UINT8 ctl4_bit) { downcast<tmsprom_device &>(device).m_ctl4_bit = ctl4_bit; }
	static void set_ctl8_bit(device_t &device, UINT8 ctl8_bit) { downcast<tmsprom_device &>(device).m_ctl8_bit = ctl8_bit; }
	static void set_reset_bit(device_t &device, UINT8 reset_bit) { downcast<tmsprom_device &>(device).m_reset_bit = reset_bit; }
	static void set_stop_bit(device_t &device, UINT8 stop_bit) { downcast<tmsprom_device &>(device).m_stop_bit = stop_bit; }
	template<class _Object> static devcb_base &set_pdc_callback(device_t &device, _Object object) { return downcast<tmsprom_device &>(device).m_pdc_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_ctl_callback(device_t &device, _Object object) { return downcast<tmsprom_device &>(device).m_ctl_cb.set_callback(object); }

	DECLARE_WRITE_LINE_MEMBER( m0_w );
	DECLARE_READ_LINE_MEMBER( data_r );

	/* offset is rom # */
	DECLARE_WRITE8_MEMBER( rom_csq_w );
	DECLARE_WRITE8_MEMBER( bit_w );
	DECLARE_WRITE_LINE_MEMBER( enable_w );

protected:
	// device-level overrides
	virtual void device_start();

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	void register_for_save_states();
	void update_prom_cnt();

	// internal state
	/* Rom interface */
	UINT32 m_address;
	/* ctl lines */
	UINT8  m_m0;
	UINT8  m_enable;
	UINT32  m_base_address;
	UINT8  m_bit;

	int m_prom_cnt;

	const char *m_prom_region;        /* prom memory region - sound region is automatically assigned */
	UINT32 m_rom_size;                /* individual rom_size */
	UINT8 m_pdc_bit;                  /* bit # of pdc line */
	/* virtual bit 8: constant 0, virtual bit 9:constant 1 */
	UINT8 m_ctl1_bit;                 /* bit # of ctl1 line */
	UINT8 m_ctl2_bit;                 /* bit # of ctl2 line */
	UINT8 m_ctl4_bit;                 /* bit # of ctl4 line */
	UINT8 m_ctl8_bit;                 /* bit # of ctl8 line */
	UINT8 m_reset_bit;                /* bit # of rom reset */
	UINT8 m_stop_bit;                 /* bit # of stop */
	devcb_write_line m_pdc_cb;      /* tms pdc func */
	devcb_write8 m_ctl_cb;          /* tms ctl func */

	emu_timer *m_romclk_timer;
	const UINT8 *m_rom;
	const UINT8 *m_prom;
};

extern const device_type TMSPROM;

#define MCFG_TMSPROM_REGION(_region) \
	tmsprom_device::set_region(*device, _region);

#define MCFG_TMSPROM_ROM_SIZE(_size) \
	tmsprom_device::set_rom_size(*device, _size);

#define MCFG_TMSPROM_PDC_BIT(_bit) \
	tmsprom_device::set_pdc_bit(*device, _bit);

#define MCFG_TMSPROM_CTL1_BIT(_bit) \
	tmsprom_device::set_ctl1_bit(*device, _bit);

#define MCFG_TMSPROM_CTL2_BIT(_bit) \
	tmsprom_device::set_ctl2_bit(*device, _bit);

#define MCFG_TMSPROM_CTL4_BIT(_bit) \
	tmsprom_device::set_ctl4_bit(*device, _bit);

#define MCFG_TMSPROM_CTL8_BIT(_bit) \
	tmsprom_device::set_ctl8_bit(*device, _bit);

#define MCFG_TMSPROM_RESET_BIT(_bit) \
	tmsprom_device::set_reset_bit(*device, _bit);

#define MCFG_TMSPROM_STOP_BIT(_bit) \
	tmsprom_device::set_stop_bit(*device, _bit);

#define MCFG_TMSPROM_PDC_CB(_devcb) \
	devcb = &tmsprom_device::set_pdc_callback(*device, DEVCB_##_devcb);

#define MCFG_TMSPROM_CTL_CB(_devcb) \
	devcb = &tmsprom_device::set_ctl_callback(*device, DEVCB_##_devcb);

#endif /* __TMS5110_H__ */
