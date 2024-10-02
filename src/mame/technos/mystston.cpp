// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Technos Mysterious Stones hardware

    driver by Nicola Salmoria

    Notes:
        * The subtitle of the two sets is slightly different:
          "Dr. John's Adventure" vs. "Dr. Kick in Adventure".
          The Dr John's is a bug fix. See the routine at 4376/4384 for example.
          The old set thrashes the Y register, the new one saves in on
          the stack. The newer set also resets the audio chips more often.

***************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "sound/ay8910.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class mystston_state : public driver_device
{
public:
	mystston_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ay8910(*this, "ay%u", 1U),
		m_ay8910_data(*this, "ay8910_data"),
		m_ay8910_select(*this, "ay8910_select"),
		m_dsw1(*this, "DSW1"),
		m_bg_videoram(*this, "bg_videoram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_scroll(*this, "scroll"),
		m_video_control(*this, "video_control"),
		m_color_prom(*this, "proms"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	void mystston(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void video_reset() override ATTR_COLD;

private:
	static constexpr XTAL MASTER_CLOCK = XTAL(12'000'000);

	// machine state
	required_device<cpu_device> m_maincpu;
	required_device_array<ay8910_device, 2> m_ay8910;
	required_shared_ptr<uint8_t> m_ay8910_data;
	required_shared_ptr<uint8_t> m_ay8910_select;
	required_ioport m_dsw1;

	// video state
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;
	emu_timer *m_interrupt_timer = nullptr;
	required_shared_ptr<uint8_t> m_bg_videoram;
	required_shared_ptr<uint8_t> m_fg_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_paletteram;
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_video_control;
	required_region_ptr<uint8_t> m_color_prom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	// HMC20
	// set_raw(MASTER_CLOCK / 2, 384, 0, 256, 272, 8, 248)
	static constexpr XTAL PIXEL_CLOCK    = (MASTER_CLOCK / 2);
	static constexpr int HTOTAL          = (384);
	static constexpr int HBEND           = (0);
	static constexpr int HBSTART         = (256);
	static constexpr int VTOTAL          = (272);  // counts from 0x08-0xff, then from 0xe8-0xff
	static constexpr int VBEND           = (8);
	static constexpr int VBSTART         = (248);
	static constexpr int FIRST_INT_VPOS  = (0x008);
	static constexpr int INT_HPOS        = (0x100);

	void irq_clear_w(uint8_t data);
	void ay8910_select_w(uint8_t data);
	void video_control_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(interrupt_callback);
	void set_palette();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx, int flip);
	void on_scanline_interrupt();
	void main_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

    There are only a few differences between the video hardware of Mysterious
    Stones and Mat Mania. The tile bank select bit is different and the sprite
    selection seems to be different as well. Additionally, the palette is stored
    differently. I'm also not sure that the 2nd tile page is really used in
    Mysterious Stones.

***************************************************************************/


/*************************************
 *
 *  Scanline interrupt system
 *
 *  There is an interrupt every 16
 *  scanlines, starting with 8.
 *
 *************************************/

TIMER_CALLBACK_MEMBER(mystston_state::interrupt_callback)
{
	int scanline = param;

	on_scanline_interrupt();

	scanline = scanline + 16;
	if (scanline >= VTOTAL)
		scanline = FIRST_INT_VPOS;

	// the vertical synch chain is clocked by H256 -- this is probably not important, but oh well
	m_interrupt_timer->adjust(m_screen->time_until_pos(scanline - 1, INT_HPOS), scanline);
}



/*************************************
 *
 *  Palette handling
 *
 *************************************/

void mystston_state::set_palette()
{
	static const int resistances_rg[3] = { 4700, 3300, 1500 };
	static const int resistances_b [2] = { 3300, 1500 };
	double weights_rg[3], weights_b[2];

	compute_resistor_weights(0, 255, -1.0,
			3, resistances_rg, weights_rg, 0, 4700,
			2, resistances_b,  weights_b,  0, 4700,
			0, nullptr, nullptr, 0, 0);

	for (int i = 0; i < 0x40; i++)
	{
		uint8_t data;

		// first half is dynamic, second half is from the PROM
		if (i & 0x20)
			data = m_color_prom[i & 0x1f];
		else
			data = m_paletteram[i];

		// red component
		int bit0 = (data >> 0) & 0x01;
		int bit1 = (data >> 1) & 0x01;
		int bit2 = (data >> 2) & 0x01;
		int r = combine_weights(weights_rg, bit0, bit1, bit2);

		// green component
		bit0 = (data >> 3) & 0x01;
		bit1 = (data >> 4) & 0x01;
		bit2 = (data >> 5) & 0x01;
		int g = combine_weights(weights_rg, bit0, bit1, bit2);

		// blue component
		bit0 = (data >> 6) & 0x01;
		bit1 = (data >> 7) & 0x01;
		int b = combine_weights(weights_b, bit0, bit1);

		m_palette->set_pen_color(i, rgb_t(r, g, b));
	}
}



/*************************************
 *
 *  Video control register
 *
 *************************************/

void mystston_state::video_control_w(uint8_t data)
{
	*m_video_control = data;

	// D0-D1 - foreground text color
	// D2 - background page select
	// D3 - unused

	// D4-D5 - coin counters in flipped order
	machine().bookkeeping().coin_counter_w(0, data & 0x20);
	machine().bookkeeping().coin_counter_w(1, data & 0x10);

	// D6 - unused
	// D7 - screen flip
}



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(mystston_state::get_bg_tile_info)
{
	int page = (*m_video_control & 0x04) << 8;
	int code = ((m_bg_videoram[page | 0x200 | tile_index] & 0x01) << 8) | m_bg_videoram[page | tile_index];
	int flags = (tile_index & 0x10) ? TILE_FLIPY : 0;

	tileinfo.set(1, code, 0, flags);
}


TILE_GET_INFO_MEMBER(mystston_state::get_fg_tile_info)
{
	int code = ((m_fg_videoram[0x400 | tile_index] & 0x07) << 8) | m_fg_videoram[tile_index];
	int color = ((*m_video_control & 0x01) << 1) | ((*m_video_control & 0x02) >> 1);

	tileinfo.set(0, code, color, 0);
}



/*************************************
 *
 *  Sprite drawing
 *
 *************************************/

void mystston_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx, int flip)
{
	for (int offs = 0; offs < 0x60; offs += 4)
	{
		int const attr = m_spriteram[offs];

		if (attr & 0x01)
		{
			int const code = ((attr & 0x10) << 4) | m_spriteram[offs + 1];
			int const color = (attr & 0x08) >> 3;
			int flipx = attr & 0x04;
			int flipy = attr & 0x02;
			int x = 240 - m_spriteram[offs + 3];
			int y = (240 - m_spriteram[offs + 2]) & 0xff;

			if (flip)
			{
				x = 240 - x;
				y = 240 - y;
				flipx = !flipx;
				flipy = !flipy;
			}

			gfx->transpen(bitmap, cliprect, code, color, flipx, flipy, x, y, 0);
		}
	}
}



/*************************************
 *
 *  Start
 *
 *************************************/

void mystston_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mystston_state::get_bg_tile_info)), TILEMAP_SCAN_COLS_FLIP_X, 16, 16, 16, 32);

	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mystston_state::get_fg_tile_info)), TILEMAP_SCAN_COLS_FLIP_X,  8,  8, 32, 32);
	m_fg_tilemap->set_transparent_pen(0);

	// create the interrupt timer
	m_interrupt_timer = timer_alloc(FUNC(mystston_state::interrupt_callback), this);
}



