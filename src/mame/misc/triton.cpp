// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*******************************************************************************

    Transam Triton

    TODO:
    - cassette interface
    - keyboard auto repeat (optional)
    - parallel printer on port 3, uses INT3 (optional)

*******************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/ay31015.h"
#include "machine/clock.h"
#include "machine/kr2376.h"
#include "machine/timer.h"
#include "sound/beep.h"
#include "video/ef9364.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "bus/rs232/rs232.h"
#include "imagedev/cassette.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "triton.lh"


namespace {

class triton_state : public driver_device
{
public:
	triton_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ef9364(*this, "ef9364")
		, m_charset(*this, "ef9364")
		, m_kr2376(*this, "kr2376")
		, m_uart(*this, "uart")
		, m_cassette(*this, "cassette")
		, m_beeper(*this, "beeper")
		, m_led(*this, "led%u", 0U)
		, m_rom(*this, "rom")
		, m_vidcon(*this, "vidcon")
		, m_chargen(*this, "chargen")
		, m_graphics(*this, "graphics")
		, m_palette(*this, "palette")
		, m_user1(*this, "user1")
		, m_user2(*this, "user2")
		, m_serial(*this, "serial")
		, m_modifiers(*this, "MODIFIERS")
		, m_config(*this, "CONFIG")
	{ }

	void triton1(machine_config &config);
	void triton2(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(pushbutton_changed);
	DECLARE_INPUT_CHANGED_MEMBER(charset_changed);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<ef9364_device> m_ef9364;
	required_region_ptr<uint8_t> m_charset;
	required_device<kr2376_device> m_kr2376;
	required_device<ay31015_device> m_uart;
	required_device<cassette_image_device> m_cassette;
	required_device<beep_device> m_beeper;
	output_finder<8> m_led;
	required_region_ptr<uint8_t> m_rom;
	required_region_ptr<uint8_t> m_vidcon;
	required_region_ptr<uint8_t> m_chargen;
	required_region_ptr<uint8_t> m_graphics;
	required_device<palette_device> m_palette;
	required_device<generic_slot_device> m_user1;
	required_device<generic_slot_device> m_user2;
	required_device<rs232_port_device> m_serial;
	required_ioport m_modifiers;
	required_ioport m_config;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	IRQ_CALLBACK_MEMBER(inta_cb);

	void update_charset();

	uint8_t rom_r(offs_t offset);

	uint8_t port0_r();
	uint8_t port1_r();
	void port3_w(uint8_t data);
	void port5_w(uint8_t data);
	void port6_w(uint8_t data);
	void port7_w(uint8_t data);

	uint16_t m_int_vector;
	std::unique_ptr<uint8_t[]> m_exp_ram;
};


void triton_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).r(FUNC(triton_state::rom_r));
	map(0x1000, 0x13ff).rw(m_ef9364, FUNC(ef9364_device::videoram_r), FUNC(ef9364_device::videoram_w));
	map(0x1400, 0x1fff).ram();
	map(0xc000, 0xdfff).rom().region("eprom_6", 0); // 8K Eprom card
	map(0xe000, 0xffff).rom().region("eprom_7", 0); // 8K Eprom card
}

void triton_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).r(FUNC(triton_state::port0_r));            // Keyboard INPUT
	map(0x01, 0x01).r(FUNC(triton_state::port1_r));            // Tape I/O UART status INPUT
	map(0x02, 0x02).w(m_uart, FUNC(ay51013_device::transmit)); // Tape I/O UART data strobe OUTPUT
	map(0x03, 0x03).w(FUNC(triton_state::port3_w));            // LEDs OUTPUT (note LEDs are on for "0")
	map(0x04, 0x04).r(m_uart, FUNC(ay51013_device::receive));  // Tape I/O UART receive data enable INPUT
	map(0x05, 0x05).w(FUNC(triton_state::port5_w));            // VDU OUTPUT (note strobe - bit 8 - has to be specially formatted by software)
	map(0x06, 0x06).w(FUNC(triton_state::port6_w));            // Serial OUTPUT on bit 8, bit 7 spare
	map(0x07, 0x07).w(FUNC(triton_state::port7_w));            // Bit 8 = Relay, Bit 7 = Oscillator (speaker)
}


