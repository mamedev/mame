// license:BSD-3-Clause
// copyright-holders:Brian Johnson
/*****************************************************************************
 * NABU PC Keyboard Interface
 *
 * Matrix
 * |       | Col 0 | Col 1 | Col 2 | Col 3 | Col 4 | Col 5 | Col 6 | Col 7 |
 * |-------|-------|-------|-------|-------|-------|-------|-------|-------|
 * | Row 0 |  N/A  |  ESC  | Right |   P   |   W   |   R   |   I   |   ]   |
 * | Row 1 |   U   |   Q   | Left  |   [   |   E   |   T   |   O   |  DEL  |
 * | Row 2 |   8   |   2   | Up    |   =   |   4   |   6   |   0   |  TV   |
 * | Row 3 |   7   |   1   | Yes   |   -   |   3   |   5   |   9   | PAUSE |
 * | Row 4 |   G   |  TAB  | No    |   L   |   A   |   D   |   J   |   '   |
 * | Row 5 | SPACE |   X   | <|||  |   /   |   V   |  N/A  |   ,   |  SYM  |
 * | Row 6 |   N   |   C   | |||>  |   .   |   B   |   Z   |   M   |  N/A  |
 * | Row 7 |   H   |   Y   | Down  |   ;   |   S   |   F   |   K   |  GO   |
 * |-------|-------|-------|-------|-------|-------|-------|-------|-------|
 * |  Mod  | Ctrl  | Caps  | Shift |  N/A  |  N/A  |  N/A  |  N/A  |  N/A  |
 *
 * Port 1 on the 6803 MCU is used to scan the keyboard matrix. The low six bits contain the row/col,
 * bit 6 is set when scanning the modifiers, and bit 7 is read to determine if the addressed key is pressed.
 *
 *   7          6    5 4 3   2 1 0
 * --------------------------------
 * | Pressed | Mod |  Row  |  Col |
 * --------------------------------
 *
 * The modifier keys are handled internally by the keyboard and are never sent to the
 * computer.
 *
 * They keyboard also contains 4 read-only gameport registers with 0 meaning
 * the digital contact is set (Only two ports are populated)
 *    * 0x5X00 - a read from these addresses reads the gameport data from port X (where X is 0-3)
 *
 *   7   6   5   4   3   2   1   0
 * ---------------------------------
 * | B | - | - | - | U | R | D | L |
 * --------------------------------
 *
 * The ADC is used to decode the analog paddle data on pins 5 and 9 of the gameports
 *   * 0xBX00 - A write to these addresses latches the ADC address to the value of X (where X is 0-7)
 *   * 0x7000 - A write to this address starts the conversion for the currently latched ADC address, an IRQ will trigger on EOC
 *   * 0xD000 - A read from here will read the currently converted value from the ADC
 *   * 0x9000 - a write to this address will acknowledge the EOC interrupt.
 *****************************************************************************/

#include "emu.h"
#include "nabupc_kbd.h"



