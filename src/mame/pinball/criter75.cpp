// license:BSD-3-Clause
// copyright-holders:
/****************************************************************************************************

  Skeleton driver for Petaco Criterium 75 pinball.

  There's an older (1975) versión of this pinball, from Recel, but it's fully 
  electromechanical (no PROMs).

  More info about the machine: https://www.recreativas.org/criterium-75-4ss-2938-petaco

  Four display PCBs with 6 7-segments (+dot) displays.
  Four logic PCBs (the chips without text had its type unreadable):
   __________________________________      __________________________________
  |       __________   __________   |     |__     __________   __________   |
  |      |_PCF7406_|  |_PCF7405_|   |     |--|   |_SN7410__|  |_PCF7418_|   |
  |__     __________   __________   |     |--|    __________   __________   |
  |--|   |_SN7403N_|  |_PCF7407_|   |     |--|   |_PCF7418_|  |_________|   |
  |--|    __________   __________   |     |--|    __________   __________   |
  |--|   |_PCF7348_|  |_PCF7405_|   |     |--|   |_________|  |_________|   |
  |--|    __________   __________   |     |--|    __________   __________   |
  |--|   |_PCF7405_|  |_________|   |     |--|   |__EMPTY__|  |_SN7445N_|   |
  |--|    __________   __________   |     |--|    __________   __________   |
  |--|   |_SN7486N_|  |_PCF7405_|   |     |--|   |__EMPTY__|  |_SN7403N_|   |
  |--|    __________   __________   |     |--|    __________   __________   |
  |__|   |_PCF7338_|  |_PCF7405_|   |     |__|   |_PCF7405_|  |_PCF7349_|   |
  |__     __________   __________   |     |       __________   __________   |
  |--|   |_PCF7401_|  |_PCF7403_|   |     |__    |_SN7400N_|  |_________|   |
  |--|    __________   __________   |     |--|    __________   __________   |
  |--|   |_SN7404__|  |_PCF7349_|   |     |--|   |_________|  |__EMPTY__|   |
  |--|    __________   __________   |     |--|    __________   __________   |
  |__|   |_PCF7348_|  |_________|   |     |--|   |__EMPTY__|  |_________|   |
  |__     __________   __________   |     |--|    __________   __________   |
  |--|   |_PCF7405_|  |_________|   |     |--|   |_________|  |_PCF7405_|   |
  |__|    __________                |     |--|    __________   __________   |
  |      |_________|                |     |__|   |_________|  |_________|   |
  |_________________________________|     |_________________________________|

   ___________B____________A_________      ________A____________B____________
  |       __________   __________   |     |     __________   __________     |
  |__    |_F9334PC_|  |_SN74155N|  1|     |11  |_________|  |_________|     |
  |--|    __________   __________   |     |     __________   __________     |
  |__|   |_F9334PC_|  |_SN74151N|  2|     |10  |_PCF7418_|  |_________|     |
  |__     __________   __________   |     |     __________   __________     |
  |--|   |_SN7404N_|  |_SN7400N_|  3|     |9     o o o o    |_________|     |
  |--|    __________   __________   |     |     __________   __________     |
  |--|   |_SN74193N|  |_SN74155N|  4|     |8   |_MM74C02N|  |_________|     |
  |--|    __________   __________   |     |     __________   __________     |
  |--|   |_SN7404N_|  |_SN7400N_|  5|     |7   |_SN7404N_|  |_________|     |
  |--|    __________   __________   |     |     __________   __________     |
  |__|   |_SN74153N|  |_SN7404N_|  6|     |6   |_SN7400N_|  |_SN74193N|     |
  |__     __________   __________   |     |     __________   __________     |
  |--|   |_SN7404N_|  |_SN7400N_|  7|     |5   |_SN7402N_|  |_SN7425N_|     |
  |--|    __________   __________   |     |     __________   __________     |
  |--|   |_F9334PC_|  |_SN74163N|  8|     |4   |_SN7493AN|  |_________|     |
  |--|    __________   __________   |     |     __________   __________     |
  |--|   |_SN74151N|  |_PROM_A9_|  9|     |3   |_SN7404N_|  |_SN7474N_|     |
  |--|    __________   __________   |     |     __________   __________     |
  |--|   |__9334PC_|  |_PROM_A10| 10|     |2   |__DIPSx8_|  |_________|     |
  |--|    __________   __________   |     |     __________   __________     |
  |--|   |_N74107A_|  |_SN74163N| 11|     |1   |_________|  |_________|     |
  |--|    __________   __________   |     |_________________________________|
  |--|   |_SN7493AN|  |_SN7493AN| 12|
  |--|    __________   __________   |
  |__|   |_SN7404N_|  |_SN7493AN| 13|
  |_________________________________|

*/

#include "emu.h"

#include "machine/netlist.h"

#include "netlist/devices/net_lib.h"

namespace {

class criter75_state : public driver_device
{
public:
	criter75_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{
	}

	void criter75(machine_config &config);

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices
	required_device<netlist_mame_device> m_maincpu;
};

static NETLIST_START(criter75)
{
	SOLVER(Solver, 48000)
	PARAM(Solver.ACCURACY, 1e-4)

	// schematics
	//...
}

void criter75_state::machine_start()
{
}

void criter75_state::machine_reset()
{
}

void criter75_state::criter75(machine_config &config)
{
	// basic machine hardware
	NETLIST_CPU(config, m_maincpu, netlist::config::DEFAULT_CLOCK()).set_source(netlist_criter75);
}

ROM_START( criter75 )
	ROM_REGION( 0x0800, "proms", ROMREGION_ERASE00 )
	ROM_LOAD( "criteum_75-e_7611.a10", 0x0000, 0x0100, CRC(2db62f9c) SHA1(6736a3be190480e8519c6f61fd1a66e42bd2be71) )
	ROM_LOAD( "criteum_75-e_7611.a9",  0x0000, 0x0100, CRC(23ee855e) SHA1(3c1f2b4e7db577e65b7e66f6fcc91b60d934962b) )
ROM_END


} // anonymous namespace

GAME( 1978, criter75, 0, criter75, 0, criter75_state, empty_init, ROT0, "Petaco", "Criterium 75", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
