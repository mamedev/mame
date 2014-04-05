/*

Chihiro is an Xbox-based arcade system from SEGA.

Games on this system include....

   Game (Known media)                                  Manufacturer      Media               Key Chip
+-+---------------------------------------------------+-----------------+-------------------+--------------|
|*|The House of the Dead III                          | Sega, 2002      | GDROM  GDX-0001   | 317-0348-COM |
| |Crazy Taxi High Roller                             | Sega, 2003      | GDROM  GDX-0002   | 317-0300-COM |
| |Crazy Taxi High Roller (Rev A)                     | Sega, 2003      | GDROM  GDX-0002A  | 317-0300-COM |
|*|Crazy Taxi High Roller (Rev B)                     | Sega, 2003      | GDROM  GDX-0002B  | 317-0300-COM |
| |Virtua Cop 3                                       | Sega, 2003      | GDROM  GDX-0003   | 317-0354-COM |
|*|Virtua Cop 3 (Rev A)                               | Sega, 2003      | GDROM  GDX-0003A  | 317-0354-COM |
| |Out Run 2                                          | Sega, 2003      | GDROM  GDX-0004   | 317-0372-COM |
|*|Out Run 2 (Rev A)                                  | Sega, 2003      | GDROM  GDX-0004A  | 317-0372-COM |
| |Out Run 2 prototype (Rev P)                        | Sega, 2003      | GDROM  GDX-0004P  |              |
| |Sega Golf Club Network Pro Tour                    | Sega, 2004      | GDROM  GDX-0005   |              |
| |Sega Network Taisen Mahjong MJ 2                   | Sega, 2004      | GDROM  GDX-0006   |              |
| |Sega Network Taisen Mahjong MJ 2 (Rev A)           | Sega, 2004      | GDROM  GDX-0006A  |              |
| |Sega Network Taisen Mahjong MJ 2 (Rev B)           | Sega, 2004      | GDROM  GDX-0006B  |              |
|*|Sega Network Taisen Mahjong MJ 2 (Rev C)           | Sega, 2004      | GDROM  GDX-0006C  |              |
| |Sega Network Taisen Mahjong MJ 2 (Rev D)           | Sega, 2004      | GDROM  GDX-0006D  |              |
| |Sega Network Taisen Mahjong MJ 2 (Rev E)           | Sega, 2005      | GDROM  GDX-0006E  |              |
| |Sega Network Taisen Mahjong MJ 2 (Rev F)           | Sega, 2005      | GDROM  GDX-0006F  |              |
|*|Sega Network Taisen Mahjong MJ 2 (Rev G)           | Sega, 2005      | GDROM  GDX-0006G  | 317-0374-JPN |
|*|Ollie King                                         | Sega, 2004      | GDROM  GDX-0007   | 317-0377-COM |
| |Wangan Midnight Maximum Tune (Japan)               | Namco, 2004     | GDROM  GDX-0008   | 317-5101-JPN |
| |Wangan Midnight Maximum Tune (Japan) (Rev A)       | Namco, 2004     | GDROM  GDX-0008A  | 317-5101-JPN |
|*|Wangan Midnight Maximum Tune (Japan) (Rev B)       | Namco, 2004     | GDROM  GDX-0008B  | 317-5101-JPN |
| |Wangan Midnight Maximum Tune (Export)              | Namco, 2004     | GDROM  GDX-0009   | 317-5101-COM |
| |Wangan Midnight Maximum Tune (Export) (Rev A)      | Namco, 2004     | GDROM  GDX-0009A  | 317-5101-COM |
|*|Wangan Midnight Maximum Tune (Export) (Rev B)      | Namco, 2004     | GDROM  GDX-0009B  | 317-5101-COM |
| |Outrun 2 SP (Japan)                                | Sega, 2004      | GDROM  GDX-0011   |              |
| |Ghost Squad                                        | Sega, 2004      | GDROM  GDX-0012   | 317-0398-COM |
|*|Ghost Squad (Rev A)                                | Sega, 2004      | GDROM  GDX-0012A  | 317-0398-COM |
|*|Gundam Battle Operating Simulator                  | Banpresto, 2005 | GDROM  GDX-0013   | 317-0400-JPN |
| |Outrun 2 Special Tours                             | Sega, 2004      | GDROM  GDX-0014   |              |
|*|Outrun 2 Special Tours (Rev A)                     | Sega, 2004      | GDROM  GDX-0014A  |              |
|*|Wangan Midnight Maximum Tune 2 (Export)            | Namco, 2005     | GDROM  GDX-0015   | 317-5106-COM |
| |Wangan Midnight Maximum Tune 2 (Japan)             | Namco, 2005     | GDROM  GDX-0016   | 317-5106-JPN |
|*|Wangan Midnight Maximum Tune 2 (Japan) (Rev A)     | Namco, 2005     | GDROM  GDX-0016A  | 317-5106-JPN |
| |Sega Network Taisen Mahjong MJ 3                   | Sega, 2005      | GDROM  GDX-0017   |              |
| |Sega Network Taisen Mahjong MJ 3 (Rev A)           | Sega, 2005      | GDROM  GDX-0017A  |              |
| |Sega Network Taisen Mahjong MJ 3 (Rev B)           | Sega, 2005      | GDROM  GDX-0017B  |              |
| |Sega Network Taisen Mahjong MJ 3 (Rev C)           | Sega, 2005      | GDROM  GDX-0017C  |              |
|*|Sega Network Taisen Mahjong MJ 3 (Rev D)           | Sega, 2005      | GDROM  GDX-0017D  |              |
| |Sega Network Taisen Mahjong MJ 3 (Rev E)           | Sega, 2005      | GDROM  GDX-0017E  |              |
|*|Sega Network Taisen Mahjong MJ 3 (Rev F)           | Sega, 2005      | GDROM  GDX-0017F  | 317-0414-JPN |
| |Sega Club Golf 2006: Next Tours                    | Sega, 2005      | GDROM  GDX-0018   |              |
|*|Sega Club Golf 2006: Next Tours (Rev A)            | Sega, 2005      | GDROM  GDX-0018A  |              |
| |Sega Network Taisen Mahjong MJ 3 Evolution         | Sega, 2006      | GDROM  GDX-0021   |              |
| |Sega Network Taisen Mahjong MJ 3 Evolution (Rev A) | Sega, 2006      | GDROM  GDX-0021A  |              |
| |Firmware Update For Compact Flash Box              | Sega, 200x      | GDROM  GDX-0024   |              |
|*|Firmware Update For Compact Flash Box (Rev A)      | Sega, 200x      | GDROM  GDX-0024A  | 317-0567-EXP |
|*|Quest Of D (Ver.1.01C)                             | Sega, 2004      | CDROM  CDV-10005C |              |
|*|Sangokushi Taisen (Ver.1.002)                      | Sega, 2005      | DVDROM CDV-10009D |              |
|*|Sangokushi Taisen 2 (Ver.2.007)                    | Sega, 2006      | DVDROM CDV-10019A |              |
|*|Sangokushi Taisen                                  | Sega, 2005      | DVDROM CDV-10022  |              |
|*|Sangokushi Taisen 2 Firmware Update                | Sega, 2006      | DVDROM CDV-10023  |              |
|*|Sangokushi Taisen 2                                | Sega, 2006      | DVDROM CDV-10029  |              |
|*|Sangokushi Taisen 3                                | Sega, 2008      | DVDROM CDV-10036  |              |
|*|Sangokushi Taisen 3 (Ver.J)                        | Sega, 2008      | DVDROM CDV-10036J |              |
|*|Sangokushi Taisen 3 War Begins (Ver.3.59)          | Sega, 2008      | DVDROM CDV-10041  |              |
|*|Sangokushi Taisen 3 War Begins                     | Sega, 2008      | DVDROM CDV-10042  |              |
+-+---------------------------------------------------+-----------------+-------------------+--------------+
* denotes these games are archived.

   Game (Unknown media)                                Manufacturer
+-+---------------------------------------------------+-----------------+
| |Quest Of D                                         | Sega, 2004      |
| |Quest Of D (Ver.1.02)                              | Sega, 2004      |
| |Quest Of D (Ver.1.10)                              | Sega, 2004      |
| |Quest Of D (Ver.1.10a)                             | Sega, 2004      |
| |Quest Of D (Ver.1.20)                              | Sega, 2005      |
| |Quest Of D (Ver.1.20a)                             | Sega, 2005      |
| |Quest Of D (Ver.1.21)                              | Sega, 2005      |
| |Quest Of D: Gofu no Keisyousya (Ver.2.00)          | Sega, 2005      |
| |Quest Of D: Gofu no Keisyousya (Ver.2.01)          | Sega, 2005      |
| |Quest Of D: Gofu no Keisyousya (Ver.2.02b)         | Sega, 2006      |
| |Quest Of D: Oukoku no Syugosya (Ver.3.00)          | Sega, 2006      |
| |Quest Of D: Oukoku no Syugosya (Ver.3.01)          | Sega, 2006      |
| |Quest Of D: The Battle Kingdom (Ver.4.00)          | Sega, 2007      |
| |Quest Of D: The Battle Kingdom (Ver.4.00b)         | Sega, 2008      |
| |Quest Of D: The Battle Kingdom (Ver.4.00c)         | Sega, 2008      |
| |Quest Of D: The Battle Kingdom (Ver.4.01)          | Sega, 2008      |
| |Sangokushi Taisen (Ver.1.03)                       | Sega, 2005      |
| |Sangokushi Taisen (Ver.1.10)                       | Sega, 2005      |
| |Sangokushi Taisen (Ver.1.11)                       | Sega, 2005      |
| |Sangokushi Taisen (Ver.1.12)                       | Sega, 2006      |
| |Sangokushi Taisen 2 (Ver.2.01)                     | Sega, 2006      |
| |Sega Golf Club Network Pro Tour 2005               | Sega, 2005      |
+-+---------------------------------------------------+-----------------+

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
      JP4-10    - Jumpers. Settings are as follows (taken from Wangan Midnight Maximum Tune 2 Ver.B)
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
      DIPSW - 8-position DIP switch. On this game (Wangan Midnight Maximum Tune 2 Ver.B) DIPs 3, 4, 6, 7 & 8 are set ON. The others are OFF.

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
#include "machine/pci.h"
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
#define LOG_BASEBOARD

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
		UINT32 memory0_sgaddress;
		UINT32 memory0_sgblocks;
		UINT32 memory0_address;
		UINT32 memory1_sgaddress;
		UINT32 memory1_sgblocks;
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
class nv2a_renderer : public poly_manager<float, nvidia_object_data, 12, 6000>
{
public:
	nv2a_renderer(running_machine &machine) : poly_manager<float, nvidia_object_data, 12, 6000>(machine)
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
		memset(words_vertex_attributes,0,sizeof(words_vertex_attributes));
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
	void savestate_items();

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
	int enabled_vertex_attributes;
	int words_vertex_attributes[16];
	bitmap_rgb32 fb;
	UINT32 dilated0[16][2048];
	UINT32 dilated1[16][2048];
	int dilatechose[256];
	nvidia_object_data *objectdata;

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

UINT32 convert_a4r4g4b4_a8r8g8b8(UINT32 a4r4g4b4)
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

UINT32 convert_a1r5g5b5_a8r8g8b8(UINT32 a1r5g5b5)
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

UINT32 convert_r5g6b5_r8g8b8(UINT32 r5g6b5)
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
	UINT32 to,s,c,sa,ca;
	UINT32 a4r4g4b4,a1r5g5b5,r5g6b5;
	int bx,by;
	int color0,color1,color0m2,color1m2;
	UINT32 codes;
	UINT64 alphas;
	int cr,cg,cb;

	switch (texture[number].format) {
		case A8R8G8B8:
			to=dilated0[texture[number].dilate][x]+dilated1[texture[number].dilate][y]; // offset of texel in texture memory
			return *(((UINT32 *)texture[number].buffer)+to); // get texel color
		case DXT1:
			bx=x >> 2;
			by=y >> 2;
			x=x & 3;
			y=y & 3;
			//to=dilated0[texture[number].dilate][bx]+dilated1[texture[number].dilate][by]; // swizzle 4x4 blocks ?
			to=bx+by*(texture[number].sizeu >> 2);
			color0=*((UINT16 *)(((UINT64 *)texture[number].buffer)+to)+0);
			color1=*((UINT16 *)(((UINT64 *)texture[number].buffer)+to)+1);
			codes=*((UINT32 *)(((UINT64 *)texture[number].buffer)+to)+1);
			s=(y << 3)+(x << 1);
			c=(codes >> s) & 3;
			c=c+(color0 > color1 ? 0 : 4);
			color0m2=color0 << 1;
			color1m2=color1 << 1;
			switch (c) {
				case 0:
					return 0xff000000+convert_r5g6b5_r8g8b8(color0);
					break;
				case 1:
					return 0xff000000+convert_r5g6b5_r8g8b8(color1);
					break;
				case 2:
					cb=pal5bit(((color0m2 & 0x003e)+(color1 & 0x001f))/3);
					cg=pal6bit(((color0m2 & 0x0fc0)+(color1 & 0x07e0))/3 >> 5);
					cr=pal5bit(((color0m2 & 0x1f000)+color1)/3 >> 11);
					return 0xff000000|(cr<<16)|(cg<<8)|(cb);
					break;
				case 3:
					cb=pal5bit(((color1m2 & 0x003e)+(color0 & 0x001f))/3);
					cg=pal6bit(((color1m2 & 0x0fc0)+(color0 & 0x07e0))/3 >> 5);
					cr=pal5bit(((color1m2 & 0x1f000)+color0)/3 >> 11);
					return 0xff000000|(cr<<16)|(cg<<8)|(cb);
					break;
				case 4:
					return 0xff000000+convert_r5g6b5_r8g8b8(color0);
					break;
				case 5:
					return 0xff000000+convert_r5g6b5_r8g8b8(color1);
					break;
				case 6:
					cb=pal5bit(((color0 & 0x001f)+(color1 & 0x001f))/2);
					cg=pal6bit(((color0 & 0x07e0)+(color1 & 0x07e0))/2 >> 5);
					cr=pal5bit((color0+color1)/2 >> 11);
					return 0xff000000|(cr<<16)|(cg<<8)|(cb);
					break;
				default:
					return 0xff000000;
					break;
			}
		case DXT3:
			bx=x >> 2;
			by=y >> 2;
			x=x & 3;
			y=y & 3;
			//to=(dilated0[texture[number].dilate][bx]+dilated1[texture[number].dilate][by]) << 1; // swizzle 4x4 blocks ?
			to=(bx+by*(texture[number].sizeu >> 2)) << 1;
			color0=*((UINT16 *)(((UINT64 *)texture[number].buffer)+to)+4);
			color1=*((UINT16 *)(((UINT64 *)texture[number].buffer)+to)+5);
			codes=*((UINT32 *)(((UINT64 *)texture[number].buffer)+to)+3);
			alphas=*(((UINT64 *)texture[number].buffer)+to);
			s=(y << 3)+(x << 1);
			sa=((y << 2)+x) << 2;
			c=(codes >> s) & 3;
			ca=(alphas >> sa) & 15;
			switch (c) {
				case 0:
					return ((ca+(ca << 4)) << 24)+convert_r5g6b5_r8g8b8(color0);
					break;
				case 1:
					return ((ca+(ca << 4)) << 24)+convert_r5g6b5_r8g8b8(color1);
					break;
				case 2:
					cb=pal5bit(((color0 & 0x001f)+(color1 & 0x001f))/2);
					cg=pal6bit(((color0 & 0x07e0)+(color1 & 0x07e0))/2 >> 5);
					cr=pal5bit((color0+color1)/2 >> 11);
					return ((ca+(ca << 4)) << 24)|(cr<<16)|(cg<<8)|(cb);
					break;
				default:
					return (ca+(ca << 4)) << 24;
					break;
			}
			break;
		case A4R4G4B4:
			to=dilated0[texture[number].dilate][x]+dilated1[texture[number].dilate][y]; // offset of texel in texture memory
			a4r4g4b4=*(((UINT16 *)texture[number].buffer)+to); // get texel color
			return convert_a4r4g4b4_a8r8g8b8(a4r4g4b4);
		case A1R5G5B5:
			to=dilated0[texture[number].dilate][x]+dilated1[texture[number].dilate][y]; // offset of texel in texture memory
			a1r5g5b5=*(((UINT16 *)texture[number].buffer)+to); // get texel color
			return convert_a1r5g5b5_a8r8g8b8(a1r5g5b5);
		case R5G6B5:
			to=dilated0[texture[number].dilate][x]+dilated1[texture[number].dilate][y]; // offset of texel in texture memory
			r5g6b5=*(((UINT16 *)texture[number].buffer)+to); // get texel color
			return 0xff000000+convert_r5g6b5_r8g8b8(r5g6b5);
		case R8G8B8_RECT:
			to=texture[number].rectangle_pitch*y+(x << 2);
			return *((UINT32 *)(((UINT8 *)texture[number].buffer)+to));
		default:
			return 0xff00ff00;
	}
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
		*((UINT32 *)objectdata.data->fb.raw_pixptr(scanline,xp))=a8r8g8b8;
		x--;
	}
}

void nv2a_renderer::render_texture_simple(INT32 scanline, const extent_t &extent, const nvidia_object_data &objectdata, int threadid)
{
	int x;

	if (!objectdata.data->texture[0].enabled) {
		return;
	}
	x=extent.stopx-extent.startx-1;
	while (x >= 0) {
		int up,vp;
		int xp=extent.startx+x; // x coordinate of current pixel

		up=(extent.param[4].start+(float)x*extent.param[4].dpdx)*(float)(objectdata.data->texture[0].sizeu-1); // x coordinate of texel in texture
		vp=extent.param[5].start*(float)(objectdata.data->texture[0].sizev-1); // y coordinate of texel in texture
		*((UINT32 *)fb.raw_pixptr(scanline,xp))=texture_get_texel(0, up, vp);
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
		*((UINT32 *)objectdata.data->fb.raw_pixptr(scanline,xp))=a8r8g8b8;
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

void nv2a_renderer::geforce_exec_method(address_space & space,UINT32 chanel,UINT32 subchannel,UINT32 method,UINT32 address,int &countlen)
{
	UINT32 maddress;
	UINT32 data;

	maddress=method*4;
	data=space.read_dword(address);
	channel[chanel][subchannel].object.method[method]=data;
	if (maddress == 0x1810) {
		// draw vertices
		int offset,count,type;
		//int vtxbuf_kind[16],vtxbuf_size[16];
		int vtxbuf_stride[16];
		UINT32 vtxbuf_address[16];
		UINT32 dmahand[2],dmaoff[2],smasiz[2];
		UINT32 tmp,n,m,u;
		render_delegate renderspans;

		offset=data & 0xffffff;
		count=(data >> 24) & 0xff;
		type=channel[chanel][subchannel].object.method[0x17fc/4];
		tmp=channel[chanel][subchannel].object.method[0x1720/4];
		dmahand[0]=channel[chanel][subchannel].object.method[0x019c/4];
		dmahand[1]=channel[chanel][subchannel].object.method[0x01a0/4];
		geforce_read_dma_object(dmahand[0],dmaoff[0],smasiz[0]);
		geforce_read_dma_object(dmahand[1],dmaoff[1],smasiz[1]);
		if (((channel[chanel][subchannel].object.method[0x1e60/4] & 7) > 0) && (combiner.used != 0)) {
			renderspans=render_delegate(FUNC(nv2a_renderer::render_register_combiners),this);
		} else if (texture[0].enabled) {
			renderspans=render_delegate(FUNC(nv2a_renderer::render_texture_simple),this);
		} else
			renderspans=render_delegate(FUNC(nv2a_renderer::render_color),this);
#ifdef LOG_NV2A
		printf("vertex %d %d %d\n\r",type,offset,count);
#endif
		for (n=0;n < 16;n++) {
#ifdef LOG_NV2A
			printf(" %08X %08X\n\r",channel[chanel][subchannel].object.method[0x1720/4+n],channel[chanel][subchannel].object.method[0x1760/4+n]);
#endif
			tmp=channel[chanel][subchannel].object.method[0x1760/4+n]; // VTXBUF_FMT
			//vtxbuf_kind[n]=tmp & 15;
			//vtxbuf_size[n]=(tmp >> 4) & 15;
			vtxbuf_stride[n]=(tmp >> 8) & 255;
			tmp=channel[chanel][subchannel].object.method[0x1720/4+n]; // VTXBUF_OFFSET
			if (tmp & 0x80000000)
				vtxbuf_address[n]=(tmp & 0x0fffffff)+dmaoff[1];
			else
				vtxbuf_address[n]=(tmp & 0x0fffffff)+dmaoff[0];
		}
		if (type == nv2a_renderer::QUADS) {
#if 0
			n=0;
			if (n == 1)
				dumpcombiners(channel[chanel][subchannel].object.method);
#endif
			for (n=0;n <= count;n+=4) {
				vertex_t xy[4];
				float z[4],w[4];
				UINT32 c[4];

				//printf("draw quad\n\r");
				for (m=0;m < 4;m++) {
					*((UINT32 *)(&xy[m].x))=space.read_dword(vtxbuf_address[0]+(n+m+offset)*vtxbuf_stride[0]+0);
					*((UINT32 *)(&xy[m].y))=space.read_dword(vtxbuf_address[0]+(n+m+offset)*vtxbuf_stride[0]+4);
					*((UINT32 *)(&z[m]))=space.read_dword(vtxbuf_address[0]+(n+m+offset)*vtxbuf_stride[0]+8);
					*((UINT32 *)(&w[m]))=space.read_dword(vtxbuf_address[0]+(n+m+offset)*vtxbuf_stride[0]+12);
					c[m]=space.read_dword(vtxbuf_address[3]+(n+m+offset)*vtxbuf_stride[0]+0); // color
					xy[m].p[0]=c[m] & 0xff; // b
					xy[m].p[1]=(c[m] & 0xff00) >> 8; // g
					xy[m].p[2]=(c[m] & 0xff0000) >> 16; // r
					xy[m].p[3]=(c[m] & 0xff000000) >> 24; // a
					for (u=0;u < 4;u++) {
						xy[m].p[4+u*2]=0;
						xy[m].p[5+u*2]=0;
						if (texture[u].enabled) {
							*((UINT32 *)(&xy[m].p[4+u*2]))=space.read_dword(vtxbuf_address[9+u]+(n+m+offset)*vtxbuf_stride[9+u]+0);
							*((UINT32 *)(&xy[m].p[5+u*2]))=space.read_dword(vtxbuf_address[9+u]+(n+m+offset)*vtxbuf_stride[9+u]+4);
						}
					}
				}

				render_polygon<4>(fb.cliprect(),renderspans,4+4*2,xy); // 4 rgba, 4 texture units 2 uv
				/*myline(fb,xy[0].x,xy[0].y,xy[1].x,xy[1].y);
				myline(fb,xy[1].x,xy[1].y,xy[2].x,xy[2].y);
				myline(fb,xy[2].x,xy[2].y,xy[3].x,xy[3].y);
				myline(fb,xy[3].x,xy[3].y,xy[0].x,xy[0].y);*/
