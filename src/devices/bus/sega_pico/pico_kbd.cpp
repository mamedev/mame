// license:BSD-3-Clause
// copyright-holders:QUFB
/***************************************************************************

    Keyboard Pico peripheral, required by a few Sega Pico games:

    * 837-19076 - Kibodeu Piko (Korea) (Samsung Pico)
    * HPC-6042-00 - Game wo Shinagara Oboe You! Kantan Wakuwaku Keyboard (Japan)
    * HPC-6052-00 - Keyboard Pico 2 Set with Sawattemiyou! Yoiko no Hajimete Keyboard (Japan)
    * HPC-6084-00 - Kitty to Minna no Keyboard Pico Sanrio Puroland ni Ikou! (Japan)

    A hidden test mode can be activated by covering sensors for pages 1, 3, and 5,
    while leaving the other sensors exposed. Afterwards, hold down the red button,
    and reset the console. If the machine configuration "Test Mode Pages" is enabled,
    the driver forces this page setup.

    Both Japanese and Korean models exist, although their layouts appear to be closely matched.
    These follow AT scan code set, except for a few custom keys.

    Japanese model PCB is marked "FK121301 (PICO KBD)", with a ROM marked "AZ165F00 0026T BPA6L1", currently undumped.

    Mappings for US ANSI 104 keyboard layout:

    | Esc            |
    | ￥ _           | ! 1 ぬ | " 2 ふ | # 3 あ | $ 4 う | % 5 え | & 6 お | ' 7 や |   ( 8 ゆ |   ) 9 よ |     0 わ |   = _ ほ | ｰ ^ へ   | Backspace | Insert   | Home    | CJK     |
                     |   Q た |   W て |   E い |   R す |   T か |   Y ん |   U な |     I に |     O ら |     P せ | { [ ｢ ゜ | } ] ｣ む | ` @ ゛    | Delete   | Sound   | Romaji  |
    | Caps Lock 英数 |   A ち |   S と |   D し |   F は |   G き |   H く |   J ま |     K の |     L り |   + ; れ |   * : け |          | Enter     |
    | Shift          |   Z つ |   X さ |   C そ |   V ひ |   B こ |   N み |   M も | < ､ , ね | > ｡ . る | ? ･ / め |                     |  ろ ー    |          |    ↑    |         |
                                                |                        Space                            |                                            |    ←     |    ↓    |    →    |

**********************************************************************/

#include "emu.h"

#include "pico_kbd.h"

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(PICO_KBD, pico_kbd_device, "pico_kbd", "Pico Keyboard")

static INPUT_PORTS_START( pico_kbd )
	PORT_START( "KEY1" )  // Scancodes 0x10-0x2f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("LSHIFT") PORT_CODE(KEYCODE_LSHIFT) // 0x12
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CJK") PORT_CODE(KEYCODE_PGUP) // 0x13
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) // 0x15
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) // 0x16
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ROMAJI") PORT_CODE(KEYCODE_PGDN) // 0x17
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) // 0x1a
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) // 0x1b
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) // 0x1c
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) // 0x1d
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) // 0x1e
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) // 0x21
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) // 0x22
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) // 0x23
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) // 0x24
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) // 0x25
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) // 0x26
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) // 0x29
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) // 0x2a
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) // 0x2b
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) // 0x2c
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) // 0x2d
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) // 0x2e
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_UNUSED)

	PORT_START( "KEY2" )  // Scancodes 0x30-0x4f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) // 0x31
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) // 0x32
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) // 0x33
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) // 0x34
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) // 0x35
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) // 0x36
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) // 0x3a
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) // 0x3b
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) // 0x3c
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) // 0x3d
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) // 0x3e
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("COMMA") PORT_CODE(KEYCODE_COMMA) // 0x41
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) // 0x42
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) // 0x43
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) // 0x44
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) // 0x45
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) // 0x46
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("STOP") PORT_CODE(KEYCODE_STOP) // 0x49
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("SLASH") PORT_CODE(KEYCODE_SLASH) // 0x4a
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) // 0x4b
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("SEMICOLON") PORT_CODE(KEYCODE_COLON) // 0x4c
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) // 0x4d
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("MINUS") PORT_CODE(KEYCODE_MINUS) // 0x4e
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_UNUSED)

	PORT_START( "KEY3" )  // Scancodes 0x50-0x6f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("RO") PORT_CODE(KEYCODE_RSHIFT) // 0x51
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("KE") PORT_CODE(KEYCODE_QUOTE) // 0x52
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("DAKUTEN") PORT_CODE(KEYCODE_BACKSLASH) // 0x54
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("EQUALS") PORT_CODE(KEYCODE_EQUALS) // 0x55
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CAPSLOCK") PORT_CODE(KEYCODE_CAPSLOCK) // 0x58
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) // 0x5a
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("OPENBRACE") PORT_CODE(KEYCODE_OPENBRACE) // 0x5b
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CLOSEBRACE") PORT_CODE(KEYCODE_CLOSEBRACE) // 0x5d
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("HOME") PORT_CODE(KEYCODE_HOME) // 0x64
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("BACKSPACE") PORT_CODE(KEYCODE_BACKSPACE) // 0x66
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("SOUND") PORT_CODE(KEYCODE_END) // 0x67
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("YEN") PORT_CODE(KEYCODE_TILDE) // 0x6a
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_UNUSED)

	PORT_START( "KEY4" )  // Scancodes 0x70-0x85
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) // 0x76
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("EQUALS") PORT_CODE(KEYCODE_EQUALS) // 0x79
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ASTERISK") PORT_CODE(KEYCODE_ASTERISK) // 0x7c
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("INSERT") PORT_CODE(KEYCODE_INSERT) // 0x81
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL) // 0x85
INPUT_PORTS_END

