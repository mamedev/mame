// license:BSD-3-Clause
// copyright-holders:
/*
    Skeleton driver for Raw Thrills PC-based games.
    Common base configuration:
    - Dell Optiplex 740 (Athlon 64 X2, 2GB DDR3 RAM).
      * Other supported setups are:
        · Dell Optiplex 580.
        · Dell Optiplex 380.
        · Dell Optiplex 390.
        · Dell Optiplex 580.
        · Microtel w/ASRock N68C-GS FX AM3+ motherboard.
    - Video GeForce GT730 (GF108).
      * Other supported setups are:
        · nVidia 8400GS (256MB+).
        · nVidia 7300GS.
    -Custom I/O boards (outside the PC, depending on each game).
    -Security dongle (HASP, USB or parallel port).

TODO:
- Cannot continue without a proper Athlon 64 X2 core (uses lots of unsupported RDMSR / WRMSR)

*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"

namespace {

class rawthrillspc_state : public driver_device
{
public:
	rawthrillspc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void rawthrillspc(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void rawthrillspc_map(address_map &map) ATTR_COLD;
};


void rawthrillspc_state::rawthrillspc_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000e0000, 0x000fffff).rom().region("bios", 0);
	map(0xfffe0000, 0xffffffff).rom().region("bios", 0);
}

static INPUT_PORTS_START( rawthrillspc )
INPUT_PORTS_END


void rawthrillspc_state::rawthrillspc(machine_config &config)
{
	// Basic machine hardware
	PENTIUM4(config, m_maincpu, 120'000'000); // Actually an Athlon 64 X2
	m_maincpu->set_addrmap(AS_PROGRAM, &rawthrillspc_state::rawthrillspc_map);

	PCI_ROOT(config, "pci", 0);
	// ...
}

/***************************************************************************

  Game drivers

***************************************************************************/

#define OPTIPLEX740_BIOS \
	ROM_REGION32_LE( 0x20000, "bios", 0 ) \
	ROM_SYSTEM_BIOS( 0, "122", "v1.2.2" ) \
	ROMX_LOAD( "1.2.2_4m.bin", 0x00000, 0x20000, CRC(43d5b4c8) SHA1(6307050961da5d647ca2fa787fd67c5ac9c690c9), ROM_BIOS(0) ) \
	ROM_SYSTEM_BIOS( 1, "104", "v1.0.3" ) \
	ROMX_LOAD( "1.0.4_4m.bin", 0x00000, 0x20000, CRC(73f0420b) SHA1(4821d21d2c75084062cb1047eb08b1b3ab2424e1), ROM_BIOS(1) )

ROM_START( guitarheroac )
	OPTIPLEX740_BIOS

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE( "slax105", 0, NO_DUMP )

	// Recovery DVD
	DISK_REGION( "recovery105" )
	DISK_IMAGE_READONLY( "slax_restore_dvd_ver.1.0.5", 0, SHA1(c063c0032bf88e4ec7a3b973323e9c84a231079a) )
ROM_END

/*
 Two I/O boards on "The Fast and The Furious":
   1. With Xilinx XC95144XL (labeled "FAST & FURIOUS U4 REV 1.0 (c)2004 RightHand Tech, Inc"),
       ST72F63BK4M1 (labeled "U6 FAST&FURIOUS Release 3 3311h (c)2004 RightHand Tech, Inc") and a bank of 8 dipswitches.
   2. With Xilinx XC9536XL (labeled "r1.0 (c)2004 RightHand Tech, Inc")
 Parallel port HASP4 1.5 dongle (MCU Marvin2)
*/
ROM_START( fnf )
	OPTIPLEX740_BIOS

	DISK_REGION( "ide:0:hdd" )
	/* Clean image created from the recovery CDs on the original machine.
	   After installing the software from the discs, the PC reboots several times for configurating
	   the hardware devices and peripherals, and then asks for controllers calibration.
	   The image is just up to this point, before performing any calibration. On the first boot from
	   this image, you'll be asked for the calibration, and after it, the game is ready for playing. */
	DISK_IMAGE( "faf306", 0, SHA1(2aefe396a79e3328f58ae5e4ccda0041af1b4a1a) )

	// Two recovery CDs, you need both for a full restore

	DISK_REGION( "recovery306d1" )
	DISK_IMAGE_READONLY( "faf3.06d1", 0, SHA1(681ab1258349e5ceb690606e6697e5b957016446) )

	DISK_REGION( "recovery306d2" )
	DISK_IMAGE_READONLY( "faf3.06d2", 0, SHA1(183664482f6665adffc74d69e28338da740443c5) )
ROM_END

/*
 Doodle Jump Arcade, by default, uses a different PC than other Raw Thrills games:
   -HP/Compaq Presario VS459AA-ABE CQ5211ES with an Asus M2N68-LA "Narra 3" motherboard (Socket AM3, nForce-based, nVidia MCP61P chipset)
     * Fintek F8000 + Realtek RTL8201EL + Realtek ALC662 + nVidia NF-6100-430-N-A3 + ST L6740L.
   -CPU Athlon II CPU (ADX2400CK23GQ).
   -2GB RAM (single SIMM, PC3-12800) [HP P/N 655409-150].
   -nVidia Club3D CGNX-G942LI.
 The game runs over Linux CentOS.
 I/O board with ICE P/N X2034X, silkscreened as "500-00040-01" and with a CPLD labeled as "RIO v0x5016 Copyright 2010 Raw Thrills Inc".
 HASP USB security dongle.
*/
ROM_START( doodlejmp )
	ROM_REGION32_LE( 0x100000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "0515", "Compaq 5.15 (11/06/2009)" )
	ROMX_LOAD( "w25x80a.bin", 0x000000, 0x100000, CRC(e91538ee) SHA1(32add79eba2049205a98fc4e854976e11d102a4c), ROM_BIOS(0) )

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE( "doodlejmp", 0, NO_DUMP )

	/* Recovery DVD:
	      -Doodle Jump 1.16
	      -OS 00.07
	      -8-Feb-2013 */
	DISK_REGION( "recovery116" )
	DISK_IMAGE_READONLY( "doodlejump_recover_dvd_1_16", 0, SHA1(67f2bc3d9d71fc924f8f784e62eaf3dd39c88f45) )
ROM_END

} // Anonymous namespace

GAME(2013, doodlejmp,    0, rawthrillspc, rawthrillspc, rawthrillspc_state, empty_init, ROT0, "ICE / Raw Thrills (Lima Sky license)",      "Doodle Jump Arcade (v1.16)",       MACHINE_IS_SKELETON)
GAME(2004, fnf,          0, rawthrillspc, rawthrillspc, rawthrillspc_state, empty_init, ROT0, "Raw Thrills",                               "The Fast And The Furious (v3.06)", MACHINE_IS_SKELETON)
GAME(2008, guitarheroac, 0, rawthrillspc, rawthrillspc, rawthrillspc_state, empty_init, ROT0, "Raw Thrills (Activision / Konami license)", "Guitar Hero Arcade (v1.0.5)",      MACHINE_IS_SKELETON)
