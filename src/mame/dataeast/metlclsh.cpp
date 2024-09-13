// license:BSD-3-Clause
// copyright-holders: Luca Elia

/***************************************************************************

                              -= Metal Clash =-

    driver by Luca Elia, based on brkthru.c by Phil Stroffolino


CPUs    :   2 x 6809
Sound   :   YM2203  +  YM3526
Video   :   TC15G008AP + TC15G032CY (TOSHIBA)

---------------------------------------------------------------------------
Year + Game         Boards
---------------------------------------------------------------------------
85  Metal Clash     DE-0212-1 & DE-0213-1
---------------------------------------------------------------------------

Notes:

- Similar hardware to that in dataeast/brkthru.cpp
- Screenshots here: www.ne.jp/asahi/cc-sakura/akkun/bekkan/metal.html

To Do:

metlclsh:
- Clocks are all unknown
- Text on the title screen has wrong colors the first time around
  (uninitialized foreground palette 1, will be initialized shortly)
- The background tilemap RAM is bankswitched with other (not understood) RAM
- There are a few unknown writes

***************************************************************************/

#include "emu.h"

#include "cpu/m6809/m6809.h"
#include "sound/ymopl.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_SCROLLX (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_SCROLLX)

#include "logmacro.h"

#define LOGSCROLLX(...) LOGMASKED(LOG_SCROLLX, __VA_ARGS__)


