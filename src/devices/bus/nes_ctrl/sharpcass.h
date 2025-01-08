// license:BSD-3-Clause
// copyright-holders:kmg
/**********************************************************************

    Nintendo Family Computer Sharp Cassette Interface AN-300SL

**********************************************************************/

#ifndef MAME_BUS_NES_CTRL_SHARPCASS_H
#define MAME_BUS_NES_CTRL_SHARPCASS_H

#pragma once

#include "ctrl.h"
#include "imagedev/cassette.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_sharpcass_device

class nes_sharpcass_device : public device_t,
							public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_sharpcass_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_exp(offs_t offset) override;
	virtual void write(u8 data) override;

protected:
	// device-level overrides
	virtual void device_start() override { }

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<cassette_image_device> m_cassette;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_SHARPCASS, nes_sharpcass_device)

#endif // MAME_BUS_NES_CTRL_SHARPCASS_H
