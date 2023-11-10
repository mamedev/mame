// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************

 Skeleton driver for "Seoul 88 Fever", a Korean video slot from Mecca.

  ____________________________________________________________________________________________________
 | ____      ____________________    ____________   _________   ________   _________  _________       |
 ||BATT|    | ZILOG Z0840006PSC  |  | ROM U02    | |PAL22V10_| |74LS245N| |__DIPSx8_||__DIPSx8_|      |
 ||____|    |____________________|  |____________|                                                    |
 |                                                   ________   ________                           ___|
 |                                   ____________   |74LS273N| |74LS245N|     ________   ___      |  
 |  ___      ________   _________   |GM76C88AL-15|                           |74LS245N|  |()|     |___
 |  Xtal    |74LS157N| |HY18CV8S_|  |____________|   ________  _________                Switch      __|
 | 12.000                                           |74LS245N||PALCE22V10H    ________              __|
 |                                                                           |74LS245N|             __|
 | ________     ________      ________    ________   ________   ________                            __|
 ||74HCTLS04N  |74LS273N|    |74LS157N|  |N82S129N| |74LS245N| |74LS174N|     ________              __|
 |                                                                           |74LS245N|             __|
 |              ________                             ________   ________                            __|
 |             |74LS273N|                           |74LS245N| |N82S129N|     ________              __|
 |                                                                           |74LS273N|             __|
 | ________                      ______________      ________   ________                            __|
 ||74LS74AN|    ____________    | ACTEL        |    |74LS245N| |N82S129N|     ________              __|
 |             | 28F1000PPC |   | A40MX04-F    |                             |74LS245N|             __|
 | ________    |____________|   | PL84 0017    |     ________   ________                            __|
 ||74LS174AN                    |              |    |74LS273N| |N82S129N|     ________              __|
 |              ____________    |______________|                             |SN76489AN             __|
 | ________    | HY6264P-12 |                        ________   ________                            __|
 ||74LS157N|   |____________|                       |74LS273N| |N82S129N|                           __|
 |                                                                                                  __|                    
 | ________                                          ________   ________                           ___|
 ||74LS157N|    ____________     ____________       |74LS273N| |N82S129N|                         |
 |             | LH5168-10L |   | A277308-90 |                                                    |___
 | ________    |____________|   |____________|       ________   ________                              |
 ||74LS157N|                                        |74LS174N| |N82S129N|                             |
 |                                                                                                    |
 | ··      ________    ________    ________    ________    ________       ________   ________         |
 | ··    MC74HC574AN  |74HC174AN   AT89C2051  |74HC245P|  |74HC245P|     MC74HC74AN IN74HC174AN       |
 | ··                                                                                                 |
 | ··      ________    ________      ____      ________                   ________   ________         |
 | ··    MC74HC574AN  |ULN2003AN     Xtal     |_DIPSx8_|    Switch       |74HC174AN |ULN2003AN        |
 |                                  24.000                                                            |
 |____________________________________________________________________________________________________|

***********************************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"

#include "screen.h"
#include "speaker.h"

namespace {

class seoul88_state : public driver_device
{
public:
	seoul88_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void seoul88( machine_config &config );

private:
	required_device<cpu_device> m_maincpu;
};

static INPUT_PORTS_START( seoul88 )
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

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW3:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW3:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW3:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW3:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW3:8")
INPUT_PORTS_END

void seoul88_state::seoul88(machine_config &config)
{
	// Basic machine hardware
	Z80( config, m_maincpu, 12.000_MHz_XTAL / 2 );

	// AT89C2051 MCU
	//...

	// Video hardware
	//SCREEN(config, "screen", SCREEN_TYPE_RASTER );

	// Audio hardware
	SPEAKER( config, "mono" ).front_center();
}

ROM_START( seoul88 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "am27c512.u02",        0x00000, 0x10000, CRC(890b2d38) SHA1(28e6f5d84c9b283ad565236747aca39bc00c0efb) )

	ROM_REGION( 0x40000, "data", 0 )
	ROM_LOAD( "amic_a277308-90.u07", 0x00000, 0x20000, CRC(ea0aafdc) SHA1(232fc5a542d7b61f466e82bc0a8e14b3f2f81e1d) )
	ROM_LOAD( "mx_28f1000ppc.u43",   0x20000, 0x20000, CRC(b824f1c6) SHA1(a390e7cc4e5705770f4f8d9c604ad304982aabf8) )

	ROM_REGION( 0x4000, "mcu", 0 )
	ROM_LOAD("at89c2051-24pc.u1",    0x0000, 0x4000, NO_DUMP) // 2 Kbytes internal ROM

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "88-s1.bin",           0x00000, 0x00100, CRC(a18f1b83) SHA1(6ea1980c5f686933ae05922671e1d7c9561ba62a) )
	ROM_LOAD( "88-s2.bin",           0x00100, 0x00100, CRC(452e6591) SHA1(83c16fdc839e634bada4f754f582806c207fe2f1) )
	ROM_LOAD( "88-s3.bin",           0x00200, 0x00100, CRC(c242965a) SHA1(0654f38af98d906e8e993907bb2e8f1084937092) )
	ROM_LOAD( "88-s4.bin",           0x00300, 0x00100, CRC(1e4cc1ad) SHA1(2d93d267320525c44d9eb4ee17cf8ebf69842cb4) )
	ROM_LOAD( "88-s5.bin",           0x00400, 0x00100, CRC(67a6b674) SHA1(5a810c5e9da71dc6465ff843c55f61bd321e0b1e) )
	ROM_LOAD( "88-s6.bin",           0x00500, 0x00100, CRC(11c88e29) SHA1(16b07fe6a83b3a300fdb081609729595c16588e9) )
	ROM_LOAD( "88-s7.bin",           0x00600, 0x00100, CRC(83c3ec8f) SHA1(4a6452ef73061a446e6a8ceb9d077bc71cc8e2b2) )

	ROM_REGION( 0x3e1f, "plds", 0 )
	ROM_LOAD( "hy18cv8s-25.u10",     0x00000, 0x00117, NO_DUMP )
	ROM_LOAD( "pal22v10-7pc.u18",    0x00117, 0x01e84, NO_DUMP )
	ROM_LOAD( "pal22v10-25pc.u17",   0x01f9b, 0x01e84, NO_DUMP )
ROM_END

} // anonymous namespace


//    YEAR  NAME     PARENT  MACHINE  INPUT    CLASS          INIT        ROT   COMPANY  FULLNAME          FLAGS
GAME( 1989, seoul88, 0,      seoul88, seoul88, seoul88_state, empty_init, ROT0, "Mecca", "Seoul 88 Fever", MACHINE_IS_SKELETON )
