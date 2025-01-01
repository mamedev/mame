// license:BSD-3-Clause
// copyright-holders:Angelo Salese, David Haywood, Pierpaolo Prazzoli
/**************************************************************************************************

Best League (c) 1993

A Big Striker Italian bootleg (made by Playmark?) running on a different hardware.

**************************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class bestleag_state : public driver_device
{
public:
	bestleag_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_vregs(*this, "vregs")
		, m_maincpu(*this, "maincpu")
		, m_oki(*this, "oki")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_bgram(*this, "bgram")
		, m_fgram(*this, "fgram")
		, m_txram(*this, "txram")
		, m_spriteram(*this, "spriteram")
	{ }

	void bestleag(machine_config &config) ATTR_COLD;

protected:
	tilemap_t *m_tx_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_shared_ptr<uint16_t> m_vregs;

private:
	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint16_t> m_bgram;
	required_shared_ptr<uint16_t> m_fgram;
	required_shared_ptr<uint16_t> m_txram;
	required_shared_ptr<uint16_t> m_spriteram;


	void txram_w(offs_t offset, uint16_t data);
	void bgram_w(offs_t offset, uint16_t data);
	void fgram_w(offs_t offset, uint16_t data);
	void oki_bank_w(uint16_t data);

	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILEMAP_MAPPER_MEMBER(bg_scan);

	virtual void video_start() override ATTR_COLD;

	void main_map(address_map &map) ATTR_COLD;
};

class bestleaw_state : public bestleag_state
{
public:
	bestleaw_state(const machine_config &mconfig, device_type type, const char *tag)
		: bestleag_state(mconfig, type, tag)
	{ }

protected:
	virtual uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) override;
};


TILE_GET_INFO_MEMBER(bestleag_state::get_tx_tile_info)
{
	int code = m_txram[tile_index];

	tileinfo.set(0,
			(code & 0x0fff)|0x8000,
			(code & 0xf000) >> 12,
			0);
}

TILE_GET_INFO_MEMBER(bestleag_state::get_bg_tile_info)
{
	int code = m_bgram[tile_index];

	tileinfo.set(1,
			(code & 0x0fff),
			(code & 0xf000) >> 12,
			0);
}

TILE_GET_INFO_MEMBER(bestleag_state::get_fg_tile_info)
{
	int code = m_fgram[tile_index];

	tileinfo.set(1,
			(code & 0x0fff)|0x1000,
			((code & 0xf000) >> 12)|0x10,
			0);
}

TILEMAP_MAPPER_MEMBER(bestleag_state::bg_scan)
{
	int offset;

	offset = ((col&0xf)*16) + (row&0xf);
	offset += (col >> 4) * 0x100;
	offset += (row >> 4) * 0x800;

	return offset;
}

void bestleag_state::video_start()
{
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bestleag_state::get_tx_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 256, 32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bestleag_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(bestleag_state::bg_scan)), 16, 16, 128, 64);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bestleag_state::get_fg_tile_info)), tilemap_mapper_delegate(*this, FUNC(bestleag_state::bg_scan)), 16, 16, 128, 64);

	m_tx_tilemap->set_transparent_pen(15);
	m_fg_tilemap->set_transparent_pen(15);
}

/*
 * Sprites are the same to sslam, but using 16x16 sprites instead of 8x8,
 * moved start address to 0x16/2?
 */
void bestleag_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0x16/2; offs < m_spriteram.length() - 3; offs += 4)
	{
		int code = m_spriteram[offs+3] & 0xfff;
		int color = (m_spriteram[offs+2] & 0xf000) >> 12;
		int sx = (m_spriteram[offs+2] & 0x1ff) - 20;
		int sy = (0xff - (m_spriteram[offs+0] & 0xff)) - 15;
		int flipx = (m_spriteram[offs+0] & 0x4000) >> 14;

		/* Sprite list end code */
		if(m_spriteram[offs+0] & 0x2000)
			return;

		/* it can change sprites color mask like the original set */
		if(m_vregs[0x00/2] & 0x1000)
			color &= 7;

		m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
					code,
					color,
					flipx, 0,
					flipx ? (sx+16) : (sx),sy,15);

		m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
					code+1,
					color,
					flipx, 0,
					flipx ? (sx) : (sx+16),sy,15);

		/* wraparound x */
		m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
					code,
					color,
					flipx, 0,
					flipx ? (sx+16 - 512) : (sx - 512),sy,15);

		m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
					code+1,
					color,
					flipx, 0,
					flipx ? (sx - 512) : (sx+16 - 512),sy,15);
	}
}

