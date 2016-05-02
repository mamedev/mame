// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#ifndef __NEOGEO_SMA_H
#define __NEOGEO_SMA_H

#include "slot.h"
#include "rom.h"
#include "prot_sma.h"
#include "prot_cmc.h"

// ======================> neogeo_sma_cart

class neogeo_sma_cart : public neogeo_rom_device
{
public:
	// construction/destruction
	neogeo_sma_cart(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT16 clock, const char *shortname, const char *source);
	neogeo_sma_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(protection_r) override { return m_sma_prot->prot_9a37_r(space, offset, mem_mask); }
	virtual DECLARE_READ16_MEMBER(addon_r) override { return m_sma_prot->random_r(space, offset, mem_mask); }
	virtual UINT32 get_bank_base(UINT16 sel) override { return 0; }

	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override {}
	virtual int get_fixed_bank_type(void) override { return 0; }

	required_device<sma_prot_device> m_sma_prot;
	required_device<cmc_prot_device> m_cmc_prot;
};



// device type definition
extern const device_type NEOGEO_SMA_CART;


/*************************************************
 kof99
**************************************************/

class neogeo_sma_kof99_cart : public neogeo_sma_cart
{
public:
	neogeo_sma_kof99_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 1; }
	virtual UINT32 get_bank_base(UINT16 sel) override { return m_sma_prot->kof99_bank_base(sel); }
};

extern const device_type NEOGEO_SMA_KOF99_CART;


/*************************************************
 garou
**************************************************/

class neogeo_sma_garou_cart : public neogeo_sma_cart
{
public:
	neogeo_sma_garou_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 1; }
	virtual UINT32 get_bank_base(UINT16 sel) override { return m_sma_prot->garou_bank_base(sel); }
};

extern const device_type NEOGEO_SMA_GAROU_CART;


/*************************************************
 garouh
 **************************************************/

class neogeo_sma_garouh_cart : public neogeo_sma_cart
{
public:
	neogeo_sma_garouh_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 1; }
	virtual UINT32 get_bank_base(UINT16 sel) override { return m_sma_prot->garouh_bank_base(sel); }
};

extern const device_type NEOGEO_SMA_GAROUH_CART;


/*************************************************
 mslug3
 **************************************************/

class neogeo_sma_mslug3_cart : public neogeo_sma_cart
{
public:
	neogeo_sma_mslug3_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 1; }
	virtual UINT32 get_bank_base(UINT16 sel) override { return m_sma_prot->mslug3_bank_base(sel); }
};

extern const device_type NEOGEO_SMA_MSLUG3_CART;


/*************************************************
 kof2000
**************************************************/

class neogeo_sma_kof2000_cart : public neogeo_sma_cart
{
public:
	neogeo_sma_kof2000_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 2; }
	virtual UINT32 get_bank_base(UINT16 sel) override { return m_sma_prot->kof2000_bank_base(sel); }
};

extern const device_type NEOGEO_SMA_KOF2000_CART;


#endif
