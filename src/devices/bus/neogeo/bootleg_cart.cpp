// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood
/***********************************************************************************************************

 NEOGEO ROM cart emulation

 ***********************************************************************************************************/


#include "emu.h"
#include "bootleg_cart.h"


//-------------------------------------------------
//  neogeo_bootleg_cart - constructor
//-------------------------------------------------

const device_type NEOGEO_BOOTLEG_CART = &device_creator<neogeo_bootleg_cart>;


neogeo_bootleg_cart::neogeo_bootleg_cart(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT16 clock, std::string shortname, std::string source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_neogeo_cart_interface(mconfig, *this),
	m_banked_cart(*this, "banked_cart"),
	m_bootleg_prot(*this, "bootleg_prot")

{
}

neogeo_bootleg_cart::neogeo_bootleg_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT16 clock)
	: device_t(mconfig, NEOGEO_BOOTLEG_CART, "NEOGEO Bootleg Cart", tag, owner, clock, "neogeo_rom", __FILE__),
	device_neogeo_cart_interface(mconfig, *this),
	m_banked_cart(*this, "banked_cart"),
	m_bootleg_prot(*this, "bootleg_prot")
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_bootleg_cart::device_start()
{
}

void neogeo_bootleg_cart::device_reset()
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ16_MEMBER(neogeo_bootleg_cart::read_rom)
{
	return m_rom[offset];
}

static MACHINE_CONFIG_FRAGMENT( bootleg_cart )
	MCFG_NEOGEO_BANKED_CART_ADD("banked_cart")
	MCFG_NGBOOTLEG_PROT_ADD("bootleg_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_bootleg_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( bootleg_cart );
}



/* Individual cartridge types (mirror DRIVER_INIT functionality) */


/*************************************************
 garoubl
**************************************************/

const device_type NEOGEO_BOOTLEG_GAROUBL_CART = &device_creator<neogeo_bootleg_garoubl_cart>;

neogeo_bootleg_garoubl_cart::neogeo_bootleg_garoubl_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : neogeo_bootleg_cart(mconfig, NEOGEO_BOOTLEG_GAROUBL_CART, "NEOGEO BOOT garoubl Cart", tag, owner, clock, "boot_garoubl_cart", __FILE__) {}

void neogeo_bootleg_garoubl_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);

}

void neogeo_bootleg_garoubl_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_bootleg_prot->neogeo_bootleg_sx_decrypt(fix_region, fix_region_size,2);
	m_bootleg_prot->neogeo_bootleg_cx_decrypt(spr_region, spr_region_size);
}

/*************************************************
 cthd2003
**************************************************/

const device_type NEOGEO_BOOTLEG_CTHD2003_CART = &device_creator<neogeo_bootleg_cthd2003_cart>;

neogeo_bootleg_cthd2003_cart::neogeo_bootleg_cthd2003_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : neogeo_bootleg_cart(mconfig, NEOGEO_BOOTLEG_CTHD2003_CART, "NEOGEO BOOT cthd2003 Cart", tag, owner, clock, "boot_cthd2003_cart", __FILE__) {}

void neogeo_bootleg_cthd2003_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);
	m_bootleg_prot->patch_cthd2003(maincpu,m_banked_cart, cpuregion, cpuregion_size);
}

void neogeo_bootleg_cthd2003_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_bootleg_prot->decrypt_cthd2003(spr_region, spr_region_size, audiocpu_region, audio_region_size, fix_region, fix_region_size);
}

/******/

const device_type NEOGEO_BOOTLEG_CT2K3SP_CART = &device_creator<neogeo_bootleg_ct2k3sp_cart>;

neogeo_bootleg_ct2k3sp_cart::neogeo_bootleg_ct2k3sp_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : neogeo_bootleg_cart(mconfig, NEOGEO_BOOTLEG_CT2K3SP_CART, "NEOGEO BOOT ct2k3sp Cart", tag, owner, clock, "boot_ct2k3sp_cart", __FILE__) {}

void neogeo_bootleg_ct2k3sp_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);
	m_bootleg_prot->patch_cthd2003(maincpu,m_banked_cart, cpuregion, cpuregion_size);
}

void neogeo_bootleg_ct2k3sp_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_bootleg_prot->decrypt_ct2k3sp(spr_region, spr_region_size, audiocpu_region, audio_region_size, fix_region, fix_region_size);
}

/******/

const device_type NEOGEO_BOOTLEG_CT2K3SA_CART = &device_creator<neogeo_bootleg_ct2k3sa_cart>;

neogeo_bootleg_ct2k3sa_cart::neogeo_bootleg_ct2k3sa_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : neogeo_bootleg_cart(mconfig, NEOGEO_BOOTLEG_CT2K3SA_CART, "NEOGEO BOOT ct2k3sa Cart", tag, owner, clock, "boot_ct2k3sa_cart", __FILE__) {}

void neogeo_bootleg_ct2k3sa_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);
}

