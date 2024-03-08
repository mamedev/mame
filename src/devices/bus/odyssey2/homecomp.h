// license:BSD-3-Clause
// copyright-holders:hap
/**********************************************************************

    Videopac+ C7420 Home Computer Module emulation

**********************************************************************/

#ifndef MAME_BUS_ODYSSEY2_HOMECOMP_H
#define MAME_BUS_ODYSSEY2_HOMECOMP_H

#pragma once

#include "slot.h"

#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "machine/gen_latch.h"


// ======================> o2_homecomp_device

class o2_homecomp_device : public device_t,
						public device_o2_cart_interface
{
public:
	// construction/destruction
	o2_homecomp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	virtual void cart_init() override;

	virtual u8 read_rom04(offs_t offset) override { return m_rom[offset]; }
	virtual u8 read_rom0c(offs_t offset) override { return m_rom[offset + 0x400]; }

	virtual void write_p1(u8 data) override;
	virtual void io_write(offs_t offset, u8 data) override;
	virtual u8 io_read(offs_t offset) override;
	virtual int t0_read() override { return m_latch[0]->pending_r(); }

private:
	required_device<cpu_device> m_maincpu;
	required_device_array<generic_latch_8_device, 2> m_latch;
	required_device<cassette_image_device> m_cass;

	std::unique_ptr<u8[]> m_ram;
	u8 m_control = 0;
	bool m_installed = false;

	void internal_io_w(offs_t offset, u8 data);
	u8 internal_io_r(offs_t offset);
	u8 internal_rom_r(offs_t offset) { return m_exrom[offset]; }

	void homecomp_io(address_map &map);
	void homecomp_mem(address_map &map);
};

// device type definition
DECLARE_DEVICE_TYPE(O2_ROM_HOMECOMP, o2_homecomp_device)

#endif // MAME_BUS_ODYSSEY2_HOMECOMP_H
