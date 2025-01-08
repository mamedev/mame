// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#ifndef MAME_BUS_NEOGEO_BOOT_SVC_H
#define MAME_BUS_NEOGEO_BOOT_SVC_H

#pragma once

#include "slot.h"
#include "boot_misc.h"
#include "prot_misc.h"
#include "prot_pvc.h"

/*************************************************
 svcboot
 **************************************************/

class neogeo_svcboot_cart_device : public neogeo_bootleg_cart_device
{
public:
	neogeo_svcboot_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint32_t get_bank_base(uint16_t sel) override { return m_pvc_prot->get_bank_base(); }
	virtual uint16_t protection_r(address_space &space, offs_t offset) override { return m_pvc_prot->protection_r(offset); }
	virtual void protection_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override { m_pvc_prot->protection_w(offset, data, mem_mask); }
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<pvc_prot_device> m_pvc_prot;
};

DECLARE_DEVICE_TYPE(NEOGEO_SVCBOOT_CART, neogeo_svcboot_cart_device)

/*************************************************
 svcplus
**************************************************/

class neogeo_svcplus_cart_device : public neogeo_bootleg_cart_device
{
public:
	neogeo_svcplus_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }
};

DECLARE_DEVICE_TYPE(NEOGEO_SVCPLUS_CART, neogeo_svcplus_cart_device)


/*************************************************
 svcplusa
**************************************************/

class neogeo_svcplusa_cart_device : public neogeo_bootleg_cart_device
{
public:
	neogeo_svcplusa_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }
};

DECLARE_DEVICE_TYPE(NEOGEO_SVCPLUSA_CART, neogeo_svcplusa_cart_device)


/*************************************************
 svcsplus
 **************************************************/

class neogeo_svcsplus_cart_device : public neogeo_bootleg_cart_device
{
public:
	neogeo_svcsplus_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint32_t get_bank_base(uint16_t sel) override { return m_pvc_prot->get_bank_base(); }
	virtual uint16_t protection_r(address_space &space, offs_t offset) override { return m_pvc_prot->protection_r(offset); }
	virtual void protection_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override { m_pvc_prot->protection_w(offset, data, mem_mask); }
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<pvc_prot_device> m_pvc_prot;
};

DECLARE_DEVICE_TYPE(NEOGEO_SVCSPLUS_CART, neogeo_svcsplus_cart_device)




#endif
