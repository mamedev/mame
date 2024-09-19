// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    Dark Seal (Rev 3)    (c) 1990 Data East Corporation (World version)
    Dark Seal (Rev 1)    (c) 1990 Data East Corporation (World version)
    Dark Seal (Rev 4)    (c) 1990 Data East Corporation (Japanese version)
    Gate Of Doom (Rev 4) (c) 1990 Data East Corporation (USA version)
    Gate of Doom (Rev 1) (c) 1990 Data East Corporation (USA version)

    Board:  DE-0332-2

    Emulation by Bryan McPhail, mish@tendril.co.uk

    2008-08
    Dip locations verified with the manual.

***************************************************************************/

#include "emu.h"

#include "deco16ic.h"
#include "decospr.h"

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

class darkseal_state : public driver_device
{
public:
	darkseal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_palette(*this, "colors")
		, m_deco_tilegen(*this, "tilegen%u", 1U)
		, m_sprgen(*this, "spritegen")
		, m_spriteram(*this, "spriteram")
		, m_soundlatch(*this, "soundlatch")
		, m_pf1_rowscroll(*this, "pf1_rowscroll")
		, m_pf3_rowscroll(*this, "pf3_rowscroll")
		, m_paletteram(*this, "palette")
		, m_paletteram_ext(*this, "palette_ext")
	{ }

	void darkseal(machine_config &config);

	void driver_init();

private:
	void irq_ack_w(uint16_t data);
	void palette_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void palette_ext_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void update_palette(int offset);
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<h6280_device> m_audiocpu;
	required_device<palette_device> m_palette;
	required_device_array<deco16ic_device, 2> m_deco_tilegen;
	required_device<decospr_device> m_sprgen;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_pf1_rowscroll;
	required_shared_ptr<uint16_t> m_pf3_rowscroll;
	required_shared_ptr<uint16_t> m_paletteram;
	required_shared_ptr<uint16_t> m_paletteram_ext;
};



/**************************************************************************

 uses 2x DECO55 tilemaps

 Sprite/Tilemap Priority Note (is this implemented?)

    Word 4:
        Mask 0x8000 - ?
        Mask 0x4000 - Sprite is drawn beneath top 8 pens of playfield 4
        Mask 0x3e00 - Colour (32 palettes, most games only use 16)
        Mask 0x01ff - X coordinate

***************************************************************************/


/******************************************************************************/

void darkseal_state::update_palette(int offset)
{
	// TODO : Values aren't written in game when higher than 0xf0,
	// It's related to hardware colour resistors?
	int const r = (m_paletteram[offset] >> 0) & 0xff;
	int const g = (m_paletteram[offset] >> 8) & 0xff;
	int const b = (m_paletteram_ext[offset] >> 0) & 0xff;

	m_palette->set_pen_color(offset,rgb_t(r, g, b));
}

void darkseal_state::palette_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_paletteram[offset]);
	update_palette(offset);
}

void darkseal_state::palette_ext_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_paletteram_ext[offset]);
	update_palette(offset);
}

/******************************************************************************/

uint32_t darkseal_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t const flip = m_deco_tilegen[1]->pf_control_r(0);
	flip_screen_set(!BIT(flip, 7));
	m_sprgen->set_flip_screen(!BIT(flip, 7));

	bitmap.fill(m_palette->black_pen(), cliprect);

	m_deco_tilegen[0]->pf_update(m_pf1_rowscroll, m_pf1_rowscroll);
	m_deco_tilegen[1]->pf_update(m_pf3_rowscroll, m_pf3_rowscroll);

	m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);

	m_deco_tilegen[0]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram->buffer(), 0x400);
	m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

/******************************************************************************/


/******************************************************************************/

void darkseal_state::irq_ack_w(uint16_t data)
{
	m_maincpu->set_input_line(M68K_IRQ_6, CLEAR_LINE);
}

/******************************************************************************/

