// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Casio FP-1000 / FP-1100 (GX-205)

TODO:
- Keyboard not working, should trigger INTF0 on key pressed (for PF only), main CPU receives
  garbage from sub (command protocol 0x04 -> <scancode> -> 0x00 -> 0x00)
  As a _ugly_ workaround you can intercept ASCII scancodes from main CPU with bp d18 A=<value> with
  any emu::keypost trigger.
- Memory maps and machine configuration for FP-1000 with reduced VRAM;
- Unimplemented instruction PER triggered in sub CPU;
- SCREEN 1 mode has heavy corrupted GFXs and runs at half speed, interlace mode?
- Cassette Load is really not working, uses a complex 6 pin discrete circuitry;
- Sub CPU needs proper WAIT line from uPD7801;
- Main CPU waitstates;
- centronics options (likely):
  - FP-1011PL (plotter)
  - FP-1012PR (OEM Epson MX-80)
  - FP-1014PRK (Kanji printer)
  - FP-1017PR (OEM Epson MX-160)

===================================================================================================

Info found at various sites:

Casio FP-1000 and FP-1100 are "pre-PC" personal computers, with
Cassette, Floppy Disk, Printer and two cartridge/expansion slots.  They
had 32K ROM, 64K main RAM, 80x25 text display, 320x200, 640x200, 640x400
graphics display.  Floppy disk is 2x 5 1/4.

The FP-1000 had 16K videoram and monochrome only.  The monitor had a
switch to invert the display (swap foreground and background colours).

The FP-1100 had 48K videoram and 8 colours.

Processors: Z80 @ 4MHz, uPD7801G @ 2MHz

Came with BASIC built in, and you could run CP/M 2.2 from the floppy
disk.

The keyboard is a separate unit.  It contains a beeper.

**************************************************************************************************/

#include "emu.h"

#include "bus/centronics/ctronics.h"
#include "bus/fp1000/fp1000_exp.h"
#include "cpu/upd7810/upd7810.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "machine/gen_latch.h"
#include "machine/input_merger.h"
#include "machine/timer.h"
#include "sound/beep.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "softlist_dev.h"

#define VERBOSE 0
#include "logmacro.h"


namespace {

#define MAIN_CLOCK 15.9744_MHz_XTAL

class fp1100_state : public driver_device
{
public:
	fp1100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "sub")
		, m_crtc(*this, "crtc")
		, m_iplview(*this, "iplview")
		, m_workram(*this, "workram")
		, m_videoram(*this, "videoram.%u", 0)
		, m_palette(*this, "palette")
		, m_keyboard(*this, "KEY.%u", 0)
		, m_beep(*this, "beeper")
		, m_centronics(*this, "centronics")
		, m_cassette(*this, "cassette")
		, m_slot(*this, "slot.%u", 0)
		, m_irqs_int(*this, { "irqs_inta", "irqs_intb", "irqs_intc", "irqs_intd"})
	{ }

	void fp1100(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER(fkey_hit_cb);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<upd7801_device> m_subcpu;
	required_device<mc6845_device> m_crtc;
	memory_view m_iplview;
	required_shared_ptr<u8> m_workram;
	required_shared_ptr_array<u8, 3> m_videoram;
	required_device<palette_device> m_palette;
	required_ioport_array<16> m_keyboard;
	required_device<beep_device> m_beep;
	required_device<centronics_device> m_centronics;
	required_device<cassette_image_device> m_cassette;
	required_device_array<fp1000_exp_slot_device, 2> m_slot;
	required_device_array<input_merger_device, 4> m_irqs_int;

	void main_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;

	void main_bank_w(u8 data);
	void irq_mask_w(u8 data);
	void colour_control_w(u8 data);
	template <unsigned N> u8 vram_r(offs_t offset);
	template <unsigned N> void vram_w(offs_t offset, u8 data);
	void kbd_row_w(u8 data);
	void porta_w(u8 data);
	u8 portb_r();
	u8 portc_r();
	void portc_w(u8 data);
	void centronics_busy_w(int state);

	MC6845_UPDATE_ROW(crtc_update_row);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_w);

	u8 m_kbd_row = 0;
	u8 m_col_border = 0;
	u8 m_col_cursor = 0;
	u8 m_col_display = 0;
	u8 m_centronics_busy = 0;
	u8 m_cassette_data[4]{};
	bool m_sub_irq_status = false;
	bool m_cassettebit = false;
	bool m_cassetteold = false;

	u8 m_sub_porta = 0;
	u8 m_sub_portc = 0;

	u8 m_pending_interrupts = 0;
	u8 m_active_interrupts = 0;
	u8 m_interrupt_mask = 0;
	int m_caps_led_state = 0;
	int m_shift_led_state = 0;

