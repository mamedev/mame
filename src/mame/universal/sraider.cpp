// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo
/***************************************************************************

Universal 8203-A + 8203-B PCB set

Space Raider
Mrs. Dynamite

Space Raider also uses the Starfield generator board from Zero Hour,
    Connected via flywires to these boards

TODO:
- decode cpu#2 writes to port 0x30 and 0x38 - resistors for sound
- decode cpu#2 writes to port 0x28-0x2f - ???
- unknown dips

***************************************************************************/

#include "emu.h"
#include "ladybug_video.h"
#include "zerohour_stars.h"

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "sound/sn76496.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include <algorithm>

namespace {

// stripped down hardware, mrsdyna runs on this
class mrsdyna_state : public driver_device
{
public:
	mrsdyna_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "sub")
		, m_palette(*this, "palette")
		, m_gfxdecode(*this, "gfxdecode")
		, m_video(*this, "video")
	{ }

	void mrsdyna(machine_config &config);

protected:
	u8 protection_r();
	void unk_0x28_w(u8 data);
	void unk_0x30_w(u8 data);
	void unk_0x38_w(u8 data);
	u8 rnd_r();
	virtual void io_w(u8 data);
	void mrsdyna_palette(palette_device &palette) const;

	virtual void machine_start() override ATTR_COLD;
	virtual u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void cpu1_map(address_map &map) ATTR_COLD;
	void cpu2_map(address_map &map) ATTR_COLD;
	void cpu2_io_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<ladybug_video_device> m_video;

	u8 m_grid_color = 0;
	u8 m_0x28 = 0;
	u8 m_0x30 = 0;
	u8 m_0x38 = 0;
};

// add stars from zerohour, uses grid layer
class sraider_state : public mrsdyna_state
{
public:
	sraider_state(const machine_config &mconfig, device_type type, const char *tag)
		: mrsdyna_state(mconfig, type, tag)
		, m_grid_data(*this, "grid_data")
		, m_stars(*this, "stars")
	{ }

	void sraider(machine_config &config);

protected:
	virtual void io_w(u8 data) override;
	void sraider_palette(palette_device &palette) const;
	TILE_GET_INFO_MEMBER(get_grid_tile_info);

	tilemap_t *m_grid_tilemap = nullptr;

	virtual void video_start() override ATTR_COLD;
	virtual u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) override;

private:
	required_shared_ptr<u8> m_grid_data;
	required_device<zerohour_stars_device> m_stars;
};

void mrsdyna_state::machine_start()
{
	save_item(NAME(m_grid_color));
	save_item(NAME(m_0x28));
	save_item(NAME(m_0x30));
	save_item(NAME(m_0x38));
}



/***************************************************************************
    Video
***************************************************************************/

