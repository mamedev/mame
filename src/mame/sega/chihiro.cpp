// license:BSD-3-Clause
// copyright-holders:Samuele Zannoli
/*
Chihiro is an Xbox-based arcade system from SEGA.

Games on this system include....

GD build date
    yyyymmdd   Game                                                 Manufacturer / Developer   Media    Number       Key Chip
+-+----------+------------------------------------------------------+--------------------------+--------+------------+--------------|
|*| 20021029 | The House of the Dead III                            | Sega / Wow Entertainment | GDROM  | GDX-0001   | 317-0348-COM |
| | 2003     | Crazy Taxi High Roller                               | Sega / Hitmaker          | GDROM  | GDX-0002   | 317-0353-COM |
| | 2003     | Crazy Taxi High Roller (Rev A)                       | Sega / Hitmaker          | GDROM  | GDX-0002A  | 317-0353-COM |
|*| 20030224 | Crazy Taxi High Roller (Rev B)                       | Sega / Hitmaker          | GDROM  | GDX-0002B  | 317-0353-COM |
| | 2003     | Virtua Cop 3                                         | Sega                     | GDROM  | GDX-0003   | 317-0354-COM |
|*| 20030226 | Virtua Cop 3 (Rev A)                                 | Sega                     | GDROM  | GDX-0003A  | 317-0354-COM |
| | 20030226 | Virtua Cop 3 (Rev A)                                 | Sega                     | CF     | MDA-G0010  | 317-0354-COM |
|*| 20030521 | Virtua Cop 3 (Rev B)                                 | Sega                     | GDROM  | GDX-0003B  | 317-0354-COM |
| | 2003     | OutRun 2                                             | Sega                     | GDROM  | GDX-0004   | 317-0372-COM |
|*| 20031017 | OutRun 2 (Rev A)                                     | Sega                     | GDROM  | GDX-0004A  | 317-0372-COM |
| | 200312   | OutRun 2 (Rev A)                                     | Sega                     | CF     | MDA-G0011  | 317-0372-COM |
| | 20030911 | OutRun 2 prototype (Rev P)                           | Sega                     | GDROM  | GDX-0004P  | 317-0372-COM |
| | 2004     | Sega Golf Club Network Pro Tour                      | Sega                     | GDROM  | GDX-0005   |              |
| | 2004     | Sega Network Taisen Mahjong MJ 2                     | Sega                     | GDROM  | GDX-0006   | 317-0374-JPN |
| | 2004     | Sega Network Taisen Mahjong MJ 2 (Rev A)             | Sega                     | GDROM  | GDX-0006A  | 317-0374-JPN |
| | 2004     | Sega Network Taisen Mahjong MJ 2 (Rev B)             | Sega                     | GDROM  | GDX-0006B  | 317-0374-JPN |
|*| 20041102 | Sega Network Taisen Mahjong MJ 2 (Rev C)             | Sega                     | GDROM  | GDX-0006C  | 317-0374-JPN |
| | 2004     | Sega Network Taisen Mahjong MJ 2 (Rev D)             | Sega                     | GDROM  | GDX-0006D  | 317-0374-JPN |
| | 2005     | Sega Network Taisen Mahjong MJ 2 (Rev E)             | Sega                     | GDROM  | GDX-0006E  | 317-0374-JPN |
|*| 2005     | Sega Network Taisen Mahjong MJ 2 (Rev F)             | Sega                     | GDROM  | GDX-0006F  | 317-0374-JPN |
|*| 20050202 | Sega Network Taisen Mahjong MJ 2 (Rev G)             | Sega                     | GDROM  | GDX-0006G  | 317-0374-JPN |
|*| 20031211 | Ollie King                                           | Sega / Amusement Vision  | GDROM  | GDX-0007   | 317-0377-COM |
| | 2004     | Wangan Midnight Maximum Tune (Japan)                 | Namco                    | GDROM  | GDX-0008   | 317-5101-JPN |
| | 2004     | Wangan Midnight Maximum Tune (Japan, Rev A)          | Namco                    | GDROM  | GDX-0008A  | 317-5101-JPN |
|*| 20040610 | Wangan Midnight Maximum Tune (Japan, Rev B)          | Namco                    | GDROM  | GDX-0008B  | 317-5101-JPN |
| | 2004     | Wangan Midnight Maximum Tune (Export)                | Namco                    | GDROM  | GDX-0009   | 317-5101-COM |
| | 2004     | Wangan Midnight Maximum Tune (Export, Rev A)         | Namco                    | GDROM  | GDX-0009A  | 317-5101-COM |
|*| 20040610 | Wangan Midnight Maximum Tune (Export, Rev B)         | Namco                    | GDROM  | GDX-0009B  | 317-5101-COM |
| | 2004     | Sega Golf Club Network Pro Tour 2005                 | Sega                     | GDROM  | GDX-0010   | ???          |
|*| 2004     | Sega Golf Club Network Pro Tour 2005 (Rev B)         | Sega                     | GDROM  | GDX-0010B  | ???          |
|*| 2004     | Sega Golf Club Network Pro Tour 2005 (Rev C)         | Sega                     | GDROM  | GDX-0010C  | ???          |
|*| 20040909 | OutRun 2 Special Tours (Japan)                       | Sega                     | GDROM  | GDX-0011   | 317-0396-COM |
|*| 20041229 | OutRun 2 Special Tours (Japan, Rev A)                | Sega                     | GDROM  | GDX-0011A  | 317-0396-COM |
|*| 20040914 | Ghost Squad                                          | Sega                     | GDROM  | GDX-0012   | 317-0398-COM |
|*| 20041209 | Ghost Squad (Rev A)                                  | Sega                     | GDROM  | GDX-0012A  | 317-0398-COM |
| | 20041209 | Ghost Squad (Rev A)                                  | Sega                     | CF     | MDA-G0013  | 317-0398-COM |
|*| 20050208 | Gundam Battle Operating Simulator                    | Banpresto                | GDROM  | GDX-0013   | 317-0400-JPN |
|*| 20040910 | OutRun 2 Special Tours                               | Sega                     | GDROM  | GDX-0014   | 317-0396-COM |
|*| 20041229 | OutRun 2 Special Tours (Rev A)                       | Sega                     | GDROM  | GDX-0014A  | 317-0396-COM |
| | 20041229 | OutRun 2 Special Tours (Rev A)                       | Sega                     | CF     | MDA-G0012  | 317-0396-COM |
|*| 20050218 | Wangan Midnight Maximum Tune 2 (Japan)               | Namco                    | GDROM  | GDX-0015   | 317-5106-JPN |
|*| 20050908 | Wangan Midnight Maximum Tune 2 (Japan, Rev A)        | Namco                    | GDROM  | GDX-0015A  | 317-5106-JPN |
|*| 20050218 | Wangan Midnight Maximum Tune 2 (Export)              | Namco                    | GDROM  | GDX-0016   | 317-5106-COM |
|*| 20050908 | Wangan Midnight Maximum Tune 2 (Export, Rev A)       | Namco                    | GDROM  | GDX-0016A  | 317-5106-COM |
| | 2005     | Sega Network Taisen Mahjong MJ 3                     | Sega                     | GDROM  | GDX-0017   | 317-0414-JPN |
| | 2005     | Sega Network Taisen Mahjong MJ 3 (Rev A)             | Sega                     | GDROM  | GDX-0017A  | 317-0414-JPN |
| | 2005     | Sega Network Taisen Mahjong MJ 3 (Rev B)             | Sega                     | GDROM  | GDX-0017B  | 317-0414-JPN |
|*| 20051109 | Sega Network Taisen Mahjong MJ 3 (Rev C)             | Sega                     | GDROM  | GDX-0017C  | 317-0414-JPN |
|*| 20060217 | Sega Network Taisen Mahjong MJ 3 (Rev D)             | Sega                     | GDROM  | GDX-0017D  | 317-0414-JPN |
| | 2006     | Sega Network Taisen Mahjong MJ 3 (Rev E)             | Sega                     | GDROM  | GDX-0017E  | 317-0414-JPN |
|*| 2006     | Sega Network Taisen Mahjong MJ 3 (Rev F)             | Sega                     | GDROM  | GDX-0017F  | 317-0414-JPN |
| | 2005     | Sega Club Golf 2006: Next Tours                      | Sega                     | GDROM  | GDX-0018   |              |
|*| 20051107 | Sega Club Golf 2006: Next Tours (Rev A)              | Sega                     | GDROM  | GDX-0018A  | 317-0428-JPN |
|*| 20050905 | Firmware Update For MJ 3                             | Sega                     | GDROM  | GDX-0019   | 317-0414-JPN |
| | 200?     | Sega Club Golf 2006                                  | Sega                     | GDROM  | GDX-0020   |              |
| | 2006     | Sega Network Taisen Mahjong MJ 3 Evolution           | Sega                     | GDROM  | GDX-0021   | 317-0457-JPN |
|*| 20070217 | Sega Network Taisen Mahjong MJ 3 Evolution (Rev A)   | Sega                     | GDROM  | GDX-0021A  | 317-0457-JPN |
|*| 20070525 | Sega Network Taisen Mahjong MJ 3 Evolution (Rev B)   | Sega                     | GDROM  | GDX-0021B  | 317-0457-JPN |
| | 200?     | Sega Network Taisen Mahjong MJ 3 Evo Test Ver        | Sega                     | GDROM  | GDX-0022   |              |
|*| 20061222 | Sega Network Taisen Mahjong MJ 3 Evo Firmware Update | Sega                     | GDROM  | GDX-0023   | 317-0457-JPN |
| | 2009     | Firmware Update For Compact Flash Box                | Sega                     | GDROM  | GDX-0024   |              |
|*| 20090331 | Firmware Update For Compact Flash Box (Rev A)        | Sega                     | GDROM  | GDX-0024A  | 317-0567-EXP |
|*| 20030226 | Chihiro Change Region GD USA                         | Sega                     | GDROM  | 611-0028A  | 317-0351-EXP |
|*| 200412   | Quest Of D Ver.1.01C                                 | Sega                     | CDROM  | CDV-10005C | 317-0376-JPN |
|*| 2005     | Sangokushi Taisen Ver.1.002                          | Sega                     | DVDROM | CDV-10009D |              |
|*| 2005     | Mobile Suit Gundam 0079 Card Builder                 | Banpresto                | DVDROM | CDV-10010  | 317-0415-JPN |
|*| 2006     | Sangokushi Taisen 2 Ver.2.007                        | Sega                     | DVDROM | CDV-10019A |              |
|*| 2005     | Sangokushi Taisen                                    | Sega                     | DVDROM | CDV-10022  |              |
|*| 2006     | Sangokushi Taisen 2 Firmware Update                  | Sega                     | DVDROM | CDV-10023  |              |
|*| 2006     | Mobile Suit Gundam 0079 Card Builder Ver.2.02        | Banpresto                | DVDROM | CDV-10024B | 317-0415-JPN |
|*| 2006     | Sangokushi Taisen 2 Ver.2.100                        | Sega                     | DVDROM | CDV-10029  |              |
|*| 2007     | Mobile Suit Gundam 0083 Card Builder                 | Banpresto                | DVDROM | CDV-10030  | 317-0484-JPN |
|*| 2008     | Sangokushi Taisen 3                                  | Sega                     | DVDROM | CDV-10036  |              |
|*| 2008     | Sangokushi Taisen 3 Ver.J                            | Sega                     | DVDROM | CDV-10036J |              |
|*| 2008     | Mobile Suit Gundam 0083 Card Builder Ver.2.10        | Bandai Namco - Banpresto | DVDROM | CDV-10037B | 317-0484-JPN |
|*| 2008     | Sangokushi Taisen 3 War Begins Ver.3.59              | Sega                     | DVDROM | CDV-10041  |              |
|*| 2008     | Sangokushi Taisen 3 War Begins                       | Sega                     | DVDROM | CDV-10042  |              |
+-+----------+------------------------------------------------------+--------------------------+--------+------------+--------------+
* denotes these games are archived.

    Year   Game (Unknown media)                                Manufacturer  Number
+-+-----------------------------------------------------------+------------+------------+
| | 2004 | Quest Of D                                         | Sega       | CDV-10005  |
| | 2004 | Quest Of D                                         | Sega       | CDV-10005A |
| | 2004 | Quest Of D                                         | Sega       | CDV-10005B |
| | 2004 | Quest Of D 1.01D                                   | Sega       | CDV-10005D |
| | 2004 | Quest Of D 1.02                                    | Sega       | CDV-10005E |
| | 2004 | Quest Of D Senko Habai                             | Sega       | CDV-10005P |
| | 2004 | Quest Of D 1.10                                    | Sega       | CDV-10006  |
| | 2004 | Quest Of D 1.10A                                   | Sega       | CDV-10006A |
| | 200? | Sangokushi Taisen                                  | Sega       | CDV-10009  |
| | 200? | Sangokushi Taisen 0.94                             | Sega       | CDV-10009A |
| | 200? | Sangokushi Taisen 1.00                             | Sega       | CDV-10009B |
| | 200? | Sangokushi Taisen 1.001                            | Sega       | CDV-10009C |
| | 200? | Sangokushi Taisen 1.002                            | Sega       | CDV-10009E |
| | 200? | Sangokushi Taisen 1.003                            | Sega       | CDV-10009F |
| | 200? | Sangokushi Taisen 1.003                            | Sega       | CDV-10009G |
| | 200? | Sangokushi Taisen 1.100                            | Sega       | CDV-10009H |
| | 200? | Sangokushi Taisen 1.003                            | Sega       | CDV-10009J |
| | 200? | Sangokushi Taisen 1.100                            | Sega       | CDV-10009K |
| | 200? | Sangokushi Taisen 1.110                            | Sega       | CDV-10009L |
| | 200? | Sangokushi Taisen 1.120                            | Sega       | CDV-10009M |
| | 200? | Sangokushi Taisen 1.120                            | Sega       | CDV-10009N |
| | 200? | Sangokushi Taisen 1.100 (kakinhenkou)              | Sega       | CDV-10009P |
| | 200? | Sangokushi Taisen 1.121                            | Sega       | CDV-10009Q |
| | 200? | Sangokushi Taisen 1.122                            | Sega       | CDV-10009R |
| | 200? | Mobile Suit Gundam 0079 Card Builder               | Banpresto  | CDV-10010A |
| | 200? | Quest Of D 1.20                                    | Sega       | CDV-10012  |
| | 200? | Quest Of D 1.20A                                   | Sega       | CDV-10012A |
| | 200? | Sangokushi Taisen 1.00 (Asia)                      | Sega       | CDV-10014  |
| | 200? | Sangokushi Taisen (Asia)                           | Sega       | CDV-10014B |
| | 200? | Sangokushi Taisen (Asia)                           | Sega       | CDV-10014C |
| | 200? | Quest Of D 1.21                                    | Sega       | CDV-10016  |
| | 200? | Quest Of D: Gofu no Keisyousya 2.00                | Sega       | CDV-10017  |
| | 200? | Quest Of D: Gofu no Keisyousya 2.01                | Sega       | CDV-10017A |
| | 200? | Quest Of D: Gofu no Keisyousya 2.02                | Sega       | CDV-10017B |
| | 200? | Quest Of D: Gofu no Keisyousya 2.02B               | Sega       | CDV-10017C |
| | 200? | Quest Of D: Gofu no Keisyousya 2.02C               | Sega       | CDV-10017D |
| | 200? | Sangokushi Taisen 2                                | Sega       | CDV-10019  |
| | 200? | Sangokushi Taisen 2 2.010                          | Sega       | CDV-10019B |
| | 200? | Mobile Suit Gundam 0079 Card Builder 2.00          | Banpresto  | CDV-10024  |
| | 200? | Mobile Suit Gundam 0079 Card Builder 2.00          | Banpresto  | CDV-10024A |
| | 200? | Quest Of D: Oukoku no Syugosya 3.00                | Sega       | CDV-10026  |
| | 200? | Quest Of D: Oukoku no Syugosya 3.01                | Sega       | CDV-10026A |
| | 200? | Quest Of D: Oukoku no Syugosya 3.01A               | Sega       | CDV-10026B |
| | 200? | Quest Of D: Oukoku no Syugosya 3.01B               | Sega       | CDV-10026C |
| | 200? | Quest Of D: Oukoku no Syugosya 3.02                | Sega       | CDV-10026D |
| | 200? | Sangokushi Taisen 2                                | Sega       | CDV-10029A |
| | 200? | Mobile Suit Gundam 0083 Card Builder               | Banpresto  | CDV-10030A |
| | 200? | Sangokushi Taisen 2 (Asia)                         | Sega       | CDV-10032  |
| | 200? | Sangokushi Taisen 2 (Asia)                         | Sega       | CDV-10032A |
| | 200? | Sangokushi Taisen 2 (Asia) Check disc              | Sega       | CDV-10033  |
| | 200? | Sangokushi Taisen 2 (Asia) Firmware update         | Sega       | CDV-10034  |
| | 200? | Quest Of D: The Battle Kingdom                     | Sega       | CDV-10035  |
| | 200? | Quest Of D: The Battle Kingdom                     | Sega       | CDV-10035A |
| | 200? | Quest Of D: The Battle Kingdom                     | Sega       | CDV-10035C |
| | 200? | Sangokushi Taisen 3                                | Sega       | CDV-10036A |
| | 200? | Sangokushi Taisen 3                                | Sega       | CDV-10036B |
| | 200? | Sangokushi Taisen 3                                | Sega       | CDV-10036D |
| | 200? | Sangokushi Taisen 3                                | Sega       | CDV-10036G |
| | 200? | Sangokushi Taisen 3                                | Sega       | CDV-10036H |
| | 200? | Sangokushi Taisen 3                                | Sega       | CDV-10036K |
| | 200? | Mobile Suit Gundam 0083 Card Builder 2.00          | Banpresto  | CDV-10037  |
| | 200? | Mobile Suit Gundam 0083 Card Builder 2.02          | Banpresto  | CDV-10037A |
| | 200? | Sangokushi Taisen 2 (Export)                       | Sega       | CDV-10038  |
| | 200? | Sangokushi Taisen 2 (Export)                       | Sega       | CDV-10038A |
| | 200? | Sangokushi Taisen 2 (Export)                       | Sega       | CDV-10038B |
| | 200? | Sega Golf Club Network Pro Tour 2005 (DVD NMC TJP) | Sega       | CDV-10039  |
| | 200? | Sangokushi Taisen 3 (Export)                       | Sega       | CDV-10040  |
| | 200? | Sangokushi Taisen 3 (Export)                       | Sega       | CDV-10040A |
+-+------+----------------------------------------------------+------------+------------+

A Chihiro system consists of several boards.
The system is in 2 separate metal boxes that fit together to form one box.
In order from top to bottom they are....
 - Network board  \
 - Media board    /  Together in the top box

 - Base board     \
 - Xbox board     /  Together in the bottom box

The 2 boxes join together via the Base Board upper connector and Media Board lower connector.

The Microsoft-manufactured XBox board is the lowest board. It's mostly the same as the V1 XBox retail
board with the exception that it has 128MB of RAM and a nVidia MCPX X2 chip. The retail XBox board has a
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
      24LC64    - Microchip 24LC64 64K I2C Serial EEPROM (SOIC8) contains firmware for AN2131 chips
      24LC024   - Microchip 24LC024 2K I2C Serial EEPROM (SOIC8) contain system/game configuration data
      M68AF127B - ST Microelectronics 1Mbit (128K x8), 5V Asynchronous SRAM (SOP32) work ram for AN2131QC
      AN2131QC  - Cypress AN2131 EZ-USB-Family 8051-based High-Speed USB IC's (QFP80) firmware in IC10
      AN2131SC  /                                                             (QFP44) firmware in IC32
      ADM3222   - Analog Devices ADM3222 High-Speed, +3.3V, 2-Channel RS232/V.28 Interface Device (SOIC20)
      SN65240   - Texas Instruments SN65240 USB Port Transient Suppressor (SOIC8)
      BA7623    - Rohm BA7623 75-Ohm driver IC with 3 internal circuits (SOIC8)
      LM1881    - National LM1881 Video Sync Separator (SOIC8)
      DS485     - National DS485 Low-Power RS-485/RS-422 Multipoint Transceiver (SOIC8)
      3771      - Fujitsu MB3771 System Reset IC (SOIC8)
      PC410     - Sharp PC410 Ultra-high Speed Response OPIC Photocoupler
      CN1       - 22-pin multi-wire cable connector joining to XBox board (JST B22B-PHTSS)
      CN5       - USB connector joining to JVS I/O board with standard USB cable
      CN8       - A/V input connector (from XBox board via short A/V cable)
      CN9       - VGA output connector
      CN10      - 14 pin connector (purpose unknown, maybe another video connector)
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
      JP4-10    - Jumpers. Settings are as follows (taken from Wangan Midnight Maximum Tune 2 (Japan, Rev A))
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
      CN4   - 11-pin connector, has signals for 2 usb ports connected to the xbox board
      CN5   - 8-pin connector, has signals for 2 rs232 ports
      CN6   - 7-pin connector
      SW1/2 - test/service buttons
      DIN1  - 96-pin connector joining to Base Board
      DIPSW - 8-position DIP switch. On this game (Wangan Midnight Maximum Tune 2 (Japan, Rev A)) DIPs 3, 4, 6, 7 & 8 are set ON. The others are OFF.

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
Region is located at Offset 0x00001F10 , 01 means JPN, 02 Means USA, 03 Means EXPORT, if you
want to change the region of your Chihiro Board, just change this byte.

Thanks to Alex, Mr Mudkips, and Philip Burke for this info.

*/

