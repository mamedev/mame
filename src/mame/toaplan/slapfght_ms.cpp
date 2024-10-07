// license:BSD-3-Clause
// copyright-holders:

/*
    Slap Fight (Modular System)

    As with most of the 'Modular System' setups, the hardware is heavily modified from the original
    and consists of a multi-board stack in a cage, hence different driver.

    For this game the Modular System cage contains 6 main boards and 1 sub board.

    MOD. 3 - Z80 board (3 ROMs + RAM + 24MHz XTAL) + small sub board with Z0840006PSC + PROM + logic (decryption?).
    MOD 21/1 - 20 MHz XTAL + logic.
    MOD 1 - Sound board (Z08400BPS, 2 x YM2203C). 2 8-dips banks.
    MOD 51/1 - Sprite board, has logic + 4 ROMs.
    MOD 4/2 - Tilemap board, has logic + 2 ROMs.
    MOD 4/2 - Tilemap board, has logic + 4 ROMs, long thin sub-board (C0425) with no chips, just routing along one edge.

    TODO:
    - stuck at RAM CHECK OK due to wrong main CPU IRQ handling, if prodded via debugger it passes ROM CHECK too, but then stops at SOUND OK again due to IRQs;
    - colors (no color PROMs?);
    - sound IRQs once testable;
    - derive from slapfght_state (slapfght.cpp) once all the common points and differences are verified.
*/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class slapfght_ms_state : public driver_device
{
public:
	slapfght_ms_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_fixvideoram(*this, "fixvideoram"),
		m_fixcolorram(*this, "fixcolorram"),
		m_mainbank(*this, "mainbank")
	{ }

	void slapfighm(machine_config &config);

	void init_decryption();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_fixvideoram;
	required_shared_ptr<uint8_t> m_fixcolorram;
	required_memory_bank m_mainbank;

	bool m_main_irq_enabled = false;
	bool m_sound_nmi_enabled = false;

	tilemap_t *m_pf1_tilemap = nullptr;
	tilemap_t *m_fix_tilemap = nullptr;

	void vblank_irq(int state);
	void irq_enable_w(int state);
	void sound_reset_w(int state);

	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void fixram_w(offs_t offset, uint8_t data);
	void fixcol_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_pf1_tile_info);
	TILE_GET_INFO_MEMBER(get_fix_tile_info);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_io_map(address_map &map) ATTR_COLD;
	void main_prg_map(address_map &map) ATTR_COLD;
	void sound_prg_map(address_map &map) ATTR_COLD;
};


TILE_GET_INFO_MEMBER(slapfght_ms_state::get_pf1_tile_info)
{
	int tile = m_videoram[tile_index] | ((m_colorram[tile_index] & 0x0f) << 8);
	int color = (m_colorram[tile_index] & 0xf0) >> 4;

	tileinfo.set(1, tile, color, 0);
}

TILE_GET_INFO_MEMBER(slapfght_ms_state::get_fix_tile_info)
{
	int tile = m_fixvideoram[tile_index] | ((m_fixcolorram[tile_index] & 0x03) << 8);
	int color = (m_fixcolorram[tile_index] & 0xfc) >> 2;

	tileinfo.set(0, tile, color, 0);
}

void slapfght_ms_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_pf1_tilemap->mark_tile_dirty(offset);
}

void slapfght_ms_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_pf1_tilemap->mark_tile_dirty(offset);
}

void slapfght_ms_state::fixram_w(offs_t offset, uint8_t data)
{
	m_fixvideoram[offset] = data;
	m_fix_tilemap->mark_tile_dirty(offset);
}

void slapfght_ms_state::fixcol_w(offs_t offset, uint8_t data)
{
	m_fixcolorram[offset] = data;
	m_fix_tilemap->mark_tile_dirty(offset);
}

void slapfght_ms_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect) // TODO: verify
{
	const uint8_t *src = m_spriteram;

	for (int offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		/*
		    0: xxxxxxxx - code low
		    1: xxxxxxxx - x low
		    2: xx...... - code high
		       ..x..... - no function?
		       ...xxxx. - color
		       .......x - x high
		    3: xxxxxxxx - y
		*/

		int code = src[offs + 0] | ((src[offs + 2] & 0xc0) << 2);
		int sy = src[offs + 3] + 2;
		int sx = (src[offs + 1] | (src[offs + 2] << 8 & 0x100)) - 17;
		int color = src[offs + 2] >> 1 & 0xf;
		int fx = 0, fy = 0;

		if (flip_screen())
		{
			sy = (238 - sy) & 0xff;
			sx = 284 - sx;
			fx = fy = 1;
		}

		if (sy > 256-8)
			sy -= 256;

		m_gfxdecode->gfx(2)->transpen(bitmap, cliprect, code, color, fx, fy, sx, sy, 0);
	}
}