namespace  {

//**************************************************************************
//  KEYBOARD ROM
//**************************************************************************

ROM_START(nabu_keyboard_rom)
	ROM_REGION( 0x800, "mcu", 0 )
	ROM_LOAD( "nabukeyboard-90020070-reva-2716.bin", 0x000, 0x800, CRC(eead3abc) SHA1(2f6ff63ca2f2ac90f3e03ef4f2b79883205e8a4e) )
ROM_END

//**************************************************************************
//  KEYBOARD PORTS
//**************************************************************************

INPUT_PORTS_START( keyboard_ports )
	// Keyboard Matrix
	PORT_START("ROW0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)        PORT_NAME("Esc")       PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)      PORT_NAME("Right")     PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_NAME("P")         PORT_CHAR('p')   PORT_CHAR('P')   PORT_CHAR(0x10)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_NAME("W")         PORT_CHAR('w')   PORT_CHAR('W')   PORT_CHAR(0x17)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_NAME("R")         PORT_CHAR('r')   PORT_CHAR('R')   PORT_CHAR(0x12)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_NAME("I")         PORT_CHAR('i')   PORT_CHAR('I')   PORT_CHAR(0x09)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("]")         PORT_CHAR(']')   PORT_CHAR('}')   PORT_CHAR(0x1D)
	PORT_START("ROW1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_NAME("U")         PORT_CHAR('u')   PORT_CHAR('U')   PORT_CHAR(0x15)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_NAME("Q")         PORT_CHAR('q')   PORT_CHAR('Q')   PORT_CHAR(0x11)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)       PORT_NAME("Left")      PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_NAME("[")         PORT_CHAR('[')   PORT_CHAR('{')   PORT_CHAR(0x1B)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_NAME("E")         PORT_CHAR('e')   PORT_CHAR('E')   PORT_CHAR(0x05)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_NAME("T")         PORT_CHAR('t')   PORT_CHAR('T')   PORT_CHAR(0x14)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_NAME("O")         PORT_CHAR('o')   PORT_CHAR('O')   PORT_CHAR(0x0F)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_NAME("Delete")    PORT_CHAR(0x08)
	PORT_START("ROW2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_NAME("8")         PORT_CHAR('8')   PORT_CHAR('*')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_NAME("2")         PORT_CHAR('2')   PORT_CHAR('@')   PORT_CHAR(0x00)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_NAME("Up")        PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_NAME("=")         PORT_CHAR('=')   PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_NAME("4")         PORT_CHAR('4')   PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_NAME("6")         PORT_CHAR('6')   PORT_CHAR('^')   PORT_CHAR(0x1E)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_NAME("0")         PORT_CHAR('0')   PORT_CHAR(')')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)         PORT_NAME("TV/NABU")   PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_START("ROW3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_NAME("7")         PORT_CHAR('7')   PORT_CHAR('&')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_NAME("1")         PORT_CHAR('1')   PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)         PORT_NAME("Yes")       PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_NAME("-")         PORT_CHAR('-')   PORT_CHAR('_')   PORT_CHAR(0x1F)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_NAME("3")         PORT_CHAR('3')   PORT_CHAR('#')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_NAME("5")         PORT_CHAR('5')   PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_NAME("9")         PORT_CHAR('9')   PORT_CHAR('(')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PAUSE)      PORT_NAME("Pause")     PORT_CHAR(UCHAR_MAMEKEY(PAUSE))
	PORT_START("ROW4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_NAME("G")         PORT_CHAR('g')   PORT_CHAR('G')   PORT_CHAR(0x07)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)        PORT_NAME("Tab")       PORT_CHAR(0x09)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)         PORT_NAME("No")        PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_NAME("L")         PORT_CHAR('l')   PORT_CHAR('L')   PORT_CHAR(0x0C)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_NAME("A")         PORT_CHAR('a')   PORT_CHAR('A')   PORT_CHAR(0x01)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_NAME("D")         PORT_CHAR('d')   PORT_CHAR('D')   PORT_CHAR(0x04)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_NAME("J")         PORT_CHAR('j')   PORT_CHAR('J')   PORT_CHAR(0x0A)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_NAME("'")         PORT_CHAR('\'')  PORT_CHAR('"')
	PORT_START("ROW5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_NAME("Space")     PORT_CHAR(' ')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_NAME("X")         PORT_CHAR('x')   PORT_CHAR('X')   PORT_CHAR(0x18)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP)       PORT_NAME("<|||")      PORT_CHAR(UCHAR_MAMEKEY(PGUP))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_NAME("/")         PORT_CHAR('/')   PORT_CHAR('?')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_NAME("V")         PORT_CHAR('v')   PORT_CHAR('V')   PORT_CHAR(0x16)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_NAME(",")         PORT_CHAR(',')   PORT_CHAR('<')   PORT_CHAR(0x1C)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)         PORT_NAME("SYM")       PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_START("ROW6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_NAME("N")         PORT_CHAR('n')   PORT_CHAR('N')   PORT_CHAR(0x0E)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_NAME("C")         PORT_CHAR('c')   PORT_CHAR('C')   PORT_CHAR(0x03)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN)       PORT_NAME("|||>")      PORT_CHAR(UCHAR_MAMEKEY(PGDN))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_NAME(".")         PORT_CHAR('.')   PORT_CHAR('>')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_NAME("B")         PORT_CHAR('b')   PORT_CHAR('B')   PORT_CHAR(0x02)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_NAME("Z")         PORT_CHAR('z')   PORT_CHAR('Z')   PORT_CHAR(0x1A)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_NAME("M")         PORT_CHAR('m')   PORT_CHAR('M')   PORT_CHAR(0x0D)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("ROW7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_NAME("H")         PORT_CHAR('h')   PORT_CHAR('H')   PORT_CHAR(0x08)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_NAME("Y")         PORT_CHAR('y')   PORT_CHAR('Y')   PORT_CHAR(0x19)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)       PORT_NAME("Down")      PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_NAME(";")         PORT_CHAR(';')   PORT_CHAR(':')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_NAME("S")         PORT_CHAR('s')   PORT_CHAR('S')   PORT_CHAR(0x13)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_NAME("F")         PORT_CHAR('f')   PORT_CHAR('F')   PORT_CHAR(0x06)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_NAME("K")         PORT_CHAR('k')   PORT_CHAR('K')   PORT_CHAR(0x0B)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_NAME("Go")        PORT_CHAR(0x0D)

	PORT_START("MODIFIERS")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)   PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("Control")   PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)   PORT_CODE(KEYCODE_CAPSLOCK)                             PORT_NAME("Caps Lock") PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)   PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)     PORT_NAME("Shift")     PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	// Joystick Ports
	PORT_START("JOYSTICK1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_START("JOYSTICK2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_START("JOYSTICK3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_START("JOYSTICK4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)

	// Analog Paddles
	PORT_START("PADDLE1")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_PLAYER(1) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_MINMAX(0, 255) // pin 9
	PORT_START("PADDLE2")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_PLAYER(2) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_MINMAX(0, 255) // pin 5
	PORT_START("PADDLE3")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_PLAYER(3) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_MINMAX(0, 255) // pin 9
	PORT_START("PADDLE4")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_PLAYER(4) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_MINMAX(0, 255) // pin 5
	PORT_START("PADDLE5")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_PLAYER(5) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_MINMAX(0, 255) // pin 9
	PORT_START("PADDLE6")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_PLAYER(6) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_MINMAX(0, 255) // pin 5
	PORT_START("PADDLE7")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_PLAYER(7) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_MINMAX(0, 255) // pin 9
	PORT_START("PADDLE8")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_PLAYER(8) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_MINMAX(0, 255) // pin 5
INPUT_PORTS_END

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NABUPC_KEYBOARD, nabupc_keyboard_device, "nabupc_keyboard", "NABU PC keyboard")