void mrsdyna_state::mrsdyna_palette(palette_device &palette) const
{
	// assumed to be the same as Lady Bug
	static constexpr int resistances[2] = { 470, 220 };

	// compute the color output resistor weights
	double rweights[2], gweights[2], bweights[2];
	compute_resistor_weights(0, 255, -1.0,
			2, resistances, rweights, 470, 0,
			2, resistances, gweights, 470, 0,
			2, resistances, bweights, 470, 0);

	const u8 *color_prom = memregion("proms")->base();

	// create a lookup table for the palette
	for (int i = 0; i < 0x20; i++)
	{
		int bit0, bit1;

		// red component
		bit0 = BIT(~color_prom[i], 3);
		bit1 = BIT(~color_prom[i], 0);
		int const r = combine_weights(rweights, bit0, bit1);

		// green component
		bit0 = BIT(~color_prom[i], 5);
		bit1 = BIT(~color_prom[i], 4);
		int const g = combine_weights(gweights, bit0, bit1);

		// blue component
		bit0 = BIT(~color_prom[i], 7);
		bit1 = BIT(~color_prom[i], 6);
		int const b = combine_weights(bweights, bit0, bit1);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x20;

	// characters
	for (int i = 0; i < 0x20; i++)
	{
		u8 const ctabentry = ((i << 3) & 0x18) | ((i >> 2) & 0x07);
		palette.set_pen_indirect(i, ctabentry);
	}

	// sprites
	for (int i = 0; i < 0x20; i++)
	{
		u8 ctabentry;

		ctabentry = bitswap<4>((color_prom[i] >> 0) & 0x0f, 0,1,2,3);
		palette.set_pen_indirect(i + 0x20, ctabentry);

		ctabentry = bitswap<4>((color_prom[i] >> 4) & 0x0f, 0,1,2,3);
		palette.set_pen_indirect(i + 0x40, ctabentry);
	}
}

void sraider_state::sraider_palette(palette_device &palette) const
{
	mrsdyna_palette(palette);

	// star colors
	for (int i = 0; i < 0x20; i++)
	{
		int bit0, bit1;

		// red component
		bit0 = BIT(i, 0);
		int const r = 0x97 * bit0;

		// green component
		bit0 = BIT(i, 2);
		bit1 = BIT(i, 1);
		int const g = 0x47 * bit0 + 0x97 * bit1;

		// blue component
		bit0 = BIT(i, 4);
		bit1 = BIT(i, 3);
		int const b = 0x47 * bit0 + 0x97 * bit1;

		palette.set_indirect_color(i + 0x20, rgb_t(r, g, b));
		palette.set_pen_indirect(i + 0x60, i + 0x20);
	}

	// stationary part of grid
	palette.set_pen_indirect(0x80, 0x00);
	palette.set_pen_indirect(0x81, 0x40);
}


TILE_GET_INFO_MEMBER(sraider_state::get_grid_tile_info)
{
	if (tile_index < 512)
	{
		tileinfo.set(3, tile_index, 0, 0);
	}
	else
	{
		int temp = tile_index / 32;
		tile_index = (31 - temp) * 32 + (tile_index % 32);
		tileinfo.set(4, tile_index, 0, 0);
	}
}

void sraider_state::video_start()
{
	mrsdyna_state::video_start();

	m_grid_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sraider_state::get_grid_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_grid_tilemap->set_scroll_rows(32);
	m_grid_tilemap->set_transparent_pen(0);
}

u32 mrsdyna_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// clear the bg bitmap
	bitmap.fill(0, cliprect);

	// now the chars/sprites
	m_video->draw(screen, bitmap, cliprect, flip_screen());

	return 0;
}

u32 sraider_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// clear the bg bitmap
	bitmap.fill(0, cliprect);

	// draw the stars
	rectangle stars_clip = cliprect;
	if (flip_screen())
	{
		stars_clip.min_x = 0x27;
		stars_clip.max_x = 0xff;
	}
	else
	{
		stars_clip.min_x = 0x00;
		stars_clip.max_x = 0xd8;
	}
	stars_clip &= cliprect;
	m_stars->draw(bitmap, stars_clip);

	// draw the gridlines
	m_palette->set_indirect_color(0x40, rgb_t(
			m_grid_color & 0x40 ? 0xff : 0,
			m_grid_color & 0x20 ? 0xff : 0,
			m_grid_color & 0x10 ? 0xff : 0));
	m_grid_tilemap->draw(screen, bitmap, cliprect, 0, flip_screen());

	for (unsigned i = 0; i < 0x100; i++)
	{
		if (m_grid_data[i] != 0)
		{
			u8 x = i;
			int height = cliprect.max_y - cliprect.min_y + 1;

			if (flip_screen())
				x = ~x;

			bitmap.plot_box(x, cliprect.min_y, 1, height, 0x81);
		}
	}

	// now the chars/sprites
	m_video->draw(screen, bitmap, cliprect, flip_screen());

	return 0;
}



/***************************************************************************
    I/O
***************************************************************************/

void mrsdyna_state::io_w(u8 data)
{
	// bit012 = stars speed/dir (not used for mrsdyna)
	// bit3 = enable stars (not used for mrsdyna)
	// bit4 = grid blue
	// bit5 = grid green
	// bit6 = grid red
	// bit7 = flip
	m_grid_color = data & 0x70;
	flip_screen_set(data & 0x80);
}

void sraider_state::io_w(u8 data)
{
	mrsdyna_state::io_w(data);

	m_stars->set_enable(BIT(data, 3));

	// There must be a subtle clocking difference between
	// Space Raider and the other games using this star generator,
	// hence the -1 here
	m_stars->set_speed((data & 0x07) - 1, 0x07);
}

// documentation TBD - 556 dual timer
u8 mrsdyna_state::rnd_r()
{
	return machine().rand() & 3;
}

// Protection - documentation TBD
u8 mrsdyna_state::protection_r()
{
	// This must return X011111X or cpu #1 will hang
	// see code at rst $10
	return 0x3e;
}

// Unknown IO - documentation TBD
void mrsdyna_state::unk_0x28_w(u8 data)
{
	// These 8 bits are stored in the LS259 latch at A7,
	// and connected to all the select lines on 2 4066s at B7/C7
	m_0x28 = data;
}

