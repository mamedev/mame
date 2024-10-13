// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#ifndef MAME_BUS_NEOGEO_BOOT_KOF2K2_H
#define MAME_BUS_NEOGEO_BOOT_KOF2K2_H

#pragma once

#include "slot.h"
#include "boot_misc.h"
#include "prot_misc.h"
#include "prot_cmc.h"
#include "prot_pcm2.h"
#include "prot_kof2k2.h"


/*************************************************
 kof2002b
 **************************************************/

class neogeo_kof2002b_cart_device : public neogeo_bootleg_cart_device
{
public:
	neogeo_kof2002b_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<cmc_prot_device> m_cmc_prot;
	required_device<pcm2_prot_device> m_pcm2_prot;
	required_device<kof2002_prot_device> m_kof2k2_prot;
};

DECLARE_DEVICE_TYPE(NEOGEO_KOF2002B_CART, neogeo_kof2002b_cart_device)


/*************************************************
 kf2k2mp
 **************************************************/

class neogeo_kf2k2mp_cart_device : public neogeo_bootleg_cart_device
{
public:
	neogeo_kf2k2mp_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<cmc_prot_device> m_cmc_prot;
	required_device<pcm2_prot_device> m_pcm2_prot;
};

DECLARE_DEVICE_TYPE(NEOGEO_KF2K2MP_CART, neogeo_kf2k2mp_cart_device)


/*************************************************
 kf2k2mp2
 **************************************************/

class neogeo_kf2k2mp2_cart_device : public neogeo_bootleg_cart_device
{
public:
	neogeo_kf2k2mp2_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<cmc_prot_device> m_cmc_prot;
	required_device<pcm2_prot_device> m_pcm2_prot;
};

DECLARE_DEVICE_TYPE(NEOGEO_KF2K2MP2_CART, neogeo_kf2k2mp2_cart_device)



#endif // MAME_BUS_NEOGEO_BOOT_KOF2K2_H
