// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood
/***********************************************************************************************************

 NEOGEO ROM cart emulation

  these are bootlegs using a mix of reimplemented original features, could be further sorted

 ***********************************************************************************************************/


#include "emu.h"
#include "bootleg_hybrid_cart.h"


//-------------------------------------------------
//  neogeo_bootleg_hybrid_hybrid_cart - constructor
//-------------------------------------------------

const device_type NEOGEO_BOOTLEG_HYBRID_HYBRID_CART = &device_creator<neogeo_bootleg_hybrid_hybrid_cart>;


neogeo_bootleg_hybrid_hybrid_cart::neogeo_bootleg_hybrid_hybrid_cart(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT16 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_neogeo_cart_interface(mconfig, *this),
	m_banked_cart(*this, "banked_cart"),
	m_bootleg_prot(*this, "bootleg_prot"),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot"),
	m_kof2002_prot(*this, "kof2002_prot"),
	m_pvc_prot(*this, "pvc_prot")
{
}

neogeo_bootleg_hybrid_hybrid_cart::neogeo_bootleg_hybrid_hybrid_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock)
	: device_t(mconfig, NEOGEO_BOOTLEG_HYBRID_HYBRID_CART, "NEOGEO SMA Cart", tag, owner, clock, "neogeo_rom", __FILE__),
	device_neogeo_cart_interface(mconfig, *this),
	m_banked_cart(*this, "banked_cart"),
	m_bootleg_prot(*this, "bootleg_prot"),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot"),
	m_kof2002_prot(*this, "kof2002_prot"),
	m_pvc_prot(*this, "pvc_prot")

{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_bootleg_hybrid_hybrid_cart::device_start()
{
}

void neogeo_bootleg_hybrid_hybrid_cart::device_reset()
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ16_MEMBER(neogeo_bootleg_hybrid_hybrid_cart::read_rom)
{
	return m_rom[offset];
}

static MACHINE_CONFIG_FRAGMENT( bootleg_hybrid_cart )
	MCFG_NEOGEO_BANKED_CART_ADD("banked_cart")
	MCFG_NGBOOTLEG_PROT_ADD("bootleg_prot")
	MCFG_CMC_PROT_ADD("cmc_prot")
	MCFG_PCM2_PROT_ADD("pcm2_prot")
	MCFG_KOF2002_PROT_ADD("kof2002_prot")
	MCFG_PVC_PROT_ADD("pvc_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_bootleg_hybrid_hybrid_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( bootleg_hybrid_cart );
}



/* Individual cartridge types (mirror DRIVER_INIT functionality) */


/*************************************************
 mslug3b6
**************************************************/

const device_type NEOGEO_BOOTLEG_HYBRID_MSLUG3B6_CART = &device_creator<neogeo_bootleg_hybrid_mslug3b6_cart>;

neogeo_bootleg_hybrid_mslug3b6_cart::neogeo_bootleg_hybrid_mslug3b6_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : neogeo_bootleg_hybrid_hybrid_cart(mconfig, NEOGEO_BOOTLEG_HYBRID_MSLUG3B6_CART, "NEOGEO BOOT mslug3b6 Cart", tag, owner, clock, "boot_mslug3b6_cart", __FILE__) {}

void neogeo_bootleg_hybrid_mslug3b6_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);

}

void neogeo_bootleg_hybrid_mslug3b6_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_bootleg_prot->neogeo_bootleg_sx_decrypt(fix_region, fix_region_size,2);
	m_cmc_prot->cmc42_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, MSLUG3_GFX_KEY);
}

/*************************************************
 kof2002b
**************************************************/

const device_type NEOGEO_BOOTLEG_HYBRID_KOF2002B_CART = &device_creator<neogeo_bootleg_hybrid_kof2002b_cart>;

neogeo_bootleg_hybrid_kof2002b_cart::neogeo_bootleg_hybrid_kof2002b_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : neogeo_bootleg_hybrid_hybrid_cart(mconfig, NEOGEO_BOOTLEG_HYBRID_KOF2002B_CART, "NEOGEO BOOT kof2002b Cart", tag, owner, clock, "boot_kof2002b_cart", __FILE__) {}

void neogeo_bootleg_hybrid_kof2002b_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);

}

void neogeo_bootleg_hybrid_kof2002b_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_kof2002_prot->kof2002_decrypt_68k(cpuregion, cpuregion_size);
	m_pcm2_prot->neo_pcm2_swap(ym_region, ym_region_size, 0);
	m_cmc_prot->neogeo_cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_bootleg_prot->kof2002b_gfx_decrypt(spr_region,0x4000000);
	m_bootleg_prot->kof2002b_gfx_decrypt(fix_region,0x20000);
}

/***/

const device_type NEOGEO_BOOTLEG_HYBRID_KF2K2MP_CART = &device_creator<neogeo_bootleg_hybrid_kf2k2mp_cart>;

