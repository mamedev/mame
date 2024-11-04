// license:BSD-3-Clause
// copyright-holders:Bryan McPhail

/***************************************************************************

  Vapor Trail (World version)  (c) 1989 Data East Corporation
  Vapor Trail (USA version)    (c) 1989 Data East USA
  Kuhga (Japanese version)     (c) 1989 Data East Corporation

  Notes:
  -----
  - If you activate the service mode dip switch during the gameplay, it acts
    like invincibility either for player 1 and player 2. It works for all sets.

  Emulation by Bryan McPhail, mish@tendril.co.uk
  added pal & prom-maps - Highwayman.
***************************************************************************/

#include "emu.h"

#include "decmxc06.h"
#include "deco16ic.h"

#include "cpu/h6280/h6280.h"
#include "cpu/m68000/m68000.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"
#include "sound/ymopn.h"
#include "video/bufsprite.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class vaportra_state : public driver_device
{
public:
	vaportra_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_deco_tilegen(*this, "tilegen%u", 1U)
		, m_spritegen(*this, "spritegen")
		, m_spriteram(*this, "spriteram")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "colors")
		, m_soundlatch(*this, "soundlatch")
		, m_paletteram(*this, "palette")
		, m_paletteram_ext(*this, "palette_ext")
	{ }

	void vaportra(machine_config &config);

	void driver_init();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<h6280_device> m_audiocpu;
	required_device_array<deco16ic_device, 2> m_deco_tilegen;
	required_device<deco_mxc06_device> m_spritegen;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_paletteram;
	required_shared_ptr<uint16_t> m_paletteram_ext;

	// misc
	uint16_t m_priority[2]{};

	uint8_t irq6_ack_r();
	void irq6_ack_w(uint8_t data);
	void priority_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void palette_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void palette_ext_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void colpri_cb(u32 &colour, u32 &pri_mask);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void update_palette( int offset );

	DECO16IC_BANK_CB_MEMBER(bank_callback);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/****************************************************************************

    2 Data East 55 chips for playfields (same as Dark Seal, etc)
    1 Data East MXC-06 chip for sprites (same as Bad Dudes, etc)

***************************************************************************/

/******************************************************************************/

void vaportra_state::priority_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_priority[offset]);
}

/******************************************************************************/

void vaportra_state::update_palette( int offset )
{
	uint8_t r, g, b;

	// TODO : Values aren't written in game when higher than 0xf0,
	// It's related to hardware colour resistors?
	r = (m_paletteram[offset] >> 0) & 0xff;
	g = (m_paletteram[offset] >> 8) & 0xff;
	b = (m_paletteram_ext[offset] >> 0) & 0xff;

	m_palette->set_pen_color(offset, rgb_t(r,g,b));
}

void vaportra_state::palette_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_paletteram[offset]);
	update_palette(offset);
}

void vaportra_state::palette_ext_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_paletteram_ext[offset]);
	update_palette(offset);
}

/******************************************************************************/

void vaportra_state::colpri_cb(u32 &colour, u32 &pri_mask)
{
	pri_mask = 0; // above back, mid, foreground
	if (colour >= m_priority[1])
	{
		pri_mask |= GFX_PMASK_4; // behind foreground
	}
}

uint32_t vaportra_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t const flip = m_deco_tilegen[0]->pf_control_r(0);
	int const pri = m_priority[0] & 0x03;

	screen.priority().fill(0, cliprect);
	flip_screen_set(!BIT(flip, 7));
	m_deco_tilegen[0]->pf_update(nullptr, nullptr);
	m_deco_tilegen[1]->pf_update(nullptr, nullptr);

	m_spritegen->set_flip_screen(!BIT(flip, 7));

	// Draw playfields
	if (pri == 0)
	{
		m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 2);
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 4);
	}
	else if (pri == 1)
	{
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);
		m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, 0, 2);
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 4);
	}
	else if (pri == 2)
	{
		m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 2);
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 4);
	}
	else
	{
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 2);
		m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, 0, 4);
	}

	m_spritegen->draw_sprites(screen, bitmap, cliprect, m_spriteram->buffer(), 0x800 / 2);
	m_deco_tilegen[0]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


/******************************************************************************/

uint8_t vaportra_state::irq6_ack_r()
{
	m_maincpu->set_input_line(M68K_IRQ_6, CLEAR_LINE);

	return (0);
}

void vaportra_state::irq6_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(M68K_IRQ_6, CLEAR_LINE);
}

/******************************************************************************/