#include "emu.h"

#include "jvs13551.h"
#include "xbox_pci.h"
#include "xbox.h"

#include "machine/pci.h"
#include "machine/idectrl.h"

#include "bus/ata/hdd.h"
#include "cpu/i386/i386.h"
#include "machine/jvshost.h"
#include "naomigd.h"

#include "debug/debugcon.h"
#include "debugger.h"

#include <functional>

#define LOG_BASEBOARD (1U << 1)
#define LOG_EXTRA     (1U << 2)

#define VERBOSE (0)
#include "logmacro.h"

/*
 * Class declaration for jvs_master
 */

DECLARE_DEVICE_TYPE(JVS_MASTER, jvs_master)

class jvs_master : public jvs_host
{
public:
	// construction/destruction
	jvs_master(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	int get_sense_line();
	void send_packet(int destination, int length, uint8_t *data);
	int received_packet(uint8_t *buffer);
};

DEFINE_DEVICE_TYPE(JVS_MASTER, jvs_master, "jvs_master", "JVS MASTER")

jvs_master::jvs_master(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: jvs_host(mconfig, JVS_MASTER, tag, owner, clock)
{
}

int jvs_master::get_sense_line()
{
	if (get_presence_line() == false)
		return 50;
	if (get_address_set_line() == true)
		return 0;
	return 25;
}

void jvs_master::send_packet(int destination, int length, uint8_t *data)
{
	push((uint8_t)destination);
	push((uint8_t)length);
	length--;
	while (length > 0)
	{
		push(*data);
		data++;
		length--;
	}
	commit_raw();
}

int jvs_master::received_packet(uint8_t *buffer)
{
	uint32_t length;
	const uint8_t *data;

	get_raw_reply(data, length);
	if (length > 0)
		memcpy(buffer, data, length);
	return (int)length;
}

/*
 * Class declaration for ohci_hlean2131qc_device
 */

DECLARE_DEVICE_TYPE(OHCI_HLEAN2131QC, ohci_hlean2131qc_device)

class ohci_hlean2131qc_device : public device_t, public device_usb_ohci_function_interface
{
public:
	ohci_hlean2131qc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void initialize() override;
	int handle_nonstandard_request(int endpoint, USBSetupPacket *setup) override;
	int handle_bulk_pid(int endpoint, int pid, uint8_t *buffer, int size) override;
	void set_region_base(uint8_t *data);
	void set_region(const char *_region_tag, int _region_offset);

protected:
	virtual void device_start() override ATTR_COLD;
private:
	void process_jvs_packet();

	static const USBStandardDeviceDescriptor devdesc;
	static const USBStandardConfigurationDescriptor condesc;
	static const USBStandardInterfaceDescriptor intdesc;
	static const USBStandardEndpointDescriptor enddesc01;
	static const USBStandardEndpointDescriptor enddesc02;
	static const USBStandardEndpointDescriptor enddesc03;
	static const USBStandardEndpointDescriptor enddesc04;
	static const USBStandardEndpointDescriptor enddesc05;
	static const USBStandardEndpointDescriptor enddesc81;
	static const USBStandardEndpointDescriptor enddesc82;
	static const USBStandardEndpointDescriptor enddesc83;
	static const USBStandardEndpointDescriptor enddesc84;
	static const USBStandardEndpointDescriptor enddesc85;
	static const uint8_t strdesc0[];
	static const uint8_t strdesc1[];
	static const uint8_t strdesc2[];
	int maximum_send;
	const char *region_tag;
	int region_offset;
	uint8_t *region;
	struct
	{
		uint8_t buffer_in[32768];
		int buffer_in_expected;
		uint8_t buffer_out[32768];
		int buffer_out_used;
		int buffer_out_packets;
	} jvs;

	required_device<jvs_master> m_jvs_master;
};

DEFINE_DEVICE_TYPE(OHCI_HLEAN2131QC, ohci_hlean2131qc_device, "ohci_hlean2131qc", "OHCI an2131qc HLE")

#define MCFG_OHCI_HLEAN2131QC_REGION(_region_tag, _region_offset) \
	downcast<ohci_hlean2131qc_device *>(device)->set_region(_region_tag, _region_offset);

/*
 * Class declaration for ohci_hlean2131sc_device
 */

DECLARE_DEVICE_TYPE(OHCI_HLEAN2131SC, ohci_hlean2131sc_device)

class ohci_hlean2131sc_device : public device_t, public device_usb_ohci_function_interface
{
public:
	ohci_hlean2131sc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void initialize() override;
	int handle_nonstandard_request(int endpoint, USBSetupPacket *setup) override;
	int handle_bulk_pid(int endpoint, int pid, uint8_t *buffer, int size) override;
	void set_region_base(uint8_t *data);
	void set_region(const char *_region_tag, int _region_offset);

protected:
	virtual void device_start() override ATTR_COLD;
private:
	void process_packet();

	static const USBStandardDeviceDescriptor devdesc;
	static const USBStandardConfigurationDescriptor condesc;
	static const USBStandardInterfaceDescriptor intdesc;
	static const USBStandardEndpointDescriptor enddesc01;
	static const USBStandardEndpointDescriptor enddesc02;
	static const USBStandardEndpointDescriptor enddesc03;
	static const USBStandardEndpointDescriptor enddesc81;
	static const USBStandardEndpointDescriptor enddesc82;
	static const USBStandardEndpointDescriptor enddesc83;
	static const uint8_t strdesc0[];
	static const uint8_t strdesc1[];
	static const uint8_t strdesc2[];
	const char *region_tag;
	int region_offset;
	uint8_t *region;
	uint8_t midi_rs232;
	uint8_t response[256];
	uint8_t packet[4];
	int response_size;
	int step;
};

DEFINE_DEVICE_TYPE(OHCI_HLEAN2131SC, ohci_hlean2131sc_device, "ohci_hlean2131sc", "OHCI an2131sc HLE")

#define MCFG_OHCI_HLEAN2131SC_REGION(_region_tag, _region_offset) \
	downcast<ohci_hlean2131sc_device *>(device)->set_region(_region_tag, _region_offset);

/*
 * Class declaration for chihiro_state
 */

class chihiro_state : public xbox_base_state
{
	friend class ide_baseboard_device;

public:
	chihiro_state(const machine_config &mconfig, device_type type, const char *tag)
		: xbox_base_state(mconfig, type, tag)
		, m_ide(*this, "ide1")
		, m_dimmboard(*this, "rom_board")
		, m_hack_index(-1)
		, m_hack_counter(0)
		, m_dimm_board_memory(nullptr)
		, m_dimm_board_memory_size(0) { }

	void chihirogd(machine_config &config);
	void chihiro_base(machine_config &config);

private:
	uint32_t mediaboard_r(offs_t offset, uint32_t mem_mask = ~0);
	void mediaboard_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	virtual void machine_start() override ATTR_COLD;
	void baseboard_ide_event(int type, uint8_t *read, uint8_t *write);
	uint8_t *baseboard_ide_dimmboard(uint32_t lba);
	void dword_write_le(uint8_t *addr, uint32_t d);
	void word_write_le(uint8_t *addr, uint16_t d);
	virtual void hack_eeprom() override;
	virtual void hack_usb() override;

	// devices
	optional_device<bus_master_ide_controller_device> m_ide;
	optional_device<naomi_gdrom_board> m_dimmboard;

	int m_hack_index;
	int m_hack_counter;
	uint8_t *m_dimm_board_memory;
	uint32_t m_dimm_board_memory_size;

	static void an2131qc_configuration(device_t *device);
	static void an2131sc_configuration(device_t *device);
	void chihiro_map(address_map &map) ATTR_COLD;
	void chihiro_map_io(address_map &map) ATTR_COLD;

	void jamtable_disasm(address_space &space, uint32_t address, uint32_t size);
	void jamtable_disasm_command(const std::vector<std::string_view> &params);
	void chihiro_help_command(const std::vector<std::string_view> &params);
	void debug_commands(const std::vector<std::string_view> &params);
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
void chihiro_state::jamtable_disasm(address_space &space, uint32_t address, uint32_t size) // 0xff000080 == fff00080
{
	debugger_console &con = machine().debugger().console();
	offs_t addr = (offs_t)address;
	address_space *tspace;
	if (!space.device().memory().translate(space.spacenum(), device_memory_interface::TR_READ, addr, tspace))
	{
		con.printf("Address is unmapped.\n");
		return;
	}
	while (1)
	{
		offs_t base = addr;

		uint32_t opcode = tspace->read_byte(addr);
		addr++;
		uint32_t op1 = tspace->read_dword_unaligned(addr);
		addr += 4;
		uint32_t op2 = tspace->read_dword_unaligned(addr);
		addr += 4;

		std::string sop1;
		std::string pcrel;
		if (opcode == 0xe1)
		{
			opcode = op2 & 255;
			op2 = op1;
			sop1 = "ACC";
			pcrel = "PC+ACC";
		}
		else
		{
			sop1 = util::string_format("%08X", op1);
			pcrel = util::string_format("%08X", base + 9 + op1);
		}
		con.printf("%08X ", base);
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
			con.printf("POKEPCI PCICONF[%08X]=%s\n", op2, sop1);
			break;
		case 0x02:
			con.printf("OUTB    PORT[%08X]=%s\n", op2, sop1);
			break;
		case 0x03:
			con.printf("POKE    MEM[%08X]=%s\n", op2, sop1);
			break;
		case 0x04:
			con.printf("BNE     IF ACC != %08X THEN PC=%s\n", op2, pcrel);
			break;
		case 0x05:
			// out cf8,op2
			// in acc,cfc
			con.printf("PEEKPCI ACC=PCICONF[%08X]\n", op2);
			break;
		case 0x06:
			con.printf("AND/OR  ACC=(ACC & %08X) | %s\n", op2, sop1);
			break;
		case 0x07:
			con.printf("BRA     PC=%s\n", pcrel);
			break;
		case 0x08:
			con.printf("INB     ACC=PORT[%08X]\n", op2);
			break;
		case 0x09:
			con.printf("PEEK    ACC=MEM[%08X]\n", op2);
			break;
		case 0xee:
			con.printf("END\n");
			break;
		default:
			con.printf("NOP     ????\n");
			break;
		}
		if (opcode == 0xee)
			break;
		if (size <= 9)
			break;
		size -= 9;
	}
}

void chihiro_state::jamtable_disasm_command(const std::vector<std::string_view> &params)
{
	address_space &space = m_maincpu->space();
	uint64_t  addr, size;

	if (params.size() < 3)
		return;
	if (!machine().debugger().console().validate_number_parameter(params[1], addr))
		return;
	if (!machine().debugger().console().validate_number_parameter(params[2], size))
		return;
	jamtable_disasm(space, (uint32_t)addr, (uint32_t)size);
}

void chihiro_state::chihiro_help_command(const std::vector<std::string_view> &params)
{
	debugger_console &con = machine().debugger().console();

	con.printf("Available Chihiro commands:\n");
	con.printf("  chihiro jamdis,<start>,<size> -- Disassemble <size> bytes of JamTable instructions starting at <start>\n");
	con.printf("  chihiro help -- this list\n");
}

void chihiro_state::debug_commands(const std::vector<std::string_view> &params)
{
	if (params.size() < 1)
		return;
	if (params[0] == "jamdis")
		jamtable_disasm_command(params);
	else
		chihiro_help_command(params);
}

void chihiro_state::hack_eeprom()
{
	// 8003b744,3b744=0x90 0x90
	m_maincpu->space(AS_PROGRAM).write_byte(0x3b744, 0x90);
	m_maincpu->space(AS_PROGRAM).write_byte(0x3b745, 0x90);
	m_maincpu->space(AS_PROGRAM).write_byte(0x3b766, 0xc9);
	m_maincpu->space(AS_PROGRAM).write_byte(0x3b767, 0xc3);
}

#define HACK_ITEMS 5
static const struct
{
	const char *game_name;
	struct {
		uint32_t address;
		uint8_t write_byte;
	} modify[16];
} hacks[HACK_ITEMS] = { { "chihiro",  { { 0, 0 } } },
						{ "outr2",    { { 0, 0 } } },
						{ "crtaxihr", { { 0x14ada5/*11fda5*/, 0x90 }, { 0x14ada6/*11fda6*/, 0x90 }, { 0, 0 } } },
						{ "ghostsqu", { { 0x78833/*4d833*/, 0x90 }, { 0x78834/*4d834*/, 0x90 }, { 0, 0 } } },
						{ "vcop3",    { { 0x61a23/*36a23*/, 0x90 }, { 0x61a24/*36a24*/, 0x90 }, { 0, 0 } } },
};

void chihiro_state::hack_usb()
{
	int p;

	if (m_hack_counter == 1)
		p = m_hack_index; // need to patch the game
	else
		p = -1;
	if (p >= 0) {
		for (int a = 0; a < 16; a++) {
			if (hacks[p].modify[a].address == 0)
				break;
			m_maincpu->space(0).write_byte(hacks[p].modify[a].address, hacks[p].modify[a].write_byte);
		}
	}
	m_hack_counter++;
}

//**************************************************************************
//  BASE BOARD USB
//**************************************************************************

//ic10
const USBStandardDeviceDescriptor ohci_hlean2131qc_device::devdesc = { 0x12,0x01,0x0100,0x60,0x00,0x00,0x40,0x0CA3,0x0002,0x0108,0x01,0x02,0x00,0x01 };  // class 0x60 subclass 0x00
const USBStandardConfigurationDescriptor ohci_hlean2131qc_device::condesc = { 0x09,0x02,0x0058,0x01,0x01,0x00,0x80,0x96 };
const USBStandardInterfaceDescriptor ohci_hlean2131qc_device::intdesc = { 0x09,0x04,0x00,0x00,0x0A,0xFF,0x00,0x00,0x00 };
const USBStandardEndpointDescriptor ohci_hlean2131qc_device::enddesc01 = { 0x07,0x05,0x01,0x02,0x0040,0x00 };
const USBStandardEndpointDescriptor ohci_hlean2131qc_device::enddesc02 = { 0x07,0x05,0x02,0x02,0x0040,0x00 };
const USBStandardEndpointDescriptor ohci_hlean2131qc_device::enddesc03 = { 0x07,0x05,0x03,0x02,0x0040,0x00 };
const USBStandardEndpointDescriptor ohci_hlean2131qc_device::enddesc04 = { 0x07,0x05,0x04,0x02,0x0040,0x00 };
const USBStandardEndpointDescriptor ohci_hlean2131qc_device::enddesc05 = { 0x07,0x05,0x05,0x02,0x0040,0x00 };
const USBStandardEndpointDescriptor ohci_hlean2131qc_device::enddesc81 = { 0x07,0x05,0x81,0x02,0x0040,0x00 };
const USBStandardEndpointDescriptor ohci_hlean2131qc_device::enddesc82 = { 0x07,0x05,0x82,0x02,0x0040,0x00 };
const USBStandardEndpointDescriptor ohci_hlean2131qc_device::enddesc83 = { 0x07,0x05,0x83,0x02,0x0040,0x00 };
const USBStandardEndpointDescriptor ohci_hlean2131qc_device::enddesc84 = { 0x07,0x05,0x84,0x02,0x0040,0x00 };
const USBStandardEndpointDescriptor ohci_hlean2131qc_device::enddesc85 = { 0x07,0x05,0x85,0x02,0x0040,0x00 };
const uint8_t ohci_hlean2131qc_device::strdesc0[] = { 0x04,0x03,0x00,0x00 };
const uint8_t ohci_hlean2131qc_device::strdesc1[] = { 0x0A,0x03,0x53,0x00,0x45,0x00,0x47,0x00,0x41,0x00 };
const uint8_t ohci_hlean2131qc_device::strdesc2[] = { 0x0E,0x03,0x42,0x00,0x41,0x00,0x53,0x00,0x45,0x00,0x42,0x03,0xFF,0x0B };

ohci_hlean2131qc_device::ohci_hlean2131qc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, OHCI_HLEAN2131QC, tag, owner, clock)
	, device_usb_ohci_function_interface(mconfig, *this)
	, m_jvs_master(*this, "^^^^jvs_master")
{
	maximum_send = 0;
	region_tag = nullptr;
	region_offset = 0;
	region = nullptr;
	jvs.buffer_in_expected = 0;
	jvs.buffer_out_used = 0;
	jvs.buffer_out_packets = 0;
}

void ohci_hlean2131qc_device::initialize()
{
	device_usb_ohci_function_interface::initialize();
	add_device_descriptor(devdesc);
	add_configuration_descriptor(condesc);
	add_interface_descriptor(intdesc);
	// it is important to add the endpoints in the same order they are found in the device firmware
	add_endpoint_descriptor(enddesc01);
	add_endpoint_descriptor(enddesc02);
	add_endpoint_descriptor(enddesc03);
	add_endpoint_descriptor(enddesc04);
	add_endpoint_descriptor(enddesc05);
	add_endpoint_descriptor(enddesc81);
	add_endpoint_descriptor(enddesc82);
	add_endpoint_descriptor(enddesc83);
	add_endpoint_descriptor(enddesc84);
	add_endpoint_descriptor(enddesc85);
	add_string_descriptor(strdesc0);
	add_string_descriptor(strdesc1);
	add_string_descriptor(strdesc2);
}

void ohci_hlean2131qc_device::set_region_base(uint8_t *data)
{
	region = data;
}

void ohci_hlean2131qc_device::set_region(const char *_region_tag, int _region_offset)
{
	region_tag = _region_tag;
	region_offset = _region_offset;
}

int ohci_hlean2131qc_device::handle_nonstandard_request(int endpoint, USBSetupPacket *setup)
{
	int sense;

	LOGMASKED(LOG_EXTRA, "Control request to an2131qc: %x %x %x %x %x %x %x\n\r", endpoint, endpoints[endpoint].controldirection, setup->bmRequestType, setup->bRequest, setup->wValue, setup->wIndex, setup->wLength);
	if (endpoint != 0)
		return -1;
	// default valuse for data stage
	for (int n = 0; n < setup->wLength; n++)
		endpoints[endpoint].buffer[n] = 0x50 ^ n;
	sense = m_jvs_master->get_sense_line();
	if (sense == 25)
		sense = 3;
	else
		sense = 0; // need to check
	// PINSA register, bits 0-2 connected do dip switches 1-3 on filter board, bit 4 to dip switch 4, bit 5 to dip switch 5, bits 6-7 to buttons 1-2 on filter board
	// bits 4-1 value must be 10 xor 15, and bit 3 is ignored since its used as the CS pin of the chip
	endpoints[endpoint].buffer[1] = 0x4b;
	// PINSB register, bits 5-7 connected to 3 leds not mounted on pcb, bit 4 connected to re/de pins of max485, bits 2-3 used as uart pins, bits 0-1 give the status of the sense pin of the jvs connector
	// if bits 0-1 are 11, the not all the connected jvs devices have been assigned an address yet
	endpoints[endpoint].buffer[2] = 0x52 | sense;
	// OUTB register
	endpoints[endpoint].buffer[3] = 0x53;
	// bRequest is a command value
	if (setup->bRequest == 0x16)
	{
		// this command is used to read data from the first i2c serial eeprom connected to the chip
		// setup->wValue = start address to read from
		// setup->wIndex = number of bytes to read
		// data will be transferred to the host using endpoint 1 (IN)
		endpoints[1].remain = setup->wIndex & 255;
		endpoints[1].position = region + setup->wValue; // usually wValue is 0x1f00
		endpoints[endpoint].buffer[0] = 0;
	}
	else if (setup->bRequest == 0x17)
	{
		// this command is used to read data from the second i2c serial eeprom connected to the chip
		// setup->wValue = start address to read from
		// setup->wIndex = number of bytes to read
		// data will be transferred to the host using endpoint 2 (IN)
		endpoints[2].remain = setup->wIndex & 255;
		endpoints[2].position = region + 0x2000 + setup->wValue;
		endpoints[endpoint].buffer[0] = 0;
	}
	else if (setup->bRequest == 0x18)
	{
		// this command is used to read data from external memory (with respect to the internal 8051 cpu)
		// setup->wValue = start address to read from
		// setup->wIndex = number of bytes to read
		// data will be transferred to the host using endpoint 3 (IN)
		endpoints[endpoint].buffer[0] = 0;
	}
	else if (setup->bRequest == 0x19)
	{
		// this command is used to retreive the jvs packets that have been received in response to the ones of 0x20
		// data for the packets will be transferred to the host using endpoint 4 (IN)
		// the nuber of bytes to transfer is returned at bytes 4 and 5 in the data stage of this control transfer
		// data transferred starts with a byte with value 0, then a byte with value the number of packets received, then a block of bytes for each packet
		// the bytes for a packet start with the jvs node address of the sender, then a dummy one (must be 0), then a 16 bit number in little endian format that specifies how many bytes follow
		// the bytes that follow contain the body of the packet as received from the jvs bus, from the 0xa0 byte to the checksum
		endpoints[endpoint].buffer[0] = 0; // 0 if not busy
		endpoints[endpoint].buffer[5] = jvs.buffer_out_used >> 8; // amount to transfer with endpoint 4
		endpoints[endpoint].buffer[4] = (jvs.buffer_out_used & 0xff);
		// the data to be sent is prepared in command 0x20
		endpoints[4].remain = jvs.buffer_out_used;
		endpoints[4].position = jvs.buffer_out;
		jvs.buffer_out_used = 0;
		endpoints[endpoint].buffer[0] = 0;
	}
	else if (setup->bRequest == 0x1c)
	{
		// this command is used to read from the RV5C386A chip
		// setup->wValue = what to read
		// setup->wIndex = number of bytes to read
		// data will be transferred to the host using endpoint 5 (IN)
		endpoints[endpoint].buffer[0] = 0;
	}
	else if (setup->bRequest == 0x1d)
	{
		// this command is used to write data to the first i2c serial eeprom connected to the chip
		// no more than 32 bytes can be written at a time
		// setup->wValue = start address to write to
		// setup->wIndex = number of bytes to write
		// data will be transferred from the host using endpoint 1 (OUT)
		endpoints[endpoint].buffer[0] = 0;
	}
	else if (setup->bRequest == 0x1e)
	{
		// this command is used to write data to the second i2c serial eeprom connected to the chip
		// no more than 8 bytes can be written at a time
		// setup->wValue = start address to write to
		// setup->wIndex = number of bytes to write
		// data will be transferred from the host using endpoint 2 (OUT)
		endpoints[endpoint].buffer[0] = 0;
	}
	else if (setup->bRequest == 0x1f)
	{
		// this command is used to write data to external memory (with respect to the internal 8051 cpu)
		// setup->wValue = start address to write to
		// setup->wIndex = number of bytes to write
		// data will be transferred from the host using endpoint 3 (OUT)
		endpoints[endpoint].buffer[0] = 0;
	}
	else if (setup->bRequest == 0x20)
	{
		// this command is used to send a set of jvs packets, each to a different node
		// for each packet sent, the respective answer will be stored, and can be retrieved with 0x19
		// setup->wIndex = number of bytes to be sent by the host
		// data for the packets will be transferred from the host using endpoint 4 (OUT)
		// data sent by the host contains first a byte with value 0 that is ignored, then a byte specifying the number of packets that follow, then the data for each packet
		// the data for each packet contains first a byte with value 0, then the sync byte (0xe0) then all the other bytes of the packet ending with the checksum byte
		// broadcast packets must have a destination node address of value 0xff
		LOGMASKED(LOG_EXTRA, " Jvs packets data of %d bytes\n\r", setup->wIndex);
		endpoints[endpoint].buffer[0] = 0;
		if (jvs.buffer_out_used == 0)
		{
			jvs.buffer_out_packets = 0;
			jvs.buffer_out[0] = 0;
			jvs.buffer_out[1] = (uint8_t)jvs.buffer_out_packets;
			jvs.buffer_out_used = 2;
		}
		jvs.buffer_in_expected = setup->wIndex;
		endpoints[4].remain = jvs.buffer_in_expected;
		endpoints[4].position = jvs.buffer_in;
	}
	else if (setup->bRequest == 0x24)
	{
		// this command is used to write to the RV5C386A chip
		// no more than 0x20 bytes can be written
		// setup->wValue = what to read
		// data will be transferred from the host using endpoint 5 (OUT)
		endpoints[endpoint].buffer[0] = 0;
	}
	else if (setup->bRequest == 0x30)
	{
		// this command first disables external interrupt 0 if the lower 8 bits of setup->wValue are 0
		// or enables it if those bits, seen as a signed 8 bit value, represent a number greater than 0
		// then it will return in byte 4 of the data stage the value 0 if external interrupt 0 has been disabled or value 1 if it has been enabled
		// and in byte 5 the value of an 8 bit counter that is incremented at every external interrupt 0
		endpoints[endpoint].buffer[0] = 0;
		if ((setup->wValue & 255) == 0)
			endpoints[endpoint].buffer[4] = 0;
		else if ((setup->wValue & 255) < 128)
			endpoints[endpoint].buffer[4] = 1;
		endpoints[endpoint].buffer[5] = 0;
	} else
		endpoints[endpoint].buffer[0] = 0x99; // usnupported command

	endpoints[endpoint].position = endpoints[endpoint].buffer;
	endpoints[endpoint].remain = setup->wLength;
	return 0;
}

int ohci_hlean2131qc_device::handle_bulk_pid(int endpoint, int pid, uint8_t *buffer, int size)
{
	LOGMASKED(LOG_EXTRA, "Bulk request to an2131qc: %x %d %x\n\r", endpoint, pid, size);
	if (((endpoint == 1) || (endpoint == 2)) && (pid == InPid))
	{
		if (size > endpoints[endpoint].remain)
			size = endpoints[endpoint].remain;
		memcpy(buffer, endpoints[endpoint].position, size);
		endpoints[endpoint].position = endpoints[endpoint].position + size;
		endpoints[endpoint].remain = endpoints[endpoint].remain - size;
	}
	if ((endpoint == 4) && (pid == InPid))
	{
		if (size > endpoints[4].remain)
			size = endpoints[4].remain;
		memcpy(buffer, endpoints[4].position, size);
		endpoints[4].position = endpoints[4].position + size;
		endpoints[4].remain = endpoints[4].remain - size;
	}
	if ((endpoint == 4) && (pid == OutPid))
	{
		if (size > endpoints[4].remain)
			size = endpoints[4].remain;
		for (int n = 0; n < size; n++)
			LOGMASKED(LOG_EXTRA, " %02x", buffer[n]);
		if (size > 0) {
			memcpy(endpoints[4].position, buffer, size);
			endpoints[4].position = endpoints[4].position + size;
			endpoints[4].remain = endpoints[4].remain - size;
			if (endpoints[4].remain == 0)
			{
				LOGMASKED(LOG_EXTRA, "\n\r");
				// extract packets
				process_jvs_packet();
			}
		}
	}
	return size;
}

void ohci_hlean2131qc_device::process_jvs_packet()
{
	int numpk = jvs.buffer_in[1];
	int p = 2;

	for (int n = 0; n < numpk; n++)
	{
		p++;
		if (jvs.buffer_in[p] != 0xe0)
			break;
		p++;
		int dest = jvs.buffer_in[p];
		p++;
		int len = jvs.buffer_in[p];
		p++;
		if ((p + len) > jvs.buffer_in_expected)
			break;
		int chk = dest + len;
		for (int m = len - 1; m > 0; m--)
			chk = chk + (int)jvs.buffer_in[p + m - 1];
		chk = chk & 255;
		if (chk != (int)jvs.buffer_in[p + len - 1])
		{
			p = p + len;
			continue;
		}
		// use data of this packet
		m_jvs_master->send_packet(dest, len, jvs.buffer_in + p);
		// generate response
		if (dest == 0xff)
			dest = 0;
		int recv = m_jvs_master->received_packet(jvs.buffer_out + jvs.buffer_out_used + 5);
		// update buffer_out
		if (recv > 0)
		{
			chk = 0;
			for (int m = 0; m < recv; m++)
				chk = chk + jvs.buffer_out[jvs.buffer_out_used + 5 + m];
			jvs.buffer_out[jvs.buffer_out_used + 5 + recv] = chk & 255;
			jvs.buffer_out_packets++;
			// jvs node address
			jvs.buffer_out[jvs.buffer_out_used] = (uint8_t)dest;
			// dummy
			jvs.buffer_out[jvs.buffer_out_used + 1] = 0;
			// length following
			recv += 2;
			jvs.buffer_out[jvs.buffer_out_used + 2] = recv & 255;
			jvs.buffer_out[jvs.buffer_out_used + 3] = (recv >> 8) & 255;
			// body
			jvs.buffer_out[jvs.buffer_out_used + 4] = 0xe0;
			jvs.buffer_out_used = jvs.buffer_out_used + recv + 5 - 1;
			jvs.buffer_out[1] = (uint8_t)jvs.buffer_out_packets;
		}
		p = p + len;
	}
}

void ohci_hlean2131qc_device::device_start()
{
	initialize();
	if (region_tag)
		set_region_base(memregion(region_tag)->base() + region_offset);
}

//pc20
const USBStandardDeviceDescriptor ohci_hlean2131sc_device::devdesc = { 0x12,0x01,0x0100,0x60,0x01,0x00,0x40,0x0CA3,0x0003,0x0110,0x01,0x02,0x00,0x01 }; // class 0x60 subclass 0x01
const USBStandardConfigurationDescriptor ohci_hlean2131sc_device::condesc = { 0x09,0x02,0x003C,0x01,0x01,0x00,0x80,0x96 };
const USBStandardInterfaceDescriptor ohci_hlean2131sc_device::intdesc = { 0x09,0x04,0x00,0x00,0x06,0xFF,0x00,0x00,0x00 };
const USBStandardEndpointDescriptor ohci_hlean2131sc_device::enddesc01 = { 0x07,0x05,0x01,0x02,0x0040,0x00 };
const USBStandardEndpointDescriptor ohci_hlean2131sc_device::enddesc02 = { 0x07,0x05,0x02,0x02,0x0040,0x00 };
const USBStandardEndpointDescriptor ohci_hlean2131sc_device::enddesc03 = { 0x07,0x05,0x03,0x02,0x0040,0x00 };
const USBStandardEndpointDescriptor ohci_hlean2131sc_device::enddesc81 = { 0x07,0x05,0x81,0x02,0x0040,0x00 };
const USBStandardEndpointDescriptor ohci_hlean2131sc_device::enddesc82 = { 0x07,0x05,0x82,0x02,0x0040,0x00 };
const USBStandardEndpointDescriptor ohci_hlean2131sc_device::enddesc83 = { 0x07,0x05,0x83,0x02,0x0040,0x00 };
const uint8_t ohci_hlean2131sc_device::strdesc0[] = { 0x04,0x03,0x00,0x00 };
const uint8_t ohci_hlean2131sc_device::strdesc1[] = { 0x0A,0x03,0x53,0x00,0x45,0x00,0x47,0x00,0x41,0x00 };
const uint8_t ohci_hlean2131sc_device::strdesc2[] = { 0x0E,0x03,0x42,0x00,0x41,0x00,0x53,0x00,0x45,0x00,0x42,0x00,0x44,0x00 };

ohci_hlean2131sc_device::ohci_hlean2131sc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, OHCI_HLEAN2131SC, tag, owner, clock)
	, device_usb_ohci_function_interface(mconfig, *this)
{
	region = nullptr;
	region_tag = nullptr;
	region_offset = 0;
	midi_rs232 = 0;
	response_size = 0;
	step = 0;
}

