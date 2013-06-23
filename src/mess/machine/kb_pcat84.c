/**********************************************************************

    IBM Model F PC/AT 84-key / 3270PC 122-key keyboard emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

/*

	Part No		Layout
	-------------------
	6450225 	UK 84-key
	6110344 	UK 122-key

*/

#include "kb_pcat84.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I8048_TAG       "i8048"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type PC_KBD_IBM_PC_AT_84 = &device_creator<ibm_pc_at_84_keyboard_device>;
const device_type PC_KBD_IBM_3270PC_122 = &device_creator<ibm_3270pc_122_keyboard_device>;


//-------------------------------------------------
//  ROM( ibm_pc_at_84_keyboard )
//-------------------------------------------------

ROM_START( ibm_pc_at_84_keyboard )
	ROM_REGION( 0x400, I8048_TAG, 0 )
	/*
	Keyboard Part No 6450225
	
	PH 1503099
	D 878154
	8441 D H
	*/
	ROM_LOAD( "1503099.bin", 0x000, 0x400, CRC(1e921f37) SHA1(5f722bdb3b57f5a532c02a5c3f78f30d785796f2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *ibm_pc_at_84_keyboard_device::device_rom_region() const
{
	return ROM_NAME( ibm_pc_at_84_keyboard );
}


//-------------------------------------------------
//  ROM( ibm_3270pc_122_keyboard )
//-------------------------------------------------

ROM_START( ibm_3270pc_122_keyboard )
	ROM_REGION( 0x400, I8048_TAG, 0 )
	/*
	Keyboard Part No 6110344
	
	PH 1385001
	D
	8512 D H
	*/
	ROM_LOAD( "1385001.bin", 0x000, 0x400, CRC(c19767e9) SHA1(a3701e4617383a4de0fd5e2e86c4b74beaf94a7b) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *ibm_3270pc_122_keyboard_device::device_rom_region() const
{
	return ROM_NAME( ibm_3270pc_122_keyboard );
}


//-------------------------------------------------
//  ADDRESS_MAP( kb_io )
//-------------------------------------------------

static ADDRESS_MAP_START( ibm_pc_at_84_keyboard_io, AS_IO, 8, ibm_pc_at_84_keyboard_device )
	AM_RANGE(MCS48_PORT_BUS, MCS48_PORT_BUS) AM_READNOP AM_WRITE(bus_w)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_READWRITE(p1_r, p1_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_READWRITE(p2_r, p2_w)
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(t0_r)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(t1_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( ibm_pc_at_84_keyboard )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( ibm_pc_at_84_keyboard )
	MCFG_CPU_ADD(I8048_TAG, I8048, 5364000)
	MCFG_CPU_IO_MAP(ibm_pc_at_84_keyboard_io)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor ibm_pc_at_84_keyboard_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ibm_pc_at_84_keyboard );
}


//-------------------------------------------------
//  INPUT_PORTS( ibm_pc_at_84_keyboard )
//-------------------------------------------------

INPUT_PORTS_START( ibm_pc_at_84_keyboard )
	PORT_START("DR00")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) 

	PORT_START("DR01")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) 

	PORT_START("DR02")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) 

	PORT_START("DR03")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) 

	PORT_START("DR04")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) 

	PORT_START("DR05")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) 

	PORT_START("DR06")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) 

	PORT_START("DR07")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) 

	PORT_START("DR08")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) 

	PORT_START("DR09")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) 

	PORT_START("DR10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) 

	PORT_START("DR11")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) 

	PORT_START("DR12")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) 

	PORT_START("DR13")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) 

	PORT_START("DR14")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) 

	PORT_START("DR15")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) 
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor ibm_pc_at_84_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ibm_pc_at_84_keyboard );
}


//-------------------------------------------------
//  INPUT_PORTS( ibm_3270pc_122_keyboard )
//-------------------------------------------------

INPUT_PORTS_START( ibm_3270pc_122_keyboard )
	PORT_INCLUDE(ibm_pc_at_84_keyboard)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor ibm_3270pc_122_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ibm_3270pc_122_keyboard );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ibm_pc_at_84_keyboard_device - constructor
//-------------------------------------------------