void darkseal_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x103fff).ram();
	map(0x120000, 0x1207ff).ram().share("spriteram");
	map(0x140000, 0x140fff).ram().w(FUNC(darkseal_state::palette_w)).share(m_paletteram);
	map(0x141000, 0x141fff).ram().w(FUNC(darkseal_state::palette_ext_w)).share(m_paletteram_ext);
	map(0x180000, 0x180001).portr("DSW");
	map(0x180002, 0x180003).portr("P1_P2");
	map(0x180004, 0x180005).portr("SYSTEM");
	map(0x180006, 0x180007).nopr().w(m_spriteram, FUNC(buffered_spriteram16_device::write));
	map(0x180008, 0x180009).w(m_soundlatch, FUNC(generic_latch_8_device::write)).umask16(0x00ff).cswidth(16);
	map(0x18000a, 0x18000b).nopr().w(FUNC(darkseal_state::irq_ack_w));

	map(0x200000, 0x201fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x202000, 0x203fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x240000, 0x24000f).w(m_deco_tilegen[1], FUNC(deco16ic_device::pf_control_w));

	map(0x220000, 0x220fff).ram().share(m_pf1_rowscroll);
	// pf2 & 4 rowscrolls are where? (maybe don't exist?)
	map(0x222000, 0x222fff).ram().share(m_pf3_rowscroll);

	map(0x260000, 0x261fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x262000, 0x263fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x2a0000, 0x2a000f).w(m_deco_tilegen[0], FUNC(deco16ic_device::pf_control_w));
}

/******************************************************************************/

void darkseal_state::sound_map(address_map &map)
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

static INPUT_PORTS_START( darkseal )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )   // button 3 - unused
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )   // button 3 - unused
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW1:8" )    // Manual says 'Always OFF'

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0100, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "4" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x3000, 0x3000, "Energy" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x1000, "2.5" )
	PORT_DIPSETTING(      0x3000, "3" )
	PORT_DIPSETTING(      0x2000, "4" )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

/******************************************************************************/

static const gfx_layout charlayout =
{
	8,8,    // 8*8 chars
	RGN_FRAC(1,2),
	4,      // 4 bits per pixel
	{ 8, 0, RGN_FRAC(1,2)+8, RGN_FRAC(1,2) },
	{ STEP8(0,1) },
	{ STEP8(0,8*2) },
	8*8*2 // every char takes 8 consecutive bytes
};

static const gfx_layout seallayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 8, 0, RGN_FRAC(1,2)+8, RGN_FRAC(1,2) },
	{ STEP8(16*8*2,1), STEP8(0,1) },
	{ STEP16(0,8*2) },
	64*8
};

static GFXDECODE_START( gfx_darkseal )
	GFXDECODE_ENTRY( "chars",   0, charlayout,    0, 16 )  // 8x8
	GFXDECODE_ENTRY( "tiles1",  0, seallayout,  768, 16 )  // 16x16
	GFXDECODE_ENTRY( "tiles2",  0, seallayout, 1024, 16 )  // 16x16
GFXDECODE_END

static GFXDECODE_START( gfx_darkseal_spr )
	GFXDECODE_ENTRY( "sprites", 0, seallayout,  256, 32 )  // 16x16
GFXDECODE_END

/******************************************************************************/

