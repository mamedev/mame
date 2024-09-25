// license:BSD-3-Clause
// copyright-holders:

/*
Skeleton driver for Oper Coin "Multi Baby" slot machine and other games on similar hardware.
  ___________________________________________________________________
 |      ········           ········   ····                          |
 |   _________      _________  _________  _________     _________   |
 |  |________|  ·  |________| |_EMPTY__| |_EMPTY__| ·  |________|   |
 |              ·   _____                           ·             ..|
 |              ·  |BATT|                           ·             ..|
 |              ·  |_6V_|                           ·             ..|
 |   ________    ________    ________    ________    ________       |
 |  |74LS259|   |74LS259|   |74LS259|   |74LS259|   |74LS259|       |
 |..       ······               ······               ······        ·|
 |.. ________    ________    ________    ________    ________      ·|
 |..|74LS259|   |74LS259|   |74LS259|   |74LS259|   |74LS259|      ·|
 |.. ___    ________                                                |
 |.. |EM|  |TC538BP|                                                |
 |.. |PT|   ________                                                |
 |.. |Y |  |TC538BP|   _____________                                |
 |.. |__|   ________  | EMPTY      |                                |
 | ___     |74LS74APC |____________| ________                       |
 | |EM|     ________  ______________|SCL4011BE ________________ ____|
 | |PT|    |74LS363| | EPROM       | ________ | GI AY-3-8910  ||4x ||
 | |Y |     ________ |_____________||_74LS02| |_______________||DIPS|
 | |__|Xtal|74LS138| ________________                               |
 |·___ 8.000        | Z80A CPU      |                  ________   · |
 |·|  |MHz  ::::::: |_______________|                 |ULN28030   · |
 |·|  |<- ULN2803A            ··········                          · |
 |·|__| ······                ········        ········            · |
 |__________________________________________________________________|

*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/ay8910.h"

#include "speaker.h"


namespace {

class multibaby_state : public driver_device
{
public:
	multibaby_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{
	}

	void multibaby(machine_config &config);
	void spirulo(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void prg_map(address_map &map) ATTR_COLD;
};

void multibaby_state::prg_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
	//map(0x8000, 0x8000).w("psg", FUNC(ay8910_device::address_w));
	//map(0x8001, 0x8001).r("psg", FUNC(ay8910_device::data_r));
	//map(0x8002, 0x8002).w("psg", FUNC(ay8910_device::data_w));
	//map(0xe000, 0xe000).nopw();
}

static INPUT_PORTS_START( multibaby )
	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
INPUT_PORTS_END

static INPUT_PORTS_START( spirulo )
	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")
INPUT_PORTS_END


void multibaby_state::multibaby(machine_config &config)
{
	Z80(config, m_maincpu, 8.000_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &multibaby_state::prg_map);

	SPEAKER(config, "mono").front_center();
	ay8910_device &psg(AY8910(config, "psg", 8.000_MHz_XTAL / 4)); // Divisor unknown
	psg.port_a_read_callback().set_ioport("DSW1");
	psg.add_route(ALL_OUTPUTS, "mono", 1.0); // Guess
}

void multibaby_state::spirulo(machine_config &config)
{
	Z80(config, m_maincpu, 3.2768_MHz_XTAL);

	SPEAKER(config, "mono").front_center();
	ay8910_device &psg(AY8910(config, "psg", 3.2768_MHz_XTAL / 2)); // Divisor unknown
	psg.port_a_read_callback().set_ioport("DSW1");
	psg.add_route(ALL_OUTPUTS, "mono", 1.0); // Guess
}


ROM_START( multibaby )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD( "oc_multi_baby_b-1923.ic13", 0x0000, 0x4000, CRC(5d1bffe2) SHA1(536492b093dd247ba0440d499920d526ee7ea439) )
ROM_END

/* Super Pirulo (c) Oper Coin (slot machine)
   Main components:
    -Z80 CPU.
    -27256 EPROM.
    -3.2768 MHz xtal.
    -9306 SEEPROM
    -58274 Real Time Clock
    -3.6 V battery
    -1 bank of 8 dipswitches
    -5564 RAM
    -AY8910
   A complete manual with schematics can be downloaded from https://www.recreativas.org/manuales
*/
ROM_START( spirulo )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "super_pirulo_b-1785.u18", 0x0000, 0x8000, CRC(d6cafc7c) SHA1(74cd142c606a6daf10b09be1a4f7dac4da654fa0) )

	ROM_REGION(0x104, "plds", 0)
	ROM_LOAD( "16l8.u31", 0x000, 0x104, NO_DUMP )
ROM_END

} // Anonymous namespace


//    YEAR  NAME       PARENT MACHINE    INPUT      CLASS            INIT        ROT   COMPANY      FULLNAME        FLAGS
GAME( 1990, multibaby, 0,     multibaby, multibaby, multibaby_state, empty_init, ROT0, "Oper Coin", "Multi Baby",   MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1988, spirulo,   0,     spirulo,   spirulo,   multibaby_state, empty_init, ROT0, "Oper Coin", "Super Pirulo", MACHINE_IS_SKELETON_MECHANICAL ) // Year from legal registry date
