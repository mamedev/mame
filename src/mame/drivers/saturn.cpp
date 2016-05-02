// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, Olivier Galibert, Mariusz Wojcieszek, R. Belmont
/**************************************************************************************************

    Driver by David Haywood, Angelo Salese, Olivier Galibert & Mariusz Wojcieszek
    SCSP driver provided by R. Belmont, based on ElSemi's SCSP sound chip emulator
    Many thanks to Guru, Fabien, Runik and Charles MacDonald for the help given.

***************************************************************************************************

Sega Saturn
(C) Sega 1994/1995/1996/1997

The Sega Saturn is a 32-bit 5th-generation home video game console that was developed by Sega and released
on November 22nd 1994 in Japan, May 11th 1995 in North America, and July 8th 1995 in Europe as the successor
to the Sega Genesis. The Saturn has a dual-CPU architecture and a total of eight processors.
The games are on CD-ROM.

Basic Hardware:

Processors (8)
----------
Two Hitachi SH2 32-bit RISC @ 28.63636MHz
One Hitachi SH1 32-bit RISC @ 20.0MHz
Two 32-bit video display processors (VDP 1 / VDP 2)
One Saturn Control Unit processor (SCU) @ 14.31818MHz
One Motorola 68EC000 sound processor @ 11.2896MHz
One Yamaha YMF292-F DSP sound processor @ 28.63636MHz

Memory
------
16 Megabits DRAM
12 Megabits VRAM (Video RAM)
4 Megabits Audio DRAM
4 Megabits CD-ROM Cache DRAM
256 Kbits battery-backed SRAM

Video
-----
VDP 1
- 32-bit video display processor
- Sprite, polygon, and geometry engine
- Dual 256Kb frame buffers for rotation and scaling effects
- Texture Mapping
- Goraud Shading
- 512Kb cache for textures
VDP 2
- 32-bit background and scroll plane video display processor
- Background engine
- 5 simulataneous scrolling backgrounds
- 2 simultaneous rotating playfields
- 200,000 Texture Mapped Polygons/Second
- 500,000 Flat Shaded Polygons/Second
- Up to 60 frames per second animation
- 24-bit True Color Graphics
- 16 Million Available Colors
- 320x224, 640x224, and 720x576 Resolution

Audio
-----
Motorola 68EC000 sound processor @ 11.2896MHz
Yamaha 24-bit Digital Signal Processor @ 28.63636MHz
- 32 PCM (Pulse Code Modulation) Channels
- 8 FM (Frequency Modulation) Channels
- 44.1 kHz Sampling Rate

Storage
-------
CD-ROM (2X)
- 320 Kb/Second transfer speed
- Compatible with Audio CD, CD+G, CD+EG, CD Single (8cm)
  and with optional hardware; Video CD, Photo CD & Digital Karaoke
- Optional additional memory cartridge

Input/Output
------------
High speed serial communications port
Internal 32-bit expansion port
Internal Multi AV port for optional Video CD (MPEG) decoder board
Composite video & stereo audio (Standard)
Optional RF(TV), S-Video & RGB outputs
2 ports for analog control pad, light gun or other controllers


PCB Layouts
-----------
There were *many* main board revisions. The two general 'sizes' are documented here.

Small board (VA revision documented)
-----------
Main board with a separate small sub-board for the controller ports, power LED and reset button.
The power supply is slightly longer than the main board, has a 5 pin connector and outputs 9VDC, 5VDC and 3.3VDC

837-12126 IC BD SATURN MAIN VA SG
171-7128B (C) SEGA 1995 PC BD SATURN MAIN VA SG
(This PCB was found in a USA Saturn, Model MK-80000, also known as Saturn model 1 with BIOS MPR-17941)

837-12135 IC BD SATURN MAIN VA PAL SD
171-7131A (C) SEGA 1995 PC BD SATURN MAIN VA PAL SD
(This PCB was found in a PAL Saturn, Model MK-80200-50, also known as Saturn model 1 PAL with BIOS MPR-17942)

(note both of these PCBs listed above are almost identical)

|VIDEO--COMM-----------------------------------------------|
|                        CART_SLOT            SW1 BATTERY  |
|ADAC         CXA1645M   |--------| |-------|    20MHz     |
|     SW2                |315-5688| |YGR019B|   TC514260   |
| ^4502161               |        | |       |   |-------|  |
| ^4502161    |--------| |--------| |-------|   |HD6437097 |
| |--------|  |315-5890|   *D489020             |       |  |
| |315-5883|  |        |    81141625 81141625   |-------|  |
| |        |  |--------|                        CARD_SLOT  |
| |--------|  ^4502161                    HC08             |
| ^5241605    ^4502161          |----| |----|   |--------| |
|  514270 |--------|HC04        |SH-2| |SH-2|   |315-5914| |
| |-----| |315-5687|HC04        |----| |----|   |--------| |
| |68000| |        |      |------|32.768kHz                |
| |-----| |--------|      315-5744       LS245  TC514260   |
|       ^74HC157 315-5746 |------|        ^62257  ^ROM  CN4|
|  CN3      CN7  &14.31818MHz  CN2       LS245  TC514260   |
|----------------------------------------------------------|
Notes: (all IC's shown. ^ denotes these parts are on the other side of the PCB)
             ROM - SOP40 mask ROM for BIOS.
                   Chip is pin-compatible with Toshiba TC574200 or MX27C4100 and can be read with a simple 1:1 DIP40 to SOP40 adapter
                   JAPAN BIOS marked 'MPR-17940-MX' or 'MPR-17940-T'
                   USA BIOS   marked 'MPR-17941-MX' or 'MPR-17941-T'
                   PAL BIOS   marked 'MPR-17942-MX' or 'MPR-17942-T'
                   T = Toshiba, MX = Macronix. Both contain identical data
                   Other BIOSes known to exist include:
                   (These are mainly on the very       MPR-16605-T
                    early version main boards VA0/VA1  MPR-16606-T
                    and are all DIP chips)             MPR-16606A-T
                                                       MPR-17577-T
                                                       EPR-17578 HI-SATURN BOOT ROM VER 1.01 SUM AA44 '95 1/27 (EPROM)
        81141625 - Fujitsu 81141625-017 128k x16-bit x 2 banks (4Mbit) SDRAM                     \ compatible
         5241605 - Hitachi HM5241605TT17S or HM524165CTT17S 128k x16-bit x 2 banks (4Mbit) SDRAM /
                   The two 81141625 are the WORK RAM HIGH and the two TC514260 (near the ROM) make up the WORK RAM LOW
                   The 5241605 is the VDP1 Sprite RAM
         D489020 - NEC D489020GF-A15 SGRAM (probably 8Mbit). *- This single chip is replaced by two 81141625 IC's on some boards
         4502161 - NEC D4502161G5-A12 or Sanyo LC382161T-17 64k x16-bit x 2 banks (2Mbit) SDRAM
                   Two chips are used for the VDP1 Frame RAM, the other two are for the VDP2 Video RAM
        TC514260 - 256k x16-bit (4Mbit) DRAM. Any of the following compatible chips are used....
                   Hitachi HM514260AJ7 / HM514260CJ7
                   Toshiba TC514260BJ-70
                   Fujitsu 814260-70
                   Mitsubishi M5M44260CJ
                   Samsung KM416C256BJ-7
                   Hyundai HY514260B JC-70
                   Vanguard VG264260AJ
                   Panasonic MN414260CSJ-07
        HM514270 - Hitachi HM514270 256k x16-bit (4Mbit) DRAM, used for the sound WORK RAM
           62257 - Epson SRM20257LLM10 32k x8-bit (256kbit) battery-backed SRAM (also used SONY CXK58257AM-10L, NEC UPD43257B-10LL, UM62257AM-70LL)
            ADAC - Dual CMOS Audio DAC. Either Burr-Brown BBPCM1710U or Philips TDA1386T
        CXA1645M - Sony CXA1645M RGB to Composite Video Encoder
            SH-2 - Hitachi HD6417095 SH-2 CPU. Clock input 28.63636MHz (14.31818*2)
           68000 - Motorola MC68EC000FN12 CPU. Clock input 11.2896MHz
               & - Master Clock. 14.31818MHz for USA revision or 17.7344MHz for PAL revision
        315-5744 - Sega 315-5744 Hitachi HD404920 microcontroller used as the System Manager and Peripheral Controller (SMPC)
        315-5746 - Sega 315-5746 Phase-locked Loop (PLL) clock generator IC
        315-5883 - Sega 315-5883 Hitachi HD64440 Video Display Processor 1 (VDP1). Earliest revision is 315-5689
        315-5687 - Sega 315-5687 Yamaha YMF292-F Saturn Custom Sound Processor (SCSP). Clock input 28.63636MHz (14.31818*2)
        315-5688 - Sega 315-5688 System Control Unit (SCU). Clock input 14.31818MHz
        315-5890 - Sega 315-5890 Video Display Processor 2 (VDP2)
        315-5914 - Sega 315-5914 DRAM controller. Earliest revision is 315-5778. Later revision is 315-5963
       HD6437097 - Hitachi HD6437097F20 SH1 (SH7034 family) microcontroller with 64k internal ROM. Clock input 20.000MHz
         YGR019B - Hitachi YGR019B CD-Subsystem LSI. Earlier revision is YGR019A. Later revision combines this IC and the SH1 together
                   into one IC (YGR022 315-5962). The SH1 and the YGR019B make up the 'CD Block' CD Authentication and CD I/O data controller.
                   Another of it's functions is to prevent copied CDs from being played
           VIDEO - 10-pin Mini-DIN video output port
            COMM - Communication port
       CARD_SLOT - Expansion slot for MPEG decoder card and other optional expansions
       CART_SLOT - Expansion slot for plug-in RAM or ROM carts
             SW1 - Master reset switch accessible behind the card slot/battery cover. Pressing this clears the battery-backed SRAM, resets the system
                   and the user has to set the language, date and time
         BATTERY - CR2032 3V lithium coin battery. When the system is off the battery provides power to the backup SRAM and SMPC which contains an RTC
             SW2 - CDROM cover open/close detection switch
             CN2 - 24-pin flat cable connector for control port board
             CN3 - 5-pin power connector
             CN4 - Flat cable connector for CDROM data cable. On some main board revisions the connector is reversed and the cable is folded so it
                   is also reversed/flipped 180 degrees at the other end
             CN7 - 5-pin connector for CDROM power


Control Port board
------------------
839-0820 EXPORT
839-0821 PAL
PC BD SATURN PIO VA EXPORT 171-7112A (C) SEGA 1995 CSPD CAD-TEAM
|---------------------------------|
|               FLAT-CABLE        |
|                                 |
| GREEN-LED         RED-LED  RESET|
|--PORT1-----PORT2----------------|
Notes:
      PORT1/2 - Controller ports for controller/joystick/lightgun etc
    GREEN-LED - Power LED
      RED-LED - CDROM drive access LED
        RESET - Push-button reset switch
   FLAT-CABLE - 24-pin flat cable joined to main board CN2


Large board (VA7 revision documented)
-----------
This is a single main board containing everything.

837-12643 IC BD SATURN MAIN VA7 USA SD
171-7208C (C) SEGA 1996 PC BD SATURN MAIN VA7 USA SD
(This PCB was found in a USA Saturn, Model MK-80000A, also known as Saturn model 2 with BIOS MPR-17941)

837-12992 IC BD SATURN MAIN VA7 PAL
171-7424A (C) SEGA 1996 PC BD SATURN MAIN VA7 PAL
(This PCB was found in a PAL Saturn, Model MK-80200A-50, also known as Saturn model 2 PAL with BIOS MPR-17942)

(note both of these PCBs listed above are almost identical)

|VIDEO--COMM-----------------------------------------------|
|                        CART_SLOT                BATTERY  |
|     SW2                |--------|                        |
|ADAC       RGB          |315-5966|   |--------| ^TC514260 |
|                        |        |   |YGR022  |           |
|                        |--------|   |315-5962|           |
|                                     |--------|           |
| |--------|  |--------|                   20MHz           |
| |315-5687|  |315-5964|                        CARD_SLOT  |
| |        |  |        |                                   |
| |--------|  |--------|           |----|   LS245 HC08     |
| ^HM514270  ^524165     HC04      |SH-2|                  |
| ^5221605   ^5221605              |----|         TC514260 |
| ^5221605   ^5221605                                      |
|           |--------|                            TC514260 |
|  |-----|  |315-5883|   HC04                              |
|  |68000|  |        |&14.31818MHz |----|  |--------|      |
|  |-----|  |--------|  %CY2292    |SH-2|  |315-5977-01    |
|CN3             |------|          |----|  |--------|      |
|                315-5744                         62257 CN4|
|            CN7 |------|                                  |
|--|             32.768kHz LS245 81141625 81141625  ROM |--|
   |            GREEN-LED                  RESET        |
   |-------------------PORT1-----PORT2------------------|
Notes: (all IC's shown. ^ denotes these parts are on the other side of the PCB)
             ROM - SOP40 mask ROM for BIOS.
                   Chip is pin-compatible with Toshiba TC574200 or MX27C4100 and can be read with a simple 1:1 DIP40 to SOP40 adapter
                   JAPAN BIOS marked 'MPR-17940-MX' or 'MPR-17940-T'
                   USA BIOS   marked 'MPR-17941-MX' or 'MPR-17941-T'
                   PAL BIOS   marked 'MPR-17942-MX' or 'MPR-17942-T'
                   T = Toshiba, MX = Macronix. Both contain identical data
        81141625 - Fujitsu 81141625-017 128k x16-bit x 2 banks (4Mbit) SDRAM
                   The two 81141625 are the WORK RAM HIGH and two TC514260 (near the SH-2) make up the WORK RAM LOW
          524165 - Hitachi HM524165CTT17S 128k x16-bit x 2 banks (4Mbit) SDRAM. This is the VDP1 Sprite RAM
         5221605 - Hitachi HM5221605TT17S 64k x16-bit x 2 banks (2Mbit) SDRAM
                   Two chips are used for the VDP1 Frame RAM, the other two are for the VDP2 Video RAM
        TC514260 - 256k x16-bit (4Mbit) DRAM. Any of the following compatible chips are used....
                   Hitachi HM514260AJ7 / HM514260CJ7
                   Toshiba TC514260BJ-70
                   Fujitsu 814260-70
                   Mitsubishi M5M44260CJ
                   Samsung KM416C256BJ-7
                   Hyundai HY514260B JC-70
                   Vanguard VG264260AJ
                   Panasonic MN414260CSJ-07
        HM514270 - Hitachi HM514270 256k x16-bit (4Mbit) DRAM, used for the sound WORK RAM
           62257 - Epson SRM20257LLM10 32k x8-bit (256kbit) battery-backed SRAM (also used SONY CXK58257AM-10L, NEC UPD43257B-10LL, UM62257AM-70LL)
            ADAC - Dual CMOS Audio DAC. Either Burr-Brown BBPCM1710U or Philips TDA1386T
             RGB - RGB to Composite Video Encoder with PAL & NTSC output capability. IC is either Fujitsu MB3516A or ROHM BH7236AF
            SH-2 - Hitachi HD6417095 SH-2 CPU. Clock input 28.63636MHz (14.31818*2)
           68000 - Motorola MC68EC000FN12 CPU. Clock input 11.2896MHz
          CY2292 - Cypress CY2292SC-04 PLL clock generator IC. % = On the VA7 PAL version this chip is replaced with the older Sega PLL (315-5746)
               & - Master Clock. 14.31818MHz for USA revision or 17.7344MHz for PAL revision
        315-5744 - Sega 315-5744 Hitachi HD404920 microcontroller used as the System Manager and Peripheral Controller (SMPC)
        315-5883 - Sega 315-5883 Hitachi HD64440 Video Display Processor 1 (VDP1).
        315-5687 - Sega 315-5687 Yamaha YMF292-F Saturn Custom Sound Processor (SCSP). Clock input 28.63636MHz (14.31818*2)
        315-5964 - Sega 315-5964 Video Display Processor 2 (VDP2)
        315-5966 - Sega 315-5966 System Control Unit (SCU). Clock input 14.31818MHz
     315-5977-01 - Sega 315-5977-01 DRAM controller
          YGR022 - Hitachi YGR022 Sega 315-5962 single IC containing CD-Subsystem LSI and Hitachi SH-1 microcontroller with 64k internal ROM. Clock input 20.000MHz
           VIDEO - 10-pin Mini-DIN video output port
            COMM - Communication port
       CARD_SLOT - Expansion slot for MPEG decoder card and other optional expansions
       CART_SLOT - Expansion slot for plug-in RAM or ROM carts
         BATTERY - CR2032 3V lithium coin battery. When the system is off the battery provides power to the backup SRAM and SMPC which contains an RTC
             SW2 - CDROM cover open/close detection switch
             CN3 - 4-pin or 5-pin power connector
             CN4 - Flat cable connector for CDROM data cable
             CN7 - 5-pin connector for CDROM power
         PORT1/2 - Controller ports for controller/joystick/lightgun etc
       GREEN-LED - Power LED
           RESET - Push-button reset switch


Motherboard List
----------------
Board types used in Model 1: VA0 to VA3
Board types used in Model 2: VA2 to VA15
If the VA-number is an even number the board uses a single 8Mbit SGRAM for some of the work RAM, if an odd number it uses two 4Mbit SDRAMs.
Note there are MANY missing. Please help to update this list if you have info for others not listed here.

837-11076    IC BD SATURN MAIN VA0.5        171-6874E PC BD SATURN MAIN VA0.5      (C) SEGA 1994
837-11076-01 IC BD SATURN MAIN VA0 CCI      171-6874D PC BD SATURN MAIN VA0.5      (C) SEGA 1994
837-11491    IC BD SATURN MAIN VA0          171-6962A PC BD SATURN MAIN VA0 USA    (C) SEGA 1995
837-11493    IC BD SATURN MAIN VA0 PAL      171-6963B PC BD SATURN MAIN VA0 PAL    (C) SEGA 1995
837-11613-01 IC BD SATURN MAIN VA1          171-7006C PC BD SATURN MAIN VA1        (C) SEGA 1995
837-11892-01 PAL                            171-7069B MAIN                         (C) SEGA 1995
837-12126    IC BD SATURN MAIN VA SG        171-7128B PC BD SATURN MAIN VA SG      (C) SEGA 1995
837-12126    IC BD SATURN MAIN VA SG        171-7128C PC BD SATURN MAIN VA SG      (C) SEGA 1995
837-12133    IC BD SATURN MAIN VA SD        171-7130C PC BD SATURN MAIN VA SD      (C) SEGA 1995
837-12134    IC BD SATURN MAIN VA USA SD    171-7130C PC BD SATURN MAIN VA USA SD  (C) SEGA 1995
837-12135    IC BD SATURN MAIN VA PAL SD    171-7131A PC BD SATURN MAIN VA PAL SD  (C) SEGA 1995
837-12459    IC BD SATURN MAIN VA6 JPN SG   171-7207A PC BD SATURN MAIN VA6 SG     (C) SEGA 1996
837-12468    IC BD SATURN MAIN VA8 JPN OCU  171-7209C PC BD SATURN MAIN VA8 OCU    (C) SEGA 1996
837-12643    IC BD SATURN MAIN VA7 USA SD   171-7208C PC BD SATURN MAIN VA7 USA SD (C) SEGA 1996
(none)                                      171-7291B PC BD SATURN MAIN VA9 PAL    (C) SEGA 1996
(none)                                      171-7291C PC BD SATURN MAIN VA9        (C) SEGA 1996
837-12650    IC BD SATURN MAIN VA13 JPN     171-7???? PC BD SATURN MAIN VA13       (C) SEGA 1996
837-12845    IC BD SATURN MAIN VA13 USA     171-7???? PC BD SATURN MAIN VA13       (C) SEGA 1996
837-12992    IC BD SATURN MAIN VA7 PAL      171-7424A PC BD SATURN MAIN VA7 PAL    (C) SEGA 1996
837-13100    IC BD SATURN MAIN VA13 PAL     171-7455D PC BD SATURN MAIN VA13 PAL   (C) SEGA 1997
837-13137    IC BD SATURN MAIN VA15 JPN     171-7462B PC BD SATURN MAIN VA15       (C) SEGA 1997


Motherboard Variations Summary
------------------------------

NTSC
----
- VA0: First revision. CD Block (YGR019A & HD6437097) is on a daughterboard. Power supply mounted on top casing.
  Has 40 pin DIP EPROM or mask ROM for BIOS. Larger board with control ports on the main board. Uses JVC CD drive units ENR-007B/ENR-007D.
- VA1: marked as 'VA' on the main board. Power supply is now bottom mounted and plugs in on top into 5 pins on the main board. Most sub boards
  integrated into the main board except the controller ports which are on a small sub board. BIOS is SOP40 mask ROM located on the bottom side of
  the main board. Battery-backup RAM and the VRAM are also on the bottom side of the main board. Uses ENR-007D CD drive units.
- VA2 & VA3: Mostly same as VA1. VA2 is marked as 'VA SG' (uses SGRAM), and VA3 is marked as VA SD (uses SDRAM). Uses ENR-011A CD drive units.
- VA4 & VA5: Same as VA2 and VA3 but in a cheaper model 2 case. Uses ENR-011A CD drive units.
- VA6: One single PCB for everything. Uses an off-the-shelf PLL chip (CY2292). Some custom chips have been revised and have different 315-xxxx numbers.
  BIOS and battery-backed RAM moved to the top side of the main board. Power supply has 4 pins and generates only +5VDC. Uses ENR-013A CD drive unit
  or Sanyo 610-6185-30 CD drive unit.
- VA6 & VA7 has the CD Block reduced to a single IC (YGR022). Some custom chips have been revised and have different 315-xxxx numbers.
- VA8 & VA9 still has the CD Block ICs separated. VA9 uses the old type PLL chip (315-5746).
- VA10 to VA15: uses HQA-001A CD drive unit or Sanyo 610-6294-30 / 610-6473-30 CD drive unit. 68000 & YMF292 integrated into a single IC (315-5965).
  The integrated sound IC has a bug with certain 68000 commands.
- VA11 has a small daughter board mounted on the main board to fix a design fault (possibly to fix the above sound IC problem?)
- VA11+ boards use a smaller TSSOP20 audio DAC, VA10 uses the old one.
- VA12, VA14 and VA16 might not exist.
- VA13 fixes the design fault on VA11 so the patch board is no longer needed.
- VA15 integrates the two SH-2 main CPUs into a larger single IC (HD6417098 / 315-6018).

PAL (only the main differences to the above are listed)
---
- No PAL board ever used SGRAM.
- All PAL boards have an odd VA-number.
- All PAL boards have a 5 pin power connector and use a 5 pin power supply.
- All PAL boards use different region & video output jumpers when compared to NTSC machines.
- All PAL boards use a 17.7344MHz master clock (NTSC units use 14.31818MHz).
- All PAL boards replace the composite sync output on the A/V OUT connector with 9VDC which is used for SCART auto switching. The 9VDC power comes from
  pin 5 of the power supplies with a 5 pin connector. Composite sync is still there at TP4 on the bottom of the board but not wired into the A/V OUT port.
- VA0 PAL - has extra jumpers to set the master clock divider (JP18 & 19), functional but unpopulated 50/60Hz switch on the back (SW4).
- VA1 PAL - unpopulated 50/60Hz switch on the back (SW4). There is a design fault as it is still connected to the master clock divider select pin.
  Therefore the switch does not work on its own, you have to cut or raise & ground the PLL pin 1 for the switch to work.
- VA3 PAL - has extra jumpers to set the master clock divider (JP20 & 21), functional but unpopulated 50/60Hz switch on the back (SW4).
- VA5 PAL - same as VA3 PAL.
- VA7 PAL - Unlike NTSC boards, this still uses the old PLL (315-5746) and pin 1 is connected to the PAL/NTSC and 50/60Hz selection pins on the video
            encoder and the VDP2.
- VA9 - same as VA7 PAL.
- VA13 PAL - Other than a 5 pin power connector it's identical to the NTSC VA13 board.
- VA17 PAL - probably the final revision specifically for EU/PAL regions. Differences are unknown.

Power supplies
--------------
Type A is used on VA0 main boards and is mounted to the top casing. Pinout is GND, GND, 3.3V, 5V, (empty pin), 9V. (5 pins total)
Type B is used on VA1 to VA5 main boards and is bottom mounted. Pinout is GND, GND, 3.3V, 5V, 9V. (5 pins total)
Type C is used on VA6+ main boards and is bottom mounted. Pinout is GND, GND, 5V, 5V (4 pins total).
PAL units use either Type B or a 5-pin version of Type C power supplies. On earlier boards such as 'VA' the 9V pin is connected to pin 1 of the CD ROM
unit power supply cable connector on the main board. On later boards it's also connected to the A/V OUT port for SCART auto switching.

CD Drives
---------
With 20 pin flat cable connector, VA0-VA1:
- JVC ENR-007B EMW10447-003E
- JVC ENR-007B EMW10447-004E
- JVC ENR-007D EMW10447-005E
- JVC ENR-007D EMW10447-006E
- Hitachi JA00292

With 21 pin flat cable connector, VA2-VA5:
- JVC ENR-011A EMW10589-002
- JVC ENR-011A EMW10589-003

With 21 pin flat cable connector, VA6-VA9:
- JVC ENR-013A EMW20035-002 610-6185-20
- Sanyo 610-6185-30 (sometimes with an extra protection board where the flat cable plugs in)

With 21 pin flat cable connector, VA10-VA15:
- JVC HQA-001A HQ100002-002 610-6294-20
- Sanyo 610-6294-30 \
- Sanyo 610-6473-30 / (sometimes with an extra protection board where the flat cable plugs in)
These are the same as the VA6-VA9 units but lack an oscillator and have a white border on the edges of the PCB.

Optical pickups used - JVC drive: Optima-6, Hitachi drive: HOP-6, Sanyo drive: SF-P101 is used in the 610-6185-30

****************************************************************************************************

Emulation Notes:
-To enter into an Advanced Test Mode,keep pressed the Test Button (F2) on the start-up.
-Memo: Some tests done on the original & working PCB,to be implemented:
 -The AD-Stick returns 0x00 or a similar value.
 -The Ports E,F & G must return 0xff

TODO:
(Main issues)
- decap the SH-1, used for CD block (needed especially for Sega Saturn)
- IRQs: some games have some issues with timing accurate IRQs, check/fix all of them.
- The Cart-Dev mode hangs even with the -dev bios,I would like to see what it does on the real HW.
- IC13 games on the dev bios doesn't even load the cartridge / crashes the emulation at start-up,
  rom rearrange needed?
- SCU DSP still has its fair share of issues, it also needs to be converted to CPU structure;
- Add the RS232c interface (serial port), needed by fhboxers (accesses some ports in the a-bus dummy range).
- Video emulation is nowhere near perfection.
- Reimplement the idle skip if possible.
- Move SCU device into its respective file;

test1f diagnostic hacks:
"chash parge error" test 0x6035d04 <- 0x0009 (nop the button check)
"chase line pearg" test 0x6036964 <- 0x0009 (nop the button check again)

****************************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "cpu/sh2/sh2.h"
#include "cpu/scudsp/scudsp.h"
#include "sound/scsp.h"
#include "sound/cdda.h"
#include "machine/smpc.h"
#include "machine/nvram.h"
#include "includes/stv.h"
#include "imagedev/chd_cd.h"
#include "coreutil.h"

#include "bus/saturn/sat_slot.h"
#include "bus/saturn/rom.h"
#include "bus/saturn/dram.h"
#include "bus/saturn/bram.h"
#include "softlist.h"

class sat_console_state : public saturn_state
{
public:
	sat_console_state(const machine_config &mconfig, device_type type, const char *tag)
				: saturn_state(mconfig, type, tag)
				, m_exp(*this, "exp")
				, m_nvram(*this, "nvram")
				, m_smpc_nv(*this, "smpc_nv")
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(nmi_reset);
	DECLARE_INPUT_CHANGED_MEMBER(tray_open);
	DECLARE_INPUT_CHANGED_MEMBER(tray_close);

	DECLARE_MACHINE_START(saturn);
	DECLARE_MACHINE_RESET(saturn);

	DECLARE_READ8_MEMBER(saturn_cart_type_r);
	DECLARE_READ32_MEMBER(abus_dummy_r);

	DECLARE_READ32_MEMBER(saturn_null_ram_r);
	DECLARE_WRITE32_MEMBER(saturn_null_ram_w);

	void saturn_init_driver(int rgn);
	DECLARE_DRIVER_INIT(saturnus);
	DECLARE_DRIVER_INIT(saturneu);
	DECLARE_DRIVER_INIT(saturnjp);

	void nvram_init(nvram_device &nvram, void *data, size_t size);

	required_device<sat_cart_slot_device> m_exp;
	required_device<nvram_device> m_nvram;
	required_device<nvram_device> m_smpc_nv;    // TODO: move this in the base class saturn_state and add it to stv in MAME
};


READ8_MEMBER(sat_console_state::saturn_cart_type_r)
{
	if (m_exp)
		return m_exp->get_cart_type();
	else
		return 0xff;
}

/* TODO: Bug! accesses this one, if returning 0 the SH-2 hard-crashes. Might be an actual bug with the CD block. */
READ32_MEMBER( sat_console_state::abus_dummy_r )
{
	logerror("A-Bus Dummy access %08x\n",offset*4);
	return -1;
}

