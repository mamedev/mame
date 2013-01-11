/***************************************************************************

        Epson HX20

        29/09/2009 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"


class hx20_state : public driver_device
{
public:
	hx20_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_ehx20(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


static ADDRESS_MAP_START(ehx20_mem, AS_PROGRAM, 8, hx20_state)
	AM_RANGE(0x0000, 0x7fff) AM_RAM // I/O
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(ehx20_io, AS_IO, 8, hx20_state)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( ehx20 )
INPUT_PORTS_END


void hx20_state::machine_reset()
{
}

void hx20_state::video_start()
{
}

UINT32 hx20_state::screen_update_ehx20(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

/* F4 Character Displayer */
static const gfx_layout hx20_1_charlayout =
{
	5, 8,                   /* 5 x 8 characters */
	102,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 8, 16, 24, 32 },
	/* y offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	5*8                 /* every char is 5 bits of 8 bytes */
};

static const gfx_layout hx20_2_charlayout =
{
	6, 8,                   /* 6 x 8 characters */
	27,                 /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 8, 16, 24, 32, 40 },
	/* y offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	6*8                 /* every char is 6 bits of 8 bytes */
};

static const gfx_layout hx20_3_charlayout =
{
	5, 8,                   /* 5 x 8 characters */
	34,                 /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 16, 24, 32, 40, 48 },
	/* y offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	5*8                 /* every char is 5 bits of 8 bytes */
};

static GFXDECODE_START( hx20 )
	GFXDECODE_ENTRY( "maincpu", 0xfb82, hx20_1_charlayout, 0, 1 )
	GFXDECODE_ENTRY( "maincpu", 0xfd80, hx20_2_charlayout, 0, 1 )
	GFXDECODE_ENTRY( "maincpu", 0xfe20, hx20_3_charlayout, 0, 1 )
GFXDECODE_END

static MACHINE_CONFIG_START( ehx20, hx20_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",HD63701, 614000) // HD6301
	MCFG_CPU_PROGRAM_MAP(ehx20_mem)
	MCFG_CPU_IO_MAP(ehx20_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(hx20_state, screen_update_ehx20)

	MCFG_GFXDECODE(hx20)
	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(black_and_white)

MACHINE_CONFIG_END

/* ROM definition */
ROM_START( ehx20 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v11", "version 1.1")
	ROMX_LOAD( "hx20_v11.3", 0xe000, 0x2000, CRC(101cb3e8) SHA1(e0b5cf107a9387e34a0e46f54328b89696c0bdc5), ROM_BIOS(1))
	ROMX_LOAD( "hx20_v11.2", 0xc000, 0x2000, CRC(26c203a1) SHA1(b282d7233b2689820fcf718dbe1e93d623b67e4f), ROM_BIOS(1))
	ROMX_LOAD( "hx20_v11.1", 0xa000, 0x2000, CRC(10d6ae76) SHA1(3163954ed9981f70f590ee98bcc8e19e4be6527a), ROM_BIOS(1))
	ROMX_LOAD( "hx20_v11.0", 0x8000, 0x2000, CRC(4de0b4b6) SHA1(f15c537824b7effde9d9b9a21e92a081fb089371), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v11e", "version 1.1 EU")
	ROMX_LOAD( "hx20_v11e.0", 0xe000, 0x2000, CRC(fd339aa5) SHA1(860c3579c45e96c5e6a877f4fbe77abacb0d674e), ROM_BIOS(2))
	ROMX_LOAD( "hx20_v11e.1", 0xc000, 0x2000, CRC(26c203a1) SHA1(b282d7233b2689820fcf718dbe1e93d623b67e4f), ROM_BIOS(2))
	ROMX_LOAD( "hx20_v11e.2", 0xa000, 0x2000, CRC(10d6ae76) SHA1(3163954ed9981f70f590ee98bcc8e19e4be6527a), ROM_BIOS(2))
	ROMX_LOAD( "hx20_v11e.3", 0x8000, 0x2000, CRC(4de0b4b6) SHA1(f15c537824b7effde9d9b9a21e92a081fb089371), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "v10", "version 1.0")
	ROMX_LOAD( "hx20_v10.3", 0x8000, 0x2000, CRC(ed7482c6) SHA1(8fba63037f2418aee9e933a353b052a5ed816ead), ROM_BIOS(3))
	ROMX_LOAD( "hx20_v10.2", 0xa000, 0x2000, CRC(f5cc8868) SHA1(3248a1ddf0d8df7e9f2fe96955385218d760c4ad), ROM_BIOS(3))
	ROMX_LOAD( "hx20_v10.1", 0xc000, 0x2000, CRC(27d743ed) SHA1(ebae367b0fa5f42ac78424df2534312296fd6fdc), ROM_BIOS(3))
	ROMX_LOAD( "hx20_v10.0", 0xe000, 0x2000, CRC(33fbb1ab) SHA1(292ace94b4dad267aa7786dc64e68ac6f3c98aa7), ROM_BIOS(3))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY   FULLNAME       FLAGS */
COMP( 1983, ehx20,  0,       0,      ehx20,     ehx20, driver_device,    0,     "Epson", "HX20", GAME_NOT_WORKING | GAME_NO_SOUND )
