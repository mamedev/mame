// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************

 Neo Geo cart emulation
 SMA encrypted cart type (+ CMC42 or CMC50)

 ***********************************************************************************************************/


#include "emu.h"
#include "sma.h"


DEFINE_DEVICE_TYPE(NEOGEO_SMA_CART,         neogeo_sma_cart_device,         "neocart_sma",     "Neo Geo SMA Cart")
DEFINE_DEVICE_TYPE(NEOGEO_SMA_KOF99_CART,   neogeo_sma_kof99_cart_device,   "neocart_kof99",   "Neo Geo KoF 99 SMA Cart")
DEFINE_DEVICE_TYPE(NEOGEO_SMA_GAROU_CART,   neogeo_sma_garou_cart_device,   "neocart_garou",   "Neo Geo Garou SMA Cart")
DEFINE_DEVICE_TYPE(NEOGEO_SMA_GAROUH_CART,  neogeo_sma_garouh_cart_device,  "neocart_garouh",  "Neo Geo Garou AES SMA Cart")
DEFINE_DEVICE_TYPE(NEOGEO_SMA_MSLUG3_CART,  neogeo_sma_mslug3_cart_device,  "neocart_mslug3",  "Neo Geo Metal Slug 3 SMA Cart (green)")
DEFINE_DEVICE_TYPE(NEOGEO_SMA_MSLUG3A_CART, neogeo_sma_mslug3a_cart_device, "neocart_mslug3a", "Neo Geo Metal Slug 3 SMA Cart (white)")
DEFINE_DEVICE_TYPE(NEOGEO_SMA_KOF2000_CART, neogeo_sma_kof2000_cart_device, "neocart_kof2000", "Neo Geo KoF 2000 SMA Cart")


//-------------------------------------------------
//  neogeo_sma_cart_device - constructor
//-------------------------------------------------

neogeo_sma_cart_device::neogeo_sma_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint16_t clock) :
	neogeo_rom_device(mconfig, type, tag, owner, clock),
	m_sma_prot(*this, "sma_prot"),
	m_cmc_prot(*this, "cmc_prot")
{
}

neogeo_sma_cart_device::neogeo_sma_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock) :
	neogeo_sma_cart_device(mconfig, NEOGEO_SMA_CART, tag, owner, clock)
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_sma_cart_device::device_start()
{
}

void neogeo_sma_cart_device::device_reset()
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

void neogeo_sma_cart_device::device_add_mconfig(machine_config &config)
{
	NG_SMA_PROT(config, m_sma_prot);
	NG_CMC_PROT(config, m_cmc_prot);
}


/*************************************************
 kof99
**************************************************/

neogeo_sma_kof99_cart_device::neogeo_sma_kof99_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_sma_cart_device(mconfig, NEOGEO_SMA_KOF99_CART, tag, owner, clock)
{
}

void neogeo_sma_kof99_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_sma_prot->kof99_decrypt_68k(cpuregion);
	m_cmc_prot->cmc42_gfx_decrypt(spr_region, spr_region_size, KOF99_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}


/*************************************************
 garou
**************************************************/

neogeo_sma_garou_cart_device::neogeo_sma_garou_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_sma_cart_device(mconfig, NEOGEO_SMA_GAROU_CART, tag, owner, clock)
{
}

void neogeo_sma_garou_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_sma_prot->garou_decrypt_68k(cpuregion);
	m_cmc_prot->cmc42_gfx_decrypt(spr_region, spr_region_size, GAROU_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}


/*************************************************
 garouh
 **************************************************/

neogeo_sma_garouh_cart_device::neogeo_sma_garouh_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_sma_cart_device(mconfig, NEOGEO_SMA_GAROUH_CART, tag, owner, clock)
{
}

void neogeo_sma_garouh_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_sma_prot->garouh_decrypt_68k(cpuregion);
	m_cmc_prot->cmc42_gfx_decrypt(spr_region, spr_region_size, GAROU_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}


/*************************************************
 mslug3
 **************************************************/

neogeo_sma_mslug3_cart_device::neogeo_sma_mslug3_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_sma_cart_device(mconfig, NEOGEO_SMA_MSLUG3_CART, tag, owner, clock)
{
}

void neogeo_sma_mslug3_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_sma_prot->mslug3_decrypt_68k(cpuregion);
	m_cmc_prot->cmc42_gfx_decrypt(spr_region, spr_region_size, MSLUG3_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}

neogeo_sma_mslug3a_cart_device::neogeo_sma_mslug3a_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_sma_cart_device(mconfig, NEOGEO_SMA_MSLUG3A_CART, tag, owner, clock)
{
}

void neogeo_sma_mslug3a_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_sma_prot->mslug3a_decrypt_68k(cpuregion);
	m_cmc_prot->cmc42_gfx_decrypt(spr_region, spr_region_size, MSLUG3_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}


/*************************************************
 kof2000
**************************************************/

neogeo_sma_kof2000_cart_device::neogeo_sma_kof2000_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_sma_cart_device(mconfig, NEOGEO_SMA_KOF2000_CART, tag, owner, clock)
{
}

void neogeo_sma_kof2000_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_sma_prot->kof2000_decrypt_68k(cpuregion);
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, KOF2000_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}
