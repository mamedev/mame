// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#ifndef __NEOGEO_CMC_H
#define __NEOGEO_CMC_H

#include "slot.h"
#include "rom.h"
#include "prot_cmc.h"

// ======================> neogeo_cmc_cart

class neogeo_cmc_cart : public neogeo_rom_device
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
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override {}
	virtual int get_fixed_bank_type(void) override { return 0; }

	required_device<cmc_prot_device> m_prot;
};



// device type definition
extern const device_type NEOGEO_CMC_CART;


/*************************************************
 zupapa
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
 mslug3h
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
 ganryu
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
 s1945p
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
 preisle2
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
 bangbead
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
 nitd
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
 sengoku3
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
 kof99k
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
 kof2001
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
 kof2000n
**************************************************/

class neogeo_cmc_kof2000n_cart : public neogeo_cmc_cart
{
public:
	neogeo_cmc_kof2000n_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 2; }
};
extern const device_type NEOGEO_CMC_KOF2000N_CART;


/*************************************************
 jockeygp
 **************************************************/

class neogeo_cmc_jockeygp_cart : public neogeo_cmc_cart
{
public:
	neogeo_cmc_jockeygp_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 1; }

	virtual DECLARE_READ16_MEMBER(ram_r) override { return m_ram[offset]; }
	virtual DECLARE_WRITE16_MEMBER(ram_w) override { COMBINE_DATA(&m_ram[offset]); }

	virtual void device_start() override;
	virtual void device_reset() override;

private:
	UINT16 m_ram[0x1000];
};

extern const device_type NEOGEO_CMC_JOCKEYGP_CART;

#endif
