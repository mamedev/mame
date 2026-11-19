// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    ETFE ShortBus card

***************************************************************************/

#include "emu.h"
#include "etfe.h"


DEFINE_DEVICE_TYPE(IDE64_SHORTBUS_ETFE, ide64_shortbus_etfe_device, "ide64_shortbus_etfe", "ETFE (ShortBus)")

static INPUT_PORTS_START( etfe )
	PORT_START("BASE")
	PORT_CONFNAME( 0x03, 0x00, "Base Address" )
	PORT_CONFSETTING(    0x00, "$DE00 (CSEL0)" )
	PORT_CONFSETTING(    0x01, "$DE10 (CSEL1)" )
	PORT_CONFSETTING(    0x02, "$DF00 (/IO2)" )
INPUT_PORTS_END

ioport_constructor ide64_shortbus_etfe_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( etfe );
}

ide64_shortbus_etfe_device::ide64_shortbus_etfe_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, IDE64_SHORTBUS_ETFE, tag, owner, clock),
	device_ide64_shortbus_interface(mconfig, *this),
	m_cs8900a(*this, "cs8900a"),
	m_base(*this, "BASE")
{
}

void ide64_shortbus_etfe_device::device_start()
{
}

void ide64_shortbus_etfe_device::device_add_mconfig(machine_config &config)
{
	CS8900A(config, m_cs8900a, 20_MHz_XTAL);
}

bool ide64_shortbus_etfe_device::selected(offs_t offset, int io1, int io2, int csel)
{
	switch (m_base->read())
	{
	case 0: return csel & ide64_shortbus_device::CSEL0;
	case 1: return csel & ide64_shortbus_device::CSEL1;
	default: return !io2 && (offset & 0xf0) == 0x00;
	}
}

uint8_t ide64_shortbus_etfe_device::shortbus_r(offs_t offset, uint8_t data, int io1, int io2, int csel)
{
	if (selected(offset, io1, io2, csel))
		data = m_cs8900a->read(offset & 0x0f);

	return data;
}

void ide64_shortbus_etfe_device::shortbus_w(offs_t offset, uint8_t data, int io1, int io2, int csel)
{
	if (selected(offset, io1, io2, csel))
		m_cs8900a->write(offset & 0x0f, data);
}
