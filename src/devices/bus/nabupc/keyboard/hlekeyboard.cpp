// license:BSD-3-Clause
// copyright-holders:Brian Johnson
/*****************************************************************************

    NABU PC HLE Keyboard Interface

 *****************************************************************************/

#include "emu.h"
#include "hlekeyboard.h"

#include "machine/keyboard.h"
#include "machine/keyboard.ipp"

#include "diserial.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class hle_keyboard_device
	: public device_t
	, public device_buffered_serial_interface<16U>
	, public device_rs232_port_interface
	, protected device_matrix_keyboard_interface<16U>
{
public:
	// constructor/destructor
	hle_keyboard_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	// device overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual ioport_constructor device_input_ports() const override;

	// device_buffered_serial_interface overrides
	virtual void tra_callback() override;

	// device_matrix_keyboard_interface overrides
	virtual void key_make(uint8_t row, uint8_t column) override;
	virtual void key_break(uint8_t row, uint8_t column) override;
	virtual void scan_complete() override;

	uint8_t translate(uint8_t row, uint8_t column);

private:
	static constexpr int START_BIT_COUNT = 1;
	static constexpr int DATA_BIT_COUNT = 8;
	static constexpr device_serial_interface::parity_t PARITY = device_serial_interface::PARITY_NONE;
	static constexpr device_serial_interface::stop_bits_t STOP_BITS = device_serial_interface::STOP_BITS_1;
	static constexpr int BAUD = 6'993;

	TIMER_CALLBACK_MEMBER(watchdog_tick);

	virtual void received_byte(uint8_t byte) override {}

	required_ioport m_modifiers;
	required_ioport m_gameport1;
	required_ioport m_gameport2;

	emu_timer *m_watchdog_timer;
	uint8_t m_prev_gameport1;
	uint8_t m_prev_gameport2;
};

namespace {
//**************************************************************************
//  TRANSLATION TABLE
//**************************************************************************

