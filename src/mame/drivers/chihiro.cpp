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
| | 2003 | OutRun 2 prototype (Rev P)                         | Sega                     | GDROM  | GDX-0004P  |              |
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
|*| 2005 | Wangan Midnight Maximum Tune 2 (Export) (Rev A)    | Namco                    | GDROM  | GDX-0016A  | 317-5106-COM |
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
|*| 2007 | Mobile Suit Gundam 0083 Card Builder               | Dimps - Banpresto        | DVDROM | CDV-10030  |              |
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
#include "machine/pic8259.h"
#include "machine/idectrl.h"
#include "machine/idehd.h"
#include "machine/naomigd.h"
#include "video/poly.h"
#include "debug/debugcon.h"
#include "debug/debugcmd.h"
#include "debug/debugcpu.h"
#include "includes/chihiro.h"
#include "includes/xbox.h"

#define LOG_PCI
//#define LOG_BASEBOARD

class chihiro_state : public xbox_base_state
{
public:
	chihiro_state(const machine_config &mconfig, device_type type, std::string tag) :
		xbox_base_state(mconfig, type, tag),
		usbhack_index(-1),
		usbhack_counter(0),
		dimm_board_memory(nullptr),
		dimm_board_memory_size(0) { }

	DECLARE_READ32_MEMBER(mediaboard_r);
	DECLARE_WRITE32_MEMBER(mediaboard_w);

	virtual void machine_start() override;
	void baseboard_ide_event(int type, UINT8 *read, UINT8 *write);
	UINT8 *baseboard_ide_dimmboard(UINT32 lba);
	void dword_write_le(UINT8 *addr, UINT32 d);
	void word_write_le(UINT8 *addr, UINT16 d);
	virtual void hack_eeprom() override;
	virtual void hack_usb() override;

	struct chihiro_devices {
		bus_master_ide_controller_device    *ide;
		naomi_gdrom_board *dimmboard;
	} chihiro_devs;
	int usbhack_index;
	int usbhack_counter;
	UINT8 *dimm_board_memory;
	UINT32 dimm_board_memory_size;
};