ioport_constructor pico_kbd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pico_kbd );
}

pico_kbd_device::pico_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PICO_KBD, tag, owner, clock)
	, device_pico_ps2_slot_interface(mconfig, *this)
	, m_io_keys(*this, "KEY%u", 1U)
{ }

void pico_kbd_device::device_start()
{
	save_item(NAME(m_caps_lock));
	save_item(NAME(m_has_caps_lock));
	save_item(NAME(m_has_read));
	save_item(NAME(m_i));
	save_item(NAME(m_key_state));
	save_item(NAME(m_is_negative));
	save_item(NAME(m_shift_state));
}

void pico_kbd_device::device_reset()
{
	m_caps_lock = 0;
	m_has_caps_lock = false;
	m_has_read = false;
	m_i = 0;
	m_key_state = 0;
	m_is_negative = false;
	m_shift_state = 0;
	m_start_time_keydown = 0;
	m_time_keydown = 0;
}

void pico_kbd_device::device_add_mconfig(machine_config &config)
{
}

uint16_t pico_kbd_device::parse_keycode()
{
	uint16_t key_shift = 0;
	uint16_t key = 0;
	uint16_t i = 0;
	for(int port = 0; port < 4; port++)
	{
		uint32_t port_bit = m_io_keys[port]->read();
		for(int bit = 0; bit < 32; bit++)
		{
			if (BIT(port_bit, bit) != 0)
			{
				i = 0x10 + (port * 0x20) + bit;
				if (i == PICO_KEYCODE_LSHIFT)
				{
					key_shift = i;
				}
				else
				{
					key = i;
				}
			}
		}
	}
	return (key_shift << 8) | key;
}

