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

DEFINE_DEVICE_TYPE(VCS_KEYPAD, vcs_keypad_device, "vcs_keypad", "Atari / CBM Keypad")


static INPUT_PORTS_START( vcs_keypad )
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

ioport_constructor vcs_keypad_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( vcs_keypad );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vcs_keypad_device - constructor
//-------------------------------------------------

vcs_keypad_device::vcs_keypad_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VCS_KEYPAD, tag, owner, clock),
	device_vcs_control_port_interface(mconfig, *this),
	m_keypad(*this, "KEYPAD"),
	m_row(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vcs_keypad_device::device_start()
{
	save_item(NAME(m_row));
}


//-------------------------------------------------
//  vcs_joy_w - joystick write
//-------------------------------------------------

void vcs_keypad_device::vcs_joy_w(uint8_t data)
{
	m_row = data & 0x0f;
}

uint8_t vcs_keypad_device::read_keys(uint8_t col)
{
	uint8_t val = 0;

	for (int row = 0; row < 4; row++)
	{
		if (!BIT(m_row, row))
		{
			if (!BIT(m_keypad->read(), 3*row + col))
			{
				val = 0xff;
			}
			break;
		}
	}

	return val;
}
