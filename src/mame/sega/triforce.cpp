// license:BSD-3-Clause
// copyright-holders:David Haywood, Ville Linde, Segher Boessenkool
/* Triforce Skeleton
 -- split from Naomi.c

Triforce uses:
- a stock Gamecube motherboard with custom BIOS
- a 'media board' which acts as a CD/DVD emulator
- the Naomi 'DIMM' board, which connects to the GD-ROM drive

Sega Triforce Hardware Overview
Sega/Namco/Nintendo 2002-2006
-----------------------------

This is a GameCube-based system containing several PCBs mounted inside a metal box.
There are two versions. One is for GDROM-based games and the other is customised for
use with Namco ROM carts.


Games on this system include....

GD build date
    yyyymmdd   Game                                                                   Manufacturer / Developer              Media   Number           Key Chip       PIC16C621A
+-+----------+----------------------------------------------------------------------+-------------------------------------+-------|----------------+--------------|---------------|
| | 2002     | Starfox Armada (planned, but not released)                           | Namco / Nintendo                    | Cart? |                |              |               |
|*| 20020730 | Virtua Striker 2002 (Japan)                                          | Sega / Amusement Vision             | GDROM | GDT-0001       | 317-0337-JPN |               |
|*| 20020730 | Virtua Striker 2002 (Export)                                         | Sega / Amusement Vision             | GDROM | GDT-0002       | 317-0337-EXP |               |
| | 2003     | F-Zero AX                                                            | Sega / Amusement Vision / Nintendo  | GDROM | GDT-0004       | 317-0362-COM | 253-5508-0362 |
| | 2003     | F-Zero AX (Rev A)                                                    | Sega / Amusement Vision / Nintendo  | GDROM | GDT-0004A      | 317-0362-COM | 253-5508-0362 |
| | 2003     | F-Zero AX (Rev B)                                                    | Sega / Amusement Vision / Nintendo  | GDROM | GDT-0004B      | 317-0362-COM | 253-5508-0362 |
|*| 20030611 | F-Zero AX (Rev C)                                                    | Sega / Amusement Vision / Nintendo  | GDROM | GDT-0004C      | 317-0362-COM | 253-5508-0362 |
|*| 20030628 | F-Zero AX (Rev D)                                                    | Sega / Amusement Vision / Nintendo  | GDROM | GDT-0004D      | 317-0362-COM | 253-5508-0362 |
|*| 20031203 | F-Zero AX (Rev E)                                                    | Sega / Amusement Vision / Nintendo  | GDROM | GDT-0004E      | 317-0362-COM | 253-5508-0362 |
| | 2003     | The Key Of Avalon: The Wizard Master (server)                        | Sega / Hitmaker                     | GDROM | GDT-0005       | 317-0355-JPN | 253-5508-0355J|
|*| 20030706 | The Key Of Avalon: The Wizard Master (server, Rev A)                 | Sega / Hitmaker                     | GDROM | GDT-0005A      | 317-0355-JPN | 253-5508-0355J|
| | 200307   | The Key Of Avalon: The Wizard Master (server, Rev B)                 | Sega / Hitmaker                     | GDROM | GDT-0005B      | 317-0355-JPN | 253-5508-0355J|
|*| 20030721 | The Key Of Avalon: The Wizard Master (server, Rev C)                 | Sega / Hitmaker                     | GDROM | GDT-0005C      | 317-0355-JPN | 253-5508-0355J|
| | 2003     | The Key Of Avalon: The Wizard Master (server, Rev D)                 | Sega / Hitmaker                     | GDROM | GDT-0005D      | 317-0355-JPN | 253-5508-0355J|
|*| 20030815 | The Key Of Avalon: The Wizard Master (server, Rev E)                 | Sega / Hitmaker                     | GDROM | GDT-0005E      | 317-0355-JPN | 253-5508-0355J|
|*| 20030924 | The Key Of Avalon: The Wizard Master (server, Rev F)                 | Sega / Hitmaker                     | GDROM | GDT-0005F      | 317-0355-JPN | 253-5508-0355J|
|*| 20031125 | The Key Of Avalon: The Wizard Master (server, Rev G)                 | Sega / Hitmaker                     | GDROM | GDT-0005G      | 317-0355-JPN | 253-5508-0355J|
| | 2003     | The Key Of Avalon: The Wizard Master (client)                        | Sega / Hitmaker                     | GDROM | GDT-0006       | 317-0355-JPN | 253-5508-0355J|
|*| 20030706 | The Key Of Avalon: The Wizard Master (client, Rev A)                 | Sega / Hitmaker                     | GDROM | GDT-0006A      | 317-0355-JPN | 253-5508-0355J|
| | 200307   | The Key Of Avalon: The Wizard Master (client, Rev B)                 | Sega / Hitmaker                     | GDROM | GDT-0006B      | 317-0355-JPN | 253-5508-0355J|
|*| 20030721 | The Key Of Avalon: The Wizard Master (client, Rev C)                 | Sega / Hitmaker                     | GDROM | GDT-0006C      | 317-0355-JPN | 253-5508-0355J|
| | 2003     | The Key Of Avalon: The Wizard Master (client, Rev D)                 | Sega / Hitmaker                     | GDROM | GDT-0006D      | 317-0355-JPN | 253-5508-0355J|
|*| 20030815 | The Key Of Avalon: The Wizard Master (client, Rev E)                 | Sega / Hitmaker                     | GDROM | GDT-0006E      | 317-0355-JPN | 253-5508-0355J|
|*| 20030924 | The Key Of Avalon: The Wizard Master (client, Rev F)                 | Sega / Hitmaker                     | GDROM | GDT-0006F      | 317-0355-JPN | 253-5508-0355J|
|*| 20031125 | The Key Of Avalon: The Wizard Master (client, Rev G)                 | Sega / Hitmaker                     | GDROM | GDT-0006G      | 317-0355-JPN | 253-5508-0355J|
| | 2003     | F-Zero AX (Asia)                                                     | Sega / Amusement Vision / Nintendo  | GDROM | GDT-0007       |              |               |
| | 2003     | Gekitou Pro Yakyuu Mizushima Shinji All Stars vs. Pro Yakyuu         | Sega / Wow Entertainment            | GDROM | GDT-0008       | 317-0371-JPN | 253-5508-0371J|
| | 2003     | Gekitou Pro Yakyuu Mizushima Shinji All Stars vs. Pro Yakyuu (Rev A) | Sega / Wow Entertainment            | GDROM | GDT-0008A      | 317-0371-JPN | 253-5508-0371J|
|*| 20031010 | Gekitou Pro Yakyuu Mizushima Shinji All Stars vs. Pro Yakyuu (Rev B) | Sega / Wow Entertainment            | GDROM | GDT-0008B      | 317-0371-JPN | 253-5508-0371J|
|*| 20031027 | Gekitou Pro Yakyuu Mizushima Shinji All Stars vs. Pro Yakyuu (Rev C) | Sega / Wow Entertainment            | GDROM | GDT-0008C      | 317-0371-JPN | 253-5508-0371J|
| | 2003     | The Key Of Avalon 1.10                                               | Sega / Hitmaker                     | GDROM |                | 317-0355-JPN | 253-5508-0355J|
| | 2004     | The Key Of Avalon 1.??: ??? (server)                                 | Sega / Hitmaker                     | GDROM | GDT-0009       | 317-0355-JPN | 253-5508-0355J|
|*| 2004     | The Key Of Avalon 1.20: Summon The New Monster (server, Rev A)       | Sega / Hitmaker                     | GDROM | GDT-0009A      | 317-0355-JPN | 253-5508-0355J|
| | 2004     | The Key Of Avalon 1.??: ??? (server, Rev B)                          | Sega / Hitmaker                     | GDROM | GDT-0009B      | 317-0355-JPN | 253-5508-0355J|
|*| 20040608 | The Key Of Avalon 1.30: Chaotic Sabbat (server, Rev C)               | Sega / Hitmaker                     | GDROM | GDT-0009C      | 317-0355-JPN | 253-5508-0355J|
| | 2004     | The Key Of Avalon 1.??: ??? (client)                                 | Sega / Hitmaker                     | GDROM | GDT-0010       | 317-0355-JPN | 253-5508-0355J|
|*| 2004     | The Key Of Avalon 1.20: Summon The New Monster (client, Rev A)       | Sega / Hitmaker                     | GDROM | GDT-0010A      | 317-0355-JPN | 253-5508-0355J|
| | 2004     | The Key Of Avalon 1.??: ??? (client, Rev B)                          | Sega / Hitmaker                     | GDROM | GDT-0010B      | 317-0355-JPN | 253-5508-0355J|
|*| 20040608 | The Key Of Avalon 1.30: Chaotic Sabbat (client, Rev C)               | Sega / Hitmaker                     | GDROM | GDT-0010C      | 317-0355-JPN | 253-5508-0355J|
|*| 20031110 | Firmware Update                                                      | Sega                                | GDROM | GDT-0011       | 317-0371-JPN |               |
|*| 20040315 | Virtua Striker 2002 (Type 3)                                         | Sega                                | GDROM | GDT-0012       | 317-0337-EXP |               |
| | 2004     | Virtua Striker 4 (Japan)                                             | Sega                                | GDROM | GDT-0013       | 317-0391-JPN |               |
| | 2004     | Virtua Striker 4 (Japan, Rev A)                                      | Sega                                | GDROM | GDT-0013A      | 317-0391-JPN |               |
| | 2004     | Virtua Striker 4 (Japan, Rev B)                                      | Sega                                | GDROM | GDT-0013B      | 317-0391-JPN |               |
|*| 20040913 | Virtua Striker 4 (Japan, Rev C)                                      | Sega                                | GDROM | GDT-0013C      | 317-0391-JPN |               |
| | 2004     | Virtua Striker 4 (Japan, Rev D)                                      | Sega                                | GDROM | GDT-0013D      | 317-0391-JPN |               |
|*| 20050217 | Virtua Striker 4 (Japan, Rev E)                                      | Sega                                | GDROM | GDT-0013E      | 317-0391-JPN |               |
|*| 20041203 | Virtua Striker 4 (Asia)                                              | Sega                                | GDROM | GDT-0014       | 317-0392-EXP | 253-5508-0392E|
| | 2004     | Virtua Striker 4 (Asia, Rev A)                                       | Sega                                | GDROM | GDT-0014A      | 317-0392-EXP | 253-5508-0392E|
|*| 20050523 | Virtua Striker 4 (Asia, Rev B)                                       | Sega                                | GDROM | GDT-0014B      | 317-0392-EXP | 253-5508-0392E|
|*| 20041202 | Virtua Striker 4 (Export)                                            | Sega                                | GDROM | GDT-0015       | 317-0393-EXP |               |
|*| 20050411 | Virtua Striker 4 (Export, Rev A)                                     | Sega                                | GDROM | GDT-0015A      | 317-0393-EXP |               |
| | 20041202 | Virtua Striker 4 (Export)                                            | Sega                                | CF    | MDA-G0016      | 317-0393-EXP |               |
| | 2004     | The Key Of Avalon 2: Eutaxy Commandment (server)                     | Sega / Hitmaker                     | GDROM | GDT-0016       | 317-0403-JPN | 253-5508-0403J|
| | 2004     | The Key Of Avalon 2: Eutaxy Commandment (client)                     | Sega / Hitmaker                     | GDROM | GDT-0017       | 317-0403-JPN | 253-5508-0403J|
| | 2004     | The Key Of Avalon 2: Eutaxy Commandment (client, Rev A)              | Sega / Hitmaker                     | GDROM | GDT-0017A      | 317-0403-JPN | 253-5508-0403J|
|*| 20041102 | The Key Of Avalon 2: Eutaxy Commandment (client, Rev B)              | Sega / Hitmaker                     | GDROM | GDT-0017B      | 317-0403-JPN | 253-5508-0403J|
|*| 2003     | F-Zero AX - Monster Ride Cycraft Edition                             | Sega / Amusement Vision / Nintendo  | Cart  | 837-14343-4S1  | 317-0373-COM | 253-5508-0373 |
|*| 2005     | Mario Kart Arcade GP (Japan, MKA1 Ver.A1)                            | Namco / Nintendo                    | Cart  | 837-14343-4T1  | 317-5109-COM | 253-5509-5109 |
| | 2005     | The Key Of Avalon 2.5: War of the Key (server)                       | Sega / Hitmaker                     | GDROM | GDT-0018       | 317-0403-JPN | 253-5508-0403J|
|*| 20050518 | The Key Of Avalon 2.5: War of the Key (server, Rev A)                | Sega / Hitmaker                     | GDROM | GDT-0018A      | 317-0403-JPN | 253-5508-0403J|
|*| 20050805 | The Key Of Avalon 2.5: War of the Key (server, Rev B)                | Sega / Hitmaker                     | GDROM | GDT-0018B      | 317-0403-JPN | 253-5508-0403J|
| | 2005     | The Key Of Avalon 2.5: War of the Key (client)                       | Sega / Hitmaker                     | GDROM | GDT-0019       | 317-0403-JPN | 253-5508-0403J|
|*| 20050518 | The Key Of Avalon 2.5: War of the Key (client, Rev A)                | Sega / Hitmaker                     | GDROM | GDT-0019A      | 317-0403-JPN | 253-5508-0403J|
|*| 20050805 | The Key Of Avalon 2.5: War of the Key (client, Rev B)                | Sega / Hitmaker                     | GDROM | GDT-0019B      | 317-0403-JPN | 253-5508-0403J|
| | 2006     | Virtua Striker 4 Ver.2006 (Japan)                                    | Sega                                | GDROM | GDT-0020       | 317-0432-JPN | 253-5508-0432J|
| | 2006     | Virtua Striker 4 Ver.2006 (Japan, Rev A)                             | Sega                                | GDROM | GDT-0020A      | 317-0432-JPN | 253-5508-0432J|
|*| 20060425 | Virtua Striker 4 Ver.2006 (Japan, Rev B)                             | Sega                                | GDROM | GDT-0020B      | 317-0432-JPN | 253-5508-0432J|
| | 2006     | Virtua Striker 4 Ver.2006 (Japan, Rev C)                             | Sega                                | GDROM | GDT-0020C      | 317-0432-JPN | 253-5508-0432J|
|*| 20061019 | Virtua Striker 4 Ver.2006 (Japan, Rev D)                             | Sega                                | GDROM | GDT-0020D      | 317-0432-JPN | 253-5508-0432J|
|*| 20060131 | Virtua Striker 4 Ver.2006 (Export)                                   | Sega                                | GDROM | GDT-0021       | 317-0433-EXP | 253-5508-0433E|
| | 2006     | Virtua Striker 4 Ver.2006 (Export)                                   | Sega                                | CF    | MDA-G00??      | 317-0433-EXP | 253-5508-0433E|
| | 2006     | Triforce Firmware Update for Compact Flash Box                       | Sega                                | GDROM | GDT-0022       | 317-0567-COM |               |
|*| 20090331 | Triforce Firmware Update for Compact Flash Box (Rev A)               | Sega                                | GDROM | GDT-0022A      | 317-0567-COM |               |
|*| 2007     | Mario Kart Arcade GP 2 (Japan, MK21 Ver.A)                           | Namco / Nintendo                    | Cart  | 837-14343-R4S0 | 317-5128-COM | 253-5509-5128 |
|*| 2007     | Mario Kart Arcade GP 2 (Japan, MK21 Ver.A, alt dump)                 | Namco / Nintendo                    | Cart  | 837-14343-R4S0 | 317-5128-COM | 253-5509-5128 |
+-+----------+----------------------------------------------------------------------+-------------------------------------+-------|----------------+--------------+---------------|
* denotes these games are archived.
! security PIC is not dumped

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
      SD_CARD   - Standard SD Card slot (present only on Namco ROM cart version PCB, used by Mario Kart game updates, and presumably for NAND cart initial programming)
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

ROM Board (attached to CN3 of Media Board, ONLY for use with Namco ROM cart version of the Media Board)
---------

837-14343 SEGA 2002
171-8228D
MDK332V-0
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
 FLASH.ICx - (except IC9) Samsung K9F1208U0B 512Mbit (64Mx8) or Toshiba TC58DVG02A1FT00 1Gbit (128Mx8), both 3.0V NAND Flash ROM (TSOP48)
 FLASH.IC9 - Macronix 29LV400CTTC-70G 4MBit (512k x8 / 256k x16) 3V FLASH ROM (TSOP48)
      CN1S - Connector joining to Media Board (located under the PCB)
       CN2 - 10-pin JTAG Connector for reprogramming the FPGA (all JTAG connectors and required support parts are not populated)
   ProASIC - Actel ProASIC Plus APA075 TQG100 75000 Gate FPGA stamped '315-xxxx' (TQFP100)


Games known to use this PCB include....
                                                              NANDROMs   FLASH    APA075     sticker
Game                                                          # TSOP48   IC9#     IC11S#     837-14343  Notes
--------------------------------------------------------------------------------------------------------------------------------------------------
F-Zero AX Monster Ride                                        4 (1Gb)    present  315-6362   -4S1       Cart: 842-06CN4000
Mario Kart Arcade Grand Prix (Japan, MKA1 Ver.A1)             4 (1Gb)    present  315-6362   -4T1       requires V257 STR PCB steering wheel board
Mario Kart Arcade Grand Prix 2 (Japan, MK21 Ver.A)            8 (512Mb)  present  315-6419B  -R4S0
Mario Kart Arcade Grand Prix 2 (Japan, MK21 Ver.A, alt dump)  8 (512Mb)  present  315-6419B  -R4S0


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
#include "naomigd.h"
#include "emupal.h"
#include "screen.h"


namespace {

class triforce_state : public driver_device
{
public:
	triforce_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu") { }

	void triforcegd(machine_config &config);
	void triforce_base(machine_config &config);

private:
	uint64_t gc_pi_r();
	void gc_pi_w(uint64_t data);
	uint64_t gc_exi_r();
	void gc_exi_w(uint64_t data);
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_triforce(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	required_device<ppc_device> m_maincpu;
	void gc_map(address_map &map) ATTR_COLD;
};

uint64_t triforce_state::gc_pi_r()
{
	return 0;
}

void triforce_state::gc_pi_w(uint64_t data)
{
}

uint64_t triforce_state::gc_exi_r()
{
	return 0;
}

void triforce_state::gc_exi_w(uint64_t data)
{
}

void triforce_state::gc_map(address_map &map)
{
	map(0x00000000, 0x017fffff).ram();
	map(0x0c003000, 0x0c003fff).rw(FUNC(triforce_state::gc_pi_r), FUNC(triforce_state::gc_pi_w));
	map(0x0c006800, 0x0c0068ff).rw(FUNC(triforce_state::gc_exi_r), FUNC(triforce_state::gc_exi_w));
	map(0xfff00000, 0xffffffff).rom().region("maincpu", 0);  /* Program ROM */
}


