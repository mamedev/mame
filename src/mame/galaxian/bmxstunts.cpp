// license:BSD-3-Clause
// copyright-holders:

/*
BMX Stunts by Jetsoft

This game runs on bootleg Galaxian style PCB, but an epoxy block is fitted in the CPU socket.
This contains a M6502, a SN76489, a PROM and logic.
The M6502 was possibly chosen because Jetsoft programmed an analogous game on the C64: BMX Trials

Until differences and commonalities can be ascertained, this doesn't derive from galaxian_state.
Might be possible to derive later.
*/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "machine/watchdog.h"
#include "sound/sn76496.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class bmxstunts_state : public driver_device
{
public:
	bmxstunts_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
	{ }

	void bmxstunts(machine_config &config);

	void init_bmxstunts();

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void palette(palette_device &palette);

	void prg_map(address_map &map);
};



void bmxstunts_state::palette(palette_device &palette) // TODO: taken from galaxian.cpp, verify if good
{
	const uint8_t *color_prom = memregion("proms")->base();
	static const int rgb_resistances[3] = { 1000, 470, 220 };

	double rweights[3], gweights[3], bweights[2];
	compute_resistor_weights(0, 224, -1.0,
			3, &rgb_resistances[0], rweights, 470, 0,
			3, &rgb_resistances[0], gweights, 470, 0,
			2, &rgb_resistances[1], bweights, 470, 0);

	// decode the palette first
	int const len = memregion("proms")->bytes();
	for (int i = 0; i < len; i++)
	{
		uint8_t bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = combine_weights(rweights, bit0, bit1, bit2);

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = combine_weights(gweights, bit0, bit1, bit2);

		// blue component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = combine_weights(bweights, bit0, bit1);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

uint32_t bmxstunts_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void bmxstunts_state::prg_map(address_map &map)
{
	map(0xc000, 0xffff).rom().region("maincpu", 0);
}


static INPUT_PORTS_START( bmxstunts )
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1") // only one 6-dip bank
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ STEP8(0,1), STEP8(8*8,1) },
	{ STEP8(0,8), STEP8(16*8,8) },
	16*16
};

static GFXDECODE_START(gfx_bmxstunts)
	GFXDECODE_SCALE("gfx", 0x0000, charlayout,   0, 8, 3, 1)
	GFXDECODE_SCALE("gfx", 0x0000, spritelayout, 0, 8, 3, 1)
GFXDECODE_END


void bmxstunts_state::bmxstunts(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 3'072'000); // TODO: verify clock
	m_maincpu->set_addrmap(AS_PROGRAM, &bmxstunts_state::prg_map);

	// video hardware
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_bmxstunts);
	PALETTE(config, m_palette, FUNC(bmxstunts_state::palette), 32);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(bmxstunts_state::screen_update));

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	SN76489(config, "snsnd", 3'072'000).add_route(ALL_OUTPUTS, "speaker", 1.0); // TODO: verify clock
}


/*
BMX Stunts by Jetsoft on Galaxian bootleg PCB.

6502A CPU in epoxy block with one 6331 prom (not dumped)
One 74LS74 and one 74LS273 logic.
One SN76489 Digital Complex Sound Generator.
There was a wire lead coming out of the epoxy and soldered
to the sound/amplifier section on the PCB.

Program ROMs were on a riser board plugged into the two sockets
on the main PCB much like the standard Galaxian by Midway except
this riser board has eight sockets instead of the normal five and
is printed with the words "MOON PROGRAM", was possibly a bootleg
Moon Cresta PCB before conversion to BMX Stunts.

Color prom is unique to this game and doesn't match any others.

Main program EPROMs are all 2716 type by different manufacturers.
Graphics ROMs are 2732 EPROMs soldered directly to the main PCB
with pins 18 lifted and a wire connecting both then going to pin 10
of the IC at location 6R on the main PCB.
Another wire goes from pin 12 of IC at 6R to the IC at 4S pin 3 that has
been cut and lifted from the PCB.

There is another wire mod at IC location 2N and looks like a trace has
been cut between pins 9 and 10?

Non working board. Powers up to screen full of graphics.

chaneman 7/31/2022
*/

