// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

Sauro
-----

driver by Zsolt Vasvari

Main CPU
--------

Memory mapped:

0000-dfff   ROM
e000-e7ff   RAM, battery backed
e800-ebff   Sprite RAM
f000-f3ff   Background Video RAM
f400-f7ff   Background Color RAM
f800-fbff   Foreground Video RAM
fc00-ffff   Foreground Color RAM

Ports:

00      R   DSW #1
20      R   DSW #2
40      R   Input Ports Player 1
60      R   Input Ports Player 2
80       W  Sound Command
c0       W  Flip Screen
c1       W  ???
c2-c4    W  ???
c6-c7    W  ??? (Loads the sound latch?)
c8       W  ???
c9       W  ???
ca-cd    W  ???
ce       W  ???
e0       W  Watchdog


Sound CPU
---------

Memory mapped:

0000-7fff       ROM
8000-87ff       RAM
a000         W  ADPCM trigger
c000-c001    W  YM3812
e000        R   Sound latch
e000-e006    W  ???
e00e-e00f    W  ???


TODO
----

- I'm only using colors 0-15. The other 3 banks are mostly the same, but,
  for example, the color that's used to paint the gradients of the sky (color 2)
  is different, so there might be a palette select. I don't see anything
  obviously wrong the way it is right now. It matches the screen shots found
  on the Spanish Dump site.

- What do the rest of the ports in the range c0-ce do?

Tricky Doc
----------

Addition by Reip


Stephh's notes (based on the games Z80 code and some tests) :

1) 'sauro'

  - Press START1 while in "test mode" to cycle through different screens
    (colors, Dip Switches, Inputs)
  - When "Freeze" Dip Switch is ON, press START1 to freeze and START2 to unfreeze.
    This setting (as well as others) must be defined before resetting the games.
  - On 'sauroa', "Test mode" crashes when trying to display "Difficult" ("Hard")
    because the full string is 15 bytes long while other string are 14, so the 15th
    "char" is NOT 0x00 :
      * 0xd49f : mask (0x30)
      * 0xd4a0-0xd4a7 : offset of settings to display (4 x 2 bytes, LSB first) :
        0xd58e, 0xd5a5, 0xd5bc, 0xd5d4
    On 'sauro' (the parent set), the "Test mode" works fine and displays the
    "Difficult" string.
  - Player 2 uses player 2 inputs only when "Cabinet" Dip Switch is set to "Cocktail"
    (code at 0x2e40 : start reading inputs).

2) 'trckydoc' and clones

  - Press START1 while in "test mode" to cycle through different screens
    (colors, Dip Switches, Inputs)
  - When "Freeze" Dip Switch is ON, press START1 to freeze and START2 to unfreeze.
    This setting (as well as others) must be defined before resetting the games.

2a) 'trckydoc'

  - Settings are the SAME as in 'sauro', but there is NO bug in the "test mode" :
      * 0xcd19 : mask (0x30)
      * 0xcd1a-0xcd22 : offset of settings to display (4 x 2 bytes, LSB first) :
        0xce0a, 0xce21, 0xce38, 0xce4f
  - Player 2 uses player 2 inputs only when "Cabinet" Dip Switch is set to "Cocktail"
    (extra code at 0xdf10 - code at 0x3f80 : start reading inputs).
  - You can't get any extra life nor extra credit.

2b) 'trckydoca'

  - Coinage B is slightly different : you have 1C_1C instead of 1C_5C (table at 0x02e1).
    Such change isn't notified in the "test mode" though.
  - Player 2 uses player 2 inputs regardless of "Cabinet" Dip Switch
    (NO extra code at 0xdf10 - code at 0x3f80 : start reading inputs).
  - You can get an extra life at 90000 points and an extra credit at 500000 points
    but there is no music/sound to tell that to you (extra code at 0xdf30).
    This is only possible if you continue a game and have already got the life or
    the credit (extra code at 0xdf90 resets the flags if you don't continue).
    This info is written in "attract mode" when you don't have any credits
    instead of displaying the "INSERT COIN" message.

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "machine/watchdog.h"
#include "sound/sp0256.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class base_state : public driver_device
{
public:
	base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_mainlatch(*this, "mainlatch"),
		m_spriteram(*this, "spriteram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_bg_colorram(*this, "bg_colorram")
	{ }

	void tecfri(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<ls259_device> m_mainlatch;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_bg_videoram;
	required_shared_ptr<uint8_t> m_bg_colorram;

	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_palette_bank = 0U;

	bool m_irq_enable = 0;

	// common
	void vblank_irq(int state);
	void irq_reset_w(int state);
	template <uint8_t Which> void coin_w(int state);
	void bg_videoram_w(offs_t offset, uint8_t data);
	void bg_colorram_w(offs_t offset, uint8_t data);
	void scroll_bg_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_tile_info_bg);
};

class trckydoc_state : public base_state
{
public:
	trckydoc_state(const machine_config &mconfig, device_type type, const char *tag) :
		base_state(mconfig, type, tag)
	{ }

	void trckydoc(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void prg_map(address_map &map) ATTR_COLD;
};

class sauro_state : public base_state
{
public:
	sauro_state(const machine_config &mconfig, device_type type, const char *tag) :
		base_state(mconfig, type, tag),
		m_soundlatch(*this, "soundlatch"),
		m_fg_videoram(*this, "fg_videoram"),
		m_fg_colorram(*this, "fg_colorram")
	{ }

	void sauro(machine_config &config);
	void saurobl(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_fg_videoram;
	required_shared_ptr<uint8_t> m_fg_colorram;

	tilemap_t *m_fg_tilemap = nullptr;

	void sound_command_w(uint8_t data);
	uint8_t sound_command_r();
	void palette_bank0_w(int state);
	void palette_bank1_w(int state);
	void scroll_fg_w(uint8_t data);
	void fg_videoram_w(offs_t offset, uint8_t data);
	void fg_colorram_w(offs_t offset, uint8_t data);

	TILE_GET_INFO_MEMBER(get_tile_info_fg);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_io_map(address_map &map) ATTR_COLD;
	void main_prg_map(address_map &map) ATTR_COLD;
	void sauro_sound_map(address_map &map) ATTR_COLD;
	void saurobl_sound_map(address_map &map) ATTR_COLD;
};


// General

void base_state::bg_videoram_w(offs_t offset, uint8_t data)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void base_state::bg_colorram_w(offs_t offset, uint8_t data)
{
	m_bg_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void sauro_state::fg_videoram_w(offs_t offset, uint8_t data)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void sauro_state::fg_colorram_w(offs_t offset, uint8_t data)
{
	m_fg_colorram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void base_state::scroll_bg_w(uint8_t data)
{
	m_bg_tilemap->set_scrollx(0, data);
}

TILE_GET_INFO_MEMBER(base_state::get_tile_info_bg)
{
	int const code = m_bg_videoram[tile_index] + ((m_bg_colorram[tile_index] & 0x07) << 8);
	int const color = ((m_bg_colorram[tile_index] >> 4) & 0x0f) | m_palette_bank;
	int const flags = m_bg_colorram[tile_index] & 0x08 ? TILE_FLIPX : 0;

	tileinfo.set(0, code, color, flags);
}

TILE_GET_INFO_MEMBER(sauro_state::get_tile_info_fg)
{
	int const code = m_fg_videoram[tile_index] + ((m_fg_colorram[tile_index] & 0x07) << 8);
	int const color = ((m_fg_colorram[tile_index] >> 4) & 0x0f) | m_palette_bank;
	int const flags = m_fg_colorram[tile_index] & 0x08 ? TILE_FLIPX : 0;

	tileinfo.set(1, code, color, flags);
}

// Sauro

static const int scroll2_map[8] = {2, 1, 4, 3, 6, 5, 0, 7};
static const int scroll2_map_flip[8] = {0, 7, 2, 1, 4, 3, 6, 5};

void sauro_state::palette_bank0_w(int state)
{
	if (state)
		m_palette_bank |= 0x10;
	else
		m_palette_bank &= ~0x10;
	machine().tilemap().mark_all_dirty();
}

void sauro_state::palette_bank1_w(int state)
{
	if (state)
		m_palette_bank |= 0x20;
	else
		m_palette_bank &= ~0x20;
	machine().tilemap().mark_all_dirty();
}

void sauro_state::scroll_fg_w(uint8_t data)
{
	const int *map = (flip_screen() ? scroll2_map_flip : scroll2_map);
	int const scroll = (data & 0xf8) | map[data & 7];

	m_fg_tilemap->set_scrollx(0, scroll);
}

void sauro_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sauro_state::get_tile_info_bg)), TILEMAP_SCAN_COLS,
			8, 8, 32, 32);

	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sauro_state::get_tile_info_fg)), TILEMAP_SCAN_COLS,
			8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
	m_palette_bank = 0;

	save_item(NAME(m_palette_bank));
}

