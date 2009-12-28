/***************************************************************************

TODO:
- understand the 7segs properly;
- inputs;
- lamps;
- Fix sound index/sound stop for the ADPCM sound chips;

===========================================================================

KUNG-FU ROUSHI
(c)1987 NAMCO

6809         (4Mhz?)
RAM 2016x1
MSM5205 x 2  (2 SPEAKERS)
8255    x 2
custom chips : none

KR1.BIN : PROGRAM
KR2-KR6 : VOICE

50pin cable to 7seg&lamp control board.

flyer.gif from
http://www.hoizinger.com/ed/EM.HTML
----------------------------------------------------------------

***************************************************************************/

#include "driver.h"
#include "cpu/m6809/m6809.h"
#include "sound/msm5205.h"
#include "machine/8255ppi.h"

//static UINT8 io_data[6];
static UINT8 led[0x10]; //not the right size,TODO
static UINT8 mux_data;

static VIDEO_START( kungfur )
{

}

/* TODO: I believe that f segment is incorrectly connected (the following is just copied from acommand.c).*/
/*
     a
    ---
f   | | b
    -g-
e   | | c
    ---
     d
a & 1
b & 2
c & 4
d & 8
e & 10
f & 20
g & 40
7f
*/

#define LED_ON		0x0001
#define LED_OFF		0x0000

static void draw_led(bitmap_t *bitmap, int x, int y,UINT8 value)
{
	plot_box(bitmap, x, y, 6, 10, 0x00000000);

	/*a*/
	*BITMAP_ADDR16(bitmap, y+0, x+1) = ((value & 0x01) ? LED_ON : LED_OFF);
	*BITMAP_ADDR16(bitmap, y+0, x+2) = ((value & 0x01) ? LED_ON : LED_OFF);
	*BITMAP_ADDR16(bitmap, y+0, x+3) = ((value & 0x01) ? LED_ON : LED_OFF);
	/*b*/
	*BITMAP_ADDR16(bitmap, y+1, x+4) = ((value & 0x02) ? LED_ON : LED_OFF);
	*BITMAP_ADDR16(bitmap, y+2, x+4) = ((value & 0x02) ? LED_ON : LED_OFF);
	*BITMAP_ADDR16(bitmap, y+3, x+4) = ((value & 0x02) ? LED_ON : LED_OFF);
	/*c*/
	*BITMAP_ADDR16(bitmap, y+5, x+4) = ((value & 0x04) ? LED_ON : LED_OFF);
	*BITMAP_ADDR16(bitmap, y+6, x+4) = ((value & 0x04) ? LED_ON : LED_OFF);
	*BITMAP_ADDR16(bitmap, y+7, x+4) = ((value & 0x04) ? LED_ON : LED_OFF);
	/*d*/
	*BITMAP_ADDR16(bitmap, y+8, x+1) = ((value & 0x08) ? LED_ON : LED_OFF);
	*BITMAP_ADDR16(bitmap, y+8, x+2) = ((value & 0x08) ? LED_ON : LED_OFF);
	*BITMAP_ADDR16(bitmap, y+8, x+3) = ((value & 0x08) ? LED_ON : LED_OFF);
	/*e*/
	*BITMAP_ADDR16(bitmap, y+5, x+0) = ((value & 0x10) ? LED_ON : LED_OFF);
	*BITMAP_ADDR16(bitmap, y+6, x+0) = ((value & 0x10) ? LED_ON : LED_OFF);
	*BITMAP_ADDR16(bitmap, y+7, x+0) = ((value & 0x10) ? LED_ON : LED_OFF);
	/*f*/
	*BITMAP_ADDR16(bitmap, y+1, x+0) = ((value & 0x20) ? LED_ON : LED_OFF);
	*BITMAP_ADDR16(bitmap, y+2, x+0) = ((value & 0x20) ? LED_ON : LED_OFF);
	*BITMAP_ADDR16(bitmap, y+3, x+0) = ((value & 0x20) ? LED_ON : LED_OFF);
	/*g*/
	*BITMAP_ADDR16(bitmap, y+4, x+1) = ((value & 0x40) ? LED_ON : LED_OFF);
	*BITMAP_ADDR16(bitmap, y+4, x+2) = ((value & 0x40) ? LED_ON : LED_OFF);
	*BITMAP_ADDR16(bitmap, y+4, x+3) = ((value & 0x40) ? LED_ON : LED_OFF);
	/*"point" (just for debugging)*/
	*BITMAP_ADDR16(bitmap, y+9, x+4) = ((value & 0x80) ? LED_ON : LED_OFF);
}

