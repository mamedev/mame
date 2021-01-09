// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Fabio Priuli, hap
/**********************************************************************

    Homebrew KTAA cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_ODYSSEY2_KTAA_H
#define MAME_BUS_ODYSSEY2_KTAA_H

#pragma once

#include "slot.h"


// ======================> o2_ktaa_device

class o2_ktaa_device : public device_t,
						public device_o2_cart_interface
{
public:
	// construction/destruction
	o2_ktaa_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	virtual void cart_init() override;

	virtual u8 read_rom04(offs_t offset) override;
	virtual u8 read_rom0c(offs_t offset) override { return read_rom04(offset + 0x800); }

	virtual void write_p1(u8 data) override { m_bank = data & m_bank_mask; }

private:
	u32 m_page_size = 0;
	u8 m_bank_mask = 0;
	u8 m_bank = 0;
};

// device type definition
DECLARE_DEVICE_TYPE(O2_ROM_KTAA, o2_ktaa_device)

#endif // MAME_BUS_ODYSSEY2_KTAA_H
