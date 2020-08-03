// license:BSD-3-Clause
// copyright-holders:hap
/**********************************************************************

    Videopac 40 cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_ODYSSEY2_4IN1_H
#define MAME_BUS_ODYSSEY2_4IN1_H

#pragma once

#include "slot.h"
#include "rom.h"


// ======================> o2_4in1_device

class o2_4in1_device : public o2_rom_device
{
public:
	// construction/destruction
	o2_4in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	virtual void cart_init() override;

	virtual u8 read_rom04(offs_t offset) override { return m_rom[offset + 0x400]; }
	virtual u8 read_rom0c(offs_t offset) override { return m_rom[offset + 0xc00]; }

	virtual void write_p1(u8 data) override { m_control = data; }
	virtual void write_p2(u8 data) override { m_bank = data & 3; }
	virtual u8 io_read(offs_t offset) override;

private:
	u8 m_control = 0;
	u8 m_bank = 0;
};

// device type definition
DECLARE_DEVICE_TYPE(O2_ROM_4IN1, o2_4in1_device)

#endif // MAME_BUS_ODYSSEY2_4IN1_H
