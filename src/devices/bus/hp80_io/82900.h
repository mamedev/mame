// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    82900.h

    82900 module (CP/M auxiliary processor)

*********************************************************************/

#ifndef MAME_BUS_HP80_IO_82900_H
#define MAME_BUS_HP80_IO_82900_H

#pragma once

#include "hp80_io.h"
#include "cpu/z80/z80.h"
#include "machine/1mb5.h"

class hp82900_io_card_device : public device_t, public device_hp80_io_interface
{
public:
	// construction/destruction
	hp82900_io_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp82900_io_card_device();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void install_read_write_handlers(address_space& space , uint16_t base_addr) override;

	virtual void inten() override;
	virtual void clear_service() override;

private:
	required_device<z80_device> m_cpu;
	required_device<hp_1mb5_device> m_translator;

	// Boot ROM
	required_region_ptr<uint8_t> m_rom;

	// RAM
	std::unique_ptr<uint8_t []> m_ram;

	bool m_rom_enabled;
	uint8_t m_addr_latch;

	void reset_w(int state);
	uint8_t cpu_mem_r(offs_t offset);
	void cpu_mem_w(offs_t offset, uint8_t data);
	uint8_t cpu_io_r(offs_t offset);
	void cpu_io_w(offs_t offset, uint8_t data);
	void cpu_mem_map(address_map &map) ATTR_COLD;
	void cpu_io_map(address_map &map) ATTR_COLD;
	void z80_m1_w(uint8_t data);
};

// device type definition
DECLARE_DEVICE_TYPE(HP82900_IO_CARD, hp82900_io_card_device)

#endif // MAME_BUS_HP80_IO_82900_H
