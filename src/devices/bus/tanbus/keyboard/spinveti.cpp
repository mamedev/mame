// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    ETI Space Invasion Key Unit

*********************************************************************/

#include "emu.h"
#include "spinveti.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MICROTAN_KBD_SPINVETI, microtan_kbd_spinveti, "microtan_kbd_spinveti", "ETI Space Invasion Key Unit")


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( spinveti )
	PORT_START("PB")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Start") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Left")  PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Fire")  PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Hold")  PORT_CODE(KEYCODE_H)

	PORT_START("SW")
	PORT_CONFNAME(0x60, 0x20, "Difficulty")
	PORT_CONFSETTING(0x00, "Easy")
	PORT_CONFSETTING(0x20, "Normal")
	PORT_CONFSETTING(0x40, "Hard")
	PORT_CONFSETTING(0x60, "Hardest")

	PORT_START("RS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Reset") PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(microtan_kbd_spinveti::trigger_reset), 0)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(microtan_kbd_spinveti::trigger_reset)
{
	m_slot->reset_w(newval ? CLEAR_LINE : ASSERT_LINE);
}

ioport_constructor microtan_kbd_spinveti::device_input_ports() const
{
	return INPUT_PORTS_NAME( spinveti );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  microtan_kbd_spinveti - constructor
//-------------------------------------------------

microtan_kbd_spinveti::microtan_kbd_spinveti(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MICROTAN_KBD_SPINVETI, tag, owner, clock)
	, device_microtan_kbd_interface(mconfig, *this)
	, m_pb(*this, "PB")
	, m_sw(*this, "SW")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void microtan_kbd_spinveti::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t microtan_kbd_spinveti::read()
{
	return m_pb->read() | m_sw->read();
}


void microtan_kbd_spinveti::write(uint8_t data)
{
	// TODO: sound effects (heartbeat, fire, saucer, and explosion)
	// bit 1 Heartbeat (active high)
	// bit 2 Gun firing (active low)
	// bit 3 Flying saucer (active high)
	// bit 4 Explosion (active high)
}
