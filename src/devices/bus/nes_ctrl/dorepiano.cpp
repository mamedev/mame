// license:BSD-3-Clause
// copyright-holders:kmg, Fabio Priuli
/**********************************************************************

    Nintendo Family Computer Konami 'Doremikko' RJ250 Piano Keyboard

**********************************************************************/

#include "emu.h"
#include "dorepiano.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NES_DOREPIANO, nes_dorepiano_device, "nes_dorepiano", "Konami 'Doremikko' Piano Keyboard")


static INPUT_PORTS_START( nes_dorepiano )
	PORT_START("PIANO.0")
	PORT_BIT( 0x3f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("PIANO.1")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 0 F")  PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 0 F#") PORT_CODE(KEYCODE_S)

	PORT_START("PIANO.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 0 G")  PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 0 G#") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 0 A")  PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 0 A#") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 0 B")  PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 0 C")  PORT_CODE(KEYCODE_B)

	PORT_START("PIANO.3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 0 C#") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 0 D")  PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 0 D#") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 0 E")  PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 1 F")  PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 1 F#") PORT_CODE(KEYCODE_L)

	PORT_START("PIANO.4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 1 G")  PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 1 G#") PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 1 A")  PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 1 A#") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 1 B")  PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 1 C")  PORT_CODE(KEYCODE_W)

	PORT_START("PIANO.5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 1 C#") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 1 D")  PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 1 D#") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 1 E")  PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 2 F")  PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 2 F#") PORT_CODE(KEYCODE_6)

	PORT_START("PIANO.6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 2 G")  PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 2 G#") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 2 A")  PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 2 A#") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 2 B")  PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 2 C")  PORT_CODE(KEYCODE_O)

	PORT_START("PIANO.7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 2 C#")  PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 2 D")   PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 2 D#")  PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave 2 E")   PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor nes_dorepiano_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nes_dorepiano );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nes_dorepiano_device - constructor
//-------------------------------------------------

nes_dorepiano_device::nes_dorepiano_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NES_DOREPIANO, tag, owner, clock)
	, device_nes_control_port_interface(mconfig, *this)
	, m_port(*this, "PIANO.%u", 0)
	, m_cur_port(0)
	, m_latch(0)
	, m_mask(0)
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void nes_dorepiano_device::device_start()
{
	save_item(NAME(m_cur_port));
	save_item(NAME(m_latch));
	save_item(NAME(m_mask));

	m_mask = 0x0f;
}


//-------------------------------------------------
//  read
//-------------------------------------------------

u8 nes_dorepiano_device::read_exp(offs_t offset)
{
	u8 ret = 0;

	if (offset == 1)    // $4017
	{
		ret = m_port[m_cur_port]->read() & m_mask;
		m_mask = ~m_mask;
		ret >>= m_mask & 0x04;
		ret <<= 1;
	}

	return ret;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void nes_dorepiano_device::write(u8 data)
{
	if (BIT(data, 1))
	{
		m_cur_port = 0;
		m_mask = 0x0f;
	}
	else if (BIT(data, 0) && !BIT(m_latch, 0))
	{
		m_cur_port = (m_cur_port + 1) & 0x07;
		m_mask = 0x0f;
	}

	m_latch = data;
}
