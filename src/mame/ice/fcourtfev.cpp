// license:BSD-3-Clause
// copyright-holders:

/***************************************************************************************************
    Skeleton driver for: I.C.E. Full Court Fever

    PCB Layout:
   _______________________________________________________________________________
  |           ______         ______         ______         ______         ______ |
  |  ______  |TIP142 ______ |TIP142 ______ |TIP142 ______ |TIP142 ______ |TIP142 |
  | |TDA2003        |TIP142        |TIP142        |TIP142        |TIP142         |
  |                                                                              |
  |                CONN                                                          |
  |              _____                                                   CONN    |
  |  ______     LM358P      _________                                  DISPENSE  |
  | |TDA2003               |TDG2003AP                                            |
  |                         _________   _________              _                 |
  |                        M74HC273B1  |TDG2003AP      LED15->(_)                |
  |                         _________                          _                 |
  |                        |SN74HC138N                 LED14->(_)                |
  |             _________   _________                                    CONN    |
  |            M74HC273B1  |SN74HC138N                                 CAPSULE   |
  |             _________   _________   _________                                |
  |            |SN74HC374N |74HC00N_|  |_LM339N_|                                |
  |         _____________               _________              _                 |
  |        | EPROM U8   |              |GD74LS74A      LED11->(_)                |
  |        |____________|    Xtal       _________              _                 |
  |          _________                 |GD74LS74A      LED10->(_)                |
  |         |74HC373N|                  _________                                |
  |                                    |GD74LS74A                       CONN     |
  |         __________________          _________                    COIN DOOR   |
  |        | TMP68HC11A1P    |         |74HC165N|              _                 |
  |        |_________________|                         LED13->(_)                |
  |                                                            _                 |
  |         _________                                  LED12->(_)                |
  |        |DM7400N_|                   _________   _________                    |
  |         _______    _               |74HC165N|  |74HC157N|           CONN     |
  |        |L7805CV   (_)<-LED1                                        BASKET    |
  |         ________       ________       ________      ________                 |
  |        | __ ___ |     | __ ___ |     | __ ___ |    | __ ___ |                |
  |        ||__||__||     ||__||__||     ||__||__||    ||__||__||                |
  |        |________|     |________|     |________|    |________|                |
  |         ________       ________       ________      ________                 |
  |        | __ ___ |     | __ ___ |     | __ ___ |    | __ ___ |                |
  |        ||__||__||     ||__||__||     ||__||__||    ||__||__||                |
  |        |________|     |________|     |________|    |________|                |
  |                                         __________             LED->(_)      |
  |                                        |MAX7219CNG                           |
  |     BATTERY                                                                  |
  |                                                                              |
  |   FUSE                                                                       |
  |                             LAMP                                    CONN     |
  |                                                            _____   _____     |
  |                                                           |    |  |    |     |
  |                                                     ________                 |
  |                                                    | __ ___ |                |
  |                                                    ||__||__||                |
  |                                                    |________|                |
  |                                                     ________                 |
  |                                                    | __ ___ |                |
  |                                                    ||__||__||                |
  |                                                    |________|                |
  |                                    ______                                    |
  |                                   |L7805CV                                   |
  |                                                     ________                 |
  |                                                    | __ ___ |                |
  |                                                    ||__||__||                |
  |                                                    |________|                |
  |                                                     ________                 |
  |  ______       ______    ______   ______   CONN     | __ ___ |                |
  | |TIP120      |LM393P  HEF4011BP |UA723CN           ||__||__||                |
  |                                                    |________|                |
  |                                                      _________               |
  |                                                     |MAX7219CNG              |
  |______________________________________________________________________________|

*/

#include "emu.h"
#include "cpu/mc68hc11/mc68hc11.h"

namespace {

class fcourtfev_state : public driver_device
{
public:
	fcourtfev_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void fcourtfev(machine_config &config);

private:
	required_device<mc68hc11_cpu_device> m_maincpu;
};

static INPUT_PORTS_START(fcourtfev)
INPUT_PORTS_END

void fcourtfev_state::fcourtfev(machine_config &config)
{
	MC68HC11A1(config, m_maincpu, 8'000'000); // TMP68HC11A1P. Unknown xtal frequency and divisors
}

/* IAMC (International Amusement Machine Corporation, part of the Cirsa Group), resold Full Court Fever as
   Star Basket, keeping the I.C.E. PCB and just replacing the artwork. The same machine was also sold by IAMC
   renamed as "Basket All Star".
   It is unknown if the ROM is different from the I.C.E. machine. */
ROM_START(stbasket)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sbk_1.4_m27c4001.bin", 0x00000, 0x80000, CRC(4141a223) SHA1(8e4874780e98fe8ed32963cc3a48d5df3f9e413e))
ROM_END

} // anonymous namespace

GAME(20??, stbasket, 0, fcourtfev, fcourtfev, fcourtfev_state, empty_init, ROT0, "IAMC", "Star Basket", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL)