neogeo_bootleg_hybrid_kf2k2mp_cart::neogeo_bootleg_hybrid_kf2k2mp_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : neogeo_bootleg_hybrid_hybrid_cart(mconfig, NEOGEO_BOOTLEG_HYBRID_KF2K2MP_CART, "NEOGEO BOOT kf2k2mp Cart", tag, owner, clock, "boot_kf2k2mp_cart", __FILE__) {}

void neogeo_bootleg_hybrid_kf2k2mp_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);

}

void neogeo_bootleg_hybrid_kf2k2mp_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_bootleg_prot->kf2k2mp_decrypt(cpuregion, cpuregion_size);
	m_pcm2_prot->neo_pcm2_swap(ym_region, ym_region_size, 0);
	m_cmc_prot->neogeo_cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_bootleg_prot->neogeo_bootleg_sx_decrypt(fix_region, fix_region_size,2);
	m_cmc_prot->cmc50_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, KOF2002_GFX_KEY);
}

/***/

const device_type NEOGEO_BOOTLEG_HYBRID_KF2K2MP2_CART = &device_creator<neogeo_bootleg_hybrid_kf2k2mp2_cart>;

neogeo_bootleg_hybrid_kf2k2mp2_cart::neogeo_bootleg_hybrid_kf2k2mp2_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : neogeo_bootleg_hybrid_hybrid_cart(mconfig, NEOGEO_BOOTLEG_HYBRID_KF2K2MP2_CART, "NEOGEO BOOT kf2k2mp2 Cart", tag, owner, clock, "boot_kf2k2mp2_cart", __FILE__) {}

void neogeo_bootleg_hybrid_kf2k2mp2_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);

}

void neogeo_bootleg_hybrid_kf2k2mp2_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_bootleg_prot->kf2k2mp2_px_decrypt(cpuregion, cpuregion_size);
	m_pcm2_prot->neo_pcm2_swap(ym_region, ym_region_size, 0);
	m_cmc_prot->neogeo_cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	m_bootleg_prot->neogeo_bootleg_sx_decrypt(fix_region, fix_region_size,1);
	m_cmc_prot->cmc50_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, KOF2002_GFX_KEY);
}

/*************************************************
 matrimbl
**************************************************/

const device_type NEOGEO_BOOTLEG_HYBRID_MATRIMBL_CART = &device_creator<neogeo_bootleg_hybrid_matrimbl_cart>;

neogeo_bootleg_hybrid_matrimbl_cart::neogeo_bootleg_hybrid_matrimbl_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : neogeo_bootleg_hybrid_hybrid_cart(mconfig, NEOGEO_BOOTLEG_HYBRID_MATRIMBL_CART, "NEOGEO BOOT matrimbl Cart", tag, owner, clock, "boot_matrimbl_cart", __FILE__) {}

void neogeo_bootleg_hybrid_matrimbl_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);

}

void neogeo_bootleg_hybrid_matrimbl_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_kof2002_prot->matrim_decrypt_68k(cpuregion, cpuregion_size);
	m_bootleg_prot->matrimbl_decrypt(spr_region, spr_region_size, audiocpu_region,audio_region_size);
	m_cmc_prot->neogeo_sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size); /* required for text layer */
}

/*************************************************
 ms5plus
**************************************************/

const device_type NEOGEO_BOOTLEG_HYBRID_MS5PLUS_CART = &device_creator<neogeo_bootleg_hybrid_ms5plus_cart>;

neogeo_bootleg_hybrid_ms5plus_cart::neogeo_bootleg_hybrid_ms5plus_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : neogeo_bootleg_hybrid_hybrid_cart(mconfig, NEOGEO_BOOTLEG_HYBRID_MS5PLUS_CART, "NEOGEO BOOT ms5plus Cart", tag, owner, clock, "boot_ms5plus_cart", __FILE__) {}

void neogeo_bootleg_hybrid_ms5plus_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);
	m_bootleg_prot->install_ms5plus_protection(maincpu,m_banked_cart);
}

void neogeo_bootleg_hybrid_ms5plus_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->cmc50_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, MSLUG5_GFX_KEY);
	m_pcm2_prot->neo_pcm2_swap(ym_region, ym_region_size, 2);
	m_bootleg_prot->neogeo_bootleg_sx_decrypt(fix_region, fix_region_size,1);
	m_cmc_prot->neogeo_cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region,audio_region_size);
}

/*************************************************
 svcboot
**************************************************/

const device_type NEOGEO_BOOTLEG_HYBRID_SVCBOOT_CART = &device_creator<neogeo_bootleg_hybrid_svcboot_cart>;

neogeo_bootleg_hybrid_svcboot_cart::neogeo_bootleg_hybrid_svcboot_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : neogeo_bootleg_hybrid_hybrid_cart(mconfig, NEOGEO_BOOTLEG_HYBRID_SVCBOOT_CART, "NEOGEO BOOT svcboot Cart", tag, owner, clock, "boot_svcboot_cart", __FILE__) {}

void neogeo_bootleg_hybrid_svcboot_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);
	m_pvc_prot->install_pvc_protection(maincpu,m_banked_cart);
}