namespace {

class metlclsh_state : public driver_device
{
public:
	metlclsh_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_fgram(*this, "fgram"),
		m_spriteram(*this, "spriteram"),
		m_bgram(*this, "bgram"),
		m_scrollx(*this, "scrollx"),
		m_rambank(*this, "rambank"),
		m_otherram(*this, "otherram", 0x800, ENDIANNESS_BIG), // banked RAM
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	void metlclsh(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_fgram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_bgram;
	required_shared_ptr<uint8_t> m_scrollx;
	required_memory_bank m_rambank;
	memory_share_creator<uint8_t> m_otherram;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	uint8_t m_write_mask = 0;
	uint8_t m_gfxbank = 0;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	void cause_irq(uint8_t data);
	void ack_nmi(uint8_t data);
	void cause_nmi2(uint8_t data);
	void ack_irq2(uint8_t data);
	void ack_nmi2(uint8_t data);
	void flipscreen_w(uint8_t data);
	void rambank_w(uint8_t data);
	void gfxbank_w(uint8_t data);
	void bgram_w(offs_t offset, uint8_t data);
	void fgram_w(offs_t offset, uint8_t data);
	TILEMAP_MAPPER_MEMBER(bgtilemap_scan);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void master_map(address_map &map);
	void slave_map(address_map &map);
};


/***************************************************************************

                              -= Metal Clash =-

    Sprites:
        16x16 tiles with 8 pens and 2 color codes,
        each sprite can be 1 or 2 tiles tall

    Background:
        size is 512x256 (scrolling, peculiar memory layout),
        16x16 tiles with 8 pens and no color code,
        1 byte per tile (tiles are banked)

    Foreground:
        size is 256x256 (no scrolling),
        8x8 tiles with 4 pens and 4 color codes,
        2 bytes per tile, low and high per tile priority

***************************************************************************/


void metlclsh_state::rambank_w(uint8_t data)
{
	m_rambank->set_entry(data & 1);
	if (data & 1)
		m_write_mask = 0;
	else
		m_write_mask = 1 << (data >> 1);
}

void metlclsh_state::gfxbank_w(uint8_t data)
{
	if (!(data & 4) && (m_gfxbank != data))
	{
		m_bg_tilemap->mark_all_dirty();
		m_gfxbank = data & 3;
	}
}

/***************************************************************************

                            Background tilemap

                memory offset of each tile of the tilemap:

                    00 08  ..  78 100 108  ..  178
                     .  .       .   .   .        .
                     .  .       .   .   .        .
                    07 0f  ..  7f 107 10f  ..  17f
                    80 88  ..  f8 180 188  ..  1f8
                     .  .       .   .   .        .
                     .  .       .   .   .        .
                    87 8f  ..  ff 187 18f  ..  1ff

***************************************************************************/

TILEMAP_MAPPER_MEMBER(metlclsh_state::bgtilemap_scan)
{
	return (row & 7) + ((row & ~7) << 4) + ((col & 0xf) << 3) + ((col & ~0xf) << 4);
}

TILE_GET_INFO_MEMBER(metlclsh_state::get_bg_tile_info)
{
	tileinfo.set(1, m_bgram[tile_index] + (m_gfxbank << 7), 0, 0);
}

void metlclsh_state::bgram_w(offs_t offset, uint8_t data)
{
	/*  This RAM is banked: it's either the tilemap (e401 = 1)
	    or bit n of another area (e401 = n << 1)? (that I don't understand) */
	if (m_write_mask)
	{
		// unknown area - the following is almost surely wrong
		// 405b (e401 = e c a 8 6 4 2 0) writes d400++
		// 4085 (e401 = e c a 8 6 4 2 0) writes d400++
		// 4085 (e401 = e a 6 2) writes d000++
		// 405b (e401 = e a 6 2) writes d000++

//      m_otherram[offset] |= (data & m_write_mask);
		m_otherram[offset] = (m_otherram[offset] & ~m_write_mask) | (data & m_write_mask);
	}
	else
	{
		// tilemap
		m_bgram[offset] = data;
		m_bg_tilemap->mark_tile_dirty(offset & 0x1ff);
	}
}

/***************************************************************************

                            Foreground tilemap

    Offset:     Bits:           Value:

        0x000                   Code
        0x400   7--- ----       Priority (0/1 -> over/below sprites and background)
                -65- ----       Color
                ---- --10       Code

***************************************************************************/

TILE_GET_INFO_MEMBER(metlclsh_state::get_fg_tile_info)
{
	uint8_t const code = m_fgram[tile_index + 0x000];
	uint8_t const attr = m_fgram[tile_index + 0x400];
	tileinfo.set(2, code + ((attr & 0x03) << 8), (attr >> 5) & 3, 0);
	tileinfo.category = ((attr & 0x80) ? 1 : 2);
}

void metlclsh_state::fgram_w(offs_t offset, uint8_t data)
{
	m_fgram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}


/***************************************************************************

                            Video Hardware Init

***************************************************************************/

void metlclsh_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(metlclsh_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(metlclsh_state::bgtilemap_scan)), 16, 16, 32, 16);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(metlclsh_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_bg_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_transparent_pen(0);

	m_rambank->configure_entry(0, m_otherram);
	m_rambank->configure_entry(1, m_bgram.target());
	m_rambank->set_entry(0);
}


/***************************************************************************

                                Sprites Drawing

    Offset:     Bits:           Value:

        0       7--- ----       0
                -65- ----       Code (high bits)
                ---4 ----       Double height (2 tiles)
                ---- 3---       Color
                ---- -2--       Flip X
                ---- --1-       Flip Y
                ---- ---0       Enable

        1                       Code (low bits)
        2                       Y (bottom -> top)
        3                       X (right -> left)

***************************************************************************/

void metlclsh_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);

	for (int offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int const attr = m_spriteram[offs];
		if (!(attr & 0x01))
			continue;   // enable

		int flipy = (attr & 0x02);
		int flipx = (attr & 0x04);
		int const color = (attr & 0x08) >> 3;
		int const sizey = (attr & 0x10);  // double height
		int const code = ((attr & 0x60) << 3) + m_spriteram[offs + 1];

		int sx = 240 - m_spriteram[offs + 3];
		if (sx < -7)
			sx += 256;

		int sy = 240 - m_spriteram[offs + 2];

		if (flip_screen())
		{
			sx = 240 - sx; flipx = !flipx;
			sy = 240 - sy; flipy = !flipy;
			if (sizey) sy += 16;
			if (sy > 240) sy -= 256;
		}

		// Draw twice, at sy and sy + 256 (wrap around)
		for (int wrapy = 0; wrapy <= 256; wrapy += 256)
		{
			if (sizey)
			{
				gfx->transpen(bitmap, cliprect, code & ~1, color, flipx, flipy,
						sx, sy + (flipy ? 0 : -16) + wrapy, 0);

				gfx->transpen(bitmap, cliprect, code | 1, color, flipx, flipy,
						sx, sy + (flipy ? -16 : 0) + wrapy, 0);
			}
			else
			{
				gfx->transpen(bitmap, cliprect, code, color, flipx, flipy,
						sx, sy + wrapy, 0);
			}
		}
	}
}

