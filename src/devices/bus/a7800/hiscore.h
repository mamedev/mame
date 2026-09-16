// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_A7800_HISCORE_H
#define MAME_BUS_A7800_HISCORE_H

#pragma once

#include "a78_slot.h"
#include "rom.h"


// ======================> a78_hiscore_device

class a78_hiscore_device : public a78_rom_device
{
public:
	// construction/destruction
	a78_hiscore_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_04xx(offs_t offset) override;
	virtual void write_04xx(offs_t offset, uint8_t data) override;
	virtual uint8_t read_10xx(offs_t offset) override;
	virtual void write_10xx(offs_t offset, uint8_t data) override;
	virtual uint8_t read_30xx(offs_t offset) override;
	virtual uint8_t read_40xx(offs_t offset) override;
	virtual void write_40xx(offs_t offset, uint8_t data) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<a78_cart_slot_device> m_hscslot;
};



// device type definition
DECLARE_DEVICE_TYPE(A78_HISCORE, a78_hiscore_device)


#endif // MAME_BUS_A7800_HISCORE_H
