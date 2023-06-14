// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit Terminal Logic Board (TLB)

    The board used in the H19 smart terminal designed and manufactured
    by Heath Company. (and the identical Z19 sold by Zenith Data Systems)
    This board is also used the Heath's H89 / H88, and ZDS's Z-89 and Z-90.

    The keyboard consists of a 9x10 matrix connected to a MM5740AAC/N
    mask-programmed keyboard controller. The output of this passes
    through a rom.

    Input can also come from the serial port (a 8250).
    Either device will signal an interrupt to the CPU when a key
    is pressed/data is received.

  TODO:
    - determine why ULTRA ROM's self-diag (ESC |) fails for the ROM and
      scratchpad memory
    - when pressing "REPEAT", the other pressed key should repeatedly trigger

****************************************************************************/
/***************************************************************************
  Memory Layout

    The U435 three-to-eight line decoder uses A14 and A15 to generate
    three memory addresses:

      1.   Program ROM        0x0000
      2.   Scratchpad RAM     0x4000
      3.   Display Memory     0xF800


  Port Layout

    Only address lines A5, A6, A7 are used by the U442 three-to-eight
    line decoder

    Address   Description
    ----------------------------------------------------
      0x00    Power-up configuration (primary - SW401)
      0x20    Power-up configuration (secondary - SW402)
      0x40    ACE (communications)
      0x60    CRT controller
      0x80    Keyboard encoder
      0xA0    Keyboard status
      0xC0    Key click enable
      0xE0    Bell enable

****************************************************************************/

#include "emu.h"

#include "tlb.h"

// Clocks
static constexpr XTAL MASTER_CLOCK = XTAL(12'288'000);

// Standard H19 used a 2.048 MHz clock
static constexpr XTAL H19_CLOCK = MASTER_CLOCK / 6;
static constexpr XTAL MC6845_CLOCK = MASTER_CLOCK / 8;
static constexpr XTAL INS8250_CLOCK = MASTER_CLOCK / 4;

// Beep Frequency is 1 KHz
static constexpr XTAL H19_BEEP_FRQ = (H19_CLOCK / 2048);

// Capacitor value in pF
static constexpr uint32_t H19_KEY_DEBOUNCE_CAPACITOR = 5000;
#define MM5740_CLOCK (mm5740_device::calc_effective_clock_key_debounce(H19_KEY_DEBOUNCE_CAPACITOR))

DEFINE_DEVICE_TYPE(HEATH_TLB, heath_tlb_device, "heath_tlb", "Heath Terminal Logic Board");
DEFINE_DEVICE_TYPE(HEATH_SUPER19, heath_super19_tlb_device, "heath_super19_tlb", "Heath Terminal Logic Board w/Super19 ROM");
DEFINE_DEVICE_TYPE(HEATH_WATZ, heath_watz_tlb_device, "heath_watz_tlb", "Heath Terminal Logic Board w/Watzman ROM");
DEFINE_DEVICE_TYPE(HEATH_ULTRA, heath_ultra_tlb_device, "heath_ultra_tlb", "Heath Terminal Logic Board w/Ultra ROM");

heath_tlb_device::heath_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	heath_tlb_device(mconfig, HEATH_TLB, tag, owner, clock)
{
}

heath_tlb_device::heath_tlb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_maincpu(*this, "maincpu"),
	m_write_sd(*this),
	m_reset(*this),
	m_palette(*this, "palette"),
	m_crtc(*this, "crtc"),
	m_ace(*this, "ins8250"),
	m_beep(*this, "beeper"),
	m_p_videoram(*this, "videoram"),
	m_p_chargen(*this, "chargen"),
	m_mm5740(*this, "mm5740"),
	m_kbdrom(*this, "keyboard"),
	m_kbspecial(*this, "MODIFIERS")
{
}

void heath_tlb_device::checkBeepState()
{
	if (!m_keyclickactive && !m_bellactive)
	{
		m_beep->set_state(0);
	}
}

TIMER_CALLBACK_MEMBER(heath_tlb_device::key_click_off)
{
	m_keyclickactive = false;

	checkBeepState();
}

TIMER_CALLBACK_MEMBER(heath_tlb_device::bell_off)
{
	m_bellactive = false;

	checkBeepState();
}

