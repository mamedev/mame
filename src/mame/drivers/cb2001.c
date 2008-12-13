/*************************************************************************************************

  Cherry Bonus 2001  (c)2000/2001 Dyna


Produttore	Dyna
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

const UINT8 cb2001_decryption_table[256] = {
	0xe8,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, /* 00 */
//	pppp
	xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, /* 10 */
//	
	xxxx,0x8e,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, /* 20 */
//	     !!!!
	xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, /* 30 */
//	
	xxxx,0xea,xxxx,xxxx,xxxx,0xb0,xxxx,xxxx, xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, /* 40 */
//	     !!!!                gggg
	xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0x49,xxxx, xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, /* 50 */
//	                              ????
	xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xba,xxxx, /* 60 */
//	                                                                       gggg
	0xc3,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, /* 70 */
//	pppp
	xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, /* 80 */
//	
	xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, xxxx,xxxx,xxxx,xxxx,0xb9,xxxx,xxxx,xxxx, /* 90 */
//	                                                             ????
	xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xb8, xxxx,xxxx,0xbc,xxxx,xxxx,xxxx,xxxx,xxxx, /* A0 */
//	                                   !!!!            ????
	xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, /* B0 */
//	
	xxxx,xxxx,0xee,xxxx,xxxx,0x75,xxxx,xxxx, xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, /* C0 */
//	          ????           pppp
	xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, xxxx,xxxx,xxxx,xxxx,xxxx,0xfa,0xc7,xxxx, /* D0 */
//                                                                      gggg !!!!
	xxxx,xxxx,xxxx,0xc6,xxxx,xxxx,xxxx,xxxx, xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, /* E0 */
//	               !!!!
	xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, /* F0 */
//	
};

/* robiza notes:
9c -> it's a counter (like mov cw,#value) -> not sure the register (cw,bw,....)
56 -> decrement the counter (like dec cw) -> not sure the register (cw,bw,....)

aa -> ????

guessed:
45 -> b0 (mov al,#value)
6e -> ba (mov dw,#value)
c2 -> ee (out dw,al)
dd -> fa (di)

probably:
00 -> e8 (call)
41 -> ea (jmp_far)
70 -> c3 (ret)
c5 -> 75 (jne)

checked against gussun and quizf1 (start up code):
21 -> 8e
a7 -> b8
de -> c7
e3 -> c6

*/


VIDEO_START(cb2001)
{

}

VIDEO_UPDATE(cb2001)
{
	return 0;
}

static ADDRESS_MAP_START( cb2001_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0xdffff) AM_RAM
	AM_RANGE(0xe0000, 0xfffff) AM_ROM AM_REGION("boot_prg",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( cb2001_io, ADDRESS_SPACE_IO, 16 )
ADDRESS_MAP_END

static INPUT_PORTS_START( cb2001 )
INPUT_PORTS_END

static INTERRUPT_GEN( vblank_irq )
{
//	cpu_set_input_line_and_vector(device,0,HOLD_LINE,0x08/4);
}

static const nec_config cb2001_config = { cb2001_decryption_table, };
static MACHINE_DRIVER_START( cb2001 )
	MDRV_CPU_ADD("main", V30, 20000000) // CPU91A-011-0016JK004; encrypted cpu like nec v25/35 used in some irem game
	MDRV_CPU_CONFIG(cb2001_config)
	MDRV_CPU_PROGRAM_MAP(cb2001_map,0)
	MDRV_CPU_IO_MAP(cb2001_io,0)
	MDRV_CPU_VBLANK_INT("main", vblank_irq)

	MDRV_SCREEN_ADD("main", RASTER)
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
	ROM_REGION( 0x020000, "boot_prg", 0 )
	ROM_LOAD16_WORD( "c01111.11f", 0x000000, 0x20000, CRC(ec6269f1) SHA1(f2428562a10e30192f2c95053f5ce448302e7cf5) )

	ROM_REGION( 0x080000, "gfx", 0 ) // not tiles, blitter based, or mem mapped?
	ROM_LOAD( "c0111.12a", 0x000000, 0x80000, CRC(342b760e) SHA1(bc168bec384ccacd73543f604e3ab5b2b8f4f441) )

	ROM_REGION( 0x200, "user1", 0 ) // ?
	ROM_LOAD( "am27s29.9b", 0x000, 0x200, CRC(6c90f6a2) SHA1(f3f592954000d189ded0ed8c6c4444ace0b616a4) )
	ROM_REGION( 0x200, "user2", 0 ) // ?
	ROM_LOAD( "am27s29.11b", 0x000, 0x200, CRC(e5aa3ec7) SHA1(675711dd6788b3d0c37573b49b6297cbcd8c8209) )
ROM_END

GAME( 2001, cb2001,    0,      cb2001,      cb2001,   0, ROT0,  "Dyna", "Cherry Bonus 2001", GAME_NOT_WORKING|GAME_NO_SOUND )
