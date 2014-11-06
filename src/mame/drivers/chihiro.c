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
| | 2003 | Out Run 2                                          | Sega                     | GDROM  | GDX-0004   | 317-0372-COM |
|*| 2003 | Out Run 2 (Rev A)                                  | Sega                     | GDROM  | GDX-0004A  | 317-0372-COM |
| | 2003 | Out Run 2 prototype (Rev P)                        | Sega                     | GDROM  | GDX-0004P  |              |
| | 2004 | Sega Golf Club Network Pro Tour                    | Sega                     | GDROM  | GDX-0005   |              |
| | 2004 | Sega Network Taisen Mahjong MJ 2                   | Sega                     | GDROM  | GDX-0006   |              |
| | 2004 | Sega Network Taisen Mahjong MJ 2 (Rev A)           | Sega                     | GDROM  | GDX-0006A  |              |
| | 2004 | Sega Network Taisen Mahjong MJ 2 (Rev B)           | Sega                     | GDROM  | GDX-0006B  |              |
|*| 2004 | Sega Network Taisen Mahjong MJ 2 (Rev C)           | Sega                     | GDROM  | GDX-0006C  |              |
| | 2004 | Sega Network Taisen Mahjong MJ 2 (Rev D)           | Sega                     | GDROM  | GDX-0006D  |              |
| | 2005 | Sega Network Taisen Mahjong MJ 2 (Rev E)           | Sega                     | GDROM  | GDX-0006E  |              |
| | 2005 | Sega Network Taisen Mahjong MJ 2 (Rev F)           | Sega                     | GDROM  | GDX-0006F  |              |
|*| 2005 | Sega Network Taisen Mahjong MJ 2 (Rev G)           | Sega                     | GDROM  | GDX-0006G  | 317-0374-JPN |
|*| 2004 | Ollie King                                         | Sega / Amusement Vision  | GDROM  | GDX-0007   | 317-0377-COM |
| | 2004 | Wangan Midnight Maximum Tune (Japan)               | Namco                    | GDROM  | GDX-0008   | 317-5101-JPN |
| | 2004 | Wangan Midnight Maximum Tune (Japan) (Rev A)       | Namco                    | GDROM  | GDX-0008A  | 317-5101-JPN |
|*| 2004 | Wangan Midnight Maximum Tune (Japan) (Rev B)       | Namco                    | GDROM  | GDX-0008B  | 317-5101-JPN |
| | 2004 | Wangan Midnight Maximum Tune (Export)              | Namco                    | GDROM  | GDX-0009   | 317-5101-COM |
| | 2004 | Wangan Midnight Maximum Tune (Export) (Rev A)      | Namco                    | GDROM  | GDX-0009A  | 317-5101-COM |
|*| 2004 | Wangan Midnight Maximum Tune (Export) (Rev B)      | Namco                    | GDROM  | GDX-0009B  | 317-5101-COM |
| | 2004 | Outrun 2 SP (Japan)                                | Sega                     | GDROM  | GDX-0011   |              |
| | 2004 | Ghost Squad                                        | Sega                     | GDROM  | GDX-0012   | 317-0398-COM |
|*| 2004 | Ghost Squad (Rev A)                                | Sega                     | GDROM  | GDX-0012A  | 317-0398-COM |
|*| 2005 | Gundam Battle Operating Simulator                  | Banpresto                | GDROM  | GDX-0013   | 317-0400-JPN |
| | 2004 | Outrun 2 Special Tours                             | Sega                     | GDROM  | GDX-0014   | 317-0xxx-COM |
|*| 2004 | Outrun 2 Special Tours (Rev A)                     | Sega                     | GDROM  | GDX-0014A  | 317-0xxx-COM |
|*| 2005 | Wangan Midnight Maximum Tune 2 (Japan)             | Namco                    | GDROM  | GDX-0015   | 317-5106-JPN |
|*| 2005 | Wangan Midnight Maximum Tune 2 (Japan) (Rev A)     | Namco                    | GDROM  | GDX-0015A  | 317-5106-JPN |
|*| 2005 | Wangan Midnight Maximum Tune 2 (Export)            | Namco                    | GDROM  | GDX-0016   | 317-5106-COM |
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
#include "osdcore.h"

#define LOG_PCI
//#define LOG_OHCI
//#define LOG_NV2A
//#define LOG_BASEBOARD

class nv2a_renderer; // forw. dec.
struct nvidia_object_data
{
	nv2a_renderer *data;
};

class chihiro_state : public driver_device
{
public:
	chihiro_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		nvidia_nv2a(NULL),
		debug_irq_active(false),
		m_maincpu(*this, "maincpu") { }

	DECLARE_READ32_MEMBER( geforce_r );
	DECLARE_WRITE32_MEMBER( geforce_w );
	DECLARE_READ32_MEMBER( usbctrl_r );
	DECLARE_WRITE32_MEMBER( usbctrl_w );
	DECLARE_READ32_MEMBER( smbus_r );
	DECLARE_WRITE32_MEMBER( smbus_w );
	DECLARE_READ32_MEMBER( mediaboard_r );
	DECLARE_WRITE32_MEMBER( mediaboard_w );
	DECLARE_READ32_MEMBER( audio_apu_r );
	DECLARE_WRITE32_MEMBER( audio_apu_w );
	DECLARE_READ32_MEMBER( audio_ac93_r );
	DECLARE_WRITE32_MEMBER( audio_ac93_w );
	DECLARE_READ32_MEMBER( dummy_r );
	DECLARE_WRITE32_MEMBER( dummy_w );

	void smbus_register_device(int address,int (*handler)(chihiro_state &chs,int command,int rw,int data));
	int smbus_pic16lc(int command,int rw,int data);
	int smbus_cx25871(int command,int rw,int data);
	int smbus_eeprom(int command,int rw,int data);
	void baseboard_ide_event(int type,UINT8 *read,UINT8 *write);
	UINT8 *baseboard_ide_dimmboard(UINT32 lba);
	void dword_write_le(UINT8 *addr,UINT32 d);
	void word_write_le(UINT8 *addr,UINT16 d);
	void debug_generate_irq(int irq,bool active);

	void vblank_callback(screen_device &screen, bool state);
	UINT32 screen_update_callback(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	virtual void machine_start();
	DECLARE_WRITE_LINE_MEMBER(chihiro_pic8259_1_set_int_line);
	DECLARE_READ8_MEMBER(get_slave_ack);
	DECLARE_WRITE_LINE_MEMBER(chihiro_pit8254_out0_changed);
	DECLARE_WRITE_LINE_MEMBER(chihiro_pit8254_out2_changed);
	IRQ_CALLBACK_MEMBER(irq_callback);
	TIMER_CALLBACK_MEMBER(audio_apu_timer);

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
		int (*devices[128])(chihiro_state &chs,int command,int rw,int data);
		UINT32 words[256/4];
	} smbusst;
	struct apu_state {
		UINT32 memory[0x60000/4];
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
		UINT32 mixer_regs[0x80/4];
		UINT32 controller_regs[0x38/4];
	} ac97st;
	UINT8 pic16lc_buffer[0xff];
	nv2a_renderer *nvidia_nv2a;
	bool debug_irq_active;
	int debug_irq_number;
	UINT8 *dimm_board_memory;
	UINT32 dimm_board_memory_size;
	int usbhack_counter;
	required_device<cpu_device> m_maincpu;
};

/*
 * geforce 3d (NV2A) vertex program disassembler
 */
class vertex_program_disassembler {
	static const char *srctypes[];
	static const char *scaops[];
	static const int scapar2[];
	static const char *vecops[];
	static const int vecpar2[];
	static const char *vecouts[];
	static const char compchar[];
	int o[6];
	int state;

	struct sourcefields
	{
		int Sign;
		int SwizzleX;
		int SwizzleY;
		int SwizzleZ;
		int SwizzleW;
		int TempIndex;
		int ParameterType;
	};

	struct fields
	{
		int ScaOperation;
		int VecOperation;
		int SourceConstantIndex;
		int InputIndex;
		sourcefields src[3];
		int VecTempWriteMask;
		int VecTempIndex;
		int ScaTempWriteMask;
		int OutputWriteMask;
		int OutputSelect;
		int OutputIndex;
		int MultiplexerControl;
		int Usea0x;
		int EndOfProgram;
	};
	fields f;

	void decodefields(unsigned int *dwords, int offset, fields &decoded);
	int disassemble_mask(int mask, char *s);
	int disassemble_swizzle(sourcefields f, char *s);
	int disassemble_source(sourcefields f, fields fi, char *s);
	int disassemble_output(fields f, char *s);
	int output_types(fields f, int *o);
public:
	vertex_program_disassembler() { state = 0; }
	int disassemble(unsigned int *instruction, char *line);
};

