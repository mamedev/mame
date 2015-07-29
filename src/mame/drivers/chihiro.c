// license:BSD-3-Clause
// copyright-holders:Samuele Zannoli
/*
Chihiro is an Xbox-based arcade system from SEGA.

Games on this system include....

   Year   Game                                                 Manufacturer / Developer   Media    Number       Key Chip
+-+------+----------------------------------------------------+--------------------------+--------+------------+--------------|
|*| 2002 | The House of the Dead III                          | Sega / Wow Entertainment | GDROM  | GDX-0001   | 317-0348-COM |
| | 2003 | Crazy Taxi High Roller                             | Sega / Hitmaker          | GDROM  | GDX-0002   | 317-0353-COM |
| | 2003 | Crazy Taxi High Roller (Rev A)                     | Sega / Hitmaker          | GDROM  | GDX-0002A  | 317-0353-COM |
|*| 2003 | Crazy Taxi High Roller (Rev B)                     | Sega / Hitmaker          | GDROM  | GDX-0002B  | 317-0353-COM |
| | 2003 | Virtua Cop 3                                       | Sega                     | GDROM  | GDX-0003   | 317-0354-COM |
|*| 2003 | Virtua Cop 3 (Rev A)                               | Sega                     | GDROM  | GDX-0003A  | 317-0354-COM |
| | 2003 | OutRun 2                                           | Sega                     | GDROM  | GDX-0004   | 317-0372-COM |
|*| 2003 | OutRun 2 (Rev A)                                   | Sega                     | GDROM  | GDX-0004A  | 317-0372-COM |
| | 2003 | OutRun 2 prototype (Rev P )                        | Sega                     | GDROM  | GDX-0004P  |              |
| | 2004 | Sega Golf Club Network Pro Tour                    | Sega                     | GDROM  | GDX-0005   |              |
| | 2004 | Sega Network Taisen Mahjong MJ 2                   | Sega                     | GDROM  | GDX-0006   |              |
| | 2004 | Sega Network Taisen Mahjong MJ 2 (Rev A)           | Sega                     | GDROM  | GDX-0006A  |              |
| | 2004 | Sega Network Taisen Mahjong MJ 2 (Rev B)           | Sega                     | GDROM  | GDX-0006B  |              |
|*| 2004 | Sega Network Taisen Mahjong MJ 2 (Rev C)           | Sega                     | GDROM  | GDX-0006C  |              |
| | 2004 | Sega Network Taisen Mahjong MJ 2 (Rev D)           | Sega                     | GDROM  | GDX-0006D  |              |
| | 2005 | Sega Network Taisen Mahjong MJ 2 (Rev E)           | Sega                     | GDROM  | GDX-0006E  |              |
|*| 2005 | Sega Network Taisen Mahjong MJ 2 (Rev F)           | Sega                     | GDROM  | GDX-0006F  |              |
|*| 2005 | Sega Network Taisen Mahjong MJ 2 (Rev G)           | Sega                     | GDROM  | GDX-0006G  | 317-0374-JPN |
|*| 2004 | Ollie King                                         | Sega / Amusement Vision  | GDROM  | GDX-0007   | 317-0377-COM |
| | 2004 | Wangan Midnight Maximum Tune (Japan)               | Namco                    | GDROM  | GDX-0008   | 317-5101-JPN |
| | 2004 | Wangan Midnight Maximum Tune (Japan) (Rev A)       | Namco                    | GDROM  | GDX-0008A  | 317-5101-JPN |
|*| 2004 | Wangan Midnight Maximum Tune (Japan) (Rev B)       | Namco                    | GDROM  | GDX-0008B  | 317-5101-JPN |
| | 2004 | Wangan Midnight Maximum Tune (Export)              | Namco                    | GDROM  | GDX-0009   | 317-5101-COM |
| | 2004 | Wangan Midnight Maximum Tune (Export) (Rev A)      | Namco                    | GDROM  | GDX-0009A  | 317-5101-COM |
|*| 2004 | Wangan Midnight Maximum Tune (Export) (Rev B)      | Namco                    | GDROM  | GDX-0009B  | 317-5101-COM |
| | 2004 | OutRun 2 SP (Japan)                                | Sega                     | GDROM  | GDX-0011   |              |
|*| 2004 | Ghost Squad                                        | Sega                     | GDROM  | GDX-0012   | 317-0398-COM |
|*| 2004 | Ghost Squad (Rev A)                                | Sega                     | GDROM  | GDX-0012A  | 317-0398-COM |
|*| 2005 | Gundam Battle Operating Simulator                  | Banpresto                | GDROM  | GDX-0013   | 317-0400-JPN |
| | 2004 | OutRun 2 Special Tours                             | Sega                     | GDROM  | GDX-0014   | 317-0xxx-COM |
|*| 2004 | OutRun 2 Special Tours (Rev A)                     | Sega                     | GDROM  | GDX-0014A  | 317-0xxx-COM |
|*| 2005 | Wangan Midnight Maximum Tune 2 (Japan)             | Namco                    | GDROM  | GDX-0015   | 317-5106-JPN |
| | 2005 | Wangan Midnight Maximum Tune 2 (Export)            | Namco                    | GDROM  | GDX-0016   | 317-5106-COM |
|*| 2005 | Wangan Midnight Maximum Tune 2 (Export)            | Namco                    | GDROM  | GDX-0016A  | 317-5106-COM |
| | 2005 | Sega Network Taisen Mahjong MJ 3                   | Sega                     | GDROM  | GDX-0017   | 317-0414-JPN |
| | 2005 | Sega Network Taisen Mahjong MJ 3 (Rev A)           | Sega                     | GDROM  | GDX-0017A  | 317-0414-JPN |
| | 2005 | Sega Network Taisen Mahjong MJ 3 (Rev B)           | Sega                     | GDROM  | GDX-0017B  | 317-0414-JPN |
| | 2005 | Sega Network Taisen Mahjong MJ 3 (Rev C)           | Sega                     | GDROM  | GDX-0017C  | 317-0414-JPN |
|*| 2005 | Sega Network Taisen Mahjong MJ 3 (Rev D)           | Sega                     | GDROM  | GDX-0017D  | 317-0414-JPN |
| | 2005 | Sega Network Taisen Mahjong MJ 3 (Rev E)           | Sega                     | GDROM  | GDX-0017E  | 317-0414-JPN |
|*| 2005 | Sega Network Taisen Mahjong MJ 3 (Rev F)           | Sega                     | GDROM  | GDX-0017F  | 317-0414-JPN |
| | 2005 | Sega Club Golf 2006: Next Tours                    | Sega                     | GDROM  | GDX-0018   |              |
|*| 2005 | Sega Club Golf 2006: Next Tours (Rev A)            | Sega                     | GDROM  | GDX-0018A  |              |
| | 2006 | Sega Network Taisen Mahjong MJ 3 Evolution         | Sega                     | GDROM  | GDX-0021   |              |
| | 2006 | Sega Network Taisen Mahjong MJ 3 Evolution (Rev A) | Sega                     | GDROM  | GDX-0021A  |              |
|*| 2007 | Sega Network Taisen Mahjong MJ 3 Evolution (Rev B) | Sega                     | GDROM  | GDX-0021B  |              |
| | 2009 | Firmware Update For Compact Flash Box              | Sega                     | GDROM  | GDX-0024   |              |
|*| 2009 | Firmware Update For Compact Flash Box (Rev A)      | Sega                     | GDROM  | GDX-0024A  | 317-0567-EXP |
|*| 2004 | Quest Of D (Ver.1.01C)                             | Sega                     | CDROM  | CDV-10005C |              |
|*| 2005 | Sangokushi Taisen (Ver.1.002)                      | Sega                     | DVDROM | CDV-10009D |              |
|*| 2006 | Sangokushi Taisen 2 (Ver.2.007)                    | Sega                     | DVDROM | CDV-10019A |              |
|*| 2005 | Sangokushi Taisen                                  | Sega                     | DVDROM | CDV-10022  |              |
|*| 2006 | Sangokushi Taisen 2 Firmware Update                | Sega                     | DVDROM | CDV-10023  |              |
|*| 2006 | Sangokushi Taisen 2                                | Sega                     | DVDROM | CDV-10029  |              |
|*| 2008 | Sangokushi Taisen 3                                | Sega                     | DVDROM | CDV-10036  |              |
|*| 2008 | Sangokushi Taisen 3 (Ver.J)                        | Sega                     | DVDROM | CDV-10036J |              |
|*| 2008 | Sangokushi Taisen 3 War Begins (Ver.3.59)          | Sega                     | DVDROM | CDV-10041  |              |
|*| 2008 | Sangokushi Taisen 3 War Begins                     | Sega                     | DVDROM | CDV-10042  |              |
+-+------+----------------------------------------------------+--------------------------+--------+------------+--------------+
* denotes these games are archived.

   Year   Game (Unknown media)                                Manufacturer
+-+-----------------------------------------------------------+------------+
| | 2004 | Quest Of D                                         | Sega       |
| | 2004 | Quest Of D (Ver.1.02)                              | Sega       |
| | 2004 | Quest Of D (Ver.1.10)                              | Sega       |
| | 2004 | Quest Of D (Ver.1.10a)                             | Sega       |
| | 2005 | Quest Of D (Ver.1.20)                              | Sega       |
| | 2005 | Quest Of D (Ver.1.20a)                             | Sega       |
| | 2005 | Quest Of D (Ver.1.21)                              | Sega       |
| | 2005 | Quest Of D: Gofu no Keisyousya (Ver.2.00)          | Sega       |
| | 2005 | Quest Of D: Gofu no Keisyousya (Ver.2.01)          | Sega       |
| | 2006 | Quest Of D: Gofu no Keisyousya (Ver.2.02b)         | Sega       |
| | 2006 | Quest Of D: Oukoku no Syugosya (Ver.3.00)          | Sega       |
| | 2006 | Quest Of D: Oukoku no Syugosya (Ver.3.01)          | Sega       |
| | 2007 | Quest Of D: The Battle Kingdom (Ver.4.00)          | Sega       |
| | 2008 | Quest Of D: The Battle Kingdom (Ver.4.00b)         | Sega       |
| | 2008 | Quest Of D: The Battle Kingdom (Ver.4.00c)         | Sega       |
| | 2008 | Quest Of D: The Battle Kingdom (Ver.4.01)          | Sega       |
| | 2005 | Sangokushi Taisen (Ver.1.03)                       | Sega       |
| | 2005 | Sangokushi Taisen (Ver.1.10)                       | Sega       |
| | 2005 | Sangokushi Taisen (Ver.1.11)                       | Sega       |
| | 2006 | Sangokushi Taisen (Ver.1.12)                       | Sega       |
| | 2006 | Sangokushi Taisen 2 (Ver.2.01)                     | Sega       |
| | 2005 | Sega Golf Club Network Pro Tour 2005               | Sega       |
+-+------+----------------------------------------------------+------------+

A Chihiro system consists of several boards.
The system is in 2 separate metal boxes that fit together to form one box.
In order from top to bottom they are....
 - Network board  \
 - Media board    /  Together in the top box

 - Base board     \
 - Xbox board     /  Together in the bottom box

The 2 boxes join together via the Base Board upper connector and Media Board lower connector.

The Microsoft-manufactured XBox board is the lowest board. It's mostly the same as the V1 XBox retail
board with the exception that it has 128MB of RAM and a NVidia MCPX X2 chip. The retail XBox board has a
MCPX X3 chip. The board was probably released to Sega very early in development and the chip was updated
in the mass-produced retail version.

The Sega-manufactured base board is connected to the XBox board via a 40 pin 80-wire flat cable and a 16
pin 16-wire flat cable connects to the LPC header, plus a couple of thin multi-wire cables which join to the
XBox game controller ports (USB1.1) and front panel connector.
On the reverse side of that board are the power output connectors going to the XBox board and a 100 pin
connector where the media board plugs in. A CR2032 coin battery is also located there and next to it are 6
jumpers....
JP3S 2-3 \
JP4S 1-2 |
JP5S 1-2 |
JP6S 2-3 | These are connected to the USB chip on the other side of this board
JP7S 1-2 |
JP8S 2-3 /
A long connector on one edge connects to the filter board (power supply input etc) and on another edge are
connectors for VGA output, audio/video input (from a short cable coming from the A/V connector on the XBox board)
and a 14 pin connector which is unused. The base board handles JVS and video output.

The upper part contains a Sega-manufactured media board with a TSOP48 flash ROM containing an xbox .xbe loader
(this is the Chihiro logo you see when you power the Chihiro) and there's a connector for a Sega network board.
The network board is 100% the same as the one in the Triforce v3 with the same firmware also.

The system requires one of the various Sega JVS I/O boards to operate.

ROMs on the boards
------------------
Network Board : One ST 29W160ET 2M x8-bit TSOP48 Flash ROM stamped 'FPR24036' at location IC2
Media Board   : One ST 29W160ET 2M x8-bit TSOP48 Flash ROM stamped 'FPR24042' at location IC8
                As in Triforce, it consists of two versions in the same flash, the first MB of the flash has
                an older version as backup, and the second MB has the current version, versions included are:
                SegaBoot Ver.2.00.0 Build:Feb  7 2003 12:28:30
                SegaBoot Ver.2.13.0 Build:Mar  3 2005 17:03:15
Base Board    : Two Microchip 24LC64 64k Serial EEPROMs at locations IC32 and IC10
                One Microchip 24LC24 2k Serial EEPROM at location IC11
                The chip at IC32 seems to be a backup of the base board firmware without region or serial.
                The chip at IC11 has an unknown purpose. It contains some strings. The interesting thing is that it
                contains the string SBJE. If you go to the system info menu and press the service button 16 times in a row
                a message will be displayed: GAME ID SBJE
                The chip at IC10 contains the firmware of the Base Board, serial number and REGION of the whole system.
                The region is located at offset 0x00001F10. 01 is JAPAN, 02 is USA, 03 is EXPORT. If you want to change
                the region of your Chihiro unit just change this byte.
                Alternatively, if you have a netboot PIC, plug that in and power up with it.
                 1) Enter the test menu (push the test button)
                 2) Go to the System Information menu.
                 3) Press the Service button 30 times in a row and a hidden menu will appear to change the region.
                 4) Once the region is changed, exit from the menus and after the Chihiro reboots, power off the system and
                    power on again.
Xbox Board    : One Macronix 29F040TC-90 512k x8-bit TSOP32 Flash ROM at location U7D1


Board Layouts
=============


XBox Board
----------

|--------------------------------------------|
|     LAN        FAN1         A/V        FAN2|
|                                            |
|                      LF353     CONEXANT    |
|DVD_PWR      WM9709             XC25871-14  |
|                                            |
|     ICS_UA431317                           |-------------------|
|                                             LM358              |
|     27MHz   PIC16LC63A                                         |
|IDE            BR24C02                                          |
|     ICS_455R-02                                                |
|                     K4D263238D                                 |
|        29F040.U7D1  *K4D263238D                                |
|                                                                |
|                                     GPU          CPU @733MHz   |
|       16_PIN_CONN                  (WITH FAN)   (SL5SN 733/128)|
|                     K4D263238D                                 |
|                     *K4D263238D                                |
|                                                                |
|                                                                |
|                          K4D263238D   K4D263238D               |
|                          *K4D263238D  *K4D263238D              |
|          NVIDIA                                                |
|          MCPX X2                                               |
|                                                    POWER_CONN  |
|                                                                |
|                                                                |
|                                                                |
|                           GAME1/2   FRONT_PANEL    GAME3/4     |
|----------------------------------------------------------------|
Notes:
      * These parts located on the other side of the PCB
      Some of the connectors are not used.
      GAME1/2      - Connected to CN1 on Base Board. JST Part Number B12B-PHDSS
      GAME3/4      - Connected to CN1 on Base Board. JST Part Number B12B-PHDSS
      FRONT_PANEL  - Connected to CN1 on Base Board. JST Part Number B10B-PHDSS


Base Board
----------

171-8204B
837-14280 SEGA 2002
Sticker: 837-14280-92
|----------------------------------------------------------------|
|*CN16S   *CN14S     *CN19S                      ADM3222         |
|                                                             CN8|
|                                          24LC64.IC32           |
|CN11    CN18        CN1                           PC410         |
|                                                     LM1881     |
|                     SN65240                                    |
|                              SN65240     AN2131SC   BA7623     |
|                                                            CN10|
|                                      12MHz     BA7623          |
|                        *CR2032                                 |
|                               SUPERCAP                         |
|                          32.768kHz                             |
|CN12                      RV5C386A                              |
|                    24LC024.IC11 M68AF127                    CN9|
|                    24LC64.IC10          AN2131QC               |
|                                 ADM3222             DS485      |
|                                               1.85MHz          |
|      3771                                                   CN5|
|-------------------------|       CN15         |-----------------|
                          |--------------------|
Notes:
      (* these parts on other side of the PCB)
      RV5C386A  - I2C Bus Serial Interface Real-Time Clock IC with Voltage Monitoring Function (SSOP10)
      24LC64    - Microchip 24LC64 64K I2C Serial EEPROM (SOIC8)
      24LC024   - Microchip 24LC024 2K I2C Serial EEPROM (SOIC8)
      M68AF127B - ST Microelectronics 1Mbit (128K x8), 5V Asynchronous SRAM (SOP32)
      AN2131QC  - Cypress AN2131 EZ-USB-Family 8051-based High-Speed USB IC's (QFP80)
      AN2131SC  /                                                             (QFP44)
      ADM3222   - Analog Devices ADM3222 High-Speed, +3.3V, 2-Channel RS232/V.28 Interface Device (SOIC20)
      SN65240   - Texas Instruments SN65240 USB Port Transient Suppressor (SOIC8)
      BA7623    - Rohm BA7623 75-Ohm driver IC with 3 internal circuits (SOIC8)
      LM1881    - National LM1881 Video Sync Separator (SOIC8)
      DS485     - National DS485 Low-Power RS-485/RS-422 Multipoint Transceiver (SOIC8)
      3771      - Fujitsu MB3771 System Reset IC (SOIC8)
      PC410     - Sharp PC410 Ultra-high Speed Response OPIC Photocoupler
      CN1       - 22-pin multi-wire cable connector joining to XBox board
      CN5       - USB connector joining to JVS I/O board with standard USB cable
      CN8       - A/V input connector (from XBox board via short A/V cable)
      CN9       - VGA output connector
      CN10      - 14 pin connector (purpose unknown but appears to be unused)
      CN11      - 16-pin flat cable connector joining to LPC connector on XBox board
      CN12      - 40-pin IDE flat cable connector joining to IDE connector on XBox board
      CN14S     - 7-pin power output connector joining to XBox board
      CN15      - 96-pin connector joining to filter board
      CN16S     - 2-pin connector joining to case fan on Chihiro lower section (next to XBox PCB)
      CN18      - 10-pin multi-wire cable connector joining to XBox board
      CN19S     - 5-pin power output connector joining to XBox board
      There are also many power-related components such as capacitors, mosfets and transistors.


Network Board
-------------

This board is identical to the network board used in Triforce games.
See src/mame/drivers/triforce.c


Media Board
-----------

171-8234C
837-14359-01 SEGA 2002
Sticker: 837-14359-91
|----------------------------------------------------------------|
|             LED LED                                            |
|                                         FLASH.IC8              |
| LED                    CN12 CN11                               |
| LED    JP4-JP10                                            CN10|
|                                                                |
|                                                                |
|                                   |----------|                 |
|                                   |SEGA      |                 |
|    CN14S*                         |315-6355  |                 |
|                                   |          |                 |
|                                   |          |                 |
| MB3800*                           |----------|              CN8|
|                                               CY25560*         |
|                                                                |
|                         CY23S09SC         49.25MHz             |
|                                                                |
|                          CN4 (DIMM)                            |
|                          CN3 (DIMM)                         CN5|
| MM1433*                                                        |
| CN13  TPC8009              CN9             CN6                 |
|----------------------------------------------------------------|
Notes:
      *         - These parts on other side of PCB
      CN3/4     - 72 pin DIMM sockets
      CN5       - GDROM data cable connector (SCSI mini-honda connection but signal/protocol is IDE)
      CN6       - 40 pin flat cable connector (unused)
      CN8       - 6 pin GDROM power connector
      CN9       - 6 pin power connector (unused)
      CN10      - Connector for small 90-degrees upright board where PIC plugs in. The upright board contains only a DIP18 socket and a 4MHz OSC.
      CN11/12   - Network board connectors joining to Sega Network PCB
      CN13      - Battery connector (maintains power to DIMM RAM)
      CN14S     - 100 pin connector joining to base board
      JP4-10    - Jumpers. Settings are as follows (taken from Wangan Midnight Maximum Tune 2 (Japan) (Rev A))
                  JP4 2-3
                  JP5 2-3. Sets DIMM RAM size. 1-2 = 1GB (2x 512M sticks), 2-3 = 512MB (1x 512M stick)
                  JP6 1-2
                  JP7 2-3
                  JP8 2-3
                  JP9 1-2
                  JP10 1-2
      FLASH.IC8 - ST M29W160 16MBit Flash ROM stamped 'FPR24042' (TSOP48)


Filter Board
------------

839-1208-02
171-8205C SEGA 2002
|----------------------------------------------------------------|
| SP-DIF     LED_STATUS2  LED_STATUS1                    CN3     |
|                                                                |
|                          DIN1                       LED_3.3V   |
|                                                     LED_5V     |
|            DIPSW(8)                                 LED_12V    |
|CN6 CN5 CN4          SW2  SW1           CN2         CN1         |
|----------------------------------------------------------------|
Notes:
      CN1   - 8-pin JVS power input connector
      CN2   - 6-pin JVS power input connector
      CN3   - Red/white RCA unamplified stereo audio output jacks
      CN4   - 11-pin connector
      CN5   - 8-pin connector
      CN6   - 7-pin connector
      SW1/2 - test/service buttons
      DIN1  - 96-pin connector joining to Base Board
      DIPSW - 8-position DIP switch. On this game (Wangan Midnight Maximum Tune 2 (Japan) (Rev A)) DIPs 3, 4, 6, 7 & 8 are set ON. The others are OFF.

Dump info:

Network Board Dump : Ver1305.bin
Media Board dump   : FPR21042_M29W160ET.bin
Base Board Dumps   : ic10_g24lc64.bin ic11_24lc024.bin pc20_g24lc64.bin
Xbox Board Dump    : chihiro_xbox_bios.bin

FPR21042_M29W160ET.bin :
As in Triforce, it consists of two versions in the same flash, the first MB of the flash has
an older version as backup, and the second MB has the current version, versions included are:
SegaBoot Ver.2.00.0 Build:Feb  7 2003 12:28:30
SegaBoot Ver.2.13.0 Build:Mar  3 2005 17:03:15

ic10_g24lc64.bin: This dump contains the firmware of the Base Board, serial number and REGION of the whole system
Region is located at Offset 0x00001F10 , 01 means JAP, 02 Means USA, 03 Means EXPORT, if you
want to change the region of your Chihiro Board, just change this byte.

Thanks to Alex, Mr Mudkips, and Philip Burke for this info.

*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/lpci.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/idectrl.h"
#include "machine/idehd.h"
#include "machine/naomigd.h"
#include "video/poly.h"
#include "bitmap.h"
#include "debug/debugcon.h"
#include "debug/debugcmd.h"
#include "debug/debugcpu.h"
#include "includes/chihiro.h"


// for now, make buggy GCC/Mingw STFU about I64FMT
#if (defined(__MINGW32__) && (__GNUC__ >= 5))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wformat-extra-args"
#endif

#define LOG_PCI
//#define LOG_OHCI
//#define LOG_BASEBOARD
//#define USB_ENABLED

struct OHCIEndpointDescriptor {
	int mps; // MaximumPacketSize
	int f; // Format
	int k; // sKip
	int s; // Speed
	int d; // Direction
	int en; // EndpointNumber
	int fa; // FunctionAddress
	UINT32 tailp; // TDQueueTailPointer
	UINT32 headp; // TDQueueHeadPointer
	UINT32 nexted; // NextED
	int c; // toggleCarry
	int h; // Halted
	UINT32 word0;
};

struct OHCITransferDescriptor {
	int cc; // ConditionCode
	int ec; // ErrorCount
	int t; // DataToggle
	int di; // DelayInterrupt
	int dp; // Direction/PID
	int r; // bufferRounding
	UINT32 cbp; // CurrentBufferPointer
	UINT32 nexttd; // NextTD
	UINT32 be; // BufferEnd
	UINT32 word0;
};

struct OHCIIsochronousTransferDescriptor {
	int cc; // ConditionCode
	int fc; // FrameCount
	int di; // DelayInterrupt
	int sf; // StartingFrame
	UINT32 bp0; // BufferPage0
	UINT32 nexttd; // NextTD
	UINT32 be; // BufferEnd
	UINT32 offset[8]; // Offset/PacketStatusWord
};

enum OHCIRegisters {
	HcRevision=0,
	HcControl,
	HcCommandStatus,
	HcInterruptStatus,
	HcInterruptEnable,
	HcInterruptDisable,
	HcHCCA,
	HcPeriodCurrentED,
	HcControlHeadED,
	HcControlCurrentED,
	HcBulkHeadED,
	HcBulkCurrentED,
	HcDoneHead,
	HcFmInterval,
	HcFmRemaining,
	HcFmNumber,
	HcPeriodicStart,
	HcLSThreshold,
	HcRhDescriptorA,
	HcRhDescriptorB,
	HcRhStatus,
	HcRhPortStatus1
};

enum OHCIHostControllerFunctionalState {
	UsbReset=0,
	UsbResume,
	UsbOperational,
	UsbSuspend
};

enum OHCIInterrupt {
	SchedulingOverrun=1,
	WritebackDoneHead=2,
	StartofFrame=4,
	ResumeDetected=8,
	UnrecoverableError=16,
	FrameNumberOverflow=32,
	RootHubStatusChange=64,
	OwnershipChange=0x40000000,
	MasterInterruptEnable=0x80000000
};

enum OHCICompletionCode {
	NoError=0,
	CRC,
	BitStuffing,
	DataToggleMismatch,
	Stall,
	DeviceNotResponding,
	PIDCheckFailure,
	UnexpectedPID,
	DataOverrun,
	DataUnderrun,
	BufferOverrun=12,
	BufferUnderrun,
	NotAccessed=14
};

struct USBSetupPacket {
	UINT8 bmRequestType;
	UINT8 bRequest;
	UINT16 wValue;
	UINT16 wIndex;
	UINT16 wLength;
};

struct USBStandardDeviceDscriptor {
	UINT8 bLength;
	UINT8 bDescriptorType;
	UINT16 bcdUSB;
	UINT8 bDeviceClass;
	UINT8 bDeviceSubClass;
	UINT8 bDeviceProtocol;
	UINT8 bMaxPacketSize0;
	UINT16 idVendor;
	UINT16 idProduct;
	UINT16 bcdDevice;
	UINT8 iManufacturer;
	UINT8 iProduct;
	UINT8 iSerialNumber;
	UINT8 bNumConfigurations;
};

struct USBStandardConfigurationDescriptor {
	UINT8 bLength;
	UINT8 bDescriptorType;
	UINT16 wTotalLength;
	UINT8 bNumInterfaces;
	UINT8 bConfigurationValue;
	UINT8 iConfiguration;
	UINT8 bmAttributes;
	UINT8 MaxPower;
};

struct USBStandardInterfaceDescriptor {
	UINT8 bLength;
	UINT8 bDescriptorType;
	UINT8 bInterfaceNumber;
	UINT8 bAlternateSetting;
	UINT8 bNumEndpoints;
	UINT8 bInterfaceClass;
	UINT8 bInterfaceSubClass;
	UINT8 bInterfaceProtocol;
	UINT8 iInterface;
};

struct USBStandardEndpointDescriptor {
	UINT8 bLength;
	UINT8 bDescriptorType;
	UINT8 bEndpointAddress;
	UINT8 bmAttributes;
	UINT16 wMaxPacketSize;
	UINT8 bInterval;
};

enum USBPid {
	SetupPid=0,
	OutPid,
	InPid
};

enum USBRequestCode {
	GET_STATUS=0,
	CLEAR_FEATURE=1,
	SET_FEATURE=3,
	SET_ADDRESS=5,
	GET_DESCRIPTOR=6,
	SET_DESCRIPTOR=7,
	GET_CONFIGURATION=8,
	SET_CONFIGURATION=9,
	GET_INTERFACE=10,
	SET_INTERFACE=11,
	SYNCH_FRAME=12
};

enum USBDescriptorType {
	DEVICE=1,
	CONFIGURATION=2,
	STRING=3,
	INTERFACE=4,
	ENDPOINT=5
};

class ohci_function_device {
public:
	ohci_function_device();
	void execute_reset();
	int execute_transfer(int address, int endpoint, int pid, UINT8 *buffer, int size);
private:
	int address;
	int controldir;
	int remain;
	UINT8 *position;
};

class chihiro_state : public driver_device
{
public:
	chihiro_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		nvidia_nv2a(NULL),
		debug_irq_active(false),
		debug_irq_number(0),
		dimm_board_memory(NULL),
		dimm_board_memory_size(0),
		usbhack_index(-1),
		usbhack_counter(0),
		m_maincpu(*this, "maincpu") { }

	DECLARE_READ32_MEMBER(geforce_r);
	DECLARE_WRITE32_MEMBER(geforce_w);
	DECLARE_READ32_MEMBER(usbctrl_r);
	DECLARE_WRITE32_MEMBER(usbctrl_w);
	DECLARE_READ32_MEMBER(smbus_r);
	DECLARE_WRITE32_MEMBER(smbus_w);
	DECLARE_READ32_MEMBER(mediaboard_r);
	DECLARE_WRITE32_MEMBER(mediaboard_w);
	DECLARE_READ32_MEMBER(audio_apu_r);
	DECLARE_WRITE32_MEMBER(audio_apu_w);
	DECLARE_READ32_MEMBER(audio_ac93_r);
	DECLARE_WRITE32_MEMBER(audio_ac93_w);
	DECLARE_READ32_MEMBER(dummy_r);
	DECLARE_WRITE32_MEMBER(dummy_w);

	void smbus_register_device(int address, int(*handler)(chihiro_state &chs, int command, int rw, int data));
	int smbus_pic16lc(int command, int rw, int data);
	int smbus_cx25871(int command, int rw, int data);
	int smbus_eeprom(int command, int rw, int data);
	void usb_ohci_plug(int port, ohci_function_device *function);
	void usb_ohci_interrupts();
	void usb_ohci_read_endpoint_descriptor(UINT32 address);
	void usb_ohci_writeback_endpoint_descriptor(UINT32 address);
	void usb_ohci_read_transfer_descriptor(UINT32 address);
	void usb_ohci_writeback_transfer_descriptor(UINT32 address);
	void usb_ohci_read_isochronous_transfer_descriptor(UINT32 address);
	void baseboard_ide_event(int type, UINT8 *read, UINT8 *write);
	UINT8 *baseboard_ide_dimmboard(UINT32 lba);
	void dword_write_le(UINT8 *addr, UINT32 d);
	void word_write_le(UINT8 *addr, UINT16 d);
	void debug_generate_irq(int irq, bool active);

	void vblank_callback(screen_device &screen, bool state);
	UINT32 screen_update_callback(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	virtual void machine_start();
	DECLARE_WRITE_LINE_MEMBER(chihiro_pic8259_1_set_int_line);
	DECLARE_READ8_MEMBER(get_slave_ack);
	DECLARE_WRITE_LINE_MEMBER(chihiro_pit8254_out0_changed);
	DECLARE_WRITE_LINE_MEMBER(chihiro_pit8254_out2_changed);
	IRQ_CALLBACK_MEMBER(irq_callback);
	TIMER_CALLBACK_MEMBER(audio_apu_timer);
	TIMER_CALLBACK_MEMBER(usb_ohci_timer);

	struct chihiro_devices {
		pic8259_device    *pic8259_1;
		pic8259_device    *pic8259_2;
		bus_master_ide_controller_device    *ide;
		naomi_gdrom_board *dimmboard;
	} chihiro_devs;
	struct smbus_state {
		int status;
		int control;
		int address;
		int data;
		int command;
		int rw;
		int(*devices[128])(chihiro_state &chs, int command, int rw, int data);
		UINT32 words[256 / 4];
	} smbusst;
	struct apu_state {
		UINT32 memory[0x60000 / 4];
		UINT32 gpdsp_sgaddress; // global processor scatter-gather
		UINT32 gpdsp_sgblocks;
		UINT32 gpdsp_address;
		UINT32 epdsp_sgaddress; // encoder processor scatter-gather
		UINT32 epdsp_sgblocks;
		UINT32 unknown_sgaddress;
		UINT32 unknown_sgblocks;
		int voice_number;
		UINT32 voices_heap_blockaddr[1024];
		UINT64 voices_active[4]; //one bit for each voice: 1 playing 0 not
		UINT32 voicedata_address;
		int voices_frequency[256]; // sample rate
		int voices_position[256]; // position in samples * 1000
		int voices_position_start[256]; // position in samples * 1000
		int voices_position_end[256]; // position in samples * 1000
		int voices_position_increment[256]; // position increment every 1ms * 1000
		emu_timer *timer;
		address_space *space;
	} apust;
	struct ac97_state {
		UINT32 mixer_regs[0x80 / 4];
		UINT32 controller_regs[0x38 / 4];
	} ac97st;
	struct ohci_state {
		UINT32 hc_regs[255];
		struct {
			ohci_function_device *function;
			int delay;
		} ports[4 + 1];
		emu_timer *timer;
		int state;
		UINT32 framenumber;
		UINT32 nextinterupted;
		UINT32 nextbulked;
		int interruptbulkratio;
		int writebackdonehadcounter;
		address_space *space;
		UINT8 buffer[1024];
		OHCIEndpointDescriptor endpoint_descriptor;
		OHCITransferDescriptor transfer_descriptor;
		OHCIIsochronousTransferDescriptor isochronous_transfer_descriptor;
	} ohcist;
	UINT8 pic16lc_buffer[0xff];
	nv2a_renderer *nvidia_nv2a;
	bool debug_irq_active;
	int debug_irq_number;
	UINT8 *dimm_board_memory;
	UINT32 dimm_board_memory_size;
	int usbhack_index;
	int usbhack_counter;
	required_device<cpu_device> m_maincpu;
};

/* jamtable instructions for Chihiro (different from console)
St.     Instr.       Comment
0x01    POKEPCI      PCICONF[OP2] := OP1
0x02    OUTB         PORT[OP2] := OP1
0x03    POKE         MEM[OP2] := OP1
0x04    BNE          IF ACC <> OP2 THEN PC := PC + OP1
0x05    PEEKPCI      ACC := PCICONF[OP2]
0x06    AND/OR       ACC := (ACC & OP2) | OP1
0x07    BRA          PC := PC + OP1
0x08    INB          ACC := PORT[OP2]
0x09    PEEK         ACC := MEM[OP2]
0xE1    (prefix)     execute the instruction code in OP2 with OP2 := OP1, OP1 := ACC
0xEE    END
*/

