// license:BSD-3-Clause
// copyright-holders:hap
/**********************************************************************

    Videopac Service Test cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_ODYSSEY2_TEST_H
#define MAME_BUS_ODYSSEY2_TEST_H

#pragma once

#include "slot.h"


// ======================> o2_test_device

class o2_test_device : public device_t,
						public device_o2_cart_interface
{
public:
	// construction/destruction
	o2_test_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	virtual void cart_init() override;

	virtual u8 read_rom04(offs_t offset) override { return m_rom[offset]; }
	virtual u8 read_rom0c(offs_t offset) override { return m_rom[offset]; }
	virtual void bus_write(u8 data) override { m_bus_data = data; }

	virtual void write_p1(u8 data) override;

private:
	output_finder<> m_digit_out;

	u8 m_control = 0;
	u8 m_bus_data = 0;
};

// device type definition
DECLARE_DEVICE_TYPE(O2_ROM_TEST, o2_test_device)

#endif // MAME_BUS_ODYSSEY2_TEST_H