void triforce_state::video_start()
{
}

uint32_t triforce_state::screen_update_triforce(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static INPUT_PORTS_START( triforce )
INPUT_PORTS_END

// bootrom descrambler reversed by segher
// Copyright 2008-2017 Segher Boessenkool <segher@kernel.crashing.org>
static void descrambler(uint8_t* data, uint32_t size)
{
	uint8_t acc = 0;
	uint8_t nacc = 0;

	uint16_t t = 0x2953;
	uint16_t u = 0xd9c2;
	uint16_t v = 0x3ff1;

	uint8_t x = 1;

	for (uint32_t it = 0; it < size;)
	{
		int t0 = t & 1;
		int t1 = (t >> 1) & 1;
		int u0 = u & 1;
		int u1 = (u >> 1) & 1;
		int v0 = v & 1;

		x ^= t1 ^ v0;
		x ^= (u0 | u1);
		x ^= (t0 ^ u1 ^ v0) & (t0 ^ u0);

		if (t0 == u0)
		{
			v >>= 1;
			if (v0)
				v ^= 0xb3d0;
		}

		if (t0 == 0)
		{
			u >>= 1;
			if (u0)
				u ^= 0xfb10;
		}

		t >>= 1;
		if (t0)
			t ^= 0xa740;

		nacc++;
		acc = 2*acc + x;
		if (nacc == 8)
		{
			data[BYTE8_XOR_BE(it)] ^= acc;
			it++;
			nacc = 0;
		}
	}
}

void triforce_state::machine_start()
{
	/* set conservative DRC options */
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS);

	uint8_t *rom = (uint8_t*)memregion("maincpu")->base();
	descrambler(&rom[0x100], 0x1afe00);
}

