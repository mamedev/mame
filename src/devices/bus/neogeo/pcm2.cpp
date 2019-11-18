// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************

 Neo Geo cart emulation
 PCM2 encrypted cart type

 ***********************************************************************************************************/


#include "emu.h"
#include "pcm2.h"


//-------------------------------------------------
//  neogeo_pcm2_cart_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NEOGEO_PCM2_CART, neogeo_pcm2_cart_device, "neocart_pcm2", "Neo Geo PCM2 Cart")


neogeo_pcm2_cart_device::neogeo_pcm2_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint16_t clock) :
	neogeo_rom_device(mconfig, type, tag, owner, clock),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot")
{
}

neogeo_pcm2_cart_device::neogeo_pcm2_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock) :
	neogeo_pcm2_cart_device(mconfig, NEOGEO_PCM2_CART, tag, owner, clock)
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_pcm2_cart_device::device_start()
{
}

void neogeo_pcm2_cart_device::device_reset()
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

void neogeo_pcm2_cart_device::device_add_mconfig(machine_config &config)
{
	NG_CMC_PROT(config, m_cmc_prot);
	NG_PCM2_PROT(config, m_pcm2_prot);
}


/*************************************************
 mslug4
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_PCM2_MSLUG4_CART, neogeo_pcm2_mslug4_cart_device, "neocart_mslug4", "Neo Geo Metal Slug 3 PCM2 Cart")

neogeo_pcm2_mslug4_cart_device::neogeo_pcm2_mslug4_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_pcm2_cart_device(mconfig, NEOGEO_PCM2_MSLUG4_CART, tag, owner, clock)
{
}

void neogeo_pcm2_mslug4_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, MSLUG4_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
	m_pcm2_prot->decrypt(ym_region, ym_region_size, 8);
}


/*************************************************
 ms4plus
 **************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_PCM2_MS4PLUS_CART, neogeo_pcm2_ms4plus_cart_device, "neocart_ms4plus", "Neo Geo Metal Slug 4 Plus PCM2 Cart")

neogeo_pcm2_ms4plus_cart_device::neogeo_pcm2_ms4plus_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_pcm2_cart_device(mconfig, NEOGEO_PCM2_MS4PLUS_CART, tag, owner, clock)
{
}

void neogeo_pcm2_ms4plus_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, MSLUG4_GFX_KEY);
	m_pcm2_prot->decrypt(ym_region, ym_region_size, 8);
}


/*************************************************
 rotd
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_PCM2_ROTD_CART, neogeo_pcm2_rotd_cart_device, "neocart_rotd", "Neo Geo Rage of the Dragon PCM2 Cart")

neogeo_pcm2_rotd_cart_device::neogeo_pcm2_rotd_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_pcm2_cart_device(mconfig, NEOGEO_PCM2_ROTD_CART, tag, owner, clock)
{
}

void neogeo_pcm2_rotd_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, ROTD_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
	m_pcm2_prot->decrypt(ym_region, ym_region_size, 16);
}

/*************************************************
 pnyaa
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_PCM2_PNYAA_CART, neogeo_pcm2_pnyaa_cart_device, "neocart_pnyaa", "Neo Geo Pnyaa PCM2 Cart")

neogeo_pcm2_pnyaa_cart_device::neogeo_pcm2_pnyaa_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_pcm2_cart_device(mconfig, NEOGEO_PCM2_PNYAA_CART, tag, owner, clock)
{
}

void neogeo_pcm2_pnyaa_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, PNYAA_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
	m_pcm2_prot->decrypt(ym_region, ym_region_size, 4);
}
