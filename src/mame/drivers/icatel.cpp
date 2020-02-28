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
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	DECLARE_READ8_MEMBER(magic_string);

	DECLARE_READ8_MEMBER(i80c31_p1_r);
	DECLARE_READ8_MEMBER(i80c31_p3_r);
	DECLARE_WRITE8_MEMBER(i80c31_p1_w);
	DECLARE_WRITE8_MEMBER(i80c31_p3_w);

	DECLARE_READ8_MEMBER(cn8_extension_r);
	DECLARE_WRITE8_MEMBER(cn8_extension_w);

	DECLARE_READ8_MEMBER(modem_r);
	DECLARE_WRITE8_MEMBER(modem_w);

	DECLARE_READ8_MEMBER(ci8_r);
	DECLARE_WRITE8_MEMBER(ci8_w);

	DECLARE_READ8_MEMBER(ci15_r);
	DECLARE_WRITE8_MEMBER(ci15_w);

	DECLARE_READ8_MEMBER(ci16_r);
	DECLARE_WRITE8_MEMBER(ci16_w);

	void icatel_palette(palette_device &palette) const;

	HD44780_PIXEL_UPDATE(icatel_pixel_update);

	void i80c31_data(address_map &map);
	void i80c31_io(address_map &map);
	void i80c31_prg(address_map &map);

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
	map(0x8000, 0x8002).ram(); /* HACK! */
	map(0x8040, 0x8041).mirror(0x3F1E).w(m_lcdc, FUNC(hd44780_device::write)); // not sure yet. CI12 (73LS273)
	map(0x8060, 0x8060).mirror(0x3F1F).rw(FUNC(icatel_state::ci8_r), FUNC(icatel_state::ci8_w));
	map(0x8080, 0x8080).mirror(0x3F1F).rw(FUNC(icatel_state::ci16_r), FUNC(icatel_state::ci16_w)); // card reader (?)
	map(0x80C0, 0x80C0).mirror(0x3F1F).rw(FUNC(icatel_state::ci15_r), FUNC(icatel_state::ci15_w)); // 74LS244 (tristate buffer)
	map(0xC000, 0xCFFF).rw(FUNC(icatel_state::cn8_extension_r), FUNC(icatel_state::cn8_extension_w));
	map(0xE000, 0xEFFF).rw(FUNC(icatel_state::modem_r), FUNC(icatel_state::modem_w));
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

READ8_MEMBER(icatel_state::magic_string)
{
//  logerror("read: magic_string, offset=%04X\n", offset);
	char mstr[] = "TP-OK";
	return mstr[offset%5];
}

READ8_MEMBER(icatel_state::i80c31_p1_r)
{
	return 0x7f;
}

READ8_MEMBER(icatel_state::i80c31_p3_r)
{
	return 0xff;
}

WRITE8_MEMBER(icatel_state::i80c31_p1_w)
{
}

WRITE8_MEMBER(icatel_state::i80c31_p3_w)
{
}

//----------------------------------------

READ8_MEMBER(icatel_state::cn8_extension_r)
{
	/* TODO: Implement-me! */
	logerror("read: cn8_extension\n");
	return 0;
}

WRITE8_MEMBER(icatel_state::cn8_extension_w)
{
	/* TODO: Implement-me! */
	logerror("write: cn8_extension [%02x]\n", data);
}

//----------------------------------------

READ8_MEMBER(icatel_state::modem_r)
{
	/* TODO: Implement-me! */
	logerror("read: modem\n");
	return 0;
}

WRITE8_MEMBER(icatel_state::modem_w)
{
	/* TODO: Implement-me! */
	logerror("write: modem [%02x]\n", data);
}

//----------------------------------------

READ8_MEMBER(icatel_state::ci8_r)
{
	/* TODO: Implement-me! */
	logerror("read: ci8\n");
	return 0;
}

WRITE8_MEMBER(icatel_state::ci8_w)
{
	/* TODO: Implement-me! */
	logerror("write: ci8 [%02x]\n", data);
}

//----------------------------------------

READ8_MEMBER(icatel_state::ci15_r)
{
	/* TODO: Implement-me! */
	//machine().debug_break();
	//logerror("read: ci15\n");
	return (1 << 3) | (1 << 0);
}

WRITE8_MEMBER(icatel_state::ci15_w)
{
	/* TODO: Implement-me! */
	logerror("write: ci15 [%02x]\n", data);
}

//----------------------------------------

READ8_MEMBER(icatel_state::ci16_r)
{
	/* TODO: Implement-me! */
	// seems to be the card reader.
	logerror("read: ci16\n");
	return 0;
}

WRITE8_MEMBER(icatel_state::ci16_w)
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
		bitmap.pix16(y, pos*6 + x) = state;
	}

	if ( pos >= 64 && pos < 80 && line==0 )
	{
		bitmap.pix16(y+9,(pos-64)*6 + x) = state;
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

	HD44780(config, m_lcdc, 0);
	m_lcdc->set_lcd_size(2, 16);
	m_lcdc->set_pixel_update_cb(FUNC(icatel_state::icatel_pixel_update));
}

ROM_START( icatel )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "icatel_tpci_em._4_v16.05.ci14",  0x00000, 0x8000, CRC(d310586e) SHA1(21736ad5a06cf9695f8cc5ff2dc2d19b101504f5) )
ROM_END

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS         INIT         COMPANY   FULLNAME                            FLAGS
COMP( 1995, icatel, 0,      0,      icatel,  0,     icatel_state, init_icatel, "Icatel", "TPCI (Brazilian public payphone)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND)
/*The hardware was clearly manufactured in 1995. There's no evindence of the actual date of the firmware.*/
