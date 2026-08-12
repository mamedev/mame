// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Stephane Humbert
/***************************************************************************

Cobra Command (World)       (c) 1988 Data East Corporation
Cobra Command (Japan)       (c) 1988 Data East Corporation

Emulation by Bryan McPhail, mish@tendril.co.uk

***************************************************************************/

#include "emu.h"

#include "decbac06.h"
#include "decmxc06.h"
#include "decrmc3.h"

#include "cpu/m6502/r65c02.h"
#include "cpu/m6809/m6809.h"
#include "machine/gen_latch.h"
#include "machine/input_merger.h"
#include "sound/ymopn.h"
#include "sound/ymopl.h"

#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "multibyte.h"


namespace {

class cobracom_state : public driver_device
{
public:
	cobracom_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_spritegen(*this, "spritegen"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundirq(*this, "soundirq"),
		m_soundlatch(*this, "soundlatch"),
		m_tilegen(*this, "tilegen%u", 1),
		m_mainbank(*this, "mainbank"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_spriteram16(*this, "spriteram16", 0x800, ENDIANNESS_BIG)
	{ }

	void cobracom(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void videoram_w(offs_t offset, u8 data);
	void buffer_spriteram16_w(u8 data);
	void bank_w(u8 data);

	TILE_GET_INFO_MEMBER(get_fix_tile_info);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void colpri_cb(u32 &colour, u32 &pri_mask);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<deco_mxc06_device> m_spritegen;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<deco_rmc3_device> m_palette;
	required_device<input_merger_device> m_soundirq;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device_array<deco_bac06_device, 2> m_tilegen;

	// memory regions
	required_memory_bank m_mainbank;

	// memory pointers
	required_shared_ptr<u8> m_videoram;

	required_shared_ptr<u8> m_spriteram;
	memory_share_creator<u16> m_spriteram16;

	// video-related
	tilemap_t *m_fix_tilemap = nullptr;
};

/******************************************************************************/

void cobracom_state::buffer_spriteram16_w(u8 data)
{
	// copy to a 16-bit region for the sprite chip
	for (int i = 0; i < 0x800/2 ; i++)
		m_spriteram16[i] = get_u16be(&m_spriteram[i * 2]);
}

void cobracom_state::videoram_w(offs_t offset, u8 data)
{
	m_videoram[offset] = data;
	m_fix_tilemap->mark_tile_dirty(offset / 2);
}

/******************************************************************************/

void cobracom_state::colpri_cb(u32 &colour, u32 &pri_mask)
{
	pri_mask = 0; // above foreground, background
	if ((colour & 4) == 0)
		pri_mask |= GFX_PMASK_2; // behind foreground, above background

	colour &= 3;
}

u32 cobracom_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0,cliprect);
	const bool flip = m_tilegen[0]->get_flip_state();
	m_tilegen[0]->set_flip_screen(flip);
	m_tilegen[1]->set_flip_screen(flip);
	m_spritegen->set_flip_screen(flip);
	m_fix_tilemap->set_flip(flip ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	m_tilegen[0]->draw(screen,bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 1);
	m_tilegen[1]->draw(screen,bitmap,cliprect,0, 2);
	m_spritegen->draw_sprites(screen, bitmap, cliprect, m_spriteram16.target(), 0x800/2);
	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


/******************************************************************************/

TILE_GET_INFO_MEMBER(cobracom_state::get_fix_tile_info)
{
	const int offs = tile_index << 1;
	const int tile = get_u16be(&m_videoram[offs]);
	const int color = (tile & 0xe000) >> 13;

	tileinfo.set(0, tile & 0xfff, color, 0);
}

void cobracom_state::video_start()
{
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cobracom_state::get_fix_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fix_tilemap->set_transparent_pen(0);
}


/******************************************************************************/

void cobracom_state::bank_w(u8 data)
{
	m_mainbank->set_entry(data & 7);
}


/******************************************************************************/

void cobracom_state::main_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x0800, 0x0fff).rw(m_tilegen[0], FUNC(deco_bac06_device::vram8_r<false>), FUNC(deco_bac06_device::vram8_w<false>));
	map(0x1000, 0x17ff).rw(m_tilegen[1], FUNC(deco_bac06_device::vram8_r<false>), FUNC(deco_bac06_device::vram8_w<false>));
	map(0x1800, 0x1fff).ram();
	map(0x2000, 0x27ff).ram().w(FUNC(cobracom_state::videoram_w)).share(m_videoram);
	map(0x2800, 0x2fff).ram().share(m_spriteram);
	map(0x3000, 0x31ff).ram().w(m_palette, FUNC(deco_rmc3_device::write8)).share("palette");
	map(0x3200, 0x37ff).nopw(); // Unused
	map(0x3800, 0x3800).portr("IN0");
	map(0x3801, 0x3801).portr("IN1");
	map(0x3802, 0x3802).portr("DSW0");
	map(0x3803, 0x3803).portr("DSW1");
	map(0x3800, 0x3807).w(m_tilegen[0], FUNC(deco_bac06_device::ctrlreg8_w));
	map(0x3810, 0x381f).w(m_tilegen[0], FUNC(deco_bac06_device::scrollreg8_w<false>));
	map(0x3a00, 0x3a00).portr("IN2");
	map(0x3a00, 0x3a07).w(m_tilegen[1], FUNC(deco_bac06_device::ctrlreg8_w));
	map(0x3a10, 0x3a1f).w(m_tilegen[1], FUNC(deco_bac06_device::scrollreg8_w<false>));
	map(0x3c00, 0x3c00).w(FUNC(cobracom_state::bank_w));
	map(0x3c02, 0x3c02).w(FUNC(cobracom_state::buffer_spriteram16_w));
	map(0x3e00, 0x3e00).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x4000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0xffff).rom().region("maincpu", 0);
}


/******************************************************************************/

void cobracom_state::sound_map(address_map &map)
{
	map(0x0000, 0x05ff).ram();
	map(0x2000, 0x2001).w("ym1", FUNC(ym2203_device::write));
	map(0x4000, 0x4001).w("ym2", FUNC(ym3812_device::write));
	map(0x6000, 0x6000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x8000, 0xffff).rom().region("audiocpu", 0);
}


/******************************************************************************/

// verified from M6809 code
static INPUT_PORTS_START( cobracom )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) // fire
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) // missile
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) // fire
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) // missile
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 ) // always adds 1 credit
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )               PORT_DIPLOCATION("SW1:1,2") // code at 0x88b7 in 'cobracom', 0x890e in 'cobracomj'
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )               PORT_DIPLOCATION("SW1:3,4") // code at 0x889b in 'cobracom', 0x88f2 in 'cobracomj'
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )              PORT_DIPLOCATION("SW1:5") // Manual says 'Off'
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )              PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )                PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "99 (Cheat)")                     // lose a life before getting 2nd bonus life !
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Bonus_Life ) )           PORT_DIPLOCATION("SW2:6") // table at 0xa898 (2* 2 words) in 'cobracomj', 0xa8fe in 'cobracomj'
	PORT_DIPSETTING(    0x20, "50k and 150k" )
	PORT_DIPSETTING(    0x00, "100k and 200k" )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:7")
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:8") // previously "Freeze" : code at 0x8849 in 'cobracomj', 0x88a0 in 'cobracomj'
INPUT_PORTS_END


