// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************

 Neo Geo cart emulation
 The King of Fighters 2002 cart type (CMC + PCM2 + Additional CPU encryption)

 ***********************************************************************************************************/


#include "emu.h"
#include "kof2k2.h"


//-------------------------------------------------
//  neogeo_kof2002_cart_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NEOGEO_K2K2_CART, neogeo_kof2k2type_cart_device, "neocart_k2k2", "Neo Geo KOF2002-Type Cart")


neogeo_kof2k2type_cart_device::neogeo_kof2k2type_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint16_t clock) :
	neogeo_rom_device(mconfig, type, tag, owner, clock),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot"),
	m_kof2k2_prot(*this, "kof2002_prot")
{
}

neogeo_kof2k2type_cart_device::neogeo_kof2k2type_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock) :
	neogeo_kof2k2type_cart_device(mconfig, NEOGEO_K2K2_CART, tag, owner, clock)
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_kof2k2type_cart_device::device_start()
{
}

void neogeo_kof2k2type_cart_device::device_reset()
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

void neogeo_kof2k2type_cart_device::device_add_mconfig(machine_config &config)
{
	NG_CMC_PROT(config, m_cmc_prot);
	NG_PCM2_PROT(config, m_pcm2_prot);
	NG_KOF2002_PROT(config, m_kof2k2_prot);
}


/*************************************************
 kof2002
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_K2K2_KOF2002_CART, neogeo_kof2002_cart_device, "neocart_kof2002", "Neo Geo KoF 2002 Cart")

neogeo_kof2002_cart_device::neogeo_kof2002_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_kof2k2type_cart_device(mconfig, NEOGEO_K2K2_KOF2002_CART, tag, owner, clock)
{
}

void neogeo_kof2002_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_kof2k2_prot->kof2002_decrypt_68k(cpuregion, cpuregion_size);
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, KOF2002_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
	m_pcm2_prot->swap(ym_region, ym_region_size, 0);
}

/*************************************************
 kf2k2pls
 **************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_K2K2_KF2K2PLS_CART, neogeo_kf2k2pls_cart_device, "neocart_kf2k2pls", "Neo Geo KoF 2002 Plus Cart")

neogeo_kf2k2pls_cart_device::neogeo_kf2k2pls_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_kof2k2type_cart_device(mconfig, NEOGEO_K2K2_KF2K2PLS_CART, tag, owner, clock)
{
}

void neogeo_kf2k2pls_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_kof2k2_prot->kof2002_decrypt_68k(cpuregion, cpuregion_size);
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, KOF2002_GFX_KEY);
	m_pcm2_prot->swap(ym_region, ym_region_size, 0);
}


/*************************************************
 matrim
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_K2K2_MATRIM_CART, neogeo_matrim_cart_device, "neocart_matrim", "Neo Geo Matrimelee Cart")

neogeo_matrim_cart_device::neogeo_matrim_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_kof2k2type_cart_device(mconfig, NEOGEO_K2K2_MATRIM_CART, tag, owner, clock)
{
}

void neogeo_matrim_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_kof2k2_prot->matrim_decrypt_68k(cpuregion, cpuregion_size);
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, MATRIM_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
	m_pcm2_prot->swap(ym_region, ym_region_size, 1);
}

/*************************************************
 samsho5
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_K2K2_SAMSHO5_CART, neogeo_samsho5_cart_device, "neocart_samsho5", "Neo Geo Samurai Shodown 5 Cart")

neogeo_samsho5_cart_device::neogeo_samsho5_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_kof2k2type_cart_device(mconfig, NEOGEO_K2K2_SAMSHO5_CART, tag, owner, clock)
{
}

void neogeo_samsho5_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_kof2k2_prot->samsho5_decrypt_68k(cpuregion, cpuregion_size);
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, SAMSHO5_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
	m_pcm2_prot->swap(ym_region, ym_region_size, 4);
}

/*************************************************
 samsh5sp
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_K2K2_SAMSHO5SP_CART, neogeo_samsho5sp_cart_device, "neocart_samsh5sp", "Neo Geo Samurai Shodown 5 Special Cart")

neogeo_samsho5sp_cart_device::neogeo_samsho5sp_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_kof2k2type_cart_device(mconfig, NEOGEO_K2K2_SAMSHO5SP_CART, tag, owner, clock)
{
}

void neogeo_samsho5sp_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_kof2k2_prot->samsh5sp_decrypt_68k(cpuregion, cpuregion_size);
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, SAMSHO5SP_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
	m_pcm2_prot->swap(ym_region, ym_region_size, 6);
}
