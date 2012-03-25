/*

Chihiro is an Xbox-based arcade system from SEGA.

Games on this system include....

   Game (Known media)                                  Manufacturer            Media             Key Chip
+-+---------------------------------------------------+-----------------------+-----------------+--------------|
|*|The House of the Dead III                          | Sega, 2002            | GDROM GDX-0001  | 317-0348-COM |
| |Crazy Taxi High Roller                             | Sega, 2003            | GDROM GDX-0002  | 317-0300-COM |
| |Crazy Taxi High Roller (Rev A)                     | Sega, 2003            | GDROM GDX-0002A | 317-0300-COM |
|*|Crazy Taxi High Roller (Rev B)                     | Sega, 2003            | GDROM GDX-0002B | 317-0300-COM |
| |Virtua Cop 3                                       | Sega, 2003            | GDROM GDX-0003  | 317-0354-COM |
|*|Virtua Cop 3 (Rev A)                               | Sega, 2003            | GDROM GDX-0003A | 317-0354-COM |
| |Out Run 2                                          | Sega, 2003            | GDROM GDX-0004  | 317-0372-COM |
|*|Out Run 2 (Rev A)                                  | Sega, 2003            | GDROM GDX-0004A | 317-0372-COM |
| |Out Run 2 Beta (Rev P)                             | Sega, 2003            | GDROM GDX-0004P |              |
| |Sega Golf Club Network Pro Tour                    | Sega, 2004            | GDROM GDX-0005  |              |
| |Sega Network Taisen Mahjong MJ 2                   | Sega, 2004            | GDROM GDX-0006  |              |
| |Sega Network Taisen Mahjong MJ 2 (Rev A)           | Sega, 2004            | GDROM GDX-0006A |              |
| |Sega Network Taisen Mahjong MJ 2 (Rev B)           | Sega, 2004            | GDROM GDX-0006B |              |
|*|Sega Network Taisen Mahjong MJ 2 (Rev C)           | Sega, 2004            | GDROM GDX-0006C |              |
| |Sega Network Taisen Mahjong MJ 2 (Rev D)           | Sega, 2004            | GDROM GDX-0006D |              |
| |Sega Network Taisen Mahjong MJ 2 (Rev E)           | Sega, 2005            | GDROM GDX-0006E |              |
|*|Ollie King                                         | Sega, 2004            | GDROM GDX-0007  | 317-0377-COM |
| |Wangan Midnight Maximum Tune (Japan)               | Namco, 2004           | GDROM GDX-0008  | 317-5101-JPN |
| |Wangan Midnight Maximum Tune (Japan) (Rev A)       | Namco, 2004           | GDROM GDX-0008A | 317-5101-JPN |
|*|Wangan Midnight Maximum Tune (Japan) (Rev B)       | Namco, 2004           | GDROM GDX-0008B | 317-5101-JPN |
| |Wangan Midnight Maximum Tune (Export)              | Namco, 2004           | GDROM GDX-0009  | 317-5101-COM |
| |Wangan Midnight Maximum Tune (Export) (Rev A)      | Namco, 2004           | GDROM GDX-0009A | 317-5101-COM |
|*|Wangan Midnight Maximum Tune (Export) (Rev B)      | Namco, 2004           | GDROM GDX-0009B | 317-5101-COM |
| |Outrun 2 SP (Japan)                                | Sega, 2004            | GDROM GDX-0011  |              |
| |Ghost Squad                                        | Sega, 2004            | GDROM GDX-0012  | 317-0398-COM |
|*|Ghost Squad (Rev A)                                | Sega, 2004            | GDROM GDX-0012A | 317-0398-COM |
|*|Gundam Battle Operating Simulator                  | Banpresto, 2005       | GDROM GDX-0013  | 317-0400-JPN |
| |Outrun 2 Special Tours                             | Sega, 2004            | GDROM GDX-0014  |              |
|*|Outrun 2 Special Tours (Rev A)                     | Sega, 2004            | GDROM GDX-0014A |              |
|*|Wangan Midnight Maximum Tune 2 (Export)            | Namco, 2005           | GDROM GDX-0015  | 317-5106-COM |
| |Wangan Midnight Maximum Tune 2 (Japan)             | Namco, 2005           | GDROM GDX-0016  | 317-5106-JPN |
|*|Wangan Midnight Maximum Tune 2 (Japan) (Rev A)     | Namco, 2005           | GDROM GDX-0016A | 317-5106-JPN |
| |Sega Network Taisen Mahjong MJ 3                   | Sega, 2005            | GDROM GDX-0017  |              |
| |Sega Network Taisen Mahjong MJ 3 (Rev A)           | Sega, 2005            | GDROM GDX-0017A |              |
| |Sega Network Taisen Mahjong MJ 3 (Rev B)           | Sega, 2005            | GDROM GDX-0017B |              |
| |Sega Network Taisen Mahjong MJ 3 (Rev C)           | Sega, 2005            | GDROM GDX-0017C |              |
|*|Sega Network Taisen Mahjong MJ 3 (Rev D)           | Sega, 2005            | GDROM GDX-0017D |              |
| |Sega Club Golf 2006: Next Tours                    | Sega, 2005            | GDROM GDX-0018  |              |
|*|Sega Club Golf 2006: Next Tours (Rev A)            | Sega, 2005            | GDROM GDX-0018A |              |
| |Sega Network Taisen Mahjong MJ 3 Evolution         | Sega, 2006            | GDROM GDX-0021  |              |
| |Sega Network Taisen Mahjong MJ 3 Evolution (Rev A) | Sega, 2006            | GDROM GDX-0021A |              |
| |Firmware Update For Compact Flash Box              | Sega, 200x            | GDROM GDX-0024  |              |
| |Firmware Update For Compact Flash Box (Rev A)      | Sega, 200x            | GDROM GDX-0024A |              |
+-+---------------------------------------------------+-----------------------+-----------------+--------------+
* denotes these games are archived.

   Game (Unknown media)                                Manufacturer            Media             Key Chip
+-+---------------------------------------------------+-----------------------+-----------------+--------------|
| |Mobile Suit Gundam 0079 - Card Builder             | Dimps/Banpresto, 2005 |                 |              |
| |Mobile Suit Gundam 0079 - Card Builder (Ver 1.007) | Dimps/Banpresto, 2006 |                 |              |
| |Mobile Suit Gundam 0079 - Card Builder (Ver 2.00)  | Dimps/Banpresto, 2006 |                 |              |
| |Mobile Suit Gundam 0079 - Card Builder (Ver 2.01)  | Dimps/Banpresto, 2006 |                 |              |
| |Mobile Suit Gundam 0079 - Card Builder (Ver 2.02)  | Dimps/Banpresto, 2006 |                 |              |
| |Mobile Suit Gundam 0083 - Card Builder             | Dimps/Banpresto, 2007 |                 |              |
| |Mobile Suit Gundam 0083 - Card Builder Ryouyuu Gek.| Dimps/Banpresto, 2007 |                 |              |
| |Quest Of D                                         | Sega, 2004            |                 |              |
| |Quest Of D (Ver 1.02)                              | Sega, 2004            |                 |              |
| |Quest Of D (Ver 1.10)                              | Sega, 2004            |                 |              |
| |Quest Of D (Ver 1.10a)                             | Sega, 2004            |                 |              |
| |Quest Of D (Ver 1.20)                              | Sega, 2005            |                 |              |
| |Quest Of D (Ver 1.20a)                             | Sega, 2005            |                 |              |
| |Quest Of D (Ver 1.21)                              | Sega, 2005            |                 |              |
| |Quest Of D (Ver 2.00)                              | Sega, 2005            |                 |              |
| |Quest Of D (Ver 2.01)                              | Sega, 2005            |                 |              |
| |Quest Of D (Ver 2.02b)                             | Sega, 2006            |                 |              |
| |Quest Of D (Ver 3.00)                              | Sega, 2006            |                 |              |
| |Quest Of D (Ver 3.01)                              | Sega, 2006            |                 |              |
| |Quest Of D (Ver 4.00)                              | Sega, 2007            |                 |              |
| |Quest Of D (Ver 4.00b)                             | Sega, 2008            |                 |              |
| |Quest Of D (Ver 4.00c)                             | Sega, 2008            |                 |              |
| |Quest Of D (Ver 4.01)                              | Sega, 2008            |                 |              |
| |Sangokushi Taisen                                  | Sega, 2005            |                 |              |
| |Sangokushi Taisen (Ver 1.03)                       | Sega, 2005            |                 |              |
| |Sangokushi Taisen (Ver 1.10)                       | Sega, 2005            |                 |              |
| |Sangokushi Taisen (Ver 1.11)                       | Sega, 2005            |                 |              |
| |Sangokushi Taisen (Ver 1.12)                       | Sega, 2006            |                 |              |
| |Sangokushi Taisen 2                                | Sega, 2006            |                 |              |
| |Sangokushi Taisen 2 (Ver 2.01)                     | Sega, 2006            |                 |              |
| |Sangokushi Taisen 3                                | Sega, 2008            |                 |              |
| |Sega Golf Club Network Pro Tour 2005               | Sega, 2005            |                 |              |
+-+---------------------------------------------------+-----------------------+-----------------+--------------+
If you can help with the undumped games or know of missing Chihiro games, please contact...
http://guru.mameworld.info/  or  http://www.mamedev.org

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

*/

