/***************************************************************************

Gyruss memory map (preliminary)

Main processor memory map.
0000-5fff ROM (6000-7fff diagnostics)
8000-83ff Color RAM
8400-87ff Video RAM
9000-a7ff RAM
a000-a17f \ sprites
a200-a27f /

memory mapped ports:

read:
c080      IN0
c0a0      IN1
c0c0      IN2
c0e0      DSW0
c000      DSW1
c100      DSW2

write:
a000-a1ff  Odd frame spriteram
a200-a3ff  Even frame spriteram
a700       Frame odd or even?
a701       Semaphore system:  tells 6809 to draw queued sprites
a702       Semaphore system:  tells 6809 to queue sprites
c000       watchdog reset
c080       trigger interrupt on audio CPU
c100       command for the audio CPU
c180       interrupt enable
c185       flip screen

interrupts:
standard NMI at 0x66


SOUND BOARD:
0000-3fff  Audio ROM (4000-5fff diagnostics)
6000-63ff  Audio RAM
8000       Read Sound Command

I/O:

Gyruss has 5 PSGs:
1)  Control: 0x00    Read: 0x01    Write: 0x02
2)  Control: 0x04    Read: 0x05    Write: 0x06
3)  Control: 0x08    Read: 0x09    Write: 0x0a
4)  Control: 0x0c    Read: 0x0d    Write: 0x0e
5)  Control: 0x10    Read: 0x11    Write: 0x12

and 1 SFX channel controlled by an 8039:
1)  SoundOn: 0x14    SoundData: 0x18

***************************************************************************/

#include "driver.h"
#include "cpu/i8039/i8039.h"
#include "sound/ay8910.h"
#include "sound/dac.h"


void konami1_decode_cpu2(void);

WRITE8_HANDLER( gyruss_flipscreen_w );
READ8_HANDLER( gyruss_scanline_r );
VIDEO_START( gyruss );
PALETTE_INIT( gyruss );
VIDEO_UPDATE( gyruss );
INTERRUPT_GEN( gyruss_6809_interrupt );


READ8_HANDLER( gyruss_portA_r );
WRITE8_HANDLER( gyruss_filter0_w );
WRITE8_HANDLER( gyruss_filter1_w );
WRITE8_HANDLER( gyruss_sh_irqtrigger_w );
WRITE8_HANDLER( gyruss_i8039_irq_w );


