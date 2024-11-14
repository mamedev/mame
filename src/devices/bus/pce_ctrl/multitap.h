// license:BSD-3-Clause
// copyright-holders:cam900
/**********************************************************************

    NEC PC Engine/TurboGrafx-16 Multi Tap emulation

**********************************************************************/

#ifndef MAME_BUS_PCE_CTRL_MULTITAP_H
#define MAME_BUS_PCE_CTRL_MULTITAP_H

#pragma once


#include "pcectrl.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pce_multitap_device

class pce_multitap_device : public device_t,
							public device_pce_control_port_interface
{
public:
	// construction/destruction
	pce_multitap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_pce_control_port_interface overrides
	virtual u8 peripheral_r() override;
	virtual void sel_w(int state) override;
	virtual void clr_w(int state) override;

private:
	// controller ports
	required_device_array<pce_control_port_device, 5> m_subctrl_port;

	// internal states
	u8 m_port_sel; // select port to read
	bool m_prev_sel; // previous SEL pin state
};


// device type definition
DECLARE_DEVICE_TYPE(PCE_MULTITAP, pce_multitap_device)


#endif // MAME_BUS_PCE_CTRL_MULTITAP_H
