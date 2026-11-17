// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy Model 12/16B/6000 low-profile Keytronic keyboard emulation

**********************************************************************/

#ifndef MAME_BUS_TRS80M2KBD_TANDY6K_KBD_H
#define MAME_BUS_TRS80M2KBD_TANDY6K_KBD_H

#pragma once

#include "kbd.h"

#include "cpu/mcs48/mcs48.h"


class tandy6k_keyboard_device : public device_t, public device_trs80m2_keyboard_interface
{
public:
	static constexpr flags_type emulation_flags() { return flags::NOT_WORKING; }

	tandy6k_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// device_trs80m2_keyboard_interface implementation
	virtual int data_r() override;
	virtual void busy_w(int state) override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_device<i8048_device> m_maincpu;
	required_ioport_array<12> m_col;
	required_ioport m_layout;
	output_finder<2> m_leds;

	uint8_t m_p1;
	uint8_t m_column;
	uint8_t m_prog;
	uint8_t m_busy;
	uint8_t m_data;
	uint8_t m_clk;

	int t0_r();
	int t1_r();
	uint8_t p1_r();
	void p1_w(uint8_t data);
	void p2_w(uint8_t data);
	void prog_w(int state);
};


DECLARE_DEVICE_TYPE(TANDY6K_KEYBOARD, tandy6k_keyboard_device)

#endif // MAME_BUS_TRS80M2KBD_TANDY6K_KBD_H
