// license:BSD-3-Clause
// copyright-holders:
/*
    Cosmodog / Namco Rockin' Bowl-o-Rama.

    PC hardware running Linux:
    -Motherboard: Elitegroup P4M800PRO-M478 v1.0.
    -CPU: Celeron, socket 478, 800/533MHz
    -Video: R9600PRO 256M DDR AGP.
    -Boot from USB flash drive.

 Additional I/O board that serves also as protection (labeled "HYdra V2.3"),
 connected via RS-232 (DB9):
     _________
   _| RS-232 |________________________
  | |__DB9___|              ········ |
  |               ______             |
  |              MAX3232C         __ |
  |  _____       |_____|         |o| |
  |  34064                       |o| |
  | ..                  ________ |o| |
  |                   SN74HC125N |o| |
  |        ______                    |
  |       MC908AP8                   |
  |       |_____|                    |
  |                                  |
  |                       ________   |
  |                      SN74HC125N  |
  |      _________________________   |
  |     |::::::::::::::::::::::::|   |
  | _______________________   ____   |
  ||oooooooooooooooooooooo|  |ooo|   |
  |__________________________________|
*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"

namespace {

class rbowlorama_state : public driver_device
{
public:
	rbowlorama_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void rbowlorama(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void rbowlorama_map(address_map &map) ATTR_COLD;
};

void rbowlorama_state::rbowlorama_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000e0000, 0x000fffff).rom().region("bios", 0x60000);
	map(0xfff80000, 0xffffffff).rom().region("bios", 0);
}

static INPUT_PORTS_START( rbowlorama )
INPUT_PORTS_END

void rbowlorama_state::rbowlorama(machine_config &config)
{
	// Basic machine hardware
	PENTIUM4(config, m_maincpu, 120'000'000); // Celeron, socket 478, 800/533MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &rbowlorama_state::rbowlorama_map);

	PCI_ROOT(config, "pci", 0);
	// ...
}

/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( rbowlorama )
	ROM_REGION32_LE(0x80000, "bios", 0)
	ROM_LOAD("p4m8p478.rom", 0x00000, 0x80000, CRC(a43b33c6) SHA1(1633273f9f06862b63aeb10899006b10fab4f4af) )

	// I/O PCB
	ROM_REGION(0x2000, "io", 0)
	ROM_LOAD("mc908ap8.u102", 0x0000, 0x2000, NO_DUMP )

	DISK_REGION( "ide:0:hdd" ) // Sandisk Cruzer USB 8GB flash drive
	DISK_IMAGE( "namco_bowling", 0, BAD_DUMP SHA1(138971fa22ba5a4f0e78daad989d444ebb072213) ) // v2.1.1. May contain operator and players data

/*
  Game Updates

  Versions 0.1.14 and older cannot be updated unless you create a folder on the game directory. Since the game is mounted as
  read only, you need first to remount it as read-write (and remount it back to read only once the update folder is created).
  To do it you must:
   1.- Plug a keyboard to the PC motherboard.
   2.- Once the game is started, press ALT+F1 to enter the system menu. Select "System Tools" -> "Terminal" to launch a terminal.
   3.- Create the folder and exit terminal (su password is b0wl1n6* (lower case 'B', number zero, lower case 'W', lower case 'L', number one, lower case 'N', number six, asterisk):
                sudo mount -o remount -o rw /bowl
                mkdir /bowl/update
                sudo mount -o remount -o ro /bowl
                exit
*/

/*
  Procedure for updating to 2.0.5:
   1.- Create a top level directory on a USB drive called "update", and copy the files "bor-2.0.5.00000.upd" and "bor-2.0.5.pak" into it.
   2.- Open both the coin door and the back door of the game.
   3.- Plug the USB drive on any availabe port on the machine motherboard and press the test button inside the coin door.
   4.- From the test menu select "software update" and then select the current update.
   5.- Read the installation instructions and press START to install the update.
   6.- Once the installation completes the game will quit and restart.
   7.- Remove the USB flash drive and close the coin door and back door.
*/
	ROM_REGION(0x80ad5ea, "update205", 0)
	ROM_LOAD("bor-2.0.5.00000.upd", 0x00000, 0x0000498, CRC(197aa4ee) SHA1(dcd3cfc34613909de85c980e83c8af3e9868b66a))
	ROM_LOAD("bor-2.0.5.pak",       0x00498, 0x80ad152, CRC(c7445c37) SHA1(f5634876302f1d8619e2e3aedfe56edf83e65cfd))

/*
  Procedure for updating to 2.1.1 (if the current version is less than 2.0.5, update first to 2.0.5):
   1.- Copy the file "bor-2.1.1.00000.xpk" into the top level folder of an USB drive.
   2.- Open both the coin door and the back door of the game.
   3.- Plug the USB drive on any availabe port on the machine motherboard and press the test button inside the coin door.
   4.- From the test menu select "software update" and then select the current update.
   5.- Read the installation instructions and press START to install the update.
   6.- Once the installation completes the game will quit and restart.
   7.- Remove the USB flash drive and close the coin door and back door.
*/
	ROM_REGION(0x2fd0000, "update211", 0)
	ROM_LOAD("bor-2.1.1.00000.xpk", 0x00000, 0x2fd0000, CRC(eac46f62) SHA1(3b00baad15ab7662b3d4cd69c4d589f722f2e1f7))


	DISK_REGION( "recovery221" )
	DISK_IMAGE_READONLY( "bor2_2_1", 0, SHA1(2c9341b81e1cda94231fb8173b506178bf163f9c) ) // 2.2.1 recovery ISO image
ROM_END

} // Anonymous namespace

GAME(2008, rbowlorama, 0, rbowlorama, rbowlorama, rbowlorama_state, empty_init, ROT0, "Namco / Cosmodog", "Rockin' Bowl-O-Rama (v2.1.1)", MACHINE_IS_SKELETON )