#ifdef LOG_NV2A
				printf(" (%f,%f,%f)-(%f,%f,%f)-(%f,%f,%f)-(%f,%f,%f)\n\r",xy[0].x,xy[0].y,z[0],xy[1].x,xy[1].y,z[1],xy[2].x,xy[2].y,z[2],xy[3].x,xy[3].y,z[3]);
#endif
			}
			wait();
		} else if (type == nv2a_renderer::TRIANGLE_STRIP) {
			vertex_t xy[3];
			float z[3],w[3];
			UINT32 c[3];

			//printf("draw triangle\n\r");
			// put first 2 vertices data in elements 0,1 of arrays
			for (m=0;m < 2;m++) {
				*((UINT32 *)(&xy[m].x))=space.read_dword(vtxbuf_address[0]+(m+offset)*vtxbuf_stride[0]+0);
				*((UINT32 *)(&xy[m].y))=space.read_dword(vtxbuf_address[0]+(m+offset)*vtxbuf_stride[0]+4);
				*((UINT32 *)(&z[m]))=space.read_dword(vtxbuf_address[0]+(m+offset)*vtxbuf_stride[0]+8);
				*((UINT32 *)(&w[m]))=space.read_dword(vtxbuf_address[0]+(m+offset)*vtxbuf_stride[0]+12);
				c[m]=space.read_dword(vtxbuf_address[3]+(m+offset)*vtxbuf_stride[0]+0); // color
				xy[m].p[0]=c[m] & 0xff; // b
				xy[m].p[1]=(c[m] & 0xff00) >> 8;  // g
				xy[m].p[2]=(c[m] & 0xff0000) >> 16;  // r
				xy[m].p[3]=(c[m] & 0xff000000) >> 24;  // a
				for (u=0;u < 4;u++) {
					xy[m].p[4+u*2]=0;
					xy[m].p[5+u*2]=0;
					if (texture[u].enabled) {
						*((UINT32 *)(&xy[m].p[4+u*2]))=space.read_dword(vtxbuf_address[9+u]+(m+offset)*vtxbuf_stride[9+u]+0);
						*((UINT32 *)(&xy[m].p[5+u*2]))=space.read_dword(vtxbuf_address[9+u]+(m+offset)*vtxbuf_stride[9+u]+4);
					}
				}
			}
			for (n=2;n <= count;n++) {
				// put vertex n data in element 2 of arrays
				*((UINT32 *)(&xy[2].x))=space.read_dword(vtxbuf_address[0]+(n+offset)*vtxbuf_stride[0]+0);
				*((UINT32 *)(&xy[2].y))=space.read_dword(vtxbuf_address[0]+(n+offset)*vtxbuf_stride[0]+4);
				*((UINT32 *)(&z[2]))=space.read_dword(vtxbuf_address[0]+(n+offset)*vtxbuf_stride[0]+8);
				*((UINT32 *)(&w[2]))=space.read_dword(vtxbuf_address[0]+(n+offset)*vtxbuf_stride[0]+12);
				c[2]=space.read_dword(vtxbuf_address[3]+(n+offset)*vtxbuf_stride[0]+0); // color
				xy[2].p[0]=c[2] & 0xff; // b
				xy[2].p[1]=(c[2] & 0xff00) >> 8; // g
				xy[2].p[2]=(c[2] & 0xff0000) >> 16; // r
				xy[2].p[3]=(c[2] & 0xff000000) >> 24; // a
				for (u=0;u < 4;u++) {
					xy[2].p[4+u*2]=0;
					xy[2].p[5+u*2]=0;
					if (texture[u].enabled) {
						*((UINT32 *)(&xy[2].p[4+u*2]))=space.read_dword(vtxbuf_address[9+u]+(n+offset)*vtxbuf_stride[9+u]+0);
						*((UINT32 *)(&xy[2].p[5+u*2]))=space.read_dword(vtxbuf_address[9+u]+(n+offset)*vtxbuf_stride[9+u]+4);
					}
				}
				// draw triangle
				render_triangle(fb.cliprect(),renderspans,4+4*2,xy[n & 1],xy[~n & 1],xy[2]); // 012,102,012,102...
				// move elements 1,2 to 0,1
				xy[0]=xy[1];
				xy[1]=xy[2];
				z[0]=z[1];
				z[1]=z[2];
				w[0]=w[1];
				w[1]=w[2];
			}
			wait();
		} else {
			logerror("Unsupported primitive %d for method 0x1810\n",type);
		}
		countlen--;
	}
	if (maddress == 0x1818) {
		int n,m,u,vwords;
		int vattrpos[16];
		int type;
		render_delegate renderspans;

		if (((channel[chanel][subchannel].object.method[0x1e60/4] & 7) > 0) && (combiner.used != 0)) {
			renderspans=render_delegate(FUNC(nv2a_renderer::render_register_combiners),this);
		} else if (texture[0].enabled) {
			renderspans=render_delegate(FUNC(nv2a_renderer::render_texture_simple),this);
		} else
			renderspans=render_delegate(FUNC(nv2a_renderer::render_color),this);
		vwords=0;
		for (n=0;n < 16;n++) {
			vattrpos[n]=vwords;
			if ((enabled_vertex_attributes & (1 << n)) != 0)
				vwords += words_vertex_attributes[n];
		}
		// vertices are taken from the next words, not from a vertex buffer
		// first send primitive type with 17fc
		// then countlen number of dwords with 1818
		// end with 17fc primitive type 0
		// at 1760 16 words specify the vertex format:for each possible vertex attribute the number of components (0=not present) and type of each
		if ((countlen % vwords) != 0) {
			logerror("Method 0x1818 got %d words, at least %d were expected\n",countlen,(countlen/vwords+1)*vwords);
			countlen=0;
			return;
		}
		type=channel[chanel][subchannel].object.method[0x17fc/4];
		if (type == nv2a_renderer::TRIANGLE_FAN) {
			vertex_t xy[3];
			float z[3],w[3];
			UINT32 c[3];

			// put first 2 vertices data in elements 0,1 of arrays
			for (m=0;m < 2;m++) {
				// consider only attributes: position,color0,texture 0-3
				// position
				*((UINT32 *)(&xy[m].x))=space.read_dword(address+vattrpos[0]*4+0);
				*((UINT32 *)(&xy[m].y))=space.read_dword(address+vattrpos[0]*4+4);
				*((UINT32 *)(&z[m]))=space.read_dword(address+vattrpos[0]*4+8);
				*((UINT32 *)(&w[m]))=space.read_dword(address+vattrpos[0]*4+12);
				// color
				c[m]=space.read_dword(address+vattrpos[3]*4+0); // color
				xy[m].p[0]=c[m] & 0xff; // b
				xy[m].p[1]=(c[m] & 0xff00) >> 8;  // g
				xy[m].p[2]=(c[m] & 0xff0000) >> 16;  // r
				xy[m].p[3]=(c[m] & 0xff000000) >> 24;  // a
				// texture 0-3
				for (u=0;u < 4;u++) {
					xy[m].p[4+u*2]=0;
					xy[m].p[5+u*2]=0;
					if (texture[u].enabled) {
						*((UINT32 *)(&xy[m].p[4+u*2]))=space.read_dword(address+vattrpos[9+u]*4+0);
						*((UINT32 *)(&xy[m].p[5+u*2]))=space.read_dword(address+vattrpos[9+u]*4+4);
					}
				}
				address=address+vwords*4;
				countlen=countlen-vwords;
			}
			if (countlen <= 0) {
				logerror("Method 0x1818 missing %d words to draw a complete primitive\n",-countlen+vwords);
				countlen=0;
				return;
			}
			for (n=2;countlen > 0;n++) {
				// put vertex n data in element 2 of arrays
				// position
				*((UINT32 *)(&xy[2].x))=space.read_dword(address+vattrpos[0]*4+0);
				*((UINT32 *)(&xy[2].y))=space.read_dword(address+vattrpos[0]*4+4);
				*((UINT32 *)(&z[2]))=space.read_dword(address+vattrpos[0]*4+8);
				*((UINT32 *)(&w[2]))=space.read_dword(address+vattrpos[0]*4+12);
				// color
				c[2]=space.read_dword(address+vattrpos[3]*4+0); // color
				xy[2].p[0]=c[2] & 0xff; // b
				xy[2].p[1]=(c[2] & 0xff00) >> 8;  // g
				xy[2].p[2]=(c[2] & 0xff0000) >> 16;  // r
				xy[2].p[3]=(c[2] & 0xff000000) >> 24;  // a
				// texture 0-3
				for (u=0;u < 4;u++) {
					xy[2].p[4+u*2]=0;
					xy[2].p[5+u*2]=0;
					if (texture[u].enabled) {
						*((UINT32 *)(&xy[2].p[4+u*2]))=space.read_dword(address+vattrpos[9+u]*4+0);
						*((UINT32 *)(&xy[2].p[5+u*2]))=space.read_dword(address+vattrpos[9+u]*4+4);
					}
				}
				address=address+vwords*4;
				countlen=countlen-vwords;
				if (countlen < 0) {
					logerror("Method 0x1818 missing %d words to draw a complete primitive\n",-countlen);
					countlen=0;
					break;
				}
				// draw triangle
				render_triangle(fb.cliprect(),renderspans,4+4*2,xy[0],xy[1],xy[2]); // 012
				// move element 2 to 1
				xy[1]=xy[2];
				z[1]=z[2];
				w[1]=w[2];
			}
			wait();
		} else if (type == nv2a_renderer::TRIANGLE_STRIP) {
			vertex_t xy[3];
			float z[3],w[3];
			UINT32 c[3];

			// put first 2 vertices data in elements 0,1 of arrays
			for (m=0;m < 2;m++) {
				// consider only attributes: position,color0,texture 0-3
				// position
				*((UINT32 *)(&xy[m].x))=space.read_dword(address+vattrpos[0]*4+0);
				*((UINT32 *)(&xy[m].y))=space.read_dword(address+vattrpos[0]*4+4);
				*((UINT32 *)(&z[m]))=space.read_dword(address+vattrpos[0]*4+8);
				*((UINT32 *)(&w[m]))=space.read_dword(address+vattrpos[0]*4+12);
				// color
				c[m]=space.read_dword(address+vattrpos[3]*4+0); // color
				xy[m].p[0]=c[m] & 0xff; // b
				xy[m].p[1]=(c[m] & 0xff00) >> 8;  // g
				xy[m].p[2]=(c[m] & 0xff0000) >> 16;  // r
				xy[m].p[3]=(c[m] & 0xff000000) >> 24;  // a
				// texture 0-3
				for (u=0;u < 4;u++) {
					xy[m].p[4+u*2]=0;
					xy[m].p[5+u*2]=0;
					if (texture[u].enabled) {
						*((UINT32 *)(&xy[m].p[4+u*2]))=space.read_dword(address+vattrpos[9+u]*4+0);
						*((UINT32 *)(&xy[m].p[5+u*2]))=space.read_dword(address+vattrpos[9+u]*4+4);
					}
				}
				address=address+vwords*4;
				countlen=countlen-vwords;
			}
			if (countlen <= 0) {
				logerror("Method 0x1818 missing %d words to draw a complete primitive\n",-countlen+vwords);
				countlen=0;
				return;
			}
			for (n=2;countlen > 0;n++) {
				// put vertex n data in element 2 of arrays
				// put vertex n data in element 2 of arrays
				// position
				*((UINT32 *)(&xy[2].x))=space.read_dword(address+vattrpos[0]*4+0);
				*((UINT32 *)(&xy[2].y))=space.read_dword(address+vattrpos[0]*4+4);
				*((UINT32 *)(&z[2]))=space.read_dword(address+vattrpos[0]*4+8);
				*((UINT32 *)(&w[2]))=space.read_dword(address+vattrpos[0]*4+12);
				// color
				c[2]=space.read_dword(address+vattrpos[3]*4+0); // color
				xy[2].p[0]=c[2] & 0xff; // b
				xy[2].p[1]=(c[2] & 0xff00) >> 8;  // g
				xy[2].p[2]=(c[2] & 0xff0000) >> 16;  // r
				xy[2].p[3]=(c[2] & 0xff000000) >> 24;  // a
				// texture 0-3
				for (u=0;u < 4;u++) {
					xy[2].p[4+u*2]=0;
					xy[2].p[5+u*2]=0;
					if (texture[u].enabled) {
						*((UINT32 *)(&xy[2].p[4+u*2]))=space.read_dword(address+vattrpos[9+u]*4+0);
						*((UINT32 *)(&xy[2].p[5+u*2]))=space.read_dword(address+vattrpos[9+u]*4+4);
					}
				}
				address=address+vwords*4;
				countlen=countlen-vwords;
				if (countlen < 0) {
					logerror("Method 0x1818 missing %d words to draw a complete primitive\n",-countlen);
					countlen=0;
					break;
				}
				// draw triangle
				render_triangle(fb.cliprect(),renderspans,4+4*2,xy[n & 1],xy[~n & 1],xy[2]); // 012,102,012,102...
				// move elements 1,2 to 0,1
				xy[0]=xy[1];
				xy[1]=xy[2];
				z[0]=z[1];
				z[1]=z[2];
				w[0]=w[1];
				w[1]=w[2];
			}
			wait();
		} else if (type == nv2a_renderer::QUADS) {
			vertex_t xy[4];
			float z[4],w[4];
			UINT32 c[4];

			for (n=0;countlen > 0;n+=4) {
				for (m=0;m < 4;m++) {
					// consider only attributes: position,color0,texture 0-3
					// position
					*((UINT32 *)(&xy[m].x))=space.read_dword(address+vattrpos[0]*4+0);
					*((UINT32 *)(&xy[m].y))=space.read_dword(address+vattrpos[0]*4+4);
					*((UINT32 *)(&z[m]))=space.read_dword(address+vattrpos[0]*4+8);
					*((UINT32 *)(&w[m]))=space.read_dword(address+vattrpos[0]*4+12);
					// color
					c[m]=space.read_dword(address+vattrpos[3]*4+0); // color
					xy[m].p[0]=c[m] & 0xff; // b
					xy[m].p[1]=(c[m] & 0xff00) >> 8;  // g
					xy[m].p[2]=(c[m] & 0xff0000) >> 16;  // r
					xy[m].p[3]=(c[m] & 0xff000000) >> 24;  // a
					// texture 0-3
					for (u=0;u < 4;u++) {
						xy[m].p[4+u*2]=0;
						xy[m].p[5+u*2]=0;
						if (texture[u].enabled) {
							*((UINT32 *)(&xy[m].p[4+u*2]))=space.read_dword(address+vattrpos[9+u]*4+0);
							*((UINT32 *)(&xy[m].p[5+u*2]))=space.read_dword(address+vattrpos[9+u]*4+4);
						}
					}
					address=address+vwords*4;
					countlen=countlen-vwords;
				}
				if (countlen < 0) {
					countlen=0;
					break;
				}

				render_polygon<4>(fb.cliprect(),renderspans,4+4*2,xy); // 4 rgba, 4 texture units 2 uv
			}
			wait();
		} else if (type == nv2a_renderer::QUAD_STRIP) {
			vertex_t xy[4];
			float z[4],w[4];
			UINT32 c[4];

			// put first 2 vertices data in elements 0,1 of arrays
			for (m=0;m < 2;m++) {
				// consider only attributes: position,color0,texture 0-3
				// position
				*((UINT32 *)(&xy[m].x))=space.read_dword(address+vattrpos[0]*4+0);
				*((UINT32 *)(&xy[m].y))=space.read_dword(address+vattrpos[0]*4+4);
				*((UINT32 *)(&z[m]))=space.read_dword(address+vattrpos[0]*4+8);
				*((UINT32 *)(&w[m]))=space.read_dword(address+vattrpos[0]*4+12);
				// color
				c[m]=space.read_dword(address+vattrpos[3]*4+0); // color
				xy[m].p[0]=c[m] & 0xff; // b
				xy[m].p[1]=(c[m] & 0xff00) >> 8;  // g
				xy[m].p[2]=(c[m] & 0xff0000) >> 16;  // r
				xy[m].p[3]=(c[m] & 0xff000000) >> 24;  // a
				// texture 0-3
				for (u=0;u < 4;u++) {
					xy[m].p[4+u*2]=0;
					xy[m].p[5+u*2]=0;
					if (texture[u].enabled) {
						*((UINT32 *)(&xy[m].p[4+u*2]))=space.read_dword(address+vattrpos[9+u]*4+0);
						*((UINT32 *)(&xy[m].p[5+u*2]))=space.read_dword(address+vattrpos[9+u]*4+4);
					}
				}
				address=address+vwords*4;
				countlen=countlen-vwords;
			}
			if (countlen <= 0) {
				countlen=0;
				return;
			}
			for (n=2;countlen > 0;n+=2) {
				// put vertices n,n+1 data in elements 3,2 of arrays
				for (m=3;m >= 2;m--) {
					// position
					*((UINT32 *)(&xy[m].x))=space.read_dword(address+vattrpos[0]*4+0);
					*((UINT32 *)(&xy[m].y))=space.read_dword(address+vattrpos[0]*4+4);
					*((UINT32 *)(&z[m]))=space.read_dword(address+vattrpos[0]*4+8);
					*((UINT32 *)(&w[m]))=space.read_dword(address+vattrpos[0]*4+12);
					// color
					c[m]=space.read_dword(address+vattrpos[3]*4+0); // color
					xy[m].p[0]=c[m] & 0xff; // b
					xy[m].p[1]=(c[m] & 0xff00) >> 8;  // g
					xy[m].p[2]=(c[m] & 0xff0000) >> 16;  // r
					xy[m].p[3]=(c[m] & 0xff000000) >> 24;  // a
					// texture 0-3
					for (u=0;u < 4;u++) {
						xy[m].p[4+u*2]=0;
						xy[m].p[5+u*2]=0;
						if (texture[u].enabled) {
							*((UINT32 *)(&xy[m].p[4+u*2]))=space.read_dword(address+vattrpos[9+u]*4+0);
							*((UINT32 *)(&xy[m].p[5+u*2]))=space.read_dword(address+vattrpos[9+u]*4+4);
						}
					}
					address=address+vwords*4;
					countlen=countlen-vwords;
				}
				if (countlen < 0) {
					countlen=0;
					break;
				}
				render_polygon<4>(fb.cliprect(),renderspans,4+4*2,xy); // 4 rgba, 4 texture units 2 uv
				// copy elements 3,2 of arrays to elements 0,1 of arrays
				xy[0]=xy[3];
				z[0]=z[3];
				w[0]=w[3];
				xy[1]=xy[2];
				z[1]=z[2];
				w[1]=w[2];
			}
			wait();
		} else {
			logerror("Unsupported primitive %d for method 0x1818\n",type);
		}
	}
	if ((maddress == 0x1d6c) || (maddress == 0x1d70) || (maddress == 0x1a4))
		countlen--;
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
		// clear framebuffer
		if (data & 0xf0) {
			// clear colors
			UINT32 color=channel[chanel][subchannel].object.method[0x1d90/4];
			fb.fill(color & 0xffffff);
			//printf("clearscreen\n\r");
		}
		if (data & 0x03) {
			// clear stencil+zbuffer
		}
		countlen--;
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
	if ((maddress >= 0x1760) && (maddress < 0x17A0)) {
		int bit=method-0x1760/4;

		data=data & 255;
		if (data > 15)
			enabled_vertex_attributes |= (1 << bit);
		else
			enabled_vertex_attributes &= ~(1 << bit);
		switch (data & 15) {
			case 0:
				words_vertex_attributes[bit]=(((data >> 4) + 3) & 15) >> 2;
				break;
			case nv2a_renderer::FLOAT:
				words_vertex_attributes[bit]=(data >> 4);
				break;
			case nv2a_renderer::UBYTE:
				words_vertex_attributes[bit]=(((data >> 4) + 3) & 15) >> 2;
				break;
			case nv2a_renderer::USHORT:
				words_vertex_attributes[bit]=(((data >> 4) + 1) & 15) >> 1;
				break;
			default:
				words_vertex_attributes[bit]=0;
		}
	}
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
		break;
		case 1:
			return combiner.register_color0[index];
		break;
		case 2:
			return combiner.register_color1[index];
		break;
		case 3:
			return combiner.register_fogcolor[index];
		break;
		case 4:
			return combiner.register_primarycolor[index];
		break;
		case 5:
			return combiner.register_secondarycolor[index];
		break;
		case 8:
			return combiner.register_texture0color[index];
		break;
		case 9:
			return combiner.register_texture1color[index];
		break;
		case 10:
			return combiner.register_texture2color[index];
		break;
		case 11:
			return combiner.register_texture3color[index];
		break;
		case 12:
			return combiner.register_spare0[index];
		break;
		case 13:
			return combiner.register_spare1[index];
		break;
		case 14:
			return combiner.variable_sumclamp[index];
		break;
		case 15:
			return combiner.variable_EF[index];
		break;
	}

	return 0;
}

