// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    ETH64 ShortBus card

***************************************************************************/

#include "emu.h"
#include "eth64.h"


DEFINE_DEVICE_TYPE(IDE64_SHORTBUS_ETH64, ide64_shortbus_eth64_device, "ide64_shortbus_eth64", "ETH64 (ShortBus)")

static INPUT_PORTS_START( eth64 )
	PORT_START("JP1")
	PORT_CONFNAME( 0x01, 0x00, "Address Select (JP1)" )
	PORT_CONFSETTING(    0x00, "CSEL0 ($DE00-$DE0F)" )
	PORT_CONFSETTING(    0x01, "CSEL1 ($DE10-$DE1F)" )

	PORT_START("JP2")
	PORT_CONFNAME( 0x01, 0x01, "NMI Generation (JP2)" )
	PORT_CONFSETTING(    0x00, "Disabled" )
	PORT_CONFSETTING(    0x01, "Enabled" )
INPUT_PORTS_END

ioport_constructor ide64_shortbus_eth64_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( eth64 );
}

ide64_shortbus_eth64_device::ide64_shortbus_eth64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, IDE64_SHORTBUS_ETH64, tag, owner, clock),
	device_ide64_shortbus_interface(mconfig, *this),
	m_smc(*this, "smc91c96"),
	m_jp1(*this, "JP1"),
	m_jp2(*this, "JP2")
{
}

void ide64_shortbus_eth64_device::device_start()
{
}

void ide64_shortbus_eth64_device::device_add_mconfig(machine_config &config)
{
	SMC91C96(config, m_smc, 20_MHz_XTAL);
	m_smc->irq_handler().set(FUNC(ide64_shortbus_eth64_device::irq_w));
}

void ide64_shortbus_eth64_device::irq_w(int state)
{
	m_host->nmi_w(state && BIT(m_jp2->read(), 0));
}

uint8_t ide64_shortbus_eth64_device::shortbus_r(offs_t offset, uint8_t data, int io1, int io2, int csel)
{
	if (csel & (BIT(m_jp1->read(), 0) ? ide64_shortbus_device::CSEL1 : ide64_shortbus_device::CSEL0))
	{
		const uint16_t mask = BIT(offset, 0) ? 0xff00 : 0x00ff;
		const uint16_t value = m_smc->read((offset & 0x0f) >> 1, mask);

		data = BIT(offset, 0) ? value >> 8 : value;
	}

	return data;
}

void ide64_shortbus_eth64_device::shortbus_w(offs_t offset, uint8_t data, int io1, int io2, int csel)
{
	if (csel & (BIT(m_jp1->read(), 0) ? ide64_shortbus_device::CSEL1 : ide64_shortbus_device::CSEL0))
	{
		const uint16_t mask = BIT(offset, 0) ? 0xff00 : 0x00ff;

		m_smc->write((offset & 0x0f) >> 1, BIT(offset, 0) ? data << 8 : data, mask);
	}
}
