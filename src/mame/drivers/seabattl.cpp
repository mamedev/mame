// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek, hap, Philip Bennett, David Haywood
/*

Sea Battle by Zaccaria

driver by Mariusz Wojcieszek, hap, Phil Bennett and David Haywood

TODO:
- correct colors (note: the flyer screenshots look faked; not a good reference)
- should it have a horizon/sky?
- video timing
- video offsets
- discrete sound


2650 + 2636

sea b b_1 *.prg are 2650 progamm

sea b blu.prg is obj blue data
sea b red.prg is obj red data
sea b green.prg is obj green data

sea b wawe.prg is sea wave data

sea b screen.prg is tile data


the sound board should be fully discrete.

*/

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "machine/s2636.h"
#include "video/dm9368.h"
#include "seabattl.lh"


class seabattl_state : public driver_device
{
public:
	seabattl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_objram(*this, "objram"),
		m_digit0(*this, "sc_thousand"),
		m_digit1(*this, "sc_hundred"),
		m_digit2(*this, "sc_half"),
		m_digit3(*this, "sc_unity"),
		m_digit4(*this, "tm_half"),
		m_digit5(*this, "tm_unity"),
		m_s2636(*this, "s2636"),
		m_waveenable(false),
		m_collision(0),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{
	}

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_objram;
	required_device<dm9368_device> m_digit0;
	required_device<dm9368_device> m_digit1;
	required_device<dm9368_device> m_digit2;
	required_device<dm9368_device> m_digit3;
	required_device<dm9368_device> m_digit4;
	required_device<dm9368_device> m_digit5;
	required_device<s2636_device> m_s2636;

	tilemap_t *m_bg_tilemap;
	bitmap_ind16 m_collision_bg;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	DECLARE_WRITE8_MEMBER(seabattl_videoram_w);
	DECLARE_WRITE8_MEMBER(seabattl_colorram_w);
	DECLARE_WRITE8_MEMBER(seabattl_control_w);
	DECLARE_READ8_MEMBER(seabattl_collision_r);
	DECLARE_WRITE8_MEMBER(seabattl_collision_clear_w);
	DECLARE_READ8_MEMBER(seabattl_collision_clear_r);
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_WRITE8_MEMBER(sound2_w);
	DECLARE_WRITE8_MEMBER(time_display_w);
	DECLARE_WRITE8_MEMBER(score_display_w);
	DECLARE_WRITE8_MEMBER(score2_display_w);

	INTERRUPT_GEN_MEMBER(seabattl_interrupt);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(seabattl);
	UINT32 screen_update_seabattl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	bool m_waveenable;
	UINT8 m_collision;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};


/***************************************************************************

  Video

***************************************************************************/

PALETTE_INIT_MEMBER(seabattl_state, seabattl)
{
	// sprites (m.obj) + s2636
	for (int i = 0; i < 8; i++)
	{
		palette.set_pen_color(i, rgb_t((i & 1) ? 0xff : 0x00, (i & 2) ? 0xff : 0x00, (i & 4) ? 0xff : 0x00));
	}

	// scr
	for (int i = 0; i < 8; i++)
	{
		palette.set_pen_color(8 + 2 * i + 0, rgb_t::black);
		palette.set_pen_color(8 + 2 * i + 1, rgb_t((i & 1) ? 0xff : 0x00, (i & 2) ? 0xff : 0x00, (i & 4) ? 0xff : 0x00));
	}

	// wave
	palette.set_pen_color(24, rgb_t::black);
	palette.set_pen_color(25, rgb_t(0x00, 0xff, 0xff)); // cyan
}

TILE_GET_INFO_MEMBER(seabattl_state::get_bg_tile_info)
{
	int code = m_videoram[tile_index];
	int color = m_colorram[tile_index];

	SET_TILE_INFO_MEMBER(1, code, (color & 0x7), 0);
}

