// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_SEGA8_MGEAR_H
#define MAME_BUS_SEGA8_MGEAR_H

#pragma once

#include "sega8_slot.h"
#include "rom.h"


// ======================> sega8_mgear_device

class sega8_mgear_device : public sega8_rom_device
{
public:
	// construction/destruction
	sega8_mgear_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_cart(offs_t offset) override { return m_subslot->read_cart(offset); }
	virtual void write_cart(offs_t offset, uint8_t data) override { m_subslot->write_cart(offset, data); }
	virtual void write_mapper(offs_t offset, uint8_t data) override { m_subslot->write_mapper(offset, data); }
	virtual int get_lphaser_xoffs() override { return m_subslot->get_lphaser_xoffs(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<sega8_cart_slot_device> m_subslot;
};


// device type definition
DECLARE_DEVICE_TYPE(SEGA8_ROM_MGEAR, sega8_mgear_device)

#endif // MAME_BUS_SEGA8_MGEAR_H