float *nv2a_renderer::combiner_map_input_select3(int code)
{
	switch (code) {
		case 0:
		default:
			return combiner.register_zero;
		break;
		case 1:
			return combiner.register_color0;
		break;
		case 2:
			return combiner.register_color1;
		break;
		case 3:
			return combiner.register_fogcolor;
		break;
		case 4:
			return combiner.register_primarycolor;
		break;
		case 5:
			return combiner.register_secondarycolor;
		break;
		case 8:
			return combiner.register_texture0color;
		break;
		case 9:
			return combiner.register_texture1color;
		break;
		case 10:
			return combiner.register_texture2color;
		break;
		case 11:
			return combiner.register_texture3color;
		break;
		case 12:
			return combiner.register_spare0;
		break;
		case 13:
			return combiner.register_spare1;
		break;
		case 14:
			return combiner.variable_sumclamp;
		break;
		case 15:
			return combiner.variable_EF;
		break;
	}

	return 0;
}

float *nv2a_renderer::combiner_map_output_select3(int code)
{
	switch (code) {
		case 0:
			return 0;
		break;
		case 1:
			return 0;
		break;
		case 2:
			return 0;
		break;
		case 3:
			return 0;
		break;
		case 4:
			return combiner.register_primarycolor;
		break;
		case 5:
			return combiner.register_secondarycolor;
		break;
		case 8:
			return combiner.register_texture0color;
		break;
		case 9:
			return combiner.register_texture1color;
		break;
		case 10:
			return combiner.register_texture2color;
		break;
		case 11:
			return combiner.register_texture3color;
		break;
		case 12:
			return combiner.register_spare0;
		break;
		case 13:
			return combiner.register_spare1;
		break;
		case 14:
			return 0;
		break;
		case 15:
		default:
			return 0;
		break;
	}

	return 0;
}

