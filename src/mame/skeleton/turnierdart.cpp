// license:BSD-3-Clause
// copyright-holders:
/************************************************************************

Skeleton driver for "Turnier Dart" darts machine from Löwen S.P.O.R.T.
(NSM-Löwen, now Löwen Entertainment, part of Novomatic Group).

PCB silkscreened "Valley Recreation Products - Coyright © 1997".
    ___________________________________________________________________
   |   (o)  (o)        ......       ...                               |
   |  TEST RESET                                                      |
   |.                  __________    __________   __________          |
   |.                 |DS14C88N_|   |HEF4052BP|  |_________|          |
   |.   __________                                            F  F  F |
   |.  |SN74LS123N            .  .  .               BATT      U  U  U |
   |.                         :  :  :                3V       S  S  S |
   |.                        __________________             . E  E  E |
   |                        | LH5168-10L      |             :         |
   |    __________          |_________________|    _____   _____    . |
   |   |SN74LS244N          ___________________   DS1210   5355ED   · |
   |                       | EPROM            |                     · |
   |.                      |__________________|         Xtal        . |
   |.                       __________                                |
   |                       |SN74LS373N               __________     . |
   |    __________          ____________________    |PEEL18CV8P     . |
   |   |JRC_324D_|         | Dallas DS80C310   |                    . |
   |                       |___________________|     __________       |
   |                        __________              |SN74LS273N     . |
   |                       |_74LS02N_|     Xtal                     · |
   |                        __________    12 MHz     __________       |
   |                       |SN74LS273N              |SN74LS244N       |
   |    __________          __________          __________         .. |
   |.  |_________|         |74HC244AP|         |TIBPAL16L8         .. |
   |.   __________          __________          __________            |
   |.  |74HC30AP_|         |74HC244AP|         |TIBPAL16L8          . |
   |.                       __________                              . |
   |.   __________         |SN74LS273N     __________               . |
   |.  |74HC30AP_|                        |SN74LS30N|                 |
   |.                       __________                              . |
   |.                      |74HC244AP|                              . |
   |.   __________          __________                              . |
   |   |_SN7406N_|         |MIC5821BN|                 _____          |
   |..                      __________                LM358N          |
   |..                     |SN74LS273N                                |
   |..                      __________                                |
   |                       |SN74LS273N                                |
   |                                           ..                     |
   |  ::::::::::::::::::     ::::::::::::      ..   :                 |
   |__________________________________________________________________|

************************************************************************/

#include "emu.h"
#include "cpu/mcs51/i80c51.h"
//#include "machine/nvram.h"
#include "speaker.h"

namespace {

class turnierdart_state : public driver_device
{

public:
	turnierdart_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void turnierdart(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
};


INPUT_PORTS_START(turnierdart)
INPUT_PORTS_END

void turnierdart_state::turnierdart(machine_config &config)
{
	I80C31(config, m_maincpu, 12_MHz_XTAL); // Dallas DS80C310

	//NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // DS1210

	SPEAKER(config, "mono").front_center();
}

/* Newer version, with the following games:

    GAMES       OPTION I     OPTION II
   ----------  -----------  -------------
   301         WIPE OUT     SHORTY
   501         DOUBLE IN    TEAM
   701         DOUBLE OUT   NDA
   901         MASTERS OUT  ADA
   CRICKET     CHANCE IT    TIME HANDICAP
   HI SCORE    SHUFFLE IT   AUTO ADJUST
   SHANGHAI    YOU PICK IT  ERNIE
   RAPID FIRE  CUT THROAT
   STOP WATCH  SPLIT SCORE
*/
ROM_START(turnierd)
	ROM_REGION(0x40000, "maincpu", 0)
	ROM_LOAD("cu-07b_15-4-10.u18", 0x00000, 0x40000, CRC(17f1dfbb) SHA1(4c93ad3573edd2d9dd2727b10f638cfb7ba14905))

	ROM_REGION(0x117, "plds", 0)
	ROM_LOAD("tibpal16l8.u27", 0x00000, 0x00117, NO_DUMP)
	ROM_LOAD("tibpal16l8.u28", 0x00000, 0x00117, NO_DUMP)
	ROM_LOAD("peel18cv8p.u30", 0x00000, 0x00117, NO_DUMP)
ROM_END

/* Older version, with the following games:

    GAMES            OPTION I        OPTION II
   ---------------  --------------  -----------------
   301              DOUBLE IN       TEAM: 2 SPIELEN
   501              DOUBLE OUT      TEAM: 4 SPIELEN
   701              MASTERS OUT     HANDICAP
   901              CHANCE IT       TIME OUT HANDICAP
   CRICKET          YOU PICK IT
   HI SCORE         CUT THROAT
   SHANGHAI         MASTERS CRCKET
   301 ELIMINATION  CUT THROAT
   SPLIT SCORE
   RAPID FIRE
*/
ROM_START(turnierda)
	ROM_REGION(0x40000, "maincpu", 0)
	ROM_LOAD("hb_8-97_spa_03_82da_30-03-98.u18", 0x00000, 0x40000, CRC(29cf8931) SHA1(35924488ec5508541b02269dfdf055b20057c51c))

	ROM_REGION(0x00117, "plds", 0)
	ROM_LOAD("tibpal16l8.u27", 0x00000, 0x00117, NO_DUMP)
	ROM_LOAD("tibpal16l8.u28", 0x00000, 0x00117, NO_DUMP)
	ROM_LOAD("peel18cv8p.u30", 0x00000, 0x00117, NO_DUMP)
ROM_END

} // Anonymous namespace

GAME(2010, turnierd,         0, turnierdart, turnierdart, turnierdart_state, empty_init, ROT0, u8"Löwen S.P.O.R.T.", "Turnier Dart",          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(1998, turnierda, turnierd, turnierdart, turnierdart, turnierdart_state, empty_init, ROT0, u8"Löwen S.P.O.R.T.", "Turnier Dart (HB8-97)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
