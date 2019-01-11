// license:BSD-3-Clause
// copyright-holders:Carl
// Innovation SSI-2001

#include "emu.h"
#include "ssi2001.h"

#include "speaker.h"


DEFINE_DEVICE_TYPE(ISA8_SSI2001, ssi2001_device, "ssi2001", "Innovation SSI-2001 Audio Adapter")

MACHINE_CONFIG_START(ssi2001_device::device_add_mconfig)
	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD("sid6581", MOS6581, XTAL(14'318'181)/16)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
	MCFG_PC_JOY_ADD("pc_joy")
MACHINE_CONFIG_END

ssi2001_device::ssi2001_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA8_SSI2001, tag, owner, clock),
	device_isa8_card_interface(mconfig, *this),
	m_joy(*this, "pc_joy"),
	m_sid(*this, "sid6581")
{
}

void ssi2001_device::device_start()
{
	set_isa_device();
	m_isa->install_device(0x0200, 0x0207, read8_delegate(FUNC(pc_joy_device::joy_port_r), subdevice<pc_joy_device>("pc_joy")), write8_delegate(FUNC(pc_joy_device::joy_port_w), subdevice<pc_joy_device>("pc_joy")));
	m_isa->install_device(0x0280, 0x029F, read8_delegate(FUNC(mos6581_device::read), subdevice<mos6581_device>("sid6581")), write8_delegate(FUNC(mos6581_device::write), subdevice<mos6581_device>("sid6581")));
}


void ssi2001_device::device_reset()
{
}