ROM_START( bmxstunts )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "bmx1.pr1", 0x0000, 0x0800, CRC(cf3061f1) SHA1(e229a2a09b56332359c3f87953acb07c4c7d3abb) )
	ROM_LOAD( "bmx2.pr2", 0x0800, 0x0800, CRC(f145e09d) SHA1(8d3f379dbb5ec9304aa61d99cac003dfb8050485) )
	ROM_LOAD( "bmx3.pr3", 0x1000, 0x0800, CRC(ea415c49) SHA1(eb55b4b24ef4e04f5c2873ad7fef2dce891cefef) )
	ROM_LOAD( "bmx4.pr4", 0x1800, 0x0800, CRC(62bdd971) SHA1(864e787d66f6deb7fa545c475d4feb551e095bf2) )
	ROM_LOAD( "bmx5.pr5", 0x2000, 0x0800, CRC(b7ae2316) SHA1(17aa542fe8d4f729758f8b21bc667bf756b481b5) )
	ROM_LOAD( "bmx6.pr6", 0x2800, 0x0800, CRC(ba9b1a69) SHA1(b17964b31435809ce174f2680f7b463658794220) )
	ROM_LOAD( "bmx7.pr7", 0x3000, 0x0800, CRC(32636839) SHA1(6371c929b7b3a819dad70b672bc3ca5c3c5c9ced) )
	ROM_LOAD( "bmx8.pr8", 0x3800, 0x0800, CRC(fe1052ee) SHA1(f8bcaaecc3dfd10c70cbd9a49b778232ba9e697b) )

	ROM_REGION( 0x2000, "gfx", 0 ) // possibly slightly corrupted (see tile viewer)
	ROM_LOAD( "bmxh.1h", 0x0000, 0x1000, CRC(b049f648) SHA1(06c5a8b15f876cb6e4798cb5f8b1351cc6c12877) )
	ROM_LOAD( "bmxl.1l", 0x1000, 0x1000, CRC(a0f44f47) SHA1(b9d40ff82bb90125f0d9ad2d9590ddd7cc600805) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "bmx6331.6l", 0x0000, 0x0020, CRC(ce3e9306) SHA1(62dc5208eea2d3126e61cc7af30e71a9e60d438c) )

	ROM_REGION( 0x0020, "epoxy_block_prom", 0 ) // maybe related to address lines scramble?
	ROM_LOAD( "6331", 0x0000, 0x0020, NO_DUMP )
ROM_END


/*
Dumped by Andrew Welburn
on the day of 18/07/10


PCB is a bootleg Galaxian, with pin headers, probably
of European origin. The signs and marking point to
it being a Moon Cresta, but I'm note sure. Also it
has a potted block in the CPU socket...
*/

ROM_START( bmxstuntsa )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "b-m.1", 0x0000, 0x0800, CRC(cf3061f1) SHA1(e229a2a09b56332359c3f87953acb07c4c7d3abb) )
	ROM_LOAD( "b-mx.2", 0x0800, 0x0800, CRC(f145e09d) SHA1(8d3f379dbb5ec9304aa61d99cac003dfb8050485) )
	ROM_LOAD( "b-mx.3", 0x1000, 0x0800, CRC(ea415c49) SHA1(eb55b4b24ef4e04f5c2873ad7fef2dce891cefef) )
	ROM_LOAD( "b-mx.4", 0x1800, 0x0800, CRC(62bdd971) SHA1(864e787d66f6deb7fa545c475d4feb551e095bf2) )
	ROM_LOAD( "b-mx.5", 0x2000, 0x0800, CRC(9fa3d4e3) SHA1(61973d99d68790e36112bdaa893fb9406f8d46ca) ) // bmx5.pr5  51.074219%
	ROM_LOAD( "b-mx.6", 0x2800, 0x0800, CRC(ba9b1a69) SHA1(b17964b31435809ce174f2680f7b463658794220) )
	ROM_LOAD( "b-mx.7", 0x3000, 0x0800, CRC(fa34441a) SHA1(f1591ef81c4fc9c3cd1b9eb96d945d53051a3ea7) ) // bmx7.pr7  58.740234%
	ROM_LOAD( "b-mx.8", 0x3800, 0x0800, CRC(8bc26d4d) SHA1(c01be14d7cd402a524b61bd845c1ae6b09967bfa) ) // bmx8.pr8  99.267578%

	ROM_REGION( 0x2000, "gfx", 0 ) // not dumped for this set, taken from above
	ROM_LOAD( "bmxh.1h", 0x0000, 0x1000, BAD_DUMP CRC(b049f648) SHA1(06c5a8b15f876cb6e4798cb5f8b1351cc6c12877) )
	ROM_LOAD( "bmxl.1l", 0x1000, 0x1000, BAD_DUMP CRC(a0f44f47) SHA1(b9d40ff82bb90125f0d9ad2d9590ddd7cc600805) )

	ROM_REGION( 0x0020, "proms", 0 ) // not dumped for this set, taken from above
	ROM_LOAD( "bmx6331.6l", 0x0000, 0x0020, BAD_DUMP CRC(ce3e9306) SHA1(62dc5208eea2d3126e61cc7af30e71a9e60d438c) )

	ROM_REGION( 0x0020, "epoxy_block_prom", 0 ) // maybe related to address lines scramble?
	ROM_LOAD( "6331", 0x0000, 0x0020, NO_DUMP )
ROM_END


void bmxstunts_state::init_bmxstunts()
{
	uint8_t *rom = memregion("maincpu")->base();

	std::vector<uint8_t> buffer(0x4000);

	memcpy(&buffer[0], rom, 0x4000);

	for (int i = 0; i < 0x4000; i++)
	{
		rom[i] = buffer[i ^ 0x01];
	}
}

} // Anonymous namespace


GAME( 198?, bmxstunts,  0,         bmxstunts, bmxstunts, bmxstunts_state, init_bmxstunts, ROT90, "Jetsoft", "BMX Stunts (set 1)", MACHINE_IS_SKELETON )
GAME( 198?, bmxstuntsa, bmxstunts, bmxstunts, bmxstunts, bmxstunts_state, init_bmxstunts, ROT90, "Jetsoft", "BMX Stunts (set 2)", MACHINE_IS_SKELETON )
