/***************************************************************************

    Incredible Technologies "Eagle" hardware

    skeleton by R. Belmont

    Known games on this hardware:
	    * Golden Tee Fore! (2000)
	    * Golden Tee Fore! 2002 (2001)
	    * Golden Tee Fore! 2003 (2002)
        * Golden Tee Fore! 2004 (2003)
    	* Silver Strike Bowling (2004)
    	* Golden Tee Fore! 2005 (2004)
    	* Golden Tee Fore! Complete (2005)
 
    Hardware overview:
        * NEC VR4310 CPU (similar to the N64's VR4300)
        * NEC VR4373 "Nile 3" system controller / PCI bridge
        * 3DFX Voodoo Banshee video
 
    TODO:
		* Everything (need new PCI subsystem to do this right) 
 
***************************************************************************/

#include "emu.h"
#include "cpu/mips/mips3.h"
#include "machine/idectrl.h"
#include "video/voodoo.h"

class iteagle_state : public driver_device
{
public:
	iteagle_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_voodoo(*this, "voodoo")
	{}

	required_device<cpu_device> m_maincpu;
	required_device<device_t>   m_voodoo;

	DECLARE_DRIVER_INIT(iteagle);
	DECLARE_WRITE_LINE_MEMBER(ide_interrupt);
	UINT32 screen_update_iteagle(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};


/*************************************
 *
 *  Machine start
 *
 *************************************/

static MACHINE_START( gtfore )
{
	/* set the fastest DRC options */
	mips3drc_set_options(machine.device("maincpu"), MIPS3DRC_FASTEST_OPTIONS);
}



/*************************************
 *
 *  Machine init
 *
 *************************************/

static MACHINE_RESET( gtfore )
{
}



/*************************************
 *
 *  Video refresh
 *
 *************************************/

UINT32 iteagle_state::screen_update_iteagle(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return voodoo_update(m_voodoo, bitmap, cliprect) ? 0 : UPDATE_HAS_NOT_CHANGED;
}



/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

WRITE_LINE_MEMBER(iteagle_state::ide_interrupt)
{
}

/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 32, iteagle_state )
	ADDRESS_MAP_UNMAP_HIGH
    AM_RANGE(0x00000000, 0x01ffffff) AM_RAM
	// Nile 3 northbridge/PCI controller at 0f000000
	AM_RANGE(0x1fc00000, 0x1fcfffff) AM_ROM AM_REGION("maincpu", 0) AM_SHARE("rombase")
ADDRESS_MAP_END

static void vblank_assert(device_t *device, int state)
{
}

/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( gtfore )
INPUT_PORTS_END

/*************************************
 *
 *  Machine driver
 *
 *************************************/

static const voodoo_config iteagle_voodoo_intf =
{
	16, //              fbmem;
	0,//                tmumem0;
	0,//                tmumem1;
	"screen",//     screen;
	"maincpu",//            cputag;
	vblank_assert,//    vblank;
	NULL,//             stall;
};

static const mips3_config r4310_config =
{
	16384,				/* code cache size */
	16384				/* data cache size */
};