	template<int Line> void int_w(int state);
	TIMER_CALLBACK_MEMBER(update_interrupts);
	IRQ_CALLBACK_MEMBER(restart_cb);

	int m_hsync_state = 0;
	bool m_sub_wait = false;
	void hsync_cb(int state);
};

MC6845_UPDATE_ROW( fp1100_state::crtc_update_row )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u32 *p = &bitmap.pix(y);
	const u8 porta = m_sub_porta;

	if (BIT(porta, 4))
	{ // green screen
		for (u16 x = 0; x < x_count; x++)
		{
			u16 const mem = (((ma + x) << 3) + ra) & 0x3fff;
			// TODO: ORs contents from the other two layers (for FP-1100 only)
			u8 const g = m_videoram[0][mem];
			for (u8 i = 0; i < 8; i++)
			{
				u8 col = BIT(g, i);
				if (x == cursor_x)
					col ^= 1;
				*p++ = palette[col << 1];
			}
		}
	}
	else
	{ // RGB screen
		for (u16 x = 0; x < x_count; x++)
		{
			u16 const mem = (((ma + x) << 3) + ra) & 0x3fff;
			u8 const b = BIT(porta, 2) ? m_videoram[0][mem] : 0;
			u8 const r = BIT(porta, 1) ? m_videoram[1][mem] : 0;
			u8 const g = BIT(porta, 0) ? m_videoram[2][mem] : 0;
			for (u8 i = 0; i < 8; i++)
			{
				u8 col = BIT(r, i) | (BIT(g, i) << 1) | (BIT(b, i) << 2);
				if (x == cursor_x)
					col = m_col_cursor;
				*p++ = palette[col];
			}
		}
	}
}

/*
d0 - Package select
d1 - Bank select (at boot time)
other bits not used
*/
void fp1100_state::main_bank_w(u8 data)
{
	m_iplview.select(BIT(data, 1));
	const u8 slot_select = BIT(data, 0);
	m_slot[slot_select ^ 1]->select_w(false);
	m_slot[slot_select]->select_w(true);
}

/*
 * x--- ---- mask for main to sub (INTF2)
 * ---x ---- INTS (sub to main)
 * ---- x--- INTD
 * ---- -x-- INTC (RS-232C)
 * ---- --x- INTB (FDC bus slot irq)
 * ---- ---x INTA (FDC bus slot drq)
 */
void fp1100_state::irq_mask_w(u8 data)
{
	m_interrupt_mask = data;
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(fp1100_state::update_interrupts), this));

	// TODO: this handling doesn't seem enough
	if (BIT(data, 7) && !m_sub_irq_status)
	{
		m_subcpu->set_input_line(UPD7810_INTF2, ASSERT_LINE);
		LOG("%s: Sub IRQ asserted\n",machine().describe_context());
		m_sub_irq_status = true;
	}
	else if (!BIT(data, 7) && m_sub_irq_status)
	{
		m_subcpu->set_input_line(UPD7810_INTF2, CLEAR_LINE);
		LOG("%s: Sub IRQ cleared\n",machine().describe_context());
		m_sub_irq_status = false;
	}

	LOG("%s: IRQmask=%X\n",machine().describe_context(),data);
}

// NOTE: BASIC is very picky if it doesn't find a configuration akin to this
// Disregard the "NOT USABLE" for page 0 from service manual (definitely wants offset + 0x9000 there)
void fp1100_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).view(m_iplview);
	m_iplview[0](0x0000, 0x7fff).rom().region("ipl", 0);
	m_iplview[0](0x8000, 0x8fff).rom().region("basic", 0);
	m_iplview[0](0x9000, 0xffff).lr8(NAME([this] (offs_t offset) { return m_workram[offset + 0x9000]; }));
	// writes always goes to work RAM
	m_iplview[0](0x0000, 0xffff).writeonly().share(m_workram);
	m_iplview[1](0x0000, 0xffff).ram().share(m_workram);
}

void fp1100_state::io_map(address_map &map)
{
	map.unmap_value_high();
	//map(0x0000, 0xfeff) slot memory area
//  map(0xff00, 0xff00).mirror(0x7f).rw(FUNC(fp1100_state::slot_id_r), FUNC(fp1100_state::slot_bank_w));
	map(0xff80, 0xff80).mirror(0x7f).r("sub2main", FUNC(generic_latch_8_device::read));
	map(0xff80, 0xff80).mirror(0x1f).w(FUNC(fp1100_state::irq_mask_w));
	map(0xffa0, 0xffa0).mirror(0x1f).w(FUNC(fp1100_state::main_bank_w));
	map(0xffc0, 0xffc0).mirror(0x3f).w("main2sub", FUNC(generic_latch_8_device::write));
}

