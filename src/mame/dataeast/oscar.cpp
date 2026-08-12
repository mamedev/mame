// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Stephane Humbert
/***************************************************************************

Psycho-Nics Oscar (World)   (c) 1987 Data East Corporation
Psycho-Nics Oscar (US)      (c) 1988 Data East USA
Psycho-Nics Oscar (Japan)   (c) 1987 Data East Corporation

Emulation by Bryan McPhail, mish@tendril.co.uk

***************************************************************************/

#include "emu.h"

#include "decbac06.h"
#include "decmxc06.h"
#include "deco222.h"
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

class oscar_state : public driver_device
{
public:
	oscar_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_audiocpu(*this, "audiocpu"),
		m_spritegen(*this, "spritegen"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundirq(*this, "soundirq"),
		m_soundlatch(*this, "soundlatch"),
		m_tilegen(*this, "tilegen"),
		m_mainbank(*this, "mainbank"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_spriteram16(*this, "spriteram16", 0x800, ENDIANNESS_BIG)
	{ }

	void oscar(machine_config &config) ATTR_COLD;
	void oscarbl(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void videoram_w(offs_t offset, u8 data);
	void buffer_spriteram16_w(u8 data);
	void bank_w(u8 data);
	void main_irq_on_w(u8 data);
	void main_irq_off_w(u8 data);
	void sub_irq_on_w(u8 data);
	void sub_irq_off_w(u8 data);
	void coin_irq(int state);
	void coin_clear_w(u8 data);

	TILE_GET_INFO_MEMBER(get_fix_tile_info);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void tile_cb(tile_data &tileinfo, u32 &tile, u32 &colour, u32 &flags);

	void oscarbl_sound_opcodes_map(address_map &map) ATTR_COLD;
	void oscar_base_map(address_map &map) ATTR_COLD;
	void oscar_map(address_map &map) ATTR_COLD;
	void oscar_sound_map(address_map &map) ATTR_COLD;
	void oscar_sub_map(address_map &map) ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device<deco_mxc06_device> m_spritegen;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<deco_rmc3_device> m_palette;
	required_device<input_merger_device> m_soundirq;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<deco_bac06_device> m_tilegen;

	// memory regions
	required_memory_bank m_mainbank;

	// memory pointers
	required_shared_ptr<u8> m_videoram;

	required_shared_ptr<u8> m_spriteram;
	memory_share_creator<u16> m_spriteram16;

	// video-related
	tilemap_t *m_fix_tilemap = nullptr;

	// misc
	bool m_coin_state = false;
};

/******************************************************************************/

void oscar_state::buffer_spriteram16_w(u8 data)
{
	// copy to a 16-bit region for the sprite chip
	for (int i = 0; i < 0x800/2 ; i++)
		m_spriteram16[i] = get_u16be(&m_spriteram[i * 2]);
}

void oscar_state::videoram_w(offs_t offset, u8 data)
{
	m_videoram[offset] = data;
	m_fix_tilemap->mark_tile_dirty(offset / 2);
}


/******************************************************************************/

// we mimic the priority scheme in dec0.cpp, this was originally a bit different, so this could be wrong
void oscar_state::tile_cb(tile_data &tileinfo, u32 &tile, u32 &colour, u32 &flags)
{
	tileinfo.group = BIT(colour, 3);
	colour &= 7;
}

u32 oscar_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const bool flip = m_tilegen->get_flip_state();
	m_tilegen->set_flip_screen(flip);
	m_spritegen->set_flip_screen(flip);
	m_fix_tilemap->set_flip(flip ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	m_tilegen->draw(screen,bitmap,cliprect,TILEMAP_DRAW_LAYER1, 0);
	m_spritegen->draw_sprites(screen, bitmap, cliprect, m_spriteram16.target(), 0x800/2);
	m_tilegen->draw(screen,bitmap,cliprect,TILEMAP_DRAW_LAYER0, 0);
	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

TILE_GET_INFO_MEMBER(oscar_state::get_fix_tile_info)
{
	const u32 offs = tile_index << 1;
	const u16 tile = get_u16be(&m_videoram[offs]);
	const u8 color = (tile & 0xf000) >> 14;

	tileinfo.set(0, tile & 0xfff, color, 0);
}

void oscar_state::video_start()
{
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(oscar_state::get_fix_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fix_tilemap->set_transparent_pen(0);
	m_tilegen->set_transmask(0, 0xffff, 0x0000);
	m_tilegen->set_transmask(1, 0x00ff, 0xff00);
}


/******************************************************************************/

void oscar_state::bank_w(u8 data)
{
	m_mainbank->set_entry(data & 3);
}


/******************************************************************************/

void oscar_state::main_irq_on_w(u8 data)
{
	m_maincpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
}

void oscar_state::main_irq_off_w(u8 data)
{
	m_maincpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
}

void oscar_state::sub_irq_on_w(u8 data)
{
	m_subcpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
}

void oscar_state::sub_irq_off_w(u8 data)
{
	m_subcpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
}


/******************************************************************************/

void oscar_state::oscar_base_map(address_map &map)
{
	map(0x0000, 0x0eff).ram().share("share1");
	map(0x0f00, 0x0fff).ram();
	map(0x1000, 0x1fff).ram().share("share2");
	map(0x3e80, 0x3e80).w(FUNC(oscar_state::sub_irq_on_w)); // IRQ 2
	map(0x3e81, 0x3e81).w(FUNC(oscar_state::main_irq_off_w)); // IRC 1
	map(0x3e82, 0x3e82).w(FUNC(oscar_state::main_irq_on_w)); // IRQ 1
	map(0x3e83, 0x3e83).w(FUNC(oscar_state::sub_irq_off_w)); // IRC 2
}

void oscar_state::oscar_map(address_map &map)
{
	oscar_base_map(map);
	map(0x2000, 0x27ff).ram().w(FUNC(oscar_state::videoram_w)).share(m_videoram);
	map(0x2800, 0x2fff).rw(m_tilegen, FUNC(deco_bac06_device::vram8_r<false>), FUNC(deco_bac06_device::vram8_w<false>));
	map(0x3000, 0x37ff).ram().share(m_spriteram);
	map(0x3800, 0x3bff).ram().w(m_palette, FUNC(deco_rmc3_device::write8)).share("palette");
	map(0x3c00, 0x3c00).portr("IN0");
	map(0x3c01, 0x3c01).portr("IN1");
	map(0x3c02, 0x3c02).portr("IN2");
	map(0x3c03, 0x3c03).portr("DSW0");
	map(0x3c04, 0x3c04).portr("DSW1");
	map(0x3c00, 0x3c07).w(m_tilegen, FUNC(deco_bac06_device::ctrlreg8_w));
	map(0x3c10, 0x3c1f).w(m_tilegen, FUNC(deco_bac06_device::scrollreg8_w<false>));
	map(0x3c80, 0x3c80).w(FUNC(oscar_state::buffer_spriteram16_w)); // DMA
	map(0x3d00, 0x3d00).w(FUNC(oscar_state::bank_w)); // BNKS
	map(0x3d80, 0x3d80).w(m_soundlatch, FUNC(generic_latch_8_device::write)); // SOUN
	map(0x3e00, 0x3e00).w(FUNC(oscar_state::coin_clear_w)); // COINCL
	map(0x4000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0xffff).rom().region("maincpu", 0);
}

void oscar_state::oscar_sub_map(address_map &map)
{
	oscar_base_map(map);
	map(0x4000, 0xffff).rom().region("sub", 0x4000);
}


/******************************************************************************/

void oscar_state::oscar_sound_map(address_map &map)
{
	map(0x0000, 0x05ff).ram();
	map(0x2000, 0x2001).w("ym1", FUNC(ym2203_device::write));
	map(0x4000, 0x4001).w("ym2", FUNC(ym3526_device::write));
	map(0x6000, 0x6000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x8000, 0xffff).rom().region("audiocpu", 0);
}

// Used by the bootleg which has a standard M6502 with predecrypted opcodes
void oscar_state::oscarbl_sound_opcodes_map(address_map &map)
{
	map(0x8000, 0xffff).rom().region("audiocpu", 0x8000);
}


/******************************************************************************/

// verified from M6809 code
static INPUT_PORTS_START( oscar )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) // shoot
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) // jump
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) // select
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL // shoot
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL // jump
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL // select
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_WRITE_LINE_DEVICE_MEMBER("coin", FUNC(input_merger_device::in_w<1>))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_WRITE_LINE_DEVICE_MEMBER("coin", FUNC(input_merger_device::in_w<0>))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_WRITE_LINE_DEVICE_MEMBER("coin", FUNC(input_merger_device::in_w<2>))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )               PORT_DIPLOCATION("SW1:1,2") // table at 0xf8e3 (4 * 2 bytes : coins then credits)
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )               PORT_DIPLOCATION("SW1:3,4") // table at 0xf8eb (4 * 2 bytes : coins then credits)
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x10, 0x10, "Freeze Mode" )                   PORT_DIPLOCATION("SW1:5")
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
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )           PORT_DIPLOCATION("SW2:5,6") // tables at 0x82d8 (4 words) and 0xf3fe (3 words)
	PORT_DIPSETTING(    0x30, "40k 100k 60k+" )
	PORT_DIPSETTING(    0x20, "60k 160k 100k+" )
	PORT_DIPSETTING(    0x10, "90k 240k 150k+" )
	PORT_DIPSETTING(    0x00, "50k only" )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)")        PORT_DIPLOCATION("SW2:7") // not when falling into void or water - also gives infinite time
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
INPUT_PORTS_END

