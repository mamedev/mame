// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Casio FP-1100

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

    TODO:
    - Memory maps and machine configuration for FP-1000 with reduced VRAM.
    - Unimplemented instruction PER triggered (can be ignored)
    - Display can be interlaced or non-interlaced.  Interlaced not emulated.
    - Cassette Load is quite complex, using 6 pins of the sub-CPU.  Not
      done.
    - Sub CPU is supposed to be in WAIT except in horizontal blanking
      period.  WAIT is not emulated in our cpu.
    - Keyboard not working.
    - FDC not done.


****************************************************************************/

#include "emu.h"

#include "bus/centronics/ctronics.h"
#include "cpu/upd7810/upd7810.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/beep.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

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
		, m_ipl(*this, "ipl")
		, m_wram(*this, "wram")
		, m_videoram(*this, "videoram.%u", 0)
		, m_palette(*this, "palette")
		, m_keyboard(*this, "KEY.%u", 0)
		, m_beep(*this, "beeper")
		, m_centronics(*this, "centronics")
		, m_cass(*this, "cassette")
	{ }

	void fp1100(machine_config &config);

protected:
	virtual void machine_reset() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<upd7801_device> m_subcpu;
	required_device<mc6845_device> m_crtc;
	required_region_ptr<u8> m_ipl;
	required_shared_ptr<u8> m_wram;
	required_shared_ptr_array<u8, 3> m_videoram;
	required_device<palette_device> m_palette;
	required_ioport_array<16> m_keyboard;
	required_device<beep_device> m_beep;
	required_device<centronics_device> m_centronics;
	required_device<cassette_image_device> m_cass;

	void main_map(address_map &map);
	void io_map(address_map &map);
	void sub_map(address_map &map);

	void main_bank_w(u8 data);
	void irq_mask_w(u8 data);
	void slot_bank_w(u8 data);
	u8 slot_id_r();
	u8 memory_r(offs_t offset);
	void colour_control_w(u8 data);
	template <unsigned N> u8 vram_r(offs_t offset);
	template <unsigned N> void vram_w(offs_t offset, u8 data);
	void kbd_row_w(u8 data);
	void porta_w(u8 data);
	u8 portb_r();
	u8 portc_r();
	void portc_w(u8 data);
	void centronics_busy_w(int state);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	MC6845_UPDATE_ROW(crtc_update_row);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_w);

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
};

MC6845_UPDATE_ROW( fp1100_state::crtc_update_row )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u32 *p = &bitmap.pix(y);
	const u8 porta = m_upd7801.porta;

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
	else if (!BIT(data, 7) && m_sub_irq_status)
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

// TODO: convert to `memory_view`
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
	map(0x0000, 0xffff).writeonly().share(m_wram); // always write to ram
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
	if (BIT(m_upd7801.porta, 5) && !BIT(data, 5))
	{
		for (int i = 0; i < 3; i++)
		{
			const u8 fill_value = BIT(m_col_display, i) ? 0xff : 0;
			std::fill(m_videoram[i].begin(), m_videoram[i].end(), fill_value);
		}
	}

	const u8 crtc_divider = BIT(data, 3) ? 16 : 8;
	m_crtc->set_unscaled_clock(MAIN_CLOCK / crtc_divider);

	m_upd7801.porta = data;
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
	u8 const bits = data ^ m_upd7801.portc;
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