/* actually debugging purpose, it will be converted to the artwork system at some point. */
static VIDEO_UPDATE( kungfur )
{
//  popmessage("%02x %02x %02x %02x %02x %02x",io_data[0],io_data[1],io_data[2],io_data[3],io_data[4],io_data[5]);
	int i;

	for(i=0;i<16;i++)
		draw_led(bitmap,  (i*8)+2, 100, led[i]);

	return 0;
}

static WRITE8_DEVICE_HANDLER( test0_w )
{
	mux_data = data & 7;	/* multiplexer selector? (00-06) */

	mux_data|= (data & 0x10)>>1;
//  printf("%02x MUX W\n",data);
}

static WRITE8_DEVICE_HANDLER( test1_w )
{
//  io_data[1] = data;  /* the whole port should be NULL */
//  printf("%02x Unk 1 W\n",data);
}

static WRITE8_DEVICE_HANDLER( test2_w )
{
//  io_data[2] = data;  /* lower nibble should be NULL */
	led[mux_data] = data;
//  printf("%02x Unk 2 W\n",data);
}

static WRITE8_DEVICE_HANDLER( test3_w )
{
//  io_data[3] = data;
//  printf("%02x Unk 3 W\n",data);
}

/*mux is always 0*/
static WRITE8_DEVICE_HANDLER( test4_w )
{
//  io_data[4] = data;
	led[mux_data] = data;
}

/* this looks like lamps. */
static WRITE8_DEVICE_HANDLER( test5_w )
{
//  io_data[5] = data;
//  printf("%02x Unk 5 W\n",data);
}

static UINT32 adpcm_pos[2];
static UINT8 adpcm_idle[2];

static WRITE8_DEVICE_HANDLER( kungfur_adpcm1_w )
{
	adpcm_pos[0] = 0x40000+(data & 0xff) * 0x100;
	adpcm_idle[0] = 0;
	msm5205_reset_w(devtag_get_device(device->machine, "adpcm1"),0);
}

static WRITE8_DEVICE_HANDLER( kungfur_adpcm2_w )
{
	adpcm_pos[1] = (data & 0xff) * 0x400;
	adpcm_idle[1] = 0;
	msm5205_reset_w(devtag_get_device(device->machine, "adpcm2"),0);
}

/*

  0000-07FF  ; RW ; Working RAM.
  4000-4000  ;  W ; M5205?
  4004-4004  ;  W ; M5205?
  4008-400B  ; RW ; PPI8255_0. PPI mode 0 - Ports B & lower C as Input.
  400C-400F  ;  W ; PPI8255_1. PPI mode 0 - All ports as output.
  C000-FFFF  ; R  ; ROM space.

*/
static ADDRESS_MAP_START( kungfur_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x4000, 0x4000) AM_DEVWRITE("adpcm1", kungfur_adpcm1_w)
	AM_RANGE(0x4004, 0x4004) AM_DEVWRITE("adpcm2", kungfur_adpcm2_w)
	AM_RANGE(0x4008, 0x400b) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)
	AM_RANGE(0x400c, 0x400f) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( kungfur )

	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "SYSTEM" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "SYSTEM" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

INPUT_PORTS_END

static const ppi8255_interface ppi8255_intf[2] =
/* Unused ports are still hooked for a easy visibility of the port writes */
{
	{/* PPI mode 0 - ports B & lower C as Input - (4008) */
		DEVCB_NULL,						/* Port A read */
		DEVCB_INPUT_PORT("IN0"),		/* Port B read */
		DEVCB_INPUT_PORT("IN1"),		/* Port C (lower) read */
		DEVCB_HANDLER(test0_w),			/* Port A write */
		DEVCB_HANDLER(test1_w),			/* Port B write */
		DEVCB_HANDLER(test2_w)			/* Port C (higher) write */
	},
	{/* PPI mode 0 - All ports as output - (400c) */
		DEVCB_NULL,						/* Port A read */
		DEVCB_NULL,						/* Port B read */
		DEVCB_NULL,						/* Port C read */
		DEVCB_HANDLER(test3_w),			/* Port A write */
		DEVCB_HANDLER(test4_w),			/* Port B write */
		DEVCB_HANDLER(test5_w)			/* Port C write */
	}
};

