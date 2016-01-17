// license:BSD-3-Clause
// copyright-holders:R. Belmont
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
#include "machine/pci.h"
#include "machine/vrc4373.h"
#include "machine/pci9050.h"

class atlantis_state : public driver_device
{
public:
	atlantis_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dcs(*this, "dcs") { }
	DECLARE_DRIVER_INIT(mwskins);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	UINT32 screen_update_mwskins(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<mips3_device> m_maincpu;
	required_device<dcs2_audio_denver_device> m_dcs;

	READ32_MEMBER (red_r);
	WRITE32_MEMBER(red_w);
	READ32_MEMBER (green_r);
	WRITE32_MEMBER(green_w);
	READ32_MEMBER (blue_r);
	WRITE32_MEMBER(blue_w);
};

READ32_MEMBER (atlantis_state::red_r)
{
	logerror("red_r %x\n", offset);
	return 0;
}

WRITE32_MEMBER(atlantis_state::red_w)
{
	logerror("red_w %x,%x\n", offset, data);
}

READ32_MEMBER (atlantis_state::green_r)
{
	logerror("green_r %x\n", offset);
	return 0;
}

WRITE32_MEMBER(atlantis_state::green_w)
{
	logerror("green_w %x,%x\n", offset, data);
}

READ32_MEMBER (atlantis_state::blue_r)
{
	logerror("blue_r %x\n", offset);
	return 0;
}

WRITE32_MEMBER(atlantis_state::blue_w)
{
	logerror("blue_w %x,%x\n", offset, data);
}


/*************************************
 *
 *  Machine start
 *
 *************************************/

void atlantis_state::machine_start()
{
	/* set the fastest DRC options */
	m_maincpu->mips3drc_set_options(MIPS3DRC_FASTEST_OPTIONS);
}



/*************************************
 *
 *  Machine init
 *
 *************************************/

void atlantis_state::machine_reset()
{
	m_dcs->reset_w(1);
	m_dcs->reset_w(0);
}



/*************************************
 *
 *  Video refresh
 *
 *************************************/

UINT32 atlantis_state::screen_update_mwskins(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( map0, AS_PROGRAM, 32, atlantis_state )
	AM_RANGE(0x000000, 0xffffff) AM_READWRITE(red_r, red_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( map1, AS_PROGRAM, 32, atlantis_state )
	AM_RANGE(0x000000, 0xffffff) AM_READWRITE(green_r, green_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( map3, AS_PROGRAM, 32, atlantis_state )
	AM_RANGE(0x000000, 0xffffff) AM_READWRITE(blue_r, blue_w)
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

static MACHINE_CONFIG_START( mwskins, atlantis_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", VR4310LE, 166666666)    // clock is TRUSTED
	MCFG_MIPS3_ICACHE_SIZE(16384)
	MCFG_MIPS3_DCACHE_SIZE(16384)

	MCFG_PCI_ROOT_ADD(                ":pci")
	MCFG_VRC4373_ADD(                 ":pci:00.0", ":maincpu")
	MCFG_PCI9050_ADD(                 ":pci:0b.0")
	MCFG_PCI9050_SET_MAP(0, map0)
	MCFG_PCI9050_SET_MAP(1, map1) // 2 skipped for testing
	MCFG_PCI9050_SET_MAP(3, map3)

	MCFG_IDE_CONTROLLER_ADD("ide", ata_devices, "hdd", nullptr, true)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(320, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE_DRIVER(atlantis_state, screen_update_mwskins)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_BBBBBGGGGGRRRRR("palette")

	/* sound hardware */
	MCFG_DEVICE_ADD("dcs", DCS2_AUDIO_DENVER, 0)
	MCFG_DCS2_AUDIO_DRAM_IN_MB(8)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( mwskins )
	ROM_REGION32_LE( 0x80000, ":pci:00.0", 0 )  /* 512k for R4310 code */
	ROM_LOAD( "skins_game_u4_boot_1.00.u4", 0x000000, 0x080000, CRC(0fe87720) SHA1(4b24abbe662a2d7b61e6a3f079e28b73605ba19f) )

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE( "mwskins", 0, SHA1(5cb293a6fdb2478293f48ddfc93cdd018acb2bb5) )
ROM_END

ROM_START( mwskinsa )
	ROM_REGION32_LE( 0x80000, ":pci:00.0", 0 )  /* 512k for R4310 code */
	ROM_LOAD( "skins_game_u4_boot_1.00.u4", 0x000000, 0x080000, CRC(0fe87720) SHA1(4b24abbe662a2d7b61e6a3f079e28b73605ba19f) )

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE( "mwskinsa", 0, SHA1(72497917b31156eb11a46bbcc6f22a254dcec044) )
ROM_END

ROM_START( mwskinso )
	ROM_REGION32_LE( 0x80000, ":pci:00.0", 0 )  /* 512k for R4310 code */
	ROM_LOAD( "skins_game_u4_boot_1.00.u4", 0x000000, 0x080000, CRC(0fe87720) SHA1(4b24abbe662a2d7b61e6a3f079e28b73605ba19f) )

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE( "mwskins104", 0, SHA1(6917f66718999c144c854795c5856bf5659b85fa) )
ROM_END

/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(atlantis_state,mwskins)
{
}

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 2000, mwskins,    0,      mwskins, mwskins, atlantis_state,  mwskins,   ROT0, "Midway", "Skins Game (1.06)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2000, mwskinsa, mwskins,  mwskins, mwskins, atlantis_state,  mwskins,   ROT0, "Midway", "Skins Game (1.06, alt)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2000, mwskinso, mwskins,  mwskins, mwskins, atlantis_state,  mwskins,   ROT0, "Midway", "Skins Game (1.04)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
