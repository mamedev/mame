// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************

  Skeleton driver for first-generation (CVAX-based) DEC MicroVAX 3100 models.


  Hardhare for MicroVAX 3100 Model 10:
                                                                         ____
  Main PCB:         _________________    _____    _____    _____    _    |   |            ___   ___________
   ________________|                |___|    |___|    |___|    |___| |___|   |___________|  |__|          |_____
  |                |________________|   |____|   |____|   |____|   |_|   |___|           |  |  |__________|    |
  |    ____       ______                __________   __________         __________       |  |  __________      |
  |   |o o|      |     |   __________  |_ITA27B4_|  |_ITA27B4_|        |DP8392BN_|       |  | 16-25072-01      |
  |   |o o|      |_____|  |_ITA27B4_|           ::::::::::::::::::::::::::               |  |                  |
  |   |o o|       ______  ___________  ___________  ___________                          |__|    ____________  |
  |   |o o|      |L5170D PE-64685-001 PE-64685-001 PE-64685-001     _______  __________         |_AM7992BDC_|  |
  |   |o o|      |_____|  |_________|  |_________|  |_________|    |74F244|  16-25072-01                       |
  |   |___|                                                                                 ____________ Xtal  |
  |  __________   __________          :::::::::::::::::::::::   :::::::::::::::::::::::    |74LS244NQST| 20MHz |
  | |74LS273N_|  |74LS240N_|                       ____          __________   ___________   __________________ |
  |   Xtal           Xtal        ___________      |___|         |_7416PC__|  | MC146818P|  |AMD AM7990DC/80  | |
  |:  3.6864MHz  ..  5.0688MHz  |          |                                 |__________|  |21-21672-09______| |
  |  __________                 |LSIL5A0065|    __________________   _________________       _________________ |
  | |_74LS92N_|                 |21-22769-01   |NCR 5380         |  |NCR 5380         |     | EPROM (E25)    | |
  |      ________________       |          |   |CP07972__________|  |CP07972__________|     |________________| |
  |     | SIEMENS       |       |__________|                                __________       _________________ |
  |     | SC21C1002     |                                                  |_PROM____|      | EPROM (E24)    | |
  |     | 21-30367-03   |  Xtal             _____              __________   __________      |________________| |
  |     |               |  69.1968MHz      |____|             18-18800-02  |74LS125AN|                         |
  |     |               |                                      __________                                      |
  |     |_______________|                                     |_74F244N_|                                      |
  |  __________________               _________________                                                        |
  | | EPROM  (E98)    |              |                |        __________                     ____   ____      |
  | |_________________|              | LSI LOGIC      |       |74F74NQST|                    |___|  |___|      |
  |  _________________               | L1A5029        |        __________                                      |
  | | D43256AC-10L   |               | 21-28651-03    |       |_74F11PC_|                     ____   ____      |
  | |________________|               |                |                 1920441Q   1920441Q  |___|  |___|      |
  |  _________________               |                |        Xtal     02MP130T   02MP130T                    |
  | | D43256AC-10L   |   __________  |________________|      66.667MHz                               ____      |
  | |________________|  |_74F00PC_|                  _____________      1920441Q   1920441Q         |___|      |
  |  _________________   __________   ____________  |21-24674-17 |      02MP130T   02MP130T                    |
  | | D43256AC-10L   |  |74F04NQST|  |21-26604-07|  |  G889-41   |                                             |
  | |________________|   __________  |  H752-28  |  |   9134     |      1920441Q   1920441Q                    |
  |  _________________  |_74F00PC_|  |   9133    |  |            |      02MP130T   02MP130T   __________       |
  | | D43256AC-10L   |   __________  |           |  |____________|                           |_74F32PC_|       |
  | |________________|  |_74F32PC_|  |___________|                      1920441Q   1920441Q                    |
  |                                                                     02MP130T   02MP130T                    |
  |                                                                       __________    __________             |
  |                                                                      |74LS240N_|   |SN74AS804|             |
  |                                  :::::::::::::::   :::::::::::::::                  __________  __________ |
  |                                                                                    |SN74AS804| |_74F32N__| |
  |____________________________________________________________________________________________________________|


  Communications PCB:
   ___________________________________________________________________________________     ___________________________
  |                                                                                  |    |                          |
  |    _______________________                           ___________   ___________   |____|       ___________        |
  |   | SCN68562C4N48        |                          |74HCT245N_|  |74HCT245N_|               |_PAL20L10_|        |
  |   |______________________|                           ___________   ___________     _______________   ___________ |
  |                                                     |74HCT245N_|  |74HCT245N_|    | HM6264AP-10  |  |74F32NQST_| |
  |    _______________________                                         ___________    |______________|   ___________ |
  |   | SCN68562C4N48        |                                        |74HCT245N_|     _______________  |74F08NQST_| |
  |   |______________________|                                         ___________    | HM6264AP-10  |   ___________ |
  | ___________  .  __________    _______________                     |74HCT245N_|    |______________|  |_74F74PC__| |
  ||_74F453N__|  : |SN74LS244N   |              |                      ___________     _______________   ___________ |
  | __________      ________     | 21-26907-02  |                     |_74LS373N_|    | HM6264AP-10  |  |74F02NQST_| |
  ||SN74LS20N|     |74F10PC|     | DC7045D      |   _____________      ___________    |______________|   ___________ |
  |                              | TAC 8944     |  | DEC 358EA  |     |74HCT245N_|     _______________  |_74F74PC__| |
  |   __________  ____________   |              |  | 78532-GA   |      ___________    | HM6264AP-10  |   ___________ |
  |  |74F191PC_| |           |   |______________|  | 21-24329-01|     |_74LS373N_|    |______________|  |74F00NQST_| |
  |   __________ |           |   ______________    |P467-17 8949|      ___________                       ___________ |
  |  |_74F74PC_| |           |  |CY7C128-45PC_|    |____________|     |PAL16L8NC_|                      |74F08NQST_| |
  |   Xtal       |           |   ______________                                    ___________        ______________ |
  | 14.7456 MHz  |___________|  |PAL20L10ACNS_|                                   |MC74F521N_|       | PLS105ANJ   | |
  | ____________  ____________   ____________  __________  __________  __________  ___________       |_____________| |
  ||SN74LS244N_| |SN74LS244N_|  |SN74LS166AN| |74F245PC_| |SN74LS139| |_74F74PC_| |74HCT245N_|        ______________ |
  | ____________  ____________   ____________  __________  __________              ___________       | PLS105ANJ   | |
  ||SN74LS244N_| |SN74LS244N_|  |SN74LS166AN| |74HCT245N| |22738-01_|  40MHz Xtal |_74F543N__|       |_____________| |
  |        _____                               __________  __________  __________  ___________        ______________ |
  |       |    |      ____                    |74HCT245N| |SN74LS375N |74F32NQST| |74HCT245N_|       | PLS105ANJ   | |
  |       | C  |     |   |                     __________  __________  __________  ___________       |_____________| |
  |       | O  |     | C |                    |74F245PC_| |74F32NQST| |74F32NQST| |_74F543N__|        ______________ |
  |       | N  |     | O |                     __________  __________  __________  ___________       | EPROM       | |
  |       | N  |     | N |                    |_74F244N_| |SN74LS09N| |74F00NQST| |74HCT245N_|       |_____________| |
  |       |    |     | N |                     __________  __________  __________  ___________        ___________    |
  |       |    |     |   |                    |MC74F240N| |74F32NQST| |_________| |_74F543N__|       |_74LS244N_|    |
  |       |    |     |   |                                                         ___________        ___________    |
  |       |    |     |___|       ______________________    ______________________ |74HCT245N_|       |_74F374N__|    |
  |       |____|                | :::::::::::::::::::: |  | ::::::::::::::::::: |  ___________                       |
  |                                                                               |_74F543N__|                       |
  |__________________________________________________________________________________________________________________|

