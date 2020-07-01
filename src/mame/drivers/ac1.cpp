// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        AC1 video driver by Miodrag Milanovic

        15/01/2009 Preliminary driver.

        24/02/2011 Added cassette support ('ac1' and 'ac1_32' only)

        Note that Z command will get you into BASIC, and BYE command
        takes you back to the Monitor.

        S xxxx yyyy = to save memory to tape.
        L  = to load it back in.

        Since there is no motor control, type in L then mount the tape.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "machine/z80pio.h"
#include "machine/z80ctc.h"
#include "screen.h"
#include "speaker.h"
#include "sound/spkrdev.h"
#include "emupal.h"

class ac1_state : public driver_device
{
public:
	ac1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cassette(*this, "cassette")
		, m_maincpu(*this, "maincpu")
		, m_vram(*this, "videoram")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_speaker(*this, "speaker")
		, m_io_line(*this, "LINE.%u", 0U)
	{ }

	void ac1(machine_config &config);
	void ac1_32(machine_config &config);
	void ac1scch(machine_config &config);

private:
	u32 screen_update_ac1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_ac1_32(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u8 ac1_port_b_r();
	u8 ac1_port_a_r();
	void ac1_port_a_w(u8 data);
	void ac1_port_b_w(u8 data);

	void ac1_32_mem(address_map &map);
	void ac1_io(address_map &map);
	void ac1scch_io(address_map &map);
	void ac1_mem(address_map &map);

	required_device<cassette_image_device> m_cassette;
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<u8> m_vram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<speaker_sound_device> m_speaker;
	required_ioport_array<7> m_io_line;
};


u8 ac1_state::ac1_port_b_r()
{
	u8 data = 0x7f;

	if (m_cassette->input() > 0.03)
		data |= 0x80;

	return data;
}

#define BNOT(x) ((x) ? 0 : 1)

u8 ac1_state::ac1_port_a_r()
{
	u8 line0 = m_io_line[0]->read();
	u8 line1 = m_io_line[1]->read();
	u8 line2 = m_io_line[2]->read();
	u8 line3 = m_io_line[3]->read();
	u8 line4 = m_io_line[4]->read();
	u8 line5 = m_io_line[5]->read();
	u8 line6 = m_io_line[6]->read();

	u8 SH    = BNOT(BIT(line6,0));
	u8 CTRL  = BNOT(BIT(line6,1));
	u8 SPACE = BIT(line6,2);
	u8 ENTER = BIT(line6,3);
	u8 BACK  = BIT(line6,4);

	u8 all = line0 | line1 | line2 | line3 | line4 | line5;
	u8 s1 = BNOT(BIT(all,0));u8 z1 = (line0 !=0) ? 0 : 1;
	u8 s2 = BNOT(BIT(all,1));u8 z2 = (line1 !=0) ? 0 : 1;
	u8 s3 = BNOT(BIT(all,2));u8 z3 = (line2 !=0) ? 0 : 1;
	u8 s4 = BNOT(BIT(all,3));u8 z4 = (line3 !=0) ? 0 : 1;
	u8 s5 = BNOT(BIT(all,4));u8 z5 = (line4 !=0) ? 0 : 1;
	u8 s6 = BNOT(BIT(all,5));u8 z6 = (line5 !=0) ? 0 : 1;
	u8 s7 = BNOT(BIT(all,6));
	u8 s8 = BNOT(BIT(all,7));
	u8 tast,td0,td1,td2,td3,td4,td5,td6,dg5;

	/* Additional double keys */
	if (SPACE) {
		z1 = 0; s1 = 0; SH = 0;
	}
	if (ENTER) {
		z4 = 0; s6 = 0; CTRL = 0;
	}
	if (BACK) {
		z4 = 0; s1 = 0; CTRL = 0;
	}

	tast = BNOT(s1 & s2 & s3 & s4 & s5 &s6 & s7 & s8);
	td0  = BNOT(s2 & s4 & s6 & s8);
	td1  = BNOT(s3 & s4 & s7 & s8);
	td2  = BNOT(s5 & s6 & s7 & s8);
	td3  = BNOT(z2 & z4 & z6);
	td4  = BNOT(BNOT(BNOT(z1 & z2) & SH) & z5 & z6);
	dg5  = BNOT(z3 & z4 & z5 & z6);
	td5  = BNOT(BNOT(dg5 & BNOT(SH)) & z1 & z2);
	td6  = (dg5 & CTRL);
	return td0 + (td1 << 1) +(td2 << 2) +(td3 << 3) +(td4 << 4) +(td5 << 5) +(td6 << 6) +(tast << 7);
}

void ac1_state::ac1_port_a_w(u8 data)
{
}

void ac1_state::ac1_port_b_w(u8 data)
{
	/*

	    bit     description

	    0
	    1       RTTY receive
	    2       RTTY transmit
	    3       RTTY PTT
	    4
	    5
	    6       cassette out
	    7       cassette in

	*/
	m_speaker->level_w(BIT(data, 0));
	m_cassette->output(BIT(data, 6) ? -1.0 : +1.0);
}

/* Address maps */
void ac1_state::ac1_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).rom();  // Monitor
	map(0x0800, 0x0fff).rom();  // BASIC
	map(0x1000, 0x17ff).ram().share("videoram");
	map(0x1800, 0x1fff).ram();
}

