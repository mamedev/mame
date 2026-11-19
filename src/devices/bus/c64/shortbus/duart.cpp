// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    DUART64 ShortBus card

***************************************************************************/

#include "emu.h"
#include "duart.h"


DEFINE_DEVICE_TYPE(IDE64_SHORTBUS_DUART, ide64_shortbus_duart_device, "ide64_shortbus_duart", "DUART64 (ShortBus)")

static INPUT_PORTS_START( duart )
	PORT_START("JP1")
	PORT_CONFNAME( 0x01, 0x00, "Address Select (JP1)" )
	PORT_CONFSETTING(    0x00, "CSEL0 ($DE00-$DE0F)" )
	PORT_CONFSETTING(    0x01, "CSEL1 ($DE10-$DE1F)" )

	PORT_START("JP2")
	PORT_CONFNAME( 0x01, 0x01, "NMI Generation (JP2)" )
	PORT_CONFSETTING(    0x00, "Disabled" )
	PORT_CONFSETTING(    0x01, "Enabled" )
INPUT_PORTS_END

ioport_constructor ide64_shortbus_duart_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( duart );
}

ide64_shortbus_duart_device::ide64_shortbus_duart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, IDE64_SHORTBUS_DUART, tag, owner, clock),
	device_ide64_shortbus_interface(mconfig, *this),
	m_duart(*this, "duart"),
	m_rs232(*this, "rs232%c", 'a'),
	m_jp1(*this, "JP1"),
	m_jp2(*this, "JP2")
{
}

void ide64_shortbus_duart_device::device_start()
{
}

void ide64_shortbus_duart_device::device_add_mconfig(machine_config &config)
{
	XR68C681(config, m_duart, 3.6864_MHz_XTAL);
	m_duart->irq_cb().set(FUNC(ide64_shortbus_duart_device::irq_w));
	m_duart->a_tx_cb().set(m_rs232[0], FUNC(rs232_port_device::write_txd));
	m_duart->b_tx_cb().set(m_rs232[1], FUNC(rs232_port_device::write_txd));
	m_duart->outport_cb().set(FUNC(ide64_shortbus_duart_device::outport_w));

	RS232_PORT(config, m_rs232[0], default_rs232_devices, nullptr);
	m_rs232[0]->rxd_handler().set(m_duart, FUNC(xr68c681_device::rx_a_w));
	m_rs232[0]->cts_handler().set(m_duart, FUNC(xr68c681_device::ip0_w));
	m_rs232[0]->dsr_handler().set(m_duart, FUNC(xr68c681_device::ip2_w));
	m_rs232[0]->dcd_handler().set(m_duart, FUNC(xr68c681_device::ip4_w));

	RS232_PORT(config, m_rs232[1], default_rs232_devices, nullptr);
	m_rs232[1]->rxd_handler().set(m_duart, FUNC(xr68c681_device::rx_b_w));
	m_rs232[1]->cts_handler().set(m_duart, FUNC(xr68c681_device::ip1_w));
	m_rs232[1]->dsr_handler().set(m_duart, FUNC(xr68c681_device::ip3_w));
	m_rs232[1]->dcd_handler().set(m_duart, FUNC(xr68c681_device::ip5_w));
}

void ide64_shortbus_duart_device::irq_w(int state)
{
	if (BIT(m_jp2->read(), 0))
		m_host->nmi_w(state);
}

void ide64_shortbus_duart_device::outport_w(uint8_t data)
{
	m_rs232[0]->write_rts(BIT(data, 0));
	m_rs232[1]->write_rts(BIT(data, 1));
	m_rs232[0]->write_dtr(BIT(data, 2));
	m_rs232[1]->write_dtr(BIT(data, 3));
}

uint8_t ide64_shortbus_duart_device::shortbus_r(offs_t offset, uint8_t data, int io1, int io2, int csel)
{
	if (csel & (BIT(m_jp1->read(), 0) ? ide64_shortbus_device::CSEL1 : ide64_shortbus_device::CSEL0))
		data = m_duart->read(offset & 0x0f);

	return data;
}

void ide64_shortbus_duart_device::shortbus_w(offs_t offset, uint8_t data, int io1, int io2, int csel)
{
	if (csel & (BIT(m_jp1->read(), 0) ? ide64_shortbus_device::CSEL1 : ide64_shortbus_device::CSEL0))
		m_duart->write(offset & 0x0f, data);
}
