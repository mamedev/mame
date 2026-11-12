// license:BSD-3-Clause
// copyright-holders: Roberto Fresca, Team Europe
/*******************************************************************************

  WEBAK MPU-2000 "Gameworld"
  Multi-game platform (Austria)

  Preliminary driver by Roberto Fresca & Team Europe.


********************************************************************************

  Dumped boards:

  set    MPU#   PCB    OS332         game modules (IC3 pages 1-4)
  ------+------+-----+-------------+---------------------------------------
  gw606  10187  REV2   V6.06 VIE     FruitStar-DELUXE 6.04, Flush Card 6.05
  gw603  11365  REV4   V6.03 VIE-M   Flush Card 6.03, Eight 4 Fun 6.03,
                                     Lucky Lemon 6.03, FrStarBon12_K10 6.03
  gw562  11397  REV4   V5.62 VIE-M   FrStarBon12_K5 5.68, Lucky Lemon 5.10,
                                     Flush Card 5.26, Eight 4 Fun 5.26
  gw558  11418  REV4   V5.58 VIE-M   Flush Card 5.23, Eight 4 Fun 5.23,
                                     Lucky Lemon 5.05, FrStarBon12_K5 5.64
  gw557  10323  REV3   V5.57 VIE-M   FruitStar-DELUXE 5.22, Eight 4 Fun 5.19,
                                     Lucky Lemon 5.03
  gw552  10521  REV4   V5.52 VIE-M   Flush Card 5.18, Eight 4 Fun 5.18,
                                     Lucky Lemon 5.02, FrStarBon12_K5 5.58
  gw545  11168  REV4   V5.45 BGL     FruitStar-DELUXE 5.15, Flush Card 5.15


********************************************************************************

  Hardware:

  - Motorola MC68332 (CPU32 + SIM + QSM + TPU), 32.768 kHz reference crystal,
     system clock 16.777216 MHz (set by the firmware through SYNCR)

  - 2x MX29F1610 16-bit flash:
     IC4 = OS332 operating system
     IC3 = up to 4 game modules of 512K each, banked at $200000

  - 2x Altera EPF10K20, configured serially by the CPU at boot:
     FPGA #1: flash decryption, 1-Wire readers, 4 UARTs, parallel I/O,
	          YM3812 interface, bank/key registers.
     FPGA #2: video (8bpp framebuffer, per-line address table, write-time
              colour expansion/transparency), 500 kHz interval timer.

  - Brooktree Bt481 RAMDAC (only its palette part is used; emulated with the generic
    RAMDAC device in 6-bit mode, colour RAM held by the driver)

  - Yamaha YM3812 (OPL2)

  - Atmel AT25040 (REV2/REV3) or AT25010 (REV4) SPI EEPROM on the QSPI

  - HY628100 SRAMs: work RAM, VRAM and battery backed NVRAM

  - Dallas DS1216 SmartWatch (phantom RTC) under the NVRAM

  - Dallas DS2401-style 1-Wire serial number chip (flash encryption key)

  - iButton socket for the operator keys


********************************************************************************

  TODO:

  - MC68332 core.
  - A lot of things...


*******************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "screen.h"
#include "speaker.h"


namespace {

class mpu2000_state : public driver_device
{
public:
	mpu2000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void mpu2000(machine_config &config);

private:
	// not the real MCU
	required_device<m68000_device> m_maincpu;

	void main_map(address_map &map);
	virtual void machine_start() override;
	virtual void machine_reset() override;
};


/*************************************************
*  Memory Map
*************************************************/

void mpu2000_state::main_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	map(0x200000, 0x20ffff).ram();  // preliminary RAM
	map(0x800000, 0x87ffff).ram();  // preliminary RAM
}


/*************************************************
*  Input Ports
*************************************************/

static INPUT_PORTS_START( mpu2000 )
INPUT_PORTS_END


/*************************************************
*  Machine Start/Reset/Handling 
*************************************************/

void mpu2000_state::machine_start()
{
}

void mpu2000_state::machine_reset()
{
}


/*************************************************
*  Machine Driver(s)
*************************************************/

