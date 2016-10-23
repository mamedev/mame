// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __MSX_CART_FS_SR022_H
#define __MSX_CART_FS_SR022_H

#include "bus/msx_cart/cartridge.h"


extern const device_type MSX_CART_FS_SR022;


class msx_cart_fs_sr022 : public device_t
						, public msx_cart_interface
{
public:
	msx_cart_fs_sr022(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void initialize_cartridge() override;

	virtual uint8_t read_cart(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_cart(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

private:
	const uint8_t *m_bunsetsu_rom;
	uint32_t m_bunsetsu_address;
};


#endif
