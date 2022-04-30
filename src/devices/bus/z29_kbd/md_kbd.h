// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_Z29_KBD_MD_KBD_H
#define MAME_BUS_Z29_KBD_MD_KBD_H

#pragma once

#include "keyboard.h"
#include "cpu/mcs48/mcs48.h"
#include "sound/beep.h"

class md_keyboard_device : public device_t, public device_z29_keyboard_interface
{
public:
	// device type constructor
	md_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	// device_z29_keyboard_interface overrides
	virtual void receive_data(bool state) override;

private:
	// MCU handlers
	u8 mcu_p1_r();
	void mcu_p1_w(u8 data);
	DECLARE_READ_LINE_MEMBER(mcu_t1_r);
	void mcu_movx_w(u8 data);

	// misc. helpers
	TIMER_CALLBACK_MEMBER(receive_data_synced);
	void ls175_w(u8 data);

	// address maps
	void prog_map(address_map &map);
	void ext_map(address_map &map);

	// object finders
	required_device<mcs48_cpu_device> m_mcu;
	required_device<beep_device> m_buzzer;
	required_ioport_array<16> m_matrix;
	output_finder<2> m_leds;

	// internal state
	u8 m_14515b_select;
	bool m_ls175_clock;
	bool m_recv_data;
};

// device type declaration
DECLARE_DEVICE_TYPE(MD_KEYBOARD, md_keyboard_device)

#endif // MAME_BUS_Z29_KBD_MD_KBD_H
