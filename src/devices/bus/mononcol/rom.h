// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_MONONCOL_ROM_H
#define MAME_BUS_MONONCOL_ROM_H

#pragma once

#include "slot.h"

#include "machine/generic_spi_flash.h"

// ======================> mononcol_rom_device

class mononcol_rom_device : public device_t, public device_mononcol_cart_interface
{
protected:
	// construction/destruction
	mononcol_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
};


// ======================> mononcol_rom_plain_device

class mononcol_rom_plain_device : public mononcol_rom_device
{
public:
	// construction/destruction
	mononcol_rom_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read() override { return m_spi->read(); }
	DECLARE_WRITE_LINE_MEMBER(dir_w) override { m_spi->dir_w(state); }
	void write(uint8_t data) override { m_spi->write(data); }
	void set_ready() override { m_spi->set_ready(); }
	void set_spi_region(uint8_t* region) override { m_spi->set_rom_ptr(region); } 
	void set_spi_size(size_t size) override { m_spi->set_rom_size(size); } 


protected:
	mononcol_rom_plain_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<generic_spi_flash_device> m_spi;

};

// device type definition
DECLARE_DEVICE_TYPE(MONONCOL_ROM_PLAIN,    mononcol_rom_plain_device)


#endif // MAME_BUS_MONONCOL_ROM_H