	uint8_t const TRANSLATION_TABLE[3][16][4] =
	{
		// unshifted
		{
			{ 0x00, 0x38, 0x67, 0x6e }, // 0
			{ 0x1b, 0x32, 0x09, 0x63 }, // 1
			{ 0xe0, 0xe2, 0xe6, 0xe4 }, // 2
			{ 0x70, 0x3d, 0x6c, 0x2e }, // 3
			{ 0x77, 0x34, 0x61, 0x62 }, // 4
			{ 0x72, 0x36, 0x64, 0x7a }, // 5
			{ 0x69, 0x30, 0x6a, 0x6d }, // 6
			{ 0x5d, 0xea, 0x27, 0x00 }, // 7
			{ 0x75, 0x37, 0x20, 0x68 }, // 8
			{ 0x71, 0x31, 0x78, 0x79 }, // 9
			{ 0xe1, 0xe7, 0xe5, 0xe3 }, // 10
			{ 0x5b, 0x2d, 0x2f, 0x3b }, // 11
			{ 0x65, 0x33, 0x76, 0x73 }, // 12
			{ 0x74, 0x35, 0x00, 0x66 }, // 13
			{ 0x6f, 0x39, 0x2c, 0x6b }, // 14
			{ 0x7f, 0xe9, 0xe8, 0x0d }, // 15
		},
		// shifted
		{
			{ 0x00, 0x2a, 0x47, 0x4e }, // 0
			{ 0x1b, 0x40, 0x09, 0x43 }, // 1
			{ 0xe0, 0xe2, 0xe6, 0xe4 }, // 2
			{ 0x50, 0x2b, 0x4c, 0x3e }, // 3
			{ 0x57, 0x24, 0x41, 0x42 }, // 4
			{ 0x52, 0x5e, 0x44, 0x5a }, // 5
			{ 0x49, 0x29, 0x4a, 0x4d }, // 6
			{ 0x7d, 0xea, 0x22, 0x00 }, // 7
			{ 0x55, 0x26, 0x20, 0x48 }, // 8
			{ 0x51, 0x21, 0x58, 0x59 }, // 9
			{ 0xe1, 0xe7, 0xe5, 0xe3 }, // 10
			{ 0x5b, 0x5f, 0x3f, 0x3a }, // 11
			{ 0x45, 0x23, 0x56, 0x53 }, // 12
			{ 0x54, 0x25, 0x00, 0x46 }, // 13
			{ 0x4f, 0x28, 0x3c, 0x4b }, // 14
			{ 0x7f, 0xe9, 0xe8, 0x0d }, // 15
		},
		// control
		{
			{ 0x00, 0x38, 0x07, 0x0e }, // 0
			{ 0x1b, 0x00, 0x09, 0x03 }, // 1
			{ 0xe0, 0xe2, 0xe6, 0xe4 }, // 2
			{ 0x10, 0x3d, 0x0c, 0x2e }, // 3
			{ 0x17, 0x34, 0x01, 0x02 }, // 4
			{ 0x12, 0x1e, 0x04, 0x1a }, // 5
			{ 0x09, 0x30, 0x0a, 0x0d }, // 6
			{ 0x1d, 0xea, 0x27, 0x00 }, // 7
			{ 0x15, 0x37, 0x20, 0x08 }, // 8
			{ 0x11, 0x31, 0x18, 0x19 }, // 9
			{ 0xe1, 0xe7, 0xe5, 0xe3 }, // 10
			{ 0x1b, 0x1f, 0x2f, 0x3b }, // 11
			{ 0x05, 0x33, 0x16, 0x13 }, // 12
			{ 0x14, 0x35, 0x00, 0x06 }, // 13
			{ 0x0f, 0x39, 0x1c, 0x0b }, // 14
			{ 0x7f, 0xe9, 0xe8, 0x0d }, // 15
		}
	};

//**************************************************************************
//  KEYBOARD MATRIX
//**************************************************************************

INPUT_PORTS_START(nabu_hle_keyboard)
	PORT_START("MODIFIERS")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)   PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("Control")   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)   PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)     PORT_NAME("Shift")     PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)   PORT_CODE(KEYCODE_CAPSLOCK)                             PORT_NAME("Caps Lock") PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_START("ROW0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED) // "Alt Mode"
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)                                 PORT_CHAR('8')  PORT_CHAR('*')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)                                 PORT_CHAR('g')  PORT_CHAR('G')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)                                 PORT_CHAR('n')  PORT_CHAR('N')
	PORT_START("ROW1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)        PORT_NAME("Esc")       PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)                                 PORT_CHAR('2')  PORT_CHAR('2')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)        PORT_NAME("Tab")       PORT_CHAR(9)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)                                 PORT_CHAR('c')  PORT_CHAR('C')
	PORT_START("ROW2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)       PORT_NAME("Right")    PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)          PORT_NAME("Up")       PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)          PORT_NAME("No")       PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN)        PORT_NAME("|||>")     PORT_CHAR(UCHAR_MAMEKEY(PGDN))
	PORT_START("ROW3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)                                 PORT_CHAR('p')  PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)                            PORT_CHAR('=')  PORT_CHAR('+')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)                                 PORT_CHAR('l')  PORT_CHAR('L')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)                              PORT_CHAR('.')  PORT_CHAR('>')
	PORT_START("ROW4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)                                 PORT_CHAR('w')  PORT_CHAR('W')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)                                 PORT_CHAR('4')  PORT_CHAR('$')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)                                 PORT_CHAR('a')  PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)                                 PORT_CHAR('b')  PORT_CHAR('B')
	PORT_START("ROW5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)                                 PORT_CHAR('r')  PORT_CHAR('R')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)                                 PORT_CHAR('6')  PORT_CHAR('^')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)                                 PORT_CHAR('d')  PORT_CHAR('D')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)                                 PORT_CHAR('z')  PORT_CHAR('Z')
	PORT_START("ROW6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)                                 PORT_CHAR('i')  PORT_CHAR('I')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)                                 PORT_CHAR('0')  PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)                                 PORT_CHAR('j')  PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)                                 PORT_CHAR('m')  PORT_CHAR('M')
	PORT_START("ROW7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)                        PORT_CHAR(']')  PORT_CHAR('}')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)         PORT_NAME("TV")        PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)                             PORT_CHAR('\'')  PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED) // "Alt Mode"
	PORT_START("ROW8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)                                 PORT_CHAR('u')  PORT_CHAR('U')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)                                 PORT_CHAR('7')  PORT_CHAR('&')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)                             PORT_CHAR(' ')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)                                 PORT_CHAR('h')  PORT_CHAR('H')
	PORT_START("ROW9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)                                 PORT_CHAR('q')  PORT_CHAR('Q')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)                                 PORT_CHAR('1')  PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)                                 PORT_CHAR('x')  PORT_CHAR('X')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)                                 PORT_CHAR('y')  PORT_CHAR('Y')
	PORT_START("ROWA")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)       PORT_NAME("Left")      PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)         PORT_NAME("Yes")       PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP)       PORT_NAME("<|||")      PORT_CHAR(UCHAR_MAMEKEY(PGUP))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)       PORT_NAME("Down")      PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_START("ROWB")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)                         PORT_CHAR('[')  PORT_CHAR('{')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)                             PORT_CHAR('-')  PORT_CHAR('_')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)                             PORT_CHAR('/')  PORT_CHAR('?')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)                             PORT_CHAR(';')  PORT_CHAR(':')
	PORT_START("ROWC")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)                                 PORT_CHAR('e')  PORT_CHAR('E')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)                                 PORT_CHAR('3')  PORT_CHAR('#')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)                                 PORT_CHAR('v')  PORT_CHAR('V')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)                                 PORT_CHAR('s')  PORT_CHAR('S')
	PORT_START("ROWD")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)                                 PORT_CHAR('t')  PORT_CHAR('T')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)                                 PORT_CHAR('5')  PORT_CHAR('%')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED) // "Alt Mode"
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)                                 PORT_CHAR('f')  PORT_CHAR('F')
	PORT_START("ROWE")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)                                 PORT_CHAR('o')  PORT_CHAR('O')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)                                 PORT_CHAR('9')  PORT_CHAR('(')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)                             PORT_CHAR(',')  PORT_CHAR('<')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)                                 PORT_CHAR('k')  PORT_CHAR('K')
	PORT_START("ROWF")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_NAME("Delete")    PORT_CHAR(UCHAR_MAMEKEY(BACKSPACE))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PAUSE)      PORT_NAME("Pause")     PORT_CHAR(UCHAR_MAMEKEY(PAUSE))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)         PORT_NAME("SYM")       PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_NAME("Go")        PORT_CHAR(13)

	PORT_START("JOYSTICK1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )                        PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )                        PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )                       PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )                          PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )                              PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("JOYSTICK2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)         PORT_CODE(INPUT_CODE_INVALID)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)         PORT_CODE(INPUT_CODE_INVALID)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)        PORT_CODE(INPUT_CODE_INVALID)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)           PORT_CODE(INPUT_CODE_INVALID)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)               PORT_CODE(INPUT_CODE_INVALID)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )
INPUT_PORTS_END
}

//**************************************************************************
//  KEYBOARD DEVICE
//**************************************************************************

//-------------------------------------------------
//  keyboard_device - constructor
//-------------------------------------------------
hle_keyboard_device::hle_keyboard_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NABUPC_HLE_KEYBOARD, tag, owner, clock)
	, device_buffered_serial_interface(mconfig, *this)
	, device_rs232_port_interface(mconfig, *this)
	, device_matrix_keyboard_interface(mconfig, *this, "ROW0", "ROW1", "ROW2", "ROW3", "ROW4", "ROW5", "ROW6", "ROW7", "ROW8", "ROW9", "ROWA", "ROWB", "ROWC", "ROWD", "ROWE", "ROWF")
	, m_modifiers(*this, "MODIFIERS")
	, m_gameport1(*this, "JOYSTICK1")
	, m_gameport2(*this, "JOYSTICK2")
	, m_watchdog_timer(nullptr)
{
}

ioport_constructor hle_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(nabu_hle_keyboard);
}

void hle_keyboard_device::device_start()
{
	m_watchdog_timer = timer_alloc(FUNC(hle_keyboard_device::watchdog_tick), this);

	save_item(NAME(m_prev_gameport1));
	save_item(NAME(m_prev_gameport2));
}

void hle_keyboard_device::device_reset()
{
	m_prev_gameport1 = m_prev_gameport2 = 0xa0;

	// initialise state
	clear_fifo();

	// configure device_buffered_serial_interface
	set_data_frame(START_BIT_COUNT, DATA_BIT_COUNT, PARITY, STOP_BITS);
	set_rate(BAUD);
	receive_register_reset();
	transmit_register_reset();

	m_watchdog_timer->adjust(attotime::from_msec(500), 1, attotime::never);

	// kick the base
	reset_key_state();
	start_processing(attotime::from_hz(BAUD));
}

TIMER_CALLBACK_MEMBER(hle_keyboard_device::watchdog_tick)
{
	if (param == 1) {
		transmit_byte(0x95);
		m_watchdog_timer->adjust(attotime::from_msec(500), 0, attotime::from_msec(3700));
	} else {
		transmit_byte(0x94);
	}
}

void hle_keyboard_device::tra_callback()
{
	output_rxd(transmit_register_get_data_bit());
}

//**************************************************************************
//  KEYBOARD SCANNING
//**************************************************************************

void hle_keyboard_device::key_make(uint8_t row, uint8_t column)
{
	transmit_byte(translate(row,column));
}

void hle_keyboard_device::key_break(uint8_t row, uint8_t column)
{
	uint8_t key = translate(row, column);
	if ((key & 0xe0) == 0xe0) {
		transmit_byte(key | 0x10);
	}
}

uint8_t hle_keyboard_device::translate(uint8_t row, uint8_t column)
{
	uint8_t const modifiers(m_modifiers->read());

	bool const ctrl(modifiers & 0x01);
	bool const shift(bool(modifiers & 0x02));
	unsigned const map(ctrl ? 2 : shift ? 1 : 0);
	uint8_t scancode = TRANSLATION_TABLE[map][row][column];

	if ((scancode >= 0x61 && scancode <= 0x7a) && (modifiers & 0x04)) {
		scancode -= 0x20;
	}

	return scancode;
}

void hle_keyboard_device::scan_complete()
{
	uint8_t gameport1 = m_gameport1->read();
	uint8_t gameport2 = m_gameport2->read();

	if (gameport1 != m_prev_gameport1) {
		m_prev_gameport1 = gameport1;
		transmit_byte(0x80);
		transmit_byte(gameport1);
	}

	if (gameport2 != m_prev_gameport2) {
		m_prev_gameport2 = gameport2;
		transmit_byte(0x81);
		transmit_byte(gameport2);
	}
}

}  // anonymous namespace

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(NABUPC_HLE_KEYBOARD, device_rs232_port_interface, hle_keyboard_device, "nabu_hle_keyboard", "NABU PC Keyboard (HLE)")
