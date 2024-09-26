// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer Subor Keyboard (used by some Famiclones)

**********************************************************************/

#ifndef MAME_BUS_NES_CTRL_SUBORKEY_H
#define MAME_BUS_NES_CTRL_SUBORKEY_H

#pragma once

#include "ctrl.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_suborkey_device

class nes_suborkey_device : public device_t,
							public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_suborkey_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_exp(offs_t offset) override;
	virtual void write(uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_ioport_array<13> m_kbd;
	uint8_t m_fck_scan, m_fck_mode;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_SUBORKEYBOARD, nes_suborkey_device)

#endif // MAME_BUS_NES_CTRL_SUBORKEY_H
