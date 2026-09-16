// license:BSD-3-Clause
// copyright-holders: R. Belmont
/*********************************************************************

    tk3000.cpp
    Datadesk TK-3000 ADB keyboard with integrated trackball
    by R. Belmont

    Port 0: Keyboard matrix row readback during MOVX (active low).
    Port 1:
            bit 0: Trackball button 1 (mouse click)
            bit 1: Trackball auxiliary button A
            bit 2: Trackball auxiliary button B
                   (latching/momentary roles of P1.1 and P1.2 are
                    swapped by configuration-jumper bit 0; see P3.4)
            bit 3: X-axis quadrature phase A
            bit 4: X-axis quadrature phase B
            bit 5: Y-axis quadrature phase A
            bit 6: Y-axis quadrature phase B
            bit 7: Power key (firmware folds it into the matrix at
                   column 6 / row bit 6, replacing whatever the
                   physical matrix returns there)
    Port 2: Keyboard matrix column strobes for columns 0-7, driven as the
            high half of the MOVX address (DPH walks $7F, $BF ... $FE, one
            bit low at a time).  Never written as a port.
    P3.3/INT1:   ADB data line (open-drain, bidirectional, bit-banged).
    P3.4/T0:     Configuration-jumper read strobe (cleared once at init at
                 $0F84 for a MOVX read of 8 jumper bits onto P0); also
                 driven as the click-lock LED for the latching trackball
                 button at runtime.

*********************************************************************/

#include "emu.h"
#include "tk3000.h"

#include "cpu/mcs51/i80c51.h"

namespace {

class tk3000_device : public adb_device_interface, public adb_slot_card_interface
{
public:
	tk3000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void adb_w(int state) override;

	required_device<i80c31_device> m_mcu;
	required_ioport_array<10> m_rows;
	required_ioport m_power, m_jumpers, m_tbbuttons, m_tbx, m_tby;
	output_finder<> m_clicklock_led;

	void program_map(address_map &map) ATTR_COLD;
	void xdata_map(address_map &map) ATTR_COLD;

private:
	u8 matrix_r(offs_t offset);
	u8 p1_r();
	u8 p3_r();
	void p3_w(u8 data);

	TIMER_CALLBACK_MEMBER(sample_inputs);
	TIMER_CALLBACK_MEMBER(step_quadrature);

	emu_timer *m_sample_timer, *m_quad_timer;

	int m_adb_state;
	int m_our_last_adb_state;
	u8 m_p3;
	u8 m_x_phase, m_y_phase;
	s16 m_x_pending, m_y_pending;
	u8 m_last_x, m_last_y;
};

ROM_START(tk3000)
	ROM_REGION(0x2000, "mcu", 0)
	ROM_LOAD( "tk-3000.bin",  0x000000, 0x002000, CRC(44bf62ac) SHA1(be1d519794c2c74ee93952d94f59c2fafbb05091) )
ROM_END

static INPUT_PORTS_START(tk3000)
	PORT_START("ROW0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT)) PORT_NAME("Option")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9))

	PORT_START("ROW1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_MAMEKEY(LSHIFT)) PORT_NAME("Shift")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))

	PORT_START("ROW2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(RALT)) PORT_NAME("Right Option")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_NAME("Caps Lock") PORT_TOGGLE
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(0x08) PORT_NAME("Delete")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))

	PORT_START("ROW3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RWIN) PORT_NAME("Solid Apple") // ADB 0x36
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))

	PORT_START("ROW4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) PORT_NAME("Keypad Enter")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(0x0d) PORT_NAME("Return")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_START("ROW5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11) PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_CHAR(0x1b) PORT_NAME("Escape")

