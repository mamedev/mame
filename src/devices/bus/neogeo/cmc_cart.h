// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood
#ifndef __NEOGEO_CMC_CART_H
#define __NEOGEO_CMC_CART_H

#include "neogeo_slot.h"
#include "banked_cart.h"
#include "sma_prot.h"
#include "cmc_prot.h"

// ======================> neogeo_cmc_cart

class neogeo_cmc_cart : public device_t,
						public device_neogeo_cart_interface
{
public:
	// construction/destruction
	neogeo_cmc_cart(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT16 clock, const char *shortname, const char *source);
	neogeo_cmc_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read_rom);

	virtual void activate_cart(ACTIVATE_CART_PARAMS) override { m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size); }
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override {}
	virtual int get_fixed_bank_type(void) override { return 0; }

	required_device<neogeo_banked_cart_device> m_banked_cart;
	required_device<cmc_prot_device> m_cmc_prot;
};



// device type definition
extern const device_type NEOGEO_CMC_CART;


/*************************************************
 ZUPAPA
**************************************************/

class neogeo_cmc_zupapa_cart : public neogeo_cmc_cart
{
public:
	neogeo_cmc_zupapa_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 1; }
};
extern const device_type NEOGEO_CMC_ZUPAPA_CART;

/*************************************************
 MSLUG3H
**************************************************/

class neogeo_cmc_mslug3h_cart : public neogeo_cmc_cart
{
public:
	neogeo_cmc_mslug3h_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 1; }
};
extern const device_type NEOGEO_CMC_MSLUG3H_CART;


/*************************************************
 GANRYU
**************************************************/

class neogeo_cmc_ganryu_cart : public neogeo_cmc_cart
{
public:
	neogeo_cmc_ganryu_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 1; }
};
extern const device_type NEOGEO_CMC_GANRYU_CART;

/*************************************************
 S1945P
**************************************************/

class neogeo_cmc_s1945p_cart : public neogeo_cmc_cart
{
public:
	neogeo_cmc_s1945p_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 1; }
};
extern const device_type NEOGEO_CMC_S1945P_CART;


/*************************************************
 PREISLE2
**************************************************/

class neogeo_cmc_preisle2_cart : public neogeo_cmc_cart
{
public:
	neogeo_cmc_preisle2_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 1; }
};
extern const device_type NEOGEO_CMC_PREISLE2_CART;

/*************************************************
 BANGBEAD
**************************************************/

class neogeo_cmc_bangbead_cart : public neogeo_cmc_cart
{
public:
	neogeo_cmc_bangbead_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 1; }
};
extern const device_type NEOGEO_CMC_BANGBEAD_CART;


/*************************************************
 NITD
**************************************************/

class neogeo_cmc_nitd_cart : public neogeo_cmc_cart
{
public:
	neogeo_cmc_nitd_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 1; }
};
extern const device_type NEOGEO_CMC_NITD_CART;

/*************************************************
 SENGOKU3
**************************************************/

class neogeo_cmc_sengoku3_cart : public neogeo_cmc_cart
{
public:
	neogeo_cmc_sengoku3_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 1; }
};
extern const device_type NEOGEO_CMC_SENGOKU3_CART;

/*************************************************
 KOF99K
**************************************************/

class neogeo_cmc_kof99k_cart : public neogeo_cmc_cart
{
public:
	neogeo_cmc_kof99k_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 1; }
};
extern const device_type NEOGEO_CMC_KOF99K_CART;

/*************************************************
 KOF2001
**************************************************/

class neogeo_cmc_kof2001_cart : public neogeo_cmc_cart
{
public:
	neogeo_cmc_kof2001_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 1; }
};
extern const device_type NEOGEO_CMC_KOF2001_CART;

/*************************************************
 KOF2000N
**************************************************/

class neogeo_cmc_kof2000n_cart : public neogeo_cmc_cart
{
public:
	neogeo_cmc_kof2000n_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 2; }
};
extern const device_type NEOGEO_CMC_KOF2000N_CART;



#endif