void heath_tlb_device::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).mirror(0x3000).rom();
	map(0x4000, 0x40ff).mirror(0x3f00).ram();
	map(0xc000, 0xc7ff).mirror(0x3800).ram().share(m_p_videoram);
}

void heath_tlb_device::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).mirror(0x1f).portr("SW401");
	map(0x20, 0x20).mirror(0x1f).portr("SW402");
	map(0x40, 0x47).mirror(0x18).rw(m_ace, FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));
	map(0x60, 0x60).mirror(0x1e).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0x61, 0x61).mirror(0x1e).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x80, 0x80).mirror(0x1f).r(FUNC(heath_tlb_device::kbd_key_r));
	map(0xa0, 0xa0).mirror(0x1f).r(FUNC(heath_tlb_device::kbd_flags_r));
	map(0xc0, 0xc0).mirror(0x1f).w(FUNC(heath_tlb_device::key_click_w));
	map(0xe0, 0xe0).mirror(0x1f).w(FUNC(heath_tlb_device::bell_w));
}


// Keyboard encoder masks
static constexpr uint8_t KB_ENCODER_KEY_VALUE_MASK = 0x7f;
static constexpr uint8_t KB_ENCODER_CONTROL_KEY_MASK = 0x80;

// Keyboard flag masks
static constexpr uint8_t KB_STATUS_SHIFT_KEYS_MASK = 0x01;
static constexpr uint8_t KB_STATUS_CAPS_LOCK_MASK = 0x02;
static constexpr uint8_t KB_STATUS_BREAK_KEY_MASK = 0x04;
static constexpr uint8_t KB_STATUS_ONLINE_KEY_MASK = 0x08;
static constexpr uint8_t KB_STATUS_REPEAT_KEYS_MASK = 0x40;
static constexpr uint8_t KB_STATUS_KEYBOARD_STROBE_MASK = 0x80;

void heath_tlb_device::device_resolve_objects()
{
	m_write_sd.resolve_safe();
	m_reset.resolve_safe();
}

void heath_tlb_device::device_start()
{
	save_item(NAME(m_transchar));
	save_item(NAME(m_strobe));
	save_item(NAME(m_keyclickactive));
	save_item(NAME(m_bellactive));
	save_item(NAME(m_reset_pending));

	m_strobe = false;
	m_keyclickactive = false;
	m_bellactive = false;
	m_reset_pending = false;

	m_key_click_timer = timer_alloc(FUNC(heath_tlb_device::key_click_off), this);
	m_bell_timer = timer_alloc(FUNC(heath_tlb_device::bell_off), this);
}

void heath_tlb_device::device_reset()
{
	m_strobe = false;
	m_keyclickactive = false;
	m_bellactive = false;
	m_reset_pending = false;

	m_crtc->reset();
	m_ace->reset();
	m_beep->reset();
	m_mm5740->reset();
	m_maincpu->reset();
}

void heath_tlb_device::key_click_w(uint8_t data)
{
	// Keyclick - 6 mSec
	m_beep->set_state(1);
	m_keyclickactive = true;
	m_key_click_timer->adjust(attotime::from_msec(6));
}

void heath_tlb_device::bell_w(uint8_t data)
{
	// Bell (^G) - 200 mSec
	m_beep->set_state(1);
	m_bellactive = true;
	m_bell_timer->adjust(attotime::from_msec(200));
}


/***************************************************************************
  MM5740 B Mapping to the ROM address

    B1     -> A0          A10  =  0
    B2     -> A1          A9   =  0
    B3     -> A2          A8   =  B8
    B4     -> A3          A7   =  B7
    B5     -> A4          A6   =  B9
    B6     -> A5          A5   =  B6
    B7     -> A7          A4   =  B5
    B8     -> A8          A3   =  B4
    B9     -> A6          A2   =  B3
    ground -> A9          A1   =  B2
    ground -> A10         A0   =  B1

****************************************************************************/
uint16_t heath_tlb_device::translate_mm5740_b(uint16_t b)
{
	return ((b & 0x100) >> 2) | ((b & 0x0c0) << 1) | (b & 0x03f);
}

