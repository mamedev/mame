// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

        SAPI-1 driver by Miodrag Milanovic

        2008-09-09 Preliminary driver.

        2010-12-07 Added some code to allow sapizps3 to read its rom.
        With no available docs, the i/o ports are a guess. The ram
        allocation is based on the actions of the various bios roms.
        Port 25 is used as a jump vector. in a,(25); ld l,a; jp(hl).

        2012-04-19 Connected sapizps3 to a terminal. It is trying to
        load a 128-byte boot sector from a floppy disk.
        Modernised driver.
        Connected sapizps2 to ascii keyboard. System is now usable.
        According to wikipedia, sapi1 & 2 have cassette facility,
        while sapi3 uses 8 inch floppy disk.

ToDo:
- Add cassette to sapi1 and 2
-- UART on port 12, save and load bytes
-- port 11 bit 7 for load, chr available
-- port 11 bit 6 for save, tx buffer empty
- sapi3 is trying to read a disk, so there is no response after showing the logo

Unable to proceed due to no info available (& in English).

****************************************************************************/


#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "cpu/z80/z80.h"
#include "video/mc6845.h"
#include "machine/ram.h"
#include "machine/keyboard.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"
#define KEYBOARD_TAG "keyboard"

class sapi1_state : public driver_device
{
public:
	sapi1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_p_videoram(*this, "videoram"),
		m_bank1(*this, "bank1"),
		m_line0(*this, "LINE0"),
		m_line1(*this, "LINE1"),
		m_line2(*this, "LINE2"),
		m_line3(*this, "LINE3"),
		m_line4(*this, "LINE4"),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette")
	{
	}