static INPUT_PORTS_START(triton)
	// GRI.756 Keyboard (Encoded by KR2376-12)
	PORT_START("X0") // 3     2     1     RS    SUB   GS    SYN   STX   NAK   DC4   NUL
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X1") // 6     5     4     ]     [     ETX   CAN   GS    DEL   DLE   SOH
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[')  PORT_CHAR('{')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')  PORT_CHAR('}')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X2") // 9     8     7     |     ~     FS    EM    ESC   SUB   HT    ACK
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('|')  PORT_CHAR('\\') PORT_NAME(u8"¦ \\ FS")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('~')  PORT_CHAR('^')  PORT_NAME(u8"~ \u2191 RS") // up arrow ↑
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))   PORT_NAME("Esc")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X3") // US    VT    DC1   BS    SI    HT    LF    FF    CR    SP    ETB
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8)                    PORT_NAME("Del")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)                   PORT_NAME("Return")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X4") // -     0     9     8     7     6     5     4     3     2     1
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-')  PORT_CHAR('=')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)          PORT_CHAR('0')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')  PORT_CHAR(')')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('(')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)          PORT_CHAR('7')  PORT_CHAR('\'')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)          PORT_CHAR('6')  PORT_CHAR('&')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)          PORT_CHAR('5')  PORT_CHAR('%')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)          PORT_CHAR('4')  PORT_CHAR('$')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)          PORT_CHAR('3')  PORT_CHAR('#')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)          PORT_CHAR('2')  PORT_CHAR('"')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)          PORT_CHAR('1')  PORT_CHAR('!')

	PORT_START("X5") // \     p     o     i     u     y     t     r     e     w     q
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\\') PORT_CHAR('@')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)          PORT_CHAR('P')  PORT_CHAR('p')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)          PORT_CHAR('O')  PORT_CHAR('o')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)          PORT_CHAR('I')  PORT_CHAR('i')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)          PORT_CHAR('U')  PORT_CHAR('u')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)          PORT_CHAR('Y')  PORT_CHAR('y')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)          PORT_CHAR('T')  PORT_CHAR('t')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)          PORT_CHAR('R')  PORT_CHAR('r')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)          PORT_CHAR('E')  PORT_CHAR('e')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)          PORT_CHAR('W')  PORT_CHAR('w')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)          PORT_CHAR('Q')  PORT_CHAR('q')

	PORT_START("X6") // :     ;     l     k     j     h     g     f     d     s     a
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(':')  PORT_CHAR('*')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR(';')  PORT_CHAR('+')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)          PORT_CHAR('L')  PORT_CHAR('l')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)          PORT_CHAR('K')  PORT_CHAR('k')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)          PORT_CHAR('J')  PORT_CHAR('j')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)          PORT_CHAR('H')  PORT_CHAR('h')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)          PORT_CHAR('G')  PORT_CHAR('g')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)          PORT_CHAR('F')  PORT_CHAR('f')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)          PORT_CHAR('D')  PORT_CHAR('d')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)          PORT_CHAR('S')  PORT_CHAR('s')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)          PORT_CHAR('A')  PORT_CHAR('a')

	PORT_START("X7") // _     /     .     ,     m     n     b     v     c     x     z
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('_')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/')  PORT_CHAR('?')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')  PORT_CHAR('>')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')  PORT_CHAR('<')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)          PORT_CHAR('M')  PORT_CHAR('m')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)          PORT_CHAR('N')  PORT_CHAR('n')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)          PORT_CHAR('B')  PORT_CHAR('b')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)          PORT_CHAR('V')  PORT_CHAR('v')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)          PORT_CHAR('C')  PORT_CHAR('c')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)          PORT_CHAR('X')  PORT_CHAR('x')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)          PORT_CHAR('Z')  PORT_CHAR('z')

	PORT_START("MODIFIERS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Shift Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_TOGGLE

	PORT_START("PUSHBUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CODE(KEYCODE_F1) PORT_NAME("Reset")        PORT_CHANGED_MEMBER(DEVICE_SELF, triton_state, pushbutton_changed, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CODE(KEYCODE_F2) PORT_NAME("Clear Screen") PORT_CHANGED_MEMBER(DEVICE_SELF, triton_state, pushbutton_changed, 0xcf) // INT1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CODE(KEYCODE_F3) PORT_NAME("Initialise")   PORT_CHANGED_MEMBER(DEVICE_SELF, triton_state, pushbutton_changed, 0xd7) // INT2
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_CODE(KEYCODE_F4) PORT_NAME("Menu")         PORT_CHANGED_MEMBER(DEVICE_SELF, triton_state, pushbutton_changed, 0xdf) // INT3
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_CODE(KEYCODE_F5) PORT_NAME("Pause")        PORT_CHANGED_MEMBER(DEVICE_SELF, triton_state, pushbutton_changed, 1) PORT_TOGGLE

	PORT_START("CONFIG")
	PORT_CONFNAME(0x07, 0x00, "8K RAM Card")
	PORT_CONFSETTING(0x00, "0x8K No RAM Card")
	PORT_CONFSETTING(0x01, "1x8K 2000-3FFF")
	PORT_CONFSETTING(0x02, "2x8K 2000-5FFF")
	PORT_CONFSETTING(0x03, "3x8K 2000-7FFF")
	PORT_CONFSETTING(0x04, "4x8K 2000-9FFF")
	PORT_CONFSETTING(0x05, "5x8K 2000-BFFF")
	PORT_CONFNAME(0x10, 0x00, "Graphics PROM") PORT_CHANGED_MEMBER(DEVICE_SELF, triton_state, charset_changed, 0)
	PORT_CONFSETTING(0x00, "Graphics")
	PORT_CONFSETTING(0x10, "Lower Case")
	//PORT_CONFNAME(0x20, 0x00, "Auto repeat")
	//PORT_CONFSETTING(0x00, DEF_STR( No ))
	//PORT_CONFSETTING(0x20, DEF_STR( Yes ))
	//PORT_CONFNAME(0x40, 0x00, "Port 3")
	//PORT_CONFSETTING(0x00, "LED's")
	//PORT_CONFSETTING(0x40, "Parallel Printer")
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(triton_state::pushbutton_changed)
{
	switch (param)
	{
	case 0:
		m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
		break;
	case 1:
		m_maincpu->set_input_line(INPUT_LINE_HALT, newval ? ASSERT_LINE : CLEAR_LINE);
		break;
	default:
		m_int_vector = param;
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, newval ? ASSERT_LINE : CLEAR_LINE);
		break;
	}
}

INPUT_CHANGED_MEMBER(triton_state::charset_changed)
{
	update_charset();
}


void triton_state::update_charset()
{
	// copy selected charset to ef9364 region
	memcpy(&m_charset[0x0000], &m_graphics[BIT(m_config->read(), 4) * 0x200], 0x200);
	memcpy(&m_charset[0x0200], &m_graphics[BIT(m_config->read(), 4) * 0x200], 0x200);
	memcpy(&m_charset[0x0100], &m_chargen[0x100], 0x100);
	memcpy(&m_charset[0x0200], &m_chargen[0x000], 0x100);
}


uint8_t triton_state::rom_r(offs_t offset)
{
	uint8_t data = m_rom[offset];

	switch (offset & 0x0c00)
	{
	case 0x0400: // User1
		if (m_user1->exists())
		{
			data = m_user1->read_rom(offset & 0x3ff);
		}
		break;
	case 0x0800: // User2
		if (m_user2->exists())
		{
			data = m_user2->read_rom(offset & 0x3ff);
		}
		break;
	}

	return data;
}


uint8_t triton_state::port0_r()
{
	// TODO: strobe goes low on a timer (capacitance discharge) for auto repeat
	return m_kr2376->data_r() | (m_kr2376->get_output_pin(kr2376_device::KR2376_SO) << 7);
}

uint8_t triton_state::port1_r()
{
	uint8_t data = 0x00;

	data |= m_uart->dav_r() << 0;
	data |= m_uart->pe_r() << 1;
	data |= m_uart->fe_r() << 2;
	data |= m_uart->or_r() << 3;
	data |= m_uart->tbmt_r() << 4;

	return data;
}

void triton_state::port3_w(uint8_t data)
{
	for (int i = 0; i < 8; i++)
		m_led[i] = BIT(data, i);
}

void triton_state::port5_w(uint8_t data)
{
	if (data & 0x80)
		m_ef9364->command_w(m_vidcon[data & 0x7f]);
	else
		m_ef9364->char_latch_w(data & 0x7f);
}

void triton_state::port6_w(uint8_t data)
{
	m_serial->write_txd(!BIT(data, 7));
}

void triton_state::port7_w(uint8_t data)
{
	m_beeper->set_state(BIT(data, 6));

	m_cassette->change_state(BIT(data, 7) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
}


IRQ_CALLBACK_MEMBER(triton_state::inta_cb)
{
	return m_int_vector;
}


void triton_state::machine_start()
{
	for (int i = 0; i < 0x400; i++)
	{
		m_graphics[i] = bitswap<8>(m_graphics[i], 4, 3, 5, 2, 6, 1, 7, 0);
	}

	m_exp_ram = make_unique_clear<uint8_t[]>(0xe000);

	m_led.resolve();

	save_pointer(NAME(m_exp_ram), 0xe000);
}

void triton_state::machine_reset()
{
	uint16_t ramsize = (m_config->read() & 7) * 0x2000;

	m_maincpu->space(AS_PROGRAM).unmap_readwrite(0x2000, 0xbfff);

	if (ramsize)
	{
		m_maincpu->space(AS_PROGRAM).install_ram(0x2000, 0x2000 + ramsize - 1, m_exp_ram.get()); // 8K RAM cards
	}

	m_kr2376->set_input_pin(kr2376_device::KR2376_DSII, 0);
	m_kr2376->set_input_pin(kr2376_device::KR2376_PII, 0);

	update_charset();
}


static const gfx_layout charlayout =
{
	8, 12,                  /* 8 x 12 characters */
	64,                     /* 64 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 0*8, 0*8, 0*8, 0*8 },
	8*8                    /* every char takes 8 bytes */
};

static GFXDECODE_START(gfx_tritan)
	GFXDECODE_ENTRY("chargen", 0, charlayout, 0, 1)
	GFXDECODE_ENTRY("graphics", 0x000, charlayout, 0, 1)
	GFXDECODE_ENTRY("graphics", 0x200, charlayout, 0, 1)
GFXDECODE_END


static DEVICE_INPUT_DEFAULTS_START(printer)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_110)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_110)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY", 0xff, RS232_PARITY_MARK)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_2)
DEVICE_INPUT_DEFAULTS_END


