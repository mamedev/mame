/* Triforce Skeleton
 -- split from Naomi.c

 Triforce uses

a stock Gamecube motherboard with custom Bios
a 'media board' which acts as a CD/DVD emulator
the Naomi 'DIMM' board, which connects to the GD-ROM drive

Sega Triforce Hardware Overview
Sega/Namco/Nintendo 2002-2006
-----------------------------

This is a GameCube-based system containing several PCBs mounted inside a metal box.
There are two versions. One is for GDROM-based games and the other is customised for
use with Namco ROM carts.


Games on this system include....

   Game                                                      Manufacturer              Media             Key Chip
+-+---------------------------------------------------------+-------------------------+-----------------+--------------|
| |Donkey Kong Jungle Fever                                 | Namco / Nintendo, 2005  | ROM Cart        | ?            |
|*|Mario Kart Arcade GP                                     | Namco / Nintendo, 2005  | ROM Cart        | 317-5109-COM |
| |Mario Kart Arcade GP 2                                   | Namco / Nintendo, 2007  | ROM Cart        | ?            |
|*|Virtua Striker 2002 (Japan)                              | Sega, 2002              | GDROM GDT-0001  | 317-0337-JPN |
|*|Virtua Striker 2002 (Export)                             | Sega, 2002              | GDROM GDT-0002  | 317-0337-EXP |
| |F-Zero AX                                                | Sega / Nintendo, 2003   | GDROM GDT-0004  | ?            |
|*|The Key Of Avalon: The Wizard Master (server) (Rev C)    | Sega, 2003              | GDROM GDT-0005C | ?            |
| |The Key Of Avalon: The Wizard Master (client) (Rev C)    | Sega, 2003              | GDROM GDT-0006C | ?            |
|*|Gekitou Pro Yakyuu (Rev C)                               | Sega, 2003              | GDROM GDT-0008C | 317-0371-JPN |
|*|The Key Of Avalon 1.30: Chaotic Sabbat (server) (Rev C)  | Sega, 2004              | GDROM GDT-0009C | ?            |
|*|The Key Of Avalon 1.30: Chaotic Sabbat (client) (Rev C)  | Sega, 2004              | GDROM GDT-0010C | ?            |
|*|Firmware Update                                          | Sega, 2004              | GDROM GDT-0011  | ?            |
|*|Virtua Striker 4 (Japan) (Rev E)                         | Sega, 2004              | GDROM GDT-0013E | 317-0391-JPN |
|*|Virtua Striker 4 (Export)                                | Sega, 2004              | GDROM GDT-0015  | 317-0393-EXP |
| |The Key Of Avalon 2: Eutaxy Commandment (server)         | Sega, 2004              | GDROM GDT-0016  | ?            |
|*|The Key Of Avalon 2: Eutaxy Commandment (client) (Rev B) | Sega, 2004              | GDROM GDT-0017B | ?            |
|*|Virtua Striker 4 Ver.2006 (Japan) (Rev D)                | Sega, 2006              | GDROM GDT-0020D | ?            |
| |Virtua Striker 4 Ver.2006 (Export)                       | Sega, 2006              | GDROM GDT-0021  | ?            |
| |F-Zero AX - Monster Ride Cycraft Edition                 | Sega / Nintendo, 2004   | GDROM ?         | ?            |
| |The Key Of Avalon 1.10                                   | Sega, 2003              | GDROM ?         | ?            |
| |The Key Of Avalon 1.20: Summon The New Monster           | Sega, 2003              | GDROM ?         | ?            |
| |The Key Of Avalon 2.5: War of the Key                    | Sega, 2005              | GDROM ?         | ?            |
| |Donkey Kong:Banana Kingdom                               | Namco / Nintendo, 2006  | ROM Cart ?      | ?            |
| |Starfox Armada (planned, but not released)               | Namco / Nintendo, 2002? | ?               | ?            |
+-+---------------------------------------------------------+-------------------------+-----------------+--------------+
* denotes these games are archived.
If you can help with the undumped games or know of missing Triforce games, please contact...
http://guru.mameworld.info/


PCB Layouts
===========


Network Board (top)
-------------

This board is not used on the older Triforce version

837-14341
171-8226C SEGA 2002
|----------------------------|
|           LLLL     070XZ1H |
|           JP1    K4S643232 |
|           JP2    24LC04    |
|      25MHz        32.768kHz|
|CN7   L80227/B        12MHz |
|      LL          |------|  |
|                  |AMD   |  |
|      CN1         |AU1500|  |
|      CN2         |------|  |
|                   FLASH.IC2|
|CN4      ADM3222            |
|     JP4 ADM3222            |
|LL   JP3 ADM3222            |
|----------------------------|
Notes:
      AMD AU1500 - AMD AU1500-333MBD MIPS32-compatible CPU (BGA424)
      K4S643232  - Samsung K4S643232H-UC60 512k x 32bit x 4 banks (64 MBit) Synchronous DRAM LVTTL,
                   3.3V, 222MHz (TSOPII-86)
      ADM3222    - Analog Devices ADM3222 High-Speed, +3.3V, 2-Channel RS232/V.28 Interface Device
                   with 460kBPS Data Rate and Shutdown and Enable Pins (SOIC20)
      24LC04     - Microchip 24LC04B 2x256x8 (4kbit) Serial EEPROM (SOIC8)
      FLASH.IC2  - ST Microelectronics M29W160ET 2M x8 (16 MBit) Flash ROM (TSOP48)
                    - Namco ROM cart version stamped 'FPR24274'
                    - Sega GDROM version stamped 'FPR24036'
      L80277/B   - LSI Logic L80227/B 10BASE-T/100BASE-TX Ethernet Network IC (QFP48)
      070XZ1H    - Sharp 070XZ1H Voltage Regulator
      CN1/CN2    - Connectors joining to Media Board (2nd Board)
      CN7        - RJ45 LAN Port A
      CN4        - DB9 Serial Port
      JP1        - 1-2
      JP2        - 1-2
      JP3        - 2-3
      JP4        - 2-3
      L          - LEDs


Media Boards
------------

Sega GDROM-specific older version without DIMM Board (Top Board)
This version requires a separate standard Sega DIMM Board (same as used on NAOMI)
This was probably used only for the very early titles like Virtua Striker 2002

837-14291 SEGA 2002
171-8209B
            |--------------------------------------|
            |                             LLLLLLLL |
            |    DIP42                             |----------|
            |                      EPC1PC8       JP4  JP2  LL |
            |                                       JP3  JP1  |
|-----------|                                                 |
|                        54MHz                  CN2           |
|                      |-----------|                          |
|    FLASH.IC4         |ALTERA     |                          |
|                      |ACEX       |    CN5S                  |
|                      |EP1K50QC208|                          |
|                      |           |                          |
|      070XZ1H         |-----------|                          |
|                                                             |
|                             32.9MHz                         |
|                                                             |
|CN6S                                                         |
|             CN3                               CN1           |
|-------------------------------------------------------------|
Notes:
      ACEX      - Altera ACEX EP1K50QC208 FPGA (QFP208)
      EPC1PC8   - Altera EPC1PC8 One Time Programmable FPGA Configuration Device (DIP8)
                   - Sticker '315-6343P'
      FLASH.IC4 - ST Microelectronics M29W160ET 2M x8 (16 MBit) Flash ROM (TSOP48)
                   - Stamped 'FPR23910'
      DIP42     - DIP42 location for M27C160 2M x8 (16 MBit) EPROM (not populated)
      CN1/2/3   - Connectors for GDROM DIMM Board
      070XZ1H   - Sharp 070XZ1H Voltage Regulator
      CN5S      - Connector joining to GameCube PCB (located under the PCB)
      CN6S      - Connector joining to JVS Filter Board (located under the PCB)
      JPx       - Jumpers. All positioned to 2-3
      L         - LEDs


Newer Square Metal Box Version (2nd Board down)

837-14355 SEGA 2002 (Sega GDROM version)
837-14356 SEGA 2002 (Namco ROM cart version)
171-8232A (Namco ROM cart version)
171-8231C (Sega GDROM version)
837-14356R91 (sticker for Namco ROM cart version)
837-14355-92 (sticker for Sega GDROM version)
            |----|----------|-----------------------------|
            |    |PIC16C621 |            50HONDA  SD_CARD |
            |    |   4MHz   |    CN6     LED              |---|
            |    |----------|                                 |
            |  DSW  3771                 50MHz                |
|-----------|                                                 |
|LED                                                          |
|LED                       |-----------|                      |
|    FLASH.IC2             |   SEGA    |                      |
|                 DIP42    | 315-6347A |    CN1S         CN3  |
|LED  71V016               |           |                      |
|LED                       |           |                      |
|                          |-----------|                      |
|                                                             |
|                                                             |
|       CN8                                                   |
|       CN9                               070XZ1H             |
|CN2S                                                     CN12|
|-------------------------------------------------------------|
Notes:
      315-6347A - Sega custom IC (BGA). Sega GDROM version uses 315-6347
      71V016    - IDT 71V016SA 64k x 16-bit (1 MBit) 3.3V CMOS Static RAM (TSOP44)
                  This chip is located under the PCB
      FLASH.IC2 - ST Microelectronics M29W160ET 2M x8 (16 MBit) Flash ROM (TSOP48)
                   - Namco ROM cart version stamped 'FPR24331'
                   - Sega GDROM version stamped 'FPR24035'
      DIP42     - DIP42 location for M27C160 2M x8 (16 MBit) EPROM (not populated)
      CN8/CN9   - Connectors joining to network PCB
      070XZ1H   - Sharp 070XZ1H Voltage Regulator
      SD_CARD   - Standard SD Card slot (present only on Namco ROM cart version PCB, unused on Mario Kart)
      50HONDA   - 50-pin mini Honda connector for connection of standard Sega GDROM unit for GDROM-based games only.
                   - This connector is not present on the Namco ROM cart version of this PCB
      CN1S      - Connector joining to GameCube PCB (located under the PCB)
      CN2S      - Connector joining to JVS Filter Board (located under the PCB)
      CN3       - Connector for ROM cartridge.
                   - Replaced by 2x 168-pin DIMM Memory Slots on GDROM-based version
      CN6       - 40 pin IDC flat cable connector (unused)
      CN12      - 3-pin battery connector, present only on Sega GDROM version PCB
      DSW       - 2-position DIP Switch, present only on Sega GDROM version PCB
      PIC16C621 - DIP18 socket and 4MHz OSC on a 90-degrees-mounted small PCB for
                  Microchip PIC16C621A protection key chip

 ? ? ? ? ? ? ? ? ?Usage:
 ? ? ? ? ? ? ? ? ?+-----------------------+-----------------+----------------+
 ? ? ? ? ? ? ? ? ?| ? ? ? ? ? ? ? ? ? ? ? | ? ? ? ? ? ? ? ? | Sega Security ?|
 ? ? ? ? ? ? ? ? ?| Game ? ? ? ? ? ? ? ? ?| Sticker ? ? ? ? | Part Number ? ?|
 ? ? ? ? ? ? ? ? ?+-----------------------+-----------------+----------------+
 ? ? ? ? ? ? ? ? ?| Mario Kart Arcade GP ?| 253-5509-5109 ? | 317-5109-COM ? |
 ? ? ? ? ? ? ? ? ?| Nintendo/Namco, 2005 ?| ? ? ? ? ? ? ? ? | ? ? ? ? ? ? ? ?|
 ? ? ? ? ? ? ? ? ?| ? ? ? ? ? ? ? ? ? ? ? | ? ? ? ? ? ? ? ? | ? ? ? ? ? ? ? ?|
 ? ? ? ? ? ? ? ? ?| Namco Code: MKA2Ver.B | ? ? ? ? ? ? ? ? | ? ? ? ? ? ? ? ?|
 ? ? ? ? ? ? ? ? ?+-----------------------+-----------------+----------------+

ROM Board (attached to CN3 of Media Board, ONLY for use with Namco ROM cart version of the Media Board)
---------

837-14343 SEGA 2002
171-8228D
837-14343R4S0 (sticker)
|---------------|
| FLASH.IC6     |
| *FLASH.8S     |
| FLASH.IC2     |
| *FLASH.4S     |
| PFLASH.IC9    |
| |-------|     |
| |*Actel |     |
| |ProASIC|     |
| |-------|*CN1S|
|LCX138         |
|     50MHz     |
|               |
|               |
| FLASH.IC1     |
| *FLASH.IC3S   |
| FLASH.IC5     |
| *FLASH.IC7S   |
|CN2            |
|---------------|
Notes:
         * - These parts on other side of PCB
 FLASH.ICx - (except IC9) Samsung K9F1208U0B 512MBit (64Mx8) 3.0V NAND Flash ROM (TSOP48)
 FLASH.IC9 - Macronix 29LV400CTTC-70G 4MBit (512k x8 / 256k x16) 3V FLASH ROM (TSOP48)
      CN1S - Connector joining to Media Board (located under the PCB)
       CN2 - 10-pin JTAG Connector for reprogramming the FPGA (all JTAG connectors and required support parts are not populated)
   ProASIC - Actel ProASIC Plus APA075 TQG100 75000 Gate FPGA stamped '315-6419B' (TQFP100)


Main Board (GameCube)
----------

This is a standard Nintendo GameCube mainboard.
Any GameCube mainboard or any version from any of the Triforce units will work.
Note! The hardware specifications are IDENTICAL to any common GameCube. See
http://hitmen.c02.at/files/yagcd/yagcd/frames.html for further detailed documentation.

DOL-CPU-10 (Sega Triforce)
DOL-CPU-20 (Namco Triforce)
838-14297  (Sticker)
|----------------------------------------|
|070XZ1H   MS3M32B    MS3M32B            |
|D4891281                                |
|P10                                     |
|           |-----------|       P9     P5|
|           | FLIPPER A |                |
|           |2001 NINTENDO               |
|P7         |ATI  NEC   |                |
| AVE_N-DOL |D8926F2011 |RTC_AM-DOL    P3|
|           |-----------|   32.768kHz    |
|                                        |
|   AMP-DOL                              |
|P2           |-------|          P6    P4|
|  CLK_B-DOL  |IBM    |                  |
|  D135       |GEKKO  |                  |
|  P1         |-------|         P8       |
|----------------------------------------|
Notes:
      070XZ1H    - Sharp 070XZ1H Voltage Regulator
      D4891281   - NEC D4891281G5 0125XU621 16MB ARAM Auxiliary/DSP DRAM, clocked at 81MHz (SSOP54)
      FLIPPER A  - Custom NEC/ATI Flipper Chip, clocked at 162MHz (large BGA)
                   Contains (something like)....
                             MX92L832 32 Poly Phony Sound Generator
                             MX96037 16bit DSP Controller
                             Custom ATI GFX controller
                             2MB of fast 1T-SRAM memory
      IBM GEKKO  - Custom IBM PowerPC 750-derivative 'Gekko' CPU with FPU extensions clocked at 486MHz (large BGA)
                   Full markings..... IBM
                                      45L8926ESD
                                      IBM9314PQ
                                      (C)IBM 2000
                                      PPCDBK-EFB486X3
                                      J1900LPB  GE
      MS3M32B    - MoSys MS3M32B-5 B 12MB 1-T SRAM, clocked at 162MHz (x2, BGA)
      AVE_N-DOL  - Rohm BU9949FS AV Encoder stamped 'AVE N-DOL' (SSOP32)
      AMP-DOL    - Amplifier chip stamped 'AMP-DOL' (SOIC14)
      RTC_AM-DOL - Macronix 1R7459A1 Real Time Clock, also contains the GameCube BIOS in some kind of on-chip serial EEPROM (SOIC14)
      CLK_B-DOL  - Macronix B021954 CLK B-DOL Clock Generator (SOIC14)
      D135       - Crystal at 13.5MHz. Most likely clocks generated are....
                   Gekko:    13.5 x36 = 486MHz
                   Flipper:  13.5 x12 = 162MHz
                   1-T SRAM: 13.5 x12 = 162MHz
                   16M ARAM: 13.5 x6  = 81MHz
      P1         - Motherboard Power Connector (located under the PCB)
      P2         - Digital Video Output Connector (not used)
      P3         - Controller Pad Board Connector (tied to lower PCB to J8 with a small flat cable)
      P4         - Memory Card Slot Connector A  \ (not used)
      P5         - Memory Card Slot Connector B  /
      P6         - Serial Port Connector 1 (located under the PCB, tied to a plug/cable that leads to J10 on lower PCB)
      P7         - Analog Video Output Connector (tied to a plug/cable that leads to J9 on lower PCB)
      P8         - Serial Port Connector 2 (located under the PCB, not used)
      P9         - Media Port Connector (joins to CN1S on the Media Board)
      P10        - Hi-Speed Parallel Port Connector (located under the PCB, not used)


Base Board
----------
1st Version (older rectangular metal box without built-in DIMM board and newer square metal box version with built in GDROM controller)
8909960107 (8909970107) NAMCO LIMITED 2002 GC-AM BASE PCB

2nd Version (newer square metal box, found in a Mario Kart Triforce unit using ROM carts)
8909961100 (8909971100) NAMCO LIMITED 2002-2003 GC-AM BASE(B) PCB

The differences between the versions seems minor... some component shuffling
and a switch to a pb-Free manufacturing process. The PCB sizes are the same.
However the two boards are not compatible when using the other media board. That is, a Base Board for
GDROM will not work with the top section using ROM carts even though the boards appear identical.
That is because when a key chip is inserted into a new board, it will marry itself with that board
(this is stated in the F-Zero AX manual). From then onwards, the Base Board can only be used for _that_ game.

                           |----------------------------------|
                           |     CR2032            070XZ01    |
                           |      5.5V        J12             |
                           |                          TPS54610|
                           |                                  |
                           |                               J13|
                           |                  ADM3222  ADM3222|
|-----------------|        |                             PC410|
|CY7C1399         |        |J10                               |
|LED     J8       |--------|                                  |
|LED                           |---------------------|        |
|LED |------|  |----|          |                     |    J9  |
|LED |ALTERA|  |CY37128        |                     |        |
|    |EPF10K|  |----|          |                     |        |
|    |------|        33MHz     |                     |CMPV-DOL|
|   BU9480F      IS63LV1024    |---------------------|        |
|                |------|   ADM485        070XZ01    CXA2067AS|
|                | H8S/ |                        |------------|
|    3414        |2676  |   ST16C550             |
|J5              |------|                        |
|---|  J1       J2          J3         J4        |
    |--------------------------------------------|
Notes:
      070XZ01    - Sharp 070XZ01 Voltage Regulator
      EPF10K     - Altera FLEX EPF10K30AQC208 FPGA (QFP208)
      CY37128    - Cypress CY37128VP100 CPLD stamped 'GCABJV1A' (QFP100)
      CY7C1399   - Cypress CY7C1399 32k x8 3.3V Static RAM (SOJ28)
      IS63LV1024 - ISSI IS63LV1024-12KL 128k x8 (1M) High-speed CMOS Static RAM 3.3V (SOJ32)
      H8S/2676   - Hitachi H8S/2676 Microcontroller (QFP144)
      BU9480F    - ROHM BU9480F 16-bit stereo D/A converter (SOIC8)
      3414       - JRC NJM3414A Dual High Current Operational Amplifier (SOIC8)
      ST16C550   - Exar ST16C550 Universal Asynchronous Receiver & Transmitter with 16 byte Transmit & Receive FIFO (TQFP48)
      ADM485     - Analog Devices ADM485 +5 V Low Power Half Duplex EIA RS-485 Transceiver (SOIC8)
      CXA2067AS  - Sony CXA2067AS Video Preamplifier (SDIP30)
      CMPV-DOL   - Macronix B055055G stamped 'CMPV-DOL' (SOP24)
      ADM3222    - Analog Devices ADM3222 High-Speed, +3.3V, 2-Channel RS232/V.28 Interface Device
                   with 460kBPS Data Rate and Shutdown and Enable Pins (SOIC20)
      PC410      - Sharp PC410 Photocoupler (SOIC4)
      CR20332    - 3.0 Volt Coin Battery
      5.5V       - 5.5 Volt 1uF Super Cap
      TPS54610   - Texas Instruments TPS54610 3V-6V Input, 6A Output Synchronous Buck PWM Switcher with Integrated FETs (SSOP28)
      J1         - USB Connector for attachment of a Sega JVS I/O PCB
      J2         - RCA Audio Output
      J3         - 15-pin DSUB Video Output 1
      J4         - 15-pin DSUB Video Output 2
      J5         - 6-pin connector (unused)
      J8         - Flat Cable Connector with Small Flat Cable tied to P3 on Main Board
      J9         - Flat Cable Connector and Plug tied to P7 Analog Video Output Connector on Main Board
      J10        - Flat Cable Connector and Plug tied to P6 Serial Port Connector 1 on Main Board
      J12        - Power Connector tied to P1 Motherboard Power Connector on Main Board


Filter Board
------------
1st Version
8909960503 (8909970503) GC-AM CNT-JV PCB

2nd Version
8909961400 (8909971400) GC-AM CNT-JV(B) PCB

The two versions are identical, the later one is pb-Free
|------------------------------------------------------|
|SW2  SW1     DIPSW              J2              J8    |
|                                                  J9  |
|J3                       J6          J7               |
|                                                      |
|J4    J5                        J1                    |
|------------------------------------------------------|
Notes:
      J1 - 60-pin Connector joins to J13 on Base Board
      J2 - 30-pin Connector joins to CN2S on Media Board
      J3 - 8-pin Connector labelled 'RS232'
      J4 - 7-pin Connector labelled 'MIDI'
      J5 - 10-pin Connector labelled 'SI'
      J6 - 6-pin JVS Power Connector
      J7 - 8-pin JVS Power Connector
      J8 - 3-pin Fan Connector
      J9 - 4-pin Namco Audio Connector (unused)

------------------------------------------------------------------------------------------------------------------

 Note: "Type 3" Triforce uploads GD-ROM firmware to a MIPS processor which is DES encrypted (same as the GD images).
 The key is 0x00 0x22 0x44 0x66 0x88 0xaa 0xcc 0xee (http://debugmo.de/2010/12/the-last-piece/).

*/

