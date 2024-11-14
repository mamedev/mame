// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                          -= Yun Sung 16 Bit Games =-

                    driver by   Luca Elia (l.elia@tin.it)


Main  CPU    :  MC68000
Sound CPU    :  Z80 [Optional]
Video Chips  :  Actel A1020B PL84C
Sound Chips  :  OKI M6295 + YM3812 [Optional]


---------------------------------------------------------------------------
Year + Game         Board#
---------------------------------------------------------------------------
199? Magic Bubble     YS-1302 / YS102
1996 Paparazzi        YS-0211? Looks identical
1997 Shocking         YS-0211
1998 Bomb Kick        YS-0211
---------------------------------------------------------------------------

- Screen flipping: not used!?

Original bugs:

- In shocking, service mode just shows the menu, with mangled graphics
  (sprites, but the charset they used is in the tiles ROMs!).
  In magicbub they used color 0 for tiles (all blacks, so you can't see
  most of it!). Again, color 0 for sprites would be ok. Some kind
  of sprites-tiles swapping, or unfinished leftovers?


Stephh's notes (based on the games M68000 code and some tests) :

1) 'magicbub'

  - No 1P Vs COM mode.
  - Starting in 1P Vs 2P mode costs only 1 credit. But, if no "Continue Play", the game is over.
    However, when a player joins in, if no "Continue Play", the winner continues.
  - There is an ingame bug when in 1P Vs 2P mode : whatever the settings are, there will
    always be 2 things displayed for each player as if it was a "Best of 3 rounds" match.
  - Another ingame bug is that the game doesn't end after 5 rounds in "Easy" puzzle mode.
    In fact, the first 5 rounds (A1 to A5) are different, then they are the same as in "Normal".

2) 'magicbua'

  - Additional 1P Vs COM mode with always only 1 winning round.
  - Starting in 1P Vs 2P mode costs 2 credits. If no "Continue Play", the winner continues.
    And when a player joins in, if no "Continue Play", the winner also continues.
  - There is an ingame bug when in 1P Vs 2P mode : whatever the settings are, there will
    always be 2 things displayed for each player as if it was a "Best of 3 rounds" match.
  - Another ingame bug is that the game doesn't end after 5 rounds in "Easy" puzzle mode.
    In fact, the first 5 rounds (A1 to A5) are different, then they are the same as in "Normal".
  - There are 50 gals pics, 25 of them are "soft" (sort of) and the 25 others are "hard".
    When nudity set to "Soft only", the game will only display "soft" gals pics.
    When nudity set to "Hard only", the game will only display "hard" gals pics.
    When nudity set to "Soft and High", the game will alternate "soft" and "hard" gals pics.
    When nudity set to "Soft then High" (code at 0x00b8a6) :
      . In puzzle mode, the game will display "soft" gals pics for rounds A1 to I5,
        then it will display "hard" gals pics for rounds J1 to Z5.
      . 1P Vs COM mode, the game will display "soft" gals pics for rounds 1 to 6,
        then it will display "hard" gals pics for rounds 7 to 100.

3) 'shocking'

  - DSW1 bit 7 was used to select language but this feature is deactivated due to code at 0x0017f2.

4) 'bombkick'

  - DSW1 bits 3 and 4 determine difficulty (code at 0x0003a4). But DSW1 bit 4 is also used
    in combination with DSW1 bit 5 to determine the number of special powers (code at 0x0003c2) !
    This means that you can have 2 or 3 special powers when you set difficulty to "Easy" or "Normal",
    but you'll ALWAYS have 3 special powers when you set difficulty to "Hard" or "Very Hard".
  - DSW2 bits 2 to 7 are checked when you select a level :
      . bit 2 must be OFF (code at 0x029ba8) : level 3 ("STONE")
      . bit 3 must be OFF (code at 0x029b96) : level 1 ("BUDDHA")
      . bit 4 must be ON  (code at 0x029b9c) : level 2 ("JOY WORLD")
      . bit 5 must be OFF (code at 0x029bae) : level 4 ("MUSIC")
      . bit 6 must be ON  (code at 0x029bba) : level 6 ("GOBLIN")
      . bit 7 must be OFF (code at 0x029bb4) : level 5 ("ALADDIN")
  - DSW2 bits 6 and 7 are also checked during boot-up sequence (code at 0x02dbc0) :
      . bit 6 must be ON
      . bit 7 must be OFF

***************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class yunsun16_state : public driver_device
{
protected:
	yunsun16_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_vram(*this, "vram_%u", 0U),
		m_scrollram(*this, "scrollram_%u", 0U),
		m_priorityram(*this, "priorityram"),
		m_spriteram(*this, "spriteram")
	{ }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr_array<uint16_t, 2> m_vram;
	required_shared_ptr_array<uint16_t, 2> m_scrollram;
	required_shared_ptr<uint16_t> m_priorityram;
	required_shared_ptr<uint16_t> m_spriteram;

	// other video-related elements
	tilemap_t *m_tilemap[2]{};
	int8_t m_sprites_scrolldx = 0;
	int8_t m_sprites_scrolldy = 0;

	void int_ack_w(uint8_t data);
	template <int Layer> void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	TILEMAP_MAPPER_MEMBER(tilemap_scan_pages);
	template <int Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


class magicbub_state : public yunsun16_state
{
public:
	magicbub_state(const machine_config &mconfig, device_type type, const char *tag) :
		yunsun16_state(mconfig, type, tag),
		m_audiocpu(*this, "audiocpu"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void magicbub(machine_config &config);

private:
	required_device<cpu_device> m_audiocpu;
	required_device<generic_latch_8_device> m_soundlatch;

	void sound_command_w(uint8_t data);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sound_port_map(address_map &map) ATTR_COLD;
};


class shocking_state : public yunsun16_state
{
public:
	shocking_state(const machine_config &mconfig, device_type type, const char *tag) :
		yunsun16_state(mconfig, type, tag),
		m_okibank(*this, "okibank")
	{ }

	void shocking(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_memory_bank m_okibank;

	void sound_bank_w(uint8_t data);

	void main_map(address_map &map) ATTR_COLD;
	void oki_map(address_map &map) ATTR_COLD;
};


/***************************************************************************


    [ 2 Scrolling Layers ]

    Tiles are 16 x 16 x 8. The layout of the tilemap is a bit weird:
    16 consecutive tile codes define a vertical column.
    16 columns form a page (256 x 256).
    The tilemap is made of 4 x 4 pages (1024 x 1024)

    [ 512? Sprites ]

    Sprites are 16 x 16 x 4 in size. There's RAM for 512, but
    the game just copies 384 entries.


***************************************************************************/

/***************************************************************************


                                    Tilemaps


***************************************************************************/

/*
#define TILES_PER_PAGE_X    (0x10)
#define TILES_PER_PAGE_Y    (0x10)
#define PAGES_PER_TMAP_X    (0x4)
#define PAGES_PER_TMAP_Y    (0x4)
*/

TILEMAP_MAPPER_MEMBER(yunsun16_state::tilemap_scan_pages)
{
	return ((row & 0x30) << 6) | ((col & 0x3f) << 4) | (row & 0xf);
}

template <int Layer>
TILE_GET_INFO_MEMBER(yunsun16_state::get_tile_info)
{
	uint16_t code = m_vram[Layer][2 * tile_index + 0];
	uint16_t attr = m_vram[Layer][2 * tile_index + 1];
	tileinfo.set(0,
			code,
			attr & 0xf,
			(attr & 0x20) ? TILE_FLIPX : 0);
}


/***************************************************************************


                            Video Hardware Init


***************************************************************************/

void yunsun16_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(yunsun16_state::get_tile_info<0>)), tilemap_mapper_delegate(*this, FUNC(yunsun16_state::tilemap_scan_pages)),
			16, 16, 0x40, 0x40);
	m_tilemap[1] = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(yunsun16_state::get_tile_info<1>)), tilemap_mapper_delegate(*this, FUNC(yunsun16_state::tilemap_scan_pages)),
			16, 16, 0x40, 0x40);

	m_tilemap[0]->set_scrolldx(-0x34, 0);
	m_tilemap[1]->set_scrolldx(-0x38, 0);

	m_tilemap[0]->set_scrolldy(-0x10, 0);
	m_tilemap[1]->set_scrolldy(-0x10, 0);

	m_tilemap[0]->set_transparent_pen(0xff);
	m_tilemap[1]->set_transparent_pen(0xff);
}


