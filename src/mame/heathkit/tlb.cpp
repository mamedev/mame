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

    Input can also come from the serial port.

  Known Issues:
    - With gp19
      - 49/50 row mode only shows half the screen.
      - In 49/50 row mode, character descenders are cut off.
      - Screen saver does not disable the screen
    - With Superset
      - Screensaver freezes the screen instead of blanking the screen

  Not Currently Implemented
    - With Imaginator I-100
      - Support for Tektronics Emulation (missing ROM image)
    - With SigmaSoft IGC
      - Interlace video support (with higher resolution)
      - Joystick support
      - Centronics support
      - Lightpen support
      - Alternate Font ROM support (missing ROM image)
    - With Super19
      - Interlace video
      - Alternate Font19 alt 128 mode and extended 256 mode (missing ROM images)
      - ANSI character set selection (missing ROM image)
      - light pen support
    - With UltraROM
      - Interace mode

****************************************************************************/
/***************************************************************************

  Memory Layout

    The U435 three-to-eight line decoder uses A14 and A15 to generate
    three memory address spaces:

    Address             Description
    ----------------------------------------------------
      0x0000            Program ROM
      0x4000            Scratchpad RAM
      0xF800 (0xC000)   Display Memory


  Port Layout

    Only address lines A5, A6, A7 are used by the U442 three-to-eight
    line decoder

    Address   Description
    ----------------------------------------------------
      0x00    Power-up configuration (primary - SW401)
      0x20    Power-up configuration (secondary - SW402)
      0x40    INS8250 ACE (communications)
      0x60    MC6845 CRT controller
      0x80    MM5740 Keyboard encoder
      0xA0    Keyboard status
      0xC0    Trigger key click
      0xE0    Trigger bell

****************************************************************************/

#include "emu.h"

#include "tlb.h"

#include <algorithm>

#define LOG_REG (1U << 1)    // Shows register setup

//#define VERBOSE (LOG_REG)

#include "logmacro.h"

#define LOGREG(...)        LOGMASKED(LOG_REG, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

// Clocks
static constexpr XTAL MASTER_CLOCK     = XTAL(12'288'000);

// DOT clocks
static constexpr XTAL BASE_DOT_CLOCK   = MASTER_CLOCK;
static constexpr XTAL GP19_DOT_CLOCK_1 = XTAL(20'282'000);   // 132 columns
static constexpr XTAL GP19_DOT_CLOCK_2 = XTAL(12'292'000);   // 80 columns
static constexpr XTAL GP19_DOT_CLOCK_3 = XTAL(10'644'000);   // Graphics mode

// Standard H19 used a 2.048 MHz clock for Z80
static constexpr XTAL H19_CLOCK        = MASTER_CLOCK / 6;
static constexpr XTAL H19_3MHZ         = MASTER_CLOCK / 4;
static constexpr XTAL H19_4MHZ         = MASTER_CLOCK / 3;
static constexpr XTAL H19_6MHZ         = MASTER_CLOCK / 2;

static constexpr XTAL INS8250_CLOCK    = MASTER_CLOCK / 4;

// Beep Frequency is 1 KHz
static constexpr XTAL H19_BEEP_FRQ     = (H19_CLOCK / 2048);

// Capacitor value in pF
static constexpr u32 H19_KEY_DEBOUNCE_CAPACITOR = 5000;
static const     u32 MM5740_CLOCK               = mm5740_device::calc_effective_clock_key_debounce(H19_KEY_DEBOUNCE_CAPACITOR);

// Keyboard flag masks
static constexpr u8 KB_STATUS_SHIFT_KEYS_MASK      = 0x01;
static constexpr u8 KB_STATUS_CONTROL_KEY_MASK     = 0x10;
static constexpr u8 KB_STATUS_KEYBOARD_STROBE_MASK = 0x80;

DEFINE_DEVICE_TYPE(HEATH_TLB_CONNECTOR, heath_tlb_connector,          "heath_tlb_connector",          "Heath Terminal Logic board connector abstraction")

DEFINE_DEVICE_TYPE(HEATH_TLB,           heath_tlb_device,             "heath_tlb",                    "Heath Terminal Logic Board")
DEFINE_DEVICE_TYPE(HEATH_SUPER19,       heath_super19_tlb_device,     "heath_super19_tlb",            "Heath Terminal Logic Board w/Super19 ROM")
DEFINE_DEVICE_TYPE(HEATH_SUPERSET,      heath_superset_tlb_device,    "heath_superset_tlb",           "Heath Terminal Logic Board w/Superset ROM")
DEFINE_DEVICE_TYPE(HEATH_ULTRA,         heath_ultra_tlb_device,       "heath_ultra_tlb",              "Heath Terminal Logic Board w/Ultra ROM")
DEFINE_DEVICE_TYPE(HEATH_WATZ,          heath_watz_tlb_device,        "heath_watz_tlb",               "Heath Terminal Logic Board w/Watzman ROM")
DEFINE_DEVICE_TYPE(HEATH_GP19,          heath_gp19_tlb_device,        "heath_gp19_tlb",               "Heath Terminal Logic Board plus Northwest Digital Systems GP-19")
DEFINE_DEVICE_TYPE(HEATH_IMAGINATOR,    heath_imaginator_tlb_device,  "heath_imaginator_tlb",         "Heath Terminal Logic Board plus Cleveland Codonics Imaginator I-100")

// Devices for the terminal boards compatible with SigmaSoft IGC
DEFINE_DEVICE_TYPE(HEATH_IGC,           heath_igc_tlb_device,         "heath_igc_tlb_device",         "Heath Terminal Logic Board plus SigmaSoft Interactive Graphics Controller")
DEFINE_DEVICE_TYPE(HEATH_IGC_SUPER19,   heath_igc_super19_tlb_device, "heath_igc_super19_tlb_device", "Heath Terminal Logic Board w/ Super19 ROM plus SigmaSoft Interactive Graphics Controller")
DEFINE_DEVICE_TYPE(HEATH_IGC_ULTRA,     heath_igc_ultra_tlb_device,   "heath_igc_ultra_tlb_device",   "Heath Terminal Logic Board w/ Ultra ROM plus SigmaSoft Interactive Graphics Controller")
DEFINE_DEVICE_TYPE(HEATH_IGC_WATZ,      heath_igc_watz_tlb_device,    "heath_igc_watz_tlb_device",    "Heath Terminal Logic Board w/Watzman ROM plus SigmaSoft Interactive Graphics Controller")


device_heath_tlb_card_interface::device_heath_tlb_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "heathtlbdevice"),
	m_slot(dynamic_cast<heath_tlb_connector *>(device.owner()))
{
}


/**
 * original Heath H19 functionality
 */
heath_tlb_device::heath_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	heath_tlb_device(mconfig, HEATH_TLB, tag, owner, clock)
{
}

heath_tlb_device::heath_tlb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_heath_tlb_card_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_screen(*this, "screen"),
	m_palette(*this, "palette"),
	m_crtc(*this, "crtc"),
	m_p_videoram(*this, "videoram"),
	m_p_chargen(*this, "chargen"),
	m_config(*this, "CONFIG"),
	m_ace(*this, "ins8250"),
	m_beep(*this, "beeper"),
	m_mm5740(*this, "mm5740"),
	m_kbdrom(*this, "keyboard"),
	m_kbspecial(*this, "MODIFIERS"),
	m_repeat_clock(*this, "repeatclock")
{
}

void heath_tlb_device::check_beep_state()
{
	if (!m_key_click_active && !m_bell_active)
	{
		m_beep->set_state(0);
	}
}

TIMER_CALLBACK_MEMBER(heath_tlb_device::key_click_off)
{
	m_key_click_active = false;

	check_beep_state();
}

TIMER_CALLBACK_MEMBER(heath_tlb_device::bell_off)
{
	m_bell_active = false;

	check_beep_state();
}

void heath_tlb_device::mem_map(address_map &map)
{
	map.unmap_value_high();

	// H19 ROM
	map(0x0000, 0x0fff).mirror(0x3000).rom();

	// Scratchpad memory
	map(0x4000, 0x40ff).mirror(0x3f00).ram();

	// Video Memory
	map(0xc000, 0xc7ff).mirror(0x3800).ram().share(m_p_videoram);
}

