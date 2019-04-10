// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************

 Neo Geo cart emulation
 Crouching Tiger Hidden Dragon Bootleg cart type

 ***********************************************************************************************************/


#include "emu.h"
#include "boot_cthd.h"


//-------------------------------------------------
//  neogeo_cthd2k3_cart_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NEOGEO_CTHD2K3_CART, neogeo_cthd2k3_cart_device, "neocart_ct2k3", "Neo Geo CTHD 2003 Cart")


neogeo_cthd2k3_cart_device::neogeo_cthd2k3_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint16_t clock) :
	neogeo_rom_device(mconfig, type, tag, owner, clock),
	m_prot(*this, "cthd_prot")
{
}

neogeo_cthd2k3_cart_device::neogeo_cthd2k3_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock) :
	neogeo_cthd2k3_cart_device(mconfig, NEOGEO_CTHD2K3_CART, tag, owner, clock)
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_cthd2k3_cart_device::device_start()
{
}

void neogeo_cthd2k3_cart_device::device_reset()
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

void neogeo_cthd2k3_cart_device::device_add_mconfig(machine_config &config)
{
	NG_CTHD_PROT(config, m_prot);
}


/*************************************************
 cthd2k3
 **************************************************/


void neogeo_cthd2k3_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->decrypt_cthd2003(spr_region, spr_region_size, audiocpu_region, audio_region_size, fix_region, fix_region_size);
	m_prot->patch_cthd2003(cpuregion, cpuregion_size);
}


/*************************************************
 ct2k3sp
 **************************************************/


DEFINE_DEVICE_TYPE(NEOGEO_CT2K3SP_CART, neogeo_ct2k3sp_cart_device, "neocart_ct2k3sp", "Neo Geo CTHD 2003 Sp Cart")

neogeo_ct2k3sp_cart_device::neogeo_ct2k3sp_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_cthd2k3_cart_device(mconfig, NEOGEO_CT2K3SP_CART, tag, owner, clock)
{
}

void neogeo_ct2k3sp_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->decrypt_ct2k3sp(spr_region, spr_region_size, audiocpu_region, audio_region_size, fix_region, fix_region_size);
	m_prot->patch_cthd2003(cpuregion, cpuregion_size);
}

/*************************************************
 ct2k3sa
 **************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_CT2K3SA_CART, neogeo_ct2k3sa_cart_device, "neocart_ct2k3sa", "Neo Geo CTHD 2003 Sp Alt Cart")

neogeo_ct2k3sa_cart_device::neogeo_ct2k3sa_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_cthd2k3_cart_device(mconfig, NEOGEO_CT2K3SA_CART, tag, owner, clock)
{
}

void neogeo_ct2k3sa_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->decrypt_ct2k3sa(spr_region, spr_region_size, audiocpu_region, audio_region_size);
	m_prot->patch_ct2k3sa(cpuregion, cpuregion_size);
}


/*************************************************
 matrimbl
 **************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_MATRIMBL_CART, neogeo_matrimbl_cart_device, "neocart_matrimbl", "Neo Geo Matrimelee Bootleg Cart")

neogeo_matrimbl_cart_device::neogeo_matrimbl_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_cthd2k3_cart_device(mconfig, NEOGEO_MATRIMBL_CART, tag, owner, clock),
	m_cmc_prot(*this, "cmc_prot"),
	m_kof2k2_prot(*this, "kof2k2_prot")
{
}

void neogeo_matrimbl_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_kof2k2_prot->matrim_decrypt_68k(cpuregion, cpuregion_size);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size); // required for text layer
	m_prot->matrimbl_decrypt(spr_region, spr_region_size, audiocpu_region, audio_region_size);
}


void neogeo_matrimbl_cart_device::device_add_mconfig(machine_config &config)
{
	NG_KOF2002_PROT(config, m_kof2k2_prot);
	NG_CMC_PROT(config, m_cmc_prot);
	NG_CTHD_PROT(config, m_prot);
}
