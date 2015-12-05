// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood
#ifndef __NEOGEO_KOF2002_CART_H
#define __NEOGEO_KOF2002_CART_H

#include "neogeo_slot.h"
#include "banked_cart.h"
#include "pcm2_prot.h"
#include "cmc_prot.h"
#include "kof2002_prot.h"

// ======================> neogeo_kof2002_cart

class neogeo_kof2002_cart : public device_t,
						public device_neogeo_cart_interface
{
public:
	// construction/destruction
	neogeo_kof2002_cart(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT16 clock, const char *shortname, const char *source);
	neogeo_kof2002_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read_rom);

	virtual void activate_cart(ACTIVATE_CART_PARAMS) override { m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size); }
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override {}
	virtual int get_fixed_bank_type(void) override { return 0; }

	required_device<neogeo_banked_cart_device> m_banked_cart;
	required_device<cmc_prot_device> m_cmc_prot;
	required_device<pcm2_prot_device> m_pcm2_prot;
	required_device<kof2002_prot_device> m_kof2002_prot;

};



// device type definition
extern const device_type NEOGEO_KOF2002_CART;


/*************************************************
 KOF2002
**************************************************/

class neogeo_kof2002_kof2002_cart : public neogeo_kof2002_cart
{
public:
	neogeo_kof2002_kof2002_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }
};
extern const device_type NEOGEO_KOF2002_KOF2002_CART;

class neogeo_kof2002_kf2k2pls_cart : public neogeo_kof2002_cart
{
public:
	neogeo_kof2002_kf2k2pls_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }
};
extern const device_type NEOGEO_KOF2002_KF2K2PLS_CART;



/*************************************************
 MATRIM
**************************************************/

class neogeo_kof2002_matrim_cart : public neogeo_kof2002_cart
{
public:
	neogeo_kof2002_matrim_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 2; }
};
extern const device_type NEOGEO_KOF2002_MATRIM_CART;

/*************************************************
 SAMSHO5
**************************************************/

class neogeo_kof2002_samsho5_cart : public neogeo_kof2002_cart
{
public:
	neogeo_kof2002_samsho5_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 1; }
};
extern const device_type NEOGEO_KOF2002_SAMSHO5_CART;

/*************************************************
 SAMSHO5SP
**************************************************/

class neogeo_kof2002_samsho5sp_cart : public neogeo_kof2002_cart
{
public:
	neogeo_kof2002_samsho5sp_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 1; }
};
extern const device_type NEOGEO_KOF2002_SAMSHO5SP_CART;


#endif