void triforce_state::triforce_base(machine_config &config)
{
	/* basic machine hardware */
	PPC603(config, m_maincpu, 64000000); /* Correct CPU is a PowerPC 750 (what Apple called "G3") with paired-single vector instructions added */
	m_maincpu->set_addrmap(AS_PROGRAM, &triforce_state::gc_map);

	config.set_maximum_quantum(attotime::from_hz(6000));


	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(640, 480);
	screen.set_visarea(0, 639, 0, 479);
	screen.set_screen_update(FUNC(triforce_state::screen_update_triforce));

	PALETTE(config, "palette").set_entries(65536);
}

void triforce_state::triforcegd(machine_config &config)
{
	triforce_base(config);
	naomi_gdrom_board &rom_board(NAOMI_GDROM_BOARD(config, "rom_board", 0, ":gdrom", "pic"));
	rom_board.irq_callback().set_nop();
}


#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_BIOS(bios))

#define TRIFORCE_BIOS \
	ROM_REGION( 0x200000, "maincpu", 0) \
	ROM_SYSTEM_BIOS( 0, "bios0", "Triforce BIOS" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0,  "triforce_bootrom.bin", 0x000000, 0x200000, CRC(d1883221) SHA1(c3cb7227e4dbc2af861e76d00cb59726105a2e4c) )
ROM_START( triforce )
	TRIFORCE_BIOS
ROM_END

ROM_START( vs2002j )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0001", 0, SHA1(6c513f45561ccb8a76a7dbef38de7dbae83d6925) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0337-jpn.data", 0x00, 0x50, CRC(4a1fca38) SHA1(3bc6dca4f8faba44bf5c5a8012cffc69dbb6aea2) )
ROM_END

/*
Title             VIRTUA_STRIKER_2002
Media ID          0DD8
Media Config      GD-ROM1/1
Regions           J
Peripheral String 0000000
Product Number    GDT-0002
Version           V1.005
Release Date      20020730

PIC
253-5508-337E
317-0337-EXP
*/
ROM_START( vs2002ex )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0002", 0, SHA1(e7bd72799b26900db77a545e0823c2dd607c5bd1) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0337-exp.data", 0x00, 0x50, CRC(aa6be604) SHA1(fabc43ecfb7ddf1d5a87f10884852027d6f4773b) )
ROM_END

/*
Title             F-ZERO AX
Media ID          6BB7
Media Config      GD-ROM1/1
Regions           J
Peripheral String 0000000
Product Number    GDT-0004E
Version           V5.002
Release Date      20031203

PIC16C621A-20/P (317-0362-COM)
Sticker: 253-5508-0362
*/
ROM_START( fzeroax )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0004e", 0, SHA1(9910d321690d8e5c1927a558c23a0b3394061e4d) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASE)
	ROM_LOAD( "317-0362-com.pic", 0x000000, 0x004000, CRC(23d83347) SHA1(ca5a629e7405710fd4fa0af7c668192bdf8ca725) )
ROM_END

/*
Title             F-ZERO AX
Media ID          A677
Media Config      GD-ROM1/1
Regions           J
Peripheral String 0000000
Product Number    GDT-0004D
Version           V4.000
Release Date      20030628

PIC16C621A-20/P (317-0362-COM)
Sticker: 253-5508-0362
*/
ROM_START( fzeroaxd )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0004d", 0, SHA1(e98498809485d0469098792e9a77aafe37c7086c) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASE)
	ROM_LOAD( "317-0362-com.pic", 0x000000, 0x004000, CRC(23d83347) SHA1(ca5a629e7405710fd4fa0af7c668192bdf8ca725) )
ROM_END

/*
Title             F-ZERO AX
Media ID          06BB
Media Config      GD-ROM1/1
Regions           J
Peripheral String 0000000
Product Number    GDT-0004C
Version           V3.000
Release Date      20030611

PIC16C621A-20/P (317-0362-COM)
Sticker: 253-5508-0362
*/
ROM_START( fzeroaxc )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0004c", 0, SHA1(f51691a26745afc36110e5d433eb216f108a43d4) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASE)
	ROM_LOAD( "317-0362-com.pic", 0x000000, 0x004000, CRC(23d83347) SHA1(ca5a629e7405710fd4fa0af7c668192bdf8ca725) )
ROM_END

ROM_START( avalonsa )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0005a", 0, SHA1(807ecf820e8054978dd637b413053dc3279c4bf4) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0355-jpn.data", 0x00, 0x50, CRC(9b9db9d8) SHA1(60aab36fcfd7a2850b2947effea70e44a06d1f78) )
ROM_END

ROM_START( avalonsc )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0005c", 0, SHA1(37b848ed131e8357ba57474f7c2aa6ab51b01781) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0355-jpn.data", 0x00, 0x50, CRC(9b9db9d8) SHA1(60aab36fcfd7a2850b2947effea70e44a06d1f78) )
ROM_END

ROM_START( avalonse )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0005e", 0, SHA1(a888a045c323f374b53295404262a3bb80a533c0) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0355-jpn.data", 0x00, 0x50, CRC(9b9db9d8) SHA1(60aab36fcfd7a2850b2947effea70e44a06d1f78) )
ROM_END