void vaportra_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x100001).portr("PLAYERS");
	map(0x100002, 0x100003).portr("COINS");
	map(0x100004, 0x100005).portr("DSW");
	map(0x100000, 0x100003).w(FUNC(vaportra_state::priority_w));
	map(0x100007, 0x100007).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x200000, 0x201fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x202000, 0x203fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x240000, 0x24000f).w(m_deco_tilegen[1], FUNC(deco16ic_device::pf_control_w));
	map(0x280000, 0x281fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x282000, 0x283fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x2c0000, 0x2c000f).w(m_deco_tilegen[0], FUNC(deco16ic_device::pf_control_w));
	map(0x300000, 0x3009ff).ram().w(FUNC(vaportra_state::palette_w)).share(m_paletteram);
	map(0x304000, 0x3049ff).ram().w(FUNC(vaportra_state::palette_ext_w)).share(m_paletteram_ext);
	map(0x308001, 0x308001).rw(FUNC(vaportra_state::irq6_ack_r), FUNC(vaportra_state::irq6_ack_w));
	map(0x30c000, 0x30c001).w(m_spriteram, FUNC(buffered_spriteram16_device::write));
	map(0x318000, 0x3187ff).mirror(0xce0000).ram().share("spriteram");
	map(0xffc000, 0xffffff).ram();
}


/******************************************************************************/

void vaportra_state::sound_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x100000, 0x100001).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x110000, 0x110001).rw("ym2", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x120000, 0x120001).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x130000, 0x130001).rw("oki2", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x140000, 0x140001).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x1f0000, 0x1f1fff).ram();
}

/******************************************************************************/

static INPUT_PORTS_START( vaportra )
	PORT_START("PLAYERS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("COINS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0002, "4" )
	PORT_DIPSETTING(      0x0001, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(      0x0020, "150k, 300k and 600k" )
	PORT_DIPSETTING(      0x0030, "200k and 600k" )
	PORT_DIPSETTING(      0x0010, "300k only" )
	PORT_DIPSETTING(      0x0000, DEF_STR( None ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x8000, IP_ACTIVE_LOW )
INPUT_PORTS_END

/******************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(0,2)+8,RGN_FRAC(0,2)+0,RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+0 },
	{ STEP8(0,1) },
	{ STEP8(0,8*2) },
	8*16
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(0,2)+8,RGN_FRAC(0,2)+0,RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+0 },
	{ STEP8(16*8*2,1), STEP8(0,1) },
	{ STEP16(0,8*2) },
	32*16
};

static GFXDECODE_START( gfx_vaportra )
	GFXDECODE_ENTRY( "tiles1",  0, charlayout, 0x000, 0x500 )    // Characters 8x8
	GFXDECODE_ENTRY( "tiles1",  0, tilelayout, 0x000, 0x500 )    // Tiles 16x16
	GFXDECODE_ENTRY( "tiles2",  0, charlayout, 0x000, 0x500 )    // Characters 8x8
	GFXDECODE_ENTRY( "tiles2",  0, tilelayout, 0x000, 0x500 )    // Tiles 16x16 // ok
GFXDECODE_END

static GFXDECODE_START( gfx_vaportra_spr )
	GFXDECODE_ENTRY( "sprites", 0, tilelayout, 0x100, 16 )       // 16x16
GFXDECODE_END

/******************************************************************************/

DECO16IC_BANK_CB_MEMBER(vaportra_state::bank_callback)
{
	return ((bank >> 4) & 0x7) * 0x1000;
}

void vaportra_state::machine_start()
{
	save_item(NAME(m_priority));
}

void vaportra_state::machine_reset()
{
	m_priority[0] = 0;
	m_priority[1] = 0;
}

void vaportra_state::vaportra(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(24'000'000) / 2); // Custom chip 59
	m_maincpu->set_addrmap(AS_PROGRAM, &vaportra_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(vaportra_state::irq6_line_assert));

	H6280(config, m_audiocpu, XTAL(24'000'000) / 4); // Custom chip 45; Audio section crystal is 32.220 MHz but CPU clock is confirmed as coming from the 24MHz crystal (6Mhz exactly on the CPU)
	m_audiocpu->set_addrmap(AS_PROGRAM, &vaportra_state::sound_map);
	m_audiocpu->add_route(ALL_OUTPUTS, "mono", 0); // internal sound unused
	m_audiocpu->set_timer_scale(2);

	// video hardware
	BUFFERED_SPRITERAM16(config, m_spriteram);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(58);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(529));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(vaportra_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_vaportra);
	PALETTE(config, m_palette).set_entries(1280);

	DECO16IC(config, m_deco_tilegen[0], 0);
	m_deco_tilegen[0]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf1_col_bank(0x00);
	m_deco_tilegen[0]->set_pf2_col_bank(0x20);
	m_deco_tilegen[0]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[0]->set_bank1_callback(FUNC(vaportra_state::bank_callback));
	m_deco_tilegen[0]->set_bank2_callback(FUNC(vaportra_state::bank_callback));
	m_deco_tilegen[0]->set_pf12_8x8_bank(0);
	m_deco_tilegen[0]->set_pf12_16x16_bank(1);
	m_deco_tilegen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO16IC(config, m_deco_tilegen[1], 0);
	m_deco_tilegen[1]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf1_col_bank(0x30);
	m_deco_tilegen[1]->set_pf2_col_bank(0x40);
	m_deco_tilegen[1]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[1]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[1]->set_bank1_callback(FUNC(vaportra_state::bank_callback));
	m_deco_tilegen[1]->set_bank2_callback(FUNC(vaportra_state::bank_callback));
	m_deco_tilegen[1]->set_pf12_8x8_bank(2);
	m_deco_tilegen[1]->set_pf12_16x16_bank(3);
	m_deco_tilegen[1]->set_gfxdecode_tag(m_gfxdecode);

	DECO_MXC06(config, m_spritegen, 0, m_palette, gfx_vaportra_spr);
	m_spritegen->set_colpri_callback(FUNC(vaportra_state::colpri_cb));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, 0);

	YM2203(config, "ym1", XTAL(32'220'000) / 8).add_route(ALL_OUTPUTS, "mono", 0.60);

	ym2151_device &ym2(YM2151(config, "ym2", XTAL(32'220'000) / 9)); // uses a preset LS163 to force the odd speed
	ym2.irq_handler().set_inputline(m_audiocpu, 1); // IRQ2
	ym2.add_route(0, "mono", 0.60);
	ym2.add_route(1, "mono", 0.60);

	OKIM6295(config, "oki1", XTAL(32'220'000) / 32, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.75);

	OKIM6295(config, "oki2", XTAL(32'220'000) / 16, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.60);
}