float nv2a_renderer::combiner_map_input_function(int code,float value)
{
	float t;

	switch (code) {
		case 0:
			return MAX(0.0,value);
		break;
		case 1:
			t=MAX(value, 0.0);
			return 1.0 - MIN(t, 1.0);
		break;
		case 2:
			return 2.0 * MAX(0.0, value) - 1.0;
		break;
		case 3:
			return -2.0 * MAX(0.0, value) + 1.0;
		break;
		case 4:
			return MAX(0.0, value) - 0.5;
		break;
		case 5:
			return -MAX(0.0, value) + 0.5;
		break;
		case 6:
			return value;
		break;
		case 7:
		default:
			return -value;
		break;
	}

	return 0;
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
		break;
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
	if (offset == 0x20010/4)
		return 0x20+4+8+0x48+0x80;
	return 0;
}

WRITE32_MEMBER( chihiro_state::audio_apu_w )
{
	logerror("Audio_APU: write at %08X mask %08X value %08X\n",0xfe800000+offset*4,mem_mask,data);
	if (offset == 0x2040/4)
		apust.memory0_sgaddress=data;
	if (offset == 0x20d4/4) {
		apust.memory0_sgblocks=data;
		apust.memory0_address=apust.space->read_dword(apust.memory0_sgaddress);
		apust.timer->enable();
		apust.timer->adjust(attotime::from_msec(1),0,attotime::from_msec(1));
	}
	if (offset == 0x2048/4)
		apust.memory1_sgaddress=data;
	if (offset == 0x20dc/4)
		apust.memory1_sgblocks=data;
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
	int cmd=apust.space->read_dword(apust.memory0_address+0x800+0x10);
	if (cmd == 3)
		apust.space->write_dword(apust.memory0_address+0x800+0x10,0);
	/*else
	    logerror("Audio_APU: unexpected value at address %d\n",apust.memory0_address+0x800+0x10);*/
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
	m_maincpu->set_irq_acknowledge_callback(device_irq_acknowledge_delegate(FUNC(chihiro_state::irq_callback),this));
	chihiro_devs.pic8259_1 = machine().device<pic8259_device>( "pic8259_1" );
	chihiro_devs.pic8259_2 = machine().device<pic8259_device>( "pic8259_2" );
	chihiro_devs.ide = machine().device<bus_master_ide_controller_device>( "ide" );
	chihiro_devs.dimmboard=machine().device<naomi_gdrom_board>("rom_board");
	if (chihiro_devs.dimmboard != NULL) {
		dimm_board_memory=chihiro_devs.dimmboard->memory(dimm_board_memory_size);
	}
	apust.space=&m_maincpu->space();
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



ROM_START( hotd3 )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0001", 0, BAD_DUMP  SHA1(174c72f851d0c97e8993227467f16b0781ed2f5c) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0348-com.data", 0x00, 0x50, CRC(d28219ef) SHA1(40dbbc092bc9f99b8d2ae67fbefacd62184f90ec) )
ROM_END

