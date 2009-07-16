/***************************************************************************

Bega's Battle (c) 1983 Data East Corporation

preliminary driver by Angelo Salese

TODO:
- laserdisc hook-ups;
- video emulation is bare bones;
- i/os

***************************************************************************/

#include "driver.h"
#include "cpu/m6502/m6502.h"
#include "sound/ay8910.h"
#include "machine/laserdsc.h"

static UINT8 vram_bank;
static const device_config *laserdisc;
static UINT8 laserdisc_data;

static VIDEO_UPDATE( rblaster )
{
	const gfx_element *gfx = screen->machine->gfx[0];
	int count = 0x0000;

	int y,x;

	for (y=0;y<32;y++)
	{
		for (x=0;x<32;x++)
		{
			int tile = videoram[count];
			int colour = (vram_bank & 0x7);
			drawgfx_opaque(bitmap,cliprect,gfx,tile,colour,0,0,x*8,y*8);

			count++;
		}
	}

	return 0;
}

#if 0
static WRITE8_HANDLER( rblaster_sound_w )
{
 	soundlatch_w(space,0,data);
 	cpu_set_input_line(space->machine->cpu[1], 0, HOLD_LINE);
}
#endif

static WRITE8_HANDLER( rblaster_vram_bank_w )
{
	vram_bank = data;
}

static READ8_HANDLER( laserdisc_r )
{
	UINT8 result = laserdisc_data_r(laserdisc);
	mame_printf_debug("laserdisc_r = %02X\n", result);
	return result;
}


static WRITE8_HANDLER( laserdisc_w )
{
	laserdisc_data = data;
}

static READ8_HANDLER( test_r )
{
	return mame_rand(space->machine);
}

static ADDRESS_MAP_START( cobra_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
	AM_RANGE(0x1000, 0x1000) AM_READ_PORT("IN1")
	AM_RANGE(0x1001, 0x1001) AM_READ(test_r)//_PORT("IN2")
	AM_RANGE(0x1002, 0x1002) AM_READ(test_r)//_PORT("IN3")
	AM_RANGE(0x1003, 0x1003) AM_READ(test_r)//AM_READ_PORT("IN0")
//  AM_RANGE(0x1004, 0x1004) AM_READ(test_r)//_PORT("IN4")
//  AM_RANGE(0x1005, 0x1005) AM_READ(test_r)//_PORT("IN5")
	AM_RANGE(0x1004, 0x1004) AM_WRITE(rblaster_vram_bank_w) //might be 1001
	AM_RANGE(0x1006, 0x1006) AM_NOP //ld status / command
	AM_RANGE(0x1007, 0x1007) AM_READWRITE(laserdisc_r,laserdisc_w) // ld data
	AM_RANGE(0x1800, 0x1fff) AM_RAM_WRITE(paletteram_RRRGGGBB_w) AM_BASE(&paletteram)
	AM_RANGE(0x2000, 0x2fff) AM_RAM
	AM_RANGE(0x3000, 0x37ff) AM_RAM //vram attr?
	AM_RANGE(0x3800, 0x3fff) AM_RAM AM_BASE(&videoram)
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( rblaster_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
//  AM_RANGE(0x1000, 0x1007) AM_NOP
	AM_RANGE(0x1001, 0x1001) AM_WRITENOP //???
	AM_RANGE(0x1003, 0x1003) AM_READ_PORT("IN0")
	AM_RANGE(0x1003, 0x1003) AM_WRITE(rblaster_vram_bank_w) //might be 1001
	AM_RANGE(0x1006, 0x1006) AM_NOP //ld status / command
	AM_RANGE(0x1007, 0x1007) AM_READWRITE(laserdisc_r,laserdisc_w) // ld data
	AM_RANGE(0x1800, 0x1fff) AM_RAM_WRITE(paletteram_RRRGGGBB_w) AM_BASE(&paletteram)
	AM_RANGE(0x2800, 0x2fff) AM_RAM AM_BASE(&videoram)
	AM_RANGE(0x3000, 0x3fff) AM_RAM
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END

/* sound arrangement is pratically identical to Zero Target. */
static int nmimask;

//static WRITE8_HANDLER( nmimask_w ) { nmimask = data & 0x80; }

static INTERRUPT_GEN ( sound_interrupt ) { if (!nmimask) cpu_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE); }


static ADDRESS_MAP_START( rblaster_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x01ff) AM_RAM
	AM_RANGE(0x2000, 0x2000) AM_DEVWRITE("ay1", ay8910_data_w)
	AM_RANGE(0x4000, 0x4000) AM_DEVWRITE("ay1", ay8910_address_w)
	AM_RANGE(0x6000, 0x6000) AM_DEVWRITE("ay2", ay8910_data_w)
	AM_RANGE(0x8000, 0x8000) AM_DEVWRITE("ay2", ay8910_address_w)
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_r)
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( cobra )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
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
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
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

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
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

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
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

	PORT_START("IN4")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
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

	PORT_START("IN5")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
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
INPUT_PORTS_END

static INPUT_PORTS_START( rblaster )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
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
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
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

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
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
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3),RGN_FRAC(1,3),RGN_FRAC(0,3) },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	8*8
};

static GFXDECODE_START( rblaster )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 8 )
GFXDECODE_END

static MACHINE_START( rblaster )
{
	laserdisc = devtag_get_device(machine, "laserdisc");
}

static MACHINE_DRIVER_START( rblaster )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu",M6502,8000000/2)
	MDRV_CPU_PROGRAM_MAP(rblaster_map)
