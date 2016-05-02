// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#ifndef __NEOGEO_BOOTKOF2K3_H
#define __NEOGEO_BOOTKOF2K3_H

#include "slot.h"
#include "boot_misc.h"
#include "prot_kof2k3bl.h"


/*************************************************
 kf2k3bl
 **************************************************/

class neogeo_kf2k3bl_cart : public neogeo_bootleg_cart
{
public:
	neogeo_kf2k3bl_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }
	
	virtual machine_config_constructor device_mconfig_additions() const override;
	
	virtual UINT32 get_bank_base(UINT16 sel) override { return m_kof2k3bl_prot->get_bank_base(); }
	virtual DECLARE_READ16_MEMBER(protection_r) override { return m_kof2k3bl_prot->protection_r(space, offset, mem_mask); }
	virtual DECLARE_WRITE16_MEMBER(protection_w) override { m_kof2k3bl_prot->kof2003_w(space, offset, data, mem_mask); }
	virtual DECLARE_READ16_MEMBER(addon_r) override { return m_kof2k3bl_prot->overlay_r(space, offset, mem_mask); }

private:
	required_device<cmc_prot_device> m_cmc_prot;
	required_device<pcm2_prot_device> m_pcm2_prot;
	required_device<kof2k3bl_prot_device> m_kof2k3bl_prot;
};

extern const device_type NEOGEO_KF2K3BL_CART;


/*************************************************
 kf2k3pl
 **************************************************/

class neogeo_kf2k3pl_cart : public neogeo_bootleg_cart
{
public:
	neogeo_kf2k3pl_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }
	
	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual UINT32 get_bank_base(UINT16 sel) override { return m_kof2k3bl_prot->get_bank_base(); }
	virtual DECLARE_READ16_MEMBER(protection_r) override { return m_kof2k3bl_prot->protection_r(space, offset, mem_mask); }
	virtual DECLARE_WRITE16_MEMBER(protection_w) override { m_kof2k3bl_prot->kof2003p_w(space, offset, data, mem_mask); }

private:
	required_device<cmc_prot_device> m_cmc_prot;
	required_device<pcm2_prot_device> m_pcm2_prot;
	required_device<kof2k3bl_prot_device> m_kof2k3bl_prot;
};

extern const device_type NEOGEO_KF2K3PL_CART;


/*************************************************
 kf2k3upl
 **************************************************/

class neogeo_kf2k3upl_cart : public neogeo_bootleg_cart
{
public:
	neogeo_kf2k3upl_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }
	
	virtual machine_config_constructor device_mconfig_additions() const override;
	
	virtual UINT32 get_bank_base(UINT16 sel) override { return m_kof2k3bl_prot->get_bank_base(); }
	virtual DECLARE_READ16_MEMBER(protection_r) override { return m_kof2k3bl_prot->protection_r(space, offset, mem_mask); }
	virtual DECLARE_WRITE16_MEMBER(protection_w) override { m_kof2k3bl_prot->kof2003_w(space, offset, data, mem_mask); }
	virtual DECLARE_READ16_MEMBER(addon_r) override { return m_kof2k3bl_prot->overlay_r(space, offset, mem_mask); }

private:
	required_device<cmc_prot_device> m_cmc_prot;
	required_device<pcm2_prot_device> m_pcm2_prot;
	required_device<kof2k3bl_prot_device> m_kof2k3bl_prot;
};

extern const device_type NEOGEO_KF2K3UPL_CART;



#endif
