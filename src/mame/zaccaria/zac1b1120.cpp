// license:BSD-3-Clause
// copyright-holders: Mike Coates

/*
 * Zaccaria 1B1120 hardware (Signetics 2650 based)
 *
 * Zaccaria - The Invaders
 * Sidam    - Super Invader Attack
 * Zaccaria - Dodgem
 *
 * The IC at location 48 in the upper right corner (lower 4 bits of 256-byte RAM)
 * was originally intended to be a battery-backed 5101. However, all known games
 * leave the battery unpopulated (Invaders schematics say "OMIT") and place a
 * 2101 RAM there instead.
 *
 * Sound on these games is generated partly by the 2636 and partly by a
 * daughterboard containing discrete components, TTL logic, and a SN76477
 * (The Invaders only).
 *
 * A later revision of The Invaders, apparently cocktail only, runs on a single
 * board labeled 1B1124/1. The discrete sound hardware is different here again.
 *
 *
 * TODO: discrete sound
 *
 */

#include "emu.h"

#include "cpu/s2650/s2650.h"
#include "machine/s2636.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "dodgem.lh"
#include "tinv2650.lh"


namespace {

class zac1b1120_state : public driver_device
{
public:
	zac1b1120_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_s2636(*this, "s2636"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_s2636_0_ram(*this, "s2636_0_ram")
	{ }

	void tinvader(machine_config &config);
	void dodgem(machine_config &config);

	ioport_value bg_collision_r() { return m_collision_background; }

protected:
	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<s2636_device> m_s2636;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	// memory pointers
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_s2636_0_ram;

	bitmap_ind16 m_bitmap;
	bitmap_ind16 m_spritebitmap;
	uint8_t m_collision_background = 0;
	uint8_t m_collision_sprite = 0;
	tilemap_t *m_bg_tilemap = nullptr;

	void sound_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	uint8_t s2636_r(offs_t offset);
	void s2636_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int sprite_collision(int first, int second);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
};


/*******************************************************************************
    Video
*******************************************************************************/

/*

Video is slightly odd on these zac boards

background is 256 x 240 pixels, but the sprite chips run at a different frequency, which means
that the output of 196x240 is stretched to fill the same screen space.

to 'properly' accomplish this, we set the screen up as 768x720 and do the background at 3 times
the size, and the sprites as 4 times the size - everything then matches up correctly.

*/

void zac1b1120_state::palette(palette_device &palette) const
{
	// FIXME: this PCB actually outputs RGB signals, not monochrome video!
	palette.set_pen_color(0, rgb_t::black());
	palette.set_pen_color(1, rgb_t::white());
	palette.set_pen_color(2, rgb_t::black());
	palette.set_pen_color(3, rgb_t::black());
}

TILE_GET_INFO_MEMBER(zac1b1120_state::get_bg_tile_info)
{
	tileinfo.set(0, m_videoram[tile_index], 0, 0);
}

void zac1b1120_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(zac1b1120_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8 * 3, 8, 32, 32);

	m_screen->register_screen_bitmap(m_bitmap);
	m_screen->register_screen_bitmap(m_spritebitmap);

	m_gfxdecode->gfx(1)->set_source(m_s2636_0_ram);
	m_gfxdecode->gfx(2)->set_source(m_s2636_0_ram);

