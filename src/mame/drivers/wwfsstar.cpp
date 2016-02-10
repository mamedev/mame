// license:BSD-3-Clause
// copyright-holders:David Haywood
// thanks-to:Richard Bush
/*******************************************************************************
 WWF Superstars (C) 1989 Technos Japan  (drivers/wwfsstar.c)
********************************************************************************
 driver by David Haywood

 Special Thanks to:

 Richard Bush & the Rest of the Raine Team - Raine's WWF Superstars driver on
 which most of this driver has been based.

********************************************************************************

WWF Superstars
Technos 1989

PCB Layout
----------

TA-0024-P1-05
|--------------------------------------------------------------------|
|M51516   558     558   YM3012        M6295       YM2151  3.579545MHz|
|                                1.056MHz                            |
|         558     558                               6116             |
|                              24A9-0.46                            |-|
|                                                   Z80             | |
|                              24J8-0.45                            | |
|                                              24AB-0.12            | |
|                                                                   | |
|               24AA-0.58                                           | |
|                                                   6116            | |
|                                                                   | |
|J                                                                  | |
|A                                                                  |-|
|M                                                                   |
|M                                                                   |
|A    DSW1                                       6116 6116           |
|     DSW2                                                           |
|                                                                   |-|
|                                                                   | |
|                                                                   | |
|                                                                   | |
|                                                                   | |
|                                                                   | |
|                                                                   | |
|                                                                   | |
|                                                                   | |
|                 68000        24AD-04.35    6264                   |-|
|                                                                    |
|                              24AC-04.34    6264                    |
|20MHz                                                               |
|--------------------------------------------------------------------|
Notes:
      Z80    - 3.579545MHz
      68000  - 10.000MHz [20/2]
      M6295  - 1.056MHz (resonator)
      YM2151 - 3.579545MHz
      VSync  - 57.4447Hz


Bottom Board

TA-0024-P2-23
|--------------------------------------------------------------------|
|                                     2018                           |
| IC119                                                              |
|                                                                    |
|                                                                   |-|
| IC118                                                             | |
|                                     2018                          | |
|                                                                   | |
| IC117                                                             | |
|                                                                   | |
|                                                                   | |
| IC116                                         2018                | |
|                                                                   | |
|                                                                   |-|
| IC115              2018             2018                           |
|                                                                    |
|                                     2018                           |
| IC114                                                              |
|                                                                   |-|
|                                                                   | |
|                                                                   | |
|                                                                   | |
|                                                                   | |
|                                                                   | |
| IC113                                                             | |
|                                                                   | |
|                                                                   | |
|                                       |-------|                   |-|
| IC112                                 |TECHNOS|  24MHz             |
|                                       |TJ-001 |                    |
|                                       |-------|                    |
|--------------------------------------------------------------------|
Notes:
      IC11x  - TC534000 MaskROMs
      TJ-001 - Probably a microcontroller badged as a Technos Custom IC (QFP80).
               Clocks: pin 1 - 24MHz, pin 3 - 24/2, pin 4 - 24/4, pin 5 - 24/8,
               pin 6 - 24/16, pin 7 - 24/32, pin 8 - 24/64, pin 64,65 - 1.5MHz

 Hardware:

 Primary CPU : 68000

 Sound CPUs : Z80

 Sound Chips : YM2151, M6295

 3 Layers from now on if mentioned will be referred to as

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
 18 Jun 2001 | Changed Interrupt Function .. it's not fully understood what
             | is meant to be going on ..
 15 Jun 2001 | Cleaned up Sprite Drawing a bit, correcting some clipping probs,
             | mapped DSW's
 15 Jun 2001 | First Submission of Driver,
 14 Jun 2001 | Started Driver, using Raine Source as a reference for getting it
             | up and running

********************************************************************************

 Notes:

 - Scrolling *might* be slightly off, i'm not sure


 - About the bootleg set:
   It matches the US (ealier) set 99.99% just in 64K chunks.  The ONLY difference
   in the data is WWFS47.BIN has 5 bytes with a single bit stuck (0x00001000):

   Offset   WWFS47.BIN   24j6-0.112 (first 0x10000 bytes)
   --------------------------------------------------------
   0xBB03      F8          F0
   0xD9B1      0F          07
   0xED63      4C          44
   0xEE00      3B          33
   0xF8B8      F8          F0

*******************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "includes/wwfsstar.h"

#define MASTER_CLOCK    XTAL_20MHz
#define CPU_CLOCK       MASTER_CLOCK / 2
#define PIXEL_CLOCK     MASTER_CLOCK / 4


/*******************************************************************************
 Memory Maps
********************************************************************************
 Pretty Straightforward
*******************************************************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, wwfsstar_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x080000, 0x080fff) AM_RAM_WRITE(fg0_videoram_w) AM_SHARE("fg0_videoram") /* FG0 Ram */
	AM_RANGE(0x0c0000, 0x0c0fff) AM_RAM_WRITE(bg0_videoram_w) AM_SHARE("bg0_videoram") /* BG0 Ram */
	AM_RANGE(0x100000, 0x1003ff) AM_RAM AM_SHARE("spriteram")       /* SPR Ram */
	AM_RANGE(0x140000, 0x140fff) AM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x180000, 0x180003) AM_WRITE(irqack_w)
	AM_RANGE(0x180000, 0x180001) AM_READ_PORT("DSW1")
	AM_RANGE(0x180002, 0x180003) AM_READ_PORT("DSW2")
	AM_RANGE(0x180004, 0x180005) AM_READ_PORT("P1")
	AM_RANGE(0x180004, 0x180007) AM_WRITE(scroll_w)
	AM_RANGE(0x180006, 0x180007) AM_READ_PORT("P2")
	AM_RANGE(0x180008, 0x180009) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x180008, 0x180009) AM_WRITE(sound_w)
	AM_RANGE(0x18000a, 0x18000b) AM_WRITE(flipscreen_w)
	AM_RANGE(0x1c0000, 0x1c3fff) AM_RAM                             /* Work Ram */
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, wwfsstar_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x8801) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE(0x9800, 0x9800) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END


