// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/*********************************************************************************************************************************

Science of Cambridge MK-14

2009-11-20 Skeleton driver.
2016-08-21 Working

Keys:

UP: MEM increments the currently displayed address, (and goes into data entry mode in V1 bios).
= : TERM changes to "data entry" mode. In this mode, entering hex digits will change the byte at the currently displayed address
- : ABORT changes to "address entry" mode. In this mode, entering hex digits will change the address.
X : GO runs the program from the currently displayed address. On exit, the instruction after the program is displayed

Pasting:
        0-F : as is
        MEM : ^
        TERM: =
        AB :  -
        GO :  X

Example program: ("organ" from p82 of the manual)
-F20=C4^0D^35^C4^00^31^C4^08^C8^F6^C5^01^E4^FF^98^08^8F^00^06^E4^07^07^90^EB^B8^E6^9C^EE^90^E5^-F20X
Pressing keys will produce different tones.

*********************************************************************************************************************************/

#include "emu.h"

#include "cpu/scmp/scmp.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "machine/ins8154.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "mk14.lh"
#include "mk14vdu.lh"

namespace {

class mk14_state : public driver_device
{
public:
	mk14_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cass(*this, "cassette")
		, m_dac(*this, "dac")
		, m_display(*this, "display")
		, m_io_keyboard(*this, "X%u", 0U)
	{ }

	void mk14(machine_config &config);

protected:
	uint8_t keyboard_r(offs_t offset);
	void display_w(offs_t offset, uint8_t data);
	void port_a_w(uint8_t data);
	void cass_w(int state);
	int cass_r();
	void mk14_map(address_map &map) ATTR_COLD;

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

	required_device<scmp_device> m_maincpu;
	required_device<cassette_image_device> m_cass;
	required_device<dac_bit_interface> m_dac;
	required_device<pwm_display_device> m_display;
	required_ioport_array<8> m_io_keyboard;
};

class mk14vdu_state : public mk14_state
{
public:
	mk14vdu_state(const machine_config &mconfig, device_type type, const char *tag)
		: mk14_state(mconfig, type, tag)
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_chargen(*this, "chargen")
		, m_cfg_ps(*this, "CFG_PS")
		, m_cfg_vdu(*this, "CFG_VDU")
	{ }

	void mk14vdu(machine_config &config);

private:
	void mk14vdu_map(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_page_character(int page, uint16_t addr, int invert, bitmap_rgb32 &bitmap);
	void draw_page_graphics(int page, uint16_t addr, int invert, bitmap_rgb32 &bitmap);

	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_region_ptr<uint8_t> m_chargen;
	required_ioport m_cfg_ps;
	required_ioport m_cfg_vdu;
};

/*
000-1FF  512 byte SCIOS ROM  Decoded by 0xxx
200-3FF  ROM Shadow / Expansion RAM
400-5FF  ROM Shadow / Expansion RAM
600-7FF  ROM Shadow / Expansion RAM
800-87F  I/O Ports  Decoded by 1xx0
880-8FF  128 bytes I/O chip RAM  Decoded by 1xx0
900-9FF  Keyboard & Display  Decoded by 1x01
A00-AFF  I/O Port & RAM Shadow
B00-BFF  256 bytes RAM (Extended) / VDU RAM  Decoded by 1011
C00-CFF  I/O Port & RAM Shadow
D00-DFF  Keyboard & Display Shadow
E00-EFF  I/O Port & RAM Shadow
F00-FFF  256 bytes RAM (Standard) / VDU RAM  Decoded by 1111
*/


uint8_t mk14_state::keyboard_r(offs_t offset)
{
	if (offset < 8)
		return m_io_keyboard[offset]->read();
	else
		return 0xff;
}

void mk14_state::display_w(offs_t offset, uint8_t data)
{
	if (offset < 8 )
		m_display->matrix(1 << offset, data);
}

void mk14_state::mk14_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x0fff);
	map(0x000, 0x1ff).mirror(0x600).rom(); // ROM
	map(0x800, 0x87f).mirror(0x600).rw("ic8", FUNC(ins8154_device::read_io), FUNC(ins8154_device::write_io)); // I/O
	map(0x880, 0x8ff).mirror(0x600).rw("ic8", FUNC(ins8154_device::read_ram), FUNC(ins8154_device::write_ram)); // 128 bytes I/O chip RAM
	map(0x900, 0x9ff).mirror(0x400).rw(FUNC(mk14_state::keyboard_r), FUNC(mk14_state::display_w));
	map(0xb00, 0xbff).ram(); // VDU RAM
	map(0xf00, 0xfff).ram(); // Standard RAM
}

