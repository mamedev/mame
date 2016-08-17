// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************

 Neo Geo cart emulation
 CMC42 & CMC50 encrypted cart type

 ***********************************************************************************************************/


#include "emu.h"
#include "cmc.h"


//-------------------------------------------------
//  neogeo_cmc_cart - constructor
//-------------------------------------------------

const device_type NEOGEO_CMC_CART = &device_creator<neogeo_cmc_cart>;


neogeo_cmc_cart::neogeo_cmc_cart(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT16 clock, const char *shortname, const char *source) :
		neogeo_rom_device(mconfig, type, name, tag, owner, clock, shortname, source),
		m_prot(*this, "cmc_prot")
{}

neogeo_cmc_cart::neogeo_cmc_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock) :
	neogeo_rom_device(mconfig, NEOGEO_CMC_CART, "Neo Geo CMC Cart", tag, owner, clock, "neocart_cmc", __FILE__),
	m_prot(*this, "cmc_prot")
{}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_cmc_cart::device_start()
{
}

void neogeo_cmc_cart::device_reset()
{
}


static MACHINE_CONFIG_FRAGMENT( cmc_cart )
	MCFG_CMC_PROT_ADD("cmc_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_cmc_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cmc_cart );
}


/*************************************************
 zupapa
**************************************************/

const device_type NEOGEO_CMC_ZUPAPA_CART = &device_creator<neogeo_cmc_zupapa_cart>;

neogeo_cmc_zupapa_cart::neogeo_cmc_zupapa_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_cmc_cart(mconfig, NEOGEO_CMC_ZUPAPA_CART, "Neo Geo Zupapa CMC42 Cart", tag, owner, clock, "neocart_zupapa", __FILE__)
{}

void neogeo_cmc_zupapa_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->cmc42_gfx_decrypt(spr_region, spr_region_size, ZUPAPA_GFX_KEY);
	m_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}


/*************************************************
 mslug3h
**************************************************/

const device_type NEOGEO_CMC_MSLUG3H_CART = &device_creator<neogeo_cmc_mslug3h_cart>;

neogeo_cmc_mslug3h_cart::neogeo_cmc_mslug3h_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_cmc_cart(mconfig, NEOGEO_CMC_MSLUG3H_CART, "Neo Geo Metal Slug 3 AES CMC42 Cart", tag, owner, clock, "neocart_mslug3h", __FILE__)
{}

void neogeo_cmc_mslug3h_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->cmc42_gfx_decrypt(spr_region, spr_region_size, MSLUG3_GFX_KEY);
	m_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}


/*************************************************
 ganryu
**************************************************/

const device_type NEOGEO_CMC_GANRYU_CART = &device_creator<neogeo_cmc_ganryu_cart>;

neogeo_cmc_ganryu_cart::neogeo_cmc_ganryu_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_cmc_cart(mconfig, NEOGEO_CMC_GANRYU_CART, "Neo Geo Ganryu CMC42 Cart", tag, owner, clock, "neocart_ganryu", __FILE__)
{}

void neogeo_cmc_ganryu_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->cmc42_gfx_decrypt(spr_region, spr_region_size, GANRYU_GFX_KEY);
	m_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}


/*************************************************
 s1945p
**************************************************/

const device_type NEOGEO_CMC_S1945P_CART = &device_creator<neogeo_cmc_s1945p_cart>;

neogeo_cmc_s1945p_cart::neogeo_cmc_s1945p_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_cmc_cart(mconfig, NEOGEO_CMC_S1945P_CART, "Neo Geo Strikers 1945 Plus CMC42 Cart", tag, owner, clock, "neocart_s1945p", __FILE__)
{}

void neogeo_cmc_s1945p_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->cmc42_gfx_decrypt(spr_region, spr_region_size, S1945P_GFX_KEY);
	m_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}

/*************************************************
 preisle2
**************************************************/

const device_type NEOGEO_CMC_PREISLE2_CART = &device_creator<neogeo_cmc_preisle2_cart>;

neogeo_cmc_preisle2_cart::neogeo_cmc_preisle2_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_cmc_cart(mconfig, NEOGEO_CMC_PREISLE2_CART, "Neo Geo Prehistorik Isle 2 CMC42 Cart", tag, owner, clock, "neocart_preisle2", __FILE__)
{}

void neogeo_cmc_preisle2_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->cmc42_gfx_decrypt(spr_region, spr_region_size, PREISLE2_GFX_KEY);
	m_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}

