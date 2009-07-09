/***************************************************************************

    Jaleco Exerion hardware

****************************************************************************

    Exerion is a unique driver in that it has idiosyncracies that are straight
    out of Bizarro World. I submit for your approval:

    * The mystery reads from $d802 - timer-based protection?
    * The freakish graphics encoding scheme, which no other MAME-supported game uses
    * The sprite-ram, and all the funky parameters that go along with it

***************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "exerion.h"
#include "sound/ay8910.h"


static UINT8 *exerion_ram;



/*************************************
 *
 *  Interrupts & inputs
 *
 *************************************/

static READ8_HANDLER( exerion_port01_r )
{
	/* the cocktail flip bit muxes between ports 0 and 1 */
	return exerion_cocktail_flip ? input_port_read(space->machine, "IN1") : input_port_read(space->machine, "IN0");
}


static INPUT_CHANGED( coin_inserted )
{
	/* coin insertion causes an NMI */
	cputag_set_input_line(field->port->machine, "maincpu", INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}



/*************************************
 *
 *  Protection??
 *
 *************************************/

/* This is the first of many Exerion "features." No clue if it's */
/* protection or some sort of timer. */
static UINT8 porta;
static UINT8 portb;

static READ8_DEVICE_HANDLER( exerion_porta_r )
{
	porta ^= 0x40;
	return porta;
}


static WRITE8_DEVICE_HANDLER( exerion_portb_w )
{
	/* pull the expected value from the ROM */
	porta = memory_region(device->machine, "maincpu")[0x5f76];
	portb = data;

	logerror("Port B = %02X\n", data);
}


static READ8_HANDLER( exerion_protection_r )
{
	if (cpu_get_pc(space->cpu) == 0x4143)
		return memory_region(space->machine, "maincpu")[0x33c0 + (exerion_ram[0xd] << 2) + offset];
	else
		return exerion_ram[0x8 + offset];
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6008, 0x600b) AM_READ(exerion_protection_r)
	AM_RANGE(0x6000, 0x67ff) AM_RAM AM_BASE(&exerion_ram)
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_BASE(&videoram) AM_SIZE(&videoram_size)
	AM_RANGE(0x8800, 0x887f) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0x8800, 0x8bff) AM_RAM
	AM_RANGE(0xa000, 0xa000) AM_READ(exerion_port01_r)
	AM_RANGE(0xa800, 0xa800) AM_READ_PORT("DSW0")
	AM_RANGE(0xb000, 0xb000) AM_READ_PORT("DSW1")
	AM_RANGE(0xc000, 0xc000) AM_WRITE(exerion_videoreg_w)
	AM_RANGE(0xc800, 0xc800) AM_WRITE(soundlatch_w)
	AM_RANGE(0xd000, 0xd001) AM_DEVWRITE("ay1", ay8910_address_data_w)
	AM_RANGE(0xd800, 0xd801) AM_DEVWRITE("ay2", ay8910_address_data_w)
	AM_RANGE(0xd802, 0xd802) AM_DEVREAD("ay2", ay8910_r)
ADDRESS_MAP_END



/*************************************
 *
 *  Sub CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( sub_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x6000, 0x6000) AM_READ(soundlatch_r)
	AM_RANGE(0x8000, 0x800c) AM_WRITE(exerion_video_latch_w)
	AM_RANGE(0xa000, 0xa000) AM_READ(exerion_video_timing_r)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( exerion )
	PORT_START("IN0")      /* player 1 inputs (muxed on 0xa000) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")      /* player 2 inputs (muxed on 0xa000) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW0")      /* dip switches (0xa800) */
	PORT_DIPNAME( 0x07, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x07, "Infinite (Cheat)")
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x08, "20000" )
	PORT_DIPSETTING(    0x10, "30000" )
	PORT_DIPSETTING(    0x18, "40000" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      /* used */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW1")      /* dip switches/VBLANK (0xb000) */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED(coin_inserted, 0)
INPUT_PORTS_END



/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0 },
	{ 16*0, 16*1, 16*2, 16*3, 16*4, 16*5, 16*6, 16*7 },
	16*8
};


