// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

 Master Gear Converter emulation

 The Master Gear Converter, also known as Master Gear, Gear Master Converter
 or (in Brazil) as Master Gear Adaptor, allows to plug western SMS cartridges
 on the Game Gear, by enabling the SMS compatibility mode on the Game Gear
 cartridge slot. Some SMS games have compatibility issues, confirmed on the
 real hardware, when run on the Game Gear.

 ***********************************************************************************************************/


#include "emu.h"
#include "mgear.h"
#include "softlist.h"

//-------------------------------------------------
//  constructors
//-------------------------------------------------

DEFINE_DEVICE_TYPE(SEGA8_ROM_MGEAR, sega8_mgear_device, "sega8_mgear", "Master Gear Converter")

sega8_mgear_device::sega8_mgear_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sega8_rom_device(mconfig, SEGA8_ROM_MGEAR, tag, owner, clock), m_subslot(*this, "subslot")
{
}


void sega8_mgear_device::device_start()
{
}

void sega8_mgear_device::device_reset()
{
}

/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

static MACHINE_CONFIG_START( sub_slot )
	MCFG_SMS_CARTRIDGE_ADD("subslot", sms_cart, nullptr)
	MCFG_SOFTWARE_LIST_ADD("cart_list","sms")
MACHINE_CONFIG_END

machine_config_constructor sega8_mgear_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sub_slot );
}