/*
d0,1,2 - border colour (B,R,G)
d3     - not used
d4,5,6 - colour of cursor; or display area (B,R,G) (see d7)
d7     - 1=display area; 0=cursor
*/
void fp1100_state::colour_control_w(u8 data)
{
	// HACK: change BRG to RGB
	data = bitswap<8>(data, 7, 4, 6, 5, 3, 0, 2, 1);
	m_col_border = data & 7;

	if (BIT(data, 7))
		m_col_display = (data >> 4) & 7;
	else
	{
		m_col_display = 0;
		m_col_cursor = data >> 4;
	}
}

template <unsigned N> u8 fp1100_state::vram_r(offs_t offset)
{
	return m_videoram[N][offset];
}

template <unsigned N> void fp1100_state::vram_w(offs_t offset, u8 data)
{
	// NOTE: POST makes sure to punt in FP-1000 mode if this don't XOR the value
	m_videoram[N][offset] = ~data;

	if (!machine().side_effects_disabled())
	{
		if (!m_sub_wait && m_hsync_state)
		{
			// TODO: actual uPD7801 WAIT line
			m_subcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			m_sub_wait = true;
		}
	}
}

/*
d0,1,2,3 - keyboard scan row
         - if 13, turn on shift-lock LED
         - if 14, turn on caps-lock LED
         - if 15, turn off both LEDs
d4       - Beeper
d5       - "3state buffer of key data line (1=open, 0=closed)"
d6,7     - not used
*/
void fp1100_state::kbd_row_w(u8 data)
{
	m_kbd_row = data;
	// guess: expect keyboard in open state for leds
	if (BIT(m_kbd_row, 5))
	{
		switch(m_kbd_row & 0xf)
		{
			case 13: m_shift_led_state = 1; break;
			case 14: m_caps_led_state = 1; break;
			case 15: m_caps_led_state = m_shift_led_state = 0; break;
		}
		// TODO: to output_finders
		//popmessage("%d %d", m_caps_led_state, m_shift_led_state);
	}
	m_beep->set_state(BIT(data, 4));
}

void fp1100_state::sub_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("sub_ipl", 0x0000);
	map(0x2000, 0x5fff).rw(FUNC(fp1100_state::vram_r<0>), FUNC(fp1100_state::vram_w<0>)).share(m_videoram[0]);
	map(0x6000, 0x9fff).rw(FUNC(fp1100_state::vram_r<1>), FUNC(fp1100_state::vram_w<1>)).share(m_videoram[1]);
	map(0xa000, 0xdfff).rw(FUNC(fp1100_state::vram_r<2>), FUNC(fp1100_state::vram_w<2>)).share(m_videoram[2]);
	map(0xe000, 0xe000).mirror(0x3fe).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0xe001, 0xe001).mirror(0x3fe).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0xe400, 0xe400).mirror(0x3ff).portr("DSW").w(FUNC(fp1100_state::kbd_row_w));
	map(0xe800, 0xe800).mirror(0x3ff).r("main2sub", FUNC(generic_latch_8_device::read));
	map(0xe800, 0xe800).mirror(0x3ff).w("sub2main", FUNC(generic_latch_8_device::write));
	map(0xec00, 0xec00).mirror(0x3ff).lw8(NAME([this] (u8 data) { m_subcpu->set_input_line(UPD7810_INTF0, CLEAR_LINE); }));
	map(0xf000, 0xf000).mirror(0x3ff).w(FUNC(fp1100_state::colour_control_w));
	map(0xf400, 0xff7f).rom().region("sub_ipl", 0x2400);
//  map(0xff80, 0xffff) internal 7801 RAM
}

/*
d0,1,2 - enable RGB guns (G,R,B)
d3     - CRTC clock (80 or 40 cols)
d4     - RGB (0) or Green (1)
d5     - clear videoram
d6     - CMT baud rate (1=300; 0=1200)
d7     - CMT load clock
The SO pin is Serial Output to CMT (1=2400Hz; 0=1200Hz)
*/
void fp1100_state::porta_w(u8 data)
{
	if (BIT(m_sub_porta, 5) && !BIT(data, 5))
	{
		for (int i = 0; i < 3; i++)
		{
			const u8 fill_value = BIT(m_col_display, i) ? 0xff : 0;
			std::fill(m_videoram[i].begin(), m_videoram[i].end(), fill_value);
		}
	}

	const u8 crtc_divider = BIT(data, 3) ? 16 : 8;
	m_crtc->set_unscaled_clock(MAIN_CLOCK / crtc_divider);

	m_sub_porta = data;
}

