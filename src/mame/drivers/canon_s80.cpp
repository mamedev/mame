// license:GPL-2.0+
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
#include "cpu/m6800/m6801.h"
//#include "video/hd44780.h"

class canons80_state : public driver_device
{
public:
	canons80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }

	void init_canons80();
	void canons80(machine_config &config);
	void canons80_map(address_map &map);
};


void canons80_state::canons80_map(address_map &map)
{
	map(0x0000, 0x7fff).ram();
	map(0x8000, 0xffff).rom();
}

void canons80_state::canons80(machine_config &config)
{
	/* basic machine hardware */
	hd6301_cpu_device &maincpu(HD6301(config, "maincpu", 5000000)); /* hd63a01xop 5 MHz guessed: TODO: check on PCB */
	maincpu.set_addrmap(AS_PROGRAM, &canons80_state::canons80_map);
}

void canons80_state::init_canons80()
{
}

ROM_START( canons80 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 6800 code */
	ROM_LOAD( "canon_8735kx_nh4-0029_064.ic6", 0x8000, 0x8000, CRC(b6cd2ff7) SHA1(e47a136300c826e480fac1be7fc090523078a2a6) )
ROM_END

/*    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT  CLASS           INIT           COMPANY  FULLNAME                            FLAGS */
COMP( 1988, canons80, 0,      0,      canons80, 0,     canons80_state, init_canons80, "Canon", "Canon S-80 electronic typewriter", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
