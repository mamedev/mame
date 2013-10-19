// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Telenova Compis keyboard emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "compiskb.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I8748_TAG       "i8748"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type COMPIS_KEYBOARD = &device_creator<compis_keyboard_device>;


//-------------------------------------------------
//  ROM( compis_keyboard )
//-------------------------------------------------

ROM_START( compis_keyboard )
	ROM_REGION( 0x800, I8748_TAG, 0 )
	ROM_LOAD( "cmpkey13.u1", 0x000, 0x800, CRC(3f87d138) SHA1(c04e2d325b9c04818bc7c47c3bf32b13862b11ec) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *compis_keyboard_device::device_rom_region() const
{
	return ROM_NAME( compis_keyboard );
}


//-------------------------------------------------
//  ADDRESS_MAP( compis_keyboard_io )
//-------------------------------------------------

static ADDRESS_MAP_START( compis_keyboard_io, AS_IO, 8, compis_keyboard_device )
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_READ(p1_r)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_READ(p2_r)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_NOP
	AM_RANGE(MCS48_PORT_BUS, MCS48_PORT_BUS) AM_WRITE(bus_w)
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( compis_keyboard )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( compis_keyboard )
	MCFG_CPU_ADD(I8748_TAG, I8748, XTAL_2MHz)
	MCFG_CPU_IO_MAP(compis_keyboard_io)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor compis_keyboard_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( compis_keyboard );
}


//-------------------------------------------------
//  INPUT_PORTS( compis_keyboard )
//-------------------------------------------------

INPUT_PORTS_START( compis_keyboard )
	PORT_START("Y0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y5")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y6")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y7")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y8")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y9")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SPECIAL")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) // CAPS LOCK
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) // SHIFT
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) // CTRL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) // SUPER
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor compis_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( compis_keyboard );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  compis_keyboard_device - constructor
//-------------------------------------------------

compis_keyboard_device::compis_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, COMPIS_KEYBOARD, "Compis Keyboard", tag, owner, clock, "compiskb", __FILE__),
		m_write_irq(*this),
		m_maincpu(*this, I8748_TAG),
		m_speaker(*this, SPEAKER_TAG),
		m_y0(*this, "Y0"),
		m_y1(*this, "Y1"),
		m_y2(*this, "Y2"),
		m_y3(*this, "Y3"),
		m_y4(*this, "Y4"),
		m_y5(*this, "Y5"),
		m_y6(*this, "Y6"),
		m_y7(*this, "Y7"),
		m_y8(*this, "Y8"),
		m_y9(*this, "Y9"),
		m_special(*this, "SPECIAL"),
		m_so(1),
		m_keylatch(0x3ff)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void compis_keyboard_device::device_start()
{
	// resolve callbacks
	m_write_irq.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void compis_keyboard_device::device_reset()
{
}


//-------------------------------------------------
//  si_w - serial input write
//-------------------------------------------------

WRITE_LINE_MEMBER( compis_keyboard_device::si_w )
{
	m_maincpu->set_input_line(MCS48_INPUT_IRQ, state ? CLEAR_LINE : ASSERT_LINE);
}


//-------------------------------------------------
//  so_r - serial output read
//-------------------------------------------------

READ_LINE_MEMBER( compis_keyboard_device::so_r )
{
	return m_so;
}


//-------------------------------------------------
//  p1_r -
//-------------------------------------------------

WRITE8_MEMBER( compis_keyboard_device::bus_w ) 
{
	/*
	
	    bit     description
	
	    0       keyboard row bit 0
	    1       keyboard row bit 1
	    2       keyboard row bit 2
	    3       keyboard row bit 3
	    4       
	    5       speaker
	    6       CAPS LED
	    7       data out
	
	*/

	// keyboard column
	m_keylatch = (data & 0x0f);

	// speaker
	m_speaker->level_w(BIT(data, 5));

	// LEDs
	output_set_led_value(LED_CAPS, BIT(data, 6));

	// serial data out
	m_so = BIT(data, 7);
}


//-------------------------------------------------
//  bus_w -
//-------------------------------------------------

READ8_MEMBER( compis_keyboard_device::p1_r )
{
	UINT8 data = 0xff;

	switch (m_keylatch)
	{
	case 0: data &= m_y0->read(); break;
	case 1: data &= m_y1->read(); break;
	case 2: data &= m_y2->read(); break;
	case 3: data &= m_y3->read(); break;
	case 4: data &= m_y4->read(); break;
	case 5: data &= m_y5->read(); break;
	case 6: data &= m_y6->read(); break;
	case 7: data &= m_y7->read(); break;
	case 8: data &= m_y8->read(); break;
	case 9: data &= m_y9->read(); break;
	}

	return data;
}


//-------------------------------------------------
//  p2_r -
//-------------------------------------------------

READ8_MEMBER( compis_keyboard_device::p2_r )
{
	UINT8 data = 0xff;

	switch (m_keylatch)
	{
	case 0: data &= m_y0->read() >> 8; break;
	case 1: data &= m_y1->read() >> 8; break;
	case 2: data &= m_y2->read() >> 8; break;
	case 3: data &= m_y3->read() >> 8; break;
	case 4: data &= m_y4->read() >> 8; break;
	case 5: data &= m_y5->read() >> 8; break;
	case 6: data &= m_y6->read() >> 8; break;
	case 7: data &= m_y7->read() >> 8; break;
	case 8: data &= m_y8->read() >> 8; break;
	case 9: data &= m_y9->read() >> 8; break;
	}

	data &= m_special->read();

	return data;
}
