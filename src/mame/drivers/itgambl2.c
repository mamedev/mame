/************************************************************************

  Italian Gambling games based on H8/3337 MCU + NEC D7759GC (sound).

  Written by Roberto Fresca.


  All these games use MCUs with internal ROM for their programs.

  They have 60KB of internal (normally flash) ROM that can't be dumped
  easily, and thus we can't emulate them at the moment because there is
  nothing to emulate.

  This driver is just a placeholder for the graphics/sound ROM loading


*************************************************************************

  --- Hardware Notes ---

  The hardware is normally composed by:


  CPU:   1x H8/3337 (HD64F3337CP16 or HD64F3337F16).
           (60KB ROM; 2KB RAM)

  Sound: 1x NEC D7759GC.
         1x TDA2003 (audio amplifier).

  PLDs:  2x ispLSI1032E-70JL.

  Clock: 1x Xtal 16.000 MHz.
         1x Xtal 14.318180 MHz.

  ROMs:  1x (up to) 27C2000 or similar (sound).
         3x or 4x 27C4001 or similar (graphics).

  Connectors: 1x 28x2 edge connector.
              1x RS232 connector.
              1x 14 legs connector.
              1x 34 legs connector (optional).

  Other: 1x battery.
         1x red led.
         2x 8 DIP switches.
         2x trimmer.


************************************************************************/

#define MAIN_CLOCK	XTAL_16MHz
#define SND_CLOCK	XTAL_14_31818MHz

#include "driver.h"
#include "cpu/h83002/h8.h"
#include "sound/upd7759.h"


/*************************
*     Video Hardware     *
*************************/

static VIDEO_START( itgambl2 )
{
}


static VIDEO_UPDATE( itgambl2 )
{
	return 0;
}


/*************************
* Memory map information *
*************************/

static ADDRESS_MAP_START( itgambl2_map, ADDRESS_SPACE_PROGRAM, 16 )
	ADDRESS_MAP_GLOBAL_MASK(0xffffff)
	AM_RANGE(0x000000, 0xffffff) AM_ROM
ADDRESS_MAP_END


/*************************
*      Input ports       *
*************************/

