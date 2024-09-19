// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Mera-Elzab Meritum (Poland)

Meritum I was a basic unit with no expansion, expensive, and so very rare.
Meritum II had all the standard TRS80 expansions, but the i/o is different
and thus not compatible.
Meritum III hires graphics mode added; only pre-production units were made.

Split from trs80.cpp on 2018-07-15

It is quite similar to the TRS80 Model 1 Level II, however instead of the
external interface, Intel peripheral chips were used (i8251, i8253, i8255),
and 2KB of ROM with new subroutines.

Model II has an additional 8255 to act as a floppy interface, plus it has more
RAM (32 or 48 KB).

Many variants of ROM existed:
- initial one without Polish characters
- with Polish characters in char table
- translated MEMORY SIZE to ROZMIAR PAO
- experimental network version made by Silesian University of Technology
- 8 KiB version with bootstrap code to load program from floppy
and others.

There's no lowercase, so the shift key will select Polish characters if
available, as well as the usual standard punctuation.
The control key appears to do nothing.
There's also a Reset key, a NMI key, and 2 blank ones.

Serial printer (@1200bps) is supported by default after power-up by all variants.
If parallel printer (centronics) is to be used press P while system starts (e.g. P and F3 to reset).
This does not work for model 1 (no parallel printer support in ROM).

Status:
- Starts up, runs Basic. Cassette works. Quickload mostly works.
- Some quickloads have corrupt text due to no lowercase.
- Some quickloads don't run at all.
- Intel chips need adding, along with the peripherals they control.
- A speaker has been included (which works), but real machine might not have
    one at that address. To be checked.
- On meritum1, type SYSTEM then /12288 to enter the Monitor.
- On meritum_net, type NET to activate the networking features.
- Add Reset key and 2 blank keys.
- Need software specific to test the hardware.
- Need boot disks (MER-DOS, CP/M 2.2)
- Due to faster CPU clock, no TRS-80 cassettes can be loaded.

For Model III:
- Add 4-colour mode, 4 shades of grey mode, and 512x192 monochrome.

****************************************************************************/

#include "emu.h"
#include "trs80.h"

#include "trs80_quik.h"

#include "bus/centronics/ctronics.h"
#include "machine/input_merger.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"

#include "emupal.h"
#include "softlist_dev.h"

#include "utf8.h"


namespace {

class meritum_state : public trs80_state
{
public:
	meritum_state(const machine_config &mconfig, device_type type, const char *tag)
		: trs80_state(mconfig, type, tag)
		, m_screen(*this, "screen")
		, m_centronics(*this, "centronics")
		, m_nmigate(*this, "nmigate")
	{ }

	void meritum1(machine_config &config);
	void meritum2(machine_config &config);
	void meritumn(machine_config &config);

private:

	u32 screen_update_meritum1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_meritum2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void mem_map2(address_map &map) ATTR_COLD;
	void io_map2(address_map &map) ATTR_COLD;
	void mainppi_portb_w(u8);
	void mainppi_portc_w(u8);

	required_device<screen_device> m_screen;
	optional_device<centronics_device> m_centronics;
	required_device<input_merger_device> m_nmigate;
};

void meritum_state::mainppi_portc_w(u8 data)
{
	m_nmigate->in_w<0>(!BIT(data, 7)); // negated PC7 => NMI
	m_centronics->write_strobe(BIT(data, 1)); // PC1 = STROBE (centronics)
}

void meritum_state::mainppi_portb_w(u8 data)
{
	m_centronics->write_data0(BIT(data, 0));
	m_centronics->write_data1(BIT(data, 1));
	m_centronics->write_data2(BIT(data, 2));
	m_centronics->write_data3(BIT(data, 3));
	m_centronics->write_data4(BIT(data, 4));
	m_centronics->write_data5(BIT(data, 5));
	m_centronics->write_data6(BIT(data, 6));
	m_centronics->write_data7(BIT(data, 7));
}

void meritum_state::mem_map(address_map &map)
{
	map(0x0000, 0x37ff).rom();
	map(0x3800, 0x38ff).r(FUNC(meritum_state::keyboard_r));
	map(0x3c00, 0x3fff).ram().share(m_p_videoram);
	map(0x4000, 0x7fff).ram();
}

void meritum_state::mem_map2(address_map &map)
{
	mem_map(map);
	map(0x4000, 0xffff).ram();
}

void meritum_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x03).rw("audiopit", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xf4, 0xf7).rw("mainppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xf8, 0xfb).rw("mainpit", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xfc, 0xfd).rw("usart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	// map(0xfe, 0xfe) audio interface
	map(0xff, 0xff).rw(FUNC(meritum_state::port_ff_r), FUNC(meritum_state::port_ff_w));
}