/******************************************************************************/

ROM_START( vaportra )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "fl_02-1.bin", 0x00000, 0x20000, CRC(9ae36095) SHA1(c8d11a6033a44277a267915b4ca471c43acd1143) )
	ROM_LOAD16_BYTE( "fl_00-1.bin", 0x00001, 0x20000, CRC(c08cc048) SHA1(b28f95856817b8a8cb6cc588d48e95196cbf52fd) )
	ROM_LOAD16_BYTE( "fl_03.bin",   0x40000, 0x20000, CRC(80bd2844) SHA1(3fcaa409c7134388fa9458df8e8aaecc93f085e6) )
	ROM_LOAD16_BYTE( "fl_01.bin",   0x40001, 0x20000, CRC(9474b085) SHA1(5510309ddab5fbf1dbb0a7b1e424a5dff5ec263d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "fj04",    0x00000, 0x10000, CRC(e9aedf9b) SHA1(f7bcf8f666015140aaad8ee5cf619636934b7066) )

	ROM_REGION( 0x080000, "tiles1", 0 )  // chars & tiles
	ROM_LOAD( "vtmaa00.bin",   0x000000, 0x80000, CRC(0330e13b) SHA1(dce70667ea738295332556752d1305c5e941b383) )

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "vtmaa02.bin",   0x000000, 0x80000, CRC(091ff98e) SHA1(814dc08c055bad5368955a4b1fe6a706b58adc02) )
	ROM_LOAD( "vtmaa01.bin",   0x080000, 0x80000, CRC(c217a31b) SHA1(e259d48190d6890781fb0338e17e14822876babb) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "vtmaa03.bin",   0x000000, 0x80000, CRC(1a30bf81) SHA1(00e6c713e12133a99d64ca80638c9cbc8e26b2c8) )
	ROM_LOAD( "vtmaa04.bin",   0x080000, 0x80000, CRC(b713e9cc) SHA1(af33943d75d2ee3a7385f624537008dca9e1d5d8) )

	ROM_REGION( 0x40000, "oki1", 0 )
	ROM_LOAD( "fj06",    0x00000, 0x20000, CRC(6e98a235) SHA1(374564b4e494d03cd1330c06e321b9452c22a075) )

	ROM_REGION( 0x40000, "oki2", 0 )
	ROM_LOAD( "fj05",    0x00000, 0x20000, CRC(39cda2b5) SHA1(f5c5a305025d451ab48f84cd63e36a3bbdefda96) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "fj-27.bin",    0x00000, 0x00200, CRC(65045742) SHA1(5dfb6c85a70b208cd16d3bf8ec1897e77f4a9b7d) )

	ROM_REGION( 0x0a00, "plds", 0 )
	ROM_LOAD( "pal16l8a.6l",  0x0000, 0x0104, CRC(ee748e8f) SHA1(6ffe8b11f076305e82f64e0a12b76ffe725ce345) )
	ROM_LOAD( "pal16l8b.13g", 0x0200, 0x0104, CRC(6da13bda) SHA1(d7bade089d87015e1e95fbf3f292db4688ee4624) )
	ROM_LOAD( "pal16l8b.13h", 0x0400, 0x0104, CRC(62a9e098) SHA1(7b7c371c040d250d41fde021d191d62ce95bfc20) )
	ROM_LOAD( "pal16l8b.14g", 0x0600, 0x0104, CRC(036768aa) SHA1(96185989031e0a9b38ff29bf4cf6162482d33964) )
	ROM_LOAD( "pal16l8b.14h", 0x0800, 0x0104, CRC(bf421fce) SHA1(e8b0895b1fe99a3d5b3dcca004a7bfd1a09766b2) )