void neogeo_bootleg_hybrid_svcboot_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_bootleg_prot->svcboot_px_decrypt(cpuregion, cpuregion_size);
	m_bootleg_prot->svcboot_cx_decrypt(spr_region, spr_region_size);
}

/***/

const device_type NEOGEO_BOOTLEG_HYBRID_SVCSPLUS_CART = &device_creator<neogeo_bootleg_hybrid_svcsplus_cart>;

neogeo_bootleg_hybrid_svcsplus_cart::neogeo_bootleg_hybrid_svcsplus_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : neogeo_bootleg_hybrid_hybrid_cart(mconfig, NEOGEO_BOOTLEG_HYBRID_SVCSPLUS_CART, "NEOGEO BOOT svcsplus Cart", tag, owner, clock, "boot_svcsplus_cart", __FILE__) {}

void neogeo_bootleg_hybrid_svcsplus_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);
	m_pvc_prot->install_pvc_protection(maincpu,m_banked_cart);

}

void neogeo_bootleg_hybrid_svcsplus_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_bootleg_prot->svcsplus_px_decrypt(cpuregion, cpuregion_size);
	m_bootleg_prot->neogeo_bootleg_sx_decrypt(fix_region, fix_region_size,2);
	m_bootleg_prot->svcboot_cx_decrypt(spr_region, spr_region_size);
	m_bootleg_prot->svcsplus_px_hack(cpuregion, cpuregion_size);
}


/*************************************************
 kf2k3bl
**************************************************/

const device_type NEOGEO_BOOTLEG_HYBRID_KF2K3BL_CART = &device_creator<neogeo_bootleg_hybrid_kf2k3bl_cart>;

neogeo_bootleg_hybrid_kf2k3bl_cart::neogeo_bootleg_hybrid_kf2k3bl_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : neogeo_bootleg_hybrid_hybrid_cart(mconfig, NEOGEO_BOOTLEG_HYBRID_KF2K3BL_CART, "NEOGEO BOOT kf2k3bl Cart", tag, owner, clock, "boot_kf2k3bl_cart", __FILE__) {}

void neogeo_bootleg_hybrid_kf2k3bl_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);
	m_bootleg_prot->kf2k3bl_install_protection(maincpu,m_banked_cart, cpuregion, cpuregion_size);

}

void neogeo_bootleg_hybrid_kf2k3bl_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->cmc50_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, KOF2003_GFX_KEY);
	m_pcm2_prot->neo_pcm2_swap(ym_region, ym_region_size, 5);
	m_bootleg_prot->neogeo_bootleg_sx_decrypt(fix_region, fix_region_size,1);

}

const device_type NEOGEO_BOOTLEG_HYBRID_KF2K3PL_CART = &device_creator<neogeo_bootleg_hybrid_kf2k3pl_cart>;

neogeo_bootleg_hybrid_kf2k3pl_cart::neogeo_bootleg_hybrid_kf2k3pl_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : neogeo_bootleg_hybrid_hybrid_cart(mconfig, NEOGEO_BOOTLEG_HYBRID_KF2K3PL_CART, "NEOGEO BOOT kf2k3pl Cart", tag, owner, clock, "boot_kf2k3pl_cart", __FILE__) {}

void neogeo_bootleg_hybrid_kf2k3pl_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);
	m_bootleg_prot->kf2k3pl_install_protection(maincpu,m_banked_cart, cpuregion, cpuregion_size);
}

void neogeo_bootleg_hybrid_kf2k3pl_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->cmc50_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, KOF2003_GFX_KEY);
	m_pcm2_prot->neo_pcm2_swap(ym_region, ym_region_size, 5);
	m_bootleg_prot->kf2k3pl_px_decrypt(cpuregion, cpuregion_size);
	m_bootleg_prot->neogeo_bootleg_sx_decrypt(fix_region, fix_region_size,1);
}


const device_type NEOGEO_BOOTLEG_HYBRID_KF2K3UPL_CART = &device_creator<neogeo_bootleg_hybrid_kf2k3upl_cart>;

neogeo_bootleg_hybrid_kf2k3upl_cart::neogeo_bootleg_hybrid_kf2k3upl_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : neogeo_bootleg_hybrid_hybrid_cart(mconfig, NEOGEO_BOOTLEG_HYBRID_KF2K3UPL_CART, "NEOGEO BOOT kf2k3upl Cart", tag, owner, clock, "boot_kf2k3upl_cart", __FILE__) {}

void neogeo_bootleg_hybrid_kf2k3upl_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);
	m_bootleg_prot->kf2k3bl_install_protection(maincpu,m_banked_cart, cpuregion, cpuregion_size);

}

void neogeo_bootleg_hybrid_kf2k3upl_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->cmc50_neogeo_gfx_decrypt(spr_region, spr_region_size, fix_region, fix_region_size, KOF2003_GFX_KEY);
	m_pcm2_prot->neo_pcm2_swap(ym_region, ym_region_size, 5);
	m_bootleg_prot->kf2k3upl_px_decrypt(cpuregion, cpuregion_size);
	m_bootleg_prot->neogeo_bootleg_sx_decrypt(fix_region, fix_region_size,2);
}
