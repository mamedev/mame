// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood
#ifndef __NEOGEO_FATFURY2_CART_H
#define __NEOGEO_FATFURY2_CART_H

#include "neogeo_slot.h"
#include "banked_cart.h"
#include "fatfury2_prot.h"

// ======================> neogeo_fatfury2_cart

class neogeo_fatfury2_cart : public device_t,
						public device_neogeo_cart_interface
{
public:
	// construction/destruction
	neogeo_fatfury2_cart(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT16 clock, std::string shortname, std::string source);
	neogeo_fatfury2_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT16 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read_rom) override;

	virtual void activate_cart(ACTIVATE_CART_PARAMS) override;

	required_device<neogeo_banked_cart_device> m_banked_cart;
	required_device<fatfury2_prot_device> m_fatfury2_prot;

};



// device type definition
extern const device_type NEOGEO_FATFURY2_CART;


#endif
