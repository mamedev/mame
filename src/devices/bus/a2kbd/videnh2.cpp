// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    Videx Enhancer ][ keyboard component

    The primary component of the Enhancer ][ is a microprocessor-
    based intelligent keyboard encoder board which fits in between
    the motherboard and the matrix of the newer-model (two-piece)
    keyboard. It replaces the AY-5-3600 keyboard scanner IC and
    associated components on the “piggyback style” encoder board,
    and is compatible with its rare add-on numeric keypad. (The
    Enhancer ][ was the successor to Videx's Keyboard and Display
    Enhancer, which only worked with revision 0-6 Apple IIs.)

    To enter lowercase mode, press Reset together with Shift. To
    subsequently return to uppercase mode, press Reset by itself.
    (Reset, as normal, is hardwired to warm-boot the system when
    pressed together with Ctrl.)

    Special characters can be generated in lowercase mode by the
    following key combinations:

        Ctrl-1      |
        Ctrl-2      ~
        Ctrl-3      DEL
        Ctrl-4      FS
        Ctrl-5      GS
        Ctrl-6      RS
        Ctrl-7      `
        Ctrl-8      {
        Ctrl-9      }
        Shift-0     @
        Ctrl-0      NUL
        Ctrl-:      US
        Ctrl--      _
        Ctrl-;      ^
        Ctrl-,      [
        Ctrl-.      ]
        Ctrl-/      \

    Buffered mode remains enabled as long as the acknowledge signal
    from the flip-flop (same polarity as bit 7 of $C000) was connected
    to pin 9 of the DIP header at cold start time.

    To allow compatible software to download macros to the keyboard,
    annunciator 3 from the addressable latch on the motherboard must
    be connected to pin 4 of the DIP header, and the acknowledge
    signal must also be connected as mentioned above. Even without
    these modifications active, macro definitions may be initiated
    by typing Ctrl-Shift-Rept; Rept alone terminates a macro.

    This device emulation does not include the lowercase character
    ROM also supplied with the Enhancer ][ (which may not be needed by
    users of key filter programs or 80-column cards such as the Videx
    VideoTerm) or the suggested patches to CAPTST and RDKEY in the
    F8 monitor. The apple2pe system provides both of these in MAME.

    Videx also advertised a Dvorak keyboard option. The firmware EPROM
    for this version is undumped.

    Unitron Eletrônica's "Teclado Inteligente" (intelligent keyboard)
    is a Brazilian clone of the Enhancer ][. It allows several more
    punctuation characters to be produced by shifting letters in
    uppercase mode, and expands the optional detached numeric keypad
    to 20 keys.

*********************************************************************/

#include "emu.h"
#include "videnh2.h"

#include "cpu/m6502/m6504.h"

namespace {

class videnh2_device : public device_t, public device_a2kbd_interface
{
public:
	// device type constructor
	videnh2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	DECLARE_INPUT_CHANGED_MEMBER(reset_changed);

protected:
	videnh2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_a2kbd_interface implementation
	virtual void an3_w(int state) override;
	virtual void ack_w(int state) override;
	virtual int shift_r() override;
	virtual int control_r() override;

	// memory map
	virtual void mem_map(address_map &map) ATTR_COLD;

	// memory-mapped I/O handlers
	void keyout_w(u8 data);
	u8 mtrix1_r(offs_t offset);
	u8 mtrix2_r(offs_t offset);

private:
	// object finders
	required_device<cpu_device> m_kbdcpu;
	required_ioport_array<9> m_keys;
	required_ioport m_spkeys;
	required_ioport m_sysreset;

	// internal state/
	bool m_an3;
	bool m_ack;
};

class uniap2ti_device : public videnh2_device
{
public:
	// device type constructor
	uniap2ti_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// memory map
	virtual void mem_map(address_map &map) override ATTR_COLD;
};

videnh2_device::videnh2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_a2kbd_interface(mconfig, *this)
	, m_kbdcpu(*this, "kbdcpu")
	, m_keys(*this, "R%u", 1U)
	, m_spkeys(*this, "SPKEYS")
	, m_sysreset(*this, "SYSRESET")
	, m_an3(true)
	, m_ack(true)
{
}

videnh2_device::videnh2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: videnh2_device(mconfig, A2KBD_VIDENH2, tag, owner, clock)
{
}

uniap2ti_device::uniap2ti_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: videnh2_device(mconfig, A2KBD_UNIAP2TI, tag, owner, clock)
{
}

void videnh2_device::device_start()
{
	save_item(NAME(m_an3));
	save_item(NAME(m_ack));
}

void videnh2_device::an3_w(int state)
{
	m_an3 = bool(state);
}

void videnh2_device::ack_w(int state)
{
	m_ack = bool(state);
}

void videnh2_device::keyout_w(u8 data)
{
	strobe_w(BIT(data, 7));
	b_w(data & 0x7f);
}

int videnh2_device::shift_r()
{
	return BIT(m_spkeys->read(), 3);
}

int videnh2_device::control_r()
{
	return BIT(m_spkeys->read(), 2);
}

INPUT_CHANGED_MEMBER(videnh2_device::reset_changed)
{
	const ioport_value sysreset = m_sysreset->read();
	const bool reset_active = sysreset && (m_spkeys->read() & (sysreset == 1 ? 0x20 : 0x24)) == 0;
	m_kbdcpu->set_input_line(INPUT_LINE_RESET, reset_active ? ASSERT_LINE : CLEAR_LINE);
	reset_w(!reset_active);
}

u8 videnh2_device::mtrix1_r(offs_t offset)
{
	u8 ret = 0xff;

	for (int i = 0; i < 9; i++)
		if (BIT(offset, i))
			ret &= m_keys[i]->read();

	return ret;
}

u8 videnh2_device::mtrix2_r(offs_t offset)
{
	u8 ret = 0x03;

	for (int i = 0; i < 9; i++)
		if (BIT(offset, i))
			ret &= m_keys[i]->read() >> 8;

	ret |= m_spkeys->read();

	if (m_ack)
		ret |= 0x40;
	if (m_an3)
		ret |= 0x80;

	return ret;
}

void videnh2_device::mem_map(address_map &map)
{
	map(0x0000, 0x03ff).mirror(0x400).ram(); // 2x 2114
	map(0x0a00, 0x0bff).r(FUNC(videnh2_device::mtrix1_r));
	map(0x0c00, 0x0dff).r(FUNC(videnh2_device::mtrix2_r));
	map(0x0e00, 0x0e00).mirror(0x1ff).w(FUNC(videnh2_device::keyout_w));
	map(0x1800, 0x1fff).rom().region("firmware", 0);
}

void uniap2ti_device::mem_map(address_map &map)
{
	map(0x0000, 0x03ff).ram();
	map(0x0400, 0x0400).nopw(); // LED output?
	map(0x0800, 0x09ff).r(FUNC(uniap2ti_device::mtrix1_r));
	map(0x0a00, 0x0bff).r(FUNC(uniap2ti_device::mtrix2_r));
	map(0x0c00, 0x0c00).w(FUNC(uniap2ti_device::keyout_w));
	map(0x1800, 0x1fff).rom().region("firmware", 0);
}

static INPUT_PORTS_START(videnh2)
	PORT_START("R1")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(0x0d)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_UNUSED) // alternate space
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_UNUSED) // alternate +
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_UNUSED) // alternate *
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR() PORT_CHAR(0x01)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) PORT_CHAR(0x1b)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR() PORT_CHAR(0x13)

	PORT_START("R2")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR(']')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('^') PORT_CHAR(0x0e)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR() PORT_CHAR(0x02)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR() PORT_CHAR(0x16)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR() PORT_CHAR(0x03)
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR() PORT_CHAR(0x18)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR() PORT_CHAR(0x1a)

	PORT_START("R3")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT), 0x15)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT), 0x08)
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR() PORT_CHAR(0x0c)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR() PORT_CHAR(0x0b)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR() PORT_CHAR(0x0a)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G  BELL") PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR() PORT_CHAR(0x07)
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR() PORT_CHAR(0x06)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR() PORT_CHAR(0x04)

	PORT_START("R4")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('@') PORT_CHAR(0x10)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR() PORT_CHAR(0x0f)
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR() PORT_CHAR(0x09)
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR() PORT_CHAR(0x19)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR() PORT_CHAR(0x14)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR() PORT_CHAR(0x12)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR() PORT_CHAR(0x05)
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR() PORT_CHAR(0x17)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR() PORT_CHAR(0x11)

	PORT_START("R5")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')

	PORT_START("R6")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_UNUSED) // NUL
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA_PAD) PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD))
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x3f0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("R7")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x3f0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("R8")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x3f0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("R9")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x3f0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("SPKEYS")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(videnh2_device::reset_changed), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Rept") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_F12) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(videnh2_device::reset_changed), 0)
	PORT_CONFNAME(0x40, 0x00, "Type-ahead buffer")
	PORT_CONFSETTING(0x40, DEF_STR(Off))
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFNAME(0x80, 0x00, "Automatic download")
	PORT_CONFSETTING(0x80, DEF_STR(Off))
	PORT_CONFSETTING(0x00, DEF_STR(On))

	PORT_START("SYSRESET")
	PORT_CONFNAME(3, 2, "System reset key")
	PORT_CONFSETTING(0, DEF_STR(None))
	PORT_CONFSETTING(1, "Reset only")
	PORT_CONFSETTING(2, "Ctrl-Reset")
INPUT_PORTS_END

static INPUT_PORTS_START(uniap2ti)
	PORT_INCLUDE(videnh2)

	PORT_MODIFY("R3")
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('\\') PORT_CHAR(0x0c)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('[') PORT_CHAR(0x0b)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('{') PORT_CHAR(0x0a)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('`')

	PORT_MODIFY("R4")
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('_') PORT_CHAR(0x0f)
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('}') PORT_CHAR(0x09)
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('~')

	PORT_MODIFY("R6")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad Esc") PORT_CODE(KEYCODE_NUMLOCK)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Keypad \xe2\x86\x92") PORT_CODE(KEYCODE_TAB_PAD) PORT_CHAR(UCHAR_MAMEKEY(TAB_PAD)) // → (above 9 key)
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad Space") PORT_CODE(KEYCODE_COMMA_PAD) // (below 3 key)
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))

	PORT_MODIFY("R7")
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Keypad \xe2\x86\x90") PORT_CODE(KEYCODE_BS_PAD) PORT_CHAR(UCHAR_MAMEKEY(BS_PAD)) // ← (above 8 key)
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD)) // (below 1 key)
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))

	PORT_MODIFY("R8")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Keypad ÷") PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD)) // (top right of pad)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) // (to right of 6 key)
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))

	PORT_MODIFY("R9")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Keypad ×") PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK)) // (to right of 9 key)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD)) // (to right of 3 key)
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
INPUT_PORTS_END

