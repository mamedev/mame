// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    ADI keyboard for an unknown system

    Hardware:
    - UM8035-8
    - 5.760 XTAL

    TODO:
    - Verify keys once a matching system is found

    Notes:
    - Serial format: 1200 baud, 8 data bits, no parity, 1 stop bit
    - Printed on PCB: 8648 D-086-001-00 1-304-227
    - Always sends two bytes: Keycode + modifier status. Keycode is < 0x80,
      modifier status is 0x80 combined with 0x01 for Ctrl, 0x02 for Alt,
      0x04 for Shift, 0x08 for Caps Lock and 0x10 for key repeat.

***************************************************************************/

#include "emu.h"
#include "adi_unk_kbd.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ADI_UNK_KBD, adi_unk_kbd_device, "adi_unk_kbd", "ADI keyboard for unknown system")

adi_unk_kbd_device::adi_unk_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ADI_UNK_KBD, tag, owner, 0U),
	m_mcu(*this, "mcu"),
	m_keys(*this, "row_%x", 0U),
	m_shift(*this, "shift"),
	m_ctrl(*this, "ctrl"),
	m_capslock(*this, "capslock"),
	m_txd_cb(*this),
	m_key_row_p1(0xff),
	m_key_row_p2(0xff)
{
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void adi_unk_kbd_device::mem_map(address_map &map)
{
	map(0x000, 0xfff).rom().region("mcu", 0); // a10 and a11 might be unconnected
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( firmware )
	ROM_REGION(0x2000, "mcu", 0)
	ROM_LOAD("f-2764-176-01-0.u4", 0x0000, 0x2000, CRC(03ecbc63) SHA1(cea2fda37315582361b47b184287a3bf430b6692))
ROM_END

const tiny_rom_entry *adi_unk_kbd_device::device_rom_region() const
{
	return ROM_NAME( firmware );
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void adi_unk_kbd_device::device_add_mconfig(machine_config &config)
{
	I8035(config, m_mcu, 5.760_MHz_XTAL);
	m_mcu->set_addrmap(AS_PROGRAM, &adi_unk_kbd_device::mem_map);
	m_mcu->t0_in_cb().set(FUNC(adi_unk_kbd_device::t0_r));
	m_mcu->t1_in_cb().set(FUNC(adi_unk_kbd_device::t1_r));
	m_mcu->bus_in_cb().set(FUNC(adi_unk_kbd_device::bus_r));
	m_mcu->p1_out_cb().set(FUNC(adi_unk_kbd_device::p1_w));
	m_mcu->p2_in_cb().set(FUNC(adi_unk_kbd_device::p2_r));
	m_mcu->p2_out_cb().set(FUNC(adi_unk_kbd_device::p2_w));
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( keyboard )
	PORT_START("ctrl")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_NAME("Ctrl")

	PORT_START("alt")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(LALT)) PORT_NAME("Alt") PORT_WRITE_LINE_MEMBER(FUNC(adi_unk_kbd_device::alt_w))

	PORT_START("shift")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1) PORT_NAME("Shift")

	PORT_START("capslock")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_NAME("Caps Lock")

	PORT_START("row_0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_0 0x01 -> 0x17") PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_0 0x02 -> 0x2c") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_0 0x04 -> 0x38") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_0 0x08 -> 0x47") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_0 0x10 -> 0x15") PORT_CODE(KEYCODE_F19)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_0 0x20 -> 0x01") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_0 0x40 -> 0x42") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_0 0x80 -> 0x6b") PORT_CODE(KEYCODE_1_PAD)

	PORT_START("row_1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_1 0x01 -> 0x1d") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_1 0x02 -> 0x61") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_1 0x04 -> 0x30") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_1 0x08 -> 0x4a") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_1 0x10 -> 0x6a") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_1 0x20 -> 0x3e") PORT_CODE(KEYCODE_PGDN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_1 0x40 -> 0x58") PORT_CODE(KEYCODE_COMMA_PAD) // actually KEYCODE_STOP_PAD?
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_1 0x80 -> 0x41") PORT_CODE(KEYCODE_9_PAD)

	PORT_START("row_2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_2 0x01 -> 0x1c") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_2 0x02 -> 0x5f") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_2 0x04 -> 0x2f") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_2 0x08 -> 0x48") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_2 0x10 -> 0x16") PORT_CODE(KEYCODE_F20)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_2 0x20 -> 0x7a") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_2 0x40 -> 0x7b") PORT_CODE(KEYCODE_00_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_2 0x80 -> 0x7c") // ?

	PORT_START("row_3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_3 0x01 -> 0x1b") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_3 0x02 -> 0x63") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_3 0x04 -> 0x2e") PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_3 0x08 -> 0x59") // LOCK
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_3 0x10 -> 0x4b") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_3 0x20 -> 0x45") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_3 0x40 -> 0x4d") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_3 0x80 -> 0x66") // ?

	PORT_START("row_4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_4 0x01 -> 0x1a") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_4 0x02 -> 0x2b") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_4 0x04 -> 0x11") PORT_CODE(KEYCODE_F16)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_4 0x08 -> 0x10") PORT_CODE(KEYCODE_F15)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_4 0x10 -> 0x2a") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_4 0x20 -> 0x3d") PORT_CODE(KEYCODE_INSERT)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_4 0x40 -> 0x53") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_4 0x80 -> 0x69") PORT_CODE(KEYCODE_HOME)

	PORT_START("row_5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_5 0x01 -> 0x19") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_5 0x02 -> 0x5c") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_5 0x04 -> 0x13") PORT_CODE(KEYCODE_F17)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_5 0x08 -> 0x46") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_5 0x10 -> 0x14") PORT_CODE(KEYCODE_F18)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_5 0x20 -> 0x03") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_5 0x40 -> 0x56") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_5 0x80 -> 0x6d") PORT_CODE(KEYCODE_3_PAD)

	PORT_START("row_6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_6 0x01 -> 0x18") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_6 0x02 -> 0x5e") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_6 0x04 -> 0x67") // ?
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_6 0x08 -> 0x51") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_6 0x10 -> 0x73") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_6 0x20 -> 0x02") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_6 0x40 -> 0x55") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_6 0x80 -> 0x6c") PORT_CODE(KEYCODE_2_PAD)

	PORT_START("row_7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_7 0x01 -> 0x22") // ?
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_7 0x02 -> 0x65") PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_7 0x04 -> 0x3a") PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_7 0x08 -> 0x23") // ?
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_7 0x10 -> 0x25") PORT_CODE(KEYCODE_PAUSE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_7 0x20 -> 0x3b") PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_7 0x40 -> 0x4f") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_7 0x80 -> 0x27") PORT_CODE(KEYCODE_ESC)

	PORT_START("row_8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_8 0x01 -> 0x0a") PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_8 0x02 -> 0x06") // ?
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_8 0x04 -> 0x0d") PORT_CODE(KEYCODE_F12)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_8 0x08 -> 0x0e") PORT_CODE(KEYCODE_F13)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_8 0x10 -> 0x12") // ?
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_8 0x20 -> 0x08") PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_8 0x40 -> 0x43") // ?
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_8 0x80 -> 0x07") PORT_CODE(KEYCODE_F6)

	PORT_START("row_9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_9 0x01 -> 0x0b") PORT_CODE(KEYCODE_F10)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_9 0x02 -> 0x44") // ?
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_9 0x04 -> 0x0c") PORT_CODE(KEYCODE_F11)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_9 0x08 -> 0x0f") PORT_CODE(KEYCODE_F14)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_9 0x10 -> 0x50") // ?
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_9 0x20 -> 0x09") PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_9 0x40 -> 0x5a") // ?
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_9 0x80 -> 0x05") PORT_CODE(KEYCODE_F5)

	PORT_START("row_a")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_a 0x01 -> 0x1e") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_a 0x02 -> 0x60") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_a 0x04 -> 0x31") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_a 0x08 -> 0x49") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_a 0x10 -> 0x77") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_a 0x20 -> 0x04") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_a 0x40 -> 0x52") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_a 0x80 -> 0x40") PORT_CODE(KEYCODE_8_PAD)

	PORT_START("row_b")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_b 0x01 -> 0x21") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_b 0x02 -> 0x64") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_b 0x04 -> 0x39") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_b 0x08 -> 0x24") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_b 0x10 -> 0x26") PORT_CODE(KEYCODE_PRTSCR)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_b 0x20 -> 0x3c") PORT_CODE(KEYCODE_END)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_b 0x40 -> 0x4e") PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_b 0x80 -> 0x28") PORT_CODE(KEYCODE_PGUP)

	PORT_START("row_c")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_c 0x01 -> 0x20") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_c 0x02 -> 0x5d") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_c 0x04 -> 0x5b") // ?
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_c 0x08 -> 0x34") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_c 0x10 -> 0x33") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_c 0x20 -> 0x35") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_c 0x40 -> 0x36") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_c 0x80 -> 0x37") PORT_CODE(KEYCODE_O)

	PORT_START("row_d")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_d 0x01 -> 0x1f") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_d 0x02 -> 0x62") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_d 0x04 -> 0x32") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_d 0x08 -> 0x4c") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_d 0x10 -> 0x68") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_d 0x20 -> 0x3f") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_d 0x40 -> 0x57") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("row_d 0x80 -> 0x29") PORT_CODE(KEYCODE_NUMLOCK)
