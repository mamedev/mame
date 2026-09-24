// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*
ABC SIO

PCB Layout
----------

  |-------------------------------------------|
|-|                                           |
|-|                                           |
|-|                         Z80SIO        CN2 |
|-|                                           |
|-|       4.9152MHz                           |
|-|                                           |
|-|            ROM0         Z80CTC            |
|-|                                       CN1 |
|-|            ROM1                           |
  |-------------------------------------------|

Notes:
    Relevant IC's shown.

    ROM0    - Hitachi HN462716 2Kx8 EPROM "SYN 1.6"
    ROM1    - Mitsubishi MB8516 2Kx8 EPROM "T80 1.3"
    Z80SIO  - Zilog Z-80A SIO/0
    Z80CTC  - Zilog Z-80A CTC
    CN1     - DB9 serial connector
    CN2     - DB25 serial connector

*/

#include "emu.h"
#include "sio.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define Z80CTC_TAG  "z80ctc"
#define Z80SIO_TAG  "z80sio"
#define RS232A_TAG  "rs232a"
#define RS232B_TAG  "rs232b"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ABC_SIO, abc_sio_device, "abcsio", "ABC SIO")


//-------------------------------------------------
//  ROM( abc_sio )
//-------------------------------------------------

ROM_START( abc_sio )
	ROM_REGION( 0x1000, "abc80", 0 )
	ROM_LOAD( "t80 1.3", 0x000, 0x800, CRC(f20ff827) SHA1(a1c4af1c374184a14872d7253d6f9e470603117f) )
	ROM_LOAD( "syn 1.6", 0x800, 0x800, CRC(7bd96b75) SHA1(d1f9b16530be28b03eeddb3f6ee4fa9e1cc9458e) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *abc_sio_device::device_rom_region() const
{
	return ROM_NAME( abc_sio );
}


//-------------------------------------------------
//  INPUT_PORTS( abc_sio )
//-------------------------------------------------

static INPUT_PORTS_START( abc_sio )
	PORT_START("SW1")
	PORT_CONFNAME( 0xff, 0x0a, "Card Address" )
	PORT_CONFSETTING(    0x0a, "10 (SI0/SI1)" )
	PORT_CONFSETTING(    0x0b, "11 (SI2/SI3)" )
	PORT_CONFSETTING(    0x0c, "12 (SI4/SI5)" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor abc_sio_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( abc_sio );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void abc_sio_device::device_add_mconfig(machine_config &config)
{
	Z80CTC(config, m_ctc, XTAL(4'915'200));
	m_ctc->set_clk<0>(XTAL(4'915'200)/16);
	m_ctc->set_clk<1>(XTAL(4'915'200)/16);
	m_ctc->set_clk<2>(XTAL(4'915'200)/16);
	m_ctc->zc_callback<0>().set(m_sio, FUNC(z80sio_device::txca_w));
	m_ctc->zc_callback<1>().set(m_sio, FUNC(z80sio_device::rxca_w));
	m_ctc->zc_callback<2>().set(m_sio, FUNC(z80sio_device::rxtxcb_w));

	Z80SIO(config, m_sio, XTAL(4'915'200));
	m_sio->out_txda_callback().set(m_rs232a, FUNC(rs232_port_device::write_txd));
	m_sio->out_dtra_callback().set(m_rs232a, FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsa_callback().set(m_rs232a, FUNC(rs232_port_device::write_rts));
	m_sio->out_txdb_callback().set(m_rs232b, FUNC(rs232_port_device::write_txd));
	m_sio->out_dtrb_callback().set(m_rs232b, FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsb_callback().set(m_rs232b, FUNC(rs232_port_device::write_rts));

	RS232_PORT(config, m_rs232a, default_rs232_devices, nullptr);
	m_rs232a->rxd_handler().set(m_sio, FUNC(z80sio_device::rxa_w));
	m_rs232a->cts_handler().set(m_sio, FUNC(z80sio_device::ctsa_w));
	m_rs232a->dcd_handler().set(m_sio, FUNC(z80sio_device::dcda_w));

	RS232_PORT(config, m_rs232b, default_rs232_devices, nullptr);
	m_rs232b->rxd_handler().set(m_sio, FUNC(z80sio_device::rxb_w));
	m_rs232b->cts_handler().set(m_sio, FUNC(z80sio_device::ctsb_w));
	m_rs232b->dcd_handler().set(m_sio, FUNC(z80sio_device::dcdb_w));
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc_sio_device - constructor
//-------------------------------------------------

abc_sio_device::abc_sio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ABC_SIO, tag, owner, clock),
	device_abcbus_card_interface(mconfig, *this),
	m_ctc(*this, Z80CTC_TAG),
	m_sio(*this, Z80SIO_TAG),
	m_rs232a(*this, RS232A_TAG),
	m_rs232b(*this, RS232B_TAG),
	m_rom(*this, "abc80"),
	m_sw1(*this, "SW1"),
	m_cs(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc_sio_device::device_start()
{
	save_item(NAME(m_cs));
}



//**************************************************************************
//  ABC BUS INTERFACE
//**************************************************************************

//-------------------------------------------------
//  abcbus_cs -
//-------------------------------------------------

void abc_sio_device::abcbus_cs(uint8_t data)
{
	m_cs = (data == m_sw1->read());
}


//-------------------------------------------------
//  abcbus_inp -
//-------------------------------------------------

uint8_t abc_sio_device::abcbus_inp(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_cs)
	{
		if (BIT(offset, 7))
		{
			data = m_sio->cd_ba_r(bitswap<2>(offset, 6, 5));
		}
		else
		{
			data = m_ctc->read(bitswap<2>(offset, 6, 5));
		}
	}

	return data;
}


//-------------------------------------------------
//  abcbus_out -
//-------------------------------------------------

void abc_sio_device::abcbus_out(offs_t offset, uint8_t data)
{
	if (m_cs)
	{
		if (BIT(offset, 7))
		{
			m_sio->cd_ba_w(bitswap<2>(offset, 6, 5), data);
		}
		else
		{
			m_ctc->write(bitswap<2>(offset, 6, 5), data);
		}
	}
}


//-------------------------------------------------
//  abcbus_xmemfl -
//-------------------------------------------------

uint8_t abc_sio_device::abcbus_xmemfl(offs_t offset)
{
	uint8_t data = 0xff;

	if (offset >= 0x4000 && offset < 0x5000)
	{
		data = m_rom->base()[offset & 0xfff];
	}

	return data;
}
