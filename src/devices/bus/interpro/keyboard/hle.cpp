// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * A high level emulation implementation of the Intergraph InterPro keyboard,
 * largely copied from the sunkbd and psi hle implementations.
 *
 * These keyboards have two primary banks of keys. The lower bank consists of
 * a total of 67 regular keyboard keyswitches plus a numeric keypad with a
 * further 18 keys. The upper bank consists of 57 membrane-style programmable
 * function keys in groups of 9, 36 and 12 from left to right.
 *
 * The following describes the key labels and positions according to the
 * standard US English keyboard layout. At least a German keyboard variant is
 * known to have existed.
 *
 * Upper bank keys indicated here with asterisks are printed in white, as are
 * all the A*, B* and C* keys; all the others are printed in brown.
 *
 *    Setup*  Home*   2nd             A1      A2      A3      A4      A5      A6      A7      A8      A9      A10     A11     A12          A13     A14     A15     2nd
 *    Help    Clear   F*                                                                                                                                           F*
 *            Screen
 *
 *    Find    Insert  Print           B1      B2      B3      B4      B5      B6      B7      B8      B9      B10     B11     B12          B13     B14     B15     B16
 *            Here    Screen*
 *                    Remove
 *
 *    Select  Prev    Next            C1      C2      C3      C4      C5      C6      C7      C8      C9      C10     C11     C12          C13     C14     C15     C16
 *            Screen  Screen
 *
 *
 * In between the banks on the right hand side, there is a row of LEDs, the
 * first three are pictures, rather than the descriptions given here).
 *
 *                                                                                                                                         Disk Lock ----- L1 L2 L3 L4
 *
 * Lower bank keys have up to 3 labels, in shifted and unshifted positions, and
 * in red on the front face of the key-cap.
 *
 *    Esc     ~       !       @       #       $       %       ^       &       *       (       )       _       +       Back    Delete       PF1     PF2     PF3     PF4
 *            `       1       2       3       4       5       6       7       8       9       0       -       =       Space                ±       ÷       ×       +
 *                                                                                                                                         Esc     Num Lk  ScrlLk  Sys
 *
 *    Alt     Tab         Q       W       E       R       T       Y       U       I       O       P       {       }                        7       8       9       _
 *    Mode                                                                                                [       ]
 *                                                                                                                                         Home    ↑       Pg Up   Prt Sc
 *
 *    Ctrl    Caps          A       S       D       F       G       H       J       K       L       :       "       |         Return       4       5       6       ,
 *            Lock
 *                                                                                                                                         ←               →       −
 *
 *            Shift     >       Z       X       C       V       B       N       M       ,       .       ?       Shift         ▲            1       2       3       =
 *                      <                                                                                                     ■
 *                                                                                                                                         End     ↓       Pg Dn
 *
 *    Hold       Super-   Line                                                                          Repeat        ◄       ■       ►    0               ◦
 *    Screen     impose   Feed                                                                                                ▼                            .       Enter
 *                                                                                                                                         Ins             Del     +
 *
 * Alt Mode and Caps Lock keys have locking switches, capturing the key in the
 * depressed position, as well as physical leds visible on the keycaps
 * themselves. The keyboard also has two physical buttons on the back face of
 * the keyboard, circular button labelled Boot, and a square one labelled Reset.
 *
 * The keyboard uses a 1200bps serial protocol to communicate with the host,
 * with 1 start bit, 8 data bits, 1 stop bit, and even parity. The protocol
 * as far as is known consists of the following sequences:
 *
 *    From Host       Purpose      Expected Response
 *    ---------       -------      -----------------
 *    0x1b 0x42 <xx>  probably controls several functions:
 *
 *              0x01  sound bell?
 *              0x04  keyclick on
 *              0x08  membrane click on
 *              0x20  bell tone loud/soft/none? (seems only loud/none)
 *
 *    0x1b 0x44       reset/diag   0xff <status>, where status bits are set to
 *                                 indicate diagnostic failure source: 0x8=EPROM
 *                                 checksum, 0x10=RAM error, 0x20=ROM checksum
 *
 *    0x1b 0x55       activate scan code keyup/down mode?
 *
 * The keyboard has a keyboard click function, and the LED indicators described
 * earlier, meaning that there are additional commands to enable and disable
 * these functions.
 *
 * The keyboard has at least two operating modes: a simple ASCII mode which is
 * active after reset, and a scancode mode generating make/break codes with
 * full support for all the qualifiers and other non-ASCII keys.
 *
 * In ASCII mode, the keyboard transmits ASCII codes corresponding to the key
 * labels for keys which map to the ASCII character set. Modifiers are applied
 * by the keyboard itself, and do not generate make/break codes of their own.
 *
 * The following non-ASCII sequences are recognised in the system software,
 * and likely correspond to specific keyboard keys or buttons:
 *
 *    Sequence    Function
 *    --------    --------
 *    <esc> ^L    Reboot, possibly maps to Reboot button
 *    <esc> ^M    Unknown
 *    <esc> ^N    Unknown, but operates as a toggle
 *    <esc> ^U    Unknown
 *
 * TODO
 *   - unmapped keys
 *   - auto-repeat
 *   - key click and LED commands
 *   - alternative layouts
 *   - scancode make/break mode
 *
 */
#include "emu.h"
#include "hle.h"

#include "machine/keyboard.ipp"
#include "speaker.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE_NS(INTERPRO_HLE_EN_US_KEYBOARD, bus::interpro::keyboard, hle_en_us_device, "kbd_hle_en_us", "InterPro Keyboard (HLE, US English)")

namespace bus { namespace interpro { namespace keyboard {

namespace {

	u8 const TRANSLATION_TABLE[4][5][16] =
	{
		// unshifted
		{
			{ 0x1b, 0x60, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x2d, 0x3d, 0x08, 0x7f }, // 0
			{ 0x00, 0x09, 0x71, 0x77, 0x65, 0x72, 0x74, 0x79, 0x75, 0x69, 0x6f, 0x70, 0x5b, 0x5d, 0x00, 0x00 }, // 1
			{ 0x00, 0x00, 0x61, 0x73, 0x64, 0x66, 0x67, 0x68, 0x6a, 0x6b, 0x6c, 0x3b, 0x27, 0x5c, 0x0d, 0x00 }, // 2
			{ 0x00, 0x3c, 0x7a, 0x78, 0x63, 0x76, 0x62, 0x6e, 0x6d, 0x2c, 0x2e, 0x2f, 0x00, 0x00, 0x00, 0x00 }, // 3
			{ 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 4
		},
		// shifted
		{
			{ 0x1b, 0x7e, 0x21, 0x40, 0x23, 0x24, 0x25, 0x5e, 0x26, 0x2a, 0x28, 0x29, 0x5f, 0x2b, 0x08, 0x7f }, // 0
			{ 0x00, 0x09, 0x51, 0x57, 0x45, 0x52, 0x54, 0x59, 0x55, 0x49, 0x4f, 0x50, 0x7b, 0x7d, 0x00, 0x00 }, // 1
			{ 0x00, 0x00, 0x41, 0x53, 0x44, 0x46, 0x47, 0x48, 0x4a, 0x4b, 0x4c, 0x3a, 0x22, 0x7c, 0x0d, 0x00 }, // 2
			{ 0x00, 0x3e, 0x5a, 0x58, 0x43, 0x56, 0x42, 0x4e, 0x4d, 0x2c, 0x2e, 0x3f, 0x00, 0x00, 0x00, 0x00 }, // 3
			{ 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 4
		},
		// unshifted-control
		{
			{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0
			{ 0x00, 0x00, 0x11, 0x17, 0x05, 0x12, 0x14, 0x19, 0x15, 0x09, 0x0f, 0x10, 0x1b, 0x1d, 0x00, 0x00 }, // 1
			{ 0x00, 0x00, 0x01, 0x13, 0x04, 0x06, 0x07, 0x08, 0x0a, 0x0b, 0x0c, 0x00, 0x00, 0x1c, 0x00, 0x00 }, // 2
			{ 0x00, 0x00, 0x1a, 0x18, 0x03, 0x16, 0x02, 0x0e, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 3
			{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 4
		},
		// shifted-control
		{
			{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0
			{ 0x00, 0x00, 0x11, 0x17, 0x05, 0x12, 0x14, 0x19, 0x15, 0x09, 0x0f, 0x10, 0x1b, 0x1d, 0x00, 0x00 }, // 1
			{ 0x00, 0x00, 0x01, 0x13, 0x04, 0x06, 0x07, 0x08, 0x0a, 0x0b, 0x0c, 0x00, 0x00, 0x1c, 0x00, 0x00 }, // 2
			{ 0x00, 0x00, 0x1a, 0x18, 0x03, 0x16, 0x02, 0x0e, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 3
			{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 4
		}
	};

INPUT_PORTS_START(interpro_en_us)

	PORT_START("modifiers")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)   PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("Control")   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)   PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)     PORT_NAME("Shift")     PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)   PORT_CODE(KEYCODE_CAPSLOCK)                             PORT_NAME("Caps Lock") PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))

	PORT_START("row_0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)        PORT_NAME("Esc")       PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)                             PORT_CHAR('`')  PORT_CHAR('~')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)                                 PORT_CHAR('1')  PORT_CHAR('!')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)                                 PORT_CHAR('2')  PORT_CHAR('@')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)                                 PORT_CHAR('3')  PORT_CHAR('#')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)                                 PORT_CHAR('4')  PORT_CHAR('$')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)                                 PORT_CHAR('5')  PORT_CHAR('%')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)                                 PORT_CHAR('6')  PORT_CHAR('^')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)                                 PORT_CHAR('7')  PORT_CHAR('&')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)                                 PORT_CHAR('8')  PORT_CHAR('*')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)                                 PORT_CHAR('9')  PORT_CHAR('(')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)                                 PORT_CHAR('0')  PORT_CHAR(')')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)                             PORT_CHAR('-')  PORT_CHAR('_')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)                            PORT_CHAR('=')  PORT_CHAR('+')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_NAME("Backspace") PORT_CHAR(8)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)        PORT_NAME("Delete")    PORT_CHAR(UCHAR_MAMEKEY(DEL))

	PORT_START("row_1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED) // "Alt Mode"
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)        PORT_NAME("Tab")       PORT_CHAR(9)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)                                 PORT_CHAR('q')  PORT_CHAR('Q')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)                                 PORT_CHAR('w')  PORT_CHAR('W')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)                                 PORT_CHAR('e')  PORT_CHAR('E')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)                                 PORT_CHAR('r')  PORT_CHAR('R')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)                                 PORT_CHAR('t')  PORT_CHAR('T')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)                                 PORT_CHAR('y')  PORT_CHAR('Y')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)                                 PORT_CHAR('u')  PORT_CHAR('U')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)                                 PORT_CHAR('i')  PORT_CHAR('I')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)                                 PORT_CHAR('o')  PORT_CHAR('O')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)                                 PORT_CHAR('p')  PORT_CHAR('P')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)                         PORT_CHAR('[')  PORT_CHAR('{')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)                        PORT_CHAR(']')  PORT_CHAR('}')

	PORT_START("row_2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED) // CTRL
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_UNUSED) // LOCK
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)                                 PORT_CHAR('a')  PORT_CHAR('A')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)                                 PORT_CHAR('s')  PORT_CHAR('S')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)                                 PORT_CHAR('d')  PORT_CHAR('D')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)                                 PORT_CHAR('f')  PORT_CHAR('F')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)                                 PORT_CHAR('g')  PORT_CHAR('G')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)                                 PORT_CHAR('h')  PORT_CHAR('H')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)                                 PORT_CHAR('j')  PORT_CHAR('J')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)                                 PORT_CHAR('k')  PORT_CHAR('K')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)                                 PORT_CHAR('l')  PORT_CHAR('L')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)                             PORT_CHAR(';')  PORT_CHAR(':')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)                             PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)                         PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_NAME("Return")    PORT_CHAR(13)

	PORT_START("row_3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED) // LSHIFT
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2)                        PORT_CHAR('<')  PORT_CHAR('>')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)                                 PORT_CHAR('z')  PORT_CHAR('Z')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)                                 PORT_CHAR('x')  PORT_CHAR('X')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)                                 PORT_CHAR('c')  PORT_CHAR('C')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)                                 PORT_CHAR('v')  PORT_CHAR('V')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)                                 PORT_CHAR('b')  PORT_CHAR('B')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)                                 PORT_CHAR('n')  PORT_CHAR('N')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)                                 PORT_CHAR('m')  PORT_CHAR('M')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)                             PORT_CHAR(',')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)                              PORT_CHAR('.')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)                             PORT_CHAR('/')  PORT_CHAR('?')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_UNUSED) // RSHIFT
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)          PORT_NAME("Up")       PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("row_4")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED) // "Hold Screen"
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_UNUSED) // "Superimpose"
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_UNUSED) // "Line Feed"
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)       PORT_NAME("Space")    PORT_CHAR(UCHAR_MAMEKEY(SPACE))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNUSED) // "Repeat"
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)        PORT_NAME("Left")     PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)        PORT_NAME("Down")     PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)       PORT_NAME("Right")    PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

