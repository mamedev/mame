// license: BSD-3-Clause
// copyright-holders: Dirk Best
/****************************************************************************

    Brother PN-8800FXB "Super PowerNote"

    Hardware:
    - HD64180RF6X CPU
    - TC551001BFL-70 (128k RAM)
    - 2x HY6264A (2x 8k, VRAM?)
    - HG62F33R32FH UC2836-A (gate array)
    - HD63266F FDC
    - RC224ATF (modem)
    - TC8521AM RTC
    - XTAL XT1 16.000312 MHz (near modem), XT2 12.228MHz (near CPU)
    - XTAL XT3 32.768kHz (near RTC), XT4 18.0MHz (near gate array)
    - XTAL XT5 16.0MHz (near FDC)

    TODO:
    - Fix ROM banking (ROM self-test shows NG)
    - Improve video emulation (offset, modes, etc.)
    - Floppy
    - Buzzer
    - Centronics
    - Modem
    - Soft power off/on
    - RAM contents should be retained when turning off
    - Bookman (unlikely to make progress, extra PCB contains custom CPU)

    Notes:
    - There is a serially connected daughterbord containing the Bookman logic
    - Press CODE+SHIFT+BS on the main menu for a self-test screen
    - Currently the system always does a cold boot. This means that the
      century byte at $6180b will be invalid, causing the system to program
      the RTC with default values.

****************************************************************************/

#include "emu.h"

#include "cpu/z180/z180.h"
#include "machine/rp5c01.h"
#include "machine/timer.h"

#include "emupal.h"
#include "screen.h"

namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class pn8800fxb_state : public driver_device
{
public:
	pn8800fxb_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_rombank(*this, "rombank"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_keys(*this, "KO%u", 0U),
		m_lomem_view(*this, "lomem")
	{ }

	void pn8800fxb(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<z80180_device> m_maincpu;
	required_memory_bank m_rombank;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_ioport_array<9> m_keys;
	memory_view m_lomem_view;

	uint8_t m_key_select;

	std::unique_ptr<uint8_t[]> m_vram;
	uint16_t m_cursor_addr;
	uint16_t m_video_addr;
	uint8_t m_video_ctrl;
	uint8_t m_video_offset;
	uint8_t m_cursor_ctrl;
	uint8_t m_mem_change;

	void mem_map(address_map &map);
	void io_map(address_map &map);

	uint8_t keyboard_r();
	void keyboard_w(uint8_t data);

	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void cursor_addr_lo_w(uint8_t data);
	void cursor_addr_hi_w(uint8_t data);
	void video_addr_lo_w(uint8_t data);
	void video_addr_hi_w(uint8_t data);
	uint8_t video_data_r();
	void video_data_w(uint8_t data);
	uint8_t video_ctrl_r();
	void video_ctrl_w(uint8_t data);
	void video_offset_w(uint8_t data);
	void cursor_ctrl_w(uint8_t data);

	void mem_change_w(uint8_t data);
	void bank_select_w(uint8_t data);
	void int_ack_w(uint8_t data);
	TIMER_DEVICE_CALLBACK_MEMBER(int1_timer);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void pn8800fxb_state::mem_map(address_map &map)
{
	map(0x00000, 0x01fff).rom();
	map(0x02000, 0x05fff).ram().share("window1");
	map(0x06000, 0x07fff).view(m_lomem_view);
	m_lomem_view[0](0x06000, 0x07fff).ram().share("window2");
	m_lomem_view[1](0x06000, 0x07fff).rom().region("maincpu", 0x06000);
	map(0x08000, 0x3ffff).rom();
	map(0x40000, 0x41fff).unmaprw();
	map(0x42000, 0x45fff).rom().region("maincpu", 0x02000);
	map(0x46000, 0x4ffff).unmaprw();
	map(0x50000, 0x5ffff).bankr(m_rombank);
	map(0x60000, 0x61fff).ram();
	map(0x62000, 0x65fff).ram().share("window1");
	map(0x66000, 0x67fff).ram().share("window2");
	map(0x68000, 0x7ffff).ram();
}

void pn8800fxb_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x3f).noprw(); // z180 internal
	map(0x70, 0x70).w(FUNC(pn8800fxb_state::cursor_addr_lo_w));
	map(0x71, 0x71).w(FUNC(pn8800fxb_state::cursor_addr_hi_w));
	map(0x72, 0x72).w(FUNC(pn8800fxb_state::video_addr_lo_w));
	map(0x73, 0x73).w(FUNC(pn8800fxb_state::video_addr_hi_w));
	map(0x74, 0x74).rw(FUNC(pn8800fxb_state::video_data_r), FUNC(pn8800fxb_state::video_data_w));
	map(0x75, 0x75).rw(FUNC(pn8800fxb_state::video_ctrl_r), FUNC(pn8800fxb_state::video_ctrl_w));
	map(0x76, 0x76).w(FUNC(pn8800fxb_state::video_offset_w));
	map(0x77, 0x77).w(FUNC(pn8800fxb_state::cursor_ctrl_w));
	// 78-7f mode select
	// 80-87 fdc
	// 88-8f input
	map(0x90, 0x90).mirror(0x07).w(FUNC(pn8800fxb_state::mem_change_w));
	// 98-9f rs break
	// a0-a7 rs232
	map(0xa8, 0xa8).mirror(0x07).nopr(); // "not used" - but read often
	// b0-b7 power supply
	map(0xb8, 0xb8).rw(FUNC(pn8800fxb_state::keyboard_r), FUNC(pn8800fxb_state::keyboard_w));
	// c0-c7 cdcc data
	// c8-cf cdcc control
	map(0xd0, 0xdf).rw("rtc", FUNC(tc8521_device::read), FUNC(tc8521_device::write));
	map(0xe0, 0xe0).mirror(0x07).w(FUNC(pn8800fxb_state::bank_select_w));
	// e8-ef ga test
	// f0-f7 buzzer
	map(0xf8, 0xf8).mirror(0x07).w(FUNC(pn8800fxb_state::int_ack_w));
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( pn8800fxb )
	PORT_START("KO0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)    PORT_CODE(KEYCODE_RSHIFT)      PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)      PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_NAME("\xe2\x86\x90  EXPR") // ←
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)                   PORT_NAME("BS")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)     PORT_CHAR(13)                  PORT_NAME("RETURN  IND CLR  =")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)        PORT_CHAR(UCHAR_MAMEKEY(UP))   PORT_NAME("\xe2\x86\x91  PRE S") // ↑
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)     PORT_CHAR(' ')

	PORT_START("KO1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[') PORT_CHAR(']') PORT_NAME(u8"[  ]  ½  ¼")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+') PORT_NAME("=  +  L OUT")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0') PORT_CHAR(')') PORT_NAME("0  )  W OUT")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('_') PORT_NAME("-  _  -")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P') PORT_NAME("p  P  PRINT  +")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?') PORT_NAME("/  ?  /")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KO2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)  PORT_CHAR(UCHAR_MAMEKEY(DOWN))                 PORT_NAME("\xe2\x86\x93  NEXT S") // ↓
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)     PORT_CHAR('l')  PORT_CHAR('L')                 PORT_NAME("l  L  L IND  3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)     PORT_CHAR('9')  PORT_CHAR('(')                 PORT_NAME("9  (  T CLR  9")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)     PORT_CHAR('o')  PORT_CHAR('O')                 PORT_NAME("o  O  JUST  6")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)  PORT_CHAR('.')  PORT_CHAR('>')  PORT_CHAR(']') PORT_NAME(".  >  ]  .")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';')  PORT_CHAR(':')                 PORT_NAME(";  :  *")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))                PORT_NAME("\xe2\x86\x92  RELOC") // →

	PORT_START("KO3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)     PORT_CHAR('k') PORT_CHAR('K')                PORT_NAME("k  K  KB  2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)     PORT_CHAR('8') PORT_CHAR('*')                PORT_NAME("8  *  DT SET  8")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)     PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)     PORT_CHAR('7') PORT_CHAR('&')                PORT_NAME("7  &  T SET  7")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)     PORT_CHAR('i') PORT_CHAR('I')                PORT_NAME("i  I  INSERT  5")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<') PORT_CHAR('[')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KO4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')                PORT_NAME("j  J  LAYOUT  1")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR('{') PORT_NAME("y  Y  {  <")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') PORT_CHAR('}') PORT_NAME("u  U  }  >")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')                PORT_NAME("h  H  HELP")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')                PORT_NAME("m  M  M CODE  0")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KO5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')                PORT_NAME("t  T  TEMP")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')                PORT_NAME("g  G  GOTO")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR(U'`')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KO6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)  PORT_CHAR(27)                 PORT_NAME("CANCEL")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)    PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)    PORT_CHAR('3') PORT_CHAR('#') PORT_NAME("3  #  M REL")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)    PORT_CHAR('5') PORT_CHAR('%') PORT_NAME("5  %  R MAR")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)    PORT_CHAR('4') PORT_CHAR('$') PORT_NAME("4  $  L MAR")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)    PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)    PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)                               PORT_NAME("MENU  FILE")

	PORT_START("KO7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)   PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR('|')  PORT_NAME(u8"x  X  |  ²")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)   PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(U'¢')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)                                  PORT_NAME("TAB  P IND")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)   PORT_CHAR('2') PORT_CHAR('@')                 PORT_NAME("2  @  LINE")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)   PORT_CHAR('s') PORT_CHAR('S')                 PORT_NAME("s  S  CALC")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KO8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)        PORT_CHAR('a')    PORT_CHAR('A')                 PORT_NAME("a  A  ABBR")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)        PORT_CHAR('q')    PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)    PORT_CHAR(U'`')   PORT_CHAR('~')                 PORT_NAME(u8"`  ~  SPELL  ±  °")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)        PORT_CHAR('1')    PORT_CHAR('!')                 PORT_NAME("1  !  PITCH")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))               PORT_NAME("CAPS  SHIFT LOCK")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)        PORT_CHAR('z')    PORT_CHAR('Z') PORT_CHAR('\\') PORT_NAME(u8"z  Z  \\  §")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)                         PORT_NAME("CODE")
