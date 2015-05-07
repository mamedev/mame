// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    COMX PL-80 plotter emulation

**********************************************************************/

/*

    TODO:

    - CPU type?

*/

#include "comxpl80.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define CX005_TAG       "cx005"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type COMX_PL80 = &device_creator<comx_pl80_device>;


//-------------------------------------------------
//  ROM( comxpl80 )
//-------------------------------------------------

ROM_START( comxpl80 )
	ROM_REGION( 0x1000, CX005_TAG, 0 )
	ROM_LOAD( "pl80.pt6",       0x0080, 0x0e00, CRC(ae059e5b) SHA1(f25812606b0082d32eb603d0a702a2187089d332) )

	ROM_REGION( 0x6000, "gfx1", ROMREGION_ERASEFF ) // Plotter fonts
	ROM_LOAD( "it.em.ou.bin",   0x2000, 0x2000, CRC(1b4a3198) SHA1(138ff6666a31c2d18cd63e609dd94d9cd1529931) )
	ROM_LOAD( "tiny.bin",       0x4000, 0x0400, CRC(940ec1ed) SHA1(ad83a3b57e2f0fbaa1e40644cd999b3f239635e8) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *comx_pl80_device::device_rom_region() const
{
	return ROM_NAME( comxpl80 );
}


//-------------------------------------------------
//  ADDRESS_MAP( comxpl80_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( comxpl80_mem, AS_PROGRAM, 8, comx_pl80_device )
/*  AM_RANGE(0x000, 0x000) AM_READWRITE(cx005_port_a_r, cx005_port_a_w)
    AM_RANGE(0x001, 0x001) AM_READWRITE(cx005_port_b_r, cx005_port_b_w)
    AM_RANGE(0x002, 0x002) AM_READWRITE(cx005_port_c_r, cx005_port_c_w)
    AM_RANGE(0x003, 0x003) AM_READ(cx005_port_d_digital_r)
    AM_RANGE(0x004, 0x004) AM_WRITE(cx005_port_a_ddr_w)
    AM_RANGE(0x005, 0x005) AM_WRITE(cx005_port_b_ddr_w)
    AM_RANGE(0x006, 0x006) AM_WRITE(cx005_port_c_ddr_w)
    AM_RANGE(0x007, 0x007) AM_READ(cx005_port_d_analog_r)
    AM_RANGE(0x008, 0x008) AM_READWRITE(cx005_timer_data_r, cx005_timer_data_w)
    AM_RANGE(0x008, 0x008) AM_READWRITE(cx005_timer_ctrl_r, cx005_timer_ctrl_w)*/
	AM_RANGE(0x00a, 0x01f) AM_NOP // Not Used
	AM_RANGE(0x020, 0x07f) AM_RAM // Internal RAM
	AM_RANGE(0x080, 0xf7f) AM_ROM AM_REGION(CX005_TAG, 0) // Internal ROM
	AM_RANGE(0xf80, 0xff7) AM_ROM AM_REGION(CX005_TAG, 0xf00)  // Self-Test
	AM_RANGE(0xff8, 0xfff) AM_ROM AM_REGION(CX005_TAG, 0xf78)  // Interrupt Vectors
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( comxpl80_io )
//-------------------------------------------------

static ADDRESS_MAP_START( comxpl80_io, AS_IO, 8, comx_pl80_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x00) AM_WRITE(pa_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(pb_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(pc_w)
	AM_RANGE(0x03, 0x03) AM_READ(pd_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( comxpl80 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( comxpl80 )
	MCFG_CPU_ADD(CX005_TAG, M6805, 4000000) // CX005: some kind of MC6805/MC68HC05 clone
	MCFG_CPU_PROGRAM_MAP(comxpl80_mem)
	MCFG_CPU_IO_MAP(comxpl80_io)
	MCFG_DEVICE_DISABLE()
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor comx_pl80_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( comxpl80 );
}


//-------------------------------------------------
//  INPUT_PORTS( comxpl80 )
//-------------------------------------------------

INPUT_PORTS_START( comxpl80 )
	PORT_START("SW")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("DOWN")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("PEN-SEL")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("UP")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("CR")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("ON LINE")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("PE")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("RIGHT")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("LEFT")

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

comx_pl80_device::comx_pl80_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, COMX_PL80, "COMX PL-80", tag, owner, clock, "comx_pl80", __FILE__),
		device_centronics_peripheral_interface(mconfig, *this),
		m_plotter(*this, "PLOTTER"),
		m_font(*this, "FONT"),
		m_sw(*this, "SW")
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