	PORT_START("ROW6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_NAME("Control")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED) // Power key - sourced from POWER ioport via P1.7
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))

	PORT_START("ROW7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_MAMEKEY(RSHIFT)) PORT_NAME("Right Shift")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F14) PORT_CHAR(UCHAR_MAMEKEY(F14))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))

	PORT_START("ROW8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED) // duplicate Shift slot per ADB_KEYCODE_TBL[76]
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RCONTROL)) PORT_NAME("Right Control")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F15) PORT_CHAR(UCHAR_MAMEKEY(F15))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))

	PORT_START("ROW9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LWIN) PORT_NAME("Open Apple") // ADB 0x37 (Command)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_NUMLOCK) PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK)) PORT_NAME("Keypad Clear")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F13) PORT_CHAR(UCHAR_MAMEKEY(F13))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))

	PORT_START("POWER")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_END) PORT_NAME("Power")

	PORT_START("TBBUTTONS")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Trackball Button 1") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Trackball Button 2") PORT_CODE(MOUSECODE_BUTTON2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Trackball Button 3") PORT_CODE(MOUSECODE_BUTTON3)
	PORT_BIT(0xf8, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("TBX")
	PORT_BIT(0xff, 0x00, IPT_TRACKBALL_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START("TBY")
	PORT_BIT(0xff, 0x00, IPT_TRACKBALL_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START("JUMPERS")
	PORT_CONFNAME(0x01, 0x01, "Trackball secondary buttons")
	PORT_CONFSETTING(0x00, "Button 2 latching, button 3 momentary")
	PORT_CONFSETTING(0x01, "Button 2 momentary, button 3 latching")
	PORT_CONFNAME(0x08, 0x00, "Numpad overlay A")
	PORT_CONFSETTING(0x00, DEF_STR(Off))
	PORT_CONFSETTING(0x08, DEF_STR(On))
	PORT_CONFNAME(0x20, 0x20, "Numpad overlay B")
	PORT_CONFSETTING(0x20, DEF_STR(Off))
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFNAME(0x40, 0x40, "Keycode $14 filter")
	PORT_CONFSETTING(0x40, DEF_STR(Off))
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_BIT(0x96, IP_ACTIVE_HIGH, IPT_UNUSED)

INPUT_PORTS_END

ioport_constructor tk3000_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(tk3000);
}

void tk3000_device::device_add_mconfig(machine_config &config)
{
	I80C31(config, m_mcu, 12_MHz_XTAL);
	m_mcu->set_addrmap(AS_PROGRAM, &tk3000_device::program_map);
	// The matrix and the jumper buffer are both read with MOVX, so they live
	// in the external data space rather than behind a P0 port handler.
	m_mcu->set_addrmap(AS_DATA, &tk3000_device::xdata_map);
	m_mcu->port_in_cb<1>().set(FUNC(tk3000_device::p1_r));
	m_mcu->port_in_cb<3>().set(FUNC(tk3000_device::p3_r));
	m_mcu->port_out_cb<3>().set(FUNC(tk3000_device::p3_w));
}

const tiny_rom_entry *tk3000_device::device_rom_region() const
{
	return ROM_NAME(tk3000);
}

tk3000_device::tk3000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	adb_device_interface(mconfig, ADB_TK3000, tag, owner, clock)
	, adb_slot_card_interface(mconfig, *this, DEVICE_SELF)
	, m_mcu(*this, "mcu")
	, m_rows{ *this, "ROW%u", 0U }
	, m_power(*this, "POWER")
	, m_jumpers(*this, "JUMPERS")
	, m_tbbuttons(*this, "TBBUTTONS")
	, m_tbx(*this, "TBX")
	, m_tby(*this, "TBY")
	, m_clicklock_led(*this, "clicklock_led")
	, m_sample_timer(nullptr)
	, m_quad_timer(nullptr)
	, m_adb_state(1)
	, m_our_last_adb_state(1)
	, m_p3(0xff)
	, m_x_phase(0), m_y_phase(0)
	, m_x_pending(0), m_y_pending(0)
	, m_last_x(0), m_last_y(0)
{
}

void tk3000_device::device_start()
{
	adb_device_interface::device_start();

	m_sample_timer = timer_alloc(FUNC(tk3000_device::sample_inputs), this);
	m_quad_timer = timer_alloc(FUNC(tk3000_device::step_quadrature), this);

	save_item(NAME(m_adb_state));
	save_item(NAME(m_our_last_adb_state));
	save_item(NAME(m_p3));
	save_item(NAME(m_x_phase));
	save_item(NAME(m_y_phase));
	save_item(NAME(m_x_pending));
	save_item(NAME(m_y_pending));
	save_item(NAME(m_last_x));
	save_item(NAME(m_last_y));
}

void tk3000_device::device_reset()
{
	adb_device_interface::device_reset();

	m_our_last_adb_state = 1;
	m_adb_state = 1;
	m_p3 = 0xff;
	m_x_phase = m_y_phase = 0;
	m_x_pending = m_y_pending = 0;
	m_last_x = m_tbx->read();
	m_last_y = m_tby->read();

	m_sample_timer->adjust(attotime::from_hz(1000), 0, attotime::from_hz(1000));
	// The firmware only accepts a sample that is stable across a ~100 us
	// debounce delay.
	m_quad_timer->adjust(attotime::from_usec(250), 0, attotime::from_usec(250));
}

void tk3000_device::adb_w(int state)
{
	m_mcu->set_input_line(MCS51_INT1_LINE, (m_adb_state && !state) ? ASSERT_LINE : CLEAR_LINE);
	m_adb_state = state;
}

void tk3000_device::program_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("mcu", 0);
}

void tk3000_device::xdata_map(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(tk3000_device::matrix_r));
}

u8 tk3000_device::matrix_r(offs_t offset)
{
	if (!BIT(m_p3, 4))
	{
		return m_jumpers->read();
	}

	u8 result = 0xff;
	u8 const cols = offset >> 8;

	for (unsigned i = 0; i < 8; i++)
	{
		if (!BIT(cols, i))
		{
			result &= m_rows[i]->read();
		}
	}

	if (!BIT(m_p3, 6))
	{
		result &= m_rows[8]->read();
	}
	if (!BIT(m_p3, 5))
	{
		result &= m_rows[9]->read();
	}

	return result;
}

u8 tk3000_device::p1_r()
{
	static constexpr u8 gray_code[4] = { 0, 1, 3, 2 };

	// Buttons are active low.  The quadrature pairs are {B, A} with A on the
	// lower pin: P1.4/P1.3 for X and P1.6/P1.5 for Y.
	u8 const buttons = m_tbbuttons->read();

	return (BIT(~buttons, 0) << 0)
		| (BIT(~buttons, 1) << 1)
		| (BIT(~buttons, 2) << 2)
		| (gray_code[m_x_phase] << 3)
		| (gray_code[m_y_phase] << 5)
		| (BIT(m_power->read(), 0) << 7);
}

TIMER_CALLBACK_MEMBER(tk3000_device::sample_inputs)
{
	u8 const x = m_tbx->read();
	u8 const y = m_tby->read();

	m_x_pending = std::clamp<s32>(m_x_pending + s8(x - m_last_x), -255, 255);
	m_y_pending = std::clamp<s32>(m_y_pending + s8(y - m_last_y), -255, 255);

	m_last_x = x;
	m_last_y = y;
}

TIMER_CALLBACK_MEMBER(tk3000_device::step_quadrature)
{
	if (m_x_pending)
	{
		int const dir = (m_x_pending > 0) ? 1 : -1;
		m_x_phase = (m_x_phase + dir) & 3;
		m_x_pending -= dir;
	}

	if (m_y_pending)
	{
		int const dir = (m_y_pending > 0) ? 1 : -1;
		m_y_phase = (m_y_phase + dir) & 3;
		m_y_pending -= dir;
	}
}

u8 tk3000_device::p3_r()
{
	return 0xf7 | (m_adb_state << 3);
}

void tk3000_device::p3_w(u8 data)
{
	m_p3 = data;

	m_clicklock_led = BIT(data, 4) ^ 1;

	if (BIT(data, 3) != m_our_last_adb_state)
	{
		m_our_last_adb_state = BIT(data, 3);
		adb_drive_w(m_our_last_adb_state);
		m_mcu->yield();
	}
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(ADB_TK3000, adb_slot_card_interface, tk3000_device, "adbtk3k", "Datadesk TK-3000 ADB keyboard and trackball");
