// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Peaksoft Prestel Module

    Also known as the New Era Interface from Peaksoft/Harry Whitehouse, which
    includes a through port for DOS cartridges.

    TODO:
    - RX/TX rate set by dipswitches (default 100001/001001?), for use with a 1200/75 modem.
    - add throughport, not known how CTS/SCS lines are switched between slots.

***************************************************************************/

#include "emu.h"
#include "dragon_serial.h"
#include "machine/clock.h"
#include "bus/rs232/rs232.h"


//-------------------------------------------------
//  ROM( dragon_serial )
//-------------------------------------------------

ROM_START(dragon_serial)
	ROM_REGION(0x2000, "eprom", 0)
	ROM_LOAD("comron_peaksoft.rom", 0x0000, 0x2000, CRC(9d18cf46) SHA1(14124dfb4bd78d1907e80d779cd7f3bae30564c9))
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(DRAGON_SERIAL, dragon_serial_device, "dragon_serial", "Dragon Peaksoft Prestel Module")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dragon_serial_device - constructor
//-------------------------------------------------

dragon_serial_device::dragon_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, DRAGON_SERIAL, tag, owner, clock)
	, device_cococart_interface(mconfig, *this )
	, m_eprom(*this, "eprom")
	, m_acia(*this, "acia")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dragon_serial_device::device_start()
{
	set_line_value(line::CART, line_value::Q);
}

//-------------------------------------------------
//  dragon_serial_device::get_cart_base
//-------------------------------------------------

u8 *dragon_serial_device::get_cart_base()
{
	return m_eprom->base();
}

//-------------------------------------------------
//  dragon_serial_device::get_cart_memregion
//-------------------------------------------------

memory_region *dragon_serial_device::get_cart_memregion()
{
	return m_eprom;
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void dragon_serial_device::device_add_mconfig(machine_config &config)
{
	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_acia->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_acia->irq_handler().set([this](int state) { set_line_value(line::NMI, state); });

	clock_device &acia_clock(CLOCK(config, "acia_clock", 2.4576_MHz_XTAL));
	acia_clock.signal_handler().set(FUNC(dragon_serial_device::write_acia_clock));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "null_modem"));
	rs232.rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
	rs232.cts_handler().set(m_acia, FUNC(acia6850_device::write_cts));
	rs232.dcd_handler().set(m_acia, FUNC(acia6850_device::write_dcd));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *dragon_serial_device::device_rom_region() const
{
	return ROM_NAME( dragon_serial );
}

//-------------------------------------------------
//  cts_read
//-------------------------------------------------

u8 dragon_serial_device::cts_read(offs_t offset)
{
	return m_eprom->base()[offset & 0x1fff];
}

//-------------------------------------------------
//  scs_read
//-------------------------------------------------

u8 dragon_serial_device::scs_read(offs_t offset)
{
	uint8_t result = 0x00;

	switch (offset)
	{
	case 0x14: case 0x15:
		result = m_acia->read(offset & 1);
		break;
	}
	return result;
}

//-------------------------------------------------
//  scs_write
//-------------------------------------------------

void dragon_serial_device::scs_write(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0x14: case 0x15:
		m_acia->write(offset & 1, data);
		break;
	}
}

WRITE_LINE_MEMBER(dragon_serial_device::write_acia_clock)
{
	m_acia->write_txc(state);
	m_acia->write_rxc(state);
}