void triton_state::triton1(machine_config &config)
{
	I8080A(config, m_maincpu, 7.2_MHz_XTAL / 9);
	m_maincpu->set_addrmap(AS_PROGRAM, &triton_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &triton_state::io_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(triton_state::inta_cb));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_screen_update("ef9364", FUNC(ef9364_device::screen_update));
	screen.set_size(64 * 8, 16 * 12);
	screen.set_visarea(0, 64 * 8 - 1, 0, 16 * 12 - 1);
	GFXDECODE(config, "gfxdecode", "palette", gfx_tritan);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	EF9364(config, m_ef9364, 1.008_MHz_XTAL); // SFF96364
	m_ef9364->set_screen("screen");
	m_ef9364->set_palette_tag("palette");
	m_ef9364->set_nb_of_pages(1);
	m_ef9364->set_erase(0x20);

	config.set_default_layout(layout_triton);

	KR2376_12(config, m_kr2376, 50000);
	m_kr2376->x<0>().set_ioport("X0");
	m_kr2376->x<1>().set_ioport("X1");
	m_kr2376->x<2>().set_ioport("X2");
	m_kr2376->x<3>().set_ioport("X3");
	m_kr2376->x<4>().set_ioport("X4");
	m_kr2376->x<5>().set_ioport("X5");
	m_kr2376->x<6>().set_ioport("X6");
	m_kr2376->x<7>().set_ioport("X7");
	m_kr2376->shift().set([this]() { return int(BIT(m_modifiers->read(), 0) || BIT(m_modifiers->read(), 2)); });
	m_kr2376->control().set([this]() { return int(BIT(m_modifiers->read(), 1)); });
	//m_kr2376->strobe().set([this](int state) { auto repeat timer });

	AY51013(config, m_uart);
	//m_uart->read_si_callback().set(FUNC(triton_state::si));
	//m_uart->write_so_callback().set(FUNC(triton_state::so));

	//CLOCK(config, m_uart_clock, 4800);
	//m_uart_clock->signal_handler().set(m_uart, FUNC(ay51013_device::write_rcp));
	//m_uart_clock->signal_handler().append(m_uart, FUNC(ay51013_device::write_tcp));

	//clock_device &uart_clock(CLOCK(config, "uart_clock", 4800));
	//uart_clock.signal_handler().set(FUNC(triton_state::kansas_w));
	//TIMER(config, "kansas_r").configure_periodic(FUNC(triton_state::kansas_r), attotime::from_hz(40000));

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 800).add_route(ALL_OUTPUTS, "mono", 1.00);

	GENERIC_SOCKET(config, m_user1, generic_plain_slot, "triton_rom", "rom,bin");
	GENERIC_SOCKET(config, m_user2, generic_plain_slot, "triton_rom", "rom,bin");

	RS232_PORT(config, m_serial, default_rs232_devices, "printer");
	m_serial->set_option_device_input_defaults("printer", DEVICE_INPUT_DEFAULTS_NAME(printer));

	SOFTWARE_LIST(config, "rom_ls").set_original("triton_rom");
}