void darkseal_state::darkseal(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(24'000'000) / 2); // Custom chip 59
	m_maincpu->set_addrmap(AS_PROGRAM, &darkseal_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(darkseal_state::irq6_line_assert)); // VBL

	H6280(config, m_audiocpu, XTAL(32'220'000) / 4); // Custom chip 45, Audio section crystal is 32.220 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &darkseal_state::sound_map);
	m_audiocpu->add_route(ALL_OUTPUTS, "mono", 0); // internal sound unused
	m_audiocpu->set_timer_scale(2);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(58);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(529));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(darkseal_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_darkseal);
	PALETTE(config, m_palette).set_entries(2048);

	BUFFERED_SPRITERAM16(config, m_spriteram);

	DECO16IC(config, m_deco_tilegen[0], 0);
	m_deco_tilegen[0]->set_pf1_size(DECO_64x64);
	m_deco_tilegen[0]->set_pf2_size(DECO_64x64);     // both these tilemaps need to be twice the y size of usual!
	m_deco_tilegen[0]->set_pf1_col_bank(0x00);
	m_deco_tilegen[0]->set_pf2_col_bank(0x00);
	m_deco_tilegen[0]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf12_8x8_bank(0);
	m_deco_tilegen[0]->set_pf12_16x16_bank(1);
	m_deco_tilegen[0]->set_gfxdecode_tag("gfxdecode");

	DECO16IC(config, m_deco_tilegen[1], 0);
	m_deco_tilegen[1]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf1_col_bank(0x00);
	m_deco_tilegen[1]->set_pf2_col_bank(0x00);
	m_deco_tilegen[1]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[1]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[1]->set_pf12_8x8_bank(0);
	m_deco_tilegen[1]->set_pf12_16x16_bank(2);
	m_deco_tilegen[1]->set_gfxdecode_tag("gfxdecode");

	DECO_SPRITE(config, m_sprgen, 0, m_palette, gfx_darkseal_spr);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, 0);

	YM2203(config, "ym1", XTAL(32'220'000) / 8).add_route(ALL_OUTPUTS, "mono", 0.45);

	ym2151_device &ym2(YM2151(config, "ym2", XTAL(32'220'000) / 9));
	ym2.irq_handler().set_inputline(m_audiocpu, 1); // IRQ2
	ym2.add_route(0, "mono", 0.55);
	ym2.add_route(1, "mono", 0.55);

	OKIM6295(config, "oki1", XTAL(32'220'000) / 32, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);

	OKIM6295(config, "oki2", XTAL(32'220'000) / 16, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.60);
}

/******************************************************************************/

ROM_START( darkseal )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "ga_04-3.j12", 0x00000, 0x20000, CRC(bafad556) SHA1(5bd8a787d41a33919701ced29212bc11301e31d9) )
	ROM_LOAD16_BYTE( "ga_01-3.h14", 0x00001, 0x20000, CRC(f409050e) SHA1(4653094aeb5dd7ba1e490c04897a23ba8990df3c) )
	ROM_LOAD16_BYTE( "ga_00.h12",   0x40000, 0x20000, CRC(fbf3ac63) SHA1(51af581ee951eedeb4aa413ecbebe8bf4d30613b) )
	ROM_LOAD16_BYTE( "ga_05.j14",   0x40001, 0x20000, CRC(d5e3ae3f) SHA1(12f6e92af115422c6ab6ef1d33675d1e1cd58e10) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "fz_06-1.j15", 0x00000, 0x10000, CRC(c4828a6d) SHA1(fbfd0c85730bbe18401879cd68c19aaec9d482d8) )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD16_BYTE( "fz_02.j1", 0x000001, 0x10000, CRC(3c9c3012) SHA1(086c2123725d4aa32838c0b6c82317d9c789c465) )
	ROM_LOAD16_BYTE( "fz_03.j2", 0x000000, 0x10000, CRC(264b90ed) SHA1(0bb1557673107c2d732a9374d5601a6eaf229473) )

	ROM_REGION( 0x080000, "tiles1", 0 )
	ROM_LOAD( "mac-03.h3", 0x000000, 0x80000, CRC(9996f3dc) SHA1(fffd9ecfe142a0c7c3c9c521778ff9c55ea8b225) )

	ROM_REGION( 0x080000, "tiles2", 0 )
	ROM_LOAD( "mac-02.e20", 0x000000, 0x80000, CRC(49504e89) SHA1(6da4733a650b9040abb2a81a49476368b514b5ab) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "mac-00.b1", 0x000000, 0x80000, CRC(52acf1d6) SHA1(a7b68782417baafc86371b106fd31c5317f5b3d8) )
	ROM_LOAD( "mac-01.b3", 0x080000, 0x80000, CRC(b28f7584) SHA1(e02ddd45130a7b50f80b6dd049059dba8071d768) )

	ROM_REGION( 0x40000, "oki1", 0 )
	ROM_LOAD( "fz_08.l17", 0x00000, 0x20000, CRC(c9bf68e1) SHA1(c81e2534a814fe44c8787946a9fbe18f1743c3b4) )

	ROM_REGION( 0x40000, "oki2", 0 )
	ROM_LOAD( "fz_07.k14", 0x00000, 0x20000, CRC(588dd3cb) SHA1(16c4e7670a4967768ddbfd52939d4e6e42268441) )
ROM_END

