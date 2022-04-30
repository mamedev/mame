// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    namcops2.cpp

    Namco System 246 / System 256 games (Sony PS2 based)

    PS2 baseboard includes:
    * R5900 "Emotion Engine" - MIPS III with 128-bit integer regs & SIMD
    * R3000 IOP - Stock R3000 with cache, not like the PSXCPU
    * VU0 - can operate either as in-line R5900 coprocessor or run independently
    * VU1 - runs independently only, has special DMA path to GS
    * GS - Graphics Synthesizer.  Nothing fancy, draws flat, Gouraud, and/or
      textured tris with no, bilinear, or trilinear filtering very quickly.
    * SPU2 - Sound Processing Unit.  Almost literally 2 PS1 SPUs stuck together,
      with 2 MB of wave RAM, a 48 kHz sample rate (vs. 44.1 on PS1), and 2
      stereo DMADACs.


Namco System 246, 256 & Super System 256 Hardware Overview
Namco 2001-2010

Games on System 246/256/S256 include.....
(Note this list is not complete and other versions or games may exist)
                                                                               Media Type             Game ID &
Name from title screen                           System   Media ID            (HDD/CD/DVD)  Cart ID   Revision               Company/Year                  Notes
-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Battle Gear 3 (Japan)........................... XX34XXX  M9005793A VER.2.04J  HDD (20GB)   NM00010   B3900065A              Taito 2002                    \  HDD: Western Digital WD200EB
Battle Gear 3 (Export).......................... XX34XXX  M9005793A VER.2.04J  HDD (20GB)   NM00010  *?                      Taito 2002                     | All require Taito JVS Universal I/O board K91X0951A otherwise no boot-up
Battle Gear 3 Tuned (Japan)..................... XX34XXX  M9006066A VER.2.03J  HDD (30GB)   NM00015  *B3900074B              Taito 2003                     | Dongle selects region using same HDD's
Battle Gear 3 Tuned (Export).................... XX34XXX  M9006066A VER.2.03J  HDD (30GB)   NM00015   B3900074C              Taito 2003                    /  HDD: Maxtor Fireball 3 30GB 2F030L0
Bloody Roar 3................................... 1234XXX  BRT1-A               CD           NM00002   BRT1 Ver.A             Namco/8ing/Raizing 2000
Capcom Fighting Jam/Capcom Fighting Evolution... XXXX56X  JAM1 DVD0            DVD          NM00018   JAM1 Ver.A             Capcom 2004
Cobra The Arcade................................ XXXX56X  CBR1-HA              HDD (40GB)   NM00021   CBR1 Ver.B             Namco 2004                    Requires RAYS I/O PCB and IR guns and IR sensors. HDD: Maxtor DiamondMax Plus 8 40GB 6E040L0
Dragon Chronicles (satellite)................... ------X *DCO31-TS CD0        *CD           NM00020   DC001 Ver.A            Namco 2002                    \
Dragon Chronicles Legend of the Master Ark (sat) ------X *DGC11 CD0           *CD          *NM00014  *DGC11 Ver.A1           Namco 200?                    | server is a custom PC
Druaga Online The Story Of Aon (satellite)...... XXXX56X  DOL160-1-ST-DVD0-H   DVD          NM00028   DOL165-1-ST-I Ver1.65  Namco 2004                    |
   "                                      ...... XXXX56X  DOL150-1-ST-DVD0-G   DVD          NM00028  *?                      Namco 2004                    |
   "                                      ...... XXXX56X  DOL140-1-ST-DVD0-F   DVD          NM00028  *?                      Namco 2004                    |
   "                                      ...... XXXX56X  DOL120-1-ST-DVD0-D   DVD          NM00028  *?                      Namco 2004                    |
   "                                      ...... XXXX56X  DOL110-1-ST-DVD0-C   DVD          NM00028  *?                      Namco 2004                    /
Fate / Unlimited Codes.......................... X23456X  FUD-HDD0-A           HDD (80GB)   NM00048   FUD1 Ver.A             Capcom/Type-Moon/Cavia/8ing 2008 HDD: Western Digital WD800BB
Gundam vs Gundam Next........................... XXXX56X  GNX100-1-NA-HDD0-A   HDD (80GB)   NM00052   GNX1001-NA-A           Bandai/Capcom 2009            HDD: Western Digital WD800BB
Idol Master..................................... ------X *IDM1-HA             *HDD         *NM00022  *IDMS1 Ver.A            Namco 2004
Kinnikuman Muscle Grand Prix.................... XXXX56X  KN1-B                DVD          NM00029   KN1 Ver.A              Banpresto 2006                #
Kinnikuman Muscle Grand Prix 2.................. XXXX56X  KN2                  DVD          NM00040   KN2 Ver.A              Banpresto 2007                #
Minna de Kitaeru Zenno Training................. ------X *ZNT100-1-NA-DVD0    *DVD          NM00036   ZNT100-1-ST-A          Namco 2006
Minna de Kitaeru Zenno Training.(Ver. 1.50)..... ------X  ZNT100-1-NA-DVD0-B   DVD          NM00036   ZNT100-1-ST-A          Namco 2007
Mobile Suit Gundam - Gundam vs Gundam........... XXXX56X  GVS1 DVD0B           DVD          NM00043   GVS1 Ver.A             Bandai/Capcom 2008
Mobile Suit Gundam SEED O.M.N.I. vs Z.A.F.T..... 123456X  SED1 DVD0            DVD          NM00024   SED1 Ver.A             Banpresto 2005                % #
M.S. Gundam SEED Destiny O.M.N.I. vs Z.A.F.T. II 123456X  GSD1 DVD0            DVD          NM00034   GSD1 Ver.A             Banpresto 2006                % #
Mobile Suit Z Gundam A.E.U.G. vs Titans......... 123456X  ZGA1 DVD0            DVD          NM00013   ZGA1 Ver.A             Capcom/Banpresto 2003         %
Mobile Suit Z Gundam DX A.E.U.G. vs Titans...... 12X456X  ZDX1 DVD0            DVD          NM00017   ZDX1 Ver.A             Capcom/Banpresto 2003         %
Moto GP......................................... XXXX56X  MGP1004-NA-HDD0-A    HDD (80GB)   NM00039   MGP1004-NA-B           Namco 2007                    HDD: Western Digital WD800BB (note only about 2.5GB is used and the remainder of the drive is 00-filled)
Netchuu Pro Yakyuu 2002......................... X23XXXX  NPY1 CD0B            CD           NM00009   NPY Ver.B              Namco 2002
Pride GP 2003................................... 123456X  PR21 DVD0            DVD          NM00011   PR21 Ver.A             Capcom 2003                   %
Quiz Mobile Suit Gundam Tou Senshi.............. 123456X  QG1                  DVD          NM00030   QG1 Ver.A              Banpresto 2006                %
Ridge Racer V Arcade Battle..................... 1XXXXXX  RRV1-A               CD           NM00001   RRV1 Ver.A             Namco 2000
   "                       ..................... 1XXXXXX  RRV1-A               CD           NM00001   RRV2 Ver.A             Namco 2000
   "                       ..................... 1XXXXXX  RRV1-A               CD           NM00001  *RRV2 Ver.B             Namco 2000
   "                       ..................... 1XXXXXX  RRV1-A               CD           NM00001   RRV3 Ver.A             Namco 2000
Sengoku Basara X Cross.......................... 123456X  BAX1 DVD0            DVD          NM00042   BAX1 VER.A             Capcom/ARC System Works 2007  % #
Smash Court Pro Tournament...................... ------X  SCP1 CD0             CD           NM00006   SCP1 Ver.A             Namco 2001
Soul Calibur II................................. 123456X  SC21 DVD0            DVD          NM00007   SC21 Ver.A             Namco 2002                    \
   "            Rev. B.......................... 123456X  SC21 DVD0B           DVD          NM00007   SC23 Ver.A             Namco 2002                    | # @ 1st & rev B DVD needs campaign memory card
   "            Rev. D.......................... 123456X  SC21 DVD0D           DVD          NM00007   SC22 Ver.D             Namco 2002                    /     rev D will allow skipping it if not available
Soul Calibur III Arcade Edition................. 123456X  SC31001-NA-DVD0-A    DVD          NM00031   SC31001-NA-A           Namco 2005                    \
   "                           ................. 123456X  SC31001-NA-DVD0-B    DVD          NM00031   SC31001-NA-A           Namco 2005                    / # % @
Sukusuku Inufuku 2 / The Dog Luck 2............. 123456X  HM-IN2               CD           NM00037   IN2 Ver.A              Hampster/Video System 2007
Super Dragon Ball Z / Chou Dragon Ball Z........ XXXX56X  DB1                  DVD          NM00027   DB1 Ver.B              Banpresto 2005
Taiko no Tatsujin 7............................. ------X  TK71 DVD0            DVD          NM00023   TK71 Ver.A             Namco 2005
Taiko no Tatsujin 8............................. ------X  TK8100-1-NA-DVD0-A   DVD          NM00033   TK81001-NA-A           Namco 2006
Taiko no Tatsujin 9............................. XXXX56X  TK9100-1-NA-DVD0-A   DVD          NM00038   TK91001-NA-A           Namco 2006
Taiko no Tatsujin 10............................ XXXX56X  TK10100-1-NA-DVD0-A  DVD          NM00041   TK101001-NA-A          Namco 2007
Taiko no Tatsujin 11............................ ------X *TK11100-1-NA-DVD0-A *DVD         *NM00044  *TK111001-NA-A          Namco 2008
Taiko no Tatsujin 12............................ ------X *TK12-HA             *HDD         *NM00051  *TK121001-NA-A          Namco 2008
Taiko no Tatsujin 12 More....................... ------X *TK12200-1-NA-HDD-A  *HDD         *NM00051  *TK121001-NA-A          Namco 2008
Taiko no Tatsujin 13............................ ------X *TK1301-NA-HDD0-A1   *HDD         *NM00056  *TK1301-NA-A            Namco 2009
Taiko no Tatsujin 14............................ ------X *T141001-NA-HDD0-A   *HDD         *NM00057  *TK141001-NA-A          Namco 2010
Taiko no Tatsujin 14 More....................... ------X *T141002-NA-HDD0-A   *HDD         *NM00057  *TK141002-NA-A          Namco 2010
Technic Beat.................................... ------X *TNB1 DVD0           *DVD         *NM000??  *TNB1 Ver.A             Arika 2002
Tekken 4........................................ 1234XXX  TEF1 DVD0            DVD          NM00004   TEF1 Ver.A             Namco 2001                    \
   "    ........................................ 1234XXX  TEF1 DVD0            DVD          NM00004   TEF1 Ver.C             Namco 2001                    |
   "    ........................................ 1234XXX  TEF1 DVD0            DVD          NM00004   TEF1 Ver.D             Namco 2001                    | %
   "    ........................................ 1234XXX  TEF1 DVD0            DVD          NM00004   TEF2 Ver.A             Namco 2001                    |
   "    ........................................ 1234XXX  TEF1 DVD0            DVD          NM00004   TEF3 Ver.A             Namco 2001                    |
   "    ........................................ 1234XXX  TEF1 DVD0            DVD          NM00004   TEF3 Ver.C             Namco 2001                    /
Tekken 5........................................ XXXX56X  TE51 DVD0            DVD          NM00019  *TE51 Ver.A             Namco 2004                    \
   "    ........................................ XXXX56X  TE51 DVD0B           DVD          NM00019   TE51 Ver.B             Namco 2004                    | @
Tekken 5.1...................................... XXXX56X *TE52 DVD0           *DVD          NM00019  *TE52 Ver.A             Namco 2005                    |
   "      ...................................... XXXX56X  TE52 DVD0B           DVD          NM00019   TE52 Ver.B             Namco 2005                    |
   "      ...................................... XXXX56X  TE52 DVD0B           DVD          NM00019   TE53 Ver.B             Namco 2005                    /
Tekken 5 Dark Resurrection...................... XXXX56X  TED1 DVD0            DVD          NM00026   TED1 VER.A             Namco 2005
   "                      ...................... XXXX56X  TED1 DVD0B           DVD          NM00026  *TED2 VER.A             Namco 2005
