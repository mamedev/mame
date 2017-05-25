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

DEFINE_DEVICE_TYPE(NEOGEO_SVCBOOT_CART, neogeo_svcboot_cart_device, "neocart_svcboot", "Neo Geo SVC Bootleg Cart")

neogeo_svcboot_cart_device::neogeo_svcboot_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_bootleg_cart_device(mconfig, NEOGEO_SVCBOOT_CART, tag, owner, clock),
	m_pvc_prot(*this, "pvc_prot")
{
}

void neogeo_svcboot_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->svcboot_px_decrypt(cpuregion, cpuregion_size);
	m_prot->svcboot_cx_decrypt(spr_region, spr_region_size);
}

static MACHINE_CONFIG_START( svcboot_cart )
	MCFG_NEOBOOT_PROT_ADD("bootleg_prot")
	MCFG_PVC_PROT_ADD("pvc_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_svcboot_cart_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( svcboot_cart );
}


/*************************************************
 svcplus
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_SVCPLUS_CART, neogeo_svcplus_cart_device, "neocart_svcplus", "Neo Geo SVC Plus Bootleg Cart")

neogeo_svcplus_cart_device::neogeo_svcplus_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_bootleg_cart_device(mconfig, NEOGEO_SVCPLUS_CART, tag, owner, clock)
{
}

void neogeo_svcplus_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->svcplus_px_decrypt(cpuregion, cpuregion_size);
	m_prot->svcboot_cx_decrypt(spr_region, spr_region_size);
	m_prot->sx_decrypt(fix_region, fix_region_size, 1);
	m_prot->svcplus_px_hack(cpuregion, cpuregion_size);
}


/*************************************************
 svcplusa
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_SVCPLUSA_CART, neogeo_svcplusa_cart_device, "neocart_svcplusa", "Neo Geo SVC Plus Alt Bootleg Cart")

neogeo_svcplusa_cart_device::neogeo_svcplusa_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_bootleg_cart_device(mconfig, NEOGEO_SVCPLUSA_CART, tag, owner, clock)
{
}


void neogeo_svcplusa_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->svcplusa_px_decrypt(cpuregion, cpuregion_size);
	m_prot->svcboot_cx_decrypt(spr_region, spr_region_size);
	m_prot->svcplus_px_hack(cpuregion, cpuregion_size);
}

/*************************************************
 svcsplus
 **************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_SVCSPLUS_CART, neogeo_svcsplus_cart_device, "neocart_svcsplus", "Neo Geo SVC S.Plus Bootleg Cart")

neogeo_svcsplus_cart_device::neogeo_svcsplus_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_bootleg_cart_device(mconfig, NEOGEO_SVCSPLUS_CART, tag, owner, clock),
	m_pvc_prot(*this, "pvc_prot")
{
}

void neogeo_svcsplus_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->svcsplus_px_decrypt(cpuregion, cpuregion_size);
	m_prot->sx_decrypt(fix_region, fix_region_size, 2);
	m_prot->svcboot_cx_decrypt(spr_region, spr_region_size);
	m_prot->svcsplus_px_hack(cpuregion, cpuregion_size);
}


static MACHINE_CONFIG_START( svcsplus_cart )
	MCFG_NEOBOOT_PROT_ADD("bootleg_prot")
	MCFG_PVC_PROT_ADD("pvc_prot")
MACHINE_CONFIG_END

machine_config_constructor neogeo_svcsplus_cart_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( svcsplus_cart );
}
