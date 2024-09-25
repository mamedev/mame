// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#ifndef MAME_BUS_NEOGEO_BOOT_MISC_H
#define MAME_BUS_NEOGEO_BOOT_MISC_H

#include "slot.h"
#include "rom.h"
#include "prot_misc.h"
#include "prot_cmc.h"
#include "prot_pcm2.h"

// ======================> neogeo_bootleg_cart_device

class neogeo_bootleg_cart_device : public neogeo_rom_device
{
public:
	// construction/destruction
	neogeo_bootleg_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock);

	// reading and writing
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override { }
	virtual int get_fixed_bank_type() override { return 0; }

protected:
	neogeo_bootleg_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint16_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<neoboot_prot_device> m_prot;
};

// device type definition
DECLARE_DEVICE_TYPE(NEOGEO_BOOTLEG_CART, neogeo_bootleg_cart_device)


/*************************************************
 garoubl
**************************************************/

class neogeo_garoubl_cart_device : public neogeo_bootleg_cart_device
{
public:
	neogeo_garoubl_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }
};

DECLARE_DEVICE_TYPE(NEOGEO_GAROUBL_CART, neogeo_garoubl_cart_device)


/*************************************************
 kof97oro
 **************************************************/

class neogeo_kof97oro_cart_device : public neogeo_bootleg_cart_device
{
public:
	neogeo_kof97oro_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }
};

DECLARE_DEVICE_TYPE(NEOGEO_KOF97ORO_CART, neogeo_kof97oro_cart_device)


/*************************************************
 kf10thep
**************************************************/

class neogeo_kf10thep_cart_device : public neogeo_bootleg_cart_device
{
public:
	neogeo_kf10thep_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }
};

DECLARE_DEVICE_TYPE(NEOGEO_KF10THEP_CART, neogeo_kf10thep_cart_device)


/*************************************************
 kf2k5uni
**************************************************/

class neogeo_kf2k5uni_cart_device : public neogeo_bootleg_cart_device
{
public:
	neogeo_kf2k5uni_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }
};

DECLARE_DEVICE_TYPE(NEOGEO_KF2K5UNI_CART, neogeo_kf2k5uni_cart_device)

/*************************************************
 kf2k4se
**************************************************/

class neogeo_kf2k4se_cart_device : public neogeo_bootleg_cart_device
{
public:
	neogeo_kf2k4se_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }
};

DECLARE_DEVICE_TYPE(NEOGEO_KF2K4SE_CART, neogeo_kf2k4se_cart_device)


/*************************************************
 lans2004
 **************************************************/

class neogeo_lans2004_cart_device : public neogeo_bootleg_cart_device
{
public:
	neogeo_lans2004_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }
};

DECLARE_DEVICE_TYPE(NEOGEO_LANS2004_CART, neogeo_lans2004_cart_device)


/*************************************************
 samsho5b
**************************************************/

class neogeo_samsho5b_cart_device : public neogeo_bootleg_cart_device
{
public:
	neogeo_samsho5b_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }
};

DECLARE_DEVICE_TYPE(NEOGEO_SAMSHO5B_CART, neogeo_samsho5b_cart_device)


/*************************************************
 mslug3b6
 **************************************************/

class neogeo_mslug3b6_cart_device : public neogeo_bootleg_cart_device
{
public:
	neogeo_mslug3b6_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<cmc_prot_device> m_cmc_prot;
};

DECLARE_DEVICE_TYPE(NEOGEO_MSLUG3B6_CART, neogeo_mslug3b6_cart_device)


/*************************************************
 ms5plus
 **************************************************/

class neogeo_ms5plus_cart_device : public neogeo_bootleg_cart_device
{
public:
	neogeo_ms5plus_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }

	virtual uint16_t protection_r(address_space &space, offs_t offset) override { return m_prot->mslug5p_prot_r(); }
	virtual uint32_t get_bank_base(uint16_t sel) override { return m_prot->mslug5p_bank_base(sel); }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<cmc_prot_device> m_cmc_prot;
	required_device<pcm2_prot_device> m_pcm2_prot;
};

DECLARE_DEVICE_TYPE(NEOGEO_MS5PLUS_CART, neogeo_ms5plus_cart_device)


/*************************************************
 mslug5b
 **************************************************/

class neogeo_mslug5b_cart_device : public neogeo_bootleg_cart_device
{
public:
	neogeo_mslug5b_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }
};

DECLARE_DEVICE_TYPE(NEOGEO_MSLUG5B_CART, neogeo_mslug5b_cart_device)


/*************************************************
 kog
**************************************************/

class neogeo_kog_cart_device : public neogeo_bootleg_cart_device
{
public:
	neogeo_kog_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint16_t protection_r(address_space &space, offs_t offset) override;
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }

private:
	required_ioport m_jumper;
};

DECLARE_DEVICE_TYPE(NEOGEO_KOG_CART, neogeo_kog_cart_device)


#endif // MAME_BUS_NEOGEO_BOOT_MISC_H