uint32_t slapfght_ms_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//m_pf1_tilemap->set_scrollx(m_scrollx_hi << 8 | m_scrollx_lo);
	//m_pf1_tilemap->set_scrolly(m_scrolly);

	m_pf1_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

void slapfght_ms_state::video_start()
{
	m_pf1_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(slapfght_ms_state::get_pf1_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(slapfght_ms_state::get_fix_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_fix_tilemap->set_scrolldy(0, 15);
	m_pf1_tilemap->set_scrolldy(0, 14);

	m_fix_tilemap->set_transparent_pen(0);
}


void slapfght_ms_state::vblank_irq(int state)
{
	if (state && m_main_irq_enabled)
		m_maincpu->set_input_line(0, ASSERT_LINE);
}

void slapfght_ms_state::irq_enable_w(int state)
{
	m_main_irq_enabled = state ? true : false;

	if (!m_main_irq_enabled)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

void slapfght_ms_state::sound_reset_w(int state)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, state ? CLEAR_LINE : ASSERT_LINE);

	if (state == 0)
		m_sound_nmi_enabled = false;
}

void slapfght_ms_state::machine_start()
{
	// 2 x 0x4000 banks, 0xc000 - 0xffff is 0xff filled but for the 0xea00 - 0xefff range which is mapped directly
	m_mainbank->configure_entry(0, memregion("maincpu")->base() + 0x8000);
	m_mainbank->configure_entry(1, memregion("maincpu")->base() + 0x10000);

	save_item(NAME(m_main_irq_enabled));
	save_item(NAME(m_sound_nmi_enabled));
}


void slapfght_ms_state::main_prg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_mainbank);
	map(0xc000, 0xc7ff).ram();
	map(0xc800, 0xc80f).ram().share("main_sound");
	map(0xc810, 0xcfff).ram();
	map(0xd000, 0xd7ff).ram().w(FUNC(slapfght_ms_state::videoram_w)).share(m_videoram);
	map(0xd800, 0xdfff).ram().w(FUNC(slapfght_ms_state::colorram_w)).share(m_colorram);
	map(0xe000, 0xe7ff).ram().share(m_spriteram);
	map(0xe900, 0xe900).lw8(NAME([this] (uint8_t data) { m_mainbank->set_entry(data & 0x01); }));
	map(0xea00, 0xefff).rom().region("maincpu", 0xea00); // title screen data gets loaded here like slapfighb2
	map(0xf000, 0xf7ff).ram().w(FUNC(slapfght_ms_state::fixram_w)).share(m_fixvideoram);
	map(0xf800, 0xffff).ram().w(FUNC(slapfght_ms_state::fixcol_w)).share(m_fixcolorram);
}

void slapfght_ms_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x0f).w("mainlatch", FUNC(ls259_device::write_a0));
}

void slapfght_ms_state::sound_prg_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0xc800, 0xc9ff).ram();
	map(0xdff0, 0xdfff).ram().share("main_sound");
	map(0xe000, 0xe001).w("ym1", FUNC(ym2203_device::write));
	map(0xe002, 0xe003).w("ym2", FUNC(ym2203_device::write));
	map(0xe008, 0xe009).r("ym1", FUNC(ym2203_device::read));
	map(0xe00a, 0xe00b).r("ym2", FUNC(ym2203_device::read));
}


static INPUT_PORTS_START( slapfighm )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_COCKTAIL

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("SW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	PORT_START("SW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout tilelayout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,8,16,24 },
	{ 0,1,2,3,4,5,6,7, 512,513,514,515,516,517,518,519 },
	{ STEP16(0,32) },
	16 * 16 * 4
};

static GFXDECODE_START( gfx_slapfighm)
	GFXDECODE_ENTRY( "chars",   0, charlayout,   0,  64 )
	GFXDECODE_ENTRY( "tiles",   0, tilelayout,   0,  16 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout, 0,  16 )
GFXDECODE_END


