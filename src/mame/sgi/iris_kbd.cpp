// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Silicon Graphics IRIS keyboard emulation.
 *
 * The physical interface is a DIN-5 which plugs into a keyboard junction box,
 * which combines keyboard and mouse signals and connects to the console port
 * on the IRIS. The keyboard contains a speaker which generates key clicks and
 * short/long beeps under host control.
 *
 * The keyboard protocol asynchronous serial at 600 baud, with 1 start, 8 data,
 * odd parity and 1 stop bit. Modifier keys and LEDs are managed by the host.
 *
 * Sources:
 *  -
 *
 * TODO:
 *  - verify mcu part and clock
 *  - break key
 *
 */

#include "emu.h"
#include "iris_kbd.h"

#include "cpu/mcs48/mcs48.h"
#include "machine/keyboard.h"
#include "sound/beep.h"

#include "speaker.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

namespace {

class iris_kbd_device
	: public device_t
	, public device_rs232_port_interface
{
public:
	iris_kbd_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: device_t(mconfig, IRIS_KBD, tag, owner, clock)
		, device_rs232_port_interface(mconfig, *this)
		, m_mcu(*this, "mcu")
		, m_beep(*this, "beep")
		, m_matrix(*this, "col.%x", 0U)
		, m_led(*this, "LED%u", 1U)
		, m_p1(0xff)
		, m_p2(0xff)
		, m_col(0)
	{
	}

	// device_rs232_port_interface implementation
	virtual void input_txd(int state) override;

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual tiny_rom_entry const *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void map_mem(address_map &map) ATTR_COLD;
	virtual void map_pio(address_map &map) ATTR_COLD;

	void p1_w(u8 data);
	void p2_w(u8 data);
	u8 led_r(offs_t offset);

private:
	required_device<i8748_device> m_mcu;
	required_device<beep_device> m_beep;

	required_ioport_array<12> m_matrix;
	output_finder<8> m_led;

	u8 m_p1;
	u8 m_p2;
	u8 m_col; // selected matrix column
};

void iris_kbd_device::device_add_mconfig(machine_config &config)
{
	I8748(config, m_mcu, 4.608_MHz_XTAL);
	m_mcu->set_addrmap(AS_PROGRAM, &iris_kbd_device::map_mem);
	m_mcu->set_addrmap(AS_IO, &iris_kbd_device::map_pio);
	m_mcu->p1_out_cb().set(FUNC(iris_kbd_device::p1_w));
	m_mcu->p1_in_cb().set([this]() { return m_matrix[m_col]->read(); });
	m_mcu->p2_out_cb().set(FUNC(iris_kbd_device::p2_w));

	speaker_device &speaker(SPEAKER(config, "speaker"));
	speaker.front_center();

	BEEP(config, m_beep, 480).add_route(ALL_OUTPUTS, speaker, 0.25);
}

void iris_kbd_device::map_mem(address_map &map)
{
	map(0x0000, 0x03ff).rom().region("mcu", 0);
}

void iris_kbd_device::map_pio(address_map &map)
{
	map(0x00, 0xff).r(FUNC(iris_kbd_device::led_r));
}

void iris_kbd_device::device_start()
{
	save_item(NAME(m_p1));
	save_item(NAME(m_p2));
	save_item(NAME(m_col));

	m_led.resolve();
}

void iris_kbd_device::input_txd(int state)
{
	m_mcu->set_input_line(INPUT_LINE_IRQ0, state ? CLEAR_LINE : ASSERT_LINE);
}

void iris_kbd_device::p1_w(u8 data)
{
	LOG("p1_w 0x%02x\n", data);

	m_p1 = data;
}

void iris_kbd_device::p2_w(u8 data)
{
	LOG("p2_w 0x%02x\n", data);

	// bit 4: latch column select
	if (!BIT(m_p2, 4) && BIT(data, 4))
		m_col = m_p1 & 0xf;

	// bit 5: keyboard transmit
	output_rxd(BIT(data, 5));

	// bit 6: beeper on/off (1=off)
	m_beep->set_state(!BIT(data, 6));

	// bit 7: caps lock LED
	m_led[7] = BIT(data, 7);

	m_p2 = data;
}

u8 iris_kbd_device::led_r(offs_t offset)
{
	// TODO: led0 and led1 are inverses?
	for (unsigned i = 0; i < 7; i++)
		m_led[i] = BIT(m_p1, i + 1);

	return 0;
}

INPUT_PORTS_START(iris_kbd)
	PORT_START("col.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Setup")             PORT_CODE(KEYCODE_HOME)         PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Control")           PORT_CODE(KEYCODE_LCONTROL)     PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
																			   PORT_CODE(KEYCODE_RCONTROL)     PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Caps Lock")         PORT_CODE(KEYCODE_CAPSLOCK)     PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R Shift")           PORT_CODE(KEYCODE_RSHIFT)       PORT_CHAR(UCHAR_MAMEKEY(RSHIFT))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L Shift")           PORT_CODE(KEYCODE_LSHIFT)       PORT_CHAR(UCHAR_MAMEKEY(LSHIFT))

	PORT_START("col.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc")               PORT_CODE(KEYCODE_ESC)          PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_1)            PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab")               PORT_CODE(KEYCODE_TAB)          PORT_CHAR(9)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_Q)            PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_A)            PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_S)            PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Scroll Lock")       PORT_CODE(KEYCODE_SCRLOCK)      PORT_CHAR(UCHAR_MAMEKEY(SCRLOCK))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("col.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_2)            PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_3)            PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_W)            PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_E)            PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_D)            PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_F)            PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_Z)            PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_X)            PORT_CHAR('x') PORT_CHAR('X')

	PORT_START("col.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_4)            PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_5)            PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_R)            PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_T)            PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_G)            PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_H)            PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_C)            PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_V)            PORT_CHAR('v') PORT_CHAR('V')

	PORT_START("col.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_6)            PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_7)            PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_Y)            PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_U)            PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_J)            PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_K)            PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_B)            PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_N)            PORT_CHAR('n') PORT_CHAR('N')

	PORT_START("col.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_8)            PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_9)            PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_I)            PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_O)            PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_L)            PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_COLON)        PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_M)            PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_COMMA)        PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("col.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_0)            PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_MINUS)        PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_P)            PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_OPENBRACE)    PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_QUOTE)        PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_ENTER)        PORT_CHAR(13)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_STOP)         PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_SLASH)        PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("col.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_EQUALS)       PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_TILDE)        PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_CLOSEBRACE)   PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_BACKSLASH)    PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP 1")              PORT_CODE(KEYCODE_1_PAD)        PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP 0")              PORT_CODE(KEYCODE_0_PAD)        PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Line Feed")                                         PORT_CHAR(10)

	PORT_START("col.8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Backspace")         PORT_CODE(KEYCODE_BACKSPACE)    PORT_CHAR(8)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Delete")            PORT_CODE(KEYCODE_DEL)          PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP 4")              PORT_CODE(KEYCODE_4_PAD)        PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP 2")              PORT_CODE(KEYCODE_2_PAD)        PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP 3")              PORT_CODE(KEYCODE_3_PAD)        PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP .")              PORT_CODE(KEYCODE_DEL_PAD)      PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))

	PORT_START("col.9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP 7")              PORT_CODE(KEYCODE_7_PAD)        PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP 8")              PORT_CODE(KEYCODE_8_PAD)        PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP 5")              PORT_CODE(KEYCODE_5_PAD)        PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP 6")              PORT_CODE(KEYCODE_6_PAD)        PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PF2")               PORT_CODE(KEYCODE_F2)           PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PF1")               PORT_CODE(KEYCODE_F1)           PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left")              PORT_CODE(KEYCODE_LEFT)         PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down")              PORT_CODE(KEYCODE_DOWN)         PORT_CHAR(UCHAR_MAMEKEY(DOWN))

	PORT_START("col.a")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP 9")              PORT_CODE(KEYCODE_9_PAD)        PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP -")              PORT_CODE(KEYCODE_MINUS_PAD)    PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP ,")              PORT_CODE(KEYCODE_PLUS_PAD)     PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PF4")               PORT_CODE(KEYCODE_F4)           PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PF3")               PORT_CODE(KEYCODE_F3)           PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right")             PORT_CODE(KEYCODE_RIGHT)        PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up")                PORT_CODE(KEYCODE_UP)           PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("col.b")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter")             PORT_CODE(KEYCODE_ENTER_PAD)    PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space")             PORT_CODE(KEYCODE_SPACE)        PORT_CHAR(' ')
INPUT_PORTS_END

ioport_constructor iris_kbd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(iris_kbd);
}

ROM_START(iris_kbd)
	ROM_REGION(0x400, "mcu", 0)
	ROM_LOAD("iris-kb.bin", 0x0, 0x400, CRC(3a2a6887) SHA1(ed59ef7ad493b686f8f2f764549772b75bbd0735))
ROM_END

tiny_rom_entry const *iris_kbd_device::device_rom_region() const
{
	return ROM_NAME(iris_kbd);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(IRIS_KBD, device_rs232_port_interface, iris_kbd_device, "iris_kbd", "Silicon Graphics IRIS Keyboard")
