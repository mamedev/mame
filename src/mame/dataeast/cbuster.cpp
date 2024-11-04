// license:BSD-3-Clause
// copyright-holders: Bryan McPhail

/***************************************************************************

  Crude Buster (World version FX)       (c) 1990 Data East Corporation
  Crude Buster (World version FU)       (c) 1990 Data East Corporation
  Crude Buster (Japanese version)       (c) 1990 Data East Corporation
  Two Crude (USA version)               (c) 1990 Data East USA

  The 'FX' board is filled with 'FU' ROMs except for the 4 program ROMs,
  both boards have 'export' stickers which usually indicates a World version.
  Maybe one is a UK or European version.  Some boards exist with 'FU-1' ROMs
  which are binary identical to the FX ROMs.

  The FX/FU-1 ROMs are largely translation corrections from the first revision:

  THEIR SOLDIERS ARE ARMED WITH THE ADVANCED AND BIZARRE WEAPONS
  -> THEIR SOLDIERS ARE ARMED WITH THE MOST ADVANCED AND BIZARRE WEAPONS.
  etc

  DE-0333-3 PCB

  Data East 59 - 64 pin (68k)
  Data East 55 - 160 pin (x2) (tilemaps)
  Data East 45 (sticker) - 80 pin (Hu6280)
  Data East 52 - 128 pin (sprites)

  OSC: 24MHz & 32.22MHz
  YM2203C, YM2152, OKI 6295 x 2
  2 x 8-position Dipswitches

  (no obvious protection chip, probably a PAL?)

  Emulation by Bryan McPhail, mish@tendril.co.uk

  2008-07
  Dip locations and settings verified with manual (JPN version)

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


// configurable logging
#define LOG_PROT (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_PROT)

#include "logmacro.h"

#define LOGPROT(...) LOGMASKED(LOG_PROT, __VA_ARGS__)


namespace {

class cbuster_state : public driver_device
{
public:
	cbuster_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_deco_tilegen(*this, "tilegen%u", 1U)
		, m_palette(*this, "palette")
		, m_spriteram(*this, "spriteram")
		, m_soundlatch(*this, "soundlatch")
		, m_sprgen(*this, "spritegen")
		, m_pf_rowscroll(*this, "pf%u_rowscroll", 1U)
	{ }

	void init_twocrude();

	void twocrude(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<h6280_device> m_audiocpu;
	required_device_array<deco16ic_device, 2> m_deco_tilegen;
	required_device<palette_device> m_palette;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<decospr_device> m_sprgen;

	// memory pointers
	required_shared_ptr_array<u16, 4> m_pf_rowscroll;

	// misc
	u16 m_prot = 0U;
	u8 m_pri = 0U;

	void prot_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 prot_r();
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECO16IC_BANK_CB_MEMBER(bank_callback);
	static rgb_t xbgr_888(u32 raw);
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/******************************************************************************/

void cbuster_state::video_start()
{
	m_sprgen->alloc_sprite_bitmap();
}

/*
    Crude Buster palette is a little strange compared to other Data East games
    of this period.  Although the digital palette is 8 bits per channel, the
    analog 'white' level is set at 0x8e.  In hardware this is done at the
    final resistors before the JAMMA connector.  It also suggests that if the
    game were to use any values above 0x8e (it doesn't) then the final output
    voltage would be out of spec.

    I suspect this setup is actually software compensating for a hardware
    design problem.
*/
rgb_t cbuster_state::xbgr_888(u32 raw)
{
	int r = (raw >>  0) & 0xff;
	int g = (raw >>  8) & 0xff;
	int b = (raw >> 16) & 0xff;

	if (r > 0x8e) r = 0x8e;
	if (g > 0x8e) g = 0x8e;
	if (b > 0x8e) b = 0x8e;

	r = (r * 255) / 0x8e;
	g = (g * 255) / 0x8e;
	b = (b * 255) / 0x8e;

	return rgb_t(r, g, b);
}