void slapfght_ms_state::slapfighm(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 24_MHz_XTAL / 4); // divisor unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &slapfght_ms_state::main_prg_map);
	m_maincpu->set_addrmap(AS_IO, &slapfght_ms_state::main_io_map);

	Z80(config, m_audiocpu, 20_MHz_XTAL / 5); // divisor unknown
	m_audiocpu->set_addrmap(AS_PROGRAM, &slapfght_ms_state::sound_prg_map);

	ls259_device &mainlatch(LS259(config, "mainlatch"));
	mainlatch.q_out_cb<0>().set(FUNC(slapfght_ms_state::sound_reset_w));
	//mainlatch.q_out_cb<1>().set(FUNC(slapfght_ms_state::flipscreen_w));
	mainlatch.q_out_cb<3>().set(FUNC(slapfght_ms_state::irq_enable_w));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER); // all wrong
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 32*8-1);
	m_screen->set_screen_update(FUNC(slapfght_ms_state::screen_update));
	m_screen->screen_vblank().set(FUNC(slapfght_ms_state::vblank_irq));
	m_screen->set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xRGB_444, 256);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_slapfighm);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	ym2203_device &ym1(YM2203(config, "ym1", 20_MHz_XTAL / 5)); // divisor unknown
	ym1.port_a_read_callback().set_ioport("IN0");
	ym1.port_b_read_callback().set_ioport("IN1");
	ym1.irq_handler().set([this](int state) { if (state) logerror("ym 1 IRQ\n"); });
	ym1.add_route(ALL_OUTPUTS, "mono", 0.15);

	ym2203_device &ym2(YM2203(config, "ym2", 20_MHz_XTAL / 5)); // divisor unknown
	ym2.port_a_read_callback().set_ioport("SW1");
	ym2.port_b_read_callback().set_ioport("SW2");
	ym2.irq_handler().set([this](int state) { if (state) logerror("ym 2 IRQ\n"); });
	ym2.add_route(ALL_OUTPUTS, "mono", 0.15);
}

