// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Casio FP-1100

    Info found at various sites:

    Casio FP1000 and FP1100 are "pre-PC" personal computers, with Cassette,
    Floppy Disk, Printer and 2 cart/expansion slots. They had 32K ROM, 64K
    main RAM, 80x25 text display, 320x200, 640x200, 640x400 graphics display.
    Floppy disk is 2x 5 1/4.

    The FP1000 had 16K videoram and monochrome only. The monitor had a switch
    to invert the display (swap foreground and background colours).

    The FP1100 had 48K videoram and 8 colours.

    Processors: Z80 @ 4MHz, uPD7801G @ 2MHz

    Came with Basic built in, and you could run CP/M 2.2 from the floppy disk.

    The keyboard is a separate unit. It contains a beeper.

    TODO:
    - unimplemented instruction PER triggered (can be ignored)
    - Display can be interlaced or non-interlaced. Interlaced not emulated.
    - Cassette Load is quite complex, using 6 pins of the sub-cpu. Not done.
    - subcpu is supposed to be in WAIT except in horizontal blanking period.
      WAIT is not emulated in our cpu.
    - Keyboard not working.
    - FDC not done.


****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/upd7810/upd7810.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "video/mc6845.h"
#include "sound/beep.h"
#include "bus/centronics/ctronics.h"
#include "imagedev/cassette.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#define VERBOSE 0
#include "logmacro.h"

#define MAIN_CLOCK 15.9744_MHz_XTAL

class fp1100_state : public driver_device
{
public:
	fp1100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "sub")
		, m_crtc(*this, "crtc")
		, m_ipl(*this, "ipl")
		, m_wram(*this, "wram")
		, m_videoram(*this, "videoram")
		, m_keyboard(*this, "KEY.%u", 0)
		, m_beep(*this, "beeper")
		, m_centronics(*this, "centronics")
		, m_cass(*this, "cassette")
	{ }

	void fp1100(machine_config &config);

protected:
	virtual void machine_reset() override;

private:
	void main_bank_w(u8 data);
	void irq_mask_w(u8 data);
	void slot_bank_w(u8 data);
	u8 slot_id_r();
	u8 memory_r(offs_t offset);
	void colour_control_w(u8 data);
	void kbd_row_w(u8 data);
	void porta_w(u8 data);
	u8 portb_r();
	u8 portc_r();
	void portc_w(u8 data);
	DECLARE_WRITE_LINE_MEMBER(centronics_busy_w);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	MC6845_UPDATE_ROW(crtc_update_row);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_w);

	required_device<palette_device> m_palette;

	void io_map(address_map &map);
	void main_map(address_map &map);
	void sub_map(address_map &map);
	void handle_int_to_main();

	u8 m_irq_mask = 0;
	u8 m_slot_num = 0;
	u8 m_kbd_row = 0;
	u8 m_col_border = 0;
	u8 m_col_cursor = 0;
	u8 m_col_display = 0;
	u8 m_centronics_busy = 0;
	u8 m_cass_data[4]{};
	bool m_bank_sel = false;
	bool m_main_irq_status = false;
	bool m_sub_irq_status = false;
	bool m_cassbit = false;
	bool m_cassold = false;

	struct {
		u8 id = 0;
	}m_slot[8];

	struct {
		u8 porta = 0;
		u8 portb = 0;
		u8 portc = 0;
	}m_upd7801;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<mc6845_device> m_crtc;
	required_region_ptr<u8> m_ipl;
	required_shared_ptr<u8> m_wram;
	required_shared_ptr<u8> m_videoram;
	required_ioport_array<16> m_keyboard;
	required_device<beep_device> m_beep;
	required_device<centronics_device> m_centronics;
	required_device<cassette_image_device> m_cass;
};

