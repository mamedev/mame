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

 Roulette of eight LEDS, seven red and one green (the upper one).
 There are at least two version, onw from 1988 and other from 1990.

 More info and dip switches:
  - https://www.recreativas.org/loto-play-88-11392-gaelco-sa
  - https://www.recreativas.org/loto-play-90-14731-covielsa

 First Games arcade cab (see the bezel left upper corner with the roulette):
  - https://www.recreativas.org/first-games-954-covielsa

 There are other versions with external ROM, and even others with PIC16C54
 instead of the MC68705 MCU.

*******************************************************************************/

#include "emu.h"
#include "cpu/m6805/m68705.h"

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
	void lotoplay_p5(machine_config &config);

protected:
	required_device<m68705p_device> m_maincpu;
};

INPUT_PORTS_START(lotoplay)
INPUT_PORTS_END

void lotoplay_state::lotoplay_p3(machine_config &config)
{
	M68705P3(config, m_maincpu, 3'579'545); // MC68705P3S, unknown clock
}

void lotoplay_state::lotoplay_p5(machine_config &config)
{
	M68705P5(config, m_maincpu, 3'579'545); // MC68705P5S, unknown clock
}

// Sets with MC68705 MCU.

ROM_START(lotoplay)
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_LOAD("lp_mostra_sp_ultima_68705p3s.bin",     0x0000, 0x0800, CRC(112645cd) SHA1(f2ad6b2fbec36d0bfe034d7bfb036ef6bf4ee395))
ROM_END

ROM_START(lotoplaya)
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_LOAD("lp_mostra_s_125d9_68705p3s.bin",       0x0000, 0x0800, CRC(9b77603c) SHA1(6799b930f9805332bf20c6146b044222fe49d243))
ROM_END

ROM_START(lotoplayb)
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_LOAD("lp_vii_sch_mostra_11302_68705p3s.bin", 0x0000, 0x0800, CRC(61b426d3) SHA1(b66dc6c382a04d8cdbaee342f179ce80abfd3c71))
ROM_END

ROM_START(lotoplayc)
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_LOAD("miv_00_68705p5s.bin",                  0x0000, 0x0800, CRC(67b1d2fc) SHA1(98ea18d4f55faef369a8fd701f936efab3cd084f))
ROM_END

} // anonymous namespace

//    YEAR   NAME       PARENT    MACHINE      INPUT     CLASS           INIT        ROT   COMPANY              FULLNAME              FLAGS
GAME( 1988?, lotoplay,  0,        lotoplay_p3, lotoplay, lotoplay_state, empty_init, ROT0, "Gaelco / Covielsa", "Loto-Play (set 1)",  MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
GAME( 1988?, lotoplaya, lotoplay, lotoplay_p3, lotoplay, lotoplay_state, empty_init, ROT0, "Gaelco / Covielsa", "Loto-Play (set 2)",  MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
GAME( 1988?, lotoplayb, lotoplay, lotoplay_p3, lotoplay, lotoplay_state, empty_init, ROT0, "Gaelco / Covielsa", "Loto-Play (set 3)",  MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
GAME( 1988?, lotoplayc, lotoplay, lotoplay_p5, lotoplay, lotoplay_state, empty_init, ROT0, "Gaelco / Covielsa", "Loto-Play (set 4)",  MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