uint32_t bestleag_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0,(m_vregs[0x00/2] & 0xfff) + (m_vregs[0x08/2] & 0x7) - 3);
	m_bg_tilemap->set_scrolly(0,m_vregs[0x02/2]);
	m_tx_tilemap->set_scrollx(0,m_vregs[0x04/2]);
	m_tx_tilemap->set_scrolly(0,m_vregs[0x06/2]);
	m_fg_tilemap->set_scrollx(0,m_vregs[0x08/2] & 0xfff8);
	m_fg_tilemap->set_scrolly(0,m_vregs[0x0a/2]);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap,cliprect);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}

uint32_t bestleaw_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0,m_vregs[0x08/2]);
	m_bg_tilemap->set_scrolly(0,m_vregs[0x0a/2]);
	m_tx_tilemap->set_scrollx(0,m_vregs[0x00/2]);
	m_tx_tilemap->set_scrolly(0,m_vregs[0x02/2]);
	m_fg_tilemap->set_scrollx(0,m_vregs[0x04/2]);
	m_fg_tilemap->set_scrolly(0,m_vregs[0x06/2]);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap,cliprect);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}

void bestleag_state::txram_w(offs_t offset, uint16_t data)
{
	m_txram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset);
}

void bestleag_state::bgram_w(offs_t offset, uint16_t data)
{
	m_bgram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void bestleag_state::fgram_w(offs_t offset, uint16_t data)
{
	m_fgram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void bestleag_state::oki_bank_w(uint16_t data)
{
	m_oki->set_rom_bank((data - 1) & 3);
}



void bestleag_state::main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x0d2000, 0x0d3fff).noprw(); // left over from the original game (only read / written in memory test)
	map(0x0e0000, 0x0e3fff).ram().w(FUNC(bestleag_state::bgram_w)).share("bgram");
	map(0x0e8000, 0x0ebfff).ram().w(FUNC(bestleag_state::fgram_w)).share("fgram");
	map(0x0f0000, 0x0f3fff).ram().w(FUNC(bestleag_state::txram_w)).share("txram");
	map(0x0f8000, 0x0f800b).ram().share("vregs");
	map(0x100000, 0x100fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x200000, 0x200fff).ram().share("spriteram");
	map(0x300010, 0x300011).portr("SYSTEM");
	map(0x300012, 0x300013).portr("P1");
	map(0x300014, 0x300015).portr("P2");
	map(0x300016, 0x300017).portr("DSWA");
	map(0x300018, 0x300019).portr("DSWB");
	map(0x30001c, 0x30001d).w(FUNC(bestleag_state::oki_bank_w));
	map(0x30001f, 0x30001f).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x304000, 0x304001).nopw();
	map(0xfe0000, 0xffffff).ram();
}

#define BESTLEAG_PLAYER_INPUT( player ) \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(player) \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(player) \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(player) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

static INPUT_PORTS_START( bestleag )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P1")
	BESTLEAG_PLAYER_INPUT( 1 )

	PORT_START("P2")
	BESTLEAG_PLAYER_INPUT( 2 )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW.A:1,2,3,4")
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )    // also set "Coin B" to "Free Play"
	/* 0x01 to 0x05 gives 2C_3C */
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW.A:5,6,7,8")
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )    // also set "Coin A" to "Free Play"
	/* 0x10 to 0x50 gives 2C_3C */

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW.B:1") // Doesn't work ?
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW.B:2,3")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x18, 0x18, "Timer Speed" ) PORT_DIPLOCATION("SW.B:4,5")
	PORT_DIPSETTING(    0x08, "Slow" )              // 65
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )   // 50
	PORT_DIPSETTING(    0x10, "Fast" )              // 35
	PORT_DIPSETTING(    0x00, "Fastest" )           // 25
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW.B:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "2 Players Game" ) PORT_DIPLOCATION("SW.B:7")
	PORT_DIPSETTING(    0x40, "1 Credit" )
	PORT_DIPSETTING(    0x00, "2 Credits" )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW.B:8")
