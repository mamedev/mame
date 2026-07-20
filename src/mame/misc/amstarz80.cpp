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

TODO:
- remove machine().rand() hacks;
- palette;
- sound;
- lamps;
- check for missing inputs;
- is there a hopper?
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/nvram.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


namespace {

class amstarz80_state : public driver_device
{
public:
	amstarz80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_gfxdecode(*this, "gfxdecode")
		, m_tileram(*this, "tileram")
		, m_attrram(*this, "attrram")
		, m_dsw3(*this, "DSW3")
	{ }

	void amstarz80(machine_config &config) ATTR_COLD;
	void amidon(machine_config &config) ATTR_COLD;

	void init_amstarz80() ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint8_t> m_tileram;
	required_shared_ptr<uint8_t> m_attrram;

	required_ioport m_dsw3;

	tilemap_t *m_bg_tilemap = nullptr;

	void init_gfx(const char *tag) ATTR_COLD;

	void tileram_w(offs_t offset, uint8_t data);
	void attrram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void amstarz80_program_map(address_map &map) ATTR_COLD;
	void amidon_program_map(address_map &map) ATTR_COLD;
};


TILE_GET_INFO_MEMBER(amstarz80_state::get_bg_tile_info)
{
	uint8_t const code = bitswap<7>(m_tileram[tile_index], 5, 4, 3, 2, 1, 0, 6);
	uint8_t const region = BIT(m_tileram[tile_index], 7);
	// uint8_t const color = m_attrram[tile_index];
	tileinfo.set(region, code, 0, 0);
}

void amstarz80_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(amstarz80_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 16);
}

