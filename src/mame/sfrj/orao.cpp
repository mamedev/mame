// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

Orao driver by Miodrag Milanovic

2008-02-22 Preliminary driver.
2008-02-23 Sound support added.
2008-03-01 Updated to work with latest SVN code

Driver is based on work of Josip Perusanec

Ctrl-V turns on keyclick
Ctrl-S turns on reversed video

BC starts BASIC. orao103: EXIT to quit. orao: unknown how to quit.
To load use LMEM""

Todo:
- When pasting, shift key doesn't work

****************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "formats/orao_cas.h"

#include "sound/spkrdev.h"
#include "imagedev/cassette.h"

namespace {

class orao_state : public driver_device
{
public:
	orao_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_memory(*this, "memory")
		, m_vram(*this, "videoram")
		, m_maincpu(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_cassette(*this, "cassette")
		, m_io_keyboard(*this, "LINE.%d", 0)
	{ }

	void orao(machine_config &config);

	void init_orao();

private:
	u8 kbd_r(offs_t offset);
	void sound_w(offs_t offset, u8 data);
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	u32 screen_update_orao(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map) ATTR_COLD;

	required_shared_ptr<u8> m_memory;
	required_shared_ptr<u8> m_vram;
	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_ioport_array<20> m_io_keyboard;
	bool m_spr_bit = false;
};


/* Address maps */
void orao_state::mem_map(address_map &map)
{
	map(0x0000, 0x5fff).ram().share("memory");
	map(0x6000, 0x7fff).ram().share("videoram");
	map(0x8000, 0x87ff).r(FUNC(orao_state::kbd_r));
	map(0x8800, 0x8fff).w(FUNC(orao_state::sound_w));
	map(0xa000, 0xafff).ram();  // extension
	map(0xb000, 0xbfff).ram();  // DOS
	map(0xc000, 0xffff).rom().region("maincpu",0);
}


/* Input ports */
// bits 0-3 are masked out in the code
static INPUT_PORTS_START( orao )
	PORT_START("LINE.0")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("LINE.1")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CR") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0xC0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE.2")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))

	PORT_START("LINE.3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0xC0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE.4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')

	PORT_START("LINE.5")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0xC0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE.6")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("LINE.7")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0xC0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE.8")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("LINE.9")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0xC0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE.10")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M')

	PORT_START("LINE.11")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0xC0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE.12")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')

	PORT_START("LINE.13")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0xC0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE.14")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N')

	PORT_START("LINE.15")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0xC0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE.16")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(U'Č')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(U'Ć')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR(U'Ž')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR(':') PORT_CHAR('*')

	PORT_START("LINE.17")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('^') PORT_CHAR('@')
	PORT_BIT(0xC0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE.18")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(U'Đ')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(U'Š')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(';') PORT_CHAR('+')

	PORT_START("LINE.19")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0xC0, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

/* Driver initialization */
void orao_state::init_orao()
{
	memset(m_memory,0xff,0x6000);
}

void orao_state::machine_reset()
{
	m_spr_bit = 0;
}

void orao_state::machine_start()
{
	save_item(NAME(m_spr_bit));
}

u8 orao_state::kbd_r(offs_t offset)
{
	switch(offset)
	{
		/* Keyboard*/
		case 0x07FC : return m_io_keyboard[0]->read();
		case 0x07FD : return m_io_keyboard[1]->read();
		case 0x07FA : return m_io_keyboard[2]->read();
		case 0x07FB : return m_io_keyboard[3]->read();
		case 0x07F6 : return m_io_keyboard[4]->read();
		case 0x07F7 : return m_io_keyboard[5]->read();
		case 0x07EE : return m_io_keyboard[6]->read();
		case 0x07EF : return m_io_keyboard[7]->read();
		case 0x07DE : return m_io_keyboard[8]->read();
		case 0x07DF : return m_io_keyboard[9]->read();
		case 0x07BE : return m_io_keyboard[10]->read();
		case 0x07BF : return m_io_keyboard[11]->read();
		case 0x077E : return m_io_keyboard[12]->read();
		case 0x077F : return m_io_keyboard[13]->read();
		case 0x06FE : return m_io_keyboard[14]->read();
		case 0x06FF : return m_io_keyboard[15]->read();
		case 0x05FE : return m_io_keyboard[16]->read();
		case 0x05FF : return m_io_keyboard[17]->read();
		case 0x03FE : return m_io_keyboard[18]->read();
		case 0x03FF : return m_io_keyboard[19]->read();
		/* Tape */
		case 0x07FF : return (m_cassette->input() >= 0) ? 0xff : 0;
	}

	return 0xff;
}


void orao_state::sound_w(offs_t offset, u8 data)
{
	m_speaker->level_w(m_spr_bit);
	m_cassette->output(m_spr_bit ? 1.0 : -1.0);
	m_spr_bit ^= 1;
}

// bitmapped graphics
u32 orao_state::screen_update_orao(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 addr = 0;
	for (u16 y = 0; y < 256; y++)
	{
		int horpos = 0;
		for (u8 x = 0; x < 32; x++)
		{
			u8 code = m_vram[addr++];
			for (u8 b = 0; b < 8; b++)
			{
				bitmap.pix(y, horpos++) =  (code >> b) & 0x01;
			}
		}
	}
	return 0;
}

/* Machine driver */
void orao_state::orao(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 8_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &orao_state::mem_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 0, 256-1);
	screen.set_screen_update(FUNC(orao_state::screen_update_orao));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* audio hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(orao_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("orao_cass");

	SOFTWARE_LIST(config, "cass_list").set_original("orao");
}

/* ROM definition */
ROM_START( orao )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "bas12.ic5", 0x0000, 0x2000, CRC(42ae6f69) SHA1(b9d4a544fae13a9c492af027545178addd557111) )
	ROM_LOAD( "crt12.ic6", 0x2000, 0x2000, CRC(94ebdc94) SHA1(3959d717f96558823ccc806c842d2fb5ab0c3890) )
ROM_END

ROM_START( orao103 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "bas13.ic5", 0x0000, 0x2000, CRC(35daf5da) SHA1(499c5a4bd930c26ec6226623c2793b4c7f771658) )
	ROM_LOAD( "crt13.ic6", 0x2000, 0x2000, CRC(e7076014) SHA1(0e213287b0b520440af6a2a6297788a9356818c2) )
ROM_END

} // Anonymous namespace

/* Driver */
//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT          COMPANY         FULLNAME    FLAGS
COMP( 1984, orao,    0,      0,      orao,    orao,  orao_state, init_orao,    "PEL Varazdin", "Orao 102", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1985, orao103, orao,   0,      orao,    orao,  orao_state, init_orao,    "PEL Varazdin", "Orao 103", MACHINE_SUPPORTS_SAVE )