void triton_state::triton2(machine_config &config)
{
	triton1(config);

	m_maincpu->set_clock(18_MHz_XTAL / 9);
}


ROM_START(triton41)
	ROM_REGION(0x1000, "rom", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "l41", "L4.1")
	ROMX_LOAD("monitor_4.1.rom", 0x0000, 0x0400, CRC(2228040d) SHA1(0e92fa1c0a1327dd77b6f9403a5f34aa8b464e9b), ROM_BIOS(0))
	ROMX_LOAD("basic_l4.1a.rom", 0x0400, 0x0400, CRC(468264b0) SHA1(bcd2705be359eb727f00ae54562afd75028ee34f), ROM_BIOS(0))
	ROMX_LOAD("basic_l4.1b.rom", 0x0800, 0x0400, CRC(858aa55c) SHA1(a49bf12a1f11d9c04ef30b4ca0278782e93bd95c), ROM_BIOS(0))

	ROM_REGION(0x2000, "eprom_6", ROMREGION_ERASEFF)
	ROM_REGION(0x2000, "eprom_7", ROMREGION_ERASEFF)

	ROM_REGION(0x0100, "vidcon", 0)
	ROM_LOAD("vducontrol.ic54", 0x0000, 0x0100, CRC(ad5d426e) SHA1(3db409fe7e3e9fc350acd5500594965c5f2bb5be)) // 74S287 (256 x 4bit)

	ROM_REGION(0x0200, "chargen", 0)
	ROM_LOAD("cgr-001.ic69",    0x0000, 0x0200, CRC(a7e567fc) SHA1(b18aae0a2d4f92f5a7e22640719bbc4652f3f4ee)) // RO-3-2513

	ROM_REGION(0x0400, "graphics", 0)
	ROM_LOAD("graphics.ic70",   0x0000, 0x0200, CRC(77364e43) SHA1(b6ec6543acacab544e8568f700a1b581025897a1)) // 74S472 (512 x 8bit)
	ROM_LOAD("lowercase.ic70",  0x0200, 0x0200, CRC(fd677f54) SHA1(7d5a907f97df1f7f7379df3eedbd36c5c071abd1)) // alternate lower case

	ROM_REGION(0x0400, "ef9364", ROMREGION_ERASE00)
