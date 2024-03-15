// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Atari CX85 Numeric Keypad

    Normal Atari 400/800 usage has this connect to controller port 2.

**********************************************************************/

#include "emu.h"
#include "cx85.h"

#include "machine/rescap.h"


//**************************************************************************
//  DEVICE TYPE DEFINITION
//**************************************************************************

DEFINE_DEVICE_TYPE(ATARI_CX85, atari_cx85_device, "atari_cx85", "Atari CX85 Numeric Keypad")


//**************************************************************************
//  INPUT PORTS 
//**************************************************************************

static INPUT_PORTS_START(atari_cx85)
	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad -") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad + Enter") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad .") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad F4 (Yes)") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad F2 (No)") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad F3 (Delete)") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad F1 (Escape)") PORT_CODE(KEYCODE_NUMLOCK)
INPUT_PORTS_END


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  atari_cx85_device - constructor
//-------------------------------------------------

atari_cx85_device::atari_cx85_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ATARI_CX85, tag, owner, clock)
	, device_vcs_control_port_interface(mconfig, *this)
	, m_encoder(*this, "encoder")
{
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void atari_cx85_device::device_add_mconfig(machine_config &config)
{
	MM74C923(config, m_encoder, 0); // MCM74C923N
	m_encoder->set_cap_osc(CAP_U(.1));
	m_encoder->set_cap_debounce(CAP_U(.47));
	m_encoder->data_tri_callback().set_constant(0);
	m_encoder->x1_rd_callback().set_ioport("X1");
	m_encoder->x2_rd_callback().set_ioport("X2");
	m_encoder->x3_rd_callback().set_ioport("X3");
	m_encoder->x4_rd_callback().set_ioport("X4");
	m_encoder->da_wr_callback().set(FUNC(atari_cx85_device::trigger_w)).invert();
}

//-------------------------------------------------
//  device_input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor atari_cx85_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(atari_cx85);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void atari_cx85_device::device_start()
{
}

//-------------------------------------------------
//  vcs_joy_r - read digital inputs
//-------------------------------------------------

u8 atari_cx85_device::vcs_joy_r()
{
	// 74C923 outputs are buffered through 4049B
	return (~m_encoder->read() & 0x0f) | (m_encoder->da_r() ? 0 : 0x20);
}

//-------------------------------------------------
//  vcs_pot_x_r - sample B pot
//-------------------------------------------------

u8 atari_cx85_device::vcs_pot_x_r()
{
	// Schematics suggests this should also be inverted through 4049B, but drivers seem to work the opposite way
	return BIT(m_encoder->read(), 4) ? 0xff : 0;
}
