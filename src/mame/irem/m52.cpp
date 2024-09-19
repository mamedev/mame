// license:BSD-3-Clause
// copyright-holders: Nicola Salmoria

/***************************************************************************

    Irem M52 hardware

****************************************************************************

    Moon Patrol memory map

    driver by Nicola Salmoria

    0000-3fff ROM
    8000-83ff Video RAM
    8400-87ff Color RAM
    e000-e7ff RAM


    read:
    8800      protection
    d000      IN0
    d001      IN1
    d002      IN2
    d003      DSW1
    d004      DSW2

    write:
    c800-c8ff sprites
    d000      sound command
    d001      flip screen

    I/O ports
    write:
    10-1f     scroll registers
    40        background #1 x position
    60        background #1 y position
    80        background #2 x position
    a0        background #2 y position
    c0        background control

*****************************************************************************

    Locations based on m58.cpp driver

***************************************************************************/

#include "emu.h"

#include "irem.h"
#include "iremipt.h"

#include "cpu/z80/z80.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


namespace {

class m52_state : public driver_device
{
public:
	m52_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_sp_gfxdecode(*this, "sp_gfxdecode"),
		m_tx_gfxdecode(*this, "tx_gfxdecode"),
		m_bg_gfxdecode(*this, "bg_gfxdecode"),
		m_sp_palette(*this, "sp_palette"),
		m_tx_palette(*this, "tx_palette"),
		m_bg_palette(*this, "bg_palette"),
		m_dsw2(*this, "DSW2")
	{ }

	void m52(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	virtual void scroll_w(uint8_t data);

	// board mod changes?
	int m_spritelimit = 0;
	bool m_do_bg_fills = false;

	tilemap_t *m_tx_tilemap = nullptr;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;

	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	uint8_t protection_r();

private:
	required_device<gfxdecode_device> m_sp_gfxdecode;
	required_device<gfxdecode_device> m_tx_gfxdecode;
	required_device<gfxdecode_device> m_bg_gfxdecode;
	required_device<palette_device> m_sp_palette;
	required_device<palette_device> m_tx_palette;
	required_device<palette_device> m_bg_palette;

	required_ioport m_dsw2;

	// video-related
	uint8_t m_bgxpos[2]{};
	uint8_t m_bgypos[2]{};
	uint8_t m_bgcontrol = 0U;

	template <uint8_t Which> void bgypos_w(uint8_t data);
	template <uint8_t Which> void bgxpos_w(uint8_t data);
	void bgcontrol_w(uint8_t data);
	void flipscreen_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	void init_palette();
	template <size_t N, size_t O, size_t P>
	void init_sprite_palette(const int *resistances_3, const int *resistances_2, double (&weights_r)[N], double (&weights_g)[O], double (&weights_b)[P], double scale);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect, int xpos, int ypos, int image);
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, int initoffs);

	void main_map(address_map &map) ATTR_COLD;
	void main_portmap(address_map &map) ATTR_COLD;
};

class alpha1v_state : public m52_state
{
public:
	alpha1v_state(const machine_config &mconfig, device_type type, const char *tag)
		: m52_state(mconfig, type, tag)
	{ }

	void alpha1v(machine_config &config);

	void main_map(address_map &map) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void scroll_w(uint8_t data) override;

private:
	void flipscreen_w(uint8_t data);

};


/*************************************
 *
 *  Palette configuration
 *
 *************************************/

void m52_state::init_palette()
{
	constexpr int resistances_3[3] = { 1000, 470, 220 };
	constexpr int resistances_2[2] = { 470, 220 };
	double weights_r[3], weights_g[3], weights_b[3], scale;

	// compute palette information for characters/backgrounds
	scale = compute_resistor_weights(0, 255, -1.0,
			3, resistances_3, weights_r, 0, 0,
			3, resistances_3, weights_g, 0, 0,
			2, resistances_2, weights_b, 0, 0);

	// character palette
	const uint8_t *char_pal = memregion("tx_pal")->base();
	for (int i = 0; i < 512; i++)
	{
		uint8_t const promval = char_pal[i];
		int const r = combine_weights(weights_r, BIT(promval, 0), BIT(promval, 1), BIT(promval, 2));
		int const g = combine_weights(weights_g, BIT(promval, 3), BIT(promval, 4), BIT(promval, 5));
		int const b = combine_weights(weights_b, BIT(promval, 6), BIT(promval, 7));

		m_tx_palette->set_pen_color(i, rgb_t(r, g, b));
	}

	// background palette
	const uint8_t *back_pal = memregion("bg_pal")->base();
	for (int i = 0; i < 32; i++)
	{
		uint8_t promval = back_pal[i];
		int r = combine_weights(weights_r, BIT(promval, 0), BIT(promval, 1), BIT(promval, 2));
		int g = combine_weights(weights_g, BIT(promval, 3), BIT(promval, 4), BIT(promval, 5));
		int b = combine_weights(weights_b, BIT(promval, 6), BIT(promval, 7));

		m_bg_palette->set_indirect_color(i, rgb_t(r, g, b));
	}

	/* background
	 the palette is a 32x8 PROM with many colors repeated. The address of
	 the colors to pick is as follows:
	 xbb00: mountains
	 0xxbb: hills
	 1xxbb: city

	 this seems hacky, surely all bytes in the PROM should be used, not just picking the ones that give the colours we want?

	 */
	m_bg_palette->set_pen_indirect(0 * 4 + 0, 0);
	m_bg_palette->set_pen_indirect(0 * 4 + 1, 4);
	m_bg_palette->set_pen_indirect(0 * 4 + 2, 8);
	m_bg_palette->set_pen_indirect(0 * 4 + 3, 12);
	m_bg_palette->set_pen_indirect(1 * 4 + 0, 0);
	m_bg_palette->set_pen_indirect(1 * 4 + 1, 1);
	m_bg_palette->set_pen_indirect(1 * 4 + 2, 2);
	m_bg_palette->set_pen_indirect(1 * 4 + 3, 3);
	m_bg_palette->set_pen_indirect(2 * 4 + 0, 0);
	m_bg_palette->set_pen_indirect(2 * 4 + 1, 16 + 1);
	m_bg_palette->set_pen_indirect(2 * 4 + 2, 16 + 2);
	m_bg_palette->set_pen_indirect(2 * 4 + 3, 16 + 3);

	init_sprite_palette(resistances_3, resistances_2, weights_r, weights_g, weights_b, scale);
}

