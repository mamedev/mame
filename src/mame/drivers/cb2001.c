/*************************************************************************************************

  Cherry Bonus 2001  (c)2000/2001 Dyna


Produttore  Dyna
N.revisione
CPU

1x DYNA CPU91A-011-0016JK004 (QFP84) custom
1x DYNA DC3001-0051A (QFP128) custom
1x DYNA 22A078803 (DIP42) (I think it's an I/O)
1x WINBOND WF19054 (equivalent to AY-3-8910)
1x oscillator 24.000MHz

ROMs

1x M27C4002 (12a)
1x M27C1001 (11f)
2x AM27S29PC (9b,11b)
2x GAL16V8D (not dumped)


Note

1x 36x2 edge connector
1x 10x2 edge connector
1x trimmer (volume)
1x pushbutton (sw8)
1x battery
7x 8x switches dip (sw1-7)
------------------------------

In title screen (c) is 2001
In test mode (c) is 2000

------------------------------

*************************************************************************************************/

#include "driver.h"
#include "cpu/nec/nec.h"

#define xxxx 0x90 /* Unknown */

static const UINT8 cb2001_decryption_table[256] = {
	0xe8,xxxx,xxxx,xxxx,xxxx,0x61,xxxx,xxxx, xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, /* 00 */
//  pppp                     ????
	xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, 0x32,xxxx,0xa0,xxxx,0x3a,xxxx,xxxx,0x1f, /* 10 */
//                                           pppp     ????       pppp           ????
	xxxx,0x8e,xxxx,0x0f,xxxx,0x49,0xbc,xxxx, xxxx,xxxx,xxxx,0x75,xxxx,xxxx,xxxx,xxxx, /* 20 */
//       !!!!      ????      ???? ????                      pppp
	0x9d,xxxx,xxxx,xxxx,xxxx,xxxx,0xbe,xxxx, xxxx,xxxx,0x74,xxxx,xxxx,0xa6,0xbf,xxxx, /* 30 */
//  ????                          pppp                 ????           ???? ????
	xxxx,0xea,xxxx,xxxx,xxxx,0xb0,xxxx,xxxx, xxxx,0xa2,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, /* 40 */
//       !!!!                gggg                 ????
	xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0x42,xxxx, xxxx,xxxx,xxxx,xxxx,0xeb,xxxx,xxxx,xxxx, /* 50 */
//                                ????                           ????
	xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, xxxx,0xa5,xxxx,xxxx,xxxx,xxxx,0xba,xxxx, /* 60 */
//                                                ????                     gggg
	0xc3,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, 0x72,xxxx,0xf2,xxxx,xxxx,xxxx,xxxx,xxxx, /* 70 */
//  pppp                                     ????      ????
	xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0x34, xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, /* 80 */
//                                     ????
	xxxx,xxxx,0xe9,xxxx,xxxx,0xbe,xxxx,xxxx, xxxx,xxxx,xxxx,xxxx,0xb9,xxxx,xxxx,xxxx, /* 90 */
//            ????           ????                                pppp
	xxxx,xxxx,xxxx,0x06,0xaa,0x9c,xxxx,0xb8, xxxx,xxxx,0xfc,xxxx,xxxx,xxxx,xxxx,xxxx, /* A0 */
//                 ???? ???? ????      !!!!            ????
	xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, xxxx,xxxx,xxxx,xxxx,0x1e,xxxx,0x07,0xcf, /* B0 */
//                                                               ????      ???? ????
	xxxx,xxxx,0xee,xxxx,xxxx,0xe2,xxxx,xxxx, xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xa4,xxxx, /* C0 */
//            ????           pppp                                          ????
	xxxx,xxxx,0x46,xxxx,0x60,xxxx,xxxx,xxxx, 0x88,xxxx,xxxx,xxxx,xxxx,0xfa,0xc7,xxxx, /* D0 */
//              pppp      ????                 pppp                     ???? !!!!
	0x8a,xxxx,xxxx,0xc6,xxxx,xxxx,xxxx,xxxx, xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, /* E0 */
//  ????           !!!!
	xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, /* F0 */
//
};

/* robiza notes:

56 -> ????

5c -> conditional jmp for sure

aa -> ????
92 -> e9 (probably)
1c ????
d8 ????
dd -> fa (di)

guessed:
45 -> b0 (mov al,#value)
6e -> ba (mov dw,#value)
c2 -> ee (out dw,al)

probably:

2b -> conditional jmp for sure (75)
36 -> be
9c -> it's a counter (like mov cw,#value) -> not sure the register (cw,bw,....) -> b9 (cw)
c5 -> 75 (loop?)

very probably:
00 -> e8 (call)
41 -> ea (jmp_far)
70 -> c3 (ret)

checked against gussun and quizf1 (start up code):
21 -> 8e
a7 -> b8
de -> c7
e3 -> c6

opcodes: 36,9c,00,18,d8,d2,c5,70 probably:
e1af1 36 62 06 mov ix,0662
      9c 04 00 mov cw,0004
      00 94 17 call e328e

e328e 18 c0 xor al,al
      d8 04 mov byte ptr [ix],al
      d2    inc ix
      c5 fb dbnz e3290
      70    ret
*/


