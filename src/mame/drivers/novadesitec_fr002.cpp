// license:BSD-3-Clause
// copyright-holders:

/*
Hardware notes:
PCB named FR002-E

MB90F533A MCU (with 128Kbytes internal ROM)
16.000 MHz XTAL (near MCU)
Lattice ispLSI 1032E High Density Programmable Logic
ADV476KP50 RAM-DAC
Oki M6376 sound chip
2x 8-dip banks

Not much can be done until the MCU is somehow dumped.
*/

#include "emu.h"

#include "cpu/f2mc16/f2mc16.h"
#include "sound/okim6376.h"

#include "screen.h"
#include "speaker.h"


namespace {

class novadesitec_fr002_state : public driver_device
{
public:
	novadesitec_fr002_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void fr002(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map);
};

uint32_t novadesitec_fr002_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void novadesitec_fr002_state::main_map(address_map &map)
{
}


static INPUT_PORTS_START( fr002 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW1:8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW2:8")
INPUT_PORTS_END


void novadesitec_fr002_state::fr002(machine_config &config)
{
	// basic machine hardware
	F2MC16(config, m_maincpu, 16_MHz_XTAL); // actually MB90F533A
	m_maincpu->set_addrmap(AS_PROGRAM, &novadesitec_fr002_state::main_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: all wrong
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(novadesitec_fr002_state::screen_update));

	// sound hardware
	SPEAKER(config, "mono").front_center(); // TODO: verify if stereo

	OKIM6376(config, "oki", 16_MHz_XTAL / 16).add_route(ALL_OUTPUTS, "mono", 0.75); // clock unknown
}


ROM_START( clrmatch )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cm v.1.9p.u5", 0x00000, 0x20000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx", 0 ) // TODO: verify ROM loading
	ROM_LOAD( "color match 1.u13",  0x000000, 0x080000, CRC(8f3978f6) SHA1(0a1b19c2df2a5b6d4875dce2809181d08dcf48c2) )
	ROM_LOAD( "color match 2.u14",  0x080000, 0x080000, CRC(42e35ca7) SHA1(cd4e2e34055d7fb71765e6dd0cecaca0f3e54ead) )
	ROM_LOAD( "color match 3.u16",  0x100000, 0x080000, CRC(88b3241e) SHA1(5636addb04c5a9a94ecfd6c72b9d91a5935ba6b0) )
	ROM_LOAD( "color match 4.u15",  0x180000, 0x080000, CRC(aa9e1764) SHA1(757a175dda53f3a6ad74cf2203c24443b77e8949) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "color match 5.u29", 0x00000, 0x80000, CRC(ad9778c9) SHA1(43bd016e8bdb43772159f088a465c5e5df9505fe) )
ROM_END

ROM_START( sportmem )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "sm ver.3.1.u5", 0x00000, 0x20000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx", 0 ) // TODO: verify ROM loading
	ROM_LOAD( "sport memory 1.u13",  0x000000, 0x080000, CRC(937b4e3d) SHA1(2f8a6b5639cc1558428ce27ee42be81e389e8748) )
	ROM_LOAD( "sport memory 2.u14",  0x080000, 0x080000, CRC(e5201108) SHA1(767eae3fbc761cdf4233693863f52b1a5eceddeb) )
	ROM_LOAD( "sport memory 3.u16",  0x100000, 0x080000, CRC(92798401) SHA1(99445aeb990e585c22a12ddb48137699ff6a147e) )
	ROM_LOAD( "sport memory 4.u15",  0x180000, 0x080000, CRC(0950af57) SHA1(743b592a87a7dd11cf6835ec9415386250e99783) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "sport memory 5.u29", 0x00000, 0x80000, CRC(b840682c) SHA1(7f2bda09f209fa72732a08fc05767219e13777ce) )
ROM_END

} // anonymous namespace


GAME( 200?, clrmatch, 0, fr002, fr002, novadesitec_fr002_state, empty_init, ROT0, "Nova Desitec", "Color Match",  MACHINE_IS_SKELETON )
GAME( 200?, sportmem, 0, fr002, fr002, novadesitec_fr002_state, empty_init, ROT0, "Nova Desitec", "Sport Memory", MACHINE_IS_SKELETON )