void mk14vdu_state::mk14vdu_map(address_map &map)
{
	mk14_map(map);
	map(0x200, 0x7ff).ram(); // Expansion RAM
}

/* Input ports */
static INPUT_PORTS_START( mk14 )
	PORT_START("X0")
		PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A")     PORT_CODE(KEYCODE_A)      PORT_CHAR('A')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8")     PORT_CODE(KEYCODE_8)      PORT_CHAR('8')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0")     PORT_CODE(KEYCODE_0)      PORT_CHAR('0')
	PORT_START("X1")
		PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B")     PORT_CODE(KEYCODE_B)      PORT_CHAR('B')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9")     PORT_CODE(KEYCODE_9)      PORT_CHAR('9')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1")     PORT_CODE(KEYCODE_1)      PORT_CHAR('1')
	PORT_START("X2")
		PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C")     PORT_CODE(KEYCODE_C)      PORT_CHAR('C')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("GO")    PORT_CODE(KEYCODE_X)      PORT_CHAR('X')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2")     PORT_CODE(KEYCODE_2)      PORT_CHAR('2')
	PORT_START("X3")
		PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D")     PORT_CODE(KEYCODE_D)      PORT_CHAR('D')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MEM")   PORT_CODE(KEYCODE_UP)     PORT_CHAR('^')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3")     PORT_CODE(KEYCODE_3)      PORT_CHAR('3')
	PORT_START("X4")
		PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ABORT") PORT_CODE(KEYCODE_MINUS)  PORT_CHAR('-')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4")     PORT_CODE(KEYCODE_4)      PORT_CHAR('4')
	PORT_START("X5")
		PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5")     PORT_CODE(KEYCODE_5)      PORT_CHAR('5')
	PORT_START("X6")
		PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E")     PORT_CODE(KEYCODE_E)      PORT_CHAR('E')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6")     PORT_CODE(KEYCODE_6)      PORT_CHAR('6')
	PORT_START("X7")
		PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F")     PORT_CODE(KEYCODE_F)      PORT_CHAR('F')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TERM")  PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7")     PORT_CODE(KEYCODE_7)      PORT_CHAR('7')
INPUT_PORTS_END

static INPUT_PORTS_START( mk14vdu )
	PORT_INCLUDE(mk14)

	PORT_START("CFG_PS")
		PORT_CONFNAME(0x0f, 0x02, "Page Select")
		PORT_CONFSETTING(0x00, "000")
		PORT_CONFSETTING(0x01, "100")
		PORT_CONFSETTING(0x02, "200")
		PORT_CONFSETTING(0x03, "300")
		PORT_CONFSETTING(0x04, "400")
		PORT_CONFSETTING(0x05, "500")
		PORT_CONFSETTING(0x06, "600")
		PORT_CONFSETTING(0x07, "700")
		PORT_CONFSETTING(0x08, "800")
		PORT_CONFSETTING(0x09, "900")
		PORT_CONFSETTING(0x0a, "A00")
		PORT_CONFSETTING(0x0b, "B00")
		PORT_CONFSETTING(0x0c, "C00")
		PORT_CONFSETTING(0x0d, "D00")
		PORT_CONFSETTING(0x0e, "E00")
		PORT_CONFSETTING(0x0f, "F00")
		PORT_CONFNAME(0xf0, 0x10, "Top Page > Page Select")
		PORT_CONFSETTING(0x00, "None")
		PORT_CONFSETTING(0x10, "PS1")
		PORT_CONFSETTING(0x20, "PS2")
		PORT_CONFSETTING(0x40, "PS3")
		PORT_CONFSETTING(0x80, "PS4")
	PORT_START("CFG_VDU")
		PORT_CONFNAME(0x01, 0x01, "Reverse Pages")
		PORT_CONFSETTING(0x00, DEF_STR( Yes ))
		PORT_CONFSETTING(0x01, DEF_STR( No ))
		PORT_CONFNAME(0x22, 0x00, "Video Mode")
		PORT_CONFSETTING(0x00, "Character (Low)" )
		PORT_CONFSETTING(0x02, "Graphics (High)" )
		PORT_CONFSETTING(0x20, "Mixed (Top Page)" )
		PORT_CONFNAME(0x44, 0x04, "Invert Video")
		PORT_CONFSETTING(0x00, "Inverted (Low)" )
		PORT_CONFSETTING(0x04, "Normal (High)")
		PORT_CONFSETTING(0x40, "Mixed (Top Page)" )