static ADDRESS_MAP_START( saturn_mem, AS_PROGRAM, 32, sat_console_state )
	AM_RANGE(0x00000000, 0x0007ffff) AM_ROM AM_SHARE("share6")  AM_WRITENOP // bios
	AM_RANGE(0x00100000, 0x0010007f) AM_READWRITE8(saturn_SMPC_r, saturn_SMPC_w,0xffffffff)
	AM_RANGE(0x00180000, 0x0018ffff) AM_READWRITE8(saturn_backupram_r, saturn_backupram_w,0xffffffff) AM_SHARE("share1")
	AM_RANGE(0x00200000, 0x002fffff) AM_RAM AM_MIRROR(0x20100000) AM_SHARE("workram_l")
	AM_RANGE(0x01000000, 0x017fffff) AM_WRITE(saturn_minit_w)
	AM_RANGE(0x01800000, 0x01ffffff) AM_WRITE(saturn_sinit_w)
//  AM_RANGE(0x02000000, 0x023fffff) AM_ROM // Cartridge area
//  AM_RANGE(0x02400000, 0x027fffff) AM_RAM // External Data RAM area
//  AM_RANGE(0x04000000, 0x047fffff) AM_RAM // External Battery RAM area
	AM_RANGE(0x04fffffc, 0x04ffffff) AM_READ8(saturn_cart_type_r,0x000000ff)
	AM_RANGE(0x05000000, 0x057fffff) AM_READ(abus_dummy_r)
	AM_RANGE(0x05800000, 0x0589ffff) AM_READWRITE(stvcd_r, stvcd_w)
	/* Sound */
	AM_RANGE(0x05a00000, 0x05a7ffff) AM_READWRITE16(saturn_soundram_r, saturn_soundram_w,0xffffffff)
	AM_RANGE(0x05b00000, 0x05b00fff) AM_DEVREADWRITE16("scsp", scsp_device, read, write, 0xffffffff)
	/* VDP1 */
	AM_RANGE(0x05c00000, 0x05c7ffff) AM_READWRITE(saturn_vdp1_vram_r, saturn_vdp1_vram_w)
	AM_RANGE(0x05c80000, 0x05cbffff) AM_READWRITE(saturn_vdp1_framebuffer0_r, saturn_vdp1_framebuffer0_w)
	AM_RANGE(0x05d00000, 0x05d0001f) AM_READWRITE16(saturn_vdp1_regs_r, saturn_vdp1_regs_w,0xffffffff)
	AM_RANGE(0x05e00000, 0x05e7ffff) AM_MIRROR(0x80000) AM_READWRITE(saturn_vdp2_vram_r, saturn_vdp2_vram_w)
	AM_RANGE(0x05f00000, 0x05f7ffff) AM_READWRITE(saturn_vdp2_cram_r, saturn_vdp2_cram_w)
	AM_RANGE(0x05f80000, 0x05fbffff) AM_READWRITE16(saturn_vdp2_regs_r, saturn_vdp2_regs_w,0xffffffff)
	AM_RANGE(0x05fe0000, 0x05fe00cf) AM_READWRITE(saturn_scu_r, saturn_scu_w)
	AM_RANGE(0x06000000, 0x060fffff) AM_RAM AM_MIRROR(0x21f00000) AM_SHARE("workram_h")
	AM_RANGE(0x20000000, 0x2007ffff) AM_ROM AM_SHARE("share6")  // bios mirror
