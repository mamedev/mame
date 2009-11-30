/*
 Two Minute Drill - Taito 1993
 -----------------------------
 Half Video, Half Mechanical?
(video hw + motion/acceleration sensor ?)

 preliminary driver by
  David Haywood
  Tomasz Slanina

TODO:
 - understand the ball hit sensor
 - simulate the sensors (there are still some shutter errors/defender errors that pops up)
 - Hook-up timers for shutter/defender sensors (check service mode)
 - Dip-Switches
 - find video control regs (layer enable, scroll)

BG scroll:
BG maps are 2048x256 (128x16 16x16 tiles).
There's some kind of double buffering - odd/even screens are at
x offsets 0 and 512 (it's visible during distance count after throw (odd/even numbers))

looks like regs @460000 are used,  pairs at N, and N+8, so
460000, 460008
460002, 46000a
460004, 46000c
460006, 46000e

*/

/*

TWO MINUTE DRILL - Taito 1993?

No idea what this game is... I do not have the pinout
See pic for more details

 Brief hardware overview:
 ------------------------

 Main processor   - 68000 16Mhz

 Sound            - Yamaha YM2610B

 Taito custom ICs - TC0400YSC
                  - TC0260DAR
                  - TC0630FDP
                  - TC0510NI0

DAC               -26.6860Mhz
                        -32.0000Mhz

*/

#include "driver.h"
#include "cpu/m68000/m68000.h"
#include "sound/2610intf.h"


typedef struct __2mindril_state _2mindril_state;
struct __2mindril_state
{
	/* memory pointers */
	UINT16 *      map1ram;
	UINT16 *      map2ram;
	UINT16 *      map3ram;
	UINT16 *      map4ram;
	UINT16 *      charram;
	UINT16 *      textram;
	UINT16 *      unkram;
//  UINT16 *      paletteram;   // currently this uses generic palette handling
	UINT16 *      iodata;

	/* input-related */
	UINT16        defender_sensor, shutter_sensor;

	/* devices */
	const device_config *maincpu;
};



#define DRAW_MAP(map,num) 	{ 	int x, y; \
			  	for (y = 0; y < 16; y++) \
	 				for (x = 0; x < 128; x++) \
	 				{ \
	 					UINT16 data0 = map[y * 128 + x * 2]; \
	 					UINT16 data1 = map[y * 128 + x * 2 + 1]; \
	 					drawgfx_transpen(bitmap, \
							cliprect,screen->machine->gfx[0], data1, \
		 					data0 & 0xff, \
							data0 & 0x4000, data0 & 0x8000, \
							x * 16 - 512 /*+(((INT16)(state->unkram[0x60000 / 2 + num])) / 32)*/, \
							y * 16 /*+(((INT16)(state->unkram[0x60008 / 2 + num])) / 32)*/,0); \
	 				}	\
			}

static VIDEO_UPDATE( drill )
{
	_2mindril_state *state = (_2mindril_state *)screen->machine->driver_data;
	bitmap_fill(bitmap, NULL, 0);

	DRAW_MAP(state->map1ram, 0)
	DRAW_MAP(state->map2ram, 1)
	DRAW_MAP(state->map3ram, 2)
	DRAW_MAP(state->map4ram, 3)

	{
		int x, y;
		for (y = 0; y < 64; y++)
	 		for(x = 0; x < 64; x++)
	 		{
	 			drawgfx_transpen(	bitmap,
						cliprect,
						screen->machine->gfx[1],
						state->textram[y * 64 + x] & 0xff, //1ff ??
						((state->textram[y * 64 + x] >> 9) & 0xf),
						0, 0,
						x*8,y*8,0);
	 		}
	}
	/*printf("%.4X %.4X %.4X %.4X %.4X %.4X\n", state->unkram[0x60000 / 2], state->unkram[0x60000 / 2 + 1], state->unkram[0x60000 / 2 + 2],
                                    state->unkram[0x60000 / 2 + 3], state->unkram[0x60000 / 2 + 4], state->unkram[0x60000 / 2 + 5]);*/
	return 0;
}

