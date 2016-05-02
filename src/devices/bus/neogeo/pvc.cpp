// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************
 
 Neo Geo cart emulation
 PVC encrypted cart type (+ CMC + PCM2)

 ***********************************************************************************************************/


#include "emu.h"
#include "pvc.h"


//-------------------------------------------------
//  neogeo_pvc_cart - constructor
//-------------------------------------------------

const device_type NEOGEO_PVC_CART = &device_creator<neogeo_pvc_cart>;


neogeo_pvc_cart::neogeo_pvc_cart(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT16 clock, const char *shortname, const char *source) :
	neogeo_rom_device(mconfig, type, name, tag, owner, clock, shortname, source),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot"),
	m_pvc_prot(*this, "pvc_prot")
{}

neogeo_pvc_cart::neogeo_pvc_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock) :
	neogeo_rom_device(mconfig, NEOGEO_PVC_CART, "Neo Geo PVC Cart", tag, owner, clock, "neocart_pvc", __FILE__),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot"),
	m_pvc_prot(*this, "pvc_prot")
{}


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

static MACHINE_CONFIG_FRAGMENT( pvc_cart )
	MCFG_CMC_PROT_ADD("cmc_prot")
	MCFG_PCM2_PROT_ADD("pcm2_prot")
	MCFG_PVC_PROT_ADD("pvc_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_pvc_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( pvc_cart );
}




/*************************************************
 mslug5
**************************************************/

const device_type NEOGEO_PVC_MSLUG5_CART = &device_creator<neogeo_pvc_mslug5_cart>;

neogeo_pvc_mslug5_cart::neogeo_pvc_mslug5_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_pvc_cart(mconfig, NEOGEO_PVC_MSLUG5_CART, "Neo Geo Metal Slug 5 PVC Cart", tag, owner, clock, "neocart_mslug5", __FILE__) {}

void neogeo_pvc_mslug5_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_pvc_prot->mslug5_decrypt_68k(cpuregion, cpuregion_size);
	m_pcm2_prot->swap(ym_region, ym_region_size, 2);
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, MSLUG5_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}

/*************************************************
 svc
**************************************************/

const device_type NEOGEO_PVC_SVC_CART = &device_creator<neogeo_pvc_svc_cart>;

neogeo_pvc_svc_cart::neogeo_pvc_svc_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_pvc_cart(mconfig, NEOGEO_PVC_SVC_CART, "Neo Geo SNK vs Capcom PVC Cart", tag, owner, clock, "neocart_svc", __FILE__)
{}

void neogeo_pvc_svc_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_pvc_prot->svc_px_decrypt(cpuregion, cpuregion_size);
	m_pcm2_prot->swap(ym_region, ym_region_size, 3);
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, SVC_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}


/*************************************************
 kof2003
**************************************************/

const device_type NEOGEO_PVC_KOF2003_CART = &device_creator<neogeo_pvc_kof2003_cart>;

neogeo_pvc_kof2003_cart::neogeo_pvc_kof2003_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_pvc_cart(mconfig, NEOGEO_PVC_KOF2003_CART, "Neo Geo KOF 2003 PVC Cart", tag, owner, clock, "neocart_kof2003", __FILE__)
{}

void neogeo_pvc_kof2003_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_pvc_prot->kof2003_decrypt_68k(cpuregion, cpuregion_size);
	m_pcm2_prot->swap(ym_region, ym_region_size, 5);
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, KOF2003_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}

/*************************************************
 kof2003h
 **************************************************/

const device_type NEOGEO_PVC_KOF2003H_CART = &device_creator<neogeo_pvc_kof2003h_cart>;

neogeo_pvc_kof2003h_cart::neogeo_pvc_kof2003h_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_pvc_cart(mconfig, NEOGEO_PVC_KOF2003H_CART, "Neo Geo KOF 2003 AES PVC Cart", tag, owner, clock, "neocart_kof2003h", __FILE__)
{}

void neogeo_pvc_kof2003h_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_pvc_prot->kof2003h_decrypt_68k(cpuregion, cpuregion_size);
	m_pcm2_prot->swap(ym_region, ym_region_size, 5);
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, KOF2003_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}