static ADDRESS_MAP_START( readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x8000, 0x87ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x9000, 0x9fff) AM_READ(MRA8_RAM)
	AM_RANGE(0xa000, 0xa7ff) AM_READ(MRA8_RAM) AM_SHARE(1)
	AM_RANGE(0xc000, 0xc000) AM_READ(input_port_4_r)	/* DSW1 */
	AM_RANGE(0xc080, 0xc080) AM_READ(input_port_0_r)	/* IN0 */
	AM_RANGE(0xc0a0, 0xc0a0) AM_READ(input_port_1_r)	/* IN1 */
	AM_RANGE(0xc0c0, 0xc0c0) AM_READ(input_port_2_r)	/* IN2 */
	AM_RANGE(0xc0e0, 0xc0e0) AM_READ(input_port_3_r)	/* DSW0 */
	AM_RANGE(0xc100, 0xc100) AM_READ(input_port_5_r)	/* DSW2 */
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(MWA8_ROM)                 /* rom space+1        */
	AM_RANGE(0x8000, 0x83ff) AM_WRITE(MWA8_RAM) AM_BASE(&colorram)
	AM_RANGE(0x8400, 0x87ff) AM_WRITE(MWA8_RAM) AM_BASE(&videoram) AM_SIZE(&videoram_size)
	AM_RANGE(0x9000, 0x9fff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0xa000, 0xa7ff) AM_WRITE(MWA8_RAM) AM_SHARE(1)
	AM_RANGE(0xc000, 0xc000) AM_WRITE(MWA8_NOP)	/* watchdog reset */
	AM_RANGE(0xc080, 0xc080) AM_WRITE(gyruss_sh_irqtrigger_w)
	AM_RANGE(0xc100, 0xc100) AM_WRITE(soundlatch_w)         /* command to soundb  */
	AM_RANGE(0xc180, 0xc180) AM_WRITE(interrupt_enable_w)      /* NMI enable         */
	AM_RANGE(0xc185, 0xc185) AM_WRITE(gyruss_flipscreen_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( m6809_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0000) AM_READ(gyruss_scanline_r)
	AM_RANGE(0x4000, 0x47ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x6000, 0x67ff) AM_READ(MRA8_RAM) AM_SHARE(1)
	AM_RANGE(0xe000, 0xffff) AM_READ(MRA8_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( m6809_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x2000, 0x2000) AM_WRITE(interrupt_enable_w)
	AM_RANGE(0x4000, 0x47ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x4040, 0x40ff) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0x6000, 0x67ff) AM_WRITE(MWA8_RAM) AM_SHARE(1)
	AM_RANGE(0xe000, 0xffff) AM_WRITE(MWA8_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_READ(MRA8_ROM)                 /* rom soundboard     */
	AM_RANGE(0x6000, 0x63ff) AM_READ(MRA8_RAM)                 /* ram soundboard     */
	AM_RANGE(0x8000, 0x8000) AM_READ(soundlatch_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_WRITE(MWA8_ROM)                 /* rom soundboard     */
	AM_RANGE(0x6000, 0x63ff) AM_WRITE(MWA8_RAM)                 /* ram soundboard     */
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_readport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x01, 0x01) AM_READ(AY8910_read_port_0_r)
  	AM_RANGE(0x05, 0x05) AM_READ(AY8910_read_port_1_r)
	AM_RANGE(0x09, 0x09) AM_READ(AY8910_read_port_2_r)
  	AM_RANGE(0x0d, 0x0d) AM_READ(AY8910_read_port_3_r)
  	AM_RANGE(0x11, 0x11) AM_READ(AY8910_read_port_4_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0x00) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x04, 0x04) AM_WRITE(AY8910_control_port_1_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(AY8910_write_port_1_w)
	AM_RANGE(0x08, 0x08) AM_WRITE(AY8910_control_port_2_w)
	AM_RANGE(0x0a, 0x0a) AM_WRITE(AY8910_write_port_2_w)
	AM_RANGE(0x0c, 0x0c) AM_WRITE(AY8910_control_port_3_w)
	AM_RANGE(0x0e, 0x0e) AM_WRITE(AY8910_write_port_3_w)
	AM_RANGE(0x10, 0x10) AM_WRITE(AY8910_control_port_4_w)
	AM_RANGE(0x12, 0x12) AM_WRITE(AY8910_write_port_4_w)
	AM_RANGE(0x14, 0x14) AM_WRITE(gyruss_i8039_irq_w)
	AM_RANGE(0x18, 0x18) AM_WRITE(soundlatch2_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( i8039_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_READ(MRA8_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( i8039_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_WRITE(MWA8_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( i8039_readport, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0xff) AM_READ(soundlatch2_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( i8039_writeport, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(I8039_p1, I8039_p1) AM_WRITE(DAC_0_data_w)
	AM_RANGE(I8039_p2, I8039_p2) AM_WRITE(MWA8_NOP)
ADDRESS_MAP_END

#define GYRUSS_COMMON\
	PORT_START_TAG("IN0")\
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )\
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )\
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )\
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )\
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )\
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )\
	PORT_START_TAG("IN1")\
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY\
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY\
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_2WAY\
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_2WAY\
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )\
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* 1p shoot 2 - unused */\
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* 2p shoot 3 - unused */\
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )\
	PORT_START_TAG("IN2")\
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL\
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL\
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_2WAY PORT_COCKTAIL\
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_2WAY PORT_COCKTAIL\
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL\
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* 2p shoot 2 - unused */\
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )\
	PORT_START_TAG("DSW0")\
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )\
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )\
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )\
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )\
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )\
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )\
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )\
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )\
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )\
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )\
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )\
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )\
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )\
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )\
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )\
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )\
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )\
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )\
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )\
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )\
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )\
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )\
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )\
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )\
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )\
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )\
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )\
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )\
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )\
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )\
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )\
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )\
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )\
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

