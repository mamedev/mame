// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_Z29_KBD_HE191_3425_H
#define MAME_BUS_Z29_KBD_HE191_3425_H

#pragma once

#include "bus/z29_kbd/keyboard.h"
#include "sound/beep.h"


class he191_3425_device : public device_t, public device_z29_keyboard_interface
{
public:
	// device type constructor
	he191_3425_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// miscellanous handlers
	void shift_reset(int state);

protected:
	// device-level overrides
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_z29_keyboard_interface overrides
	virtual void receive_data(bool state) override;

private:
	// MCU port handlers
	u8 mcu_pa_r();
	void mcu_pb_w(u8 data);
	void mcu_pc_w(u8 data);
	int mcu_t1_r();

	// misc. helpers
	TIMER_CALLBACK_MEMBER(receive_data_synced);

	// object finders
	required_device<cpu_device> m_mcu;
	required_device<beep_device> m_buzzer;
	required_ioport_array<12> m_matrix;
	required_ioport m_modifiers;
	output_finder<4> m_leds;

	// internal state
	u8 m_select;
	bool m_recv_data;
};

// device type declaration
DECLARE_DEVICE_TYPE(HE191_3425, he191_3425_device)

#endif // MAME_BUS_Z29_KBD_HE191_3425_H
