/*

Sea Battle by Zaccaria

driver by Mariusz Wojcieszek, hap, Phil Bennett and David Haywood

TODO: 
- improve collision detection?
- verify colors
- video timing
- discrete sound

Memory map in pics...


2650 + 2636

sea b b_1 *.prg are 2650 progamm

sea b blu.prg is blue data?
sea b red.prg is red data?
sea b green.prg is green data?  for video?

sea b wawe.prg is sea wave data?

sea b screen.prg ???


the sound board should be fully discrete.


DS0     1   2   3
PLAY TIME   ON  ON  ON  free game
        ON  OFF ON  75 seconds
        OFF     OFF ON  90 seconds
        OFF ON  ON  105 seconds

SHIP NUMBER ON  ON  OFF free game
        ON  OFF OFF 3 ships
        OFF OFF OFF 4 ships
        OFF ON  OFF 5 ships
    I don't forget anything, this is a copy of the manual
    DS0-3   seem to select from time based games to ships based game.



DS0     4   5   6
COIN SLOT 2 ON  ON  ON  2 coin 1 play
        ON  OFF ON  1 coin 1 play
        ON  ON  OFF 1 coin 2 plays
        ON  OFF OFF 1 coin 3 plays
        OFF ON  ON  1 coin 4 plays
        OFF OFF ON  1 coin 5 plays
        OFF ON  OFF 1 coin 6 plays
        OFF OFF OFF 1 coin 7 plays


DS0     7   8   DS1-1
COIN SLOT 1 ON  ON  ON  2 coin 1 play
        ON  OFF ON  1 coin 1 play
        ON  ON  OFF 1 coin 2 plays
        ON  OFF OFF 1 coin 3 plays
        OFF ON  ON  1 coin 4 plays
        OFF OFF ON  1 coin 5 plays
        OFF ON  OFF 1 coin 6 plays
        OFF OFF OFF 1 coin 7 plays

DS1     2
SHIP SPEED  ON  fast
        OFF slow


DS1     3   4
EXTEND PLAY OFF OFF no extended
        ON  OFF 2000 points
        OFF ON  3000 points
        ON  ON  4000 points

DS1     5
GRID        ON  game
        OFF grid

DS1 6-7-8 not used

*/

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "sound/s2636.h"
#include "video/s2636.h"
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
		m_mobjram(*this, "mobjram"),
		m_digit0(*this, "sc_thousand"),
		m_digit1(*this, "sc_hundred"),
		m_digit2(*this, "sc_half"),
		m_digit3(*this, "sc_unity"),
		m_digit4(*this, "tm_half"),
		m_digit5(*this, "tm_unity"),
		m_waveenable(false),
		m_collision(0)
	{
	}

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_mobjram;
	required_device<dm9368_device> m_digit0;
	required_device<dm9368_device> m_digit1;
	required_device<dm9368_device> m_digit2;
	required_device<dm9368_device> m_digit3;
	required_device<dm9368_device> m_digit4;
	required_device<dm9368_device> m_digit5;

	tilemap_t *m_bg_tilemap;
	bitmap_ind16 m_collision_bg;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	DECLARE_WRITE8_MEMBER(seabattl_videoram_w);
	DECLARE_WRITE8_MEMBER(seabattl_colorram_w);
	DECLARE_WRITE8_MEMBER(seabattl_wrtc_w);
	DECLARE_READ8_MEMBER(seabattl_redc_r);
	DECLARE_WRITE8_MEMBER(seabattl_portd_w);
	DECLARE_READ8_MEMBER(seabattl_portd_r);
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_WRITE8_MEMBER(sound2_w);
	DECLARE_WRITE8_MEMBER(time_display_w);
	DECLARE_WRITE8_MEMBER(score_display_w);
	DECLARE_WRITE8_MEMBER(score2_display_w);
	DECLARE_READ8_HANDLER(input_1e05_r);
	DECLARE_READ8_HANDLER(input_1e06_r);
	DECLARE_READ8_HANDLER(input_1e07_r);

	INTERRUPT_GEN_MEMBER(seabattl_interrupt);

	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_seabattl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	bool m_waveenable;
	UINT8 m_collision;
};

