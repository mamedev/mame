// license:BSD-3-Clause
// copyright-holders:

/*
Amstar Z80 based hardware for card games

Dumper's notes:

Etched in copper    AMSTAR ELEC
                    ASSY 1061-3700/
                    SER NO  42-109      42-109 was hand written

graphics dump showed card characters

.e5 2708        stickered   001-1201
.d5 AMD 4708    stickered   1102        read as a 2708 - couldn't get a steady reading
.b6 2708        stickered   001-8000
.b7 2708        stickered   001-8200
.c7 2708        stickered   001-8100
.c8 2708        stickered   001-8500
.d8 2708        stickered   001-8400
.e8 2708        stickered   001-8300
.d2 7611        stickered   001-1400
.f2 7611        stickered   001-1500
.g1 7611        stickered   001-130
.g2 6301        stickered   G2-     read as 7611
.h2 6301        stickered   H2-     read as 7611
.j2 6301        stickered   J2-     read as 7611

empty 14 pin socket at j1
empty 24 pin socket at b8
empty 16 pin socket at g11

Z80
10MHz Crystal
2101    x8
5101    x2

2 24 pin chips (.b4 and .b5) that look like EPROMs, but no window
Couldn't read as 2708 or 2716 or any of the 24 pin 82sxxx types
Stamped with the following
    .b4
        "S" logo
        8001E
        C27139M
        4502
    .b5
        "S" logo
        7847E
        C27138M
        4501
*/

#include "emu.h"

#include "cpu/z80/z80.h"

#include "emupal.h"
#include "screen.h"


namespace {

class amstarz80_state : public driver_device
{
public:
	amstarz80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void amstarz80(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void prg_map(address_map &map) ATTR_COLD;
};


uint32_t amstarz80_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void amstarz80_state::prg_map(address_map &map)
{
	map(0x0000, 0x17ff).rom();
	map(0x1c00, 0x1cff).ram();
	map(0x2000, 0x21ff).ram();
	map(0x2400, 0x25ff).ram();
	map(0x2800, 0x29ff).ram();
	map(0x4000, 0x4000).portr("IN0");
	map(0x4001, 0x4001).portr("IN1");
}


static INPUT_PORTS_START( holddraw )
	PORT_START("IN0")
	PORT_BIT(0x75, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0a, IP_ACTIVE_HIGH, IPT_UNKNOWN)

	PORT_START("IN1")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

// 2x 8 dip banks
// 1x 4 dip bank
INPUT_PORTS_END


// TODO: not right, just to get a decode
static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0, 1) },
	{ STEP8(0, 1*8) },
	8*8*1
};

static GFXDECODE_START( gfx_amstarz80 )
	GFXDECODE_ENTRY( "tiles", 0, charlayout, 0, 1 )
GFXDECODE_END

void amstarz80_state::amstarz80(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 10_MHz_XTAL / 4); // divisor not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &amstarz80_state::prg_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: all wrong, verify when redumped and working
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(8*8, 48*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(amstarz80_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::RGB_3BIT); // TODO: wrong

	GFXDECODE(config, "gfxdecode", "palette", gfx_amstarz80);

	// TODO: sound (TTL?)
}


ROM_START( holddraw )
	ROM_REGION( 0x1800, "maincpu", 0 )
	ROM_LOAD( "001-8000.b6", 0x0000, 0x0400, CRC(a25d17d4) SHA1(a5b9c83ace554811ac1442b44cdd72542ea2c425) )
	ROM_LOAD( "001-8100.c7", 0x0400, 0x0400, CRC(2208a37a) SHA1(8cb528c3f83f779873e82b0d959810d5e3073291) )
	ROM_LOAD( "001-8200.b7", 0x0800, 0x0400, CRC(9c27e9d7) SHA1(43103979e014ecc7b17a44a935257869ca184c2f) )
	ROM_LOAD( "001-8300.e8", 0x0c00, 0x0400, CRC(2b703868) SHA1(ac4e2903e0eeb6d248acd931ca37032bd9d928cc) )
	ROM_LOAD( "001-8400.d8", 0x1000, 0x0400, CRC(1b66fa9f) SHA1(98a1d53701a4dedeec9fa61a85dcbc0ce6910d5f) )
	ROM_LOAD( "001-8500.c8", 0x1400, 0x0400, CRC(9c219088) SHA1(7430fe4eb3a6bf1838fd5513127f5248723bec0a) )
	// one empty ROM socket at b8

	ROM_REGION(0x800, "tiles", 0 )
	ROM_LOAD( "001_1201.e5", 0x000, 0x400, CRC(32197f7d) SHA1(07c8739033820e4e4e2fe4428f921d8fc7c8698b) )
	ROM_LOAD( "1102.d5_a",   0x400, 0x400, BAD_DUMP CRC(bb4bd952) SHA1(a869d5d5c8bc45d414b1103c6fe2b2d7bb07291e) ) // handwritten label, 3 different reads until it can be determined if one is good or a good one can be assembled
	ROM_LOAD( "1102.d5_b",   0x400, 0x400, BAD_DUMP CRC(ce57a4ee) SHA1(732fc54f32212d7590daa2da738acd1cce4d7c0d) )
	ROM_LOAD( "1102.d5_c",   0x400, 0x400, BAD_DUMP CRC(0b30d921) SHA1(992709bb0ad18f646bfa1bccea45454a5f273457) )

	ROM_REGION(0xc00, "proms", 0 )
	ROM_LOAD( "001-1400.d2", 0x000, 0x200, CRC(32c99cfc) SHA1(64d561230d69514a02f30d7bc69caa563e069d69) )
	ROM_LOAD( "001-1500.f2", 0x200, 0x200, CRC(a27a7513) SHA1(84f5a29e55112d99a757b902aeef28e1370ef78f) )
	ROM_LOAD( "001-1300.g1", 0x400, 0x200, CRC(eb581932) SHA1(b04cf5bb18bfc91654f63984aaf8e656e616b36f) )
	ROM_LOAD( "g2.g2",       0x600, 0x200, CRC(6aa24121) SHA1(63fe82043653e753fb3d6ffb8a750b0433dae679) ) // handwritten label
	ROM_LOAD( "h2.h2",       0x800, 0x200, CRC(9096f7c8) SHA1(b51b068ae279f0cee6048415bffe14995fb7d269) ) // handwritten label
	ROM_LOAD( "j2.j2",       0xa00, 0x200, CRC(d7195174) SHA1(660e52c0d1c250ec9566d629c9e57e7b20acff26) ) // handwritten label
ROM_END

} // anonymous namespace


GAME( 1981, holddraw, 0, amstarz80, holddraw, amstarz80_state, empty_init, ROT0, "Amstar", "Hold & Draw", MACHINE_IS_SKELETON ) // supposedly, but might actually be another similar game
