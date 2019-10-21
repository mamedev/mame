// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/*****************************************************************

    Odyssey.
    Silicon Gaming.
    PC based hardware.

    Preliminary driver by Roberto Fresca.

******************************************************************

  Hardware Notes
  --------------

  System Hardware:

  The slot machine system is based on an Intel Pentium motherboard.
  Could be either a Thor or Tucson motherboard with a Pentium micro
  processor running at 133 MHz or higher, with the following bus
  interfaces:

  - PCI bus for connecting to the video controller, peripheral memory
    board, and SCSI disk controller.

  - ISA bus for connecting to the GPIO system (through parallel port),
    ethernet board, on-board sound chip.

  The Thor motherboard is based on Intel's Triton I chipset, which includes
  the 82437FX/82438FX PCI bridge chips, the PIIX ISA bridge chip, the 87306
  chip that provides the serial ports, timers and interrupts, and the IEEE
  1284 parallel port interface to the GPIO system.

  The Tucson motherboard is based on Intel's Triton-II chipset, which includes
  the 82439HX PCI bridge chip, the PIIX3 ISA bridge chip, and the 87306B super
  I/O chip that provides the serial ports, timers and interrupts, and the IEEE
  1284 parallel port interface to the GPIO system.

  The original manufacturer's BIOS is removed from the motherboard. The system
  uses the Silicon Gaming BIOS on the Peripheral Memory Board instead.

  The motherboard has 4x 16MB SIMMs, getting an amount of 64MB of RAM.


  Peripheral Memory Board:

  The Peripheral Memory Board stores the boot code, motherboard basic I/O
  system (BIOS), operating system (OS), drivers, authentication software,
  system configuration, statistics, and game state information. Data on
  the Peripheral Memory Board remains after the system is powered off,
  using the following memory modules:

  - ROM
  - NVRAM
  - EEPROM

  A GPIO box is connected to the motherboard through parallel interface.
  Could be either GPIO I or GPIO II.


  The display monitor is a 26" Philips CRT with a 16:9 aspect ratio,
  mounted in portrait mode onto the monitor bezel. The electronic
  chassis is manufactured by Neotec.


******************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "screen.h"

class odyssey_state : public driver_device
{
public:
	odyssey_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void odyssey(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void odyssey_map(address_map &map);
};

void odyssey_state::video_start()
{
}

uint32_t odyssey_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}


/**************************************
*             Memory Map              *
**************************************/

void odyssey_state::odyssey_map(address_map &map)
{
}


/**************************************
*            Input Ports              *
**************************************/

static INPUT_PORTS_START( odyssey )
INPUT_PORTS_END


/**************************************
*        Machine Start/Reset          *
**************************************/

void odyssey_state::machine_start()
{
}

void odyssey_state::machine_reset()
{
}


/**************************************
*           Machine Config            *
**************************************/

void odyssey_state::odyssey(machine_config &config)
{
	/* basic machine hardware */
	PENTIUM(config, m_maincpu, 133000000); // a Celeron at 1.70 GHz on the MB I checked.
	m_maincpu->set_addrmap(AS_PROGRAM, &odyssey_state::odyssey_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(odyssey_state::screen_update));
}


/**************************************
*              ROM Load               *
**************************************/