//  AM_RANGE(0x22000000, 0x24ffffff) AM_ROM // Cartridge area mirror
	AM_RANGE(0x45000000, 0x46ffffff) AM_WRITENOP
	AM_RANGE(0x60000000, 0x600003ff) AM_WRITENOP // cache address array
	AM_RANGE(0xc0000000, 0xc0000fff) AM_RAM // cache data array, Dragon Ball Z sprites relies on this
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_mem, AS_PROGRAM, 16, sat_console_state )
	AM_RANGE(0x000000, 0x0fffff) AM_RAM AM_SHARE("sound_ram")
	AM_RANGE(0x100000, 0x100fff) AM_DEVREADWRITE("scsp", scsp_device, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( scudsp_mem, AS_PROGRAM, 32, sat_console_state )
	AM_RANGE(0x00, 0xff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( scudsp_data, AS_DATA, 32, sat_console_state )
	AM_RANGE(0x00, 0xff) AM_RAM
ADDRESS_MAP_END



INPUT_CHANGED_MEMBER(sat_console_state::nmi_reset)
{
	/* TODO: correct? */
	if(!m_NMI_reset)
		return;

	/* TODO: NMI doesn't stay held on SH-2 core so we can't use ASSERT_LINE/CLEAR_LINE with that yet */
	if(newval)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INPUT_CHANGED_MEMBER(sat_console_state::tray_open)
{
	if(newval)
		stvcd_set_tray_open();
}

INPUT_CHANGED_MEMBER(sat_console_state::tray_close)
{
	if(newval)
		stvcd_set_tray_close();
}

static INPUT_PORTS_START( saturn )
	PORT_START("RESET") /* hardwired buttons */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CHANGED_MEMBER(DEVICE_SELF, sat_console_state, nmi_reset,0) PORT_NAME("Reset Button")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CHANGED_MEMBER(DEVICE_SELF, sat_console_state, tray_open,0) PORT_NAME("Tray Open Button")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CHANGED_MEMBER(DEVICE_SELF, sat_console_state, tray_close,0) PORT_NAME("Tray Close")

	PORT_START("fake")
	PORT_CONFNAME(0x01,0x00,"Master-Slave Comms")
	PORT_CONFSETTING(0x00,"Normal (400 cycles)")
	PORT_CONFSETTING(0x01,"One Shot (Hack)")
INPUT_PORTS_END


/* TODO: if you change the driver configuration then NVRAM contents gets screwed, needs mods in MAME framework */
void sat_console_state::nvram_init(nvram_device &nvram, void *data, size_t size)
{
	static const UINT8 init[64] = {
	'B', 'a', 'c', 'k', 'U', 'p', 'R', 'a', 'm', ' ', 'F', 'o', 'r', 'm', 'a', 't',
	'B', 'a', 'c', 'k', 'U', 'p', 'R', 'a', 'm', ' ', 'F', 'o', 'r', 'm', 'a', 't',
	'B', 'a', 'c', 'k', 'U', 'p', 'R', 'a', 'm', ' ', 'F', 'o', 'r', 'm', 'a', 't',
	'B', 'a', 'c', 'k', 'U', 'p', 'R', 'a', 'm', ' ', 'F', 'o', 'r', 'm', 'a', 't', };

	memset(data, 0x00, size);
	memcpy(data, init, sizeof(init));
}


MACHINE_START_MEMBER(sat_console_state, saturn)
{
	system_time systime;
	machine().base_datetime(systime);

	machine().device<scsp_device>("scsp")->set_ram_base(m_sound_ram);

	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x02400000, 0x027fffff, read32_delegate(FUNC(sat_console_state::saturn_null_ram_r),this), write32_delegate(FUNC(sat_console_state::saturn_null_ram_w),this));
	m_slave->space(AS_PROGRAM).install_readwrite_handler(0x02400000, 0x027fffff, read32_delegate(FUNC(sat_console_state::saturn_null_ram_r),this), write32_delegate(FUNC(sat_console_state::saturn_null_ram_w),this));

	m_maincpu->space(AS_PROGRAM).nop_readwrite(0x04000000, 0x047fffff);
	m_slave->space(AS_PROGRAM).nop_readwrite(0x04000000, 0x047fffff);

	m_nvram->set_base(m_backupram.get(), 0x8000);
	m_smpc_nv->set_base(&m_smpc.SMEM, 4);

	if (m_exp)
	{
		switch (m_exp->get_cart_type())
		{
			case 0x21:  // Battery RAM cart
			case 0x22:
			case 0x23:
			case 0x24:
				m_maincpu->space(AS_PROGRAM).install_read_handler(0x04000000, 0x047fffff, read32_delegate(FUNC(sat_cart_slot_device::read_ext_bram), (sat_cart_slot_device*)m_exp));
				m_maincpu->space(AS_PROGRAM).install_write_handler(0x04000000, 0x047fffff, write32_delegate(FUNC(sat_cart_slot_device::write_ext_bram), (sat_cart_slot_device*)m_exp));
				m_slave->space(AS_PROGRAM).install_read_handler(0x04000000, 0x047fffff, read32_delegate(FUNC(sat_cart_slot_device::read_ext_bram), (sat_cart_slot_device*)m_exp));
				m_slave->space(AS_PROGRAM).install_write_handler(0x04000000, 0x047fffff, write32_delegate(FUNC(sat_cart_slot_device::write_ext_bram), (sat_cart_slot_device*)m_exp));
				m_maincpu->space(AS_PROGRAM).install_read_handler(0x24000000, 0x247fffff, read32_delegate(FUNC(sat_cart_slot_device::read_ext_bram), (sat_cart_slot_device*)m_exp));
				m_maincpu->space(AS_PROGRAM).install_write_handler(0x24000000, 0x247fffff, write32_delegate(FUNC(sat_cart_slot_device::write_ext_bram), (sat_cart_slot_device*)m_exp));
				m_slave->space(AS_PROGRAM).install_read_handler(0x24000000, 0x247fffff, read32_delegate(FUNC(sat_cart_slot_device::read_ext_bram), (sat_cart_slot_device*)m_exp));
				m_slave->space(AS_PROGRAM).install_write_handler(0x24000000, 0x247fffff, write32_delegate(FUNC(sat_cart_slot_device::write_ext_bram), (sat_cart_slot_device*)m_exp));
				break;
			case 0x5a:  // Data RAM cart
			case 0x5c:
				m_maincpu->space(AS_PROGRAM).install_read_handler(0x02400000, 0x025fffff, read32_delegate(FUNC(sat_cart_slot_device::read_ext_dram0), (sat_cart_slot_device*)m_exp));
				m_maincpu->space(AS_PROGRAM).install_write_handler(0x02400000, 0x025fffff, write32_delegate(FUNC(sat_cart_slot_device::write_ext_dram0), (sat_cart_slot_device*)m_exp));
				m_maincpu->space(AS_PROGRAM).install_read_handler(0x02600000, 0x027fffff, read32_delegate(FUNC(sat_cart_slot_device::read_ext_dram1), (sat_cart_slot_device*)m_exp));
				m_maincpu->space(AS_PROGRAM).install_write_handler(0x02600000, 0x027fffff, write32_delegate(FUNC(sat_cart_slot_device::write_ext_dram1), (sat_cart_slot_device*)m_exp));
				m_slave->space(AS_PROGRAM).install_read_handler(0x02400000, 0x025fffff, read32_delegate(FUNC(sat_cart_slot_device::read_ext_dram0), (sat_cart_slot_device*)m_exp));
				m_slave->space(AS_PROGRAM).install_write_handler(0x02400000, 0x025fffff, write32_delegate(FUNC(sat_cart_slot_device::write_ext_dram0), (sat_cart_slot_device*)m_exp));
				m_slave->space(AS_PROGRAM).install_read_handler(0x02600000, 0x027fffff, read32_delegate(FUNC(sat_cart_slot_device::read_ext_dram1), (sat_cart_slot_device*)m_exp));
				m_slave->space(AS_PROGRAM).install_write_handler(0x02600000, 0x027fffff, write32_delegate(FUNC(sat_cart_slot_device::write_ext_dram1), (sat_cart_slot_device*)m_exp));
				m_maincpu->space(AS_PROGRAM).install_read_handler(0x22400000, 0x225fffff, read32_delegate(FUNC(sat_cart_slot_device::read_ext_dram0), (sat_cart_slot_device*)m_exp));
				m_maincpu->space(AS_PROGRAM).install_write_handler(0x22400000, 0x225fffff, write32_delegate(FUNC(sat_cart_slot_device::write_ext_dram0), (sat_cart_slot_device*)m_exp));
				m_maincpu->space(AS_PROGRAM).install_read_handler(0x22600000, 0x227fffff, read32_delegate(FUNC(sat_cart_slot_device::read_ext_dram1), (sat_cart_slot_device*)m_exp));
				m_maincpu->space(AS_PROGRAM).install_write_handler(0x22600000, 0x227fffff, write32_delegate(FUNC(sat_cart_slot_device::write_ext_dram1), (sat_cart_slot_device*)m_exp));
				m_slave->space(AS_PROGRAM).install_read_handler(0x22400000, 0x225fffff, read32_delegate(FUNC(sat_cart_slot_device::read_ext_dram0), (sat_cart_slot_device*)m_exp));
				m_slave->space(AS_PROGRAM).install_write_handler(0x22400000, 0x225fffff, write32_delegate(FUNC(sat_cart_slot_device::write_ext_dram0), (sat_cart_slot_device*)m_exp));
				m_slave->space(AS_PROGRAM).install_read_handler(0x22600000, 0x227fffff, read32_delegate(FUNC(sat_cart_slot_device::read_ext_dram1), (sat_cart_slot_device*)m_exp));
				m_slave->space(AS_PROGRAM).install_write_handler(0x22600000, 0x227fffff, write32_delegate(FUNC(sat_cart_slot_device::write_ext_dram1), (sat_cart_slot_device*)m_exp));
				break;
			case 0xff: // ROM cart + mirror
				m_maincpu->space(AS_PROGRAM).install_read_handler(0x02000000, 0x023fffff, read32_delegate(FUNC(sat_cart_slot_device::read_rom), (sat_cart_slot_device*)m_exp));
				m_maincpu->space(AS_PROGRAM).install_read_handler(0x22000000, 0x223fffff, read32_delegate(FUNC(sat_cart_slot_device::read_rom), (sat_cart_slot_device*)m_exp));
				m_slave->space(AS_PROGRAM).install_read_handler(0x02000000, 0x023fffff, read32_delegate(FUNC(sat_cart_slot_device::read_rom), (sat_cart_slot_device*)m_exp));
				m_slave->space(AS_PROGRAM).install_read_handler(0x22000000, 0x223fffff, read32_delegate(FUNC(sat_cart_slot_device::read_rom), (sat_cart_slot_device*)m_exp));
				break;
		}
	}

	// save states
	save_pointer(NAME(m_scu_regs.get()), 0x100/4);
	save_pointer(NAME(m_scsp_regs.get()), 0x1000/2);
	save_item(NAME(m_NMI_reset));
	save_item(NAME(m_en_68k));
	save_item(NAME(m_smpc.IOSEL1));
	save_item(NAME(m_smpc.IOSEL2));
	save_item(NAME(m_smpc.EXLE1));
	save_item(NAME(m_smpc.EXLE2));
	save_item(NAME(m_smpc.PDR1));
	save_item(NAME(m_smpc.PDR2));
//  save_item(NAME(m_port_sel));
//  save_item(NAME(mux_data));
	save_item(NAME(m_scsp_last_line));
	save_item(NAME(m_smpc.intback_stage));
	save_item(NAME(m_smpc.pmode));
	save_item(NAME(m_smpc.SR));
	save_item(NAME(m_smpc.SMEM));

	machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(sat_console_state::stvcd_exit), this));

	m_smpc.rtc_data[0] = DectoBCD(systime.local_time.year /100);
	m_smpc.rtc_data[1] = DectoBCD(systime.local_time.year %100);
	m_smpc.rtc_data[2] = (systime.local_time.weekday << 4) | (systime.local_time.month+1);
	m_smpc.rtc_data[3] = DectoBCD(systime.local_time.mday);
	m_smpc.rtc_data[4] = DectoBCD(systime.local_time.hour);
	m_smpc.rtc_data[5] = DectoBCD(systime.local_time.minute);
	m_smpc.rtc_data[6] = DectoBCD(systime.local_time.second);

	m_stv_rtc_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sat_console_state::stv_rtc_increment),this));

	m_audiocpu->set_reset_callback(write_line_delegate(FUNC(sat_console_state::m68k_reset_callback),this));
}