void seabattl_state::palette_init()
{
	int i;
	// m.obj
	for ( i = 0; i < 8; i++ )
	{
		palette_set_color( machine(), i, MAKE_RGB( (i&1)?0xff:0x00, (i&2)?0xff:0x00, (i&4)?0xff:0x00) );
	}
	// scr
	for(i=0;i<8;++i)
	{
		palette_set_color( machine(),8+2*i, MAKE_RGB(0x00, 0x00, 0x00) );
		palette_set_color( machine(),8+2*i+1, MAKE_RGB( (i&1)?0xff:0x00, (i&2)?0xff:0x00, (i&4)?0xff:0x00) );
	}
	// wave
	palette_set_color( machine(), 24, RGB_BLACK );
	palette_set_color( machine(), 25, RGB_WHITE );
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

	static int s2636_enabled = 1;
	static int mobj_enabled = 1;
	static int tilemap_enabled = 1;

	if(machine().input().code_pressed_once(KEYCODE_Q))
	{
		s2636_enabled ^= 1;
		popmessage("S2636 layer %sabled", s2636_enabled ? "en" : "dis");
	}
	if(machine().input().code_pressed_once(KEYCODE_W))
	{
		mobj_enabled ^= 1;
		popmessage("mobj layer %sabled", mobj_enabled ? "en" : "dis");
	}
	if(machine().input().code_pressed_once(KEYCODE_E))
	{
		tilemap_enabled ^= 1;
		popmessage("mobj layer %sabled", tilemap_enabled ? "en" : "dis");
	}

	// wave
	if ( m_waveenable )
	{
		for ( y = 0; y < 32; y++ )
		{
			for ( x = 0; x < 32; x++ )
			{
				drawgfx_opaque( bitmap, cliprect, machine().gfx[2], (y & 0x0f) + (((x & 0x0f) + ((screen.frame_number() & 0xe0) >> 4)) << 4), 0, 0, 0, x*8, y*8 );
			}
		}
	}
	else
	{
		bitmap.fill(get_black_pen(machine()), cliprect);
	}
	// bg tilemap
	if ( tilemap_enabled )
	{
		m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
		m_bg_tilemap->draw(m_collision_bg, cliprect, 0, 0);
	}

	// m.obj
	if ( mobj_enabled )
	{
		for ( offset = 0; offset < 256; offset++ )
		{
			// bits 0-3: sprite num
			// bits 4-7: x coordinate
			if ( m_mobjram[offset] & 0xf )
			{
				drawgfx_transpen(bitmap, cliprect, machine().gfx[0], (m_mobjram[offset] & 0x0f) | 0x10, 0, 0, 0, ((offset & 0x0f) << 4) - ((m_mobjram[offset] & 0xf0) >> 4), (offset & 0xf0), 0 );
			}
		}
	}

	// s2636 layer
	bitmap_ind16 &s2636_0_bitmap = s2636_update(machine().device("s2636"), cliprect);

	// collisions
	// bit 0: m.obj - pvi-bkg
	// bit 1: pvi-bkg - src.sm.obj
	// bit 2: m.obj - src.sm.obj
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			if ( (bitmap.pix(y,x) > 0) && (bitmap.pix(y,x) < 8) && S2636_IS_PIXEL_DRAWN(s2636_0_bitmap.pix16(y, x)) )
			{
				m_collision |= 0x01;
			}
			if ( S2636_IS_PIXEL_DRAWN(s2636_0_bitmap.pix16(y, x)) && (m_collision_bg.pix(y,x) > 8) && (m_collision_bg.pix(y,x) < 24) && (palette_get_color(machine(), m_collision_bg.pix(y,x)) != RGB_BLACK))
			{
				m_collision |= 0x02;
			}
			if ( ( bitmap.pix(y,x) > 0  ) && ( bitmap.pix(y,x) < 8 ) && ( m_collision_bg.pix(y,x) < 24 ) && (m_collision_bg.pix(y,x) > 8) && (palette_get_color(machine(), m_collision_bg.pix(y,x)) != RGB_BLACK)) 
			{
				m_collision |= 0x04;
			}
		}
	}

	if ( s2636_enabled )
	{
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
	}
	return 0;
}

