// license:BSD-3-Clause
// copyright-holders:Carl
// Innovation SSI-2001

#include "emu.h"
#include "ssi2001.h"

#include "speaker.h"


DEFINE_DEVICE_TYPE(ISA8_SSI2001, ssi2001_device, "ssi2001", "Innovation SSI-2001 Audio Adapter")

void ssi2001_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	MOS6581(config, m_sid, XTAL(14'318'181)/16);
	m_sid->add_route(ALL_OUTPUTS, "mono", 1.00);
	PC_JOY(config, m_joy);
}

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
	m_isa->install_device(0x0200, 0x0207, read8_delegate(*subdevice<pc_joy_device>("pc_joy"), FUNC(pc_joy_device::joy_port_r)), write8_delegate(*subdevice<pc_joy_device>("pc_joy"), FUNC(pc_joy_device::joy_port_w)));
	m_isa->install_device(0x0280, 0x029F, read8sm_delegate(*subdevice<mos6581_device>("sid6581"), FUNC(mos6581_device::read)), write8sm_delegate(*subdevice<mos6581_device>("sid6581"), FUNC(mos6581_device::write)));
}

void ssi2001_device::device_reset()
{
}
