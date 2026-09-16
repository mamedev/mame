// license:BSD-3-Clause
// copyright-holders:kmg
/**********************************************************************

    Nintendo Family Computer Konami 'Doremikko' RJ250 Piano Keyboard

**********************************************************************/

#ifndef MAME_BUS_NES_CTRL_DOREPIANO_H
#define MAME_BUS_NES_CTRL_DOREPIANO_H

#pragma once

#include "ctrl.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_dorepiano_device

class nes_dorepiano_device :
	public device_t,
	public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_dorepiano_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_exp(offs_t offset) override;
	virtual void write(u8 data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_ioport_array<8> m_port;
	u8 m_cur_port;
	u8 m_latch;
	u8 m_mask;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_DOREPIANO, nes_dorepiano_device)

#endif // MAME_BUS_NES_CTRL_DOREPIANO_H