INPUT_PORTS_END

void mk14_state::port_a_w(uint8_t data)
{
}

void mk14_state::cass_w(int state)
{
	m_cass->output(state ? -1.0 : +1.0);
	m_dac->write(state);
}

int mk14_state::cass_r()
{
	return (m_cass->input() > 0.03) ? 1 : 0;
}

//-------------------------------------------------
//  gfx_layout acorn_vdu80_charlayout
//-------------------------------------------------

static const gfx_layout mk14vdu_charlayout =
{
	5, 7,                   /* 5 x 7 characters */
	64,                     /* 64 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0 * 8,  1 * 8,  2 * 8,  3 * 8,  4 * 8,  5 * 8,  6 * 8 },
	8 * 8                   /* every char takes 8 bytes */
};

//-------------------------------------------------
//  GFXDECODE( gfx_mk14vdu )
//-------------------------------------------------

static GFXDECODE_START(gfx_mk14vdu)
	GFXDECODE_ENTRY("chargen", 0, mk14vdu_charlayout, 0, 1)
GFXDECODE_END

uint32_t mk14vdu_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int page = 0; page < 2; page++)
	{
		// top page adjusted with reverse pages
		bool top_page = BIT(m_cfg_vdu->read(), 0) ? (page == 1) : (page == 0);

		// page select
		uint16_t addr = BIT(m_cfg_ps->read(), 0, 4) << 8;
		if (top_page)
			addr |= (BIT(m_cfg_ps->read(), 4, 4) << 8);

		// invert video
		int invert = BIT(m_cfg_vdu->read(), 6) ? !top_page : !BIT(m_cfg_vdu->read(), 2);

		// character/graphic mode
		if (BIT(m_cfg_vdu->read(), 5) ? top_page : BIT(m_cfg_vdu->read(), 1))
			draw_page_graphics(page, addr, invert, bitmap);
		else
			draw_page_character(page, addr, invert, bitmap);
	}
	return 0;
}

void mk14vdu_state::draw_page_character(int page, uint16_t addr, int invert, bitmap_rgb32 &bitmap)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	const pen_t *pens = m_palette->pens();

	for (int y = 0; y < 128; y++)
	{
		uint16_t videoram_addr = addr + ((y >> 3) * 16);

		int x = 0;

		for (int sx = 0; sx < 16; sx++)
		{
			uint8_t videoram_data = space.read_byte(videoram_addr++);
			uint16_t const charrom_addr = ((videoram_data & 0x3f) << 3) | (y % 8);
			uint8_t charrom_data = m_chargen[charrom_addr];

			for (int bit = 0; bit < 8; bit++)
			{
				// invert single character (implemented in some replica boards)
				int color = BIT(charrom_data, 6) ^ invert ^ BIT(videoram_data, 7);
				bitmap.pix(y + (page * 128), x++) = pens[color];
				charrom_data <<= 1;
			}
		}
	}
}

void mk14vdu_state::draw_page_graphics(int page, uint16_t addr, int invert, bitmap_rgb32 &bitmap)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	const pen_t *pens = m_palette->pens();

	for (int y = 0; y < 128; y++)
	{
		uint16_t videoram_addr = addr + ((y >> 2) * 8);

		int x = 0;

		for (int sx = 0; sx < 8; sx++)
		{
			uint8_t videoram_data = space.read_byte(videoram_addr++);

			for (int bit = 0; bit < 8; bit++)
			{
				int color = BIT(videoram_data, 7) ^ invert;
				bitmap.pix(y + (page * 128), x++) = pens[color];
				bitmap.pix(y + (page * 128), x++) = pens[color];
				videoram_data <<= 1;
			}
		}
	}
}

QUICKLOAD_LOAD_MEMBER(mk14_state::quickload_cb)
{
	if (image.software_entry() == nullptr)
		return std::make_pair(image_error::UNSUPPORTED, "Unsupported quickload format");

	uint16_t const size = image.length();
	int load_addr, exec_addr;
	sscanf(image.get_feature("load"), "%x", &load_addr);
	sscanf(image.get_feature("exec"), "%x", &exec_addr);

	for (uint16_t i = 0; i < size; i++)
	{
		uint8_t data;

		if (image.fread(&data, 1) != 1)
			return std::make_pair(image_error::UNSPECIFIED, std::string());
		m_maincpu->space(AS_PROGRAM).write_byte(load_addr + i, data);
	}

	m_maincpu->set_pc(exec_addr);

	return std::make_pair(std::error_condition(), std::string());
}

