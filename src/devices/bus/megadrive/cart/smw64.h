// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_MEGADRIVE_CART_SMW64_H
#define MAME_BUS_MEGADRIVE_CART_SMW64_H

#pragma once

#include "rom.h"
#include "slot.h"

class megadrive_unl_smw64_device : public megadrive_rom_device
{
public:
	megadrive_unl_smw64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type imperfect_features() { return feature::PROTECTION; }

	virtual void cart_map(address_map &map) override ATTR_COLD;
protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
private:
	memory_bank_array_creator<2> m_page_rom;

	u8 m_ctrl[2];
	u8 m_data[2];
	u8 m_latch[3];
	u8 m_page_61, m_page_67;
};



DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_SMW64,        megadrive_unl_smw64_device)


#endif // MAME_BUS_MEGADRIVE_CART_SMW64_H
