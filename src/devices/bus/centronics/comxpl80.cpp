// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    COMX PL-80 plotter emulation

**********************************************************************/

/*

    TODO:

    - CPU type?

*/

#include "emu.h"
#include "comxpl80.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define CX005_TAG       "cx005"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(COMX_PL80, comx_pl80_device, "comx_pl80", "COMX PL-80")


//-------------------------------------------------
//  ROM( comxpl80 )
//-------------------------------------------------

ROM_START( comxpl80 )
	ROM_REGION( 0x1000, CX005_TAG, 0 ) // TODO: what is this? (not 6805 code)
	ROM_LOAD( "pl80.pt6",       0x0080, 0x0e00, CRC(ae059e5b) SHA1(f25812606b0082d32eb603d0a702a2187089d332) )

	ROM_REGION( 0x6000, "gfx1", ROMREGION_ERASEFF ) // Plotter fonts
	ROM_LOAD( "it.em.ou.bin",   0x2000, 0x2000, CRC(1b4a3198) SHA1(138ff6666a31c2d18cd63e609dd94d9cd1529931) )
	ROM_LOAD( "tiny.bin",       0x4000, 0x0400, CRC(940ec1ed) SHA1(ad83a3b57e2f0fbaa1e40644cd999b3f239635e8) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *comx_pl80_device::device_rom_region() const
{
	return ROM_NAME( comxpl80 );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void comx_pl80_device::device_add_mconfig(machine_config &config)
{
#if 0
	m6805_device &cx005(M6805(config, CX005_TAG, 4000000)); // CX005: some kind of MC6805/MC68HC05 clone
	cx005.set_disable();
#endif
}


//-------------------------------------------------
//  INPUT_PORTS( comxpl80 )
//-------------------------------------------------

INPUT_PORTS_START( comxpl80 )
	PORT_START("SW")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("DOWN")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("PEN-SEL")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("UP")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("CR")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("ON LINE")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("PE")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("RIGHT")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("LEFT")

	PORT_START("FONT")
	PORT_CONFNAME( 0x03, 0x00, "COMX PL-80 Font Pack")
	PORT_CONFSETTING( 0x00, DEF_STR( None ) )
	PORT_CONFSETTING( 0x01, "Italic, Emphasized and Outline" )
	PORT_CONFSETTING( 0x02, "Tiny" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor comx_pl80_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( comxpl80 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  comx_pl80_device - constructor
//-------------------------------------------------

comx_pl80_device::comx_pl80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, COMX_PL80, tag, owner, clock),
		device_centronics_peripheral_interface(mconfig, *this),
		m_plotter(*this, "gfx1"),
		m_font(*this, "FONT"),
		m_sw(*this, "SW"), m_font_addr(0), m_x_motor_phase(0), m_y_motor_phase(0), m_z_motor_phase(0), m_plotter_data(0), m_plotter_ack(0), m_plotter_online(0), m_data(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void comx_pl80_device::device_start()
{
	// state saving
	save_item(NAME(m_font_addr));
	save_item(NAME(m_x_motor_phase));
	save_item(NAME(m_y_motor_phase));
	save_item(NAME(m_z_motor_phase));
	save_item(NAME(m_plotter_data));
	save_item(NAME(m_plotter_ack));
	save_item(NAME(m_plotter_online));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void comx_pl80_device::device_reset()
{
}


//-------------------------------------------------
//  pa_w -
//-------------------------------------------------

WRITE8_MEMBER( comx_pl80_device::pa_w )
{
	/*

	    bit     description

	    0       Y motor phase A
	    1       Y motor phase B
	    2       Y motor phase C
	    3       Y motor phase D
	    4       ROM A12
	    5       ROM CE /PT5 CK
	    6       PT4 OE
	    7       SW & PE ENABLE

	*/

	m_y_motor_phase = data & 0x0f;
	m_font_addr = (BIT(data, 4) << 12) | (m_font_addr & 0xfff);

	m_plotter_data = 0xff;

	if (BIT(data, 5))
	{
		// write motor phase data
	}
	else
	{
		// read data from font ROM
		int font_rom = (m_font->read() & 0x03) * 0x2000;

		m_plotter_data = m_plotter->base()[font_rom | m_font_addr];
	}

	if (!BIT(data, 6))
	{
		// read data from Centronics bus
		m_plotter_data = m_data;
	}

	if (BIT(data, 7))
	{
		// read switches
		m_plotter_data = m_sw->read();
	}
}


//-------------------------------------------------
//  pb_w -
//-------------------------------------------------

WRITE8_MEMBER( comx_pl80_device::pb_w )
{
	/*

	    bit     description

	    0       Z motor phase A
	    1       Z motor phase B
	    2       Z motor phase C
	    3       Z motor phase D
	    4       ROM A8
	    5       ROM A9
	    6       ROM A10
	    7       ROM A11

	*/

	m_z_motor_phase = data & 0x0f;

	m_font_addr = (m_font_addr & 0x10ff) | (data << 4);
}


//-------------------------------------------------
//  pc_w -
//-------------------------------------------------

WRITE8_MEMBER( comx_pl80_device::pc_w )
{
	/*

	    bit     description

	    0       ROM A0 /X motor phase A
	    1       ROM A1 /X motor phase B
	    2       ROM A2 /X motor phase C
	    3       ROM A3 /X motor phase D
	    4       ROM A4 /ACK
	    5       ROM A5 /On-line LED
	    6       ROM A6
	    7       ROM A7

	*/

	m_font_addr = (m_font_addr & 0x1f00) | data;

	m_x_motor_phase = data & 0x0f;

	m_plotter_ack = BIT(data, 4);
	m_plotter_online = BIT(data, 5);
}


//-------------------------------------------------
//  pd_r -
//-------------------------------------------------

READ8_MEMBER( comx_pl80_device::pd_r )
{
	/*

	    bit     description

	    0       D0 /ROM D0 /DOWN SW
	    1       D1 /ROM D1 /PEN-SEL SW
	    2       D2 /ROM D2 /UP SW
	    3       D3 /ROM D3 /CRSW
	    4       D4 /ROM D4 /ON LINE SW
	    5       D5 /ROM D5 /PE Sensor
	    6       D6 /ROM D6 /RIGHT SW
	    7       D7 /ROM D7 /LEFT SW

	*/

	return m_plotter_data;
}
