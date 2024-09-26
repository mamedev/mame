// license:BSD-3-Clause
// copyright-holders:

/*
  Skeleton driver for MCS51-based Necta/Zanussi vending machines.

  Zanussi / Necta Venezia coffee vending machine (same PCB found also on other models, like Brio):

CPU PCB (6735-365-02)
  _______________________________________________________________________________________________________
 |              ··      ···········                                ··············                       |
 |   ____                                                                                               |
 | T2512NH   ___  ___  ___  ___  ___  ___  ___  ___  ___  ___  ___  ___  ___  ___  ___  ___  ___        |
 |           RLY  RLY  RLY  RLY  RLY  RLY  RLY  RLY  RLY  RLY  RLY  RLY  RLY  RLY  RLY  RLY  RLY <- 17 relays
 |           ___   ___   _________   _________   _________   _________   _________  ___      _________  |
 |        LM393N LM393N |TD62083AP  |74HC273AN  |74HC273AN  |74HC273AN  |TD62083AP ADM692AN |74HC373N|  |
 |                          ___      _________             _________     _________     _______________  |
 |   ___                   67WR1K   |74HC244N|            |GAL16V8B|    |_DIPSx8_|    | mS6264L-10PC |  |
 | LM3578AN      ____                            _________   _________   _________    |______________|  |
 |             LM340T12    HEAT LED             |MM74HC14N  |HIN232CP|  |74HC244N|     _______________  |
 |                          RUN LED              _________               _________    | EPROM        |  |
 |                                              |MM74HC14N              |P80C32EBAA   |______________|  |
 |                                                                      |        |     Xtal       ____  |
 |                                                                      |________|  11.0592 MHz  93C56N |
 |      ...                .......   ..........   .......   .........                            EEPROM |
 |______________________________________________________________________________________________________|

KEYBOARD / DISPLAY PCB (4 digits, there are version with five)
  _______________________________________
 |·      ___   ___   ___   ___     ____ |
 |·     |__|  |__|  |__|  |__|    |    ||
 |·     |__|. |__|. |__|. |__|.   |    |<-Unknown MCU
 |                                |    ||
 |·        ____        ____       |    ||
 |·   LED |SW6 |      |SW1 | LED  |    ||
 |·       |____|      |____|      |    ||
 |·        ____        ____       |____||
 |·   LED |SW7 |      |SW2 | LED        |
 |        |____|      |____|          · |
 |·        ____        ____           · |
 |    LED |SW8 |      |SW3 | LED      · |
 | D      |____|      |____|            |
 | B       ____        ____             |
 | 9  LED |SW9 |      |SW4 | LED  ____  |
 |        |____|      |____|     |PIP | |
 |··       ____        ____      |____| |
 |..  LED |SW10|      |SW5 | LED        |
 |·       |____|      |____|            |
 |______________________________________|

*/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/eepromser.h"

namespace {

class zvenezia_state : public driver_device
{
public:
	zvenezia_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void zvenezia(machine_config &config);
	void zunknecta(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};

void zvenezia_state::machine_start()
{
}

void zvenezia_state::machine_reset()
{
}

static INPUT_PORTS_START( zvenezia )
	PORT_START("DSW0")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW0:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW0:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW0:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW0:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW0:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW0:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW0:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW0:8")
INPUT_PORTS_END

static INPUT_PORTS_START( zunknecta )
INPUT_PORTS_END

void zvenezia_state::zvenezia(machine_config &config)
{
	I80C32(config, m_maincpu, 11.0592_MHz_XTAL); // Philips P80C32EBAA

	EEPROM_93C56_8BIT(config, "eeprom");
}

void zvenezia_state::zunknecta(machine_config &config)
{
	P80C552(config, m_maincpu, 12_MHz_XTAL); // Philips PCB 80C552-5 16WP
}

ROM_START( zvenezia )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "venezia_rev_1.40_ife_14-04-95.u7", 0x00000, 0x10000, CRC(e03d9bf1) SHA1(ca728d754841993c818b5505fd3de66b3b9df13b) )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8b.bin", 0x000, 0x117, NO_DUMP )
ROM_END


/* Unknown Necta / Zanussi vending machine model:

 MAIN PCB (6735-360-00)
  _____________________________________________________________________
 |                                       _________   _________        |
 | ···      _____     ..                 |PCF8574P|  M74HC540B1  ..   |
 | ···   ·  RELAY     ..      _________  _________   _________   ..   |
 | ···   ·  _____     ..     |PCF8574P|  |PCF8574P|  |PCF8574P|  ..   |
 |       ·  RELAY     ..      _________   _________   _________  ..   |
 | ···     ··    ___  ..     M74HC541B1  M74HC541B1  M74HC541B1  ..   |
 |   ___   ··  M0C3020                   _________   _________       :|
 | M0C3020 ··                           |74HC00N_|  |74HC00N_|       :|
 | ·    ___   ___     ________   ________   ________   ________      :|
 | ·   |<-RELAY |    |ULN2064B  |ULN2064B  |ULN2064B  |ULN2064B       |
 | ·   |  |  |<-RELAY                                                :|
 | ·   |__|  |__|                                                    :|
 |  ·······   ······· ······ ······   ··········· ·········· ······· :|
 |____________________________________________________________________|

 CPU PCB
  ________________________________________________
 |                ___________     ___________    |
 | ..             |M74HC541B1|   |M74HC541B1| .. |
 | ..          ____________   _____________   .. |
 | ..          |Philips    | | EPROM       |  .. |
 | ..  _____   |80C552-5   | |_____________|  .. |
 | ..  Xtal    |           |     __________   .. |
 |    12 MHz   |___________|    |GD74HC273|      |
 |  _________  _________  ____    ____           |
 | M74HC141B1  PC74HC74P DS75176BN DS75176BN     |
 |_________________________________···· ·····____|

*/
ROM_START( zunknecta )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "prg-io_v1.1_marzo_92.u3", 0x0000, 0x8000, CRC(0a45a803) SHA1(a32f68bfc1532c6b7cc35517f408210aab962767) )
ROM_END

} // anonymous namespace


//    YEAR  NAME       PARENT  COMPAT  MACHINE    INPUT      CLASS           INIT        COMPANY            FULLNAME                                   FLAGS
SYST( 1995, zvenezia,  0,      0,      zvenezia,  zvenezia,  zvenezia_state, empty_init, "Zanussi / Necta", "Venezia (coffee vending machine)",        MACHINE_IS_SKELETON )
SYST( 1992, zunknecta, 0,      0,      zunknecta, zunknecta, zvenezia_state, empty_init, "Zanussi / Necta", "unknown Zanussi / Necta vending machine", MACHINE_IS_SKELETON )
