// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************

    Skeleton driver for "Stop", a screenless coinop machine from Spain
    (unknown manufacturer and date). Machine has no sound hardware.

    The gameplay is simple: Stop the counter on the big display (4 digits
    with 28 leds each one) as near as posible, but without going over, from
    the number on the smaller one (4 + 1 digits, 7 segments each one).

    Four PCBs:
  _____________________________________________________________
 | _____________                                              |
 || EPROM      |     __________  __________                   |
 ||____________|    |GD74LS373| |TC4011BP_|                   |
 |                                                            |
 | _____________     ______________________  _____            |
 ||TC5517CPL-15|    |  SCN8031HCCN40      | |Xtal|     _____  |
 ||____________|    |_____________________| 8.000 MHz  7B05CT |
 |                        __________________                  |
 |_______________________|                 |__________________|
                         |_________________|

   __________________________________________________________
 _|__ ····················································· |  Each digit (28 leds)
|   | · __________ · __________ · __________ · __________ · |      o o o o
|   | ·|_ULN2803A| ·|_ULN2803A| ·|_ULN2803A| ·|_ULN2803A| · |     o       o
|   | ····················································· |     o       o
|   | · _________  · _________  · _________  · _________  · |     o       o
|___| · |MC14495P| · |MC14495P| · |MC14495P| · |MC14495P| · |     o       o
|   | ····················································· |      o o o o
|   |   __________             ____                         |     o       o
|   |  |GD74LS154|             LM386                        |     o       o
|___|                                                       |     o       o
  |_________________________________________________________|     o       o
 4 digits display.                                                 o o o o

 ___________________________
|                _________ |
| ...........   |MC14495P| |
| ·  4 x    ·    _________ |
| · 7 seg   ·   |MC14495P| |
| · display ·    _________ |
| ·         ·   |MC14495P| |
| ·         ·    _________ |
| ...........   |MC14495P| |
|        ______            |
|        7B05CT            |
| ...........   _________  |
| · 1 x 7seg·  |MC14495P|  |
| · display .              |
|   ___________________    |
|__|                  |____|
   |__________________|
 4 + 1 (7 segments) digits display

   ______________________________________________
 _|__            |ooooooooo|ooooooooo|          |
|   |     ____                                  |
|   |    |   |     _________      _________     |
|   |   74LS154   |ULN2803A|     |ULN2803A|     |
|   |    |   |     ________  ________  ________ |
|   |    |___|    |74LS04N| |74LS04N| |74LS04N| |
|___|  _____                                    |
  |___|oooo|____________________________________|

***************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"

namespace
{

class stop_state : public driver_device
{
public:
	stop_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void stop(machine_config &config);

protected:
	required_device<i8031_device> m_maincpu;
};

INPUT_PORTS_START(stop)
INPUT_PORTS_END

void stop_state::stop(machine_config &config)
{
	I8031(config, m_maincpu, 8_MHz_XTAL);
}

/* The ROM contains the string:
   "Programa desarrollado y realizado por Victoriano Angel Martinez Sanchez TORREJON DE ARDOZ (Madrid)" */
ROM_START(stop)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("m2732.bin", 0x0000, 0x1000, CRC(a596988b) SHA1(1b1a2028b6c4644cc942e46fad764b34173d25d0))
ROM_END

} // anonymous namespace

//    YEAR  NAME  PARENT MACHINE INPUT CLASS       INIT        ROT   COMPANY    FULLNAME FLAGS
GAME( 19??, stop, 0,     stop,   stop, stop_state, empty_init, ROT0, "unknown", "Stop",  MACHINE_IS_SKELETON )
