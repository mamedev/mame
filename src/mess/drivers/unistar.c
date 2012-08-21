/***************************************************************************

        Callan Unistar Terminal

        09/12/2009 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"


class unistar_state : public driver_device
{
public:
	unistar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

};


static ADDRESS_MAP_START(unistar_mem, AS_PROGRAM, 8, unistar_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(unistar_io, AS_IO, 8, unistar_state)
	//ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( unistar )
INPUT_PORTS_END


static MACHINE_RESET( unistar )
{
}

static VIDEO_START( unistar )
{
}

static SCREEN_UPDATE_IND16( unistar )
{
	return 0;
}

/* F4 Character Displayer */
static const gfx_layout unistar_charlayout =
{
	8, 16,					/* 8 x 16 characters */
	128,					/* 128 characters */
	1,					/* 1 bits per pixel */
	{ 0 },					/* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8, 8*8,  9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16					/* every char takes 16 bytes */
};

static GFXDECODE_START( unistar )
	GFXDECODE_ENTRY( "chargen", 0x0000, unistar_charlayout, 0, 1 )
GFXDECODE_END

static MACHINE_CONFIG_START( unistar, unistar_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8085A, XTAL_2MHz)
	MCFG_CPU_PROGRAM_MAP(unistar_mem)
	MCFG_CPU_IO_MAP(unistar_io)

	MCFG_MACHINE_RESET(unistar)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_VIDEO_START(unistar)
	MCFG_SCREEN_UPDATE_STATIC(unistar)
	MCFG_GFXDECODE(unistar)
	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(black_and_white)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( unistar )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "callan_video_pgm1.bin", 0x0000, 0x1000, CRC(613ef521) SHA1(a77459e91617d2882778ab2dada74fcb5f44e949))
	ROM_LOAD( "callan_video_pgm2.bin", 0x1000, 0x1000, CRC(6cc5e704) SHA1(fb93645f51d5ad0635cbc8a9174c61f96799313d))
	ROM_LOAD( "callan_video_pgm3.bin", 0x2000, 0x1000, CRC(0b9ca5a5) SHA1(20bf4aeacda14ff7a3cf988c7c0bff6ec60406c7))

	ROM_REGION( 0x0800, "chargen", ROMREGION_ERASEFF )
	ROM_LOAD( "callan_video_vid.bin",  0x0000, 0x0800, CRC(a9e1b5b2) SHA1(6f5b597ee1417f1108ac5957b005a927acb5314a))
ROM_END

/* Driver */

/*    YEAR  NAME     PARENT  COMPAT   MACHINE    INPUT    INIT        COMPANY            FULLNAME              FLAGS */
COMP( 198?, unistar, 0,      0,       unistar,   unistar, driver_device, 0,  "Callan Data Systems", "Unistar Terminal", GAME_NOT_WORKING | GAME_NO_SOUND)