void ac1_state::ac1_32_mem(address_map &map)
{
	ac1_mem(map);
	map(0x1800, 0xffff).ram();
}

void ac1_state::ac1_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));  // on the board, but ignored by standard bios
	map(0x04, 0x07).rw("pio", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
}

void ac1_state::ac1scch_io(address_map &map)
{
	map.global_mask(0xff);
	ac1_io(map);
	map(0x08, 0x0b).rw("pio2", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	// more...
}

/* Input ports */
static INPUT_PORTS_START( ac1 )
	PORT_START("LINE.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("LINE.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR('<') PORT_CHAR(',')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('=') PORT_CHAR('-')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('>') PORT_CHAR('.')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('?') PORT_CHAR('/')

	PORT_START("LINE.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')

	PORT_START("LINE.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')

	PORT_START("LINE.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')

	PORT_START("LINE.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl")  PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
INPUT_PORTS_END


const gfx_layout charlayout =
{
	6, 8,               /* 6x8 characters */
	256,                /* 256 characters */
	1,                  /* 1 bits per pixel */
	{0},                /* no bitplanes; 1 bit per pixel */
	{7, 6, 5, 4, 3, 2},
	{0 * 8, 1 * 8, 2 * 8, 3 * 8, 4 * 8, 5 * 8, 6 * 8, 7 * 8},
	8*8                 /* size of one char */
};

static GFXDECODE_START( gfx_ac1 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout, 0, 1 )
GFXDECODE_END

u32 ac1_state::screen_update_ac1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (u8 y = 0; y < 16; y++ )
		for (u8 x = 0; x < 64; x++ )
			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect, m_vram[x + y*64], 0, 0, 0, 63*6-x*6, 15*8-y*8);

	return 0;
}

u32 ac1_state::screen_update_ac1_32(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (u8 y = 0; y < 32; y++ )
		for (u8 x = 0; x < 64; x++ )
			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect, m_vram[x + y*64], 0, 0, 0, 63*6-x*6, 31*8-y*8);

	return 0;
}

void ac1_state::ac1(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(8'000'000) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &ac1_state::ac1_mem);
	m_maincpu->set_addrmap(AS_IO, &ac1_state::ac1_io);

	z80pio_device& pio(Z80PIO(config, "pio", XTAL(8'000'000)/4));
	pio.in_pa_callback().set(FUNC(ac1_state::ac1_port_a_r));
	pio.out_pa_callback().set(FUNC(ac1_state::ac1_port_a_w));
	pio.in_pb_callback().set(FUNC(ac1_state::ac1_port_b_r));
	pio.out_pb_callback().set(FUNC(ac1_state::ac1_port_b_w));

	Z80CTC(config, "ctc", 8'000'000 / 4); // all connections go external

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*6, 16*8);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(ac1_state::screen_update_ac1));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ac1);

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
}

void ac1_state::ac1_32(machine_config &config)
{
	ac1(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &ac1_state::ac1_32_mem);

	screen_device &screen(*subdevice<screen_device>("screen"));
	screen.set_size(64*6, 32*8);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(ac1_state::screen_update_ac1_32));
}

void ac1_state::ac1scch(machine_config &config)
{
	ac1_32(config);
	m_maincpu->set_addrmap(AS_IO, &ac1_state::ac1scch_io);

	Z80PIO(config, "pio2", XTAL(8'000'000)/4);  // for V24 and printer
	// more...
}


