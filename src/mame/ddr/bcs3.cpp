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

System Clock
    - is not a crystal, but instead an LC oscillator at 5 MHz
    - frequency divided by 2 to form the CPU clock.
    - bcs3b increased oscillator frequency to 7 MHz to handle 40 characters per line

CTC channels
    - 0, 15625Hz, TV line sync
    - 1, 244Hz, scanline counter for character generator (976Hz on bcs3)
    - 2, 49Hz, TV frame sync
    - 3, 1Hz, not used
    During cassette save, ch 1 is disabled, which causes ch 2 and 3 to stop.
    Ch 0 will output pulses at a rate of 1220Hz while each bit is different from
    the last, or 2441Hz if bits are the same (1708 and 3417Hz for bcs3b).
    These pulses are fed to a flipflop to create a train of wide and narrow
    cycles, to be recorded onto the tape.

Cassette
    - is hooked up according to the documentation, but doesn't work.
    - on the real machine it overrides the video output, so the video will
      be lost during cassette operations.
    - does not exist on "bcs3". The commands SAVE and LOAD will attempt to
      use a non-existing rom at 0800, and crash the system.
    - When you start any cassette-based system it asks for the number of lines
      you'd like on the screen, or hit enter for the default. Depending on the
      number, the BASIC program starts in a different place. So, to LOAD a tape,
      you must have the same number of lines that the program was SAVED at.
    - Recordings are not compatible between the versions of BASIC.

Bugs:
    - Tell it to PRINT ! (or any character it doesn't understand) and the screen
      fills up with zeroes. String are enclosed in single quotes. Double quotes
      will cause this bug.
    - Undefined strings print as 0 instead of nothing.

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
#include "machine/z80daisy.h"
#include "machine/z80ctc.h"
#include "imagedev/cassette.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class bcs3_state : public driver_device
{
public:
	bcs3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_ctc(*this, "ctc")
		, m_p_chargen(*this, "chargen")
		, m_p_videoram(*this, "videoram")
		, m_cass(*this, "cassette")
		, m_io_keyboard(*this, "KEY.%u", 0U)
	{ }

	void bcs3(machine_config &config);
	void bcs3a(machine_config &config);
	void bcs3b(machine_config &config);
	void init_bcs3a();
	void init_bcs3b();
	void init_bcs3c();
	void init_bcs3d();

private:
	u8 keyboard_r(offs_t offset);
	u8 video_r(offs_t offset);
	u8 zx_r();
	void ctc_z0_w(int state);
	void ctc_z1_w(int state);
	u32 screen_update_bcs3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_bcs3a(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void bcs3_io(address_map &map) ATTR_COLD;
	void bcs3_mem(address_map &map) ATTR_COLD;
	void bcs3a_mem(address_map &map) ATTR_COLD;
	void machine_start() override ATTR_COLD;
	bool m_cassbit = 0;
	u8 s_curs = 0U;
	u8 s_init = 0U;
	u8 s_rows = 0U;
	u8 s_cols = 0U;

	required_device<z80_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<z80ctc_device> m_ctc;
	required_region_ptr<u8> m_p_chargen;
	required_shared_ptr<u8> m_p_videoram;
	optional_device<cassette_image_device> m_cass;
	required_ioport_array<10> m_io_keyboard;
};

u8 bcs3_state::keyboard_r(offs_t offset)
{
	u8 data = 0;

	if (offset == 0 && m_cass)
		data = (m_cass->input() > +0.01) ? 0x80 : 0;

	offset ^= 0x3ff;

	for (u8 i = 0; i < 10; i++)
		if (BIT(offset, i))
			data |= m_io_keyboard[i]->read();

	return data;
}

// 00-7F = NUL, 0xE0 = end of line.
u8 bcs3_state::video_r(offs_t offset)
{
	u8 data = m_p_videoram[offset];
	return BIT(data, 7) ? data : 0;
}

// Unsure of how this works.
// 00-7F = NUL, 0xFF = end of line, 0xF7 = finish.
u8 bcs3_state::zx_r()
{
	return 0xf7;
}

void bcs3_state::bcs3_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).mirror(0x2000).rom().region("roms", 0);
	map(0x1000, 0x13ff).mirror(0x2000).r(FUNC(bcs3_state::keyboard_r));
	map(0x1400, 0x17ff).mirror(0x2000).noprw(); //  /WAIT circuit
	map(0x1800, 0x1bff).mirror(0x2000).r(FUNC(bcs3_state::video_r));
	map(0x1c00, 0x1fff).mirror(0x2000).ram().share("videoram");
}

