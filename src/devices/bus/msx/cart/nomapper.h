// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_NOMAPPER_H
#define MAME_BUS_MSX_CART_NOMAPPER_H

#pragma once

#include "bus/msx/slot/cartridge.h"


DECLARE_DEVICE_TYPE(MSX_CART_NOMAPPER, msx_cart_nomapper_device)


class msx_cart_nomapper_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_nomapper_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { }

	virtual image_init_result initialize_cartridge(std::string &message) override;

private:
	uint32_t m_start_address;
	uint32_t m_end_address;

	void install_memory();
};

#endif // MAME_BUS_MSX_CART_NOMAPPER_H