/* jamtable instructions for Chihiro (different from Xbox console)
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

static void help_command(running_machine &machine, int ref, int params, const char **param)
{
	debug_console_printf(machine, "Available Chihiro commands:\n");
	debug_console_printf(machine, "  chihiro jamdis,<start>,<size> -- Disassemble <size> bytes of JamTable instructions starting at <start>\n");
	debug_console_printf(machine, "  chihiro help -- this list\n");
}

static void chihiro_debug_commands(running_machine &machine, int ref, int params, const char **param)
{
	if (params < 1)
		return;
	if (strcmp("jamdis", param[0]) == 0)
		jamtable_disasm_command(machine, ref, params - 1, param + 1);
	else
		help_command(machine, ref, params - 1, param + 1);
}

void chihiro_state::hack_eeprom()
{
	// 8003b744,3b744=0x90 0x90
	m_maincpu->space(0).write_byte(0x3b744, 0x90);
	m_maincpu->space(0).write_byte(0x3b745, 0x90);
	m_maincpu->space(0).write_byte(0x3b766, 0xc9);
	m_maincpu->space(0).write_byte(0x3b767, 0xc3);
}

static const struct {
	const char *game_name;
	struct {
		UINT32 address;
		UINT8 write_byte;
	} modify[16];
} hacks[2] = { { "chihiro", { { 0x6a79f, 0x01 }, { 0x6a7a0, 0x00 }, { 0x6b575, 0x00 }, { 0x6b576, 0x00 }, { 0x6b5af, 0x75 }, { 0x6b78a, 0x75 }, { 0x6b7ca, 0x00 }, { 0x6b7b8, 0x00 }, { 0x8f5b2, 0x75 }, { 0x79a9e, 0x74 }, { 0x79b80, 0x74 }, { 0x79b97, 0x74 }, { 0, 0 } } },
				{ "outr2", { { 0x12e4cf, 0x01 }, { 0x12e4d0, 0x00 }, { 0x4793e, 0x01 }, { 0x4793f, 0x00 }, { 0x47aa3, 0x01 }, { 0x47aa4, 0x00 }, { 0x14f2b6, 0x84 }, { 0x14f2d1, 0x75 }, { 0x8732f, 0x7d }, { 0x87384, 0x7d }, { 0x87388, 0xeb }, { 0, 0 } } } };

void chihiro_state::hack_usb()
{
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
}

// ======================> ide_baseboard_device

class ide_baseboard_device : public ata_mass_storage_device
{
public:
	// construction/destruction
	ide_baseboard_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual int  read_sector(UINT32 lba, void *buffer) override;
	virtual int  write_sector(UINT32 lba, const void *buffer) override;
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
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

ide_baseboard_device::ide_baseboard_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
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
	if (data != nullptr)
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
	xbox_base_devs.pic8259_2->ir2_w(1);
}

UINT8 *chihiro_state::baseboard_ide_dimmboard(UINT32 lba)
{
	// return pointer to memory containing decrypted gdrom data (contains an image of a fatx partition)
	if (chihiro_devs.dimmboard != nullptr)
		return dimm_board_memory + lba * 512;
	return nullptr;
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
		xbox_base_devs.pic8259_2->ir2_w(0);
}

static ADDRESS_MAP_START(chihiro_map, AS_PROGRAM, 32, chihiro_state)
	AM_IMPORT_FROM(xbox_base_map)
ADDRESS_MAP_END

static ADDRESS_MAP_START(chihiro_map_io, AS_IO, 32, chihiro_state)
	AM_IMPORT_FROM(xbox_base_map_io)
	AM_RANGE(0x4000, 0x40ff) AM_READWRITE(mediaboard_r, mediaboard_w)
ADDRESS_MAP_END

static INPUT_PORTS_START(chihiro)
INPUT_PORTS_END

void chihiro_state::machine_start()
{
	xbox_base_state::machine_start();
	chihiro_devs.ide = machine().device<bus_master_ide_controller_device>("ide");
	chihiro_devs.dimmboard = machine().device<naomi_gdrom_board>("rom_board");
	if (chihiro_devs.dimmboard != nullptr) {
		dimm_board_memory = chihiro_devs.dimmboard->memory(dimm_board_memory_size);
	}
	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
		debug_console_register_command(machine(), "chihiro", CMDFLAG_NONE, 0, 1, 4, chihiro_debug_commands);
	usbhack_index = -1;
	for (int a = 1; a < 2; a++)
		if (strcmp(machine().basename(), hacks[a].game_name) == 0) {
			usbhack_index = a;
			break;
		}
	usbhack_counter = 0;
	// savestates
	save_item(NAME(usbhack_counter));
}

static SLOT_INTERFACE_START(ide_baseboard)
	SLOT_INTERFACE("bb", IDE_BASEBOARD)
SLOT_INTERFACE_END

static MACHINE_CONFIG_DERIVED_CLASS(chihiro_base, xbox_base, chihiro_state)
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(chihiro_map)
	MCFG_CPU_IO_MAP(chihiro_map_io)

	//MCFG_BUS_MASTER_IDE_CONTROLLER_ADD("ide", ide_baseboard, NULL, "bb", true)
	MCFG_DEVICE_MODIFY("ide:0")
	MCFG_DEVICE_SLOT_INTERFACE(ide_baseboard, nullptr, true)
	MCFG_DEVICE_MODIFY("ide:1")
	MCFG_DEVICE_SLOT_INTERFACE(ide_baseboard, "bb", true)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED(chihirogd, chihiro_base)
	MCFG_NAOMI_GDROM_BOARD_ADD("rom_board", ":gdrom", ":pic", nullptr, NOOP)
MACHINE_CONFIG_END

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_BIOS(bios+1)) /* Note '+1' */

#define ROM_LOAD_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_BIOS(bios+1)) /* Note '+1' */

#define CHIHIRO_BIOS \
	ROM_REGION( 0x100000, "bios", 0) \
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
/*Chihiro*/ GAME( 2002, chihiro,  0,        chihiro_base, chihiro, driver_device, 0, ROT0, "Sega",                     "Chihiro Bios", MACHINE_NO_SOUND|MACHINE_NOT_WORKING|MACHINE_IS_BIOS_ROOT )

