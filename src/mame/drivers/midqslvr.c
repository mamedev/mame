/***************************************************************************

    Midway Quicksilver skeleton driver

    Main CPU : Intel Celeron 333/366MHz
    Motherboard : Intel SE440BX-2
    RAM : 64MB PC100-222-620 non-ecc
    Sound: Integrated YMF740G
    Networking: SMC EZ Card 10 / SMC1208T (probably 10ec:8029 1113:1208)
    Graphics Chips : Quantum Obsidian 3DFX
    Storage : Hard Drive

    Chipsets (440BX AGPset):
    - 82371EB PCI-ISA bridge
    - 82371EB Power Management Controller
    - 82371AB/EB Universal Host Controller (USB UHCI)
    - 82371AB/EB PCI Bus Master IDE Controller

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


class midqslvr_state : public driver_device
{
public:
	midqslvr_state(const machine_config &mconfig, device_type type, const char *tag)
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


void midqslvr_state::video_start()
{
}

bool midqslvr_state::screen_update(screen_device &screen, bitmap_t &bitmap, const rectangle &cliprect)
{
	return 0;
}

static ADDRESS_MAP_START(midqslvr_map, AS_PROGRAM, 32, midqslvr_state)
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM
	AM_RANGE(0xfff80000, 0xffffffff) AM_ROM AM_REGION("user1", 0)	/* System BIOS */
ADDRESS_MAP_END

static ADDRESS_MAP_START(midqslvr_io, AS_IO, 32, midqslvr_state)
ADDRESS_MAP_END

static INPUT_PORTS_START( midqslvr )
INPUT_PORTS_END

static MACHINE_CONFIG_START( midqslvr, midqslvr_state )
	MCFG_CPU_ADD("maincpu", PENTIUM, 333000000)	// actually Celeron 333
	MCFG_CPU_PROGRAM_MAP(midqslvr_map)
	MCFG_CPU_IO_MAP(midqslvr_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 199)
	MCFG_PALETTE_LENGTH(16)
MACHINE_CONFIG_END


ROM_START( offrthnd )
	ROM_REGION32_LE(0x80000, "user1", 0)
	ROM_LOAD( "lh28f004sct.u8b1", 0x000000, 0x080000, CRC(ab04a343) SHA1(ba77933400fe470f45ab187bc0d315922caadb12) )

	DISK_REGION( "disk" )
	DISK_IMAGE( "offrthnd", 0, SHA1(d88f1c5b75361a1e310565a8a5a09c674a4a1a22) )
ROM_END

ROM_START( hydrthnd )
	ROM_REGION32_LE(0x80000, "user1", 0)
	ROM_LOAD( "lh28f004sct.u8b1", 0x000000, 0x080000, CRC(ab04a343) SHA1(ba77933400fe470f45ab187bc0d315922caadb12) )

	DISK_REGION( "disk" )
	DISK_IMAGE( "hydro", 0,  SHA1(d481d178782943c066b41764628a419cd55f676d) )
ROM_END

ROM_START( arctthnd )
	ROM_REGION32_LE(0x80000, "user1", 0)
	ROM_LOAD( "m29f002bt.u6", 0x000000, 0x040000, CRC(012c9290) SHA1(cdee6f19d5e5ea5bb1dd6a5ec397ac70b3452790) )

	DISK_REGION( "disk" )
	DISK_IMAGE( "arctthnd", 0,  SHA1(f4373e57c3f453ac09c735b5d8d99ff811416a23) )
ROM_END


// there are almost certainly multiple versions of these; updates were offered on floppy disk.  The version numbers for the existing CHDs are unknown.
GAME(1999, hydrthnd, 0, midqslvr, midqslvr, 0, ROT0, "Midway Games", "Hydro Thunder", GAME_NO_SOUND|GAME_NOT_WORKING)
GAME(2000, offrthnd, 0, midqslvr, midqslvr, 0, ROT0, "Midway Games", "Offroad Thunder", GAME_NO_SOUND|GAME_NOT_WORKING)
GAME(2001, arctthnd, 0, midqslvr, midqslvr, 0, ROT0, "Midway Games", "Arctic Thunder (v1.002)", GAME_NO_SOUND|GAME_NOT_WORKING)
