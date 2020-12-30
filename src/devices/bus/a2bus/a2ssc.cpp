// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2ssc.c

    Apple II Super Serial Card

*********************************************************************/

#include "emu.h"
#include "a2ssc.h"
#include "bus/rs232/rs232.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_SSC, a2bus_ssc_device, "a2ssc", "Apple Super Serial Card")

#define SSC_ROM_REGION  "ssc_rom"
#define SSC_ACIA_TAG    "ssc_acia"
#define SSC_RS232_TAG   "ssc_rs232"

ROM_START( ssc )
	ROM_REGION(0x000800, SSC_ROM_REGION, 0)
	ROM_LOAD( "341-0065-a.bin", 0x000000, 0x000800, CRC(b7539d4c) SHA1(6dab633470c6bc4cb3e81d09fda46597caf8ee57) )
ROM_END

static INPUT_PORTS_START( ssc )
	PORT_START("DSW1")
	PORT_DIPNAME( 0xf0, 0xe0, "Baud Rate" ) PORT_DIPLOCATION("SW1:4,3,2,1")
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
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x03, 0x00, "Mode" ) PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x00, "Communications Mode" )
	PORT_DIPSETTING(    0x01, "SIC P8 Emulation Mode" )
	PORT_DIPSETTING(    0x02, "Printer Mode" )
	PORT_DIPSETTING(    0x03, "SIC P8A Emulation Mode" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x80, 0x00, "Stop Bits" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, "1")
	PORT_DIPSETTING(    0x80, "2")
	PORT_DIPNAME( 0x20, 0x00, "Data Bits" ) PORT_DIPLOCATION("SW2:2") PORT_CONDITION("DSW1", 0x03, NOTEQUALS, 0x02)
	PORT_DIPSETTING(    0x20, "7")
	PORT_DIPSETTING(    0x00, "8")
	PORT_DIPNAME( 0x20, 0x00, "Delay After CR" ) PORT_DIPLOCATION("SW2:2") PORT_CONDITION("DSW1", 0x03, EQUALS, 0x02)
	PORT_DIPSETTING(    0x20, DEF_STR(None))
	PORT_DIPSETTING(    0x00, "1/4 sec")
	PORT_BIT( 0x50, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0c, 0x00, "Parity" ) PORT_DIPLOCATION("SW2:4,3") PORT_CONDITION("DSW1", 0x03, NOTEQUALS, 0x02)
	PORT_DIPSETTING(    0x00, DEF_STR(None))
	PORT_DIPSETTING(    0x08, "None (2)")
	PORT_DIPSETTING(    0x04, "Odd")
	PORT_DIPSETTING(    0x0c, "Even")
	PORT_DIPNAME( 0x0c, 0x00, "Line Width" ) PORT_DIPLOCATION("SW2:4,3") PORT_CONDITION("DSW1", 0x03, EQUALS, 0x02)
	PORT_DIPSETTING(    0x00, "40 Characters")
	PORT_DIPSETTING(    0x04, "72 Characters")
	PORT_DIPSETTING(    0x08, "80 Characters")
	PORT_DIPSETTING(    0x0c, "132 Characters")
	PORT_DIPNAME( 0x02, 0x02, "End of Line" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, "Add LF after CR")
	PORT_DIPSETTING(    0x02, "Don't add LF after CR")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER(SSC_RS232_TAG, rs232_port_device, cts_r)

	PORT_START("DSWX") // Non-memory-mapped DIP switches
	PORT_DIPNAME( 0x04, 0x04, "Interrupts" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x04, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x02, 0x00, "DTR Connected" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x01, 0x00, "Clear To Send" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, "Normal Clear To Send" )
	PORT_DIPSETTING(    0x01, "Secondary Clear To Send" )
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor a2bus_ssc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ssc );
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_ssc_device::device_add_mconfig(machine_config &config)
{
	MOS6551(config, m_acia, 0);
	m_acia->set_xtal(1.8432_MHz_XTAL);
	m_acia->irq_handler().set(FUNC(a2bus_ssc_device::acia_irq_w));
	m_acia->txd_handler().set(SSC_RS232_TAG, FUNC(rs232_port_device::write_txd));
	m_acia->dtr_handler().set(SSC_RS232_TAG, FUNC(rs232_port_device::write_dtr));

	rs232_port_device &rs232(RS232_PORT(config, SSC_RS232_TAG, default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_acia, FUNC(mos6551_device::write_rxd));
	rs232.dcd_handler().set(m_acia, FUNC(mos6551_device::write_dcd));
	rs232.dsr_handler().set(m_acia, FUNC(mos6551_device::write_dsr));
	rs232.cts_handler().set(m_acia, FUNC(mos6551_device::write_cts));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *a2bus_ssc_device::device_rom_region() const
{
	return ROM_NAME( ssc );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_ssc_device::a2bus_ssc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		a2bus_ssc_device(mconfig, A2BUS_SSC, tag, owner, clock)
{
}

a2bus_ssc_device::a2bus_ssc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, type, tag, owner, clock),
		device_a2bus_card_interface(mconfig, *this),
		m_dsw1(*this, "DSW1"),
		m_dsw2(*this, "DSW2"),
		m_dswx(*this, "DSWX"),
		m_acia(*this, SSC_ACIA_TAG),
		m_rom(*this, SSC_ROM_REGION)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_ssc_device::device_start()
{
}

void a2bus_ssc_device::device_reset()
{
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

uint8_t a2bus_ssc_device::read_cnxx(uint8_t offset)
{
	return m_rom[(offset&0xff)+0x700];
}

/*-------------------------------------------------
    read_c800 - called for reads from this card's c800 space
-------------------------------------------------*/

uint8_t a2bus_ssc_device::read_c800(uint16_t offset)
{
	return m_rom[offset];
}

/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_ssc_device::read_c0nx(uint8_t offset)
{
	// dips at C0n1/C0n2, ACIA at C0n8/9/A/B

	if (BIT(offset, 3))
		return m_acia->read(offset & 3);
	else
	{
		uint8_t buffer = 0xff;
		if (!BIT(offset, 1))
			buffer &= m_dsw1->read();
		if (!BIT(offset, 0))
			buffer &= m_dsw2->read();
		return buffer;
	}
}

/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_ssc_device::write_c0nx(uint8_t offset, uint8_t data)
{
	if (BIT(offset, 3))
		m_acia->write(offset & 3, data);
}

WRITE_LINE_MEMBER( a2bus_ssc_device::acia_irq_w )
{
	if (machine().ioport().safe_to_read())
	{
		if (!(m_dswx->read() & 4))
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