WRITE8_MEMBER(seabattl_state::seabattl_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(seabattl_state::seabattl_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

UINT32 seabattl_state::screen_update_seabattl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y, offset;

	// wave
	if ( m_waveenable )
	{
		for ( y = 0; y < 32; y++ )
		{
			for ( x = 0; x < 32; x++ )
			{
				m_gfxdecode->gfx(2)->opaque(bitmap,cliprect, (y & 0x0f) + (((x & 0x0f) + ((screen.frame_number() & 0xe0) >> 4)) << 4), 0, 0, 0, x*8, y*8 );
			}
		}
	}
	else
	{
		bitmap.fill(0, cliprect);
	}

	// background (scr.sm.obj)
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_bg_tilemap->draw(screen, m_collision_bg, cliprect, TILEMAP_DRAW_OPAQUE, 0);

	// sprites (m.obj)
	for ( offset = 0; offset < 256; offset++ )
	{
		// bits 0-3: sprite num
		// bits 4-7: x coordinate
		if ( m_objram[offset] & 0xf )
		{
			int code = (m_objram[offset] & 0x0f) | 0x10;
			int x = ((offset & 0x0f) << 4) - ((m_objram[offset] & 0xf0) >> 4);
			int y = (offset & 0xf0);

			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect, code, 0, 0, 0, x, y, 0);
		}
	}

	bitmap_ind16 &s2636_0_bitmap = m_s2636->update(cliprect);

	// collisions
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			// bit 0: m.obj - pvi-bkg
			// bit 1: pvi-bkg - scr.sm.obj
			// bit 2: m.obj - scr.sm.obj
			bool obj = (bitmap.pix(y,x) > 0) && (bitmap.pix(y,x) < 8);
			bool pvi = S2636_IS_PIXEL_DRAWN(s2636_0_bitmap.pix16(y, x));
			bool scr = (m_collision_bg.pix(y,x) & 1) != 0;

			if (obj && pvi)
				m_collision |= 0x01;

			if (pvi && scr)
				m_collision |= 0x02;

			if (obj && scr)
				m_collision |= 0x04;
		}
	}

	// s2636 layer
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			int pixel = s2636_0_bitmap.pix16(y, x);
			if (S2636_IS_PIXEL_DRAWN(pixel))
			{
				bitmap.pix16(y, x) = S2636_PIXEL_COLOR(pixel);
			}
		}
	}

	return 0;
}