#include "emu.h"
#include "cpu/powerpc/ppc.h"
#include "machine/naomigd.h"

static ADDRESS_MAP_START( gc_map, AS_PROGRAM, 32 )
	AM_RANGE(0xffe00000, 0xffffffff) AM_ROM AM_REGION("maincpu", 0) AM_SHARE("share2")	/* Program ROM */
ADDRESS_MAP_END


static VIDEO_START(triforce)
{

}

static SCREEN_UPDATE(triforce)
{
	return 0;
}

static INPUT_PORTS_START( triforce )
INPUT_PORTS_END

static MACHINE_CONFIG_START( triforce_base, driver_device )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PPC403GA, 64000000) /* Correct CPU is a PowerPC 750 (what Apple called "G3") with paired-single vector instructions added */
	MCFG_CPU_PROGRAM_MAP(gc_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 479)
	MCFG_SCREEN_UPDATE(triforce)

	MCFG_PALETTE_LENGTH(65536)

	MCFG_VIDEO_START(triforce)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( triforcegd, triforce_base )
	MCFG_NAOMI_GDROM_BOARD_ADD("rom_board", "gdrom", "picreturn", NULL, "maincpu", NULL)
MACHINE_CONFIG_END

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_BIOS(bios+1)) /* Note '+1' */

