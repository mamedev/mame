// license:BSD-3-Clause
// copyright-holders:Carl
// Innovation SSI-2001

#include "ssi2001.h"

const device_type ISA8_SSI2001 = &device_creator<ssi2001_device>;

static MACHINE_CONFIG_FRAGMENT( ssi2001 )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("sid6581", MOS6581, XTAL_14_31818MHz/16)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
	MCFG_PC_JOY_ADD("pc_joy")
MACHINE_CONFIG_END

machine_config_constructor ssi2001_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ssi2001 );
}

ssi2001_device::ssi2001_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, ISA8_SSI2001, "Innovation SSI-2001 Audio Adapter", tag, owner, clock, "ssi2001", __FILE__),
	device_isa8_card_interface(mconfig, *this),
	m_joy(*this, "pc_joy"),
	m_sid(*this, "sid6581")
{
}

void ssi2001_device::device_start()
{
	set_isa_device();
	m_isa->install_device(0x0200, 0x0207, 0, 0, read8_delegate(FUNC(pc_joy_device::joy_port_r), subdevice<pc_joy_device>("pc_joy")), write8_delegate(FUNC(pc_joy_device::joy_port_w), subdevice<pc_joy_device>("pc_joy")));
	m_isa->install_device(0x0280, 0x029F, 0, 0, read8_delegate(FUNC(mos6581_device::read), subdevice<mos6581_device>("sid6581")), write8_delegate(FUNC(mos6581_device::write), subdevice<mos6581_device>("sid6581")));
}


void ssi2001_device::device_reset()
{
}
