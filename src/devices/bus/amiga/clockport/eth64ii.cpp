// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    ETH64 II Ethernet card

***************************************************************************/

#include "emu.h"
#include "eth64ii.h"


DEFINE_DEVICE_TYPE(AMIGA_CLOCKPORT_ETH64II, amiga_clockport_eth64ii_device, "amiga_clockport_eth64ii", "ETH64 II")

static INPUT_PORTS_START( eth64ii )
	PORT_START("JP1")
	PORT_CONFNAME( 0x01, 0x01, "Interrupt (JP1)" )
	PORT_CONFSETTING(    0x00, "Disabled" )
	PORT_CONFSETTING(    0x01, "Enabled" )
INPUT_PORTS_END

ioport_constructor amiga_clockport_eth64ii_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( eth64ii );
}

amiga_clockport_eth64ii_device::amiga_clockport_eth64ii_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, AMIGA_CLOCKPORT_ETH64II, tag, owner, clock),
	device_amiga_clockport_interface(mconfig, *this),
	m_smc(*this, "smc91c96"),
	m_jp1(*this, "JP1")
{
}

void amiga_clockport_eth64ii_device::device_start()
{
}

void amiga_clockport_eth64ii_device::device_add_mconfig(machine_config &config)
{
	SMC91C96(config, m_smc, 20_MHz_XTAL);
	m_smc->irq_handler().set(FUNC(amiga_clockport_eth64ii_device::irq_w));
}

void amiga_clockport_eth64ii_device::irq_w(int state)
{
	m_host->int6_w(state && BIT(m_jp1->read(), 0));
}

uint8_t amiga_clockport_eth64ii_device::spare_r(offs_t offset)
{
	const uint16_t mask = BIT(offset, 0) ? 0xff00 : 0x00ff;
	const uint16_t data = m_smc->read(offset >> 1, mask);

	return BIT(offset, 0) ? data >> 8 : data;
}

void amiga_clockport_eth64ii_device::spare_w(offs_t offset, uint8_t data)
{
	const uint16_t mask = BIT(offset, 0) ? 0xff00 : 0x00ff;

	m_smc->write(offset >> 1, BIT(offset, 0) ? data << 8 : data, mask);
}
