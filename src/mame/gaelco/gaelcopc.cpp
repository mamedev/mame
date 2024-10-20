// license:BSD-3-Clause
// copyright-holders: David Haywood

/* Gaelco PC based hardware

TODO:
- Implement all of the components listed below, notice that we need to consolidate
  PCI individual interfaces before even attempting to emulate this;
- Analyze OS images;
- Reportedly requires a special monitor with a very specific refresh rate (very Gaelco like),
  likely gonna play with monitor DDC;

Custom motherboard with
Intel 82815 (host + bridge + AGP devices)
Intel 82801 (PCI 2.3 + integrated LAN + IDE + USB 2.0 + AC'97 + LPC +
             ACPI 2.0 + Flash BIOS control + SMBus + GPIO)
Intel 82562 (LAN)
RTM 560-25R (Audio)
nVidia GeForce 4 TI4200 128Mb AGP
256 Mb PC133
Pentium 4 (??? XXXXMhz), <- contradicts 82815 datasheet and an internal BIOS string at $ce (Socket 370),
                            expect Celeron or Pentium 3 at very least.

I/O Board with Altera Flex EPF15K50EQC240-3

The graphics cards are swappable between nVidia cards from
the era. There is no protection on the games, you can just swap out
hard drives to change games, though they do seem to have their own
motherboard bioses.

*/


#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"


namespace {

class gaelcopc_state : public driver_device
{
public:
	gaelcopc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	void gaelcopc(machine_config &config);

private:
	void gaelcopc_map(address_map &map) ATTR_COLD;

	// devices
	required_device<pentium3_device> m_maincpu;
};


void gaelcopc_state::gaelcopc_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000e0000, 0x000fffff).rom().region("bios", 0x60000);
	map(0xfff80000, 0xffffffff).rom().region("bios", 0);
}

static INPUT_PORTS_START( gaelcopc )
INPUT_PORTS_END


void gaelcopc_state::gaelcopc(machine_config &config)
{
	// basic machine hardware
	// BIOS mentions Socket 370, so at very least a Celeron or a Pentium 3 class CPU
	// TODO: lowered rate for debugging aid, needs a slot option anyway
	PENTIUM3(config, m_maincpu, 100'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &gaelcopc_state::gaelcopc_map); // TODO: remove me

	PCI_ROOT(config, "pci", 0);
	// ...
}

// TODO: All of the provided BIOSes just have different ACFG table configs at $10000, investigate

ROM_START(tokyocop)
	ROM_REGION32_LE(0x80000, "bios", 0)  // motherboard BIOS
	// $40 Award 6.00 PG 12/19/2002
	// $7413c Intel 815 Hardware version v0.0 08/13/1999
	ROM_SYSTEM_BIOS(0, "default", "v0.0 08/13/1999")
	ROMX_LOAD("al1.u10", 0x000000, 0x80000, CRC(e426e030) SHA1(52bdb6d46c12150077169ac3add8c450326ad4af), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "alt_acfg", "v0.0 08/13/1999 (alt ACFG)")
	ROMX_LOAD("al1 49lf004a.u10", 0x00000, 0x80000, CRC(db8e4a37) SHA1(9ef4bf1d72955a89e7dc9b984e76ce86c824b461), ROM_BIOS(1) )

/* Dumper's note: The drive was ordered from Gaelco and they used a 250 GB drive that apparently used to have something
else on it because when I ripped the entire drive and compressed it, the compressed image was 30 GB which is too much for me
to upload. So I just ripped the partitions (it had 3) and the size was reasonable. This rip was burned into another drive and
tested working on the real hardware. It uses the same hardware and bios as the kit version.*/

	DISK_REGION( "disks" )
	DISK_IMAGE( "tokyocop", 0, SHA1(f3b60046da7094743822191473e05ee9cbc1af86) )
ROM_END

ROM_START(tokyocopk)
	ROM_REGION32_LE(0x80000, "bios", 0)  // motherboard BIOS
	// $40 6.00 PG 12/19/2002
	// $7413c v0.0 08/13/1999
	ROM_LOAD("al1.u10", 0x000000, 0x80000, CRC(e426e030) SHA1(52bdb6d46c12150077169ac3add8c450326ad4af) )

	DISK_REGION( "disks" ) // Maxtor 2F040J0310613 VAM051JJ0
	DISK_IMAGE( "tokyocopk", 0, SHA1(3805e41903719d8ed163f9879db65e71aba2e3e7) )
ROM_END

ROM_START(tokyocopi)
	ROM_REGION32_LE(0x80000, "bios", 0)  // motherboard BIOS
	// Not specifically provided, assume "works right" with same BIOS.
	ROM_LOAD("al1.u10", 0x000000, 0x80000, CRC(e426e030) SHA1(52bdb6d46c12150077169ac3add8c450326ad4af) )

	DISK_REGION( "disks" )
	DISK_IMAGE( "tokyocopi", 0, SHA1(a3cf011c8ef8ec80724c28e1534191b40ae8515d) )
ROM_END

ROM_START(rriders)
	ROM_REGION32_LE(0x80000, "bios", 0)  // motherboard BIOS
	// $40 6.00 PG 12/19/2002
	// $7413c v0.0 08/13/1999
	ROM_LOAD("22-03.u10", 0x000000, 0x80000, CRC(0ccae12f) SHA1(a8878fa73d5a4f5e9b6e3f35994fddea08cd3c2d) )

	DISK_REGION( "disks" ) // 250 MB compact flash card
	DISK_IMAGE( "rriders", 0, SHA1(46e10517ee1b383e03c88cac67a54318c227e3e1) )
ROM_END

// Gaelco I/O card "REF. 050210" (Altera Cyclone EP1C3T144C8, AD9288...)
ROM_START(tuningrc)
	ROM_REGION32_LE(0x80000, "bios", 0)  // motherboard BIOS
	// $40 6.00 PG 12/19/2002
	// $7413c v0.0 08/13/1999
	ROM_LOAD("310j.u10", 0x000000, 0x80000, CRC(70b0797a) SHA1(696f602e83359d5d36798d4d2962ee85171e3622) )

	DISK_REGION( "disks" ) // Hitachi HDS728080PLAT20 ATA/IDE
	DISK_IMAGE( "tuningrc", 0, SHA1(4055cdc0b0c0e99252b90fbfafc48b693b144d67) )
ROM_END

} // anonymous namespace


GAME( 2003, tokyocop,  0,        gaelcopc, gaelcopc, gaelcopc_state, empty_init, ROT0, "Gaelco", "Tokyo Cop (US, dedicated version)",   MACHINE_IS_SKELETON )
GAME( 2003, tokyocopk, tokyocop, gaelcopc, gaelcopc, gaelcopc_state, empty_init, ROT0, "Gaelco", "Tokyo Cop (US, kit version)",         MACHINE_IS_SKELETON )
GAME( 2003, tokyocopi, tokyocop, gaelcopc, gaelcopc, gaelcopc_state, empty_init, ROT0, "Gaelco", "Tokyo Cop (Italy)",                   MACHINE_IS_SKELETON )
GAME( 2004, rriders,   0,        gaelcopc, gaelcopc, gaelcopc_state, empty_init, ROT0, "Gaelco", "Ring Riders (Software version v2.2)", MACHINE_IS_SKELETON )
GAME( 2005, tuningrc,  0,        gaelcopc, gaelcopc, gaelcopc_state, empty_init, ROT0, "Gaelco", "Gaelco Championship Tuning Race",     MACHINE_IS_SKELETON )
