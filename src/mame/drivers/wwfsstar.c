/*******************************************************************************
 WWF Superstars (C) 1989 Technos Japan  (drivers/wwfsstar.c)
********************************************************************************
 driver by David Haywood

 Special Thanks to:

 Richard Bush & the Rest of the Raine Team - Raine's WWF Superstars driver on
 which most of this driver has been based.

********************************************************************************

 Hardware:

 Primary CPU : 68000

 Sound CPUs : Z80

 Sound Chips : YM2151, M6295

 3 Layers from now on if mentioned will be refered to as

 BG0 - Background
 SPR - Sprites
 FG0 - Foreground / Text Layer

********************************************************************************

 Change Log:
 03 Jun 2005 - Pierpaolo Prazzoli
             | Fixed VBlank (i disagree ;-)
             | Fixed some bad sprites
 04 Mar 2002 | Fixed Dip Switches and Inputs    (Steph)
             | Fixed screen flipping by using similar routine to the one
             | in src/video/wwfwfest.c        (Steph)
 18 Jun 2001 | Changed Interrupt Function .. its not fully understood whats
             | is meant to be going on ..
 15 Jun 2001 | Cleaned up Sprite Drawing a bit, correcting some clipping probs,
             | mapped DSW's
 15 Jun 2001 | First Submission of Driver,
 14 Jun 2001 | Started Driver, using Raine Source as a reference for getting it
             | up and running

********************************************************************************

 Notes:

 - Scrolling *might* be slightly off, i'm not sure

*******************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"

/* in (video/wwfsstar.c) */
VIDEO_START( wwfsstar );
VIDEO_UPDATE( wwfsstar );
WRITE16_HANDLER( wwfsstar_fg0_videoram_w );
WRITE16_HANDLER( wwfsstar_bg0_videoram_w );

extern UINT16 *wwfsstar_fg0_videoram, *wwfsstar_bg0_videoram;

static READ16_HANDLER( input_port_2_word_r_cust );
static WRITE16_HANDLER( wwfsstar_irqack_w );
static WRITE16_HANDLER( wwfsstar_flipscreen_w );
static WRITE16_HANDLER ( wwfsstar_soundwrite );
static WRITE16_HANDLER ( wwfsstar_scrollwrite );

static int vblank;

/*******************************************************************************
 Memory Maps
********************************************************************************
 Pretty Straightforward
*******************************************************************************/

