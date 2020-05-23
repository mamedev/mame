// license:BSD-3-Clause
// copyright-holders:R. Belmont, O. Galibert
/*********************************************************************

    a2iwm.h

    Apple II IWM Controller Card, new floppy

*********************************************************************/

#ifndef MAME_BUS_A2BUS_A2IWM_H
#define MAME_BUS_A2BUS_A2IWM_H

#pragma once

#include "a2bus.h"
#include "imagedev/floppy.h"
#include "formats/flopimg.h"
#include "machine/iwm.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


class a2bus_iwm_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_iwm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(u8 offset) override;
	virtual void write_c0nx(u8 offset, u8 data) override;

	required_device<iwm_device> m_iwm;
	required_device_array<floppy_connector, 2> m_floppy;

private:
	void devsel_w(u8 data);
	void phases_w(u8 data);
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_IWM, a2bus_iwm_device)

#endif  // MAME_BUS_A2BUS_A2IWM_H
