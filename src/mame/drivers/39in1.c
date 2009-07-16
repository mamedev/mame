/**************************************************************************
 *
 * 39in1.c - bootleg MAME-based "39-in-1" arcade PCB
 * Skeleton by R. Belmont, thanks to the Guru
 * Decrypt by Andreas Naive
 *
 * CPU: Intel Xscale PXA255 series @ 200 MHz, configured little-endian
 * Xscale PXA consists of:
 *    ARMv5TE instruction set without the FPU
 *    ARM standard MMU
 *    ARM DSP extensions
 *    VGA-ish frame buffer with some 2D acceleration features
 *    AC97 stereo audio CODEC
 *
 * PCB also contains a custom ASIC, probably used for the decryption
 *
 * TODO:
 *   Figure out weird memory test subroutine at 1c8
 *   PXA255 MMU and peripherals
 *
 **************************************************************************/

#include "driver.h"
#include "video/generic.h"
#include "cpu/arm7/arm7.h"

static UINT32 counter_timer;

// free-running 3.something MHz counter.  this is a complete hack.
static READ32_HANDLER( os_timer_counter_r )
{
	UINT32 ret;

	ret = counter_timer;
	counter_timer += 0x300;

	return ret;
}

static ADDRESS_MAP_START( 39in1_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x0007ffff) AM_ROM
	AM_RANGE(0x40a00010, 0x40a00013) AM_READ( os_timer_counter_r )
	AM_RANGE(0xa0000000, 0xa3ffffff) AM_RAM
ADDRESS_MAP_END

static INPUT_PORTS_START( 39in1 )
INPUT_PORTS_END

static VIDEO_UPDATE( 39in1 )
{
	return 0;
}

static MACHINE_START(39in1)
{
	UINT8 *ROM = memory_region(machine, "maincpu");
	int i;

	for (i = 0; i < 0x80000; i += 2)
	{
		ROM[i] = BITSWAP8(ROM[i],7,2,5,6,0,3,1,4) ^ BITSWAP8((i>>3)&0xf, 3,2,4,1,4,4,0,4) ^ 0x90;
	}

	counter_timer = 0x300;
}

static MACHINE_DRIVER_START( 39in1 )
	MDRV_CPU_ADD("maincpu", ARM9, 200000000)	// actually Xscale PXA255, but ARM9 is a compatible subset
	MDRV_CPU_PROGRAM_MAP(39in1_map)

	MDRV_PALETTE_LENGTH(32768)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(16777216/4, 308, 0,  240, 228, 0,  160)	// completely bogus for this h/w

	MDRV_MACHINE_START(39in1)

	MDRV_VIDEO_UPDATE(39in1)

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
MACHINE_DRIVER_END

ROM_START( 39in1 )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
        ROM_LOAD( "27c4096_plz-v001_ver.300.bin", 0x000000, 0x080000, CRC(9149dbc4) SHA1(40efe1f654f11474f75ae7fee1613f435dbede38) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x200000, "data", 0 )
        ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) )
ROM_END

GAME(2004, 39in1, 0, 39in1, 39in1, 0, ROT0, "????", "39 in 1 MAME bootleg", GAME_NOT_WORKING|GAME_NO_SOUND)

/*

cpu #0 (PC=00000010): unmapped program memory dword write to 40E00024 = FFFFFFFF & FFFFFFFF      GPIO pin output clear 31:0
cpu #0 (PC=00000014): unmapped program memory dword write to 40E00028 = FFFFFFFF & FFFFFFFF                63:32
cpu #0 (PC=00000018): unmapped program memory dword write to 40E0002C = FFFFFFFF & FFFFFFFF                80:64
cpu #0 (PC=00000028): unmapped program memory dword write to 40E00018 = 01008000 & FFFFFFFF      GPIO pin direction register 63:32
cpu #0 (PC=0000002C): unmapped program memory dword write to 40E0001C = 00020802 & FFFFFFFF           pin output set   63:32
cpu #0 (PC=00000030): unmapped program memory dword write to 40E00020 = 0001CC00 & FFFFFFFF                80:64
cpu #0 (PC=00000040): unmapped program memory dword write to 40E0000C = DFE3FFDC & FFFFFFFF      GPIO pin direction    31:0
cpu #0 (PC=00000044): unmapped program memory dword write to 40E00010 = FC9FAF83 & FFFFFFFF                63:32
cpu #0 (PC=00000048): unmapped program memory dword write to 40E00014 = 0001FFFF & FFFFFFFF
cpu #0 (PC=00000064): unmapped program memory dword write to 40E00054 = 80000000 & FFFFFFFF      GPIO alternate function
cpu #0 (PC=00000068): unmapped program memory dword write to 40E00058 = 59000110 & FFFFFFFF
cpu #0 (PC=0000006C): unmapped program memory dword write to 40E0005C = 600A9559 & FFFFFFFF
cpu #0 (PC=00000070): unmapped program memory dword write to 40E00060 = AAA00008 & FFFFFFFF
cpu #0 (PC=00000074): unmapped program memory dword write to 40E00064 = AAAAAAAA & FFFFFFFF
cpu #0 (PC=00000078): unmapped program memory dword write to 40E00068 = 00000002 & FFFFFFFF
cpu #0 (PC=00000084): unmapped program memory dword write to 40F00004 = 00000020 & FFFFFFFF      power manager sleep status
test_rt_r_callback opcode=ee160e10
test_rt_w_callback opcode=ee060e10, data from ARM7 register=2
cpu #0 (PC=0000009C): unmapped program memory dword write to 41300000 = 00000141 & FFFFFFFF      core clock configure (multiplier=27, RAM is 99.53 MHz + multiplier = 2 * memory + turbo = 1x)
test_rt_w_callback opcode=ee060e10, data from ARM7 register=3
test_rt_w_callback opcode=ee060e10, data from ARM7 register=1
cpu #0 (PC=000000B4): unmapped program memory dword read from 40A00010 & FFFFFFFF            OS timer/counter
cpu #0 (PC=000000B8): unmapped program memory dword read from 40A00010 & FFFFFFFF
cpu #0 (PC=000000B8): unmapped program memory dword read from 40A00010 & FFFFFFFF

*/
