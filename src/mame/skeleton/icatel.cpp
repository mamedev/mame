// license:GPL-2.0+
// copyright-holders: Felipe Sanches
/***************************************************************************

    icatel - Brazilian public payphone
    manufactured by icatel http://www.icatel.com.br/

    Partial schematics (drawn based on PCB inspection) available at:
    https://github.com/garoa/Icatel/blob/master/doc/icatel.pdf

    Driver by Felipe Sanches <juca@members.fsf.org>

    Changelog:

    2014 DEC 14 [Felipe Sanches]:
    * Initial driver skeleton

***************************************************************************/

#include "emu.h"

#include "cpu/mcs51/mcs51.h"
#include "video/hd44780.h"
//#include "sound/speaker.h"

#include "debugger.h"
#include "emupal.h"
#include "screen.h"


namespace {

class icatel_state : public driver_device
{
public:
	icatel_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc(*this, "hd44780")
	{ }

	void icatel(machine_config &config);

	void init_icatel();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	[[maybe_unused]] uint8_t magic_string(offs_t offset);

	uint8_t i80c31_p1_r();
	uint8_t i80c31_p3_r();
	void i80c31_p1_w(uint8_t data);
	void i80c31_p3_w(uint8_t data);

	uint8_t cn8_extension_r();
	void cn8_extension_w(uint8_t data);

	uint8_t modem_r(offs_t offset);
	void modem_w(offs_t offset, uint8_t data);

	void ci8_w(uint8_t data);
	void ci12_w(uint8_t data);
	uint8_t ci15_r();
	void ci16_w(uint8_t data);

	void icatel_palette(palette_device &palette) const;

	HD44780_PIXEL_UPDATE(icatel_pixel_update);

	void i80c31_data(address_map &map) ATTR_COLD;
	void i80c31_io(address_map &map) ATTR_COLD;
	void i80c31_prg(address_map &map) ATTR_COLD;

