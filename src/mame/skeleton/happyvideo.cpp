// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************

 Skeleton driver for "Happy Video" coin operated "yaoyaoche" machine (摇摇车, kiddie ride,
 literally "rocking car") with video screen.  Machine has a builder's plate with the text
 "LEYAOYAO HAPPY KIDS" and a logo depicting two birds in flight surrounded by a crescent moon,
 in silhouette.  Seems to be unrelated to Guangzhou Leyaoyao Information Technology.

   ________________________________________________________________________
  |                          _______                                      |
  |           _____         | ···· |                                      |
  |          |    |          BACKCON                                      |
  |          |    |<-MK25V4GL                                             |
  |          |____|                                                       |
  |           _____       Xtal                                            |
  |          |    |      24.000MHz                                     .. |
  |          |____|                                            _____   .. |
  |        W25Q32JVSIQ   __________                           |    |   .. |
  |                     |ALLWINNER|                           |    |   .. |
  |                     | F1C100S |                           |    |   .. |
 _|__                   |         |                           |    |   .. |
|USB |                  |_________|                           |____|   .. |
|PORT|                                                      MS90C385B     |
|____|  ______                                                        __  |
  |    | · · |                                                        · | |
  |      12V                                                          · | |
  |             ______                                                __| |
  |            |_____|<-CS8822E                                           |
  |        __________  ________     ________________   ________   ______  |
  |_______| | | | | |_| | | | |____| | | | | | | | |__| | | | |__| | | |__|
          |_|_|_|_|_| |_|_|_|_|    |_|_|_|_|_|_|_|_|  |_|_|_|_|  | | | |
    NC/GND/RX/TX/12V  GND/VDD/AVIN                               |_|_|_|


* The CS8822 is a single-cycle 8051-based MCU with 64K-byte mask ROM and 10K-byte data SRAM
  for controlling USB 2.0 mass storage devices, Flash card reader controller, ATA/ATAPI drive
  controller and MP3 audio decoder.

* MS90C385B is a +3.3V, 150 MHz, 24-Bit LVDS Flat Panel Display Transmitter.

PCB silkscreen says:
2018.08_08
CK_RB1808P_V4.1

Labeled with a sticker "单8"

The PCB was found without any USB mass storage on the USB port.  It is unknown if it needs one
(the serial EEPROM contents refers to files that are not on the MK25V4GL).

***********************************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"

namespace {

class hppyvideo_state : public driver_device
{
public:
	hppyvideo_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void hppyvideo(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
};

static INPUT_PORTS_START( hppyvideo )
INPUT_PORTS_END

void hppyvideo_state::hppyvideo(machine_config &config)
{
	// Basic machine hardware
	ARM9(config, m_maincpu, 24_MHz_XTAL);

	// USB MCU core
	//I8051(...)

	// Video hardware
	//SCREEN(...)

	// Audio hardware
	//SPEAKER(...)
}

ROM_START( hppyvideo )
	ROM_REGION( 0x040000, "maincpu", 0 )
	ROM_LOAD( "f1c100s_boot_rom.bin", 0x000000, 0x040000, NO_DUMP ) // 32 KBytes internal BROM on the F1C100S

	ROM_REGION( 0x400000, "program", 0 )
	ROM_LOAD( "w25q32jvsiq.bin",      0x000000, 0x400000, CRC(cec0842c) SHA1(1a0340321e1178a5b5033f3904a7aca7e8f0f9ec) )

	DISK_REGION( "card" )
	DISK_IMAGE( "mk25v4gl", 0, SHA1(d438cc574b7b89ae6647cdcfaca27201f920847f) ) // Not a regular SD card, but an SD card chip (controller + flash) directly soldered to the PCB

	ROM_REGION( 0x080000, "usb", 0 )
	ROM_LOAD( "cs8822e.bin",          0x000000, 0x080000, NO_DUMP ) // 64 Kbytes internal mask ROM
ROM_END

} // anonymous namespace


//    YEAR  NAME       PARENT  MACHINE    INPUT      CLASS            INIT        ROT   COMPANY                 FULLNAME       FLAGS
GAME( 20??, hppyvideo, 0,      hppyvideo, hppyvideo, hppyvideo_state, empty_init, ROT0, "Leyaoyao Happy Kids?", "Happy Video", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