/*************************************************
 bangbead
**************************************************/

const device_type NEOGEO_CMC_BANGBEAD_CART = &device_creator<neogeo_cmc_bangbead_cart>;

neogeo_cmc_bangbead_cart::neogeo_cmc_bangbead_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_cmc_cart(mconfig, NEOGEO_CMC_BANGBEAD_CART, "Neo Geo Bangbead CMC42 Cart", tag, owner, clock, "neocart_bangbead", __FILE__)
{}

void neogeo_cmc_bangbead_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->cmc42_gfx_decrypt(spr_region, spr_region_size, BANGBEAD_GFX_KEY);
	m_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}

/*************************************************
 ntd
**************************************************/

const device_type NEOGEO_CMC_NITD_CART = &device_creator<neogeo_cmc_nitd_cart>;

neogeo_cmc_nitd_cart::neogeo_cmc_nitd_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_cmc_cart(mconfig, NEOGEO_CMC_NITD_CART, "Neo Geo NITD CMC42 Cart", tag, owner, clock, "neocart_nitd", __FILE__)
{}

void neogeo_cmc_nitd_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->cmc42_gfx_decrypt(spr_region, spr_region_size, NITD_GFX_KEY);
	m_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}


/*************************************************
 sengoku3
**************************************************/

const device_type NEOGEO_CMC_SENGOKU3_CART = &device_creator<neogeo_cmc_sengoku3_cart>;

neogeo_cmc_sengoku3_cart::neogeo_cmc_sengoku3_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_cmc_cart(mconfig, NEOGEO_CMC_SENGOKU3_CART, "Neo Geo Sengoku 3 CMC42 Cart", tag, owner, clock, "neocart_sengoku3", __FILE__)
{}

void neogeo_cmc_sengoku3_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->cmc42_gfx_decrypt(spr_region, spr_region_size, SENGOKU3_GFX_KEY);
	m_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}

/*************************************************
 kof99k
**************************************************/

const device_type NEOGEO_CMC_KOF99K_CART = &device_creator<neogeo_cmc_kof99k_cart>;

neogeo_cmc_kof99k_cart::neogeo_cmc_kof99k_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_cmc_cart(mconfig, NEOGEO_CMC_KOF99K_CART, "Neo Geo KOF 99 Korea CMC42 Cart", tag, owner, clock, "neocart_kof99k", __FILE__)
{}

void neogeo_cmc_kof99k_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->cmc42_gfx_decrypt(spr_region, spr_region_size, KOF99_GFX_KEY);
	m_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}


/*************************************************
 kof2001
**************************************************/

const device_type NEOGEO_CMC_KOF2001_CART = &device_creator<neogeo_cmc_kof2001_cart>;

neogeo_cmc_kof2001_cart::neogeo_cmc_kof2001_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_cmc_cart(mconfig, NEOGEO_CMC_KOF2001_CART, "Neo Geo KOF 2001 CMC50 Cart", tag, owner, clock, "neocart_kof2001", __FILE__)
{}

void neogeo_cmc_kof2001_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, KOF2001_GFX_KEY);
	m_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}

/*************************************************
 kof2000n
**************************************************/

const device_type NEOGEO_CMC_KOF2000N_CART = &device_creator<neogeo_cmc_kof2000n_cart>;

neogeo_cmc_kof2000n_cart::neogeo_cmc_kof2000n_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_cmc_cart(mconfig, NEOGEO_CMC_KOF2000N_CART, "Neo Geo KOF 2000 CMC50 Cart", tag, owner, clock, "neocart_kof2000n", __FILE__)
{}

void neogeo_cmc_kof2000n_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, KOF2000_GFX_KEY);
	m_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}


/*************************************************
 jockeygp
 **************************************************/

const device_type NEOGEO_CMC_JOCKEYGP_CART = &device_creator<neogeo_cmc_jockeygp_cart>;

neogeo_cmc_jockeygp_cart::neogeo_cmc_jockeygp_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_cmc_cart(mconfig, NEOGEO_CMC_JOCKEYGP_CART, "Neo Geo Jockey GP CMC50 Cart", tag, owner, clock, "neocart_jockeygp", __FILE__)
{}


void neogeo_cmc_jockeygp_cart::device_start()
{
	save_item(NAME(m_ram));
}

void neogeo_cmc_jockeygp_cart::device_reset()
{
	memset(m_ram, 0, 0x2000);
}

void neogeo_cmc_jockeygp_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, JOCKEYGP_GFX_KEY);
	m_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}