	optional_shared_ptr<UINT8> m_p_videoram;
	DECLARE_READ8_MEMBER(sapi1_keyboard_r);
	DECLARE_WRITE8_MEMBER(sapi1_keyboard_w);
	DECLARE_READ8_MEMBER(sapi2_keyboard_status_r);
	DECLARE_READ8_MEMBER(sapi2_keyboard_data_r);
	DECLARE_READ8_MEMBER(sapi3_0c_r);
	DECLARE_WRITE8_MEMBER(sapi3_00_w);
	DECLARE_READ8_MEMBER(sapi3_25_r);
	DECLARE_WRITE8_MEMBER(sapi3_25_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_DRIVER_INIT(sapizps3);
	DECLARE_DRIVER_INIT(sapizps3a);
	DECLARE_DRIVER_INIT(sapizps3b);
	DECLARE_MACHINE_RESET(sapi1);
	DECLARE_MACHINE_RESET(sapizps3);
	MC6845_UPDATE_ROW(crtc_update_row);
	UINT32 screen_update_sapi1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_sapi3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
private:
	UINT8 m_term_data;
	UINT8 m_keyboard_mask;
	UINT8 m_refresh_counter;
	UINT8 m_zps3_25;
	optional_memory_bank m_bank1;   // Only for sapi3
	required_ioport m_line0;
	required_ioport m_line1;
	required_ioport m_line2;
	required_ioport m_line3;
	required_ioport m_line4;
	required_device<cpu_device> m_maincpu;
public:
	optional_device<palette_device> m_palette;
};

static const UINT8 MHB2501[] = {
	0x0c,0x11,0x13,0x15,0x17,0x10,0x0e,0x00, // @
	0x04,0x0a,0x11,0x11,0x1f,0x11,0x11,0x00, // A
	0x1e,0x11,0x11,0x1e,0x11,0x11,0x1e,0x00, // B
	0x0e,0x11,0x10,0x10,0x10,0x11,0x0e,0x00, // C
	0x1e,0x09,0x09,0x09,0x09,0x09,0x1e,0x00, // D
	0x1f,0x10,0x10,0x1e,0x10,0x10,0x1f,0x00, // E
	0x1f,0x10,0x10,0x1e,0x10,0x10,0x10,0x00, // F
	0x0e,0x11,0x10,0x10,0x13,0x11,0x0f,0x00, // G

	0x11,0x11,0x11,0x1f,0x11,0x11,0x11,0x00, // H
	0x0e,0x04,0x04,0x04,0x04,0x04,0x0e,0x00, // I
	0x01,0x01,0x01,0x01,0x11,0x11,0x0e,0x00, // J
	0x11,0x12,0x14,0x18,0x14,0x12,0x11,0x00, // K
	0x10,0x10,0x10,0x10,0x10,0x10,0x1f,0x00, // L
	0x11,0x1b,0x15,0x15,0x11,0x11,0x11,0x00, // M
	0x11,0x11,0x19,0x15,0x13,0x11,0x11,0x00, // N
	0x0e,0x11,0x11,0x11,0x11,0x11,0x0e,0x00, // O

	0x1e,0x11,0x11,0x1e,0x10,0x10,0x10,0x00, // P
	0x0e,0x11,0x11,0x11,0x15,0x12,0x0d,0x00, // Q
	0x1e,0x11,0x11,0x1e,0x14,0x12,0x11,0x00, // R
	0x0e,0x11,0x10,0x0e,0x01,0x11,0x0e,0x00, // S
	0x1f,0x04,0x04,0x04,0x04,0x04,0x04,0x00, // T
	0x11,0x11,0x11,0x11,0x11,0x11,0x0e,0x00, // U
	0x11,0x11,0x11,0x0a,0x0a,0x04,0x04,0x00, // V
	0x11,0x11,0x11,0x15,0x15,0x15,0x0a,0x00, // W

	0x11,0x11,0x0a,0x04,0x0a,0x11,0x11,0x00, // X
	0x11,0x11,0x0a,0x04,0x04,0x04,0x04,0x00, // Y
	0x1f,0x01,0x02,0x04,0x08,0x10,0x1f,0x00, // Z
	0x1c,0x10,0x10,0x10,0x10,0x10,0x1c,0x00, // [
	0x00,0x10,0x08,0x04,0x02,0x01,0x00,0x00, // backslash
	0x07,0x01,0x01,0x01,0x01,0x01,0x07,0x00, // ]
	0x0e,0x11,0x00,0x00,0x00,0x00,0x00,0x00, // ^
	0x00,0x00,0x00,0x00,0x00,0x00,0x1f,0x00, // _

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //
	0x04,0x04,0x04,0x04,0x04,0x00,0x04,0x00, // !
	0x0a,0x0a,0x0a,0x00,0x00,0x00,0x00,0x00, // "
	0x0a,0x0a,0x1f,0x0a,0x1f,0x0a,0x0a,0x00, // #
	0x00,0x11,0x0e,0x0a,0x0e,0x11,0x00,0x00, //
	0x18,0x19,0x02,0x04,0x08,0x13,0x03,0x00, // %
	0x04,0x0a,0x0a,0x0c,0x15,0x12,0x0d,0x00, // &
	0x04,0x04,0x08,0x00,0x00,0x00,0x00,0x00, // '

	0x02,0x04,0x08,0x08,0x08,0x04,0x02,0x00, // (
	0x08,0x04,0x02,0x02,0x02,0x04,0x08,0x00, // )
	0x00,0x04,0x15,0x0e,0x15,0x04,0x00,0x00, // *
	0x00,0x04,0x04,0x1f,0x04,0x04,0x00,0x00, // +
	0x00,0x00,0x00,0x00,0x08,0x08,0x10,0x00, // ,
	0x00,0x00,0x00,0x1f,0x00,0x00,0x00,0x00, // -
	0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00, // .
	0x00,0x01,0x02,0x04,0x08,0x10,0x00,0x00, // /

	0x0e,0x11,0x13,0x15,0x19,0x11,0x0e,0x00, // 0
	0x04,0x0c,0x04,0x04,0x04,0x04,0x0e,0x00, // 1
	0x0e,0x11,0x01,0x06,0x08,0x10,0x1f,0x00, // 2
	0x1f,0x01,0x02,0x06,0x01,0x11,0x0e,0x00, // 3
	0x02,0x06,0x0a,0x12,0x1f,0x02,0x02,0x00, // 4
	0x1f,0x10,0x1e,0x01,0x01,0x11,0x0e,0x00, // 5
	0x07,0x08,0x10,0x1e,0x11,0x11,0x0e,0x00, // 6
	0x1f,0x01,0x02,0x04,0x08,0x08,0x08,0x00, // 7

	0x0e,0x11,0x11,0x0e,0x11,0x11,0x0e,0x00, // 8
	0x0e,0x11,0x11,0x0f,0x01,0x02,0x1c,0x00, // 9
	0x00,0x00,0x00,0x00,0x08,0x00,0x08,0x00, // :
	0x00,0x00,0x04,0x00,0x04,0x04,0x08,0x00, // ;
	0x02,0x04,0x08,0x10,0x08,0x04,0x02,0x00, // <
	0x00,0x00,0x1f,0x00,0x1f,0x00,0x00,0x00, // =
	0x08,0x04,0x02,0x01,0x02,0x04,0x08,0x00, // >
	0x0e,0x11,0x01,0x02,0x04,0x00,0x04,0x00  // ?
};


/* Address maps */
static ADDRESS_MAP_START(sapi1_mem, AS_PROGRAM, 8, sapi1_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x1000, 0x1fff) AM_ROM // Extension ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	AM_RANGE(0x2400, 0x27ff) AM_READWRITE(sapi1_keyboard_r, sapi1_keyboard_w) // PORT 0 - keyboard
	//AM_RANGE(0x2800, 0x2bff) AM_NOP // PORT 1
	//AM_RANGE(0x2c00, 0x2fff) AM_NOP // PORT 2
	//AM_RANGE(0x3000, 0x33ff) AM_NOP // 3214
	AM_RANGE(0x3800, 0x3fff) AM_RAM AM_SHARE("videoram") // AND-1 (video RAM)
	AM_RANGE(0x4000, 0x7fff) AM_RAM // REM-1
ADDRESS_MAP_END

static ADDRESS_MAP_START(sapi2_mem, AS_PROGRAM, 8, sapi1_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x1000, 0x1fff) AM_ROM // Extension ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	AM_RANGE(0x2400, 0x27ff) AM_READ(sapi2_keyboard_status_r)
	AM_RANGE(0x2800, 0x28ff) AM_READ(sapi2_keyboard_data_r)
	AM_RANGE(0x3800, 0x3fff) AM_RAM AM_SHARE("videoram") // AND-1 (video RAM)
	AM_RANGE(0x4000, 0x7fff) AM_RAM // REM-1
ADDRESS_MAP_END

static ADDRESS_MAP_START(sapi3_mem, AS_PROGRAM, 8, sapi1_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_RAMBANK("bank1")
	AM_RANGE(0x0800, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xffff) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START(sapi3a_mem, AS_PROGRAM, 8, sapi1_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_RAMBANK("bank1")
	AM_RANGE(0x0800, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xfdff) AM_ROM
	AM_RANGE(0xfe00, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(sapi3b_mem, AS_PROGRAM, 8, sapi1_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_RAMBANK("bank1")
	AM_RANGE(0x0800, 0xafff) AM_RAM
	AM_RANGE(0xb000, 0xb7ff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0xb800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sapi3_io, AS_IO, 8, sapi1_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(sapi3_00_w)
	AM_RANGE(0x25, 0x25) AM_READWRITE(sapi3_25_r,sapi3_25_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sapi3a_io, AS_IO, 8, sapi1_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(sapi3_00_w)
	AM_RANGE(0x12, 0x12) AM_READ(sapi2_keyboard_data_r) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE(0x25, 0x25) AM_READWRITE(sapi3_25_r,sapi3_25_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sapi3b_io, AS_IO, 8, sapi1_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(sapi3_00_w)
	AM_RANGE(0x0c, 0x0c) AM_READ(sapi3_0c_r)
	AM_RANGE(0x25, 0x25) AM_READWRITE(sapi3_25_r,sapi3_25_w)
	AM_RANGE(0xe0, 0xe0) AM_DEVREADWRITE("crtc", mc6845_device, status_r, address_w)
	AM_RANGE(0xe1, 0xe1) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( sapi1 )
	PORT_START("LINE0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('~')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR(';')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('+')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('/')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('\'')

	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('\xA4')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR(',')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('-')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR(':')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('<')

	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('"')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('.')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('*')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('@')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('>')

	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('?')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('=')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('#')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')

	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
INPUT_PORTS_END



/**************************************

    Video

**************************************/

UINT32 sapi1_state::screen_update_sapi1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bool val;
	UINT16 addr,xpos;
	UINT8 chr,attr,ra,x,y,b;

	for(y = 0; y < 24; y++ )
	{
		addr = y*64;
		xpos = 0;
		for(x = 0; x < 40; x++ )
		{
			chr = m_p_videoram[addr + x];
			attr = (chr >> 6) & 3;
			chr &= 0x3f;
			for(ra = 0; ra < 9; ra++ )
			{
				for(b = 0; b < 6; b++ )
				{
					val = 0;

					if (ra==8)
					{
						if (attr==2)
							val = BIT(m_refresh_counter, 5);
					}
					else
					{
						val = BIT(MHB2501[(chr<<3) | ra], 5-b);
						if (attr==1)
							val = BIT(m_refresh_counter, 5) ? val : 0;
					}

					if(attr==3)
					{
						bitmap.pix16(y*9+ra, xpos+2*b   ) = val;
						bitmap.pix16(y*9+ra, xpos+2*b+1 ) = val;
					}
					else
					{
						bitmap.pix16(y*9+ra, xpos+b ) = val;
					}
				}
			}
			xpos+= (attr==3) ? 12 : 6;
			if (xpos>=6*40) break;
		}
	}
	m_refresh_counter++;
	return 0;
}

// The attributes seem to be different on this one, they need to be understood, so disabled for now
UINT32 sapi1_state::screen_update_sapi3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bool val;
	UINT16 addr,xpos;
	UINT8 chr,attr,ra,x,y,b;

	for(y = 0; y < 20; y++ )
	{
		addr = y*64;
		xpos = 0;
		for(x = 0; x < 40; x++ )
		{
			chr = m_p_videoram[addr + x];
			attr = 0;//(chr >> 6) & 3;
			if (chr > 0x3f)
				chr &= 0x1f;

			for(ra = 0; ra < 9; ra++ )
			{
				for(b = 0; b < 6; b++ )
				{
					val = 0;

					if (ra==8)
					{
						if (attr==2)
							val = BIT(m_refresh_counter, 5);
					}
					else
					{
						val = BIT(MHB2501[(chr<<3) | ra], 5-b);
						if (attr==1)
							val = BIT(m_refresh_counter, 5) ? val : 0;
					}

					if(attr==3)
					{
						bitmap.pix16(y*9+ra, xpos+2*b   ) = val;
						bitmap.pix16(y*9+ra, xpos+2*b+1 ) = val;
					}
					else
					{
						bitmap.pix16(y*9+ra, xpos+b ) = val;
					}
				}
			}
			xpos+= (attr==3) ? 12 : 6;
			if (xpos>=6*40) break;
		}
	}
	m_refresh_counter++;
	return 0;
}

MC6845_UPDATE_ROW( sapi1_state::crtc_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT8 chr,gfx,inv;
	UINT16 mem,x;
	UINT32 *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)
	{
		inv = gfx = 0;
		if (x == cursor_x) inv ^= 0xff;
		mem = (2*(ma + x)) & 0xfff;
		chr = m_p_videoram[mem] & 0x3f;

		if (ra < 8)
			gfx = MHB2501[(chr<<3) | ra] ^ inv;

		/* Display a scanline of a character */
		*p++ = palette[BIT(gfx, 5)];
		*p++ = palette[BIT(gfx, 4)];
		*p++ = palette[BIT(gfx, 3)];
		*p++ = palette[BIT(gfx, 2)];
		*p++ = palette[BIT(gfx, 1)];
		*p++ = palette[BIT(gfx, 0)];
	}
}

/**************************************

    Keyboard

**************************************/

READ8_MEMBER( sapi1_state::sapi1_keyboard_r )
{
	UINT8 key = 0xff;
	if (BIT(m_keyboard_mask, 0)) { key &= m_line0->read(); }
	if (BIT(m_keyboard_mask, 1)) { key &= m_line1->read(); }
	if (BIT(m_keyboard_mask, 2)) { key &= m_line2->read(); }
	if (BIT(m_keyboard_mask, 3)) { key &= m_line3->read(); }
	if (BIT(m_keyboard_mask, 4)) { key &= m_line4->read(); }
	return key;
}

WRITE8_MEMBER( sapi1_state::sapi1_keyboard_w )
{
	m_keyboard_mask = (data ^ 0xff ) & 0x1f;
}

READ8_MEMBER( sapi1_state::sapi2_keyboard_status_r)
{
	return (m_term_data) ? 0 : 1;
}

READ8_MEMBER( sapi1_state::sapi2_keyboard_data_r)
{
	UINT8 ret = ~m_term_data;
	m_term_data = 0;
	return ret;
}

WRITE8_MEMBER( sapi1_state::kbd_put )
{
	m_term_data = data;
}

/**************************************

    Machine

**************************************/

READ8_MEMBER( sapi1_state::sapi3_0c_r )
{
	return 0xc0;
}

/* switch out the rom shadow */
WRITE8_MEMBER( sapi1_state::sapi3_00_w )
{
	m_bank1->set_entry(0);
}

/* to stop execution in random ram */
READ8_MEMBER( sapi1_state::sapi3_25_r )
{
	return m_zps3_25;
}

WRITE8_MEMBER( sapi1_state::sapi3_25_w )
{
	m_zps3_25 = data & 0xfc; //??
}

MACHINE_RESET_MEMBER( sapi1_state, sapi1 )
{
	m_keyboard_mask = 0;
	m_refresh_counter = 0x20;
}

MACHINE_RESET_MEMBER( sapi1_state, sapizps3 )
{
	m_keyboard_mask = 0;
	m_bank1->set_entry(1);
}

DRIVER_INIT_MEMBER( sapi1_state, sapizps3 )
{
	UINT8 *RAM = memregion("maincpu")->base();
	m_bank1->configure_entries(0, 2, &RAM[0x0000], 0x10000);
}

DRIVER_INIT_MEMBER( sapi1_state, sapizps3a )
{
	UINT8 *RAM = memregion("maincpu")->base();
	m_bank1->configure_entries(0, 2, &RAM[0x0000], 0xf800);
}

DRIVER_INIT_MEMBER( sapi1_state, sapizps3b )
{
	UINT8 *RAM = memregion("maincpu")->base();
	m_bank1->configure_entries(0, 2, &RAM[0x0000], 0x10000);
}


/* Machine driver */
static MACHINE_CONFIG_START( sapi1, sapi1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8080, 2000000)
	MCFG_CPU_PROGRAM_MAP(sapi1_mem)
	MCFG_MACHINE_RESET_OVERRIDE(sapi1_state, sapi1)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(40*6, 24*9)
	MCFG_SCREEN_VISIBLE_AREA(0, 40*6-1, 0, 24*9-1)
	MCFG_SCREEN_UPDATE_DRIVER(sapi1_state, screen_update_sapi1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_MONOCHROME("palette")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sapi2, sapi1 )
	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sapi2_mem)
	MCFG_DEVICE_ADD(KEYBOARD_TAG, GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(WRITE8(sapi1_state, kbd_put))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sapi3, sapi2 )
	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sapi3_mem)
	MCFG_CPU_IO_MAP(sapi3_io)
	MCFG_MACHINE_RESET_OVERRIDE(sapi1_state, sapizps3 )
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_SIZE(40*6, 20*9)
	MCFG_SCREEN_VISIBLE_AREA(0, 40*6-1, 0, 20*9-1)
	MCFG_SCREEN_UPDATE_DRIVER(sapi1_state, screen_update_sapi3)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sapi3b, sapi3 )
	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sapi3b_mem)
	MCFG_CPU_IO_MAP(sapi3b_io)

	MCFG_MC6845_ADD("crtc", MC6845, "screen", 1008000) // guess
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(6)
	MCFG_MC6845_UPDATE_ROW_CB(sapi1_state, crtc_update_row)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)
	MCFG_SCREEN_NO_PALETTE
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( sapi3a, sapi1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 2000000)
	MCFG_CPU_PROGRAM_MAP(sapi3a_mem)
	MCFG_CPU_IO_MAP(sapi3a_io)
	MCFG_MACHINE_RESET_OVERRIDE(sapi1_state, sapizps3 )

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(sapi1_state, kbd_put))

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END




