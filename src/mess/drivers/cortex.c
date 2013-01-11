/***************************************************************************

        Powertran Cortex

        20/04/2012 Skeleton driver.

        ftp://ftp.whtech.com/Powertran Cortex/
        http://www.powertrancortex.com/index.html

        Uses Texas Instruments parts and similar to other TI computers.
        It was designed by TI engineers, so it may perhaps be a clone
        of another TI or the Geneve.

        Video chip is TMS9928 or TMS9929.

        64K RAM.
        I saw somewhere that the roms are copied into ram at startup.

****************************************************************************/

#include "emu.h"
#include "cpu/tms9900/tms9900l.h"

class cortex_state : public driver_device
{
public:
	cortex_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_cortex(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

static ADDRESS_MAP_START( cortex_mem, AS_PROGRAM, 8, cortex_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cortex_io, AS_IO, 8, cortex_state )
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( cortex )
INPUT_PORTS_END


void cortex_state::machine_reset()
{
}

void cortex_state::video_start()
{
}

UINT32 cortex_state::screen_update_cortex(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static const struct tms9995reset_param cortex_processor_config =
{
	0,  /* disable automatic wait state generation */
	0,  /* no IDLE callback */
	0   /* no MP9537 mask */
};


static MACHINE_CONFIG_START( cortex, cortex_state )
	/* basic machine hardware */
	/* TMS9995 CPU @ 12.0 MHz */
	MCFG_CPU_ADD("maincpu", TMS9995L, 12000000)
	MCFG_CPU_CONFIG(cortex_processor_config)
	MCFG_CPU_PROGRAM_MAP(cortex_mem)
	MCFG_CPU_IO_MAP(cortex_io)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(cortex_state, screen_update_cortex)
	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(black_and_white)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( cortex )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "basic", "Cortex Bios")
	ROMX_LOAD( "cortex_ic47.bin", 0x0000, 0x2000, CRC(bdb8c7bd) SHA1(340829dcb7a65f2e830fd5aff82a312e3ed7918f), ROM_BIOS(1))
	ROMX_LOAD( "cortex_ic46.bin", 0x2000, 0x2000, CRC(4de459ea) SHA1(00a42fe556d4ffe1f85b2ce369f544b07fbd06d9), ROM_BIOS(1))
	ROMX_LOAD( "cortex_ic45.bin", 0x4000, 0x2000, CRC(b0c9b6e8) SHA1(4e20c3f0b7546b803da4805cd3b8616f96c3d923), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "forth", "FIG-Forth")
	ROMX_LOAD( "forth_ic47.bin",  0x0000, 0x2000, CRC(999034be) SHA1(0dcc7404c38aa0ae913101eb0aa98da82104b5d4), ROM_BIOS(2))
	ROMX_LOAD( "forth_ic46.bin",  0x2000, 0x2000, CRC(8eca54cc) SHA1(0f1680e941ef60bb9bde9a4b843b78f30dff3202), ROM_BIOS(2))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY                  FULLNAME       FLAGS */
COMP( 1982, cortex, 0,      0,       cortex,    cortex, driver_device,  0,    "Powertran Cybernetics",   "Cortex", GAME_NOT_WORKING | GAME_NO_SOUND)
