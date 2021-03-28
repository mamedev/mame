// license:BSD-3-Clause
// copyright-holders:Enik Land
/***********************************************************************************************************

 Sega 3-D Adaptor emulation

 The 3-D Adaptor allows to plug the Sega 3-D glasses into the card slot of
 a Mark III or Master System 1 console.

 ***********************************************************************************************************/


#include "emu.h"
#include "3dadp.h"

//-------------------------------------------------
//  constructors
//-------------------------------------------------

DEFINE_DEVICE_TYPE(SEGA8_ROM_3D_ADAPTOR, sega8_3d_adaptor_device, "sega8_3d_adaptor", "3-D Adaptor")

sega8_3d_adaptor_device::sega8_3d_adaptor_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sega8_rom_device(mconfig, SEGA8_ROM_3D_ADAPTOR, tag, owner, clock), m_port_3d(*this, "3dport")
{
}


void sega8_3d_adaptor_device::device_add_mconfig(machine_config &config)
{
	SMS_3D_PORT(config, m_port_3d, sms_3d_port_devices, "3dglass");
	if (dynamic_cast<sms_card_slot_device *>(m_slot)) {
		m_port_3d->set_screen_tag(dynamic_cast<sms_card_slot_device *>(m_slot)->m_screen);
	}
}

