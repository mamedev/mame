// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#ifndef MAME_BUS_NEOGEO_PVC_H
#define MAME_BUS_NEOGEO_PVC_H

#pragma once

#include "slot.h"
#include "rom.h"
#include "prot_pcm2.h"
#include "prot_cmc.h"
#include "prot_pvc.h"

// ======================> neogeo_pvc_cart_device

class neogeo_pvc_cart_device : public neogeo_rom_device
{
public:
	// construction/destruction
	neogeo_pvc_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock);

	// reading and writing
	virtual uint32_t get_bank_base(uint16_t sel) override { return m_pvc_prot->get_bank_base(); }
	virtual uint16_t protection_r(address_space &space, offs_t offset) override { return m_pvc_prot->protection_r(offset); }
	virtual void protection_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override { m_pvc_prot->protection_w(offset, data, mem_mask); }

	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override { }
	virtual int get_fixed_bank_type() override { return 0; }

protected:
	neogeo_pvc_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint16_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<cmc_prot_device> m_cmc_prot;
	required_device<pcm2_prot_device> m_pcm2_prot;
	required_device<pvc_prot_device> m_pvc_prot;

};

// device type definition
DECLARE_DEVICE_TYPE(NEOGEO_PVC_CART, neogeo_pvc_cart_device)


/*************************************************
 mslug5
**************************************************/

class neogeo_pvc_mslug5_cart_device : public neogeo_pvc_cart_device
{
public:
	neogeo_pvc_mslug5_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }
};

DECLARE_DEVICE_TYPE(NEOGEO_PVC_MSLUG5_CART, neogeo_pvc_mslug5_cart_device)


/*************************************************
 svc
**************************************************/

class neogeo_pvc_svc_cart_device : public neogeo_pvc_cart_device
{
public:
	neogeo_pvc_svc_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 2; }
};

DECLARE_DEVICE_TYPE(NEOGEO_PVC_SVC_CART, neogeo_pvc_svc_cart_device)


/*************************************************
 kof2003
**************************************************/

class neogeo_pvc_kof2003_cart_device : public neogeo_pvc_cart_device
{
public:
	neogeo_pvc_kof2003_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 2; }
};

DECLARE_DEVICE_TYPE(NEOGEO_PVC_KOF2003_CART, neogeo_pvc_kof2003_cart_device)


/*************************************************
 kof2003h
**************************************************/

class neogeo_pvc_kof2003h_cart_device : public neogeo_pvc_cart_device
{
public:
	neogeo_pvc_kof2003h_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 2; }
};

DECLARE_DEVICE_TYPE(NEOGEO_PVC_KOF2003H_CART, neogeo_pvc_kof2003h_cart_device)


#endif // MAME_BUS_NEOGEO_PVC_H
