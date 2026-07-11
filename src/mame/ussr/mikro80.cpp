// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************************

MIKRO80 driver by Miodrag Milanovic

2008-03-10 Preliminary driver.


Cassette:
* Mikro80: loads software items, but not its own saves
* Radio99: can load its own saves; can load radio86 tapes, can load ut88 tapes.
* Kristall2: can load its own saves; can load radio86 tapes; can load radio99 tapes.


TODO:
- Cassette - need schematic of CMT.

*****************************************************************************************/


#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "imagedev/cassette.h"
#include "machine/i8255.h"
#include "sound/dac.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "formats/rk_cas.h"


namespace {


class mikro80_state : public driver_device
{
public:
	mikro80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_aram(*this, "attrram")
		, m_vram(*this, "videoram")
		, m_ppi(*this, "ppi8255")
		, m_cassette(*this, "cassette")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_p_chargen(*this, "chargen")
		, m_io_keyboard(*this, "LINE%u", 0U)
		, m_dac(*this, "dac")
		, m_maincpu(*this, "maincpu")
	{ }

	void kristall(machine_config &config) ATTR_COLD;
	void radio99(machine_config &config) ATTR_COLD;
	void mikro80(machine_config &config) ATTR_COLD;

	void init_radio99() ATTR_COLD;
	void init_mikro80() ATTR_COLD;

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	u8 m_keyboard_mask = 0;
	u8 m_key_mask = 0;
	void sound_w(u8 data);
	u8 portb_r();
	u8 portc_r();
	u8 kristall2_portc_r();
	void porta_w(u8 data);
	void portc_w(u8 data);
	void tape_w(u8 data);
	u8 tape_r();
	u32 screen_update_mikro80(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void kristall_io(address_map &map) ATTR_COLD;
	void mikro80_io(address_map &map) ATTR_COLD;
	void mikro80_mem(address_map &map) ATTR_COLD;
	void radio99_io(address_map &map) ATTR_COLD;

	memory_passthrough_handler m_rom_shadow_tap;
	required_shared_ptr<uint8_t> m_aram;
	required_shared_ptr<uint8_t> m_vram;
	required_device<i8255_device> m_ppi;
	required_device<cassette_image_device> m_cassette;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
	required_region_ptr<u8> m_p_chargen;
	required_ioport_array<9> m_io_keyboard;
	optional_device<dac_bit_interface> m_dac;
	required_device<cpu_device> m_maincpu;
};


/* Driver initialization */
void mikro80_state::init_mikro80()
{
	m_key_mask = 0x7f;
}

void mikro80_state::init_radio99()
{
	m_key_mask = 0xff;
}

u8 mikro80_state::portb_r()
{
	u8 key = 0xff;
	for (u8 i = 0; i < 8; i++)
		if (BIT(m_keyboard_mask, i))
			key &= m_io_keyboard[i]->read();

	return key & m_key_mask;
}

u8 mikro80_state::portc_r()
{
	return m_io_keyboard[8]->read();
}

u8 mikro80_state::kristall2_portc_r()
{
	return (m_io_keyboard[8]->read() & 0xfe) | ((m_cassette->input() < 0.04) ? 1 : 0);
}

void mikro80_state::porta_w(u8 data)
{
	m_keyboard_mask = data ^ 0xff;
}

void mikro80_state::portc_w(u8 data)
{
	m_cassette->output(BIT(data, 7) ? 1.0 : -1.0);   // for Kristall2 only
}

void mikro80_state::machine_start()
{
	save_item(NAME(m_keyboard_mask));
	save_item(NAME(m_key_mask));
}

void mikro80_state::machine_reset()
{
	m_keyboard_mask = 0;

	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.install_rom(0x0000, 0x07ff, m_rom);   // do it here for F3
	m_rom_shadow_tap.remove();
	m_rom_shadow_tap = program.install_read_tap(
			0xf800, 0xffff,
			"rom_shadow_r",
			[this] (offs_t offset, u8 &data, u8 mem_mask)
			{
				if (!machine().side_effects_disabled())
				{
					// delete this tap
					m_rom_shadow_tap.remove();

					// reinstall RAM over the ROM shadow
					m_maincpu->space(AS_PROGRAM).install_ram(0x0000, 0x07ff, m_ram);
				}
			},
			&m_rom_shadow_tap);
}

void mikro80_state::tape_w(u8 data)
{
	// TODO: this is incorrect, to be fixed when the CMT schematic can be found
	m_cassette->output(BIT(data, 0) ? 1.0 : -1.0);
}


u8 mikro80_state::tape_r()
{
	return (m_cassette->input() < 0.04) ? 0xff : 0;
}

void mikro80_state::sound_w(u8 data)
{
	m_dac->write(BIT(data, 1));
}


/* Address maps */
void mikro80_state::mikro80_mem(address_map &map)
{
	map(0x0000, 0xdfff).ram().share("mainram");
	map(0xe000, 0xe7ff).ram().share("attrram");
	map(0xe800, 0xefff).ram().share("videoram");
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xffff).rom().region("maincpu",0);
}

void mikro80_state::mikro80_io(address_map &map)
{
	map.unmap_value_high();
	map(0x01, 0x01).rw(FUNC(mikro80_state::tape_r), FUNC(mikro80_state::tape_w));
	map(0x04, 0x07).lr8(NAME([this] (offs_t offset) { return m_ppi->read(offset^3); })).lw8(NAME([this] (offs_t offset, u8 data) { m_ppi->write(offset^3, data); }));
}

void mikro80_state::kristall_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x03).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x02, 0x02).w(FUNC(mikro80_state::portc_w));
	// map(0x20, 0x23).  init byte 8B, so possibly another ppi with reversed offset like mikro80
}

