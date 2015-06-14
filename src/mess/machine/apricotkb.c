// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Apricot keyboard emulation

*********************************************************************/

#include "apricotkb.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define UPD7507C_TAG    "ic2"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type APRICOT_KEYBOARD = &device_creator<apricot_keyboard_device>;


//-------------------------------------------------
//  ROM( apricot_keyboard )
//-------------------------------------------------

ROM_START( apricot_keyboard )
	ROM_REGION( 0x800, UPD7507C_TAG, 0 )
	ROM_LOAD( "keyboard controller upd7507c.ic2", 0x000, 0x800, NO_DUMP )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *apricot_keyboard_device::device_rom_region() const
{
	return ROM_NAME( apricot_keyboard );
}


//-------------------------------------------------
//  ADDRESS_MAP( kb_io )
//-------------------------------------------------

#ifdef UPD7507_EMULATED
static ADDRESS_MAP_START( apricot_keyboard_io, AS_IO, 8, apricot_keyboard_device )
	AM_RANGE(0x00, 0x00) AM_READ(kb_lo_r)
	AM_RANGE(0x01, 0x01) AM_READ(kb_hi_r)
	AM_RANGE(0x03, 0x03) AM_WRITE(kb_p3_w)
	AM_RANGE(0x04, 0x04) AM_WRITE(kb_y0_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(kb_y4_w)
	AM_RANGE(0x06, 0x06) AM_READWRITE(kb_p6_r, kb_yc_w)
	AM_RANGE(0x07, 0x07) AM_WRITE(kb_y8_w)
ADDRESS_MAP_END
#endif


//-------------------------------------------------
//  MACHINE_DRIVER( apricot_keyboard )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( apricot_keyboard )
#ifdef UPD7507_EMULATED
	MCFG_CPU_ADD(UPD7507C_TAG, UPD7507, XTAL_32_768kHz)
	MCFG_CPU_IO_MAP(apricot_keyboard_io)
#endif
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor apricot_keyboard_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( apricot_keyboard );
}


//-------------------------------------------------
//  INPUT_PORTS( apricot_keyboard )
//-------------------------------------------------

INPUT_PORTS_START( apricot_keyboard )
	PORT_START("Y0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y4")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y5")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y6")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y8")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y9")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("YA")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("YB")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("YC")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("MODIFIERS")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor apricot_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( apricot_keyboard );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  apricot_keyboard_device - constructor
//-------------------------------------------------

apricot_keyboard_device::apricot_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, APRICOT_KEYBOARD, "Apricot Keyboard", tag, owner, clock, "aprikb", __FILE__),
	m_write_txd(*this),
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
	m_modifiers(*this, "MODIFIERS")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void apricot_keyboard_device::device_start()
{
	// resolve callbacks
	m_write_txd.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void apricot_keyboard_device::device_reset()
{
}


//-------------------------------------------------
//  read_keyboard -
//-------------------------------------------------

UINT8 apricot_keyboard_device::read_keyboard()
{
	UINT8 data = 0xff;

	if (!BIT(m_kb_y, 0)) data &= m_y0->read();
	if (!BIT(m_kb_y, 1)) data &= m_y1->read();
	if (!BIT(m_kb_y, 2)) data &= m_y2->read();
	if (!BIT(m_kb_y, 3)) data &= m_y3->read();
	if (!BIT(m_kb_y, 4)) data &= m_y4->read();
	if (!BIT(m_kb_y, 5)) data &= m_y5->read();
	if (!BIT(m_kb_y, 6)) data &= m_y6->read();
	if (!BIT(m_kb_y, 7)) data &= m_y7->read();
	if (!BIT(m_kb_y, 8)) data &= m_y8->read();
	if (!BIT(m_kb_y, 9)) data &= m_y9->read();
	if (!BIT(m_kb_y, 10)) data &= m_ya->read();
	if (!BIT(m_kb_y, 11)) data &= m_yb->read();
	if (!BIT(m_kb_y, 12)) data &= m_yc->read();

	return data;
}


//-------------------------------------------------
//  kb_lo_r -
//-------------------------------------------------

READ8_MEMBER( apricot_keyboard_device::kb_lo_r )
{
	return read_keyboard() & 0x0f;
}


//-------------------------------------------------
//  kb_hi_r -
//-------------------------------------------------

READ8_MEMBER( apricot_keyboard_device::kb_hi_r )
{
	return read_keyboard() >> 4;
}


//-------------------------------------------------
//  kb_p6_r -
//-------------------------------------------------

READ8_MEMBER( apricot_keyboard_device::kb_p6_r )
{
	/*

	    bit     description

	    P60
	    P61     SHIFT
	    P62     CONTROL

	*/

	UINT8 modifiers = m_modifiers->read();

	return modifiers << 1;
}


//-------------------------------------------------
//  kb_p3_w -
//-------------------------------------------------

WRITE8_MEMBER( apricot_keyboard_device::kb_p3_w )
{
	// bit 1 controls the IR LEDs
}


//-------------------------------------------------
//  kb_y0_w -
//-------------------------------------------------

WRITE8_MEMBER( apricot_keyboard_device::kb_y0_w )
{
	m_kb_y = (m_kb_y & 0xfff0) | (data & 0x0f);
}


//-------------------------------------------------
//  kb_y4_w -
//-------------------------------------------------

WRITE8_MEMBER( apricot_keyboard_device::kb_y4_w )
{
	m_kb_y = (m_kb_y & 0xff0f) | ((data & 0x0f) << 4);
}


//-------------------------------------------------
//  kb_y8_w -
//-------------------------------------------------

WRITE8_MEMBER( apricot_keyboard_device::kb_y8_w )
{
	m_kb_y = (m_kb_y & 0xf0ff) | ((data & 0x0f) << 8);
}


//-------------------------------------------------
//  kb_yc_w -
//-------------------------------------------------

WRITE8_MEMBER( apricot_keyboard_device::kb_yc_w )
{
	m_kb_y = (m_kb_y & 0xf0ff) | ((data & 0x0f) << 12);
}