static INPUT_PORTS_START( fp1100 )
	PORT_START("KEY.0")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)                                  PORT_NAME("BREAK")
	PORT_BIT(0x60, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)                                 PORT_NAME("\u30ab\u30ca (KANA)")  // カナ
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)                             PORT_NAME("CAPS")   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RALT)                                 PORT_NAME("GRAPH")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("CTRL")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)   PORT_CODE(KEYCODE_RSHIFT)   PORT_NAME("SHIFT")  PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("KEY.2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)        PORT_NAME("PF0")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)  PORT_NAME("ENTER")                  PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)   PORT_NAME("KP*")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_NAME(u8"Z  \u30c4  \u30c3")    PORT_CHAR('Z') PORT_CHAR('z')  // ツ ッ
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_NAME(u8"Q  \u30bf")            PORT_CHAR('Q') PORT_CHAR('q')  // タ
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)  PORT_NAME("KP-")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)        PORT_NAME("ESC")                    PORT_CHAR(27)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_NAME(u8"A  \u30c1")            PORT_CHAR('A') PORT_CHAR('a')  // チ

	PORT_START("KEY.3")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)         PORT_NAME("PF1")                    PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA_PAD)  PORT_NAME("KP,")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)  PORT_NAME("KP/")                    PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_NAME(u8"X  \u30b5")            PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_NAME(u8"W  \u30c6")            PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)   PORT_NAME("KP+")                    PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_NAME(u8"1  !  \u30cc")         PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_NAME(u8"S  \u30c8")            PORT_CHAR('S') PORT_CHAR('s')

	PORT_START("KEY.4")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)         PORT_NAME("PF2")                    PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)    PORT_NAME("KP.")                    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)        PORT_NAME("DEL")                    PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_NAME(u8"C  \u30bd")            PORT_CHAR('C') PORT_CHAR('c')  // ソ
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_NAME(u8"E  \u30a4  \u30a3")    PORT_CHAR('E') PORT_CHAR('e')  // イ ィ
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)      PORT_NAME("KP3")                    PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_NAME(u8"2  \"  \u30d5")        PORT_CHAR('2') PORT_CHAR('"')  // フ
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_NAME(u8"D  \u30b7")            PORT_CHAR('D') PORT_CHAR('d')  // シ

	PORT_START("KEY.5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)         PORT_NAME("PF3")                    PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_000_PAD)    PORT_NAME("KP000")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)      PORT_NAME("Right")                  PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_NAME(u8"V  \u30d2")            PORT_CHAR('V') PORT_CHAR('v')  // ヒ
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_NAME(u8"R  \u30b9")            PORT_CHAR('R') PORT_CHAR('r')  // ス
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)      PORT_NAME("KP6")                    PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_NAME(u8"3  #  \u30a2  \u30a1") PORT_CHAR('3') PORT_CHAR('#')  // ア ァ
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_NAME(u8"F  \u30cf")            PORT_CHAR('F') PORT_CHAR('f')  // ハ

	PORT_START("KEY.6")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)         PORT_NAME("PF4")                    PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_NAME("Space")                  PORT_CHAR(' ')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN)       PORT_NAME("INS")                    PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_NAME(u8"B  \u30b3")            PORT_CHAR('B') PORT_CHAR('b')  // コ
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_NAME(u8"T  \u30ab")            PORT_CHAR('T') PORT_CHAR('t')  // カ
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)      PORT_NAME("KP9")                    PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_NAME(u8"4  $  \u30a6  \u30a5") PORT_CHAR('4') PORT_CHAR('$')  // ウ ゥ
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_NAME(u8"G  \u30ad")            PORT_CHAR('G') PORT_CHAR('g')  // キ

	PORT_START("KEY.7")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)         PORT_NAME("PF5")                    PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)      PORT_NAME("KP0")                    PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)       PORT_NAME("Down")                   PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_NAME(u8"N  \u30df")            PORT_CHAR('N') PORT_CHAR('n')  // ミ
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_NAME(u8"Y  \u30f3")            PORT_CHAR('Y') PORT_CHAR('y')  // ン
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)      PORT_NAME("KP8")                    PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_NAME(u8"5  %  \u30a8  \u30a7") PORT_CHAR('5') PORT_CHAR('%')  // エ ェ
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_NAME(u8"H  \u30af")            PORT_CHAR('H') PORT_CHAR('h')  // ク

	PORT_START("KEY.8")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)         PORT_NAME("PF6")                    PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)      PORT_NAME("KP2")                    PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_NAME("Up")                     PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_NAME(u8"M  \u30e2")            PORT_CHAR('M') PORT_CHAR('m')  // モ
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_NAME(u8"U  \u30ca")            PORT_CHAR('U') PORT_CHAR('u')  // ナ
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)      PORT_NAME("KP5")                    PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_NAME(u8"6  &  \u30aa  \u30a9") PORT_CHAR('6') PORT_CHAR('&')  // オ ォ
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_NAME(u8"J  \u30de")            PORT_CHAR('J') PORT_CHAR('j')  // マ

	PORT_START("KEY.9")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)         PORT_NAME("PF7")                    PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)      PORT_NAME("KP1")                    PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP)       PORT_NAME("HOME  CLS")              PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_NAME(u8",  <  \u30cd  \u3001") PORT_CHAR(',') PORT_CHAR('<')  // ネ 、
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_NAME(u8"I  \u30cb")            PORT_CHAR('I') PORT_CHAR('i')  // ニ
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)      PORT_NAME("KP4")                    PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_NAME(u8"7  '  \u30e4  \u30e3") PORT_CHAR('7') PORT_CHAR('\'') // ヤ ャ
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_NAME(u8"K  \u30ce")            PORT_CHAR('K') PORT_CHAR('k')  // ノ

	PORT_START("KEY.10")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)         PORT_NAME("PF8")                    PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_NAME(u8"]  }  \u30e0  \u300d") PORT_CHAR(']') PORT_CHAR('}')  // ム 」
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)       PORT_NAME("Left")                   PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_NAME(u8".  >  \u30eb  \u3002") PORT_CHAR('.') PORT_CHAR('>')  // ル 。
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_NAME(u8"O  \u30e9")            PORT_CHAR('O') PORT_CHAR('o')  // ラ
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)      PORT_NAME("KP7")                    PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_NAME(u8"8  (  \u30e6  \u30e5") PORT_CHAR('8') PORT_CHAR('(')  // ユ ュ
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_NAME(u8"L  \u30ea")            PORT_CHAR('L') PORT_CHAR('l')  // リ

	PORT_START("KEY.11")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)         PORT_NAME("PF9")                    PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME(u8"[  {  \u309c  \u300c") PORT_CHAR('[') PORT_CHAR('{')  // ゜ 「
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_NAME("BS")                     PORT_CHAR(8)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_NAME(u8"/  ?  \u30e1  \u30fb") PORT_CHAR('/') PORT_CHAR('?')  // メ ・
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_NAME(u8"P  \u30bb")            PORT_CHAR('P') PORT_CHAR('p')  // セ
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_NAME("RETURN")                 PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_NAME(u8"9  )  \u30e8  \u30e7") PORT_CHAR('9') PORT_CHAR(')')  // ヨ ョ
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_NAME(u8";  +  \u30ec")         PORT_CHAR(';') PORT_CHAR('+')  // レ

	PORT_START("KEY.12")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_NUMLOCK)    PORT_NAME("STOP/CONT")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_NAME(u8"-  =  \u30db")         PORT_CHAR('-') PORT_CHAR('=')  // ホ
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_NAME(u8"¥  |  \u30fc")         PORT_CHAR(U'¥') PORT_CHAR('|') // ー
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_NAME(u8"@  `  \u309b")         PORT_CHAR('@') PORT_CHAR('`')  // ゛
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_NAME(u8"_  \u30ed")            PORT_CHAR('_')                 // ロ
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_NAME(u8"^  ~  \u30d8")         PORT_CHAR('^') PORT_CHAR('~')  // ヘ
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_NAME(u8"0  \u30ef  \u30f2")    PORT_CHAR('0')                 // ワ ヲ
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_NAME(u8":  *  \u30b1")         PORT_CHAR(':') PORT_CHAR('*')  // ケ

	PORT_START("KEY.13")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED) // Capslock LED on

	PORT_START("KEY.14")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED) // Kana LED on

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

