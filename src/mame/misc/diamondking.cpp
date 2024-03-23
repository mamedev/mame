// license:BSD-3-Clause
// copyright-holders:
/******************************************************************************************************

    Skeleton driver for "SEGASA-SONIC 1B-20xx-202" hardware slot games by SegaSA / Sonic.
    Different configurations found for different games around the same main PCB (8085 + AY8910 + 8155).

*******************************************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "machine/68340.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"
#include "sound/okim6376.h"
#include "speaker.h"

namespace
{

class ss1b202base_state : public driver_device
{
public:
	ss1b202base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{
	}

	void ss1b202base(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
};

class ss1b550base_state : public driver_device
{
public:
	ss1b550base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{
	}

	void ss1b550base(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
};

class diamondking_state : public ss1b202base_state
{
public:
	diamondking_state(const machine_config &mconfig, device_type type, const char *tag)
		: ss1b202base_state(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_iocpu(*this, "iocpu"),
		m_okim6376(*this, "oki")
	{
	}

	void diamondking(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<m68340_cpu_device> m_iocpu;
	required_device<okim6376_device> m_okim6376;
};

static INPUT_PORTS_START(caribe)
	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW3:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW3:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW3:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW3:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW3:8")
INPUT_PORTS_END

static INPUT_PORTS_START(diamondking)
	// On main board, near the AY-3-8910
	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	// On I/O board
	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")

	// On I/O board
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

static INPUT_PORTS_START(goldenchip)
	// On main board, near the AY-3-8910
	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	// On main board, near one of the 8155
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

void ss1b202base_state::ss1b202base(machine_config &config)
{
	I8085A(config, m_maincpu, 6.144_MHz_XTAL);

	SPEAKER(config, "mono").front_center();

	ay8910_device &ay8910(AY8910(config, "ay8910", 2'000'000)); // Frequency unknown
	ay8910.port_a_read_callback().set_ioport("DSW1");
	ay8910.add_route(ALL_OUTPUTS, "mono", 1.0); // Guess
}

void ss1b550base_state::ss1b550base(machine_config &config)
{
	M68340(config, m_maincpu, 3.68640_MHz_XTAL);

	SPEAKER(config, "mono").front_center();

	msm6585_device &msm6585(MSM6585(config, "msm6585", 640'000)); // 0.640 MHz resonator
	msm6585.add_route(ALL_OUTPUTS, "mono", 1.0); // Guess
}

void diamondking_state::diamondking(machine_config &config)
{
	ss1b202base(config);

	M68340(config, m_iocpu, 16'000'000); // Frequency unknown

	msm6585_device &msm6585(MSM6585(config, "msm6585", 640'000)); // Frequency unknown
	msm6585.add_route(ALL_OUTPUTS, "mono", 1.0); // Guess

	// OkiM6376
	OKIM6376(config, m_okim6376, XTAL(4'194'304)/4).add_route(ALL_OUTPUTS, "mono", 1.0); // Frequency divisor is a guess
}


/* Sega S.A. "Caribe" electromechanical slot machine.
   Found just one PCB, named "1B-2004-550", which is essentially the same as the "INTEGRATED IO-3 BOARD" found on Diamond King, with minor differences.
   May need more PCBs.
*/
ROM_START(caribe)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("na.03_segasa_c-2089_1_caribe_01d-0399_b-h240190.u1", 0x00000, 0x80000, CRC(146a565e) SHA1(dcf147b3673c669af9974369d91f7ec9149b897e))
	ROM_LOAD("na.03_segasa_c-2089_2_caribe_01d-0399_b-h240190.u2", 0x80000, 0x80000, CRC(1d1b25eb) SHA1(965132d32ed573d0123ac64d5f16ca31ca2a24ff))
ROM_END

