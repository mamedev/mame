/***************************************************************************

        TIM-011

        04/09/2010 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/z180/z180.h"


class tim011_state : public driver_device
{
public:
	tim011_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }
	virtual void machine_reset();
	virtual void video_start();
};


static ADDRESS_MAP_START(tim011_mem, AS_PROGRAM, 8, tim011_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x01fff) AM_ROM
	AM_RANGE(0x40000, 0x7ffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(tim011_io, AS_IO, 8, tim011_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x007f) AM_RAM	/* Z180 internal registers */
	AM_RANGE(0x8000, 0xffff) AM_RAM
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( tim011 )
INPUT_PORTS_END


void tim011_state::machine_reset()
{
}

void tim011_state::video_start()
{
}

static SCREEN_UPDATE_IND16( tim011 )
{
	return 0;
}

static MACHINE_CONFIG_START( tim011,tim011_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z180, XTAL_12_288MHz / 2)
	MCFG_CPU_PROGRAM_MAP(tim011_mem)
	MCFG_CPU_IO_MAP(tim011_io)
	MCFG_CPU_VBLANK_INT("screen",irq0_line_hold)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_STATIC(tim011)
	MCFG_PALETTE_LENGTH(4)
	MCFG_PALETTE_INIT(black_and_white)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( tim011 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "sys_tim011.bin", 0x0000, 0x2000, CRC(5b4f1300) SHA1(d324991c4292d7dcde8b8d183a57458be8a2be7b))
	ROM_REGION( 0x10000, "keyboard", ROMREGION_ERASEFF )
	ROM_LOAD( "keyb_tim011.bin", 0x0000, 0x1000, CRC(a99c40a6) SHA1(d6d505271d91df4e079ec3c0a4abbe75ae9d649b))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY                FULLNAME       FLAGS */
COMP( 1987, tim011, 0,      0,       tim011,    tim011, driver_device,  0, "Mihajlo Pupin Institute", "TIM-011", GAME_NOT_WORKING | GAME_NO_SOUND)