static ADDRESS_MAP_START( readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_READ(SMH_ROM)	/* Rom */
	AM_RANGE(0x080000, 0x080fff) AM_READ(SMH_RAM)	/* FG0 Ram */
	AM_RANGE(0x0c0000, 0x0c0fff) AM_READ(SMH_RAM)	/* BG0 Ram */
	AM_RANGE(0x100000, 0x1003ff) AM_READ(SMH_RAM)	/* SPR Ram */
	AM_RANGE(0x180000, 0x180001) AM_READ(input_port_3_word_r)	/* DSW0 */
	AM_RANGE(0x180002, 0x180003) AM_READ(input_port_4_word_r)	/* DSW1 */
	AM_RANGE(0x180004, 0x180005) AM_READ(input_port_0_word_r)	/* CTRLS0 */
	AM_RANGE(0x180006, 0x180007) AM_READ(input_port_1_word_r)	/* CTRLS1 */
	AM_RANGE(0x180008, 0x180009) AM_READ(input_port_2_word_r_cust)	/* MISC */
	AM_RANGE(0x1c0000, 0x1c3fff) AM_READ(SMH_RAM)	/* Work Ram */
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_WRITE(SMH_ROM)	/* Rom */
	AM_RANGE(0x080000, 0x080fff) AM_WRITE(wwfsstar_fg0_videoram_w) AM_BASE(&wwfsstar_fg0_videoram)	/* FG0 Ram */
	AM_RANGE(0x0c0000, 0x0c0fff) AM_WRITE(wwfsstar_bg0_videoram_w) AM_BASE(&wwfsstar_bg0_videoram)	/* BG0 Ram */
	AM_RANGE(0x100000, 0x1003ff) AM_WRITE(SMH_RAM) AM_BASE(&spriteram16)	/* SPR Ram */
	AM_RANGE(0x140000, 0x140fff) AM_WRITE(paletteram16_xxxxBBBBGGGGRRRR_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0x180000, 0x180003) AM_WRITE(wwfsstar_irqack_w)
	AM_RANGE(0x180004, 0x180007) AM_WRITE(wwfsstar_scrollwrite)
	AM_RANGE(0x180008, 0x180009) AM_WRITE(wwfsstar_soundwrite)
	AM_RANGE(0x18000a, 0x18000b) AM_WRITE(wwfsstar_flipscreen_w)
	AM_RANGE(0x1c0000, 0x1c3fff) AM_WRITE(SMH_RAM)	/* Work Ram */
ADDRESS_MAP_END

static ADDRESS_MAP_START( readmem_sound, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(SMH_ROM)
	AM_RANGE(0x8000, 0x87ff) AM_READ(SMH_RAM)
	AM_RANGE(0x8801, 0x8801) AM_READ(YM2151_status_port_0_r)
	AM_RANGE(0x9800, 0x9800) AM_READ(OKIM6295_status_0_r)
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem_sound, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(SMH_ROM)
	AM_RANGE(0x8000, 0x87ff) AM_WRITE(SMH_RAM)
	AM_RANGE(0x8800, 0x8800) AM_WRITE(YM2151_register_port_0_w)
	AM_RANGE(0x8801, 0x8801) AM_WRITE(YM2151_data_port_0_w)
	AM_RANGE(0x9800, 0x9800) AM_WRITE(OKIM6295_data_0_w)
ADDRESS_MAP_END


/*******************************************************************************
 Read / Write Handlers
********************************************************************************
 as used by the above memory map
*******************************************************************************/

int wwfsstar_scrollx, wwfsstar_scrolly; /* used in (video/wwfsstar.c) */

static WRITE16_HANDLER ( wwfsstar_scrollwrite )
{
	switch (offset)
	{
		case 0x00:
			wwfsstar_scrollx = data;
			break;
		case 0x01:
			wwfsstar_scrolly = data;
			break;
	}
}

static WRITE16_HANDLER ( wwfsstar_soundwrite )
{
	soundlatch_w(machine,1,data & 0xff);
	cpunum_set_input_line(machine, 1, INPUT_LINE_NMI, PULSE_LINE );
}

static WRITE16_HANDLER( wwfsstar_flipscreen_w )
{
	flip_screen_set(data & 1);
}

static WRITE16_HANDLER( wwfsstar_irqack_w )
{
	if(offset == 0)
		cpunum_set_input_line(Machine, 0, 6, CLEAR_LINE);

	else
		cpunum_set_input_line(Machine, 0, 5, CLEAR_LINE);
}

/*
    Interrupt behaviour verified from actual PCB.

    After the post third match intermission, there's a tight loop
    which polls the vblank input bit until it is active.
    The subsequent vblank ISR does not complete during the vblank
    duration. On the real PCB, the 68000 would catch the active
    vblank value before the interrupt was taken. The MAME
    implementation does not and thus hangs.

    A hack is required: raise the vblank bit a scanline early.
*/

static INTERRUPT_GEN( wwfsstars_interrupt )
{
	int scanline = 271 - cpu_getiloops();

	/* Vblank is lowered on scanline 0 (8) */
	if (scanline == 0)
	{
		vblank = 0;
	}
	/* Hack */
	else if (scanline==239)
	{
		vblank = 1;
	}
	/* Vblank is raised on scanline 240 (248) */
	else if (scanline==240)
	{
		video_screen_update_partial(machine->primary_screen, scanline);
		cpunum_set_input_line(machine, 0, 6, ASSERT_LINE);
	}

	/* An interrupt is generated every 16 scanlines */
	if (scanline%16 == 0)
	{
		video_screen_update_partial(machine->primary_screen, scanline);
		cpunum_set_input_line(machine, 0, 5, ASSERT_LINE);
	}
}

static READ16_HANDLER( input_port_2_word_r_cust )
{
	return readinputport(2) | vblank;
}

/*******************************************************************************
 Input Ports
********************************************************************************
 2 Sets of Player Controls
 A Misc Inputs inc. Coins
 2 Sets of Dipswitches
*******************************************************************************/

static INPUT_PORTS_START( wwfsstar )
	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Button A (1P VS CPU - Power Up)")

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_START3 ) PORT_NAME("Button C (1P/2P VS CPU)")
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Button B (1P VS 2P - Buy-in)")

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SPECIAL ) /* VBlank */
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* DSW0 */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00,  DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01,  DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02,  DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07,  DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06,  DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05,  DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04,  DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03,  DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00,  DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08,  DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10,  DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38,  DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30,  DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28,  DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20,  DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18,  DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x40, 0x40, "Unused SW 0-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* DSW1 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Super Techniques" )			// Check code at 0x014272
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x30, 0x30, "Time" )
	PORT_DIPSETTING(    0x20, "+2:30" )
	PORT_DIPSETTING(    0x30, "Default" )
	PORT_DIPSETTING(    0x10, "-2:30" )
	PORT_DIPSETTING(    0x00, "-5:00" )
	PORT_DIPNAME( 0x40, 0x40, "Unused SW 1-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Health For Winning" )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END

/*******************************************************************************
 Graphic Decoding
********************************************************************************
 Tiles are decoded the same as Double Dragon, Strangely Enough another
 Technos Game ;)
*******************************************************************************/

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 2, 4, 6 },
	{ 1, 0, 8*8+1, 8*8+0, 16*8+1, 16*8+0, 24*8+1, 24*8+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	32*8
};

static const gfx_layout tiles16x16_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0, 4 },
	{ 3, 2, 1, 0, 16*8+3, 16*8+2, 16*8+1, 16*8+0,
		  32*8+3, 32*8+2, 32*8+1, 32*8+0, 48*8+3, 48*8+2, 48*8+1, 48*8+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		  8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	64*8
};

static GFXDECODE_START( wwfsstar )
	GFXDECODE_ENTRY( REGION_GFX1, 0, tiles8x8_layout,     0, 16 )	/* colors   0-255 */
	GFXDECODE_ENTRY( REGION_GFX2, 0, tiles16x16_layout, 128, 16 )	/* colors   128-383 */
	GFXDECODE_ENTRY( REGION_GFX3, 0, tiles16x16_layout, 256,  8 )	/* colors   256-383 */
GFXDECODE_END


/*******************************************************************************
 Sound Stuff..
********************************************************************************
 Straight from Ddragon 3
*******************************************************************************/

static void wwfsstar_ymirq_handler(int irq)
{
	cpunum_set_input_line(Machine, 1, 0 , irq ? ASSERT_LINE : CLEAR_LINE );
}

static const struct YM2151interface ym2151_interface =
{
	wwfsstar_ymirq_handler
};

/*******************************************************************************
 Machine Driver(s)
*******************************************************************************/