void heath_tlb_device::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).mirror(0x1f).portr("SW401");
	map(0x20, 0x20).mirror(0x1f).portr("SW402");
	map(0x40, 0x47).mirror(0x18).rw(m_ace, FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));
	map(0x60, 0x60).mirror(0x1a).select(0x04).w(FUNC(heath_tlb_device::crtc_addr_w));
	map(0x61, 0x61).mirror(0x1a).select(0x04).rw(FUNC(heath_tlb_device::crtc_reg_r), FUNC(heath_tlb_device::crtc_reg_w));
	map(0x80, 0x80).mirror(0x1f).r(FUNC(heath_tlb_device::kbd_key_r));
	map(0xa0, 0xa0).mirror(0x1f).r(FUNC(heath_tlb_device::kbd_flags_r));
	map(0xc0, 0xc0).mirror(0x1f).w(FUNC(heath_tlb_device::key_click_w));
	map(0xe0, 0xe0).mirror(0x1f).w(FUNC(heath_tlb_device::bell_w));
}

void heath_tlb_device::device_start()
{
	save_item(NAME(m_transchar));
	save_item(NAME(m_strobe));
	save_item(NAME(m_key_click_active));
	save_item(NAME(m_bell_active));
	save_item(NAME(m_reset_pending));
	save_item(NAME(m_right_shift));
	save_item(NAME(m_reset_key));
	save_item(NAME(m_keyboard_irq_raised));
	save_item(NAME(m_serial_irq_raised));
	save_item(NAME(m_break_key_irq_raised));
	save_item(NAME(m_allow_vsync_nmi));

	m_strobe               = false;
	m_key_click_active     = false;
	m_bell_active          = false;
	m_reset_pending        = false;
	m_right_shift          = false;
	m_reset_key            = false;
	m_keyboard_irq_raised  = false;
	m_serial_irq_raised    = false;
	m_break_key_irq_raised = false;
	m_allow_vsync_nmi      = false;

	m_key_click_timer      = timer_alloc(FUNC(heath_tlb_device::key_click_off), this);
	m_bell_timer           = timer_alloc(FUNC(heath_tlb_device::bell_off), this);

	// Flip bits in font ROM to avoid having to flip them during scan out since some
	// boards with graphics has the pixel graphic reversed from the font layout. Doing
	// it for all devices for consistency.
	for (size_t i = 0; i < m_p_chargen.length(); i++)
	{
		m_p_chargen[i] = bitswap<8>(m_p_chargen[i], 0, 1, 2, 3, 4, 5, 6, 7);
	}
}

void heath_tlb_device::device_reset()
{
	m_strobe           = false;
	m_key_click_active = false;
	m_bell_active      = false;
	m_allow_vsync_nmi  = false;

	ioport_value const cfg(m_config->read());

	// CPU clock speed
	switch (BIT(cfg, 0, 2))
	{
	case 0x01:
		m_maincpu->set_clock(H19_3MHZ);
		break;
	case 0x02:
		m_maincpu->set_clock(H19_4MHZ);
		break;
	case 0x03:
		m_maincpu->set_clock(H19_6MHZ);
		break;
	case 0x00:
	default:
		// Standard Clock
		m_maincpu->set_clock(H19_CLOCK);
		break;
	}

	// Set screen color
	switch (BIT(cfg, 2, 2))
	{
	case 0x01:
		m_screen->set_color(rgb_t::white());
		break;
	case 0x02:
		m_screen->set_color(rgb_t::amber());
		break;
	case 0x00:
	default:
		m_screen->set_color(rgb_t::green());
		break;
	}
}

void heath_tlb_device::key_click_w(u8 data)
{
	// Keyclick - 6 mSec
	m_beep->set_state(1);
	m_key_click_active = true;
	m_key_click_timer->adjust(attotime::from_msec(6));
}

void heath_tlb_device::bell_w(u8 data)
{
	// Bell (^G) - 200 mSec
	m_beep->set_state(1);
	m_bell_active = true;
	m_bell_timer->adjust(attotime::from_msec(200));
}

/**
 * MM5740 B Mapping to the ROM address
 */
u16 heath_tlb_device::translate_mm5740_b(u16 b)
{
	return bitswap<9>(b, 7, 6, 8, 5, 4, 3, 2, 1, 0);
}

u8 heath_tlb_device::kbd_key_r()
{
	m_strobe              = false;
	m_keyboard_irq_raised = false;

	set_irq_line();

	// high bit is for control key pressed, this is handled in the ROM,
	// no processing needed.
	return m_transchar;
}

u8 heath_tlb_device::kbd_flags_r()
{
	u16 modifiers = m_kbspecial->read();
	u8 rv         = modifiers & 0x7e;

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
	return ((m_kbspecial->read() & 0x120) != 0x120) ? 1 : 0;
}

int heath_tlb_device::mm5740_control_r()
{
	return (m_kbspecial->read() & KB_STATUS_CONTROL_KEY_MASK) ? 0 : 1;
}

void heath_tlb_device::mm5740_data_ready_w(int state)
{
	if (state)
	{
		u8 *decode            = m_kbdrom->base();

		m_transchar           = decode[translate_mm5740_b(m_mm5740->b_r())];
		m_strobe              = true;
		m_keyboard_irq_raised = true;

		set_irq_line();
	}
}

void heath_tlb_device::crtc_addr_w(offs_t reg, u8 val)
{
	m_allow_vsync_nmi = bool(BIT(reg, 2));

	m_crtc->address_w(val);
}

u8 heath_tlb_device::crtc_reg_r(offs_t reg)
{
	if (!machine().side_effects_disabled())
	{
		m_allow_vsync_nmi = bool(BIT(reg, 2));
	}

	return m_crtc->register_r();
}

void heath_tlb_device::crtc_reg_w(offs_t reg, u8 val)
{
	m_allow_vsync_nmi = bool(BIT(reg, 2));

	m_crtc->register_w(val);
}

void heath_tlb_device::crtc_vsync_w(int val)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, m_allow_vsync_nmi ? val : CLEAR_LINE);
}

void heath_tlb_device::serial_irq_w(int state)
{
	m_serial_irq_raised = bool(state);

	set_irq_line();
}

void heath_tlb_device::set_irq_line()
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0,
		(m_keyboard_irq_raised || m_serial_irq_raised || m_break_key_irq_raised) ?
		 ASSERT_LINE : CLEAR_LINE);
}

void heath_tlb_device::check_for_reset()
{
	if (m_reset_key && m_right_shift)
	{
		m_reset_pending = true;
		m_slot->reset_out(1);
		m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	}
	else if (m_reset_pending)
	{
		m_reset_pending = false;
		reset();
		m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		m_slot->reset_out(0);
	}
}

void heath_tlb_device::reset_key_w(int state)
{
	m_reset_key = (state == 0);

	check_for_reset();
}

void heath_tlb_device::right_shift_w(int state)
{
	m_right_shift = (state == 0);

	check_for_reset();
}

void heath_tlb_device::repeat_key_w(int state)
{
	// when repeat key pressed, set duty cycle to 50%, else 0%.
	m_repeat_clock->set_duty_cycle(state == 0 ? 0.5 : 0);
}

void heath_tlb_device::break_key_w(int state)
{
	m_break_key_irq_raised = (state == 0);

	set_irq_line();
}

