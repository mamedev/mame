// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    A51xx

    12/05/2009 Skeleton driver.

    http://www.robotrontechnik.de/index.htm?/html/computer/a5120.htm
    http://www.robotrontechnik.de/index.htm?/html/computer/a5130.htm

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"


class a51xx_state : public driver_device
{
public:
	a51xx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu") { }

	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_MACHINE_RESET(a5130);
	DECLARE_VIDEO_START(a5130);
	UINT32 screen_update_a5120(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_a5130(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
};


static ADDRESS_MAP_START(a5120_mem, AS_PROGRAM, 8, a51xx_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x03ff ) AM_ROM
	AM_RANGE( 0x0400, 0xffff ) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( a5120_io, AS_IO, 8, a51xx_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

static ADDRESS_MAP_START(a5130_mem, AS_PROGRAM, 8, a51xx_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x0fff ) AM_ROM
	AM_RANGE( 0x1000, 0xffff ) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( a5130_io, AS_IO, 8, a51xx_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( a5120 )
INPUT_PORTS_END


void a51xx_state::machine_reset()
{
}

void a51xx_state::video_start()
{
}

UINT32 a51xx_state::screen_update_a5120(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


/* Input ports */
static INPUT_PORTS_START( a5130 )
INPUT_PORTS_END


MACHINE_RESET_MEMBER(a51xx_state,a5130)
{
}

VIDEO_START_MEMBER(a51xx_state,a5130)
{
}

UINT32 a51xx_state::screen_update_a5130(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

/* F4 Character Displayer */
static const gfx_layout a51xx_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 1024*8, 1025*8, 1026*8, 1027*8, 1028*8, 1029*8, 1030*8, 1031*8 },
	8*8                 /* every char takes 2 x 8 bytes */
};

static GFXDECODE_START( a51xx )
	GFXDECODE_ENTRY( "chargen", 0x0000, a51xx_charlayout, 0, 1 )
GFXDECODE_END

static MACHINE_CONFIG_START( a5120, a51xx_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(a5120_mem)
	MCFG_CPU_IO_MAP(a5120_io)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(a51xx_state, screen_update_a5120)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", a51xx)

	MCFG_PALETTE_ADD_MONOCHROME("palette")

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( a5130, a5120 )
	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(a5130_mem)
	MCFG_CPU_IO_MAP(a5130_io)

	MCFG_MACHINE_RESET_OVERRIDE(a51xx_state,a5130)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(a51xx_state, screen_update_a5130)

	MCFG_VIDEO_START_OVERRIDE(a51xx_state,a5130)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( a5120 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v1", "v1" )
	ROMX_LOAD( "a5120_v1.rom", 0x0000, 0x0400, CRC(b2b3fee0) SHA1(6198513b263d8a7a867f1dda368b415bb37fcdae), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "v2", "v2" )
	ROMX_LOAD( "a5120_v2.rom", 0x0000, 0x0400, CRC(052386c2) SHA1(e545d30a0882cb7ee7acdbea842b57440552e4a6), ROM_BIOS(2))

	ROM_REGION(0x0800, "chargen",0)
	ROM_LOAD( "bab47_1_lat.bin", 0x0000, 0x0400, CRC(93220886) SHA1(a5a1ab4e2e06eabc58c84991caa6a1cf55f1462d))
	ROM_LOAD( "bab46_2_lat.bin", 0x0400, 0x0400, CRC(7a578ec8) SHA1(d17d3f1c182c23e9e9fd4dd56f3ac3de4c18fb1a))
ROM_END

/* ROM definition */
ROM_START( a5130 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "dzr5130.rom", 0x0000, 0x1000, CRC(4719beb7) SHA1(09295a658b8c5b75b20faea57ad925f69f07a9b5))

	ROM_REGION(0x0800, "chargen",0)
	ROM_LOAD( "bab47_1_lat.bin", 0x0000, 0x0400, CRC(93220886) SHA1(a5a1ab4e2e06eabc58c84991caa6a1cf55f1462d))
	ROM_LOAD( "bab46_2_lat.bin", 0x0400, 0x0400, CRC(7a578ec8) SHA1(d17d3f1c182c23e9e9fd4dd56f3ac3de4c18fb1a))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE     INPUT    INIT    COMPANY           FULLNAME       FLAGS */
COMP( 1982, a5120,  0,      0,       a5120,      a5120, driver_device,   0,      "VEB Robotron",   "A5120", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1983, a5130,  a5120,  0,       a5130,      a5130, driver_device,   0,      "VEB Robotron",   "A5130", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