/* ROM definition */
ROM_START( ac1 )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v1", "Version 3.1 (orig)" )
	ROMX_LOAD("mon_v31_16.bin",  0x0000, 0x0800, CRC(1ba65e4d) SHA1(3382b8d03f31166a56aea49fd1ec1e82a7108300), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v2", "Version 3.1 (fixed)" )
	ROMX_LOAD("mon_v31_16_v2.bin",  0x0000, 0x0800, CRC(8904beb4) SHA1(db8d00a2537ac3a662e3c91e55eb2bf824a72062), ROM_BIOS(1))
	// from Funkamateur 01/85
	ROM_LOAD("minibasic.bin",   0x0800, 0x0800, CRC(06782639) SHA1(3fd57b3ae3f538374b0d794d8aa15d06bcaaddd8))

	ROM_REGION(0x0800, "gfx1",0)
	// 64 chars - U402 BM513
	ROM_LOAD("u402.bin", 0x0000, 0x0200, CRC(cfb67f28) SHA1(e3a62a3a8bce0d098887e31fd16410f38832fd18))
	ROM_COPY("gfx1", 0x0000, 0x0200, 0x0200)
	ROM_COPY("gfx1", 0x0000, 0x0400, 0x0200)
	ROM_COPY("gfx1", 0x0000, 0x0600, 0x0200)
ROM_END

ROM_START( ac1_32 )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("mon_v31_32.bin",  0x0000, 0x0800, CRC(bea78b1a) SHA1(8a3e2ac2033aa0bb016be742cfea7e4b09c0813b))
	// from Funkamateur 01/85
	ROM_LOAD("minibasic.bin",   0x0800, 0x0800, CRC(06782639) SHA1(3fd57b3ae3f538374b0d794d8aa15d06bcaaddd8))

	ROM_REGION(0x0800, "gfx1",0)
	ROM_SYSTEM_BIOS( 0, "128", "128 chars" )
	// 128 chars - U555 or 2708 from Funkamateur 06/86 128 including pseudo graphics
	ROMX_LOAD("zg_128.bin", 0x0000, 0x0400, CRC(0a6f7796) SHA1(64d77639b1ea23f45b4bd38c251851acb2d03822), ROM_BIOS(0))
	ROMX_LOAD("zg_128.bin", 0x0400, 0x0400, CRC(0a6f7796) SHA1(64d77639b1ea23f45b4bd38c251851acb2d03822), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "256", "256 chars" )
	// 256 chars - 2716 from Computerclub Dessau  including pseudo graphics
	ROMX_LOAD("zg_256.bin", 0x0000, 0x0800, CRC(b4171df5) SHA1(abdec4e00257f86b1a57e02b9c6b4d2df2a2a2db), ROM_BIOS(1))
ROM_END

ROM_START( ac1scch )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v7", "Version 7" )
	ROMX_LOAD("mon_v7.bin",  0x0000, 0x1000, CRC(fd17b0cf) SHA1(e47113025bd9dadc1522425e21703f43e584b00f), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v8", "Version 8" )
	ROMX_LOAD("mon_v8.bin",  0x0000, 0x1000, CRC(5af68da5) SHA1(e760d4400b9c937e7e789d52b8ec975ff253a122), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "v10", "Version 10" )
	ROMX_LOAD("mon_v10.bin",  0x0000, 0x1000, CRC(f8e67ecb) SHA1(7953676fc8c22824ceff464c7177e9ac0343b8ce), ROM_BIOS(2)) // not working
	ROM_SYSTEM_BIOS( 3, "v1088", "Version 10/88" )
	ROMX_LOAD("mon_v1088.bin",  0x0000, 0x1000, CRC(bbb0a6df) SHA1(de9389e142541a8b5ff238b59e98bf571c794bef), ROM_BIOS(3))

	ROM_REGION(0x0800, "gfx1",0)
	ROM_LOAD("zg_scch.bin", 0x0000, 0x0800, CRC(fbfaf5da) SHA1(667568c5909e9a17675cf09dfbce2fc090c420ab))
ROM_END

// Driver
//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT      COMPANY         FULLNAME                                 FLAGS
COMP( 1984, ac1,     0,      0,      ac1,     ac1,   ac1_state, empty_init, "Frank Heyder", "Amateurcomputer AC1 Berlin",            MACHINE_SUPPORTS_SAVE )
COMP( 1984, ac1_32,  ac1,    0,      ac1_32,  ac1,   ac1_state, empty_init, "Frank Heyder", "Amateurcomputer AC1 Berlin (32 lines)", MACHINE_SUPPORTS_SAVE )
COMP( 1984, ac1scch, ac1,    0,      ac1scch, ac1,   ac1_state, empty_init, "Frank Heyder", "Amateurcomputer AC1 SCCH", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
// ac1_2010
// ac1_2017