// verified from M6809 code
static INPUT_PORTS_START( oscarj )
	PORT_INCLUDE(oscar)

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )               PORT_DIPLOCATION("SW1:1,2") // table at 0xf8d6 (4 * 2 bytes : coins then credits) in 'oscarj1', 0xf8e6 in 'oscarj2', 0xf8f2 in 'oscaru'
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )               PORT_DIPLOCATION("SW1:3,4") // table at 0xf8de (4 * 2 bytes : coins then credits) in 'oscarj1', 0xf8ee in 'oscarj2', 0xf8fa in 'oscaru'
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )

	// bonus lives : tables at 0x82d8 (4 words) and 0xf3f1 (3 words) in 'oscarj1', 0x82de and 0xf401 in 'orscarj2', 0x82d8 and 0xf412 in 'orscaru' - same as in 'oscar'
INPUT_PORTS_END


/******************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,4),
	3,
	{ RGN_FRAC(3,4),RGN_FRAC(2,4),RGN_FRAC(1,4) },
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

static GFXDECODE_START( gfx_oscar )
	GFXDECODE_ENTRY( "char",    0, charlayout, 256, 8 ) // Chars
	GFXDECODE_ENTRY( "tiles",   0, tilelayout, 384, 8 ) // Tiles
GFXDECODE_END

static GFXDECODE_START( gfx_oscar_spr )
	GFXDECODE_ENTRY( "sprites", 0, tilelayout,   0, 16 ) // Sprites
GFXDECODE_END


/******************************************************************************/

