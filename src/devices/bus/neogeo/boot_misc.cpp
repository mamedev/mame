// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************

 Neo Geo cart emulation
 Misc. bootleg cart types (possibly to be split further at a later stage)

 ***********************************************************************************************************/


#include "emu.h"
#include "boot_misc.h"


//-------------------------------------------------
//  neogeo_bootleg_cart_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NEOGEO_BOOTLEG_CART, neogeo_bootleg_cart_device, "neocart_boot", "Neo Geo Bootleg Protected Cart")


neogeo_bootleg_cart_device::neogeo_bootleg_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint16_t clock) :
	neogeo_rom_device(mconfig, type, tag, owner, clock),
	m_prot(*this, "bootleg_prot")
{
}

neogeo_bootleg_cart_device::neogeo_bootleg_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock) :
	neogeo_bootleg_cart_device(mconfig, NEOGEO_BOOTLEG_CART, tag, owner, clock)
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_bootleg_cart_device::device_start()
{
}

void neogeo_bootleg_cart_device::device_reset()
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

void neogeo_bootleg_cart_device::device_add_mconfig(machine_config &config)
{
	NEOBOOT_PROT(config, m_prot);
}


/*************************************************
 garoubl
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_GAROUBL_CART, neogeo_garoubl_cart_device, "neocart_garoubl", "Neo Geo Garou Bootleg Cart")

neogeo_garoubl_cart_device::neogeo_garoubl_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_bootleg_cart_device(mconfig, NEOGEO_GAROUBL_CART, tag, owner, clock)
{
}


void neogeo_garoubl_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->sx_decrypt(fix_region, fix_region_size, 2);
	m_prot->cx_decrypt(spr_region, spr_region_size);
}


/*************************************************
 kof97oro
 **************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_KOF97ORO_CART, neogeo_kof97oro_cart_device, "neocart_kof97oro", "Neo Geo KoF 97 Orochi Bootleg Cart")

neogeo_kof97oro_cart_device::neogeo_kof97oro_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_bootleg_cart_device(mconfig, NEOGEO_KOF97ORO_CART, tag, owner, clock)
{
}


void neogeo_kof97oro_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->kof97oro_px_decode(cpuregion, cpuregion_size);
	m_prot->sx_decrypt(fix_region, fix_region_size,1);
	m_prot->cx_decrypt(spr_region, spr_region_size);
}


/*************************************************
 kf10thep
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_KF10THEP_CART, neogeo_kf10thep_cart_device, "neocart_kf10thep", "Neo Geo KoF 10th Ann. EP Bootleg Cart")

neogeo_kf10thep_cart_device::neogeo_kf10thep_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_bootleg_cart_device(mconfig, NEOGEO_KF10THEP_CART, tag, owner, clock)
{
}


void neogeo_kf10thep_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->kf10thep_px_decrypt(cpuregion, cpuregion_size);
	m_prot->sx_decrypt(fix_region, fix_region_size, 1);
}


/*************************************************
 kf2k5uni
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_KF2K5UNI_CART, neogeo_kf2k5uni_cart_device, "neocart_kf2k5uni", "Neo Geo KoF 2005 Unique Bootleg Cart")

neogeo_kf2k5uni_cart_device::neogeo_kf2k5uni_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_bootleg_cart_device(mconfig, NEOGEO_KF2K5UNI_CART, tag, owner, clock)
{
}


void neogeo_kf2k5uni_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->kf2k5uni_px_decrypt(cpuregion, cpuregion_size);
	m_prot->kf2k5uni_sx_decrypt(fix_region, fix_region_size);
	m_prot->kf2k5uni_mx_decrypt(audiocpu_region, audio_region_size);
}


/*************************************************
 kf2k4se
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_KF2K4SE_CART, neogeo_kf2k4se_cart_device, "neocart_kf2k4se", "Neo Geo KoF 2004 SE Bootleg Cart")

neogeo_kf2k4se_cart_device::neogeo_kf2k4se_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_bootleg_cart_device(mconfig, NEOGEO_KF2K4SE_CART, tag, owner, clock)
{
}


void neogeo_kf2k4se_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->decrypt_kof2k4se_68k(cpuregion, cpuregion_size);
}


/*************************************************
 lans2004
 **************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_LANS2004_CART, neogeo_lans2004_cart_device, "neocart_lans2004", "Neo Geo Lansquenet 2004 Bootleg Cart")

neogeo_lans2004_cart_device::neogeo_lans2004_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_bootleg_cart_device(mconfig, NEOGEO_LANS2004_CART, tag, owner, clock)
{
}

void neogeo_lans2004_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->lans2004_decrypt_68k(cpuregion, cpuregion_size);
	m_prot->lans2004_vx_decrypt(ym_region, ym_region_size);
	m_prot->sx_decrypt(fix_region, fix_region_size,1);
	m_prot->cx_decrypt(spr_region, spr_region_size);
}


/*************************************************
 samsho5b
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_SAMSHO5B_CART, neogeo_samsho5b_cart_device, "neocart_samsho5b", "Neo Geo Samurai Shodown 5 Bootleg Cart")

neogeo_samsho5b_cart_device::neogeo_samsho5b_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_bootleg_cart_device(mconfig, NEOGEO_SAMSHO5B_CART, tag, owner, clock)
{
}

void neogeo_samsho5b_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->samsho5b_px_decrypt(cpuregion, cpuregion_size);
	m_prot->samsho5b_vx_decrypt(ym_region, ym_region_size);
	m_prot->sx_decrypt(fix_region, fix_region_size, 1);
	m_prot->cx_decrypt(spr_region, spr_region_size);
}


/*************************************************
 mslug3b6
 **************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_MSLUG3B6_CART, neogeo_mslug3b6_cart_device, "neocart_mslug3b6", "Neo Geo Metal Slug 6 Bootleg Cart")

neogeo_mslug3b6_cart_device::neogeo_mslug3b6_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_bootleg_cart_device(mconfig, NEOGEO_MSLUG3B6_CART, tag, owner, clock),
	m_cmc_prot(*this, "cmc_prot")
{
}

void neogeo_mslug3b6_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->sx_decrypt(fix_region, fix_region_size, 2);
	m_cmc_prot->cmc42_gfx_decrypt(spr_region, spr_region_size, MSLUG3_GFX_KEY);
}

void neogeo_mslug3b6_cart_device::device_add_mconfig(machine_config &config)
{
	NG_CMC_PROT(config, m_cmc_prot);
	NEOBOOT_PROT(config, m_prot);
}


/*************************************************
 ms5plus
 **************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_MS5PLUS_CART, neogeo_ms5plus_cart_device, "neocart_ms5plus", "Neo Geo Metal Slug 5 Plus Bootleg Cart")

neogeo_ms5plus_cart_device::neogeo_ms5plus_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_bootleg_cart_device(mconfig, NEOGEO_MS5PLUS_CART, tag, owner, clock),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot")
{
}

void neogeo_ms5plus_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region,audio_region_size);
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, MSLUG5_GFX_KEY);
	m_pcm2_prot->swap(ym_region, ym_region_size, 2);
	m_prot->sx_decrypt(fix_region, fix_region_size, 1);
}

void neogeo_ms5plus_cart_device::device_add_mconfig(machine_config &config)
{
	NEOBOOT_PROT(config, m_prot);
	NG_CMC_PROT(config, m_cmc_prot);
	NG_PCM2_PROT(config, m_pcm2_prot);
}


/*************************************************
 kog
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_KOG_CART, neogeo_kog_cart_device, "neocart_kog", "Neo Geo King of Gladiators Bootleg Cart")

neogeo_kog_cart_device::neogeo_kog_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_bootleg_cart_device(mconfig, NEOGEO_KOG_CART, tag, owner, clock),
	m_jumper(*this, "JUMPER")
{
}


static INPUT_PORTS_START( kog )
	// a jumper on the pcb overlays a ROM address, very strange but that's how it works.
	PORT_START("JUMPER")
	PORT_DIPNAME( 0x0001, 0x0001, "Title Language" ) PORT_DIPLOCATION("CART-JUMPER:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0000, "Non-English" )
	PORT_BIT( 0x00fe, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor neogeo_kog_cart_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( kog );
}


READ16_MEMBER(neogeo_kog_cart_device::protection_r)
{
	return m_jumper->read();
}

void neogeo_kog_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->kog_px_decrypt(cpuregion, cpuregion_size);
	m_prot->sx_decrypt(fix_region, fix_region_size, 1);
	m_prot->cx_decrypt(spr_region, spr_region_size);
}
