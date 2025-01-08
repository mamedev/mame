// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Pass (c)1992, Oksan

 Driver by David Haywood
 Inputs by Stephh

 Is that a Korean flag I see?  Are Oksan maybe a Korean Developer?

 Information from ReadMe
 -----------------------

 Pass, Oksan, 1992
 CPU : MC68000P10
 Sound : YM2203C
 Other main chips : Goldstar Z8400B PS, 4 * Hyundai HY6264ALP-10
 Also : PAL20L8ACNS
 The rest all TTl chips

 8 dipswitches (1 bank)
 speed : 14.31818MHz
 Rest see pic
 (included was scans of the board)

 ----------------------

 Working Notes:

 68k interrupts
 lev 1 : 0x64 : 0000 0500 - vblank?
 lev 2 : 0x68 : 0000 0500 - vblank?
 lev 3 : 0x6c : 0000 0500 - vblank?
 lev 4 : 0x70 : 0000 0500 - vblank?
 lev 5 : 0x74 : 0000 0500 - vblank?
 lev 6 : 0x78 : 0000 0500 - vblank?
 lev 7 : 0x7c : 0000 0500 - vblank?
    (all point to the same place ..)

 z80 interrupts
 0x38 looks to have a valid IRQ
 0x66 might be valid NMI

 -- stephh's notes on the inputs --

 reads from 0x230100.w :

   0x001066 : mask = 0xe000 (coinage)

   0x00124e : mask = 0x0300 (player 1 lives)
   0x001292 : mask = 0x0300 (player 2 lives)

   0x0046ea : mask = 0x0001 (unknown effect - flip ? demo sounds ?) ->

   0x004182 : mask = 0x1800 (time, difficulty)


 reads from 0x230200.w :

   0x001000 : mask = 0xffff -> >>  0 in 0x080010
   0x001000 : mask = 0x00f0 -> >>  4 in 0x080016 (player 1 directions)
   0x001000 : mask = 0xf000 -> >> 12 in 0x080018 (player 2 directions)


 0x080010.w : inputs

   bit 00 : COIN1
   bit 01 : START1
   bit 02 : tested at 0x002bca, 0x002f00, 0x004c26, 0x00c25e, 0x001f48, 0x00c474, 0x00c628
   bit 03 : tested at 0x002bca, 0x002f00, 0x004c4c, 0x00c2ce, 0x00c644
   bit 04 : UP
   bit 05 : DOWN
   bit 06 : LEFT
   bit 07 : RIGHT

   bit 08 : COIN2
   bit 09 : START2
   bit 10 : tested at 0x002bca, 0x002f00, 0x004c8a, 0x00c2a8, 0x0021b8
   bit 11 : tested at 0x002bca, 0x002f00, 0x004cb0, 0x00c282
   bit 12 : UP
   bit 13 : DOWN
   bit 14 : LEFT
   bit 15 : RIGHT


 0x080014.w : credits (max = 0x005a)

 0x08007e.w : lives (player 1)
 0x080080.w : lives (player 2)

 0x080002.w : time (0x0000-0x0099, BCD coded)

 --- Game Notes ---

 Graphical glitches caused when 2 sprites are close together are NOT bugs, the sprites are
 in fact constructed from a tilemap made of 4x4 tiles.

 I imagine flicker on the main character at times is also correct.

 It's rather interesting to see a game this old using 8bpp tiles


 */

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class pass_state : public driver_device
{
public:
	pass_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bg_tilemap(nullptr),
		m_fg_tilemap(nullptr),
		m_bg_videoram(*this, "bg_videoram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode")
	{
	}

	void pass(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	void bg_videoram_w(offs_t offset, uint16_t data);
	void fg_videoram_w(offs_t offset, uint16_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;

	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_fg_videoram;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};


// background tilemap stuff

TILE_GET_INFO_MEMBER(pass_state::get_bg_tile_info)
{
	int tileno = m_bg_videoram[tile_index] & 0x1fff;
	int fx = (m_bg_videoram[tile_index] & 0xc000) >> 14;
	tileinfo.set(1, tileno, 0, TILE_FLIPYX(fx));

}

void pass_state::bg_videoram_w(offs_t offset, uint16_t data)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

// foreground 'sprites' tilemap stuff

TILE_GET_INFO_MEMBER(pass_state::get_fg_tile_info)
{
	int tileno = m_fg_videoram[tile_index] & 0x3fff;
	int flip = (m_fg_videoram[tile_index] & 0xc000) >>14;

	tileinfo.set(0, tileno, 0, TILE_FLIPYX(flip));

}

void pass_state::fg_videoram_w(offs_t offset, uint16_t data)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

// video update / start

void pass_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(pass_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8,  64, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(pass_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 4, 4, 128, 64);

	m_fg_tilemap->set_transparent_pen(255);
}

uint32_t pass_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


// TODO: check all memory regions actually readable / read from
void pass_state::main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x080000, 0x083fff).ram();
	map(0x200000, 0x200fff).ram().w(FUNC(pass_state::bg_videoram_w)).share(m_bg_videoram);
	map(0x210000, 0x213fff).ram().w(FUNC(pass_state::fg_videoram_w)).share(m_fg_videoram);
	map(0x220000, 0x2203ff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x230001, 0x230001).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x230100, 0x230101).portr("DSW");
	map(0x230200, 0x230201).portr("INPUTS");
}