MC6845_UPDATE_ROW(heath_tlb_device::crtc_update_row)
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u32               *p       = &bitmap.pix(y);

	if (de)
	{
		for (int x = 0; x < x_count; x++)
		{
			u8 inv = (x == cursor_x) ? 0xff : 0;
			u8 chr = m_p_videoram[(ma + x) & 0x7ff];

			if (chr & 0x80)
			{
				inv ^= 0xff;
				chr &= 0x7f;
			}

			// get pattern of pixels for that character scanline
			u8 const gfx = m_p_chargen[(chr << 4) | ra] ^ inv;

			// Display a scanline of a character (8 pixels)
			for (int b = 0; 8 > b; ++b)
			{
				*p++ = palette[BIT(gfx, b)];
			}
		}
	}
	else
	{
		std::fill_n(p, x_count * 8, palette[0]);
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

static GFXDECODE_START( gfx_h19 )
	GFXDECODE_ENTRY("chargen", 0x0000, h19_charlayout, 0, 1)
GFXDECODE_END


// Input ports
static INPUT_PORTS_START( tlb )

	PORT_START("MODIFIERS")
	// bit 0 - 0x001 connects to B8 of MM5740 - low if either shift key is
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CapsLock")   PORT_CODE(KEYCODE_CAPSLOCK)   PORT_TOGGLE
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Break")      PORT_CODE(KEYCODE_PAUSE)      PORT_WRITE_LINE_MEMBER(heath_tlb_device, break_key_w)
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("OffLine")    PORT_CODE(KEYCODE_F12)        PORT_TOGGLE
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL")       PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LeftShift")  PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Repeat")     PORT_CODE(KEYCODE_RALT)       PORT_WRITE_LINE_MEMBER(heath_tlb_device, repeat_key_w)
	// bit 7 - 0x080 is low if any key is pressed
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RightShift") PORT_CODE(KEYCODE_RSHIFT)     PORT_WRITE_LINE_MEMBER(heath_tlb_device, right_shift_w)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Reset")      PORT_CODE(KEYCODE_F10)        PORT_WRITE_LINE_MEMBER(heath_tlb_device, reset_key_w)

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
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("; :")        PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';')  PORT_CHAR(':')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("' \"")       PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("{ }")        PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('{')  PORT_CHAR('}')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return")     PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP-1 IL")    PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP-2 Down")  PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP-3 DL")    PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X3")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P")          PORT_CODE(KEYCODE_P)          PORT_CHAR('p')  PORT_CHAR('P')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[ ]")        PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[')  PORT_CHAR(']')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\ |")       PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Line Feed")  PORT_CODE(KEYCODE_RWIN)       PORT_CHAR(10)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEL")        PORT_CODE(KEYCODE_DEL)        PORT_CHAR(127)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP-4 LEFT")  PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP-5 HOME")  PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP-6 RIGHT") PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X4")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0 )")        PORT_CODE(KEYCODE_0)          PORT_CHAR('0')  PORT_CHAR(')')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("- _")        PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-')  PORT_CHAR('_')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("= +")        PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=')  PORT_CHAR('+')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("` ~")        PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`')  PORT_CHAR('~')
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
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z")          PORT_CODE(KEYCODE_Z)          PORT_CHAR('z')  PORT_CHAR('Z')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X")          PORT_CODE(KEYCODE_X)          PORT_CHAR('x')  PORT_CHAR('X')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C")          PORT_CODE(KEYCODE_C)          PORT_CHAR('c')  PORT_CHAR('C')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V")          PORT_CODE(KEYCODE_V)          PORT_CHAR('v')  PORT_CHAR('V')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B")          PORT_CODE(KEYCODE_B)          PORT_CHAR('b')  PORT_CHAR('B')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N")          PORT_CODE(KEYCODE_N)          PORT_CHAR('n')  PORT_CHAR('N')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M")          PORT_CODE(KEYCODE_M)          PORT_CHAR('m')  PORT_CHAR('M')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(", <")        PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')  PORT_CHAR('<')
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(". >")        PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')  PORT_CHAR('>')
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space")      PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')

	PORT_START("X7")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A")          PORT_CODE(KEYCODE_A)          PORT_CHAR('a')  PORT_CHAR('A')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S")          PORT_CODE(KEYCODE_S)          PORT_CHAR('s')  PORT_CHAR('S')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D")          PORT_CODE(KEYCODE_D)          PORT_CHAR('d')  PORT_CHAR('D')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F")          PORT_CODE(KEYCODE_F)          PORT_CHAR('f')  PORT_CHAR('F')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G")          PORT_CODE(KEYCODE_G)          PORT_CHAR('g')  PORT_CHAR('G')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H")          PORT_CODE(KEYCODE_H)          PORT_CHAR('h')  PORT_CHAR('H')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J")          PORT_CODE(KEYCODE_J)          PORT_CHAR('j')  PORT_CHAR('J')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K")          PORT_CODE(KEYCODE_K)          PORT_CHAR('k')  PORT_CHAR('K')
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L")          PORT_CODE(KEYCODE_L)          PORT_CHAR('l')  PORT_CHAR('L')
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Scroll")     PORT_CODE(KEYCODE_F11)

	PORT_START("X8")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q")          PORT_CODE(KEYCODE_Q)          PORT_CHAR('q')  PORT_CHAR('Q')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W")          PORT_CODE(KEYCODE_W)          PORT_CHAR('w')  PORT_CHAR('W')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E")          PORT_CODE(KEYCODE_E)          PORT_CHAR('e')  PORT_CHAR('E')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R")          PORT_CODE(KEYCODE_R)          PORT_CHAR('r')  PORT_CHAR('R')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T")          PORT_CODE(KEYCODE_T)          PORT_CHAR('t')  PORT_CHAR('T')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y")          PORT_CODE(KEYCODE_Y)          PORT_CHAR('y')  PORT_CHAR('Y')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U")          PORT_CODE(KEYCODE_U)          PORT_CHAR('u')  PORT_CHAR('U')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I")          PORT_CODE(KEYCODE_I)          PORT_CHAR('i')  PORT_CHAR('I')
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O")          PORT_CODE(KEYCODE_O)          PORT_CHAR('o')  PORT_CHAR('O')
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab")        PORT_CODE(KEYCODE_TAB)        PORT_CHAR(9)

	PORT_START("X9")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 !")        PORT_CODE(KEYCODE_1)          PORT_CHAR('1')  PORT_CHAR('!')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 @")        PORT_CODE(KEYCODE_2)          PORT_CHAR('2')  PORT_CHAR('@')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 #")        PORT_CODE(KEYCODE_3)          PORT_CHAR('3')  PORT_CHAR('#')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 $")        PORT_CODE(KEYCODE_4)          PORT_CHAR('4')  PORT_CHAR('$')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 %")        PORT_CODE(KEYCODE_5)          PORT_CHAR('5')  PORT_CHAR('%')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 ^")        PORT_CODE(KEYCODE_6)          PORT_CHAR('6')  PORT_CHAR('^')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 &")        PORT_CODE(KEYCODE_7)          PORT_CHAR('7')  PORT_CHAR('&')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 *")        PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('*')
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 (")        PORT_CODE(KEYCODE_9)          PORT_CHAR('9')  PORT_CHAR('(')
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc")        PORT_CODE(KEYCODE_ESC)        PORT_CHAR(27)

	PORT_START("SW401")
	PORT_DIPNAME( 0x0f, 0x0c, "Baud Rate")                                   PORT_DIPLOCATION("SW401:1,2,3,4")
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
	PORT_DIPNAME( 0x30, 0x00, "Parity")                                      PORT_DIPLOCATION("SW401:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x10, "Odd")
	PORT_DIPSETTING(    0x20, DEF_STR( None ) )
	PORT_DIPSETTING(    0x30, "Even")
	PORT_DIPNAME( 0x40, 0x00, "Parity Type")                                 PORT_DIPLOCATION("SW401:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, "Stick")
	PORT_DIPNAME( 0x80, 0x80, "Duplex")                                      PORT_DIPLOCATION("SW401:8")
	PORT_DIPSETTING(    0x00, "Half")
	PORT_DIPSETTING(    0x80, "Full")

	PORT_START("SW402")
	PORT_DIPNAME( 0x01, 0x00, "Cursor")                                      PORT_DIPLOCATION("SW402:1")
	PORT_DIPSETTING(    0x00, "Underline")
	PORT_DIPSETTING(    0x01, "Block")
	PORT_DIPNAME( 0x02, 0x00, "Keyclick")                                    PORT_DIPLOCATION("SW402:2")
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x00, "Wrap at EOL")                                 PORT_DIPLOCATION("SW402:3")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, "Auto LF on CR")                               PORT_DIPLOCATION("SW402:4")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, "Auto CR on LF")                               PORT_DIPLOCATION("SW402:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Mode")                                        PORT_DIPLOCATION("SW402:6")
	PORT_DIPSETTING(    0x00, "Heath/VT52")
	PORT_DIPSETTING(    0x20, "ANSI")
	PORT_DIPNAME( 0x40, 0x00, "Keypad Shifted")                              PORT_DIPLOCATION("SW402:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, "Refresh")                                     PORT_DIPLOCATION("SW402:8")
	PORT_DIPSETTING(    0x00, "60Hz")
	PORT_DIPSETTING(    0x80, "50Hz")

	PORT_START("CONFIG")
	PORT_CONFNAME(0x03, 0x00, "CPU Clock")
	PORT_CONFSETTING(   0x00, "2 MHz")
	PORT_CONFSETTING(   0x01, "3 MHz")
	PORT_CONFSETTING(   0x02, "4 MHz")
	PORT_CONFSETTING(   0x03, "6 MHz")
	PORT_CONFNAME(0x0C, 0x00, "CRT Color")
	PORT_CONFSETTING(   0x00, "Green")
	PORT_CONFSETTING(   0x04, "White")
	PORT_CONFSETTING(   0x08, "Amber")
INPUT_PORTS_END


static INPUT_PORTS_START( super19 )
  PORT_INCLUDE( tlb )

  PORT_MODIFY("SW401")
	PORT_DIPNAME( 0x0f, 0x0c, "Baud Rate")                                   PORT_DIPLOCATION("SW401:1,2,3,4")
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
	PORT_DIPNAME( 0x70, 0x00, "8 bit mode")                                  PORT_DIPLOCATION("SW401:5,6,7")
	PORT_DIPSETTING(    0x00, "Mode A/0 - 8th bit ignored, sent as 0")
	PORT_DIPSETTING(    0x10, "Mode B/1 - 8th bit ignored, sent as 1")
	PORT_DIPSETTING(    0x20, "Mode C/2 - 8 bit escape mode")
	PORT_DIPSETTING(    0x30, "Mode D/3 - 8 bit escape mode, invert 8th bit")
	PORT_DIPSETTING(    0x40, "Mode E/4 - 8 bit data mode")
	PORT_DIPSETTING(    0x50, "Mode F/5 - 8 bit data mode, invert 8th bit")
	PORT_DIPSETTING(    0x60, "7 bit data with odd parity, 8th bit ignored on input")
	PORT_DIPSETTING(    0x70, "7 bit data with even parity, 8th bit ignored on input")
	PORT_DIPNAME( 0x80, 0x80, "Duplex")                                      PORT_DIPLOCATION("SW401:8")
	PORT_DIPSETTING(    0x00, "Half")
	PORT_DIPSETTING(    0x80, "Full")

	PORT_MODIFY("SW402")
	PORT_DIPNAME( 0x02, 0x00, "Transmit mode")                               PORT_DIPLOCATION("SW402:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, "Slow")
	PORT_DIPNAME( 0x80, 0x00, "DEC Keypad Codes")                            PORT_DIPLOCATION("SW402:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( superset )
	PORT_INCLUDE( tlb )

	PORT_MODIFY("SW401")
	PORT_DIPNAME( 0x0f, 0x0c, "Baud Rate")                                   PORT_DIPLOCATION("SW401:1,2,3,4")
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
	PORT_DIPNAME( 0x10, 0x10, "Duplex")                                      PORT_DIPLOCATION("SW401:5")
	PORT_DIPSETTING(    0x00, "Half")
	PORT_DIPSETTING(    0x80, "Full")
	PORT_DIPNAME( 0x20, 0x00, "Word Length")                                 PORT_DIPLOCATION("SW401:6")
	PORT_DIPSETTING(    0x00, "8-bit" )
	PORT_DIPSETTING(    0x20, "7-bit" )
	PORT_DIPNAME( 0x40, 0x00, "Parity Type")                                 PORT_DIPLOCATION("SW401:7")
	PORT_DIPSETTING(    0x00, DEF_STR( None )  )
	PORT_DIPSETTING(    0x40, "Even")
	PORT_DIPNAME( 0x80, 0x80, "Parity")                                      PORT_DIPLOCATION("SW401:8")
	PORT_DIPSETTING(    0x00, "Disabled")
	PORT_DIPSETTING(    0x80, "Enabled")
INPUT_PORTS_END


static INPUT_PORTS_START( ultra19 )
	PORT_INCLUDE( tlb )

	PORT_MODIFY("SW401")
	PORT_DIPNAME( 0x07, 0x05, "Baud Rate")                                   PORT_DIPLOCATION("SW401:1,2,3")
	PORT_DIPSETTING(    0x00, "110")
	PORT_DIPSETTING(    0x01, "300")
	PORT_DIPSETTING(    0x02, "1200")
	PORT_DIPSETTING(    0x03, "2400")
	PORT_DIPSETTING(    0x04, "4800")
	PORT_DIPSETTING(    0x05, "9600")
	PORT_DIPSETTING(    0x06, "19200")
	PORT_DIPSETTING(    0x07, "38400")
	PORT_DIPNAME( 0x08, 0x00, "Parity")                                      PORT_DIPLOCATION("SW401:4")
	PORT_DIPSETTING(    0x00, "Disabled")
	PORT_DIPSETTING(    0x08, "Enabled")
	PORT_DIPNAME( 0x10, 0x00, "Parity Type")                                 PORT_DIPLOCATION("SW401:5")
	PORT_DIPSETTING(    0x00, "Odd")
	PORT_DIPSETTING(    0x10, "Even")
	PORT_DIPNAME( 0x20, 0x00, "Data Size")                                   PORT_DIPLOCATION("SW401:6")
	PORT_DIPSETTING(    0x00, "8-bit")
	PORT_DIPSETTING(    0x20, "7-bit")
	PORT_DIPNAME( 0x40, 0x40, "Duplex")                                      PORT_DIPLOCATION("SW401:7")
	PORT_DIPSETTING(    0x00, "Half")
	PORT_DIPSETTING(    0x40, "Full")
	PORT_DIPNAME( 0x80, 0x80, "Software Handshaking")                        PORT_DIPLOCATION("SW401:8")
	PORT_DIPSETTING(    0x00, "Enabled")
	PORT_DIPSETTING(    0x80, "Disabled")

	PORT_MODIFY("SW402")
	PORT_DIPNAME( 0x08, 0x00, "Keypad Shifted")                              PORT_DIPLOCATION("SW402:4")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, "Default Key Values")                          PORT_DIPLOCATION("SW402:5")
	PORT_DIPSETTING(    0x00, "HDOS Values")
	PORT_DIPSETTING(    0x10, "CP/M Values")
	PORT_DIPNAME( 0x60, 0x60, "Cursor Blink")                                PORT_DIPLOCATION("SW402:6,7")
	PORT_DIPSETTING(    0x00, "Steady")
	PORT_DIPSETTING(    0x20, "Invisible")
	PORT_DIPSETTING(    0x40, "Fast Blink")
	PORT_DIPSETTING(    0x60, "Slow Blink")
	PORT_DIPNAME( 0x80, 0x00, "Interlace Scan Mode")                         PORT_DIPLOCATION("SW402:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( watz19 )
	PORT_INCLUDE( tlb )

	PORT_MODIFY("SW401")
	PORT_DIPNAME( 0x0f, 0x0c, "Baud Rate")                                   PORT_DIPLOCATION("SW401:1,2,3,4")
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
	PORT_DIPNAME( 0x40, 0x00, "Word Size")                                   PORT_DIPLOCATION("SW401:7")
	PORT_DIPSETTING(    0x00, "8-bit Word")
	PORT_DIPSETTING(    0x40, "7-bit Word")
	PORT_DIPNAME( 0x80, 0x80, "Duplex")                                      PORT_DIPLOCATION("SW401:8")
	PORT_DIPSETTING(    0x00, "Half")
	PORT_DIPSETTING(    0x80, "Full")
INPUT_PORTS_END


static INPUT_PORTS_START( gp19 )
	PORT_INCLUDE( tlb )

	PORT_MODIFY("SW402")
	PORT_DIPNAME( 0x80, 0x00, "Automatic Holdscreen")                        PORT_DIPLOCATION("SW402:8")
	PORT_DIPSETTING(    0x00, "Disabled (VT100 mode)")
	PORT_DIPSETTING(    0x80, "Enabled (Z19 mode)")

	PORT_START("SW1")
	PORT_DIPNAME( 0x03, 0x01, "Trailing Characters for Tektronix Message")   PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x01, "CR")
	PORT_DIPSETTING(    0x02, DEF_STR( None ) )
	PORT_DIPSETTING(    0x03, "CR,EOT")
	PORT_DIPNAME( 0x04, 0x01, "Shift Key")                                   PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, "Shift key inverts CAPS LOCK")
	PORT_DIPNAME( 0x08, 0x00, "Terminal Transmission Rate")                  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "Only limited by baud rate")
	PORT_DIPSETTING(    0x08, "One character per 16.7 msec")
	PORT_DIPNAME( 0x10, 0x10, "Character Set")                               PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "VT100")
	PORT_DIPSETTING(    0x10, "Zenith")
	PORT_DIPNAME( 0x20, 0x00, "ANSI Mode")                                   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "Zenith ANSI")
	PORT_DIPSETTING(    0x20, "EDT compatible")
	PORT_DIPNAME( 0x40, 0x00, "Tektronix Character Wrap")                    PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "Column 74")
	PORT_DIPSETTING(    0x40, "Column 73")
	PORT_DIPNAME( 0x80, 0x00, "Blank Screen")                                PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "After 10 mins")
	PORT_DIPSETTING(    0x80, "Never")