The Battle of Yu Yu Hakusho Dark Tournament..... ------X  YH1                  DVD          NM00035   YH1 Ver.A              Banpresto 2006
Time Crisis 3................................... 1234XXX  TST1 DVD0            DVD          NM00012   TST1 Ver.A             Namco 2002                    \ Uses 'V185 I/O PCB' for light guns
   "         ................................... 1234XXX  TST1 DVD0            DVD          NM00012   TST2 Ver.A             Namco 2002                    | or 'V221 MIU PCB' for CCD camera guns
   "         ................................... 1234XXX  TST1 DVD0            DVD          NM00012  *TST3 Ver.A             Namco 2002                    / %
Time Crisis 4................................... XXXXXX7  TSF1-HA              HDD (40GB)   NM00032   TSF1002-NA-A           Namco 2005                      Uses 'V329 NA-JV PCB' for guns
Vampire Night................................... 1234XXX  VPN1 CD0             CD           NM00003   VPN3 Ver.B             Namco/Sega/WOW Entertainment 2000
Wangan Midnight (Japan)......................... 1XXXXXX  WMN1-A               CD           NM00008   WMN1 Ver.A             Namco 2001
   "            (Japan)......................... 1XXXXXX  WMN1-A               CD           NM00008  *WMN1 Ver.B             Namco 2001
   "            (Export)........................ 1XXXXXX *WMN2-A               CD           NM00008  *WMN2 Ver.A             Namco 2001
   "            (Export)........................ 1XXXXXX *WMN2-A               CD           NM00008  *WMN2 Ver.B             Namco 2001
Wangan Midnight R............................... 1XXXXXX  WMR1-A               CD           NM00005   WMR1 Ver.A             Namco 2002
Zoids Infinity.................................. ------X  M9006212A Ver.2.02J  HDD (40GB)   NM00016   B3900076A              Taito 2004                    HDD: Maxtor Fireball 3 40GB 2F040L0
Zoids Infinity Ex............................... ------X *?                   *HDD          NM00025  *B3900098B              Taito 2005
Zoids Infinity Ex Plus.......................... ------X  M9006907A Ver.2.10J  HDD (20GB)   NM00025   B3900107A              Taito 2006                    HDD: Seagate ST320014A
-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
* - denotes these items are NOT dumped
? - unknown info

Unused NM000xx numbers:
NM00045 ?
NM00046 ?
NM00047 ?
NM00049 ?
NM00050 ?
NM00053 ?
NM00054 ?
NM00055 ?

Note about System:
This is a general guideline flag showing compatibility between the different revisions of System 246 hardware.
1-7: Tested Working
X  : Tested Not Working
-  : Not Tested
%  : 1 and 2; For testing purposes these games need a DVD drive. Using this hardware to play the actual game is
     normally impossible due to requiring controls/IO boards that can't be connected to this hardware version.
#  : 5 and 6; Working only in S246+ mode.
@  : Multiple different DVD's and/or dongles are available and are compatible in any combination.

For 'Tested Working', working is defined as "boots to the attract mode".
Some games need specific controls and are unplayable even though the game loads (eg. most games on earliest rev 246
or the driving or gun games on non-factory-provided hardware). Some games will start to boot but either get stuck
somewhere in the boot process or nothing loads after the initial booting.
The DVD drive used for these tests was a Toshiba/Samsung SD-M1802. In the case where the original drive didn't work
(i.e. factory drive was a CDROM and the disc was a DVD), this drive was substituted.
Finally, all games were tested with the SD-M1802 and worked, therefore no game requires a specific make and model of
optical drive to BOOT. But of course not all drive models will work or work for extended periods. YMMV.
These makes and models have been tested and worked.....
H-L Data Storage LG GSA-4164B Super Multi DVD Drive (September 2005)
H-L Data Storage LG GSA-4082B Super Multi DVD Drive (February 2004)
H-L Data Storage LG GCC-4120B CD-RW/DVD-ROM Drive (April 2002)
NEC DV-5800C DVD-ROM Drive (February 2004)
Sony DW-U10A (May 2003) (Factory Supplied)
Toshiba/Samsung SD-616 DVD-ROM Drive (December 2001) (Factory Supplied)
Toshiba/Samsung SD-816 DVD-ROM Drive (Factory Supplied)
Toshiba/Samsung SD-M1612 DVD-ROM Drive (January 2003) (Factory Supplied)
Toshiba/Samsung SD-M1712 DVD-ROM Drive (November 2003) (Factory Supplied)
Toshiba/Samsung SD-M1802 DVD-ROM Drive (August 2007 & December 2007) (Both factory Supplied)

Different security cart revisions of the same game are filled in with the same status even if they are not dumped as
long as at least one revision is tested.

1 = 246  1st generation using COH-H30000 or COH-H31000 Playstation 2 main board (GH-004/GH-006 with PS2 power supply)
         and custom Namco interface board 'System 246 MOTHER PCB' and Teac CD-540E CD drive
2 = 246A 2nd generation using COH-H30000 or COH-H31000 Playstation 2 main board (GH-004/GH-006 with PS2 power supply)
         and custom Namco interface board 'System 246 MOTHER(A) PCB' and Teac CD-540E CD drive
3 = 246B 3rd generation using COH-H31000 Playstation 2 main board (GH-006 without PS2 power supply) and custom Namco
         interface board 'System 246 PMOTHER PCB'
4 = 246C 4th generation using integrated single main board 'System 246 Main PCB'
5 = 256  5th generation using integrated single main board 'System 256 Main PCB'. Note some games requiring S246 will
         work in S246+ mode.
6 = 256B 6th generation using integrated single main board 'System 256 Main(B) PCB'. Note some games requiring S246
         will work in S246+ mode.
7 = S256 7th generation using integrated single main board 'Super System 256 Main PCB'. All other games are X because
         this system only runs Time Crisis 4.


Namco System 246 readme (earliest type using standard PS2 main board)
-----------------------
The info below covers the first version of Namco System 246 hardware used with
games including Ridge Racer V, Wangan Midnight, Vampire Night and Bloody Roar 3.
It uses a standard Japanese COH-H30000 Playstation 2 main board (GH-004).
A later model Playstation 2 main board COH-H31000 (GH-006) has also been tested and works fine.
The PS2 main board is interfaced to the Namco MOTHER PCB via a small adapter board.
The boot ROM is inside a MagicGate security cart and plugs into the memory cart slot.
After the boot ROM loads, some of the game data is loaded from CDROM and stored in the
RAM32 PCB. However, the CDROM is continually accessed as the game plays.
The CDROM drive is a TEAC CD-540E.


PCB Layouts
===========

Namco Interface Board (1st version)
---------------------
This PCB is used with Ridge Racer V and Wangan Midnight.

System 246 MOTHER PCB
8908960202 (8908970202)
|------------------------------------------------------------------------------|
|J1    ADM485    Ti6734                |------|                            J12 |
|                6358N       TMP95C061 | C448 |                                |
|      MAX232                          |      |                                |
|J2                                    |------|                                |
|                                      CY7C199                                 |
|                                                                    K4S281632 |
|                CXA2055P                                            K4S281632 |
|                                                                          J11 |
|J3                                             66.666MHz                      |
|                             J13              |--------|                  J10 |
|                           EPM3128            |ALTERA  |                      |
|                                              |EP20K100E                      |
|J4                         LC35256            |QC208   |                      |
|                                              |--------|                      |
|                                                                              |
|                                                                              |
|          J6                                                                  |
|                                                                              |
|J5            BATTERY                            PQ30RV31                     |
|                          J7                                  J8         J9   |
|------------------------------------------------------------------------------|
Notes:
       J1 - USB connector (not used)
       J2 - Two RCA jacks for stereo audio output
    J3/J4 - DSUB15 VGA output connectors
       J5 - 3-pin connector (joins to V257 STR PCB to J104)
       J6 - Multi-pin connector (PS2 interface adapter plugged in here)
       J7 - Playstation 2 video cable connector (input)
       J8 - 2-pin power connector (+5V input)
       J9 - 40-pin IDE flat cable connector for CDROM drive
      J10 - Multi-pin connector (this is the same as the J9 connector on the 2nd version, not used)
      J11 - Multi-pin connector (RAM32 PCB plugged in here, same as J10 on 2nd version)
      J12 - Multi-pin connector (this is the same as the J8 connector on the 2nd version, not used)
      J13 - 6-pin connector for programming EPM3128 CPLD (not populated)
TMP95C061 - Toshiba TMP95C061 TLCS-900-series 16-bit Microcontroller (internal RAM/ROM = none)
     C448 - Namco Custom C448
  EPM3128 - Altera MAX EMP3128ATC100-10 CPLD labelled 'P2AMO1'
 CXA2055P - Sony CXA2055P Video Amplifier IC
   Ti6734 - Texas Instruments LMH6734 Single Supply, Ultra High-Speed, Triple Selectable Gain Buffer
    6358N - Sanyo 6358N High-Performance Dual Operational Amplifier
 PQ30RV31 - Sharp PQ30RV31 Voltage Regulator
  CY7C199 - Cypress CY7C199-15VC 32k x8-bit SRAM
  LC35256 - Sanyo LC35256DM70 32k x8-bit SRAM
 EP20K100 - Altera APEX EP20K100EQC208-2X FPGA
K4S281632 - Samsung K4S281632B-TC1H 8M x16-bit SDRAM
   ADM485 - Analog Devices ADM485 5V Low Power EIA RS-485 Transceiver
   MAX232 - Maxim MAX232 dual serial to TTL logic level driver/receiver


Namco Interface Board (2nd version)
---------------------
This PCB is used with Bloody Roar 3 and Vampire Night.

System 246 MOTHER(A) PCB
8908961500 (8908971500)
|------------------------------------------------------------------------------|
|                             |------|                   BATTERY               |
|         J2    TMP95C061     | C448 |                                         |
|J1                           |      |    LC35256                              |
|                             |------|                            K4S281632    |
|                                               66.666MHz         K4S281632    |
|                 J11         CY7C199                                          |
|               EPM3128                        |--------|         J10          |
|                                              |ALTERA  |                      |
|                                              |EP20K100E                      |
|                                              |QC208   |         J9           |
|                                              |--------|                      |
|               Ti6734                                                         |
|                                                                              |
|    SP232EET   6358N                                                          |
|                                                                              |
|J3                     J4                                        J8           |
|                                                                              |
|               CXA2055P                                                       |
|                            PQ30RV31                                          |
|                                             J5    J6                 J7      |
|------------------------------------------------------------------------------|
Notes:
       J1 - USB connector (not used for Bloody Roar 3)
       J2 - I/O board connector
       J3 - PAC PCB connector
       J4 - Multi-pin connector (PS2 interface adapter plugged in here)
       J5 - 2-pin power connector (+5V input)
       J6 - Playstation 2 video cable connector (input)
       J7 - 40-pin IDE flat cable connector for CDROM drive
       J8 - Multi-pin connector labelled 'ROM' (not used)
       J9 - Multi-pin connector labelled 'RAM2' (not used)
      J10 - Multi-pin connector labelled 'RAM1' (RAM32 PCB plugged in here)
      J11 - 6-pin connector for programming EPM3128 CPLD (not populated)
TMP95C061 - Toshiba TMP95C061 TLCS-900-series 16-bit Microcontroller (internal RAM/ROM = none)
     C448 - Namco Custom C448
  EPM3128 - Altera MAX EMP3128ATC100-10 CPLD labelled 'P2AMO1'
 CXA2055P - Sony CXA2055P Video Amplifier IC
   Ti6734 - Texas Instruments LMH6734 Single Supply, Ultra High-Speed, Triple Selectable Gain Buffer
    6358N - Sanyo 6358N High-Performance Dual Operational Amplifier
 PQ30RV31 - Sharp PQ30RV31 Voltage Regulator
  CY7C199 - Cypress CY7C199-15VC 32k x8-bit SRAM
  LC35256 - Sanyo LC35256DM70 32k x8-bit SRAM
 EP20K100 - Altera APEX EP20K100EQC208-2X FPGA
K4S281632 - Samsung K4S281632B-TC1H 8M x16-bit SDRAM


