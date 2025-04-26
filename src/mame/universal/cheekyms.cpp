// license:BSD-3-Clause
// copyright-holders: Lee Taylor, Chris Moore

/*************************************************************************

    Universal Cheeky Mouse Driver
    (c)Lee Taylor May/June 1998

**************************************************************************/

#include "emu.h"

#include "cheekyms_a.h"

#include "cpu/z80/z80.h"
#include "sound/dac.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


namespace {

class cheekyms_state : public driver_device
{
public:
	cheekyms_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_sound_board(*this, "soundboard")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_videoram(*this, "videoram")
		, m_spriteram(*this, "spriteram")
		, m_port_80(*this, "port_80")
	{
	}

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

	void cheekyms(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cheekyms_audio_device> m_sound_board;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	// memory pointers
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_port_80;

	// video-related
	tilemap_t *m_cm_tilemap = nullptr;
	std::unique_ptr<bitmap_ind16> m_bitmap_buffer;

	uint8_t m_irq_mask = 0U;

	void port_40_w(uint8_t data);
	void port_80_w(uint8_t data);

	void vblank_int_w(int state);

	TILE_GET_INFO_MEMBER(get_tile_info);

	void palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx, int flip);

	void io_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};


// bit 3 and 7 of the char color PROMs are used for something -- not currently emulated - thus GAME_IMPERFECT_GRAPHICS

void cheekyms_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 0x20; j++)
		{
			int bit;

			// red component
			bit = BIT(color_prom[0x20 * (i / 2) + j], (4 * (i & 1)) + 0);
			int const r = 0xff * bit;

			// green component
			bit = BIT(color_prom[0x20 * (i / 2) + j], (4 * (i & 1)) + 1);
			int const g = 0xff * bit;

			// blue component
			bit = BIT(color_prom[0x20 * (i / 2) + j], (4 * (i & 1)) + 2);
			int const b = 0xff * bit;

			palette.set_pen_color((i * 0x20) + j, rgb_t(r, g, b));
		}
	}
}

void cheekyms_state::port_40_w(uint8_t data)
{
	m_sound_board->music_w(BIT(data, 7));
	m_sound_board->cheese_w(BIT(data, 6));
	m_sound_board->hammer_w(BIT(data, 5));
	m_sound_board->mouse_dies_w(BIT(data, 4));
	m_sound_board->pest_dies_w(BIT(data, 3));
	m_sound_board->mouse_w(BIT(data, 2));
	m_sound_board->pest_w(BIT(data, 1));
}


void cheekyms_state::port_80_w(uint8_t data)
{
	m_sound_board->coin_extra_w(BIT(data, 1));
	m_sound_board->mute_w(BIT(data, 0));

	// d3-d5 - man scroll amount
	// d6 - palette select (selects either 0 = PROM M9, 1 = PROM M8)
	// d7 - screen flip
	*m_port_80 = data;

	// d2 - interrupt enable
	m_irq_mask = BIT(data, 2);
	if (!m_irq_mask)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}



TILE_GET_INFO_MEMBER(cheekyms_state::get_tile_info)
{
	int color;

	int const x = tile_index & 0x1f;
	int const y = tile_index >> 5;
	int const code = m_videoram[tile_index];
	int const palette = (*m_port_80 >> 2) & 0x10;

	if (x >= 0x1e)
	{
		if (y < 0x0c)
			color = 0x15;
		else if (y < 0x14)
			color = 0x16;
		else
			color = 0x14;
	}
	else
	{
		if ((y == 0x04) || (y == 0x1b))
			color = palette | 0x0c;
		else
			color = palette | (x >> 1);
	}

	tileinfo.set(0, code, color, 0);
}

void cheekyms_state::video_start()
{
	int const width = m_screen->width();
	int const height = m_screen->height();
	m_bitmap_buffer = std::make_unique<bitmap_ind16>(width, height);

	m_cm_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cheekyms_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_cm_tilemap->set_transparent_pen(0);
}


void cheekyms_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx, int flip)
{
	for (offs_t offs = 0; offs < 0x20; offs += 4)
	{
		if ((m_spriteram[offs + 3] & 0x08) == 0x00) continue;

		int const x = 256 - m_spriteram[offs + 2];
		int const y = m_spriteram[offs + 1];
		int code = (~m_spriteram[offs + 0] & 0x0f) << 1;
		int const color = (~m_spriteram[offs + 3] & 0x07);

		if (m_spriteram[offs + 0] & 0x80)
		{
			if (!flip)
				code++;

			gfx->transpen(bitmap, cliprect, code, color, 0, 0, x, y, 0);
		}
		else
		{
			if (m_spriteram[offs + 0] & 0x02)
			{
				gfx->transpen(bitmap, cliprect, code | 0x20, color, 0, 0, x, y, 0);
				gfx->transpen(bitmap, cliprect, code | 0x21, color, 0, 0, 0x10 + x, y, 0);
			}
			else
			{
				gfx->transpen(bitmap, cliprect, code | 0x20, color, 0, 0, x, y, 0);
				gfx->transpen(bitmap, cliprect, code | 0x21, color, 0, 0, x, 0x10 + y, 0);
			}
		}
	}
}


