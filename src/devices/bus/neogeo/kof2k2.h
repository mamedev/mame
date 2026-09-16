// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#ifndef MAME_BUS_NEOGEO_KOF2K2_H
#define MAME_BUS_NEOGEO_KOF2K2_H

#pragma once

#include "slot.h"
#include "rom.h"
#include "prot_pcm2.h"
#include "prot_cmc.h"
#include "prot_kof2k2.h"

// ======================> neogeo_kof2002_cart_device

class neogeo_kof2k2type_cart_device : public neogeo_rom_device
{
public:
	// construction/destruction
	neogeo_kof2k2type_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock);

	// reading and writing
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override { }
	virtual int get_fixed_bank_type() override { return 0; }

protected:
	neogeo_kof2k2type_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint16_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<cmc_prot_device> m_cmc_prot;
	required_device<pcm2_prot_device> m_pcm2_prot;
	required_device<kof2002_prot_device> m_kof2k2_prot;
};


// device type definition
DECLARE_DEVICE_TYPE(NEOGEO_K2K2_CART, neogeo_kof2k2type_cart_device)


/*************************************************
 kof2002
**************************************************/

class neogeo_kof2002_cart_device : public neogeo_kof2k2type_cart_device
{
public:
	neogeo_kof2002_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }
};

DECLARE_DEVICE_TYPE(NEOGEO_K2K2_KOF2002_CART, neogeo_kof2002_cart_device)



class neogeo_kf2k2pls_cart_device : public neogeo_kof2k2type_cart_device
{
public:
	neogeo_kf2k2pls_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }
};

DECLARE_DEVICE_TYPE(NEOGEO_K2K2_KF2K2PLS_CART, neogeo_kf2k2pls_cart_device)


/*************************************************
 matrim
**************************************************/

class neogeo_matrim_cart_device : public neogeo_kof2k2type_cart_device
{
public:
	neogeo_matrim_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 2; }
};

DECLARE_DEVICE_TYPE(NEOGEO_K2K2_MATRIM_CART, neogeo_matrim_cart_device)


/*************************************************
 samsho5
**************************************************/

class neogeo_samsho5_cart_device : public neogeo_kof2k2type_cart_device
{
public:
	neogeo_samsho5_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }
};

DECLARE_DEVICE_TYPE(NEOGEO_K2K2_SAMSHO5_CART, neogeo_samsho5_cart_device)


/*************************************************
 samsho5sp
**************************************************/

class neogeo_samsho5sp_cart_device : public neogeo_kof2k2type_cart_device
{
public:
	neogeo_samsho5sp_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }
};

DECLARE_DEVICE_TYPE(NEOGEO_K2K2_SAMSHO5SP_CART, neogeo_samsho5sp_cart_device)


#endif // MAME_BUS_NEOGEO_KOF2K2_H