void ohci_hlean2131sc_device::set_region_base(uint8_t *data)
{
	region = data;
}

void ohci_hlean2131sc_device::set_region(const char *_region_tag, int _region_offset)
{
	region_tag = _region_tag;
	region_offset = _region_offset;
}

void ohci_hlean2131sc_device::initialize()
{
	device_usb_ohci_function_interface::initialize();
	add_device_descriptor(devdesc);
	add_configuration_descriptor(condesc);
	add_interface_descriptor(intdesc);
	// it is important to add the endpoints in the same order they are found in the device firmware
	add_endpoint_descriptor(enddesc01);
	add_endpoint_descriptor(enddesc02);
	add_endpoint_descriptor(enddesc03);
	add_endpoint_descriptor(enddesc81);
	add_endpoint_descriptor(enddesc82);
	add_endpoint_descriptor(enddesc83);
	add_string_descriptor(strdesc0);
	add_string_descriptor(strdesc1);
	add_string_descriptor(strdesc2);
}

int ohci_hlean2131sc_device::handle_nonstandard_request(int endpoint, USBSetupPacket *setup)
{
	LOGMASKED(LOG_EXTRA, "Control request to an2131sc: %x %x %x %x %x %x %x\n\r", endpoint, endpoints[endpoint].controldirection, setup->bmRequestType, setup->bRequest, setup->wValue, setup->wIndex, setup->wLength);
	if (endpoint != 0)
		return -1;
	// default valuse for data stage
	for (int n = 0; n < setup->wLength; n++)
		endpoints[endpoint].buffer[n] = 0x50 ^ n;
	endpoints[endpoint].buffer[1] = 0;
	// PINSB register, bits 0-1 uset as rts/cts signals for uart0, bits 2-3 used as uart1, bits 5-7 used as leds (not mounted on pcb)
	endpoints[endpoint].buffer[2] = 0x52; // PINSB
	// OUTB register
	endpoints[endpoint].buffer[3] = 0x53; // OUTB
	endpoints[endpoint].buffer[4] = 0;
	endpoints[endpoint].buffer[5] = 0;
	// PINSC register, bit 7 selects default value for bit 6 after reset (connected to dip switch 5 on filter board), bits 0-1 used as uart0 pins
	endpoints[endpoint].buffer[6] = 0x56;
	// OUTC register, bit 6 specifies if uart1 should be connected to midi or rs232
	endpoints[endpoint].buffer[7] = 0x57;
	// bRequest is a command value
	if (setup->bRequest == 0x16)
	{
		// this command is used to read data from the i2c serial eeprom connected to the chip
		// setup->wValue = start address to read from
		// setup->wIndex = number of bytes to read
		// data will be transferred to the host using endpoint 1 (IN)
		endpoints[1].remain = setup->wIndex & 255;
		endpoints[1].position = region + setup->wValue; // usually wValue is 0x1f00
		endpoints[endpoint].buffer[0] = 0;
	}
	else if (setup->bRequest == 0x17)
	{
		endpoints[endpoint].buffer[0] = 0x90;
	}
	else if (setup->bRequest == 0x1a)
	{
		// used to get data received by uart0 data transferred with endpoint 2 (IN)
		endpoints[endpoint].buffer[0] = 0x99;
	}
	else if (setup->bRequest == 0x1b)
	{
		// used to get data received by uart 1 data transferred with endpoint 3 (IN)
		// setup->wIndex = number of bytes host would like to retrieve (must be lower than 256, 0 means get all available)
		endpoints[endpoint].buffer[0] = 0; //
		endpoints[endpoint].buffer[1] = 0; // bit 0 in buffer overflow bit 1 in parity error
		if ((setup->wIndex == 0) || (setup->wIndex > response_size))
		{
			endpoints[endpoint].buffer[4] = response_size & 255; // how many bytes to transfer with endpoint 3
			endpoints[endpoint].buffer[5] = (response_size >> 8) & 255;
			memcpy(endpoints[3].buffer, response, response_size);
			response_size = 0;
		}
		else
		{
			endpoints[endpoint].buffer[4] = setup->wIndex & 255;
			endpoints[endpoint].buffer[5] = (setup->wIndex >> 8) & 255;
			memcpy(endpoints[3].buffer, response, setup->wIndex);
			for (int n = setup->wIndex; n < response_size; n++)
				response[n - setup->wIndex] = response[n];
			response_size = response_size - setup->wIndex;
		}
		endpoints[3].remain = setup->wIndex;
		endpoints[3].position = endpoints[3].buffer;
	}
	else if (setup->bRequest == 0x1d)
	{
		// this command is used to write data to the i2c serial eeprom connected to the chip
		// no more than 32 bytes can be written at a time
		// setup->wValue = start address to write to
		// setup->wIndex = number of bytes to write
		// data will be transferred from the host using endpoint 1 (OUT)
		endpoints[endpoint].buffer[0] = 0;
	}
	else if (setup->bRequest == 0x1e)
	{
		endpoints[endpoint].buffer[0] = 0x90;
	}
	else if (setup->bRequest == 0x22)
	{
		// send data to be output from uart0
		endpoints[endpoint].buffer[0] = 0x99;
	}
	else if (setup->bRequest == 0x23)
	{
		// this command is used to send data to be output from uart1
		// setup->wIndex = number of bytes to send
		// data will be transferred to the host using endpoint 3 (OUT)
		endpoints[endpoint].buffer[0] = 0;
		endpoints[3].remain = setup->wIndex;
		endpoints[3].position = endpoints[3].buffer;
	}
	else if (setup->bRequest == 0x25) //
	{
		endpoints[endpoint].buffer[0] = 0;
	}
	else if (setup->bRequest == 0x26) //
	{
		// set uart0 or uart1 mode
		endpoints[endpoint].buffer[0] = 0;
	}
	else if (setup->bRequest == 0x27) //
	{
		endpoints[endpoint].buffer[0] = 0;
	}
	else if (setup->bRequest == 0x28)
	{
		endpoints[endpoint].buffer[0] = 0x99;
	}
	else if (setup->bRequest == 0x29)
	{
		endpoints[endpoint].buffer[0] = 0x99;
	}
	else if (setup->bRequest == 0x2a)
	{
		endpoints[endpoint].buffer[0] = 0x99;
	}
	else if (setup->bRequest == 0x2b)
	{
		endpoints[endpoint].buffer[0] = 0x99;
	}
	else if (setup->bRequest == 0x2c)
	{
		endpoints[endpoint].buffer[0] = 0x99;
	}
	else if (setup->bRequest == 0x2d)
	{
		// set pin 1 of PORTB, used as the RTS signal of the rs232 port implemented with uart0
		endpoints[endpoint].buffer[0] = 0x99;
	}
	else if (setup->bRequest == 0x2e)
	{
		// switches uart1 between rs232 mode and midi mode
		// low byte of wValue sets the mode: 0 mode is selected using pin 7 of PORTC (0 midi 1 rs232), 1 rs232, 2 midi, other values no mode change
		endpoints[endpoint].buffer[0] = 0;
		midi_rs232 = setup->wValue & 255;
	}
	else if (setup->bRequest == 0x2f)
	{
		// return low byte of wValue from command 0x2e
		endpoints[endpoint].buffer[0] = 0;
		endpoints[endpoint].buffer[4] = midi_rs232;
	}
	else if (setup->bRequest == 0x30)
	{
		// this command first disables external interrupt 0 if the lower 8 bits of setup->wValue are 0
		// or enables it if those bits, seen as a signed 8 bit value, represent a number greater than 0
		// then it will return in byte 4 of the data stage the value 0 if external interrupt 0 has been disabled or value 1 if it has been enabled
		// and in byte 5 the value of an 8 bit counter that is incremented at every external interrupt 0
		endpoints[endpoint].buffer[0] = 0;
		if ((setup->wValue & 255) == 0)
			endpoints[endpoint].buffer[4] = 0;
		else if ((setup->wValue & 255) < 128)
			endpoints[endpoint].buffer[4] = 1;
		endpoints[endpoint].buffer[5] = 0;
	}
	else if (setup->bRequest == 0x31)
	{
		// set pins 4-7 of PORTB
		// bits 4-7 of wValue & 255 set the direction
		// bits 4-7 of wValue >> 8 set the level
		endpoints[endpoint].buffer[0] = 0;
	} else
		endpoints[endpoint].buffer[0] = 0x99; // usnupported command

	endpoints[endpoint].position = endpoints[endpoint].buffer;
	endpoints[endpoint].remain = setup->wLength;
	return 0;
}

