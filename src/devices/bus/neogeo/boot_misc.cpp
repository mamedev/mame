// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************
 
 Neo Geo cart emulation
 Misc. bootleg cart types (possibly to be split further at a later stage)

 ***********************************************************************************************************/


#include "emu.h"
#include "boot_misc.h"


//-------------------------------------------------
//  neogeo_bootleg_cart - constructor
//-------------------------------------------------

const device_type NEOGEO_BOOTLEG_CART = &device_creator<neogeo_bootleg_cart>;


neogeo_bootleg_cart::neogeo_bootleg_cart(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT16 clock, const char *shortname, const char *source) :
	neogeo_rom_device(mconfig, type, name, tag, owner, clock, shortname, source),
	m_prot(*this, "bootleg_prot")
{}

neogeo_bootleg_cart::neogeo_bootleg_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock) :
	neogeo_rom_device(mconfig, NEOGEO_BOOTLEG_CART, "Neo Geo Bootleg Protected Cart", tag, owner, clock, "neocart_boot", __FILE__),
	m_prot(*this, "bootleg_prot")
{}


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

static MACHINE_CONFIG_FRAGMENT( bootleg_cart )
	MCFG_NEOBOOT_PROT_ADD("bootleg_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_bootleg_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( bootleg_cart );
}



/*************************************************
 garoubl
**************************************************/

const device_type NEOGEO_GAROUBL_CART = &device_creator<neogeo_garoubl_cart>;

neogeo_garoubl_cart::neogeo_garoubl_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_bootleg_cart(mconfig, NEOGEO_GAROUBL_CART, "Neo Geo Garou Bootleg Cart", tag, owner, clock, "neocart_garoubl", __FILE__)
{}


void neogeo_garoubl_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->sx_decrypt(fix_region, fix_region_size, 2);
	m_prot->cx_decrypt(spr_region, spr_region_size);
}


/*************************************************
 kof97oro
 **************************************************/

const device_type NEOGEO_KOF97ORO_CART = &device_creator<neogeo_kof97oro_cart>;

neogeo_kof97oro_cart::neogeo_kof97oro_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_bootleg_cart(mconfig, NEOGEO_KOF97ORO_CART, "Neo Geo KOF 97 Orochi Bootleg Cart", tag, owner, clock, "neocart_kof97oro", __FILE__)
{}


void neogeo_kof97oro_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->kof97oro_px_decode(cpuregion, cpuregion_size);
	m_prot->sx_decrypt(fix_region, fix_region_size,1);
	m_prot->cx_decrypt(spr_region, spr_region_size);
}


/*************************************************
 kf10thep
**************************************************/

const device_type NEOGEO_KF10THEP_CART = &device_creator<neogeo_kf10thep_cart>;

neogeo_kf10thep_cart::neogeo_kf10thep_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_bootleg_cart(mconfig, NEOGEO_KF10THEP_CART, "Neo Geo KOF 10th Ann. EP Bootleg Cart", tag, owner, clock, "neocart_kf10thep", __FILE__)
{}


void neogeo_kf10thep_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->kf10thep_px_decrypt(cpuregion, cpuregion_size);
	m_prot->sx_decrypt(fix_region, fix_region_size, 1);
}


/*************************************************
 kf2k5uni
**************************************************/

const device_type NEOGEO_KF2K5UNI_CART = &device_creator<neogeo_kf2k5uni_cart>;

neogeo_kf2k5uni_cart::neogeo_kf2k5uni_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_bootleg_cart(mconfig, NEOGEO_KF2K5UNI_CART, "Neo Geo KOF 2005 Unique Bootleg Cart", tag, owner, clock, "neocart_kf2k5uni", __FILE__)
{}


void neogeo_kf2k5uni_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->kf2k5uni_px_decrypt(cpuregion, cpuregion_size);
	m_prot->kf2k5uni_sx_decrypt(fix_region, fix_region_size);
	m_prot->kf2k5uni_mx_decrypt(audiocpu_region, audio_region_size);
}


/*************************************************
 kf2k4se
**************************************************/

const device_type NEOGEO_KF2K4SE_CART = &device_creator<neogeo_kf2k4se_cart>;

neogeo_kf2k4se_cart::neogeo_kf2k4se_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_bootleg_cart(mconfig, NEOGEO_KF2K4SE_CART, "Neo Geo KOF 2004 SE Bootleg Cart", tag, owner, clock, "neocart_kf2k4se", __FILE__)
{}


void neogeo_kf2k4se_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->decrypt_kof2k4se_68k(cpuregion, cpuregion_size);
}


/*************************************************
 lans2004
 **************************************************/

const device_type NEOGEO_LANS2004_CART = &device_creator<neogeo_lans2004_cart>;