INPUT_PORTS_END

uint8_t pn8800fxb_state::keyboard_r()
{
	if (m_key_select < 9)
		return m_keys[m_key_select]->read();
	else
		logerror("keyboard_r from %02x\n", m_key_select);

	return 0xff;
}

void pn8800fxb_state::keyboard_w(uint8_t data)
{
	m_key_select = data;
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

void pn8800fxb_state::palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(86, 228, 94));
	palette.set_pen_color(1, rgb_t(58, 86, 143));
}

uint32_t pn8800fxb_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_gfxdecode->gfx(0)->mark_all_dirty();

	const pen_t *const pen = m_palette->pens();

	// 22 line text mode doesn't cover the full screen
	bitmap.fill(pen[0], cliprect);

	if (BIT(m_video_ctrl, 0))
	{
		if (BIT(m_video_ctrl, 3))
		{
			// graphics mode
			uint8_t *vram = m_vram.get();

			// left 512 pixels of screen
			for (int y = 0; y < 200; y++)
				for (int x = 0; x < 512; x += 8, vram++)
					for (int p = 0; p < 8; p++)
						bitmap.pix(y, x + p) = pen[BIT(*vram, 7 - p)];

			// right 128 pixels of screen
			for (int y = 0; y < 200; y++)
				for (int x = 0; x < 128; x += 8, vram++)
					for (int p = 0; p < 8; p++)
						bitmap.pix(y, 512 + x + p) = pen[BIT(*vram, 7 - p)];
		}
		else
		{
			// text mode
			int lines = BIT(m_video_ctrl, 5) ? 25 : 22;

			for (int y = 0; y < lines; y++)
			{
				for (int x = 0; x < 80; x++)
				{
					// attributes
					//
					// 7-------  unknown
					// -6------  subscript
					// --5-----  superscript
					// ---4----  invert
					// ----3---  vertical line
					// -----2--  code high bit
					// ------1-  overline
					// -------0  underline

					uint8_t attr = m_vram[y * 0x100 + x * 2 + 0];
					uint16_t code = m_vram[y * 0x100 + x * 2 + 1];

					// apply code high bit (high table contains bold characters)
					code |= (BIT(attr, 2) << 8) | code;

					bool cursor_active = (y * 0x100 + x * 2) == (m_cursor_addr << 1);

					// draw 8 or 9 lines
					for (int l = 0; l < 9; l++)
					{
						uint8_t data = 0x00;

						// skip empty line in 25 line mode
						if (lines == 25 && l == 0)
							continue;

						// character data (or cursor) in lines 1 to 8
						if (l > 0)
						{
							if (cursor_active)
								data = 0xff;
							else
								data = m_vram[0x3000 + (code << 3) + l - 1];
						}

						// underline?
						if (BIT(attr, 0) && l == 8)
							data = 0xff;

						// overline?
						if (BIT(attr, 1) && l == 0)
							data = 0xff;

						// vertical line?
						if (BIT(attr, 3))
							data |= 0x01;

						// invert?
						if (BIT(attr, 4))
							data ^= 0xff;

						// draw 8 pixels of the character
						for (int b = 0; b < 8; b++)
						{
							if (lines == 25)
								bitmap.pix(y * 8 + l - 1, x * 8 + b) = pen[BIT(data, 7 - b)];
							else
								bitmap.pix(y * 9 + l, x * 8 + b) = pen[BIT(data, 7 - b)];
						}
					}
				}
			}
		}
	}

	return 0;
}

void pn8800fxb_state::cursor_addr_lo_w(uint8_t data)
{
	m_cursor_addr = (m_cursor_addr & 0xff00) | (data << 0);
}

void pn8800fxb_state::cursor_addr_hi_w(uint8_t data)
{
	data &= 0x0f;
	m_cursor_addr = (data << 8) | (m_cursor_addr & 0x00ff);
}

void pn8800fxb_state::video_addr_lo_w(uint8_t data)
{
	m_video_addr = (m_video_addr & 0xff00) | (data << 0);
}

