// license:BSD-3-Clause
// copyright-holders:Bryan McPhail,Stephane Humbert
/***************************************************************************

Super Real Darwin (World)   (c) 1987 Data East Corporation
Super Real Darwin (Japan)   (c) 1987 Data East Corporation

Emulation by Bryan McPhail, mish@tendril.co.uk
Thanks to Jose Miguel Morales Farreras for Super Real Darwin information!

TODO:
- 'double' sprites appearing from the top of the screen are clipped

***************************************************************************/

#include "emu.h"

#include "deco222.h"
#include "decrmc3.h"

#include "cpu/m6809/m6809.h"
#include "cpu/mcs51/i8051.h"
#include "machine/gen_latch.h"
#include "machine/input_merger.h"
#include "sound/ymopn.h"
#include "sound/ymopl.h"
#include "video/bufsprite.h"

#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "multibyte.h"


namespace {

class srdarwin_state : public driver_device
{
public:
	srdarwin_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_spriteram(*this, "spriteram"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundirq(*this, "soundirq"),
		m_soundlatch(*this, "soundlatch"),
		m_mainbank(*this, "maincpu"),
		m_videoram(*this, "videoram"),
		m_bg_ram(*this, "bg_ram")
	{ }

	void srdarwinbl(machine_config &config) ATTR_COLD;

	void init_srdarwinbl() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void flip_screen_w(u8 data);
	void bg_ram_w(offs_t offset, u8 data);
	u8 bg_ram_r(offs_t offset);

	void videoram_w(offs_t offset, u8 data);
	void control_w(offs_t offset, u8 data);

	TILE_GET_INFO_MEMBER(get_fix_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &primap);

	void main_nomcu_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<buffered_spriteram8_device> m_spriteram;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<deco_rmc3_device> m_palette;
	required_device<input_merger_device> m_soundirq;
	required_device<generic_latch_8_device> m_soundlatch;

	// memory regions
	required_memory_bank m_mainbank;

	// memory pointers
	required_shared_ptr<u8> m_videoram;
	required_shared_ptr<u8> m_bg_ram;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fix_tilemap = nullptr;
	u8 m_scroll[2]{};
};

// with MCU
class srdarwin_mcu_state : public srdarwin_state
{
public:
	srdarwin_mcu_state(const machine_config &mconfig, device_type type, const char *tag) :
		srdarwin_state(mconfig, type, tag),
		m_mcu(*this, "mcu")
	{ }

	void srdarwin(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u8 i8751_hi_r();
	u8 i8751_lo_r();
	void i8751_hi_w(u8 data);
	void i8751_lo_w(u8 data);

	u8 i8751_port0_r();
	void i8751_port0_w(u8 data);

	void mcu_to_main_w(u8 data);

	void main_mcu_map(address_map &map) ATTR_COLD;

	// devices
	required_device<i8751_device> m_mcu;

	// MCU communication
	u8 m_i8751_p2 = 0;
	u8 m_i8751_port0 = 0;
	u16 m_i8751_return = 0;
	u16 m_i8751_value = 0;
};

/******************************************************************************/

void srdarwin_state::bg_ram_w(offs_t offset, u8 data)
{
	m_bg_ram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

u8 srdarwin_state::bg_ram_r(offs_t offset)
{
	return m_bg_ram[offset];
}

void srdarwin_state::videoram_w(offs_t offset, u8 data)
{
	m_videoram[offset] = data;
	m_fix_tilemap->mark_tile_dirty(offset);
}

void srdarwin_state::control_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 0: // Top 3 bits - bank switch, bottom 4 - scroll MSB
			m_mainbank->set_entry((data >> 5) & 7);
			m_scroll[0] = data & 0xf;
			break;

		case 1:
			m_scroll[1] = data;
			break;
	}
}

/******************************************************************************/

void srdarwin_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &primap)
{
	const u8 *const buffered_spriteram = m_spriteram->buffer();

	// Sprites
	for (int offs = 0x200 - 4; offs >= 0; offs -= 4)
	{
		u32 pri_mask = 0;

		const u32 color = (buffered_spriteram[offs + 1] & 0x03) + ((buffered_spriteram[offs + 1] & 0x08) >> 1);
		if (color == 0) pri_mask |= GFX_PMASK_2;

		const u32 code = buffered_spriteram[offs + 3] + ((buffered_spriteram[offs + 1] & 0xe0) << 3);
		if (!code) continue;

		int sy = buffered_spriteram[offs];
		if (sy == 0xf8) continue;

		int sx = (241 - buffered_spriteram[offs + 2]);

		bool fx = BIT(buffered_spriteram[offs + 1], 2);
		const bool multi = BIT(buffered_spriteram[offs + 1], 4);

		int sy2;
		if (flip_screen())
		{
			sy = 240 - sy;
			sx = 240 - sx;
			fx = !fx;
			sy2 = sy - 16;
		}
		else
			sy2 = sy + 16;

		m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
				code,
				color,
				fx,flip_screen(),
				sx,sy,primap,pri_mask,0);
		if (multi)
		{
			m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
					code+1,
					color,
					fx,flip_screen(),
					sx,sy2,primap,pri_mask,0);
		}
	}
}