INPUT_PORTS_END


ROM_START( h19 )
	// Original terminal code
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "2732_444-46_h19code.u437",         0x0000, 0x1000, CRC(f4447da0) SHA1(fb4093d5b763be21a9580a0defebed664b1f7a7b))

	// Original font
	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "2716_444-29_h19font.u420",         0x0000, 0x0800, CRC(d595ac1d) SHA1(130fb4ea8754106340c318592eec2d8a0deaf3d0))

	// Original keyboard
	ROM_REGION( 0x0800, "keyboard", 0 )
	ROM_LOAD( "2716_444-37_h19keyb.u445",         0x0000, 0x0800, CRC(5c3e6972) SHA1(df49ce64ae48652346a91648c58178a34fb37d3c))
ROM_END

ROM_START( super19 )
	// Super-19 ROM
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "2732_super19_h447.u437",           0x0000, 0x1000, CRC(6c51aaa6) SHA1(5e368b39fe2f1af44a905dc474663198ab630117))

	// Original font
	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "2716_444-29_h19font.u420",         0x0000, 0x0800, CRC(d595ac1d) SHA1(130fb4ea8754106340c318592eec2d8a0deaf3d0))

	// Original keyboard
	ROM_REGION( 0x0800, "keyboard", 0 )
	ROM_LOAD( "2716_444-37_h19keyb.u445",         0x0000, 0x0800, CRC(5c3e6972) SHA1(df49ce64ae48652346a91648c58178a34fb37d3c))