ROM_START( outr2 )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0004a", 0, BAD_DUMP SHA1(27acd2d0680e6bafa0d052f60b4372adc37224fd) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0372-com.data", 0x00, 0x50, CRC(a15c9666) SHA1(fd36c524744acb33e579ccb257c71375a5d3af67) )
ROM_END

/*

Title   GHOST SQUAD
Media ID    004F
Media Config    GD-ROM1/1
Regions J
Peripheral String   0000000
Product Number  GDX-0012A
Version V2.000
Release Date    20041209
Manufacturer ID

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
	ROM_LOAD("gdx-0013.data", 0x00, 0x50, CRC(0479c383) SHA1(7e86a037d2f9d09cec61a38cb19de510bf9482b3) )
ROM_END

/*

Title   VIRTUA COP 3
Media ID    C4AD
Media Config    GD-ROM1/1
Regions J
Peripheral String   0000000
Product Number  GDX-0003A
Version V2.004
Release Date    20030226
Manufacturer ID
TOC DISC
Track   Start Sector    End Sector  Track Size
track01.bin 150 599 1058400
track02.raw 750 2101    3179904
track03.bin 45150   549299  1185760800


PIC
255-5508-354
317-054-COM

*/

ROM_START( vcop3 )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0003a", 0, BAD_DUMP  SHA1(cdfec1d2ef02ae9e29cb1462f08904177bc4c9ea) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0354-com.data", 0x00, 0x50,  CRC(df7e3217) SHA1(9f0f4bf6b15f3b6eeea81eaa27b3d25bd94110da) )
ROM_END