/***************************************************************************


                                Sprites Drawing


        0.w                             X

        2.w                             Y

        4.w                             Code

        6.w     fedc ba98 7--- ----
                ---- ---- -6-- ----     Flip Y
                ---- ---- --5- ----     Flip X
                ---- ---- ---4 3210     Color


***************************************************************************/

void yunsun16_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const rectangle &visarea = m_screen->visible_area();

	int max_x = visarea.max_x + 1;
	int max_y = visarea.max_y + 1;

	int pri = *m_priorityram & 3;
	int pri_mask;

	switch (pri)
	{
		case 1:
			pri_mask = (1 << 1) | (1 << 2) | (1 << 3);
			break;
		case 2:
			pri_mask = (1 << 2) | (1 << 3);
			break;
		case 3:
		default:
			pri_mask = 0;
			break;
	}

	for (int offs = (m_spriteram.bytes() - 8) / 2; offs >= 0; offs -= 8 / 2)
	{
		int x = m_spriteram[offs + 0];
		int y = m_spriteram[offs + 1];
		int code = m_spriteram[offs + 2];
		int attr = m_spriteram[offs + 3];
		int flipx = attr & 0x20;
		int flipy = attr & 0x40;

		x += m_sprites_scrolldx;
		y += m_sprites_scrolldy;

		if (flip_screen())   // not used?
		{
			flipx = !flipx;     x = max_x - x - 16;
			flipy = !flipy;     y = max_y - y - 16;
		}

		m_gfxdecode->gfx(1)->prio_transpen(bitmap, cliprect,
					code,
					attr & 0x1f,
					flipx, flipy,
					x, y,
					screen.priority(),
					pri_mask, 15);
	}
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/


uint32_t yunsun16_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap[0]->set_scrollx(0, m_scrollram[0][0]);
	m_tilemap[0]->set_scrolly(0, m_scrollram[0][1]);

	m_tilemap[1]->set_scrollx(0, m_scrollram[1][0]);
	m_tilemap[1]->set_scrolly(0, m_scrollram[1][1]);

	//popmessage("%04X", *m_priorityram);

	screen.priority().fill(0, cliprect);

	if ((*m_priorityram & 0x0c) == 4)
	{
		// The color of this layer's transparent pen goes below everything
		m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 1);
		m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 2);
	}
	else if ((*m_priorityram & 0x0c) == 8)
	{
		// The color of this layer's transparent pen goes below everything
		m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 1);
		m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 2);
	}

	draw_sprites(screen, bitmap, cliprect);
	return 0;
}


/***************************************************************************


                            Memory Maps - Main CPU


***************************************************************************/

void shocking_state::sound_bank_w(uint8_t data)
{
	m_okibank->set_entry(data & 3);
}

template <int Layer>
void yunsun16_state::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vram[Layer][offset]);
	m_tilemap[Layer]->mark_tile_dirty(offset / 2);
}

void yunsun16_state::int_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(M68K_IRQ_2, CLEAR_LINE);
}

void yunsun16_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x800000, 0x800001).portr("INPUTS");
	map(0x800018, 0x800019).portr("SYSTEM");
	map(0x80001a, 0x80001b).portr("DSW1");
	map(0x80001c, 0x80001d).portr("DSW2");
	map(0x800030, 0x800031).nopw();    // ? (value: don't care)
	map(0x800100, 0x800101).nopw();    // ? $9100
	map(0x800102, 0x800103).nopw();    // ? $9080
	map(0x800104, 0x800105).nopw();    // ? $90c0
	map(0x80010a, 0x80010b).nopw();    // ? $9000
	map(0x80010c, 0x80010f).ram().share(m_scrollram[1]);
	map(0x800114, 0x800117).ram().share(m_scrollram[0]);
	map(0x800154, 0x800155).ram().share(m_priorityram);
	map(0x8001fe, 0x8001fe).w(FUNC(yunsun16_state::int_ack_w));
	map(0x900000, 0x903fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x908000, 0x90bfff).ram().w(FUNC(yunsun16_state::vram_w<1>)).share(m_vram[1]); // Layer 1
	map(0x90c000, 0x90ffff).ram().w(FUNC(yunsun16_state::vram_w<0>)).share(m_vram[0]); // Layer 0
	map(0x910000, 0x910fff).ram().share(m_spriteram);
	map(0xff0000, 0xffffff).ram();
}

void magicbub_state::main_map(address_map &map)
{
	yunsun16_state::main_map(map);
	map(0x800189, 0x800189).w(FUNC(magicbub_state::sound_command_w));
}

void shocking_state::main_map(address_map &map)
{
	yunsun16_state::main_map(map);
	map(0x800181, 0x800181).w(FUNC(shocking_state::sound_bank_w));
	map(0x800189, 0x800189).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}


void magicbub_state::sound_command_w(uint8_t data)
{
	// HACK: the game continuously sends this. It'll play the oki sample number 0 on each voice. That sample is 00000-00000.
	if (data != 0x3a)
	{
		m_soundlatch->write(data);
		m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
}


/***************************************************************************


                            Memory Maps - Sound CPU


***************************************************************************/

void magicbub_state::sound_map(address_map &map)
{
	map(0x0000, 0xdfff).rom();
	map(0xe000, 0xe7ff).ram();
}

void magicbub_state::sound_port_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x10, 0x11).rw("ymsnd", FUNC(ym3812_device::read), FUNC(ym3812_device::write));
	map(0x18, 0x18).r(m_soundlatch, FUNC(generic_latch_8_device::read));     // From Main CPU
	map(0x1c, 0x1c).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