int ohci_hlean2131sc_device::handle_bulk_pid(int endpoint, int pid, uint8_t *buffer, int size)
{
	LOGMASKED(LOG_EXTRA, "Bulk request to an2131sc: %x %d %x\n\r", endpoint, pid, size);
	if (((endpoint == 1) || (endpoint == 2)) && (pid == InPid))
	{
		if (size > endpoints[endpoint].remain)
			size = endpoints[endpoint].remain;
		memcpy(buffer, endpoints[endpoint].position, size);
		endpoints[endpoint].position = endpoints[endpoint].position + size;
		endpoints[endpoint].remain = endpoints[endpoint].remain - size;
	}
	if ((endpoint == 3) && (pid == InPid))
	{
		if (size > endpoints[3].remain)
			size = endpoints[3].remain;
		memcpy(buffer, endpoints[3].position, size);
		endpoints[3].position = endpoints[3].position + size;
		endpoints[3].remain = endpoints[3].remain - size;
	}
	if ((endpoint == 3) && (pid == OutPid))
	{
		if (size > endpoints[3].remain)
			size = endpoints[3].remain;
		memcpy(endpoints[3].position, buffer, size);
		endpoints[3].position = endpoints[3].position + size;
		endpoints[3].remain = endpoints[3].remain - size;
		for (int n = 0; n < size; n++)
		{
			uint8_t byt = buffer[n];
			switch(step)
			{
			case 0:
				if ((byt & 0x80) != 0)
				{
					packet[0] = byt;
					step = 1;
				}
				break;
			case 1:
				packet[1] = byt;
				step = 2;
				break;
			case 2:
				packet[2] = byt;
				step = 3;
				break;
			case 3:
				packet[3] = byt;
				step = 0;
				process_packet();
				break;
			}
		}
	}
	return size;
}

