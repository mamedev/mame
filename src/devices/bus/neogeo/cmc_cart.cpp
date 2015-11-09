// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood
/***********************************************************************************************************

 NEOGEO ROM cart emulation

 ***********************************************************************************************************/


#include "emu.h"
#include "cmc_cart.h"


//-------------------------------------------------
//  neogeo_cmc_cart - constructor
//-------------------------------------------------

const device_type NEOGEO_CMC_CART = &device_creator<neogeo_cmc_cart>;


neogeo_cmc_cart::neogeo_cmc_cart(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT16 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_neogeo_cart_interface(mconfig, *this),
	m_banked_cart(*this, "banked_cart"),
	m_cmc_prot(*this, "cmc_prot")

{
}

neogeo_cmc_cart::neogeo_cmc_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock)
	: device_t(mconfig, NEOGEO_CMC_CART, "NEOGEO SMA Cart", tag, owner, clock, "neogeo_rom", __FILE__),
	device_neogeo_cart_interface(mconfig, *this),
	m_banked_cart(*this, "banked_cart"),
	m_cmc_prot(*this, "cmc_prot")
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_cmc_cart::device_start()
{
}

void neogeo_cmc_cart::device_reset()
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ16_MEMBER(neogeo_cmc_cart::read_rom)
{
	return m_rom[offset];
}

static MACHINE_CONFIG_FRAGMENT( cmc_cart )
	MCFG_NEOGEO_BANKED_CART_ADD("banked_cart")
	MCFG_CMC_PROT_ADD("cmc_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_cmc_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cmc_cart );
}



/* Individual cartridge types (mirror DRIVER_INIT functionality) */

/*************************************************
 Zupapa
**************************************************/

const device_type NEOGEO_CMC_ZUPAPA_CART = &device_creator<neogeo_cmc_zupapa_cart>;

neogeo_cmc_zupapa_cart::neogeo_cmc_zupapa_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : neogeo_cmc_cart(mconfig, NEOGEO_CMC_ZUPAPA_CART, "NEOGEO CMC zupapa Cart", tag, owner, clock, "cmc_zupapa_cart", __FILE__) {}

void neogeo_cmc_zupapa_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->kof99_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, ZUPAPA_GFX_KEY);
}


/*************************************************
 Zupapa
**************************************************/

const device_type NEOGEO_CMC_MSLUG3H_CART = &device_creator<neogeo_cmc_mslug3h_cart>;

neogeo_cmc_mslug3h_cart::neogeo_cmc_mslug3h_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : neogeo_cmc_cart(mconfig, NEOGEO_CMC_MSLUG3H_CART, "NEOGEO CMC mslug3h Cart", tag, owner, clock, "cmc_mslug3h_cart", __FILE__) {}

void neogeo_cmc_mslug3h_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->kof99_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, MSLUG3_GFX_KEY);
}


/*************************************************
 Ganryu
**************************************************/

const device_type NEOGEO_CMC_GANRYU_CART = &device_creator<neogeo_cmc_ganryu_cart>;

neogeo_cmc_ganryu_cart::neogeo_cmc_ganryu_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : neogeo_cmc_cart(mconfig, NEOGEO_CMC_GANRYU_CART, "NEOGEO CMC ganryu Cart", tag, owner, clock, "cmc_ganryu_cart", __FILE__) {}

void neogeo_cmc_ganryu_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->kof99_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, GANRYU_GFX_KEY);
}


/*************************************************
 S1945P
**************************************************/

const device_type NEOGEO_CMC_S1945P_CART = &device_creator<neogeo_cmc_s1945p_cart>;

neogeo_cmc_s1945p_cart::neogeo_cmc_s1945p_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : neogeo_cmc_cart(mconfig, NEOGEO_CMC_S1945P_CART, "NEOGEO CMC s1945p Cart", tag, owner, clock, "cmc_s1945p_cart", __FILE__) {}

void neogeo_cmc_s1945p_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->kof99_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, S1945P_GFX_KEY);
}

/*************************************************
 PREISLE2
**************************************************/