void sauro_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int const flipy = flip_screen();

	for (int offs = 3; offs < m_spriteram.bytes() - 1; offs += 4)
	{
		int sy = m_spriteram[offs];
		if (sy == 0xf8) continue;

		int const code = m_spriteram[offs + 1] + ((m_spriteram[offs + 3] & 0x03) << 8);
		int sx = m_spriteram[offs + 2];
		sy = 236 - sy;
		int const color = ((m_spriteram[offs + 3] >> 4) & 0x0f) | m_palette_bank;

		// I'm not really sure how this bit works
		if (m_spriteram[offs + 3] & 0x08)
		{
			if (sx > 0xc0)
			{
				// Sign extend
				sx = (signed int)(signed char)sx;
			}
		}
		else
		{
			if (sx < 0x40) continue;
		}

		int flipx = m_spriteram[offs + 3] & 0x04;

		if (flipy)
		{
			flipx = !flipx;
			sx = (235 - sx) & 0xff;  // The &0xff is not 100% percent correct
			sy = 240 - sy;
		}

		m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,
				code,
				color,
				flipx, flipy,
				sx, sy, 0);
	}
}

uint32_t sauro_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}

// Tricky Doc

void trckydoc_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(trckydoc_state::get_tile_info_bg)), TILEMAP_SCAN_COLS,
			8, 8, 32, 32);

	m_palette_bank = 0;
}

void trckydoc_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int const flipy = flip_screen();

	// Weird, sprites entries don't start on DWORD boundary
	for (int offs = 3; offs < m_spriteram.bytes() - 1; offs += 4)
	{
		int sy = m_spriteram[offs];

		if (m_spriteram[offs + 3] & 0x08)
		{
			// needed by the elevator cable (2nd stage), balls bouncing (3rd stage) and maybe other things
			sy += 6;
		}

		int const code = m_spriteram[offs + 1] + ((m_spriteram[offs + 3] & 0x01) << 8);

		int sx = m_spriteram[offs + 2] - 2;
		int const color = (m_spriteram[offs + 3] >> 4) & 0x0f;

		sy = 236 - sy;

		// similar to sauro but different bit is used ..
		if (m_spriteram[offs + 3] & 0x02)
		{
			if (sx > 0xc0)
			{
				// Sign extend
				sx = (signed int)(signed char)sx;
			}
		}
		else
		{
			if (sx < 0x40) continue;
		}

		int flipx = m_spriteram[offs + 3] & 0x04;

		if (flipy)
		{
			flipx = !flipx;
			sx = (235 - sx) & 0xff;  // The &0xff is not 100% percent correct
			sy = 240 - sy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
				code,
				color,
				flipx, flipy,
				sx, sy, 0);
	}
}

uint32_t trckydoc_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}


void base_state::machine_start()
{
	save_item(NAME(m_irq_enable));
}

void sauro_state::sound_command_w(uint8_t data)
{
	data |= 0x80;
	m_soundlatch->write(data);
}

uint8_t sauro_state::sound_command_r()
{
	int ret = m_soundlatch->read();
	m_soundlatch->clear_w();
	return ret;
}

void base_state::vblank_irq(int state)
{
	if (state && m_irq_enable)
		m_maincpu->set_input_line(0, ASSERT_LINE);
}

void base_state::irq_reset_w(int state)
{
	m_irq_enable = !state;
	if (m_irq_enable)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

template <uint8_t Which>
void base_state::coin_w(int state)
{
	machine().bookkeeping().coin_counter_w(Which, state);
}

void sauro_state::main_prg_map(address_map &map)
{
	map(0x0000, 0xdfff).rom();
	map(0xe000, 0xe7ff).ram().share("nvram");
	map(0xe800, 0xebff).ram().share(m_spriteram);
	map(0xf000, 0xf3ff).ram().w(FUNC(sauro_state::bg_videoram_w)).share(m_bg_videoram);
	map(0xf400, 0xf7ff).ram().w(FUNC(sauro_state::bg_colorram_w)).share(m_bg_colorram);
	map(0xf800, 0xfbff).ram().w(FUNC(sauro_state::fg_videoram_w)).share(m_fg_videoram);
	map(0xfc00, 0xffff).ram().w(FUNC(sauro_state::fg_colorram_w)).share(m_fg_colorram);
}

void sauro_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("DSW1");
	map(0x20, 0x20).portr("DSW2");
	map(0x40, 0x40).portr("P1");
	map(0x60, 0x60).portr("P2");
	map(0x80, 0x80).w(FUNC(sauro_state::sound_command_w));
	map(0xa0, 0xa0).w(FUNC(sauro_state::scroll_bg_w));
	map(0xa1, 0xa1).w(FUNC(sauro_state::scroll_fg_w));
	map(0xc0, 0xcf).w(m_mainlatch, FUNC(ls259_device::write_a0));
	map(0xe0, 0xe0).w("watchdog", FUNC(watchdog_timer_device::reset_w));
}