ROM_START( darkseal1 )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "fz_04-4.j12", 0x00000, 0x20000, CRC(a1a985a9) SHA1(eac3f43ff4016dcc21fe34b6bfed36e0d4b86959) ) // sldh w/darksealj
	ROM_LOAD16_BYTE( "fz_01-1.h14", 0x00001, 0x20000, CRC(98bd2940) SHA1(88ac727c3797e646834266320a71aa159e2b2541) )
	ROM_LOAD16_BYTE( "fz_00-2.h12", 0x40000, 0x20000, CRC(fbf3ac63) SHA1(51af581ee951eedeb4aa413ecbebe8bf4d30613b) ) // sldh w/darksealj
	ROM_LOAD16_BYTE( "fz_05-2.j14", 0x40001, 0x20000, CRC(d5e3ae3f) SHA1(12f6e92af115422c6ab6ef1d33675d1e1cd58e10) ) // sldh w/darksealj

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "fz_06-1.j15", 0x00000, 0x10000, CRC(c4828a6d) SHA1(fbfd0c85730bbe18401879cd68c19aaec9d482d8) )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD16_BYTE( "fz_02-1.j1", 0x000001, 0x10000, CRC(3c9c3012) SHA1(086c2123725d4aa32838c0b6c82317d9c789c465) )
	ROM_LOAD16_BYTE( "fz_03-1.j2", 0x000000, 0x10000, CRC(264b90ed) SHA1(0bb1557673107c2d732a9374d5601a6eaf229473) )

	ROM_REGION( 0x080000, "tiles1", 0 )
	ROM_LOAD( "mac-03.h3", 0x000000, 0x80000, CRC(9996f3dc) SHA1(fffd9ecfe142a0c7c3c9c521778ff9c55ea8b225) )

	ROM_REGION( 0x080000, "tiles2", 0 )
	ROM_LOAD( "mac-02.e20", 0x000000, 0x80000, CRC(49504e89) SHA1(6da4733a650b9040abb2a81a49476368b514b5ab) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "mac-00.b1", 0x000000, 0x80000, CRC(52acf1d6) SHA1(a7b68782417baafc86371b106fd31c5317f5b3d8) )
	ROM_LOAD( "mac-01.b3", 0x080000, 0x80000, CRC(b28f7584) SHA1(e02ddd45130a7b50f80b6dd049059dba8071d768) )

	ROM_REGION( 0x40000, "oki1", 0 )
	ROM_LOAD( "fz_08-1.k17", 0x00000, 0x20000, CRC(c9bf68e1) SHA1(c81e2534a814fe44c8787946a9fbe18f1743c3b4) )

	ROM_REGION( 0x40000, "oki2", 0 )
	ROM_LOAD( "fz_07-.k14", 0x00000, 0x20000, CRC(588dd3cb) SHA1(16c4e7670a4967768ddbfd52939d4e6e42268441) )
ROM_END

