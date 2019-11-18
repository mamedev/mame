// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Sega g80 common sound hardware

*************************************************************************/
#ifndef MAME_AUDIO_SEGASND_H
#define MAME_AUDIO_SEGASND_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "machine/timer.h"

class segag80snd_common : public driver_device {
public:
	segag80snd_common(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_audiocpu(*this, "audiocpu")
	{ }

	virtual ~segag80snd_common() = default;

	DECLARE_WRITE_LINE_MEMBER(segaspeech_int_w);

	void sega_speech_board(machine_config &config);

protected:
	void speech_map(address_map &map);
	void speech_portmap(address_map &map);

	optional_device<cpu_device> m_audiocpu;
};

#define SEGASND_SEGASPEECH_REGION "segaspeech:speech"

class speech_sound_device : public device_t, public device_sound_interface
{
public:
	speech_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto int_cb() { return m_int_cb.bind(); }

	DECLARE_WRITE8_MEMBER( data_w );
	DECLARE_WRITE8_MEMBER( control_w );

	DECLARE_READ_LINE_MEMBER( t0_r );
	DECLARE_READ_LINE_MEMBER( t1_r );
	DECLARE_READ8_MEMBER( p1_r );
	DECLARE_READ8_MEMBER( rom_r );
	DECLARE_WRITE8_MEMBER( p1_w );
	DECLARE_WRITE8_MEMBER( p2_w );

	DECLARE_WRITE_LINE_MEMBER(drq_w);

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	devcb_write_line m_int_cb;
	required_memory_region m_speech;

	// internal state
	u8 m_drq;
	u8 m_latch;
	u8 m_t0;
	u8 m_p2;

	TIMER_CALLBACK_MEMBER( delayed_speech_w );
};

DECLARE_DEVICE_TYPE(SEGASPEECH, speech_sound_device)

class usb_sound_device : public device_t, public device_sound_interface
{
public:
	template <typename T> usb_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&maincpu_tag)
		: usb_sound_device(mconfig, tag, owner, clock)
	{
		m_maincpu.set_tag(maincpu_tag);
	}

	usb_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	DECLARE_READ8_MEMBER( status_r );
	DECLARE_WRITE8_MEMBER( data_w );
	DECLARE_READ8_MEMBER( ram_r );
	DECLARE_WRITE8_MEMBER( ram_w );

	DECLARE_READ8_MEMBER( workram_r );
	DECLARE_WRITE8_MEMBER( workram_w );

	TIMER_DEVICE_CALLBACK_MEMBER( increment_t1_clock_timer_cb );

	void usb_map(address_map &map);
	void usb_map_rom(address_map &map);
	void usb_portmap(address_map &map);

protected:
	usb_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	required_device<i8035_device> m_ourcpu;
	required_device<cpu_device> m_maincpu;

private:
	struct g80_filter_state
	{
		g80_filter_state() { }

		void configure(double r, double c);
		double step_rc(double input);
		double step_cr(double input);

		double capval   = 0.0; // current capacitor value
		double exponent = 0.0; // constant exponent
	};


	struct timer8253
	{
		struct channel
		{
			channel() { }

			void clock();

			u8  holding     = 0; // holding until counts written? */
			u8  latchmode   = 0; // latching mode */
			u8  latchtoggle = 0; // latching state */
			u8  clockmode   = 0; // clocking mode */
			u8  bcdmode     = 0; // BCD mode? */
			u8  output      = 0; // current output value */
			u8  lastgate    = 0; // previous gate value */
			u8  gate        = 0; // current gate value */
			u8  subcount    = 0; // subcount (2MHz clocks per input clock) */
			u16 count       = 0; // initial count
			u16 remain      = 0; // current down counter value
		};

		timer8253() : env{ 0.0, 0.0, 0.0 } { }

		channel             chan[3];        // three channels' worth of information
		double              env[3];         // envelope value for each channel
		g80_filter_state    chan_filter[2]; // filter states for the first two channels
		g80_filter_state    gate1;          // first RC filter state
		g80_filter_state    gate2;          // second RC filter state
		u8                  config = 0;     // configuration for this timer
	};

	// internal state
	sound_stream            *m_stream;              // output stream
	u8                      m_in_latch;             // input latch
	u8                      m_out_latch;            // output latch
	u8                      m_last_p2_value;        // current P2 output value
	optional_shared_ptr<u8> m_program_ram;          // pointer to program RAM
	required_shared_ptr<u8> m_work_ram;             // pointer to work RAM
	u8                      m_work_ram_bank;        // currently selected work RAM bank
	u8                      m_t1_clock;             // T1 clock value
	u8                      m_t1_clock_mask;        // T1 clock mask (configured via jumpers)
	timer8253               m_timer_group[3];       // 3 groups of timers
	u8                      m_timer_mode[3];        // mode control for each group
	u32                     m_noise_shift;
	u8                      m_noise_state;
	u8                      m_noise_subcount;
	double                  m_gate_rc1_exp[2];
	double                  m_gate_rc2_exp[2];
	g80_filter_state        m_final_filter;
	g80_filter_state        m_noise_filters[5];

	TIMER_CALLBACK_MEMBER( delayed_usb_data_w );
	void timer_w(int which, u8 offset, u8 data);
	void env_w(int which, u8 offset, u8 data);

	DECLARE_READ8_MEMBER( p1_r );
	DECLARE_WRITE8_MEMBER( p1_w );
	DECLARE_WRITE8_MEMBER( p2_w );
	DECLARE_READ_LINE_MEMBER( t1_r );
};

DECLARE_DEVICE_TYPE(SEGAUSB, usb_sound_device)


class usb_rom_sound_device : public usb_sound_device
{
public:
	template <typename T> usb_rom_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&maincpu_tag)
		: usb_rom_sound_device(mconfig, tag, owner, clock)
	{
		m_maincpu.set_tag(maincpu_tag);
	}

	usb_rom_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
};

DECLARE_DEVICE_TYPE(SEGAUSBROM, usb_rom_sound_device)

#endif // MAME_AUDIO_SEGASND_H
