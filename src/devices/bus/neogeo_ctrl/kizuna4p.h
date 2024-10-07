// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    SNK Neo Geo Kizuna 4Players controller emulation

**********************************************************************/

#ifndef MAME_BUS_NEOGEO_CTRL_KIZUNA4P_H
#define MAME_BUS_NEOGEO_CTRL_KIZUNA4P_H

#pragma once


#include "ctrl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> neogeo_kizuna4p_device

class neogeo_kizuna4p_device : public device_t, public device_neogeo_ctrl_edge_interface
{
public:
	// construction/destruction
	neogeo_kizuna4p_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_neogeo_control_port_interface overrides
	virtual uint8_t in0_r() override;
	virtual uint8_t in1_r() override;
	virtual uint8_t read_start_sel() override;
	virtual void write_ctrlsel(uint8_t data) override;

private:
	required_ioport_array<4> m_joy;
	required_ioport_array<2> m_ss;
	uint8_t m_ctrl_sel;
};


// device type definition
DECLARE_DEVICE_TYPE(NEOGEO_KIZ4P, neogeo_kizuna4p_device)


#endif // MAME_BUS_NEOGEO_CTRL_KIZUNA4P_H
