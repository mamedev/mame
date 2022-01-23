// license:BSD-3-Clause
// copyright-holders:

/*
Skeleton driver for Oper Coin "Multi Baby" slot machine.
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
#include "speaker.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"

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

private:
	required_device<cpu_device> m_maincpu;

	void prg_map(address_map &map);
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


void multibaby_state::multibaby(machine_config &config)
{
	Z80(config, m_maincpu, 8.000_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &multibaby_state::prg_map);

	SPEAKER(config, "mono").front_center();
	ay8910_device &psg(AY8910(config, "psg", 8.000_MHz_XTAL / 4)); // Divisor unknown
	psg.port_a_read_callback().set_ioport("DSW1");
	psg.add_route(ALL_OUTPUTS, "mono", 1.0); // Guess
}

ROM_START( multibaby )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD( "oc_multi_baby_b-1923.ic13", 0x000, 0x4000, CRC(5d1bffe2) SHA1(536492b093dd247ba0440d499920d526ee7ea439) )
ROM_END

} // Anonymous namespace

GAME( 1990, multibaby, 0, multibaby, multibaby, multibaby_state, empty_init, ROT0, "Oper Coin", "Multi Baby", MACHINE_IS_SKELETON_MECHANICAL )
