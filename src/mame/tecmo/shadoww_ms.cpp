/*
    Shadow Warriors (Modular System)

    As with most of the 'Modular System' setups, the hardware is heavily modified from the original
    and consists of a multi-board stack in a cage, hence different driver.

    For this game the Modular System cage contains 7 boards and 1 sub-board:

    MOD-6/1   - MC68000P10, 4 ROMs, RAMs, 20 MHz Xtal.
    MOD-7/3   - Logic, RAMs.
    MOD-8     - Tilemap board, 20 ROMs.
    MOD 21/1  - Logic, 2 PROMs, 20 MHz Xtal.
    MOD 51/3  - Sprite board, has logic + 1 PROM. Has MOD-5/1 PCB plugged into its 4 ROMs sockets.
    MODULAR SYSTEM 2 MOD 5/1 - Red sprite ROM board, 16 sprite ROMs populated (maximum 24 ROMs)
    MOD 1/3   - Sound board (Z840006PSC, two YM2203C, two Y3014B, 1 ROM, 1 PROM, and a small sub board with OKI M5205). Two 8 dip switches banks.

    PCB pics, dip switches, and more info: https://www.recreativas.org/modular-system-shadow-warriors-554-ervisa

    Very similar to mastninj in tecmo/gaiden.cpp. However the differences (palette and sprite RAM format, scrolling registers, sound system)
    maybe make it worthwhile to leave it as a separate driver.

    TODO:
    - sound;
    - tilemaps y scroll needs slight adjustment;
    - sprites positioning may be slightly off.
*/


#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class shadoww_ms_state : public driver_device
{
public:
	shadoww_ms_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_msm(*this, "msm5205"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram%u", 1)
	{ }

	void shadowwm(machine_config &config) ATTR_COLD;

	void init_gfx() ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<msm5205_device> m_msm;

	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr_array<uint16_t, 3> m_videoram;

	tilemap_t *m_text_layer = nullptr;
	tilemap_t *m_foreground = nullptr;
	tilemap_t *m_background = nullptr;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	void bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void tx_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


void shadoww_ms_state::video_start()
{
	m_background = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(shadoww_ms_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_foreground = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(shadoww_ms_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_text_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(shadoww_ms_state::get_tx_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_foreground->set_transparent_pen(15);
	m_text_layer->set_transparent_pen(15);
}

TILE_GET_INFO_MEMBER(shadoww_ms_state::get_bg_tile_info)
{
	uint16_t const *const videoram1 = &m_videoram[2][0x0800];
	uint16_t const *const videoram2 = m_videoram[2];
	tileinfo.set(1,
			videoram1[tile_index] & 0x0fff,
			(videoram2[tile_index] & 0xf0) >> 4,
			0);
}

TILE_GET_INFO_MEMBER(shadoww_ms_state::get_fg_tile_info)
{
	uint16_t const *const videoram1 = &m_videoram[1][0x0800];
	uint16_t const *const videoram2 = m_videoram[1];
	tileinfo.set(2,
			videoram1[tile_index] & 0x0fff,
			(videoram2[tile_index] & 0xf0) >> 4,
			0);
}

TILE_GET_INFO_MEMBER(shadoww_ms_state::get_tx_tile_info)
{
	uint16_t const *const videoram1 = &m_videoram[0][0x0400];
	uint16_t const *const videoram2 = m_videoram[0];
	tileinfo.set(0, videoram1[tile_index] & 0x07ff, (videoram2[tile_index] & 0xf0) >> 4, 0);
}

void shadoww_ms_state::bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram[2][offset]);
	m_background->mark_tile_dirty(offset & 0x07ff);
}

void shadoww_ms_state::fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram[1][offset]);
	m_foreground->mark_tile_dirty(offset & 0x07ff);
}

void shadoww_ms_state::tx_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram[0][offset]);
	m_text_layer->mark_tile_dirty(offset & 0x03ff);
}