ROM_END

ROM_START( superset )
	// SuperSet ROM
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "27256_101-402_superset_code.u437", 0x0000, 0x2000, CRC(a6896076) SHA1(a4e2d25028dc75a665c3f5a830a4e8cdecbd481c))
	// 1st 8k is unused, so overwrite it, with the next 16k (but only first 8k is valid)
	ROM_CONTINUE(0x0000, 0x4000)
	// overwrite at 8k starting address with the last 8k of the chip.
	ROM_CONTINUE(0x2000, 0x2000)

	// Superset SUPERFONT
	ROM_REGION( 0x8000, "chargen", 0 )
	ROM_LOAD( "27256_101-431_superset_font.u420", 0x0000, 0x8000, CRC(4c0688f6) SHA1(be6059913420ad66f5c839d619fdcb164ffae85a))

	// Superset Keyboard
	ROM_REGION( 0x0800, "keyboard", 0 )
	ROM_LOAD( "2716_101-422_superset_kbd.u445",   0x0000, 0x0800, CRC(549d15b3) SHA1(981962e5e05bbdc5a66b0e86870853ce5596e877))
ROM_END

ROM_START( watz19 )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("watzman-a")

	// Watzman ROM
	ROM_SYSTEM_BIOS(0, "watzman", "Watzman")
	ROMX_LOAD("watzman.u437",                     0x0000, 0x1000, CRC(8168b6dc) SHA1(bfaebb9d766edbe545d24bc2b6630be4f3aa0ce9), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "watzman-a", "Watzman w/clock persists after reset")
	ROMX_LOAD("watzman-a.u437",                   0x0000, 0x1000, CRC(1f7553e9) SHA1(ac6ddb12b4fb46c1a0ad08ee43978ad3153b51aa), ROM_BIOS(1))

	// Original font
	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "2716_444-29_h19font.u420",         0x0000, 0x0800, CRC(d595ac1d) SHA1(130fb4ea8754106340c318592eec2d8a0deaf3d0))

	// Watzman keyboard
	ROM_REGION( 0x0800, "keyboard", 0 )
	ROM_LOAD( "keybd.u445",                       0x0000, 0x0800, CRC(58dc8217) SHA1(1b23705290bdf9fc6342065c6a528c04bff67b13))
ROM_END

ROM_START( ultra19 )
	// Ultra ROM
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "2532_h19_ultra_firmware.u437",     0x0000, 0x1000, CRC(8ad4cdb4) SHA1(d6e1fc37a1f52abfce5e9adb1819e0030bed1df3))

	// Original font
	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "2716_444-29_h19font.u420",         0x0000, 0x0800, CRC(d595ac1d) SHA1(130fb4ea8754106340c318592eec2d8a0deaf3d0))

	// Ultra keyboard
	ROM_REGION( 0x0800, "keyboard", 0 )
	ROM_LOAD( "2716_h19_ultra_keyboard.u445",     0x0000, 0x0800, CRC(76130c92) SHA1(ca39c602af48505139d2750a084b5f8f0e662ff7))
ROM_END

ROM_START( gp19 )
	// GP-19 ROMs
	ROM_REGION( 0x3000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gp19_rom_1_ver_1_1.u26",           0x0000, 0x1000, CRC(7cbe8e0d) SHA1(977df895b067fa4e4646b4518cffb08d4e6bb6e0))
	ROM_LOAD( "gp19_rom_2_ver_1_1.u25",           0x1000, 0x1000, CRC(a85e3bc4) SHA1(f64e851c5d3ef98a9a48e0fc482baa576773ff43))
	ROM_LOAD( "gp19_rom_3_ver_1_1.u24",           0x2000, 0x1000, CRC(878f577c) SHA1(0756435914dcfb981de4e40d5f81af3e0f5bb1c5))

	// GP-19 font
	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "gp19_char_gen_cg_1.u21",           0x0000, 0x1000, CRC(49ec9242) SHA1(770a8c7b5b15bcfe465fd84326d0ae3dcaa85311))

	// Original keyboard
	ROM_REGION( 0x0800, "keyboard", 0 )
	ROM_LOAD( "2716_444-37_h19keyb.u445",         0x0000, 0x0800, CRC(5c3e6972) SHA1(df49ce64ae48652346a91648c58178a34fb37d3c))
ROM_END

ROM_START( imaginator )
	// Program code
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASEFF )
	// Original terminal code
	ROM_LOAD( "2732_444-46_h19code.u437",                     0x0000, 0x1000, CRC(f4447da0) SHA1(fb4093d5b763be21a9580a0defebed664b1f7a7b))
	// Imaginator ROMs
	ROM_LOAD( "2732_imaginator_gpc_rev_1.2_pn_9000_0002.u9a", 0x2000, 0x1000, CRC(507bb13f) SHA1(5b210f8d77e22fdf063f611eb5c29636cdb01250))

	// Original font
	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "2716_444-29_h19font.u420",                     0x0000, 0x0800, CRC(d595ac1d) SHA1(130fb4ea8754106340c318592eec2d8a0deaf3d0))

	// Original keyboard
	ROM_REGION( 0x0800, "keyboard", 0 )
	ROM_LOAD( "2716_444-37_h19keyb.u445",                     0x0000, 0x0800, CRC(5c3e6972) SHA1(df49ce64ae48652346a91648c58178a34fb37d3c))
ROM_END


ioport_constructor heath_tlb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(tlb);
}

const tiny_rom_entry *heath_tlb_device::device_rom_region() const
{
	return ROM_NAME(h19);
}

void heath_tlb_device::serial_out_b(int data)
{
	m_slot->serial_out_b(data);
}

void heath_tlb_device::dtr_out(int data)
{
	m_slot->dtr_out(data);
}

void heath_tlb_device::rts_out(int data)
{
	m_slot->rts_out(data);
}

void heath_tlb_device::serial_in_w(int state)
{
	m_ace->rx_w(state);
}

void heath_tlb_device::rlsd_in_w(int state)
{
	m_ace->dcd_w(state);
}

void heath_tlb_device::dsr_in_w(int state)
{
	m_ace->dsr_w(state);
}

void heath_tlb_device::cts_in_w(int state)
{
	m_ace->cts_w(state);
}

void heath_tlb_device::device_add_mconfig(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, H19_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &heath_tlb_device::mem_map);
	m_maincpu->set_addrmap(AS_IO, &heath_tlb_device::io_map);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER, rgb_t::green());
	// based on the H19 ROM code for 60 Hz
	m_screen->set_raw(BASE_DOT_CLOCK, 768, 32, 672, 270, 0, 250);
	m_screen->set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	GFXDECODE(config, "gfxdecode", m_palette, gfx_h19);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	// MC 6845 uses a character clock, divide the DOT clock by 8.
	MC6845(config, m_crtc, BASE_DOT_CLOCK / 8);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(heath_tlb_device::crtc_update_row));
	// frame pulse
	m_crtc->out_vsync_callback().set(FUNC(heath_tlb_device::crtc_vsync_w));

	// serial port
	INS8250(config, m_ace, INS8250_CLOCK);
	m_ace->out_int_callback().set(FUNC(heath_tlb_device::serial_irq_w));
	m_ace->out_tx_callback().set(FUNC(heath_tlb_device::serial_out_b));
	m_ace->out_dtr_callback().set(FUNC(heath_tlb_device::dtr_out));
	m_ace->out_rts_callback().set(FUNC(heath_tlb_device::rts_out));

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

	// clock for repeat key
	CLOCK(config, m_repeat_clock, 40);
	m_repeat_clock->set_duty_cycle(0);
	m_repeat_clock->signal_handler().set(m_mm5740, FUNC(mm5740_device::repeat_line_w));
}


/**
 * Super-19 ROM
 *
 * Developed by ATG Systems
 */
heath_super19_tlb_device::heath_super19_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
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


/**
 * Superset ROM
 *
 * Developed by TMSI
 */
