/*
Virtual Combat hardware games.
 
Driver by Jason Eckhardt and Andrew Gardner. 
 
----

There are two known games on this hardware.  Both are developed by
Kyle Hodgetts.

Virtual Combat (c) VR8 Inc. 1993
http://arcade.sonzogni.com/VRCombat/

Shadow Fighters (German) (c) Sega? 1989?

----

There are two boards to this hardware.  The upper, which contains the
graphics ROMs and the i860, and the lower which contains the main
and sound CPU's.  Virtual Combat sports two upper boards which presumably
output a different rasterization of the scene for each stereo eye.

UPPER:
    Intel I860 XR processor
    MB8298-25P-SK RAMS x12 (silkscreen said 62256)
    Analog device ADV476KN50E (silkscreen said BT476)
    20 MHZ Oscillator
    8-way DIP switch
    574200D x4
    PAL palce24v10 x2 (next to the i860)

LOWER:
    Motorola MC68000P12 x2
    12 MHz Oscillator x2
    Harris ADC0804LCN x2
    4 MB8298-25P-SK RAMS (in groups of 2 off by themselves)
    1 CXK58257SP-10L at each end of the SNDCPU ROMS and the CPU ROMS (4 chips total)
    Motorola MC6845P CRT controller
    2x 27C010A containing sound code
    Xx 27C040 containing sound data (VOC files)
    Dallas DS1220Y - closest to pin 64 of CPU - read as a 2716 - (silkscreened "6116")
    Xx 27c040 containing program code, etc.

----

NOTES : Shadow Fighters appears to have been dumped from an earlier
            revision of the hardware.  There are no IC labels, and
            lots of factory rework has been done to the bottom board.
        Because the board was so early for Shadow Fighters, there were
            no IC locations silkscreened on the PCB.  The locations
            from Virtual Combat have been used.
        The Shadow Fighters bottom board has an extra 20 mhz xtal on it.
        The data stored in "samples" is simply a series of
            Creative Media VOC files concatenated to eachother.
        The sound program ("sound") is about 640 bytes long.
        The graphics ROMs have had images successfully extracted from
            them.  Pictures for Shadow Fighters can be found online.
        The hardware is said to run at medium resolution.
        The SRAM module dump can likely be thrown away for both games.
        The PAL that's dumped for Shadow Fighters looks pretty bad.
        Websites seem to say Shadow Fighters is a SEGA game, but I
            couldn't find a SEGA string anywhere in the ROMs.  I also,
            however, could not find a VR8 string in the Virtual Combat
            ROMs, so who knows...  Kyle's name is easily found in both
            though :).

TODO :  This is a skeleton driver.  Nearly everything.
        i860XR-25 CPU core!

*/

#include <stdio.h>
#include "driver.h"
#include "cpu/m68000/m68000.h"
#include "video/generic.h"

static UINT16* framebuffer;


static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x200000, 0x2fffff) AM_RAM
	AM_RANGE(0x300000, 0x3fffff) AM_RAM AM_BASE(&framebuffer)

//	AM_RANGE(0x400000, 0x43ffff) i860 #1 shared RAM
//	AM_RANGE(0x440000, 0x440003) i860 #1 com 1
//	AM_RANGE(0x480000, 0x480003) i860 #1 com 2
//	AM_RANGE(0x4c0000, 0x4c0003) i860 #1 stop/start/reset

//	AM_RANGE(0x500000, 0x53ffff) i860 #2 shared RAM
//	AM_RANGE(0x540000, 0x540003) i860 #2 com 1
//	AM_RANGE(0x580000, 0x580003) i860 #2 com 2
//	AM_RANGE(0x5c0000, 0x5c0003) i860 #2 stop/start/reset

//	AM_RANGE(0x706000, 0x706003) Bt476 RAMDAC 
//	AM_RANGE(0x706004, 0x706007) Bt476 RAMDAC (not sure if there are 2?)
ADDRESS_MAP_END


// The first 3d CPU
//static ADDRESS_MAP_START( 3d_1_map, ADDRESS_SPACE_PROGRAM, ?? )
//	AM_RANGE(0x00000000, 0x000fffff) Shared framebuffer
//	AM_RANGE(0xfffc0000, 0xffffffff) Shared RAM (0x400000 in 68k-land)
//	AM_RANGE(0x20000000, 0x20000003) com 1      (0x440000 in 68k-land)
//	AM_RANGE(0x40000000, 0x401fffff) AM_ROM
//	AM_RANGE(0x80000000, 0x80000003) com 2      (0x480000 in 68k-land)
//ADDRESS_MAP_END


// The second 3d CPU
//static ADDRESS_MAP_START( 3d_2_map, ADDRESS_SPACE_PROGRAM, ?? )
//	AM_RANGE(0x00000000, 0x000fffff) Shared framebuffer
//	AM_RANGE(0xfffc0000, 0xffffffff) Shared RAM (0x500000 in 68k-land)
//	AM_RANGE(0x20000000, 0x20000003) com 1      (0x540000 in 68k-land)
//	AM_RANGE(0x40000000, 0x401fffff) AM_ROM
//	AM_RANGE(0x80000000, 0x80000003) com 2      (0x580000 in 68k-land)
//ADDRESS_MAP_END


