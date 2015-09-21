// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
/*************************************************************************

    decwritr.c
    Digital Equipment Corporation
    DECwriter III (LA120) Teletype/Teleprinter, 1978

**************************************************************************/

#include "emu.h"
#include "render.h"
#include "ui/selgame.h"
#include "cpu/i8085/i8085.h"


//**************************************************************************
//  DRIVER STATE
//**************************************************************************

class decwriter_state : public driver_device
{
public:
	// constructor
	decwriter_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{
	}
/*
	virtual void machine_start()
	{
		// force the UI to show the game select screen
		ui_menu_select_game::force_game_select(machine(), &machine().render().ui_container());
	}
*/
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
	{
		bitmap.fill(rgb_t::black);
		return 0;
	}
private:
	required_device<cpu_device> m_maincpu;
	//required_device<generic_terminal_device> m_terminal;
};

static ADDRESS_MAP_START(la120_mem, AS_PROGRAM, 8, decwriter_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x2fff ) AM_ROM
	//AM_RANGE( 0x3000, 0x3fff ) // keyboard 
	AM_RANGE( 0x4000, 0x43ff ) AM_MIRROR(0x0c00) AM_RAM // 1k 'low ram'
	AM_RANGE( 0x5000, 0x53ff ) AM_MIRROR(0x0c00) AM_RAM // 1k 'high ram'
	AM_RANGE( 0x6000, 0x6fff ) AM_RAM // ER1400 EAROM
	AM_RANGE( 0x7000, 0x7fff ) // DC305 printer controller ASIC stuff
	// 8000-ffff is reserved for expansion (i.e. unused, open bus)
ADDRESS_MAP_END

static ADDRESS_MAP_START(la120_io, AS_IO, 8, decwriter_state)
	ADDRESS_MAP_UNMAP_HIGH
	//AM_RANGE(0x00, 0x00) AM_MIRROR(0x7D) // 8251 usart
	//AM_RANGE(0x02, 0x02) AM_MIRROR(0x7D) // other io ports
	// 0x80-0xff are reserved for expansion (i.e. unused, open bus)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

static MACHINE_CONFIG_START( la120, decwriter_state )

	MCFG_CPU_ADD("maincpu",I8080, XTAL_24_8832MHz / 9)
	MCFG_CPU_PROGRAM_MAP(la120_mem)
	MCFG_CPU_IO_MAP(la120_io)
	
	// video hardware
	//TODO: no actual screen! just paper.
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(decwriter_state, screen_update)
	MCFG_SCREEN_SIZE(640,480)
	MCFG_SCREEN_VISIBLE_AREA(0,639, 0,479)
	MCFG_SCREEN_REFRESH_RATE(30)
MACHINE_CONFIG_END



//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( la120 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	// later romset, with 23-003e2, 23-004e2, 23-005e2, 23-006e2 replaced by one rom, 23-038e4
	ROM_LOAD( "23-038e4-00.e6", 0x0000, 0x2000, CRC(12b80c00) SHA1(35875a85c5037454ac4a82ee19ea9f0337ad0dbe))
	ROM_LOAD( "23-007e2-00.e4", 0x2000, 0x0800, CRC(41eaebf1) SHA1(c7d05417b24b853280d1636776d399a0aea34720)) // used by both earlier and later romset
	// there is an optional european and APL rom which goes from 2800-2fff, undumped.
ROM_END


//**************************************************************************
//  GAME DRIVERS
//**************************************************************************
/*    YEAR  NAME   PARENT  COMPAT  MACHINE  STATE          INPUT    INIT  COMPANY                          FULLNAME       FLAGS */
GAME( 1978, la120, 0,      la120,  0,       driver_device, 0,       0,    "Digital Equipment Corporation", "DECwriter III (LA120)", MACHINE_NO_SOUND | MACHINE_IS_SKELETON | MACHINE_NOT_WORKING )