void neogeo_bootleg_ct2k3sa_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_bootleg_prot->decrypt_ct2k3sa(spr_region, spr_region_size, audiocpu_region, audio_region_size);
	m_bootleg_prot->patch_ct2k3sa(cpuregion, cpuregion_size);
}

/*************************************************
 kf10thep
**************************************************/

const device_type NEOGEO_BOOTLEG_KF10THEP_CART = &device_creator<neogeo_bootleg_kf10thep_cart>;

neogeo_bootleg_kf10thep_cart::neogeo_bootleg_kf10thep_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : neogeo_bootleg_cart(mconfig, NEOGEO_BOOTLEG_KF10THEP_CART, "NEOGEO BOOT kf10thep Cart", tag, owner, clock, "boot_kf10thep_cart", __FILE__) {}

void neogeo_bootleg_kf10thep_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);

}

void neogeo_bootleg_kf10thep_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_bootleg_prot->kf10thep_px_decrypt(cpuregion, cpuregion_size);
	m_bootleg_prot->neogeo_bootleg_sx_decrypt(fix_region,fix_region_size,1);
}


/*************************************************
 kf2k5uni
**************************************************/

const device_type NEOGEO_BOOTLEG_KF2K5UNI_CART = &device_creator<neogeo_bootleg_kf2k5uni_cart>;

neogeo_bootleg_kf2k5uni_cart::neogeo_bootleg_kf2k5uni_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : neogeo_bootleg_cart(mconfig, NEOGEO_BOOTLEG_KF2K5UNI_CART, "NEOGEO BOOT kf2k5uni Cart", tag, owner, clock, "boot_kf2k5uni_cart", __FILE__) {}

void neogeo_bootleg_kf2k5uni_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);

}

void neogeo_bootleg_kf2k5uni_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_bootleg_prot->decrypt_kf2k5uni(cpuregion,cpuregion_size, audiocpu_region, audio_region_size, fix_region, fix_region_size);
}


/*************************************************
 kf2k4se
**************************************************/

const device_type NEOGEO_BOOTLEG_KF2K4SE_CART = &device_creator<neogeo_bootleg_kf2k4se_cart>;

neogeo_bootleg_kf2k4se_cart::neogeo_bootleg_kf2k4se_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : neogeo_bootleg_cart(mconfig, NEOGEO_BOOTLEG_KF2K4SE_CART, "NEOGEO BOOT kf2k4se Cart", tag, owner, clock, "boot_kf2k4se_cart", __FILE__) {}

void neogeo_bootleg_kf2k4se_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);

}

void neogeo_bootleg_kf2k4se_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_bootleg_prot->decrypt_kof2k4se_68k(cpuregion, cpuregion_size);
}



/*************************************************
 svcplus
**************************************************/

const device_type NEOGEO_BOOTLEG_SVCPLUS_CART = &device_creator<neogeo_bootleg_svcplus_cart>;

neogeo_bootleg_svcplus_cart::neogeo_bootleg_svcplus_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : neogeo_bootleg_cart(mconfig, NEOGEO_BOOTLEG_SVCPLUS_CART, "NEOGEO BOOT svcplus Cart", tag, owner, clock, "boot_svcplus_cart", __FILE__) {}

void neogeo_bootleg_svcplus_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);

}

void neogeo_bootleg_svcplus_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_bootleg_prot->svcplus_px_decrypt(cpuregion, cpuregion_size);
	m_bootleg_prot->svcboot_cx_decrypt(spr_region, spr_region_size);
	m_bootleg_prot->neogeo_bootleg_sx_decrypt(fix_region, fix_region_size, 1);
	m_bootleg_prot->svcplus_px_hack(cpuregion, cpuregion_size);
}


/*************************************************
 svcplusaa
**************************************************/

const device_type NEOGEO_BOOTLEG_SVCPLUSA_CART = &device_creator<neogeo_bootleg_svcplusa_cart>;

neogeo_bootleg_svcplusa_cart::neogeo_bootleg_svcplusa_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : neogeo_bootleg_cart(mconfig, NEOGEO_BOOTLEG_SVCPLUSA_CART, "NEOGEO BOOT svcplusa Cart", tag, owner, clock, "boot_svcplusa_cart", __FILE__) {}

void neogeo_bootleg_svcplusa_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);

}

void neogeo_bootleg_svcplusa_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_bootleg_prot->svcplusa_px_decrypt(cpuregion, cpuregion_size);
	m_bootleg_prot->svcboot_cx_decrypt(spr_region, spr_region_size);
	m_bootleg_prot->svcplus_px_hack(cpuregion, cpuregion_size);
}

/*************************************************
 samsho5b
**************************************************/

const device_type NEOGEO_BOOTLEG_SAMSHO5B_CART = &device_creator<neogeo_bootleg_samsho5b_cart>;