void seabattl_state::video_start()
{
	machine().primary_screen->register_screen_bitmap(m_collision_bg);
	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(seabattl_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_transparent_pen(0);
	machine().gfx[1]->set_colorbase(8);
	machine().gfx[2]->set_colorbase(24);
}

static ADDRESS_MAP_START( seabattl_map, AS_PROGRAM, 8, seabattl_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x13ff) AM_ROM
	AM_RANGE(0x2000, 0x33ff) AM_ROM

	AM_RANGE(0x1400, 0x17ff) AM_MIRROR(0x2000) AM_RAM_WRITE(seabattl_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x1800, 0x1bff) AM_MIRROR(0x2000) AM_RAM_WRITE(seabattl_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x1c00, 0x1cff) AM_MIRROR(0x2000) AM_RAM
	AM_RANGE(0x1d00, 0x1dff) AM_MIRROR(0x2000) AM_RAM AM_SHARE("mobjram")
	AM_RANGE(0x1e00, 0x1e00) AM_MIRROR(0x20f0) AM_WRITE(time_display_w)
	AM_RANGE(0x1e01, 0x1e01) AM_MIRROR(0x20f0) AM_WRITE(score_display_w)
	AM_RANGE(0x1e02, 0x1e02) AM_MIRROR(0x20f0) AM_READ_PORT("IN0") AM_WRITE(score2_display_w)
	AM_RANGE(0x1e05, 0x1e05) AM_MIRROR(0x20f0) AM_READ(input_1e05_r)
	AM_RANGE(0x1e06, 0x1e06) AM_MIRROR(0x20f0) AM_READ(input_1e06_r) AM_WRITE(sound_w)
	AM_RANGE(0x1e07, 0x1e07) AM_MIRROR(0x20f0) AM_READ(input_1e07_r) AM_WRITE(sound2_w)
	AM_RANGE(0x1fcc, 0x1fcc) AM_MIRROR(0x2000) AM_READ_PORT("IN1")
	AM_RANGE(0x1f00, 0x1fff) AM_MIRROR(0x2000) AM_DEVREADWRITE_LEGACY("s2636", s2636_work_ram_r, s2636_work_ram_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( seabattl_io_map, AS_IO, 8, seabattl_state )
	AM_RANGE(S2650_CTRL_PORT, S2650_CTRL_PORT) AM_READWRITE( seabattl_redc_r, seabattl_wrtc_w )
	AM_RANGE(S2650_DATA_PORT, S2650_DATA_PORT) AM_READWRITE( seabattl_portd_r, seabattl_portd_w )
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ_PORT("SENSE")
ADDRESS_MAP_END

READ8_HANDLER(seabattl_state::seabattl_redc_r)
{
	machine().primary_screen->update_partial(machine().primary_screen->vpos());
	return m_collision;
}

WRITE8_MEMBER(seabattl_state::seabattl_wrtc_w)
{
	// bit 0: play counter
	// bit 1: super bonus counter
	coin_counter_w( machine(), 0, BIT(data, 2) );
	// bit 3: inverse image
	output_set_lamp_value(0, BIT(data,4) );
	m_waveenable = BIT(data, 5);
}

READ8_HANDLER(seabattl_state::seabattl_portd_r)
{
	machine().primary_screen->update_partial(machine().primary_screen->vpos());
	m_collision = 0;
	return 0;
}

WRITE8_HANDLER(seabattl_state::seabattl_portd_w )
{
	machine().primary_screen->update_partial(machine().primary_screen->vpos());
	m_collision = 0;
}

WRITE8_HANDLER(seabattl_state::sound_w )
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

WRITE8_HANDLER(seabattl_state::sound2_w )
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

WRITE8_HANDLER(seabattl_state::time_display_w )
{
	m_digit5->a_w(data & 0x0f);
	m_digit4->a_w((data >> 4) & 0x0f);
}

WRITE8_HANDLER(seabattl_state::score_display_w )
{
	m_digit3->a_w(data & 0x0f);
	m_digit2->a_w((data >> 4) & 0x0f);
}

WRITE8_HANDLER(seabattl_state::score2_display_w )
{
	m_digit1->a_w(data & 0x0f);
	m_digit0->a_w((data >> 4) & 0x0f);
}

READ8_HANDLER(seabattl_state::input_1e05_r)
{
	UINT8 val = 0xf0;
	UINT8 dsw1 = ioport("DSW1")->read();
	val |= (BIT(dsw1,6) ? 1 : 0);
	val |= (BIT(dsw1,5) ? 2 : 0);
	val |= (BIT(dsw1,4) ? 4 : 0);
	val |= (BIT(dsw1,7) ? 8 : 0);
	return val;
}

READ8_HANDLER(seabattl_state::input_1e06_r)
{
	UINT8 val = 0xc0;
	UINT8 dsw1 = ioport("DSW1")->read();
	val |= (BIT(dsw1,1) ? 1 : 0);
	val |= (BIT(dsw1,2) ? 2 : 0);
	val |= (BIT(dsw1,3) ? 4 : 0);
	val |= (BIT(dsw1,0) ? 8 : 0);
	val |= (BIT(dsw1,6) ? 16 : 0);
	val |= (BIT(dsw1,7) ? 32 : 0);
	return val;
}

READ8_HANDLER(seabattl_state::input_1e07_r)
{
	UINT8 val = 0xc0;
	UINT8 dsw0 = ioport("DSW0")->read();
	val |= (BIT(dsw0,2) ? 1 : 0);
	val |= (BIT(dsw0,1) ? 2 : 0);
	val |= (BIT(dsw0,0) ? 4 : 0);
	val |= (BIT(dsw0,3) ? 8 : 0);
	val |= (BIT(dsw0,5) ? 16 : 0);
	val |= (BIT(dsw0,4) ? 32 : 0);
	return val;
}

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

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Game_Time ) ) PORT_CONDITION("DSW0", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) ) PORT_CONDITION("DSW0", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0x02, "75 seconds" ) PORT_CONDITION("DSW0", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0x03, "90 seconds" ) PORT_CONDITION("DSW0", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0x01, "105 seconds" ) PORT_CONDITION("DSW0", 0x04, EQUALS, 0x00)
	PORT_DIPNAME( 0x03, 0x02, "Ships number" ) PORT_CONDITION("DSW0", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) ) PORT_CONDITION("DSW0", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(    0x02, "3 ships" ) PORT_CONDITION("DSW0", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(    0x03, "4 ships" ) PORT_CONDITION("DSW0", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(    0x01, "5 ships" ) PORT_CONDITION("DSW0", 0x04, EQUALS, 0x04)
	PORT_DIPNAME( 0x04, 0x00, "Game Type" )
	PORT_DIPSETTING(    0x00, "Time based" )
	PORT_DIPSETTING(    0x04, "Ships based" )
	PORT_DIPNAME( 0x38, 0x10, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_3C ) ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_6C ) ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_7C ) ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "Coin B Mode" )
	PORT_DIPSETTING(    0x01, "Coin B Mode 2" )
	PORT_DIPSETTING(    0x00, "Coin B Mode 1" )
	PORT_DIPNAME( 0x02, 0x00, "Ships speed" )
	PORT_DIPSETTING(    0x02, "Slow ships" )
	PORT_DIPSETTING(    0x00, "Fast ships" )
	PORT_DIPNAME( 0x0c, 0x0c, "Extend play" )
	PORT_DIPSETTING(    0x0c, "Not extended" )
	PORT_DIPSETTING(    0x08, "2000 points" )
	PORT_DIPSETTING(    0x04, "3000 points" )
	PORT_DIPSETTING(    0x00, "4000 points" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Test ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPUNUSED( 0xe0, 0xe0 )

	PORT_START("SENSE")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END

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
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout, 0, 1 )
	GFXDECODE_ENTRY( "gfx3", 0, tiles8x8_layout, 0, 1 )
