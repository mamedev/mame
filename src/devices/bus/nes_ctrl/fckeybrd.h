// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer Keyboard HVC-007 and Data Recorder HVC-008

**********************************************************************/

#ifndef MAME_BUS_NES_CTRL_FCKEYBOARD_H
#define MAME_BUS_NES_CTRL_FCKEYBOARD_H

#pragma once

#include "ctrl.h"
#include "imagedev/cassette.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_fckeybrd_device

class nes_fckeybrd_device : public device_t,
							public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_fckeybrd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_exp(offs_t offset) override;
	virtual void write(u8 data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<cassette_image_device> m_cassette;
	required_ioport_array<10> m_kbd;
	u8 m_fck_scan, m_fck_nibble, m_fck_enable;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_FCKEYBOARD, nes_fckeybrd_device)

#endif // MAME_BUS_NES_CTRL_FCKEYBOARD_H
