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
	virtual void device_start() override;
	virtual void device_reset() override;

	// device-level overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

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

	DECLARE_WRITE_LINE_MEMBER(reset_w);
	DECLARE_READ8_MEMBER(cpu_mem_r);
	DECLARE_WRITE8_MEMBER(cpu_mem_w);
	DECLARE_READ8_MEMBER(cpu_io_r);
	DECLARE_WRITE8_MEMBER(cpu_io_w);
	void cpu_mem_map(address_map &map);
	void cpu_io_map(address_map &map);
	void z80_m1_w(uint8_t data);
};

// device type definition
DECLARE_DEVICE_TYPE(HP82900_IO_CARD, hp82900_io_card_device)

#endif // MAME_BUS_HP80_IO_82900_H
