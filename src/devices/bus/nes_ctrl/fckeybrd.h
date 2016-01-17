// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer Keyboard Component

**********************************************************************/

#pragma once

#ifndef __NES_FCKEYBRD__
#define __NES_FCKEYBRD__


#include "emu.h"
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
	nes_fckeybrd_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual ioport_constructor device_input_ports() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual UINT8 read_exp(offs_t offset) override;
	virtual void write(UINT8 data) override;

private:
	required_device<cassette_image_device> m_cassette;
	required_ioport_array<9> m_kbd;
	UINT8 m_fck_scan, m_fck_mode;
};


// device type definition
extern const device_type NES_FCKEYBOARD;


#endif