void sauro_state::sauro_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xc000, 0xc001).w("ymsnd", FUNC(ym3812_device::write));
	map(0xa000, 0xa000).w("speech", FUNC(sp0256_device::ald_w));
	map(0xe000, 0xe000).r(FUNC(sauro_state::sound_command_r));
	map(0xe000, 0xe006).nopw();    // echo from write to e0000
	map(0xe00e, 0xe00f).nopw();
}


void sauro_state::saurobl_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xc000, 0xc001).w("ymsnd", FUNC(ym3812_device::write));
	map(0xa000, 0xa000).nopw();
	map(0xe000, 0xe000).r(FUNC(sauro_state::sound_command_r));
	map(0xe000, 0xe006).nopw();    // echo from write to e0000
	map(0xe00e, 0xe00f).nopw();
}


void trckydoc_state::prg_map(address_map &map)
{
	map(0x0000, 0xdfff).rom();
	map(0xe000, 0xe7ff).ram().share("nvram");
	map(0xe800, 0xebff).ram().mirror(0x400).share(m_spriteram);
	map(0xf000, 0xf3ff).ram().w(FUNC(trckydoc_state::bg_videoram_w)).share(m_bg_videoram);
	map(0xf400, 0xf7ff).ram().w(FUNC(trckydoc_state::bg_colorram_w)).share(m_bg_colorram);
	map(0xf800, 0xf800).portr("DSW1");
	map(0xf808, 0xf808).portr("DSW2");
	map(0xf810, 0xf810).portr("P1");
	map(0xf818, 0xf818).portr("P2");
	map(0xf820, 0xf821).w("ymsnd", FUNC(ym3812_device::write));
	map(0xf828, 0xf828).r("watchdog", FUNC(watchdog_timer_device::reset_r));
	map(0xf830, 0xf830).w(FUNC(trckydoc_state::scroll_bg_w));
	map(0xf838, 0xf83f).w(m_mainlatch, FUNC(ls259_device::write_d0));
}


// Verified from Z80 code
static INPUT_PORTS_START( tecfri )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY

	PORT_START("P2")                                                  // See notes
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL

	PORT_START("DSW1")
	PORT_SERVICE( 0x01, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x20, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )            // This crashes test mode in 'sauro' but not in other games !!! - see notes
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, "Freeze" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x30, 0x20, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_HIGH )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_HIGH )
INPUT_PORTS_END


// Verified from Z80 code
static INPUT_PORTS_START( trckydoca )
	PORT_INCLUDE(tecfri)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
INPUT_PORTS_END


static INPUT_PORTS_START( saurobl )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN )  PORT_8WAY

	PORT_START("P2")                                                  // See notes
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL

	PORT_START("DSW1")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )             // This crashes test mode in 'sauro' but not in other games !!! - see notes
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Easy ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "5" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,    // 8*8 chars
	2048,   // 2048 characters
	4,      // 4 bits per pixel
	{ 0,1,2,3 },  // The 4 planes are packed together
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4},
	{ 0*4*8, 1*4*8, 2*4*8, 3*4*8, 4*4*8, 5*4*8, 6*4*8, 7*4*8},
	8*8*4     // Every char takes 32 consecutive bytes
};

static const gfx_layout trckydoc_spritelayout =
{
	16,16,  // 16*16 sprites
	512,    // 512 sprites
	4,      // 4 bits per pixel
	{ 0,1,2,3 },  // The 4 planes are packed together
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4, 9*4, 8*4, 11*4, 10*4, 13*4, 12*4, 15*4, 14*4},
	{ RGN_FRAC(3,4)+0*4*16, RGN_FRAC(2,4)+0*4*16, RGN_FRAC(1,4)+0*4*16, RGN_FRAC(0,4)+0*4*16,
		RGN_FRAC(3,4)+1*4*16, RGN_FRAC(2,4)+1*4*16, RGN_FRAC(1,4)+1*4*16, RGN_FRAC(0,4)+1*4*16,
		RGN_FRAC(3,4)+2*4*16, RGN_FRAC(2,4)+2*4*16, RGN_FRAC(1,4)+2*4*16, RGN_FRAC(0,4)+2*4*16,
		RGN_FRAC(3,4)+3*4*16, RGN_FRAC(2,4)+3*4*16, RGN_FRAC(1,4)+3*4*16, RGN_FRAC(0,4)+3*4*16 },
	16*16     // Every sprite takes 32 consecutive bytes
};

static const gfx_layout sauro_spritelayout =
{
	16,16,  // 16*16 sprites
	1024,   // 1024 sprites
	4,      // 4 bits per pixel
	{ 0,1,2,3 },  // The 4 planes are packed together
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4, 9*4, 8*4, 11*4, 10*4, 13*4, 12*4, 15*4, 14*4},
	{ RGN_FRAC(3,4)+0*4*16, RGN_FRAC(2,4)+0*4*16, RGN_FRAC(1,4)+0*4*16, RGN_FRAC(0,4)+0*4*16,
		RGN_FRAC(3,4)+1*4*16, RGN_FRAC(2,4)+1*4*16, RGN_FRAC(1,4)+1*4*16, RGN_FRAC(0,4)+1*4*16,
		RGN_FRAC(3,4)+2*4*16, RGN_FRAC(2,4)+2*4*16, RGN_FRAC(1,4)+2*4*16, RGN_FRAC(0,4)+2*4*16,
		RGN_FRAC(3,4)+3*4*16, RGN_FRAC(2,4)+3*4*16, RGN_FRAC(1,4)+3*4*16, RGN_FRAC(0,4)+3*4*16 },
	16*16     // Every sprite takes 32 consecutive bytes
};

static GFXDECODE_START( gfx_sauro )
	GFXDECODE_ENTRY( "bgtiles", 0, charlayout, 0, 64 )
	GFXDECODE_ENTRY( "fgtiles", 0, charlayout, 0, 64 )
	GFXDECODE_ENTRY( "sprites", 0, sauro_spritelayout, 0, 64 )
GFXDECODE_END

static GFXDECODE_START( gfx_trckydoc )
	GFXDECODE_ENTRY( "bgtiles", 0, charlayout, 0, 64 )
	GFXDECODE_ENTRY( "sprites", 0, trckydoc_spritelayout, 0, 64 )
GFXDECODE_END


void base_state::tecfri(machine_config &config)
{
	// Basic machine hardware
	Z80(config, m_maincpu, XTAL(20'000'000) / 4);       // Verified on PCB

	LS259(config, m_mainlatch);
	m_mainlatch->q_out_cb<4>().set(FUNC(base_state::irq_reset_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	WATCHDOG_TIMER(config, "watchdog");

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(55.72);   // Verified on PCB
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(5000));  // frames per second, vblank duration (otherwise sprites lag)
	screen.set_size(32 * 8, 32 * 8);
	screen.set_visarea(1 * 8, 31 * 8 - 1, 2 * 8, 30 * 8 - 1);
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(base_state::vblank_irq));

	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 1024);

	// Sound hardware
	SPEAKER(config, "mono").front_center();

	YM3812(config, "ymsnd", XTAL(20'000'000) / 8).add_route(ALL_OUTPUTS, "mono", 1.0);       // Verified on PCB
}

