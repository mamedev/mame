// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_SOFTCARD_H
#define MAME_BUS_MSX_CART_SOFTCARD_H

#pragma once

#include "cartridge.h"
#include "bus/msx/softcard/softcard.h"
#include "imagedev/cartrom.h"


DECLARE_DEVICE_TYPE(MSX_CART_SOFTCARD, msx_cart_softcard_device)


class msx_cart_softcard_device : public device_t
								, public device_cartrom_image_interface
								, public device_single_card_slot_interface<softcard_interface>
								, public msx_cart_interface
{
public:
	msx_cart_softcard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "softcard"; }
	virtual const char *file_extensions() const noexcept override { return "bin,rom"; }

protected:
	virtual void device_resolve_objects() override;
	virtual void device_start() override { }
	virtual void device_add_mconfig(machine_config &config) override;
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	softcard_interface *m_softcard;
};


#endif // MAME_BUS_MSX_CART_SOFTCARD_H