static MACHINE_CONFIG_START( gtfore, iteagle_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", VR4310LE, 166666666)
	MCFG_CPU_CONFIG(r4310_config)
	MCFG_CPU_PROGRAM_MAP(main_map)

	MCFG_MACHINE_START(gtfore)
	MCFG_MACHINE_RESET(gtfore)

	MCFG_IDE_CONTROLLER_ADD("ide", ide_devices, "hdd", NULL, true)
	MCFG_IDE_CONTROLLER_IRQ_HANDLER(DEVWRITELINE(DEVICE_SELF, iteagle_state, ide_interrupt))

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(320, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE_DRIVER(iteagle_state, screen_update_iteagle)

	MCFG_3DFX_VOODOO_BANSHEE_ADD("voodoo", STD_VOODOO_BANSHEE_CLOCK, iteagle_voodoo_intf)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

#define EAGLE_BIOS \
    ROM_REGION( 0x100000, "maincpu", 0 ) /* MIPS code */ \
	ROM_SYSTEM_BIOS( 0, "209", "bootrom 2.09" ) \
    ROM_LOAD( "eagle209.u15", 0x000000, 0x100000, CRC(e0fc1a16) SHA1(c9524f7ee6b95bd484a3b75bcbe2243cb273f84c) ) \
	ROM_SYSTEM_BIOS( 1, "208", "bootrom 2.08" ) \
    ROM_LOAD( "eagle208.u15", 0x000000, 0x100000, CRC(772f2864) SHA1(085063a4e34f29ebe3814823cd2c6323a050da36) ) \
	ROM_SYSTEM_BIOS( 2, "204", "bootrom 2.04" ) \
    ROM_LOAD( "eagle204.u15", 0x000000, 0x100000, CRC(f02e5523) SHA1(b979cf72a6992f1ecad9695a08c8d51e315ab537) ) \
	ROM_SYSTEM_BIOS( 3, "201", "bootrom 2.01" ) \
    ROM_LOAD( "eagle201.u15", 0x000000, 0x100000, CRC(e180442b) SHA1(4f50821fed5bcd786d989520aa2559d6c416fb1f) ) \
	ROM_SYSTEM_BIOS( 4, "107", "bootrom 1.07" ) \
    ROM_LOAD( "eagle107.u15", 0x000000, 0x100000, CRC(97a01fc9) SHA1(a421dbf4d097b2f50cc005d3cd0d63e562e03df8) ) \
	ROM_SYSTEM_BIOS( 5, "106a", "bootrom 1.06a" ) \
    ROM_LOAD( "eagle106a.u15", 0x000000, 0x100000, CRC(9c79b7ad) SHA1(ccf1c86e79d65bee30f399e0fa33a7839570d93b) ) \
	ROM_SYSTEM_BIOS( 6, "106", "bootrom 1.06" ) \
    ROM_LOAD( "eagle106.u15", 0x000000, 0x100000, CRC(56bc193d) SHA1(e531d208ef27f777d0784414885f390d1be654b9) ) \
	ROM_SYSTEM_BIOS( 7, "105", "bootrom 1.05" ) \
    ROM_LOAD( "eagle105.u15", 0x000000, 0x100000, CRC(3870dbe0) SHA1(09be2d86c7259cd81d945c757044b167a76f30db) ) \
	ROM_SYSTEM_BIOS( 8, "103", "bootrom 1.03" ) \
    ROM_LOAD( "eagle103.u15", 0x000000, 0x100000, CRC(c35f4cf2) SHA1(45301c18c7f8f78754c8ad60ea4d2da5a7dc55fb) ) \
	ROM_SYSTEM_BIOS( 9, "101", "bootrom 1.01" ) \
    ROM_LOAD( "eagle101.u15", 0x000000, 0x100000, CRC(2600bc2b) SHA1(c4b89e69c51e4a3bb1874407c4d30b6caed4f396) )

ROM_START( iteagle )
    EAGLE_BIOS    

	DISK_REGION( "drive_0" )
ROM_END

ROM_START( gtfore04 )
    EAGLE_BIOS    

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "gt2004", 0, SHA1(739a52d6ce13bb6ac7a543ee0e8086fb66be19b9) )
ROM_END

ROM_START( gtfore05 )
    EAGLE_BIOS    

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "gt2005", 0, SHA1(d8de569d8cf97b5aaada10ce896eb3c75f1b37f1) )
ROM_END

DRIVER_INIT_MEMBER(iteagle_state, iteagle)
{
}

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 2000, iteagle,        0, gtfore, gtfore, iteagle_state, iteagle, ROT0, "Incredible Technologies", "Eagle BIOS", GAME_IS_BIOS_ROOT )
GAME( 2003, gtfore04, iteagle, gtfore, gtfore, iteagle_state, iteagle, ROT0, "Incredible Technologies", "Golden Tee Fore! 2004", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2004, gtfore05, iteagle, gtfore, gtfore, iteagle_state, iteagle, ROT0, "Incredible Technologies", "Golden Tee Fore! 2005", GAME_NOT_WORKING | GAME_NO_SOUND )

