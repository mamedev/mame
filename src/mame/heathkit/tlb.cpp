// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

		Heathkit Terminal Logic Board (TLB)

		The board used in the H19 smart terminal designed and manufactured
		by Heath Company. (and identical Z19 sold by Zenith Data Systems)

		The keyboard consists of a 9x10 matrix connected to a MM5740AAC/N
		mask-programmed keyboard controller. The output of this passes
		through a rom.

		Input can also come from the serial port (a 8250).
		Either device will signal an interrupt to the CPU when a key
		is pressed/data is received.

		TODO:
		- speed up emulation
		- update SW401 baud rate options for Watz ROM
		- update SW401 & SW402 definitions for Super-19 ROM
		- update SW401 & SW402 definitions for ULTRA ROM
		- add option for ULTRA ROMs second page of screen RAM

****************************************************************************/
/***************************************************************************
 Memory Layout
	 The U435 three-to-eight line decoder uses A14 and A15 to generate three memory addresses:

	 1.   Program ROM        0x0000

	 2.   Scratchpad RAM     0x4000

	 3.   Display Memory     0xF800


 Port Layout

	 Only address lines A5, A6, A7 are used by the U442 three-to-eight line decoder

Address   Description
----------------------------------------------------
 0x00   Power-up configuration (primary - SW401)
 0x20   Power-up configuration (secondary - SW402)
 0x40   ACE (communications)
 0x60   CRT controller
 0x80   Keyboard encoder
 0xA0   Keyboard status
 0xC0   Key click enable
 0xE0   Bell enable

****************************************************************************/

#include "tlb.h"

// Standard H19 used a 2.048 MHz clock
#define H19_CLOCK (XTAL(12'288'000) / 6)
#define MC6845_CLOCK (XTAL(12'288'000) /8)
#define INS8250_CLOCK (XTAL(12'288'000) /4)

// Capacitor value in pF
#define H19_KEY_DEBOUNCE_CAPACITOR 5000
#define MM5740_CLOCK (mm5740_device::calc_effective_clock_key_debounce(H19_KEY_DEBOUNCE_CAPACITOR))

// Beep Frequency is 1 KHz
#define H19_BEEP_FRQ (H19_CLOCK / 2048)


TIMER_CALLBACK_MEMBER(h19_state::key_click_off)
{
	m_keyclickactive = false;

	if (!m_keyclickactive && !m_bellactive)
		m_beep->set_state(0);
}

TIMER_CALLBACK_MEMBER(h19_state::bell_off)
{
	m_bellactive = false;

	if (!m_keyclickactive && !m_bellactive)
		m_beep->set_state(0);
}



void h19_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).mirror(0x2000).rom();
	map(0x4000, 0x4100).mirror(0x3e00).ram();
	map(0xc000, 0xc7ff).mirror(0x3800).ram().share("videoram");
}

void h19_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).mirror(0x1f).portr("SW401");
	map(0x20, 0x20).mirror(0x1f).portr("SW402");
	map(0x40, 0x47).mirror(0x18).rw(m_ace, FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));
	map(0x60, 0x60).mirror(0x1E).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0x61, 0x61).mirror(0x1E).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x80, 0x80).mirror(0x1f).r(FUNC(h19_state::kbd_key_r));
	map(0xa0, 0xa0).mirror(0x1f).r(FUNC(h19_state::kbd_flags_r));
	map(0xc0, 0xc0).mirror(0x1f).w(FUNC(h19_state::key_click_w));
	map(0xe0, 0xe0).mirror(0x1f).w(FUNC(h19_state::bell_w));
}


// Keyboard encoder masks
#define KB_ENCODER_KEY_VALUE_MASK 0x7f
#define KB_ENCODER_CONTROL_KEY_MASK 0x80

// Keyboard flag masks
#define KB_STATUS_SHIFT_KEYS_MASK 0x01
#define KB_STATUS_CAPS_LOCK_MASK  0x02
#define KB_STATUS_BREAK_KEY_MASK  0x04
#define KB_STATUS_ONLINE_KEY_MASK 0x08
#define KB_STATUS_REPEAT_KEYS_MASK 0x40
#define KB_STATUS_KEYBOARD_STROBE_MASK 0x80


