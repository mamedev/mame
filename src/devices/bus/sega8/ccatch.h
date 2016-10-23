// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __SEGA8_CCATCH_H
#define __SEGA8_CCATCH_H

#include "sega8_slot.h"
#include "rom.h"

// ======================> sega8_cardcatch_device

class sega8_cardcatch_device : public sega8_rom_device
{
public:
	// construction/destruction
	sega8_cardcatch_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_cart(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_cart(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual void write_mapper(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override {}

	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	required_device<sega8_card_slot_device> m_card;
};





// device type definition
extern const device_type SEGA8_ROM_CARDCATCH;

#endif
