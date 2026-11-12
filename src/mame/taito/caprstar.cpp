// license:BSD-3-Clause
// copyright-holders:

/*
Capriccio Star RS crane game board

カプリチオスター RS
K11J1004A - STAR RS

Main P.C.B.

6412394TE20 H8S/2394 CPU (XTAL unreadable, but rated for 20 MHz)
MSP430F1111A MCU with 2KB + 256B Flash Memory, 128B RAM
Xilinx Spartan XC3S200 FPGA
AT93C66 EEPROM

Video reference: https://www.youtube.com/watch?v=0NjoJH1p2U8
*/

#include "emu.h"

#include "cpu/h8/h8s2357.h"
#include "machine/eepromser.h"

#include "speaker.h"


namespace {

class caprstar_state : public driver_device
{
public:
	caprstar_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void caprstar(machine_config &config) ATTR_COLD;

private:
	void program_map(address_map &map) ATTR_COLD;

	required_device<h8s2394_device> m_maincpu;
};


void caprstar_state::program_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
}


static INPUT_PORTS_START( caprstar )
INPUT_PORTS_END


void caprstar_state::caprstar(machine_config &config)
{
	H8S2394(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &caprstar_state::program_map);

	// MSP430F1111A TODO: no core, no dump

	EEPROM_93C66_16BIT(config, "eeprom");

	SPEAKER(config, "speaker", 2).front();
}


ROM_START( caprstrs )
	ROM_REGION16_BE( 0x80000, "maincpu", 0 )
	ROM_LOAD( "f58-04.ic18", 0x00000, 0x80000, CRC(c6e6eac2) SHA1(a068e5d611e4beeae9fae26aaf933df061d60fc9) )

	DISK_REGION( "cfcard" )
	DISK_IMAGE( "f58-02", 0, SHA1(6b62241d04326cc49bb04a8ceda5a8a91e77c6e0) ) // maybe had been already read without protecting it before it was dumped
ROM_END

} // anonymous namespace


GAME( 2007, caprstrs, 0, caprstar, caprstar, caprstar_state, empty_init, ROT270, "Taito", "Capriccio Star RS", MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_NO_SOUND )