void mk14_state::mk14(machine_config &config)
{
	// IC1 1SP-8A/600 (8060) SC/MP Microprocessor
	INS8060(config, m_maincpu, 4.433619_MHz_XTAL / 2);
	m_maincpu->flag_out().set(FUNC(mk14_state::cass_w));
	m_maincpu->s_out().set_nop();
	m_maincpu->s_in().set(FUNC(mk14_state::cass_r));
	m_maincpu->sense_a().set_constant(0);
	m_maincpu->sense_b().set(FUNC(mk14_state::cass_r));
	m_maincpu->halt().set_nop();
	m_maincpu->set_addrmap(AS_PROGRAM, &mk14_state::mk14_map);

	config.set_default_layout(layout_mk14);
	PWM_DISPLAY(config, m_display).set_size(8, 8);
	m_display->set_segmask(0xff, 0xff);

	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.25);
	ZN425E(config, "dac8", 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // Ferranti ZN425E

	ins8154_device &ic8(INS8154(config, "ic8"));
	ic8.out_a().set(FUNC(mk14_state::port_a_w));
	ic8.out_b().set("dac8", FUNC(dac_byte_interface::data_w));

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "speaker", 0.05);

	quickload_image_device &quickload(QUICKLOAD(config, "quickload", "bin"));
	quickload.set_load_callback(FUNC(mk14_state::quickload_cb));
	quickload.set_interface("mk14_quik");

	SOFTWARE_LIST(config, "quik_ls").set_original("mk14_quik");
}

void mk14vdu_state::mk14vdu(machine_config &config)
{
	mk14(config);

	m_maincpu->set_clock(4_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &mk14vdu_state::mk14vdu_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(4_MHz_XTAL, 256, 0, 128, 312, 0, 256);
	m_screen->set_screen_update(FUNC(mk14vdu_state::screen_update));
	config.set_default_layout(layout_mk14vdu);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_mk14vdu);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	subdevice<software_list_device>("quik_ls")->set_filter("VDU");
}

/* ROM definition */
ROM_START( mk14 )
	ROM_REGION( 0x200, "maincpu", 0 )
	// IC2,3 74S571 512 x 4 bit ROM
	ROM_DEFAULT_BIOS("v2")
	ROM_SYSTEM_BIOS(0, "v2", "SCIOS V2")
	ROMX_LOAD( "scios_v2.bin", 0x0000, 0x0200, CRC(8b667daa) SHA1(802dc637ce5391a2a6627f76f919b12a869b56ef), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v1", "SCIOS V1")
	ROMX_LOAD( "scios_v1.bin", 0x0000, 0x0200, CRC(3d2477e7) SHA1(795829a2025e24d87a413e245d72a284f872e0db), ROM_BIOS(1))
ROM_END

ROM_START( mk14vdu )
	ROM_REGION( 0x200, "maincpu", 0 )
	// IC2,3 74S571 512 x 4 bit ROM
	ROM_DEFAULT_BIOS("v2")
	ROM_SYSTEM_BIOS(0, "v2", "SCIOS V2")
	ROMX_LOAD( "scios_v2.bin", 0x0000, 0x0200, CRC(8b667daa) SHA1(802dc637ce5391a2a6627f76f919b12a869b56ef), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v1", "SCIOS V1")
	ROMX_LOAD( "scios_v1.bin", 0x0000, 0x0200, CRC(3d2477e7) SHA1(795829a2025e24d87a413e245d72a284f872e0db), ROM_BIOS(1))

	ROM_REGION( 0x200, "chargen", 0 )
	ROM_LOAD( "dm8678cab.bin", 0x0000, 0x0200, CRC(8da502e7) SHA1(30d2dd9658823cdc2b2f6ef37f5a05d6f3e0db76))
ROM_END

} // Anonymous namespace

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT     CLASS          INIT        COMPANY                 FULLNAME      FLAGS
COMP( 1977, mk14,    0,      0,      mk14,    mk14,     mk14_state,    empty_init, "Science of Cambridge", "MK-14",      MACHINE_SUPPORTS_SAVE )
COMP( 1978, mk14vdu, mk14,   0,      mk14vdu, mk14vdu,  mk14vdu_state, empty_init, "Science of Cambridge", "MK-14 VDU",  MACHINE_SUPPORTS_SAVE )