MC6845_UPDATE_ROW( fp1100_state::crtc_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	u32 *p = &bitmap.pix(y);

	if (BIT(m_upd7801.porta, 4))
	{ // green screen
		for (u16 x = 0; x < x_count; x++)
		{
			u16 mem = (((ma+x)<<3) + ra) & 0x3fff;
			u8 g = m_videoram[mem];
			for (u8 i = 0; i < 8; i++)
			{
				u8 col = BIT(g, i);
				if (x == cursor_x) col ^= 1;
				*p++ = palette[col<<1];
			}
		}
	}
	else
	{ // RGB screen
		for (u16 x = 0; x < x_count; x++)
		{
			u16 mem = (((ma+x)<<3) + ra) & 0x3fff;
			u8 b = m_videoram[mem];
			u8 r = m_videoram[mem+0x4000];
			u8 g = m_videoram[mem+0x8000];
			for (u8 i = 0; i < 8; i++)
			{
				u8 col = BIT(r, i) + (BIT(g, i) << 1) + (BIT(b, i) << 2);
				if (x == cursor_x) col = m_col_cursor;
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
	m_bank_sel = BIT(data, 1);
	m_slot_num = (m_slot_num & 3) | ((data & 1) << 2); //??
}

// tell sub that latch has a byte
void fp1100_state::irq_mask_w(u8 data)
{
	m_irq_mask = data;
	handle_int_to_main();

	if (BIT(data, 7) && !m_sub_irq_status)
	{
		m_subcpu->set_input_line(UPD7810_INTF2, ASSERT_LINE);
		LOG("%s: Sub IRQ asserted\n",machine().describe_context());
		m_sub_irq_status = true;
	}
	else
	if (!BIT(data, 7) && m_sub_irq_status)
	{
		m_subcpu->set_input_line(UPD7810_INTF2, CLEAR_LINE);
		LOG("%s: Sub IRQ cleared\n",machine().describe_context());
		m_sub_irq_status = false;
	}

	LOG("%s: IRQmask=%X\n",machine().describe_context(),data);
}

void fp1100_state::slot_bank_w(u8 data)
{
	m_slot_num = (data & 3) | (m_slot_num & 4);
}

u8 fp1100_state::slot_id_r()
{
	//return 0xff;
	return m_slot[m_slot_num & 7].id;
}

u8 fp1100_state::memory_r(offs_t offset)
{
	if (offset < 0x9000 && !m_bank_sel)
		return m_ipl[offset];
	else
		return m_wram[offset];
}

void fp1100_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).r(FUNC(fp1100_state::memory_r));
	map(0x0000, 0xffff).writeonly().share("wram"); // always write to ram
}

void fp1100_state::io_map(address_map &map)
{
	map.unmap_value_high();
	//map(0x0000, 0xfeff) slot memory area
	map(0xff00, 0xff00).mirror(0x7f).rw(FUNC(fp1100_state::slot_id_r), FUNC(fp1100_state::slot_bank_w));
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
	data = bitswap<8>(data, 7, 4, 6, 5, 3, 0, 2, 1);  // change BRG to RGB

	m_col_border = data & 7;

	if (BIT(data, 7))
		m_col_display = (data >> 4) & 7;
	else
		m_col_cursor = data >> 4;
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
	m_beep->set_state(BIT(data, 4));
}

void fp1100_state::sub_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("sub_ipl", 0x0000);
	map(0x2000, 0xdfff).ram().share("videoram"); //vram B/R/G
	map(0xe000, 0xe000).mirror(0x3fe).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0xe001, 0xe001).mirror(0x3fe).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0xe400, 0xe400).mirror(0x3ff).portr("DSW").w(FUNC(fp1100_state::kbd_row_w));
	map(0xe800, 0xe800).mirror(0x3ff).r("main2sub", FUNC(generic_latch_8_device::read));
	map(0xe800, 0xe800).mirror(0x3ff).w("sub2main", FUNC(generic_latch_8_device::write));
	map(0xec00, 0xec00).mirror(0x3ff).lw8(NAME([this] (u8 data) { m_subcpu->set_input_line(UPD7810_INTF0, CLEAR_LINE); }));
	map(0xf000, 0xf000).mirror(0x3ff).w(FUNC(fp1100_state::colour_control_w));
	map(0xf400, 0xff7f).rom().region("sub_ipl", 0x2400);
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
	m_upd7801.porta = data;

	if (BIT(data, 5))
		memset(m_videoram, 0, 0xc000);
}