void trckydoc_state::trckydoc(machine_config &config)
{
	tecfri(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &trckydoc_state::prg_map);

	m_mainlatch->q_out_cb<1>().set(FUNC(trckydoc_state::flip_screen_set));
	m_mainlatch->q_out_cb<2>().set(FUNC(trckydoc_state::coin_w<0>));
	m_mainlatch->q_out_cb<3>().set(FUNC(trckydoc_state::coin_w<1>));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_trckydoc);

	subdevice<screen_device>("screen")->set_screen_update(FUNC(trckydoc_state::screen_update));
}

void sauro_state::saurobl(machine_config &config)
{
	tecfri(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &sauro_state::main_prg_map);
	m_maincpu->set_addrmap(AS_IO, &sauro_state::main_io_map);

	// Z3
	m_mainlatch->q_out_cb<0>().set(FUNC(sauro_state::flip_screen_set));
	m_mainlatch->q_out_cb<1>().set(FUNC(sauro_state::coin_w<0>));
	m_mainlatch->q_out_cb<2>().set(FUNC(sauro_state::coin_w<1>));
	m_mainlatch->q_out_cb<3>().set_nop(); // sound IRQ trigger?
	m_mainlatch->q_out_cb<5>().set(FUNC(sauro_state::palette_bank0_w));
	m_mainlatch->q_out_cb<6>().set(FUNC(sauro_state::palette_bank1_w));

	z80_device &audiocpu(Z80(config, "audiocpu", XTAL(20'000'000) / 5));     // Verified on PCB
	audiocpu.set_addrmap(AS_PROGRAM, &sauro_state::saurobl_sound_map);
	audiocpu.set_periodic_int(FUNC(sauro_state::irq0_line_hold), attotime::from_hz(8 * 60)); // ?

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sauro);

	subdevice<screen_device>("screen")->set_screen_update(FUNC(sauro_state::screen_update));

	GENERIC_LATCH_8(config, m_soundlatch);

	subdevice<ym3812_device>("ymsnd")->set_clock(XTAL(20'000'000) / 5);     // Verified on PCB
}

void sauro_state::sauro(machine_config &config)
{
	saurobl(config);

	subdevice<z80_device>("audiocpu")->set_addrmap(AS_PROGRAM, &sauro_state::sauro_sound_map);

	// Sound hardware
	sp0256_device &sp0256(SP0256(config, "speech", XTAL(20'000'000) / 5));     // Verified on PCB
	sp0256.data_request_callback().set_inputline("audiocpu", INPUT_LINE_NMI);
	sp0256.add_route(ALL_OUTPUTS, "mono", 1.0);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( sauro )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sauro-2.bin",     0x00000, 0x8000, CRC(2e356e2d) SHA1(2f893e9184f0227de4de17b7011c1bd7ea2c11b1) ) // Same label as 'sauroa', but different content
	ROM_LOAD( "sauro-1.bin",     0x08000, 0x8000, CRC(95d03e5e) SHA1(ae584ea9cecdadac46aa3565765ae0027010f8ca) ) // Same label as 'sauroa', but different content

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sauro-3.bin",     0x00000, 0x8000, CRC(0d501e1b) SHA1(20a56ff30d4fa5d2f483a449703b49153839f6bc) )

	ROM_REGION( 0x10000, "bgtiles", 0 )
	ROM_LOAD( "sauro-6.bin",     0x00000, 0x8000, CRC(4b77cb0f) SHA1(7b9cb2dca561d81390106c1a5c0533dcecaf6f1a) )
	ROM_LOAD( "sauro-7.bin",     0x08000, 0x8000, CRC(187da060) SHA1(1df156e58379bb39acade02aabab6ff1cb7cc288) )

	ROM_REGION( 0x10000, "fgtiles", 0 )
	ROM_LOAD( "sauro-4.bin",     0x00000, 0x8000, CRC(9b617cda) SHA1(ce26b84ad5ecd6185ae218520e9972645bbf09ad) )
	ROM_LOAD( "sauro-5.bin",     0x08000, 0x8000, CRC(a6e2640d) SHA1(346ffcf62e27ce8134f4e5e0dbcf11f110e19e04) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "sauro-8.bin",     0x00000, 0x8000, CRC(e08b5d5e) SHA1(eaaeaa08b19c034ab2a2140f887edffca5f441b9) )
	ROM_LOAD( "sauro-9.bin",     0x08000, 0x8000, CRC(7c707195) SHA1(0529f6808b0cec3e12ca51bee189841d21577786) )
	ROM_LOAD( "sauro-10.bin",    0x10000, 0x8000, CRC(c93380d1) SHA1(fc9655cc94c2d2058f83eb341be7e7856a08194f) )
	ROM_LOAD( "sauro-11.bin",    0x18000, 0x8000, CRC(f47982a8) SHA1(cbaeac272c015d9439f151cfb3449082f11a57a1) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "82s137-3.bin",    0x00000, 0x0400, CRC(d52c4cd0) SHA1(27d6126b46616c06b55d8018c97f6c3d7805ae9e) )  // Red component
	ROM_LOAD( "82s137-2.bin",    0x00400, 0x0400, CRC(c3e96d5d) SHA1(3f6f21526a4357e4a9a9d56a6f4ef5911af2d120) )  // Green component
	ROM_LOAD( "82s137-1.bin",    0x00800, 0x0400, CRC(bdfcf00c) SHA1(9faf4d7f8959b64faa535c9945eec59c774a3760) )  // Blue component

	ROM_REGION( 0x10000, "speech", 0 )
	// SP0256 mask ROM
	ROM_LOAD( "sp0256-al2.bin",  0x01000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc) )
ROM_END

