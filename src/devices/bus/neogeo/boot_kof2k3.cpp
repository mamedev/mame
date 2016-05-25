// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************

 Neo Geo cart emulation
 The King of Fighters 2003 Bootleg cart type

 ***********************************************************************************************************/


#include "emu.h"
#include "boot_kof2k3.h"


static MACHINE_CONFIG_FRAGMENT( kof2k3bl_cart )
	MCFG_NEOBOOT_PROT_ADD("bootleg_prot")
	MCFG_CMC_PROT_ADD("cmc_prot")
	MCFG_PCM2_PROT_ADD("pcm2_prot")
	MCFG_KOF2K3BL_PROT_ADD("kof2k3bl_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_kf2k3bl_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( kof2k3bl_cart );
}

machine_config_constructor neogeo_kf2k3pl_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( kof2k3bl_cart );
}

machine_config_constructor neogeo_kf2k3upl_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( kof2k3bl_cart );
}

/*************************************************
 kf2k3bl
 **************************************************/

const device_type NEOGEO_KF2K3BL_CART = &device_creator<neogeo_kf2k3bl_cart>;

neogeo_kf2k3bl_cart::neogeo_kf2k3bl_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_bootleg_cart(mconfig, NEOGEO_KF2K3BL_CART, "Neo Geo KOF 2003 Bootleg Cart", tag, owner, clock, "neocart_kf2k3bl", __FILE__),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot"),
	m_kof2k3bl_prot(*this, "kof2k3bl_prot")
{}

void neogeo_kf2k3bl_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, KOF2003_GFX_KEY);
	m_pcm2_prot->swap(ym_region, ym_region_size, 5);
	m_prot->sx_decrypt(fix_region, fix_region_size, 1);

}

/*************************************************
 kf2k3pl
 **************************************************/

const device_type NEOGEO_KF2K3PL_CART = &device_creator<neogeo_kf2k3pl_cart>;

neogeo_kf2k3pl_cart::neogeo_kf2k3pl_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_bootleg_cart(mconfig, NEOGEO_KF2K3PL_CART, "Neo Geo KOF 2003 PL Cart", tag, owner, clock, "neocart_kf2k3pl", __FILE__),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot"),
	m_kof2k3bl_prot(*this, "kof2k3bl_prot")
{}

void neogeo_kf2k3pl_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, KOF2003_GFX_KEY);
	m_pcm2_prot->swap(ym_region, ym_region_size, 5);
	m_kof2k3bl_prot->pl_px_decrypt(cpuregion, cpuregion_size);
	m_prot->sx_decrypt(fix_region, fix_region_size, 1);
}


/*************************************************
 kf2k3upl
 **************************************************/

const device_type NEOGEO_KF2K3UPL_CART = &device_creator<neogeo_kf2k3upl_cart>;

neogeo_kf2k3upl_cart::neogeo_kf2k3upl_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_bootleg_cart(mconfig, NEOGEO_KF2K3UPL_CART, "Neo Geo KOF 2003 UPL Cart", tag, owner, clock, "neocart_kf2k3upl", __FILE__),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot"),
	m_kof2k3bl_prot(*this, "kof2k3bl_prot")
{}

void neogeo_kf2k3upl_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, KOF2003_GFX_KEY);
	m_pcm2_prot->swap(ym_region, ym_region_size, 5);
	m_kof2k3bl_prot->upl_px_decrypt(cpuregion, cpuregion_size);
	m_prot->sx_decrypt(fix_region, fix_region_size, 2);
}
