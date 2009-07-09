/***************************************************************************

Atari Destroyer Driver

***************************************************************************/

#include "driver.h"
#include "cpu/m6800/m6800.h"
#include "deprecat.h"

extern VIDEO_UPDATE( destroyr );

extern int destroyr_wavemod;
extern int destroyr_cursor;

extern UINT8* destroyr_major_obj_ram;
extern UINT8* destroyr_minor_obj_ram;
extern UINT8* destroyr_alpha_num_ram;

static int destroyr_potmask[2];
static int destroyr_potsense[2];
static int destroyr_attract;
static int destroyr_motor_speed;
static int destroyr_noise;


static TIMER_CALLBACK( destroyr_dial_callback	)
{
	int dial = param;

	/* Analog inputs come from the player's depth control potentiometer.
       The voltage is compared to a voltage ramp provided by a discrete
       analog circuit that conditions the VBLANK signal. When the ramp
       voltage exceeds the input voltage an NMI signal is generated. The
       computer then reads the VSYNC data functions to tell where the
       cursor should be located. */

	destroyr_potsense[dial] = 1;

	if (destroyr_potmask[dial])
	{
		cputag_set_input_line(machine, "maincpu", INPUT_LINE_NMI, PULSE_LINE);
	}
}


static TIMER_CALLBACK( destroyr_frame_callback )
{
	destroyr_potsense[0] = 0;
	destroyr_potsense[1] = 0;

	/* PCB supports two dials, but cab has only got one */

	timer_set(machine, video_screen_get_time_until_pos(machine->primary_screen, input_port_read(machine, "PADDLE"), 0), NULL, 0, destroyr_dial_callback);
	timer_set(machine, video_screen_get_time_until_pos(machine->primary_screen, 0, 0), NULL, 0, destroyr_frame_callback);
}


static MACHINE_RESET( destroyr )
{
	timer_set(machine, video_screen_get_time_until_pos(machine->primary_screen, 0, 0), NULL, 0, destroyr_frame_callback);
}


static WRITE8_HANDLER( destroyr_misc_w )
{
	/* bits 0 to 2 connect to the sound circuits */

	destroyr_attract = data & 1;
	destroyr_noise = data & 2;
	destroyr_motor_speed = data & 4;
	destroyr_potmask[0] = data & 8;
	destroyr_wavemod = data & 16;
	destroyr_potmask[1] = data & 32;

	coin_lockout_w(0, !destroyr_attract);
	coin_lockout_w(1, !destroyr_attract);
}


static WRITE8_HANDLER( destroyr_cursor_load_w )
{
	destroyr_cursor = data;

	watchdog_reset_w(space, offset, data);
}


static WRITE8_HANDLER( destroyr_interrupt_ack_w )
{
	cputag_set_input_line(space->machine, "maincpu", 0, CLEAR_LINE);
}


static WRITE8_HANDLER( destroyr_output_w )
{
	offset &= 15;

	switch (offset)
	{
	case 0:
		set_led_status(0, data & 1);
		break;
	case 1:
		set_led_status(1, data & 1); /* no second LED present on cab */
		break;
	case 2:
		/* bit 0 => songate */
		break;
	case 3:
		/* bit 0 => launch */
		break;
	case 4:
		/* bit 0 => explosion */
		break;
	case 5:
		/* bit 0 => sonar */
		break;
	case 6:
		/* bit 0 => high explosion */
		break;
	case 7:
		/* bit 0 => low explosion */
		break;
	case 8:
		destroyr_misc_w(space, offset, data);
		break;
	default:
		logerror("unmapped output port %d\n", offset);
		break;
	}
}


static READ8_HANDLER( destroyr_input_r )
{
	offset &= 15;

	if (offset == 0)
	{
		UINT8 ret = input_port_read(space->machine, "IN0");

		if (destroyr_potsense[0] && destroyr_potmask[0])
			ret |= 4;
		if (destroyr_potsense[1] && destroyr_potmask[1])
			ret |= 8;

		return ret;
	}

	if (offset == 1)
	{
		return input_port_read(space->machine, "IN1");
	}

	logerror("unmapped input port %d\n", offset);

	return 0;
}


static READ8_HANDLER( destroyr_scanline_r )
{
	return video_screen_get_vpos(space->machine->primary_screen);
}


static ADDRESS_MAP_START( destroyr_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x00ff) AM_MIRROR(0xf00) AM_RAM
	AM_RANGE(0x1000, 0x1fff) AM_READWRITE(destroyr_input_r, destroyr_output_w)
	AM_RANGE(0x2000, 0x2fff) AM_READ_PORT("IN2")
	AM_RANGE(0x3000, 0x30ff) AM_WRITE(SMH_RAM) AM_BASE(&destroyr_alpha_num_ram)
	AM_RANGE(0x4000, 0x401f) AM_WRITE(SMH_RAM) AM_BASE(&destroyr_major_obj_ram)
	AM_RANGE(0x5000, 0x5000) AM_WRITE(destroyr_cursor_load_w)
	AM_RANGE(0x5001, 0x5001) AM_WRITE(destroyr_interrupt_ack_w)
	AM_RANGE(0x5002, 0x5007) AM_WRITE(SMH_RAM) AM_BASE(&destroyr_minor_obj_ram)
	AM_RANGE(0x6000, 0x6fff) AM_READ(destroyr_scanline_r)
	AM_RANGE(0x7000, 0x77ff) AM_NOP				/* missing translation ROMs */
	AM_RANGE(0x7800, 0x7fff) AM_ROM				/* program */