static VIDEO_START( drill )
{
	_2mindril_state *state = (_2mindril_state *)machine->driver_data;

	machine->gfx[0]->color_granularity = 16;
	gfx_element_set_source(machine->gfx[1], (UINT8 *)state->charram);
}

static READ16_HANDLER( drill_io_r )
{
	_2mindril_state *state = (_2mindril_state *)space->machine->driver_data;

//  if (offset * 2 == 0x4)
	/*popmessage("PC=%08x %04x %04x %04x %04x %04x %04x %04x %04x", cpu_get_pc(space->cpu), state->iodata[0/2], state->iodata[2/2], state->iodata[4/2], state->iodata[6/2],
                                        state->iodata[8/2], state->iodata[0xa/2], state->iodata[0xc/2], state->iodata[0xe/2]);*/

	switch(offset)
	{
		case 0x0/2: return input_port_read(space->machine, "DSW");
		case 0x2/2:
		{
			int arm_pwr = input_port_read(space->machine, "IN0");//throw
			//popmessage("PC=%08x %02x",cpu_get_pc(space->cpu),arm_pwr);

			if(arm_pwr > 0xe0) return ~0x1800;
			if(arm_pwr > 0xc0) return ~0x1400;
			if(arm_pwr > 0x80) return ~0x1200;
			if(arm_pwr > 0x40) return ~0x1000;
			else return ~0x0000;
		}
		case 0x4/2: return (state->defender_sensor) | (state->shutter_sensor);
		case 0xe/2: return input_port_read(space->machine, "IN2");//coins
//      default:  printf("PC=%08x [%04x] -> %04x R\n", cpu_get_pc(space->cpu), offset * 2, state->iodata[offset]);
	}

	return 0xffff;
}

static WRITE16_HANDLER( drill_io_w )
{
	_2mindril_state *state = (_2mindril_state *)space->machine->driver_data;
	COMBINE_DATA(&state->iodata[offset]);

	switch(offset)
	{
		case 0x8/2:
			coin_counter_w(space->machine, 0, state->iodata[offset] & 0x0400);
			coin_counter_w(space->machine, 1, state->iodata[offset] & 0x0800);
			coin_lockout_w(space->machine, 0, ~state->iodata[offset] & 0x0100);
			coin_lockout_w(space->machine, 1, ~state->iodata[offset] & 0x0200);
			break;
	}

//  if(data != 0 && offset != 8)
//  printf("PC=%08x [%04x] <- %04x W\n", cpu_get_pc(space->cpu), offset * 2, data);
}

/*
    PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Unknown ) )//up sensor <- shutter
    PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
    PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) )//down sensor
    PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
    PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) )//left sensor <-defender
    PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
    PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) )//right sensor
    PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
*/
#ifdef UNUSED_FUNCTION
static TIMER_CALLBACK( shutter_req )
{
	_2mindril_state *state = (_2mindril_state *)machine->driver_data;
	state->shutter_sensor = param;
}

static TIMER_CALLBACK( defender_req )
{
	_2mindril_state *state = (_2mindril_state *)machine->driver_data;
	state->defender_sensor = param;
}
#endif

static WRITE16_HANDLER( sensors_w )
{
	_2mindril_state *state = (_2mindril_state *)space->machine->driver_data;

	/*---- xxxx ---- ---- select "lamps" (guess)*/
	/*---- ---- ---- -x-- lamp*/
	if (data & 1)
	{
		//timer_set(space->machine,  ATTOTIME_IN_SEC(2), NULL, 0x100, shutter_req );
		state->shutter_sensor = 0x100;
	}
	else if (data & 2)
	{
		//timer_set( space->machine, ATTOTIME_IN_SEC(2), NULL, 0x200, shutter_req );
		state->shutter_sensor = 0x200;
	}

	if (data & 0x1000 || data & 0x4000)
	{
		//timer_set( space->machine, ATTOTIME_IN_SEC(2), NULL, 0x800, defender_req );
		state->defender_sensor = 0x800;
	}
	else if (data & 0x2000 || data & 0x8000)
	{
		//timer_set( space->machine, ATTOTIME_IN_SEC(2), NULL, 0x400, defender_req );
		state->defender_sensor = 0x400;
	}
}