Add-on Memory Boards (plugs into J10 on MOTHER(A) PCB)
--------------------

System246 RAM32 PCB
8908960400 (8908970400)
|--------------|
|  16M   16M   |
|              |
|      J1      |
|--------------|
Notes:
      16M  - Samsung K4S281632B-TC1H 128Mbit (2Mbit x16-bit x 4 banks) SDRAM
        J1 - Multi-pin connector
This PCB is used with Bloody Roar 3, Vampire Night and Ridge Racer V.

System246 RAM64 PCB
8908962800 (8908972800)
|--------------|
|16M   16M     |
|   16M   16M  |
|      J1      |
|--------------|
Notes:
      16M - Hyundai HY57V281620AT-P 128Mbit (2Mbit x16-bit x 4 banks) SDRAM
       J1 - Multi-pin connector
This PCB is used with Wangan Midnight and Wangan Midnight R.


PS2 Adapter Board
-----------------
This PCB is used with Bloody Roar 3, Vampire Night, Ridge Racer V and Wangan Midnight.

System246 T004 PCB
8908960501 (8908970501)
|----------------|
|     *J1        |
|    J2          |
|----------------|
Notes:
      J1 - Multi-pin connector (*located on other side of PCB, plugs into J4 on MOTHER(A) PCB)
      J2 - Multi-pin connector (plugs into PS2 main board)


I/O Board
---------
This PCB is used with Bloody Roar 3. It is bolted into the metal box and the JAMMA
edge connector sits perpendicular to the front face of the metal box protruding outwards.

System246 JAMMA PCB
8908961601 (8908971601)
|--------------------------------------------|
|TLP281-4  J6          TLP281-4     LA4705   |
|                 TLP281-4                   |
|         EPM7064                    BA3121  |
|     J5             TLP281-4                |
|     TLP281-4 TLP281-4 TLP281-4             |
|       TLP281-4 TLP281-4     VOLUME         |
|J4                               J2       J1|
|--|       J3 JAMMA            |-------------|
   |---------------------------|
Notes:
      J1 - 4-pin connector for stereo audio output
      J2 - 10-pin connector for extra buttons
      J3 - JAMMA connector
      J4 - 2-pin power connector for 5V
      J5 - 6-pin connector for programming CPLD (not populated)
      J6 - Multi-pin connector (located on other side of PCB, plugs into J2 on MOTHER(A) PCB)
 EPM7064 - Altera MAX EPM7064STC100-10 CPLD labelled 'S246 J01'
  BA3121 - Rohm BA3121 Ground Isolation Amplifier IC
TLP281-4 - Toshiba TLP281-4 Optocoupler
  LA4705 - Sanyo LA4705 2-channel BTL Power Amplifier

Connector J2 pinout       Connector J1 pinout
1 - GND                   1 - Left +
2 - NC                    2 - Left -
3 - P1 Button 4           3 - Right +
4 - P1 Button 5           4 - Right -
5 - P1 Button 6
6 - NC
7 - P2 Button 4
8 - P2 Button 5
9 - P2 Button 6
10- GND


Power Input/Video Output/Audio Output Board
-------------------------------------------
This PCB is used with Bloody Roar 3 and Vampire Night. It is bolted to the front of the metal box.

System246 PAC PCB
8908961700 (8908971700)
                         |----------------------------|
                         |                  G5LE-1    |
                         |                            |
                         |    J6   J7               L1|
|------------------------|                  J9   J10  |
|           J3     J4    J5           J8            L2|
|J1  J2              SW1                              |
|-----------------------------------------------------|
Notes:
      J1/J2 - RCA jacks for stereo audio output (mono audio is also output from the JAMMA connector on the I/O board
              for Bloody Roar 3)
         J3 - Multi-pin connector (located on other side of PCB, plugs into J3 on MOTHER(A) PCB)
         J4 - 4-pin connector (not used)
         J5 - 3-pin connector (not used)
      J6/J7 - DSUB15 VGA output connectors
         J8 - 10-pin power output connector for 5V/12V (located on other side of PCB, joins
              to PS2 main board, MOTHER(A) PCB and CDROM drive)
         J9 - 6-pin JVS power input connector
        J10 - 4-pin power connector
     G5LE-1 - Omron G5LE-1 single-pole 10A power relay (located on other side of PCB)
        SW1 - 4-position DIP switch (all OFF)
         L1 - Green LED (lights when 5 volts is present)
         L2 - Blue LED (lights when 12 volts is present)


Gun I/O Boards
--------------

V185 I/O PCB
2479961102 (2479971102)
|-----------------------------------------|
|   J601      LED1 LED2         |-------| |
|   DSW(4)JP1 |-------|         |TSSIO  | |
|     |-----| | C78   |14.746MHz|PLD    | |
|     |TSSIO| |       |PST592   |-------| |
|     |PROG | |-------|                   |
|     |-----|                             |
|                                         |
|     62256                               |
|ADM485                                   |
|J1               SLA4060               J5|
|                          J3       J4    |
|-----------------------------------------|
Notes:
      TSSIOPROG - Atmel AT29C020 EEPROM stamped 'TSSIOP8' (PLCC32)
      C78       - Namco Custom C78, actually a rebadged Hitachi HD643334 MCU, clock input 14.746MHz (PLCC84)
      TSSIOPLD  - Altera MAX EPM7128ELC84 CPLD with label 'TSSIOPLD' (PLCC84)
      SLA4060   - Sanken Electric SLA4060 NPN general purpose darlington transistor (used to drive the kick-back
                  solenoid in the gun)
      PST592    - System Reset IC (SOIC4)
      J1        - 12 pin connector for power and I/O communication joined to main board
      J3        - 12 pin connector for cabinet buttons UP/DOWN/ENTER/TEST/SERVICE/COIN etc
      J4        - 4-pin connector for pedal
      J5        - 6 position connector for gun trigger/optical signal/power/gnd)
      J601      - not used?
      JP1       - jumper set to 1-2 (lower position), labelled 'WR'
      DSW       - 4 position dipswitch block, all off

This board is used with Time Crisis 3 (System 246B/C twin CRT-screen version)
It's also used with Time Crisis II (on System 23)
Note this board requires a standard light gun.


V221 MIU PCB
2512960101 (2512970101)
additional sticker for Vampire Night says '8662969301 (JV) TMIU PCB'
|---------------------------------------------|
|J10      J9    29C020     LC35256  DSW(4)    |
|    M0105          PRG.8F                LED |
|2267     6393                            LED |
|    T082  T082                 |------|      |
|           |--------|          | C78  |   J8 |
|           |ALTERA  |          |      |      |
|J11        |MAX     |          |------|    J7|
|   LM1881  |EPM7128 |                  3771  |
|R305526    |--------|                        |
|      ZUW1R51212            14.746MHz        |
|                                 ADM485    J6|
|                                             |
|  J1       J2   J3          J4    J5         |
|---------------------------------------------|
Notes:
      2267 - JRC2267 Current limiting diode array? (SOIC8)
   R305526 - Some kind of mini transformer or regulator?
   LC35256 - Sanyo LC35256 32k x8 SRAM (SOP28)
    LM1881 - National Semiconductor LM1881 Video Sync Separator (SOIC8)
     M0105 - Matsushita Panasonic 0105 = ? (SOIC16)
      T082 - Texas Instruments T082 (=TL082) JFET-Input operational amplifier (SOIC8)
      6393 - Sanyo 6393 (LA6393) High Performance Dual Comparator (SOIC8)
    ADM485 - Analog Devices ADM485 5V Low Power EIA RS-485 Transceiver (SOIC8)
      3771 - Fujitsu MB3771 Power Supply Monitor and Master Reset IC (SOIC8)
   EPM7128 - Altera MAX EPM7128SLC84-15 PLD labelled 'TMIU1 PLD0'
    29C020 - location for 29C020 PLCC32 Flash/EP ROM (not populated)
ZUW1R51212 - Cosel ZUW1R51212 DC to DC Power Supply Module (input 9-18VDC, output +-12VDC or +24VDC)
       DSW - 4 position dipswitch block, all off
        J1 - 6-pin power input connector
        J2 - 12-pin connector (cabinet buttons UP/DOWN/ENTER/TEST/SERVICE/COIN etc)
        J3 - 4 pin connector (not used)
        J4 - 9 pin Namco female plug connector for gun (solenoid +24V/trigger/pedal/sensor)
        J5 - 5 pin connector used for I/O --> S246 communications (connects to J1 on MOTHER(A) PCB)
        J6 - 7-pin connector (not used)
        J9 - 6-pin connector (not used)
       J10 - 2-pin Namco female plug connector (not used)
       J11 - 6-pin Namco female plug connector (video input from CCD camera)
    PRG.8F - 27C1001 EPROM with label...
                                        - 'TMIU1 PRG0B' (I/O program for Vampire Night)
                                        - 'XMIU1 PRG0'  (I/O program for Time Crisis 3)
                                        - 'CSZ1 PRG0A'  (I/O program for Crisis Zone)

This PCB is used with Vampire Night and Time Crisis 3 (DX projector-screen version on System 246B/C)
It's also used with Crisis Zone (on System 23 Evolution2)
Note this board requires a CCD camera gun sensor.


JVS I/O RAYS PCB
8903960101 (8903970101)
|---------------------------------------------|
|   DSW(4)  DSW(4)        SDRAM       33.33MHz|
|                                    |-----|  |
|         EPM7064                    | SH4 |  |
|J10                 24.576MHz       |-----|  |
|    FLASH.8D  LLLLLLLL   XCS20XL             |
|                                             |
|                  KS0127B                    |
|                                             |
|                              NJM2267        |
|   ADM485                                    |
|J9                                  L        |
|                                             |
|  J8   J7     J6   J5    J4   J3   J2   J1   |
|---------------------------------------------|
Notes:
       SH4 - Hitachi HD6417750 SH4 CPU
  FLASH.8D - SHARP LH28F160S5T-L70 Flash ROM labelled 'JVS RAYS OA' (TSOP56)
     SDRAM - SAMSUNG 2M x32-bit SDRAM (TSSOP86)
   EPM7064 - Altera MAX EPM7064 CPLD labelled 'JVS RAY 1A' (TQFP100)
   XCS20XL - Xilinx Spartan XCS20XL FPGA (TQFP144)
   KS0127B - Samsung Multistandard Composite to RGB Video Decoder & Scaler (PQFP100)
   NJM2267 - New Japan Radio NJM2267 Dual Video Amplifier (TSSOP8)
    ADM485 - Analog Devices ADM485 RS-485 Transceiver (SOIC8)
       DSW - 4 position dipswitch block, all off
         L - LED
        J1 - 6-pin connector for power input
        J2 - 3-pin connector (not used)
        J3 - 2-pin connector (not used)
        J4 - 6-pin connector for CCD camera input
        J5 - 12-pin connector for cabinet buttons (UP/DOWN/ENTER/TEST/SERVICE/COIN etc)
        J6 - 4-pin connector for step pedal
        J7 - 9-pin connector for gun LED, gun motor, gun trigger
        J8 - 5-pin connector for communication with main PCB (sense, data+, data-)
        J9 - 6-pin connector (not used)
       J10 - 80-pin Namco connector (not used, probably for an add-on PCB with ROMs for diagnostics)

This PCB is used with Cobra The Arcade. The game will boot with several I/O boards but in the test mode in 'I/O TEST'
the I/O board is only reported as good with the RAYS PCB, otherwise it reports 'NG'.
This same PCB is also used with the DX rear-projection-screen version of Crisis Zone (on System 23 Evolution 2 hardware).
Note this board requires a CCD camera and infrared guns and infrared sensors.


Digital & Analog I/O boards
---------------------------
1st Revsion
FCA PCB
8662969102 (8662979102)