/* jamtable disassembler */
static void jamtable_disasm(running_machine &machine, address_space &space, UINT32 address, UINT32 size) // 0xff000080 == fff00080
{
	offs_t base, addr;
	UINT32 opcode, op1, op2;
	char sop1[16];
	char sop2[16];
	char pcrel[16];

	addr = (offs_t)address;
	if (!debug_cpu_translate(space, TRANSLATE_READ_DEBUG, &addr))
	{
		debug_console_printf(machine, "Address is unmapped.\n");
		return;
	}
	while (1)
	{
		base = addr;
		opcode = space.read_byte(addr);
		addr++;
		op1 = space.read_dword_unaligned(addr);
		addr += 4;
		op2 = space.read_dword_unaligned(addr);
		addr += 4;
		if (opcode == 0xe1)
		{
			opcode = op2 & 255;
			op2 = op1;
			//op1=edi;
			sprintf(sop2, "%08X", op2);
			sprintf(sop1, "ACC");
			sprintf(pcrel, "PC+ACC");
		}
		else
		{
			sprintf(sop2, "%08X", op2);
			sprintf(sop1, "%08X", op1);
			sprintf(pcrel, "%08X", base + 9 + op1);
		}
		debug_console_printf(machine, "%08X ", base);
		// dl=instr ebx=par1 eax=par2
		switch (opcode)
		{
		case 0x01:
			// if ((op2 & 0xff) == 0x880) op1=op1 & 0xfffffffd
			// out cf8,op2
			// out cfc,op1
			// out cf8,0
			// cf8 (CONFIG_ADDRESS) format:
			// 31 30      24 23        16 15           11 10              8 7               2 1 0
			// +-+----------+------------+---------------+-----------------+-----------------+-+-+
			// | | Reserved | Bus Number | Device Number | Function Number | Register Number |0|0|
			// +-+----------+------------+---------------+-----------------+-----------------+-+-+
			// 31 - Enable bit
			debug_console_printf(machine, "POKEPCI PCICONF[%s]=%s\n", sop2, sop1);
			break;
		case 0x02:
			debug_console_printf(machine, "OUTB    PORT[%s]=%s\n", sop2, sop1);
			break;
		case 0x03:
			debug_console_printf(machine, "POKE    MEM[%s]=%s\n", sop2, sop1);
			break;
		case 0x04:
			debug_console_printf(machine, "BNE     IF ACC != %s THEN PC=%s\n", sop2, pcrel);
			break;
		case 0x05:
			// out cf8,op2
			// in acc,cfc
			debug_console_printf(machine, "PEEKPCI ACC=PCICONF[%s]\n", sop2);
			break;
		case 0x06:
			debug_console_printf(machine, "AND/OR  ACC=(ACC & %s) | %s\n", sop2, sop1);
			break;
		case 0x07:
			debug_console_printf(machine, "BRA     PC=%s\n", pcrel);
			break;
		case 0x08:
			debug_console_printf(machine, "INB     ACC=PORT[%s]\n", sop2);
			break;
		case 0x09:
			debug_console_printf(machine, "PEEK    ACC=MEM[%s]\n", sop2);
			break;
		case 0xee:
			debug_console_printf(machine, "END\n");
			break;
		default:
			debug_console_printf(machine, "NOP     ????\n");
			break;
		}
		if (opcode == 0xee)
			break;
		if (size <= 9)
			break;
		size -= 9;
	}
}

