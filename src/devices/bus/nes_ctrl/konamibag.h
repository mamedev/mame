// license:BSD-3-Clause
// copyright-holders:kmg
/**********************************************************************

    Nintendo Family Computer - Konami Exciting Boxing Air Bag

**********************************************************************/

#ifndef MAME_BUS_NES_CTRL_KONAMIBAG_H
#define MAME_BUS_NES_CTRL_KONAMIBAG_H

#pragma once

#include "ctrl.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_konamibag_device

class nes_konamibag_device :
	public device_t,
	public device_nes_control_port_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::CONTROLS; }

	// construction/destruction
	nes_konamibag_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_exp(offs_t offset) override;
	virtual void write(u8 data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_ioport_array<2> m_sensor;
	u8 m_cur_sensor;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_KONAMIBAG, nes_konamibag_device)

#endif // MAME_BUS_NES_CTRL_KONAMIBAG_H