ROM_START( sauroa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sauro-2.bin",     0x00000, 0x8000, CRC(19f8de25) SHA1(52eea7c0416ab0a8dbb3d1664b2f57ab7a405a67) ) // Same label as 'sauro', but different content
	ROM_LOAD( "sauro-1.bin",     0x08000, 0x8000, CRC(0f8b876f) SHA1(6e61a8934a2cc3c80c1f47dd59aa43aaeec12f75) ) // Same label as 'sauro', but different content

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sauro-3.bin",     0x00000, 0x8000, CRC(0d501e1b) SHA1(20a56ff30d4fa5d2f483a449703b49153839f6bc) )

	ROM_REGION( 0x10000, "bgtiles", 0 )
	ROM_LOAD( "sauro-6.bin",     0x00000, 0x8000, CRC(4b77cb0f) SHA1(7b9cb2dca561d81390106c1a5c0533dcecaf6f1a) )
	ROM_LOAD( "sauro-7.bin",     0x08000, 0x8000, CRC(187da060) SHA1(1df156e58379bb39acade02aabab6ff1cb7cc288) )

	ROM_REGION( 0x10000, "fgtiles", 0 )
	ROM_LOAD( "sauro-4.bin",     0x00000, 0x8000, CRC(9b617cda) SHA1(ce26b84ad5ecd6185ae218520e9972645bbf09ad) )
	ROM_LOAD( "sauro-5.bin",     0x08000, 0x8000, CRC(a6e2640d) SHA1(346ffcf62e27ce8134f4e5e0dbcf11f110e19e04) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "sauro-8.bin",     0x00000, 0x8000, CRC(e08b5d5e) SHA1(eaaeaa08b19c034ab2a2140f887edffca5f441b9) )
	ROM_LOAD( "sauro-9.bin",     0x08000, 0x8000, CRC(7c707195) SHA1(0529f6808b0cec3e12ca51bee189841d21577786) )
	ROM_LOAD( "sauro-10.bin",    0x10000, 0x8000, CRC(c93380d1) SHA1(fc9655cc94c2d2058f83eb341be7e7856a08194f) )
	ROM_LOAD( "sauro-11.bin",    0x18000, 0x8000, CRC(f47982a8) SHA1(cbaeac272c015d9439f151cfb3449082f11a57a1) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "82s137-3.bin",    0x00000, 0x0400, CRC(d52c4cd0) SHA1(27d6126b46616c06b55d8018c97f6c3d7805ae9e) )  // Red component
	ROM_LOAD( "82s137-2.bin",    0x00400, 0x0400, CRC(c3e96d5d) SHA1(3f6f21526a4357e4a9a9d56a6f4ef5911af2d120) )  // Green component
	ROM_LOAD( "82s137-1.bin",    0x00800, 0x0400, CRC(bdfcf00c) SHA1(9faf4d7f8959b64faa535c9945eec59c774a3760) )  // Blue component

	ROM_REGION( 0x10000, "speech", 0 )
	// SP0256 mask ROM
	ROM_LOAD( "sp0256-al2.bin",  0x01000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc) )
ROM_END

ROM_START( saurob )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2 tecfri",        0x00000, 0x8000, CRC(961567c7) SHA1(08008381b1f74ec452c4eca821b66ddcdf32e4d5) )
	ROM_LOAD( "1 tecfri",        0x08000, 0x8000, CRC(6b564429) SHA1(202bb9fa511a689f97980e01298900aca55ea84b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "3 tecfri",        0x00000, 0x8000, CRC(3eca1c5c) SHA1(0a16ddfbc3bb948023456f1c9a32593cbca5d9b0) )

	ROM_REGION( 0x10000, "bgtiles", 0 )
	ROM_LOAD( "6 tecfri",        0x00000, 0x8000, CRC(4b77cb0f) SHA1(7b9cb2dca561d81390106c1a5c0533dcecaf6f1a) )
	ROM_LOAD( "7 tecfri",        0x08000, 0x8000, CRC(187da060) SHA1(1df156e58379bb39acade02aabab6ff1cb7cc288) )

	ROM_REGION( 0x10000, "fgtiles", 0 )
	ROM_LOAD( "4 tecfri",        0x00000, 0x8000, CRC(9b617cda) SHA1(ce26b84ad5ecd6185ae218520e9972645bbf09ad) )
	ROM_LOAD( "5 tecfri",        0x08000, 0x8000, CRC(a6e2640d) SHA1(346ffcf62e27ce8134f4e5e0dbcf11f110e19e04) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "8 tecfri",        0x00000, 0x8000, CRC(e08b5d5e) SHA1(eaaeaa08b19c034ab2a2140f887edffca5f441b9) )
	ROM_LOAD( "9 tecfri",        0x08000, 0x8000, CRC(7c707195) SHA1(0529f6808b0cec3e12ca51bee189841d21577786) )
	ROM_LOAD( "10 tecfri",       0x10000, 0x8000, CRC(c93380d1) SHA1(fc9655cc94c2d2058f83eb341be7e7856a08194f) )
	ROM_LOAD( "11 tecfri",       0x18000, 0x8000, CRC(f47982a8) SHA1(cbaeac272c015d9439f151cfb3449082f11a57a1) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "82s137-3.bin",    0x00000, 0x0400, CRC(d52c4cd0) SHA1(27d6126b46616c06b55d8018c97f6c3d7805ae9e) )  // Red component
	ROM_LOAD( "82s137-2.bin",    0x00400, 0x0400, CRC(c3e96d5d) SHA1(3f6f21526a4357e4a9a9d56a6f4ef5911af2d120) )  // Green component
	ROM_LOAD( "82s137-1.bin",    0x00800, 0x0400, CRC(bdfcf00c) SHA1(9faf4d7f8959b64faa535c9945eec59c774a3760) )  // Blue component

	ROM_REGION( 0x10000, "speech", 0 )
	// SP0256 mask ROM
	ROM_LOAD( "sp0256-al2.bin",  0x01000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc) )
ROM_END

ROM_START( sauroc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sauro_facil.2",   0x00000, 0x8000, CRC(ac2e1290) SHA1(b7907ba7b910d88aed751d2129f60c783f8e583e) ) // Labeled as "facil" (easy)
	ROM_LOAD( "sauro_facil.1",   0x08000, 0x8000, CRC(c7705d1d) SHA1(2f2781ece590eb11c08f2ab13d185a932f0b340b) ) // Labeled as "facil" (easy)

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sauro-3.bin",     0x00000, 0x8000, CRC(0d501e1b) SHA1(20a56ff30d4fa5d2f483a449703b49153839f6bc) )

	ROM_REGION( 0x10000, "bgtiles", 0 )
	ROM_LOAD( "sauro-6.bin",     0x00000, 0x8000, CRC(4b77cb0f) SHA1(7b9cb2dca561d81390106c1a5c0533dcecaf6f1a) )
	ROM_LOAD( "sauro-7.bin",     0x08000, 0x8000, CRC(187da060) SHA1(1df156e58379bb39acade02aabab6ff1cb7cc288) )

	ROM_REGION( 0x10000, "fgtiles", 0 )
	ROM_LOAD( "sauro-4.bin",     0x00000, 0x8000, CRC(9b617cda) SHA1(ce26b84ad5ecd6185ae218520e9972645bbf09ad) )
	ROM_LOAD( "sauro-5.bin",     0x08000, 0x8000, CRC(a6e2640d) SHA1(346ffcf62e27ce8134f4e5e0dbcf11f110e19e04) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "sauro-8.bin",     0x00000, 0x8000, CRC(e08b5d5e) SHA1(eaaeaa08b19c034ab2a2140f887edffca5f441b9) )
	ROM_LOAD( "sauro-9.bin",     0x08000, 0x8000, CRC(7c707195) SHA1(0529f6808b0cec3e12ca51bee189841d21577786) )
	ROM_LOAD( "sauro-10.bin",    0x10000, 0x8000, CRC(c93380d1) SHA1(fc9655cc94c2d2058f83eb341be7e7856a08194f) )
	ROM_LOAD( "sauro-11.bin",    0x18000, 0x8000, CRC(f47982a8) SHA1(cbaeac272c015d9439f151cfb3449082f11a57a1) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "82s137-3.bin",    0x00000, 0x0400, CRC(d52c4cd0) SHA1(27d6126b46616c06b55d8018c97f6c3d7805ae9e) )  // Red component
	ROM_LOAD( "82s137-2.bin",    0x00400, 0x0400, CRC(c3e96d5d) SHA1(3f6f21526a4357e4a9a9d56a6f4ef5911af2d120) )  // Green component
	ROM_LOAD( "82s137-1.bin",    0x00800, 0x0400, CRC(bdfcf00c) SHA1(9faf4d7f8959b64faa535c9945eec59c774a3760) )  // Blue component

	ROM_REGION( 0x10000, "speech", 0 )
	// SP0256 mask ROM
	ROM_LOAD( "sp0256-al2.bin",  0x01000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc) )