/***************************************************************************

                                Screen Drawing

    Video register e402 (metlclsh seems to only use the values 0,8,9,b):

        7654 ----       0
        ---- 3---       Background enable
        ---- -2--       0
        ---- --1-       Background scroll x high bit
        ---- ---0       ? (not bg flipx!)

***************************************************************************/

uint32_t metlclsh_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x10, cliprect);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 1, 0); // low priority tiles of foreground

	if (m_scrollx[0] & 0x08) // background (if enabled)
	{
		// The background seems to be always flipped along x
		m_bg_tilemap->set_flip((flip_screen() ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0) ^ TILEMAP_FLIPX);
		m_bg_tilemap->set_scrollx(0, m_scrollx[1] + ((m_scrollx[0] & 0x02) << 7));
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	}
	draw_sprites(bitmap, cliprect);          // sprites
	m_fg_tilemap->draw(screen, bitmap, cliprect, 2, 0); // high priority tiles of foreground

	LOGSCROLLX("%02x", m_scrollx[0]);
	return 0;
}


/***************************************************************************

                            Memory Maps - CPU #1

***************************************************************************/

void metlclsh_state::cause_irq(uint8_t data)
{
	m_subcpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
}

void metlclsh_state::ack_nmi(uint8_t data)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void metlclsh_state::master_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).ram().share("shared");
	map(0xa000, 0xbfff).rom();
	map(0xc000, 0xc000).portr("IN0");
	map(0xc001, 0xc001).portr("IN1");
	map(0xc002, 0xc002).portr("IN2");
	map(0xc003, 0xc003).portr("DSW");
	map(0xc080, 0xc080).nopw(); // ? 0
	map(0xc0c2, 0xc0c2).w(FUNC(metlclsh_state::cause_irq)); // cause irq on cpu #2
	map(0xc0c3, 0xc0c3).w(FUNC(metlclsh_state::ack_nmi));
	map(0xc800, 0xc82f).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xcc00, 0xcc2f).ram().w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0xd000, 0xd001).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xd800, 0xdfff).ram().w(FUNC(metlclsh_state::fgram_w)).share(m_fgram);
	map(0xe000, 0xe001).w("ym2", FUNC(ym3526_device::write));
	map(0xe800, 0xe9ff).ram().share(m_spriteram);
	map(0xfff0, 0xffff).rom(); // Reset/IRQ vectors
}


/***************************************************************************

                            Memory Maps - CPU #2

***************************************************************************/

void metlclsh_state::cause_nmi2(uint8_t data)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

void metlclsh_state::ack_irq2(uint8_t data)
{
	m_subcpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
}

void metlclsh_state::ack_nmi2(uint8_t data)
{
	m_subcpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void metlclsh_state::flipscreen_w(uint8_t data)
{
	flip_screen_set(data & 1);
}

void metlclsh_state::slave_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).ram().share("shared");
	map(0xc000, 0xc000).portr("IN0").w(FUNC(metlclsh_state::gfxbank_w)); // bg tiles bank
	map(0xc001, 0xc001).portr("IN1");
	map(0xc002, 0xc002).portr("IN2");
	map(0xc003, 0xc003).portr("DSW");
	map(0xc0c0, 0xc0c0).w(FUNC(metlclsh_state::cause_nmi2)); // cause nmi on cpu #1
	map(0xc0c1, 0xc0c1).w(FUNC(metlclsh_state::ack_irq2));
	map(0xd000, 0xd7ff).bankr("rambank").w(FUNC(metlclsh_state::bgram_w)).share(m_bgram); // this is banked
	map(0xe301, 0xe301).w(FUNC(metlclsh_state::flipscreen_w));
	map(0xe401, 0xe401).w(FUNC(metlclsh_state::rambank_w));
	map(0xe402, 0xe403).writeonly().share(m_scrollx);
//  map(0xe404, 0xe404).nopw(); // ? 0
//  map(0xe410, 0xe410).nopw(); // ? 0 on startup only
	map(0xe417, 0xe417).w(FUNC(metlclsh_state::ack_nmi2));
	map(0xfff0, 0xffff).rom(); // Reset/IRQ vectors
}


/***************************************************************************

                                Input Ports

***************************************************************************/