void pn8800fxb_state::video_addr_hi_w(uint8_t data)
{
	data &= 0x7f;
	m_video_addr = (data << 8) | (m_video_addr & 0x00ff);
}

uint8_t pn8800fxb_state::video_data_r()
{
	return m_vram[m_video_addr];
}

void pn8800fxb_state::video_data_w(uint8_t data)
{
	m_vram[m_video_addr] = data;

	// auto-increment, assume wrap
	m_video_addr = (m_video_addr + 1) & 0x7fff;
}

uint8_t pn8800fxb_state::video_ctrl_r()
{
	// 7-------  mm
	// -6------  not used
	// --5-----  8r (in text mode, 8 pixel height line instead of 9?)
	// ---4----  blp
	// ----3---  grph
	// -----2--  not used
	// ------1-  rev
	// -------0  disp

	return m_video_ctrl;
}

void pn8800fxb_state::video_ctrl_w(uint8_t data)
{
	if (0)
		logerror("video_ctrl_w: %02x\n", data);

	m_video_ctrl = data;
}

void pn8800fxb_state::video_offset_w(uint8_t data)
{
	// 765-----  not used
	// ---4----  oy
	// ----3210  ox6 to ox3

	m_video_offset = data;
}

void pn8800fxb_state::cursor_ctrl_w(uint8_t data)
{
	// 7-------  curu
	// -6------  curl
	// --5-----  cbkh
	// ---4----  crev
	// ----3---  curb
	// -----210  not used

	m_cursor_ctrl = data;
}

static const gfx_layout charlayout =
{
	8, 8,
	512,
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	8*8
};

static GFXDECODE_START( gfx )
GFXDECODE_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void pn8800fxb_state::mem_change_w(uint8_t data)
{
	// 7-------  dua
	// -6------  ramin
	// --5432--  not used
	// ------1-  dicsel
	// -------0  rgex

	if (data != 0x40)
		logerror("mem_change_w: dua %d ramin %d dicsel %d rgex %d\n", BIT(data, 7), BIT(data, 6), BIT(data, 1), BIT(data, 0));

	m_mem_change = data;
	m_lomem_view.select(BIT(data, 6));
}

void pn8800fxb_state::bank_select_w(uint8_t data)
{
	// 7654----  not used
	// ----3210  bank selection

	if (0)
		logerror("bank_select_w: %02x\n", data);

	if (data < 12)
		m_rombank->set_entry(data & 0x07); // TODO, but works for now
}

void pn8800fxb_state::int_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER( pn8800fxb_state::int1_timer )
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
}

void pn8800fxb_state::machine_start()
{
	// allocate space for vram
	m_vram = std::make_unique<uint8_t[]>(0x4000);

	// init gfxdecode
	m_gfxdecode->set_gfx(0, std::make_unique<gfx_element>(m_palette, charlayout, m_vram.get() + 0x3000, 0, 1, 0));

	// configure rom banking (first 0x40000 fixed?)
	m_rombank->configure_entries(0, 12, memregion("maincpu")->base() + 0x40000, 0x10000);

	// register for save states
	save_item(NAME(m_key_select));
	save_pointer(NAME(m_vram), 0x4000);
	save_item(NAME(m_cursor_addr));
	save_item(NAME(m_video_addr));
	save_item(NAME(m_video_ctrl));
	save_item(NAME(m_video_offset));
	save_item(NAME(m_cursor_ctrl));
	save_item(NAME(m_mem_change));
}

void pn8800fxb_state::machine_reset()
{
	m_mem_change = 0x00;
	m_lomem_view.select(0);
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void pn8800fxb_state::pn8800fxb(machine_config &config)
{
	Z80180(config, m_maincpu, 12.288_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &pn8800fxb_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &pn8800fxb_state::io_map);

	TIMER(config, "1khz").configure_periodic(FUNC(pn8800fxb_state::int1_timer), attotime::from_hz(1000));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_size(640, 200);
	screen.set_visarea_full();
	screen.set_refresh_hz(70.23);
	screen.set_screen_update(FUNC(pn8800fxb_state::screen_update));

	PALETTE(config, m_palette, FUNC(pn8800fxb_state::palette), 2);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx);

	TC8521(config, "rtc", XTAL(32'768));
	// alarm output connected to auto power-on
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( pn8800 )
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("uc8254-a-pn88.5", 0x000000, 0x100000, CRC(d9601c1a) SHA1(1699714befeaf2fe17232c1b4f49d4242f5367f4))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY     FULLNAME      FLAGS
COMP( 1996, pn8800, 0,      0,      pn8800fxb, pn8800fxb, pn8800fxb_state, empty_init, "Brother",  "PN-8800FXB", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
