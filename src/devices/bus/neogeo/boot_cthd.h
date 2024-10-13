// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#ifndef MAME_BUS_NEOGEO_BOOT_CTHD_H
#define MAME_BUS_NEOGEO_BOOT_CTHD_H

#pragma once

#include "slot.h"
#include "prot_cmc.h"
#include "prot_cthd.h"
#include "prot_kof2k2.h"
#include "rom.h"


// ======================> neogeo_cthd2k3_cart_device

class neogeo_cthd2k3_cart_device : public neogeo_rom_device
{
public:
	// construction/destruction
	neogeo_cthd2k3_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock);

	// reading and writing
	virtual uint32_t get_bank_base(uint16_t sel) override { return m_prot->get_bank_base(sel); }
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }

protected:
	neogeo_cthd2k3_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint16_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<cthd_prot_device> m_prot;
};

// device type definition
DECLARE_DEVICE_TYPE(NEOGEO_CTHD2K3_CART, neogeo_cthd2k3_cart_device)



/*************************************************
 ct2k3sp
 **************************************************/

class neogeo_ct2k3sp_cart_device : public neogeo_cthd2k3_cart_device
{
public:
	neogeo_ct2k3sp_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }
};

DECLARE_DEVICE_TYPE(NEOGEO_CT2K3SP_CART, neogeo_ct2k3sp_cart_device)


/*************************************************
 ct2k3sa
 **************************************************/

class neogeo_ct2k3sa_cart_device : public neogeo_cthd2k3_cart_device
{
public:
	neogeo_ct2k3sa_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }
};

DECLARE_DEVICE_TYPE(NEOGEO_CT2K3SA_CART, neogeo_ct2k3sa_cart_device)


/*************************************************
 matrimbl
 **************************************************/

class neogeo_matrimbl_cart_device : public neogeo_cthd2k3_cart_device
{
public:
	neogeo_matrimbl_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 2; }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<cmc_prot_device> m_cmc_prot;
	required_device<kof2002_prot_device> m_kof2k2_prot;
};

DECLARE_DEVICE_TYPE(NEOGEO_MATRIMBL_CART, neogeo_matrimbl_cart_device)


#endif // MAME_BUS_NEOGEO_BOOT_CTHD_H