static VIDEO_START(cb2001)
{

}

static VIDEO_UPDATE(cb2001)
{
	return 0;
}

static ADDRESS_MAP_START( cb2001_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0xbffff) AM_RAM
	AM_RANGE(0xc0000, 0xfffff) AM_ROM AM_REGION("boot_prg",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( cb2001_io, ADDRESS_SPACE_IO, 16 )
ADDRESS_MAP_END

static INPUT_PORTS_START( cb2001 )
INPUT_PORTS_END

static INTERRUPT_GEN( vblank_irq )
{
//  cpu_set_input_line_and_vector(device,0,HOLD_LINE,0x08/4);
}

static const gfx_layout cb2001_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 8,12,0,4,24,28, 16,20 },
	{ 0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32 },
	8*32
};

static const gfx_layout cb2001_layout32 =
	{
	8,32,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 8,12,0,4,24,28, 16,20 },
	{ 0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32, 8*32, 9*32, 10*32, 11*32, 12*32, 13*32,14*32,15*32,16*32,17*32,18*32,19*32,20*32,21*32,22*32,23*32,24*32,25*32,26*32,27*32,28*32,29*32,30*32,31*32 },
	32*32
};

static GFXDECODE_START( cb2001 )
	GFXDECODE_ENTRY( "gfx", 0, cb2001_layout,   0x0, 32  )
	GFXDECODE_ENTRY( "gfx", 0, cb2001_layout32, 0x0, 32 )
GFXDECODE_END

static PALETTE_INIT(cb2001)
{
	int i;
	for (i = 0; i < 0x200; i++)
	{
		int r,g,b;

		UINT8*proms = memory_region(machine, "proms");
		UINT16 dat;

		dat = (proms[0x000+i] << 8) | proms[0x200+i];


		b = ((dat >> 1) & 0x1f)<<3;
		r = ((dat >> 6 )& 0x1f)<<3;
		g = ((dat >> 11 ) & 0x1f)<<3;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}


static const nec_config cb2001_config = { cb2001_decryption_table, };
static MACHINE_DRIVER_START( cb2001 )
	MDRV_CPU_ADD("maincpu", V30, 20000000) // CPU91A-011-0016JK004; encrypted cpu like nec v25/35 used in some irem game
	MDRV_CPU_CONFIG(cb2001_config)
	MDRV_CPU_PROGRAM_MAP(cb2001_map)
	MDRV_CPU_IO_MAP(cb2001_io)
	MDRV_CPU_VBLANK_INT("screen", vblank_irq)

	MDRV_GFXDECODE(cb2001)

	MDRV_PALETTE_INIT( cb2001 )

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(64*8, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(0, 64*8-1, 0, 32*8-1)

	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(cb2001)
	MDRV_VIDEO_UPDATE(cb2001)
MACHINE_DRIVER_END


ROM_START( cb2001 )
	ROM_REGION( 0x040000, "boot_prg", 0 )
	ROM_LOAD16_WORD( "c01111.11f", 0x020000, 0x20000, CRC(ec6269f1) SHA1(f2428562a10e30192f2c95053f5ce448302e7cf5) )

	ROM_REGION( 0x080000, "gfx", 0 )
	ROM_LOAD( "c0111.12a", 0x000000, 0x80000, CRC(342b760e) SHA1(bc168bec384ccacd73543f604e3ab5b2b8f4f441) )

	ROM_REGION( 0x400, "proms", 0 ) // ?
	ROM_LOAD( "am27s29.9b",  0x000, 0x200, CRC(6c90f6a2) SHA1(f3f592954000d189ded0ed8c6c4444ace0b616a4) )
	ROM_LOAD( "am27s29.11b", 0x200, 0x200, CRC(e5aa3ec7) SHA1(675711dd6788b3d0c37573b49b6297cbcd8c8209) )
ROM_END

ROM_START( scherrym )
	ROM_REGION( 0x040000, "boot_prg", 0 )
	ROM_LOAD16_WORD( "f11.bin", 0x000000, 0x40000, CRC(8967f58d) SHA1(eb01a16b7d108f5fbe5de8f611b4f77869aedbf1) )

	ROM_REGION( 0x080000, "gfx", 0 )
	ROM_LOAD( "12a.bin", 0x000000, 0x80000,NO_DUMP ) // missing on PCB

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "n82s135-1.bin", 0x000, 0x100, CRC(66ed363f) SHA1(65bd37842c441c2e712844b07c0cfe37ef16d0ef) )
	ROM_LOAD( "n82s135-2.bin", 0x200, 0x100, CRC(a19821db) SHA1(62dda90dd67dfbc0b96f161f1f2b7a46a5805eae) )
ROM_END

GAME( 2001, cb2001,    0,      cb2001,      cb2001,   0, ROT0,  "Dyna", "Cherry Bonus 2001", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2001, scherrym,  0,      cb2001,      cb2001,   0, ROT0,  "Dyna", "Super Cherry Master", GAME_NOT_WORKING|GAME_NO_SOUND ) // 2001 version? (we have bootlegs running on z80 hw of a 1996 version)