/* 16 x 16 sprites -- requires reorganizing characters in init_exerion() */
static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{  3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0,
			16+3, 16+2, 16+1, 16+0, 24+3, 24+2, 24+1, 24+0 },
	{ 32*0, 32*1, 32*2, 32*3, 32*4, 32*5, 32*6, 32*7,
			32*8, 32*9, 32*10, 32*11, 32*12, 32*13, 32*14, 32*15 },
	64*8
};


/* Quick and dirty way to emulate pixel-doubled sprites. */
static const gfx_layout bigspritelayout =
{
	32,32,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{  3, 3, 2, 2, 1, 1, 0, 0,
			8+3, 8+3, 8+2, 8+2, 8+1, 8+1, 8+0, 8+0,
			16+3, 16+3, 16+2, 16+2, 16+1, 16+1, 16+0, 16+0,
			24+3, 24+3, 24+2, 24+2, 24+1, 24+1, 24+0, 24+0 },
	{ 32*0, 32*0, 32*1, 32*1, 32*2, 32*2, 32*3, 32*3,
			32*4, 32*4, 32*5, 32*5, 32*6, 32*6, 32*7, 32*7,
			32*8, 32*8, 32*9, 32*9, 32*10, 32*10, 32*11, 32*11,
			32*12, 32*12, 32*13, 32*13, 32*14, 32*14, 32*15, 32*15 },
	64*8
};


static GFXDECODE_START( exerion )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,         0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,     256, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, bigspritelayout,  256, 64 )
GFXDECODE_END