uint32_t cheekyms_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int const scrolly = ((*m_port_80 >> 3) & 0x07);
	int const flip = *m_port_80 & 0x80;

	machine().tilemap().mark_all_dirty();
	machine().tilemap().set_flip_all(flip ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);

	bitmap.fill(0, cliprect);
	m_bitmap_buffer->fill(0, cliprect);

	// sprites go under the playfield
	draw_sprites(bitmap, cliprect, m_gfxdecode->gfx(1), flip);

	// draw the tilemap to a temp bitmap
	m_cm_tilemap->draw(screen, *m_bitmap_buffer, cliprect, 0, 0);

	// draw the tilemap to the final bitmap applying the scroll to the man character
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			int in_man_area;

			if (flip)
			{
				in_man_area = (x >= (32 - 12 - 1) * 8 && x < (32 - 8) * 8 && y > 5 * 8 && y < 27 * 8);
			}
			else
			{
				in_man_area = (x >= 8 * 8 && x < 12 * 8 && y > 5 * 8 && y < 27 * 8);
			}

			if (in_man_area)
			{
				if ((y + scrolly) < 27 * 8 && m_bitmap_buffer->pix(y + scrolly, x) != 0)
					bitmap.pix(y, x) = m_bitmap_buffer->pix(y + scrolly, x);
			}
			else
			{
				if(m_bitmap_buffer->pix(y, x) != 0)
					bitmap.pix(y, x) = m_bitmap_buffer->pix(y, x);
			}
		}
	}


	return 0;
}


INPUT_CHANGED_MEMBER(cheekyms_state::coin_inserted)
{
	// this starts a 556 one-shot timer (and triggers a sound effect)
	if (newval)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


void cheekyms_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x3000, 0x33ff).ram();
	map(0x3800, 0x3bff).ram().share(m_videoram);
}

void cheekyms_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("DSW");
	map(0x01, 0x01).portr("INPUTS");
	map(0x20, 0x3f).writeonly().share(m_spriteram);
	map(0x40, 0x40).w(FUNC(cheekyms_state::port_40_w));
	map(0x80, 0x80).w(FUNC(cheekyms_state::port_80_w)).share(m_port_80);
}


static INPUT_PORTS_START( cheekyms )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) ) // duplicate
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x40, "3000" )
	PORT_DIPSETTING(    0x80, "4500" )
	PORT_DIPSETTING(    0xc0, "6000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(cheekyms_state::coin_inserted), 0)
INPUT_PORTS_END



static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(1,2), 0 },
	{ STEP16(0,1) },
	{ STEP16(0,16) },
	32*8
};



static GFXDECODE_START( gfx_cheekyms )
	GFXDECODE_ENTRY( "chars",   0, gfx_8x8x2_planar, 0x00, 0x20 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,     0x80, 0x10 )
GFXDECODE_END


void cheekyms_state::machine_start()
{
	save_item(NAME(m_irq_mask));
}

void cheekyms_state::vblank_int_w(int state)
{
	if (m_irq_mask)
		m_maincpu->set_input_line(0, ASSERT_LINE);
}


void cheekyms_state::cheekyms(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 5_MHz_XTAL / 2);  // 2.5 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &cheekyms_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &cheekyms_state::io_map);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(10.816_MHz_XTAL / 2, 352, 0, 256, 262, 32, 224);
	m_screen->set_screen_update(FUNC(cheekyms_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(cheekyms_state::vblank_int_w));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cheekyms);
	PALETTE(config, m_palette, FUNC(cheekyms_state::palette), 0xc0);

	// audio hardware
	CHEEKY_MOUSE_AUDIO(config, m_sound_board, 0);
}




/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( cheekyms )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cm03.c5", 0x0000, 0x0800, CRC(1ad0cb40) SHA1(2a751395ac19a3218c22cfd3597f9a17b8e31527) )
	ROM_LOAD( "cm04.c6", 0x0800, 0x0800, CRC(2238f607) SHA1(35df9eb49f6e3c6351fae220d773442cf8536f90) )
	ROM_LOAD( "cm05.c7", 0x1000, 0x0800, CRC(4169eba8) SHA1(52a059f29c724d087483c7cd733f75d7b8a5b103) )
	ROM_LOAD( "cm06.c8", 0x1800, 0x0800, CRC(7031660c) SHA1(1370702e30897e45ee172609c1d983f8a4fdf157) )

	ROM_REGION( 0x1000, "chars", 0 )
	ROM_LOAD( "cm02.c2", 0x0000, 0x0800, CRC(885887c3) SHA1(62ce8e39d27c0cfea9ebd51757ad31b0baf6b3cd) )
	ROM_LOAD( "cm01.c1", 0x0800, 0x0800, CRC(26f73bd7) SHA1(fa4db5df5be1a5f4531cba86a83f89b7eb7fa3ec) )

	ROM_REGION( 0x1000, "sprites", 0 )
	ROM_LOAD( "cm07.n5", 0x0000, 0x0800, CRC(2738c88d) SHA1(3ccd6c1d49bfe2c1b141854ec705e692252e8af8) )
	ROM_LOAD( "cm08.n6", 0x0800, 0x0800, CRC(b3fbd4ac) SHA1(9f45cc6d9e0bf580149e18de5c3e37d4de347b92) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "cm.m9", 0x0000, 0x0020, CRC(db9c59a5) SHA1(357ed5ac8e954a4c8b4d78d36e57bf2de36c1d57) ) // Character colors /
	ROM_LOAD( "cm.m8", 0x0020, 0x0020, CRC(2386bc68) SHA1(6676082860cd8678a71339a352d2c6286e78ba44) ) // Character colors \ Selected by Bit 6 of Port 0x80
	ROM_LOAD( "cm.p3", 0x0040, 0x0020, CRC(6ac41516) SHA1(05bf40790a0de1e859362df892f7f158c183e247) ) // Sprite colors
ROM_END

} // anonymous namespace


GAME( 1980, cheekyms, 0, cheekyms, cheekyms, cheekyms_state, empty_init, ROT270, "Universal", "Cheeky Mouse", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