ROM_START( avalonsf )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0005f", 0, SHA1(3752e406a5542a912f8b1ef48096ed30e7d28279) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0355-jpn.data", 0x00, 0x50, CRC(9b9db9d8) SHA1(60aab36fcfd7a2850b2947effea70e44a06d1f78) )
ROM_END

ROM_START( avalons )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0005g", 0, SHA1(f2dca7ecd6c07ff098ac91e353ffc3fd843054e3) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0355-jpn.data", 0x00, 0x50, CRC(9b9db9d8) SHA1(60aab36fcfd7a2850b2947effea70e44a06d1f78) )
ROM_END

ROM_START( avalonca )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0006a", 0, SHA1(cef39bcc6351d597474674699a193fa115a51447) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0355-jpn.data", 0x00, 0x50, CRC(9b9db9d8) SHA1(60aab36fcfd7a2850b2947effea70e44a06d1f78) )
ROM_END

ROM_START( avaloncc )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0006c", 0, SHA1(be761300165b152e2872750eea98ab3b894bddde) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0355-jpn.data", 0x00, 0x50, CRC(9b9db9d8) SHA1(60aab36fcfd7a2850b2947effea70e44a06d1f78) )
ROM_END

ROM_START( avalonce )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0006e", 0, SHA1(59aeb7d767d390b3ee7f079d3d1a31df3e92773b) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0355-jpn.data", 0x00, 0x50, CRC(9b9db9d8) SHA1(60aab36fcfd7a2850b2947effea70e44a06d1f78) )
ROM_END

ROM_START( avaloncf )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0006f", 0, SHA1(35c98e41095273326ac0c6fe633c3f6e8b328f63) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0355-jpn.data", 0x00, 0x50, CRC(9b9db9d8) SHA1(60aab36fcfd7a2850b2947effea70e44a06d1f78) )
ROM_END

ROM_START( avalonc )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0006g", 0, SHA1(96e1db3f395152a62d0b344c350f38306ab1e0ae) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0355-jpn.data", 0x00, 0x50, CRC(9b9db9d8) SHA1(60aab36fcfd7a2850b2947effea70e44a06d1f78) )
ROM_END

ROM_START( gekpuryb )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0008b", 0, SHA1(ecbf8589fc973bf121b53c3c27122a7c6ff22d8d) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0371-jpn.data", 0x00, 0x50, CRC(08434e5e) SHA1(2121999e851f6f62ab845e6de40849d850ac9d1c) )
ROM_END

ROM_START( gekpurya )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0008c", 0, SHA1(722e2e2702a2f55b93cdc8b2fac3f920fbc81977) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0371-jpn.data", 0x00, 0x50, CRC(08434e5e) SHA1(2121999e851f6f62ab845e6de40849d850ac9d1c) )
ROM_END

ROM_START( avalns12 )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0009a", 0, SHA1(abeb242cb7f55d003eb7ca6d42dccdb5d8080bf1) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0355-jpn.data", 0x00, 0x50, CRC(9b9db9d8) SHA1(60aab36fcfd7a2850b2947effea70e44a06d1f78) )
ROM_END

ROM_START( avalns13 )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0009c", 0, SHA1(b960815d85e255521fc26bd4a9e4b7e5c4469487) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0355-jpn.data", 0x00, 0x50, CRC(9b9db9d8) SHA1(60aab36fcfd7a2850b2947effea70e44a06d1f78) )
ROM_END

ROM_START( avalnc12 )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0010a", 0, SHA1(a82f803a2097890c29e44ef30d97ad1fcf550435) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0355-jpn.data", 0x00, 0x50, CRC(9b9db9d8) SHA1(60aab36fcfd7a2850b2947effea70e44a06d1f78) )
ROM_END

/*
Title             TRF GDROM TBA EX SATL
Media ID          92C5
Media Config      GD-ROM1/1
Regions           J
Peripheral String 0000000
Product Number    GDT-0010C
Version           V4.000
Release Date      20040608
Manufacturer ID

TOC DISC
Track       Start Sector  End Sector  Track Size
track01.bin          150         599     1058400
track02.raw          750        2101     3179904
track03.bin        45150      549299  1185760800
*/
ROM_START( avalnc13 )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0010c", 0, SHA1(c7a3fdb467ccd03b5b9b9e8e290faabcbbc77546) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0355-jpn.data", 0x00, 0x50, CRC(9b9db9d8) SHA1(60aab36fcfd7a2850b2947effea70e44a06d1f78) )
ROM_END

ROM_START( tfupdate )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0011", 0, SHA1(fbfd7960ca7070721e25c08ccc428a2e0cf4766f) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0371-jpn.data", 0x00, 0x50, CRC(08434e5e) SHA1(2121999e851f6f62ab845e6de40849d850ac9d1c) )
ROM_END

ROM_START( vs2002t3 )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0012", 0, SHA1(d533b99edea5814d446e80744166f3f1630c9d98) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0337-exp.data", 0x00, 0x50, CRC(aa6be604) SHA1(fabc43ecfb7ddf1d5a87f10884852027d6f4773b) )
ROM_END

ROM_START( vs4jc )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0013c", 0, SHA1(b6483c83973bacb8b66bbeff4c26ae37b2b28ca9) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0391-jpn.data", 0x00, 0x50, CRC(0f2dbb73) SHA1(7b9d66abe85303b3e26b442a3a63feca1a0edbdb) )
ROM_END

/*
Title             VIRTUA STRIKER 4
Media ID          7BC9
Media Config      GD-ROM1/1
Regions           J
Peripheral String 0000000
Product Number    GDT-0013E
Version           V6.000
Release Date      20050217

PIC
255-5508-391J
317-0391-JPN
*/
ROM_START( vs4j )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0013e", 0, SHA1(8f839d3ade4019131f18bb327e69087ed52e1cc4) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0391-jpn.data", 0x00, 0x50, CRC(0f2dbb73) SHA1(7b9d66abe85303b3e26b442a3a63feca1a0edbdb) )
ROM_END

ROM_START( vs4eo )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0014", 0, SHA1(e7f7304b1c9075f37b8fed4e8d3f22f7fc0ae5ab) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD("317-0392-exp.pic", 0x00, 0x4000, CRC(c8931261) SHA1(063faea92a987a5b3dc96455494cf5fc13052428) )
ROM_END

ROM_START( vs4e )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0014b", 0, SHA1(5d038a76a101fafcab5a514d3911f727becd7091) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD("317-0392-exp.pic", 0x00, 0x4000, CRC(c8931261) SHA1(063faea92a987a5b3dc96455494cf5fc13052428) )
ROM_END

/*
Title             VIRTUA STRIKER 4
Media ID          93B2
Media Config      GD-ROM1/1
Regions           J
Peripheral String 0000000
Product Number    GDT-0015
Version           V1.001
Release Date      20041202
Manufacturer ID
TOC DISC
Track        Start Sector  End Sector  Track Size
track01.bin           150         449      705600
track02.raw           600        1951     3179904
track03.bin         45150      549299  1185760800

PIC
255-5508-393E
317-0393-EXP
*/
ROM_START( vs4o )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0015", 0, SHA1(3fa43fdab579b657cb8aeebb315d98cf0d24f921) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0393-exp.data", 0x00, 0x50, CRC(2dcfecd7) SHA1(d805168e1564051ae5c47876ade2c9843253c6b4) )
ROM_END

ROM_START( vs4 )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0015a", 0, SHA1(471cdaa3a14c37249f2dd47f6a661e325dd1cf8f) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0393-exp.data", 0x00, 0x50, CRC(2dcfecd7) SHA1(d805168e1564051ae5c47876ade2c9843253c6b4) )
ROM_END

/*
Title             TRF GDROM TBT SATL
Media ID          1348
Media Config      GD-ROM1/1
Regions           J
Peripheral String 0000000
Product Number    GDT-0017B
Version           V3.001
Release Date      20041102
Manufacturer ID

TOC DISC
Track        Start Sector  End Sector  Track Size
track01.bin           150         599     1058400
track02.raw           750        2101     3179904
track03.bin         45150      549299  1185760800
*/
ROM_START( avalon20 )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0017b", 0, SHA1(22827a78415c967c9b6abff5aa1037e01785a7b3) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0403-jpn.data", 0x00, 0x50, CRC(c5071ada) SHA1(70018180b917beec1b2c272f9125fa43fd1a7b00) )
ROM_END

ROM_START( avalns25a )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0018a", 0, SHA1(548aecb062009e62c0bf97bd6b575b9ac205751f) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0403-jpn.data", 0x00, 0x50, CRC(c5071ada) SHA1(70018180b917beec1b2c272f9125fa43fd1a7b00) )
ROM_END