/**************************************

    Roms

**************************************/

ROM_START( sapi1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "mb1", "MB1" )
	ROMX_LOAD( "sapi1.rom", 0x0000, 0x1000, CRC(c6e85b01) SHA1(2a26668249c6161aef7215a1e2b92bfdf6fe3671), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "mb2", "MB2 (ANK-1)" )
	ROMX_LOAD( "mb2_4.bin", 0x0000, 0x1000, CRC(a040b3e0) SHA1(586990a07a96323741679a11ff54ad0023da87bc), ROM_BIOS(2))
ROM_END

ROM_START( sapizps2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "v4", "MIKOS 4" )
	ROMX_LOAD( "36.bin", 0x0000, 0x0800, CRC(a27f340a) SHA1(d07d208fcbe428897336c17197d3e8fb52181f38), ROM_BIOS(1))
	ROMX_LOAD( "37.bin", 0x0800, 0x0800, CRC(30daa708) SHA1(66e990c40788ee25cf6cabd4842a78daf4fcdddd), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "v5", "MIKOS 5" )
	ROMX_LOAD( "mikos5_1.bin", 0x0000, 0x0800, CRC(c2a83ca3) SHA1(a3678253d7690c89945e791ea0f8e15b081c9126), ROM_BIOS(2))
	ROMX_LOAD( "mikos5_2.bin", 0x0800, 0x0800, CRC(c4458a04) SHA1(0cc909323f0e6507d95e57ea39e1deb8bd57bf89), ROM_BIOS(2))
	ROMX_LOAD( "mikos5_3.bin", 0x1000, 0x0800, CRC(efb499f3) SHA1(78f0ca3ff10d7af4ae94ab820723296beb035f8f), ROM_BIOS(2))
	ROMX_LOAD( "mikos5_4.bin", 0x1800, 0x0800, CRC(4d90e9be) SHA1(8ec554198697550a49432e8210d43700ef1d6a32), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 2, "mb3", "MB3 (Consul)" )
	ROMX_LOAD( "mb3_1.bin", 0x0000, 0x1000, CRC(be895f88) SHA1(7fc2a92f41d978a9f0ccd0e235ea3c6146adfb6f), ROM_BIOS(3))
