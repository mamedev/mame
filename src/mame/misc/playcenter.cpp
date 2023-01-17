// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

    Skeleton driver for "PlayCenter" PC-based touch games

    Hardware overview for "PlayCenter Champions Tournament":
    Motherboard: EPoX EP-MVP4F
    Unknown RAM size, AMD K6 processor (unknown speed)
    PCI Ethernet card (unknown chipset)

    The security scheme is quite simple:
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

Dump contains a raw image for a (c) 1998 Trident video card (C:\videorom.bin)
The EPoX EP-MVP4F is a Socket 7 MB with VIA Apollo MVP4 AGP, built-in Trident AGP,
VT82C686A, built-in SoundBlaster Pro with AC'97 codec support

TODO:
- HDD image doesn't boot in neither shutms11 nor pcipc. The dump contains a
  Spanish windows partition (named "u") and no autoexec.bat / config.sys.
  The CHD has -chs 3644,16,58 but WinImage reports back a ~4 GB partition.
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
	void mem_map(address_map &map);

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
	PENTIUM(config, m_maincpu, 166'000'000); // Actually an AMD K6, frequency unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &playcenter_state::mem_map);

	PCI_ROOT(config, "pci", 0);
	// ...
}

ROM_START(plycntrchtr)
	ROM_REGION32_LE(0x40000, "bios", 0) // Bios date: 03/13/2001
	ROM_LOAD("vp4f1313.bin", 0x00000, 0x40000, CRC(bd4b155f) SHA1(3eafe71e89bf84b72a42e933187676fe08db0492))

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE("playcenter_epox_9.3_tournament", 0, BAD_DUMP SHA1(64a88d4ab10d82ba0bd175511242ba6771cfc5ce))
ROM_END

} // Anonymous namespace

GAME(2000, plycntrchtr, 0, playcenter, playcenter, playcenter_state, empty_init, ROT0, "Recreativos Presas / Undergaming", "PlayCenter Champions Tournament (v9.3)", MACHINE_IS_SKELETON)
