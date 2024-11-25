// license:BSD-3-Clause
// copyright-holders:R. Belmont
/************************************************************************************

Star Trek Voyager (c) 2002 Team Play, Inc. / Game Refuge / Monaco Entertainment
Police Trainer 2 (c) 2003 Team Play, Inc. / Phantom Systems

skeleton driver by R. Belmont

All of these games run Linux.

Motherboard is FIC AZIIEA with AMD Duron processor of unknown speed
Chipset: VIA KT133a with VT8363A Northbridge and VT82C686B Southbridge
Video: Jaton 3DForce2MX-32, based on nVidia GeForce 2MX chipset w/32 MB of VRAM
I/O: JAMMA adapter board connects to parallel port, VGA out, audio out.
    Labelled "MEGAJAMMA 101 REV A2" for the stand-up Voyager

HDD for stand-up Voyager is a Maxtor D740X-6L 20 GB model.

Upright Voyager runs at 15 kHz standard res, sit-down at 24 kHz medium res.

TODO:
- VIA KT133a chipset support, GeForce 2MX video support, lots of things ;-)
- Run this under shutms11 will report an AC'97 error for a Trident 4DWave PCI card.

*************************************************************************************/

#include "emu.h"

#include "cpu/i386/i386.h"
#include "machine/pci.h"



namespace {

class voyager_state : public driver_device
{
public:
	voyager_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void voyager(machine_config &config);

private:
	void voyager_map(address_map &map) ATTR_COLD;

	required_device<pentium3_device> m_maincpu;
};


void voyager_state::voyager_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000e0000, 0x000fffff).rom().region("bios", 0x20000);
	map(0xfffc0000, 0xffffffff).rom().region("bios", 0);
}

static INPUT_PORTS_START( voyager )
INPUT_PORTS_END

void voyager_state::voyager(machine_config &config)
{
	PENTIUM3(config, m_maincpu, 133000000); // actually AMD Duron CPU of unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &voyager_state::voyager_map);
//  m_maincpu->set_addrmap(AS_IO, &voyager_state::voyager_io);
//  m_maincpu->set_irq_acknowledge_callback("pic8259_1", FUNC(pic8259_device::inta_cb));

	PCI_ROOT(config, "pci", 0);
	// ...
}

// unknown version and cabinet style, but believed to be the deluxe sit-down.
ROM_START( voyager )
	ROM_REGION32_LE( 0x40000, "bios", 0 )
	ROM_LOAD( "stv.u23", 0x000000, 0x040000, CRC(0bed28b6) SHA1(8e7f17af65ca9d17c5c7ddedb2313507d0ea8181) )

	ROM_REGION( 0x800, "nvram", ROMREGION_ERASE00 )

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE_READONLY( "voyager", 0, SHA1(8b94f2420f6abb40148e4ba6eed8819d8e85dbde))
ROM_END

// upright version 1.002
ROM_START( voyagers )
	ROM_REGION32_LE( 0x40000, "bios", 0 )
	ROM_LOAD( "stv.u23", 0x000000, 0x040000, CRC(0bed28b6) SHA1(8e7f17af65ca9d17c5c7ddedb2313507d0ea8181) )

	ROM_REGION( 0x800, "nvram", ROMREGION_ERASE00 )

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE_READONLY( "voyagers", 0, SHA1(839527eee24272e5ad59b871975feadfdfc07a9a))
ROM_END

ROM_START( policet2 )
	ROM_REGION32_LE( 0x40000, "bios", 0 )
	ROM_LOAD( "pm29f002t.u22", 0x000000, 0x040000, CRC(eb32ace6) SHA1(1b1eeb07e20822c690d05959077c7ddcc22d1708) )

	ROM_REGION( 0x800, "nvram", ROMREGION_ERASE00 )

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE_READONLY( "pt2", 0, SHA1(11d29548c685f12bc9bc1db7791957cd5e62db10))
ROM_END

} // anonymous namespace

GAME( 2002, voyager,  0,       voyager, voyager, voyager_state, empty_init, ROT0, "Team Play/Game Refuge/Monaco Entertainment", "Star Trek: Voyager", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
GAME( 2002, voyagers, voyager, voyager, voyager, voyager_state, empty_init, ROT0, "Team Play/Game Refuge/Monaco Entertainment", "Star Trek: Voyager (stand-up version 1.002)", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
GAME( 2003, policet2, 0,       voyager, voyager, voyager_state, empty_init, ROT0, "Team Play/Phantom Entertainment", "Police Trainer 2", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
