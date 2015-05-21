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
	nes_fckeybrd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual ioport_constructor device_input_ports() const;
	virtual machine_config_constructor device_mconfig_additions() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	virtual UINT8 read_exp(offs_t offset);
	virtual void write(UINT8 data);

private:
	required_device<cassette_image_device> m_cassette;
	required_ioport_array<9> m_kbd;
	UINT8 m_fck_scan, m_fck_mode;
};


// device type definition
extern const device_type NES_FCKEYBOARD;


#endif
