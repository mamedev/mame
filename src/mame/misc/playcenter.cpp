// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

    Skeleton driver for "PlayCenter" PC-based touch games

All PlayCenter machines:
    The machines were sold with three different hardware configurations (internally named "Epox",
    "K6" and "Celeron"), each one with its own "official" recovery images (usually Norton Ghost).
      "Epox" version:
         -EPoX EP-MVP4F motherboard (Socket 7 with VIA Apollo MVP4 AGP, built-in Trident AGP,
          VT82C686A, built-in SoundBlaster Pro with AC'97 codec support).
         -128MB RAM PC133, AMD K6-2 processor (K6-2/500(100*5)).
         -PCI Ethernet card (RTL8029AS chipset, PCI).
         -Trident Blade3D/MVP4 AGP video.
         -56K Modem (S56MR, HAMR5603 + Si3014-KS).
         -Elo Touch CTR-231000 touch screen (87C51-based, undumped) or custom touch I/O PCB named
          "Touch Presas" with unknown (and undumped) MCU.
      "K6" version:
         -Unknown AMD K6 CPU based hardware.
      "Celeron" version:
         -Unknown Intel Celeron CPU based hardware.

About "PlayCenter Champions Tournament" (may not apply to other variants) security scheme:
    Each minigame is a VisualBasic executable that receives three numbers when
    invoked, the hard disk serial number (readed through the Windows enumerator
    file), the Ethernet MAC address, and a third one, and concatenates them
    moving some bits around.
    Then, calls the security board via RS-232 (it's a simple PCB with a PIC and
    a serial port), getting another number (the PIC always returns the same
    number, there are no states or logic).
    Finally, the game compares both numbers with a simple formula, refusing to
    work if there's a mismatch.
    Since the security involves hardware unique serial numbers, each PIC program
    is unique for each single machine (and these hardware parts are not
    replaceble by the user / operator).

TODO (about the "Epox" version of "PlayCenter Champions Tournament"):
- In pcipc it shows a '1' logo then on first boot it tries to install basically
  everything it can possibly install from scratch. Once done it tries to boot the
  main program but it either do one of the following things:
  - fails with a '2', resets the machine;
  - fails with a Spanish popup: "Detected a problem with the machine.
    Please call tech service Ref.: ICRP/E-3R" then it shows a '2' and reset;
  - fails with above but instead of resetting will show a '3' and hangs there,
    with a Spanish message about detecting a power failure;
  Notice you can't neither enter in Safe Mode nor install from CD-Rom since there's no
  CD drive installed. Also that the main program will kick in if you try to auto PnP the missing
  devices.
  The ICRP/E-3R error appears if "C:\Play.Center\RPCDR.dat" is newer than 24 hours, it contains
  the number of reboots and if it's >= 3 it shows the message.
  Error '2' appears if Windows is set on anything that isn't 800x600x16bpp.

**************************************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"

namespace {

class playcenter_state : public driver_device
{
public:
	playcenter_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void playcenter(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};

void playcenter_state::mem_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000e0000, 0x000fffff).rom().region("bios", 0x20000);
	map(0xfffc0000, 0xffffffff).rom().region("bios", 0);
}

static INPUT_PORTS_START(playcenter)
INPUT_PORTS_END

void playcenter_state::playcenter(machine_config &config)
{
	PENTIUM(config, m_maincpu, 166'000'000); // Actually an AMD K6, AMD K6-2 or Intel Celeron
	m_maincpu->set_addrmap(AS_PROGRAM, &playcenter_state::mem_map);

	PCI_ROOT(config, "pci", 0);
	// ...
}

// 'Epox' version.
ROM_START(plycntrchtr)
	ROM_REGION32_LE(0x40000, "bios", 0) // BIOS date: 03/13/2001
	ROM_LOAD("vp4f1313.bin", 0x00000, 0x40000, CRC(bd4b155f) SHA1(3eafe71e89bf84b72a42e933187676fe08db0492))

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE("playcenter_epox_9.3_tournament", 0, SHA1(93a73d32c5b12e1e34e691c6a96717b0da709eee)) // Dump contains a raw image for a (c) 1998 Trident video card (C:\videorom.bin)
ROM_END

/* 'K6' version.
   Unknown AMD K6 based PC hardware. */
ROM_START(plycntrchtrk)
	ROM_REGION32_LE(0x40000, "bios", 0)
	ROM_LOAD("bios.bin", 0x00000, 0x40000, NO_DUMP)

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE("playcenter_k6_9.3_tournament", 0, SHA1(7e8935c45c52eb1e28cd3ea18f9f58f8262ffeb3))
ROM_END

/* 'Celeron' version.
   Unknown Intel Celeron based PC hardware. */
ROM_START(plycntrchtrc)
	ROM_REGION32_LE(0x40000, "bios", 0)
	ROM_LOAD("bios.bin", 0x00000, 0x40000, NO_DUMP)

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE("playcenter_celeron_9.3_tournament", 0, SHA1(eb98f6af20a98d4bf6dc7311035431467ad56605))
ROM_END

// 'Epox' version. Windows 98 SE.
ROM_START(plycntre3)
	ROM_REGION32_LE(0x40000, "bios", 0) // BIOS date: 03/13/2001
	ROM_LOAD("vp4f1313.bin", 0x00000, 0x40000, CRC(bd4b155f) SHA1(3eafe71e89bf84b72a42e933187676fe08db0492))

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE("playcenter_e_14_0_t", 0, SHA1(b7c7fce1522e64fc5132f223d8a73555c24002a9)) // From an operator, may contain user data. Contains (C:\videorom.bin) a BIOS for a Trident Blade 3D (PCIR 1023:9880)
ROM_END

} // Anonymous namespace

//   YEAR  NAME          PARENT       MACHINE     INPUT       CLASS             INIT        ROT   COMPANY                             FULLNAME                                                      FLAGS
GAME(2000, plycntrchtr,  0,           playcenter, playcenter, playcenter_state, empty_init, ROT0, "Recreativos Presas / Undergaming", "PlayCenter Champions Tournament (v9.3, 'Epox' hardware)",    MACHINE_IS_SKELETON)
GAME(2000, plycntrchtrk, plycntrchtr, playcenter, playcenter, playcenter_state, empty_init, ROT0, "Recreativos Presas / Undergaming", "PlayCenter Champions Tournament (v9.3, 'K6' hardware)",      MACHINE_IS_SKELETON)
GAME(2000, plycntrchtrc, plycntrchtr, playcenter, playcenter, playcenter_state, empty_init, ROT0, "Recreativos Presas / Undergaming", "PlayCenter Champions Tournament (v9.3, 'Celeron' hardware)", MACHINE_IS_SKELETON)
GAME(2004, plycntre3,    0,           playcenter, playcenter, playcenter_state, empty_init, ROT0, "Recreativos Presas / Undergaming", "Playcenter Evolution III (v14.0, 'Epox' hardware)",          MACHINE_IS_SKELETON) // E.14.0.TCT