void heath_tlb_device::check_for_reset()
{
	ioport_value value = m_kbspecial->read();

	if ((value & 0x300) == 0)
	{
		m_reset_pending = true;
	}
	else if (m_reset_pending)
	{
		m_reset_pending = false;
		m_maincpu->reset();
		m_reset(ASSERT_LINE);
	}
}

uint8_t heath_tlb_device::kbd_key_r()
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	m_strobe = false;

	// high bit is for control key pressed, this is handled in the ROM,
	// no processing needed.
	return m_transchar;
}

uint8_t heath_tlb_device::kbd_flags_r()
{
	uint16_t modifiers = m_kbspecial->read();
	uint8_t rv = modifiers & 0x7e;

	// check both shifts
	if ((modifiers & 0x120) != 0x120)
	{
		rv |= KB_STATUS_SHIFT_KEYS_MASK;
	}

	if (!m_strobe)
	{
		rv |= KB_STATUS_KEYBOARD_STROBE_MASK;
	}

	return rv;
}

int heath_tlb_device::mm5740_shift_r()
{
	return ((m_kbspecial->read() & 0x120) != 0x120) ? ASSERT_LINE : CLEAR_LINE;
}

int heath_tlb_device::mm5740_control_r()
{
	return (m_kbspecial->read() & 0x10) ? CLEAR_LINE : ASSERT_LINE;
}

void heath_tlb_device::mm5740_data_ready_w(int state)
{
	check_for_reset();

	if (state == ASSERT_LINE)
	{
		uint8_t *decode = m_kbdrom->base();

		m_transchar = decode[translate_mm5740_b(m_mm5740->b_r())];
		m_strobe = true;
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
	}
}

MC6845_UPDATE_ROW(heath_tlb_device::crtc_update_row)
{
	if (!de)
	{
		return;
	}

	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	uint32_t *p = &bitmap.pix(y);

	for (uint16_t x = 0; x < x_count; x++)
	{
		uint8_t inv = (x == cursor_x) ? 0xff : 0;

		uint8_t chr = m_p_videoram[(ma + x) & 0x7ff];

		if (chr & 0x80)
		{
			inv ^= 0xff;
			chr &= 0x7f;
		}

		// get pattern of pixels for that character scanline
		uint8_t const gfx = m_p_chargen[(chr<<4) | ra] ^ inv;

		// Display a scanline of a character (8 pixels)
		*p++ = palette[BIT(gfx, 7)];
		*p++ = palette[BIT(gfx, 6)];
		*p++ = palette[BIT(gfx, 5)];
		*p++ = palette[BIT(gfx, 4)];
		*p++ = palette[BIT(gfx, 3)];
		*p++ = palette[BIT(gfx, 2)];
		*p++ = palette[BIT(gfx, 1)];
		*p++ = palette[BIT(gfx, 0)];
	}
}


// F4 Character Displayer
static const gfx_layout h19_charlayout =
{
	8, 10,                  // 8 x 10 characters
	128,                    // 128 characters
	1,                      // 1 bits per pixel
	{ 0 },                  // no bitplanes
	// x offsets
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	// y offsets
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8 },
	8*16                    // every char takes 16 bytes
};

static GFXDECODE_START(gfx_h19)
	GFXDECODE_ENTRY("chargen", 0x0000, h19_charlayout, 0, 1)
GFXDECODE_END


