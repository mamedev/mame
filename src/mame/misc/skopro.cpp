// license:BSD-3-Clause
// copyright-holders:
/*
    Skonec SkoPro v1.0 PC hardware

    Intel Pentium E2160 1M Cache, 1.80 GHz, 800 MHz FSB
    Biostar MG31-M7 TE motherboard
    1GB RAM
    nVidia GeForce 8400GS
    Integrated sound?
    Hitachi Deskstar HDS721680PLA320 80 GB
    EGIS2JVS V1.2 card
    Windows XP embedded

    The system was meant to be modular, but only 2 titles are said to have been actually released.

    Announced titles:
    Douga De Puzzle - Idol Paradise
    Dragon Dance
    Exception
    Otenami Haiken Ritaanzu!
    Shanghai
*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"


namespace {

class skopro_state : public driver_device
{
public:
	skopro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void skopro(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void skopro_map(address_map &map) ATTR_COLD;
};

void skopro_state::skopro_map(address_map &map)
{
}

static INPUT_PORTS_START( skopro )
INPUT_PORTS_END

void skopro_state::skopro(machine_config &config)
{
	/* basic machine hardware */
	PENTIUM4(config, m_maincpu, 100'000'000); // actually a Pentium E2160 at 1.80 GHz
	m_maincpu->set_addrmap(AS_PROGRAM, &skopro_state::skopro_map);
	m_maincpu->set_disable();

	PCI_ROOT(config, "pci", 0);
	// ...
}

/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( drgdance )
	ROM_REGION(0x20000, "bios", 0)
	ROM_LOAD("mbbios", 0x10000, 0x10000, NO_DUMP )

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE( "dragon_dance", 0, SHA1(73868dd9354d936100ba56f460e872087ede012c) )
ROM_END

} // anonymous namespace


GAME( 2008, drgdance,  0,   skopro, skopro, skopro_state, empty_init, ROT0, "Success", "Dragon Dance (V1.02J)",  MACHINE_IS_SKELETON )
