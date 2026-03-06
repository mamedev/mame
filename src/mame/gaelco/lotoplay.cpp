// license:BSD-3-Clause
// copyright-holders:
/*******************************************************************************

 Skeleton driver for "Loto-Play", a small PCB with a LED roulette installed on
 "First Games" arcade cabs from Covielsa that gives player the option to win a
 free play.

 It's a very simple PCB with this layout:
  ___________________________________
 |   _________               O <- GREEN LED
 |  |_8xDIPS_|            O     O   |
 |  _________________   O         O |
 | | MC68705P3S     |     O     O   |
 | |________________|        O      |
=|=                                 |
=|= <- 8-Pin Connector              |
 |__________________________________|

 Roulette of eight LEDs, seven red and one green (the upper one).
 There are at least two versions, one from 1988 and other from 1990.
 Some units use a M68705P5 or a Z80 instead of a MC68705P3S.

 More info and dip switches:
  - https://www.recreativas.org/loto-play-88-11392-gaelco-sa
  - https://www.recreativas.org/loto-play-90-14731-covielsa

 First Games arcade cab (see the bezel left upper corner with the roulette):
  - https://www.recreativas.org/first-games-954-covielsa

 Version with a PIC16C54 as main CPU:
  _____________________________________________
 |  |_8xDIPS_|   __________            O <- GREEN LED
 |              |SN74LS166N         O     O   |
 |                   Xtal         O         O |
 |                  4 MHz           O     O   |
 |         ___   ___                   O      |
=|=       |  |  |  |<-PIC16C54-XT/P           |
=|= <- 8-Pin Connector     ___                |
 |        |  |<-CNY/74-4  |  |<-TL7702ACP     |
 |        |__|  |__|      |__|                |
 |____________________________________________|

 There is another type of PCB  with a Z80 (exact model unknown) and a external ROM.

*******************************************************************************/

#include "emu.h"

#include "cpu/m6805/m68705.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "cpu/z80/z80.h"

namespace
{

class lotoplay_state : public driver_device
{
public:
	lotoplay_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void lotoplay_p3(machine_config &config);
	void lotoplay_pic(machine_config &config);
	void lotoplay_z80(machine_config &config);

protected:
	required_device<cpu_device> m_maincpu;
};

INPUT_PORTS_START(lotoplay)
INPUT_PORTS_END

void lotoplay_state::lotoplay_p3(machine_config &config)
{
	M68705P3(config, m_maincpu, 3'579'545); // MC68705P3S, unknown clock
}

void lotoplay_state::lotoplay_pic(machine_config &config)
{
	PIC16C54(config, m_maincpu, 4_MHz_XTAL); // PIC16C54
}

void lotoplay_state::lotoplay_z80(machine_config &config)
{
	Z80(config, m_maincpu, 4'000'000); // unknown clock
}


// Sets with MC68705.

ROM_START(lotoply)
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_LOAD("lp_mostra_sp_ultima_68705p3s.bin",     0x0000, 0x0800, CRC(112645cd) SHA1(f2ad6b2fbec36d0bfe034d7bfb036ef6bf4ee395))
ROM_END

ROM_START(lotoplya)
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_LOAD("lp_mostra_s_125d9_68705p3s.bin",       0x0000, 0x0800, CRC(9b77603c) SHA1(6799b930f9805332bf20c6146b044222fe49d243))
ROM_END

ROM_START(lotoplyb)
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_LOAD("lp_vii_sch_mostra_11302_68705p3s.bin", 0x0000, 0x0800, CRC(61b426d3) SHA1(b66dc6c382a04d8cdbaee342f179ce80abfd3c71))
ROM_END


// Sets with PIC16C54.

ROM_START(lotoplyp)
	ROM_REGION(0x1fff, "maincpu", 0)
	ROM_LOAD("loto_play_ff46_pic16c54.bin",          0x0000, 0x1fff, CRC(8840349d) SHA1(e9dcc572c7b577618ddda06be1538be69eb15584))
ROM_END


// Sets with Z80.

ROM_START(lotoplyz)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("grab._lp_sp_ultima_27c64.bin",         0x0000, 0x2000, CRC(980e14ac) SHA1(3f6dc75a8cb3fe38941b8a7900ecccdafabc14e9))
ROM_END

ROM_START(lotoplyza)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("grab._lp_s_3b4c_27c64.bin",            0x0000, 0x2000, CRC(556e2c35) SHA1(612f160592fd122e5a91914618e19eade5b52c3e))
ROM_END

ROM_START(lotoplyzb)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("multn_0.0_27c64.bin",                  0x0000, 0x2000, CRC(e74bce2a) SHA1(66a09f5df3a27b0c4bc964b19450734b736dc768))
ROM_END


} // anonymous namespace

//    YEAR   NAME       PARENT   MACHINE       INPUT     CLASS           INIT        ROT   COMPANY              FULLNAME                      FLAGS
GAME( 1988?, lotoply,   0,       lotoplay_p3,  lotoplay, lotoplay_state, empty_init, ROT0, "Gaelco / Covielsa", "Loto-Play (MC68705, set 1)", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
GAME( 1988?, lotoplya,  lotoply, lotoplay_p3,  lotoplay, lotoplay_state, empty_init, ROT0, "Gaelco / Covielsa", "Loto-Play (MC68705, set 2)", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
GAME( 1988?, lotoplyb,  lotoply, lotoplay_p3,  lotoplay, lotoplay_state, empty_init, ROT0, "Gaelco / Covielsa", "Loto-Play (MC68705, set 3)", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
GAME( 1990?, lotoplyp,  lotoply, lotoplay_pic, lotoplay, lotoplay_state, empty_init, ROT0, "Gaelco / Covielsa", "Loto-Play (PIC16C54)",       MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
GAME( 1990?, lotoplyz,  lotoply, lotoplay_z80, lotoplay, lotoplay_state, empty_init, ROT0, "Gaelco / Covielsa", "Loto-Play (Z80, set 1)",     MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
GAME( 1990?, lotoplyza, lotoply, lotoplay_z80, lotoplay, lotoplay_state, empty_init, ROT0, "Gaelco / Covielsa", "Loto-Play (Z80, set 2)",     MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
GAME( 1990?, lotoplyzb, lotoply, lotoplay_z80, lotoplay, lotoplay_state, empty_init, ROT0, "Gaelco / Covielsa", "Loto-Play (Z80, set 3)",     MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