ADDRESS_MAP_END


static INPUT_PORTS_START( destroyr )
	PORT_START("IN0")	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNUSED ) /* call 7400 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED ) /* potsense1 */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED ) /* potsense2 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_DIPNAME( 0xc0, 0x80, "Extended Play" ) PORT_DIPLOCATION("SW2:8,7")
	PORT_DIPSETTING( 0x40, "1500 points" )
	PORT_DIPSETTING( 0x80, "2500 points" )
	PORT_DIPSETTING( 0xc0, "3500 points" )
	PORT_DIPSETTING( 0x00, "never" )

	PORT_START("IN1")	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) /* actually a lever */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )

	PORT_START("IN2")	/* IN2 */
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:4,3")
	PORT_DIPSETTING( 0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING( 0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING( 0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x08, "Play Time" ) PORT_DIPLOCATION("SW:2,1")
	PORT_DIPSETTING( 0x00, "50 seconds" )
	PORT_DIPSETTING( 0x04, "75 seconds" )
	PORT_DIPSETTING( 0x08, "100 seconds" )
	PORT_DIPSETTING( 0x0c, "125 seconds" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("SW2:5,6") /* requires translation ROMs */
	PORT_DIPSETTING( 0x30, DEF_STR( German ) )
	PORT_DIPSETTING( 0x20, DEF_STR( French ) )
	PORT_DIPSETTING( 0x10, DEF_STR( Spanish ) )
	PORT_DIPSETTING( 0x00, DEF_STR( English ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PADDLE")	/* IN3 */
	PORT_BIT( 0xff, 0x00, IPT_PADDLE_V ) PORT_MINMAX(0,160) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_REVERSE
INPUT_PORTS_END


static const gfx_layout destroyr_alpha_num_layout =
{
	8, 8,     /* width, height */
	64,       /* total         */
	1,        /* planes        */
	{ 0 },    /* plane offsets */
	{
		0x4, 0x5, 0x6, 0x7, 0xC, 0xD, 0xE, 0xF
	},
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70
	},
	0x80      /* increment */
};


static const gfx_layout destroyr_minor_object_layout =
{
	16, 16,   /* width, height */
	16,       /* total         */
	1,        /* planes        */
	{ 0 },    /* plane offsets */
	{
	  0x04, 0x05, 0x06, 0x07, 0x0C, 0x0D, 0x0E, 0x0F,
	  0x14, 0x15, 0x16, 0x17, 0x1C, 0x1D, 0x1E, 0x1F
	},
	{
	  0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0,
	  0x100, 0x120, 0x140, 0x160, 0x180, 0x1a0, 0x1c0, 0x1e0
	},
	0x200     /* increment */
};

static const UINT32 destroyr_major_object_layout_xoffset[64] =
{
	0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E,
	0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E,
	0x20, 0x22, 0x24, 0x26, 0x28, 0x2A, 0x2C, 0x2E,
	0x30, 0x32, 0x34, 0x36, 0x38, 0x3A, 0x3C, 0x3E,
	0x40, 0x42, 0x44, 0x46, 0x48, 0x4A, 0x4C, 0x4E,
	0x50, 0x52, 0x54, 0x56, 0x58, 0x5A, 0x5C, 0x5E,
	0x60, 0x62, 0x64, 0x66, 0x68, 0x6A, 0x6C, 0x6E,
	0x70, 0x72, 0x74, 0x76, 0x78, 0x7A, 0x7C, 0x7E
};

static const gfx_layout destroyr_major_object_layout =
{
	64, 16,   /* width, height */
	4,        /* total         */
	2,        /* planes        */
	{ 1, 0 },  /* plane offsets */
	EXTENDED_XOFFS,
	{
		0x000, 0x080, 0x100, 0x180, 0x200, 0x280, 0x300, 0x380,
		0x400, 0x480, 0x500, 0x580, 0x600, 0x680, 0x700, 0x780
	},
	0x0800,   /* increment */
	destroyr_major_object_layout_xoffset,
	NULL
};

static const UINT32 destroyr_waves_layout_xoffset[64] =
{
	0x00, 0x01, 0x02, 0x03, 0x08, 0x09, 0x0A, 0x0B,
	0x10, 0x11, 0x12, 0x13, 0x18, 0x19, 0x1A, 0x1B,
	0x20, 0x21, 0x22, 0x23, 0x28, 0x29, 0x2A, 0x2B,
	0x30, 0x31, 0x32, 0x33, 0x38, 0x39, 0x3A, 0x3B,
	0x40, 0x41, 0x42, 0x43, 0x48, 0x49, 0x4A, 0x4B,
	0x50, 0x51, 0x52, 0x53, 0x58, 0x59, 0x5A, 0x5B,
	0x60, 0x61, 0x62, 0x63, 0x68, 0x69, 0x6A, 0x6B,
	0x70, 0x71, 0x72, 0x73, 0x78, 0x79, 0x7A, 0x7B
};

static const gfx_layout destroyr_waves_layout =
{
	64, 2,    /* width, height */
	2,        /* total         */
	1,        /* planes        */
	{ 0 },
	EXTENDED_XOFFS,
	{ 0x00, 0x80 },
	0x04,     /* increment */
	destroyr_waves_layout_xoffset,
	NULL
};


static GFXDECODE_START( destroyr )
	GFXDECODE_ENTRY( "gfx1", 0, destroyr_alpha_num_layout, 4, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, destroyr_minor_object_layout, 4, 1 )
	GFXDECODE_ENTRY( "gfx3", 0, destroyr_major_object_layout, 0, 1 )
	GFXDECODE_ENTRY( "gfx4", 0, destroyr_waves_layout, 4, 1 )
GFXDECODE_END


static PALETTE_INIT( destroyr )
{
	palette_set_color(machine, 0, MAKE_RGB(0x00, 0x00, 0x00));   /* major objects */
	palette_set_color(machine, 1, MAKE_RGB(0x50, 0x50, 0x50));
	palette_set_color(machine, 2, MAKE_RGB(0xAF, 0xAF, 0xAF));
	palette_set_color(machine, 3, MAKE_RGB(0xFF ,0xFF, 0xFF));
	palette_set_color(machine, 4, MAKE_RGB(0x00, 0x00, 0x00));   /* alpha numerics, waves, minor objects */
	palette_set_color(machine, 5, MAKE_RGB(0xFF, 0xFF, 0xFF));
	palette_set_color(machine, 6, MAKE_RGB(0x00, 0x00, 0x00));   /* cursor */
	palette_set_color(machine, 7, MAKE_RGB(0x78, 0x78, 0x78));
}


static MACHINE_DRIVER_START( destroyr )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6800, 12096000 / 16)
	MDRV_CPU_PROGRAM_MAP(destroyr_map)
	MDRV_CPU_VBLANK_INT_HACK(irq0_line_assert, 4)

	MDRV_MACHINE_RESET(destroyr)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 262)
	MDRV_SCREEN_VISIBLE_AREA(0, 255, 0, 239)

	MDRV_GFXDECODE(destroyr)
	MDRV_PALETTE_LENGTH(8)
	MDRV_PALETTE_INIT(destroyr)
	MDRV_VIDEO_UPDATE(destroyr)

	/* sound hardware */
MACHINE_DRIVER_END


ROM_START( destroyr )
	ROM_REGION( 0x8000, "maincpu", 0 )                  /* program code */
	ROM_LOAD( "30146-01.c3", 0x7800, 0x0800, CRC(e560c712) SHA1(0505ab57eee5421b4ff4e87d14505e02b18fd54c) )

	ROM_REGION( 0x0400, "gfx1", 0 )   /* alpha numerics */
	ROM_LOAD( "30135-01.p4", 0x0000, 0x0400, CRC(184824cf) SHA1(713cfd1d41ef7b1c345ea0038b652c4ba3f08301) )

	ROM_REGION( 0x0400, "gfx2", 0 )   /* minor objects */
	ROM_LOAD( "30132-01.f4", 0x0000, 0x0400, CRC(e09d3d55) SHA1(b26013397ef2cb32d0416ecb118387b9c2dffa9a) )

	ROM_REGION( 0x0400, "gfx3", 0 )   /* major objects */
	ROM_LOAD_NIB_HIGH( "30134-01.p8", 0x0000, 0x0400, CRC(6259e007) SHA1(049f5f7160305cb4f4b499dd113cb11eea73fc95) )
	ROM_LOAD_NIB_LOW ( "30133-01.n8", 0x0000, 0x0400, CRC(108d3e2c) SHA1(8c993369d37c6713670483af78e6d04d38f4b4fc) )

	ROM_REGION( 0x0020, "gfx4", 0 )   /* waves */
	ROM_LOAD( "30136-01.k2", 0x0000, 0x0020, CRC(532c11b1) SHA1(18ab5369a3f2cfcc9a44f38fa8649524bea5b203) )

	ROM_REGION( 0x0100, "user1", 0 )                  /* sync (unused) */
	ROM_LOAD( "30131-01.m1", 0x0000, 0x0100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) )
ROM_END


GAME( 1977, destroyr, 0, destroyr, destroyr, 0, ORIENTATION_FLIP_X, "Atari", "Destroyer", GAME_NO_SOUND )