void h19_state::machine_start()
{
	save_item(NAME(m_transchar));
	save_item(NAME(m_strobe));
	save_item(NAME(m_keyclickactive));
	save_item(NAME(m_bellactive));

	m_key_click_timer = timer_alloc(FUNC(h19_state::key_click_off), this);
	m_bell_timer = timer_alloc(FUNC(h19_state::bell_off), this);
}


void h19_state::key_click_w(uint8_t data)
{
/* Keyclick - 6 mSec */

	m_beep->set_state(1);
	m_keyclickactive = true;
	m_key_click_timer->adjust(attotime::from_msec(6));
}

void h19_state::bell_w(uint8_t data)
{
/* Bell (^G) - 200 mSec */

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
uint16_t h19_state::translate_mm5740_b(uint16_t b)
{
	return ((b & 0x100) >> 2) | ((b & 0x0c0) << 1) | (b & 0x03f);
}

uint8_t h19_state::kbd_key_r()
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	m_strobe = false;

	// high bit is for control key pressed, this is handled in the ROM, no processing needed.
	return m_transchar;
}

uint8_t h19_state::kbd_flags_r()
{
	uint16_t modifiers = m_kbspecial->read();
	uint8_t rv = modifiers & 0x7f;

	// check both shifts
	if ((modifiers & 0x020) == 0 || (modifiers & 0x100) == 0)
	{
		rv |= KB_STATUS_SHIFT_KEYS_MASK;
	}

	// invert offline switch
	rv ^= KB_STATUS_ONLINE_KEY_MASK;

	if (!m_strobe)
	{
		rv |= KB_STATUS_KEYBOARD_STROBE_MASK;
	}

	return rv;
}

READ_LINE_MEMBER(h19_state::mm5740_shift_r)
{
	return ((m_kbspecial->read() ^ 0x120) & 0x120) ? ASSERT_LINE : CLEAR_LINE;
}

READ_LINE_MEMBER(h19_state::mm5740_control_r)
{
	return ((m_kbspecial->read() ^ 0x10) & 0x10) ? ASSERT_LINE: CLEAR_LINE;
}

WRITE_LINE_MEMBER(h19_state::mm5740_data_ready_w)
{
	if (state == ASSERT_LINE)
	{
		uint8_t *decode = m_kbdrom->base();

		m_transchar = decode[translate_mm5740_b(m_mm5740->b_r())];
		m_strobe = true;
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
	}
}

MC6845_UPDATE_ROW( h19_state::crtc_update_row )
{
	if (!de)
		return;

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

		/* get pattern of pixels for that character scanline */
		uint8_t const gfx = m_p_chargen[(chr<<4) | ra] ^ inv;

		/* Display a scanline of a character (8 pixels) */
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


/* F4 Character Displayer */
static const gfx_layout h19_charlayout =
{
	8, 10,                  /* 8 x 10 characters */
	128,                    /* 128 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_h19 )
	GFXDECODE_ENTRY( "chargen", 0x0000, h19_charlayout, 0, 1 )
GFXDECODE_END

void h19_state::h19(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, H19_CLOCK); // From schematics
	m_maincpu->set_addrmap(AS_PROGRAM, &h19_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &h19_state::io_map);

	/* video hardware */
	// TODO: make configurable, Heath offered 3 different CRTs - White, Green, Amber.
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_refresh_hz(60);   // TODO- this is adjustable by dipswitch.
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));
	screen.set_size(640, 250);
	screen.set_visarea(0, 640 - 1, 0, 250 - 1);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_h19);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	MC6845(config, m_crtc, MC6845_CLOCK);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(true);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(h19_state::crtc_update_row));
	m_crtc->out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_NMI); // frame pulse

	ins8250_device &uart(INS8250(config, "ins8250", INS8250_CLOCK));
	uart.out_int_callback().set_inputline("maincpu", INPUT_LINE_IRQ0);

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
	m_mm5740->shift_cb().set(FUNC(h19_state::mm5740_shift_r));
	m_mm5740->control_cb().set(FUNC(h19_state::mm5740_control_r));
	m_mm5740->data_ready_cb().set(FUNC(h19_state::mm5740_data_ready_w));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, H19_BEEP_FRQ).add_route(ALL_OUTPUTS, "mono", 1.00);
}