ROM_END

ROM_START( sapizps3 )
	ROM_REGION( 0x10800, "maincpu", 0 )
	// These 2 bioses use videoram at F800
	ROM_SYSTEM_BIOS( 0, "per", "Perina" )
	ROMX_LOAD( "perina_1988.bin",0x10000, 0x0800, CRC(d71e8d3a) SHA1(9b3a26ea7c2f2c8a1fb10b51c1c880acc9fd806d), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "1zmod", "JPR-1Zmod" )
	ROMX_LOAD( "jpr1zmod.bin",   0x10000, 0x0800, CRC(69a29b07) SHA1(1cd31032954fcd7d10b1586be62db6f7597eb4f2), ROM_BIOS(2))
ROM_END

ROM_START( sapizps3a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	// This bios uses a terminal
	ROM_LOAD( "jpr1a.bin",      0xf800, 0x0800, CRC(3ed89786) SHA1(dcc8657b4884bfe58d114c539b733b73d038ee30))
ROM_END

ROM_START( sapizps3b )
	ROM_REGION( 0x10800, "maincpu", 0 )
	// This bios uses a 6845
	ROM_LOAD( "pkt1.bin",       0x10000, 0x0800, CRC(ed5a2725) SHA1(3383c15f87f976400b8d0f31829e2a95236c4b6c))
ROM_END


/* Driver */

/*    YEAR  NAME      PARENT   COMPAT  MACHINE     INPUT  CLASS           INIT      COMPANY    FULLNAME       FLAGS */
COMP( 1985, sapi1,    0,       0,      sapi1,      sapi1, driver_device,  0,         "Tesla", "SAPI-1 ZPS 1", MACHINE_NO_SOUND_HW)
COMP( 1985, sapizps2, sapi1,   0,      sapi2,      sapi1, driver_device,  0,         "Tesla", "SAPI-1 ZPS 2", MACHINE_NO_SOUND_HW)
COMP( 1985, sapizps3, sapi1,   0,      sapi3,      sapi1, sapi1_state,    sapizps3,  "Tesla", "SAPI-1 ZPS 3", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
COMP( 1985, sapizps3a,sapi1,   0,      sapi3a,     sapi1, sapi1_state,    sapizps3a, "Tesla", "SAPI-1 ZPS 3 (terminal)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
COMP( 1985, sapizps3b,sapi1,   0,      sapi3b,     sapi1, sapi1_state,    sapizps3b, "Tesla", "SAPI-1 ZPS 3 (6845)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