ROM_END

ROM_START( vaportra3 ) // 74 bytes of 68k code have been changed compared to vaportra set
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "fl02-3.bin", 0x00000, 0x20000, CRC(6c59be54) SHA1(ce60be53fb2cc3a26a28e8632c8638771d3db3c9) ) // == fl_02-1.bin (99.973297%)
	ROM_LOAD16_BYTE( "fl00-3.bin", 0x00001, 0x20000, CRC(69f8bef4) SHA1(7249d097c33adac9b42dd98d1328ad1c496ff927) ) // == fl_00-1.bin (99.970245%)
	ROM_LOAD16_BYTE( "fl_03.bin",   0x40000, 0x20000, CRC(80bd2844) SHA1(3fcaa409c7134388fa9458df8e8aaecc93f085e6) )
	ROM_LOAD16_BYTE( "fl_01.bin",   0x40001, 0x20000, CRC(9474b085) SHA1(5510309ddab5fbf1dbb0a7b1e424a5dff5ec263d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "fj04",    0x00000, 0x10000, CRC(e9aedf9b) SHA1(f7bcf8f666015140aaad8ee5cf619636934b7066) )

	ROM_REGION( 0x080000, "tiles1", 0 )  // chars & tiles // original DE board with mask ROM split into 4 ROMs
	ROM_LOAD16_BYTE( "fl23",   0x00000, 0x20000, CRC(6089f9e7) SHA1(a036068398a8e72c8fcf29fadaaad5a8930c2bfe) )
	ROM_LOAD16_BYTE( "fl25",   0x00001, 0x20000, CRC(3989290a) SHA1(7d2d8a334d4c206298d806eac4f2cd46e7d4f918) )
	ROM_LOAD16_BYTE( "fl24",   0x40000, 0x20000, CRC(41551bfa) SHA1(bc636fec6d610f651101656d6e9ad06656ce516a) )
	ROM_LOAD16_BYTE( "fl26",   0x40001, 0x20000, CRC(dc67fa5c) SHA1(459a1ee059d6bb2fb2c6744fffeb25b915b29e67) )

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "vtmaa02.bin",   0x000000, 0x80000, CRC(091ff98e) SHA1(814dc08c055bad5368955a4b1fe6a706b58adc02) )
	ROM_LOAD( "vtmaa01.bin",   0x080000, 0x80000, CRC(c217a31b) SHA1(e259d48190d6890781fb0338e17e14822876babb) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "vtmaa03.bin",   0x000000, 0x80000, CRC(1a30bf81) SHA1(00e6c713e12133a99d64ca80638c9cbc8e26b2c8) )
	ROM_LOAD( "vtmaa04.bin",   0x080000, 0x80000, CRC(b713e9cc) SHA1(af33943d75d2ee3a7385f624537008dca9e1d5d8) )

	ROM_REGION( 0x40000, "oki1", 0 )
	ROM_LOAD( "fj06",    0x00000, 0x20000, CRC(6e98a235) SHA1(374564b4e494d03cd1330c06e321b9452c22a075) )

	ROM_REGION( 0x40000, "oki2", 0 )
	ROM_LOAD( "fj05",    0x00000, 0x20000, CRC(39cda2b5) SHA1(f5c5a305025d451ab48f84cd63e36a3bbdefda96) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "fj-27.bin",    0x00000, 0x00200, CRC(65045742) SHA1(5dfb6c85a70b208cd16d3bf8ec1897e77f4a9b7d) )

	ROM_REGION( 0x0a00, "plds", 0 )
	ROM_LOAD( "pal16l8a.6l",  0x0000, 0x0104, CRC(ee748e8f) SHA1(6ffe8b11f076305e82f64e0a12b76ffe725ce345) )
	ROM_LOAD( "pal16l8b.13g", 0x0200, 0x0104, CRC(6da13bda) SHA1(d7bade089d87015e1e95fbf3f292db4688ee4624) )
	ROM_LOAD( "pal16l8b.13h", 0x0400, 0x0104, CRC(62a9e098) SHA1(7b7c371c040d250d41fde021d191d62ce95bfc20) )
	ROM_LOAD( "pal16l8b.14g", 0x0600, 0x0104, CRC(036768aa) SHA1(96185989031e0a9b38ff29bf4cf6162482d33964) )
	ROM_LOAD( "pal16l8b.14h", 0x0800, 0x0104, CRC(bf421fce) SHA1(e8b0895b1fe99a3d5b3dcca004a7bfd1a09766b2) )
