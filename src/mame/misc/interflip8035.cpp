// license:BSD-3-Clause
// copyright-holders:

// Skeleton driver for early I8035 based Interflip mechanical slots.

#include "emu.h"

#include "cpu/mcs48/mcs48.h"
#include "machine/i8243.h"
#include "machine/i8279.h"

#include "speaker.h"

/*
Main CPU: 8035 @ 6 mHz
4x 5101  (RAM 512 bytes)

Sound: 8035 @ 4mhz
RAM 2K

4051 + resistor
convertor 3 bit

3x I/O devices 8243
1x 8279 keyboard scan and display

discrete sound
*/


namespace {

class interflip8035_state : public driver_device
{
public:
	interflip8035_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void interflip(machine_config &config);

private:
	void audio_program_map(address_map &map);
	void main_program_map(address_map &map);

	required_device<cpu_device> m_maincpu;
};

void interflip8035_state::main_program_map(address_map &map)
{
	map(0x000, 0xfff).rom().region("maincpu", 0);
}

void interflip8035_state::audio_program_map(address_map &map)
{
	map(0x000, 0x7ff).rom().region("audiocpu", 0);
}


static INPUT_PORTS_START( interflip )
INPUT_PORTS_END

void interflip8035_state::interflip(machine_config &config)
{
	I8035(config, m_maincpu, 6'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &interflip8035_state::main_program_map);

	i8035_device &audiocpu(I8035(config, "audiocpu", 4'000'000));
	audiocpu.set_addrmap(AS_PROGRAM, &interflip8035_state::audio_program_map);

	I8243(config, "i8243_0");

	I8243(config, "i8243_1");

	I8243(config, "i8243_2");

	I8279(config, "kdc", 6'000'000 / 6); // clock unknown

	SPEAKER(config, "mono").front_center();

	// discrete sound
}


ROM_START( cbrava )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "cbr2p81.pal", 0x0000, 0x1000, CRC(89209629) SHA1(8f2e6acfcb3f9d3663a40b6714bc6c784a2af8db) )

	ROM_REGION( 0x800, "audiocpu", 0 ) // same as sevilla
	ROM_LOAD( "sonsev.pal", 0x000, 0x800, CRC(1043a346) SHA1(3d45e3795653a51dca7992848eb4b9ed66492b0c) ) // 1xxxxxxxxxx = 0xFF
ROM_END

ROM_START( sevilla )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "sev2p81.pal", 0x0000, 0x1000, CRC(362acdf4) SHA1(82913fe5c646be9c10252c2337ceaac2fc8173df) )

	ROM_REGION( 0x800, "audiocpu", 0 ) // same as cbrava
	ROM_LOAD( "sonsev.pal", 0x000, 0x800, CRC(1043a346) SHA1(3d45e3795653a51dca7992848eb4b9ed66492b0c) ) // 1xxxxxxxxxx = 0xFF
ROM_END

ROM_START( toledo )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "told2p87.pal", 0x0000, 0x1000, CRC(9990f5ed) SHA1(b556eb3c9ebec7b974a19ec077e81ef0429ccfe0) ) // 1xxxxxxxxxx = 0xFF

	ROM_REGION( 0x800, "audiocpu", 0 )
	ROM_LOAD( "sontol.pal", 0x000, 0x800, CRC(5066dc8c) SHA1(9bb81671525c645a633db2b8f6aed0dfe198fe63) )
ROM_END

} // anonymous namespace


GAME( 198?, cbrava,  0, interflip, interflip, interflip8035_state, empty_init, ROT0, "Interflip", "Costa Brava", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 198?, sevilla, 0, interflip, interflip, interflip8035_state, empty_init, ROT0, "Interflip", "Sevilla",     MACHINE_IS_SKELETON_MECHANICAL )
GAME( 198?, toledo,  0, interflip, interflip, interflip8035_state, empty_init, ROT0, "Interflip", "Toledo",      MACHINE_IS_SKELETON_MECHANICAL )
