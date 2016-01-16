// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood
/***********************************************************************************************************

 NEOGEO ROM cart emulation

 ***********************************************************************************************************/


#include "emu.h"
#include "sma_cart.h"


//-------------------------------------------------
//  neogeo_sma_cart - constructor
//-------------------------------------------------

const device_type NEOGEO_SMA_CART = &device_creator<neogeo_sma_cart>;


neogeo_sma_cart::neogeo_sma_cart(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT16 clock, std::string shortname, std::string source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_neogeo_cart_interface(mconfig, *this),
	m_banked_cart(*this, "banked_cart"),
	m_sma_prot(*this, "sma_prot"),
	m_cmc_prot(*this, "cmc_prot")

{
}

neogeo_sma_cart::neogeo_sma_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT16 clock)
	: device_t(mconfig, NEOGEO_SMA_CART, "NEOGEO SMA Cart", tag, owner, clock, "neogeo_rom", __FILE__),
	device_neogeo_cart_interface(mconfig, *this),
	m_banked_cart(*this, "banked_cart"),
	m_sma_prot(*this, "sma_prot"),
	m_cmc_prot(*this, "cmc_prot")
{
}


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

READ16_MEMBER(neogeo_sma_cart::read_rom)
{
	return m_rom[offset];
}

static MACHINE_CONFIG_FRAGMENT( sma_cart )
	MCFG_NEOGEO_BANKED_CART_ADD("banked_cart")
	MCFG_SMA_PROT_ADD("sma_prot")
	MCFG_CMC_PROT_ADD("cmc_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_sma_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sma_cart );
}



/* Individual cartridge types (mirror DRIVER_INIT functionality) */

/*************************************************
 KOF 99
**************************************************/

const device_type NEOGEO_SMA_KOF99_CART = &device_creator<neogeo_sma_kof99_cart>;

neogeo_sma_kof99_cart::neogeo_sma_kof99_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : neogeo_sma_cart(mconfig, NEOGEO_SMA_KOF99_CART, "NEOGEO SMA Kof99 Cart", tag, owner, clock, "sma_kof99_cart", __FILE__) {}

void neogeo_sma_kof99_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_sma_prot->kof99_decrypt_68k(cpuregion);
	m_cmc_prot->kof99_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, KOF99_GFX_KEY);
}

void neogeo_sma_kof99_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);
	m_sma_prot->kof99_install_protection(maincpu, m_banked_cart);
}




/*************************************************
 Garou
**************************************************/

const device_type NEOGEO_SMA_GAROU_CART = &device_creator<neogeo_sma_garou_cart>;

neogeo_sma_garou_cart::neogeo_sma_garou_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : neogeo_sma_cart(mconfig, NEOGEO_SMA_GAROU_CART, "NEOGEO SMA Garou Cart", tag, owner, clock, "sma_garou_cart", __FILE__) {}

void neogeo_sma_garou_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_sma_prot->garou_decrypt_68k(cpuregion);
	m_cmc_prot->kof99_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, GAROU_GFX_KEY);
}

void neogeo_sma_garou_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);
	m_sma_prot->garou_install_protection(maincpu, m_banked_cart);
}

const device_type NEOGEO_SMA_GAROUH_CART = &device_creator<neogeo_sma_garouh_cart>;

neogeo_sma_garouh_cart::neogeo_sma_garouh_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : neogeo_sma_cart(mconfig, NEOGEO_SMA_GAROU_CART, "NEOGEO SMA Garou (alt) Cart", tag, owner, clock, "sma_garouh_cart", __FILE__) {}

void neogeo_sma_garouh_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_sma_prot->garouh_decrypt_68k(cpuregion);
	m_cmc_prot->kof99_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, GAROU_GFX_KEY);
}

void neogeo_sma_garouh_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);
	m_sma_prot->garouh_install_protection(maincpu, m_banked_cart);
}


/*************************************************
 Metal Slug 3
**************************************************/

const device_type NEOGEO_SMA_MSLUG3_CART = &device_creator<neogeo_sma_mslug3_cart>;

neogeo_sma_mslug3_cart::neogeo_sma_mslug3_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : neogeo_sma_cart(mconfig, NEOGEO_SMA_MSLUG3_CART, "NEOGEO SMA Mslug3 Cart", tag, owner, clock, "sma_mslug3_cart", __FILE__) {}

void neogeo_sma_mslug3_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_sma_prot->mslug3_decrypt_68k(cpuregion);
	m_cmc_prot->kof99_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, MSLUG3_GFX_KEY);
}

void neogeo_sma_mslug3_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);
	m_sma_prot->mslug3_install_protection(maincpu, m_banked_cart);
}

/*************************************************
 KOF2000
**************************************************/

const device_type NEOGEO_SMA_KOF2000_CART = &device_creator<neogeo_sma_kof2000_cart>;

neogeo_sma_kof2000_cart::neogeo_sma_kof2000_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : neogeo_sma_cart(mconfig, NEOGEO_SMA_KOF2000_CART, "NEOGEO SMA KOF2000 Cart", tag, owner, clock, "sma_kof2000_cart", __FILE__) {}

void neogeo_sma_kof2000_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_sma_prot->kof2000_decrypt_68k(cpuregion);
	m_cmc_prot->neogeo_cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->kof2000_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, KOF2000_GFX_KEY);
}

void neogeo_sma_kof2000_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);
	m_sma_prot->kof2000_install_protection(maincpu, m_banked_cart);
}