//**************************************************************************
//  KEYBOARD DEVICE
//**************************************************************************

//-------------------------------------------------
//  nabupc_keyboard_device - constructor
//-------------------------------------------------

nabupc_keyboard_device::nabupc_keyboard_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NABUPC_KEYBOARD, tag, owner, clock)
	, m_mcu(*this, "mcu")
	, m_adc(*this, "adc")
	, m_rxd_cb(*this)
	, m_modifiers(*this, "MODIFIERS")
	, m_keyboard(*this, "ROW%u", 0U)
	, m_gameport(*this, "JOYSTICK%u", 1U)
	, m_port1(0)
	, m_eoc(0)
{
}

void nabupc_keyboard_device::device_start()
{
	save_item(NAME(m_port1));
	save_item(NAME(m_eoc));
}

void nabupc_keyboard_device::device_reset()
{
	m_port1 &= 0x7f;
	m_rxd_cb(1);
}

void nabupc_keyboard_device::device_add_mconfig(machine_config &config)
{
	M6803(config, m_mcu, 3.579545_MHz_XTAL); // Crystal verified from schematics and visual inspection
	m_mcu->set_addrmap(AS_PROGRAM, &nabupc_keyboard_device::nabu_kb_mem);
	m_mcu->in_p1_cb().set(FUNC(nabupc_keyboard_device::port1_r));
	m_mcu->out_p1_cb().set(FUNC(nabupc_keyboard_device::port1_w));
	m_mcu->out_ser_tx_cb().set(FUNC(nabupc_keyboard_device::ser_tx_w));

	ADC0809(config, m_adc, 3.579545_MHz_XTAL / 4);
	m_adc->eoc_callback().set(FUNC(nabupc_keyboard_device::irq_w));
	m_adc->in_callback<0>().set_ioport("PADDLE1");
	m_adc->in_callback<1>().set_ioport("PADDLE2");
	m_adc->in_callback<2>().set_ioport("PADDLE3");
	m_adc->in_callback<3>().set_ioport("PADDLE4");
	m_adc->in_callback<4>().set_ioport("PADDLE5");
	m_adc->in_callback<5>().set_ioport("PADDLE6");
	m_adc->in_callback<6>().set_ioport("PADDLE7");
	m_adc->in_callback<7>().set_ioport("PADDLE8");
}