/*******************************************************************************
 Read / Write Handlers
********************************************************************************
 as used by the above memory map
*******************************************************************************/

WRITE16_MEMBER(wwfsstar_state::scroll_w)
{
	switch (offset)
	{
		case 0x00:
			m_scrollx = data;
			break;
		case 0x01:
			m_scrolly = data;
			break;
	}
}

WRITE16_MEMBER(wwfsstar_state::sound_w)
{
	soundlatch_byte_w(space, 1, data & 0xff);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE );
}

WRITE16_MEMBER(wwfsstar_state::flipscreen_w)
{
	flip_screen_set(data & 1);
}

WRITE16_MEMBER(wwfsstar_state::irqack_w)
{
	if (offset == 0)
		m_maincpu->set_input_line(6, CLEAR_LINE);

	else
		m_maincpu->set_input_line(5, CLEAR_LINE);
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

TIMER_DEVICE_CALLBACK_MEMBER(wwfsstar_state::scanline)
{
	int scanline = param;

	/* Vblank is lowered on scanline 0 */
	if (scanline == 0)
	{
		m_vblank = 0;
	}
	/* Hack */
	else if (scanline == (240-1))       /* -1 is an hack needed to avoid deadlocks */
	{
		m_vblank = 1;
	}

	/* An interrupt is generated every 16 scanlines */
	if (scanline % 16 == 0)
	{
		if (scanline > 0)
			m_screen->update_partial(scanline - 1);
		m_maincpu->set_input_line(5, ASSERT_LINE);
	}

	/* Vblank is raised on scanline 240 */
	if (scanline == 240)
	{
		m_screen->update_partial(scanline - 1);
		m_maincpu->set_input_line(6, ASSERT_LINE);
	}
}

CUSTOM_INPUT_MEMBER(wwfsstar_state::vblank_r)
{
	return m_vblank;
}

/*******************************************************************************
 Input Ports
********************************************************************************
 2 Sets of Player Controls
 A Misc Inputs inc. Coins
 2 Sets of Dipswitches
*******************************************************************************/

static INPUT_PORTS_START( wwfsstar )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Button A (1P VS CPU - Power Up)")

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START3 ) PORT_NAME("Button C (1P/2P VS CPU)")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Button B (1P VS 2P - Buy-in)")

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, wwfsstar_state, vblank_r, NULL) /* VBlank */
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00,  DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01,  DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02,  DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07,  DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06,  DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05,  DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04,  DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03,  DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x00,  DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08,  DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10,  DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38,  DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30,  DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28,  DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20,  DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18,  DEF_STR( 1C_5C ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7" )        /* Manual shows Cabinet Type: Off=Upright & On=Table, has no effect */
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Super Techniques" )      PORT_DIPLOCATION("SW2:4")   // Check code at 0x014272
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x30, 0x30, "Time" )          PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, "+2:30" )
	PORT_DIPSETTING(    0x30, "Default" )
	PORT_DIPSETTING(    0x10, "-2:30" )
	PORT_DIPSETTING(    0x00, "-5:00" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )        /* Manual shows "3 Buttons" has no or unknown effect */
	PORT_DIPNAME( 0x80, 0x80, "Health For Winning" )    PORT_DIPLOCATION("SW2:8")
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
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout,     0, 16 )    /* colors   0-255 */
	GFXDECODE_ENTRY( "gfx2", 0, tiles16x16_layout, 128, 16 )    /* colors   128-383 */
	GFXDECODE_ENTRY( "gfx3", 0, tiles16x16_layout, 256,  8 )    /* colors   256-383 */
