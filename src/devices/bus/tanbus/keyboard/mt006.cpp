// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Microtan Keypad (MT006)

    http://www.microtan.ukpc.net/pageProducts.html#KEYBOARDS

*********************************************************************/

#include "emu.h"
#include "mt006.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MICROTAN_KBD_MT006, microtan_kbd_mt006, "microtan_kbd_mt006", "Microtan Keypad (MT006)")


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( mt006 )
	PORT_START("KPAD0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("0")      PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("4")      PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("8 P")    PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C M")    PORT_CODE(KEYCODE_C)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("SHIFT")  PORT_CODE(KEYCODE_LSHIFT)

	PORT_START("KPAD1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("1")      PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("5 O")    PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("9 ESC")  PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("D G")    PORT_CODE(KEYCODE_D)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("LF DEL") PORT_CODE(KEYCODE_DEL_PAD)

	PORT_START("KPAD2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("2")      PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("6 C")    PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("A")      PORT_CODE(KEYCODE_A)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("E S")    PORT_CODE(KEYCODE_E)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CR SP")  PORT_CODE(KEYCODE_ENTER_PAD)

	PORT_START("KPAD3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("3 '")    PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("7 R")    PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("B L")    PORT_CODE(KEYCODE_B)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F N")    PORT_CODE(KEYCODE_F)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RESET") PORT_CODE(KEYCODE_F12) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(microtan_kbd_mt006::trigger_reset), 0)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(microtan_kbd_mt006::trigger_reset)
{
	m_slot->reset_w(newval ? CLEAR_LINE : ASSERT_LINE);
}

ioport_constructor microtan_kbd_mt006::device_input_ports() const
{
	return INPUT_PORTS_NAME( mt006 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  microtan_kbd_mt006 - constructor
//-------------------------------------------------

microtan_kbd_mt006::microtan_kbd_mt006(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MICROTAN_KBD_MT006, tag, owner, clock)
	, device_microtan_kbd_interface(mconfig, *this)
	, m_keypad(*this, "KPAD%u", 0U)
	, m_column(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void microtan_kbd_mt006::device_start()
{
	save_item(NAME(m_column));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t microtan_kbd_mt006::read()
{
	uint8_t data = 0x00;

	for (int i = 0; i < 4; i++)
		if (BIT(m_column, i))
			data |= m_keypad[i]->read();

	return data;
}


void microtan_kbd_mt006::write(uint8_t data)
{
	m_column = data & 0x0f;

	m_slot->strobe_w(BIT(data, 4));
}
