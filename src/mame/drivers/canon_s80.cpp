// license:GPL2+
// copyright-holders:FelipeSanches
/*
 * canon_s80.c
 *
 *    CANON S-80 electronic typewriter
 *
 * skeleton driver by:
 *    Felipe Correa da Silva Sanches <juca@members.fsf.org>
 *
 * known issues:
 *  - memory-map is uncertain
 *  - maincpu clock is guessed
 *  - still need to hookup the Hitachi HD44780 LCD Controller
 *  - still lacks description of the keyboard inputs
 *  - as well as a "paper" device to plot the output of the dot matrix print head
 */

#include "emu.h"
#include "cpu/m6800/m6800.h"
//#include "video/hd44780.h"

class canons80_state : public driver_device
{
public:
	canons80_state(const machine_config &mconfig, device_type type, std::string tag)
			: driver_device(mconfig, type, tag)
			{ }

	DECLARE_DRIVER_INIT(canons80);
};


static ADDRESS_MAP_START(canons80_map, AS_PROGRAM, 8, canons80_state )
	AM_RANGE(0x0000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static MACHINE_CONFIG_START( canons80, canons80_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD6301, 5000000) /* hd63a01xop 5 MHz guessed: TODO: check on PCB */
	MCFG_CPU_PROGRAM_MAP(canons80_map)
MACHINE_CONFIG_END

DRIVER_INIT_MEMBER(canons80_state, canons80)
{}

ROM_START( canons80 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 6800 code */
	ROM_LOAD( "canon_8735kx_nh4-0029_064.ic6", 0x8000, 0x8000, CRC(b6cd2ff7) SHA1(e47a136300c826e480fac1be7fc090523078a2a6) )
ROM_END

/*    YEAR  NAME      PARENT  COMPAT   MACHINE  INPUT  INIT                      COMPANY  FULLNAME                            FLAGS */
COMP( 1988, canons80,      0,      0, canons80,     0, canons80_state, canons80, "Canon", "Canon S-80 electronic typewriter", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