void meritum_state::io_map2(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	io_map(map);
	map(0xf0, 0xf3).rw("flopppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
}


static INPUT_PORTS_START( meritum )
	PORT_START("LINE0")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR('@')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('A', 'a') PORT_CHAR(0x0104, 0x0105)
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('B', 'b')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_CHAR('C', 'c') PORT_CHAR(0x0106, 0x0107)
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_CHAR('D', 'd')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('E', 'e') PORT_CHAR(0x0118, 0x0119)
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_CHAR('F', 'f')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_CHAR('G', 'g')

	PORT_START("LINE1")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_CHAR('H', 'h')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('I', 'i')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_CHAR('J', 'j')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_CHAR('K', 'k')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_CHAR('L', 'l') PORT_CHAR(0x0141, 0x0142)
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('M', 'm')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_CHAR('N', 'n') PORT_CHAR(0x0143, 0x0144)
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('O', 'o') PORT_CHAR(0x00d3, 0x00f3)

	PORT_START("LINE2")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('P', 'p')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('Q', 'q')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('R', 'r')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_CHAR('S', 's') PORT_CHAR(0x015a, 0x015b)
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('T', 't')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('U', 'u')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_CHAR('V', 'v')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('W', 'w')

	PORT_START("LINE3")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('X', 'x') PORT_CHAR(0x0179, 0x017a)
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('Y', 'y')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('Z', 'z') PORT_CHAR(0x017b, 0x017c)
	PORT_BIT(0xF8, 0x00, IPT_UNUSED)

	PORT_START("LINE4")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("LINE5")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("LINE6")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER)     PORT_CHAR(13)
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("Clear") PORT_CODE(KEYCODE_HOME)      PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_END)       PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_TAB)       PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP), '[')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_OPENBRACE)  PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT), 8)
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_QUOTE)     PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT), 9)
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)     PORT_CHAR(' ')

	PORT_START("LINE7")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)               PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0xFC, 0x00, IPT_UNUSED)

	PORT_START("NMI")
	PORT_BIT(0x01, 0x01, IPT_KEYBOARD) PORT_NAME("NMI") PORT_CODE(KEYCODE_F1) PORT_WRITE_LINE_DEVICE_MEMBER("nmigate", input_merger_device, in_w<1>)
INPUT_PORTS_END