static void jamtable_disasm_command(running_machine &machine, int ref, int params, const char **param)
{
	chihiro_state *state = machine.driver_data<chihiro_state>();
	address_space &space = state->m_maincpu->space();
	UINT64  addr, size;

	if (params < 2)
		return;
	if (!debug_command_parameter_number(machine, param[0], &addr))
		return;
	if (!debug_command_parameter_number(machine, param[1], &size))
		return;
	jamtable_disasm(machine, space, (UINT32)addr, (UINT32)size);
}

static void dump_string_command(running_machine &machine, int ref, int params, const char **param)
{
	chihiro_state *state = machine.driver_data<chihiro_state>();
	address_space &space = state->m_maincpu->space();
	UINT64  addr;
	offs_t address;
	UINT32 length, maximumlength;
	offs_t buffer;

	if (params < 1)
		return;
	if (!debug_command_parameter_number(machine, param[0], &addr))
		return;
	address = (offs_t)addr;
	if (!debug_cpu_translate(space, TRANSLATE_READ_DEBUG, &address))
	{
		debug_console_printf(machine, "Address is unmapped.\n");
		return;
	}
	length = space.read_word_unaligned(address);
	maximumlength = space.read_word_unaligned(address + 2);
	buffer = space.read_dword_unaligned(address + 4);
	debug_console_printf(machine, "Length %d word\n", length);
	debug_console_printf(machine, "MaximumLength %d word\n", maximumlength);
	debug_console_printf(machine, "Buffer %08X byte* ", buffer);
	if (!debug_cpu_translate(space, TRANSLATE_READ_DEBUG, &buffer))
	{
		debug_console_printf(machine, "\nBuffer is unmapped.\n");
		return;
	}
	if (length > 256)
		length = 256;
	for (int a = 0; a < length; a++)
	{
		UINT8 c = space.read_byte(buffer + a);
		debug_console_printf(machine, "%c", c);
	}
	debug_console_printf(machine, "\n");
}

static void dump_process_command(running_machine &machine, int ref, int params, const char **param)
{
	chihiro_state *state = machine.driver_data<chihiro_state>();
	address_space &space = state->m_maincpu->space();
	UINT64 addr;
	offs_t address;

	if (params < 1)
		return;
	if (!debug_command_parameter_number(machine, param[0], &addr))
		return;
	address = (offs_t)addr;
	if (!debug_cpu_translate(space, TRANSLATE_READ_DEBUG, &address))
	{
		debug_console_printf(machine, "Address is unmapped.\n");
		return;
	}
	debug_console_printf(machine, "ReadyListHead {%08X,%08X} _LIST_ENTRY\n", space.read_dword_unaligned(address), space.read_dword_unaligned(address + 4));
	debug_console_printf(machine, "ThreadListHead {%08X,%08X} _LIST_ENTRY\n", space.read_dword_unaligned(address + 8), space.read_dword_unaligned(address + 12));
	debug_console_printf(machine, "StackCount %d dword\n", space.read_dword_unaligned(address + 16));
	debug_console_printf(machine, "ThreadQuantum %d dword\n", space.read_dword_unaligned(address + 20));
	debug_console_printf(machine, "BasePriority %d byte\n", space.read_byte(address + 24));
	debug_console_printf(machine, "DisableBoost %d byte\n", space.read_byte(address + 25));
	debug_console_printf(machine, "DisableQuantum %d byte\n", space.read_byte(address + 26));
	debug_console_printf(machine, "_padding %d byte\n", space.read_byte(address + 27));
}

static void dump_list_command(running_machine &machine, int ref, int params, const char **param)
{
	chihiro_state *state = machine.driver_data<chihiro_state>();
	address_space &space = state->m_maincpu->space();
	UINT64 addr, offs, start, old;
	offs_t address, offset;

	if (params < 1)
		return;
	if (!debug_command_parameter_number(machine, param[0], &addr))
		return;
	offs = 0;
	offset = 0;
	if (params >= 2)
	{
		if (!debug_command_parameter_number(machine, param[1], &offs))
			return;
		offset = (offs_t)offs;
	}
	start = addr;
	address = (offs_t)addr;
	if (!debug_cpu_translate(space, TRANSLATE_READ_DEBUG, &address))
	{
		debug_console_printf(machine, "Address is unmapped.\n");
		return;
	}
	if (params >= 2)
		debug_console_printf(machine, "Entry    Object\n");
	else
		debug_console_printf(machine, "Entry\n");
	for (int num = 0; num < 32; num++)
	{
		if (params >= 2)
			debug_console_printf(machine, "%08X %08X\n", (UINT32)addr, (offs_t)addr - offset);
		else
			debug_console_printf(machine, "%08X\n", (UINT32)addr);
		old = addr;
		addr = space.read_dword_unaligned(address);
		if (addr == start)
			break;
		if (addr == old)
			break;
		address = (offs_t)addr;
		if (!debug_cpu_translate(space, TRANSLATE_READ_DEBUG, &address))
			break;
	}
}

static void dump_dpc_command(running_machine &machine, int ref, int params, const char **param)
{
	chihiro_state *state = machine.driver_data<chihiro_state>();
	address_space &space = state->m_maincpu->space();
	UINT64 addr;
	offs_t address;

	if (params < 1)
		return;
	if (!debug_command_parameter_number(machine, param[0], &addr))
		return;
	address = (offs_t)addr;
	if (!debug_cpu_translate(space, TRANSLATE_READ_DEBUG, &address))
	{
		debug_console_printf(machine, "Address is unmapped.\n");
		return;
	}
	debug_console_printf(machine, "Type %d word\n", space.read_word_unaligned(address));
	debug_console_printf(machine, "Inserted %d byte\n", space.read_byte(address + 2));
	debug_console_printf(machine, "Padding %d byte\n", space.read_byte(address + 3));
	debug_console_printf(machine, "DpcListEntry {%08X,%08X} _LIST_ENTRY\n", space.read_dword_unaligned(address + 4), space.read_dword_unaligned(address + 8));
	debug_console_printf(machine, "DeferredRoutine %08X dword\n", space.read_dword_unaligned(address + 12));
	debug_console_printf(machine, "DeferredContext %08X dword\n", space.read_dword_unaligned(address + 16));
	debug_console_printf(machine, "SystemArgument1 %08X dword\n", space.read_dword_unaligned(address + 20));
	debug_console_printf(machine, "SystemArgument2 %08X dword\n", space.read_dword_unaligned(address + 24));
}

static void dump_timer_command(running_machine &machine, int ref, int params, const char **param)
{
	chihiro_state *state = machine.driver_data<chihiro_state>();
	address_space &space = state->m_maincpu->space();
	UINT64 addr;
	offs_t address;

	if (params < 1)
		return;
	if (!debug_command_parameter_number(machine, param[0], &addr))
		return;
	address = (offs_t)addr;
	if (!debug_cpu_translate(space, TRANSLATE_READ_DEBUG, &address))
	{
		debug_console_printf(machine, "Address is unmapped.\n");
		return;
	}
	debug_console_printf(machine, "Header.Type %d byte\n", space.read_byte(address));
	debug_console_printf(machine, "Header.Absolute %d byte\n", space.read_byte(address + 1));
	debug_console_printf(machine, "Header.Size %d byte\n", space.read_byte(address + 2));
	debug_console_printf(machine, "Header.Inserted %d byte\n", space.read_byte(address + 3));
	debug_console_printf(machine, "Header.SignalState %08X dword\n", space.read_dword_unaligned(address + 4));
	debug_console_printf(machine, "Header.WaitListEntry {%08X,%08X} _LIST_ENTRY\n", space.read_dword_unaligned(address + 8), space.read_dword_unaligned(address + 12));
	debug_console_printf(machine, "DueTime %" I64FMT "x qword\n", (INT64)space.read_qword_unaligned(address + 16));
	debug_console_printf(machine, "TimerListEntry {%08X,%08X} _LIST_ENTRY\n", space.read_dword_unaligned(address + 24), space.read_dword_unaligned(address + 28));
	debug_console_printf(machine, "Dpc %08X dword\n", space.read_dword_unaligned(address + 32));
	debug_console_printf(machine, "Period %d dword\n", space.read_dword_unaligned(address + 36));
}

static void curthread_command(running_machine &machine, int ref, int params, const char **param)
{
	chihiro_state *state = machine.driver_data<chihiro_state>();
	address_space &space = state->m_maincpu->space();
	UINT64 fsbase;
	UINT32 kthrd, topstack, tlsdata;
	offs_t address;

	fsbase = state->m_maincpu->state_int(44);
	address = (offs_t)fsbase + 0x28;
	if (!debug_cpu_translate(space, TRANSLATE_READ_DEBUG, &address))
	{
		debug_console_printf(machine, "Address is unmapped.\n");
		return;
	}
	kthrd = space.read_dword_unaligned(address);
	debug_console_printf(machine, "Current thread is %08X\n", kthrd);
	address = (offs_t)kthrd + 0x1c;
	if (!debug_cpu_translate(space, TRANSLATE_READ_DEBUG, &address))
		return;
	topstack = space.read_dword_unaligned(address);
	debug_console_printf(machine, "Current thread stack top is %08X\n", topstack);
	address = (offs_t)kthrd + 0x28;
	if (!debug_cpu_translate(space, TRANSLATE_READ_DEBUG, &address))
		return;
	tlsdata = space.read_dword_unaligned(address);
	if (tlsdata == 0)
		address = (offs_t)topstack - 0x210 - 8;
	else
		address = (offs_t)tlsdata - 8;
	if (!debug_cpu_translate(space, TRANSLATE_READ_DEBUG, &address))
		return;
	debug_console_printf(machine, "Current thread function is %08X\n", space.read_dword_unaligned(address));
}

static void generate_irq_command(running_machine &machine, int ref, int params, const char **param)
{
	UINT64 irq;
	chihiro_state *chst = machine.driver_data<chihiro_state>();

	if (params < 1)
		return;
	if (!debug_command_parameter_number(machine, param[0], &irq))
		return;
	if (irq > 15)
		return;
	if (irq == 2)
		return;
	chst->debug_generate_irq((int)irq, true);
}

static void nv2a_combiners_command(running_machine &machine, int ref, int params, const char **param)
{
	int en;

	chihiro_state *chst = machine.driver_data<chihiro_state>();
	en = chst->nvidia_nv2a->toggle_register_combiners_usage();
	if (en != 0)
		debug_console_printf(machine, "Register combiners enabled\n");
	else
		debug_console_printf(machine, "Register combiners disabled\n");
}

static void waitvblank_command(running_machine &machine, int ref, int params, const char **param)
{
	int en;

	chihiro_state *chst = machine.driver_data<chihiro_state>();
	en = chst->nvidia_nv2a->toggle_wait_vblank_support();
	if (en != 0)
		debug_console_printf(machine, "Vblank method enabled\n");
	else
		debug_console_printf(machine, "Vblank method disabled\n");
}