// Coins generate NMI's
void oscar_state::coin_irq(int state)
{
	if (state && !m_coin_state)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	m_coin_state = bool(state);
}

void oscar_state::coin_clear_w(u8 data)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}


/******************************************************************************/

void oscar_state::machine_start()
{
	u8 *ROM = memregion("mainbank")->base();
	m_mainbank->configure_entries(0, 4, &ROM[0], 0x4000);

	save_item(NAME(m_coin_state));
}


void oscar_state::oscar(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, 12_MHz_XTAL / 8); // PCB seen both HD6309EP or MC6809EP, clock verified on pcb
	m_maincpu->set_addrmap(AS_PROGRAM, &oscar_state::oscar_map);

	MC6809E(config, m_subcpu, 12_MHz_XTAL / 8); // PCB seen both HD6309EP or MC6809EP, clock verified on pcb
	m_subcpu->set_addrmap(AS_PROGRAM, &oscar_state::oscar_sub_map);

	DECO_222(config, m_audiocpu, 12_MHz_XTAL / 8); // IC labeled "C10707-1"
	m_audiocpu->set_addrmap(AS_PROGRAM, &oscar_state::oscar_sound_map); // NMIs are caused by the main CPU

	config.set_maximum_quantum(attotime::from_hz(6000));

	INPUT_MERGER_ANY_LOW(config, "coin").output_handler().set(FUNC(oscar_state::coin_irq)); // 1S1588 x3 (D1-D3) + RCDM-I5

	// video hardware
	DECO_BAC06(config, m_tilegen);
	m_tilegen->set_gfx_region_wide(1, 1, 0);
	m_tilegen->set_gfxdecode_tag(m_gfxdecode);
	m_tilegen->set_tile_callback(FUNC(oscar_state::tile_cb));

	DECO_MXC06(config, m_spritegen, m_palette, gfx_oscar_spr);

	SCREEN(config, m_screen);
	// DECO video CRTC, matches PCB measurements
	m_screen->set_raw(12_MHz_XTAL / 2, 384, 0, 256, 272, 8, 248);
	m_screen->set_screen_update(FUNC(oscar_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_oscar);
	DECO_RMC3(config, m_palette, 0, 1024); // xxxxBBBBGGGGRRRR with custom weighting

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	INPUT_MERGER_ANY_HIGH(config, m_soundirq);
	m_soundirq->output_handler().set_inputline(m_audiocpu, m6502_device::IRQ_LINE);

	ym2203_device &ym1(YM2203(config, "ym1", 12_MHz_XTAL / 8)); // verified on pcb
	ym1.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<0>));
	ym1.add_route(0, "mono", 0.20);
	ym1.add_route(1, "mono", 0.20);
	ym1.add_route(2, "mono", 0.20);
	ym1.add_route(3, "mono", 0.40);

	ym3526_device &ym2(YM3526(config, "ym2", 12_MHz_XTAL / 4)); // verified on pcb
	ym2.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<1>));
	ym2.add_route(ALL_OUTPUTS, "mono", 0.80);
}

