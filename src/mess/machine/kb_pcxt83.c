/**********************************************************************

    IBM Model F PC/XT 83-key keyboard emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

/*

	Part No		Layout
	-------------------
	1501100		US
	1501102		Germany
	1501105		UK

*/

#include "kb_pcxt83.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I8048_TAG       "i8048"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type PC_KBD_IBM_PC_XT_83 = &device_creator<ibm_pc_xt_83_keyboard_device>;


//-------------------------------------------------
//  ROM( ibm_pc_xt_83_keyboard )
//-------------------------------------------------

ROM_START( ibm_pc_xt_83_keyboard )
	ROM_REGION( 0x400, I8048_TAG, 0 )
	/*
	Keyboard Part No. 1501105

	MOI 74 01
	PN 4584751
	GX 344231

	i 4429745
	ZO P 379297
	8143 P
	(C) INTEL 76
	*/
	ROM_LOAD( "4584751.m1", 0x000, 0x400, CRC(c59aa9d1) SHA1(4f5b2a075c68f6493310ec1e2a24271ceea330df) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *ibm_pc_xt_83_keyboard_device::device_rom_region() const
{
	return ROM_NAME( ibm_pc_xt_83_keyboard );
}


//-------------------------------------------------
//  ADDRESS_MAP( kb_io )
//-------------------------------------------------

static ADDRESS_MAP_START( ibm_pc_xt_83_keyboard_io, AS_IO, 8, ibm_pc_xt_83_keyboard_device )
	AM_RANGE(MCS48_PORT_BUS, MCS48_PORT_BUS) AM_READNOP AM_WRITE(bus_w)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_WRITE(p1_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_WRITE(p2_w)
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(t0_r)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(t1_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( ibm_pc_xt_83_keyboard )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( ibm_pc_xt_83_keyboard )
	MCFG_CPU_ADD(I8048_TAG, I8048, MCS48_LC_CLOCK(IND_U(47), CAP_P(20.7)))
	MCFG_CPU_IO_MAP(ibm_pc_xt_83_keyboard_io)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor ibm_pc_xt_83_keyboard_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ibm_pc_xt_83_keyboard );
}


//-------------------------------------------------
//  INPUT_PORTS( ibm_pc_xt_83_keyboard )
//-------------------------------------------------

INPUT_PORTS_START( ibm_pc_xt_83_keyboard )
	PORT_START("MD00")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) //4e
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) //51
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) //4d
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) //4a
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) //49
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) //46

	PORT_START("MD01")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) //53
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) //50
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) //4f
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) //4c
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) //4b
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) //47
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) //48
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) //45

	PORT_START("MD02")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) //52
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) //37
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) //36
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) //29
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) //1c
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) //0e

	PORT_START("MD03")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) //3a
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) //35
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) //28
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) //1b
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) //1a
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) //0d

	PORT_START("MD04")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) //34
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) //27
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) //26
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) //19
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) //0c
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) //0b

	PORT_START("MD05")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) //32
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) //33
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) //25
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) //24
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) //18
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) //17
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) //0a
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) //09

	PORT_START("MD06")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) //39
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) //31
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) //30
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) //23
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) //16
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) //15
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) //08
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) //07

	PORT_START("MD07")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) //2e
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) //2f
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) //22
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) //21
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) //14
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) //13
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) //06
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) //05

	PORT_START("MD08")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) //2d
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) //2c
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) //20
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) //1f
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) //12
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) //11
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) //04
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) //03

	PORT_START("MD09")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) //2b
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) //2a
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) //1d
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) //1e
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) //10
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) //0f
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) //02
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) //01

	PORT_START("MD10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) //38
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) //44
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) //42
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) //40
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) //3e
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) //3c

	PORT_START("MD11")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) //43
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) //41
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) //3f
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) //3d
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) //76
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) //3b
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor ibm_pc_xt_83_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ibm_pc_xt_83_keyboard );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ibm_pc_xt_83_keyboard_device - constructor
//-------------------------------------------------

