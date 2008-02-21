/**************************************************************************
 *
 * 39in1.c - bootleg MAME-based "39-in-1" arcade PCB
 * Skeleton by R. Belmont, thanks to the Guru
 *
 * The program ROM appears to be encrypted, please help if you can!
 *
 * CPU: Intel Xscale PXA255 series @ 200 MHz, configured little-endian
 * Xscale PXA consists of:
 *    ARMv5 instruction set without the FPU
 *    ARM DSP extensions
 *    VGA-ish frame buffer with some 2D acceleration features
 *    AC97 stereo audio CODEC
 *
 * PCB also contains a custom ASIC, probably used for the decryption
 *
 **************************************************************************/

#include "driver.h"
#include "video/generic.h"
#include "cpu/arm7/arm7.h"

static ADDRESS_MAP_START( 39in1_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x0007ffff) AM_ROM
	AM_RANGE(0xfff80000, 0xffffffff) AM_ROM AM_REGION(REGION_CPU1, 0)	// mirror to prevent crashing
ADDRESS_MAP_END

static INPUT_PORTS_START( 39in1 )
INPUT_PORTS_END

static MACHINE_DRIVER_START( 39in1 )
	MDRV_CPU_ADD(ARM7, 200000000)	// actually Xscale PXA255, but ARM7 is a compatible subset
	MDRV_CPU_PROGRAM_MAP(39in1_map,0)

	MDRV_PALETTE_LENGTH(32768)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(16777216/4, 308, 0,  240, 228, 0,  160)	// completely bogus for this h/w

	MDRV_VIDEO_START(generic_bitmapped)
	MDRV_VIDEO_UPDATE(generic_bitmapped)

	MDRV_SPEAKER_STANDARD_STEREO("left", "right")
MACHINE_DRIVER_END

ROM_START( 39in1 )
	// main program, appears to be encrypted
	ROM_REGION( 0x80000, REGION_CPU1, 0 )
        ROM_LOAD( "27c4096_plz-v001_ver.300.bin", 0x000000, 0x080000, CRC(9149dbc4) SHA1(40efe1f654f11474f75ae7fee1613f435dbede38) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x200000, REGION_USER1, 0 )
        ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) )
ROM_END

GAME(2004, 39in1, 0, 39in1, 39in1, 0, ROT0, "????", "39 in 1 MAME bootleg", GAME_NOT_WORKING|GAME_NO_SOUND)
