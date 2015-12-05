// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __MSX_CART_DOOLY_H
#define __MSX_CART_DOOLY_H

#include "bus/msx_cart/cartridge.h"


extern const device_type MSX_CART_DOOLY;


class msx_cart_dooly : public device_t
						, public msx_cart_interface
{
public:
	msx_cart_dooly(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void initialize_cartridge() override;

	virtual DECLARE_READ8_MEMBER(read_cart);
	virtual DECLARE_WRITE8_MEMBER(write_cart);

private:
	UINT8 m_prot;
};


#endif