/******************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(1,2),0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 // every sprite takes 8 consecutive bytes
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4),RGN_FRAC(2,4),RGN_FRAC(1,4),0 },
	{ 16*8, 1+(16*8), 2+(16*8), 3+(16*8), 4+(16*8), 5+(16*8), 6+(16*8), 7+(16*8),
		0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8},
	16*16
};

static GFXDECODE_START( gfx_cobracom )
	GFXDECODE_ENTRY( "char",    0, charlayout,   0, 8 )
	GFXDECODE_ENTRY( "tiles2",  0, tilelayout, 128, 4 )
	GFXDECODE_ENTRY( "tiles1",  0, tilelayout, 192, 4 )
GFXDECODE_END

static GFXDECODE_START( gfx_cobracom_spr )
	GFXDECODE_ENTRY( "sprites", 0, tilelayout, 64, 4 )
GFXDECODE_END


/******************************************************************************/

void cobracom_state::machine_start()
{
	u8 *ROM = memregion("mainbank")->base();
	m_mainbank->configure_entries(0, 8, &ROM[0], 0x4000);
}


void cobracom_state::cobracom(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, 1'500'000); // MC68B09EP
	m_maincpu->set_addrmap(AS_PROGRAM, &cobracom_state::main_map);

	R65C02(config, m_audiocpu, 1'500'000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &cobracom_state::sound_map); // NMIs are caused by the main CPU

	// video hardware
	DECO_BAC06(config, m_tilegen[0]);
	m_tilegen[0]->set_gfx_region_wide(1, 1, 0);
	m_tilegen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO_BAC06(config, m_tilegen[1]);
	m_tilegen[1]->set_gfx_region_wide(2, 2, 0);
	m_tilegen[1]->set_gfxdecode_tag(m_gfxdecode);

	DECO_MXC06(config, m_spritegen, m_palette, gfx_cobracom_spr);
	m_spritegen->set_colpri_callback(FUNC(cobracom_state::colpri_cb));

	SCREEN(config, m_screen);
	// DECO video CRTC, matches PCB measurements
	m_screen->set_raw(12_MHz_XTAL / 2, 384, 0, 256, 272, 8, 248);
	m_screen->set_screen_update(FUNC(cobracom_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cobracom);
	DECO_RMC3(config, m_palette, 0, 256); // xxxxBBBBGGGGRRRR with custom weighting

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	INPUT_MERGER_ANY_HIGH(config, m_soundirq);
	m_soundirq->output_handler().set_inputline(m_audiocpu, m6502_device::IRQ_LINE);

	ym2203_device &ym1(YM2203(config, "ym1", 1'500'000));
	ym1.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<0>));
	ym1.add_route(0, "mono", 0.20);
	ym1.add_route(1, "mono", 0.20);
	ym1.add_route(2, "mono", 0.20);
	ym1.add_route(3, "mono", 0.40);

	ym3812_device &ym2(YM3812(config, "ym2", 3'000'000));
	ym2.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<1>));
	ym2.add_route(ALL_OUTPUTS, "mono", 0.80);
}


