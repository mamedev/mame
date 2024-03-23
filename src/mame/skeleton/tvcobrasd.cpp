// license:BSD-3-Clause
// copyright-holders:

/*
Skeleton driver for Cobra Sport Dart, darts machine from the Spanish company TourVisión.
Manual, pics and some more info can be found at https://www.recreativas.org/cobra-6479-tour-vision-games

 COBRA/CPU-1
  _____________________________________________________________________________________
 |  ······  ·····  ······   ··········                                                |
 |               __________               ________________   ___________              |
 | __________   |TD62083AP|   __________ | EPROM U1      |  | OKI      |              |
 ||MAX232CPE|                |4116R-001| |_______________|  | M6376    |    TDA1010A  |
 |               __________              Xtal   __________  |__________|              |__
 |              |4116R-001|             4.000   CD74HC4060E                            __|
 |                                       MHz            __________   __________        __|
 |               ___________                           |SN74LS273N  |SN74LS138N        __|
 |              |SN74LS244N|   __________               __________                     __|
 | ___________   ___________  |SN74LS273N              |TD62083AP|                     __|
 ||SN74LS14N_|  |PALCE16V8H|   __________  __________   __________   __________        __|
 | ___________                |PALCE16V8H||SN74LS244N| |SN74LS273N  |4116R-001|        __|
 ||CD4011BCN_|    ______________     ________________   __________                     __|
 | ____          | NEC V25     |    | EPROM U16     |  |SN74LS244N|                    __|
 |PCF8583P       | D70320L-8   |    |_______________|   __________   __________        __|
 |  Xtal         |             |      _______________  |SN74LS244N| |4116R-001|        __|
 |  S833         |             |     |CY62256L-70PC |   __________   __________        __|
 |               |_____________|     |______________|  |SN74LS273N  |TD62083AP|        __|
 |                         Xtal                         __________   __________  ____  __|
 |                    16.000 MHz                       |SN74LS273N  |TD62083AP| |RESET __|
 |    BATT        __________   __________  __________   __________   __________  ____  __|
 |               |_SN7407N_|  |__DIPSx8_| |__DIPSx8_|  |SN74LS244N  |SN74LS244N |TEST |
 |   __________   __________                                                          |
 |  |MAX691CPE|  |_SN7407N_|       ::::::::::::        :::::::::::::::::::            |
 |____________________________________________________________________________________|

*/

#include "emu.h"
#include "cpu/nec/v25.h"
#include "machine/pcf8583.h"
#include "sound/okim6376.h"
#include "speaker.h"

namespace {

class cobrasd_state : public driver_device
{
public:
	cobrasd_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void cobrasd(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
};


static INPUT_PORTS_START(cobrasd)
	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")
INPUT_PORTS_END


void cobrasd_state::cobrasd(machine_config &config)
{
	// Basic machine hardware

	V25(config, m_maincpu, 16_MHz_XTAL);

	PCF8583(config, "rtc", 32.768_kHz_XTAL); // External xtal labeled "S833", unknown frequency

	// Sound hardware

	SPEAKER(config, "mono").front_center();

	OKIM6376(config, "oki", 4_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.5); // Divider not verified
}


ROM_START(cobrasd)
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("27c010a.u16", 0x00000, 0x20000, CRC(13ae6417) SHA1(cd96505c6826bc4efd1bebe27298624a8ccf3557))

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("27c040.u1",   0x00000, 0x80000, CRC(8e4a0e35) SHA1(1e182ae28b4ec63cbbadb3e36809f31396bdb27b))

	ROM_REGION(0x22e, "plds", 0)
	ROM_LOAD("palce16v8h-25.u8",  0x000, 0x117, NO_DUMP) // protected
	ROM_LOAD("palce16v8h-25.u11", 0x117, 0x117, NO_DUMP) // protected
ROM_END


} // Anonymous namespace


GAME(1998, cobrasd, 0, cobrasd, cobrasd, cobrasd_state, empty_init, ROT0, u8"TourVisión", "Cobra Sport Dart", MACHINE_IS_SKELETON_MECHANICAL) // Also knwon as Tour Sport Dart