ROM_START( mj2 )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0006c", 0, BAD_DUMP SHA1(505653117a73ed8b256ccf19450e7573a4dc57e9) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE) // key was missing
	ROM_LOAD("gdx-0006c.pic_data", 0x00, 0x50, NO_DUMP )
ROM_END

ROM_START( ollie )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0007", 0, BAD_DUMP SHA1(8898a571a427936bffcecd3ef27cb79087d22798) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("gdx-0007.data", 0x00, 0x50, CRC(d2a8b31f) SHA1(e9ee2df30031826db6bc4bd91969e6680255dcf9) )
ROM_END



ROM_START( wangmid )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0009b", 0, BAD_DUMP SHA1(e032b9fd8d5d09255592f02f7531a608e8179c9c) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("gdx-0009b.data", 0x00, 0x50, CRC(3af801f3) SHA1(e9a2558930f3f1f55d5b3c2cadad69329d931f26) )
ROM_END


ROM_START( wangmid2 )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0015", 0, BAD_DUMP SHA1(259483fd211a70c23205ffd852316d616c5a2740) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-5106-com.data", 0x00, 0x50, CRC(75c716aa) SHA1(5c2bcf3d28a80b336c6882d5aeb010d04327f8c1) )
ROM_END


ROM_START( wangmd2b )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0016a", 0, BAD_DUMP SHA1(cb306df60550bbd8df312634cb97014bb39f1631) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-5106-com.data", 0x00, 0x50, CRC(75c716aa) SHA1(5c2bcf3d28a80b336c6882d5aeb010d04327f8c1) )
ROM_END