/* Die Hard Trilogy tests RAM address 0x25e7ffe bit 2 with Slave during FRT minit irq, in-development tool for breaking execution of it? */
READ32_MEMBER(sat_console_state::saturn_null_ram_r)
{
	return 0xffffffff;
}

WRITE32_MEMBER(sat_console_state::saturn_null_ram_w)
{
}

MACHINE_RESET_MEMBER(sat_console_state,saturn)
{
	m_scsp_last_line = 0;

	// don't let the slave cpu and the 68k go anywhere
	m_slave->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_scudsp->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	m_smpc.SR = 0x40;   // this bit is always on according to docs

	scu_reset();

	m_en_68k = 0;
	m_NMI_reset = 0;
	m_smpc.slave_on = 0;

	//memset(stv_m_workram_l, 0, 0x100000);
	//memset(stv_m_workram_h, 0, 0x100000);

	m_maincpu->set_unscaled_clock(MASTER_CLOCK_320/2);
	m_slave->set_unscaled_clock(MASTER_CLOCK_320/2);

	stvcd_reset();

	m_vdp2.old_crmd = -1;
	m_vdp2.old_tvmd = -1;

	m_stv_rtc_timer->adjust(attotime::zero, 0, attotime::from_seconds(1));
}


static MACHINE_CONFIG_START( saturn, sat_console_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SH2, MASTER_CLOCK_352/2) // 28.6364 MHz
	MCFG_CPU_PROGRAM_MAP(saturn_mem)
	MCFG_SH2_IS_SLAVE(0)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", sat_console_state, saturn_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("slave", SH2, MASTER_CLOCK_352/2) // 28.6364 MHz
	MCFG_CPU_PROGRAM_MAP(saturn_mem)
	MCFG_SH2_IS_SLAVE(1)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("slave_scantimer", sat_console_state, saturn_slave_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("audiocpu", M68000, 11289600) //256 x 44100 Hz = 11.2896 MHz
	MCFG_CPU_PROGRAM_MAP(sound_mem)

	MCFG_CPU_ADD("scudsp", SCUDSP, MASTER_CLOCK_352/4) // 14 MHz
	MCFG_CPU_PROGRAM_MAP(scudsp_mem)
	MCFG_CPU_DATA_MAP(scudsp_data)
	MCFG_SCUDSP_OUT_IRQ_CB(WRITELINE(saturn_state, scudsp_end_w))
	MCFG_SCUDSP_IN_DMA_CB(READ16(saturn_state, scudsp_dma_r))
	MCFG_SCUDSP_OUT_DMA_CB(WRITE16(saturn_state, scudsp_dma_w))

//  SH-1

//  SMPC MCU, running at 4 MHz (+ custom RTC device that runs at 32.768 KHz)

	MCFG_MACHINE_START_OVERRIDE(sat_console_state,saturn)
	MCFG_MACHINE_RESET_OVERRIDE(sat_console_state,saturn)

	MCFG_NVRAM_ADD_CUSTOM_DRIVER("nvram", sat_console_state, nvram_init)
	MCFG_NVRAM_ADD_0FILL("smpc_nv") // TODO: default for each region (+ move it inside SMPC when converted to device)

	MCFG_TIMER_DRIVER_ADD("sector_timer", sat_console_state, stv_sector_cb)
	MCFG_TIMER_DRIVER_ADD("sh1_cmd", sat_console_state, stv_sh1_sim)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK_320/8, 427, 0, 320, 263, 0, 224)
	MCFG_SCREEN_UPDATE_DRIVER(sat_console_state, screen_update_stv_vdp2)
	MCFG_PALETTE_ADD("palette", 2048+(2048*2))//standard palette + extra memory for rgb brightness.

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", stv)

	MCFG_VIDEO_START_OVERRIDE(sat_console_state,stv_vdp2)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("scsp", SCSP, 0)
	MCFG_SCSP_IRQ_CB(WRITE8(saturn_state, scsp_irq))
	MCFG_SCSP_MAIN_IRQ_CB(WRITELINE(saturn_state, scsp_to_main_irq))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_SOUND_ADD("cdda", CDDA, 0)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_SATURN_CONTROL_PORT_ADD("ctrl1", saturn_controls, "joypad")
	MCFG_SATURN_CONTROL_PORT_ADD("ctrl2", saturn_controls, "joypad")