void pass_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xf800, 0xffff).ram();
}

void pass_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x70, 0x71).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x80, 0x80).w("oki", FUNC(okim6295_device::write));
	map(0xc0, 0xc0).w("soundlatch", FUNC(generic_latch_8_device::clear_w));
}

// TODO : work out function of unknown but used dsw
static INPUT_PORTS_START( pass )
	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "Unknown SW 0-0" )    // USED ! Check code at 0x0046ea
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Unused SW 0-1" )     // Unused ?
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Unused SW 0-2" )     // Unused ?
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Unused SW 0-3" )     // Unused ?
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Unused SW 0-4" )     // Unused ?
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Unused SW 0-5" )     // Unused ?
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Unused SW 0-6" )     // Unused ?
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Unused SW 0-7" )     // Unused ?
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0100, "4" )
	PORT_DIPSETTING(      0x0200, "5" )
	PORT_DIPNAME( 0x0400, 0x0400, "Unused SW 0-10" )    // Unused ?
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1800, 0x0000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )         // Time = 99
	PORT_DIPSETTING(      0x1800, DEF_STR( Normal ) )       // Time = 88
	PORT_DIPSETTING(      0x0800, DEF_STR( Hard ) )         // Time = 77
	PORT_DIPSETTING(      0x1000, DEF_STR( Hardest ) )      // Time = 66
	PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )

	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
INPUT_PORTS_END

// for something so simple this took a while to see
static const gfx_layout tiles4x4_fg_layout =
{
	4,4,
	RGN_FRAC(1,1),
	8,
	{ 0,1, 2,3, 4,5,6,7 },
	{ 0, 8, 16, 24 },
	{ 0*32, 1*32, 2*32, 3*32 },
	4*32
};

static GFXDECODE_START( gfx_pass )
	GFXDECODE_ENTRY( "fgtiles", 0, tiles4x4_fg_layout, 256, 2 )
	GFXDECODE_ENTRY( "bgtiles", 0, gfx_8x8x8_raw, 0, 2 )
GFXDECODE_END

// TODO : is this correct?
void pass_state::pass(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 14'318'180 / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &pass_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(pass_state::irq1_line_hold)); // all the same

	z80_device &audiocpu(Z80(config, "audiocpu", 14'318'180 / 4));
	audiocpu.set_addrmap(AS_PROGRAM, &pass_state::sound_map);
	audiocpu.set_addrmap(AS_IO, &pass_state::sound_io_map);
	audiocpu.set_periodic_int(FUNC(pass_state::irq0_line_hold), attotime::from_hz(60)); // probably not accurate, unknown timing and generation (ym2203 sound chip?).

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(8*8, 48*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(pass_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 0x200);
	GFXDECODE(config, m_gfxdecode, "palette", gfx_pass);


	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	YM2203(config, "ymsnd", 14'318'180 / 4).add_route(ALL_OUTPUTS, "mono", 0.60);

	OKIM6295(config, "oki", 792'000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.60); // clock frequency & pin 7 not verified
}


ROM_START( pass )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68k
	ROM_LOAD16_BYTE( "33_3.u1", 0x00001, 0x20000, CRC(0c5f18f6) SHA1(49b60d46e4149ad1d49b044522a6888737c17e7d) )
	ROM_LOAD16_BYTE( "34_4.u2", 0x00000, 0x20000, CRC(7b54573d) SHA1(251e99fa1f045ae4c90676e1953e49e8191440e4) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // z80 clone?
	ROM_LOAD( "32_2.u202", 0x00000, 0x10000, CRC(b9a0ccde) SHA1(33e7dda247aa44b1933ae9c033c161c152276ce6) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "31_1.u210", 0x00000, 0x20000, CRC(c7315bbd) SHA1(c0bb392793cafc7b3f76da8fb26c6c16948f87e5) )

	ROM_REGION( 0x40000, "fgtiles", 0 )
	ROM_LOAD16_BYTE( "35_5.u512", 0x00000, 0x20000, CRC(2ab33f07) SHA1(23f2481450b3f43bbe3856c4cf595af74b1da2e0) )
	ROM_LOAD16_BYTE( "36_6.u511", 0x00001, 0x20000, CRC(6677709d) SHA1(0d3df11097855294d606e46c0db0cf801c1dc28a) )

	ROM_REGION( 0x80000, "bgtiles", 0 )
	ROM_LOAD16_BYTE( "38_8.u421",  0x00000, 0x20000, CRC(7f11b81a) SHA1(50253da7c13f9390fe7afd2faf17b8057f0bee1b) )
	ROM_LOAD16_BYTE( "40_10.u420", 0x00001, 0x20000, CRC(80e0a71d) SHA1(e62c855f357e7492a59f8719c62a16d418dfa60b) )
	ROM_LOAD16_BYTE( "37_7.u425",  0x40000, 0x20000, CRC(296499e7) SHA1(b7727f7942e20a2428df84e99075a572189a0096) )
	ROM_LOAD16_BYTE( "39_9.u426",  0x40001, 0x20000, CRC(35c0ad5c) SHA1(78e3ca8b2e382a3c7bc53ede2ef5611c520ab095) )
ROM_END

} // anonymous namespace


GAME( 1992, pass, 0, pass, pass, pass_state, empty_init, ROT0, "Oksan", "Pass", MACHINE_SUPPORTS_SAVE )
