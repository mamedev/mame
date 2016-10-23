// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __MSX_CART_HALNOTE_H
#define __MSX_CART_HALNOTE_H

#include "bus/msx_cart/cartridge.h"


extern const device_type MSX_CART_HALNOTE;


class msx_cart_halnote : public device_t
						, public msx_cart_interface
{
public:
	msx_cart_halnote(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void initialize_cartridge() override;

	virtual uint8_t read_cart(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_cart(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	void restore_banks();

private:
	uint8_t m_selected_bank[8];
	uint8_t *m_bank_base[8];

	void map_bank(int bank);
};


#endif
