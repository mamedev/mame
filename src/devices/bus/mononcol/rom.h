// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_MONONCOL_ROM_H
#define MAME_BUS_MONONCOL_ROM_H

#pragma once

#include "slot.h"

#include "machine/generic_spi_flash.h"


// ======================> mononcol_rom_plain_device

class mononcol_rom_plain_device : public device_t, public device_mononcol_cart_interface
{
public:
	// construction/destruction
	mononcol_rom_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read() override { return m_spi->read(); }
	void dir_w(int state) override { m_spi->dir_w(state); }
	void write(uint8_t data) override { m_spi->write(data); }
	void set_ready() override { m_spi->set_ready(); }

	void set_spi_region(uint8_t *region, size_t size) override { m_spi->set_rom_ptr(region); m_spi->set_rom_size(size); }

protected:
	mononcol_rom_plain_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	required_device<generic_spi_flash_device> m_spi;

};


// device type declarations
DECLARE_DEVICE_TYPE(MONONCOL_ROM_PLAIN, mononcol_rom_plain_device)

#endif // MAME_BUS_MONONCOL_ROM_H
