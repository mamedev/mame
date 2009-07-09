/***************************************************************************

Tank Battalion memory map (preliminary)

driver by Brad Oliver

$0000-$000f : bullet ram, first entry is player's bullet
$0010-$01ff : zero page & stack
$0200-$07ff : RAM
$0800-$0bff : videoram
$0c00-$0c1f : I/O

Read:
    $0c00-$0c03 : p1 joystick
    $0c04
    $0c07       : stop at grid self-test if bit 7 is low
    $0c0f       : stop at first self-test if bit 7 is low

    $0c18       : Cabinet, 0 = table, 1 = upright
    $0c19-$0c1a : Coinage, 00 = free play, 01 = 2 coin 1 credit, 10 = 1 coin 2 credits, 11 = 1 coin 1 credit
    $0c1b-$0c1c : Bonus, 00 = 10000, 01 = 15000, 10 = 20000, 11 = none
    $0c1d       : Tanks, 0 = 3, 1 = 2
    $0c1e-$0c1f : ??

Write:
    $0c00-$0c01 : p1/p2 start leds
    $0c02       : ?? written to at end of IRQ, either 0 or 1 - coin counter?
    $0c03       : ?? written to during IRQ if grid test is on
    $0c08       : ?? written to during IRQ if grid test is on
    $0c09       : Sound - coin ding
    $0c0a       : NMI enable (active low) ?? game only ??
    $0c0b       : Sound - background noise, 0 - low rumble, 1 - high rumble
    $0c0c       : Sound - player fire
    $0c0d       : Sound - explosion
    $0c0f       : NMI enable (active high) ?? demo only ??

    $0c10       : IRQ ack ??
    $0c18       : Watchdog ?? Not written to while game screen is up

$2000-$3fff : ROM

TODO:
    . Resistor values on the color prom need to be verified

Changes:
    28 Feb 98 LBO
        . Fixed the coin interrupts
        . Fixed the color issues, should be 100% if I guessed at the resistor values properly
        . Fixed the 2nd player cocktail joystick, had the polarity reversed
        . Hacked the sound sample triggers so they work better

Known issues:
    . The 'moving' tank rumble noise seems to keep playing a second too long
    . Sample support is all a crapshoot. I have no idea how it really works

***************************************************************************/

#include "driver.h"
#include "cpu/m6502/m6502.h"
#include "sound/samples.h"

extern UINT8 *tankbatt_bulletsram;
extern size_t tankbatt_bulletsram_size;

static int tankbatt_nmi_enable; /* No need to init this - the game will set it on reset */
static int tankbatt_sound_enable;

extern WRITE8_HANDLER( tankbatt_videoram_w );

extern PALETTE_INIT( tankbatt );
extern VIDEO_START( tankbatt );
extern VIDEO_UPDATE( tankbatt );

static WRITE8_HANDLER( tankbatt_led_w )
{
	set_led_status(offset,data & 1);
}

static READ8_HANDLER( tankbatt_in0_r )
{
	int val;

	val = input_port_read(space->machine, "P1");
	return ((val << (7 - offset)) & 0x80);
}

static READ8_HANDLER( tankbatt_in1_r )
{
	int val;

	val = input_port_read(space->machine, "P2");
	return ((val << (7 - offset)) & 0x80);
}

static READ8_HANDLER( tankbatt_dsw_r )
{
	int val;

	val = input_port_read(space->machine, "DSW");
	return ((val << (7 - offset)) & 0x80);
}

static WRITE8_HANDLER( tankbatt_interrupt_enable_w )
{
	tankbatt_nmi_enable = !data;
	tankbatt_sound_enable = !data;
	if (data != 0)
	{
		cputag_set_input_line(space->machine, "maincpu", 0, CLEAR_LINE);
		cputag_set_input_line(space->machine, "maincpu", INPUT_LINE_NMI, CLEAR_LINE);
	}
	/* hack - turn off the engine noise if the normal game nmi's are disabled */
	if (data) sample_stop (devtag_get_device(space->machine, "samples"), 2);
//  interrupt_enable_w (offset, !data);
}

static WRITE8_HANDLER( tankbatt_demo_interrupt_enable_w )
{
	tankbatt_nmi_enable = data;
	if (data != 0)
	{
		cputag_set_input_line(space->machine, "maincpu", 0, CLEAR_LINE);
		cputag_set_input_line(space->machine, "maincpu", INPUT_LINE_NMI, CLEAR_LINE);
	}
//  interrupt_enable_w (offset, data);
}

static WRITE8_HANDLER( tankbatt_sh_expl_w )
{
	if (tankbatt_sound_enable)
	{
		const device_config *samples = devtag_get_device(space->machine, "samples");
		sample_start (samples, 1, 3, 0);
	}
}

static WRITE8_HANDLER( tankbatt_sh_engine_w )
{
	const device_config *samples = devtag_get_device(space->machine, "samples");
	if (tankbatt_sound_enable)
	{
		if (data)
			sample_start (samples, 2, 2, 1);
		else
			sample_start (samples, 2, 1, 1);
	}
	else sample_stop (samples, 2);
}