u32 cbuster_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const u16 flip = m_deco_tilegen[0]->pf_control_r(0);

	flip_screen_set(!BIT(flip, 7));
	m_sprgen->set_flip_screen(!BIT(flip, 7));

	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram->buffer(), 0x400);

	m_deco_tilegen[0]->pf_update(m_pf_rowscroll[0], m_pf_rowscroll[1]);
	m_deco_tilegen[1]->pf_update(m_pf_rowscroll[2], m_pf_rowscroll[3]);

	// Draw playfields & sprites
	m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	m_sprgen->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0800, 0x0900, 0x100, 0x0ff);
	m_sprgen->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0900, 0x0900, 0x500, 0x0ff);

	if (m_pri)
	{
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	}
	else
	{
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);
	}

	m_sprgen->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0900, 0x100, 0x0ff);
	m_sprgen->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0100, 0x0900, 0x500, 0x0ff);
	m_deco_tilegen[0]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void cbuster_state::prot_w(offs_t offset, u16 data, u16 mem_mask)
{
	data &= mem_mask;

	/* Protection, maybe this is a PAL on the board?

	80046 is level number
	stop at stage and enter.
	see also 8216..

	    9a 00 = pf4 over pf3 (normal) (level 0)
	    9a f1 =  (level 1 - water), pf3 over ALL sprites + pf4
	    9a 80 = pf3 over pf4 (Level 2 - copter)
	    9a 40 = pf3 over ALL sprites + pf4 (snow) level 3
	    9a c0 = doesn't matter?
	    9a ff = pf 3 over pf4

	I can't find a priority register, I assume it's tied to the
	protection?!

	*/
	if ((data & 0xffff) == 0x9a00) m_prot = 0;
	if ((data & 0xffff) == 0xaa)   m_prot = 0x74;
	if ((data & 0xffff) == 0x0200) m_prot = 0x63 << 8;
	if ((data & 0xffff) == 0x9a)   m_prot = 0xe;
	if ((data & 0xffff) == 0x55)   m_prot = 0x1e;
	if ((data & 0xffff) == 0x0e)  {m_prot = 0x0e; m_pri = 0;} // start
	if ((data & 0xffff) == 0x00)  {m_prot = 0x0e; m_pri = 0;} // level 0
	if ((data & 0xffff) == 0xf1)  {m_prot = 0x36; m_pri = 1;} // level 1
	if ((data & 0xffff) == 0x80)  {m_prot = 0x2e; m_pri = 1;} // level 2
	if ((data & 0xffff) == 0x40)  {m_prot = 0x1e; m_pri = 1;} // level 3
	if ((data & 0xffff) == 0xc0)  {m_prot = 0x3e; m_pri = 0;} // level 4
	if ((data & 0xffff) == 0xff)  {m_prot = 0x76; m_pri = 1;} // level 5
}

u16 cbuster_state::prot_r()
{
	LOGPROT("%04x : protection control read at 0bc004\n", m_maincpu->pc());
	return m_prot;
}

/******************************************************************************/

void cbuster_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x080000, 0x083fff).ram();

	map(0x0a0000, 0x0a1fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x0a2000, 0x0a2fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x0a4000, 0x0a47ff).ram().share(m_pf_rowscroll[0]);
	map(0x0a6000, 0x0a67ff).ram().share(m_pf_rowscroll[1]);

	map(0x0a8000, 0x0a8fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x0aa000, 0x0aafff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x0ac000, 0x0ac7ff).ram().share(m_pf_rowscroll[2]);
	map(0x0ae000, 0x0ae7ff).ram().share(m_pf_rowscroll[3]);

	map(0x0b0000, 0x0b07ff).ram().share("spriteram");
	map(0x0b4000, 0x0b4001).nopw();
	map(0x0b5000, 0x0b500f).w(m_deco_tilegen[0], FUNC(deco16ic_device::pf_control_w));
	map(0x0b6000, 0x0b600f).w(m_deco_tilegen[1], FUNC(deco16ic_device::pf_control_w));
	map(0x0b8000, 0x0b8fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x0b9000, 0x0b9fff).ram().w(m_palette, FUNC(palette_device::write16_ext)).share("palette_ext");
	map(0x0bc000, 0x0bc001).portr("P1_P2").w(m_spriteram, FUNC(buffered_spriteram16_device::write));
	map(0x0bc002, 0x0bc003).portr("DSW");
	map(0x0bc002, 0x0bc003).w(m_soundlatch, FUNC(generic_latch_8_device::write)).umask16(0x00ff).cswidth(16);
	map(0x0bc004, 0x0bc005).rw(FUNC(cbuster_state::prot_r), FUNC(cbuster_state::prot_w));
	map(0x0bc006, 0x0bc007).portr("COINS").lw16(NAME([this] (u16 data) { m_maincpu->set_input_line(4, CLEAR_LINE); }));
}


/******************************************************************************/

void cbuster_state::sound_map(address_map &map)
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