ROM_START( avalns25 )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0018b", 0, SHA1(4cd10813e66b72021ca344886638025f979d40c0) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0403-jpn.data", 0x00, 0x50, CRC(c5071ada) SHA1(70018180b917beec1b2c272f9125fa43fd1a7b00) )
ROM_END

ROM_START( avalnc25a )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0019a", 0, SHA1(1784a659027a4798ec4332063bd145afcecbda0e) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0403-jpn.data", 0x00, 0x50, CRC(c5071ada) SHA1(70018180b917beec1b2c272f9125fa43fd1a7b00) )
ROM_END

ROM_START( avalnc25 )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0019b", 0, SHA1(7f978ec998a86ed9c36a495bfa72073c6d2bbe31) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0403-jpn.data", 0x00, 0x50, CRC(c5071ada) SHA1(70018180b917beec1b2c272f9125fa43fd1a7b00) )
ROM_END

ROM_START( vs42006b )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0020b", 0, SHA1(ac875c5d8da72e75186790acf839e775050d3591) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0432-jpn.data", 0x00, 0x50, CRC(e3d13191) SHA1(4255c09aad06eb38c16bdec881897404a3a68b37) )
ROM_END

ROM_START( vs42006 )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0020d", 0, SHA1(09000f5b6ef680c603a16ea8028414aec0cc05f2) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0432-jpn.data", 0x00, 0x50, CRC(e3d13191) SHA1(4255c09aad06eb38c16bdec881897404a3a68b37) )
ROM_END

/*
Title             VIRTUA STRIKER 4 VER.2006
Media ID          9ED0
Media Config      GD-ROM1/1
Regions           J
Peripheral String 0000000
Product Number    GDT-0021
Version           V1.003
Release Date      20060131
Manufacturer ID
Ring Code

PIC
253-5508-0433E
317-0433-EXP
*/
ROM_START( vs42k6ex )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0021", 0, SHA1(fa1511d3a0f7755df77ec535f78beff0c5a7fcbe) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD("317-0433-exp.pic", 0x00, 0x4000, CRC(9e52aba5) SHA1(40eff0ed8c801644849bbadada871f2abb1d95a0) )
ROM_END

/*
Title          BOX GDROM CF-BOX FIRM
Product Number GDT-0022A
Hardware       Tri-Force
Sec Key        253-5508-0567
Pic            317-0567-COM
Ver            0001
*/
ROM_START( tcfboxa )
	TRIFORCE_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdt-0022a", 0, SHA1(14973058d87eff93782f59878ec856a7be994b6e) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD("317-0567-com.pic", 0x00, 0x4000, CRC(cd1d2b2d) SHA1(78203ee0339f76eb76da08d7de43e7e44e4b7d32) )
ROM_END


// the following triforce games use a flash board instead of a gdrom
// the flash board contains a 29lv400t chip used as a block index, whose format works as follows:
// * 0x00000-0x77fff is an index of 16-bit little endian offsets, in "sectors" (see below)
//   into the flash area, each corresponding to where to look for the data for sector n, where n is the
//   sector of data being looked for by the host system. Unused sectors past the end of the data have their
//   index set to FF FF.
// * 0x78000-0x78007 contains what seems to be two 32-bit checksums, this isn't clear.
// * 0x78020-0x7802f contains what seems to be a list of 16 error counts (possibly corresponding to d0 thru d15)
//   and written during the 'formatting/copying' portion of creating the flash board for a given game.
//   For instance, the dump of mkartagp skips two blocks in its lookup list, and the 16 'lanes' in the
//   0x78020-0x7802f area are 00 except for two values, which are 1. The corresponding two skipped blocks
//   in the flash area are all zero/unused.
//   Likewise, with mkartagp2, there are three 1 value and one 2 values in the table,
//   and mkartagp2 skips 5 blocks in its mapping table.
// * 0x79ffc-0x79fff contains what may be a checksum of the 0x78000-0x79ffb area? unclear.

// the flash board has up to 8 k9f1208u0b or k9k1g08uoa flash roms, which contain both sector data broken into 512 byte sectors,
// and 16 bytes of metadata appearing immediately after each sector.
// In the implementation on this board, the data for each sector is 512 words long
// (technically 512 bytes long, but in this implementation it is interleaved between two flash roms)
// followed by a 16-word metadata area which contains what seems to be the sector checksums.
// This checksum data may be specific to its local rom, meaning it is only the checksum
// for the high half or the low half of each word, depending on which rom the metadata appears in. The
// checksum algorithm is unknown, and it is unclear if the checksum is for the des-encrypted or decrypted data.
// Each rom is (512+16)*131072 or (512+16)*262144 bytes long respectively, and the rom can be used in
// either an 8 bit wide or 16 bit wide mode. It is unclear whether the pcb uses it in the 8 or 16 bit wide mode,
// however the data is interleaved bytewise (not wordwise), regardless of which mode is used.

// The original dumper wrote: "This dump is tested good on h/w: was flashed to a dead cart and it then ran fine"
// further research has indicated that the dump below has a lot of missing data, and one rom entirely blank/bad.
ROM_START( mkartagp )
	TRIFORCE_BIOS

	ROM_REGION(0x19000000, "rom_board", 0)
	ROM_LOAD16_BYTE( "ic1_k9f1208u0b",  0x00000000, 0x4200000, BAD_DUMP CRC(7edb6ff2) SHA1(c544c09fc0441f940623c7368919e46153d49c20) ) // all data from 0x3de0000 - 0x41fffff is 0xFF and should not be
	ROM_LOAD16_BYTE( "ic2_k9f1208u0b",  0x00000001, 0x4200000, BAD_DUMP CRC(beb58594) SHA1(826ddc3db46f7644b08488618453917430bb16a1) ) // all data from 0x3de0000 - 0x41fffff is 0xFF and should not be
	ROM_LOAD16_BYTE( "ic35_k9f1208u0b", 0x08400000, 0x4200000, BAD_DUMP CRC(9a67892f) SHA1(f2beb56d07a42a01a8cfffbf683d8ec58c8407cc) ) // This dump is completely blank (0xFF)
	ROM_LOAD16_BYTE( "ic45_k9f1208u0b", 0x08400001, 0x4200000, BAD_DUMP CRC(274e7b81) SHA1(d97951c19d4ea430e09bc56777d99651a1f888d1) ) // all data from 0x3de0000 - 0x41fffff is 0xFF and should not be
	ROM_LOAD16_BYTE( "ic5_k9f1208u0b",  0x10800000, 0x4200000, BAD_DUMP CRC(fd7b9a28) SHA1(bc56c0a786e70de7365bd1b46fe82b3c43388f0c) ) // all data from 0x3de0000 - 0x41fffff is 0xFF and MAYBE should not be, since it is after the end of the data in the rom, so it may be correct
	ROM_LOAD16_BYTE( "ic6_k9f1208u0b",  0x10800001, 0x4200000, BAD_DUMP CRC(26bcfe14) SHA1(893e6b38cccca62037fc01012410d535634f8bc1) ) // all data from 0x3de0000 - 0x41fffff is 0xFF and MAYBE should not be, since it is after the end of the data in the rom, so it may be correct
	ROM_LOAD( "ic9_29lv400t",     0x18c00000, 0x0080000, CRC(f1ba67b2) SHA1(212fe4b28b6f9590bff200a6680bf7ee381780c7) ) // block mapping flash rom

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD("317-5109-com.pic", 0x00, 0x4000, CRC(b7159fe3) SHA1(886c026ae62651bb5f700dfd6f9ebb2c415c1ae8) )
ROM_END