/* Four PCBs found for Diamond King (there may be more):

  ____|_|_|_|___|_|_|_|_|_|_|_|_|____|_|_|_|____|_|_|_|____|_|_|_|___|_|_|_|____________________________________
 |   _|_|_|_|  _|_|_|_|_|_|_|_|_|   _|_|_|_|_  _|_|_|_|   _|_|_|_|  _|_|_|_|                ··                 |
 |  |_CON11__||___CON10__________| |__CON9__| |__CON8_|  |__CON7_| |__CON6_|               CON5                |
 | __                                                                          ________                        |
_||CON __     ________  ________                         ________   ________  |_______|     ________  ________ |
_||1| | |    |_7407N_| |74LS14N|                        4116R-001  4116R-001  ________     |74LS14N|  CD4001BE |
_||2| | |<-7407N                                                              74LS393B1                        |
_||_| |_|     ___________________                    ___________________                                       |
 | __        | OKI M81C55-5     |        ________   | OKI M81C55-5     |       ________         ____           |
_||C|        |__________________|       |ULN2803A   |__________________|      74LS153B1        LM311P          |
_||O|                                                                                                          |
_||N|                                                                                                          |
_||1|      _________                                                                                           |
_||3|     |DIPS x 8|                                                                                           |
_||_|         ___________________  ___________________  ________  ________                 ___________________ |
 |           | AY-3-8910        | | OKI M81C55-5     |  CD4011BE  74LS139N                | NEC D8085A       | |
 |           |__________________| |__________________|                                    |__________________| |
 |                                                                                      ________               |
 |                ________         ________           ________             ____________|74LS373N|_____________ |
 |               |_______|        4116R-001 ________ |_7407N_|   __   __  |                   _______________|Xtal
 |                                         |4116R-001           | |  | |  | _______________  | ROM U3       ||6.144
 |                                                     74LS08N->| |  | |  || ROM U2       |  |______________||MHz
 |              __________    _________   _________  ________   |_|  |_|  ||______________|                  | |
 |             |___CON1__|   |___CON2_|  |__CON3___| |_CON4_|      74LS27N| ··· <- J2 (Eur/Ptas)             | |
 |______________|_|_|_|_|_____|_|_|_|_____|_|_|_|____|_|_|_|______________|__________________________________|_|
                | | | | |     | | | |     | | | |    | | | |                           SEGASA-SONIC 1B-2010-202

 Note about 1B-2010-202: The sub-PCB shown in the above diagram (where the ROMs are) is an add-on for supporting Euros.
 It's plugged on the program ROM socket (CI-16). The original version (supporting only Ptas) had exactly the same ROM
 at U3 plugged onto CI-16, without the subboard.

       ___________________________________________________________
      |     _______  ___________  ____________  ___________      |
      |    |_CONN_| |___CONN___| |___CONN____| |_LED_MPX__|      |
      | ___                                                   ___|
      ||CO|<-POWER                                           |CO||
      ||NN|        ___                                       |NN||
      ||__|       L7805CV                        ____        |__||
      |                                         |___|            |
      | ___          __                             ____   ____  |
      ||CO|<-PAYOUT | |<-TDA1013B                  |___|  |___|  |
      ||NN|         | |  _________  _________  ____              |
      ||__|         |_| |ULN2803A| |ULN2803A| |___|           ___|
      |                   ____  ____  ____       ____        |C |<-RS232 CHB
      |        _________ |___| |___| |___|      |___|        |O ||
      |  ____ |TPIC5259N        __________       ____        |N ||
      | |___|  _________  ____ | 8xDIPS   |     |___|        |N_||
      | ___   |TPIC5259N |___| |__________|      ____         ___|
      ||CO|    _________  ____  __________      |___|        |C ||
      ||NN| F |TPIC5259N |___| | 8xDIPS   |      ____        |O ||
      |     U  _________       |__________|     |___|        |N ||
      |     S |TPIC5259N                         ____        |N ||
      | __  E  _________  INTEGRATED IO-3 BOARD |___|        |__||
REEL4->|C| __ |TPIC5259N     ___________                         |
      ||N||C|  _________    |_OKI_M6585|                      ___|
      ||_||O| |TPIC5259N   O<-GREEN  _____________           |C ||
      | __|N|  _________      LED   | ACTEL      |           |O ||
REEL3->|C||N| |TPIC5259N  ____      | A40MX02    |           |N ||
      ||N||_|  _________ |___|      | SEGASA FPGA|           |N ||
      ||_|    |TPIC5259N   O<-RED   | 2.1-15880.1|           |__||
      | __                    LED   |____________|            ___|
REEL2->|C|     ____________                                  |C ||
      ||N|    | U2 EPROM  |             ____                 |O ||
      ||_|    |___________|            |___|                 |N ||
      | __ B   ____________                                  |N ||
REEL1->|C| A  |K6T0808C10 |    ____________                  |__||
      ||N| T  |___________|   |MOTOROLA   |                   ___|
      ||_| T   ____________   |MC68340FT16E              __  |C ||
      | __    | U1 EPROM  |   |           |             | |  |O ||
      ||C| PCF ____EMPTY__|   |           |             | |  |N ||
      ||N| 8583N __________   |___________|             |_|  |N ||
      | ____  |K6T0808C10 |                              __  |  ||
      |74HC140|___________|                             | |  |  ||
      | _____                                           | |  |  ||
      ||CONN I2C                                        |_|  |__||
      |    _____   ________________                   _____      |
      |   |CONN|  |__CONN__________|              CONN RS232     |
      |__________________________________________________________|

 _____________________________________________________________________________________
 |   |······CON1······|  |······CON2······|  |······CON3······|  |······CON4······|   |
 |         unused                                                                     |
 |  __________            __________                              __________          |
 | |CI-1 unused          |_ULN2803A|                             |_ULN2803A|          |
 |                         _________                                                  |
 |  __________            |HCF4094BE                               _________          |
 | |CI-2 unused                                                   |HCF4094BE          |
 |             __________                                                  __________ |
 |            |411GR-001|                                                 |411GR-001| |
 |SEGASA-SONIC __________       O <- LED                     __________  __________   |
 |1B-2006-283 |HCF4094BE|     ___________  ___________      |_74LS14N_| |HCF4094BE|   |
 |___________________________|···CON5···|_|···CON6···|________________________________|

 ______________________________________________________
 |       |····CON2····|                               _|
 |                                                   |C|
 |                                                   |O|
 |                                                   |N|
 | ___                                               |3|
 | |O|<-Volumen                            __________  |
 |                                        |_74LS14N_|  |
 |                                         __________  |
 |    ___                                 |HCF4094BE|  |
 |_  |  | <- MC14094BCP                                |
 | | |  |           ___________                        |
 |C| |  |           | OKI     |            __________  |
 |O| |__|           | M6376   |           |_74LS139N|  |
 |N|                |_________|            __________  |
 |1|                                      |_SN7417N_|  |
 |_|                                                   |
 |                _________________        __________  |
 |               | CI-1 unused    |       |MC14060BCP  |
 |  SEGASA-SONIC |________________|                    |
 |  1B-2005-425   _________________  ______            |
 |               | CI-4 EPROM     | XTAL 4.194304 MHz  |
 |               |________________|                    |
 |_____________________________________________________|

*/
ROM_START(diamondking) // With Euro support
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("mb_ve_segasa_m-12_diamond_king_eur_ef4d_97-5848_b2018.u3", 0x00000, 0x10000, CRC(7e702012) SHA1(2858edc92fd1f672966af81ded4d6519427356bd))
	ROM_LOAD("mb_d_segasa_m-12_diamond_king_3d65_97-5848_b2018.u2",      0x10000, 0x10000, CRC(36d16147) SHA1(03060841482444eb032eca7dab777fe56f124654))

	ROM_REGION(0x100000, "iocpu", 0)
	ROM_LOAD("io_na_6.0_segasa_m-12_diamond_king_8e96_01-1105_b-00-2194.u2", 0x00000, 0x100000, CRC(e0760b1f) SHA1(eafdab3832a70e3f848a2cb9a3cb4ff6f36815db))

	ROM_REGION( 0x080000, "oki", 0 ) // M6376 Samples
	ROM_LOAD( "b_segasa_m-12_diamond_king_sonido.ci4", 0x00000, 0x80000, CRC(1c0f8b4d) SHA1(38cf35e545db8f24320b0c80e6655d0a59aaec10) )