INPUT_PORTS_END


static const gfx_layout bestleag_charlayout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ STEP8(0, 1) },
	{ 0*8, 2*8, 4*8, 6*8, 1*8, 3*8, 5*8, 7*8},
	8*8
};

static const gfx_layout bestleag_char16layout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ STEP8(0, 1), STEP8(128, 1) },
	{ STEP16(0, 8) },
	16*16
};

static GFXDECODE_START( gfx_bestleag )
	GFXDECODE_ENTRY( "gfx1", 0, bestleag_charlayout,     0x200, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, bestleag_char16layout,   0x000, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, bestleag_char16layout,   0x300, 16 )
GFXDECODE_END

void bestleag_state::bestleag(machine_config &config)
{
	M68000(config, m_maincpu, 12000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &bestleag_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(bestleag_state::irq6_line_hold));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(bestleag_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_bestleag);
	PALETTE(config, m_palette).set_format(palette_device::RRRRGGGGBBBBRGBx, 0x800);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	OKIM6295(config, m_oki, 1000000, okim6295_device::PIN7_HIGH); /* Hand-tuned */
	m_oki->add_route(ALL_OUTPUTS, "lspeaker", 1.00);
	m_oki->add_route(ALL_OUTPUTS, "rspeaker", 1.00);
}


ROM_START( bestleag )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "2.bin",  0x00000, 0x20000, CRC(d2be3431) SHA1(37815c80b9fbc246fcdaa202d40fb40b10f55b45) ) // sldh
	ROM_LOAD16_BYTE( "3.bin",  0x00001, 0x20000, CRC(f29c613a) SHA1(c66fa53f38bfa77ce1b894db74f94ce573c62412) ) // sldh

	ROM_REGION( 0x200000, "gfx1", 0 ) /* 16x16x4 BG and 8x8x4 FG Tiles */
	ROM_LOAD( "4.bin",         0x000000, 0x80000, CRC(47f7c9bc) SHA1(f0e5ef971f3bd6972316c248175436055cb5789d) ) // sldh
	ROM_LOAD( "5.bin",         0x080000, 0x80000, CRC(6a6f499d) SHA1(cacdccc64d09fa7289221cdea4654e6c2d811647) ) // sldh
	ROM_LOAD( "6.bin",         0x100000, 0x80000, CRC(0c3d2609) SHA1(6e1f1c5b010ef0dfa3f7b4ff9a832e758fbb97d5) ) // sldh
	ROM_LOAD( "7.bin",         0x180000, 0x80000, CRC(dcece871) SHA1(7db919ab7f51748b77b3bd35228bbf71b951349f) ) // sldh

	ROM_REGION( 0x080000, "gfx2", 0 ) /* 16x16x4 Sprites */
	ROM_LOAD( "27_27c010.u86", 0x000000, 0x20000, CRC(a463422a) SHA1(a3b6efd1c57b0a3b0ce4ce734a9a9b79540c4136) )
	ROM_LOAD( "28_27c010.u85", 0x020000, 0x20000, CRC(ebec74ed) SHA1(9a1620f4ca163470f5e567f650663ae368bdd3c1) )
	ROM_LOAD( "29_27c010.u84", 0x040000, 0x20000, CRC(7ea4e22d) SHA1(3c7f05dfd1c5889bfcbc14d08026e2a484870216) )
	ROM_LOAD( "30_27c010.u83", 0x060000, 0x20000, CRC(283d9ba6) SHA1(6054853f76907a4a0f89ad5aa02dde9d3d4ff196) )

	ROM_REGION( 0x80000, "oki_rom", 0 ) /* Samples */
	ROM_LOAD( "20_27c040.u16", 0x00000, 0x80000, CRC(e152138e) SHA1(9d41b61b98414e1d5804b5a9edf4acb4c5f31615) )

	ROM_REGION( 0xc0000, "oki", 0 )
	ROM_COPY( "oki_rom", 0x000000, 0x000000, 0x020000)
	ROM_COPY( "oki_rom", 0x020000, 0x020000, 0x020000)
	ROM_COPY( "oki_rom", 0x000000, 0x040000, 0x020000)
	ROM_COPY( "oki_rom", 0x040000, 0x060000, 0x020000)
	ROM_COPY( "oki_rom", 0x000000, 0x080000, 0x020000)
	ROM_COPY( "oki_rom", 0x060000, 0x0a0000, 0x020000)