u8 fp1100_state::portb_r()
{
	u8 data = m_keyboard[m_kbd_row & 15]->read() ^ 0xff;
	LOG("%s: PortB:%X:%X\n",machine().describe_context(),m_kbd_row,data);
	//m_subcpu->set_input_line(UPD7810_INTF0, BIT(data, 7) ? ASSERT_LINE : CLEAR_LINE);
	if (BIT(m_kbd_row, 5))
		return data;
	else
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
	return (m_upd7801.portc & 0x78) | m_centronics_busy;
}

/*
d3 - cause INT on main cpu
d4 - Centronics port is used for input or output
d5 - CMT relay
d6 - Centronics strobe
*/
void fp1100_state::portc_w(u8 data)
{
	u8 bits = data ^ m_upd7801.portc;
	m_upd7801.portc = data;

	if (BIT(bits, 3))
	{
		LOG("%s: PortC:%X\n",machine().describe_context(),data);
		handle_int_to_main();
	}
	if (BIT(bits, 5))
		m_cass->change_state(BIT(data, 5) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
	if (BIT(bits, 6))
		m_centronics->write_strobe(BIT(data, 6));
}

// HOLD_LINE used because the interrupt is set and cleared by successive instructions, too fast for the maincpu to notice
void fp1100_state::handle_int_to_main()
{
	// IRQ is on if bit 4 of mask AND bit 3 portC
	if (BIT(m_upd7801.portc, 3) && BIT(m_irq_mask, 4))
	{
		if (!m_main_irq_status)
		{
			m_maincpu->set_input_line(0, HOLD_LINE);
			LOG("%s: Main IRQ asserted\n",machine().describe_context());
//          m_main_irq_status = true;
		}
	}
	else
	{
		if (m_main_irq_status)
		{
//          m_maincpu->set_input_line(0, CLEAR_LINE);
//          LOG("%s: Main IRQ cleared\n",machine().describe_context());
			m_main_irq_status = false;
		}
	}
}

/* Input ports */
static INPUT_PORTS_START( fp1100 )
	PORT_START("KEY.0")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Break")
	PORT_BIT(0x60, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP) PORT_NAME("Kanji")  // guess, it's in Japanese
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_NAME("Caps")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT) PORT_NAME("Graph")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("Ctrl")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Shift") PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("KEY.2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10) PORT_NAME("PF0")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter") PORT_CHAR(13)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME("*")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_NAME("Z") PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_NAME("Q") PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP-")    PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_NAME("Esc") PORT_CHAR(27)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_NAME("A") PORT_CHAR('A') PORT_CHAR('a')

	PORT_START("KEY.3")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_NAME("PF1")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT) PORT_NAME(",")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME("/")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_NAME("X") PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_NAME("W") PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP+")    PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_NAME("1") PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_NAME("S") PORT_CHAR('S') PORT_CHAR('s')

	PORT_START("KEY.4")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_NAME("PF2")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL) PORT_NAME(".")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("Del")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_NAME("C") PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_NAME("E") PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP3")    PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_NAME("2") PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_NAME("D") PORT_CHAR('D') PORT_CHAR('d')

	PORT_START("KEY.5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_NAME("PF3")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("000")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Right")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_NAME("V") PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_NAME("R") PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP6")    PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_NAME("3") PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_NAME("F") PORT_CHAR('F') PORT_CHAR('f')

	PORT_START("KEY.6")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_NAME("PF4")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Space") PORT_CHAR(32)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Ins")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_NAME("B") PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_NAME("T") PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP9")    PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_NAME("4") PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_NAME("G") PORT_CHAR('G') PORT_CHAR('g')

	PORT_START("KEY.7")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_NAME("PF5")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP0")    PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Down")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_NAME("N") PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_NAME("Y") PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP8")    PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_NAME("5") PORT_CHAR('5') PORT_CHAR('^')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_NAME("H") PORT_CHAR('H') PORT_CHAR('h')

	PORT_START("KEY.8")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) PORT_NAME("PF6")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP2")    PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_NAME("Up")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_NAME("M") PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_NAME("U") PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP5")    PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_NAME("6") PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_NAME("J") PORT_CHAR('J') PORT_CHAR('j')

	PORT_START("KEY.9")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_NAME("PF7")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP1")    PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME) PORT_NAME("Home")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_NAME(",") PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_NAME("I") PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP4")    PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_NAME("7") PORT_CHAR('7') PORT_CHAR('%')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_NAME("K") PORT_CHAR('K') PORT_CHAR('k')

	PORT_START("KEY.10")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) PORT_NAME("PF8")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("]") PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Left")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_NAME(".") PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_NAME("O") PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP7")    PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_NAME("8") PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_NAME("L") PORT_CHAR('L') PORT_CHAR('l')

	PORT_START("KEY.11")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9) PORT_NAME("PF9")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("[") PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Rubout") PORT_CHAR(8)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("/") PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_NAME("P") PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Return") PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_NAME("9") PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_NAME(";") PORT_CHAR(';') PORT_CHAR('+')

	PORT_START("KEY.12")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Stop/Cont")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("-") PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("Yen") PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("(c)")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("|")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("^") PORT_CHAR('^') PORT_CHAR('+')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_NAME("0") PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME(":") PORT_CHAR(':') PORT_CHAR('*')

	PORT_START("KEY.13")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED) // Capslock LED on

	PORT_START("KEY.14")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED) // Kanji LED on

	PORT_START("KEY.15")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED) // LEDs off

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

	PORT_START("SLOTS")
	PORT_CONFNAME( 0x0003, 0x0002, "Slot #0" )
	PORT_CONFSETTING(    0x0000, "(null)" )
	PORT_CONFSETTING(    0x0001, "ROM" )
	PORT_CONFSETTING(    0x0002, "RAM" )
	PORT_CONFSETTING(    0x0003, "FDC" )
	PORT_CONFNAME( 0x000c, 0x0008, "Slot #1" )
	PORT_CONFSETTING(    0x0000, "(null)" )
	PORT_CONFSETTING(    0x0004, "ROM" )
	PORT_CONFSETTING(    0x0008, "RAM" )
	PORT_CONFSETTING(    0x000c, "FDC" )
	PORT_CONFNAME( 0x0030, 0x0020, "Slot #2" )
	PORT_CONFSETTING(    0x0000, "(null)" )
	PORT_CONFSETTING(    0x0010, "ROM" )
	PORT_CONFSETTING(    0x0020, "RAM" )
	PORT_CONFSETTING(    0x0030, "FDC" )
	PORT_CONFNAME( 0x00c0, 0x0080, "Slot #3" )
	PORT_CONFSETTING(    0x0000, "(null)" )
	PORT_CONFSETTING(    0x0040, "ROM" )
	PORT_CONFSETTING(    0x0080, "RAM" )
	PORT_CONFSETTING(    0x00c0, "FDC" )
	PORT_CONFNAME( 0x0300, 0x0200, "Slot #4" )
	PORT_CONFSETTING(    0x0000, "(null)" )
	PORT_CONFSETTING(    0x0100, "ROM" )
	PORT_CONFSETTING(    0x0200, "RAM" )
	PORT_CONFSETTING(    0x0300, "FDC" )
	PORT_CONFNAME( 0x0c00, 0x0800, "Slot #5" )
	PORT_CONFSETTING(    0x0000, "(null)" )
	PORT_CONFSETTING(    0x0400, "ROM" )
	PORT_CONFSETTING(    0x0800, "RAM" )
	PORT_CONFSETTING(    0x0c00, "FDC" )
	PORT_CONFNAME( 0x3000, 0x2000, "Slot #6" )
	PORT_CONFSETTING(    0x0000, "(null)" )
	PORT_CONFSETTING(    0x1000, "ROM" )
	PORT_CONFSETTING(    0x2000, "RAM" )
	PORT_CONFSETTING(    0x3000, "FDC" )
	PORT_CONFNAME( 0xc000, 0x8000, "Slot #7" )
	PORT_CONFSETTING(    0x0000, "(null)" )
	PORT_CONFSETTING(    0x4000, "ROM" )
	PORT_CONFSETTING(    0x8000, "RAM" )
	PORT_CONFSETTING(    0xc000, "FDC" )
