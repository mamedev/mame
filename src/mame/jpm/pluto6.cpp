// license:BSD-3-Clause
// copyright-holders:

/*
Pluto 6 hardware from Heber Ltd.

The manual lists the following components:
Motorola ColdFire MCF5206e at 40 MHz
Boot flash socket capable of accepting up to 512Kbyte
Program:
* 1 off Compact Flash (CF) Slot
* 1 off IDE port for Hard Disk or CD-ROM
* EPROM/FLASH Card in DIN41612 socket.
256K bytes (128Kx16), battery backed static RAM
2Mbytes EDO DRAM
Compact Flash card
Stereo Codec with software multichannel mixing
RTC
* 2 off 8 way DIL Switches
Serial:
* 4 off RS232 Levels (including one configured as BACTA Dataport)
* 1 off TTL level
* 1 off RS485 level
* 2 off HI2/ccTalk levels
* 4 UARTs
* Socket for optional DUART to provide a total of 6 UARTs

Optional single or dual video expansion (Calypso 32 card based on Fujitsu Cremson GPU)
*/

#include "emu.h"

#include "cpu/m68000/mcf5206e.h"
#include "machine/mcf5206e.h"

#include "speaker.h"


namespace {

class pluto6_state : public driver_device
{
public:
	pluto6_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	void pluto6(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	void program_map(address_map &map) ATTR_COLD;
};


void pluto6_state::program_map(address_map &map)
{
}


static INPUT_PORTS_START( pluto6 )
INPUT_PORTS_END


void pluto6_state::machine_start()
{
}


void pluto6_state::pluto6(machine_config &config)
{
	MCF5206E(config, m_maincpu, 40'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &pluto6_state::program_map);

	MCF5206E_PERIPHERAL(config, "maincpu_onboard", 0, m_maincpu);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}


ROM_START( pl6_kfp )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "bootrom", 0x00000, 0x80000, NO_DUMP )

	DISK_REGION( "cfcard" )
	DISK_IMAGE( "pl6_kfp", 0, SHA1(a2506e2ff67f2632bcde3281baeaad1d094309b2) )
ROM_END

ROM_START( pl6_lgk )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "bootrom", 0x00000, 0x80000, NO_DUMP )

	DISK_REGION( "cfcard" )
	DISK_IMAGE( "pl6_lgk", 0, SHA1(7d8631a4e336e93c9bfddf9166a22b756d8382fc) )
ROM_END

} // anonymous namespace


GAME( 2014, pl6_kfp, 0, pluto6, pluto6, pluto6_state, empty_init, ROT0, "G Squared", "Kung Fu Pounda",   MACHINE_IS_SKELETON_MECHANICAL )
GAME( 2014, pl6_lgk, 0, pluto6, pluto6, pluto6_state, empty_init, ROT0, "Betcom",    "Let's Get Kraken", MACHINE_IS_SKELETON_MECHANICAL )