INPUT_PORTS_END

INPUT_PORTS_START(hle_en_us_device)
	PORT_INCLUDE(interpro_en_us)
INPUT_PORTS_END

} // anonymous namespace

hle_device_base::hle_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_buffered_serial_interface(mconfig, *this)
	, device_interpro_keyboard_port_interface(mconfig, *this)
	, device_matrix_keyboard_interface(mconfig, *this, "row_0", "row_1", "row_2", "row_3", "row_4")
	, m_click_timer(nullptr)
	, m_beeper(*this, "beeper")
	, m_make_count(0U)
	, m_rx_state(RX_IDLE)
	, m_keyclick(0U)
	, m_beeper_state(0U)
{
}

hle_device_base::~hle_device_base()
{
}

WRITE_LINE_MEMBER(hle_device_base::input_txd)
{
	device_buffered_serial_interface::rx_w(state);
}

void hle_device_base::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "bell").front_center();
	BEEP(config, m_beeper, ATTOSECONDS_TO_HZ(480 * ATTOSECONDS_PER_MICROSECOND)).add_route(ALL_OUTPUTS, "bell", 1.0);
}

void hle_device_base::device_start()
{
	m_click_timer = timer_alloc(CLICK_TIMER_ID);

	save_item(NAME(m_make_count));
	save_item(NAME(m_rx_state));
	save_item(NAME(m_keyclick));
	save_item(NAME(m_beeper_state));
}

