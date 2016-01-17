// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        Unior

    2009-05-12 Skeleton driver.
    2013-10-09 Added DMA and CRTC
    2013-10-11 Added PPI, PIT, UART, sound

    Some info obtained from EMU-80.
    The schematic is difficult to read, and some code is guesswork.

The monitor will only allow certain characters to be typed, thus the
modifier keys appear to do nothing. There is no need to use the enter
key; using spacebar and the correct parameters is enough.

If you press Shift, indicators for numlock and capslock will appear.

Monitor commands:
C
D - hex dump
E - save
F
G
H - set register
I - load
J - modify memory
K
L - list registers
M

ToDo:
- Cassette
- Colour

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "machine/i8257.h"
#include "video/i8275.h"
#include "sound/speaker.h"


class unior_state : public driver_device
{
public:
	unior_state(const machine_config &mconfig, device_type type, std::string tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pit(*this, "pit"),
		m_dma(*this, "dma"),
		m_uart(*this, "uart"),
		m_palette(*this, "palette")
	{
	}

	DECLARE_WRITE8_MEMBER(vram_w);
	DECLARE_WRITE8_MEMBER(scroll_w);
	DECLARE_WRITE_LINE_MEMBER(write_uart_clock);
	DECLARE_READ8_MEMBER(ppi0_b_r);
	DECLARE_WRITE8_MEMBER(ppi0_b_w);
	DECLARE_READ8_MEMBER(ppi1_a_r);
	DECLARE_READ8_MEMBER(ppi1_b_r);
	DECLARE_READ8_MEMBER(ppi1_c_r);
	DECLARE_WRITE8_MEMBER(ppi1_a_w);
	DECLARE_WRITE8_MEMBER(ppi1_c_w);
	DECLARE_WRITE_LINE_MEMBER(hrq_w);
	DECLARE_PALETTE_INIT(unior);
	DECLARE_READ8_MEMBER(dma_r);
	I8275_DRAW_CHARACTER_MEMBER(display_pixels);

	UINT8 *m_p_vram;
	UINT8 *m_p_chargen;
private:
	UINT8 m_4c;
	UINT8 m_4e;
	virtual void machine_reset() override;
	virtual void video_start() override;
	required_device<cpu_device> m_maincpu;
	required_device<pit8253_device> m_pit;
	required_device<i8257_device> m_dma;
	required_device<i8251_device> m_uart;
public:
	required_device<palette_device> m_palette;
};

static ADDRESS_MAP_START( unior_mem, AS_PROGRAM, 8, unior_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xffff) AM_ROM AM_WRITE(vram_w) // main video
ADDRESS_MAP_END

static ADDRESS_MAP_START( unior_io, AS_IO, 8, unior_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x30, 0x38) AM_DEVREADWRITE("dma", i8257_device, read, write) // dma data
	AM_RANGE(0x3c, 0x3f) AM_DEVREADWRITE("ppi0", i8255_device, read, write) // cassette player control
	AM_RANGE(0x4c, 0x4f) AM_DEVREADWRITE("ppi1", i8255_device, read, write)
	AM_RANGE(0x50, 0x50) AM_WRITE(scroll_w)
	AM_RANGE(0x60, 0x61) AM_DEVREADWRITE("crtc", i8275_device, read, write)
	AM_RANGE(0xdc, 0xdf) AM_DEVREADWRITE("pit", pit8253_device, read, write )
	AM_RANGE(0xec, 0xec) AM_DEVREADWRITE("uart",i8251_device, data_r, data_w)
	AM_RANGE(0xed, 0xed) AM_DEVREADWRITE("uart", i8251_device, status_r, control_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( unior )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED) // nothing
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED) // nothing
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_CHAR(27) // cancel input
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR(9) // Russian little M
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Rus/Lat") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NUMLK") PORT_CODE(KEYCODE_NUMLOCK) // switches 1st indicator between N and B - numlock? rctrl
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') // ;
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CAPS") PORT_CODE(KEYCODE_CAPSLOCK) // switches 2nd indicator between U and B - capslock? rshift
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) // A
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('^') // ^ (')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) // B
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) // H
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) // I

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) // C
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BSPACE") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) // G
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LF") PORT_CODE(KEYCODE_HOME) PORT_CHAR(10) // line feed?

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('\xA4')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I') // I then hangs
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) // Russian A
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) // ? (/)

	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) // D
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) // Russian bl
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) // Russian signpost
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED) // } (<>)

	PORT_START("X8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) // E
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('>') // >
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("= :") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(':') // :
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("- =") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('=') // =

	PORT_START("X9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) // Russian U
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')

	PORT_START("XA")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('@') // (`)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR('<') // <
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
INPUT_PORTS_END


/*************************************************

    Video

*************************************************/

/* F4 Character Displayer */
static const gfx_layout unior_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{  0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( unior )
	GFXDECODE_ENTRY( "chargen", 0x0000, unior_charlayout, 0, 1 )
GFXDECODE_END

WRITE8_MEMBER( unior_state::vram_w )
{
	m_p_vram[offset] = data;
}

// pulses a 1 to scroll
WRITE8_MEMBER( unior_state::scroll_w )
{
	if (data)
		memcpy(m_p_vram, m_p_vram+80, 24*80);
}

I8275_DRAW_CHARACTER_MEMBER(unior_state::display_pixels)
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT8 gfx = m_p_chargen[(linecount & 7) | (charcode << 3)];

	if(linecount == 8)
		gfx = 0;

	if (vsp)
		gfx = 0;

	if (lten)
		gfx = 0xff;

	if (rvv)
		gfx ^= 0xff;

	for(UINT8 i=0;i<6;i++)
		bitmap.pix32(y, x + i) = palette[BIT(gfx, 5-i) ? (hlgt ? 2 : 1) : 0];
}

static const rgb_t unior_palette[3] =
{
	rgb_t(0x00, 0x00, 0x00), // black
	rgb_t(0xa0, 0xa0, 0xa0), // white
	rgb_t(0xff, 0xff, 0xff)  // highlight
};

PALETTE_INIT_MEMBER(unior_state,unior)
{
	palette.set_pen_colors(0, unior_palette, ARRAY_LENGTH(unior_palette));
}


/*************************************************

    i8255

*************************************************/


WRITE_LINE_MEMBER(unior_state::write_uart_clock)
{
	m_uart->write_txc(state);
	m_uart->write_rxc(state);
}

READ8_MEMBER( unior_state::ppi0_b_r )
{
	return 0;
}

WRITE8_MEMBER( unior_state::ppi0_b_w )
{
}

READ8_MEMBER( unior_state::ppi1_a_r )
{
	return m_4c;
}

READ8_MEMBER( unior_state::ppi1_b_r )
{
	char kbdrow[6];
	sprintf(kbdrow,"X%X", m_4c&15);
	return ioport(kbdrow)->read();
}

READ8_MEMBER( unior_state::ppi1_c_r )
{
	return m_4e;
}

WRITE8_MEMBER( unior_state::ppi1_a_w )
{
	m_4c = data;
}

/*
d0,1,2 = connect to what might be a 74LS138, then to an external slot
d4 = speaker gate
d5 = unknown
d6 = connect to A7 of the palette prom
d7 = not used
*/
WRITE8_MEMBER( unior_state::ppi1_c_w )
{
	m_4e = data;
	m_pit->write_gate2(BIT(data, 4));
}

/*************************************************

    i8257

*************************************************/

READ8_MEMBER(unior_state::dma_r)
{
	if (offset < 0xf800)
		return m_maincpu->space(AS_PROGRAM).read_byte(offset);
	else
		return m_p_vram[offset & 0x7ff];
}

WRITE_LINE_MEMBER( unior_state::hrq_w )
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);
	m_dma->hlda_w(state);
}