INPUT_PORTS_END


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

WRITE_LINE_MEMBER( fp1100_state::centronics_busy_w )
{
	m_centronics_busy = state;
}

TIMER_DEVICE_CALLBACK_MEMBER( fp1100_state::kansas_w )
{
	m_cass_data[3]++;

	if (m_cassbit != m_cassold)
	{
		m_cass_data[3] = 0;
		m_cassold = m_cassbit;
	}

	if (m_cassbit)
		m_cass->output(BIT(m_cass_data[3], 0) ? -1.0 : +1.0); // 2400Hz
	else
		m_cass->output(BIT(m_cass_data[3], 1) ? -1.0 : +1.0); // 1200Hz
}

INTERRUPT_GEN_MEMBER( fp1100_state::vblank_irq )
{
//  if (BIT(m_irq_mask, 4))
//      m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xf8); // Z80
}

void fp1100_state::machine_reset()
{
	m_main_irq_status = false;
	m_sub_irq_status = false;
	int i;
	u8 slot_type;
	const u8 id_type[4] = { 0xff, 0x00, 0x01, 0x04};
	for(i=0;i<8;i++)
	{
		slot_type = (ioport("SLOTS")->read() >> i*2) & 3;
		m_slot[i].id = id_type[slot_type];
	}

	m_beep->set_state(0);

	m_bank_sel = false; // point at rom

	m_irq_mask = 0;
	m_slot_num = 0;
	m_kbd_row = 0;
	m_col_border = 0;
	m_col_cursor = 0;
	m_col_display = 0;
	m_upd7801.porta = 0;
	m_upd7801.portb = 0;
	m_upd7801.portc = 0;
	m_maincpu->set_input_line_vector(0, 0xF0);
}

