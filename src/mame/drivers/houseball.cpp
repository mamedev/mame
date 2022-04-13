// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************

    Skeleton driver for "House Ball", a electromechanical machine from the
    Spanish company Olakoa with the same game mechanics as Taito's Ice Cold
    Beer (but with different hardware). Gives bubble gums as prizes.
    There are two different versions, one with a river on the background
    artwork (the dumped one) and another one without it (with more holes).

 Main PCB
     _________________    __________________    _________________________     __________
  ___|                 |__|                  |__|                         |___|          |____
 |   |_________________|  |__________________|  |_________________________|   |__________|   |
 |  (o)                                                                                   __ |
 |    ___________  ___________              _________   _________  _________  ___________ |·||
 |__ | MC14515B | | MC14515B |  _________  |74HC14881| |ULN2803A| |ULN2803A| |MC14514BCP| |·||
 ||·||__________| |__________| SN74HC373N   _________                        |__________| |·||
 ||·|   _________  ___________ ___________ |74HC14881|                                    |_||
 ||_|  |ULN2803A| | SONIDO01 || SONIDO02 |  _________   _________  _________  ___________ __ |
 |__              |__________||__________| |_74HC08N_| |ULN2803A| |ULN2803A| |MC14514BCP| |·||
 ||·|   _________ ·· ·· ·· ··  ·· ·· ·· ··  _________                        |__________| |·||
 ||·|  SN74HC373N     __________      ____ |74HC4053N|   _________   _________  _________ |·||
 ||·|   _________    |__DIPSx8__|   LM311N  _________   |SN74LS09N  |74HC4053N |74HC4053N |_||
 ||·|  SN74HC373N                Xtal (o)  |CD4069UBCN  __________                        __ |
 ||·|   _________           8.000 MHz       _________  | CD4514BCN|  _________  _________ |·||
 ||_|  SN74HC373N                          |CD4069UBCN |__________| |74HC4053N |74HC4053N |_||
 |__    _________   _________  ___________                                                __ |
 ||·|  SN74HC373N  SN74HC245N |ST62E65BF1|  _________    _________   _________            |·||
 ||_|                         |__________| SN74HC373N   M74HC14881  |_L293B__|            |_||
 |                                           ______  _____  ______  _________________        |
 |                                          |·····| |····| |·····| |················|        |
 |___________________________________________________________________________________________|

 Control expendedoras (vending disposal)
  ____________________________________________
 | _______  _______  _______  _________      |
 ||······| |······| |······| |········|  __  |
 | _____   _____   _____   ___   ___    |·|  |
 | TIP120  TIP120  TIP120 |  |  |  |    |·|  |
 |                        |  |  |  |    |·|  |
 |            TIBPAL16L8->|  |  |  |    |·|  |
 |                        |__|  |__|<-L293NE |
 |PULSADORES VACIADO CHICLETERA         |_|  |
 | (o) (o) (o) (o) (o)        FUSE           |
 |___________________________________________|

***************************************************************************/

#include "emu.h"
#include "cpu/st62xx/st62xx.h"

namespace
{

class houseball_state : public driver_device
{
public:
	houseball_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void houseball(machine_config &config);

protected:
	required_device<st6228_device> m_maincpu;
};

INPUT_PORTS_START(houseball)
INPUT_PORTS_END

void houseball_state::houseball(machine_config &config)
{
	ST6228(config, m_maincpu, 8_MHz_XTAL); // Actually a ST62E65B
}

ROM_START(houseball)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("g.r.61156_st62e65b.u10", 0x0000, 0x1000, CRC(becdf495) SHA1(2b57f10bb7accea559c2c44d53d9a6f58aac20a9))

	// The sound is known to contain samples from Carlos Vives (La Gota Fría), probably without license
	ROM_REGION(0x2000, "samples", 0)
	ROM_LOAD("sonido01_isd2590p.u15", 0x0000, 0x1000, NO_DUMP) // Internal ROM size unknown
	ROM_LOAD("sonido02_isd2590p.u14", 0x1000, 0x1000, NO_DUMP) // Internal ROM size unknown

	// On a separate PCB for dispenser control
	ROM_REGION(0x104, "pld", 0)
	ROM_LOAD("tibpal16l8.u4", 0x000, 0x104, CRC(5d0024d4) SHA1(8e0425ca1f47c8a2e3376f0c78b3692bce3e0341))
ROM_END

} // anonymous namespace

//   YEAR  NAME       PARENT MACHINE    INPUT       CLASS            INIT        ROT   COMPANY   FULLNAME       FLAGS
GAME(1989, houseball, 0,     houseball, houseball,  houseball_state, empty_init, ROT0, "Olakoa", "House Ball",  MACHINE_IS_SKELETON_MECHANICAL)