// documentation TBD
void mrsdyna_state::unk_0x30_w(u8 data)
{
	// bits 0-2 select 4051s at M7 and M8
	// bits 3-5 select 4051s at K7 and K8
	m_0x30 = data & 0x3f;
}

// documentation TBD
void mrsdyna_state::unk_0x38_w(u8 data)
{
	// These 6 bits are stored in the LS174 latch at N8
	// bits 0-2 select 4051s at H7 and H8
	// bits 3-5 select 4051s at E7 and E8
	m_0x38 = data & 0x3f;
}



/***************************************************************************
    Address Maps
***************************************************************************/

void mrsdyna_state::cpu1_map(address_map &map)
{
	// LS138 @ J4
	map(0x0000, 0x5fff).rom();  // 2764s at R4, N4, and M4
	// LS138 @ J4 and LS139 @ H4
	map(0x6000, 0x6fff).ram();  // 6116s @ K3 & M3, also connected to clk on PAL K2 (16R6, U001)
	map(0x7000, 0x73ff).w("video", FUNC(ladybug_video_device::spr_w)); // pin 29 on ribbon
	//map(0x77ff, 0x7fff);  // LS139 @ H4 pin7 is NC

	// LS138 @ J4, Pin11 (0x8000-0x9fff) and
	// LS138 @ N3 (bottom 3 bits)
	// (all of these are read/write)
	map(0x8005, 0x8005).mirror(0x1ff8).r(FUNC(mrsdyna_state::protection_r));  // OE on PAL @ K2 (16R6, U001) (100x xxxx xxxx x101)
	map(0x8006, 0x8006).mirror(0x1ff8).w("soundlatch_low", FUNC(generic_latch_8_device::write)); // LS374 @ P6
	map(0x8007, 0x8007).mirror(0x1ff8).w("soundlatch_high", FUNC(generic_latch_8_device::write)); // LS374 @ R6
	map(0x8000, 0x8000).mirror(0x1ff8).portr("IN0");
	map(0x8001, 0x8001).mirror(0x1ff8).portr("IN1");
	map(0x8002, 0x8002).mirror(0x1ff8).portr("DSW0");
	map(0x8003, 0x8003).mirror(0x1ff8).portr("DSW1");
	//map(0x8004, 0x8004).mirror(0x1ff8).portr("IN2"); // extra JAMMA pins
	// LS138 @ J4, Pin10 (0xa000-0xbfff) // NC
	// LS138 @ J4, Pin9 (0xc000-0xdfff)
	map(0xd000, 0xd7ff).w("video", FUNC(ladybug_video_device::bg_w)); // pin 27 on ribbon
	// LS138 @ J4, Pin7 (0xe000-0xffff)
	map(0xe000, 0xe000).nopw();  //unknown 0x10 when in attract, 0x20 when coined/playing - disabled watchdog based on LS123 @ F4
}

void mrsdyna_state::cpu2_map(address_map &map)
{
	// LS138 @ P7
	map(0x0000, 0x5fff).rom(); // 2764s at H6,J6, and L6
	map(0x6000, 0x63ff).mirror(0x0400).ram(); // 2x2114 @ M6/N6
	map(0x8000, 0x8000).mirror(0x1fff).r("soundlatch_low", FUNC(generic_latch_8_device::read)); // LS374 @ P6
	map(0xa000, 0xa000).mirror(0x1fff).r("soundlatch_high", FUNC(generic_latch_8_device::read)); // LS374 @ R6
	map(0xc000, 0xc000).mirror(0x1fff).r(FUNC(mrsdyna_state::rnd_r)); // LS125 @ P8 - reads 556 outputs to D1 and D0?
	// LS138 @ P7 (nY7) and LS139 @ H4
	map(0xe000, 0xe0ff).mirror(0x0300).writeonly().share("grid_data"); // HD6148P @ D6
	map(0xe800, 0xefff).w(FUNC(mrsdyna_state::io_w)); // LS273 @ D4
	//map(0xf000, 0xf7ff)  // NC
	//map(0xf800, 0xffff)  // NC
}