// Sound CPU - temprarily disabled
//static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 16 )
//	AM_RANGE(0x000000, 0x03ffff) AM_ROM
//ADDRESS_MAP_END


static DRIVER_INIT( vcombat )
{
	UINT8 *ROM = memory_region(machine, "main");

	// pc==4016 : jump 4038 ... There's something strange about how it waits at 402e (interrupts all masked out)
	ROM[0x4017] = 0x66;
	// pc==40fa : jump 40fc ... 600004 should be -16? (interrupts still masked out)
	ROM[0x40fb] = 0x67;
	// pc==410e : jump 4110 ... 600004 should be 31 now? (interrupts still masked out)
	ROM[0x410f] = 0x67;
	// pc==e220 : jump e222 ... 20119a should not be 0. (no interrupts masked)
	// TODO: I wonder if this is an input bit or something.  The menu selection continually crawls up.
}


static INPUT_PORTS_START( vcombat )
INPUT_PORTS_END


static VIDEO_UPDATE( vcombat )
{
	int x, y;
	int count = 0;

	for(y = 0; y < 480; y++)
	{
		for(x = 0; x < 256; x++)
		{
			//UINT32 r,g,b;
			UINT32 color;

			if (x % 2) color = (framebuffer[count] & 0xff00) >> 8;
			else	   color =  framebuffer[count] & 0x00ff;

			if(x < video_screen_get_visible_area(screen)->max_x && y < video_screen_get_visible_area(screen)->max_y)
				*BITMAP_ADDR32(bitmap, y, x) = color | (color<<8) | (color<<16);

			if (x % 2) count++;
		}
	}
	return 0;
}


// This is just here to show that there is text in the main program ROM.  It probably won't really be a useful decode.
static const gfx_layout vcombat_charlayout =
{
	8, 8,
	0x100000 / 0x80,
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0*16,   1*16,   2*16,   3*16,   4*16,   5*16,   6*16,   7*16 },
	{ 0*16*8, 1*16*8, 2*16*8, 3*16*8, 4*16*8, 5*16*8, 6*16*8, 7*16*8 },
	8 * 0x80
};


static GFXDECODE_START( vcombat )
	GFXDECODE_ENTRY( "main", 0, vcombat_charlayout, 0, 256 )
GFXDECODE_END


static MACHINE_DRIVER_START( vcombat )
	MDRV_CPU_ADD("main", M68000, XTAL_12MHz)
	MDRV_CPU_PROGRAM_MAP(main_map,0)
	//MDRV_CPU_VBLANK_INT("main", irq7_line_hold)

	// Disabled for now
	//MDRV_CPU_ADD("sound", M68000, XTAL_12MHz)
	//MDRV_CPU_PROGRAM_MAP(sound_map,0)

	// Likely will go away
	MDRV_GFXDECODE(vcombat)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(640, 480)
	MDRV_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)

	// Seems there is a RAMDAC out there.  Maybe it's for the palette?
	/*MDRV_PALETTE_LENGTH(0x256)*/

	MDRV_VIDEO_UPDATE(vcombat)
MACHINE_DRIVER_END


ROM_START( vcombat )
	ROM_REGION( 0x100000, "main", 0 )
	ROM_LOAD16_BYTE( "ep8v2.b49", 0x00000, 0x80000, CRC(98d5a45d) SHA1(099e314f11c93ad6e642ceaa311e2a5b6fd7193c) )
	ROM_LOAD16_BYTE( "ep7v2.b51", 0x00001, 0x80000, CRC(06185bcb) SHA1(855b11ae7644d6c7c1c935b2f5aec484071ca870) )

	ROM_REGION( 0x40000, "sound", 0 )
	ROM_LOAD16_BYTE( "ep1v2.b42", 0x00000, 0x20000, CRC(560b2e6c) SHA1(e35c0466a1e14beab080e3155f873e9c2a1c028b) )
	ROM_LOAD16_BYTE( "ep6v2.b33", 0x00001, 0x20000, CRC(37928a5d) SHA1(7850be26dbd356cdeef2a0d87738de16420f6291) )

	ROM_REGION( 0x180000, "samples", 0 )
	ROM_LOAD16_BYTE( "ep2v2.b41", 0x000000, 0x80000, CRC(7dad3458) SHA1(deae5ebef0346250d3f9744933423253a336bb67) )
	ROM_LOAD16_BYTE( "ep4v2.b37", 0x000001, 0x80000, CRC(b0be2e91) SHA1(66f3a9f5abeb4b95ac806e4bb165f938dca38b2d) )
	ROM_LOAD16_BYTE( "ep3v2.b40", 0x100000, 0x40000, CRC(8c491526) SHA1(95c6bcbe0adcfffb12fd2b86c9f4ca26aa188bbf) )
	ROM_LOAD16_BYTE( "ep5v2.b36", 0x100001, 0x40000, CRC(7592b2eb) SHA1(92a540726306d7adbf207fe86a4c4fa66958f90b) )

	ROM_REGION( 0x800, "user1", 0 )	/* The SRAM module */
	ROM_LOAD( "ds1220y.b53", 0x000, 0x800, CRC(b21cfe5f) SHA1(898ace3cd0913ea4b0dc84320219777773ef856f) )

	/* These roms are identical on both of the upper boards */
	ROM_REGION( 0x200000, "3d", 0 )
	ROM_LOAD16_BYTE( "11.u56", 0x000000, 0x80000, CRC(a83094ce) SHA1(c3512375fecdb5e7eb02a4aa140ae4efe0233cb8) )
	ROM_LOAD16_BYTE( "9.u54",  0x000001, 0x80000, CRC(a276e18b) SHA1(6d60e519196a4858b82241504592413df498e12f) )
	ROM_LOAD16_BYTE( "12.u57", 0x100000, 0x80000, CRC(0cdffd4f) SHA1(65ace78711b3ef6e0ff9a7ad7343b5558e652f6c) )
	ROM_LOAD16_BYTE( "10.u55", 0x100001, 0x80000, CRC(8921f20e) SHA1(6e9ca2eaad3e1108ba0e1d7792fd5d0305bec201) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "pal1_w2.u51", 0x000, 0x1f1, CRC(af497420) SHA1(03aa82189d91ae194dd5a6e7b9dbdb7cd473ddb6) )
	ROM_LOAD( "pal2_w2.u52", 0x200, 0x1f1, CRC(4a6df05d) SHA1(236b951e5daf927c050d0f35558c171a020156ab) )
