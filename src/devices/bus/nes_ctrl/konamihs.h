// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer Konami Hyper Shot Controllers

**********************************************************************/

#pragma once

#ifndef __NES_KONAMIHS__
#define __NES_KONAMIHS__


#include "ctrl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_konamihs_device

class nes_konamihs_device : public device_t,
							public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_konamihs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read_exp(offs_t offset) override;
	virtual void write(uint8_t data) override;

	required_ioport m_ipt_p1;
	required_ioport m_ipt_p2;
	uint32_t m_latch_p1, m_latch_p2;
};

// device type definition
extern const device_type NES_KONAMIHS;

#endif
