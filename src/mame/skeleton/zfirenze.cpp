// license:BSD-3-Clause
// copyright-holders:

/*
  Skeleton driver for Z80-based Necta/Zanussi vending machines.

  Zanussi / Necta Firenze vending machine:

   MAIN PCB
   __________________________________________________________________________________________
  |  ······  ···········  ·········  ·········      ······  :::::::::::::  :::::::::::::    |
  |          __________    ___      __________  __________  ___  ___  ___  ___  ___  ___    |
  |         |__DS1229_|  SN76176BP |74HC374AN| |TD62083AP| |__| |__| |__| |__| |__| |__| <- 6 x TLP504A
  |                        ___       ___  ___  ___  ___    __________   __________   ___   ·|
  |                      SN76176BP  |<- 4 x TLP504A ->|   |_GET_GAL_|  |ULN2062B_| TLP504A ·|
  |                 __________      __________  __________  __________  __________   ___    |
  |                |CD74HCT86E     |74HC374AN| |74HCT244B1 |MM74HCT138N CD74HC125E TLP504A ·|
  |                 __________                                                             ·|
  |                |MM74HC163N        SPEAKER   LED                  LED                   ·|
  |                 __________                  _____  __________  __________  ____  ____  ·|
  |   ____         |SCAN-GAL_|                 DIPSx4 |DS1238-10| |M74HC04B1| 93C66N 93C66N |
  | MIC2940A        __________             Xtal                         ______________     ·|
  |                |_________|            ??? MHz                      |M5M5256CP-70LL      |
  |                 __________                          _________      |_____________|      |
  |                |M74HC32B1|   __________            |TOSHIBA  |                          |
  | __________      __________  |_D48-GAL_|            |TMPZ84C015BF-8  ··············      |
  ||MM74HCT8N|     |_KEY-GAL_|   __________            |         |           E1             |
  |                             |OKI 6242B             |         |      ··············      |
  |  ············· Osc                                 |_________|                          |
  |______________??? MHz____________________________________________________________________|

   SCHEDA EXPANSIONE MEMORIA EPROM 6735-354-00 (connected to the main PCB EPROM socket E1)
   _____________________________________________
  |        ______________   ______________     |
  |       | EPROM A     |  | EPROM C     |     |
  |       |_____________|  |_____________|     |
  |  __________   __________                   |
  | |M74HC02B1|  |PC74HC08T|   ··············  |
  |  __________   __________        J1         |
  | |M74HC32B1|  |PC74HC08T|   ··············  |
  |____________________________________________|

*/

#include "emu.h"
#include "cpu/z80/tmpz84c015.h"
#include "machine/msm6242.h"

namespace {

class zfirenze_state : public driver_device
{
public:
	zfirenze_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void zfirenze(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};

void zfirenze_state::machine_start()
{
}

void zfirenze_state::machine_reset()
{
}

static INPUT_PORTS_START( zfirenze )
	PORT_START("DSW0")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW0:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW0:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW0:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW0:4")
INPUT_PORTS_END

void zfirenze_state::zfirenze(machine_config &config)
{
	TMPZ84C015(config, m_maincpu, XTAL(8'000'000)); // Toshiba TMPZ84C015BF-8, unknown clock

	MSM6242(config, "rtc", XTAL(32'768'000)); // OKI M6242B, unknown clock
}

ROM_START( zfirenze )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "951200-1_firenze_c6m_a_8e00_20-01-95.u6", 0x00000, 0x10000, CRC(1ca85d3d) SHA1(4cb30a83b8c20eac7b31dd4fe5c79dfca6815dc8) )
	ROM_LOAD( "951200-2_firenze_c6m_c_8e00_20-01-95.u5", 0x10000, 0x10000, CRC(0107bb35) SHA1(1317d1055b9f9d05a4103612779059e84c4ac16e) )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "d48_gal_palce16v8h-25.u15",  0x000, 0x117, CRC(7467d098) SHA1(2085cccd4049ba07a1fafc1f16680e1aa8c8bb96) )
	ROM_LOAD( "get_gal_palce16v8h-25.u9",   0x000, 0x117, CRC(b4d4f0e1) SHA1(6a741e08082c46759a5eda914a91f7d18ef9128f) )
	ROM_LOAD( "key_gal_palce16v8h-25.u25",  0x000, 0x117, CRC(b52825d1) SHA1(b820c73929b03320378a4625ee4451bd81e9c5aa) )
	ROM_LOAD( "scan_gal_palce16v8h-25.u28", 0x000, 0x117, CRC(2dbb9247) SHA1(d80085bd5e9d17231a24c83797751a15b0440462) )
ROM_END


} // anonymous namespace


//    YEAR  NAME       PARENT  COMPAT  MACHINE    INPUT      CLASS           INIT        COMPANY            FULLNAME                     FLAGS
SYST( 1995, zfirenze,  0,      0,      zfirenze,  zfirenze,  zfirenze_state, empty_init, "Zanussi / Necta", "Firenze (vending machine)", MACHINE_IS_SKELETON )
