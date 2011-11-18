/* Come On Baby
  (c) 2000 ExPotato Co. Ltd (Excellent Potato)

  There also appears to be a sequel which may be running on the same hardware
  Come On Baby - Ballympic Heroes!  (c) 2001

  This is a Korean PC based board running Windows.  The game runs fully from
  the hard disk making these things rather fragile and prone to damage.

  Very little is known about the actual PC at this time, and the driver is
  just a skeleton placeholder for the CHD dump of the hard disk.

  The donor PC looks like a standard Windows98 setup.
  The only exceptions I see are that there's a game logo.sys/logo.bmp in the
  root directory to hide the Windows98 startup screen, and a shortcut to
  the game in the startup programs.
  Also of interest, Windows98 was installed from a setup folder on the HD.
  To me this hints that there may have been some expectation of the disk
  being plugged into random hardware.

  The game is pretty much standard PC game running on Windows98 PC.
  It uses DirectSound and the Microsoft MCI interfaces and Glide for video.
  The PC that the game was dumped from has Sound Blaster and Ensoniq drivers,
  but it works fine with some other sound configurations.
  The donor PC has a 3dfxvoodoo3 driver installation directory, but it works
  fine with a Voodoo4 4500.

  The game itself has some protection, it expects a file C:\\WINDOWS\win386p.swp of 84 bytes
  to have the hard disk volume serial number in ascii number (not hex) format at offset 4.

  The game appears to use parallel port (0x378,0x379)for I/O.
  The direct port access means it won't run on XP
  For the controls, it writes a device select to 0x378, and reads the device value from 0x379.
  There is some other output, maybe lights?
   --------------------------------------------
  |SELECT|RETURN                               |
  |--------------------------------------------|
  | 0x8  | self test/protection, return 5      |
  | 0x0  | P1 4 way joystick                   |
  |      | ----x--- right                      |
  |      | -----x-- left                       |
  |      | ------x- down                       |
  |      | -------x up                         |
  | 0x1  | P1 buttons                          |
  |      | -----x-- C+D (is also start button) |
  |      | ------x- B                          |
  |      | -------x A                          |
  | 0x2  | P2 joystick (as P1)                 |
  | 0x3  | P2 buttons  (as P1)                 |
  | 0x4  | Coin/Service                        |
  |      | -----x-- Coin                       |
  |      | ------x- Coin                       |
  |      | -------x Test                       |
   --------------------------------------------

  Easy enough to fix a broken game if you have the controls to plug into it.
*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/i386/i386.h"


class comebaby_state : public driver_device
{
public:
	comebaby_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }
	
protected:
	
	// devices
	required_device<cpu_device> m_maincpu;
	
	// driver_device overrides
	virtual void video_start();
	virtual bool screen_update(screen_device &screen, bitmap_t &bitmap, const rectangle &cliprect);
};


void comebaby_state::video_start()
{
}

bool comebaby_state::screen_update(screen_device &screen, bitmap_t &bitmap, const rectangle &cliprect)
{
	return 0;
}

static ADDRESS_MAP_START( comebaby_map, AS_PROGRAM, 32, comebaby_state )
	AM_RANGE(0x00000000, 0x0001ffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( comebaby )
INPUT_PORTS_END


static MACHINE_CONFIG_START( comebaby, comebaby_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PENTIUM, 2000000000) /* Probably a Pentium .. ?? Mhz*/
	MCFG_CPU_PROGRAM_MAP(comebaby_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)

	MCFG_PALETTE_LENGTH(0x100)
MACHINE_CONFIG_END


ROM_START(comebaby)
	ROM_REGION32_LE(0x20000, "maincpu", 0)	/* motherboard bios */
	ROM_LOAD("comeonbaby.pcbios", 0x000000, 0x10000, NO_DUMP )

	DISK_REGION( "disks" )
	DISK_IMAGE( "comebaby", 0, SHA1(ea57919319c0b6a1d4abd7822cff028855bf082f) )
ROM_END


GAME( 2000, comebaby,  0,   comebaby,  comebaby,  0,  ROT0,  "ExPotato",    "Come On Baby",   GAME_NOT_WORKING|GAME_NO_SOUND )