ROM_START( mj3 )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0017d", 0, BAD_DUMP SHA1(cfbbd452c8f4efe0e99f398f5521fc3574b913bb) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE) // key was missing
	ROM_LOAD("gdx-0017d.pic_data", 0x00, 0x50, NO_DUMP )
ROM_END

ROM_START( mj3f )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0017f", 0, SHA1(a859313c80c5303bba5514ff734a7205cd12e456) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE) // key was missing
	ROM_LOAD("gdx-0017f.pic_data", 0x00, 0x50, NO_DUMP )
ROM_END

ROM_START( scg06nt )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0018a", 0, BAD_DUMP SHA1(e6f3dc8066392854ad7d83f81d3cbc81a5e340b3) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("gdx-0018.data", 0x00, 0x50, CRC(1a210abd) SHA1(43a54d028315d2dfa9f8ea6fb59265e0b980b02f) )
ROM_END

ROM_START( outr2st )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0014a", 0, BAD_DUMP SHA1(4f9656634c47631f63eab554a13d19b15558217e) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)   // number was not readable on pic, please fix if known
	ROM_LOAD( "317-0xxx-com.pic", 0x000000, 0x004000, CRC(f94cf26f) SHA1(dd4af2b52935c7b2d8cd196ec1a30c0ef0993322) )
