// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    mouse.h

    Implemention of the Apple II Mouse Card

*********************************************************************/

#ifndef MAME_DEVICES_A2BUS_MOUSE_H
#define MAME_DEVICES_A2BUS_MOUSE_H

#pragma once

#include "a2bus.h"
#include "machine/6821pia.h"
#include "cpu/m6805/m68705.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_mouse_device:
		public device_t,
		public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

protected:
	a2bus_mouse_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;

	DECLARE_WRITE8_MEMBER(pia_out_a);
	DECLARE_WRITE8_MEMBER(pia_out_b);
	DECLARE_WRITE_LINE_MEMBER(pia_irqa_w);
	DECLARE_WRITE_LINE_MEMBER(pia_irqb_w);

	DECLARE_READ8_MEMBER(mcu_port_a_r);
	DECLARE_READ8_MEMBER(mcu_port_b_r);
	DECLARE_WRITE8_MEMBER(mcu_port_a_w);
	DECLARE_WRITE8_MEMBER(mcu_port_b_w);
	DECLARE_WRITE8_MEMBER(mcu_port_c_w);

	required_device<pia6821_device> m_pia;
	required_device<m68705p_device> m_mcu;
	required_ioport m_mouseb;
	required_ioport_array<2> m_mousexy;

private:
	template <unsigned AXIS, u8 DIR, u8 CLK> void update_axis();

	required_region_ptr<uint8_t> m_rom;
	int m_rom_bank;
	uint8_t m_port_a_in, m_port_b_in;
	int m_last[2], m_count[2];
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_MOUSE, a2bus_mouse_device)

#endif // MAME_DEVICES_A2BUS_MOUSE_H