ROM_END

ROM_START(triton51)
	ROM_REGION(0x1000, "rom", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "l51", "L5.1")
	ROMX_LOAD("humbug_5.1a.rom", 0x0000, 0x0400, CRC(502a7b4f) SHA1(b2e4dc177676c528c5f11518e0d0d4b40385be25), ROM_BIOS(0))
	ROMX_LOAD("basic_l5.1a.rom", 0x0400, 0x0400, CRC(08830f6f) SHA1(1467e35aa8e4d5c7e73a7b5bc0d1ef06ef1e0b4b), ROM_BIOS(0))
	ROMX_LOAD("basic_l5.1b.rom", 0x0800, 0x0400, CRC(4470590a) SHA1(a7aa5b900f33a4e7e973ccf4fc07cc7b02dbd336), ROM_BIOS(0))
	ROMX_LOAD("humbug_5.1b.rom", 0x0c00, 0x0400, CRC(6d47d54e) SHA1(c672ce87944a91f481c66202b7f1109348c3c6d1), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "l51p", "L5.1P") // supports parallel printer, implementation unknown.
	ROMX_LOAD("humbug_5.1p.rom", 0x0000, 0x0400, CRC(f68b10ac) SHA1(30b19012df1507781d2d8a070021dec50c1d8461), ROM_BIOS(1))
	ROMX_LOAD("basic_l5.1a.rom", 0x0400, 0x0400, CRC(08830f6f) SHA1(1467e35aa8e4d5c7e73a7b5bc0d1ef06ef1e0b4b), ROM_BIOS(1))
	ROMX_LOAD("basic_l5.1b.rom", 0x0800, 0x0400, CRC(4470590a) SHA1(a7aa5b900f33a4e7e973ccf4fc07cc7b02dbd336), ROM_BIOS(1))
	ROMX_LOAD("humbug_5.1b.rom", 0x0c00, 0x0400, CRC(6d47d54e) SHA1(c672ce87944a91f481c66202b7f1109348c3c6d1), ROM_BIOS(1))

	ROM_REGION(0x2000, "eprom_6", ROMREGION_ERASEFF)
	ROM_REGION(0x2000, "eprom_7", ROMREGION_ERASEFF)

	ROM_REGION(0x0100, "vidcon", 0)
	ROM_LOAD("vducontrol.ic54", 0x0000, 0x0100, CRC(ad5d426e) SHA1(3db409fe7e3e9fc350acd5500594965c5f2bb5be)) // 74S287 (256 x 4bit)

	ROM_REGION(0x0200, "chargen", 0)
	ROM_LOAD("cgr-001.ic69",    0x0000, 0x0200, CRC(a7e567fc) SHA1(b18aae0a2d4f92f5a7e22640719bbc4652f3f4ee)) // RO-3-2513

	ROM_REGION(0x0400, "graphics", 0)
	ROM_LOAD("graphics.ic70",   0x0000, 0x0200, CRC(77364e43) SHA1(b6ec6543acacab544e8568f700a1b581025897a1)) // 74S472 (512 x 8bit)
	ROM_LOAD("lowercase.ic70",  0x0200, 0x0200, CRC(fd677f54) SHA1(7d5a907f97df1f7f7379df3eedbd36c5c071abd1)) // alternate lower case

	ROM_REGION(0x400, "ef9364", ROMREGION_ERASE00)
