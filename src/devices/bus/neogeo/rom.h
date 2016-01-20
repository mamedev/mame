// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood
#ifndef __NEOGEO_ROM_H
#define __NEOGEO_ROM_H

#include "neogeo_slot.h"
#include "banked_cart.h"

// ======================> neogeo_rom_device

class neogeo_rom_device : public device_t,
						public device_neogeo_cart_interface
{
public:
	// construction/destruction
	neogeo_rom_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT16 clock, std::string shortname, std::string source);
	neogeo_rom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT16 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read_rom) override;

	virtual void activate_cart(ACTIVATE_CART_PARAMS) override;

	required_device<neogeo_banked_cart_device> m_banked_cart;
};



// device type definition
extern const device_type NEOGEO_ROM;


#endif