void fp1100_state::centronics_busy_w(int state)
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
	Z80(config, m_maincpu, MAIN_CLOCK/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &fp1100_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &fp1100_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(fp1100_state::vblank_irq));

	UPD7801(config, m_subcpu, MAIN_CLOCK/4);
	m_subcpu->set_addrmap(AS_PROGRAM, &fp1100_state::sub_map);
	m_subcpu->pa_out_cb().set(FUNC(fp1100_state::porta_w));
	m_subcpu->pb_in_cb().set(FUNC(fp1100_state::portb_r));
	m_subcpu->pb_out_cb().set("cent_data_out", FUNC(output_latch_device::write));
	m_subcpu->pc_in_cb().set(FUNC(fp1100_state::portc_r));
	m_subcpu->pc_out_cb().set(FUNC(fp1100_state::portc_w));
	m_subcpu->txd_func().set([this] (bool state) { m_cassbit = state; });

	GENERIC_LATCH_8(config, "main2sub");
	GENERIC_LATCH_8(config, "sub2main");

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(fp1100_state::centronics_busy_w));

	output_latch_device &latch(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(latch);

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
	TIMER(config, "kansas_w").configure_periodic(FUNC(fp1100_state::kansas_w), attotime::from_hz(4800));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
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

	SPEAKER(config, "mono").front_center();
	BEEP(config, "beeper", 950) // guess
		.add_route(ALL_OUTPUTS, "mono", 0.50); // inside the keyboard
}

