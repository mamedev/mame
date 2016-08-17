// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************

 Neo Geo cart emulation
 Crouching Tiger Hidden Dragon Bootleg cart type

 ***********************************************************************************************************/


#include "emu.h"
#include "boot_cthd.h"


//-------------------------------------------------
//  neogeo_cthd2k3_cart - constructor
//-------------------------------------------------

const device_type NEOGEO_CTHD2K3_CART = &device_creator<neogeo_cthd2k3_cart>;


neogeo_cthd2k3_cart::neogeo_cthd2k3_cart(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT16 clock, const char *shortname, const char *source) :
	neogeo_rom_device(mconfig, type, name, tag, owner, clock, shortname, source),
	m_prot(*this, "cthd_prot")
{}

neogeo_cthd2k3_cart::neogeo_cthd2k3_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock) :
	neogeo_rom_device(mconfig, NEOGEO_CTHD2K3_CART, "Neo Geo CTHD 2003 Cart", tag, owner, clock, "neocart_ct2k3", __FILE__),
	m_prot(*this, "cthd_prot")
{}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_cthd2k3_cart::device_start()
{
}

void neogeo_cthd2k3_cart::device_reset()
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

static MACHINE_CONFIG_FRAGMENT( cthd_cart )
	MCFG_CTHD_PROT_ADD("cthd_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_cthd2k3_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cthd_cart );
}


/*************************************************
 cthd2k3
 **************************************************/


void neogeo_cthd2k3_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->decrypt_cthd2003(spr_region, spr_region_size, audiocpu_region, audio_region_size, fix_region, fix_region_size);
	m_prot->patch_cthd2003(cpuregion, cpuregion_size);
}


/*************************************************
 ct2k3sp
 **************************************************/


const device_type NEOGEO_CT2K3SP_CART = &device_creator<neogeo_ct2k3sp_cart>;

neogeo_ct2k3sp_cart::neogeo_ct2k3sp_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_cthd2k3_cart(mconfig, NEOGEO_CT2K3SP_CART, "Neo Geo CTHD 2003 Sp Cart", tag, owner, clock, "neocart_ct2k3sp", __FILE__)
{}

void neogeo_ct2k3sp_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->decrypt_ct2k3sp(spr_region, spr_region_size, audiocpu_region, audio_region_size, fix_region, fix_region_size);
	m_prot->patch_cthd2003(cpuregion, cpuregion_size);
}

/*************************************************
 ct2k3sa
 **************************************************/

const device_type NEOGEO_CT2K3SA_CART = &device_creator<neogeo_ct2k3sa_cart>;

neogeo_ct2k3sa_cart::neogeo_ct2k3sa_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_cthd2k3_cart(mconfig, NEOGEO_CT2K3SA_CART, "Neo Geo CTHD 2003 Sp Alt Cart", tag, owner, clock, "neocart_ct2k3sa", __FILE__)
{}

void neogeo_ct2k3sa_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->decrypt_ct2k3sa(spr_region, spr_region_size, audiocpu_region, audio_region_size);
	m_prot->patch_ct2k3sa(cpuregion, cpuregion_size);
}


/*************************************************
 matrimbl
 **************************************************/

const device_type NEOGEO_MATRIMBL_CART = &device_creator<neogeo_matrimbl_cart>;

neogeo_matrimbl_cart::neogeo_matrimbl_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_cthd2k3_cart(mconfig, NEOGEO_MATRIMBL_CART, "Neo Geo Matrimelee Bootleg Cart", tag, owner, clock, "neocart_matrimbl", __FILE__),
	m_cmc_prot(*this, "cmc_prot"),
	m_kof2k2_prot(*this, "kof2k2_prot")
{}

void neogeo_matrimbl_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_kof2k2_prot->matrim_decrypt_68k(cpuregion, cpuregion_size);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size); // required for text layer
	m_prot->matrimbl_decrypt(spr_region, spr_region_size, audiocpu_region, audio_region_size);
}


static MACHINE_CONFIG_FRAGMENT( matrimbl_cart )
	MCFG_KOF2002_PROT_ADD("kof2k2_prot")
	MCFG_CMC_PROT_ADD("cmc_prot")
	MCFG_CTHD_PROT_ADD("cthd_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_matrimbl_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( matrimbl_cart );
}