static INPUT_PORTS_START( gyruss )
GYRUSS_COMMON

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "255 (Cheat)")
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "30000 60000" )
	PORT_DIPSETTING(    0x00, "40000 70000" )
	PORT_DIPNAME( 0x70, 0x70, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x70, "1 (Easiest)" )
	PORT_DIPSETTING(    0x60, "2" )
	PORT_DIPSETTING(    0x50, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x30, "5 (Average)" )
	PORT_DIPSETTING(    0x20, "6" )
	PORT_DIPSETTING(    0x10, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hardest)" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "Demo Music" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* other bits probably unused */
INPUT_PORTS_END

/* This is identical to gyruss except for the bonus that has different
   values */
static INPUT_PORTS_START( gyrussce )
GYRUSS_COMMON

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "255 (Cheat)")
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "50000 70000" )
	PORT_DIPSETTING(    0x00, "60000 80000" )
	PORT_DIPNAME( 0x70, 0x70, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x70, "1 (Easiest)" )
	PORT_DIPSETTING(    0x60, "2" )
	PORT_DIPSETTING(    0x50, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x30, "5 (Average)" )
	PORT_DIPSETTING(    0x20, "6" )
	PORT_DIPSETTING(    0x10, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hardest)" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "Demo Music" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* other bits probably unused */
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,	/* 8*8 characters */
	512,	/* 512 characters */
	2,	/* 2 bits per pixel */
	{ 4, 0 },
	{ 0, 1, 2, 3, 8*8+0,8*8+1,8*8+2,8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8	/* every char takes 16 consecutive bytes */
};
static const gfx_layout spritelayout =
{
	8,16,	/* 8*16 sprites */
	256,	/* 256 sprites */
	4,	/* 4 bits per pixel */
	{ 0x4000*8+4, 0x4000*8+0, 4, 0  },
	{ 0, 1, 2, 3,  8*8, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8	/* every sprite takes 64 consecutive bytes */
};



static GFXDECODE_START( gyruss )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, charlayout,      0, 16 )
	GFXDECODE_ENTRY( REGION_GFX2, 0x0000, spritelayout, 16*4, 16 )	/* upper half */
	GFXDECODE_ENTRY( REGION_GFX2, 0x0010, spritelayout, 16*4, 16 )	/* lower half */
GFXDECODE_END



static const struct AY8910interface ay8910_interface_1 =
{
	0,
	0,
	0,
	gyruss_filter0_w
};

static const struct AY8910interface ay8910_interface_2 =
{
	0,
	0,
	0,
	gyruss_filter1_w
};

static const struct AY8910interface ay8910_interface_3 =
{
	gyruss_portA_r
};