#define TRIFORCE_BIOS \
	ROM_REGION( 0x200000, "maincpu", 0) \
	ROM_SYSTEM_BIOS( 0, "bios0", "Triforce Bios" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0,  "triforce_bootrom.bin", 0x000000, 0x200000, CRC(d1883221) SHA1(c3cb7227e4dbc2af861e76d00cb59726105a2e4c) ) \

ROM_START( triforce )
	TRIFORCE_BIOS
ROM_END

ROM_START( vs2002j )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0001", 0, SHA1(1b4b16b0715fa5717904f0b3141cc48cca99b7a4) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("gdt-0001.data", 0x00, 0x50, CRC(4a1fca38) SHA1(3bc6dca4f8faba44bf5c5a8012cffc69dbb6aea2) )
ROM_END

/*
Title   VIRTUA_STRIKER_2002
Media ID    0DD8
Media Config    GD-ROM1/1
Regions J
Peripheral String   0000000
Product Number  GDT-0002
Version V1.005
Release Date    20020730


PIC

253-5508-337E
317-0337-EXP
*/

ROM_START( vs2002ex )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0002", 0, SHA1(471e896d43167c93cc229cfc94ff7ac6de7cf9a4) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("317-0337-exp.data", 0x00, 0x50, CRC(aa6be604) SHA1(fabc43ecfb7ddf1d5a87f10884852027d6f4773b) )
ROM_END


