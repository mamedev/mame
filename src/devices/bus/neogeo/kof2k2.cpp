// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************

 Neo Geo cart emulation
 The King of Fighers 2002 cart type (CMC + PCM2 + Additional CPU encryption)

 ***********************************************************************************************************/


#include "emu.h"
#include "kof2k2.h"


//-------------------------------------------------
//  neogeo_kof2002_cart - constructor
//-------------------------------------------------

const device_type NEOGEO_K2K2_CART = &device_creator<neogeo_kof2k2type_cart>;


neogeo_kof2k2type_cart::neogeo_kof2k2type_cart(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT16 clock, const char *shortname, const char *source) :
	neogeo_rom_device(mconfig, type, name, tag, owner, clock, shortname, source),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot"),
	m_kof2k2_prot(*this, "kof2002_prot")
{}

neogeo_kof2k2type_cart::neogeo_kof2k2type_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock) :
	neogeo_rom_device(mconfig, NEOGEO_K2K2_CART, "Neo Geo KOF2002-Type Cart", tag, owner, clock, "neocart_k2k2", __FILE__),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot"),
	m_kof2k2_prot(*this, "kof2002_prot")
{}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_kof2k2type_cart::device_start()
{
}

void neogeo_kof2k2type_cart::device_reset()
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

static MACHINE_CONFIG_FRAGMENT( kof2002_cart )
	MCFG_CMC_PROT_ADD("cmc_prot")
	MCFG_PCM2_PROT_ADD("pcm2_prot")
	MCFG_KOF2002_PROT_ADD("kof2002_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_kof2k2type_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( kof2002_cart );
}


/*************************************************
 kof2002
**************************************************/

const device_type NEOGEO_K2K2_KOF2002_CART = &device_creator<neogeo_kof2002_cart>;

neogeo_kof2002_cart::neogeo_kof2002_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_kof2k2type_cart(mconfig, NEOGEO_K2K2_KOF2002_CART, "Neo Geo KOF2002 Cart", tag, owner, clock, "neocart_kof2002", __FILE__)
{}


void neogeo_kof2002_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_kof2k2_prot->kof2002_decrypt_68k(cpuregion, cpuregion_size);
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, KOF2002_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
	m_pcm2_prot->swap(ym_region, ym_region_size, 0);
}

const device_type NEOGEO_K2K2_KF2K2PLS_CART = &device_creator<neogeo_kf2k2pls_cart>;

/*************************************************
 kf2k2pls
 **************************************************/

neogeo_kf2k2pls_cart::neogeo_kf2k2pls_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_kof2k2type_cart(mconfig, NEOGEO_K2K2_KF2K2PLS_CART, "Neo Geo KOF2002 Plus Cart", tag, owner, clock, "neocart_kf2k2pls", __FILE__)
{}

void neogeo_kf2k2pls_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_kof2k2_prot->kof2002_decrypt_68k(cpuregion, cpuregion_size);
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, KOF2002_GFX_KEY);
	m_pcm2_prot->swap(ym_region, ym_region_size, 0);
}


/*************************************************
 matrim
**************************************************/

const device_type NEOGEO_K2K2_MATRIM_CART = &device_creator<neogeo_matrim_cart>;

neogeo_matrim_cart::neogeo_matrim_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_kof2k2type_cart(mconfig, NEOGEO_K2K2_MATRIM_CART, "Neo Geo Matrimelee Cart", tag, owner, clock, "neocart_matrim", __FILE__)
{}

void neogeo_matrim_cart::decrypt_all(DECRYPT_ALL_PARAMS)
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

const device_type NEOGEO_K2K2_SAMSHO5_CART = &device_creator<neogeo_samsho5_cart>;

neogeo_samsho5_cart::neogeo_samsho5_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_kof2k2type_cart(mconfig, NEOGEO_K2K2_SAMSHO5_CART, "Neo Geo Samurai Shodown 5 Cart", tag, owner, clock, "neocart_samsho5", __FILE__)
{}

void neogeo_samsho5_cart::decrypt_all(DECRYPT_ALL_PARAMS)
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

const device_type NEOGEO_K2K2_SAMSHO5SP_CART = &device_creator<neogeo_samsho5sp_cart>;

neogeo_samsho5sp_cart::neogeo_samsho5sp_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_kof2k2type_cart(mconfig, NEOGEO_K2K2_SAMSHO5SP_CART, "Neo Geo Samurai Shodown 5 Special Cart", tag, owner, clock, "neocart_samsh5sp", __FILE__)
{}

void neogeo_samsho5sp_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_kof2k2_prot->samsh5sp_decrypt_68k(cpuregion, cpuregion_size);
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, SAMSHO5SP_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
	m_pcm2_prot->swap(ym_region, ym_region_size, 6);
}