template <size_t N, size_t O, size_t P>
void m52_state::init_sprite_palette(const int *resistances_3, const int *resistances_2, double (&weights_r)[N], double (&weights_g)[O], double (&weights_b)[P], double scale)
{
	const uint8_t *sprite_pal = memregion("spr_pal")->base();
	const uint8_t *sprite_table = memregion("spr_clut")->base();

	// compute palette information for sprites
	compute_resistor_weights(0, 255, scale,
			2, resistances_2, weights_r, 470, 0,
			3, resistances_3, weights_g, 470, 0,
			3, resistances_3, weights_b, 470, 0);

	// sprite palette
	for (int i = 0; i < 32; i++)
	{
		uint8_t const promval = sprite_pal[i];
		int const r = combine_weights(weights_r, BIT(promval, 6), BIT(promval, 7));
		int const g = combine_weights(weights_g, BIT(promval, 3), BIT(promval, 4), BIT(promval, 5));
		int const b = combine_weights(weights_b, BIT(promval, 0), BIT(promval, 1), BIT(promval, 2));

		m_sp_palette->set_indirect_color(i, rgb_t(r, g, b));
	}

	// sprite lookup table
	for (int i = 0; i < 256; i++)
	{
		uint8_t promval = sprite_table[i];
		m_sp_palette->set_pen_indirect(i, promval);
	}
}


/*************************************
 *
 *  Tilemap info callback
 *
 *************************************/

TILE_GET_INFO_MEMBER(m52_state::get_tile_info)
{
	uint8_t video = m_videoram[tile_index];
	uint8_t const color = m_colorram[tile_index];

	int flag = 0;
	int code = 0;

	code = video;

	if (color & 0x80)
	{
		code |= 0x100;
	}

	if (tile_index / 32 <= 6)
	{
		flag |= TILE_FORCE_LAYER0; // lines 0 to 6 are opaqe?
	}

	tileinfo.set(0, code, color & 0x7f, flag);
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

void m52_state::video_start()
{
	m_tx_tilemap = &machine().tilemap().create(*m_tx_gfxdecode, tilemap_get_info_delegate(*this, FUNC(m52_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_tx_tilemap->set_transparent_pen(0);
	m_tx_tilemap->set_scrolldx(127, 127);
	m_tx_tilemap->set_scrolldy(16, 16);
	m_tx_tilemap->set_scroll_rows(4); // only lines 192-256 scroll

	init_palette();

	save_item(NAME(m_bgxpos));
	save_item(NAME(m_bgypos));
	save_item(NAME(m_bgcontrol));

	m_spritelimit = 0x100 - 4;
	m_do_bg_fills = true;
}

void alpha1v_state::video_start()
{
	m52_state::video_start();

	// is the limit really just higher anyway or is this a board mod?
	m_spritelimit = 0x200 - 4;
	m_do_bg_fills = false; // or you get solid green areas below the stars bg image.  does the doubled up tilemap ROM maybe mean double height instead?

	// the scrolling orange powerups 'orbs' are a single tile in the tilemap, your ship is huge, it is unclear where the hitboxes are meant to be
	// using the same value as mpatrol puts the collision at the very back of your ship
	// maybe the sprite positioning is incorrect instead or this is just how the game is designed
	//m_tx_tilemap->set_scrolldx(127 + 8, 127 - 8);
}



/*************************************
 *
 *  Scroll registers
 *
 *************************************/

void m52_state::scroll_w(uint8_t data)
{
/*
    According to the schematics there is only one video register that holds the X scroll value
    with a NAND gate on the V64 and V128 lines to control when it's read, and when
    255 (via 8 pull up resistors) is used.

    So we set the first 3 quarters to 255 and the last to the scroll value
*/
	m_tx_tilemap->set_scrollx(0, 255);
	m_tx_tilemap->set_scrollx(1, 255);
	m_tx_tilemap->set_scrollx(2, 255);
	m_tx_tilemap->set_scrollx(3, -(data + 1));
}

void alpha1v_state::scroll_w(uint8_t data)
{
/*
   alpha1v must have some board mod to invert scroll register use, as it expects only the first block to remain static
   the scrolling powerups are part of the tx layer!

   TODO: check if this configuration works with Moon Patrol, maybe the schematics were read incorrectly?
*/
	m_tx_tilemap->set_scrollx(0, 255);
	m_tx_tilemap->set_scrollx(1, -(data + 1));
	m_tx_tilemap->set_scrollx(2, -(data + 1));
	m_tx_tilemap->set_scrollx(3, -(data + 1));
}


/*************************************
 *
 *  Video RAM write handlers
 *
 *************************************/

void m52_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset);
}


void m52_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset);
}



/*************************************
 *
 *  Custom protection
 *
 *************************************/