ROM_START( avalons )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0005c", 0, SHA1(9edb3d9ff492d2207d57bfdb6859e796f76c5e0c) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("gdt-0005.data", 0x00, 0x50, CRC(6c51e5d6) SHA1(84afef983f1f855fe8722f55baa8ea5121da9369) )
ROM_END


ROM_START( gekpurya )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0008c", 0, SHA1(2c1bdb8324efc216edd771fe45c680ac726111a0) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("317-0371-jpn.data", 0x00, 0x50, CRC(08434e5e) SHA1(2121999e851f6f62ab845e6de40849d850ac9d1c) )
ROM_END


ROM_START( tfupdate )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0011", 0, SHA1(71bfa8f53d211085c020d54f55eeeabf85212a0b) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("gdt-0011.data", 0x00, 0x50, CRC(08434e5e) SHA1(2121999e851f6f62ab845e6de40849d850ac9d1c) )
ROM_END


/*

Title   VIRTUA STRIKER 4
Media ID    93B2
Media Config    GD-ROM1/1
Regions J
Peripheral String   0000000
Product Number  GDT-0015
Version V1.001
Release Date    20041202
Manufacturer ID
TOC DISC
Track   Start Sector    End Sector  Track Size
track01.bin 150 449 705600
track02.raw 600 1951    3179904
track03.bin 45150   549299  1185760800


PIC
255-5508-393E
317-0393-EXP

*/

