// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/***************************************************************************

Atari Flyball Driver

Etched in copper on top of board:
    FLYBALL
    ATARI (c)1976
    A005629
    MADE IN USA
    PAT NO 3793483


TODO:
- discrete sound
- accurate video timing

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"

#define MASTER_CLOCK    XTAL_12_096MHz
#define PIXEL_CLOCK     (MASTER_CLOCK / 2)


class flyball_state : public driver_device
{
public:
	enum
	{
		TIMER_POT_ASSERT,
		TIMER_POT_CLEAR,
		TIMER_QUARTER
	};

	flyball_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_playfield_ram(*this, "playfield_ram") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<UINT8> m_playfield_ram;

	/* video-related */
	tilemap_t  *m_tmap;
	UINT8    m_pitcher_vert;
	UINT8    m_pitcher_horz;
	UINT8    m_pitcher_pic;
	UINT8    m_ball_vert;
	UINT8    m_ball_horz;

	/* misc */
	UINT8    m_potmask;
	UINT8    m_potsense;

	emu_timer *m_pot_clear_timer;
	emu_timer *m_quarter_timer;

	DECLARE_READ8_MEMBER(input_r);
	DECLARE_READ8_MEMBER(scanline_r);
	DECLARE_READ8_MEMBER(potsense_r);
	DECLARE_WRITE8_MEMBER(potmask_w);
	DECLARE_WRITE8_MEMBER(pitcher_pic_w);
	DECLARE_WRITE8_MEMBER(ball_vert_w);
	DECLARE_WRITE8_MEMBER(ball_horz_w);
	DECLARE_WRITE8_MEMBER(pitcher_vert_w);
	DECLARE_WRITE8_MEMBER(pitcher_horz_w);
	DECLARE_WRITE8_MEMBER(misc_w);

	TILEMAP_MAPPER_MEMBER(get_memory_offset);
	TILE_GET_INFO_MEMBER(get_tile_info);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(flyball);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_CALLBACK_MEMBER(joystick_callback);
	TIMER_CALLBACK_MEMBER(quarter_callback);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};


/*************************************
 *
 *  Video emulation
 *
 *************************************/

TILEMAP_MAPPER_MEMBER(flyball_state::get_memory_offset)
{
	if (col == 0)
		col = num_cols;

	return num_cols * (num_rows - row) - col;
}


TILE_GET_INFO_MEMBER(flyball_state::get_tile_info)
{
	UINT8 data = m_playfield_ram[tile_index];
	int flags = ((data & 0x40) ? TILE_FLIPX : 0) | ((data & 0x80) ? TILE_FLIPY : 0);
	int code = data & 63;

	if ((flags & TILE_FLIPX) && (flags & TILE_FLIPY))
		code += 64;

	SET_TILE_INFO_MEMBER(0, code, 0, flags);
}


void flyball_state::video_start()
{
	m_tmap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(flyball_state::get_tile_info),this), tilemap_mapper_delegate(FUNC(flyball_state::get_memory_offset),this), 8, 16, 32, 16);
}


UINT32 flyball_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int pitcherx = m_pitcher_horz;
	int pitchery = m_pitcher_vert - 31;

	int ballx = m_ball_horz - 1;
	int bally = m_ball_vert - 17;

	m_tmap->mark_all_dirty();

	/* draw playfield */
	m_tmap->draw(screen, bitmap, cliprect, 0, 0);

	/* draw pitcher */
	m_gfxdecode->gfx(1)->transpen(bitmap,cliprect, m_pitcher_pic ^ 0xf, 0, 1, 0, pitcherx, pitchery, 1);

	/* draw ball */
	for (int y = bally; y < bally + 2; y++)
		for (int x = ballx; x < ballx + 2; x++)
			if (cliprect.contains(x, y))
				bitmap.pix16(y, x) = 1;

	return 0;
}


void flyball_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_POT_ASSERT:
		joystick_callback(ptr, param);
		break;
	case TIMER_POT_CLEAR:
		m_maincpu->set_input_line(0, CLEAR_LINE);
		break;
	case TIMER_QUARTER:
		quarter_callback(ptr, param);
		break;

	default:
		assert_always(FALSE, "Unknown id in flyball_state::device_timer");
	}
}


TIMER_CALLBACK_MEMBER(flyball_state::joystick_callback)
{
	int potsense = param;

	if (potsense & ~m_potmask)
	{
		// pot irq is active at hsync
		m_maincpu->set_input_line(0, ASSERT_LINE);
		m_pot_clear_timer->adjust(attotime::from_ticks(32, PIXEL_CLOCK), 0);
	}

	m_potsense |= potsense;
}