/******************************************************************************/

u32 srdarwin_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	m_bg_tilemap->set_scrollx(0, get_u16be(&m_scroll[0]));

	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 1);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 2);
	draw_sprites(bitmap, cliprect, screen.priority());
	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

TILE_GET_INFO_MEMBER(srdarwin_state::get_fix_tile_info)
{
	const u8 tile = m_videoram[tile_index];
	const u8 color = 0; // ?

	tileinfo.category = 0;

	tileinfo.set(0, tile, color, 0);
}

TILE_GET_INFO_MEMBER(srdarwin_state::get_bg_tile_info)
{
	const u16 tile = get_u16be(&m_bg_ram[2 * tile_index]);
	const u8 color = (tile >> 12) & 3;
	const u8 bank = ((tile >> 8) & 3) + 2;

	tileinfo.set(bank, tile & 0xff, color, 0);
	tileinfo.group = color;
}

void srdarwin_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(srdarwin_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 16);
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(srdarwin_state::get_fix_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fix_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_transmask(0, 0xffff, 0x0000); // draw as background only
	m_bg_tilemap->set_transmask(1, 0x00ff, 0xff00); // Bottom 8 pens
	m_bg_tilemap->set_transmask(2, 0x00ff, 0xff00); // Bottom 8 pens
	m_bg_tilemap->set_transmask(3, 0x0000, 0xffff); // draw as foreground only
}


/******************************************************************************/

u8 srdarwin_mcu_state::i8751_hi_r()
{
	return m_i8751_return >> 8; // MSB
}

u8 srdarwin_mcu_state::i8751_lo_r()
{
	return m_i8751_return & 0xff; // LSB
}


/***************************************************
*
* Hook-up for games that have a proper MCU dump.
*
***************************************************/

void srdarwin_mcu_state::i8751_lo_w(u8 data)
{
	m_i8751_value = (m_i8751_value & 0xff00) | data;
}

void srdarwin_mcu_state::i8751_hi_w(u8 data)
{
	m_i8751_value = (m_i8751_value & 0xff) | (u16(data) << 8);

	// SECIRQ is triggered on activating this latch
	if (m_i8751_p2 & 2)
		m_mcu->set_input_line(MCS51_INT1_LINE, ASSERT_LINE);
}


void srdarwin_state::flip_screen_w(u8 data) { flip_screen_set(data); }


/******************************************************************************/