static INPUT_PORTS_START( itgambl2 )
    PORT_START("IN0")
    PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
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
INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout gfxlayout_8x8x8 =
{
/* this is wrong and need to be fixed */

	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	8*64
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( itgambl2 )
    GFXDECODE_ENTRY( "gfx1", 0, gfxlayout_8x8x8,   0, 16  )
GFXDECODE_END


/**************************
*      Machine Reset      *
**************************/

static MACHINE_RESET( itgambl2 )
{
	/* stop the CPU, we have no code for it anyway */
	cpu_set_input_line(machine->cpu[0], INPUT_LINE_HALT, ASSERT_LINE);
}


/**************************
*     Machine Drivers     *
**************************/

static MACHINE_DRIVER_START( itgambl2 )

    /* basic machine hardware */
	MDRV_CPU_ADD("main", H83044, MAIN_CLOCK)	/* wrong CPU, but we have not a H8/3337 core ATM */
	MDRV_CPU_PROGRAM_MAP(itgambl2_map,0)

    /* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MDRV_MACHINE_RESET( itgambl2 )

	MDRV_GFXDECODE(itgambl2)
	MDRV_PALETTE_LENGTH(0x200)
	MDRV_VIDEO_START( itgambl2 )
	MDRV_VIDEO_UPDATE( itgambl2 )

    /* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("upd", UPD7759, UPD7759_STANDARD_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


/*************************
*        Rom Load        *
*************************/

/* NtCash

CPU:

1x HD64F3337CP16 (main)
2x ispLSI1032E-70JL (PLD)
1x NEC D7759GC (sound)
1x TDA2003 (audio amp)
1x oscillator 14.318180 MHz
1x oscillator 16.000 MHz

ROMs:

1x M27C1001 (0)
4x M27C4001 (1, 2, 3, 4)

Note:

1x 28x2 edge connector
1x RS232 connector
1x 14 legs isp connector
1x battery
1x red led
2x 8x2 switches dip
2x trimmer

*/

ROM_START( ntcash )
	ROM_REGION( 0x1000000, "main", 0 ) /* all the program code is in here */
	ROM_LOAD( "ntcash_hd64f3337cp16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "ntcashep1.bin", 0x000000, 0x80000, CRC(f1e8b74d) SHA1(b84e36ab101d6b5b1f60d9778bd8e5d89b3d437d) )
	ROM_LOAD( "ntcashep2.bin", 0x080000, 0x80000, CRC(b51513c8) SHA1(27b6469daecb92d8a8ed6e9ab317d20f49dd6475) )
	ROM_LOAD( "ntcashep3.bin", 0x100000, 0x80000, CRC(ba46f1b2) SHA1(61f5b2f1732bbdb2bd21835d2c6e2890c1f0fc8c) )
	ROM_LOAD( "ntcashep4.bin", 0x180000, 0x80000, CRC(1e42142d) SHA1(82444584b1d61ff0a34d7767f70cab995d26e1e1) )

	ROM_REGION( 0x20000, "upd", 0 ) /* NEC D7759GC samples */
	ROM_LOAD( "ntcashmsg0.bin", 0x00000, 0x20000, CRC(e3022f30) SHA1(859bdf0ce871c0b39224dc93b8005a5e0a5552b1) )
ROM_END


/* Wizard (Ver 1.0)

CPU:

1x HD64F3337CP16 (main)
2x ispLSI1032E-70JL (PLD)
1x NEC D7759GC (sound)
1x TDA2003 (audio amp)
1x oscillator 14.318180 MHz
1x oscillator 16.000 MHz

ROMs:

1x M27C1001 (0)
4x M27C4001 (1, 2, 3, 4)

Note:

1x 28x2 edge connector
1x RS232 connector
1x 14 legs isp connector
1x battery
1x red led
2x 8x2 DIP switches
2x trimmer

*/

ROM_START( wizard )
	ROM_REGION( 0x1000000, "main", 0 ) /* all the program code is in here */
	ROM_LOAD( "wizard_ver1.2_hd64f3337cp16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "wizardep1.bin", 0x000000, 0x80000, CRC(a99af86f) SHA1(4bf32df74e93a6b40cf8213e99ec6ef538d9802d) )
	ROM_LOAD( "wizardep2.bin", 0x080000, 0x80000, CRC(bc52566d) SHA1(ecd4f3852c3ba8981316686042dfc2c0013f139f) )
	ROM_LOAD( "wizardep3.bin", 0x100000, 0x80000, CRC(98e1905a) SHA1(805df94fef011b48d5eb2abbd294b7cd338d7124) )
	ROM_LOAD( "wizardep4.bin", 0x180000, 0x80000, CRC(f129916a) SHA1(c1c0fcb04622dde196299c2e88a807b2aa00bf5e) )

	ROM_REGION( 0x20000, "upd", 0 ) /* NEC D7759GC samples */
	ROM_LOAD( "wizardmsg0.bin", 0x00000, 0x20000, CRC(94b28a4b) SHA1(2c10462cd7c8dc79dba735a061841a9c8b423091) )
ROM_END


/* Laser 2001 (Ver 1.2)

CPU:

1x HD64F3337CP16 (main)
2x ispLSI1032E-70JL (PLD)
1x NEC D7759GC (sound)
1x TDA2003 (audio amp)
1x oscillator 14.318180 MHz
1x oscillator 16.000 MHz

ROMs:

1x M27C1001 (0)
4x M27C4001 (1, 2, 3, 4)

Note:

1x 28x2 edge connector
1x RS232 connector
1x 14 legs isp connector
1x battery
1x red led
2x 8x2 switches dip
2x trimmer

*/

ROM_START( laser2k1 )
	ROM_REGION( 0x1000000, "main", 0 ) /* all the program code is in here */
	ROM_LOAD( "laser2k1_ver1.2_hd64f3337cp16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "xlep1.bin", 0x000000, 0x80000, CRC(b45c9491) SHA1(1fa0572d3efb847dcf49bb99f429322dcb72b0d1) )
	ROM_LOAD( "xlep2.bin", 0x080000, 0x80000, CRC(75c82293) SHA1(e6d847a2259393ef8877e9237c7624bf2e36f197) )
	ROM_LOAD( "xlep3.bin", 0x100000, 0x80000, CRC(3a45d626) SHA1(c804916b6bfe04bacd7ac6f32e5041ed65e7b91e) )
	ROM_LOAD( "xlep4.bin", 0x180000, 0x80000, CRC(d0381819) SHA1(30bab7e1c68192a2e1c324ef4c8a3d3b5696eb2b) )

	ROM_REGION( 0x20000, "upd", 0 ) /* NEC D7759GC samples */
	ROM_LOAD( "xlmsg0.bin", 0x00000, 0x20000, CRC(36287068) SHA1(d964837cb5370c7b878e1e531ef6d8c3840f776c) )
ROM_END


/* Magic Drink (Ver 1.2)

CPU:

1x HD64F3337CP16 (main)
2x ispLSI1032E-70JL (PLD)
1x NEC D7759GC (sound)
1x TDA2003 (audio amp)
1x oscillator 14.318180 MHz
1x oscillator 16.000 MHz

ROMs:

1x 27C2000 (s)
3x M27C4001 (1, 2, 3)

Note:

1x 28x2 edge connector
1x RS232 connector
1x 14 legs isp connector
1x battery
1x red led
2x 8x2 switches dip
2x trimmer

*/

ROM_START( mdrink )
	ROM_REGION( 0x1000000, "main", 0 ) /* all the program code is in here */
	ROM_LOAD( "mdrink_ver1.2_hd64f3337cp16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x180000, "gfx1", 0 ) /* only 3 ROMs?? */
	ROM_LOAD( "mdrink-1.bin", 0x000000, 0x80000, CRC(25a7cea9) SHA1(d67a7264501699c8f7a48c3f3956903a5c95898f) )
	ROM_LOAD( "mdrink-2.bin", 0x080000, 0x80000, CRC(c2a14bca) SHA1(8d0095333c34d81d103f15ee5731e2e4aa4d1fac) )
	ROM_LOAD( "mdrink-3.bin", 0x100000, 0x80000, CRC(ff593676) SHA1(b21bb85df0b7b79c07ded2c4b950c94719e08302) )

	ROM_REGION( 0x40000, "upd", 0 ) /* NEC D7759GC samples */
	ROM_LOAD( "mdrink-s.bin", 0x00000, 0x40000, CRC(d78b7823) SHA1(ca01e4aa3e25c3a40517b4fe07c31915e79af650) )
ROM_END


/* Unknown... (Ver 1.2)

CPU

1x H8/3337-HDY1A3-64F3337F16 (main)
1x NEC D7759GC-0124XY007 (sound)
2x ispLSI1032E-70LJ-C110AA02 (main)
1x oscillator 14.318180 MHz
1x oscillator 16.000 MHz

ROMs

1x M27C1001 (0)
1x 27C4000 (1)
3x M27C4001 (2, 3, 4)

Note

1x 28x2 edge connector (not JAMMA)
1x RS232 connector
1x 14 legs connector
1x 34 legs connector
2x 8 DIP switches
1x battery
1x trimmer (volume)
1x trimmer (unknown)

--------------------------

Silkscreened on PCB:
"CE Angelo Arena - Via Vighi, 26  40026 - Imola (BO)"

PCB n. 2-0276 TE04.01

Formely named "videopoker1"

*/

ROM_START( te0144 )
	ROM_REGION( 0x1000000, "main", 0 ) /* all the program code is in here */
	ROM_LOAD( "te0144_ver1.2_hdy1a3-64f3337f16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "pb1.bin", 0x000000, 0x80000, CRC(b7b4ea0f) SHA1(d11096684059e6063747f3e082d70aef1ee8d259) )
	ROM_LOAD( "pb2.bin", 0x080000, 0x80000, CRC(b02fd07e) SHA1(415a834cd47fdcb180b2a5fa267c1566b9ca0b61) )
	ROM_LOAD( "pb3.bin", 0x100000, 0x80000, CRC(1984427e) SHA1(0200360f083019235f464ed9b96bf7f78a07df37) )
	ROM_LOAD( "pb4.bin", 0x180000, 0x80000, CRC(ac513c2d) SHA1(aedc29b12157f02a014359ceae71a2a7892afa72) )

	ROM_REGION( 0x20000, "upd", 0 ) /* NEC D7759GC samples */
	ROM_LOAD( "pb0.bin", 0x00000, 0x20000, CRC(123ef964) SHA1(b36d91b58119c15211a54ff7d78c7137d638ea88) )
ROM_END


/* Carta Magica (Ver 1.8)

CPU:

1x H8/3337-HD64F3337CP16 (main)
1x NEC D7759GC (sound)
1x TDA2003 (sound)
2x ispLSI1032E-70LJ-E011J02
1x oscillator 14.318180 MHz
1x oscillator 16.000 MHz

ROMs:

1x 27C1001 or similar (0)
3x 27C4001 or similar (1, 2, 3)

Note:

1x 28x2 edge connector (not JAMMA)
1x RS232 connector
1x 14 legs connector
2x 8 DIP switches
1x battery
1x trimmer (volume)
1x trimmer (unknown)

--------------------------

Silkscreened on PCB:
"SMS distribuzione"
"Base 2 Synth Rev.1"
"APM1"

*/

ROM_START( cmagica )
	ROM_REGION( 0x1000000, "main", 0 ) /* all the program code is in here */
	ROM_LOAD( "cmagica_ver1.8_hd64f3337cp16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x180000, "gfx1", 0 ) /* gfx seems 3bpp, not separated bitplanes? */
	ROM_LOAD( "1.u6", 0x000000, 0x80000, CRC(3e7e6c9f) SHA1(53a7c4422d9a7c63a21cf4d35d4d883dc2d0eac0) )
	ROM_LOAD( "2.u7", 0x080000, 0x80000, CRC(6339b62d) SHA1(160030e07600c8db365429c27a33081cfa7d3d61) )
	ROM_LOAD( "3.u4", 0x100000, 0x80000, CRC(ba636099) SHA1(3d3d9eee5d6808d7666dbf113d7c17a03b6b461e) )

	ROM_REGION( 0x20000, "upd", 0 ) /* NEC D7759GC samples */
	ROM_LOAD( "sound.bin", 0x00000, 0x20000, CRC(9dab99a6) SHA1(ce34056dd964be32359acd2e53a6101cb4d9ddff) )
ROM_END


/* Millennium Sun

CPU:

1x H8/3337-HD64F3337F16 (main)
1x maybe NEC D7759GC (sound)
1x TDA2003 (audio amp)
2x ispLSI1032E-70LJ
1x oscillator 14.318180 MHz
1x oscillator 16.000 MHz

ROMs:

1x 27C1001 or similar (msg0)
4x 27C4001 or similar (ep1, ep2, ep3)

Note:

1x 28x2 edge connector (not JAMMA)
1x RS232 connector
1x 14 legs connector
1x 34 legs connector
2x 8 DIP switches
1x battery
1x trimmer (volume)
1x trimmer (unknown)

--------------------------

Silkscreened on PCB:
"BV 2-0257/A"

*/

ROM_START( millsun )
	ROM_REGION( 0x1000000, "main", 0 ) /* all the program code is in here */
	ROM_LOAD( "millsun_hd64f3337f16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* gfx seems 4bpp */
	ROM_LOAD( "msun_ep1.bin", 0x000000, 0x80000, CRC(06f10795) SHA1(f88a36e11f8ba38439aa066dc013427f204be3d7) )
	ROM_LOAD( "msun_ep2.bin", 0x080000, 0x80000, CRC(f85d10e6) SHA1(d33017c4a4883a4c9c76132deb5c57eb38f9fdb3) )
	ROM_LOAD( "msun_ep3.bin", 0x100000, 0x80000, CRC(329d380c) SHA1(618a7010fca8be6c368c3cc09fe129d8a4c72087) )
	ROM_LOAD( "msun_ep4.bin", 0x180000, 0x80000, CRC(071f5257) SHA1(891116086f5ce99327d9752c99465c25bd6dd69e) )

	ROM_REGION( 0x20000, "upd", 0 ) /* NEC D7759GC samples */
	ROM_LOAD( "msun_msg0.bin", 0x00000, 0x20000, CRC(b4bfbbb9) SHA1(ba2d6555f169273fa43de320614a5ea3ba2857e8) )
ROM_END


/* Super Space 2001

CPU:

1x H8/3337-HD64F3337F16 (main)
1x maybe NEC D7759GC (sound)
1x TDA2003 (audio amp)
2x ispLSI1032E-70LJ
1x oscillator 14.318180 MHz
1x oscillator 16.000 MHz

ROMs:

1x 27C1001 or similar (msg0)
4x 27C4001 or similar (ep1, ep2, ep3)

Note:

1x 28x2 edge connector (not JAMMA)
1x RS232 connector
1x 14 legs connector
1x 34 legs connector
2x 8 DIP switches
1x battery
1x trimmer (volume)
1x trimmer (unknown)

--------------------------

Silkscreened on PCB:
"BV 2-0257/A"

*/

ROM_START( sspac2k1 )
	ROM_REGION( 0x1000000, "main", 0 ) /* all the program code is in here */
	ROM_LOAD( "sspac2k1_hd64f3337f16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* gfx seems 4bpp */
	ROM_LOAD( "sup_spaces_ep1.bin", 0x000000, 0x80000, CRC(d512ee80) SHA1(f113218899394bf1dfe81518746414c4eda9a94c) )
	ROM_LOAD( "sup_spaces_ep2.bin", 0x080000, 0x80000, CRC(775eb938) SHA1(a83851ea6d90aaf3cad064cdbcc8379eed3d90ca) )
	ROM_LOAD( "sup_spaces_ep3.bin", 0x100000, 0x80000, CRC(d1d9c06c) SHA1(64993b5572201cc2c29d8900a89f036e96221e05) )
	ROM_LOAD( "sup_spaces_ep4.bin", 0x180000, 0x80000, CRC(0c02ad49) SHA1(64b382bf6dabf08229324807c6b66e600f38039d) )

	ROM_REGION( 0x20000, "upd", 0 ) /* NEC D7759GC samples */
	ROM_LOAD( "sup_spaces_msg0.bin", 0x00000, 0x20000, CRC(93edd0ad) SHA1(f122e147c918c6cb12043008ede729d6e0a4e543) )
ROM_END


/* Elvis?

CPU:

1x H8/3337-HD64F3337CP16
1x NEC D7759GC (sound)
1x TDA2003 (audio amp)
2x ispLSI1032E-70LJ
1x oscillator 14.318180 MHz
1x oscillator 16.000 MHz

ROMs:

1x 27C1001 or similar (0)
3x 27C4001 or similar (1, 2, 3)

Note:

1x 28x2 edge connector (not JAMMA)
1x RS232 connector
1x 14 legs connector
2x 8 DIP switches
1x battery
1x trimmer (volume)
1x trimmer (unknown)

--------------------------

Silkscreened on PCB:
"2-0250"

*/

ROM_START( elvis )
	ROM_REGION( 0x1000000, "main", 0 ) /* all the program code is in here */
	ROM_LOAD( "elvis_hd64f3337cp16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "1.bin", 0x000000, 0x80000, CRC(9e15983f) SHA1(272673ac9685cf0f5cc8a9263c91e4f93c30197f) )
	ROM_LOAD( "2.bin", 0x080000, 0x80000, CRC(c420af73) SHA1(fb0e03456a4b2f18c35d5ee2efeb29e3f2f26eae) )
	ROM_LOAD( "3.bin", 0x100000, 0x80000, CRC(bc10b1b6) SHA1(ef25f974cd0b44b91a8db215ff8d2dd3f4313bd8) )

	ROM_REGION( 0x20000, "upd", 0 ) /* NEC D7759GC samples */
	ROM_LOAD( "0.bin", 0x00000, 0x20000, CRC(833c5be5) SHA1(89110cb52265ee5bfdf73c0af343b7ce2356e394) )
ROM_END


/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME      PARENT  MACHINE   INPUT     INIT ROT    COMPANY    FULLNAME                          FLAGS  */
GAME( 1999, ntcash,   0,      itgambl2, itgambl2, 0,   ROT0, "Unknown", "NtCash",                          GAME_NOT_WORKING )
GAME( 1999, wizard,   0,      itgambl2, itgambl2, 0,   ROT0, "A.A.",    "Wizard (Ver 1.0)",                GAME_NOT_WORKING )
GAME( 2001, laser2k1, 0,      itgambl2, itgambl2, 0,   ROT0, "Unknown", "Laser 2001 (Ver 1.2)",            GAME_NOT_WORKING )
GAME( 2001, mdrink,   0,      itgambl2, itgambl2, 0,   ROT0, "Unknown", "Magic Drink (Ver 1.2)",           GAME_NOT_WORKING )
GAME( 2001, te0144,   0,      itgambl2, itgambl2, 0,   ROT0, "Unknown", "Unknown italian gambling game",   GAME_NOT_WORKING )
GAME( 200?, cmagica,  0,      itgambl2, itgambl2, 0,   ROT0, "Unknown", "Carta Magica (Ver 1.8)",          GAME_NOT_WORKING )
GAME( 200?, millsun,  0,      itgambl2, itgambl2, 0,   ROT0, "Unknown", "Millennium Sun",                  GAME_NOT_WORKING )
GAME( 200?, sspac2k1, 0,      itgambl2, itgambl2, 0,   ROT0, "Unknown", "Super Space 2001",                GAME_NOT_WORKING )
GAME( 200?, elvis,    0,      itgambl2, itgambl2, 0,   ROT0, "Unknown", "Elvis?",                          GAME_NOT_WORKING )
