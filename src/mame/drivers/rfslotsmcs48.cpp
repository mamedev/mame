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
|  | SN74154N |  | D8243C   | | D8243C   | | D8243C   | | D8243C   | | D8243C   |     ·|
|  |__________|  |__________| |__________| |__________| |__________| |__________|     ·|
|                                                                                     ·|
|                                                        XTAL                          |
|   _______   _______  _______ _______    _______     6.0000 MHz ________________      |
|  |LM380N|  CD4051BCN |6xDSW| 74LS00PC  |7402PC|               | Intel P8035L  |    __|
|                                                               |_______________|   |·||
|                      ___________  ___________                                     |·||
|   ________________  |S5101L-1PC| |S5101L-1PC|    ___________     ___________      |·||
|  | Intel D8748   |                              | EPROM     |   | AMD P8212 |     |_||
|  |_______________|   ___________  ___________   |___________|   |___________|        |
|       XTAL          |S5101L-1PC| |S5101L-1PC|                                       ·|
|     4.0000 MHz                                                       LED-> O        ·|
|                                                                                     ·|
|                  RF-3115                                                            ·|
|   BATTERY                                                                           ·|
|______________________________________________________________________________________|

***************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/i8212.h"
#include "sound/ay8910.h"
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
		, m_audiosubcpu(*this, "audiosubcpu")
		, m_aysnd1(*this, "aysnd1")
		, m_aysnd2(*this, "aysnd2")
	{
	}

	void ajofrin(machine_config &config);
	void babyfrts(machine_config &config);

protected:
	required_device<i8035_device> m_maincpu;
	optional_device<i8748_device> m_audiocpu;
	optional_device<i8035_device> m_audiosubcpu;
	optional_device<ay8910_device> m_aysnd1;
	optional_device<ay8910_device> m_aysnd2;
};

INPUT_PORTS_START(ajofrin)
INPUT_PORTS_END

void rfslotsmcs48_state::ajofrin(machine_config &config)
{
	I8035(config, m_maincpu, 6_MHz_XTAL);

	I8748(config, m_audiocpu, 4_MHz_XTAL);

	I8212(config, "i8212");

	SPEAKER(config, "mono").front_center();
}

void rfslotsmcs48_state::babyfrts(machine_config &config)
{
	// Main PCB

	I8035(config, m_maincpu, 6_MHz_XTAL);

	I8212(config, "i8212");

	// Audio PCB

	I8035(config, m_audiosubcpu, 6_MHz_XTAL);

	I8212(config, "i8212_2");

	SPEAKER(config, "mono").front_center();

	AY8910(config, m_aysnd1, 6_MHz_XTAL / 4).add_route(ALL_OUTPUTS, "mono", 0.50); // Divider unknown
	AY8910(config, m_aysnd2, 6_MHz_XTAL / 4).add_route(ALL_OUTPUTS, "mono", 0.50); // Divider unknown
}

// Found just one PCB, may be missing some more boards (extra sound, I/O, etc.)
ROM_START(ajofrin)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("ajofr_90percent_4k.bin", 0x0000, 0x1000, CRC(9e1fd7fe) SHA1(e7a5a2a10d17537edb039ac0f53358ee35465c90))

	ROM_REGION(0x0400, "audiocpu", 0)
	ROM_LOAD( "ajo_d8748.bin",         0x0000, 0x0400, CRC(414dc0e3) SHA1(016ed2aa36b36a637163ac7cba0a944a258c02a4))
ROM_END

/* It's an upgrade from "babyfrts25" for accepting 100 pts coins instead of just 25 pts ones.
   Standard RF-3115 PCB but:
   -Two S5101 static RAM chips (instead of 4, one empty socket in the middle)
   -Four NEC D8243C (instead of 5, two empty sockets).
   -No D8748 (the socket is empty), uses an external PCB (RF 53/3131) for sound with this layout:
      ___________________________________
     |     __________________________   |
     |    |·························|   |
     |             Xtal                 |
     |            6.000 MHz             |
     |           _____________________  |
     |  _______ |  AM8035PC          |  |
     |  DM7486N |____________________|  |
     |                                  |
     |     ____________   ____________  |
     |    |Intel D8212|  |2532 EPROM |  |
     |    |___________|  |___________|  |
     |                                  |
     |           _____________________  |
     |          | GI AY-3-8910       |  |
     |          |____________________|  |
     |           _____________________  |
     |          | GI AY-3-8910       |  |
     |          |____________________|  |
     |  _______                         |
     | |· · · ·|                        |
     |__________________________________|

*/
ROM_START(babyfrts)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("bbr_cpu_100pts.bin",     0x0000, 0x1000, CRC(22c47a4d) SHA1(051c82b20a823047119a6d5460c07df601e44a2e))

	ROM_REGION(0x1000, "audiocpu", 0)
	ROM_LOAD( "bbs_100pts_sonido.bin", 0x0000, 0x1000, CRC(c84e20d3) SHA1(ab9457d3877a10bc54485a4954ef5b4d006ed3be))
ROM_END

// Same PCBs configuration as "babyfrts"
ROM_START(babyfrts25)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("baby_8.3_av_25pts.bin",  0x0000, 0x1000, CRC(af45e046) SHA1(d4896f428a2061ad6bc12eed7d56eca4182d237d))

	ROM_REGION(0x1000, "audiocpu", 0)
	ROM_LOAD( "bbs_25pts_sonido.bin",  0x0000, 0x1000, CRC(384dc9b4) SHA1(0a3ab8a7dfba958858b06d23850d1a8a2b9a348f))
ROM_END

} // anonymous namespace

//    YEAR  NAME        PARENT    MACHINE   INPUT    CLASS               INIT        ROT   COMPANY               FULLNAME                         FLAGS
GAME( 1981, ajofrin,    0,        ajofrin,  ajofrin, rfslotsmcs48_state, empty_init, ROT0, "Recreativos Franco", "Ajofrin City",                  MACHINE_IS_SKELETON_MECHANICAL )
GAME( 198?, babyfrts,   0,        babyfrts, ajofrin, rfslotsmcs48_state, empty_init, ROT0, "Recreativos Franco", "Baby Fruits (100 pts version)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1981, babyfrts25, babyfrts, babyfrts, ajofrin, rfslotsmcs48_state, empty_init, ROT0, "Recreativos Franco", "Baby Fruits (25 pts version)",  MACHINE_IS_SKELETON_MECHANICAL )
