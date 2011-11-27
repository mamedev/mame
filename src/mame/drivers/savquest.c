/***************************************************************************

    savquest.c

    "Savage Quest" (c) 1999 Interactive Light, developed by Angel Studios.
    Skeleton by R. Belmont

    H/W is a white-box PC consisting of:
    Pentium II 450 CPU
    DFI P2XBL motherboard (i440BX chipset)
    128 MB RAM
    Guillemot Maxi Gamer 3D2 Voodoo II
    Sound Blaster AWE64

    Protected by a HASP brand parallel port dongle.
    I/O board has a PIC17C43 which is not readable.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/i386/i386.h"
#include "memconv.h"
#include "devconv.h"
#include "machine/8237dma.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/mc146818.h"
#include "machine/pcshare.h"
#include "machine/pci.h"
#include "machine/8042kbdc.h"
#include "machine/pckeybrd.h"
#include "machine/idectrl.h"


class savquest_state : public driver_device
{
public:
	savquest_state(const machine_config &mconfig, device_type type, const char *tag)
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


void savquest_state::video_start()
{
}

bool savquest_state::screen_update(screen_device &screen, bitmap_t &bitmap, const rectangle &cliprect)
{
	return 0;
}

static ADDRESS_MAP_START(savquest_map, AS_PROGRAM, 32, savquest_state)
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM
	AM_RANGE(0xfffc0000, 0xffffffff) AM_ROM AM_REGION("user1", 0)	/* System BIOS */
ADDRESS_MAP_END

static ADDRESS_MAP_START(savquest_io, AS_IO, 32, savquest_state)
ADDRESS_MAP_END

static INPUT_PORTS_START( savquest )
INPUT_PORTS_END

static MACHINE_CONFIG_START( savquest, savquest_state )
	MCFG_CPU_ADD("maincpu", PENTIUM, 450000000)	// actually Pentium II 450
	MCFG_CPU_PROGRAM_MAP(savquest_map)
	MCFG_CPU_IO_MAP(savquest_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 199)
	MCFG_PALETTE_LENGTH(16)
MACHINE_CONFIG_END

ROM_START( savquest )
	ROM_REGION32_LE(0x40000, "user1", 0)
	ROM_LOAD( "sq-aflash.bin", 0x000000, 0x040000, CRC(0b4f406f) SHA1(4003b0e6d46dcb47012acc118837f0f7cf529faf) )

	DISK_REGION( "disk" )
	DISK_IMAGE( "savquest", 0, SHA1(b20cacf45e093b533c538bf4fc08f05f9475d640) )
ROM_END


GAME(1999, savquest, 0, savquest, savquest, 0, ROT0, "Interactive Light", "Savage Quest", GAME_NO_SOUND|GAME_NOT_WORKING)
