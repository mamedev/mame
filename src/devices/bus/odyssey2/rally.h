// license:BSD-3-Clause
// copyright-holders:hap
/**********************************************************************

    Videopac+ 55/58/59/60 cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_ODYSSEY2_RALLY_H
#define MAME_BUS_ODYSSEY2_RALLY_H

#pragma once

#include "slot.h"


// ======================> o2_rally_device

class o2_rally_device : public device_t,
						public device_o2_cart_interface
{
public:
	// construction/destruction
	o2_rally_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	virtual void cart_init() override;

	virtual u8 read_rom04(offs_t offset) override;
	virtual u8 read_rom0c(offs_t offset) override { return read_rom04(offset + 0x400); }

	virtual void write_p1(u8 data) override { m_control = data; }
	virtual void io_write(offs_t offset, u8 data) override;

private:
	u8 m_control = 0;
	u8 m_bank = 0;
};

// device type definition
DECLARE_DEVICE_TYPE(O2_ROM_RALLY, o2_rally_device)

#endif // MAME_BUS_ODYSSEY2_RALLY_H