neogeo_bootleg_samsho5b_cart::neogeo_bootleg_samsho5b_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : neogeo_bootleg_cart(mconfig, NEOGEO_BOOTLEG_SAMSHO5B_CART, "NEOGEO BOOT samsho5b Cart", tag, owner, clock, "boot_samsho5b_cart", __FILE__) {}

void neogeo_bootleg_samsho5b_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);

}

void neogeo_bootleg_samsho5b_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_bootleg_prot->samsho5b_px_decrypt(cpuregion, cpuregion_size);
	m_bootleg_prot->samsho5b_vx_decrypt(ym_region, ym_region_size);
	m_bootleg_prot->neogeo_bootleg_sx_decrypt(fix_region, fix_region_size,1);
	m_bootleg_prot->neogeo_bootleg_cx_decrypt(spr_region, spr_region_size);
}


/*************************************************
 kof97oro
**************************************************/

const device_type NEOGEO_BOOTLEG_KOF97ORO_CART = &device_creator<neogeo_bootleg_kof97oro_cart>;

neogeo_bootleg_kof97oro_cart::neogeo_bootleg_kof97oro_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : neogeo_bootleg_cart(mconfig, NEOGEO_BOOTLEG_KOF97ORO_CART, "NEOGEO BOOT kof97oro Cart", tag, owner, clock, "boot_kof97oro_cart", __FILE__) {}

void neogeo_bootleg_kof97oro_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);

}

void neogeo_bootleg_kof97oro_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_bootleg_prot->kof97oro_px_decode(cpuregion, cpuregion_size);
	m_bootleg_prot->neogeo_bootleg_sx_decrypt(fix_region, fix_region_size,1);
	m_bootleg_prot->neogeo_bootleg_cx_decrypt(spr_region, spr_region_size);
}


/*************************************************
 lans2004
**************************************************/

const device_type NEOGEO_BOOTLEG_LANS2004_CART = &device_creator<neogeo_bootleg_lans2004_cart>;

neogeo_bootleg_lans2004_cart::neogeo_bootleg_lans2004_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : neogeo_bootleg_cart(mconfig, NEOGEO_BOOTLEG_LANS2004_CART, "NEOGEO BOOT lans2004 Cart", tag, owner, clock, "boot_lans2004_cart", __FILE__) {}

void neogeo_bootleg_lans2004_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);

}

void neogeo_bootleg_lans2004_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_bootleg_prot->lans2004_decrypt_68k(cpuregion, cpuregion_size);
	m_bootleg_prot->lans2004_vx_decrypt(ym_region, ym_region_size);
	m_bootleg_prot->neogeo_bootleg_sx_decrypt(fix_region, fix_region_size,1);
	m_bootleg_prot->neogeo_bootleg_cx_decrypt(spr_region, spr_region_size);
}


/*************************************************
 kof10th
**************************************************/

const device_type NEOGEO_BOOTLEG_KOF10TH_CART = &device_creator<neogeo_bootleg_kof10th_cart>;

neogeo_bootleg_kof10th_cart::neogeo_bootleg_kof10th_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : neogeo_bootleg_cart(mconfig, NEOGEO_BOOTLEG_KOF10TH_CART, "NEOGEO BOOT kof10th Cart", tag, owner, clock, "boot_kof10th_cart", __FILE__) {}

void neogeo_bootleg_kof10th_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);
	m_bootleg_prot->install_kof10th_protection(maincpu,m_banked_cart, cpuregion, cpuregion_size, fixedregion, fixedregion_size);

}

void neogeo_bootleg_kof10th_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_bootleg_prot->decrypt_kof10th(cpuregion, cpuregion_size);
}



/*************************************************
 kog
**************************************************/

const device_type NEOGEO_BOOTLEG_KOG_CART = &device_creator<neogeo_bootleg_kog_cart>;

neogeo_bootleg_kog_cart::neogeo_bootleg_kog_cart(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) : neogeo_bootleg_cart(mconfig, NEOGEO_BOOTLEG_KOG_CART, "NEOGEO BOOT kog Cart", tag, owner, clock, "boot_kog_cart", __FILE__),
	m_kog_prot(*this, "kog_prot")
{}

static MACHINE_CONFIG_FRAGMENT( kog_bootleg_cart )
	MCFG_NEOGEO_BANKED_CART_ADD("banked_cart")
	MCFG_NGBOOTLEG_PROT_ADD("bootleg_prot")
	MCFG_KOG_PROT_ADD("kog_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_bootleg_kog_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( kog_bootleg_cart );
}


void neogeo_bootleg_kog_cart::activate_cart(ACTIVATE_CART_PARAMS)
{
	m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);
	m_kog_prot->kog_install_protection(maincpu);
}

void neogeo_bootleg_kog_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_kog_prot->kog_px_decrypt(cpuregion, cpuregion_size);
	m_bootleg_prot->neogeo_bootleg_sx_decrypt(fix_region, fix_region_size,1);
	m_bootleg_prot->neogeo_bootleg_cx_decrypt(spr_region, spr_region_size);
}