MACHINE_CONFIG_END

static SLOT_INTERFACE_START(saturn_cart)
	SLOT_INTERFACE_INTERNAL("rom",    SATURN_ROM)
	SLOT_INTERFACE_INTERNAL("ram8",   SATURN_DRAM_8MB)
	SLOT_INTERFACE_INTERNAL("ram32",  SATURN_DRAM_32MB)
	SLOT_INTERFACE_INTERNAL("bram4",  SATURN_BRAM_4MB)
	SLOT_INTERFACE_INTERNAL("bram8",  SATURN_BRAM_8MB)
	SLOT_INTERFACE_INTERNAL("bram16", SATURN_BRAM_16MB)
	SLOT_INTERFACE_INTERNAL("bram32", SATURN_BRAM_32MB)
SLOT_INTERFACE_END


MACHINE_CONFIG_DERIVED( saturnus, saturn )
	MCFG_CDROM_ADD( "cdrom" )
	MCFG_CDROM_INTERFACE("sat_cdrom")
	MCFG_SOFTWARE_LIST_ADD("cd_list","saturn")
	MCFG_SOFTWARE_LIST_FILTER("cd_list","NTSC-U")

	MCFG_SATURN_CARTRIDGE_ADD("exp", saturn_cart, nullptr)
	MCFG_SOFTWARE_LIST_ADD("cart_list","sat_cart")

MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( saturneu, saturn )
	MCFG_CDROM_ADD( "cdrom" )
	MCFG_CDROM_INTERFACE("sat_cdrom")
	MCFG_SOFTWARE_LIST_ADD("cd_list","saturn")
	MCFG_SOFTWARE_LIST_FILTER("cd_list","PAL")

	MCFG_SATURN_CARTRIDGE_ADD("exp", saturn_cart, nullptr)
	MCFG_SOFTWARE_LIST_ADD("cart_list","sat_cart")

MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( saturnjp, saturn )
	MCFG_CDROM_ADD( "cdrom" )
	MCFG_CDROM_INTERFACE("sat_cdrom")
	MCFG_SOFTWARE_LIST_ADD("cd_list","saturn")
	MCFG_SOFTWARE_LIST_FILTER("cd_list","NTSC-J")

	MCFG_SATURN_CARTRIDGE_ADD("exp", saturn_cart, nullptr)
	MCFG_SOFTWARE_LIST_ADD("cart_list","sat_cart")

MACHINE_CONFIG_END


void sat_console_state::saturn_init_driver(int rgn)
{
	m_saturn_region = rgn;
	m_vdp2.pal = (rgn == 12) ? 1 : 0;

	// set compatible options
	m_maincpu->sh2drc_set_options(SH2DRC_STRICT_VERIFY|SH2DRC_STRICT_PCREL);
	m_slave->sh2drc_set_options(SH2DRC_STRICT_VERIFY|SH2DRC_STRICT_PCREL);

	m_maincpu->sh2drc_add_fastram(0x00000000, 0x0007ffff, 1, &m_rom[0]);
	m_maincpu->sh2drc_add_fastram(0x00200000, 0x002fffff, 0, &m_workram_l[0]);
	m_maincpu->sh2drc_add_fastram(0x06000000, 0x060fffff, 0, &m_workram_h[0]);
	m_slave->sh2drc_add_fastram(0x00000000, 0x0007ffff, 1, &m_rom[0]);
	m_slave->sh2drc_add_fastram(0x00200000, 0x002fffff, 0, &m_workram_l[0]);
	m_slave->sh2drc_add_fastram(0x06000000, 0x060fffff, 0, &m_workram_h[0]);

	/* amount of time to boost interleave for on MINIT / SINIT, needed for communication to work */
	m_minit_boost = 400;
	m_sinit_boost = 400;
	m_minit_boost_timeslice = attotime::zero;
	m_sinit_boost_timeslice = attotime::zero;

	m_scu_regs = make_unique_clear<UINT32[]>(0x100/4);
	m_scsp_regs = make_unique_clear<UINT16[]>(0x1000/2);
	m_backupram = make_unique_clear<UINT8[]>(0x8000);
}