ROM_START( darksealj )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "fz_04-4.j12", 0x00000, 0x20000, CRC(817faa2c) SHA1(8a79703f0e3aeb2ceeb098466561ab604baef301) ) // sldh w/darkseal1
	ROM_LOAD16_BYTE( "fz_01-4.h14", 0x00001, 0x20000, CRC(373caeee) SHA1(5cfa0c7672c439e9d011d9ec93da32c2377dce19) )
	ROM_LOAD16_BYTE( "fz_00-2.h12", 0x40000, 0x20000, CRC(1ab99aa7) SHA1(1da51f3ee0d15094911d4090264b945090d51242) ) // sldh w/darkseal1
	ROM_LOAD16_BYTE( "fz_05-2.j14", 0x40001, 0x20000, CRC(3374ef8c) SHA1(4144e71e452e281078bcd9b9a996db9f5dccc346) ) // sldh w/darkseal1

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "fz_06-1.j15", 0x00000, 0x10000, CRC(c4828a6d) SHA1(fbfd0c85730bbe18401879cd68c19aaec9d482d8) )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD16_BYTE( "fz_02.j1", 0x000001, 0x10000, CRC(3c9c3012) SHA1(086c2123725d4aa32838c0b6c82317d9c789c465) )
	ROM_LOAD16_BYTE( "fz_03.j2", 0x000000, 0x10000, CRC(264b90ed) SHA1(0bb1557673107c2d732a9374d5601a6eaf229473) )

	ROM_REGION( 0x080000, "tiles1", 0 )
	ROM_LOAD( "mac-03.h3", 0x000000, 0x80000, CRC(9996f3dc) SHA1(fffd9ecfe142a0c7c3c9c521778ff9c55ea8b225) )

	ROM_REGION( 0x080000, "tiles2", 0 )
	ROM_LOAD( "mac-02.e20", 0x000000, 0x80000, CRC(49504e89) SHA1(6da4733a650b9040abb2a81a49476368b514b5ab) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "mac-00.b1", 0x000000, 0x80000, CRC(52acf1d6) SHA1(a7b68782417baafc86371b106fd31c5317f5b3d8) )
	ROM_LOAD( "mac-01.b3", 0x080000, 0x80000, CRC(b28f7584) SHA1(e02ddd45130a7b50f80b6dd049059dba8071d768) )

	ROM_REGION( 0x40000, "oki1", 0 )
	ROM_LOAD( "fz_08.l17", 0x00000, 0x20000, CRC(c9bf68e1) SHA1(c81e2534a814fe44c8787946a9fbe18f1743c3b4) )

	ROM_REGION( 0x40000, "oki2", 0 )
	ROM_LOAD( "fz_07.k14", 0x00000, 0x20000, CRC(588dd3cb) SHA1(16c4e7670a4967768ddbfd52939d4e6e42268441) )
ROM_END