static MACHINE_DRIVER_START( wwfsstar )

	/* basic machine hardware */
	MDRV_CPU_ADD(M68000, 10000000)
	MDRV_CPU_PROGRAM_MAP(readmem,writemem)
	MDRV_CPU_VBLANK_INT_HACK(wwfsstars_interrupt,272)

	MDRV_CPU_ADD(Z80, 3579545)
	MDRV_CPU_PROGRAM_MAP(readmem_sound,writemem_sound)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(57.44)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(wwfsstar)
	MDRV_PALETTE_LENGTH(384)

	MDRV_VIDEO_START(wwfsstar)
	MDRV_VIDEO_UPDATE(wwfsstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(YM2151, 3579545)
	MDRV_SOUND_CONFIG(ym2151_interface)
	MDRV_SOUND_ROUTE(0, "left", 0.45)
	MDRV_SOUND_ROUTE(1, "right", 0.45)

	MDRV_SOUND_ADD(OKIM6295, 1122000)
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.47)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.47)
MACHINE_DRIVER_END

/*******************************************************************************
 Rom Loaders / Game Drivers
*******************************************************************************/

ROM_START( wwfsstar )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* Main CPU  (68000) */
	ROM_LOAD16_BYTE( "24ac-0_j-1.34", 0x00000, 0x20000, CRC(ec8fd2c9) SHA1(04ab93e2a1becdc480750c3b55839328b2af4639) )
	ROM_LOAD16_BYTE( "24ad-0_j-1.35", 0x00001, 0x20000, CRC(54e614e4) SHA1(ee924dea977606fcb1222d1aa89211994126a182)  )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Sound CPU (Z80)  */
	ROM_LOAD( "b.12",    0x00000, 0x08000, CRC(1e44f8aa) SHA1(e03857d6954e9b9b6073b211e2d6570032af8807) )

	ROM_REGION( 0x40000, REGION_SOUND1, 0 )	/* ADPCM samples */
	ROM_LOAD( "24a9.46",	   0x00000, 0x20000, CRC(703ff08f) SHA1(08c4d33208eb4c76c751a1a0fe16a817bdc30820) )
	ROM_LOAD( "wwfs03.bin",    0x20000, 0x10000, CRC(8a35a20e) SHA1(3bc1a43f956b6840a4bee9e8fb2a6e3d4ac18f75) )
	ROM_LOAD( "wwfs05.bin",    0x30000, 0x10000, CRC(6df08962) SHA1(e3dec81644fe5867024a2fcf34a67924622f3a5b) )

	ROM_REGION( 0x20000, REGION_GFX1, ROMREGION_DISPOSE ) /* FG0 Tiles (8x8) */
	/* this rom may not be correct for this set.. the rom in the set had half the data missing and only a 99%
       match with wwfsstau for the first part */
	ROM_LOAD( "24a4-0.58",    0x00000, 0x20000, BAD_DUMP CRC(cb12ba40) SHA1(2d39f778d9daf0d3606b63975bd6cfc45847a265) )

	/* these are bootleg roms ...  the original has mask roms */
	ROM_REGION( 0x200000, REGION_GFX2, ROMREGION_DISPOSE ) /* SPR Tiles (16x16) */
	ROM_LOAD( "wwfs39.bin",    0x000000, 0x010000, CRC(d807b09a) SHA1(e5a221ac57e16cb3fb47d986e62f265ebbc5b0e6) )
	ROM_LOAD( "wwfs38.bin",    0x010000, 0x010000, CRC(d8ea94d3) SHA1(3a9e200dbcd456364317858e4b5fa6a149cb3c61) )
	ROM_LOAD( "wwfs37.bin",    0x020000, 0x010000, CRC(5e8d7407) SHA1(829cc0c2013138097aa49c9072b87452bf8c8936) )
	ROM_LOAD( "wwfs36.bin",    0x030000, 0x010000, CRC(9005e942) SHA1(d0276419c21b866e17be85382f4e6f3baa4ce40b) )
	ROM_LOAD( "wwfs43.bin",    0x040000, 0x010000, CRC(aafc4a38) SHA1(ac48f13fc4d51e425748190f68b32c099acf532d) )
	ROM_LOAD( "wwfs42.bin",    0x050000, 0x010000, CRC(e48b88fb) SHA1(0fbf9109b86fc6376b8705d28c4c3aeb7fb9cdd8) )
	ROM_LOAD( "wwfs41.bin",    0x060000, 0x010000, CRC(ed7f69d5) SHA1(ae11aad3af43a0e240d17f4db26d68eaae7f1cf0) )
	ROM_LOAD( "wwfs40.bin",    0x070000, 0x010000, CRC(4d75fd89) SHA1(76a1f4a169648e00fcb150157393e3a45613f232) )
	ROM_LOAD( "wwfs19.bin",    0x080000, 0x010000, CRC(7426d444) SHA1(1c1af9492bb711701100bfcecab35f0c38260756) )
	ROM_LOAD( "wwfs18.bin",    0x090000, 0x010000, CRC(af11ad2a) SHA1(4214b16ada1679c6e18c5f2b6e5d6ddb4b731361) )
	ROM_LOAD( "wwfs17.bin",    0x0a0000, 0x010000, CRC(ef12069f) SHA1(5748646c0b0d6e00b6eea26fda3a3699e1553473) )
	ROM_LOAD( "wwfs16.bin",    0x0b0000, 0x010000, CRC(08343e7f) SHA1(2085350e2506cf2d9c7aa74211cca912b36b203d) )
	ROM_LOAD( "wwfs15.bin",    0x0c0000, 0x010000, CRC(aac5a928) SHA1(1298a5d29b388768ed6508522830e02f95fb54fc) )
	ROM_LOAD( "wwfs14.bin",    0x0d0000, 0x010000, CRC(67eb7bea) SHA1(1de39072f96a80a41c383e495bb686adb353586c) )
	ROM_LOAD( "wwfs13.bin",    0x0e0000, 0x010000, CRC(970b6e76) SHA1(c0da2237f759980d2d879c55c6855633c99fc418) )
	ROM_LOAD( "wwfs12.bin",    0x0f0000, 0x010000, CRC(242caff5) SHA1(9e2a836d9c5415c9313e6609a2eebcb661fa0301) )
	ROM_LOAD( "wwfs27.bin",    0x100000, 0x010000, CRC(f3eb8ab9) SHA1(4032f96d9c738706e353af7f00de921c2c1b72be) )
	ROM_LOAD( "wwfs26.bin",    0x110000, 0x010000, CRC(2ca91eaf) SHA1(191512aaf9542cbbd441886455cbfb5e7a0ab5d4) )
	ROM_LOAD( "wwfs25.bin",    0x120000, 0x010000, CRC(bbf69c6a) SHA1(c9502c9f1fa257f506a4aed22c015524a9fca074) )
	ROM_LOAD( "wwfs24.bin",    0x130000, 0x010000, CRC(76b08bcd) SHA1(c60bc47cf172203e570e693244a1c6308fa36f0b) )
	ROM_LOAD( "wwfs23.bin",    0x140000, 0x010000, CRC(681f5b5e) SHA1(17ac4dbfa84f5161f8d1c740ee91ccecf9f83f5f) )
	ROM_LOAD( "wwfs22.bin",    0x150000, 0x010000, CRC(81fe1bf7) SHA1(37102a6d276907bfeaccc81f1d6693e1c1f26cce) )
	ROM_LOAD( "wwfs21.bin",    0x160000, 0x010000, CRC(c52eee5e) SHA1(6bf7c63b3c18487dd7d964fe05cef348c6069775) )
	ROM_LOAD( "wwfs20.bin",    0x170000, 0x010000, CRC(b2a8050e) SHA1(6db9463321973a3141b6ceda35d11f851d0b9e1f) )
	ROM_LOAD( "wwfs35.bin",    0x180000, 0x010000, CRC(9d648d82) SHA1(81be2ca9f8384b29cf6ce9d59dedf8be1f37fd5d) )
	ROM_LOAD( "wwfs34.bin",    0x190000, 0x010000, CRC(742a79db) SHA1(5c2a5b578817ea1ed8b6993a8bc554840d7302a9) )
	ROM_LOAD( "wwfs33.bin",    0x1a0000, 0x010000, CRC(f6923db6) SHA1(5d0aba7f8e3fbde890ef67e91dbdd2bd3e67a23c) )
	ROM_LOAD( "wwfs32.bin",    0x1b0000, 0x010000, CRC(9becd621) SHA1(200c485d4d5acaf55f47d716a0df3218b64f813a) )
	ROM_LOAD( "wwfs31.bin",    0x1c0000, 0x010000, CRC(f94c74d5) SHA1(8f740860562876bd21a47ba8be758ecd6913207c) )
	ROM_LOAD( "wwfs30.bin",    0x1d0000, 0x010000, CRC(94094518) SHA1(e010b211ea9c08a3c1f36a0e04f2c4320acaa2b7) )
	ROM_LOAD( "wwfs29.bin",    0x1e0000, 0x010000, CRC(7b5b9d83) SHA1(e7381e48a3a63f28fc9a997bfda3e612f4fcccf9) )
	ROM_LOAD( "wwfs28.bin",    0x1f0000, 0x010000, CRC(70fda626) SHA1(049ef67f57953266ef2c750f58c0ee9baf963b39) )

	ROM_REGION( 0x80000, REGION_GFX3, ROMREGION_DISPOSE ) /* BG0 Tiles (16x16) */
	ROM_LOAD( "wwfs51.bin",    0x00000, 0x10000, CRC(51157385) SHA1(fa9f74ace9432d8686402e410cbc03a8c3b86f4d) )
	ROM_LOAD( "wwfs50.bin",    0x10000, 0x10000, CRC(7fc79df5) SHA1(c57e8bb55a1d176b9232395207c5a28c622de9a4) )
	ROM_LOAD( "wwfs49.bin",    0x20000, 0x10000, CRC(a14076b0) SHA1(6817f56d2c6e2d596ebc7827d816ad331b425eeb) )
	ROM_LOAD( "wwfs48.bin",    0x30000, 0x10000, CRC(251372fd) SHA1(e6036807c902fb34071da8287dedcef6cadae06a) )
	ROM_LOAD( "wwfs47.bin",    0x40000, 0x10000, CRC(6fd7b6ea) SHA1(7e77e7647153bcaf09e1002b03f851fe474925a2) )
	ROM_LOAD( "wwfs46.bin",    0x50000, 0x10000, CRC(985e5180) SHA1(9fd8b1ae844a2be465748e3a95ea24aa032e490d) )
	ROM_LOAD( "wwfs45.bin",    0x60000, 0x10000, CRC(b2fad792) SHA1(083977c041c42c50e4f1f7140d97a7b792f768e9) )
	ROM_LOAD( "wwfs44.bin",    0x70000, 0x10000, CRC(4f965fa9) SHA1(4312838e216d2a90fe413d027f46d77c74a0aa07) )