static void grab_texture_command(running_machine &machine, int ref, int params, const char **param)
{
	UINT64 type;
	chihiro_state *chst = machine.driver_data<chihiro_state>();

	if (params < 2)
		return;
	if (!debug_command_parameter_number(machine, param[0], &type))
		return;
	if ((param[1][0] == 0) || (strlen(param[1]) > 127))
		return;
	chst->nvidia_nv2a->debug_grab_texture((int)type, param[1]);
}

static void grab_vprog_command(running_machine &machine, int ref, int params, const char **param)
{
	chihiro_state *chst = machine.driver_data<chihiro_state>();
	UINT32 instruction[4];
	FILE *fil;

	if (params < 1)
		return;
	if ((param[0][0] == 0) || (strlen(param[0]) > 127))
		return;
	if ((fil = fopen(param[0], "wb")) == NULL)
		return;
	for (int n = 0; n < 136; n++) {
		chst->nvidia_nv2a->debug_grab_vertex_program_slot(n, instruction);
		fwrite(instruction, sizeof(UINT32), 4, fil);
	}
	fclose(fil);
}

static void vprogdis_command(running_machine &machine, int ref, int params, const char **param)
{
	UINT64 address, length, type;
	UINT32 instruction[4];
	offs_t addr;
	vertex_program_disassembler vd;
	char line[64];
	chihiro_state *chst = machine.driver_data<chihiro_state>();
	address_space &space = chst->m_maincpu->space();

	if (params < 2)
		return;
	if (!debug_command_parameter_number(machine, param[0], &address))
		return;
	if (!debug_command_parameter_number(machine, param[1], &length))
		return;
	type = 0;
	if (params > 2)
		if (!debug_command_parameter_number(machine, param[2], &type))
			return;
	while (length > 0) {
		if (type == 1) {
			addr = (offs_t)address;
			if (!debug_cpu_translate(space, TRANSLATE_READ_DEBUG, &addr))
				return;
			instruction[0] = space.read_dword_unaligned(address);
			instruction[1] = space.read_dword_unaligned(address + 4);
			instruction[2] = space.read_dword_unaligned(address + 8);
			instruction[3] = space.read_dword_unaligned(address + 12);
		}
		else
			chst->nvidia_nv2a->debug_grab_vertex_program_slot((int)address, instruction);
		while (vd.disassemble(instruction, line) != 0)
			debug_console_printf(machine, "%s\n", line);
		if (type == 1)
			address = address + 4 * 4;
		else
			address++;
		length--;
	}
}

static void help_command(running_machine &machine, int ref, int params, const char **param)
{
	debug_console_printf(machine, "Available Chihiro commands:\n");
	debug_console_printf(machine, "  chihiro jamdis,<start>,<size> -- Disassemble <size> bytes of JamTable instructions starting at <start>\n");
	debug_console_printf(machine, "  chihiro dump_string,<address> -- Dump _STRING object at <address>\n");
	debug_console_printf(machine, "  chihiro dump_process,<address> -- Dump _PROCESS object at <address>\n");
	debug_console_printf(machine, "  chihiro dump_list,<address>[,<offset>] -- Dump _LIST_ENTRY chain starting at <address>\n");
	debug_console_printf(machine, "  chihiro dump_dpc,<address> -- Dump _KDPC object at <address>\n");
	debug_console_printf(machine, "  chihiro dump_timer,<address> -- Dump _KTIMER object at <address>\n");
	debug_console_printf(machine, "  chihiro curthread -- Print information about current thread\n");
	debug_console_printf(machine, "  chihiro irq,<number> -- Generate interrupt with irq number 0-15\n");
	debug_console_printf(machine, "  chihiro nv2a_combiners -- Toggle use of register combiners\n");
	debug_console_printf(machine, "  chihiro waitvblank -- Toggle support for wait vblank method\n");
	debug_console_printf(machine, "  chihiro grab_texture,<type>,<filename> -- Save to <filename> the next used texture of type <type>\n");
	debug_console_printf(machine, "  chihiro grab_vprog,<filename> -- save current vertex program instruction slots to <filename>\n");
	debug_console_printf(machine, "  chihiro vprogdis,<address>,<length>[,<type>] -- disassemble <lenght> vertex program instructions at <address> of <type>\n");
	debug_console_printf(machine, "  chihiro help -- this list\n");
}

static void chihiro_debug_commands(running_machine &machine, int ref, int params, const char **param)
{
	if (params < 1)
		return;
	if (strcmp("jamdis", param[0]) == 0)
		jamtable_disasm_command(machine, ref, params - 1, param + 1);
	else if (strcmp("dump_string", param[0]) == 0)
		dump_string_command(machine, ref, params - 1, param + 1);
	else if (strcmp("dump_process", param[0]) == 0)
		dump_process_command(machine, ref, params - 1, param + 1);
	else if (strcmp("dump_list", param[0]) == 0)
		dump_list_command(machine, ref, params - 1, param + 1);
	else if (strcmp("dump_dpc", param[0]) == 0)
		dump_dpc_command(machine, ref, params - 1, param + 1);
	else if (strcmp("dump_timer", param[0]) == 0)
		dump_timer_command(machine, ref, params - 1, param + 1);
	else if (strcmp("curthread", param[0]) == 0)
		curthread_command(machine, ref, params - 1, param + 1);
	else if (strcmp("irq", param[0]) == 0)
		generate_irq_command(machine, ref, params - 1, param + 1);
	else if (strcmp("nv2a_combiners", param[0]) == 0)
		nv2a_combiners_command(machine, ref, params - 1, param + 1);
	else if (strcmp("waitvblank", param[0]) == 0)
		waitvblank_command(machine, ref, params - 1, param + 1);
	else if (strcmp("grab_texture", param[0]) == 0)
		grab_texture_command(machine, ref, params - 1, param + 1);
	else if (strcmp("grab_vprog", param[0]) == 0)
		grab_vprog_command(machine, ref, params - 1, param + 1);
	else if (strcmp("vprogdis", param[0]) == 0)
		vprogdis_command(machine, ref, params - 1, param + 1);
	else
		help_command(machine, ref, params - 1, param + 1);
}

void chihiro_state::debug_generate_irq(int irq, bool active)
{
	int state;

	if (active)
	{
		debug_irq_active = true;
		debug_irq_number = irq;
		state = 1;
	}
	else
	{
		debug_irq_active = false;
		state = 0;
	}
	switch (irq)
	{
	case 0:
		chihiro_devs.pic8259_1->ir0_w(state);
		break;
	case 1:
		chihiro_devs.pic8259_1->ir1_w(state);
		break;
	case 3:
		chihiro_devs.pic8259_1->ir3_w(state);
		break;
	case 4:
		chihiro_devs.pic8259_1->ir4_w(state);
		break;
	case 5:
		chihiro_devs.pic8259_1->ir5_w(state);
		break;
	case 6:
		chihiro_devs.pic8259_1->ir6_w(state);
		break;
	case 7:
		chihiro_devs.pic8259_1->ir7_w(state);
		break;
	case 8:
		chihiro_devs.pic8259_2->ir0_w(state);
		break;
	case 9:
		chihiro_devs.pic8259_2->ir1_w(state);
		break;
	case 10:
		chihiro_devs.pic8259_2->ir2_w(state);
		break;
	case 11:
		chihiro_devs.pic8259_2->ir3_w(state);
		break;
	case 12:
		chihiro_devs.pic8259_2->ir4_w(state);
		break;
	case 13:
		chihiro_devs.pic8259_2->ir5_w(state);
		break;
	case 14:
		chihiro_devs.pic8259_2->ir6_w(state);
		break;
	case 15:
		chihiro_devs.pic8259_2->ir7_w(state);
		break;
	}
}

void chihiro_state::vblank_callback(screen_device &screen, bool state)
{
	if (nvidia_nv2a->vblank_callback(screen, state))
		chihiro_devs.pic8259_1->ir3_w(1); // IRQ 3
	else
		chihiro_devs.pic8259_1->ir3_w(0); // IRQ 3
}

UINT32 chihiro_state::screen_update_callback(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return nvidia_nv2a->screen_update_callback(screen, bitmap, cliprect);
}

READ32_MEMBER(chihiro_state::geforce_r)
{
	return nvidia_nv2a->geforce_r(space, offset, mem_mask);
}

WRITE32_MEMBER(chihiro_state::geforce_w)
{
	nvidia_nv2a->geforce_w(space, offset, data, mem_mask);
}

static UINT32 geforce_pci_r(device_t *busdevice, device_t *device, int function, int reg, UINT32 mem_mask)
{
#ifdef LOG_PCI
	//  logerror("  bus:1 device:NV_2A function:%d register:%d mask:%08X\n",function,reg,mem_mask);
#endif
	return 0;
}

static void geforce_pci_w(device_t *busdevice, device_t *device, int function, int reg, UINT32 data, UINT32 mem_mask)
{
#ifdef LOG_PCI
	//  logerror("  bus:1 device:NV_2A function:%d register:%d data:%08X mask:%08X\n",function,reg,data,mem_mask);
#endif
}

/*
 * ohci usb controller (placeholder for now)
 */

#ifdef LOG_OHCI
static const char *const usbregnames[] = {
	"HcRevision",
	"HcControl",
	"HcCommandStatus",
	"HcInterruptStatus",
	"HcInterruptEnable",
	"HcInterruptDisable",
	"HcHCCA",
	"HcPeriodCurrentED",
	"HcControlHeadED",
	"HcControlCurrentED",
	"HcBulkHeadED",
	"HcBulkCurrentED",
	"HcDoneHead",
	"HcFmInterval",
	"HcFmRemaining",
	"HcFmNumber",
	"HcPeriodicStart",
	"HcLSThreshold",
	"HcRhDescriptorA",
	"HcRhDescriptorB",
	"HcRhStatus",
	"HcRhPortStatus[1]"
};
#endif

static const struct {
	const char *game_name;
	struct {
		UINT32 address;
		UINT8 write_byte;
	} modify[16];
} hacks[2] = { { "chihiro", { { 0x6a79f, 0x01 }, { 0x6a7a0, 0x00 }, { 0x6b575, 0x00 }, { 0x6b576, 0x00 }, { 0x6b5af, 0x75 }, { 0x6b78a, 0x75 }, { 0x6b7ca, 0x00 }, { 0x6b7b8, 0x00 }, { 0x8f5b2, 0x75 }, { 0x79a9e, 0x74 }, { 0x79b80, 0x74 }, { 0x79b97, 0x74 }, { 0, 0 } } },
				{ "outr2", { { 0x12e4cf, 0x01 }, { 0x12e4d0, 0x00 }, { 0x4793e, 0x01 }, { 0x4793f, 0x00 }, { 0x47aa3, 0x01 }, { 0x47aa4, 0x00 }, { 0x14f2b6, 0x84 }, { 0x14f2d1, 0x75 }, { 0x8732f, 0x7d }, { 0x87384, 0x7d }, { 0x87388, 0xeb }, { 0, 0 } } } };

READ32_MEMBER(chihiro_state::usbctrl_r)
{
	UINT32 ret;

#ifdef LOG_OHCI
	if (offset >= 0x54 / 4)
		logerror("usb controller 0 register HcRhPortStatus[%d] read\n", (offset - 0x54 / 4) + 1);
	else
		logerror("usb controller 0 register %s read\n", usbregnames[offset]);
#endif
	ret=ohcist.hc_regs[offset];
	if (offset == 0) { /* hacks needed until usb (and jvs) is implemented */
#ifdef USB_ENABLED
#else
		int p;

		if (usbhack_counter == 0)
			p = 0;
		else if (usbhack_counter == 1) // after game loaded
			p = usbhack_index;
		else
			p = -1;
		if (p >= 0) {
			for (int a = 0; a < 16; a++) {
				if (hacks[p].modify[a].address == 0)
					break;
				m_maincpu->space(0).write_byte(hacks[p].modify[a].address, hacks[p].modify[a].write_byte);
			}
		}
		usbhack_counter++;
#endif
	}
	return ret;
}

WRITE32_MEMBER(chihiro_state::usbctrl_w)
{
#ifdef USB_ENABLED
	UINT32 old = ohcist.hc_regs[offset];
#endif

#ifdef LOG_OHCI
	if (offset >= 0x54 / 4)
		logerror("usb controller 0 register HcRhPortStatus[%d] write %08X\n", (offset - 0x54 / 4) + 1, data);
	else
		logerror("usb controller 0 register %s write %08X\n", usbregnames[offset], data);
#endif
#ifdef USB_ENABLED
	if (offset == HcRhStatus) {
		if (data & 0x80000000)
			ohcist.hc_regs[HcRhStatus] &= ~0x8000;
		if (data & 0x00020000)
			ohcist.hc_regs[HcRhStatus] &= ~0x0002;
		if (data & 0x00010000)
			ohcist.hc_regs[HcRhStatus] &= ~0x0001;
		return;
	}
	if (offset == HcControl) {
		int hcfs;

		hcfs = (data >> 6) & 3;
		if (hcfs == UsbOperational) {
			ohcist.timer->enable();
			ohcist.timer->adjust(attotime::from_msec(1), 0, attotime::from_msec(1));
			ohcist.writebackdonehadcounter = 7;
		}
		else
			ohcist.timer->enable(false);
		ohcist.state = hcfs;
		ohcist.interruptbulkratio = (data & 3) + 1;
	}
	if (offset == HcCommandStatus) {
		if (data & 1)
			ohcist.hc_regs[HcControl] |= 3 << 6;
		ohcist.hc_regs[HcCommandStatus] |= data;
		return;
	}
	if (offset == HcInterruptStatus) {
		ohcist.hc_regs[HcInterruptStatus] &= ~data;
		usb_ohci_interrupts();
		return;
	}
	if (offset == HcInterruptEnable) {
		ohcist.hc_regs[HcInterruptEnable] |= data;
		usb_ohci_interrupts();
		return;
	}
	if (offset == HcInterruptDisable) {
		ohcist.hc_regs[HcInterruptEnable] &= ~data;
		usb_ohci_interrupts();
		return;
	}
	if (offset >= HcRhPortStatus1) {
		int port = offset - HcRhPortStatus1 + 1; // port 0 not used
		// bit 0 ClearPortEnable: 1 clears PortEnableStatus
		// bit 1 SetPortEnable: 1 sets PortEnableStatus
		// bit 2 SetPortSuspend: 1 sets PortSuspendStatus
		// bit 3 ClearSuspendStatus: 1 clears PortSuspendStatus
		// bit 4 SetPortReset: 1 sets PortResetStatus
		if (data & 0x10) {
			ohcist.hc_regs[offset] |= 0x10;
			ohcist.ports[port].function->execute_reset();
			// after 10ms set PortResetStatusChange and clear PortResetStatus and set PortEnableStatus
			ohcist.ports[port].delay = 10;
		}
		// bit 8 SetPortPower: 1 sets PortPowerStatus
		// bit 9 ClearPortPower: 1 clears PortPowerStatus
		// bit 16 1 clears ConnectStatusChange
		// bit 17 1 clears PortEnableStatusChange
		// bit 18 1 clears PortSuspendStatusChange
		// bit 19 1 clears PortOverCurrentIndicatorChange
		// bit 20 1 clears PortResetStatusChange
		if (ohcist.hc_regs[offset] != old)
			ohcist.hc_regs[HcInterruptStatus] |= RootHubStatusChange;
		usb_ohci_interrupts();
		return;
	}
#endif
	ohcist.hc_regs[offset] = data;
}