2nd Revision
FCA2(B) PCB
8662969200 (8662979100)
|---------------------------------------------------|
| J101         3771         J106                    |
|            4.9152MHz                              |
|    DSW(6)               D1017 D1017 D1017 D1017   |
| LED2              |-----|  D1017 D1017 D1017 D1017|
|                   | MCU |                         |
|     LEDS3-10      |     |                         |
|  PIC16F84         |-----|                         |
|   JP1 LED1                           ADM485       |
|                                                   |
|                     J102              J104        |
|---------------------------------------------------|
Notes:
      J101 - 6 pin connector for power input
      J102 - 60 pin flat cable connector
      J104 - 5 pin connector
      J106 - 30 pin flat cable connector
       JP1 - 3 pin jumper, set to 'NORM'. Alt setting 'WR'
      3771 - Fujitsu MB3771 System Reset IC (SOIC8)
  PIC16F84 - Microchip PIC16F84 PIC with sticker 'FCAP11' (SOIC20)
       MCU - Fujitsu MB90F574 Microcontroller with sticker 'FCAF11' (QFP120)
    ADM485 - Analog Devices ADM485 +5V Low Power EIA RS-485 Transceiver (SOIC8)
     D1017 - D1017 transistor (8 parts populated)

This PCB is used with Ridge Racer V, Wangan Midnight and Wangan Midnight R.
The FCA2(B) PCB is pb-free but otherwise identical to FCA PCB.


V290 FCB PCB
2582960101 (2582970101)
|---------------------------------------------------|
| J101 ADM231L    3771 J108    J106                 |
|            4.9152MHz                              |
|    DSW(6)                X     X     X    D1017   |
| LED2              |-----|   X     X    D1017 D1017|
|                   | MCU |                         |
|     LEDS3-10      |     |                         |
|  PIC16F84         |-----|                         |
|   JP1 LED1                           ADM485       |
|                                       J204        |
|                     J102                          |
|---------------------------------------------------|
Notes:
      J101 - 6 pin connector for power input
      J102 - 60 pin flat cable connector
      J104 - 5 pin connector (not populated)
      J204 - USB-B connector (also wired in-line with J104. Note the signal is still the
             normal Namco communication signal; i.e. RS485)
      J106 - 30 pin flat cable connector
      J108 - 4 pin connector
       JP1 - 3 pin jumper position (not populated)
      3771 - Fujitsu MB3771 System Reset IC (SOIC8)
  PIC16F84 - Microchip PIC16F84 PIC without sticker (SOIC20)
       MCU - Fujitsu MB90F574 Microcontroller with sticker 'FCB1 IO 0B'
    ADM485 - Analog Devices ADM485 +5V Low Power EIA RS-485 Transceiver (SOIC8)
     D1017 - D1017 transistor (X = 5 parts not populated)
   ADM231L - Analog Devices ADM231L RS-232 Driver/Receiver (SOIC16)

V290 FCB PCB is almost identical to FCA PCB. The main differences are changed internal MCU code & PIC code,
some extra/different connectors, less D1017 driver transistors and an added RS-232 IC.
The V290 FCB PCB is used with touchscreen games such as Dragon Chronicles, Druaga Online, Idol Master etc.
It supports a serial touchscreen interface, card readers and buttons.
The additional devices are supported via J108 which connects to another PCB 'XOU020-A' which contains a
Texas Instruments TMS32VC540x DSP, TSOP32 flash ROM and other components.


System246 JAMMA(B) PCB
8908962701 (8908972701)
|--------------------------------------------|
|74HC132A   J5      J6        J7    LA4705   |
|      34161                                 |
|              J8      EPM7064     BA3121    |
|        74F04                               |
|                                            |
|                                            |
|J4                         VOLUME J2    J3  |
|--|       J1 JAMMA            |-------------|
   |---------------------------|
Notes:
      J1 - JAMMA connector
      J2 - 4-pin connector for stereo audio output
      J3 - 10-pin connector for extra buttons
      J4 - 6-pin power connector for 5V/12V/GND
      J5 - 15-pin VGA connector for RGB/Sync video signals
      J6 - Left/Right RCA Audio Jacks
      J7 - 20-pin connector for flat cable joining to System 246B/C & S256
      J8 - 6-pin connector for programming CPLD (not populated)
 EPM7064 - Altera MAX EPM7064STC100-10 CPLD labelled 'S246 J01B'
  BA3121 - Rohm BA3121 Ground Isolation Amplifier IC
  LA4705 - Sanyo LA4705 2-channel BTL Power Amplifier

Connector J3 pinout       Connector J2 pinout
1 - GND                   1 - Left + \
2 - P2 Button 6           2 - Left - / Also wired to JAMMA audio output pins
3 - P2 Button 5           3 - Right +
4 - P2 Button 4           4 - Right -
5 -
6 - P1 Button 6
7 - P1 Button 5
8 - P1 Button 4
9 -
10- GND

This I/O board works with all the games that require regular joysticks and buttons which run on System 246B/C and
System 256.


Drive/Feedback Board
--------------------

V194 STR PCB
2487960103 (2487970103)
Additional sticker for Ridge Racer V: 'V257 STR PCB 2553960100'
|----------------------------------------------------------|
|         SOP44.IC16            TRANSFORMER        J105    |
| DIP42                                                    |
|    LED  N341256                  FUSE                    |
|    LED                           FUSE            BF150G8E|
|         N341256                                          |
|                                                          |
|RESET_SW      32MHz             7815                 K2682|
|   MB3771                                                 |
|J101                   DSW2(4, all off)                   |
|            MB90242A                                      |
|                       LED  MB3773    HP3150              |
|                       LED                           K2682|
|                                      HP3150              |
|            EPM7064                                       |
|J104  MAX232                          LM393               |
|       LED   JP1 O O=O                                    |
|       LED                            HP3150         K2682|
|                                                          |
|                                      HP3150              |
|                                                          |
|J103                                                      |
|                UPC358  LM393   UPC358               K2682|
|            J102                            J106          |
|----------------------------------------------------------|
Notes:
      SOP44.IC16 - Fujitsu MB29F400TC 512k x8 flash ROM labelled 'RRV3 STR-0A' (SOP44)
         EPM7064 - Altera EPM7064 CPLD labelled 'STR-DR1' (PLCC44)
         N341256 - NKK 32k x8 SRAM (SOP28)
           K2682 - 2SK2682 N-Channel Silicon MOSFET
        BF150G8E - Large power transistor(?) connected to the transformer
          UPC358 - NEC uPC358 Dual operational amplifier (SOIC8)
           LM393 - National LM393 Low Power Low Offset Voltage Dual Comparator (SOIC8)
          MAX232 - Maxim MAX232 dual serial to TTL logic level driver/receiver (SOIC16)
          HP3150 - HP 3150 Optocoupler (DIP8)
          MB3773 - Fujitsu MB3773 Power Supply Monitor with Watch Dog Timer and Reset (SOIC8)
          MB3771 - Fujitsu MB3771 System Reset IC (SOIC8)
           DIP42 - Unpopulated DIP42 socket for 27C4096 EPROM
        MB90242A - Fujitsu MB90242A 16-Bit CISC ROM-less F2MC-16F Family Microcontroller optimized for mechatronics
                   control applications (TQFP80)
            7815 - LM7815 15V voltage regulator
            J101 - 8 pin connector (purpose unknown)
            J102 - 3 pin connector input from potentiometer connected to the steering wheel mechanism
            J103 - Power input connector (5v/GND/12v)
            J104 - 6 pin connector joined with a cable to J5 on the System 246 MOTHER PCB.
                   This cable provides the feed-back connection to/from the main board.
            J105 - 110VAC power input
            J106 - DC variable power output to feed-back motor

This PCB is used with Ridge Racer V and Wangan Midnight and controls the steering feed-back motor.
When the driving games boot they test the feed-back motor by monitoring a potentiometer
connected to the steering wheel mechanism. On Ridge Racer V, if the pot is faulty or not connected
or if the drive/feedback board isn't connected (including the 110VAC input voltage), the
steering check will fail after a time-out period and the game will not continue further.
Wangan Midnight doesn't test the potentiometer and will boot without it connected, but it does
test for the presence of the feedback PCB.
Note this same PCB (with different ROMs) is also used with Mario Kart (on Triforce hardware), all of the Wangan
Midnight Maximum Tune series up to Maximum Tune 3 DX Plus and Race On! (on System 23 hardware).


System 246B Main Board Layout
-----------------------------

System246 PMOTHER PCB
8908962201 (8908972201)
|-----------J12-----J13---------------------------------------|
| 16M                                    PQ30RV31         J15 |
| 16M    |--------|                                           |
|        |ALTERA  |                BATTERY                    |
|        |EP20K100E   LC35256     RS5C348       J14           |
|        |QC208   |            32.768kHz BR9080F              |
| 16M    |--------|                                |----------|
| 16M            66.666MHz               C807U-1V86|
|                                              4MHz|
|                                    J11     MM1537|
|   |------|         J10                           |
|   | C448 |  61C256    EPM3128                    |
|   |      |                                       |
|   |------|                                       |
|                                       J9         |
|    TMP95C061                                     |
J8               CXA2055P                          |
|  MAX232    ADM485       6358N                   J7
|             J3                                   |
|--SW1--J1----J2-----J4--------J5---------J6-------|
Notes:
       SW1 - 4-position DIP Switch; 1=TEST, 2=RGB Level 3.0p-p or 0.7p-p, 3=15k/31k output, 4=sync composite/separate
        J1 - 3-pin connector (not used)
        J2 - USB connector (used to connect some I/O boards e.g for Time Crisis 3)
        J3 - 4-pin connector (not used)
        J4 - Left/right RCA jacks for unamplified stereo audio output
     J5/J6 - DSUB15 VGA output connectors
        J7 - Playstation 2 video cable connector (input)
        J8 - 20-pin flat cable connector used with 'System246 JAMMA(B) PCB'
        J9 - Multi-pin male socket connector for flat cable with female socket joining to PS2 main board
       J10 - 6-pin connector for programming EPM3128 CPLD (not populated)
       J11 - Flat cable connector joining to PS2 main board
       J12 - 40-pin IDE connector for CD/DVD/HDD drives
       J13 - 4-pin power input connector (+5V/GND)
       J14 - 34-pin connector plugged directly into PS2 main board
       J15 - 4-pin power output connector plugged directly into PS2 main board (the PS2 main board PSU is not populated)
C807U-1V86 - Toshiba TMP87PH47U microcontroller with 16k x8-bit OTP EPROM re-badged as 'SONY C8707U-1V86 (C)2000 SCEI'
             This chip is connected to J11
 TMP95C061 - Toshiba TMP95C061 TLCS-900-series 16-bit Microcontroller (internal RAM/ROM = none)
      C448 - Namco Custom C448
   EPM3128 - Altera MAX EMP3128ATC100-10 CPLD labelled 'P2AMO1'
  CXA2055P - Sony CXA2055P Video Amplifier IC
     6358N - Sanyo 6358N High-Performance Dual Operational Amplifier
  PQ30RV31 - Sharp PQ30RV31 Voltage Regulator
    61C256 - ISSI IS61C256 32k x8-bit SRAM
   LC35256 - Sanyo LC35256DM70 32k x8-bit SRAM
  EP20K100 - Altera APEX EP20K100EQC208-2X FPGA
       16M - Hyundai HY57V281620AT-P 128Mbit (2Mbit x16-bit x 4 banks) SDRAM
    ADM485 - Analog Devices ADM485 +5V Low Power EIA RS-485 Transceiver (SOIC8)
   RS5C348 - Ricoh RS5C348 RTC
   BR9080F - ROHN BR9080F 512 word x16-bit serial EEPROM
    MM1537 - Mitsumi MM1537 reset chip. No info available but pin 2 of MM1537 tied to TMP87PH47 reset pin 14 so it's a reset chip
    MAX232 - Maxim MAX232 dual serial to TTL logic level driver/receiver
   BATTERY - CR2032 3.0V battery

This revision was initially used with Tekken 4, Time Crisis 3, Battle Gear 3 and Soul Calibur II. There are reports
that the DVD-based games on this system are fussy about the model of DVD-ROM drive used. Powering the drive from an
external PSU will help in some cases.

A PS2 main board (such as GH-006) is required and plugs into J14 & J15 directly and two flat cables
from J9 & J11 also connect to the PS2 main board.
The board documented here is green. A blue PCB has been seen used in Time Crisis 3. It's not known if the PCB
is different to this one.


System 246C Main Board Layout
-----------------------------
[to-do]

System 256 Main Board Layout
----------------------------
[to-do]