void ohci_hlean2131sc_device::process_packet()
{
	uint8_t result = 0;
	LOGMASKED(LOG_EXTRA, "%02X %02X %02X %02X\n\r", packet[0], packet[1], packet[2], packet[3]);
	if (packet[0] == 0xff) // 00 00 7f
		result = 2;
	else if (packet[0] == 0x81) // 30 7f 4e
		result = 1; // must be 1
	else if (packet[0] == 0xfc) // 00 20 5c
		result = 3;
	else if (packet[0] == 0xfd) // 00 00 7d
		result = 0;
	else if (packet[0] == 0xfa) // 00 1f 65
		result = 0;
	else if (packet[0] == 0x83) // 40 04 47
		result = 0;
	else if (packet[0] == 0x86) // 01 02 05
		result = 0;
	else if (packet[0] == 0x88) // 00 04 0c
		result = 0;
	else if (packet[0] == 0x80) // 01 01 00
		result = 0;
	else if (packet[0] == 0x84) // 01 00 05
		result = 0;
	else if (packet[0] == 0xf0) // 00 00 70
		result = 0;
	else if (packet[0] == 0x9d)
		result = 0;
	else if (packet[0] == 0x9e)
		result = 0;
	if (response_size < 256)
	{
		response[response_size] = result + (result << 4);
		response_size++;
	}
}

void ohci_hlean2131sc_device::device_start()
{
	initialize();
	if (region_tag)
		set_region_base(memregion(region_tag)->base() + region_offset);
}

// ======================> ide_baseboard_device

