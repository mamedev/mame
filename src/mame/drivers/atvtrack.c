/*
  ATV Track
  (c)2002 Gaelco

ATV Track
Gaelco 2002

PCB Layout

GAELCO
REF. 020419
 |--------------------------------------------------------------|
 |                                                              |
 |   SW3                                               EPC1PC8  |
 |                                         K4S643232            |
 |    LC245A                                         7LB176    |-|
 |                        FLASH.IC14  FLASH.IC19     7LB176    | |
 |       |-----|                                     7LB176    | |DB9
 |       | SH4 |                                     7LB176    |-|
 |       |     |          FLASH.IC15  FLASH.IC20                |
 |       |-----|                                                |
 |                                                |----------|  |
|-|                            K4S643232          |ALTERA    |  |
| |  L4955                                        |FLEX      |  |
| |      |-----|               K4S643232          |EPF10K50  |  |
| |CN1   | SH4 |                                  |EQC240-3  |  |
| |      |     |                                  |----------|  |
| |      |-----|                                                |
|-|                                                             |
 |                                                              |
 |      33MHz                       K4S643232     |----------|  |
 |                                  K4S643232     | GFX      |  |
 |             LED                                |          | |-|
 |             LED                                |          | | |
 |                                  K4S643232     |          | | |DB9
 |                                  K4S643232     |----------| |-|
 | TL074C   TL074C                                     385-1    |
 |     TDA1387   TDA1387                          14.31818MHz   |
 |                                                              |
 |--------------------------------------------------------------|
Notes:
      SH4       - Hitachi HD6417750S SH4 CPU (BGA)
      K4S643232 - Samsung K4S643232E-TC70 64M x 32-bit SDRAM (TSSOP86)
      GFX       - Unknown BGA graphics chip (heatsinked)
      FLASH.IC* - Samsung K9F2808U0B 128MBit (16M + 512k Spare x 8-bit) FlashROM (TSOP48)
      EPF10K50  - Altera Flex EPF10K50EQC240-3 FPGA (QFP240)
      EPC1PC8   - Altera EPC1PC8 FPGA Configuration Device (DIP8)
      TL074C    - Texas Instruments TL074C Low Noise Quad JFet Operational Amplifier (SOIC14)
      TDA1387   - Philips TDA1387 Stereo Continuous Calibration DAC (SOIC8)
      L4955     - ST Microelectronics L4955 low-power, quad channel, 8-bit buffered voltage output DAC and amplifier
      7LB176    - Texas Instruments 7LB176 Differential Bus Tranceiver (SOIC8)
      385-1     - National LM385 Adjustable Micropower Voltage Reference Diode (SOIC8)
      CN1       - Multi-pin connector for filter board (input power & controls connectors etc)
      DB9       - Video output connector (for twin monitors)
      SW3       - Push button switch

*/

#include "emu.h"
#include "cpu/sh4/sh4.h"


class atvtrack_state : public driver_device
{
public:
	atvtrack_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

};


VIDEO_START(atvtrack)
{
}

SCREEN_UPDATE(atvtrack)
{
	return 0;
}

static ADDRESS_MAP_START( atvtrack_map, ADDRESS_SPACE_PROGRAM, 64 )
	AM_RANGE(0x00000000, 0x001fffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( atvtrack_port, ADDRESS_SPACE_IO, 64 )
ADDRESS_MAP_END


static INPUT_PORTS_START( atvtrack )
INPUT_PORTS_END

// ?
#define ATV_CPU_CLOCK 200000000
// ?
static const struct sh4_config sh4cpu_config = {  1,  0,  1,  0,  0,  0,  1,  1,  0, ATV_CPU_CLOCK };

static MACHINE_CONFIG_START( atvtrack, atvtrack_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SH4, ATV_CPU_CLOCK)
	MCFG_CPU_CONFIG(sh4cpu_config)
	MCFG_CPU_PROGRAM_MAP(atvtrack_map)
	MCFG_CPU_IO_MAP(atvtrack_port)

	MCFG_CPU_ADD("subcpu", SH4, ATV_CPU_CLOCK)
	MCFG_CPU_CONFIG(sh4cpu_config)
	MCFG_CPU_PROGRAM_MAP(atvtrack_map)
	MCFG_CPU_IO_MAP(atvtrack_port)
	MCFG_DEVICE_DISABLE()

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))  /* not accurate */
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE(atvtrack)

	MCFG_PALETTE_LENGTH(0x1000)

	MCFG_VIDEO_START(atvtrack)
MACHINE_CONFIG_END

ROM_START( atvtrack )
	ROM_REGION( 0x4200000, "maincpu", ROMREGION_ERASEFF) // NAND roms, contain additional data hence the sizes
	ROM_LOAD32_BYTE("15.bin", 0x0000000, 0x1080000, CRC(84eaede7) SHA1(6e6230165c3bb35e49c660dfd0d07c132ed89e6a) )
	ROM_LOAD32_BYTE("20.bin", 0x0000001, 0x1080000, CRC(649dc331) SHA1(0cac2d0c15dd564c7fdebdf4365422958f453d63) )
	ROM_LOAD32_BYTE("14.bin", 0x0000002, 0x1080000, CRC(67983453) SHA1(05389a0ffc1a1bae9bac16a53a97d78b6eccc626) )
	ROM_LOAD32_BYTE("19.bin", 0x0000003, 0x1080000, CRC(9fc5c579) SHA1(8829329ef229564952aea2108ef1750dc226cbac) )

	ROM_REGION( 0x20000, "eeprom", ROMREGION_ERASEFF)
	ROM_LOAD("epc1pc8.ic23", 0x0000000, 0x1ff01, CRC(752444c7) SHA1(c77e8fcfcbe15b53eda25553763bdac45f0ef7df) ) // contains a large amount of data, maybe used for some form of protection
ROM_END

ROM_START( atvtracka )
	ROM_REGION( 0x4200000, "maincpu", ROMREGION_ERASEFF) // NAND roms, contain additional data hence the sizes
	ROM_LOAD32_BYTE("k9f2808u0b.ic15", 0x0000000, 0x1080000, CRC(10730001) SHA1(48c685a6ff7135abd074dc7fb7d10834c44da58f) )
	ROM_LOAD32_BYTE("k9f2808u0b.ic20", 0x0000001, 0x1080000, CRC(b0c34433) SHA1(852c79bb3d7082cd2c056140071ae7d71679ec1d) )
	ROM_LOAD32_BYTE("k9f2808u0b.ic14", 0x0000002, 0x1080000, CRC(02a12085) SHA1(acb112c9c7b29d92610465fb92268ce787ca06f4) )
	ROM_LOAD32_BYTE("k9f2808u0b.ic19", 0x0000003, 0x1080000, CRC(856c1e6a) SHA1(a6b2839120d61811c36cc6b4095de9cefceb394b) )
ROM_END

GAME( 2002, atvtrack,  0,          atvtrack,    atvtrack,    0, ROT0, "Gaelco", "ATV Track (set 1)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2002, atvtracka, atvtrack,   atvtrack,    atvtrack,    0, ROT0, "Gaelco", "ATV Track (set 2)", GAME_NOT_WORKING | GAME_NO_SOUND )
