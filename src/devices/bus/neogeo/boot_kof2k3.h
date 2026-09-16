// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#ifndef MAME_BUS_NEOGEO_BOOT_KOF2K3_H
#define MAME_BUS_NEOGEO_BOOT_KOF2K3_H

#pragma once

#include "slot.h"
#include "boot_misc.h"
#include "prot_kof2k3bl.h"


/*************************************************
 kf2k3bl
 **************************************************/

class neogeo_kf2k3bl_cart_device : public neogeo_bootleg_cart_device
{
public:
	neogeo_kf2k3bl_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }

	virtual uint32_t get_bank_base(uint16_t sel) override { return m_kof2k3bl_prot->get_bank_base(); }
	virtual uint16_t protection_r(address_space &space, offs_t offset) override { return m_kof2k3bl_prot->protection_r(offset); }
	virtual void protection_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override { m_kof2k3bl_prot->kof2003_w(offset, data, mem_mask); }
	virtual uint16_t addon_r(offs_t offset) override { return m_kof2k3bl_prot->overlay_r(); }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<cmc_prot_device> m_cmc_prot;
	required_device<pcm2_prot_device> m_pcm2_prot;
	required_device<kof2k3bl_prot_device> m_kof2k3bl_prot;
};

DECLARE_DEVICE_TYPE(NEOGEO_KF2K3BL_CART, neogeo_kf2k3bl_cart_device)


/*************************************************
 kf2k3pl
 **************************************************/

class neogeo_kf2k3pl_cart_device : public neogeo_bootleg_cart_device
{
public:
	neogeo_kf2k3pl_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }

	virtual uint32_t get_bank_base(uint16_t sel) override { return m_kof2k3bl_prot->get_bank_base(); }
	virtual uint16_t protection_r(address_space &space, offs_t offset) override { return m_kof2k3bl_prot->protection_r(offset); }
	virtual void protection_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override { m_kof2k3bl_prot->kof2003p_w(offset, data, mem_mask); }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<cmc_prot_device> m_cmc_prot;
	required_device<pcm2_prot_device> m_pcm2_prot;
	required_device<kof2k3bl_prot_device> m_kof2k3bl_prot;
};

DECLARE_DEVICE_TYPE(NEOGEO_KF2K3PL_CART, neogeo_kf2k3pl_cart_device)


/*************************************************
 kf2k3upl
 **************************************************/

class neogeo_kf2k3upl_cart_device : public neogeo_bootleg_cart_device
{
public:
	neogeo_kf2k3upl_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }

	virtual uint32_t get_bank_base(uint16_t sel) override { return m_kof2k3bl_prot->get_bank_base(); }
	virtual uint16_t protection_r(address_space &space, offs_t offset) override { return m_kof2k3bl_prot->protection_r(offset); }
	virtual void protection_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override { m_kof2k3bl_prot->kof2003_w(offset, data, mem_mask); }
	virtual uint16_t addon_r(offs_t offset) override { return m_kof2k3bl_prot->overlay_r(); }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<cmc_prot_device> m_cmc_prot;
	required_device<pcm2_prot_device> m_pcm2_prot;
	required_device<kof2k3bl_prot_device> m_kof2k3bl_prot;
};

DECLARE_DEVICE_TYPE(NEOGEO_KF2K3UPL_CART, neogeo_kf2k3upl_cart_device)



#endif // MAME_BUS_NEOGEO_BOOT_KOF2K3_H
