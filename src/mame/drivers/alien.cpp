// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**********************************************************************************

    Alien: The Arcade Medal Edition (c) 2005 Capcom

    skeleton driver

    - sh-4 clocked with 200MHz
    - 2 x Panasonic MN677511DE chips (MPEG2 decoders)
    - Altera ACEX 1K PLD
    - M48T35Y timekeeper device
    - CF interface
    - YMZ770B-F

***********************************************************************************/


#include "emu.h"
#include "cpu/sh4/sh4.h"

#define MASTER_CLOCK    XTAL_200MHz

class alien_state : public driver_device
{
public:
	alien_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	DECLARE_READ64_MEMBER(test_r);

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
	virtual void video_start();
};

void alien_state::video_start()
{
}

UINT32 alien_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

READ64_MEMBER( alien_state::test_r )
{
	return machine().rand();
}

static ADDRESS_MAP_START( alien_map, AS_PROGRAM, 64, alien_state )
	AM_RANGE(0x00000000, 0x0003ffff) AM_ROM
	AM_RANGE(0x08000000, 0x08000007) AM_READ(test_r) //hangs if zero
	AM_RANGE(0x0cfe0000, 0x0cffffff) AM_RAM
	AM_RANGE(0x10000000, 0x13ffffff) AM_RAM
	AM_RANGE(0x18000000, 0x1800000f) AM_READ(test_r) AM_WRITENOP
ADDRESS_MAP_END




static INPUT_PORTS_START( alien )

INPUT_PORTS_END


void alien_state::machine_reset()
{
	//m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

static MACHINE_CONFIG_START( alien, alien_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SH4LE, MASTER_CLOCK)    /* 200MHz */
	MCFG_CPU_PROGRAM_MAP(alien_map)

	/* video hardware */

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_UPDATE_DRIVER(alien_state, screen_update)
	MCFG_SCREEN_SIZE((32)*8, (32)*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)

	MCFG_PALETTE_ADD("palette", 0x1000)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

MACHINE_CONFIG_END


/*************************
*        Rom Load        *
*************************/

ROM_START( alien )
	ROM_REGION( 0x800000, "maincpu", 0 ) // BIOS code
	ROM_LOAD32_WORD( "aln_s04.4.ic30", 0x000000, 0x400000, CRC(11777d3f) SHA1(8cc9fcae7911e6be273b4532d89b44a309687ead) )
	ROM_LOAD32_WORD( "aln_s05.5.ic33", 0x000002, 0x400000, CRC(71d2f22c) SHA1(16b25aa34f8b0d988565e7ab7cecc4df62ee8cf3) )

	ROM_REGION( 0x800100, "unk", 0 ) //sound samples flash rom
	ROM_LOAD( "s29jl064hxxtfi00.u35", 0x000000, 0x800100, CRC(01890c61) SHA1(4fad321f42eab835351c6d5f73539bdbed80affe) )

	ROM_REGION( 0x8000, "nvram", ROMREGION_ERASEFF) //timekeeper device
	ROM_LOAD( "m48t35y.3.ic26", 0x000000, 0x007ff8, CRC(060b0a75) SHA1(7ddf380ee0e7b54533ef7e248405bfce1c5dbb4b) )

	DISK_REGION( "card" ) //compact flash
	DISK_IMAGE( "alien", 0, SHA1(0328f12765db41a9ef5c8bfb88d4983345093072) )
ROM_END


GAME( 2005, alien,  0,      alien, alien, driver_device, 0, ROT0, "Capcom", "Alien: The Arcade Medal Edition", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
