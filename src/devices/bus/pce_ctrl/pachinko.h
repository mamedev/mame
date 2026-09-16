// license:BSD-3-Clause
// copyright-holders:kmg
/**********************************************************************

    NEC PC Engine Coconuts Japan CJPC-101 Pachinko Controller

**********************************************************************/

#ifndef MAME_BUS_PCE_CTRL_PACHINKO_H
#define MAME_BUS_PCE_CTRL_PACHINKO_H

#pragma once


#include "pcectrl.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pce_pachinko_device

class pce_pachinko_device : public device_t,
							public device_pce_control_port_interface
{
public:
	// construction/destruction
	pce_pachinko_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_pce_control_port_interface overrides
	virtual u8 peripheral_r() override;
	virtual void sel_w(int state) override;
	virtual void clr_w(int state) override;

private:
	// IO ports
	required_ioport m_buttons;
	required_ioport m_dpad;
	required_ioport m_trigger;

	// internal states
	u8 m_counter;
	bool m_prev_sel; // previous SEL pin state
	bool m_prev_clr; // previous CLR pin state
};


// device type definition
DECLARE_DEVICE_TYPE(PCE_PACHINKO, pce_pachinko_device)


#endif // MAME_BUS_PCE_CTRL_PACHINKO_H