void srdarwin_state::main_nomcu_map(address_map &map)
{
	map(0x0000, 0x05ff).ram();
	map(0x0600, 0x07ff).ram().share("spriteram");
	map(0x0800, 0x0fff).ram().w(FUNC(srdarwin_state::videoram_w)).share(m_videoram);
	map(0x1000, 0x13ff).ram();
	map(0x1400, 0x17ff).rw(FUNC(srdarwin_state::bg_ram_r), FUNC(srdarwin_state::bg_ram_w)).share(m_bg_ram);
	map(0x1800, 0x1801).nopw();
	map(0x1803, 0x1803).nopw();
	map(0x1804, 0x1804).w(m_spriteram, FUNC(buffered_spriteram8_device::write));
	map(0x1805, 0x1806).w(FUNC(srdarwin_state::control_w));
	map(0x2000, 0x2000).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x2001, 0x2001).portr("COIN").w(FUNC(srdarwin_state::flip_screen_w));
	map(0x2800, 0x288f).w(m_palette, FUNC(deco_rmc3_device::write8)).share("palette");
	map(0x3000, 0x308f).w(m_palette, FUNC(deco_rmc3_device::write8_ext)).share("palette_ext");
	map(0x3800, 0x3800).portr("DSW0");
	map(0x3801, 0x3801).portr("IN0");
	map(0x3802, 0x3802).portr("IN1");
	map(0x3803, 0x3803).portr("DSW1");
	map(0x4000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0xffff).rom().region("maincpu", 0x18000);
}

void srdarwin_mcu_state::main_mcu_map(address_map &map)
{
	main_nomcu_map(map);
	map(0x1800, 0x1800).w(FUNC(srdarwin_mcu_state::i8751_hi_w));
	map(0x1801, 0x1801).w(FUNC(srdarwin_mcu_state::i8751_lo_w));
	map(0x2000, 0x2000).r(FUNC(srdarwin_mcu_state::i8751_hi_r));
	map(0x2001, 0x2001).r(FUNC(srdarwin_mcu_state::i8751_lo_r));
}


/******************************************************************************/

void srdarwin_state::sound_map(address_map &map)
{
	map(0x0000, 0x05ff).ram();
	map(0x2000, 0x2001).w("ym1", FUNC(ym2203_device::write));
	map(0x4000, 0x4001).w("ym2", FUNC(ym3812_device::write));
	map(0x6000, 0x6000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x8000, 0xffff).rom().region("audiocpu", 0);
}


/******************************************************************************/

/*
    Gondomania schematics show the following:

    Port P0 - attached to 2 * LS374 at location 4C & 1C
    Port P1 - attached to 2 * LS374 at location 3C & 2C
    Port P2.2 -> SECIRQ (IRQ to main CPU)
    Port P2.3 -> 'COUNT' (Enable coin counter - also wired directly to coinage) [not emulated]
    Port P2.4-7 -> Enable latches 4C, 1C, 3C, 2C
    Port P3.4-7 -> Directly attached to coinage connector (3 coins & service)

*/

u8 srdarwin_mcu_state::i8751_port0_r()
{
	return m_i8751_port0;
}

void srdarwin_mcu_state::i8751_port0_w(u8 data)
{
	m_i8751_port0 = data;
}

// Super Real Darwin is similar but only appears to have a single port
void srdarwin_mcu_state::mcu_to_main_w(u8 data)
{
	const u8 fall = ~data & m_i8751_p2;
	//const u8 rise = data & ~m_i8751_p2;
	m_i8751_p2 = data;

	// P24-P27: controls latches for main CPU communication
	if (BIT(fall, 4))
		m_i8751_port0 = m_i8751_value >> 8;
	if (BIT(fall, 5))
		m_i8751_port0 = m_i8751_value & 0xff;
	if (BIT(fall, 6))
		m_i8751_return = (m_i8751_return & 0xff) | (u16(m_i8751_port0) << 8);
	if (BIT(fall, 7))
		m_i8751_return = (m_i8751_return & 0xff00) | m_i8751_port0;

	// P22: maincpu has no IRQ/FIRQ handler

	// P21: clear MCU INT1
	if (BIT(~data, 1))
		m_mcu->set_input_line(MCS51_INT1_LINE, CLEAR_LINE);
}


/******************************************************************************/