u8 fp1100_state::portb_r()
{
	u8 data = m_keyboard[m_kbd_row & 15]->read();
	LOG("%s: PortB:%X:%X\n",machine().describe_context(),m_kbd_row,data);

	if (BIT(m_kbd_row, 5))
		return data;

	return 0;
}

/*
d0 - Centronics busy
d1 - Centronics error
d2 - CMT load input clock
d7 - CMT load serial data
*/
u8 fp1100_state::portc_r()
{
	return (m_sub_portc & 0x78) | m_centronics_busy;
}

/*
d3 - cause INT on main cpu
d4 - Centronics port is used for input or output
d5 - CMT relay
d6 - Centronics strobe
*/
void fp1100_state::portc_w(u8 data)
{
	u8 const bits = data ^ m_sub_portc;
	const int main_int_state = BIT(data, 3);
	if (BIT(m_sub_portc, 3) != main_int_state)
		int_w<4>(main_int_state);
	m_sub_portc = data;

	if (BIT(bits, 5))
		m_cassette->change_state(BIT(data, 5) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
	if (BIT(bits, 6))
		m_centronics->write_strobe(BIT(data, 6));
}

// IRQ section (main)
// TODO: merge with skeleton/cdc721.cpp (reuses SN74LS148N)

template <int Line> void fp1100_state::int_w(int state)
{
	if (BIT(m_pending_interrupts, Line) == state)
		return;

	if (state)
		m_pending_interrupts |= 0x01 << Line;
	else
		m_pending_interrupts &= ~(0x01 << Line);

	machine().scheduler().synchronize(timer_expired_delegate(FUNC(fp1100_state::update_interrupts), this));
}

TIMER_CALLBACK_MEMBER(fp1100_state::update_interrupts)
{
	m_active_interrupts = m_pending_interrupts & m_interrupt_mask;
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, m_active_interrupts != 0 ? ASSERT_LINE : CLEAR_LINE);
}

IRQ_CALLBACK_MEMBER(fp1100_state::restart_cb)
{
	u8 vector = 0xf0;
	// INTS > INTA > INTB > INTC > INTD priority order
	// INTS uses 0xf0, INTA 0xf2 and so on.
	u8 active = bitswap<5>(m_active_interrupts, 3, 2, 1, 0, 4);
	while (vector < 0xfa && !BIT(active, 0))
	{
		active >>= 1;
		vector += 0x02;
	}
	return vector;
}

// IRQ section (sub)

void fp1100_state::hsync_cb(int state)
{
	m_hsync_state = state;
	if (m_sub_wait && !state)
	{
		m_subcpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_sub_wait = false;
	}
}

// KI8 specifically connects to INT0 on hit
// TODO: stub, hangs by banging keys too fast
INPUT_CHANGED_MEMBER(fp1100_state::fkey_hit_cb)
{
	if (!oldval && newval)
	{
		m_subcpu->set_input_line(UPD7810_INTF0, ASSERT_LINE);
	}
}