class ide_baseboard_device : public ata_mass_storage_device_base, public device_ata_interface
{
public:
	// construction/destruction
	ide_baseboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_ata_interface implementation
	virtual uint16_t read_dma() override { return dma_r(); }
	virtual uint16_t read_cs0(offs_t offset, uint16_t mem_mask) override { return command_r(offset); }
	virtual uint16_t read_cs1(offs_t offset, uint16_t mem_mask) override { return control_r(offset); }

	virtual void write_dma(uint16_t data) override { dma_w(data); }
	virtual void write_cs0(offs_t offset, uint16_t data, uint16_t mem_mask) override { command_w(offset, data); }
	virtual void write_cs1(offs_t offset, uint16_t data, uint16_t mem_mask) override { control_w(offset, data); }

	virtual void write_dmack(int state) override { set_dmack_in(state); }
	virtual void write_csel(int state) override { set_csel_in(state); }
	virtual void write_dasp(int state) override { set_dasp_in(state); }
	virtual void write_pdiag(int state) override { set_pdiag_in(state); }

	// ata_mass_storage_device_base implementation
	virtual int  read_sector(uint32_t lba, void *buffer) override;
	virtual int  write_sector(uint32_t lba, const void *buffer) override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	uint8_t read_buffer[0x20]{};
	uint8_t write_buffer[0x20]{};
	chihiro_state *chihirosystem{};
	static const int size_factor = 2;

private:
	// ata_hle_device_base implementation
	virtual void set_irq_out(int state) override { device_ata_interface::set_irq(state); }
	virtual void set_dmarq_out(int state) override { device_ata_interface::set_dmarq(state); }
	virtual void set_dasp_out(int state) override { device_ata_interface::set_dasp(state); }
	virtual void set_pdiag_out(int state) override { device_ata_interface::set_pdiag(state); }
};

//**************************************************************************
//  IDE HARD DISK IMAGE DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(IDE_BASEBOARD, ide_baseboard_device, "ide_baseboard", "IDE Baseboard")

//-------------------------------------------------
//  ide_baseboard_device - constructor
//-------------------------------------------------

ide_baseboard_device::ide_baseboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ata_mass_storage_device_base(mconfig, IDE_BASEBOARD, tag, owner, clock)
	, device_ata_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ide_baseboard_device::device_start()
{
	ata_mass_storage_device_base::device_start();
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

	ata_mass_storage_device_base::device_reset();
}