void shocking_state::oki_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x20000, 0x3ffff).bankr(m_okibank);
}


/***************************************************************************


                                Input Ports


***************************************************************************/


/***************************************************************************
                                Magic Bubble
***************************************************************************/

static INPUT_PORTS_START( magicbub )
	PORT_START("INPUTS")    // $800000.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")    // $800019.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")  // $80001b.b -> $ff0003.b
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPUNUSED( 0x0020, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0080, IP_ACTIVE_LOW )

	PORT_START("DSW2")  // $80001d.b -> $ff0004.b
	PORT_DIPUNUSED( 0x0001, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x0002, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x000c, 0x000c, "1P Vs 2P Rounds (Start)" )
	PORT_DIPSETTING(      0x0008, "Best of 1" )             // 1 winning round needed
	PORT_DIPSETTING(      0x000c, "Best of 3" )             // 2 winning rounds needed
	PORT_DIPSETTING(      0x0004, "Best of 5" )             // 3 winning rounds needed
	PORT_DIPSETTING(      0x0000, "Best of 7" )             // 4 winning rounds needed
	PORT_DIPNAME( 0x0010, 0x0010, "1P Vs 2P Rounds (Join-in)" )
	PORT_DIPSETTING(      0x0000, "Best of 1" )             // 1 winning round needed
	PORT_DIPSETTING(      0x0010, "Best of 3" )             // 2 winning rounds needed
	PORT_DIPUNUSED( 0x0020, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x0040, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x0080, IP_ACTIVE_LOW )
INPUT_PORTS_END

/***************************************************************************
                        Magic Bubble (Adult version)
***************************************************************************/

static INPUT_PORTS_START( magicbua )
	PORT_INCLUDE(magicbub)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0003, 0x0003, "Nudity" )                // Read notes
	PORT_DIPSETTING(      0x0003, "Soft only" )
	PORT_DIPSETTING(      0x0000, "Hard only" )
	PORT_DIPSETTING(      0x0001, "Soft and Hard" )
	PORT_DIPSETTING(      0x0002, "Soft then Hard" )
INPUT_PORTS_END

/***************************************************************************
                                Shocking
***************************************************************************/

static INPUT_PORTS_START( shocking )
	PORT_START("INPUTS")    // $800000.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")    // $800019.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")  // $80001b.b -> $ff0c06.b
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPUNUSED( 0x0008, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x0010, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x0020, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x0040, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unused ) )       // Used to be "Language" - read notes
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )          // "Korean"
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )           // "English"

	PORT_START("DSW2")  // $80001d.b -> $ff0c07.b
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Easiest ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( Easier ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Harder ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPUNUSED( 0x0008, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0020, "2" )
	PORT_DIPSETTING(      0x0030, "3" )
	PORT_DIPSETTING(      0x0010, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0080, IP_ACTIVE_LOW )
INPUT_PORTS_END

/***************************************************************************
                                Bomb Kick
***************************************************************************/

static INPUT_PORTS_START( bombkick )
	PORT_START("INPUTS")    // $800000.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")    // $800019.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")  // $80001b.b -> $ff0004.b
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0020, 0x0000, "Special Powers" )        // Only has an effect when difficulty set to "Easy" or "Normal" - read notes
	PORT_DIPSETTING(      0x0020, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0080, IP_ACTIVE_LOW )

	PORT_START("DSW2")  // $80001d.b -> $ff0005.b
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0002, "4" )
	PORT_DIPSETTING(      0x0001, "5" )
	PORT_DIPNAME( 0x0004, 0x0004, "DSW 2:3 - LEAVE OFF!" )  // Must be OFF ! - read notes
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "DSW 2:4 - LEAVE OFF!" )  // Must be OFF ! - read notes
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, "DSW 2:5 - LEAVE ON!" )   // Must be ON  ! - read notes
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "DSW 2:6 - LEAVE OFF!" )  // Must be OFF ! - read notes
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, "DSW 2:7 - LEAVE ON!" )   // Must be ON  ! - read notes
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "DSW 2:8 - LEAVE OFF!" )  // Must be OFF ! - read notes
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

/***************************************************************************
                                Paparazzi
***************************************************************************/

static INPUT_PORTS_START( paprazzi )
	PORT_START("INPUTS")    // $800000.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")    // $800019.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")  // $80001b.b -> $ff0aca.b
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_2C ) )
	PORT_DIPUNKNOWN( 0x0004, 0x0004 )  // $25bc
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Language ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Korean ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( English ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Enemies" ) // something else.. but related to enemy types
	PORT_DIPSETTING(      0x0000, "Type 1" )
	PORT_DIPSETTING(      0x0010, "Type 2" )
	PORT_DIPUNKNOWN( 0x0020, 0x0020 )
	PORT_DIPNAME( 0x00c0, 0x0080, "Time" )
	PORT_DIPSETTING(      0x0000, "80" )
	PORT_DIPSETTING(      0x0040, "100" )
	PORT_DIPSETTING(      0x0080, "120" )
	PORT_DIPSETTING(      0x00c0, "150" )

	PORT_START("DSW2")  /* $80001d.b -> $ff0acb.b */
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Difficulty ) ) //not sure what it is. All 3 bits tested @ $be48
	PORT_DIPSETTING(      0x0004, DEF_STR( Easiest ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( Easier ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Harder ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPUNKNOWN( 0x0008, 0x0008 )  // $448a
	PORT_DIPNAME( 0x0030, 0x0020, DEF_STR( Lives ) ) // $be24
	PORT_DIPSETTING(      0x0020, "3" )
	PORT_DIPSETTING(      0x0030, "2" )
	PORT_DIPSETTING(      0x0010, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Gfx Viewer" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

/***************************************************************************


                            Graphics Layouts


***************************************************************************/


// 16x16x4
static const gfx_layout layout_16x16x4 =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(0,4) },
	{ STEP16(0,1) },
	{ STEP16(0,16) },
	16*16
};

// 16x16x8
static const gfx_layout layout_16x16x8 =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 6*8,4*8, 2*8,0*8, 7*8,5*8, 3*8,1*8 },
	{ STEP8(0,1),STEP8(8*8,1) },
	{ STEP16(0,16*8) },
	16*16*8
};


static GFXDECODE_START( gfx_yunsun16 )
	GFXDECODE_ENTRY( "tiles",   0, layout_16x16x8, 0x1000, 0x10 )
	GFXDECODE_ENTRY( "sprites", 0, layout_16x16x4, 0x0000, 0x20 )
GFXDECODE_END


/***************************************************************************


                                Machine Drivers


***************************************************************************/