static void kfr_adpcm1_int(const device_config *device)
{
	static UINT8 trigger,adpcm_data;

	if (adpcm_pos[0] >= 0x40000 || adpcm_idle[0])
	{
		msm5205_reset_w(devtag_get_device(device->machine, "adpcm1"),1);
		trigger = 0;
	}
	else
	{
		UINT8 *ROM = memory_region(device->machine, "adpcm1");

		adpcm_data = ((trigger ? (ROM[adpcm_pos[0]] & 0x0f) : (ROM[adpcm_pos[0]] & 0xf0)>>4) );
		msm5205_data_w(devtag_get_device(device->machine, "adpcm1"),adpcm_data & 0xf);
		trigger^=1;
		if(trigger == 0)
		{
			adpcm_pos[0]++;
			if((ROM[adpcm_pos[0]] & 0xff) == 0xff)
				adpcm_idle[0] = 1;
		}
	}
}


static void kfr_adpcm2_int(const device_config *device)
{
	static UINT8 trigger,adpcm_data;

	if (adpcm_pos[1] >= 0x10000 || adpcm_idle[1])
	{
		msm5205_reset_w(devtag_get_device(device->machine, "adpcm2"),1);
		trigger = 0;
	}
	else
	{
		UINT8 *ROM = memory_region(device->machine, "adpcm2");

		adpcm_data = ((trigger ? (ROM[adpcm_pos[1]] & 0x0f) : (ROM[adpcm_pos[1]] & 0xf0)>>4) );
		msm5205_data_w(devtag_get_device(device->machine, "adpcm2"),adpcm_data & 0xf);
		trigger^=1;
		if(trigger == 0)
		{
			adpcm_pos[1]++;
			if((ROM[adpcm_pos[1]] & 0xff) == 0xff)
				adpcm_idle[1] = 1;
		}
	}
}

static const msm5205_interface msm5205_config_1 =
{
	kfr_adpcm1_int,	/* interrupt function */
	MSM5205_S48_4B	/* 8kHz */
};

static const msm5205_interface msm5205_config_2 =
{
	kfr_adpcm2_int,	/* interrupt function */
	MSM5205_S48_4B	/* 8kHz */
};

static MACHINE_RESET( kungfur )
{
	adpcm_pos[0] =	adpcm_pos[1] = 0;
	adpcm_idle[0] = adpcm_idle[1] = 1;
}

static INTERRUPT_GEN( kungfur_irq )
{
	cputag_set_input_line(device->machine, "maincpu", M6809_IRQ_LINE, HOLD_LINE);
}

static MACHINE_DRIVER_START( kungfur )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu",M6809,8000000/2)
	MDRV_CPU_PROGRAM_MAP(kungfur_map)
	MDRV_CPU_VBLANK_INT("screen",kungfur_irq)

	MDRV_PPI8255_ADD( "ppi8255_0", ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", ppi8255_intf[1] )

	MDRV_MACHINE_RESET( kungfur )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MDRV_PALETTE_LENGTH(512)

	MDRV_VIDEO_START(kungfur)
	MDRV_VIDEO_UPDATE(kungfur)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("adpcm1", MSM5205, 400000)
	MDRV_SOUND_CONFIG(msm5205_config_1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("adpcm2", MSM5205, 400000)
	MDRV_SOUND_CONFIG(msm5205_config_2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( kungfur )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "kr1.bin",   0xc000, 0x4000, CRC(f5b93cc7) SHA1(ed962915aeafea823a6562e6f284a88422f09a08) )

	ROM_REGION( 0x40000, "adpcm1", 0 )
	ROM_LOAD( "kr2.bin",   0x00000, 0x10000, CRC(13f5eba8) SHA1(a3ae2d54ec60d48bfff6192e61033ec583e3603f) )
	ROM_LOAD( "kr3.bin",   0x10000, 0x10000, CRC(05fd1301) SHA1(6871d872315ffb025fea7d2ccd9a203863dc142d) )
	ROM_LOAD( "kr4.bin",   0x20000, 0x10000, CRC(58929279) SHA1(d90f68dd8cf2ddc5e73ed40eb31ebbb0be7e35a4) )
	ROM_LOAD( "kr5.bin",   0x30000, 0x10000, CRC(31ed39c8) SHA1(8da50b2183a287fe3a41ec13078aff7fb40c43a3) )

	ROM_REGION( 0x10000, "adpcm2", 0 )
	ROM_LOAD( "kr6.bin",   0x00000, 0x10000, CRC(9ea75d4a) SHA1(57445ccb961acb11a25cdac81f2e543d92bcb7f9) )
ROM_END

GAME( 1987, kungfur,  0,       kungfur,  kungfur,  0, ROT0, "Namco", "Kung Fu Roushi", GAME_NOT_WORKING )
