// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
/***********************************************************************************************************

 Neo Geo cart emulation
 SNK Vs Capcom Bootleg cart type

 ***********************************************************************************************************/


#include "emu.h"
#include "boot_svc.h"


/*************************************************
 svcboot
 **************************************************/

const device_type NEOGEO_SVCBOOT_CART = &device_creator<neogeo_svcboot_cart>;

neogeo_svcboot_cart::neogeo_svcboot_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_bootleg_cart(mconfig, NEOGEO_SVCBOOT_CART, "Neo Geo SVC Bootleg Cart", tag, owner, clock, "neocart_svcboot", __FILE__),
	m_pvc_prot(*this, "pvc_prot")
{}

void neogeo_svcboot_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->svcboot_px_decrypt(cpuregion, cpuregion_size);
	m_prot->svcboot_cx_decrypt(spr_region, spr_region_size);
}

static MACHINE_CONFIG_FRAGMENT( svcboot_cart )
	MCFG_NEOBOOT_PROT_ADD("bootleg_prot")
	MCFG_PVC_PROT_ADD("pvc_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_svcboot_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( svcboot_cart );
}


/*************************************************
 svcplus
**************************************************/

const device_type NEOGEO_SVCPLUS_CART = &device_creator<neogeo_svcplus_cart>;

neogeo_svcplus_cart::neogeo_svcplus_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_bootleg_cart(mconfig, NEOGEO_SVCPLUS_CART, "Neo Geo SVC Plus Bootleg Cart", tag, owner, clock, "neocart_svcplus", __FILE__)
{}

void neogeo_svcplus_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->svcplus_px_decrypt(cpuregion, cpuregion_size);
	m_prot->svcboot_cx_decrypt(spr_region, spr_region_size);
	m_prot->sx_decrypt(fix_region, fix_region_size, 1);
	m_prot->svcplus_px_hack(cpuregion, cpuregion_size);
}


/*************************************************
 svcplusa
**************************************************/

const device_type NEOGEO_SVCPLUSA_CART = &device_creator<neogeo_svcplusa_cart>;

neogeo_svcplusa_cart::neogeo_svcplusa_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_bootleg_cart(mconfig, NEOGEO_SVCPLUSA_CART, "Neo Geo SVC Plus Alt Bootleg Cart", tag, owner, clock, "neocart_svcplusa", __FILE__)
{}

void neogeo_svcplusa_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->svcplusa_px_decrypt(cpuregion, cpuregion_size);
	m_prot->svcboot_cx_decrypt(spr_region, spr_region_size);
	m_prot->svcplus_px_hack(cpuregion, cpuregion_size);
}

/*************************************************
 svcsplus
 **************************************************/

const device_type NEOGEO_SVCSPLUS_CART = &device_creator<neogeo_svcsplus_cart>;

neogeo_svcsplus_cart::neogeo_svcsplus_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	neogeo_bootleg_cart(mconfig, NEOGEO_SVCSPLUS_CART, "Neo Geo SVC S.Plus Bootleg Cart", tag, owner, clock, "neocart_svcsplus", __FILE__),
	m_pvc_prot(*this, "pvc_prot")
{}

void neogeo_svcsplus_cart::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->svcsplus_px_decrypt(cpuregion, cpuregion_size);
	m_prot->sx_decrypt(fix_region, fix_region_size, 2);
	m_prot->svcboot_cx_decrypt(spr_region, spr_region_size);
	m_prot->svcsplus_px_hack(cpuregion, cpuregion_size);
}


static MACHINE_CONFIG_FRAGMENT( svcsplus_cart )
	MCFG_NEOBOOT_PROT_ADD("bootleg_prot")
	MCFG_PVC_PROT_ADD("pvc_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_svcsplus_cart::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( svcsplus_cart );
}