// verified from M6809 code
static INPUT_PORTS_START( srdarwin )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COIN") // hooked up on the i8751
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )               PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )               PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW1:5")
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
	PORT_DIPSETTING(    0x00, "28 (Cheat)")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )           PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "Every 50k" )                     // table at 0xab06 - last bonus life at 850k
	PORT_DIPSETTING(    0x00, "Every 100k" )                    // table at 0xab17 - last bonus life at 900k
	PORT_DIPNAME( 0x20, 0x20, "After Stage 10" )                PORT_DIPLOCATION("SW2:6") // code at 0xab94
	PORT_DIPSETTING(    0x20, "Back to Stage 1" )
	PORT_DIPSETTING(    0x00, "Game Over" )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:7")
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
INPUT_PORTS_END

// verified from M6809 code
static INPUT_PORTS_START( srdarwinj )
	PORT_INCLUDE(srdarwin)

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )               PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )               PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
INPUT_PORTS_END


/******************************************************************************/

// SRDarwin characters - very unusual layout for Data East
static const gfx_layout charlayout =
{
	8,8,    // 8*8 characters
	RGN_FRAC(1,2),
	2,  // 2 bits per pixel
	{ 0, 4 },   // the two bitplanes for 4 pixels are packed into one byte
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+1, RGN_FRAC(1,2)+2, RGN_FRAC(1,2)+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 // every char takes 8 consecutive bytes
};

// Darwin sprites - only 3bpp
static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(1,3),RGN_FRAC(2,3),0 },
	{ 16*8, 1+(16*8), 2+(16*8), 3+(16*8), 4+(16*8), 5+(16*8), 6+(16*8), 7+(16*8),
		0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
//  { 0*8, 2*8, 1*8, 3*8, 4*8, 6*8, 5*8, 7*8 ,8*8,10*8,9*8,11*8,12*8,14*8,13*8,15*8 }, bootleg decode
	16*16
};

