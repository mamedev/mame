// license:BSD-3-Clause
// copyright-holders:
/****************************************************************************

    Skeleton driver for Diamond King slot game by SegaSA / Sonic.

Two PCBs found, the main one and an I/O board:

  ____|_|_|_|___|_|_|_|_|_|_|_|_|____|_|_|_|____|_|_|_|____|_|_|_|___|_|_|_|____________________________________
 |   _|_|_|_|  _|_|_|_|_|_|_|_|_|   _|_|_|_|_  _|_|_|_|   _|_|_|_|  _|_|_|_|                                   |
 |  |_CON11__||___CON10__________| |__CON9__| |__CON8_|  |__CON8_| |__CON8_|                                   |
 | __                                                                          ________                        |
_||CON __     ________  ________                         ________   ________  |_______|     ________  ________ |
_||1| | |    |_______| |_______|                        |_______|  |_______|  ________     |_______|  CD4001BE |
_||2| | |                                                                     74LS393B1                        |
_||_| |_|     ___________________                    ___________________                                       |
 | __        | OKI M81C55-5     |        ________   | OKI M81C55-5     |       ________         ____           |
_||C|        |__________________|       |_______|   |__________________|      74LS153B1        |___|           |
_||O|                                                                                                          |
_||N|                                                                                                          |
_||1|      _________                                                                                           |
_||3|     |DIPS x 8|                                                                                           |
_||_|         ___________________  ___________________  ________  ________                 ___________________ |
 |           | AY-3-8910        | | OKI M81C55-5     |  CD4011BE  74LS139N                | NEC D8085A       | |
 |           |__________________| |__________________|                                    |__________________| |
 |                                                                                      ________               |
 |                ________         ________           ________             ____________|________|_____________ |
 |               |_______|        |_______| ________ |_______|   __   __  |                   _______________|Xtal
 |                                         |_______|            | |  | |  | _______________  | ROM U3       ||6.144
 |                                                     74LS08N->| |  | |  || ROM U2       |  |______________||MHz
 |              __________    _________   _________  ________   |_|  |_|  ||______________|                  | |
 |             |___CON1__|   |___CON2_|  |________| |_______|             | ··· <- J2 (Eur/Ptas)             | |
 |______________|_|_|_|_|_____|_|_|_|_____|_|_|_|____|_|_|_|______________|__________________________________|_|
                | | | | |     | | | |     | | | |    | | | |                           SEGASA-SONIC 1B-2010-202

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
      |  ____ |________|        __________       ____        |N ||
      | |___|  _________  ____ | 8xDIPS   |     |___|        |N_||
      | ___   |________| |___| |__________|      ____         ___|
      ||CO|    _________  ____  __________      |___|        |C ||
      ||NN| F |________| |___| | 8xDIPS   |      ____        |O ||
      |     U  _________       |__________|     |___|        |N ||
      |     S |________|                         ____        |N ||
      | __  E  _________  INTEGRATED IO-3 BOARD |___|        |__||
REEL4->|C| __ |________|     ___________                         |
      ||N||C|  _________    |_OKI_M6585|                      ___|
      ||_||O| |________|   O<-GREEN  _____________           |C ||
      | __|N|  _________      LED   | ACTEL      |           |O ||
REEL3->|C||N| |________|  ____      | A40MX02    |           |N ||
      ||N||_|  _________ |___|      | SEGASA FPGA|           |N ||
      ||_|    |________|   O<-RED   | 2.1-15880.1|           |__||
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

****************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "machine/68340.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"
#include "speaker.h"

namespace
{

class diamondking_state : public driver_device
{
public:
	diamondking_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_iocpu(*this, "iocpu")
	{
	}

	void diamondking(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<m68340_cpu_device> m_iocpu;
};

static INPUT_PORTS_START(diamondking)
	// On main board
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

void diamondking_state::diamondking(machine_config &config)
{
	I8085A(config, m_maincpu, 6.144_MHz_XTAL);

	SPEAKER(config, "mono").front_center();

	ay8910_device &ay8910(AY8910(config, "ay8910", 2'000'000)); // Frequency unknown
	ay8910.port_a_read_callback().set_ioport("DSW1");
	ay8910.add_route(ALL_OUTPUTS, "mono", 1.0); // Guess

	M68340(config, m_iocpu, 16'000'000); // Frequency unknown

	msm6585_device &msm6585(MSM6585(config, "msm6585", 640'000)); // Frequency unknown
	msm6585.add_route(ALL_OUTPUTS, "mono", 1.0); // Guess
}

ROM_START(diamondking)
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("mb_ve_segasa_m-12_diamond_king_eur_ef4d_97-5848_b2018.u3", 0x00000, 0x10000, CRC(7e702012) SHA1(2858edc92fd1f672966af81ded4d6519427356bd))
	ROM_LOAD("mb_d_segasa_m-12_diamond_king_3d65_97-5848_b2018.u2",      0x10000, 0x10000, CRC(36d16147) SHA1(03060841482444eb032eca7dab777fe56f124654))

	ROM_REGION(0x100000, "iocpu", 0)
	ROM_LOAD("io_na_6.0_segasa_m-12_diamond_king_8e96_01-1105_b-00-2194.u2", 0x00000, 0x100000, CRC(e0760b1f) SHA1(eafdab3832a70e3f848a2cb9a3cb4ff6f36815db))
ROM_END

} // anonymous namespace

GAME(1997, diamondking, 0, diamondking, diamondking, diamondking_state, empty_init, ROT0, "SegaSA / Sonic", "Diamond King", MACHINE_IS_SKELETON_MECHANICAL)
