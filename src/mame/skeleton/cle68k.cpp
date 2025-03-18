// license:BSD-3-Clause
// copyright-holders:

/*
CLE-TOUCH REV. 1 PCB

Video slots by Chain Luck Electronic (CLE).
At least some of the games were distributed in the USA by Lucky Sunshine Enterprises (LSE).

The main components are:
MC68HC000FN12 CPU
Lattice ispLSI 1032E 70LJ
Lattice ispLSI 1016 60LJ
Lattice iM4A5-32/32 10JC-12JI
2x HM6264LP-70 RAM (near ispLSI 1016)
2x HM6264LP-70 RAM (near CPU ROMs)
2x HM86171-80 RAM (near CPU ROMs)
12 MHz XTAL (for M68K)
AT90S4414 MCU (AVR core)
11.0592 MHz XTAL (for AT90?)
U6295 sound chip
6x 8-DIP banks


TODO: everything. Needs GFX decode to proceed further.
*/


#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class cle68k_state : public driver_device
{
public:
	cle68k_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void cle68k(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


uint32_t cle68k_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{

	return 0;
}

void cle68k_state::video_start()
{
}


void cle68k_state::program_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x180000, 0x180fff).ram();
	map(0x181000, 0x181fff).ram();
	map(0x182000, 0x182fff).ram();
	map(0x183000, 0x183fff).ram();
	// map(0x1e0004, 0x1e0005).portr("IN0");
	map(0x1e0009, 0x1e0009).w("oki", FUNC(okim6295_device::write));
	// map(0x1e0032, 0x1e0033).portr("DSW1");
	// map(0x1e0034, 0x1e0035).portr("DSW2");
	// map(0x1e0036, 0x1e0037).portr("DSW3");
	map(0x1f0000, 0x1fffff).ram();
}


static INPUT_PORTS_START( dmndhrt )
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

	PORT_START("IN2")
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

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x0001, 0x0001, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x0002, 0x0002, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x0004, 0x0004, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x0008, 0x0008, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x0010, 0x0010, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x0020, 0x0020, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x0040, 0x0040, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x0080, 0x0080, "SW1:8")
	PORT_DIPUNKNOWN_DIPLOC(0x0100, 0x0100, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x0200, 0x0200, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x0400, 0x0400, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x0800, 0x0800, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x1000, 0x1000, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x2000, 0x2000, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x4000, 0x4000, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x8000, 0x8000, "SW2:8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x0001, 0x0001, "SW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x0002, 0x0002, "SW3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x0004, 0x0004, "SW3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x0008, 0x0008, "SW3:4")
	PORT_DIPUNKNOWN_DIPLOC(0x0010, 0x0010, "SW3:5")
	PORT_DIPUNKNOWN_DIPLOC(0x0020, 0x0020, "SW3:6")
	PORT_DIPUNKNOWN_DIPLOC(0x0040, 0x0040, "SW3:7")
	PORT_DIPUNKNOWN_DIPLOC(0x0080, 0x0080, "SW3:8")
	PORT_DIPUNKNOWN_DIPLOC(0x0100, 0x0100, "SW4:1")
	PORT_DIPUNKNOWN_DIPLOC(0x0200, 0x0200, "SW4:2")
	PORT_DIPUNKNOWN_DIPLOC(0x0400, 0x0400, "SW4:3")
	PORT_DIPUNKNOWN_DIPLOC(0x0800, 0x0800, "SW4:4")
	PORT_DIPUNKNOWN_DIPLOC(0x1000, 0x1000, "SW4:5")
	PORT_DIPUNKNOWN_DIPLOC(0x2000, 0x2000, "SW4:6")
	PORT_DIPUNKNOWN_DIPLOC(0x4000, 0x4000, "SW4:7")
	PORT_DIPUNKNOWN_DIPLOC(0x8000, 0x8000, "SW4:8")

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC(0x0001, 0x0001, "SW5:1")
	PORT_DIPUNKNOWN_DIPLOC(0x0002, 0x0002, "SW5:2")
	PORT_DIPUNKNOWN_DIPLOC(0x0004, 0x0004, "SW5:3")
	PORT_DIPUNKNOWN_DIPLOC(0x0008, 0x0008, "SW5:4")
	PORT_DIPUNKNOWN_DIPLOC(0x0010, 0x0010, "SW5:5")
	PORT_DIPUNKNOWN_DIPLOC(0x0020, 0x0020, "SW5:6")
	PORT_DIPUNKNOWN_DIPLOC(0x0040, 0x0040, "SW5:7")
	PORT_DIPUNKNOWN_DIPLOC(0x0080, 0x0080, "SW5:8")
	PORT_DIPUNKNOWN_DIPLOC(0x0100, 0x0100, "SW6:1")
	PORT_DIPUNKNOWN_DIPLOC(0x0200, 0x0200, "SW6:2")
	PORT_DIPUNKNOWN_DIPLOC(0x0400, 0x0400, "SW6:3")
	PORT_DIPUNKNOWN_DIPLOC(0x0800, 0x0800, "SW6:4")
	PORT_DIPUNKNOWN_DIPLOC(0x1000, 0x1000, "SW6:5")
	PORT_DIPUNKNOWN_DIPLOC(0x2000, 0x2000, "SW6:6")
	PORT_DIPUNKNOWN_DIPLOC(0x4000, 0x4000, "SW6:7")
	PORT_DIPUNKNOWN_DIPLOC(0x8000, 0x8000, "SW6:8")
INPUT_PORTS_END


// TODO
static GFXDECODE_START( gfx_cle68k )
GFXDECODE_END


