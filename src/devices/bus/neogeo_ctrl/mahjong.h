// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    SNK Neo Geo Mahjong controller emulation

**********************************************************************/
#ifndef MAME_BUS_NEOGEO_CTRL_MAHJONG_H
#define MAME_BUS_NEOGEO_CTRL_MAHJONG_H

#pragma once

#include "ctrl.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> neogeo_mjctrl_ac_device

class neogeo_mjctrl_ac_device : public device_t, public device_neogeo_control_port_interface
{
public:
	// construction/destruction
	neogeo_mjctrl_ac_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	neogeo_mjctrl_ac_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual uint8_t read_start_sel() override;
	virtual void device_start() override ATTR_COLD;

	// device_neogeo_control_port_interface overrides
	virtual uint8_t read_ctrl() override;
	virtual void write_ctrlsel(uint8_t data) override;

	uint8_t m_ctrl_sel;
	required_ioport m_ss;

private:
	required_ioport_array<3> m_mjpanel;
};

// ======================> neogeo_mjctrl_device

class neogeo_mjctrl_device : public neogeo_mjctrl_ac_device
{
public:
	// construction/destruction
	neogeo_mjctrl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_neogeo_control_port_interface overrides
	virtual uint8_t read_start_sel() override;
};


// device type definition
DECLARE_DEVICE_TYPE(NEOGEO_MJCTRL_AC, neogeo_mjctrl_ac_device)
DECLARE_DEVICE_TYPE(NEOGEO_MJCTRL,    neogeo_mjctrl_device)


#endif // MAME_BUS_NEOGEO_CTRL_MAHJONG_H
