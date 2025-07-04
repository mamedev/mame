// license:BSD-3-Clause
// copyright-holders:
/****************************************************************************************

 Skeleton driver for Tom's Adventures and Ice Cold Beer (ICE) electromechanical machines.

 Both machines use the same pair of PCBs.
 Main PCB:
 ___________________________________________________
|                ________________             ___   |
|o  _________   | EPROM (empty) |           TDA1519C|
|o  HD74LS144P  |_______________|   ______   |   |  |
|o               ________________  |U6295|   |___|  |
|o  _________   | EPROM         |  |_____|        o |
|o  HD74LS144P  |_______________|                 o |
|o               ________________                 o |
|   _________   | EPROM         |   ______          |
|.. HD74LS144P  |_______________|  |U6295|        o |
|..   ________                     |_____|        o |
|..  |ALTERA |           ____________             o |
|..  |EPM7032LC4412     |U635H16BDC |        ____   |
|..  |_______|          |___________|   ___  7805CT |
| _________  _________  _____________  |  |       · |
|HD74HC244P HD74LS139P | EPROM       | | <-HD74LS373P
|                      |_____________| |__|       · |
| _________  _________  _______________  ___  ___ · |
||_DIPSx8_| |_DIPSx8_| |W87C32C       | Xtal HA17555|
|                      |______________| 12.288MHz   |
| _________  _________  _________  _________      o |
||_DIPSx8_| |_DIPSx8_|  HD74LS273P HD74LS04P      o |
|                       _________  _________      o |
|                       HD74LS273P |ULN2003A|     o |
| _________  _________  _________  _________  ____  |
||ULN2003A| |ULN2003A| |ULN2003A|  HD74LS32P DS1307 |
|                                              (__) |
|  ......    .....    :::::::::::::::    ooo   BATT |
|___________________________________________________|

 And an aux I/O PCB with an EPM7032LC44-12.

 Both machines share the same game mechanics as Taito's Ice Cold Beer (ICE probably got
 some kind of license from Taito).

*****************************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/okim6295.h"
#include "speaker.h"

namespace
{

class tomsadvs_state : public driver_device
{
public:
	tomsadvs_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void tomsadvs(machine_config &config);

protected:
	required_device<mcs51_cpu_device> m_maincpu;
};

INPUT_PORTS_START(tomsadvs)
INPUT_PORTS_END

void tomsadvs_state::tomsadvs(machine_config &config)
{
	I80C32(config, m_maincpu, 12.288_MHz_XTAL); // Actually a Winbond W87C32C-40

	SPEAKER(config, "speaker", 2).front();

	okim6295_device &oki1(OKIM6295(config, "oki1", 12.288_MHz_XTAL/16, okim6295_device::PIN7_HIGH)); // Clock frequency & pin 7 not verified
	oki1.add_route(ALL_OUTPUTS, "speaker", 0.45, 0); // Guess
	oki1.add_route(ALL_OUTPUTS, "speaker", 0.45, 1); // Guess

	okim6295_device &oki2(OKIM6295(config, "oki2", 12.288_MHz_XTAL/16, okim6295_device::PIN7_HIGH)); // Clock frequency & pin 7 not verified
	oki2.add_route(ALL_OUTPUTS, "speaker", 0.45, 0); // Guess
	oki2.add_route(ALL_OUTPUTS, "speaker", 0.45, 1); // Guess
}

ROM_START(tomsadvs)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ea.u3",    0x000000, 0x010000, CRC(aaae7237) SHA1(278e366f4dafd8edd96eaadf0c9832d6ffdb906e))

	ROM_REGION(0x200000, "samples", 0)
	ROM_LOAD("tv_1.u13", 0x000000, 0x100000, CRC(af202d64) SHA1(eccfaba3783471ea2545251b83bac1ac2efe7b8f))
	ROM_LOAD("u16.u16",  0x100000, 0x100000, CRC(99ff19e0) SHA1(4fff8bc288deba062ce6f08402af057d2c41ce4c))
ROM_END

ROM_START(icecoldice)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("e982.u3", 0x000000, 0x010000, CRC(0851385e) SHA1(2b5c871a18e8d3cdc18043e18d7174cac49abbf6))

	ROM_REGION(0x200000, "samples", 0)
	ROM_LOAD("ic1.u13", 0x000000, 0x100000, CRC(eef665c1) SHA1(ef14fcaf5329cb78a96f18c6a5edbfd8f93e1580))
	ROM_LOAD("ic2.u16", 0x100000, 0x100000, CRC(7234f570) SHA1(479e2879cdfcef65dc3dbdfca650dbbe057aba26))
ROM_END

} // anonymous namespace

//    YEAR   NAME        PARENT MACHINE   INPUT     CLASS           INIT        ROT   COMPANY      FULLNAME               FLAGS
GAME( 19??,  tomsadvs,   0,     tomsadvs, tomsadvs, tomsadvs_state, empty_init, ROT0, "<unknown>", "Tom's Adventures",    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 2003?, icecoldice, 0,     tomsadvs, tomsadvs, tomsadvs_state, empty_init, ROT0, "ICE",       "Ice Cold Beer (ICE)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