/*************************************
 *
 *  Reset
 *
 *************************************/

void mystston_state::video_reset()
{
	m_interrupt_timer->adjust(m_screen->time_until_pos(FIRST_INT_VPOS - 1, INT_HPOS), FIRST_INT_VPOS);
}



/*************************************
 *
 *  Update
 *
 *************************************/

uint32_t mystston_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int const flip = (*m_video_control & 0x80) ^ ((m_dsw1->read() & 0x20) << 2);

	set_palette();

	machine().tilemap().mark_all_dirty();
	m_bg_tilemap->set_scrolly(0, *m_scroll);
	machine().tilemap().set_flip_all(flip ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect, m_gfxdecode->gfx(2), flip);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}



/*************************************
 *
 *  Graphics decoding
 *
 *************************************/

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7,
		16*0+0, 16*0+1, 16*0+2, 16*0+3, 16*0+4, 16*0+5, 16*0+6, 16*0+7 },
	{ 0*8, 1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};


static GFXDECODE_START( gfx_mystston )
	GFXDECODE_ENTRY( "fgtiles_sprites", 0, gfx_8x8x3_planar, 4*8, 4 )
	GFXDECODE_ENTRY( "bgtiles",         0, spritelayout,     2*8, 1 )
	GFXDECODE_ENTRY( "fgtiles_sprites", 0, spritelayout,     0*8, 2 )