/* This looks like some kind of protection implemented by a custom chip on the
   scroll board. It mangles the value written to the port bg1xpos_w, as
   follows: result = popcount(value & 0x7f) ^ (value >> 7) */
uint8_t m52_state::protection_r()
{
	int popcount = 0;

	for (int temp = m_bgxpos[0] & 0x7f; temp != 0; temp >>= 1)
		popcount += temp & 1;
	return popcount ^ (m_bgxpos[0] >> 7);
}



/*************************************
 *
 *  Background control write handlers
 *
 *************************************/

template <uint8_t Which>
void m52_state::bgypos_w(uint8_t data)
{
	m_bgypos[Which] = data;
}

template <uint8_t Which>
void m52_state::bgxpos_w(uint8_t data)
{
	m_bgxpos[Which] = data;
}

void m52_state::bgcontrol_w(uint8_t data)
{
	m_bgcontrol = data;
}



/*************************************
 *
 *  Outputs
 *
 *************************************/

void m52_state::flipscreen_w(uint8_t data)
{
	// screen flip is handled both by software and hardware
	flip_screen_set((data & 0x01) ^ (~m_dsw2->read() & 0x01));

	machine().bookkeeping().coin_counter_w(0, data & 0x02);
	machine().bookkeeping().coin_counter_w(1, data & 0x20);
}

void alpha1v_state::flipscreen_w(uint8_t data)
{
	flip_screen_set(data & 0x01);
}



/*************************************
 *
 *  Background rendering
 *
 *************************************/

void m52_state::draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect, int xpos, int ypos, int image)
{
	rectangle rect;
	const rectangle &visarea = m_screen->visible_area();
	const pen_t *paldata = m_bg_palette->pens();
	constexpr uint8_t BGHEIGHT = 128;


	if (flip_screen())
	{
		xpos = 264 - xpos;
		ypos = 264 - ypos - BGHEIGHT;
	}

	xpos += 124;

	// this may not be correct
	ypos += 16;


	m_bg_gfxdecode->gfx(image)->transpen(bitmap, cliprect,
		0, 0,
		flip_screen(),
		flip_screen(),
		xpos,
		ypos, 0);


	m_bg_gfxdecode->gfx(image)->transpen(bitmap, cliprect,
		0, 0,
		flip_screen(),
		flip_screen(),
		xpos - 256,
		ypos, 0);

	// create a solid fill below the 64 pixel high bg images
	if (m_do_bg_fills)
	{
		rect.min_x = visarea.min_x;
		rect.max_x = visarea.max_x;

		if (flip_screen())
		{
			rect.min_y = ypos - BGHEIGHT;
			rect.max_y = ypos - 1;
		}
		else
		{
			rect.min_y = ypos + BGHEIGHT;
			rect.max_y = ypos + 2 * BGHEIGHT - 1;
		}

		bitmap.fill(paldata[m_bg_gfxdecode->gfx(image)->colorbase() + 3], rect);
	}
}



/*************************************
 *
 *  Sprites rendering
 *
 *************************************/

void m52_state::draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, int initoffs)
{
	// draw the sprites
	for (int offs = initoffs; offs >= (initoffs & 0xc0); offs -= 4)
	{
		int sy = 257 - m_spriteram[offs];
		int const color = m_spriteram[offs + 1] & 0x3f;
		int flipx = m_spriteram[offs + 1] & 0x40;
		int flipy = m_spriteram[offs + 1] & 0x80;
		int const code = m_spriteram[offs + 2];
		int sx = m_spriteram[offs + 3];

		// sprites from offsets $00-$7F are processed in the upper half of the frame
		// sprites from offsets $80-$FF are processed in the lower half of the frame
		rectangle clip = cliprect;
		if (!(offs & 0x80))
			clip.min_y = 0, clip.max_y = 127;
		else
			clip.min_y = 128, clip.max_y = 255;

		// adjust for flipping
		if (flip_screen())
		{
			int temp = clip.min_y;
			clip.min_y = 255 - clip.max_y;
			clip.max_y = 255 - temp;
			flipx = !flipx;
			flipy = !flipy;
			sx = 238 - sx;
			sy = 282 - sy;
		}

		sx += 129;

		// in theory anyways; in practice, some of the molecule-looking guys get clipped
#ifdef SPLIT_SPRITES
		sect_rect(&clip, cliprect);
#else
		clip = cliprect;
#endif

		m_sp_gfxdecode->gfx(0)->transmask(bitmap, clip,
			code, color, flipx, flipy, sx, sy,
			m_sp_palette->transpen_mask(*m_sp_gfxdecode->gfx(0), color,  0));
	}
}



/*************************************
 *
 *  Video render
 *
 *************************************/

uint32_t m52_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *paldata = m_sp_palette->pens();

	bitmap.fill(paldata[0], cliprect);

	if (!(m_bgcontrol & 0x20))
	{
		if (!(m_bgcontrol & 0x10))
			draw_background(bitmap, cliprect, m_bgxpos[1], m_bgypos[1], 0); // distant mountains

		// only one of these be drawn at once (they share the same scroll register) (alpha1v leaves everything enabled)
		if (!(m_bgcontrol & 0x02))
			draw_background(bitmap, cliprect, m_bgxpos[0], m_bgypos[0], 1); // hills
		else if (!(m_bgcontrol & 0x04))
			draw_background(bitmap, cliprect, m_bgxpos[0], m_bgypos[0], 2); // cityscape
	}

	m_tx_tilemap->set_flip(flip_screen() ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);

	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// draw the sprites
	for (int offs = 0x3c; offs <= m_spritelimit; offs += 0x40)
		draw_sprites(bitmap, cliprect, offs);

	return 0;
}



