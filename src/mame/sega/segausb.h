// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Sega g80 common sound hardware

*************************************************************************/
#ifndef MAME_SEGA_SEGAUSB_H
#define MAME_SEGA_SEGAUSB_H

#pragma once

#define ENABLE_SEGAUSB_NETLIST (0)

#include "cpu/mcs48/mcs48.h"
#include "machine/netlist.h"
#include "machine/timer.h"
#include "netlist/nl_setup.h"
#include "nl_segausb.h"

#if (ENABLE_SEGAUSB_NETLIST)
#include "machine/pit8253.h"
#endif

class usb_sound_device : public device_t, public device_mixer_interface
{
public:
	template <typename T> usb_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&maincpu_tag)
		: usb_sound_device(mconfig, tag, owner, clock)
	{
		m_maincpu.set_tag(maincpu_tag);
	}

	usb_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	uint8_t status_r();
	void data_w(uint8_t data);
	uint8_t ram_r(offs_t offset);
	void ram_w(offs_t offset, uint8_t data);

	uint8_t workram_r(offs_t offset);
	void workram_w(offs_t offset, uint8_t data);

	TIMER_DEVICE_CALLBACK_MEMBER( increment_t1_clock_timer_cb );

	void usb_map(address_map &map) ATTR_COLD;
	void usb_map_rom(address_map &map) ATTR_COLD;
	void usb_portmap(address_map &map) ATTR_COLD;

#if (ENABLE_SEGAUSB_NETLIST)
	TIMER_DEVICE_CALLBACK_MEMBER( gos_timer );
#endif

protected:

	usb_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

#if (!ENABLE_SEGAUSB_NETLIST)
	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;
#endif

private:
	// internal state
	u8 m_in_latch;             // input latch
	u8 m_out_latch;            // output latch
	u8 m_last_p2_value;        // current P2 output value
	optional_shared_ptr<u8> m_program_ram;          // pointer to program RAM
	memory_share_creator<u8> m_work_ram;            // pointer to work RAM
	u8 m_work_ram_bank;        // currently selected work RAM bank
	u8 m_t1_clock;             // T1 clock value
	u8 m_t1_clock_mask;        // T1 clock mask (configured via jumpers)

protected:
	// devices
	required_device<i8035_device> m_ourcpu;
	required_device<cpu_device> m_maincpu;

private:
#if (ENABLE_SEGAUSB_NETLIST)
	// PIT devices
	required_device_array<pit8253_device, 3> m_pit;

	// channel 0
	required_device_array<netlist_mame_analog_input_device, 3> m_nl_dac0;
	required_device<netlist_mame_logic_input_device> m_nl_sel0;
	required_device_array<netlist_mame_logic_input_device, 3> m_nl_pit0_out;

	// channel 1
	required_device_array<netlist_mame_analog_input_device, 3> m_nl_dac1;
	required_device<netlist_mame_logic_input_device> m_nl_sel1;
	required_device_array<netlist_mame_logic_input_device, 3> m_nl_pit1_out;

	// channel 2
	required_device_array<netlist_mame_analog_input_device, 3> m_nl_dac2;
	required_device<netlist_mame_logic_input_device> m_nl_sel2;
	required_device_array<netlist_mame_logic_input_device, 3> m_nl_pit2_out;

	u8 m_gos_clock;            // state of the GOD clock
#else
	struct g80_filter_state
	{
		g80_filter_state() { }

		void configure(double r, double c);
		double step_rc(double input) { return capval += (input - capval) * exponent; }
		double step_cr(double input) { double const result = input - capval; capval += result * exponent; return result; }

		double capval = 0.0; // current capacitor value
		double exponent = 0.0; // constant exponent
	};

	struct timer8253
	{
		struct channel
		{
			channel() { }

			void clock();

			u8  holding     = 0; // holding until counts written?
			u8  latchmode   = 0; // latching mode
			u8  latchtoggle = 0; // latching state
			u8  clockmode   = 0; // clocking mode
			u8  bcdmode     = 0; // BCD mode?
			u8  output      = 0; // current output value
			u8  lastgate    = 0; // previous gate value
			u8  gate        = 0; // current gate value
			u16 count       = 0; // initial count
			u16 remain      = 0; // current down counter value
			s32 subcount    = 0; // subcount (2MHz clocks per input clock)
		};

		timer8253() : env{ 0.0, 0.0, 0.0 } { }

		channel chan[3];                 // three channels' worth of information
		double env[3];                   // envelope value for each channel
		g80_filter_state chan_filter[2]; // filter states for the first two channels
		g80_filter_state gate1;          // first RC filter state
		g80_filter_state gate2;          // second RC filter state
		u8 config = 0;                   // configuration for this timer
	};

	sound_stream *m_stream;      // output stream
	timer8253 m_timer_group[3];  // 3 groups of timers
	u8 m_timer_mode[3];          // mode control for each group
	u32 m_noise_shift;
	u8 m_noise_state;
	s32 m_noise_subcount;
	double m_gate_rc1_exp[2];
	double m_gate_rc2_exp[2];
	g80_filter_state m_final_filter;
	g80_filter_state m_noise_filters[5];
#endif

	TIMER_CALLBACK_MEMBER( delayed_usb_data_w );
	void timer_w(int which, u8 offset, u8 data);
	void env_w(int which, u8 offset, u8 data);

	uint8_t p1_r();
	void p1_w(uint8_t data);
	void p2_w(uint8_t data);
	int t1_r();
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
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(SEGAUSBROM, usb_rom_sound_device)

#endif // MAME_SEGA_SEGAUSB_H