/*

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
#include "machine/naomigd.h"
#include "debug/debugcon.h"
#include "debug/debugcmd.h"
#include "debug/debugcpu.h"

#define LOG_PCI
#define LOG_OHCI

static struct {
	device_t	*pic8259_1;
	device_t	*pic8259_2;
	device_t	*ide;
} chihiro_devices;


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
static void jamtable_disasm(running_machine &machine, address_space *space,UINT32 address,UINT32 size) // 0xff000080 == fff00080
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
		opcode=space->read_byte(addr);
		addr++;
		op1=space->read_dword_unaligned(addr);
		addr+=4;
		op2=space->read_dword_unaligned(addr);
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
	address_space *space=machine.firstcpu->space();
	UINT64	addr,size;

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
	address_space *space=machine.firstcpu->space();
	UINT64	addr;
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
	length=space->read_word_unaligned(address);
	maximumlength=space->read_word_unaligned(address+2);
	buffer=space->read_dword_unaligned(address+4);
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
		UINT8 c=space->read_byte(buffer+a);
		debug_console_printf(machine,"%c",c);
	}
	debug_console_printf(machine,"\n");
}

static void dump_process_command(running_machine &machine, int ref, int params, const char **param)
{
	address_space *space=machine.firstcpu->space();
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
	debug_console_printf(machine,"ReadyListHead {%08X,%08X} _LIST_ENTRY\n",space->read_dword_unaligned(address),space->read_dword_unaligned(address+4));
	debug_console_printf(machine,"ThreadListHead {%08X,%08X} _LIST_ENTRY\n",space->read_dword_unaligned(address+8),space->read_dword_unaligned(address+12));
	debug_console_printf(machine,"StackCount %d dword\n",space->read_dword_unaligned(address+16));
	debug_console_printf(machine,"ThreadQuantum %d dword\n",space->read_dword_unaligned(address+20));
	debug_console_printf(machine,"BasePriority %d byte\n",space->read_byte(address+24));
	debug_console_printf(machine,"DisableBoost %d byte\n",space->read_byte(address+25));
	debug_console_printf(machine,"DisableQuantum %d byte\n",space->read_byte(address+26));
	debug_console_printf(machine,"_padding %d byte\n",space->read_byte(address+27));
}

static void dump_list_command(running_machine &machine, int ref, int params, const char **param)
{
	address_space *space=machine.firstcpu->space();
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
		addr=space->read_dword_unaligned(address);
		if (addr == start)
			break;
		if (addr == old)
			break;
		address=(offs_t)addr;
		if (!debug_cpu_translate(space,TRANSLATE_READ_DEBUG,&address))
			break;
	}
}

static void help_command(running_machine &machine, int ref, int params, const char **param)
{
	debug_console_printf(machine,"Available Chihiro commands:\n");
	debug_console_printf(machine,"  chihiro jamdis,<start>,<size> -- Disassemble <size> bytes of JamTable instructions starting at <start>\n");
	debug_console_printf(machine,"  chihiro dump_string,<address> -- Dump _STRING object at <address>\n");
	debug_console_printf(machine,"  chihiro dump_process,<address> -- Dump _PROCESS object at <address>\n");
	debug_console_printf(machine,"  chihiro dump_list,<address>[,<offset>] -- Dump _LIST_ENTRY chain starting at <address>\n");
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
	else
		help_command(machine,ref,params-1,param+1);
}

/*
 * geforce 3d placeholder
 */