void oscar_state::oscarbl(machine_config &config)
{
	oscar(config);

	R65C02(config.replace(), m_audiocpu, 12_MHz_XTAL / 8);
	m_audiocpu->set_addrmap(AS_PROGRAM, &oscar_state::oscar_sound_map); // NMIs are caused by the main CPU
	m_audiocpu->set_addrmap(AS_OPCODES, &oscar_state::oscarbl_sound_opcodes_map);
}


/******************************************************************************/

ROM_START( oscar )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "du10", 0x00000, 0x08000, CRC(120040d8) SHA1(22d5f84f3ca724cbf39dfc4790f2175ba4945aaf) ) // This label is probably incorrect. The correct label is needed

	ROM_REGION( 0x10000, "mainbank", 0 )
	ROM_LOAD( "ed09", 0x00000, 0x10000, CRC(e2d4bba9) SHA1(99f0310debe51f4bcd00b5fdaedc1caf2eeccdeb) ) // for the world set because DU is the code for the Japanese version

	ROM_REGION( 0x10000, "sub", 0 ) // CPU 2, 1st 16k is empty
	ROM_LOAD( "du11", 0x0000, 0x10000, CRC(ff45c440) SHA1(4769944bcfebcdcba7ed7d5133d4d9f98890d75c) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "ed12", 0x0000, 0x8000, CRC(432031c5) SHA1(af2deea48b98eb0f9e85a4fb1486021f999f9abd) )

	ROM_REGION( 0x04000, "char", 0 )
	ROM_LOAD( "ed08", 0x00000, 0x04000, CRC(308ac264) SHA1(fd1c4ec4e4f99c33e93cd15e178c4714251a9b7e) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "ed04", 0x00000, 0x10000, CRC(416a791b) SHA1(e6541b713225289a43962363029eb0e22a1ecb4a) )
	ROM_LOAD( "ed05", 0x10000, 0x10000, CRC(fcdba431) SHA1(0be2194519c36ddf136610f60890506eda1faf0b) )
	ROM_LOAD( "ed06", 0x20000, 0x10000, CRC(7d50bebc) SHA1(06375f3273c48c7c7d81f1c15cbc5d3f3e05b8ed) )
	ROM_LOAD( "ed07", 0x30000, 0x10000, CRC(8fdf0fa5) SHA1(2b4d1ca1436864e89b13b3fa151a4a3708592e0a) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "ed01", 0x00000, 0x10000, CRC(d3a58e9e) SHA1(35eda2aa630fc2c11a1aff2b00bcfabe2f3d4249) )
	ROM_LOAD( "ed03", 0x10000, 0x10000, CRC(4fc4fb0f) SHA1(0906762a3adbffe765e072255262fedaa78bdb2a) )
	ROM_LOAD( "ed00", 0x20000, 0x10000, CRC(ac201f2d) SHA1(77f13eb6a1a44444ca9b25363031451b0d68c988) )
	ROM_LOAD( "ed02", 0x30000, 0x10000, CRC(7ddc5651) SHA1(f5ec5245cf3d9d4d9c1df6a8128c24565e331317) )