ROM_END

ROM_START( wwfsstau )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* Main CPU  (68000) */
	ROM_LOAD16_BYTE( "24ac-04.34", 0x00000, 0x20000, CRC(ee9b850e) SHA1(6b634ad98b6104b9e860d05e73f3a139c2a19a78) )
	ROM_LOAD16_BYTE( "24ad-04.35", 0x00001, 0x20000, CRC(057c2eef) SHA1(6eb5f60fa51b3e7f17fc6a81182a01ea406febea) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Sound CPU (Z80)  */
	ROM_LOAD( "b.12",    0x00000, 0x08000, CRC(1e44f8aa) SHA1(e03857d6954e9b9b6073b211e2d6570032af8807) )

	ROM_REGION( 0x40000, REGION_SOUND1, 0 )	/* ADPCM samples */
	ROM_LOAD( "24a9.46",	   0x00000, 0x20000, CRC(703ff08f) SHA1(08c4d33208eb4c76c751a1a0fe16a817bdc30820) )
	ROM_LOAD( "wwfs03.bin",    0x20000, 0x10000, CRC(8a35a20e) SHA1(3bc1a43f956b6840a4bee9e8fb2a6e3d4ac18f75) )
	ROM_LOAD( "wwfs05.bin",    0x30000, 0x10000, CRC(6df08962) SHA1(e3dec81644fe5867024a2fcf34a67924622f3a5b) )

	ROM_REGION( 0x20000, REGION_GFX1, ROMREGION_DISPOSE ) /* FG0 Tiles (8x8) */
	ROM_LOAD( "24a4-0.58",    0x00000, 0x20000, CRC(cb12ba40) SHA1(2d39f778d9daf0d3606b63975bd6cfc45847a265) )

	/* these are bootleg roms ...  the original has mask roms */
	ROM_REGION( 0x200000, REGION_GFX2, ROMREGION_DISPOSE ) /* SPR Tiles (16x16) */
	ROM_LOAD( "wwfs39.bin",    0x000000, 0x010000, CRC(d807b09a) SHA1(e5a221ac57e16cb3fb47d986e62f265ebbc5b0e6) )
	ROM_LOAD( "wwfs38.bin",    0x010000, 0x010000, CRC(d8ea94d3) SHA1(3a9e200dbcd456364317858e4b5fa6a149cb3c61) )
	ROM_LOAD( "wwfs37.bin",    0x020000, 0x010000, CRC(5e8d7407) SHA1(829cc0c2013138097aa49c9072b87452bf8c8936) )
	ROM_LOAD( "wwfs36.bin",    0x030000, 0x010000, CRC(9005e942) SHA1(d0276419c21b866e17be85382f4e6f3baa4ce40b) )
	ROM_LOAD( "wwfs43.bin",    0x040000, 0x010000, CRC(aafc4a38) SHA1(ac48f13fc4d51e425748190f68b32c099acf532d) )
	ROM_LOAD( "wwfs42.bin",    0x050000, 0x010000, CRC(e48b88fb) SHA1(0fbf9109b86fc6376b8705d28c4c3aeb7fb9cdd8) )
	ROM_LOAD( "wwfs41.bin",    0x060000, 0x010000, CRC(ed7f69d5) SHA1(ae11aad3af43a0e240d17f4db26d68eaae7f1cf0) )
	ROM_LOAD( "wwfs40.bin",    0x070000, 0x010000, CRC(4d75fd89) SHA1(76a1f4a169648e00fcb150157393e3a45613f232) )
	ROM_LOAD( "wwfs19.bin",    0x080000, 0x010000, CRC(7426d444) SHA1(1c1af9492bb711701100bfcecab35f0c38260756) )
	ROM_LOAD( "wwfs18.bin",    0x090000, 0x010000, CRC(af11ad2a) SHA1(4214b16ada1679c6e18c5f2b6e5d6ddb4b731361) )
	ROM_LOAD( "wwfs17.bin",    0x0a0000, 0x010000, CRC(ef12069f) SHA1(5748646c0b0d6e00b6eea26fda3a3699e1553473) )
	ROM_LOAD( "wwfs16.bin",    0x0b0000, 0x010000, CRC(08343e7f) SHA1(2085350e2506cf2d9c7aa74211cca912b36b203d) )
	ROM_LOAD( "wwfs15.bin",    0x0c0000, 0x010000, CRC(aac5a928) SHA1(1298a5d29b388768ed6508522830e02f95fb54fc) )
	ROM_LOAD( "wwfs14.bin",    0x0d0000, 0x010000, CRC(67eb7bea) SHA1(1de39072f96a80a41c383e495bb686adb353586c) )
	ROM_LOAD( "wwfs13.bin",    0x0e0000, 0x010000, CRC(970b6e76) SHA1(c0da2237f759980d2d879c55c6855633c99fc418) )
	ROM_LOAD( "wwfs12.bin",    0x0f0000, 0x010000, CRC(242caff5) SHA1(9e2a836d9c5415c9313e6609a2eebcb661fa0301) )
	ROM_LOAD( "wwfs27.bin",    0x100000, 0x010000, CRC(f3eb8ab9) SHA1(4032f96d9c738706e353af7f00de921c2c1b72be) )
	ROM_LOAD( "wwfs26.bin",    0x110000, 0x010000, CRC(2ca91eaf) SHA1(191512aaf9542cbbd441886455cbfb5e7a0ab5d4) )
	ROM_LOAD( "wwfs25.bin",    0x120000, 0x010000, CRC(bbf69c6a) SHA1(c9502c9f1fa257f506a4aed22c015524a9fca074) )
	ROM_LOAD( "wwfs24.bin",    0x130000, 0x010000, CRC(76b08bcd) SHA1(c60bc47cf172203e570e693244a1c6308fa36f0b) )
	ROM_LOAD( "wwfs23.bin",    0x140000, 0x010000, CRC(681f5b5e) SHA1(17ac4dbfa84f5161f8d1c740ee91ccecf9f83f5f) )
	ROM_LOAD( "wwfs22.bin",    0x150000, 0x010000, CRC(81fe1bf7) SHA1(37102a6d276907bfeaccc81f1d6693e1c1f26cce) )
	ROM_LOAD( "wwfs21.bin",    0x160000, 0x010000, CRC(c52eee5e) SHA1(6bf7c63b3c18487dd7d964fe05cef348c6069775) )
	ROM_LOAD( "wwfs20.bin",    0x170000, 0x010000, CRC(b2a8050e) SHA1(6db9463321973a3141b6ceda35d11f851d0b9e1f) )
	ROM_LOAD( "wwfs35.bin",    0x180000, 0x010000, CRC(9d648d82) SHA1(81be2ca9f8384b29cf6ce9d59dedf8be1f37fd5d) )
	ROM_LOAD( "wwfs34.bin",    0x190000, 0x010000, CRC(742a79db) SHA1(5c2a5b578817ea1ed8b6993a8bc554840d7302a9) )
	ROM_LOAD( "wwfs33.bin",    0x1a0000, 0x010000, CRC(f6923db6) SHA1(5d0aba7f8e3fbde890ef67e91dbdd2bd3e67a23c) )
	ROM_LOAD( "wwfs32.bin",    0x1b0000, 0x010000, CRC(9becd621) SHA1(200c485d4d5acaf55f47d716a0df3218b64f813a) )
	ROM_LOAD( "wwfs31.bin",    0x1c0000, 0x010000, CRC(f94c74d5) SHA1(8f740860562876bd21a47ba8be758ecd6913207c) )
	ROM_LOAD( "wwfs30.bin",    0x1d0000, 0x010000, CRC(94094518) SHA1(e010b211ea9c08a3c1f36a0e04f2c4320acaa2b7) )
	ROM_LOAD( "wwfs29.bin",    0x1e0000, 0x010000, CRC(7b5b9d83) SHA1(e7381e48a3a63f28fc9a997bfda3e612f4fcccf9) )
	ROM_LOAD( "wwfs28.bin",    0x1f0000, 0x010000, CRC(70fda626) SHA1(049ef67f57953266ef2c750f58c0ee9baf963b39) )

	ROM_REGION( 0x80000, REGION_GFX3, ROMREGION_DISPOSE ) /* BG0 Tiles (16x16) */
	ROM_LOAD( "wwfs51.bin",    0x00000, 0x10000, CRC(51157385) SHA1(fa9f74ace9432d8686402e410cbc03a8c3b86f4d) )
	ROM_LOAD( "wwfs50.bin",    0x10000, 0x10000, CRC(7fc79df5) SHA1(c57e8bb55a1d176b9232395207c5a28c622de9a4) )
	ROM_LOAD( "wwfs49.bin",    0x20000, 0x10000, CRC(a14076b0) SHA1(6817f56d2c6e2d596ebc7827d816ad331b425eeb) )
	ROM_LOAD( "wwfs48.bin",    0x30000, 0x10000, CRC(251372fd) SHA1(e6036807c902fb34071da8287dedcef6cadae06a) )
	ROM_LOAD( "wwfs47.bin",    0x40000, 0x10000, CRC(6fd7b6ea) SHA1(7e77e7647153bcaf09e1002b03f851fe474925a2) )
	ROM_LOAD( "wwfs46.bin",    0x50000, 0x10000, CRC(985e5180) SHA1(9fd8b1ae844a2be465748e3a95ea24aa032e490d) )
	ROM_LOAD( "wwfs45.bin",    0x60000, 0x10000, CRC(b2fad792) SHA1(083977c041c42c50e4f1f7140d97a7b792f768e9) )
	ROM_LOAD( "wwfs44.bin",    0x70000, 0x10000, CRC(4f965fa9) SHA1(4312838e216d2a90fe413d027f46d77c74a0aa07) )
ROM_END

/* this set is using the proper mask rom dumps */
ROM_START( wwfsstaa )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* Main CPU  (68000) */
	ROM_LOAD16_BYTE( "24ac-06.34", 0x00000, 0x20000, CRC(924a50e4) SHA1(e163ffc6bada5db0d979523dde77355acedcd456) )
	ROM_LOAD16_BYTE( "24ad-07.35", 0x00001, 0x20000, CRC(9a76a50e) SHA1(adde96956a7602ae1ece797732e8295dc176b071) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Sound CPU (Z80)  */
	ROM_LOAD( "24ab-0.12", 0x00000, 0x08000, CRC(1e44f8aa) SHA1(e03857d6954e9b9b6073b211e2d6570032af8807) )

	ROM_REGION( 0x40000, REGION_SOUND1, 0 )	/* ADPCM samples */
	ROM_LOAD( "24a9-0.46", 0x00000, 0x20000, CRC(703ff08f) SHA1(08c4d33208eb4c76c751a1a0fe16a817bdc30820) )
	ROM_LOAD( "24j8-0.45", 0x20000, 0x20000, CRC(61138487) SHA1(6d5e3b12acdefb6923aa8ae0704f6c328f4747b3) )

	ROM_REGION( 0x20000, REGION_GFX1, ROMREGION_DISPOSE ) /* FG0 Tiles (8x8) */
	ROM_LOAD( "24aa-0.58", 0x00000, 0x20000, CRC(cb12ba40) SHA1(2d39f778d9daf0d3606b63975bd6cfc45847a265) )

	ROM_REGION( 0x200000, REGION_GFX2, ROMREGION_DISPOSE ) /* SPR Tiles (16x16) */
	ROM_LOAD( "c951.114",   0x000000, 0x80000, CRC(fa76d1f0) SHA1(f69f8e6d1c5f27b054133e0faa49a8e1a9c391b2) )
	ROM_LOAD( "24j4-0.115", 0x080000, 0x40000, CRC(c4a589a3) SHA1(5511e77c8b381419d7c63971023783c26ef6d94b) )
	ROM_LOAD( "24j5-0.116", 0x0c0000, 0x40000, CRC(d6bca436) SHA1(25857a840b93f7f106a3a5c7dde8e0a732f45013) )
	ROM_LOAD( "c950.117",   0x100000, 0x80000, CRC(cca5703d) SHA1(d10ef7ef1789a4f1a732a7c08ab163ec0d347da1) )
	ROM_LOAD( "24j2-0.118", 0x180000, 0x40000, CRC(dc1b7600) SHA1(bd80d7d4063f2b739ac9420132859c23473d9968) )
	ROM_LOAD( "24j3-0.119", 0x1c0000, 0x40000, CRC(3ba12d43) SHA1(f60d5ff54fdef5a31fe1ee7041dda325ef6649c8) )

	ROM_REGION( 0x80000, REGION_GFX3, ROMREGION_DISPOSE ) /* BG0 Tiles (16x16) */
	ROM_LOAD( "24j7-0.113", 0x00000, 0x40000, CRC(e0a1909e) SHA1(6ec0db2e0297256d1c6d003a0e5b29236048bd88) )
	ROM_LOAD( "24j6-0.112", 0x40000, 0x40000, CRC(77932ef8) SHA1(a6ee3fc05ca0001d5181b69f2b754170ba7a814a) )
ROM_END

ROM_START( wwfsstaj )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* Main CPU  (68000) */
	ROM_LOAD16_BYTE( "24ac-0_j-1_japan.34", 0x00000, 0x20000, CRC(f872e968) SHA1(e52298817348601ed88c369018d3110e467cf602) )
	ROM_LOAD16_BYTE( "24ad-0_j-1_japan.35", 0x00001, 0x20000, NO_DUMP )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Sound CPU (Z80)  */
	ROM_LOAD( "b.12",    0x00000, 0x08000, CRC(1e44f8aa) SHA1(e03857d6954e9b9b6073b211e2d6570032af8807) )

	ROM_REGION( 0x40000, REGION_SOUND1, 0 )	/* ADPCM samples */
	ROM_LOAD( "24a9.46",	   0x00000, 0x20000, CRC(703ff08f) SHA1(08c4d33208eb4c76c751a1a0fe16a817bdc30820) )
	ROM_LOAD( "wwfs03.bin",    0x20000, 0x10000, CRC(8a35a20e) SHA1(3bc1a43f956b6840a4bee9e8fb2a6e3d4ac18f75) )
	ROM_LOAD( "wwfs05.bin",    0x30000, 0x10000, CRC(6df08962) SHA1(e3dec81644fe5867024a2fcf34a67924622f3a5b) )

	ROM_REGION( 0x20000, REGION_GFX1, ROMREGION_DISPOSE ) /* FG0 Tiles (8x8) */
	/* this rom may not be correct for this set.. the rom in the set had half the data missing and only a 99%
       match with wwfsstau for the first part */
	ROM_LOAD( "24a4-0.58",    0x00000, 0x20000, BAD_DUMP CRC(cb12ba40) SHA1(2d39f778d9daf0d3606b63975bd6cfc45847a265) )

	/* these are bootleg roms ...  the original has mask roms */
	ROM_REGION( 0x200000, REGION_GFX2, ROMREGION_DISPOSE ) /* SPR Tiles (16x16) */
	ROM_LOAD( "wwfs39.bin",    0x000000, 0x010000, CRC(d807b09a) SHA1(e5a221ac57e16cb3fb47d986e62f265ebbc5b0e6) )
	ROM_LOAD( "wwfs38.bin",    0x010000, 0x010000, CRC(d8ea94d3) SHA1(3a9e200dbcd456364317858e4b5fa6a149cb3c61) )
	ROM_LOAD( "wwfs37.bin",    0x020000, 0x010000, CRC(5e8d7407) SHA1(829cc0c2013138097aa49c9072b87452bf8c8936) )
	ROM_LOAD( "wwfs36.bin",    0x030000, 0x010000, CRC(9005e942) SHA1(d0276419c21b866e17be85382f4e6f3baa4ce40b) )
	ROM_LOAD( "wwfs43.bin",    0x040000, 0x010000, CRC(aafc4a38) SHA1(ac48f13fc4d51e425748190f68b32c099acf532d) )
	ROM_LOAD( "wwfs42.bin",    0x050000, 0x010000, CRC(e48b88fb) SHA1(0fbf9109b86fc6376b8705d28c4c3aeb7fb9cdd8) )
	ROM_LOAD( "wwfs41.bin",    0x060000, 0x010000, CRC(ed7f69d5) SHA1(ae11aad3af43a0e240d17f4db26d68eaae7f1cf0) )
	ROM_LOAD( "wwfs40.bin",    0x070000, 0x010000, CRC(4d75fd89) SHA1(76a1f4a169648e00fcb150157393e3a45613f232) )
	ROM_LOAD( "wwfs19.bin",    0x080000, 0x010000, CRC(7426d444) SHA1(1c1af9492bb711701100bfcecab35f0c38260756) )
	ROM_LOAD( "wwfs18.bin",    0x090000, 0x010000, CRC(af11ad2a) SHA1(4214b16ada1679c6e18c5f2b6e5d6ddb4b731361) )
	ROM_LOAD( "wwfs17.bin",    0x0a0000, 0x010000, CRC(ef12069f) SHA1(5748646c0b0d6e00b6eea26fda3a3699e1553473) )
	ROM_LOAD( "wwfs16.bin",    0x0b0000, 0x010000, CRC(08343e7f) SHA1(2085350e2506cf2d9c7aa74211cca912b36b203d) )
	ROM_LOAD( "wwfs15.bin",    0x0c0000, 0x010000, CRC(aac5a928) SHA1(1298a5d29b388768ed6508522830e02f95fb54fc) )
	ROM_LOAD( "wwfs14.bin",    0x0d0000, 0x010000, CRC(67eb7bea) SHA1(1de39072f96a80a41c383e495bb686adb353586c) )
	ROM_LOAD( "wwfs13.bin",    0x0e0000, 0x010000, CRC(970b6e76) SHA1(c0da2237f759980d2d879c55c6855633c99fc418) )
	ROM_LOAD( "wwfs12.bin",    0x0f0000, 0x010000, CRC(242caff5) SHA1(9e2a836d9c5415c9313e6609a2eebcb661fa0301) )
	ROM_LOAD( "wwfs27.bin",    0x100000, 0x010000, CRC(f3eb8ab9) SHA1(4032f96d9c738706e353af7f00de921c2c1b72be) )
	ROM_LOAD( "wwfs26.bin",    0x110000, 0x010000, CRC(2ca91eaf) SHA1(191512aaf9542cbbd441886455cbfb5e7a0ab5d4) )
	ROM_LOAD( "wwfs25.bin",    0x120000, 0x010000, CRC(bbf69c6a) SHA1(c9502c9f1fa257f506a4aed22c015524a9fca074) )
	ROM_LOAD( "wwfs24.bin",    0x130000, 0x010000, CRC(76b08bcd) SHA1(c60bc47cf172203e570e693244a1c6308fa36f0b) )
	ROM_LOAD( "wwfs23.bin",    0x140000, 0x010000, CRC(681f5b5e) SHA1(17ac4dbfa84f5161f8d1c740ee91ccecf9f83f5f) )
	ROM_LOAD( "wwfs22.bin",    0x150000, 0x010000, CRC(81fe1bf7) SHA1(37102a6d276907bfeaccc81f1d6693e1c1f26cce) )
	ROM_LOAD( "wwfs21.bin",    0x160000, 0x010000, CRC(c52eee5e) SHA1(6bf7c63b3c18487dd7d964fe05cef348c6069775) )
	ROM_LOAD( "wwfs20.bin",    0x170000, 0x010000, CRC(b2a8050e) SHA1(6db9463321973a3141b6ceda35d11f851d0b9e1f) )
	ROM_LOAD( "wwfs35.bin",    0x180000, 0x010000, CRC(9d648d82) SHA1(81be2ca9f8384b29cf6ce9d59dedf8be1f37fd5d) )
	ROM_LOAD( "wwfs34.bin",    0x190000, 0x010000, CRC(742a79db) SHA1(5c2a5b578817ea1ed8b6993a8bc554840d7302a9) )
	ROM_LOAD( "wwfs33.bin",    0x1a0000, 0x010000, CRC(f6923db6) SHA1(5d0aba7f8e3fbde890ef67e91dbdd2bd3e67a23c) )
	ROM_LOAD( "wwfs32.bin",    0x1b0000, 0x010000, CRC(9becd621) SHA1(200c485d4d5acaf55f47d716a0df3218b64f813a) )
	ROM_LOAD( "wwfs31.bin",    0x1c0000, 0x010000, CRC(f94c74d5) SHA1(8f740860562876bd21a47ba8be758ecd6913207c) )
	ROM_LOAD( "wwfs30.bin",    0x1d0000, 0x010000, CRC(94094518) SHA1(e010b211ea9c08a3c1f36a0e04f2c4320acaa2b7) )
	ROM_LOAD( "wwfs29.bin",    0x1e0000, 0x010000, CRC(7b5b9d83) SHA1(e7381e48a3a63f28fc9a997bfda3e612f4fcccf9) )
	ROM_LOAD( "wwfs28.bin",    0x1f0000, 0x010000, CRC(70fda626) SHA1(049ef67f57953266ef2c750f58c0ee9baf963b39) )

	ROM_REGION( 0x80000, REGION_GFX3, ROMREGION_DISPOSE ) /* BG0 Tiles (16x16) */
	ROM_LOAD( "wwfs51.bin",    0x00000, 0x10000, CRC(51157385) SHA1(fa9f74ace9432d8686402e410cbc03a8c3b86f4d) )
	ROM_LOAD( "wwfs50.bin",    0x10000, 0x10000, CRC(7fc79df5) SHA1(c57e8bb55a1d176b9232395207c5a28c622de9a4) )
	ROM_LOAD( "wwfs49.bin",    0x20000, 0x10000, CRC(a14076b0) SHA1(6817f56d2c6e2d596ebc7827d816ad331b425eeb) )
	ROM_LOAD( "wwfs48.bin",    0x30000, 0x10000, CRC(251372fd) SHA1(e6036807c902fb34071da8287dedcef6cadae06a) )
	ROM_LOAD( "wwfs47.bin",    0x40000, 0x10000, CRC(6fd7b6ea) SHA1(7e77e7647153bcaf09e1002b03f851fe474925a2) )
	ROM_LOAD( "wwfs46.bin",    0x50000, 0x10000, CRC(985e5180) SHA1(9fd8b1ae844a2be465748e3a95ea24aa032e490d) )
	ROM_LOAD( "wwfs45.bin",    0x60000, 0x10000, CRC(b2fad792) SHA1(083977c041c42c50e4f1f7140d97a7b792f768e9) )
	ROM_LOAD( "wwfs44.bin",    0x70000, 0x10000, CRC(4f965fa9) SHA1(4312838e216d2a90fe413d027f46d77c74a0aa07) )
ROM_END



GAME( 1989, wwfsstar, 0,        wwfsstar, wwfsstar,  0, ROT0, "Technos Japan", "WWF Superstars (Europe)", 0 )
GAME( 1989, wwfsstau, wwfsstar, wwfsstar, wwfsstar,  0, ROT0, "Technos Japan", "WWF Superstars (US)", 0 )
GAME( 1989, wwfsstaa, wwfsstar, wwfsstar, wwfsstar,  0, ROT0, "Technos Japan", "WWF Superstars (US, Newer)", 0 )
GAME( 1989, wwfsstaj, wwfsstar, wwfsstar, wwfsstar,  0, ROT0, "Technos Japan", "WWF Superstars (Japan)",GAME_NOT_WORKING ) // missing a program rom