For Super System 256, the main board is identical to the regular System 256 except the mode jumper
is not shorted. The regular System 256 games won't load, only displaying an error message about
the wrong mode being set. If the mode jumper is changed the board either doesn't boot at all or just
displays the same error message.

Info for Time Crisis 4
----------------------
Main Board sticker: Super System 256 Main PCB 8692960100
Graphics Chip 'EE+GS CXD9833GB'


Super System 256 I/O boards
---------------------------

8682960602 (8682970602)
NA-JV PCB
Sticker for Time Crisis 4 - 26169605 V329 NA-JV PCB
|-------------------------------------|
|CN4      CN3           CN2       CN1 |
|                                     |
|                                     |
|             LED2  |----|        CN7 |
|             LED3  |3062| 14.7456MHz |
|             LED4  |----|        CN6 |
|     CN5     LED5               LED1 |
|-------------------------------------|
Notes:
      3062 - Hitachi F3062F25V H8/3062 microcontroller stamped 'TSF1APRA' (QFP100)
      CN1  - 4 pin power input connector
      CN2  - 32 pin connector for controls (Gun Position)
      CN3  - 40 pin connector for controls (Buttons: Trigger/Pedal/Select/Test/Service/Coin/Up/Down etc)
      CN4  - 8 pin connector for I/O link to main board USB connector (uses a special cable)
      CN5  - Not populated, possibly for a standard USB connector
      CN6  - 5 pin connector (not used for Time Crisis 4)


Extra Sound board (used on Super System 256 for Time Crisis 4)
-----------------
Note the game works without this PCB.

V329 EXSOUND PCB
2616961002
(2616971002)
|---------------------|
|        CN3          |-----------|
|                              CN2|
|                                 |-|
| EPM3128             CS42516     |-|
|                                 |
|               *                 |
|                                 |
|                                 |
| CN7                      CN6    |
|---------------------------------|
Notes:
      EPM3128 - Altera Max EPM3128ATC-100 CPLD stamped 'TSF' (TQFP100)
      CS42516 - Cirrus Logic CS42516-CQZ 114 dB, 192-kHz 6-Ch CODEC with S/PDIF Receiver (QFP48)
            * - unpopulated position for H8/3062 MCU
          CN2 - RCA Sound output connector
          CN3 - 40 pin connector with perpendicular adapter board plugged into main board to connector J11
          CN6 - 2 pin connector joined with a small cable to the board where the dongle plugs in
          CN7 - 6 pin connector (unused)
***************************************************************************/


#include "emu.h"
#include "cpu/mips/mips3.h"
#include "cpu/mips/mips1.h"
#include "emupal.h"
#include "screen.h"


class namcops2_state : public driver_device
{
public:
	namcops2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	void system246(machine_config &config);
	void system256(machine_config &config);

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void ps2_map(address_map &map);

	// devices
	required_device<mips3_device> m_maincpu;

	// driver_device overrides
	virtual void video_start() override;
};


void namcops2_state::video_start()
{
}

uint32_t namcops2_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void namcops2_state::ps2_map(address_map &map)
{
	map(0x00000000, 0x01ffffff).ram(); // 32 MB RAM in consumer PS2s, do these have more?
	map(0x1fc00000, 0x1fdfffff).rom().region("bios", 0);
}

static INPUT_PORTS_START( system246 )
INPUT_PORTS_END

void namcops2_state::system246(machine_config &config)
{
	R5000LE(config, m_maincpu, 294000000); // actually R5900 @ 294 MHz
	m_maincpu->set_icache_size(16384);
	m_maincpu->set_dcache_size(16384);
	m_maincpu->set_addrmap(AS_PROGRAM, &namcops2_state::ps2_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_screen_update(FUNC(namcops2_state::screen_update));
	screen.set_size(640, 480);
	screen.set_visarea(0, 639, 0, 479);

	PALETTE(config, "palette").set_entries(65536);
}

void namcops2_state::system256(machine_config &config)
{
	system246(config);
}

#define SYSTEM246_BIOS  \
	ROM_LOAD( "r27v1602f.7d", 0x000000, 0x200000, CRC(2b2e41a2) SHA1(f0a74bbcaf801f3fd0b7002ebd0118564aae3528) )

#define SYSTEM256_BIOS  \
	ROM_LOAD( "r27v1602f.8g", 0x000000, 0x200000, CRC(b2a8eeb6) SHA1(bc4fb4e1e53adbd92385f1726bd69663ff870f1e) )

ROM_START( sys246 )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	DISK_REGION("dvd")
ROM_END

ROM_START( sys256 )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM256_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	DISK_REGION("dvd")
ROM_END

ROM_START( dragchrn )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "dc001vera.ic002", 0x000000, 0x800000, CRC(923351f0) SHA1(b34c46836af8fa7ab164156a70120da38fa1c31f) )
	ROM_LOAD( "dc001vera_spr.ic002", 0x800000, 0x040000, CRC(1f42dca9) SHA1(10f75649653b4cfa53c25f6c08308e404ed7b0f2) )

	DISK_REGION("dvd")
ROM_END

ROM_START( fghtjam )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "jam1vera.ic002", 0x000000, 0x800000, CRC(61cf3746) SHA1(165195a773bac717b5701647bca4073d86906f4e) )
	ROM_LOAD( "jam1vera_spr.ic002", 0x800000, 0x040000, CRC(5ff79918) SHA1(60146cddc3474cd4c5b51d13cf116dce1664a759) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "jam1-dvd0", 0, SHA1(c3a5814c2391a0727b7ebf5f52f08ac8b429733f) )
ROM_END

ROM_START( kinniku )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM256_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "kn1vera.ic002", 0x000000, 0x800000, CRC(17aac6c3) SHA1(dddf37e88385f01bba27496d03f053fdc33882e2) )
	ROM_LOAD( "kn1vera_spr.ic002", 0x800000, 0x040000, CRC(a601f981) SHA1(39485ab3c10f3d58a2c9651cca82a73617b2fe52) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "kn1-b", 0, SHA1(2f0f9ebe74cdafe3713890221532b4d1dc18c74f) )
ROM_END

ROM_START( kinniku2 )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM256_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "kn2vera.ic002", 0x000000, 0x800000, CRC(fb2f71f7) SHA1(29a331cc171d395ad10b352b9b30a61a455a50fe) )
	ROM_LOAD( "kn2vera_spr.ic002", 0x800000, 0x040000, CRC(9c18fa50) SHA1(1f75052cf264c3f2e5b332a755d30544d6e5f45c) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "kn2", 0, SHA1(3e1b773cc584911b673d46f9296a5b1a2cef9a45) )
ROM_END

ROM_START( netchu02 )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "npy1verb.ic002", 0x000000, 0x800000, CRC(43c0f334) SHA1(5a7f6d607ae012b8477ff32cdfd091b765264499) )
	ROM_LOAD( "npy1verb_spr.ic002", 0x800000, 0x040000, CRC(6a3374f0) SHA1(0c0845edc0ac0e9871e65caade8b4157614b81eb) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "npy1cd0b", 0, SHA1(514adcd2d4205873b3d144a05c033822344798e3) )
ROM_END

ROM_START( soulclb2 )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "sc23vera.ic002", 0x000000, 0x800000, CRC(5c537182) SHA1(ff4213db24b1200b494e6c3bd3eb7b75789e4032) )
	ROM_LOAD( "sc23vera_spr.ic002", 0x800000, 0x040000, CRC(8f548cbc) SHA1(81b844dc5873bb397cd4cd5aca101d7486d60385) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "sc21-dvd0d", 0, SHA1(9a7b1ea836adc9d78481928a3067530e0f8d74a6) )
ROM_END

ROM_START( soulcl2a )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "sc22vera.ic002", 0x000000, 0x800000, CRC(2a1031b4) SHA1(81ad0b9273734758da917c62910906f06e774bd6) )
	ROM_LOAD( "sc22vera_spr.ic002", 0x800000, 0x040000, CRC(6dd152e4) SHA1(1eb23b2c65f12b39fecf34d6b21916165441ebe4) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "sc21-dvd0d", 0, SHA1(9a7b1ea836adc9d78481928a3067530e0f8d74a6) )
ROM_END

ROM_START( soulcl2b )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "sc21vera.ic002", 0x000000, 0x800000, CRC(7e92ceeb) SHA1(0c8d9337476c04f30ed86c7a77996f81733c1953) )
	ROM_LOAD( "sc21vera_spr.ic002", 0x800000, 0x040000, CRC(f5502fdf) SHA1(064196982d855bd41bafe97db5ff5694b933016a) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "sc21-dvd0d", 0, SHA1(9a7b1ea836adc9d78481928a3067530e0f8d74a6) )
ROM_END

ROM_START( soulcl2w )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	// Guru says this disc works with any SC2x key
	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "sc23vera.ic002", 0x000000, 0x800000, CRC(5c537182) SHA1(ff4213db24b1200b494e6c3bd3eb7b75789e4032) )
	ROM_LOAD( "sc23vera_spr.ic002", 0x800000, 0x040000, CRC(8f548cbc) SHA1(81b844dc5873bb397cd4cd5aca101d7486d60385) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "sc21-dvd0b", 0, SHA1(2403a0dc6d21103957676ab2df410994c12588a3) )
ROM_END

ROM_START( soulclb3 )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "sc31001-na-a.ic002", 0x000000, 0x800000, CRC(ddbe9774) SHA1(6bb2d31cb669336345b5508bcca56936ea97c04a) )
	ROM_LOAD( "sc31001-na-a_spr.ic002", 0x800000, 0x040000, CRC(18c6f56d) SHA1(13bc6a3688985c0cd9900b063824a4af691a1b31) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "sc31001-na-dvd0-b", 0, SHA1(b46ee35083f8fcc091ce562951c55fbdbb929e4b) )
ROM_END

ROM_START( soulclb3a )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "sc31002-na-a.ic002", 0x000000, 0x840000, CRC(2ebf91ff) SHA1(01e628344b2cde2edbda9ffea53af6a63e3bddf1) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "sc31001-na-dvd0-b", 0, SHA1(b46ee35083f8fcc091ce562951c55fbdbb929e4b) )
ROM_END

ROM_START( soulclb3b )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "sc31001-na-a.ic002", 0x000000, 0x800000, CRC(ddbe9774) SHA1(6bb2d31cb669336345b5508bcca56936ea97c04a) )
	ROM_LOAD( "sc31001-na-a_spr.ic002", 0x800000, 0x040000, CRC(18c6f56d) SHA1(13bc6a3688985c0cd9900b063824a4af691a1b31) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "sc31001-na-dvd0-a", 0, SHA1(2bb8669d094d470ddf99feecd4d2026bfd54f487) )
ROM_END

ROM_START( sukuinuf )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "in2vera.ic002", 0x000000, 0x800000, CRC(bba7a744) SHA1(c1c6857317d0d6648898e9b51d4c693b83e49f16) )
	ROM_LOAD( "in2vera_spr.ic002", 0x800000, 0x040000, CRC(c43fed95) SHA1(b6001dc8ff34198400a7bf3e41e5ab73823685b0) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "hm-in2", 0, SHA1(4e2d95798a2bcc6f93bc82c364379a3936d68986) )
ROM_END

ROM_START( taiko7 )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "tk71.ic002",   0x000000, 0x800000, CRC(0560e525) SHA1(3ae378de908ec2f6472867d4d0c3c19eb51cf8bc) )
	ROM_LOAD( "tk71_spr.ic002", 0x800000, 0x040000, CRC(245233a5) SHA1(a5dd1eb0350d454396984241c1178ec708a7de55) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "tk71dvd0", 0, SHA1(622ffc8f71f50e93069a8e91b56a7e63cf98b5ae) )
ROM_END

ROM_START( taiko8 )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "tk81001-na-a.ic002", 0x000000, 0x800000, CRC(205410cf) SHA1(8379771d82c9d8b09ad593e28872107ecc0100ad) )
	ROM_LOAD( "tk81001-na-a_spr.ic002", 0x800000, 0x040000, CRC(590c8d80) SHA1(30622142428e37b8c3b91dee7fdd147d593b4d6f) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "tk8100-1-na-dvd0-a", 0, SHA1(81a2a9d7164495af825ad038fbf6f696e755ab9c) )