static WRITE16_HANDLER( charram_w )
{
	_2mindril_state *state = (_2mindril_state *)space->machine->driver_data;

	COMBINE_DATA(&state->charram[offset]);
	gfx_element_mark_dirty(space->machine->gfx[1], offset / 16);
}

static ADDRESS_MAP_START( drill_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM
	AM_RANGE(0x300000, 0x3000ff) AM_RAM
	AM_RANGE(0x410000, 0x411fff) AM_RAM AM_BASE_MEMBER(_2mindril_state, map1ram)
	AM_RANGE(0x412000, 0x413fff) AM_RAM AM_BASE_MEMBER(_2mindril_state, map2ram)
	AM_RANGE(0x414000, 0x415fff) AM_RAM AM_BASE_MEMBER(_2mindril_state, map3ram)
	AM_RANGE(0x416000, 0x417fff) AM_RAM AM_BASE_MEMBER(_2mindril_state, map4ram)
	AM_RANGE(0x41c000, 0x41dfff) AM_RAM AM_BASE_MEMBER(_2mindril_state, textram)
	AM_RANGE(0x41e000, 0x41ffff) AM_RAM_WRITE(charram_w) AM_BASE_MEMBER(_2mindril_state, charram)
	AM_RANGE(0x400000, 0x4fffff) AM_RAM AM_BASE_MEMBER(_2mindril_state, unkram)// video stuff, 460000 - video regs ?
	AM_RANGE(0x500000, 0x501fff) AM_RAM_WRITE(paletteram16_RRRRGGGGBBBBRGBx_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x502000, 0x503fff) AM_RAM
	AM_RANGE(0x600000, 0x600007) AM_DEVREADWRITE8("ymsnd", ym2610_r, ym2610_w, 0x00ff)
	AM_RANGE(0x60000c, 0x60000d) AM_RAM
	AM_RANGE(0x60000e, 0x60000f) AM_RAM
	AM_RANGE(0x700000, 0x70000f) AM_READWRITE(drill_io_r,drill_io_w) AM_BASE_MEMBER(_2mindril_state, iodata) // i/o
	AM_RANGE(0x800000, 0x800001) AM_WRITE(sensors_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( drill )
	PORT_START("DSW")//Dip-Switches
	PORT_DIPNAME( 0x0001, 0x0001, "DSW" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

   	PORT_START("IN0")//sensors
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_START("IN1")
	PORT_DIPNAME( 0x0001, 0x0000, "IN1" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Unknown ) )//up sensor
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) )//down sensor
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) )//left sensor
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) )//right sensor
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("IN2")//coins
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Select SW-1")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Select SW-2")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Select SW-3")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Select SW-4")
INPUT_PORTS_END

