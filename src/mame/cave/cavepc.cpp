// license:BSD-3-Clause
// copyright-holders:David Haywood
/**************************************************************************************************

    CAVE PC hardware
    placeholder file for information

TODO:
- Cannot continue without a proper Athlon 64 X2 core (uses lots of unsupported RDMSR / WRMSR)

***************************************************************************************************

 Cave used a one-off PC platform for

 Deathsmiles II (c)2009

 According to
 http://forum.arcadeotaku.com/viewtopic.php?f=26&t=9851
 It uses a ASUS M3A78-EM motherboard (boot screen is shown)
 http://www.asus.com/Motherboards/M3A78EM/

 fitted with
 AMD Athlon 64 X2 5050e Brisbane 2.60GHz, 1024KB L2 Cache
 2048MB (2GB) 800MHz DDR

 and a custom JVS I/O board providing security etc.
 'CV2000XP Rev 2.0'

 The game is contained on a
 Transcend 2GB 300x UDMA Compact Flash Card
 plugged into an adapter board
 with 'Windows(r) Embedded Standard'

 There don't appear to be any dedicated video / sound boards so it
 presumably uses the onboard capabilities of the board
 'Integrated ATI Radeon(tm) HD 3200 GPU'
 'Realtek(r) ALC1200 8 -Channel High Definition Audio CODEC'

 There should be at least 3 game revisions?

 Pictures of the hardware can be seen at
 http://ikotsu.blogspot.co.uk/2010/03/deathsmiles-ii-pos-arcade-pc.html
 however this revision is using a Gigabyte motherboard instead, possibly
 different boards were used?
 appears to be this one
 http://www.gigabyte.com/products/product-page.aspx?pid=3016#ov
 GA-MA78GPM-UD2H (rev. 1.0)

 The JVS board is said to be quite problematic, and the game will boot
 to an error screen if it isn't functioning correctly.
 http://forum.arcadeotaku.com/viewtopic.php?f=26&t=14850&start=60

*/


#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"


namespace {

class cavepc_state : public driver_device
{
public:
	cavepc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }


	void cavepc(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	void cavepc_io(address_map &map) ATTR_COLD;
	void cavepc_map(address_map &map) ATTR_COLD;
};

/*****************************************************************************/

void cavepc_state::cavepc_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000e0000, 0x000fffff).rom().region("bios", 0xe0000);
	map(0xfff00000, 0xffffffff).rom().region("bios", 0);
}

void cavepc_state::cavepc_io(address_map &map)
{
}

/*****************************************************************************/


static INPUT_PORTS_START(cavepc)
INPUT_PORTS_END


void cavepc_state::cavepc(machine_config &config)
{
	PENTIUM3(config, m_maincpu, 200'000'000); /*  AMD Athlon 64 X2 5050e Brisbane 2.60GHz, 1024KB L2 Cache ! */
	m_maincpu->set_addrmap(AS_PROGRAM, &cavepc_state::cavepc_map);
	m_maincpu->set_addrmap(AS_IO, &cavepc_state::cavepc_io);

	PCI_ROOT(config, "pci", 0);
	// ...
}


/*****************************************************************************/

/*

Deathshmiles II (2009/10/14 MASTER VER 4.00)

CAVE's venture into PC based hardware platforms. The game did not
sell that well and was plagued by BSODs and hardware issues. The
motherboard bios version that shipped out with the game is F2 on
a Gigabyte GA-MA78GPM-UD2H board
( http://www.gigabyte.com/products/product-page.aspx?pid=3016#ov )

The following versions are known to have existed.

1.00 - released 2009/05/14
2.00
3.00 - sometimes scrolls the text "2ND UPDATE MASTER VER 3.00" at
       the bottom of the title screen

The archive contains the following:

./images, documentaiton
./cf_card_2gb, a dd image of the 2GB CF FLASH card
./usb_drive, the game is updated using a USB drive and will not
  start if it is not present
./motherboard manual
./motherboard bios download version F2

*/

ROM_START(deathsm2)
	ROM_REGION32_LE(0x100000, "bios", 0)
	ROM_LOAD( "ma78gu2h.f2",     0x000000, 0x100000, CRC(c85742c4) SHA1(9e2a4b4a2137d1a19bf4cce20a3e2642fc6c6e05) )

	DISK_REGION( "cfcard" )
	DISK_IMAGE( "ds2_4.0", 0,  SHA1(111c2c7a3b987d47f4b6666a8ba9c5d9552b9653) )

	DISK_REGION( "usb" ) // the USB stick used to upgrade the game to Version 4.00 MUST be present for it to run once upgraded
	DISK_IMAGE( "cave_ds2_usb", 0, SHA1(b601985c7f6e6a20b0b7999167b7ccdd12ab80d0) )
ROM_END

} // anonymous namespace


/*****************************************************************************/

GAME(2009, deathsm2, 0, cavepc, cavepc, cavepc_state, empty_init, ROT0, "Cave", "Deathsmiles II: Makai no Merry Christmas (2009/10/14 MASTER VER 4.00)", MACHINE_IS_SKELETON )
