// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    IBM 5150 83-key keyboard emulation

*********************************************************************/

#include "pc83.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I8048_TAG       "u1"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type PC_KBD_IBM_PC_83 = &device_creator<ibm_pc_83_keyboard_device>;


//-------------------------------------------------
//  ROM( ibm_pc_83_keyboard )
//-------------------------------------------------

ROM_START( ibm_pc_83_keyboard )
	ROM_REGION( 0x400, I8048_TAG, 0 )
	ROM_LOAD( "8048.u1", 0x000, 0x400, NO_DUMP )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *ibm_pc_83_keyboard_device::device_rom_region() const
{
	return ROM_NAME( ibm_pc_83_keyboard );
}


//-------------------------------------------------
//  ADDRESS_MAP( kb_io )
//-------------------------------------------------

static ADDRESS_MAP_START( ibm_pc_83_keyboard_io, AS_IO, 8, ibm_pc_83_keyboard_device )
	AM_RANGE(MCS48_PORT_BUS, MCS48_PORT_BUS) AM_WRITE(bus_w)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_READ(p1_r) AM_WRITENOP
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_WRITE(p2_w)
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(t1_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( ibm_pc_83_keyboard )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( ibm_pc_83_keyboard )
	MCFG_CPU_ADD(I8048_TAG, I8048, MCS48_LC_CLOCK(IND_U(47), CAP_P(20)))
	MCFG_CPU_IO_MAP(ibm_pc_83_keyboard_io)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor ibm_pc_83_keyboard_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ibm_pc_83_keyboard );
}


//-------------------------------------------------
//  INPUT_PORTS( ibm_pc_83_keyboard )
//-------------------------------------------------

INPUT_PORTS_START( ibm_pc_83_keyboard )
	PORT_START("DR00")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("DR01")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("DR02")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("DR03")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("DR04")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("DR05")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("DR06")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("DR07")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("DR08")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("DR09")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("DR10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("DR11")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("DR12")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("DR13")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("DR14")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("DR15")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("DR16")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("DR17")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("DR18")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("DR19")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("DR20")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("DR21")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("DR22")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("DR23")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor ibm_pc_83_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ibm_pc_83_keyboard );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ibm_pc_83_keyboard_device - constructor
//-------------------------------------------------

ibm_pc_83_keyboard_device::ibm_pc_83_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PC_KBD_IBM_PC_83, "IBM PC Keyboard", tag, owner, clock, "kb_pc83", __FILE__),
		device_pc_kbd_interface(mconfig, *this),
		m_maincpu(*this, I8048_TAG),
		m_dr00(*this, "DR00"),
		m_dr01(*this, "DR01"),
		m_dr02(*this, "DR02"),
		m_dr03(*this, "DR03"),
		m_dr04(*this, "DR04"),
		m_dr05(*this, "DR05"),
		m_dr06(*this, "DR06"),
		m_dr07(*this, "DR07"),
		m_dr08(*this, "DR08"),
		m_dr09(*this, "DR09"),
		m_dr10(*this, "DR10"),
		m_dr11(*this, "DR11"),
		m_dr12(*this, "DR12"),
		m_dr13(*this, "DR13"),
		m_dr14(*this, "DR14"),
		m_dr15(*this, "DR15"),
		m_dr16(*this, "DR16"),
		m_dr17(*this, "DR17"),
		m_dr18(*this, "DR18"),
		m_dr19(*this, "DR19"),
		m_dr20(*this, "DR20"),
		m_dr21(*this, "DR21"),
		m_dr22(*this, "DR22"),
		m_dr23(*this, "DR23")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ibm_pc_83_keyboard_device::device_start()
{
	// state saving
	save_item(NAME(m_cnt));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ibm_pc_83_keyboard_device::device_reset()
{
	m_maincpu->reset();
}


//-------------------------------------------------
//  bus_w -
//-------------------------------------------------

WRITE8_MEMBER( ibm_pc_83_keyboard_device::bus_w )
{
	/*

	    bit     description

	    0       CNT 1
	    1       CNT 2
	    2       CNT 4
	    3       CNT 8
	    4       CNT 16
	    5       CNT 32
	    6       CNT 64
	    7

	*/

	m_cnt = data & 0x7f;
}


//-------------------------------------------------
//  p1_r -
//-------------------------------------------------

READ8_MEMBER( ibm_pc_83_keyboard_device::p1_r )
{
	/*

	    bit     description

	    0       -REQ IN
	    1       DATA IN
	    2
	    3
	    4
	    5
	    6
	    7

	*/

	UINT8 data = 0;

	data |= clock_signal();
	data |= data_signal() << 1;

	return data;
}


//-------------------------------------------------
//  p2_w -
//-------------------------------------------------

WRITE8_MEMBER( ibm_pc_83_keyboard_device::p2_w )
{
	/*

	    bit     description

	    0       -MATRIX STROBE
	    1       CLOCK OUT
	    2       DATA OUT
	    3
	    4
	    5
	    6
	    7

	*/

	m_pc_kbdc->clock_write_from_kb(BIT(data, 1));
	m_pc_kbdc->data_write_from_kb(BIT(data, 2));
}


//-------------------------------------------------
//  t1_r -
//-------------------------------------------------

READ8_MEMBER( ibm_pc_83_keyboard_device::t1_r )
{
	UINT8 data = 0xff;

	switch (m_cnt >> 2)
	{
	case  0: data = m_dr00->read(); break;
	case  1: data = m_dr01->read(); break;
	case  2: data = m_dr02->read(); break;
	case  3: data = m_dr03->read(); break;
	case  4: data = m_dr04->read(); break;
	case  5: data = m_dr05->read(); break;
	case  6: data = m_dr06->read(); break;
	case  7: data = m_dr07->read(); break;
	case  8: data = m_dr08->read(); break;
	case  9: data = m_dr09->read(); break;
	case 10: data = m_dr10->read(); break;
	case 11: data = m_dr11->read(); break;
	case 12: data = m_dr12->read(); break;
	case 13: data = m_dr13->read(); break;
	case 14: data = m_dr14->read(); break;
	case 15: data = m_dr15->read(); break;
	case 16: data = m_dr16->read(); break;
	case 17: data = m_dr17->read(); break;
	case 18: data = m_dr18->read(); break;
	case 19: data = m_dr19->read(); break;
	case 20: data = m_dr20->read(); break;
	case 21: data = m_dr21->read(); break;
	case 22: data = m_dr22->read(); break;
	case 23: data = m_dr23->read(); break;
	}

	int sense = m_cnt & 0x03;

	return BIT(data, sense);
}