ROM_END

ROM_START( oscarbl ) // very similar to the original, main difference it's it uses a standard M6502 for sound with predecrypted opcodes.
	ROM_REGION( 0x8000, "maincpu", 0 ) // same as the original
	ROM_LOAD( "m27c256.3",  0x00000, 0x08000, CRC(120040d8) SHA1(22d5f84f3ca724cbf39dfc4790f2175ba4945aaf) )

	ROM_REGION( 0x10000, "mainbank", 0 )
	ROM_LOAD( "at27c512.2", 0x00000, 0x10000, CRC(e2d4bba9) SHA1(99f0310debe51f4bcd00b5fdaedc1caf2eeccdeb) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "at27c512.4", 0x00000, 0x10000, CRC(2ad9ef5d) SHA1(19db4446a6a5f75c7ddb2807b69d7c40d8b2d55a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "at27c512.1", 0x00000, 0x10000, CRC(302ff92c) SHA1(222cc1e4673a5439da1cdd07cc65dc23f522da1c) ) // data in the first half, opcodes in the second

	ROM_REGION( 0x04000, "char", 0 )
	ROM_LOAD( "ed08", 0x00000, 0x04000, BAD_DUMP CRC(308ac264) SHA1(fd1c4ec4e4f99c33e93cd15e178c4714251a9b7e) ) // not included in this set, probably same

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "at27c512.6", 0x00000, 0x10000, CRC(967315b5) SHA1(b7a081241477ab8e62fe3df1b7025b50ceaba180) )
	ROM_LOAD( "at27c512.7", 0x10000, 0x10000, CRC(fcdba431) SHA1(0be2194519c36ddf136610f60890506eda1faf0b) )
	ROM_LOAD( "at27c512.8", 0x20000, 0x10000, CRC(7d50bebc) SHA1(06375f3273c48c7c7d81f1c15cbc5d3f3e05b8ed) )
	ROM_LOAD( "at27c512.9", 0x30000, 0x10000, CRC(8fdf0fa5) SHA1(2b4d1ca1436864e89b13b3fa151a4a3708592e0a) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "at27c512.10", 0x00000, 0x10000, CRC(98cb4ffc) SHA1(75465a1e7c113db766e14d22b31bf999c520a8bf) )
	ROM_LOAD( "at27c512.11", 0x10000, 0x10000, CRC(bff7fddc) SHA1(129e99fa99920e647a53b9af64c9c288c1e8ad57) )
	ROM_LOAD( "at27c512.12", 0x20000, 0x10000, CRC(eff3b56c) SHA1(427d3aa053fa81ef004019eb8bb0aa97787f283e) )
	ROM_LOAD( "at27c512.13", 0x30000, 0x10000, CRC(7ddc5651) SHA1(f5ec5245cf3d9d4d9c1df6a8128c24565e331317) )
ROM_END