ROM_END

ROM_START(diamondkinp) // Without Euro support, just Ptas
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("mb_ve_segasa_m-12_diamond_king_eur_ef4d_97-5848_b2018.ci16", 0x00000, 0x10000, CRC(7e702012) SHA1(2858edc92fd1f672966af81ded4d6519427356bd))

	ROM_REGION(0x100000, "iocpu", 0)
	ROM_LOAD("io_na_6.0_segasa_m-12_diamond_king_8e96_01-1105_b-00-2194.u2", 0x00000, 0x100000, CRC(e0760b1f) SHA1(eafdab3832a70e3f848a2cb9a3cb4ff6f36815db))

	ROM_REGION( 0x080000, "oki", 0 ) // M6376 Samples
	ROM_LOAD( "b_segasa_m-12_diamond_king_sonido.ci4", 0x00000, 0x80000, CRC(1c0f8b4d) SHA1(38cf35e545db8f24320b0c80e6655d0a59aaec10) )
ROM_END

/* Golden Chip uses an different revision of the main PCB found on Diamond King. It has a battery backed RAM (undumped) and just one ROM:

  ____|_|_|_|___|_|_|_|_|_|_|_|_|__________________________|_|_|_|______________________________________________
 |   _|_|_|_|  _|_|_|_|_|_|_|_|_|   _________  ________   _|_|_|_|  ________                ··                 |
 |  |_CON11__||___CON10__________| |__CON9__| |__CON8_|  |__CON7_| |__CON6_|               CON5                |
 | __                                unused     unused              unused     ________   unused               |
_||CON __     ________  ________                         ________   ________  |74LS74AN     ________  ________ |
_||1| | |    |_7407N_| |74LS14N|                        4116R-001  4116R-001  ________     |74LS14N|  CD4001BE |
_||2| | |<-7407N                                                              74LS393B1                        |
_||_| |_|     ___________________                    ___________________                                       |
 | __        | NEC D81C55HC     |        ________   | NEC D81C55HC     |       ________         ____           |
_||C|        |__________________|       |_DIPSx8|   |__________________|      74LS153B1        LM311P          |
_||O|                                                                       _____________                      |
_||N|                                                                      |  ___       |                      |
_||1|      _________                                                       | /   \ BATT |                      |
_||3|     |DIPS x 8|                                                       | \___/      |                      |
_||_|         ___________________  ___________________  ________  ________ | ___________| ___________________  |
 |           | AY-3-8910        | | NEC D81C55HC     |  CD4011BE  74LS139N || 6116L-5  ||| NEC D8085A       |  |
 |           |__________________| |__________________|                     ||__________|||__________________|  |
 |  ________                                                               |____________| ________             |
 | |_LM380N|      ________         ________           ________                           |74LS373N             |
 |               4116R-001        4116R-001          |_7407N_|            _______________                      |
 |                                          ________                     | ROM U2       |             Xtal     |
 |                                         |4116R-001            ______  |______________|             6.144    |
 |              __________    _________   _________  ________   |BATT  |                              MHz      |
 |             |___CON1__|   |___CON2_|  |__CON3___| |_CON4_|   |3.6V__|                                       |
 |______________|_|_|_|_|_____|_|_|_|_____|_|_|_|____|_|_|_|___________________________________________________|
                | | | | |     | | | |     | | | |    | | | |                           SEGASA-SONIC 1B-2003-202

Additional PCBs found on Golden Chip:

 _________|_|_|_|_______|_|_|_|_____________
 |       _|_|_|_|_     _|_|_|_|_            |
 |      | CON 1  |    | CON 2  |            |
 | __    _________     _________          __|
_||C|   |899-3-R470   |4116R-001         |C||_
_||O|    _________     _________         |O||_
_||N|   |74LS299N|    |74LS299N|   ___   |N||_
_||7|                             |  |   |3||_
_|| |       1B-2002-205           |  |   | ||_
_|| |                             |__|   | ||_
_|| |    _________  _________  _________ |_||
_|| |   |74LS299N| |74LS299N| |74LS299N|    |
 ||_|    _________  _________  _________    |
 |      |ULN2003A| |ULN2003A| |ULN2003A|    |
 |       _________  _________  _________    |
 |      | CON 6  | | CON 5  | | CON 4  |    |
 |_______|_|_|_|_____|_|_|_|____|_|_|_|_____|
         | | | |    SEGASA SONIC 1B-2002-205
 ________________________________________
 | __               _________  _________ |
_||C|              |SN7407N_| |________| |
_||O|    _________  _________  _________ |
_||N|   |74LS251N| |_EMPTY__| |_EMPTY__| |
_||4|                                    |
_||_|    _________  _________  _________ |
 |      |4116R-001 |_EMPTY__| |_EMPTY__| |
 |       _________  _________  _________ |
 |      | CON 3  | | CON 2  | | CON 1  | |
 |_______|_|_|_|____ unused ___ unused __|
         | | | |  SEGASA SONIC 1B-2001-206
*/
ROM_START(goldenchip)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("segasa_g.chip_l_chl.bin", 0x00000, 0x08000, CRC(1bc48a7d) SHA1(765129c552cc1e515a5ec1aba23d8663add5f025))

	ROM_REGION(0x00800, "aux", 0)
	ROM_LOAD("hm3_6116l-5.bin", 0x00000, 0x00800, NO_DUMP) // Battery backed RAM, undumped
ROM_END

} // anonymous namespace

//   YEAR  NAME         PARENT       MACHINE      INPUT        CLASS              INIT        ROT   COMPANY           FULLNAME                               FLAGS
GAME(2001, caribe,      0,           ss1b550base, caribe,      ss1b550base_state, empty_init, ROT0, "SegaSA / Sonic", "Caribe",                              MACHINE_IS_SKELETON_MECHANICAL)
GAME(1997, diamondking, 0,           diamondking, diamondking, diamondking_state, empty_init, ROT0, "SegaSA / Sonic", "Diamond King (with Euro support)",    MACHINE_IS_SKELETON_MECHANICAL)
GAME(1997, diamondkinp, diamondking, diamondking, diamondking, diamondking_state, empty_init, ROT0, "SegaSA / Sonic", "Diamond King (without Euro support)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(19??, goldenchip,  0,           ss1b202base, goldenchip,  ss1b202base_state, empty_init, ROT0, "SegaSA / Sonic", "Golden Chip",                         MACHINE_IS_SKELETON_MECHANICAL)