/******************************************************************************/

ROM_START( cobracom )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "el11-5.5j", 0x00000, 0x08000, CRC(af0a8b05) SHA1(096e4e7f2785a20bfaec14277413ce4e20e90214) )

	ROM_REGION( 0x20000, "mainbank", 0 )
	ROM_LOAD( "el12-4.7j", 0x00000, 0x10000, CRC(7a44ef38) SHA1(d7dc277dce08f9d073290e100be4a7ca2e2b82cb) )
	ROM_LOAD( "el13.9j",   0x10000, 0x10000, CRC(04505acb) SHA1(2220efb277884588859375dab9960f04f07273a7) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "el10-4.1f", 0x0000, 0x8000, CRC(edfad118) SHA1(10de8805472346fead62460a3fdc09ae26a4e0d5) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "el14.14j", 0x00000, 0x08000, CRC(47246177) SHA1(51b025740dc03b04009ac97d8d110ab521894386) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "el00-4.2a", 0x00000, 0x10000, CRC(122da2a8) SHA1(ce72f16abf7e5449c7d044d4b827e8735c3be0ff) )
	ROM_LOAD( "el01-4.3a", 0x10000, 0x10000, CRC(27bf705b) SHA1(196c35aaf3816d3eef4c2af6d146a90a48365d33) )
	ROM_LOAD( "el02-4.5a", 0x20000, 0x10000, CRC(c86fede6) SHA1(97584fa19591651fcfb39d1b2b6306165e93554c) )
	ROM_LOAD( "el03-4.6a", 0x30000, 0x10000, CRC(1d8a855b) SHA1(429261c200dddc62a330be8aea150b2037133188) )

	ROM_REGION( 0x40000, "tiles1", 0 )
	ROM_LOAD( "el05.15a", 0x00000, 0x10000, CRC(1c4f6033) SHA1(4a7dece911166d1ff5f41df6ec5140596206d8d4) )
	ROM_LOAD( "el06.16a", 0x10000, 0x10000, CRC(d24ba794) SHA1(b34b7bbaab4ebdd81c87d363f087cc92e27e8d1c) )
	ROM_LOAD( "el04.13a", 0x20000, 0x10000, CRC(d80a49ce) SHA1(1a92413b5ab53f80e44a954433e69ec5fe2c0aa6) )
	ROM_LOAD( "el07.18a", 0x30000, 0x10000, CRC(6d771fc3) SHA1(f29979f3aa07bdb544fb0c1d773c5558b4533390) )

	ROM_REGION( 0x20000, "tiles2", 0 )
	ROM_LOAD( "el08.7d", 0x00000, 0x08000, CRC(cb0dcf4c) SHA1(e14853f83ee9ba5cbf2eb1e085fee4e65af3cc25) )
	ROM_CONTINUE(        0x10000, 0x08000 )
	ROM_LOAD( "el09.9d", 0x08000, 0x08000, CRC(1fae5be7) SHA1(be6e090b0b82648b385d9b6d11775f3ff40f0af3) )
	ROM_CONTINUE(        0x18000, 0x08000 )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "eh15.12f", 0x0000, 0x0200, CRC(279e540c) SHA1(9e5e707da9f7c403c63d77fa378f22da9906d4e5) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pt-0.16h", 0x0000, 0x0117, CRC(add5074f) SHA1(352d984ba4d50c437ee9e5d8e8d00b0da6b3b8f3) )