// ROM definitions

ROM_START( fp1100 )
	ROM_REGION( 0x9000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "basic.rom", 0x0000, 0x9000, BAD_DUMP CRC(7c7dd17c) SHA1(985757b9c62abd17b0bd77db751d7782f2710ec3))

	ROM_REGION( 0x3000, "sub_ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "sub1.rom", 0x0000, 0x1000, CRC(8feda489) SHA1(917d5b398b9e7b9a6bfa5e2f88c5b99923c3c2a3))
	ROM_LOAD( "sub2.rom", 0x1000, 0x1000, CRC(359f007e) SHA1(0188d5a7b859075cb156ee55318611bd004128d7))
	ROM_LOAD( "sub3.rom", 0x2000, 0xf80, BAD_DUMP CRC(fb2b577a) SHA1(a9ae6b03e06ea2f5db30dfd51ebf5aede01d9672))
ROM_END

/* FP-1000 has video RAM locations RAM9 to RAM24 unpopulated (only RAM1 to RAM8 are populated) - needs its own machine configuration.
   PCB parts overlay silkscreen for sub-CPU shows "µPD7801G-101", but all examples seen have chips silksreened "D7108G 118". */
ROM_START( fp1000 )
	ROM_REGION( 0x9000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "2l_a10_kkk_fp1000_basic.c1", 0x0000, 0x1000, CRC(9322dedd) SHA1(40a00684ced2b7ead53ca15a915d98f3fe00d3ba))

	ROM_REGION( 0x3000, "sub_ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "jka_fp1000.e8",    0x0000, 0x1000, CRC(2aefa4e4) SHA1(b3cc5484426c19a7266d17ea5c4d55441b4e3be8))
	ROM_LOAD( "jkc_fp1000.e21",   0x1000, 0x1000, CRC(67a668a9) SHA1(37fb9308505b47db36f8c341144ca3fe3fec64af))
	ROM_LOAD( "upd7801g_118.bin", 0x2000, 0xf80, BAD_DUMP CRC(fb2b577a) SHA1(a9ae6b03e06ea2f5db30dfd51ebf5aede01d9672)) // Not dumped, borrowed from 'fp1100'
ROM_END

} // anonymous namespace


// Drivers

COMP( 1983, fp1100, 0,      0, fp1100, fp1100, fp1100_state, empty_init, "Casio", "FP-1100", MACHINE_NOT_WORKING)
COMP( 1982, fp1000, 0, fp1100, fp1100, fp1100, fp1100_state, empty_init, "Casio", "FP-1000", MACHINE_NOT_WORKING)
