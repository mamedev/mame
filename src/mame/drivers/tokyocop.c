/* Tokyo Cop

Italian Version

PC based hardware
Custom motherboard with
82815
82801
82562 (LAN)
RTM 560-25R (Audio)
TI4200 128Mb AGP
256 Mb PC133
Pentium 4 (??? XXXXMhz)

I/O Board with Altera Flex EPF15K50EQC240-3

*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/i386/i386.h"


class tokyocop_state : public driver_device
{
public:
	tokyocop_state(const machine_config &mconfig, device_type type, const char *tag)
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


void tokyocop_state::video_start()
{
}

bool tokyocop_state::screen_update(screen_device &screen, bitmap_t &bitmap, const rectangle &cliprect)
{
	return 0;
}

static ADDRESS_MAP_START( tokyocop_map, AS_PROGRAM, 32, tokyocop_state )
	AM_RANGE(0x00000000, 0x0001ffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( tokyocop )
INPUT_PORTS_END


static MACHINE_CONFIG_START( tokyocop, tokyocop_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PENTIUM, 2000000000) /* Pentium4? */
	MCFG_CPU_PROGRAM_MAP(tokyocop_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)

	MCFG_PALETTE_LENGTH(0x100)
MACHINE_CONFIG_END


ROM_START(tokyocop)
	ROM_REGION32_LE(0x20000, "maincpu", 0)	/* motherboard bios */
	ROM_LOAD("tokyocop.pcbios", 0x000000, 0x10000, NO_DUMP )

	DISK_REGION( "disks" )
	DISK_IMAGE( "tokyocop", 0, SHA1(a3cf011c8ef8ec80724c28e1534191b40ae8515d) )
ROM_END


GAME( 2003, tokyocop,  0,   tokyocop, tokyocop, 0, ROT0, "Gaelco", "Tokyo Cop (Italy)", GAME_IS_SKELETON )