static MACHINE_DRIVER_START( gyruss )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80, 3072000)	/* 3.072 MHz (?) */
	MDRV_CPU_PROGRAM_MAP(readmem,writemem)
	MDRV_CPU_VBLANK_INT(nmi_line_pulse,1)

	MDRV_CPU_ADD(M6809, 2000000)        /* 2 MHz ??? */
	MDRV_CPU_PROGRAM_MAP(m6809_readmem,m6809_writemem)
	MDRV_CPU_VBLANK_INT(gyruss_6809_interrupt,256)

	MDRV_CPU_ADD(Z80,14318180/4)
	/* audio CPU */	/* 3.579545 MHz */
	MDRV_CPU_PROGRAM_MAP(sound_readmem,sound_writemem)
	MDRV_CPU_IO_MAP(sound_readport,sound_writeport)

	MDRV_CPU_ADD(I8039,8000000)
	/* audio CPU */	/* 8MHz crystal */
	MDRV_CPU_PROGRAM_MAP(i8039_readmem,i8039_writemem)
	MDRV_CPU_IO_MAP(i8039_readport,i8039_writeport)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MDRV_GFXDECODE(gyruss)
	MDRV_PALETTE_LENGTH(16*4+16*16)

	MDRV_PALETTE_INIT(gyruss)
	MDRV_VIDEO_START(gyruss)
	MDRV_VIDEO_UPDATE(gyruss)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(AY8910, 14318180/8)
	MDRV_SOUND_CONFIG(ay8910_interface_1)
	MDRV_SOUND_ROUTE(0, "filter.0.0", 0.22)
	MDRV_SOUND_ROUTE(1, "filter.0.1", 0.22)
	MDRV_SOUND_ROUTE(2, "filter.0.2", 0.22)

	MDRV_SOUND_ADD(AY8910, 14318180/8)
	MDRV_SOUND_CONFIG(ay8910_interface_2)
	MDRV_SOUND_ROUTE(0, "filter.1.0", 0.22)
	MDRV_SOUND_ROUTE(1, "filter.1.1", 0.22)
	MDRV_SOUND_ROUTE(2, "filter.1.2", 0.22)

	MDRV_SOUND_ADD(AY8910, 14318180/8)
	MDRV_SOUND_CONFIG(ay8910_interface_3)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.44)

	MDRV_SOUND_ADD(AY8910, 14318180/8)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.44)

	MDRV_SOUND_ADD(AY8910, 14318180/8)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.44)

	MDRV_SOUND_ADD(DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.28)

	MDRV_SOUND_ADD_TAG("filter.0.0", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 1.0)
	MDRV_SOUND_ADD_TAG("filter.0.1", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 1.0)
	MDRV_SOUND_ADD_TAG("filter.0.2", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 1.0)

	MDRV_SOUND_ADD_TAG("filter.1.0", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 1.0)
	MDRV_SOUND_ADD_TAG("filter.1.1", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 1.0)
	MDRV_SOUND_ADD_TAG("filter.1.2", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 1.0)
MACHINE_DRIVER_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( gyruss )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "gyrussk.1",    0x0000, 0x2000, CRC(c673b43d) SHA1(7c464fb154bac35dd6e2f547e157addeb8798194) )
	ROM_LOAD( "gyrussk.2",    0x2000, 0x2000, CRC(a4ec03e4) SHA1(08c33ad7fcc2ad5e5787a1050284e3f8164f4618) )
	ROM_LOAD( "gyrussk.3",    0x4000, 0x2000, CRC(27454a98) SHA1(030c7df225652ee20d5ef64d005eb011dc89a27d) )
	/* the diagnostics ROM would go here */

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "gyrussk.9",    0xe000, 0x2000, CRC(822bf27e) SHA1(36d5bea2392a7d3476dd797dc05602705cfa23ef) )

	ROM_REGION( 0x10000, REGION_CPU3, 0 )
	ROM_LOAD( "gyrussk.1a",   0x0000, 0x2000, CRC(f4ae1c17) SHA1(ae568c96a31d910afe30d2b7eeb9ed1ed07290e3) )
	ROM_LOAD( "gyrussk.2a",   0x2000, 0x2000, CRC(ba498115) SHA1(9cd1f42898cc590f39ba7cb3c975b0b3d3062eba) )
	/* the diagnostics ROM would go here */

	ROM_REGION( 0x1000, REGION_CPU4, 0 )	/* 8039 */
	ROM_LOAD( "gyrussk.3a",   0x0000, 0x1000, CRC(3f9b5dea) SHA1(6e807da02c2885b18e8cc2199f12f6be9040bf75) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "gyrussk.4",    0x0000, 0x2000, CRC(27d8329b) SHA1(564ff945465a23d93a93137ad277298770dfa06a) )

	ROM_REGION( 0x8000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "gyrussk.6",    0x0000, 0x2000, CRC(c949db10) SHA1(fcb8bcbd2bdd751fecb322a33c8a92fb6f07a7ab) )
	ROM_LOAD( "gyrussk.5",    0x2000, 0x2000, CRC(4f22411a) SHA1(763bcd039f8c1838a0d7da7d4dadc14a26e25596) )
	ROM_LOAD( "gyrussk.8",    0x4000, 0x2000, CRC(47cd1fbc) SHA1(8203c4ff0b1cd7b4dbc708e300bfeac1e7366e09) )
	ROM_LOAD( "gyrussk.7",    0x6000, 0x2000, CRC(8e8d388c) SHA1(8f2928d71c02aba977d67575d6e34d69bda2b9d4) )

	ROM_REGION( 0x0220, REGION_PROMS, 0 )
	ROM_LOAD( "gyrussk.pr3",  0x0000, 0x0020, CRC(98782db3) SHA1(b891e43b25187faca8002919ccb44d744daa3594) )	/* palette */
	ROM_LOAD( "gyrussk.pr1",  0x0020, 0x0100, CRC(7ed057de) SHA1(c04069ae1e2c62f9b3048844cd8cf5e1b03b7d3c) )	/* sprite lookup table */
	ROM_LOAD( "gyrussk.pr2",  0x0120, 0x0100, CRC(de823a81) SHA1(1af94b2a6a319a89b238a5076a2867f1cfd279b0) )	/* character lookup table */