void seabattl_state::video_start()
{
	m_screen->register_screen_bitmap(m_collision_bg);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(seabattl_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scrolldx(-12, 0);
}


/***************************************************************************

  Memory Maps, I/O

***************************************************************************/

static ADDRESS_MAP_START( seabattl_map, AS_PROGRAM, 8, seabattl_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x13ff) AM_ROM
	AM_RANGE(0x2000, 0x33ff) AM_ROM
	AM_RANGE(0x1400, 0x17ff) AM_MIRROR(0x2000) AM_RAM_WRITE(seabattl_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x1800, 0x1bff) AM_MIRROR(0x2000) AM_RAM_WRITE(seabattl_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x1c00, 0x1cff) AM_MIRROR(0x2000) AM_RAM
	AM_RANGE(0x1d00, 0x1dff) AM_MIRROR(0x2000) AM_RAM AM_SHARE("objram")
	AM_RANGE(0x1e00, 0x1e00) AM_MIRROR(0x20f0) AM_WRITE(time_display_w)
	AM_RANGE(0x1e01, 0x1e01) AM_MIRROR(0x20f0) AM_WRITE(score_display_w)
	AM_RANGE(0x1e02, 0x1e02) AM_MIRROR(0x20f0) AM_READ_PORT("IN0") AM_WRITE(score2_display_w)
	AM_RANGE(0x1e05, 0x1e05) AM_MIRROR(0x20f0) AM_READ_PORT("DIPS2")
	AM_RANGE(0x1e06, 0x1e06) AM_MIRROR(0x20f0) AM_READ_PORT("DIPS1") AM_WRITE(sound_w)
	AM_RANGE(0x1e07, 0x1e07) AM_MIRROR(0x20f0) AM_READ_PORT("DIPS0") AM_WRITE(sound2_w)
	AM_RANGE(0x1fcc, 0x1fcc) AM_MIRROR(0x2000) AM_READ_PORT("IN1")
	AM_RANGE(0x1f00, 0x1fff) AM_MIRROR(0x2000) AM_DEVREADWRITE("s2636", s2636_device, work_ram_r, work_ram_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( seabattl_io_map, AS_IO, 8, seabattl_state )
	AM_RANGE(S2650_CTRL_PORT, S2650_CTRL_PORT) AM_READWRITE( seabattl_collision_r, seabattl_control_w )
	AM_RANGE(S2650_DATA_PORT, S2650_DATA_PORT) AM_READWRITE( seabattl_collision_clear_r, seabattl_collision_clear_w )
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ_PORT("SENSE")
ADDRESS_MAP_END

READ8_MEMBER(seabattl_state::seabattl_collision_r)
{
	m_screen->update_partial(m_screen->vpos());
	return m_collision;
}

WRITE8_MEMBER(seabattl_state::seabattl_control_w)
{
	// bit 0: play counter
	// bit 1: super bonus counter
	// bit 2: coin counter
	// bit 3: inverse image
	// bit 4: lamp
	// bit 5: enable wave
	coin_counter_w(machine(), 0, BIT(data, 2));
	output_set_lamp_value(0, BIT(data,4));
	m_waveenable = BIT(data, 5);
}

READ8_MEMBER(seabattl_state::seabattl_collision_clear_r)
{
	m_screen->update_partial(m_screen->vpos());
	m_collision = 0;
	return 0;
}

WRITE8_MEMBER(seabattl_state::seabattl_collision_clear_w )
{
	m_screen->update_partial(m_screen->vpos());
	m_collision = 0;
}

WRITE8_MEMBER(seabattl_state::sound_w )
{
	// sound effects
	// bits:
	// 0 - missile
	// 1 - ship
	// 2 - aircraft
	// 3 - silence
	// 4 - torpedo
	// 5 - bomb
	// 6 - unused
	// 7 - unused
}

WRITE8_MEMBER(seabattl_state::sound2_w )
{
	// sound effects
	// bits:
	// 0 - melody
	// 1 - expl. a
	// 2 - expl. b
	// 3 - expl. c
	// 4 - expl. d
	// 5 - fall aircraft
	// 6 - unused
	// 7 - unused
}

WRITE8_MEMBER(seabattl_state::time_display_w )
{
	m_digit5->a_w(data & 0x0f);
	m_digit4->a_w((data >> 4) & 0x0f);
}

WRITE8_MEMBER(seabattl_state::score_display_w )
{
	m_digit3->a_w(data & 0x0f);
	m_digit2->a_w((data >> 4) & 0x0f);
}

WRITE8_MEMBER(seabattl_state::score2_display_w )
{
	m_digit1->a_w(data & 0x0f);
	m_digit0->a_w((data >> 4) & 0x0f);
}


/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( seabattl )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(20) PORT_KEYDELTA(20) PORT_CENTERDELTA(0) PORT_PLAYER(1)

	PORT_START("DIPS0")
	PORT_DIPNAME( 0x01, 0x00, "Game Type" ) PORT_DIPLOCATION("DS0:3")
	PORT_DIPSETTING(    0x00, "Time Based" )
	PORT_DIPSETTING(    0x01, "Lives Based" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Game_Time ) ) PORT_CONDITION("DIPS0", 0x01, EQUALS, 0x00) PORT_DIPLOCATION("DS0:2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) ) PORT_CONDITION("DIPS0", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x02, "75 seconds" )         PORT_CONDITION("DIPS0", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x06, "90 seconds" )         PORT_CONDITION("DIPS0", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x04, "105 seconds" )        PORT_CONDITION("DIPS0", 0x01, EQUALS, 0x00)
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Lives ) )     PORT_CONDITION("DIPS0", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) ) PORT_CONDITION("DIPS0", 0x01, EQUALS, 0x01) PORT_DIPLOCATION("DS0:2,1")
	PORT_DIPSETTING(    0x02, "3" )                  PORT_CONDITION("DIPS0", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x06, "4" )                  PORT_CONDITION("DIPS0", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x04, "5" )                  PORT_CONDITION("DIPS0", 0x01, EQUALS, 0x01)
	PORT_DIPNAME( 0x38, 0x08, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("DS0:6,5,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_7C ) )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DIPS1")
	PORT_DIPNAME( 0x01, 0x00, "Enemies Speed" ) PORT_DIPLOCATION("DS1:2")
	PORT_DIPSETTING(    0x01, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x06, 0x00, "Extended Play" ) PORT_DIPLOCATION("DS1:3,4")
	PORT_DIPSETTING(    0x06, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, "2000 points" )
	PORT_DIPSETTING(    0x02, "3000 points" )
	PORT_DIPSETTING(    0x00, "4000 points" )
	PORT_DIPNAME( 0x38, 0x08, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("DS0:8,DS1:1,DS0:7")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_7C ) )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DIPS2")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "DS1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "DS1:6" )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_HIGH, "DS1:5")
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "DS1:8" )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SENSE")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END


