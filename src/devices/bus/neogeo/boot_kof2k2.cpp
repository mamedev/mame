// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************

 Neo Geo cart emulation
 The King of Fighers 2002 Bootleg cart type

 ***********************************************************************************************************/


#include "emu.h"
#include "boot_kof2k2.h"


void neogeo_kof2002b_cart_device::device_add_mconfig(machine_config &config)
{
	NEOBOOT_PROT(config, m_prot);
	NG_CMC_PROT(config, m_cmc_prot);
	NG_PCM2_PROT(config, m_pcm2_prot);
	NG_KOF2002_PROT(config, m_kof2k2_prot);
}

void neogeo_kf2k2mp_cart_device::device_add_mconfig(machine_config &config)
{
	NEOBOOT_PROT(config, m_prot);
	NG_CMC_PROT(config, m_cmc_prot);
	NG_PCM2_PROT(config, m_pcm2_prot);
}

void neogeo_kf2k2mp2_cart_device::device_add_mconfig(machine_config &config)
{
	NEOBOOT_PROT(config, m_prot);
	NG_CMC_PROT(config, m_cmc_prot);
	NG_PCM2_PROT(config, m_pcm2_prot);
}

/*************************************************
 kof2002b
 **************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_KOF2002B_CART, neogeo_kof2002b_cart_device, "neocart_kof2002b", "Neo Geo KoF 2002 Bootleg Cart")

neogeo_kof2002b_cart_device::neogeo_kof2002b_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_bootleg_cart_device(mconfig, NEOGEO_KOF2002B_CART, tag, owner, clock),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot"),
	m_kof2k2_prot(*this, "kof2k2_prot")
{
}


void neogeo_kof2002b_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_kof2k2_prot->kof2002_decrypt_68k(cpuregion, cpuregion_size);
	m_pcm2_prot->swap(ym_region, ym_region_size, 0);
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_prot->kof2002b_gfx_decrypt(spr_region, 0x4000000);
	m_prot->kof2002b_gfx_decrypt(fix_region, 0x20000);
}

/*************************************************
 kf2k2mp
 **************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_KF2K2MP_CART, neogeo_kf2k2mp_cart_device, "neocart_kf2k2mp", "Neo Geo KoF 2002 MP Cart")

neogeo_kf2k2mp_cart_device::neogeo_kf2k2mp_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_bootleg_cart_device(mconfig, NEOGEO_KF2K2MP_CART, tag, owner, clock),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot")
{
}


void neogeo_kf2k2mp_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->kf2k2mp_decrypt(cpuregion, cpuregion_size);
	m_pcm2_prot->swap(ym_region, ym_region_size, 0);
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_prot->sx_decrypt(fix_region, fix_region_size, 2);
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, KOF2002_GFX_KEY);
}

/*************************************************
 kf2k2mp2
 **************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_KF2K2MP2_CART, neogeo_kf2k2mp2_cart_device, "neocart_kf2k2mp2", "Neo Geo KoF 2002 MP2 Cart")

neogeo_kf2k2mp2_cart_device::neogeo_kf2k2mp2_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_bootleg_cart_device(mconfig, NEOGEO_KF2K2MP2_CART, tag, owner, clock),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot")
{
}


void neogeo_kf2k2mp2_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->kf2k2mp2_px_decrypt(cpuregion, cpuregion_size);
	m_pcm2_prot->swap(ym_region, ym_region_size, 0);
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_prot->sx_decrypt(fix_region, fix_region_size, 1);
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, KOF2002_GFX_KEY);
}