// Input ports
static INPUT_PORTS_START( tlb )

	PORT_START("MODIFIERS")
	// bit 0 - 0x001 connects to B8 of MM5740 - low if either shift key is
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CapsLock")   PORT_CODE(KEYCODE_CAPSLOCK)  PORT_TOGGLE
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Break")      PORT_CODE(KEYCODE_PAUSE)
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("OffLine")    PORT_CODE(KEYCODE_F12)       PORT_TOGGLE
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL")       PORT_CODE(KEYCODE_LCONTROL)  PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LeftShift")  PORT_CODE(KEYCODE_LSHIFT)    PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Repeat")     PORT_CODE(KEYCODE_LALT)
	// bit 7 - 0x080 is low if a key is pressed
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RightShift") PORT_CODE(KEYCODE_RSHIFT)    PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Reset")      PORT_CODE(KEYCODE_F10)       PORT_CHAR(UCHAR_MAMEKEY(F10))

	PORT_START("X1")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/")          PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("k_X2")
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP-0")       PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP-.")       PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP-Enter")   PORT_CODE(KEYCODE_ENTER_PAD)  PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X2")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("; :")        PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\' \"")      PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("{ }")        PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('{') PORT_CHAR('}')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return")     PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP-1 IL")    PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP-2 Down")  PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP-3 DL")    PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X3")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P")          PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[ ]")        PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR(']')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\ |")       PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Line Feed")  PORT_CODE(KEYCODE_RWIN)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEL")        PORT_CODE(KEYCODE_DEL)        PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP-4 LEFT")  PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP-5 HOME")  PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP-6 RIGHT") PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X4")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0 )")        PORT_CODE(KEYCODE_0)          PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("- _")        PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("= +")        PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("` ~")        PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Back Space") PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("k_X1")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP-7 IC")    PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP-8 UP")    PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP-9 DC")    PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X5")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1")         PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2")         PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3")         PORT_CODE(KEYCODE_F3)         PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4")         PORT_CODE(KEYCODE_F4)         PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5")         PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Erase")      PORT_CODE(KEYCODE_F6)         PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Blue")       PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Red")        PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Gray")       PORT_CODE(KEYCODE_F9)         PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X6")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z")          PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X")          PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C")          PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V")          PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B")          PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N")          PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M")          PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(", <")        PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(". >")        PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space")      PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')

	PORT_START("X7")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A")          PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S")          PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D")          PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F")          PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G")          PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H")          PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J")          PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K")          PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L")          PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Scroll")     PORT_CODE(KEYCODE_F11)

	PORT_START("X8")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q")          PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W")          PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E")          PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R")          PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T")          PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y")          PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U")          PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I")          PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O")          PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab")        PORT_CODE(KEYCODE_TAB)        PORT_CHAR(UCHAR_MAMEKEY(TAB))

	PORT_START("X9")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 !")        PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 @")        PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 #")        PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 $")        PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 %")        PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 ^")        PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 &")        PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 *")        PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 (")        PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc")        PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("SW401")
	PORT_DIPNAME( 0x0f, 0x0c, "Baud Rate")           PORT_DIPLOCATION("SW401:1,2,3,4")
	PORT_DIPSETTING(    0x01, "110")
	PORT_DIPSETTING(    0x02, "150")
	PORT_DIPSETTING(    0x03, "300")
	PORT_DIPSETTING(    0x04, "600")
	PORT_DIPSETTING(    0x05, "1200")
	PORT_DIPSETTING(    0x06, "1800")
	PORT_DIPSETTING(    0x07, "2000")
	PORT_DIPSETTING(    0x08, "2400")
	PORT_DIPSETTING(    0x09, "3600")
	PORT_DIPSETTING(    0x0a, "4800")
	PORT_DIPSETTING(    0x0b, "7200")
	PORT_DIPSETTING(    0x0c, "9600")
	PORT_DIPSETTING(    0x0d, "19200")
	PORT_DIPNAME( 0x30, 0x00, "Parity")              PORT_DIPLOCATION("SW401:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR(None))
	PORT_DIPSETTING(    0x10, "Odd")
	PORT_DIPSETTING(    0x20, "None")
	PORT_DIPSETTING(    0x30, "Even")
	PORT_DIPNAME( 0x40, 0x00, "Parity Type")         PORT_DIPLOCATION("SW401:7")
	PORT_DIPSETTING(    0x00, DEF_STR(Normal))
	PORT_DIPSETTING(    0x40, "Stick")
	PORT_DIPNAME( 0x80, 0x80, "Duplex")              PORT_DIPLOCATION("SW401:8")
	PORT_DIPSETTING(    0x00, "Half")
	PORT_DIPSETTING(    0x80, "Full")

	PORT_START("SW402")
	PORT_DIPNAME( 0x01, 0x00, "Cursor")              PORT_DIPLOCATION("SW402:1")
	PORT_DIPSETTING(    0x00, "Underline")
	PORT_DIPSETTING(    0x01, "Block")
	PORT_DIPNAME( 0x02, 0x00, "Keyclick")            PORT_DIPLOCATION("SW402:2")
	PORT_DIPSETTING(    0x02, DEF_STR(No))
	PORT_DIPSETTING(    0x00, DEF_STR(Yes))
	PORT_DIPNAME( 0x04, 0x00, "Wrap at EOL")         PORT_DIPLOCATION("SW402:3")
	PORT_DIPSETTING(    0x00, DEF_STR(No))
	PORT_DIPSETTING(    0x04, DEF_STR(Yes))
	PORT_DIPNAME( 0x08, 0x00, "Auto LF on CR")       PORT_DIPLOCATION("SW402:4")
	PORT_DIPSETTING(    0x00, DEF_STR(No))
	PORT_DIPSETTING(    0x08, DEF_STR(Yes))
	PORT_DIPNAME( 0x10, 0x00, "Auto CR on LF")       PORT_DIPLOCATION("SW402:5")
	PORT_DIPSETTING(    0x00, DEF_STR(No))
	PORT_DIPSETTING(    0x10, DEF_STR(Yes))
	PORT_DIPNAME( 0x20, 0x00, "Mode")                PORT_DIPLOCATION("SW402:6")
	PORT_DIPSETTING(    0x00, "Heath/VT52")
	PORT_DIPSETTING(    0x20, "ANSI")
	PORT_DIPNAME( 0x40, 0x00, "Keypad Shifted")      PORT_DIPLOCATION("SW402:7")
	PORT_DIPSETTING(    0x00, DEF_STR(No))
	PORT_DIPSETTING(    0x40, DEF_STR(Yes))
	PORT_DIPNAME( 0x80, 0x00, "Refresh")             PORT_DIPLOCATION("SW402:8")
	PORT_DIPSETTING(    0x00, "60Hz")
	PORT_DIPSETTING(    0x80, "50Hz")
INPUT_PORTS_END


static INPUT_PORTS_START( super19 )
  PORT_INCLUDE( tlb )

  PORT_MODIFY("SW401")
	PORT_DIPNAME( 0x0f, 0x0c, "Baud Rate")           PORT_DIPLOCATION("SW401:1,2,3,4")
	PORT_DIPSETTING(    0x01, "110")
	PORT_DIPSETTING(    0x02, "150")
	PORT_DIPSETTING(    0x03, "300")
	PORT_DIPSETTING(    0x04, "600")
	PORT_DIPSETTING(    0x05, "1200")
	PORT_DIPSETTING(    0x06, "1800")
	PORT_DIPSETTING(    0x07, "2000")
	PORT_DIPSETTING(    0x08, "2400")
	PORT_DIPSETTING(    0x09, "3600")
	PORT_DIPSETTING(    0x0a, "4800")
	PORT_DIPSETTING(    0x0b, "7200")
	PORT_DIPSETTING(    0x0c, "9600")
	PORT_DIPSETTING(    0x0d, "19200")
	PORT_DIPSETTING(    0x0e, "38400")
	PORT_DIPNAME( 0x70, 0x00, "8 bit mode")          PORT_DIPLOCATION("SW401:5,6,7")
	PORT_DIPSETTING(    0x00, "Mode A/0 - 8th bit ignored, sent as 0")
	PORT_DIPSETTING(    0x10, "Mode B/1 - 8th bit ignored, sent as 1")
	PORT_DIPSETTING(    0x20, "Mode C/2 - 8 bit escape mode")
	PORT_DIPSETTING(    0x30, "Mode D/3 - 8 bit escape mode, invert 8th bit")
	PORT_DIPSETTING(    0x40, "Mode E/4 - 8 bit data mode")
	PORT_DIPSETTING(    0x50, "Mode F/5 - 8 bit data mode, invert 8th bit")
	PORT_DIPSETTING(    0x60, "7 bit data with odd parity, 8th bit ignored on input")
	PORT_DIPSETTING(    0x70, "7 bit data with even parity, 8th bit ignored on input")
	PORT_DIPNAME( 0x80, 0x80, "Duplex")              PORT_DIPLOCATION("SW401:8")
	PORT_DIPSETTING(    0x00, "Half")
	PORT_DIPSETTING(    0x80, "Full")

	PORT_MODIFY("SW402")
	PORT_DIPNAME( 0x02, 0x00, "Transmit mode")       PORT_DIPLOCATION("SW402:2")
	PORT_DIPSETTING(    0x00, DEF_STR(Normal))
	PORT_DIPSETTING(    0x02, "Slow")
	PORT_DIPNAME( 0x80, 0x00, "DEC Keypad Codes")    PORT_DIPLOCATION("SW402:8")
	PORT_DIPSETTING(    0x00, "Off")
	PORT_DIPSETTING(    0x80, "On")

INPUT_PORTS_END

static INPUT_PORTS_START( ultra19 )
	PORT_INCLUDE( tlb )

	PORT_MODIFY("SW401")
	PORT_DIPNAME( 0x07, 0x05, "Baud Rate")           PORT_DIPLOCATION("SW401:1,2,3")
	PORT_DIPSETTING(    0x00, "110")
	PORT_DIPSETTING(    0x01, "300")
	PORT_DIPSETTING(    0x02, "1200")
	PORT_DIPSETTING(    0x03, "2400")
	PORT_DIPSETTING(    0x04, "4800")
	PORT_DIPSETTING(    0x05, "9600")
	PORT_DIPSETTING(    0x06, "19200")
	PORT_DIPSETTING(    0x07, "38400")
	PORT_DIPNAME( 0x08, 0x00, "Parity")              PORT_DIPLOCATION("SW401:4")
	PORT_DIPSETTING(    0x00, "Disabled")
	PORT_DIPSETTING(    0x08, "Enabled")

	PORT_DIPNAME( 0x10, 0x00, "Parity Type")         PORT_DIPLOCATION("SW401:5")
	PORT_DIPSETTING(    0x00, "Odd")
	PORT_DIPSETTING(    0x10, "Even")
	PORT_DIPNAME( 0x20, 0x00, "Data Size")           PORT_DIPLOCATION("SW401:6")
	PORT_DIPSETTING(    0x00, "8-bit")
	PORT_DIPSETTING(    0x20, "7-bit")
	PORT_DIPNAME( 0x40, 0x40, "Duplex")              PORT_DIPLOCATION("SW401:7")
	PORT_DIPSETTING(    0x00, "Half")
	PORT_DIPSETTING(    0x40, "Full")
	PORT_DIPNAME( 0x80, 0x80, "Software Handshaking")  PORT_DIPLOCATION("SW401:8")
	PORT_DIPSETTING(    0x00, "Enabled")
	PORT_DIPSETTING(    0x80, "Disabled")

	PORT_MODIFY("SW402")
	PORT_DIPNAME( 0x08, 0x00, "Keypad Shifted")      PORT_DIPLOCATION("SW402:4")
	PORT_DIPSETTING(    0x00, DEF_STR(No))
	PORT_DIPSETTING(    0x08, DEF_STR(Yes))
	PORT_DIPNAME( 0x10, 0x00, "Default Key Values")  PORT_DIPLOCATION("SW402:5")
	PORT_DIPSETTING(    0x00, "HDOS Values")
	PORT_DIPSETTING(    0x10, "CP/M Values")
	PORT_DIPNAME( 0x60, 0x60, "Cursor Blink")        PORT_DIPLOCATION("SW402:6,7")
	PORT_DIPSETTING(    0x00, "Steady")
	PORT_DIPSETTING(    0x20, "Invisible")
	PORT_DIPSETTING(    0x40, "Fast Blink")
	PORT_DIPSETTING(    0x60, "Slow Blink")
	PORT_DIPNAME( 0x80, 0x00, "Interlace Scan Mode") PORT_DIPLOCATION("SW402:8")
	PORT_DIPSETTING(    0x00, "Off")
	PORT_DIPSETTING(    0x80, "On")

INPUT_PORTS_END

static INPUT_PORTS_START( watz19 )
	PORT_INCLUDE( tlb )

	PORT_MODIFY("SW401")
	PORT_DIPNAME( 0x0f, 0x0c, "Baud Rate")           PORT_DIPLOCATION("SW401:1,2,3,4")
	PORT_DIPSETTING(    0x00, "75")
	PORT_DIPSETTING(    0x01, "110")
	PORT_DIPSETTING(    0x02, "150")
	PORT_DIPSETTING(    0x03, "300")
	PORT_DIPSETTING(    0x04, "600")
	PORT_DIPSETTING(    0x05, "1200")
	PORT_DIPSETTING(    0x06, "1800")
	PORT_DIPSETTING(    0x07, "2000")
	PORT_DIPSETTING(    0x08, "2400")
	PORT_DIPSETTING(    0x09, "3600")
	PORT_DIPSETTING(    0x0a, "4800")
	PORT_DIPSETTING(    0x0b, "7200")
	PORT_DIPSETTING(    0x0c, "9600")
	PORT_DIPSETTING(    0x0d, "19200")
	PORT_DIPSETTING(    0x0e, "38400")
	PORT_DIPSETTING(    0x0f, "134.5")
	PORT_DIPNAME( 0x40, 0x00, "Word Size")           PORT_DIPLOCATION("SW401:7")
	PORT_DIPSETTING(    0x00, "8-bit Word")
	PORT_DIPSETTING(    0x40, "7-bit Word")
	PORT_DIPNAME( 0x80, 0x80, "Duplex")              PORT_DIPLOCATION("SW401:8")
	PORT_DIPSETTING(    0x00, "Half")
	PORT_DIPSETTING(    0x80, "Full")

INPUT_PORTS_END


ROM_START( h19 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	// Original
	ROM_LOAD( "2732_444-46_h19code.bin", 0x0000, 0x1000, CRC(f4447da0) SHA1(fb4093d5b763be21a9580a0defebed664b1f7a7b))

	ROM_REGION( 0x0800, "chargen", 0 )
	// Original font dump
	ROM_LOAD( "2716_444-29_h19font.bin", 0x0000, 0x0800, CRC(d595ac1d) SHA1(130fb4ea8754106340c318592eec2d8a0deaf3d0))

	ROM_REGION( 0x1000, "keyboard", 0 )
	// Original dump
	ROM_LOAD( "2716_444-37_h19keyb.bin", 0x0000, 0x0800, CRC(5c3e6972) SHA1(df49ce64ae48652346a91648c58178a34fb37d3c))
ROM_END

ROM_START( super19 )
	// Super H19 ROM
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "2732_super19_h447.bin", 0x0000, 0x1000, CRC(6c51aaa6) SHA1(5e368b39fe2f1af44a905dc474663198ab630117))

	ROM_REGION( 0x0800, "chargen", 0 )
	// Original font dump
	ROM_LOAD( "2716_444-29_h19font.bin", 0x0000, 0x0800, CRC(d595ac1d) SHA1(130fb4ea8754106340c318592eec2d8a0deaf3d0))

	ROM_REGION( 0x1000, "keyboard", 0 )
	// Original dump
	ROM_LOAD( "2716_444-37_h19keyb.bin", 0x0000, 0x0800, CRC(5c3e6972) SHA1(df49ce64ae48652346a91648c58178a34fb37d3c))
ROM_END

ROM_START( watz19 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("watzman-a")

	// Watzman ROM
	ROM_SYSTEM_BIOS(0, "watzman", "Watzman")
	ROMX_LOAD("watzman.bin", 0x0000, 0x1000, CRC(8168b6dc) SHA1(bfaebb9d766edbe545d24bc2b6630be4f3aa0ce9), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "watzman-a", "Watzman w/clock persists after reset")
	ROMX_LOAD("watzman-a.bin", 0x0000, 0x1000, CRC(1f7553e9) SHA1(ac6ddb12b4fb46c1a0ad08ee43978ad3153b51aa), ROM_BIOS(1))

	ROM_REGION( 0x0800, "chargen", 0 )
	// Original font dump
	ROM_LOAD( "2716_444-29_h19font.bin", 0x0000, 0x0800, CRC(d595ac1d) SHA1(130fb4ea8754106340c318592eec2d8a0deaf3d0))
	// Watzman keyboard
	ROM_REGION( 0x1000, "keyboard", 0 )
	ROM_LOAD( "keybd.bin", 0x0000, 0x0800, CRC(58dc8217) SHA1(1b23705290bdf9fc6342065c6a528c04bff67b13))
ROM_END

ROM_START( ultra19 )
	// ULTRA ROM
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "2532_h19_ultra_firmware.bin", 0x0000, 0x1000, CRC(8ad4cdb4) SHA1(d6e1fc37a1f52abfce5e9adb1819e0030bed1df3))

	ROM_REGION( 0x0800, "chargen", 0 )
	// Original font dump
	ROM_LOAD( "2716_444-29_h19font.bin", 0x0000, 0x0800, CRC(d595ac1d) SHA1(130fb4ea8754106340c318592eec2d8a0deaf3d0))
	// Watzman keyboard
	ROM_REGION( 0x1000, "keyboard", 0 )
	ROM_LOAD( "2716_h19_ultra_keyboard.bin", 0x0000, 0x0800, CRC(76130c92) SHA1(ca39c602af48505139d2750a084b5f8f0e662ff7))
ROM_END


ioport_constructor heath_tlb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(tlb);
}