ibm_pc_xt_83_keyboard_device::ibm_pc_xt_83_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PC_KBD_IBM_PC_XT_83, "IBM PC/XT Keyboard", tag, owner, clock, "kb_pcxt83", __FILE__),
	  device_pc_kbd_interface(mconfig, *this),
	  m_maincpu(*this, I8048_TAG),
	  m_md00(*this, "MD00"),
	  m_md01(*this, "MD01"),
	  m_md02(*this, "MD02"),
	  m_md03(*this, "MD03"),
	  m_md04(*this, "MD04"),
	  m_md05(*this, "MD05"),
	  m_md06(*this, "MD06"),
	  m_md07(*this, "MD07"),
	  m_md08(*this, "MD08"),
	  m_md09(*this, "MD09"),
	  m_md10(*this, "MD10"),
	  m_md11(*this, "MD11"),
	  m_bus(0xff),
	  m_p1(0xff),
	  m_p2(0xff),
	  m_sense(0),
	  m_q(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ibm_pc_xt_83_keyboard_device::device_start()
{
	set_pc_kbdc_device();
	
	// state saving
	save_item(NAME(m_bus));
	save_item(NAME(m_p1));
	save_item(NAME(m_p2));
	save_item(NAME(m_sense));
	save_item(NAME(m_q));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ibm_pc_xt_83_keyboard_device::device_reset()
{
}


//-------------------------------------------------
//  clock_write -
//-------------------------------------------------

WRITE_LINE_MEMBER( ibm_pc_xt_83_keyboard_device::clock_write )
{
}


//-------------------------------------------------
//  data_write -
//-------------------------------------------------

WRITE_LINE_MEMBER( ibm_pc_xt_83_keyboard_device::data_write )
{
	m_maincpu->set_input_line(MCS48_INPUT_IRQ, state ? CLEAR_LINE : ASSERT_LINE);
}


//-------------------------------------------------
//  bus_w -
//-------------------------------------------------

WRITE8_MEMBER( ibm_pc_xt_83_keyboard_device::bus_w )
{
	/*
	
	    bit     description
	
	    0       
	    1       
	    2       
	    3       
	    4       
	    5       DATAOUT
	    6       -REQOUT
	    7       Z1 RESET
	
	*/

	m_pc_kbdc->data_write_from_kb(BIT(data, 5));
	m_pc_kbdc->clock_write_from_kb(BIT(data, 6));

	if (!BIT(m_bus, 7) && BIT(data, 7))
	{
		UINT8 data = 0xff;

		if (BIT(m_p1, 0)) data &= m_md00->read();
		if (BIT(m_p1, 1)) data &= m_md01->read();
		if (BIT(m_p1, 2)) data &= m_md02->read();
		if (BIT(m_p1, 3)) data &= m_md03->read();
		if (BIT(m_p1, 4)) data &= m_md04->read();
		if (BIT(m_p1, 5)) data &= m_md05->read();
		if (BIT(m_p1, 6)) data &= m_md06->read();
		if (BIT(m_p1, 7)) data &= m_md07->read();
		if (BIT(m_p2, 4)) data &= m_md08->read();
		if (BIT(m_p2, 5)) data &= m_md09->read();
		if (BIT(m_p2, 6)) data &= m_md10->read();
		if (BIT(m_p2, 7)) data &= m_md11->read();
		
		m_q = BIT(data, m_sense);
	}

	m_bus = data;
}


//-------------------------------------------------
//  p1_w -
//-------------------------------------------------

WRITE8_MEMBER( ibm_pc_xt_83_keyboard_device::p1_w )
{
	/*
	
	    bit     description
	
	    0       MD00
	    1       MD01
	    2       MD02
	    3       MD03
	    4       MD04
	    5       MD05
	    6       MD06
	    7       MD07
	
	*/

	m_p1 = data;
}


//-------------------------------------------------
//  p2_w -
//-------------------------------------------------

WRITE8_MEMBER( ibm_pc_xt_83_keyboard_device::p2_w )
{
	/*
	
	    bit     description
	
	    0       SELECT 2
	    1       SELECT 1
	    2       SELECT 0
	    3       SA CLOSED
	    4       MD08
	    5       MD09
	    6       MD10
	    7       MD11
	
	*/

	if (!BIT(m_p2, 3) && BIT(data, 3))
	{
		m_sense = data & 0x07;
	}

	m_p2 = data;
}


//-------------------------------------------------
//  t0_r -
//-------------------------------------------------

READ8_MEMBER( ibm_pc_xt_83_keyboard_device::t0_r )
{
	return clock_signal();
}


//-------------------------------------------------
//  t1_r -
//-------------------------------------------------

READ8_MEMBER( ibm_pc_xt_83_keyboard_device::t1_r )
{
	return BIT(m_p2, 3) && m_q;
}
