// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#ifndef __NEOGEO_PVC_H
#define __NEOGEO_PVC_H

#include "slot.h"
#include "rom.h"
#include "prot_pcm2.h"
#include "prot_cmc.h"
#include "prot_pvc.h"

// ======================> neogeo_pvc_cart

class neogeo_pvc_cart : public neogeo_rom_device
{
public:
	// construction/destruction
	neogeo_pvc_cart(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT16 clock, const char *shortname, const char *source);
	neogeo_pvc_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual UINT32 get_bank_base(UINT16 sel) override { return m_pvc_prot->get_bank_base(); }
	virtual DECLARE_READ16_MEMBER(protection_r) override { return m_pvc_prot->protection_r(space, offset, mem_mask); }
	virtual DECLARE_WRITE16_MEMBER(protection_w) override { m_pvc_prot->protection_w(space, offset, data, mem_mask); }

	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override {}
	virtual int get_fixed_bank_type(void) override { return 0; }

	required_device<cmc_prot_device> m_cmc_prot;
	required_device<pcm2_prot_device> m_pcm2_prot;
	required_device<pvc_prot_device> m_pvc_prot;

};

// device type definition
extern const device_type NEOGEO_PVC_CART;


/*************************************************
 mslug5
**************************************************/

class neogeo_pvc_mslug5_cart : public neogeo_pvc_cart
{
public:
	neogeo_pvc_mslug5_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 1; }
};

extern const device_type NEOGEO_PVC_MSLUG5_CART;


/*************************************************
 svc
**************************************************/

class neogeo_pvc_svc_cart : public neogeo_pvc_cart
{
public:
	neogeo_pvc_svc_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 2; }
};

extern const device_type NEOGEO_PVC_SVC_CART;


/*************************************************
 kof2003
**************************************************/

class neogeo_pvc_kof2003_cart : public neogeo_pvc_cart
{
public:
	neogeo_pvc_kof2003_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 2; }
};

extern const device_type NEOGEO_PVC_KOF2003_CART;


/*************************************************
 kof2003h
**************************************************/

class neogeo_pvc_kof2003h_cart : public neogeo_pvc_cart
{
public:
	neogeo_pvc_kof2003h_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 2; }
};
extern const device_type NEOGEO_PVC_KOF2003H_CART;



#endif