ROM_START( vs4 )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0015", 0, SHA1(1f83712b2b170d6edf4a27c15b6f763cc3cc4b71) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("317-0393-exp.data", 0x00, 0x50, CRC(2dcfecd7) SHA1(d805168e1564051ae5c47876ade2c9843253c6b4) )
ROM_END


/*

Title   VIRTUA STRIKER 4
Media ID    7BC9
Media Config    GD-ROM1/1
Regions J
Peripheral String   0000000
Product Number  GDT-0013E
Version V6.000
Release Date    20050217


PIC
255-5508-391J
317-0391-JPN

*/

ROM_START( vs4j )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0013e", 0, SHA1(b69cc5cab889114eda5c6e9ddcca42de9bc235b3) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("317-0391-jpn.data", 0x00, 0x50, CRC(0f2dbb73) SHA1(7b9d66abe85303b3e26b442a3a63feca1a0edbdb) )
ROM_END


/*

Title   TRF GDROM TBA EX SATL
Media ID    92C5
Media Config    GD-ROM1/1
Regions J
Peripheral String   0000000
Product Number  GDT-0010C
Version V4.000
Release Date    20040608
Manufacturer ID

TOC DISC
Track   Start Sector    End Sector  Track Size
track01.bin 150 599 1058400
track02.raw 750 2101    3179904
track03.bin 45150   549299  1185760800
*/