ROM_END

ROM_START( cobracoma )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "el11-4.5j", 0x00000, 0x08000, CRC(6dca6734) SHA1(1d165845680df2f1febd2b7d2f3163d68523496e) )

	ROM_REGION( 0x20000, "mainbank", 0 )
	ROM_LOAD( "el12-4.7j", 0x00000, 0x10000, CRC(7a44ef38) SHA1(d7dc277dce08f9d073290e100be4a7ca2e2b82cb) )
	ROM_LOAD( "el13.9j",   0x10000, 0x10000, CRC(04505acb) SHA1(2220efb277884588859375dab9960f04f07273a7) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "el10-4.1f", 0x0000, 0x8000, CRC(edfad118) SHA1(10de8805472346fead62460a3fdc09ae26a4e0d5) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "el14.14j", 0x00000, 0x08000, CRC(47246177) SHA1(51b025740dc03b04009ac97d8d110ab521894386) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "el00-4.2a", 0x00000, 0x10000, CRC(122da2a8) SHA1(ce72f16abf7e5449c7d044d4b827e8735c3be0ff) )
	ROM_LOAD( "el01-4.3a", 0x10000, 0x10000, CRC(27bf705b) SHA1(196c35aaf3816d3eef4c2af6d146a90a48365d33) )
	ROM_LOAD( "el02-4.5a", 0x20000, 0x10000, CRC(c86fede6) SHA1(97584fa19591651fcfb39d1b2b6306165e93554c) )
	ROM_LOAD( "el03-4.6a", 0x30000, 0x10000, CRC(1d8a855b) SHA1(429261c200dddc62a330be8aea150b2037133188) )

	ROM_REGION( 0x40000, "tiles1", 0 )
	ROM_LOAD( "el05.15a", 0x00000, 0x10000, CRC(1c4f6033) SHA1(4a7dece911166d1ff5f41df6ec5140596206d8d4) )
	ROM_LOAD( "el06.16a", 0x10000, 0x10000, CRC(d24ba794) SHA1(b34b7bbaab4ebdd81c87d363f087cc92e27e8d1c) )
	ROM_LOAD( "el04.13a", 0x20000, 0x10000, CRC(d80a49ce) SHA1(1a92413b5ab53f80e44a954433e69ec5fe2c0aa6) )
	ROM_LOAD( "el07.18a", 0x30000, 0x10000, CRC(6d771fc3) SHA1(f29979f3aa07bdb544fb0c1d773c5558b4533390) )

	ROM_REGION( 0x20000, "tiles2", 0 )
	ROM_LOAD( "el08.7d", 0x00000, 0x08000, CRC(cb0dcf4c) SHA1(e14853f83ee9ba5cbf2eb1e085fee4e65af3cc25) )
	ROM_CONTINUE(        0x10000, 0x08000 )
	ROM_LOAD( "el09.9d", 0x08000, 0x08000, CRC(1fae5be7) SHA1(be6e090b0b82648b385d9b6d11775f3ff40f0af3) )
	ROM_CONTINUE(        0x18000, 0x08000 )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "eh15.12f", 0x0000, 0x0200, CRC(279e540c) SHA1(9e5e707da9f7c403c63d77fa378f22da9906d4e5) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pt-0.16h", 0x0000, 0x0117, CRC(add5074f) SHA1(352d984ba4d50c437ee9e5d8e8d00b0da6b3b8f3) )