/*************************************************

    Machine

*************************************************/

void unior_state::machine_reset()
{
	m_maincpu->set_state_int(I8085_PC, 0xF800);
}

void unior_state::video_start()
{
	m_p_chargen = memregion("chargen")->base();
	m_p_vram = memregion("vram")->base();
}

static MACHINE_CONFIG_START( unior, unior_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8080, XTAL_20MHz / 9)
	MCFG_CPU_PROGRAM_MAP(unior_mem)
	MCFG_CPU_IO_MAP(unior_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(640, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", i8275_device, screen_update)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", unior)
	MCFG_PALETTE_ADD("palette", 3)
	MCFG_PALETTE_INIT_OWNER(unior_state,unior)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* Devices */
	MCFG_DEVICE_ADD("uart", I8251, 0)

	MCFG_DEVICE_ADD("pit", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_20MHz / 12)
	MCFG_PIT8253_CLK1(XTAL_20MHz / 9)
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(unior_state, write_uart_clock))
	MCFG_PIT8253_CLK2(XTAL_16MHz / 9 / 64) // unknown frequency
	MCFG_PIT8253_OUT2_HANDLER(DEVWRITELINE("speaker", speaker_sound_device, level_w))

	MCFG_DEVICE_ADD("ppi0", I8255, 0)
	// ports a & c connect to an external slot
	MCFG_I8255_IN_PORTB_CB(READ8(unior_state, ppi0_b_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(unior_state, ppi0_b_w))

	MCFG_DEVICE_ADD("ppi1", I8255, 0)
	// ports a & b are for the keyboard
	// port c operates various control lines for mostly unknown purposes
	MCFG_I8255_IN_PORTA_CB(READ8(unior_state, ppi1_a_r))
	MCFG_I8255_OUT_PORTA_CB(WRITE8(unior_state, ppi1_a_w))
	MCFG_I8255_IN_PORTB_CB(READ8(unior_state, ppi1_b_r))
	MCFG_I8255_IN_PORTC_CB(READ8(unior_state, ppi1_c_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(unior_state, ppi1_c_w))

	MCFG_DEVICE_ADD("dma", I8257, XTAL_20MHz / 9)
	MCFG_I8257_OUT_HRQ_CB(WRITELINE(unior_state, hrq_w))
	MCFG_I8257_IN_MEMR_CB(READ8(unior_state, dma_r))
	MCFG_I8257_OUT_IOW_2_CB(DEVWRITE8("crtc", i8275_device, dack_w))

	MCFG_DEVICE_ADD("crtc", I8275, XTAL_20MHz / 12)
	MCFG_I8275_CHARACTER_WIDTH(6)
	MCFG_I8275_DRAW_CHARACTER_CALLBACK_OWNER(unior_state, display_pixels)
	MCFG_I8275_DRQ_CALLBACK(DEVWRITELINE("dma",i8257_device, dreq2_w))
	MCFG_VIDEO_SET_SCREEN("screen")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( unior )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "unior.rom", 0xf800, 0x0800, CRC(23a347e8) SHA1(2ef3134e2f4a696c3b52a145fa5a2d4c3487194b))

	ROM_REGION( 0x0840, "chargen", ROMREGION_ERASEFF )
	ROM_LOAD( "unior.fnt",   0x0000, 0x0800, CRC(4f654828) SHA1(8c0ac11ea9679a439587952e4908940b67c4105e))
	// according to schematic this should be 256 bytes
	ROM_LOAD( "palette.rom", 0x0800, 0x0040, CRC(b4574ceb) SHA1(f7a82c61ab137de8f6a99b0c5acf3ac79291f26a))

	ROM_REGION( 0x0800, "vram", ROMREGION_ERASEFF )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT    COMPAT   MACHINE    INPUT  CLASS           INIT    COMPANY      FULLNAME       FLAGS */
COMP( 19??, unior,  radio86,  0,       unior,     unior, driver_device,   0,    "<unknown>",   "Unior", MACHINE_NOT_WORKING )