ROM_START( oscaru )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "ed10", 0x00000, 0x08000, CRC(f9b0d4d4) SHA1(dc2aba978ba96f365027c7be5714728d5d7fb802) )

	ROM_REGION( 0x10000, "mainbank", 0 )
	ROM_LOAD( "ed09", 0x00000, 0x10000, CRC(e2d4bba9) SHA1(99f0310debe51f4bcd00b5fdaedc1caf2eeccdeb) )

	ROM_REGION( 0x10000, "sub", 0 ) // CPU 2, 1st 16k is empty
	ROM_LOAD( "ed11", 0x0000, 0x10000,  CRC(10e5d919) SHA1(13bc3497cb4aaa6dd272853819212ad63656f8a7) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "ed12", 0x0000, 0x8000,  CRC(432031c5) SHA1(af2deea48b98eb0f9e85a4fb1486021f999f9abd) )

	ROM_REGION( 0x04000, "char", 0 )
	ROM_LOAD( "ed08", 0x00000, 0x04000, CRC(308ac264) SHA1(fd1c4ec4e4f99c33e93cd15e178c4714251a9b7e) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "ed04", 0x00000, 0x10000, CRC(416a791b) SHA1(e6541b713225289a43962363029eb0e22a1ecb4a) )
	ROM_LOAD( "ed05", 0x10000, 0x10000, CRC(fcdba431) SHA1(0be2194519c36ddf136610f60890506eda1faf0b) )
	ROM_LOAD( "ed06", 0x20000, 0x10000, CRC(7d50bebc) SHA1(06375f3273c48c7c7d81f1c15cbc5d3f3e05b8ed) )
	ROM_LOAD( "ed07", 0x30000, 0x10000, CRC(8fdf0fa5) SHA1(2b4d1ca1436864e89b13b3fa151a4a3708592e0a) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "ed01", 0x00000, 0x10000, CRC(d3a58e9e) SHA1(35eda2aa630fc2c11a1aff2b00bcfabe2f3d4249) )
	ROM_LOAD( "ed03", 0x10000, 0x10000, CRC(4fc4fb0f) SHA1(0906762a3adbffe765e072255262fedaa78bdb2a) )
	ROM_LOAD( "ed00", 0x20000, 0x10000, CRC(ac201f2d) SHA1(77f13eb6a1a44444ca9b25363031451b0d68c988) )
	ROM_LOAD( "ed02", 0x30000, 0x10000, CRC(7ddc5651) SHA1(f5ec5245cf3d9d4d9c1df6a8128c24565e331317) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "du-13.bin", 0x0000, 0x0200, CRC(bea1f87e) SHA1(f5215992e4b53c9cd4c7e0b20ff5cfdce3ab6d02) ) // Priority (Not yet used)
ROM_END

ROM_START( oscarj1 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "du10-1", 0x00000, 0x08000, CRC(4ebc9f7a) SHA1(df8fdc4b4b203dc1bdd048f069fb6b723bdea0d2) )

	ROM_REGION( 0x10000, "mainbank", 0 )
	ROM_LOAD( "ed09",   0x00000, 0x10000, CRC(e2d4bba9) SHA1(99f0310debe51f4bcd00b5fdaedc1caf2eeccdeb) )

	ROM_REGION( 0x10000, "sub", 0 ) // CPU 2, 1st 16k is empty
	ROM_LOAD( "du11", 0x0000, 0x10000, CRC(ff45c440) SHA1(4769944bcfebcdcba7ed7d5133d4d9f98890d75c) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "ed12", 0x0000, 0x8000, CRC(432031c5) SHA1(af2deea48b98eb0f9e85a4fb1486021f999f9abd) )

	ROM_REGION( 0x04000, "char", 0 )
	ROM_LOAD( "ed08", 0x00000, 0x04000, CRC(308ac264) SHA1(fd1c4ec4e4f99c33e93cd15e178c4714251a9b7e) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "ed04", 0x00000, 0x10000, CRC(416a791b) SHA1(e6541b713225289a43962363029eb0e22a1ecb4a) )
	ROM_LOAD( "ed05", 0x10000, 0x10000, CRC(fcdba431) SHA1(0be2194519c36ddf136610f60890506eda1faf0b) )
	ROM_LOAD( "ed06", 0x20000, 0x10000, CRC(7d50bebc) SHA1(06375f3273c48c7c7d81f1c15cbc5d3f3e05b8ed) )
	ROM_LOAD( "ed07", 0x30000, 0x10000, CRC(8fdf0fa5) SHA1(2b4d1ca1436864e89b13b3fa151a4a3708592e0a) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "ed01", 0x00000, 0x10000, CRC(d3a58e9e) SHA1(35eda2aa630fc2c11a1aff2b00bcfabe2f3d4249) )
	ROM_LOAD( "ed03", 0x10000, 0x10000, CRC(4fc4fb0f) SHA1(0906762a3adbffe765e072255262fedaa78bdb2a) )
	ROM_LOAD( "ed00", 0x20000, 0x10000, CRC(ac201f2d) SHA1(77f13eb6a1a44444ca9b25363031451b0d68c988) )
	ROM_LOAD( "ed02", 0x30000, 0x10000, CRC(7ddc5651) SHA1(f5ec5245cf3d9d4d9c1df6a8128c24565e331317) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "du-13.bin", 0x0000, 0x0200, CRC(bea1f87e) SHA1(f5215992e4b53c9cd4c7e0b20ff5cfdce3ab6d02) ) // Priority (Not yet used)
ROM_END