ROM_END

ROM_START( saurop )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "s2.3k",           0x00000, 0x8000, CRC(79846222) SHA1(59ccfbaad0251f771c0fd624d00d93a50bca67d8) )
	ROM_LOAD( "s1.3f",           0x08000, 0x8000, CRC(3efd13ed) SHA1(3920d21d5d9c285c5bcf47aa12b4e9a42294f149) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "s3.5x",           0x00000, 0x8000, CRC(0d501e1b) SHA1(20a56ff30d4fa5d2f483a449703b49153839f6bc) )

	ROM_REGION( 0x10000, "bgtiles", 0 )
	ROM_LOAD( "s6.7x",           0x00000, 0x8000, CRC(4b77cb0f) SHA1(7b9cb2dca561d81390106c1a5c0533dcecaf6f1a) )
	ROM_LOAD( "s7.7z",           0x08000, 0x8000, CRC(187da060) SHA1(1df156e58379bb39acade02aabab6ff1cb7cc288) )

	ROM_REGION( 0x10000, "fgtiles", 0 )
	ROM_LOAD( "s4.7h",           0x00000, 0x8000, CRC(9b617cda) SHA1(ce26b84ad5ecd6185ae218520e9972645bbf09ad) )
	ROM_LOAD( "s5.7k",           0x08000, 0x8000, CRC(de5cd249) SHA1(e3752b88b539e1057a35619ffbad01720ab60d7d) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "s8.10l",          0x00000, 0x8000, CRC(e08b5d5e) SHA1(eaaeaa08b19c034ab2a2140f887edffca5f441b9) )
	ROM_LOAD( "s9.10p",          0x08000, 0x8000, CRC(7c707195) SHA1(0529f6808b0cec3e12ca51bee189841d21577786) )
	ROM_LOAD( "s10.10r",         0x10000, 0x8000, CRC(c93380d1) SHA1(fc9655cc94c2d2058f83eb341be7e7856a08194f) )
	ROM_LOAD( "s11.10t",         0x18000, 0x8000, CRC(f47982a8) SHA1(cbaeac272c015d9439f151cfb3449082f11a57a1) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "82s137-3.bin",    0x00000, 0x0400, CRC(d52c4cd0) SHA1(27d6126b46616c06b55d8018c97f6c3d7805ae9e) )  // Red component
	ROM_LOAD( "82s137-2.bin",    0x00400, 0x0400, CRC(c3e96d5d) SHA1(3f6f21526a4357e4a9a9d56a6f4ef5911af2d120) )  // Green component
	ROM_LOAD( "82s137-1.bin",    0x00800, 0x0400, CRC(bdfcf00c) SHA1(9faf4d7f8959b64faa535c9945eec59c774a3760) )  // Blue component

	ROM_REGION( 0x10000, "speech", 0 )
	// SP0256 mask ROM
	ROM_LOAD( "sp0256-al2.bin",  0x01000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc) )
ROM_END

ROM_START( saurorr ) // all roms have original Tecfri stickers
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27256-2.bin",     0x00000, 0x8000, CRC(b0d80eab) SHA1(60cbe16d6c87d4681155814a5034b7e9d10bbd81) )
	ROM_LOAD( "27256-1.bin",     0x08000, 0x8000, CRC(cbb5f06e) SHA1(f93c01006d308e0b6950d720b6fe4409728c79e2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sauro-3.bin",     0x00000, 0x8000, CRC(0d501e1b) SHA1(20a56ff30d4fa5d2f483a449703b49153839f6bc) )

	ROM_REGION( 0x10000, "bgtiles", 0 )
	ROM_LOAD( "sauro-6.bin",     0x00000, 0x8000, CRC(4b77cb0f) SHA1(7b9cb2dca561d81390106c1a5c0533dcecaf6f1a) )
	ROM_LOAD( "sauro-7.bin",     0x08000, 0x8000, CRC(187da060) SHA1(1df156e58379bb39acade02aabab6ff1cb7cc288) )

	ROM_REGION( 0x10000, "fgtiles", 0 )
	ROM_LOAD( "sauro-4.bin",     0x00000, 0x8000, CRC(9b617cda) SHA1(ce26b84ad5ecd6185ae218520e9972645bbf09ad) )
	ROM_LOAD( "27256-5.bin",     0x08000, 0x8000, CRC(9aabdbe5) SHA1(ef008e368024f9377a8d2bc5863b01c63bc8f55b) ) // contains the changed license logo

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "sauro-8.bin",     0x00000, 0x8000, CRC(e08b5d5e) SHA1(eaaeaa08b19c034ab2a2140f887edffca5f441b9) )
	ROM_LOAD( "sauro-9.bin",     0x08000, 0x8000, CRC(7c707195) SHA1(0529f6808b0cec3e12ca51bee189841d21577786) )
	ROM_LOAD( "sauro-10.bin",    0x10000, 0x8000, CRC(c93380d1) SHA1(fc9655cc94c2d2058f83eb341be7e7856a08194f) )
	ROM_LOAD( "sauro-11.bin",    0x18000, 0x8000, CRC(f47982a8) SHA1(cbaeac272c015d9439f151cfb3449082f11a57a1) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "82s137-3.bin",    0x00000, 0x0400, CRC(d52c4cd0) SHA1(27d6126b46616c06b55d8018c97f6c3d7805ae9e) )  // Red component
	ROM_LOAD( "82s137-2.bin",    0x00400, 0x0400, CRC(c3e96d5d) SHA1(3f6f21526a4357e4a9a9d56a6f4ef5911af2d120) )  // Green component
	ROM_LOAD( "82s137-1.bin",    0x00800, 0x0400, CRC(bdfcf00c) SHA1(9faf4d7f8959b64faa535c9945eec59c774a3760) )  // Blue component

	ROM_REGION( 0x10000, "speech", 0 )
	// SP0256 mask ROM
	ROM_LOAD( "sp0256-al2.bin",   0x1000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc) )
ROM_END