/*************************************
 *
 *  Sound interfaces
 *
 *************************************/

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_HANDLER(exerion_porta_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(exerion_portb_w)
};



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_DRIVER_START( exerion )

	MDRV_CPU_ADD("maincpu", Z80, EXERION_CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(main_map)

	MDRV_CPU_ADD("sub", Z80, EXERION_CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(sub_map)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(EXERION_PIXEL_CLOCK, EXERION_HTOTAL, EXERION_HBEND, EXERION_HBSTART, EXERION_VTOTAL, EXERION_VBEND, EXERION_VBSTART)

	MDRV_GFXDECODE(exerion)
	MDRV_PALETTE_LENGTH(256*3)

	MDRV_PALETTE_INIT(exerion)
	MDRV_VIDEO_START(exerion)
	MDRV_VIDEO_UPDATE(exerion)

	/* audio hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay1", AY8910, EXERION_AY8910_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MDRV_SOUND_ADD("ay2", AY8910, EXERION_AY8910_CLOCK)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( exerion )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "exerion.07",   0x0000, 0x2000, CRC(4c78d57d) SHA1(ac702e9ad2bc05493fb1355858667c31c36acfe4) )
	ROM_LOAD( "exerion.08",   0x2000, 0x2000, CRC(dcadc1df) SHA1(91388f617cfaa4289ca1c84c697fcfdd8834ae15) )
	ROM_LOAD( "exerion.09",   0x4000, 0x2000, CRC(34cc4d14) SHA1(511c9de038f7bcaf6f7c96f2cbbe50a80673fa72) )

	ROM_REGION( 0x2000, "sub", 0 )     /* 64k for the second CPU */
	ROM_LOAD( "exerion.05",   0x0000, 0x2000, CRC(32f6bff5) SHA1(a4d0289f9d1d9eea7ca9a32a0616af48da74b401) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "exerion.06",   0x00000, 0x2000, CRC(435a85a4) SHA1(f6846bfee11df754405d4d796e7d8ac0321b6eb6) ) /* fg chars */

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "exerion.11",   0x00000, 0x2000, CRC(f0633a09) SHA1(8989bcb12abadde34777f7c189cfa6e2dfe92d62) ) /* sprites */
	ROM_LOAD( "exerion.10",   0x02000, 0x2000, CRC(80312de0) SHA1(4fa3bb9d5c62e41a54e8909f8d3b47637137e913) )

	ROM_REGION( 0x08000, "gfx3", 0 )
	ROM_LOAD( "exerion.03",   0x00000, 0x2000, CRC(790595b8) SHA1(8016ac2394b25db38e962bcff4805380082f6683) ) /* bg data */
	ROM_LOAD( "exerion.04",   0x02000, 0x2000, CRC(d7abd0b9) SHA1(ca6413ecd324cf84e11b703a4eda2c1e6d28ff15) )
	ROM_LOAD( "exerion.01",   0x04000, 0x2000, CRC(5bb755cb) SHA1(ec92c518c116a78dbb23381468cefb3f930212cc) )
	ROM_LOAD( "exerion.02",   0x06000, 0x2000, CRC(a7ecbb70) SHA1(3c359d5bb21290a45d3eb18fea2b1f9439b931be) )

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "exerion.e1",   0x0000, 0x0020, CRC(2befcc20) SHA1(a24d3f691413378fde545a6ddcef7e5118e74019) ) /* palette */
	ROM_LOAD( "exerion.i8",   0x0020, 0x0100, CRC(31db0e08) SHA1(1041a778e86d3fe6f057cf40a0a08b30760f3887) ) /* fg char lookup table */
	ROM_LOAD( "exerion.h10",  0x0120, 0x0100, CRC(63b4c555) SHA1(30243041be4fa77ada71e8b29d721cad51640c29) ) /* sprite lookup table */
	ROM_LOAD( "exerion.i3",   0x0220, 0x0100, CRC(fe72ab79) SHA1(048a72e6db4768df687df927acaa70ef906b3dc0) ) /* bg char lookup table */
	ROM_LOAD( "exerion.k4",   0x0320, 0x0100, CRC(ffc2ba43) SHA1(03be1c41d6ac3fc11439caef04ef5ffa60d6aec4) ) /* bg char mixer */
ROM_END


ROM_START( exeriont )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "prom5.4p",     0x0000, 0x4000, CRC(58b4dc1b) SHA1(3e34d1eda0b0537dac1062e96259d4cc7c64049c) )
	ROM_LOAD( "prom6.4s",     0x4000, 0x2000, CRC(fca18c2d) SHA1(31077dada3ed4aa2e26af933f589e01e0c71e5cd) )

	ROM_REGION( 0x2000, "sub", 0 )     /* 64k for the second CPU */
	ROM_LOAD( "exerion.05",   0x0000, 0x2000, CRC(32f6bff5) SHA1(a4d0289f9d1d9eea7ca9a32a0616af48da74b401) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "exerion.06",   0x00000, 0x2000, CRC(435a85a4) SHA1(f6846bfee11df754405d4d796e7d8ac0321b6eb6) ) /* fg chars */

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "exerion.11",   0x00000, 0x2000, CRC(f0633a09) SHA1(8989bcb12abadde34777f7c189cfa6e2dfe92d62) ) /* sprites */
	ROM_LOAD( "exerion.10",   0x02000, 0x2000, CRC(80312de0) SHA1(4fa3bb9d5c62e41a54e8909f8d3b47637137e913) )

	ROM_REGION( 0x08000, "gfx3", 0 )
	ROM_LOAD( "exerion.03",   0x00000, 0x2000, CRC(790595b8) SHA1(8016ac2394b25db38e962bcff4805380082f6683) ) /* bg data */
	ROM_LOAD( "exerion.04",   0x02000, 0x2000, CRC(d7abd0b9) SHA1(ca6413ecd324cf84e11b703a4eda2c1e6d28ff15) )
	ROM_LOAD( "exerion.01",   0x04000, 0x2000, CRC(5bb755cb) SHA1(ec92c518c116a78dbb23381468cefb3f930212cc) )
	ROM_LOAD( "exerion.02",   0x06000, 0x2000, CRC(a7ecbb70) SHA1(3c359d5bb21290a45d3eb18fea2b1f9439b931be) )

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "exerion.e1",   0x0000, 0x0020, CRC(2befcc20) SHA1(a24d3f691413378fde545a6ddcef7e5118e74019) ) /* palette */
	ROM_LOAD( "exerion.i8",   0x0020, 0x0100, CRC(31db0e08) SHA1(1041a778e86d3fe6f057cf40a0a08b30760f3887) ) /* fg char lookup table */
	ROM_LOAD( "exerion.h10",  0x0120, 0x0100, CRC(63b4c555) SHA1(30243041be4fa77ada71e8b29d721cad51640c29) ) /* sprite lookup table */
	ROM_LOAD( "exerion.i3",   0x0220, 0x0100, CRC(fe72ab79) SHA1(048a72e6db4768df687df927acaa70ef906b3dc0) ) /* bg char lookup table */
	ROM_LOAD( "exerion.k4",   0x0320, 0x0100, CRC(ffc2ba43) SHA1(03be1c41d6ac3fc11439caef04ef5ffa60d6aec4) ) /* bg char mixer */