TIMER_CALLBACK_MEMBER(chihiro_state::usb_ohci_timer)
{
	UINT32 hcca;
	int changed = 0;
	int list = 1;
	bool cont = false;
	int pid, remain, mps;

	hcca = ohcist.hc_regs[HcHCCA];
	if (ohcist.state == UsbOperational) {
		// increment frame number
		ohcist.framenumber = (ohcist.framenumber + 1) & 0xffff;
		ohcist.space->write_dword(hcca + 0x80, ohcist.framenumber);
		ohcist.hc_regs[HcFmNumber] = ohcist.framenumber;
	}
	// port reset delay
	for (int p = 1; p <= 4; p++) {
		if (ohcist.ports[p].delay > 0) {
			ohcist.ports[p].delay--;
			if (ohcist.ports[p].delay == 0) {
				ohcist.hc_regs[HcRhPortStatus1 + p - 1] = (ohcist.hc_regs[HcRhPortStatus1 + p - 1] & ~(1 << 4)) | (1 << 20) | (1 << 1); // bit 1 PortEnableStatus
				changed = 1;
			}
		}
	}
	if (ohcist.state == UsbOperational) {
		while (list >= 0)
		{
			// select list, do transfer
			if (list == 0) {
				if (ohcist.hc_regs[HcControl] & (1 << 2)) {
					// periodic
					if (ohcist.hc_regs[HcControl] & (1 << 3)) {
						// isochronous
					}
				}
				list = -1;
			}
			if (list == 1) {
				// control
				if (ohcist.hc_regs[HcControl] & (1 << 4)) {
					cont = true;
					while (cont == true) {
						// if current endpoint descriptor is not 0 use it, otherwise ...
						if (ohcist.hc_regs[HcControlCurrentED] == 0) {
							// ... check the filled bit ...
							if (ohcist.hc_regs[HcCommandStatus] & (1 << 1)) {
								// ... if 1 start processing from the head of the list
								ohcist.hc_regs[HcControlCurrentED] = ohcist.hc_regs[HcControlHeadED];
								ohcist.hc_regs[HcCommandStatus] &= ~(1 << 1);
								// but if the list is empty, go to the next list
								if (ohcist.hc_regs[HcControlCurrentED] == 0)
									cont = false;
							}
							else
								cont = false;
						}
						if (cont == true) {
							// service endpoint descriptor
							usb_ohci_read_endpoint_descriptor(ohcist.hc_regs[HcControlCurrentED]);
							// only if it is not halted and not to be skipped
							if (!(ohcist.endpoint_descriptor.h | ohcist.endpoint_descriptor.k)) {
								// compare the Endpoint Descriptor’s TailPointer and NextTransferDescriptor fields.
								if (ohcist.endpoint_descriptor.headp != ohcist.endpoint_descriptor.tailp) {
									UINT32 a, b;
									// service transfer descriptor
									usb_ohci_read_transfer_descriptor(ohcist.endpoint_descriptor.headp);
									// get pid
									if (ohcist.endpoint_descriptor.d == 1)
										pid=OutPid; // out
									else if (ohcist.endpoint_descriptor.d == 2)
										pid=InPid; // in
									else {
										pid = ohcist.transfer_descriptor.dp; // 0 setup 1 out 2 in
									}
									// determine how much data to transfer
									// setup pid must be 8 bytes
									a = ohcist.transfer_descriptor.be & 0xfff;
									b = ohcist.transfer_descriptor.cbp & 0xfff;
									if ((ohcist.transfer_descriptor.be ^ ohcist.transfer_descriptor.cbp) & 0xfffff000)
										a |= 0x1000;
									remain = a - b + 1;
									if (pid == InPid) {
										mps = ohcist.endpoint_descriptor.mps;
										if (remain < mps)
											mps = remain;
									}
									else {
										mps = ohcist.endpoint_descriptor.mps;
									}
									if (ohcist.transfer_descriptor.cbp == 0)
										mps = 0;
									b = ohcist.transfer_descriptor.cbp;
									// if sending ...
									if (pid != InPid) {
										// ... get mps bytes
										for (int c = 0; c < mps; c++) {
											ohcist.buffer[c] = ohcist.space->read_byte(b);
											b++;
											if ((b & 0xfff) == 0)
												b = ohcist.transfer_descriptor.be & 0xfffff000;
										}
									}
									// should check for time available
									// execute transaction
									mps=ohcist.ports[1].function->execute_transfer(ohcist.endpoint_descriptor.fa, ohcist.endpoint_descriptor.en, pid, ohcist.buffer, mps);
									// if receiving ...
									if (pid == InPid) {
										// ... store mps bytes
										for (int c = 0; c < mps; c++) {
											ohcist.space->write_byte(b,ohcist.buffer[c]);
											b++;
											if ((b & 0xfff) == 0)
												b = ohcist.transfer_descriptor.be & 0xfffff000;
										}
									}
									// status writeback (CompletionCode field, DataToggleControl field, CurrentBufferPointer field, ErrorCount field)
									ohcist.transfer_descriptor.cc = NoError;
									ohcist.transfer_descriptor.t = (ohcist.transfer_descriptor.t ^ 1) | 2;
									ohcist.transfer_descriptor.cbp = b;
									ohcist.transfer_descriptor.ec = 0;
									if ((remain == mps) || (mps == 0)) {
										// retire transfer descriptor
										a = ohcist.endpoint_descriptor.headp;
										ohcist.endpoint_descriptor.headp = ohcist.transfer_descriptor.nexttd;
										ohcist.transfer_descriptor.nexttd = ohcist.hc_regs[HcDoneHead];
										ohcist.hc_regs[HcDoneHead] = a;
										ohcist.endpoint_descriptor.c = ohcist.transfer_descriptor.t & 1;
										if (ohcist.transfer_descriptor.di != 7) {
											if (ohcist.transfer_descriptor.di < ohcist.writebackdonehadcounter)
												ohcist.writebackdonehadcounter = ohcist.transfer_descriptor.di;
										}
										usb_ohci_writeback_transfer_descriptor(a);
										usb_ohci_writeback_endpoint_descriptor(ohcist.hc_regs[HcControlCurrentED]);
									} else {
										usb_ohci_writeback_transfer_descriptor(ohcist.endpoint_descriptor.headp);
									}
								} else
									ohcist.hc_regs[HcControlCurrentED] = ohcist.endpoint_descriptor.nexted;
							} else
								ohcist.hc_regs[HcControlCurrentED] = ohcist.endpoint_descriptor.nexted;
							// one bulk every n control transfers
							ohcist.interruptbulkratio--;
							if (ohcist.interruptbulkratio <= 0) {
								ohcist.interruptbulkratio = (ohcist.hc_regs[HcControl] & 3) + 1;
								cont = false;
							}
						}
					}
				}
				list = 2;
			}
			if (list == 2) {
				// bulk
				if (ohcist.hc_regs[HcControl] & (1 << 5)) {
					ohcist.hc_regs[HcCommandStatus] &= ~(1 << 2);
					if (ohcist.hc_regs[HcControlCurrentED] == 0)
						list = 0;
					else if (ohcist.hc_regs[HcControl] & (1 << 4))
						list = 1;
					else
						list = 0;
				}
			}
		}
		if (ohcist.framenumber == 0)
			ohcist.hc_regs[HcInterruptStatus] |= FrameNumberOverflow;
		ohcist.hc_regs[HcInterruptStatus] |= StartofFrame;
		if ((ohcist.writebackdonehadcounter != 0) && (ohcist.writebackdonehadcounter != 7))
			ohcist.writebackdonehadcounter--;
		if ((ohcist.writebackdonehadcounter == 0) && ((ohcist.hc_regs[HcInterruptStatus] & WritebackDoneHead) == 0)) {
			UINT32 b = 0;

			if ((ohcist.hc_regs[HcInterruptStatus] & ohcist.hc_regs[HcInterruptEnable]) != WritebackDoneHead)
				b = 1;
			ohcist.hc_regs[HcInterruptStatus] |= WritebackDoneHead;
			ohcist.space->write_dword(hcca + 0x84, ohcist.hc_regs[HcDoneHead] | b);
			ohcist.hc_regs[HcDoneHead] = 0;
			ohcist.writebackdonehadcounter = 7;
		}
	}
	if (changed != 0) {
		ohcist.hc_regs[HcInterruptStatus] |= RootHubStatusChange;
	}
	usb_ohci_interrupts();
}

void chihiro_state::usb_ohci_plug(int port, ohci_function_device *function)
{
	if ((port > 0) && (port <= 4)) {
		ohcist.ports[port].function = function;
		ohcist.hc_regs[HcRhPortStatus1+port-1] = 1;
	}
}

static USBStandardDeviceDscriptor devdesc = {18,1,0x201,0xff,0x34,0x56,64,0x100,0x101,0x301,0,0,0,1};

ohci_function_device::ohci_function_device()
{
	address = 0;
	controldir = 0;
	remain = 0;
	position = NULL;
}

void ohci_function_device::execute_reset()
{
	address = 0;
}

int ohci_function_device::execute_transfer(int address, int endpoint, int pid, UINT8 *buffer, int size)
{
	if (endpoint == 0) {
		if (pid == SetupPid) {
			struct USBSetupPacket *p=(struct USBSetupPacket *)buffer;
			// define direction
			controldir = p->bmRequestType & 128;
			// case !=0, in data stage and out status stage
			// case ==0, out data stage and in status stage
			position = NULL;
			remain = p->wLength;
			if ((p->bmRequestType & 0x60) == 0) {
				switch (p->bRequest) {
				case GET_DESCRIPTOR:
					if ((p->wValue >> 8) == 1) { // device descriptor
						//p->wValue & 255;
						position = (UINT8 *)&devdesc;
						remain = sizeof(devdesc);
					}
					break;
				case SET_ADDRESS:
					//p->wValue;
					break;
				default:
					break;
				}
			}
		}
		else if (pid == InPid) {
			// case !=0, give data
			// case ==0, nothing
			if (size > remain)
				size = remain;
			if (controldir != 0) {
				if (position != NULL)
					memcpy(buffer, position, size);
				position = position + size;
				remain = remain - size;
			}
		}
		else if (pid == OutPid) {
			// case !=0, nothing
			// case ==0, give data
			if (size > remain)
				size = remain;
			if (controldir == 0) {
				if (position != NULL)
					memcpy(position, buffer, size);
				position = position + size;
				remain = remain - size;
			}
		}
	}
	return size;
}

void chihiro_state::usb_ohci_interrupts()
{
	if (((ohcist.hc_regs[HcInterruptStatus] & ohcist.hc_regs[HcInterruptEnable]) != 0) && ((ohcist.hc_regs[HcInterruptEnable] & MasterInterruptEnable) != 0))
		chihiro_devs.pic8259_1->ir1_w(1);
	else
		chihiro_devs.pic8259_1->ir1_w(0);
}

void chihiro_state::usb_ohci_read_endpoint_descriptor(UINT32 address)
{
	UINT32 w;

	w = ohcist.space->read_dword(address);
	ohcist.endpoint_descriptor.word0 = w;
	ohcist.endpoint_descriptor.fa = w & 0x7f;
	ohcist.endpoint_descriptor.en = (w >> 7) & 15;
	ohcist.endpoint_descriptor.d = (w >> 11) & 3;
	ohcist.endpoint_descriptor.s = (w >> 13) & 1;
	ohcist.endpoint_descriptor.k = (w >> 14) & 1;
	ohcist.endpoint_descriptor.f = (w >> 15) & 1;
	ohcist.endpoint_descriptor.mps = (w >> 16) & 0x7ff;
	ohcist.endpoint_descriptor.tailp = ohcist.space->read_dword(address + 4);
	w = ohcist.space->read_dword(address + 8);
	ohcist.endpoint_descriptor.headp = w & 0xfffffffc;
	ohcist.endpoint_descriptor.h = w & 1;
	ohcist.endpoint_descriptor.c = (w >> 1) & 1;
	ohcist.endpoint_descriptor.nexted = ohcist.space->read_dword(address + 12);
}

void chihiro_state::usb_ohci_writeback_endpoint_descriptor(UINT32 address)
{
	UINT32 w;

	w = ohcist.endpoint_descriptor.word0 & 0xf8000000;
	w = w | (ohcist.endpoint_descriptor.mps << 16) | (ohcist.endpoint_descriptor.f << 15) | (ohcist.endpoint_descriptor.k << 14) | (ohcist.endpoint_descriptor.s << 13) | (ohcist.endpoint_descriptor.d << 11) | (ohcist.endpoint_descriptor.en << 7) | ohcist.endpoint_descriptor.fa;
	ohcist.space->write_dword(address, w);
	w = ohcist.endpoint_descriptor.headp | (ohcist.endpoint_descriptor.c << 1) | ohcist.endpoint_descriptor.h;
	ohcist.space->write_dword(address + 8, w);
}

void chihiro_state::usb_ohci_read_transfer_descriptor(UINT32 address)
{
	UINT32 w;

	w = ohcist.space->read_dword(address);
	ohcist.transfer_descriptor.word0 = w;
	ohcist.transfer_descriptor.cc = (w >> 28) & 15;
	ohcist.transfer_descriptor.ec= (w >> 26) & 3;
	ohcist.transfer_descriptor.t= (w >> 24) & 3;
	ohcist.transfer_descriptor.di= (w >> 21) & 7;
	ohcist.transfer_descriptor.dp= (w >> 19) & 3;
	ohcist.transfer_descriptor.r = (w >> 18) & 1;
	ohcist.transfer_descriptor.cbp = ohcist.space->read_dword(address + 4);
	ohcist.transfer_descriptor.nexttd = ohcist.space->read_dword(address + 8);
	ohcist.transfer_descriptor.be = ohcist.space->read_dword(address + 12);
}

void chihiro_state::usb_ohci_writeback_transfer_descriptor(UINT32 address)
{
	UINT32 w;

	w = ohcist.transfer_descriptor.word0 & 0x0003ffff;
	w = w | (ohcist.transfer_descriptor.cc << 28) | (ohcist.transfer_descriptor.ec << 26) | (ohcist.transfer_descriptor.t << 24) | (ohcist.transfer_descriptor.di << 21) | (ohcist.transfer_descriptor.dp << 19) | (ohcist.transfer_descriptor.r << 18);
	ohcist.space->write_dword(address, w);
	ohcist.space->write_dword(address + 4, ohcist.transfer_descriptor.cbp);
	ohcist.space->write_dword(address + 8, ohcist.transfer_descriptor.nexttd);
}

void chihiro_state::usb_ohci_read_isochronous_transfer_descriptor(UINT32 address)
{
	UINT32 w;

	w = ohcist.space->read_dword(address);
	ohcist.isochronous_transfer_descriptor.cc = (w >> 28) & 15;
	ohcist.isochronous_transfer_descriptor.fc = (w >> 24) & 7;
	ohcist.isochronous_transfer_descriptor.di = (w >> 21) & 7;
	ohcist.isochronous_transfer_descriptor.sf = w & 0xffff;
	ohcist.isochronous_transfer_descriptor.bp0 = ohcist.space->read_dword(address + 4) & 0xfffff000;
	ohcist.isochronous_transfer_descriptor.nexttd = ohcist.space->read_dword(address + 8);
	ohcist.isochronous_transfer_descriptor.be = ohcist.space->read_dword(address + 12);
	w = ohcist.space->read_dword(address + 16);
	ohcist.isochronous_transfer_descriptor.offset[0] = w & 0xffff;
	ohcist.isochronous_transfer_descriptor.offset[1] = (w >> 16) & 0xffff;
	w = ohcist.space->read_dword(address + 20);
	ohcist.isochronous_transfer_descriptor.offset[2] = w & 0xffff;
	ohcist.isochronous_transfer_descriptor.offset[3] = (w >> 16) & 0xffff;
	w = ohcist.space->read_dword(address + 24);
	ohcist.isochronous_transfer_descriptor.offset[4] = w & 0xffff;
	ohcist.isochronous_transfer_descriptor.offset[5] = (w >> 16) & 0xffff;
	w = ohcist.space->read_dword(address + 28);
	ohcist.isochronous_transfer_descriptor.offset[6] = w & 0xffff;
	ohcist.isochronous_transfer_descriptor.offset[7] = (w >> 16) & 0xffff;
}

/*
 * Audio
 */

READ32_MEMBER(chihiro_state::audio_apu_r)
{
	logerror("Audio_APU: read from %08X mask %08X\n", 0xfe800000 + offset * 4, mem_mask);
	if (offset == 0x20010 / 4) // some kind of internal counter or state value
		return 0x20 + 4 + 8 + 0x48 + 0x80;
	return apust.memory[offset];
}

