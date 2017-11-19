// license:GPL-2.0+
// copyright-holders:Felipe Sanches; Werner Moecke
/*
    Zezinho - Instituto Tecnológico da Aeronáutica (ITA)

    Presumably the first computer designed and built in Brazil.
    There are 2 variants. The first one, from 1961, is clearly not a turing-complete machine since
    it only has got 4 instructions (add, subtract, data input and data output).

    The second one, from 1962, was also called ITA-II, and has got 14 instructions including things like branching.
    Unfortunately I am unaware of any software for the second iteration.

    This driver only emulates the second variant, but runs a demo from the manual of the first one.
    While there's now proper artwork, the only useful way to use this driver is by running it with MAME's debugger
    to run the example code step-by-step.

    See also the "Patinho Feio" driver in MAME, for info on another early Brazilian computer (which was
    designed a decade later in 1971).
*/

#include "emu.h"
#include "cpu/zezinho/zezinho_cpu.h"
#include "includes/zezinho.h"
//#include "zezinho.lh"

/*
    driver init function
*/
DRIVER_INIT_MEMBER(zezinho_state, ita2)
{
}

static MACHINE_CONFIG_START( ita2 )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", ZEZINHO2_CPU, 5000)

//	MCFG_DEFAULT_LAYOUT(layout_zezinho)
MACHINE_CONFIG_END

ROM_START( ita2 )
	ROM_REGION( 0x1000, "maincpu", 0 ) //Manual de demonstração
	ROM_LOAD( "exemplo.rom", 0x000, 0x010, CRC(590b78e9) SHA1(a71e63697068cc167b84802681c65dcbcdd489a5) )
ROM_END

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  STATE          INIT  COMPANY                                                FULLNAME            FLAGS
COMP( 1962, ita2, 0,      0,      ita2,    0,     zezinho_state, ita2, "ITA - Instituto Tecnologico da Aeronautica (Brazil)", "Zezinho (ITA-II)", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
//COMP( 1961, zezinho,  0,      0,      zezinho,  0,     zezinho_state, zezinho, "ITA - Instituto Tecnologico da Aeronautica (Brazil)", "Zezinho"           , MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