ROM_END

ROM_START( zoidsinf )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "b3900076a.ic002", 0x000000, 0x800000, CRC(4cdc2e4f) SHA1(7c53e519683903e5ae53823b7d0644323be23680) )
	ROM_LOAD( "b3900076a_spr.ic002", 0x800000, 0x040000, CRC(7a7bf195) SHA1(0970251ba203720b2b769d6195bff06b41931b17) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "zoidsinf", 0, SHA1(aca35eb554bf906898b3ebc27e65f652a72d63f8) )
ROM_END

ROM_START( zoidiexp )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "b3900107a.ic002", 0x000000, 0x800000, CRC(1729af4a) SHA1(df5fb0841f4a81aae68382f731b96437572cdffd) )
	ROM_LOAD( "b3900107a_spr.ic002", 0x800000, 0x040000, CRC(b3d56cd4) SHA1(1e0afeba4881892682d1f91be8e0b880ee7a7fcb) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "zoidsinf-ex-plus-ver2-10", 0, SHA1(6671afc45c8b506a15e5b4b09645b956ab4cfe99) )
ROM_END

ROM_START( taiko9 )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM256_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "tk91001-na-a.ic002", 0x000000, 0x800000, CRC(db4efc9a) SHA1(a24f10c726f5bc7313559a515d5c4c34cd129c97) )
	ROM_LOAD( "tk91001-na-a_spr.ic002", 0x800000, 0x040000, CRC(99ece8c0) SHA1(871b1c76ccc0311da04b81c59240e65117cbc9f4) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "tk9100-1-na-dvd0-a", 0, SHA1(6bd40b2c19f30a81689601c3dd46b6dac6d0a2f1) )
ROM_END

// only known System Super 256 game; if more surface the BIOS should be moved out like 246/256
ROM_START( timecrs4 )
	ROM_REGION32_LE(0x200000, "bios", 0)
	ROM_LOAD( "r27v1602f.8g", 0x000000, 0x200000, CRC(b2a8eeb6) SHA1(bc4fb4e1e53adbd92385f1726bd69663ff870f1e) )

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "tsf1002-na-a.ic002", 0x000000, 0x800000, CRC(406183a4) SHA1(dd6afaa4808254b277c5969d071f1dd0019633a0) )
	ROM_LOAD( "tsf1002-na-a_spr.ic002", 0x800000, 0x040000, CRC(e7339b66) SHA1(99a2fd5528daf11a7ea548d9de804f899a2a9c6b) )

	DISK_REGION("dvd")  // HDD for this game
	DISK_IMAGE_READONLY( "tsf1-ha", 0, SHA1(8ba7eec0d1add2192a115b295a32265c8d084aea) )
ROM_END

ROM_START( timecrs4j )
	ROM_REGION32_LE(0x200000, "bios", 0)
	ROM_LOAD( "r27v1602f.8g", 0x000000, 0x200000, CRC(b2a8eeb6) SHA1(bc4fb4e1e53adbd92385f1726bd69663ff870f1e) )

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "tsf1001-na-a.ic002", 0x000000, 0x840000, CRC(e115a2ae) SHA1(73cbb166d105809e9454e77e386c5c9b4010719c) )

	DISK_REGION("dvd")  // HDD for this game
	DISK_IMAGE_READONLY( "tsf1-ha", 0, SHA1(8ba7eec0d1add2192a115b295a32265c8d084aea) )
ROM_END

ROM_START( taiko10 )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM256_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "t101001-na-a.ic002", 0x000000, 0x800000, CRC(fa7f4c4d) SHA1(4f6b24243f2c2fdffadc7acaa3a6fb668e497606) )
	ROM_LOAD( "t101001-na-a_spr.ic002", 0x800000, 0x040000, CRC(0a2926c4) SHA1(fb3d23545b5f9a649c4a14b6424c606139723bd5) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "tk10100-1-na-dvd0-a", 0, SHA1(9aef4a6b64295a6684d56334904b4c92a20abe15) )
ROM_END

ROM_START( tekken4 )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "tef3verc.ic002", 0x000000, 0x800000, CRC(8a41290c) SHA1(2c674e3203c7b5302430b1c1115fcf591a0dcbf2) )
	ROM_LOAD( "tef3verc_spr.ic002", 0x800000, 0x040000, CRC(af248bf7) SHA1(b99193fcdad683c0bbd684f37dfea5c5412b398e) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "tef1dvd0", 0, SHA1(f39aa37156245f622a6e19e8a0e081418e247b36) )
ROM_END

ROM_START( tekken4a )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "tef2vera.ic002", 0x000000, 0x800000, CRC(6dbbde96) SHA1(101711f36fe428f3fdb5de88cb03efccebc6e68d) )
	ROM_LOAD( "tef2vera_spr.ic002", 0x800000, 0x040000, CRC(a95fd114) SHA1(669229d47d49a511ab77a6f9b8c8541c00d478cf) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "tef1dvd0", 0, SHA1(f39aa37156245f622a6e19e8a0e081418e247b36) )
ROM_END

ROM_START( tekken4b )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "tef1vera.bin", 0x000000, 0x800000, CRC(154c615b) SHA1(3823daa6dd5e8d9699f8d832d7ca690559b84e96) )
	ROM_LOAD( "tef1vera.spr", 0x800000, 0x040000, CRC(64e12053) SHA1(04383cf928b4fd82290d7cccc7b23104fbf2c2f2) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "tef1dvd0", 0, SHA1(f39aa37156245f622a6e19e8a0e081418e247b36) )
ROM_END

ROM_START( tekken4c )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "tef1verc.ic002", 0x000000, 0x840000, CRC(92697a2b) SHA1(e9ec254d52187f5be0d9be58b25821c1e63bba8e) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "tef1dvd0", 0, SHA1(f39aa37156245f622a6e19e8a0e081418e247b36) )
ROM_END

ROM_START( tekken51 )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM256_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "te51verb.ic002", 0x000000, 0x800000, CRC(b4031e38) SHA1(72ee2aea4032e9b03a735b1b6c7574233f0c7711) )
	ROM_LOAD( "te51verb_spr.ic002", 0x800000, 0x040000, CRC(683bad0d) SHA1(ef10accbdc82143c31d29e2b8b812a209b341b1b) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "te51-dvd0", 0, SHA1(2a0ac3723725572c1810b0ef4bcfa7aa114062f8) )
ROM_END

ROM_START( tekken51b )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM256_BIOS

	// this key should work with Tekken 5 or 5.1 discs, according to Guru
	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "te53verb.ic002", 0x000000, 0x800000, CRC(4d605636) SHA1(417b02d6c69f883920cd43a7278d0af183583c55) )
	ROM_LOAD( "te53verb_spr.ic002", 0x800000, 0x040000, CRC(b7094978) SHA1(1e4903cd5f594c13dad2fd74666ba35c62550044) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "te51-dvd0", 0, SHA1(2a0ac3723725572c1810b0ef4bcfa7aa114062f8) )
ROM_END

ROM_START( tekken5d )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM256_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "ted1vera.ic002", 0x000000, 0x800000, CRC(491521d1) SHA1(9c27836445690bc083c6f274a4b1a499d5677830) )
	ROM_LOAD( "ted1vera_spr.ic002", 0x800000, 0x040000, CRC(a9e1e92b) SHA1(3843d0fea2f12f14f83d0a04430bb9b01cfdef07) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "ted1dvd0b", 0, SHA1(5940cc628a1555763ef2055e518f840f9a44d123) )
ROM_END

ROM_START( zgundm )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "zga1vera.ic002", 0x000000, 0x800000, CRC(b9e0fcdc) SHA1(ed7329351e951b5a2aed893e55311018547b852b) )
	ROM_LOAD( "zga1vera_spr.ic002", 0x800000, 0x040000, CRC(8e4c715b) SHA1(a2218051f54d5ce4cdd21ef021b9acf7a384b766) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "zga1dvd0", 0, SHA1(7930e5a65f6079851438669dfb1f0e5f9e11329a) )
ROM_END

ROM_START( prdgp03 )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "pr21vera.ic002", 0x000000, 0x800000, CRC(36634ad2) SHA1(e365a79220202640e5bc80bbd8a329012f22f9c4) )
	ROM_LOAD( "pr21vera_spr.ic002", 0x000000, 0x040000, CRC(4e81ef24) SHA1(7b7b9d9a0193bcaccb1578cae9dde37fc456e6f8) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "pr21dvd0", 0, SHA1(6bad5c25996bbe68da71199fbe8377b51fe78d81) )
ROM_END

ROM_START( timecrs3 )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "tst1vera.ic002", 0x000000, 0x800000, CRC(2c7ede91) SHA1(b3d3547f5aac402da2fe76ef51dca3841a982a5e) )
	ROM_LOAD( "tst1vera_spr.ic002", 0x800000, 0x040000, CRC(ee9c8132) SHA1(fb00e102389e2163d2c7efcfefd4f680f0b4d4e8) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "tst1dvd0", 0, SHA1(f8a447d9a4224282516bea590f5217c751bdc4ae) )

	ROM_REGION(0x40000, "iocpu", 0)   // NAMCO V291 I/O PCB, HD643334 H8/3334 MCU code
	ROM_LOAD("tss-iopc.ic3", 0x000000, 0x040000, CRC(bb112fe0) SHA1(6fa5dc81d258137c1b1ad427d49d136d0bbf53fa) )
ROM_END

ROM_START( timecrs3e )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "tst2vera.ic002", 0x000000, 0x800000, CRC(5e0d136f) SHA1(a0a7ca028518cb7399c96fc03b2a0815d5b805a7) )
	ROM_LOAD( "tst2vera_spr.ic002", 0x800000, 0x040000, CRC(6dcee22f) SHA1(1b395250cf99b5228d02c06efd639f7d39adc27d) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "tst1dvd0", 0, SHA1(f8a447d9a4224282516bea590f5217c751bdc4ae) )

	ROM_REGION(0x40000, "iocpu", 0)   // NAMCO V291 I/O PCB, HD643334 H8/3334 MCU code
	ROM_LOAD("tss-iopc.ic3", 0x000000, 0x040000, CRC(bb112fe0) SHA1(6fa5dc81d258137c1b1ad427d49d136d0bbf53fa) )
ROM_END

ROM_START( timecrs3u )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "tst3vera.ic002",     0x000000, 0x800000, CRC(4f8312b4) SHA1(c7a24679677b46fd35aac1210a333ce0f0076bc6) )
	ROM_LOAD( "tst3vera_spr.ic002", 0x800000, 0x040000, CRC(1ad67985) SHA1(37a420f9a5a5467fcf7bc611f71fcd5070868b56) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "tst1dvd0", 0, SHA1(f8a447d9a4224282516bea590f5217c751bdc4ae) )

	ROM_REGION(0x40000, "iocpu", 0)   // NAMCO V291 I/O PCB, HD643334 H8/3334 MCU code
	ROM_LOAD("tss-iopc.ic3", 0x000000, 0x040000, CRC(bb112fe0) SHA1(6fa5dc81d258137c1b1ad427d49d136d0bbf53fa) )
ROM_END

ROM_START( zgundmdx )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "zdx1vera.ic002", 0x000000, 0x800000, CRC(ffcb6f3b) SHA1(57cae327a0af3f6a77291d6cda948d1349a43c00) )
	ROM_LOAD( "zdx1vera_spr.ic002", 0x800000, 0x040000, CRC(16446b28) SHA1(65bdcf216917beec7a36ff640e16aa5cf413c5e4) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "zdx1dvd0", 0, SHA1(fa21626f771106e2441c4515a0e5dff478187ccd) )
ROM_END

ROM_START( gundzaft )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "sed1vera.ic002", 0x000000, 0x800000, CRC(db52309d) SHA1(3e325dfa68dadcc2f9abd9d338e47ffa511e73f8) )
	ROM_LOAD( "sed1vera_spr.ic002", 0x800000, 0x040000, CRC(12641e0e) SHA1(64b7655f95a2e5e41b5a89998f2b858dab05ae75) )

	DISK_REGION("dvd") // V-050853
	DISK_IMAGE_READONLY( "sed1dvd0", 0, SHA1(0e6db61d94f66a4ddd7d4a3013983a838d256c5d) )
