// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    titan3plus2.h

    Implemention of the Titan /// Plus II card

*********************************************************************/

#ifndef MAME_BUS_A2BUS_TITAN3PLUS2_H
#define MAME_BUS_A2BUS_TITAN3PLUS2_H

#pragma once

#include "a2bus.h"
#include "bus/a2gameio/gameio.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_titan3plus2_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_titan3plus2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_titan3plus2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual u8 read_inh_rom(u16 offset) override;
	virtual void write_inh_rom(u16 offset, u8 data) override;
	virtual bool inh_check(uint16_t offset, bool bIsWrite) override;

private:
	required_device<apple2_gameio_device> m_gameio;

	u8 read_c08x(u8 offset);
	void write_c08x(u8 offset, u8 data);
	u8 read_cnxx(u8 offset) override;
	void do_io(int offset);

	int m_inh_state;
	int m_last_offset;
	int m_dxxx_bank;
	int m_main_bank;
	u8 m_ram[128*1024];
	bool m_enabled;

	double m_joystick_x1_time, m_joystick_y1_time, m_joystick_x2_time, m_joystick_y2_time, m_x_calibration, m_y_calibration;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_TITAN3PLUS2, a2bus_titan3plus2_device)

#endif // MAME_BUS_A2BUS_TITAN3PLUS2_H