void amstarz80_state::tileram_w(offs_t offset, uint8_t data)
{
	m_tileram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void amstarz80_state::attrram_w(offs_t offset, uint8_t data)
{
	m_attrram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

uint32_t amstarz80_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


void amstarz80_state::amstarz80_program_map(address_map &map)
{
	map(0x0000, 0x1bff).rom();
	map(0x1c00, 0x1fff).ram();
	map(0x2000, 0x21ff).ram().w(FUNC(amstarz80_state::tileram_w)).share(m_tileram);
	map(0x2400, 0x25ff).ram().w(FUNC(amstarz80_state::attrram_w)).share(m_attrram);
	map(0x2800, 0x29ff).ram(); // ??
	map(0x4000, 0x4000).lr8(NAME([this] () -> uint8_t { return (machine().rand() & 0x40) | (m_dsw3->read() & 0xbf); })); // TODO: bit 6?
	map(0x4001, 0x4001).lr8(NAME([this] () -> uint8_t { return (machine().rand() & 0x80) | 0x7f;  })); // TODO: bit 7?
	map(0x4002, 0x4002).portr("IN0");
	map(0x4003, 0x4003).portr("IN1");
	map(0x4004, 0x4004).portr("IN2");
//  map(0x4005, 0x4005).r();
	map(0x4006, 0x4006).portr("DSW1");
	map(0x4007, 0x4007).portr("DSW2");
	map(0x6000, 0x60ff).ram().share("nvram");
}

void amstarz80_state::amidon_program_map(address_map &map)
{
	amstarz80_program_map(map);

	map(0x1c00, 0x1fff).rom().unmapw();
}


static INPUT_PORTS_START( amstarz80 )
	PORT_START("DSW1")
	PORT_DIPUNKNOWN( 0x01, 0x00 ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPUNKNOWN( 0x02, 0x00 ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPUNKNOWN( 0x04, 0x00 ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPUNKNOWN( 0x08, 0x00 ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPUNKNOWN( 0x10, 0x00 ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPUNKNOWN( 0x20, 0x00 ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPUNKNOWN( 0x40, 0x00 ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPUNKNOWN( 0x80, 0x00 ) PORT_DIPLOCATION("SW1:8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN( 0x01, 0x00 ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPUNKNOWN( 0x02, 0x00 ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPUNKNOWN( 0x04, 0x00 ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPNAME(    0x08, 0x00, DEF_STR( Test ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(       0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x08, DEF_STR( On ) )         // -> $1054
	PORT_DIPUNKNOWN( 0x10, 0x00 ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPUNKNOWN( 0x20, 0x00 ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPUNKNOWN( 0x40, 0x00 ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPUNKNOWN( 0x80, 0x00 ) PORT_DIPLOCATION("SW2:8")

	PORT_START("DSW3") // holddraw checks (return & 0x0f) == 0x05, the two clones don't care
	PORT_DIPUNKNOWN(    0x01, 0x01 ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPUNKNOWN(    0x02, 0x00 ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPUNKNOWN(    0x04, 0x04 ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPUNKNOWN(    0x08, 0x00 ) PORT_DIPLOCATION("SW3:4")
	PORT_BIT(           0x70, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT(           0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank)) // ??

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Stand / Hold 5")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) // seems to have the same effect of gamble book
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // hopper line?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Draw / Take")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) // ante
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	16, 16,
	RGN_FRAC(1, 1),
	1,
	{ 0 },
	{ STEP8(7, -1), STEP8(15, -1) },
	{ STEP16(0, 16) },
	16 * 16
};

static GFXDECODE_START( gfx_amstarz80 )
	GFXDECODE_ENTRY( "tiles2", 0, charlayout, 0, 1 )
	GFXDECODE_ENTRY( "tiles", 0, charlayout, 0, 1 )
GFXDECODE_END


void amstarz80_state::amstarz80(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 10_MHz_XTAL / 4); // divisor not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &amstarz80_state::amstarz80_program_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(32 * 16, 16 * 16);
	m_screen->set_visarea_full();
	m_screen->set_screen_update(FUNC(amstarz80_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", palette_device::RGB_3BIT); // TODO PROM colours

	GFXDECODE(config, m_gfxdecode, "palette", gfx_amstarz80);

	// TODO: sound (TTL?)
}

void amstarz80_state::amidon(machine_config &config)
{
	amstarz80(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &amstarz80_state::amidon_program_map);
}


ROM_START( holddraw ) // AMSTAR ELEC ASSY 1061-3700
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "001-8000.b6", 0x0000, 0x0400, CRC(a25d17d4) SHA1(a5b9c83ace554811ac1442b44cdd72542ea2c425) )
	ROM_LOAD( "001-8100.c7", 0x0400, 0x0400, CRC(2208a37a) SHA1(8cb528c3f83f779873e82b0d959810d5e3073291) )
	ROM_LOAD( "001-8200.b7", 0x0800, 0x0400, CRC(9c27e9d7) SHA1(43103979e014ecc7b17a44a935257869ca184c2f) )
	ROM_LOAD( "001-8300.e8", 0x0c00, 0x0400, CRC(2b703868) SHA1(ac4e2903e0eeb6d248acd931ca37032bd9d928cc) )
	ROM_LOAD( "001-8400.d8", 0x1000, 0x0400, CRC(1b66fa9f) SHA1(98a1d53701a4dedeec9fa61a85dcbc0ce6910d5f) )
	ROM_LOAD( "001-8500.c8", 0x1400, 0x0400, CRC(9c219088) SHA1(7430fe4eb3a6bf1838fd5513127f5248723bec0a) )
	// one empty ROM socket at b8

	ROM_REGION( 0x800, "tiles", 0 )
	ROM_LOAD( "001_1201.e5", 0x000, 0x400, CRC(32197f7d) SHA1(07c8739033820e4e4e2fe4428f921d8fc7c8698b) )
	ROM_LOAD( "1102.d5_a",   0x400, 0x400, BAD_DUMP CRC(bb4bd952) SHA1(a869d5d5c8bc45d414b1103c6fe2b2d7bb07291e) ) // handwritten label, 3 different reads until it can be determined if one is good or a good one can be assembled
	ROM_LOAD( "1102.d5_b",   0x400, 0x400, BAD_DUMP CRC(ce57a4ee) SHA1(732fc54f32212d7590daa2da738acd1cce4d7c0d) )
	ROM_LOAD( "1102.d5_c",   0x400, 0x400, BAD_DUMP CRC(0b30d921) SHA1(992709bb0ad18f646bfa1bccea45454a5f273457) )

	// these custom ROMs were dumped via an adapter built on educated guessing of the pinout, so resulting dump isn't confirmed to be 100% correct
	// they do contain good GFX data
	ROM_REGION( 0x1000, "tiles2", 0 )
	ROM_LOAD( "c27140m.b5", 0x0000, 0x0800, CRC(78fee34e) SHA1(3cef1c03d91ec5b74272ec63c4962009cf309b52) ) // 1xxxxxxxxxxx = 0x00
	ROM_IGNORE(                     0x0800 )
	ROM_LOAD( "c27139m.b4", 0x0800, 0x0800, CRC(6e2b1b39) SHA1(61ccc2b78976633e6e8241c1f82bd63e1905b8d3) ) // 1xxxxxxxxxxx = 0x00
	ROM_IGNORE(                     0x0800 )

	ROM_REGION( 0xc00, "proms", 0 )
	ROM_LOAD( "001-1400.d2", 0x000, 0x200, CRC(32c99cfc) SHA1(64d561230d69514a02f30d7bc69caa563e069d69) )
	ROM_LOAD( "001-1500.f2", 0x200, 0x200, CRC(a27a7513) SHA1(84f5a29e55112d99a757b902aeef28e1370ef78f) )
	ROM_LOAD( "001-1300.g1", 0x400, 0x200, CRC(eb581932) SHA1(b04cf5bb18bfc91654f63984aaf8e656e616b36f) )
	ROM_LOAD( "g2.g2",       0x600, 0x200, CRC(6aa24121) SHA1(63fe82043653e753fb3d6ffb8a750b0433dae679) ) // handwritten label
	ROM_LOAD( "h2.h2",       0x800, 0x200, CRC(9096f7c8) SHA1(b51b068ae279f0cee6048415bffe14995fb7d269) ) // handwritten label
	ROM_LOAD( "j2.j2",       0xa00, 0x200, CRC(d7195174) SHA1(660e52c0d1c250ec9566d629c9e57e7b20acff26) ) // handwritten label
ROM_END

ROM_START( holddrawa ) // AMSTAR ELEC ASSY 1061-3700. Had ROM labels removed / unreadable
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "e8", 0x0000, 0x2000, CRC(7edc245b) SHA1(85e79c94386dbc618130b3afa0dc426c29e2efca) ) // on a tiny daughter-board fit in e8
	// empty ROM sockets at b6, b7, b8, c7, c8, d8

	ROM_REGION( 0x400, "tiles", 0 )
	ROM_LOAD( "2708.e5", 0x000, 0x400, CRC(8a0a90f5) SHA1(2bb2f27a3617dcf776c9ac9b713eafa3d8017c38) )
	// empty ROM socket at d5

	// these custom ROMs were dumped via an adapter built on educated guessing of the pinout, so resulting dump isn't confirmed to be 100% correct
	// they do contain good GFX data
	ROM_REGION( 0x1000, "tiles2", 0 )
	ROM_LOAD( "c27140m.b5", 0x0000, 0x0800, CRC(78fee34e) SHA1(3cef1c03d91ec5b74272ec63c4962009cf309b52) ) // 1xxxxxxxxxxx = 0x00
	ROM_IGNORE(                     0x0800 )
	ROM_LOAD( "c27139m.b4", 0x0800, 0x0800, CRC(6e2b1b39) SHA1(61ccc2b78976633e6e8241c1f82bd63e1905b8d3) ) // 1xxxxxxxxxxx = 0x00
	ROM_IGNORE(                     0x0800 )

	ROM_REGION( 0x600, "proms", 0 )
	ROM_LOAD( "d2", 0x000, 0x100, CRC(77d1cf3b) SHA1(f2891b9ea0af028c8b4f6aac254e4dfc27531da2) ) // d2 and g1 are identical?
	ROM_LOAD( "f2", 0x100, 0x100, BAD_DUMP CRC(025996b1) SHA1(16e927c3a94c46ab2d870a37aa0dfacb4f95bdbf) ) // programmer said chip is bad
	ROM_LOAD( "g1", 0x200, 0x100, CRC(77d1cf3b) SHA1(f2891b9ea0af028c8b4f6aac254e4dfc27531da2) )
	ROM_LOAD( "g2", 0x300, 0x100, CRC(6d15591d) SHA1(424397e1d0f5c5aa203ce74b0f0ac3bedcb090bc) )
	ROM_LOAD( "h2", 0x400, 0x100, CRC(5751f3ac) SHA1(5e0b5623688752ea18f8892c38682bc665524a16) )
	ROM_LOAD( "j2", 0x500, 0x100, CRC(1ae17610) SHA1(8c41f936f2a21cdd63a68eb43b86897dad309255) )
	// empty sockets at j1 and g11
ROM_END

ROM_START( holddrawb ) // AMSTAR ELEC ASSY 1061-3700. Had ROM labels removed / unreadable
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "e8", 0x0000, 0x2000, CRC(e1f86655) SHA1(41b7e8cb7bc1c7baabaf2c32259f07c905a95902) ) // on a tiny daughter-board fit in e8
	// empty ROM sockets at b6, b7, b8, c7, c8, d8

	ROM_REGION( 0x400, "tiles", 0 )
	ROM_LOAD( "2708.e5", 0x000, 0x400, CRC(8a0a90f5) SHA1(2bb2f27a3617dcf776c9ac9b713eafa3d8017c38) )
	// empty ROM socket at d5

	// these custom ROMs were dumped via an adapter built on educated guessing of the pinout, so resulting dump isn't confirmed to be 100% correct
	// they do contain good GFX data
	ROM_REGION( 0x1000, "tiles2", 0 )
	ROM_LOAD( "c29114.b5",  0x0000, 0x0800, CRC(78fee34e) SHA1(3cef1c03d91ec5b74272ec63c4962009cf309b52) ) // 1xxxxxxxxxxx = 0x00,  different chip code but same contents as the other sets
	ROM_IGNORE(                     0x0800 )
	ROM_LOAD( "c27139m.b4", 0x0800, 0x0800, CRC(6e2b1b39) SHA1(61ccc2b78976633e6e8241c1f82bd63e1905b8d3) ) // 1xxxxxxxxxxx = 0x00
	ROM_IGNORE(                     0x0800 )

	ROM_REGION( 0x600, "proms", 0 )
	ROM_LOAD( "d2",          0x000, 0x100, CRC(77d1cf3b) SHA1(f2891b9ea0af028c8b4f6aac254e4dfc27531da2) )
	ROM_LOAD( "f2",          0x100, 0x100, CRC(a33b5045) SHA1(e7eb46c2f75a6a754d023542c4b229f27058a79b) )
	ROM_LOAD( "g1",          0x200, 0x100, CRC(7238328c) SHA1(8693aa21f0ede154f0fc17abc1c4f392de4b8018) )
	ROM_LOAD( "001_1602.g2", 0x300, 0x100, CRC(8364f735) SHA1(888376546430a9db223893896014ecc95e40d566) )
	ROM_LOAD( "h2",          0x400, 0x100, CRC(5274d0b7) SHA1(a5b429c52434605b9dce336d55a4ffa4f47f3c1d) )
	ROM_LOAD( "j2",          0x500, 0x100, CRC(3333f7bc) SHA1(d4f346ad853c6e46163da651f2bea4730e5774fc) )
	// empty sockets at j1 and g11
ROM_END


void amstarz80_state::init_gfx(const char *tag)
{
	// De-interleave the character generator. In the ROM every 64-byte block
	// holds two 16x16x1bpp glyphs interleaved one row (2 bytes) at a time:
	//   [even row0][odd row0][even row1][odd row1]...[even row15][odd row15]
	// Rearrange into 256 contiguous 32-byte glyphs so charlayout can read them.
	uint8_t *const rom = memregion(tag)->base();
	const size_t len = memregion(tag)->bytes();
	std::vector<uint8_t> buf(len);

	for (size_t block = 0; block < len; block += 64)
	{
		for (int row = 0; row < 16; row++)
		{
			// even glyph -> first 32 bytes, odd glyph -> next 32 bytes
			buf[block + row * 2 + 0]      = rom[block + row * 4 + 0];
			buf[block + row * 2 + 1]      = rom[block + row * 4 + 1];
			buf[block + 32 + row * 2 + 0] = rom[block + row * 4 + 2];
			buf[block + 32 + row * 2 + 1] = rom[block + row * 4 + 3];
		}
	}

	std::copy(buf.begin(), buf.end(), rom);
}

void amstarz80_state::init_amstarz80()
{
	init_gfx("tiles");
	init_gfx("tiles2");
}


} // anonymous namespace


GAME( 1981, holddraw,  0,        amstarz80, amstarz80, amstarz80_state, init_amstarz80, ROT0, "Amstar", "Hold & Draw",                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // supposedly, but might actually be another similar game
GAME( 198?, holddrawa, holddraw, amidon,    amstarz80, amstarz80_state, init_amstarz80, ROT0, "Amidon", "Hold & Draw (Amidon, set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 198?, holddrawb, holddraw, amidon,    amstarz80, amstarz80_state, init_amstarz80, ROT0, "Amidon", "Hold & Draw (Amidon, set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
