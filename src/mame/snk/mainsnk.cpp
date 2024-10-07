// license:BSD-3-Clause
// copyright-holders:David Haywood, Tomasz Slanina
/*
Main Event - SNK 1984
Canvas Croquis - SNK 1985

driver by David Haywood, Tomasz Slanina


Notes:
------
- mainsnk controls:
  The game uses 2 joysticks (with button on top) and 2 buttons per player.
  Left stick
  up: left straight punch to enemy's face
  left: sway to left

  Right stick
  up: right straight punch to enemy's face
  right: sway to right

  Left + Right stick combinations
  L down + R up: right straight punch to enemy's body
  L up + R down: left straight punch to enemy's body
  L right + R left: guard player's body

  to move the boxer, use joystick while pressing the button on top of the stick.

  Left button: left uppercut
  Right button: right uppercut
  to recover from down, press these buttons rapidly.


- canvas doesn't use the tx layer, though the circuitry is presumably still
  present on the PCB. One gfx ROM socket is left empty which causes the tx layer
  to be completely transparent.

- neither mainsnk nor canvas pass the ROM test in service mode. This looks like
  a bug (as in madcrash). SNK obviously didn't pay too much attention to details
  in those days.
  Note that in canvas you can't see the error since the tx layer is transparent.
  Load cc_p9.a2 at 0x0000 to see it.

TODO:
-----
- are mainsnk control right? The way the button works is awkward.

- several unknown dip switches

- the hardware surely supports sprite shadows as most of the games in snk.cpp, but
  the feature isn't used by these two games.

- mainsnk doesn't seem to write to the bg/sprite scroll registers? So I hardcoded
  scroll values for these two games, even if canvas does seem to write to them.

- canvas writes to several unknown addresses on startup. Most of them should be
  the scroll registers while others are unknown.

- the bg tilemap is set to 256x256, however it could well be 512x256 as in the
  other early SNK games in snk.cpp.

-----

Canvas Croquis

file : readme.txt
author : Stefan Lindberg
created: 2005-12-24
updated: *
version: 1.0

Canvas Croquis, SNK 1984

Note:

The BPROMs(MB7054) were read as 74s572.
I have not tested this PCB yet so I have no idea if it's working.
All BPROMs and P1-P8 are on top PCB, P9-P14 on bottom board, see pictures.

Documentation:

Name Size CRC32
-----------------------------------------------------------
cc_top_pcb.jpg 974024 0xd2fb553e
cc_bottom_pcb.jpg 964448 0xe8bad203
Roms:
Name Size CRC32 Chip Type
-----------------------------------------------------------
cc_bprom1.j10 1024 0xfbbbf911 MB7054 (read as 74s572)
cc_bprom2.j9 1024 0x19efe7df MB7054 (read as 74s572)
cc_bprom3.j8 1024 0x21f72498 MB7054 (read as 74s572)
cc_p1.a2 8192 0xfa7109e1 M5L2764k
cc_p2.a3 8192 0x8b8beb34 M5L2764k
cc_p3.a4 8192 0xea342f87 M5L2764k
cc_p4.a5 8192 0x9cf35d98 M5L2764k
cc_p5.a7 8192 0xc5ef1eda M5L2764k
cc_p6.a8 8192 0x7b1dd7fc M5L2764k
cc_p7.h2 16384 0x029b5ea0 M5L27128k
cc_p8.f2 8192 0x0f0368ce M5L2764k
cc_p9.a2 16384 0xb58c5f24 M5L27128k
cc_p10.b2 16384 0x3c0a4eeb M5L27128k
cc_p11.c2 16384 0x4c8c2156 M5L27128k
cc_p12.j8 8192 0x9003a979 M5L2764k
cc_p13.j5 8192 0xa52cd549 M5L2764k
cc_p14.j2 8192 0xedc6a1eb M5L2764k

. Board supplied by Stefan Lindberg
. Board dumped by Stefan Lindberg

*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class mainsnk_state : public driver_device
{
public:
	mainsnk_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_bgram(*this, "bgram"),
		m_spriteram(*this, "spriteram"),
		m_fgram(*this, "fgram")
	{ }

	void mainsnk(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_bgram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_fgram;

	tilemap_t *m_tx_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;
	uint32_t m_bg_tile_offset = 0;

	uint8_t sound_ack_r();
	void c600_w(uint8_t data);
	void fgram_w(offs_t offset, uint8_t data);
	void bgram_w(offs_t offset, uint8_t data);

	TILEMAP_MAPPER_MEMBER(tx_scan_cols);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	void palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int scrollx, int scrolly);
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sound_portmap(address_map &map) ATTR_COLD;
};


void mainsnk_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	constexpr int num_colors = 0x400;

	for (int i = 0; i < num_colors; i++)
	{
		int bit0, bit1, bit2, bit3;

		bit0 = BIT(color_prom[i + 2*num_colors], 3);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		bit3 = BIT(color_prom[i], 3);
		int const r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = BIT(color_prom[i + 2*num_colors], 2);
		bit1 = BIT(color_prom[i + num_colors], 2);
		bit2 = BIT(color_prom[i + num_colors], 3);
		bit3 = BIT(color_prom[i], 0);
		int const g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = BIT(color_prom[i + 2*num_colors], 0);
		bit1 = BIT(color_prom[i + 2*num_colors], 1);
		bit2 = BIT(color_prom[i + num_colors], 0);
		bit3 = BIT(color_prom[i + num_colors], 1);
		int const b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

TILEMAP_MAPPER_MEMBER(mainsnk_state::tx_scan_cols)
{
	// tilemap is 36x28, the central part is from the first RAM page and the
	// extra 4 columns are from the second page
	col -= 2;
	if (col & 0x20)
		return 0x400 + row + ((col & 0x1f) << 5);
	else
		return row + (col << 5);
}

TILE_GET_INFO_MEMBER(mainsnk_state::get_tx_tile_info)
{
	int code = m_fgram[tile_index];

	tileinfo.set(0,
			code,
			0,
			tile_index & 0x400 ? TILE_FORCE_LAYER0 : 0);
}

TILE_GET_INFO_MEMBER(mainsnk_state::get_bg_tile_info)
{
	int code = (m_bgram[tile_index]);

	tileinfo.set(0,
			m_bg_tile_offset + code,
			0,
			0);
}


void mainsnk_state::video_start()
{
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mainsnk_state::get_tx_tile_info)), tilemap_mapper_delegate(*this, FUNC(mainsnk_state::tx_scan_cols)), 8, 8, 36, 28);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mainsnk_state::get_bg_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 32, 32);

	m_tx_tilemap->set_transparent_pen(15);
	m_tx_tilemap->set_scrolldy(8, 8);

	m_bg_tilemap->set_scrolldx(16, 16);
	m_bg_tilemap->set_scrolldy(8,  8);

	save_item(NAME(m_bg_tile_offset));
}


void mainsnk_state::c600_w(uint8_t data)
{
	int total_elements = m_gfxdecode->gfx(0)->elements();

	flip_screen_set(BIT(data, 7));

	m_bg_tilemap->set_palette_offset((data & 0x07) << 4);
	m_tx_tilemap->set_palette_offset((data & 0x07) << 4);

	int bank = 0;
	if (total_elements == 0x400)    // mainsnk
		bank = ((data & 0x30) >> 4);
	else if (total_elements == 0x800)   // canvas
		bank = ((data & 0x40) >> 6) | ((data & 0x30) >> 3);

	if (m_bg_tile_offset != (bank << 8))
	{
		m_bg_tile_offset = bank << 8;
		m_bg_tilemap->mark_all_dirty();
	}
}

void mainsnk_state::fgram_w(offs_t offset, uint8_t data)
{
	m_fgram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset);
}

void mainsnk_state::bgram_w(offs_t offset, uint8_t data)
{
	m_bgram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}



void mainsnk_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int scrollx, int scrolly)
{
	gfx_element *gfx = m_gfxdecode->gfx(1);
	const uint8_t *source = m_spriteram;
	const uint8_t *finish = source + 25 * 4;

	while (source < finish)
	{
		int attributes = source[3];
		int tile_number = source[1];
		int sy = source[0];
		int sx = source[2];
		int color = attributes & 0xf;
		int flipx = 0;
		int flipy = 0;
		if (sy > 240) sy -= 256;

		tile_number |= attributes << 4 & 0x300;

		sx = 288 - 16 - sx;
		sy += 8;

		if (flip_screen())
		{
			sx = 288 - 16 - sx;
			sy = 224 + 8 - 16 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		gfx->transpen(bitmap, cliprect,
			tile_number,
			color,
			flipx, flipy,
			sx, sy, 7);

		source += 4;
	}
}


uint32_t mainsnk_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect, 0, 0);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


uint8_t mainsnk_state::sound_ack_r()
{
	m_audiocpu->set_input_line(0, CLEAR_LINE);
	return 0xff;
}



void mainsnk_state::main_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc000).portr("IN0");
	map(0xc100, 0xc100).portr("IN1");
	map(0xc200, 0xc200).portr("IN2");
	map(0xc300, 0xc300).portr("IN3");
	map(0xc400, 0xc400).portr("DSW1");
	map(0xc500, 0xc500).portr("DSW2");
	map(0xc600, 0xc600).w(FUNC(mainsnk_state::c600_w));
	map(0xc700, 0xc700).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xd800, 0xdbff).ram().w(FUNC(mainsnk_state::bgram_w)).share(m_bgram);
	map(0xdc00, 0xe7ff).ram();
	map(0xe800, 0xefff).ram().share(m_spriteram);
	map(0xf000, 0xf7ff).ram().w(FUNC(mainsnk_state::fgram_w)).share(m_fgram);    // + work RAM
}

void mainsnk_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xa000, 0xa000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xc000, 0xc000).r(m_soundlatch, FUNC(generic_latch_8_device::acknowledge_r));
	map(0xe000, 0xe001).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0xe002, 0xe003).nopw();    // ? always FFFF, snkwave leftover?
	map(0xe008, 0xe009).w("ay2", FUNC(ay8910_device::address_data_w));
}

void mainsnk_state::sound_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(FUNC(mainsnk_state::sound_ack_r));
}



static INPUT_PORTS_START( mainsnk )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("soundlatch", generic_latch_8_device, pending_r)  // sound CPU status
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_SERVICE )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_PLAYER(2)

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )    // button on top of left stick
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )    // button on top of right stick
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )    // left button
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )    // right button
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	// the manual only mentions one dip switch apparently.
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )    // duplicate
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )    // duplicate
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x10, 0x10, "Round Time" ) PORT_DIPLOCATION("DSW2:5") // $1ecf
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Short" )
	PORT_DIPNAME( 0x60, 0x20, "Game mode" ) PORT_DIPLOCATION("DSW2:6,7")
	PORT_DIPSETTING(    0x60, "Demo Sounds Off" )
	PORT_DIPSETTING(    0x20, "Demo Sounds On" )
	PORT_DIPSETTING(    0x00, "Freeze" )
	PORT_DIPSETTING(    0x40, "Endless Game (Cheat)" )
	PORT_DIPNAME( 0x80, 0x80, "2 Players Game" ) PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, "1 Credit" )
	PORT_DIPSETTING(    0x00, "2 Credits" )
INPUT_PORTS_END

static INPUT_PORTS_START( canvas )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("soundlatch", generic_latch_8_device, pending_r)  // sound CPU status
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_SERVICE )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) ) PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DSW1:4,5,6")
	PORT_DIPSETTING(    0x08, DEF_STR( 5C_1C ) )    // duplicate
	PORT_DIPSETTING(    0x10, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:7")   // bonus life?
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:8")   // bonus life?
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:1")   // bonus life?
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:2")   // difficulty?
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:3")   // difficulty?
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x10, "Game mode" ) PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(    0x18, "Demo Sounds Off" )
	PORT_DIPSETTING(    0x10, "Demo Sounds On" )
	PORT_DIPSETTING(    0x00, "Freeze" )
	PORT_DIPSETTING(    0x08, "Infinite Lives (Cheat)" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Must Be On" ) PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )   // needs to be on otherwise pictures in later levels are wrong
INPUT_PORTS_END


static const gfx_layout sprite_layout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3),RGN_FRAC(1,3),RGN_FRAC(0,3) },
	{ 7,6,5,4,3,2,1,0, 15,14,13,12,11,10,9,8 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	256
};


static GFXDECODE_START( gfx_mainsnk )
	GFXDECODE_ENTRY( "tiles",   0, gfx_8x8x4_packed_lsb,   0x100, 0x080 >> 4 )
	GFXDECODE_ENTRY( "sprites", 0, sprite_layout,          0x000, 0x080 >> 3 )
GFXDECODE_END



void mainsnk_state::mainsnk(machine_config &config)
{
	Z80(config, m_maincpu, 3360000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mainsnk_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(mainsnk_state::irq0_line_hold));

	Z80(config, m_audiocpu, 4000000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &mainsnk_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &mainsnk_state::sound_portmap);
	m_audiocpu->set_periodic_int(FUNC(mainsnk_state::irq0_line_assert), attotime::from_hz(244));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(36*8, 28*8);
	screen.set_visarea(0*8, 36*8-1, 1*8, 28*8-1);
	screen.set_screen_update(FUNC(mainsnk_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mainsnk);
	PALETTE(config, m_palette, FUNC(mainsnk_state::palette), 0x400);
	m_palette->enable_shadows();

	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	m_soundlatch->set_separate_acknowledge(true);

	AY8910(config, "ay1", 2000000).add_route(ALL_OUTPUTS, "mono", 0.35);
	AY8910(config, "ay2", 2000000).add_route(ALL_OUTPUTS, "mono", 0.35);
}


ROM_START(mainsnk)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "snk.p01",      0x0000, 0x2000, CRC(00db1ca2) SHA1(efe83488cf88adc185e6024b8f6ad5f8ef7f4cfd) )
	ROM_LOAD( "snk.p02",      0x2000, 0x2000, CRC(df5c86b5) SHA1(e9c854524e3d8231c874314cdff321e66ec7f0c4) )
	ROM_LOAD( "snk.p03",      0x4000, 0x2000, CRC(5c2b7bca) SHA1(e02c72fcd029999b730abd91f07866418cfe6216) )
	ROM_LOAD( "snk.p04",      0x6000, 0x2000, CRC(68b4b2a1) SHA1(8f3abc826df93f0748151624066e956b9670bc9d) )
	ROM_LOAD( "snk.p05",      0x8000, 0x2000, CRC(580a29b4) SHA1(4a96af92d65f86aca7f3a70032b5e4dc29048483) )
	ROM_LOAD( "snk.p06",      0xa000, 0x2000, CRC(5f8a60a2) SHA1(88a051e13d6b3bbd3606a4c4cc0395da07e0f109) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "snk.p07",         0x0000, 0x4000, CRC(4208391e) SHA1(d110ca4ff9d21fe7813f04ec43c2c23471c6517f) )

	ROM_REGION( 0x08000, "tiles", 0 )
	ROM_LOAD( "snk.p12",      0x0000, 0x2000, CRC(ecf87eb7) SHA1(83b8d19070d5930b306a0309ebba05b04c2abebf) )
	ROM_LOAD( "snk.p11",      0x2000, 0x2000, CRC(3f6bc5ba) SHA1(02e49f58f5d94117113b59037fa49b8897d05b4b) )
	ROM_LOAD( "snk.p10",      0x4000, 0x2000, CRC(b5147a96) SHA1(72641fadabd16f2de4f4cf6ff3ef07233de5ddfd) )
	ROM_LOAD( "snk.p09",      0x6000, 0x2000, CRC(0ebcf837) SHA1(7b93cdffd3b8d768b98bb01956114e4ff012d029) )

	ROM_REGION( 0x12000, "sprites", 0 )
	ROM_LOAD( "snk.p13",      0x00000, 0x2000, CRC(2eb624a4) SHA1(157d7beb6ff0baa9276e388774a85996dc03821d) )
	ROM_LOAD( "snk.p16",      0x02000, 0x2000, CRC(dc502869) SHA1(024c868e8cd74c52f4787a19b9ad292b7a9dcc1c) )
	ROM_LOAD( "snk.p19",      0x04000, 0x2000, CRC(58d566a1) SHA1(1451b223ddb7c975b770f28af6c41775daaf95c1) )
	ROM_LOAD( "snk.p14",      0x06000, 0x2000, CRC(bb927d82) SHA1(ac7ae1850cf22b73e31c92b6f598fb057470a570) )
	ROM_LOAD( "snk.p17",      0x08000, 0x2000, CRC(66f60c32) SHA1(7a08d0a2c1804cdaad702a23ff33128d0b6d8084) )
	ROM_LOAD( "snk.p20",      0x0a000, 0x2000, CRC(d12c6333) SHA1(bed1a0aedaa8f6fe9c33f49b5da00ab1c9045ddd) )
	ROM_LOAD( "snk.p15",      0x0c000, 0x2000, CRC(d242486d) SHA1(0c24a3fdcb604b6231b75069c99009d68023bb8f) )
	ROM_LOAD( "snk.p18",      0x0e000, 0x2000, CRC(838b12a3) SHA1(a3444f9b2aeef70caa93e5f642cb6c3b75e88ea4) )
	ROM_LOAD( "snk.p21",      0x10000, 0x2000, CRC(8961a51e) SHA1(4f9d8358bc76118c4fab631ae73a02ab5aa0c036) )

	ROM_REGION( 0x1000, "proms", 0 )    // overdumps? 2nd half is empty
	ROM_LOAD( "main3.bin",    0x0000, 0x0800, CRC(78b29dde) SHA1(c2f93cde6fd8bc175e9e0d38af41b7710d7f1c82) )
	ROM_LOAD( "main2.bin",    0x0400, 0x0800, CRC(7c314c93) SHA1(c6bd2a0eaf617448ef65dcbadced313b0d69ab88) )
	ROM_LOAD( "main1.bin",    0x0800, 0x0800, CRC(deb895c4) SHA1(f1281dcb3471d9627565706ff09ba72f09dc62a4) )
ROM_END

ROM_START( canvas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cc_p1.a2",      0x0000, 0x2000, CRC(fa7109e1) SHA1(23e31e14af2171ee2fd7290194805b95b0f7b35c) )
	ROM_LOAD( "cc_p2.a3",      0x2000, 0x2000, CRC(8b8beb34) SHA1(c678ed7ec302eaac3594950f10f0a170353345e5) )
	ROM_LOAD( "cc_p3.a4",      0x4000, 0x2000, CRC(ea342f87) SHA1(087e1260ba51bf47bf19942b59d21d067515989d) )
	ROM_LOAD( "cc_p4.a5",      0x6000, 0x2000, CRC(9cf35d98) SHA1(08de7863f1a540b69487c87eb0a493ceeacffa1b) )
	ROM_LOAD( "cc_p5.a7",      0x8000, 0x2000, CRC(c5ef1eda) SHA1(31cf3e7fe52718bebffdac9b3666454b0956a6d9) )
	ROM_LOAD( "cc_p6.a8",      0xa000, 0x2000, CRC(7b1dd7fc) SHA1(1287ab261885d5e9ba957024d7a00c7a0d31235b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "cc_p7.h2",      0x0000, 0x4000, CRC(029b5ea0) SHA1(88f84b4dd01656ded8d983396ded404c9d8186f1) )
	ROM_LOAD( "cc_p8.f2",      0x4000, 0x2000, CRC(0f0368ce) SHA1(a02f066ea024285a931b85709822a50a4099e0b0) )

	ROM_REGION( 0x10000, "tiles", 0 )
	ROM_FILL(                  0x0000, 0x4000, 0xff )   // empty, causes tx layer to be fully transparent
	ROM_LOAD( "cc_p11.c2",     0x4000, 0x4000, CRC(4c8c2156) SHA1(7f1d9a1e1c6cab91f24c7fc75d0c7ec2702137af) )   // banks = 18&58
	ROM_LOAD( "cc_p10.b2",     0x8000, 0x4000, CRC(3c0a4eeb) SHA1(53742a5bef16e71bebefb0e43a175341f5bf0aa6) )   // banks = 28&68
	ROM_LOAD( "cc_p9.a2",      0xc000, 0x4000, CRC(b58c5f24) SHA1(7026b3d4f8060fd6607eb6d356d6b61cc9cb75c3) )   // banks = 30&70

	ROM_REGION( 0x6000, "sprites", 0 )
	ROM_LOAD( "cc_p12.j8",     0x0000, 0x2000, CRC(9003a979) SHA1(f63959a9dc9ee67622865e783d2e501c640a4bed) )
	ROM_LOAD( "cc_p13.j5",     0x2000, 0x2000, CRC(a52cd549) SHA1(1902b8c107c5156113068ced74349ac576ac047c) )
	ROM_LOAD( "cc_p14.j2",     0x4000, 0x2000, CRC(edc6a1e8) SHA1(8c948a5f057e13bb9ed9738b66c702f45586fe59) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "cc_bprom3.j8",  0x0000, 0x0400, CRC(21f72498) SHA1(a586c869cb4633fec0df92b5646ece78f99b6f2a) )
	ROM_LOAD( "cc_bprom2.j9",  0x0400, 0x0400, CRC(19efe7df) SHA1(7e49af8b8b01fb929b87d6285da32fbe4c58606d) )
	ROM_LOAD( "cc_bprom1.j10", 0x0800, 0x0400, CRC(fbbbf911) SHA1(86394a7f67bc4f89f72b9607ca3733ab3d690289) )
ROM_END

} // anonymous namespace


GAME( 1984, mainsnk, 0, mainsnk, mainsnk, mainsnk_state, empty_init, ROT180, "SNK", "Main Event (1984)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, canvas,  0, mainsnk, canvas,  mainsnk_state, empty_init, ROT0,   "SNK", "Canvas Croquis",    MACHINE_SUPPORTS_SAVE )
