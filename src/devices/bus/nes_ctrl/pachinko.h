// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer Pachinko Controller

**********************************************************************/

#pragma once

#ifndef __NES_PACHINKO__
#define __NES_PACHINKO__


#include "emu.h"
#include "ctrl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_pachinko_device

class nes_pachinko_device : public device_t,
							public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_pachinko_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual UINT8 read_exp(offs_t offset) override;
	virtual void write(UINT8 data) override;

	required_ioport m_joypad;
	required_ioport m_trigger;
	UINT32 m_latch;
};

// device type definition
extern const device_type NES_PACHINKO;

#endif
