// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood
#ifndef __NEOGEO_KOF98_CART_H
#define __NEOGEO_KOF98_CART_H

#include "neogeo_slot.h"
#include "banked_cart.h"
#include "kof98_prot.h"

// ======================> neogeo_kof98_cart

class neogeo_kof98_cart : public device_t,
						public device_neogeo_cart_interface
{
public:
	// construction/destruction
	neogeo_kof98_cart(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT16 clock, const char *shortname, const char *source);
	neogeo_kof98_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual machine_config_constructor device_mconfig_additions() const;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read_rom);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual void activate_cart(ACTIVATE_CART_PARAMS);

	required_device<neogeo_banked_cart_device> m_banked_cart;
	required_device<kof98_prot_device> m_kof98_prot;

};



// device type definition
extern const device_type NEOGEO_KOF98_CART;


#endif