ROM_START( avalon13 )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0010c", 0, SHA1(716c441d8dc9036a13c66ef0048cd6d32ac63c4e) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("gdt-0010c.data", 0x00, 0x50, CRC(6c51e5d6) SHA1(84afef983f1f855fe8722f55baa8ea5121da9369) )
ROM_END


/*

Title   TRF GDROM TBT SATL
Media ID    1348
Media Config    GD-ROM1/1
Regions J
Peripheral String   0000000
Product Number  GDT-0017B
Version V3.001
Release Date    20041102
Manufacturer ID

TOC DISC
Track   Start Sector    End Sector  Track Size
track01.bin 150 599 1058400
track02.raw 750 2101    3179904
track03.bin 45150   549299  1185760800

*/

ROM_START( avalon20 )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0017b", 0, SHA1(e2dd32c322ffcaf38b82275d2721b71bb3dfc1f2) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("gdt-0017b.data", 0x00, 0x50, CRC(32cb46d4) SHA1(a58b9e03d57b317133d9b6c29e42852af8e77559) )
ROM_END


ROM_START( vs42006 )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0020d", 0, SHA1(db256d094b9754d452d7a2b8a370699d21141c1f) )

	ROM_REGION( 0x50, "picreturn", ROMREGION_ERASE)
	ROM_LOAD("gdt-0020.data", 0x00, 0x50, CRC(e3d13191) SHA1(4255c09aad06eb38c16bdec881897404a3a68b37) )
