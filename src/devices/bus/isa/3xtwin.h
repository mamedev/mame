// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Emerald Technology Inc. 3XTwin IBM 5251/11 emulation board

***************************************************************************/

#ifndef MAME_BUS_ISA_3XTWIN_H
#define MAME_BUS_ISA_3XTWIN_H

#pragma once

#include "isa.h"


// ======================> isa8_3xtwin_device

class isa8_3xtwin_device : public device_t, public device_isa8_card_interface
{
public:
	// device type constructor
	isa8_3xtwin_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	// local memory map
	void mpu_map(address_map &map);
};

// device type declaration
DECLARE_DEVICE_TYPE(ISA8_3XTWIN, isa8_3xtwin_device)

#endif // MAME_BUS_ISA_3XTWIN_H