DRIVER_INIT_MEMBER(sat_console_state,saturnus)
{
	saturn_init_driver(4);
}

DRIVER_INIT_MEMBER(sat_console_state,saturneu)
{
	saturn_init_driver(12);
}

DRIVER_INIT_MEMBER(sat_console_state,saturnjp)
{
	saturn_init_driver(1);
}


/* Japanese Saturn */
ROM_START(saturnjp)
	ROM_REGION( 0x480000, "maincpu", ROMREGION_ERASEFF ) /* SH2 code */
	ROM_SYSTEM_BIOS(0, "101", "Japan v1.01 (941228)")
	ROMX_LOAD("sega_101.bin", 0x00000000, 0x00080000, CRC(224b752c) SHA1(df94c5b4d47eb3cc404d88b33a8fda237eaf4720), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "1003", "Japan v1.003 (941012)")
	ROMX_LOAD("sega1003.bin", 0x00000000, 0x00080000, CRC(b3c63c25) SHA1(7b23b53d62de0f29a23e423d0fe751dfb469c2fa), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "100", "Japan v1.00 (940921)")
	ROMX_LOAD("sega_100.bin", 0x00000000, 0x00080000, CRC(2aba43c2) SHA1(2b8cb4f87580683eb4d760e4ed210813d667f0a2), ROM_BIOS(3))
	ROM_REGION( 0x080000, "slave", 0 ) /* SH2 code */
	ROM_COPY( "maincpu",0x000000,0,0x080000)