void fp1100_state::fp1100(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MAIN_CLOCK/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &fp1100_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &fp1100_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(fp1100_state::vblank_irq));

	upd7801_device &sub(UPD7801(config, m_subcpu, MAIN_CLOCK/4));
	sub.set_addrmap(AS_PROGRAM, &fp1100_state::sub_map);
	sub.pa_out_cb().set(FUNC(fp1100_state::porta_w));
	sub.pb_in_cb().set(FUNC(fp1100_state::portb_r));
	sub.pb_out_cb().set("cent_data_out", FUNC(output_latch_device::write));
	sub.pc_in_cb().set(FUNC(fp1100_state::portc_r));
	sub.pc_out_cb().set(FUNC(fp1100_state::portc_w));
	sub.txd_func().set([this] (bool state) { m_cassbit = state; });

	GENERIC_LATCH_8(config, "main2sub");
	GENERIC_LATCH_8(config, "sub2main");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 480);
	screen.set_visarea_full();
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));
	PALETTE(config, m_palette).set_entries(8);
	GFXDECODE(config, "gfxdecode", m_palette, gfx_fp1100);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, "beeper", 950) // guess
			.add_route(ALL_OUTPUTS, "mono", 0.50); // inside the keyboard

	/* CRTC */
	MC6845(config, m_crtc, MAIN_CLOCK/8);   /* unknown variant; hand tuned to get ~60 fps */
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(fp1100_state::crtc_update_row));

	/* Printer */
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(fp1100_state::centronics_busy_w));

	output_latch_device &latch(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(latch);

	/* Cassette */
	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
	TIMER(config, "kansas_w").configure_periodic(FUNC(fp1100_state::kansas_w), attotime::from_hz(4800)); // cass write
}

/* ROM definition */
ROM_START( fp1100 )
	ROM_REGION( 0x9000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "basic.rom", 0x0000, 0x9000, BAD_DUMP CRC(7c7dd17c) SHA1(985757b9c62abd17b0bd77db751d7782f2710ec3))

	ROM_REGION( 0x3000, "sub_ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "sub1.rom", 0x0000, 0x1000, CRC(8feda489) SHA1(917d5b398b9e7b9a6bfa5e2f88c5b99923c3c2a3))
	ROM_LOAD( "sub2.rom", 0x1000, 0x1000, CRC(359f007e) SHA1(0188d5a7b859075cb156ee55318611bd004128d7))
	ROM_LOAD( "sub3.rom", 0x2000, 0xf80, BAD_DUMP CRC(fb2b577a) SHA1(a9ae6b03e06ea2f5db30dfd51ebf5aede01d9672))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY  FULLNAME   FLAGS */
COMP( 1983, fp1100, 0,      0,      fp1100,  fp1100, fp1100_state, empty_init, "Casio", "FP-1100", MACHINE_NOT_WORKING)