static INPUT_PORTS_START( twocrude )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("COINS")
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
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW1:8" )    // Manual says OFF "Don't Change"

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "4" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x1000, 0x1000, "SW2:5" )    // Manual says OFF "Don't Change"
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "SW2:6" )    // Manual says OFF "Don't Change"
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
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(8*3,-8) },
	{ STEP8(0,1) },
	{ STEP8(0,8*4) },
	8*32
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(8*3,-8) },
	{ STEP8(16*8*4,1), STEP8(0,1) },
	{ STEP16(0,8*4) },
	128*8
};

static GFXDECODE_START( gfx_cbuster )
	GFXDECODE_ENTRY( "tiles1",  0, charlayout,     0, 128 )  // Characters 8x8
	GFXDECODE_ENTRY( "tiles1",  0, tilelayout,     0, 128 )  // Tiles 16x16
	GFXDECODE_ENTRY( "tiles2",  0, tilelayout,     0, 128 )  // Tiles 16x16
GFXDECODE_END

static GFXDECODE_START( gfx_cbuster_spr )
	GFXDECODE_ENTRY( "sprites", 0, tilelayout, 0x100,  80 )  // Sprites 16x16
GFXDECODE_END

/******************************************************************************/

DECO16IC_BANK_CB_MEMBER(cbuster_state::bank_callback)
{
	return (bank & 0x70) << 8;
}

void cbuster_state::machine_start()
{
	save_item(NAME(m_prot));
	save_item(NAME(m_pri));
}

void cbuster_state::machine_reset()
{
	m_prot = 0;
	m_pri = 0;
}

void cbuster_state::twocrude(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(24'000'000) / 2); // Custom chip 59 @ 12MHz Verified
	m_maincpu->set_addrmap(AS_PROGRAM, &cbuster_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(cbuster_state::irq4_line_assert)); // VBL

	H6280(config, m_audiocpu, XTAL(24'000'000) / 4); // Custom chip 45, 6MHz Verified
	m_audiocpu->set_addrmap(AS_PROGRAM, &cbuster_state::sound_map);
	m_audiocpu->add_route(ALL_OUTPUTS, "mono", 0); // internal sound unused
	m_audiocpu->set_timer_scale(2);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(58);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(529));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(cbuster_state::screen_update));

	GFXDECODE(config, "gfxdecode", m_palette, gfx_cbuster);
	PALETTE(config, m_palette).set_format(4, &cbuster_state::xbgr_888, 2048);

	BUFFERED_SPRITERAM16(config, m_spriteram);

	DECO16IC(config, m_deco_tilegen[0], 0);
	m_deco_tilegen[0]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf1_col_bank(0x00);
	m_deco_tilegen[0]->set_pf2_col_bank(0x20);
	m_deco_tilegen[0]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[0]->set_bank1_callback(FUNC(cbuster_state::bank_callback));
	m_deco_tilegen[0]->set_bank2_callback(FUNC(cbuster_state::bank_callback));
	m_deco_tilegen[0]->set_pf12_8x8_bank(0);
	m_deco_tilegen[0]->set_pf12_16x16_bank(1);
	m_deco_tilegen[0]->set_gfxdecode_tag("gfxdecode");

	DECO16IC(config, m_deco_tilegen[1], 0);
	m_deco_tilegen[1]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf1_col_bank(0x30);
	m_deco_tilegen[1]->set_pf2_col_bank(0x40);
	m_deco_tilegen[1]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[1]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[1]->set_bank1_callback(FUNC(cbuster_state::bank_callback));
	m_deco_tilegen[1]->set_bank2_callback(FUNC(cbuster_state::bank_callback));
	m_deco_tilegen[1]->set_pf12_8x8_bank(0);
	m_deco_tilegen[1]->set_pf12_16x16_bank(2);
	m_deco_tilegen[1]->set_gfxdecode_tag("gfxdecode");

	DECO_SPRITE(config, m_sprgen, 0, m_palette, gfx_cbuster_spr);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, 0);

	// YM2203_PITCH_HACK - Pitch is too low at 1.3425MHz (see also stfight.cpp)
	YM2203(config, "ym1", XTAL(32'220'000) / 24 * 3).add_route(ALL_OUTPUTS, "mono", 0.60); // 1.3425MHz Verified

	ym2151_device &ym2(YM2151(config, "ym2", XTAL(32'220'000) / 9)); // 3.58MHz Verified
	ym2.irq_handler().set_inputline(m_audiocpu, 1); // IRQ2
	ym2.add_route(ALL_OUTPUTS, "mono", 0.45);

	OKIM6295(config, "oki1", XTAL(32'220'000) / 32, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.75); // 1.0068MHz Verified

	OKIM6295(config, "oki2", XTAL(32'220'000) / 16, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.60); // 2.01375MHz Verified
}