static const gfx_layout tilelayout =
{
	16,16,
	256,
	4,
	{ 0x4000*8, 0x4000*8+4, 0, 4 },
	{ 0, 1, 2, 3, 0x2000*8+0, 0x2000*8+1, 0x2000*8+2, 0x2000*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+0x2000*8+0, 16*8+0x2000*8+1, 16*8+0x2000*8+2, 16*8+0x2000*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    // every tile takes 32 consecutive bytes
};

static GFXDECODE_START( gfx_srdarwin )
	GFXDECODE_ENTRY( "char",    0x00000, charlayout,  128, 4 ) // Only 1 used so far :/
	GFXDECODE_ENTRY( "sprites", 0x00000, spritelayout, 64, 8 )
	GFXDECODE_ENTRY( "tiles",   0x00000, tilelayout,    0, 8 )
	GFXDECODE_ENTRY( "tiles",   0x08000, tilelayout,    0, 8 )
	GFXDECODE_ENTRY( "tiles",   0x10000, tilelayout,    0, 8 )
	GFXDECODE_ENTRY( "tiles",   0x18000, tilelayout,    0, 8 )
GFXDECODE_END


/******************************************************************************/

void srdarwin_state::machine_start()
{
	u8 *ROM = memregion("maincpu")->base();
	m_mainbank->configure_entries(0, 8, &ROM[0], 0x4000);

	save_item(NAME(m_scroll));
}

void srdarwin_state::machine_reset()
{
	m_scroll[0] = m_scroll[1] = 0;
}


void srdarwin_mcu_state::machine_start()
{
	srdarwin_state::machine_start();

	m_i8751_p2 = 0xff;

	save_item(NAME(m_i8751_p2));
	save_item(NAME(m_i8751_port0));
	save_item(NAME(m_i8751_return));
	save_item(NAME(m_i8751_value));
}

void srdarwin_mcu_state::machine_reset()
{
	srdarwin_state::machine_reset();

	m_i8751_return = m_i8751_value = 0;
}


void srdarwin_state::srdarwinbl(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, 1'500'000); // MC68A09EP or HD63?09EP
	m_maincpu->set_addrmap(AS_PROGRAM, &srdarwin_state::main_nomcu_map);

	DECO_222(config, m_audiocpu, 1'500'000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &srdarwin_state::sound_map);
	// NMIs are caused by the main CPU

	// video hardware
	BUFFERED_SPRITERAM8(config, m_spriteram);

	SCREEN(config, m_screen);
	// DECO video CRTC, matches PCB measurements
	m_screen->set_raw(12_MHz_XTAL / 2, 384, 0, 256, 272, 8, 248);
	m_screen->set_screen_update(FUNC(srdarwin_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_srdarwin);
	DECO_RMC3(config, m_palette, 0, 144); // xxxxBBBBGGGGRRRR with custom weighting

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

void srdarwin_mcu_state::srdarwin(machine_config &config)
{
	srdarwinbl(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &srdarwin_mcu_state::main_mcu_map);

	I8751(config, m_mcu, 8_MHz_XTAL); // unknown frequency
	m_mcu->port_in_cb<0>().set(FUNC(srdarwin_mcu_state::i8751_port0_r));
	m_mcu->port_out_cb<0>().set(FUNC(srdarwin_mcu_state::i8751_port0_w));
	m_mcu->port_out_cb<2>().set(FUNC(srdarwin_mcu_state::mcu_to_main_w));
	m_mcu->port_in_cb<3>().set_ioport("COIN");

	config.set_perfect_quantum(m_mcu);
}


/******************************************************************************/

ROM_START( srdarwin )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "dy_00.b16",   0x00000, 0x10000, CRC(2bf6b461) SHA1(435d922c7b9df7f2b2f774346caed81d330be8a0) )
	ROM_LOAD( "dy_01-e.b14", 0x10000, 0x10000, CRC(176e9299) SHA1(20cd44ab610e384ab4f0172054c9adc432b12e9c) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "dy_04.d7", 0x0000, 0x8000, CRC(2ae3591c) SHA1(f21b06d84e2c3d3895be0812024641fd006e45cf) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	// 0160: B4 6B 0D : cjne  a,#$6B,$0170 (ID code 0x6b = World version)
	ROM_LOAD( "dy-e.d11", 0x0000, 0x1000, CRC(11cd6ca4) SHA1(ec70f84228e37f9fc1bda85fa52a009f61c500b2) )

	ROM_REGION( 0x4000, "char", 0 )
	ROM_LOAD( "dy_05.b6", 0x0000, 0x4000, CRC(8780e8a3) SHA1(03ea91fdc5aba8e139201604fb3bf9b69f71f056) )

	ROM_REGION( 0x30000, "sprites", 0 )
	ROM_LOAD( "dy_07.h16", 0x00000, 0x8000, CRC(97eaba60) SHA1(e3252b67bad7babcf4ece39f46ae4aeb950eb92b) )
	ROM_LOAD( "dy_06.h14", 0x08000, 0x8000, CRC(c279541b) SHA1(eb3737413499d07b6c2af99a95b27b2590e670c5) )
	ROM_LOAD( "dy_09.k13", 0x10000, 0x8000, CRC(d30d1745) SHA1(647b6121ab6fa812368da45e1295cc41f73be89d) )
	ROM_LOAD( "dy_08.k11", 0x18000, 0x8000, CRC(71d645fd) SHA1(a74a9b9697fc39b4e675e732a9d7d82976cc95dd) )
	ROM_LOAD( "dy_11.k16", 0x20000, 0x8000, CRC(fd9ccc5b) SHA1(b38c44c01acdc455d4192e4c8be1d68d9eb0c7b6) )
	ROM_LOAD( "dy_10.k14", 0x28000, 0x8000, CRC(88770ab8) SHA1(0a4a807a8d3b0653864bd984872d5567836f8cf8) )

	ROM_REGION( 0x20000, "tiles", 0 )
	ROM_LOAD( "dy_03.b4",  0x00000, 0x4000, CRC(44f2a4f9) SHA1(97368dd112451cd630f2fa5ba54679e84e7d4d97) )
	ROM_CONTINUE(          0x08000, 0x4000 )
	ROM_CONTINUE(          0x10000, 0x4000 )
	ROM_CONTINUE(          0x18000, 0x4000 )
	ROM_LOAD( "dy_02.b5",  0x04000, 0x4000, CRC(522d9a9e) SHA1(248274ed6df604357cad386fcf0521b26810aa0e) )
	ROM_CONTINUE(          0x0c000, 0x4000 )
	ROM_CONTINUE(          0x14000, 0x4000 )
	ROM_CONTINUE(          0x1c000, 0x4000 )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "dy_12.f4",  0x0000, 0x0100, CRC(ebfaaed9) SHA1(5723dbfa3eb3fc4df8c8975b320a5c49848309d8) ) // Priority (Not yet used)
ROM_END

ROM_START( srdarwinj )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "dy_00.b16", 0x00000, 0x10000, CRC(2bf6b461) SHA1(435d922c7b9df7f2b2f774346caed81d330be8a0) )
	ROM_LOAD( "dy_01.b14", 0x10000, 0x10000, CRC(1eeee4ff) SHA1(89a70de8bd61c671582b11773ce69b2edcd9c2f8) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "dy_04.d7", 0x0000, 0x8000, CRC(2ae3591c) SHA1(f21b06d84e2c3d3895be0812024641fd006e45cf) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	ROM_LOAD( "dy.d11", 0x0000, 0x1000, BAD_DUMP CRC(4ac2ca9d) SHA1(6e07788df9fcf4248a9d3e87b8c5f54776bd269e) ) // hand-modified copy of world version to correct region + coinage

	ROM_REGION( 0x4000, "char", 0 )
	ROM_LOAD( "dy_05.b6", 0x0000, 0x4000, CRC(8780e8a3) SHA1(03ea91fdc5aba8e139201604fb3bf9b69f71f056) )

	ROM_REGION( 0x30000, "sprites", 0 )
	ROM_LOAD( "dy_07.h16", 0x00000, 0x8000, CRC(97eaba60) SHA1(e3252b67bad7babcf4ece39f46ae4aeb950eb92b) )
	ROM_LOAD( "dy_06.h14", 0x08000, 0x8000, CRC(c279541b) SHA1(eb3737413499d07b6c2af99a95b27b2590e670c5) )
	ROM_LOAD( "dy_09.k13", 0x10000, 0x8000, CRC(d30d1745) SHA1(647b6121ab6fa812368da45e1295cc41f73be89d) )
	ROM_LOAD( "dy_08.k11", 0x18000, 0x8000, CRC(71d645fd) SHA1(a74a9b9697fc39b4e675e732a9d7d82976cc95dd) )
	ROM_LOAD( "dy_11.k16", 0x20000, 0x8000, CRC(fd9ccc5b) SHA1(b38c44c01acdc455d4192e4c8be1d68d9eb0c7b6) )
	ROM_LOAD( "dy_10.k14", 0x28000, 0x8000, CRC(88770ab8) SHA1(0a4a807a8d3b0653864bd984872d5567836f8cf8) )

	ROM_REGION( 0x20000, "tiles", 0 )
	ROM_LOAD( "dy_03.b4",  0x00000, 0x4000, CRC(44f2a4f9) SHA1(97368dd112451cd630f2fa5ba54679e84e7d4d97) )
	ROM_CONTINUE(          0x08000, 0x4000 )
	ROM_CONTINUE(          0x10000, 0x4000 )
	ROM_CONTINUE(          0x18000, 0x4000 )
	ROM_LOAD( "dy_02.b5",  0x04000, 0x4000, CRC(522d9a9e) SHA1(248274ed6df604357cad386fcf0521b26810aa0e) )
	ROM_CONTINUE(          0x0c000, 0x4000 )
	ROM_CONTINUE(          0x14000, 0x4000 )
	ROM_CONTINUE(          0x1c000, 0x4000 )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "dy_12.f4",  0x0000, 0x0100, CRC(ebfaaed9) SHA1(5723dbfa3eb3fc4df8c8975b320a5c49848309d8) ) // Priority (Not yet used)