ROM_END

ROM_START( cobracomb )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "el11.5j", 0x00000, 0x08000, CRC(c6a102e3) SHA1(51d9781d13a8a98c9bbdf34fa79ebfd8a152567e) ) // "zero" revision - IE: Original version

	ROM_REGION( 0x20000, "mainbank", 0 )
	ROM_LOAD( "el12.7j", 0x00000, 0x10000, CRC(72b2dab4) SHA1(ff84a04cfe920a0d8105fbbd1f44ee86822cf0d3) ) // "zero" revision - IE: Original version
	ROM_LOAD( "el13.9j", 0x10000, 0x10000, CRC(04505acb) SHA1(2220efb277884588859375dab9960f04f07273a7) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "el10.1f", 0x0000, 0x8000, CRC(62ca5e89) SHA1(b04acaccc58846e0d277868a873a440b7f8071b0) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "el14.14j", 0x00000, 0x08000, CRC(47246177) SHA1(51b025740dc03b04009ac97d8d110ab521894386) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "el00.2a", 0x00000, 0x10000, CRC(d96b6797) SHA1(01c4a9f2bebb13cba14636690cd5356db73f045e) )
	ROM_LOAD( "el01.3a", 0x10000, 0x10000, CRC(3fef9c02) SHA1(e4b731faf6a2f4e5fed8ba9bd07e0f203981ffec) )
	ROM_LOAD( "el02.5a", 0x20000, 0x10000, CRC(bfae6c34) SHA1(9503a120e11e9466cd9a2931fd44a631d72ca5f0) )
	ROM_LOAD( "el03.6a", 0x30000, 0x10000, CRC(d56790f8) SHA1(1cc7cb9f7102158de14a737e9317a54f01790ba8) )

	ROM_REGION( 0x40000, "tiles1", 0 )
	ROM_LOAD( "el05.15a", 0x00000, 0x10000, CRC(1c4f6033) SHA1(4a7dece911166d1ff5f41df6ec5140596206d8d4) )
	ROM_LOAD( "el06.16a", 0x10000, 0x10000, CRC(d24ba794) SHA1(b34b7bbaab4ebdd81c87d363f087cc92e27e8d1c) )
	ROM_LOAD( "el04.13a", 0x20000, 0x10000, CRC(d80a49ce) SHA1(1a92413b5ab53f80e44a954433e69ec5fe2c0aa6) )
	ROM_LOAD( "el07.18a", 0x30000, 0x10000, CRC(6d771fc3) SHA1(f29979f3aa07bdb544fb0c1d773c5558b4533390) )

	ROM_REGION( 0x20000, "tiles2", 0 )
	ROM_LOAD( "el08.7d", 0x00000, 0x08000, CRC(cb0dcf4c) SHA1(e14853f83ee9ba5cbf2eb1e085fee4e65af3cc25) )
	ROM_CONTINUE(        0x10000, 0x08000 )
	ROM_LOAD( "el09.9d", 0x08000, 0x08000, CRC(1fae5be7) SHA1(be6e090b0b82648b385d9b6d11775f3ff40f0af3) )
	ROM_CONTINUE(        0x18000, 0x08000 )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "eh15.12f", 0x0000, 0x0200, CRC(279e540c) SHA1(9e5e707da9f7c403c63d77fa378f22da9906d4e5) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pt-0.16h", 0x0000, 0x0117, CRC(add5074f) SHA1(352d984ba4d50c437ee9e5d8e8d00b0da6b3b8f3) )
ROM_END