GFXDECODE_END


/*************************************
 *
 *  Interrupt system
 *
 *************************************/

void mystston_state::on_scanline_interrupt()
{
	m_maincpu->set_input_line(0, ASSERT_LINE);
}


void mystston_state::irq_clear_w(uint8_t data)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}



/*************************************
 *
 *  Coin handling
 *
 *************************************/

INPUT_CHANGED_MEMBER(mystston_state::coin_inserted)
{
	// coin insertion causes an NMI
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}



/*************************************
 *
 *  AY-8910 memory interface
 *
 *************************************/

void mystston_state::ay8910_select_w(uint8_t data)
{
	// bit 5 goes to 8910 #0 BDIR pin
	if (((*m_ay8910_select & 0x20) == 0x20) && ((data & 0x20) == 0x00))
	{
		// bit 4 goes to the 8910 #0 BC1 pin
		m_ay8910[0]->data_address_w(*m_ay8910_select >> 4, *m_ay8910_data);
	}

	// bit 7 goes to 8910 #1 BDIR pin
	if (((*m_ay8910_select & 0x80) == 0x80) && ((data & 0x80) == 0x00))
	{
		// bit 6 goes to the 8910 #1 BC1 pin
		m_ay8910[1]->data_address_w(*m_ay8910_select >> 6, *m_ay8910_data);
	}

	*m_ay8910_select = data;
}



/*************************************
 *
 *  Memory map
 *
 *************************************/