INPUT_CHANGED_MEMBER(metlclsh_state::coin_inserted)
{
	if (oldval)
		m_subcpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

static INPUT_PORTS_START( metlclsh )
	PORT_START("IN0")       // c000
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Infinite Energy (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Infinite Lives (Cheat)")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")       // c001
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN2")       // c002
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, metlclsh_state, coin_inserted, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, metlclsh_state, coin_inserted, 0)

	PORT_START("DSW")       // c003
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPNAME( 0x02, 0x02, "Enemies Speed" )
	PORT_DIPSETTING(    0x02, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x04, 0x04, "Enemies Energy" )
	PORT_DIPSETTING(    0x04, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x08, 0x08, "Time" )
	PORT_DIPSETTING(    0x00, "75" )
	PORT_DIPSETTING(    0x08, "90" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )   // cpu2 will clr c040 on startup forever
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, metlclsh_state, coin_inserted, 0)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END


/***************************************************************************

                            Graphics Layouts

***************************************************************************/

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ STEP8(8*8*2,1), STEP8(8*8*0,1) },
	{ STEP8(8*8*0,8), STEP8(8*8*1,8) },
	16*16
};

static const gfx_layout tilelayout16 =
{
	16,16,
	RGN_FRAC(1,4),
	3,
	{ RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ STEP8(8*8*0+7,-1), STEP8(8*8*2+7,-1) },
	{ STEP8(8*8*0,8), STEP8(8*8*1,8) },
	16*16
};

static const gfx_layout tilelayout8 =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ 0, 4 },
	{ STEP4(RGN_FRAC(1,2),1), STEP4(0,1) },
	{ STEP8(0,8) },
	8*8
};

static GFXDECODE_START( gfx_metlclsh )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout, 0x00, 2 )
	GFXDECODE_ENTRY( "bgtiles", 0, tilelayout16, 0x10, 1 )
	GFXDECODE_ENTRY( "fgtiles", 0, tilelayout8,  0x20, 4 )
GFXDECODE_END


/***************************************************************************

                                Machine Drivers

***************************************************************************/

void metlclsh_state::machine_start()
{
	save_item(NAME(m_write_mask));
	save_item(NAME(m_gfxbank));
}

void metlclsh_state::machine_reset()
{
	flip_screen_set(0);

	m_write_mask = 0;
	m_gfxbank = 0;
}

void metlclsh_state::metlclsh(machine_config &config)
{
	// basic machine hardware
	M6809(config, m_maincpu, 1'500'000); // ?
	m_maincpu->set_addrmap(AS_PROGRAM, &metlclsh_state::master_map);
	// IRQ by YM3526, NMI by CPU #2

	M6809(config, m_subcpu, 1'500'000); // ?
	m_subcpu->set_addrmap(AS_PROGRAM, &metlclsh_state::slave_map);
	// IRQ by CPU #1, NMI by coin insertion

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(58);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate. We're using PORT_VBLANK
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 1*8, 30*8-1);
	screen.set_screen_update(FUNC(metlclsh_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_metlclsh);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 3 * 16);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2203_device &ym1(YM2203(config, "ym1", 1'500'000));
	ym1.add_route(0, "mono", 0.10);
	ym1.add_route(1, "mono", 0.10);
	ym1.add_route(2, "mono", 0.10);
	ym1.add_route(3, "mono", 0.50);

	ym3526_device &ym2(YM3526(config, "ym2", 3'000'000));
	ym2.irq_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);
	ym2.add_route(ALL_OUTPUTS, "mono", 0.50);
}


/***************************************************************************

                                ROMs Loading

***************************************************************************/

/***************************************************************************

METAL CLASH[DATA EAST] JAPAN (c)1985
ROM Type:2764,27256

Name            Size    Location
--------------------------------
CS00.BIN    2764    C11 cpu
CS01.BIN    27256   C12 cpu
CS02.BIN    27256   C14 cpu
CS03.BIN    27256   C15 cpu
CS04.BIN    27256   C17 cpu
CS05.BIN    27256   H7  sound

CS06.BIN    27256   D9  Video
CS07.BIN    27256   D10 Video
CS08.BIN    27256   D12 Video

TTL-PROM 82S123(Color Table,8bit x 32Byte).
0000:3A 78 79 71 75 74 76 32
0008:3A 3D 29 21 25 14 16 12
0010:00 00 00 00 00 00 00 00
0018:00 00 00 00 00 00 00 00

This ROM work at DE-0212-1 & DE-0213-1

cpu   :6809(MAIN),6809(SOUND)
sound :YM2203,YM3526
custom:TC15G008AP,TC15G032CY(TOSHIBA)
color :82S123

DIP-SW
SW1
1 Coin CHARGE SELECT 1
2 Coin CHARGE SELECT 1
3 Coin CHARGE SELECT 2
4 Coin CHARGE SELECT 2
5 DON't CHANGE(for SERVICE ??)
6 ATTRACT SOUND
7 My ROBOT Infinity
8 My ROBOT LEFT not Decriment

SW2
1 My ROBOT LEFT 2/3
2 EMENY SPEED  EASY/DIFFICULT
3 EMENY ENERGY EASY/DIFFICULT
4 TIME LONG/DIFFICULT
5 SCREEN CHANGE NORMAL/FLIP
6 none ??
7 none ??
8 DON't CHANGE(for SERVICE ??)

"DARWIN 4078" use TC15G032CY too.

***************************************************************************/

