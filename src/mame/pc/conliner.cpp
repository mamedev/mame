// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

 Skeleton driver for Comatel "Onliner" PC-based touch games machines.
 The machines need an online server to work, they won't boot up the games without it.
 More info: https://www.recreativas.org/onliner-6342-comatel

 -Gigabyte GA-7VKMP Rev 3.4 motherboard (VIA KM266, VIA T8235, IT8705F, RTL8100BL, ALC650, etc.).
 -256MB RAM PC2700 DDR
 -AMD Athlon AXDA1800DLT3C processor.
 -ATI Rage 128 Pro 32MB AGP video.
 -Elo Touch touch screen.
 -Logitech QuickCam.
 -Custom I/O board for coin management, inputs, etc.


 I/O board layout:

            Jack         DB37 female (I/O)
             __   _____________________________
           __| |__|                            |____
          |  |_|  |____________________________|    |
          |                                       __|
          | __        ______   ______             :||
          || |       ULN2003A ULN2003A            :|<- Reset
 TDA15520->| |    _______  _______  _______  __     |
          ||_|   |_HC574| |_HC574| |_HC245|  PC817  |
          |                _______  _______  __   __|
          |__             |_HC245| |MAX233| 24LC16B |
          ||:                ______________       :||
          ||:<-Power        |__PIC16F873__|       :|<- COM RS-232
          |__   __________   __________         o o <- RX/TX LEDs
    Power->|:  | ::::::::|  | ::::::::|    Xtal     |
          |______________________________ 4.0 MHz __|
              Credits Mgmt.  Expansion Buses

The I/O board is connected to the internal COMB port of the Gigabyte motherboard using
the COM RS-232 port of the I/O PCB.

**************************************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"

namespace {

class conliner_state : public driver_device
{
public:
	conliner_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void conliner(machine_config &config);

private:
	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
};

void conliner_state::mem_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000e0000, 0x000fffff).rom().region("bios", 0x20000);
	map(0xfffc0000, 0xffffffff).rom().region("bios", 0);
}

static INPUT_PORTS_START(conliner)
INPUT_PORTS_END

void conliner_state::conliner(machine_config &config)
{
	PENTIUM(config, m_maincpu, 166'000'000); // Actually an AMD Athlon AXDA1800DLT3C
	m_maincpu->set_addrmap(AS_PROGRAM, &conliner_state::mem_map);

	PCI_ROOT(config, "pci", 0);
	// ...
}

ROM_START(onlinertp)
	ROM_REGION32_LE(0x40000, "bios", 0)
	ROM_LOAD("w49f002u.u10",  0x00000, 0x40000, CRC(b1f90793) SHA1(5833b134cd6b88260e98d5901d96197705cf5e94))

	DISK_REGION( "ide:0:hdd" ) // Seagate ST340014A
	DISK_IMAGE("comatel_onliner_v4.222.493_u4.44.450", 0, SHA1(ece4b51196b0196be83c20696a49fb6b4d720b65)) // Dump contains users and operator data

	// I/O board
	ROM_REGION(0x4000, "io", 0)
	ROM_LOAD("pic16f873.ic1", 0x00000, 0x04000, NO_DUMP) // Protected
	ROM_LOAD("24lc16b.ic4",   0x00000, 0x00800, CRC(78afb692) SHA1(ad511fc3403dbf693ede6413d22d45302776ac85))

	// VGA BIOS
	ROM_REGION(0x10000, "vga", 0)
	ROM_LOAD("ht27c512.u5",   0x00000, 0x10000, CRC(6b0e2837) SHA1(fca2609c12b595df7c4f8e2b38b3cb089983e523)) // ATI RAGE 128 PRO AGP
ROM_END

} // Anonymous namespace

//   YEAR  NAME       PARENT  MACHINE   INPUT     CLASS           INIT        ROT   COMPANY                    FULLNAME               FLAGS
GAME(200?, onlinertp, 0,      conliner, conliner, conliner_state, empty_init, ROT0, "Comatel / Atata Systems", "Onliner Touch Party", MACHINE_IS_SKELETON) // v4.222.493, with v4.44.450 update
