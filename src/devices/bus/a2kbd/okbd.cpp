// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    Apple II original keyboard (605-4114)

    This TTY-layout keyboard is based on an off-the-shelf model from
    Datanetics Corp. that was commonly shipped with the Apple I.
    There were a few PCB revisions, but each has the keys, the
    MM5740AAE encoder and support logic chips all mounted on a single
    board.

    The output of the Reset key is not gated with Ctrl, nor is it
    filtered in any way. On the actual keyboard, Reset also has the
    convenience of being positioned directly above the Return key. :-)

    Adjusting the positions of K, L and O within the key matrix allows
    these letters to produce [, \ and _ codes when shifted and ESC,
    FS and US when pressed with both Shift and Ctrl. This modification
    was described in an article by David Hudson published in
    International Apple Core under the title "Stalking the Wild '['".

*********************************************************************/

#include "emu.h"
#include "okbd.h"

#include "machine/mm5740.h"
#include "machine/rescap.h"

namespace {

class a2okbd_device : public device_t, public device_a2kbd_interface
{
public:
	// device type constructor
	a2okbd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void repeat_w(int state);

	// PORT_WRITE_LINE_MEMBER needs this to be public here
	using device_a2kbd_interface::reset_w;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_a2kbd_interface implementation
	virtual int shift_r() override;
	virtual int control_r() override;

private:
	void data_ready_w(int state);

	// timer callbacks
	TIMER_CALLBACK_MEMBER(repeat_on);
	TIMER_CALLBACK_MEMBER(repeat_off);

	// object finders
	required_device<mm5740_device> m_encoder;
	required_ioport m_modifiers;

	// timers
	emu_timer *m_repeat_on_timer;
	emu_timer *m_repeat_off_timer;
};

a2okbd_device::a2okbd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, APPLE2_OKBD, tag, owner, clock)
	, device_a2kbd_interface(mconfig, *this)
	, m_encoder(*this, "encoder")
	, m_modifiers(*this, "MODIFIERS")
{
}

void a2okbd_device::device_start()
{
	m_repeat_on_timer = timer_alloc(FUNC(a2okbd_device::repeat_on), this);
	m_repeat_off_timer = timer_alloc(FUNC(a2okbd_device::repeat_off), this);
}

TIMER_CALLBACK_MEMBER(a2okbd_device::repeat_on)
{
	if (BIT(m_modifiers->read(), 4))
	{
		m_encoder->repeat_line_w(1);
		m_repeat_off_timer->adjust(PERIOD_OF_555_ASTABLE(RES_M(1), RES_K(220) / 2.0, CAP_U(.1))); // charge time
	}
}

void a2okbd_device::repeat_w(int state)
{
	if (!state)
		m_encoder->repeat_line_w(0);
	else if (!m_repeat_on_timer->enabled() && !m_repeat_off_timer->enabled())
	{
		m_encoder->repeat_line_w(1);
		m_repeat_off_timer->adjust(PERIOD_OF_555_ASTABLE(RES_M(1), RES_K(220) / 2.0, CAP_U(.1))); // charge time
	}
}

TIMER_CALLBACK_MEMBER(a2okbd_device::repeat_off)
{
	m_encoder->repeat_line_w(0);
	m_repeat_on_timer->adjust(PERIOD_OF_555_ASTABLE(0.0, RES_K(220) / 2.0, CAP_U(.1))); // discharge time
}

static INPUT_PORTS_START(okbd)
	PORT_START("X1")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('0') PORT_CODE(KEYCODE_0)
	PORT_BIT(0x3bf, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X2")
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CHAR(0x0d) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CHAR(' ') PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u2190") PORT_CHAR(UCHAR_MAMEKEY(LEFT), 0x08) PORT_CODE(KEYCODE_LEFT) // ←
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('-') PORT_CHAR('=') PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x0bd, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X3")
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P  @") PORT_CHAR('P') PORT_CHAR('@') PORT_CHAR(0x10) PORT_CODE(KEYCODE_P)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(';') PORT_CHAR('+') PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('/') PORT_CHAR('?') PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('O') PORT_CHAR() PORT_CHAR(0x0f) PORT_CODE(KEYCODE_O)
												 PORT_CONDITION("MODIFIERS", 0x20, EQUALS, 0x00)
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_UNUSED)   PORT_CONDITION("MODIFIERS", 0x20, EQUALS, 0x20)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(':') PORT_CHAR('*') PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x0ab, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X4")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('9') PORT_CHAR(')') PORT_CODE(KEYCODE_9)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CHAR('I') PORT_CHAR() PORT_CHAR(0x09) PORT_CODE(KEYCODE_I)
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O  _") PORT_CHAR('O') PORT_CHAR('_') PORT_CHAR(0x0f) PORT_CODE(KEYCODE_O)
												 PORT_CONDITION("MODIFIERS", 0x20, EQUALS, 0x20)
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K  [") PORT_CHAR('K') PORT_CHAR('[') PORT_CHAR(0x0b) PORT_CODE(KEYCODE_K)
												 PORT_CONDITION("MODIFIERS", 0x20, EQUALS, 0x20)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L  \\") PORT_CHAR('L') PORT_CHAR('\\') PORT_CHAR(0x0c) PORT_CODE(KEYCODE_L)
												 PORT_CONDITION("MODIFIERS", 0x20, EQUALS, 0x20)
	PORT_BIT(0x01c, IP_ACTIVE_LOW, IPT_UNUSED)   PORT_CONDITION("MODIFIERS", 0x20, EQUALS, 0x00)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(',') PORT_CHAR('<') PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('.') PORT_CHAR('>') PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CHAR('L') PORT_CHAR() PORT_CHAR(0x0c) PORT_CODE(KEYCODE_L)
												 PORT_CONDITION("MODIFIERS", 0x20, EQUALS, 0x00)
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CHAR('K') PORT_CHAR() PORT_CHAR(0x0b) PORT_CODE(KEYCODE_K)
												 PORT_CONDITION("MODIFIERS", 0x20, EQUALS, 0x00)
	PORT_BIT(0x180, IP_ACTIVE_LOW, IPT_UNUSED)   PORT_CONDITION("MODIFIERS", 0x20, EQUALS, 0x20)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('8') PORT_CHAR('(') PORT_CODE(KEYCODE_8)

	PORT_START("X5")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('6') PORT_CHAR('&') PORT_CODE(KEYCODE_6)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('U') PORT_CODE(KEYCODE_U)
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CHAR('Y') PORT_CHAR() PORT_CHAR(0x19) PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CHAR('J') PORT_CHAR() PORT_CHAR(0x0a) PORT_CODE(KEYCODE_J)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('H') PORT_CODE(KEYCODE_H)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CHAR('M') PORT_CHAR(']') PORT_CODE(KEYCODE_M)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N  ^") PORT_CHAR('N') PORT_CHAR('^') PORT_CHAR(0x0e) PORT_CODE(KEYCODE_N)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('7') PORT_CHAR('\'') PORT_CODE(KEYCODE_7)
	PORT_BIT(0x180, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X6")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('5') PORT_CHAR('%') PORT_CODE(KEYCODE_5)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CHAR('R') PORT_CHAR() PORT_CHAR(0x12) PORT_CODE(KEYCODE_R)
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CHAR('T') PORT_CHAR() PORT_CHAR(0x14) PORT_CODE(KEYCODE_T)
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CHAR('F') PORT_CHAR() PORT_CHAR(0x06) PORT_CODE(KEYCODE_F)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G  BELL") PORT_CHAR('G') PORT_CHAR() PORT_CHAR(0x07) PORT_CODE(KEYCODE_G)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CHAR('V') PORT_CHAR() PORT_CHAR(0x16) PORT_CODE(KEYCODE_V)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CHAR('B') PORT_CHAR() PORT_CHAR(0x02) PORT_CODE(KEYCODE_B)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('4') PORT_CHAR('$') PORT_CODE(KEYCODE_4)
	PORT_BIT(0x180, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X7")
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CHAR('E') PORT_CHAR() PORT_CHAR(0x05) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CHAR('D') PORT_CHAR() PORT_CHAR(0x04) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CHAR('C') PORT_CHAR() PORT_CHAR(0x03) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u2192") PORT_CHAR(UCHAR_MAMEKEY(RIGHT), 0x15) PORT_CODE(KEYCODE_RIGHT) // →
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('3') PORT_CHAR('#') PORT_CODE(KEYCODE_3)
	PORT_BIT(0x195, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X8")
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CHAR('W') PORT_CHAR() PORT_CHAR(0x17) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CHAR('S') PORT_CHAR() PORT_CHAR(0x13) PORT_CODE(KEYCODE_S)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CHAR('X') PORT_CHAR() PORT_CHAR(0x18) PORT_CODE(KEYCODE_X)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('2') PORT_CHAR('"') PORT_CODE(KEYCODE_2)
	PORT_BIT(0x1d5, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X9")
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CHAR('Q') PORT_CHAR() PORT_CHAR(0x11) PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CHAR(0x1b) PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CHAR('A') PORT_CHAR() PORT_CHAR(0x01) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CHAR('Z') PORT_CHAR() PORT_CHAR(0x1a) PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('1') PORT_CHAR('!') PORT_CODE(KEYCODE_1)
	PORT_BIT(0x1d1, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("MODIFIERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CHAR(UCHAR_SHIFT_1) PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Control") PORT_CHAR(UCHAR_SHIFT_2) PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_F12) PORT_WRITE_LINE_MEMBER(FUNC(a2okbd_device::reset_w))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Rept") PORT_CODE(KEYCODE_BACKSLASH) PORT_WRITE_LINE_MEMBER(FUNC(a2okbd_device::repeat_w))
	PORT_CONFNAME(0x20, 0x00, "Key matrix")
	PORT_CONFSETTING(0x00, "Original")
	PORT_CONFSETTING(0x20, "Modified")
INPUT_PORTS_END

ioport_constructor a2okbd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(okbd);
}

int a2okbd_device::shift_r()
{
	return (m_modifiers->read() & 0x03) == 0x03;
}

int a2okbd_device::control_r()
{
	return BIT(m_modifiers->read(), 2);
}

void a2okbd_device::data_ready_w(int state)
{
	strobe_w(state);
	if (state)
		b_w(~m_encoder->b_r() & 0x7f);
}

void a2okbd_device::device_add_mconfig(machine_config &config)
{
	// MM5740 clocked by 7400-based astable circuit; frequency given in _Understanding the Apple II_ by Jim Sather
	MM5740AAE(config, m_encoder, 50'000);
	m_encoder->x_cb<1>().set_ioport("X1");
	m_encoder->x_cb<2>().set_ioport("X2");
	m_encoder->x_cb<3>().set_ioport("X3");
	m_encoder->x_cb<4>().set_ioport("X4");
	m_encoder->x_cb<5>().set_ioport("X5");
	m_encoder->x_cb<6>().set_ioport("X6");
	m_encoder->x_cb<7>().set_ioport("X7");
	m_encoder->x_cb<8>().set_ioport("X8");
	m_encoder->x_cb<9>().set_ioport("X9");
	m_encoder->shift_cb().set(FUNC(a2okbd_device::shift_r)).invert();
	m_encoder->control_cb().set(FUNC(a2okbd_device::control_r)).invert();
	m_encoder->data_ready_cb().set(FUNC(a2okbd_device::data_ready_w));
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(APPLE2_OKBD, device_a2kbd_interface, a2okbd_device, "a2okbd", "Apple II original (Datanetics) keyboard")