ROM_START( mkartagpc )
	TRIFORCE_BIOS

	ROM_REGION(0x19000000, "rom_board", 0)
	ROM_LOAD16_BYTE( "ic1_k9f1208u0b",  0x00000000, 0x4200000, BAD_DUMP CRC(7edb6ff2) SHA1(c544c09fc0441f940623c7368919e46153d49c20) ) // all data from 0x3de0000 - 0x41fffff is 0xFF and should not be
	ROM_LOAD16_BYTE( "ic2_k9f1208u0b",  0x00000001, 0x4200000, BAD_DUMP CRC(beb58594) SHA1(826ddc3db46f7644b08488618453917430bb16a1) ) // all data from 0x3de0000 - 0x41fffff is 0xFF and should not be
	ROM_LOAD16_BYTE( "ic35_k9f1208u0b", 0x08400000, 0x4200000, BAD_DUMP CRC(9a67892f) SHA1(f2beb56d07a42a01a8cfffbf683d8ec58c8407cc) ) // This dump is completely blank (0xFF)
	ROM_LOAD16_BYTE( "ic45_k9f1208u0b", 0x08400001, 0x4200000, BAD_DUMP CRC(274e7b81) SHA1(d97951c19d4ea430e09bc56777d99651a1f888d1) ) // all data from 0x3de0000 - 0x41fffff is 0xFF and should not be
	ROM_LOAD16_BYTE( "ic5_k9f1208u0b",  0x10800000, 0x4200000, BAD_DUMP CRC(fd7b9a28) SHA1(bc56c0a786e70de7365bd1b46fe82b3c43388f0c) ) // all data from 0x3de0000 - 0x41fffff is 0xFF and MAYBE should not be, since it is after the end of the data in the rom, so it may be correct
	ROM_LOAD16_BYTE( "ic6_k9f1208u0b",  0x10800001, 0x4200000, BAD_DUMP CRC(26bcfe14) SHA1(893e6b38cccca62037fc01012410d535634f8bc1) ) // all data from 0x3de0000 - 0x41fffff is 0xFF and MAYBE should not be, since it is after the end of the data in the rom, so it may be correct
	ROM_LOAD( "ic9_29lv400t",     0x18c00000, 0x0080000, CRC(f1ba67b2) SHA1(212fe4b28b6f9590bff200a6680bf7ee381780c7) ) // block mapping flash rom

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD("317-5109-com.pic", 0x00, 0x4000, CRC(b7159fe3) SHA1(886c026ae62651bb5f700dfd6f9ebb2c415c1ae8) )

	DISK_REGION( "sdcard" )
	// Hagiwara SYS-COM SD-card labeled "UPDATE SOFTWARE MKA1 Ver. C"
	// BAD_DUMP note: card was dumped using Windows without write blocker and "System Volume Information" folder was automatically created, actual update data is good.
	DISK_IMAGE( "update_software_mka1_ver_c", 0, BAD_DUMP SHA1(92efde39eb65bf788ef4d022eb97925d13af5088) )
ROM_END

/*

MK21 Ver.A
Number on ROM PCB  837-14343R4S0
Number on Actel ProAsic  315-6419B
Number on Key Chip (PIC)  235-5509-5128

*/

ROM_START( mkartag2 )
	TRIFORCE_BIOS

	ROM_REGION(0x21200000, "rom_board", 0)
	// DES encrypted
	ROM_LOAD16_BYTE( "ic1_k9f1208u0b.bin",    0x00000000, 0x4200000, BAD_DUMP CRC(e52f17ef) SHA1(1e007d3136cacb89c396b8261e3978956cc21bdd) ) // all data from 0x3de0000 - 0x41fffff is 0xFF and should not be
	ROM_LOAD16_BYTE( "ic2_k9f1208u0b.bin",    0x00000001, 0x4200000, BAD_DUMP CRC(8a6a2649) SHA1(fd4318e7fb5020c499e06fdb1996b8d40161b674) ) // all data from 0x3de0000 - 0x41fffff is 0xFF and should not be
	ROM_LOAD16_BYTE( "ic3_k9f1208u0b.bin",    0x08400000, 0x4200000, BAD_DUMP CRC(8fd44a29) SHA1(9392bc4da6541960a83e9c7b3ab4f36bc5564fb7) ) // all data from 0x3de0000 - 0x41fffff is 0xFF and should not be
	ROM_LOAD16_BYTE( "ic4_k9f1208u0b.bin",    0x08400001, 0x4200000, BAD_DUMP CRC(ae3fb198) SHA1(c9c0beb9f6875dbf7ce015454a59481524ef3ef6) ) // all data from 0x3de0000 - 0x41fffff is 0xFF and should not be
	ROM_LOAD16_BYTE( "ic5_k9f1208u0b.bin",    0x10800000, 0x4200000, BAD_DUMP CRC(e9208514) SHA1(ad0ed3e681cd78a61d7ad3af83db20e364bb47fd) ) // all data from 0x3de0000 - 0x41fffff is 0xFF and should not be
	ROM_LOAD16_BYTE( "ic6_k9f1208u0b.bin",    0x10800001, 0x4200000, BAD_DUMP CRC(7363697c) SHA1(997c96d0b41774a24a2e0427a703bc295e784187) ) // all data from 0x3de0000 - 0x41fffff is 0xFF and should not be
	ROM_LOAD16_BYTE( "ic7_k9f1208u0b.bin",    0x18c00000, 0x4200000, BAD_DUMP CRC(407da1d2) SHA1(c185ebd2f3d8654d8fd394c56ac9bfff7e49f125) ) // all data from 0x3de0000 - 0x41fffff is 0xFF and MAYBE should not be, since it is after the end of the data in the rom, so it may be correct
	ROM_LOAD16_BYTE( "ic8_k9f1208u0b.bin",    0x18c00001, 0x4200000, BAD_DUMP CRC(9ab76062) SHA1(36a6317c646da5cf682d46ca438ce05e600bc354) )  // all data from 0x3de0000 - 0x41fffff is 0xFF and MAYBE should not be, since it is after the end of the data in the rom, so it may be correct
	// below contain rom board NAND block list/map (16bit words), the "block" is 2 * 32 NAND pages (512 data bytes +16 ECC?)
	ROM_LOAD( "ic9_mx29lv400cttc.bin",        0x21000000, 0x0080000, CRC(2bb9f1fe) SHA1(935511d93a0ab06436b0674bef90c790c100e0b1) ) // block mapping flash rom

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD("317-5128-com.pic", 0x00, 0x4000, CRC(0231b10c) SHA1(e060d74753a39081364e3175ac12e724ad585c33) )
ROM_END

ROM_START( mkartag2a )
	TRIFORCE_BIOS

	ROM_REGION(0x21200000, "rom_board", 0)
	// DES encrypted
	ROM_LOAD16_BYTE( "ic1_k9f1208u0b.bin",    0x00000000, 0x4200000, BAD_DUMP CRC(c5624816) SHA1(d6f2a2ff9e9e14d857a0ec810521c378ba1fabd5) ) // sldh; all data from 0x3de0000 - 0x41fffff is 0xFF and should not be
	ROM_LOAD16_BYTE( "ic2_k9f1208u0b.bin",    0x00000001, 0x4200000, BAD_DUMP CRC(44e59a1f) SHA1(68a8c1178a33e23446980ec84486bc614f830dad) ) // sldh; all data from 0x3de0000 - 0x41fffff is 0xFF and should not be
	ROM_LOAD16_BYTE( "ic3_k9f1208u0b.bin",    0x08400000, 0x4200000, BAD_DUMP CRC(6688e7f9) SHA1(7d1e60806c02fd765dd0981e790a698a29050aae) ) // sldh; all data from 0x3de0000 - 0x41fffff is 0xFF and should not be
	ROM_LOAD16_BYTE( "ic4_k9f1208u0b.bin",    0x08400001, 0x4200000, BAD_DUMP CRC(e043eac2) SHA1(0108d940a852ff03e919170957f2bca2c1a88a03) ) // sldh; all data from 0x3de0000 - 0x41fffff is 0xFF and should not be
	ROM_LOAD16_BYTE( "ic5_k9f1208u0b.bin",    0x10800000, 0x4200000, BAD_DUMP CRC(20882926) SHA1(c802de32ca24bf4e9fbabf47fed23a91b3d614ac) ) // sldh; all data from 0x3de0000 - 0x41fffff is 0xFF and should not be
	ROM_LOAD16_BYTE( "ic6_k9f1208u0b.bin",    0x10800001, 0x4200000, BAD_DUMP CRC(14171ba4) SHA1(3ddace539cd8a4b53a1ef03238e8404db7dcd85e) ) // sldh; all data from 0x3de0000 - 0x41fffff is 0xFF and should not be
	ROM_LOAD16_BYTE( "ic7_k9f1208u0b.bin",    0x18c00000, 0x4200000, BAD_DUMP CRC(bd0199df) SHA1(aafb171e9f5a4c8dc2ef55ba344a0eb310c63467) ) // sldh; all data from 0x3de0000 - 0x41fffff is 0xFF and MAYBE should not be, since it is after the end of the data in the rom, so it may be correct
	ROM_LOAD16_BYTE( "ic8_k9f1208u0b.bin",    0x18c00001, 0x4200000, BAD_DUMP CRC(8ad6c7ae) SHA1(749b99a944f62aefb895a622c029656c69b3c736) ) // sldh; all data from 0x3de0000 - 0x41fffff is 0xFF and MAYBE should not be, since it is after the end of the data in the rom, so it may be correct
	// below contain rom board NAND block list/map (16bit words), the "block" is 2 * 32 NAND pages (512 data bytes +16 ECC?)
	ROM_LOAD16_BYTE( "ic9_mx29lv400cttc.bin", 0x21000000, 0x0080000, CRC(ff854fd0) SHA1(0e42aff9a60aacd200b7a29d4d180abdab6a732e) ) // sldh; block mapping flash rom

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD("317-5128-com.pic", 0x00, 0x4000, CRC(0231b10c) SHA1(e060d74753a39081364e3175ac12e724ad585c33) )
ROM_END

