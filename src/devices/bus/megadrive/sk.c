// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 Sonic & Knuckles pass-thorugh cart emulation


 TODO: currently we only support loading of base carts with no bankswitch or protection...
       shall we support other as well?


 ***********************************************************************************************************/




#include "emu.h"
#include "sk.h"
#include "rom.h"


//-------------------------------------------------
//  md_rom_device - constructor
//-------------------------------------------------

const device_type MD_ROM_SK = &device_creator<md_rom_sk_device>;


md_rom_sk_device::md_rom_sk_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_md_cart_interface( mconfig, *this ),
						m_exp(*this, "subslot")
{
}

md_rom_sk_device::md_rom_sk_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, MD_ROM_SK, "MD Sonic & Knuckles", tag, owner, clock, "md_rom_sk", __FILE__),
						device_md_cart_interface( mconfig, *this ),
						m_exp(*this, "subslot")
{
}


void md_rom_sk_device::device_start()
{
}

/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ16_MEMBER(md_rom_sk_device::read)
{
	if (m_exp->m_cart != NULL && m_exp->m_cart->get_rom_base() != NULL && offset >= 0x200000/2 && offset < (0x200000 + m_exp->m_cart->get_rom_size())/2)
		return m_exp->m_cart->m_rom[offset - 0x200000/2];
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}

WRITE16_MEMBER(md_rom_sk_device::write)
{
// should there be anything here?
}

//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( sk_slot )
//-------------------------------------------------

static SLOT_INTERFACE_START(sk_sub_cart)
	SLOT_INTERFACE_INTERNAL("rom",  MD_STD_ROM)
	SLOT_INTERFACE_INTERNAL("rom_svp",  MD_STD_ROM)
	SLOT_INTERFACE_INTERNAL("rom_sram",  MD_ROM_SRAM)
	SLOT_INTERFACE_INTERNAL("rom_sramsafe",  MD_ROM_SRAM)
	SLOT_INTERFACE_INTERNAL("rom_fram",  MD_ROM_FRAM)
// add all types??
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( sk_slot )
	MCFG_MD_CARTRIDGE_ADD("subslot", sk_sub_cart, NULL)
	MCFG_MD_CARTRIDGE_NOT_MANDATORY
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor md_rom_sk_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sk_slot );
}
