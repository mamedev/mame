// license:BSD-3-Clause
// copyright-holders:

/*
    Black Tiger (Modular System)

    As with most of the 'Modular System' setups, the hardware is heavily modified from the original
    and consists of a multi-board stack in a cage, hence different driver.

    The Modular System cage contains 6 main boards for this game.

    MOD3-4 - Z80 board (CPU + 5 ROMs + RAM + 24MHz XTAL).
    MOD21/1 - RAM board? 20 MHz XTAL.
    MOD1/5 - Sound board (Z80, 2xYM2203C). 2 8-dips banks.
    MOD51/1 - Sprite board, has logic + 4 sprite ROMs.
    MOD 4/3 - Tilemap board, has logic + 2 tilemap ROMs, long thin sub-board (CAR-0484/1 SOLD) with no chips, just routing along one edge.
    MOD 4/3 - Tilemap board, has logic + 4 tilemap ROMs, long thin sub-board (CAR-0484/1 SOLD) with no chips, just routing along one edge.

    TODO: practically everything:
    - derive from blktiger_state? (eventually, if differences aren't too much)
    - chars and sprites related functions are copied from blktiger.cpp. Verify differences
    - correct GFX decoding
    - tiles
    - palette
    - banking: seems to work by accident, tweaking the ROM loading. Pretty sure it's wrong.
    - where are the sound latch and the other functions that the original has mapped in the io space?
    - lots of other stuff
*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/ymopn.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class blktiger_ms_state : public driver_device
{
public:
	blktiger_ms_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_txvideoram(*this, "txvideoram"),
		m_spriteram(*this, "spriteram")
	{ }

	void blktigerm(machine_config &config);

	void init_blktigerm();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint8_t> m_txvideoram;
	required_shared_ptr<uint8_t> m_spriteram;

	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	void txvideoram_w(offs_t offset, uint8_t data);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	tilemap_t *m_tx_tilemap = nullptr;
};


void blktiger_ms_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0xbfff).bankr("mainbank");
	map(0xc000, 0xcfff).ram();
	map(0xc000, 0xc000).portr("IN0");
	map(0xc001, 0xc001).portr("IN1");
	map(0xc002, 0xc002).portr("IN2");
	map(0xc003, 0xc003).portr("DSW0");
	map(0xc004, 0xc004).portr("DSW1");
	map(0xd000, 0xd7ff).ram().w(FUNC(blktiger_ms_state::txvideoram_w)).share(m_txvideoram);
	map(0xd800, 0xd800).lw8(NAME([this] (u8 data) { membank("mainbank")->set_entry(data & 0x0f); })); // very probably wrong
	map(0xe000, 0xfdff).ram();
	map(0xfe00, 0xffff).ram().share(m_spriteram);
}

void blktiger_ms_state::sound_map(address_map &map) // seems similar to toki_ms.cpp and raiden_ms.cpp
{
	map(0x0000, 0x7fff).rom().region("audiocpu", 0);
	map(0xc000, 0xc7ff).ram();
	map(0xc900, 0xc900).noprw(); // what lives here?
	map(0xdff8, 0xdff8).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0xdff0, 0xdfff).nopw(); // what lives here?
	map(0xe000, 0xe001).w("ym1", FUNC(ym2203_device::write));
	map(0xe002, 0xe003).w("ym2", FUNC(ym2203_device::write));
	map(0xe008, 0xe009).r("ym1", FUNC(ym2203_device::read));
	map(0xe00a, 0xe00b).r("ym2", FUNC(ym2203_device::read));
}

void blktiger_ms_state::machine_start()
{
	membank("mainbank")->configure_entries(0, 16, memregion("maincpu")->base() + 0x10000, 0x4000);
}

void blktiger_ms_state::video_start()
{
	m_tx_tilemap =  &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(blktiger_ms_state::get_tx_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

void blktiger_ms_state::txvideoram_w(offs_t offset, uint8_t data)
{
	m_txvideoram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset & 0x3ff);
}

void blktiger_ms_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int attr = m_spriteram[offs + 1];
		int sx = m_spriteram[offs + 3] - ((attr & 0x10) << 4);
		int sy = m_spriteram[offs + 2];
		int code = m_spriteram[offs] | ((attr & 0xe0) << 3);
		int color = attr & 0x07;
		int flipx = attr & 0x08;

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
				code,
				color,
				flipx, flip_screen(),
				sx, sy, 15);
	}
}

uint32_t blktiger_ms_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	draw_sprites(bitmap, cliprect);

	return 0;
}

TILE_GET_INFO_MEMBER(blktiger_ms_state::get_tx_tile_info)
{
	uint8_t attr = m_txvideoram[tile_index + 0x400];
	tileinfo.set(0,
			m_txvideoram[tile_index] + ((attr & 0xe0) << 3),
			attr & 0x1f,
			0);
}

static INPUT_PORTS_START( blktigerm )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION( "SW1:1,2,3" )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )       PORT_DIPLOCATION( "SW1:4,5,6" )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION( "SW1:7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Test ) )         PORT_DIPLOCATION( "SW1:8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION( "SW2:1,2" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7")
	PORT_DIPNAME( 0x1c, 0x0c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION( "SW2:3,4,5" )
	PORT_DIPSETTING(    0x1c, "1 (Easiest)")
	PORT_DIPSETTING(    0x18, "2" )
	PORT_DIPSETTING(    0x14, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x0c, "5 (Normal)" )
	PORT_DIPSETTING(    0x08, "6" )
	PORT_DIPSETTING(    0x04, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hardest)" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION( "SW2:6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION( "SW2:7" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION( "SW2:8" )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout tiles16x16x4_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,8,16,24 },
	{ 0,1,2,3,4,5,6,7, 512,513,514,515,516,517,518,519 },
	{ STEP16(0,32) },
	16 * 16 * 4
};

static GFXDECODE_START( gfx_blktiger_ms )
	GFXDECODE_ENTRY( "chars", 0, charlayout, 0x300, 32 )
	GFXDECODE_ENTRY( "sprites", 0, tiles16x16x4_layout, 0x100, 32 )
GFXDECODE_END

void blktiger_ms_state::blktigerm(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 24_MHz_XTAL / 4); // divisor unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &blktiger_ms_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(blktiger_ms_state::irq0_line_hold));

	z80_device &audiocpu(Z80(config, "audiocpu", 24_MHz_XTAL / 8)); // divisor unknown, no XTAL on the PCB, might also use the 20 MHz one
	audiocpu.set_addrmap(AS_PROGRAM, &blktiger_ms_state::sound_map);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER); // all wrong
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(blktiger_ms_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xBRG_444, 1024);

	GFXDECODE(config, "gfxdecode", "palette", gfx_blktiger_ms);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	YM2203(config, "ym1", 24_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown, no XTAL on the PCB, might also use the 20 MHz one

	YM2203(config, "ym2", 24_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown, no XTAL on the PCB, might also use the 20 MHz one
}

ROM_START( blktigerm )
	ROM_REGION( 0x50000, "maincpu", 0 ) // on MOD 3/4 board
	ROM_LOAD( "3_bl_301.ic14",   0x00000, 0x08000, CRC(b4525312) SHA1(958ce9a7f41422f3011412e4a16dd3ad65019733) )
	ROM_LOAD( "3_bl_304.ic18",   0x10000, 0x10000, CRC(36f669c6) SHA1(e91fd222919904d4d4b3ef70a22ab599a7cfa263) )
	ROM_LOAD( "3_bl_305.ic19",   0x20000, 0x10000, CRC(b30a99af) SHA1(19d6dbe11ffeb9a21dbfe75eb39ce9ceaef5eb31) )
	ROM_LOAD( "3_bl_302_c.ic13", 0x30000, 0x10000, CRC(c943415f) SHA1(08e82d2557de01c979a0a58b5433e9bdcdb118cb) )
	ROM_LOAD( "3_bl_303_b.ic12", 0x40000, 0x10000, CRC(72114f6b) SHA1(049e747431cd95db2e7414467754424f68a8d757) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "1_bl_101.ic12",  0x0000, 0x8000, CRC(14028686) SHA1(64dc219d906f1bdd0c2bc05aff5aa73e001a6901) )

	ROM_REGION( 0x10000, "chars", ROMREGION_INVERT ) // on one of the MOD 4/3 boards, both ROMs 0xxxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "4_bl_401.ic17",  0x0000, 0x8000, CRC(ab1afb3d) SHA1(555332ccfb69e65b776f94ffac9a4a051fb6f09e) )
	ROM_LOAD( "4_bl_402.ic16",  0x8000, 0x8000, CRC(89445b11) SHA1(7d9ab6e88d7de3a0e31b3b8a5e57dd39afd7940d) )

	ROM_REGION( 0x40000, "tiles", 0 ) // on the other MOD 4/3 board
	ROM_LOAD32_BYTE( "4_a_bl_4a01.ic17",  0x00003, 0x10000, CRC(fc00fd6c) SHA1(066714421fa782c6cfad9f695f05e6e3551ffdc7) )
	ROM_LOAD32_BYTE( "4_a_bl_4a02.ic16",  0x00002, 0x10000, CRC(49cd8afa) SHA1(147b65dad4f05b940d1f7a4028f5748ba1b7daa5) )
	ROM_LOAD32_BYTE( "4_a_bl_4a03.ic15",  0x00001, 0x10000, CRC(50207dd4) SHA1(bee343fbefc817fa14b8b303f013bf5b89e911f8) )
	ROM_LOAD32_BYTE( "4_a_bl_4a04.ic14",  0x00000, 0x10000, CRC(4bb0c1ba) SHA1(9433c78f0de7cb1f92b22612b9f3c51d813b025a) )

	ROM_REGION( 0x40000, "sprites", ROMREGION_INVERT ) // on MOD 51/1 board
	ROM_LOAD32_BYTE( "51_bl_501.ic43",   0x00003, 0x10000, CRC(ace32b94) SHA1(4a09e8dc73bd16f7d378aefbe5d433dd1396f97d) )
	ROM_LOAD32_BYTE( "51_bl_502.ic42",   0x00002, 0x10000, CRC(f6c4cc0b) SHA1(678fd71e90237f8e19eae78fe150a4c5d3494c6c) )
	ROM_LOAD32_BYTE( "51_bl_503.ic41",   0x00001, 0x10000, CRC(61e3ae0b) SHA1(cbb83827f027acd6b905f4210476810bcd4b03a9) )
	ROM_LOAD32_BYTE( "51_bl_504.ic40",   0x00000, 0x10000, CRC(2cb45034) SHA1(b2fd3b7b7b9d68b6ff1985166b106e65b17dbb23) )

	ROM_REGION( 0x0700, "proms", 0 )    // PROMs (function unknown)
	ROM_LOAD( "1_p0101_82s123.ic20",       0x0000, 0x0020, CRC(3fd60d3a) SHA1(8100fa7453638ac40193b5d92404f41b101ed2cc) )
	ROM_LOAD( "21_p0201_82s129.ic4",       0x0100, 0x0100, CRC(2697da58) SHA1(e62516b886ff6e204b718e5f0c6ce2712e4b7fc5) )
	ROM_LOAD( "21_p0202_82s129.ic12",      0x0200, 0x0100, CRC(e434128a) SHA1(ef0f6d8daef8b25211095577a182cdf120a272c1) )
	ROM_LOAD( "3_subcpu_p0322_82s147.bin", 0x0300, 0x0200, CRC(6d427336) SHA1(6ce1ef41df2b450fafe11c3021991618bdb771ed) )
	ROM_LOAD( "4_a_bl_82s123.ic13",        0x0500, 0x0020, CRC(671f1183) SHA1(dbc21c3922c5e69340daa9008e1200f1304b3e8f) )
	ROM_LOAD( "51_p0502_82s129.ic10",      0x0600, 0x0100, CRC(15085e44) SHA1(646e7100fcb112594023cf02be036bd3d42cc13c) )

	ROM_REGION( 0x1000, "plds", ROMREGION_ERASEFF )
	ROM_LOAD( "4_p0403_pal16r8a-2cn.ic29",      0x000, 0x104, CRC(506156cc) SHA1(5560671fc2c9872ed28620491af5dc486909fc6e) )
	ROM_LOAD( "4_a_p0403_pal16r8a-2cn.ic29",    0x000, 0x104, CRC(506156cc) SHA1(5560671fc2c9872ed28620491af5dc486909fc6e) ) // identical to the above one (same PCB type)
	ROM_LOAD( "51_p0503_pal16r8a.ic56",         0x000, 0x104, CRC(07eb86d2) SHA1(482eb325df5bc60353bac85412cf45429cd03c6d) )
ROM_END

void blktiger_ms_state::init_blktigerm()
{
	uint8_t *const src = memregion("maincpu")->base();
	int const len = 0x50000;

	// bitswap data
	for (int i = 0; i < len; i++)
		src[i] = bitswap<8>(src[i], 5, 3, 7, 2, 0, 1, 4, 6);

	// descramble address
	std::vector<uint8_t> buffer(len);
	memcpy(&buffer[0], src, len);

	for (int i = 0; i < len; i++)
		src[i] = buffer[bitswap<24>(i, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 10, 8, 6, 4, 0, 2, 7, 3, 1, 9, 11, 5)];
}

} // Anonymous namespace


GAME( 1991, blktigerm,  blktiger,  blktigerm,  blktigerm,  blktiger_ms_state, init_blktigerm, ROT0, "bootleg (Gaelco / Ervisa)", "Black Tiger (Modular System)", MACHINE_IS_SKELETON )