ROM_END

ROM_START( vaportrau )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "fj02",   0x00000, 0x20000, CRC(a2affb73) SHA1(0d49397cc9891047a0b92e92e2e3d0e7fcaf8db9) )
	ROM_LOAD16_BYTE( "fj00",   0x00001, 0x20000, CRC(ef05e07b) SHA1(0e505709fa251e6b30f019c0c28ee9ba2b29a50a) )
	ROM_LOAD16_BYTE( "fj03",   0x40000, 0x20000, CRC(44893379) SHA1(da1340bc1821a552c317cb9a7c1ba69eb080b055) )
	ROM_LOAD16_BYTE( "fj01",   0x40001, 0x20000, CRC(97fbc107) SHA1(b2899eb4347c0471397b83051e46c94dff3526f5) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "fj04",    0x00000, 0x10000, CRC(e9aedf9b) SHA1(f7bcf8f666015140aaad8ee5cf619636934b7066) )

	ROM_REGION( 0x080000, "tiles1", 0 )  // chars & tiles
	ROM_LOAD( "vtmaa00.bin",   0x000000, 0x80000, CRC(0330e13b) SHA1(dce70667ea738295332556752d1305c5e941b383) )

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "vtmaa02.bin",   0x000000, 0x80000, CRC(091ff98e) SHA1(814dc08c055bad5368955a4b1fe6a706b58adc02) )
	ROM_LOAD( "vtmaa01.bin",   0x080000, 0x80000, CRC(c217a31b) SHA1(e259d48190d6890781fb0338e17e14822876babb) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "vtmaa03.bin",   0x000000, 0x80000, CRC(1a30bf81) SHA1(00e6c713e12133a99d64ca80638c9cbc8e26b2c8) )
	ROM_LOAD( "vtmaa04.bin",   0x080000, 0x80000, CRC(b713e9cc) SHA1(af33943d75d2ee3a7385f624537008dca9e1d5d8) )

	ROM_REGION( 0x40000, "oki1", 0 )
	ROM_LOAD( "fj06",    0x00000, 0x20000, CRC(6e98a235) SHA1(374564b4e494d03cd1330c06e321b9452c22a075) )

	ROM_REGION( 0x40000, "oki2", 0 )
	ROM_LOAD( "fj05",    0x00000, 0x20000, CRC(39cda2b5) SHA1(f5c5a305025d451ab48f84cd63e36a3bbdefda96) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "fj-27.bin",    0x00000, 0x00200, CRC(65045742) SHA1(5dfb6c85a70b208cd16d3bf8ec1897e77f4a9b7d) )

	ROM_REGION( 0x0a00, "plds", 0 )
	ROM_LOAD( "pal16l8a.6l",  0x0000, 0x0104, CRC(ee748e8f) SHA1(6ffe8b11f076305e82f64e0a12b76ffe725ce345) )
	ROM_LOAD( "pal16l8b.13g", 0x0200, 0x0104, CRC(6da13bda) SHA1(d7bade089d87015e1e95fbf3f292db4688ee4624) )
	ROM_LOAD( "pal16l8b.13h", 0x0400, 0x0104, CRC(62a9e098) SHA1(7b7c371c040d250d41fde021d191d62ce95bfc20) )
	ROM_LOAD( "pal16l8b.14g", 0x0600, 0x0104, CRC(036768aa) SHA1(96185989031e0a9b38ff29bf4cf6162482d33964) )
	ROM_LOAD( "pal16l8b.14h", 0x0800, 0x0104, CRC(bf421fce) SHA1(e8b0895b1fe99a3d5b3dcca004a7bfd1a09766b2) )
