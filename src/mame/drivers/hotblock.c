/* Hot Blocks */
/*
driver by David Haywood
*/

/*
HotBlock board

Tetris with naughty bits

        ||||||||||||||||
+-------++++++++++++++++-------+
|                              |
|  YM2149 TESTSW               |
|                              |
|    62256 62256   6116 6116   |
|                              |
|    24mhz  TPC1020AFN 24c04a  |
|                              |
|                     PAL      |
| P8088-1 IC4 IC5 62256 62256  |
|                              |
+------------------------------+

330ohm resistor packs for colours


--

there are a variety of test modes which can be obtained
by resetting while holding down player 2 buttons

eeprom / backup data not hooked up ( 24c04a on port4 )

most sources say this is a game by Nics but I believe Nics
to be a company from Korea, this game is quite clearly a
Spanish game, we know for a fact that NIX are from Spain
so it could be by them instead



*/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "sound/ay8910.h"

class hotblock_state : public driver_device
{
public:
	hotblock_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  vram;
	UINT8 *  pal;

	/* misc */
	int      port0;
	int      port4;
};



static READ8_HANDLER( hotblock_video_read )
{
	hotblock_state *state = space->machine->driver_data<hotblock_state>();
	/* right?, anything else?? */
	if (state->port0 & 0x20) // port 0 = a8 e8 -- palette
	{
		return state->pal[offset];
	}
	else // port 0 = 88 c8
	{
		return state->vram[offset];
	}
}

/* port 4 is some kind of eeprom / storage .. used to store the scores */
static READ8_HANDLER( hotblock_port4_r )
{
//  mame_printf_debug("port4_r\n");
	return 0x00;
}


static WRITE8_HANDLER( hotblock_port4_w )
{
//  mame_printf_debug("port4_w: pc = %06x : data %04x\n", cpu_get_pc(space->cpu), data);
//  popmessage("port4_w: pc = %06x : data %04x", cpu_get_pc(space->cpu), data);
	hotblock_state *state = space->machine->driver_data<hotblock_state>();
	state->port4 = data;
}



static WRITE8_HANDLER( hotblock_port0_w )
{
//  popmessage("port4_w: pc = %06x : data %04x", cpu_get_pc(space->cpu), data);
	hotblock_state *state = space->machine->driver_data<hotblock_state>();
	state->port0 = data;
}

static WRITE8_HANDLER( hotblock_video_write )
{
	hotblock_state *state = space->machine->driver_data<hotblock_state>();
	/* right?, anything else?? */
	if (state->port0 & 0x20) // port 0 = a8 e8 -- palette
	{
		state->pal[offset] = data;
	}
	else // port 0 = 88 c8
	{
		state->vram[offset] = data;
	}
}

static ADDRESS_MAP_START( hotblock_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x0ffff) AM_RAM
	AM_RANGE(0x10000, 0x1ffff) AM_READWRITE(hotblock_video_read, hotblock_video_write) AM_BASE_MEMBER(hotblock_state, vram)
	AM_RANGE(0x20000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( hotblock_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x0000, 0x0000) AM_WRITE(hotblock_port0_w)
	AM_RANGE(0x0004, 0x0004) AM_READWRITE(hotblock_port4_r, hotblock_port4_w)
	AM_RANGE(0x8000, 0x8001) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	AM_RANGE(0x8001, 0x8001) AM_DEVREAD("aysnd", ay8910_r)
ADDRESS_MAP_END



static VIDEO_START(hotblock)
{
	hotblock_state *state = machine->driver_data<hotblock_state>();
	state->pal = auto_alloc_array(machine, UINT8, 0x10000);
	state_save_register_global_pointer(machine, state->pal, 0x10000);
}

static VIDEO_UPDATE(hotblock)
{
	hotblock_state *state = screen->machine->driver_data<hotblock_state>();
	int y, x, count;
	int i;
	static const int xxx = 320, yyy = 204;

	bitmap_fill(bitmap, 0, get_black_pen(screen->machine));

	for (i = 0; i < 256; i++)
	{
		int dat = (state->pal[i * 2 + 1] << 8) | state->pal[i * 2];
		palette_set_color_rgb(screen->machine, i, pal5bit(dat >> 0), pal5bit(dat >> 5), pal5bit(dat >> 10));
	}

	count = 0;
	for (y = 0; y < yyy; y++)
	{
		for(x = 0; x < xxx; x++)
		{
			if (state->port0 & 0x40)
				*BITMAP_ADDR16(bitmap, y, x) = state->vram[count];
			count++;
		}
	}

	return 0;
}


static INPUT_PORTS_START( hotblock )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) // unused?

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) // used to get test mode
INPUT_PORTS_END


static INTERRUPT_GEN( hotblocks_irq ) /* right? */
{
	cpu_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);
}

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("P1"),
	DEVCB_INPUT_PORT("P2"),
	DEVCB_NULL,
	DEVCB_NULL
};


static MACHINE_CONFIG_START( hotblock, hotblock_state )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", I8088, 10000000)
	MDRV_CPU_PROGRAM_MAP(hotblock_map)
	MDRV_CPU_IO_MAP(hotblock_io)
	MDRV_CPU_VBLANK_INT("screen", hotblocks_irq)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(1024,1024)
	MDRV_SCREEN_VISIBLE_AREA(0, 320-1, 0, 200-1)

	MDRV_PALETTE_LENGTH(256)

	MDRV_VIDEO_START(hotblock)
	MDRV_VIDEO_UPDATE(hotblock)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 1000000)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

ROM_START( hotblock )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "hotblk5.ic4", 0x000000, 0x080000, CRC(5f90f776) SHA1(5ca74714a7d264b4fafaad07dc11e57308828d30) )
	ROM_LOAD( "hotblk6.ic5", 0x080000, 0x080000, CRC(3176d231) SHA1(ac22fd0e9820c6714f51a3d8315eb5d43ef91eeb) )
ROM_END

GAME( 1993, hotblock, 0,        hotblock, hotblock, 0, ROT0,  "NIX?", "Hot Blocks - Tetrix II", 0 )