void mrsdyna_state::cpu2_io_map(address_map &map)
{
	map.global_mask(0xff);
	// LS138 @ A8
	map(0x00, 0x07).w("sn1", FUNC(sn76489_device::write)); // J214X2 @ N9
	map(0x08, 0x0f).w("sn2", FUNC(sn76489_device::write)); // J214X2 @ M9
	map(0x10, 0x17).w("sn3", FUNC(sn76489_device::write)); // J214X2 @ L9
	map(0x18, 0x1f).w("sn4", FUNC(sn76489_device::write)); // J214X2 @ K9
	map(0x20, 0x27).w("sn5", FUNC(sn76489_device::write)); // J214X2 @ J9
	map(0x28, 0x2f).w("unklatch", FUNC(ls259_device::write_d0)); // LS259 @ A7
	map(0x30, 0x37).w(FUNC(mrsdyna_state::unk_0x30_w)); // LS174 @ N7
	map(0x38, 0x3f).w(FUNC(mrsdyna_state::unk_0x38_w)); // LS174 @ N8
}



/***************************************************************************
    Input Ports
***************************************************************************/

static INPUT_PORTS_START( sraider )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // JAMMA PIN 12

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // VBLANK????
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW0") // DSW0 @ R3 via '244 @ R2
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )  PORT_DIPLOCATION("SW0:7,8")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x04, "High Score Names" )     PORT_DIPLOCATION("SW0:6")
	PORT_DIPSETTING(    0x00, "3 Letters" )
	PORT_DIPSETTING(    0x04, "10 Letters" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Allow_Continue ) )  PORT_DIPLOCATION("SW0:5")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW0:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW0:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )  PORT_DIPLOCATION("SW0:1,2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0x40, "5" )

	// Free Play setting works when it's set for both
	PORT_START("DSW1") // DSW1 @ P3 via '244 @ P2
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:8,7,6,5")
	// settings 0x00 through 0x05 all give 1 Coin/1 Credit
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )    PORT_DIPLOCATION("SW1:4,3,2,1")
	// settings 0x00 through 0x50 all give 1 Coin/1 Credit
	PORT_DIPSETTING(    0x60, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mrsdyna )
	PORT_INCLUDE( sraider )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW0:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW0:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "High Score Names" )  PORT_DIPLOCATION("SW0:6")
	PORT_DIPSETTING(    0x00, "3 Letters" )
	PORT_DIPSETTING(    0x04, "12 Letters" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW0:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW0:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW0:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW0:1,2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0x40, "5" )
INPUT_PORTS_END



/***************************************************************************
    GFX Layouts
***************************************************************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	512,    /* 512 characters */
	2,  /* 2 bits per pixel */
	{ 0, 512*8*8 }, /* the two bitplanes are separated */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	128,    /* 128 sprites */
	2,  /* 2 bits per pixel */
	{ 1, 0 },   /* the two bitplanes are packed in two consecutive bits */
	{ 0, 2, 4, 6, 8, 10, 12, 14,
			8*16+0, 8*16+2, 8*16+4, 8*16+6, 8*16+8, 8*16+10, 8*16+12, 8*16+14 },
	{ 23*16, 22*16, 21*16, 20*16, 19*16, 18*16, 17*16, 16*16,
			7*16, 6*16, 5*16, 4*16, 3*16, 2*16, 1*16, 0*16 },
	64*8    /* every sprite takes 64 consecutive bytes */
};
static const gfx_layout spritelayout2 =
{
	8,8,    /* 8*8 sprites */
	512,    /* 512 sprites */
	2,  /* 2 bits per pixel */
	{ 1, 0 },   /* the two bitplanes are packed in two consecutive bits */
	{ 0, 2, 4, 6, 8, 10, 12, 14 },
	{ 7*16, 6*16, 5*16, 4*16, 3*16, 2*16, 1*16, 0*16 },
	16*8    /* every sprite takes 16 consecutive bytes */
};

static const gfx_layout gridlayout =
{
	8,8,    /* 8*8 characters */
	512,    /* 512 characters */
	1,  /* 1 bit per pixel */
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8},
	8*8 /* every char takes 8 consecutive bytes */
};
static const gfx_layout gridlayout2 =
{
	8,8,    /* 8*8 characters */
	512,    /* 512 characters */
	1,  /* 1 bit per pixel */
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static GFXDECODE_START( gfx_mrsdyna )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,      0,  8 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,  4*8, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout2, 4*8, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_sraider )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,              0,  8 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,          4*8, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout2,         4*8, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, gridlayout,    4*8+4*16+32,  1 )
	GFXDECODE_ENTRY( "gfx3", 0, gridlayout2,   4*8+4*16+32,  1 )
GFXDECODE_END