void yunsun16_state::machine_start()
{
	save_item(NAME(m_sprites_scrolldx));
	save_item(NAME(m_sprites_scrolldy));
}

void yunsun16_state::machine_reset()
{
	m_sprites_scrolldx = -0x40;
	m_sprites_scrolldy = -0x0f;
}

void shocking_state::machine_start()
{
	yunsun16_state::machine_start();
	m_okibank->configure_entries(0, 0x80000 / 0x20000, memregion("oki")->base(), 0x20000);
}

void shocking_state::machine_reset()
{
	yunsun16_state::machine_reset();
	m_okibank->set_entry(0);
}

/***************************************************************************
                                Magic Bubble
***************************************************************************/

void magicbub_state::magicbub(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(16'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &magicbub_state::main_map);

	z80_device &audiocpu(Z80(config, "audiocpu", XTAL(16'000'000) / 4));
	audiocpu.set_addrmap(AS_PROGRAM, &magicbub_state::sound_map);
	audiocpu.set_addrmap(AS_IO, &magicbub_state::sound_port_map);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(16'000'000)/2, 512, 0x20, 0x180-0x20, 260, 0, 0xe0); // TODO: completely inaccurate
	m_screen->set_screen_update(FUNC(magicbub_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set_inputline(m_maincpu, M68K_IRQ_2, ASSERT_LINE);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_yunsun16);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 8192);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);

	ym3812_device &ymsnd(YM3812(config, "ymsnd", XTAL(16'000'000) / 4));
	ymsnd.irq_handler().set_inputline("audiocpu", 0);
	ymsnd.add_route(ALL_OUTPUTS, "lspeaker", 0.80);
	ymsnd.add_route(ALL_OUTPUTS, "rspeaker", 0.80);

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(16'000'000) / 16, okim6295_device::PIN7_HIGH));
	oki.add_route(ALL_OUTPUTS, "lspeaker", 0.80);
	oki.add_route(ALL_OUTPUTS, "rspeaker", 0.80);
}


/***************************************************************************
                                Shocking
***************************************************************************/

void shocking_state::shocking(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(16'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &shocking_state::main_map);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(16'000'000)/2, 512, 0, 0x180-4, 260, 0, 0xe0); // TODO: completely inaccurate
	m_screen->set_screen_update(FUNC(shocking_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set_inputline(m_maincpu, M68K_IRQ_2, ASSERT_LINE);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_yunsun16);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 8192);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(16'000'000) / 16, okim6295_device::PIN7_HIGH));
	oki.add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	oki.add_route(ALL_OUTPUTS, "rspeaker", 1.0);
	oki.set_addrmap(0, &shocking_state::oki_map);
}


/***************************************************************************


                                ROMs Loading


***************************************************************************/

/***************************************************************************

                                Magic Bubble

Yun Sung 199x

YunSung YS1302

PCB Layout
----------

+-------------------------------------------------+
|             u131               u20     6116     |
| YM3014 6116  Z80 M6295         u22     6116     |
| YM3812 u143                    u21              |
|      62256                     u23     6116     |
|      62256                             6116     |
|                  PAL                      PAL   |
|J                 PAL                      PAL   |
|A  DSW1                       PAL                |
|M                                          PAL   |
|M  DSW2                PAL           6116  6116  |
|A        PAL           PAL   ACTEL   6116  6116  |
|         PAL  PAL      PAL   A1020B     u70 u74  |
|              PAL                       u69 u73  |
|                62256                   u68 u72  |
|                62256                   u67 u71  |
|        68000   u32                62256  62256  |
|16MHz           u33                              |
+-------------------------------------------------+


U143 -------------------27c512
U23, 21, 22, 20, 131 ---27c010
U67, 68, 69, 70 --------27c040
U32, 33 ----------------27c020

U143, 131 .......Sound CPU code & Samples
U32, 33 .........Program code
U20-23 ..........Sprites
U67-70 ..........Backgrounds

Actel A1020B is close to U67-70

68HC000P16 is close to  U32,33

16.000000 MHz

Sound section:
 SMD Z80
  "KS8001" (YM3812)
  "KS8002" (YM3014)
  "AD-65"  (OKI M6295 and is SMD)

***************************************************************************/