void mystston_state::main_map(address_map &map)
{
	map(0x0000, 0x077f).ram();
	map(0x0780, 0x07df).ram().share(m_spriteram);
	map(0x07e0, 0x0fff).ram();
	map(0x1000, 0x17ff).ram().share(m_fg_videoram);
	map(0x1800, 0x1fff).ram().share(m_bg_videoram);
	map(0x2000, 0x2000).mirror(0x1f8f).portr("IN0").w(FUNC(mystston_state::video_control_w)).share(m_video_control);
	map(0x2010, 0x2010).mirror(0x1f8f).portr("IN1").w(FUNC(mystston_state::irq_clear_w));
	map(0x2020, 0x2020).mirror(0x1f8f).portr("DSW0").writeonly().share(m_scroll);
	map(0x2030, 0x2030).mirror(0x1f8f).portr("DSW1").writeonly().share(m_ay8910_data);
	map(0x2040, 0x2040).mirror(0x1f8f).nopr().w(FUNC(mystston_state::ay8910_select_w)).share(m_ay8910_select);
	map(0x2050, 0x2050).mirror(0x1f8f).noprw();
	map(0x2060, 0x207f).mirror(0x1f80).ram().share(m_paletteram);
	map(0x4000, 0xffff).rom();
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( mystston )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, mystston_state, coin_inserted, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, mystston_state, coin_inserted, 0)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW0")
	PORT_DIPNAME(0x01, 0x01, DEF_STR( Lives ) )     PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(   0x01, "3" )
	PORT_DIPSETTING(   0x00, "5" )
	PORT_DIPNAME(0x02, 0x02, DEF_STR( Difficulty ) )    PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(   0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(   0x00, DEF_STR( Hard ) )
	PORT_DIPNAME(0x04, 0x00, DEF_STR( Demo_Sounds ) )   PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(   0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )        // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )        // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )        // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )        // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        // Listed as "Unused"

	PORT_START("DSW1")
	PORT_DIPNAME(0x03, 0x03, DEF_STR( Coin_A ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(   0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(   0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(   0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(   0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME(0x0c, 0x0c, DEF_STR( Coin_B ) )        PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(   0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(   0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(   0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(   0x04, DEF_STR( 1C_3C ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW1:5" )        // Listed as "Unused"
	PORT_DIPNAME(0x20, 0x00, DEF_STR( Flip_Screen ) )   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x20, DEF_STR( On ) )
	PORT_DIPNAME(0x40, 0x00, DEF_STR( Cabinet ) )       PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(   0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(   0x40, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END

static INPUT_PORTS_START( myststonoi )
	PORT_INCLUDE(mystston)

	PORT_MODIFY("DSW0")
	PORT_DIPNAME(0x01, 0x01, DEF_STR( Lives ) )     PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(   0x01, "2" )
	PORT_DIPSETTING(   0x00, "3" )
INPUT_PORTS_END

/*************************************
 *
 *  Machine driver
 *
 *************************************/

void mystston_state::mystston(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, MASTER_CLOCK / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &mystston_state::main_map);

	// video hardware
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mystston);
	PALETTE(config, m_palette).set_entries(0x40);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	m_screen->set_screen_update(FUNC(mystston_state::screen_update));
	m_screen->set_palette(m_palette);

	// audio hardware
	SPEAKER(config, "mono").front_center();

	AY8910(config, m_ay8910[0], MASTER_CLOCK / 8).add_route(ALL_OUTPUTS, "mono", 0.30);

	AY8910(config, m_ay8910[1], MASTER_CLOCK / 8).add_route(ALL_OUTPUTS, "mono", 0.30);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( mystston )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom6.bin",     0x4000, 0x2000, CRC(7bd9c6cd) SHA1(4d14edc783ba1a6c01d2fb9ea29ec85b8fec3c3b) )
	ROM_LOAD( "rom5.bin",     0x6000, 0x2000, CRC(a83f04a6) SHA1(d8cdf310511c1fef4fbde80ef2161fda00f965d7) )
	ROM_LOAD( "rom4.bin",     0x8000, 0x2000, CRC(46c73714) SHA1(5b9ac3a35aeeea6a0cd2d838c144925d83b36a7f) )
	ROM_LOAD( "rom3.bin",     0xa000, 0x2000, CRC(34f8b8a3) SHA1(a270f6665a9f76f97ac02201d51fe2817e6e8f22) )
	ROM_LOAD( "rom2.bin",     0xc000, 0x2000, CRC(bfd22cfc) SHA1(137cd61c8b1e997e7e50edd57f1671031d8e3ac5) )
	ROM_LOAD( "rom1.bin",     0xe000, 0x2000, CRC(fb163e38) SHA1(d6f02e90bfd9badd7751bc0a87fdfdd1d0a7e202) )

	ROM_REGION( 0x0c000, "fgtiles_sprites", 0 )
	ROM_LOAD( "ms6",          0x00000, 0x2000, CRC(85c83806) SHA1(cdfed6c224754e8f79b154533b06b7de4a44b4d3) )
	ROM_LOAD( "ms9",          0x02000, 0x2000, CRC(b146c6ab) SHA1(712c0c17780f222be5c8b09185a22e900ab23944) )
	ROM_LOAD( "ms7",          0x04000, 0x2000, CRC(d025f84d) SHA1(eaaaa0bde3db850098d04a0af85993026e503fc5) )
	ROM_LOAD( "ms10",         0x06000, 0x2000, CRC(d85015b5) SHA1(f4afab248dfde354650e59fadd5ab9616b04dac1) )
	ROM_LOAD( "ms8",          0x08000, 0x2000, CRC(53765d89) SHA1(c8bfc311123b076dccae9f7e3b95460bf9fc843d) )
	ROM_LOAD( "ms11",         0x0a000, 0x2000, CRC(919ee527) SHA1(609ee854ab3a4fdbf3404a68a4a657b85250f742) )

	ROM_REGION( 0x0c000, "bgtiles", 0 )
	ROM_LOAD( "ms12",         0x00000, 0x2000, CRC(72d8331d) SHA1(f0a3bc6c9d9966f169f4721c2453f7ee210f0feb) )
	ROM_LOAD( "ms13",         0x02000, 0x2000, CRC(845a1f9b) SHA1(aa2eabd2a5e89e150b5d2fb3d88f91902e5ebb48) )
	ROM_LOAD( "ms14",         0x04000, 0x2000, CRC(822874b0) SHA1(9376d48045bf67df91d103effd1d08bd8debad26) )
	ROM_LOAD( "ms15",         0x06000, 0x2000, CRC(4594e53c) SHA1(a011a5269a9b0ca7a964181efe8413d5637c34f4) )
	ROM_LOAD( "ms16",         0x08000, 0x2000, CRC(2f470b0f) SHA1(79b50a7d113fed4669361c5f6c60ec96c94344c6) )
	ROM_LOAD( "ms17",         0x0a000, 0x2000, CRC(38966d1b) SHA1(89e3e54d3298cefeb35922d2292e3e7b8e995871) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "ic61",         0x0000, 0x0020, CRC(e802d6cf) SHA1(233ceb9e3a91939e1925766a696bc65ab0dffa50) )
ROM_END

/*
actual label format is:
--------------
|       T.M. |
| MYSTERIOUS |
|   STONES   |
|   BB00-    |
|  @ 1984    |
| DATA EAST  |
--------------
*/
ROM_START( myststono ) // TA-0010-P1-1 + TA-0010-P2-1 PCBs
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bb00.ic102", 0x4000, 0x2000, CRC(6dacc05f) SHA1(43054199901639516205c7ea145462d0abea8fb1) )
	ROM_LOAD( "bw01.ic90",  0x6000, 0x2000, CRC(a3546df7) SHA1(89c0349885a9369406a1121cd3db28963b25f2e6) )
	ROM_LOAD( "bw02.ic76",  0x8000, 0x2000, CRC(43bc6182) SHA1(dc36c10eee20009922e89d9bfdf6c2f6ffb881ce) )
	ROM_LOAD( "bw03.ic75",  0xa000, 0x2000, CRC(9322222b) SHA1(25192ac9e8e66cd2bc21c66c690c57c6b9836f2d) )
	ROM_LOAD( "bw04.ic62",  0xc000, 0x2000, CRC(47cefe9b) SHA1(49422b664b1322373a9cd3cb2907f8f5492faf87) )
	ROM_LOAD( "bw05.ic61",  0xe000, 0x2000, CRC(b37ae12b) SHA1(55ee1193088145c85adddd377d9e5ee58aca922f) )

	ROM_REGION( 0xc000, "fgtiles_sprites", 0 )
	ROM_LOAD( "bw06.ic105", 0x00000, 0x2000, CRC(85c83806) SHA1(cdfed6c224754e8f79b154533b06b7de4a44b4d3) )
	ROM_LOAD( "bw09.ic93",  0x02000, 0x2000, CRC(b146c6ab) SHA1(712c0c17780f222be5c8b09185a22e900ab23944) )
	ROM_LOAD( "bw07.ic107", 0x04000, 0x2000, CRC(d025f84d) SHA1(eaaaa0bde3db850098d04a0af85993026e503fc5) )
	ROM_LOAD( "bw10.ic95",  0x06000, 0x2000, CRC(d85015b5) SHA1(f4afab248dfde354650e59fadd5ab9616b04dac1) )
	ROM_LOAD( "bw08.ic109", 0x08000, 0x2000, CRC(53765d89) SHA1(c8bfc311123b076dccae9f7e3b95460bf9fc843d) )
	ROM_LOAD( "bw11.ic97",  0x0a000, 0x2000, CRC(919ee527) SHA1(609ee854ab3a4fdbf3404a68a4a657b85250f742) )

	ROM_REGION( 0x0c000, "bgtiles", 0 )
	ROM_LOAD( "bw12.ic15", 0x00000, 0x2000, CRC(72d8331d) SHA1(f0a3bc6c9d9966f169f4721c2453f7ee210f0feb) )
	ROM_LOAD( "bw13.ic20", 0x02000, 0x2000, CRC(845a1f9b) SHA1(aa2eabd2a5e89e150b5d2fb3d88f91902e5ebb48) )
	ROM_LOAD( "bw14.ic24", 0x04000, 0x2000, CRC(822874b0) SHA1(9376d48045bf67df91d103effd1d08bd8debad26) )
	ROM_LOAD( "bw15.ic29", 0x06000, 0x2000, CRC(4594e53c) SHA1(a011a5269a9b0ca7a964181efe8413d5637c34f4) )
	ROM_LOAD( "bw16.ic34", 0x08000, 0x2000, CRC(2f470b0f) SHA1(79b50a7d113fed4669361c5f6c60ec96c94344c6) )
	ROM_LOAD( "bw17.ic38", 0x0a000, 0x2000, CRC(38966d1b) SHA1(89e3e54d3298cefeb35922d2292e3e7b8e995871) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "hlo.ic61", 0x0000, 0x0020, CRC(e802d6cf) SHA1(233ceb9e3a91939e1925766a696bc65ab0dffa50) )
ROM_END

// looks like Itisa made a (very) minor mod to the ROMs when producing the PCBs
ROM_START( myststonoi )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "14.bin",          0x4000, 0x2000, CRC(78bf2a58) SHA1(92e61041acad3293a103d957507b091321eed8f1) ) // 2 bytes changed in here (values for the lives' dip-switch)
	ROM_LOAD( "13.bin",          0x6000, 0x2000, CRC(a3546df7) SHA1(89c0349885a9369406a1121cd3db28963b25f2e6) )
	ROM_LOAD( "12.bin",          0x8000, 0x2000, CRC(43bc6182) SHA1(dc36c10eee20009922e89d9bfdf6c2f6ffb881ce) )
	ROM_LOAD( "11.bin",          0xa000, 0x2000, CRC(9322222b) SHA1(25192ac9e8e66cd2bc21c66c690c57c6b9836f2d) )
	ROM_LOAD( "8.bin",           0xc000, 0x2000, CRC(47cefe9b) SHA1(49422b664b1322373a9cd3cb2907f8f5492faf87) )
	ROM_LOAD( "7.bin",           0xe000, 0x2000, CRC(b37ae12b) SHA1(55ee1193088145c85adddd377d9e5ee58aca922f) )

	ROM_REGION( 0x0c000, "fgtiles_sprites", 0 )
	ROM_LOAD( "18.bin",          0x00000, 0x2000, CRC(85c83806) SHA1(cdfed6c224754e8f79b154533b06b7de4a44b4d3) )
	ROM_LOAD( "15.bin",          0x02000, 0x2000, CRC(b146c6ab) SHA1(712c0c17780f222be5c8b09185a22e900ab23944) )
	ROM_LOAD( "19.bin",          0x04000, 0x2000, CRC(d025f84d) SHA1(eaaaa0bde3db850098d04a0af85993026e503fc5) )
	ROM_LOAD( "16.bin",          0x06000, 0x2000, CRC(d85015b5) SHA1(f4afab248dfde354650e59fadd5ab9616b04dac1) )
	ROM_LOAD( "20.bin",          0x08000, 0x2000, CRC(53765d89) SHA1(c8bfc311123b076dccae9f7e3b95460bf9fc843d) )
	ROM_LOAD( "17.bin",          0x0a000, 0x2000, CRC(919ee527) SHA1(609ee854ab3a4fdbf3404a68a4a657b85250f742) )

	ROM_REGION( 0x0c000, "bgtiles", 0 )
	ROM_LOAD( "1.bin",         0x00000, 0x2000, CRC(72d8331d) SHA1(f0a3bc6c9d9966f169f4721c2453f7ee210f0feb) )
	ROM_LOAD( "2.bin",         0x02000, 0x2000, CRC(845a1f9b) SHA1(aa2eabd2a5e89e150b5d2fb3d88f91902e5ebb48) )
	ROM_LOAD( "3.bin",         0x04000, 0x2000, CRC(822874b0) SHA1(9376d48045bf67df91d103effd1d08bd8debad26) )
	ROM_LOAD( "4.bin",         0x06000, 0x2000, CRC(4594e53c) SHA1(a011a5269a9b0ca7a964181efe8413d5637c34f4) )
	ROM_LOAD( "5.bin",         0x08000, 0x2000, CRC(2f470b0f) SHA1(79b50a7d113fed4669361c5f6c60ec96c94344c6) )
	ROM_LOAD( "6.bin",         0x0a000, 0x2000, CRC(38966d1b) SHA1(89e3e54d3298cefeb35922d2292e3e7b8e995871) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.bin",         0x0000, 0x0020, CRC(e802d6cf) SHA1(233ceb9e3a91939e1925766a696bc65ab0dffa50) )

	ROM_REGION( 0x0104, "pals", 0 ) // not verified if these were protected
	ROM_LOAD( "pal10l8.bin",         0x0000, 0x002c, CRC(2d4d034c) SHA1(1d43f21d4522c71da507c1ded1b2d3bfa0fe043b) )
	ROM_LOAD( "pal16r4-1.bin",       0x0000, 0x0104, CRC(c57555d0) SHA1(c1cda869de8457b9f8ca4f41f0ed49916110ff2e) )
	ROM_LOAD( "pal16r4-2.bin",       0x0000, 0x0104, CRC(c57555d0) SHA1(c1cda869de8457b9f8ca4f41f0ed49916110ff2e) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1984, mystston,   0,        mystston, mystston,   mystston_state, empty_init, ROT270, "Technos Japan", "Mysterious Stones - Dr. John's Adventure",              MACHINE_SUPPORTS_SAVE )
GAME( 1984, myststono,  mystston, mystston, mystston,   mystston_state, empty_init, ROT270, "Technos Japan", "Mysterious Stones - Dr. Kick in Adventure",             MACHINE_SUPPORTS_SAVE )
GAME( 1984, myststonoi, mystston, mystston, myststonoi, mystston_state, empty_init, ROT270, "Technos Japan", "Mysterious Stones - Dr. Kick in Adventure (Itisa PCB)", MACHINE_SUPPORTS_SAVE )