const char *vertex_program_disassembler::srctypes[] = { "??", "Rn", "Vn", "Cn" };
const char *vertex_program_disassembler::scaops[] = { "NOP", "IMV", "RCP", "RCC", "RSQ", "EXP", "LOG", "LIT", "???", "???", "???", "???", "???", "???", "???", "???", "???" };
const int vertex_program_disassembler::scapar2[] = { 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
const char *vertex_program_disassembler::vecops[] = { "NOP", "MOV", "MUL", "ADD", "MAD", "DP3", "DPH", "DP4", "DST", "MIN", "MAX", "SLT", "SGE", "ARL", "???", "???", "???" };
const int vertex_program_disassembler::vecpar2[] = { 0, 4, 6, 5, 7, 6, 6, 6, 6, 6, 6, 6, 6, 4, 0, 0, 0 };
const char *vertex_program_disassembler::vecouts[] = { "oPos", "???", "???", "oD0", "oD1", "oFog", "oPts", "oB0", "oB1", "oT0", "oT1", "oT2", "oT3" };
const char vertex_program_disassembler::compchar[] = { 'x', 'y', 'z', 'w' };

/*
Each vertex program instruction is a 128 bit word made of the fields:
d         f
w   b     i
o   i     e
r   t     l
d   s     d
+-+-----+-------
|0|31-0 |not used
+-+-----+-------
| |31-29|not used
| +-----+-------
| |28-25|scalar operation
| +-----+-------
| |24-21|vectorial operation
| +-----+-------
| |20-13|index for source constant C[]
| +-----+-------
| |12-9 |input vector index
| +-----+-------
|1|  8  |parameter A:sign
| +-----+-------
| | 7-6 |parameter A:swizzle x
| +-----+-------
| | 5-4 |parameter A:swizzle y
| +-----+-------
| | 3-2 |parameter A:swizzle z
| +-----+-------
| | 1-0 |parameter A:swizzle w
|-+-----+-------
| |31-28|parameter A:parameter Rn index
| +-----+-------
| |27-26|parameter A:input type 1:Rn 2:Vn 3:C[n]
| +-----+-------
| | 25  |parameter B:sign
| +-----+-------
| |24-23|parameter B:swizzle x
| +-----+-------
| |22-21|parameter B:swizzle y
| +-----+-------
| |20-19|parameter B:swizzle z
| +-----+-------
|2|18-17|parameter B:swizzle w
| +-----+-------
| |16-13|parameter B:parameter Rn index
| +-----+-------
| |12-11|parameter B:input type 1:Rn 2:Vn 3:C[n]
| +-----+-------
| | 10  |parameter C:sign
| +-----+-------
| | 9-8 |parameter C:swizzle x
| +-----+-------
| | 7-6 |parameter C:swizzle y
| +-----+-------
| | 5-4 |parameter C:swizzle z
| +-----+-------
| | 3-2 |parameter C:swizzle w
| +-----+-------
| | 1-0 |
|-+     |parameter C:parameter Rn index
| |31-30|
| +-----+-------
| |29-28|parameter C:input type 1:Rn 2:Vn 3:C[n]
| +-----+-------
| |27-24|output Rn mask from vectorial operation
| +-----+-------
| |23-20|output Rn index from vectorial operation
| +-----+-------
| |19-16|output Rn mask from scalar operation
| +-----+-------
|3|15-12|output vector write mask
| +-----+-------
| | 11  |1:output is output vector 0:output is constant C[]
| +-----+-------
| |10-3 |output vector/constant index
| +-----+-------
| |  2  |0:output Rn from vectorial operation 1:output Rn from scalar operation
| +-----+-------
| |  1  |1:add a0x to index for source constant C[]
| +-----+-------
| |  0  |1:end of program
+-+-----+-------
Each vertex program instruction can generate up to three destination values using up to three source values.
The first possible destination is to Rn from a vectorial operation.
The second possible destination is to a vertex shader output or C[n] from a vectorial or scalar operation.
The third possible destination is to Rn from a scalar operation.
*/
void vertex_program_disassembler::decodefields(unsigned int *dwords, int offset, fields &decoded)
{
	unsigned int srcbits[3];
	int a;

	srcbits[0] = ((dwords[1 + offset] & 0x1ff) << 6) | (dwords[2 + offset] >> 26);
	srcbits[1] = (dwords[2 + offset] >> 11) & 0x7fff;
	srcbits[2] = ((dwords[2 + offset] & 0x7ff) << 4) | (dwords[3 + offset] >> 28);
	decoded.ScaOperation = (int)(dwords[1 + offset] >> 25) & 0xf;
	decoded.VecOperation = (int)(dwords[1 + offset] >> 21) & 0xf;
	decoded.SourceConstantIndex = (int)(dwords[1 + offset] >> 13) & 0xff;
	decoded.InputIndex = (int)(dwords[1 + offset] >> 9) & 0xf;
	for (a = 0; a < 3; a++)
	{
		decoded.src[a].Sign = (int)(srcbits[a] >> 14) & 1;
		decoded.src[a].SwizzleX = (int)(srcbits[a] >> 12) & 3;
		decoded.src[a].SwizzleY = (int)(srcbits[a] >> 10) & 3;
		decoded.src[a].SwizzleZ = (int)(srcbits[a] >> 8) & 3;
		decoded.src[a].SwizzleW = (int)(srcbits[a] >> 6) & 3;
		decoded.src[a].TempIndex = (int)(srcbits[a] >> 2) & 0xf;
		decoded.src[a].ParameterType = (int)(srcbits[a] >> 0) & 3;
	}

	decoded.VecTempWriteMask = (int)(dwords[3 + offset] >> 24) & 0xf;
	decoded.VecTempIndex = (int)(dwords[3 + offset] >> 20) & 0xf;
	decoded.ScaTempWriteMask = (int)(dwords[3 + offset] >> 16) & 0xf;
	decoded.OutputWriteMask = (int)(dwords[3 + offset] >> 12) & 0xf;
	decoded.OutputSelect = (int)(dwords[3 + offset] >> 11) & 0x1;
	decoded.OutputIndex = (int)(dwords[3 + offset] >> 3) & 0xff;
	decoded.MultiplexerControl = (int)(dwords[3 + offset] >> 2) & 0x1;
	decoded.Usea0x = (int)(dwords[3 + offset] >> 1) & 0x1;
	decoded.EndOfProgram = (int)(dwords[3 + offset] >> 0) & 0x1;
}

int vertex_program_disassembler::disassemble_mask(int mask, char *s)
{
	int l;

	*s = 0;
	if (mask == 15)
		return 0;
	s[0] = '.';
	l = 1;
	if ((mask & 8) != 0) {
		s[l] = 'x';
		l++;
	}
	if ((mask & 4) != 0){
		s[l] = 'y';
		l++;
	}
	if ((mask & 2) != 0){
		s[l] = 'z';
		l++;
	}
	if ((mask & 1) != 0){
		s[l] = 'w';
		l++;
	}
	s[l] = 0;
	return l;
}

int vertex_program_disassembler::disassemble_swizzle(sourcefields f, char *s)
{
	int t, l;

	t = 4;
	if (f.SwizzleW == 3)
	{
		t = t - 1;
		if (f.SwizzleZ == 2)
		{
			t = t - 1;
			if (f.SwizzleY == 1)
			{
				t = t - 1;
				if (f.SwizzleX == 0)
				{
					t = t - 1;
				}
			}
		}
	}
	*s = 0;
	if (t == 0)
		return 0;
	s[0] = '.';
	l = 1;
	if (t > 0)
	{
		s[l] = compchar[f.SwizzleX];
		l++;
	}
	if (t > 1)
	{
		s[l] = compchar[f.SwizzleY];
		l++;
	}
	if (t > 2)
	{
		s[l] = compchar[f.SwizzleZ];
		l++;
	}
	if (t > 3)
	{
		s[l] = compchar[f.SwizzleW];
		l++;
	}
	s[l] = 0;
	return l;
}

int vertex_program_disassembler::disassemble_source(sourcefields f, fields fi, char *s)
{
	int l;

	if (f.ParameterType == 0) {
		strcpy(s, ",???");
		return 4;
	}
	l = 0;
	if (f.Sign != 0) {
		s[l] = '-';
		l++;
	}
	if (f.ParameterType == 1) {
		s[l] = 'r';
		l = l + 1 + sprintf(s + l + 1, "%d", f.TempIndex);
	}
	else if (f.ParameterType == 2){
		s[l] = 'v';
		l = l + 1 + sprintf(s + l + 1, "%d", fi.InputIndex);
	}
	else
	{
		if (fi.Usea0x != 0)
		{
			if (fi.SourceConstantIndex >= 96) {
				strcpy(s + l, "c[");
				l = l + 2;
				l = l + sprintf(s + l, "%d", fi.SourceConstantIndex - 96);
				strcpy(s + l, "+a0.x]");
				l = l + 6;
			}
			else {
				strcpy(s + l, "c[a0.x");
				l = l + 6;
				l = l + sprintf(s + l, "%d", fi.SourceConstantIndex - 96);
				s[l] = ']';
				l++;
			}
		}
		else {
			strcpy(s + l, "c[");
			l = l + 2;
			l = l + sprintf(s + l, "%d", fi.SourceConstantIndex - 96);
			s[l] = ']';
			l++;
		}
	}
	l = l + disassemble_swizzle(f, s + l);
	s[l] = 0;
	return l;
}

int vertex_program_disassembler::disassemble_output(fields f, char *s)
{
	int l;

	if (f.OutputSelect == 1) {
		strcpy(s, vecouts[f.OutputIndex]);
		return strlen(s);
	}
	else {
		strcpy(s, "c[");
		l = 2;
		l = l + sprintf(s + l, "%d", f.OutputIndex - 96);
		s[l] = ']';
		l++;
	}
	s[l] = 0;
	return l;
}

int vertex_program_disassembler::output_types(fields f, int *o)
{
	o[0] = o[1] = o[2] = o[3] = o[4] = o[5] = 0;
	if ((f.VecOperation > 0) && (f.VecTempWriteMask != 0))
		o[0] = 1;
	if ((f.VecOperation > 0) && (f.OutputWriteMask != 0) && (f.MultiplexerControl == 0))
		o[1] = 1;
	if ((f.ScaOperation > 0) && (f.OutputWriteMask != 0) && (f.MultiplexerControl == 1))
		o[2] = 1;
	if ((f.ScaOperation > 0) && (f.ScaTempWriteMask != 0))
		o[3] = 1;
	if (f.VecOperation == 13)
		o[4] = 1;
	if (f.EndOfProgram == 1)
		o[5] = 1;
	return o[0] + o[1] + o[2] + o[3] + o[4] + o[5];
}

int vertex_program_disassembler::disassemble(unsigned int *instruction, char *line)
{
	int b, p;
	char *c;

	if (state == 0) {
		decodefields(instruction, 0, f);
		output_types(f, o);
		state = 1;
	}
	if (o[0] != 0)
	{
		o[0] = 0;
		c = line;
		strcpy(c, vecops[f.VecOperation]);
		c = c + strlen(c);
		strcpy(c, " r");
		c = c + 2;
		c = c + sprintf(c, "%d", f.VecTempIndex);
		c = c + disassemble_mask(f.VecTempWriteMask, c);
		b = 0;
		for (p = 4; p != 0; p = p >> 1)
		{
			if ((vecpar2[f.VecOperation] & p) != 0) {
				c[0] = ',';
				c++;
				c = c + disassemble_source(f.src[b], f, c);
			}
			b++;
		}
		*c = 0;
		return 1;
	}
	if (o[1] != 0)
	{
		o[1] = 0;
		c = line;
		strcpy(c, vecops[f.VecOperation]);
		c = c + strlen(c);
		*c = ' ';
		c++;
		c = c + disassemble_output(f, c);
		c = c + disassemble_mask(f.OutputWriteMask, c);
		b = 0;
		for (p = 4; p != 0; p = p >> 1)
		{
			if ((vecpar2[f.VecOperation] & p) != 0) {
				*c = ',';
				c++;
				c = c + disassemble_source(f.src[b], f, c);
			}
			b++;
		}
		*c = 0;
		return 1;
	}
	if (o[2] != 0)
	{
		o[2] = 0;
		c = line;
		strcpy(c, scaops[f.ScaOperation]);
		c = c + strlen(c);
		*c = ' ';
		c++;
		c = c + disassemble_output(f, c);
		c = c + disassemble_mask(f.OutputWriteMask, c);
		b = 0;
		for (p = 4; p != 0; p = p >> 1)
		{
			if ((scapar2[f.ScaOperation] & p) != 0) {
				*c = ',';
				c++;
				c = c + disassemble_source(f.src[b], f, c);
			}
			b++;
		}
		*c = 0;
		return 1;
	}
	if (o[3] != 0)
	{
		if (f.VecOperation > 0)
			b = 1;
		else
			b = f.VecTempIndex;
		o[3] = 0;
		c = line;
		strcpy(c, scaops[f.ScaOperation]);
		c = c + strlen(c);
		strcpy(c, " r");
		c = c + 2;
		c = c + sprintf(c, "%d", b);
		c = c + disassemble_mask(f.ScaTempWriteMask, c);
		b = 0;
		for (p = 4; p != 0; p = p >> 1)
		{
			if ((scapar2[f.ScaOperation] & p) != 0) {
				*c = ',';
				c++;
				c = c + disassemble_source(f.src[b], f, c);
			}
			b++;
		}
		*c = 0;
		return 1;
	}
	if (o[4] != 0)
	{
		o[4] = 0;
		c = line;
		c = c + sprintf(c, "MOV a0.x,");
		c = c + disassemble_source(f.src[0], f, c);
		*c = 0;
		return 1;
	}
	if (o[5] != 0)
	{
		o[5] = 0;
		strcpy(line, "END");
		return 1;
	}
	state = 0;
	return 0;
}

/*
 * geforce 3d (NV2A) accellerator
 */
/* very simplified view
there is a set of context objects

context objects are stored in RAMIN
each context object is identified by an handle stored in RAMHT

each context object can be assigned to a channel
to assign you give to the channel an handle for the object

offset in ramht=(((((handle >> 11) xor handle) >> 11) xor handle) & 0x7ff)*8
offset in ramht contains the handle itself
offset in ramht+4 contains in the lower 16 bits the offset in RAMIN divided by 16

objects have methods used to do drawing
most methods set parameters, others actually draw
*/
class nv2a_renderer : public poly_manager<float, nvidia_object_data, 12, 8192>
{
public:
	nv2a_renderer(running_machine &machine) : poly_manager<float, nvidia_object_data, 12, 8192>(machine)
	{
		memset(channel,0,sizeof(channel));
		memset(pfifo,0,sizeof(pfifo));
		memset(pcrtc,0,sizeof(pcrtc));
		memset(pmc,0,sizeof(pmc));
		memset(ramin,0,sizeof(ramin));
		computedilated();
		fb.allocate(640,480);
		objectdata=&(object_data_alloc());
		objectdata->data=this;
		combiner.used=0;
		combiner.lock=osd_lock_alloc();
		enabled_vertex_attributes=0;
		indexesleft_count = 0;
		vertex_pipeline = 4;
		alpha_test_enabled = false;
		alpha_reference = 0;
		alpha_func = nv2a_renderer::ALWAYS;
		blending_enabled = false;
		blend_equation = nv2a_renderer::FUNC_ADD;
		blend_color = 0;
		blend_function_destination = nv2a_renderer::ZERO;
		blend_function_source = nv2a_renderer::ONE;
		logical_operation_enabled = false;
		logical_operation = nv2a_renderer::COPY;
		debug_grab_texttype = -1;
		debug_grab_textfile = NULL;
		memset(vertex_attribute_words, 0, sizeof(vertex_attribute_words));
		memset(vertex_attribute_offset, 0, sizeof(vertex_attribute_offset));
	}
	DECLARE_READ32_MEMBER( geforce_r );
	DECLARE_WRITE32_MEMBER( geforce_w );
	void vblank_callback(screen_device &screen, bool state);
	UINT32 screen_update_callback(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void render_texture_simple(INT32 scanline, const extent_t &extent, const nvidia_object_data &extradata, int threadid);
	void render_color(INT32 scanline, const extent_t &extent, const nvidia_object_data &extradata, int threadid);
	void render_register_combiners(INT32 scanline, const extent_t &extent, const nvidia_object_data &objectdata, int threadid);

	int geforce_commandkind(UINT32 word);
	UINT32 geforce_object_offset(UINT32 handle);
	void geforce_read_dma_object(UINT32 handle,UINT32 &offset,UINT32 &size);
	void geforce_exec_method(address_space &space,UINT32 channel,UINT32 subchannel,UINT32 method,UINT32 address,int &countlen);
	UINT32 texture_get_texel(int number,int x,int y);
	void write_pixel(int x, int y, UINT32 color);
	void combiner_initialize_registers(UINT32 argb8[6]);
	void combiner_initialize_stage(int stage_number);
	void combiner_initialize_final();
	void combiner_map_input(int stage_number); // map combiner registers to variables A..D
	void combiner_map_output(int stage_number); // map combiner calculation results to combiner registers
	void combiner_map_final_input(); // map final combiner registers to variables A..F
	void combiner_final_output(); // generate final combiner output
	float combiner_map_input_select(int code,int index); // get component index in register code
	float *combiner_map_input_select3(int code); // get pointer to register code
	float *combiner_map_output_select3(int code); // get pointer to register code for output
	float combiner_map_input_function(int code,float value); // apply input mapping function code to value
	void combiner_map_input_function3(int code,float *data); // apply input mapping function code to data
	void combiner_function_AB(float result[4]);
	void combiner_function_AdotB(float result[4]);
	void combiner_function_CD(float result[4]);
	void combiner_function_CdotD(float result[4]);
	void combiner_function_ABmuxCD(float result[4]);
	void combiner_function_ABsumCD(float result[4]);
	void combiner_compute_rgb_outputs(int index);
	void combiner_compute_a_outputs(int index);
	void combiner_argb8_float(UINT32 color,float reg[4]);
	UINT32 combiner_float_argb8(float reg[4]);
	UINT32 dilate0(UINT32 value,int bits);
	UINT32 dilate1(UINT32 value,int bits);
	void computedilated(void);
	void putpixtex(int xp,int yp,int up,int vp);
	int toggle_register_combiners_usage();
	void debug_grab_texture(int type, const char *filename);
	void debug_grab_vertex_program_slot(int slot, UINT32 *instruction);
	void savestate_items();

	struct vertex {
		union {
			float fv[4];
			UINT32 iv[4];
		} attribute[16];
	};
	int read_vertices_0x1810(address_space & space, vertex *destination, int offset, int limit);
	int read_vertices_0x1800(address_space & space, vertex *destination, UINT32 address, int limit);
	int read_vertices_0x1818(address_space & space, vertex *destination, UINT32 address, int limit);
	void convert_vertices_poly(vertex *source, vertex_t *destination, int count);

	struct {
		UINT32 regs[0x80/4];
		struct {
			UINT32 objhandle;
			UINT32 objclass;
			UINT32 method[0x2000/4];
		} object;
	} channel[32][8];
	UINT32 pfifo[0x2000/4];
	UINT32 pcrtc[0x1000/4];
	UINT32 pmc[0x1000/4];
	UINT32 ramin[0x100000/4];
	UINT32 dma_offset[2];
	UINT32 dma_size[2];
	UINT32 vertexbuffer_address[16];
	int vertexbuffer_stride[16];
	struct {
		int enabled;
		int sizeu;
		int sizev;
		int sizew;
		int dilate;
		int format;
		int rectangle_pitch;
		void *buffer;
	} texture[4];
	int primitives_count;
	int indexesleft_count;
	int indexesleft_first;
	UINT32 indexesleft[8];
	struct {
		float variable_A[4]; // 0=R 1=G 2=B 3=A
		float variable_B[4];
		float variable_C[4];
		float variable_D[4];
		float variable_E[4];
		float variable_F[4];
		float variable_G;
		float variable_EF[4];
		float variable_sumclamp[4];
		float function_RGBop1[4]; // 0=R 1=G 2=B
		float function_RGBop2[4];
		float function_RGBop3[4];
		float function_Aop1;
		float function_Aop2;
		float function_Aop3;
		float register_primarycolor[4]; // rw
		float register_secondarycolor[4];
		float register_texture0color[4];
		float register_texture1color[4];
		float register_texture2color[4];
		float register_texture3color[4];
		float register_color0[4];
		float register_color1[4];
		float register_spare0[4];
		float register_spare1[4];
		float register_fogcolor[4]; // ro
		float register_zero[4];
		float output[4];
		struct {
			float register_constantcolor0[4];
			float register_constantcolor1[4];
			int mapin_aA_input;
			int mapin_aA_component;
			int mapin_aA_mapping;
			int mapin_aB_input;
			int mapin_aB_component;
			int mapin_aB_mapping;
			int mapin_aC_input;
			int mapin_aC_component;
			int mapin_aC_mapping;
			int mapin_aD_input;
			int mapin_aD_component;
			int mapin_aD_mapping;
			int mapin_rgbA_input;
			int mapin_rgbA_component;
			int mapin_rgbA_mapping;
			int mapin_rgbB_input;
			int mapin_rgbB_component;
			int mapin_rgbB_mapping;
			int mapin_rgbC_input;
			int mapin_rgbC_component;
			int mapin_rgbC_mapping;
			int mapin_rgbD_input;
			int mapin_rgbD_component;
			int mapin_rgbD_mapping;
			int mapout_aCD_output;
			int mapout_aAB_output;
			int mapout_aSUM_output;
			int mapout_aCD_dotproduct;
			int mapout_aAB_dotproduct;
			int mapout_a_muxsum;
			int mapout_a_bias;
			int mapout_a_scale;
			int mapout_rgbCD_output;
			int mapout_rgbAB_output;
			int mapout_rgbSUM_output;
			int mapout_rgbCD_dotproduct;
			int mapout_rgbAB_dotproduct;
			int mapout_rgb_muxsum;
			int mapout_rgb_bias;
			int mapout_rgb_scale;
		} stage[8];
		struct {
			float register_constantcolor0[4];
			float register_constantcolor1[4];
			int color_sum_clamp;
			int mapin_rgbA_input;
			int mapin_rgbA_component;
			int mapin_rgbA_mapping;
			int mapin_rgbB_input;
			int mapin_rgbB_component;
			int mapin_rgbB_mapping;
			int mapin_rgbC_input;
			int mapin_rgbC_component;
			int mapin_rgbC_mapping;
			int mapin_rgbD_input;
			int mapin_rgbD_component;
			int mapin_rgbD_mapping;
			int mapin_rgbE_input;
			int mapin_rgbE_component;
			int mapin_rgbE_mapping;
			int mapin_rgbF_input;
			int mapin_rgbF_component;
			int mapin_rgbF_mapping;
			int mapin_aG_input;
			int mapin_aG_component;
			int mapin_aG_mapping;
		} final;
		int stages;
		int used;
		osd_lock *lock;
	} combiner;
	bool alpha_test_enabled;
	int alpha_func;
	int alpha_reference;
	bool blending_enabled;
	int blend_equation;
	int blend_function_source;
	int blend_function_destination;
	UINT32 blend_color;
	bool logical_operation_enabled;
	int logical_operation;
	struct {
		float modelview[16];
		float modelview_inverse[16];
		float projection[16];
		float translate[4];
		float scale[4];
	} matrix;
	struct {
		UINT32 instruction[1024];
		int instructions;
		int upload_instruction;
		int start_instruction;
		float parameter[1024];
		int upload_parameter;
	} vertexprogram;
	int vertex_pipeline;
	int enabled_vertex_attributes;
	int vertex_attribute_words[16];
	int vertex_attribute_offset[16];
	bitmap_rgb32 fb;
	UINT32 dilated0[16][2048];
	UINT32 dilated1[16][2048];
	int dilatechose[256];
	nvidia_object_data *objectdata;
	int debug_grab_texttype;
	char *debug_grab_textfile;

	enum NV2A_BEGIN_END {
		STOP=0,
		POINTS=1,
		LINES=2,
		LINE_LOOP=3,
		LINE_STRIP=4,
		TRIANGLES=5,
		TRIANGLE_STRIP=6,
		TRIANGLE_FAN=7,
		QUADS=8,
		QUAD_STRIP=9,
		POLYGON=10
	};
	enum NV2A_VERTEX_ATTR {
		POS=0,
		WEIGHT=1,
		NORMAL=2,
		COLOR0=3,
		COLOR1=4,
		FOG=5,
		TEX0=9,
		TEX1=10,
		TEX2=11,
		TEX3=12
	};
	enum NV2A_VTXBUF_TYPE {
		FLOAT=2,
		UBYTE=4,
		USHORT=5
	};
	enum NV2A_TEX_FORMAT {
		L8=0x0,
		I8=0x1,
		A1R5G5B5=0x2,
		A4R4G4B4=0x4,
		R5G6B5=0x5,
		A8R8G8B8=0x6,
		X8R8G8B8=0x7,
		INDEX8=0xb,
		DXT1=0xc,
		DXT3=0xe,
		DXT5=0xf,
		A1R5G5B5_RECT=0x10,
		R5G6B5_RECT=0x11,
		A8R8G8B8_RECT=0x12,
		L8_RECT=0x13,
		DSDT8_RECT=0x17,
		A8L8=0x1a,
		I8_RECT=0x1b,
		A4R4G4B4_RECT=0x1d,
		R8G8B8_RECT=0x1e,
		A8L8_RECT=0x20,
		Z24=0x2a,
		Z24_RECT=0x2b,
		Z16=0x2c,
		Z16_RECT=0x2d,
		DSDT8=0x28,
		HILO16=0x33,
		HILO16_RECT=0x36,
		HILO8=0x44,
		SIGNED_HILO8=0x45,
		HILO8_RECT=0x46,
		SIGNED_HILO8_RECT=0x47
	};
	enum NV2A_LOGIC_OP {
		CLEAR=0x1500,
		AND=0x1501,
		AND_REVERSE=0x1502,
		COPY=0x1503,
		AND_INVERTED=0x1504,
		NOOP=0x1505,
		XOR=0x1506,
		OR=0x1507,
		NOR=0x1508,
		EQUIV=0x1509,
		INVERT=0x150a,
		OR_REVERSE=0x150b,
		COPY_INVERTED=0x150c,
		OR_INVERTED=0x150d,
		NAND=0x150e,
		SET=0x150f
	};
	enum NV2A_BLEND_EQUATION {
		FUNC_ADD=0x8006,
		MIN=0x8007,
		MAX=0x8008,
		FUNC_SUBTRACT=0x800a,
		FUNC_REVERSE_SUBTRACT=0x80b
	};
	enum NV2A_BLEND_FACTOR {
		ZERO=0x0000,
		ONE=0x0001,
		SRC_COLOR=0x0300,
		ONE_MINUS_SRC_COLOR=0x0301,
		SRC_ALPHA=0x0302,
		ONE_MINUS_SRC_ALPHA=0x0303,
		DST_ALPHA=0x0304,
		ONE_MINUS_DST_ALPHA=0x0305,
		DST_COLOR=0x0306,
		ONE_MINUS_DST_COLOR=0x0307,
		SRC_ALPHA_SATURATE=0x0308,
		CONSTANT_COLOR=0x8001,
		ONE_MINUS_CONSTANT_COLOR=0x8002,
		CONSTANT_ALPHA=0x8003,
		ONE_MINUS_CONSTANT_ALPHA=0x8004
	};
	enum NV2A_COMPARISON_OP {
		NEVER=0x0200,
		LESS=0x0201,
		EQUAL=0x0202,
		LEQUAL=0x0203,
		GREATER=0x0204,
		NOTEQUAL=0x0205,
		GEQUAL=0x0206,
		ALWAYS=0x0207
	};
	enum NV2A_STENCIL_OP {
		ZEROOP=0x0000,
		INVERTOP=0x150a,
		KEEP=0x1e00,
		REPLACE=0x1e01,
		INCR=0x1e02,
		DECR=0x1e03,
		INCR_WRAP=0x8507,
		DECR_WRAP=0x8508
	};
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
static void jamtable_disasm(running_machine &machine, address_space &space,UINT32 address,UINT32 size) // 0xff000080 == fff00080
{
	offs_t base,addr;
	UINT32 opcode,op1,op2;
	char sop1[16];
	char sop2[16];
	char pcrel[16];

	addr=(offs_t)address;
	if (!debug_cpu_translate(space,TRANSLATE_READ_DEBUG,&addr))
	{
		debug_console_printf(machine,"Address is unmapped.\n");
		return;
	}
	while (1)
	{
		base=addr;
		opcode=space.read_byte(addr);
		addr++;
		op1=space.read_dword_unaligned(addr);
		addr+=4;
		op2=space.read_dword_unaligned(addr);
		addr+=4;
		if (opcode == 0xe1)
		{
			opcode=op2 & 255;
			op2=op1;
			//op1=edi;
			sprintf(sop2,"%08X",op2);
			sprintf(sop1,"ACC");
			sprintf(pcrel,"PC+ACC");
		}
		else
		{
			sprintf(sop2,"%08X",op2);
			sprintf(sop1,"%08X",op1);
			sprintf(pcrel,"%08X",base+9+op1);
		}
		debug_console_printf(machine,"%08X ",base);
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
				debug_console_printf(machine,"POKEPCI PCICONF[%s]=%s\n",sop2,sop1);
				break;
			case 0x02:
				debug_console_printf(machine,"OUTB    PORT[%s]=%s\n",sop2,sop1);
				break;
			case 0x03:
				debug_console_printf(machine,"POKE    MEM[%s]=%s\n",sop2,sop1);
				break;
			case 0x04:
				debug_console_printf(machine,"BNE     IF ACC != %s THEN PC=%s\n",sop2,pcrel);
				break;
			case 0x05:
				// out cf8,op2
				// in acc,cfc
				debug_console_printf(machine,"PEEKPCI ACC=PCICONF[%s]\n",sop2);
				break;
			case 0x06:
				debug_console_printf(machine,"AND/OR  ACC=(ACC & %s) | %s\n",sop2,sop1);
				break;
			case 0x07:
				debug_console_printf(machine,"BRA     PC=%s\n",pcrel);
				break;
			case 0x08:
				debug_console_printf(machine,"INB     ACC=PORT[%s]\n",sop2);
				break;
			case 0x09:
				debug_console_printf(machine,"PEEK    ACC=MEM[%s]\n",sop2);
				break;
			case 0xee:
				debug_console_printf(machine,"END\n");
				break;
			default:
				debug_console_printf(machine,"NOP     ????\n");
				break;
		}
		if (opcode == 0xee)
			break;
		if (size <= 9)
			break;
		size-=9;
	}
}

static void jamtable_disasm_command(running_machine &machine, int ref, int params, const char **param)
{
	chihiro_state *state = machine.driver_data<chihiro_state>();
	address_space &space=state->m_maincpu->space();
	UINT64  addr,size;

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
	address_space &space=state->m_maincpu->space();
	UINT64  addr;
	offs_t address;
	UINT32 length,maximumlength;
	offs_t buffer;

	if (params < 1)
		return;
	if (!debug_command_parameter_number(machine, param[0], &addr))
		return;
	address=(offs_t)addr;
	if (!debug_cpu_translate(space,TRANSLATE_READ_DEBUG,&address))
	{
		debug_console_printf(machine,"Address is unmapped.\n");
		return;
	}
	length=space.read_word_unaligned(address);
	maximumlength=space.read_word_unaligned(address+2);
	buffer=space.read_dword_unaligned(address+4);
	debug_console_printf(machine,"Length %d word\n",length);
	debug_console_printf(machine,"MaximumLength %d word\n",maximumlength);
	debug_console_printf(machine,"Buffer %08X byte* ",buffer);
	if (!debug_cpu_translate(space,TRANSLATE_READ_DEBUG,&buffer))
	{
		debug_console_printf(machine,"\nBuffer is unmapped.\n");
		return;
	}
	if (length > 256)
		length=256;
	for (int a=0;a < length;a++)
	{
		UINT8 c=space.read_byte(buffer+a);
		debug_console_printf(machine,"%c",c);
	}
	debug_console_printf(machine,"\n");
}

static void dump_process_command(running_machine &machine, int ref, int params, const char **param)
{
	chihiro_state *state = machine.driver_data<chihiro_state>();
	address_space &space=state->m_maincpu->space();
	UINT64 addr;
	offs_t address;

	if (params < 1)
		return;
	if (!debug_command_parameter_number(machine, param[0], &addr))
		return;
	address=(offs_t)addr;
	if (!debug_cpu_translate(space,TRANSLATE_READ_DEBUG,&address))
	{
		debug_console_printf(machine,"Address is unmapped.\n");
		return;
	}
	debug_console_printf(machine,"ReadyListHead {%08X,%08X} _LIST_ENTRY\n",space.read_dword_unaligned(address),space.read_dword_unaligned(address+4));
	debug_console_printf(machine,"ThreadListHead {%08X,%08X} _LIST_ENTRY\n",space.read_dword_unaligned(address+8),space.read_dword_unaligned(address+12));
	debug_console_printf(machine,"StackCount %d dword\n",space.read_dword_unaligned(address+16));
	debug_console_printf(machine,"ThreadQuantum %d dword\n",space.read_dword_unaligned(address+20));
	debug_console_printf(machine,"BasePriority %d byte\n",space.read_byte(address+24));
	debug_console_printf(machine,"DisableBoost %d byte\n",space.read_byte(address+25));
	debug_console_printf(machine,"DisableQuantum %d byte\n",space.read_byte(address+26));
	debug_console_printf(machine,"_padding %d byte\n",space.read_byte(address+27));
}

static void dump_list_command(running_machine &machine, int ref, int params, const char **param)
{
	chihiro_state *state = machine.driver_data<chihiro_state>();
	address_space &space=state->m_maincpu->space();
	UINT64 addr,offs,start,old;
	offs_t address,offset;

	if (params < 1)
		return;
	if (!debug_command_parameter_number(machine, param[0], &addr))
		return;
	offs=0;
	offset=0;
	if (params >= 2)
	{
		if (!debug_command_parameter_number(machine, param[1], &offs))
			return;
		offset=(offs_t)offs;
	}
	start=addr;
	address=(offs_t)addr;
	if (!debug_cpu_translate(space,TRANSLATE_READ_DEBUG,&address))
	{
		debug_console_printf(machine,"Address is unmapped.\n");
		return;
	}
	if (params >= 2)
		debug_console_printf(machine,"Entry    Object\n");
	else
		debug_console_printf(machine,"Entry\n");
	for (int num=0;num < 32;num++)
	{
		if (params >= 2)
			debug_console_printf(machine,"%08X %08X\n",(UINT32)addr,(offs_t)addr-offset);
		else
			debug_console_printf(machine,"%08X\n",(UINT32)addr);
		old=addr;
		addr=space.read_dword_unaligned(address);
		if (addr == start)
			break;
		if (addr == old)
			break;
		address=(offs_t)addr;
		if (!debug_cpu_translate(space,TRANSLATE_READ_DEBUG,&address))
			break;
	}
}

static void curthread_command(running_machine &machine, int ref, int params, const char **param)
{
	chihiro_state *state = machine.driver_data<chihiro_state>();
	address_space &space=state->m_maincpu->space();
	UINT64 fsbase;
	UINT32 kthrd,topstack,tlsdata;
	offs_t address;

	fsbase = state->m_maincpu->state_int(44);
	address=(offs_t)fsbase+0x28;
	if (!debug_cpu_translate(space,TRANSLATE_READ_DEBUG,&address))
	{
		debug_console_printf(machine,"Address is unmapped.\n");
		return;
	}
	kthrd=space.read_dword_unaligned(address);
	debug_console_printf(machine,"Current thread is %08X\n",kthrd);
	address=(offs_t)kthrd+0x1c;
	if (!debug_cpu_translate(space,TRANSLATE_READ_DEBUG,&address))
		return;
	topstack=space.read_dword_unaligned(address);
	debug_console_printf(machine,"Current thread stack top is %08X\n",topstack);
	address=(offs_t)kthrd+0x28;
	if (!debug_cpu_translate(space,TRANSLATE_READ_DEBUG,&address))
		return;
	tlsdata=space.read_dword_unaligned(address);
	if (tlsdata == 0)
		address=(offs_t)topstack-0x210-8;
	else
		address=(offs_t)tlsdata-8;
	if (!debug_cpu_translate(space,TRANSLATE_READ_DEBUG,&address))
		return;
	debug_console_printf(machine,"Current thread function is %08X\n",space.read_dword_unaligned(address));
}

static void generate_irq_command(running_machine &machine, int ref, int params, const char **param)
{
	UINT64 irq;
	chihiro_state *chst=machine.driver_data<chihiro_state>();

	if (params < 1)
		return;
	if (!debug_command_parameter_number(machine, param[0], &irq))
		return;
	if (irq > 15)
		return;
	if (irq == 2)
		return;
	chst->debug_generate_irq((int)irq,true);
}

static void nv2a_combiners_command(running_machine &machine, int ref, int params, const char **param)
{
	int en;

	chihiro_state *chst=machine.driver_data<chihiro_state>();
	en=chst->nvidia_nv2a->toggle_register_combiners_usage();
	if (en != 0)
		debug_console_printf(machine,"Register combiners enabled\n");
	else
		debug_console_printf(machine,"Register combiners disabled\n");
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
	chst->nvidia_nv2a->debug_grab_texture((int)type,param[1]);
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
			instruction[1] = space.read_dword_unaligned(address+4);
			instruction[2] = space.read_dword_unaligned(address+8);
			instruction[3] = space.read_dword_unaligned(address+12);
		} else
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
	debug_console_printf(machine,"Available Chihiro commands:\n");
	debug_console_printf(machine,"  chihiro jamdis,<start>,<size> -- Disassemble <size> bytes of JamTable instructions starting at <start>\n");
	debug_console_printf(machine,"  chihiro dump_string,<address> -- Dump _STRING object at <address>\n");
	debug_console_printf(machine,"  chihiro dump_process,<address> -- Dump _PROCESS object at <address>\n");
	debug_console_printf(machine,"  chihiro dump_list,<address>[,<offset>] -- Dump _LIST_ENTRY chain starting at <address>\n");
	debug_console_printf(machine,"  chihiro curthread -- Print information about current thread\n");
	debug_console_printf(machine,"  chihiro irq,<number> -- Generate interrupt with irq number 0-15\n");
	debug_console_printf(machine,"  chihiro nv2a_combiners -- Toggle use of register combiners\n");
	debug_console_printf(machine,"  chihiro grab_texture,<type>,<filename> -- Save to <filename> the next used texture of type <type>\n");
	debug_console_printf(machine,"  chihiro grab_vprog,<filename> -- save current vertex program instruction slots to <filename>\n");
	debug_console_printf(machine,"  chihiro vprogdis,<address>,<length>[,<type>] -- disassemble <lenght> vertex program instructions at <address> of <type>\n");
	debug_console_printf(machine,"  chihiro help -- this list\n");
}

static void chihiro_debug_commands(running_machine &machine, int ref, int params, const char **param)
{
	if (params < 1)
		return;
	if (strcmp("jamdis",param[0]) == 0)
		jamtable_disasm_command(machine,ref,params-1,param+1);
	else if (strcmp("dump_string",param[0]) == 0)
		dump_string_command(machine,ref,params-1,param+1);
	else if (strcmp("dump_process",param[0]) == 0)
		dump_process_command(machine,ref,params-1,param+1);
	else if (strcmp("dump_list",param[0]) == 0)
		dump_list_command(machine,ref,params-1,param+1);
	else if (strcmp("curthread",param[0]) == 0)
		curthread_command(machine,ref,params-1,param+1);
	else if (strcmp("irq",param[0]) == 0)
		generate_irq_command(machine,ref,params-1,param+1);
	else if (strcmp("nv2a_combiners",param[0]) == 0)
		nv2a_combiners_command(machine,ref,params-1,param+1);
	else if (strcmp("grab_texture", param[0]) == 0)
		grab_texture_command(machine, ref, params - 1, param + 1);
	else if (strcmp("grab_vprog", param[0]) == 0)
		grab_vprog_command(machine, ref, params - 1, param + 1);
	else if (strcmp("vprogdis", param[0]) == 0)
		vprogdis_command(machine, ref, params - 1, param + 1);
	else
		help_command(machine,ref,params-1,param+1);
}

/*
 * Graphics
 */

UINT32 nv2a_renderer::dilate0(UINT32 value,int bits) // dilate first "bits" bits in "value"
{
	UINT32 x,m1,m2,m3;
	int a;

	x = value;
	for (a=0;a < bits;a++)
	{
		m2 = 1 << (a << 1);
		m1 = m2 - 1;
		m3 = (~m1) << 1;
		x = (x & m1) + (x & m2) + ((x & m3) << 1);
	}
	return x;
}

UINT32 nv2a_renderer::dilate1(UINT32 value,int bits) // dilate first "bits" bits in "value"
{
	UINT32 x,m1,m2,m3;
	int a;

	x = value;
	for (a=0;a < bits;a++)
	{
		m2 = 1 << (a << 1);
		m1 = m2 - 1;
		m3 = (~m1) << 1;
		x = (x & m1) + ((x & m2) << 1) + ((x & m3) << 1);
	}
	return x;
}

void nv2a_renderer::computedilated(void)
{
	int a,b;

	for (b=0;b < 16;b++)
		for (a=0;a < 2048;a++) {
			dilated0[b][a]=dilate0(a,b);
			dilated1[b][a]=dilate1(a,b);
		}
	for (b=0;b < 16;b++)
		for (a=0;a < 16;a++)
			dilatechose[(b << 4) + a]=(a < b ? a : b);
}

int nv2a_renderer::geforce_commandkind(UINT32 word)
{
	if ((word & 0x00000003) == 0x00000002)
		return 7; // call
	if ((word & 0x00000003) == 0x00000001)
		return 6; // jump
	if ((word & 0xE0030003) == 0x40000000)
		return 5; // non increasing
	if ((word & 0xE0000003) == 0x20000000)
		return 4; // old jump
	if ((word & 0xFFFF0003) == 0x00030000)
		return 3; // long non icreasing
	if ((word & 0xFFFFFFFF) == 0x00020000)
		return 2; // return
	if ((word & 0xFFFF0003) == 0x00010000)
		return 1; // sli conditional
	if ((word & 0xE0030003) == 0x00000000)
		return 0; // increasing
	return -1;
}

UINT32 nv2a_renderer::geforce_object_offset(UINT32 handle)
{
	UINT32 h=((((handle >> 11) ^ handle) >> 11) ^ handle) & 0x7ff;
	UINT32 o=(pfifo[0x210/4] & 0x1f) << 8; // or 12 ?
	UINT32 e=o+h*8; // at 0xfd000000+0x00700000
	UINT32 w;

	if (ramin[e/4] != handle)
		e=0;
	w=ramin[e/4+1];
	return (w & 0xffff)*0x10;
}

void nv2a_renderer::geforce_read_dma_object(UINT32 handle,UINT32 &offset,UINT32 &size)
{
	//UINT32 objclass,pt_present,pt_linear,access,target,rorw;
	UINT32 dma_adjust,dma_frame;
	UINT32 o=geforce_object_offset(handle);

	o=o/4;
	//objclass=ramin[o] & 0xfff;
	//pt_present=(ramin[o] >> 12) & 1;
	//pt_linear=(ramin[o] >> 13) & 1;
	//access=(ramin[o] >> 14) & 3;
	//target=(ramin[o] >> 16) & 3;
	dma_adjust=(ramin[o] >> 20) & 0xfff;
	size=ramin[o+1];
	//rorw=ramin[o+2] & 1;
	dma_frame=ramin[o+2] & 0xfffff000;
	offset=dma_frame+dma_adjust;
}

/*void myline(bitmap_rgb32 &bmp,float x1,float y1,float x2,float y2)
{
    int xx1,yy1,xx2,yy2;

    xx1=x1;
    xx2=x2;
    yy1=y1;
    yy2=y2;
    if (xx1 == xx2) {
        if (yy1 > yy2) {
            int t=yy1;
            yy1=yy2;
            yy2=t;
        }
        for (int y=yy1;y <= yy2;y++)
            *((UINT32 *)bmp.raw_pixptr(y,xx1))= -1;
    } else if (yy1 == yy2) {
        if (xx1 > xx2) {
            int t=xx1;
            xx1=xx2;
            xx2=t;
        }
        for (int x=xx1;x <= xx2;x++)
            *((UINT32 *)bmp.raw_pixptr(yy1,x))= -1;
    }
}*/

inline UINT32 convert_a4r4g4b4_a8r8g8b8(UINT32 a4r4g4b4)
{
	UINT32 a8r8g8b8;
	int ca,cr,cg,cb;

	cb=pal4bit(a4r4g4b4 & 0x000f);
	cg=pal4bit((a4r4g4b4 & 0x00f0) >> 4);
	cr=pal4bit((a4r4g4b4 & 0x0f00) >> 8);
	ca=pal4bit((a4r4g4b4 & 0xf000) >> 12);
	a8r8g8b8=(ca<<24)|(cr<<16)|(cg<<8)|(cb); // color converted to 8 bits per component
	return a8r8g8b8;
}

inline UINT32 convert_a1r5g5b5_a8r8g8b8(UINT32 a1r5g5b5)
{
	UINT32 a8r8g8b8;
	int ca,cr,cg,cb;

	cb=pal5bit(a1r5g5b5 & 0x001f);
	cg=pal5bit((a1r5g5b5 & 0x03e0) >> 5);
	cr=pal5bit((a1r5g5b5 & 0x7c00) >> 10);
	ca=a1r5g5b5 & 0x8000 ? 0xff : 0;
	a8r8g8b8=(ca<<24)|(cr<<16)|(cg<<8)|(cb); // color converted to 8 bits per component
	return a8r8g8b8;
}

inline UINT32 convert_r5g6b5_r8g8b8(UINT32 r5g6b5)
{
	UINT32 r8g8b8;
	int cr,cg,cb;

	cb=pal5bit(r5g6b5 & 0x001f);
	cg=pal6bit((r5g6b5 & 0x07e0) >> 5);
	cr=pal5bit((r5g6b5 & 0xf800) >> 11);
	r8g8b8=(cr<<16)|(cg<<8)|(cb); // color converted to 8 bits per component
	return r8g8b8;
}

UINT32 nv2a_renderer::texture_get_texel(int number,int x,int y)
{
	UINT32 to, s, c, sa, ca;
	UINT32 a4r4g4b4, a1r5g5b5, r5g6b5;
	int bx, by;
	int color0, color1, color0m2, color1m2, alpha0, alpha1;
	UINT32 codes;
	UINT64 alphas;
	int cr, cg, cb;

	// force to [0,size-1]
	x = (unsigned int)x & (texture[number].sizeu - 1);
	y = (unsigned int)y & (texture[number].sizev - 1);
	switch (texture[number].format) {
	case A8R8G8B8:
		to = dilated0[texture[number].dilate][x] + dilated1[texture[number].dilate][y]; // offset of texel in texture memory
		return *(((UINT32 *)texture[number].buffer) + to); // get texel color
	case DXT1:
		bx = x >> 2;
		by = y >> 2;
		x = x & 3;
		y = y & 3;
		to = bx + by*(texture[number].sizeu >> 2);
		color0 = *((UINT16 *)(((UINT64 *)texture[number].buffer) + to) + 0);
		color1 = *((UINT16 *)(((UINT64 *)texture[number].buffer) + to) + 1);
		codes = *((UINT32 *)(((UINT64 *)texture[number].buffer) + to) + 1);
		s = (y << 3) + (x << 1);
		c = (codes >> s) & 3;
		c = c + (color0 > color1 ? 0 : 4);
		color0m2 = color0 << 1;
		color1m2 = color1 << 1;
		switch (c) {
		case 0:
			return 0xff000000 + convert_r5g6b5_r8g8b8(color0);
		case 1:
			return 0xff000000 + convert_r5g6b5_r8g8b8(color1);
		case 2:
			cb = pal5bit(((color0m2 & 0x003e) + (color1 & 0x001f)) / 3);
			cg = pal6bit(((color0m2 & 0x0fc0) + (color1 & 0x07e0)) / 3 >> 5);
			cr = pal5bit(((color0m2 & 0x1f000) + color1) / 3 >> 11);
			return 0xff000000 | (cr << 16) | (cg << 8) | (cb);
		case 3:
			cb = pal5bit(((color1m2 & 0x003e) + (color0 & 0x001f)) / 3);
			cg = pal6bit(((color1m2 & 0x0fc0) + (color0 & 0x07e0)) / 3 >> 5);
			cr = pal5bit(((color1m2 & 0x1f000) + color0) / 3 >> 11);
			return 0xff000000 | (cr << 16) | (cg << 8) | (cb);
		case 4:
			return 0xff000000 + convert_r5g6b5_r8g8b8(color0);
		case 5:
			return 0xff000000 + convert_r5g6b5_r8g8b8(color1);
		case 6:
			cb = pal5bit(((color0 & 0x001f) + (color1 & 0x001f)) / 2);
			cg = pal6bit(((color0 & 0x07e0) + (color1 & 0x07e0)) / 2 >> 5);
			cr = pal5bit(((color0 & 0xf800) + (color1 & 0xf800)) / 2 >> 11);
			return 0xff000000 | (cr << 16) | (cg << 8) | (cb);
		default:
			return 0xff000000;
		}
	case DXT3:
		bx = x >> 2;
		by = y >> 2;
		x = x & 3;
		y = y & 3;
		to = (bx + by*(texture[number].sizeu >> 2)) << 1;
		color0 = *((UINT16 *)(((UINT64 *)texture[number].buffer) + to) + 4);
		color1 = *((UINT16 *)(((UINT64 *)texture[number].buffer) + to) + 5);
		codes = *((UINT32 *)(((UINT64 *)texture[number].buffer) + to) + 3);
		alphas = *(((UINT64 *)texture[number].buffer) + to);
		s = (y << 3) + (x << 1);
		sa = ((y << 2) + x) << 2;
		c = (codes >> s) & 3;
		ca = (alphas >> sa) & 15;
		switch (c) {
		case 0:
			return ((ca + (ca << 4)) << 24) + convert_r5g6b5_r8g8b8(color0);
		case 1:
			return ((ca + (ca << 4)) << 24) + convert_r5g6b5_r8g8b8(color1);
		case 2:
			cb = pal5bit((2 * (color0 & 0x001f) + (color1 & 0x001f)) / 3);
			cg = pal6bit((2 * (color0 & 0x07e0) + (color1 & 0x07e0)) / 3 >> 5);
			cr = pal5bit((2 * (color0 & 0xf800) + (color1 & 0xf800)) / 3 >> 11);
			return ((ca + (ca << 4)) << 24) | (cr << 16) | (cg << 8) | (cb);
		default:
			cb = pal5bit(((color0 & 0x001f) + 2 * (color1 & 0x001f)) / 3);
			cg = pal6bit(((color0 & 0x07e0) + 2 * (color1 & 0x07e0)) / 3 >> 5);
			cr = pal5bit(((color0 & 0xf800) + 2 * (color1 & 0xf800)) / 3 >> 11);
			return ((ca + (ca << 4)) << 24) | (cr << 16) | (cg << 8) | (cb);
		}
	case A4R4G4B4:
		to = dilated0[texture[number].dilate][x] + dilated1[texture[number].dilate][y]; // offset of texel in texture memory
		a4r4g4b4 = *(((UINT16 *)texture[number].buffer) + to); // get texel color
		return convert_a4r4g4b4_a8r8g8b8(a4r4g4b4);
	case A1R5G5B5:
		to = dilated0[texture[number].dilate][x] + dilated1[texture[number].dilate][y]; // offset of texel in texture memory
		a1r5g5b5 = *(((UINT16 *)texture[number].buffer) + to); // get texel color
		return convert_a1r5g5b5_a8r8g8b8(a1r5g5b5);
	case R5G6B5:
		to = dilated0[texture[number].dilate][x] + dilated1[texture[number].dilate][y]; // offset of texel in texture memory
		r5g6b5 = *(((UINT16 *)texture[number].buffer) + to); // get texel color
		return 0xff000000 + convert_r5g6b5_r8g8b8(r5g6b5);
	case R8G8B8_RECT:
		to = texture[number].rectangle_pitch*y + (x << 2);
		return *((UINT32 *)(((UINT8 *)texture[number].buffer) + to));
	case A8R8G8B8_RECT:
		to = texture[number].rectangle_pitch*y + (x << 2);
		return *((UINT32 *)(((UINT8 *)texture[number].buffer) + to));
	case DXT5:
		bx = x >> 2;
		by = y >> 2;
		x = x & 3;
		y = y & 3;
		to = (bx + by*(texture[number].sizeu >> 2)) << 1;
		color0 = *((UINT16 *)(((UINT64 *)texture[number].buffer) + to) + 4);
		color1 = *((UINT16 *)(((UINT64 *)texture[number].buffer) + to) + 5);
		codes = *((UINT32 *)(((UINT64 *)texture[number].buffer) + to) + 3);
		alpha0 = *((UINT8 *)(((UINT64 *)texture[number].buffer) + to) + 0);
		alpha1 = *((UINT8 *)(((UINT64 *)texture[number].buffer) + to) + 1);
		alphas = *(((UINT64 *)texture[number].buffer) + to);
		s = (y << 3) + (x << 1);
		sa = ((y << 2) + x) * 3;
		c = (codes >> s) & 3;
		ca = (alphas >> sa) & 7;
		ca = ca + (alpha0 > alpha1 ? 0 : 8);
		switch (ca) {
		case 0:
			ca = alpha0;
			break;
		case 1:
			ca = alpha1;
			break;
		case 2:
			ca = (6 * alpha0 + 1 * alpha1) / 7;
			break;
		case 3:
			ca = (5 * alpha0 + 2 * alpha1) / 7;
			break;
		case 4:
			ca = (4 * alpha0 + 3 * alpha1) / 7;
			break;
		case 5:
			ca = (3 * alpha0 + 4 * alpha1) / 7;
			break;
		case 6:
			ca = (2 * alpha0 + 5 * alpha1) / 7;
			break;
		case 7:
			ca = (1 * alpha0 + 6 * alpha1) / 7;
			break;
		case 8:
			ca = alpha0;
			break;
		case 9:
			ca = alpha1;
			break;
		case 10:
			ca = (4 * alpha0 + 1 * alpha1) / 5;
			break;
		case 11:
			ca = (3 * alpha0 + 2 * alpha1) / 5;
			break;
		case 12:
			ca = (2 * alpha0 + 3 * alpha1) / 5;
			break;
		case 13:
			ca = (1 * alpha0 + 4 * alpha1) / 5;
			break;
		case 14:
			ca = 0;
			break;
		case 15:
			ca = 255;
			break;
		}
		switch (c) {
		case 0:
			return (ca << 24) + convert_r5g6b5_r8g8b8(color0);
		case 1:
			return (ca << 24) + convert_r5g6b5_r8g8b8(color1);
		case 2:
			cb = pal5bit((2 * (color0 & 0x001f) + (color1 & 0x001f)) / 3);
			cg = pal6bit((2 * (color0 & 0x07e0) + (color1 & 0x07e0)) / 3 >> 5);
			cr = pal5bit((2 * (color0 & 0xf800) + (color1 & 0xf800)) / 3 >> 11);
			return (ca << 24) | (cr << 16) | (cg << 8) | (cb);
		default:
			cb = pal5bit(((color0 & 0x001f) + 2 * (color1 & 0x001f)) / 3);
			cg = pal6bit(((color0 & 0x07e0) + 2 * (color1 & 0x07e0)) / 3 >> 5);
			cr = pal5bit(((color0 & 0xf800) + 2 * (color1 & 0xf800)) / 3 >> 11);
			return (ca << 24) | (cr << 16) | (cg << 8) | (cb);
		}
	default:
		return 0xff00ff00;
	}
}

void nv2a_renderer::write_pixel(int x, int y, UINT32 color)
{
	void *addr;
	UINT32 fbcolor;
	UINT32 c[4], fb[4], s[4], d[4], cc[4];

	addr = this->fb.raw_pixptr(y, x);
	fbcolor = *((UINT32 *)addr);
	c[3] = color >> 24;
	c[2] = (color >> 16) & 255;
	c[1] = (color >> 8) & 255;
	c[0] = color & 255;
	fb[3] = fbcolor >> 24;
	fb[2] = (fbcolor >> 16) & 255;
	fb[1] = (fbcolor >> 8) & 255;
	fb[0] = fbcolor & 255;
	cc[3] = blend_color >> 24;
	cc[2] = (blend_color >> 16) & 255;
	cc[1] = (blend_color >> 8) & 255;
	cc[0] = blend_color & 255;
	// ownership test and scissor test not done
	// alpha test
	if (alpha_test_enabled) {
		switch (alpha_func) {
		case nv2a_renderer::NEVER:
			return;
		case nv2a_renderer::ALWAYS:
		default:
			break;
		case nv2a_renderer::LESS:
			if (c[3] >= alpha_reference)
				return;
			break;
		case nv2a_renderer::LEQUAL:
			if (c[3] > alpha_reference)
				return;
			break;
		case nv2a_renderer::EQUAL:
			if (c[3] != alpha_reference)
				return;
			break;
		case nv2a_renderer::GEQUAL:
			if (c[3] < alpha_reference)
				return;
			break;
		case nv2a_renderer::GREATER:
			if (c[3] <= alpha_reference)
				return;
			break;
		case nv2a_renderer::NOTEQUAL:
			if (c[3] == alpha_reference)
				return;
			break;
		}
	}
	// stencil test not done
	// depth buffer test not done
	// blending
	if (blending_enabled) {
		switch (blend_function_source) {
		case nv2a_renderer::ZERO:
			s[3] = s[2] = s[1] = s[0] = 0;
			break;
		case nv2a_renderer::ONE:
		default:
			s[3] = s[2] = s[1] = s[0] = 255;
			break;
		case nv2a_renderer::DST_COLOR:
			s[3] = fb[3];
			s[2] = fb[2];
			s[1] = fb[1];
			s[0] = fb[0];
			break;
		case nv2a_renderer::ONE_MINUS_DST_COLOR:
			s[3] = fb[3] ^ 255;
			s[2] = fb[2] ^ 255;
			s[1] = fb[1] ^ 255;
			s[0] = fb[0] ^ 255;
			break;
		case nv2a_renderer::SRC_ALPHA:
			s[3] = s[2] = s[1] = s[0] = c[3];
			break;
		case nv2a_renderer::ONE_MINUS_SRC_ALPHA:
			s[3] = s[2] = s[1] = s[0] = c[3] ^ 255;
			break;
		case nv2a_renderer::DST_ALPHA:
			s[3] = s[2] = s[1] = s[0] = fb[3];
			break;
		case nv2a_renderer::ONE_MINUS_DST_ALPHA:
			s[3] = s[2] = s[1] = s[0] = fb[3] ^ 255;
			break;
		case nv2a_renderer::CONSTANT_COLOR:
			s[3] = cc[3];
			s[2] = cc[2];
			s[1] = cc[1];
			s[0] = cc[0];
			break;
		case nv2a_renderer::ONE_MINUS_CONSTANT_COLOR:
			s[3] = cc[3] ^ 255;
			s[2] = cc[2] ^ 255;
			s[1] = cc[1] ^ 255;
			s[0] = cc[0] ^ 255;
			break;
		case nv2a_renderer::CONSTANT_ALPHA:
			s[3] = s[2] = s[1] = s[0] = cc[3];
			break;
		case nv2a_renderer::ONE_MINUS_CONSTANT_ALPHA:
			s[3] = s[2] = s[1] = s[0] = cc[3] ^ 255;
			break;
		case nv2a_renderer::SRC_ALPHA_SATURATE:
			s[3] = 255;
			if (c[3] < (fb[3] ^ 255))
				s[2] = c[3];
			else
				s[2] = fb[3];
			s[1] = s[0] = s[2];
			break;
		}
		switch (blend_function_destination) {
		case nv2a_renderer::ZERO:
		default:
			d[3] = d[2] = d[1] = d[0] = 0;
			break;
		case nv2a_renderer::ONE:
			d[3] = d[2] = d[1] = d[0] = 255;
			break;
		case nv2a_renderer::SRC_COLOR:
			d[3] = c[3];
			d[2] = c[2];
			d[1] = c[1];
			d[0] = c[0];
			break;
		case nv2a_renderer::ONE_MINUS_SRC_COLOR:
			d[3] = c[3] ^ 255;
			d[2] = c[2] ^ 255;
			d[1] = c[1] ^ 255;
			d[0] = c[0] ^ 255;
			break;
		case nv2a_renderer::SRC_ALPHA:
			d[3] = d[2] = d[1] = d[0] = c[3];
			break;
		case nv2a_renderer::ONE_MINUS_SRC_ALPHA:
			d[3] = d[2] = d[1] = d[0] = c[3] ^ 255;
			break;
		case nv2a_renderer::DST_ALPHA:
			d[3] = d[2] = d[1] = d[0] = fb[3];
			break;
		case nv2a_renderer::ONE_MINUS_DST_ALPHA:
			d[3] = d[2] = d[1] = d[0] = fb[3] ^ 255;
			break;
		case nv2a_renderer::CONSTANT_COLOR:
			d[3] = cc[3];
			d[2] = cc[2];
			d[1] = cc[1];
			d[0] = cc[0];
			break;
		case nv2a_renderer::ONE_MINUS_CONSTANT_COLOR:
			d[3] = cc[3] ^ 255;
			d[2] = cc[2] ^ 255;
			d[1] = cc[1] ^ 255;
			d[0] = cc[0] ^ 255;
			break;
		case nv2a_renderer::CONSTANT_ALPHA:
			d[3] = d[2] = d[1] = d[0] = cc[3];
			break;
		case nv2a_renderer::ONE_MINUS_CONSTANT_ALPHA:
			d[3] = d[2] = d[1] = d[0] = cc[3] ^ 255;
			break;
		}
		switch (blend_equation) {
		case nv2a_renderer::FUNC_ADD:
			c[3] = (c[3]*s[3] + fb[3]*d[3]) / 255;
			if (c[3] > 255)
				c[3] = 255;
			c[2] = (c[2]*s[2] + fb[2]*d[2]) / 255;
			if (c[2] > 255)
				c[2] = 255;
			c[1] = (c[1]*s[1] + fb[1]*d[1]) / 255;
			if (c[1] > 255)
				c[1] = 255;
			c[0] = (c[0]*s[0] + fb[0]*d[0]) / 255;
			if (c[0] > 255)
				c[0] = 255;
			break;
		case nv2a_renderer::FUNC_SUBTRACT:
			c[3] = (c[3]*s[3] - fb[3]*d[3]) / 255;
			if (c[3] < 0)
				c[3] = 255;
			c[2] = (c[2]*s[2] - fb[2]*d[2]) / 255;
			if (c[2] < 0)
				c[2] = 255;
			c[1] = (c[1]*s[1] - fb[1]*d[1]) / 255;
			if (c[1] < 0)
				c[1] = 255;
			c[0] = (c[0]*s[0] - fb[0]*d[0]) / 255;
			if (c[0] < 0)
				c[0] = 255;
			break;
		case nv2a_renderer::FUNC_REVERSE_SUBTRACT:
			c[3] = (fb[3] * d[3] - c[3] * s[3]) / 255;
			if (c[3] < 0)
				c[3] = 255;
			c[2] = (fb[2] * d[2] - c[2] * s[2]) / 255;
			if (c[2] < 0)
				c[2] = 255;
			c[1] = (fb[1] * d[1] - c[1] * s[1]) / 255;
			if (c[1] < 0)
				c[1] = 255;
			c[0] = (fb[0] * d[0] - c[0] * s[0]) / 255;
			if (c[0] < 0)
				c[0] = 255;
			break;
		case nv2a_renderer::MIN:
			c[3] = s[3];
			if (d[3] < c[3])
				c[3] = d[3];
			c[2] = s[2];
			if (d[2] < c[2])
				c[2] = d[2];
			c[1] = s[1];
			if (d[1] < c[1])
				c[1] = d[1];
			c[0] = s[0];
			if (d[0] < c[0])
				c[0] = d[0];
			break;
		case nv2a_renderer::MAX:
			c[3] = s[3];
			if (d[3] > c[3])
				c[3] = d[3];
			c[2] = s[2];
			if (d[2] > c[2])
				c[2] = d[2];
			c[1] = s[1];
			if (d[1] > c[1])
				c[1] = d[1];
			c[0] = s[0];
			if (d[0] > c[0])
				c[0] = d[0];
			break;
		}
	}
	// dithering not done
	// logical operation
	if (logical_operation_enabled) {
		switch (logical_operation) {
		case  nv2a_renderer::CLEAR:
			c[3] = 0;
			c[2] = 0;
			c[1] = 0;
			c[0] = 0;
			break;
		case  nv2a_renderer::AND:
			c[3] = c[3] & fb[3];
			c[2] = c[2] & fb[2];
			c[1] = c[1] & fb[1];
			c[0] = c[0] & fb[0];
			break;
		case  nv2a_renderer::AND_REVERSE:
			c[3] = c[3] & (fb[3] ^ 255);
			c[2] = c[2] & (fb[2] ^ 255);
			c[1] = c[1] & (fb[1] ^ 255);
			c[0] = c[0] & (fb[0] ^ 255);
			break;
		case  nv2a_renderer::COPY:
		default:
			break;
		case  nv2a_renderer::AND_INVERTED:
			c[3] = (c[3] ^ 255) & fb[3];
			c[2] = (c[2] ^ 255) & fb[2];
			c[1] = (c[1] ^ 255) & fb[1];
			c[0] = (c[0] ^ 255) & fb[0];
			break;
		case  nv2a_renderer::NOOP:
			c[3] = fb[3];
			c[2] = fb[2];
			c[1] = fb[1];
			c[0] = fb[0];
			break;
		case  nv2a_renderer::XOR:
			c[3] = c[3] ^ fb[3];
			c[2] = c[2] ^ fb[2];
			c[1] = c[1] ^ fb[1];
			c[0] = c[0] ^ fb[0];
			break;
		case  nv2a_renderer::OR:
			c[3] = c[3] | fb[3];
			c[2] = c[2] | fb[2];
			c[1] = c[1] | fb[1];
			c[0] = c[0] | fb[0];
			break;
		case  nv2a_renderer::NOR:
			c[3] = (c[3] | fb[3]) ^ 255;
			c[2] = (c[2] | fb[2]) ^ 255;
			c[1] = (c[1] | fb[1]) ^ 255;
			c[0] = (c[0] | fb[0]) ^ 255;
			break;
		case  nv2a_renderer::EQUIV:
			c[3] = (c[3] ^ fb[3]) ^ 255;
			c[2] = (c[2] ^ fb[2]) ^ 255;
			c[1] = (c[1] ^ fb[1]) ^ 255;
			c[0] = (c[0] ^ fb[0]) ^ 255;
			break;
		case  nv2a_renderer::INVERT:
			c[3] = fb[3] ^ 255;
			c[2] = fb[2] ^ 255;
			c[1] = fb[1] ^ 255;
			c[0] = fb[0] ^ 255;
			break;
		case  nv2a_renderer::OR_REVERSE:
			c[3] = c[3] | (fb[3] ^ 255);
			c[2] = c[2] | (fb[2] ^ 255);
			c[1] = c[1] | (fb[1] ^ 255);
			c[0] = c[0] | (fb[0] ^ 255);
			break;
		case  nv2a_renderer::COPY_INVERTED:
			c[3] = c[3] ^ 255;
			c[2] = c[2] ^ 255;
			c[1] = c[1] ^ 255;
			c[0] = c[0] ^ 255;
			break;
		case  nv2a_renderer::OR_INVERTED:
			c[3] = (c[3] ^ 255) | fb[3];
			c[2] = (c[2] ^ 255) | fb[2];
			c[1] = (c[1] ^ 255) | fb[1];
			c[0] = (c[0] ^ 255) | fb[0];
			break;
		case  nv2a_renderer::NAND:
			c[3] = (c[3] & fb[3]) ^ 255;
			c[2] = (c[2] & fb[2]) ^ 255;
			c[1] = (c[1] & fb[1]) ^ 255;
			c[0] = (c[0] & fb[0]) ^ 255;
			break;
		case  nv2a_renderer::SET:
			c[3] = 255;
			c[2] = 255;
			c[1] = 255;
			c[0] = 255;
			break;
		}
	}
	fbcolor = (c[3] << 24) | (c[2] << 16) | (c[1] << 8) | c[0];
	*((UINT32 *)addr) = fbcolor;
}

void nv2a_renderer::render_color(INT32 scanline, const extent_t &extent, const nvidia_object_data &objectdata, int threadid)
{
	int x;

	x=extent.stopx-extent.startx-1; // number of pixels to draw
	while (x >= 0) {
		UINT32 a8r8g8b8;
		int ca,cr,cg,cb;
		int xp=extent.startx+x; // x coordinate of current pixel

		cb=(extent.param[0].start+(float)x*extent.param[0].dpdx);
		cg=(extent.param[1].start+(float)x*extent.param[1].dpdx);
		cr=(extent.param[2].start+(float)x*extent.param[2].dpdx);
		ca=(extent.param[3].start+(float)x*extent.param[3].dpdx);
		a8r8g8b8=(ca << 24)+(cr << 16)+(cg << 8)+cb; // pixel color obtained by interpolating the colors of the vertices
		write_pixel(xp, scanline, a8r8g8b8);
		x--;
	}
}

void nv2a_renderer::render_texture_simple(INT32 scanline, const extent_t &extent, const nvidia_object_data &objectdata, int threadid)
{
	int x;
	UINT32 a8r8g8b8;

	if (!objectdata.data->texture[0].enabled) {
		return;
	}
	x=extent.stopx-extent.startx-1;
	while (x >= 0) {
		int up,vp;
		int xp=extent.startx+x; // x coordinate of current pixel

		up=(extent.param[4].start+(float)x*extent.param[4].dpdx)*(float)(objectdata.data->texture[0].sizeu-1); // x coordinate of texel in texture
		vp=extent.param[5].start*(float)(objectdata.data->texture[0].sizev-1); // y coordinate of texel in texture
		a8r8g8b8=texture_get_texel(0, up, vp);
		write_pixel(xp, scanline, a8r8g8b8);
		x--;
	}
}

void nv2a_renderer::render_register_combiners(INT32 scanline, const extent_t &extent, const nvidia_object_data &objectdata, int threadid)
{
	int x,xp;
	int up,vp;
	int ca,cr,cg,cb;
	UINT32 color[6];
	UINT32 a8r8g8b8;
	int n;//,m,i,j,k;

	color[0] = color[1] = color[2] = color[3] = color[4] = color[5] = 0;

	osd_lock_acquire(combiner.lock); // needed since multithreading is not supported yet
	x=extent.stopx-extent.startx-1; // number of pixels to draw
	while (x >= 0) {
		xp=extent.startx+x;
		// 1: fetch data
		// 1.1: interpolated color from vertices
		cb=(extent.param[0].start+(float)x*extent.param[0].dpdx);
		cg=(extent.param[1].start+(float)x*extent.param[1].dpdx);
		cr=(extent.param[2].start+(float)x*extent.param[2].dpdx);
		ca=(extent.param[3].start+(float)x*extent.param[3].dpdx);
		color[0]=(ca << 24)+(cr << 16)+(cg << 8)+cb; // pixel color obtained by interpolating the colors of the vertices
		color[1]=0; // lighting not yet
		// 1.2: color for each of the 4 possible textures
		for (n=0;n < 4;n++) {
			if (texture[n].enabled) {
				up=(extent.param[4+n*2].start+(float)x*extent.param[4+n*2].dpdx)*(float)(objectdata.data->texture[n].sizeu-1);
				vp=extent.param[5+n*2].start*(float)(objectdata.data->texture[n].sizev-1);
				color[n+2]=texture_get_texel(n, up, vp);
			}
		}
		// 2: compute
		// 2.1: initialize
		combiner_initialize_registers(color);
		// 2.2: general cmbiner stages
		for (n=0;n < combiner.stages;n++) {
			// 2.2.1 initialize
			combiner_initialize_stage(n);
			// 2.2.2 map inputs
			combiner_map_input(n);
			// 2.2.3 compute possible outputs
			combiner_compute_rgb_outputs(n);
			combiner_compute_a_outputs(n);
			// 2.2.4 map outputs to registers
			combiner_map_output(n);
		}
		// 2.3: final cmbiner stage
		combiner_initialize_final();
		combiner_map_final_input();
		combiner_final_output();
		a8r8g8b8=combiner_float_argb8(combiner.output);
		// 3: write pixel
		write_pixel(xp, scanline, a8r8g8b8);
		x--;
	}
	osd_lock_release(combiner.lock);
}

#if 0
const char *rc_mapping_str[]={
	"UNSIGNED_IDENTITY",
	"UNSIGNED_INVERT",
	"EXPAND_NORMAL",
	"EXPAND_NEGATE",
	"HALF_BIAS_NORMAL",
	"HALF_BIAS_NEGATE",
	"SIGNED_IDENTITY",
	"SIGNED_NEGATE"
};

const char *rc_usage_rgb_str[]={
	"RGB",
	"ALPHA"
};

const char *rc_usage_alpha_str[]={
	"BLUE",
	"ALPHA"
};

const char *rc_variable_str[]={
	"ZERO",
	"CONSTANT_COLOR0",
	"CONSTANT_COLOR1",
	"FOG",
	"PRIMARY_COLOR",
	"SECONDARY_COLOR",
	"???",
	"???",
	"TEXTURE0",
	"TEXTURE1",
	"TEXTURE2",
	"TEXTURE3",
	"SPARE0",
	"SPARE1",
	"SPARE0_PLUS_SECONDARY_COLOR",
	"E_TIMES_F"
};

const char *rc_bias_str[]={
	"NONE",
	"BIAS_BY_NEGATIVE_ONE_HALF"
};

const char *rc_scale_str[]={
	"NONE",
	"SCALE_BY_TWO",
	"SCALE_BY_FOUR",
	"SCALE_BY_ONE_HALF"
};

/* Dump the current setup of the register combiners */
void dumpcombiners(UINT32 *m)
{
	int a,b,n,v;

	n=m[0x1e60/4] & 0xf;
	printf("Combiners active: %d\n\r",n);
	for (a=0;a < n;a++) {
		printf("Combiner %d\n\r",a+1);
		printf(" RC_IN_ALPHA %08X\n\r",m[0x0260/4+a]);
		for (b=24;b >= 0;b=b-8) {
			v=(m[0x0260/4+a] >> b) & 0xf;
			printf("  %c_INPUT %s\n\r",'A'+3-b/8,rc_variable_str[v]);
			v=(m[0x0260/4+a] >> (b+4)) & 1;
			printf("  %c_COMPONENT_USAGE %s\n\r",'A'+3-b/8,rc_usage_alpha_str[v]);
			v=(m[0x0260/4+a] >> (b+5)) & 7;
			printf("  %c_MAPPING %s\n\r",'A'+3-b/8,rc_mapping_str[v]);
		}
		printf(" RC_IN_RGB %08X\n\r",m[0x0ac0/4+a]);
		for (b=24;b >= 0;b=b-8) {
			v=(m[0x0ac0/4+a] >> b) & 0xf;
			printf("  %c_INPUT %s\n\r",'A'+3-b/8,rc_variable_str[v]);
			v=(m[0x0ac0/4+a] >> (b+4)) & 1;
			printf("  %c_COMPONENT_USAGE %s\n\r",'A'+3-b/8,rc_usage_rgb_str[v]);
			v=(m[0x0ac0/4+a] >> (b+5)) & 7;
			printf("  %c_MAPPING %s\n\r",'A'+3-b/8,rc_mapping_str[v]);
		}
		printf(" RC_OUT_ALPHA %08X\n\r",m[0x0aa0/4+a]);
		v=m[0x0aa0/4+a] & 0xf;
		printf("  CD_OUTPUT %s\n\r",rc_variable_str[v]);
		v=(m[0x0aa0/4+a] >> 4) & 0xf;
		printf("  AB_OUTPUT %s\n\r",rc_variable_str[v]);
		v=(m[0x0aa0/4+a] >> 8) & 0xf;
		printf("  SUM_OUTPUT %s\n\r",rc_variable_str[v]);
		v=(m[0x0aa0/4+a] >> 12) & 1;
		printf("  CD_DOT_PRODUCT %d\n\r",v);
		v=(m[0x0aa0/4+a] >> 13) & 1;
		printf("  AB_DOT_PRODUCT %d\n\r",v);
		v=(m[0x0aa0/4+a] >> 14) & 1;
		printf("  MUX_SUM %d\n\r",v);
		v=(m[0x0aa0/4+a] >> 15) & 1;
		printf("  BIAS %s\n\r",rc_bias_str[v]);
		v=(m[0x0aa0/4+a] >> 16) & 3;
		printf("  SCALE %s\n\r",rc_scale_str[v]);
		//v=(m[0x0aa0/4+a] >> 27) & 7;
		printf(" RC_OUT_RGB %08X\n\r",m[0x1e40/4+a]);
		v=m[0x1e40/4+a] & 0xf;
		printf("  CD_OUTPUT %s\n\r",rc_variable_str[v]);
		v=(m[0x1e40/4+a] >> 4) & 0xf;
		printf("  AB_OUTPUT %s\n\r",rc_variable_str[v]);
		v=(m[0x1e40/4+a] >> 8) & 0xf;
		printf("  SUM_OUTPUT %s\n\r",rc_variable_str[v]);
		v=(m[0x1e40/4+a] >> 12) & 1;
		printf("  CD_DOT_PRODUCT %d\n\r",v);
		v=(m[0x1e40/4+a] >> 13) & 1;
		printf("  AB_DOT_PRODUCT %d\n\r",v);
		v=(m[0x1e40/4+a] >> 14) & 1;
		printf("  MUX_SUM %d\n\r",v);
		v=(m[0x1e40/4+a] >> 15) & 1;
		printf("  BIAS %s\n\r",rc_bias_str[v]);
		v=(m[0x1e40/4+a] >> 16) & 3;
		printf("  SCALE %s\n\r",rc_scale_str[v]);
		//v=(m[0x1e40/4+a] >> 27) & 7;
		printf("\n\r");
	}
	printf("Combiner final %08X %08X\n\r",m[0x0288/4],m[0x028c/4]);
	for (a=24;a >= 0;a=a-8) {
		n=(m[0x0288/4] >> a) & 0xf;
		printf("  %c_INPUT %s\n\r",'A'+3-a/8,rc_variable_str[n]);
		n=(m[0x0288/4] >> (a+4)) & 1;
		printf("  %c_COMPONENT_USAGE %s\n\r",'A'+3-a/8,rc_usage_rgb_str[n]);
		n=(m[0x0288/4] >> (a+5)) & 7;
		printf("  %c_MAPPING %s\n\r",'A'+3-a/8,rc_mapping_str[n]);
	}
	for (a=24;a >= 8;a=a-8) {
		n=(m[0x028c/4] >> a) & 0xf;
		printf("  %c_INPUT %s\n\r",'E'+3-a/8,rc_variable_str[n]);
		n=(m[0x028c/4] >> (a+4)) & 1;
		printf("  %c_COMPONENT_USAGE %s\n\r",'E'+3-a/8,rc_usage_rgb_str[n]);
		n=(m[0x028c/4] >> (a+5)) & 7;
		printf("  %c_MAPPING %s\n\r",'E'+3-a/8,rc_mapping_str[n]);
	}
	n=(m[0x028c/4] >> 7) & 1;
	printf(" color sum clamp: %d\n\r",n);
}
#endif

/* Read vertices data from system memory. Method 0x1810 */
int nv2a_renderer::read_vertices_0x1810(address_space & space, vertex *destination, int offset, int limit)
{
	UINT32 m, u;

	for (m = 0; m < limit; m++) {
		destination[m].attribute[0].iv[0] = space.read_dword(vertexbuffer_address[0] + (m + offset)*vertexbuffer_stride[0] + 0);
		destination[m].attribute[0].iv[1] = space.read_dword(vertexbuffer_address[0] + (m + offset)*vertexbuffer_stride[0] + 4);
		destination[m].attribute[0].iv[2] = space.read_dword(vertexbuffer_address[0] + (m + offset)*vertexbuffer_stride[0] + 8);
		destination[m].attribute[0].iv[3] = space.read_dword(vertexbuffer_address[0] + (m + offset)*vertexbuffer_stride[0] + 12);
		destination[m].attribute[3].iv[0] = space.read_dword(vertexbuffer_address[3] + (m + offset)*vertexbuffer_stride[3] + 0); // color
		for (u = 0; u < 4; u++) {
			destination[m].attribute[9 + u].iv[0] = space.read_dword(vertexbuffer_address[9 + u] + (m + offset)*vertexbuffer_stride[9 + u] + 0);
			destination[m].attribute[9 + u].iv[1] = space.read_dword(vertexbuffer_address[9 + u] + (m + offset)*vertexbuffer_stride[9 + u] + 4);
		}
	}
	return m;
}

/* Read vertices data from system memory. Method 0x1800 */
int nv2a_renderer::read_vertices_0x1800(address_space & space, vertex *destination, UINT32 address, int limit)
{
	UINT32 data;
	UINT32 m, u, i, c;

	c = 0;
	for (m = 0; m < limit; m++) {
		if (indexesleft_count == 0) {
			data = space.read_dword(address);
			i = (indexesleft_first + indexesleft_count) & 7;
			indexesleft[i] = data & 0xffff;
			indexesleft[(i + 1) & 7] = (data >> 16) & 0xffff;
			indexesleft_count = indexesleft_count + 2;
			address += 4;
			c++;
		}
		destination[m].attribute[0].iv[0] = space.read_dword(vertexbuffer_address[0] + indexesleft[indexesleft_first] * vertexbuffer_stride[0] + 0);
		destination[m].attribute[0].iv[1] = space.read_dword(vertexbuffer_address[0] + indexesleft[indexesleft_first] * vertexbuffer_stride[0] + 4);
		destination[m].attribute[0].iv[2] = space.read_dword(vertexbuffer_address[0] + indexesleft[indexesleft_first] * vertexbuffer_stride[0] + 8);
		destination[m].attribute[0].iv[3] = space.read_dword(vertexbuffer_address[0] + indexesleft[indexesleft_first] * vertexbuffer_stride[0] + 12);
		destination[m].attribute[3].iv[0] = space.read_dword(vertexbuffer_address[3] + indexesleft[indexesleft_first] * vertexbuffer_stride[3] + 0); // color
		for (u = 0; u < 4; u++) {
			destination[m].attribute[9 + u].iv[0] = space.read_dword(vertexbuffer_address[9 + u] + indexesleft[indexesleft_first] * vertexbuffer_stride[9 + u] + 0);
			destination[m].attribute[9 + u].iv[1] = space.read_dword(vertexbuffer_address[9 + u] + indexesleft[indexesleft_first] * vertexbuffer_stride[9 + u] + 4);
		}
		indexesleft_first = (indexesleft_first + 1) & 7;
		indexesleft_count--;
	}
	return (int)c;
}

/* Read vertices data from system memory. Method 0x1818 */
int nv2a_renderer::read_vertices_0x1818(address_space & space, vertex *destination, UINT32 address, int limit)
{
	UINT32 m, u, vwords;

	vwords = vertex_attribute_words[15] + vertex_attribute_offset[15];
	for (m = 0; m < limit; m++) {
		destination[m].attribute[0].iv[0] = space.read_dword(address + vertex_attribute_offset[0] * 4 + 0);
		destination[m].attribute[0].iv[1] = space.read_dword(address + vertex_attribute_offset[0] * 4 + 4);
		destination[m].attribute[0].iv[2] = space.read_dword(address + vertex_attribute_offset[0] * 4 + 8);
		destination[m].attribute[0].iv[3] = space.read_dword(address + vertex_attribute_offset[0] * 4 + 12);
		destination[m].attribute[3].iv[0] = space.read_dword(address + vertex_attribute_offset[3] * 4 + 0); // color
		for (u = 0; u < 4; u++) {
			destination[m].attribute[9 + u].iv[0] = space.read_dword(address + vertex_attribute_offset[9 + u] * 4 + 0);
			destination[m].attribute[9 + u].iv[1] = space.read_dword(address + vertex_attribute_offset[9 + u] * 4 + 4);
		}
		address = address + vwords * 4;
	}
	return (int)(m*vwords);
}

void nv2a_renderer::convert_vertices_poly(vertex *source, vertex_t *destination, int count)
{
	int m, u;

	for (m = 0; m < count; m++) {
		destination[m].x = source[m].attribute[0].fv[0];
		destination[m].y = source[m].attribute[0].fv[1];
		u = source[m].attribute[3].iv[0];
		destination[m].p[0] = u & 0xff; // b
		destination[m].p[1] = (u & 0xff00) >> 8;  // g
		destination[m].p[2] = (u & 0xff0000) >> 16;  // r
		destination[m].p[3] = (u & 0xff000000) >> 24;  // a
		for (u = 0; u < 4; u++) {
			destination[m].p[4 + u * 2] = 0;
			destination[m].p[5 + u * 2] = 0;
			if (texture[u].enabled) {
				destination[m].p[4 + u * 2] = source[m].attribute[9 + u].fv[0];
				destination[m].p[5 + u * 2] = source[m].attribute[9 + u].fv[1];
			}
		}
	}
}

void nv2a_renderer::geforce_exec_method(address_space & space,UINT32 chanel,UINT32 subchannel,UINT32 method,UINT32 address,int &countlen)
{
	UINT32 maddress;
	UINT32 data;

	maddress=method*4;
	data=space.read_dword(address);
	channel[chanel][subchannel].object.method[method]=data;
	if (maddress == 0x17fc) {
		indexesleft_count = 0;
		indexesleft_first = 0;
		primitives_count = 0;
		countlen--;
	}
	if (maddress == 0x1810) {
		// draw vertices
		int offset,count,type;
		UINT32 n;
		render_delegate renderspans;

		offset=data & 0xffffff;
		count=(data >> 24) & 0xff;
		type=channel[chanel][subchannel].object.method[0x17fc/4];
		if (((channel[chanel][subchannel].object.method[0x1e60/4] & 7) > 0) && (combiner.used != 0)) {
			renderspans=render_delegate(FUNC(nv2a_renderer::render_register_combiners),this);
		} else if (texture[0].enabled) {
			renderspans=render_delegate(FUNC(nv2a_renderer::render_texture_simple),this);
		} else
			renderspans=render_delegate(FUNC(nv2a_renderer::render_color),this);
#ifdef LOG_NV2A
		printf("vertex %d %d %d\n\r",type,offset,count);
#endif
		if (type == nv2a_renderer::QUADS) {
			for (n = 0; n <= count; n += 4) {
				vertex vert[4];
				vertex_t xy[4];

				read_vertices_0x1810(space, vert, n+offset, 4);
				convert_vertices_poly(vert, xy, 4);
				render_polygon<4>(fb.cliprect(), renderspans, 4 + 4 * 2, xy); // 4 rgba, 4 texture units 2 uv
			}
			wait();
		} else if (type == nv2a_renderer::TRIANGLE_STRIP) {
			vertex vert[4];
			vertex_t xy[4];

			read_vertices_0x1810(space, vert, offset, 2);
			convert_vertices_poly(vert, xy, 2);
			count = count - 2;
			offset = offset + 2;
			for (n = 0; n <= count; n++) {
				read_vertices_0x1810(space, vert + ((n+2) & 3), offset + n, 1);
				convert_vertices_poly(vert + ((n + 2) & 3), xy + ((n + 2) & 3), 1);
				render_triangle(fb.cliprect(), renderspans, 4 + 4 * 2, xy[((n & 1)+n) & 3], xy[((~n & 1)+n) & 3], xy[(2+n) & 3]);
			}
			wait();
		} else {
			logerror("Unsupported primitive %d for method 0x1810\n",type);
		}
		countlen--;
	}
	if (maddress == 0x1800) {
		UINT32 type, n;
		render_delegate renderspans;

		// vertices are selected from the vertex buffer using an array of indexes
		// each dword after 1800 contains two 16 bit index values to select the vartices
		type = channel[chanel][subchannel].object.method[0x17fc / 4];
		if (((channel[chanel][subchannel].object.method[0x1e60 / 4] & 7) > 0) && (combiner.used != 0)) {
			renderspans = render_delegate(FUNC(nv2a_renderer::render_register_combiners), this);
		}
		else if (texture[0].enabled) {
			renderspans = render_delegate(FUNC(nv2a_renderer::render_texture_simple), this);
		}
		else
			renderspans = render_delegate(FUNC(nv2a_renderer::render_color), this);
#ifdef LOG_NV2A
		printf("vertex %d %d %d\n\r", type, offset, count);
#endif
		if (type == nv2a_renderer::QUADS) {
			while (1) {
				vertex vert[4];
				vertex_t xy[4];
				int c;

				if ((countlen * 2 + indexesleft_count) < 4)
					break;
				c=read_vertices_0x1800(space, vert, address, 4);
				address = address + c*4;
				countlen = countlen - c;
				convert_vertices_poly(vert, xy, 4);
				render_polygon<4>(fb.cliprect(), renderspans, 4 + 4 * 2, xy); // 4 rgba, 4 texture units 2 uv
			}
			while (countlen > 0) {
				data = space.read_dword(address);
				n = (indexesleft_first + indexesleft_count) & 7;
				indexesleft[n] = data & 0xffff;
				indexesleft[(n + 1) & 7] = (data >> 16) & 0xffff;
				indexesleft_count = indexesleft_count + 2;
				address += 4;
				countlen--;
			}
			wait();
		}
		else if (type == nv2a_renderer::TRIANGLES) {
			while (1) {
				vertex vert[3];
				vertex_t xy[3];
				int c;

				if ((countlen * 2 + indexesleft_count) < 3)
					break;
				c = read_vertices_0x1800(space, vert, address, 3);
				address = address + c * 4;
				countlen = countlen - c;
				convert_vertices_poly(vert, xy, 3);
				render_triangle(fb.cliprect(), renderspans, 4 + 4 * 2, xy[0], xy[1], xy[2]); // 4 rgba, 4 texture units 2 uv
			}
			while (countlen > 0) {
				data = space.read_dword(address);
				n = (indexesleft_first + indexesleft_count) & 7;
				indexesleft[n] = data & 0xffff;
				indexesleft[(n + 1) & 7] = (data >> 16) & 0xffff;
				indexesleft_count = indexesleft_count + 2;
				address += 4;
				countlen--;
			}
			wait();
		}
		else if (type == nv2a_renderer::TRIANGLE_STRIP) {
			if ((countlen * 2 + indexesleft_count) >= 3) {
				vertex vert[4];
				vertex_t xy[4];
				int c, count;

				c = read_vertices_0x1800(space, vert, address, 2);
				convert_vertices_poly(vert, xy, 2);
				address = address + c * 4;
				countlen = countlen - c;
				count = countlen * 2 + indexesleft_count;
				for (n = 0; n < count; n++) { // <=
					c = read_vertices_0x1800(space, vert + ((n + 2) & 3), address, 1);
					address = address + c * 4;
					countlen = countlen - c;
					convert_vertices_poly(vert + ((n + 2) & 3), xy + ((n + 2) & 3), 1);
					render_triangle(fb.cliprect(), renderspans, 4 + 4 * 2, xy[((n & 1) + n) & 3], xy[((~n & 1) + n) & 3], xy[(2 + n) & 3]);
				}
			}
			while (countlen > 0) {
				data = space.read_dword(address);
				n = (indexesleft_first + indexesleft_count) & 7;
				indexesleft[n] = data & 0xffff;
				indexesleft[(n + 1) & 7] = (data >> 16) & 0xffff;
				indexesleft_count = indexesleft_count + 2;
				address += 4;
				countlen--;
			}
			wait();
		}
		else {
			logerror("Unsupported primitive %d for method 0x1800\n", type);
			countlen = 0;
		}
	}
	if (maddress == 0x1818) {
		int n;
		int type;
		render_delegate renderspans;

		if (((channel[chanel][subchannel].object.method[0x1e60/4] & 7) > 0) && (combiner.used != 0)) {
			renderspans=render_delegate(FUNC(nv2a_renderer::render_register_combiners),this);
		} else if (texture[0].enabled) {
			renderspans=render_delegate(FUNC(nv2a_renderer::render_texture_simple),this);
		} else
			renderspans=render_delegate(FUNC(nv2a_renderer::render_color),this);
		// vertices are taken from the next words, not from a vertex buffer
		// first send primitive type with 17fc
		// then countlen number of dwords with 1818
		// end with 17fc primitive type 0
		// at 1760 16 words specify the vertex format:for each possible vertex attribute the number of components (0=not present) and type of each
		type=channel[chanel][subchannel].object.method[0x17fc/4];
		if (type == nv2a_renderer::TRIANGLE_FAN) {
			vertex vert[3];
			vertex_t xy[3];
			int c;

			c=read_vertices_0x1818(space, vert, address, 2);
			convert_vertices_poly(vert, xy, 2);
			countlen = countlen - c;
			if (countlen < 0) {
				logerror("Method 0x1818 missing %d words to draw a complete primitive\n", -countlen);
				countlen = 0;
				return;
			}
			address = address + c * 4;
			for (n = 1; countlen > 0; n++) {
				c=read_vertices_0x1818(space, vert + ((n & 1) + 1), address, 1);
				countlen = countlen - c;
				if (countlen < 0) {
					logerror("Method 0x1818 missing %d words to draw a complete primitive\n", -countlen);
					countlen = 0;
					break;
				}
				address = address + c * 4;
				convert_vertices_poly(vert + ((n & 1) + 1), xy + ((n & 1) + 1), 1);
				render_triangle(fb.cliprect(), renderspans, 4 + 4 * 2, xy[0], xy[(~n & 1) + 1], xy[(n & 1) + 1]);
			}
			wait();
		} else if (type == nv2a_renderer::TRIANGLE_STRIP) {
			vertex vert[4];
			vertex_t xy[4];
			int c;

			c=read_vertices_0x1818(space, vert, address, 2);
			convert_vertices_poly(vert, xy, 2);
			countlen = countlen - c;
			if (countlen < 0) {
				logerror("Method 0x1818 missing %d words to draw a complete primitive\n", -countlen);
				countlen = 0;
				return;
			}
			address = address + c * 4;
			for (n = 0;countlen > 0; n++) {
				c=read_vertices_0x1818(space, vert + ((n + 2) & 3), address, 1);
				convert_vertices_poly(vert + ((n + 2) & 3), xy + ((n + 2) & 3), 1);
				countlen = countlen - c;
				if (countlen < 0) {
					logerror("Method 0x1818 missing %d words to draw a complete primitive\n", -countlen);
					countlen = 0;
					break;
				}
				address = address + c * 4;
				render_triangle(fb.cliprect(), renderspans, 4 + 4 * 2, xy[((n & 1) + n) & 3], xy[((~n & 1) + n) & 3], xy[(2 + n) & 3]);
			}
			wait();
		} else if (type == nv2a_renderer::QUADS) {
			while (countlen > 0) {
				vertex vert[4];
				vertex_t xy[4];
				int c;

				c = read_vertices_0x1818(space, vert, address, 4);
				convert_vertices_poly(vert, xy, 4);
				countlen = countlen - c;
				if (countlen < 0) {
					logerror("Method 0x1818 missing %d words to draw a complete primitive\n", -countlen);
					countlen = 0;
					break;
				}
				address = address + c * 4;
				render_polygon<4>(fb.cliprect(), renderspans, 4 + 4 * 2, xy); // 4 rgba, 4 texture units 2 uv
			}
			wait();
		} else if (type == nv2a_renderer::QUAD_STRIP) {
			vertex vert[4];
			vertex_t xy[4];
			int c;

			c=read_vertices_0x1818(space, vert, address, 2);
			convert_vertices_poly(vert, xy, 2);
			countlen = countlen - c;
			if (countlen < 0) {
				logerror("Method 0x1818 missing %d words to draw a complete primitive\n", -countlen);
				countlen = 0;
				return;
			}
			address = address + c * 4;
			for (n = 0; countlen > 0; n+=2) {
				c = read_vertices_0x1818(space, vert + ((n + 2) & 3), address + ((n + 2) & 3), 2);
				convert_vertices_poly(vert + ((n + 2) & 3), xy + ((n + 2) & 3), 2);
				countlen = countlen - c;
				if (countlen < 0) {
					logerror("Method 0x1818 missing %d words to draw a complete primitive\n", -countlen);
					countlen = 0;
					return;
				}
				address = address + c * 4;
				render_triangle(fb.cliprect(), renderspans, 4 + 4 * 2, xy[n & 3], xy[(n + 1) & 3], xy[(n + 2) & 3]);
				render_triangle(fb.cliprect(), renderspans, 4 + 4 * 2, xy[(n + 2) & 3], xy[(n + 1) & 3], xy[(n + 3) & 3]);
			}
			wait();
		} else {
			logerror("Unsupported primitive %d for method 0x1818\n",type);
			countlen = 0;
		}
	}
	if ((maddress >= 0x1720) && (maddress < 0x1760)) {
		int bit = method - 0x1720 / 4;

		if (data & 0x80000000)
			vertexbuffer_address[bit] = (data & 0x0fffffff) + dma_offset[1];
		else
			vertexbuffer_address[bit] = (data & 0x0fffffff) + dma_offset[0];
	}
	if ((maddress >= 0x1760) && (maddress < 0x17A0)) {
		int bit=method-0x1760/4;

		vertexbuffer_stride[bit] = (data >> 8) & 255;
		//vertexbuffer_kind[n]=tmp & 15;
		//vertexbuffer_size[n]=(tmp >> 4) & 15;
		data = data & 255;
		switch (data & 15) {
			case 0:
				vertex_attribute_words[bit]=(((data >> 4) + 3) & 15) >> 2;
				break;
			case nv2a_renderer::FLOAT:
				vertex_attribute_words[bit]=(data >> 4);
				break;
			case nv2a_renderer::UBYTE:
				vertex_attribute_words[bit]=(((data >> 4) + 3) & 15) >> 2;
				break;
			case nv2a_renderer::USHORT:
				vertex_attribute_words[bit]=(((data >> 4) + 1) & 15) >> 1;
				break;
			default:
				vertex_attribute_words[bit]=0;
		}
		if (data > 15)
			enabled_vertex_attributes |= (1 << bit);
		else
			enabled_vertex_attributes &= ~(1 << bit);
		for (int n = bit+1; n < 16; n++) {
			if ((enabled_vertex_attributes & (1 << (n - 1))) != 0)
				vertex_attribute_offset[n] = vertex_attribute_offset[n - 1] + vertex_attribute_words[n - 1];
			else
				vertex_attribute_offset[n] = vertex_attribute_offset[n - 1];
		}
		countlen--;
	}
	if ((maddress == 0x1d6c) || (maddress == 0x1d70) || (maddress == 0x1a4))
		countlen--;
	if (maddress == 0x019c) {
		geforce_read_dma_object(data, dma_offset[0], dma_size[0]);
	}
	if (maddress == 0x01a0) {
		geforce_read_dma_object(data, dma_offset[1], dma_size[1]);
	}
	if (maddress == 0x1d70) {
		// with 1d70 write the value at offest [1d6c] inside dma object [1a4]
		UINT32 offset,base;
		UINT32 dmahand,dmaoff,smasiz;

		offset=channel[chanel][subchannel].object.method[0x1d6c/4];
		dmahand=channel[chanel][subchannel].object.method[0x1a4/4];
		geforce_read_dma_object(dmahand,dmaoff,smasiz);
		base=dmaoff;
		space.write_dword(base+offset,data);
		countlen--;
	}
	if (maddress == 0x1d94) {
		// possible buffers: color, depth, stencil, and accumulation
		// clear framebuffer
		if (data & 0xf0) {
			// clear colors
			UINT32 color=channel[chanel][subchannel].object.method[0x1d90/4];
			fb.fill(color);
			//printf("clearscreen\n\r");
		}
		if (data & 0x03) {
			// clear stencil+zbuffer
		}
		countlen--;
	}
	if (maddress == 0x0300) {
		alpha_test_enabled = data != 0;
	}
	if (maddress == 0x033c) {
		alpha_func = data;
	}
	if (maddress == 0x0340) {
		alpha_reference = data;
	}
	if (maddress == 0x0304) {
		if (logical_operation_enabled)
			blending_enabled = false;
		else
			blending_enabled = data != 0;
	}
	if (maddress == 0x0344) {
		blend_function_source = data;
	}
	if (maddress == 0x0348) {
		blend_function_destination = data;
	}
	if (maddress == 0x034c) {
		blend_color = data;
	}
	if (maddress == 0x0350) {
		blend_equation = data;
	}
	if (maddress == 0x0d40) {
		if (data != 0)
			blending_enabled = false;
		else
			blending_enabled = channel[chanel][subchannel].object.method[0x0304 / 4] != 0;
		logical_operation_enabled = data != 0;
	}
	if (maddress == 0x0d44) {
		logical_operation = data;
	}
	// Texture Units
	if ((maddress >= 0x1b00) && (maddress < 0x1c00)) {
		int unit;//,off;

		unit=(maddress >> 6) & 3;
		//off=maddress & 0xc0;
		maddress=maddress & ~0xc0;
		if (maddress == 0x1b00) {
			UINT32 offset;//,base;
			//UINT32 dmahand,dmaoff,dmasiz;

			offset=data;
			texture[unit].buffer=space.get_read_ptr(offset);
			/*if (dma0 != 0) {
			    dmahand=channel[channel][subchannel].object.method[0x184/4];
			    geforce_read_dma_object(dmahand,dmaoff,smasiz);
			} else if (dma1 != 0) {
			    dmahand=channel[channel][subchannel].object.method[0x188/4];
			    geforce_read_dma_object(dmahand,dmaoff,smasiz);
			}*/
		}
		if (maddress == 0x1b04) {
			//int dma0,dma1,cubic,noborder,dims,mipmap;
			int basesizeu,basesizev,basesizew,format;

			//dma0=(data >> 0) & 1;
			//dma1=(data >> 1) & 1;
			//cubic=(data >> 2) & 1;
			//noborder=(data >> 3) & 1;
			//dims=(data >> 4) & 15;
			//mipmap=(data >> 19) & 1;
			format=(data >> 8) & 255;
			basesizeu=(data >> 20) & 15;
			basesizev=(data >> 24) & 15;
			basesizew=(data >> 28) & 15;
			texture[unit].sizeu=1 << basesizeu;
			texture[unit].sizev=1 << basesizev;
			texture[unit].sizew=1 << basesizew;
			texture[unit].dilate=dilatechose[(basesizeu << 4)+basesizev];
			texture[unit].format=format;
			if (debug_grab_texttype == format) {
				FILE *f;
				int written;

				debug_grab_texttype = -1;
				f = fopen(debug_grab_textfile, "wb");
				if (f) {
					written=(int)fwrite(texture[unit].buffer, texture[unit].sizeu*texture[unit].sizev*4, 1, f);
					fclose(f);
					logerror("Written %d bytes of texture to specified file\n", written);
				} else
					logerror("Unable to save texture to specified file\n");
			}
		}
		if (maddress == 0x1b0c) {
			// enable texture
			int enable;

			enable=(data >> 30) & 1;
			texture[unit].enabled=enable;
		}
		if (maddress == 0x1b10) {
			texture[unit].rectangle_pitch=data >> 16;
		}
		countlen--;
	}
	// modelview matrix
	if ((maddress >= 0x0480) && (maddress < 0x04c0)) {
		maddress = (maddress - 0x0480) / 4;
		*(UINT32 *)(&matrix.modelview[maddress]) = data;
		countlen--;
	}
	// inverse modelview matrix
	if ((maddress >= 0x0580) && (maddress < 0x05c0)) {
		maddress = (maddress - 0x0580) / 4;
		*(UINT32 *)(&matrix.modelview_inverse[maddress]) = data;
		countlen--;
	}
	// projection matrix
	if ((maddress >= 0x0680) && (maddress < 0x06c0)) {
		maddress = (maddress - 0x0680) / 4;
		*(UINT32 *)(&matrix.projection[maddress]) = data;
		countlen--;
	}
	// viewport translate
	if ((maddress >= 0x0a20) && (maddress < 0x0a30)) {
		maddress = (maddress - 0x0a20) / 4;
		*(UINT32 *)(&matrix.translate[maddress]) = data;
		countlen--;
	}
	// viewport scale
	if ((maddress >= 0x0af0) && (maddress < 0x0b00)) {
		maddress = (maddress - 0x0af0) / 4;
		*(UINT32 *)(&matrix.scale[maddress]) = data;
		countlen--;
	}
	// Vertex program (shader)
	if (maddress == 0x1e94) {
		/*if (data == 2)
		    logerror("Enabled vertex program\n");
		else if (data == 4)
		    logerror("Enabled fixed function pipeline\n");
		else if (data == 6)
		    logerror("Enabled both fixed function pipeline and vertex program ?\n");
		else
		    logerror("Unknown value %d to method 0x1e94\n",data);*/
		vertex_pipeline = data & 6;
		countlen--;
	}
	if (maddress == 0x1e9c) {
		//logerror("VP_UPLOAD_FROM_ID %d\n",data);
		vertexprogram.upload_instruction=data*4;
		countlen--;
	}
	if (maddress == 0x1ea0) {
		//logerror("VP_START_FROM_ID %d\n",data);
		vertexprogram.instructions=vertexprogram.upload_instruction/4;
		vertexprogram.start_instruction = data * 4;
		countlen--;
	}
	if (maddress == 0x1ea4) {
		//logerror("VP_UPLOAD_CONST_ID %d\n",data);
		vertexprogram.upload_parameter=data*4;
		countlen--;
	}
	if ((maddress >= 0x0b00) && (maddress < 0x0b80)) {
		//logerror("VP_UPLOAD_INST\n");
		if (vertexprogram.upload_instruction < 1024)
			vertexprogram.instruction[vertexprogram.upload_instruction]=data;
		else
			logerror("Need to increase size of vertexprogram.instruction to %d\n\r", vertexprogram.upload_parameter);
		vertexprogram.upload_instruction++;
	}
	if ((maddress >= 0x0b80) && (maddress < 0x0c00)) {
		//logerror("VP_UPLOAD_CONST\n");
		if (vertexprogram.upload_parameter < 1024)
			*(UINT32 *)(&vertexprogram.parameter[vertexprogram.upload_parameter]) = data;
		else
			logerror("Need to increase size of vertexprogram.parameter to %d\n\r", vertexprogram.upload_parameter);
		vertexprogram.upload_parameter++;
	}
	// Register combiners
	if (maddress == 0x1e60) {
		combiner.stages=data & 15;
		countlen--;
	}
	if (maddress == 0x0288) {
		combiner.final.mapin_rgbD_input=data & 15;
		combiner.final.mapin_rgbD_component=(data >> 4) & 1;
		combiner.final.mapin_rgbD_mapping=(data >> 5) & 7;
		combiner.final.mapin_rgbC_input=(data >> 8) & 15;
		combiner.final.mapin_rgbC_component=(data >> 12) & 1;
		combiner.final.mapin_rgbC_mapping=(data >> 13) & 7;
		combiner.final.mapin_rgbB_input=(data >> 16) & 15;
		combiner.final.mapin_rgbB_component=(data >> 20) & 1;
		combiner.final.mapin_rgbB_mapping=(data >> 21) & 7;
		combiner.final.mapin_rgbA_input=(data >> 24) & 15;
		combiner.final.mapin_rgbA_component=(data >> 28) & 1;
		combiner.final.mapin_rgbA_mapping=(data >> 29) & 7;
		countlen--;
	}
	if (maddress == 0x028c) {
		combiner.final.color_sum_clamp=(data >> 7) & 1;
		combiner.final.mapin_aG_input=(data >> 8) & 15;
		combiner.final.mapin_aG_component=(data >> 12) & 1;
		combiner.final.mapin_aG_mapping=(data >> 13) & 7;
		combiner.final.mapin_rgbF_input=(data >> 16) & 15;
		combiner.final.mapin_rgbF_component=(data >> 20) & 1;
		combiner.final.mapin_rgbF_mapping=(data >> 21) & 7;
		combiner.final.mapin_rgbE_input=(data >> 24) & 15;
		combiner.final.mapin_rgbE_component=(data >> 28) & 1;
		combiner.final.mapin_rgbE_mapping=(data >> 29) & 7;
		countlen--;
	}
	if (maddress == 0x1e20) {
		combiner_argb8_float(data,combiner.final.register_constantcolor0);
		countlen--;
	}
	if (maddress == 0x1e24) {
		combiner_argb8_float(data,combiner.final.register_constantcolor1);
		countlen--;
	}
	if ((maddress >= 0x0260) && (maddress < 0x0280)) {
		int n;

		n=(maddress-0x0260) >> 2;
		combiner.stage[n].mapin_aD_input=data & 15;
		combiner.stage[n].mapin_aD_component=(data >> 4) & 1;
		combiner.stage[n].mapin_aD_mapping=(data >> 5) & 7;
		combiner.stage[n].mapin_aC_input=(data >> 8) & 15;
		combiner.stage[n].mapin_aC_component=(data >> 12) & 1;
		combiner.stage[n].mapin_aC_mapping=(data >> 13) & 7;
		combiner.stage[n].mapin_aB_input=(data >> 16) & 15;
		combiner.stage[n].mapin_aB_component=(data >> 20) & 1;
		combiner.stage[n].mapin_aB_mapping=(data >> 21) & 7;
		combiner.stage[n].mapin_aA_input=(data >> 24) & 15;
		combiner.stage[n].mapin_aA_component=(data >> 28) & 1;
		combiner.stage[n].mapin_aA_mapping=(data >> 29) & 7;
		countlen--;
	}
	if ((maddress >= 0x0ac0) && (maddress < 0x0ae0)) {
		int n;

		n=(maddress-0x0ac0) >> 2;
		combiner.stage[n].mapin_rgbD_input=data & 15;
		combiner.stage[n].mapin_rgbD_component=(data >> 4) & 1;
		combiner.stage[n].mapin_rgbD_mapping=(data >> 5) & 7;
		combiner.stage[n].mapin_rgbC_input=(data >> 8) & 15;
		combiner.stage[n].mapin_rgbC_component=(data >> 12) & 1;
		combiner.stage[n].mapin_rgbC_mapping=(data >> 13) & 7;
		combiner.stage[n].mapin_rgbB_input=(data >> 16) & 15;
		combiner.stage[n].mapin_rgbB_component=(data >> 20) & 1;
		combiner.stage[n].mapin_rgbB_mapping=(data >> 21) & 7;
		combiner.stage[n].mapin_rgbA_input=(data >> 24) & 15;
		combiner.stage[n].mapin_rgbA_component=(data >> 28) & 1;
		combiner.stage[n].mapin_rgbA_mapping=(data >> 29) & 7;
		countlen--;
	}
	if ((maddress >= 0x0a60) && (maddress < 0x0a80)) {
		int n;

		n=(maddress-0x0a60) >> 2;
		combiner_argb8_float(data,combiner.stage[n].register_constantcolor0);
		countlen--;
	}
	if ((maddress >= 0x0a80) && (maddress < 0x0aa0)) {
		int n;

		n=(maddress-0x0a80) >> 2;
		combiner_argb8_float(data,combiner.stage[n].register_constantcolor1);
		countlen--;
	}
	if ((maddress >= 0x0aa0) && (maddress < 0x0ac0)) {
		int n;

		n=(maddress-0x0aa0) >> 2;
		combiner.stage[n].mapout_aCD_output=data & 15;
		combiner.stage[n].mapout_aAB_output=(data >> 4) & 15;
		combiner.stage[n].mapout_aSUM_output=(data >> 8) & 15;
		combiner.stage[n].mapout_aCD_dotproduct=(data >> 12) & 1;
		combiner.stage[n].mapout_aAB_dotproduct=(data >> 13) & 1;
		combiner.stage[n].mapout_a_muxsum=(data >> 14) & 1;
		combiner.stage[n].mapout_a_bias=(data >> 15) & 1;
		combiner.stage[n].mapout_a_scale=(data >> 16) & 3;
		//combiner.=(data >> 27) & 7;
		countlen--;
	}
	if ((maddress >= 0x1e40) && (maddress < 0x1e60)) {
		int n;

		n=(maddress-0x1e40) >> 2;
		combiner.stage[n].mapout_rgbCD_output=data & 15;
		combiner.stage[n].mapout_rgbAB_output=(data >> 4) & 15;
		combiner.stage[n].mapout_rgbSUM_output=(data >> 8) & 15;
		combiner.stage[n].mapout_rgbCD_dotproduct=(data >> 12) & 1;
		combiner.stage[n].mapout_rgbAB_dotproduct=(data >> 13) & 1;
		combiner.stage[n].mapout_rgb_muxsum=(data >> 14) & 1;
		combiner.stage[n].mapout_rgb_bias=(data >> 15) & 1;
		combiner.stage[n].mapout_rgb_scale=(data >> 16) & 3;
		//combiner.=(data >> 27) & 7;
		countlen--;
	}
}

int nv2a_renderer::toggle_register_combiners_usage()
{
	combiner.used=1-combiner.used;
	return combiner.used;
}

void nv2a_renderer::debug_grab_texture(int type, const char *filename)
{
	debug_grab_texttype = type;
	if (debug_grab_textfile == NULL)
		debug_grab_textfile = (char *)malloc(128);
	strncpy(debug_grab_textfile, filename, 127);
}

void nv2a_renderer::debug_grab_vertex_program_slot(int slot, UINT32 *instruction)
{
	if (slot >= 1024 / 4)
		return;
	instruction[0] = vertexprogram.instruction[slot * 4 + 0];
	instruction[1] = vertexprogram.instruction[slot * 4 + 1];
	instruction[2] = vertexprogram.instruction[slot * 4 + 2];
	instruction[3] = vertexprogram.instruction[slot * 4 + 3];
}

void nv2a_renderer::savestate_items()
{
}

void nv2a_renderer::combiner_argb8_float(UINT32 color,float reg[4])
{
	reg[0]=(float)(color & 0xff)/255.0;
	reg[1]=(float)((color >> 8) & 0xff)/255.0;
	reg[2]=(float)((color >> 16) & 0xff)/255.0;
	reg[3]=(float)((color >> 24) & 0xff)/255.0;
}

UINT32 nv2a_renderer::combiner_float_argb8(float reg[4])
{
	UINT32 r,g,b,a;

	a=reg[3]*255.0;
	b=reg[2]*255.0;
	g=reg[1]*255.0;
	r=reg[0]*255.0;
	return (a << 24) | (r << 16) | (g << 8) | b;
}

float nv2a_renderer::combiner_map_input_select(int code,int index)
{
	switch (code) {
		case 0:
		default:
			return combiner.register_zero[index];
		case 1:
			return combiner.register_color0[index];
		case 2:
			return combiner.register_color1[index];
		case 3:
			return combiner.register_fogcolor[index];
		case 4:
			return combiner.register_primarycolor[index];
		case 5:
			return combiner.register_secondarycolor[index];
		case 8:
			return combiner.register_texture0color[index];
		case 9:
			return combiner.register_texture1color[index];
		case 10:
			return combiner.register_texture2color[index];
		case 11:
			return combiner.register_texture3color[index];
		case 12:
			return combiner.register_spare0[index];
		case 13:
			return combiner.register_spare1[index];
		case 14:
			return combiner.variable_sumclamp[index];
		case 15:
			return combiner.variable_EF[index];
	}

	// never executed
	//return 0;
}

float *nv2a_renderer::combiner_map_input_select3(int code)
{
	switch (code) {
		case 0:
		default:
			return combiner.register_zero;
		case 1:
			return combiner.register_color0;
		case 2:
			return combiner.register_color1;
		case 3:
			return combiner.register_fogcolor;
		case 4:
			return combiner.register_primarycolor;
		case 5:
			return combiner.register_secondarycolor;
		case 8:
			return combiner.register_texture0color;
		case 9:
			return combiner.register_texture1color;
		case 10:
			return combiner.register_texture2color;
		case 11:
			return combiner.register_texture3color;
		case 12:
			return combiner.register_spare0;
		case 13:
			return combiner.register_spare1;
		case 14:
			return combiner.variable_sumclamp;
		case 15:
			return combiner.variable_EF;
	}

	// never executed
	//return 0;
}

float *nv2a_renderer::combiner_map_output_select3(int code)
{
	switch (code) {
		case 0:
			return 0;
		case 1:
			return 0;
		case 2:
			return 0;
		case 3:
			return 0;
		case 4:
			return combiner.register_primarycolor;
		case 5:
			return combiner.register_secondarycolor;
		case 8:
			return combiner.register_texture0color;
		case 9:
			return combiner.register_texture1color;
		case 10:
			return combiner.register_texture2color;
		case 11:
			return combiner.register_texture3color;
		case 12:
			return combiner.register_spare0;
		case 13:
			return combiner.register_spare1;
		case 14:
			return 0;
		case 15:
		default:
			return 0;
	}
}

float nv2a_renderer::combiner_map_input_function(int code,float value)
{
	float t;

	switch (code) {
		case 0:
			return MAX(0.0,value);
		case 1:
			t=MAX(value, 0.0);
			return 1.0 - MIN(t, 1.0);
		case 2:
			return 2.0 * MAX(0.0, value) - 1.0;
		case 3:
			return -2.0 * MAX(0.0, value) + 1.0;
		case 4:
			return MAX(0.0, value) - 0.5;
		case 5:
			return -MAX(0.0, value) + 0.5;
		case 6:
			return value;
		case 7:
		default:
			return -value;
	}

	// never executed
	//return 0;
}

void nv2a_renderer::combiner_map_input_function3(int code,float *data)
{
	float t;

	switch (code) {
		case 0:
			data[0]=MAX(0.0,data[0]);
			data[1]=MAX(0.0,data[1]);
			data[2]=MAX(0.0,data[2]);
		break;
		case 1:
			t=MAX(data[0], 0.0);
			data[0]=1.0 - MIN(t, 1.0);
			t=MAX(data[1], 0.0);
			data[1]=1.0 - MIN(t, 1.0);
			t=MAX(data[2], 0.0);
			data[2]=1.0 - MIN(t, 1.0);
		break;
		case 2:
			data[0]=2.0 * MAX(0.0, data[0]) - 1.0;
			data[1]=2.0 * MAX(0.0, data[1]) - 1.0;
			data[2]=2.0 * MAX(0.0, data[2]) - 1.0;
		break;
		case 3:
			data[0]=-2.0 * MAX(0.0, data[0]) + 1.0;
			data[1]=-2.0 * MAX(0.0, data[1]) + 1.0;
			data[2]=-2.0 * MAX(0.0, data[2]) + 1.0;
		break;
		case 4:
			data[0]=MAX(0.0, data[0]) - 0.5;
			data[1]=MAX(0.0, data[1]) - 0.5;
			data[2]=MAX(0.0, data[2]) - 0.5;
		break;
		case 5:
			data[0]=-MAX(0.0, data[0]) + 0.5;
			data[1]=-MAX(0.0, data[1]) + 0.5;
			data[2]=-MAX(0.0, data[2]) + 0.5;
		break;
		case 6:
			return;
		case 7:
		default:
			data[0]=-data[0];
			data[1]=-data[1];
			data[2]=-data[2];
		break;
	}
}

void nv2a_renderer::combiner_initialize_registers(UINT32 argb8[6])
{
	combiner_argb8_float(argb8[0],combiner.register_primarycolor);
	combiner_argb8_float(argb8[1],combiner.register_secondarycolor);
	combiner_argb8_float(argb8[2],combiner.register_texture0color);
	combiner_argb8_float(argb8[3],combiner.register_texture1color);
	combiner_argb8_float(argb8[4],combiner.register_texture2color);
	combiner_argb8_float(argb8[5],combiner.register_texture3color);
	combiner.register_spare0[3]=combiner.register_texture0color[3];
	combiner.register_zero[0]=combiner.register_zero[1]=combiner.register_zero[2]=combiner.register_zero[3]=0;
}

void nv2a_renderer::combiner_initialize_stage(int stage_number)
{
	int n=stage_number;

	// put register_constantcolor0 in register_color0
	combiner.register_color0[0]=combiner.stage[n].register_constantcolor0[0];
	combiner.register_color0[1]=combiner.stage[n].register_constantcolor0[1];
	combiner.register_color0[2]=combiner.stage[n].register_constantcolor0[2];
	combiner.register_color0[3]=combiner.stage[n].register_constantcolor0[3];
	// put register_constantcolor1 in register_color1
	combiner.register_color1[0]=combiner.stage[n].register_constantcolor1[0];
	combiner.register_color1[1]=combiner.stage[n].register_constantcolor1[1];
	combiner.register_color1[2]=combiner.stage[n].register_constantcolor1[2];
	combiner.register_color1[3]=combiner.stage[n].register_constantcolor1[3];
}

void nv2a_renderer::combiner_initialize_final()
{
	// put register_constantcolor0 in register_color0
	combiner.register_color0[0]=combiner.final.register_constantcolor0[0];
	combiner.register_color0[1]=combiner.final.register_constantcolor0[1];
	combiner.register_color0[2]=combiner.final.register_constantcolor0[2];
	combiner.register_color0[3]=combiner.final.register_constantcolor0[3];
	// put register_constantcolor1 in register_color1
	combiner.register_color1[0]=combiner.final.register_constantcolor1[0];
	combiner.register_color1[1]=combiner.final.register_constantcolor1[1];
	combiner.register_color1[2]=combiner.final.register_constantcolor1[2];
	combiner.register_color1[3]=combiner.final.register_constantcolor1[3];
}

void nv2a_renderer::combiner_map_input(int stage_number)
{
	int n=stage_number;
	int c,d,i;
	float v,*pv;

	// A
	v=combiner_map_input_select(combiner.stage[n].mapin_aA_input,2+combiner.stage[n].mapin_aA_component);
	combiner.variable_A[3]=combiner_map_input_function(combiner.stage[n].mapin_aA_mapping,v);
	// B
	v=combiner_map_input_select(combiner.stage[n].mapin_aB_input,2+combiner.stage[n].mapin_aB_component);
	combiner.variable_B[3]=combiner_map_input_function(combiner.stage[n].mapin_aB_mapping,v);
	// C
	v=combiner_map_input_select(combiner.stage[n].mapin_aC_input,2+combiner.stage[n].mapin_aC_component);
	combiner.variable_C[3]=combiner_map_input_function(combiner.stage[n].mapin_aC_mapping,v);
	// D
	v=combiner_map_input_select(combiner.stage[n].mapin_aD_input,2+combiner.stage[n].mapin_aD_component);
	combiner.variable_D[3]=combiner_map_input_function(combiner.stage[n].mapin_aD_mapping,v);

	// A
	pv=combiner_map_input_select3(combiner.stage[n].mapin_rgbA_input);
	c=combiner.stage[n].mapin_rgbA_component*3;
	i=~combiner.stage[n].mapin_rgbA_component & 1;
	for (d=0;d < 3;d++) {
		combiner.variable_A[d]=pv[c];
		c=c+i;
	}
	combiner_map_input_function3(combiner.stage[n].mapin_rgbA_mapping,combiner.variable_A);
	// B
	pv=combiner_map_input_select3(combiner.stage[n].mapin_rgbB_input);
	c=combiner.stage[n].mapin_rgbB_component*3;
	i=~combiner.stage[n].mapin_rgbB_component & 1;
	for (d=0;d < 3;d++) {
		combiner.variable_B[d]=pv[c];
		c=c+i;
	}
	combiner_map_input_function3(combiner.stage[n].mapin_rgbB_mapping,combiner.variable_B);
	// C
	pv=combiner_map_input_select3(combiner.stage[n].mapin_rgbC_input);
	c=combiner.stage[n].mapin_rgbC_component*3;
	i=~combiner.stage[n].mapin_rgbC_component & 1;
	for (d=0;d < 3;d++) {
		combiner.variable_C[d]=pv[c];
		c=c+i;
	}
	combiner_map_input_function3(combiner.stage[n].mapin_rgbC_mapping,combiner.variable_C);
	// D
	pv=combiner_map_input_select3(combiner.stage[n].mapin_rgbD_input);
	c=combiner.stage[n].mapin_rgbD_component*3;
	i=~combiner.stage[n].mapin_rgbD_component & 1;
	for (d=0;d < 3;d++) {
		combiner.variable_D[d]=pv[c];
		c=c+i;
	}
	combiner_map_input_function3(combiner.stage[n].mapin_rgbD_mapping,combiner.variable_D);
}

void nv2a_renderer::combiner_map_output(int stage_number)
{
	int n=stage_number;
	float *f;

	// rgb
	f=combiner_map_output_select3(combiner.stage[n].mapout_rgbAB_output);
	if (f) {
		f[0]=combiner.function_RGBop1[0];
		f[1]=combiner.function_RGBop1[1];
		f[2]=combiner.function_RGBop1[2];
	}
	f=combiner_map_output_select3(combiner.stage[n].mapout_rgbCD_output);
	if (f) {
		f[0]=combiner.function_RGBop2[0];
		f[1]=combiner.function_RGBop2[1];
		f[2]=combiner.function_RGBop2[2];
	}
	if ((combiner.stage[n].mapout_rgbAB_dotproduct | combiner.stage[n].mapout_rgbCD_dotproduct) == 0) {
		f=combiner_map_output_select3(combiner.stage[n].mapout_rgbSUM_output);
		if (f) {
			f[0]=combiner.function_RGBop3[0];
			f[1]=combiner.function_RGBop3[1];
			f[2]=combiner.function_RGBop3[2];
		}
	}
	// a
	f=combiner_map_output_select3(combiner.stage[n].mapout_aAB_output);
	if (f)
		f[3]=combiner.function_Aop1;
	f=combiner_map_output_select3(combiner.stage[n].mapout_aCD_output);
	if (f)
		f[3]=combiner.function_Aop2;
	f=combiner_map_output_select3(combiner.stage[n].mapout_aSUM_output);
	if (f)
		f[3]=combiner.function_Aop3;
}

void nv2a_renderer::combiner_map_final_input()
{
	int i,c,d;
	float *pv;

	// E
	pv=combiner_map_input_select3(combiner.final.mapin_rgbE_input);
	c=combiner.final.mapin_rgbE_component*3;
	i=~combiner.final.mapin_rgbE_component & 1;
	for (d=0;d < 3;d++) {
		combiner.variable_E[d]=pv[c];
		c=c+i;
	}
	combiner_map_input_function3(combiner.final.mapin_rgbE_mapping,combiner.variable_E);
	// F
	pv=combiner_map_input_select3(combiner.final.mapin_rgbF_input);
	c=combiner.final.mapin_rgbF_component*3;
	i=~combiner.final.mapin_rgbF_component & 1;
	for (d=0;d < 3;d++) {
		combiner.variable_F[d]=pv[c];
		c=c+i;
	}
	combiner_map_input_function3(combiner.final.mapin_rgbF_mapping,combiner.variable_F);
	// EF
	combiner.variable_EF[0]=combiner.variable_E[0]*combiner.variable_F[0];
	combiner.variable_EF[1]=combiner.variable_E[1]*combiner.variable_F[1];
	combiner.variable_EF[2]=combiner.variable_E[2]*combiner.variable_F[2];
	// sumclamp
	combiner.variable_sumclamp[0]=MAX(0,combiner.register_spare0[0])+MAX(0,combiner.register_secondarycolor[0]);
	combiner.variable_sumclamp[1]=MAX(0,combiner.register_spare0[1])+MAX(0,combiner.register_secondarycolor[1]);
	combiner.variable_sumclamp[2]=MAX(0,combiner.register_spare0[2])+MAX(0,combiner.register_secondarycolor[2]);
	if (combiner.final.color_sum_clamp != 0) {
		combiner.variable_sumclamp[0]=MIN(combiner.variable_sumclamp[0],1.0);
		combiner.variable_sumclamp[1]=MIN(combiner.variable_sumclamp[1],1.0);
		combiner.variable_sumclamp[2]=MIN(combiner.variable_sumclamp[2],1.0);
	}
	// A
	pv=combiner_map_input_select3(combiner.final.mapin_rgbA_input);
	c=combiner.final.mapin_rgbA_component*3;
	i=~combiner.final.mapin_rgbA_component & 1;
	for (d=0;d < 3;d++) {
		combiner.variable_A[d]=pv[c];
		c=c+i;
	}
	combiner_map_input_function3(combiner.final.mapin_rgbA_mapping,combiner.variable_A);
	// B
	pv=combiner_map_input_select3(combiner.final.mapin_rgbB_input);
	c=combiner.final.mapin_rgbB_component*3;
	i=~combiner.final.mapin_rgbB_component & 1;
	for (d=0;d < 3;d++) {
		combiner.variable_B[d]=pv[c];
		c=c+i;
	}
	combiner_map_input_function3(combiner.final.mapin_rgbB_mapping,combiner.variable_B);
	// C
	pv=combiner_map_input_select3(combiner.final.mapin_rgbC_input);
	c=combiner.final.mapin_rgbC_component*3;
	i=~combiner.final.mapin_rgbC_component & 1;
	for (d=0;d < 3;d++) {
		combiner.variable_C[d]=pv[c];
		c=c+i;
	}
	combiner_map_input_function3(combiner.final.mapin_rgbC_mapping,combiner.variable_C);
	// D
	pv=combiner_map_input_select3(combiner.final.mapin_rgbD_input);
	c=combiner.final.mapin_rgbD_component*3;
	i=~combiner.final.mapin_rgbD_component & 1;
	for (d=0;d < 3;d++) {
		combiner.variable_D[d]=pv[c];
		c=c+i;
	}
	combiner_map_input_function3(combiner.final.mapin_rgbD_mapping,combiner.variable_D);
	// G
	combiner.variable_G=combiner_map_input_select(combiner.final.mapin_aG_input,2+combiner.final.mapin_aG_component);
}

void nv2a_renderer::combiner_final_output()
{
	// rgb
	combiner.output[0]=combiner.variable_A[0]*combiner.variable_B[0]+(1.0-combiner.variable_A[0])*combiner.variable_C[0]+combiner.variable_D[0];
	combiner.output[1]=combiner.variable_A[1]*combiner.variable_B[1]+(1.0-combiner.variable_A[1])*combiner.variable_C[1]+combiner.variable_D[1];
	combiner.output[2]=combiner.variable_A[2]*combiner.variable_B[2]+(1.0-combiner.variable_A[2])*combiner.variable_C[2]+combiner.variable_D[2];
	combiner.output[0]=MIN(combiner.output[0],1.0);
	combiner.output[1]=MIN(combiner.output[1],1.0);
	combiner.output[2]=MIN(combiner.output[2],1.0);
	// a
	combiner.output[3]=combiner_map_input_function(combiner.final.mapin_aG_mapping,combiner.variable_G);
}

void nv2a_renderer::combiner_function_AB(float result[4])
{
	result[0]=combiner.variable_A[0]*combiner.variable_B[0];
	result[1]=combiner.variable_A[1]*combiner.variable_B[1];
	result[2]=combiner.variable_A[2]*combiner.variable_B[2];
}

void nv2a_renderer::combiner_function_AdotB(float result[4])
{
	result[0]=combiner.variable_A[0]*combiner.variable_B[0]+combiner.variable_A[1]*combiner.variable_B[1]+combiner.variable_A[2]*combiner.variable_B[2];
	result[1]=result[0];
	result[2]=result[0];
}

void nv2a_renderer::combiner_function_CD(float result[4])
{
	result[0]=combiner.variable_C[0]*combiner.variable_D[0];
	result[1]=combiner.variable_C[1]*combiner.variable_D[1];
	result[2]=combiner.variable_C[2]*combiner.variable_D[2];
}

void nv2a_renderer::combiner_function_CdotD(float result[4])
{
	result[0]=combiner.variable_C[0]*combiner.variable_D[0]+combiner.variable_C[1]*combiner.variable_D[1]+combiner.variable_C[2]*combiner.variable_D[2];
	result[1]=result[0];
	result[2]=result[0];
}

void nv2a_renderer::combiner_function_ABmuxCD(float result[4])
{
	if (combiner.register_spare0[3] >= 0.5)
		combiner_function_AB(result);
	else
		combiner_function_CD(result);
}

void nv2a_renderer::combiner_function_ABsumCD(float result[4])
{
	result[0]=combiner.variable_A[0]*combiner.variable_B[0]+combiner.variable_C[0]*combiner.variable_D[0];
	result[1]=combiner.variable_A[1]*combiner.variable_B[1]+combiner.variable_C[1]*combiner.variable_D[1];
	result[2]=combiner.variable_A[2]*combiner.variable_B[2]+combiner.variable_C[2]*combiner.variable_D[2];
}

void nv2a_renderer::combiner_compute_rgb_outputs(int stage_number)
{
	int n=stage_number;
	int m;
	float biasrgb,scalergb;

	if (combiner.stage[n].mapout_rgb_bias)
		biasrgb= -0.5;
	else
		biasrgb=0;
	switch (combiner.stage[n].mapout_rgb_scale) {
		case 0:
		default:
			scalergb=1.0;
		break;
		case 1:
			scalergb=2.0;
		break;
		case 2:
			scalergb=4.0;
		break;
		case 3:
			scalergb=0.5;
		break;
	}
	if (combiner.stage[n].mapout_rgbAB_dotproduct) {
		m=1;
		combiner_function_AdotB(combiner.function_RGBop1);
	} else {
		m=0;
		combiner_function_AB(combiner.function_RGBop1);
		}
	combiner.function_RGBop1[0]=MAX(MIN((combiner.function_RGBop1[0] + biasrgb) * scalergb, 1.0), -1.0);
	combiner.function_RGBop1[1]=MAX(MIN((combiner.function_RGBop1[1] + biasrgb) * scalergb, 1.0), -1.0);
	combiner.function_RGBop1[2]=MAX(MIN((combiner.function_RGBop1[2] + biasrgb) * scalergb, 1.0), -1.0);
	if (combiner.stage[n].mapout_rgbCD_dotproduct) {
		m=m | 1;
		combiner_function_CdotD(combiner.function_RGBop2);
	} else
		combiner_function_CD(combiner.function_RGBop2);
	combiner.function_RGBop2[0]=MAX(MIN((combiner.function_RGBop2[0] + biasrgb) * scalergb, 1.0), -1.0);
	combiner.function_RGBop2[1]=MAX(MIN((combiner.function_RGBop2[1] + biasrgb) * scalergb, 1.0), -1.0);
	combiner.function_RGBop2[2]=MAX(MIN((combiner.function_RGBop2[2] + biasrgb) * scalergb, 1.0), -1.0);
	if (m == 0) {
		if (combiner.stage[n].mapout_rgb_muxsum)
			combiner_function_ABmuxCD(combiner.function_RGBop3);
		else
			combiner_function_ABsumCD(combiner.function_RGBop3);
		combiner.function_RGBop3[0]=MAX(MIN((combiner.function_RGBop3[0] + biasrgb) * scalergb, 1.0), -1.0);
		combiner.function_RGBop3[1]=MAX(MIN((combiner.function_RGBop3[1] + biasrgb) * scalergb, 1.0), -1.0);
		combiner.function_RGBop3[2]=MAX(MIN((combiner.function_RGBop3[2] + biasrgb) * scalergb, 1.0), -1.0);
	}
}

void nv2a_renderer::combiner_compute_a_outputs(int stage_number)
{
	int n=stage_number;
	float biasa,scalea;

	if (combiner.stage[n].mapout_a_bias)
		biasa= -0.5;
	else
		biasa=0;
	switch (combiner.stage[n].mapout_a_scale) {
		case 0:
		default:
			scalea=1.0;
		break;
		case 1:
			scalea=2.0;
		break;
		case 2:
			scalea=4.0;
		break;
		case 3:
			scalea=0.5;
		break;
	}
	combiner.function_Aop1=combiner.variable_A[3]*combiner.variable_B[3];
	combiner.function_Aop1=MAX(MIN((combiner.function_Aop1 + biasa) * scalea, 1.0), -1.0);
	combiner.function_Aop2=combiner.variable_C[3]*combiner.variable_D[3];
	combiner.function_Aop2=MAX(MIN((combiner.function_Aop2 + biasa) * scalea, 1.0), -1.0);
	if (combiner.stage[n].mapout_a_muxsum) {
		if (combiner.register_spare0[3] >= 0.5)
			combiner.function_Aop3=combiner.variable_A[3]*combiner.variable_B[3];
		else
			combiner.function_Aop3=combiner.variable_C[3]*combiner.variable_D[3];
	} else
		combiner.function_Aop3=combiner.variable_A[3]*combiner.variable_B[3]+combiner.variable_C[3]*combiner.variable_D[3];
	combiner.function_Aop3=MAX(MIN((combiner.function_Aop3 + biasa) * scalea, 1.0), -1.0);
}

void nv2a_renderer::vblank_callback(screen_device &screen, bool state)
{
	chihiro_state *chst=machine().driver_data<chihiro_state>();

	//printf("vblank_callback\n\r");
	if (state == true)
		pcrtc[0x100/4] |= 1;
	else
		pcrtc[0x100/4] &= ~1;
	if (pcrtc[0x100/4] & pcrtc[0x140/4])
		pmc[0x100/4] |= 0x1000000;
	else
		pmc[0x100/4] &= ~0x1000000;
	if ((pmc[0x100/4] != 0) && (pmc[0x140/4] != 0)) {
		// send interrupt
		chst->chihiro_devs.pic8259_1->ir3_w(1); // IRQ 3
	} else
		chst->chihiro_devs.pic8259_1->ir3_w(0); // IRQ 3
}

UINT32 nv2a_renderer::screen_update_callback(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 *dst=(UINT32 *)bitmap.raw_pixptr(0,0);
	UINT32 *src=(UINT32 *)fb.raw_pixptr(0,0);

	//printf("updatescreen\n\r");
	memcpy(dst,src,bitmap.rowbytes()*bitmap.height());
	return 0;
}

void chihiro_state::debug_generate_irq(int irq,bool active)
{
	int state;

	if (active)
	{
		debug_irq_active=true;
		debug_irq_number=irq;
		state=1;
	}
	else
	{
		debug_irq_active=false;
		state=0;
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
	nvidia_nv2a->vblank_callback(screen,state);
}

UINT32 chihiro_state::screen_update_callback(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return nvidia_nv2a->screen_update_callback(screen, bitmap, cliprect);
}

READ32_MEMBER( nv2a_renderer::geforce_r )
{
static int x,ret;

	ret=0;
	if (offset == 0x1804f6) {
		x = x ^ 0x08080808;
		ret=x;
	}
	if ((offset >= 0x00101000/4) && (offset < 0x00102000/4)) {
		//logerror("NV_2A: read STRAPS[%06X] mask %08X value %08X\n",offset*4-0x00101000,mem_mask,ret);
	} else if ((offset >= 0x00002000/4) && (offset < 0x00004000/4)) {
		ret=pfifo[offset-0x00002000/4];
		// PFIFO.CACHE1.STATUS or PFIFO.RUNOUT_STATUS
		if ((offset == 0x3214/4) || (offset == 0x2400/4))
			ret=0x10;
		//logerror("NV_2A: read PFIFO[%06X] value %08X\n",offset*4-0x00002000,ret);
	} else if ((offset >= 0x00700000/4) && (offset < 0x00800000/4)) {
		ret=ramin[offset-0x00700000/4];
		//logerror("NV_2A: read PRAMIN[%06X] value %08X\n",offset*4-0x00700000,ret);
	} else if ((offset >= 0x00400000/4) && (offset < 0x00402000/4)) {
		//logerror("NV_2A: read PGRAPH[%06X] value %08X\n",offset*4-0x00400000,ret);
	} else if ((offset >= 0x00600000/4) && (offset < 0x00601000/4)) {
		ret=pcrtc[offset-0x00600000/4];
		//logerror("NV_2A: read PCRTC[%06X] value %08X\n",offset*4-0x00600000,ret);
	} else if ((offset >= 0x00000000/4) && (offset < 0x00001000/4)) {
		ret=pmc[offset-0x00000000/4];
		//logerror("NV_2A: read PMC[%06X] value %08X\n",offset*4-0x00000000,ret);
	} else if ((offset >= 0x00800000/4) && (offset < 0x00900000/4)) {
		// 32 channels size 0x10000 each, 8 subchannels per channel size 0x2000 each
		int chanel,subchannel,suboffset;

		suboffset=offset-0x00800000/4;
		chanel=(suboffset >> (16-2)) & 31;
		subchannel=(suboffset >> (13-2)) & 7;
		suboffset=suboffset & 0x7ff;
		if (suboffset < 0x80/4)
			ret=channel[chanel][subchannel].regs[suboffset];
		//logerror("NV_2A: read channel[%02X,%d,%04X]=%08X\n",chanel,subchannel,suboffset*4,ret);
		return ret;
	} else ;
		//logerror("NV_2A: read at %08X mask %08X value %08X\n",0xfd000000+offset*4,mem_mask,ret);
	return ret;
}

WRITE32_MEMBER( nv2a_renderer::geforce_w )
{
	if ((offset >= 0x00101000/4) && (offset < 0x00102000/4)) {
		//logerror("NV_2A: write STRAPS[%06X] mask %08X value %08X\n",offset*4-0x00101000,mem_mask,data);
	} else if ((offset >= 0x00002000/4) && (offset < 0x00004000/4)) {
		COMBINE_DATA(pfifo+offset-0x00002000/4);
		//logerror("NV_2A: read PFIFO[%06X]=%08X\n",offset*4-0x00002000,data & mem_mask); // 2210 pfifo ramht & 1f0 << 12
	} else if ((offset >= 0x00700000/4) && (offset < 0x00800000/4)) {
		COMBINE_DATA(ramin+offset-0x00700000/4);
		//logerror("NV_2A: write PRAMIN[%06X]=%08X\n",offset*4-0x00700000,data & mem_mask);
	} else if ((offset >= 0x00400000/4) && (offset < 0x00402000/4)) {
		//logerror("NV_2A: write PGRAPH[%06X]=%08X\n",offset*4-0x00400000,data & mem_mask);
	} else if ((offset >= 0x00600000/4) && (offset < 0x00601000/4)) {
		COMBINE_DATA(pcrtc+offset-0x00600000/4);
		//logerror("NV_2A: write PCRTC[%06X]=%08X\n",offset*4-0x00600000,data & mem_mask);
	} else if ((offset >= 0x00000000/4) && (offset < 0x00001000/4)) {
		COMBINE_DATA(pmc+offset-0x00000000/4);
		//logerror("NV_2A: write PMC[%06X]=%08X\n",offset*4-0x00000000,data & mem_mask);
	} else if ((offset >= 0x00800000/4) && (offset < 0x00900000/4)) {
		// 32 channels size 0x10000 each, 8 subchannels per channel size 0x2000 each
		int chanel,subchannel,suboffset;
		int method,count,handle,objclass;
#ifdef LOG_NV2A
		int subch;
#endif

		suboffset=offset-0x00800000/4;
		chanel=(suboffset >> (16-2)) & 31;
		subchannel=(suboffset >> (13-2)) & 7;
		suboffset=suboffset & 0x7ff;
		//logerror("NV_2A: write channel[%02X,%d,%04X]=%08X\n",chanel,subchannel,suboffset*4,data & mem_mask);
		if (suboffset >= 0x80/4)
			return;
		COMBINE_DATA(&channel[chanel][subchannel].regs[suboffset]);
		if ((suboffset == 0x40/4) || (suboffset == 0x44/4)) { // DMA_PUT or DMA_GET
			UINT32 *dmaput,*dmaget;
			UINT32 cmd,cmdtype;
			int countlen;

			dmaput=&channel[chanel][subchannel].regs[0x40/4];
			dmaget=&channel[chanel][subchannel].regs[0x44/4];
			//printf("dmaget %08X dmaput %08X\n\r",*dmaget,*dmaput);
			if ((*dmaput == 0x048cf000) && (*dmaget == 0x07f4d000))
				*dmaget = *dmaput;
			while (*dmaget != *dmaput) {
				cmd=space.read_dword(*dmaget);
				*dmaget += 4;
				cmdtype=geforce_commandkind(cmd);
				switch (cmdtype)
				{
					case 6: // jump
#ifdef LOG_NV2A
						printf("jump dmaget %08X",*dmaget);
#endif
						*dmaget=cmd & 0xfffffffc;
#ifdef LOG_NV2A
						printf(" -> %08X\n\r",*dmaget);
#endif
						break;
					case 0: // increasing method
						method=(cmd >> 2) & 2047; // method*4 is address // if method >= 0x40 send it to assigned object
#ifdef LOG_NV2A
						subch=(cmd >> 13) & 7;
#endif
						count=(cmd >> 18) & 2047;
						if ((method == 0) && (count == 1)) {
							handle=space.read_dword(*dmaget);
							handle=geforce_object_offset(handle);
#ifdef LOG_NV2A
							logerror("  assign to subchannel %d object at %d\n",subch,handle);
#endif
							channel[chanel][subchannel].object.objhandle=handle;
							handle=ramin[handle/4];
							objclass=handle & 0xff;
							channel[chanel][subchannel].object.objclass=objclass;
							*dmaget += 4;
						} else {
#ifdef LOG_NV2A
							logerror("  subch. %d method %04x offset %04x count %d\n",subch,method,method*4,count);
#endif
							while (count > 0) {
								countlen=1;
								geforce_exec_method(space,chanel,subchannel,method,*dmaget,countlen);
								count--;
								method++;
								*dmaget += 4;
							}
						}
						break;
					case 5: // non-increasing method
						method=(cmd >> 2) & 2047;
#ifdef LOG_NV2A
						subch=(cmd >> 13) & 7;
#endif
						count=(cmd >> 18) & 2047;
						if ((method == 0) && (count == 1)) {
#ifdef LOG_NV2A
							logerror("  assign channel %d\n",subch);
#endif
							handle=space.read_dword(*dmaget);
							handle=geforce_object_offset(handle);
#ifdef LOG_NV2A
							logerror("  assign to subchannel %d object at %d\n",subch,handle);
#endif
							channel[chanel][subchannel].object.objhandle=handle;
							handle=ramin[handle/4];
							objclass=handle & 0xff;
							channel[chanel][subchannel].object.objclass=objclass;
							*dmaget += 4;
						} else {
#ifdef LOG_NV2A
							logerror("  subch. %d method %04x offset %04x count %d\n",subch,method,method*4,count);
#endif
							while (count > 0) {
								countlen=count;
								geforce_exec_method(space,chanel,subchannel,method,*dmaget,countlen);
								*dmaget += 4*(count-countlen);
								count=countlen;
							}
						}
						break;
					case 3: // long non-increasing method
						method=(cmd >> 2) & 2047;
#ifdef LOG_NV2A
						subch=(cmd >> 13) & 7;
#endif
						count=space.read_dword(*dmaget);
						*dmaget += 4;
						if ((method == 0) && (count == 1)) {
							handle=space.read_dword(*dmaget);
							handle=geforce_object_offset(handle);
#ifdef LOG_NV2A
							logerror("  assign to subchannel %d object at %d\n",subch,handle);
#endif
							channel[chanel][subchannel].object.objhandle=handle;
							handle=ramin[handle/4];
							objclass=handle & 0xff;
							channel[chanel][subchannel].object.objclass=objclass;
							*dmaget += 4;
						} else {
#ifdef LOG_NV2A
							logerror("  subch. %d method %04x offset %04x count %d\n",subch,method,method*4,count);
#endif
							while (count > 0) {
								countlen=count;
								geforce_exec_method(space,chanel,subchannel,method,*dmaget,countlen);
								*dmaget += 4*(count-countlen);
								count=countlen;
							}
						}
						break;
					default:
						logerror("  unimplemented command %08X\n",cmd);
				}
			}
		}
	} else ;
//      logerror("NV_2A: write at %08X mask %08X value %08X\n",0xfd000000+offset*4,mem_mask,data);
}

READ32_MEMBER( chihiro_state::geforce_r )
{
	return nvidia_nv2a->geforce_r(space,offset,mem_mask);
}

WRITE32_MEMBER( chihiro_state::geforce_w )
{
	nvidia_nv2a->geforce_w(space,offset,data,mem_mask);
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
 * ohci usb controller placeholder
 */

#ifdef LOG_OHCI
static const char *const usbregnames[]={
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

READ32_MEMBER( chihiro_state::usbctrl_r )
{
	if (offset == 0) { /* hack needed until usb (and jvs) is implemented */
		if (usbhack_counter == 0) {
			m_maincpu->space(0).write_byte(0x6a79f,0x01);
			m_maincpu->space(0).write_byte(0x6a7a0,0x00);
			m_maincpu->space(0).write_byte(0x6b575,0x00);
			m_maincpu->space(0).write_byte(0x6b576,0x00);
			m_maincpu->space(0).write_byte(0x6b5af,0x75);
			m_maincpu->space(0).write_byte(0x6b78a,0x75);
			m_maincpu->space(0).write_byte(0x6b7ca,0x00);
			m_maincpu->space(0).write_byte(0x6b7b8,0x00);
			m_maincpu->space(0).write_byte(0x8f5b2,0x75);
			m_maincpu->space(0).write_byte(0x79a9e,0x74);
			m_maincpu->space(0).write_byte(0x79b80,0x74);
			m_maincpu->space(0).write_byte(0x79b97,0x74);
		}
		// after game loaded
		if (usbhack_counter == 1) {
			m_maincpu->space(0).write_byte(0x12e4cf,0x01);
			m_maincpu->space(0).write_byte(0x12e4d0,0x00);
			m_maincpu->space(0).write_byte(0x4793e,0x01);
			m_maincpu->space(0).write_byte(0x4793f,0x00);
			m_maincpu->space(0).write_byte(0x47aa3,0x01);
			m_maincpu->space(0).write_byte(0x47aa4,0x00);
			m_maincpu->space(0).write_byte(0x14f2b6,0x84);
			m_maincpu->space(0).write_byte(0x14f2d1,0x75);
			m_maincpu->space(0).write_byte(0x8732f,0x7d);
			m_maincpu->space(0).write_byte(0x87384,0x7d);
			m_maincpu->space(0).write_byte(0x87388,0xeb);
		}
		usbhack_counter++;
	}
#ifdef LOG_OHCI
	if (offset >= 0x54/4)
		logerror("usb controller 0 register HcRhPortStatus[%d] read\n",(offset-0x54/4)+1);
	else
		logerror("usb controller 0 register %s read\n",usbregnames[offset]);
#endif
	return 0;
}

WRITE32_MEMBER( chihiro_state::usbctrl_w )
{
#ifdef LOG_OHCI
	if (offset >= 0x54/4)
		logerror("usb controller 0 register HcRhPortStatus[%d] write %08X\n",(offset-0x54/4)+1,data);
	else
		logerror("usb controller 0 register %s write %08X\n",usbregnames[offset],data);
#endif
}

/*
 * Audio
 */

READ32_MEMBER( chihiro_state::audio_apu_r )
{
	logerror("Audio_APU: read from %08X mask %08X\n",0xfe800000+offset*4,mem_mask);
	if (offset == 0x20010/4) // some kind of internal counter or state value
		return 0x20+4+8+0x48+0x80;
	return apust.memory[offset];
}

WRITE32_MEMBER( chihiro_state::audio_apu_w )
{
	//UINT32 old;
	UINT32 v;

	logerror("Audio_APU: write at %08X mask %08X value %08X\n",0xfe800000+offset*4,mem_mask,data);
	//old = apust.memory[offset];
	apust.memory[offset] = data;
	if (offset == 0x02040/4) // address of memory area with scatter-gather info (gpdsp scratch dma)
		apust.gpdsp_sgaddress=data;
	if (offset == 0x020d4/4) { // block count (gpdsp)
		apust.gpdsp_sgblocks=data;
		apust.gpdsp_address=apust.space->read_dword(apust.gpdsp_sgaddress); // memory address of first block
		apust.timer->enable();
		apust.timer->adjust(attotime::from_msec(1),0,attotime::from_msec(1));
	}
	if (offset == 0x02048 / 4) // (epdsp scratch dma)
		apust.epdsp_sgaddress=data;
	if (offset == 0x020dc / 4) // (epdsp)
		apust.epdsp_sgblocks=data;
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
		float vv = ((float)v) / 4096.0; // divide by 4096
		float vvv = powf(2, vv); // two to the vv
		int f = vvv*48000.0; // sample rate
		apust.voices_frequency[apust.voice_number] = f;
		return;
	}
	if (offset == 0x203a0 / 4) // start offset of data in scatter-gather heap
		return;
	if (offset == 0x203a4 / 4) { // first sample to play
		apust.voices_position_start[apust.voice_number] = data*1000;
		return;
	}
	if (offset == 0x203dc / 4) { // last sample to play
		apust.voices_position_end[apust.voice_number] = data*1000;
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

READ32_MEMBER( chihiro_state::audio_ac93_r )
{
	UINT32 ret=0;

	logerror("Audio_AC3: read from %08X mask %08X\n",0xfec00000+offset*4,mem_mask);
	if (offset < 0x80/4)
	{
		ret=ac97st.mixer_regs[offset];
	}
	if ((offset >= 0x100/4) && (offset <= 0x138/4))
	{
		offset=offset-0x100/4;
		if (offset == 0x18/4)
		{
			ac97st.controller_regs[offset] &= ~0x02000000; // REGRST: register reset
		}
		if (offset == 0x30/4)
		{
			ac97st.controller_regs[offset] |= 0x100; // PCRDY: primary codec ready
		}
		if (offset == 0x34/4)
		{
			ac97st.controller_regs[offset] &= ~1; // CAS: codec access semaphore
		}
		ret=ac97st.controller_regs[offset];
	}
	return ret;
}

WRITE32_MEMBER( chihiro_state::audio_ac93_w )
{
	logerror("Audio_AC3: write at %08X mask %08X value %08X\n",0xfec00000+offset*4,mem_mask,data);
	if (offset < 0x80/4)
	{
		COMBINE_DATA(ac97st.mixer_regs+offset);
	}
	if ((offset >= 0x100/4) && (offset <= 0x138/4))
	{
		offset=offset-0x100/4;
		COMBINE_DATA(ac97st.controller_regs+offset);
	}
}

TIMER_CALLBACK_MEMBER(chihiro_state::audio_apu_timer)
{
	int cmd;
	int bb, b, v;
	UINT64 bv;
	UINT32 phys;

	cmd=apust.space->read_dword(apust.gpdsp_address+0x800+0x10);
	if (cmd == 3)
		apust.space->write_dword(apust.gpdsp_address+0x800+0x10,0);
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
	if (reg >= 16) logerror("  bus:0 function:%d register:%d data:%08X mask:%08X\n",function,reg,data,mem_mask);
#endif
}

READ32_MEMBER( chihiro_state::dummy_r )
{
	return 0;
}

WRITE32_MEMBER( chihiro_state::dummy_w )
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
	chihirosystem=machine().driver_data<chihiro_state>();
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
		m_num_cylinders=65535;
		m_num_sectors=255;
		m_num_heads=255;
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
	logerror("baseboard: read sector lba %08x\n",lba);
	if (lba >= 0x08000000) {
		off=(lba&0x7ff)*512;
		data=memregion(":others")->base();
		memcpy(buffer,data+off,512);
		return 1;
	}
	if (lba >= 0xf8000) {
		memset(buffer,0,512);
		lba=lba-0xf8000;
		if (lba == 0x4800)
			memcpy(buffer,read_buffer,0x20);
		else if (lba == 0x4801)
			memcpy(buffer,write_buffer,0x20);
		return 1;
	}
	// in a type 1 chihiro this gets data from the dimm board memory
	data=chihirosystem->baseboard_ide_dimmboard(lba);
	if (data != NULL)
		memcpy(buffer,data,512);
	return 1;
}

int ide_baseboard_device::write_sector(UINT32 lba, const void *buffer)
{
	logerror("baseboard: write sector lba %08x\n",lba);
	if (lba >= 0xf8000) {
		lba=lba-0xf8000;
		if (lba == 0x4800)
			memcpy(read_buffer,buffer,0x20);
		else if (lba == 0x4801) {
			memcpy(write_buffer,buffer,0x20);
			// call chihiro driver
			chihirosystem->baseboard_ide_event(3,read_buffer,write_buffer);
		}
	}
	return 1;
}

/*
 * Chihiro Type 1 baseboard
 */

void chihiro_state::dword_write_le(UINT8 *addr,UINT32 d)
{
	addr[0]=d & 255;
	addr[1]=(d >> 8) & 255;
	addr[2]=(d >> 16) & 255;
	addr[3]=(d >> 24) & 255;
}

void chihiro_state::word_write_le(UINT8 *addr,UINT16 d)
{
	addr[0]=d & 255;
	addr[1]=(d >> 8) & 255;
}

void chihiro_state::baseboard_ide_event(int type,UINT8 *read_buffer,UINT8 *write_buffer)
{
	int c;

	if ((type != 3) || ((write_buffer[0] == 0) && (write_buffer[1] == 0)))
		return;
#ifdef LOG_BASEBOARD
	logerror("Baseboard sector command:\n");
	for (int a=0;a < 32;a++)
		logerror(" %02X",write_buffer[a]);
	logerror("\n");
#endif
	// response
	// second word 8001 (8000+counter), first word=first word of written data (command ?), second dword ?
	read_buffer[0]=write_buffer[0];
	read_buffer[1]=write_buffer[1];
	read_buffer[2]=0x01; // write_buffer[2];
	read_buffer[3]=0x80; // write_buffer[3] | 0x80;
	c=write_buffer[2]+(write_buffer[3] << 8); // 0001 0101 0103
	switch (c)
	{
		case 0x0001:
			// second dword
			dword_write_le(read_buffer+4,0x00f00000); // ?
			break;
		case 0x0100:
			// second dword third dword
			dword_write_le(read_buffer+4,5); // game data loading phase
			dword_write_le(read_buffer+8,0); // completion %
			break;
		case 0x0101:
			// third word fourth word
			word_write_le(read_buffer+4,0xca); // ?
			word_write_le(read_buffer+6,0xcb); // ?
			break;
		case 0x0102:
			// second dword
			dword_write_le(read_buffer+4,0); // bit 16 develop. mode
			break;
		case 0x0103:
			// dwords 1 3 4
			memcpy(read_buffer+4,"-abc-abc12345678",16); // ?
			break;
	}
	// clear
	write_buffer[0]=write_buffer[1]=write_buffer[2]=write_buffer[3]=0;
	// irq 10 active
	chihiro_devs.pic8259_2->ir2_w(1);
}

UINT8 *chihiro_state::baseboard_ide_dimmboard(UINT32 lba)
{
	// return pointer to memory containing decrypted gdrom data (contains an image of a fatx partition)
	if (chihiro_devs.dimmboard != NULL)
		return dimm_board_memory+lba*512;
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
	if (offset==2) { // IRQ = 2
		return chihiro_devs.pic8259_2->acknowledge();
	}
	return 0x00;
}

IRQ_CALLBACK_MEMBER(chihiro_state::irq_callback)
{
	int r = 0;
	r = chihiro_devs.pic8259_2->acknowledge();
	if (r==0)
	{
		r = chihiro_devs.pic8259_1->acknowledge();
	}
	if (debug_irq_active)
		debug_generate_irq(debug_irq_number,false);
	return r;
}

WRITE_LINE_MEMBER(chihiro_state::chihiro_pit8254_out0_changed)
{
	if ( chihiro_devs.pic8259_1 )
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

int smbus_callback_pic16lc(chihiro_state &chs,int command,int rw,int data)
{
	return chs.smbus_pic16lc(command, rw, data);
}

int chihiro_state::smbus_pic16lc(int command,int rw,int data)
{
	if (rw == 1) { // read
		if (command == 0) {
			if (pic16lc_buffer[0] == 'D')
				pic16lc_buffer[0]='X';
			else if (pic16lc_buffer[0] == 'X')
				pic16lc_buffer[0]='B';
			else if (pic16lc_buffer[0] == 'B')
				pic16lc_buffer[0]='D';
		}
		logerror("pic16lc: %d %d %d\n",command,rw,pic16lc_buffer[command]);
		return pic16lc_buffer[command];
	} else
		if (command == 0)
			pic16lc_buffer[0]='B';
		else
			pic16lc_buffer[command]=(UINT8)data;
	logerror("pic16lc: %d %d %d\n",command,rw,data);
	return 0;
}

int smbus_callback_cx25871(chihiro_state &chs,int command,int rw,int data)
{
	return chs.smbus_cx25871(command, rw, data);
}

int chihiro_state::smbus_cx25871(int command,int rw,int data)
{
	logerror("cx25871: %d %d %d\n",command,rw,data);
	return 0;
}

// let's try to fake the missing eeprom
static int dummyeeprom[256]={0x94,0x18,0x10,0x59,0x83,0x58,0x15,0xDA,0xDF,0xCC,0x1D,0x78,0x20,0x8A,0x61,0xB8,0x08,0xB4,0xD6,0xA8,
	0x9E,0x77,0x9C,0xEB,0xEA,0xF8,0x93,0x6E,0x3E,0xD6,0x9C,0x49,0x6B,0xB5,0x6E,0xAB,0x6D,0xBC,0xB8,0x80,0x68,0x9D,0xAA,0xCD,0x0B,0x83,
	0x17,0xEC,0x2E,0xCE,0x35,0xA8,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x61,0x62,0x63,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0x00,0x00,
	0x4F,0x6E,0x6C,0x69,0x6E,0x65,0x6B,0x65,0x79,0x69,0x6E,0x76,0x61,0x6C,0x69,0x64,0x00,0x03,0x80,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,
	0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

int smbus_callback_eeprom(chihiro_state &chs,int command,int rw,int data)
{
	return chs.smbus_eeprom(command, rw, data);
}

int chihiro_state::smbus_eeprom(int command,int rw,int data)
{
	if (command >= 112)
		return 0;
	if (rw == 1) { // if reading
		// 8003b744,3b744=0x90 0x90
		// hack to avoid hanging if eeprom contents are not correct
		// this would need dumping the serial eeprom on the xbox board
		if (command == 0) {
			m_maincpu->space(0).write_byte(0x3b744,0x90);
			m_maincpu->space(0).write_byte(0x3b745,0x90);
			m_maincpu->space(0).write_byte(0x3b766,0xc9);
			m_maincpu->space(0).write_byte(0x3b767,0xc3);
		}
		data = dummyeeprom[command]+dummyeeprom[command+1]*256;
		logerror("eeprom: %d %d %d\n",command,rw,data);
		return data;
	}
	logerror("eeprom: %d %d %d\n",command,rw,data);
	dummyeeprom[command]=data;
	return 0;
}

/*
 * SMbus controller
 */

void chihiro_state::smbus_register_device(int address,int (*handler)(chihiro_state &chs,int command,int rw,int data))
{
	if (address < 128)
		smbusst.devices[address]=handler;
}

READ32_MEMBER( chihiro_state::smbus_r )
{
	if ((offset == 0) && (mem_mask == 0xff)) // 0 smbus status
		smbusst.words[offset] = (smbusst.words[offset] & ~mem_mask) | ((smbusst.status << 0) & mem_mask);
	if ((offset == 1) && ((mem_mask == 0x00ff0000) || (mem_mask == 0xffff0000))) // 6 smbus data
		smbusst.words[offset] = (smbusst.words[offset] & ~mem_mask) | ((smbusst.data << 16) & mem_mask);
	return smbusst.words[offset];
}

WRITE32_MEMBER( chihiro_state::smbus_w )
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
		data=data>>16;
		smbusst.control = data;
		int cycletype = smbusst.control & 7;
		if (smbusst.control & 8) { // start
			if ((cycletype & 6) == 2)
			{
				if (smbusst.devices[smbusst.address])
					if (smbusst.rw == 0)
						smbusst.devices[smbusst.address](*this,smbusst.command,smbusst.rw,smbusst.data);
					else
						smbusst.data=smbusst.devices[smbusst.address](*this,smbusst.command,smbusst.rw,smbusst.data);
				else
					logerror("SMBUS: access to missing device at address %d\n",smbusst.address);
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
		data=data>>16;
		smbusst.data = data;
	}
	if ((offset == 2) && (mem_mask == 0xff)) // 8 smbus command
		smbusst.command = data;
}

READ32_MEMBER( chihiro_state::mediaboard_r )
{
	UINT32 r;

	logerror("I/O port read %04x mask %08X\n",offset*4+0x4000,mem_mask);
	r=0;
	if ((offset == 7) && ACCESSING_BITS_16_31)
		r=0x10000000;
	if ((offset == 8) && ACCESSING_BITS_0_15)
		r=0x000000a0;
	if ((offset == 8) && ACCESSING_BITS_16_31)
		r=0x42580000;
	if ((offset == 9) && ACCESSING_BITS_0_15)
		r=0x00004d41;
	if ((offset == 0x3c) && ACCESSING_BITS_0_15)
		r=0x00000000; // bits 15-0 0 if media board present
	if ((offset == 0x3d) && ACCESSING_BITS_0_15)
		r=0x00000002; // bits 3-0 size of dimm board memory. Must be 2
	return r;
}

WRITE32_MEMBER( chihiro_state::mediaboard_w )
{
	logerror("I/O port write %04x mask %08X value %08X\n",offset*4+0x4000,mem_mask,data);
	// irq 10
	if ((offset == 0x38) && ACCESSING_BITS_8_15)
		chihiro_devs.pic8259_2->ir2_w(0);
}

static ADDRESS_MAP_START( xbox_map, AS_PROGRAM, 32, chihiro_state )
	AM_RANGE(0x00000000, 0x07ffffff) AM_RAM // 128 megabytes
	AM_RANGE(0xf0000000, 0xf0ffffff) AM_RAM
	AM_RANGE(0xfd000000, 0xfdffffff) AM_RAM AM_READWRITE(geforce_r, geforce_w)
	AM_RANGE(0xfed00000, 0xfed003ff) AM_READWRITE(usbctrl_r, usbctrl_w)
	AM_RANGE(0xfe800000, 0xfe85ffff) AM_READWRITE(audio_apu_r, audio_apu_w)
	AM_RANGE(0xfec00000, 0xfec001ff) AM_READWRITE(audio_ac93_r, audio_ac93_w)
	AM_RANGE(0xff000000, 0xffffffff) AM_ROM AM_REGION("bios", 0) AM_MIRROR(0x00f80000)
ADDRESS_MAP_END

static ADDRESS_MAP_START(xbox_map_io, AS_IO, 32, chihiro_state )
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

static INPUT_PORTS_START( chihiro )
INPUT_PORTS_END

void chihiro_state::machine_start()
{
	nvidia_nv2a=auto_alloc(machine(), nv2a_renderer(machine()));
	memset(pic16lc_buffer,0,sizeof(pic16lc_buffer));
	pic16lc_buffer[0]='B';
	pic16lc_buffer[4]=0; // A/V connector, 2=vga
	smbus_register_device(0x10,smbus_callback_pic16lc);
	smbus_register_device(0x45,smbus_callback_cx25871);
	smbus_register_device(0x54,smbus_callback_eeprom);
	chihiro_devs.pic8259_1 = machine().device<pic8259_device>( "pic8259_1" );
	chihiro_devs.pic8259_2 = machine().device<pic8259_device>( "pic8259_2" );
	chihiro_devs.ide = machine().device<bus_master_ide_controller_device>( "ide" );
	chihiro_devs.dimmboard=machine().device<naomi_gdrom_board>("rom_board");
	if (chihiro_devs.dimmboard != NULL) {
		dimm_board_memory=chihiro_devs.dimmboard->memory(dimm_board_memory_size);
	}
	memset(apust.memory, 0, sizeof(apust.memory));
	memset(apust.voices_heap_blockaddr, 0, sizeof(apust.voices_heap_blockaddr));
	memset(apust.voices_active, 0, sizeof(apust.voices_active));
	memset(apust.voices_position, 0, sizeof(apust.voices_position));
	memset(apust.voices_position_start, 0, sizeof(apust.voices_position_start));
	memset(apust.voices_position_end, 0, sizeof(apust.voices_position_end));
	memset(apust.voices_position_increment, 0, sizeof(apust.voices_position_increment));
	apust.space = &m_maincpu->space();
	apust.timer=machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(chihiro_state::audio_apu_timer),this),(void *)"APU Timer");
	apust.timer->enable(false);
	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
		debug_console_register_command(machine(),"chihiro",CMDFLAG_NONE,0,1,4,chihiro_debug_commands);
	usbhack_counter=0;
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
	nvidia_nv2a->savestate_items();
}

static SLOT_INTERFACE_START(ide_baseboard)
	SLOT_INTERFACE("bb", IDE_BASEBOARD)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( chihiro_base, chihiro_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PENTIUM3, 733333333) /* Wrong! family 6 model 8 stepping 10 */
	MCFG_CPU_PROGRAM_MAP(xbox_map)
	MCFG_CPU_IO_MAP(xbox_map_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(chihiro_state,irq_callback)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	MCFG_PCI_BUS_LEGACY_ADD("pcibus", 0)
	MCFG_PCI_BUS_LEGACY_DEVICE(0, "PCI Bridge Device - Host Bridge", dummy_pci_r, dummy_pci_w)
	MCFG_PCI_BUS_LEGACY_DEVICE(1, "HUB Interface - ISA Bridge", dummy_pci_r, dummy_pci_w)
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
	MCFG_PIC8259_ADD( "pic8259_1", WRITELINE(chihiro_state, chihiro_pic8259_1_set_int_line), VCC, READ8(chihiro_state,get_slave_ack) )
	MCFG_PIC8259_ADD( "pic8259_2", DEVWRITELINE("pic8259_1", pic8259_device, ir2_w), GND, NULL )

	MCFG_DEVICE_ADD("pit8254", PIT8254, 0)
	MCFG_PIT8253_CLK0(1125000) /* heartbeat IRQ */
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(chihiro_state, chihiro_pit8254_out0_changed))
	MCFG_PIT8253_CLK1(1125000) /* (unused) dram refresh */
	MCFG_PIT8253_CLK2(1125000) /* (unused) pio port c pin 4, and speaker polling enough */
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(chihiro_state, chihiro_pit8254_out2_changed))

	MCFG_BUS_MASTER_IDE_CONTROLLER_ADD( "ide", ide_baseboard, NULL, "bb", true)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(DEVWRITELINE("pic8259_2", pic8259_device, ir6_w))
	MCFG_BUS_MASTER_IDE_CONTROLLER_SPACE("maincpu", AS_PROGRAM)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))  /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 479)
	MCFG_SCREEN_UPDATE_DRIVER(chihiro_state,screen_update_callback)
	MCFG_SCREEN_VBLANK_DRIVER(chihiro_state,vblank_callback)

	MCFG_PALETTE_ADD("palette", 65536)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( chihirogd, chihiro_base )
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
BFN.BIN
a8 0b f8 f2 b9 20 b9 97
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
317-054-COM
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
	DISK_IMAGE_READONLY( "gdx-0004a", 0, BAD_DUMP SHA1(27acd2d0680e6bafa0d052f60b4372adc37224fd) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0372-com.data", 0x00, 0x50, CRC(a15c9666) SHA1(fd36c524744acb33e579ccb257c71375a5d3af67) )
ROM_END

ROM_START( mj2c )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0006c", 0, BAD_DUMP SHA1(505653117a73ed8b256ccf19450e7573a4dc57e9) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE) // key was missing
	ROM_LOAD("317-0374-jpn.data", 0x00, 0x50, NO_DUMP )
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
ROM_START( mj2 )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0006g", 0, SHA1(e306837d5c093fdf1e9ff02239a8563535b1c181) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE) // key was missing
	ROM_LOAD("317-0374-jpn.data", 0x00, 0x50, NO_DUMP )
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
	DISK_IMAGE_READONLY( "gdx-0009b", 0, BAD_DUMP SHA1(e032b9fd8d5d09255592f02f7531a608e8179c9c) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-5101-com.data", 0x00, 0x50, CRC(3af801f3) SHA1(e9a2558930f3f1f55d5b3c2cadad69329d931f26) )
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

ROM_START( wangmd2b )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0015a", 0, BAD_DUMP SHA1(cb306df60550bbd8df312634cb97014bb39f1631) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-5106-jpn.data", 0x00, 0x50, CRC(75c716aa) SHA1(5c2bcf3d28a80b336c6882d5aeb010d04327f8c1) )
ROM_END

ROM_START( wangmid2 )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0016", 0, BAD_DUMP SHA1(259483fd211a70c23205ffd852316d616c5a2740) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-5106-com.data", 0x00, 0x50, CRC(75c716aa) SHA1(5c2bcf3d28a80b336c6882d5aeb010d04327f8c1) )
ROM_END

