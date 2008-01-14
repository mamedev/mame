/***************************************************************************

Irem Red Alert Driver

Everything in this driver is guesswork and speculation.  If something
seems wrong, it probably is.

If you have any questions about how this driver works, don't hesitate to
ask.  - Mike Balfour (mab22@po.cwru.edu)
***************************************************************************/

#include "driver.h"
#include "machine/6821pia.h"
#include "sound/ay8910.h"

/* video/redalert.c */
extern UINT8 *redalert_backram;
extern UINT8 *redalert_spriteram1;
extern UINT8 *redalert_spriteram2;
extern UINT8 *redalert_spriteram3;
extern UINT8 *redalert_characterram;
extern UINT8 *redalert_characterram2;
WRITE8_HANDLER( redalert_backram_w );
WRITE8_HANDLER( redalert_spriteram1_w );
WRITE8_HANDLER( redalert_spriteram2_w );
WRITE8_HANDLER( redalert_spriteram3_w );
WRITE8_HANDLER( redalert_spriteram4_w );
WRITE8_HANDLER( redalert_characterram_w );
WRITE8_HANDLER( redalert_characterram2_w );
extern VIDEO_UPDATE( redalert );
WRITE8_HANDLER( redalert_c040_w );
WRITE8_HANDLER( demoneye_c040_w );
WRITE8_HANDLER( redalert_backcolor_w );


/* audio/redalert.c */
WRITE8_HANDLER( redalert_c030_w );
READ8_HANDLER( redalert_voicecommand_r );
WRITE8_HANDLER( redalert_soundlatch_w );
READ8_HANDLER( redalert_AY8910_A_r );
WRITE8_HANDLER( redalert_AY8910_B_w );
WRITE8_HANDLER( redalert_AY8910_w );
READ8_HANDLER( redalert_sound_register_IC1_r );
WRITE8_HANDLER( redalert_sound_register_IC2_w );


static ADDRESS_MAP_START( redalert_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x01ff) AM_READ(MRA8_RAM) /* Zero page / stack */
	AM_RANGE(0x0200, 0x0fff) AM_READ(MRA8_RAM) /* ? */
	AM_RANGE(0x1000, 0x1fff) AM_READ(MRA8_RAM) /* Scratchpad video RAM */
	AM_RANGE(0x2000, 0x4fff) AM_READ(MRA8_RAM) /* Video RAM */
	AM_RANGE(0x5000, 0xbfff) AM_READ(MRA8_ROM)
	AM_RANGE(0xc100, 0xc100) AM_READ(input_port_0_r)
	AM_RANGE(0xc110, 0xc110) AM_READ(input_port_1_r)
	AM_RANGE(0xc120, 0xc120) AM_READ(input_port_2_r)
	AM_RANGE(0xc170, 0xc170) AM_READ(watchdog_reset_r)
	AM_RANGE(0xf000, 0xffff) AM_READ(MRA8_ROM) /* remapped ROM for 6502 vectors */
ADDRESS_MAP_END

