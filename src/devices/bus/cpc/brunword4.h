// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*

    Brunword MK4 - Word processor ROM / expansion

*/
#ifndef MAME_BUS_CPC_BRUNWORD4_H
#define MAME_BUS_CPC_BRUNWORD4_H

#pragma once

#include "cpcexp.h"

class cpc_brunword4_device  : public device_t,
				public device_cpc_expansion_card_interface
{
public:
	// construction/destruction
	cpc_brunword4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	void rombank_w(uint8_t data);
	virtual void set_mapping(uint8_t type) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	cpc_expansion_slot_device *m_slot;

	bool m_rombank_active;
	uint8_t m_bank_sel;
};

// device type definition
DECLARE_DEVICE_TYPE(CPC_BRUNWORD_MK4, cpc_brunword4_device)

#endif // MAME_BUS_CPC_BRUNWORD4_H
