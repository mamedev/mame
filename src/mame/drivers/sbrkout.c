/***************************************************************************

    Atari Super Breakout hardware

    driver by Mike Balfour

    Games supported:
        * Super Breakout

    Known issues:
        * none at this time

****************************************************************************

    Note:  I'm cheating a little bit with the paddle control.  The original
    game handles the paddle control as following.  The paddle is a potentiometer.
    Every VBlank signal triggers the start of a voltage ramp.  Whenever the
    ramp has the same value as the potentiometer, an NMI is generated.  In the
    NMI code, the current scanline value is used to calculate the value to
    put into location $1F in memory.  I cheat in this driver by just putting
    the paddle value directly into $1F, which has the same net result.

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

    CHANGES:
    MAB 05 MAR 99 - changed overlay support to use artwork functions


Stephh's notes (based on the games M6502 code and some tests) :

  - Each time the game is reset, it is set to "Cavity".
    I can't remember if it's the correct behaviour or not,
    but the VBLANK interruption is not called in "demo mode".
  - You can only select the game after 1st coin is inserted
    and before you press BUTTON1 to launch the first ball,
    then the VBLANK interruption is no more called.
    This means that player 2 plays the same game as player 1.

***************************************************************************/

#include "driver.h"
#include "sound/dac.h"
#include "includes/sbrkout.h"

#include "sbrkout.lh"


/*************************************
 *
 *  Temporary sound hardware
 *
 *************************************/

#define TIME_4V ATTOTIME_IN_USEC(4075000/4/1000)

static UINT8 *sbrkout_sound;

static WRITE8_HANDLER( sbrkout_dac_w )
{
	sbrkout_sound[offset]=data;
}


static TIMER_CALLBACK( sbrkout_tones_4V )
{
	static int vlines=0;

	if ((*sbrkout_sound) & vlines)
		DAC_data_w(0,255);
	else
		DAC_data_w(0,0);

	vlines = (vlines+1) % 16;
}


static MACHINE_RESET( sbrkout )
{
	timer_pulse(TIME_4V, NULL, 0, sbrkout_tones_4V);
}


/*************************************
 *
 *  Palette generation
 *
 *************************************/

static PALETTE_INIT( sbrkout )
{
	palette_set_color(machine,0,MAKE_RGB(0x00,0x00,0x00));
	palette_set_color(machine,1,MAKE_RGB(0xff,0xff,0xff));
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x001f, 0x001f) AM_READ(input_port_6_r) /* paddle value */
	AM_RANGE(0x0000, 0x00ff) AM_READ(MRA8_RAM) /* Zero Page RAM */
	AM_RANGE(0x0100, 0x01ff) AM_READ(MRA8_RAM) /* ??? */
	AM_RANGE(0x0400, 0x077f) AM_READ(MRA8_RAM) /* Video Display RAM */
	AM_RANGE(0x0828, 0x0828) AM_READ(sbrkout_select1_r) /* Select 1 */
	AM_RANGE(0x082f, 0x082f) AM_READ(sbrkout_select2_r) /* Select 2 */
	AM_RANGE(0x082e, 0x082e) AM_READ(input_port_5_r) /* Serve Switch */
	AM_RANGE(0x0830, 0x0833) AM_READ(sbrkout_read_DIPs_r) /* DIP Switches */
	AM_RANGE(0x0840, 0x0840) AM_READ(input_port_1_r) /* Coin Switches */
	AM_RANGE(0x0880, 0x0880) AM_READ(input_port_2_r) /* Start Switches */
	AM_RANGE(0x08c0, 0x08c0) AM_READ(input_port_3_r) /* Self Test Switch */
	AM_RANGE(0x0c00, 0x0c00) AM_READ(input_port_4_r) /* Vertical Sync Counter */
	AM_RANGE(0x2c00, 0x3fff) AM_READ(MRA8_ROM) /* PROGRAM */
	AM_RANGE(0xfff0, 0xffff) AM_READ(MRA8_ROM) /* PROM8 for 6502 vectors */
ADDRESS_MAP_END