	save_item(NAME(m_collision_background));
	save_item(NAME(m_collision_sprite));
}

void zac1b1120_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

uint8_t zac1b1120_state::s2636_r(offs_t offset)
{
	if (offset != 0xcb)
		return m_s2636_0_ram[offset];
	else
		return m_collision_sprite;
}

void zac1b1120_state::s2636_w(offs_t offset, uint8_t data)
{
	m_s2636_0_ram[offset] = data;
	m_gfxdecode->gfx(1)->mark_dirty(offset / 8);
	m_gfxdecode->gfx(2)->mark_dirty(offset / 8);

	if (offset == 0xc7)
		m_s2636->write_data(offset, data);
}

int zac1b1120_state::sprite_collision(int first, int second)
{
	int checksum = 0;
	const rectangle &visarea = m_screen->visible_area();

	if ((m_s2636_0_ram[first * 0x10 + 10] < 0xf0) && (m_s2636_0_ram[second * 0x10 + 10] < 0xf0))
	{
		int const fx = (m_s2636_0_ram[first * 0x10 + 10] * 4) - 22;
		int const fy = m_s2636_0_ram[first * 0x10 + 12] + 1;
		int const expand = (first == 1) ? 2 : 1;

		// Draw first sprite
		m_gfxdecode->gfx(expand)->opaque(m_spritebitmap, m_spritebitmap.cliprect(),
				first * 2,
				0,
				0, 0,
				fx, fy);

		// Get fingerprint
		for (int x = fx; x < fx + m_gfxdecode->gfx(expand)->width(); x++)
		{
			for (int y = fy; y < fy + m_gfxdecode->gfx(expand)->height(); y++)
			{
				if (visarea.contains(x, y))
					checksum += m_spritebitmap.pix(y, x);
			}
		}

		// Blackout second sprite
		m_gfxdecode->gfx(1)->transpen(m_spritebitmap, m_spritebitmap.cliprect(),
				second * 2,
				1,
				0, 0,
				(m_s2636_0_ram[second * 0x10 + 10] * 4) - 22, m_s2636_0_ram[second * 0x10 + 12] + 1, 0);

		// Remove fingerprint
		for (int x = fx; x < fx + m_gfxdecode->gfx(expand)->width(); x++)
		{
			for (int y = fy; y < fy + m_gfxdecode->gfx(expand)->height(); y++)
			{
				if (visarea.contains(x, y))
					checksum -= m_spritebitmap.pix(y, x);
			}
		}

		// Zero bitmap
		m_gfxdecode->gfx(expand)->opaque(m_spritebitmap, m_spritebitmap.cliprect(),
				first * 2,
				1,
				0, 0,
				fx, fy);
	}

	return checksum;
}

void zac1b1120_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const rectangle &visarea = m_screen->visible_area();

	/* -------------------------------------------------------------- */
	/* There seems to be a strange setup with this board, in that it  */
	/* appears that the S2636 runs from a different clock than the    */
	/* background generator. When the program maps sprite position to */
	/* character position it only has 6 pixels of sprite for 8 pixels */
	/* of character.                                                  */
	/* -------------------------------------------------------------- */
	/* n.b. The original has several graphic glitches as well, so it  */
	/* does not seem to be a fault of the emulation!                  */
	/* -------------------------------------------------------------- */

	m_collision_background = 0; // Read from 0x1e80 bit 7

	// for collision detection checking
	copybitmap(m_bitmap, bitmap, 0, 0, 0, 0, visarea);

	for (int i = 0; i < 4; i++)
	{
		int const spriteno = (i == 0) ? 0 : (1 << i); // 0, 2, 4, 8
		int const offs = spriteno << 3; // 0, 0x10, 0x20, 0x40

		if (m_s2636_0_ram[offs + 10] < 0xf0)
		{
			int const expand = (m_s2636_0_ram[0xc0] & (1 << (i * 2))) ? 2 : 1;
			int const bx = (m_s2636_0_ram[offs + 10] * 4) - 22;
			int const by = m_s2636_0_ram[offs + 12] + 1;

			// Sprite->Background collision detection
			m_gfxdecode->gfx(expand)->transpen(bitmap, cliprect,
					spriteno,
					1,
					0, 0,
					bx, by, 0);

			for (int x = bx; x < bx + m_gfxdecode->gfx(expand)->width(); x++)
			{
				for (int y = by; y < by + m_gfxdecode->gfx(expand)->height(); y++)
				{
					if (visarea.contains(x, y))
						if (bitmap.pix(y, x) != m_bitmap.pix(y, x))
						{
							m_collision_background = 1;
							break;
						}
				}
			}

			m_gfxdecode->gfx(expand)->transpen(bitmap, cliprect,
					spriteno,
					0,
					0, 0,
					bx, by, 0);
		}
	}

	// Sprite->Sprite collision detection
	m_collision_sprite = 0;
//  if(sprite_collision(0, 1)) m_collision_sprite |= 0x20; // Not used
	if(sprite_collision(0, 2)) m_collision_sprite |= 0x10;
	if(sprite_collision(0, 4)) m_collision_sprite |= 0x08;
	if(sprite_collision(1, 2)) m_collision_sprite |= 0x04;
	if(sprite_collision(1, 4)) m_collision_sprite |= 0x02;
//  if(sprite_collision(2, 4)) m_collision_sprite |= 0x01; // Not used
}

uint32_t zac1b1120_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}


/*******************************************************************************
    Sound
*******************************************************************************/