ROM_END

ROM_START(triton52)
	ROM_REGION(0x1000, "rom", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "l52", "L5.2")
	ROMX_LOAD("humbug_5.2a.rom", 0x0000, 0x0400, CRC(9020f7ed) SHA1(ad0561c51b20684e49b180fb5d2535e2cf0e05e7), ROM_BIOS(0))
	ROMX_LOAD("basic_l5.1a.rom", 0x0400, 0x0400, CRC(08830f6f) SHA1(1467e35aa8e4d5c7e73a7b5bc0d1ef06ef1e0b4b), ROM_BIOS(0))
	ROMX_LOAD("basic_l5.1b.rom", 0x0800, 0x0400, CRC(4470590a) SHA1(a7aa5b900f33a4e7e973ccf4fc07cc7b02dbd336), ROM_BIOS(0))
	ROMX_LOAD("humbug_5.2b.rom", 0x0c00, 0x0400, CRC(47bc3961) SHA1(37f5c09e43f84227e6b4b76e02ba95721dcbf2f1), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "l52p", "L5.2P") // supports parallel printer, implementation unknown.
	ROMX_LOAD("humbug_5.2p.rom", 0x0000, 0x0400, CRC(eb2ded3d) SHA1(4e5b291f7a84deb629e351ec5166aadb6477d436), ROM_BIOS(1))
	ROMX_LOAD("basic_l5.1a.rom", 0x0400, 0x0400, CRC(08830f6f) SHA1(1467e35aa8e4d5c7e73a7b5bc0d1ef06ef1e0b4b), ROM_BIOS(1))
	ROMX_LOAD("basic_l5.1b.rom", 0x0800, 0x0400, CRC(4470590a) SHA1(a7aa5b900f33a4e7e973ccf4fc07cc7b02dbd336), ROM_BIOS(1))
	ROMX_LOAD("humbug_5.2b.rom", 0x0c00, 0x0400, CRC(47bc3961) SHA1(37f5c09e43f84227e6b4b76e02ba95721dcbf2f1), ROM_BIOS(1))

	ROM_REGION(0x2000, "eprom_6", ROMREGION_ERASEFF)
	ROM_LOAD("trap_2.1a.rom", 0x0000, 0x0400, CRC(115d1c80) SHA1(6b7a5b9f75432c012f1cc37ad3fe82d92adcd1e7))
	ROM_LOAD("trap_2.1b.rom", 0x0400, 0x0400, CRC(c8d4ba47) SHA1(e89b087630bf1deb4542a2ab6d697047430bd8a1))
	ROM_LOAD("trap_2.1c.rom", 0x0800, 0x0400, CRC(ad383c51) SHA1(95dac006c1e22bb9e820d9aee9b0ab0fdc745045))
	ROM_LOAD("trap_2.1d.rom", 0x0c00, 0x0400, CRC(cf0c1358) SHA1(9ad965f17589490753fb81e7781dc1bf4c82663d))
	ROM_LOAD("trap_2.1e.rom", 0x1000, 0x0400, CRC(211e5173) SHA1(e4cbd7b852ecd4fe46557efa0b3b2db24f83d5a1))
	ROM_LOAD("trap_2.1f.rom", 0x1400, 0x0400, CRC(5f45fe51) SHA1(4218328b697d52a5b85cf3c0280e0e097af3bf44))
	ROM_LOAD("trap_2.1g.rom", 0x1800, 0x0400, CRC(455e3212) SHA1(17e01b3a9b0a358cc483952d25a92823ced15698))
	ROM_LOAD("trap_2.1h.rom", 0x1c00, 0x0400, CRC(4fc2666d) SHA1(fdae9a762d10be5a69e0c5f889ffc86d370b2ae4))

	ROM_REGION(0x2000, "eprom_7", ROMREGION_ERASEFF)

	ROM_REGION(0x0100, "vidcon", 0)
	ROM_LOAD("vducontrol.ic54", 0x0000, 0x0100, CRC(ad5d426e) SHA1(3db409fe7e3e9fc350acd5500594965c5f2bb5be)) // 74S287 (256 x 4bit)

	ROM_REGION(0x0200, "chargen", 0)
	ROM_LOAD("cgr-001.ic69",    0x0000, 0x0200, CRC(a7e567fc) SHA1(b18aae0a2d4f92f5a7e22640719bbc4652f3f4ee)) // RO-3-2513

	ROM_REGION(0x0400, "graphics", 0)
	ROM_LOAD("graphics.ic70",   0x0000, 0x0200, CRC(77364e43) SHA1(b6ec6543acacab544e8568f700a1b581025897a1)) // 74S472 (512 x 8bit)
	ROM_LOAD("lowercase.ic70",  0x0200, 0x0200, CRC(fd677f54) SHA1(7d5a907f97df1f7f7379df3eedbd36c5c071abd1)) // alternate lower case

	ROM_REGION(0x400, "ef9364", ROMREGION_ERASE00)