ROM_START( cobracomj )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "eh11.5j", 0x00000, 0x08000, CRC(868637e1) SHA1(8b1e3e045e341bb94b1f6c7d89198b22e6c19de7) )

	ROM_REGION( 0x20000, "mainbank", 0 )
	ROM_LOAD( "eh12.7j", 0x00000, 0x10000, CRC(7c878a83) SHA1(9b2a3083c6dae69626fdab16d97517d30eaa1859) )
	ROM_LOAD( "el13.9j", 0x10000, 0x10000, CRC(04505acb) SHA1(2220efb277884588859375dab9960f04f07273a7) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "eh10.1f", 0x0000, 0x8000, CRC(62ca5e89) SHA1(b04acaccc58846e0d277868a873a440b7f8071b0) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "eh14.14j", 0x00000, 0x08000, CRC(47246177) SHA1(51b025740dc03b04009ac97d8d110ab521894386) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "eh00.2a", 0x00000, 0x10000, CRC(d96b6797) SHA1(01c4a9f2bebb13cba14636690cd5356db73f045e) )
	ROM_LOAD( "eh01.3a", 0x10000, 0x10000, CRC(3fef9c02) SHA1(e4b731faf6a2f4e5fed8ba9bd07e0f203981ffec) )
	ROM_LOAD( "eh02.5a", 0x20000, 0x10000, CRC(bfae6c34) SHA1(9503a120e11e9466cd9a2931fd44a631d72ca5f0) )
	ROM_LOAD( "eh03.6a", 0x30000, 0x10000, CRC(d56790f8) SHA1(1cc7cb9f7102158de14a737e9317a54f01790ba8) )

	ROM_REGION( 0x40000, "tiles1", 0 )
	ROM_LOAD( "eh05.15a", 0x00000, 0x10000, CRC(1c4f6033) SHA1(4a7dece911166d1ff5f41df6ec5140596206d8d4) )
	ROM_LOAD( "eh06.16a", 0x10000, 0x10000, CRC(d24ba794) SHA1(b34b7bbaab4ebdd81c87d363f087cc92e27e8d1c) )
	ROM_LOAD( "eh04.13a", 0x20000, 0x10000, CRC(d80a49ce) SHA1(1a92413b5ab53f80e44a954433e69ec5fe2c0aa6) )
	ROM_LOAD( "eh07.18a", 0x30000, 0x10000, CRC(6d771fc3) SHA1(f29979f3aa07bdb544fb0c1d773c5558b4533390) )

	ROM_REGION( 0x20000, "tiles2", 0 )
	ROM_LOAD( "eh08.7d", 0x00000, 0x08000, CRC(cb0dcf4c) SHA1(e14853f83ee9ba5cbf2eb1e085fee4e65af3cc25) )
	ROM_CONTINUE(        0x10000, 0x08000 )
	ROM_LOAD( "eh09.9d", 0x08000, 0x08000, CRC(1fae5be7) SHA1(be6e090b0b82648b385d9b6d11775f3ff40f0af3) )
	ROM_CONTINUE(        0x18000, 0x08000 )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "eh15.12f", 0x0000, 0x0200, CRC(279e540c) SHA1(9e5e707da9f7c403c63d77fa378f22da9906d4e5) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pt-0.16h", 0x0000, 0x0117, CRC(add5074f) SHA1(352d984ba4d50c437ee9e5d8e8d00b0da6b3b8f3) )
ROM_END