ROM_END


ROM_START( exerionb )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "eb5.bin",      0x0000, 0x4000, CRC(da175855) SHA1(11ea46fd1d504e16e5ffc604d74c1ce210d6be1c) )
	ROM_LOAD( "eb6.bin",      0x4000, 0x2000, CRC(0dbe2eff) SHA1(5b0e5e8453619beec46c4350d1b2ed571fe3dc24) )

	ROM_REGION( 0x2000, "sub", 0 )     /* 64k for the second CPU */
	ROM_LOAD( "exerion.05",   0x0000, 0x2000, CRC(32f6bff5) SHA1(a4d0289f9d1d9eea7ca9a32a0616af48da74b401) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "exerion.06",   0x00000, 0x2000, CRC(435a85a4) SHA1(f6846bfee11df754405d4d796e7d8ac0321b6eb6) ) /* fg chars */

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "exerion.11",   0x00000, 0x2000, CRC(f0633a09) SHA1(8989bcb12abadde34777f7c189cfa6e2dfe92d62) ) /* sprites */
	ROM_LOAD( "exerion.10",   0x02000, 0x2000, CRC(80312de0) SHA1(4fa3bb9d5c62e41a54e8909f8d3b47637137e913) )

	ROM_REGION( 0x08000, "gfx3", 0 )
	ROM_LOAD( "exerion.03",   0x00000, 0x2000, CRC(790595b8) SHA1(8016ac2394b25db38e962bcff4805380082f6683) ) /* bg data */
	ROM_LOAD( "exerion.04",   0x02000, 0x2000, CRC(d7abd0b9) SHA1(ca6413ecd324cf84e11b703a4eda2c1e6d28ff15) )
	ROM_LOAD( "exerion.01",   0x04000, 0x2000, CRC(5bb755cb) SHA1(ec92c518c116a78dbb23381468cefb3f930212cc) )
	ROM_LOAD( "exerion.02",   0x06000, 0x2000, CRC(a7ecbb70) SHA1(3c359d5bb21290a45d3eb18fea2b1f9439b931be) )

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "exerion.e1",   0x0000, 0x0020, CRC(2befcc20) SHA1(a24d3f691413378fde545a6ddcef7e5118e74019) ) /* palette */
	ROM_LOAD( "exerion.i8",   0x0020, 0x0100, CRC(31db0e08) SHA1(1041a778e86d3fe6f057cf40a0a08b30760f3887) ) /* fg char lookup table */
	ROM_LOAD( "exerion.h10",  0x0120, 0x0100, CRC(63b4c555) SHA1(30243041be4fa77ada71e8b29d721cad51640c29) ) /* sprite lookup table */
	ROM_LOAD( "exerion.i3",   0x0220, 0x0100, CRC(fe72ab79) SHA1(048a72e6db4768df687df927acaa70ef906b3dc0) ) /* bg char lookup table */
	ROM_LOAD( "exerion.k4",   0x0320, 0x0100, CRC(ffc2ba43) SHA1(03be1c41d6ac3fc11439caef04ef5ffa60d6aec4) ) /* bg char mixer */
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