ioport_constructor videnh2_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(videnh2);
}

ioport_constructor uniap2ti_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(uniap2ti);
}

void videnh2_device::device_add_mconfig(machine_config &config)
{
	M6504(config, m_kbdcpu, 600'000); // approximate clock (no XTAL, just a resistor, capacitor and inverter between φ2 and φ0)
	m_kbdcpu->set_addrmap(AS_PROGRAM, &videnh2_device::mem_map);

	// Macro downloading requires fairly tight synchronization between the 6502 and 6504
	config.set_maximum_quantum(attotime::from_usec(20));
}

ROM_START(videnh2)
	ROM_REGION(0x800, "firmware", 0)
	ROM_LOAD("videx enhancer ii rom - 2716.bin", 0x000, 0x800, CRC(5cb9e0cb) SHA1(91f3e4a56382aea91d9197f06550e42fcfbcd1bf))
ROM_END

ROM_START(uniap2ti)
	ROM_REGION(0x800, "firmware", 0)
	ROM_LOAD("unitron_apii+_keyboard.ic3", 0x000, 0x800, CRC(edc43205) SHA1(220cc21d86f1ab63a301ae7a9c5ff0f3f6cddb70))
ROM_END

const tiny_rom_entry *videnh2_device::device_rom_region() const
{
	return ROM_NAME(videnh2);
}

const tiny_rom_entry *uniap2ti_device::device_rom_region() const
{
	return ROM_NAME(uniap2ti);
}

} // anonymous namespace

// device type definitions
DEFINE_DEVICE_TYPE_PRIVATE(A2KBD_VIDENH2, device_a2kbd_interface, videnh2_device, "videnh2", "Videx Keyboard Enhancer ][")
DEFINE_DEVICE_TYPE_PRIVATE(A2KBD_UNIAP2TI, device_a2kbd_interface, uniap2ti_device, "uniap2ti", "Unitron AP II+ Teclado Inteligente")
