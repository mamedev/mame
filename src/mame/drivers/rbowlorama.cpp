// license:BSD-3-Clause
// copyright-holders:
/*
    Cosmodog / Namco Rockin' Bowl-o-Rama.

    PC hardware running Linux:
    -Motherboard: Elitegroup P4M800PRO-M478 v1.0.
    -CPU: Celeron, socket 478, 800/533MHz
    -Video: R9600PRO 256M DDR AGP.
    -Boot from USB flash drive.
*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "screen.h"

namespace {

class rbowlorama_state : public driver_device
{
public:
	rbowlorama_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void rbowlorama(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void rbowlorama_map(address_map &map);
};

void rbowlorama_state::video_start()
{
}

uint32_t rbowlorama_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void rbowlorama_state::rbowlorama_map(address_map &map)
{
}

static INPUT_PORTS_START( rbowlorama )
INPUT_PORTS_END


void rbowlorama_state::machine_start()
{
}

void rbowlorama_state::machine_reset()
{
}

void rbowlorama_state::rbowlorama(machine_config &config)
{
	// Basic machine hardware
	PENTIUM4(config, m_maincpu, 120000000); // Celeron, socket 478, 800/533MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &rbowlorama_state::rbowlorama_map);

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(800, 600); // Guess
	screen.set_visarea(0, 800-1, 0, 600-1);
	screen.set_screen_update(FUNC(rbowlorama_state::screen_update));
}

/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( rbowlorama )
	ROM_REGION(0x80000, "bios", 0)
	ROM_LOAD("p4m8p478.rom", 0x00000, 0x80000, CRC(a43b33c6) SHA1(1633273f9f06862b63aeb10899006b10fab4f4af) )

	DISK_REGION( "ide:0:hdd:image" ) // Sandisk Cruzer USB 8GB flash drive
	DISK_IMAGE( "namco_bowling", 0, BAD_DUMP SHA1(138971fa22ba5a4f0e78daad989d444ebb072213) ) // v2.1.1. May contain operator and players data

/*
PROCEDURE FOR UPDATING GAMES

Determining the Need for This Update

Take these steps to determine if your game needs this update:

    1. open the coin door and press TEST
    2. from the test menu, select "system info"
    3. if the software revision (the second line) is less than 2.1.1 then this update is necessary

Update Process

The process can be broken into three steps (step I is only necessary for machines with version 0.1.14 code):

    I. create the update directory (skip this step if current version is later than 0.1.14)

    Very early Rockin' Bowl-O-Rama games (version 0.1.14) have a problem which prevents the software update process from working correctly. The following step corrects the problem so that future updates are simpler and work correctly.
       1. Turn the game off
       2. Open the back door
       3. Plug a PS/2 keyboard into the keyboard (purple) connector on the game's motherboard
       4. Turn the game on
       5. After the game starts up, press "ALT-F1" on the keyboard (press F1 while holding the ALT key)
       6. A pulldown menu should appear. Use the down arrow to move down to "System Tools"
       7. Use the right arrow to move into the "System Tools" submenu
       8. Use the down arrow to move down to "Terminal"
       9  Press "Enter" to launch a terminal
      10. In the terminal, type the following commands:
                sudo mount -o remount -o rw /bowl
                mkdir /bowl/update
                sudo mount -o remount -o ro /bowl
                exit
          After entering the first command the machine will prompt for a password. 
          Enter this password: b0wl1n6* (lower case 'B', number zero, lower case 'W', lower case 'L', number one, lower case 'N', number six, asterisk).
          Watch carefully as you type. Since the game is consuming most of the machine's processing power it tends to misread or double-read the keyboard.
      11. Unplug the keyboard.

   II. create the update USB flash drive
       The update is applied to the machine using a USB flash drive. Be sure the flash drive is large enough and has sufficient free space.
       The flash drive can be set up using a PC or a Mac. If your current software is older than 2.0.5 then you must apply both the 2.0.5 update then the current update.
       First follow these steps:
         1. Download the update: bor-2.0.5.zip
         2. Unzip the update by double clicking on it. Your browser may have done this automatically. The update contains two files, bor-2.0.5.00000.upd and bor-2.0.5.pak.
         3. Plug the USB flash drive into a USB socket on your computer. It should mount automatically.
         4. Create a directory called "update" (all lower case) at the top level of the USB flash drive
         5. Copy the two update files into this directory on the USB flash drive
         6. Unmount (eject) the USB flash drive, then unplug it

       Follow these steps to get the current update:
         1. Download the update: bor-2.1.1.00000.xpk
         2. Plug the USB flash drive into a USB socket on your computer. It should mount automatically.
         3. Copy the file anywhere onto the USB flash drive. Be sure the file name hasn't been modified by your computer. It should be bor-2.1.1.00000.xpk.
         4. Unmount (eject) the USB flash drive, then unplug it

  III. apply the update
       This step reads the update from the USB flash drive and installs it into the game. 
       When it is complete the game will quit, leaving a blank screen for a few seconds, then it will restart. Once it has restarted it will be running the updated version of the game.
         1. if it's not already on, turn the game on and wait for it to start running
         2. open the coin door and the back door of the game
         3. plug the USB flash drive into any available USB socket on the motherboard
         4. press the TEST button inside the coin door
         5. from the test menu select "software update"
         6. from the software update menu select the current update (if your current version is less than 2.0.5 select the 2.0.5 update first, then repeat this process for the current update)
         7. read the installation instructions
         8. press START to install the update
         9. once the installation completes the game will quit and restart
        10. remove the USB flash drive and close the coin door and back door
*/
	ROM_REGION(0xb07d5ea, "update211", 0)
	ROM_LOAD("bor-2.0.5.00000.upd", 0x00000, 0x0000498, CRC(197aa4ee) SHA1(dcd3cfc34613909de85c980e83c8af3e9868b66a))
	ROM_LOAD("bor-2.0.5.pak",       0x00000, 0x80ad152, CRC(c7445c37) SHA1(f5634876302f1d8619e2e3aedfe56edf83e65cfd))
	ROM_LOAD("bor-2.1.1.00000.xpk", 0x00000, 0x2fd0000, CRC(eac46f62) SHA1(3b00baad15ab7662b3d4cd69c4d589f722f2e1f7))

	DISK_REGION( "recovery211" )
	DISK_IMAGE_READONLY( "bor2_2_1", 0, SHA1(2c9341b81e1cda94231fb8173b506178bf163f9c) ) // 2.1.1 recovery ISO image
ROM_END

} // Anonymous namespace

GAME(2008, rbowlorama, 0, rbowlorama, rbowlorama, rbowlorama_state, empty_init, ROT0, "Namco / Cosmodog", "Rockin' Bowl-O-Rama (v2.1.1)", MACHINE_IS_SKELETON)