void bcs3_state::bcs3a_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).mirror(0x2000).rom().region("roms", 0);
	map(0x1000, 0x13ff).mirror(0x2000).r(FUNC(bcs3_state::keyboard_r));
	map(0x1400, 0x17ff).mirror(0x2000).noprw(); //  /WAIT circuit
	map(0x1800, 0x1bff).mirror(0x2000).r(FUNC(bcs3_state::zx_r));
	map(0x3c00, 0x7fff).ram().share("videoram");
	map(0xf000, 0xf3ff).rom().region("roms", 0x1000);
}

void bcs3_state::bcs3_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(3);
	// coded in the rom as F8 to FB
	map(0x00, 0x03).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}

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
u32 bcs3_state::screen_update_bcs3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 sy=0,ma=0x50;

	for (u8 y = 0; y < 12; y++)
	{
		for (u8 ra = 0; ra < 10; ra++)
		{
			uint16_t *p = &bitmap.pix(sy++);
			u8 const rat = (ra + 1) & 7;

			for (u16 x = ma; x < ma + 28; x++)
			{
				u8 gfx;
				if (ra < 8)
				{
					u8 const chr = m_p_videoram[x] & 0x7f;

					/* get pattern of pixels for that character scanline */
					gfx = m_p_chargen[(chr<<3) | rat] ^ 0xff;
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
u32 bcs3_state::screen_update_bcs3a(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 sy = 0, ma = s_init;
	u16 const cursor = (m_p_videoram[s_curs] | (m_p_videoram[s_curs+1] << 8)) - 0x3c00 - ma;  // get cursor relative position
	u8 const cw = cursor / (s_cols+1);
	if (cw > (s_rows-1)) ma += (cw-(s_rows-1)) * (s_cols+1);

	for (u8 y = 0; y < s_rows; y++)
	{
		for (u8 ra = 0; ra < 10; ra++)
		{
			uint16_t *p = &bitmap.pix(sy++);
			u8 const rat = (ra + 1) & 7;

			for (u16 x = ma; x < ma + s_cols; x++)
			{
				u8 gfx;
				if (ra < 8)
				{
					u8 const chr = m_p_videoram[x] & 0x7f;

					/* get pattern of pixels for that character scanline */
					gfx = m_p_chargen[(chr<<3) | rat] ^ 0xff;
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

static GFXDECODE_START( gfx_bcs3 )
	GFXDECODE_ENTRY( "chargen", 0x0000, bcs3_charlayout, 0, 1 )
GFXDECODE_END

void bcs3_state::ctc_z0_w(int state)
{
	m_ctc->trg1(state);
	if (state && m_cass)
	{
		m_cassbit ^= 1;
		m_cass->output(m_cassbit ? -1.0 : +1.0);
	}
}

// The manual says the cassette pulses come from here, but
// it's total silence during cassette saving.
void bcs3_state::ctc_z1_w(int state)
{
	m_ctc->trg2(state);
}

void bcs3_state::machine_start()
{
	save_item(NAME(m_cassbit));
	save_item(NAME(s_curs));
	save_item(NAME(s_init));
	save_item(NAME(s_rows));
	save_item(NAME(s_cols));
}

static const z80_daisy_config daisy_chain_intf[] =
{
	{ "ctc" },
	{ nullptr }
};

void bcs3_state::init_bcs3a()
{
	s_curs = 0x7a;
	s_init = 0x80;
	s_rows = 12;
	s_cols = 29;
}

void bcs3_state::init_bcs3b()
{
	s_curs = 0x7a;
	s_init = 0x80;
	s_rows = 24;
	s_cols = 40;
}

void bcs3_state::init_bcs3c()
{
	s_curs = 0x08;
	s_init = 0xa0;
	s_rows = 12;
	s_cols = 29;
}

void bcs3_state::init_bcs3d()
{
	s_curs = 0x08;
	s_init = 0xb4;
	s_rows = 12;
	s_cols = 29;
}

void bcs3_state::bcs3(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(5'000'000) /2);
	m_maincpu->set_addrmap(AS_PROGRAM, &bcs3_state::bcs3_mem);
	m_maincpu->set_addrmap(AS_IO, &bcs3_state::bcs3_io);
	m_maincpu->set_daisy_config(daisy_chain_intf);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(28*8, 12*10);
	m_screen->set_visarea_full();
	m_screen->set_screen_update(FUNC(bcs3_state::screen_update_bcs3));
	m_screen->set_palette("palette");
	GFXDECODE(config, "gfxdecode", "palette", gfx_bcs3);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	Z80CTC(config, m_ctc, XTAL(5'000'000) / 2);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<0>().set(FUNC(bcs3_state::ctc_z0_w));
	m_ctc->zc_callback<1>().set(FUNC(bcs3_state::ctc_z1_w));
}

void bcs3_state::bcs3a(machine_config &config)
{
	bcs3(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &bcs3_state::bcs3a_mem);
	m_screen->set_size(29*8, 12*10);
	m_screen->set_visarea(0, 29*8-1, 0, 12*10-1);
	m_screen->set_screen_update(FUNC(bcs3_state::screen_update_bcs3a));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
}

void bcs3_state::bcs3b(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(7'000'000) /2);
	m_maincpu->set_addrmap(AS_PROGRAM, &bcs3_state::bcs3a_mem);
	m_maincpu->set_addrmap(AS_IO, &bcs3_state::bcs3_io);
	m_maincpu->set_daisy_config(daisy_chain_intf);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(40*8, 24*10);
	m_screen->set_visarea_full();
	m_screen->set_screen_update(FUNC(bcs3_state::screen_update_bcs3a));
	m_screen->set_palette("palette");
	GFXDECODE(config, "gfxdecode", "palette", gfx_bcs3);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	Z80CTC(config, m_ctc, XTAL(7'000'000) / 2);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<0>().set(FUNC(bcs3_state::ctc_z0_w));
	m_ctc->zc_callback<1>().set(FUNC(bcs3_state::ctc_z1_w));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
}


/* ROM definition */
ROM_START( bcs3 )
	ROM_REGION( 0x2000, "roms", ROMREGION_ERASEFF )
	//ROM_LOAD( "se24.bin", 0x0000, 0x0800, CRC(268de5ee) SHA1(78784945956c1b0282a4e82ad55e7c3a77389e50))
	ROM_LOAD( "se24_000.d6", 0x0000, 0x0400, CRC(157a0d28) SHA1(0a6666c289b95d98128fd282478dff6319031b6e) )
	ROM_LOAD( "se24_400.d7", 0x0400, 0x0400, CRC(2159de0f) SHA1(09b567e750931019de914f25d5ab1e4910465de6) )
	// Cassette rom goes here starting at 0x0800 ... but did this rom ever exist??

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

} // anonymous namespace


/* Driver */

/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY             FULLNAME                   FLAGS */
COMP( 1984, bcs3,  0,      0,      bcs3,    bcs3,  bcs3_state, empty_init, "Eckhard Schiller", "BCS 3 rev 2.4",           MACHINE_NO_SOUND_HW )
COMP( 1986, bcs3a, bcs3,   0,      bcs3a,   bcs3,  bcs3_state, init_bcs3a, "Eckhard Schiller", "BCS 3 rev 3.1 29-column", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1986, bcs3b, bcs3,   0,      bcs3b,   bcs3,  bcs3_state, init_bcs3b, "Eckhard Schiller", "BCS 3 rev 3.1 40-column", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1986, bcs3c, bcs3,   0,      bcs3a,   bcs3,  bcs3_state, init_bcs3c, "Eckhard Schiller", "BCS 3 rev 3.2",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1986, bcs3d, bcs3,   0,      bcs3a,   bcs3,  bcs3_state, init_bcs3d, "Eckhard Schiller", "BCS 3 rev 3.3",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
