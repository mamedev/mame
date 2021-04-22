// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************

    Skeleton driver for MCS48-based slots by Recreativos Franco

 Recreativos Franco PCB "RF-3115"
 _______________________________________________________________________________________
|    ······································································            |
|      _______                                                                        ·|
|      |4xDSW|                                                                        ·|
|                                                                                     ·|
|   ___________   ___________  ___________  ___________  ___________  ___________     ·|
|  | SN74154N |  | D8245C   | | D8245C   | | D8245C   | | D8245C   | | D8245C   |     ·|
|  |__________|  |__________| |__________| |__________| |__________| |__________|     ·|
|                                                                                     ·|
|                                                        XTAL                          |
|   _______   _______  _______ _______    _______     6.0000 MHz ________________      |
|  |LM380N|  CD4051BCN |6xDSW| 74LS00PC  |7402PC|               | Intel P8035L  |    __|
|                                                               |_______________|   |·||
|                      ___________  ___________                                     |·||
|   ________________  |55101L-1PC| |55101L-1PC|    ___________     ___________      |·||
|  | Intel D8748   |                              | EPROM     |   | AMD P8212 |     |_||
|  |_______________|   ___________  ___________   |___________|   |___________|        |
|       XTAL          |55101L-1PC| |55101L-1PC|                                       ·|
|     4.0000 MHz                                                                      ·|
|                                                                                     ·|
|                  RF-3115                                                            ·|
|   BATTERY                                                                           ·|
|______________________________________________________________________________________|

***************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "speaker.h"

namespace
{

class rfslotsmcs48_state : public driver_device
{
public:
	rfslotsmcs48_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
	{
	}

	void ajofrin(machine_config &config);

protected:
	required_device<i8035_device> m_maincpu;
	required_device<i8748_device> m_audiocpu;
};
	
INPUT_PORTS_START(ajofrin)
INPUT_PORTS_END

void rfslotsmcs48_state::ajofrin(machine_config &config)
{
	I8035(config, m_maincpu, 6_MHz_XTAL);

	I8748(config, m_audiocpu, 4_MHz_XTAL);

	SPEAKER(config, "mono").front_center();
}

// Found just one PCB, probably missing some more boards (sound, I/O, etc.)
ROM_START(ajofrin)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("ajofr_90percent_4k.bin", 0x0000, 0x1000, CRC(9e1fd7fe) SHA1(e7a5a2a10d17537edb039ac0f53358ee35465c90))

	ROM_REGION(0x400, "audiocpu", 0)
	ROM_LOAD( "d8748.bin", 0x000, 0x400, NO_DUMP )
ROM_END

} // anonymous namespace

//    YEAR  NAME     PARENT MACHINE  INPUT    CLASS               INIT        ROT   COMPANY               FULLNAME        FLAGS
GAME( 198?, ajofrin, 0,     ajofrin, ajofrin, rfslotsmcs48_state, empty_init, ROT0, "Recreativos Franco", "Ajofrin City", MACHINE_IS_SKELETON_MECHANICAL )
