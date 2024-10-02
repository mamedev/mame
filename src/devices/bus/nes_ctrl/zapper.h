// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer & Entertainment System Zapper Lightgun

**********************************************************************/

#ifndef MAME_BUS_NES_CTRL_ZAPPER
#define MAME_BUS_NES_CTRL_ZAPPER

#pragma once

#include "ctrl.h"
#include "zapper_sensor.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_zapper_device

class nes_zapper_device : public device_t,
							public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_zapper_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_bit34() override;
	virtual u8 read_exp(offs_t offset) override;

	DECLARE_INPUT_CHANGED_MEMBER(trigger);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_device<nes_zapper_sensor_device> m_sensor;
	required_ioport m_lightx;
	required_ioport m_lighty;
	emu_timer *m_trigger;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_ZAPPER, nes_zapper_device)

#endif // MAME_BUS_NES_CTRL_ZAPPER