ROM_END

/* Overseas Saturn */
ROM_START(saturn)
	ROM_REGION( 0x480000, "maincpu", ROMREGION_ERASEFF ) /* SH2 code */
	ROM_SYSTEM_BIOS(0, "101a", "Overseas v1.01a (941115)")
	/* Confirmed by ElBarto */
	ROMX_LOAD("mpr-17933.bin", 0x00000000, 0x00080000, CRC(4afcf0fa) SHA1(faa8ea183a6d7bbe5d4e03bb1332519800d3fbc3), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "100a", "Overseas v1.00a (941115)")
	ROMX_LOAD("sega_100a.bin", 0x00000000, 0x00080000, CRC(f90f0089) SHA1(3bb41feb82838ab9a35601ac666de5aacfd17a58), ROM_BIOS(2))
	ROM_REGION( 0x080000, "slave", 0 ) /* SH2 code */
	ROM_COPY( "maincpu",0x000000,0,0x080000)
ROM_END

ROM_START(saturneu)
	ROM_REGION( 0x480000, "maincpu", ROMREGION_ERASEFF ) /* SH2 code */
	ROM_SYSTEM_BIOS(0, "101a", "Overseas v1.01a (941115)")
	/* Confirmed by ElBarto */
	ROMX_LOAD("mpr-17933.bin", 0x00000000, 0x00080000, CRC(4afcf0fa) SHA1(faa8ea183a6d7bbe5d4e03bb1332519800d3fbc3), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "100a", "Overseas v1.00a (941115)")
	ROMX_LOAD("sega_100a.bin", 0x00000000, 0x00080000, CRC(f90f0089) SHA1(3bb41feb82838ab9a35601ac666de5aacfd17a58), ROM_BIOS(2))
	ROM_REGION( 0x080000, "slave", 0 ) /* SH2 code */
	ROM_COPY( "maincpu",0x000000,0,0x080000)
ROM_END

ROM_START(vsaturn)
	ROM_REGION( 0x480000, "maincpu", ROMREGION_ERASEFF ) /* SH2 code */
	ROM_LOAD("vsaturn.bin", 0x00000000, 0x00080000, CRC(e4d61811) SHA1(4154e11959f3d5639b11d7902b3a393a99fb5776))
	ROM_REGION( 0x080000, "slave", 0 ) /* SH2 code */
	ROM_COPY( "maincpu",0x000000,0,0x080000)
ROM_END

ROM_START(hisaturn)
	ROM_REGION( 0x480000, "maincpu", ROMREGION_ERASEFF ) /* SH2 code */
	ROM_SYSTEM_BIOS(0, "102", "v1.02 (950519)")
	ROMX_LOAD( "mpr-18100.bin", 0x000000, 0x080000, CRC(3408dbf4) SHA1(8a22710e09ce75f39625894366cafe503ed1942d), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "101", "v1.01 (950130)")
	ROMX_LOAD("hisaturn.bin", 0x00000000, 0x00080000, CRC(721e1b60) SHA1(49d8493008fa715ca0c94d99817a5439d6f2c796), ROM_BIOS(2))
	ROM_REGION( 0x080000, "slave", 0 ) /* SH2 code */
	ROM_COPY( "maincpu",0x000000,0,0x080000)
ROM_END

/*    YEAR  NAME        PARENT  COMPAT  MACHINE INPUT   INIT        COMPANY     FULLNAME            FLAGS */
CONS( 1994, saturn,     0,      0,      saturnus, saturn, sat_console_state, saturnus,   "Sega",     "Saturn (USA)",     MACHINE_NOT_WORKING )
CONS( 1994, saturnjp,   saturn, 0,      saturnjp, saturn, sat_console_state, saturnjp,   "Sega",     "Saturn (Japan)",   MACHINE_NOT_WORKING )
CONS( 1994, saturneu,   saturn, 0,      saturneu, saturn, sat_console_state, saturneu,   "Sega",     "Saturn (PAL)",     MACHINE_NOT_WORKING )
CONS( 1995, vsaturn,    saturn, 0,      saturnjp, saturn, sat_console_state, saturnjp,   "JVC",      "V-Saturn",         MACHINE_NOT_WORKING )
CONS( 1995, hisaturn,   saturn, 0,      saturnjp, saturn, sat_console_state, saturnjp,   "Hitachi",  "HiSaturn",         MACHINE_NOT_WORKING )