static READ32_HANDLER( geforce_r )
{
static int x,ret;

	ret=0;
	if (offset == 0x1804f6) {
		x = x ^ 0x08080808;
		ret=x;
	}
	logerror("NV_2A: read at %08X mask %08X value %08X\n",0xfd000000+offset*4,mem_mask,ret);
	return ret;
}

static WRITE32_HANDLER( geforce_w )
{
	logerror("NV_2A: write at %08X mask %08X value %08X\n",0xfd000000+offset*4,mem_mask,data);
}

static UINT32 geforce_pci_r(device_t *busdevice, device_t *device, int function, int reg, UINT32 mem_mask)
{
#ifdef LOG_PCI
	logerror("  bus:1 function:%d register:%d mask:%08X\n",function,reg,mem_mask);
#endif
	return 0;
}

static void geforce_pci_w(device_t *busdevice, device_t *device, int function, int reg, UINT32 data, UINT32 mem_mask)
{
#ifdef LOG_PCI
	logerror("  bus:1 function:%d register:%d data:%08X mask:%08X\n",function,reg,data,mem_mask);
#endif
}

/*
 * ohci usb controller placeholder
 */

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

static READ32_HANDLER( usbctrl_r )
{
	if (offset == 0) { /* hack needed until usb (and jvs) is implemented */
		chihiro_devices.pic8259_1->machine().firstcpu->space(0)->write_byte(0x6a79f,0x01);
		chihiro_devices.pic8259_1->machine().firstcpu->space(0)->write_byte(0x6a7a0,0x00);
	}
#ifdef LOG_OHCI
	if (offset >= 0x54/4)
		logerror("usb controller 0 register HcRhPortStatus[%d] read\n",(offset-0x54/4)+1);
	else
		logerror("usb controller 0 register %s read\n",usbregnames[offset]);
#endif
	return 0;
}