WRITE32_MEMBER(chihiro_state::audio_apu_w)
{
	//UINT32 old;
	UINT32 v;

	logerror("Audio_APU: write at %08X mask %08X value %08X\n", 0xfe800000 + offset * 4, mem_mask, data);
	//old = apust.memory[offset];
	apust.memory[offset] = data;
	if (offset == 0x02040 / 4) // address of memory area with scatter-gather info (gpdsp scratch dma)
		apust.gpdsp_sgaddress = data;
	if (offset == 0x020d4 / 4) { // block count (gpdsp)
		apust.gpdsp_sgblocks = data;
		apust.gpdsp_address = apust.space->read_dword(apust.gpdsp_sgaddress); // memory address of first block
		apust.timer->enable();
		apust.timer->adjust(attotime::from_msec(1), 0, attotime::from_msec(1));
	}
	if (offset == 0x02048 / 4) // (epdsp scratch dma)
		apust.epdsp_sgaddress = data;
	if (offset == 0x020dc / 4) // (epdsp)
		apust.epdsp_sgblocks = data;
	if (offset == 0x0204c / 4) // address of memory area with information about blocks
		apust.unknown_sgaddress = data;
	if (offset == 0x020e0 / 4) // block count - 1
		apust.unknown_sgblocks = data;
	if (offset == 0x0202c / 4) { // address of memory area with 0x80 bytes for each voice
		apust.voicedata_address = data;
		return;
	}
	if (offset == 0x04024 / 4) // offset in memory area indicated by 0x204c (analog output ?)
		return;
	if (offset == 0x04034 / 4) // size
		return;
	if (offset == 0x04028 / 4) // offset in memory area indicated by 0x204c (digital output ?)
		return;
	if (offset == 0x04038 / 4) // size
		return;
	if (offset == 0x20804 / 4) { // block number for scatter-gather heap that stores sampled audio to be played
		if (data >= 1024) {
			logerror("Audio_APU: sg block number too high, increase size of voices_heap_blockaddr\n");
			apust.memory[offset] = 1023;
		}
		return;
	}
	if (offset == 0x20808 / 4) { // block address for scatter-gather heap that stores sampled audio to be played
		apust.voices_heap_blockaddr[apust.memory[0x20804 / 4]] = data;
		return;
	}
	if (offset == 0x202f8 / 4) { // voice number for parameters ?
		apust.voice_number = data;
		return;
	}
	if (offset == 0x202fc / 4) // 1 when accessing voice parameters 0 otherwise
		return;
	if (offset == 0x20304 / 4) { // format
		/*
		bits 28-31 sample format:
		0  8-bit pcm
		5  16-bit pcm
		10 adpcm ?
		14 24-bit pcm
		15 32-bit pcm
		bits 16-20 number of channels - 1:
		0  mono
		1  stereo
		*/
		return;
	}
	if (offset == 0x2037c / 4) { // value related to sample rate
		INT16 v = (INT16)(data >> 16); // upper 16 bits as a signed 16 bit value
		float vv = ((float)v) / 4096.0f; // divide by 4096
		float vvv = powf(2, vv); // two to the vv
		int f = vvv*48000.0f; // sample rate
		apust.voices_frequency[apust.voice_number] = f;
		return;
	}
	if (offset == 0x203a0 / 4) // start offset of data in scatter-gather heap
		return;
	if (offset == 0x203a4 / 4) { // first sample to play
		apust.voices_position_start[apust.voice_number] = data * 1000;
		return;
	}
	if (offset == 0x203dc / 4) { // last sample to play
		apust.voices_position_end[apust.voice_number] = data * 1000;
		return;
	}
	if (offset == 0x2010c / 4) // voice processor 0 idle 1 not idle ?
		return;
	if (offset == 0x20124 / 4) { // voice number to activate ?
		v = apust.voice_number;
		apust.voices_active[v >> 6] |= ((UINT64)1 << (v & 63));
		apust.voices_position[v] = apust.voices_position_start[apust.voice_number];
		apust.voices_position_increment[apust.voice_number] = apust.voices_frequency[apust.voice_number];
		return;
	}
	if (offset == 0x20128 / 4) { // voice number to deactivate ?
		v = apust.voice_number;
		apust.voices_active[v >> 6] &= ~(1 << (v & 63));
		return;
	}
	if (offset == 0x20140 / 4) // voice number to ?
		return;
	if ((offset >= 0x20200 / 4) && (offset < 0x20280 / 4)) // headroom for each of the 32 mixbins
		return;
	if (offset == 0x20280 / 4) // hrtf headroom ?
		return;
}

READ32_MEMBER(chihiro_state::audio_ac93_r)
{
	UINT32 ret = 0;

	logerror("Audio_AC3: read from %08X mask %08X\n", 0xfec00000 + offset * 4, mem_mask);
	if (offset < 0x80 / 4)
	{
		ret = ac97st.mixer_regs[offset];
	}
	if ((offset >= 0x100 / 4) && (offset <= 0x138 / 4))
	{
		offset = offset - 0x100 / 4;
		if (offset == 0x18 / 4)
		{
			ac97st.controller_regs[offset] &= ~0x02000000; // REGRST: register reset
		}
		if (offset == 0x30 / 4)
		{
			ac97st.controller_regs[offset] |= 0x100; // PCRDY: primary codec ready
		}
		if (offset == 0x34 / 4)
		{
			ac97st.controller_regs[offset] &= ~1; // CAS: codec access semaphore
		}
		ret = ac97st.controller_regs[offset];
	}
	return ret;
}

WRITE32_MEMBER(chihiro_state::audio_ac93_w)
{
	logerror("Audio_AC3: write at %08X mask %08X value %08X\n", 0xfec00000 + offset * 4, mem_mask, data);
	if (offset < 0x80 / 4)
	{
		COMBINE_DATA(ac97st.mixer_regs + offset);
	}
	if ((offset >= 0x100 / 4) && (offset <= 0x138 / 4))
	{
		offset = offset - 0x100 / 4;
		COMBINE_DATA(ac97st.controller_regs + offset);
	}
}

TIMER_CALLBACK_MEMBER(chihiro_state::audio_apu_timer)
{
	int cmd;
	int bb, b, v;
	UINT64 bv;
	UINT32 phys;

	cmd = apust.space->read_dword(apust.gpdsp_address + 0x800 + 0x10);
	if (cmd == 3)
		apust.space->write_dword(apust.gpdsp_address + 0x800 + 0x10, 0);
	/*else
	logerror("Audio_APU: unexpected value at address %d\n",apust.gpdsp_address+0x800+0x10);*/
	for (b = 0; b < 4; b++) {
		bv = 1;
		for (bb = 0; bb < 64; bb++) {
			if (apust.voices_active[b] & bv) {
				v = bb + (b << 6);
				apust.voices_position[v] += apust.voices_position_increment[v];
				while (apust.voices_position[v] >= apust.voices_position_end[v])
					apust.voices_position[v] = apust.voices_position_start[v] + apust.voices_position[v] - apust.voices_position_end[v] - 1000;
				phys = apust.voicedata_address + 0x80 * v;
				apust.space->write_dword(phys + 0x58, apust.voices_position[v] / 1000);
			}
			bv = bv << 1;
		}
	}
}

static UINT32 hubintiasbridg_pci_r(device_t *busdevice, device_t *device, int function, int reg, UINT32 mem_mask)
{
#ifdef LOG_PCI
	//  logerror("  bus:0 function:%d register:%d mask:%08X\n",function,reg,mem_mask);
#endif
	if ((function == 0) && (reg == 8))
		return 0xb4; // 0:1:0 revision id must be at least 0xb4, otherwise usb will require a hub
	return 0;
}

static void hubintiasbridg_pci_w(device_t *busdevice, device_t *device, int function, int reg, UINT32 data, UINT32 mem_mask)
{
#ifdef LOG_PCI
	if (reg >= 16) logerror("  bus:0 function:%d register:%d data:%08X mask:%08X\n", function, reg, data, mem_mask);
#endif
}

/*
 * dummy for non connected devices
 */

static UINT32 dummy_pci_r(device_t *busdevice, device_t *device, int function, int reg, UINT32 mem_mask)
{
#ifdef LOG_PCI
	//  logerror("  bus:0 function:%d register:%d mask:%08X\n",function,reg,mem_mask);
#endif
	return 0;
}

static void dummy_pci_w(device_t *busdevice, device_t *device, int function, int reg, UINT32 data, UINT32 mem_mask)
{
#ifdef LOG_PCI
	if (reg >= 16) logerror("  bus:0 function:%d register:%d data:%08X mask:%08X\n", function, reg, data, mem_mask);
#endif
}

READ32_MEMBER(chihiro_state::dummy_r)
{
	return 0;
}

WRITE32_MEMBER(chihiro_state::dummy_w)
{
}

// ======================> ide_baseboard_device

class ide_baseboard_device : public ata_mass_storage_device
{
public:
	// construction/destruction
	ide_baseboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual int  read_sector(UINT32 lba, void *buffer);
	virtual int  write_sector(UINT32 lba, const void *buffer);
protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	UINT8 read_buffer[0x20];
	UINT8 write_buffer[0x20];
	chihiro_state *chihirosystem;
};

//**************************************************************************
//  IDE HARD DISK IMAGE DEVICE
//**************************************************************************

// device type definition
const device_type IDE_BASEBOARD = &device_creator<ide_baseboard_device>;

//-------------------------------------------------
//  ide_baseboard_device - constructor
//-------------------------------------------------

