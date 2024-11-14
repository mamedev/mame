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

	void chameleonrx1_map(address_map &map) ATTR_COLD;
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
	/* basic machine hardware */
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
	ROM_LOAD("p45m6_1010_c728_gm_u5_sst49lf002a.u26", 0x00000, 0x40000, CRC(2dd3d3eb) SHA1(5bf639442807cc1aa2ad910817a2e8d2a80e7226) )

	DISK_REGION( "ide:0:hdd" ) // Samsung SP6003H/OMD Rev.A. LBA 117,304,992 60GB PUMA
	DISK_IMAGE( "chamrx1", 0, SHA1(01da428b8aa347222856afbdfe9dbc083ae2171c) )
ROM_END

} // Anonymous namespace


GAME( 2003, chamrx1,  0,   chameleonrx1, chameleonrx1, chameleonrx1_state, empty_init, ROT0, "Digital Sunnil (Covielsa license)", "Chameleon RX-1",  MACHINE_IS_SKELETON )
