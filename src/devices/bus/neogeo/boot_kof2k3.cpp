// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************

 Neo Geo cart emulation
 The King of Fighters 2003 Bootleg cart type

 ***********************************************************************************************************/


#include "emu.h"
#include "boot_kof2k3.h"


MACHINE_CONFIG_MEMBER( neogeo_kf2k3bl_cart_device::device_add_mconfig )
	MCFG_NEOBOOT_PROT_ADD("bootleg_prot")
	MCFG_CMC_PROT_ADD("cmc_prot")
	MCFG_PCM2_PROT_ADD("pcm2_prot")
	MCFG_KOF2K3BL_PROT_ADD("kof2k3bl_prot")
MACHINE_CONFIG_END

MACHINE_CONFIG_MEMBER( neogeo_kf2k3pl_cart_device::device_add_mconfig )
	MCFG_NEOBOOT_PROT_ADD("bootleg_prot")
	MCFG_CMC_PROT_ADD("cmc_prot")
	MCFG_PCM2_PROT_ADD("pcm2_prot")
	MCFG_KOF2K3BL_PROT_ADD("kof2k3bl_prot")
MACHINE_CONFIG_END

MACHINE_CONFIG_MEMBER( neogeo_kf2k3upl_cart_device::device_add_mconfig )
	MCFG_NEOBOOT_PROT_ADD("bootleg_prot")
	MCFG_CMC_PROT_ADD("cmc_prot")
	MCFG_PCM2_PROT_ADD("pcm2_prot")
	MCFG_KOF2K3BL_PROT_ADD("kof2k3bl_prot")
MACHINE_CONFIG_END

/*************************************************
 kf2k3bl
 **************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_KF2K3BL_CART, neogeo_kf2k3bl_cart_device, "neocart_kf2k3bl", "Neo Geo KoF 2003 Bootleg Cart")

neogeo_kf2k3bl_cart_device::neogeo_kf2k3bl_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_bootleg_cart_device(mconfig, NEOGEO_KF2K3BL_CART, tag, owner, clock),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot"),
	m_kof2k3bl_prot(*this, "kof2k3bl_prot")
{
}

void neogeo_kf2k3bl_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, KOF2003_GFX_KEY);
	m_pcm2_prot->swap(ym_region, ym_region_size, 5);
	m_prot->sx_decrypt(fix_region, fix_region_size, 1);

}

/*************************************************
 kf2k3pl
 **************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_KF2K3PL_CART, neogeo_kf2k3pl_cart_device, "neocart_kf2k3pl", "Neo Geo KoF 2003 PL Cart")

neogeo_kf2k3pl_cart_device::neogeo_kf2k3pl_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_bootleg_cart_device(mconfig, NEOGEO_KF2K3PL_CART, tag, owner, clock),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot"),
	m_kof2k3bl_prot(*this, "kof2k3bl_prot")
{
}

void neogeo_kf2k3pl_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, KOF2003_GFX_KEY);
	m_pcm2_prot->swap(ym_region, ym_region_size, 5);
	m_kof2k3bl_prot->pl_px_decrypt(cpuregion, cpuregion_size);
	m_prot->sx_decrypt(fix_region, fix_region_size, 1);
}


/*************************************************
 kf2k3upl
 **************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_KF2K3UPL_CART, neogeo_kf2k3upl_cart_device, "neocart_kf2k3upl", "Neo Geo KoF 2003 UPL Cart")

neogeo_kf2k3upl_cart_device::neogeo_kf2k3upl_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_bootleg_cart_device(mconfig, NEOGEO_KF2K3UPL_CART, tag, owner, clock),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot"),
	m_kof2k3bl_prot(*this, "kof2k3bl_prot")
{
}

void neogeo_kf2k3upl_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, KOF2003_GFX_KEY);
	m_pcm2_prot->swap(ym_region, ym_region_size, 5);
	m_kof2k3bl_prot->upl_px_decrypt(cpuregion, cpuregion_size);
	m_prot->sx_decrypt(fix_region, fix_region_size, 2);
}