ROM_END

ROM_START(triton72)
	ROM_REGION(0x1000, "rom", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "l72", "L7.2")
	ROMX_LOAD("monitor_7.2a.rom", 0x0000, 0x0400, CRC(0ea43528) SHA1(40fd2d435879dbb4931eec1c091c8c7531a1a96f), ROM_BIOS(0))
	ROMX_LOAD("monitor_7.2b.rom", 0x0c00, 0x0400, CRC(0e75d714) SHA1(bfc9c68dc8e86503c5ecacd454b633f95b407727), ROM_BIOS(0))

	ROM_REGION(0x2000, "eprom_6", ROMREGION_ERASEFF)
	ROM_LOAD("trap_2.1a.rom", 0x0000, 0x0400, CRC(115d1c80) SHA1(6b7a5b9f75432c012f1cc37ad3fe82d92adcd1e7))
	ROM_LOAD("trap_2.1b.rom", 0x0400, 0x0400, CRC(c8d4ba47) SHA1(e89b087630bf1deb4542a2ab6d697047430bd8a1))
	ROM_LOAD("trap_2.1c.rom", 0x0800, 0x0400, CRC(ad383c51) SHA1(95dac006c1e22bb9e820d9aee9b0ab0fdc745045))
	ROM_LOAD("trap_2.1d.rom", 0x0c00, 0x0400, CRC(cf0c1358) SHA1(9ad965f17589490753fb81e7781dc1bf4c82663d))
	ROM_LOAD("trap_2.1e.rom", 0x1000, 0x0400, CRC(211e5173) SHA1(e4cbd7b852ecd4fe46557efa0b3b2db24f83d5a1))
	ROM_LOAD("trap_2.1f.rom", 0x1400, 0x0400, CRC(5f45fe51) SHA1(4218328b697d52a5b85cf3c0280e0e097af3bf44))
	ROM_LOAD("trap_2.1g.rom", 0x1800, 0x0400, CRC(455e3212) SHA1(17e01b3a9b0a358cc483952d25a92823ced15698))
	ROM_LOAD("trap_2.1h.rom", 0x1c00, 0x0400, CRC(4fc2666d) SHA1(fdae9a762d10be5a69e0c5f889ffc86d370b2ae4))

	ROM_REGION(0x2000, "eprom_7", ROMREGION_ERASEFF)
	ROM_LOAD("basic_l7.2a.rom", 0x0000, 0x0400, CRC(d0713894) SHA1(f56ad2af91f25b86737f70c25e08d3aa358eb009))
	ROM_LOAD("basic_l7.2b.rom", 0x0400, 0x0400, CRC(7604032f) SHA1(103cbf0a9f8706cf6a8e0e759bab587636a4fff6))
	ROM_LOAD("basic_l7.2c.rom", 0x0800, 0x0400, CRC(1255fb1e) SHA1(6cb76813a3604c4fcc689fdf861b9e1de361f3fa))
	ROM_LOAD("basic_l7.2d.rom", 0x0c00, 0x0400, CRC(df2db6a1) SHA1(da0b185707fe30596b25a560709a1eeb535070ed))
	ROM_LOAD("basic_l7.2e.rom", 0x1000, 0x0400, CRC(9fc0a643) SHA1(5d1758a4cf6741e1e5ef6f655f21a4704ac72125))
	ROM_LOAD("basic_l7.2f.rom", 0x1400, 0x0400, CRC(002723ad) SHA1(146ff347d893d4aba4148fc2ce88bbe08abab479))
	ROM_LOAD("basic_l7.2g.rom", 0x1800, 0x0400, CRC(b43d50de) SHA1(dde0d26713b3a346c8bbcff9c40e2c1d448fb8d5))
	ROM_LOAD("basic_l7.2h.rom", 0x1c00, 0x0400, CRC(2af7f180) SHA1(7f18375147eb18f90b15606cd9b1578fd3fb3e76))

	ROM_REGION(0x0100, "vidcon", 0)
	ROM_LOAD("vducontrol.ic54", 0x0000, 0x0100, CRC(ad5d426e) SHA1(3db409fe7e3e9fc350acd5500594965c5f2bb5be)) // 74S287 (256 x 4bit)

	ROM_REGION(0x0200, "chargen", 0)
	ROM_LOAD("cgr-001.ic69",    0x0000, 0x0200, CRC(a7e567fc) SHA1(b18aae0a2d4f92f5a7e22640719bbc4652f3f4ee)) // RO-3-2513

	ROM_REGION(0x0400, "graphics", 0)
	ROM_LOAD("graphics.ic70",   0x0000, 0x0200, CRC(77364e43) SHA1(b6ec6543acacab544e8568f700a1b581025897a1)) // 74S472 (512 x 8bit)
	ROM_LOAD("lowercase.ic70",  0x0200, 0x0200, CRC(fd677f54) SHA1(7d5a907f97df1f7f7379df3eedbd36c5c071abd1)) // alternate lower case

	ROM_REGION(0x400, "ef9364", ROMREGION_ERASE00)
