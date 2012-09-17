/*
    Hewlett-Packard HP16500b Logic Analyzer

    MC68EC030 @ 25 MHz

    WD37C65C floppy controller (NEC765 type)
    Bt475 video RAMDAC
    TMS9914A GPIB bus interface
    Dallas DS1286 RTC/CMOS RAM

    IRQ 1 = 17732
    IRQ 2 = 35b8
    IRQ 3 = 35ce (jump 840120)
    IRQ 4 = 17768
    IRQ 5 = 814a
    IRQ 6 = 35c8 (jump 840120)
    IRQ 7 = 35d4 (jump 840120)
*/


#include "emu.h"
#include "cpu/m68000/m68000.h"


class hp16500_state : public driver_device
{
public:
	hp16500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	virtual void video_start();
	UINT32 screen_update_hp16500(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};


static ADDRESS_MAP_START(hp16500_map, AS_PROGRAM, 32, hp16500_state)
	AM_RANGE(0x00000000, 0x0001ffff) AM_ROM AM_REGION("bios", 0)
	AM_RANGE(0x00600000, 0x0063ffff) AM_RAM
	AM_RANGE(0x00800000, 0x009fffff) AM_RAM	    // 284e end of test - d0 = 0 for pass
ADDRESS_MAP_END

void hp16500_state::video_start()
{
}

UINT32 hp16500_state::screen_update_hp16500(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static MACHINE_CONFIG_START( hp16500, hp16500_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68EC030, 25000000)
	MCFG_CPU_PROGRAM_MAP(hp16500_map)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(25175000, 800, 0, 640, 525, 0, 480)

	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_SIZE(1024, 768)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(hp16500_state, screen_update_hp16500)

	MCFG_PALETTE_LENGTH(256)


	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
MACHINE_CONFIG_END

static INPUT_PORTS_START( hp16500 )
INPUT_PORTS_END

ROM_START( hp16500b )
	ROM_REGION32_BE(0x20000, "bios", 0)
        ROM_LOAD32_BYTE( "16500-80014.bin", 0x000000, 0x008000, CRC(35187716) SHA1(82067737892ecd356a5454a43d9ce9b38ac2397f) )
        ROM_LOAD32_BYTE( "16500-80015.bin", 0x000001, 0x008000, CRC(d8d26c1c) SHA1(b5b956c17c9d6e54519a686b5e4aa733b266bf6f) )
        ROM_LOAD32_BYTE( "16500-80016.bin", 0x000002, 0x008000, CRC(61457b39) SHA1(f209315ec22a8ee9d44a0ec009b1afb47794bece) )
        ROM_LOAD32_BYTE( "16500-80017.bin", 0x000003, 0x008000, CRC(e0b1096b) SHA1(426bb9a4756d8087bded4f6b61365d733ffbb09a) )
ROM_END

COMP( 1994, hp16500b, 0, 0, hp16500, hp16500, driver_device, 0,  "Hewlett Packard", "HP 16500b", GAME_NOT_WORKING|GAME_NO_SOUND)
