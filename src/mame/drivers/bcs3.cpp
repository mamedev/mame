// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        BCS 3

    2009-05-12 Skeleton driver.
    2015-09-25 Improvements

    http://hc-ddr.hucki.net/wiki/doku.php/homecomputer:bcs3

    East German home built computer. No sound facilities.
    All documents are in German.

    Main CPU is a U880 (Z80 equivalent). Other ICs also have unusual names.

    The CTC sends an interrupt every so often. This uses a lookup table to
    jump to an address in the range 38xx-39xx. This seems to work much the
    same as reading video memory in the ZX80. This, I think, is to stop snow
    appearing on the screen. It also slows everything down noticeably.

    It appears that a read of 1400 activates the Z80's /WAIT pin. This will
    be released by a VS pulse. (Not emulated)

    Cassette is hooked up according to the documentation, but it doesn't work.

    Known Memory Map:
    0000 - 0FFF: Main ROM
    1000 - 13FF: Keyboard
    1400 - 17FF: /WAIT circuit
    1800 - 1BFF: Output one character's scanline to the monitor. ZX-video process.
    1C00 - 1FFF: Video RAM
    2000 - 3FFF: Mirror of 0000 - 1FFF.
    4000 - up  : Extra RAM (required for hack versions)

    Hack versions:
    - They are fitted with Basic 3.x, require more RAM, and use hardware scrolling.
    - No schematic has been found, so the code is educated guesswork.
    - The ZX process is still to be worked out. For now, we return 0xF7 to finish.
    - There is a machine-language monitor fitted to some models. To access:
    -- Y = USR(0F000H)
    -- Commands are S (substitute), M (move), T (test), G (go), Q (quit)

    To Do:
    - Need software
    - Fix cassette
    - Hack versions: fix the ZX process

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80ctc.h"
#include "imagedev/cassette.h"