ROM_END

ROM_START( bestleaw )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "21_27c101.u67", 0x00000, 0x20000, CRC(ab5abd37) SHA1(822a4ab77041ea4d62d9f8df6197c4afe2558f21) )
	ROM_LOAD16_BYTE( "22_27c010.u66", 0x00001, 0x20000, CRC(4abc0580) SHA1(c834ad0710d1ecd3babb446df4b3b4e5d0b23cbd) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* 16x16x4 BG and 8x8x4 FG Tiles */
	ROM_LOAD( "23_27c040.u36", 0x000000, 0x80000, CRC(dcd53a97) SHA1(ed22c51a3501bbe164d8ec4b19f1f67e28e10427) )
	ROM_LOAD( "24_27c040.u42", 0x080000, 0x80000, CRC(2984c1a0) SHA1(ddab53cc6e9debb7f1fb7dae8196ff6df31cbedc) )
	ROM_LOAD( "25_27c040.u38", 0x100000, 0x80000, CRC(8bb5d73a) SHA1(bc93825aab08340ef182cde56323bf30ea6c5edf) )
	ROM_LOAD( "26_27c4001.u45", 0x180000, 0x80000, CRC(a82c905d) SHA1(b1c1098ad79eb66943bc362246983427d0263b6e) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* 16x16x4 Sprites */
	ROM_LOAD( "27_27c010.u86", 0x000000, 0x20000, CRC(a463422a) SHA1(a3b6efd1c57b0a3b0ce4ce734a9a9b79540c4136) )
	ROM_LOAD( "28_27c010.u85", 0x020000, 0x20000, CRC(ebec74ed) SHA1(9a1620f4ca163470f5e567f650663ae368bdd3c1) )
	ROM_LOAD( "29_27c010.u84", 0x040000, 0x20000, CRC(7ea4e22d) SHA1(3c7f05dfd1c5889bfcbc14d08026e2a484870216) )
	ROM_LOAD( "30_27c010.u83", 0x060000, 0x20000, CRC(283d9ba6) SHA1(6054853f76907a4a0f89ad5aa02dde9d3d4ff196) )

	ROM_REGION( 0x80000, "oki_rom", 0 ) /* Samples */
	ROM_LOAD( "20_27c040.u16", 0x00000, 0x80000, CRC(e152138e) SHA1(9d41b61b98414e1d5804b5a9edf4acb4c5f31615) )

	ROM_REGION( 0xc0000, "oki", 0 )
	ROM_COPY( "oki_rom", 0x000000, 0x000000, 0x020000)
	ROM_COPY( "oki_rom", 0x020000, 0x020000, 0x020000)
	ROM_COPY( "oki_rom", 0x000000, 0x040000, 0x020000)
	ROM_COPY( "oki_rom", 0x040000, 0x060000, 0x020000)
	ROM_COPY( "oki_rom", 0x000000, 0x080000, 0x020000)
	ROM_COPY( "oki_rom", 0x060000, 0x0a0000, 0x020000)

	ROM_REGION( 0x2000, "plds", 0 )
	ROM_LOAD( "85c060.bin",            0x0000, 0x032f, CRC(537100ac) SHA1(3d5e9013e3cba660671f02e78c233c866dad2e53) )
	ROM_LOAD( "gal16v8-25hb1.u182",    0x0200, 0x0117, NO_DUMP ) /* Protected */
	ROM_LOAD( "gal16v8-25hb1.u183",    0x0400, 0x0117, NO_DUMP ) /* Protected */
	ROM_LOAD( "gal16v8-25hb1.u58",     0x0800, 0x0117, NO_DUMP ) /* Protected */
	ROM_LOAD( "palce20v8h-15pc-4.u38", 0x1000, 0x0157, NO_DUMP ) /* Protected */
ROM_END

} // anonymous namespace


GAME( 1993, bestleag, bigstrik, bestleag, bestleag, bestleag_state, empty_init, ROT0, "bootleg", "Best League (bootleg of Big Striker, Italian Serie A)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, bestleaw, bigstrik, bestleag, bestleag, bestleaw_state, empty_init, ROT0, "bootleg", "Best League (bootleg of Big Striker, World Cup)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