const device_type NEOGEO_CMC_PREISLE2_CART = &device_creator<neogeo_cmc_preisle2_cart>;

neogeo_cmc_preisle2_cart::neogeo_cmc_preisle2_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : neogeo_cmc_cart(mconfig, NEOGEO_CMC_PREISLE2_CART, "NEOGEO CMC preisle2 Cart", tag, owner, clock, "cmc_preisle2_cart", __FILE__) {}

void neogeo_cmc_preisle2_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->kof99_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, PREISLE2_GFX_KEY);
}

/*************************************************
 BANGBEAD
**************************************************/

const device_type NEOGEO_CMC_BANGBEAD_CART = &device_creator<neogeo_cmc_bangbead_cart>;

neogeo_cmc_bangbead_cart::neogeo_cmc_bangbead_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : neogeo_cmc_cart(mconfig, NEOGEO_CMC_BANGBEAD_CART, "NEOGEO CMC bangbead Cart", tag, owner, clock, "cmc_bangbead_cart", __FILE__) {}

void neogeo_cmc_bangbead_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->kof99_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, BANGBEAD_GFX_KEY);
}

/*************************************************
 NITD
**************************************************/

const device_type NEOGEO_CMC_NITD_CART = &device_creator<neogeo_cmc_nitd_cart>;

neogeo_cmc_nitd_cart::neogeo_cmc_nitd_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : neogeo_cmc_cart(mconfig, NEOGEO_CMC_NITD_CART, "NEOGEO CMC nitd Cart", tag, owner, clock, "cmc_nitd_cart", __FILE__) {}

void neogeo_cmc_nitd_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->kof99_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, NITD_GFX_KEY);
}


/*************************************************
 SENGOKU3
**************************************************/

const device_type NEOGEO_CMC_SENGOKU3_CART = &device_creator<neogeo_cmc_sengoku3_cart>;

neogeo_cmc_sengoku3_cart::neogeo_cmc_sengoku3_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : neogeo_cmc_cart(mconfig, NEOGEO_CMC_SENGOKU3_CART, "NEOGEO CMC sengoku3 Cart", tag, owner, clock, "cmc_sengoku3_cart", __FILE__) {}

void neogeo_cmc_sengoku3_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->kof99_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, SENGOKU3_GFX_KEY);
}

/*************************************************
 KOF99K
**************************************************/

const device_type NEOGEO_CMC_KOF99K_CART = &device_creator<neogeo_cmc_kof99k_cart>;

neogeo_cmc_kof99k_cart::neogeo_cmc_kof99k_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : neogeo_cmc_cart(mconfig, NEOGEO_CMC_KOF99K_CART, "NEOGEO CMC kof99k Cart", tag, owner, clock, "cmc_kof99k_cart", __FILE__) {}

void neogeo_cmc_kof99k_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->kof99_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, KOF99_GFX_KEY);
}


/*************************************************
 KOF2001
**************************************************/

const device_type NEOGEO_CMC_KOF2001_CART = &device_creator<neogeo_cmc_kof2001_cart>;

neogeo_cmc_kof2001_cart::neogeo_cmc_kof2001_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : neogeo_cmc_cart(mconfig, NEOGEO_CMC_KOF2001_CART, "NEOGEO CMC kof2001 Cart", tag, owner, clock, "cmc_kof2001_cart", __FILE__) {}

void neogeo_cmc_kof2001_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->neogeo_cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->kof2000_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, KOF2001_GFX_KEY);
}

/*************************************************
 KOF2000N
**************************************************/

const device_type NEOGEO_CMC_KOF2000N_CART = &device_creator<neogeo_cmc_kof2000n_cart>;

neogeo_cmc_kof2000n_cart::neogeo_cmc_kof2000n_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : neogeo_cmc_cart(mconfig, NEOGEO_CMC_KOF2000N_CART, "NEOGEO CMC kof2000n Cart", tag, owner, clock, "cmc_kof2000n_cart", __FILE__) {}

void neogeo_cmc_kof2000n_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->neogeo_cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->kof2000_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, KOF2000_GFX_KEY);
}