/***************************************************************************

Psycho-Nics Oscar (Data East, 1987)
Hardware info by Guru
---------------------

DE-0286-2
OSCAR 7891-1077 (sticker)
|--------------------------------------------------------------|
|       2018(2)     PR-1.F20           |--------|    DU07.A20  |
|       2018(2)                        |DATAEAST|              |
|                   PR-0.F18           |MXC 06  |              |
|RCDM-15                               |        |    DU06.A16  |
|      SW2                       2018  |--------|              |
|        DU11.H16   68B09(2)     2018                          |
|        5864                                        DU05.A14  |
|J                                     |-----------|           |
|A       DU10-2.H12              2018  | DATAEAST  |           |
|M                  68B09(1)           |  BAC 06   | DU04.A11  |
|M       DU09.H10                      |           |           |
|A                                     |           |           |
|                                      |-----------|   8416(1) |
|      SW1                                             8416(1) |
|                                        DU-13.C8    DU03.A6   |
|8416(3)   C10707-1              8416(2)                       |
|                      DU08.E5                       DU02.A5   |
|DU12.K5   YM3812                                              |
|3403 YM3014(1)                                      DU01.A3   |
|VOL  YM3014(2)                                                |
|MB3730    YM2203                            12MHz   DU00.A1   |
|--------------------------------------------------------------|
Notes:
    68B09(1) - Motorola MC68B09EP CPU. Clock Q & E 1.500MHz [12/8] (main program CPU)
    68B09(2) - Motorola MC68B09EP CPU. Clock Q & E 1.500MHz [12/8] (sub program CPU)
               Note: These are EP types so the clocks are measured on the Q & E quadrature clock inputs
    C10707-1 - Data East custom CPU marked 'C10707-1'. PCB marked 'DECO 222'. This is actually an encrypted 6502 CPU.
               Clocks are 1.500MHz on pins 3, 37 & 39
      YM2203 - Yamaha YM2203 FM Operator Type-N (OPN) sound chip. Clock 1.500MHz [12/8]
      YM3812 - Yamaha YM3812 FM Operator Type-L II (OPL II) Sound Chip. Clock 3.000MHz [12/4]
               Note this is fully compatible with YM3526 and either chip can be seen on PCBs in the wild
   YM3014(1) - Yamaha YM3014 Serial Input Floating D/A Converter. Clock 750kHz [12/4/4]. This is used with the YM3812
   YM3014(2) - Yamaha YM3014 Serial Input Floating D/A Converter. Clock 1.000MHz [12/4/3]. This is used with the YM2203
     8416(1) - Fujitsu MB8416 2kBx8-bit SRAM (background tile RAM)
     8416(2) - Fujitsu MB8416 2kBx8-bit SRAM (characters / text layer RAM)
     8416(3) - Fujitsu MB8416 2kBx8-bit SRAM (sound CPU RAM)
        5864 - Sony CXK5864 8kBx8-bit SRAM (main/sub CPU RAM)
        2018 - Toshiba TMM2018 2kBx8-bit SRAM (sprite RAM)
     2018(2) - Toshiba TMM2018 2kBx8-bit SRAM (color RAM)
       SW1/2 - 8-position DIP switch
        3403 - NEC uPC3403 Quad Operational Amplifier
      MB3730 - Fujitsu MB3730 14W BTL Audio Power Amplifier
     RCDM-15 - Custom SIP package for coin counters
      MXC 06 - Data East MXC 06 custom sprite generator
      BAC 06 - Data East BAC 06 custom tile generator
        PR-1 - MMI PAL16L8
        PR-0 - MMI PAL10L8
  DU10-2.H12 - 27C256 32kBx8-bit OTP EPROM (main CPU program)
    DU09.H10 - 27C512 64kBx8-bit OTP EPROM (main CPU program)
    DU11.H16 - 27C512 64kBx8-bit OTP EPROM (sub CPU program)
     DU12.K5 - 27C256 32kBx8-bit OTP EPROM (C10707-1 sound program)
     DU08.E5 - 27C128 16kBx8-bit OTP EPROM (characters / text layer)
     DU00.A1 - 27C512 64kBx8-bit OTP EPROM (tiles)
     DU01.A3 \
     DU02.A5 | 27C512 64kBx8-bit EPROM (tiles)
     DU03.A6 /
    DU04.A11 \ 27C512 64kBx8-bit EPROM (sprites)
    DU05.A14 /
    DU06.A16 \ 27C512 64kBx8-bit OTP EPROM (sprites)
    DU07.A20 /
    DU-13.C8 - Fujitsu MB7124 512x8-bit Bipolar PROM (priority)
               Compatible with 82S147. When removed sprites and characters/texts do not show on screen.
       HSync - 15.6178kHz. Measured on PROM at C8
       VSync - 57.4184Hz. Measured on logic near the 12MHz xtal

***************************************************************************/

