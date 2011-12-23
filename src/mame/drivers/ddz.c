/*
    Dou Di Zhu (by IGS?)

    uses Crystal System-like hardware (same CPU + Video Chip, but single PCB)
    meant to be some kind of poker game?

    Rom appears to be encrypted / scrambled

*/

#include "emu.h"
#include "cpu/se3208/se3208.h"
#include "video/vrender0.h"
#include "machine/ds1302.h"
#include "machine/nvram.h"
#include "sound/vrender0.h"


class ddz_state : public driver_device
{
public:
	ddz_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

};


static ADDRESS_MAP_START( ddz_mem, AS_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x003fffff) AM_ROM AM_WRITENOP
ADDRESS_MAP_END

static VIDEO_START(ddz)
{
}


static SCREEN_UPDATE(ddz)
{
	return 0;
}

static SCREEN_EOF(ddz)
{

}

static INTERRUPT_GEN(ddz_interrupt)
{
//  IntReq(24);     //VRender0 VBlank
}

static INPUT_PORTS_START(ddz)

INPUT_PORTS_END


static const vr0_interface vr0_config =
{
	0x04800000
};


static MACHINE_CONFIG_START( ddz, ddz_state )
	MCFG_CPU_ADD("maincpu", SE3208, 43000000)
	MCFG_CPU_PROGRAM_MAP(ddz_mem)
	MCFG_CPU_VBLANK_INT("screen", ddz_interrupt)

	//MCFG_MACHINE_RESET(ddz)

	//MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MCFG_SCREEN_SIZE(320, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE(ddz)
	MCFG_SCREEN_EOF(ddz)

	MCFG_PALETTE_LENGTH(8192)

	MCFG_VIDEO_START(ddz)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("vrender", VRENDER0, 0)
	MCFG_SOUND_CONFIG(vr0_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END



ROM_START( ddz )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD("ddz.001.rom",  0x000000, 0x400000, CRC(b379f823) SHA1(531885b35d668d22c75a9759994f4aca6eacb046) )
	ROM_LOAD("ddz.002.rom",  0x000000, 0x400000, CRC(285c744d) SHA1(2f8bc70825e55e3114015cb263e786df35cde275) )
	ROM_LOAD("ddz.003.rom",  0x000000, 0x400000, CRC(61c9b5c9) SHA1(0438417398403456a1c49408881797a94aa86f49) )
ROM_END


GAME( 200?, ddz,	0,  ddz, ddz, 0, ROT0, "IGS?", "Dou Di Zhu", GAME_IS_SKELETON )