static const gfx_layout drill_layout =
{
	16,16,
	RGN_FRAC(1,2),
	6,
	{ RGN_FRAC(1,2)+1, RGN_FRAC(1,2)+0 ,0,1,2,3 },
	{ 20, 16, 28, 24, 4, 0, 12, 8,        52, 48, 60, 56, 36, 32, 44, 40 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64, 8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	16*64
};

static const gfx_layout vramlayout=
{
    8,8,
    256,
    4,
    { 0, 1, 2, 3 },
    {20,16,28,24,4,0,12,8},
	  { 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
    32*8
};

static GFXDECODE_START( 2mindril )
	GFXDECODE_ENTRY( "gfx1", 0, drill_layout,  0, 256  )
	GFXDECODE_ENTRY( NULL,   0, vramlayout,    0, 256 )
GFXDECODE_END


static INTERRUPT_GEN( drill_interrupt )
{
	cpu_set_input_line(device, 4, HOLD_LINE);
}

/* WRONG,it does something with 60000c & 700002,likely to be called when the player throws the ball.*/
static void irqhandler(const device_config *device, int irq)
{
//  _2mindril_state *state = (_2mindril_state *)machine->driver_data;
//  cpu_set_input_line(state->maincpu, 5, irq ? ASSERT_LINE : CLEAR_LINE);
}

static const ym2610_interface ym2610_config =
{
	irqhandler
};


static MACHINE_START( drill )
{
	_2mindril_state *state = (_2mindril_state *)machine->driver_data;

	state->maincpu = devtag_get_device(machine, "maincpu");

	state_save_register_global(machine, state->defender_sensor);
	state_save_register_global(machine, state->shutter_sensor);
}

static MACHINE_RESET( drill )
{
	_2mindril_state *state = (_2mindril_state *)machine->driver_data;

	state->defender_sensor = 0;
	state->shutter_sensor = 0;
}

static MACHINE_DRIVER_START( drill )
	MDRV_DRIVER_DATA(_2mindril_state)

	MDRV_CPU_ADD("maincpu", M68000, 16000000 )
	MDRV_CPU_PROGRAM_MAP(drill_map)
	MDRV_CPU_VBLANK_INT("screen", drill_interrupt)
	MDRV_GFXDECODE(2mindril)

	MDRV_MACHINE_START(drill)
	MDRV_MACHINE_RESET(drill)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(128*16, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(0, 319, 0, 239-16)
	MDRV_PALETTE_LENGTH(0x1000)

	MDRV_VIDEO_START(drill)
	MDRV_VIDEO_UPDATE(drill)

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2610, 16000000/2)
	MDRV_SOUND_CONFIG(ym2610_config)
	MDRV_SOUND_ROUTE(0, "lspeaker",  0.25)
	MDRV_SOUND_ROUTE(0, "rspeaker", 0.25)
	MDRV_SOUND_ROUTE(1, "lspeaker",  1.0)
	MDRV_SOUND_ROUTE(2, "rspeaker", 1.0)
MACHINE_DRIVER_END


ROM_START( 2mindril )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "d58-38.ic11", 0x00000, 0x40000, CRC(c58e8e4f) SHA1(648db679c3bfb5de1cd6c1b1217773a2fe56f11b) )
	ROM_LOAD16_BYTE( "d58-37.ic9",  0x00001, 0x40000, CRC(19e5cc3c) SHA1(04ac0eef893c579fe90d91d7fd55c5741a2b7460) )

	ROM_REGION( 0x200000, "ymsnd", 0 ) /* Samples */
	ROM_LOAD( "d58-11.ic31", 0x000000, 0x200000,  CRC(dc26d58d) SHA1(cffb18667da18f5367b02af85a2f7674dd61ae97) )

	ROM_REGION( 0x800000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD32_WORD( "d58-09.ic28", 0x000000, 0x200000, CRC(d8f6a86a) SHA1(d6b2ec309e21064574ee63e025ae4716b1982a98) )
	ROM_LOAD32_WORD( "d58-08.ic27", 0x000002, 0x200000, CRC(9f5a3f52) SHA1(7b696bd823819965b974c853cebc1660750db61e) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD32_WORD( "d58-10.ic29", 0x000000, 0x200000, CRC(74c87e08) SHA1(f39b3a64f8338ccf5ca6eb76cee92a10fe0aad8f) )
ROM_END

static DRIVER_INIT( drill )
{
	// rearrange gfx roms to something we can decode, two of the roms form 4bpp of the graphics, the third forms another 2bpp but is in a different format
	UINT32 *src = (UINT32*)memory_region( machine, "gfx2" );
	UINT32 *dst = (UINT32*)memory_region( machine, "gfx1" );// + 0x400000;
//  UINT8 *rom = memory_region( machine, "maincpu" );
	int i;

	for (i = 0; i < 0x400000 / 4; i++)
	{
		UINT32 dat1 = src[i];
		dat1 = BITSWAP32(dat1, 3, 11, 19, 27, 2, 10, 18, 26, 1, 9, 17, 25, 0, 8, 16, 24, 7, 15, 23, 31, 6, 14, 22, 30, 5, 13, 21, 29, 4, 12, 20, 28 );
		dst[(0x400000 / 4) + i] = dat1;
	}

	//enable some kind of debug mode (ignore errors)
//  rom[0x7fffb] = 0;
//  rom[0x7fffc] = 0;
//  rom[0x7fffd] = 0;
//  rom[0x7fffe] = 0;
}

GAME( 1993, 2mindril,    0,        drill,    drill,    drill, ROT0,  "Taito", "Two Minute Drill", GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
