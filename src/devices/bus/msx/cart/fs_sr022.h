// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_FS_SR022_H
#define MAME_BUS_MSX_CART_FS_SR022_H

#pragma once

#include "bus/msx/slot/cartridge.h"


DECLARE_DEVICE_TYPE(MSX_CART_FS_SR022, msx_cart_fs_sr022_device)


class msx_cart_fs_sr022_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_fs_sr022_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual image_init_result initialize_cartridge(std::string &message) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	u8 buns_r();
	void buns_w(offs_t offset, u8 data);

	u8 *m_bunsetsu_rom;
	u32 m_bunsetsu_address;
};


#endif // MAME_BUS_MSX_CART_FS_SR022_H
