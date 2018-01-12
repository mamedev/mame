// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Video Computer System keypad emulation

**********************************************************************/

#include "emu.h"
#include "keypad.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(A7800_KEYPAD, a7800_keypad_device, "a7800_keypad", "Atari / CBM Keypad")


static INPUT_PORTS_START( a7800_keypad )
	PORT_START("KEYPAD")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("keypad 1") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("keypad 2") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("keypad 3") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("keypad 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("keypad 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("keypad 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("keypad 7") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("keypad 8") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("keypad 9") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("keypad *") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("keypad 0") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("keypad #") PORT_CODE(KEYCODE_ENTER_PAD)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor a7800_keypad_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( a7800_keypad );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a7800_keypad_device - constructor
//-------------------------------------------------

a7800_keypad_device::a7800_keypad_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, A7800_KEYPAD, tag, owner, clock),
	device_a7800_control_port_interface(mconfig, *this),
	m_keypad(*this, "KEYPAD"),
	m_column(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a7800_keypad_device::device_start()
{
	m_column = 0;
	save_item(NAME(m_column));
}


//-------------------------------------------------
//  a7800_joy_r - joystick read
//-------------------------------------------------

uint8_t a7800_keypad_device::a7800_joy_r()
{
	for ( int i = 0; i < 4; i++ )
	{
		if ( ! ( ( m_column >> i ) & 0x01 ) )
		{
			if ( ( m_keypad->read() >> 3*i ) & 0x04 )
			{
				return 0xff;
			}
			else
			{
				return 0;
			}
		}
	}
	return 0xff;
}

//-------------------------------------------------
//  a7800_joy_w - joystick write
//-------------------------------------------------

void a7800_keypad_device::a7800_joy_w( uint8_t data )
{
	// TODO: a real keypad controller takes ~400us to respond to a a row change.
	// the column setting should really only be updated after the delay.
	m_column = data & 0x0F;
}

uint8_t a7800_keypad_device::a7800_pot_x_r()
{
	for ( int i = 0; i < 4; i++ )
	{
		if ( ! ( ( m_column >> i ) & 0x01 ) )
		{
			if ( ( m_keypad->read() >> 3*i ) & 0x01 )
			{
				return 0;
			}
			else
			{
				return 0xff;
			}
		}
	}
	return 0;
}

uint8_t a7800_keypad_device::a7800_pot_y_r()
{
	for ( int i = 0; i < 4; i++ )
	{
		if ( ! ( ( m_column >> i ) & 0x01 ) )
		{
			if ( ( m_keypad->read() >> 3*i ) & 0x02 )
			{
				return 0;
			}
			else
			{
				return 0xff;
			}
		}
	}
	return 0;
}
