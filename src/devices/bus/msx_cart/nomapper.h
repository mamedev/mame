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
	msx_cart_nomapper(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;

	virtual void initialize_cartridge() override;

	virtual DECLARE_READ8_MEMBER(read_cart) override;

private:
	UINT32 m_start_address;
	UINT32 m_end_address;
};

#endif