void zac1b1120_state::sound_w(uint8_t data)
{
	// sounds are NOT the same as space invaders
	logerror("Register %x = Data %d\n", data & 0xfe, data & 0x01);

	// 08 = hit invader
	// 20 = bonus (extra base)
	// 40 = saucer
	// 84 = fire
	// 90 = die
	// c4 = hit saucer
}


/*******************************************************************************
    Address Maps
*******************************************************************************/

void zac1b1120_state::main_map(address_map &map)
{
	map(0x0000, 0x17ff).rom();
	map(0x1800, 0x1bff).ram().w(FUNC(zac1b1120_state::videoram_w)).share(m_videoram);
	map(0x1c00, 0x1cff).ram();
	map(0x1d00, 0x1dff).ram();
	map(0x1e80, 0x1e80).portr("1E80").w(FUNC(zac1b1120_state::sound_w));
	map(0x1e81, 0x1e81).portr("1E81");
	map(0x1e82, 0x1e82).portr("1E82");
	map(0x1e85, 0x1e85).portr("1E85"); // Dodgem only
	map(0x1e86, 0x1e86).portr("1E86").nopw(); // Dodgem only
	map(0x1f00, 0x1fff).rw(FUNC(zac1b1120_state::s2636_r), FUNC(zac1b1120_state::s2636_w)).share(m_s2636_0_ram);
}


/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( tinvader )
	PORT_START("1E80")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(zac1b1120_state, bg_collision_r)

	PORT_START("1E81")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPNAME( 0x02, 0x00, "Lightning Speed" )   // Velocita Laser Inv
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x02, "Fast" )
	PORT_DIPNAME( 0x1C, 0x04, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0C, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x1C, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x20, "1500" )
	PORT_DIPNAME( 0x40, 0x00, "Extended Play" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )

	PORT_START("1E82")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("1E85")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("1E86")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

// Almost identical, no number of bases selection
static INPUT_PORTS_START( sinvader )
	PORT_INCLUDE( tinvader )

	PORT_MODIFY("1E80")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("1E81")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED  )
INPUT_PORTS_END

static INPUT_PORTS_START( dodgem )
	PORT_START("1E80")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(zac1b1120_state, bg_collision_r)

	PORT_START("1E81")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPNAME( 0x02, 0x00, "Time" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x1C, 0x04, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0C, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x1C, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x20, 0x00, "Show High Scores" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("1E82")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("1E85")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("1E86")
	PORT_DIPNAME( 0x01, 0x01, "Collision Detection (Cheat)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
INPUT_PORTS_END


/*******************************************************************************
    GFX Layouts
*******************************************************************************/

static const gfx_layout s2636_character =
{
	8,10,
	16,
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8), STEP2(8*8,8) },
	8*8
};

static GFXDECODE_START( gfx_tinvader )
	GFXDECODE_SCALE( "gfx1", 0, gfx_8x8x1, 0, 2, 3, 1 )
	GFXDECODE_SCALE( nullptr, 0x1f00, s2636_character, 0, 2, 4, 1 ) // dynamic
	GFXDECODE_SCALE( nullptr, 0x1f00, s2636_character, 0, 2, 8, 2 ) // dynamic
GFXDECODE_END


/*******************************************************************************
    Machine Configs
*******************************************************************************/

void zac1b1120_state::tinvader(machine_config &config)
{
	// basic machine hardware
	s2650_device &maincpu(S2650(config, m_maincpu, 15.625_MHz_XTAL / 16));
	maincpu.set_addrmap(AS_PROGRAM, &zac1b1120_state::main_map);
	maincpu.sense_handler().set(m_screen, FUNC(screen_device::vblank)).invert();

	// video hardware (timings generated by an (overclocked) Signetics 2621 PAL Universal Sync Generator; screen refresh measured at 55Hz)
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE); // for collision detection
	m_screen->set_raw(15.625_MHz_XTAL, 227 * 4, 0, 180 * 4, 312, 0, 256);
	m_screen->set_screen_update(FUNC(zac1b1120_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tinvader);
	PALETTE(config, m_palette, FUNC(zac1b1120_state::palette), 4);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	S2636(config, m_s2636, 0).add_route(ALL_OUTPUTS, "mono", 0.25);
}

void zac1b1120_state::dodgem(machine_config &config)
{
	tinvader(config);

	// XTAL value is different
	m_maincpu->set_clock(14.318181_MHz_XTAL / 16);
	m_screen->set_raw(14.318181_MHz_XTAL, 227 * 4, 0, 180 * 4, 312, 0, 256); // TBD: verify refresh rate

	// TODO: sound board (different from The Invaders)
}


