// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

    Skeleton driver for "EuroByte Electronics & Multimedia IASA" PC-based touch
    games, sold in Spain by Sleic / Petaco.

    -----+----------------------------+------+----------------------------------
    Dump | Game                       | Year | Notes
    -----+----------------------------+------+----------------------------------
    NO   | Superchip                  | 1999 |
    NO   | Star Touch                 | 2000 |
    YES  | Star Touch / EuroPlay 2001 | 2001 | Original Game: http://www.eurobyte.com.gr/gb_europlay.htm

    Hardware overview:
    MB Soyo M5EH or similar (e.g. Biostar M5ATD)
    16384 KB RAM
    Intel Pentium MMX 233 MHz or compatible (e.g. Cyrix M II-300GP 66MHz Bus 3.5x 2.9V)

    Soyo M5EH uses a VIA Apollo MVP3 chipset with VT82C597 + VT82C586B

    MicroTouch ISA
    ExpertColor Med3931 ISA sound card or other 82C931-based similar card (e.g. BTC 1817DS OPTi ISA)
    PCI VGA ExpertColor M50-02 (S3, Trio64V2/DX 86C775, 512KB RAM)
    Parallel port dongle HASP4
    Creative Video Blaster camera (parallel port)
    HDD Samsung SV0322A or other IDE HDD with similar capacity (e.g. Seagate ST32122A).

    TODO:
    - europl01 boots Windows 3.11 fine in pcipc & shutms11 but will:
    1. "could not find sound card" in C:\wingk\audio\sndinit (which has a poor attempt at
       suppressing the log, should be NUL but it's NULL instead)
    2. Fails loading windows for "video device conflict" (s3vcp64.drv)
    Note that the underlying .ini files will also load a Greek driver keyboard ...

**************************************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"

namespace {

class startouch_state : public driver_device
{
public:
	startouch_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void europl01(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};

void startouch_state::mem_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000e0000, 0x000fffff).rom().region("bios", 0);
	map(0xfffe0000, 0xffffffff).rom().region("bios", 0);
}


static INPUT_PORTS_START(europl01)
INPUT_PORTS_END

void startouch_state::europl01(machine_config &config)
{
	// Super Socket 7
	PENTIUM_MMX(config, m_maincpu, 233'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &startouch_state::mem_map);

	PCI_ROOT(config, "pci", 0);
	// ...
}

ROM_START(europl01)

	// Sleic used different motherboards for this machine. By now, we're adding all the known BIOSes here
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "soyo_5ehm13_1aa1", "Soyo 5EHM (Award BIOS 5EH V1.3-1AA1)")                                                                 // Soyo 5EHM V1.3
	ROMX_LOAD("award_1998_pci-pnp_586_223123413.bin", 0x00000, 0x20000, CRC(d30fe6c2) SHA1(022cf24d982b82e4c13ebbe974adae3a1638d1cd), ROM_BIOS(0)) //   39SF010
	ROM_SYSTEM_BIOS(1, "soyo_5ehm12_1ca2", "Soyo 5EHM (Award BIOS 5EH V1.2-1CA2)")                                                                 // Soyo 5EHM V1.2 (1MB cache)
	ROMX_LOAD("award_1998_pci-pnp_586_222951562.u2",  0x00000, 0x20000, CRC(5bb1bcbc) SHA1(6e2a7b5b3fc892ed20d0b12a1a533231c8953177), ROM_BIOS(1)) //   39SF010
	ROM_SYSTEM_BIOS(2, "bst_m5atd1228b_1", "Biostar M5ATD (Award BIOS ATD1228B, set 1)")                                                           // Biostar M5ATD V1.2 (ALi M5819P + ALi M1543 B1 + ALi M1531 B1 + UMC UM61L6464F-6)
	ROMX_LOAD("award_1998_pci_pnp_586_149197780.u11", 0x00000, 0x20000, CRC(1ec5749b) SHA1(3dd1dac852b00c8108aaf9c89f47ae1922d645f0), ROM_BIOS(2)) //   W29C011
	ROM_SYSTEM_BIOS(3, "bst_m5atd1228b_2", "Biostar M5ATD (Award BIOS ATD1228B, set 2)")                                                           // Biostar M5ATD
	ROMX_LOAD("award_1998_pci-pnp_586_149278871.bin", 0x00000, 0x20000, CRC(3c6aea4d) SHA1(9e56b0f27c204a0eaaf1174070fc95faacc84b0b), ROM_BIOS(3)) //   W29C011

	ROM_REGION(0x20000, "hd_firmware", 0) // Samsung SV0322A
	ROM_LOAD("jk200-35.bin", 0x00000, 0x20000, CRC(601fa709) SHA1(13ded4826a64209faac8bc81708172b81195ab96))

	ROM_REGION(0x66da4, "dongle", 0)
	ROM_LOAD("9b91f19d.hsp", 0x00000, 0x66da4, CRC(0cf78908) SHA1(c596f415accd6b91be85ea8c1b89ea380d0dc6c8))

	DISK_REGION( "ide:0:hdd" ) // Sleic-Petaco Star Touch 2001. Version 2.0. Date 06/April/2001
	DISK_IMAGE("sleic-petaco_startouch_2001_v2.0", 0, SHA1(3164a5786d6b9bb0dd9910b4d27a77a6b746dedf)) // Labeled "Star Touch 2001" but when running, game title is EuroPlay 2001
ROM_END

} // Anonymous namespace

GAME(2001, europl01, 0, europl01, europl01, startouch_state, empty_init, ROT0, "Sleic / Petaco", "EuroPlay 2001", MACHINE_IS_SKELETON)