/*************************************
 *
 *  Memory maps
 *
 *************************************/

void m52_state::main_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x8000, 0x83ff).ram().w(FUNC(m52_state::videoram_w)).share(m_videoram);
	map(0x8400, 0x87ff).ram().w(FUNC(m52_state::colorram_w)).share(m_colorram);
	map(0x8800, 0x8800).mirror(0x07ff).r(FUNC(m52_state::protection_r));
	map(0xc800, 0xcbff).mirror(0x0400).writeonly().share(m_spriteram); // only 0x100 of this used by video code?
	map(0xd000, 0xd000).mirror(0x07fc).w("irem_audio", FUNC(irem_audio_device::cmd_w));
	map(0xd001, 0xd001).mirror(0x07fc).w(FUNC(m52_state::flipscreen_w));   // + coin counters
	map(0xd000, 0xd000).mirror(0x07f8).portr("IN0");
	map(0xd001, 0xd001).mirror(0x07f8).portr("IN1");
	map(0xd002, 0xd002).mirror(0x07f8).portr("IN2");
	map(0xd003, 0xd003).mirror(0x07f8).portr("DSW1");
	map(0xd004, 0xd004).mirror(0x07f8).portr("DSW2");
	map(0xe000, 0xe7ff).ram();
}


void alpha1v_state::main_map(address_map &map)
{
	map(0x0000, 0x6fff).rom();
	map(0x8000, 0x83ff).ram().w(FUNC(alpha1v_state::videoram_w)).share(m_videoram);
	map(0x8400, 0x87ff).ram().w(FUNC(alpha1v_state::colorram_w)).share(m_colorram);
	map(0x8800, 0x8800).mirror(0x07ff).r(FUNC(alpha1v_state::protection_r)); // result is ignored
	map(0xc800, 0xc9ff).writeonly().share(m_spriteram);
	map(0xd000, 0xd000).portr("IN0").w("irem_audio", FUNC(irem_audio_device::cmd_w));
	map(0xd001, 0xd001).portr("IN1").w(FUNC(alpha1v_state::flipscreen_w));
	map(0xd002, 0xd002).portr("IN2");
	map(0xd003, 0xd003).portr("DSW1");
	map(0xd004, 0xd004).portr("DSW2");
	map(0xe000, 0xefff).ram(); // bigger or mirrored?
}


void m52_state::main_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).mirror(0x1f).w(FUNC(m52_state::scroll_w));
	map(0x40, 0x40).mirror(0x1f).w(FUNC(m52_state::bgxpos_w<0>));
	map(0x60, 0x60).mirror(0x1f).w(FUNC(m52_state::bgypos_w<0>));
	map(0x80, 0x80).mirror(0x1f).w(FUNC(m52_state::bgxpos_w<1>));
	map(0xa0, 0xa0).mirror(0x1f).w(FUNC(m52_state::bgypos_w<1>));
	map(0xc0, 0xc0).mirror(0x1f).w(FUNC(m52_state::bgcontrol_w));
}


/*************************************
 *
 *  Port definitions
 *
 *************************************/

// Same as m57, m58 and m62 (IREM Z80 hardware)
static INPUT_PORTS_START( m52 )
	PORT_START("IN0")
	/* Start 1 & 2 also restarts and freezes the game with stop mode on
	   and are used in test mode to enter and exit the various tests */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	// coin input must be active for 19 frames to be consistently recognized
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_IMPULSE(19)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	// DSW1 is so different from game to game that it isn't included here

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, "Coin Mode" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)") PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END


static INPUT_PORTS_START( mpatrol )
	PORT_INCLUDE(m52)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )             // IPT_JOYSTICK_DOWN
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )             // IPT_JOYSTICK_UP

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )             // IPT_JOYSTICK_DOWN PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )             // IPT_JOYSTICK_UP   PORT_COCKTAIL

	PORT_MODIFY("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )   // should have been "slow motion" but no conditional jump at 0x00c3
	// In stop mode, press 2 to stop and 1 to restart
	PORT_DIPNAME( 0x10, 0x10, "Stop Mode (Cheat)") PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Sector Selection (Cheat)") PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, "10000 30000 50000" )
	PORT_DIPSETTING(    0x08, "20000 40000 60000" )
	PORT_DIPSETTING(    0x04, "10000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	IREM_Z80_COINAGE_TYPE_1_LOC(SW1)
INPUT_PORTS_END


static INPUT_PORTS_START( mpatrolw )
	PORT_INCLUDE(mpatrol)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
INPUT_PORTS_END


static INPUT_PORTS_START( alpha1v )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )           // table at 0x5ef4 - 16 bytes (coins) + 16 bytes (credits)
	PORT_DIPSETTING(    0x0b, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 3C_1C ) )
//  PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
//  PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
//  PORT_DIPSETTING(    0x0a, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_1C ) )
//  PORT_DIPSETTING(    0x0e, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x05, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )           // table at 0x5f14 - 16 bytes (coins) + 16 bytes (credits)
//  PORT_DIPSETTING(    0xf0, "1 Coin/0 Credit" )
//  PORT_DIPSETTING(    0x70, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x60, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x50, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x20, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_7C ) )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(0,3), RGN_FRAC(1,3) },
	{ STEP8(0,1), STEP8(16 * 8,1) },
	{ STEP16(0,8) },
	32 * 8
};

