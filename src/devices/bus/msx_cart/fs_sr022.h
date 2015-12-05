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
	msx_cart_fs_sr022(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void initialize_cartridge() override;

	virtual DECLARE_READ8_MEMBER(read_cart);
	virtual DECLARE_WRITE8_MEMBER(write_cart);

private:
	const UINT8 *m_bunsetsu_rom;
	UINT32 m_bunsetsu_address;
};


#endif
