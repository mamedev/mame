// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

Festival: Goldfish Scoop (c) 2008 CAVE/PLABI/SEVENGEARS/cavia

PCIe era with Gigabyte GA-G31M-S2L (rev. 1.1/2.0), connected cards doesn't matter as long as the
CV1000XP/CV2000XP is connected thru USB.

- Intel G31 + ICH7
- IT8718 Super I/O
GA-G31M-S2L/GA-G31M-S2C datasheet reports these two (mutually exclusive?)
- RTL8111C (PCIe Integrated Gigabit Ethernet Controller)
- RTL8102E (PCIe Fast Ethernet NIC)

Notes:
- CF card looks sysprep-ed, can't boot in shutms11.
- Following are drivers found in C:\WINDOWS\INF folder:
\- Several OEM*.inf files, targeting the chipset above;
\- eGalax Touchscreen Controller (VID=0eef, PID=0001/0002)
\- Renesas USB serial ports driver (VID=045b, PID=0020)

===================================================================================================

Technical
=========

The system consists of a standard motherboard and the CV1000XP board.
(ETA: alt setup seen with CV2000XP Rev. 1.0 on it)

The CV1000XP has the following connectors and switches.

IDC connectors.

CN1: goes to the F_PANEL (Front Panel Header) on the motherboard.
     Power switch, Reset switch, IDE LED, Sleep LED
CN2: goes to the F_USB1 header on the motherboard. This supplies
     5V as well. The 5V is always on regardless it is used for the
     soft start.

Molex connector.

CN7: Standard molex, although it is plugged in it has no effect since
     JP3 is in the position where power is taken from the F_USB1 connector.

Switches:

SW1 : Soft power off
SW5 : Reset

Hacking
=======

Game boots up on the HW but cannot be controlled due to lack of
touch screen.

On a standard XP machine the game boots with an error due
to the missing CV1000XP card.

Connecting directly into the CV1000XP CN2 with a standard
USB cable results in the message:  USB device not recognized. Attempts to
increase the current on the 5V have not been succesful.

Make a dd of the image, and write to an SSD disk,
the game will boot happily. Rename cvgame.exe to
goldfish.exe and copy in cmd.exe as cvgame.exe will
get us a command prompt.

PS2 keyboard is working, USB keyboard and mouse have been
disabled in BIOS. BIOS is easily accessible using: DEL

Use portmon to log the serial connection to the CV1000XP (COM3)
then start up the game. This gives a nice log of the communication.

**************************************************************************************************/


#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"


namespace {

class fstgfish_state : public driver_device
{
public:
	fstgfish_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }


	void fstgfish(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};

/*****************************************************************************/

void fstgfish_state::main_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000e0000, 0x000fffff).rom().region("bios", 0x60000);
	map(0xfff80000, 0xffffffff).rom().region("bios", 0);
}

void fstgfish_state::main_io(address_map &map)
{
}

/*****************************************************************************/


static INPUT_PORTS_START(fstgfish)
INPUT_PORTS_END


void fstgfish_state::fstgfish(machine_config &config)
{
	PENTIUM3(config, m_maincpu, 200'000'000); /*  Intel Core 2 Duo, 333/266/200 FSB clocks */
	m_maincpu->set_addrmap(AS_PROGRAM, &fstgfish_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &fstgfish_state::main_io);

	PCI_ROOT(config, "pci", 0);
	// ...
}

ROM_START(fstgfish)
	ROM_REGION32_LE(0x80000, "bios", 0)
	ROM_LOAD( "g31ms2l.f7",     0x000000, 0x80000, CRC(4f92f678) SHA1(c29ce14f093b5d6ef308f674fe4d514eb10e4742) )

	DISK_REGION( "cfcard" )
	// CF with Windows XP embedded
	DISK_IMAGE( "x11_15305", 0, SHA1(67bce99fb55760d0c06d698e68656eebbda8a28f) )

	// H8S/2218 on CV2000XP board, assume with internal ROM
	ROM_REGION(0x20000, "cv2000xp", ROMREGION_ERASEFF)
	ROM_LOAD( "h8s2218.bin",     0x000000, 0x20000, NO_DUMP )
ROM_END

} // anonymous namespace


/*****************************************************************************/

GAME(2008, fstgfish, 0, fstgfish, fstgfish, fstgfish_state, empty_init, ROT0, "Cave", "Festival: Goldfish Scoop", MACHINE_IS_SKELETON )