static const uint32_t bgcharlayout_xoffset[256] =
{
	STEP4(0x000,1), STEP4(0x008,1), STEP4(0x010,1), STEP4(0x018,1),
	STEP4(0x020,1), STEP4(0x028,1), STEP4(0x030,1), STEP4(0x038,1),
	STEP4(0x040,1), STEP4(0x048,1), STEP4(0x050,1), STEP4(0x058,1),
	STEP4(0x060,1), STEP4(0x068,1), STEP4(0x070,1), STEP4(0x078,1),
	STEP4(0x080,1), STEP4(0x088,1), STEP4(0x090,1), STEP4(0x098,1),
	STEP4(0x0a0,1), STEP4(0x0a8,1), STEP4(0x0b0,1), STEP4(0x0b8,1),
	STEP4(0x0c0,1), STEP4(0x0c8,1), STEP4(0x0d0,1), STEP4(0x0d8,1),
	STEP4(0x0e0,1), STEP4(0x0e8,1), STEP4(0x0f0,1), STEP4(0x0f8,1),
	STEP4(0x100,1), STEP4(0x108,1), STEP4(0x110,1), STEP4(0x118,1),
	STEP4(0x120,1), STEP4(0x128,1), STEP4(0x130,1), STEP4(0x138,1),
	STEP4(0x140,1), STEP4(0x148,1), STEP4(0x150,1), STEP4(0x158,1),
	STEP4(0x160,1), STEP4(0x168,1), STEP4(0x170,1), STEP4(0x178,1),
	STEP4(0x180,1), STEP4(0x188,1), STEP4(0x190,1), STEP4(0x198,1),
	STEP4(0x1a0,1), STEP4(0x1a8,1), STEP4(0x1b0,1), STEP4(0x1b8,1),
	STEP4(0x1c0,1), STEP4(0x1c8,1), STEP4(0x1d0,1), STEP4(0x1d8,1),
	STEP4(0x1e0,1), STEP4(0x1e8,1), STEP4(0x1f0,1), STEP4(0x1f8,1)
};

static const uint32_t bgcharlayout_yoffset[128] =
{
	STEP32(0x0000,0x200), STEP32(0x4000,0x200), STEP32(0x8000,0x200), STEP32(0xc000,0x200)
};

static const gfx_layout bgcharlayout =
{
	256, 128, // 256x64 image format
	1,       // 1 image
	2,       // 2 bits per pixel
	{ 4, 0 },       // the two bitplanes for 4 pixels are packed into one byte
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	0x8000,
	bgcharlayout_xoffset,
	bgcharlayout_yoffset
};


static GFXDECODE_START(gfx_m52_sp)
	GFXDECODE_ENTRY("sp", 0x0000, spritelayout, 0, 16)
GFXDECODE_END

static GFXDECODE_START(gfx_m52_tx)
	GFXDECODE_ENTRY("tx", 0x0000, gfx_8x8x2_planar, 0, 128)
GFXDECODE_END

static GFXDECODE_START(gfx_m52_bg)
	GFXDECODE_ENTRY("bg0", 0x0000, bgcharlayout, 0 * 4, 1)
	GFXDECODE_ENTRY("bg1", 0x0000, bgcharlayout, 1 * 4, 1)
	GFXDECODE_ENTRY("bg2", 0x0000, bgcharlayout, 2 * 4, 1)
GFXDECODE_END




/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void m52_state::machine_reset()
{
	m_bgxpos[0] = 0;
	m_bgypos[0] = 0;
	m_bgxpos[1] = 0;
	m_bgypos[1] = 0;
	m_bgcontrol = 0;
}

void m52_state::m52(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 18.432_MHz_XTAL / 6);
	m_maincpu->set_addrmap(AS_PROGRAM, &m52_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &m52_state::main_portmap);
	m_maincpu->set_vblank_int("screen", FUNC(m52_state::irq0_line_hold));

	// video hardware
	PALETTE(config, m_sp_palette).set_entries(256, 32);
	GFXDECODE(config, m_sp_gfxdecode, m_sp_palette, gfx_m52_sp);

	PALETTE(config, m_tx_palette).set_entries(512);
	GFXDECODE(config, m_tx_gfxdecode, m_tx_palette, gfx_m52_tx);

	PALETTE(config, m_bg_palette).set_entries(3 * 4, 32);
	GFXDECODE(config, m_bg_gfxdecode, m_bg_palette, gfx_m52_bg);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(18.432_MHz_XTAL / 3, 384, 136, 376, 282, 22, 274);
	m_screen->set_screen_update(FUNC(m52_state::screen_update));

	// sound hardware
	IREM_M52_SOUNDC_AUDIO(config, "irem_audio", 0);
}