uint8_t pico_kbd_device::ps2_r(offs_t offset)
{
	uint8_t d = 0;

	if (machine().side_effects_disabled()) {
		return d;
	}

	m_has_read = true;

	uint16_t io_key = parse_keycode();
	uint32_t key_shift = (io_key & 0xff00) >> 8;
	uint32_t key = (io_key & 0x00ff);

	// The Shift key requires 2 key up events to be registered:
	// SHIFT_UP_HELD_DOWN to allow the game to register the key down event
	// for the next held down key(s), and SHIFT_UP when the Shift key
	// is no longer held down.
	//
	// For the second key up event, we need to
	// override the parsed key code with PICO_KEYCODE_LSHIFT,
	// otherwise it will be zero and the game won't clear its Shift key state.
	uint32_t key_code = (key_shift
			&& !key
			&& m_key_state != KEY_UP
			&& m_shift_state != SHIFT_UP_HELD_DOWN)
		? key_shift
		: m_shift_state == SHIFT_UP ? PICO_KEYCODE_LSHIFT : key;
	uint32_t key_code_7654 = (key_code & 0xf0) >> 4;
	uint32_t key_code_3210 = (key_code & 0x0f);
	switch (m_i)
	{
		case 0x0:
			d = 1; // m5id
			break;
		case 0x1:
			d = 3; // m6id
			break;
		case 0x2:
			d = 4; // data size
			break;
		case 0x3:
			d = 0; // pad1 rldu
			break;
		case 0x4:
			d = 0; // pad2 sacb
			break;
		case 0x5:
			d = 0; // pad3 rxyz
			break;
		case 0x6:
			d = 0; // l&kbtype
			break;
		case 0x7: // cap/num/scr
			if (key == PICO_KEYCODE_CAPSLOCK && m_has_caps_lock)
			{
				m_caps_lock = m_caps_lock == 4 ? 0 : 4;
				m_has_caps_lock = false;
			}
			d = m_caps_lock;
			break;
		case 0x8:
			d = 6;
			if (key)
			{
				m_key_state = KEY_DOWN;
			}
			if (!key)
			{
				m_key_state = !m_key_state ? 0 : (m_key_state + 1) % (KEY_UP + 1);
				m_start_time_keydown = 0;
			}
			if (key_shift && !key)
			{
				if (m_shift_state < SHIFT_RELEASED_HELD_DOWN)
				{
					m_shift_state++;
				}
				m_start_time_keydown = 0;
			}
			if (!key_shift)
			{
				m_shift_state = !m_shift_state ? 0 : (m_shift_state + 1) % (SHIFT_UP + 1);
			}

			if (m_key_state == KEY_DOWN || m_shift_state == SHIFT_DOWN)
			{
				uint64_t ticks_msec = osd_ticks() * 1000 / osd_ticks_per_second();
				if (m_start_time_keydown == 0)
				{
					d |= 8; // Send key down a.k.a. make
					m_time_keydown = 0;
					m_start_time_keydown = ticks_msec;
					if (m_key_state == KEY_DOWN)
						LOG("m_key_state: KEY DOWN\n");
					else
						LOG("m_key_state: SHIFT DOWN\n");
				}
				// Simulate key repeat while held down a.k.a. typematic
				// FIXME: Guessed default timing
				m_time_keydown = ticks_msec - m_start_time_keydown;
				LOG("time: %d\n", m_time_keydown);
				if (m_time_keydown > 350
						// Modifier keys don't have typematic
						&& key_code != PICO_KEYCODE_CAPSLOCK
						&& key_code != PICO_KEYCODE_LSHIFT)
				{
					d |= 8; // Send key down a.k.a. make
					if (m_key_state == KEY_DOWN)
						LOG("m_key_state: KEY DOWN\n");
					else
						LOG("m_key_state: SHIFT DOWN\n");
				}
				// Must register key up while typematic not active (expected by Kibodeu Piko)
				if ((d & 8) == 0)
				{
					d |= 1; // Send key up a.k.a. break
				}
			}
			if (m_key_state == KEY_UP
					|| m_shift_state == SHIFT_UP_HELD_DOWN
					|| m_shift_state == SHIFT_UP)
			{
				d |= 1; // Send key up a.k.a. break
				m_start_time_keydown = 0;
				if (m_key_state == KEY_UP)
					LOG("m_key_state: KEY UP\n");
				else
					LOG("m_key_state: SHIFT UP\n");
			}
			break;
		case 0x9:
			d = key_code_7654; // data 7654
			break;
		case 0xa:
			d = key_code_3210; // data 3210
			break;
		case 0xb:
			// Fallthrough (unused?)
		case 0xc:
			// Fallthrough (unused?)
		default:
			d = 0;
			break;
	}

	if (m_is_negative)
	{
		d |= 0xf0;
	}

	return d;
}

void pico_kbd_device::ps2_w(offs_t offset, uint8_t data)
{
	switch (data)
	{
		case 0x0:
			m_is_negative = false;
			m_i++;
			break;
		case 0x20:
			m_is_negative = true;
			if (m_has_read)
			{
				m_i++;
			}
			break;
		case 0x40:
			break;
		case 0x60:
			m_i = 0;
			m_has_read = false;
			break;
		default:
			break;
	}
}