ROM_START( seawolft )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin",           0x00000, 0x8000, CRC(bd8bd328) SHA1(f898d882790cc167ca82b55f93e47512c5195b45) ) // Passes the ROM test OK
	ROM_LOAD( "2.bin",           0x08000, 0x8000, CRC(870b05ef) SHA1(c841f80a7e014a90fc81b00fa8c99405de9f660f) ) // Passes the ROM test OK

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "tmm24256ap.bin",  0x00000, 0x8000, CRC(0d501e1b) SHA1(20a56ff30d4fa5d2f483a449703b49153839f6bc) ) // Same as parent

	ROM_REGION( 0x10000, "bgtiles", 0 )
	ROM_LOAD( "4.bin",           0x00000, 0x8000, CRC(4b77cb0f) SHA1(7b9cb2dca561d81390106c1a5c0533dcecaf6f1a) ) // Same as parent
	ROM_LOAD( "3.bin",           0x08000, 0x8000, CRC(883bb7d1) SHA1(7320e5cddb5c2127b3679b7bc72b273860d178b9) )

	ROM_REGION( 0x10000, "fgtiles", 0 )
	ROM_LOAD( "6.bin",           0x00000, 0x8000, CRC(9b617cda) SHA1(ce26b84ad5ecd6185ae218520e9972645bbf09ad) ) // Same as parent
	ROM_LOAD( "5.bin",           0x08000, 0x8000, CRC(a6e2640d) SHA1(346ffcf62e27ce8134f4e5e0dbcf11f110e19e04) ) // Same as parent

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "10.bin",          0x00000, 0x8000, CRC(b93f5487) SHA1(a3f36793ded053db7b370bc54a1b59d7b0603590) )
	ROM_LOAD( "9.bin",           0x08000, 0x8000, CRC(0964ac95) SHA1(acc55ed318adee33c76ac24002a0cd7d35f38d98) )
	ROM_LOAD( "8.bin",           0x10000, 0x8000, CRC(e71726a9) SHA1(2ef83432eb02ea0849547e5cb2b2215b8e68d714) )
	ROM_LOAD( "7.bin",           0x18000, 0x8000, CRC(8a700276) SHA1(7c450d472a41f74c69bf357a31ad5418d7dbd2ed) )

	// PROMs not dumped on this PCB
	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "82s137-3.bin",    0x00000, 0x0400, CRC(d52c4cd0) SHA1(27d6126b46616c06b55d8018c97f6c3d7805ae9e) )  // Red component
	ROM_LOAD( "82s137-2.bin",    0x00400, 0x0400, CRC(c3e96d5d) SHA1(3f6f21526a4357e4a9a9d56a6f4ef5911af2d120) )  // Green component
	ROM_LOAD( "82s137-1.bin",    0x00800, 0x0400, CRC(bdfcf00c) SHA1(9faf4d7f8959b64faa535c9945eec59c774a3760) )  // Blue component

	ROM_REGION( 0x10000, "speech", 0 )
	// SP0256 mask ROM, not dumped on this PCB, but it's a generic GI ROM
	ROM_LOAD( "sp0256-al2.bin",  0x01000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc) )
ROM_END

/*
Sauro (bootleg)

CPU  : Z80
Sound: Z80, YM3526, YM3014
RAM  : 2016 (x4), 6116 (x5)
Xtal : 20.000MHz, 8.000MHz
DIPs : 8 position (x2)
Other: ULN2003
       unpopulated position for SP0256-AL

Only ROMs 01, 02 & 03 are different to existing archive.
Color PROMs match existing archive. One extra PROM was found near ROMs 6 & 7 (sauropr4.16h)
*/
ROM_START( saurobl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sauro02.7c",      0x00000, 0x8000, CRC(72026b9a) SHA1(538f6bffab5cb0f7609a5afaab4d839baf26a1a7) )
	ROM_LOAD( "sauro01.6c",      0x08000, 0x8000, CRC(4ff12c25) SHA1(e9f240d0476a821488006e7b28490e2e7c0c1819) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sauro03.16e",     0x00000, 0x8000, CRC(a30b60fc) SHA1(48ea586a333e42852a6c9a5df48b2f2ccace6d36) )

	ROM_REGION( 0x10000, "bgtiles", 0 )
	ROM_LOAD( "sauro-6.bin",     0x00000, 0x8000, CRC(4b77cb0f) SHA1(7b9cb2dca561d81390106c1a5c0533dcecaf6f1a) ) // sauro06.16g
	ROM_LOAD( "sauro-7.bin",     0x08000, 0x8000, CRC(187da060) SHA1(1df156e58379bb39acade02aabab6ff1cb7cc288) ) // sauro07.18g

	ROM_REGION( 0x10000, "fgtiles", 0 )
	ROM_LOAD( "sauro-4.bin",     0x00000, 0x8000, CRC(9b617cda) SHA1(ce26b84ad5ecd6185ae218520e9972645bbf09ad) ) // sauro04.7g
	ROM_LOAD( "sauro-5.bin",     0x08000, 0x8000, CRC(a6e2640d) SHA1(346ffcf62e27ce8134f4e5e0dbcf11f110e19e04) ) // sauro05.8g

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "sauro-8.bin",     0x00000, 0x8000, CRC(e08b5d5e) SHA1(eaaeaa08b19c034ab2a2140f887edffca5f441b9) ) // sauro08.9j
	ROM_LOAD( "sauro-9.bin",     0x08000, 0x8000, CRC(7c707195) SHA1(0529f6808b0cec3e12ca51bee189841d21577786) ) // sauro09.11j
	ROM_LOAD( "sauro-10.bin",    0x10000, 0x8000, CRC(c93380d1) SHA1(fc9655cc94c2d2058f83eb341be7e7856a08194f) ) // sauro10.12j
	ROM_LOAD( "sauro-11.bin",    0x18000, 0x8000, CRC(f47982a8) SHA1(cbaeac272c015d9439f151cfb3449082f11a57a1) ) // sauro11.14j

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "82s137-3.bin",    0x00000, 0x0400, CRC(d52c4cd0) SHA1(27d6126b46616c06b55d8018c97f6c3d7805ae9e) )  // Red component
	ROM_LOAD( "82s137-2.bin",    0x00400, 0x0400, CRC(c3e96d5d) SHA1(3f6f21526a4357e4a9a9d56a6f4ef5911af2d120) )  // Green component
	ROM_LOAD( "82s137-1.bin",    0x00800, 0x0400, CRC(bdfcf00c) SHA1(9faf4d7f8959b64faa535c9945eec59c774a3760) )  // Blue component

	ROM_REGION( 0x0200, "user1", 0 ) // Unknown PROM was found near ROMs 6 & 7
	ROM_LOAD( "sauropr4.16h",    0x00000, 0x0200, CRC(5261bc11) SHA1(1cc7a9a7376e65f4587b75ef9382049458656372) )
ROM_END