heath_superset_tlb_device::heath_superset_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	heath_tlb_device(mconfig, HEATH_SUPERSET, tag, owner, clock),
	m_selected_char_set(0)
{
}

void heath_superset_tlb_device::device_add_mconfig(machine_config &config)
{
	heath_tlb_device::device_add_mconfig(config);

	// part of the Superset upgrade was to upgrade the CPU to 3 MHz.
	m_maincpu->set_clock(H19_3MHZ);
	m_maincpu->set_addrmap(AS_PROGRAM, &heath_superset_tlb_device::mem_map);
	m_maincpu->set_addrmap(AS_IO, &heath_superset_tlb_device::io_map);

	// per line updates are needed for onscreen menu to display properly
	m_screen->set_video_attributes(VIDEO_UPDATE_SCANLINE);

	m_crtc->set_update_row_callback(FUNC(heath_superset_tlb_device::crtc_update_row));

	// link up the serial port outputs to font chip.
	m_ace->out_dtr_callback().set(FUNC(heath_superset_tlb_device::dtr_internal));
	m_ace->out_out1_callback().set(FUNC(heath_superset_tlb_device::out1_internal));
	m_ace->out_out2_callback().set(FUNC(heath_superset_tlb_device::out2_internal));
}

void heath_superset_tlb_device::device_start()
{
	heath_tlb_device::device_start();

	save_item(NAME(m_selected_char_set));

	m_selected_char_set = 0;
}

void heath_superset_tlb_device::mem_map(address_map &map)
{
	heath_tlb_device::mem_map(map);

	// update rom space for the 16k ROM.
	map(0x0000, 0x3fff).rom();

	// page 2 memory
	map(0x8000, 0x87ff).ram();
}

void heath_superset_tlb_device::io_map(address_map &map)
{
	heath_tlb_device::io_map(map);

	map(0x61, 0x61).mirror(0x12).select(0x0c).rw(FUNC(heath_superset_tlb_device::crtc_reg_r), FUNC(heath_superset_tlb_device::crtc_reg_w));
}

const tiny_rom_entry *heath_superset_tlb_device::device_rom_region() const
{
	return ROM_NAME(superset);
}

ioport_constructor heath_superset_tlb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(superset);
}

u8 heath_superset_tlb_device::crtc_reg_r(offs_t reg)
{
	if (!machine().side_effects_disabled())
	{
		m_reverse_video_disabled = bool(BIT(reg, 3));
	}

	return heath_tlb_device::crtc_reg_r(reg);
}

void heath_superset_tlb_device::crtc_reg_w(offs_t reg, u8 val)
{
	m_reverse_video_disabled = bool(BIT(reg, 3));

	heath_tlb_device::crtc_reg_w(reg, val);
}

MC6845_UPDATE_ROW(heath_superset_tlb_device::crtc_update_row)
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u32               *p       = &bitmap.pix(y);

	if (de)
	{
		for (int x = 0; x < x_count; x++)
		{
			u8  inv                  = (x == cursor_x) ? 0xff : 0;
			u8  chr                  = m_p_videoram[(ma + x) & 0x7ff];
			u16 char_set_base_offset = (m_selected_char_set << 11);

			if (chr & 0x80)
			{
				if (m_reverse_video_disabled)
				{
					// set A13
					char_set_base_offset |= 0x2000;
				}
				else
				{
					inv ^= 0xff;
				}

				chr &= 0x7f;
			}

			// get pattern of pixels for the character scanline
			u8 const gfx = m_p_chargen[char_set_base_offset | (chr << 4) | ra] ^ inv;

			// Display a scanline of a character (8 pixels)
			for (int b = 0; 8 > b; ++b)
			{
				*p++ = palette[BIT(gfx, b)];
			}
		}
	}
	else
	{
		std::fill_n(p, bitmap.rowpixels(), palette[0]);
	}
}

// DTR (pin 33) to A14 (pin 27) of 27256 ROM
void heath_superset_tlb_device::dtr_internal(int data)
{
	m_selected_char_set = (m_selected_char_set & 0x03) | ((data & 0x01) << 3);
}

// OUT1 (pin 34) to A12 (pin 2) of 27256 ROM
void heath_superset_tlb_device::out1_internal(int data)
{
	m_selected_char_set = (m_selected_char_set & 0x09) | ((data & 0x01) << 1);
}

// OUT2 (pin 31) to A11? on board
void heath_superset_tlb_device::out2_internal(int data)
{
	m_selected_char_set = (m_selected_char_set & 0x0a) | (data & 0x01);
}


/**
 * Watzman ROM
 *
 * Developed by Barry Watzman, sold by HUG (Heath Users Group)
*/
heath_watz_tlb_device::heath_watz_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
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


/**
 * UltraROM
 *
 * Developed by William G. Parrott, III, sold by Software Wizardry, Inc.
 */
heath_ultra_tlb_device::heath_ultra_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
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


/**
 * Northwest Digital Systems GP-19 add-in board
 */
heath_gp19_tlb_device::heath_gp19_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	heath_tlb_device(mconfig, HEATH_GP19, tag, owner, clock)
{
}

void heath_gp19_tlb_device::device_add_mconfig(machine_config &config)
{
	heath_tlb_device::device_add_mconfig(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &heath_gp19_tlb_device::mem_map);
	m_maincpu->set_addrmap(AS_IO, &heath_gp19_tlb_device::io_map);

	m_crtc->set_update_row_callback(FUNC(heath_gp19_tlb_device::crtc_update_row));
	m_crtc->set_clock(GP19_DOT_CLOCK_2 / 8);

	m_screen->set_raw(GP19_DOT_CLOCK_2, 776, 32, 672, 264, 10, 250);
}

void heath_gp19_tlb_device::device_start()
{
	heath_tlb_device::device_start();

	save_item(NAME(m_char_gen_a11));
	save_item(NAME(m_graphic_mode));
	save_item(NAME(m_col_132));
	save_item(NAME(m_reverse_video));

	m_char_gen_a11  = false;
	m_graphic_mode  = false;
	m_col_132       = false;
	m_reverse_video = false;
}

void heath_gp19_tlb_device::mem_map(address_map &map)
{
	// ROMs 1, 2, 3
	map(0x0000, 0x02fff).rom();

	// Optional external board - Program ROM 4(not aware if any ever existed), external I/O
	// map(0x3000, 0x03fff).rom();

	map(0x4000, 0x40ff).mirror(0x3f00).ram();
	map(0x6000, 0x67ff).mirror(0x1800).ram();
	map(0x8000, 0xbfff).mirror(0x4000).ram().share(m_p_videoram);
}

void heath_gp19_tlb_device::io_map(address_map &map)
{
	heath_tlb_device::io_map(map);

	// Latch U5
	map(0x68, 0x68).mirror(0x07).w(FUNC(heath_gp19_tlb_device::latch_u5_w));

	// Switch on GP-19 board
	map(0x70, 0x70).mirror(0x07).portr("SW1");

	// Optional Auxiliary I/O connector(not aware if any ever existed)
	// map(0x78, 0x78).mirror(0x07);
}

/**
 * U5 Latch
 *
 *  Q0 Graph/character H
 *  Q1 132/80 columns H
 *  Q2 Reverse Screen H
 *  Q3 Alt Character L
 *  Q4-Q7 LED indicators (debug LEDs on circuit board)
 */
void heath_gp19_tlb_device::latch_u5_w(u8 data)
{
	m_graphic_mode  = bool(BIT(data, 0));
	m_col_132       = bool(BIT(data, 1));
	m_reverse_video = bool(BIT(data, 2));
	m_char_gen_a11  = bool(BIT(data, 3));

	// TODO handle LED indicators

	if (m_graphic_mode)
	{
		m_crtc->set_clock(GP19_DOT_CLOCK_3 / 8);
	}
	else if (m_col_132)
	{
		m_crtc->set_clock(GP19_DOT_CLOCK_1 / 8);
	}
	else
	{
		m_crtc->set_clock(GP19_DOT_CLOCK_2 / 8);
	}
}

