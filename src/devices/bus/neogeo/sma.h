// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#ifndef MAME_BUS_NEOGEO_SMA_H
#define MAME_BUS_NEOGEO_SMA_H

#include "slot.h"
#include "rom.h"
#include "prot_sma.h"
#include "prot_cmc.h"

// ======================> neogeo_sma_cart_device

class neogeo_sma_cart_device : public neogeo_rom_device
{
public:
	// construction/destruction
	neogeo_sma_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock);

	// reading and writing
	virtual uint16_t protection_r(address_space &space, offs_t offset) override { return m_sma_prot->prot_9a37_r(); }
	virtual uint16_t addon_r(offs_t offset) override { return m_sma_prot->random_r(); }
	virtual uint32_t get_bank_base(uint16_t sel) override { return 0; }

	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override {}
	virtual int get_fixed_bank_type() override { return 0; }

protected:
	neogeo_sma_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint16_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<sma_prot_device> m_sma_prot;
	required_device<cmc_prot_device> m_cmc_prot;
};

// device type definition
DECLARE_DEVICE_TYPE(NEOGEO_SMA_CART, neogeo_sma_cart_device)


/*************************************************
 kof99
**************************************************/

class neogeo_sma_kof99_cart_device : public neogeo_sma_cart_device
{
public:
	neogeo_sma_kof99_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }
	virtual uint32_t get_bank_base(uint16_t sel) override { return m_sma_prot->kof99_bank_base(sel); }
};

DECLARE_DEVICE_TYPE(NEOGEO_SMA_KOF99_CART, neogeo_sma_kof99_cart_device)


/*************************************************
 garou
**************************************************/

class neogeo_sma_garou_cart_device : public neogeo_sma_cart_device
{
public:
	neogeo_sma_garou_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }
	virtual uint32_t get_bank_base(uint16_t sel) override { return m_sma_prot->garou_bank_base(sel); }
};

DECLARE_DEVICE_TYPE(NEOGEO_SMA_GAROU_CART, neogeo_sma_garou_cart_device)


/*************************************************
 garouh
 **************************************************/

class neogeo_sma_garouh_cart_device : public neogeo_sma_cart_device
{
public:
	neogeo_sma_garouh_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }
	virtual uint32_t get_bank_base(uint16_t sel) override { return m_sma_prot->garouh_bank_base(sel); }
};

DECLARE_DEVICE_TYPE(NEOGEO_SMA_GAROUH_CART, neogeo_sma_garouh_cart_device)


/*************************************************
 mslug3
 **************************************************/

class neogeo_sma_mslug3_cart_device : public neogeo_sma_cart_device
{
public:
	neogeo_sma_mslug3_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }
	virtual uint32_t get_bank_base(uint16_t sel) override { return m_sma_prot->mslug3_bank_base(sel); }
};

DECLARE_DEVICE_TYPE(NEOGEO_SMA_MSLUG3_CART, neogeo_sma_mslug3_cart_device)

class neogeo_sma_mslug3a_cart_device : public neogeo_sma_cart_device
{
public:
	neogeo_sma_mslug3a_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }
	virtual uint32_t get_bank_base(uint16_t sel) override { return m_sma_prot->mslug3a_bank_base(sel); }
};

DECLARE_DEVICE_TYPE(NEOGEO_SMA_MSLUG3A_CART, neogeo_sma_mslug3a_cart_device)


/*************************************************
 kof2000
**************************************************/

class neogeo_sma_kof2000_cart_device : public neogeo_sma_cart_device
{
public:
	neogeo_sma_kof2000_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 2; }
	virtual uint32_t get_bank_base(uint16_t sel) override { return m_sma_prot->kof2000_bank_base(sel); }
};

DECLARE_DEVICE_TYPE(NEOGEO_SMA_KOF2000_CART, neogeo_sma_kof2000_cart_device)


#endif // MAME_BUS_NEOGEO_SMA_H
