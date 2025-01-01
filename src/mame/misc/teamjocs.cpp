// license:BSD-3-Clause
// copyright-holders:

/*
 TEAM JOCS Grupo Presas
 Mod. PELU CPU
 ____________________________________________________
 |  _____________  _CN5_  _CN2_  _______________     |
 |  |___CN7______| |____| |____| |____CN4_______|    |
 |                                                   |
 |__               ULN2003A                 ULN2003A |
 || |                                                |
 || |  HC245A            74HC273D       74HC273D  __ |
 ||C|                                             | ||
 ||N|           M28C17      HC245A HC245A         | ||
 ||6|                                             | ||
 || |  HC245A          XTAL                       | ||
 || |                4.0000MHz                    | ||
 ||_|                                             | ||
 |                           PIC17C44-16          | ||
 |__   74HC2730                                   | ||
 || |                                             | ||
 || |          OKI-M6375                          |_||
 ||C|                                                |
 ||N|  74HC2730               NM27C010Q           __ |
 ||3|                                             | ||
 ||_|  HC245A                               CN10->|_||
 |                                BATT               |
 |   ______________          ___             ___     |
 |   |_____________|<-CN8    |__|<-CN12 CN9->|__|    |
 |___________________________________________________|
*/

#include "emu.h"
#include "cpu/pic17/pic17c4x.h"
#include "machine/nvram.h"
#include "sound/okim6376.h"
#include "speaker.h"


namespace {

class teamjocs_state : public driver_device
{
public:
	teamjocs_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void teamjocs(machine_config &config);

private:

	required_device<cpu_device> m_maincpu;
};

static INPUT_PORTS_START(teamjocs)
INPUT_PORTS_END

void teamjocs_state::teamjocs(machine_config &config)
{
	PIC17C44(config, m_maincpu, 4_MHz_XTAL);

	SPEAKER(config, "mono").front_center();

	OKIM6376(config, "oki", 4_MHz_XTAL); // divided unknown

}

ROM_START(diaelite)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "elite_3_4_6_pic17c44.ic7", 0x0000, 0x8000, NO_DUMP )

	ROM_REGION(0x20000, "oki", 0)
	ROM_LOAD( "b.ic4", 0x00000, 0x20000, CRC(c8505516) SHA1(cd56e25b57752970f6a0de7e5efdda5527e21019) )

	ROM_REGION(0x800, "28c17", 0)
	ROM_LOAD( "m28c17.ic13", 0x000, 0x800, CRC(4a292b0d) SHA1(1f86e52b52ed53a7a8197e1ee394876dfc51fb91) )
ROM_END

} // anonymous namespace


GAME(199?, diaelite, 0, teamjocs, teamjocs, teamjocs_state, empty_init, ROT0, "Recreativos Presas", "Diana Elite", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