ROM_END

ROM_START( cobrata )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "cbr1verb.ic002", 0x000000, 0x800000, CRC(aecda462) SHA1(97c8e98c44d66231ee3f2527756d92dbc947b76d) )
	ROM_LOAD( "cbr1verb_spr.ic002", 0x800000, 0x040000, CRC(65aaadcf) SHA1(5cc642f71bef3b5e44db5e999e8abccca7fdaa4c) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "cbr1-ha", 0, SHA1(a20d4ace91a2f2caab0804ebdf62c87ab267239b) )
ROM_END

ROM_START( rrvac )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "rrv3vera.ic002", 0x000000, 0x800000, CRC(dd20c4a2) SHA1(07bddaac958ac62d9fc29671fc83bd1e3b27f4b8) )
	ROM_LOAD( "rrv3vera_spr.ic002", 0x800000, 0x040000, CRC(712e0e9a) SHA1(d396aaf918036ff7f909a84daefe8f651fdf9b05) )

	ROM_REGION(0xc000, "jvsio", 0)  // Namco "FCA" JVS I/O board PIC16F84 code (see namcos23.cpp for FCA details)
	ROM_LOAD( "fcap11.ic2",   0x000000, 0x004010, CRC(1b2592ce) SHA1(a1a487361053af564f6ec67e545413e370a3b38c) )
	// Fujitsu MB90F574 code, partial dumps, only last 48KB of 256KB flash was extracted
	ROM_LOAD( "fcaf11.ic4",     0x000000, 0x00c000, BAD_DUMP CRC(9794f16b) SHA1(94e1c036a6d23d39b2ad69dd1ad2cfa6163287e0) ) // almost good dump, all JVS related code and data is in place
	ROM_LOAD( "fcb1_io-0b.ic4", 0x000000, 0x00c000, BAD_DUMP CRC(5e25b73f) SHA1(fa805a422ff8793989b0ce901cc868ec1a87c7ac) ) // most JVS handling code is in undumped area

	ROM_REGION(0x80000, "steering", 0)  // Steering I/O board MB90242A code (see namcos23.cpp for steering board details)
	ROM_LOAD( "rrv3_str-0a.ic16", 0x000000, 0x080000, CRC(df8b6cac) SHA1(d45e150678218084925673e1d77edefc04135035) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "rrv1-a", 0, SHA1(77bb70407511cbb12ab999410e797dcaf0779229) )
ROM_END

ROM_START( rrvac2 )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "rrv2vera.ic002", 0x000000, 0x800000, CRC(4666f6b5) SHA1(974ed4f6c5869ecf879c0d3540db6ea576225c04) )
	ROM_LOAD( "rrv2vera_spr.ic002", 0x800000, 0x040000, CRC(8d98ef04) SHA1(3f33046a8283b918226301fcf5538729be84bfbe) )

	ROM_REGION(0xc000, "jvsio", 0)  // Namco "FCA" JVS I/O board PIC16F84 code (see namcos23.cpp for FCA details)
	ROM_LOAD( "fcap11.ic2",   0x000000, 0x004010, CRC(1b2592ce) SHA1(a1a487361053af564f6ec67e545413e370a3b38c) )
	// Fujitsu MB90F574 code, partial dumps, only last 48KB of 256KB flash was extracted
	ROM_LOAD( "fcaf11.ic4",     0x000000, 0x00c000, BAD_DUMP CRC(9794f16b) SHA1(94e1c036a6d23d39b2ad69dd1ad2cfa6163287e0) ) // almost good dump, all JVS related code and data is in place
	ROM_LOAD( "fcb1_io-0b.ic4", 0x000000, 0x00c000, BAD_DUMP CRC(5e25b73f) SHA1(fa805a422ff8793989b0ce901cc868ec1a87c7ac) ) // most JVS handling code is in undumped area

	ROM_REGION(0x80000, "steering", 0)  // Steering I/O board MB90242A code (see namcos23.cpp for steering board details)
	ROM_LOAD( "rrv3_str-0a.ic16", 0x000000, 0x080000, CRC(df8b6cac) SHA1(d45e150678218084925673e1d77edefc04135035) )

	// is this the same disc as rrvac?
	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "rrv1-a", 0, SHA1(77bb70407511cbb12ab999410e797dcaf0779229) )
ROM_END

ROM_START( rrvac1 )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "rrv1vera.ic002", 0x000000, 0x800000, CRC(e28bb0be) SHA1(9683ddc228e9aedd338cf2eb4d4373faeaea5b75) )
	ROM_LOAD( "rrv1vera_spr.ic002", 0x800000, 0x040000, CRC(81c370b7) SHA1(7693c03b544c79fa3b6e536abb32ad34cf14dfbf) )

	ROM_REGION(0xc000, "jvsio", 0)  // Namco "FCA" JVS I/O board PIC16F84 code (see namcos23.cpp for FCA details)
	ROM_LOAD( "fcap11.ic2",   0x000000, 0x004010, CRC(1b2592ce) SHA1(a1a487361053af564f6ec67e545413e370a3b38c) )
	// Fujitsu MB90F574 code, partial dumps, only last 48KB of 256KB flash was extracted
	ROM_LOAD( "fcaf11.ic4",     0x000000, 0x00c000, BAD_DUMP CRC(9794f16b) SHA1(94e1c036a6d23d39b2ad69dd1ad2cfa6163287e0) ) // almost good dump, all JVS related code and data is in place
	ROM_LOAD( "fcb1_io-0b.ic4", 0x000000, 0x00c000, BAD_DUMP CRC(5e25b73f) SHA1(fa805a422ff8793989b0ce901cc868ec1a87c7ac) ) // most JVS handling code is in undumped area

	ROM_REGION(0x80000, "steering", 0)  // Steering I/O board MB90242A code (see namcos23.cpp for steering board details)
	ROM_LOAD( "rrv3_str-0a.ic16", 0x000000, 0x080000, CRC(df8b6cac) SHA1(d45e150678218084925673e1d77edefc04135035) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "rrv1-a", 0, SHA1(77bb70407511cbb12ab999410e797dcaf0779229) )
ROM_END

ROM_START( scptour )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "scp1vera.ic002", 0x000000, 0x800000, CRC(4743a999) SHA1(97ae15d75dd9b80411d101b97dd215e31de56390) )
	ROM_LOAD( "scp1vera_spr.ic002", 0x800000, 0x040000, CRC(b7094978) SHA1(1e4903cd5f594c13dad2fd74666ba35c62550044) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "scp1cd0", 0, SHA1(19fa70ba22787704c40f0a8f27bc841218bbc99b) )
ROM_END

ROM_START( superdbz )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM256_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "db1verb.ic002", 0x000000, 0x800000, CRC(ae9aa06d) SHA1(dabb6d797f706bb3523ce4ca77e9ffb1652e845a) )
	ROM_LOAD( "db1verb_spr.ic002", 0x800000, 0x040000, CRC(baae64a1) SHA1(f82c5b1e98255976518f7b78f764e7a7bb3c9017) )

	DISK_REGION("dvd") // V-055127
	DISK_IMAGE_READONLY( "db1", 0, SHA1(e126319da6a222e81ca3db22439d06e175f2ec88) )
ROM_END

ROM_START( wanganmd )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "wmn1vera.ic002", 0x000000, 0x800000, CRC(436b08a7) SHA1(b574c25ba2d4a8b497654a7cf6491103130f1317) )
	ROM_LOAD( "wmn1vera_spr.ic002", 0x800000, 0x040000, CRC(97253f9e) SHA1(652807972c62a96ba329b400e17dabd313134392) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "wmn1-a", 0, SHA1(4254e987e71d0d4038a87f11dc1a304396b3dffc) )
ROM_END

ROM_START( wanganmr )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "wmr1vera.ic002", 0x000000, 0x800000, CRC(b431936b) SHA1(e2c543936cb5689a432662a69d0042c6179a3728) )
	ROM_LOAD( "wmr1vera_spr.ic002", 0x800000, 0x040000, CRC(b8b7539c) SHA1(f415bdc8e3ebf3b0c3d0d7607b894440e89b0fe7) )

	DISK_REGION("dvd")  // actually single-track CD-ROM
	DISK_IMAGE_READONLY( "wmr1-a", 0, SHA1(02feab4380dcc2dd95c85b209192f858bafc721e) )
ROM_END

ROM_START( vnight )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "vpn3verb.ic002", 0x000000, 0x800000, CRC(d0011dc6) SHA1(d01a418b4b83057708e8f0ac4b271112b4a24d15) )
	ROM_LOAD( "vpn3verb_spr.ic002", 0x800000, 0x040000, CRC(41169c24) SHA1(40bffbe93da65fe5512be9f80254b034a071c38b) )

	DISK_REGION("dvd")  // actually single-track CD-ROM
	DISK_IMAGE_READONLY( "vpn1cd0", 0, SHA1(714bd19eee3b31a060223003e4567e405ce04cd7) )
ROM_END

ROM_START( bldyr3b )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "br3-dongle.bin", 0x000000, 0x840000, CRC(abed2289) SHA1(e5220dbfd790b582ff6f828a636190e55d9cbc93) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "bldyr3b", 0, SHA1(1de9b14107a7a37ed31bccba17c1d062f0af5065) )
ROM_END

ROM_START( qgundam )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "qg1vera.ic002", 0x000000, 0x800000, CRC(650d55fa) SHA1(cf1210bc1f2d48c298ed19e3c6a1e5e564840e47) )
	ROM_LOAD( "qg1vera_spr.ic002", 0x800000, 0x040000, CRC(d9715f53) SHA1(e45f0eef5b82b2e1afb054a137aced0344ddbd71) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "qg1", 0, SHA1(80fe5cb325c7cfa439d66e9d264337c01559d0e5) )
ROM_END

ROM_START( minnadk )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "nm00036_znt100-1-st-a.bin", 0x000000, 0x840000, CRC(420b9ac5) SHA1(7094ca4b1840ba5f312aeaa28165cd683a1621a3) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "znt150-1-na-dvd0-b", 0, SHA1(d8343b3248e88051ca0c15d198a6dcd16eedf931) )
ROM_END

ROM_START( fateulc )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "fud1vera.ic002", 0x000000, 0x800000, CRC(892ffdd1) SHA1(4a444ed49e5c89dd0af4f026626a164b9eec61d1) )
	ROM_LOAD( "fud1vera_spr.ic002", 0x800000, 0x040000, CRC(0fca4e99) SHA1(0bd74de26f10089ee848f03093229abfa8c84663) )

	DISK_REGION("dvd")  // actually HDD for this game
	DISK_IMAGE_READONLY( "fud-hdd0-a", 0, SHA1(1189863ae0e339e11708b9660521f86b3b97bc9e) )
ROM_END

ROM_START( fateulcb )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "fates-dongle.bin", 0x000000, 0x840000, CRC(b0f15996) SHA1(8161c61f18700ddaeecd89bf3a7fb685431355e7) )

	DISK_REGION("dvd")  // actually HDD for this game
	DISK_IMAGE_READONLY( "fateulcb", 0, SHA1(073e67a5219ad53292716093b8c35deb20761c04) )
ROM_END

ROM_START( gdvsgd )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM256_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "gvs1vera.ic002", 0x000000, 0x800000, CRC(b938b96d) SHA1(e79bc7f8c234d51d1b6a34be88f34abc8205a370) )
	ROM_LOAD( "gvs1vera_spr.ic002", 0x800000, 0x040000, CRC(f2d65d54) SHA1(297726098c3723e38cbaf3a3150a4a027a9c2124) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "gvs1dvd0b", 0, SHA1(3cf9ade5495982fcb8e106e7be4067429530f864) )
ROM_END

ROM_START( gdvsgdnx )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM256_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "gnx1001-na-a.ic002", 0x000000, 0x800000, CRC(1d6d2f54) SHA1(17f6e7278e61b5b81605175d3f2df7b747ca7246) )
	ROM_LOAD( "gnx1001-na-a_spr.ic002", 0x800000, 0x040000, CRC(a999ba5c) SHA1(009a56f7be50b57bf435fb8c8b41cf14086b1d1a) )

	DISK_REGION("dvd")  // actually HDD for this game
	DISK_IMAGE_READONLY( "gnx100-1na-a", 0, SHA1(a2344f533895793a2e13198c7de0c759f0dbf817) )
