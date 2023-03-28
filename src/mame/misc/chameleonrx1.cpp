// license:BSD-3-Clause
// copyright-holders:
/*
    Chameleon RX-1 by Digital Sunnil, licensed to Covielsa for distribution in Spain

    Intel Pentium 4 2.66GHZ/512/533
    ASUS P4B533-M motherboard
    2x Samsung 256MB DDR PC2100 CL2.5 RAM
    SUMA GFX 5600X 128MB (Model SV3MDN0)
    Sound Blaster Live! 5.1 Digital (Model SB0220)
    Samsung SP6003H/OMD REV. A PUMA 60.0 GB

    TODO:
    - in shutms11 loads Windows 2000 fine but MAME throws "Caught unhandled exception" as soon
      as progress bar completes.
*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"

namespace {

class chameleonrx1_state : public driver_device
{
public:
	chameleonrx1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void chameleonrx1(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void chameleonrx1_map(address_map &map);
};


void chameleonrx1_state::chameleonrx1_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000e0000, 0x000fffff).rom().region("bios", 0x20000);
	map(0xfffc0000, 0xffffffff).rom().region("bios", 0);
}

static INPUT_PORTS_START( chameleonrx1 )
INPUT_PORTS_END

void chameleonrx1_state::chameleonrx1(machine_config &config)
{
	// basic machine hardware
	PENTIUM4(config, m_maincpu, 100'000'000); // actually 2.66 GHz
	m_maincpu->set_addrmap(AS_PROGRAM, &chameleonrx1_state::chameleonrx1_map);

	PCI_ROOT(config, "pci", 0);
	// ...
}

/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( chamrx1 )
	ROM_REGION32_LE(0x40000, "bios", 0)
	ROM_SYSTEM_BIOS( 0, "1003", "1003" )
	ROMX_LOAD("1003p4be.awd",                          0x00000, 0x40000, CRC(eed630e5) SHA1(18ded2b541366f00dc8a9326dfc95435f0133fb0), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "1004", "1004" )
	ROMX_LOAD("1004p4be.awd",                          0x00000, 0x40000, CRC(4f32373d) SHA1(d048eecdca3e21d4f4068f5b58e453f6e82847e2), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "1008", "1008" )
	ROMX_LOAD("1008p4be.awd",                          0x00000, 0x40000, CRC(2f395a63) SHA1(036c77905b1ab4e1ca90b0d69cee639b5ff3c8e3), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "1010", "1010" )
	ROMX_LOAD("p45m6_1010_c728_gm_u5_sst49lf002a.u26", 0x00000, 0x40000, CRC(2dd3d3eb) SHA1(5bf639442807cc1aa2ad910817a2e8d2a80e7226), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "1015", "1015 beta 2" )
	ROMX_LOAD("p4b533-m-1015.002",                     0x00000, 0x40000, CRC(1f63c423) SHA1(1fed992a969a4c932ff0a42e632ac4edf226057c), ROM_BIOS(4) )
	ROM_DEFAULT_BIOS("1010") // The one dumped from the actual arcade motherboard

	DISK_REGION( "ide:0:hdd:image" ) // Samsung SP6003H/OMD Rev.A. LBA 117,304,992 60GB PUMA
	DISK_IMAGE( "chamrx1", 0, SHA1(01da428b8aa347222856afbdfe9dbc083ae2171c) )
ROM_END

} // Anonymous namespace


GAME( 2003, chamrx1, 0, chameleonrx1, chameleonrx1, chameleonrx1_state, empty_init, ROT0, "Digital Sunnil (Covielsa license)", "Chameleon RX-1", MACHINE_IS_SKELETON )