ROM_END

} // anonymous namespace

//    YEAR  NAME       PARENT     COMPAT  MACHINE   INPUT    CLASS          INIT        COMPANY     FULLNAME        FLAGS
COMP( 1978, triton41,  triton72,  0,      triton1,  triton,  triton_state,  empty_init, "Transam",  "Triton L4.1",                MACHINE_NOT_WORKING )
COMP( 1979, triton51,  triton72,  0,      triton1,  triton,  triton_state,  empty_init, "Transam",  "Triton L5.1",                MACHINE_NOT_WORKING )
//COMP( 1979, triton61,  triton72,  0,      triton1,  triton,  triton_state,  empty_init, "Transam",  "Triton L6.1",                MACHINE_NOT_WORKING )
COMP( 1980, triton52,  triton72,  0,      triton2,  triton,  triton_state,  empty_init, "Transam",  "Triton L5.2",                MACHINE_NOT_WORKING )
//COMP( 1980, triton62,  triton72,  0,      triton2,  triton,  triton_state,  empty_init, "Transam",  "Triton L6.2",                MACHINE_NOT_WORKING )
COMP( 1980, triton72,  0,         0,      triton2,  triton,  triton_state,  empty_init, "Transam",  "Triton L7.2",                MACHINE_NOT_WORKING )
//COMP( 1980, triton82,  0,         0,      triton2,  triton,  triton_state,  empty_init, "Transam",  "Triton L8.2 Pascal System",  MACHINE_NOT_WORKING )
//COMP( 1980, triton92,  0,         0,      triton2,  triton,  triton_state,  empty_init, "Transam",  "Triton L9.2 Disk System",    MACHINE_NOT_WORKING )