ROM_START( magicbub ) // YS1302 PCB

	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "magbuble.u33", 0x000000, 0x040000, CRC(18fdd582) SHA1(89f4c52ec0e213285a04743da88f6e39408b573d) )
	ROM_LOAD16_BYTE( "magbuble.u32", 0x000001, 0x040000, CRC(f6ea7004) SHA1(069541e37b60370810451616ee66bbd05dc10137) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        // Z80 Code
	ROM_LOAD( "u143.bin", 0x00000, 0x10000, CRC(04192753) SHA1(9c56ba70e1d074906ea1dc593c2a8516c6ba2074) )

	ROM_REGION( 0x200000*8, "tiles", ROMREGION_ERASEFF ) // 16x16x8
	ROM_LOAD64_WORD( "magbuble.u67", 0x000000, 0x080000, CRC(6355e57d) SHA1(5e9234dd474ddcf0a9e1001080f3de11c7d0ee55) )
	ROM_LOAD64_WORD( "magbuble.u68", 0x000002, 0x080000, CRC(53ae6c2b) SHA1(43c02aa4cfdfa5bc009b42cd4be633787a35cb59) )
	ROM_LOAD64_WORD( "magbuble.u69", 0x000004, 0x080000, CRC(b892e64c) SHA1(b1156c8f02371ee2c5d6c930483c50eef5da10b5) )
	ROM_LOAD64_WORD( "magbuble.u70", 0x000006, 0x080000, CRC(37794837) SHA1(11597614e1e048544326fbbe281b364278d6350d) )

	ROM_REGION( 0x080000, "sprites", 0 )   // 16x16x4
	ROM_LOAD( "u20.bin", 0x000000, 0x020000, CRC(f70e3b8c) SHA1(d925c27bbd0f915228d22589a98e3ea7181a87ca) )
	ROM_LOAD( "u21.bin", 0x020000, 0x020000, CRC(ad082cf3) SHA1(0bc3cf6c54d47be4f1940192fc1585cb48767e97) )
	ROM_LOAD( "u22.bin", 0x040000, 0x020000, CRC(7c68df7a) SHA1(88acf9dd43892a790415b418f77d88c747aa84f5) )
	ROM_LOAD( "u23.bin", 0x060000, 0x020000, CRC(c7763fc1) SHA1(ed68b3c3c5155073afb7b55d6d92d3057e40df6c) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "u131.bin", 0x000000, 0x020000, CRC(03e04e89) SHA1(7d80e6a7be2322e32e40acae72bedd8d7e90ad33) )

ROM_END

ROM_START( magicbuba ) // YS1302 PCB

	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "u33.bin", 0x000000, 0x040000, CRC(a8164a02) SHA1(7275209d5d73881839f7fa3ac7d362194ef2cfd9) )
	ROM_LOAD16_BYTE( "u32.bin", 0x000001, 0x040000, CRC(58f885ad) SHA1(e66f5bb1ac0acd9abc2def439af7f932c3a09cbd) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        // Z80 Code
	ROM_LOAD( "u143.bin", 0x00000, 0x10000, CRC(04192753) SHA1(9c56ba70e1d074906ea1dc593c2a8516c6ba2074) )

	ROM_REGION( 0x200000*8, "tiles", ROMREGION_ERASEFF ) // 16x16x8
	ROM_LOAD64_WORD( "u67.bin", 0x000000, 0x080000, CRC(89523dcd) SHA1(edea2bbec615aa253d940bbc3bbdb33f6873a8ee) )
	ROM_LOAD64_WORD( "u68.bin", 0x000002, 0x080000, CRC(30e01a70) SHA1(3a98c2ef61307b44bf4e155663117199587ff4a4) )
	ROM_LOAD64_WORD( "u69.bin", 0x000004, 0x080000, CRC(fe357f52) SHA1(5aff9a0bf70fc8a78820c4d13838ad238852c594) )
	ROM_LOAD64_WORD( "u70.bin", 0x000006, 0x080000, CRC(1398a473) SHA1(f58bda6cbf5f553a9632d910b2ffef5d5bfedf18) )
	ROM_LOAD64_WORD( "u71.bin", 0x200000, 0x080000, CRC(0844e017) SHA1(2ae5c9da521fea7aa5811627d7b3eca82cdc0821) )
	ROM_LOAD64_WORD( "u72.bin", 0x200002, 0x080000, CRC(591db1cb) SHA1(636fbfe9e048d6418d43f947004b281f61081fd8) )
	ROM_LOAD64_WORD( "u73.bin", 0x200004, 0x080000, CRC(cb4f3c3c) SHA1(fbd804bb70f09c2471557675af4c5b4abedea3b2) )
	ROM_LOAD64_WORD( "u74.bin", 0x200006, 0x080000, CRC(81ff4910) SHA1(69241fe2d20b53984aa67f17d8da32e1b74ce696) )

	ROM_REGION( 0x080000, "sprites", 0 )   // 16x16x4
	ROM_LOAD( "u20.bin", 0x000000, 0x020000, CRC(f70e3b8c) SHA1(d925c27bbd0f915228d22589a98e3ea7181a87ca) )
	ROM_LOAD( "u21.bin", 0x020000, 0x020000, CRC(ad082cf3) SHA1(0bc3cf6c54d47be4f1940192fc1585cb48767e97) )
	ROM_LOAD( "u22.bin", 0x040000, 0x020000, CRC(7c68df7a) SHA1(88acf9dd43892a790415b418f77d88c747aa84f5) )
	ROM_LOAD( "u23.bin", 0x060000, 0x020000, CRC(c7763fc1) SHA1(ed68b3c3c5155073afb7b55d6d92d3057e40df6c) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "u131.bin", 0x000000, 0x020000, CRC(03e04e89) SHA1(7d80e6a7be2322e32e40acae72bedd8d7e90ad33) )

ROM_END

ROM_START( magicbubb ) // YS1302 PCB

	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "p2.u33", 0x000000, 0x040000, CRC(24e7e2b4) SHA1(9cceef7ee7eb909ab2f4f5438695d62e2448f142) )
	ROM_LOAD16_BYTE( "p1.u32", 0x000001, 0x040000, CRC(0fa8b089) SHA1(acac9879a5c2ba34fa71af00907676b1f4a81b16) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        // Z80 Code
	ROM_LOAD( "u143.bin", 0x00000, 0x10000, CRC(04192753) SHA1(9c56ba70e1d074906ea1dc593c2a8516c6ba2074) )

	ROM_REGION( 0x200000*8, "tiles", ROMREGION_ERASEFF ) // 16x16x8
	ROM_LOAD64_WORD( "u67.bin", 0x000000, 0x080000, CRC(89523dcd) SHA1(edea2bbec615aa253d940bbc3bbdb33f6873a8ee) )
	ROM_LOAD64_WORD( "u68.bin", 0x000002, 0x080000, CRC(30e01a70) SHA1(3a98c2ef61307b44bf4e155663117199587ff4a4) )
	ROM_LOAD64_WORD( "u69.bin", 0x000004, 0x080000, CRC(fe357f52) SHA1(5aff9a0bf70fc8a78820c4d13838ad238852c594) )
	ROM_LOAD64_WORD( "u70.bin", 0x000006, 0x080000, CRC(1398a473) SHA1(f58bda6cbf5f553a9632d910b2ffef5d5bfedf18) )
	ROM_LOAD64_WORD( "u71.bin", 0x200000, 0x080000, CRC(0844e017) SHA1(2ae5c9da521fea7aa5811627d7b3eca82cdc0821) )
	ROM_LOAD64_WORD( "u72.bin", 0x200002, 0x080000, CRC(591db1cb) SHA1(636fbfe9e048d6418d43f947004b281f61081fd8) )
	ROM_LOAD64_WORD( "u73.bin", 0x200004, 0x080000, CRC(cb4f3c3c) SHA1(fbd804bb70f09c2471557675af4c5b4abedea3b2) )
	ROM_LOAD64_WORD( "u74.bin", 0x200006, 0x080000, CRC(81ff4910) SHA1(69241fe2d20b53984aa67f17d8da32e1b74ce696) )

	ROM_REGION( 0x080000, "sprites", 0 )   // 16x16x4
	ROM_LOAD( "1.u20", 0x000000, 0x020000, CRC(d3bba963) SHA1(39f04c8cb7f3c43a4df15a4a326c360a3c1e617c) )
	ROM_LOAD( "3.u21", 0x020000, 0x020000, CRC(0017f6d3) SHA1(305aea32936babc86f60aac08f0e6a6af8b132ee) )
	ROM_LOAD( "2.u22", 0x040000, 0x020000, CRC(7d71f838) SHA1(61b6e3af9a1b89b90297fe9c164e4ba62730caf0) )
	ROM_LOAD( "4.u23", 0x060000, 0x020000, CRC(ecee1f63) SHA1(7a94ca5b749dc3fc302b28dbeab7e756167b0793) )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "u131", 0x000000, 0x040000, CRC(9bdb08e4) SHA1(4d8bdeb9b503b0959a6ae3f3fb3574350b01b1a1) )

ROM_END

