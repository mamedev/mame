// license:BSD-3-Clause
// copyright-holders: Phil Stroffolino

/****************************************************************************

    Irem M57 hardware

*****************************************************************************

    Tropical Angel

    driver by Phil Stroffolino

    IREM M57 board stack with a M52-SOUND-E sound PCB.

    M57-A-A:
     TA-A-xx ROMs and PROMs
     NEC D780C (Z80) CPU
     NANAO KNA6032601 custom chip
     NANAO KNA6032701 custom chip
     8-way dipswitch (x2)
     M58725P RAM (x3)
     CN1 - Ribbon cable connector
     CN2 - Ribbon cable connector
     Ribbon cable connector to sound PCB

    M57-B-A:
     TA-B-xx ROMs and PROMs
     18.432 MHz OSC
     CN1 - Ribbon cable connector
     CN2 - Ribbon cable connector

    M52:
     HD6803 CPU
     AY-3-9810 (x2) sound chips
     MSM5205 OKI sound chip (and an unpopulated socket for a second MSM5202)
     3.579545 MHz OSC
     2764 Program ROM labeled "TA S-1A-"
     Ribbon cable connector to M57-A-A PCB

    New Tropical Angel:
     ROMs were found on an official IREM board with genuine IREM Tropical Angel
     license seal and genuine IREM serial number sticker.
     The "new" ROMs have hand written labels, while those that match the current
     Tropical Angel set look to be factory labeled chips.

*****************************************************************************

    Locations based on m58.cpp driver

****************************************************************************/

#include "emu.h"

#include "irem.h"
#include "iremipt.h"

#include "cpu/z80/z80.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


namespace {

class m57_state : public driver_device
{
public:
	m57_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_scrollram(*this, "scrollram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_dsw2(*this, "DSW2")
	{ }

	void m57(machine_config &config);

protected:
	virtual void video_start() override;

private:
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_scrollram;
	required_shared_ptr<uint8_t> m_spriteram;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_ioport m_dsw2;

	tilemap_t * m_bg_tilemap = nullptr;
	uint8_t m_flipscreen = 0;

	void videoram_w(offs_t offset, uint8_t data);
	void flipscreen_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map);
};


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Tropical Angel has two 256x4 character palette PROMs, one 32x8 sprite
  palette PROM, and one 256x4 sprite color lookup table PROM.

  I don't know for sure how the palette PROMs are connected to the RGB
  output, but it's probably something like this; note that RED and BLUE
  are swapped wrt the usual configuration.

  bit 7 -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
  bit 0 -- 1  kohm resistor  -- BLUE

***************************************************************************/

void m57_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	// character palette
	for (int i = 0; i < 256; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = 0;
		bit1 = BIT(color_prom[256], 2);
		bit2 = BIT(color_prom[256], 3);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// green component
		bit0 = BIT(color_prom[0], 3);
		bit1 = BIT(color_prom[256], 0);
		bit2 = BIT(color_prom[256], 1);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// blue component
		bit0 = BIT(color_prom[0], 0);
		bit1 = BIT(color_prom[0], 1);
		bit2 = BIT(color_prom[0], 2);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_indirect_color(i, rgb_t(r, g, b));
		palette.set_pen_indirect(i, i);
		color_prom++;
	}

	color_prom += 256;
	// color_prom now points to the beginning of the sprite palette

	// sprite palette
	for (int i = 0; i < 16; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = 0;
		bit1 = BIT(*color_prom, 6);
		bit2 = BIT(*color_prom, 7);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// green component
		bit0 = BIT(*color_prom, 3);
		bit1 = BIT(*color_prom, 4);
		bit2 = BIT(*color_prom, 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// blue component
		bit0 = BIT(*color_prom, 0);
		bit1 = BIT(*color_prom, 1);
		bit2 = BIT(*color_prom, 2);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_indirect_color(i + 256, rgb_t(r, g, b));
		color_prom++;
	}

	color_prom += 16;
	// color_prom now points to the beginning of the sprite lookup table

	// sprite lookup table
	for (int i = 0; i < 32 * 8; i++)
	{
		palette.set_pen_indirect(i + 32 * 8, 256 + (~*color_prom & 0x0f));
		color_prom++;
	}
}


/*************************************
 *
 *  Tilemap info callback
 *
 *************************************/

TILE_GET_INFO_MEMBER(m57_state::get_tile_info)
{
	uint8_t const attr = m_videoram[tile_index * 2 + 0];
	uint16_t const code = m_videoram[tile_index * 2 + 1] | ((attr & 0xc0) << 2);

	tileinfo.set(0, code, attr & 0x0f, TILE_FLIPXY(attr >> 4));
}


/*************************************
 *
 *  Video RAM access
 *
 *************************************/

void m57_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}


/*************************************
 *
 *  Video startup
 *
 *************************************/

