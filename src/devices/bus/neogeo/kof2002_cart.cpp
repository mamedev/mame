// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood
/***********************************************************************************************************

 NEOGEO ROM cart emulation

 ***********************************************************************************************************/


#include "emu.h"
#include "kof2002_cart.h"


//-------------------------------------------------
//  neogeo_kof2002_cart - constructor
//-------------------------------------------------

const device_type NEOGEO_KOF2002_CART = &device_creator<neogeo_kof2002_cart>;


neogeo_kof2002_cart::neogeo_kof2002_cart(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT16 clock, std::string shortname, std::string source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_neogeo_cart_interface(mconfig, *this),
	m_banked_cart(*this, "banked_cart"),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot"),
	m_kof2002_prot(*this, "kof2002_prot")
{
}

neogeo_kof2002_cart::neogeo_kof2002_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT16 clock)
	: device_t(mconfig, NEOGEO_KOF2002_CART, "NEOGEO KOF2002 Cart", tag, owner, clock, "neogeo_rom", __FILE__),
	device_neogeo_cart_interface(mconfig, *this),
	m_banked_cart(*this, "banked_cart"),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot"),
	m_kof2002_prot(*this, "kof2002_prot")
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_kof2002_cart::device_start()
{
}

void neogeo_kof2002_cart::device_reset()
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ16_MEMBER(neogeo_kof2002_cart::read_rom)
{
	return m_rom[offset];
}

static MACHINE_CONFIG_FRAGMENT( kof2002_cart )
	MCFG_NEOGEO_BANKED_CART_ADD("banked_cart")
	MCFG_CMC_PROT_ADD("cmc_prot")
	MCFG_PCM2_PROT_ADD("pcm2_prot")
	MCFG_KOF2002_PROT_ADD("kof2002_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_kof2002_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( kof2002_cart );
}



/* Individual cartridge types (mirror DRIVER_INIT functionality) */

/*************************************************
 KOF2002
**************************************************/

const device_type NEOGEO_KOF2002_KOF2002_CART = &device_creator<neogeo_kof2002_kof2002_cart>;

neogeo_kof2002_kof2002_cart::neogeo_kof2002_kof2002_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : neogeo_kof2002_cart(mconfig, NEOGEO_KOF2002_KOF2002_CART, "NEOGEO Kof2002-type kof2002 Cart", tag, owner, clock, "k2k_k2k_cart", __FILE__) {}

void neogeo_kof2002_kof2002_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_kof2002_prot->kof2002_decrypt_68k(cpuregion, cpuregion_size);
	m_cmc_prot->neogeo_cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->kof2000_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, KOF2002_GFX_KEY);
	m_pcm2_prot->neo_pcm2_swap(ym_region, ym_region_size, 0);

}

const device_type NEOGEO_KOF2002_KF2K2PLS_CART = &device_creator<neogeo_kof2002_kf2k2pls_cart>;

neogeo_kof2002_kf2k2pls_cart::neogeo_kof2002_kf2k2pls_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : neogeo_kof2002_cart(mconfig, NEOGEO_KOF2002_KF2K2PLS_CART, "NEOGEO Kof2002-type kof2002plus Cart", tag, owner, clock, "k2k_k2kpls_cart", __FILE__) {}

void neogeo_kof2002_kf2k2pls_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_kof2002_prot->kof2002_decrypt_68k(cpuregion, cpuregion_size);
	m_cmc_prot->cmc50_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, KOF2002_GFX_KEY);
	m_cmc_prot->neogeo_cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region,audio_region_size);
	m_pcm2_prot->neo_pcm2_swap(ym_region, ym_region_size, 0);
}


/*************************************************
 MATRIM
**************************************************/

const device_type NEOGEO_KOF2002_MATRIM_CART = &device_creator<neogeo_kof2002_matrim_cart>;

neogeo_kof2002_matrim_cart::neogeo_kof2002_matrim_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : neogeo_kof2002_cart(mconfig, NEOGEO_KOF2002_MATRIM_CART, "NEOGEO Kof2002-type matrim Cart", tag, owner, clock, "k2k_matrim_cart", __FILE__) {}

void neogeo_kof2002_matrim_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_kof2002_prot->matrim_decrypt_68k(cpuregion, cpuregion_size);
	m_cmc_prot->neogeo_cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->kof2000_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, MATRIM_GFX_KEY);
	m_pcm2_prot->neo_pcm2_swap(ym_region, ym_region_size, 1);

}

/*************************************************
 SAMSHO5
**************************************************/

const device_type NEOGEO_KOF2002_SAMSHO5_CART = &device_creator<neogeo_kof2002_samsho5_cart>;

neogeo_kof2002_samsho5_cart::neogeo_kof2002_samsho5_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : neogeo_kof2002_cart(mconfig, NEOGEO_KOF2002_SAMSHO5_CART, "NEOGEO Kof2002-type Samsho5 Cart", tag, owner, clock, "k2k_ss5_cart", __FILE__) {}

void neogeo_kof2002_samsho5_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_kof2002_prot->samsho5_decrypt_68k(cpuregion, cpuregion_size);
	m_cmc_prot->neogeo_cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->kof2000_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, SAMSHO5_GFX_KEY);
	m_pcm2_prot->neo_pcm2_swap(ym_region, ym_region_size, 4);
}

/*************************************************
 SAMSHO5SP
**************************************************/

const device_type NEOGEO_KOF2002_SAMSHO5SP_CART = &device_creator<neogeo_kof2002_samsho5sp_cart>;

neogeo_kof2002_samsho5sp_cart::neogeo_kof2002_samsho5sp_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : neogeo_kof2002_cart(mconfig, NEOGEO_KOF2002_SAMSHO5SP_CART, "NEOGEO Kof2002-type Samsho5sp Cart", tag, owner, clock, "k2k_ss5s_cart", __FILE__) {}

void neogeo_kof2002_samsho5sp_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_kof2002_prot->samsh5sp_decrypt_68k(cpuregion, cpuregion_size);
	m_cmc_prot->neogeo_cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->kof2000_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, SAMSHO5SP_GFX_KEY);
	m_pcm2_prot->neo_pcm2_swap(ym_region, ym_region_size, 6);
}