static WRITE32_HANDLER( usbctrl_w )
{
#ifdef LOG_OHCI
	if (offset >= 0x54/4)
		logerror("usb controller 0 register HcRhPortStatus[%d] write %08X\n",(offset-0x54/4)+1,data);
	else
		logerror("usb controller 0 register %s write %08X\n",usbregnames[offset],data);
#endif
}

/*
 * dummy for non connected devices
 */

static UINT32 dummy_pci_r(device_t *busdevice, device_t *device, int function, int reg, UINT32 mem_mask)
{
#ifdef LOG_PCI
	logerror("  bus:0 function:%d register:%d mask:%08X\n",function,reg,mem_mask);
#endif
	return 0;
}

static void dummy_pci_w(device_t *busdevice, device_t *device, int function, int reg, UINT32 data, UINT32 mem_mask)
{
#ifdef LOG_PCI
	logerror("  bus:0 function:%d register:%d data:%08X mask:%08X\n",function,reg,data,mem_mask);
#endif
}

static READ32_HANDLER( dummy_r )
{
	return 0;
}

static WRITE32_HANDLER( dummy_w )
{
}

/*
 * IDE
 */

INLINE int convert_to_offset_and_size32(offs_t *offset, UINT32 mem_mask)
{
	int size = 4;

	/* determine which real offset */
	if (!ACCESSING_BITS_0_7)
	{
		(*offset)++, size = 3;
		if (!ACCESSING_BITS_8_15)
		{
			(*offset)++, size = 2;
			if (!ACCESSING_BITS_16_23)
				(*offset)++, size = 1;
		}
	}

	/* determine the real size */
	if (ACCESSING_BITS_24_31)
		return size;
	size--;
	if (ACCESSING_BITS_16_23)
		return size;
	size--;
	if (ACCESSING_BITS_8_15)
		return size;
	size--;
	return size;
}