ROM_START( oscarj2 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "du10-2.h12", 0x00000, 0x08000, CRC(114e898d) SHA1(1072ccabe6d53c50cdfa1e27d5d848ecdd6559cc) )

	ROM_REGION( 0x10000, "mainbank", 0 )
	ROM_LOAD( "du09.h10",   0x00000, 0x10000, CRC(e2d4bba9) SHA1(99f0310debe51f4bcd00b5fdaedc1caf2eeccdeb) )

	ROM_REGION( 0x10000, "sub", 0 ) // CPU 2, 1st 16k is empty
	ROM_LOAD( "du11.h16", 0x0000, 0x10000, CRC(ff45c440) SHA1(4769944bcfebcdcba7ed7d5133d4d9f98890d75c) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "du12.k5", 0x0000, 0x8000, CRC(432031c5) SHA1(af2deea48b98eb0f9e85a4fb1486021f999f9abd) )

	ROM_REGION( 0x04000, "char", 0 )
	ROM_LOAD( "du08.e5", 0x00000, 0x04000, CRC(308ac264) SHA1(fd1c4ec4e4f99c33e93cd15e178c4714251a9b7e) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "du04.a11", 0x00000, 0x10000, CRC(416a791b) SHA1(e6541b713225289a43962363029eb0e22a1ecb4a) )
	ROM_LOAD( "du05.a14", 0x10000, 0x10000, CRC(fcdba431) SHA1(0be2194519c36ddf136610f60890506eda1faf0b) )
	ROM_LOAD( "du06.a16", 0x20000, 0x10000, CRC(7d50bebc) SHA1(06375f3273c48c7c7d81f1c15cbc5d3f3e05b8ed) )
	ROM_LOAD( "du07.a20", 0x30000, 0x10000, CRC(8fdf0fa5) SHA1(2b4d1ca1436864e89b13b3fa151a4a3708592e0a) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "du01.a3", 0x00000, 0x10000, CRC(d3a58e9e) SHA1(35eda2aa630fc2c11a1aff2b00bcfabe2f3d4249) )
	ROM_LOAD( "du03.a6", 0x10000, 0x10000, CRC(4fc4fb0f) SHA1(0906762a3adbffe765e072255262fedaa78bdb2a) )
	ROM_LOAD( "du00.a1", 0x20000, 0x10000, CRC(ac201f2d) SHA1(77f13eb6a1a44444ca9b25363031451b0d68c988) )
	ROM_LOAD( "du02.a5", 0x30000, 0x10000, CRC(7ddc5651) SHA1(f5ec5245cf3d9d4d9c1df6a8128c24565e331317) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "du-13.c8", 0x0000, 0x0200, CRC(bea1f87e) SHA1(f5215992e4b53c9cd4c7e0b20ff5cfdce3ab6d02) ) // Priority (Not yet used)
ROM_END


/******************************************************************************/

} // anonymous namespace

GAME( 1987, oscar,      0,        oscar,     oscar,     oscar_state,    empty_init,     ROT0,   "Data East Corporation", "Psycho-Nics Oscar (World revision 0)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, oscarbl,    oscar,    oscarbl,   oscar,     oscar_state,    empty_init,     ROT0,   "bootleg",               "Psycho-Nics Oscar (World revision 0, bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, oscaru,     oscar,    oscar,     oscarj,    oscar_state,    empty_init,     ROT0,   "Data East USA",         "Psycho-Nics Oscar (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, oscarj1,    oscar,    oscar,     oscarj,    oscar_state,    empty_init,     ROT0,   "Data East Corporation", "Psycho-Nics Oscar (Japan revision 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, oscarj2,    oscar,    oscar,     oscarj,    oscar_state,    empty_init,     ROT0,   "Data East Corporation", "Psycho-Nics Oscar (Japan revision 2)", MACHINE_SUPPORTS_SAVE )