static ADDRESS_MAP_START( writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0011, 0x0011) AM_WRITE(sbrkout_dac_w) AM_BASE(&sbrkout_sound) /* Noise Generation Bits */
	AM_RANGE(0x0010, 0x0014) AM_WRITE(MWA8_RAM) AM_BASE(&sbrkout_horiz_ram) /* Horizontal Ball Position */
	AM_RANGE(0x0018, 0x001d) AM_WRITE(MWA8_RAM) AM_BASE(&sbrkout_vert_ram) /* Vertical Ball Position / ball picture */
	AM_RANGE(0x0000, 0x00ff) AM_WRITE(MWA8_RAM) /* WRAM */
	AM_RANGE(0x0100, 0x01ff) AM_WRITE(MWA8_RAM) /* ??? */
	AM_RANGE(0x0400, 0x07ff) AM_WRITE(sbrkout_videoram_w) AM_BASE(&videoram) /* DISPLAY */
	AM_RANGE(0x0c10, 0x0c11) AM_WRITE(sbrkout_serve_led_w) /* Serve LED */
	AM_RANGE(0x0c30, 0x0c31) AM_WRITE(sbrkout_start_1_led_w) /* 1 Player Start Light */
	AM_RANGE(0x0c40, 0x0c41) AM_WRITE(sbrkout_start_2_led_w) /* 2 Player Start Light */
	AM_RANGE(0x0c50, 0x0c51) AM_WRITE(MWA8_RAM) /* NMI Pot Reading Enable */
	AM_RANGE(0x0c70, 0x0c71) AM_WRITE(MWA8_RAM) /* Coin Counter */
	AM_RANGE(0x0c80, 0x0c80) AM_WRITE(MWA8_NOP) /* Watchdog */
	AM_RANGE(0x0e00, 0x0e00) AM_WRITE(MWA8_NOP) /* IRQ Enable? */
	AM_RANGE(0x1000, 0x1000) AM_WRITE(MWA8_RAM) /* LSB of Pot Reading */
	AM_RANGE(0x2c00, 0x3fff) AM_WRITE(MWA8_ROM) /* PROM1-PROM8 */
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( sbrkout )
	PORT_START		/* DSW - fake port, gets mapped to Super Breakout ports */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(	0x00, DEF_STR( English ) )
	PORT_DIPSETTING(	0x01, DEF_STR( German ) )
	PORT_DIPSETTING(	0x02, DEF_STR( French ) )
	PORT_DIPSETTING(	0x03, DEF_STR( Spanish ) )
	PORT_DIPNAME( 0x0C, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING(	0x0C, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x70, 0x00, "Extended Play" )
	/* Progressive */
	PORT_DIPSETTING(	0x10, "200" )	PORT_CONDITION("SELECT",0x07,PORTCOND_EQUALS,0x01)
	PORT_DIPSETTING(	0x20, "400" )	PORT_CONDITION("SELECT",0x07,PORTCOND_EQUALS,0x01)
	PORT_DIPSETTING(	0x30, "600" )	PORT_CONDITION("SELECT",0x07,PORTCOND_EQUALS,0x01)
	PORT_DIPSETTING(	0x40, "900" )	PORT_CONDITION("SELECT",0x07,PORTCOND_EQUALS,0x01)
	PORT_DIPSETTING(	0x50, "1200" )	PORT_CONDITION("SELECT",0x07,PORTCOND_EQUALS,0x01)
	PORT_DIPSETTING(	0x60, "1600" )	PORT_CONDITION("SELECT",0x07,PORTCOND_EQUALS,0x01)
	PORT_DIPSETTING(	0x70, "2000" )	PORT_CONDITION("SELECT",0x07,PORTCOND_EQUALS,0x01)
	/* Double */
	PORT_DIPSETTING(	0x10, "200" )	PORT_CONDITION("SELECT",0x07,PORTCOND_EQUALS,0x02)
	PORT_DIPSETTING(	0x20, "400" )	PORT_CONDITION("SELECT",0x07,PORTCOND_EQUALS,0x02)
	PORT_DIPSETTING(	0x30, "600" )	PORT_CONDITION("SELECT",0x07,PORTCOND_EQUALS,0x02)
	PORT_DIPSETTING(	0x40, "800" )	PORT_CONDITION("SELECT",0x07,PORTCOND_EQUALS,0x02)
	PORT_DIPSETTING(	0x50, "1000" )	PORT_CONDITION("SELECT",0x07,PORTCOND_EQUALS,0x02)
	PORT_DIPSETTING(	0x60, "1200" )	PORT_CONDITION("SELECT",0x07,PORTCOND_EQUALS,0x02)
	PORT_DIPSETTING(	0x70, "1500" )	PORT_CONDITION("SELECT",0x07,PORTCOND_EQUALS,0x02)
	/* Cavity */
	PORT_DIPSETTING(	0x10, "200" )	PORT_CONDITION("SELECT",0x07,PORTCOND_EQUALS,0x04)
	PORT_DIPSETTING(	0x20, "300" )	PORT_CONDITION("SELECT",0x07,PORTCOND_EQUALS,0x04)
	PORT_DIPSETTING(	0x30, "400" )	PORT_CONDITION("SELECT",0x07,PORTCOND_EQUALS,0x04)
	PORT_DIPSETTING(	0x40, "700" )	PORT_CONDITION("SELECT",0x07,PORTCOND_EQUALS,0x04)
	PORT_DIPSETTING(	0x50, "900" )	PORT_CONDITION("SELECT",0x07,PORTCOND_EQUALS,0x04)
	PORT_DIPSETTING(	0x60, "1100" )	PORT_CONDITION("SELECT",0x07,PORTCOND_EQUALS,0x04)
	PORT_DIPSETTING(	0x70, "1400" )	PORT_CONDITION("SELECT",0x07,PORTCOND_EQUALS,0x04)
	PORT_DIPSETTING(	0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Lives ) )
	PORT_DIPSETTING(	0x80, "3" )
	PORT_DIPSETTING(	0x00, "5" )

	PORT_START		/* IN0 */
	PORT_BIT ( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT ( 0x80, IP_ACTIVE_HIGH, IPT_COIN2 )

	PORT_START		/* IN1 */
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START		/* IN2 */
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START		/* IN3 */
	PORT_BIT ( 0xFF, IP_ACTIVE_LOW, IPT_VBLANK )

	PORT_START		/* IN4 */
	PORT_BIT ( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START		/* IN5 */
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_REVERSE

	PORT_START_TAG("SELECT")		/* IN6 - fake port, used to set the game select dial */
	PORT_CONFNAME( 0x07, 0x01, "Game Select" )
	PORT_CONFSETTING(    0x01, "Progressive" )
	PORT_CONFSETTING(    0x02, "Double" )
	PORT_CONFSETTING(    0x04, "Cavity" )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	64,
	1,
	{ 0 },
	{ 4, 5, 6, 7, 0x200*8 + 4, 0x200*8 + 5, 0x200*8 + 6, 0x200*8 + 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static const gfx_layout balllayout =
{
	3,3,
	2,
	1,
	{ 0 },
	{ 0, 1, 2 },
	{ 0*8, 1*8, 2*8 },
	3*8
};


static GFXDECODE_START( sbrkout )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout, 0, 1 )
	GFXDECODE_ENTRY( REGION_GFX2, 0, balllayout, 0, 1 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( sbrkout )

	/* basic machine hardware */
	MDRV_CPU_ADD(M6502,375000) 	   /* 375 KHz? Should be 750KHz? */
	MDRV_CPU_PROGRAM_MAP(readmem,writemem)
	MDRV_CPU_VBLANK_INT(sbrkout_interrupt,1)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_REAL_60HZ_VBLANK_DURATION)

	MDRV_MACHINE_RESET(sbrkout)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 28*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 28*8-1)
	MDRV_GFXDECODE(sbrkout)
	MDRV_PALETTE_LENGTH(2)

	MDRV_PALETTE_INIT(sbrkout)
	MDRV_VIDEO_START(sbrkout)
	MDRV_VIDEO_UPDATE(sbrkout)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( sbrkout )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "033453.c1",    0x2800, 0x0800, CRC(a35d00e3) SHA1(53617ed1d362e82d6f45abd66056bffe23300e3b) )
	ROM_LOAD( "033454.d1",    0x3000, 0x0800, CRC(d42ea79a) SHA1(66c9b29226cde36d1ac6d1e81f34ebb5c79eded4) )
	ROM_LOAD( "033455.e1",    0x3800, 0x0800, CRC(e0a6871c) SHA1(1bdfa73d7b8d91e1c68b7847fc310cac314ee02d) )
	ROM_RELOAD(               0xf800, 0x0800 )

	ROM_REGION( 0x0400, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "033280.p4",    0x0000, 0x0200, CRC(5a69ce85) SHA1(ad9078d12495c350738bdb0b1e1b6120d9e01f60) )
	ROM_LOAD( "033281.r4",    0x0200, 0x0200, CRC(066bd624) SHA1(cfb86c7013a70b8375126b23a4e66df5f3b9186b) )

	ROM_REGION( 0x0020, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "033282.k6",    0x0000, 0x0020, CRC(6228736b) SHA1(bc176261dba11521df19d545ce604f8cc294287a) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "006400.m2",    0x0000, 0x0100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) )	/* sync (not used) */
	ROM_LOAD( "006401.e2",    0x0100, 0x0020, CRC(857df8db) SHA1(06313d5bde03220b2bc313d18e50e4bb1d0cfbbb) )	/* unknown */
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAMEL( 1978, sbrkout, 0, sbrkout, sbrkout, 0, ROT270, "Atari", "Super Breakout", 0, layout_sbrkout )