void m57_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(m57_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_scroll_rows(256);

	save_item(NAME(m_flipscreen));
}


/*************************************
 *
 *  Outputs
 *
 *************************************/

void m57_state::flipscreen_w(uint8_t data)
{
	// screen flip is handled both by software and hardware
	m_flipscreen = (data & 0x01) ^ (~m_dsw2->read() & 0x01);
	m_bg_tilemap->set_flip(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	machine().bookkeeping().coin_counter_w(0, data & 0x02);
	machine().bookkeeping().coin_counter_w(1, data & 0x20);
}


/*************************************
 *
 *  Background rendering
 *
 *************************************/

void m57_state::draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// from 64 to 127: not wrapped
	for (int y = 64; y < 128; y++)
		m_bg_tilemap->set_scrollx(y, m_scrollram[0x40]);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// from 128 to 255: wrapped
	for (int y = 128; y <= cliprect.max_y; y++)
	{
		int16_t const scrolly = m_scrollram[y] + (m_scrollram[y + 0x100] << 8);

		if (scrolly >= 0)
		{
			for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
			{
				if ((x + scrolly) <= cliprect.max_x)
					bitmap.pix(y, x) = bitmap.pix(y, x + scrolly);
				else
					bitmap.pix(y, x) = bitmap.pix(y, cliprect.max_x);
			}
		}
		else
		{
			for (int x = cliprect.max_x; x >= cliprect.min_x; x--)
			{
				if ((x + scrolly) >= cliprect.min_x)
					bitmap.pix(y, x) = bitmap.pix(y, x + scrolly);
				else
					bitmap.pix(y, x) = bitmap.pix(y, cliprect.min_x);
			}
		}
	}
}

/*************************************
 *
 *  Sprite rendering
 *
 *************************************/

void m57_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		uint8_t const attributes = m_spriteram[offs + 1];
		int sx = m_spriteram[offs + 3];
		int sy = ((224 - m_spriteram[offs + 0] - 32) & 0xff) + 32;
		int const code = m_spriteram[offs + 2];
		int const color = attributes & 0x1f;
		int flipy = attributes & 0x80;
		int flipx = attributes & 0x40;

		int const tile_number = code & 0x3f;

		int bank = 0;
		if (code & 0x80) bank += 1;
		if (attributes & 0x20) bank += 2;

		if (m_flipscreen)
		{
			sx = 240 - sx;
			sy = 224 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1 + bank)->transmask(bitmap, cliprect,
			tile_number,
			color,
			flipx, flipy,
			sx, sy,
			m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 256 + 15));
	}
}



/*************************************
 *
 *  Video update
 *
 *************************************/

uint32_t m57_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_background(screen, bitmap, cliprect);
	draw_sprites(bitmap, cliprect);
	return 0;
}


/*************************************
 *
 *  Memory maps
 *
 *************************************/

void m57_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram().w(FUNC(m57_state::videoram_w)).share(m_videoram);
	map(0x9000, 0x91ff).ram().share(m_scrollram);
	map(0xc820, 0xc8ff).writeonly().share(m_spriteram);
	map(0xd000, 0xd000).w("irem_audio", FUNC(irem_audio_device::cmd_w));
	map(0xd001, 0xd001).w(FUNC(m57_state::flipscreen_w)); // + coin counters
	map(0xd000, 0xd000).portr("IN0");
	map(0xd001, 0xd001).portr("IN1");
	map(0xd002, 0xd002).portr("IN2");
	map(0xd003, 0xd003).portr("DSW1");
	map(0xd004, 0xd004).portr("DSW2");
	map(0xe000, 0xe7ff).ram();
}



/*************************************
 *
 *  Generic port definitions
 *
 *************************************/

// Same as m52, m58 and m62 (IREM Z80 hardware)
static INPUT_PORTS_START( m57 )
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

/*************************************
 *
 *  Games port definitions
 *
 *************************************/

static INPUT_PORTS_START( troangel )
	PORT_INCLUDE(m57)

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
	/* TODO: the following enables an analog accelerator input read from 0xd003
	   however that is the DSW1 input so it must be multiplexed some way */
	PORT_DIPNAME( 0x08, 0x08, "Analog Accelerator" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	// In stop mode, press 2 to stop and 1 to restart
	PORT_DIPNAME( 0x10, 0x10, "Stop Mode (Cheat)") PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:6" )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Time" ) PORT_DIPLOCATION("SW1:1,2") // table at 0x6110 - 4 * 8 bytes (B1 B2 bonus A1 A2 bonus M1 M2)
	PORT_DIPSETTING(    0x03, "B:180/A:160/M:140/BG:120" )
	PORT_DIPSETTING(    0x02, "B:160/A:140/M:120/BG:100" )
	PORT_DIPSETTING(    0x01, "B:140/A:120/M:100/BG:80" )
	PORT_DIPSETTING(    0x00, "B:120/A:100/M:100/BG:80" )
	PORT_DIPNAME( 0x04, 0x04, "Crash Loss Time" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x08, 0x08, "Background Sound" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "Boat Motor" )
	PORT_DIPSETTING(    0x00, "Music" )
	IREM_Z80_COINAGE_TYPE_2_LOC(SW1)