/***************************************************************************
    Machine Configs
***************************************************************************/

void mrsdyna_state::mrsdyna(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 4000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mrsdyna_state::cpu1_map);
	m_maincpu->set_vblank_int("screen", FUNC(mrsdyna_state::irq0_line_hold));

	Z80(config, m_subcpu, 4000000);
	m_subcpu->set_addrmap(AS_PROGRAM, &mrsdyna_state::cpu2_map);
	m_subcpu->set_addrmap(AS_IO, &mrsdyna_state::cpu2_io_map);
	m_subcpu->set_vblank_int("screen", FUNC(mrsdyna_state::irq0_line_hold));

	GENERIC_LATCH_8(config, "soundlatch_low");
	GENERIC_LATCH_8(config, "soundlatch_high");

	ls259_device &unklatch(LS259(config, "unklatch"));
	unklatch.parallel_out_cb().set(FUNC(mrsdyna_state::unk_0x28_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(9'828'000 / 2, 312, 8, 248, 262, 32, 224);
	screen.set_screen_update(FUNC(mrsdyna_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mrsdyna);
	PALETTE(config, m_palette, FUNC(mrsdyna_state::mrsdyna_palette), 4*8 + 4*16, 32);

	LADYBUG_VIDEO(config, m_video, 4000000).set_gfxdecode_tag(m_gfxdecode);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	SN76489(config, "sn1", 4000000).add_route(ALL_OUTPUTS, "mono", 0.2);
	SN76489(config, "sn2", 4000000).add_route(ALL_OUTPUTS, "mono", 0.2);
	SN76489(config, "sn3", 4000000).add_route(ALL_OUTPUTS, "mono", 0.2);
	SN76489(config, "sn4", 4000000).add_route(ALL_OUTPUTS, "mono", 0.2);
	SN76489(config, "sn5", 4000000).add_route(ALL_OUTPUTS, "mono", 0.2);
}

void sraider_state::sraider(machine_config &config)
{
	mrsdyna(config);

	// video hardware
	GFXDECODE(config.replace(), m_gfxdecode, m_palette, gfx_sraider);
	PALETTE(config.replace(), m_palette, FUNC(sraider_state::sraider_palette), 4*8 + 4*16 + 32 + 2, 32 + 32 + 1);

	ZEROHOUR_STARS(config, m_stars).has_va_bit(false);
	subdevice<screen_device>("screen")->screen_vblank().set(m_stars, FUNC(zerohour_stars_device::update_state));
}



/***************************************************************************
    ROM Definitions
***************************************************************************/

ROM_START( sraider )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sraid3.r4",    0x0000, 0x2000, CRC(0f389774) SHA1(c67596e6bf00175ff0a241506cd2f88114d05933) )
	ROM_LOAD( "sraid2.n4",    0x2000, 0x2000, CRC(38a48db0) SHA1(6f4f384d702fb8ee4bb2ef579638239d57e32ddd) )
	ROM_LOAD( "sraid1.m4",    0x4000, 0x2000, CRC(2f302a4e) SHA1(3a902ce6858f38df88b60830bef4b1d45b09b2df) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "sraid-s4.h6",  0x0000, 0x2000, CRC(57173a12) SHA1(6cb8fd4826e499f9a4e63621d58bc4b596cc261e) )
	ROM_LOAD( "sraid-s5.j6",  0x2000, 0x2000, CRC(5a459179) SHA1(a261c8f3c7c4cd4587c003bbbe815d2c4e01ffbc) )
	ROM_LOAD( "sraid-s6.l6",  0x4000, 0x2000, CRC(ea3aa25d) SHA1(353c0d075d5e0a3bc25a65e2748f5eb5212a844d) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sraid-s0.k6",  0x0000, 0x1000, CRC(a0373909) SHA1(00e3bd5dd90769d670fc3c51edd1cd4b69e6132d) )
	ROM_LOAD( "sraids11.l6",  0x1000, 0x1000, CRC(ba22d949) SHA1(83762ced1df92ff594887e44d5b783826bbfb0c9) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "sraid-s7.m2",  0x0000, 0x1000, CRC(299f8e07) SHA1(1de71f251286088487da7285d6f8070147002af5) )
	ROM_LOAD( "sraid-s8.n2",  0x1000, 0x1000, CRC(57ba8888) SHA1(2aa1a5f682d146a55a96e471bb78e5c60da02bf9) )

	ROM_REGION( 0x1000, "gfx3", 0 ) /* fixed portion of the grid */
	ROM_LOAD( "sraid-s9.f6",  0x0000, 0x1000, CRC(2380b90f) SHA1(0310554e3f2ec973c2bb6e816d04e5c0c1e0a0b9) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "srpr10-1.a2",  0x0000, 0x0020, CRC(121fdb99) SHA1(3bc092da40beb129a4df3db2f55d22bbbcf7bad8) )
	ROM_LOAD( "srpr10-2.l3",  0x0020, 0x0020, CRC(88b67e70) SHA1(e21ee2939e96dffee101bd92c62ed975b6b64001) )
	ROM_LOAD( "srpr10-3.c1",  0x0040, 0x0020, CRC(27fa3a50) SHA1(7cf59b7a37c156640d6ea91554d1c4276c1780e0) ) /* ?? */
ROM_END

ROM_START( mrsdyna )
	ROM_REGION( 0x10000, "maincpu", 0 )
	// NOTE: Mrs. Dynamite returns ROM ERROR in test mode.  It does an 8-bit checksum on these 3
	//       ROMs and computes 0xFF.  The answer to pass the test is 0x00.
	//       However, these images were dumped twice, and seem to work fine.
	ROM_LOAD( "mrsd-8203a-r4.f3", 0x0000, 0x2000, CRC(c944062c) SHA1(c61fc327d67595e601f6a7e5e337646f5f9d351b) )
	ROM_LOAD( "mrsd-8203a-n4.f2", 0x2000, 0x2000, CRC(d1b9c7bb) SHA1(c139c8ae5b14924eb04a265095a7ab95ac5370af) )
	ROM_LOAD( "mrsd-8203a-m4.f1", 0x4000, 0x2000, CRC(d25b1dfe) SHA1(f68c6fb2cda37fcffbe7c3c2a3cc5cb372c4101b) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "mrsd-8203a-h6.f4", 0x0000, 0x2000, CRC(04f8617b) SHA1(64deef2269790d8460d0ad510548e178f0f61607) )
	ROM_LOAD( "mrsd-8203a-j6.f5", 0x2000, 0x2000, CRC(1ffb5fc3) SHA1(e8fc7b95663a396ef7d46ba6ce24973a3c343381) )
	ROM_LOAD( "mrsd-8203a-l6.f6", 0x4000, 0x2000, CRC(5a0f5030) SHA1(d1530230fe6c666f7920cb82cb47f5fcc7e1ecc8) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "mrsd-8203b-k6.f10", 0x0000, 0x1000, CRC(e33cb26e) SHA1(207fa986754f8d7cd0bb3e56fd271ee0c1990269) )
	ROM_LOAD( "mrsd-8203b-l6.f11", 0x1000, 0x1000, CRC(a327ba05) SHA1(5eac27b48d14fec179919fe0902a6c7ada95f2b2) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "mrsd-8203b-m2.f7", 0x0000, 0x1000, CRC(a00ae797) SHA1(adff7f38870b7e8fa114886792a3acbb7a5726ab) )
	ROM_LOAD( "mrsd-8203b-n2.f8", 0x1000, 0x1000, CRC(81f2bdbd) SHA1(45ee1d62462cfadf7d2c46767f03ccfb3c876c08) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "mrsd-10-1.a2",  0x0000, 0x0020, CRC(4a819ad4) SHA1(d9072af7e52b506c1bcf8a327242d470eb240857) )
	ROM_LOAD( "mrsd-10-2.l3",  0x0020, 0x0020, CRC(2d926a3a) SHA1(129fb60ce3df67614e39dcaac9c93f0652addbbb) )
	ROM_LOAD( "mrsd-10-3.c1",  0x0040, 0x0020, CRC(27fa3a50) SHA1(7cf59b7a37c156640d6ea91554d1c4276c1780e0) ) /* ?? */
ROM_END

} // anonymous namespace



/***************************************************************************
    Drivers
***************************************************************************/

//    YEAR  NAME     PARENT MACHINE  INPUT    STATE          INIT        SCREEN  COMPANY      FULLNAME         FLAGS
GAME( 1982, sraider, 0,     sraider, sraider, sraider_state, empty_init, ROT270, "Universal", "Space Raider",  MACHINE_SUPPORTS_SAVE )
GAME( 1982, mrsdyna, 0,     mrsdyna, mrsdyna, mrsdyna_state, empty_init, ROT270, "Universal", "Mrs. Dynamite", MACHINE_SUPPORTS_SAVE )
