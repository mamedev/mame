/***************************************************************************

Atari Flyball Driver

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"

#define MASTER_CLOCK ( XTAL_12_096MHz )



class flyball_state : public driver_device
{
public:
	flyball_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  rombase;
	UINT8 *  playfield_ram;

	/* video-related */
	tilemap_t  *tmap;
	UINT8    pitcher_vert;
	UINT8    pitcher_horz;
	UINT8    pitcher_pic;
	UINT8    ball_vert;
	UINT8    ball_horz;

	/* misc */
	UINT8    potmask;
	UINT8    potsense;

	/* devices */
	device_t *maincpu;
};


/*************************************
 *
 *  Video emulation
 *
 *************************************/

static TILEMAP_MAPPER( flyball_get_memory_offset )
{
	if (col == 0)
		col = num_cols;

	return num_cols * (num_rows - row) - col;
}


static TILE_GET_INFO( flyball_get_tile_info )
{
	flyball_state *state = machine->driver_data<flyball_state>();
	UINT8 data = state->playfield_ram[tile_index];
	int flags = ((data & 0x40) ? TILE_FLIPX : 0) | ((data & 0x80) ? TILE_FLIPY : 0);
	int code = data & 63;

	if ((flags & TILE_FLIPX) && (flags & TILE_FLIPY))
	{
		code += 64;
	}

	SET_TILE_INFO(0, code, 0, flags);
}


static VIDEO_START( flyball )
{
	flyball_state *state = machine->driver_data<flyball_state>();
	state->tmap = tilemap_create(machine, flyball_get_tile_info, flyball_get_memory_offset, 8, 16, 32, 16);
}


static VIDEO_UPDATE( flyball )
{
	flyball_state *state = screen->machine->driver_data<flyball_state>();
	int pitcherx = state->pitcher_horz;
	int pitchery = state->pitcher_vert - 31;

	int ballx = state->ball_horz - 1;
	int bally = state->ball_vert - 17;

	int x;
	int y;

	tilemap_mark_all_tiles_dirty(state->tmap);

	/* draw playfield */
	tilemap_draw(bitmap, cliprect, state->tmap, 0, 0);

	/* draw pitcher */
	drawgfx_transpen(bitmap, cliprect, screen->machine->gfx[1], state->pitcher_pic ^ 0xf, 0, 1, 0, pitcherx, pitchery, 1);

	/* draw ball */

	for (y = bally; y < bally + 2; y++)
		for (x = ballx; x < ballx + 2; x++)
			if (x >= cliprect->min_x &&
			    x <= cliprect->max_x &&
			    y >= cliprect->min_y &&
			    y <= cliprect->max_y)
				*BITMAP_ADDR16(bitmap, y, x) = 1;
	return 0;
}


static TIMER_CALLBACK( flyball_joystick_callback )
{
	flyball_state *state = machine->driver_data<flyball_state>();
	int potsense = param;

	if (potsense & ~state->potmask)
		generic_pulse_irq_line(state->maincpu, 0);

	state->potsense |= potsense;
}