ROM_START( magicbubc ) // Found on a YS-0211 PCB like below

	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "u33", 0x000000, 0x040000, CRC(db651555) SHA1(41dbf35147e1c646db585437b378529559d3decb) )
	ROM_LOAD16_BYTE( "u32", 0x000001, 0x040000, CRC(c9cb4d88) SHA1(ee41b9b307b423db7a9d706dfa9718efefa3b625) )

	ROM_REGION( 0x200000*8, "tiles", ROMREGION_ERASEFF ) // 16x16x8
	ROM_LOAD64_WORD( "u67.bin", 0x000000, 0x080000, CRC(89523dcd) SHA1(edea2bbec615aa253d940bbc3bbdb33f6873a8ee) )
	ROM_LOAD64_WORD( "u68.bin", 0x000002, 0x080000, CRC(30e01a70) SHA1(3a98c2ef61307b44bf4e155663117199587ff4a4) )
	ROM_LOAD64_WORD( "u69.bin", 0x000004, 0x080000, CRC(fe357f52) SHA1(5aff9a0bf70fc8a78820c4d13838ad238852c594) )
	ROM_LOAD64_WORD( "u70.bin", 0x000006, 0x080000, CRC(1398a473) SHA1(f58bda6cbf5f553a9632d910b2ffef5d5bfedf18) )
	ROM_LOAD64_WORD( "u71.bin", 0x200000, 0x080000, CRC(0844e017) SHA1(2ae5c9da521fea7aa5811627d7b3eca82cdc0821) )
	ROM_LOAD64_WORD( "u72.bin", 0x200002, 0x080000, CRC(591db1cb) SHA1(636fbfe9e048d6418d43f947004b281f61081fd8) )
	ROM_LOAD64_WORD( "u73.bin", 0x200004, 0x080000, CRC(cb4f3c3c) SHA1(fbd804bb70f09c2471557675af4c5b4abedea3b2) )
	ROM_LOAD64_WORD( "u74.bin", 0x200006, 0x080000, CRC(81ff4910) SHA1(69241fe2d20b53984aa67f17d8da32e1b74ce696) )

	ROM_REGION( 0x080000, "sprites", 0 )   // 16x16x4
	ROM_LOAD( "u20.bin", 0x000000, 0x020000, CRC(f70e3b8c) SHA1(d925c27bbd0f915228d22589a98e3ea7181a87ca) )
	ROM_LOAD( "u21.bin", 0x020000, 0x020000, CRC(ad082cf3) SHA1(0bc3cf6c54d47be4f1940192fc1585cb48767e97) )
	ROM_LOAD( "u22.bin", 0x040000, 0x020000, CRC(7c68df7a) SHA1(88acf9dd43892a790415b418f77d88c747aa84f5) )
	ROM_LOAD( "u23.bin", 0x060000, 0x020000, CRC(c7763fc1) SHA1(ed68b3c3c5155073afb7b55d6d92d3057e40df6c) )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "u131", 0x000000, 0x040000, CRC(9bdb08e4) SHA1(4d8bdeb9b503b0959a6ae3f3fb3574350b01b1a1) )

ROM_END


/***************************************************************************

YunSung YS-0211 based games:

Paparazzi (c) 1996 (no PCB label but looks identical)
Shocking  (c) 1997
Bomb Kick (c) 1998
Magic Bubble (c) 199?

PCB Layout
----------

|-------------------------------------------------|
|UPC1242    u131               u20       6116     |
|     VOL M6295                u22       6116     |
|  PAL 6264                    u21                |
|      6264                    u23       6116     |
|                                        6116     |
|                  PAL                      PAL   |
|J                 PAL                      PAL   |
|A  DSW1                       PAL                |
|M                                                |
|M  DSW2  PAL           PAL                 PAL   |
|A        PAL           PAL   ACTEL   6116  6116  |
|              PAL      PAL   A1020B  6116  6116  |
|          6   PAL                      u70 u74   |
|          8     62256                  u69 u73   |
|          0     62256                  u68 u72   |
|          0      u32                   u67 u71   |
|16MHz     0      u33              62256  62256   |
|-------------------------------------------------|
Notes:
      68000 clock - 16MHz
      M6295 clock - 1.000MHz, sample rate 1000000Hz / 132
      HSync - 14.84kHz
      VSync - 60Hz

***************************************************************************/

/***************************************************************************

                       Paparazzi  -  Yun Sung, 1996

***************************************************************************/

ROM_START( paprazzi )

	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "u33.bin", 0x000000, 0x020000, CRC(91f33abd) SHA1(694868bc1ef612ba47cb38957d965f271bf16105) )
	ROM_LOAD16_BYTE( "u32.bin", 0x000001, 0x020000, CRC(ad5a3fec) SHA1(a2db3f2926bdbb5bc44f307b919a0431c9deb76d) )

	ROM_REGION( 0x200000*8, "tiles", ROMREGION_ERASEFF ) // 16x16x8
	ROM_LOAD64_WORD( "u67.bin", 0x000000, 0x080000, CRC(ea0b9e27) SHA1(e68f728158d0c42523002fe4270784891f5492ce) )
	ROM_LOAD64_WORD( "u68.bin", 0x000002, 0x080000, CRC(6b7ff4dd) SHA1(b06036f08e8f65860077a71d91676bf5c2f804fc) )
	ROM_LOAD64_WORD( "u69.bin", 0x000004, 0x080000, CRC(06749294) SHA1(375fe1c05355f789f846aa28b2012d08bfa2b2b5) )
	ROM_LOAD64_WORD( "u70.bin", 0x000006, 0x080000, CRC(0adacdf8) SHA1(d33680e7139e78929284b81e880bd5baa45c6675) )
	ROM_LOAD64_WORD( "u71.bin", 0x200000, 0x080000, CRC(69178fc4) SHA1(1ec06d360e098e15cfb673e5de7124a7c10757f8) )
	ROM_LOAD64_WORD( "u72.bin", 0x200002, 0x080000, CRC(7c3384b9) SHA1(b9e1ba7ec009e15f1061c3994ed4cf48a8e700c6) )
	ROM_LOAD64_WORD( "u73.bin", 0x200004, 0x080000, CRC(73fbc13e) SHA1(a19a05764ca010be025aae12fa82f97f5dc7d4b9) )
	ROM_LOAD64_WORD( "u74.bin", 0x200006, 0x080000, CRC(f1afda11) SHA1(c62e318dde2ed7ac9b649764ccec8e991d2869c2) )

	ROM_REGION( 0x100000, "sprites", 0 )   // 16x16x4
	ROM_LOAD( "u20.bin", 0x000000, 0x040000, CRC(ccb0ad6b) SHA1(ca66b7c7cb1418a86f209d071935aa45bb0a6e7d) )
	ROM_LOAD( "u21.bin", 0x040000, 0x040000, CRC(125badf0) SHA1(ae63469e1fb1328c554774ca8c47878df2b02b96) )
	ROM_LOAD( "u22.bin", 0x080000, 0x040000, CRC(436499c7) SHA1(ec1390b6d5656c99d91cf6425d319f4796bcb28a) )
	ROM_LOAD( "u23.bin", 0x0c0000, 0x040000, CRC(358280fe) SHA1(eac3cb65fe75bc2da14896734f4a339480b54a2c) )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "u131.bin", 0x000000, 0x080000, CRC(bcf7aa12) SHA1(f7bf5258396ed0eb7e85eccf250c6d0a333a4d61) )

