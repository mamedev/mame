// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************

    Skeleton driver for "Twin Basket", by Automatics Pasqual

    TODO:
        - Implement ST6225 microcontroller handling based on existing
          ST6228 core
        - Everything else (peripherals, artwork, etc.)

  ___________________________________________________________________________________
  |  _______  _______  _______   ________  _____  ___________   ____________         |
  |  | RELAY| | RELAY| | RELAY|  |_CONN__| |CONN| |___CONN___| | 3 x FUSES  |     __ |
  |  |______| |______| |______|                                |            |CN2->|_||
  |__    _________  _________                                  |____________|     __ |
  || |   |ULN2803A| |ULN2803A|         TDA2040                                    | ||
  || |   _________  _________                                     _________       |C||
  || |   |74HC374N| |74HC374N|   ___________                      M74HC244B1      |O||
  || |                           |ISD2590P  |                     _________       |N||
  || |<-CN9                      |__________|                     M74HC244B1      |N||
  || | ________  ________  _______  _________                                     | ||
  ||_| |74HC374N |74HC374N ULN2064B |74HC374N|                    _________       |_||
  |__       ___  ________  _______  _________                     M74HC244B1      __ |
  || |    93C46N |74HC374N ULN2064B |74HC374N|                    _________  SW2->| ||
  || |        _______  ______  ______  ______                     M74HC244B1 4DIPS|_||
  ||_|<-LEFT  74LS47N  74LS47N 74LS138 74LS138             XTAL   ___________     __ |
  |__  DIGITS                                            8.000MHz |ST62T25B6 |    | ||
  || |                                                            |__________|    | ||
  || |<-RIGHT                                                                SW1->|_||
  ||_| DIGITS                                                                8DIPS   |
  |__                                                                             __ |
  || |                                                                            | ||
  || |<-CN8                                                                  CN7->| ||
  || |  _________  _________  _________  _________  _________  ________  ________ | ||
  || |  |ULN2803A| |ULN2803A| |ULN2803A| |ULN2803A| |ULN2803A| ULN2803A |ULN2803A|| ||
  || |  _________  _________  _________  _________  _________  ________  ________ | ||
  ||_|  |74HC374N| |74HC374N| |74HC374N| |74HC374N| |74HC374N| 74HC374N |74HC374N||_||
  |  _______  _______  _______                                                       |
  |  | RELAY| | RELAY| | RELAY|  ________  _____                                     |
  |  |______| |______| |______|  |_CONN__| |CONN|       TWIN BASKET CPU-2            |
  |__________________________________________________________________________________|

*******************************************************************************/

#include "emu.h"
#include "cpu/st62xx/st62xx.h"

namespace
{

class twinbskt_state : public driver_device
{
public:
	twinbskt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void twinbskt(machine_config &config);

protected:
	required_device <st6228_device> m_maincpu;
};

static INPUT_PORTS_START( twinbskt )
INPUT_PORTS_END

void twinbskt_state::twinbskt(machine_config &config)
{
	ST6228(config, m_maincpu, XTAL(8'000'000));
}

ROM_START( twinbskt )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "fab_b-159_b-basket.ic8", 0x0000, 0x1000, CRC(186f1de5) SHA1(d0bc48097e17d9515cec99f86bc92b1399fbeb0b) )

	ROM_REGION( 0x1000, "samples", 0 )
	ROM_LOAD( "isd2590p.bin", 0x0000, 0x1000, NO_DUMP ) // ROM size unknown
ROM_END

} // anonymous namespace

//    YEAR  NAME      PARENT  MACHINE   INPUT     CLASS           INIT        ROT   COMPANY               FULLNAME       FLAGS
GAME( 1995, twinbskt, 0,      twinbskt, twinbskt, twinbskt_state, empty_init, ROT0, "Automatics Pasqual", "Twin Basket", MACHINE_IS_SKELETON_MECHANICAL )
