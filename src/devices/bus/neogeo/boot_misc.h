// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#ifndef __NEOGEO_BOOTCART_H
#define __NEOGEO_BOOTCART_H

#include "slot.h"
#include "rom.h"
#include "prot_misc.h"
#include "prot_cmc.h"
#include "prot_pcm2.h"

// ======================> neogeo_bootleg_cart

class neogeo_bootleg_cart : public neogeo_rom_device
{
public:
	// construction/destruction
	neogeo_bootleg_cart(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT16 clock, const char *shortname, const char *source);
	neogeo_bootleg_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing

	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override {}
	virtual int get_fixed_bank_type(void) override { return 0; }

	required_device<neoboot_prot_device> m_prot;
};

// device type definition
extern const device_type NEOGEO_BOOTLEG_CART;


/*************************************************
 garoubl
**************************************************/

class neogeo_garoubl_cart : public neogeo_bootleg_cart
{
public:
	neogeo_garoubl_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }
};

extern const device_type NEOGEO_GAROUBL_CART;


/*************************************************
 kof97oro
 **************************************************/

class neogeo_kof97oro_cart : public neogeo_bootleg_cart
{
public:
	neogeo_kof97oro_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }
};

extern const device_type NEOGEO_KOF97ORO_CART;


/*************************************************
 kf10thep
**************************************************/

class neogeo_kf10thep_cart : public neogeo_bootleg_cart
{
public:
	neogeo_kf10thep_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }
};

extern const device_type NEOGEO_KF10THEP_CART;


/*************************************************
 kf2k5uni
**************************************************/

class neogeo_kf2k5uni_cart : public neogeo_bootleg_cart
{
public:
	neogeo_kf2k5uni_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }
};

extern const device_type NEOGEO_KF2K5UNI_CART;

/*************************************************
 kf2k4se
**************************************************/

class neogeo_kf2k4se_cart : public neogeo_bootleg_cart
{
public:
	neogeo_kf2k4se_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }
};

extern const device_type NEOGEO_KF2K4SE_CART;


/*************************************************
 lans2004
 **************************************************/

class neogeo_lans2004_cart : public neogeo_bootleg_cart
{
public:
	neogeo_lans2004_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }
};

extern const device_type NEOGEO_LANS2004_CART;


/*************************************************
 samsho5b
**************************************************/

class neogeo_samsho5b_cart : public neogeo_bootleg_cart
{
public:
	neogeo_samsho5b_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }
};

extern const device_type NEOGEO_SAMSHO5B_CART;


/*************************************************
 mslug3b6
 **************************************************/

class neogeo_mslug3b6_cart : public neogeo_bootleg_cart
{
public:
	neogeo_mslug3b6_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }

	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	required_device<cmc_prot_device> m_cmc_prot;
};

extern const device_type NEOGEO_MSLUG3B6_CART;

/*************************************************
 ms5plus
 **************************************************/

class neogeo_ms5plus_cart : public neogeo_bootleg_cart
{
public:
	neogeo_ms5plus_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 1; }

	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual DECLARE_READ16_MEMBER(protection_r) override { return m_prot->mslug5p_prot_r(space, offset, mem_mask); }
	virtual UINT32 get_bank_base(UINT16 sel) override { return m_prot->mslug5p_bank_base(sel); }

private:
	required_device<cmc_prot_device> m_cmc_prot;
	required_device<pcm2_prot_device> m_pcm2_prot;
};

extern const device_type NEOGEO_MS5PLUS_CART;


/*************************************************
 kog
**************************************************/

class neogeo_kog_cart : public neogeo_bootleg_cart
{
public:
	neogeo_kog_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual ioport_constructor device_input_ports() const override;

	virtual DECLARE_READ16_MEMBER(protection_r) override;
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }

private:
	required_ioport m_jumper;
};

extern const device_type NEOGEO_KOG_CART;


#endif