int ide_baseboard_device::read_sector(uint32_t lba, void *buffer)
{
	int off;
	uint8_t *data;

	/*
	It assumes there are 4 "partitions", the size of the first one depends on bits 3-0 of io port 40f4:
	Value Size lba
	0     0x40000-0x8000
	1     0x80000-0x8000
	2     0x100000-0x8000
	3     0x200000-0x8000
	4     0x400000-0x8000
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
		data = memregion(":mediaboard")->base();
		memcpy(buffer, data + off, 512);
		return 1;
	}
	if (lba >= ((0x40000 << size_factor) - 0x8000)) {
		memset(buffer, 0, 512);
		lba = lba - ((0x40000 << size_factor) - 0x8000);
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

int ide_baseboard_device::write_sector(uint32_t lba, const void *buffer)
{
	logerror("baseboard: write sector lba %08x\n", lba);
	if (lba >= ((0x40000 << size_factor) - 0x8000)) {
		lba = lba - ((0x40000 << size_factor) - 0x8000);
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

void chihiro_state::dword_write_le(uint8_t *addr, uint32_t d)
{
	addr[0] = d & 255;
	addr[1] = (d >> 8) & 255;
	addr[2] = (d >> 16) & 255;
	addr[3] = (d >> 24) & 255;
}

void chihiro_state::word_write_le(uint8_t *addr, uint16_t d)
{
	addr[0] = d & 255;
	addr[1] = (d >> 8) & 255;
}

void chihiro_state::baseboard_ide_event(int type, uint8_t *read_buffer, uint8_t *write_buffer)
{
	int c;

	if ((type != 3) || ((write_buffer[0] == 0) && (write_buffer[1] == 0)))
		return;

	LOGMASKED(LOG_BASEBOARD, "Baseboard sector command:\n");
	for (int a = 0; a < 32; a++)
		LOGMASKED(LOG_BASEBOARD, " %02X", write_buffer[a]);
	LOGMASKED(LOG_BASEBOARD, "\n");

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
		word_write_le(read_buffer + 4, 0x1234); // dimm board firmware version (1234 -> 12.34)
		word_write_le(read_buffer + 6, 0x4567); // ?
		break;
	case 0x0102:
		// second dword
		dword_write_le(read_buffer + 4, 0); // bit 16 develop. mode
		break;
	case 0x0103:
		// dwords 1 3 4
		memcpy(read_buffer + 4, "-abc-abc12345678", 16); // dimm board serial number
		break;
	default:
		logerror("Unknown baseboard sector command %04X\n", c);
	}
	// clear
	write_buffer[0] = write_buffer[1] = write_buffer[2] = write_buffer[3] = 0;
	// irq 10 active
	mcpxlpc->irq10(1);
}

uint8_t *chihiro_state::baseboard_ide_dimmboard(uint32_t lba)
{
	// return pointer to memory containing decrypted gdrom data (contains an image of a fatx partition)
	if (m_dimmboard.found())
		return &m_dimm_board_memory[lba * 512];
	return nullptr;
}

uint32_t chihiro_state::mediaboard_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t r;

	logerror("I/O port read %04x mask %08X\n", offset * 4 + 0x4000, mem_mask);
	r = 0;
	if ((offset == 0x1c/4) && ACCESSING_BITS_16_31)
		r = 0x10000000;
	if ((offset == 0x20/4) && ACCESSING_BITS_0_15)
		r = 0x000000a0;
	if ((offset == 0x20/4) && ACCESSING_BITS_16_31)
		r = 0x42580000;
	if ((offset == 0x24/4) && ACCESSING_BITS_0_15)
		r = 0x00004d41;
	if ((offset == 0xf0/4) && ACCESSING_BITS_0_15)
		r = 0x00000000; // bits 15-0 0 if media board present
	if ((offset == 0xf4/4) && ACCESSING_BITS_0_15)
		r = 2; // bits 3-0 size of dimm board memory. 0=128 1=256 2=512 3=1024 Must be 2
	return r;
}

void chihiro_state::mediaboard_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	logerror("I/O port write %04x mask %08X value %08X\n", offset * 4 + 0x4000, mem_mask, data);
	// irq 10
	if ((offset == 0xe0/4) && ACCESSING_BITS_8_15)
		mcpxlpc->irq10(0);
}

void chihiro_state::chihiro_map(address_map &map)
{
	map(0xff000000, 0xff07ffff).rom().region("bios", 0).mirror(0x00f80000);
}

void chihiro_state::chihiro_map_io(address_map &map)
{
	map(0x4000, 0x40ff).rw(FUNC(chihiro_state::mediaboard_r), FUNC(chihiro_state::mediaboard_w));
}

static INPUT_PORTS_START( chihiro )
	PORT_START("TILT")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_TILT)
	PORT_BIT(0x7f, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("P1")
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_8WAY
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_8WAY
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_8WAY
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_8WAY
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_BUTTON2)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_BUTTON5)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_BUTTON6)
	PORT_BIT(0x400f, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("P2")
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_START2)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_PLAYER(2)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_PLAYER(2)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_PLAYER(2)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_PLAYER(2)
	PORT_BIT(0x400f, IP_ACTIVE_HIGH, IPT_UNUSED)

	/* Dummy so we can easily get the analog ch # */
	PORT_START("A0")
	PORT_BIT(0x80ff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("A1")
	PORT_BIT(0x81ff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("A2")
	PORT_BIT(0x82ff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("A3")
	PORT_BIT(0x83ff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("A4")
	PORT_BIT(0x84ff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("A5")
	PORT_BIT(0x85ff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("A6")
	PORT_BIT(0x86ff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("A7")
	PORT_BIT(0x87ff, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

void chihiro_state::machine_start()
{
	xbox_base_state::machine_start();

	if (m_dimmboard.found())
		m_dimm_board_memory = m_dimmboard->memory(m_dimm_board_memory_size);

	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
	{
		using namespace std::placeholders;
		machine().debugger().console().register_command("chihiro", CMDFLAG_NONE, 1, 4, std::bind(&chihiro_state::debug_commands, this, _1));
	}
	m_hack_index = -1;
	for (int a = 1; a < HACK_ITEMS; a++)
		if (machine().basename() == hacks[a].game_name) {
			m_hack_index = a;
			break;
		}
	m_hack_counter = 0;
	// savestates
	save_item(NAME(m_hack_counter));
}

class sega_network_board : public device_t
{
public:
	sega_network_board(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
};

DEFINE_DEVICE_TYPE(SEGA_NETWORK_BOARD, sega_network_board, "seganetw", "Sega Network Board")

sega_network_board::sega_network_board(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SEGA_NETWORK_BOARD, tag, owner, clock)
{
}

void sega_network_board::device_start()
{
}

ROM_START(seganetw)
	ROM_REGION( 0x200000, "seganetw", 0)
	ROM_LOAD16_WORD_SWAP( "ver1305.bin", 0x000000, 0x200000, CRC(a738ea1c) SHA1(45d94d0c39be1cb3db9fab6610a88a550adda4e9) )
ROM_END

const tiny_rom_entry *sega_network_board::device_rom_region() const
{
	return ROM_NAME(seganetw);
}

static void ide_baseboard(device_slot_interface &device)
{
	device.option_add("bb", IDE_BASEBOARD);
}

void usb_baseboard(device_slot_interface &device)
{
	device.option_add("an2131qc", OHCI_HLEAN2131QC);
	device.option_add("an2131sc", OHCI_HLEAN2131SC);
	device.option_add("xbox_controller", OHCI_GAME_CONTROLLER);
}

void chihiro_state::an2131qc_configuration(device_t *device)
{
	MCFG_OHCI_HLEAN2131QC_REGION(":others", 0)
}

void chihiro_state::an2131sc_configuration(device_t *device)
{
	MCFG_OHCI_HLEAN2131SC_REGION(":others", 0x2080)
}

void chihiro_state::chihiro_base(machine_config &config)
{
	xbox_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &chihiro_state::chihiro_map);
	m_maincpu->set_addrmap(AS_IO, &chihiro_state::chihiro_map_io);

	subdevice<ide_controller_32_device>("pci:09.0:ide1")->options(ide_baseboard, nullptr, "bb", true);

	OHCI_USB_CONNECTOR(config, "pci:02.0:port1", usb_baseboard, "an2131qc", true).set_option_machine_config("an2131qc", an2131qc_configuration);
	OHCI_USB_CONNECTOR(config, "pci:02.0:port2", usb_baseboard, "an2131sc", true).set_option_machine_config("an2131sc", an2131sc_configuration);
	OHCI_USB_CONNECTOR(config, "pci:02.0:port3", usb_baseboard, nullptr, false);
	OHCI_USB_CONNECTOR(config, "pci:02.0:port4", usb_baseboard, nullptr, false);

	JVS_MASTER(config, "jvs_master", 0);
	sega_837_13551_device &sega837(SEGA_837_13551(config, "837_13551", 0, "jvs_master"));
	sega837.set_port_tag<0>("TILT");
	sega837.set_port_tag<1>("P1");
	sega837.set_port_tag<2>("P2");
	sega837.set_port_tag<3>("A0");
	sega837.set_port_tag<4>("A1");
	sega837.set_port_tag<5>("A2");
	sega837.set_port_tag<6>("A3");
	sega837.set_port_tag<7>("A4");
	sega837.set_port_tag<8>("A5");
	sega837.set_port_tag<9>("A6");
	sega837.set_port_tag<10>("A7");
	sega837.set_port_tag<11>("OUTPUT");
}

void chihiro_state::chihirogd(machine_config &config)
{
	chihiro_base(config);
	NAOMI_GDROM_BOARD(config, m_dimmboard, 0, ":gdrom", "pic");
	m_dimmboard->irq_callback().set_nop();
	SEGA_NETWORK_BOARD(config, "network", 0);
}

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_BIOS(bios))

#define ROM_LOAD_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_BIOS(bios))

#define CHIHIRO_BIOS \
	ROM_REGION32_LE( 0x80000, "bios", 0) \
	ROM_SYSTEM_BIOS( 0, "bios0", "Chihiro BIOS" ) \
	ROM_LOAD_BIOS( 0,  "chihiro_xbox_bios.bin", 0x000000, 0x80000, CRC(66232714) SHA1(b700b0041af8f84835e45d1d1250247bf7077188) ) \
	ROM_REGION( 0x200000, "mediaboard", 0) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0,  "fpr-23887_29lv160te.ic4", 0x000000, 0x200000, CRC(13034372) SHA1(77197fba2781ed1d81402c48bd743adb26d3161a) ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0,  "fpr21042_m29w160et.bin", 0x000000, 0x200000, CRC(a4fcab0b) SHA1(a13cf9c5cdfe8605d82150b7573652f419b30197) ) \
	ROM_REGION( 0x204080, "others", 0) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0,  "315-6351a_epc1pc8.ic2", 0x0000, 0x1ff01, CRC(488624ea) SHA1(1a607337a186415ae90b48934ce2aae81639d4de) ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0,  "ic10_g24lc64.bin", 0x0000, 0x2000,   CRC(cfc5e06f) SHA1(3ababd4334d8d57abb22dd98bd2d347df39648d9) ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0,  "ic11_24lc024.bin", 0x2000, 0x80,     CRC(8dc8374e) SHA1(cc03a0650bfac4bf6cb66e414bbef121cba53efe) ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0,  "pc20_g24lc64.bin", 0x2080, 0x2000,   CRC(7742ab62) SHA1(82dad6e2a75bab4a4840dc6939462f1fb9b95101) )

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
	DISK_IMAGE_READONLY( "gdx-0001", 0, SHA1(e41a2b236ec26db2d8b07643b8222e64440d1f31) )

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
ROM_START( vcop3a )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0003a", 0, SHA1(04cd12bec50a9e9f1f05e7b7c2ef396800a385dd) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0354-com.data", 0x00, 0x50,  CRC(df7e3217) SHA1(9f0f4bf6b15f3b6eeea81eaa27b3d25bd94110da) )
ROM_END

ROM_START( vcop3 )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0003b", 0, SHA1(4268aadb83c880d4f3dab1d43ddd0a3a2f8befa2) )

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
	DISK_IMAGE_READONLY( "gdx-0006c", 0, SHA1(545ef902833d53822a8544dfc3f7538ee6025c9e) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD( "317-0374-jpn.pic", 0x000000, 0x004000, CRC(004f77a1) SHA1(bc5c6950293f3bff60bf7913d20a2046aa19ea69) )
ROM_END

ROM_START( mj2f )
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
ROM_START( mj2 )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0006g", 0, SHA1(b8c8b440d4cd2488be78e3a002058ea5b176a1f2) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD( "317-0374-jpn.pic", 0x000000, 0x004000, CRC(004f77a1) SHA1(bc5c6950293f3bff60bf7913d20a2046aa19ea69) )
ROM_END

ROM_START( ollie )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0007", 0, SHA1(0a3ceb57f1c53154d8b449c38d0546cf35342a50) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0377-com.data", 0x00, 0x50, CRC(d2a8b31f) SHA1(e9ee2df30031826db6bc4bd91969e6680255dcf9) )
ROM_END

ROM_START( wangmidj )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0008b", 0, SHA1(ddf8bde014fee2f0a8a320e4ce19a1729b487e48) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	//PIC16C621A (317-5101-COM)
	//(sticker 253-5509-5101)
	ROM_LOAD("317-5101-com.data", 0x00, 0x50, CRC(3af801f3) SHA1(e9a2558930f3f1f55d5b3c2cadad69329d931f26) )
ROM_END

ROM_START( wangmid )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0009b", 0, SHA1(6fcbebb95b53eaabbc5da6ee08fbe15c2922b8d4) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-5101-com.data", 0x00, 0x50, CRC(3af801f3) SHA1(e9a2558930f3f1f55d5b3c2cadad69329d931f26) )

	// Sanwa CRP-1231LR-10NAB card reader-printer
	ROM_REGION( 0x20000, "card_reader", ROMREGION_ERASE)
	// CRP1231LR10     8B16
	// Ver. 01.10      ???
	// 01/10/12        NA
	// ?? : ME163-5258Z01
	ROM_LOAD("crp1231lr10_ver0110.ic2", 0, 0x20000, CRC(0d30707c) SHA1(425e25c6203d0b400d12391916db3f7cdad00f7a) ) // H8/3003 code
ROM_END

ROM_START( scg05ntb )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0010b", 0, SHA1(8da05ff36679f45b2b43d9c5ca3177b8a9e76af5) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD( "317-unknown.pic", 0x000000, 0x004000, CRC(36858860) SHA1(b36a0c10b614fdeb7dcd94f4898efc24e5db896a) )
ROM_END

ROM_START( scg05nt )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0010c", 0, SHA1(ef3d7577960a21cc71b4523fa26880f7897b628c) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD( "317-unknown.pic", 0x000000, 0x004000, CRC(36858860) SHA1(b36a0c10b614fdeb7dcd94f4898efc24e5db896a) )
ROM_END

ROM_START( outr2stjo )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0011", 0, SHA1(b2cc163109c7f218ce83c76cd995c34e5d2f2812) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD( "317-0396-com.pic", 0x000000, 0x004000, CRC(f94cf26f) SHA1(dd4af2b52935c7b2d8cd196ec1a30c0ef0993322) )
ROM_END

ROM_START( outr2stj )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0011a", 0, SHA1(097c0c746f3bd2b926a7a9a03f868d99b8f77b09) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD( "317-0396-com.pic", 0x000000, 0x004000, CRC(f94cf26f) SHA1(dd4af2b52935c7b2d8cd196ec1a30c0ef0993322) )
ROM_END

ROM_START( ghostsqo )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0012", 0, SHA1(ad5d08cc3b8cfd0890feb152670b429c28659512) )

	ROM_REGION( 0x4010, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0398-com.pic", 0x00, 0x4010, CRC(0b9dfe4b) SHA1(eaa6663cfc49a7bda0ee60844f9d710c99a2b48a) )
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

PIC16C621A-20/P
253-5508-0398
317-0398-COM
*/
ROM_START( ghostsqu )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0012a", 0, SHA1(d14adac9cdfd8095362fa9600c50bf038d4e5a99) )

	ROM_REGION( 0x4010, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0398-com.pic", 0x00, 0x4010, CRC(0b9dfe4b) SHA1(eaa6663cfc49a7bda0ee60844f9d710c99a2b48a) )
ROM_END

ROM_START( gundamos )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0013", 0, SHA1(f97dceb9b4c4adff51d222ab2e6b9b0fe36394a8) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0400-jpn.data", 0x00, 0x50, CRC(0479c383) SHA1(7e86a037d2f9d09cec61a38cb19de510bf9482b3) )
ROM_END

ROM_START( outr2sto )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0014", 0, SHA1(95fa57242e3b708c134c2a34616b745d96c6c811) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD( "317-0396-com.pic", 0x000000, 0x004000, CRC(f94cf26f) SHA1(dd4af2b52935c7b2d8cd196ec1a30c0ef0993322) )
ROM_END

ROM_START( outr2st )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0014a", 0, SHA1(ed60aa1a402bcb01229b18987af199566b930b0b) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD( "317-0396-com.pic", 0x000000, 0x004000, CRC(f94cf26f) SHA1(dd4af2b52935c7b2d8cd196ec1a30c0ef0993322) )
ROM_END

ROM_START( wangmid2j )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0015", 0, SHA1(489bdb96cecaa8c45908a630f64b3cf10e433619) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-5106-jpn.data", 0x00, 0x50, CRC(75c716aa) SHA1(5c2bcf3d28a80b336c6882d5aeb010d04327f8c1) )
ROM_END

ROM_START( wangmid2ja )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0015a", 0, SHA1(0c8e5170e178fcc25acd93a4c2ae79c1e4688f85) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-5106-jpn.data", 0x00, 0x50, CRC(75c716aa) SHA1(5c2bcf3d28a80b336c6882d5aeb010d04327f8c1) )
ROM_END

ROM_START( wangmid2o )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0016", 0, SHA1(a373e3af96bbeea00a44bbe3adfd863d5a94aed9) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-5106-com.data", 0x00, 0x50, CRC(75c716aa) SHA1(5c2bcf3d28a80b336c6882d5aeb010d04327f8c1) )
ROM_END

ROM_START( wangmid2 )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0016a", 0, SHA1(1cbc5e3e9ef1ab26468b9f4ee0fc32a0a320afe7) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-5106-com.data", 0x00, 0x50, CRC(75c716aa) SHA1(5c2bcf3d28a80b336c6882d5aeb010d04327f8c1) )
ROM_END

ROM_START( mj3c )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0017c", 0, SHA1(72545708369aefe8fac82fb0b67774390bb956c6) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD( "317-0414-jpn.pic", 0x000000, 0x004000, CRC(27d1c541) SHA1(c85a8229dd769af02ab43c97f09f995743cdb315) )
ROM_END

ROM_START( mj3d )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0017d", 0, SHA1(d90e06bd1e4c637cb9949d411da11537e72ac3d2) )

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

ROM_START( mj3up )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0019", 0, SHA1(39ac33e857a6f66814c8fc5487705dbf43d47888) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD( "317-0414-jpn.pic", 0x000000, 0x004000, CRC(27d1c541) SHA1(c85a8229dd769af02ab43c97f09f995743cdb315) )
ROM_END

ROM_START( scg06nt )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0018a", 0, SHA1(3c10775aefc5e3e49837bf473fb32e94507ee892) )

	ROM_REGION( 0x50, "pic", ROMREGION_ERASE)
	ROM_LOAD("317-0428-jpn.data", 0x00, 0x50, CRC(1a210abd) SHA1(43a54d028315d2dfa9f8ea6fb59265e0b980b02f) )
ROM_END

ROM_START( mj3evoa )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0021a", 0, SHA1(02c67e8f618b6f4de54898b0c0033b4e0077f2b5) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD( "317-0457-jpn.pic", 0x000000, 0x004000, CRC(650fcc94) SHA1(c88488900460fb3deecb3cf376fc043b10c020ef) )
ROM_END

ROM_START( mj3evo )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0021b", 0, SHA1(c97d1dc95cdf1b4bd5d7cf6b4db0757f3d6bd723) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD( "317-0457-jpn.pic", 0x000000, 0x004000, CRC(650fcc94) SHA1(c88488900460fb3deecb3cf376fc043b10c020ef) )
ROM_END

ROM_START( mj3evoup )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "gdx-0023", 0, SHA1(fd88b7abff4f0932ac650d103e52ac598e82e5fb) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD( "317-0457-jpn.pic", 0x000000, 0x004000, CRC(650fcc94) SHA1(c88488900460fb3deecb3cf376fc043b10c020ef) )
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

/*
Title             CHIHIRO CHANGE REGION GD USA
Checksum          E588
Media Config      GD-ROM1/1
Regions           J
Peripheral String 0000000
Product Number    611-0028A
Version           V2.001
Release Date      20030226
*/
ROM_START( cregchg )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "611-0028a", 0, SHA1(f88d2525ed8c68b380ebd95d22fac28383a7642f) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	//PIC16C621A (317-0351-EXP)
	//(sticker 253-5508-0351E)
	ROM_LOAD("317-0351-exp.pic", 0x00, 0x4000, CRC(25f37472) SHA1(8ffdd637c1eb9989b3b635aface0def9e841f227) )
ROM_END


/* CDV-1xxxx (Sega network CD-ROM and DVD-ROM games) */

ROM_START( questofd )
	CHIHIRO_BIOS

	// "Quest of D"
	// DVD QOD 1.01C
	// CDV-10005C
	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "cdv-10005c", 0, SHA1(b30238cf8697fb7313fedbe75b70641e9418090f) ) // DVD

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	//PIC16C621A (317-0376-JPN)
	ROM_LOAD("317-0376-jpn.pic", 0x00, 0x4000, CRC(c6914d97) SHA1(e86897efcca86f303117d1ead6ede53ac410add8) )

	// "Quest of D"
	// DVD QOD FIRMWARE UPDATE
	// CDV-10018
	DISK_REGION("update")
	DISK_IMAGE_READONLY( "cdv-10018", 0, SHA1(46b00118450f68d5e9319ee3111db47efe3c3098) ) // DVD
ROM_END

ROM_START( gundcb79 )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "cdv-10010", 0, SHA1(88b97408315515909e79d101b37f580fd1f079ce) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	//PIC16C621A (317-0415-JPN)
	//(sticker 253-5508-0415J)
	ROM_LOAD("317-0415-jpn.pic", 0x00, 0x4000, CRC(e5490747) SHA1(91de42a562a265e4cfa1788e40985a5b9055a10a) )
ROM_END

ROM_START( gundcb79a )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "cdv-10024b", 0, SHA1(acc344d7583df191e7c60ff968dedcfe12600018) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	//PIC16C621A (317-0415-JPN)
	//(sticker 253-5508-0415J)
	ROM_LOAD("317-0415-jpn.pic", 0x00, 0x4000, CRC(e5490747) SHA1(91de42a562a265e4cfa1788e40985a5b9055a10a) )
ROM_END