ROM_END

ROM_START( srdarwinjbl )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "4.bin",  0x00000, 0x10000, CRC(2bf6b461) SHA1(435d922c7b9df7f2b2f774346caed81d330be8a0) )
	ROM_LOAD( "3.bin",  0x10000, 0x10000, CRC(7942b43f) SHA1(15de0c02d45d06c145fba48ef05baae793a1cb46) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "5.bin", 0x0000, 0x8000, CRC(2ae3591c) SHA1(f21b06d84e2c3d3895be0812024641fd006e45cf) )

	ROM_REGION( 0x4000, "char", 0 )
	ROM_LOAD( "12.bin",  0x0000, 0x4000, CRC(f5c835cd) SHA1(8b41862dc18ba2e2677c94ecef45a40467aa89ad) )
	ROM_CONTINUE(        0x0000, 0x4000 ) // all data in the 2nd half

	ROM_REGION( 0x30000, "sprites", 0 )
	ROM_LOAD( "6.bin",   0x00000, 0x8000, CRC(3c84a2c6) SHA1(558a7d9acb5af06a7728d010262e3a35c3cdbe25) )
	ROM_LOAD( "7.bin",   0x08000, 0x8000, CRC(990cfc7b) SHA1(84ab42010a483e8a4cd86357898f55e644d1b11e) )
	ROM_LOAD( "10.bin",  0x10000, 0x8000, CRC(cf7dcdc1) SHA1(a94a970ff564da0fef8cd7cdcbf9aee5f83b596c) )
	ROM_LOAD( "11.bin",  0x18000, 0x8000, CRC(3674e392) SHA1(d91387c2412bf950993751c2e4764f818489316f) )
	ROM_LOAD( "8.bin",   0x20000, 0x8000, CRC(cc39b73f) SHA1(8cdbef67526f29ccf77c04aec5e34253bcdf96c3) )
	ROM_LOAD( "9.bin",   0x28000, 0x8000, CRC(d15aaa08) SHA1(69649f0cb2f11107b37ae7914dd90f4c6269316f) )

	ROM_REGION( 0x20000, "tiles", 0 )
	ROM_LOAD( "1.bin",  0x00000, 0x4000, CRC(44f2a4f9) SHA1(97368dd112451cd630f2fa5ba54679e84e7d4d97) )
	ROM_CONTINUE(       0x08000, 0x4000 )
	ROM_CONTINUE(       0x10000, 0x4000 )
	ROM_CONTINUE(       0x18000, 0x4000 )
	ROM_LOAD( "2.bin",  0x04000, 0x4000, CRC(522d9a9e) SHA1(248274ed6df604357cad386fcf0521b26810aa0e) )
	ROM_CONTINUE(       0x0c000, 0x4000 )
	ROM_CONTINUE(       0x14000, 0x4000 )
	ROM_CONTINUE(       0x1c000, 0x4000 )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s123.bin",  0x0000, 0x0100, NO_DUMP ) // Priority (Not yet used)