TIMER_CALLBACK_MEMBER(flyball_state::quarter_callback)
{
	int scanline = param;
	int potsense[64], i;

	memset(potsense, 0, sizeof potsense);

	potsense[ioport("STICK1_Y")->read()] |= 1;
	potsense[ioport("STICK1_X")->read()] |= 2;
	potsense[ioport("STICK0_Y")->read()] |= 4;
	potsense[ioport("STICK0_X")->read()] |= 8;

	for (i = 0; i < 64; i++)
		if (potsense[i] != 0)
			timer_set(m_screen->time_until_pos(scanline + i), TIMER_POT_ASSERT, potsense[i]);

	scanline += 0x40;
	scanline &= 0xff;

	m_quarter_timer->adjust(m_screen->time_until_pos(scanline), scanline);

	m_potsense = 0;
	m_potmask = 0;
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

/* two physical buttons (start game and stop runner) share the same port bit */
READ8_MEMBER(flyball_state::input_r)
{
	return ioport("IN0")->read() & ioport("IN1")->read();
}

READ8_MEMBER(flyball_state::scanline_r)
{
	return m_screen->vpos() & 0x3f;
}

READ8_MEMBER(flyball_state::potsense_r)
{
	return m_potsense & ~m_potmask;
}

WRITE8_MEMBER(flyball_state::potmask_w)
{
	m_potmask |= data & 0xf;
}

WRITE8_MEMBER(flyball_state::pitcher_pic_w)
{
	m_pitcher_pic = data & 0xf;
}

WRITE8_MEMBER(flyball_state::ball_vert_w)
{
	m_ball_vert = data;
}

WRITE8_MEMBER(flyball_state::ball_horz_w)
{
	m_ball_horz = data;
}

WRITE8_MEMBER(flyball_state::pitcher_vert_w)
{
	m_pitcher_vert = data;
}

WRITE8_MEMBER(flyball_state::pitcher_horz_w)
{
	m_pitcher_horz = data;
}

WRITE8_MEMBER(flyball_state::misc_w)
{
	int bit = ~data & 1;

	switch (offset)
	{
	case 0:
		output().set_led_value(0, bit);
		break;
	case 1:
		/* crowd very loud */
		break;
	case 2:
		/* footstep off-on */
		break;
	case 3:
		/* crowd off-on */
		break;
	case 4:
		/* crowd soft-loud */
		break;
	case 5:
		/* bat hit */
		break;
	}
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( flyball_map, AS_PROGRAM, 8, flyball_state )
	ADDRESS_MAP_GLOBAL_MASK(0x1fff)
	AM_RANGE(0x0000, 0x00ff) AM_MIRROR(0x100) AM_RAM
	AM_RANGE(0x0800, 0x0800) AM_NOP
	AM_RANGE(0x0801, 0x0801) AM_WRITE(pitcher_pic_w)
	AM_RANGE(0x0802, 0x0802) AM_READ(scanline_r)
	AM_RANGE(0x0803, 0x0803) AM_READ(potsense_r)
	AM_RANGE(0x0804, 0x0804) AM_WRITE(ball_vert_w)
	AM_RANGE(0x0805, 0x0805) AM_WRITE(ball_horz_w)
	AM_RANGE(0x0806, 0x0806) AM_WRITE(pitcher_vert_w)
	AM_RANGE(0x0807, 0x0807) AM_WRITE(pitcher_horz_w)
	AM_RANGE(0x0900, 0x0900) AM_WRITE(potmask_w)
	AM_RANGE(0x0a00, 0x0a07) AM_WRITE(misc_w)
	AM_RANGE(0x0b00, 0x0b00) AM_READ(input_r)
	AM_RANGE(0x0d00, 0x0eff) AM_WRITEONLY AM_SHARE("playfield_ram")
	AM_RANGE(0x1000, 0x1fff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( flyball )
	PORT_START("IN0") /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW ) PORT_DIPLOCATION("DSW1:6")
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DSW1:4,5")
	PORT_DIPSETTING( 0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING( 0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING( 0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x40, 0x40, "Innings Per Game" ) PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING( 0x00, "1" )
	PORT_DIPSETTING( 0x40, "2" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "DSW1:1" )

	PORT_START("STICK1_Y") /* IN1 */
	PORT_BIT( 0x3f, 0x20, IPT_AD_STICK_Y ) PORT_MINMAX(1,63) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("STICK1_X") /* IN2 */
	PORT_BIT( 0x3f, 0x20, IPT_AD_STICK_X ) PORT_MINMAX(1,63) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("STICK0_Y") /* IN3 */
	PORT_BIT( 0x3f, 0x20, IPT_AD_STICK_Y ) PORT_MINMAX(1,63) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("STICK0_X") /* IN4 */
	PORT_BIT( 0x3f, 0x20, IPT_AD_STICK_X ) PORT_MINMAX(1,63) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("IN1") /* IN5 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xFE, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout flyball_tiles_layout =
{
	8, 16,    /* width, height */
	128,      /* total         */
	1,        /* planes        */
	{ 0 },    /* plane offsets */
	{
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
	},
	{
		0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38,
		0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x78
	},
	0x80      /* increment */
};

static const gfx_layout flyball_sprites_layout =
{
	16, 16,   /* width, height */
	16,       /* total         */
	1,        /* planes        */
	{ 0 },    /* plane offsets */
	{
		0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
		0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF
	},
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
		0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0
	},
	0x100     /* increment */
};

static GFXDECODE_START( flyball )
	GFXDECODE_ENTRY( "gfx1", 0, flyball_tiles_layout, 0, 2 )
	GFXDECODE_ENTRY( "gfx2", 0, flyball_sprites_layout, 2, 2 )
GFXDECODE_END


PALETTE_INIT_MEMBER(flyball_state, flyball)
{
	palette.set_pen_color(0, rgb_t(0x3F, 0x3F, 0x3F));  /* tiles, ball */
	palette.set_pen_color(1, rgb_t(0xFF, 0xFF, 0xFF));
	palette.set_pen_color(2, rgb_t(0xFF ,0xFF, 0xFF));  /* sprites */
	palette.set_pen_color(3, rgb_t(0x00, 0x00, 0x00));
}


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void flyball_state::machine_start()
{
	/* address bits 0 through 8 are inverted */
	UINT8 *ROM = memregion("maincpu")->base();
	int len = memregion("maincpu")->bytes();
	dynamic_buffer buf(len);
	for (int i = 0; i < len; i++)
		buf[i ^ 0x1ff] = ROM[i];
	memcpy(ROM, &buf[0], len);

	m_pot_clear_timer = timer_alloc(TIMER_POT_CLEAR);
	m_quarter_timer = timer_alloc(TIMER_QUARTER);

	save_item(NAME(m_pitcher_vert));
	save_item(NAME(m_pitcher_horz));
	save_item(NAME(m_pitcher_pic));
	save_item(NAME(m_ball_vert));
	save_item(NAME(m_ball_horz));
	save_item(NAME(m_potmask));
	save_item(NAME(m_potsense));
}

void flyball_state::machine_reset()
{
	m_quarter_timer->adjust(m_screen->time_until_pos(0));

	m_pitcher_vert = 0;
	m_pitcher_horz = 0;
	m_pitcher_pic = 0;
	m_ball_vert = 0;
	m_ball_horz = 0;
	m_potmask = 0;
	m_potsense = 0;
}


static MACHINE_CONFIG_START( flyball, flyball_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, MASTER_CLOCK/16)
	MCFG_CPU_PROGRAM_MAP(flyball_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", flyball_state, nmi_line_pulse)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(256, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 255, 0, 239)
	MCFG_SCREEN_UPDATE_DRIVER(flyball_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", flyball)
	MCFG_PALETTE_ADD("palette", 4)
	MCFG_PALETTE_INIT_OWNER(flyball_state, flyball)

	/* sound hardware */
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( flyball )
	ROM_REGION( 0x1000, "maincpu", 0 )  /* program */
	ROM_LOAD( "6129-02.d5", 0x0000, 0x0200, CRC(105ffe40) SHA1(20225571ccf76df5d96a42168d9223cccdff90a8) )
	ROM_LOAD( "6130-02.f5", 0x0200, 0x0200, CRC(188210e1) SHA1(6d837dd9ea44d16f0d54ea9e14260de5f7c05b6b) )
	ROM_LOAD( "6131-01.h5", 0x0400, 0x0200, CRC(a9c7e858) SHA1(aee4a359d6a5729dc1be5b8ce8fbe54d032d12b0) ) /* Roms found with and without the "-01" extension */
	ROM_LOAD( "6132-01.j5", 0x0600, 0x0200, CRC(31fefd8a) SHA1(97e3ef278ce2175cd33c0f3147bdf7974752c836) ) /* Roms found with and without the "-01" extension */
	ROM_LOAD( "6133-01.k5", 0x0800, 0x0200, CRC(6fdb09b1) SHA1(04ad412b437bb24739b3e31fa5a413e63d5897f8) ) /* Roms found with and without the "-01" extension */
	ROM_LOAD( "6134-01.m5", 0x0A00, 0x0200, CRC(7b526c73) SHA1(e47c8f33b7edc143ab1713556c59b93571933daa) ) /* Roms found with and without the "-01" extension */
	ROM_LOAD( "6135-01.n5", 0x0C00, 0x0200, CRC(b352cb51) SHA1(39b9062fb51d0a78a47dcd470ceae47fcdbd7891) ) /* Roms found with and without the "-01" extension */
	ROM_LOAD( "6136-02.r5", 0x0E00, 0x0200, CRC(ae06a0f5) SHA1(6034176b255eeaa2980e8fef1b17ef6f0a743941) )

	ROM_REGION( 0x0C00, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "6142.l2", 0x0000, 0x0200, CRC(65650cfa) SHA1(7d17455146fc9def22c7bd06f7fde32df0a0c2bc) )
	ROM_LOAD( "6139.j2", 0x0200, 0x0200, CRC(a5d1358e) SHA1(33cecbe40ae299549a3395e3dffbe7b6021803ba) )
	ROM_LOAD( "6141.m2", 0x0400, 0x0200, CRC(98b5f803) SHA1(c4e323ced2393fa4a9720ff0086c559fb9b3a9f8) )
	ROM_LOAD( "6140.k2", 0x0600, 0x0200, CRC(66aeec61) SHA1(f577bad015fe9e3708fd95d5d2bc438997d14d2c) )

	ROM_REGION( 0x0400, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "6137.e2", 0x0000, 0x0200, CRC(68961fda) SHA1(a06c7b453cce04716f49bd65ecfe1ba67cb8681e) )
	ROM_LOAD16_BYTE( "6138.f2", 0x0001, 0x0200, CRC(aab314f6) SHA1(6625c719fdc000d6af94bc9474de8f7e977cee97) )
ROM_END

ROM_START( flyball1 )
	ROM_REGION( 0x1000, "maincpu", 0 )  /* program */
	ROM_LOAD( "6129.d5", 0x0000, 0x0200, CRC(17eda069) SHA1(e4ef0bf4546cf00668d759a188e0989a4f003825) )
	ROM_LOAD( "6130.f5", 0x0200, 0x0200, CRC(a756955b) SHA1(220b7f1789bba4481d595b36b4bae25f98d3ad8d) )
	ROM_LOAD( "6131.h5", 0x0400, 0x0200, CRC(a9c7e858) SHA1(aee4a359d6a5729dc1be5b8ce8fbe54d032d12b0) )
	ROM_LOAD( "6132.j5", 0x0600, 0x0200, CRC(31fefd8a) SHA1(97e3ef278ce2175cd33c0f3147bdf7974752c836) )
	ROM_LOAD( "6133.k5", 0x0800, 0x0200, CRC(6fdb09b1) SHA1(04ad412b437bb24739b3e31fa5a413e63d5897f8) )
	ROM_LOAD( "6134.m5", 0x0A00, 0x0200, CRC(7b526c73) SHA1(e47c8f33b7edc143ab1713556c59b93571933daa) )
	ROM_LOAD( "6135.n5", 0x0C00, 0x0200, CRC(b352cb51) SHA1(39b9062fb51d0a78a47dcd470ceae47fcdbd7891) )
	ROM_LOAD( "6136.r5", 0x0E00, 0x0200, CRC(1622d890) SHA1(9ad342aefdc02e022eb79d84d1c856bed538bebe) )

	ROM_REGION( 0x0C00, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "6142.l2", 0x0000, 0x0200, CRC(65650cfa) SHA1(7d17455146fc9def22c7bd06f7fde32df0a0c2bc) )
	ROM_LOAD( "6139.j2", 0x0200, 0x0200, CRC(a5d1358e) SHA1(33cecbe40ae299549a3395e3dffbe7b6021803ba) )
	ROM_LOAD( "6141.m2", 0x0400, 0x0200, CRC(98b5f803) SHA1(c4e323ced2393fa4a9720ff0086c559fb9b3a9f8) )
	ROM_LOAD( "6140.k2", 0x0600, 0x0200, CRC(66aeec61) SHA1(f577bad015fe9e3708fd95d5d2bc438997d14d2c) )

	ROM_REGION( 0x0400, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "6137.e2", 0x0000, 0x0200, CRC(68961fda) SHA1(a06c7b453cce04716f49bd65ecfe1ba67cb8681e) )
	ROM_LOAD16_BYTE( "6138.f2", 0x0001, 0x0200, CRC(aab314f6) SHA1(6625c719fdc000d6af94bc9474de8f7e977cee97) )
ROM_END


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1976, flyball,  0,       flyball, flyball, driver_device, 0, 0, "Atari", "Flyball (rev 2)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1976, flyball1, flyball, flyball, flyball, driver_device, 0, 0, "Atari", "Flyball (rev 1)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