MC6845_UPDATE_ROW(heath_gp19_tlb_device::crtc_update_row)
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u32               *p       = &bitmap.pix(y);

	if (de)
	{
		u8 screen_inv = m_reverse_video ? 0xff : 0x00;

		if (m_graphic_mode)
		{
			for (int x = 0; x < x_count; x++)
			{
				u8 const gfx = m_p_videoram[((ma << 1) + (ra * x_count) + x) & 0x3fff] ^ screen_inv;

				for (int b = 0; 8 > b; ++b)
				{
					*p++ = palette[BIT(gfx, b)];
				}
			}
		}
		else
		{
			u16 const base = m_char_gen_a11 ? 0x800 : 0x0;

			for (int x = 0; x < x_count; x++)
			{
				u8 inv = (x == cursor_x) ? 0xff : 0;
				u8 chr = m_p_videoram[(ma + x) & 0x3fff];

				if (chr & 0x80)
				{
					inv ^= 0xff;
					chr &= 0x7f;
				}

				inv ^= screen_inv;

				// select proper character set
				u8 const gfx = m_p_chargen[base | (chr << 4) | ra] ^ inv;

				// Display a scanline of a character (8 pixels)
				for (int b = 0; 8 > b; ++b)
				{
					*p++ = palette[BIT(gfx, b)];
				}
			}
		}
	}
	else
	{
		std::fill_n(p, x_count * 8, palette[0]);
	}
}

const tiny_rom_entry *heath_gp19_tlb_device::device_rom_region() const
{
	return ROM_NAME(gp19);
}

ioport_constructor heath_gp19_tlb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(gp19);
}


/**
 * Cleveland Codonics Imaginator-100 (I-100) add-in board
 *
 *    Memory Range     Description
 *   -----------------------------------------------------------------
 *    0x0000-0x1fff    Program ROM (Heath TLB board)
 *    0x2000-0x3fff    Graphics Command Processor(GCP) ROM (I-100)
 *    0x4000-0x5fff    Scratchpad RAM (Heath TLB board)
 *    0x6000-0x7fff    Graphics Scratchpad  R/W RAM (I-100)
 *    0x8000-0xbfff    Graphics Display R/W RAM(I-100)
 *    0xc000-0xdfff    Optional Graphics Command Processor memory (I-100)
 *    0xe000-0xffff    Display Memory (Heath TLB board)
 *
 * New interrupt source
 *   - horizontal retrace interrupt
 *
 */
heath_imaginator_tlb_device::heath_imaginator_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	heath_tlb_device(mconfig, HEATH_IMAGINATOR, tag, owner, clock),
	m_mem_bank(*this,      "membank"),
	m_p_graphic_ram(*this, "graphicram")
{
}

void heath_imaginator_tlb_device::device_add_mconfig(machine_config &config)
{
	heath_tlb_device::device_add_mconfig(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &heath_imaginator_tlb_device::mem_map);
	m_maincpu->set_addrmap(AS_IO, &heath_imaginator_tlb_device::io_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(heath_imaginator_tlb_device::irq_ack_cb));

	m_crtc->out_hsync_callback().set(FUNC(heath_imaginator_tlb_device::crtc_hsync_w));
}

void heath_imaginator_tlb_device::device_start()
{
	heath_tlb_device::device_start();

	save_item(NAME(m_im2_val));
	save_item(NAME(m_alphanumeric_mode_active));
	save_item(NAME(m_graphics_mode_active));
	save_item(NAME(m_allow_tlb_interrupts));
	save_item(NAME(m_allow_imaginator_interrupts));
	save_item(NAME(m_hsync_irq_raised));

	m_mem_bank->configure_entries(0, 2, memregion("maincpu")->base(), 0x2000);

	m_maincpu->space(AS_PROGRAM).install_readwrite_tap(0x8000, 0xbfff, "irq_update",
			[this] (offs_t offset, u8 &data, u8 mem_mask) { if (!machine().side_effects_disabled()) { tap_8000h(); } },
			[this] (offs_t offset, u8 &data, u8 mem_mask) { if (!machine().side_effects_disabled()) { tap_8000h(); } });
}

void heath_imaginator_tlb_device::device_reset()
{
	heath_tlb_device::device_reset();

	m_mem_bank->set_entry(1);
	m_tap_6000h.remove();
	m_tap_6000h = m_maincpu->space(AS_PROGRAM).install_readwrite_tap(0x6000, 0x7fff, "mem_map_update",
			[this] (offs_t offset, u8 &data, u8 mem_mask) { if (!machine().side_effects_disabled()) { tap_6000h(); } },
			[this] (offs_t offset, u8 &data, u8 mem_mask) { if (!machine().side_effects_disabled()) { tap_6000h(); } });

	m_alphanumeric_mode_active = true;
	m_graphics_mode_active     = false;

	m_hsync_irq_raised         = false;

	allow_tlb_intr();
}

void heath_imaginator_tlb_device::mem_map(address_map &map)
{
	map.unmap_value_high();

	map(0x0000, 0x1fff).bankr(m_mem_bank);

	// Normal spot of the GCP ROM
	map(0x2000, 0x3fff).rom();

	// TLB Scratchpad
	map(0x4000, 0x40ff).mirror(0x1f00).ram();

	// GCP Scratchpad
	map(0x6000, 0x607f).mirror(0x1f80).ram();

	// Graphics RAM
	map(0x8000, 0xbfff).ram().share(m_p_graphic_ram);

	// optional Graphics Command Processor (GCP)
	// map(0xc000, 0xdfff);

	// Alphanumeric RAM
	map(0xe000, 0xe7ff).mirror(0x1800).ram().share(m_p_videoram);
}

void heath_imaginator_tlb_device::tap_6000h()
{
	m_mem_bank->set_entry(0);
	m_tap_6000h.remove();
}

void heath_imaginator_tlb_device::tap_8000h()
{
	if (!m_allow_tlb_interrupts)
	{
		allow_tlb_intr();

		set_irq_line();
	}
}

void heath_imaginator_tlb_device::allow_tlb_intr()
{
	m_allow_tlb_interrupts        = true;
	m_allow_imaginator_interrupts = false;
	m_im2_val                     = 0x02;
}

void heath_imaginator_tlb_device::allow_hsync_intr()
{
	m_allow_tlb_interrupts        = false;
	m_allow_imaginator_interrupts = true;
	m_im2_val                     = 0x00;
}

/**
 * An input/output request (IORQ) is sent to the TERMINAL LOGIC board only when
 * both IORQ (pin 20 of U7) AND A4 are low.
 */
void heath_imaginator_tlb_device::io_map(address_map &map)
{
	heath_tlb_device::io_map(map);

	// interrupt routing
	map(0x10, 0x10).select(0x08).mirror(0x07).w(FUNC(heath_imaginator_tlb_device::config_irq_w));

	// display enabling (graphics or alphanumeric)
	map(0x30, 0x30).select(0xc0).mirror(0x0f).w(FUNC(heath_imaginator_tlb_device::display_enable_w));

	// Avoids writes to TLB when a4 is high on unused
	map(0x50, 0x50).select(0x80).mirror(0x0f).w(FUNC(heath_imaginator_tlb_device::nop_w));
}

IRQ_CALLBACK_MEMBER(heath_imaginator_tlb_device::irq_ack_cb)
{
	return m_im2_val;
}

void heath_imaginator_tlb_device::config_irq_w(offs_t reg, u8 val)
{
	if (BIT(reg, 3))
	{
		allow_tlb_intr();
	}
	else
	{
		allow_hsync_intr();
	}

	set_irq_line();
}

void heath_imaginator_tlb_device::display_enable_w(offs_t reg, u8 val)
{
	m_alphanumeric_mode_active = bool(BIT(reg, 7));
	m_graphics_mode_active     = bool(BIT(reg, 6));
}

void heath_imaginator_tlb_device::nop_w(offs_t, u8)
{
}

const tiny_rom_entry *heath_imaginator_tlb_device::device_rom_region() const
{
	return ROM_NAME(imaginator);
}

MC6845_UPDATE_ROW(heath_imaginator_tlb_device::crtc_update_row)
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u32               *p       = &bitmap.pix(y);

	if (de)
	{
		for (int x = 0; x < x_count; x++)
		{
			u8 output = 0x00;

			if (m_alphanumeric_mode_active)
			{
				u8 inv = (x == cursor_x) ? 0xff : 0;
				u8 chr = m_p_videoram[(ma + x) & 0x7ff];

				if (chr & 0x80)
				{
					inv ^= 0xff;
					chr &= 0x7f;
				}

				output |= m_p_chargen[(chr << 4) | ra] ^ inv;
			}

			if (m_graphics_mode_active && (x > 7 ) && (x < 73))
			{
				output |= m_p_graphic_ram[((y * 64) + (x - 8)) & 0x3fff];
			}

			for (int b = 0; 8 > b; ++b)
			{
				*p++ = palette[BIT(output, b)];
			}
		}
	}
	else
	{
		std::fill_n(p, x_count * 8, palette[0]);
	}
}

void heath_imaginator_tlb_device::crtc_hsync_w(int val)
{
	m_hsync_irq_raised = bool(val);

	set_irq_line();
}