const tiny_rom_entry *heath_tlb_device::device_rom_region() const
{
	return ROM_NAME(h19);
}

void heath_tlb_device::serial_out_b(uint8_t data)
{
	m_write_sd(data);
}

void heath_tlb_device::cb1_w(int state)
{
	m_ace->rx_w(state);
}

void heath_tlb_device::device_add_mconfig(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, H19_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &heath_tlb_device::mem_map);
	m_maincpu->set_addrmap(AS_IO, &heath_tlb_device::io_map);

	// video hardware
	// TODO: make configurable, Heath offered 2 different CRTs - White, Green
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_raw(MC6845_CLOCK * 45 / 2, 900, 260, 0, 640, 0, 250);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	GFXDECODE(config, "gfxdecode", m_palette, gfx_h19);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	MC6845(config, m_crtc, MC6845_CLOCK);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(true);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(heath_tlb_device::crtc_update_row));
	m_crtc->out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_NMI); // frame pulse

	// serial port
	INS8250(config, m_ace, INS8250_CLOCK);
	m_ace->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ace->out_tx_callback().set(FUNC(heath_tlb_device::serial_out_b));

	// keyboard
	MM5740(config, m_mm5740, MM5740_CLOCK);
	m_mm5740->x_cb<1>().set_ioport("X1");
	m_mm5740->x_cb<2>().set_ioport("X2");
	m_mm5740->x_cb<3>().set_ioport("X3");
	m_mm5740->x_cb<4>().set_ioport("X4");
	m_mm5740->x_cb<5>().set_ioport("X5");
	m_mm5740->x_cb<6>().set_ioport("X6");
	m_mm5740->x_cb<7>().set_ioport("X7");
	m_mm5740->x_cb<8>().set_ioport("X8");
	m_mm5740->x_cb<9>().set_ioport("X9");
	m_mm5740->shift_cb().set(FUNC(heath_tlb_device::mm5740_shift_r));
	m_mm5740->control_cb().set(FUNC(heath_tlb_device::mm5740_control_r));
	m_mm5740->data_ready_cb().set(FUNC(heath_tlb_device::mm5740_data_ready_w));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, H19_BEEP_FRQ).add_route(ALL_OUTPUTS, "mono", 0.05);
}

