// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Sega g80 common sound hardware

*************************************************************************/
#ifndef MAME_AUDIO_SEGAUSB_H
#define MAME_AUDIO_SEGAUSB_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "machine/pit8253.h"
#include "machine/netlist.h"
#include "machine/timer.h"
#include "netlist/nl_setup.h"

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
	TIMER_DEVICE_CALLBACK_MEMBER( gos_timer );

	void usb_map(address_map &map);
	void usb_map_rom(address_map &map);
	void usb_portmap(address_map &map);

protected:
	usb_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	required_device<i8035_device> m_ourcpu;
	required_device<cpu_device> m_maincpu;
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

// temporary for debugging
	template<int _PIT, int _CH>
	DECLARE_WRITE_LINE_MEMBER(pit_write_line)
	{
		attotime time = machine().scheduler().time();
		if (_PIT == 0)
			m_nl_pit0_out[_CH]->write_line(state);
		else if (_PIT == 1)
			m_nl_pit1_out[_CH]->write_line(state);
		else if (_PIT == 2)
			m_nl_pit2_out[_CH]->write_line(state);
	}

private:
	// internal state
	u8 m_in_latch;             // input latch
	u8 m_out_latch;            // output latch
	u8 m_last_p2_value;        // current P2 output value
	optional_shared_ptr<u8> m_program_ram;          // pointer to program RAM
	required_shared_ptr<u8> m_work_ram;             // pointer to work RAM
	u8 m_work_ram_bank;        // currently selected work RAM bank
	u8 m_t1_clock;             // T1 clock value
	u8 m_t1_clock_mask;        // T1 clock mask (configured via jumpers)
	u8 m_gos_clock;            // state of the GOD clock

	TIMER_CALLBACK_MEMBER( delayed_usb_data_w );
	void timer_w(int which, u8 offset, u8 data);
	void env_w(int which, u8 offset, u8 data);

	uint8_t p1_r();
	void p1_w(uint8_t data);
	void p2_w(uint8_t data);
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

#endif // MAME_AUDIO_SEGAUSB_H