static INPUT_PORTS_START( fp1100 )
	PORT_START("KEY.0")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEY.1")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)                                  PORT_NAME("BREAK") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(fp1100_state::fkey_hit_cb), 1)
	PORT_BIT(0x60, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)                                 PORT_NAME("\u30ab\u30ca (KANA)")  // カナ
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)                             PORT_NAME("CAPS")   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RALT)                                 PORT_NAME("GRAPH")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("CTRL")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)   PORT_CODE(KEYCODE_RSHIFT)   PORT_NAME("SHIFT")  PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("KEY.2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)        PORT_NAME("PF0") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(fp1100_state::fkey_hit_cb), 2)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)  PORT_NAME("ENTER")                  PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)   PORT_NAME("KP*")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_NAME(u8"Z  \u30c4  \u30c3")    PORT_CHAR('Z') PORT_CHAR('z')  // ツ ッ
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_NAME(u8"Q  \u30bf")            PORT_CHAR('Q') PORT_CHAR('q')  // タ
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)  PORT_NAME("KP-")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)        PORT_NAME("ESC")                    PORT_CHAR(27)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_NAME(u8"A  \u30c1")            PORT_CHAR('A') PORT_CHAR('a')  // チ

	PORT_START("KEY.3")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)         PORT_NAME("PF1")                    PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(fp1100_state::fkey_hit_cb), 3)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA_PAD)  PORT_NAME("KP,")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)  PORT_NAME("KP/")                    PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_NAME(u8"X  \u30b5")            PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_NAME(u8"W  \u30c6")            PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)   PORT_NAME("KP+")                    PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_NAME(u8"1  !  \u30cc")         PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_NAME(u8"S  \u30c8")            PORT_CHAR('S') PORT_CHAR('s')

	PORT_START("KEY.4")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)         PORT_NAME("PF2")                    PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(fp1100_state::fkey_hit_cb), 4)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)    PORT_NAME("KP.")                    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)        PORT_NAME("DEL")                    PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_NAME(u8"C  \u30bd")            PORT_CHAR('C') PORT_CHAR('c')  // ソ
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_NAME(u8"E  \u30a4  \u30a3")    PORT_CHAR('E') PORT_CHAR('e')  // イ ィ
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)      PORT_NAME("KP3")                    PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_NAME(u8"2  \"  \u30d5")        PORT_CHAR('2') PORT_CHAR('"')  // フ
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_NAME(u8"D  \u30b7")            PORT_CHAR('D') PORT_CHAR('d')  // シ

	PORT_START("KEY.5")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)         PORT_NAME("PF3")                    PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(fp1100_state::fkey_hit_cb), 5)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_000_PAD)    PORT_NAME("KP000")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)      PORT_NAME("Right")                  PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_NAME(u8"V  \u30d2")            PORT_CHAR('V') PORT_CHAR('v')  // ヒ
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_NAME(u8"R  \u30b9")            PORT_CHAR('R') PORT_CHAR('r')  // ス
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)      PORT_NAME("KP6")                    PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_NAME(u8"3  #  \u30a2  \u30a1") PORT_CHAR('3') PORT_CHAR('#')  // ア ァ
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_NAME(u8"F  \u30cf")            PORT_CHAR('F') PORT_CHAR('f')  // ハ

	PORT_START("KEY.6")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)         PORT_NAME("PF4")                    PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(fp1100_state::fkey_hit_cb), 6)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_NAME("Space")                  PORT_CHAR(' ')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN)       PORT_NAME("INS")                    PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_NAME(u8"B  \u30b3")            PORT_CHAR('B') PORT_CHAR('b')  // コ
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_NAME(u8"T  \u30ab")            PORT_CHAR('T') PORT_CHAR('t')  // カ
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)      PORT_NAME("KP9")                    PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_NAME(u8"4  $  \u30a6  \u30a5") PORT_CHAR('4') PORT_CHAR('$')  // ウ ゥ
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_NAME(u8"G  \u30ad")            PORT_CHAR('G') PORT_CHAR('g')  // キ

	PORT_START("KEY.7")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)         PORT_NAME("PF5")                    PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(fp1100_state::fkey_hit_cb), 7)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)      PORT_NAME("KP0")                    PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)       PORT_NAME("Down")                   PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_NAME(u8"N  \u30df")            PORT_CHAR('N') PORT_CHAR('n')  // ミ
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_NAME(u8"Y  \u30f3")            PORT_CHAR('Y') PORT_CHAR('y')  // ン
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)      PORT_NAME("KP8")                    PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_NAME(u8"5  %  \u30a8  \u30a7") PORT_CHAR('5') PORT_CHAR('%')  // エ ェ
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_NAME(u8"H  \u30af")            PORT_CHAR('H') PORT_CHAR('h')  // ク

	PORT_START("KEY.8")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)         PORT_NAME("PF6")                    PORT_CHAR(UCHAR_MAMEKEY(F6)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(fp1100_state::fkey_hit_cb), 8)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)      PORT_NAME("KP2")                    PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_NAME("Up")                     PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_NAME(u8"M  \u30e2")            PORT_CHAR('M') PORT_CHAR('m')  // モ
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_NAME(u8"U  \u30ca")            PORT_CHAR('U') PORT_CHAR('u')  // ナ
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)      PORT_NAME("KP5")                    PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_NAME(u8"6  &  \u30aa  \u30a9") PORT_CHAR('6') PORT_CHAR('&')  // オ ォ
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_NAME(u8"J  \u30de")            PORT_CHAR('J') PORT_CHAR('j')  // マ

	PORT_START("KEY.9")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)         PORT_NAME("PF7")                    PORT_CHAR(UCHAR_MAMEKEY(F7)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(fp1100_state::fkey_hit_cb), 9)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)      PORT_NAME("KP1")                    PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP)       PORT_NAME("HOME  CLS")              PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_NAME(u8",  <  \u30cd  \u3001") PORT_CHAR(',') PORT_CHAR('<')  // ネ 、
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_NAME(u8"I  \u30cb")            PORT_CHAR('I') PORT_CHAR('i')  // ニ
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)      PORT_NAME("KP4")                    PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_NAME(u8"7  '  \u30e4  \u30e3") PORT_CHAR('7') PORT_CHAR('\'') // ヤ ャ
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_NAME(u8"K  \u30ce")            PORT_CHAR('K') PORT_CHAR('k')  // ノ

	PORT_START("KEY.10")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)         PORT_NAME("PF8")                    PORT_CHAR(UCHAR_MAMEKEY(F8)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(fp1100_state::fkey_hit_cb), 10)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_NAME(u8"]  }  \u30e0  \u300d") PORT_CHAR(']') PORT_CHAR('}')  // ム 」
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)       PORT_NAME("Left")                   PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_NAME(u8".  >  \u30eb  \u3002") PORT_CHAR('.') PORT_CHAR('>')  // ル 。
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_NAME(u8"O  \u30e9")            PORT_CHAR('O') PORT_CHAR('o')  // ラ
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)      PORT_NAME("KP7")                    PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_NAME(u8"8  (  \u30e6  \u30e5") PORT_CHAR('8') PORT_CHAR('(')  // ユ ュ
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_NAME(u8"L  \u30ea")            PORT_CHAR('L') PORT_CHAR('l')  // リ

	PORT_START("KEY.11")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)         PORT_NAME("PF9")                    PORT_CHAR(UCHAR_MAMEKEY(F9)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(fp1100_state::fkey_hit_cb), 11)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME(u8"[  {  \u309c  \u300c") PORT_CHAR('[') PORT_CHAR('{')  // ゜ 「
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_NAME("BS")                     PORT_CHAR(8)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_NAME(u8"/  ?  \u30e1  \u30fb") PORT_CHAR('/') PORT_CHAR('?')  // メ ・
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_NAME(u8"P  \u30bb")            PORT_CHAR('P') PORT_CHAR('p')  // セ
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_NAME("RETURN")                 PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_NAME(u8"9  )  \u30e8  \u30e7") PORT_CHAR('9') PORT_CHAR(')')  // ヨ ョ
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_NAME(u8";  +  \u30ec")         PORT_CHAR(';') PORT_CHAR('+')  // レ

	PORT_START("KEY.12")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_NUMLOCK)    PORT_NAME("STOP/CONT") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(fp1100_state::fkey_hit_cb), 12)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_NAME(u8"-  =  \u30db")         PORT_CHAR('-') PORT_CHAR('=')  // ホ
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_NAME(u8"¥  |  \u30fc")         PORT_CHAR(U'¥') PORT_CHAR('|') // ー
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_NAME(u8"@  `  \u309b")         PORT_CHAR('@') PORT_CHAR('`')  // ゛
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_NAME(u8"_  \u30ed")            PORT_CHAR('_')                 // ロ
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_NAME(u8"^  ~  \u30d8")         PORT_CHAR('^') PORT_CHAR('~')  // ヘ
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_NAME(u8"0  \u30ef  \u30f2")    PORT_CHAR('0')                 // ワ ヲ
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_NAME(u8":  *  \u30b1")         PORT_CHAR(':') PORT_CHAR('*')  // ケ

	PORT_START("KEY.13")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED) // Capslock LED on

	PORT_START("KEY.14")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED) // Kana LED on

	PORT_START("KEY.15")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED) // LEDs off

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Text width" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "40 chars/line" )
	PORT_DIPSETTING(    0x01, "80 chars/line" )
	PORT_DIPNAME( 0x02, 0x02, "Screen Mode" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "Screen 0" )
	PORT_DIPSETTING(    0x02, "Screen 1" )
	PORT_DIPNAME( 0x04, 0x04, "FP Mode" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "FP-1000" ) // Green screen
	PORT_DIPSETTING(    0x04, "FP-1100" ) // RGB
	PORT_DIPNAME( 0x08, 0x08, "CMT Baud Rate" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "1200 Baud" )
	PORT_DIPSETTING(    0x08, "300 Baud" )
	PORT_DIPNAME( 0x10, 0x10, "Printer Type" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "<undefined>" )
	PORT_DIPSETTING(    0x10, "FP-1012PR" )
	PORT_DIPNAME( 0x20, 0x20, "Keyboard Type" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "<undefined>" )
	PORT_DIPSETTING(    0x20, "Normal" )
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