	required_device<i80c31_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;
};

void icatel_state::i80c31_prg(address_map &map)
{
	map(0x0000, 0x7FFF).mirror(0x8000).rom();
}

void icatel_state::i80c31_io(address_map &map)
{
	map(0x0000, 0x3FFF).ram();
	map(0x8000, 0x8001).mirror(0x3F3C).w(m_lcdc, FUNC(hd44780_device::write));
	map(0x8002, 0x8003).mirror(0x3F3C).r(m_lcdc, FUNC(hd44780_device::read));
	map(0x8040, 0x8040).mirror(0x3F1F).w(FUNC(icatel_state::ci12_w)); // 74LS273
	map(0x8060, 0x8060).mirror(0x3F1F).w(FUNC(icatel_state::ci8_w));
	map(0x8080, 0x8080).mirror(0x3F1F).w(FUNC(icatel_state::ci16_w)); // card reader (?)
	map(0x80C0, 0x80C0).mirror(0x3F1F).r(FUNC(icatel_state::ci15_r)); // 74LS244 (tristate buffer)
	map(0xC000, 0xCFFF).rw(FUNC(icatel_state::cn8_extension_r), FUNC(icatel_state::cn8_extension_w));
	map(0xE000, 0xE0FF).mirror(0xF00).rw(FUNC(icatel_state::modem_r), FUNC(icatel_state::modem_w));
}

void icatel_state::i80c31_data(address_map &map)
{
//  map(0x0056,0x005A).r(FUNC(icatel_state::magic_string)); /* This is a hack! */
}

void icatel_state::init_icatel()
{
}

void icatel_state::machine_start()
{
}

void icatel_state::machine_reset()
{
}

uint8_t icatel_state::magic_string(offs_t offset)
{
//  logerror("read: magic_string, offset=%04X\n", offset);
	char mstr[] = "TP-OK";
	return mstr[offset%5];
}

uint8_t icatel_state::i80c31_p1_r()
{
	return 0x7f;
}

uint8_t icatel_state::i80c31_p3_r()
{
	return 0xff;
}

void icatel_state::i80c31_p1_w(uint8_t data)
{
}

void icatel_state::i80c31_p3_w(uint8_t data)
{
}

//----------------------------------------

uint8_t icatel_state::cn8_extension_r()
{
	/* TODO: Implement-me! */
	logerror("read: cn8_extension\n");
	return 0;
}

void icatel_state::cn8_extension_w(uint8_t data)
{
	/* TODO: Implement-me! */
	logerror("write: cn8_extension [%02x]\n", data);
}

//----------------------------------------

uint8_t icatel_state::modem_r(offs_t offset)
{
	/* TODO: Implement-me! */
	logerror("read: modem\n");
	return 0;
}

void icatel_state::modem_w(offs_t offset, uint8_t data)
{
	/* TODO: Implement-me! */
	logerror("write: modem %02x:[%02x]\n", offset, data);
}

//----------------------------------------

void icatel_state::ci8_w(uint8_t data)
{
	/* TODO: Implement-me! */
	logerror("write: ci8 [%02x]\n", data);
}

//----------------------------------------

void icatel_state::ci12_w(uint8_t data)
{
	/* TODO: Implement-me! */
	logerror("write: ci12 [%02x]\n", data);
}

//----------------------------------------

uint8_t icatel_state::ci15_r()
{
	/* TODO: Implement-me! */
	//machine().debug_break();
	//logerror("read: ci15\n");
	return (1 << 3) | (1 << 0);
}

//----------------------------------------

void icatel_state::ci16_w(uint8_t data)
{
	/* TODO: Implement-me! */
	// seems to be the card reader.
	logerror("write: ci16 [%02x]\n", data);
}

//----------------------------------------

void icatel_state::icatel_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

static const gfx_layout prot_charlayout =
{
	5, 8,                   /* 5 x 8 characters */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	{ 3, 4, 5, 6, 7},
	{ 0, 8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8},
	8*8                     /* 8 bytes */
};

static GFXDECODE_START( gfx_icatel )
	GFXDECODE_ENTRY( "hd44780:cgrom", 0x0000, prot_charlayout, 0, 1 )
GFXDECODE_END

HD44780_PIXEL_UPDATE(icatel_state::icatel_pixel_update)
{
	if ( pos < 16 && line==0 )
	{
		bitmap.pix(y, pos*6 + x) = state;
	}

	if ( pos >= 64 && pos < 80 && line==0 )
	{
		bitmap.pix(y+9,(pos-64)*6 + x) = state;
	}
}

void icatel_state::icatel(machine_config &config)
{
	/* basic machine hardware */
	I80C31(config, m_maincpu, XTAL(2'097'152));
	m_maincpu->set_addrmap(AS_PROGRAM, &icatel_state::i80c31_prg);
	m_maincpu->set_addrmap(AS_DATA, &icatel_state::i80c31_data);
	m_maincpu->set_addrmap(AS_IO, &icatel_state::i80c31_io);
	m_maincpu->port_in_cb<1>().set(FUNC(icatel_state::i80c31_p1_r));
	m_maincpu->port_out_cb<1>().set(FUNC(icatel_state::i80c31_p1_w));
	m_maincpu->port_in_cb<3>().set(FUNC(icatel_state::i80c31_p3_r));
	m_maincpu->port_out_cb<3>().set(FUNC(icatel_state::i80c31_p3_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("hd44780", FUNC(hd44780_device::screen_update));
	screen.set_size(6*16, 9*2);
	screen.set_visarea(0, 6*16-1, 0, 9*2-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(icatel_state::icatel_palette), 2);
	GFXDECODE(config, "gfxdecode", "palette", gfx_icatel);

	HD44780(config, m_lcdc, 270'000); /* TODO: clock not measured, datasheet typical clock used */
	m_lcdc->set_lcd_size(2, 16);
	m_lcdc->set_pixel_update_cb(FUNC(icatel_state::icatel_pixel_update));
}

ROM_START( icatel )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "icatel_tpci_em._4_v16.05.ci14",  0x00000, 0x8000, CRC(d310586e) SHA1(21736ad5a06cf9695f8cc5ff2dc2d19b101504f5) )
ROM_END

} // anonymous namespace


//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS         INIT         COMPANY   FULLNAME                            FLAGS
COMP( 1995, icatel, 0,      0,      icatel,  0,     icatel_state, init_icatel, "Icatel", "TPCI (Brazilian public payphone)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND)
/*The hardware was clearly manufactured in 1995. There's no evindence of the actual date of the firmware.*/