void mpu2000_state::mpu2000(machine_config &config)
{
	// basic machine hardware - using standard M68000 instead of MC68332
	m68000_device &maincpu(M68000(config, "maincpu", 32.768_kHz_XTAL));
	maincpu.set_addrmap(AS_PROGRAM, &mpu2000_state::main_map);

	// sound hardware
	SPEAKER(config, "mono").front_center();
}

/*************************************************
*  ROM Loading Routines
*************************************************/

ROM_START( gw606 )  // MPU# 10187, PCB REV2/0
	ROM_REGION16_BE( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "mx29f1610.ic4", 0x000000, 0x200000, CRC(b50a6a24) SHA1(7ede518a8eceb3b7bb3a7876c9790dfa23b06fe8) )

	ROM_REGION16_BE( 0x200000, "games", 0 )
	ROM_LOAD16_WORD_SWAP( "mx29f1610.ic3", 0x000000, 0x200000, CRC(7dcb8880) SHA1(136212e09c89fc21ab435685ceef574338e3dc3e) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "at25040.bin", 0x000, 0x200, CRC(b8b7520d) SHA1(a4d00f86d5cff046d6092810155066e2e03b5dbe) )
ROM_END

ROM_START( gw603 )  // MPU# 11365, PCB REV4/0
	ROM_REGION16_BE( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "mx29f1610amc.ic4", 0x000000, 0x200000, CRC(02a81a22) SHA1(847ddb24d3aec66950b803605b117db663b31983) )

	ROM_REGION16_BE( 0x200000, "games", 0 )
	ROM_LOAD16_WORD_SWAP( "mx29f1610amc.ic3", 0x000000, 0x200000, CRC(9555834a) SHA1(ba6c6e8751cf2ce0aa68029329dbd9e054dae189) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "at25010.bin", 0x00, 0x80, CRC(9259f7fd) SHA1(7c6e19a83324972cf891cecf2e225c42a294964f) )
ROM_END

ROM_START( gw562 )  // MPU# 11397, PCB REV4/0
	ROM_REGION16_BE( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "mx29f1610amc.ic4", 0x000000, 0x200000, CRC(7715f333) SHA1(052671bee8d5156e6c285e25f358985ab03cfa5b) )

	ROM_REGION16_BE( 0x200000, "games", 0 )
	ROM_LOAD16_WORD_SWAP( "mx29f1610amc.ic3", 0x000000, 0x200000, CRC(aacabbfe) SHA1(0dc6880c2a46631a7a526043fa0c052546bca96c) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "at25010.bin", 0x00, 0x80, CRC(400875fd) SHA1(ab1c5ac7350a1d1908d261ac2d0c9ebf33bbc0eb) )
ROM_END

ROM_START( gw558 )  // MPU# 11418, PCB REV4/0
	ROM_REGION16_BE( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "mx29f1610amc.ic4", 0x000000, 0x200000, CRC(910dc0a7) SHA1(f18c65d22e4bed307cc2780b102d4dad9afd43de) )

	ROM_REGION16_BE( 0x200000, "games", 0 )
	ROM_LOAD16_WORD_SWAP( "mx29f1610amc.ic3", 0x000000, 0x200000, CRC(cda95ac2) SHA1(38a45da3a9b31cf2440ae9512e6bd5d0916a8771) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "at25010.bin", 0x00, 0x80, CRC(2ef3ffbe) SHA1(ecaf65ac339bfefbd192ec347a17d501277c7cde) )
ROM_END

ROM_START( gw557 )  // MPU# 10323, PCB REV3/0
	ROM_REGION16_BE( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "mx29f1610.ic4", 0x000000, 0x200000, CRC(84c8e8d4) SHA1(7280abee036256343a2be72cdbd8f647b3768cf0) )

	ROM_REGION16_BE( 0x200000, "games", 0 )
	ROM_LOAD16_WORD_SWAP( "mx29f1610.ic3", 0x000000, 0x200000, CRC(5d45d768) SHA1(c7626cf11baf02cef1d0430db44e66bdbba38cb6) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "at25040.bin", 0x000, 0x200, CRC(0868269f) SHA1(cec266a1bcd48ac38835bc3540751316e057fc92) )
ROM_END