ROM_END

ROM_START( kuhga )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "fp02-3.bin", 0x00000, 0x20000, CRC(d0705ef4) SHA1(781efbf36d9dda543895e0a59cd4d72667439a93) )
	ROM_LOAD16_BYTE( "fp00-3.bin", 0x00001, 0x20000, CRC(1da92e48) SHA1(6507bd9bbc31ee03e38b82cc135aebf090902761) )
	ROM_LOAD16_BYTE( "fp03.bin",   0x40000, 0x20000, CRC(ea0da0f1) SHA1(ca40e694cb0aa0c13672c14fd4a389bc6d26cbc6) )
	ROM_LOAD16_BYTE( "fp01.bin",   0x40001, 0x20000, CRC(e3ecbe86) SHA1(382e959111ec37ad94da8fd6dcefe2d2aab346b6) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "fj04",    0x00000, 0x10000, CRC(e9aedf9b) SHA1(f7bcf8f666015140aaad8ee5cf619636934b7066) )

	ROM_REGION( 0x080000, "tiles1", 0 )  // chars & tiles
	ROM_LOAD( "vtmaa00.bin",   0x000000, 0x80000, CRC(0330e13b) SHA1(dce70667ea738295332556752d1305c5e941b383) )

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "vtmaa02.bin",   0x000000, 0x80000, CRC(091ff98e) SHA1(814dc08c055bad5368955a4b1fe6a706b58adc02) )
	ROM_LOAD( "vtmaa01.bin",   0x080000, 0x80000, CRC(c217a31b) SHA1(e259d48190d6890781fb0338e17e14822876babb) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "vtmaa03.bin",   0x000000, 0x80000, CRC(1a30bf81) SHA1(00e6c713e12133a99d64ca80638c9cbc8e26b2c8) )
	ROM_LOAD( "vtmaa04.bin",   0x080000, 0x80000, CRC(b713e9cc) SHA1(af33943d75d2ee3a7385f624537008dca9e1d5d8) )

	ROM_REGION( 0x40000, "oki1", 0 )
	ROM_LOAD( "fj06",    0x00000, 0x20000, CRC(6e98a235) SHA1(374564b4e494d03cd1330c06e321b9452c22a075) )

	ROM_REGION( 0x40000, "oki2", 0 )
	ROM_LOAD( "fj05",    0x00000, 0x20000, CRC(39cda2b5) SHA1(f5c5a305025d451ab48f84cd63e36a3bbdefda96) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "fj-27.bin",    0x00000, 0x00200, CRC(65045742) SHA1(5dfb6c85a70b208cd16d3bf8ec1897e77f4a9b7d) )
ROM_END

/******************************************************************************/

void vaportra_state::driver_init()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0x00000; i < 0x80000; i++)
		rom[i] = bitswap<8>(rom[i], 0, 6, 5, 4, 3, 2, 1, 7);
}

/******************************************************************************/

} // anonymous namespace


GAME( 1989, vaportra,  0,        vaportra, vaportra, vaportra_state, driver_init, ROT270, "Data East Corporation", "Vapor Trail - Hyper Offence Formation (World revision 1)",  MACHINE_SUPPORTS_SAVE )
GAME( 1989, vaportra3, vaportra, vaportra, vaportra, vaportra_state, driver_init, ROT270, "Data East Corporation", "Vapor Trail - Hyper Offence Formation (World revision 3?)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, vaportrau, vaportra, vaportra, vaportra, vaportra_state, driver_init, ROT270, "Data East USA",         "Vapor Trail - Hyper Offence Formation (US)",                MACHINE_SUPPORTS_SAVE )
GAME( 1989, kuhga,     vaportra, vaportra, vaportra, vaportra_state, driver_init, ROT270, "Data East Corporation", "Kuhga - Operation Code 'Vapor Trail' (Japan revision 3)",   MACHINE_SUPPORTS_SAVE )
