// license:BSD-3-Clause
// copyright-holders:

/*
Skeleton driver for Cobra Sport Dart, darts machines from the Spanish company TourVisión.
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

There's another PCB (same connectors, fully compatible) with almost the same layout
and components, but replacing the sound OKI M6376 with a Winbond WF19054 (AY-3-8910A clone):

 FLE/V25-2
  ________________________________________________________________________________
 |  ······  ·····  ······   ··········                                           |
 |                                                                               |
 | __________    __________   __________   __________                            |
 ||MAX232CPE|   |4116R-001|  |ULN2803A_|  |4116R-001|           TDA2003          |
 |                                                      __________               |__
 |                            ___________              |ULN2803A_|                __|
 |                           |DM74LS273N|               __________                __|
 |               ___________  ___________  __________  |SN74LS273N                __|
 |              |SN74LS244N| |PALCE16V8H| |SN74LS244N|  __________   __________   __|
 | ___________   ___________         _______________   |SN74LS244N  |4116R-001|   __|
 ||SN74LS14N_|  |PALCE16V8H|        | EPROM        |    __________                __|
 | ___________                      |______________|   |SN74LS244N                __|
 ||CD4011BCN_|    ______________     _______________    __________   __________   __|
 | ____          | NEC V25     |     |CY62256L-70PC|   |DM74LS273N| |4116R-001|   __|
 |PCF8583P       | D70320L-8   |     |_____________|    __________   __________   __|
 |  Xtal         |             |                       |DM74LS273N| |ULN2803A_|   __|
 |  S833         |             |                                     __________   __|
 |               |_____________|                                    |ULN2803A_|   __|
 |                        Xtal   _________  _________   _______________     ____  __|
 |                   16.000 MHz  |_DIPSx8_| |_DIPSx8_| |WF19054       |    |RESET __|
 |    BATT        __________    __________             |______________|     ____  __|
 |               74HCT7007B1   74HCT7007B1                                 |TEST |
 |   __________                                                                  |
 |  |MAX691CPE|                    ::::::::::::        :::::::::::::::::::       |
 |_______________________________________________________________________________|

*/

#include "emu.h"

#include "cpu/nec/v25.h"
#include "machine/pcf8583.h"
#include "sound/ay8910.h"
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

	void cobrasdoki(machine_config &config);
	void cobrasday(machine_config &config);

private:
	required_device<v25_device> m_maincpu;

	void cobrasd(machine_config &config);

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;
};


void cobrasd_state::program_map(address_map &map)
{
	map(0x00000, 0x03fff).ram();
	map(0xe0000, 0xfffff).rom().region("maincpu", 0);
}

void cobrasd_state::io_map(address_map &map)
{
	// map(0x8000, 0x8000).w();
}

void cobrasd_state::data_map(address_map &map)
{
	map(0x000, 0x1ff).ram();
}


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
	m_maincpu->set_addrmap(AS_PROGRAM, &cobrasd_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &cobrasd_state::io_map);
	m_maincpu->set_addrmap(AS_DATA, &cobrasd_state::data_map);
	m_maincpu->pt_in_cb().set([this] () { logerror("%s: pt in\n", machine().describe_context()); return uint8_t(0); });
	m_maincpu->p0_in_cb().set([this] () { logerror("%s: p0 in\n", machine().describe_context()); return uint8_t(0); });
	m_maincpu->p1_in_cb().set([this] () { logerror("%s: p1 in\n", machine().describe_context()); return uint8_t(0); });
	m_maincpu->p2_in_cb().set([this] () { logerror("%s: p2 in\n", machine().describe_context()); return uint8_t(0); });
	m_maincpu->p0_out_cb().set([this] (uint8_t data) { logerror("%s: p0 out %02X\n", machine().describe_context(), data); });
	m_maincpu->p1_out_cb().set([this] (uint8_t data) { logerror("%s: p1 out %02X\n", machine().describe_context(), data); });
	m_maincpu->p2_out_cb().set([this] (uint8_t data) { logerror("%s: p2 out %02X\n", machine().describe_context(), data); });

	PCF8583(config, "rtc", 32.768_kHz_XTAL); // External xtal labeled "S833", unknown frequency
}

void cobrasd_state::cobrasdoki(machine_config &config)
{
	// Basic machine hardware

	cobrasd(config);

	// Sound hardware

	SPEAKER(config, "mono").front_center();

	OKIM6376(config, "oki", 4_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.5); // Divider not verified
}

void cobrasd_state::cobrasday(machine_config &config)
{
	// Basic machine hardware

	cobrasd(config);

	// Sound hardware

	SPEAKER(config, "mono").front_center();

	AY8910(config, "psg", 16_MHz_XTAL / 12).add_route(ALL_OUTPUTS, "mono", 0.5); // Divider not verified
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

ROM_START(cobrasda)
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("model_sc8_version_2.1.u16", 0x00000, 0x20000, CRC(3ba60087) SHA1(9a2ebc0d99dcb1f5ccf8f586776d7344d0799cf4))

	ROM_REGION(0x22e, "plds", 0)
	ROM_LOAD("palce16v8h-25.u8",  0x000, 0x117, NO_DUMP)
	ROM_LOAD("palce16v8h-25.u11", 0x117, 0x117, NO_DUMP)
ROM_END

} // anonymous namespace


//   YEAR  NAME      PARENT   MACHINE     INPUT    CLASS          INIT        ROT   COMPANY         FULLNAME                                                FLAGS
GAME(1998, cobrasd,  0,       cobrasdoki, cobrasd, cobrasd_state, empty_init, ROT0, u8"TourVisión", "Cobra Sport Dart / Tour Sport Dart (OKI M6376 sound)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1997, cobrasda, cobrasd, cobrasday,  cobrasd, cobrasd_state, empty_init, ROT0, u8"TourVisión", "Cobra Sport Dart / Tour Sport Dart (AY-8910 sound)",   MACHINE_IS_SKELETON_MECHANICAL)