void shadoww_ms_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const int NUM_SPRITES = 0x100;
	const int X_EXTRA_OFFSET = -0x10;

	for (int i = 0; i < NUM_SPRITES; i += 2)
	{
		gfx_element *gfx = m_gfxdecode->gfx(3);

		uint16_t const attr0 = m_spriteram[i + 0];
		uint16_t const attr1 = m_spriteram[i + 1];

		uint16_t const attr2 = m_spriteram[i + NUM_SPRITES];
		//uint16_t const attr3 = m_spriteram[i + NUM_SPRITES + 1]; // unused?

		if (!(attr2 & 0x8000))
			continue;

		int ypos = attr0 & 0x00ff;
		int const xpos = (attr1 & 0xff00) >> 8;

		ypos = (0xff - ypos);

		int tile = (attr0 & 0xff00) >> 8;
		tile |= (attr1 & 0x001f) << 8;

		int const col = (attr2 & 0x0f00) >> 8;

		int const flipx = (attr1 & 0x0040);
		int const flipy = (attr2 & 0x4000);

		int const pri_mask = (col & 0x08) ? 0x00 : 0xf0 | 0xcc;

		gfx->prio_transpen(bitmap, cliprect, tile, col, flipx, flipy, xpos - 16 - X_EXTRA_OFFSET, ypos - 16, screen.priority(), pri_mask, 15);
	}
}

uint32_t shadoww_ms_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	m_background->draw(screen, bitmap, cliprect, 0, 1);
	m_foreground->draw(screen, bitmap, cliprect, 0, 2);
	m_text_layer->draw(screen, bitmap, cliprect, 0, 4);
	draw_sprites(screen, bitmap, cliprect);

	return 0;
}


void shadoww_ms_state::main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x060000, 0x063fff).ram();
	map(0x070000, 0x070fff).ram().w(FUNC(shadoww_ms_state::tx_videoram_w)).share(m_videoram[0]);
	map(0x072000, 0x073fff).ram().ram().w(FUNC(shadoww_ms_state::fg_videoram_w)).share(m_videoram[1]);
	map(0x074000, 0x075fff).ram().ram().w(FUNC(shadoww_ms_state::bg_videoram_w)).share(m_videoram[2]);
	map(0x076000, 0x076fff).ram().share(m_spriteram);
	map(0x078000, 0x0783ff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x078400, 0x0787ff).ram().w("palette", FUNC(palette_device::write16_ext)).share("palette_ext");
	map(0x078800, 0x079fff).ram();
	map(0x07a000, 0x07a001).portr("SYSTEM");
	map(0x07a002, 0x07a003).portr("P1_P2");
	map(0x07a004, 0x07a005).portr("DSW");
	map(0x07e000, 0x07e000).lw8(NAME([this] (uint8_t data) { m_maincpu->set_input_line(5, CLEAR_LINE); }));
	map(0x07f000, 0x07f001).lw16(NAME([this] (uint16_t data) { m_foreground->set_scrollx(0, (0x33 - data) & 0x3ff); }));
	map(0x07f002, 0x07f003).lw16(NAME([this] (uint16_t data) { m_foreground->set_scrolly(0, -data & 0x1ff); })); // not 100% correct
	map(0x07f004, 0x07f005).lw16(NAME([this] (uint16_t data) { m_background->set_scrollx(0, (0x30 - data) & 0x3ff); }));
	map(0x07f006, 0x07f007).lw16(NAME([this] (uint16_t data) { m_background->set_scrolly(0, -data & 0x1ff); })); // not 100% correct
}

void shadoww_ms_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	// map(0x8000, 0x8000).w() // MSM5205?
	map(0xc000, 0xc001).w("ym1", FUNC(ym2203_device::write));
	map(0xc002, 0xc003).w("ym2", FUNC(ym2203_device::write));
	map(0xc008, 0xc009).r("ym1", FUNC(ym2203_device::read));
	map(0xc00a, 0xc00b).r("ym2", FUNC(ym2203_device::read));
	map(0xf000, 0xffff).ram();
}


