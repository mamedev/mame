// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood
/***********************************************************************************************************

 NEOGEO ROM cart emulation

 ***********************************************************************************************************/


#include "emu.h"
#include "pcm2_cart.h"


//-------------------------------------------------
//  neogeo_pcm2_cart - constructor
//-------------------------------------------------

const device_type NEOGEO_PCM2_CART = &device_creator<neogeo_pcm2_cart>;


neogeo_pcm2_cart::neogeo_pcm2_cart(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT16 clock, std::string shortname, std::string source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_neogeo_cart_interface(mconfig, *this),
	m_banked_cart(*this, "banked_cart"),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot")
{
}

neogeo_pcm2_cart::neogeo_pcm2_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT16 clock)
	: device_t(mconfig, NEOGEO_PCM2_CART, "NEOGEO PCM2 Cart", tag, owner, clock, "neogeo_rom", __FILE__),
	device_neogeo_cart_interface(mconfig, *this),
	m_banked_cart(*this, "banked_cart"),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot")
{
}


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

READ16_MEMBER(neogeo_pcm2_cart::read_rom)
{
	return m_rom[offset];
}

static MACHINE_CONFIG_FRAGMENT( pcm2_cart )
	MCFG_NEOGEO_BANKED_CART_ADD("banked_cart")
	MCFG_CMC_PROT_ADD("cmc_prot")
	MCFG_PCM2_PROT_ADD("pcm2_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_pcm2_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( pcm2_cart );
}



/* Individual cartridge types (mirror DRIVER_INIT functionality) */

/*************************************************
 MSLUG4
**************************************************/

const device_type NEOGEO_PCM2_MSLUG4_CART = &device_creator<neogeo_pcm2_mslug4_cart>;

neogeo_pcm2_mslug4_cart::neogeo_pcm2_mslug4_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : neogeo_pcm2_cart(mconfig, NEOGEO_PCM2_MSLUG4_CART, "NEOGEO PCM2 mslug4 Cart", tag, owner, clock, "p2_mslug4_cart", __FILE__) {}

void neogeo_pcm2_mslug4_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->neogeo_cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->kof2000_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, MSLUG4_GFX_KEY);
	m_pcm2_prot->neo_pcm2_snk_1999(ym_region, ym_region_size, 8);

}


const device_type NEOGEO_PCM2_MS4PLUS_CART = &device_creator<neogeo_pcm2_ms4plus_cart>;

neogeo_pcm2_ms4plus_cart::neogeo_pcm2_ms4plus_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : neogeo_pcm2_cart(mconfig, NEOGEO_PCM2_MS4PLUS_CART, "NEOGEO PCM2 ms4plus Cart", tag, owner, clock, "p2_ms4plus_cart", __FILE__) {}

void neogeo_pcm2_ms4plus_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->cmc50_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, MSLUG4_GFX_KEY);
	m_cmc_prot->neogeo_cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region,audio_region_size);
	m_pcm2_prot->neo_pcm2_snk_1999(ym_region, ym_region_size, 8);
}


/*************************************************
 ROTD
**************************************************/

const device_type NEOGEO_PCM2_ROTD_CART = &device_creator<neogeo_pcm2_rotd_cart>;

neogeo_pcm2_rotd_cart::neogeo_pcm2_rotd_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : neogeo_pcm2_cart(mconfig, NEOGEO_PCM2_ROTD_CART, "NEOGEO PCM2 rotd Cart", tag, owner, clock, "p2_rotd_cart", __FILE__) {}

void neogeo_pcm2_rotd_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->neogeo_cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->kof2000_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, ROTD_GFX_KEY);
	m_pcm2_prot->neo_pcm2_snk_1999(ym_region, ym_region_size, 16);

}

/*************************************************
 PNYAA
**************************************************/

const device_type NEOGEO_PCM2_PNYAA_CART = &device_creator<neogeo_pcm2_pnyaa_cart>;

neogeo_pcm2_pnyaa_cart::neogeo_pcm2_pnyaa_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : neogeo_pcm2_cart(mconfig, NEOGEO_PCM2_PNYAA_CART, "NEOGEO PCM2 pnyaa Cart", tag, owner, clock, "p2_pnyaa_cart", __FILE__) {}

void neogeo_pcm2_pnyaa_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->neogeo_cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->kof2000_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, PNYAA_GFX_KEY);
	m_pcm2_prot->neo_pcm2_snk_1999(ym_region, ym_region_size, 4);
}