ROM_END

/***************************************************************************

                       Shocking  -  Yun Sung, 1997

***************************************************************************/

ROM_START( shocking )

	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "yunsun16.u33", 0x000000, 0x040000, CRC(8a155521) SHA1(000c9095558e6cae30ce43a885c3fbcf55713f40) )
	ROM_LOAD16_BYTE( "yunsun16.u32", 0x000001, 0x040000, CRC(c4998c10) SHA1(431ae1f9982a70421650e1bfe4bf87152e2fe85c) )

	ROM_REGION( 0x200000*8, "tiles", ROMREGION_ERASEFF ) // 16x16x8
	ROM_LOAD64_WORD( "yunsun16.u67", 0x000000, 0x080000, CRC(e30fb2c4) SHA1(0d33a1593d7ebcd5da6971a04c3300c0b4eef219) )
	ROM_LOAD64_WORD( "yunsun16.u68", 0x000002, 0x080000, CRC(7d702538) SHA1(ae4c8ca6f172e204589f2f70ca114f7c38e7cabd) )
	ROM_LOAD64_WORD( "yunsun16.u69", 0x000004, 0x080000, CRC(97447fec) SHA1(e52184f96b2337ccbef130ada21a959c8bc1d73b) )
	ROM_LOAD64_WORD( "yunsun16.u70", 0x000006, 0x080000, CRC(1b1f7895) SHA1(939c386dbef82e4833b7038e7c603d2ec67fa23e) )

	ROM_REGION( 0x100000, "sprites", 0 )   // 16x16x4
	ROM_LOAD( "yunsun16.u20", 0x000000, 0x040000, CRC(124699d0) SHA1(e55c8fb35f193abf98b1df07b94b99bf33bb5207) )
	ROM_LOAD( "yunsun16.u21", 0x040000, 0x040000, CRC(4eea29a2) SHA1(c8173eeef0228a7635a96251ae3776726ffaf0f4) )
	ROM_LOAD( "yunsun16.u22", 0x080000, 0x040000, CRC(d6db0388) SHA1(f5d8f7740b602c402a8dd6c4ebd357cf15a0dfac) )
	ROM_LOAD( "yunsun16.u23", 0x0c0000, 0x040000, CRC(1fa33b2e) SHA1(4aa0dee8d34aac19cf6b7ba3f79ca022ad8d7760) )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "yunsun16.131", 0x000000, 0x080000, CRC(d0a1bb8c) SHA1(10f33521bd6031ed73ee5c7be1382165925aa8f8) )

ROM_END

ROM_START( shockingk )

	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "u33.bin", 0x000000, 0x040000, CRC(870108ad) SHA1(2d059ee0f189ed404211f6041cd382f90b53d0cd) )
	ROM_LOAD16_BYTE( "u32.bin", 0x000001, 0x040000, CRC(be2125f4) SHA1(fab38697266a1f95b8ebfff0c692d8e8239710aa) )

	ROM_REGION( 0x200000*8, "tiles", ROMREGION_ERASEFF ) // 16x16x8
	ROM_LOAD64_WORD( "u67.bin", 0x000000, 0x080000, CRC(7b0f3944) SHA1(0954610e0a1b39e8e68411b98c3fe487da6bd77a) )
	ROM_LOAD64_WORD( "u68.bin", 0x000002, 0x080000, CRC(aa736da6) SHA1(0d8bbfc1fb014c6e662e4dc376bcd87b4157a7aa) )
	ROM_LOAD64_WORD( "u69.bin", 0x000004, 0x080000, CRC(292bb626) SHA1(78a7ecc72dde6d397d2137e528dabcd247d382bd) )
	ROM_LOAD64_WORD( "u70.bin", 0x000006, 0x080000, CRC(2f9eeb81) SHA1(4e84c4451cbe3feee95a828790830e95f278f2e7) )

	ROM_REGION( 0x100000, "sprites", 0 )   // 16x16x4
	ROM_LOAD( "u20.bin", 0x000000, 0x040000, CRC(3502a477) SHA1(f317c12491b35470ceb178793c6e332c3afcf2b5) )
	ROM_LOAD( "u21.bin", 0x040000, 0x040000, CRC(ffe0af85) SHA1(124d8375fd366333fb3cb16bb94d7fa3c79534b3) )
	ROM_LOAD( "u22.bin", 0x080000, 0x040000, CRC(59260de1) SHA1(2dd2d7ab93fa751cb9142400a3ff91391477d555) )
	ROM_LOAD( "u23.bin", 0x0c0000, 0x040000, CRC(00e4af23) SHA1(a4d23f16748385dd8c87cae3e16593e5a0195c24) )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "yunsun16.131", 0x000000, 0x080000, CRC(d0a1bb8c) SHA1(10f33521bd6031ed73ee5c7be1382165925aa8f8) )

ROM_END

ROM_START( shockingko )

	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "shoc_kor.u33", 0x000000, 0x040000, CRC(646303ec) SHA1(d01264f8495fdea882a9d75129665a67a9acfc42) )
	ROM_LOAD16_BYTE( "shoc_kor.u32", 0x000001, 0x040000, CRC(6d9ac2f2) SHA1(2374cc053233940d5da610ec95539b43dfbeef3b) )

	ROM_REGION( 0x200000*8, "tiles", ROMREGION_ERASEFF ) // 16x16x8
	ROM_LOAD64_WORD( "shoc_kor.u67", 0x000000, 0x080000, CRC(e30fb2c4) SHA1(0d33a1593d7ebcd5da6971a04c3300c0b4eef219) )
	ROM_LOAD64_WORD( "shoc_kor.u68", 0x000002, 0x080000, CRC(7d702538) SHA1(ae4c8ca6f172e204589f2f70ca114f7c38e7cabd) )
	ROM_LOAD64_WORD( "shoc_kor.u69", 0x000004, 0x080000, CRC(97447fec) SHA1(e52184f96b2337ccbef130ada21a959c8bc1d73b) )
	ROM_LOAD64_WORD( "shoc_kor.u70", 0x000006, 0x080000, CRC(1b1f7895) SHA1(939c386dbef82e4833b7038e7c603d2ec67fa23e) )

	ROM_REGION( 0x100000, "sprites", 0 )   // 16x16x4
	ROM_LOAD( "shoc_kor.u20", 0x000000, 0x040000, CRC(9f729220) SHA1(3206c87c7aebd8912d3486225ccae0a6e3b2061e) )
	ROM_LOAD( "shoc_kor.u21", 0x040000, 0x040000, CRC(cde84679) SHA1(261a6570449bce22458c49edee427dda6dc504b7) )
	ROM_LOAD( "shoc_kor.u22", 0x080000, 0x040000, CRC(61fe98ab) SHA1(745fe3b9d513b8e10c405d9ba2e055de1a261e33) )
	ROM_LOAD( "shoc_kor.u23", 0x0c0000, 0x040000, CRC(50c29191) SHA1(bb2c22f2f452ca0940e98df6efc754c7522696bd) )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "yunsun16.131", 0x000000, 0x080000, CRC(d0a1bb8c) SHA1(10f33521bd6031ed73ee5c7be1382165925aa8f8) )