heath_super19_tlb_device::heath_super19_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	heath_tlb_device(mconfig, HEATH_SUPER19, tag, owner, clock)
{
}

const tiny_rom_entry *heath_super19_tlb_device::device_rom_region() const
{
	return ROM_NAME(super19);
}

ioport_constructor heath_super19_tlb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(super19);
}

heath_watz_tlb_device::heath_watz_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	heath_tlb_device(mconfig, HEATH_WATZ, tag, owner, clock)
{
}

const tiny_rom_entry *heath_watz_tlb_device::device_rom_region() const
{
	return ROM_NAME(watz19);
}

ioport_constructor heath_watz_tlb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(watz19);
}

heath_ultra_tlb_device::heath_ultra_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	heath_tlb_device(mconfig, HEATH_ULTRA, tag, owner, clock)
{
}

void heath_ultra_tlb_device::device_add_mconfig(machine_config &config)
{
	heath_tlb_device::device_add_mconfig(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &heath_ultra_tlb_device::mem_map);
}

void heath_ultra_tlb_device::mem_map(address_map &map)
{
	heath_tlb_device::mem_map(map);

	// update rom mirror setting to allow page 2 memory
	map(0x0000, 0x0fff).mirror(0x2000).rom();

	// Page 2 memory
	map(0x1000, 0x1fff).mirror(0x2000).ram();
}

const tiny_rom_entry *heath_ultra_tlb_device::device_rom_region() const
{
	return ROM_NAME(ultra19);
}

ioport_constructor heath_ultra_tlb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ultra19);
}