ROM_END



GAME( 2002, triforce, 0,        triforcegd,    triforce,    0, ROT0, "Sega",           "Triforce Bios", GAME_IS_SKELETON|GAME_IS_BIOS_ROOT )
GAME( 2002, vs2002j,  triforce, triforcegd,    triforce,    0, ROT0, "Sega",           "Virtua Striker 2002 (GDT-0001)", GAME_IS_SKELETON )
GAME( 2002, vs2002ex, triforce, triforcegd,    triforce,    0, ROT0, "Sega",           "Virtua Striker 2002 (GDT-0002)", GAME_IS_SKELETON )
GAME( 2003, avalons,  triforce, triforcegd,    triforce,    0, ROT0, "Sega",           "The Key Of Avalon - The Wizard Master - Server (GDT-0005C) (V4.001)", GAME_IS_SKELETON )
GAME( 2003, gekpurya, triforce, triforcegd,    triforce,    0, ROT0, "Sega",           "Gekitou Pro Yakyuu Mizushima Shinji All Stars vs. Pro Yakyuu (Rev C) (GDT-0008C)", GAME_IS_SKELETON )
GAME( 2004, avalon13, triforce, triforcegd,    triforce,    0, ROT0, "Sega",           "The Key Of Avalon 1.3 - Chaotic Sabbat - Client (GDT-0010C) (V4.000)", GAME_IS_SKELETON )
GAME( 2004, tfupdate, triforce, triforcegd,    triforce,    0, ROT0, "Sega",           "Triforce DIMM Updater (GDT-0011)", GAME_IS_SKELETON )
GAME( 2004, vs4j,     triforce, triforcegd,    triforce,    0, ROT0, "Sega",           "Virtua Striker 4 (Japan) (GDT-0013E)", GAME_IS_SKELETON )
GAME( 2004, vs4,      triforce, triforcegd,    triforce,    0, ROT0, "Sega",           "Virtua Striker 4 (Export) (GDT-0015)", GAME_IS_SKELETON )
GAME( 2004, avalon20, triforce, triforcegd,    triforce,    0, ROT0, "Sega",           "The Key Of Avalon 2.0 - Eutaxy and Commandment - Client (GDT-0017B) (V3.001)", GAME_IS_SKELETON )
GAME( 2006, vs42006,  triforce, triforcegd,    triforce,    0, ROT0, "Sega",           "Virtua Striker 4 Ver.2006 (Japan) (Rev D) (GDT-0020D)", GAME_IS_SKELETON )