class bcs3_state : public driver_device
{
public:
	bcs3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ctc(*this, "ctc")
		, m_p_chargen(*this, "chargen")
		, m_p_videoram(*this, "videoram")
		, m_cass(*this, "cassette")
		, m_io_keyboard(*this, "KEY")
	{ }

	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_READ8_MEMBER(video_r);
	DECLARE_READ8_MEMBER(zx_r);
	DECLARE_WRITE_LINE_MEMBER(ctc_z0_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_z1_w);
	DECLARE_DRIVER_INIT(bcs3a);
	DECLARE_DRIVER_INIT(bcs3b);
	DECLARE_DRIVER_INIT(bcs3c);
	DECLARE_DRIVER_INIT(bcs3d);
	UINT32 screen_update_bcs3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_bcs3a(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

private:
	bool m_cass_bit;
	UINT8 s_curs;
	UINT8 s_init;
	UINT8 s_rows;
	UINT8 s_cols;

	required_device<cpu_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_memory_region m_p_chargen;
	required_shared_ptr<UINT8> m_p_videoram;
	required_device<cassette_image_device> m_cass;
	required_ioport_array<10> m_io_keyboard;
};

READ8_MEMBER( bcs3_state::keyboard_r )
{
	UINT8 i, data = 0;

	if (offset == 0)
		data = (m_cass->input() > +0.01) ? 0x80 : 0;

	offset ^= 0x3ff;

	for (i = 0; i < 10; i++)
		if BIT(offset, i)
			data |= m_io_keyboard[i]->read();

	return data;
}

// 00-7F = NUL, 0xE0 = end of line.
READ8_MEMBER( bcs3_state::video_r )
{
	UINT8 data = m_p_videoram[offset];
	return BIT(data, 7) ? data : 0;
}

// Unsure of how this works.
// 00-7F = NUL, 0xFF = end of line, 0xF7 = finish.
READ8_MEMBER( bcs3_state::zx_r )
{
	return 0xf7;
}

static ADDRESS_MAP_START(bcs3_mem, AS_PROGRAM, 8, bcs3_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x0fff ) AM_MIRROR(0x2000) AM_ROM AM_REGION("roms", 0)
	AM_RANGE( 0x1000, 0x13ff ) AM_MIRROR(0x2000) AM_READ(keyboard_r)
	AM_RANGE( 0x1400, 0x17ff ) AM_MIRROR(0x2000) AM_NOP //  /WAIT circuit
	AM_RANGE( 0x1800, 0x1bff ) AM_MIRROR(0x2000) AM_READ(video_r)
	AM_RANGE( 0x1c00, 0x1fff ) AM_MIRROR(0x2000) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START(bcs3a_mem, AS_PROGRAM, 8, bcs3_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x0fff ) AM_MIRROR(0x2000) AM_ROM AM_REGION("roms", 0)
	AM_RANGE( 0x1000, 0x13ff ) AM_MIRROR(0x2000) AM_READ(keyboard_r)
	AM_RANGE( 0x1400, 0x17ff ) AM_MIRROR(0x2000) AM_NOP //  /WAIT circuit
	AM_RANGE( 0x1800, 0x1bff ) AM_MIRROR(0x2000) AM_READ(zx_r)
	AM_RANGE( 0x3c00, 0x7fff ) AM_RAM AM_SHARE("videoram")
	AM_RANGE( 0xf000, 0xf3ff ) AM_ROM AM_REGION("roms", 0x1000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( bcs3_io, AS_IO, 8, bcs3_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(3)
	// coded in the rom as F8 to FB
	AM_RANGE( 0x00, 0x03 ) AM_DEVREADWRITE("ctc", z80ctc_device, read, write)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( bcs3 )
	PORT_START("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K :") PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR(':')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A &") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')

	PORT_START("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L ;") PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR(';')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B '") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("KEY.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M <") PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('<')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C (") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2 \"") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR(34)

	PORT_START("KEY.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N =") PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('=')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D )") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')

	PORT_START("KEY.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O >") PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('>')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E *") PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')

	PORT_START("KEY.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F +") PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('+')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')

	PORT_START("KEY.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(32)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G ,") PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR(',')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6 &") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')

	PORT_START("KEY.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H -") PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('-')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7 '") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR(39)

	PORT_START("KEY.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I .") PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('.')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8 (") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')

	PORT_START("KEY.9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J /") PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('/')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9 )") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
INPUT_PORTS_END

// Official version
UINT32 bcs3_state::screen_update_bcs3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 y,ra,chr,gfx,rat;
	UINT16 sy=0,ma=0x50,x;

	for (y = 0; y < 12; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);
			rat = (ra + 1) & 7;

			for (x = ma; x < ma + 28; x++)
			{
				if (ra < 8)
				{
					chr = m_p_videoram[x] & 0x7f;

					/* get pattern of pixels for that character scanline */
					gfx = m_p_chargen->base()[(chr<<3) | rat ] ^ 0xff;
				}
				else
					gfx = 0xff;

				/* Display a scanline of a character (8 pixels) */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma+=28;
	}
	return 0;
}

/* Hacks: When it starts, it has 4 lines of data. Pressing enter causes it to allocate 100 lines.
   I'm assuming that it only shows a portion of this, with the cursor always in sight. */
UINT32 bcs3_state::screen_update_bcs3a(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 y,ra,chr,gfx,rat;
	UINT16 sy = 0, ma = s_init, x;
	UINT16 cursor = (m_p_videoram[s_curs] | (m_p_videoram[s_curs+1] << 8)) - 0x3c00 - ma;  // get cursor relative position
	rat = cursor / (s_cols+1);
	if (rat > (s_rows-1)) ma += (rat-(s_rows-1)) * (s_cols+1);

	for (y = 0; y < s_rows; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);
			rat = (ra + 1) & 7;

			for (x = ma; x < ma + s_cols; x++)
			{
				if (ra < 8)
				{
					chr = m_p_videoram[x] & 0x7f;

					/* get pattern of pixels for that character scanline */
					gfx = m_p_chargen->base()[(chr<<3) | rat ] ^ 0xff;
				}
				else
					gfx = 0xff;

				/* Display a scanline of a character (8 pixels) */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma+=(s_cols+1);
	}
	return 0;
}


/* F4 Character Displayer */
static const gfx_layout bcs3_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 0*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( bcs3 )
	GFXDECODE_ENTRY( "chargen", 0x0000, bcs3_charlayout, 0, 1 )
GFXDECODE_END

WRITE_LINE_MEMBER( bcs3_state::ctc_z0_w )
{
	m_ctc->trg1(state);
	if (state)
	{
		m_cass_bit ^= 1;
		m_cass->output(m_cass_bit ? -1.0 : +1.0);
	}
}

WRITE_LINE_MEMBER( bcs3_state::ctc_z1_w )
{
	m_ctc->trg2(state);
}

static const z80_daisy_config daisy_chain_intf[] =
{
	{ "ctc" },
	{ nullptr }
};

DRIVER_INIT_MEMBER( bcs3_state, bcs3a )
{
	s_curs = 0x7a;
	s_init = 0x80;
	s_rows = 12;
	s_cols = 29;
}

DRIVER_INIT_MEMBER( bcs3_state, bcs3b )
{
	s_curs = 0x7a;
	s_init = 0x80;
	s_rows = 24;
	s_cols = 40;
}

DRIVER_INIT_MEMBER( bcs3_state, bcs3c )
{
	s_curs = 0x08;
	s_init = 0xa0;
	s_rows = 12;
	s_cols = 29;
}

DRIVER_INIT_MEMBER( bcs3_state, bcs3d )
{
	s_curs = 0x08;
	s_init = 0xb4;
	s_rows = 12;
	s_cols = 29;
}

static MACHINE_CONFIG_START( bcs3, bcs3_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_5MHz /2)
	MCFG_CPU_PROGRAM_MAP(bcs3_mem)
	MCFG_CPU_IO_MAP(bcs3_io)
	MCFG_CPU_CONFIG(daisy_chain_intf)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(28*8, 12*10)
	MCFG_SCREEN_VISIBLE_AREA(0,28*8-1,0,12*10-1)
	MCFG_SCREEN_UPDATE_DRIVER(bcs3_state, screen_update_bcs3)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", bcs3)
	MCFG_PALETTE_ADD_MONOCHROME("palette")