INPUT_PORTS_END



/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout spritelayout =
{
	16,32,
	64,
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(0,1), STEP8(16*8,1) },
	{ STEP16(0,8), STEP16(256*64,8) },
	32*8
};

static GFXDECODE_START( gfx_m57 )
	GFXDECODE_ENTRY( "tiles",   0x0000, gfx_8x8x3_planar, 0, 32 )
	GFXDECODE_ENTRY( "sprites", 0x0000, spritelayout, 32*8, 32 )
	GFXDECODE_ENTRY( "sprites", 0x1000, spritelayout, 32*8, 32 )
	GFXDECODE_ENTRY( "sprites", 0x2000, spritelayout, 32*8, 32 )
	GFXDECODE_ENTRY( "sprites", 0x3000, spritelayout, 32*8, 32 )
GFXDECODE_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void m57_state::m57(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 18.432_MHz_XTAL / 6);  // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &m57_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(m57_state::irq0_line_hold));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(57);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(1790)); // accurate frequency, measured on a Moon Patrol board, is 56.75Hz.
				/* the Lode Runner manual (similar but different hardware)
				   talks about 55Hz and 1790ms vblank duration. */
	screen.set_size(32*8, 32*8);
	screen.set_visarea(1*8, 31*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(m57_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_m57);
	PALETTE(config, m_palette, FUNC(m57_state::palette), 32*8+32*8, 256+16);

	// sound hardware
	IREM_M52_SOUNDC_AUDIO(config, "irem_audio", 0);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( troangel )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ta-a-3k", 0x0000, 0x2000, CRC(f21f8196) SHA1(7cbf74b77a559ee70312b799e707394d9b849f5b) )
	ROM_LOAD( "ta-a-3m", 0x2000, 0x2000, CRC(58801e55) SHA1(91bdda778f2c4486001bc4ad26d6f21ba275ae08) )
	ROM_LOAD( "ta-a-3n", 0x4000, 0x2000, CRC(de3dea44) SHA1(1290755ffc04dc3b3667e063118669a0eab6fb79) )
	ROM_LOAD( "ta-a-3q", 0x6000, 0x2000, CRC(fff0fc2a) SHA1(82f3f5a8817e956192323eb555daa85b7766676d) )

	ROM_REGION(  0x8000 , "irem_audio:iremsound", 0 )
	ROM_LOAD( "ta-s-1a", 0x6000, 0x2000, CRC(15a83210) SHA1(8ada510db689ffa372b2f4dc4bd1b1c69a0c5307) )

	ROM_REGION( 0x06000, "tiles", 0 )
	ROM_LOAD( "ta-a-3e", 0x00000, 0x2000, CRC(e49f7ad8) SHA1(915de1084fd3c5fc81dd8c80107c28cc57b33226) )
	ROM_LOAD( "ta-a-3d", 0x02000, 0x2000, CRC(06eef241) SHA1(4f327a54169046d8d84b5f5cf5d9f45e1df4dae6) )
	ROM_LOAD( "ta-a-3c", 0x04000, 0x2000, CRC(7ff5482f) SHA1(fe8c181fed113007d69d11e8aa467e86a6357ffb) )

	ROM_REGION( 0x0c000, "sprites", 0 )
	ROM_LOAD( "ta-b-5j", 0x00000, 0x2000, CRC(86895c0c) SHA1(b42b041e3e20dadd8411805d492133d371426ebf) )
	ROM_LOAD( "ta-b-5h", 0x02000, 0x2000, CRC(f8cff29d) SHA1(dabf3bbf50f73a381056131c2239c84dd966b63e) )
	ROM_LOAD( "ta-b-5e", 0x04000, 0x2000, CRC(8b21ee9a) SHA1(1272722211d22d5b153e9415cc189a5aa9028543) )
	ROM_LOAD( "ta-b-5d", 0x06000, 0x2000, CRC(cd473d47) SHA1(854cb532bd62851a206da2affd66a1257b7085b6) )
	ROM_LOAD( "ta-b-5c", 0x08000, 0x2000, CRC(c19134c9) SHA1(028660e66fd033473c468b694e870c633ca05ec6) )
	ROM_LOAD( "ta-b-5a", 0x0a000, 0x2000, CRC(0012792a) SHA1(b4380f5fbe5e9ce9b44f87ce48a8b402bab58b52) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "ta-a-5a", 0x0000, 0x0100, CRC(01de1167) SHA1(b9070f8c70eb362fc4d6a0a92235ce0a5b2ab858) ) // chars palette low 4 bits
	ROM_LOAD( "ta-a-5b", 0x0100, 0x0100, CRC(efd11d4b) SHA1(7c7c356063ab35e4ffb8d65cd20c27c2a4b36537) ) // chars palette high 4 bits
	ROM_LOAD( "ta-b-1b", 0x0200, 0x0020, CRC(f94911ea) SHA1(ad61a323476a97156a255a72048a28477b421284) ) // sprites palette
	ROM_LOAD( "ta-b-3d", 0x0220, 0x0100, CRC(ed3e2aa4) SHA1(cfdfc151803080d1ecdd04af1bfea3dbdce8dca0) ) // sprites lookup table
ROM_END

ROM_START( newtangl ) // Official "upgrade" or hack?
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3k", 0x0000, 0x2000, CRC(3c6299a8) SHA1(a21a8452b75ce6174076878128d4f20b39b6d69d) )
	ROM_LOAD( "3m", 0x2000, 0x2000, CRC(8d09056c) SHA1(4d2585103cc6e6c04015501d3c9e1578a8f9c0f5) )
	ROM_LOAD( "3n", 0x4000, 0x2000, CRC(17b5a775) SHA1(d85c3371080bea82f19ac96fa0f1b332e1c86e27) )
	ROM_LOAD( "3q", 0x6000, 0x2000, CRC(2e5fa773) SHA1(9a34fa43bde021fc7b00d8c3762c248e7b96dbf1) )

	ROM_REGION(  0x8000 , "irem_audio:iremsound", 0 )
	ROM_LOAD( "ta-s-1a-", 0x6000, 0x2000, CRC(ea8a05cb) SHA1(5683e4dca93066ee788287ab73a766fa303ebe84) )

	ROM_REGION( 0x06000, "tiles", 0 )
	ROM_LOAD( "ta-a-3e", 0x00000, 0x2000, CRC(e49f7ad8) SHA1(915de1084fd3c5fc81dd8c80107c28cc57b33226) )
	ROM_LOAD( "ta-a-3d", 0x02000, 0x2000, CRC(06eef241) SHA1(4f327a54169046d8d84b5f5cf5d9f45e1df4dae6) )
	ROM_LOAD( "ta-a-3c", 0x04000, 0x2000, CRC(7ff5482f) SHA1(fe8c181fed113007d69d11e8aa467e86a6357ffb) )

	ROM_REGION( 0x0c000, "sprites", 0 )
	ROM_LOAD( "5j",      0x00000, 0x2000, CRC(89409130) SHA1(3f37f820b1b86166cde7c039d657ebd036d490dd) )
	ROM_LOAD( "ta-b-5h", 0x02000, 0x2000, CRC(f8cff29d) SHA1(dabf3bbf50f73a381056131c2239c84dd966b63e) )
	ROM_LOAD( "5e",      0x04000, 0x2000, CRC(5460a467) SHA1(505c1d9e69c39a74369da17f354b90486ee6afcd) )
	ROM_LOAD( "ta-b-5d", 0x06000, 0x2000, CRC(cd473d47) SHA1(854cb532bd62851a206da2affd66a1257b7085b6) )
	ROM_LOAD( "5c",      0x08000, 0x2000, CRC(4a20637a) SHA1(74099cb7f1727c2de2f066497097f1a9eeec0cea) )
	ROM_LOAD( "ta-b-5a", 0x0a000, 0x2000, CRC(0012792a) SHA1(b4380f5fbe5e9ce9b44f87ce48a8b402bab58b52) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "ta-a-5a", 0x0000, 0x0100, CRC(01de1167) SHA1(b9070f8c70eb362fc4d6a0a92235ce0a5b2ab858) ) // chars palette low 4 bits
	ROM_LOAD( "ta-a-5b", 0x0100, 0x0100, CRC(efd11d4b) SHA1(7c7c356063ab35e4ffb8d65cd20c27c2a4b36537) ) // chars palette high 4 bits
	ROM_LOAD( "ta-b-1b", 0x0200, 0x0020, CRC(f94911ea) SHA1(ad61a323476a97156a255a72048a28477b421284) ) // sprites palette
	ROM_LOAD( "ta-b-3d", 0x0220, 0x0100, CRC(ed3e2aa4) SHA1(cfdfc151803080d1ecdd04af1bfea3dbdce8dca0) ) // sprites lookup table
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1983, troangel, 0,        m57, troangel, m57_state, empty_init, ROT0, "Irem", "Tropical Angel",     MACHINE_SUPPORTS_SAVE )
GAME( 1983, newtangl, troangel, m57, troangel, m57_state, empty_init, ROT0, "Irem", "New Tropical Angel", MACHINE_SUPPORTS_SAVE )