void heath_imaginator_tlb_device::set_irq_line()
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0,
		(m_allow_imaginator_interrupts && m_hsync_irq_raised) ||
		(m_allow_tlb_interrupts && (m_keyboard_irq_raised || m_serial_irq_raised || m_break_key_irq_raised)) ?
		 ASSERT_LINE : CLEAR_LINE);
}


/**
 * SigmaSoft Interactive Graphics Controller (IGC)
 *
 *
 */
heath_igc_tlb_device::heath_igc_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	heath_tlb_device(mconfig, HEATH_IGC, tag, owner, clock)
{
}

heath_igc_tlb_device::heath_igc_tlb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	heath_tlb_device(mconfig, type, tag, owner, clock)
{
}

void heath_igc_tlb_device::device_add_mconfig(machine_config &config)
{
	heath_tlb_device::device_add_mconfig(config);

	m_crtc->set_update_row_callback(FUNC(heath_igc_tlb_device::crtc_update_row));
}

void heath_igc_tlb_device::device_start()
{
	heath_tlb_device::device_start();

	m_p_graphic_ram = make_unique_clear<u8[]>(0x3ffff);
	save_pointer(NAME(m_p_graphic_ram), 0x3ffff);

	save_item(NAME(m_data_reg));
	save_item(NAME(m_pixel_video_enabled));
	save_item(NAME(m_character_video_disabled));
	save_item(NAME(m_video_invert_enabled));
	save_item(NAME(m_alternate_character_set_enabled));
	save_item(NAME(m_video_fill_enabled));
	save_item(NAME(m_read_address_increment_disabled));
	save_item(NAME(m_memory_bank_select));
	save_item(NAME(m_io_address));
	save_item(NAME(m_window_address));
}

void heath_igc_tlb_device::device_reset()
{
	heath_tlb_device::device_reset();

	sigma_ctrl_w(0);
	m_data_reg       = 0x00;
	m_io_address     = 0x0000;
	m_window_address = 0x0000;
}

void heath_igc_tlb_device::sigma_ctrl_w(u8 data)
{
	LOGREG("%s: data: %02x\n", FUNCNAME, data);

	m_pixel_video_enabled             = bool(BIT(data, 0));
	m_character_video_disabled        = bool(BIT(data, 1));
	m_video_invert_enabled            = bool(BIT(data, 2));
	m_alternate_character_set_enabled = bool(BIT(data, 3));
	m_video_fill_enabled              = bool(BIT(data, 4));
	m_read_address_increment_disabled = bool(BIT(data, 5));

	m_memory_bank_select              = bitswap<2>(data, 6, 7);

	if (m_video_fill_enabled)
	{
		std::fill_n(&m_p_graphic_ram[(m_memory_bank_select << 16) & 0x3ffff], 0x10000, m_data_reg);
	}
}

u8 heath_igc_tlb_device::sigma_ctrl_r()
{
	u8 ret_val = 0x00;

	ret_val |= m_pixel_video_enabled             ? 0x01 : 0x00;
	ret_val |= m_character_video_disabled        ? 0x02 : 0x00;
	ret_val |= m_video_invert_enabled            ? 0x04 : 0x00;
	ret_val |= m_alternate_character_set_enabled ? 0x08 : 0x00;
	ret_val |= m_video_fill_enabled              ? 0x10 : 0x00;
	ret_val |= m_read_address_increment_disabled ? 0x20 : 0x00;

	ret_val |= bitswap<2>(m_memory_bank_select, 0, 1) << 6;

	LOGREG("%s: ret_val: %02x\n", FUNCNAME, ret_val);

	return ret_val;
}

void heath_igc_tlb_device::sigma_video_mem_w(u8 data)
{
	LOGREG("%s: data: %02x\n", FUNCNAME, data);

	m_data_reg                                                     = data;

	m_p_graphic_ram[(m_memory_bank_select << 16) + m_io_address++] = data;
}

u8 heath_igc_tlb_device::sigma_video_mem_r()
{
	// control whether m_io_address is incremented during a read
	u32 addr = (m_read_address_increment_disabled || machine().side_effects_disabled()) ?
		((m_memory_bank_select << 16) + m_io_address) :
		((m_memory_bank_select << 16) + m_io_address++);

	return m_p_graphic_ram[addr];
}

void heath_igc_tlb_device::sigma_io_lo_addr_w(u8 data)
{
	LOGREG("%s: data: %02x\n", FUNCNAME, data);

	m_io_address = (m_io_address & 0xff00) | data;
}

void heath_igc_tlb_device::sigma_io_hi_addr_w(u8 data)
{
	LOGREG("%s: data: %02x\n", FUNCNAME, data);

	m_io_address = (data << 8) | (m_io_address & 0x00ff);
}

void heath_igc_tlb_device::sigma_window_lo_addr_w(u8 data)
{
	LOGREG("%s: data: %02x\n", FUNCNAME, data);

	m_window_address = (m_window_address & 0xff00) | data;
}

void heath_igc_tlb_device::sigma_window_hi_addr_w(u8 data)
{
	LOGREG("%s: data: %02x\n", FUNCNAME, data);

	m_window_address = (data << 8) | (m_window_address & 0x00ff);
}

MC6845_UPDATE_ROW(heath_igc_tlb_device::crtc_update_row)
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u32               *p       = &bitmap.pix(y);

	if (de)
	{
		u8 video_invert = m_video_invert_enabled ? 0xff : 0;

		for (int x = 0; x < x_count; x++)
		{
			u8 output = 0x00;

			if (!m_character_video_disabled)
			{
				u8 inv = (x == cursor_x) ? 0xff : 0;
				u8 chr = m_p_videoram[(ma + x) & 0x7ff];

				if (chr & 0x80)
				{
					inv ^= 0xff;
					chr &= 0x7f;
				}

				// TODO handle alt font
				output |= m_p_chargen[(chr << 4) | ra] ^ inv;
			}

			if (m_pixel_video_enabled)
			{
				output |= m_p_graphic_ram[(((y * 80) + x) + m_window_address) & 0xffff];
			}

			output ^= video_invert;

			for (int b = 0; 8 > b; ++b)
			{
				*p++ = palette[BIT(output, b)];
			}
		}
	}
	else
	{
		std::fill_n(p, x_count * 8, palette[0]);
	}
}


/**
 * SigmaSoft and Systems IGC plus TLB with Super-19 ROM
 *
 */
heath_igc_super19_tlb_device::heath_igc_super19_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	heath_igc_tlb_device(mconfig, HEATH_IGC_SUPER19, tag, owner, clock)
{
}

const tiny_rom_entry *heath_igc_super19_tlb_device::device_rom_region() const
{
	return ROM_NAME(super19);
}

ioport_constructor heath_igc_super19_tlb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(super19);
}


/**
 * SigmaSoft and Systems IGC plus TLB with UltraROM
 *
 */
heath_igc_ultra_tlb_device::heath_igc_ultra_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	heath_igc_tlb_device(mconfig, HEATH_IGC_ULTRA, tag, owner, clock)
{
}

void heath_igc_ultra_tlb_device::device_add_mconfig(machine_config &config)
{
	heath_tlb_device::device_add_mconfig(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &heath_igc_ultra_tlb_device::mem_map);
}

void heath_igc_ultra_tlb_device::mem_map(address_map &map)
{
	heath_tlb_device::mem_map(map);

	// update rom mirror setting to allow page 2 memory
	map(0x0000, 0x0fff).mirror(0x2000).rom();

	// Page 2 memory
	map(0x1000, 0x1fff).mirror(0x2000).ram();
}

const tiny_rom_entry *heath_igc_ultra_tlb_device::device_rom_region() const
{
	return ROM_NAME(ultra19);
}

ioport_constructor heath_igc_ultra_tlb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ultra19);
}


/**
 * SigmaSoft and Systems IGC plus TLB with Watzman ROM
 *
 */
heath_igc_watz_tlb_device::heath_igc_watz_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	heath_igc_tlb_device(mconfig, HEATH_IGC_WATZ, tag, owner, clock)
{
}

const tiny_rom_entry *heath_igc_watz_tlb_device::device_rom_region() const
{
	return ROM_NAME(watz19);
}

ioport_constructor heath_igc_watz_tlb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(watz19);
}


/**
 * Terminal Logic Board Connector
 *
 */
heath_tlb_connector::heath_tlb_connector(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, HEATH_TLB_CONNECTOR, tag, owner, clock),
	device_single_card_slot_interface(mconfig, *this),
	m_write_sd(*this),
	m_dtr_cb(*this),
	m_rts_cb(*this),
	m_reset(*this),
	m_tlb(nullptr)
{
}

heath_tlb_connector::~heath_tlb_connector()
{
}

void heath_tlb_connector::device_start()
{
	m_tlb = get_card_device();
}