ROM_START( trckydoc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "trckydoc.d9",     0x00000, 0x8000, CRC(c6242fc3) SHA1(c8a6f6abe8b51061a113ed75fead0479df68ec40) )
	ROM_LOAD( "trckydoc.b9",     0x08000, 0x8000, CRC(8645c840) SHA1(79c2acfc1aeafbe94afd9d230200bd7cdd7bcd1b) )

	ROM_REGION( 0x10000, "bgtiles", 0 )
	ROM_LOAD( "trckydoc.e6",     0x00000, 0x8000, CRC(ec326392) SHA1(e6954fecc501a821caa21e67597914519fbbe58f) )
	ROM_LOAD( "trckydoc.g6",     0x08000, 0x8000, CRC(6a65c088) SHA1(4a70c104809d86b4eef6cc0df9452966fe7c9859) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "trckydoc.h1",     0x00000, 0x4000, CRC(8b73cbf3) SHA1(d10f79a38c1596c90bac9cf4c64ba38ae6ecd8cb) )
	ROM_LOAD( "trckydoc.e1",     0x04000, 0x4000, CRC(841be98e) SHA1(82da07490b73edcbffc3b9247205aab3a1f7d7ad) )
	ROM_LOAD( "trckydoc.c1",     0x08000, 0x4000, CRC(1d25574b) SHA1(924e4376a7fe6cdfff0fa6045aaa3f7c0633d275) )
	ROM_LOAD( "trckydoc.a1",     0x0c000, 0x4000, CRC(436c59ba) SHA1(2aa9c155c432a3c81420520c53bb944dcc613a94) )

	ROM_REGION( 0x0c00, "proms", 0 ) // colour PROMs
	ROM_LOAD( "tdclr3.prm",      0x00000, 0x0100, CRC(671d0140) SHA1(7d5fcd9589c46590b0a240cac428f993201bec2a) )
	ROM_LOAD( "tdclr2.prm",      0x00400, 0x0100, CRC(874f9050) SHA1(db40d68f5166657fce0eadcd82143112b0388894) )
	ROM_LOAD( "tdclr1.prm",      0x00800, 0x0100, CRC(57f127b0) SHA1(3d2b18a7a31933579f06d92fa0cc3f0e1fe8b98a) )

	ROM_REGION( 0x0200, "user1", 0 ) // unknown
	ROM_LOAD( "tdprm.prm",       0x00000, 0x0200, CRC(5261bc11) SHA1(1cc7a9a7376e65f4587b75ef9382049458656372) )
ROM_END

ROM_START( trckydoca )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "trckydca.d9",     0x00000, 0x8000, CRC(99c38aa4) SHA1(298a19439cc17743e10d101c50a26b9a7348299e) )
	ROM_LOAD( "trckydca.b9",     0x08000, 0x8000, CRC(b6048a15) SHA1(d982fafbfa391ef9bab50bfd52607494e2a9eedf) )

	ROM_REGION( 0x10000, "bgtiles", 0 )
	ROM_LOAD( "trckydoc.e6",     0x00000, 0x8000, CRC(ec326392) SHA1(e6954fecc501a821caa21e67597914519fbbe58f) )
	ROM_LOAD( "trckydoc.g6",     0x08000, 0x8000, CRC(6a65c088) SHA1(4a70c104809d86b4eef6cc0df9452966fe7c9859) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "trckydoc.h1",     0x00000, 0x4000, CRC(8b73cbf3) SHA1(d10f79a38c1596c90bac9cf4c64ba38ae6ecd8cb) )
	ROM_LOAD( "trckydoc.e1",     0x04000, 0x4000, CRC(841be98e) SHA1(82da07490b73edcbffc3b9247205aab3a1f7d7ad) )
	ROM_LOAD( "trckydoc.c1",     0x08000, 0x4000, CRC(1d25574b) SHA1(924e4376a7fe6cdfff0fa6045aaa3f7c0633d275) )
	ROM_LOAD( "trckydoc.a1",     0x0c000, 0x4000, CRC(436c59ba) SHA1(2aa9c155c432a3c81420520c53bb944dcc613a94) )

	ROM_REGION( 0x0c00, "proms", 0 ) // colour PROMs
	ROM_LOAD( "tdclr3.prm",      0x00000, 0x0100, CRC(671d0140) SHA1(7d5fcd9589c46590b0a240cac428f993201bec2a) )
	ROM_LOAD( "tdclr2.prm",      0x00400, 0x0100, CRC(874f9050) SHA1(db40d68f5166657fce0eadcd82143112b0388894) )
	ROM_LOAD( "tdclr1.prm",      0x00800, 0x0100, CRC(57f127b0) SHA1(3d2b18a7a31933579f06d92fa0cc3f0e1fe8b98a) )

	ROM_REGION( 0x0200, "user1", 0 ) // unknown
	ROM_LOAD( "tdprm.prm",       0x00000, 0x0200, CRC(5261bc11) SHA1(1cc7a9a7376e65f4587b75ef9382049458656372) )
ROM_END

} // anonymous namespace


GAME( 1987, sauro,     0,        sauro,    tecfri,    sauro_state,    empty_init, ROT0, "Tecfri",                                "Sauro (set 1)",                         MACHINE_SUPPORTS_SAVE )
GAME( 1987, sauroa,    sauro,    sauro,    tecfri,    sauro_state,    empty_init, ROT0, "Tecfri",                                "Sauro (set 2)",                         MACHINE_SUPPORTS_SAVE )
GAME( 1987, saurob,    sauro,    sauro,    tecfri,    sauro_state,    empty_init, ROT0, "Tecfri",                                "Sauro (set 3)",                         MACHINE_SUPPORTS_SAVE )
GAME( 1987, sauroc,    sauro,    sauro,    tecfri,    sauro_state,    empty_init, ROT0, "Tecfri",                                "Sauro (set 4, easier)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1987, saurop,    sauro,    sauro,    tecfri,    sauro_state,    empty_init, ROT0, "Tecfri (Philko license)",               "Sauro (Philko license)",                MACHINE_SUPPORTS_SAVE )
GAME( 1987, saurorr,   sauro,    sauro,    tecfri,    sauro_state,    empty_init, ROT0, "Tecfri (Recreativos Real S.A. license)","Sauro (Recreativos Real S.A. license)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, saurobl,   sauro,    saurobl,  saurobl,   sauro_state,    empty_init, ROT0, "bootleg",                               "Sauro (bootleg)",                       MACHINE_SUPPORTS_SAVE )
GAME( 1987, seawolft,  sauro,    sauro,    tecfri,    sauro_state,    empty_init, ROT0, "Tecfri",                                "Sea Wolf (Tecfri)",                     MACHINE_SUPPORTS_SAVE )

GAME( 1987, trckydoc,  0,        trckydoc, tecfri,    trckydoc_state, empty_init, ROT0, "Tecfri",                                "Tricky Doc (set 1)",                    MACHINE_SUPPORTS_SAVE )
GAME( 1987, trckydoca, trckydoc, trckydoc, trckydoca, trckydoc_state, empty_init, ROT0, "Tecfri",                                "Tricky Doc (set 2)",                    MACHINE_SUPPORTS_SAVE )