GFXDECODE_END


/*******************************************************************************
 Machine Driver(s)
*******************************************************************************/

static MACHINE_CONFIG_START( wwfsstar, wwfsstar_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", wwfsstar_state, scanline, "screen", 0, 1)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_3_579545MHz)
	MCFG_CPU_PROGRAM_MAP(sound_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, 320, 0, 256, 272, 8, 248)   /* HTOTAL and VTOTAL are guessed */
	MCFG_SCREEN_UPDATE_DRIVER(wwfsstar_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", wwfsstar)
	MCFG_PALETTE_ADD("palette", 384)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", XTAL_3_579545MHz)
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.45)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.45)

	MCFG_OKIM6295_ADD("oki", XTAL_1_056MHz, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.47)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.47)
MACHINE_CONFIG_END

/*******************************************************************************
 Rom Loaders / Game Drivers
*******************************************************************************/

ROM_START( wwfsstar )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* Main CPU  (68000) */
	ROM_LOAD16_BYTE( "24ac-0_j-1.34", 0x00000, 0x20000, CRC(ec8fd2c9) SHA1(04ab93e2a1becdc480750c3b55839328b2af4639) )
	ROM_LOAD16_BYTE( "24ad-0_j-1.35", 0x00001, 0x20000, CRC(54e614e4) SHA1(ee924dea977606fcb1222d1aa89211994126a182) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU (Z80)  */
	ROM_LOAD( "24ab-0.12", 0x00000, 0x08000, CRC(1e44f8aa) SHA1(e03857d6954e9b9b6073b211e2d6570032af8807) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "24a9-0.46", 0x00000, 0x20000, CRC(703ff08f) SHA1(08c4d33208eb4c76c751a1a0fe16a817bdc30820) )
	ROM_LOAD( "24j8-0.45", 0x20000, 0x20000, CRC(61138487) SHA1(6d5e3b12acdefb6923aa8ae0704f6c328f4747b3) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* FG0 Tiles (8x8) */
	ROM_LOAD( "24aa-0.58", 0x00000, 0x20000, CRC(cb12ba40) SHA1(2d39f778d9daf0d3606b63975bd6cfc45847a265) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* SPR Tiles (16x16) */
	ROM_LOAD( "c951.114",   0x000000, 0x80000, CRC(fa76d1f0) SHA1(f69f8e6d1c5f27b054133e0faa49a8e1a9c391b2) )
	ROM_LOAD( "24j4-0.115", 0x080000, 0x40000, CRC(c4a589a3) SHA1(5511e77c8b381419d7c63971023783c26ef6d94b) )
	ROM_LOAD( "24j5-0.116", 0x0c0000, 0x40000, CRC(d6bca436) SHA1(25857a840b93f7f106a3a5c7dde8e0a732f45013) )
	ROM_LOAD( "c950.117",   0x100000, 0x80000, CRC(cca5703d) SHA1(d10ef7ef1789a4f1a732a7c08ab163ec0d347da1) )
	ROM_LOAD( "24j2-0.118", 0x180000, 0x40000, CRC(dc1b7600) SHA1(bd80d7d4063f2b739ac9420132859c23473d9968) )
	ROM_LOAD( "24j3-0.119", 0x1c0000, 0x40000, CRC(3ba12d43) SHA1(f60d5ff54fdef5a31fe1ee7041dda325ef6649c8) )

	ROM_REGION( 0x80000, "gfx3", 0 ) /* BG0 Tiles (16x16) */
	ROM_LOAD( "24j7-0.113", 0x00000, 0x40000, CRC(e0a1909e) SHA1(6ec0db2e0297256d1c6d003a0e5b29236048bd88) )
	ROM_LOAD( "24j6-0.112", 0x40000, 0x40000, CRC(77932ef8) SHA1(a6ee3fc05ca0001d5181b69f2b754170ba7a814a) )
ROM_END

ROM_START( wwfsstaru )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* Main CPU  (68000) */
	ROM_LOAD16_BYTE( "24ac-06.34", 0x00000, 0x20000, CRC(924a50e4) SHA1(e163ffc6bada5db0d979523dde77355acedcd456) )
	ROM_LOAD16_BYTE( "24ad-07.35", 0x00001, 0x20000, CRC(9a76a50e) SHA1(adde96956a7602ae1ece797732e8295dc176b071) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU (Z80)  */
	ROM_LOAD( "24ab-0.12", 0x00000, 0x08000, CRC(1e44f8aa) SHA1(e03857d6954e9b9b6073b211e2d6570032af8807) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "24a9-0.46", 0x00000, 0x20000, CRC(703ff08f) SHA1(08c4d33208eb4c76c751a1a0fe16a817bdc30820) )
	ROM_LOAD( "24j8-0.45", 0x20000, 0x20000, CRC(61138487) SHA1(6d5e3b12acdefb6923aa8ae0704f6c328f4747b3) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* FG0 Tiles (8x8) */
	ROM_LOAD( "24aa-0.58", 0x00000, 0x20000, CRC(cb12ba40) SHA1(2d39f778d9daf0d3606b63975bd6cfc45847a265) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* SPR Tiles (16x16) */
	ROM_LOAD( "c951.114",   0x000000, 0x80000, CRC(fa76d1f0) SHA1(f69f8e6d1c5f27b054133e0faa49a8e1a9c391b2) )
	ROM_LOAD( "24j4-0.115", 0x080000, 0x40000, CRC(c4a589a3) SHA1(5511e77c8b381419d7c63971023783c26ef6d94b) )
	ROM_LOAD( "24j5-0.116", 0x0c0000, 0x40000, CRC(d6bca436) SHA1(25857a840b93f7f106a3a5c7dde8e0a732f45013) )
	ROM_LOAD( "c950.117",   0x100000, 0x80000, CRC(cca5703d) SHA1(d10ef7ef1789a4f1a732a7c08ab163ec0d347da1) )
	ROM_LOAD( "24j2-0.118", 0x180000, 0x40000, CRC(dc1b7600) SHA1(bd80d7d4063f2b739ac9420132859c23473d9968) )
	ROM_LOAD( "24j3-0.119", 0x1c0000, 0x40000, CRC(3ba12d43) SHA1(f60d5ff54fdef5a31fe1ee7041dda325ef6649c8) )

	ROM_REGION( 0x80000, "gfx3", 0 ) /* BG0 Tiles (16x16) */
	ROM_LOAD( "24j7-0.113", 0x00000, 0x40000, CRC(e0a1909e) SHA1(6ec0db2e0297256d1c6d003a0e5b29236048bd88) )
	ROM_LOAD( "24j6-0.112", 0x40000, 0x40000, CRC(77932ef8) SHA1(a6ee3fc05ca0001d5181b69f2b754170ba7a814a) )
ROM_END

ROM_START( wwfsstarua )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* Main CPU  (68000) */
	ROM_LOAD16_BYTE( "24ac-04.34", 0x00000, 0x20000, CRC(ee9b850e) SHA1(6b634ad98b6104b9e860d05e73f3a139c2a19a78) )
	ROM_LOAD16_BYTE( "24ad-04.35", 0x00001, 0x20000, CRC(057c2eef) SHA1(6eb5f60fa51b3e7f17fc6a81182a01ea406febea) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU (Z80)  */
	ROM_LOAD( "24ab-0.12", 0x00000, 0x08000, CRC(1e44f8aa) SHA1(e03857d6954e9b9b6073b211e2d6570032af8807) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "24a9-0.46", 0x00000, 0x20000, CRC(703ff08f) SHA1(08c4d33208eb4c76c751a1a0fe16a817bdc30820) )
	ROM_LOAD( "24j8-0.45", 0x20000, 0x20000, CRC(61138487) SHA1(6d5e3b12acdefb6923aa8ae0704f6c328f4747b3) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* FG0 Tiles (8x8) */
	ROM_LOAD( "24aa-0.58", 0x00000, 0x20000, CRC(cb12ba40) SHA1(2d39f778d9daf0d3606b63975bd6cfc45847a265) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* SPR Tiles (16x16) */
	ROM_LOAD( "c951.114",   0x000000, 0x80000, CRC(fa76d1f0) SHA1(f69f8e6d1c5f27b054133e0faa49a8e1a9c391b2) )
	ROM_LOAD( "24j4-0.115", 0x080000, 0x40000, CRC(c4a589a3) SHA1(5511e77c8b381419d7c63971023783c26ef6d94b) )
	ROM_LOAD( "24j5-0.116", 0x0c0000, 0x40000, CRC(d6bca436) SHA1(25857a840b93f7f106a3a5c7dde8e0a732f45013) )
	ROM_LOAD( "c950.117",   0x100000, 0x80000, CRC(cca5703d) SHA1(d10ef7ef1789a4f1a732a7c08ab163ec0d347da1) )
	ROM_LOAD( "24j2-0.118", 0x180000, 0x40000, CRC(dc1b7600) SHA1(bd80d7d4063f2b739ac9420132859c23473d9968) )
	ROM_LOAD( "24j3-0.119", 0x1c0000, 0x40000, CRC(3ba12d43) SHA1(f60d5ff54fdef5a31fe1ee7041dda325ef6649c8) )

	ROM_REGION( 0x80000, "gfx3", 0 ) /* BG0 Tiles (16x16) */
	ROM_LOAD( "24j7-0.113", 0x00000, 0x40000, CRC(e0a1909e) SHA1(6ec0db2e0297256d1c6d003a0e5b29236048bd88) )
	ROM_LOAD( "24j6-0.112", 0x40000, 0x40000, CRC(77932ef8) SHA1(a6ee3fc05ca0001d5181b69f2b754170ba7a814a) )
ROM_END

ROM_START( wwfsstarj )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* Main CPU  (68000) */
	ROM_LOAD16_BYTE( "24ac-0_j-1_japan.34", 0x00000, 0x20000, CRC(f872e968) SHA1(e52298817348601ed88c369018d3110e467cf602) )
	ROM_LOAD16_BYTE( "24ad-0_j-1_japan.35", 0x00001, 0x20000, CRC(c70bcd23) SHA1(b6128b051b68fcca05da34b42ced01916b18a139) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU (Z80)  */
	ROM_LOAD( "24ab-0.12", 0x00000, 0x08000, CRC(1e44f8aa) SHA1(e03857d6954e9b9b6073b211e2d6570032af8807) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "24a9-0.46", 0x00000, 0x20000, CRC(703ff08f) SHA1(08c4d33208eb4c76c751a1a0fe16a817bdc30820) )
	ROM_LOAD( "24j8-0.45", 0x20000, 0x20000, CRC(61138487) SHA1(6d5e3b12acdefb6923aa8ae0704f6c328f4747b3) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* FG0 Tiles (8x8) */
	ROM_LOAD( "24aa-0_j.58", 0x00000, 0x20000, CRC(b9201b36) SHA1(743b86528f6936eb6a4e37d5a23c347ae9d68fa0) ) /* Hand written "J" on label */

	ROM_REGION( 0x200000, "gfx2", 0 ) /* SPR Tiles (16x16) */
	ROM_LOAD( "c951.114",   0x000000, 0x80000, CRC(fa76d1f0) SHA1(f69f8e6d1c5f27b054133e0faa49a8e1a9c391b2) )
	ROM_LOAD( "24j4-0.115", 0x080000, 0x40000, CRC(c4a589a3) SHA1(5511e77c8b381419d7c63971023783c26ef6d94b) )
	ROM_LOAD( "24j5-0.116", 0x0c0000, 0x40000, CRC(d6bca436) SHA1(25857a840b93f7f106a3a5c7dde8e0a732f45013) )
	ROM_LOAD( "c950.117",   0x100000, 0x80000, CRC(cca5703d) SHA1(d10ef7ef1789a4f1a732a7c08ab163ec0d347da1) )
	ROM_LOAD( "24j2-0.118", 0x180000, 0x40000, CRC(dc1b7600) SHA1(bd80d7d4063f2b739ac9420132859c23473d9968) )
	ROM_LOAD( "24j3-0.119", 0x1c0000, 0x40000, CRC(3ba12d43) SHA1(f60d5ff54fdef5a31fe1ee7041dda325ef6649c8) )

	ROM_REGION( 0x80000, "gfx3", 0 ) /* BG0 Tiles (16x16) */
	ROM_LOAD( "24j7-0.113", 0x00000, 0x40000, CRC(e0a1909e) SHA1(6ec0db2e0297256d1c6d003a0e5b29236048bd88) )
	ROM_LOAD( "24j6-0.112", 0x40000, 0x40000, CRC(77932ef8) SHA1(a6ee3fc05ca0001d5181b69f2b754170ba7a814a) )
ROM_END

ROM_START( wwfsstarb )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* Main CPU  (68000) */
	ROM_LOAD16_BYTE( "wwfs08.bin", 0x00000, 0x10000, CRC(621df265) SHA1(eded019352428f2caf1de88eac837beb4eea7562) ) /* These 2 == 24ac-04.34 */
	ROM_LOAD16_BYTE( "wwfs10.bin", 0x20000, 0x10000, CRC(a3382dfe) SHA1(49f78464c51892a84c7f06ce08e900be849fb012) )
	ROM_LOAD16_BYTE( "wwfs07.bin", 0x00001, 0x10000, CRC(369559e6) SHA1(32afd7ea0e0e9e8d5c36e9ef2fb18f7f2cfdcf01) ) /* These 2 == 24ad-04.35 */
	ROM_LOAD16_BYTE( "wwfs09.bin", 0x20001, 0x10000, CRC(8cbcd5aa) SHA1(cb3d7a4a48e4e414da758af248085322b5809914) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU (Z80)  */
	ROM_LOAD( "wwfs01.bin", 0x00000, 0x08000, CRC(1e44f8aa) SHA1(e03857d6954e9b9b6073b211e2d6570032af8807) ) /* This == 24ab-0.12 */

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "wwfs02.bin", 0x00000, 0x10000, CRC(6e63c457) SHA1(9d87345fc55e7af7311974f3890874ebe719aca3) ) /* These 2 == 24a9-0.46 */
	ROM_LOAD( "wwfs04.bin", 0x10000, 0x10000, CRC(d7018a9c) SHA1(7d3a6dd5f70654c8e617d9cba88fcaf1801c4d16) )
	ROM_LOAD( "wwfs03.bin", 0x20000, 0x10000, CRC(8a35a20e) SHA1(3bc1a43f956b6840a4bee9e8fb2a6e3d4ac18f75) ) /* These 2 == 24j8-0.44 */
	ROM_LOAD( "wwfs05.bin", 0x30000, 0x10000, CRC(6df08962) SHA1(e3dec81644fe5867024a2fcf34a67924622f3a5b) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* FG0 Tiles (8x8) */
	ROM_LOAD( "wwfs06.bin", 0x00000, 0x10000, CRC(154ca5ce) SHA1(fc358cd8e1d62c9b299c4261901992d798bf6953) ) /* These 2 == 24aa-0.58 */
	ROM_LOAD( "wwfs11.bin", 0x10000, 0x10000, CRC(3d4684dc) SHA1(f6372d41de9bd7458cbab59f29053325ffdf8d69) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* SPR Tiles (16x16) */
	ROM_LOAD( "wwfs39.bin", 0x000000, 0x010000, CRC(d807b09a) SHA1(e5a221ac57e16cb3fb47d986e62f265ebbc5b0e6) ) /* Data matches original MASK roms 100% */
	ROM_LOAD( "wwfs38.bin", 0x010000, 0x010000, CRC(d8ea94d3) SHA1(3a9e200dbcd456364317858e4b5fa6a149cb3c61) )
	ROM_LOAD( "wwfs37.bin", 0x020000, 0x010000, CRC(5e8d7407) SHA1(829cc0c2013138097aa49c9072b87452bf8c8936) )
	ROM_LOAD( "wwfs36.bin", 0x030000, 0x010000, CRC(9005e942) SHA1(d0276419c21b866e17be85382f4e6f3baa4ce40b) )
	ROM_LOAD( "wwfs43.bin", 0x040000, 0x010000, CRC(aafc4a38) SHA1(ac48f13fc4d51e425748190f68b32c099acf532d) )
	ROM_LOAD( "wwfs42.bin", 0x050000, 0x010000, CRC(e48b88fb) SHA1(0fbf9109b86fc6376b8705d28c4c3aeb7fb9cdd8) )
	ROM_LOAD( "wwfs41.bin", 0x060000, 0x010000, CRC(ed7f69d5) SHA1(ae11aad3af43a0e240d17f4db26d68eaae7f1cf0) )
	ROM_LOAD( "wwfs40.bin", 0x070000, 0x010000, CRC(4d75fd89) SHA1(76a1f4a169648e00fcb150157393e3a45613f232) )
	ROM_LOAD( "wwfs19.bin", 0x080000, 0x010000, CRC(7426d444) SHA1(1c1af9492bb711701100bfcecab35f0c38260756) )
	ROM_LOAD( "wwfs18.bin", 0x090000, 0x010000, CRC(af11ad2a) SHA1(4214b16ada1679c6e18c5f2b6e5d6ddb4b731361) )
	ROM_LOAD( "wwfs17.bin", 0x0a0000, 0x010000, CRC(ef12069f) SHA1(5748646c0b0d6e00b6eea26fda3a3699e1553473) )
	ROM_LOAD( "wwfs16.bin", 0x0b0000, 0x010000, CRC(08343e7f) SHA1(2085350e2506cf2d9c7aa74211cca912b36b203d) )
	ROM_LOAD( "wwfs15.bin", 0x0c0000, 0x010000, CRC(aac5a928) SHA1(1298a5d29b388768ed6508522830e02f95fb54fc) )
	ROM_LOAD( "wwfs14.bin", 0x0d0000, 0x010000, CRC(67eb7bea) SHA1(1de39072f96a80a41c383e495bb686adb353586c) )
	ROM_LOAD( "wwfs13.bin", 0x0e0000, 0x010000, CRC(970b6e76) SHA1(c0da2237f759980d2d879c55c6855633c99fc418) )
	ROM_LOAD( "wwfs12.bin", 0x0f0000, 0x010000, CRC(242caff5) SHA1(9e2a836d9c5415c9313e6609a2eebcb661fa0301) )
	ROM_LOAD( "wwfs27.bin", 0x100000, 0x010000, CRC(f3eb8ab9) SHA1(4032f96d9c738706e353af7f00de921c2c1b72be) )
	ROM_LOAD( "wwfs26.bin", 0x110000, 0x010000, CRC(2ca91eaf) SHA1(191512aaf9542cbbd441886455cbfb5e7a0ab5d4) )
	ROM_LOAD( "wwfs25.bin", 0x120000, 0x010000, CRC(bbf69c6a) SHA1(c9502c9f1fa257f506a4aed22c015524a9fca074) )
	ROM_LOAD( "wwfs24.bin", 0x130000, 0x010000, CRC(76b08bcd) SHA1(c60bc47cf172203e570e693244a1c6308fa36f0b) )
	ROM_LOAD( "wwfs23.bin", 0x140000, 0x010000, CRC(681f5b5e) SHA1(17ac4dbfa84f5161f8d1c740ee91ccecf9f83f5f) )
	ROM_LOAD( "wwfs22.bin", 0x150000, 0x010000, CRC(81fe1bf7) SHA1(37102a6d276907bfeaccc81f1d6693e1c1f26cce) )
	ROM_LOAD( "wwfs21.bin", 0x160000, 0x010000, CRC(c52eee5e) SHA1(6bf7c63b3c18487dd7d964fe05cef348c6069775) )
	ROM_LOAD( "wwfs20.bin", 0x170000, 0x010000, CRC(b2a8050e) SHA1(6db9463321973a3141b6ceda35d11f851d0b9e1f) )
	ROM_LOAD( "wwfs35.bin", 0x180000, 0x010000, CRC(9d648d82) SHA1(81be2ca9f8384b29cf6ce9d59dedf8be1f37fd5d) )
	ROM_LOAD( "wwfs34.bin", 0x190000, 0x010000, CRC(742a79db) SHA1(5c2a5b578817ea1ed8b6993a8bc554840d7302a9) )
	ROM_LOAD( "wwfs33.bin", 0x1a0000, 0x010000, CRC(f6923db6) SHA1(5d0aba7f8e3fbde890ef67e91dbdd2bd3e67a23c) )
	ROM_LOAD( "wwfs32.bin", 0x1b0000, 0x010000, CRC(9becd621) SHA1(200c485d4d5acaf55f47d716a0df3218b64f813a) )
	ROM_LOAD( "wwfs31.bin", 0x1c0000, 0x010000, CRC(f94c74d5) SHA1(8f740860562876bd21a47ba8be758ecd6913207c) )
	ROM_LOAD( "wwfs30.bin", 0x1d0000, 0x010000, CRC(94094518) SHA1(e010b211ea9c08a3c1f36a0e04f2c4320acaa2b7) )
	ROM_LOAD( "wwfs29.bin", 0x1e0000, 0x010000, CRC(7b5b9d83) SHA1(e7381e48a3a63f28fc9a997bfda3e612f4fcccf9) )
	ROM_LOAD( "wwfs28.bin", 0x1f0000, 0x010000, CRC(70fda626) SHA1(049ef67f57953266ef2c750f58c0ee9baf963b39) )

	ROM_REGION( 0x80000, "gfx3", 0 ) /* BG0 Tiles (16x16) */
	ROM_LOAD( "wwfs51.bin", 0x00000, 0x10000, CRC(51157385) SHA1(fa9f74ace9432d8686402e410cbc03a8c3b86f4d) ) /* These 4 == 24j7-0.113 */
	ROM_LOAD( "wwfs50.bin", 0x10000, 0x10000, CRC(7fc79df5) SHA1(c57e8bb55a1d176b9232395207c5a28c622de9a4) )
	ROM_LOAD( "wwfs49.bin", 0x20000, 0x10000, CRC(a14076b0) SHA1(6817f56d2c6e2d596ebc7827d816ad331b425eeb) )
	ROM_LOAD( "wwfs48.bin", 0x30000, 0x10000, CRC(251372fd) SHA1(e6036807c902fb34071da8287dedcef6cadae06a) )
	ROM_LOAD( "wwfs47.bin", 0x40000, 0x10000, CRC(6fd7b6ea) SHA1(7e77e7647153bcaf09e1002b03f851fe474925a2) ) /* See notes above about this rom */
	ROM_LOAD( "wwfs46.bin", 0x50000, 0x10000, CRC(985e5180) SHA1(9fd8b1ae844a2be465748e3a95ea24aa032e490d) ) /* These 3 == 24j6-0.112 (from 0x10000-0x3ffff) */
	ROM_LOAD( "wwfs45.bin", 0x60000, 0x10000, CRC(b2fad792) SHA1(083977c041c42c50e4f1f7140d97a7b792f768e9) )
	ROM_LOAD( "wwfs44.bin", 0x70000, 0x10000, CRC(4f965fa9) SHA1(4312838e216d2a90fe413d027f46d77c74a0aa07) )
ROM_END



GAME( 1989, wwfsstar,   0,        wwfsstar, wwfsstar, driver_device,  0, ROT0, "Technos Japan", "WWF Superstars (Europe)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, wwfsstaru,  wwfsstar, wwfsstar, wwfsstar, driver_device,  0, ROT0, "Technos Japan", "WWF Superstars (US, Newer)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, wwfsstarua, wwfsstar, wwfsstar, wwfsstar, driver_device,  0, ROT0, "Technos Japan", "WWF Superstars (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, wwfsstarj,  wwfsstar, wwfsstar, wwfsstar, driver_device,  0, ROT0, "Technos Japan", "WWF Superstars (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, wwfsstarb,  wwfsstar, wwfsstar, wwfsstar, driver_device,  0, ROT0, "bootleg",       "WWF Superstars (bootleg)", MACHINE_SUPPORTS_SAVE )