static TIMER_CALLBACK( flyball_quarter_callback	)
{
	flyball_state *state = machine->driver_data<flyball_state>();
	int scanline = param;
	int potsense[64], i;

	memset(potsense, 0, sizeof potsense);

	potsense[input_port_read(machine, "STICK1_Y")] |= 1;
	potsense[input_port_read(machine, "STICK1_X")] |= 2;
	potsense[input_port_read(machine, "STICK0_Y")] |= 4;
	potsense[input_port_read(machine, "STICK0_X")] |= 8;

	for (i = 0; i < 64; i++)
		if (potsense[i] != 0)
			timer_set(machine, machine->primary_screen->time_until_pos(scanline + i), NULL, potsense[i], flyball_joystick_callback);

	scanline += 0x40;
	scanline &= 0xff;

	timer_set(machine, machine->primary_screen->time_until_pos(scanline), NULL, scanline, flyball_quarter_callback);

	state->potsense = 0;
	state->potmask = 0;
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

/* two physical buttons (start game and stop runner) share the same port bit */
static READ8_HANDLER( flyball_input_r )
{
	return input_port_read(space->machine, "IN0") & input_port_read(space->machine, "IN1");
}

static READ8_HANDLER( flyball_scanline_r )
{
	return space->machine->primary_screen->vpos() & 0x3f;
}

static READ8_HANDLER( flyball_potsense_r )
{
	flyball_state *state = space->machine->driver_data<flyball_state>();
	return state->potsense & ~state->potmask;
}

static WRITE8_HANDLER( flyball_potmask_w )
{
	flyball_state *state = space->machine->driver_data<flyball_state>();
	state->potmask |= data & 0xf;
}

static WRITE8_HANDLER( flyball_pitcher_pic_w )
{
	flyball_state *state = space->machine->driver_data<flyball_state>();
	state->pitcher_pic = data & 0xf;
}

static WRITE8_HANDLER( flyball_ball_vert_w )
{
	flyball_state *state = space->machine->driver_data<flyball_state>();
	state->ball_vert = data;
}

static WRITE8_HANDLER( flyball_ball_horz_w )
{
	flyball_state *state = space->machine->driver_data<flyball_state>();
	state->ball_horz = data;
}

static WRITE8_HANDLER( flyball_pitcher_vert_w )
{
	flyball_state *state = space->machine->driver_data<flyball_state>();
	state->pitcher_vert = data;
}

static WRITE8_HANDLER( flyball_pitcher_horz_w )
{
	flyball_state *state = space->machine->driver_data<flyball_state>();
	state->pitcher_horz = data;
}

static WRITE8_HANDLER( flyball_misc_w )
{
	int bit = ~data & 1;

	switch (offset)
	{
	case 0:
		set_led_status(space->machine, 0, bit);
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

static ADDRESS_MAP_START( flyball_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x1fff)
	AM_RANGE(0x0000, 0x00ff) AM_MIRROR(0x100) AM_RAM
	AM_RANGE(0x0800, 0x0800) AM_NOP
	AM_RANGE(0x0801, 0x0801) AM_WRITE(flyball_pitcher_pic_w)
	AM_RANGE(0x0802, 0x0802) AM_READ(flyball_scanline_r)
	AM_RANGE(0x0803, 0x0803) AM_READ(flyball_potsense_r)
	AM_RANGE(0x0804, 0x0804) AM_WRITE(flyball_ball_vert_w)
	AM_RANGE(0x0805, 0x0805) AM_WRITE(flyball_ball_horz_w)
	AM_RANGE(0x0806, 0x0806) AM_WRITE(flyball_pitcher_vert_w)
	AM_RANGE(0x0807, 0x0807) AM_WRITE(flyball_pitcher_horz_w)
	AM_RANGE(0x0900, 0x0900) AM_WRITE(flyball_potmask_w)
	AM_RANGE(0x0a00, 0x0a07) AM_WRITE(flyball_misc_w)
	AM_RANGE(0x0b00, 0x0b00) AM_READ(flyball_input_r)
	AM_RANGE(0x0d00, 0x0eff) AM_WRITEONLY AM_BASE_MEMBER(flyball_state, playfield_ram)
	AM_RANGE(0x1000, 0x1fff) AM_ROM AM_BASE_MEMBER(flyball_state, rombase) /* program */
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


static PALETTE_INIT( flyball )
{
	palette_set_color(machine, 0, MAKE_RGB(0x3F, 0x3F, 0x3F));  /* tiles, ball */
	palette_set_color(machine, 1, MAKE_RGB(0xFF, 0xFF, 0xFF));
	palette_set_color(machine, 2, MAKE_RGB(0xFF ,0xFF, 0xFF));  /* sprites */
	palette_set_color(machine, 3, MAKE_RGB(0x00, 0x00, 0x00));
}


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_START( flyball )
{
	flyball_state *state = machine->driver_data<flyball_state>();

	state->maincpu = machine->device("maincpu");

	state_save_register_global(machine, state->pitcher_vert);
	state_save_register_global(machine, state->pitcher_horz);
	state_save_register_global(machine, state->pitcher_pic);
	state_save_register_global(machine, state->ball_vert);
	state_save_register_global(machine, state->ball_horz);
	state_save_register_global(machine, state->potmask);
	state_save_register_global(machine, state->potsense);
}

static MACHINE_RESET( flyball )
{
	flyball_state *state = machine->driver_data<flyball_state>();
	int i;

	/* address bits 0 through 8 are inverted */
	UINT8* ROM = machine->region("maincpu")->base() + 0x2000;

	for (i = 0; i < 0x1000; i++)
		state->rombase[i] = ROM[i ^ 0x1ff];

	machine->device("maincpu")->reset();

	timer_set(machine, machine->primary_screen->time_until_pos(0), NULL, 0, flyball_quarter_callback);

	state->pitcher_vert = 0;
	state->pitcher_horz = 0;
	state->pitcher_pic = 0;
	state->ball_vert = 0;
	state->ball_horz = 0;
	state->potmask = 0;
	state->potsense = 0;
}


static MACHINE_CONFIG_START( flyball, flyball_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, MASTER_CLOCK/16)
	MCFG_CPU_PROGRAM_MAP(flyball_map)
	MCFG_CPU_VBLANK_INT("screen", nmi_line_pulse)

	MCFG_MACHINE_START(flyball)
	MCFG_MACHINE_RESET(flyball)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(256, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 255, 0, 239)

	MCFG_GFXDECODE(flyball)
	MCFG_PALETTE_LENGTH(4)

	MCFG_PALETTE_INIT(flyball)
	MCFG_VIDEO_UPDATE(flyball)
	MCFG_VIDEO_START(flyball)

	/* sound hardware */
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( flyball )
	ROM_REGION( 0x3000, "maincpu", 0 )                  /* program */
	ROM_LOAD( "6129.d5", 0x2000, 0x0200, CRC(17eda069) SHA1(e4ef0bf4546cf00668d759a188e0989a4f003825) )
	ROM_LOAD( "6130.f5", 0x2200, 0x0200, CRC(a756955b) SHA1(220b7f1789bba4481d595b36b4bae25f98d3ad8d) )
	ROM_LOAD( "6131.h5", 0x2400, 0x0200, CRC(a9c7e858) SHA1(aee4a359d6a5729dc1be5b8ce8fbe54d032d12b0) )
	ROM_LOAD( "6132.j5", 0x2600, 0x0200, CRC(31fefd8a) SHA1(97e3ef278ce2175cd33c0f3147bdf7974752c836) )
	ROM_LOAD( "6133.k5", 0x2800, 0x0200, CRC(6fdb09b1) SHA1(04ad412b437bb24739b3e31fa5a413e63d5897f8) )
	ROM_LOAD( "6134.m5", 0x2A00, 0x0200, CRC(7b526c73) SHA1(e47c8f33b7edc143ab1713556c59b93571933daa) )
	ROM_LOAD( "6135.n5", 0x2C00, 0x0200, CRC(b352cb51) SHA1(39b9062fb51d0a78a47dcd470ceae47fcdbd7891) )
	ROM_LOAD( "6136.r5", 0x2E00, 0x0200, CRC(1622d890) SHA1(9ad342aefdc02e022eb79d84d1c856bed538bebe) )

	ROM_REGION( 0x0C00, "gfx1", 0 )   /* tiles */
	ROM_LOAD( "6142.l2", 0x0000, 0x0200, CRC(65650cfa) SHA1(7d17455146fc9def22c7bd06f7fde32df0a0c2bc) )
	ROM_LOAD( "6139.j2", 0x0200, 0x0200, CRC(a5d1358e) SHA1(33cecbe40ae299549a3395e3dffbe7b6021803ba) )
	ROM_LOAD( "6141.m2", 0x0400, 0x0200, CRC(98b5f803) SHA1(c4e323ced2393fa4a9720ff0086c559fb9b3a9f8) )
	ROM_LOAD( "6140.k2", 0x0600, 0x0200, CRC(66aeec61) SHA1(f577bad015fe9e3708fd95d5d2bc438997d14d2c) )

	ROM_REGION( 0x0400, "gfx2", 0 )   /* sprites */
	ROM_LOAD16_BYTE( "6137.e2", 0x0000, 0x0200, CRC(68961fda) SHA1(a06c7b453cce04716f49bd65ecfe1ba67cb8681e) )
	ROM_LOAD16_BYTE( "6138.f2", 0x0001, 0x0200, CRC(aab314f6) SHA1(6625c719fdc000d6af94bc9474de8f7e977cee97) )
ROM_END


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1976, flyball, 0, flyball, flyball, 0, 0, "Atari", "Flyball", GAME_NO_SOUND )