static INPUT_PORTS_START( shadowwm )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW") // from manual
	PORT_DIPNAME( 0x00e0, 0x00e0, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:3,2,1")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:6,5,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW1:7") // Flip Screen on original, listed empty in this game manual
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0xc000, "1" )
	PORT_DIPSETTING(      0x4000, "2" )
	PORT_DIPSETTING(      0x8000, "3" )
	PORT_DIPNAME( 0x3000, 0x3000, "Energy" ) PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x3000, "3" )
	PORT_DIPSETTING(      0x1000, "4" )
	PORT_DIPSETTING(      0x2000, "5" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,5")
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, "TBL 1" )
	PORT_DIPSETTING(      0x0800, "TBL 2" )
	PORT_DIPSETTING(      0x0000, "TBL 3" )
	PORT_DIPUNUSED_DIPLOC( 0x0200, 0x0200, "SW2:7" ) // listed as 'don't change'
	PORT_DIPUNUSED_DIPLOC( 0x0100, 0x0100, "SW2:8" ) // listed as 'don't change'

INPUT_PORTS_END


static const gfx_layout tile_layout =
{
	16, 16,
	RGN_FRAC(1, 1),
	4,
	{ STEP4(24, -8) },
	{ STEP8(0, 1), STEP8(256, 1) },
	{ STEP8(0, 32), STEP8(512, 32) },
	32 * 32
};

static const gfx_layout sprites_layout =
{
	16, 16,
	RGN_FRAC(1, 1),
	4,
	{ STEP4(0, 8) },
	{ STEP8(0, 1), STEP8(512, 1) },
	{ STEP16(0,32) },
	16 * 16 * 4
};


static GFXDECODE_START( gfx_shadowmm )
	GFXDECODE_ENTRY( "txtiles", 0, gfx_8x8x4_planar, 0, 16 )
	GFXDECODE_ENTRY( "tiles", 0, tile_layout, 0x300, 16 )
	GFXDECODE_ENTRY( "tiles", 0x80000, tile_layout, 0x200, 16 )
	GFXDECODE_ENTRY( "sprites", 0, sprites_layout, 0x100, 16 )
GFXDECODE_END


void shadoww_ms_state::shadowwm(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 20_MHz_XTAL / 2); // divisor unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &shadoww_ms_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(shadoww_ms_state::irq5_line_assert));

	z80_device &audiocpu(Z80(config, "audiocpu", 20_MHz_XTAL / 5)); // divisor unknown
	audiocpu.set_addrmap(AS_PROGRAM, &shadoww_ms_state::sound_map);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER); // all wrong
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 16, 240-1);
	m_screen->set_screen_update(FUNC(shadoww_ms_state::screen_update));
	m_screen->set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_shadowmm);

	PALETTE(config, "palette").set_format(palette_device::xBGR_444, 0x400);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	YM2203(config, "ym1", 20_MHz_XTAL / 5).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown

	YM2203(config, "ym2", 20_MHz_XTAL / 5).add_route(ALL_OUTPUTS, "mono", 0.15); // divisor unknown

	MSM5205(config, m_msm, XTAL(384'000)); // unknown clock
	m_msm->set_prescaler_selector(msm5205_device::S48_4B); // unverified
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.25);
}