// Quest of D Oukoku no Syugosya
// note: all following CD/DVD discs for server PC, game image from CDV-10026D uploaded via network to satellite Chihiro units
ROM_START( qofd3 )
	CHIHIRO_BIOS

	// "Quest of D Ver.3.02"
	// DVD QOD 3.02
	// CDV-10026D
	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "cdv-10026d", 0, SHA1(b079778f7837100a9b4fa2a536a4efc7817dd2d2) )  // DVD

	// satellite Chihiro security PIC, label is unknown
	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	ROM_LOAD("317-xxxx-jpn.pic", 0x00, 0x4000, CRC(bacf6f52) SHA1(72892aba23a540c02d3260be8c68d2b3fa45bdae) )

	// "Quest of D Ver. 3.0"
	// CD QOD3 VERSION UPDATE
	// CDP-10062
	DISK_REGION("update")
	DISK_IMAGE_READONLY( "cdp-10062", 0, SHA1(abe337cb8782155c4cb92895ba22454a175d479d) )   // CD

	// "Quest of D Ver. 2.0"
	// DVD QOD CHECK DISC
	// CDV-10028
	DISK_REGION("check")
	DISK_IMAGE_READONLY( "cdv-10028", 0, SHA1(9f0f64cb4278cf51a42a21f880cda82b585c63f6) )   // DVD
ROM_END

ROM_START( gundcb83 )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "cdv-10030", 0, SHA1(fc4afdd465e397a12a58714ed9c7a35863580869) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	//PIC16C621A (317-0484-JPN)
	//(sticker 253-5508-0484J)
	ROM_LOAD("317-0484-jpn.pic", 0x00, 0x4000, CRC(308995bb) SHA1(9459ca99bfb5c3cf227821739e7008ae9bd6e710) )
ROM_END

ROM_START( gundcb83a )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "cdv-10031", 0, SHA1(ab165b0c8753c1ab5d9cce82af7ed720a2f83c45) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	//PIC16C621A (317-0484-JPN)
	//(sticker 253-5508-0484J)
	ROM_LOAD("317-0484-jpn.pic", 0x00, 0x4000, CRC(308995bb) SHA1(9459ca99bfb5c3cf227821739e7008ae9bd6e710) )
ROM_END

// Quest of D The Battle Kingdom
// note: all following CD/DVD discs for server PC, game image from CDV-10035B uploaded via network to satellite Chihiro units
ROM_START( qofdtbk )
	CHIHIRO_BIOS

	// "Quest of D The Battle Kingdom"
	// DVD QOD VS
	// CDV-10035B
	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "cdv-10035b", 0, SHA1(710776b88e7403193c1e0889bbd2d15fc8a92880) )  // DVD

	// satellite Chihiro security PIC
	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	//PIC16C621A 317-0506-JPN
	//(sticker 253-5508-0506J)
	ROM_LOAD("317-0506-jpn.pic", 0x00, 0x4000, CRC(e105c6c8) SHA1(63e17b330a2f7d30bf0c263b163469f7f8e6a495) )

	// "Quest of D The Battle Kingdom"
	// CD QOD VS VERSION UPDATE
	// CDP-10078
	DISK_REGION("update")
	DISK_IMAGE_READONLY( "cdp-10078", 0, SHA1(f7dde6a95c8b9087f984f92248c22a3b148ef645) )   // CD

	// "Quest of D The Battle Kingdom"
	// CD QOD SERVICE END
	// CDP-10136
	DISK_REGION("serv_end")
	DISK_IMAGE_READONLY( "cdp-10136", 0, SHA1(3bfb6258bf9c08e1c8056183d02fe8aa3b65db49) )   // CD
ROM_END

ROM_START( gundcb83b )
	CHIHIRO_BIOS

	DISK_REGION( "gdrom" )
	DISK_IMAGE_READONLY( "cdv-10037b", 0, SHA1(f25cb967127d06bef24c64c731c087fc44c1face) )

	ROM_REGION( 0x4000, "pic", ROMREGION_ERASEFF)
	//PIC16C621A (317-0484-JPN)
	//(sticker 253-5508-0484J)
	ROM_LOAD("317-0484-jpn.pic", 0x00, 0x4000, CRC(308995bb) SHA1(9459ca99bfb5c3cf227821739e7008ae9bd6e710) )
ROM_END

/* Main board */
/*Chihiro*/ GAME( 2002, chihiro,  0,        chihiro_base, chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Chihiro BIOS", MACHINE_NO_SOUND|MACHINE_NOT_WORKING|MACHINE_IS_BIOS_ROOT )

/* GDX-xxxx (Sega GD-ROM games) */
/* 0001  */ GAME( 2002, hotd3,    chihiro,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega / Wow Entertainment", "The House of the Dead III (GDX-0001)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0002     GAME( 2003, crtaxhro, crtaxihr, chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega / Hitmaker",          "Crazy Taxi High Roller (GDX-0002)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0002A    GAME( 2003, crtaxhra, crtaxihr, chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega / Hitmaker",          "Crazy Taxi High Roller (Rev A) (GDX-0002A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0002B */ GAME( 2003, crtaxihr, chihiro,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega / Hitmaker",          "Crazy Taxi High Roller (Rev B) (GDX-0002B)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0003     GAME( 2003, vcop3o,   vcop3,    chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Virtua Cop 3 (GDX-0003)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0003A */ GAME( 2003, vcop3a,   vcop3,    chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Virtua Cop 3 (Rev A) (GDX-0003A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0003B */ GAME( 2003, vcop3,    chihiro,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Virtua Cop 3 (Rev B) (GDX-0003B)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0004     GAME( 2003, outr2o,   outr2,    chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "OutRun 2 (GDX-0004)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING|MACHINE_SUPPORTS_SAVE )
/* 0004A */ GAME( 2003, outr2,    chihiro,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "OutRun 2 (Rev A) (GDX-0004A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING|MACHINE_SUPPORTS_SAVE )
// 0005     GAME( 2004, sgolcnpt, chihiro,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Sega Golf Club Network Pro Tour (GDX-0005)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING|MACHINE_SUPPORTS_SAVE )
// 0006     GAME( 2004, mj2o,     mj2,      chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (GDX-0006)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0006A    GAME( 2004, mj2a,     mj2,      chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (Rev A) (GDX-0006A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0006B    GAME( 2004, mj2b,     mj2,      chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (Rev B) (GDX-0006B)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0006C */ GAME( 2004, mj2c,     mj2,      chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (Rev C) (GDX-0006C)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0006D    GAME( 2004, mj2d,     mj2,      chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (Rev D) (GDX-0006D)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0006E    GAME( 2005, mj2e,     mj2,      chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (Rev E) (GDX-0006E)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0006F */ GAME( 2005, mj2f,     mj2,      chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (Rev F) (GDX-0006F)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0006G */ GAME( 2005, mj2,      chihiro,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 2 (Rev G) (GDX-0006G)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0007  */ GAME( 2003, ollie,    chihiro,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega / Amusement Vision",  "Ollie King (GDX-0007)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0008     GAME( 2004, wangmidjo,wangmid,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Namco",                    "Wangan Midnight Maximum Tune (Japan) (GDX-0008)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0008A    GAME( 2004, wangmidja,wangmid,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Namco",                    "Wangan Midnight Maximum Tune (Japan, Rev A) (GDX-0008A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0008B */ GAME( 2004, wangmidj, wangmid,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Namco",                    "Wangan Midnight Maximum Tune (Japan, Rev B) (GDX-0008B)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0009     GAME( 2004, wangmido, wangmid,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Namco",                    "Wangan Midnight Maximum Tune (Export) (GDX-0009)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0009A    GAME( 2004, wangmida, wangmid,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Namco",                    "Wangan Midnight Maximum Tune (Export, Rev A) (GDX-0009A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0009B */ GAME( 2004, wangmid,  chihiro,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Namco",                    "Wangan Midnight Maximum Tune (Export, Rev B) (GDX-0009B)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0010  Sega Golf Club Network Pro Tour 2005
/* 0010B */ GAME( 2004, scg05ntb, scg05nt,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Sega Golf Club Network Pro Tour 2005 (Rev B) (GDX-0010B)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0010C */ GAME( 2004, scg05nt,  chihiro,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Sega Golf Club Network Pro Tour 2005 (Rev C) (GDX-0010C)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0011  */ GAME( 2004, outr2stjo,outr2st,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "OutRun 2 Special Tours (Japan) (GDX-0011)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING|MACHINE_SUPPORTS_SAVE )
/* 0011A */ GAME( 2004, outr2stj, outr2st,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "OutRun 2 Special Tours (Japan, Rev A) (GDX-0011A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING|MACHINE_SUPPORTS_SAVE )
/* 0012  */ GAME( 2004, ghostsqo, ghostsqu, chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Ghost Squad (GDX-0012)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0012A */ GAME( 2004, ghostsqu, chihiro,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Ghost Squad (Rev A) (GDX-0012A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0013  */ GAME( 2005, gundamos, chihiro,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Banpresto",                "Gundam Battle Operating Simulator (GDX-0013)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0014  */ GAME( 2004, outr2sto, outr2st,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "OutRun 2 Special Tours (GDX-0014)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0014A */ GAME( 2004, outr2st,  chihiro,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "OutRun 2 Special Tours (Rev A) (GDX-0014A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0015  */ GAME( 2005, wangmid2j,wangmid2, chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Namco",                    "Wangan Midnight Maximum Tune 2 (Japan) (GDX-0015)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0015A */ GAME( 2005, wangmid2ja,wangmid2,chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Namco",                    "Wangan Midnight Maximum Tune 2 (Japan, Rev A) (GDX-0015A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0016  */ GAME( 2005, wangmid2o,wangmid2, chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Namco",                    "Wangan Midnight Maximum Tune 2 (Export) (GDX-0016)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0016A */ GAME( 2005, wangmid2, chihiro,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Namco",                    "Wangan Midnight Maximum Tune 2 (Export, Rev A) (GDX-0016A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0017     GAME( 2005, mj3o,     mj3,      chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 (GDX-0017)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0017A    GAME( 2005, mj3a,     mj3,      chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 (Rev A) (GDX-0017A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0017B    GAME( 2005, mj3b,     mj3,      chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 (Rev B) (GDX-0017B)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0017C */ GAME( 2005, mj3c,     mj3,      chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 (Rev C) (GDX-0017C)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0017D */ GAME( 2006, mj3d,     mj3,      chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 (Rev D) (GDX-0017D)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0017E    GAME( 2006, mj3e,     mj3,      chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 (Rev E) (GDX-0017E)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0017F */ GAME( 2006, mj3,      chihiro,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 (Rev F) (GDX-0017F)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0018     GAME( 2005, scg06nto, scg06nt,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Sega Club Golf 2006 Next Tours (GDX-0018)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0018A */ GAME( 2005, scg06nt,  chihiro,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Sega Club Golf 2006 Next Tours (Rev A) (GDX-0018A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0019  */ GAME( 2005, mj3up,    chihiro,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 Firmware Update (GDX-0019)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0020  Sega Golf Club 2006
// 0021     GAME( 2006, mj3evoo,  mj3evo,    chihirogd,   chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 Evolution (GDX-0021)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0021A */ GAME( 2007, mj3evoa,  mj3evo,    chihirogd,   chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 Evolution (Rev A) (GDX-0021A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0021B */ GAME( 2007, mj3evo,   chihiro,   chihirogd,   chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 Evolution (Rev B) (GDX-0021B)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0022  Taisen Mahjong MJ 3 Evolution Test Version
/* 0023  */ GAME( 2006, mj3evoup, chihiro,   chihirogd,   chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Sega Network Taisen Mahjong MJ 3 Evolution Firmware Update (GDX-0023)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
// 0024     GAME( 2009, ccfboxo,  ccfboxa,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Chihiro Firmware Update For Compact Flash Box (GDX-0024)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0024A */ GAME( 2009, ccfboxa,  chihiro,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Chihiro Firmware Update For Compact Flash Box (4.01) (GDX-0024A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )

/* 611-xxxx Sega (workshop/factory?) GD-ROMs */
/* 0028A */ GAME( 2003, cregchg,  chihiro,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Chihiro Change Region GD USA (611-0028A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )

/* CDV-1xxxx (Sega network CD-ROM and DVD-ROM games) */
/* 0005C */ GAME( 2004, questofd, chihiro,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Quest of D (CDV-10005C)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0010  */ GAME( 2005, gundcb79, chihiro,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Banpresto",                "Mobile Suit Gundam 0079 Card Builder (CDV-10010)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0024B */ GAME( 2006, gundcb79a,gundcb79, chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Banpresto",                "Mobile Suit Gundam 0079 Card Builder Ver.2.02 (CDV-10024B)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0026D */ GAME( 2007, qofd3,    chihiro,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Quest of D Oukoku no Syugosya Ver. 3.02 (CDV-10026D)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0030  */ GAME( 2007, gundcb83, chihiro,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Banpresto",                "Mobile Suit Gundam 0083 Card Builder (CDV-10030)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0031  */ GAME( 2007, gundcb83a,gundcb83, chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Banpresto",                "Mobile Suit Gundam 0083 Card Builder Check Disk (CDV-10031)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0035B */ GAME( 2007, qofdtbk,  chihiro,  chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Sega",                     "Quest of D The Battle Kingdom (CDV-10035B)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
/* 0037B */ GAME( 2008, gundcb83b,gundcb83, chihirogd,    chihiro, chihiro_state, empty_init, ROT0, "Banpresto",                "Mobile Suit Gundam 0083 Card Builder Ver.2.10 (CDV-10037B)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