/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( sia2650 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "42_1.bin",   0x0000, 0x0800, CRC(a85550a9) SHA1(3f1e6b8e61894ff997e31b9c5ff819aa4678394e) )
	ROM_LOAD( "44_2.bin",   0x0800, 0x0800, CRC(48d5a3ed) SHA1(7f6421ba8225d49c1038595517f31b076d566586) )
	ROM_LOAD( "46_3.bin",   0x1000, 0x0800, CRC(d766e784) SHA1(88c113855c4cde8cefbe862d3e5abf80bd17aaa0) )

	ROM_REGION( 0x400, "gfx1", 0 )
	ROM_LOAD( "06_inv.bin", 0x0000, 0x0400, CRC(7bfed23e) SHA1(f754f0a4d6c8f9812bf333c30fa433b63d49a750) )
ROM_END

ROM_START( tinv2650 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "42_1.bin",   0x0000, 0x0800, CRC(a85550a9) SHA1(3f1e6b8e61894ff997e31b9c5ff819aa4678394e) )
	ROM_LOAD( "44_2t.bin",  0x0800, 0x0800, CRC(083c8621) SHA1(d9b33d532903b0e6dee2357b9e3b329856505a73) )
	ROM_LOAD( "46_3t.bin",  0x1000, 0x0800, CRC(12c0934f) SHA1(9fd67d425c533b0e09b201301020639eb9e452f7) )

	ROM_REGION( 0x400, "gfx1", 0 )
	ROM_LOAD( "06_inv.bin", 0x0000, 0x0400, CRC(7bfed23e) SHA1(f754f0a4d6c8f9812bf333c30fa433b63d49a750) )

	ROM_REGION( 0x0200, "proms", 0 ) // SYNC
	ROM_LOAD( "82s130.ic31", 0x0000, 0x0200, CRC(c9f62cb3) SHA1(d972602e632cca5ebd2074b2e8d493ab6d1628eb) ) // N82S130 BPROM
ROM_END

ROM_START( dodgem )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "rom1.bin",     0x0000, 0x0400, CRC(a327b57d) SHA1(a9cb17e60ab7b4ed9d5a9e7f8451a8f29bb7d00d) )
	ROM_LOAD( "rom2.bin",     0x0400, 0x0400, CRC(2a06ec74) SHA1(34fd3cbb1ddadb81abde54046bf245e2285bb740) )
	ROM_LOAD( "rom3.bin",     0x0800, 0x0400, CRC(e9ed656d) SHA1(a36ec04fd7cdf26aa7fa36e18cd44b159ed53906) )
	ROM_LOAD( "rom4.bin",     0x0c00, 0x0400, CRC(ecbfd906) SHA1(89f921a3d69b30977cd09a62dff4be02e6604550) )
	ROM_LOAD( "rom5.bin",     0x1000, 0x0400, CRC(bdae09fe) SHA1(76517d432d9bff5a2eea438f6edc3e04b889448a) )
	ROM_LOAD( "rom6.bin",     0x1400, 0x0400, CRC(e131eacf) SHA1(6f5244a9d27b3c5696ed83843e46079d579f7b39) )

	ROM_REGION( 0x400, "gfx1", 0 )
	ROM_LOAD( "93451.bin",    0x0000, 0x0400, CRC(004b26d2) SHA1(0b825510e7a8afa9db589f87ec93467ab8c73f93) )

	ROM_REGION( 0x0200, "proms", 0 ) // SYNC
	ROM_LOAD( "74s571",       0x0000, 0x0200, CRC(cc0b407e) SHA1(e675e3d7ff82e1cff9001e367620208bffa8b42f) )
ROM_END

} // anonymous namespace


/*******************************************************************************
    Game Drivers
*******************************************************************************/

GAMEL( 1979?, tinv2650, 0,        tinvader, tinvader, zac1b1120_state, empty_init, ROT270, "Zaccaria / Zelco", "The Invaders",                                   MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_tinv2650 )
GAME(  1979?, sia2650,  tinv2650, tinvader, sinvader, zac1b1120_state, empty_init, ROT270, "bootleg (Sidam)",  "Super Invader Attack (bootleg of The Invaders)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 1980?
GAMEL( 1979,  dodgem,   0,        dodgem,   dodgem,   zac1b1120_state, empty_init, ROT0,   "Zaccaria",         "Dodgem",                                         MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_dodgem )