/******************************************************************************/

ROM_START( cbuster )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "fx01.7l", 0x00000, 0x20000, CRC(ddae6d83) SHA1(ce3fed1393b71821730fb8d87869a89c8e07c456) ) // Same data exists with label fu01-1
	ROM_LOAD16_BYTE( "fx00.4l", 0x00001, 0x20000, CRC(5bc2c0de) SHA1(fa9c357ae4a5c814b7113df3b2f12982077f3e6b) ) // Same data exists with label fu00-1
	ROM_LOAD16_BYTE( "fx03.9l", 0x40000, 0x20000, CRC(c3d65bf9) SHA1(99dd650fd4b427bca25a0776fbd6221f93504106) )
	ROM_LOAD16_BYTE( "fx02.6l", 0x40001, 0x20000, CRC(b875266b) SHA1(a76f8e061392e17394a3f975584823ad39e0097e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "fu11-.19h", 0x00000, 0x10000, CRC(65f20f10) SHA1(cf914893edd98a0f39bbf7068a469ed7d34bd90e) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mab-00.4c",       0x00000, 0x80000, CRC(660eaabd) SHA1(e3d614e13fdb9af159d9758a869d9dae3dbe14e0) ) // Tiles
	ROM_LOAD16_BYTE( "fu05-.6c", 0x80000, 0x10000, CRC(8134d412) SHA1(9c70ff6f9f24ec89c0bb4645afdf2a5ca27e9a0c) ) // Chars
	ROM_LOAD16_BYTE( "fu06-.7c", 0x80001, 0x10000, CRC(2f914a45) SHA1(bb44ba4779e45ee77ef0006363df91aac1f4559a) )

	ROM_REGION( 0x080000, "tiles2", 0 )
	ROM_LOAD( "mab-01.19a", 0x00000, 0x80000, CRC(1080d619) SHA1(68f33a1580d33e4dd0858248c12a0a10ac117249) )

	ROM_REGION( 0x140000, "sprites", 0 )
	ROM_LOAD32_WORD( "mab-02.10a", 0x000000, 0x80000, CRC(58b7231d) SHA1(5b51a2fa42c67f23648be205295184a1fddc00f5) )
	ROM_LOAD32_WORD( "mab-03.11a", 0x000002, 0x80000, CRC(76053b9d) SHA1(093cd01a13509701ec9dd1a806132600a5bd1915) )

	ROM_LOAD32_BYTE( "fu07-.4a", 0x100000, 0x10000, CRC(ca8d0bb3) SHA1(9262d6003cf0cb8c33d0f6c1d0ef35490b29f9b4) )
	ROM_LOAD32_BYTE( "fu08-.5a", 0x100001, 0x10000, CRC(c6afc5c8) SHA1(feddd546f09884c51e4d1802477de4e152a51082) )
	ROM_LOAD32_BYTE( "fu09-.7a", 0x100002, 0x10000, CRC(526809ca) SHA1(2cb9e7417211c1eb23d32e3fee71c5254d34a3ff) )
	ROM_LOAD32_BYTE( "fu10-.8a", 0x100003, 0x10000, CRC(6be6d50e) SHA1(b944db4b3a7c76190f6b40f71f033e16e7964f6a) )

	ROM_REGION( 0x40000, "oki1", 0 ) // ADPCM samples
	ROM_LOAD( "fu12-.16k", 0x00000, 0x20000, CRC(2d1d65f2) SHA1(be3d57b9976ddf7ee6d20ee9e78fe826ee411d79) )

	ROM_REGION( 0x40000, "oki2", 0 ) // ADPCM samples
	ROM_LOAD( "fu13-.21e", 0x00000, 0x20000, CRC(b8525622) SHA1(4a6ec5e3f64256b1383bfbab4167cbd2ec11b5c5) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7114h.18e", 0x0000, 0x0100, CRC(3645b70f) SHA1(7d3831867362037892b43efb007e27d3bd5f6488) ) // Priority (not used)
ROM_END

ROM_START( cbusterw )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "fu01-.7l", 0x00000, 0x20000, CRC(0203e0f8) SHA1(7709636429f2cab43caba3422122dba970dfb50b) )
	ROM_LOAD16_BYTE( "fu00-.4l", 0x00001, 0x20000, CRC(9c58626d) SHA1(6bc950929391221755972658258937a1ef96c244) )
	ROM_LOAD16_BYTE( "fu03-.9l", 0x40000, 0x20000, CRC(def46956) SHA1(e1f71a440430f8f9351ee9e1826ca2d0d5a372f8) )
	ROM_LOAD16_BYTE( "fu02-.6l", 0x40001, 0x20000, CRC(649c3338) SHA1(06373b364283706f0b00ab6d014c674e4b9818fa) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "fu11-.19h", 0x00000, 0x10000, CRC(65f20f10) SHA1(cf914893edd98a0f39bbf7068a469ed7d34bd90e) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mab-00.4c",       0x00000, 0x80000, CRC(660eaabd) SHA1(e3d614e13fdb9af159d9758a869d9dae3dbe14e0) ) // Tiles
	ROM_LOAD16_BYTE( "fu05-.6c", 0x80000, 0x10000, CRC(8134d412) SHA1(9c70ff6f9f24ec89c0bb4645afdf2a5ca27e9a0c) ) // Chars
	ROM_LOAD16_BYTE( "fu06-.7c", 0x80001, 0x10000, CRC(2f914a45) SHA1(bb44ba4779e45ee77ef0006363df91aac1f4559a) )

	ROM_REGION( 0x080000, "tiles2", 0 )
	ROM_LOAD( "mab-01.19a", 0x00000, 0x80000, CRC(1080d619) SHA1(68f33a1580d33e4dd0858248c12a0a10ac117249) )

	ROM_REGION( 0x140000, "sprites", 0 )
	ROM_LOAD32_WORD( "mab-02.10a", 0x000000, 0x80000, CRC(58b7231d) SHA1(5b51a2fa42c67f23648be205295184a1fddc00f5) )
	ROM_LOAD32_WORD( "mab-03.11a", 0x000002, 0x80000, CRC(76053b9d) SHA1(093cd01a13509701ec9dd1a806132600a5bd1915) )

	ROM_LOAD32_BYTE( "fu07-.4a", 0x100000, 0x10000, CRC(ca8d0bb3) SHA1(9262d6003cf0cb8c33d0f6c1d0ef35490b29f9b4) )
	ROM_LOAD32_BYTE( "fu08-.5a", 0x100001, 0x10000, CRC(c6afc5c8) SHA1(feddd546f09884c51e4d1802477de4e152a51082) )
	ROM_LOAD32_BYTE( "fu09-.7a", 0x100002, 0x10000, CRC(526809ca) SHA1(2cb9e7417211c1eb23d32e3fee71c5254d34a3ff) )
	ROM_LOAD32_BYTE( "fu10-.8a", 0x100003, 0x10000, CRC(6be6d50e) SHA1(b944db4b3a7c76190f6b40f71f033e16e7964f6a) )

	ROM_REGION( 0x40000, "oki1", 0 ) // ADPCM samples
	ROM_LOAD( "fu12-.16k", 0x00000, 0x20000, CRC(2d1d65f2) SHA1(be3d57b9976ddf7ee6d20ee9e78fe826ee411d79) )

	ROM_REGION( 0x40000, "oki2", 0 ) // ADPCM samples
	ROM_LOAD( "fu13-.21e", 0x00000, 0x20000, CRC(b8525622) SHA1(4a6ec5e3f64256b1383bfbab4167cbd2ec11b5c5) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7114h.18e",   0x0000, 0x0100, CRC(3645b70f) SHA1(7d3831867362037892b43efb007e27d3bd5f6488) ) // Priority (not used)
ROM_END

ROM_START( cbusterj )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "fr01-1.7l", 0x00000, 0x20000, CRC(af3c014f) SHA1(a7724c48f73e52b19f3688a413e2ed013e226c6b) )
	ROM_LOAD16_BYTE( "fr00-1.4l", 0x00001, 0x20000, CRC(f666ad52) SHA1(6f7325bc3bb79fd8112df677250c4bae572dfa43) )
	ROM_LOAD16_BYTE( "fr03.9l",   0x40000, 0x20000, CRC(02c06118) SHA1(a251f936f80d8a9af033fe6d0d42e1e17ebbbf98) )
	ROM_LOAD16_BYTE( "fr02.6l",   0x40001, 0x20000, CRC(b6c34332) SHA1(c1215c72a03b368655e20f4557475a2fc4c46c9e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "fu11-.19h", 0x00000, 0x10000, CRC(65f20f10) SHA1(cf914893edd98a0f39bbf7068a469ed7d34bd90e) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mab-00.4c",        0x00000, 0x80000, CRC(660eaabd) SHA1(e3d614e13fdb9af159d9758a869d9dae3dbe14e0) ) // Tiles
	// Note: Revision 01 fixes some Japanese fonts: see characters 0x118a / 0x118b / 0x11f9 (16x16)
	ROM_LOAD16_BYTE( "fr05-1.6c", 0x80000, 0x10000, CRC(b1f0d910) SHA1(a2a2ee3a99db52e77e9c108dacffb0387da131a9) ) // Chars
	ROM_LOAD16_BYTE( "fr06-1.7c", 0x80001, 0x10000, CRC(2f914a45) SHA1(bb44ba4779e45ee77ef0006363df91aac1f4559a) )

	ROM_REGION( 0x080000, "tiles2", 0 )
	ROM_LOAD( "mab-01.19a", 0x00000, 0x80000, CRC(1080d619) SHA1(68f33a1580d33e4dd0858248c12a0a10ac117249) )

	ROM_REGION( 0x140000, "sprites", 0 )
	ROM_LOAD32_WORD( "mab-02.10a", 0x000000, 0x80000, CRC(58b7231d) SHA1(5b51a2fa42c67f23648be205295184a1fddc00f5) )
	ROM_LOAD32_WORD( "mab-03.11a", 0x000002, 0x80000, CRC(76053b9d) SHA1(093cd01a13509701ec9dd1a806132600a5bd1915) )

	ROM_LOAD32_BYTE( "fr07.4a", 0x100000, 0x10000, CRC(52c85318) SHA1(74032dac7cb7e7d3028aab4c5f5b0a4e2a7caa03) )
	ROM_LOAD32_BYTE( "fr08.5a", 0x100001, 0x10000, CRC(ea25fbac) SHA1(d00dce24e94ffc212ab3880c00fcadb7b2116f01) )
	ROM_LOAD32_BYTE( "fr09.7a", 0x100002, 0x10000, CRC(f8363424) SHA1(6a6b143a3474965ef89f75e9d7b15946ae26d0d4) )
	ROM_LOAD32_BYTE( "fr10.8a", 0x100003, 0x10000, CRC(241d5760) SHA1(cd216ecf7e88939b91a6e0f02a23c8b875ac24dc) )

	ROM_REGION( 0x40000, "oki1", 0 ) // ADPCM samples
	ROM_LOAD( "fu12-.16k", 0x00000, 0x20000, CRC(2d1d65f2) SHA1(be3d57b9976ddf7ee6d20ee9e78fe826ee411d79) )

	ROM_REGION( 0x40000, "oki2", 0 ) // ADPCM samples
	ROM_LOAD( "fu13-.21e", 0x00000, 0x20000, CRC(b8525622) SHA1(4a6ec5e3f64256b1383bfbab4167cbd2ec11b5c5) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7114h.18e",   0x0000, 0x0100, CRC(3645b70f) SHA1(7d3831867362037892b43efb007e27d3bd5f6488) ) // Priority (not used)
ROM_END

ROM_START( twocrude )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "ft01-1.7l", 0x00000, 0x20000, CRC(7342ffc4) SHA1(dcf611552f0d085f0b552985970f66d1bd6e51e9) )
	ROM_LOAD16_BYTE( "ft00-1.4l", 0x00001, 0x20000, CRC(3f5f535f) SHA1(f5d2b12f98bfcc3426dd2596baaaeca775835e6b) )
	ROM_LOAD16_BYTE( "ft03.9l",   0x40000, 0x20000, CRC(28002c99) SHA1(6397b05a1a237bb17657bee6c8185f61c60c6a2c) )
	ROM_LOAD16_BYTE( "ft02.6l",   0x40001, 0x20000, CRC(37ea0626) SHA1(ec1822eda83829c599cad217b6d5dd34fb970101) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ft11-.19h", 0x00000, 0x10000, CRC(65f20f10) SHA1(cf914893edd98a0f39bbf7068a469ed7d34bd90e) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mab-00.4c",        0x00000, 0x80000, CRC(660eaabd) SHA1(e3d614e13fdb9af159d9758a869d9dae3dbe14e0) ) // Tiles
	// Note: Revision 01 fixes some Japanese fonts: see characters 0x118a / 0x118b / 0x11f9 (16x16)
	ROM_LOAD16_BYTE( "ft05-1.6c", 0x80000, 0x10000, CRC(b1f0d910) SHA1(a2a2ee3a99db52e77e9c108dacffb0387da131a9) ) // Chars
	ROM_LOAD16_BYTE( "ft06-1.7c", 0x80001, 0x10000, CRC(2f914a45) SHA1(bb44ba4779e45ee77ef0006363df91aac1f4559a) )

	ROM_REGION( 0x080000, "tiles2", 0 )
	ROM_LOAD( "mab-01.19a", 0x00000, 0x80000, CRC(1080d619) SHA1(68f33a1580d33e4dd0858248c12a0a10ac117249) ) // Tiles

	ROM_REGION( 0x140000, "sprites", 0 )
	ROM_LOAD32_WORD( "mab-02.10a", 0x000000, 0x80000, CRC(58b7231d) SHA1(5b51a2fa42c67f23648be205295184a1fddc00f5) )
	ROM_LOAD32_WORD( "mab-03.11a", 0x000002, 0x80000, CRC(76053b9d) SHA1(093cd01a13509701ec9dd1a806132600a5bd1915) )

	ROM_LOAD32_BYTE( "ft07-.4a", 0x100000, 0x10000, CRC(e3465c25) SHA1(5369a87847e6f881efc8460e6e8efcf8ff46e87f) )
	ROM_LOAD32_BYTE( "ft08-.5a", 0x100001, 0x10000, CRC(c7f1d565) SHA1(d5dc55cf879f7feaff166a6708d60ef0bf31ddf5) )
	ROM_LOAD32_BYTE( "ft09-.7a", 0x100002, 0x10000, CRC(6e3657b9) SHA1(7e6a140e33f9bc18e35c255680eebe152a5d8858) )
	ROM_LOAD32_BYTE( "ft10-.8a", 0x100003, 0x10000, CRC(cdb83560) SHA1(8b258c4436ccea5a74edff1b6219ab7a5eac0328) )

	ROM_REGION( 0x40000, "oki1", 0 ) // ADPCM samples
	ROM_LOAD( "ft12-.16k", 0x00000, 0x20000, CRC(2d1d65f2) SHA1(be3d57b9976ddf7ee6d20ee9e78fe826ee411d79) )

	ROM_REGION( 0x40000, "oki2", 0 ) // ADPCM samples
	ROM_LOAD( "ft13-.21e", 0x00000, 0x20000, CRC(b8525622) SHA1(4a6ec5e3f64256b1383bfbab4167cbd2ec11b5c5) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7114h.18e", 0x0000, 0x0100, CRC(3645b70f) SHA1(7d3831867362037892b43efb007e27d3bd5f6488) ) // Priority (not used)
ROM_END

ROM_START( twocrudea )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "ft01.7l", 0x00000, 0x20000, CRC(08e96489) SHA1(1e75893cc086d6d6b428ca055851b51d0bc367aa) )
	ROM_LOAD16_BYTE( "ft00.4l", 0x00001, 0x20000, CRC(6765c445) SHA1(b2bbb86414eafe32ed66f3f8ab095a2bce3a1a4b) )
	ROM_LOAD16_BYTE( "ft03.9l", 0x40000, 0x20000, CRC(28002c99) SHA1(6397b05a1a237bb17657bee6c8185f61c60c6a2c) )
	ROM_LOAD16_BYTE( "ft02.6l", 0x40001, 0x20000, CRC(37ea0626) SHA1(ec1822eda83829c599cad217b6d5dd34fb970101) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ft11-.19h", 0x00000, 0x10000, CRC(65f20f10) SHA1(cf914893edd98a0f39bbf7068a469ed7d34bd90e) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mab-00.4c",       0x00000, 0x80000, CRC(660eaabd) SHA1(e3d614e13fdb9af159d9758a869d9dae3dbe14e0) ) // Tiles
	ROM_LOAD16_BYTE( "ft05-.6c", 0x80000, 0x10000, CRC(8134d412) SHA1(9c70ff6f9f24ec89c0bb4645afdf2a5ca27e9a0c) ) // Chars
	ROM_LOAD16_BYTE( "ft06-.7c", 0x80001, 0x10000, CRC(2f914a45) SHA1(bb44ba4779e45ee77ef0006363df91aac1f4559a) )

	ROM_REGION( 0x080000, "tiles2", 0 )
	ROM_LOAD( "mab-01.19a", 0x00000, 0x80000, CRC(1080d619) SHA1(68f33a1580d33e4dd0858248c12a0a10ac117249) ) // Tiles

	ROM_REGION( 0x140000, "sprites", 0 )
	ROM_LOAD32_WORD( "mab-02.10a", 0x000000, 0x80000, CRC(58b7231d) SHA1(5b51a2fa42c67f23648be205295184a1fddc00f5) )
	ROM_LOAD32_WORD( "mab-03.11a", 0x000002, 0x80000, CRC(76053b9d) SHA1(093cd01a13509701ec9dd1a806132600a5bd1915) )

	ROM_LOAD32_BYTE( "ft07-.4a", 0x100000, 0x10000, CRC(e3465c25) SHA1(5369a87847e6f881efc8460e6e8efcf8ff46e87f) )
	ROM_LOAD32_BYTE( "ft08-.5a", 0x100001, 0x10000, CRC(c7f1d565) SHA1(d5dc55cf879f7feaff166a6708d60ef0bf31ddf5) )
	ROM_LOAD32_BYTE( "ft09-.7a", 0x100002, 0x10000, CRC(6e3657b9) SHA1(7e6a140e33f9bc18e35c255680eebe152a5d8858) )
	ROM_LOAD32_BYTE( "ft10-.8a", 0x100003, 0x10000, CRC(cdb83560) SHA1(8b258c4436ccea5a74edff1b6219ab7a5eac0328) )

	ROM_REGION( 0x40000, "oki1", 0 ) // ADPCM samples
	ROM_LOAD( "ft12-.16k", 0x00000, 0x20000, CRC(2d1d65f2) SHA1(be3d57b9976ddf7ee6d20ee9e78fe826ee411d79) )

	ROM_REGION( 0x40000, "oki2", 0 ) // ADPCM samples
	ROM_LOAD( "ft13-.21e", 0x00000, 0x20000, CRC(b8525622) SHA1(4a6ec5e3f64256b1383bfbab4167cbd2ec11b5c5) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7114h.18e", 0x0000, 0x0100, CRC(3645b70f) SHA1(7d3831867362037892b43efb007e27d3bd5f6488) ) // Priority (not used)
ROM_END

/******************************************************************************/

void cbuster_state::init_twocrude()
{
	u8 *ram = memregion("maincpu")->base();

	// Main CPU decrypt
	for (int i = 0x00000; i < 0x80000; i += 2)
	{
		int h = i + NATIVE_ENDIAN_VALUE_LE_BE(1,0), l = i + NATIVE_ENDIAN_VALUE_LE_BE(0,1);

		ram[h] = (ram[h] & 0xcf) | ((ram[h] & 0x10) << 1) | ((ram[h] & 0x20) >> 1);
		ram[h] = (ram[h] & 0x5f) | ((ram[h] & 0x20) << 2) | ((ram[h] & 0x80) >> 2);

		ram[l] = (ram[l] & 0xbd) | ((ram[l] & 0x2) << 5) | ((ram[l] & 0x40) >> 5);
		ram[l] = (ram[l] & 0xf5) | ((ram[l] & 0x2) << 2) | ((ram[l] & 0x8) >> 2);
	}
}

} // anonymous namespace


/******************************************************************************/

GAME( 1990, cbuster,   0,       twocrude, twocrude, cbuster_state, init_twocrude, ROT0, "Data East Corporation", "Crude Buster (World FX version)",    MACHINE_SUPPORTS_SAVE )
GAME( 1990, cbusterw,  cbuster, twocrude, twocrude, cbuster_state, init_twocrude, ROT0, "Data East Corporation", "Crude Buster (World FU version)",    MACHINE_SUPPORTS_SAVE )
GAME( 1990, cbusterj,  cbuster, twocrude, twocrude, cbuster_state, init_twocrude, ROT0, "Data East Corporation", "Crude Buster (Japan FR revision 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, twocrude,  cbuster, twocrude, twocrude, cbuster_state, init_twocrude, ROT0, "Data East USA",         "Two Crude (US FT revision 1)",       MACHINE_SUPPORTS_SAVE )
GAME( 1990, twocrudea, cbuster, twocrude, twocrude, cbuster_state, init_twocrude, ROT0, "Data East USA",         "Two Crude (US FT version)",          MACHINE_SUPPORTS_SAVE )
