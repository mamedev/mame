// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Victor 9000 keyboard emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "victor9kb.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I8021_TAG       "z3"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type VICTOR9K_KEYBOARD = &device_creator<victor9k_keyboard_device>;


//-------------------------------------------------
//  ROM( victor9k_keyboard )
//-------------------------------------------------

ROM_START( victor9k_keyboard )
	ROM_REGION( 0x400, I8021_TAG, 0)
	ROM_LOAD( "20-8021-139.z3", 0x000, 0x400, CRC(0fe9d53d) SHA1(61d92ba90f98f8978bbd9303c1ac3134cde8cdcb) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *victor9k_keyboard_device::device_rom_region() const
{
	return ROM_NAME( victor9k_keyboard );
}


//-------------------------------------------------
//  ADDRESS_MAP( kb_io )
//-------------------------------------------------

static ADDRESS_MAP_START( victor9k_keyboard_io, AS_IO, 8, victor9k_keyboard_device )
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_READWRITE(kb_p1_r, kb_p1_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_WRITE(kb_p2_w)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(kb_t1_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( victor9k_keyboard )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( victor9k_keyboard )
	MCFG_CPU_ADD(I8021_TAG, I8021, XTAL_3_579545MHz)
	MCFG_CPU_IO_MAP(victor9k_keyboard_io)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor victor9k_keyboard_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( victor9k_keyboard );
}


//-------------------------------------------------
//  INPUT_PORTS( victor9k_keyboard )
//-------------------------------------------------

INPUT_PORTS_START( victor9k_keyboard )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("YA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("YB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("YC")
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

ioport_constructor victor9k_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( victor9k_keyboard );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  victor9k_keyboard_device - constructor
//-------------------------------------------------

victor9k_keyboard_device::victor9k_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, VICTOR9K_KEYBOARD, "Victor 9000 Keyboard", tag, owner, clock, "victor9kb", __FILE__),
	m_maincpu(*this, I8021_TAG),
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
	m_ya(*this, "YA"),
	m_yb(*this, "YB"),
	m_yc(*this, "YC"),
	m_kbrdy_handler(*this),
	m_kbdata_handler(*this),
	m_y(0),
	m_kbrdy(1),
	m_kbdata(1),
	m_kback(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void victor9k_keyboard_device::device_start()
{
	// resolve callbacks
	m_kbrdy_handler.resolve_safe();
	m_kbdata_handler.resolve_safe();

	// state saving
	save_item(NAME(m_y));
	save_item(NAME(m_kbrdy));
	save_item(NAME(m_kbdata));
	save_item(NAME(m_kback));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void victor9k_keyboard_device::device_reset()
{
}


//-------------------------------------------------
//  kback_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( victor9k_keyboard_device::kback_w )
{
	//logerror("KBACK %u\n", state);
	m_kback = state;
}


//-------------------------------------------------
//  kb_p1_r -
//-------------------------------------------------

READ8_MEMBER( victor9k_keyboard_device::kb_p1_r )
{
	UINT8 data = 0xff;

	switch (m_y)
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
		case 0xa: data &= m_ya->read(); break;
		case 0xb: data &= m_yb->read(); break;
		case 0xc: data &= m_yc->read(); break;
	}

	return data;
}


//-------------------------------------------------
//  kb_p1_w -
//-------------------------------------------------

WRITE8_MEMBER( victor9k_keyboard_device::kb_p1_w )
{
	if ((data & 0xf0) == 0x20)
	{
		m_y = data & 0x0f;
	}

	//logerror("P1 %02x\n", data);
}


//-------------------------------------------------
//  kb_p2_w -
//-------------------------------------------------

WRITE8_MEMBER( victor9k_keyboard_device::kb_p2_w )
{
	/*

	    bit     description

	    P20     ?
	    P21     KBRDY
	    P22     ?
	    P23     KBDATA

	*/

	int kbrdy = BIT(data, 1);

	if (m_kbrdy != kbrdy)
	{
		m_kbrdy = kbrdy;
		m_kbrdy_handler(m_kbrdy);
	}

	int kbdata = BIT(data, 3);

	if (m_kbdata != kbdata)
	{
		m_kbdata = kbdata;
		m_kbdata_handler(m_kbdata);
	}

	//logerror("P2 %02x\n", data);
}


//-------------------------------------------------
//  kb_t1_r -
//-------------------------------------------------

READ8_MEMBER( victor9k_keyboard_device::kb_t1_r )
{
	return m_kback;
}
