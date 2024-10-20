// license:BSD-3-Clause
// copyright-holders:hap
/**********************************************************************

    Nintendo HVC-012 Family Computer Robot / Nintendo NES-012 R.O.B.

**********************************************************************/

#ifndef MAME_BUS_NES_CTRL_ROB
#define MAME_BUS_NES_CTRL_ROB

#pragma once

#include "ctrl.h"
#include "cpu/sm510/sm590.h"
#include "zapper_sensor.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_rob_device

class nes_rob_device : public device_t,
							public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_rob_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_device<sm590_device> m_maincpu;
	required_device<nes_zapper_sensor_device> m_sensor;
	required_ioport m_eye_x;
	required_ioport m_eye_y;
	output_finder<6> m_motor_out;
	output_finder<> m_led_out;

	u8 input_r();
	void output_w(offs_t offset, u8 data);
};

// device type definition
DECLARE_DEVICE_TYPE(NES_ROB, nes_rob_device)

#endif // MAME_BUS_NES_CTRL_ROB