neogeo_lans2004_cart::neogeo_lans2004_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_bootleg_cart(mconfig, NEOGEO_LANS2004_CART, "Neo Geo Lansquenet 2004 Bootleg Cart", tag, owner, clock, "neocart_lans2004", __FILE__)
{}

void neogeo_lans2004_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->lans2004_decrypt_68k(cpuregion, cpuregion_size);
	m_prot->lans2004_vx_decrypt(ym_region, ym_region_size);
	m_prot->sx_decrypt(fix_region, fix_region_size,1);
	m_prot->cx_decrypt(spr_region, spr_region_size);
}


/*************************************************
 samsho5b
**************************************************/

const device_type NEOGEO_SAMSHO5B_CART = &device_creator<neogeo_samsho5b_cart>;

neogeo_samsho5b_cart::neogeo_samsho5b_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_bootleg_cart(mconfig, NEOGEO_SAMSHO5B_CART, "Neo Geo Samurai Shodown 5 Bootleg Cart", tag, owner, clock, "neocart_samsho5b", __FILE__)
{}

void neogeo_samsho5b_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->samsho5b_px_decrypt(cpuregion, cpuregion_size);
	m_prot->samsho5b_vx_decrypt(ym_region, ym_region_size);
	m_prot->sx_decrypt(fix_region, fix_region_size, 1);
	m_prot->cx_decrypt(spr_region, spr_region_size);
}


/*************************************************
 mslug3b6
 **************************************************/

const device_type NEOGEO_MSLUG3B6_CART = &device_creator<neogeo_mslug3b6_cart>;

neogeo_mslug3b6_cart::neogeo_mslug3b6_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_bootleg_cart(mconfig, NEOGEO_MSLUG3B6_CART, "Neo Geo Metal Slug 6 Bootleg Cart", tag, owner, clock, "neocart_mslug3b6", __FILE__),
	m_cmc_prot(*this, "cmc_prot")
{}

void neogeo_mslug3b6_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->sx_decrypt(fix_region, fix_region_size, 2);
	m_cmc_prot->cmc42_gfx_decrypt(spr_region, spr_region_size, MSLUG3_GFX_KEY);
}

static MACHINE_CONFIG_FRAGMENT( mslug3b6_cart )
	MCFG_CMC_PROT_ADD("cmc_prot")
	MCFG_NEOBOOT_PROT_ADD("bootleg_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_mslug3b6_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( mslug3b6_cart );
}


/*************************************************
 ms5plus
 **************************************************/

const device_type NEOGEO_MS5PLUS_CART = &device_creator<neogeo_ms5plus_cart>;

neogeo_ms5plus_cart::neogeo_ms5plus_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_bootleg_cart(mconfig, NEOGEO_MS5PLUS_CART, "Neo Geo Metal Slug 5 Plus Bootleg Cart", tag, owner, clock, "neocart_ms5plus", __FILE__),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot")
{}

void neogeo_ms5plus_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region,audio_region_size);
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, MSLUG5_GFX_KEY);
	m_pcm2_prot->swap(ym_region, ym_region_size, 2);
	m_prot->sx_decrypt(fix_region, fix_region_size, 1);
}

static MACHINE_CONFIG_FRAGMENT( ms5plus_cart )
	MCFG_NEOBOOT_PROT_ADD("bootleg_prot")
	MCFG_CMC_PROT_ADD("cmc_prot")
	MCFG_PCM2_PROT_ADD("pcm2_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_ms5plus_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ms5plus_cart );
}


/*************************************************
 kog
**************************************************/

const device_type NEOGEO_KOG_CART = &device_creator<neogeo_kog_cart>;

neogeo_kog_cart::neogeo_kog_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_bootleg_cart(mconfig, NEOGEO_KOG_CART, "Neo Geo King of Gladiators Bootleg Cart", tag, owner, clock, "neocart_kog", __FILE__),
	m_jumper(*this, "JUMPER")
{}


static INPUT_PORTS_START( kog )
	// a jumper on the pcb overlays a ROM address, very strange but that's how it works.
	PORT_START("JUMPER")
	PORT_DIPNAME( 0x0001, 0x0001, "Title Language" ) PORT_DIPLOCATION("CART-JUMPER:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0000, "Non-English" )
	PORT_BIT( 0x00fe, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor neogeo_kog_cart::device_input_ports() const
{
	return INPUT_PORTS_NAME( kog );
}


READ16_MEMBER(neogeo_kog_cart::protection_r)
{
	return m_jumper->read();
}

void neogeo_kog_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->kog_px_decrypt(cpuregion, cpuregion_size);
	m_prot->sx_decrypt(fix_region, fix_region_size, 1);
	m_prot->cx_decrypt(spr_region, spr_region_size);
}

