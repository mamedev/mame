// license:BSD-3-Clause
// copyright-holders:
/****************************************************************************************************************

  Skeleton driver for "Sport Damjes 1" darts machine

  PCB labeled as "DARDO" ("Dart" in Spanish) on both sides
  _______________________________________________________________________________________________________________
 |   .................   ...                                            .....     ........    ........          |
 |  __________   ___________   ___________                             ___________   ___________   ___________  |
 | |TC74HC367P  |TC74HC367P   |TC74HC367P                             |_ULN2803A_|  |_ULN2803A_|  |_ULN2803A_| ·|
 |  ________________________  ___________  ___________                 ___________   ___________   ___________ ·|
 | | Microchip AY-3-8910A  | |__DIPSx8__|  |__DIPSx8__|               |T74LS259B1|  |T74LS259B1|  |_ULN2803A_| ·|
 | |_______________________|                                           ___________   ___________               ·|
 |                                                                    |T74LS259B1|  |T74LS259B1|               ·|
 |                                                                     ___________   ___________               ·|
 |                                                                    |T74LS259B1|  |T74LS259B1|                |
 |  ___________________    __________                                                                           |
 | | UM6264-10L       |   |BATT 3.6V|                                                              ___________  |
 | |__________________|   |         |                                                             |_ULN2803A_| ·|
 |  ___________________   |_________|                                                              ___________ ·|
 | | EPROM            |                      ___________                                          |T74LS259B1| ·|
 | |__________________|                     |PAL16L8A-2CN                                          ___________ ·|
 |  ________________________                                                                      |T74LS259B1| ·|
 | | Zilog Z0840004PSC     |                                                                                    |
 | |_______________________|                                                                                    |
 |                                                                                                              |
 | ..                                                                  ___________   ___________                |
 | ..                                                                 |T74LS259B1|  |T74LS259B1|               ·|
 | ..                                                                                                          ·|
 | ..   Xtal     ___________                                                                                   ·|
 | ..  8.000    |__CM3080__|                         DARDO                                                     ·|
 | ..                                                                                              ___________ ·|
 | ..                                                                                             |_ULN2803A_|  |
 |                         ........       ::::::::::                         ............. ...........          |
 |______________________________________________________________________________________________________________|
  _______________________________________________________________________________________________________________
 |                                              ______ ______ ______                                            |
 |                                             |  __ ||  __ ||  __ |                                            |
 |                                             | |_| || |_| || |_| |                                            |
 |                                             | |_|.|| |_|.|| |_|.|                                            |
 |                                             |_____||_____||_____|                                            |
 |                                                                                                              |
 |                                             _                   _                                            |
 |                   _                        (_)                 (_)                       _                   |
 |                  (_)     ______ ______ ______                    ______ ______ ______   (_)                  |
 |                         |  __ ||  __ ||  __ |                   |  __ ||  __ ||  __ |                        |
 |                         | |_| || |_| || |_| |                   | |_| || |_| || |_| |                        |
 |                         | |_|.|| |_|.|| |_|.|                   | |_|.|| |_|.|| |_|.|                        |
 |                         |_____||_____||_____|                   |_____||_____||_____|                        |
 |                          ______ ______ ______                    ______ ______ ______                        |
 |                         |  __ ||  __ ||  __ |                   |  __ ||  __ ||  __ |                        |
 |                         | |_| || |_| || |_| |                   | |_| || |_| || |_| |                        |
 |                         | |_|.|| |_|.|| |_|.|                   | |_|.|| |_|.|| |_|.|                        |
 |                         |_____||_____||_____|                   |_____||_____||_____|                        |
 |                   _                                                                      _                   |
 |                  (_)                        _                   _                       (_)                  |
 |                                            (_)                 (_)                                           |
 |                                                                                                              |
 |                              ______ ______                               ______          ______ ______       |
 |                             |  __ ||  __ |                              |  __ |         |  __ ||  __ |       |
 |                             | |_| || |_| |                              | |_| |         | |_| || |_| |       |
 |                             | |_|.|| |_|.|         DARDO                | |_|.|         | |_|.|| |_|.|       |
 |                             |_____||_____|                              |_____|         |_____||_____|       |
 |______________________________________________________________________________________________________________|

****************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "speaker.h"

namespace {

class spdamjes_state : public driver_device
{

public:
	spdamjes_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void spdamjes(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};

void spdamjes_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0x9fff).ram().share("nvram");
	map(0xa000, 0xa000).w("ay8910", FUNC(ay8910_device::address_w));
	map(0xa001, 0xa001).r("ay8910", FUNC(ay8910_device::data_r));
	map(0xa002, 0xa002).w("ay8910", FUNC(ay8910_device::data_w));
	map(0xc000, 0xc007).nopw(); // output latches
	map(0xd000, 0xd007).nopw(); // more output latches
}


INPUT_PORTS_START(spdamjes)
	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

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

void spdamjes_state::spdamjes(machine_config &config)
{
	Z80(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &spdamjes_state::mem_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SPEAKER(config, "mono").front_center();

	ay8910_device &ay8910(AY8910(config, "ay8910", 8_MHz_XTAL / 4)); // Divisor unknown
	ay8910.port_a_read_callback().set_ioport("DSW1");
	ay8910.port_b_read_callback().set_ioport("DSW2");
	ay8910.add_route(ALL_OUTPUTS, "mono", 1.0); // Guess
}

ROM_START(spdamjes)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("sport_damjes.ic2", 0x0000, 0x8000, CRC(31ac5806) SHA1(cf4cb3636538687c1b72f902e77a5277996d06b2))

	ROM_REGION(0x104, "plds", 0)
	ROM_LOAD("pal16l8a.ic4", 0x000, 0x104, NO_DUMP) // Protected
ROM_END

} // Anonymous namespace

GAME(19??, spdamjes, 0, spdamjes, spdamjes, spdamjes_state, empty_init, ROT0, "T-90", "Sport Damjes 1", MACHINE_IS_SKELETON_MECHANICAL)
