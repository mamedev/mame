// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#ifndef __NEOGEO_BOOTSVC_H
#define __NEOGEO_BOOTSVC_H

#include "slot.h"
#include "boot_misc.h"
#include "prot_misc.h"
#include "prot_pvc.h"

/*************************************************
 svcboot
 **************************************************/

class neogeo_svcboot_cart : public neogeo_bootleg_cart
{
public:
	neogeo_svcboot_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual UINT32 get_bank_base(UINT16 sel) override { return m_pvc_prot->get_bank_base(); }
	virtual DECLARE_READ16_MEMBER(protection_r) override { return m_pvc_prot->protection_r(space, offset, mem_mask); }
	virtual DECLARE_WRITE16_MEMBER(protection_w) override { m_pvc_prot->protection_w(space, offset, data, mem_mask); }
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }

private:
	required_device<pvc_prot_device> m_pvc_prot;
};

extern const device_type NEOGEO_SVCBOOT_CART;

/*************************************************
 svcplus
**************************************************/

class neogeo_svcplus_cart : public neogeo_bootleg_cart
{
public:
	neogeo_svcplus_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }
};

extern const device_type NEOGEO_SVCPLUS_CART;


/*************************************************
 svcplusa
**************************************************/

class neogeo_svcplusa_cart : public neogeo_bootleg_cart
{
public:
	neogeo_svcplusa_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }
};

extern const device_type NEOGEO_SVCPLUSA_CART;


/*************************************************
 svcsplus
 **************************************************/

class neogeo_svcsplus_cart : public neogeo_bootleg_cart
{
public:
	neogeo_svcsplus_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual UINT32 get_bank_base(UINT16 sel) override { return m_pvc_prot->get_bank_base(); }
	virtual DECLARE_READ16_MEMBER(protection_r) override { return m_pvc_prot->protection_r(space, offset, mem_mask); }
	virtual DECLARE_WRITE16_MEMBER(protection_w) override { m_pvc_prot->protection_w(space, offset, data, mem_mask); }
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }

private:
	required_device<pvc_prot_device> m_pvc_prot;
};

extern const device_type NEOGEO_SVCSPLUS_CART;




#endif