static DRIVER_INIT( exerion )
{
	UINT32 oldaddr, newaddr, length;
	UINT8 *src, *dst, *temp;

	/* allocate some temporary space */
	temp = alloc_array_or_die(UINT8, 0x10000);

	/* make a temporary copy of the character data */
	src = temp;
	dst = memory_region(machine, "gfx1");
	length = memory_region_length(machine, "gfx1");
	memcpy(src, dst, length);

	/* decode the characters */
	/* the bits in the ROM are ordered: n8-n7 n6 n5 n4-v2 v1 v0 n3-n2 n1 n0 h2 */
	/* we want them ordered like this:  n8-n7 n6 n5 n4-n3 n2 n1 n0-v2 v1 v0 h2 */
	for (oldaddr = 0; oldaddr < length; oldaddr++)
	{
		newaddr = ((oldaddr     ) & 0x1f00) |       /* keep n8-n4 */
		          ((oldaddr << 3) & 0x00f0) |       /* move n3-n0 */
		          ((oldaddr >> 4) & 0x000e) |       /* move v2-v0 */
		          ((oldaddr     ) & 0x0001);        /* keep h2 */
		dst[newaddr] = src[oldaddr];
	}

	/* make a temporary copy of the sprite data */
	src = temp;
	dst = memory_region(machine, "gfx2");
	length = memory_region_length(machine, "gfx2");
	memcpy(src, dst, length);

	/* decode the sprites */
	/* the bits in the ROMs are ordered: n9 n8 n3 n7-n6 n5 n4 v3-v2 v1 v0 n2-n1 n0 h3 h2 */
	/* we want them ordered like this:   n9 n8 n7 n6-n5 n4 n3 n2-n1 n0 v3 v2-v1 v0 h3 h2 */
	for (oldaddr = 0; oldaddr < length; oldaddr++)
	{
		newaddr = ((oldaddr << 1) & 0x3c00) |       /* move n7-n4 */
		          ((oldaddr >> 4) & 0x0200) |       /* move n3 */
		          ((oldaddr << 4) & 0x01c0) |       /* move n2-n0 */
		          ((oldaddr >> 3) & 0x003c) |       /* move v3-v0 */
		          ((oldaddr     ) & 0xc003);        /* keep n9-n8 h3-h2 */
		dst[newaddr] = src[oldaddr];
	}

	free(temp);
}


static DRIVER_INIT( exerionb )
{
	UINT8 *ram = memory_region(machine, "maincpu");
	int addr;

	/* the program ROMs have data lines D1 and D2 swapped. Decode them. */
	for (addr = 0; addr < 0x6000; addr++)
		ram[addr] = (ram[addr] & 0xf9) | ((ram[addr] & 2) << 1) | ((ram[addr] & 4) >> 1);

	/* also convert the gfx as in Exerion */
	DRIVER_INIT_CALL(exerion);
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1983, exerion,  0,       exerion, exerion, exerion,  ROT90, "Jaleco", "Exerion", 0 )
GAME( 1983, exeriont, exerion, exerion, exerion, exerion,  ROT90, "Jaleco (Taito America license)", "Exerion (Taito)", 0 )
GAME( 1983, exerionb, exerion, exerion, exerion, exerionb, ROT90, "Jaleco", "Exerion (bootleg)", 0 )