ROM_END

ROM_START( shadfgtr )
	ROM_REGION( 0x100000, "main", 0 )
	ROM_LOAD16_BYTE( "shadfgtr.b49", 0x00000, 0x80000, CRC(2d9d31a1) SHA1(45854915bcb9db2e4076a7f26a0a349077cd10bc) )
	ROM_LOAD16_BYTE( "shadfgtr.b51", 0x00001, 0x80000, CRC(03d0f075) SHA1(06013a4363305a23d7e8ba8fe2fa961cd540391d) )

	ROM_REGION( 0x40000, "sound", 0 )
	ROM_LOAD16_BYTE( "shadfgtr.b42", 0x00000, 0x20000, CRC(f8605dcd) SHA1(1b29f47856ccc757bc96674682ae48f87e6b0e54) )
	ROM_LOAD16_BYTE( "shadfgtr.b33", 0x00001, 0x20000, CRC(291d59ac) SHA1(cc4904c2ac8ef6a12033c10893246a438ac44014) )

	ROM_REGION( 0x100000, "samples", 0 )
	ROM_LOAD16_BYTE( "shadfgtr.b41", 0x00000, 0x80000, CRC(9e4b4df3) SHA1(8101197275e9f728acdeef85737eecbdec132b27) )
	ROM_LOAD16_BYTE( "shadfgtr.b37", 0x00001, 0x80000, CRC(98446ba2) SHA1(1c8cc0d9c5de54d9e53699a5ab281579d15edc96) )

	ROM_REGION( 0x800, "user1", 0 )	/* The SRAM module */
	ROM_LOAD( "shadfgtr.b53", 0x000, 0x800, CRC(e766a3ab) SHA1(e7696ec08d5c86f64d768480f43edbd19ded162d) )

	/* These roms are identical on both of the upper boards */
	ROM_REGION( 0x200000, "3d", 0 )
	ROM_LOAD16_BYTE( "shadfgtr.u56", 0x000000, 0x80000, CRC(fb76db5a) SHA1(fa546f465df113c13037abed1162bfa6f9b1dc9b) )
	ROM_LOAD16_BYTE( "shadfgtr.u54", 0x000001, 0x80000, CRC(c45d68d6) SHA1(a133e4f13d3af18bccf0d060a659d64ac699b159) )
	ROM_LOAD16_BYTE( "shadfgtr.u57", 0x100000, 0x80000, CRC(60d701d7) SHA1(936473b5e3b2e9e9e3b50cf977fc5a670a097850) )
	ROM_LOAD16_BYTE( "shadfgtr.u55", 0x100001, 0x80000, CRC(e807631d) SHA1(9027ff7dc60b808434dac292c08f0630d3d52186) )

	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "shadfgtr.u51", 0x000, 0x1f1, CRC(bab58337) SHA1(c4a79c8e53aeadb7f64d49d214b607b5b36f144e) )
	/* The second upper-board PAL couldn't be read */
ROM_END

/*    YEAR  NAME      PARENT  MACHINE  INPUT    INIT     MONITOR COMPANY     FULLNAME           FLAGS */
GAME( 1993, vcombat,  0,      vcombat, vcombat, vcombat, ROT0,   "VR8 Inc.", "Virtual Combat",  GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 1989, shadfgtr, 0,      vcombat, vcombat, 0,       ROT0,   "Sega?",    "Shadow Fighters", GAME_NOT_WORKING | GAME_NO_SOUND )