ROM_END

ROM_START( gyrussce )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "gya-1.bin",    0x0000, 0x2000, CRC(85f8b7c2) SHA1(5dde696b53efedee671d500feae1d314e95b1c96) )
	ROM_LOAD( "gya-2.bin",    0x2000, 0x2000, CRC(1e1a970f) SHA1(5a2e391489608f7571bbb4f85549a79795e2177e) )
	ROM_LOAD( "gya-3.bin",    0x4000, 0x2000, CRC(f6dbb33b) SHA1(19cab8e7f2f2358b6271ab402f132654e8be95d4) )
	/* the diagnostics ROM would go here */

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "gyrussk.9",    0xe000, 0x2000, CRC(822bf27e) SHA1(36d5bea2392a7d3476dd797dc05602705cfa23ef) )

	ROM_REGION( 0x10000, REGION_CPU3, 0 )
	ROM_LOAD( "gyrussk.1a",   0x0000, 0x2000, CRC(f4ae1c17) SHA1(ae568c96a31d910afe30d2b7eeb9ed1ed07290e3) )
	ROM_LOAD( "gyrussk.2a",   0x2000, 0x2000, CRC(ba498115) SHA1(9cd1f42898cc590f39ba7cb3c975b0b3d3062eba) )
	/* the diagnostics ROM would go here */

	ROM_REGION( 0x1000, REGION_CPU4, 0 )	/* 8039 */
	ROM_LOAD( "gyrussk.3a",   0x0000, 0x1000, CRC(3f9b5dea) SHA1(6e807da02c2885b18e8cc2199f12f6be9040bf75) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "gyrussk.4",    0x0000, 0x2000, CRC(27d8329b) SHA1(564ff945465a23d93a93137ad277298770dfa06a) )

	ROM_REGION( 0x8000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "gyrussk.6",    0x0000, 0x2000, CRC(c949db10) SHA1(fcb8bcbd2bdd751fecb322a33c8a92fb6f07a7ab) )
	ROM_LOAD( "gyrussk.5",    0x2000, 0x2000, CRC(4f22411a) SHA1(763bcd039f8c1838a0d7da7d4dadc14a26e25596) )
	ROM_LOAD( "gyrussk.8",    0x4000, 0x2000, CRC(47cd1fbc) SHA1(8203c4ff0b1cd7b4dbc708e300bfeac1e7366e09) )
	ROM_LOAD( "gyrussk.7",    0x6000, 0x2000, CRC(8e8d388c) SHA1(8f2928d71c02aba977d67575d6e34d69bda2b9d4) )

	ROM_REGION( 0x0220, REGION_PROMS, 0 )
	ROM_LOAD( "gyrussk.pr3",  0x0000, 0x0020, CRC(98782db3) SHA1(b891e43b25187faca8002919ccb44d744daa3594) )	/* palette */
	ROM_LOAD( "gyrussk.pr1",  0x0020, 0x0100, CRC(7ed057de) SHA1(c04069ae1e2c62f9b3048844cd8cf5e1b03b7d3c) )	/* sprite lookup table */
	ROM_LOAD( "gyrussk.pr2",  0x0120, 0x0100, CRC(de823a81) SHA1(1af94b2a6a319a89b238a5076a2867f1cfd279b0) )	/* character lookup table */