ROM_START( shadowwm )
	ROM_REGION( 0x40000, "maincpu", 0 ) // on MOD 6/1 board, very similar to mastninj
	ROM_LOAD16_BYTE( "mod_6-1_3sw_603.ic17", 0x00000, 0x10000, CRC(a74f4849) SHA1(8c2e50bb1795e6bef3dc76c15a9087771300bdaa) )
	ROM_LOAD16_BYTE( "mod_6-1_3sw_606.ic8",  0x00001, 0x10000, CRC(bd51890f) SHA1(ffad07f01618363e144428823bd0e83be27db6c3) )
	ROM_LOAD16_BYTE( "mod_6-1_3sw_605.ic11", 0x20001, 0x08000, CRC(4aba6041) SHA1(22a9913e6894d7b567d17fca38c4f053ca4fb159) )
	ROM_LOAD16_BYTE( "mod_6-1_3sw_602.ic20", 0x30001, 0x08000, CRC(e642241b) SHA1(7abcda6b86595ba86ae12874d6787ef8dcd64f84) )
	ROM_CONTINUE(                            0x20000, 0x08000 ) // some weird addressing

	ROM_REGION( 0x10000, "audiocpu", 0 ) // on MOD 1/3 board
	ROM_LOAD( "mod_1-3_3sw_415.ic12", 0x00000, 0x10000, CRC(4dba665b) SHA1(8d068cd588f31cbe944a2f07dbbb81a74d80bcac) )

	ROM_REGION( 0x10000, "txtiles", 0 ) // on MOD 8 board
	ROM_LOAD( "mod_8_801_3sw.ic15", 0x0000, 0x4000, CRC(8af36af3) SHA1(5fd1e65b9e481cd6135710629d7d74a5edd94378) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_IGNORE(                             0x4000 )
	ROM_LOAD( "mod_8_802_3sw.ic22", 0x4000, 0x4000, CRC(1c3aadc2) SHA1(e14c46cf41713632d8514a0baa6bbf4581a90e7a) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_IGNORE(                             0x4000 )
	ROM_LOAD( "mod_8_803_3sw.ic30", 0x8000, 0x4000, CRC(829f8213) SHA1(4adbddf1a31d9ba766ded3a701d57e12fca56de7) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_IGNORE(                             0x4000 )
	ROM_LOAD( "mod_8_804_3sw.ic37", 0xc000, 0x4000, CRC(4543e707) SHA1(b6e3a8ed9e8b4014007a0b254400baae7aa050bf) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_IGNORE(                             0x4000 )

	ROM_REGION( 0x100000, "tiles", 0 ) // on MOD 8 board
	ROM_LOAD32_BYTE( "mod_8_3sw.ic11",     0x00000, 0x10000, CRC(e3987e0f) SHA1(8805443e56575fc455d21703bd2f9ebef434e262) )
	ROM_LOAD32_BYTE( "mod_8_3sw.ic18",     0x00001, 0x10000, CRC(5e8afc68) SHA1(ecef113a947b9bda6abbef5f75557cd201e355b3) )
	ROM_LOAD32_BYTE( "mod_8_3sw.ic26",     0x00002, 0x10000, CRC(2713e9f1) SHA1(87614a79596216d2b710925167f3130d4c2e07c9) )
	ROM_LOAD32_BYTE( "mod_8_3sw.ic33",     0x00003, 0x10000, CRC(ca59280f) SHA1(d5a1d85f75ea667812708758915f43f01c8c9830) )
	ROM_LOAD32_BYTE( "mod_8_821_3sw.ic10", 0x40000, 0x10000, CRC(72e5c1c2) SHA1(0e744407c52a61ed657557978cdfe455fe5e931e) )
	ROM_LOAD32_BYTE( "mod_8_822_3sw.ic17", 0x40001, 0x10000, CRC(55affaf8) SHA1(c121a904ba44dc53c8a10b8d56c4c25ab879d8be) )
	ROM_LOAD32_BYTE( "mod_8_823_3sw.ic25", 0x40002, 0x10000, CRC(bd76fd53) SHA1(bc2ad054b63573f16c99f82c680f0f6de2ee4683) )
	ROM_LOAD32_BYTE( "mod_8_824_3sw.ic32", 0x40003, 0x10000, CRC(f3bfcfd6) SHA1(7d73a2ae00825979b3d09502f52d78f61f3ea1a9) )
	ROM_LOAD32_BYTE( "mod_8_3sw.ic13",     0x80000, 0x10000, CRC(ae043a2e) SHA1(3513b21d4ee7f869c9ebda68707845d030b0ecad) )
	ROM_LOAD32_BYTE( "mod_8_3sw.ic20",     0x80001, 0x10000, CRC(e197fd97) SHA1(1e92139ae6a1c15de629039757c21e32cacc42d6) )
	ROM_LOAD32_BYTE( "mod_8_3sw.ic28",     0x80002, 0x10000, CRC(0871971c) SHA1(ede9bb5d6d968fc532217b4eb1cd4c0d7ea9a4a1) )
	ROM_LOAD32_BYTE( "mod_8_3sw.ic35",     0x80003, 0x10000, CRC(6850aea3) SHA1(670820ba2df040ded8739907bfdde4ac97373200) )
	ROM_LOAD32_BYTE( "mod_8_3sw.ic12",     0xc0000, 0x10000, CRC(dd162ce7) SHA1(70ec5a722ea31434be2a4b3104f9c54a48b8ec05) )
	ROM_LOAD32_BYTE( "mod_8_3sw.ic19",     0xc0001, 0x10000, CRC(edd65385) SHA1(3b5c0115ae1972bfe696a22edd2da9e6fb9739f4) )
	ROM_LOAD32_BYTE( "mod_8_3sw.ic27",     0xc0002, 0x10000, CRC(ca691635) SHA1(177f94a17cfaf67c764c2a2dff48475039207fae) )
	ROM_LOAD32_BYTE( "mod_8_3sw.ic34",     0xc0003, 0x10000, CRC(2ae70f42) SHA1(aad89dbd0309a5e3a786aa028995b56859d5b5ff) )

	ROM_REGION( 0x100000, "sprites", ROMREGION_INVERT ) // MODULAR SYSTEM 2 MOD 5/1
	ROM_LOAD32_BYTE( "mod_5-1_3sw_5d01.ic24", 0x00000, 0x10000, CRC(3c117e62) SHA1(dee45d6bbe053996e0b3faaba0293a273faf1ffa) )
	ROM_LOAD32_BYTE( "mod_5-1_3sw_5c01.ic18", 0x00001, 0x10000, CRC(940f3dbb) SHA1(4e2f224ed2ec1b8da992bd375d3ab1cf6fbfdd1f) )
	ROM_LOAD32_BYTE( "mod_5-1_3sw_5b1.ic12",  0x00002, 0x10000, CRC(012da98d) SHA1(413e1f02e2e3267fb4b893b14f627105789aa1c9) )
	ROM_LOAD32_BYTE( "mod_5-1_3sw_5a01.ic03", 0x00003, 0x10000, CRC(216eeef5) SHA1(5167af8cef220a5092add2bf578e8323360132a5) )
	ROM_LOAD32_BYTE( "mod_5-1_3sw_5d02.ic25", 0x40000, 0x10000, CRC(f6d6422d) SHA1(933487b09d3bcff9714fb2469b3d751b38459cfd) )
	ROM_LOAD32_BYTE( "mod_5-1_3sw_5c02.ic19", 0x40001, 0x10000, CRC(f6baccb0) SHA1(2244d16127efe67fcf59a59e50eabc54e3081dd1) )
	ROM_LOAD32_BYTE( "mod_5-1_3sw_5b2.ic13",  0x40002, 0x10000, CRC(fa32da96) SHA1(5e240f6f91813bdafacf1d29ea65704f2c4f2ae6) )
	ROM_LOAD32_BYTE( "mod_5-1_3sw_5a02.ic04", 0x40003, 0x10000, CRC(f72f8bfd) SHA1(ccf0aab11987e76c927a73f12e5cd4bb125c1258) )
	ROM_LOAD32_BYTE( "mod_5-1_3sw_5d03.ic26", 0x80000, 0x10000, CRC(642f06e7) SHA1(5b30b5029884b7eddcad201224a639f94ee27823) )
	ROM_LOAD32_BYTE( "mod_5-1_3sw_5c03.ic20", 0x80001, 0x10000, CRC(bb46ef1b) SHA1(4c8f9e06fa4d7f14206f6180a999b3f32681785a) )
	ROM_LOAD32_BYTE( "mod_5-1_3sw_5b3.ic14",  0x80002, 0x10000, CRC(910fccdb) SHA1(99523b53ae0dbf82783ab5a731df3c02984c72fe) )
	ROM_LOAD32_BYTE( "mod_5-1_3sw_5a03.ic05", 0x80003, 0x10000, CRC(6de96087) SHA1(0b9028320cb622dad07cf8bde015428eba7f8a5e) )
	ROM_LOAD32_BYTE( "mod_5-1_3sw_5d04.ic27", 0xc0000, 0x10000, CRC(7f7f70b6) SHA1(727333a7825a04d9eb2bdea49b8203bbec7b9c24) )
	ROM_LOAD32_BYTE( "mod_5-1_3sw_5c04.ic21", 0xc0001, 0x10000, CRC(065591f5) SHA1(84481c023d722a54682c22421f97f3d221237195) )
	ROM_LOAD32_BYTE( "mod_5-1_3sw_5b4.ic15",  0xc0002, 0x10000, CRC(bb9a59e3) SHA1(5e674305267e7e20172c97b5b85f6b58ce51962c) )
	ROM_LOAD32_BYTE( "mod_5-1_3sw_5a04.ic06", 0xc0003, 0x10000, CRC(44275bc9) SHA1(72bfc098561e47f2820c676e551e8eaa402c88fc) )

	ROM_REGION( 0x100, "proms", 0 )    // PROMs (function unknown)
	ROM_LOAD( "mod_51-3_502_82s129.ic10",   0x000, 0x100, CRC(15085e44) SHA1(646e7100fcb112594023cf02be036bd3d42cc13c) )
	ROM_LOAD( "mod_21-1_p0202_82s129.ic12", 0x000, 0x100, CRC(e434128a) SHA1(ef0f6d8daef8b25211095577a182cdf120a272c1) )
	ROM_LOAD( "mod_21-1_p0201_82s129.ic4",  0x000, 0x100, CRC(2697da58) SHA1(e62516b886ff6e204b718e5f0c6ce2712e4b7fc5) )

	ROM_REGION( 0x257, "plds", ROMREGION_ERASEFF )
	ROM_LOAD( "mod_6-1_604_gal16v8.ic13",  0x000, 0x117, NO_DUMP )
	ROM_LOAD( "mod_6-1_634_gal16v8.ic7",   0x000, 0x117, CRC(959911c9) SHA1(396c43febb5db59aab2ca68d830c02df1b99636c) )
	ROM_LOAD( "mod_5-1_5134_gal16v8.ic9",  0x000, 0x117, CRC(e0f3022f) SHA1(6fa3b16768c73c5eabed845791ca951bde6b8947) )
	ROM_LOAD( "mod_5-1_5234_gal16v8.ic8",  0x000, 0x117, CRC(9d29fe2e) SHA1(5bab55cd6ac1f2e01fbdfb69bcdf0e018e679493) )
	ROM_LOAD( "mod_7-3_7134_gal20v8.ic7",  0x000, 0x157, CRC(2f44a624) SHA1(c1c2a5f4a8e5bf230acd4f5550712e34e843d5ef) )
	ROM_LOAD( "mod_7-3_7234_gal20v8.ic54", 0x000, 0x157, CRC(ddc39934) SHA1(ea23244a2ee57665f2e25c155af0c1496c64c167) )
	ROM_LOAD( "mod_7-3_7434_gal16v8.ic9",  0x000, 0x117, CRC(02520cd3) SHA1(309f2dab8c4a8503cbdd86530b4b839cfcb96904) )
	ROM_LOAD( "mod_7-3_7534_gal16v8.ic59", 0x000, 0x117, CRC(dd6383fe) SHA1(78ce49638e3cef09b767860d86431393e663241e) )
	ROM_LOAD( "mod_7-3_7634_gal20v8.ic44", 0x000, 0x157, CRC(155136d4) SHA1(68af03527e8a11011569f35cfb9e601136d5d1ee) )
	ROM_LOAD( "mod_51-3_503_gal16v8.ic46", 0x000, 0x117, CRC(11470ea1) SHA1(cfcafbcc7e55be717348f895df61e144fdd0cc9b) )
ROM_END


void shadoww_ms_state::init_gfx()
{
	uint8_t *src = memregion("tiles")->base();
	int const len = 0x100000;

	//  rearrange gfx
	{
		std::vector<uint8_t> buffer(len);
		for (int i = 0;i < len; i++)
		{
			buffer[i] = src[bitswap<24>(i, 23, 22, 21, 20, 19, 18, 17, 16, 15, 5, 6, 14, 13, 12, 11, 10, 9, 8, 7, 4, 3, 2, 1, 0)];
		}
		std::copy(buffer.begin(), buffer.end(), &src[0]);
	}
}

} // anonymous namespace


GAME( 1988, shadowwm, shadoww, shadowwm, shadowwm, shadoww_ms_state, init_gfx, ROT0, "bootleg (Gaelco / Ervisa)", "Shadow Warriors (Modular System)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