void mikro80_state::radio99_io(address_map &map)
{
	map.unmap_value_high();
	map(0x01, 0x01).rw(FUNC(mikro80_state::tape_r), FUNC(mikro80_state::tape_w));
	// no init byte, so ppi has been replaced by ordinary latches
	map(0x04, 0x04).w(FUNC(mikro80_state::sound_w));
	//map(0x05, 0x05).rw(FUNC(mikro80_state::portc_r), FUNC(mikro80_state::portc_w));
	map(0x05, 0x05).r(FUNC(mikro80_state::portc_r)).nopw();
	map(0x06, 0x06).r(FUNC(mikro80_state::portb_r));
	map(0x07, 0x07).w(FUNC(mikro80_state::porta_w));
}

/* Input ports */
static INPUT_PORTS_START( mikro80 )
	PORT_START("LINE0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('@')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('~')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("<>") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('_')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Rus/Lat") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

/* F4 Character Displayer */
static const gfx_layout charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_mikro80 )
	GFXDECODE_ENTRY( "chargen", 0x0000, charlayout, 0, 1 )
GFXDECODE_END

u32 mikro80_state::screen_update_mikro80(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 sy=0,ma=0;

	for (u8 y = 0; y < 32; y++)
	{
		for (u8 ra = 0; ra < 8; ra++)
		{
			u16 *p = &bitmap.pix(sy++);

			for (u16 x = ma; x < ma + 64; x++)
			{
				bool attr = BIT(m_aram[x+1], 7);
				u8 chr = m_vram[x];
				u8 gfx = m_p_chargen[(chr<<3) | ra ] ^ (attr ? 0xff : 0);

				/* Display a scanline of a character */
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
		ma+=64;
	}
	return 0;
}


void mikro80_state::mikro80(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, 2000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mikro80_state::mikro80_mem);
	m_maincpu->set_addrmap(AS_IO, &mikro80_state::mikro80_io);

	I8255(config, m_ppi);
	m_ppi->out_pa_callback().set(FUNC(mikro80_state::porta_w));
	m_ppi->in_pb_callback().set(FUNC(mikro80_state::portb_r));
	m_ppi->in_pc_callback().set(FUNC(mikro80_state::portc_r));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0, 64*8-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(mikro80_state::screen_update_mikro80));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_mikro80);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	SPEAKER(config, "speaker").front_center();

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(rk8_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "speaker", 0.05);
	m_cassette->set_interface("mikro80_cass");

	SOFTWARE_LIST(config, "cass_list").set_original("mikro80");
}

void mikro80_state::radio99(machine_config &config)
{
	mikro80(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &mikro80_state::radio99_io);

	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.50);
}

void mikro80_state::kristall(machine_config &config)
{
	mikro80(config);
	m_maincpu->set_addrmap(AS_IO, &mikro80_state::kristall_io);
	m_ppi->in_pc_callback().set(FUNC(mikro80_state::kristall2_portc_r));
}


/* ROM definition */

ROM_START( mikro80 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mikro80.rom", 0x0000, 0x0800, CRC(63a4b72a) SHA1(6bd3e396539a15e2ccffa7486cae06ef6ddd1d03))

	ROM_REGION(0x0800, "chargen",0)
	ROM_LOAD ("mikro80.fnt", 0x0000, 0x0800, CRC(43eb72bb) SHA1(761319cc6747661b33e84aa449cec83800543b5b) )
ROM_END

ROM_START( radio99 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "monrk88.bin", 0x0000, 0x0800, CRC(5415d847) SHA1(c8233c72548bc79846b9d998766a10df349c5bda))

	ROM_REGION(0x0800, "chargen",0)
	ROM_LOAD ("mikro80.fnt", 0x0000, 0x0800, CRC(43eb72bb) SHA1(761319cc6747661b33e84aa449cec83800543b5b) )
ROM_END

ROM_START( kristall2 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "kristall-2.rom", 0x0000, 0x0800, CRC(e1b5c60f) SHA1(8ce5158def7fca91ec7e11efbb10aa5d70b7c36d))

	ROM_REGION(0x0800, "chargen",0)
	ROM_LOAD( "kristall-2.fnt", 0x0000, 0x0800, CRC(9661c9f5) SHA1(830c38735dcb1c8a271fa0027f94b4e034848fc8))
ROM_END

} // anonymous namespace

/* Driver */
/*    YEAR  NAME       PARENT   COMPAT  MACHINE   INPUT    CLASS          INIT          COMPANY      FULLNAME       FLAGS */
COMP( 1983, mikro80,   0,       0,      mikro80,  mikro80, mikro80_state, init_mikro80, "<unknown>", "Mikro-80",    MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1993, radio99,   mikro80, 0,      radio99,  mikro80, mikro80_state, init_radio99, "<unknown>", "Radio-99DM",  MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1987, kristall2, mikro80, 0,      kristall, mikro80, mikro80_state, init_mikro80, "<unknown>", "Kristall-2",  MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
