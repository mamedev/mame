// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#ifndef MAME_BUS_NEOGEO_CMC_H
#define MAME_BUS_NEOGEO_CMC_H

#pragma once

#include "slot.h"
#include "rom.h"
#include "prot_cmc.h"
#include "machine/nvram.h"

// ======================> neogeo_cmc_cart_device

class neogeo_cmc_cart_device : public neogeo_rom_device
{
public:
	// construction/destruction
	neogeo_cmc_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock);

	// reading and writing
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override { }
	virtual int get_fixed_bank_type() override { return 0; }

protected:
	neogeo_cmc_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint16_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<cmc_prot_device> m_prot;
};

// device type definition
DECLARE_DEVICE_TYPE(NEOGEO_CMC_CART, neogeo_cmc_cart_device)


/*************************************************
 zupapa
**************************************************/

class neogeo_cmc_zupapa_cart_device : public neogeo_cmc_cart_device
{
public:
	neogeo_cmc_zupapa_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }
};

DECLARE_DEVICE_TYPE(NEOGEO_CMC_ZUPAPA_CART, neogeo_cmc_zupapa_cart_device)


/*************************************************
 mslug3h
**************************************************/

class neogeo_cmc_mslug3h_cart_device : public neogeo_cmc_cart_device
{
public:
	neogeo_cmc_mslug3h_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }
};

DECLARE_DEVICE_TYPE(NEOGEO_CMC_MSLUG3H_CART, neogeo_cmc_mslug3h_cart_device)


/*************************************************
 ganryu
**************************************************/

class neogeo_cmc_ganryu_cart_device : public neogeo_cmc_cart_device
{
public:
	neogeo_cmc_ganryu_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }
};

DECLARE_DEVICE_TYPE(NEOGEO_CMC_GANRYU_CART, neogeo_cmc_ganryu_cart_device)


/*************************************************
 s1945p
**************************************************/

class neogeo_cmc_s1945p_cart_device : public neogeo_cmc_cart_device
{
public:
	neogeo_cmc_s1945p_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }
};

DECLARE_DEVICE_TYPE(NEOGEO_CMC_S1945P_CART, neogeo_cmc_s1945p_cart_device)


/*************************************************
 preisle2
**************************************************/

class neogeo_cmc_preisle2_cart_device : public neogeo_cmc_cart_device
{
public:
	neogeo_cmc_preisle2_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }
};

DECLARE_DEVICE_TYPE(NEOGEO_CMC_PREISLE2_CART, neogeo_cmc_preisle2_cart_device)


/*************************************************
 bangbead
**************************************************/

class neogeo_cmc_bangbead_cart_device : public neogeo_cmc_cart_device
{
public:
	neogeo_cmc_bangbead_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }
};

DECLARE_DEVICE_TYPE(NEOGEO_CMC_BANGBEAD_CART, neogeo_cmc_bangbead_cart_device)


/*************************************************
 nitd
**************************************************/

class neogeo_cmc_nitd_cart_device : public neogeo_cmc_cart_device
{
public:
	neogeo_cmc_nitd_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }
};

DECLARE_DEVICE_TYPE(NEOGEO_CMC_NITD_CART, neogeo_cmc_nitd_cart_device)


/*************************************************
 sengoku3
**************************************************/

class neogeo_cmc_sengoku3_cart_device : public neogeo_cmc_cart_device
{
public:
	neogeo_cmc_sengoku3_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }
};

DECLARE_DEVICE_TYPE(NEOGEO_CMC_SENGOKU3_CART, neogeo_cmc_sengoku3_cart_device)


/*************************************************
 kof99k
**************************************************/

class neogeo_cmc_kof99k_cart_device : public neogeo_cmc_cart_device
{
public:
	neogeo_cmc_kof99k_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }
};

DECLARE_DEVICE_TYPE(NEOGEO_CMC_KOF99K_CART, neogeo_cmc_kof99k_cart_device)


/*************************************************
 kof2001
**************************************************/

class neogeo_cmc_kof2001_cart_device : public neogeo_cmc_cart_device
{
public:
	neogeo_cmc_kof2001_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }
};

DECLARE_DEVICE_TYPE(NEOGEO_CMC_KOF2001_CART, neogeo_cmc_kof2001_cart_device)


/*************************************************
 kof2000n
**************************************************/

class neogeo_cmc_kof2000n_cart_device : public neogeo_cmc_cart_device
{
public:
	neogeo_cmc_kof2000n_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 2; }
};

DECLARE_DEVICE_TYPE(NEOGEO_CMC_KOF2000N_CART, neogeo_cmc_kof2000n_cart_device)


/*************************************************
 jockeygp
 **************************************************/

class neogeo_cmc_jockeygp_cart_device : public neogeo_cmc_cart_device
{
public:
	neogeo_cmc_jockeygp_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }

	virtual uint16_t ram_r(offs_t offset) override { return m_ram[offset]; }
	virtual void ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override { COMBINE_DATA(&m_ram[offset]); }

protected:
	virtual void device_start() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	std::unique_ptr<uint16_t[]> m_ram;

	required_device<nvram_device> m_nvram;
};

DECLARE_DEVICE_TYPE(NEOGEO_CMC_JOCKEYGP_CART, neogeo_cmc_jockeygp_cart_device)

#endif // MAME_BUS_NEOGEO_CMC_H