void cle68k_state::cle68k(machine_config &config)
{
	M68000(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &cle68k_state::program_map);
	m_maincpu->set_vblank_int("screen", FUNC(cle68k_state::irq1_line_hold));

	// AT90S4414 (needs core)

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0, 64*8-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(cle68k_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_cle68k);

	PALETTE(config, "palette").set_entries(0x100); // TODO

	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 12_MHz_XTAL / 12, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // pin 7 and clock not verified
}


ROM_START( dmndhrt )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "diamond_heart_u.s.a_u8.u8", 0x00000, 0x20000, CRC(d1f340ce) SHA1(7567448c8694bb24f7957bb461d3be51d138634a) )
	ROM_LOAD16_BYTE( "diamond_heart_u.s.a_u3.u3", 0x00001, 0x20000, CRC(78885bb8) SHA1(51e360036d32b609b4036be086549c011ab41fe3) )

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "at90s4414.u51", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD( "diamond_heart_u.s.a_u10.u10", 0x00000, 0x80000, CRC(00b691a7) SHA1(8cc530ad204cf9168d59419a01abf338c46a49e1) )
	ROM_LOAD( "diamond_heart_u.s.a_u11.u11", 0x80000, 0x80000, CRC(2c666c44) SHA1(15c8e97900444046adb9455bfa827735c226a727) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "diamond_heart_u.s.a_u33.u33", 0x00000, 0x40000, CRC(63b0bc97) SHA1(12adb70a8283c6fec10e2221f1216a7fbfc99355) )

	// PAL locations not readable
	ROM_REGION( 0x800, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "palce20v8h_1", 0x000, 0x157, NO_DUMP )
	ROM_LOAD( "palce20v8h_2", 0x200, 0x157, NO_DUMP )
	ROM_LOAD( "palce20v8h_3", 0x400, 0x157, NO_DUMP )
	ROM_LOAD( "palce20v8h_4", 0x600, 0x157, NO_DUMP )
ROM_END

ROM_START( dmndhrtn ) // u51 was scratched for this set but believed to be AT90S4414, too
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "w27e010.u8", 0x00000, 0x20000, CRC(2a1ba91e) SHA1(af340d9e0aa7874669557067a9e043eecdf5301b) ) // no sticker
	ROM_LOAD16_BYTE( "w27e010.u3", 0x00001, 0x20000, CRC(cdb26ff2) SHA1(33ddda977a5f6436a690fa53763f36c7e6acfb94) ) // no sticker

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "at90s4414.u51", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD( "diamond_heart_new_mon_v20.0_u10.u10", 0x00000, 0x80000, CRC(7525bd95) SHA1(b34ab59bde9ecdfe03489a6eceda2c95afdee6c8) )
	ROM_LOAD( "diamond_heart_new_mon_v20.0_u11.u11", 0x80000, 0x80000, CRC(1ffc66a6) SHA1(fd5bfa9ec01ad7aa3060929dbce417babe241700) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "f29c51002t.u33", 0x00000, 0x40000, CRC(97f774cd) SHA1(7c5a1c4a0e7cfb71e24d174c43a83735abfc59c8) ) // no sticker

	// PAL locations not readable
	ROM_REGION( 0x800, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "gal20v8b_1", 0x000, 0x157, NO_DUMP )
	ROM_LOAD( "gal20v8b_2", 0x200, 0x157, NO_DUMP )
	ROM_LOAD( "gal20v8b_3", 0x400, 0x157, NO_DUMP )
	ROM_LOAD( "gal20v8b_4", 0x600, 0x157, NO_DUMP )
ROM_END

ROM_START( honeybee ) // u51 was scratched for this set but believed to be AT90S4414, too
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "honey_bee_hb_tw_u8.u8", 0x00000, 0x40000, CRC(1e7e53a3) SHA1(30d426cca499adf82338ba6cc1391f754e908a5b) )
	ROM_LOAD16_BYTE( "honey_bee_hb_tw_u3.u3", 0x00001, 0x40000, CRC(0ed5f0cc) SHA1(f64c27f04f74162027070f889daaec6f1847f19e) )

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "at90s4414.u51", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD( "honey_bee_hb_tw_u10.u10", 0x00000, 0x80000, CRC(40526fe1) SHA1(58a3a16c4dc0fa6527571b924f43377657f0cc76) )
	ROM_LOAD( "honey_bee_hb_tw_u11.u11", 0x80000, 0x80000, CRC(3036a082) SHA1(16393fac3ccd5c2fc6ab9fd11f8530aace94e4fc) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "honey_bee_hb_tw_u33.u33", 0x00000, 0x40000, CRC(a85f1bfc) SHA1(c2b83a2570280a43241b89fdb21e87c8cf033409) )

	// PAL locations not readable
	ROM_REGION( 0x800, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "gal20v8b_1", 0x000, 0x157, NO_DUMP )
	ROM_LOAD( "gal20v8b_2", 0x200, 0x157, NO_DUMP )
	ROM_LOAD( "gal20v8b_3", 0x400, 0x157, NO_DUMP )
	ROM_LOAD( "gal20v8b_4", 0x600, 0x157, NO_DUMP )
ROM_END

} // anonymous namespace


// TODO: possibly licensed to LSE, verify once it works
GAME( 2001, dmndhrt,  0, cle68k, dmndhrt, cle68k_state, empty_init, ROT0, "LSE", "Diamond Heart (v1.06)",     MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // 2001/02/15
GAME( 2003, dmndhrtn, 0, cle68k, dmndhrt, cle68k_state, empty_init, ROT0, "CLE", "Diamond Heart New (v20.0)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // 2003/04/25
GAME( 2004, honeybee, 0, cle68k, dmndhrt, cle68k_state, empty_init, ROT0, "LSE", "Honey-Bee (v3.0)",          MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // 2004/07/01
