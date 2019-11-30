// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************

 Neo Geo cart emulation
 PVC encrypted cart type (+ CMC + PCM2)

 ***********************************************************************************************************/


#include "emu.h"
#include "pvc.h"


//-------------------------------------------------
//  neogeo_pvc_cart_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NEOGEO_PVC_CART, neogeo_pvc_cart_device, "neocart_pvc", "Neo Geo PVC Cart")


neogeo_pvc_cart_device::neogeo_pvc_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint16_t clock) :
	neogeo_rom_device(mconfig, type, tag, owner, clock),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot"),
	m_pvc_prot(*this, "pvc_prot")
{
}

neogeo_pvc_cart_device::neogeo_pvc_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock) :
	neogeo_pvc_cart_device(mconfig, NEOGEO_PVC_CART, tag, owner, clock)
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_pvc_cart_device::device_start()
{
}

void neogeo_pvc_cart_device::device_reset()
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

void neogeo_pvc_cart_device::device_add_mconfig(machine_config &config)
{
	NG_CMC_PROT(config, m_cmc_prot);
	NG_PCM2_PROT(config, m_pcm2_prot);
	NG_PVC_PROT(config, m_pvc_prot);
}


/*************************************************
 mslug5
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_PVC_MSLUG5_CART, neogeo_pvc_mslug5_cart_device, "neocart_mslug5", "Neo Geo Metal Slug 5 PVC Cart")

neogeo_pvc_mslug5_cart_device::neogeo_pvc_mslug5_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_pvc_cart_device(mconfig, NEOGEO_PVC_MSLUG5_CART, tag, owner, clock)
{
}

void neogeo_pvc_mslug5_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
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

DEFINE_DEVICE_TYPE(NEOGEO_PVC_SVC_CART, neogeo_pvc_svc_cart_device, "neocart_svc", "Neo Geo SNK vs Capcom PVC Cart")

neogeo_pvc_svc_cart_device::neogeo_pvc_svc_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_pvc_cart_device(mconfig, NEOGEO_PVC_SVC_CART, tag, owner, clock)
{
}

void neogeo_pvc_svc_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
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

DEFINE_DEVICE_TYPE(NEOGEO_PVC_KOF2003_CART, neogeo_pvc_kof2003_cart_device, "neocart_kof2003", "Neo Geo KoF 2003 PVC Cart")

neogeo_pvc_kof2003_cart_device::neogeo_pvc_kof2003_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_pvc_cart_device(mconfig, NEOGEO_PVC_KOF2003_CART, tag, owner, clock)
{
}

void neogeo_pvc_kof2003_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
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

DEFINE_DEVICE_TYPE(NEOGEO_PVC_KOF2003H_CART, neogeo_pvc_kof2003h_cart_device, "neocart_kof2003h", "Neo Geo KoF 2003 AES PVC Cart")

neogeo_pvc_kof2003h_cart_device::neogeo_pvc_kof2003h_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_pvc_cart_device(mconfig, NEOGEO_PVC_KOF2003H_CART, tag, owner, clock)
{
}

void neogeo_pvc_kof2003h_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_pvc_prot->kof2003h_decrypt_68k(cpuregion, cpuregion_size);
	m_pcm2_prot->swap(ym_region, ym_region_size, 5);
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, KOF2003_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
}
