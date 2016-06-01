// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#ifndef __NEOGEO_BOOTKOF2K2_H
#define __NEOGEO_BOOTKOF2K2_H

#include "slot.h"
#include "boot_misc.h"
#include "prot_misc.h"
#include "prot_cmc.h"
#include "prot_pcm2.h"
#include "prot_kof2k2.h"


/*************************************************
 kof2002b
 **************************************************/

class neogeo_kof2002b_cart : public neogeo_bootleg_cart
{
public:
	neogeo_kof2002b_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	required_device<cmc_prot_device> m_cmc_prot;
	required_device<pcm2_prot_device> m_pcm2_prot;
	required_device<kof2002_prot_device> m_kof2k2_prot;
};

extern const device_type NEOGEO_KOF2002B_CART;


/*************************************************
 kf2k2mp
 **************************************************/

class neogeo_kf2k2mp_cart : public neogeo_bootleg_cart
{
public:
	neogeo_kf2k2mp_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	required_device<cmc_prot_device> m_cmc_prot;
	required_device<pcm2_prot_device> m_pcm2_prot;
};

extern const device_type NEOGEO_KF2K2MP_CART;


/*************************************************
 kf2k2mp2
 **************************************************/

class neogeo_kf2k2mp2_cart : public neogeo_bootleg_cart
{
public:
	neogeo_kf2k2mp2_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	required_device<cmc_prot_device> m_cmc_prot;
	required_device<pcm2_prot_device> m_pcm2_prot;
};

extern const device_type NEOGEO_KF2K2MP2_CART;



#endif
