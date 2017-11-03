// license:GPL-2.0+
// copyright-holders:Felipe Sanches
/*
    Zezinho
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
	ROM_REGION( 0x0, "exemplo", 0 ) //Manual de demonstração
	ROM_LOAD( "exemplo.rom", 0x000, 0x010, CRC(0b33574e) SHA1(e4eb38a8d8327a7b674101f1ee468cb82607fd03) )
ROM_END

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  STATE          INIT  COMPANY                                                FULLNAME            FLAGS
COMP( 1962, ita2, 0,      0,      ita2,    0,     zezinho_state, ita2, "ITA - Instituto Tecnologico da Aeronautica (Brazil)", "Zezinho (ITA-II)", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
//COMP( 1961, zezinho,  0,      0,      zezinho,  0,     zezinho_state, zezinho, "ITA - Instituto Tecnologico da Aeronautica (Brazil)", "Zezinho"           , MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