// debugging only
static const gfx_layout chars_8x8 =
{
	8,8,
	256,
	1,
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_fp1100 )
	GFXDECODE_ENTRY( "sub_ipl", 0x2400, chars_8x8, 0, 1 )
GFXDECODE_END

void fp1100_state::centronics_busy_w(int state)
{
	m_centronics_busy = state;
}

TIMER_DEVICE_CALLBACK_MEMBER( fp1100_state::kansas_w )
{
	m_cassette_data[3]++;

	if (m_cassettebit != m_cassetteold)
	{
		m_cassette_data[3] = 0;
		m_cassetteold = m_cassettebit;
	}

	if (m_cassettebit)
		m_cassette->output(BIT(m_cassette_data[3], 0) ? -1.0 : +1.0); // 2400Hz
	else
		m_cassette->output(BIT(m_cassette_data[3], 1) ? -1.0 : +1.0); // 1200Hz
}

void fp1100_state::machine_start()
{
	save_item(NAME(m_sub_wait));
	save_item(NAME(m_sub_irq_status));
	save_item(NAME(m_kbd_row));
	save_item(NAME(m_col_border));
	save_item(NAME(m_col_cursor));
	save_item(NAME(m_col_display));
	save_item(NAME(m_centronics_busy));

	save_item(NAME(m_sub_porta));
	save_item(NAME(m_sub_portc));

	save_item(NAME(m_pending_interrupts));
	save_item(NAME(m_active_interrupts));
	save_item(NAME(m_interrupt_mask));

	save_item(NAME(m_caps_led_state));
	save_item(NAME(m_shift_led_state));

	// FIXME: cassette state intentionally not saved for now
}

