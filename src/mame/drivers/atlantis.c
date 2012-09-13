/***************************************************************************

    Midway "Atlantis" hardware

    skeleton by R. Belmont

    Games supported:
        * Midway Skins Game
        * Midway Skins Game Tournament Edition (not dumped)
        * Midway Swingers Tour (not dumped)

    Hardware overview:
        * VR4310 CPU (similar to the N64's VR4300)
        * VR4373 "Nile 3" system controller / PCI bridge
        * CMD 646U2 Ultra DMA IDE controller
        * M4T28-8R128H1 TimeKeeper RTC/CMOS
        * PLX PCI9050 Bus Target Interface Chip (interfaces ISA-style designs to PCI)
        * Midway Zeus-series custom video
        * TL16c552 dual UART
        * ADSP-2181 based DCS2 audio (unclear which variant)
        * PIC16C57 (protection? serial #?)
        * Quantum Fireball CX 6.4GB IDE HDD (C/H/S 13328/15/63)

    TODO:
        * Proper VR4373 implementation
        * Proper PCI bus implementation
        * PCI peripherals

    NOTES:
        * Skins Game is Linux based; the kernel is a customized 2.2.10 build of Linux-MIPS with Midway PCBs
          added as recognized system types

***************************************************************************/

#include "emu.h"
#include "cpu/mips/mips3.h"
#include "cpu/adsp2100/adsp2100.h"
#include "machine/idectrl.h"
#include "machine/midwayic.h"
#include "audio/dcs.h"


class atlantis_state : public driver_device
{
public:
	atlantis_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) {}
	DECLARE_DRIVER_INIT(mwskins);
	virtual void machine_start();
	virtual void machine_reset();
};


/*************************************
 *
 *  Machine start
 *
 *************************************/

void atlantis_state::machine_start()
{
	/* set the fastest DRC options */
	mips3drc_set_options(machine().device("maincpu"), MIPS3DRC_FASTEST_OPTIONS);
}



/*************************************
 *
 *  Machine init
 *
 *************************************/

void atlantis_state::machine_reset()
{
    dcs_reset_w(machine(), 1);
    dcs_reset_w(machine(), 0);
}



/*************************************
 *
 *  Video refresh
 *
 *************************************/

static SCREEN_UPDATE_IND16( mwskins )
{
	return 0;
}



/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

static void ide_interrupt(device_t *device, int state)
{
}

/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 32, atlantis_state )
	ADDRESS_MAP_UNMAP_HIGH
    AM_RANGE(0x00000000, 0x007fffff) AM_RAM // 8 MB main RAM
    // 04000000 - PCI slot (ActionTec modem, ROM dump TBD)
    // 08000000 - PLX9050 chip (Zeus interface?)
    // 0F000000 - VR4373 ("Nile 3") registers
	AM_RANGE(0x1fc00000, 0x1fc7ffff) AM_ROM AM_REGION("user1", 0) AM_SHARE("rombase")
ADDRESS_MAP_END




/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( mwskins )
INPUT_PORTS_END

/*************************************
 *
 *  Machine driver
 *
 *************************************/

static const mips3_config r4310_config =
{
	16384,				/* code cache size */
	16384				/* data cache size */
};

static const ide_config ide_intf = 
{
	ide_interrupt, 
	NULL, 
	0
};

static MACHINE_CONFIG_START( mwskins, atlantis_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", VR4310LE, 166666666)    // clock is TRUSTED
	MCFG_CPU_CONFIG(r4310_config)
	MCFG_CPU_PROGRAM_MAP(main_map)


	MCFG_IDE_CONTROLLER_ADD("ide", ide_intf, ide_devices, "hdd", NULL, true)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(320, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE_STATIC(mwskins)

	MCFG_PALETTE_INIT(BBBBB_GGGGG_RRRRR)
	MCFG_PALETTE_LENGTH(32768)

	/* sound hardware */
	MCFG_FRAGMENT_ADD(dcs2_audio_denver)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( mwskins )
	ROM_REGION32_LE( 0x80000, "user1", 0 )	/* 512k for R4310 code */
    ROM_LOAD( "skins_game_u4_boot_1.00.u4", 0x000000, 0x080000, CRC(0fe87720) SHA1(4b24abbe662a2d7b61e6a3f079e28b73605ba19f) )

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "mwskins", 0, SHA1(5cb293a6fdb2478293f48ddfc93cdd018acb2bb5) )
ROM_END

ROM_START( mwskinsa )
	ROM_REGION32_LE( 0x80000, "user1", 0 )	/* 512k for R4310 code */
    ROM_LOAD( "skins_game_u4_boot_1.00.u4", 0x000000, 0x080000, CRC(0fe87720) SHA1(4b24abbe662a2d7b61e6a3f079e28b73605ba19f) )

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "mwskinsa", 0, SHA1(72497917b31156eb11a46bbcc6f22a254dcec044) )
ROM_END

ROM_START( mwskinso )
	ROM_REGION32_LE( 0x80000, "user1", 0 )	/* 512k for R4310 code */
    ROM_LOAD( "skins_game_u4_boot_1.00.u4", 0x000000, 0x080000, CRC(0fe87720) SHA1(4b24abbe662a2d7b61e6a3f079e28b73605ba19f) )

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "mwskins104", 0, SHA1(6917f66718999c144c854795c5856bf5659b85fa) )
ROM_END

/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(atlantis_state,mwskins)
{
	dcs2_init(machine(), 8, 0);
}

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 2000, mwskins,    0,      mwskins, mwskins, atlantis_state,  mwskins,   ROT0, "Midway", "Skins Game (1.06)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2000, mwskinsa, mwskins,  mwskins, mwskins, atlantis_state,  mwskins,   ROT0, "Midway", "Skins Game (1.06, alt)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2000, mwskinso, mwskins,  mwskins, mwskins, atlantis_state,  mwskins,   ROT0, "Midway", "Skins Game (1.04)", GAME_NOT_WORKING | GAME_NO_SOUND )

