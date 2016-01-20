// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2ssc.c

    Apple II Super Serial Card

*********************************************************************/

#include "a2ssc.h"
#include "bus/rs232/rs232.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_SSC = &device_creator<a2bus_ssc_device>;

#define SSC_ROM_REGION  "ssc_rom"
#define SSC_ACIA_TAG    "ssc_acia"
#define SSC_RS232_TAG   "ssc_rs232"

MACHINE_CONFIG_FRAGMENT( ssc )
	MCFG_DEVICE_ADD(SSC_ACIA_TAG, MOS6551, 0)
	MCFG_MOS6551_XTAL(XTAL_1_8432MHz)
	MCFG_MOS6551_IRQ_HANDLER(WRITELINE(a2bus_ssc_device, acia_irq_w))
	MCFG_MOS6551_TXD_HANDLER(DEVWRITELINE(SSC_RS232_TAG, rs232_port_device, write_txd))

	MCFG_RS232_PORT_ADD(SSC_RS232_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(SSC_ACIA_TAG, mos6551_device, write_rxd))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(SSC_ACIA_TAG, mos6551_device, write_dcd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE(SSC_ACIA_TAG, mos6551_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(SSC_ACIA_TAG, mos6551_device, write_cts))
MACHINE_CONFIG_END

ROM_START( ssc )
	ROM_REGION(0x000800, SSC_ROM_REGION, 0)
	ROM_LOAD( "341-0065-a.bin", 0x000000, 0x000800, CRC(b7539d4c) SHA1(6dab633470c6bc4cb3e81d09fda46597caf8ee57) )
ROM_END

static INPUT_PORTS_START( ssc )
	PORT_START("DSW1")
	PORT_DIPNAME( 0xf0, 0xf0, "Baud rate" )
	PORT_DIPSETTING(    0x00, "Undefined/115200" )
	PORT_DIPSETTING(    0x10, "50" )
	PORT_DIPSETTING(    0x20, "75" )
	PORT_DIPSETTING(    0x30, "110" )
	PORT_DIPSETTING(    0x40, "135" )
	PORT_DIPSETTING(    0x50, "150" )
	PORT_DIPSETTING(    0x60, "300" )
	PORT_DIPSETTING(    0x70, "600" )
	PORT_DIPSETTING(    0x80, "1200" )
	PORT_DIPSETTING(    0x90, "1800" )
	PORT_DIPSETTING(    0xa0, "2400" )
	PORT_DIPSETTING(    0xb0, "3600" )
	PORT_DIPSETTING(    0xc0, "4800" )
	PORT_DIPSETTING(    0xd0, "7200" )
	PORT_DIPSETTING(    0xe0, "9600" )
	PORT_DIPSETTING(    0xf0, "19200" )

	PORT_DIPNAME( 0x0c, 0x00, "Mode" )
	PORT_DIPSETTING(    0x00, "Communications Mode" )
	PORT_DIPSETTING(    0x04, "SIC P8 Emulation Mode" )
	PORT_DIPSETTING(    0x08, "Printer Mode" )
	PORT_DIPSETTING(    0x0c, "SIC P8A Emulation Mode" )

	PORT_DIPNAME( 0x01, 0x00, "Clear To Send" )
	PORT_DIPSETTING(    0x00, "Normal Clear To Send" )
	PORT_DIPSETTING(    0x01, "Secondary Clear To Send" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0xc0, 0x00, "Format" )
	PORT_DIPSETTING(    0x00, "8 data, 1 stop")
	PORT_DIPSETTING(    0x40, "7 data, 1 stop")
	PORT_DIPSETTING(    0x80, "8 data, 2 stop")
	PORT_DIPSETTING(    0xc0, "7 data, 2 stop")

	PORT_DIPNAME( 0x30, 0x00, "Parity" )
	PORT_DIPSETTING(    0x00, "None")
	PORT_DIPSETTING(    0x10, "Odd")
	PORT_DIPSETTING(    0x30, "Even")

	PORT_DIPNAME( 0x08, 0x08, "End of Line" )
	PORT_DIPSETTING(    0x00, "Add LF after CR")
	PORT_DIPSETTING(    0x08, "Don't add LF after CR")

	PORT_DIPNAME( 0x04, 0x04, "Interrupts" )
	PORT_DIPSETTING(    0x00, "On")
	PORT_DIPSETTING(    0x04, "Off")
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor a2bus_ssc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ssc );
}

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a2bus_ssc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ssc );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *a2bus_ssc_device::device_rom_region() const
{
	return ROM_NAME( ssc );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_ssc_device::a2bus_ssc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, A2BUS_SSC, "Apple Super Serial Card", tag, owner, clock, "a2ssc", __FILE__),
		device_a2bus_card_interface(mconfig, *this),
		m_dsw1(*this, "DSW1"),
		m_dsw2(*this, "DSW2"),
		m_acia(*this, SSC_ACIA_TAG), m_rom(nullptr),
		m_started(false)
{
}

a2bus_ssc_device::a2bus_ssc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_a2bus_card_interface(mconfig, *this),
		m_dsw1(*this, "DSW1"),
		m_dsw2(*this, "DSW2"),
		m_acia(*this, SSC_ACIA_TAG), m_rom(nullptr),
		m_started(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_ssc_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();

	m_rom = device().machine().root_device().memregion(this->subtag(SSC_ROM_REGION).c_str())->base();
}

void a2bus_ssc_device::device_reset()
{
	m_started = true;
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

UINT8 a2bus_ssc_device::read_cnxx(address_space &space, UINT8 offset)
{
	return m_rom[(offset&0xff)+0x700];
}

/*-------------------------------------------------
    read_c800 - called for reads from this card's c800 space
-------------------------------------------------*/

UINT8 a2bus_ssc_device::read_c800(address_space &space, UINT16 offset)
{
	return m_rom[offset];
}

/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

UINT8 a2bus_ssc_device::read_c0nx(address_space &space, UINT8 offset)
{
	// dips at C0n1/C0n2, ACIA at C0n8/9/A/B

	switch (offset)
	{
		case 1:
			return m_dsw1->read();
		case 2:
			return m_dsw2->read();

		case 8:
		case 9:
		case 0xa:
		case 0xb:
			return m_acia->read(space, offset-8);

	}

	return 0;
}

/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_ssc_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
	switch (offset)
	{
		case 8:
		case 9:
		case 0xa:
		case 0xb:
			m_acia->write(space, offset-8, data);
			break;
	}
}

WRITE_LINE_MEMBER( a2bus_ssc_device::acia_irq_w )
{
	if (m_started)
	{
		if (!(m_dsw2->read() & 4))
		{
			if (state)
			{
				raise_slot_irq();
			}
			else
			{
				lower_slot_irq();
			}
		}
	}
}