void alpha1v_state::alpha1v(machine_config &config)
{
	m52(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &alpha1v_state::main_map);
	m_screen->set_raw(18.432_MHz_XTAL / 3, 384, 136, 376, 282, 16, 272);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START(mpatrol)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mpa-1.3m", 0x0000, 0x1000, CRC(5873a860) SHA1(8c03726d6e049c3edbc277440184e31679f78258))
	ROM_LOAD("mpa-2.3l", 0x1000, 0x1000, CRC(f4b85974) SHA1(dfb2efb57378a20af6f20569f4360cde95596f93))
	ROM_LOAD("mpa-3.3k", 0x2000, 0x1000, CRC(2e1a598c) SHA1(112c3c9678db8a8540a8df3708020c87fd10c91b))
	ROM_LOAD("mpa-4.3j", 0x3000, 0x1000, CRC(dd05b587) SHA1(727961b0dafa4a96b580d51013336db2a18aff1e))

	ROM_REGION(0x8000, "irem_audio:iremsound", 0)
	ROM_LOAD("mp-s1.1a", 0x7000, 0x1000, CRC(561d3108) SHA1(4998c68a9e9a8002251fa8f07aa1082444a9dc80))

	ROM_REGION(0x2000, "tx", 0)
	ROM_LOAD("mpe-4.3f", 0x0000, 0x1000, CRC(cca6d023) SHA1(fecb3059fb09897a096add9452b50aec55c07545))
	ROM_LOAD("mpe-5.3e", 0x1000, 0x1000, CRC(e3ee7f75) SHA1(b03d0d56150d3e9da4a4c871338097b4f450b649))

	// 0x2000-0x2fff is intentionally left as 0x00 fill, unused bitplane
	ROM_REGION(0x3000, "sp", ROMREGION_ERASE00)
	ROM_LOAD("mpb-2.3m", 0x0000, 0x1000, CRC(707ace5e) SHA1(93c682e13e74bce29ced3a87bffb29569c114c3b))
	ROM_LOAD("mpb-1.3n", 0x1000, 0x1000, CRC(9b72133a) SHA1(1393ef92ae1ad58a4b62ca1660c0793d30a8b5e2))

	// 0x1000-01fff is intentionally left as 0xff fill for the bg regions
	ROM_REGION(0x2000, "bg0", ROMREGION_ERASEFF)
	ROM_LOAD("mpe-1.3l", 0x0000, 0x1000, CRC(c46a7f72) SHA1(8bb7c9acaf6833fb6c0575b015991b873a305a84))

	ROM_REGION(0x2000, "bg1", ROMREGION_ERASEFF)
	ROM_LOAD("mpe-2.3k", 0x0000, 0x1000, CRC(c7aa1fb0) SHA1(14c6c76e1d0db2c0745e5d6d33ea6945fac8e9ee))

	ROM_REGION(0x2000, "bg2", ROMREGION_ERASEFF)
	ROM_LOAD("mpe-3.3h", 0x0000, 0x1000, CRC(a0919392) SHA1(8a090cb8d483a3d67c7360058e3fdd70e151cd62))

	ROM_REGION(0x0200, "tx_pal", 0)
	ROM_LOAD("mpc-4.2a", 0x0000, 0x0200, CRC(07f99284) SHA1(dfc52958f2520e1ce4446dd4c84c91413bbacf76))

	ROM_REGION(0x0020, "bg_pal", 0)
	ROM_LOAD("mpc-3.1m", 0x0000, 0x0020, CRC(6a57eff2) SHA1(2d1c12dab5915da2ccd466e39436c88be434d634))

	ROM_REGION(0x0020, "spr_pal", 0)
	ROM_LOAD("mpc-1.1f", 0x0000, 0x0020, CRC(26979b13) SHA1(8c41a8cce4f3384c392a9f7a223a50d7be0e14a5))

	ROM_REGION(0x0100, "spr_clut", 0)
	ROM_LOAD("mpc-2.2h", 0x0000, 0x0100, CRC(7ae4cd97) SHA1(bc0662fac82ffe65f02092d912b2c2b0c7a8ac2b))

	ROM_REGION(0x0200, "unkprom", 0) // PROM is on bottom board of 4-board stack
	ROM_LOAD("mp_7621-5.7h", 0x0000, 0x0200, CRC(cf1fd9d0) SHA1(f9575bc59bf21dfecd10133264835e02890562f8))
ROM_END

ROM_START(mpatrolw)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mpa-1w.3m", 0x0000, 0x1000, CRC(baa1a1d4) SHA1(7968a7f221e7f4c9c81ddc8de17f6568e17b9ea8))
	ROM_LOAD("mpa-2w.3l", 0x1000, 0x1000, CRC(52459e51) SHA1(ae685b7848baa1b87a3f2bce97356286171e16d4))
	ROM_LOAD("mpa-3w.3k", 0x2000, 0x1000, CRC(9b249fe5) SHA1(c01e0d572c4c163f3cf4b2aa9f4246427811b78d))
	ROM_LOAD("mpa-4w.3j", 0x3000, 0x1000, CRC(fee76972) SHA1(c3166b027f89f61964ead804d3c2da387454c4c2))

	ROM_REGION(0x8000, "irem_audio:iremsound", 0)
	ROM_LOAD("mp-s1.1a", 0x7000, 0x1000, CRC(561d3108) SHA1(4998c68a9e9a8002251fa8f07aa1082444a9dc80))

	ROM_REGION(0x2000, "tx", 0)
	ROM_LOAD("mpe-4w.3f", 0x0000, 0x1000, CRC(caaba2d9) SHA1(7016a26c2d01e3209749598e993cd8ce91f12c88))
	ROM_LOAD("mpe-5w.3e", 0x1000, 0x1000, CRC(f56e01fe) SHA1(93f582d63b9cd5c6dca207aa57b213c939cdda1d))

	// 0x2000-0x2fff is intentionally left as 0x00 fill, unused bitplane
	ROM_REGION(0x3000, "sp", ROMREGION_ERASE00)
	ROM_LOAD("mpb-2.3m", 0x0000, 0x1000, CRC(707ace5e) SHA1(93c682e13e74bce29ced3a87bffb29569c114c3b))
	ROM_LOAD("mpb-1.3n", 0x1000, 0x1000, CRC(9b72133a) SHA1(1393ef92ae1ad58a4b62ca1660c0793d30a8b5e2))

	// 0x1000-01fff is intentionally left as 0xff fill for the bg regions
	ROM_REGION(0x2000, "bg0", ROMREGION_ERASEFF)
	ROM_LOAD("mpe-1.3l", 0x0000, 0x1000, CRC(c46a7f72) SHA1(8bb7c9acaf6833fb6c0575b015991b873a305a84))

	ROM_REGION(0x2000, "bg1", ROMREGION_ERASEFF)
	ROM_LOAD("mpe-2.3k", 0x0000, 0x1000, CRC(c7aa1fb0) SHA1(14c6c76e1d0db2c0745e5d6d33ea6945fac8e9ee))

	ROM_REGION(0x2000, "bg2", ROMREGION_ERASEFF)
	ROM_LOAD("mpe-3.3h", 0x0000, 0x1000, CRC(a0919392) SHA1(8a090cb8d483a3d67c7360058e3fdd70e151cd62))

	ROM_REGION(0x0200, "tx_pal", 0)
	ROM_LOAD("mpc-4a.2a", 0x0000, 0x0200, CRC(cb0a5ff3) SHA1(d3f88b4e0c4858abac8b52105656ecece0cf4df9))

	ROM_REGION(0x0020, "bg_pal", 0)
	ROM_LOAD("mpc-3.1m", 0x0000, 0x0020, CRC(6a57eff2) SHA1(2d1c12dab5915da2ccd466e39436c88be434d634))

	ROM_REGION(0x0020, "spr_pal", 0)
	ROM_LOAD("mpc-1.1f", 0x0000, 0x0020, CRC(26979b13) SHA1(8c41a8cce4f3384c392a9f7a223a50d7be0e14a5))

	ROM_REGION(0x0100, "spr_clut", 0)
	ROM_LOAD("mpc-2.2h", 0x0000, 0x0100, CRC(7ae4cd97) SHA1(bc0662fac82ffe65f02092d912b2c2b0c7a8ac2b))

	ROM_REGION(0x0200, "unkprom", 0) // PROM is on bottom board of 4-board stack
	ROM_LOAD("mp_7621-5.7h", 0x0000, 0x0200, CRC(cf1fd9d0) SHA1(f9575bc59bf21dfecd10133264835e02890562f8))
ROM_END

ROM_START(mranger)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mpa-1.3m", 0x0000, 0x1000, CRC(5873a860) SHA1(8c03726d6e049c3edbc277440184e31679f78258))
	ROM_LOAD("mra-2.3l", 0x1000, 0x1000, CRC(217dd431) SHA1(7b81f854209afc1fd3df11b7375f36de6bc4a7c3))
	ROM_LOAD("mra-3.3k", 0x2000, 0x1000, CRC(9f0af7b2) SHA1(3daaec15b0d3bc30723ebb14b50f66f288f0d096))
	ROM_LOAD("mra-4.3j", 0x3000, 0x1000, CRC(7fe8e2cd) SHA1(4ffad9c7a9360999b213b790c6c76cc79c8e49d5))

	ROM_REGION(0x8000, "irem_audio:iremsound", 0)
	ROM_LOAD("mp-s1.1a", 0x7000, 0x1000, CRC(561d3108) SHA1(4998c68a9e9a8002251fa8f07aa1082444a9dc80))

	ROM_REGION(0x2000, "tx", 0)
	ROM_LOAD("mpe-4.3f", 0x0000, 0x1000, CRC(cca6d023) SHA1(fecb3059fb09897a096add9452b50aec55c07545))
	ROM_LOAD("mpe-5.3e", 0x1000, 0x1000, CRC(e3ee7f75) SHA1(b03d0d56150d3e9da4a4c871338097b4f450b649))

	// 0x2000-0x2fff is intentionally left as 0x00 fill, unused bitplane
	ROM_REGION(0x3000, "sp", ROMREGION_ERASE00)
	ROM_LOAD("mpb-2.3m", 0x0000, 0x1000, CRC(707ace5e) SHA1(93c682e13e74bce29ced3a87bffb29569c114c3b))
	ROM_LOAD("mpb-1.3n", 0x1000, 0x1000, CRC(9b72133a) SHA1(1393ef92ae1ad58a4b62ca1660c0793d30a8b5e2))

	// 0x1000-01fff is intentionally left as 0xff fill for the bg regions
	ROM_REGION(0x2000, "bg0", ROMREGION_ERASEFF)
	ROM_LOAD("mpe-1.3l", 0x0000, 0x1000, CRC(c46a7f72) SHA1(8bb7c9acaf6833fb6c0575b015991b873a305a84))

	ROM_REGION(0x2000, "bg1", ROMREGION_ERASEFF)
	ROM_LOAD("mpe-2.3k", 0x0000, 0x1000, CRC(c7aa1fb0) SHA1(14c6c76e1d0db2c0745e5d6d33ea6945fac8e9ee))

	ROM_REGION(0x2000, "bg2", ROMREGION_ERASEFF)
	ROM_LOAD("mpe-3.3h", 0x0000, 0x1000, CRC(a0919392) SHA1(8a090cb8d483a3d67c7360058e3fdd70e151cd62))

	ROM_REGION(0x0200, "tx_pal", 0)
	ROM_LOAD("mpc-4.2a", 0x0000, 0x0200, CRC(07f99284) SHA1(dfc52958f2520e1ce4446dd4c84c91413bbacf76))

	ROM_REGION(0x0020, "bg_pal", 0)
	ROM_LOAD("mpc-3.1m", 0x0000, 0x0020, CRC(6a57eff2) SHA1(2d1c12dab5915da2ccd466e39436c88be434d634))

	ROM_REGION(0x0020, "spr_pal", 0)
	ROM_LOAD("mpc-1.1f", 0x0000, 0x0020, CRC(26979b13) SHA1(8c41a8cce4f3384c392a9f7a223a50d7be0e14a5))

	ROM_REGION(0x0100, "spr_clut", 0)
	ROM_LOAD("mpc-2.2h", 0x0000, 0x0100, CRC(7ae4cd97) SHA1(bc0662fac82ffe65f02092d912b2c2b0c7a8ac2b))
ROM_END


ROM_START(alpha1v)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("2-m3",  0x0000, 0x1000, CRC(3a679d34) SHA1(1a54a43070c56dc91d4d258f29e29613bb309f1c))
	ROM_LOAD("3-l3",  0x1000, 0x1000, CRC(2f09df64) SHA1(e91602e9e41ad24dd1d7f384ed81b9bdaadd03e1))
	ROM_LOAD("4-k3",  0x2000, 0x1000, CRC(64fb9c8a) SHA1(735fd00cc42193a417e6cde75f12b4cf2e804942))
	ROM_LOAD("5-j3",  0x3000, 0x1000, CRC(d1643d18) SHA1(7c794b82e17e2ba0a6237e3fc20d8314f6c2481c))
	ROM_LOAD("6-h3",  0x4000, 0x1000, CRC(cf34ab51) SHA1(3696da71e2bc7edd1ee7aeaac87be5386608c09e))
	ROM_LOAD("7-f3",  0x5000, 0x1000, CRC(99db9781) SHA1(a56a675cc4cbc9681bfe8052f51f19336eb2a0a6))
	ROM_LOAD("7a e3", 0x6000, 0x1000, CRC(3b0b4b0d) SHA1(0d8eea1e2db269943611289b3490a578ee347f85))

	ROM_REGION(0x8000, "irem_audio:iremsound", 0)
	ROM_LOAD("1-a1", 0x7000, 0x1000, CRC(9e07fdd5) SHA1(ed4f462fcfe91fa8e88bfeaaba0a0c11fa0b4601))

	ROM_REGION(0x2000, "tx", 0)
	ROM_LOAD("13-f3", 0x0000, 0x1000, CRC(4b799229) SHA1(42cbdcf787b08b041d30504d699a12c378224933))
	ROM_LOAD("14-e3", 0x1000, 0x1000, CRC(cf00c737) SHA1(415e90289039cac4d04cb1d559f1378ca6a32132))

	ROM_REGION(0x3000, "sp", 0) // 3bpp (mpatrol is 2bpp)
	ROM_LOAD("15-n3", 0x0000, 0x1000, CRC(dc26df76) SHA1(dd1cff7935f5559f9d1b440e02d5e5aa521b0054))
	ROM_LOAD("16-l3", 0x1000, 0x1000, CRC(39b9863b) SHA1(da9da9a1066188f050c422dfed1bbbd3ba612ccc))
	ROM_LOAD("17-k3", 0x2000, 0x1000, CRC(cfd90773) SHA1(052e126888b6de636db9c521a090699c282b620b))

	// all the background ROMs just contain stars, looks like it wants 2x128 px high images, instead of 3x64, arrangement unclear
	ROM_REGION(0x2000, "bg0", ROMREGION_ERASEFF)
	ROM_LOAD("11-k3",  0x0000, 0x1000, CRC(7659440a) SHA1(2efd27c82913513dd03e799f1ed3c10b0863677d)) // ROM is duplicated
	ROM_LOAD("12-jh3", 0x1000, 0x1000, CRC(7659440a) SHA1(2efd27c82913513dd03e799f1ed3c10b0863677d))

	ROM_REGION(0x2000, "bg1", ROMREGION_ERASEFF)
	ROM_LOAD("9-n3",   0x0000, 0x1000, CRC(0fdb7d13) SHA1(e828254a4f94df633d338b5772719276d41c6b7f))
	ROM_LOAD("10-lm3", 0x1000, 0x1000, CRC(9dde3a75) SHA1(293d093485be19bfb20685d76a08ac78e24062bf))

	ROM_REGION(0x2000, "bg2", ROMREGION_ERASEFF)
	// unused layer

	ROM_REGION(0x0200, "tx_pal", 0)
	ROM_LOAD("63s481-a2", 0x0000, 0x0200, CRC(58678ea8) SHA1(b13a78a5bca8ad5bdec1293512b53654768a7a7a))

	ROM_REGION(0x0020, "bg_pal", 0)
	ROM_LOAD("18s030-m1", 0x0000, 0x0020, CRC(6a57eff2) SHA1(2d1c12dab5915da2ccd466e39436c88be434d634))

	ROM_REGION(0x0020, "spr_pal", 0)
	ROM_LOAD("mb7051-f1", 0x0000, 0x0020, CRC(d8bdd0df) SHA1(ca522428927911808214d319af314f601497ded4))

	ROM_REGION(0x0100, "spr_clut", 0)
	ROM_LOAD("mb7052-h2", 0x0000, 0x0100, CRC(ce9f0ef9) SHA1(3afb94ed033f272983bbed22a59856df7824ef8a))
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME(1982, mpatrol,  0,       m52,     mpatrol,  m52_state,     empty_init, ROT0, "Irem",                    "Moon Patrol",                          MACHINE_SUPPORTS_SAVE)
GAME(1982, mpatrolw, mpatrol, m52,     mpatrolw, m52_state,     empty_init, ROT0, "Irem (Williams license)", "Moon Patrol (Williams)",               MACHINE_SUPPORTS_SAVE) // USA
GAME(1982, mranger,  mpatrol, m52,     mpatrol,  m52_state,     empty_init, ROT0, "bootleg",                 "Moon Ranger (bootleg of Moon Patrol)", MACHINE_SUPPORTS_SAVE) // Italy

GAME(1988, alpha1v,  0,       alpha1v, alpha1v,  alpha1v_state, empty_init, ROT0, "Vision Electronics",      "Alpha One (Vision Electronics)",       MACHINE_SUPPORTS_SAVE)