ide_baseboard_device::ide_baseboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ata_mass_storage_device(mconfig, IDE_BASEBOARD, "IDE Baseboard", tag, owner, clock, "ide_baseboard", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ide_baseboard_device::device_start()
{
	ata_mass_storage_device::device_start();
	chihirosystem = machine().driver_data<chihiro_state>();
	// savestates
	save_item(NAME(read_buffer));
	save_item(NAME(write_buffer));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ide_baseboard_device::device_reset()
{
	if (!m_can_identify_device)
	{
		m_num_cylinders = 65535;
		m_num_sectors = 255;
		m_num_heads = 255;
		ide_build_identify_device();
		m_can_identify_device = 1;
	}

	ata_mass_storage_device::device_reset();
}

int ide_baseboard_device::read_sector(UINT32 lba, void *buffer)
{
	int off;
	UINT8 *data;

	/*
	It assumes there are 4 "partitions", the size of the first one depends on bits 3-0 of io port 40f4:
	Value    Size lba
	0   0x40000-0x8000
	...
	4   0x400000-0x8000
	The size of the second one is always 0x8000 sectors, and is used as a special communication area
	This is a list of the partitions in the minimum size case:
	Name          Start lba  Size lba Size
	\??\mbfs:     0x0        0x38000  112MB
	\??\mbcom:    0x38000    0x8000   16MB
	\??\mbrom0:   0x8000000  0x800    1MB
	\??\mbrom1:   0x8000800  0x800    1MB
	This is a list of the partitions in the maximum size case:
	Name          Start lba  Size lba Size
	\??\mbfs:     0x0        0x3f8000 2032MB
	\??\mbcom:    0x3f8000   0x8000   16MB
	\??\mbrom0:   0x8000000  0x800    1MB
	\??\mbrom1:   0x8000800  0x800    1MB
	*/
	logerror("baseboard: read sector lba %08x\n", lba);
	if (lba >= 0x08000000) {
		off = (lba & 0x7ff) * 512;
		data = memregion(":others")->base();
		memcpy(buffer, data + off, 512);
		return 1;
	}
	if (lba >= 0xf8000) {
		memset(buffer, 0, 512);
		lba = lba - 0xf8000;
		if (lba == 0x4800)
			memcpy(buffer, read_buffer, 0x20);
		else if (lba == 0x4801)
			memcpy(buffer, write_buffer, 0x20);
		return 1;
	}
	// in a type 1 chihiro this gets data from the dimm board memory
	data = chihirosystem->baseboard_ide_dimmboard(lba);
	if (data != NULL)
		memcpy(buffer, data, 512);
	return 1;
}

int ide_baseboard_device::write_sector(UINT32 lba, const void *buffer)
{
	logerror("baseboard: write sector lba %08x\n", lba);
	if (lba >= 0xf8000) {
		lba = lba - 0xf8000;
		if (lba == 0x4800)
			memcpy(read_buffer, buffer, 0x20);
		else if (lba == 0x4801) {
			memcpy(write_buffer, buffer, 0x20);
			// call chihiro driver
			chihirosystem->baseboard_ide_event(3, read_buffer, write_buffer);
		}
	}
	return 1;
}

/*
 * Chihiro Type 1 baseboard
 */

void chihiro_state::dword_write_le(UINT8 *addr, UINT32 d)
{
	addr[0] = d & 255;
	addr[1] = (d >> 8) & 255;
	addr[2] = (d >> 16) & 255;
	addr[3] = (d >> 24) & 255;
}

void chihiro_state::word_write_le(UINT8 *addr, UINT16 d)
{
	addr[0] = d & 255;
	addr[1] = (d >> 8) & 255;
}

void chihiro_state::baseboard_ide_event(int type, UINT8 *read_buffer, UINT8 *write_buffer)
{
	int c;

	if ((type != 3) || ((write_buffer[0] == 0) && (write_buffer[1] == 0)))
		return;
#ifdef LOG_BASEBOARD
	logerror("Baseboard sector command:\n");
	for (int a = 0; a < 32; a++)
		logerror(" %02X", write_buffer[a]);
	logerror("\n");
#endif
	// response
	// second word 8001 (8000+counter), first word=first word of written data (command ?), second dword ?
	read_buffer[0] = write_buffer[0];
	read_buffer[1] = write_buffer[1];
	read_buffer[2] = 0x01; // write_buffer[2];
	read_buffer[3] = 0x80; // write_buffer[3] | 0x80;
	c = write_buffer[2] + (write_buffer[3] << 8); // 0001 0101 0103
	switch (c)
	{
	case 0x0001:
		// second dword
		dword_write_le(read_buffer + 4, 0x00f00000); // ?
		break;
	case 0x0100:
		// second dword third dword
		dword_write_le(read_buffer + 4, 5); // game data loading phase
		dword_write_le(read_buffer + 8, 0); // completion %
		break;
	case 0x0101:
		// third word fourth word
		word_write_le(read_buffer + 4, 0xca); // ?
		word_write_le(read_buffer + 6, 0xcb); // ?
		break;
	case 0x0102:
		// second dword
		dword_write_le(read_buffer + 4, 0); // bit 16 develop. mode
		break;
	case 0x0103:
		// dwords 1 3 4
		memcpy(read_buffer + 4, "-abc-abc12345678", 16); // ?
		break;
	}
	// clear
	write_buffer[0] = write_buffer[1] = write_buffer[2] = write_buffer[3] = 0;
	// irq 10 active
	chihiro_devs.pic8259_2->ir2_w(1);
}

UINT8 *chihiro_state::baseboard_ide_dimmboard(UINT32 lba)
{
	// return pointer to memory containing decrypted gdrom data (contains an image of a fatx partition)
	if (chihiro_devs.dimmboard != NULL)
		return dimm_board_memory + lba * 512;
	return NULL;
}

/*
 * PIC & PIT
 */

WRITE_LINE_MEMBER(chihiro_state::chihiro_pic8259_1_set_int_line)
{
	m_maincpu->set_input_line(0, state ? HOLD_LINE : CLEAR_LINE);
}

READ8_MEMBER(chihiro_state::get_slave_ack)
{
	if (offset == 2) { // IRQ = 2
		return chihiro_devs.pic8259_2->acknowledge();
	}
	return 0x00;
}

IRQ_CALLBACK_MEMBER(chihiro_state::irq_callback)
{
	int r = 0;
	r = chihiro_devs.pic8259_2->acknowledge();
	if (r == 0)
	{
		r = chihiro_devs.pic8259_1->acknowledge();
	}
	if (debug_irq_active)
		debug_generate_irq(debug_irq_number, false);
	return r;
}

WRITE_LINE_MEMBER(chihiro_state::chihiro_pit8254_out0_changed)
{
	if (chihiro_devs.pic8259_1)
	{
		chihiro_devs.pic8259_1->ir0_w(state);
	}
}

WRITE_LINE_MEMBER(chihiro_state::chihiro_pit8254_out2_changed)
{
	//chihiro_speaker_set_input( state ? 1 : 0 );
}

/*
 * SMbus devices
 */

int smbus_callback_pic16lc(chihiro_state &chs, int command, int rw, int data)
{
	return chs.smbus_pic16lc(command, rw, data);
}

int chihiro_state::smbus_pic16lc(int command, int rw, int data)
{
	if (rw == 1) { // read
		if (command == 0) {
			if (pic16lc_buffer[0] == 'D')
				pic16lc_buffer[0] = 'X';
			else if (pic16lc_buffer[0] == 'X')
				pic16lc_buffer[0] = 'B';
			else if (pic16lc_buffer[0] == 'B')
				pic16lc_buffer[0] = 'D';
		}
		logerror("pic16lc: %d %d %d\n", command, rw, pic16lc_buffer[command]);
		return pic16lc_buffer[command];
	}
	else
		if (command == 0)
			pic16lc_buffer[0] = 'B';
		else
			pic16lc_buffer[command] = (UINT8)data;
	logerror("pic16lc: %d %d %d\n", command, rw, data);
	return 0;
}

int smbus_callback_cx25871(chihiro_state &chs, int command, int rw, int data)
{
	return chs.smbus_cx25871(command, rw, data);
}

int chihiro_state::smbus_cx25871(int command, int rw, int data)
{
	logerror("cx25871: %d %d %d\n", command, rw, data);
	return 0;
}

// let's try to fake the missing eeprom
static int dummyeeprom[256]={0x94,0x18,0x10,0x59,0x83,0x58,0x15,0xDA,0xDF,0xCC,0x1D,0x78,0x20,0x8A,0x61,0xB8,0x08,0xB4,0xD6,0xA8,
	0x9E,0x77,0x9C,0xEB,0xEA,0xF8,0x93,0x6E,0x3E,0xD6,0x9C,0x49,0x6B,0xB5,0x6E,0xAB,0x6D,0xBC,0xB8,0x80,0x68,0x9D,0xAA,0xCD,0x0B,0x83,
	0x17,0xEC,0x2E,0xCE,0x35,0xA8,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x61,0x62,0x63,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0x00,0x00,
	0x4F,0x6E,0x6C,0x69,0x6E,0x65,0x6B,0x65,0x79,0x69,0x6E,0x76,0x61,0x6C,0x69,0x64,0x00,0x03,0x80,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,
	0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

int smbus_callback_eeprom(chihiro_state &chs, int command, int rw, int data)
{
	return chs.smbus_eeprom(command, rw, data);
}

int chihiro_state::smbus_eeprom(int command, int rw, int data)
{
	if (command >= 112)
		return 0;
	if (rw == 1) { // if reading
		// 8003b744,3b744=0x90 0x90
		// hack to avoid hanging if eeprom contents are not correct
		// this would need dumping the serial eeprom on the xbox board
		if (command == 0) {
			m_maincpu->space(0).write_byte(0x3b744, 0x90);
			m_maincpu->space(0).write_byte(0x3b745, 0x90);
			m_maincpu->space(0).write_byte(0x3b766, 0xc9);
			m_maincpu->space(0).write_byte(0x3b767, 0xc3);
		}
		data = dummyeeprom[command] + dummyeeprom[command + 1] * 256;
		logerror("eeprom: %d %d %d\n", command, rw, data);
		return data;
	}
	logerror("eeprom: %d %d %d\n", command, rw, data);
	dummyeeprom[command] = data;
	return 0;
}

/*
 * SMbus controller
 */

void chihiro_state::smbus_register_device(int address, int(*handler)(chihiro_state &chs, int command, int rw, int data))
{
	if (address < 128)
		smbusst.devices[address] = handler;
}

READ32_MEMBER(chihiro_state::smbus_r)
{
	if ((offset == 0) && (mem_mask == 0xff)) // 0 smbus status
		smbusst.words[offset] = (smbusst.words[offset] & ~mem_mask) | ((smbusst.status << 0) & mem_mask);
	if ((offset == 1) && ((mem_mask == 0x00ff0000) || (mem_mask == 0xffff0000))) // 6 smbus data
		smbusst.words[offset] = (smbusst.words[offset] & ~mem_mask) | ((smbusst.data << 16) & mem_mask);
	return smbusst.words[offset];
}

WRITE32_MEMBER(chihiro_state::smbus_w)
{
	COMBINE_DATA(smbusst.words);
	if ((offset == 0) && (mem_mask == 0xff)) // 0 smbus status
	{
		if (!((smbusst.status ^ data) & 0x10)) // clearing interrupt
			chihiro_devs.pic8259_2->ir3_w(0); // IRQ 11
		smbusst.status &= ~data;
	}
	if ((offset == 0) && (mem_mask == 0xff0000)) // 2 smbus control
	{
		data = data >> 16;
		smbusst.control = data;
		int cycletype = smbusst.control & 7;
		if (smbusst.control & 8) { // start
			if ((cycletype & 6) == 2)
			{
				if (smbusst.devices[smbusst.address])
					if (smbusst.rw == 0)
						smbusst.devices[smbusst.address](*this, smbusst.command, smbusst.rw, smbusst.data);
					else
						smbusst.data = smbusst.devices[smbusst.address](*this, smbusst.command, smbusst.rw, smbusst.data);
				else
					logerror("SMBUS: access to missing device at address %d\n", smbusst.address);
				smbusst.status |= 0x10;
				if (smbusst.control & 0x10)
				{
					chihiro_devs.pic8259_2->ir3_w(1); // IRQ 11
				}
			}
		}
	}
	if ((offset == 1) && (mem_mask == 0xff)) // 4 smbus address
	{
		smbusst.address = data >> 1;
		smbusst.rw = data & 1;
	}
	if ((offset == 1) && ((mem_mask == 0x00ff0000) || (mem_mask == 0xffff0000))) // 6 smbus data
	{
		data = data >> 16;
		smbusst.data = data;
	}
	if ((offset == 2) && (mem_mask == 0xff)) // 8 smbus command
		smbusst.command = data;
}

READ32_MEMBER(chihiro_state::mediaboard_r)
{
	UINT32 r;

	logerror("I/O port read %04x mask %08X\n", offset * 4 + 0x4000, mem_mask);
	r = 0;
	if ((offset == 7) && ACCESSING_BITS_16_31)
		r = 0x10000000;
	if ((offset == 8) && ACCESSING_BITS_0_15)
		r = 0x000000a0;
	if ((offset == 8) && ACCESSING_BITS_16_31)
		r = 0x42580000;
	if ((offset == 9) && ACCESSING_BITS_0_15)
		r = 0x00004d41;
	if ((offset == 0x3c) && ACCESSING_BITS_0_15)
		r = 0x00000000; // bits 15-0 0 if media board present
	if ((offset == 0x3d) && ACCESSING_BITS_0_15)
		r = 0x00000002; // bits 3-0 size of dimm board memory. Must be 2
	return r;
}

WRITE32_MEMBER(chihiro_state::mediaboard_w)
{
	logerror("I/O port write %04x mask %08X value %08X\n", offset * 4 + 0x4000, mem_mask, data);
	// irq 10
	if ((offset == 0x38) && ACCESSING_BITS_8_15)
		chihiro_devs.pic8259_2->ir2_w(0);
}

static ADDRESS_MAP_START(xbox_map, AS_PROGRAM, 32, chihiro_state)
	AM_RANGE(0x00000000, 0x07ffffff) AM_RAM // 128 megabytes
	AM_RANGE(0xf0000000, 0xf0ffffff) AM_RAM
	AM_RANGE(0xfd000000, 0xfdffffff) AM_RAM AM_READWRITE(geforce_r, geforce_w)
	AM_RANGE(0xfed00000, 0xfed003ff) AM_READWRITE(usbctrl_r, usbctrl_w)
	AM_RANGE(0xfe800000, 0xfe85ffff) AM_READWRITE(audio_apu_r, audio_apu_w)
	AM_RANGE(0xfec00000, 0xfec001ff) AM_READWRITE(audio_ac93_r, audio_ac93_w)
	AM_RANGE(0xff000000, 0xffffffff) AM_ROM AM_REGION("bios", 0) AM_MIRROR(0x00f80000)
ADDRESS_MAP_END

static ADDRESS_MAP_START(xbox_map_io, AS_IO, 32, chihiro_state)
	AM_RANGE(0x0020, 0x0023) AM_DEVREADWRITE8("pic8259_1", pic8259_device, read, write, 0xffffffff)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE8("pit8254", pit8254_device, read, write, 0xffffffff)
	AM_RANGE(0x00a0, 0x00a3) AM_DEVREADWRITE8("pic8259_2", pic8259_device, read, write, 0xffffffff)
	AM_RANGE(0x01f0, 0x01f7) AM_DEVREADWRITE("ide", bus_master_ide_controller_device, read_cs0, write_cs0)
	AM_RANGE(0x0cf8, 0x0cff) AM_DEVREADWRITE("pcibus", pci_bus_legacy_device, read, write)
	AM_RANGE(0x4000, 0x40ff) AM_READWRITE(mediaboard_r, mediaboard_w)
	AM_RANGE(0x8000, 0x80ff) AM_READWRITE(dummy_r, dummy_w)
	AM_RANGE(0xc000, 0xc0ff) AM_READWRITE(smbus_r, smbus_w)
	AM_RANGE(0xff60, 0xff67) AM_DEVREADWRITE("ide", bus_master_ide_controller_device, bmdma_r, bmdma_w)
ADDRESS_MAP_END

static INPUT_PORTS_START(chihiro)
INPUT_PORTS_END

void chihiro_state::machine_start()
{
	nvidia_nv2a = auto_alloc(machine(), nv2a_renderer(machine()));
	memset(pic16lc_buffer, 0, sizeof(pic16lc_buffer));
	pic16lc_buffer[0] = 'B';
	pic16lc_buffer[4] = 0; // A/V connector, 2=vga
	smbus_register_device(0x10, smbus_callback_pic16lc);
	smbus_register_device(0x45, smbus_callback_cx25871);
	smbus_register_device(0x54, smbus_callback_eeprom);
	chihiro_devs.pic8259_1 = machine().device<pic8259_device>("pic8259_1");
	chihiro_devs.pic8259_2 = machine().device<pic8259_device>("pic8259_2");
	chihiro_devs.ide = machine().device<bus_master_ide_controller_device>("ide");
	chihiro_devs.dimmboard = machine().device<naomi_gdrom_board>("rom_board");
	if (chihiro_devs.dimmboard != NULL) {
		dimm_board_memory = chihiro_devs.dimmboard->memory(dimm_board_memory_size);
	}
	memset(apust.memory, 0, sizeof(apust.memory));
	memset(apust.voices_heap_blockaddr, 0, sizeof(apust.voices_heap_blockaddr));
	memset(apust.voices_active, 0, sizeof(apust.voices_active));
	memset(apust.voices_position, 0, sizeof(apust.voices_position));
	memset(apust.voices_position_start, 0, sizeof(apust.voices_position_start));
	memset(apust.voices_position_end, 0, sizeof(apust.voices_position_end));
	memset(apust.voices_position_increment, 0, sizeof(apust.voices_position_increment));
	apust.space = &m_maincpu->space();
	apust.timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(chihiro_state::audio_apu_timer), this), (void *)"APU Timer");
	apust.timer->enable(false);
	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
		debug_console_register_command(machine(), "chihiro", CMDFLAG_NONE, 0, 1, 4, chihiro_debug_commands);
	memset(&ohcist, 0, sizeof(ohcist));
#ifdef USB_ENABLED
	ohcist.hc_regs[HcRevision] = 0x10;
	ohcist.hc_regs[HcFmInterval] = 0x2edf;
	ohcist.hc_regs[HcLSThreshold] = 0x628;
	ohcist.hc_regs[HcRhDescriptorA] = 4;
	ohcist.interruptbulkratio = 1;
	ohcist.writebackdonehadcounter = 7;
	ohcist.space = &m_maincpu->space();
	ohcist.timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(chihiro_state::usb_ohci_timer), this), (void *)"USB OHCI Timer");
	ohcist.timer->enable(false);
	usb_ohci_plug(1, new ohci_function_device()); // test connect
#endif
	usbhack_index = -1;
	for (int a = 1; a < 2; a++)
		if (strcmp(machine().basename(), hacks[a].game_name) == 0) {
			usbhack_index = a;
			break;
		}
	usbhack_counter = 0;
	// savestates
	save_item(NAME(debug_irq_active));
	save_item(NAME(debug_irq_number));
	save_item(NAME(smbusst.status));
	save_item(NAME(smbusst.control));
	save_item(NAME(smbusst.address));
	save_item(NAME(smbusst.data));
	save_item(NAME(smbusst.command));
	save_item(NAME(smbusst.rw));
	save_item(NAME(smbusst.words));
	save_item(NAME(pic16lc_buffer));
	save_item(NAME(usbhack_counter));
	nvidia_nv2a->start(&m_maincpu->space());
	nvidia_nv2a->savestate_items();
}

static SLOT_INTERFACE_START(ide_baseboard)
	SLOT_INTERFACE("bb", IDE_BASEBOARD)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START(chihiro_base, chihiro_state)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PENTIUM3, 733333333) /* Wrong! family 6 model 8 stepping 10 */
	MCFG_CPU_PROGRAM_MAP(xbox_map)
	MCFG_CPU_IO_MAP(xbox_map_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(chihiro_state, irq_callback)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	MCFG_PCI_BUS_LEGACY_ADD("pcibus", 0)
	MCFG_PCI_BUS_LEGACY_DEVICE(0, "PCI Bridge Device - Host Bridge", dummy_pci_r, dummy_pci_w)
	MCFG_PCI_BUS_LEGACY_DEVICE(1, "HUB Interface - ISA Bridge", hubintiasbridg_pci_r, hubintiasbridg_pci_w)
	MCFG_PCI_BUS_LEGACY_DEVICE(2, "OHCI USB Controller 1", dummy_pci_r, dummy_pci_w)
	MCFG_PCI_BUS_LEGACY_DEVICE(3, "OHCI USB Controller 2", dummy_pci_r, dummy_pci_w)
	MCFG_PCI_BUS_LEGACY_DEVICE(4, "MCP Networking Adapter", dummy_pci_r, dummy_pci_w)
	MCFG_PCI_BUS_LEGACY_DEVICE(5, "MCP APU", dummy_pci_r, dummy_pci_w)
	MCFG_PCI_BUS_LEGACY_DEVICE(6, "AC`97 Audio Codec Interface", dummy_pci_r, dummy_pci_w)
	MCFG_PCI_BUS_LEGACY_DEVICE(9, "IDE Controller", dummy_pci_r, dummy_pci_w)
	MCFG_PCI_BUS_LEGACY_DEVICE(30, "AGP Host to PCI Bridge", dummy_pci_r, dummy_pci_w)
	MCFG_PCI_BUS_LEGACY_ADD("agpbus", 1)
	MCFG_PCI_BUS_LEGACY_SIBLING("pcibus")
	MCFG_PCI_BUS_LEGACY_DEVICE(0, "NV2A GeForce 3MX Integrated GPU/Northbridge", geforce_pci_r, geforce_pci_w)
	MCFG_PIC8259_ADD("pic8259_1", WRITELINE(chihiro_state, chihiro_pic8259_1_set_int_line), VCC, READ8(chihiro_state, get_slave_ack))
	MCFG_PIC8259_ADD("pic8259_2", DEVWRITELINE("pic8259_1", pic8259_device, ir2_w), GND, NULL)

	MCFG_DEVICE_ADD("pit8254", PIT8254, 0)
	MCFG_PIT8253_CLK0(1125000) /* heartbeat IRQ */
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(chihiro_state, chihiro_pit8254_out0_changed))
	MCFG_PIT8253_CLK1(1125000) /* (unused) dram refresh */
	MCFG_PIT8253_CLK2(1125000) /* (unused) pio port c pin 4, and speaker polling enough */
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(chihiro_state, chihiro_pit8254_out2_changed))

	MCFG_BUS_MASTER_IDE_CONTROLLER_ADD("ide", ide_baseboard, NULL, "bb", true)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(DEVWRITELINE("pic8259_2", pic8259_device, ir6_w))
	MCFG_BUS_MASTER_IDE_CONTROLLER_SPACE("maincpu", AS_PROGRAM)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))  /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 479)
	MCFG_SCREEN_UPDATE_DRIVER(chihiro_state, screen_update_callback)
	MCFG_SCREEN_VBLANK_DRIVER(chihiro_state, vblank_callback)

	MCFG_PALETTE_ADD("palette", 65536)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED(chihirogd, chihiro_base)
	MCFG_NAOMI_GDROM_BOARD_ADD("rom_board", ":gdrom", ":pic", NULL, NOOP)
MACHINE_CONFIG_END

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_BIOS(bios+1)) /* Note '+1' */

#define ROM_LOAD_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_BIOS(bios+1)) /* Note '+1' */

#define CHIHIRO_BIOS \
	ROM_REGION( 0x1000000, "bios", 0) \
	ROM_SYSTEM_BIOS( 0, "bios0", "Chihiro Bios" ) \
	ROM_LOAD_BIOS( 0,  "chihiro_xbox_bios.bin", 0x000000, 0x80000, CRC(66232714) SHA1(b700b0041af8f84835e45d1d1250247bf7077188) ) \
	ROM_REGION( 0x404080, "others", 0) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0,  "fpr21042_m29w160et.bin", 0x000000, 0x200000, CRC(a4fcab0b) SHA1(a13cf9c5cdfe8605d82150b7573652f419b30197) ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0,  "ic10_g24lc64.bin", 0x200000, 0x2000, CRC(cfc5e06f) SHA1(3ababd4334d8d57abb22dd98bd2d347df39648d9) ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0,  "ic11_24lc024.bin", 0x202000, 0x80, CRC(8dc8374e) SHA1(cc03a0650bfac4bf6cb66e414bbef121cba53efe) ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0,  "pc20_g24lc64.bin", 0x202080, 0x2000, CRC(7742ab62) SHA1(82dad6e2a75bab4a4840dc6939462f1fb9b95101) ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0,  "ver1305.bin", 0x204080, 0x200000, CRC(a738ea1c) SHA1(45d94d0c39be1cb3db9fab6610a88a550adda4e9) )