GFXDECODE_END

static const s2636_interface s2636_config =
{
	"screen",
	0x100,
	0, -16,
	"s2636snd"
};

static DM9368_INTERFACE( digit_score_thousand_intf )
{
	0,
	DEVCB_NULL,
	DEVCB_NULL
};

static DM9368_INTERFACE( digit_score_hundred_intf )
{
	1,
	DEVCB_NULL,
	DEVCB_NULL
};

static DM9368_INTERFACE( digit_score_half_a_score_intf )
{
	2,
	DEVCB_NULL,
	DEVCB_NULL
};

static DM9368_INTERFACE( digit_score_unity_intf )
{
	3,
	DEVCB_NULL,
	DEVCB_NULL
};

static DM9368_INTERFACE( digit_time_half_a_score_intf )
{
	4,
	DEVCB_NULL,
	DEVCB_NULL
};

static DM9368_INTERFACE( digit_time_unity_intf )
{
	5,
	DEVCB_NULL,
	DEVCB_NULL
};

static MACHINE_CONFIG_START( seabattl, seabattl_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", S2650, 14318180/4/2)
	MCFG_CPU_PROGRAM_MAP(seabattl_map)
	MCFG_CPU_IO_MAP(seabattl_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", seabattl_state, seabattl_interrupt)

	MCFG_PALETTE_LENGTH(26)

	MCFG_GFXDECODE(seabattl)

	MCFG_S2636_ADD("s2636", s2636_config)

	MCFG_DM9368_ADD("sc_thousand", digit_score_thousand_intf)
	MCFG_DM9368_ADD("sc_hundred", digit_score_hundred_intf)
	MCFG_DM9368_ADD("sc_half", digit_score_half_a_score_intf)
	MCFG_DM9368_ADD("sc_unity", digit_score_unity_intf)
	MCFG_DM9368_ADD("tm_half", digit_time_half_a_score_intf)
	MCFG_DM9368_ADD("tm_unity", digit_time_unity_intf)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(1*8, 29*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(seabattl_state, screen_update_seabattl)
	MCFG_DEFAULT_LAYOUT(layout_seabattl)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("s2636snd", S2636_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* discrete sound */
MACHINE_CONFIG_END


ROM_START( seabattl )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "sea b b_1 1.prg",      0x0000, 0x0400, CRC(16a475c0) SHA1(5380d3be39c421227e52012d1bcf0516e99f6a3f) )
	ROM_CONTINUE(                     0x2000, 0x0400 )
	ROM_LOAD( "sea b b_1 2.prg",      0x0400, 0x0400, CRC(4bd73a82) SHA1(9ab4edf24fcd437ecd8e9e551ce0ed33be3bbad7) )
	ROM_CONTINUE(                     0x2400, 0x0400 )
	ROM_LOAD( "sea b b_1 3.prg",      0x0800, 0x0400, CRC(e251492b) SHA1(a152f9b6f189909ff478b4d95ee764f1898405b5) )
	ROM_CONTINUE(                     0x2800, 0x0400 )
	ROM_LOAD( "sea b b_1 4.prg",      0x0c00, 0x0400, CRC(6012b83f) SHA1(57de9e45253609b71f14fb3541760fd33647a651) )
	ROM_CONTINUE(                     0x2c00, 0x0400 )
	ROM_LOAD( "sea b b_1 5.prg",      0x1000, 0x0400, CRC(55c263f6) SHA1(33eba61cb8c9318cf19b771c93a14397b4ee0ace) )
	ROM_CONTINUE(                     0x3000, 0x0400 )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "sea b red.prg",      0x0000, 0x0800, CRC(fe7192df) SHA1(0b262bc1ac959d8dd79d71780e16237075f4a099) )
	ROM_LOAD( "sea b green.prg",    0x0800, 0x0800, CRC(cea4c0c9) SHA1(697c136ef363676b346692740d3c3a482dde6207) )
	ROM_LOAD( "sea b blu.prg",      0x1000, 0x0800, CRC(cd972c4a) SHA1(fcb8149bc462912c8393431ccb792ea4b1b1109d) )

	ROM_REGION( 0x0800, "gfx2", 0 )
	ROM_LOAD( "sea b screen.prg",     0x0000, 0x0800, CRC(4e98f719) SHA1(2cdbc23aed790807b2dc730258916cc32dab1a31) )

	ROM_REGION( 0x0800, "gfx3", 0 )
	ROM_LOAD( "sea b wawe.prg",     0x0000, 0x0800, CRC(7e356dc5) SHA1(71d34fa39ff0b7d0fa6d32ba2b9dc0006a03d1bb) )
ROM_END

ROM_START( seabattla ) // this was a very different looking PCB (bootleg called armada maybe?) most parts had been stripped
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "program roms",      0x0000, 0x0400, NO_DUMP )

	ROM_REGION( 0xc00, "gfx1", 0 ) // probably the same as above without the blank data at the start
	ROM_LOAD( "armadared.ic26",      0x0000, 0x0400, CRC(b588f509) SHA1(073f9dc584aba1351969ef597cd80a0037938dfb) )
	ROM_LOAD( "armadagreen.ic25",    0x0400, 0x0400, CRC(3cc861c9) SHA1(d9159ee045cc0994f468035ae28cd8b79b5985ee) )
	ROM_LOAD( "armadablu.ic24",      0x0800, 0x0400, CRC(3689e530) SHA1(b30ab0d5ddc9b296437aa1bc2887f1416eb69f9c) )

	ROM_REGION( 0x0800, "gfx2", 0 )
	ROM_LOAD( "greenobj.ic38",     0x0000, 0x0800, CRC(81a9a741) SHA1(b2725c320a232d4abf6e6fc58ccf6a5edb8dd9a0) )

	ROM_REGION( 0x0800, "gfx3", 0 )
	ROM_LOAD( "seawawe.ic9",     0x0000, 0x0800, CRC(7e356dc5) SHA1(71d34fa39ff0b7d0fa6d32ba2b9dc0006a03d1bb) ) // identical to above set
ROM_END

GAME( 1980, seabattl,  0,        seabattl, seabattl, driver_device, 0, ROT0, "Zaccaria", "Sea Battle (set 1)", GAME_IMPERFECT_COLORS | GAME_IMPERFECT_GRAPHICS | GAME_NO_SOUND )
GAME( 1980, seabattla, seabattl, seabattl, seabattl, driver_device, 0, ROT0, "Zaccaria", "Sea Battle (set 2)", GAME_IMPERFECT_COLORS | GAME_IMPERFECT_GRAPHICS | GAME_NO_SOUND | GAME_NOT_WORKING )
