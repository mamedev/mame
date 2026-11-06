// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, MetalliC
#ifndef MAME_BUS_MEGADRIVE_STM95_H
#define MAME_BUS_MEGADRIVE_STM95_H

#pragma once

#include "md_slot.h"

#include "machine/eepromspi.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> md_eeprom_stm95_device

class md_eeprom_stm95_device : public device_t, public device_md_cart_interface
{
public:
	// construction/destruction
	md_eeprom_stm95_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual uint16_t read_a13(offs_t offset) override;
	virtual void write_a13(offs_t offset, uint16_t data) override;

protected:
	md_eeprom_stm95_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void unload(device_image_interface &image) override ATTR_COLD;

private:
	required_device<eeprom_spi_m95320_device> m_stm95;

	uint8_t m_bank[3];
	int m_rdcnt;
};


// device type definition
DECLARE_DEVICE_TYPE(MD_EEPROM_STM95, md_eeprom_stm95_device)

#endif // MAME_BUS_MEGADRIVE_STM95_H