/*

F-ZERO AX MONSTER RIDE
Cart: 842-06CN4000
ROM PCB: 837-14343-4S1
Actel ProAsic: 315-6362
Key Chip (PIC): 253-5508-0373
IC1-IC4 Samsung K9K1G08UOA
IC9     MX 29LV400TTC

*/

ROM_START( fzeroaxm )
	TRIFORCE_BIOS

	ROM_REGION(0x21200000, "rom_board", 0)
	// DES encrypted
	ROM_LOAD16_BYTE( "ic1.bin",     0x00000000, 0x8400000, CRC(50d1363d) SHA1(d673ef4e5f6c9dfb7a3b9c3bd453a9d7b86077a2) )
	ROM_LOAD16_BYTE( "ic2.bin",     0x00000001, 0x8400000, CRC(ce749357) SHA1(ecbd318fa6d183ba9428170b61699d8808356df8) )
	ROM_LOAD16_BYTE( "ic3s.bin",    0x10800000, 0x8400000, CRC(0f7bd95a) SHA1(33fa62685e2039266be3448bec386c774174952b) )
	ROM_LOAD16_BYTE( "ic4s.bin",    0x10800001, 0x8400000, CRC(9f10994f) SHA1(987b8b3051a17f3085fe7fbad595c56c1d05f605) )
	// below contain rom board NAND block list/map (16bit words), the "block" is 2 * 32 NAND pages (512 data bytes +16 ECC?)
	ROM_LOAD16_BYTE( "ic9.bin",     0x21000000, 0x0080000, CRC(bf461723) SHA1(4c18a4c3fabde7e63e36a7fcda19cd6acfe45f84) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD("317-0373-com.pic", 0x00, 0x4000, CRC(0e48f645) SHA1(ec057b0f536c3e9ce5353c02676cf17e96bb3239) )
ROM_END

} // anonymous namespace


/* Main board */
/*Triforce*/GAME( 2002, triforce, 0,        triforce_base, triforce, triforce_state, empty_init, ROT0, "Sega",                               "Triforce BIOS", MACHINE_IS_SKELETON|MACHINE_IS_BIOS_ROOT )