ROM_START( chihiro )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
ROM_END

/*
Title             THE HOUSE OF THE DEAD 3
Media ID          2673
Media Config      GD-ROM1/1
Regions           J
Peripheral String 0000000
Product Number    GDX-0001
Version           V1.004
Release Date      20021029
Manufacturer ID
TOC DISC
Track        Start Sector  End Sector  Track Size
track01.bin           150         599     1058400
track02.raw           750        2101     3179904
track03.bin         45150      549299  1185760800

PIC
253-5508-0348
317-0348-com
*/
ROM_START( hotd3 )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0001", 0, BAD_DUMP  SHA1(174c72f851d0c97e8993227467f16b0781ed2f5c) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0348-com.data", 0x00, 0x50, CRC(d28219ef) SHA1(40dbbc092bc9f99b8d2ae67fbefacd62184f90ec) )
ROM_END

/*
Title             CRAZY TAXI HIGHROLLER
Media ID          0D2E
Media Config      GD-ROM1/1
Regions           J
Peripheral String 0000000
Product Number    GDX-0002B
Version           V3.000
Release Date      20030224
Manufacturer ID
TOC DISC
Track        Start Sector  End Sector  Track Size
track01.bin           150         599     1058400
track02.raw           750        2101     3179904
track03.bin         45150      549299  1185760800

PIC
253-5508-0353
317-0353-COM
*/
ROM_START( crtaxihr )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0002b", 0, SHA1(e77d31aea9d4bf150e427aecf29b97855c2096f6) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD( "317-0353-com.pic", 0x000000, 0x004000, CRC(1c6830b1) SHA1(75be47441783c18ee296209a34c432864deed70d) )
ROM_END

/*
Title             VIRTUA COP 3
Media ID          C4AD
Media Config      GD-ROM1/1
Regions           J
Peripheral String 0000000
Product Number    GDX-0003A
Version           V2.004
Release Date      20030226
Manufacturer ID
TOC DISC
Track        Start Sector  End Sector  Track Size
track01.bin           150         599     1058400
track02.raw           750        2101     3179904
track03.bin         45150      549299  1185760800

PIC
255-5508-354
317-0354-COM
*/
ROM_START( vcop3 )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0003a", 0, SHA1(04cd12bec50a9e9f1f05e7b7c2ef396800a385dd) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0354-com.data", 0x00, 0x50,  CRC(df7e3217) SHA1(9f0f4bf6b15f3b6eeea81eaa27b3d25bd94110da) )
ROM_END

ROM_START( outr2 )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0004a", 0, SHA1(055a13a5dc4f54e6b6bdf5ce29dbda14cc9741d7) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0372-com.data", 0x00, 0x50, CRC(a15c9666) SHA1(fd36c524744acb33e579ccb257c71375a5d3af67) )
ROM_END

ROM_START( mj2c )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0006c", 0, BAD_DUMP SHA1(505653117a73ed8b256ccf19450e7573a4dc57e9) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD( "317-0374-jpn.pic", 0x000000, 0x004000, CRC(004f77a1) SHA1(bc5c6950293f3bff60bf7913d20a2046aa19ea69) )
ROM_END

ROM_START( mj2 )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0006f", 0, SHA1(d3900ca5135f9001e642c78b4d323d353880b41b) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD( "317-0374-jpn.pic", 0x000000, 0x004000, CRC(004f77a1) SHA1(bc5c6950293f3bff60bf7913d20a2046aa19ea69) )
ROM_END

/*
Title             MJ2
Media ID          3580
Media Config      GD-ROM1/1
Regions           J
Peripheral String 0000000
Product Number    GDX-0006G
Version           V8.000
Release Date      20050202
Manufacturer ID
TOC DISC
Track        Start Sector  End Sector  Track Size
track01.bin           150         599     1058400
track02.raw 750 2101    3179904
track03.bin 45150   549299  1185760800
*/
ROM_START( mj2g )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	// this is not CHDv4, but a really bad dump, only ~1/3 of disk content is dumped
	DISK_IMAGE_READONLY( "gdx-0006g", 0, BAD_DUMP SHA1(e306837d5c093fdf1e9ff02239a8563535b1c181) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD( "317-0374-jpn.pic", 0x000000, 0x004000, CRC(004f77a1) SHA1(bc5c6950293f3bff60bf7913d20a2046aa19ea69) )
ROM_END

ROM_START( ollie )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0007", 0, BAD_DUMP SHA1(8898a571a427936bffcecd3ef27cb79087d22798) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0377-com.data", 0x00, 0x50, CRC(d2a8b31f) SHA1(e9ee2df30031826db6bc4bd91969e6680255dcf9) )
ROM_END

ROM_START( wangmid )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0009b", 0, SHA1(6fcbebb95b53eaabbc5da6ee08fbe15c2922b8d4) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-5101-com.data", 0x00, 0x50, CRC(3af801f3) SHA1(e9a2558930f3f1f55d5b3c2cadad69329d931f26) )
ROM_END

ROM_START( ghostsqo )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0012", 0, SHA1(ad5d08cc3b8cfd0890feb152670b429c28659512) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0398-com.data", 0x00, 0x50, CRC(8c5391a2) SHA1(e64cadeb30c94c3cd4002630cd79cc76c7bde2ed) )
ROM_END

/*
Title             GHOST SQUAD
Media ID          004F
Media Config      GD-ROM1/1
Regions           J
Peripheral String 0000000
Product Number    GDX-0012A
Version           V2.000
Release Date      20041209
Manufacturer ID
BHU.BIN
970efe79ce32ab4a
PIC
253-5508-0398
317-0398-COM
*/
ROM_START( ghostsqu )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0012a", 0, BAD_DUMP  SHA1(d7d78ce4992cb16ee5b4ac6ca7a37c46b07e8c14) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0398-com.data", 0x00, 0x50, CRC(8c5391a2) SHA1(e64cadeb30c94c3cd4002630cd79cc76c7bde2ed) )
ROM_END

ROM_START( gundamos )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0013", 0, BAD_DUMP SHA1(96b3dafcc2d2d6803fe3bf43a245d43ee5e0e5a6) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0400-jpn.data", 0x00, 0x50, CRC(0479c383) SHA1(7e86a037d2f9d09cec61a38cb19de510bf9482b3) )
ROM_END

ROM_START( outr2st )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0014a", 0, BAD_DUMP SHA1(4f9656634c47631f63eab554a13d19b15558217e) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)   // number was not readable on pic, please fix if known
	ROM_LOAD( "317-0xxx-com.pic", 0x000000, 0x004000, CRC(f94cf26f) SHA1(dd4af2b52935c7b2d8cd196ec1a30c0ef0993322) )
ROM_END

ROM_START( wangmid2 )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0015", 0, BAD_DUMP SHA1(259483fd211a70c23205ffd852316d616c5a2740) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-5106-jpn.data", 0x00, 0x50, CRC(75c716aa) SHA1(5c2bcf3d28a80b336c6882d5aeb010d04327f8c1) )
ROM_END

ROM_START( wangmd2b )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0016a", 0, BAD_DUMP SHA1(cb306df60550bbd8df312634cb97014bb39f1631) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-5106-com.data", 0x00, 0x50, CRC(75c716aa) SHA1(5c2bcf3d28a80b336c6882d5aeb010d04327f8c1) )
ROM_END

ROM_START( mj3d )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0017d", 0, BAD_DUMP SHA1(cfbbd452c8f4efe0e99f398f5521fc3574b913bb) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD( "317-0414-jpn.pic", 0x000000, 0x004000, CRC(27d1c541) SHA1(c85a8229dd769af02ab43c97f09f995743cdb315) )
ROM_END

ROM_START( mj3 )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0017f", 0, SHA1(8641be9b2e1d8eb33cf27d3444956c0117debc2f) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD( "317-0414-jpn.pic", 0x000000, 0x004000, CRC(27d1c541) SHA1(c85a8229dd769af02ab43c97f09f995743cdb315) )
ROM_END

ROM_START( scg06nt )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0018a", 0, BAD_DUMP SHA1(e6f3dc8066392854ad7d83f81d3cbc81a5e340b3) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("gdx-0018.data", 0x00, 0x50, CRC(1a210abd) SHA1(43a54d028315d2dfa9f8ea6fb59265e0b980b02f) )
ROM_END

ROM_START( mj3evo )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0021b", 0, SHA1(c97d1dc95cdf1b4bd5d7cf6b4db0757f3d6bd723) )

	// PIC label is unknown
	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD( "317-xxxx-jpn.pic", 0x000000, 0x004000, CRC(650fcc94) SHA1(c88488900460fb3deecb3cf376fc043b10c020ef) )
ROM_END

/*
Title             BOX GDROM CF-BOX FIRM
Media ID          EB08
Media Config      GD-ROM1/1
Regions           J
Peripheral String 0000000
Product Number    GDX-0024A
Version           V2.000
Release Date      20090331
Manufacturer ID
TOC DISC
Track        Start Sector  End Sector  Track Size
track01.bin           150        8740    20206032
track02.raw          8891       10242     3179904
track03.bin         45150      549299  1185760800
*/
ROM_START( ccfboxa )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0024a", 0, SHA1(79d8c0faeec7cf6882f014760b8af938800b7e52) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	//PIC16C621A (317-0567-EXP)
	//(sticker 253-5508-0567)
	ROM_LOAD("317-0567-exp.pic", 0x00, 0x4000, CRC(cd1d2b2d) SHA1(78203ee0339f76eb76da08d7de43e7e44e4b7d32) )
ROM_END


/* Main board */
/*Chihiro*/ GAME( 2002, chihiro,  0,        chihiro_base, chihiro, driver_device, 0, ROT0, "Sega",                     "Chihiro Bios", GAME_NO_SOUND|GAME_NOT_WORKING|GAME_IS_BIOS_ROOT )

/* GDX-xxxx (Sega GD-ROM games) */
/* 0001  */ GAME( 2002, hotd3,    chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega / Wow Entertainment", "The House of the Dead III (GDX-0001)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0002     GAME( 2003, crtaxhro, crtaxihr, chihirogd,    chihiro, driver_device, 0, ROT0, "Sega / Hitmaker",          "Crazy Taxi High Roller (GDX-0002)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0002A    GAME( 2003, crtaxhra, crtaxihr, chihirogd,    chihiro, driver_device, 0, ROT0, "Sega / Hitmaker",          "Crazy Taxi High Roller (Rev A) (GDX-0002A)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0002B */ GAME( 2003, crtaxihr, chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega / Hitmaker",          "Crazy Taxi High Roller (Rev B) (GDX-0002B)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0003     GAME( 2003, vcop3o,   vcop3,    chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Virtua Cop 3 (GDX-0003)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0003A */ GAME( 2003, vcop3,    chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Virtua Cop 3 (Rev A) (GDX-0003A)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0004     GAME( 2003, outr2o,   outr2,    chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "OutRun 2 (GDX-0004)", GAME_NO_SOUND|GAME_NOT_WORKING|GAME_SUPPORTS_SAVE )
/* 0004A */ GAME( 2003, outr2,    chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "OutRun 2 (Rev A) (GDX-0004A)", GAME_NO_SOUND|GAME_NOT_WORKING|GAME_SUPPORTS_SAVE )
// 0005     GAME( 2004, sgolcnpt, chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Golf Club Network Pro Tour (GDX-0005)", GAME_NO_SOUND|GAME_NOT_WORKING|GAME_SUPPORTS_SAVE )
// 0006     GAME( 2004, mj2o,     mj2,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (GDX-0006)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0006A    GAME( 2004, mj2a,     mj2,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (Rev A) (GDX-0006A)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0006B    GAME( 2004, mj2b,     mj2,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (Rev B) (GDX-0006B)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0006C */ GAME( 2004, mj2c,     mj2,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (Rev C) (GDX-0006C)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0006D    GAME( 2004, mj2d,     mj2,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (Rev D) (GDX-0006D)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0006E    GAME( 2004, mj2e,     mj2,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (Rev E) (GDX-0006E)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0006F */ GAME( 2004, mj2,      chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (Rev F) (GDX-0006F)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0006G */ GAME( 2004, mj2g,     mj2,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (Rev G) (GDX-0006G)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0007  */ GAME( 2004, ollie,    chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega / Amusement Vision",  "Ollie King (GDX-0007)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0008     GAME( 2004, wangmdjo, wangmidj, chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",                    "Wangan Midnight Maximum Tune (Japan) (GDX-0008)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0008A    GAME( 2004, wangmdja, wangmidj, chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",                    "Wangan Midnight Maximum Tune (Japan) (Rev A) (GDX-0008A)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0008B    GAME( 2004, wangmidj, wangmidj, chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",                    "Wangan Midnight Maximum Tune (Japan) (Rev B) (GDX-0008B)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0009     GAME( 2004, wangmido, wangmid,  chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",                    "Wangan Midnight Maximum Tune (Export) (GDX-0009)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0009A    GAME( 2004, wangmida, wangmid,  chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",                    "Wangan Midnight Maximum Tune (Export) (Rev A) (GDX-0009A)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0009B */ GAME( 2004, wangmid,  chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",                    "Wangan Midnight Maximum Tune (Export) (Rev B) (GDX-0009B)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0010
// 0011     GAME( 2004, outr2sp,  chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "OutRun 2 SP (Japan) (GDX-0011)", GAME_NO_SOUND|GAME_NOT_WORKING|GAME_SUPPORTS_SAVE )
/* 0012  */ GAME( 2004, ghostsqo, ghostsqu, chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Ghost Squad (GDX-0012)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0012A */ GAME( 2004, ghostsqu, chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Ghost Squad (Rev A) (GDX-0012A)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0013  */ GAME( 2005, gundamos, chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Banpresto",                "Gundam Battle Operating Simulator (GDX-0013)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0014     GAME( 2004, outr2sto, outr2st,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "OutRun 2 Special Tours (GDX-0014)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0014A */ GAME( 2004, outr2st,  chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "OutRun 2 Special Tours (Rev A) (GDX-0014A)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0015  */ GAME( 2005, wangmid2, chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",                    "Wangan Midnight Maximum Tune 2 (Japan) (GDX-0015)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0016     GAME( 2005, wanmd2bo, wangmd2b, chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",                    "Wangan Midnight Maximum Tune 2 (Export) (GDX-0016)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0016A */ GAME( 2005, wangmd2b, chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",                    "Wangan Midnight Maximum Tune 2 (Export) (Rev A) (GDX-0016A)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0017     GAME( 2005, mj3o,     mj3,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 (GDX-0017)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0017A    GAME( 2005, mj3a,     mj3,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 (Rev A) (GDX-0017A)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0017B    GAME( 2005, mj3b,     mj3,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 (Rev B) (GDX-0017B)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0017C    GAME( 2005, mj3c,     mj3,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 (Rev C) (GDX-0017C)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0017D */ GAME( 2005, mj3d,     mj3,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 (Rev D) (GDX-0017D)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0017E    GAME( 2005, mj3e,     mj3,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 (Rev E) (GDX-0017E)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0017F */ GAME( 2005, mj3,      chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 (Rev F) (GDX-0017F)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0018     GAME( 2005, scg06nto, scg06nt,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Club Golf 2006 Next Tours (GDX-0018)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0018A */ GAME( 2005, scg06nt,  chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Club Golf 2006 Next Tours (Rev A) (GDX-0018A)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0019
// 0020
// 0021     GAME( 2006, mj3evoo,  mj3evo,    chihirogd,   chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 Evolution (GDX-0021)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0021A    GAME( 2006, mj3evoa,  mj3evo,    chihirogd,   chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 Evolution (Rev A) (GDX-0021A)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0021B */ GAME( 2007, mj3evo,   chihiro,   chihirogd,   chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 Evolution (Rev B) (GDX-0021B)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0022
// 0023
// 0024     GAME( 2009, ccfboxo,  ccfboxa,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Chihiro Firmware Update For Compact Flash Box (GDX-0024)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0024A */ GAME( 2009, ccfboxa,  chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Chihiro Firmware Update For Compact Flash Box (Rev A) (GDX-0024A)", GAME_NO_SOUND|GAME_NOT_WORKING )

#if (defined(__MINGW32__) && (__GNUC__ >= 5))
#pragma GCC diagnostic pop
#endif