ROM_END

ROM_START( crtaxihr )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0002b", 0, BAD_DUMP SHA1(4056ebd5587d6c897f475240bc5a4075a995aa8c) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD( "317-0353-com.pic", 0x000000, 0x004000, CRC(1c6830b1) SHA1(75be47441783c18ee296209a34c432864deed70d) )
ROM_END



GAME( 2002, chihiro,  0,       chihiro_base, chihiro, driver_device, 0, ROT0, "Sega",      "Chihiro Bios", GAME_NO_SOUND|GAME_NOT_WORKING|GAME_IS_BIOS_ROOT )
GAME( 2002, hotd3,    chihiro, chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",      "The House of the Dead III (GDX-0001)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2003, crtaxihr, chihiro, chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",      "Crazy Taxi High Roller (Rev B) (GDX-0002B)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2003, vcop3,    chihiro, chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",      "Virtua Cop 3 (Rev A) (GDX-0003A)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2003, outr2,    chihiro, chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",      "Out Run 2 (Rev A) (GDX-0004A)", GAME_NO_SOUND|GAME_NOT_WORKING|GAME_SUPPORTS_SAVE )
GAME( 2004, mj2,      chihiro, chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",      "Sega Network Taisen Mahjong MJ 2 (Rev C) (GDX-0006C)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2004, ollie,    chihiro, chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",      "Ollie King (GDX-0007)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2004, wangmid,  chihiro, chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",     "Wangan Midnight Maximum Tune (Export) (Rev B) (GDX-0009B)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2004, ghostsqu, chihiro, chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",      "Ghost Squad (Rev A) (GDX-0012A)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2005, gundamos, chihiro, chihirogd,    chihiro, driver_device, 0, ROT0, "Banpresto", "Gundam Battle Operating Simulator (GDX-0013)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2004, outr2st,  chihiro, chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",      "Out Run 2 Special Tours (Rev A) (GDX-0014A)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2005, wangmid2, chihiro, chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",     "Wangan Midnight Maximum Tune 2 (Export) (GDX-0015)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2005, wangmd2b, chihiro, chihirogd,    chihiro, driver_device, 0, ROT0, "Namco",     "Wangan Midnight Maximum Tune 2 (Japan) (Rev A) (GDX-0016A)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2005, mj3,      chihiro, chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",      "Sega Network Taisen Mahjong MJ 3 (Rev D) (GDX-0017D)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2005, mj3f,     mj3,     chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",      "Sega Network Taisen Mahjong MJ 3 (Rev F) (GDX-0017F)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2005, scg06nt,  chihiro, chihirogd,    chihiro, driver_device, 0, ROT0, "Sega",      "Sega Club Golf 2006 Next Tours (Rev A) (GDX-0018A)", GAME_NO_SOUND|GAME_NOT_WORKING )
