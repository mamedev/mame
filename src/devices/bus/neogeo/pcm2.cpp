// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************
 
 Neo Geo cart emulation
 PCM2 encrypted cart type

 ***********************************************************************************************************/


#include "emu.h"
#include "pcm2.h"


//-------------------------------------------------
//  neogeo_pcm2_cart - constructor
//-------------------------------------------------

const device_type NEOGEO_PCM2_CART = &device_creator<neogeo_pcm2_cart>;


neogeo_pcm2_cart::neogeo_pcm2_cart(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT16 clock, const char *shortname, const char *source) :
	neogeo_rom_device(mconfig, type, name, tag, owner, clock, shortname, source),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot")
{}

neogeo_pcm2_cart::neogeo_pcm2_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock) :
	neogeo_rom_device(mconfig, NEOGEO_PCM2_CART, "Neo Geo PCM2 Cart", tag, owner, clock, "neocart_pcm2", __FILE__),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot")
{}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_pcm2_cart::device_start()
{
}

void neogeo_pcm2_cart::device_reset()
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

static MACHINE_CONFIG_FRAGMENT( pcm2_cart )
	MCFG_CMC_PROT_ADD("cmc_prot")
	MCFG_PCM2_PROT_ADD("pcm2_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_pcm2_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( pcm2_cart );
}


/*************************************************
 mslug4
**************************************************/

const device_type NEOGEO_PCM2_MSLUG4_CART = &device_creator<neogeo_pcm2_mslug4_cart>;

neogeo_pcm2_mslug4_cart::neogeo_pcm2_mslug4_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_pcm2_cart(mconfig, NEOGEO_PCM2_MSLUG4_CART, "Neo Geo Metal Slug 4 PCM2 Cart", tag, owner, clock, "neocart_mslug4", __FILE__)
{}

void neogeo_pcm2_mslug4_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, MSLUG4_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
	m_pcm2_prot->decrypt(ym_region, ym_region_size, 8);
}


/*************************************************
 ms4plus
 **************************************************/

const device_type NEOGEO_PCM2_MS4PLUS_CART = &device_creator<neogeo_pcm2_ms4plus_cart>;

neogeo_pcm2_ms4plus_cart::neogeo_pcm2_ms4plus_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_pcm2_cart(mconfig, NEOGEO_PCM2_MS4PLUS_CART, "Neo Geo Metal Slug 4 Plus PCM2 Cart", tag, owner, clock, "neocart_ms4plus", __FILE__)
{}

void neogeo_pcm2_ms4plus_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, MSLUG4_GFX_KEY);
	m_pcm2_prot->decrypt(ym_region, ym_region_size, 8);
}


/*************************************************
 rotd
**************************************************/

const device_type NEOGEO_PCM2_ROTD_CART = &device_creator<neogeo_pcm2_rotd_cart>;

neogeo_pcm2_rotd_cart::neogeo_pcm2_rotd_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_pcm2_cart(mconfig, NEOGEO_PCM2_ROTD_CART, "Neo Geo Rage of the Dragon PCM2 Cart", tag, owner, clock, "neocart_rotd", __FILE__)
{}

void neogeo_pcm2_rotd_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, ROTD_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
	m_pcm2_prot->decrypt(ym_region, ym_region_size, 16);
}

/*************************************************
 pnyaa
**************************************************/

const device_type NEOGEO_PCM2_PNYAA_CART = &device_creator<neogeo_pcm2_pnyaa_cart>;

neogeo_pcm2_pnyaa_cart::neogeo_pcm2_pnyaa_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_pcm2_cart(mconfig, NEOGEO_PCM2_PNYAA_CART, "Neo Geo Pnyaa PCM2 Cart", tag, owner, clock, "neocart_pnyaa", __FILE__)
{}

void neogeo_pcm2_pnyaa_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, PNYAA_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
	m_pcm2_prot->decrypt(ym_region, ym_region_size, 4);
}