ROM_START( metlclsh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cs04.bin",    0x00000, 0x8000, CRC(c2cc79a6) SHA1(0f586d4145afabbb45ea4865ed7a6590b14a2ab0) )
	ROM_LOAD( "cs00.bin",    0x0a000, 0x2000, CRC(af0f2998) SHA1(09dd2516406168660d5cd3a36be1e5f0adbcdb8a) )
	ROM_COPY( "maincpu", 0x7ff0, 0xfff0, 0x10 )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "cs03.bin",    0x00000, 0x8000, CRC(51c4720c) SHA1(7fd93bdcf029e7d2509b73b32f61fddf85f3453f) )
	ROM_COPY( "sub", 0x7ff0, 0xfff0, 0x10 )

	ROM_REGION( 0x18000, "sprites", 0 )
	ROM_LOAD( "cs06.bin",    0x00000, 0x8000, CRC(9f61403f) SHA1(0ebb1cb9d4983746b6b32ec948e7b9efd90783d1) )
	ROM_LOAD( "cs07.bin",    0x08000, 0x8000, CRC(d0610ea5) SHA1(3dfa16cbe93a4c08993111f78a8dd22c874fdd28) )
	ROM_LOAD( "cs08.bin",    0x10000, 0x8000, CRC(a8b02125) SHA1(145a22b2910b2fbfb28925f58968ee2bdeae1dda) )

	ROM_REGION( 0x10000, "bgtiles", 0 )
	ROM_LOAD( "cs01.bin",    0x00000, 0x1000, CRC(9c72343d) SHA1(c5618be7874ab6c930b0e68935c93f1958a1916d) )
	ROM_CONTINUE(            0x04000, 0x1000 )
	ROM_CONTINUE(            0x08000, 0x1000 )
	ROM_CONTINUE(            0x0c000, 0x1000 )
	ROM_CONTINUE(            0x01000, 0x1000 )
	ROM_CONTINUE(            0x05000, 0x1000 )
	ROM_CONTINUE(            0x09000, 0x1000 )
	ROM_CONTINUE(            0x0d000, 0x1000 )
	ROM_LOAD( "cs02.bin",    0x02000, 0x1000, CRC(3674673e) SHA1(8ba8864cefcb79afe5fe6821005a9d19742756e9) )
	ROM_CONTINUE(            0x06000, 0x1000 )
	ROM_CONTINUE(            0x0a000, 0x1000 )
	ROM_CONTINUE(            0x0e000, 0x1000 )
	ROM_CONTINUE(            0x03000, 0x1000 )
	ROM_CONTINUE(            0x07000, 0x1000 )
	ROM_CONTINUE(            0x0b000, 0x1000 )
	ROM_CONTINUE(            0x0f000, 0x1000 )

	ROM_REGION( 0x04000, "fgtiles", 0 )
	ROM_LOAD( "cs05.bin",    0x00000, 0x4000, CRC(f90c9c6b) SHA1(ca8e497e9c388078343dd1303beef6ee38748d6a) )
	ROM_CONTINUE(            0x00000, 0x4000 )  // first half is empty

	ROM_REGION( 0x020, "proms", 0 ) // ?
	ROM_LOAD( "82s123.prm",   0x0000, 0x20, CRC(6844cc88) SHA1(89d23367aa6ff541205416e82781fe938dfeeb52) )
ROM_END

} // anonymous namespace


GAME( 1985, metlclsh, 0, metlclsh, metlclsh, metlclsh_state, empty_init, ROT0, "Data East Corporation", "Metal Clash (Japan)", MACHINE_SUPPORTS_SAVE )