ROM_END

ROM_START( venus )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "r1",           0x0000, 0x2000, CRC(d030abb1) SHA1(14a70e15f5df9ef957779771d8915203d3828532) )
	ROM_LOAD( "r2",           0x2000, 0x2000, CRC(dbf65d4d) SHA1(a0ad0dc3420442f06691bda2115fadd961ce86a7) )
	ROM_LOAD( "r3",           0x4000, 0x2000, CRC(db246fcd) SHA1(c0228b35591c9e1c778370a2abd3739c441f14aa) )
	/* the diagnostics ROM would go here */

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "gyrussk.9",    0xe000, 0x2000, CRC(822bf27e) SHA1(36d5bea2392a7d3476dd797dc05602705cfa23ef) )

	ROM_REGION( 0x10000, REGION_CPU3, 0 )
	ROM_LOAD( "gyrussk.1a",   0x0000, 0x2000, CRC(f4ae1c17) SHA1(ae568c96a31d910afe30d2b7eeb9ed1ed07290e3) )
	ROM_LOAD( "gyrussk.2a",   0x2000, 0x2000, CRC(ba498115) SHA1(9cd1f42898cc590f39ba7cb3c975b0b3d3062eba) )
	/* the diagnostics ROM would go here */

	ROM_REGION( 0x1000, REGION_CPU4, 0 )	/* 8039 */
	ROM_LOAD( "gyrussk.3a",   0x0000, 0x1000, CRC(3f9b5dea) SHA1(6e807da02c2885b18e8cc2199f12f6be9040bf75) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "gyrussk.4",    0x0000, 0x2000, CRC(27d8329b) SHA1(564ff945465a23d93a93137ad277298770dfa06a) )

	ROM_REGION( 0x8000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "gyrussk.6",    0x0000, 0x2000, CRC(c949db10) SHA1(fcb8bcbd2bdd751fecb322a33c8a92fb6f07a7ab) )
	ROM_LOAD( "gyrussk.5",    0x2000, 0x2000, CRC(4f22411a) SHA1(763bcd039f8c1838a0d7da7d4dadc14a26e25596) )
	ROM_LOAD( "gyrussk.8",    0x4000, 0x2000, CRC(47cd1fbc) SHA1(8203c4ff0b1cd7b4dbc708e300bfeac1e7366e09) )
	ROM_LOAD( "gyrussk.7",    0x6000, 0x2000, CRC(8e8d388c) SHA1(8f2928d71c02aba977d67575d6e34d69bda2b9d4) )

	ROM_REGION( 0x0220, REGION_PROMS, 0 )
	ROM_LOAD( "gyrussk.pr3",  0x0000, 0x0020, CRC(98782db3) SHA1(b891e43b25187faca8002919ccb44d744daa3594) )	/* palette */
	ROM_LOAD( "gyrussk.pr1",  0x0020, 0x0100, CRC(7ed057de) SHA1(c04069ae1e2c62f9b3048844cd8cf5e1b03b7d3c) )	/* sprite lookup table */
	ROM_LOAD( "gyrussk.pr2",  0x0120, 0x0100, CRC(de823a81) SHA1(1af94b2a6a319a89b238a5076a2867f1cfd279b0) )	/* character lookup table */
ROM_END


static DRIVER_INIT( gyruss )
{
	konami1_decode_cpu2();
}


GAME( 1983, gyruss,   0,      gyruss, gyruss,   gyruss, ROT90, "Konami", "Gyruss (Konami)", GAME_SUPPORTS_SAVE )
GAME( 1983, gyrussce, gyruss, gyruss, gyrussce, gyruss, ROT90, "Konami (Centuri license)", "Gyruss (Centuri)", GAME_SUPPORTS_SAVE )
GAME( 1983, venus,    gyruss, gyruss, gyrussce, gyruss, ROT90, "bootleg", "Venus", GAME_SUPPORTS_SAVE )
