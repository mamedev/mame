// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#ifndef __NEOGEO_KOF2002_H
#define __NEOGEO_KOF2002_H

#include "slot.h"
#include "rom.h"
#include "prot_pcm2.h"
#include "prot_cmc.h"
#include "prot_kof2k2.h"

// ======================> neogeo_kof2002_cart

class neogeo_kof2k2type_cart : public neogeo_rom_device
{
public:
	// construction/destruction
	neogeo_kof2k2type_cart(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT16 clock, const char *shortname, const char *source);
	neogeo_kof2k2type_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing

	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override {}
	virtual int get_fixed_bank_type(void) override { return 0; }

	required_device<cmc_prot_device> m_cmc_prot;
	required_device<pcm2_prot_device> m_pcm2_prot;
	required_device<kof2002_prot_device> m_kof2k2_prot;

};


// device type definition
extern const device_type NEOGEO_K2K2_CART;


/*************************************************
 kof2002
**************************************************/

class neogeo_kof2002_cart : public neogeo_kof2k2type_cart
{
public:
	neogeo_kof2002_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }
};

extern const device_type NEOGEO_K2K2_KOF2002_CART;



class neogeo_kf2k2pls_cart : public neogeo_kof2k2type_cart
{
public:
	neogeo_kf2k2pls_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }
};

extern const device_type NEOGEO_K2K2_KF2K2PLS_CART;


/*************************************************
 matrim
**************************************************/

class neogeo_matrim_cart : public neogeo_kof2k2type_cart
{
public:
	neogeo_matrim_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 2; }
};

extern const device_type NEOGEO_K2K2_MATRIM_CART;


/*************************************************
 samsho5
**************************************************/

class neogeo_samsho5_cart : public neogeo_kof2k2type_cart
{
public:
	neogeo_samsho5_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 1; }
};

extern const device_type NEOGEO_K2K2_SAMSHO5_CART;


/*************************************************
 samsho5sp
**************************************************/

class neogeo_samsho5sp_cart : public neogeo_kof2k2type_cart
{
public:
	neogeo_samsho5sp_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 1; }
};

extern const device_type NEOGEO_K2K2_SAMSHO5SP_CART;


#endif
