// license:BSD-3-Clause
// copyright-holders:

/***************************************************************************************************
    Skeleton driver for ITISA Flash Gun

    The game projects "clay pigeons" (really just white light spots) on a fixed Surface with a
    painted landscape, moving them with a combination of lenses and motors, and then the player
    shoots at them with a lightgun (there are two rifles, it's a two players game).

    More info: https://www.recreativas.org/flash-gun-2404-itisa-electronics

    PCB layout:
   ______________________________________________________________________________________
  |  _    ___________________       __________                __________    _______     |
  | (_)  | Z8400APS Z80 CPU |      |M74LS74AP|   Xtal        |74LS259N_|   |DIPSx4|     |
  | LED  |__________________|                    12 MHz                                 |
 _|_      __________   __________   _______      __________   __________   __________   |
|   |    |M74LS244P|  |M74LS74AP|  |RC555N|     |M74LS04P_|  |M74LS138P|  |M74LS74AP|   |
|   |                                                                                   |
|   |     __________   __________   __________   __________   __________   __________   |
|   |    |M74LS244P|  |M74LS245P|  |_74LS14N_|  |M74LS138P|  |74LS259N_|  |M74LS240P|   |
|   |   ____________                                                                    |
|___|  | EPROM     |   __________   __________   __________   __________   __________   |
  |    |___________|  |__7406N__|  |M74LS138P|  |M74LS273P|  |ULN2803A_|  |M74LS240P|   |
  |     ____________                                                                    |
  |    |HM6116LP-3 |   __________   __________   __________   __________   __________   |
  |    |___________|  |__7406N__|  |M74LS174P|  |M74LS174P|  |__7407N__|  |M74LS240P|   |
  |                                                                                     |
  |       __________   __________   __________   __________                             |
  |      |__7402N__|  |__7402N__|  |__7407N__|  |__7407N__|                             |
 _|_    ____________________                                                            |
|   |  | AY-3-8910         |              __________                                    |
|   |  |___________________|             |HA1366WR |                                    |
|   |   ____________________                                                            |
|   |  | AY-3-8910         |              __________                                    |
|   |  |___________________|             |HA1366WR |                                    |
|___|   ____________________                                                            |
  |    | AY-3-8910         |              __________                                    |
  |    |___________________|             |HA1366WR |                                    |
  |                                  ____                                         ______|
  |_________________________________|    |_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|

*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/ay8910.h"

#include "speaker.h"

namespace {

class flashgun_state : public driver_device
{
public:
	flashgun_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ay8910(*this, "ay%u", 1U)
	{
	}

	void flashgun(machine_config &config);

private:
	required_device<z80_device> m_maincpu;
	required_device_array<ay8910_device, 3> m_ay8910;
};

static INPUT_PORTS_START(flashgun)
	PORT_START("DSW0")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW0:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW0:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW0:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW0:4")
INPUT_PORTS_END

void flashgun_state::flashgun(machine_config &config)
{
	Z80(config, m_maincpu, 12_MHz_XTAL / 4); // Guess

	SPEAKER(config, "speaker").front_center();

	AY8910(config, m_ay8910[0], 12_MHz_XTAL / 8); // Guess
	m_ay8910[0]->port_a_read_callback().set_ioport("DSW0");
	m_ay8910[0]->add_route(ALL_OUTPUTS, "speaker", 0.25);

	AY8910(config, m_ay8910[1], 12_MHz_XTAL / 8); // Guess
	m_ay8910[1]->add_route(ALL_OUTPUTS, "speaker", 0.25);

	AY8910(config, m_ay8910[2], 12_MHz_XTAL / 8); // Guess
	m_ay8910[2]->add_route(ALL_OUTPUTS, "speaker", 0.25);

}

ROM_START(flashgun)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("2764.a4", 0x0000, 0x2000, CRC(c3a1fc10) SHA1(356a4afab196f972d9bfcec554a06ba51162068d))
ROM_END

} // anonymous namespace

GAME(1986, flashgun, 0, flashgun, flashgun, flashgun_state, empty_init, ROT0, "Itisa", "Flash Gun", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL)