ibm_pc_at_84_keyboard_device::ibm_pc_at_84_keyboard_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
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
	  m_db(0),
	  m_cnt(0),
	  m_sense(0)
{
}

ibm_pc_at_84_keyboard_device::ibm_pc_at_84_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PC_KBD_IBM_PC_AT_84, "IBM PC/AT Keyboard", tag, owner, clock, "kb_pcat84", __FILE__),
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
	  m_db(0),
	  m_cnt(0),
	  m_sense(0)
{
}

ibm_3270pc_122_keyboard_device::ibm_3270pc_122_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ibm_pc_at_84_keyboard_device(mconfig, PC_KBD_IBM_3270PC_122, "IBM 3270PC Keyboard", tag, owner, clock, "kb_3270pc", __FILE__) { }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ibm_pc_at_84_keyboard_device::device_start()
{
	// state saving
	save_item(NAME(m_db));
	save_item(NAME(m_cnt));
	save_item(NAME(m_sense));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ibm_pc_at_84_keyboard_device::device_reset()
{
	m_maincpu->reset();
}


//-------------------------------------------------
//  bus_w -
//-------------------------------------------------

WRITE8_MEMBER( ibm_pc_at_84_keyboard_device::bus_w )
{
	/*
	
	    bit     description
	
	    0       SENSE 0
	    1       SENSE 1
	    2       SENSE 2
	    3       CNT 0
	    4       CNT 1
	    5       CNT 2
	    6       CNT 3
	    7       CNT G
	
	*/

	m_db = data;

	if (BIT(data, 7))
	{
		m_cnt = (data >> 3) & 0x0f;
	}
}


//-------------------------------------------------
//  p1_r -
//-------------------------------------------------

READ8_MEMBER( ibm_pc_at_84_keyboard_device::p1_r )
{
	/*
	
	    bit     description
	
	    0       
	    1       OUT
	    2       DSW1
	    3       DSW2
	    4       DSW3
	    5       DSW4
	    6       DSW5
	    7       DSW6
	
	*/

	UINT8 data = 0xfc;

	return data;
}


//-------------------------------------------------
//  p1_w -
//-------------------------------------------------

WRITE8_MEMBER( ibm_pc_at_84_keyboard_device::p1_w )
{
	/*
	
	    bit     description
	
	    0       SENSE G
	    1       T1
	    2       
	    3       
	    4       
	    5       
	    6       
	    7       
	
	*/

	if (BIT(data, 0))
	{
		m_sense = m_db & 0x07;
	}
}


//-------------------------------------------------
//  p2_r -
//-------------------------------------------------

READ8_MEMBER( ibm_pc_at_84_keyboard_device::p2_r )
{
	/*
	
	    bit     description
	
	    0       JUMPER1
	    1       JUMPER2
	    2       JUMPER3
	    3       JUMPER4
	    4       DSW7
	    5       DSW8
	    6       
	    7       
	
	*/

	return 0xff;
}


//-------------------------------------------------
//  p2_w -
//-------------------------------------------------

WRITE8_MEMBER( ibm_pc_at_84_keyboard_device::p2_w )
{
	/*
	
	    bit     description
	
	    0       SCROLL LED
	    1       NUM LED
	    2       CAPS LED
	    3       
	    4       
	    5       
	    6       CLOCK
	    7       DATA
	
	*/

	output_set_led_value(LED_SCROLL, !BIT(data, 0));
	output_set_led_value(LED_NUM, !BIT(data, 1));
	output_set_led_value(LED_CAPS, !BIT(data, 2));

	m_pc_kbdc->clock_write_from_kb(!BIT(data, 6));
	m_pc_kbdc->data_write_from_kb(!BIT(data, 7));
}


//-------------------------------------------------
//  t0_r -
//-------------------------------------------------

READ8_MEMBER( ibm_pc_at_84_keyboard_device::t0_r )
{
	return !data_signal();
}


//-------------------------------------------------
//  t1_r -
//-------------------------------------------------

READ8_MEMBER( ibm_pc_at_84_keyboard_device::t1_r )
{
	UINT8 data = 0xff;

	switch (m_cnt)
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
	}

	return BIT(data, m_sense);
}
