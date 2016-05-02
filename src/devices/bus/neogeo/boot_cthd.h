// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#ifndef __NEOGEO_BOOTCTHD_H
#define __NEOGEO_BOOTCTHD_H

#include "slot.h"
#include "rom.h"
#include "prot_cthd.h"

// ======================> neogeo_cthd2k3_cart

class neogeo_cthd2k3_cart : public neogeo_rom_device
{
public:
	// construction/destruction
	neogeo_cthd2k3_cart(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT16 clock, const char *shortname, const char *source);
	neogeo_cthd2k3_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual UINT32 get_bank_base(UINT16 sel) override { return m_prot->get_bank_base(sel); }
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }

protected:
	required_device<cthd_prot_device> m_prot;
};

// device type definition
extern const device_type NEOGEO_CTHD2K3_CART;



/*************************************************
 ct2k3sp
 **************************************************/

class neogeo_ct2k3sp_cart : public neogeo_cthd2k3_cart
{
public:
	neogeo_ct2k3sp_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }
};

extern const device_type NEOGEO_CT2K3SP_CART;


/*************************************************
 ct2k3sa
 **************************************************/

class neogeo_ct2k3sa_cart : public neogeo_cthd2k3_cart
{
public:
	neogeo_ct2k3sa_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }
};

extern const device_type NEOGEO_CT2K3SA_CART;


/*************************************************
 matrimbl
 **************************************************/

#include "prot_cmc.h"
#include "prot_kof2k2.h"

class neogeo_matrimbl_cart : public neogeo_cthd2k3_cart
{
public:
	neogeo_matrimbl_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 2; }

	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	required_device<cmc_prot_device> m_cmc_prot;
	required_device<kof2002_prot_device> m_kof2k2_prot;
};

extern const device_type NEOGEO_MATRIMBL_CART;


#endif