//  MDRV_CPU_VBLANK_INT("screen",irq0_line_hold)
	MDRV_CPU_VBLANK_INT("screen",nmi_line_pulse)

	MDRV_CPU_ADD("audiocpu",M6502,8000000/2)
	MDRV_CPU_PROGRAM_MAP(rblaster_sound_map)
//  MDRV_CPU_VBLANK_INT("screen",irq0_line_hold) //test
	MDRV_CPU_PERIODIC_INT(sound_interrupt, 640)

	MDRV_LASERDISC_ADD("laserdisc", PIONEER_LDV1000, "screen", "ldsound") //Sony LDP-1000A, is it truly compatible with the Pioneer?
	MDRV_LASERDISC_OVERLAY(rblaster, 256, 256, BITMAP_FORMAT_INDEXED16)

	/* video hardware */
	MDRV_LASERDISC_SCREEN_ADD_NTSC("screen", BITMAP_FORMAT_INDEXED16)
	MDRV_GFXDECODE(rblaster)
	MDRV_PALETTE_LENGTH(512)
	MDRV_MACHINE_START(rblaster)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MDRV_SOUND_ADD("ay1", AY8910, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)

	MDRV_SOUND_ADD("ay2", AY8910, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)

	MDRV_SOUND_ADD("ldsound", LASERDISC, 0)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( cobra )
	MDRV_IMPORT_FROM( rblaster )
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(cobra_map)

	MDRV_DEVICE_REMOVE("audiocpu")
MACHINE_DRIVER_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( rblaster )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "01.bin",   0xc000, 0x2000, CRC(e4733c49) SHA1(357f46a80273f8a365d16cddf5e2caaeeacaf4ad) )
	ROM_LOAD( "00.bin",   0xe000, 0x2000, CRC(084d6ae2) SHA1(f49eb2d53bad5af88a12535ba628c9decce690ff) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "02.bin",   0xe000, 0x2000, CRC(6c20335d) SHA1(b28e80f112553af8e3fba9ebbfc10d1f56396ac1) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "03.bin",   0x0000, 0x2000, CRC(d1ff5ffb) SHA1(29df207e225e3b0477d5566d256198310d6ae526) )
	ROM_LOAD( "06.bin",   0x2000, 0x2000, CRC(d1ff5ffb) SHA1(29df207e225e3b0477d5566d256198310d6ae526) )
	ROM_LOAD( "04.bin",   0x4000, 0x2000, CRC(da2c84d9) SHA1(3452b0e2a45fa771e226c3a3668afbf3ceb0ec11) )
	ROM_LOAD( "07.bin",   0x6000, 0x2000, CRC(da2c84d9) SHA1(3452b0e2a45fa771e226c3a3668afbf3ceb0ec11) )
	ROM_LOAD( "05.bin",   0x8000, 0x2000, CRC(4608b516) SHA1(44af4be84a0b807ea0813ce86376a4b6fd927e5a) )
	ROM_LOAD( "08.bin",   0xa000, 0x2000, CRC(4608b516) SHA1(44af4be84a0b807ea0813ce86376a4b6fd927e5a) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "rblaster", 0, NO_DUMP )
ROM_END

ROM_START( cobra )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "au03-2",   0x8000, 0x2000, CRC(8f0a8fba) SHA1(8e11d2bd665a5ca6b3bb11aa2b707458c1534327) )
	ROM_LOAD( "au02-2",   0xa000, 0x2000, CRC(7db11acf) SHA1(1eebae0741f5735bc8966f3c31a9c07dac2e3916) )
	ROM_LOAD( "au01-2",   0xc000, 0x2000, CRC(523dd8f6) SHA1(47bd4c9b2272e9a710e6e97f2505075df68101ed) )
	ROM_LOAD( "au00-2",   0xe000, 0x2000, CRC(6c0f1f16) SHA1(ed05d3eaa24e84b1dfb4e1eb5f69b23e4a1494ba) )
	ROM_COPY( "maincpu",  0x8000, 0x4000, 0x4000 )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "au0a",   0x0000, 0x2000, CRC(6aaedcf3) SHA1(52dc913eecf8a159784d500217cffd7a6d8eb45c) )
	ROM_LOAD( "au0b",   0x4000, 0x2000, CRC(92247877) SHA1(f9bb0c20212ab13caabfb5beb9b6afc807bc9555) )
	ROM_LOAD( "au0c",   0x8000, 0x2000, CRC(d00a2762) SHA1(84d4329b39b9fd30682b7efa5cb2744934c5ee5c) )

	ROM_LOAD( "au07",   0x2000, 0x2000, CRC(d4bf12a5) SHA1(e172f69ae02ac2670b70af0cfcf3887dd99c2761) )
	ROM_LOAD( "au08",   0x6000, 0x2000, CRC(63158274) SHA1(c728e8ba0a11ea67cf508877ad74a3aab9ef26fc) )
	ROM_LOAD( "au09",   0xa000, 0x2000, CRC(74e93394) SHA1(7a1470cf2008b1bef8d950939b758707297b3655) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "cobra", 0, SHA1(8390498294aca97a5d1769032e7b115d1a42f5d3) )
ROM_END

// Bega's Battle
GAME( 1984, cobra,  0,       cobra,  cobra,  0, ROT0, "Data East", "Cobra Command (Data East LD)", GAME_NOT_WORKING )
// Thunder Storm (Cobra Command Japanese version)
GAME( 1985, rblaster,  0,    rblaster,  rblaster,  0, ROT0, "Data East", "Road Blaster (Data East LD)", GAME_NOT_WORKING )