void nabupc_keyboard_device::nabu_kb_mem(address_map &map)
{
	map(0x5000, 0x5300).r(FUNC(nabupc_keyboard_device::gameport_r));
	map(0x7000, 0x7000).w(FUNC(nabupc_keyboard_device::adc_start_w));
	map(0x9000, 0x9000).w(FUNC(nabupc_keyboard_device::cpu_ack_irq_w));
	map(0xb000, 0xb700).w(FUNC(nabupc_keyboard_device::adc_latch_w));
	map(0xd000, 0xd000).r(m_adc, FUNC(adc0809_device::data_r));
	map(0xf800, 0xffff).rom().region("mcu", 0);
}

const tiny_rom_entry *nabupc_keyboard_device::device_rom_region() const
{
	return ROM_NAME(nabu_keyboard_rom);
}

ioport_constructor nabupc_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( keyboard_ports );
}

uint8_t nabupc_keyboard_device::port1_r()
{
	uint8_t data;
	uint8_t column_mask = 1 << (m_port1 & 7);
	uint8_t row = (m_port1 >> 3) & 7;

	if (m_port1 & 0x40) {
		data = m_modifiers->read();
	} else {
		data = m_keyboard[row]->read();
	}

	if (data & column_mask) {
		return 0x80 | m_port1;
	}

	return m_port1;
}

void nabupc_keyboard_device::port1_w(uint8_t data)
{
	m_port1 = data & 0x7f;
}

void nabupc_keyboard_device::irq_w(uint8_t data)
{
	if (data && !m_eoc) {
		m_mcu->set_input_line(0, ASSERT_LINE);
	}
	m_eoc = data;
}

uint8_t nabupc_keyboard_device::gameport_r(offs_t offset)
{
	uint8_t port = (offset >> 8) & 0x03;
	return m_gameport[port]->read();
}

void nabupc_keyboard_device::adc_latch_w(offs_t offset, uint8_t data)
{
	uint8_t addr = (offset >> 8) & 0x07;
	m_adc->address_w(addr);
}

void nabupc_keyboard_device::adc_start_w(offs_t offset, uint8_t data)
{
	m_adc->start_w(1);
	m_adc->start_w(0);
}

void nabupc_keyboard_device::cpu_ack_irq_w(offs_t offset, uint8_t data)
{
	m_mcu->set_input_line(0, CLEAR_LINE);
}

void nabupc_keyboard_device::ser_tx_w(uint8_t data)
{
	m_rxd_cb(data & 1);
}