ROM_START( mj3d )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0017d", 0, BAD_DUMP SHA1(cfbbd452c8f4efe0e99f398f5521fc3574b913bb) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE) // key was missing
	ROM_LOAD("317-0414-jpn.data", 0x00, 0x50, NO_DUMP )
ROM_END

ROM_START( mj3 )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0017f", 0, SHA1(8641be9b2e1d8eb33cf27d3444956c0117debc2f) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE) // key was missing
	ROM_LOAD("317-0414-jpn.data", 0x00, 0x50, NO_DUMP )
ROM_END

ROM_START( scg06nt )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0018a", 0, BAD_DUMP SHA1(e6f3dc8066392854ad7d83f81d3cbc81a5e340b3) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("gdx-0018.data", 0x00, 0x50, CRC(1a210abd) SHA1(43a54d028315d2dfa9f8ea6fb59265e0b980b02f) )
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

PIC16C621A ()
Sticker: 235-5508-0567
VER0001, TEST_OK, BRN.BIN, '70 1F 71 1F' D96446469BDCE9C1
*/
ROM_START( ccfboxa )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0024a", 0, SHA1(79d8c0faeec7cf6882f014760b8af938800b7e52) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0567-exp.data", 0x00, 0x50, NO_DUMP )
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
// 0004     GAME( 2003, outr2o,   outr2,    chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Out Run 2 (GDX-0004)", GAME_NO_SOUND|GAME_NOT_WORKING|GAME_SUPPORTS_SAVE )
/* 0004A */ GAME( 2003, outr2,    chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Out Run 2 (Rev A) (GDX-0004A)", GAME_NO_SOUND|GAME_NOT_WORKING|GAME_SUPPORTS_SAVE )
// 0005     GAME( 2004, sgolcnpt, chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Golf Club Network Pro Tour (GDX-0005)", GAME_NO_SOUND|GAME_NOT_WORKING|GAME_SUPPORTS_SAVE )
// 0006     GAME( 2004, mj2o,     mj2,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (GDX-0006)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0006A    GAME( 2004, mj2a,     mj2,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (Rev A) (GDX-0006A)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0006B    GAME( 2004, mj2b,     mj2,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (Rev B) (GDX-0006B)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0006C */ GAME( 2004, mj2c,     mj2,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (Rev C) (GDX-0006C)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0006D    GAME( 2004, mj2d,     mj2,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (Rev D) (GDX-0006D)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0006E    GAME( 2004, mj2e,     mj2,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (Rev E) (GDX-0006E)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0006F    GAME( 2004, mj2f,     mj2,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (Rev F) (GDX-0006F)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0006G */ GAME( 2004, mj2,      chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (Rev G) (GDX-0006G)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0007  */ GAME( 2004, ollie,    chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega / Amusement Vision",  "Ollie King (GDX-0007)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0008     GAME( 2004, wangmdjo, wangmidj, chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",                    "Wangan Midnight Maximum Tune (Japan) (GDX-0008)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0008A    GAME( 2004, wangmdja, wangmidj, chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",                    "Wangan Midnight Maximum Tune (Japan) (Rev A) (GDX-0008A)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0008B    GAME( 2004, wangmidj, wangmidj, chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",                    "Wangan Midnight Maximum Tune (Japan) (Rev B) (GDX-0008B)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0009     GAME( 2004, wangmido, wangmid,  chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",                    "Wangan Midnight Maximum Tune (Export) (GDX-0009)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0009A    GAME( 2004, wangmida, wangmid,  chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",                    "Wangan Midnight Maximum Tune (Export) (Rev A) (GDX-0009A)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0009B */ GAME( 2004, wangmid,  chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",                    "Wangan Midnight Maximum Tune (Export) (Rev B) (GDX-0009B)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0011     GAME( 2004, outr2sp,  chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Out Run 2 SP (Japan) (GDX-0011)", GAME_NO_SOUND|GAME_NOT_WORKING|GAME_SUPPORTS_SAVE )
// 0012     GAME( 2004, ghostsqo, ghostsqu, chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Ghost Squad (GDX-0012)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0012A */ GAME( 2004, ghostsqu, chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Ghost Squad (Rev A) (GDX-0012A)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0013  */ GAME( 2005, gundamos, chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Banpresto",                "Gundam Battle Operating Simulator (GDX-0013)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0014     GAME( 2004, outr2sto, outr2st,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Out Run 2 Special Tours (GDX-0014)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0014A */ GAME( 2004, outr2st,  chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Out Run 2 Special Tours (Rev A) (GDX-0014A)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0015     GAME( 2005, wanmd2bo, wangmd2b, chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",                    "Wangan Midnight Maximum Tune 2 (Japan) (GDX-0015)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0015A */ GAME( 2005, wangmd2b, chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",                    "Wangan Midnight Maximum Tune 2 (Japan) (Rev A) (GDX-0015A)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0016  */ GAME( 2005, wangmid2, chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",                    "Wangan Midnight Maximum Tune 2 (Export) (GDX-0016)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0017     GAME( 2005, mj3o,     mj3,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 (GDX-0017)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0017A    GAME( 2005, mj3a,     mj3,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 (Rev A) (GDX-0017A)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0017B    GAME( 2005, mj3b,     mj3,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 (Rev B) (GDX-0017B)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0017C    GAME( 2005, mj3c,     mj3,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 (Rev C) (GDX-0017C)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0017D */ GAME( 2005, mj3d,     mj3,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 (Rev D) (GDX-0017D)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0017E    GAME( 2005, mj3e,     mj3,      chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 (Rev E) (GDX-0017E)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0017F */ GAME( 2005, mj3,      chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 (Rev F) (GDX-0017F)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0018     GAME( 2005, scg06nto, scg06nt,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Club Golf 2006 Next Tours (GDX-0018)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0018A */ GAME( 2005, scg06nt,  chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Club Golf 2006 Next Tours (Rev A) (GDX-0018A)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0021     GAME( 2005, mj3evo,   mj3ev,    chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 Evolution (GDX-0021)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0021A    GAME( 2005, mj3ev,    chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 Evolution (Rev A) (GDX-0021A)", GAME_NO_SOUND|GAME_NOT_WORKING )
// 0024A    GAME( 2009, ccfboxo,  ccfboxa,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Chihiro Firmware Update For Compact Flash Box (GDX-0024)", GAME_NO_SOUND|GAME_NOT_WORKING )
/* 0024A */ GAME( 2009, ccfboxa,  chihiro,  chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",                     "Chihiro Firmware Update For Compact Flash Box (Rev A) (GDX-0024A)", GAME_NO_SOUND|GAME_NOT_WORKING )
