// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************

 Neo Geo cart emulation
 SMA encrypted cart type (+ CMC42 or CMC50)

 ***********************************************************************************************************/


#include "emu.h"
#include "sma.h"


//-------------------------------------------------
//  neogeo_sma_cart - constructor
//-------------------------------------------------

const device_type NEOGEO_SMA_CART = &device_creator<neogeo_sma_cart>;


neogeo_sma_cart::neogeo_sma_cart(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT16 clock, const char *shortname, const char *source) :
	neogeo_rom_device(mconfig, type, name, tag, owner, clock, shortname, source),
	m_sma_prot(*this, "sma_prot"),
	m_cmc_prot(*this, "cmc_prot")
{}

neogeo_sma_cart::neogeo_sma_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock) :
	neogeo_rom_device(mconfig, NEOGEO_SMA_CART, "Neo Geo SMA Cart", tag, owner, clock, "neocart_sma", __FILE__),
	m_sma_prot(*this, "sma_prot"),
	m_cmc_prot(*this, "cmc_prot")
{}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_sma_cart::device_start()
{
}

void neogeo_sma_cart::device_reset()
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

static MACHINE_CONFIG_FRAGMENT( sma_cart )
	MCFG_SMA_PROT_ADD("sma_prot")
	MCFG_CMC_PROT_ADD("cmc_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_sma_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sma_cart );
}


/*************************************************
 kof99
**************************************************/

const device_type NEOGEO_SMA_KOF99_CART = &device_creator<neogeo_sma_kof99_cart>;

neogeo_sma_kof99_cart::neogeo_sma_kof99_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_sma_cart(mconfig, NEOGEO_SMA_KOF99_CART, "Neo Geo KOF 99 SMA Cart", tag, owner, clock, "neocart_kof99", __FILE__)
{}

void neogeo_sma_kof99_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_sma_prot->kof99_decrypt_68k(cpuregion);
	m_cmc_prot->cmc42_gfx_decrypt(spr_region, spr_region_size, KOF99_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}


/*************************************************
 garou
**************************************************/

const device_type NEOGEO_SMA_GAROU_CART = &device_creator<neogeo_sma_garou_cart>;

neogeo_sma_garou_cart::neogeo_sma_garou_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_sma_cart(mconfig, NEOGEO_SMA_GAROU_CART, "Neo Geo Garou SMA Cart", tag, owner, clock, "neocart_garou", __FILE__)
{}

void neogeo_sma_garou_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_sma_prot->garou_decrypt_68k(cpuregion);
	m_cmc_prot->cmc42_gfx_decrypt(spr_region, spr_region_size, GAROU_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}


/*************************************************
 garouh
 **************************************************/

const device_type NEOGEO_SMA_GAROUH_CART = &device_creator<neogeo_sma_garouh_cart>;

neogeo_sma_garouh_cart::neogeo_sma_garouh_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_sma_cart(mconfig, NEOGEO_SMA_GAROUH_CART, "Neo Geo Garou AES SMA Cart", tag, owner, clock, "neocart_garouh", __FILE__)
{}

void neogeo_sma_garouh_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_sma_prot->garouh_decrypt_68k(cpuregion);
	m_cmc_prot->cmc42_gfx_decrypt(spr_region, spr_region_size, GAROU_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}


/*************************************************
 mslug3
 **************************************************/

const device_type NEOGEO_SMA_MSLUG3_CART = &device_creator<neogeo_sma_mslug3_cart>;

neogeo_sma_mslug3_cart::neogeo_sma_mslug3_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_sma_cart(mconfig, NEOGEO_SMA_MSLUG3_CART, "Neo Geo Metal Slug 3 SMA Cart", tag, owner, clock, "neocart_mslug3", __FILE__)
{}

void neogeo_sma_mslug3_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_sma_prot->mslug3_decrypt_68k(cpuregion);
	m_cmc_prot->cmc42_gfx_decrypt(spr_region, spr_region_size, MSLUG3_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}


/*************************************************
 kof2000
**************************************************/

const device_type NEOGEO_SMA_KOF2000_CART = &device_creator<neogeo_sma_kof2000_cart>;

neogeo_sma_kof2000_cart::neogeo_sma_kof2000_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_sma_cart(mconfig, NEOGEO_SMA_KOF2000_CART, "Neo Geo KOF 2000 SMA Cart", tag, owner, clock, "neocart_kof2000", __FILE__)
{}

void neogeo_sma_kof2000_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_sma_prot->kof2000_decrypt_68k(cpuregion);
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, KOF2000_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}
