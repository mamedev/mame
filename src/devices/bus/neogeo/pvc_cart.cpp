// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood
/***********************************************************************************************************

 NEOGEO ROM cart emulation

 ***********************************************************************************************************/


#include "emu.h"
#include "pvc_cart.h"


//-------------------------------------------------
//  neogeo_pvc_cart - constructor
//-------------------------------------------------

const device_type NEOGEO_PVC_CART = &device_creator<neogeo_pvc_cart>;


neogeo_pvc_cart::neogeo_pvc_cart(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT16 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_neogeo_cart_interface(mconfig, *this),
	m_banked_cart(*this, "banked_cart"),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot"),
	m_pvc_prot(*this, "pvc_prot")
{
}

neogeo_pvc_cart::neogeo_pvc_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock)
	: device_t(mconfig, NEOGEO_PVC_CART, "NEOGEO PCM2 Cart", tag, owner, clock, "neogeo_rom", __FILE__),
	device_neogeo_cart_interface(mconfig, *this),
	m_banked_cart(*this, "banked_cart"),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot"),
	m_pvc_prot(*this, "pvc_prot")
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_pvc_cart::device_start()
{
}

void neogeo_pvc_cart::device_reset()
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ16_MEMBER(neogeo_pvc_cart::read_rom)
{
	return m_rom[offset];
}

static MACHINE_CONFIG_FRAGMENT( pvc_cart )
	MCFG_NEOGEO_BANKED_CART_ADD("banked_cart")
	MCFG_CMC_PROT_ADD("cmc_prot")
	MCFG_PCM2_PROT_ADD("pcm2_prot")
	MCFG_PVC_PROT_ADD("pvc_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_pvc_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( pvc_cart );
}



/* Individual cartridge types (mirror DRIVER_INIT functionality) */

/*************************************************
 MSLUG5
**************************************************/

const device_type NEOGEO_PVC_MSLUG5_CART = &device_creator<neogeo_pvc_mslug5_cart>;

neogeo_pvc_mslug5_cart::neogeo_pvc_mslug5_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : neogeo_pvc_cart(mconfig, NEOGEO_PVC_MSLUG5_CART, "NEOGEO PCM2 mslug5 Cart", tag, owner, clock, "p2_mslug5_cart", __FILE__) {}

void neogeo_pvc_mslug5_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_pvc_prot->mslug5_decrypt_68k(cpuregion, cpuregion_size);
	m_pcm2_prot->neo_pcm2_swap(ym_region, ym_region_size, 2);
	m_cmc_prot->neogeo_cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->kof2000_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, MSLUG5_GFX_KEY);
}

/*************************************************
 SVC
**************************************************/

const device_type NEOGEO_PVC_SVC_CART = &device_creator<neogeo_pvc_svc_cart>;

neogeo_pvc_svc_cart::neogeo_pvc_svc_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : neogeo_pvc_cart(mconfig, NEOGEO_PVC_SVC_CART, "NEOGEO PCM2 svc Cart", tag, owner, clock, "p2_svc_cart", __FILE__) {}

void neogeo_pvc_svc_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_pvc_prot->svc_px_decrypt(cpuregion, cpuregion_size);
	m_pcm2_prot->neo_pcm2_swap(ym_region, ym_region_size, 3);
	m_cmc_prot->neogeo_cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->kof2000_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, SVC_GFX_KEY);
}


/*************************************************
 KOF2003
**************************************************/

const device_type NEOGEO_PVC_KOF2003_CART = &device_creator<neogeo_pvc_kof2003_cart>;

neogeo_pvc_kof2003_cart::neogeo_pvc_kof2003_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : neogeo_pvc_cart(mconfig, NEOGEO_PVC_KOF2003_CART, "NEOGEO PCM2 kof2003 Cart", tag, owner, clock, "p2_kof2003_cart", __FILE__) {}

void neogeo_pvc_kof2003_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_pvc_prot->kof2003_decrypt_68k(cpuregion, cpuregion_size);
	m_pcm2_prot->neo_pcm2_swap(ym_region, ym_region_size, 5);
	m_cmc_prot->neogeo_cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->kof2000_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, KOF2003_GFX_KEY);
}

const device_type NEOGEO_PVC_KOF2003H_CART = &device_creator<neogeo_pvc_kof2003h_cart>;

neogeo_pvc_kof2003h_cart::neogeo_pvc_kof2003h_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : neogeo_pvc_cart(mconfig, NEOGEO_PVC_KOF2003H_CART, "NEOGEO PCM2 kof2003h Cart", tag, owner, clock, "p2_kof2003h_cart", __FILE__) {}

void neogeo_pvc_kof2003h_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_pvc_prot->kof2003h_decrypt_68k(cpuregion, cpuregion_size);
	m_pcm2_prot->neo_pcm2_swap(ym_region, ym_region_size, 5);
	m_cmc_prot->neogeo_cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->kof2000_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, KOF2003_GFX_KEY);
}