ROM_START( slapfighm )
	ROM_REGION( 0x14000, "maincpu", 0 ) // on MOD. 3 board
	ROM_LOAD( "3_cpu_fi_303.ic12-512", 0x0000,  0x8000, CRC(639b0817) SHA1(0999d447d1cc182c5d1fb1be91e47f1e64675a78) ) // 27256
	ROM_LOAD( "3_cpu_fi_302.ic13-512", 0x8000,  0x8000, CRC(97403de3) SHA1(b96acd3b4bbe6e6dc060255aaa1b237100c41847) ) // 27256
	ROM_LOAD( "3_cpu_fi_301.ic14-513", 0x10000, 0x4000, CRC(c63b02fe) SHA1(8d1991780b7aed718f60b6a8f5de7571682fb713) ) // 27128, identical to the original once decrypted

	ROM_REGION( 0x4000, "audiocpu", 0 ) // on MOD 1 board
	ROM_LOAD( "1_snd_fi_0101.ic12", 0x0000, 0x4000, CRC(4e3ae13e) SHA1(4ab29064f3a4cb4fd70a9eb8ddbffb4f5ee74057) ) // 1xxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x4000, "chars", 0 ) // on one MOD 4/2 board, contains the same data as the original among the weirdness
	ROM_LOAD( "4-2_fi_401.ic17", 0x0000, 0x4000, CRC(fbdc3ded) SHA1(5c1db5fe0ce32d996f40acf51b9ea09f7709a032) ) // BADADDR         --xxxxxxxxxxxxx, 4-2_fi_401.ic17  [3/4] a77_03.6g IDENTICAL
	ROM_CONTINUE(                0x0000, 0x4000 )
	ROM_LOAD( "4-2_fi_402.ic16", 0x2000, 0x2000, CRC(6e9ce9ea) SHA1(9b80360050022fb7b938a874d5c9adb344907bab) ) // BADADDR         x-xxxxxxxxxxxxx, 4-2_fi_402.ic16  [3/4] a77_04.6f IDENTICAL
	ROM_CONTINUE(                0x2000, 0x2000 )
	ROM_CONTINUE(                0x2000, 0x2000 )
	ROM_IGNORE(                          0x2000 )

	ROM_REGION( 0x20000, "tiles", 0 ) // on a second MOD 4/2 board, identical to the original
	ROM_LOAD( "4-2-b_fi_4a01.ic17", 0x00000, 0x8000, CRC(974e2ea9) SHA1(3840550fc3a833828dad8f3e300d2ea583d69ce7) )
	ROM_LOAD( "4-2-b_fi_4a02.ic16", 0x08000, 0x8000, CRC(5faeeea3) SHA1(696fba24bcf1f3a7e914a4403854da5eededaf7f) )
	ROM_LOAD( "4-2-b_fi_4a03.ic15", 0x10000, 0x8000, CRC(e92d9d60) SHA1(2554617e0e6615ca8c85a49299a4a0e762478339) )
	ROM_LOAD( "4-2-b_fi_4a04.ic14", 0x18000, 0x8000, CRC(b6358305) SHA1(c7bb4236a75ec6b88f011bc30f8fb9a718e2ca3e) )

	ROM_REGION( 0x20000, "sprites", ROMREGION_INVERT ) // on MOD 51/1 board, different format from the original
	ROM_LOAD32_BYTE( "51-1_fi_501.ic43", 0x0003, 0x8000, CRC(fa901876) SHA1(e27a8f9014419343bf08a7536b4b37bded611ec0) )
	ROM_LOAD32_BYTE( "51-1_fi_502.ic42", 0x0002, 0x8000, CRC(9bf4f6a4) SHA1(a4b1287258d1635108b349805f15ae25224e2ab6) )
	ROM_LOAD32_BYTE( "51-1_fi_503.ic41", 0x0001, 0x8000, CRC(b03543cf) SHA1(2b8f0e8c60dbccc40e82f4af799d151efc259d52) )
	ROM_LOAD32_BYTE( "51-1_fi_504.ic40", 0x0000, 0x8000, CRC(b80afe0a) SHA1(02efc3cb0c968d24ae0da4d16515c244f59d1d93) )

	ROM_REGION( 0x0600, "proms", 0 )    // PROMs (function unknown)
	ROM_LOAD( "1_snd_p0102_82s123.ic20",    0x000, 0x020, CRC(6228a653) SHA1(a8cfceba0546875940e9124c5ca2a6c625232856) )
	ROM_LOAD( "21-1_p0201_82s129.ic5",      0x100, 0x100, CRC(2697da58) SHA1(e62516b886ff6e204b718e5f0c6ce2712e4b7fc5) )
	ROM_LOAD( "21-1_p0203_82s129.ic12",     0x200, 0x100, CRC(d56e29b4) SHA1(d94157a9cd75d1f6d305f3d48291d0ae4c41e006) )
	ROM_LOAD( "3-sub_cpu_p0312_82s147.bin", 0x300, 0x200, CRC(28d906a7) SHA1(da11a515aa798c0fb687d8617461b0372364b332) )
	ROM_LOAD( "51-1_p0502.ic10",            0x500, 0x100, CRC(15085e44) SHA1(646e7100fcb112594023cf02be036bd3d42cc13c) )

	ROM_REGION( 0x1000, "plds", ROMREGION_ERASEFF )
	ROM_LOAD( "4-2_0400_pal16r6acn.ic29", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "4-2-b_pal16r6_p0400.ic29", 0x000, 0x104, CRC(19ac48ea) SHA1(bb79d2e1522f144609cdfe32a01defd19753668d) )
	ROM_LOAD( "51-1_p0503_pal16r6.ic46",  0x000, 0x104, CRC(07eb86d2) SHA1(482eb325df5bc60353bac85412cf45429cd03c6d) )
ROM_END

void slapfght_ms_state::init_decryption() // same as the one for blktiger_ms
{
	uint8_t *const src = memregion("maincpu")->base();
	auto const len = memregion("maincpu")->bytes();

	// bitswap data
	for (int i = 0; i < len; i++)
		src[i] = bitswap<8>(src[i], 5, 3, 7, 2, 0, 1, 4, 6);

	// descramble address
	std::vector<uint8_t> buffer(len);
	memcpy(&buffer[0], src, len);

	for (int i = 0; i < len; i++)
		src[i] = buffer[bitswap<24>(i, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 10, 8, 6, 4, 0, 2, 7, 3, 1, 9, 11, 5)];
}

} // anonymous namespace


GAME( 199?, slapfighm, alcon, slapfighm, slapfighm, slapfght_ms_state, init_decryption, ROT270, "bootleg (Gaelco / Ervisa)", "Slap Fight (Modular System)", MACHINE_IS_SKELETON )