ROM_START( gw552 )  // MPU# 10521, PCB REV4/0
	ROM_REGION16_BE( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "mx29f1610mc.ic4", 0x000000, 0x200000, CRC(53078284) SHA1(dd86aec7123ea9a4f69100b9b4dff75fb830320e) )

	ROM_REGION16_BE( 0x200000, "games", 0 )
	ROM_LOAD16_WORD_SWAP( "mx29f1610mc.ic3", 0x000000, 0x200000, CRC(2c7a6f5e) SHA1(7ecf41f2a58bd37da050b57a5a6cb920fafa2b69) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "at25010.bin", 0x00, 0x80, CRC(4e5b82e8) SHA1(9786ac02c7df3931a966f569917434db3c4cdb3f) )
ROM_END

ROM_START( gw545 )  // MPU# 11168, PCB REV4/0
	ROM_REGION16_BE( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "mx29f1610mc.ic4", 0x000000, 0x200000, CRC(8519384d) SHA1(60d33da5536344f25800d70b802466d2eeb3a387) )

	ROM_REGION16_BE( 0x200000, "games", 0 )
	ROM_LOAD16_WORD_SWAP( "mx29f1610mc.ic3", 0x000000, 0x200000, CRC(a843b3e8) SHA1(f2391666cf406803c48c43053c730794ef061669) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "at25010.bin", 0x00, 0x80, CRC(88369ca7) SHA1(448f605c2b4fa5fefbfc3947a2398ca4eb089885) )
ROM_END

} // anonymous namespace


/*************************************************
*  Game Drivers
*************************************************/

//    YEAR  NAME   PARENT MACHINE  INPUT    CLASS          INIT        ROT   COMPANY  FULLNAME                                                                                                           FLAGS
GAME( 2003, gw606, 0,     mpu2000, mpu2000, mpu2000_state, empty_init, ROT0, "WEBAK", "Gameworld (OS332 V6.06 VIE: FruitStar-DELUXE V6.04 / Flush Card V6.05)",                                          MACHINE_NOT_WORKING )
GAME( 2002, gw603, gw606, mpu2000, mpu2000, mpu2000_state, empty_init, ROT0, "WEBAK", "Gameworld (OS332 V6.03 VIE-M: Flush Card V6.03 / Eight 4 Fun V6.03 / Lucky Lemon V6.03 / FrStarBon12_K10 V6.03)", MACHINE_NOT_WORKING )
GAME( 2002, gw562, gw606, mpu2000, mpu2000, mpu2000_state, empty_init, ROT0, "WEBAK", "Gameworld (OS332 V5.62 VIE-M: FrStarBon12_K5 V5.68 / Lucky Lemon V5.10 / Flush Card V5.26 / Eight 4 Fun V5.26)",  MACHINE_NOT_WORKING )
GAME( 2002, gw558, gw606, mpu2000, mpu2000, mpu2000_state, empty_init, ROT0, "WEBAK", "Gameworld (OS332 V5.58 VIE-M: Flush Card V5.23 / Eight 4 Fun V5.23 / Lucky Lemon V5.05 / FrStarBon12_K5 V5.64)",  MACHINE_NOT_WORKING )
GAME( 2002, gw557, gw606, mpu2000, mpu2000, mpu2000_state, empty_init, ROT0, "WEBAK", "Gameworld (OS332 V5.57 VIE-M: FruitStar-DELUXE V5.22 / Eight 4 Fun V5.19 / Lucky Lemon V5.03)",                   MACHINE_NOT_WORKING )
GAME( 2002, gw552, gw606, mpu2000, mpu2000, mpu2000_state, empty_init, ROT0, "WEBAK", "Gameworld (OS332 V5.52 VIE-M: Flush Card V5.18 / Eight 4 Fun V5.18 / Lucky Lemon V5.02 / FrStarBon12_K5 V5.58)",  MACHINE_NOT_WORKING )
GAME( 2001, gw545, gw606, mpu2000, mpu2000, mpu2000_state, empty_init, ROT0, "WEBAK", "Gameworld Burgenland (OS332 V5.45 BGL: FruitStar-DELUXE V5.15 / Flush Card V5.15)",                               MACHINE_NOT_WORKING )