/*
#define ODYSSEY_BIOS \
    ROM_REGION( 0x80000, "maincpu", 0 ) \
    ROM_SYSTEM_BIOS( 0, "bios0", "SGI BIOS 76" ) \
    ROM_LOAD_BIOS( 0,  "sgi_bios_76.bin", 0x000000, 0x80000, CRC(00592222) SHA1(29281d25aaf2051e0794dece8be146bb63d5c488) ) \
    ROM_SYSTEM_BIOS( 1, "bios1", "SGI BIOS 65" ) \
    ROM_LOAD_BIOS( 1,  "sgi_bios_65.bin", 0x000000, 0x80000, CRC(00592222) SHA1(29281d25aaf2051e0794dece8be146bb63d5c488) ) \
    ROM_SYSTEM_BIOS( 2, "bios2", "SGI BIOS 55" ) \
    ROM_LOAD_BIOS( 2,  "sgi_bios_55.bin", 0x000000, 0x80000, CRC(00592222) SHA1(29281d25aaf2051e0794dece8be146bb63d5c488) ) \
    ROM_SYSTEM_BIOS( 3, "bios3", "SGI BIOS 46" ) \
    ROM_LOAD_BIOS( 3,  "sgi_bios_46.bin", 0x000000, 0x80000, CRC(00592222) SHA1(29281d25aaf2051e0794dece8be146bb63d5c488) ) \
    ROM_SYSTEM_BIOS( 4, "bios4", "SGI BIOS 31" ) \
    ROM_LOAD_BIOS( 4,  "sgi_bios_31.bin", 0x000000, 0x80000, CRC(00592222) SHA1(29281d25aaf2051e0794dece8be146bb63d5c488) ) \
    ROM_SYSTEM_BIOS( 5, "bios5", "SGI BIOS 00" ) \
    ROM_LOAD_BIOS( 5,  "sgi_bios_00.bin", 0x000000, 0x80000, CRC(00592222) SHA1(29281d25aaf2051e0794dece8be146bb63d5c488) )
*/

ROM_START( odyssey )

//  ODYSSEY_BIOS

	ROM_REGION( 0x80000, "maincpu", 0 )  // main BIOS
	ROM_LOAD( "sgi_bios_76.bin", 0x000000, 0x80000, CRC(00592222) SHA1(29281d25aaf2051e0794dece8be146bb63d5c488) )

	ROM_REGION( 0x500000, "other", 0 )  // remaining BIOS
	ROM_LOAD( "sgi_bios_65.bin", 0x000000, 0x80000, CRC(af970c2a) SHA1(0fb49bca34dbd0725b5abb9c876bb849be31b3ed) )
	ROM_LOAD( "sgi_bios_55.bin", 0x080000, 0x80000, CRC(0138ef08) SHA1(fad1c0edf37042fffcb5a4006fd69ac59b55ab33) )
	ROM_LOAD( "sgi_bios_46.bin", 0x100000, 0x80000, CRC(37090b87) SHA1(431c0a1954d5bf7fd4fa6f2b983010fbf3c8ce13) )
	ROM_LOAD( "sgi_bios_31.bin", 0x180000, 0x80000, CRC(0954278b) SHA1(dc04a0604159ddd3d24bdd292b2947cc443054f8) )
	ROM_LOAD( "sgi_bios_00.bin", 0x200000, 0x80000, CRC(41480fb5) SHA1(073596d3ba40ae67e3be3f410d7b29c77988df47) )

	ROM_REGION( 0x100000, "pmb", 0 )   // Peripheral Memory Board (II) ROMS
	ROM_LOAD( "sgi_u13_165_0017_0_rev_a_l97_1352.bin", 0x00000, 0x80000, CRC(31ca868c) SHA1(d1db4ef12add336e25374fcf5d3238b8fbca05dd) )  // U13 - 165-0017 BIOS (27C040/27C4001 EPROM)
	ROM_LOAD( "sgi_u5_165_0030_0_at28c010.bin",        0x80000, 0x20000, CRC(75a80169) SHA1(a8ece0f82a49f721fb178dbe25fc859bd65ce44f) )  // U5 - 165-0030 CONFIG (Atmel 28C010-12PC EEPROM)

	ROM_REGION( 0x10000, "vbios", 0 )   // video card BIOS
	ROM_LOAD( "videobios", 0x000000, 0x00d000, NO_DUMP )

	ROM_REGION( 0x10000, "scsibios", 0 )   // SCSI card BIOS
	ROM_LOAD( "scsibios", 0x000000, 0x00d000, NO_DUMP )

	DISK_REGION( "scsi_hdd_image" ) // SCSI HDD
	DISK_IMAGE( "odyssey", 0, NO_DUMP )

ROM_END


/**************************************
*           Game Driver(s)            *
**************************************/

/*    YEAR  NAME      PARENT  MACHINE  INPUT    STATE          INIT        ROT   COMPANY           FULLNAME    FLAGS  */
GAME( 1998, odyssey,  0,      odyssey, odyssey, odyssey_state, empty_init, ROT0, "Silicon Gaming", "Odyssey",   MACHINE_IS_SKELETON )