ROM_END

ROM_START( yuyuhaku )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM256_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "dongle.bin",   0x000000, 0x840000, CRC(36492878) SHA1(afd14aee033cf360c07d281112566d0463d17a1f) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "yh1", 0, SHA1(ffdf1333d2c235e5fcec3780480f110afd20a7df) )
ROM_END

ROM_START( sbxc )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM246_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "bax1vera.ic002", 0x000000, 0x800000, CRC(18a6f424) SHA1(027a8d371fb6782c906434b86db9779057eaa954) )
	ROM_LOAD( "bax1vera_spr.ic002", 0x800000, 0x040000, CRC(abfb749b) SHA1(b45f8c79dd0cc0359f27c33f55626d6cad82127c) )

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "bax1_dvd0", 0, SHA1(56d58e66eeaa57ff07668000491360853b064936) )
ROM_END

ROM_START( motogp )
	ROM_REGION32_LE(0x200000, "bios", 0)
	SYSTEM256_BIOS

	ROM_REGION(0x840000, "key", ROMREGION_ERASE00)
	ROM_LOAD( "mgp1004-na-b.ic002", 0x000000, 0x840000, CRC(31cc0cc7) SHA1(269ddaa55f230c697da132b565bbca9338cf6820) )

	DISK_REGION("dvd")  // actually HDD for this game
	DISK_IMAGE_READONLY( "mgp1004-na-hdd0-a", 0, SHA1(599940b9fe7fc8e46fd80a32c7e795ae143ee645) )
ROM_END

// System 246
GAME(2001, sys246,          0, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "System 246 BIOS", MACHINE_IS_SKELETON|MACHINE_IS_BIOS_ROOT)
GAME(2000, rrvac,      sys246, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "Ridge Racer V Arcade Battle (RRV3 Ver. A)", MACHINE_IS_SKELETON)
GAME(2000, rrvac2,      rrvac, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "Ridge Racer V Arcade Battle (RRV2 Ver. A)", MACHINE_IS_SKELETON)
GAME(2000, rrvac1,      rrvac, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "Ridge Racer V Arcade Battle (RRV1 Ver. A)", MACHINE_IS_SKELETON)
GAME(2001, vnight,     sys246, system246, system246, namcops2_state, empty_init, ROT0, "Sega / Namco", "Vampire Night (VPN3 Ver. B)", MACHINE_IS_SKELETON)
GAME(2001, bldyr3b,    sys246, system246, system246, namcops2_state, empty_init, ROT0, "bootleg", "Bloody Roar 3 (bootleg)", MACHINE_IS_SKELETON)
GAME(2001, tekken4,    sys246, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "Tekken 4 (TEF3 Ver. C)", MACHINE_IS_SKELETON)
GAME(2001, tekken4a,  tekken4, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "Tekken 4 (TEF2 Ver. A)", MACHINE_IS_SKELETON)
GAME(2001, tekken4b,  tekken4, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "Tekken 4 (TEF1 Ver. A)", MACHINE_IS_SKELETON)
GAME(2001, tekken4c,  tekken4, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "Tekken 4 (TEF1 Ver. C)", MACHINE_IS_SKELETON)
GAME(2001, wanganmd,   sys246, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "Wangan Midnight (WMN1 Ver. A)", MACHINE_IS_SKELETON)
GAME(2002, dragchrn,   sys246, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "Dragon Chronicles (DC001 Ver. A)", MACHINE_IS_SKELETON)
GAME(2002, netchu02,   sys246, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "Netchuu Pro Yakyuu 2002 (NPY1 Ver. A)", MACHINE_IS_SKELETON)
GAME(2002, scptour,    sys246, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "Smash Court Pro Tournament (SCP1)", MACHINE_IS_SKELETON)
GAME(2002, soulclb2,   sys246, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "Soul Calibur II (SC23 Ver. A)", MACHINE_IS_SKELETON)
GAME(2002, soulcl2a, soulclb2, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "Soul Calibur II (SC22 Ver. A)", MACHINE_IS_SKELETON)
GAME(2002, soulcl2b, soulclb2, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "Soul Calibur II (SC21 Ver. A)", MACHINE_IS_SKELETON)
GAME(2002, soulcl2w, soulclb2, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "Soul Calibur II (SC23 world version)", MACHINE_IS_SKELETON)
GAME(2002, wanganmr,   sys246, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "Wangan Midnight R (WMR1 Ver. A)", MACHINE_IS_SKELETON)
GAME(2003, prdgp03,    sys246, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "Pride GP 2003 (PR21 Ver. A)", MACHINE_IS_SKELETON)
GAME(2003, timecrs3,   sys246, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "Time Crisis 3 (TST1)", MACHINE_IS_SKELETON)
GAME(2003, timecrs3e,timecrs3, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "Time Crisis 3 (TST2 Ver. A)", MACHINE_IS_SKELETON)
GAME(2003, timecrs3u,timecrs3, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "Time Crisis 3 (TST3 Ver. A)", MACHINE_IS_SKELETON)
GAME(2003, zgundm,     sys246, system246, system246, namcops2_state, empty_init, ROT0, "Capcom / Banpresto", "Mobile Suit Z-Gundam: A.E.U.G. vs Titans (ZGA1 Ver. A)", MACHINE_IS_SKELETON)
GAME(2004, fghtjam,    sys246, system246, system246, namcops2_state, empty_init, ROT0, "Capcom / Namco", "Capcom Fighting Jam (JAM1 Ver. A)", MACHINE_IS_SKELETON)
GAME(2004, sukuinuf,   sys246, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "Quiz and Variety Suku Suku Inufuku 2 (IN2 Ver. A)", MACHINE_IS_SKELETON)
GAME(2004, zgundmdx,   sys246, system246, system246, namcops2_state, empty_init, ROT0, "Capcom / Banpresto", "Mobile Suit Z-Gundam: A.E.U.G. vs Titans DX (ZDX1 Ver. A)", MACHINE_IS_SKELETON)
GAME(2004, zoidsinf,   sys246, system246, system246, namcops2_state, empty_init, ROT0, "Tomy / Taito", "Zoids Infinity", MACHINE_IS_SKELETON)
GAME(2005, cobrata,    sys246, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "Cobra: The Arcade (CBR1 Ver. B)", MACHINE_IS_SKELETON)
GAME(2005, gundzaft,   sys246, system246, system246, namcops2_state, empty_init, ROT0, "Capcom / Banpresto", "Gundam Seed: Federation vs. Z.A.F.T. (SED1 Ver. A)", MACHINE_IS_SKELETON)
GAME(2005, soulclb3,   sys246, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "Soul Calibur III (SC31001-NA-A key, NA-B disc)", MACHINE_IS_SKELETON)
GAME(2005, soulclb3a,soulclb3, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "Soul Calibur III (SC31002-NA-A key, NA-B disc)", MACHINE_IS_SKELETON)
GAME(2005, soulclb3b,soulclb3, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "Soul Calibur III (SC31002-NA-A key, NA-A disc)", MACHINE_IS_SKELETON)
GAME(2005, taiko7,     sys246, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "Taiko no Tatsujin 7 (TK71-NA-A)", MACHINE_IS_SKELETON)
GAME(2006, taiko8,     sys246, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "Taiko no Tatsujin 8 (TK8100-1-NA-A)", MACHINE_IS_SKELETON)
GAME(2006, qgundam,    sys246, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "Quiz Mobile Suit Gundam: Monsenshi (QG1 Ver. A)", MACHINE_IS_SKELETON)
GAME(2007, minnadk,    sys246, system246, system246, namcops2_state, empty_init, ROT0, "Namco", "Minna de Kitaeru Zenno Training (Ver. 1.50)", MACHINE_IS_SKELETON)
GAME(2008, fateulc,    sys246, system246, system246, namcops2_state, empty_init, ROT0, "Capcom / Namco", "Fate: Unlimited Codes (FUD1 ver. A)", MACHINE_IS_SKELETON)
GAME(2008, fateulcb,  fateulc, system246, system246, namcops2_state, empty_init, ROT0, "bootleg", "Fate: Unlimited Codes (bootleg)", MACHINE_IS_SKELETON)
GAME(2008, sbxc,       sys246, system246, system246, namcops2_state, empty_init, ROT0, "Capcom / Arc System Works", "Sengoku Basara X Cross", MACHINE_IS_SKELETON)

// System 256
GAME(2004, sys256,          0, system256, system246, namcops2_state, empty_init, ROT0, "Namco", "System 256 BIOS", MACHINE_IS_SKELETON|MACHINE_IS_BIOS_ROOT)
GAME(2005, tekken51,   sys256, system256, system246, namcops2_state, empty_init, ROT0, "Namco", "Tekken 5.1 (TE51 Ver. B)", MACHINE_IS_SKELETON)
GAME(2005, tekken51b,tekken51, system256, system246, namcops2_state, empty_init, ROT0, "Namco", "Tekken 5.1 (TE53 Ver. B)", MACHINE_IS_SKELETON)
GAME(2005, tekken5d,   sys256, system256, system246, namcops2_state, empty_init, ROT0, "Namco", "Tekken 5 Dark Resurrection (TED1 Ver. A)", MACHINE_IS_SKELETON)
GAME(2005, superdbz,   sys256, system256, system246, namcops2_state, empty_init, ROT0, "Banpresto / Spike", "Super Dragon Ball Z (DB1 Ver. B)", MACHINE_IS_SKELETON)
GAME(2006, kinniku,    sys256, system256, system246, namcops2_state, empty_init, ROT0, "Namco", "Kinnikuman Muscle Grand Prix (KN1 Ver. A)", MACHINE_IS_SKELETON)
GAME(2006, taiko9,     sys256, system256, system246, namcops2_state, empty_init, ROT0, "Namco", "Taiko no Tatsujin 9 (TK91001-NA-A)", MACHINE_IS_SKELETON)
GAME(2006, yuyuhaku,   sys256, system256, system246, namcops2_state, empty_init, ROT0, "Banpresto", "The Battle of Yu Yu Hakusho: Shitou! Ankoku Bujutsukai!", MACHINE_IS_SKELETON)
GAME(2006, zoidiexp,   sys246, system246, system246, namcops2_state, empty_init, ROT0, "Tomy / Taito", "Zoids Infinity EX Plus (ver. 2.10)", MACHINE_IS_SKELETON)
GAME(2007, kinniku2,   sys256, system256, system246, namcops2_state, empty_init, ROT0, "Namco", "Kinnikuman Muscle Grand Prix 2 (KN2 Ver. A)", MACHINE_IS_SKELETON)
GAME(2007, motogp,     sys256, system256, system246, namcops2_state, empty_init, ROT0, "Namco", "Moto GP (MGP1004-NA-B)", MACHINE_IS_SKELETON)
GAME(2007, taiko10,    sys256, system256, system246, namcops2_state, empty_init, ROT0, "Namco", "Taiko no Tatsujin 10 (T101001-NA-A)", MACHINE_IS_SKELETON)
GAME(2008, gdvsgd,     sys256, system256, system246, namcops2_state, empty_init, ROT0, "Capcom / Bandai", "Gundam vs. Gundam (GVS1 Ver. A)", MACHINE_IS_SKELETON)
GAME(2009, gdvsgdnx,   sys256, system256, system246, namcops2_state, empty_init, ROT0, "Capcom / Bandai", "Gundam vs. Gundam Next", MACHINE_IS_SKELETON)

// System Super 256
GAME(2006, timecrs4,   sys256, system256, system246, namcops2_state, empty_init, ROT0, "Namco", "Time Crisis 4 (World, TSF1002-NA-A)", MACHINE_IS_SKELETON)
GAME(2006, timecrs4j,timecrs4, system256, system246, namcops2_state, empty_init, ROT0, "Namco", "Time Crisis 4 (Japan, TSF1001-NA-A)", MACHINE_IS_SKELETON)