/* GDX-xxxx (Sega GD-ROM games) */
/* 0001  */ GAME( 2002, hotd3,    chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega / Wow Entertainment", "The House of the Dead III (GDX-0001)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0002     GAME( 2003, crtaxhro, crtaxihr, chihirogd,    chihiro, driver_device, 0, ROT0, "Sega / Hitmaker",          "Crazy Taxi High Roller (GDX-0002)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0002A    GAME( 2003, crtaxhra, crtaxihr, chihirogd,    chihiro, driver_device, 0, ROT0, "Sega / Hitmaker",          "Crazy Taxi High Roller (Rev A) (GDX-0002A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0002B */ GAME( 2003, crtaxihr, chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega / Hitmaker",          "Crazy Taxi High Roller (Rev B) (GDX-0002B)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0003     GAME( 2003, vcop3o,   vcop3,    chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Virtua Cop 3 (GDX-0003)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0003A */ GAME( 2003, vcop3,    chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Virtua Cop 3 (Rev A) (GDX-0003A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0004     GAME( 2003, outr2o,   outr2,    chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "OutRun 2 (GDX-0004)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING|MACHINE_SUPPORTS_SAVE )
/* 0004A */ GAME( 2003, outr2,    chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "OutRun 2 (Rev A) (GDX-0004A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING|MACHINE_SUPPORTS_SAVE )
// 0005     GAME( 2004, sgolcnpt, chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Golf Club Network Pro Tour (GDX-0005)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING|MACHINE_SUPPORTS_SAVE )
// 0006     GAME( 2004, mj2o,     mj2,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (GDX-0006)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0006A    GAME( 2004, mj2a,     mj2,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (Rev A) (GDX-0006A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0006B    GAME( 2004, mj2b,     mj2,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (Rev B) (GDX-0006B)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0006C */ GAME( 2004, mj2c,     mj2,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (Rev C) (GDX-0006C)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0006D    GAME( 2004, mj2d,     mj2,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (Rev D) (GDX-0006D)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0006E    GAME( 2004, mj2e,     mj2,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (Rev E) (GDX-0006E)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0006F */ GAME( 2004, mj2,      chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (Rev F) (GDX-0006F)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0006G */ GAME( 2004, mj2g,     mj2,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (Rev G) (GDX-0006G)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0007  */ GAME( 2004, ollie,    chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega / Amusement Vision",  "Ollie King (GDX-0007)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0008     GAME( 2004, wangmdjo, wangmidj, chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",                    "Wangan Midnight Maximum Tune (Japan) (GDX-0008)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0008A    GAME( 2004, wangmdja, wangmidj, chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",                    "Wangan Midnight Maximum Tune (Japan) (Rev A) (GDX-0008A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0008B    GAME( 2004, wangmidj, wangmidj, chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",                    "Wangan Midnight Maximum Tune (Japan) (Rev B) (GDX-0008B)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0009     GAME( 2004, wangmido, wangmid,  chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",                    "Wangan Midnight Maximum Tune (Export) (GDX-0009)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0009A    GAME( 2004, wangmida, wangmid,  chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",                    "Wangan Midnight Maximum Tune (Export) (Rev A) (GDX-0009A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0009B */ GAME( 2004, wangmid,  chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",                    "Wangan Midnight Maximum Tune (Export) (Rev B) (GDX-0009B)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0010
// 0011     GAME( 2004, outr2sp,  chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "OutRun 2 SP (Japan) (GDX-0011)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING|MACHINE_SUPPORTS_SAVE )
/* 0012  */ GAME( 2004, ghostsqo, ghostsqu, chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Ghost Squad (GDX-0012)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0012A */ GAME( 2004, ghostsqu, chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Ghost Squad (Rev A) (GDX-0012A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0013  */ GAME( 2005, gundamos, chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Banpresto",                "Gundam Battle Operating Simulator (GDX-0013)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0014     GAME( 2004, outr2sto, outr2st,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "OutRun 2 Special Tours (GDX-0014)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0014A */ GAME( 2004, outr2st,  chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "OutRun 2 Special Tours (Rev A) (GDX-0014A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0015  */ GAME( 2005, wangmid2, chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",                    "Wangan Midnight Maximum Tune 2 (Japan) (GDX-0015)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0016     GAME( 2005, wanmd2bo, wangmd2b, chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",                    "Wangan Midnight Maximum Tune 2 (Export) (GDX-0016)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0016A */ GAME( 2005, wangmd2b, chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",                    "Wangan Midnight Maximum Tune 2 (Export) (Rev A) (GDX-0016A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0017     GAME( 2005, mj3o,     mj3,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 (GDX-0017)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0017A    GAME( 2005, mj3a,     mj3,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 (Rev A) (GDX-0017A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0017B    GAME( 2005, mj3b,     mj3,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 (Rev B) (GDX-0017B)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0017C    GAME( 2005, mj3c,     mj3,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 (Rev C) (GDX-0017C)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0017D */ GAME( 2005, mj3d,     mj3,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 (Rev D) (GDX-0017D)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0017E    GAME( 2005, mj3e,     mj3,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 (Rev E) (GDX-0017E)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0017F */ GAME( 2005, mj3,      chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 (Rev F) (GDX-0017F)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0018     GAME( 2005, scg06nto, scg06nt,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Club Golf 2006 Next Tours (GDX-0018)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0018A */ GAME( 2005, scg06nt,  chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Club Golf 2006 Next Tours (Rev A) (GDX-0018A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0019
// 0020
// 0021     GAME( 2006, mj3evoo,  mj3evo,    chihirogd,   chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 Evolution (GDX-0021)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0021A    GAME( 2006, mj3evoa,  mj3evo,    chihirogd,   chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 Evolution (Rev A) (GDX-0021A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0021B */ GAME( 2007, mj3evo,   chihiro,   chihirogd,   chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 Evolution (Rev B) (GDX-0021B)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0022
// 0023
// 0024     GAME( 2009, ccfboxo,  ccfboxa,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Chihiro Firmware Update For Compact Flash Box (GDX-0024)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0024A */ GAME( 2009, ccfboxa,  chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Chihiro Firmware Update For Compact Flash Box (Rev A) (GDX-0024A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