/***************************************************************************

  Machine Config/Interface

***************************************************************************/

void seabattl_state::machine_start()
{
}

void seabattl_state::machine_reset()
{
}

INTERRUPT_GEN_MEMBER(seabattl_state::seabattl_interrupt)
{
	device.execute().set_input_line_and_vector(0, HOLD_LINE, 0x03);
}

static const gfx_layout tiles32x16x3_layout =
{
	32,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 384,385,386,387,388,389,390,391, 0, 1, 2, 3, 4, 5, 6, 7, 128,129,130,131,132,133,134,135, 256,257,258,259,260,261,262,263 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	16*8*4
};


static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8
};

static GFXDECODE_START( seabattl )
	GFXDECODE_ENTRY( "gfx1", 0, tiles32x16x3_layout, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout, 8, 8 )
	GFXDECODE_ENTRY( "gfx3", 0, tiles8x8_layout, 24, 1 )
GFXDECODE_END

static MACHINE_CONFIG_START( seabattl, seabattl_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", S2650, 14318180/4/2)
	MCFG_CPU_PROGRAM_MAP(seabattl_map)
	MCFG_CPU_IO_MAP(seabattl_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", seabattl_state, seabattl_interrupt)

	MCFG_DEVICE_ADD("s2636", S2636, 0)
	MCFG_S2636_WORKRAM_SIZE(0x100)
	MCFG_S2636_OFFSETS(3, -21)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MCFG_DEVICE_ADD("sc_thousand", DM9368, 0)
	MCFG_OUTPUT_INDEX(0)
	MCFG_DEVICE_ADD("sc_hundred", DM9368, 0)
	MCFG_OUTPUT_INDEX(1)
	MCFG_DEVICE_ADD("sc_half", DM9368, 0)
	MCFG_OUTPUT_INDEX(2)
	MCFG_DEVICE_ADD("sc_unity", DM9368, 0)
	MCFG_OUTPUT_INDEX(3)
	MCFG_DEVICE_ADD("tm_half", DM9368, 0)
	MCFG_OUTPUT_INDEX(4)
	MCFG_DEVICE_ADD("tm_unity", DM9368, 0)
	MCFG_OUTPUT_INDEX(5)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(1*8, 29*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(seabattl_state, screen_update_seabattl)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", seabattl)
	MCFG_PALETTE_ADD("palette", 26)
	MCFG_PALETTE_INIT_OWNER(seabattl_state, seabattl)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	/* discrete sound */
MACHINE_CONFIG_END



/******************************************************************************/

ROM_START( seabattl )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "sea b b_1 1.prg",  0x0000, 0x0400, CRC(16a475c0) SHA1(5380d3be39c421227e52012d1bcf0516e99f6a3f) )
	ROM_CONTINUE(                 0x2000, 0x0400 )
	ROM_LOAD( "sea b b_1 2.prg",  0x0400, 0x0400, CRC(4bd73a82) SHA1(9ab4edf24fcd437ecd8e9e551ce0ed33be3bbad7) )
	ROM_CONTINUE(                 0x2400, 0x0400 )
	ROM_LOAD( "sea b b_1 3.prg",  0x0800, 0x0400, CRC(e251492b) SHA1(a152f9b6f189909ff478b4d95ee764f1898405b5) )
	ROM_CONTINUE(                 0x2800, 0x0400 )
	ROM_LOAD( "sea b b_1 4.prg",  0x0c00, 0x0400, CRC(6012b83f) SHA1(57de9e45253609b71f14fb3541760fd33647a651) )
	ROM_CONTINUE(                 0x2c00, 0x0400 )
	ROM_LOAD( "sea b b_1 5.prg",  0x1000, 0x0400, CRC(55c263f6) SHA1(33eba61cb8c9318cf19b771c93a14397b4ee0ace) )
	ROM_CONTINUE(                 0x3000, 0x0400 )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "sea b red.prg",    0x0000, 0x0800, CRC(fe7192df) SHA1(0b262bc1ac959d8dd79d71780e16237075f4a099) )
	ROM_LOAD( "sea b green.prg",  0x0800, 0x0800, CRC(cea4c0c9) SHA1(697c136ef363676b346692740d3c3a482dde6207) )
	ROM_LOAD( "sea b blu.prg",    0x1000, 0x0800, CRC(cd972c4a) SHA1(fcb8149bc462912c8393431ccb792ea4b1b1109d) )

	ROM_REGION( 0x0800, "gfx2", 0 )
	ROM_LOAD( "sea b screen.prg", 0x0000, 0x0800, CRC(8e4391dd) SHA1(f5698d66e5a3c46082b515ce86f9d3e96fd9ff77) )

	ROM_REGION( 0x0800, "gfx3", 0 )
	ROM_LOAD( "sea b wawe.prg",   0x0000, 0x0800, CRC(7e356dc5) SHA1(71d34fa39ff0b7d0fa6d32ba2b9dc0006a03d1bb) )
ROM_END

ROM_START( seabattla ) // this was a very different looking PCB (bootleg called armada maybe?) most parts had been stripped
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "program roms",     0x0000, 0x0400, NO_DUMP )

	ROM_REGION( 0xc00, "gfx1", 0 ) // probably the same as above without the blank data at the start
	ROM_LOAD( "armadared.ic26",   0x0000, 0x0400, CRC(b588f509) SHA1(073f9dc584aba1351969ef597cd80a0037938dfb) )
	ROM_LOAD( "armadagreen.ic25", 0x0400, 0x0400, CRC(3cc861c9) SHA1(d9159ee045cc0994f468035ae28cd8b79b5985ee) )
	ROM_LOAD( "armadablu.ic24",   0x0800, 0x0400, CRC(3689e530) SHA1(b30ab0d5ddc9b296437aa1bc2887f1416eb69f9c) )

	ROM_REGION( 0x0800, "gfx2", 0 )
	ROM_LOAD( "greenobj.ic38",    0x0000, 0x0800, CRC(81a9a741) SHA1(b2725c320a232d4abf6e6fc58ccf6a5edb8dd9a0) )

	ROM_REGION( 0x0800, "gfx3", 0 )
	ROM_LOAD( "seawawe.ic9",      0x0000, 0x0800, CRC(7e356dc5) SHA1(71d34fa39ff0b7d0fa6d32ba2b9dc0006a03d1bb) ) // identical to above set
ROM_END


GAMEL(1980, seabattl,  0,        seabattl, seabattl, driver_device, 0, ROT0, "Zaccaria", "Sea Battle (set 1)", MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND, layout_seabattl )
GAMEL(1980, seabattla, seabattl, seabattl, seabattl, driver_device, 0, ROT0, "Zaccaria", "Sea Battle (set 2)", MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND | MACHINE_NOT_WORKING, layout_seabattl ) // incomplete dump