u32 meritum_state::screen_update_meritum1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// lores characters are in the character generator. Each character is 6x12 (basic characters are 6x7 excluding descenders/ascenders).
	u16 sy=0,ma=0;
	const u8 cols = m_cpl ? 32 : 64;
	const u8 skip = m_cpl ? 2 : 1;

	if (cols != m_cols)
	{
		m_cols = cols;
		screen.set_visible_area(0, cols*6-1, 0, 16*12-1);
	}

	for (u8 y = 0; y < 16; y++)
	{
		for (u8 ra = 0; ra < 12; ra++)
		{
			u16 *p = &bitmap.pix(sy++);

			for (u16 x = ma; x < ma + 64; x+=skip)
			{
				const u8 chr = m_p_videoram[x] & 0xbf;
				u8 gfx;

				// shift down comma and semicolon
				// not sure Meritum I got the circuit for this (like TRS80)
				// but position of ';' suggests most likely yes
				if ((chr == 0x2c) && (ra >= 2))
					gfx = m_p_chargen[0x2be + ra];
				else if ((chr == 0x3b) && (ra >= 1))
					gfx = m_p_chargen[0x3af + ra];
				else
					gfx = m_p_chargen[(chr<<4) | ra];

				// Display a scanline of a character (6 pixels)
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma+=64;
	}
	return 0;
}

u32 meritum_state::screen_update_meritum2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 sy=0,ma=0;
	const u8 cols = m_cpl ? 32 : 64;
	const u8 skip = m_cpl ? 2 : 1;

	if (cols != m_cols)
	{
		m_cols = cols;
		screen.set_visible_area(0, cols*6-1, 0, 16*12-1);
	}

	for (u8 y = 0; y < 16; y++)
	{
		for (u8 ra = 0; ra < 12; ra++)
		{
			u16 *p = &bitmap.pix(sy++);

			for (u16 x = ma; x < ma + 64; x+=skip)
			{
				const u8 chr = m_p_videoram[x];

				// get pattern of pixels for that character scanline
				const u8 gfx = m_p_chargen[(chr<<4) | ra];

				// Display a scanline of a character (6 pixels)
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma+=64;
	}
	return 0;
}


/**************************** F4 CHARACTER DISPLAYER ***********************************************************/
static const gfx_layout charlayout =
{
	6, 12,          /* 6 x 12 characters */
	256,            /* 256 characters */
	1,          /* 1 bits per pixel */
	{ 0 },          /* no bitplanes */
	/* x offsets */
	{ 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{  0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	8*16           /* every char takes 16 bytes (unused scanlines are blank) */
};

static GFXDECODE_START(gfx_meritum)
	GFXDECODE_ENTRY( "chargen", 0, charlayout, 0, 1 )
GFXDECODE_END



void meritum_state::meritum1(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 10_MHz_XTAL / 4); // U880D @ 2.5 MHz or 1.67 MHz by jumper selection
	m_maincpu->set_addrmap(AS_PROGRAM, &meritum_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &meritum_state::io_map);

	i8251_device &usart(I8251(config, "usart", 10_MHz_XTAL / 4)); // same as CPU clock
	usart.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set("usart", FUNC(i8251_device::write_rxd));
	rs232.cts_handler().set("usart", FUNC(i8251_device::write_cts));

	INPUT_MERGER_ALL_HIGH(config, "nmigate").output_handler().set("mainpit", FUNC(pit8253_device::write_gate2)).invert();

	pit8253_device &pit(PIT8253(config, "mainpit", 0));
	pit.set_clk<0>(10_MHz_XTAL / 5); // 2 MHz
	pit.set_clk<1>(10_MHz_XTAL / 10); // 1 MHz
	pit.set_clk<2>(10_MHz_XTAL / 4); // same as CPU clock
	pit.out_handler<0>().set("usart", FUNC(i8251_device::write_txc));
	pit.out_handler<0>().append("usart", FUNC(i8251_device::write_rxc));
	// Channel 1 generates INT pulse through 123 monostable
	pit.out_handler<2>().set_inputline(m_maincpu, INPUT_LINE_NMI);

	i8255_device &mainppi(I8255(config, "mainppi")); // parallel interface
	mainppi.out_pc_callback().set(FUNC(meritum_state::mainppi_portc_w));
	mainppi.out_pb_callback().set(FUNC(meritum_state::mainppi_portb_w));

	// printer
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->set_data_input_buffer("cent_data_in");
	m_centronics->ack_handler().set("mainppi", FUNC(i8255_device::pc2_w));

	INPUT_BUFFER(config, "cent_data_in");
	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	PIT8253(config, "audiopit", 0); // optional audio interface

	// video
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(10_MHz_XTAL, 107 * 6, 0, 64 * 6, 312, 0, 192);
	m_screen->set_screen_update(FUNC(meritum_state::screen_update_meritum1));
	m_screen->set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_meritum);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	// sound
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	// media
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_PLAY);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	TRS80_QUICKLOAD(config, "quickload", m_maincpu, attotime::from_seconds(1));
	SOFTWARE_LIST(config, "quik_list").set_original("trs80_quik").set_filter("M1");
}

void meritum_state::meritum2(machine_config &config)
{
	meritum1(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &meritum_state::mem_map2);
	m_maincpu->set_addrmap(AS_IO, &meritum_state::io_map2);
	I8255(config, "flopppi", 0); // floppy disk interface
	m_screen->set_screen_update(FUNC(meritum_state::screen_update_meritum2));
	SOFTWARE_LIST(config.replace(), "quik_list").set_original("trs80_quik").set_filter("M2");
}

void meritum_state::meritumn(machine_config &config)
{
	meritum2(config);
	SOFTWARE_LIST(config.replace(), "quik_list").set_original("trs80_quik").set_filter("MN");
}


/***************************************************************************

  Game driver(s)

***************************************************************************/


ROM_START( meritum1 )
	ROM_REGION(0x3800, "maincpu",0)
	ROM_LOAD( "rom_0.ic7",  0x0000, 0x0800, CRC(1ecf7205) SHA1(e91cedfe2ce7636d37d5b765e5bbc8168deaba77))
	ROM_LOAD( "rom_1.ic8",  0x0800, 0x0800, CRC(ac297d99) SHA1(ccf31d3f9d02c3b68a0ee3be4984424df0e83ab0))
	ROM_LOAD( "rom_2.ic9",  0x1000, 0x0800, CRC(a21d0d62) SHA1(6dfdf3806ed2b6502e09a1b6922f21494134cc05))
	ROM_LOAD( "rom_3.ic10", 0x1800, 0x0800, CRC(3a5ea239) SHA1(8c489670977892d7f2bfb098f5df0b4dfa8fbba6))
	ROM_LOAD( "rom_4.ic11", 0x2000, 0x0800, CRC(2ba025d7) SHA1(232efbe23c3f5c2c6655466ebc0a51cf3697be9b))
	ROM_LOAD( "rom_5.ic12", 0x2800, 0x0800, CRC(ed547445) SHA1(20102de89a3ee4a65366bc2d62be94da984a156b))
	ROM_LOAD( "rom_6.ic13", 0x3000, 0x0800, CRC(650c0f47) SHA1(05f67fed3c3f69ad210823460bacf40166cbf06e))

	ROM_REGION(0x1000, "chargen", ROMREGION_INVERT)
	ROM_LOAD( "char_gen.ic72", 0x0000, 0x0400, CRC(626fb8b1) SHA1(1274d14efad46e5397bd9952e1277ebee44e0491))
	ROM_CONTINUE( 0x0800, 0x0400)
	ROM_RELOAD(   0x0400, 0x0400)
	ROM_CONTINUE( 0x0c00, 0x0400)
ROM_END

ROM_START( meritum2)
	ROM_REGION(0x3800, "maincpu",0)
	ROM_LOAD( "01.ic7",  0x0000, 0x0800, CRC(ed705a47) SHA1(dae8b14eb2ddb2a8b4458215180ebc0fb781816a))
	ROM_LOAD( "02.ic8",  0x0800, 0x0800, CRC(ac297d99) SHA1(ccf31d3f9d02c3b68a0ee3be4984424df0e83ab0))
	ROM_LOAD( "03.ic9",  0x1000, 0x0800, CRC(a21d0d62) SHA1(6dfdf3806ed2b6502e09a1b6922f21494134cc05))
	ROM_LOAD( "04.ic10", 0x1800, 0x0800, CRC(3610bdda) SHA1(602f0ba1e1267f24620f993acac019ac6342a594))
	ROM_LOAD( "05.ic11", 0x2000, 0x0800, CRC(461fbf0d) SHA1(bd19187dd992168af43bd68055343d515f152624))
	ROM_LOAD( "06.ic12", 0x2800, 0x0800, CRC(ed547445) SHA1(20102de89a3ee4a65366bc2d62be94da984a156b))
	ROM_LOAD( "07.ic13", 0x3000, 0x0800, CRC(044b1459) SHA1(faace7353ffbef6587b1b9e7f8b312e0892e3427))

	ROM_REGION(0x1000, "chargen", ROMREGION_INVERT)
	ROM_LOAD( "chargen.ic72", 0x0000, 0x1000, CRC(3dfc6439) SHA1(6e45a27f68c3491c403b4eafe45a108f348dd2fd))
ROM_END

ROM_START( meritum_net )
	ROM_REGION(0x3800, "maincpu",0)
	ROM_LOAD( "01_447_m07_015m.ic7",  0x0000, 0x0800, CRC(6d30cb49) SHA1(558241340a84eebcbbf8d92540e028e9164b6f8a))
	ROM_LOAD( "02_440_m08_01.ic8",    0x0800, 0x0800, CRC(ac297d99) SHA1(ccf31d3f9d02c3b68a0ee3be4984424df0e83ab0))
	ROM_LOAD( "03_440_m09_015m.ic9",  0x1000, 0x0800, CRC(88e267da) SHA1(9cb8626801f8e969f35291de43c1b643c809a3c3))
	ROM_LOAD( "04_447_m10_015m.ic10", 0x1800, 0x0800, CRC(e51991e4) SHA1(a7d42436da1af405970f9f99ab34b6d9abd05adf))
	ROM_LOAD( "05_440_m11_02.ic11",   0x2000, 0x0800, CRC(461fbf0d) SHA1(bd19187dd992168af43bd68055343d515f152624))
	ROM_LOAD( "06_440_m12_01.ic12",   0x2800, 0x0800, CRC(ed547445) SHA1(20102de89a3ee4a65366bc2d62be94da984a156b))
	ROM_LOAD( "07_447_m13_015m.ic13", 0x3000, 0x0800, CRC(789f6964) SHA1(9b2231ca7ffd82bbca1f53988a7df833290ddbf2))

	ROM_REGION(0x1000, "chargen", ROMREGION_INVERT)
	ROM_LOAD( "char.ic72", 0x0000, 0x1000, CRC(2c09a5a7) SHA1(146891b3ddfc2de95e6a5371536394a657880054))
ROM_END

} // anonymous namespace


//    YEAR  NAME         PARENT    COMPAT    MACHINE   INPUT      CLASS          INIT             COMPANY              FULLNAME                FLAGS
COMP( 1983, meritum1,    0,        trs80l2,  meritum1, meritum,   meritum_state, empty_init,  "Mera-Elzab", "Meritum I (Model 1)",           MACHINE_SUPPORTS_SAVE )
COMP( 1985, meritum2,    meritum1, 0,        meritum2, meritum,   meritum_state, empty_init,  "Mera-Elzab", "Meritum I (Model 2)",           MACHINE_SUPPORTS_SAVE )
COMP( 1985, meritum_net, meritum1, 0,        meritumn, meritum,   meritum_state, empty_init,  "Mera-Elzab", "Meritum I (Model 2) (network)", MACHINE_SUPPORTS_SAVE )