void hle_device_base::device_reset()
{
	// initialise state
	clear_fifo();
	m_make_count = 0U;
	m_rx_state = RX_IDLE;
	m_keyclick = 0U;
	m_beeper_state = 0x00U;

	// configure device_buffered_serial_interface
	set_data_frame(START_BIT_COUNT, DATA_BIT_COUNT, PARITY, STOP_BITS);
	set_rate(BAUD);
	receive_register_reset();
	transmit_register_reset();

	// no beep
	m_click_timer->reset();

	// kick the base
	reset_key_state();
	start_processing(attotime::from_hz(1'200));
}

void hle_device_base::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case CLICK_TIMER_ID:
		m_beeper_state &= ~u8(BEEPER_CLICK);
		m_beeper->set_state(m_beeper_state ? 1 : 0);
		break;

	default:
		break;
	}
}

void hle_device_base::tra_callback()
{
	output_rxd(transmit_register_get_data_bit());
}

void hle_device_base::tra_complete()
{
	if (fifo_full())
		start_processing(attotime::from_hz(1'200));

	device_buffered_serial_interface::tra_complete();
}

void hle_device_base::key_make(u8 row, u8 column)
{
	// we should have stopped processing if we filled the FIFO
	assert(!fifo_full());

	// send the make code, click if desired
	transmit_byte(translate(row, column));
	if (m_keyclick)
	{
		m_beeper_state |= u8(BEEPER_CLICK);
		m_beeper->set_state(m_beeper_state ? 1 : 0);
		m_click_timer->reset(attotime::from_msec(5));
	}

	// count keys
	++m_make_count;
	assert(m_make_count);
}

void hle_device_base::key_break(u8 row, u8 column)
{
	// we should have stopped processing if we filled the FIFO
	assert(!fifo_full());
	assert(m_make_count);

	--m_make_count;

	// check our counting
	assert(are_all_keys_up() == !bool(m_make_count));
}

void hle_device_base::transmit_byte(u8 byte)
{
	LOG("transmit_byte 0x%02x\n", byte);
	device_buffered_serial_interface::transmit_byte(byte);
	if (fifo_full())
		stop_processing();
}

void hle_device_base::received_byte(u8 byte)
{
	LOG("received_byte 0x%02x\n", byte);

	switch (m_rx_state)
	{
	case RX_COMMAND:
		switch (byte)
		{
		case 0x42: // configure flags
			m_rx_state = RX_FLAGS;
			break;

		case 0x44: // reset/diagnostic
			transmit_byte(0xff);
			transmit_byte(0x00);

			m_rx_state = RX_IDLE;
			break;

		case 0x55:
			break;
		}
		break;

	case RX_FLAGS:
		// FIXME: this logic is wrong (should decode the various fields), but
		// generates bell sounds at the right time for now
		switch (byte)
		{
		case 0x28:
			LOG("bell on\n");
			m_beeper_state |= u8(BEEPER_BELL);
			m_beeper->set_state(m_beeper_state ? 1 : 0);
			break;

		case 0x29:
			LOG("bell off\n");
			m_beeper_state &= ~u8(BEEPER_BELL);
			m_beeper->set_state(m_beeper_state ? 1 : 0);
			break;

		default:
			break;
		}
		m_rx_state = RX_IDLE;
		break;

	case RX_IDLE:
		if (byte == 0x1b)
			m_rx_state = RX_COMMAND;
		break;
	}
}

hle_en_us_device::hle_en_us_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: hle_device_base(mconfig, INTERPRO_HLE_EN_US_KEYBOARD, tag, owner, clock),
	m_modifiers(*this, "modifiers")
{
}

ioport_constructor hle_en_us_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hle_en_us_device);
}

u8 hle_en_us_device::translate(u8 row, u8 column)
{
	u8 const modifiers(m_modifiers->read());

	bool const ctrl(modifiers & 0x01);
	bool const shift(bool(modifiers & 0x02) || (bool(modifiers & 0x04)));
	bool const ctrl_shift(ctrl && shift);

	unsigned const map(ctrl_shift ? 3 : ctrl ? 2 : shift ? 1 : 0);

	return TRANSLATION_TABLE[map][row][column];
}

} } } // namespace bus::interpro::keyboard