static READ32_HANDLER( ide_r )
{
	int size;

	offset *= 4;
	size = convert_to_offset_and_size32(&offset, mem_mask);

	return ide_controller_r(chihiro_devices.ide, offset+0x01f0, size) << ((offset & 3) * 8);
}

static WRITE32_HANDLER( ide_w )
{
	int size;

	offset *= 4;
	size = convert_to_offset_and_size32(&offset, mem_mask);
	data = data >> ((offset & 3) * 8);

	ide_controller_w(chihiro_devices.ide, offset+0x01f0, size, data);
}

static void ide_interrupt(device_t *device, int state)
{
	pic8259_ir6_w(chihiro_devices.pic8259_2, state); // IRQ 14
}

// ======================> ide_baseboard_device

class ide_baseboard_device : public ide_hdd_device
{
public:
    // construction/destruction
    ide_baseboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual int	 read_sector(UINT32 lba, void *buffer);
	virtual int	 write_sector(UINT32 lba, const void *buffer);
	virtual bool is_ready() { return true; }
	virtual void read_key(UINT8 key[]) { }
protected:
    // device-level overrides
    virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "ide_baseboard"; }
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
    : ide_hdd_device(mconfig, IDE_BASEBOARD, "IDE Baseboard", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ide_baseboard_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ide_baseboard_device::device_reset()
{
	m_num_cylinders=65535;
	m_num_sectors=255;
	m_num_heads=255;
	ide_build_features();
}

int ide_baseboard_device::read_sector(UINT32 lba, void *buffer)
{
	int off;
	UINT8 *data;

	logerror("baseboard: read sector lba %08x\n",lba);
	off=(lba&0x7ff)*512;
	data=machine().region("others")->base();
	memcpy(buffer,data+off,512);
	return 1;
}
int ide_baseboard_device::write_sector(UINT32 lba, const void *buffer)
{
	logerror("baseboard: write sector lba %08x\n",lba);
	return 1;
}

/*
 * PIC & PIT
 */

static WRITE_LINE_DEVICE_HANDLER( chihiro_pic8259_1_set_int_line )
{
	cputag_set_input_line(device->machine(), "maincpu", 0, state ? HOLD_LINE : CLEAR_LINE);
}

static READ8_DEVICE_HANDLER( get_slave_ack )
{
	if (offset==2) { // IRQ = 2
		return pic8259_acknowledge(chihiro_devices.pic8259_2);
	}
	return 0x00;
}

static const struct pic8259_interface chihiro_pic8259_1_config =
{
	DEVCB_LINE(chihiro_pic8259_1_set_int_line),
	DEVCB_LINE_VCC,
	DEVCB_HANDLER(get_slave_ack)
};

static const struct pic8259_interface chihiro_pic8259_2_config =
{
	DEVCB_DEVICE_LINE("pic8259_1", pic8259_ir2_w),
	DEVCB_LINE_GND,
	DEVCB_NULL
};

static IRQ_CALLBACK(irq_callback)
{
	int r = 0;
	r = pic8259_acknowledge(chihiro_devices.pic8259_2);
	if (r==0)
	{
		r = pic8259_acknowledge( chihiro_devices.pic8259_1);
	}
	return r;
}

static WRITE_LINE_DEVICE_HANDLER( chihiro_pit8254_out0_changed )
{
	if ( device->machine().device("pic8259_1") )
	{
		pic8259_ir0_w(device->machine().device("pic8259_1"), state);
	}
}

static WRITE_LINE_DEVICE_HANDLER( chihiro_pit8254_out2_changed )
{
	//chihiro_speaker_set_input( state ? 1 : 0 );
}

static const struct pit8253_config chihiro_pit8254_config =
{
	{
		{
			1125000,				/* heartbeat IRQ */
			DEVCB_NULL,
			DEVCB_LINE(chihiro_pit8254_out0_changed)
		}, {
			1125000,				/* (unused) dram refresh */
			DEVCB_NULL,
			DEVCB_NULL
		}, {
			1125000,				/* (unused) pio port c pin 4, and speaker polling enough */
			DEVCB_NULL,
			DEVCB_LINE(chihiro_pit8254_out2_changed)
		}
	}
};

/*
 * SMbus devices
 */

static UINT8 pic16lc_buffer[0xff];

int smbus_pic16lc(int command,int rw,int data)
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

int smbus_cx25871(int command,int rw,int data)
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

int smbus_eeprom(int command,int rw,int data)
{
	if (command >= 112)
		return 0;
	if (rw == 1) { // if reading
		// 8003b744,3b744=0x90 0x90
		// hack to avoid hanging if eeprom contents are not correct
		// this would need dumping the serial eeprom on the xbox board
		if (command == 0) {
			chihiro_devices.pic8259_1->machine().firstcpu->space(0)->write_byte(0x3b744,0x90);
			chihiro_devices.pic8259_1->machine().firstcpu->space(0)->write_byte(0x3b745,0x90);
			chihiro_devices.pic8259_1->machine().firstcpu->space(0)->write_byte(0x3b766,0xc9);
			chihiro_devices.pic8259_1->machine().firstcpu->space(0)->write_byte(0x3b767,0xc3);
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

typedef struct _smbus_state {
	int status;
	int control;
	int address;
	int data;
	int command;
	int rw;
	int (*devices[128])(int command,int rw,int data);
	UINT32 words[256/4];
} smbus_state;
smbus_state smbusst;

void smbus_register_device(int address,int (*handler)(int command,int rw,int data))
{
	if (address < 128)
		smbusst.devices[address]=handler;
}

static READ32_HANDLER( smbus_r )
{
	if ((offset == 0) && (mem_mask == 0xff)) // 0 smbus status
		smbusst.words[offset] = (smbusst.words[offset] & ~mem_mask) | ((smbusst.status << 0) & mem_mask);
	if ((offset == 1) && ((mem_mask == 0x00ff0000) || (mem_mask == 0xffff0000))) // 6 smbus data
		smbusst.words[offset] = (smbusst.words[offset] & ~mem_mask) | ((smbusst.data << 16) & mem_mask);
	return smbusst.words[offset];
}

static WRITE32_HANDLER( smbus_w )
{
	COMBINE_DATA(smbusst.words);
	if ((offset == 0) && (mem_mask == 0xff)) // 0 smbus status
	{
		if (!((smbusst.status ^ data) & 0x10)) // clearing interrupt
			pic8259_ir3_w(chihiro_devices.pic8259_2, 0); // IRQ 11
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
						smbusst.devices[smbusst.address](smbusst.command,smbusst.rw,smbusst.data);
					else
						smbusst.data=smbusst.devices[smbusst.address](smbusst.command,smbusst.rw,smbusst.data);
				else
					logerror("SMBUS: access to missing device at address %d\n",smbusst.address);
				smbusst.status |= 0x10;
				if (smbusst.control & 0x10)
				{
					pic8259_ir3_w(chihiro_devices.pic8259_2, 1); // IRQ 11
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


static ADDRESS_MAP_START( xbox_map, AS_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x07ffffff) AM_RAM
	AM_RANGE(0xfd000000, 0xfdffffff) AM_READWRITE(geforce_r, geforce_w)
	AM_RANGE(0xfed00000, 0xfed003ff) AM_READWRITE(usbctrl_r, usbctrl_w)
	AM_RANGE(0xff000000, 0xffffffff) AM_ROM AM_REGION("bios", 0) AM_MIRROR(0x00f80000)
ADDRESS_MAP_END

static ADDRESS_MAP_START(xbox_map_io, AS_IO, 32)
	AM_RANGE(0x0020, 0x0023) AM_DEVREADWRITE8("pic8259_1", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE8("pit8254", pit8253_r, pit8253_w, 0xffffffff)
	AM_RANGE(0x00a0, 0x00a3) AM_DEVREADWRITE8("pic8259_2", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x01f0, 0x01f7) AM_READWRITE(ide_r, ide_w)
	AM_RANGE(0x0cf8, 0x0cff) AM_DEVREADWRITE("pcibus", pci_32le_r, pci_32le_w)
	AM_RANGE(0x8000, 0x80ff) AM_READWRITE(dummy_r, dummy_w)
	AM_RANGE(0xc000, 0xc0ff) AM_READWRITE(smbus_r, smbus_w)
	AM_RANGE(0xff60, 0xff67) AM_DEVREADWRITE("ide", ide_bus_master32_r, ide_bus_master32_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( chihiro )
INPUT_PORTS_END

static SCREEN_UPDATE_RGB32( chihiro )
{
    return 0;
}

static MACHINE_START( chihiro )
{
	memset(pic16lc_buffer,0,sizeof(pic16lc_buffer));
	pic16lc_buffer[0]='B';
	pic16lc_buffer[4]=2; // A/V connector, 2=vga
	smbus_register_device(0x10,smbus_pic16lc);
	smbus_register_device(0x45,smbus_cx25871);
	smbus_register_device(0x54,smbus_eeprom);
	device_set_irq_callback(machine.device("maincpu"), irq_callback);
	chihiro_devices.pic8259_1 = machine.device( "pic8259_1" );
	chihiro_devices.pic8259_2 = machine.device( "pic8259_2" );
	chihiro_devices.ide = machine.device( "ide" );
	if (machine.debug_flags & DEBUG_FLAG_ENABLED)
		debug_console_register_command(machine,"chihiro",CMDFLAG_NONE,0,1,4,chihiro_debug_commands);
}

static SLOT_INTERFACE_START(ide_baseboard)
	SLOT_INTERFACE("bb", IDE_BASEBOARD)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( chihiro_base, driver_device )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PENTIUM, 733333333) /* Wrong! */
	MCFG_CPU_PROGRAM_MAP(xbox_map)
	MCFG_CPU_IO_MAP(xbox_map_io)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	MCFG_PCI_BUS_ADD("pcibus", 0)
	MCFG_PCI_BUS_DEVICE(0, "PCI Bridge Device - Host Bridge", dummy_pci_r, dummy_pci_w)
	MCFG_PCI_BUS_DEVICE(1, "HUB Interface - ISA Bridge", dummy_pci_r, dummy_pci_w)
	MCFG_PCI_BUS_DEVICE(2, "OHCI USB Controller 1", dummy_pci_r, dummy_pci_w)
	MCFG_PCI_BUS_DEVICE(3, "OHCI USB Controller 2", dummy_pci_r, dummy_pci_w)
	MCFG_PCI_BUS_DEVICE(30, "AGP Host to PCI Bridge", dummy_pci_r, dummy_pci_w)
	MCFG_PCI_BUS_ADD("agpbus", 1)
	MCFG_PCI_BUS_SIBLING("pcibus")
	MCFG_PCI_BUS_DEVICE(0, "NV2A GeForce 3MX Integrated GPU/Northbridge", geforce_pci_r, geforce_pci_w)
	MCFG_PIC8259_ADD( "pic8259_1", chihiro_pic8259_1_config )
	MCFG_PIC8259_ADD( "pic8259_2", chihiro_pic8259_2_config )
	MCFG_PIT8254_ADD( "pit8254", chihiro_pit8254_config )
	MCFG_IDE_CONTROLLER_ADD( "ide", ide_interrupt , ide_baseboard, "bb", NULL)
	MCFG_IDE_BUS_MASTER_SPACE( "ide", "maincpu", PROGRAM )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 479)
	MCFG_SCREEN_UPDATE_STATIC(chihiro)

	MCFG_MACHINE_START(chihiro)

	MCFG_PALETTE_LENGTH(65536)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( chihirogd, chihiro_base )
	MCFG_NAOMI_GDROM_BOARD_ADD("rom_board", ":gdrom", "pic", NULL, "maincpu", NULL)
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
	ROM_LOAD16_WORD_SWAP_BIOS( 0,  "ver1305.bin", 0x204080, 0x200000, CRC(a738ea1c) SHA1(45d94d0c39be1cb3db9fab6610a88a550adda4e9) ) \

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

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)	// number was not readable on pic, please fix if known
	ROM_LOAD( "317-0xxx-com.pic", 0x000000, 0x004000, CRC(f94cf26f) SHA1(dd4af2b52935c7b2d8cd196ec1a30c0ef0993322) )
ROM_END

ROM_START( crtaxihr )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0002b", 0, BAD_DUMP SHA1(4056ebd5587d6c897f475240bc5a4075a995aa8c) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD( "317-0353-com.pic", 0x000000, 0x004000, CRC(1c6830b1) SHA1(75be47441783c18ee296209a34c432864deed70d) )
ROM_END



GAME( 2002, chihiro,  0,       chihiro_base, chihiro, 0, ROT0, "Sega",      "Chihiro Bios", GAME_NO_SOUND|GAME_NOT_WORKING|GAME_IS_BIOS_ROOT )
GAME( 2002, hotd3,    chihiro, chihirogd,    chihiro, 0, ROT0, "Sega",      "The House of the Dead III (GDX-0001)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2003, crtaxihr, chihiro, chihirogd,    chihiro, 0, ROT0, "Sega",      "Crazy Taxi High Roller (Rev B) (GDX-0002B)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2003, vcop3,    chihiro, chihirogd,    chihiro, 0, ROT0, "Sega",      "Virtua Cop 3 (Rev A) (GDX-0003A)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2003, outr2,    chihiro, chihirogd,    chihiro, 0, ROT0, "Sega",      "Out Run 2 (Rev A) (GDX-0004A)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2004, mj2,      chihiro, chihirogd,    chihiro, 0, ROT0, "Sega",      "Sega Network Taisen Mahjong MJ 2 (Rev C) (GDX-0006C)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2004, ollie,    chihiro, chihirogd,    chihiro, 0, ROT0, "Sega",      "Ollie King (GDX-0007)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2004, wangmid,  chihiro, chihirogd,    chihiro, 0, ROT0, "Namco",     "Wangan Midnight Maximum Tune (Export) (Rev B) (GDX-0009B)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2004, ghostsqu, chihiro, chihirogd,    chihiro, 0, ROT0, "Sega",      "Ghost Squad (Rev A) (GDX-0012A)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2005, gundamos, chihiro, chihirogd,    chihiro, 0, ROT0, "Banpresto", "Gundam Battle Operating Simulator (GDX-0013)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2004, outr2st,  chihiro, chihirogd,    chihiro, 0, ROT0, "Sega",      "Out Run 2 Special Tours (Rev A) (GDX-0014A)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2005, wangmid2, chihiro, chihirogd,    chihiro, 0, ROT0, "Namco",     "Wangan Midnight Maximum Tune 2 (Export) (GDX-0015)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2005, wangmd2b, chihiro, chihirogd,    chihiro, 0, ROT0, "Namco",     "Wangan Midnight Maximum Tune 2 (Japan) (Rev A) (GDX-0016A)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2005, mj3,      chihiro, chihirogd,    chihiro, 0, ROT0, "Sega",      "Sega Network Taisen Mahjong MJ 3 (Rev D) (GDX-0017D)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 2005, scg06nt,  chihiro, chihirogd,    chihiro, 0, ROT0, "Sega",      "Sega Club Golf 2006 Next Tours (Rev A) (GDX-0018A)", GAME_NO_SOUND|GAME_NOT_WORKING )
