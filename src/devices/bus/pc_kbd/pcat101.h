// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * IBM Model M PC/AT and PS/2 101-key keyboard emulation.
 */

#ifndef MAME_BUS_PC_KBD_PCAT101_H
#define MAME_BUS_PC_KBD_PCAT101_H

#pragma once

#include "cpu/m6805/m68705.h"
#include "pc_kbdc.h"
#include "machine/rescap.h"

class ibm_pc_at_101_keyboard_device
	: public device_t
	, public device_pc_kbd_interface
{
public:
	// construction/destruction
	ibm_pc_at_101_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	// device_pc_kbd_interface overrides
	virtual DECLARE_WRITE_LINE_MEMBER(clock_write) override { m_maincpu->set_input_line(M68705_IRQ_LINE, state); };
	virtual DECLARE_WRITE_LINE_MEMBER(data_write) override { };

private:
	// ports A, C: matrix column
	// port D: matrix row
	// port B: clock/data and leds

	enum
	{
		LED_SCROLL = 0,
		LED_NUM,
		LED_CAPS
	};

	required_device<m68705_device> m_maincpu;
	required_ioport_array<16> m_column;
	output_finder<3> m_leds;
};

// device type definition
DECLARE_DEVICE_TYPE(PC_KBD_IBM_PC_AT_101, ibm_pc_at_101_keyboard_device)

#endif // MAME_BUS_PC_KBD_PCAT101_H
