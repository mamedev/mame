// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer Keyboard Component

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
	nes_fckeybrd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_exp(offs_t offset) override;
	virtual void write(uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<cassette_image_device> m_cassette;
	required_ioport_array<9> m_kbd;
	uint8_t m_fck_scan, m_fck_mode;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_FCKEYBOARD, nes_fckeybrd_device)

#endif // MAME_BUS_NES_CTRL_FCKEYBOARD_H