/* GDT-xxxx (Sega GD-ROM games) */
/* 0001  */ GAME( 2002, vs2002j,  triforce, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Amusement Vision",            "Virtua Striker 2002 (Japan) (GDT-0001)", MACHINE_IS_SKELETON )
/* 0002  */ GAME( 2002, vs2002ex, triforce, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Amusement Vision",            "Virtua Striker 2002 (Export) (GDT-0002)", MACHINE_IS_SKELETON )
// 0003
// 0004     GAME( 2003, fzeroaxo, fzeroax,  triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Amusement Vision / Nintendo", "F-Zero AX (GDT-0004)", MACHINE_IS_SKELETON )
// 0004A    GAME( 2003, fzeroaxa, fzeroax,  triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Amusement Vision / Nintendo", "F-Zero AX (Rev A) (GDT-0004A)", MACHINE_IS_SKELETON )
// 0004B    GAME( 2003, fzeroaxb, fzeroax,  triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Amusement Vision / Nintendo", "F-Zero AX (Rev B) (GDT-0004B)", MACHINE_IS_SKELETON )
/* 0004C */ GAME( 2003, fzeroaxc, fzeroax,  triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Amusement Vision / Nintendo", "F-Zero AX (Rev C) (GDT-0004C)", MACHINE_IS_SKELETON )
/* 0004D */ GAME( 2003, fzeroaxd, fzeroax,  triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Amusement Vision / Nintendo", "F-Zero AX (Rev D) (GDT-0004D)", MACHINE_IS_SKELETON )
/* 0004E */ GAME( 2003, fzeroax,  triforce, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Amusement Vision / Nintendo", "F-Zero AX (Rev E) (GDT-0004E)", MACHINE_IS_SKELETON )
// 0005     GAME( 2003, avalonso, avalons,  triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon - The Wizard Master (server) (GDT-0005)", MACHINE_IS_SKELETON )
/* 0005A */ GAME( 2003, avalonsa, avalons,  triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon - The Wizard Master (server, Rev A) (GDT-0005A)", MACHINE_IS_SKELETON )
// 0005B    GAME( 2003, avalonsb, avalons,  triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon - The Wizard Master (server, Rev B) (GDT-0005B)", MACHINE_IS_SKELETON )
/* 0005C */ GAME( 2003, avalonsc, avalons,  triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon - The Wizard Master (server, Rev C) (GDT-0005C)", MACHINE_IS_SKELETON )
// 0005D    GAME( 2003, avalonsd, avalons,  triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon - The Wizard Master (server, Rev D) (GDT-0005D)", MACHINE_IS_SKELETON )
/* 0005E */ GAME( 2003, avalonse, avalons,  triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon - The Wizard Master (server, Rev E) (GDT-0005E)", MACHINE_IS_SKELETON )
/* 0005F */ GAME( 2003, avalonsf, avalons,  triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon - The Wizard Master (server, Rev F) (GDT-0005F)", MACHINE_IS_SKELETON )
/* 0005G */ GAME( 2003, avalons,  triforce, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon - The Wizard Master (server, Rev G) (GDT-0005G)", MACHINE_IS_SKELETON )
// 0006     GAME( 2003, avalonco, avalonc,  triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon - The Wizard Master (client) (GDT-0006)", MACHINE_IS_SKELETON )
/* 0006A */ GAME( 2003, avalonca, avalonc,  triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon - The Wizard Master (client, Rev A) (GDT-0006A)", MACHINE_IS_SKELETON )
// 0006B    GAME( 2003, avaloncb, avalonc,  triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon - The Wizard Master (client, Rev B) (GDT-0006B)", MACHINE_IS_SKELETON )
/* 0006C */ GAME( 2003, avaloncc, avalonc,  triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon - The Wizard Master (client, Rev C) (GDT-0006C)", MACHINE_IS_SKELETON )
// 0006D    GAME( 2003, avaloncd, avalonc,  triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon - The Wizard Master (client, Rev D) (GDT-0006D)", MACHINE_IS_SKELETON )
/* 0006E */ GAME( 2003, avalonce, avalonc,  triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon - The Wizard Master (client, Rev E) (GDT-0006E)", MACHINE_IS_SKELETON )
/* 0006F */ GAME( 2003, avaloncf, avalonc,  triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon - The Wizard Master (client, Rev F) (GDT-0006F)", MACHINE_IS_SKELETON )
/* 0006G */ GAME( 2003, avalonc,  triforce, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon - The Wizard Master (client, Rev G) (GDT-0006G)", MACHINE_IS_SKELETON )
// 0007  F-Zero AX (Asia)
// 0008     GAME( 2003, gekpuryao,gekpurya, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Wow Entertainment",           "Gekitou Pro Yakyuu Mizushima Shinji All Stars vs. Pro Yakyuu (GDT-0008)", MACHINE_IS_SKELETON )
// 0008A    GAME( 2003, gekpuryaa,gekpurya, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Wow Entertainment",           "Gekitou Pro Yakyuu Mizushima Shinji All Stars vs. Pro Yakyuu (Rev A) (GDT-0008A)", MACHINE_IS_SKELETON )
/* 0008B */ GAME( 2003, gekpuryb, gekpurya, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Wow Entertainment",           "Gekitou Pro Yakyuu Mizushima Shinji All Stars vs. Pro Yakyuu (Rev B) (GDT-0008B)", MACHINE_IS_SKELETON )
/* 0008C */ GAME( 2003, gekpurya, triforce, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Wow Entertainment",           "Gekitou Pro Yakyuu Mizushima Shinji All Stars vs. Pro Yakyuu (Rev C) (GDT-0008C)", MACHINE_IS_SKELETON )
// 0009     GAME( 2004, avalns11, avalns13, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon 1.? - ??? (server) (GDT-0009)", MACHINE_IS_SKELETON )
/* 0009A */ GAME( 2004, avalns12, avalns13, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon 1.2 - Summon The New Monsters (server, Rev A) (GDT-0009A)", MACHINE_IS_SKELETON )
// 0009B    GAME( 2004, avals13b, avalns13, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon 1.? - ??? (server, Rev B) (GDT-0009B)", MACHINE_IS_SKELETON )
/* 0009C */ GAME( 2004, avalns13, triforce, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon 1.3 - Chaotic Sabbat (server, Rev C) (GDT-0009C)", MACHINE_IS_SKELETON )
// 0010     GAME( 2004, avalnc11, avalnc13, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon 1.? - ??? (client) (GDT-0010)", MACHINE_IS_SKELETON )
/* 0010A */ GAME( 2004, avalnc12, avalnc13, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon 1.2 - Summon The New Monsters (client, Rev A) (GDT-0010A)", MACHINE_IS_SKELETON )
// 0010B    GAME( 2004, avalc13b, avalnc13, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon 1.? - ??? (client, Rev B) (GDT-0010B)", MACHINE_IS_SKELETON )
/* 0010C */ GAME( 2004, avalnc13, triforce, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon 1.3 - Chaotic Sabbat (client, Rev C) (GDT-0010C)", MACHINE_IS_SKELETON )
/* 0011  */ GAME( 2003, tfupdate, triforce, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega",                               "Triforce DIMM Updater (3.17) (GDT-0011)", MACHINE_IS_SKELETON )
/* 0012  */ GAME( 2004, vs2002t3, triforce, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Amusement Vision",            "Virtua Striker 2002 (Type 3) (GDT-0012)", MACHINE_IS_SKELETON )
// 0013     GAME( 2004, vs4jo,    vs4j,     triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega",                               "Virtua Striker 4 (Japan) (GDT-0013)", MACHINE_IS_SKELETON )
// 0013A    GAME( 2004, vs4ja,    vs4j,     triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega",                               "Virtua Striker 4 (Japan, Rev A) (GDT-0013A)", MACHINE_IS_SKELETON )
// 0013B    GAME( 2004, vs4jb,    vs4j,     triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega",                               "Virtua Striker 4 (Japan, Rev B) (GDT-0013B)", MACHINE_IS_SKELETON )
/* 0013C */ GAME( 2004, vs4jc,    vs4j,     triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega",                               "Virtua Striker 4 (Japan, Rev C) (GDT-0013C)", MACHINE_IS_SKELETON )
// 0013D    GAME( 2004, vs4jd,    vs4j,     triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega",                               "Virtua Striker 4 (Japan, Rev D) (GDT-0013D)", MACHINE_IS_SKELETON )
/* 0013E */ GAME( 2005, vs4j,     triforce, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega",                               "Virtua Striker 4 (Japan, Rev E) (GDT-0013E)", MACHINE_IS_SKELETON )
/* 0014  */ GAME( 2004, vs4eo,    vs4,      triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega",                               "Virtua Striker 4 (Asia) (GDT-0014)", MACHINE_IS_SKELETON )
// 0014A    GAME( 2004, vs4ea,    vs4,      triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega",                               "Virtua Striker 4 (Asia, Rev A) (GDT-0014A)", MACHINE_IS_SKELETON )
/* 0014B */ GAME( 2005, vs4e,     vs4,      triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega",                               "Virtua Striker 4 (Asia, Rev B) (GDT-0014B)", MACHINE_IS_SKELETON )
/* 0015  */ GAME( 2004, vs4o,     vs4,      triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega",                               "Virtua Striker 4 (Export) (GDT-0015)", MACHINE_IS_SKELETON )
/* 0015A */ GAME( 2005, vs4,      triforce, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega",                               "Virtua Striker 4 (Export, Rev A) (GDT-0015A)", MACHINE_IS_SKELETON )
// 0016     GAME( 2004, avalons20,triforce, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon 2.0 - Eutaxy and Commandment (server) (GDT-0016)", MACHINE_IS_SKELETON )
// 0017     GAME( 2004, avalon20o,avalon20, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon 2.0 - Eutaxy and Commandment (client) (GDT-0017)", MACHINE_IS_SKELETON )
// 0017A    GAME( 2004, avalon20a,avalon20, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon 2.0 - Eutaxy and Commandment (client, Rev A) (GDT-0017A)", MACHINE_IS_SKELETON )
/* 0017B */ GAME( 2004, avalon20, triforce, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon 2.0 - Eutaxy and Commandment (client, Rev B) (GDT-0017B)", MACHINE_IS_SKELETON )
// 0018     GAME( 2005, avalns25o,avalns25, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon 2.5 - War of the Key (server) (GDT-0018)", MACHINE_IS_SKELETON )
/* 0018A */ GAME( 2005, avalns25a,avalns25, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon 2.5 - War of the Key (server, Rev A) (GDT-0018A)", MACHINE_IS_SKELETON )
/* 0018B */ GAME( 2005, avalns25, triforce, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon 2.5 - War of the Key (server, Rev B) (GDT-0018B)", MACHINE_IS_SKELETON )
// 0019     GAME( 2005, avalnc25o,avalnc25, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon 2.5 - War of the Key (client) (GDT-0019)", MACHINE_IS_SKELETON )
/* 0019A */ GAME( 2005, avalnc25a,avalnc25, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon 2.5 - War of the Key (client, Rev A) (GDT-0019A)", MACHINE_IS_SKELETON )
/* 0019B */ GAME( 2005, avalnc25, triforce, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega / Hitmaker",                    "The Key Of Avalon 2.5 - War of the Key (client, Rev B) (GDT-0019B)", MACHINE_IS_SKELETON )
// 0020     GAME( 2006, vs42006o, vs42006,  triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega",                               "Virtua Striker 4 Ver.2006 (Japan) (GDT-0020)", MACHINE_IS_SKELETON )
// 0020A    GAME( 2006, vs42006a, vs42006,  triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega",                               "Virtua Striker 4 Ver.2006 (Japan, Rev A) (GDT-0020A)", MACHINE_IS_SKELETON )
/* 0020B */ GAME( 2006, vs42006b, vs42006,  triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega",                               "Virtua Striker 4 Ver.2006 (Japan, Rev B) (GDT-0020B)", MACHINE_IS_SKELETON )
// 0020C    GAME( 2006, vs42006c, vs42006,  triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega",                               "Virtua Striker 4 Ver.2006 (Japan, Rev C) (GDT-0020C)", MACHINE_IS_SKELETON )
/* 0020D */ GAME( 2006, vs42006,  triforce, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega",                               "Virtua Striker 4 Ver.2006 (Japan, Rev D) (GDT-0020D)", MACHINE_IS_SKELETON )
/* 0021  */ GAME( 2006, vs42k6ex, triforce, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega",                               "Virtua Striker 4 Ver.2006 (Export) (GDT-0021)", MACHINE_IS_SKELETON )
// 0022     GAME( 2006, tcfboxo,  tcfboxa,  triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega",                               "Triforce Firmware Update For Compact Flash Box (GDT-0022)", MACHINE_IS_SKELETON )
/* 0022A */ GAME( 2009, tcfboxa,  triforce, triforcegd,    triforce, triforce_state, empty_init, ROT0, "Sega",                               "Triforce Firmware Update For Compact Flash Box (4.01) (GDT-0022A)", MACHINE_IS_SKELETON )

// 837-xxxxx (Sega cart games)
/* 14343-4S1  */ GAME( 2003, fzeroaxm, triforce, triforce_base, triforce, triforce_state, empty_init, ROT0, "Sega / Amusement Vision / Nintendo", "F-Zero AX Monster Ride", MACHINE_IS_SKELETON ) // 2003/09/04
/* 14343-4T1  */ GAME( 2005, mkartagp, triforce, triforce_base, triforce, triforce_state, empty_init, ROT0, "Namco / Nintendo", "Mario Kart Arcade GP (Japan, MKA1 Ver.A1)", MACHINE_IS_SKELETON )
/* 14343-4T1  */ GAME( 2006, mkartagpc,mkartagp, triforce_base, triforce, triforce_state, empty_init, ROT0, "Namco / Nintendo", "Mario Kart Arcade GP (Japan, MKA1 Ver.C, update)", MACHINE_IS_SKELETON )
/* 14343-R4S0 */ GAME( 2007, mkartag2, triforce, triforce_base, triforce, triforce_state, empty_init, ROT0, "Namco / Nintendo", "Mario Kart Arcade GP 2 (Japan, MK21 Ver.A)", MACHINE_IS_SKELETON )
/* 14343-R4S0 */ GAME( 2007, mkartag2a,mkartag2, triforce_base, triforce, triforce_state, empty_init, ROT0, "Namco / Nintendo", "Mario Kart Arcade GP 2 (Japan, MK21 Ver.A, alt dump)", MACHINE_IS_SKELETON )
