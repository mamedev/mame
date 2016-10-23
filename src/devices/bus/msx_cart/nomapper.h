// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __MSX_CART_NOMAPPER_H
#define __MSX_CART_NOMAPPER_H

#include "bus/msx_cart/cartridge.h"


extern const device_type MSX_CART_NOMAPPER;


class msx_cart_nomapper : public device_t
						, public msx_cart_interface
{
public:
	msx_cart_nomapper(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	virtual void initialize_cartridge() override;

	virtual uint8_t read_cart(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;

private:
	uint32_t m_start_address;
	uint32_t m_end_address;
};

#endif
