// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

Partner driver by Miodrag Milanovic

2008-06-09 Preliminary driver.

Works well with cassette. Need to find out how to access the built-in
rom programs. Also need to find out how to access floppy disks, including
those in the softlist.

Cursor is one position too far to the right.

****************************************************************************/


#include "emu.h"
#include "partner.h"

#include "cpu/i8085/i8085.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "formats/rk_cas.h"
#include "formats/smx_dsk.h"


/* Address maps */
void partner_state::mem_map(address_map &map)
{
	map(0x0000, 0x07ff).bankrw("bank1");
	map(0x0800, 0x3fff).bankrw("bank2");
	map(0x4000, 0x5fff).bankrw("bank3");
	map(0x6000, 0x7fff).bankrw("bank4");
	map(0x8000, 0x9fff).bankrw("bank5");
	map(0xa000, 0xb7ff).bankrw("bank6");
	map(0xb800, 0xbfff).bankrw("bank7");
	map(0xc000, 0xc7ff).bankrw("bank8");
	map(0xc800, 0xcfff).bankrw("bank9");
	map(0xd000, 0xd7ff).bankrw("bank10");
	map(0xd800, 0xd8ff).rw("crtc", FUNC(i8275_device::read), FUNC(i8275_device::write));
	map(0xd900, 0xd9ff).rw(m_ppi1, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xda00, 0xdaff).w(FUNC(partner_state::mem_page_w));
	map(0xdb00, 0xdbff).w(m_dma, FUNC(i8257_device::write));
	map(0xdc00, 0xddff).bankrw("bank11");
	map(0xde00, 0xdeff).w(FUNC(partner_state::win_memory_page_w));
	map(0xe000, 0xe7ff).bankrw("bank12");
	map(0xe800, 0xffff).bankrw("bank13");
}

/* Input ports */
static INPUT_PORTS_START( partner )
	PORT_START("LINE0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC),27)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))

	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Back") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))

	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR(0xA4)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G')

	PORT_START("LINE5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O')

	PORT_START("LINE6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W')

	PORT_START("LINE7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('^')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("LINE8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)  // This is a toggle to switch between Latin and Russian Keyboard!
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_1) // This is the real shift
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Rus/Lat") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT) // doesn't do anything useful
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

/* Machine driver */
void partner_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_SMX_FORMAT);
}

/* F4 Character Displayer */
static const gfx_layout charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	512,                    /* 512 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_partner )
	GFXDECODE_ENTRY( "chargen", 0x0000, charlayout, 0, 1 )
GFXDECODE_END


void partner_state::partner(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, 16_MHz_XTAL / 9);
	m_maincpu->set_addrmap(AS_PROGRAM, &partner_state::mem_map);

	I8255(config, m_ppi1);
	m_ppi1->out_pa_callback().set(FUNC(partner_state::radio86_8255_porta_w2));
	m_ppi1->in_pb_callback().set(FUNC(partner_state::radio86_8255_portb_r2));
	m_ppi1->in_pc_callback().set(FUNC(partner_state::radio86_8255_portc_r2));
	m_ppi1->out_pc_callback().set(FUNC(partner_state::radio86_8255_portc_w2));

	auto &i8275(I8275(config, "crtc", 16_MHz_XTAL / 12));
	i8275.set_character_width(6);
	i8275.set_display_callback(FUNC(partner_state::display_pixels));
	i8275.drq_wr_callback().set(m_dma, FUNC(i8257_device::dreq2_w));

	/* video hardware */
	auto &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update("crtc", FUNC(i8275_device::screen_update));
	screen.set_refresh_hz(50);
	screen.set_size(78*6, 30*10);
	screen.set_visarea(0, 78*6-1, 0, 30*10-1);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_partner);
	PALETTE(config, m_palette, FUNC(partner_state::radio86_palette), 3);

	SPEAKER(config, "mono").front_center();

	I8257(config, m_dma, 16_MHz_XTAL / 9);
	m_dma->out_hrq_cb().set(FUNC(partner_state::hrq_w));
	m_dma->in_memr_cb().set(FUNC(partner_state::memory_read_byte));
	m_dma->out_memw_cb().set(FUNC(partner_state::memory_write_byte));
	m_dma->in_ior_cb<0>().set("fdc", FUNC(fd1793_device::data_r));
	m_dma->out_iow_cb<0>().set("fdc", FUNC(fd1793_device::data_w));
	m_dma->out_iow_cb<2>().set("crtc", FUNC(i8275_device::dack_w));
	m_dma->set_reverse_rw_mode(true);

	auto &cassette(CASSETTE(config, "cassette"));
	cassette.set_formats(rkp_cassette_formats);
	cassette.set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	cassette.add_route(ALL_OUTPUTS, "mono", 0.05);
	cassette.set_interface("partner_cass");

	SOFTWARE_LIST(config, "cass_list").set_original("partner_cass");

	FD1793(config, "fdc", 16_MHz_XTAL / 16);

	FLOPPY_CONNECTOR(config, "fd0", "525qd", FLOPPY_525_QD, true, floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fd1", "525qd", FLOPPY_525_QD, true, floppy_formats).enable_sound(true);

	SOFTWARE_LIST(config, "flop_list").set_original("partner_flop");

	/* internal ram */
	RAM(config, m_ram);
	m_ram->set_default_size("64K");
	m_ram->set_default_value(0x00);
}

/* ROM definition */
ROM_START( partner )
	ROM_REGION( 0xA000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "partner.rom", 0x0000, 0x2000, CRC(be1eaa10) SHA1(f9658d8055bf434240ec020d7892ea98cb5cbb76))
	ROM_LOAD( "basic.rom",   0x2000, 0x2000, CRC(1e9be0ec) SHA1(2c431f487cffddaac8413efddfc0527ad595f03b))
	ROM_LOAD( "mcpg.rom",    0x4000, 0x0800, CRC(3401225c) SHA1(6c252393ee73ed1a53d3e583547d86ab6718a533))
	ROM_LOAD( "fdd.rom",     0x6000, 0x0800, CRC(8ca350b5) SHA1(76fc92298726fb2840f4c19d7edc860d1ed86356))
	ROM_LOAD( "font.rom",    0x8000, 0x2000, CRC(dc4f1723) SHA1(6b8d5efb403cf0aeb3fd3197a0529d23c8e2f93c))

	ROM_REGION(0x2000, "chargen",0)
	ROM_LOAD ("partner.d25", 0x0000, 0x2000, CRC(2705f726) SHA1(3d7b33901ef098a405d7ddad924ba9677f6a9b15))
ROM_END

/* Driver */
//    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT    CLASS          INIT          COMPANY       FULLNAME         FLAGS
COMP( 1987, partner, radio86, 0,      partner, partner, partner_state, init_partner, "SAM SKB VM", "Partner-01.01", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