// bootleg of the Japanese release manufactured in Italy. Contents are the same but for PROMs and PAL
// main PCB is marked: "LC" on component side ("LC" is the Italian for "Lato Componenti" which translates to "Components Side")
// main PCB is marked: "LS" on solder side ("LS" is the Italian for "Lato Saldature" which translates to "Solders Side")
ROM_START( cobracomjbl )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "cobra4.bin", 0x00000, 0x08000, CRC(868637e1) SHA1(8b1e3e045e341bb94b1f6c7d89198b22e6c19de7) )

	ROM_REGION( 0x20000, "mainbank", 0 )
	ROM_LOAD( "cobra3.bin", 0x00000, 0x10000, CRC(7c878a83) SHA1(9b2a3083c6dae69626fdab16d97517d30eaa1859) )
	ROM_LOAD( "cobra2.bin", 0x10000, 0x10000, CRC(04505acb) SHA1(2220efb277884588859375dab9960f04f07273a7) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "cobra5.bin", 0x0000, 0x8000, CRC(62ca5e89) SHA1(b04acaccc58846e0d277868a873a440b7f8071b0) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "cobra1.bin", 0x00000, 0x08000, CRC(47246177) SHA1(51b025740dc03b04009ac97d8d110ab521894386) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "cob17.bin", 0x00000, 0x10000, CRC(d96b6797) SHA1(01c4a9f2bebb13cba14636690cd5356db73f045e) )
	ROM_LOAD( "cob16.bin", 0x10000, 0x10000, CRC(3fef9c02) SHA1(e4b731faf6a2f4e5fed8ba9bd07e0f203981ffec) )
	ROM_LOAD( "cob15.bin", 0x20000, 0x10000, CRC(bfae6c34) SHA1(9503a120e11e9466cd9a2931fd44a631d72ca5f0) )
	ROM_LOAD( "cob14.bin", 0x30000, 0x10000, CRC(d56790f8) SHA1(1cc7cb9f7102158de14a737e9317a54f01790ba8) )

	ROM_REGION( 0x40000, "tiles1", 0 )
	ROM_LOAD( "cob13.bin", 0x00000, 0x10000, CRC(1c4f6033) SHA1(4a7dece911166d1ff5f41df6ec5140596206d8d4) )
	ROM_LOAD( "cob12.bin", 0x10000, 0x10000, CRC(d24ba794) SHA1(b34b7bbaab4ebdd81c87d363f087cc92e27e8d1c) )
	ROM_LOAD( "cob11.bin", 0x20000, 0x10000, CRC(d80a49ce) SHA1(1a92413b5ab53f80e44a954433e69ec5fe2c0aa6) )
	ROM_LOAD( "cob10.bin", 0x30000, 0x10000, CRC(6d771fc3) SHA1(f29979f3aa07bdb544fb0c1d773c5558b4533390) )

	ROM_REGION( 0x20000, "tiles2", 0 )
	ROM_LOAD( "cobra6.bin", 0x00000, 0x08000, CRC(c991298f) SHA1(3f79773a8def6b79b77c2ca6b8e8dbde5bdcd127) )
	ROM_LOAD( "cobra8.bin", 0x08000, 0x08000, CRC(6bcc5982) SHA1(5bafa9a7a115d2b67f4fd582dc79f7e2848e5010) )
	ROM_LOAD( "cobra7.bin", 0x10000, 0x08000, CRC(f5e267e5) SHA1(4abda964b8ebceeaba7717d4df23a85862934490) )
	ROM_LOAD( "cobra9.bin", 0x18000, 0x08000, CRC(c90443b5) SHA1(2ede9a05e54a662c1b08672921a0390d8ce128a7) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "63s481n.3",   0x0000, 0x0200, CRC(279e540c) SHA1(9e5e707da9f7c403c63d77fa378f22da9906d4e5) )
	ROM_LOAD( "am27s21pc.1", 0x0200, 0x0100, CRC(9f6aa3e5) SHA1(518247d4581eee3a078269fcf0c86d182cf622cd) )
	ROM_LOAD( "am27s21pc.2", 0x0300, 0x0100, CRC(af46d1ee) SHA1(281bcc61d9d67b007c1399e228ec6baf6ab5d4ff) )
	ROM_LOAD( "am27s21pc.3", 0x0400, 0x0100, CRC(1e4189e8) SHA1(ce62b19005f565b734d8db3ee49d7c070ed53ad6) )

	ROM_REGION( 0x0104, "plds", 0 )
	ROM_LOAD( "pal16l8a-2cn.bin", 0x0000, 0x0104, CRC(3ef8cf68) SHA1(9410a139fb10628bc612d198f5c9f04b2b34f52f) )
ROM_END


/******************************************************************************/

} // anonymous namespace

// Unlike most Deco games of this period Cobra Command does not seem to have a Data East USA release.  Instead the Data East Corporation release
// was used in the US as evidenced by boards with the EL romset bearing AAMA seal stickers (American Amusement Machine Association)
GAME( 1988, cobracom,    0,        cobracom,  cobracom,  cobracom_state,    empty_init,     ROT0,   "Data East Corporation", "Cobra-Command (World/US revision 5)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, cobracoma,   cobracom, cobracom,  cobracom,  cobracom_state,    empty_init,     ROT0,   "Data East Corporation", "Cobra-Command (World/US revision 4)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, cobracomb,   cobracom, cobracom,  cobracom,  cobracom_state,    empty_init,     ROT0,   "Data East Corporation", "Cobra-Command (World/US)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, cobracomj,   cobracom, cobracom,  cobracom,  cobracom_state,    empty_init,     ROT0,   "Data East Corporation", "Cobra-Command (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, cobracomjbl, cobracom, cobracom,  cobracom,  cobracom_state,    empty_init,     ROT0,   "bootleg",               "Cobra-Command (Japan, bootleg)", MACHINE_SUPPORTS_SAVE )
