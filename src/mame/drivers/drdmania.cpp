// license:BSD-3-Clause
// copyright-holders:
/*******************************************************************************

    Skeleton driver for Sleic Dardomania darts machines
    This game has a monitor to select the game type.
    PCB is marked "SLEIC", "DIANA'94" and "13/94".

    Hardware overview:
    -Main CPU: Z0840006PSC
    -Sound: AY-3-8910
    -Other: MK48Z02B-20
    -OSCs: 18.432 MHz
    -Dips: 1 x 6 dips banks

   ______________________________________________________SN74LS74N_________________________________
  |  ______            ___      ___   ___             ___   _V_   ___                           ___|
  | |Xtal |           |  |     |  |  |  | SN74LS14N->|  |  |  |  |  |<-74LS04N                  ___|
  | |18.432Mhz        |  |     |  |  |__|<-MX699     |  |  |  |  |  |                           ___|
  |          SN74LS04N->_|     |__|<-SN74LS92N       |__|  |__|  |__|                           ___|
  |                                              ______________                                 ___|
  |     __________   __________     __________  |DARDOMANIA    |  __________                    ___|
  |    SN74LS393N|  |_74LS153N|    |AM9114EPC|  |DMP05.V2.1    | SN74LS166N|                    ___|
  |     __________                              |______________|                 ____           ___|
  |    |SN74LS08N|                               ______________                 |6x  |          ___|
  |     __________   __________     __________  |DARDOMANIA    |  __________    |DIPS|          ___|
  |    SN74LS393N|  |74LS257BN|    |AM9114EPC|  |DMP04.V2.1    | SN74LS166N|    |    |          ___|
  |                                             |______________|                |____|          ___|
  |                                              ______________                                 ___|
  |     __________   __________     __________  |DARDOMANIA    |  __________                    ___|
  |    |SN74LS08N|  |74LS157BN|    |AM9114EPC|  |DMP03.V2.1    | SN74LS166N|                    ___|
  |                                 __________  |______________|                                ___|
  |                                SN74LS125AN                       ___                        ___|
  |     __________   __________     __________       __________     |  |<-SN74LS07N            |
  |    |SN74LS20N|  |SN74LS10N|    |SN74LS32N|       SN74LS241N     |  |        __________     |
  |                                 __________   ______________     |__|        SN74LS273N     |
  |                                |SN74LS139N  |DARDOMANIA    |     _______    __________     |
  |     __________   __________                 |DMP02.V2.1    |    |AY-3  |    SN74LS273N     |
  |     SN74LS74AN  |SN74LS32N|     __________  |______________|    |8910  |    __________     |
  |                  _______       |SN74LS241N   ______________     |      |    SN74LS244N     |
  |     __________  |Z80   |                    |DARDOMANIA    |    |GI    |    __________     |
  |     SN74LS74AN  |CPU   |        __________  |DMP01.V2.1    |    |      |    SN74LS244N     |
  |       ___       |Z8040006      |SN74LS00N|  |______________|    |      |    __________     |
  |      |  |       |PSC   |                     ______________     |      |    SN74LS244N     |
  |      |  |       |      |        __________  |MK48Z02B-20   |    |      |                   |
74LS245N->  |       |Zilog |       |SN74LS241N  |ZEROPOWER RAM |    |      |                   |
  |      |__|       |      |        __________  |______________|    |      |          DIANA'94 |
  |                 |      |       |_N82S123N|                      |______|                   |
  |     __________  |______|        __________  SLEIC             __________   __________      |
  |     SN74LS74AN                 |SN74LS08N|  13-94            |_74LS86N_|  |_ LM380N_|      |
  |____________________________________________________________________________________________|

*******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "screen.h"
#include "speaker.h"

class drdmania_state : public driver_device
{
public:
	drdmania_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void drdmania(machine_config &config);

private:
	void mem_map(address_map &map);
	void io_map(address_map &map);

	required_device<cpu_device> m_maincpu;
};

void drdmania_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
}

void drdmania_state::io_map(address_map &map)
{
}

static INPUT_PORTS_START(drdmania)
INPUT_PORTS_END

void drdmania_state::drdmania(machine_config &config)
{
	Z80(config, m_maincpu, 18.432_MHz_XTAL / 4); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &drdmania_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &drdmania_state::io_map);

	//NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // MK48Z02B-20

	//SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	SPEAKER(config, "mono").front_center();

	AY8910(config, "ay8910", 18.432_MHz_XTAL / 10).add_route(ALL_OUTPUTS, "mono", 0.5); // divider not verified
}

ROM_START(drdmania)
	ROM_REGION(0x08000, "maincpu", 0)
	ROM_LOAD( "dardomania_dmp01_v2.1.ic38", 0x00000, 0x8000, CRC(9f24336f) SHA1(9a82b851d5c67a50118a3669d3bc5793e94219e4) )

	ROM_REGION(0x20000, "unsorted", 0)
	ROM_LOAD( "dardomania_dmp02_v2.1.ic33", 0x00000, 0x8000, CRC(e5dbf948) SHA1(241be0f2969b962bba602548dab3e2bdbf8f0696) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "dardomania_dmp03_v2.1.ic21", 0x08000, 0x8000, CRC(b458975e) SHA1(862d62d147ac09b86aa8d2c54b2e03a6c5436f85) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "dardomania_dmp04_v2.1.ic16", 0x10000, 0x8000, CRC(8564d0ba) SHA1(38c81173f1cf788d1a524abfae9ef7b6697383e4) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "dardomania_dmp05_v2.1.ic10", 0x18000, 0x8000, CRC(e24f2a02) SHA1(16f3a9c80b3d60c66b070521a90c958b0fc690e7) ) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION(0x20, "proms", 0)
	ROM_LOAD( "n82s123n.ic49", 0x00, 0x20, CRC(dcbd2352) SHA1(ce72e84129ed1b455aaf648e1dfaa4333e7e7628) )
ROM_END

GAME(1994, drdmania, 0, drdmania, drdmania, drdmania_state, empty_init, ROT0, "Sleic", "Dardomania (v2.1)", MACHINE_IS_SKELETON_MECHANICAL)
