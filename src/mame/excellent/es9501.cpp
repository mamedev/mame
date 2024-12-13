// license:BSD-3-Clause
// copyright-holders:

/*
Excellent System's ES-9501 PCB

Main components:
TMP68HC000-P16 CPU
2x HM6265LK-70 RAM (near CPU)
28.6363 MHz XTAL (near CPU)
ES-9409 custom (GFX, same as dblcrown.cpp)
6x N341256P-15 RAM (near custom)
IS61C64 RAM (near custom, exact type not readable)
YMZ280B-F sound chip (no XTAL, running internal?)
2x N341256P-15 RAM (near YMZ280B-F)
MAX693ACPE watchdog
93C56 EEPROM
bank of 8 DIP switches

Undumped games known to run on this PCB:
* Multi Spin
* Star Ball
*/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "machine/watchdog.h"
#include "sound/ymz280b.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class es9501_state : public driver_device
{
public:
	es9501_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void es9501(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


void es9501_state::video_start()
{
}

uint32_t es9501_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void es9501_state::program_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x3fc000, 0x3fffff).ram();
	map(0x400000, 0x407fff).ram();
	map(0x600000, 0x600001).portr("IN0");
	map(0x600002, 0x600003).portr("IN1");
	map(0x600004, 0x600005).portr("DSW");
	// map(0x600008, 0x600009).w // watchdog?
	map(0x700000, 0x700003).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write)).umask16(0x00ff); // ??
}


static INPUT_PORTS_START( specd9 )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")
INPUT_PORTS_END


static const gfx_layout char_16x16_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 4,0, 12,8, 20,16, 28,24, 36,32, 44,40, 52,48, 60,56 },
	{ STEP16(0,8*8) },
	8*8*16
};

static GFXDECODE_START( gfx_es9501 )
	GFXDECODE_ENTRY( "gfx", 0, char_16x16_layout, 0, 0x10 )
GFXDECODE_END


void es9501_state::es9501(machine_config &config)
{
	M68000(config, m_maincpu, 28.636363_MHz_XTAL / 2); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &es9501_state::program_map);

	EEPROM_93C56_16BIT(config, "eeprom");

	WATCHDOG_TIMER(config, "watchdog", 0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER); // TODO: everything
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(40*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(es9501_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_es9501);
	PALETTE(config, m_palette).set_format(palette_device::RRRRGGGGBBBBRGBx, 0x1000 / 2); // TODO

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ymz280b_device &ymz(YMZ280B(config, "ymz", 16.9344_MHz_XTAL));
	ymz.add_route(0, "lspeaker", 1.0);
	ymz.add_route(1, "rspeaker", 1.0);
}


ROM_START( specd9 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "3.u33", 0x00000, 0x40000, CRC(e4b00f37) SHA1(4c33912b7c38399ba2ca5e4dc0335458d929bd52) )
	ROM_LOAD16_BYTE( "2.u31", 0x00001, 0x40000, CRC(620bc09e) SHA1(fce0e9c7394aa782d0b6f1558a3b4c76c5c1e787) )

	ROM_REGION( 0x280000, "gfx", 0 )
	ROM_LOAD( "t58.u10", 0x000000, 0x200000, CRC(7a572d9e) SHA1(9a1d842ac78fea6047242c405aaf81c827dc2358) ) // contains Multi Spin logo
	ROM_LOAD( "u51.u51", 0x200000, 0x080000, CRC(a213c33b) SHA1(42b4c3d3cb2db50ea0fad06509e3e73b81f3db4c) ) // TODO: this is an EPROM, contains Special Dream 9 logo, should be overlayed on the mask ROM contents, IGS style

	ROM_REGION( 0x200000, "ymz", 0 )
	ROM_LOAD( "t59.u23", 0x000000, 0x200000, CRC(b11857b4) SHA1(c0a6478fd8a8ef1ed35cfbfa9fd2af44eb258725) )

	ROM_REGION16_BE( 0x100, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "93c56.u12", 0x000, 0x100, CRC(dba91cd8) SHA1(dfbe41e3a8d7e8ad7068d25afe10a1d93bf3cc4d) )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "3.u37", 0x000, 0x117, CRC(bea4cb24) SHA1(09987e6b903cc3bd202a9d933474b36bdbb99d9a) ) // PALCE16V8H
ROM_END

} // anonymous namespace


GAME( 1997, specd9, 0, es9501, specd9, es9501_state, empty_init, ROT0, "Excellent System", "Special Dream 9 (v1.0.5G)", MACHINE_IS_SKELETON )