ROM_START( gatedoom )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "gb_04-4.j12", 0x00000, 0x20000, CRC(8e3a0bfd) SHA1(1d20bd678a630e2006c7f50f4d13656136db3721) )
	ROM_LOAD16_BYTE( "gb_01-4.h14", 0x00001, 0x20000, CRC(8d0fd383) SHA1(797e3cf43c9315b4195232eb1787a2292af4901b) )
	ROM_LOAD16_BYTE( "gb_00.h12",   0x40000, 0x20000, CRC(a88c16a1) SHA1(e02d5470692f23afa658b9bda933bb20be64602f) )
	ROM_LOAD16_BYTE( "gb_05.j14",   0x40001, 0x20000, CRC(252d7e14) SHA1(b2f27cd9686dfc697f3faca74d20b298a59efab2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "fz_06-1.j15", 0x00000, 0x10000, CRC(c4828a6d) SHA1(fbfd0c85730bbe18401879cd68c19aaec9d482d8) )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD16_BYTE( "fz_02.j1", 0x000001, 0x10000, CRC(3c9c3012) SHA1(086c2123725d4aa32838c0b6c82317d9c789c465) )
	ROM_LOAD16_BYTE( "fz_03.j2", 0x000000, 0x10000, CRC(264b90ed) SHA1(0bb1557673107c2d732a9374d5601a6eaf229473) )

	ROM_REGION( 0x080000, "tiles1", 0 )
	ROM_LOAD( "mac-03.h3", 0x000000, 0x80000, CRC(9996f3dc) SHA1(fffd9ecfe142a0c7c3c9c521778ff9c55ea8b225) )

	ROM_REGION( 0x080000, "tiles2", 0 )
	ROM_LOAD( "mac-02.e20", 0x000000, 0x80000, CRC(49504e89) SHA1(6da4733a650b9040abb2a81a49476368b514b5ab) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "mac-00.b1", 0x000000, 0x80000, CRC(52acf1d6) SHA1(a7b68782417baafc86371b106fd31c5317f5b3d8) )
	ROM_LOAD( "mac-01.b3", 0x080000, 0x80000, CRC(b28f7584) SHA1(e02ddd45130a7b50f80b6dd049059dba8071d768) )

	ROM_REGION( 0x40000, "oki1", 0 )
	ROM_LOAD( "fz_08.l17", 0x00000, 0x20000, CRC(c9bf68e1) SHA1(c81e2534a814fe44c8787946a9fbe18f1743c3b4) )

	ROM_REGION( 0x40000, "oki2", 0 )
	ROM_LOAD( "fz_07.k14", 0x00000, 0x20000, CRC(588dd3cb) SHA1(16c4e7670a4967768ddbfd52939d4e6e42268441) )
ROM_END

ROM_START( gatedoom1 )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "gb_04.j12", 0x00000, 0x20000, CRC(4c3bbd2b) SHA1(e74a532edd01a559d0c388b37a2ead98c19fe5de) )
	ROM_LOAD16_BYTE( "gb_01.h14", 0x00001, 0x20000, CRC(59e367f4) SHA1(f88fa23b8e91f312103eb8a1d9a91d8171ec3ad4) )
	ROM_LOAD16_BYTE( "gb_00.h12", 0x40000, 0x20000, CRC(a88c16a1) SHA1(e02d5470692f23afa658b9bda933bb20be64602f) )
	ROM_LOAD16_BYTE( "gb_05.j14", 0x40001, 0x20000, CRC(252d7e14) SHA1(b2f27cd9686dfc697f3faca74d20b298a59efab2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "fz_06-1.j15", 0x00000, 0x10000, CRC(c4828a6d) SHA1(fbfd0c85730bbe18401879cd68c19aaec9d482d8) )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD16_BYTE( "fz_02.j1", 0x000001, 0x10000, CRC(3c9c3012) SHA1(086c2123725d4aa32838c0b6c82317d9c789c465) )
	ROM_LOAD16_BYTE( "fz_03.j2", 0x000000, 0x10000, CRC(264b90ed) SHA1(0bb1557673107c2d732a9374d5601a6eaf229473) )

	ROM_REGION( 0x080000, "tiles1", 0 )
	ROM_LOAD( "mac-03.h3", 0x000000, 0x80000, CRC(9996f3dc) SHA1(fffd9ecfe142a0c7c3c9c521778ff9c55ea8b225) )

	ROM_REGION( 0x080000, "tiles2", 0 )
	ROM_LOAD( "mac-02.e20", 0x000000, 0x80000, CRC(49504e89) SHA1(6da4733a650b9040abb2a81a49476368b514b5ab) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "mac-00.b1", 0x000000, 0x80000, CRC(52acf1d6) SHA1(a7b68782417baafc86371b106fd31c5317f5b3d8) )
	ROM_LOAD( "mac-01.b3", 0x080000, 0x80000, CRC(b28f7584) SHA1(e02ddd45130a7b50f80b6dd049059dba8071d768) )

	ROM_REGION( 0x40000, "oki1", 0 )
	ROM_LOAD( "fz_08.l17", 0x00000, 0x20000, CRC(c9bf68e1) SHA1(c81e2534a814fe44c8787946a9fbe18f1743c3b4) )

	ROM_REGION( 0x40000, "oki2", 0 )
	ROM_LOAD( "fz_07.k14", 0x00000, 0x20000, CRC(588dd3cb) SHA1(16c4e7670a4967768ddbfd52939d4e6e42268441) )
ROM_END

/******************************************************************************/

void darkseal_state::driver_init()
{
	uint8_t *rom = memregion("maincpu")->base();
	for (int i = 0x00000; i < 0x80000; i++)
		rom[i]=(rom[i] & 0xbd) | ((rom[i] & 0x02) << 5) | ((rom[i] & 0x40) >> 5);
}

/******************************************************************************/

} // anonymous namespace


GAME( 1990, darkseal,  0,        darkseal, darkseal, darkseal_state, driver_init, ROT0, "Data East Corporation", "Dark Seal (World revision 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, darkseal1, darkseal, darkseal, darkseal, darkseal_state, driver_init, ROT0, "Data East Corporation", "Dark Seal (World revision 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, darksealj, darkseal, darkseal, darkseal, darkseal_state, driver_init, ROT0, "Data East Corporation", "Dark Seal (Japan revision 4)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, gatedoom,  darkseal, darkseal, darkseal, darkseal_state, driver_init, ROT0, "Data East Corporation", "Gate of Doom (US revision 4)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, gatedoom1, darkseal, darkseal, darkseal, darkseal_state, driver_init, ROT0, "Data East Corporation", "Gate of Doom (US revision 1)", MACHINE_SUPPORTS_SAVE )