ROM_END


void srdarwin_state::init_srdarwinbl()
{
	// this bootleg has the sprite ROMs with bytes swapped.
	// just worked here to avoid a new gfxdecode and machine driver.
	u8 *rom = memregion("sprites")->base();

	for (int i = 0; i < 0x30000; i += 4)
	{
		u8 byte1 = rom[i + 1];

		rom[i + 1] = rom[i + 2];
		rom[i + 2] = byte1;
	}
}


/******************************************************************************/

} // anonymous namespace

GAME( 1987, srdarwin,    0,        srdarwin,   srdarwin,  srdarwin_mcu_state, empty_init,      ROT270, "Data East Corporation", "SRD: Super Real Darwin (World)",          MACHINE_SUPPORTS_SAVE )
GAME( 1987, srdarwinj,   srdarwin, srdarwin,   srdarwinj, srdarwin_mcu_state, empty_init,      ROT270, "Data East Corporation", "SRD: Super Real Darwin (Japan)",          MACHINE_SUPPORTS_SAVE )
GAME( 1987, srdarwinjbl, srdarwin, srdarwinbl, srdarwinj, srdarwin_state,     init_srdarwinbl, ROT270, "bootleg",               "SRD: Super Real Darwin (Japan, bootleg)", MACHINE_SUPPORTS_SAVE )