***********************************************************************************************************************/

#include "emu.h"
#include "cpu/vax/vax.h"

#include "machine/am79c90.h"
//#include "machine/ncr5380.h"
//#include "machine/scnxx562.h"
#include "machine/terminal.h"


namespace {

class uvax3100_state : public driver_device
{
public:
	uvax3100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, "terminal")
	{ }

	void uvax3100(machine_config &config) ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
};


void uvax3100_state::mem_map(address_map &map)
{
	map(0x20040000, 0x2007ffff).rom().region("maincpu", 0);
}


// Input ports
static INPUT_PORTS_START( uvax3100 )
INPUT_PORTS_END


// Model 10
void uvax3100_state::uvax3100(machine_config &config)
{
	// Basic machine hardware
	DC341(config, m_maincpu, 66.6667_MHz_XTAL / 6); // CPU CVAX 21-24674-17 11.11 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &uvax3100_state::mem_map);

	AM7990(config, "lance1", 0); // AMD AM7990PC/80

	// NCR5380(...)

	// DUSCC68562(...) // Signetics SCN68562C4N48

	// Video hardware
	GENERIC_TERMINAL(config, m_terminal, 0);
}

ROM_START( mv3100m10 )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "dec89_23-116e8-00_system_rom_lo_word.e24", 0x00000, 0x20000, CRC(69ef8cf5) SHA1(a59a500921278dc356a519cb435641426c844779) )
	ROM_LOAD32_WORD( "dec89_23-117e8-00_system_rom_hi_word.e25", 0x00002, 0x20000, CRC(df572ac3) SHA1(5e91d0f4fc8442e3ebc2424d9f85078ba00d2de5) )

	ROM_REGION( 0x20000, "scsi", ROMREGION_ERASEFF )
	ROM_LOAD(  "dec89_23-061e8-00_scsi_rom.e98",      0x00000, 0x20000, CRC(51fb8268) SHA1(a930869dce955b9b7a2b0fb68840e863c74e6512) )

	ROM_REGION( 0x10000, "comms", ROMREGION_ERASEFF )
	ROM_LOAD(  "dec90_fx9123_248e7.bin",              0x00000, 0x10000, CRC(d50801e6) SHA1(e67b1d732ce775381eb8f41684a3366db7a435d7) )

	ROM_REGION( 0x00117, "plds", ROMREGION_ERASEFF ) // All of them on the comms PCB
	ROM_LOAD(  "dec90_lm9019_032j7_pal20l10acns.bin", 0x00000, 0x00117, NO_DUMP )
	ROM_LOAD(  "dec90_lm9027_111l1_pls105anj.bin",    0x00000, 0x00100, NO_DUMP )
	ROM_LOAD(  "dec90_lm9028_519j5_pal16l8nc.bin",    0x00000, 0x00117, NO_DUMP )
	ROM_LOAD(  "dec90_lm9029_124l1_pls105anj.bin",    0x00000, 0x00100, NO_DUMP )
	ROM_LOAD(  "dec90_lm9030_031j7_pal20l10acns.bin", 0x00000, 0x00117, NO_DUMP )
	ROM_LOAD(  "dec90_lm9031_112l1_pls105anj.bin",    0x00000, 0x00100, NO_DUMP )

	ROM_REGION( 0x00100, "prom", ROMREGION_ERASEFF ) // On the main PCB
	ROM_LOAD(  "dec84_fx9206_365a1.bin",              0x00000, 0x00100, NO_DUMP )
ROM_END

} // anonymous namespace


//    YEAR  NAME       PARENT  COMPAT  MACHINE   INPUT     STATE           INIT        COMPANY                          FULLNAME                  FLAGS
COMP( 1989, mv3100m10, 0,      0,      uvax3100, uvax3100, uvax3100_state, empty_init, "Digital Equipment Corporation", "MicroVAX 3100 Model 10", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