void fp1100_state::machine_reset()
{
	m_sub_wait = false;
	m_sub_irq_status = false;

	m_beep->set_state(0);

	m_interrupt_mask = m_active_interrupts = m_pending_interrupts = 0;
	m_kbd_row = 0;
	m_caps_led_state = m_shift_led_state = 0;
	m_col_border = 0;
	m_col_cursor = 0;
	m_col_display = 0;
	m_sub_porta = 0;
	m_sub_portc = 0;

	m_iplview.select(0);
}

void fp1100_state::fp1100(machine_config &config)
{
	Z80(config, m_maincpu, MAIN_CLOCK/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &fp1100_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &fp1100_state::io_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(fp1100_state::restart_cb));

	UPD7801(config, m_subcpu, MAIN_CLOCK/4);
	m_subcpu->set_addrmap(AS_PROGRAM, &fp1100_state::sub_map);
	m_subcpu->pa_out_cb().set(FUNC(fp1100_state::porta_w));
	m_subcpu->pb_in_cb().set(FUNC(fp1100_state::portb_r));
	m_subcpu->pb_out_cb().set("cent_data_out", FUNC(output_latch_device::write));
	m_subcpu->pc_in_cb().set(FUNC(fp1100_state::portc_r));
	m_subcpu->pc_out_cb().set(FUNC(fp1100_state::portc_w));
	m_subcpu->txd_func().set([this] (bool state) { m_cassettebit = state; });

	GENERIC_LATCH_8(config, "main2sub");
	GENERIC_LATCH_8(config, "sub2main");
	// NOTE: Needs some sync otherwise it outright refuses to boot
	config.set_perfect_quantum("maincpu");
//  config.set_perfect_quantum("sub");

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(fp1100_state::centronics_busy_w));

	output_latch_device &latch(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(latch);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("fp1100_cass");

	TIMER(config, "kansas_w").configure_periodic(FUNC(fp1100_state::kansas_w), attotime::from_hz(4800));

	SOFTWARE_LIST(config, "cass_list").set_original("fp1100_cass");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	// doesn't matter, will be reset by 6845 anyway
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(640, 480);
	screen.set_visarea_full();
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));
	PALETTE(config, m_palette).set_entries(8);
	GFXDECODE(config, "gfxdecode", m_palette, gfx_fp1100);

	HD6845S(config, m_crtc, MAIN_CLOCK/8);   // hand tuned to get ~60 fps
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(fp1100_state::crtc_update_row));
	m_crtc->out_hsync_callback().set(FUNC(fp1100_state::hsync_cb));

	SPEAKER(config, "mono").front_center();
	BEEP(config, "beeper", 950) // guess
		.add_route(ALL_OUTPUTS, "mono", 0.50); // inside the keyboard

	INPUT_MERGER_ANY_HIGH(config, m_irqs_int[0]).output_handler().set(FUNC(fp1100_state::int_w<0>));
	INPUT_MERGER_ANY_HIGH(config, m_irqs_int[1]).output_handler().set(FUNC(fp1100_state::int_w<1>));
	INPUT_MERGER_ANY_HIGH(config, m_irqs_int[2]).output_handler().set(FUNC(fp1100_state::int_w<2>));
	INPUT_MERGER_ANY_HIGH(config, m_irqs_int[3]).output_handler().set(FUNC(fp1100_state::int_w<3>));

	FP1000_EXP_SLOT(config, m_slot[0], fp1000_exp_devices, nullptr);
	m_slot[0]->set_iospace(m_maincpu, AS_IO);
	m_slot[0]->inta_callback().set(m_irqs_int[0], FUNC(input_merger_device::in_w<0>));
	m_slot[0]->intb_callback().set(m_irqs_int[1], FUNC(input_merger_device::in_w<0>));
	m_slot[0]->intc_callback().set(m_irqs_int[2], FUNC(input_merger_device::in_w<0>));
	m_slot[0]->intd_callback().set(m_irqs_int[3], FUNC(input_merger_device::in_w<0>));

	FP1000_EXP_SLOT(config, m_slot[1], fp1000_exp_devices, nullptr);
	m_slot[1]->set_iospace(m_maincpu, AS_IO);
	m_slot[1]->inta_callback().set(m_irqs_int[0], FUNC(input_merger_device::in_w<1>));
	m_slot[1]->intb_callback().set(m_irqs_int[1], FUNC(input_merger_device::in_w<1>));
	m_slot[1]->intc_callback().set(m_irqs_int[2], FUNC(input_merger_device::in_w<1>));
	m_slot[1]->intd_callback().set(m_irqs_int[3], FUNC(input_merger_device::in_w<1>));
}

