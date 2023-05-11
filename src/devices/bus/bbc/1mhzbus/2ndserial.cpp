// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Sprow 2nd Serial Port

    http://www.sprow.co.uk/bbc/extraserial.htm

**********************************************************************/


#include "emu.h"
#include "2ndserial.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_2NDSERIAL, bbc_2ndserial_device, "bbc_2ndserial", "Sprow 2nd Serial Port");


//-------------------------------------------------
//  INPUT_PORTS( 2ndserial )
//-------------------------------------------------

INPUT_PORTS_START( 2ndserial )
	PORT_START("LINKS")
	PORT_CONFNAME(0x03, 0x00, "Base Address")
	PORT_CONFSETTING(0x00, "&FC60")
	PORT_CONFSETTING(0x01, "&FC64")
	PORT_CONFSETTING(0x02, "&FC68")
	PORT_CONFSETTING(0x03, "&FC6C")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor bbc_2ndserial_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( 2ndserial );
}


//-------------------------------------------------
//  ROM( 2ndserial )
//-------------------------------------------------

ROM_START( 2ndserial )
	ROM_REGION(0x2000, "exp_rom", 0)
	ROM_LOAD("2ndserial.rom", 0x0000, 0x2000, CRC(e58cdb76) SHA1(9ebbe845734d77ebf1730ea622ce2db43f18b1c2))
ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_2ndserial_device::device_add_mconfig(machine_config &config)
{
	ACIA6850(config, m_acia);
	m_acia->txd_handler().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_acia->rts_handler().set(m_rs232, FUNC(rs232_port_device::write_rts));
	m_acia->irq_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::irq_w));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
	m_rs232->cts_handler().set(m_acia, FUNC(acia6850_device::write_cts));

	CLOCK(config, m_acia_clock_tx, 2.4576_MHz_XTAL);
	m_acia_clock_tx->signal_handler().set(m_acia, FUNC(acia6850_device::write_txc));
	CLOCK(config, m_acia_clock_rx, 2.4576_MHz_XTAL);
	m_acia_clock_rx->signal_handler().set(m_acia, FUNC(acia6850_device::write_rxc));

	BBC_1MHZBUS_SLOT(config, m_1mhzbus, DERIVED_CLOCK(1, 1), bbc_1mhzbus_devices, nullptr);
	m_1mhzbus->irq_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::irq_w));
	m_1mhzbus->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::nmi_w));
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry* bbc_2ndserial_device::device_rom_region() const
{
	return ROM_NAME(2ndserial);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_2ndserial_device - constructor
//-------------------------------------------------

bbc_2ndserial_device::bbc_2ndserial_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_2NDSERIAL, tag, owner, clock)
	, device_bbc_1mhzbus_interface(mconfig, *this)
	, m_1mhzbus(*this, "1mhzbus")
	, m_acia(*this, "acia6850")
	, m_acia_clock_tx(*this, "acia_clock_tx")
	, m_acia_clock_rx(*this, "acia_clock_rx")
	, m_rs232(*this, "rs232")
	, m_links(*this, "LINKS")
	, m_control(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_2ndserial_device::device_start()
{
	/* register for save states */
	save_item(NAME(m_control));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_2ndserial_device::fred_r(offs_t offset)
{
	uint8_t data = 0xff;
	uint8_t base = 0x60 + (m_links->read() << 2);

	if ((offset & 0xfc) == base)
	{
		switch (offset & 0x02)
		{
		case 0x00:
			data = m_acia->read(offset & 1);
			break;
		case 0x02:
			break;
		}
	}

	data &= m_1mhzbus->fred_r(offset);

	return data;
}

void bbc_2ndserial_device::fred_w(offs_t offset, uint8_t data)
{
	static const int serial_clocks[8] =
	{
		1,    // 000
		16,   // 001
		4,    // 010
		128,  // 011
		2,    // 100
		64,   // 101
		8,    // 110
		256   // 111
	};

	uint8_t base = 0x60 + (m_links->read() << 2);

	if ((offset & 0xfc) == base)
	{
		switch (offset & 0x02)
		{
		case 0x00:
			m_acia->write(offset & 1, data);
			break;
		case 0x02:
			m_control = data;
			m_acia_clock_tx->set_clock_scale((double) 1 / serial_clocks[data & 0x07] );
			m_acia_clock_rx->set_clock_scale((double) 1 / serial_clocks[data & 0x07] );
			break;
		}
	}

	m_1mhzbus->fred_w(offset, data);
}

uint8_t bbc_2ndserial_device::jim_r(offs_t offset)
{
	return m_1mhzbus->jim_r(offset);
}

void bbc_2ndserial_device::jim_w(offs_t offset, uint8_t data)
{
	m_1mhzbus->jim_w(offset, data);
}


WRITE_LINE_MEMBER(bbc_2ndserial_device::write_acia_clock)
{
	m_acia->write_txc(state);
	m_acia->write_rxc(state);
}