	MCFG_DEVICE_ADD("ctc", Z80CTC, XTAL_5MHz / 2)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(WRITELINE(bcs3_state, ctc_z0_w))
	MCFG_Z80CTC_ZC1_CB(WRITELINE(bcs3_state, ctc_z1_w))

	MCFG_CASSETTE_ADD( "cassette" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( bcs3a, bcs3_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_7MHz /2)
	MCFG_CPU_PROGRAM_MAP(bcs3a_mem)
	MCFG_CPU_IO_MAP(bcs3_io)
	MCFG_CPU_CONFIG(daisy_chain_intf)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(29*8, 12*10)
	MCFG_SCREEN_VISIBLE_AREA(0,29*8-1,0,12*10-1)
	MCFG_SCREEN_UPDATE_DRIVER(bcs3_state, screen_update_bcs3a)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", bcs3)
	MCFG_PALETTE_ADD_MONOCHROME("palette")

	MCFG_DEVICE_ADD("ctc", Z80CTC, XTAL_7MHz / 2)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(WRITELINE(bcs3_state, ctc_z0_w))
	MCFG_Z80CTC_ZC1_CB(WRITELINE(bcs3_state, ctc_z1_w))

	MCFG_CASSETTE_ADD( "cassette" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( bcs3b, bcs3a )
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_SIZE(40*8, 24*10)
	MCFG_SCREEN_VISIBLE_AREA(0,40*8-1,0,24*10-1)
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( bcs3 )
	ROM_REGION( 0x2000, "roms", ROMREGION_ERASEFF )
	//ROM_LOAD( "se24.bin", 0x0000, 0x0800, CRC(268de5ee) SHA1(78784945956c1b0282a4e82ad55e7c3a77389e50))
	ROM_LOAD( "se24_000.d6", 0x0000, 0x0400, CRC(157a0d28) SHA1(0a6666c289b95d98128fd282478dff6319031b6e) )
	ROM_LOAD( "se24_400.d7", 0x0400, 0x0400, CRC(2159de0f) SHA1(09b567e750931019de914f25d5ab1e4910465de6) )

	ROM_REGION( 0x0400, "chargen", 0 )
	ROM_LOAD( "bcs_zg_24.d21", 0x0000, 0x0400, CRC(eaed9d84) SHA1(7023a6187cd6bd0c6489d76ff662453f14e5b636))
ROM_END

ROM_START( bcs3a )
	ROM_REGION( 0x2000, "roms", ROMREGION_ERASEFF )
	//ROM_LOAD( "se31_29.bin", 0x0000, 0x1000, CRC(e9b55544) SHA1(82bae68c4bcaecf66632f5b43913b50a1acba316))
	ROM_LOAD( "se31_000.d6", 0x0000, 0x0400, CRC(0765bd83) SHA1(137ceffd50eeaf21caab286d3e01161ba3267ea4) )
	ROM_LOAD( "se31_400.d7", 0x0400, 0x0400, CRC(1a87a3ed) SHA1(c8121ff198f8cf0c7bc7bc7e258ecfa51d3bb02c) )
	ROM_LOAD( "se31_800.d8", 0x0800, 0x0400, CRC(05654a8f) SHA1(b42fa6cf5710dab23f062dbeea81e85b4c18e1b0) )
	ROM_LOAD( "se31_c00.d9", 0x0c00, 0x0400, CRC(858ca28b) SHA1(90f943b0c1d102dd058a859ba139057a0bd278a6) )
	ROM_LOAD( "se31mceditor.bin", 0x1000, 0x0400, CRC(8eac92ec) SHA1(8950a3ef05d02abf34269bfce002c46d273ce113))

	ROM_REGION( 0x0400, "chargen", 0 )
	ROM_LOAD( "bcs_zg_31.d21", 0x0000, 0x0400, CRC(a20c93c9) SHA1(b2be1c0d98b7ac05713349b099b392975968be1d))
ROM_END

ROM_START( bcs3b )
	ROM_REGION( 0x2000, "roms", ROMREGION_ERASEFF )
	ROM_LOAD( "se31_40.bin", 0x0000, 0x1000, CRC(4e993152) SHA1(6bb01ff5779627fa2eb2df432fffcfccc1e33231))
	ROM_LOAD( "se31mceditor.bin", 0x1000, 0x0400, CRC(8eac92ec) SHA1(8950a3ef05d02abf34269bfce002c46d273ce113))

	ROM_REGION( 0x0400, "chargen", 0 )
	ROM_LOAD( "bcs_zg_31.d21", 0x0000, 0x0400, CRC(a20c93c9) SHA1(b2be1c0d98b7ac05713349b099b392975968be1d))
ROM_END

ROM_START( bcs3c )
	ROM_REGION( 0x2000, "roms", ROMREGION_ERASEFF )
	ROM_LOAD( "bcs32.bin", 0x0000, 0x1000, CRC(1523b846) SHA1(ca5e3213707a604e02d9e7a7ebfc362ef294ddb8) )

	ROM_REGION( 0x0400, "chargen", 0 )
	ROM_LOAD( "bcs_zg_32.d21", 0x0000, 0x0400, CRC(abe9e820) SHA1(03a8792d08774cd67b98efd3f83a78e897b4e001) )
ROM_END

ROM_START( bcs3d )
	ROM_REGION( 0x2000, "roms", ROMREGION_ERASEFF )
	ROM_LOAD( "sp33_29.bin", 0x0000, 0x1000, CRC(1c851eb2) SHA1(4f8bb5274ea1861a35a840e8f3482bdc693047c4))

	ROM_REGION( 0x0400, "chargen", 0 )
	ROM_LOAD( "bcs_zg_33.d21", 0x0000, 0x0400, CRC(b27f1c07) SHA1(61c80c585f198370ba5e856839c12b15acdc58ee))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT  CLASS          INIT        COMPANY             FULLNAME       FLAGS */
COMP( 1984, bcs3,   0,       0,      bcs3,      bcs3,  driver_device,   0,     "Eckhard Schiller", "BCS 3 rev 2.4", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1986, bcs3a,  bcs3,    0,      bcs3a,     bcs3,  bcs3_state,  bcs3a,     "Eckhard Schiller", "BCS 3 rev 3.1 29-column", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1986, bcs3b,  bcs3,    0,      bcs3b,     bcs3,  bcs3_state,  bcs3b,     "Eckhard Schiller", "BCS 3 rev 3.1 40-column", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1986, bcs3c,  bcs3,    0,      bcs3a,     bcs3,  bcs3_state,  bcs3c,     "Eckhard Schiller", "BCS 3 rev 3.2", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1986, bcs3d,  bcs3,    0,      bcs3a,     bcs3,  bcs3_state,  bcs3d,     "Eckhard Schiller", "BCS 3 rev 3.3", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
