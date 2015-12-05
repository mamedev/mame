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
	sega8_cardcatch_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_cart);
	virtual DECLARE_WRITE8_MEMBER(write_cart);
	virtual DECLARE_WRITE8_MEMBER(write_mapper) override {}

	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	required_device<sega8_card_slot_device> m_card;
};





// device type definition
extern const device_type SEGA8_ROM_CARDCATCH;

#endif
