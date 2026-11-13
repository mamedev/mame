// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    RR-Net Ethernet card

    The card inverts A3, which swaps the two halves of the CS8900A I/O space.

***************************************************************************/

#include "emu.h"
#include "rrnet.h"


DEFINE_DEVICE_TYPE(AMIGA_CLOCKPORT_RRNET, amiga_clockport_rrnet_device, "amiga_clockport_rrnet", "RR-Net")

amiga_clockport_rrnet_device::amiga_clockport_rrnet_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, AMIGA_CLOCKPORT_RRNET, tag, owner, clock),
	device_amiga_clockport_interface(mconfig, *this),
	m_cs8900a(*this, "cs8900a")
{
}

void amiga_clockport_rrnet_device::device_start()
{
}

void amiga_clockport_rrnet_device::device_add_mconfig(machine_config &config)
{
	CS8900A(config, m_cs8900a, 20_MHz_XTAL);
}

uint8_t amiga_clockport_rrnet_device::spare_r(offs_t offset)
{
	return m_cs8900a->read(offset ^ 8);
}

void amiga_clockport_rrnet_device::spare_w(offs_t offset, uint8_t data)
{
	m_cs8900a->write(offset ^ 8, data);
}