// TODO: chargen, keyboard ROM and key tops can be substituted on actual FP-1000/FP-1100
// HN462732G-JKA to position E8 ("common for all languages")
// HN462532G-G?? to position E21 (chargen)
// - GKA Spain
// - GLA Italy
// - GMA France
// - GNA Germany
// - GPA UK
// - GQA Netherlands
// - GRA Norway/Denmark
// - GSA Sweden/Finland
// - GTA <default chargen, Japan?>

ROM_START( fp1100 )
	ROM_REGION( 0x9000, "bios", ROMREGION_ERASEFF )
	// TODO: split into two roms
	ROM_LOAD( "basic.rom", 0x0000, 0x9000, BAD_DUMP CRC(7c7dd17c) SHA1(985757b9c62abd17b0bd77db751d7782f2710ec3))

	ROM_REGION( 0x8000, "ipl", ROMREGION_ERASEFF )
	ROM_COPY( "bios", 0x0000, 0x0000, 0x8000 )

	ROM_REGION( 0x1000, "basic", ROMREGION_ERASEFF )
	ROM_COPY( "bios", 0x8000, 0x0000, 0x1000 )

	ROM_REGION( 0x3000, "sub_ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "sub1.rom", 0x0000, 0x1000, CRC(8feda489) SHA1(917d5b398b9e7b9a6bfa5e2f88c5b99923c3c2a3))
	ROM_LOAD( "sub2.rom", 0x1000, 0x1000, CRC(359f007e) SHA1(0188d5a7b859075cb156ee55318611bd004128d7))
	// Japan chargen ROM (GTA?)
	ROM_LOAD( "sub3.rom", 0x2000, 0xf80, BAD_DUMP CRC(fb2b577a) SHA1(a9ae6b03e06ea2f5db30dfd51ebf5aede01d9672))
ROM_END

/* FP-1000 has video RAM locations RAM9 to RAM24 unpopulated (only RAM1 to RAM8 are populated) - needs its own machine configuration.
   PCB parts overlay silkscreen for sub-CPU shows "µPD7801G-101", but all examples seen have chips silksreened "D7108G 118". */
ROM_START( fp1000 )
	ROM_REGION( 0x8000, "ipl", ROMREGION_ERASE00 )
	ROM_LOAD( "ipl.rom", 0x0000, 0x8000, NO_DUMP)

	ROM_REGION( 0x1000, "basic", ROMREGION_ERASEFF )
	ROM_LOAD( "2l_a10_kkk_fp1000_basic.c1", 0x0000, 0x1000, CRC(9322dedd) SHA1(40a00684ced2b7ead53ca15a915d98f3fe00d3ba))

	ROM_REGION( 0x3000, "sub_ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "sub1.rom", 0x0000, 0x1000, BAD_DUMP CRC(8feda489) SHA1(917d5b398b9e7b9a6bfa5e2f88c5b99923c3c2a3)) // Not dumped, borrowed from 'fp1100'
	ROM_LOAD( "jka_fp1000.e8",    0x1000, 0x1000, CRC(2aefa4e4) SHA1(b3cc5484426c19a7266d17ea5c4d55441b4e3be8))
	// Spain chargen ROM (GKA really?)
	ROM_LOAD( "jkc_fp1000.e21",   0x2000, 0x1000, CRC(67a668a9) SHA1(37fb9308505b47db36f8c341144ca3fe3fec64af))
ROM_END

} // anonymous namespace


COMP( 1983, fp1100, 0,      0, fp1100, fp1100, fp1100_state, empty_init, "Casio", "FP-1100", MACHINE_NOT_WORKING)
COMP( 1982, fp1000, 0, fp1100, fp1100, fp1100, fp1100_state, empty_init, "Casio", "FP-1000", MACHINE_NOT_WORKING)