static ADDRESS_MAP_START( redalert_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x01ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x0200, 0x0fff) AM_WRITE(MWA8_RAM) /* ? */
	AM_RANGE(0x1000, 0x1fff) AM_WRITE(MWA8_RAM) /* Scratchpad video RAM */
	AM_RANGE(0x2000, 0x3fff) AM_WRITE(redalert_backram_w) AM_BASE(&redalert_backram)
	AM_RANGE(0x4000, 0x43ff) AM_WRITE(MWA8_RAM) AM_BASE(&videoram) AM_SIZE(&videoram_size)
	AM_RANGE(0x4400, 0x47ff) AM_WRITE(redalert_spriteram1_w) AM_BASE(&redalert_spriteram1)
	AM_RANGE(0x4800, 0x4bff) AM_WRITE(redalert_characterram_w) AM_BASE(&redalert_characterram)
	AM_RANGE(0x4c00, 0x4fff) AM_WRITE(redalert_spriteram2_w) AM_BASE(&redalert_spriteram2)
	AM_RANGE(0x5000, 0xbfff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0xc130, 0xc130) AM_WRITE(redalert_c030_w)
	AM_RANGE(0xc140, 0xc140) AM_WRITE(redalert_c040_w)
	AM_RANGE(0xc150, 0xc150) AM_WRITE(redalert_backcolor_w)
	AM_RANGE(0xc160, 0xc160) AM_WRITE(redalert_soundlatch_w)
	AM_RANGE(0xf000, 0xffff) AM_WRITE(MWA8_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( redalert_sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x03ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x7800, 0x7fff) AM_READ(MRA8_ROM)
	AM_RANGE(0xf800, 0xffff) AM_READ(MRA8_ROM)
	AM_RANGE(0x1001, 0x1001) AM_READ(redalert_sound_register_IC1_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( redalert_sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x03ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x7800, 0x7fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0xf800, 0xffff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x1000, 0x1000) AM_WRITE(redalert_AY8910_w)
	AM_RANGE(0x1001, 0x1001) AM_WRITE(redalert_sound_register_IC2_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( redalert_voice_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x8000, 0x83ff) AM_READ(MRA8_RAM)
	AM_RANGE(0xc000, 0xc000) AM_READ(redalert_voicecommand_r) /* reads command from D0-D5? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( redalert_voice_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x8000, 0x83ff) AM_WRITE(MWA8_RAM)
ADDRESS_MAP_END


static ADDRESS_MAP_START( demoneye_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x01ff) AM_READ(MRA8_RAM) /* Zero page / stack */
	AM_RANGE(0x0200, 0x0fff) AM_READ(MRA8_RAM) /* ? */
	AM_RANGE(0x1000, 0x1fff) AM_READ(MRA8_RAM) /* Scratchpad video RAM */
	AM_RANGE(0x2000, 0x4fff) AM_READ(MRA8_RAM) /* Video RAM */
	AM_RANGE(0x5000, 0x5fff) AM_READ(MRA8_RAM)
	AM_RANGE(0x6000, 0xbfff) AM_READ(MRA8_ROM)
	AM_RANGE(0xc100, 0xc100) AM_READ(input_port_0_r)
	AM_RANGE(0xc110, 0xc110) AM_READ(input_port_1_r)
	AM_RANGE(0xc120, 0xc120) AM_READ(input_port_2_r)
	AM_RANGE(0xf000, 0xffff) AM_READ(MRA8_ROM) /* remapped ROM for 6502 vectors */
ADDRESS_MAP_END

static ADDRESS_MAP_START( demoneye_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x01ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x0200, 0x0fff) AM_WRITE(MWA8_RAM) /* ? */
	AM_RANGE(0x1000, 0x1fff) AM_WRITE(MWA8_RAM) /* Scratchpad video RAM */
	AM_RANGE(0x2000, 0x3fff) AM_WRITE(redalert_backram_w) AM_BASE(&redalert_backram)
	AM_RANGE(0x4000, 0x43ff) AM_WRITE(redalert_characterram_w) AM_BASE(&redalert_characterram)
	AM_RANGE(0x4400, 0x47ff) AM_WRITE(redalert_spriteram1_w) AM_BASE(&redalert_spriteram1)
	AM_RANGE(0x4800, 0x4bff) AM_WRITE(redalert_characterram2_w) AM_BASE(&redalert_characterram2)
	AM_RANGE(0x4c00, 0x4fff) AM_WRITE(redalert_spriteram2_w) AM_BASE(&redalert_spriteram2)
	AM_RANGE(0x5000, 0x53ff) AM_WRITE(MWA8_RAM) AM_BASE(&videoram) AM_SIZE(&videoram_size)
	AM_RANGE(0x5400, 0x57ff) AM_WRITE(redalert_spriteram3_w) AM_BASE(&redalert_spriteram3)
	AM_RANGE(0x5800, 0x5bff) AM_WRITE(MWA8_RAM) //???
	AM_RANGE(0x5c00, 0x5fff) AM_WRITE(MWA8_RAM) //???
	AM_RANGE(0x6000, 0xbfff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0xc130, 0xc130) AM_WRITE(MWA8_NOP)
	AM_RANGE(0xc140, 0xc140) AM_WRITE(demoneye_c040_w)
	AM_RANGE(0xc150, 0xc150) AM_WRITE(redalert_backcolor_w)
	AM_RANGE(0xc160, 0xc160) AM_WRITE(MWA8_NOP)
	AM_RANGE(0xc161, 0xc161) AM_WRITE(MWA8_NOP)
	AM_RANGE(0xc162, 0xc162) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0xc163, 0xc163) AM_WRITE(MWA8_NOP)
	AM_RANGE(0xc170, 0xc170) AM_WRITE(MWA8_NOP)
	AM_RANGE(0xf000, 0xffff) AM_WRITE(MWA8_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( demoneye_sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x007f) AM_READ(MRA8_RAM)
	AM_RANGE(0x0500, 0x0503) AM_READ(pia_0_r)
	AM_RANGE(0x2000, 0x2fff) AM_READ(MRA8_ROM)
	AM_RANGE(0xf000, 0xffff) AM_READ(MRA8_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( demoneye_sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x007f) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x0500, 0x0503) AM_WRITE(pia_0_w)
	AM_RANGE(0x2000, 0x2fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0xf000, 0xffff) AM_WRITE(MWA8_ROM)
ADDRESS_MAP_END


static INPUT_PORTS_START( redalert )
	PORT_START			   /* DIP Switches */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x00, "Cabinet in Service Mode" )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x08, "7000" )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )

	PORT_START			   /* IN1 */
	PORT_BIT ( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT ( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT ( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT ( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT ( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT ( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT ( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT ( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* Meter */

	PORT_START			   /* IN2  */
	PORT_BIT ( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT ( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* Meter */
	PORT_BIT ( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT ( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT ( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT ( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT ( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT ( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* Meter */

	PORT_START			   /* Fake input for coins */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
INPUT_PORTS_END


static INPUT_PORTS_START( demoneye )
	PORT_START			   /* DIP Switches */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x08, "7000" )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START			   /* IN1 */
	PORT_BIT ( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT ( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT ( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT ( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT ( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT ( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT ( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT ( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* Meter */

	PORT_START			   /* IN2  */
	PORT_BIT ( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT ( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* Meter */
	PORT_BIT ( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT ( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT ( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT ( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT ( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT ( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* Meter */

	PORT_START			   /* Fake input for coins */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
INPUT_PORTS_END


static const gfx_layout backlayout =
{
	8,8,	/* 8*8 characters */
	0x400,	  /* 1024 characters */
	1,	/* 1 bits per pixel */
	{ 0 }, /* No info needed for bit offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout charlayout =
{
	8,8,	/* 8*8 characters */
	128,	/* 128 characters */
	1,	/* 1 bits per pixel */
	{ 0 }, /* No info needed for bit offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	8,8,	/* 8*8 characters */
	128,	/* 128 characters */
	2,		/* 2 bits per pixel */
	{ 0, 0x800*8 }, /* No info needed for bit offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static GFXDECODE_START( redalert )
	GFXDECODE_ENTRY( 0, 0x3000, backlayout,	0, 8 ) 	/* the game dynamically modifies this */
	GFXDECODE_ENTRY( 0, 0x4800, charlayout,	0, 8 ) 	/* the game dynamically modifies this */
	GFXDECODE_ENTRY( 0, 0x4400, spritelayout,16, 4 ) 	/* the game dynamically modifies this */
GFXDECODE_END

static GFXDECODE_START( demoneye )
	GFXDECODE_ENTRY( 0, 0x3000, backlayout,	0, 8 ) 	/* the game dynamically modifies this */
	GFXDECODE_ENTRY( 0, 0x4800, spritelayout,16, 4 ) 	/* the game dynamically modifies this */
	GFXDECODE_ENTRY( 0, 0x4400, spritelayout,16, 4 ) 	/* the game dynamically modifies this */
	GFXDECODE_ENTRY( 0, 0x5400, spritelayout,16, 4 ) 	/* the game dynamically modifies this */
GFXDECODE_END

/* Arbitrary colortable */
static const UINT16 colortable_source[] =
{
	0,7,
	0,6,
	0,2,
	0,4,
	0,3,
	0,6,
	0,1,
	0,8,

	0,8,8,8,
	0,6,4,7,
	0,6,4,1,
	0,8,5,1,
};

static PALETTE_INIT( redalert )
{
	/* Arbitrary colors */
	palette_set_color(machine,0,MAKE_RGB(0x40,0x80,0xff));	/* Background */
	palette_set_color(machine,1,MAKE_RGB(0x00,0x00,0xff));	/* Blue */
	palette_set_color(machine,2,MAKE_RGB(0xff,0x00,0xff));	/* Magenta */
	palette_set_color(machine,3,MAKE_RGB(0x00,0xff,0xff));	/* Cyan */
	palette_set_color(machine,4,MAKE_RGB(0xff,0x00,0x00));	/* Red */
	palette_set_color(machine,5,MAKE_RGB(0xff,0x80,0x00));	/* Orange */
	palette_set_color(machine,6,MAKE_RGB(0xff,0xff,0x00));	/* Yellow */
	palette_set_color(machine,7,MAKE_RGB(0xff,0xff,0xff));	/* White */
	palette_set_color(machine,8,MAKE_RGB(0x00,0x00,0x00));	/* Black */

	memcpy(colortable,colortable_source,sizeof(colortable_source));
}

static INTERRUPT_GEN( redalert_interrupt )
{
	if( readinputport(3) )
	{
		cpunum_set_input_line(0, INPUT_LINE_NMI, PULSE_LINE);
	}
	else
	{
		cpunum_set_input_line(0, 0, HOLD_LINE);
	}
}

static const struct AY8910interface redalert_ay8910_interface =
{
	redalert_AY8910_A_r,		/* Port A Read */
	0,		/* Port B Read */
	0,		/* Port A Write */
	redalert_AY8910_B_w		/* Port B Write */
};

static MACHINE_DRIVER_START( redalert )

	/* basic machine hardware */
	MDRV_CPU_ADD(M6502, 1000000)	   /* ???? */
	MDRV_CPU_PROGRAM_MAP(redalert_readmem,redalert_writemem)
	MDRV_CPU_VBLANK_INT(redalert_interrupt,1)

	MDRV_CPU_ADD(M6502, 1000000)
	/* audio CPU */	   /* 1 MHz */
	MDRV_CPU_PROGRAM_MAP(redalert_sound_readmem,redalert_sound_writemem)
			/* IRQ is hooked to a 555 timer, whose freq is 1150 Hz */
	MDRV_CPU_PERIODIC_INT(irq0_line_hold,1150)

	MDRV_CPU_ADD(8085A, 1000000)
	/* audio CPU */	   /* 1 MHz? */
	MDRV_CPU_PROGRAM_MAP(redalert_voice_readmem,redalert_voice_writemem)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)
	MDRV_GFXDECODE(redalert)
	MDRV_PALETTE_LENGTH(9)
	MDRV_COLORTABLE_LENGTH(sizeof(colortable_source) / sizeof(colortable_source[0]))

	MDRV_PALETTE_INIT(redalert)
	MDRV_VIDEO_START(generic)
	MDRV_VIDEO_UPDATE(redalert)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD(AY8910, 2000000)
	MDRV_SOUND_CONFIG(redalert_ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


/* PIA 0, sound CPU */
static const pia6821_interface pia_0_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ 0, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, 0, 0, 0,
	/*irqs   : A/B             */ 0, 0
};

static MACHINE_START( demoneye )
{
	pia_config(0, &pia_0_intf);
}

static MACHINE_RESET( demoneye )
{
	pia_reset();
}

static MACHINE_DRIVER_START( demoneye )

	/* basic machine hardware */
	MDRV_CPU_ADD(M6502, 11730000/2)	/* 11.73MHz */
	MDRV_CPU_PROGRAM_MAP(demoneye_readmem,demoneye_writemem)
	MDRV_CPU_VBLANK_INT(redalert_interrupt,1)

	MDRV_CPU_ADD(M6802, 3579545)	/* 3.579545 MHz */
	/* audio CPU */
	MDRV_CPU_PROGRAM_MAP(demoneye_sound_readmem,demoneye_sound_writemem)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)

	MDRV_MACHINE_START(demoneye)
	MDRV_MACHINE_RESET(demoneye)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)
	MDRV_GFXDECODE(demoneye)
	MDRV_PALETTE_LENGTH(9)
	MDRV_COLORTABLE_LENGTH(sizeof(colortable_source) / sizeof(colortable_source[0]))

	MDRV_PALETTE_INIT(redalert)
	MDRV_VIDEO_START(generic)
	MDRV_VIDEO_UPDATE(redalert)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 3579545)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD(AY8910, 3579545)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( redalert )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "rag5",         	0x5000, 0x1000, CRC(d7c9cdd6) SHA1(5ff5cdceaa00083b745cf5c74b096f7edfadf737) )
	ROM_LOAD( "rag6",         	0x6000, 0x1000, CRC(cb2a308c) SHA1(9f3bc22bad31165e080e81d4a3fb0ec2aad235fe) )
	ROM_LOAD( "rag7n",        	0x7000, 0x1000, CRC(82ab2dae) SHA1(f8328b048384afac245f1c16a2d0864ffe0b4741) )
	ROM_LOAD( "rag8n",        	0x8000, 0x1000, CRC(b80eece9) SHA1(d986449bdb1d94832187c7f953f01330391ef4c9) )
	ROM_RELOAD(                 0xf000, 0x1000 )
	ROM_LOAD( "rag9",         	0x9000, 0x1000, CRC(2b7d1295) SHA1(1498af0c55bd38fe79b91afc38921085102ebbc3) )
	ROM_LOAD( "ragab",        	0xa000, 0x1000, CRC(ab99f5ed) SHA1(a93713bb03d61cce64adc89b874b67adea7c53cd) )
	ROM_LOAD( "ragb",         	0xb000, 0x1000, CRC(8e0d1661) SHA1(bff4ddca761ddd70113490f50777e62c66813685) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "w3s1",         	0x7800, 0x0800, CRC(4af956a5) SHA1(25368a40d7ebc60316fd2d78ec4c686e701b96dc) )
	ROM_RELOAD(                0xf800, 0x0800 )

	ROM_REGION( 0x10000, REGION_CPU3, 0 )
	ROM_LOAD( "ras1b",        	0x0000, 0x1000, CRC(ec690845) SHA1(26a84738bd45ed21dac6c8383ebd9c3b9831024a) )
	ROM_LOAD( "ras2",         	0x1000, 0x1000, CRC(fae94cfc) SHA1(2fd798706bb3afda3fb55bc877e597cc4e5d0c15) )
	ROM_LOAD( "ras3",         	0x2000, 0x1000, CRC(20d56f3e) SHA1(5c32ee3365407e6d3f7ab5662e9ecbac437ed4cb) )
	ROM_LOAD( "ras4",         	0x3000, 0x1000, CRC(130e66db) SHA1(385b8f889fee08fddbb2f75a691af569109eacd1) )

	ROM_REGION( 0x0200, REGION_PROMS, 0 ) /* unknow */
	ROM_LOAD( "m-257sc.1a",		0x0000, 0x0200, CRC(b1aca792) SHA1(db37f99b9880cc3c434e2a55a0bbb017d9a72aa3) )
ROM_END


/********************************************************************
IREM 'DEMONEYE-X' proto 1981

proto sound board

8910
6821   8910
6802  sound6 sound7
    3.579545MHz

 main board M-27M-C

  11.73MHz              6502            x x x xx
                                         x x x  on
                  4116            8
                  4116            -
                  4116            9
                  4116            6
                  4116            A
                  4116            7
                  4116            B
                  4116

 sub board 1 M-27Sb

  1a2

  2114
  2114
              2114 <- two parts piggy-backed
              2114 <- two parts piggy-backed
              2114 2114
              2114 2114

sub board 2 M-42-S

  1a       clr(missing)

                  2114
                  2114
                  2114
                  2114
                  2114
                  2114

*********************************************************************/

ROM_START( demoneye )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "demoneye.6",  0x6000, 0x1000, CRC(b03ee3a9) SHA1(66b6115fbb4e8097152702022c59c464e8211e5a) )
	ROM_LOAD( "demoneye.7",  0x7000, 0x1000, CRC(667a5de7) SHA1(c3ce7fbbc6c98250e9d5f85854e6887017ca5ff9) )
	ROM_LOAD( "demoneye.8",  0x8000, 0x1000, CRC(257484d7) SHA1(3937cce546462a471adbdc1da63ddfc20cfc7b79) )
	ROM_RELOAD(              0xf000, 0x1000 )
	ROM_LOAD( "demoneye.9",  0x9000, 0x1000, CRC(bd8d79a8) SHA1(68c1443ef78b545eb9e612573b86515c3ad7f103) )
	ROM_LOAD( "demoneye.a",  0xa000, 0x1000, CRC(a27d08aa) SHA1(659ad22778e852fc58f3951d62bc01151c973d36) )
	ROM_LOAD( "demoneye.b",  0xb000, 0x1000, CRC(1fd3585b) SHA1(b1697b7b21b739499fda1e155530dbfab89f3358) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "demoneye.7s", 0x2000, 0x1000, CRC(8fdc9364) SHA1(3fccb5b22f08d6a0cde85863c1ce5399c84f233e) )
	ROM_LOAD( "demoneye.6s", 0xf000, 0x1000, CRC(0a23def9) SHA1(b52f52be312ec7810e3c9cbd3913e887f983b1ee) )

	ROM_REGION( 0x0400, REGION_USER1, 0 ) /* unknow */
	ROM_LOAD( "demoneye.1a",  0x0000, 0x0200, CRC(d03488ea) SHA1(11027f502ad2a9255b2e5611ab2eee16ede1d704) )
	ROM_LOAD( "demoneye.1a2", 0x0200, 0x0200, CRC(eaf5a66e) SHA1(d8ebe05ba5d75fbf6ad45f710e5bd27b6afad44b) )
ROM_END

GAME( 1981, redalert, 0, redalert, redalert, 0, ROT270, "Irem + GDI",	"Red Alert",  GAME_WRONG_COLORS | GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1981, demoneye, 0, demoneye, demoneye, 0, ROT270, "Irem",		"Demoneye-X", GAME_WRONG_COLORS | GAME_IMPERFECT_GRAPHICS | GAME_NO_SOUND )
