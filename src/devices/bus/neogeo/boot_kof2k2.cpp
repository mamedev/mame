// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************

 Neo Geo cart emulation
 The King of Fighers 2002 Bootleg cart type

 ***********************************************************************************************************/


#include "emu.h"
#include "boot_kof2k2.h"


static MACHINE_CONFIG_FRAGMENT( kof2k2bl_cart )
	MCFG_NEOBOOT_PROT_ADD("bootleg_prot")
	MCFG_CMC_PROT_ADD("cmc_prot")
	MCFG_PCM2_PROT_ADD("pcm2_prot")
	MCFG_KOF2002_PROT_ADD("kof2k2_prot")
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( kof2k2mp_cart )
	MCFG_NEOBOOT_PROT_ADD("bootleg_prot")
	MCFG_CMC_PROT_ADD("cmc_prot")
	MCFG_PCM2_PROT_ADD("pcm2_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_kof2002b_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( kof2k2bl_cart );
}

machine_config_constructor neogeo_kf2k2mp_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( kof2k2mp_cart );
}

machine_config_constructor neogeo_kf2k2mp2_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( kof2k2mp_cart );
}

/*************************************************
 kof2002b
 **************************************************/

const device_type NEOGEO_KOF2002B_CART = &device_creator<neogeo_kof2002b_cart>;

neogeo_kof2002b_cart::neogeo_kof2002b_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_bootleg_cart(mconfig, NEOGEO_KOF2002B_CART, "Neo Geo KOF 2002 Bootleg Cart", tag, owner, clock, "neocart_kof2002b", __FILE__),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot"),
	m_kof2k2_prot(*this, "kof2k2_prot")
{}


void neogeo_kof2002b_cart::decrypt_all(DECRYPT_ALL_PARAMS)
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

const device_type NEOGEO_KF2K2MP_CART = &device_creator<neogeo_kf2k2mp_cart>;

neogeo_kf2k2mp_cart::neogeo_kf2k2mp_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_bootleg_cart(mconfig, NEOGEO_KF2K2MP_CART, "Neo Geo KOF 2002 MP Cart", tag, owner, clock, "neocart_kf2k2mp", __FILE__),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot")
{}


void neogeo_kf2k2mp_cart::decrypt_all(DECRYPT_ALL_PARAMS)
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

const device_type NEOGEO_KF2K2MP2_CART = &device_creator<neogeo_kf2k2mp2_cart>;

neogeo_kf2k2mp2_cart::neogeo_kf2k2mp2_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_bootleg_cart(mconfig, NEOGEO_KF2K2MP2_CART, "Neo Geo KOF 2002 MP2 Cart", tag, owner, clock, "neocart_kf2k2mp2", __FILE__),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot")
{}


void neogeo_kf2k2mp2_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->kf2k2mp2_px_decrypt(cpuregion, cpuregion_size);
	m_pcm2_prot->swap(ym_region, ym_region_size, 0);
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_prot->sx_decrypt(fix_region, fix_region_size, 1);
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, KOF2002_GFX_KEY);
}