ROM_END


/***************************************************************************

                       Bomb Kick  -  Yun Sung, 1998

   Title screen shows 1998, but service mode shows 1997 for both sets.

***************************************************************************/

ROM_START( bombkick )

	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "bk_u33", 0x000000, 0x040000, CRC(d6eb50bf) SHA1(a24c31f212f86f066c35d39da137ef0933323e43) )
	ROM_LOAD16_BYTE( "bk_u32", 0x000001, 0x040000, CRC(d55388a2) SHA1(928f1a8933b986cf099e184002660e30ee1aeb0a) )

	ROM_REGION( 0x200000*8, "tiles", ROMREGION_ERASEFF ) // 16x16x8
	ROM_LOAD64_WORD( "bk_u67", 0x000000, 0x080000, CRC(1962f536) SHA1(36d3c73a322330058e963efcb9b81324724382cc) )
	ROM_LOAD64_WORD( "bk_u68", 0x000002, 0x080000, CRC(d80c75a4) SHA1(330c20d126b9f1f61f17750028c92843be55ec78) )
	ROM_LOAD64_WORD( "bk_u69", 0x000004, 0x080000, CRC(615e1e6f) SHA1(73875313010514ff5ca9e0bc96d6f93baaee391e) )
	ROM_LOAD64_WORD( "bk_u70", 0x000006, 0x080000, CRC(59817ef1) SHA1(d23df30b34223575d6a9c814f2ec3db990b18679) )

	ROM_REGION( 0x100000, "sprites", 0 )   // 16x16x4
	ROM_LOAD( "bk_u20", 0x000000, 0x040000, CRC(c2b83e3f) SHA1(8bcd862dbf56cf579058d045f89f900ebfea2f1d) )
	ROM_LOAD( "bk_u21", 0x040000, 0x040000, CRC(d6890192) SHA1(3c26a08580ceecf2f61f008861a459e175c99ed9) )
	ROM_LOAD( "bk_u22", 0x080000, 0x040000, CRC(9538c46c) SHA1(d7d0e167d5abc2ee81eae6fde152b2f5cc716c0e) )
	ROM_LOAD( "bk_u23", 0x0c0000, 0x040000, CRC(e3831f3d) SHA1(096658ee5a7b83d774b671c0a38113533c8751d1) )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "bk_u131", 0x000000, 0x080000, CRC(22cc5732) SHA1(38aefa4e543ea54e004eee428ee087121eb20905) )

ROM_END

ROM_START( bombkicka ) // marked 'Bomb Kick 98'

	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "u33.bin", 0x000000, 0x040000, CRC(4624d618) SHA1(4d9862740e1f759860eeedf56efd16e4bfdc3376) )
	ROM_LOAD16_BYTE( "u32.bin", 0x000001, 0x040000, CRC(c5a105f3) SHA1(937fb780c5e0f635a03097263f68cad0732f3a21) )

	ROM_REGION( 0x200000*8, "tiles", ROMREGION_ERASEFF ) // 16x16x8
	ROM_LOAD64_WORD( "bk_u67", 0x000000, 0x080000, CRC(1962f536) SHA1(36d3c73a322330058e963efcb9b81324724382cc) )
	ROM_LOAD64_WORD( "bk_u68", 0x000002, 0x080000, CRC(d80c75a4) SHA1(330c20d126b9f1f61f17750028c92843be55ec78) )
	ROM_LOAD64_WORD( "bk_u69", 0x000004, 0x080000, CRC(615e1e6f) SHA1(73875313010514ff5ca9e0bc96d6f93baaee391e) )
	ROM_LOAD64_WORD( "bk_u70", 0x000006, 0x080000, CRC(59817ef1) SHA1(d23df30b34223575d6a9c814f2ec3db990b18679) )

	ROM_REGION( 0x100000, "sprites", 0 )   // 16x16x4
	ROM_LOAD( "bk_u20", 0x000000, 0x040000, CRC(c2b83e3f) SHA1(8bcd862dbf56cf579058d045f89f900ebfea2f1d) )
	ROM_LOAD( "bk_u21", 0x040000, 0x040000, CRC(d6890192) SHA1(3c26a08580ceecf2f61f008861a459e175c99ed9) )
	ROM_LOAD( "bk_u22", 0x080000, 0x040000, CRC(9538c46c) SHA1(d7d0e167d5abc2ee81eae6fde152b2f5cc716c0e) )
	ROM_LOAD( "bk_u23", 0x0c0000, 0x040000, CRC(e3831f3d) SHA1(096658ee5a7b83d774b671c0a38113533c8751d1) )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "bk_u131", 0x000000, 0x080000, CRC(22cc5732) SHA1(38aefa4e543ea54e004eee428ee087121eb20905) )

ROM_END

} // anonymous namespace


/***************************************************************************


                                Game Drivers


***************************************************************************/

GAME( 199?, magicbub,   0,        magicbub, magicbub, magicbub_state, empty_init, ROT0,   "Yun Sung", "Magic Bubble",                                     MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 199?, magicbuba,  magicbub, magicbub, magicbua, magicbub_state, empty_init, ROT0,   "Yun Sung", "Magic Bubble (Adult version, YS1302 PCB, set 1)",  MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 199?, magicbubb,  magicbub, magicbub, magicbua, magicbub_state, empty_init, ROT0,   "Yun Sung", "Magic Bubble (Adult version, YS1302 PCB, set 2)",  MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 199?, magicbubc,  magicbub, shocking, magicbua, shocking_state, empty_init, ROT0,   "Yun Sung", "Magic Bubble (Adult version, YS-0211 PCB)",        MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1996, paprazzi,   0,        shocking, paprazzi, shocking_state, empty_init, ROT270, "Yun Sung", "Paparazzi",                                        MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1997, shocking,   0,        shocking, shocking, shocking_state, empty_init, ROT0,   "Yun Sung", "Shocking",                                         MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1997, shockingk,  shocking, shocking, shocking, shocking_state, empty_init, ROT0,   "Yun Sung", "Shocking (Korea, set 1)",                          MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1997, shockingko, shocking, shocking, shocking, shocking_state, empty_init, ROT0,   "Yun Sung", "Shocking (Korea, set 2)",                          MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1998, bombkick,   0,        shocking, bombkick, shocking_state, empty_init, ROT0,   "Yun Sung", "Bomb Kick (set 1)",                                MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1998, bombkicka,  bombkick, shocking, bombkick, shocking_state, empty_init, ROT0,   "Yun Sung", "Bomb Kick (set 2)",                                MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
