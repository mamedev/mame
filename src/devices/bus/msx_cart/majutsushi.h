// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __MSX_CART_MAJUTSUSHI_H
#define __MSX_CART_MAJUTSUSHI_H

#include "bus/msx_cart/cartridge.h"
#include "sound/dac.h"


extern const device_type MSX_CART_MAJUTSUSHI;


class msx_cart_majutsushi : public device_t
						, public msx_cart_interface
{
public:
	msx_cart_majutsushi(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual void initialize_cartridge() override;

	virtual uint8_t read_cart(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_cart(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	void restore_banks();

private:
	required_device<dac_byte_interface> m_dac;

	uint8_t m_selected_bank[4];
	uint8_t *m_bank_base[8];
};


#endif