INPUT_PORTS_END

ioport_constructor adi_unk_kbd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( keyboard );
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void adi_unk_kbd_device::device_start()
{
	// register for save states
	save_item(NAME(m_key_row_p1));
	save_item(NAME(m_key_row_p2));
}

void adi_unk_kbd_device::device_reset()
{
}

void adi_unk_kbd_device::alt_w(int state)
{
	m_mcu->set_input_line(INPUT_LINE_IRQ0, state ? CLEAR_LINE : ASSERT_LINE);
}

int adi_unk_kbd_device::t0_r()
{
	return m_shift->read();
}

int adi_unk_kbd_device::t1_r()
{
	return m_ctrl->read();
}

uint8_t adi_unk_kbd_device::bus_r()
{
	uint8_t data = 0xff;

	// read keys selected by port 1 (bit 1 to 7)
	for (unsigned i = 1; i < 8; i++)
		if (BIT(m_key_row_p1, i) == 0)
			data &= m_keys[i - 1]->read();

	// read keys selected by port 2 (bit 0 to 6)
	for (unsigned i = 0; i < 7; i++)
		if (BIT(m_key_row_p2, i) == 0)
			data &= m_keys[i + 7]->read();

	return data;
}

void adi_unk_kbd_device::p1_w(uint8_t data)
{
	// 7654321-  keyboard row
	// -------0  serial data out

	m_key_row_p1 = data;
	m_txd_cb(BIT(data, 0));
}

uint8_t adi_unk_kbd_device::p2_r()
{
	// 7-------  caps lock
	// -6543210  n/a (write only)

	return (m_capslock->read() << 7) | (m_key_row_p2 & 0x7f);
}

void adi_unk_kbd_device::p2_w(uint8_t data)
{
	// 7-------  caps lock led
	// -6543210  keyboard row

	m_key_row_p2 = data;
}