static WRITE8_HANDLER( tankbatt_sh_fire_w )
{
	if (tankbatt_sound_enable)
	{
		const device_config *samples = devtag_get_device(space->machine, "samples");
		sample_start (samples, 0, 0, 0);
	}
}

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x01ff) AM_READ(SMH_RAM)
	AM_RANGE(0x0000, 0x000f) AM_WRITE(SMH_RAM) AM_BASE(&tankbatt_bulletsram) AM_SIZE(&tankbatt_bulletsram_size)
	AM_RANGE(0x0010, 0x01ff) AM_WRITE(SMH_RAM)
	AM_RANGE(0x0800, 0x0bff) AM_WRITE(tankbatt_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0x0c00, 0x0c07) AM_READ(tankbatt_in0_r)
	AM_RANGE(0x0c00, 0x0c01) AM_WRITE(tankbatt_led_w)
	AM_RANGE(0x0c08, 0x0c0f) AM_READ(tankbatt_in1_r)
	AM_RANGE(0x0c0a, 0x0c0a) AM_WRITE(tankbatt_interrupt_enable_w)
	AM_RANGE(0x0c0b, 0x0c0b) AM_WRITE(tankbatt_sh_engine_w)
	AM_RANGE(0x0c0c, 0x0c0c) AM_WRITE(tankbatt_sh_fire_w)
	AM_RANGE(0x0c0d, 0x0c0d) AM_WRITE(tankbatt_sh_expl_w)
	AM_RANGE(0x0c0f, 0x0c0f) AM_WRITE(tankbatt_demo_interrupt_enable_w)
	AM_RANGE(0x0c18, 0x0c1f) AM_READ(tankbatt_dsw_r)
	AM_RANGE(0x0c18, 0x0c18) AM_WRITENOP	/* watchdog ?? */
	AM_RANGE(0x0200, 0x0bff) AM_READ(SMH_RAM)
	AM_RANGE(0x0200, 0x07ff) AM_WRITE(SMH_RAM)
	AM_RANGE(0x2000, 0x3fff) AM_WRITE(SMH_ROM)
	AM_RANGE(0x6000, 0x7fff) AM_READ(SMH_ROM)
	AM_RANGE(0xf800, 0xffff) AM_READ(SMH_ROM)	/* for the reset / interrupt vectors */
ADDRESS_MAP_END

static INTERRUPT_GEN( tankbatt_interrupt )
{
	if ((input_port_read(device->machine, "P1") & 0x60) == 0) cpu_set_input_line(device,0,HOLD_LINE);
	else if (tankbatt_nmi_enable) cpu_set_input_line(device,INPUT_LINE_NMI,PULSE_LINE);
}

static INPUT_PORTS_START( tankbatt )
	PORT_START("P1")	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x60, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_TILT )

	PORT_START("P2")	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW")	/* DSW */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(	0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Cocktail ) )
 	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Coinage ) )
	PORT_DIPSETTING(	0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x06, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(	0x00, "10000" )
	PORT_DIPSETTING(	0x10, "15000" )
	PORT_DIPSETTING(	0x08, "20000" )
	PORT_DIPSETTING(	0x18, DEF_STR( None ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(	0x20, "2" )
	PORT_DIPSETTING(	0x00, "3" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,	/* 8*8 characters */
	256,	/* 256 characters */
	1,	/* 1 bit per pixel */
	{ 0 },	/* only one bitplane */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8	/* every char takes 8 consecutive bytes */
};

static const gfx_layout bulletlayout =
{
	/* there is no gfx ROM for this one, it is generated by the hardware */
	3,3,	/* 3*3 box */
	1,	/* just one */
	1,	/* 1 bit per pixel */
	{ 8*8 },
	{ 2, 2, 2 },   /* I "know" that this bit of the */
	{ 2, 2, 2 },   /* graphics ROMs is 1 */
	0	/* no use */
};


static GFXDECODE_START( tankbatt )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 256 )
	GFXDECODE_ENTRY( "gfx1", 0, bulletlayout, 0, 256 )
GFXDECODE_END



static const char *const tankbatt_sample_names[] =
{
	"*tankbatt",
	"fire.wav",
	"engine1.wav",
	"engine2.wav",
	"explode1.wav",
    0	/* end of array */
};

static const samples_interface tankbatt_samples_interface =
{
	3,	/* 3 channels */
	tankbatt_sample_names
};



static MACHINE_DRIVER_START( tankbatt )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6502, 1000000)	/* 1 MHz ???? */
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT("screen", tankbatt_interrupt)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(tankbatt)
	MDRV_PALETTE_LENGTH(256*2)

	MDRV_PALETTE_INIT(tankbatt)
	MDRV_VIDEO_START(tankbatt)
	MDRV_VIDEO_UPDATE(tankbatt)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("samples", SAMPLES, 0)
	MDRV_SOUND_CONFIG(tankbatt_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tankbatt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tb1-1.1a",  0x6000, 0x0800, CRC(278a0b8c) SHA1(11ea8fe8401b3cd986616a30a759c0ac1a5ce73b) )
	ROM_LOAD( "tb1-2.1b",  0x6800, 0x0800, CRC(e0923370) SHA1(8d3dbea877bed9f9c267d8002dc180f6eb1e5a8f) )
	ROM_LOAD( "tb1-3.1c",  0x7000, 0x0800, CRC(85005ea4) SHA1(91583081803a5ef600fb90bee34be9edd87f157e) )
	ROM_LOAD( "tb1-4.1d",  0x7800, 0x0800, CRC(3dfb5bcf) SHA1(aa24bf74f4d5dc81baf3843196c837e0b731077b) )
	ROM_RELOAD(            0xf800, 0x0800 )	/* for the reset and interrupt vectors */

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "tb1-5.2k",  0x0000, 0x0800, CRC(aabd4fb1) SHA1(5cff659b531d0f1b6faa503f7c06045c3a209a84) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "bct1-1.l3", 0x0000, 0x0100, CRC(d17518bc) SHA1(f3b0deffa586808bc59e9a24ec1699c54ebe84cc) ) /* prom is a Fujitsu MB7052 or equivalent */
ROM_END



GAME( 1980, tankbatt, 0, tankbatt, tankbatt, 0, ROT90, "Namco", "Tank Battalion", 0 )
