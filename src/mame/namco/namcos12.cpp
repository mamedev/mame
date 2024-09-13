// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

  Namco System 12 - Arcade Playstation-based Hardware
  ===================================================
  Driver by smf
  Board notes by The Guru
  H8/3002 and Golgo13 support by R. Belmont based on work by The_Author and DynaChicken
  JVS Support by R. Belmont

  Notes:
    - technodr, kartduel: Calibrate controls by turning on the service mode switch (F2) while holding service coin (9).

  Issues:
    - not all games work due to either banking, dma or protection issues.
    - graphics are glitchy in some games.
    - kartduel frame rate is choppy, it freezes every half second
        memo: this is due to the link PCB not being implemented, it keeps trying to communicate over the network causing hitching.
        Forcing 0x1f781701 (which is within the shared RAM between the PCBs) to 0xff will stop the hitching.
    - technodr: printer not emulated. To play the game, press F2 to enter the test menu, navigate to GAME OPTIONS and disable
      the printer by setting "PRINTER" to OFF.
    - sws2001 crashes at random times in-game, and always after the opening video. You can spam insert coin and start to get in-game.
    - sws2000 also crashes after opening video
    - toukon3 has garbage graphics

Namco System 12 - Arcade Playstation-based Hardware
===================================================

Game & software revision                 Company/Year            CPU board   Mother board        Daughter board   Keycus
------------------------------------------------------------------------------------------------------------------------
Aqua Rush (AQ1/VER.A1)                   (C) Namco,        1999  COH-700     SYSTEM12 MOTHER(C)  SYSTEM12 M5F2    KC053
Attack Pla-Rail (AP1/VER.A)              (C) Tomy/Namco,   1998  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M5F4    KC032
Derby Quiz My Dream Horse (MDH1/VER.A2)  (C) Namco,        1998  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M10X64  KC035
Ehrgeiz (EG1/VER.A)                      (C) Square/Namco, 1998  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M4F6    KC021
Ehrgeiz (EG2/VER.A)                      (C) Square/Namco, 1998  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M4F6    KC021
Ehrgeiz (EG3/VER.A)                      (C) Square/Namco, 1998  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M4F6    KC021
Fighting Layer (FTL3/VER.A)              (C) Arika/Namco,  1998  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M5F4    KC037
Fighting Layer (FTL0/VER.A)              (C) Arika/Namco,  1998  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M5F4    KC037
Ghoul Panic (OB2/VER.A)                  (C) Namco/Raizing,1999  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M4F6    KC045
Oh! Bakyuuun (OB1/VER.A)                 (C) Namco/Raizing,1999  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M4F6    KC045
Golgo 13 (GLG1/VER.A)                    (C) Raizing/Namco,1999  COH-700     SYSTEM12 MOTHER(C)  SYSTEM12 M8F6    KC054
Golgo 13 Kiseki no Dandou (GLS1/VER.A)   (C) Raizing/Namco,2000  COH-700     SYSTEM12 MOTHER(C)  SYSTEM12 M8F6    KC059
Kaiun Quiz (KW1/VER.A1)                  (C) Namco/MOSS,   1999  COH-700     SYSTEM12 MOTHER(C)  SYSTEM12 M10X64  KC050
Kart Duel (KTD1/VER.A)                   (C) Namco,        2000  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M4F0    KC057
Libero Grande (LG2/VER.A)                (C) Namco,        1997  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M8F2F   KC014
Techno Drive (TH1/VER.B)                 (C) Namco,        1998  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M8F2F   KC056
Mr Driller (DRI3/VER.A2)                 (C) Namco,        1999  COH-700     SYSTEM12 MOTHER(C)  SYSTEM12 M8F2F   KC048
Mr Driller (DRI1/VER.A2)                 (C) Namco,        1999  COH-700     SYSTEM12 MOTHER(C)  SYSTEM12 M8F2F   KC048
Paca Paca Passion (PPP1/VER.A2)          (C) Produce/Namco,1999  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M8F2F   KC038
Paca Paca Passion Special (PSP1/VER.A)   (C) Produce/Namco,1999  COH-700     SYSTEM12 MOTHER(C)  SYSTEM12 M5F2    KC052
Paca Paca Passion 2 (PKS1/VER.A)         (C) Produce/Namco,1999  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M5F2    KC046
Gunbarl (GNB4/VER.A)                     (C) Namco,        1999  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M4F0    KC042
Point Blank 2 (GNB5/VER.A)               (C) Namco,        1999  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M4F0    KC042
Soul Calibur (SOC11/VER.A2)              (C) Namco,        1998  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M5F2    KC020
Soul Calibur (SOC11/VER.B)               (C) Namco,        1998  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M5F2    KC020
Soul Calibur (SOC11/VER.C)               (C) Namco,        1998  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M5F4    KC020
Soul Calibur (SOC13/VER.B)               (C) Namco,        1998  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M5F4    KC020
Soul Calibur (SOC13/VER.C)               (C) Namco,        1998  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M5F4    KC020
Soul Calibur (SOC14/VER.B)               (C) Namco,        1998  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M5F4    KC020
Soul Calibur (SOC14/VER.C)               (C) Namco,        1998  COH-700     SYSTEM12 MOTHER     JO 11-04-98      none
Super World Stadium '98 (SS81/VER.A)     (C) Namco,        1998  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M4F6    KC025
Super World Stadium '99 (SS91/VER.A3)    (C) Namco,        1999  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M5F4    KC043
Super World Stadium 2000 (SS01/VER.A3)   (C) Namco,        2000  COH-700     SYSTEM12 MOTHER(C)  SYSTEM12 M5F4    KC055
Super World Stadium 2001 (SS11/VER.A2)   (C) Namco,        2001  COH-716     SYSTEM12 MOTHER(C)  SYSTEM12 F2M5    KC061
Tenkomori Shooting (TKM1/VER.A1)         (C) Namco,        1998  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M4F6    KC036
Tenkomori Shooting (TKM2/VER.A1)         (C) Namco,        1998  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M4F6    KC036
Tekken 3 (TET1/VER.A)                    (C) Namco,        1996  COH-700     SYSTEM12 MOTHER     SYSTEM12 M8F2F   KC006
Tekken 3 (TET2/VER.A)                    (C) Namco,        1996  COH-700     SYSTEM12 MOTHER     SYSTEM12 M8F2F   KC006
Tekken 3 (TET3/VER.A)                    (C) Namco,        1996  COH-700     SYSTEM12 MOTHER     SYSTEM12 M8F2F   KC006
Tekken 3 (TET3/VER.B)                    (C) Namco,        1996  COH-700     SYSTEM12 MOTHER     SYSTEM12 M8F2F   KC006
Tekken 3 (TET2/VER.C)                    (C) Namco,        1996  COH-700     SYSTEM12 MOTHER     SYSTEM12 M8F2F   KC006
Tekken 3 (TET2/VER.D)                    (C) Namco,        1996  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M8F2F   KC006
Tekken 3 (TET3/VER.D)                    (C) Namco,        1996  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M8F2F   KC006
Tekken 3 (TET1/VER.E1)                   (C) Namco,        1996  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M8F2F   KC006
Tekken 3 (TET2/VER.E1)                   (C) Namco,        1996  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M8F2F   KC006
Tekken Tag Tournament (TEG3/VER.C1)      (C) Namco,        1999  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M8F4    KC044
Tekken Tag Tournament (TEG3/VER.B)       (C) Namco,        1999  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M8F4    KC044
Toukon Retsuden 3 (TR1/VER.A)            (C) Namco,        1997  COH-700     SYSTEM12 MOTHER(B)  SYSTEM12 M8F2F   KC019
Truck Kyosokyoku (TKK2/VER.A)            (C) Namco,        2000  COH-700     SYSTEM12 MOTHER(C)  SYSTEM12 M8F6    KC056

Wanted Games
------------
Aerosmith - Quest for Fame              (C) Namco,        2001
http://www.bandainamcogames.co.jp/am/vg/questforfame/

Soul Calibur Ver.B                      (C) Namco,        1998 (flyer date)
https://flyers.arcade-museum.com/?page=flyer&db=videodb&id=1009&image=1
https://flyers.arcade-museum.com/?page=flyer&db=videodb&id=4766&image=1
Probably doesn't exist

Submarines                              (C) Namco,        1999 (flyer date)
https://flyers.arcade-museum.com/?page=flyer&db=videodb&id=5531&image=1
This was cancelled, only the flyer exists.
It was shown only at an amusement show. Possibly a prototype still exists. Possibly not.

Tekno Werk                              (C) Namco,        1999
A music game with two 17-key Yamaha keyboards side by side for 2 players
Video: https://www.youtube.com/watch?v=uGlt0HM67l8

The Namco System 12 system comprises 3 mandatory PCB's....
MOTHER PCB  - This is the main PCB. It holds all sound circuitry, sound ROMs, program ROMs, shared RAM, bank-switching
              logic, controller/input logic (including sound CPU) and some video output circuitry.
              There are three known revisions of this PCB. The 3rd revision has some extra circuitry for analog controls,
              some of the PALs are different and it has the capability to use 1x TSOP56 ROM in the program location,
              instead of the regular TSOP40/48 ROMs. The first 2 revisions appear to be identical.
CPU PCB     - All games use the exact same PCB. Contains main CPU/RAM and GPU/Video RAM
ROM PCB     - There are nine known revisions of this PCB (so far). They're mostly identical except for the type and
              number of ROMs used. Most have a PAL and a CPLD known as a 'KEYCUS'. Some also have an additional CPLD for
              protection.

And 4 game-specific/optional PCBs....
NETWORK PCB - Used to connect 2 PCBs together using standard USB cables. The board plugs in where the CPU board would
              normally be, and the CPU board plugs into another connector on the Network PCB. Seems to be only used on
              Kart Duel, Libero Grande, Tekken 3 and Ehrgeiz (so far?)
GUN I/F PCB - Used to connect and control the light guns. The board plugs in where the CPU board would
              normally be, and the CPU board plugs into another connector on the GUN I/F PCB. The guns plug into this
              PCB directly, no extra gun hardware is needed. Used only on Ghoul Panic and Point Blank 2 so far.
CDXA PCB    - Used on Truck Kyosokyoku to control the CDROM drive. The board plugs in where the CPU board would
              normally be, and the CPU board plugs into another connector on the CDXA PCB.
V233 DECO PCB-This is the I/O board used by Truck Kyosokyoku. It contains a custom Namco C78 (Hitachi H8/3334) and one
              EPROM. A similar board might also be required for Attack Pla-Rail as it will not boot without some kind
              of I/O board.

Each game has a 3 or 4 digit letter code assigned to it which is printed on a small sticker and placed on the underside
of the main PCB.
The 4 digit code is then proceeded by a number (generally 1, 2 or 3), then 'Rev.' then A/B/C/D/E which denotes the software
revision, and in some cases a sub-revision such as 1 or 2 (usually only listed in the test mode).
The first 1 denotes a Japanese version. 2 (and maybe 3) denotes a World version. For general use there are no other numbers
used other than 1, 2, or 3. There is one exception so far. Point Blank 2 was first produced on System 11 hardware. To
solve a naming conflict, the System 12 version uses a '4' to denote a Japanese version and a '5' to denote a World version.
For World versions usually only the main program is changed, the rest of the ROMs are the Japanese region code '1' ROMs.
See the Main PCB and ROM Daughterboard PCB texts below for more details on ROM naming specifics.

Main PCB
--------
1st Revision
SYSTEM12 MOTHER PCB 8661960105 (8661970105)
SYSTEM12 MOTHER PCB 8661960106 (8661970106)

2nd Revision
SYSTEM12 MOTHER (B) PCB 8661961000 (8661971000)

3rd Revision
SYSTEM12 MOTHER (C) PCB (System11 Plus) 8661961101 (8661971101)
Rev. (C) Main Board is only slightly different to the other revs. Mostly some PALs are different and the
PCB is wired for 1x 32M TSOP56 FlashROM (replaces 2E & 2J) and 2x 16M TSOP48 FlashROMs (replaces 2R/2P/2N/2L).
One other difference is that the motherboard has a serial number sticker on it and the program ROMs also contain
that same serial number, so all the games running on the Rev. C mother board have serialised ROMs. I'm not sure why,
they're not exactly _easily_ traceable and no one cares either way ;-)
  |--------------------------------------------------------------|
|-|     SOUND.11S       |--------------|               AT28C16   |
|                       |--------------|                         |
|                     |------|J103                               |
|          |-----|    | C352 |                     %PRG.2R       |
|          |H8/  |    |------|                                   |
|J         |3002 |                2061ASC-1         PRG.2P       |
|A         |-----|                   14.7456MHz                  |
|M                                                 %PRG.2N       |
|M                     |-------|                                 |
|A                     |       |                    PRG.2L       |
|                      | C416  |                                 |
|    J110 N341256      |       |                                 |
|         N341256      |-------|                   %PRG.2J       |
|      DSW(2)                                 MOT1 *PRG.2F R4543 |
|-|                                                %PRG.2E       |
  |                                                              |
  |                           MOT6    MOT5    MOT2   MOT4        |
|-|                                                       3V_BATT|
|N   *D6345     6734 6358 MB88347                                |
|A                                            MOT7   MOT3        |
|M                                                               |
|C    LC78832M      CXA1779P                                     |
|O        3414A                        J111            PQ30RV21  |
|4                          |--------------------------|         |
|8                          |--------------------------|         |
|-|                                                              |
  |LA4705  J109     ADM485                                       |
  |        VOL   J108     J107           J106   J105      J104   |
  |--------------------------------------------------------------|
(logic chips omitted from the PCB layout)

Notes:
      *            : These parts only on MOTHER (C) PCB
      %            : These parts not on MOTHER (C) PCB
      J103         : Custom Namco connector for plug-in ROM PCB
      J104         : JAMMA2 Power Connector (only 5V/12V/0V used). Note! This has common connections and is
                     powered by the JAMMA connector as well. It's presence is purely for convenience when
                     connecting to a JVS-only cabinet.
      J105/J106    : JAMMA2 15 PIN VGA D-Type connector (x2) for video output (output sync can be 15kHz or 31kHz)
                     To enable this feature, ensure a VGA cable is connected to this port when the PCB is booted.
                     The PCB will detect it and output the video using this connector. If there is an option in test
                     mode in the 'DISPLAY TEST' option, enabling 'Non-Interlace' mode will give much sharper graphics.
      J107         : JAMMA2 RCA Connectors (x4) for twin stereo audio output
      J108         : JAMMA2 USB Connector for Controls. Note! To enable JVS/JAMMA2 controls, a Namco JVS adapter
                     module must be connected to this port. This port is NOT compatible with common PC USB controls.
      J109         : 3-pin connector (populated only on MOTHER (C) PCB, labelled on PCB 'AUDIO OUT')
      J110         : 4-pin connector (purpose unknown, 2 outer pins labelled on PCB 'VCC' & 'GND')
      J111         : Custom Namco connector for plug-in CPU PCB
      MB88347      : Fujitsu MB88347 8-bit 8-Channel D/A Converter with OP AMP Output Buffers (SOIC16)
      6358         : Toshiba TD6358N Frequency Synthesizer for TV/CATV (SOIC8)
      6734         : MAX734 +12V 120mA Flash Memory Programming Supply Switching Regulator (SOIC8)
      3414A        : NJM3414A 70mA Dual Op Amp (SOIC8)
      LC78832M     : Sanyo LM78832M 2-Channel 16-Bit D/A Converter LSI with 2 On-Chip Digital Filters (SOIC20)
      2061ASC-1    : IC Designs 2061ASC-1 Programmable Clock Generator (SOIC16), master input of 14.7456MHz
      R4543        : EPSON Real Time Clock Module (SOIC14)
      DSW(2)       : 2-Position DIP Switch (All OFF)
      N341256      : NKK N341256 32k x8 SRAM (x2, SOJ28)
      H8/3002      : Sound CPU, Hitachi H8/3002 HD6413002F17 (QFP100), clocked at 16.737MHz
      C416         : Namco custom C416 (QFP176)
      C352         : Namco custom C352 PCM sound chip (QFP100), clocked at 29.168MHz, from 2061 pin 9
      MOT1         : PALCE 22V10H (PLCC28, labelled 'S12MOT1A')
                     *Replaced by PALCE 22V10H (PLCC28, labelled 'S12MOT1C') on MOTHER (C) PCB
      MOT2         : PALCE 22V10H (PLCC28, labelled 'S12MOT2A')
                     *Replaced by PALCE 22V10H (PLCC28, labelled 'S12MOT2C') on MOTHER (C) PCB
      MOT3         : PALCE 16V8H  (PLCC20, labelled 'S12MOT3A')
                     *Replaced by PALCE 16V8H (PLCC20, labelled 'S12MOT3C') on MOTHER (C) PCB
      MOT4         : PALCE 22V10H (PLCC28, labelled 'S12MOT4A')
      MOT5         : PALCE 22V10H (PLCC28, labelled 'S12MOT5A')
      MOT6         : PALCE 22V10H (PLCC28, labelled 'S12MOT6A')
      MOT7         : PALCE 16V8H  (PLCC20, labelled 'S12MOT7A')
      AT28C16      : Atmel AT28C16 2k x8 EEPROM (SOIC24)
      ADM485       : Analog Devices ADM485JR 5V Low Power EIA RS-485 Transceiver (SOIC8)
      CXA1179P     : Sony CXA1779P TV/Video Circuit RGB Pre-Driver (DIP28)
      LA4705       : Sanyo LA4705 15W 2-Channel Power Amplifier (SIP18)
      VOL          : Master Volume Potentiometer
      PQ30RV21     : Sharp PQ30RV21 5V to 3.3V Voltage Regulator
      3V_BATT      : Sony CR2032 3 Volt Coin Battery
      D6345        : NEC uPD6345 Serial 8-bit Shift Register IC (SOIC16)

      PRG.2L/PRG.2P: Main program ROMs, Intel 28F016S5 2M x8 FLASHROM (both TSOP40)
                     or Fujitsu 29F016A 2M x8 FLASHROM (both TSOP48)
                     These ROMs are populated on the following games....

                                                            Test Mode S/W
                     Game                      Sticker      Revision         MOTHER PCB
                     --------------------------------------------------------------------
                     Aqua Rush                 AQ1 Ver.A    AQ1/VER.A1       MOTHER(C) (ROMs serialised)
                     Attack Pla-Rail           AP1 Ver.A    none             MOTHER(B)
                     Derby Quiz My Dream Horse MDH1 Ver.A   MDH1/VER.A2      MOTHER(B)
                     Ehrgeiz                   EG3 Ver.A    EG3/VER.A        MOTHER(B)
                     Golgo 13                  GLG1 Ver.A   GLG1/VER.A       MOTHER(C) (ROMs serialised)
                     Golgo 13 Kiseki no Dandou GLS1 Ver.A   GLS1/VER.A       MOTHER(C) (ROMs serialised)
                     Kaiun Quiz                KW1 Ver.A    KW1/VER.A1       MOTHER(C) (ROMs serialised)
                     Libero Grande             LG2 Ver.A    LG2/VER.A        MOTHER(B)
                     Mr Driller                DRI3 Ver.A   DRI3/VER.A2      MOTHER(C) (ROMs serialised)
                     Mr Driller                DRI1 Ver.A   DRI1/VER.A2      MOTHER(C) (ROMs serialised)
                     Oh! Bakyuuun              OB1 Ver.A    OB1/VER.A        MOTHER(B)
                     Paca Paca Passion         PPP1 Ver.A   PPP1/VER.A2      MOTHER(B)
                     Paca Paca Passion 2       PKS1 Ver.A   PKS1/VER.A       MOTHER(B)
                     Paca Paca Passion SP      PSP1 Ver.A   PSP1/VER.A       MOTHER(C) (ROMs serialised)
                     Point Blank 2             GNB5 Ver.A   none             MOTHER(B)
                     Soul Calibur              SOC1 Ver.A   SOC11/VER.A2     MOTHER(B)
                     Soul Calibur              SOC3 Ver.B   SOC13/VER.B      MOTHER(B)
                     Soul Calibur              SOC1 Ver.C   SOC11/VER.C      MOTHER(B)
                     Soul Calibur              SOC4 Ver.B   SOC14/VER.B      MOTHER(B)
                     Super World Stadium 2000  SS01 Ver.A   SS01/VER.A3      MOTHER(C) (ROMs serialised)
                     Super World Stadium 2001  SS11 Ver.A   SS11/VER.A2      MOTHER(C) (ROMs serialised)
                     Tekken Tag Tournament     TEG3 Ver.B   TEG3/VER.B       MOTHER(B)
                     Tekken Tag Tournament     TEG3 Ver.B   TEG3/VER.C1      MOTHER(B)
                     Truck Kyosokyoku          TKK2 Ver.A   none             MOTHER(C) (ROMs serialised)

                     Note: Games with 'none' for S/W Revision have a date in the test menu in the 'OTHER' item

      PRG.2N/PRG.2R: Main program ROMs \ Intel 28F016S5 2M x8 FLASHROM (for 2N, TSOP40)
                                       / Intel 28F008SA 1M x8 FLASHROM (for 2R, TSOP40)
                     These ROMs are not populated on any System12 PCB dumped so far. Probably they are completely
                     unused on any game since they were phased out with the Rev (C) main board.

             PRG.2F: Main program ROM (only on MOTHER (C) PCB) - Intel E28F0320 4M x8 FLASHROM (TSOP56)
                     This ROM is not populated on any System12 PCB dumped so far.

      PRG.2J/PRG.2E: Main program ROMs, Fujitsu 29F016 2M x8 FLASHROM (both TSOP48)
                     These ROMs are populated on the following games....

                                                            Test Mode S/W
                     Game                      Sticker      Revision         MOTHER PCB
                     --------------------------------------------------------------------
                     Attack Pla-Rail           AP1 Ver.A    none             MOTHER(B)
                     Ehrgeiz                   EG2 Ver.A    EG2/VER.A        MOTHER(B)
                     Fighting Layer            FTL3 Ver.A   FTL3/VER.A       MOTHER(B)
                     Fighting Layer            FTL1 Ver.A   FTL0/VER.A       MOTHER(B)
                     Ghoul Panic               OB2 Ver.A    OB2/VER.A        MOTHER(B)
                     Soul Calibur              SOC1 Ver.B   SOC11/VER.B      MOTHER(B)
                     Soul Calibur              TET1 Ver.C   SOC14/VER.C      MOTHER  (factory upgraded from Tekken 3 to Soul Calibur)
                     Super World Stadium '98   SS81 Ver.A   SS81/VER.A       MOTHER(B)
                     Super World Stadium '99   SS91 Ver.A   SS91/VER.A3      MOTHER(B)
                     Tekken 3                  TET1 Ver.A   TET1/VER.E1      MOTHER(B)
                     Tekken 3                  TET2 Ver.A   TET2/VER.A       MOTHER
                     Tekken 3                  TET3 Ver.A   TET3/VER.A       MOTHER
                     Tekken 3                  TET3 Ver.B   TET3/VER.B       MOTHER
                     Tekken 3                  TET3 Ver.D   TET3/VER.D       MOTHER
                     Tenkomori Shooting        TKM1 Ver.A   TKM1/VER.A1      MOTHER(B)
                     Tenkomori Shooting        TKM2 Ver.A   TKM2/VER.A1      MOTHER(B)

      SOUND.11S    : Fujitsu 29F400TA-90 512k x8 EEPROM (holds H8/3002 program, SOP44)
                     This ROM is not labelled with any other markings except the manufacturer and chip type. The ROM labels
                     in the archives are simply made up for convenience.


ROM Daughterboard PCB
----------------------
This PCB holds the remainder of the ROMs, used for graphics and the 3D geometry.
There are 8 known types of ROM daughterboards used on S12 games (so far).
All of the PCBs are the same size (approx 2" x 7") containing one custom connector and some mask ROMs/flash ROMs, a PLCC
PAL and a KEYCUS (which is a PLCC44 CPLD) and in some cases an extra TQFP CPLD.
The PCBs are named with a special coding. First a letter M, denoting mask ROM (always SOP44), then a number denoting how
many ROMs of that type, then another letter F, denoting FLASHROM (always TSOP40/48/56), then a number denoting how many
ROMs of that type. That number is always the maximum amount of that ROM type that can be used on the PCB. The actual number
of ROMs populated on the PCB can be less.

********
*Type 1*
********
SYSTEM 12 M4F6 PCB 8661960901 (8661970901)
|----------------------------------------------------------|
|WAVE0.IC2      IC4  FL3L.IC5                              |
|                       FL3U.IC6                           |
|                          FL2L.IC7             ROM0L.IC12 |
|                             FL2U.IC8                     |
|                                FL1L.IC9                  |
|WAVE1.IC1                          FL1U.IC10   ROM0U.IC11 |
|             IC3  |-------------------|                   |
|                  |-------------------|                   |
|----------------------------------------------------------|
Notes:
      This PCB has zero Ohm resistor jumpers to set the ROM sizes.
      Note X means 'not populated', O means 'populated'
      |----------|----|----|   |----------|----|
      |  MASK    | R6 | R7 |   |  FLASH   | R8 |
      |----------|----|----|   |----------|----|
      | WAVE 32M | X  | O  |   |  NORMAL  | X  |
      |      64M | O  | X  |   |32M FLASH | O  |
      |----------|----|----|   |----------|----|

      WAVEx: 64M SOP44 mask ROM (R6 populated)
      FLx  : Intel E28F016 TSOP40 16M FlashROM or Fujitsu 29F016A TSOP48 16M FlashROM (R8 not populated)
      ROM0x: 64M SOP44 mask ROM (size fixed at 64M, no configure options)
      IC3  : MACH211 CPLD (PLCC44, labelled 'KEYCUS' and stamped with 'KC' & a 3-digit number which is different per game.)
      IC4  : PALCE 16V8H (PLCC20, PCB labelled 'A_DECO', chip stamped 'A DECO')

This PCB is used on:

              Software
Game          Revision     PCB                                            KEYCUS   ROMs Populated
------------------------------------------------------------------------------------------------------------------
Ehrgeiz       EG2/VER.A    SYSTEM 12 M4F6 PCB 8661960901 (8661970901)     KC021    EG1 WAVE0
                                                                                   EG1 ROM0U, EG1 ROM0L
                                                                                   EG1 FL1U, EG1 FL1L, EG1 FL2U
                                                                                   EG1 FL2L, EG1 FL3U, EG1 FL3L

Ehrgeiz       EG3/VER.A    SYSTEM 12 M4F6 PCB 8661960901 (8661970901)     KC021    EG1 WAVE0
                                                                                   EG1 ROM0U, EG1 ROM0L
                                                                                   EG1 FL1U, EG1 FL1L, EG1 FL2U
                                                                                   EG1 FL2L, EG1 FL3U, EG1 FL3L

Tenkomori     TKM1/VER.A1  SYSTEM 12 M4F6 PCB 8661960901 (8661970901)     KC036    TKM1 WAVE0, TKM1 WAVE1
Shooting                                                                           TKM1 ROM0U, TKM1 ROM0L
                                                                                   TKM1 FL1U, TKM1 FL1L, TKM1 FL2U
                                                                                   TKM1 FL2L, TKM1 FL3U, TKM1 FL3L

Tenkomori     TKM2/VER.A1  SYSTEM 12 M4F6 PCB 8661960901 (8661970901)     KC036    TKM1 WAVE0, TKM1 WAVE1
Shooting                                                                           TKM1 ROM0U, TKM1 ROM0L
                                                                                   TKM1 FL1U, TKM1 FL1L, TKM1 FL2U
                                                                                   TKM1 FL2L, TKM1 FL3U, TKM1 FL3L

Ghoul Panic   OB2/VER.A    Same PCB but sticker says....                  KC045    OB1 WAVEB.IC2
                           'SYSTEM 12 M4F0 PCB 8661962000'                         OB1 PRG0U.IC11, OB1 PRG0L.IC12
                           (i.e no FlashROMs)

Oh! Bakyuuun  OB1/VER.A    Same PCB but sticker says....                  KC045    OB1 WAVEB.IC2
                           'SYSTEM 12 M4F0 PCB 8661962000'                         OB1 PRG0U.IC11, OB1 PRG0L.IC12
                           (i.e no FlashROMs)
                           (i.e no FlashROMs)

Kart Duel     KTD1/VER.A   Same PCB but sticker says....                  KC057    KTD1 WAVE-0.IC2
                           'SYSTEM 12 M4F0 PCB 8661962000'                         KTD1 ROM-0U.IC11, KTD1 ROM-0L.IC12
                           (i.e no FlashROMs)

Point Blank 2 GNB5/VER.A   Same PCB but sticker says....                  KC042    GNB1 WAVEB.IC2
                           'SYSTEM 12 M4F0 PCB 8661962000'                         GNB1 PRG0U.IC11, GNB1 PRG0L.IC12
                           (i.e no FlashROMs)                                      (note these ROMs contains the same data as)
                                                                                   (the ones used on System 11 Point Blank 2)

Gunbarl       GNB4/VER.A   Same PCB but sticker says....                  KC042    GNB1 WAVEB.IC2
                           'SYSTEM 12 M4F0 PCB 8661962000'                         GNB1 PRG0U.IC11, GNB1 PRG0L.IC12
                           (i.e no FlashROMs)                                      (note these ROMs contains the same data as)
                                                                                   (the ones used on System 11 Point Blank 2)

Super World   SS81/VER.A   SYSTEM 12 M4F6 PCB 8661960901 (8661970901)     KC025    SS81 WAVE0, SS81 WAVE1
Stadium '98                                                                        SS81 FL1U, SS81 FL1L, SS81 FL2U
                                                                                   SS81 FL2L, SS81 FL3U, SS81 FL3L

********
*Type 2*
********
SYSTEM 12 M5F2 PCB 8661961300 (8661971300)
|----------------------------------------------------------|
|WAVE0.IC2      IC4     FL4.IC5   FL3.IC6                  |
|                                                          |
|                                        ROM0.IC7          |
|                                                          |
|                                            ROM1.IC8      |
|WAVE1.IC1                                                 |
|             IC3  |-------------------|          ROM2.IC9 |
|                  |-------------------|                   |
|----------------------------------------------------------|
Notes:
      This PCB has zero Ohm resistor jumpers to set the ROM sizes.
      Note X means 'not populated', O means 'populated'
      |----------|----|----|
      |  MASK    | R6 | R7 |
      |----------|----|----|
      | WAVE 32M | X  | O  |
      |      64M | O  | X  |
      |----------|----|----|

      |----------|----|    |------|---|---|---|---|---|----|   The Flash ROMs can be configured to 3.3V 64M Strata Flash
      |  FLASH   | R8 |    |FLASH |R11|R12|C1 |C2 |D1 |REG1|   when used with some extra components (voltage regulator/diode
      |----------|----|    |------|---|---|---|---|---|----|   /resistors/caps etc). However, the extra component positions
      |  NORMAL  | X  |    | 5V   | O | O | X | X | X | X  |   are not populated on any of the games dumped so far.
      |64M STRATA| O  |    | 3.3V | X | X | O | O | O | O  |
      |----------|----|    |------|---|---|---|---|---|----|

      WAVEx: 64M SOP44 mask ROM (R6 populated)
      FLx  : Intel E28F320J5 TSOP56 32M FlashROM (R11 & R12 populated)
      ROMx : 64M SOP44 mask ROM (size fixed at 64M, no configure options)
      IC3  : MACH211 CPLD or Cypress CY37064 CPLD (PLCC44, labelled 'KEYCUS' and stamped with 'KC' & a 3-digit number which
             is different per game.)
      IC4  : PALCE 16V8H (PLCC20, PCB labelled 'A_DECO', chip stamped 'M5F4')

This PCB is used on:

              Software
Game          Revision     PCB                                            KEYCUS   ROMs Populated
------------------------------------------------------------------------------------------------------------------
Soul Calibur  SOC11/VER.A2 SYSTEM 12 M5F2 PCB 8661961300 (8661971300)     KC020    SOC1 WAVE0
                                                                                   SOC1 ROM0, SOC1 ROM1, SOC1 ROM2
                                                                                   SOC1 FL3, SOC1 FL4

Soul Calibur  SOC11/VER.B  SYSTEM 12 M5F2 PCB 8661961300 (8661971300)     KC020    SOC1 WAVE0
                                                                                   SOC1 ROM0, SOC1 ROM1, SOC1 ROM2
                                                                                   SOC1 FL3, SOC1 FL4

Aqua Rush     AQ1/VER.A1   Same PCB but sticker says....                  KC053    AQ1 WAVE0
                           'SYSTEM 12 M5F0 PCB 8661962200'                         AQ1 ROM0, AQ1 ROM1
                           (i.e no FlashROMs)

Paca Paca     PKS1/VER.A   Same PCB but sticker says....                  KC046    PKS1 WAVE0, PKS1 WAVE 1
Passion 2                  'SYSTEM 12 M5F0 PCB 8661962200'                         PKS1 ROM0, PKS1 ROM1, PKS1 ROM2
                           (i.e no FlashROMs)

Paca Paca     PSP1/VER.A   Same PCB but sticker says....                  KC052    PSP1 WAVE0, PSP1 WAVE 1
Passion Special            'SYSTEM 12 M5F0 PCB 8661962200'                         PSP1 ROM0, PSP1 ROM1, PSP1 ROM2
                           (i.e no FlashROMs)

********
*Type 3*
********
SYSTEM 12 M5F4 PCB 8661961200 (8661971200)
|----------------------------------------------------------|
|WAVE0.IC2      IC4                                        |
|                    FL4L.IC5              ROM0.IC9        |
|                         FL4H.IC6                         |
|                             FL3L.IC7        ROM1.IC10    |
|                                 FL3H.IC8                 |
|WAVE1.IC1                                       ROM2.IC11 |
|             IC3  |-------------------|                   |
|                  |-------------------|                   |
|----------------------------------------------------------|
Notes:
      This PCB has zero Ohm resistor jumpers to set the ROM sizes.
      Note X means 'not populated', O means 'populated'
      |----------|----|----|   |----------|----|
      |  MASK    | R6 | R7 |   |  FLASH   | R8 |
      |----------|----|----|   |----------|----|
      | WAVE 32M | X  | O  |   |  NORMAL  | X  |
      |      64M | O  | X  |   |64M STRATA| O  |
      |----------|----|----|   |----------|----|

      WAVEx: 64M SOP44 mask ROM (R6 populated)
      FLx  : Fujitsu 29F016 TSOP48 16M FlashROM (R8 not populated)
      ROMx : 64M SOP44 mask ROM (size fixed at 64M, no configure options)
      IC3  : MACH211 CPLD (PLCC44, labelled 'KEYCUS' and stamped with 'KC' & a 3-digit number which is different per game.)
      IC4  : PALCE 16V8H (PLCC20, PCB labelled 'A_DECO', chip stamped 'M5F4')

This PCB is used on:

               Software
Game           Revision     PCB                                            KEYCUS   ROMs Populated
-------------------------------------------------------------------------------------------------------------------
Attack Pla-Rail AP1/VER.A   SYSTEM 12 M5F4 PCB 8661961200 (8661971200)     KC032    AP1 WAVE0, AP1 WAVE1
                                                                                    AP1 ROM0, AP1 ROM1, AP1 ROM2
                                                                                    AP1 FL4L, AP1 FL4H
                                                                                    AP1 FL3L, AP1 FL3H

Fighting Layer FTL3/VER.A   SYSTEM 12 M5F4 PCB 8661961200 (8661971200)     KC037    FTL1 WAVE0, FTL1 WAVE1
                                                                                    FTL1 ROM0, FTL1 ROM1, FTL1 ROM2
                                                                                    FTL1 FL4L, FTL1 FL4H
                                                                                    FTL1 FL3L, FTL1 FL3H

Fighting Layer FTL0/VER.A   SYSTEM 12 M5F4 PCB 8661961200 (8661971200)     KC037    FTL1 WAVE0, FTL1 WAVE1
                                                                                    FTL1 ROM0, FTL1 ROM1, FTL1 ROM2
                                                                                    FTL1 FL4L, FTL1 FL4H
                                                                                    FTL1 FL3L, FTL1 FL3H

Super World    SS91/VER.A3  SYSTEM 12 M5F4 PCB 8661961200 (8661971200)     KC027    SS91 WAVE0, SS91 WAVE1
Stadium '99                                                         (sticker KC043) SS91 ROM0, SS91 ROM1
                                                                                    SS91 FL4L, SS91 FL4H
                                                                                    SS91 FL3L, SS91 FL3H

Super World    SS01/VER.A3  SYSTEM 12 M5F4 PCB 8661961200 (8661971200)     KC055    SS01 WAVE0, SS01 WAVE1
Stadium 2000                                                                        SS01 ROM0, SS01 ROM1, SS01 ROM2
                                                                                    SS01 FL3L, SS01 FL3H

Soul Calibur   SOC13/VER.B  SYSTEM 12 M5F4 PCB 8661961200 (8661971200)     KC020    SOC1 WAVE0
                                                                                    SOC1 ROM0, SOC1 ROM1, SOC1 ROM2
                                                                                    SOC1 FL4L, SOC1 FL4H
                                                                                    SOC1 FL3L, SOC1 FL3H

Soul Calibur   SOC11/VER.C  SYSTEM 12 M5F4 PCB 8661961200 (8661971200)     KC020    SOC1 WAVE0
                                                                                    SOC1 ROM0, SOC1 ROM1, SOC1 ROM2
                                                                                    SOC1 FL4L, SOC1 FL4H
                                                                                    SOC1 FL3L, SOC1 FL3H

Soul Calibur   SOC14/VER.B  SYSTEM 12 M5F4 PCB 8661961000 (8661971000)     none     SOC1 WAVE0
                                                                                    SOC1 ROM0, SOC1 ROM1, SOC1 ROM2
                                                                                    SOC1 FL4L, SOC1 FL4H
                                                                                    SOC1 FL3L, SOC1 FL3H


********
*Type 4*
********
SYSTEM 12 M8F2F PCB 8661960301 (8661970301)
|----------------------------------------------------------|
|WAVE0.IC5   ROM0L.IC6        ROM0U.IC9                    |
|                                                          |
|                 ROM1L.IC7        ROM1U.IC10    FL3U.IC13 |
|                                                          |
|                      ROM2L.IC8        ROM2U.IC11         |
|WAVE1.IC4                IC2         IC3        FL3L.IC12 |
|             IC1  |-------------------|                   |
|                  |-------------------|                   |
|----------------------------------------------------------|
Notes:
      This PCB has zero Ohm resistor jumpers to set the ROM sizes.

      |-------|----|
      | W128M | R3 | Wave ROMs are 128M if R3 is populated
      |  W32M | R4 | Wave ROMs are 32M if R4 is populated
      |  W64M | R5 | Wave ROMs are 64M if R5 is populated
      |  M64M | R6 | Mask ROMs are 64M if R6 is populated
      |  M32M | R7 | Mask ROMs are 32M if R7 is populated
      |-------|----|

      WAVEx: 32M/64M SOP44 mask ROM
      FLx  : Fujitsu 29F016 TSOP48 16M FlashROM (size fixed at 16M, no configure options)
      ROMx : 32M/64M SOP44 mask ROM
      IC1  : MACH211 CPLD or Cypress CY37064 CPLD (PLCC44, labelled 'KEYCUS' and stamped with 'KC' & a 3-digit number which
             is different per game.)
      IC2  : 74F139 logic IC
      IC3  : 74HC157 logic IC

This PCB is used on:

               Software
Game           Revision      PCB                                            KEYCUS   ROMs Populated           Jumpers
---------------------------------------------------------------------------------------------------------------------
Tekken 3       TET1/VER.E1   SYSTEM 12 M8F2F PCB 8661960301 (8661970301)    KC006    TET1 WAVE0, TET1 WAVE1   R4, R7
                                                                                     TET1 ROM0U, TET1 ROM1U
                                                                                     TET1 ROM2U, TET1 ROM0L
                                                                                     TET1 ROM1L, TET1 ROM2L
                                                                                     TET1 FL3L,  TET1 FL3U

Tekken 3       TET2/VER.A    SYSTEM 12 M8F2F PCB 8661960301 (8661970301)    KC006    TET1 WAVE0, TET1 WAVE1   R4, R7
                                                                                     TET1 ROM0U, TET1 ROM1U
                                                                                     TET1 ROM2U, TET1 ROM0L
                                                                                     TET1 ROM1L, TET1 ROM2L
                                                                                     TET1 FL3L,  TET1 FL3U

Tekken 3       TET3/VER.A    SYSTEM 12 M8F2F PCB 8661960301 (8661970301)    KC006    TET1 WAVE0, TET1 WAVE1   R4, R7
                                                                                     TET1 ROM0U, TET1 ROM1U
                                                                                     TET1 ROM2U, TET1 ROM0L
                                                                                     TET1 ROM1L, TET1 ROM2L
                                                                                     TET1 FL3L,  TET1 FL3U

Tekken 3       TET3/VER.B    SYSTEM 12 M8F2F PCB 8661960301 (8661970301)    KC006    TET1 WAVE0, TET1 WAVE1   R4, R7
                                                                                     TET1 ROM0U, TET1 ROM1U
                                                                                     TET1 ROM2U, TET1 ROM0L
                                                                                     TET1 ROM1L, TET1 ROM2L
                                                                                     TET1 FL3L,  TET1 FL3U

Paca Paca      PPP1/VER.A2   Same PCB but sticker says....                  KC038    PPP1 WAVE0, PPP1 WAVE1   R5, R7
Passion                      'SYSTEM 12 M8F0 PCB 8661961700'                         PPP1 ROM0U, PPP1 ROM1U
                             (i.e no FlashROMs)                                      PPP1 ROM2U, PPP1 ROM0L
                                                                                     PPP1 ROM1L, PPP1 ROM2L

Mr Driller     DRI3/VER.A2   Same as Paca Paca Passion                      KC048    DRI1 WAVE0               R5, R7
                                                                                     DRI1 ROM0U, DRI1 ROM0L

Mr Driller     DRI1/VER.A2   Same as Paca Paca Passion                      KC048    DRI1 WAVE0               R5, R7
                                                                                     DRI1 ROM0U, DRI1 ROM0L

Libero Grande  LG2/VER.A     SYSTEM 12 M8F2F PCB 8661960301 (8661970301)    KC014    LG1 WAVE0                R4, R7
                                                                                     LG1 ROM0U, LG1 ROM0L
                                                                                     LG1 FL3L, LG1 FL3U

Toukon         TR1/VER.A     SYSTEM 12 M8F2F PCB 8661960301 (8661970301)    KC019    TR1 WAVE0, TR1 WAVE1    R4, R7
Retsuden 3                                                                           TR1 ROM0U, TR1 ROM0L

********
*Type 5*
********
SYSTEM 12 M8F4 PCB 8661961901 (8661971901)
|----------------------------------------------------------|
|          IC3                              *ROM0O.IC13    |
|               FLEL.IC4    FLOL.IC6         ROM0E.IC9     |
|                                                          |
| WAVE0.IC1         FLEU.IC5    FLOU.IC7    *ROM1O.IC14    |
|*WAVE1.IC12                                 ROM1E.IC10    |
|                                       IC8                |
|             IC2  |-------------------|    *ROM2O.IC15    |
|                  |-------------------|     ROM2E.IC11    |
|----------------------------------------------------------|
Notes:
      This PCB has zero Ohm resistor jumpers to set the ROM sizes.
      Note X means 'not populated', O means 'populated'
      |----------|----|----|
      |   MASK   | R2 | R3 |
      |----------|----|----|
      | WAVE 32M | O  | X  |
      |      64M | X  | O  |
      |----------|----|----|

      *    : These parts on other side of PCB
      WAVEx: 32M/64M SOP44 mask ROM
      FLx  : Fujitsu 29F016 TSOP48 16M FlashROM (size fixed at 16M, no configure options)
      ROMx : 64M SOP44 mask ROM (size fixed at 64M, no configure options)
      IC2  : MACH211 CPLD (PLCC44, labelled 'KEYCUS' and stamped with 'KC' & a 3-digit number which is different per game.)
      IC3  : PALCE 22V10H (PLCC28, labelled 'S12M840A')
      IC8  : ALTERA MAX EPM7128STC100-10 FPGA (TQFP100, labelled 'S12M841')

This PCB is used on:

               Software
Game           Revision      PCB                                            KEYCUS   ROMs Populated           Jumper   Notes
-----------------------------------------------------------------------------------------------------------------------------------
Tekken Tag     TEG3/VER.C1   SYSTEM 12 M8F4 PCB 8661961901 (8661971901)     KC044    TEG1 WAVE0, TEG1 WAVE1   R3
Tournament                                                                           TEG1 ROM0E, TEG1 ROM1E
                                                                                     TEG1 ROM2E, TEG1 ROM0O
                                                                                     TEG1 ROM1O, TEG1 ROM2O
                                                                                     TEG1 FLEL, TEG1 FLEU
                                                                                     TEG1 FLOL, TEG1 FLOU

Tekken Tag     TEG3/VER.B    SYSTEM 12 M8F4 PCB 8661961901 (8661971901)     KC044    TEG1 WAVE0, TEG1 WAVE1   R3       IC8 labelled 'S12M841A'
Tournament                                                                           TEG1 ROM0E, TEG1 ROM1E            ROM board can not be
                                                                                     TEG1 ROM2E, TEG1 ROM0O            swapped to a different
                                                                                     TEG1 ROM1O, TEG1 ROM2O            TEG3 revision
                                                                                     TEG1 FLEL, TEG1 FLEU
                                                                                     TEG1 FLOL, TEG1 FLOU

********
*Type 6*
********
SYSTEM 12 M8F6 PCB 8661961800 (8661971800)
|----------------------------------------------------------|
|          IC3                              *ROM0U.IC14    |
|               FL3L.IC4    FL4U.IC7         ROM0L.IC10    |
|                                                          |
| WAVE0.IC1       FL3U.IC5    FL5L.IC8      *ROM1U.IC15    |
|*WAVE1.IC13                                 ROM1L.IC11    |
|                   FL4L.IC6    FL5U.IC9                   |
|             IC2  |-------------------|    *ROM2U.IC16    |
|                  |-------------------|     ROM2L.IC12    |
|----------------------------------------------------------|
Notes:
      This PCB has zero Ohm resistor jumpers to set the ROM sizes.
      Note X means 'not populated', O means 'populated'
      |----------|----|----|
      |   MASK   | R2 | R3 |
      |----------|----|----|
      | WAVE 32M | O  | X  |
      |      64M | X  | O  |
      |----------|----|----|

      *    : These parts on other side of PCB
      WAVEx: 32M/64M SOP44 mask ROM
      FLx  : Fujitsu 29F016 TSOP48 16M FlashROM (size fixed at 16M, no configure options)
      ROMx : 64M SOP44 mask ROM (size fixed at 64M, no configure options)
      IC2  : Cypress CY37064 CPLD (PLCC44, labelled 'KEYCUS' and stamped with 'KC' & a 3-digit number which is different
             per game.)
      IC3  : PALCE 22V10H (PLCC28, PCB labelled 'A_DECO', chip stamped 'S12M8F6')

This PCB is used on:

               Software
Game           Revision      PCB                                            KEYCUS   ROMs Populated           Jumper
---------------------------------------------------------------------------------------------------------------------
Golgo 13       GLG1/VER.A    SYSTEM 12 M8F6 PCB 8661961800 (8661971800)     KC054    GLG1 WAVE0, GLG1 WAVE1   R3
                                                                                     GLG1 ROM0L, GLG1 ROM1L
                                                                                     GLG1 ROM2L, GLG1 ROM0U
                                                                                     GLG1 ROM1U, GLG1 ROM2U
                                                                                     GLG1 FL3L, GLG1 FL3U
                                                                                     GLG1 FL4L, GLG1 FL4U
                                                                                     GLG1 FL5L, GLG1 FL5U

Golgo 13       GLS1/VER.A    SYSTEM 12 M8F6 PCB 8661961800 (8661971800)     KC059    GLS1 WAVE0, GLS1 WAVE1   R3
Kiseki no Dandou                                                                     GLS1 ROM0L, GLS1 ROM1L
                                                                                     GLS1 ROM2L, GLS1 ROM0U
                                                                                     GLS1 ROM1U, GLS1 ROM2U
                                                                                     GLS1 FL3L, GLS1 FL3U
                                                                                     GLS1 FL4L, GLS1 FL4U
                                                                                     GLS1 FL5L, GLS1 FL5U

Truck          none          Same PCB but sticker says....                  KC056    TKK1 WAVE0               R3
Kyosokyoku     (sticker:     'SYSTEM 12 M8F6-4 PCB 8661962400'                       TKK1 ROM0L, TKK1 ROM1L
               TKK2 VER.A)                                                           TKK1 ROM2L, TKK1 ROM0U
                                                                                     TKK1 ROM1U, TKK1 ROM2U
                                                                                     TKK1 FL3L, TKK1 FL3U


********
*Type 7*
********
SYSTEM 12 M10X64 PCB 8661961500 (8661971500)
|----------------------------------------------------------|
|WAVE0                                                     |
|              ROM3L ROM3U ROM2L ROM2U   ROM1L    ROM0L    |
|         IC4                                              |
|                                        ROM1U    ROM0U    |
|                                                          |
|WAVE1                                                     |
|           KEYCUS |-------------------|                   |
|                  |-------------------|                   |
|----------------------------------------------------------|
Notes:
      This PCB has zero Ohm resistor jumpers to set the ROM sizes.
      Note X means 'not populated', O means 'populated'
      |----------|----|----|
      |   MASK   | R4 | R5 |
      |----------|----|----|
      | WAVE 32M | O  | X  |
      |      64M | X  | O  |
      |----------|----|----|

      WAVEx : 32M/64M SOP44 mask ROM
      ROMx  : 64M SOP44 mask ROM (size fixed at 64M, no configure options)
      KEYCUS: CY37064 CPLD (PLCC44, labelled 'KEYCUS' and stamped with 'KC' & a 3-digit number which is different per game.)
      IC4   : PALCE 16V8H (PLCC20, PCB labelled 'A_DECO', chip stamped 'S12M10X')

This PCB is used on:

               Software
Game           Revision      PCB                                            KEYCUS   ROMs Populated
-----------------------------------------------------------------------------------------------------------
Derby Quiz My  MDH1/VER.A2   SYSTEM 12 M10X64 PCB 8661961500 (8661971500)   KC035    MDH1 WAVE0
Dream Horse                                                                          MDH1 ROM0L, MDH1 ROM1L
                                                                                     MDH1 ROM2L, MDH1 ROM0U
                                                                                     MDH1 ROM1U, MDH1 ROM2U

Kaiun Quiz     KW1/VER.A1    SYSTEM 12 M10X64 PCB 8661961500 (8661971500)   KC050    KW1 WAVE0
                                                                                     KW1 ROM0L, KW1 ROM1L
                                                                                     KW1 ROM2L, KW1 ROM0U
                                                                                     KW1 ROM1U, KW1 ROM2U

********
*Type 8*
********
JO 11-04-98
|----------------------------------------------------------|
|WAVE0.IC2 IC4                                             |
|           FL4.IC5  ROM0-1.IC9A ROM1-1.IC10A  ROM2-1.IC11A|
|                                                          |
|           FL3.IC6  ROM0.IC9    ROM1.IC10     ROM2.IC11   |
|                                                          |
|WAVE1.IC1                                                 |
|           KEYCUS |-------------------|                   |
|                  |-------------------|                   |
|----------------------------------------------------------|
Notes:
      This PCB _almost_ looks like a bootleg. There are no Namco markings, no Namco part numbers, and all ROMs are unmarked
      except for the original manufacturer part number. The PCB does not have zero Ohm resistor jumpers, but instead, blobs
      of solder to jumper pads which are not labelled anywhere on the PCB.
      There is a location for a KEYCUS chip, but it's not populated. There is a PAL but it's not marked except for the
      original manufacturer part number. The main PCB used by this game was a Tekken 3 which has been converted. The game
      it was converted to (Soul Calibur) was more popular but has an unusual ROM board that might not have been available
      and this PCB might have been produced to replace it, perhaps at a lower cost as well?
      The actual dump of the ROMs on this PCB match the original Soul Calibur dump exactly (a type 2 ROM PCB), even though
      the ROM sizes and types are different.

      WAVEx : MR27C3252CZ 32M SOP44 mask ROM
      ROMx  : MR27C3252CZ 32M SOP44 mask ROM
      FLx   : MR27C3252CZ 32M SOP44 mask ROM
      KEYCUS: Not populated
      IC4   : GAL16V8B (PLCC20, no other markings)

This PCB is used on:

               Software
Game           Revision      PCB                                            KEYCUS   ROMs Populated
-----------------------------------------------------------------------------------------------------------
Soul Calibur   SOC14/VER.C   JO 11-04-98                                    none     WAVE0, WAVE1
                                                                                     ROM0, ROM0-1, ROM1
                                                                                     ROM1-1, ROM2, ROM2-1
                                                                                     FL3, FL4

********
*Type 9*
********
SYSTEM 12 F2M5 PCB 8661962600 (8661972600)
|----------------------------------------------------------|
|WAVE0.IC2      IC4                                        |
|                                     ROM0.IC7             |
|                   FL1L.IC5   FL1U.IC6                    |
|                                           ROM1.IC8       |
|                                                          |
|WAVE1.IC1                                         ROM2.IC9|
|             IC3  |-------------------|                   |
|                  |-------------------|                   |
|----------------------------------------------------------|
Notes:
      WAVEx: 64M SOP44 mask ROM
      FLx  : Intel Flash DA28F640J5 64M FlashROM (SSOP56)
      ROMx : 64M SOP44 mask ROM
      IC3  : CY37064 CPLD (PLCC44, labelled 'KEYCUS' and stamped with 'KC' & a 3-digit number which is different per game.)
      IC4  : GAL22V10 (PLCC28, PCB labelled 'S12F2M', chip stamped 'S12F2M')

This PCB is used on:

               Software
Game           Revision     PCB                                            KEYCUS   ROMs Populated
-------------------------------------------------------------------------------------------------------------------
Super World    SS11/VER.A2  SYSTEM 12 F2M5 PCB 8661962600 (8661972600)     KC061    SS01 WAVE0, SS01 WAVE1
Stadium 2001                                                                        SS01 ROM0, SS01 ROM1, SS01 ROM2
                                                                                    SS11FL1L, SS11FL1U


CPU PCB
-------

There are 2 known types (so far). Type 1 is the most common.

Type 1
------
COH-700 GP-15 (manufactured by Sony)
|-------------------------------------------|
|    74F74   74ALS08                        |
|                                           |
| |------|                                  |
| |KM4132|   |-----------|  |-----------|   |
| |G271Q |   | SONY      |  | SONY      |   |
| |------|   | CXD8654Q  |  | CXD8661R  |   |
|            |           |  |           |   |
| |------|   |           |  |           |   |
| |KM4132|   |           |  |           |   |
| |G271Q |   |-----------|  |-----------|   |
| |------|     53.693MHz       100MHz       |
|                                           |
|                                           |
|                                           |
|                                           |
|                 KM416V1204   KM416V1204   |
|  MC44200FT                                |
|                     *            *        |
|                                           |
|-------------------------------------------|
Notes:
      KM4132G271Q: Samsung Electronics 128k x 32-bit x 2 Banks SGRAM (x2, QFP100)
      KM416V1204 : Samsung Electronics 1M x16 EDO DRAM (x2, TSOP44(40))
      *          : Unpopulated positions for KM416V1204 EDO DRAM
      CXD8654Q   : Sony CXD8654Q GPU (QFP208)
      CXD8661R   : Sony CXD8661R R3000A-based CPU (QFP208)
      MC44200FT  : Motorola MC44200FT Triple 8-bit Video DAC (QFP44)

Type 2
------
      SYSTEM12 COH716 PCB 8661962301 (8661972301)
      (manufactured by Namco, probably in 2001, under license from Sony)
      Component layout is identical to COH-700 PCB with some updated components. Generally
      it has 4X the amount of program RAM that the COH-700 PCB has.
      KM4132G271Q replaced with OKI 54V25632A 128k x 32-bit x 2 Banks SGRAM (x2, QFP100)
      KM416V1204 replaced with Samsung Electronics K4E6416120D 4M x16 EDO DRAM (x2, TSOP44)
      Updated Sony CXD8561CQ GPU (QFP208)
      Updated Sony CXD8606BQ R3000A-based CPU (QFP208)
      Updated Motorola MC141685FT Video DAC (QFP44)

      The Type 2 PCB is used only by Super World Stadium 2001 (so far)
      Note, this CPU PCB was tested with some of the older S12 games and
      they worked fine, so it seems to be 100% compatible with all S12 games.
      The older COH-700 CPU board will also work with SWS2001.


NETWORK PCB
-----------
SYSTEM12 HSC PCB 8661960601 (8661970601)
|---------------------------------------------------|
|           USB-A   USB-B                           |
|                                   DS8921          |
|                                                   |
|    J2 74LVT245                                    |
|    |-|            N341256         N341256         |
|    | |74LVT245    N341256         N341256         |
|    | |                                            |
|    | |74LVT244                       |------|     |
|    | |        |--------|             | C422 |     |
|    | |74F244  |        |             |      |     |
|    | |        |  C446  |             |------|     |
|    | |74F244  |        |                          |
|    | |        |        |                          |
|    | |74F244  |        |                          |
|    | |        |--------|          74F74  |-----|  |
|    | |74F244               2061ASC-1     |H8/  |  |
|    |-|                                   |3002 |  |
|         S12HSC3A  S12HSC1A  14.7456MHz   |-----|  |
|                                                   |
|                             S12HSC2A    74HC32    |
|                                                   |
|                                                   |
|---------------------------------------------------|
Notes:
      DS8921   : National DS8921ATM Differential Line Driver and Receiver Pair (SOIC8)
      USB-A    : Standard USB type A connector, PCB labelled '(TX)'
      USB-B    : Standard USB type B connector, PCB labelled '(RX)'
      C446     : Namco Custom C446 (QFP160)
      C422     : Namco Custom C422 (QFP64)
      2061ASC-1: IC Designs 2061ASC-1 programmable clock generator (SOIC16)
      N341256  : NKK N341256 32k x8 SRAM (x4, SOJ28)
      H8/3002  : Hitachi H8/3002 HD6413002F17 Microcontroller (QFP100), clocked at 14.7456MHz.
      S12HSC1A : PALCE 16V8H  (PLCC20, labelled 'S12HSC1A')
      S12HSC2A : PALCE 22V10H (PLCC28, labelled 'S12HSC2A')
      S12HSC3A : PALCE 22V10H (PLCC28, labelled 'S12HSC3A')
      J2       : Custom Namco connector for plug-in CPU PCB

      This PCB was found on the following games (so far)....

      Libero Grande (LG2/VER.A)
      Tekken 3      (TET2/VER.A)
      Ehrgeiz       (EG2/VER.A)
      Kart Duel     (KTD1/VER.A)

      Note that the games will also work without this PCB, minus the network functionality.


Gun Board (This is the same Gun Board used with Point Blank 2 on System 11 hardware)
---------

SYSTEM11 GUN I/F A4111 PCB 8645960701 (8645970701)
|-------------------------------------------|
|    |-------|    |-------|    |-------|    |
|    |S11GUN3|    |S11GUN2|    |S11GUN1|    |
|    |       |    |       |    |       |    |
|    |       |    |       |    |       |    |
|    |-------|    |-------|    |-------|    |
|                                           |
|                                           |
|                                           |
|      SLA4060                              |
|J3                     |---------|         |
|                       | S11GUN0 |         |
|                       |         |         |
|                       |         |         |
|                       |         |  AV9170 |
|                       |---------|         |
|                                           |
|                   J2                      |
|                                           |
|                   J1                      |
|-------------------------------------------|
Notes:
      SLA4060 - Sanken Electric Co. NPN Darlington Transistor Array (SIP12)
      J1      - 96 pin connector joining to the mother board (connector below the PCB)
      J2      - 96 pin connector joining to the CPU board (connector above the PCB)
      J3      - 10 pin connector joining to the gun via a 24V solenoid driver board (for the gun opto/recoil)
      S11GUN0 - PLCC84 FPGA (not populated)
      S11GUN1 - Altera Max EPM7128STC100-10 EPLD (QFP100, not populated)
      S11GUN2 - Altera Max EPM7128STC100-10 EPLD (QFP100, labelled 'S11GUN2)
      S11GUN3 - Altera Max EPM7128STC100-10 EPLD (QFP100, not populated)
      AV9170  - Integrated Circuit Systems Inc. AV9170 Clock Synchronizer and Multiplier, Voltage Controlled Oscillator (SOIC8)

      This PCB was found on the following games (so far)....
      Ghoul Panic (OB2/VER.A)
      Oh! Bakyuuun (OB1/VER.A)
      Gunbarl (GNB4/VER.A)
      Point Blank 2 (GNB5/VER.A)

To connect a normal (i.e. HAPP) light gun only 4 wires are needed.
Those are +5V, GND, trigger and gun optical sensor using pins 5 & 9 of J3 and pins 1, 3 & 22 (both solder and parts side)
of the JAMMA connector. A Namco gun can also be connected the same way, with or without the kickback solenoid.

J3 connector:
Pin 1 - Not Used
Pin 2 - Not Used
Pin 3 - Gun 1 Solenoid
Pin 4 - Player 1 Start Button Lamp
Pin 5 - Gun 1 Sensor
Pin 6 - GND
Pin 7 - Gun 2 Solenoid
Pin 8 - Player 2 Start Button Lamp
Pin 9 - Gun 2 Sensor
Pin 10- Not Used

JAMMA Harness:
Pin 1 Parts Side - GND
Pin 3 Parts Side - +5V
Pin 22 Parts Side - Gun 1 Trigger
Pin 22 Solder Side - Gun 2 Trigger


CDXA PCB
--------
SYSTEM12 CDXA PCB 8661962101 (8661972101)
|---------------------------------------------------|
|                               J8     J9           |
|           6734               4556   3121          |
|                                                   |
|    J2                                    MB87078  |
|    |-|        IS61C256                            |
|    | |        IS61C256                   LC78836M |
|    | |                                            |
|    | |                                            |
|    | |        |--------|                          |
|    | |        |        |    |------|   TC558128   |
|    | |        |  C448  |    | SH2  |              |
|    | |        |        |    |      |              |
|    | |        |        |    |------|   TC558128   |
|    | |        |--------|                          |
|    | |                                            |
|    | |                                            |
|    |-|                                            |
|        2061ASC-1        |------|                  |
|                         |ALTERA|                  |
|        14.7456MHz       |MAX   |                  |
|                         |EPM7128                  |
|                         |------|                  |
|               J3              J4                  |
|---------------------------------------------------|
Notes:
      6734     : Texas Instruments TPS6734 Fixed 12V 120mA Boost-Converter Supply compatible with MAX734 (SOIC8)
      4556     : NJM4556 Dual High Current Operational Amplifier (SOIC8)
      3121     : Rohm BA3121 Ground Isolation Amplifier (SOIC8)
      C448     : Namco Custom C448 (QFP160)
      SH2      : Hitachi SH2 HD6417014F28 CPU (QFP112)
      2061ASC-1: IC Designs 2061ASC-1 programmable clock generator (SOIC16)
      TC558128 : Toshiba TC558128BFT-15 128k x8 SRAM (x2, SOJ32)
      IS61C256 : ISSI IS61C256AH-12T 32k x8 SRAM (x2, TSOP28)
      EPM7128  : Altera Max EPM7128 CPLD with sticker 'S12C DX0A' (QFP100)
      LC78836  : Sanyo LC78836 2-Channel 16-Bit D/A Converter with On-Chip 8X Oversampling Digital Filters (SOIC24)
      MB87078  : Fujitsu MB87078 6-bit, 4-channel Electronic Volume Controller (SOIC24)
      J2       : Custom Namco connector for plug-in CPU PCB
      J3       : 40 pin connector for IDE CDROM data cable
      J4       : 6 pin connector (serial)
                 For Um Jammer Lammy Now this goes to J205 on the M148 EMI DRIVE PCB.
                 According to the manual schematics, only GND and TX are connected going to RxD0 on EMI DRIVE PCB side.
      J8       : 4 pin connector (left/right audio output, RGND/R/LGND/L pinout)
      J9       : 3 pin connector (left/right audio output, L/GND/R pinout)

      This PCB was found on the following games (so far)....

      -Truck Kyosokyoku (TKK2/VER.A)
            A CDROM drive and CDROM disc is also required
            The disc is labelled 'TKK2-A'
      -Tekno Werk
            A CDROM drive and CDROM disc is also required

I/O Board (for use with Truck Kyosokyoku)
---------

V233 DECO PCB
2532960102 (2532970102)
|-------------------------------------------|
|    DSW(2)  JP1       14.7456MHz           |
|LED                                        |
|             C78              MB3771       |
|J8                    TKK1PRG0.IC7         |
|                                           |
|    SLA4031 SLA4031 SLA4031 SLA4031 SLA4031|
|                                           |
|J9          J1          J2             J3  |
|-------------------------------------------|
Notes:
      TKK1PRG0.IC7 - 27C1001 EPROM (DIP28)
      SLA4031      - Sanken SLA4031 Quad High-Voltage, High-Current NPN Darlington transistor
                     array for sink drive with built-in flywheel diode (SIL12)
      C78          - Hitachi H8/3334 Micro-controller (PLCC84)
      JP1          - Jumper to configure the H8/3334 for internal or external ROM usage. Set to 2-3 (EXT)
      J1/2/3/8     - Multi-pin connectors joining to controls and main PCB
      J9           - Power input connector

I/O board (for Attack Pla-Rail)
-------------------------------
Attack Pla-Rail requires an I/O board to boot. Several I/O boards are accepted including TSS-I/O, FCA, ASCA3, ASCA5
and also the common JVS I/O boards manufactured by Sega.
The game uses 3 buttons and a 5k potentiometer for the lever. The button signals come from the I/O board.
The lever must be wired to analog port 0 (pin B22 parts side) of the Namco 48-way edge connector.

*/

#include "emu.h"

#include "bus/ata/ataintf.h"
#include "cpu/h8/h83002.h"
#include "cpu/h8/h83337.h"
#include "cpu/sh/sh7014.h"
#include "cpu/psx/psx.h"
#include "machine/at28c16.h"
#include "machine/ram.h"
#include "machine/rtc4543.h"
#include "sound/c352.h"
#include "video/psx.h"

#include "namco_settings.h"
#include "namcos12_cdxa.h"

#include "screen.h"
#include "speaker.h"

#include <cstdarg>


namespace {

#define VERBOSE_LEVEL ( 0 )

class namcos12_state : public driver_device
{
public:
	namcos12_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, "maincpu:ram")
		, m_sub(*this, "sub")
		, m_adc(*this, "sub:adc")
		, m_rtc(*this, "rtc")
		, m_settings(*this, "namco_settings")
		, m_sharedram(*this, "sharedram")
		, m_mainrom(*this, "maincpu:rom")
		, m_bankedroms(*this, "bankedroms")
		, m_mainbank(*this, "mainbank")
		, m_lightgun_io(*this, {"LIGHT0_X", "LIGHT0_Y", "LIGHT1_X", "LIGHT1_Y"})
		, m_service_io(*this, "SERVICE")
		, m_start_lamp(*this, "P%u_Start_lamp", 1U)
		, m_gun_recoil(*this, "Player%u_Gun_Recoil", 1U)
	{
	}

	void coh700(machine_config &config);
	void coh716(machine_config &config);
	void namcos12_mobo(machine_config &config);

	void init_namcos12();
	void init_ptblank2();
	void init_technodr();
	void init_alt_bank1();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void jvsmap(address_map &map);
	void namcos12_map(address_map &map);
	void plarailjvsmap(address_map &map);
	void ptblank2_map(address_map &map);
	void s12h8rwjvsmap(address_map &map);
	void s12h8rwmap(address_map &map);
	void tdjvsmap(address_map &map);
	void tektagt_map(address_map &map);

	uint16_t s12_mcu_gun_h_r();
	uint16_t s12_mcu_gun_v_r();
	uint8_t s12_mcu_p8_r();
	uint8_t s12_mcu_jvs_p8_r();
	uint8_t s12_mcu_pa_r();
	void s12_mcu_pa_w(uint8_t data);
	uint8_t s12_mcu_portB_r();
	void s12_mcu_portB_w(uint8_t data);
	uint8_t s12_mcu_p6_r();
	uint8_t iob_p4_r();
	uint8_t iob_p6_r();
	void iob_p4_w(uint8_t data);

	required_device<psxcpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<h83002_device> m_sub;

private:
	required_device<h8_adc_device> m_adc;
	required_device<rtc4543_device> m_rtc;
	required_device<namco_settings_device> m_settings;
	required_shared_ptr<uint16_t> m_sharedram;

	required_memory_region m_mainrom;
	optional_memory_region m_bankedroms;
	optional_memory_bank m_mainbank;

	optional_ioport_array<4> m_lightgun_io;
	optional_ioport m_service_io;

	output_finder<2> m_start_lamp;
	output_finder<2> m_gun_recoil;

	uint16_t m_n_bankoffset;
	uint32_t m_n_dmaoffset;
	uint32_t m_n_tektagdmaoffset;
	int m_has_tektagt_dma;
	int m_alt_bank;

	int m_ttt_cnt;
	uint32_t m_ttt_val[2];
	uint8_t m_sub_porta;
	uint8_t m_sub_portb;
	uint8_t m_jvssense;

	void sharedram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t sharedram_r(offs_t offset, uint16_t mem_mask = ~0);
	void bankoffset_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void dmaoffset_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void system11gun_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t system11gun_r(offs_t offset, uint16_t mem_mask = ~0);
	void tektagt_protection_1_w(offs_t offset, uint16_t data);
	uint16_t tektagt_protection_1_r(offs_t offset);
	void tektagt_protection_2_w(offs_t offset, uint16_t data);
	uint16_t tektagt_protection_2_r(offs_t offset);
	uint16_t tektagt_protection_3_r();

	inline void ATTR_PRINTF(3,4) verboselog( int n_level, const char *s_fmt, ... );
	void namcos12_rom_read( uint32_t *p_n_psxram, uint32_t n_address, int32_t n_size );
	void namcos12_sub_irq(int state);
};


class namcos12_boothack_state : public namcos12_state
{
public:
	using namcos12_state::namcos12_state;

	void technodr(machine_config &config);
	void golgo13(machine_config &config);
	void aplarail(machine_config &config);
	void tektagt(machine_config &config);
	void ptblank2(machine_config &config);
	void kartduel(machine_config &config);

private:
	virtual void machine_reset() override;
};


class namcos12_cdxa_state : public namcos12_boothack_state
{
public:
	namcos12_cdxa_state(const machine_config &mconfig, device_type type, const char *tag)
		: namcos12_boothack_state(mconfig, type, tag)
		, m_cdxa_pcb(*this, "cdxa_pcb")
	{
	}

	void cdxa_pcb(machine_config &config);

	void truckk(machine_config &config);

	void init_truckk();

private:
	void cdxa_psx_map(address_map &map);

	required_device<namcos12_cdxa_device> m_cdxa_pcb;
};


inline void ATTR_PRINTF(3,4) namcos12_state::verboselog( int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: %s", machine().describe_context(), buf );
	}
}

void namcos12_state::sharedram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	verboselog(1, "sharedram_w( %08x, %08x, %08x )\n", ( offset * 4 ), data, mem_mask );
	COMBINE_DATA( &m_sharedram[ offset ] );
}

uint16_t namcos12_state::sharedram_r(offs_t offset, uint16_t mem_mask)
{
	verboselog(1, "sharedram_r( %08x, %08x ) %08x\n", ( offset * 4 ), mem_mask, m_sharedram[ offset ] );
	return m_sharedram[ offset ];
}

void namcos12_state::bankoffset_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/*
	Banking notes for various alt bank games:
	mdhorse, kaiunqz:
	    *0x1f000000 = ((val >> 0x17) & 0xe) | 8;

	golgo13, g13knd:
	    *0x1f000000 = ((val >> 0x17) & 0x6) + 8;
	    followed by
	    *0x1f000000 = (val >> 0x15) & 0x7;

	truckk:
	    *0x1f000000 = ((val >> 0x17) & 0x1e) + 8;
	    followed by
	    *0x1f000000 = (val >> 0x15) & 0x7ff; (lower bits only set for manual transfers, mask is implicit)

	sws2001:
	    The upper bits are set using fixed range checks:
	    if (val <= 0x0ffffff)
	        *0x1f000000 = 0x08;
	    else if (val >= 0x1000000 && val <= 0x1ffffff)
	        *0x1f000000 = 0x0a;
	    else if (val >= 0x2000000 && val <= 0x2ffffff)
	        *0x1f000000 = 0x0c;
	    else if (val >= 0x3000000)
	        *0x1f000000 = 0x0e;

	    The function at 8004d01c is the same logic as above + logic for the lower 3 bits:
	    *0x1f000000 = (val >> 0x15) & 0x7;
	*/
	if( m_alt_bank == 1 )
	{
		if( ( data & 8 ) != 0 )
		{
			m_n_bankoffset = ( data - 8 ) << 2;
		}
		else
		{
			m_n_bankoffset = ( m_n_bankoffset & ~0x7 ) | ( data & 0x7 );
		}
	}
	else
	{
		m_n_bankoffset = data;
	}

	m_mainbank->set_entry( m_n_bankoffset );

	verboselog(1, "bankoffset_w( %08x, %08x, %08x ) %08x\n", offset, data, mem_mask, m_n_bankoffset );
}

void namcos12_state::dmaoffset_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_n_dmaoffset = ( offset * 2 ) | ( data << 16 );

	verboselog(1, "dmaoffset_w( %08x, %08x, %08x ) %08x\n", offset, data, mem_mask, m_n_dmaoffset );
}

void namcos12_state::namcos12_rom_read( uint32_t *p_n_psxram, uint32_t n_address, int32_t n_size )
{
	memory_region *n_region;
	int n_offset;

	int32_t n_romleft;

	uint16_t *source;
	uint16_t *destination;
	int32_t n_ramleft;

	// TODO: the check for going past the end of ram should be in dma.c
	uint32_t m_n_psxramsize = m_ram->size();

	n_offset = m_has_tektagt_dma ? m_n_tektagdmaoffset : m_n_dmaoffset;

	// Depending on the game, the method of specifying the main ROM region is different.
	// Some games set exp_base to 0x1f300000 (toukon3, tenkomor) and others set the msb of the DMA offset to 1 (tekken3, sws2000 for example).
	const bool is_mainrom = BIT( n_offset, 31 ) || m_maincpu->exp_base() == 0x1f300000;

	if ( is_mainrom )
	{
		n_region = m_mainrom;
		n_offset &= 0xffffff; // although the area should really be masked with 0x3fffff, the bootloader used by sws2000, sws2001, kaiunqz intentionally go over by a small amount when copying the program code into memory
	}
	else
	{
		n_region = m_bankedroms;
		n_offset &= 0x7fffffff;

		if( !m_has_tektagt_dma && m_alt_bank ) // alt bank method stores the upper bits of the bank into m_n_bankoffset
			n_offset += m_n_bankoffset * 0x200000;
	}

	m_has_tektagt_dma = 0;

	verboselog(1, "namcos12_rom_read( %08x, %08x ) %s %08x %08x\n", n_address, n_size, is_mainrom ? "mainrom" : "game", m_n_dmaoffset, n_offset );

	source = (uint16_t *) n_region->base();
	n_romleft = int32_t( n_region->bytes() - n_offset ) / 4; // without the cast this math never becomes negative so the truncation logic never happens (needed by sws2000, sws2001, kaiunqz)

	if( n_size > n_romleft )
	{
		verboselog(1, "namcos12_rom_read dma truncated %d to %d passed end of rom\n", n_size, n_romleft );
		n_size = n_romleft;
	}

	destination = (uint16_t *)p_n_psxram;

	n_ramleft = int32_t( m_n_psxramsize - n_address ) / 4;
	if( n_size > n_ramleft )
	{
		verboselog(1, "namcos12_rom_read dma truncated %d to %d passed end of ram\n", n_size, n_ramleft );
		n_size = n_ramleft;
	}

	n_size *= 2;
	n_address /= 2;
	n_offset /= 2;

	while( n_size > 0 )
	{
		destination[ WORD_XOR_LE( n_address ) ] = source[ WORD_XOR_LE( n_offset ) ];
		n_address++;
		n_offset++;
		n_size--;
	}
}

void namcos12_state::namcos12_sub_irq(int state)
{
	m_sub->set_input_line(1, state ? ASSERT_LINE : CLEAR_LINE);
	m_adc->adtrg_w(state);
	m_sub_portb = (m_sub_portb & 0x7f) | (state << 7);
}

void namcos12_state::namcos12_map(address_map &map)
{
	map(0x1f000000, 0x1f000003).nopr();
	map(0x1f000000, 0x1f000001).w(FUNC(namcos12_state::bankoffset_w));          /* banking */
	map(0x1f010000, 0x1f010003).nopw(); // very spammy
	map(0x1f080000, 0x1f083fff).rw(FUNC(namcos12_state::sharedram_r), FUNC(namcos12_state::sharedram_w)); /* shared ram?? */
	map(0x1f140000, 0x1f140fff).rw("at28c16", FUNC(at28c16_device::read), FUNC(at28c16_device::write)).umask32(0x00ff00ff); /* EEPROM */
	map(0x1f018000, 0x1f018003).nopw(); // very spammy
	map(0x1f1bff08, 0x1f1bff0f).nopw();    /* ?? */
	map(0x1f700000, 0x1f70ffff).w(FUNC(namcos12_state::dmaoffset_w));  /* dma */
	/* Network area */
//  map(0x1f780000, 0x1f78ffff).rw(FUNC(namcos12_state::link_sharedram_r), FUNC(namcos12_state::link_sharedram_w)); /* H8 link CPU code */
//  map(0x1f796002, 0x1f796003).w(FUNC(namcos12_state::linkcpu_enable_w));
//  map(0x1f796022, 0x1f796023).w(FUNC(namcos12_state::linkcpu_disable_w));

	map(0x1fa00000, 0x1fbfffff).bankr("mainbank"); /* banked ROMs */
}

void namcos12_state::ptblank2_map(address_map &map)
{
	namcos12_map(map);

	map(0x1f780000, 0x1f78000f).r(FUNC(namcos12_state::system11gun_r));
	map(0x1f788000, 0x1f788003).w(FUNC(namcos12_state::system11gun_w));
}

void namcos12_state::tektagt_map(address_map &map)
{
	namcos12_map(map);

	map(0x1fb00000, 0x1fb00003).rw(FUNC(namcos12_state::tektagt_protection_1_r), FUNC(namcos12_state::tektagt_protection_1_w));
	map(0x1fb80000, 0x1fb80003).rw(FUNC(namcos12_state::tektagt_protection_2_r), FUNC(namcos12_state::tektagt_protection_2_w));
	map(0x1f700000, 0x1f700003).r(FUNC(namcos12_state::tektagt_protection_3_r));
}

void namcos12_state::system11gun_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (offset)
	{
	case 0:
		/* blowback 1 */
		/* blowback 2 */
		/* Note: output label has been changed for the Engrish Impaired ;-) */
		m_gun_recoil[0] =  BIT(~data, 1);
		m_gun_recoil[1] =  BIT(~data, 0);

		/* start 1 */
		m_start_lamp[0] =  BIT(~data, 3);
		/* start 2 */
		m_start_lamp[1] =  BIT(~data, 2);

		verboselog(1, "system11gun_w: outputs (%08x %08x)\n", data, mem_mask );
		break;

	case 1:
		verboselog(2, "system11gun_w: start reading (%08x %08x)\n", data, mem_mask );
		break;
	}
}

uint16_t namcos12_state::system11gun_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0;

	switch (offset)
	{
	case 0:
		data = m_lightgun_io[0]->read();
		break;

	case 2:
		data = m_lightgun_io[1]->read();
		break;

	case 3:
		data = m_lightgun_io[1]->read() + 1;
		break;

	case 4:
		data = m_lightgun_io[2]->read();
		break;

	case 6:
		data = m_lightgun_io[3]->read();
		break;

	case 7:
		data = m_lightgun_io[3]->read() + 1;
		break;
	}
	verboselog(2, "system11gun_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
	return data;
}

void namcos12_state::tektagt_protection_1_w(offs_t offset, uint16_t data)
{
	// Second dma offset or protection ref values write

	switch (offset)
	{
	case 0:
		m_n_tektagdmaoffset = data;

		if(m_ttt_cnt != 2)
			m_ttt_val[m_ttt_cnt] = data;
		break;
	case 1:
		m_n_tektagdmaoffset |= data << 16;

		if(m_ttt_cnt != 2)
			m_ttt_val[m_ttt_cnt++] |= data << 16;
		break;
	}
}

uint16_t namcos12_state::tektagt_protection_1_r(offs_t offset)
{
	switch (offset)
	{
	case 0:
		// Reads are either ignored or bit 15 is tested for a busy flag
		return 0x8000;
	case 1:
		break;
	}

	return 0;
}

void namcos12_state::tektagt_protection_2_w(offs_t offset, uint16_t data)
{
	switch (offset)
	{
	case 0:
		// Writes are 0 or machine().rand(), only used as a "start prot value write" trigger
		m_ttt_cnt = 0;
		break;
	case 1:
		break;
	}
}

uint16_t namcos12_state::tektagt_protection_2_r(offs_t offset)
{
	if( m_ttt_cnt == 2 )
	{
		if( m_ttt_val[0] == 0x806d2c24 && m_ttt_val[1] == 0xd5545715 )
		{
			if( offset == 0 )
				return 0x36e2;
		}
		else if( m_ttt_val[0] == 0x804c2c84 && m_ttt_val[1] == 0xd5545615 )
		{
			if( offset == 0 )
				return 0x2651;
		}
		else if( m_ttt_val[0] == 0x2aaba8e6 && m_ttt_val[1] == 0x00820040 )
		{
			if( offset == 1 )
				return 0x4186;
		}
		else if( m_ttt_val[0] == 0x2aaba592 && m_ttt_val[1] == 0x01780544 )
		{
			if( offset == 1 )
				return 0x3c7d;
		}
		else
		{
			if(((m_ttt_val[1] >> 16) & 0xff) == 0xa9)
				return 0x552e;
		}
	}

	return 0;
}

uint16_t namcos12_state::tektagt_protection_3_r()
{
	m_has_tektagt_dma = 1;
	// Always ignored
	return 0;
}

void namcos12_state::machine_start()
{
	m_start_lamp.resolve();
	m_gun_recoil.resolve();
}

void namcos12_state::machine_reset()
{
	bankoffset_w(0,0,0xffff);

	m_jvssense = 1;
	m_has_tektagt_dma = 0;
}

void namcos12_boothack_state::machine_reset()
{
	namcos12_state::machine_reset();

	/* HACK: this is based on guesswork, it might not even be keycus. */
	uint8_t *rom = memregion( "maincpu:rom" )->base() + 0x20280;
	uint8_t *ram = m_ram->pointer() + 0x10000;

	memcpy( ram, rom, 12 );
}

/* H8/3002 MCU stuff */
void namcos12_state::s12h8rwmap(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x080000, 0x08ffff).ram().share("sharedram");
	map(0x280000, 0x287fff).rw("c352", FUNC(c352_device::read), FUNC(c352_device::write));
	map(0x300000, 0x300001).portr("IN0");
	map(0x300002, 0x300003).portr("IN1");
	map(0x300010, 0x300011).noprw(); // golgo13 writes here a lot, possibly also a wait state generator?
	map(0x300030, 0x300031).noprw(); // most S12 bioses write here simply to generate a wait state.  there is no deeper meaning.
}

// map for JVS games w/o controls connected directly
void namcos12_state::s12h8rwjvsmap(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x080000, 0x08ffff).ram().share("sharedram");
	map(0x280000, 0x287fff).rw("c352", FUNC(c352_device::read), FUNC(c352_device::write));
	map(0x300000, 0x300001).portr("DIN0");
	map(0x300002, 0x300003).portr("DIN1");
	map(0x300010, 0x300011).noprw();
	map(0x300030, 0x300031).noprw(); // most S12 bioses write here simply to generate a wait state.  there is no deeper meaning.
}

uint8_t namcos12_state::s12_mcu_p8_r()
{
	return 0x02;
}

// emulation of the Epson R4543 real time clock
// in System 12, bit 0 of H8/3002 port A is connected to its chip enable
// the actual I/O takes place through the H8/3002's serial port B.

uint8_t namcos12_state::s12_mcu_pa_r()
{
	return m_sub_porta;
}

void namcos12_state::s12_mcu_pa_w(uint8_t data)
{
	m_sub_porta = data;
	m_rtc->ce_w((m_sub_portb & 0x20) && (m_sub_porta & 1));
	m_settings->ce_w((m_sub_portb & 0x20) && !(m_sub_porta & 1));
}

uint8_t namcos12_state::s12_mcu_portB_r()
{
	return m_sub_portb;
}

void namcos12_state::s12_mcu_portB_w(uint8_t data)
{
	m_sub_portb = (m_sub_portb & 0x80) | (data & 0x7f);
	m_rtc->ce_w((m_sub_portb & 0x20) && (m_sub_porta & 1));
	m_settings->ce_w((m_sub_portb & 0x20) && !(m_sub_porta & 1));
}

uint8_t namcos12_state::s12_mcu_p6_r()
{
	// bit 1 = JVS cable present sense (1 = I/O board plugged in)
	return (m_jvssense << 1) | 0xfd;
}

uint8_t namcos12_state::s12_mcu_jvs_p8_r()
{
	return 0x12;    // bit 4 = JVS enable.  aplarail requires it to be on, soulclbr & others will require JVS I/O if it's on
}

// Golgo 13 lightgun inputs

uint16_t namcos12_state::s12_mcu_gun_h_r()
{
	return m_lightgun_io[0]->read();
}

uint16_t namcos12_state::s12_mcu_gun_v_r()
{
	return m_lightgun_io[1]->read();
}

void namcos12_state::init_namcos12()
{
	m_mainbank->configure_entries(0, memregion( "bankedroms" )->bytes() / 0x200000, memregion( "bankedroms" )->base(), 0x200000 );

	m_sub_porta = 0;
	m_sub_portb = 0x50;

	m_n_tektagdmaoffset = 0;
	m_n_dmaoffset = 0;
	m_n_bankoffset = 0;
	m_mainbank->set_entry( 0 );
	m_alt_bank = 0;

	save_item( NAME(m_n_dmaoffset) );
	save_item( NAME(m_n_bankoffset) );
}

void namcos12_state::init_ptblank2()
{
	init_namcos12();

	/* HACK: patch out wait for dma 5 to complete */
	*( (uint32_t *)( memregion( "maincpu:rom" )->base() + 0x331c4 ) ) = 0;
}

void namcos12_state::init_technodr()
{
	init_namcos12();

	// HACK: patch H8 to fix COIN ERROR
	*( (uint32_t *)( memregion( "sub" )->base() + 0x14b6 ) ) = 0;
}

void namcos12_state::init_alt_bank1()
{
	init_namcos12();
	m_alt_bank = 1;
}

// SYSTEM12 MOTHER PCB
void namcos12_state::namcos12_mobo(machine_config &config)
{
	/* basic machine hardware */
	H83002(config, m_sub, 16934400); // frequency based on research (superctr)
	m_sub->set_addrmap(AS_PROGRAM, &namcos12_state::s12h8rwmap);
	m_sub->read_adc<0>().set_constant(0);
	m_sub->read_adc<1>().set_constant(0);
	m_sub->read_adc<2>().set_constant(0);
	m_sub->read_adc<3>().set_constant(0);
	m_sub->read_port6().set(FUNC(namcos12_state::s12_mcu_p6_r));
	m_sub->read_port7().set_ioport("DSW");
	m_sub->read_port8().set(FUNC(namcos12_state::s12_mcu_p8_r));
	m_sub->write_port8().set_nop();
	m_sub->read_porta().set(FUNC(namcos12_state::s12_mcu_pa_r));
	m_sub->write_porta().set(FUNC(namcos12_state::s12_mcu_pa_w));
	m_sub->read_portb().set(FUNC(namcos12_state::s12_mcu_portB_r));
	m_sub->write_portb().set(FUNC(namcos12_state::s12_mcu_portB_w));

	NAMCO_SETTINGS(config, m_settings, 0);

	RTC4543(config, m_rtc, XTAL(32'768));
	m_rtc->data_cb().set(m_sub, FUNC(h8_device::sci_rx_w<1>));

	m_sub->write_sci_tx<1>().set(m_settings, FUNC(namco_settings_device::data_w));
	m_sub->write_sci_clk<1>().set(m_rtc, FUNC(rtc4543_device::clk_w)).invert();
	m_sub->write_sci_clk<1>().append(m_settings, FUNC(namco_settings_device::clk_w));

	AT28C16(config, "at28c16", 0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	c352_device &c352(C352(config, "c352", 25401600, 288));
	c352.add_route(0, "lspeaker", 1.00);
	c352.add_route(1, "rspeaker", 1.00);
	//c352.add_route(2, "lspeaker", 1.00); // Second DAC not present.
	//c352.add_route(3, "rspeaker", 1.00);
}

// CPU PCB COH700
void namcos12_state::coh700(machine_config &config)
{
	namcos12_mobo(config);
	/* basic machine hardware */
	CXD8661R(config, m_maincpu, XTAL(100'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &namcos12_state::namcos12_map);
	m_maincpu->subdevice<ram_device>("ram")->set_default_size("4M"); // 2 KM416V1204s
	m_maincpu->subdevice<psxdma_device>("dma")->install_read_handler(5, psxdma_device::read_delegate(&namcos12_state::namcos12_rom_read, this));

	/* video hardware */
	CXD8654Q(config, "gpu", XTAL(53'693'175), 0x200000, subdevice<psxcpu_device>("maincpu")).set_screen("screen"); // 2 KM4132G271Qs

	SCREEN(config, "screen", SCREEN_TYPE_RASTER).screen_vblank().set(FUNC(namcos12_state::namcos12_sub_irq));
}

// CPU PCB COH716
void namcos12_state::coh716(machine_config &config)
{
	namcos12_mobo(config);
	/* basic machine hardware */
	CXD8606BQ(config, m_maincpu, XTAL(100'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &namcos12_state::namcos12_map);
	m_maincpu->subdevice<ram_device>("ram")->set_default_size("16M"); // 2 K4E6416120Ds
	m_maincpu->subdevice<psxdma_device>("dma")->install_read_handler(5, psxdma_device::read_delegate(&namcos12_state::namcos12_rom_read, this));

	/* video hardware */
	CXD8561CQ(config, "gpu", XTAL(53'693'175), 0x200000, subdevice<psxcpu_device>("maincpu")).set_screen("screen"); // 2 54V25632As

	SCREEN(config, "screen", SCREEN_TYPE_RASTER).screen_vblank().set(FUNC(namcos12_state::namcos12_sub_irq));
}

void namcos12_boothack_state::ptblank2(machine_config &config)
{
	coh700(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &namcos12_boothack_state::ptblank2_map);
}

void namcos12_boothack_state::tektagt(machine_config &config)
{
	coh700(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &namcos12_boothack_state::tektagt_map);
}

void namcos12_boothack_state::golgo13(machine_config &config)
{
	coh700(config);

	/* basic machine hardware */
	m_sub->read_adc<1>().set(FUNC(namcos12_boothack_state::s12_mcu_gun_h_r));
	m_sub->read_adc<2>().set(FUNC(namcos12_boothack_state::s12_mcu_gun_v_r));
}

void namcos12_boothack_state::kartduel(machine_config &config)
{
	coh700(config);

	/* basic machine hardware */
	m_sub->read_adc<0>().set_ioport("BRAKE");
	m_sub->read_adc<1>().set_ioport("GAS");
	m_sub->read_adc<2>().set_ioport("STEER");
}

#define JVSCLOCK    (XTAL(14'745'600))
void namcos12_state::jvsmap(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("iocpu", 0);
	map(0xc000, 0xfb7f).ram();
}

uint8_t namcos12_state::iob_p4_r()
{
	return 0;
}

void namcos12_state::iob_p4_w(uint8_t data)
{
	// bit 2 = SENSE line back to main (0 = asserted, 1 = dropped)
	m_jvssense = (data & 0x04) ? 0 : 1;
}

uint8_t namcos12_state::iob_p6_r()
{
	// d4 is service button
	uint8_t sb = (m_service_io->read() & 1) << 4;
	// other bits: unknown

	return sb | 0;
}

void namcos12_state::tdjvsmap(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("iocpu", 0);
	map(0x6000, 0x6001).portr("IN01");
	map(0x6002, 0x6003).portr("IN23");
	map(0xc000, 0xfb7f).ram();
}

void namcos12_state::plarailjvsmap(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("iocpu", 0);
	map(0x6000, 0x6001).portr("IN01");
	map(0x6002, 0x6003).portr("IN23");
	map(0xc000, 0xfb7f).ram();
}

void namcos12_boothack_state::technodr(machine_config &config)
{
	coh700(config);
	// Timer at 115200*16 for the jvs serial clock
	m_sub->sci_set_external_clock_period(0, attotime::from_hz(JVSCLOCK/8));

	// modify H8/3002 map to omit direct-connected controls
	m_sub->set_addrmap(AS_PROGRAM, &namcos12_boothack_state::s12h8rwjvsmap);
	m_sub->read_port8().set(FUNC(namcos12_boothack_state::s12_mcu_jvs_p8_r));

	h83334_device &iocpu(H83334(config, "iocpu", JVSCLOCK));
	iocpu.set_addrmap(AS_PROGRAM, &namcos12_boothack_state::tdjvsmap);
	iocpu.read_adc<0>().set_ioport("STEER");
	iocpu.read_adc<1>().set_ioport("BRAKE");
	iocpu.read_adc<2>().set_ioport("GAS");
	iocpu.read_port4().set(FUNC(namcos12_boothack_state::iob_p4_r));
	iocpu.write_port4().set(FUNC(namcos12_boothack_state::iob_p4_w));
	iocpu.read_port6().set(FUNC(namcos12_boothack_state::iob_p6_r));

	iocpu.write_sci_tx<0>().set(m_sub, FUNC(h8_device::sci_rx_w<0>));
	m_sub->write_sci_tx<0>().set(iocpu, FUNC(h8_device::sci_rx_w<0>));

	config.set_maximum_quantum(attotime::from_hz(2*115200));
}

void namcos12_boothack_state::aplarail(machine_config &config)
{
	coh700(config);
	// Timer at 115200*16 for the jvs serial clock
	m_sub->sci_set_external_clock_period(0, attotime::from_hz(JVSCLOCK/8));

	// modify H8/3002 map to omit direct-connected controls
	m_sub->set_addrmap(AS_PROGRAM, &namcos12_boothack_state::s12h8rwjvsmap);
	m_sub->read_adc<0>().set_ioport("LEVER");
	m_sub->read_adc<1>().set_constant(0);
	m_sub->read_adc<2>().set_constant(0);
	m_sub->read_adc<3>().set_constant(0);
	m_sub->read_port6().set(FUNC(namcos12_boothack_state::s12_mcu_p6_r));
	m_sub->read_port7().set_ioport("DSW");
	m_sub->read_port8().set(FUNC(namcos12_boothack_state::s12_mcu_jvs_p8_r));
	m_sub->write_port8().set_nop();
	m_sub->read_porta().set(FUNC(namcos12_boothack_state::s12_mcu_pa_r));
	m_sub->write_porta().set(FUNC(namcos12_boothack_state::s12_mcu_pa_w));
	m_sub->read_portb().set(FUNC(namcos12_boothack_state::s12_mcu_portB_r));
	m_sub->write_portb().set(FUNC(namcos12_boothack_state::s12_mcu_portB_w));

	h83334_device &iocpu(H83334(config, "iocpu", JVSCLOCK));
	iocpu.set_addrmap(AS_PROGRAM, &namcos12_boothack_state::plarailjvsmap);
	iocpu.read_adc<0>().set_constant(0);
	iocpu.read_adc<1>().set_constant(0);
	iocpu.read_adc<2>().set_constant(0);
	iocpu.read_adc<3>().set_constant(0);
	iocpu.read_port4().set(FUNC(namcos12_boothack_state::iob_p4_r));
	iocpu.write_port4().set(FUNC(namcos12_boothack_state::iob_p4_w));
	iocpu.read_port6().set_ioport("SERVICE");

	iocpu.write_sci_tx<0>().set(m_sub, FUNC(h8_device::sci_rx_w<0>));
	m_sub->write_sci_tx<0>().set(iocpu, FUNC(h8_device::sci_rx_w<0>));

	config.set_maximum_quantum(attotime::from_hz(2*115200));
}

///

void namcos12_cdxa_state::cdxa_pcb(machine_config &config)
{
	coh700(config);

	NAMCOS12_CDXA(config, m_cdxa_pcb, XTAL(14'745'600));
	m_cdxa_pcb->add_route(0, "lspeaker", 0.30); // roughly matched the volume of speaking lines between the CDXA audio vs non-CDXA audio
	m_cdxa_pcb->add_route(1, "rspeaker", 0.30);
	m_cdxa_pcb->psx_int10_callback().set("maincpu:irq", FUNC(psxirq_device::intin10));

	m_maincpu->set_addrmap(AS_PROGRAM, &namcos12_cdxa_state::cdxa_psx_map);
}

void namcos12_cdxa_state::cdxa_psx_map(address_map &map)
{
	namcos12_map(map);

	map(0x1f7c0000, 0x1f7cffff).rw(m_cdxa_pcb, FUNC(namcos12_cdxa_device::sh2_ram_r), FUNC(namcos12_cdxa_device::sh2_ram_w));

	map(0x1f7d6002, 0x1f7d6003).w(m_cdxa_pcb, FUNC(namcos12_cdxa_device::reset_sh2_w));
	map(0x1f7d6004, 0x1f7d600b).w(m_cdxa_pcb, FUNC(namcos12_cdxa_device::clockgen_w));
	map(0x1f7d6010, 0x1f7d6011).w(m_cdxa_pcb, FUNC(namcos12_cdxa_device::ide_sh2_enabled_w));
	map(0x1f7d6012, 0x1f7d6013).w(m_cdxa_pcb, FUNC(namcos12_cdxa_device::ide_ps1_enabled_w));
	// 1f7d6018 unknown, only set once to 1 between the "SH2 Reset" and "SH2 Pll Clock Set" steps during boot
	map(0x1f7d601a, 0x1f7d601b).w(m_cdxa_pcb, FUNC(namcos12_cdxa_device::sram_enabled_w));
	map(0x1f7d601e, 0x1f7d601f).w(m_cdxa_pcb, FUNC(namcos12_cdxa_device::ps1_int10_finished_w));

	map(0x1f7d800a, 0x1f7d800b).lr16(NAME([] () {
		// Might be for the M148 PCB instead of CDXA PCB
		// Code loops until this returns 0x20 before it starts writing to 0x1f7d8000
		// Writes "55 x y z" to 0x1f7d8000 where x, y, z are related to the current I/O state
		// Maybe used for lights?
		return 0x20;
	}));

	map(0x1f7e0000, 0x1f7e000f).rw(m_cdxa_pcb, FUNC(namcos12_cdxa_device::cdrom_cs0_r), FUNC(namcos12_cdxa_device::cdrom_cs0_w));
	// map(0x1f7e8000, 0x1f7e800f).rw(m_cdxa_pcb, FUNC(namcos12_cdxa_device::cdrom_cs1_r), FUNC(namcos12_cdxa_device::cdrom_cs1_w));
	// 1f7d7000 volume enabled/set? gets set to 6 between the "SH2 Volume Set" and "SH2 Trf Program" steps, after setting 4 volumes registers to 0x7e
	map(0x1f7f8000, 0x1f7f80ff).w(m_cdxa_pcb, FUNC(namcos12_cdxa_device::volume_w));
}

void namcos12_cdxa_state::truckk(machine_config &config)
{
	cdxa_pcb(config);

	m_sub->read_adc<0>().set_ioport("STEER");
	m_sub->read_adc<1>().set_ioport("BRAKE");
	m_sub->read_adc<2>().set_ioport("GAS");

	// Timer at 115200*16 for the jvs serial clock
	m_sub->sci_set_external_clock_period(0, attotime::from_hz(JVSCLOCK/8));

	h83334_device &iocpu(H83334(config, "iocpu", JVSCLOCK));
	iocpu.set_addrmap(AS_PROGRAM, &namcos12_cdxa_state::jvsmap);

	iocpu.write_sci_tx<0>().set(m_sub, FUNC(h8_device::sci_rx_w<0>));
	m_sub->write_sci_tx<0>().set(iocpu, FUNC(h8_device::sci_rx_w<0>));

	config.set_maximum_quantum(attotime::from_hz(2*115200));
}

void namcos12_cdxa_state::init_truckk()
{
	init_alt_bank1();

	/*
	HACK: Change order of code so that the status flags are set before DMA 5 is started
	Can be checked at 0x8001f6e4
	*/
	uint8_t temp[5 * 4];
	memcpy(temp, memregion( "maincpu:rom" )->base() + 0x376ec, sizeof(temp)); // copy the 5 instructions that initialize statuses before DMA 5 finishes
	memcpy(memregion( "maincpu:rom" )->base() + 0x376e4 + sizeof(temp), memregion( "maincpu:rom" )->base() + 0x376e4, 2 * 4); // move the 2 DMA start call instructions below where the initialization code will go
	memcpy(memregion( "maincpu:rom" )->base() + 0x376e4, temp, sizeof(temp)); // write initialization code in new spot
}

static INPUT_PORTS_START( namcos12 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR(Service_Mode) ) PORT_DIPLOCATION( "DIP SW2:1" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Freeze" ) PORT_DIPLOCATION( "DIP SW2:2" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0xff3f, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE_NO_TOGGLE( 0x4000, IP_ACTIVE_LOW )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SERVICE1 )
INPUT_PORTS_END

static INPUT_PORTS_START( namcos124w )
	PORT_INCLUDE( namcos12 )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( ptblank2 )
	PORT_INCLUDE( namcos12 )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x6f6f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x00ee, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("LIGHT0_X")
	PORT_BIT( 0xffff, 0x022f, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0xd8,0x387) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("LIGHT0_Y")
	PORT_BIT( 0xffff, 0x00a8, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0x2c,0x11b) PORT_SENSITIVITY(50) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("LIGHT1_X")
	PORT_BIT( 0xffff, 0x022f, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0xd8,0x387) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("LIGHT1_Y")
	PORT_BIT( 0xffff, 0x00a8, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0x2c,0x11b) PORT_SENSITIVITY(50) PORT_KEYDELTA(15) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( ghlpanic )
	PORT_INCLUDE( namcos12 )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x6f6f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x00ee, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("LIGHT0_X")
	PORT_BIT( 0xffff, 0x0210, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0xc0,0x35f) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("LIGHT0_Y")
	PORT_BIT( 0xffff, 0x0091, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0x1a,0x109) PORT_SENSITIVITY(50) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("LIGHT1_X")
	PORT_BIT( 0xffff, 0x0210, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0xc0,0x35f) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("LIGHT1_Y")
	PORT_BIT( 0xffff, 0x0091, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0x1a,0x109) PORT_SENSITIVITY(50) PORT_KEYDELTA(15) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( golgo13 )
	PORT_INCLUDE( namcos12 )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xff43, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x10ee, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x4000, IP_ACTIVE_LOW )

	PORT_START("LIGHT0_X")
	PORT_BIT( 0xffff, 0x019b, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0x9c,0x29b) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("LIGHT0_Y")
	PORT_BIT( 0xffff, 0x00fe, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, -1.0, 0.0, 0) PORT_MINMAX(0x1f,0x1de) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_PLAYER(1) PORT_REVERSE
INPUT_PORTS_END

static INPUT_PORTS_START( kartduel )
	PORT_INCLUDE( namcos12 )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) // enter switch
	PORT_BIT( 0xffe3, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN1")
	PORT_SERVICE( 0x4000, IP_ACTIVE_LOW )
	PORT_BIT( 0x1fff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("GAS")
	PORT_BIT( 0x3ff, 0x0200, IPT_PEDAL )  PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_NAME("Gas Pedal")

	PORT_START("BRAKE")
	PORT_BIT( 0x3ff, 0x0200, IPT_PEDAL2 ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_NAME("Brake Pedal")

	PORT_START("STEER")
	PORT_BIT( 0x3ff, 0x0200, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_NAME("Steering Wheel")
INPUT_PORTS_END

static INPUT_PORTS_START( technodr )
	PORT_START("DSW")
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR(Service_Mode) ) PORT_DIPLOCATION( "DIP SW2:1" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Freeze" ) PORT_DIPLOCATION( "DIP SW2:2" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0xff3f, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN01")
	PORT_SERVICE( 0x100, IP_ACTIVE_LOW ) // service switch
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON1 ) // select switch
	PORT_BIT( 0x7eff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN23")
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_COIN1 ) // coin switch
	PORT_BIT(0xf7ff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DIN0")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("DIN1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )  // service coin

	PORT_START("GAS")
	PORT_BIT( 0x3ff, 0x0200, IPT_PEDAL )  PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_NAME("Gas Pedal") PORT_REVERSE

	PORT_START("BRAKE")
	PORT_BIT( 0x3ff, 0x0200, IPT_PEDAL2 ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_NAME("Brake Pedal") PORT_REVERSE

	PORT_START("STEER")
	PORT_BIT( 0x3ff, 0x0200, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_NAME("Steering Wheel") PORT_REVERSE
INPUT_PORTS_END

static INPUT_PORTS_START( aplarail )
	PORT_START("DSW")
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR(Service_Mode) ) PORT_DIPLOCATION( "DIP SW2:1" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Freeze" ) PORT_DIPLOCATION( "DIP SW2:2" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0xff3f, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN01")
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON1 )  // left
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_START1 )   // OK / Start
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 )  // right
	PORT_BIT( 0xc7ff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN23")
	PORT_BIT( 0xf7ff, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN1 )        // coin

	PORT_START("DIN0")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("DIN1")
	PORT_SERVICE_NO_TOGGLE( 0x4000, IP_ACTIVE_LOW )
	PORT_BIT(0xbfff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("LEVER")
	PORT_BIT( 0x07ff, 0x03ff, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_NAME("Speed Lever") PORT_REVERSE

	PORT_START("SERVICE")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)
INPUT_PORTS_END

static INPUT_PORTS_START( truckk )
	PORT_START("DSW")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0xfc23, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Select Up")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("Select Down")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Horn")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("View Change")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Music Next")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Music Back")

	PORT_START("IN1")
	PORT_BIT( 0x1fff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE( 0x4000, IP_ACTIVE_LOW )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("GAS")
	PORT_BIT( 0x3ff, 0x0200, IPT_PEDAL )  PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_NAME("Gas Pedal") PORT_REVERSE

	PORT_START("BRAKE")
	PORT_BIT( 0x3ff, 0x0200, IPT_PEDAL2 ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_NAME("Brake Pedal") PORT_REVERSE

	PORT_START("STEER")
	PORT_BIT( 0x3ff, 0x0200, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_NAME("Steering Wheel")
INPUT_PORTS_END

static INPUT_PORTS_START( ujlnow )
	PORT_START("DSW")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("P1 Chin Switch (C)")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("P1 Joe Switch (J)")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Select Down")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("Select Up")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Circle / Enter") // The cabinet enter switch is linked to the P1 circle input as per the manual (bottom of pg 33)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Cross")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Triangle")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("P2 Chin Switch (C)")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("P2 Joe Switch (J)")
	PORT_BIT( 0x0c00, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Must be high to get rid of I/O errors
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Circle")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Cross")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Triangle")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x13ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Square")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 Square")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE( 0x4000, IP_ACTIVE_LOW )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SERVICE1 )
INPUT_PORTS_END

ROM_START( aquarush )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "aq1vera.2l",   0x0000000, 0x200000, CRC(91eb9258) SHA1(30e225eb551bfe1bed6b342dd6d597345d64b677) )
	ROM_LOAD16_BYTE( "aq1vera.2p",   0x0000001, 0x200000, CRC(a92f21aa) SHA1(bde33f1f66aaa55031c6b2972b042eef87047cce) )

	ROM_REGION32_LE( 0x1000000, "bankedroms", 0 ) /* main data */
	ROM_LOAD(        "aq1rom0.7",    0x000000, 0x800000, CRC(3e64dd33) SHA1(dce3bb84c3069bc202d04f24d7702158dd3194d4) )
	ROM_LOAD(        "aq1rom1.8",    0x800000, 0x800000, CRC(e4d415cf) SHA1(bbd244adaf704d7daf7341ff3b0a92162927a59b) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "aq1vera.11s", 0x000000, 0x080000, CRC(78277e02) SHA1(577ebb6d7ab5e304fb1dc1e7fd5649762e0d1786) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "aq1wave0.2",          0x000000, 0x800000, CRC(0cf7278d) SHA1(aee31e4d9b3522f42325071768803c542aa6de09) )
ROM_END

ROM_START( ehrgeiz )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "eg2vera.2e",   0x0000000, 0x200000, CRC(9174ec90) SHA1(273bb9c9f0a7eb48470601a0eadf450908ac4d92) )
	ROM_LOAD16_BYTE( "eg2vera.2j",   0x0000001, 0x200000, CRC(a8388248) SHA1(89eeb6095cc8c7ad6cdc8480cff6f688f07f64d7) )

	ROM_REGION32_LE( 0x1c00000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "eg1rom0l.12",  0x0000000, 0x800000, CRC(fe11a48e) SHA1(bdb2af5949bb4a324dab01489e9488aa5b9bd129) )
	ROM_LOAD16_BYTE( "eg1rom0u.11",  0x0000001, 0x800000, CRC(7cb90c25) SHA1(0ff6f7bf8b7eb2e0706bd235fcb9a95ea639fae6) )
	ROM_LOAD16_BYTE( "eg1fl1l.9",    0x1000000, 0x200000, CRC(dd4cac2b) SHA1(f49d0055303b3d8639e94f93bce6bf160cc99913) )
	ROM_LOAD16_BYTE( "eg1fl1u.10",   0x1000001, 0x200000, CRC(e3348407) SHA1(829bf1be0f74fd385e325d774f1449e28aba1e4d) )
	ROM_LOAD16_BYTE( "eg1fl2l.7",    0x1400000, 0x200000, CRC(34493262) SHA1(9dd5b1b25e7232460bf5ee5b339d9f536ec26979) )
	ROM_LOAD16_BYTE( "eg1fl2u.8",    0x1400001, 0x200000, CRC(4fb84f1d) SHA1(714e0ef56871d7b4568bc6dda08bbb1c01dbba37) )
	ROM_LOAD16_BYTE( "eg1fl3l.5",    0x1800000, 0x200000, CRC(f9441f87) SHA1(70944160620ba2fc80b4fc3a7b61c33622298a8b) )
	ROM_LOAD16_BYTE( "eg1fl3u.6",    0x1800001, 0x200000, CRC(cea397de) SHA1(3791dbadb5699c805c27930e59c61af4d62f77f7) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "eg1vera.11s", 0x0000000, 0x080000, CRC(9e44645e) SHA1(374eb4a4c09d6b5b7af5ff0efec16b4d2aacbe2b) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "eg1wave0.2",          0x0000000, 0x800000, CRC(961fe69f) SHA1(0189a061959a8d94b9d2db627911264faf9f28fd) )
ROM_END

ROM_START( ehrgeizua )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "eg3vera.2l",   0x0000000, 0x200000, CRC(64c00ff0) SHA1(fc7980bc8d98c810aed2eb6b3265d150784dfc15) )
	ROM_LOAD16_BYTE( "eg3vera.2p",   0x0000001, 0x200000, CRC(e722c030) SHA1(4669a7861c14d97048728989708a0fa3733f83a8) )

	ROM_REGION32_LE( 0x1c00000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "eg1rom0l.12",  0x0000000, 0x800000, CRC(fe11a48e) SHA1(bdb2af5949bb4a324dab01489e9488aa5b9bd129) )
	ROM_LOAD16_BYTE( "eg1rom0u.11",  0x0000001, 0x800000, CRC(7cb90c25) SHA1(0ff6f7bf8b7eb2e0706bd235fcb9a95ea639fae6) )
	ROM_LOAD16_BYTE( "eg1fl1l.9",    0x1000000, 0x200000, CRC(dd4cac2b) SHA1(f49d0055303b3d8639e94f93bce6bf160cc99913) )
	ROM_LOAD16_BYTE( "eg1fl1u.10",   0x1000001, 0x200000, CRC(e3348407) SHA1(829bf1be0f74fd385e325d774f1449e28aba1e4d) )
	ROM_LOAD16_BYTE( "eg1fl2l.7",    0x1400000, 0x200000, CRC(34493262) SHA1(9dd5b1b25e7232460bf5ee5b339d9f536ec26979) )
	ROM_LOAD16_BYTE( "eg1fl2u.8",    0x1400001, 0x200000, CRC(4fb84f1d) SHA1(714e0ef56871d7b4568bc6dda08bbb1c01dbba37) )
	ROM_LOAD16_BYTE( "eg1fl3l.5",    0x1800000, 0x200000, CRC(f9441f87) SHA1(70944160620ba2fc80b4fc3a7b61c33622298a8b) )
	ROM_LOAD16_BYTE( "eg1fl3u.6",    0x1800001, 0x200000, CRC(cea397de) SHA1(3791dbadb5699c805c27930e59c61af4d62f77f7) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "eg1vera.11s", 0x0000000, 0x080000, CRC(9e44645e) SHA1(374eb4a4c09d6b5b7af5ff0efec16b4d2aacbe2b) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "eg1wave0.2",          0x0000000, 0x800000, CRC(961fe69f) SHA1(0189a061959a8d94b9d2db627911264faf9f28fd) )
ROM_END

ROM_START( ehrgeizja )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "eg1vera.2l",   0x0000000, 0x200000, CRC(302d62cf) SHA1(e2de280ae4475829398a6770aed8eab0ed35b1ce) )
	ROM_LOAD16_BYTE( "eg1vera.2p",   0x0000001, 0x200000, CRC(1d7fb3a1) SHA1(dc038a639f89d7c8daaf987b728fde175fe4dbec) )

	ROM_REGION32_LE( 0x1c00000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "eg1rom0l.12",  0x0000000, 0x800000, CRC(fe11a48e) SHA1(bdb2af5949bb4a324dab01489e9488aa5b9bd129) )
	ROM_LOAD16_BYTE( "eg1rom0u.11",  0x0000001, 0x800000, CRC(7cb90c25) SHA1(0ff6f7bf8b7eb2e0706bd235fcb9a95ea639fae6) )
	ROM_LOAD16_BYTE( "eg1fl1l.9",    0x1000000, 0x200000, CRC(dd4cac2b) SHA1(f49d0055303b3d8639e94f93bce6bf160cc99913) )
	ROM_LOAD16_BYTE( "eg1fl1u.10",   0x1000001, 0x200000, CRC(e3348407) SHA1(829bf1be0f74fd385e325d774f1449e28aba1e4d) )
	ROM_LOAD16_BYTE( "eg1fl2l.7",    0x1400000, 0x200000, CRC(34493262) SHA1(9dd5b1b25e7232460bf5ee5b339d9f536ec26979) )
	ROM_LOAD16_BYTE( "eg1fl2u.8",    0x1400001, 0x200000, CRC(4fb84f1d) SHA1(714e0ef56871d7b4568bc6dda08bbb1c01dbba37) )
	ROM_LOAD16_BYTE( "eg1fl3l.5",    0x1800000, 0x200000, CRC(f9441f87) SHA1(70944160620ba2fc80b4fc3a7b61c33622298a8b) )
	ROM_LOAD16_BYTE( "eg1fl3u.6",    0x1800001, 0x200000, CRC(cea397de) SHA1(3791dbadb5699c805c27930e59c61af4d62f77f7) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "eg1vera.11s", 0x0000000, 0x080000, CRC(9e44645e) SHA1(374eb4a4c09d6b5b7af5ff0efec16b4d2aacbe2b) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "eg1wave0.2",          0x0000000, 0x800000, CRC(961fe69f) SHA1(0189a061959a8d94b9d2db627911264faf9f28fd) )
ROM_END

ROM_START( fgtlayer )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "ftl3vera.2e",  0x0000000, 0x200000, CRC(17c23471) SHA1(22b5a25569c085f059c1c846cf628a1f140035e9) )
	ROM_LOAD16_BYTE( "ftl3vera.2j",  0x0000001, 0x200000, CRC(1626465d) SHA1(941e7f8fbbaa232aab10a52aba1c462b4c91bb71) )

	ROM_REGION32_LE( 0x2000000, "bankedroms", 0 ) /* main data */
	ROM_LOAD(        "ftl1rom0.9",   0x0000000, 0x800000, CRC(e33ce365) SHA1(f7e5b98d2e8e5f233265909477c84331f016ebfb) )
	ROM_LOAD(        "ftl1rom1.10",  0x0800000, 0x800000, CRC(a1ec7d08) SHA1(e693a0b16235c44d4165b2f53dc25d5288297c27) )
	ROM_LOAD(        "ftl1rom2.11",  0x1000000, 0x800000, CRC(204be858) SHA1(95b22b678b7d11b0ec907698c18cebd84437c656) )
	ROM_LOAD16_BYTE( "ftl1fl3l.7",   0x1800000, 0x200000, CRC(e4fb01e1) SHA1(c7883e86afb58412281a317bfdf62a21488421be) )
	ROM_LOAD16_BYTE( "ftl1fl3h.8",   0x1800001, 0x200000, CRC(67a5c56f) SHA1(6f0a4f93b4975b24efb26c3dd47b791c92c55fbf) )
	ROM_LOAD16_BYTE( "ftl1fl4l.5",   0x1c00000, 0x200000, CRC(8039242c) SHA1(9c3f9637d7cd0c004c0c39aee815eed228ebad20) )
	ROM_LOAD16_BYTE( "ftl1fl4h.6",   0x1c00001, 0x200000, CRC(5ad59726) SHA1(b3a68f7ba2052b99407a5423223202001f2a4f67) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "ftl1vera.11s", 0x0000000, 0x080000, CRC(e3f957cd) SHA1(1c7f2033025fb9c40654cff26d78697baf697c59) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "ftl1wave0.2",         0x0000000, 0x800000, CRC(ee009a2b) SHA1(c332bf59917b2673d7acb864bf92d25d74a350b6) )
	ROM_LOAD( "ftl1wave1.1",         0x0800000, 0x800000, CRC(a54a89cd) SHA1(543b47c6442f7a78e26992b041235a91d719bb89) )
ROM_END

ROM_START( fgtlayerj )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "ftl0vera.2e",  0x0000000, 0x200000, CRC(f4156e79) SHA1(cedb917940be8c74fa4ddb48213ce6917444e306) ) // Service mode shows FTL0/VER.A
	ROM_LOAD16_BYTE( "ftl0vera.2j",  0x0000001, 0x200000, CRC(c65b57c0) SHA1(0051aa46d09fbe9d896ae5f534e21955373f1d46) )

	ROM_REGION32_LE( 0x2000000, "bankedroms", 0 ) /* main data */
	ROM_LOAD(        "ftl1rom0.9",   0x0000000, 0x800000, CRC(e33ce365) SHA1(f7e5b98d2e8e5f233265909477c84331f016ebfb) )
	ROM_LOAD(        "ftl1rom1.10",  0x0800000, 0x800000, CRC(a1ec7d08) SHA1(e693a0b16235c44d4165b2f53dc25d5288297c27) )
	ROM_LOAD(        "ftl1rom2.11",  0x1000000, 0x800000, CRC(204be858) SHA1(95b22b678b7d11b0ec907698c18cebd84437c656) )
	ROM_LOAD16_BYTE( "ftl1fl3l.7",   0x1800000, 0x200000, CRC(e4fb01e1) SHA1(c7883e86afb58412281a317bfdf62a21488421be) )
	ROM_LOAD16_BYTE( "ftl1fl3h.8",   0x1800001, 0x200000, CRC(67a5c56f) SHA1(6f0a4f93b4975b24efb26c3dd47b791c92c55fbf) )
	ROM_LOAD16_BYTE( "ftl1fl4l.5",   0x1c00000, 0x200000, CRC(8039242c) SHA1(9c3f9637d7cd0c004c0c39aee815eed228ebad20) )
	ROM_LOAD16_BYTE( "ftl1fl4h.6",   0x1c00001, 0x200000, CRC(5ad59726) SHA1(b3a68f7ba2052b99407a5423223202001f2a4f67) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "ftl1vera.11s", 0x0000000, 0x080000, CRC(e3f957cd) SHA1(1c7f2033025fb9c40654cff26d78697baf697c59) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "ftl1wave0.2",         0x0000000, 0x800000, CRC(ee009a2b) SHA1(c332bf59917b2673d7acb864bf92d25d74a350b6) )
	ROM_LOAD( "ftl1wave1.1",         0x0800000, 0x800000, CRC(a54a89cd) SHA1(543b47c6442f7a78e26992b041235a91d719bb89) )
ROM_END

ROM_START( golgo13 )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "glg1vera.2l",  0x0000000, 0x200000, CRC(aa15abfe) SHA1(e82b408746e01c50c5cb0dcef804974d1e97078a) )
	ROM_LOAD16_BYTE( "glg1vera.2p",  0x0000001, 0x200000, CRC(37a4cf90) SHA1(b5470d44036e9de8220b669f71b50bcec42d9a18) )

	ROM_REGION32_LE( 0x3c00000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "glg1rom0l.10", 0x0000000, 0x800000, CRC(2837524e) SHA1(81d811c2cf8121854ed6046dc421cda7fcd44014) )
	ROM_LOAD16_BYTE( "glg1rom0u.14", 0x0000001, 0x800000, CRC(4482deff) SHA1(03dcd12633b167d9af664ae826fc2c2ff096c0a6) )
	ROM_LOAD16_BYTE( "glg1rom1l.11", 0x1000000, 0x800000, CRC(080f3550) SHA1(40a5257baeb366e32ff51f571efb12a556f93430) )
	ROM_LOAD16_BYTE( "glg1rom1u.15", 0x1000001, 0x800000, CRC(b49e8613) SHA1(d25ede8046ce309d5a5515a2e44b6773fdd56333) )
	ROM_LOAD16_BYTE( "glg1rom2l.12", 0x2000000, 0x800000, CRC(e1a79a87) SHA1(c3f31b1bc6a3b51361df0d89d3ff0717875fd800) )
	ROM_LOAD16_BYTE( "glg1rom2u.16", 0x2000001, 0x800000, CRC(cda22852) SHA1(e609a55e0ec91b532469c3607dcc96456eec6a07) )
	ROM_LOAD16_BYTE( "glg1fl3l.4",   0x3000000, 0x200000, CRC(16ddc2cb) SHA1(71460bd4ad35b3488dc1326fe2f600df40988e06) )
	ROM_LOAD16_BYTE( "glg1fl3u.5",   0x3000001, 0x200000, CRC(d90d72d1) SHA1(0027389344862486c53cd0e1554b36eed65aa5b0) )
	ROM_LOAD16_BYTE( "glg1fl4l.6",   0x3400000, 0x200000, CRC(820d8133) SHA1(2d780612055a0fa5de756cfa46ce7f134139e550) )
	ROM_LOAD16_BYTE( "glg1fl4u.7",   0x3400001, 0x200000, CRC(02d78160) SHA1(2eafcbd3ba550acf7816074e6c7eee65f7d64859) )
	ROM_LOAD16_BYTE( "glg1fl5l.8",   0x3800000, 0x200000, CRC(090b2508) SHA1(c3587aeab71f6a9dcc90bf2af496303e9d20eac8) )
	ROM_LOAD16_BYTE( "glg1fl5u.9",   0x3800001, 0x200000, CRC(2231a07c) SHA1(480e17219101a0dae5cd64507e31cd7e711c95fa) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "glg1vera.11s", 0x0000000, 0x080000, CRC(5c33f240) SHA1(ec8fc8d83466b28dfa35b93e16d8164883513b19) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "glg1wave0.1",         0x0000000, 0x800000, CRC(672d4f7c) SHA1(16a7564a2c68840a438a33ac2381df4e70e1bb45) )
	ROM_LOAD( "glg1wave1.13",        0x0800000, 0x800000, CRC(480b0a1a) SHA1(341d5ec8ad0f3c0a121eeeec9466aaeec2bd1c74) )
ROM_END

ROM_START( g13knd )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "gls1vera.2e",    0x0000000, 0x200000, CRC(904c39a7) SHA1(e62a518657b639d31e390b8d1a36eee8a46ab179) )
	ROM_LOAD16_BYTE( "gls1vera.2j",    0x0000001, 0x200000, CRC(f8f9d6d2) SHA1(ec02192f3874fea289d123fd6d828148c77fbf6d) )

	ROM_REGION32_LE( 0x3c00000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "gls1rom0l.ic10", 0x0000000, 0x800000, CRC(91e04e7e) SHA1(70603ec5c17d9074f7176e7214f7507a3e408578) )
	ROM_LOAD16_BYTE( "gls1rom0u.ic14", 0x0000001, 0x800000, CRC(29f67094) SHA1(d17baa723bca5cadb483cd777d0ef25c29367426) )
	ROM_LOAD16_BYTE( "gls1rom1l.ic11", 0x1000000, 0x800000, CRC(fc69578f) SHA1(20bcd687aeb3741893588b492f93531e58405c3e) )
	ROM_LOAD16_BYTE( "gls1rom1u.ic15", 0x1000001, 0x800000, CRC(db46d626) SHA1(a623097fd17e99ee31c22ed62cb88aa9dc25721c) )
	ROM_LOAD16_BYTE( "gls1rom2l.ic12", 0x2000000, 0x800000, CRC(707cec76) SHA1(59257acc0b5a7f86a895873fc679ea184d2c0a50) )
	ROM_LOAD16_BYTE( "gls1rom2u.ic16", 0x2000001, 0x800000, CRC(970abad0) SHA1(fb41c379d28df2115baee967cc9645da869d2e2d) )
	ROM_LOAD16_BYTE( "gls1fl3l.ic4",   0x3000000, 0x200000, CRC(3a0ffd17) SHA1(03c704012040b9b9c24453baa8dbfbb3aec82854) )
	ROM_LOAD16_BYTE( "gls1fl3u.ic5",   0x3000001, 0x200000, CRC(565567ff) SHA1(36a26ed8fbba95dccbde6300363acb45d9c029b3) )
	ROM_LOAD16_BYTE( "gls1fl4l.ic6",   0x3400000, 0x200000, CRC(b4591cc0) SHA1(8da1defb74957bd02324d5b2d3af406dc1f7061b) )
	ROM_LOAD16_BYTE( "gls1fl4u.ic7",   0x3400001, 0x200000, CRC(679d545c) SHA1(b0e04cb123b4a15d935d773856e9cc954e81ba21) )
	ROM_LOAD16_BYTE( "gls1fl5l.ic8",   0x3800000, 0x200000, CRC(a5581533) SHA1(58721d3acc1a740e616b721b400c6912f21d516f) )
	ROM_LOAD16_BYTE( "gls1fl5u.ic9",   0x3800001, 0x200000, CRC(a8b6832a) SHA1(c0e392b7aa51dfcfce602fb0a46ccbc18074385c) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "gls1vera.11s", 0x0000000, 0x080000, CRC(a1e68d02) SHA1(bcacea12531539776d5ad5d34b64f7f4a10a0b97) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "gls1wave0.ic1",         0x0000000, 0x800000, CRC(9de10ef5) SHA1(c391998ed3d882d49d6dbe7079414a2bfd180b49) )
	ROM_LOAD( "gls1wave1.ic13",        0x0800000, 0x800000, CRC(4c92b6bf) SHA1(a463b5825f635efe402a6ea6c1ed8ecdc3d466e0) )
ROM_END

ROM_START( ghlpanic )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "ob2vera.2e",     0x0000000, 0x200000, CRC(77162ae0) SHA1(cdc0833756037562b49bb2ae02931b3b24d27329) )
	ROM_LOAD16_BYTE( "ob2vera.2j",     0x0000001, 0x200000, CRC(628f0830) SHA1(a547880674d95b84acc9c05413cba4fd3a81e0cf) )

	ROM_REGION32_LE( 0x00800000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "ob1rom0l.ic12",  0x0000000, 0x400000, CRC(f8b6a599) SHA1(2af3186242a8cbf14ab7532496a91041300527e9) )
	ROM_LOAD16_BYTE( "ob1rom0u.ic11",  0x0000001, 0x400000, CRC(0625db92) SHA1(b63be6e41b2c6e2194f02f0c31da1f30c4e08232) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "ob1vera.11s", 0x0000000, 0x080000, CRC(f8c459f2) SHA1(681520c891f5c8a0f321652d8834910310c88d1a) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "ob1wave.ic2",           0x000000, 0x800000, CRC(e7bc7202) SHA1(f0f598304866ebe62642eaac6b7d8709baa14fe1) )
ROM_END

ROM_START( ohbakyuun )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "ob1vera.2l",     0x0000000, 0x200000, CRC(4cb3c25a) SHA1(18679b9ae54f2b001316d9a55065c859df292c4c) )
	ROM_LOAD16_BYTE( "ob1vera.2p",     0x0000001, 0x200000, CRC(85abbe30) SHA1(0edfa34c45bd95882639a7d3e43bc9df3bc23586) )

	ROM_REGION32_LE( 0x00800000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "ob1rom0l.ic12",  0x0000000, 0x400000, CRC(f8b6a599) SHA1(2af3186242a8cbf14ab7532496a91041300527e9) )
	ROM_LOAD16_BYTE( "ob1rom0u.ic11",  0x0000001, 0x400000, CRC(0625db92) SHA1(b63be6e41b2c6e2194f02f0c31da1f30c4e08232) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "ob1vera.11s", 0x0000000, 0x080000, CRC(f8c459f2) SHA1(681520c891f5c8a0f321652d8834910310c88d1a) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "ob1wave.ic2",           0x000000, 0x800000, CRC(e7bc7202) SHA1(f0f598304866ebe62642eaac6b7d8709baa14fe1) )
ROM_END

ROM_START( kaiunqz )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "kw1vera.2l",   0x000000, 0x200000, CRC(fd1d2324) SHA1(eedb4627dfa17e9aac2c99592628f1fa7060edb5) )
	ROM_LOAD16_BYTE( "kw1vera.2p",   0x000001, 0x200000, CRC(d8bdea6b) SHA1(d32118846a1f43eecff7f56dcda03adf04975784) )

	ROM_REGION32_LE( 0x3000000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "kw1rom0l.12",  0x0000000, 0x800000, CRC(e0768f0b) SHA1(3ffec7c789a95fcff7f06432723f155b99a20c42) )
	ROM_LOAD16_BYTE( "kw1rom0u.11",  0x0000001, 0x800000, CRC(04101a16) SHA1(c2d79333ee382e2fea229bba23918fb5f88a25d4) )
	ROM_LOAD16_BYTE( "kw1rom1l.10",  0x1000000, 0x800000, CRC(6a4edfa6) SHA1(a59fa28446b28db28b38368c794af71e673f69a2) )
	ROM_LOAD16_BYTE( "kw1rom1u.9",   0x1000001, 0x800000, CRC(e6150b0d) SHA1(7dd1ba524246aea007e7bd6d3e7d457033b23a09) )
	ROM_LOAD16_BYTE( "kw1rom2l.7",   0x2000000, 0x800000, CRC(21c376e4) SHA1(4b82494c1e13ebcea3c4e97803c0667be73bf6ae) )
	ROM_LOAD16_BYTE( "kw1rom2u.8",   0x2000001, 0x800000, CRC(d850c222) SHA1(802c1782b323802eae871e2b9ccc5eae34277861) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "kw1vera.11s",  0x000000, 0x080000, CRC(d863fafa) SHA1(3c2bb38c24165e3a1a4d1d257fcfc019b63d5199) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "kw1wave0.2",   0x000000, 0x800000, CRC(060e52ae) SHA1(7ea95cae9d3c648163b225d0c7d365644be90241) )
ROM_END

ROM_START( kartduel )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "ktd2vera.2l",  0x0000000, 0x200000, CRC(7e7d8454) SHA1(4e0db56e433015cf1b9fd4cad35df99f1730e567) )
	ROM_LOAD16_BYTE( "ktd2vera.2p",  0x0000001, 0x200000, CRC(6c8af7e7) SHA1(602975500180de98a0a7e22c079344e7a3ece25e) )

	ROM_REGION32_LE( 0x01000000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "kdt1rom0l.ic12", 0x000000, 0x800000, CRC(f8a79aef) SHA1(8c2a6aef5cc0ffe4b5e9f2a98ef5a29edd59eb4d) )
	ROM_LOAD16_BYTE( "kdt1rom0u.ic11", 0x000001, 0x800000, CRC(bab0d328) SHA1(9a15bfb38c63b0012f29755b2be071e9c82d1c20) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "ktd1vera.11s", 0x000000, 0x080000, CRC(c2ff1971) SHA1(32ee2afe08e92049d8139c9324a0ea1a3b7ee5a1) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "ktd1wave0.ic2",  0x0000000, 0x800000, CRC(c54d5539) SHA1(17518dc76bb3627ca42bac665938bb11ea926396) )
ROM_END

ROM_START( kartduelj )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "ktd1vera.2l",  0x0000000, 0x200000, CRC(0c207249) SHA1(6c57de25d452226a25f658638d89b81257960741) )
	ROM_LOAD16_BYTE( "ktd1vera.2p",  0x0000001, 0x200000, CRC(f6e2581f) SHA1(06eb108c2775290590dba75f964f26443a585d70) )

	ROM_REGION32_LE( 0x01000000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "kdt1rom0l.ic12", 0x000000, 0x800000, CRC(f8a79aef) SHA1(8c2a6aef5cc0ffe4b5e9f2a98ef5a29edd59eb4d) )
	ROM_LOAD16_BYTE( "kdt1rom0u.ic11", 0x000001, 0x800000, CRC(bab0d328) SHA1(9a15bfb38c63b0012f29755b2be071e9c82d1c20) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "ktd1vera.11s", 0x000000, 0x080000, CRC(c2ff1971) SHA1(32ee2afe08e92049d8139c9324a0ea1a3b7ee5a1) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "ktd1wave0.ic2",  0x0000000, 0x800000, CRC(c54d5539) SHA1(17518dc76bb3627ca42bac665938bb11ea926396) )
ROM_END

ROM_START( lbgrande )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "lg2vera.2l",   0x0000000, 0x200000, CRC(5ed6b152) SHA1(fdab457862bd6e0a3178c9329bd0978b6aa3ae2f) )
	ROM_LOAD16_BYTE( "lg2vera.2p",   0x0000001, 0x200000, CRC(97c57149) SHA1(bb9bc1ba3ea826eb1c987b11218c0afa0fc54bdc) )

	ROM_REGION32_LE( 0x1c00000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "lg1rom0l.6",   0x0000000, 0x400000, CRC(c5df7f27) SHA1(07c596efb2533b9adc579874b7e8ef7fcc7f73c3) )
	ROM_LOAD16_BYTE( "lg1rom0u.9",   0x0000001, 0x400000, CRC(74607817) SHA1(448a9213fa566fdbab5d789df064da7dc946ba2c) )
	ROM_LOAD16_BYTE( "lg1fl3l.12",   0x1800000, 0x200000, CRC(c9947d3e) SHA1(239b1f81ffac6a54b438082664124b6cf51a9b1c) )
	ROM_LOAD16_BYTE( "lg1fl3u.13",   0x1800001, 0x200000, CRC(f3d69f45) SHA1(546f588f144e1a75eee4a6d0a6cef8a3f79f0238) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "lg1vera.11s", 0x0000000, 0x080000, CRC(de717a09) SHA1(78f26ff630c50632916fa17fa870dcde7f13781d) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "lg1wave0.5",          0x0000000, 0x400000, CRC(4647fada) SHA1(99f5e9ded0c83f1a0d3670f6380bc15c1380671e) )
ROM_END

ROM_START( mdhorse )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "mdh1vera.2l",  0x0000000, 0x200000, CRC(fbb567b2) SHA1(899dccdfbc8dcbcdaf9b5df93e249a36f8cbf999) )
	ROM_LOAD16_BYTE( "mdh1vera.2p",  0x0000001, 0x200000, CRC(a0f182ab) SHA1(70c789ea88248c1f810f9fdb3feaf808acbaa8cd) )

	ROM_REGION32_LE( 0x3000000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "mdh1rom0l",    0x0000000, 0x800000, CRC(ca5bf806) SHA1(8c88b1f29c96a0696ac9428c411cde10faacce35) )
	ROM_LOAD16_BYTE( "mdh1rom0u",    0x0000001, 0x800000, CRC(315e9539) SHA1(340fcd196f53f64b3f56ef73101eddc9e142d907) )
	ROM_LOAD16_BYTE( "mdh1rom1l",    0x1000000, 0x800000, CRC(9f610211) SHA1(8459733c52d1c62033a4aeb9985b4a6e863a62d0) )
	ROM_LOAD16_BYTE( "mdh1rom1u",    0x1000001, 0x800000, CRC(a2e43560) SHA1(b0657a22701c6f2098f210d59e4c9bc88593a750) )
	ROM_LOAD16_BYTE( "mdh1rom2l",    0x2000000, 0x800000, CRC(84840fa9) SHA1(0f051db55b30f7dd473f4df9bb5e7d38c39dc785) )
	ROM_LOAD16_BYTE( "mdh1rom2u",    0x2000001, 0x800000, CRC(9490dafe) SHA1(c1bd9343535876eac5369a86105013b4a7d731b3) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "mdh1vera.11s", 0x0000000, 0x080000, CRC(20d7ba29) SHA1(95a056d1f1ac70dda8ced832b506076485348a33) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "mdh1wave0",           0x0000000, 0x800000, CRC(7b031123) SHA1(7cbc1f71d259405f9f1ef26026d51abcb255b057) )
ROM_END

ROM_START( mrdrillr )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "dri3vera2.2l",  0x0000000, 0x200000, CRC(36b9eeab) SHA1(85f5d57d4fc6160bfed49a08ad31a2cc2b4a38c5) )
	ROM_LOAD16_BYTE( "dri3vera2.2p",  0x0000001, 0x200000, CRC(811c00d5) SHA1(627685078fceb023e8af1b7af5269f9b01a06dba) )

	ROM_REGION32_LE( 0x0800000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "dri1rom0l.6",  0x0000000, 0x400000, CRC(021bb2fa) SHA1(bfe3e46e9728d5b5a692f432515267ff8b8255e7) )
	ROM_LOAD16_BYTE( "dri1rom0u.9",  0x0000001, 0x400000, CRC(5aae85ea) SHA1(a54dcc050c12ed3d77efc328e366e99c392eb139) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "dri1vera.11s", 0x0000000, 0x080000, CRC(33ea9c0e) SHA1(5018d7a1a45ec3133cd928435db8804f66321924) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "dri1wave0.5",         0x0000000, 0x800000, CRC(32928df1) SHA1(79af92a2d24a0e3d5bfe1785776b0f86a93882ce) )
ROM_END

ROM_START( mrdrillrj )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "dri1vera2.2l",  0x0000000, 0x200000, CRC(751ca21d) SHA1(1c271bba83d387c797ce8daa43885bcb6e1a51a6) )
	ROM_LOAD16_BYTE( "dri1vera2.2p",  0x0000001, 0x200000, CRC(2a2b0704) SHA1(5a8b40c6cf0adc43ca2ee0c576ec82f314aacd2c) )

	ROM_REGION32_LE( 0x0800000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "dri1rom0l.6",  0x0000000, 0x400000, CRC(021bb2fa) SHA1(bfe3e46e9728d5b5a692f432515267ff8b8255e7) )
	ROM_LOAD16_BYTE( "dri1rom0u.9",  0x0000001, 0x400000, CRC(5aae85ea) SHA1(a54dcc050c12ed3d77efc328e366e99c392eb139) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "dri1vera.11s", 0x0000000, 0x080000, CRC(33ea9c0e) SHA1(5018d7a1a45ec3133cd928435db8804f66321924) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "dri1wave0.5",         0x0000000, 0x800000, CRC(32928df1) SHA1(79af92a2d24a0e3d5bfe1785776b0f86a93882ce) )
ROM_END

ROM_START( pacapp )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "ppp1vera.2l",  0x0000000, 0x200000, CRC(6e74bd05) SHA1(41a2e06538cea3bced2992f5858a3f0cd1c0b4aa) )
	ROM_LOAD16_BYTE( "ppp1vera.2p",  0x0000001, 0x200000, CRC(b7a2f724) SHA1(820ae04ec416b8394a1d919279748bde3460cb96) )

	ROM_REGION32_LE( 0x1800000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "ppp1rom0l.6",  0x0000000, 0x400000, CRC(b152fdd8) SHA1(23c5c07680a62e941a7b1a28897f986dd9399801) )
	ROM_LOAD16_BYTE( "ppp1rom0u.9",  0x0000001, 0x400000, CRC(c615c26e) SHA1(db1aae37ebee2a74636415e4b1b0b17790a6a67e) )
	ROM_LOAD16_BYTE( "ppp1rom1l.7",  0x0800000, 0x400000, CRC(46eaedbd) SHA1(afe3c966fcf083d89b45d44d871bed1b8caa3014) )
	ROM_LOAD16_BYTE( "ppp1rom1u.10", 0x0800001, 0x400000, CRC(32f27dce) SHA1(06de870b83c972403d96b8b9a8ee5a192a99451d) )
	ROM_LOAD16_BYTE( "ppp1rom2l.8",  0x1000000, 0x400000, CRC(dca7e5ed) SHA1(b43b6e086912009295ceae11d2d540733353d7b6) )
	ROM_LOAD16_BYTE( "ppp1rom2u.11", 0x1000001, 0x400000, CRC(cada7a0d) SHA1(e8d927a4680911b77de1d906b5e0140697e9c67b) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "ppp1vera.11s", 0x0000000, 0x080000, CRC(22242317) SHA1(e46bf3c594136168faedebbd59f53ff9a6ecf3c1) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "ppp1wave0.5",         0x0000000, 0x800000, CRC(184ccc7d) SHA1(b74638cebef209638c625aac7e8c5b924f56a8bb) )
	ROM_LOAD( "ppp1wave1.4",         0x0800000, 0x800000, CRC(cbcf74c5) SHA1(a089277c9befc87b5bbe0d4e5b8187a4ad5ef143) )
ROM_END

ROM_START( pacappsp )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "psp1vera.2l",  0x0000000, 0x200000, CRC(4b6943af) SHA1(63b21794719bc1fc075e9cc4f1c1783442860036) )
	ROM_LOAD16_BYTE( "psp1vera.2p",  0x0000001, 0x200000, CRC(91397f04) SHA1(db3dd59edcdec10eb2fee74450c024a7ecffe1c9) )

	ROM_REGION32_LE( 0x1800000, "bankedroms", 0 ) /* main data */
	ROM_LOAD(        "psp1rom0.ic7", 0x0000000, 0x800000, CRC(7c26ff47) SHA1(f9d366ae9eb11e14875b611112bc82f7c7d391e0) )
	ROM_LOAD(        "psp1rom1.ic8", 0x0800000, 0x800000, CRC(4b764fc3) SHA1(92a7f0f1537f2f4d5a8b0218720978b576f78dad) )
	ROM_LOAD(        "psp1rom2.ic9", 0x1000000, 0x800000, CRC(a3fa6730) SHA1(0126a77661c3add0bfec9f6ef15021e602b7a614) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "psp1vera.11s", 0x0000000, 0x080000, CRC(47b9ccab) SHA1(15d4c5ac9ae7202df2d0ef84092679d190f2c45a) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "psp1wave0.ic2",       0x0000000, 0x800000, CRC(44b9a327) SHA1(ab20d5371f7a8c3a7fbc21fcf54a93c1cd007cd2) )
	ROM_LOAD( "psp1wave1.ic1",       0x0800000, 0x800000, CRC(c045b09c) SHA1(1cfd7003771576f6574d91d862af1f76ceaed64f) )
ROM_END

ROM_START( pacapp2 )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "pks1vera.2l",  0x0000000, 0x200000, CRC(aec428d3) SHA1(c13aecc6a367d6da501dce66fecbab5458ecac53) )
	ROM_LOAD16_BYTE( "pks1vera.2p",  0x0000001, 0x200000, CRC(289e6e8a) SHA1(b8197355bee5660e8ff78a1c427c6d2b94a12b9d) )

	ROM_REGION32_LE( 0x1800000, "bankedroms", 0 ) /* main data */
	ROM_LOAD(        "pks1rom0.ic7", 0x0000000, 0x800000, CRC(91f151fc) SHA1(84b4c9b2a9ae5687e1cba5f0a0db96495956cbfe) )
	ROM_LOAD(        "pks1rom1.ic8", 0x0800000, 0x800000, CRC(8195f981) SHA1(1b9c2289e0975606ff52b9974778f82c9c29e955) )
	ROM_LOAD(        "pks1rom2.ic9", 0x1000000, 0x800000, CRC(fea93ddc) SHA1(5f9c3f899a11797746d9c1077960f918aaca95f4) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "pks1vera.11s", 0x0000000, 0x080000, CRC(4942b588) SHA1(b28eb64a7347259619e32fdd396770983e25a707) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "pks1wave0.ic2",       0x0000000, 0x800000, CRC(3ebd27c3) SHA1(af874c3fb4cf1651075f307daefe7e3434769ecb) )
	ROM_LOAD( "pks1wave1.ic1",       0x0800000, 0x800000, CRC(07e426b7) SHA1(6dd038d4f2c95470f1ca09cfead83802e9480d02) )
ROM_END

ROM_START( ptblank2 )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "gnb5vera.2l",  0x0000000, 0x200000, CRC(4d0ef3b7) SHA1(6c4077316fa90b734c4a4e0aa3eadd26e97bd6ce) )
	ROM_LOAD16_BYTE( "gnb5vera.2p",  0x0000001, 0x200000, CRC(5d1d19ff) SHA1(4aa8ba7233d7f9bac759c98f53e637c1f3659c3f) )

	ROM_REGION32_LE( 0x1000000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "gnb1prg0l.ic12", 0x000000, 0x800000, CRC(78746037) SHA1(d130ca1153a730e3c967945248f00662f9fab304) )
	ROM_LOAD16_BYTE( "gnb1prg0u.ic11", 0x000001, 0x800000, CRC(697d3279) SHA1(40302780f7494d9413888b2d1da38bd14a9a444f) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "gnb_vera.11s", 0x000000, 0x080000, CRC(d45a53eb) SHA1(01eb4f659b29671d417c5d3684f15a5876f76009) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "gnb1waveb.ic2",  0x0000000, 0x400000, CRC(4e19d9d6) SHA1(0a92c987536999a789663a30c787950ab6995128) )
	ROM_RELOAD( 0x800000, 0x400000 )
ROM_END

ROM_START( gunbarl )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "gnb4vera.2l",  0x0000000, 0x200000, CRC(88c05cde) SHA1(80d210b06c8eda19e37430fb34492885d9eec671) )
	ROM_LOAD16_BYTE( "gnb4vera.2p",  0x0000001, 0x200000, CRC(7d57437a) SHA1(b4fc960b11e7dc9d3bde567f534635f264276e53) )

	ROM_REGION32_LE( 0x1000000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "gnb1prg0l.ic12", 0x000000, 0x800000, CRC(78746037) SHA1(d130ca1153a730e3c967945248f00662f9fab304) )
	ROM_LOAD16_BYTE( "gnb1prg0u.ic11", 0x000001, 0x800000, CRC(697d3279) SHA1(40302780f7494d9413888b2d1da38bd14a9a444f) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "gnb_vera.11s", 0x000000, 0x080000, CRC(d45a53eb) SHA1(01eb4f659b29671d417c5d3684f15a5876f76009) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "gnb1waveb.ic2",  0x0000000, 0x400000, CRC(4e19d9d6) SHA1(0a92c987536999a789663a30c787950ab6995128) )
	ROM_RELOAD( 0x800000, 0x400000 )
ROM_END

ROM_START( soulclbr )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "soc14verc.2e", 0x0000000, 0x200000, CRC(c40e9614) SHA1(dc20469f0d657423e472fdf5897852ab9fb8bb73) )
	ROM_LOAD16_BYTE( "soc14verc.2j", 0x0000001, 0x200000, CRC(80c41446) SHA1(e5620a4f0ffba913169a779df73384b7ca8780b9) )

	ROM_REGION32_LE( 0x2000000, "bankedroms", 0 ) /* main data */
	ROM_LOAD(        "soc1rom0.7",   0x0000000, 0x800000, CRC(cdc47b55) SHA1(315ea6b819de5c4883aa400f1b9f4172637757bf) )
	ROM_LOAD(        "soc1rom1.8",   0x0800000, 0x800000, CRC(30d2dd5a) SHA1(1c0c467ba339e0241efb8d5c3b025a046b2ca676) )
	ROM_LOAD(        "soc1rom2.9",   0x1000000, 0x800000, CRC(dbb93955) SHA1(624cd8ad94e8ae53206f798bff81784afe95e5f1) )
	ROM_LOAD(        "soc1fl3.6",    0x1800000, 0x400000, CRC(24d94c38) SHA1(0f9b9ab11dd4e02086d7b9104ce2f5d4e93cd696) )
	ROM_LOAD(        "soc1fl4.5",    0x1c00000, 0x400000, CRC(6212090e) SHA1(ed5e50771180935a0c2e760e7369673098722201) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "soc1vera.11s", 0x0000000, 0x080000, CRC(52aa206a) SHA1(5abe9d6f800fa1b9623aa08b16e9b959b840e50b) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "soc1wave0.2",         0x0000000, 0x800000, CRC(c100618d) SHA1(b87f88ee42ad9c5affa674e5f816d902143fed99) )
ROM_END

ROM_START( soulclbrab )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "soc14verb.2l", 0x000000, 0x200000, CRC(6af5c5f6) SHA1(51d1e7d78d95cfc765cd219ed07b405cd920044b) )
	ROM_LOAD16_BYTE( "soc14verb.2p", 0x000001, 0x200000, CRC(23e7a4c4) SHA1(a97f36cafdeff9e26fbd24e54ab8ac8080763761) )

	ROM_REGION32_LE( 0x2000000, "bankedroms", 0 ) /* main data */
	ROM_LOAD(        "soc1rom0.7",   0x0000000, 0x800000, CRC(cdc47b55) SHA1(315ea6b819de5c4883aa400f1b9f4172637757bf) )
	ROM_LOAD(        "soc1rom1.8",   0x0800000, 0x800000, CRC(30d2dd5a) SHA1(1c0c467ba339e0241efb8d5c3b025a046b2ca676) )
	ROM_LOAD(        "soc1rom2.9",   0x1000000, 0x800000, CRC(dbb93955) SHA1(624cd8ad94e8ae53206f798bff81784afe95e5f1) )
	ROM_LOAD(        "soc1fl3.6",    0x1800000, 0x400000, CRC(24d94c38) SHA1(0f9b9ab11dd4e02086d7b9104ce2f5d4e93cd696) )
	ROM_LOAD(        "soc1fl4.5",    0x1c00000, 0x400000, CRC(6212090e) SHA1(ed5e50771180935a0c2e760e7369673098722201) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "soc1vera.11s", 0x0000000, 0x080000, CRC(52aa206a) SHA1(5abe9d6f800fa1b9623aa08b16e9b959b840e50b) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "soc1wave0.2",         0x0000000, 0x800000, CRC(c100618d) SHA1(b87f88ee42ad9c5affa674e5f816d902143fed99) )
ROM_END

ROM_START( soulclbruc )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "soc13verc.2l", 0x0000000, 0x200000, CRC(4ba962fb) SHA1(e2b5e543d92a4157788482f3ab7c6b0e5ff30367) )
	ROM_LOAD16_BYTE( "soc13verc.2p", 0x0000001, 0x200000, CRC(140c40de) SHA1(352faec0fff5a8422ee7c8db2e0c946b139be03f) )

	ROM_REGION32_LE( 0x2000000, "bankedroms", 0 ) /* main data */
	ROM_LOAD(        "soc1rom0.7",   0x0000000, 0x800000, CRC(cdc47b55) SHA1(315ea6b819de5c4883aa400f1b9f4172637757bf) )
	ROM_LOAD(        "soc1rom1.8",   0x0800000, 0x800000, CRC(30d2dd5a) SHA1(1c0c467ba339e0241efb8d5c3b025a046b2ca676) )
	ROM_LOAD(        "soc1rom2.9",   0x1000000, 0x800000, CRC(dbb93955) SHA1(624cd8ad94e8ae53206f798bff81784afe95e5f1) )
	ROM_LOAD(        "soc1fl3.6",    0x1800000, 0x400000, CRC(24d94c38) SHA1(0f9b9ab11dd4e02086d7b9104ce2f5d4e93cd696) )
	ROM_LOAD(        "soc1fl4.5",    0x1c00000, 0x400000, CRC(6212090e) SHA1(ed5e50771180935a0c2e760e7369673098722201) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "soc1vera.11s", 0x0000000, 0x080000, CRC(52aa206a) SHA1(5abe9d6f800fa1b9623aa08b16e9b959b840e50b) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "soc1wave0.2",         0x0000000, 0x800000, CRC(c100618d) SHA1(b87f88ee42ad9c5affa674e5f816d902143fed99) )
ROM_END

ROM_START( soulclbrjc )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "soc11verc.2l", 0x0000000, 0x200000, CRC(f5e3679c) SHA1(b426cfc7707a6772e6aabbaf4a19b7f008324d55) )
	ROM_LOAD16_BYTE( "soc11verc.2p", 0x0000001, 0x200000, CRC(7537719c) SHA1(d83d4c762fa7fcfd5d84de550568e92999e5bdfb) )

	ROM_REGION32_LE( 0x2000000, "bankedroms", 0 ) /* main data */
	ROM_LOAD(        "soc1rom0.7",   0x0000000, 0x800000, CRC(cdc47b55) SHA1(315ea6b819de5c4883aa400f1b9f4172637757bf) )
	ROM_LOAD(        "soc1rom1.8",   0x0800000, 0x800000, CRC(30d2dd5a) SHA1(1c0c467ba339e0241efb8d5c3b025a046b2ca676) )
	ROM_LOAD(        "soc1rom2.9",   0x1000000, 0x800000, CRC(dbb93955) SHA1(624cd8ad94e8ae53206f798bff81784afe95e5f1) )
	ROM_LOAD(        "soc1fl3.6",    0x1800000, 0x400000, CRC(24d94c38) SHA1(0f9b9ab11dd4e02086d7b9104ce2f5d4e93cd696) )
	ROM_LOAD(        "soc1fl4.5",    0x1c00000, 0x400000, CRC(6212090e) SHA1(ed5e50771180935a0c2e760e7369673098722201) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "soc1vera.11s", 0x0000000, 0x080000, CRC(52aa206a) SHA1(5abe9d6f800fa1b9623aa08b16e9b959b840e50b) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "soc1wave0.2",         0x0000000, 0x800000, CRC(c100618d) SHA1(b87f88ee42ad9c5affa674e5f816d902143fed99) )
ROM_END

ROM_START( soulclbrub )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "soc13verb.2e", 0x0000000, 0x200000, CRC(ad7cfb1e) SHA1(7d1e7fd0024e31780335690906846e91ba063003) )
	ROM_LOAD16_BYTE( "soc13verb.2j", 0x0000001, 0x200000, CRC(7449c045) SHA1(1c7a8b659d0f12dded2a00bc83baeb392fd7a719) )

	ROM_REGION32_LE( 0x2000000, "bankedroms", 0 ) /* main data */
	ROM_LOAD(        "soc1rom0.7",   0x0000000, 0x800000, CRC(cdc47b55) SHA1(315ea6b819de5c4883aa400f1b9f4172637757bf) )
	ROM_LOAD(        "soc1rom1.8",   0x0800000, 0x800000, CRC(30d2dd5a) SHA1(1c0c467ba339e0241efb8d5c3b025a046b2ca676) )
	ROM_LOAD(        "soc1rom2.9",   0x1000000, 0x800000, CRC(dbb93955) SHA1(624cd8ad94e8ae53206f798bff81784afe95e5f1) )
	ROM_LOAD(        "soc1fl3.6",    0x1800000, 0x400000, CRC(24d94c38) SHA1(0f9b9ab11dd4e02086d7b9104ce2f5d4e93cd696) )
	ROM_LOAD(        "soc1fl4.5",    0x1c00000, 0x400000, CRC(6212090e) SHA1(ed5e50771180935a0c2e760e7369673098722201) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "soc1vera.11s", 0x0000000, 0x080000, CRC(52aa206a) SHA1(5abe9d6f800fa1b9623aa08b16e9b959b840e50b) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "soc1wave0.2",         0x0000000, 0x800000, CRC(c100618d) SHA1(b87f88ee42ad9c5affa674e5f816d902143fed99) )
ROM_END

ROM_START( soulclbrjb )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "soc11verb.2e", 0x0000000, 0x200000, CRC(9660d996) SHA1(6361abfd8b0d29848aabad6a5c517ba0d336359a) )
	ROM_LOAD16_BYTE( "soc11verb.2j", 0x0000001, 0x200000, CRC(49939880) SHA1(a53fb8ecd71c8d59b0e08d6233ea658ae083bc6d) )

	ROM_REGION32_LE( 0x2000000, "bankedroms", 0 ) /* main data */
	ROM_LOAD(        "soc1rom0.7",   0x0000000, 0x800000, CRC(cdc47b55) SHA1(315ea6b819de5c4883aa400f1b9f4172637757bf) )
	ROM_LOAD(        "soc1rom1.8",   0x0800000, 0x800000, CRC(30d2dd5a) SHA1(1c0c467ba339e0241efb8d5c3b025a046b2ca676) )
	ROM_LOAD(        "soc1rom2.9",   0x1000000, 0x800000, CRC(dbb93955) SHA1(624cd8ad94e8ae53206f798bff81784afe95e5f1) )
	ROM_LOAD(        "soc1fl3.6",    0x1800000, 0x400000, CRC(24d94c38) SHA1(0f9b9ab11dd4e02086d7b9104ce2f5d4e93cd696) )
	ROM_LOAD(        "soc1fl4.5",    0x1c00000, 0x400000, CRC(6212090e) SHA1(ed5e50771180935a0c2e760e7369673098722201) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "soc1vera.11s", 0x0000000, 0x080000, CRC(52aa206a) SHA1(5abe9d6f800fa1b9623aa08b16e9b959b840e50b) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "soc1wave0.2",         0x0000000, 0x800000, CRC(c100618d) SHA1(b87f88ee42ad9c5affa674e5f816d902143fed99) )
ROM_END

ROM_START( soulclbra )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "soc12vera.2l",  0x0000000, 0x200000, CRC(ceadcc9a) SHA1(68cd3d15f28178fcea8aada6067b7060edfb2913) )
	ROM_LOAD16_BYTE( "soc12vera.2p",  0x0000001, 0x200000, CRC(65a74cf0) SHA1(1129c078925bd02f4e9cef64cd7761968be53537) )

	ROM_REGION32_LE( 0x2000000, "bankedroms", 0 ) /* main data */
	ROM_LOAD(        "soc1rom0.7",   0x0000000, 0x800000, CRC(cdc47b55) SHA1(315ea6b819de5c4883aa400f1b9f4172637757bf) )
	ROM_LOAD(        "soc1rom1.8",   0x0800000, 0x800000, CRC(30d2dd5a) SHA1(1c0c467ba339e0241efb8d5c3b025a046b2ca676) )
	ROM_LOAD(        "soc1rom2.9",   0x1000000, 0x800000, CRC(dbb93955) SHA1(624cd8ad94e8ae53206f798bff81784afe95e5f1) )
	ROM_LOAD(        "soc1fl3.6",    0x1800000, 0x400000, CRC(24d94c38) SHA1(0f9b9ab11dd4e02086d7b9104ce2f5d4e93cd696) )
	ROM_LOAD(        "soc1fl4.5",    0x1c00000, 0x400000, CRC(6212090e) SHA1(ed5e50771180935a0c2e760e7369673098722201) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "soc1vera.11s", 0x0000000, 0x080000, CRC(52aa206a) SHA1(5abe9d6f800fa1b9623aa08b16e9b959b840e50b) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "soc1wave0.2",         0x0000000, 0x800000, CRC(c100618d) SHA1(b87f88ee42ad9c5affa674e5f816d902143fed99) )
ROM_END

ROM_START( soulclbrja )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "soc1vera.2l",  0x0000000, 0x200000, CRC(37e0a203) SHA1(3915b5e530c8e70a07aa8ccedeb66633ae5f670e) )
	ROM_LOAD16_BYTE( "soc1vera.2p",  0x0000001, 0x200000, CRC(7cd87a35) SHA1(5a4837b6f6a49c88126a0ddbb8059a4da77127bc) )

	ROM_REGION32_LE( 0x2000000, "bankedroms", 0 ) /* main data */
	ROM_LOAD(        "soc1rom0.7",   0x0000000, 0x800000, CRC(cdc47b55) SHA1(315ea6b819de5c4883aa400f1b9f4172637757bf) )
	ROM_LOAD(        "soc1rom1.8",   0x0800000, 0x800000, CRC(30d2dd5a) SHA1(1c0c467ba339e0241efb8d5c3b025a046b2ca676) )
	ROM_LOAD(        "soc1rom2.9",   0x1000000, 0x800000, CRC(dbb93955) SHA1(624cd8ad94e8ae53206f798bff81784afe95e5f1) )
	ROM_LOAD(        "soc1fl3.6",    0x1800000, 0x400000, CRC(24d94c38) SHA1(0f9b9ab11dd4e02086d7b9104ce2f5d4e93cd696) )
	ROM_LOAD(        "soc1fl4.5",    0x1c00000, 0x400000, CRC(6212090e) SHA1(ed5e50771180935a0c2e760e7369673098722201) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "soc1vera.11s", 0x0000000, 0x080000, CRC(52aa206a) SHA1(5abe9d6f800fa1b9623aa08b16e9b959b840e50b) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "soc1wave0.2",         0x0000000, 0x800000, CRC(c100618d) SHA1(b87f88ee42ad9c5affa674e5f816d902143fed99) )
ROM_END

ROM_START( sws98 )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "ss81vera.2l",  0x0000000, 0x200000, CRC(94b1f34c) SHA1(0c8491fda366b5b2874e5f49959dccd11d372e46) )
	ROM_LOAD16_BYTE( "ss81vera.2p",  0x0000001, 0x200000, CRC(7d0ed33d) SHA1(34342ce57b29ee15c6279c099bb145ac7ad262f3) )

	ROM_REGION32_LE( 0x1c00000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "ss81fl1l.9",   0x1000000, 0x200000, CRC(b0b5dc77) SHA1(abce4e6ae60858b7c7408d10975b7a1e0d183115) )
	ROM_LOAD16_BYTE( "ss81fl1u.10",  0x1000001, 0x200000, CRC(e526dba5) SHA1(889fdfba17282eb05a3e254af81ee15c3e16acc4) )
	ROM_LOAD16_BYTE( "ss81fl2l.7",   0x1400000, 0x200000, CRC(2dc6f6b5) SHA1(e6c7bb804d7d027acca5adb15b0bb95321905ff3) )
	ROM_LOAD16_BYTE( "ss81fl2u.8",   0x1400001, 0x200000, CRC(c8341c9f) SHA1(b7c5a960a238e963c7ec5bdb0751a8730d3b09f7) )
	ROM_LOAD16_BYTE( "ss81fl3l.5",   0x1800000, 0x200000, CRC(ce5d2a96) SHA1(08f3955e1ce949ce436f6f2f19fd42204682ae8c) )
	ROM_LOAD16_BYTE( "ss81fl3u.6",   0x1800001, 0x200000, CRC(02df4df8) SHA1(01fd66c905cde9ca073dc4b6dbd826c5fe96b79b) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "ss81vera.11s", 0x0000000, 0x080000, CRC(c6bc5c31) SHA1(c6ef46c3fa8a7749618126d360e614ea6c8d9c54) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "ss81wave0.2",         0x0000000, 0x800000, CRC(1c5e2ff1) SHA1(4f29dfd49f6b5c3ca3b758823d368051354bd673) )
	ROM_LOAD( "ss81wave1.1",         0x0800000, 0x800000, CRC(5f4c8861) SHA1(baee7182c32bc064d805de5a16948faf78941ac4) )
ROM_END

ROM_START( sws99 )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "ss91vera.2e",  0x0000000, 0x200000, CRC(4dd928d7) SHA1(d76c0f52d1a2cd101a6879e6ff57ed1c52b5e228) )
	ROM_LOAD16_BYTE( "ss91vera.2j",  0x0000001, 0x200000, CRC(40777a48) SHA1(6e3052ddbe3943eb2418cd50102cead88b850240) )

	ROM_REGION32_LE( 0x2000000, "bankedroms", 0 ) /* main data */
	ROM_LOAD(        "ss91rom0.9",   0x0000000, 0x800000, CRC(db5bc50d) SHA1(b8b59f6db3a374277871c39b3657cb193525e558) )
	ROM_LOAD(        "ss91rom1.10",  0x0800000, 0x800000, CRC(4d71f29f) SHA1(3ccc8410d383bfd9fde44574ebb9c24a235cc734) )
	ROM_FILL(                        0x1000000, 0x800000, 0x00 )
	ROM_LOAD16_BYTE( "ss91fl3l.7",   0x1800000, 0x200000, CRC(61efd65b) SHA1(1e97d5cd51bf778995c3e568ac7e1d3514264d48) )
	ROM_LOAD16_BYTE( "ss91fl3h.8",   0x1800001, 0x200000, CRC(7f3c8c54) SHA1(b473a503cc5d532922780139ab3cab974d3df65b) )
	ROM_LOAD16_BYTE( "ss91fl4l.5",   0x1c00000, 0x200000, CRC(a6af9511) SHA1(63f14f4fc2b210348e41cdfa552a7a7c86fb1b99) )
	ROM_LOAD16_BYTE( "ss91fl4h.6",   0x1c00001, 0x200000, CRC(be3730a4) SHA1(48802229d31c1eef7f4173eb05060a328b702336) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "ss91vera.11s", 0x0000000, 0x080000, CRC(c6bc5c31) SHA1(c6ef46c3fa8a7749618126d360e614ea6c8d9c54) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "ss91wave0.2",         0x0000000, 0x800000, CRC(1c5e2ff1) SHA1(4f29dfd49f6b5c3ca3b758823d368051354bd673) )
	ROM_LOAD( "ss91wave1.1",         0x0800000, 0x800000, CRC(5f4c8861) SHA1(baee7182c32bc064d805de5a16948faf78941ac4) )
ROM_END

ROM_START( sws2000 )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "ss01vera.2l",  0x000000, 0x200000, CRC(6ddbdcaa) SHA1(cff31d75e7780851b2c2c025ee34fd8990e2f502) )
	ROM_LOAD16_BYTE( "ss01vera.2p",  0x000001, 0x200000, CRC(6ade7d28) SHA1(3e8d7bc9a284324c4de0ac265872b613d108cb57) )

	ROM_REGION32_LE( 0x2000000, "bankedroms", 0 ) /* main data */
	ROM_LOAD( "ss01rom0.9",   0x0000000, 0x800000, CRC(c08cc59c) SHA1(f2d8064491c98acbb40260d7f13d0f9b394c1383) )
	ROM_LOAD( "ss01rom1.10",  0x0800000, 0x800000, CRC(d4aa1dc6) SHA1(68fcb60aadf35668d7d746da1e29d5d985e40aec) )
	ROM_LOAD( "ss01rom2.11",  0x1000000, 0x800000, CRC(3371904a) SHA1(58bbf98a44560bc021bac4bc513fe89deea85b50) )
	ROM_LOAD16_BYTE( "ss01fl3l.7",   0x1800000, 0x200000, CRC(35bad813) SHA1(19ceffb78695c8c22b30659aad56ac237ffb56ac) )
	ROM_LOAD16_BYTE( "ss01fl3h.8",   0x1800001, 0x200000, CRC(a14828fc) SHA1(fd8a052451f74f420f4a4a2b902e0c2e83e3239b) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "ss01vera.11s", 0x000000, 0x080000, CRC(641e6584) SHA1(dfc0ac21bea5b19dbce5d50fb681854889d756dc) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "ss01wave0.2",  0x000000, 0x800000, CRC(1c5e2ff1) SHA1(4f29dfd49f6b5c3ca3b758823d368051354bd673) )
	ROM_LOAD( "ss01wave1.1",  0x800000, 0x800000, CRC(5f4c8861) SHA1(baee7182c32bc064d805de5a16948faf78941ac4) )
ROM_END

ROM_START( sws2001 )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "ss11vera.2l",  0x000000, 0x200000, CRC(a7b4dbe5) SHA1(1bcb8d127388e2ead9ca04b527779896c69daf7f) )
	ROM_LOAD16_BYTE( "ss11vera.2p",  0x000001, 0x200000, CRC(3ef76b4e) SHA1(34c21b6002d3f88aa3f4b4606c8aace24be92920) )

	ROM_REGION32_LE( 0x4000000, "bankedroms", 0 ) /* main data */
	ROM_LOAD( "ss01rom0.9",   0x0000000, 0x800000, CRC(c08cc59c) SHA1(f2d8064491c98acbb40260d7f13d0f9b394c1383) )
	ROM_LOAD( "ss01rom1.10",  0x1000000, 0x800000, CRC(d4aa1dc6) SHA1(68fcb60aadf35668d7d746da1e29d5d985e40aec) )
	ROM_LOAD( "ss01rom2.11",  0x2000000, 0x800000, CRC(3371904a) SHA1(58bbf98a44560bc021bac4bc513fe89deea85b50) )
	ROM_LOAD( "ss11fl1l.5",   0x3000000, 0x800000, CRC(0546a2b4) SHA1(b61457c9c5136f94ace48e83de8043ce20ca4d99) )
	ROM_LOAD( "ss11fl1u.6",   0x3800000, 0x800000, CRC(b3de0b2c) SHA1(a89d3a2eb0952a98cc4765af2897b94136ca58ad) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "ss01vera.11s", 0x000000, 0x080000, CRC(641e6584) SHA1(dfc0ac21bea5b19dbce5d50fb681854889d756dc) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "ss01wave0.2",  0x000000, 0x800000, CRC(1c5e2ff1) SHA1(4f29dfd49f6b5c3ca3b758823d368051354bd673) )
	ROM_LOAD( "ss01wave1.1",  0x800000, 0x800000, CRC(5f4c8861) SHA1(baee7182c32bc064d805de5a16948faf78941ac4) )
ROM_END

ROM_START( tekken3 )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "tet2vere1.2e",  0x0000000, 0x200000, CRC(7ded5461) SHA1(3a5638c6ad40bfde6e12fdfd6d469f6ea5e9f4fb) )
	ROM_LOAD16_BYTE( "tet2vere1.2j",  0x0000001, 0x200000, CRC(25c96e1e) SHA1(554d1e42886d6a6c9c5857e9cbd5d7c37d7a6e67) )

	ROM_REGION32_LE( 0x1c00000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "tet1rom0l.6",  0x0000000, 0x400000, CRC(2886bb32) SHA1(08ad9da2df25ad8c933a812ac238c81135072929) )
	ROM_LOAD16_BYTE( "tet1rom0u.9",  0x0000001, 0x400000, CRC(c5705b92) SHA1(20df20c8d18eb4712d565a3df9a8d9270dee6aaa) )
	ROM_LOAD16_BYTE( "tet1rom1l.7",  0x0800000, 0x400000, CRC(0397d283) SHA1(ebafcd14cdb2601214129a84fc6830846f5cd274) )
	ROM_LOAD16_BYTE( "tet1rom1u.10", 0x0800001, 0x400000, CRC(502ba5cd) SHA1(19c1282245c6dbfc945c0bd0f3918968c3e5c3ed) )
	ROM_LOAD16_BYTE( "tet1rom2l.8",  0x1000000, 0x400000, CRC(e03b1c24) SHA1(8579b95a8fd06b7d2893ff88b228fd794162dff1) )
	ROM_LOAD16_BYTE( "tet1rom2u.11", 0x1000001, 0x400000, CRC(75eb2ab3) SHA1(dee43884e542391903f6aaae2c166e7921a86fb4) )
	ROM_LOAD16_BYTE( "tet1fl3l.12",  0x1800000, 0x200000, CRC(45513073) SHA1(8a36f58ee2d292b50e00c6bf275f9def358032d8) )
	ROM_LOAD16_BYTE( "tet1fl3u.13",  0x1800001, 0x200000, CRC(1917d993) SHA1(cabc44514a3e62a18a7f8f883603241447d6948b) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "tet1verb.11s", 0x0000000, 0x080000, CRC(c92b98d1) SHA1(8ae6fba8c5b6b9a2ab9541eac8553b282f35750d) ) /* No label but different than tet1vera.11s */

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "tet1wave0.5",         0x0000000, 0x400000, CRC(77ba7975) SHA1(fe9434dcf0fb232c85efaaae1b4b13d36099620a) )
	ROM_LOAD( "tet1wave1.4",         0x0400000, 0x400000, CRC(ffeba79f) SHA1(941412bbe9d0305d9a23c224c1bb774c4321f6df) )
ROM_END

ROM_START( tekken3je1 )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "tet1vere1.2e",  0x0000000, 0x200000, CRC(8b01113b) SHA1(45fdfd58293641ed16bc59c633a85a9cf64ccbaf) )
	ROM_LOAD16_BYTE( "tet1vere1.2j",  0x0000001, 0x200000, CRC(df4c96fb) SHA1(2e223045bf5b80ccf615106e869760c5b7aa8d44) )

	ROM_REGION32_LE( 0x1c00000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "tet1rom0l.6",  0x0000000, 0x400000, CRC(2886bb32) SHA1(08ad9da2df25ad8c933a812ac238c81135072929) )
	ROM_LOAD16_BYTE( "tet1rom0u.9",  0x0000001, 0x400000, CRC(c5705b92) SHA1(20df20c8d18eb4712d565a3df9a8d9270dee6aaa) )
	ROM_LOAD16_BYTE( "tet1rom1l.7",  0x0800000, 0x400000, CRC(0397d283) SHA1(ebafcd14cdb2601214129a84fc6830846f5cd274) )
	ROM_LOAD16_BYTE( "tet1rom1u.10", 0x0800001, 0x400000, CRC(502ba5cd) SHA1(19c1282245c6dbfc945c0bd0f3918968c3e5c3ed) )
	ROM_LOAD16_BYTE( "tet1rom2l.8",  0x1000000, 0x400000, CRC(e03b1c24) SHA1(8579b95a8fd06b7d2893ff88b228fd794162dff1) )
	ROM_LOAD16_BYTE( "tet1rom2u.11", 0x1000001, 0x400000, CRC(75eb2ab3) SHA1(dee43884e542391903f6aaae2c166e7921a86fb4) )
	ROM_LOAD16_BYTE( "tet1fl3l.12",  0x1800000, 0x200000, CRC(45513073) SHA1(8a36f58ee2d292b50e00c6bf275f9def358032d8) )
	ROM_LOAD16_BYTE( "tet1fl3u.13",  0x1800001, 0x200000, CRC(1917d993) SHA1(cabc44514a3e62a18a7f8f883603241447d6948b) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "tet1verb.11s", 0x0000000, 0x080000, CRC(c92b98d1) SHA1(8ae6fba8c5b6b9a2ab9541eac8553b282f35750d) ) /* No label but different than tet1vera.11s */

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "tet1wave0.5",         0x0000000, 0x400000, CRC(77ba7975) SHA1(fe9434dcf0fb232c85efaaae1b4b13d36099620a) )
	ROM_LOAD( "tet1wave1.4",         0x0400000, 0x400000, CRC(ffeba79f) SHA1(941412bbe9d0305d9a23c224c1bb774c4321f6df) )

	// Namco Cyber Lead cabinet JVS I/O and LED display controller
	ROM_REGION(0x40000, "cabinet_io", 0)
	// JVS I/O board (namco ltd.;I/O CYBER LEAD;Ver1.03;JPN,LED-0100)
	// labels: CL1 I/OB, I/O LED (I/O) PCB
	// ICs: Namco C77 H8/???? MCU, Atmel AT29C020 256k x8 FlashROM, NEC N341256 32k x8 SRAM, 14.7MHz Xtal, Altera EPM7064 labeled I/OLEDM1
	ROM_LOAD("cl1-iob.ic5", 0x0000, 0x40000, CRC(abb90360) SHA1(d938b1e1ae596d0ab1007352f61b0b800363c762) )
	// LED display controller, connected to above I/O
	// labels: CL1 LEDA, I/O LED (LED) PCB
	// ICs: same as above plus EPSON SED1351F LCD controller, 12MHz Xtal
	ROM_LOAD("cl1-leda.ic5", 0x0000, 0x40000, CRC(43602a58) SHA1(64156ded8c43dbbe84b5d6ae13a068c8b18e8aed) )
ROM_END

ROM_START( tekken3d )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "tet2verd.2e",   0x0000000, 0x200000, CRC(ff269bcd) SHA1(f118b69ffe3ee1ad785c115c39d5166f3c546554) )
	ROM_LOAD16_BYTE( "tet2verd.2j",   0x0000001, 0x200000, CRC(46f9205c) SHA1(662b8f910e4ccc1a0e9f3fef0992a92abbebebd0) )

	ROM_REGION32_LE( 0x1c00000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "tet1rom0l.6",  0x0000000, 0x400000, CRC(2886bb32) SHA1(08ad9da2df25ad8c933a812ac238c81135072929) )
	ROM_LOAD16_BYTE( "tet1rom0u.9",  0x0000001, 0x400000, CRC(c5705b92) SHA1(20df20c8d18eb4712d565a3df9a8d9270dee6aaa) )
	ROM_LOAD16_BYTE( "tet1rom1l.7",  0x0800000, 0x400000, CRC(0397d283) SHA1(ebafcd14cdb2601214129a84fc6830846f5cd274) )
	ROM_LOAD16_BYTE( "tet1rom1u.10", 0x0800001, 0x400000, CRC(502ba5cd) SHA1(19c1282245c6dbfc945c0bd0f3918968c3e5c3ed) )
	ROM_LOAD16_BYTE( "tet1rom2l.8",  0x1000000, 0x400000, CRC(e03b1c24) SHA1(8579b95a8fd06b7d2893ff88b228fd794162dff1) )
	ROM_LOAD16_BYTE( "tet1rom2u.11", 0x1000001, 0x400000, CRC(75eb2ab3) SHA1(dee43884e542391903f6aaae2c166e7921a86fb4) )
	ROM_LOAD16_BYTE( "tet1fl3l.12",  0x1800000, 0x200000, CRC(45513073) SHA1(8a36f58ee2d292b50e00c6bf275f9def358032d8) )
	ROM_LOAD16_BYTE( "tet1fl3u.13",  0x1800001, 0x200000, CRC(1917d993) SHA1(cabc44514a3e62a18a7f8f883603241447d6948b) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "tet1verb.11s", 0x0000000, 0x080000, CRC(c92b98d1) SHA1(8ae6fba8c5b6b9a2ab9541eac8553b282f35750d) ) /* No label but different than tet1vera.11s */

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "tet1wave0.5",         0x0000000, 0x400000, CRC(77ba7975) SHA1(fe9434dcf0fb232c85efaaae1b4b13d36099620a) )
	ROM_LOAD( "tet1wave1.4",         0x0400000, 0x400000, CRC(ffeba79f) SHA1(941412bbe9d0305d9a23c224c1bb774c4321f6df) )
ROM_END

ROM_START( tekken3ud )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "tet3verd.2e",  0x0000000, 0x200000, CRC(9056a8d1) SHA1(08269de80361672f1a193e5cdcd0d4571b746a85) )
	ROM_LOAD16_BYTE( "tet3verd.2j",  0x0000001, 0x200000, CRC(60ae06f4) SHA1(898355cc6bae4745b6b9913e34d50fe2a00f1c2c) )

	ROM_REGION32_LE( 0x1c00000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "tet1rom0l.6",  0x0000000, 0x400000, CRC(2886bb32) SHA1(08ad9da2df25ad8c933a812ac238c81135072929) )
	ROM_LOAD16_BYTE( "tet1rom0u.9",  0x0000001, 0x400000, CRC(c5705b92) SHA1(20df20c8d18eb4712d565a3df9a8d9270dee6aaa) )
	ROM_LOAD16_BYTE( "tet1rom1l.7",  0x0800000, 0x400000, CRC(0397d283) SHA1(ebafcd14cdb2601214129a84fc6830846f5cd274) )
	ROM_LOAD16_BYTE( "tet1rom1u.10", 0x0800001, 0x400000, CRC(502ba5cd) SHA1(19c1282245c6dbfc945c0bd0f3918968c3e5c3ed) )
	ROM_LOAD16_BYTE( "tet1rom2l.8",  0x1000000, 0x400000, CRC(e03b1c24) SHA1(8579b95a8fd06b7d2893ff88b228fd794162dff1) )
	ROM_LOAD16_BYTE( "tet1rom2u.11", 0x1000001, 0x400000, CRC(75eb2ab3) SHA1(dee43884e542391903f6aaae2c166e7921a86fb4) )
	ROM_LOAD16_BYTE( "tet1fl3l.12",  0x1800000, 0x200000, CRC(45513073) SHA1(8a36f58ee2d292b50e00c6bf275f9def358032d8) )
	ROM_LOAD16_BYTE( "tet1fl3u.13",  0x1800001, 0x200000, CRC(1917d993) SHA1(cabc44514a3e62a18a7f8f883603241447d6948b) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "tet1verb.11s", 0x0000000, 0x080000, CRC(c92b98d1) SHA1(8ae6fba8c5b6b9a2ab9541eac8553b282f35750d) ) /* No label but different than tet1vera.11s */

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "tet1wave0.5",         0x0000000, 0x400000, CRC(77ba7975) SHA1(fe9434dcf0fb232c85efaaae1b4b13d36099620a) )
	ROM_LOAD( "tet1wave1.4",         0x0400000, 0x400000, CRC(ffeba79f) SHA1(941412bbe9d0305d9a23c224c1bb774c4321f6df) )
ROM_END

ROM_START( tekken3c )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "tet2verc.2e",  0x0000000, 0x200000, CRC(4483d76e) SHA1(c43b7e445acb77ddeb8a647bef1fc509ced0412e) )
	ROM_LOAD16_BYTE( "tet2verc.2j",  0x0000001, 0x200000, CRC(3b4fee42) SHA1(b71b2969b0416e6fd348297876fc55d44e14ebef) )

	ROM_REGION32_LE( 0x1c00000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "tet1rom0l.6",  0x0000000, 0x400000, CRC(2886bb32) SHA1(08ad9da2df25ad8c933a812ac238c81135072929) )
	ROM_LOAD16_BYTE( "tet1rom0u.9",  0x0000001, 0x400000, CRC(c5705b92) SHA1(20df20c8d18eb4712d565a3df9a8d9270dee6aaa) )
	ROM_LOAD16_BYTE( "tet1rom1l.7",  0x0800000, 0x400000, CRC(0397d283) SHA1(ebafcd14cdb2601214129a84fc6830846f5cd274) )
	ROM_LOAD16_BYTE( "tet1rom1u.10", 0x0800001, 0x400000, CRC(502ba5cd) SHA1(19c1282245c6dbfc945c0bd0f3918968c3e5c3ed) )
	ROM_LOAD16_BYTE( "tet1rom2l.8",  0x1000000, 0x400000, CRC(e03b1c24) SHA1(8579b95a8fd06b7d2893ff88b228fd794162dff1) )
	ROM_LOAD16_BYTE( "tet1rom2u.11", 0x1000001, 0x400000, CRC(75eb2ab3) SHA1(dee43884e542391903f6aaae2c166e7921a86fb4) )
	ROM_LOAD16_BYTE( "tet1fl3l.12",  0x1800000, 0x200000, CRC(45513073) SHA1(8a36f58ee2d292b50e00c6bf275f9def358032d8) )
	ROM_LOAD16_BYTE( "tet1fl3u.13",  0x1800001, 0x200000, CRC(1917d993) SHA1(cabc44514a3e62a18a7f8f883603241447d6948b) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "tet1verb.11s", 0x0000000, 0x080000, CRC(c92b98d1) SHA1(8ae6fba8c5b6b9a2ab9541eac8553b282f35750d) ) /* No label but different than tet1vera.11s */

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "tet1wave0.5",         0x0000000, 0x400000, CRC(77ba7975) SHA1(fe9434dcf0fb232c85efaaae1b4b13d36099620a) )
	ROM_LOAD( "tet1wave1.4",         0x0400000, 0x400000, CRC(ffeba79f) SHA1(941412bbe9d0305d9a23c224c1bb774c4321f6df) )
ROM_END

ROM_START( tekken3b )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "tet2verb.2e",  0x0000000, 0x200000, CRC(a6cbc434) SHA1(859d84e6e9a52c2cdd54a2a0bb8104169eb19c07) )
	ROM_LOAD16_BYTE( "tet2verb.2j",  0x0000001, 0x200000, CRC(c8f95ec5) SHA1(7f34c42e1fbc35118e8476cdb78fbdb9564001de) )

	ROM_REGION32_LE( 0x1c00000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "tet1rom0l.6",  0x0000000, 0x400000, CRC(2886bb32) SHA1(08ad9da2df25ad8c933a812ac238c81135072929) )
	ROM_LOAD16_BYTE( "tet1rom0u.9",  0x0000001, 0x400000, CRC(c5705b92) SHA1(20df20c8d18eb4712d565a3df9a8d9270dee6aaa) )
	ROM_LOAD16_BYTE( "tet1rom1l.7",  0x0800000, 0x400000, CRC(0397d283) SHA1(ebafcd14cdb2601214129a84fc6830846f5cd274) )
	ROM_LOAD16_BYTE( "tet1rom1u.10", 0x0800001, 0x400000, CRC(502ba5cd) SHA1(19c1282245c6dbfc945c0bd0f3918968c3e5c3ed) )
	ROM_LOAD16_BYTE( "tet1rom2l.8",  0x1000000, 0x400000, CRC(e03b1c24) SHA1(8579b95a8fd06b7d2893ff88b228fd794162dff1) )
	ROM_LOAD16_BYTE( "tet1rom2u.11", 0x1000001, 0x400000, CRC(75eb2ab3) SHA1(dee43884e542391903f6aaae2c166e7921a86fb4) )
	ROM_LOAD16_BYTE( "tet1fl3l.12",  0x1800000, 0x200000, CRC(45513073) SHA1(8a36f58ee2d292b50e00c6bf275f9def358032d8) )
	ROM_LOAD16_BYTE( "tet1fl3u.13",  0x1800001, 0x200000, CRC(1917d993) SHA1(cabc44514a3e62a18a7f8f883603241447d6948b) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "tet1verb.11s", 0x0000000, 0x080000, CRC(c92b98d1) SHA1(8ae6fba8c5b6b9a2ab9541eac8553b282f35750d) ) /* No label but different than tet1vera.11s */

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "tet1wave0.5",         0x0000000, 0x400000, CRC(77ba7975) SHA1(fe9434dcf0fb232c85efaaae1b4b13d36099620a) )
	ROM_LOAD( "tet1wave1.4",         0x0400000, 0x400000, CRC(ffeba79f) SHA1(941412bbe9d0305d9a23c224c1bb774c4321f6df) )
ROM_END

ROM_START( tekken3a )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "tet2vera.2e",  0x0000000, 0x200000, CRC(7270f157) SHA1(e73c5970e58f9e8c5696f4e3b15908fbec6c21ce) )
	ROM_LOAD16_BYTE( "tet2vera.2j",  0x0000001, 0x200000, CRC(94ceb446) SHA1(c730eb5c770991ae3ae0b9ba63681ce037e46746) )

	ROM_REGION32_LE( 0x1c00000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "tet1rom0l.6",     0x0000000, 0x400000, CRC(2886bb32) SHA1(08ad9da2df25ad8c933a812ac238c81135072929) )
	ROM_LOAD16_BYTE( "tet1rom0u.9",     0x0000001, 0x400000, CRC(c5705b92) SHA1(20df20c8d18eb4712d565a3df9a8d9270dee6aaa) )
	ROM_LOAD16_BYTE( "tet1rom1l.7",     0x0800000, 0x400000, CRC(0397d283) SHA1(ebafcd14cdb2601214129a84fc6830846f5cd274) )
	ROM_LOAD16_BYTE( "tet1rom1u.10",    0x0800001, 0x400000, CRC(502ba5cd) SHA1(19c1282245c6dbfc945c0bd0f3918968c3e5c3ed) )
	ROM_LOAD16_BYTE( "tet1rom2l.8",     0x1000000, 0x400000, CRC(e03b1c24) SHA1(8579b95a8fd06b7d2893ff88b228fd794162dff1) )
	ROM_LOAD16_BYTE( "tet1rom2u.11",    0x1000001, 0x400000, CRC(75eb2ab3) SHA1(dee43884e542391903f6aaae2c166e7921a86fb4) )
	ROM_LOAD16_BYTE( "tet1verafl3l.12", 0x1800000, 0x200000, CRC(64fa1f83) SHA1(5bc8b4657826ead19827f63d04f67597970d5337) ) /* Specific to Ver A. */
	ROM_LOAD16_BYTE( "tet1verafl3u.13", 0x1800001, 0x200000, CRC(32a2516b) SHA1(6f4e4fc1b11d17a867d3e7bdfdd351438390a5a2) ) /* Specific to Ver A. */

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "tet1vera.11s", 0x0000000, 0x080000, CRC(a74dfe7f) SHA1(854096a6f12ee9073fb1f38e41c8e6e0725a7521) ) /* Specific to Ver A. */

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "tet1wave0.5",         0x0000000, 0x400000, CRC(77ba7975) SHA1(fe9434dcf0fb232c85efaaae1b4b13d36099620a) )
	ROM_LOAD( "tet1wave1.4",         0x0400000, 0x400000, CRC(ffeba79f) SHA1(941412bbe9d0305d9a23c224c1bb774c4321f6df) )
ROM_END

ROM_START( tekken3ua )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "tet3vera.2e",  0x0000000, 0x200000, CRC(2fb6fac2) SHA1(518f74f09fa879cc1507c6afa2dd922b38cecd55) )
	ROM_LOAD16_BYTE( "tet3vera.2j",  0x0000001, 0x200000, CRC(968af792) SHA1(6187128d5ca07fd394f674b8dda0c190e6cd7f9d) )

	ROM_REGION32_LE( 0x1c00000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "tet1rom0l.6",     0x0000000, 0x400000, CRC(2886bb32) SHA1(08ad9da2df25ad8c933a812ac238c81135072929) )
	ROM_LOAD16_BYTE( "tet1rom0u.9",     0x0000001, 0x400000, CRC(c5705b92) SHA1(20df20c8d18eb4712d565a3df9a8d9270dee6aaa) )
	ROM_LOAD16_BYTE( "tet1rom1l.7",     0x0800000, 0x400000, CRC(0397d283) SHA1(ebafcd14cdb2601214129a84fc6830846f5cd274) )
	ROM_LOAD16_BYTE( "tet1rom1u.10",    0x0800001, 0x400000, CRC(502ba5cd) SHA1(19c1282245c6dbfc945c0bd0f3918968c3e5c3ed) )
	ROM_LOAD16_BYTE( "tet1rom2l.8",     0x1000000, 0x400000, CRC(e03b1c24) SHA1(8579b95a8fd06b7d2893ff88b228fd794162dff1) )
	ROM_LOAD16_BYTE( "tet1rom2u.11",    0x1000001, 0x400000, CRC(75eb2ab3) SHA1(dee43884e542391903f6aaae2c166e7921a86fb4) )
	ROM_LOAD16_BYTE( "tet1verafl3l.12", 0x1800000, 0x200000, CRC(64fa1f83) SHA1(5bc8b4657826ead19827f63d04f67597970d5337) ) /* Specific to Ver A. */
	ROM_LOAD16_BYTE( "tet1verafl3u.13", 0x1800001, 0x200000, CRC(32a2516b) SHA1(6f4e4fc1b11d17a867d3e7bdfdd351438390a5a2) ) /* Specific to Ver A. */

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "tet1vera.11s", 0x0000000, 0x080000, CRC(a74dfe7f) SHA1(854096a6f12ee9073fb1f38e41c8e6e0725a7521) ) /* Specific to Ver A. */

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "tet1wave0.5",         0x0000000, 0x400000, CRC(77ba7975) SHA1(fe9434dcf0fb232c85efaaae1b4b13d36099620a) )
	ROM_LOAD( "tet1wave1.4",         0x0400000, 0x400000, CRC(ffeba79f) SHA1(941412bbe9d0305d9a23c224c1bb774c4321f6df) )
ROM_END

ROM_START( tekken3ja )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "tet1vera.2e",  0x0000000, 0x200000, CRC(98fe53b4) SHA1(0d3380f368908a21cb1e4cea353687d3f6295d79) )
	ROM_LOAD16_BYTE( "tet1vera.2j",  0x0000001, 0x200000, CRC(4dc6bb4a) SHA1(1a0ae22410fb6f7757d21fdaf713893ca4e177fe) )

	ROM_REGION32_LE( 0x1c00000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "tet1rom0l.6",     0x0000000, 0x400000, CRC(2886bb32) SHA1(08ad9da2df25ad8c933a812ac238c81135072929) )
	ROM_LOAD16_BYTE( "tet1rom0u.9",     0x0000001, 0x400000, CRC(c5705b92) SHA1(20df20c8d18eb4712d565a3df9a8d9270dee6aaa) )
	ROM_LOAD16_BYTE( "tet1rom1l.7",     0x0800000, 0x400000, CRC(0397d283) SHA1(ebafcd14cdb2601214129a84fc6830846f5cd274) )
	ROM_LOAD16_BYTE( "tet1rom1u.10",    0x0800001, 0x400000, CRC(502ba5cd) SHA1(19c1282245c6dbfc945c0bd0f3918968c3e5c3ed) )
	ROM_LOAD16_BYTE( "tet1rom2l.8",     0x1000000, 0x400000, CRC(e03b1c24) SHA1(8579b95a8fd06b7d2893ff88b228fd794162dff1) )
	ROM_LOAD16_BYTE( "tet1rom2u.11",    0x1000001, 0x400000, CRC(75eb2ab3) SHA1(dee43884e542391903f6aaae2c166e7921a86fb4) )
	ROM_LOAD16_BYTE( "tet1verafl3l.12", 0x1800000, 0x200000, CRC(64fa1f83) SHA1(5bc8b4657826ead19827f63d04f67597970d5337) ) /* Specific to Ver A. */
	ROM_LOAD16_BYTE( "tet1verafl3u.13", 0x1800001, 0x200000, CRC(32a2516b) SHA1(6f4e4fc1b11d17a867d3e7bdfdd351438390a5a2) ) /* Specific to Ver A. */

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "tet1vera.11s", 0x0000000, 0x080000, CRC(a74dfe7f) SHA1(854096a6f12ee9073fb1f38e41c8e6e0725a7521) ) /* Specific to Ver A. */

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "tet1wave0.5",         0x0000000, 0x400000, CRC(77ba7975) SHA1(fe9434dcf0fb232c85efaaae1b4b13d36099620a) )
	ROM_LOAD( "tet1wave1.4",         0x0400000, 0x400000, CRC(ffeba79f) SHA1(941412bbe9d0305d9a23c224c1bb774c4321f6df) )
ROM_END

ROM_START( tektagt )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "teg2verc1.2e",  0x0000000, 0x200000, CRC(c6da0717) SHA1(9e01ae64710d85eb9899d6fa6fd0a2152aee8c11) ) /* Modified to work with alt ROM board? */
	ROM_LOAD16_BYTE( "teg2verc1.2j",  0x0000001, 0x200000, CRC(25a1d2ff) SHA1(529a11a1bbb8655534d7ec371f1c09e9e387ed11) )

	ROM_REGION32_LE( 0x3800000, "bankedroms", 0 ) /* main data */
	ROM_LOAD32_WORD( "teg1_rom0e.ic9",  0x0000000, 0x800000, CRC(c962a373) SHA1(d662dbd89ef62c5ac3150a018fc2d35ef2ee94ac) )
	ROM_LOAD32_WORD( "teg1_rom0o.ic13", 0x0000002, 0x800000, CRC(badb7dcf) SHA1(8c0bf7f6351c5a2a0996df371a901cf90c68cd8c) )
	ROM_LOAD32_WORD( "teg1_rom1e.ic10", 0x1000000, 0x800000, CRC(b3d56124) SHA1(4df20c74ba63f7362caf15e9b8949fab655704fb) )
	ROM_LOAD32_WORD( "teg1_rom1o.ic14", 0x1000002, 0x800000, CRC(2434ceb6) SHA1(f19f1599acbd6fd48793a2ee5a500ca817d9df56) )
	ROM_LOAD32_WORD( "teg1_rom2e.ic11", 0x2000000, 0x800000, CRC(6e5c3428) SHA1(e3cdb60a4445406877b2e273385f34bfb0974220) )
	ROM_LOAD32_WORD( "teg1_rom2o.ic15", 0x2000002, 0x800000, CRC(21ce9dfa) SHA1(f27e8210ee236c327aa3e1ce4dd408abc6580a1b) )

	ROM_LOAD32_BYTE( "flel.ic4", 0x3000000, 0x200000, CRC(88b3823c) SHA1(6f31acb642c57daccbfdb87b790037e261c8c73c) ) /* Flash ROMs with no labels */
	ROM_LOAD32_BYTE( "fleu.ic5", 0x3000001, 0x200000, CRC(36df0867) SHA1(6bec8560ad4c122dc909daa83aa9089ba5b281f7) )
	ROM_LOAD32_BYTE( "flol.ic6", 0x3000002, 0x200000, CRC(03a76765) SHA1(ae35ae28375f2a3e52d72b77ec09750c326cc269) )
	ROM_LOAD32_BYTE( "flou.ic7", 0x3000003, 0x200000, CRC(6d6947d1) SHA1(2f307bc4070fadb510c0473bc91d917b2d845ca5) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "teg1.11s", 0x0000000, 0x080000, CRC(67d0c469) SHA1(da164702fc21b9f46a9e32c89e7b1d36070ddf79) ) /* No label for this one */

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "teg1_wave0.ic1",  0x0000000, 0x800000, CRC(4bd99104) SHA1(f76b0576cc28fe49d3c1c402988b933933e52e15) )
	ROM_LOAD( "teg1_wave1.ic12", 0x0800000, 0x800000, CRC(dbc74fff) SHA1(601b7e7361ea744b34e3fa1fc39d88641de7f4c6) )
ROM_END

ROM_START( tektagtc1 )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "teg2ver_c1.2e", 0x0000000, 0x200000, CRC(c0800960) SHA1(80fc8910ebb2399b3be3c9ea87cc1d9283b42676) )
	ROM_LOAD16_BYTE( "teg2ver_c1.2j", 0x0000001, 0x200000, CRC(c0476713) SHA1(e51e4f3cd20ad6838fb05aaede0ab288e145e7a2) )

	ROM_REGION32_LE( 0x3800000, "bankedroms", 0 ) /* main data */
	ROM_LOAD32_WORD( "teg2roml0",       0x0000000, 0x400000, CRC(cf984e85) SHA1(4d8296998b5f225b3f20bfdcb092a64b6e1fa76e) )
	ROM_LOAD32_WORD( "teg2roml00.ic13", 0x0000002, 0x400000, CRC(927723a5) SHA1(075aaca1700c9134496eb9a73cbe325ddc74eae1) ) /* located on the opposite side of the ROM PCB */
	ROM_LOAD32_WORD( "teg2romh0",       0x0800000, 0x400000, CRC(ea088657) SHA1(fbacca78832f68a310bafd06188d645e3424e0ed) )
	ROM_LOAD32_WORD( "teg2romh00",      0x0800002, 0x400000, CRC(a85aa306) SHA1(07318133c7c48cb61590ed0eceb519a8cc1ca781) )
	ROM_LOAD32_WORD( "teg2roml1",       0x1000000, 0x400000, CRC(8552f0ef) SHA1(45ed0dacdad94e708260a98b9cd47861543d2b7a) )
	ROM_LOAD32_WORD( "teg2roml10.ic14", 0x1000002, 0x400000, CRC(13eb424b) SHA1(59d6c77136050de1e7250e6627636d5b178bb8a8) ) /* located on the opposite side of the ROM PCB */
	ROM_LOAD32_WORD( "teg2romh1",       0x1800000, 0x400000, CRC(86eb5abe) SHA1(66a3807a18d81e8291baf56533694f60e31a6367) )
	ROM_LOAD32_WORD( "teg2romh10",      0x1800002, 0x400000, CRC(25131a87) SHA1(77fa236c4879a59323bc2dac26c195b7c47f812c) )
	ROM_LOAD32_WORD( "teg2roml2",       0x2000000, 0x400000, CRC(abdfc6ca) SHA1(70033becd29ea15611be2fc099a43712181bcbec) )
	ROM_LOAD32_WORD( "teg2roml20.ic15", 0x2000002, 0x400000, CRC(ed5ec7f7) SHA1(c91085a64ced95ee6618303987de61893e069742) ) /* located on the opposite side of the ROM PCB */
	ROM_LOAD32_WORD( "teg2romh2",       0x2800000, 0x400000, CRC(8c03b301) SHA1(f395a4685e2deb8f7e9afc18ce15fa0d392b0cfa) )
	ROM_LOAD32_WORD( "teg2romh20",      0x2800002, 0x400000, CRC(c873c362) SHA1(d95451a7477b716c0d75ba04e1d8df3d8a4e10dd) )
	ROM_LOAD32_WORD( "teg2rf4",         0x3000000, 0x400000, CRC(612f6a37) SHA1(bd9597ef802e3c46f891f0ef523a3e0b0fe22672) ) /* Different data?? Second half blank, all 0x00? */
	ROM_LOAD32_WORD( "teg2rf6",         0x3000002, 0x400000, CRC(0c9292ce) SHA1(664c6ccf18042ed8ebc29b1781ae7454acb05ba1) ) /* Different data?? Second half blank, all 0x00? */

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "teg1.11s", 0x0000000, 0x080000, CRC(67d0c469) SHA1(da164702fc21b9f46a9e32c89e7b1d36070ddf79) ) /* No label for this one */

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "teg2wavel0.ic12",     0x0000000, 0x400000, CRC(1865336f) SHA1(deda6d129daf709b0316f6260e67613fbf849e57) ) /* located on the opposite side of the ROM PCB */
	ROM_LOAD( "teg2wavel1",          0x0400000, 0x400000, CRC(f5ab70e6) SHA1(cdb62d8eb92ca961be684caf768d1961d98ece22) )
	ROM_LOAD( "teg2waveh0",          0x0800000, 0x400000, CRC(bc0325bf) SHA1(77059c300a9d53d0e97bb898a06c58076078d7d2) )
	ROM_LOAD( "teg2waveh1",          0x0c00000, 0x400000, CRC(6c654921) SHA1(3849157b30b4b459956b823511551abda2e4b6e5) )
ROM_END

ROM_START( tektagtuc1 )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "teg3verc1.2l",  0x0000000, 0x200000, CRC(1efb7b85) SHA1(0623bb6571caf046ff7b4f83f11ee84a92c4b462) )
	ROM_LOAD16_BYTE( "teg3verc1.2p",  0x0000001, 0x200000, CRC(7caef9b2) SHA1(5c56d69ba2f723d0a4fbe4902196efc6ba9d5094) )

	ROM_REGION32_LE( 0x3800000, "bankedroms", 0 ) /* main data */
	ROM_LOAD32_WORD( "teg1_rom0e.ic9",  0x0000000, 0x800000, CRC(c962a373) SHA1(d662dbd89ef62c5ac3150a018fc2d35ef2ee94ac) )
	ROM_LOAD32_WORD( "teg1_rom0o.ic13", 0x0000002, 0x800000, CRC(badb7dcf) SHA1(8c0bf7f6351c5a2a0996df371a901cf90c68cd8c) )
	ROM_LOAD32_WORD( "teg1_rom1e.ic10", 0x1000000, 0x800000, CRC(b3d56124) SHA1(4df20c74ba63f7362caf15e9b8949fab655704fb) )
	ROM_LOAD32_WORD( "teg1_rom1o.ic14", 0x1000002, 0x800000, CRC(2434ceb6) SHA1(f19f1599acbd6fd48793a2ee5a500ca817d9df56) )
	ROM_LOAD32_WORD( "teg1_rom2e.ic11", 0x2000000, 0x800000, CRC(6e5c3428) SHA1(e3cdb60a4445406877b2e273385f34bfb0974220) )
	ROM_LOAD32_WORD( "teg1_rom2o.ic15", 0x2000002, 0x800000, CRC(21ce9dfa) SHA1(f27e8210ee236c327aa3e1ce4dd408abc6580a1b) )

	ROM_LOAD32_BYTE( "flel.ic4", 0x3000000, 0x200000, CRC(88b3823c) SHA1(6f31acb642c57daccbfdb87b790037e261c8c73c) ) /* Flash ROMs with no labels */
	ROM_LOAD32_BYTE( "fleu.ic5", 0x3000001, 0x200000, CRC(36df0867) SHA1(6bec8560ad4c122dc909daa83aa9089ba5b281f7) )
	ROM_LOAD32_BYTE( "flol.ic6", 0x3000002, 0x200000, CRC(03a76765) SHA1(ae35ae28375f2a3e52d72b77ec09750c326cc269) )
	ROM_LOAD32_BYTE( "flou.ic7", 0x3000003, 0x200000, CRC(6d6947d1) SHA1(2f307bc4070fadb510c0473bc91d917b2d845ca5) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "teg1.11s", 0x0000000, 0x080000, CRC(67d0c469) SHA1(da164702fc21b9f46a9e32c89e7b1d36070ddf79) ) /* No label for this one */

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "teg1_wave0.ic1",  0x0000000, 0x800000, CRC(4bd99104) SHA1(f76b0576cc28fe49d3c1c402988b933933e52e15) )
	ROM_LOAD( "teg1_wave1.ic12", 0x0800000, 0x800000, CRC(dbc74fff) SHA1(601b7e7361ea744b34e3fa1fc39d88641de7f4c6) )
ROM_END

ROM_START( tektagtub )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "teg3verb.2l",  0x0000000, 0x200000, CRC(97df2855) SHA1(c1b61df8e79348424f4bd2660ab5179ef21bdb07) )
	ROM_LOAD16_BYTE( "teg3verb.2p",  0x0000001, 0x200000, CRC(1dbe7591) SHA1(af464caa03fdd12024ad482e9c853a36510bfba7) )

	ROM_REGION32_LE( 0x3800000, "bankedroms", 0 ) /* main data */
	ROM_LOAD32_WORD( "teg1_rom0e.ic9",  0x0000000, 0x800000, CRC(c962a373) SHA1(d662dbd89ef62c5ac3150a018fc2d35ef2ee94ac) )
	ROM_LOAD32_WORD( "teg1_rom0o.ic13", 0x0000002, 0x800000, CRC(badb7dcf) SHA1(8c0bf7f6351c5a2a0996df371a901cf90c68cd8c) )
	ROM_LOAD32_WORD( "teg1_rom1e.ic10", 0x1000000, 0x800000, CRC(b3d56124) SHA1(4df20c74ba63f7362caf15e9b8949fab655704fb) )
	ROM_LOAD32_WORD( "teg1_rom1o.ic14", 0x1000002, 0x800000, CRC(2434ceb6) SHA1(f19f1599acbd6fd48793a2ee5a500ca817d9df56) )
	ROM_LOAD32_WORD( "teg1_rom2e.ic11", 0x2000000, 0x800000, CRC(6e5c3428) SHA1(e3cdb60a4445406877b2e273385f34bfb0974220) )
	ROM_LOAD32_WORD( "teg1_rom2o.ic15", 0x2000002, 0x800000, CRC(21ce9dfa) SHA1(f27e8210ee236c327aa3e1ce4dd408abc6580a1b) )

	ROM_LOAD32_BYTE( "flel.ic4", 0x3000000, 0x200000, CRC(88b3823c) SHA1(6f31acb642c57daccbfdb87b790037e261c8c73c) ) /* Flash ROMs with no labels */
	ROM_LOAD32_BYTE( "fleu.ic5", 0x3000001, 0x200000, CRC(36df0867) SHA1(6bec8560ad4c122dc909daa83aa9089ba5b281f7) )
	ROM_LOAD32_BYTE( "flol.ic6", 0x3000002, 0x200000, CRC(03a76765) SHA1(ae35ae28375f2a3e52d72b77ec09750c326cc269) )
	ROM_LOAD32_BYTE( "flou.ic7", 0x3000003, 0x200000, CRC(6d6947d1) SHA1(2f307bc4070fadb510c0473bc91d917b2d845ca5) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "teg1.11s", 0x0000000, 0x080000, CRC(67d0c469) SHA1(da164702fc21b9f46a9e32c89e7b1d36070ddf79) ) /* No label for this one */

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "teg1_wave0.ic1",  0x0000000, 0x800000, CRC(4bd99104) SHA1(f76b0576cc28fe49d3c1c402988b933933e52e15) )
	ROM_LOAD( "teg1_wave1.ic12", 0x0800000, 0x800000, CRC(dbc74fff) SHA1(601b7e7361ea744b34e3fa1fc39d88641de7f4c6) )
ROM_END

ROM_START( tektagtjc1 )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "teg1verc1.2e", 0x0000000, 0x200000, CRC(adbdfc2e) SHA1(e18458efb0f9b341970132b512d2e5104a942c6e) )
	ROM_LOAD16_BYTE( "teg1verc1.2j", 0x0000001, 0x200000, CRC(2fa33418) SHA1(e15f4c8cc4309ebb90d8d015b1f17f75ce5c8879) )

	ROM_REGION32_LE( 0x3800000, "bankedroms", 0 ) /* main data */
	ROM_LOAD32_WORD( "teg1_rom0e.ic9",  0x0000000, 0x800000, CRC(c962a373) SHA1(d662dbd89ef62c5ac3150a018fc2d35ef2ee94ac) )
	ROM_LOAD32_WORD( "teg1_rom0o.ic13", 0x0000002, 0x800000, CRC(badb7dcf) SHA1(8c0bf7f6351c5a2a0996df371a901cf90c68cd8c) )
	ROM_LOAD32_WORD( "teg1_rom1e.ic10", 0x1000000, 0x800000, CRC(b3d56124) SHA1(4df20c74ba63f7362caf15e9b8949fab655704fb) )
	ROM_LOAD32_WORD( "teg1_rom1o.ic14", 0x1000002, 0x800000, CRC(2434ceb6) SHA1(f19f1599acbd6fd48793a2ee5a500ca817d9df56) )
	ROM_LOAD32_WORD( "teg1_rom2e.ic11", 0x2000000, 0x800000, CRC(6e5c3428) SHA1(e3cdb60a4445406877b2e273385f34bfb0974220) )
	ROM_LOAD32_WORD( "teg1_rom2o.ic15", 0x2000002, 0x800000, CRC(21ce9dfa) SHA1(f27e8210ee236c327aa3e1ce4dd408abc6580a1b) )

	ROM_LOAD32_BYTE( "flel.ic4", 0x3000000, 0x200000, CRC(88b3823c) SHA1(6f31acb642c57daccbfdb87b790037e261c8c73c) ) /* Flash ROMs with no labels */
	ROM_LOAD32_BYTE( "fleu.ic5", 0x3000001, 0x200000, CRC(36df0867) SHA1(6bec8560ad4c122dc909daa83aa9089ba5b281f7) )
	ROM_LOAD32_BYTE( "flol.ic6", 0x3000002, 0x200000, CRC(03a76765) SHA1(ae35ae28375f2a3e52d72b77ec09750c326cc269) )
	ROM_LOAD32_BYTE( "flou.ic7", 0x3000003, 0x200000, CRC(6d6947d1) SHA1(2f307bc4070fadb510c0473bc91d917b2d845ca5) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "teg1.11s", 0x0000000, 0x080000, CRC(67d0c469) SHA1(da164702fc21b9f46a9e32c89e7b1d36070ddf79) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "teg1_wave0.ic1",  0x0000000, 0x800000, CRC(4bd99104) SHA1(f76b0576cc28fe49d3c1c402988b933933e52e15) )
	ROM_LOAD( "teg1_wave1.ic12", 0x0800000, 0x800000, CRC(dbc74fff) SHA1(601b7e7361ea744b34e3fa1fc39d88641de7f4c6) )
ROM_END

ROM_START( tektagtjb )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "teg1verb.2e",  0x0000000, 0x200000, BAD_DUMP CRC(ca6c305f) SHA1(264a85566b74f544fe63a01332d92c65d23b6608) )
	ROM_LOAD16_BYTE( "teg1verb.2j",  0x0000001, 0x200000, BAD_DUMP CRC(5413e2ed) SHA1(d453f7932654d8258c67eb7fe3639d71db7e414c) )

	ROM_REGION32_LE( 0x3800000, "bankedroms", 0 ) /* main data */
	ROM_LOAD32_WORD( "teg1_rom0e.ic9",  0x0000000, 0x800000, CRC(c962a373) SHA1(d662dbd89ef62c5ac3150a018fc2d35ef2ee94ac) )
	ROM_LOAD32_WORD( "teg1_rom0o.ic13", 0x0000002, 0x800000, CRC(badb7dcf) SHA1(8c0bf7f6351c5a2a0996df371a901cf90c68cd8c) )
	ROM_LOAD32_WORD( "teg1_rom1e.ic10", 0x1000000, 0x800000, CRC(b3d56124) SHA1(4df20c74ba63f7362caf15e9b8949fab655704fb) )
	ROM_LOAD32_WORD( "teg1_rom1o.ic14", 0x1000002, 0x800000, CRC(2434ceb6) SHA1(f19f1599acbd6fd48793a2ee5a500ca817d9df56) )
	ROM_LOAD32_WORD( "teg1_rom2e.ic11", 0x2000000, 0x800000, CRC(6e5c3428) SHA1(e3cdb60a4445406877b2e273385f34bfb0974220) )
	ROM_LOAD32_WORD( "teg1_rom2o.ic15", 0x2000002, 0x800000, CRC(21ce9dfa) SHA1(f27e8210ee236c327aa3e1ce4dd408abc6580a1b) )

	ROM_LOAD32_BYTE( "flel.ic4", 0x3000000, 0x200000, CRC(88b3823c) SHA1(6f31acb642c57daccbfdb87b790037e261c8c73c) ) /* Flash ROMs with no labels */
	ROM_LOAD32_BYTE( "fleu.ic5", 0x3000001, 0x200000, CRC(36df0867) SHA1(6bec8560ad4c122dc909daa83aa9089ba5b281f7) )
	ROM_LOAD32_BYTE( "flol.ic6", 0x3000002, 0x200000, CRC(03a76765) SHA1(ae35ae28375f2a3e52d72b77ec09750c326cc269) )
	ROM_LOAD32_BYTE( "flou.ic7", 0x3000003, 0x200000, CRC(6d6947d1) SHA1(2f307bc4070fadb510c0473bc91d917b2d845ca5) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "teg1.11s", 0x0000000, 0x080000, CRC(67d0c469) SHA1(da164702fc21b9f46a9e32c89e7b1d36070ddf79) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "teg1_wave0.ic1",  0x0000000, 0x800000, CRC(4bd99104) SHA1(f76b0576cc28fe49d3c1c402988b933933e52e15) )
	ROM_LOAD( "teg1_wave1.ic12", 0x0800000, 0x800000, CRC(dbc74fff) SHA1(601b7e7361ea744b34e3fa1fc39d88641de7f4c6) )
ROM_END

ROM_START( tektagtja )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "teg1vera.2e",  0x0000000, 0x200000, BAD_DUMP CRC(17c4bf36) SHA1(abf2dfb3e35344cf4449ade6e63b36c590d9c131) )
	ROM_LOAD16_BYTE( "teg1vera.2j",  0x0000001, 0x200000, BAD_DUMP CRC(97cd9524) SHA1(8031cb465db378a6d9db9b132cf1169b94cba7dc) )

	ROM_REGION32_LE( 0x3800000, "bankedroms", 0 ) /* main data */
	ROM_LOAD32_WORD( "teg1_rom0e.ic9",  0x0000000, 0x800000, CRC(c962a373) SHA1(d662dbd89ef62c5ac3150a018fc2d35ef2ee94ac) )
	ROM_LOAD32_WORD( "teg1_rom0o.ic13", 0x0000002, 0x800000, CRC(badb7dcf) SHA1(8c0bf7f6351c5a2a0996df371a901cf90c68cd8c) )
	ROM_LOAD32_WORD( "teg1_rom1e.ic10", 0x1000000, 0x800000, CRC(b3d56124) SHA1(4df20c74ba63f7362caf15e9b8949fab655704fb) )
	ROM_LOAD32_WORD( "teg1_rom1o.ic14", 0x1000002, 0x800000, CRC(2434ceb6) SHA1(f19f1599acbd6fd48793a2ee5a500ca817d9df56) )
	ROM_LOAD32_WORD( "teg1_rom2e.ic11", 0x2000000, 0x800000, CRC(6e5c3428) SHA1(e3cdb60a4445406877b2e273385f34bfb0974220) )
	ROM_LOAD32_WORD( "teg1_rom2o.ic15", 0x2000002, 0x800000, CRC(21ce9dfa) SHA1(f27e8210ee236c327aa3e1ce4dd408abc6580a1b) )

	ROM_LOAD32_BYTE( "teg1flel.4",   0x3000000, 0x200000, NO_DUMP ) /* It's NOT been verified these should be the same, like the Tekken 3 Ver A  */
	ROM_LOAD32_BYTE( "teg1fleu.5",   0x3000001, 0x200000, NO_DUMP ) /* sets above, these Ver A set(s) might require a specific set of flash ROMs */
	ROM_LOAD32_BYTE( "teg1flol.6",   0x3000002, 0x200000, NO_DUMP )
	ROM_LOAD32_BYTE( "teg1flou.7",   0x3000003, 0x200000, NO_DUMP )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "teg1.11s", 0x0000000, 0x080000, CRC(67d0c469) SHA1(da164702fc21b9f46a9e32c89e7b1d36070ddf79) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "teg1_wave0.ic1",  0x0000000, 0x800000, CRC(4bd99104) SHA1(f76b0576cc28fe49d3c1c402988b933933e52e15) )
	ROM_LOAD( "teg1_wave1.ic12", 0x0800000, 0x800000, CRC(dbc74fff) SHA1(601b7e7361ea744b34e3fa1fc39d88641de7f4c6) )
ROM_END

ROM_START( tenkomor )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "tkm2vera.2e",  0x0000000, 0x200000, CRC(a9b81653) SHA1(9199505019234140b0d89e199f0db307d5bcca02) )
	ROM_LOAD16_BYTE( "tkm2vera.2j",  0x0000001, 0x200000, CRC(28cff9ee) SHA1(d1996d45cca3a9bbd6a7f39721b2ec9f3d052422) )

	ROM_REGION32_LE( 0x1c00000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "tkm1rom0l.12", 0x0000000, 0x800000, CRC(dddebb39) SHA1(44169b0c6be4d387e7b6087ce723476ee96b09b4) )
	ROM_LOAD16_BYTE( "tkm1rom0u.11", 0x0000001, 0x800000, CRC(dbcc3838) SHA1(c4e11800c5e8122044914152227b8a29e9446c9d) )
	ROM_LOAD16_BYTE( "tkm1fl1l.9",   0x1000000, 0x200000, CRC(071ef722) SHA1(7c0317b3bca2763dbbac5454901cd48a0b195edd) )
	ROM_LOAD16_BYTE( "tkm1fl1u.10",  0x1000001, 0x200000, CRC(580f8391) SHA1(2205c80721631ea8016efbe6d79f5ea7d1924278) )
	ROM_LOAD16_BYTE( "tkm1fl2l.7",   0x1400000, 0x200000, CRC(bd54efe3) SHA1(ff3573066ad2498a33ceabf378a3c69af11ee7db) )
	ROM_LOAD16_BYTE( "tkm1fl2u.8",   0x1400001, 0x200000, CRC(6e4e6320) SHA1(8d220b0028cfc2f02eb34df41a5dbb23be3e7908) )
	ROM_LOAD16_BYTE( "tkm1fl3l.5",   0x1800000, 0x200000, CRC(a31ffb10) SHA1(4f8e41e263611a7f9a63bd53ebee4e68ad0095e6) )
	ROM_LOAD16_BYTE( "tkm1fl3u.6",   0x1800001, 0x200000, CRC(bc566162) SHA1(db40d28c57dbf1351adfa2dd740edbe0b897307f) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "tkm1vera.11s", 0x0000000, 0x080000, CRC(0b414dae) SHA1(a8f77ae7ee0dc516cd9aaf944431a9a0c9dc7def) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "tkm1wave0.2",  0x0000000, 0x800000, CRC(6085387d) SHA1(a2a55f6ebe9de2d5415a1c4f3ec7975af95b45b4) )
	ROM_LOAD( "tkm1wave1.1",  0x0800000, 0x800000, CRC(7567796b) SHA1(99e4b867477da2ccddfa9bebc4be84adc5cba53c) )
ROM_END

ROM_START( tenkomorja )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "tkm1vera.2e",  0x000000, 0x200000, CRC(d4c89229) SHA1(aba6686eef924868b3bd2142fd073303fe9c4042) )
	ROM_LOAD16_BYTE( "tkm1vera.2j",  0x000001, 0x200000, CRC(a6bfcaf4) SHA1(55dfa65e07a63a413f6eb47084e60b4fc32bcde5) )

	ROM_REGION32_LE( 0x1c00000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "tkm1rom0l.12", 0x0000000, 0x800000, CRC(dddebb39) SHA1(44169b0c6be4d387e7b6087ce723476ee96b09b4) )
	ROM_LOAD16_BYTE( "tkm1rom0u.11", 0x0000001, 0x800000, CRC(dbcc3838) SHA1(c4e11800c5e8122044914152227b8a29e9446c9d) )
	ROM_LOAD16_BYTE( "tkm1fl1l.9",   0x1000000, 0x200000, CRC(071ef722) SHA1(7c0317b3bca2763dbbac5454901cd48a0b195edd) )
	ROM_LOAD16_BYTE( "tkm1fl1u.10",  0x1000001, 0x200000, CRC(580f8391) SHA1(2205c80721631ea8016efbe6d79f5ea7d1924278) )
	ROM_LOAD16_BYTE( "tkm1fl2l.7",   0x1400000, 0x200000, CRC(bd54efe3) SHA1(ff3573066ad2498a33ceabf378a3c69af11ee7db) )
	ROM_LOAD16_BYTE( "tkm1fl2u.8",   0x1400001, 0x200000, CRC(6e4e6320) SHA1(8d220b0028cfc2f02eb34df41a5dbb23be3e7908) )
	ROM_LOAD16_BYTE( "tkm1fl3l.5",   0x1800000, 0x200000, CRC(a31ffb10) SHA1(4f8e41e263611a7f9a63bd53ebee4e68ad0095e6) )
	ROM_LOAD16_BYTE( "tkm1fl3u.6",   0x1800001, 0x200000, CRC(bc566162) SHA1(db40d28c57dbf1351adfa2dd740edbe0b897307f) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "tkm1vera.11s", 0x0000000, 0x080000, CRC(0b414dae) SHA1(a8f77ae7ee0dc516cd9aaf944431a9a0c9dc7def) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "tkm1wave0.2",  0x0000000, 0x800000, CRC(6085387d) SHA1(a2a55f6ebe9de2d5415a1c4f3ec7975af95b45b4) )
	ROM_LOAD( "tkm1wave1.1",  0x0800000, 0x800000, CRC(7567796b) SHA1(99e4b867477da2ccddfa9bebc4be84adc5cba53c) )
ROM_END

ROM_START( toukon3 )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "tr1vera.2e",  0x000000, 0x200000, CRC(126ebb73) SHA1(de429e335e03f2b5116fc50f556a5507475a0535) )
	ROM_LOAD16_BYTE( "tr1vera.2j",  0x000001, 0x200000, CRC(2edb3ad2) SHA1(d1d2d78b781c7f6fb5a201785295daa825ad057e) )

	ROM_REGION32_LE( 0x00800000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "tr1rom0l.6", 0x0000000, 0x400000, CRC(42946d26) SHA1(dc7944cb6daceda41ecee7a4e3f549cba916ab87) )
	ROM_LOAD16_BYTE( "tr1rom0u.9", 0x0000001, 0x400000, CRC(e3cd0be0) SHA1(f27d14cd086b5961be33e635662aca89a5e8c857) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "tr1vera.11s", 0x0000000, 0x080000, CRC(f9ecbd19) SHA1(ad64f95ef85ee5c45dcf15217ea5adff61fddba6) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "tr1wave0.5",  0x0000000, 0x400000, CRC(07d10e55) SHA1(3af8f7019a221981984741181b6be98e2315d441) )
	ROM_LOAD( "tr1wave1.4",  0x0400000, 0x400000, CRC(34539cdd) SHA1(afb7079c0f447fbda285a5b97a37c04baf26db75) )
ROM_END

ROM_START( truckk )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "tkk2vera.2l",  0x000000, 0x200000, CRC(321344e0) SHA1(0273284d05707b76ca38fd160ef6f17572314a8b) )
	ROM_LOAD16_BYTE( "tkk2vera.2p",  0x000001, 0x200000, CRC(a7b5e4ea) SHA1(f11eefd80559b4d42318a920088b77bd67b70cc3) )

	ROM_REGION32_LE( 0x3400000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "tkk1rom0l.ic10", 0x0000000, 0x800000, CRC(5e1c8660) SHA1(05fcfa9cc834f85c38e41610a32624464d67ee21) )
	ROM_LOAD16_BYTE( "tkk1rom0u.ic14", 0x0000001, 0x800000, CRC(ee4e6566) SHA1(f3c7e227681003a3491a3996a90887e1091c0eed) )
	ROM_LOAD16_BYTE( "tkk1rom1l.ic11", 0x1000000, 0x800000, CRC(fde2863c) SHA1(a01141df8c84730974c9f37fa2ee911e2f4e45f9) )
	ROM_LOAD16_BYTE( "tkk1rom1u.ic15", 0x1000001, 0x800000, CRC(e51a56d7) SHA1(c5bad8df8edd5a5f352e5bf3bb93133503ca4ba0) )
	ROM_LOAD16_BYTE( "tkk1rom2l.ic12", 0x2000000, 0x800000, CRC(029cf107) SHA1(30cbcf00ffb9c379c64718914874936ff32ed186) )
	ROM_LOAD16_BYTE( "tkk1rom2u.ic16", 0x2000001, 0x800000, CRC(30c426dd) SHA1(2dae3d415715c779251c79006ea18e3bd14a1f51) )
	ROM_LOAD16_BYTE( "tkk1fl3l.ic4", 0x3000000, 0x200000, CRC(b0e98cb2) SHA1(8c53037b8e20410a395db8d9bb9229b550ebf228) )
	ROM_LOAD16_BYTE( "tkk1fl3u.ic5", 0x3000001, 0x200000, CRC(16d9ede7) SHA1(e989e0a6afbfca5e7ef81bf7c24e8e6b38a11eca) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "tkk2vera.11s", 0x000000, 0x080000, CRC(d3d0a46a) SHA1(20569462df49ad93ff1118b587d6d32edf9d851e) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "tkk1wave0.ic1", 0x000000, 0x800000, CRC(037d3095) SHA1(cc343bdd45d023c133964321e2df5cb1c91525ef) )

	ROM_REGION( 0x20000, "iocpu", 0)  /* Truck K. I/O board */
	ROM_LOAD( "tkk1prg0.ic7", 0x000000, 0x020000, CRC(11fd9c31) SHA1(068b8364ec0eb1e88f9f85f40b8b322876f6f3e2) )

	DISK_REGION( "cdxa_pcb:ata:0:cdrom" )
	DISK_IMAGE_READONLY( "tkk2-a", 0, SHA1(6b7c3686b22a508c44f67295b188504b757dd482) )
ROM_END

ROM_START( technodr )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "th1verb.2l",   0x000000, 0x200000, CRC(736fae08) SHA1(099e648784f617cc3b5a57a5838b8fbb54cacca1) ) // service mode reports Ver 1.06 JPN
	ROM_LOAD16_BYTE( "th1verb.2p",   0x000001, 0x200000, CRC(1fafb2d2) SHA1(ea0617714dcd7636e21a10fa2665a6f9c0f0a93b) )

	ROM_REGION32_LE( 0x3400000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "th1rom0l.6",   0x0000000, 0x400000, CRC(f8274106) SHA1(5c0b60c7440cfa01572b10ed564446d7f1a81a3a) )
	ROM_LOAD16_BYTE( "th1rom0u.9",   0x0000001, 0x400000, CRC(260ae0c5) SHA1(2ecb82e069fa64b9d3f63d6193befae02f3140e4) )
	ROM_LOAD16_BYTE( "th1rom1l.7",   0x0800000, 0x400000, CRC(56d9b477) SHA1(101b5acfbe5d292418f7fd8db642187f2b571d0b) )
	ROM_LOAD16_BYTE( "th1rom1u.10",  0x0800001, 0x400000, CRC(a45d337e) SHA1(10e230b61ab4c6aa386c68463badef0c4ba58f0e) )
	ROM_LOAD16_BYTE( "th1fl3l.12",   0x1800000, 0x200000, CRC(c330116f) SHA1(b12de5a82d6ebb5f893882b578e02af732231512) )
	ROM_LOAD16_BYTE( "th1fl3u.13",   0x1800001, 0x200000, CRC(cd4422c0) SHA1(97a9788cf0c589477f77c5403cc715ce4980a7bd) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "th1verb.11s",  0x000000, 0x080000, CRC(85806e2e) SHA1(9e0a7e6924b72e7b5b1d0e72eeec10045984dd4b) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "th1wave0.5",   0x000000, 0x400000, CRC(6cdd06fb) SHA1(31bd0b359e3b93bd40c3416a4805d0523e7f54c3) )
	ROM_LOAD( "th1wave1.4",   0x400000, 0x400000, CRC(40fd413b) SHA1(b7edf89b5fa196a0787646c73b9aa07fc062fc8b) )

	ROM_REGION( 0x40000, "iocpu", 0)  /* Truck K. I/O board */
	ROM_LOAD( "th1io-a.4f",   0x000000, 0x040000, CRC(1cbbce27) SHA1(71d61d9218543e1b0b2a6c550a8ff2b7c6267257) )

	ROM_REGION( 0x800, "at28c16", 0)    /* NVRAM with printer off by default */
	ROM_LOAD( "at28c16",      0x000000, 0x000800, CRC(01227f45) SHA1(fb4e6ebaa789e4dd2794607d149627ff17047076) )
ROM_END

ROM_START( aplarail )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "ap1vera.2e",   0x000000, 0x200000, CRC(386ffe26) SHA1(27b4bb0f0b178b4236e18d7ef9d810d1afc99798) )
	ROM_LOAD16_BYTE( "ap1vera.2j",   0x000001, 0x200000, CRC(cab954e1) SHA1(3d3b8cfbb8160bb0c421af88137b9de7f8ea22bf) )

	ROM_REGION32_LE( 0x3400000, "bankedroms", 0 ) /* main data */
	ROM_LOAD( "ap1rom0.ic9",  0x0000000, 0x800000, CRC(c8042aad) SHA1(48d4205535fd20ffc99a0caab6c5f9c66b7f64ed) )
	ROM_LOAD( "ap1rom1.ic10", 0x0800000, 0x800000, CRC(0f3210ec) SHA1(d3e0782f01e952a27d4fbe2470159e92ba57a57a) )
	ROM_LOAD( "ap1rom2.ic11", 0x1000000, 0x800000, CRC(9ef8382d) SHA1(bb1e56a4911bfc9c4f763f35885da74a1c1e96c5) )
	ROM_LOAD16_BYTE( "ap1fl3l.ic7",  0x1800000, 0x200000, CRC(b52c2df6) SHA1(9d07b546395486e7cae86d24c8a352d9e48d00d4) )
	ROM_LOAD16_BYTE( "ap1fl3h.ic8",  0x1800001, 0x200000, CRC(050af45d) SHA1(4cd93222b9a2fbfad5c93b60cdb7ccd4d58678d5) )
	ROM_LOAD16_BYTE( "ap1fl4l.ic5",  0x1c00000, 0x200000, CRC(9a4109e5) SHA1(ba59caac5f5a80fc52c507d8a47f322a380aa9a1) )
	ROM_LOAD16_BYTE( "ap1fl4h.ic6",  0x1c00001, 0x200000, CRC(9a4109e5) SHA1(ba59caac5f5a80fc52c507d8a47f322a380aa9a1) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "ap1vera.11s",  0x000000, 0x080000, CRC(126aaebc) SHA1(3f5a709c38ca38753d7a93d81e59a7ed48515b92) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "ap1wave0.ic2", 0x000000, 0x800000, CRC(003abebb) SHA1(edd70a0cc2f8648d3cbd3457ee36aa3772f97d2e) )

	ROM_REGION( 0x40000, "iocpu", 0)  /* Truck K. I/O board */
	ROM_LOAD( "asca1_io-a.ic2", 0x000000, 0x040000, CRC(77cdf69a) SHA1(497af1059f85c07bea2dd0d303481623f6019dcf) )

	ROM_REGION( 0x800, "at28c16", 0)    /* pre-calibrated NVRAM */
	ROM_LOAD( "at28c16",      0x000000, 0x000800, CRC(db1b63c5) SHA1(01fc3386a2d1cb1bed1b7fd9bd2fd59e503832d3) )
ROM_END

ROM_START( ujlnow )
	ROM_REGION32_LE( 0x00400000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "ul1vera.2l",  0x000000, 0x200000, CRC(40251bc3) SHA1(377a156d738b29d8f19d9696a261cc6e3470ee3e) )
	ROM_LOAD16_BYTE( "ul1vera.2p",  0x000001, 0x200000, CRC(98181955) SHA1(febd73361d0de10282a850611b5f966a99eecdec) )

	ROM_REGION32_LE( 0x2000000, "bankedroms", 0 ) /* main data */
	ROM_LOAD16_BYTE( "ul1rom0l.ic12", 0x0000000, 0x800000, CRC(7a77cffd) SHA1(6bd18837f475615061be73ee6d1bc09726e96fb5) )
	ROM_LOAD16_BYTE( "ul1rom0u.ic11", 0x0000001, 0x800000, CRC(1fdb34b3) SHA1(3773b3fb1574a0b4acebfaf510c75fa117394ae4) )
	ROM_LOAD16_BYTE( "ul1rom1l.ic10", 0x1000000, 0x800000, CRC(2d300d8c) SHA1(eabd0b4525530e5b33a6d85fa4d959e88c6fbd42) )
	ROM_LOAD16_BYTE( "ul1rom1u.ic9",  0x1000001, 0x800000, CRC(a4762002) SHA1(0faadc04bcc32e4f805069af8e635765a0b77ca9) )

	ROM_REGION( 0x0080000, "sub", 0 ) /* sound prg */
	ROM_LOAD16_WORD_SWAP( "ul1vera.11s", 0x000000, 0x080000, CRC(2f466df3) SHA1(6468089c6cf3e056a30fa04274056ff2c9a01d5b) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "ul1wave0.ic2", 0x0000000, 0x800000, CRC(4efbff5d) SHA1(add4caf9aa9443707bcb6cf9ebad0a5a59900ca9) )
	ROM_LOAD( "ul1wave1.ic1", 0x0800000, 0x800000, CRC(ad512f4c) SHA1(cfbecaad13a2239be0e697f3f9856401f11a1bc5) )

	ROM_REGION( 0x80000, "m148", 0) /* M148 EMI DRIVE PCB */
	ROM_LOAD( "ul1spro.10c", 0x000000, 0x080000, CRC(1bd0e763) SHA1(07e9ad77bb983fcb4e4e6f66fbe55808b5a27d2e) )

	DISK_REGION( "cdxa_pcb:ata:0:cdrom" )
	DISK_IMAGE_READONLY( "ul1-a", 0, SHA1(676f2c530f8d422ab9895ea04a43b2e9272fb8f8) )
ROM_END

} // anonymous namespace


//    YEAR  NAME       PARENT    MACHINE   INPUT      CLASS                    INIT           ROT   COMPANY            FULLNAME, FLAGS
GAME( 1996, tekken3,   0,        coh700,   namcos12,  namcos12_state,          init_namcos12, ROT0, "Namco",           "Tekken 3 (World, TET2/VER.E1)", 0 ) /* KC006 */
GAME( 1996, tekken3d,  tekken3,  coh700,   namcos12,  namcos12_state,          init_namcos12, ROT0, "Namco",           "Tekken 3 (World, TET2/VER.D)", 0 ) /* KC006 */
GAME( 1996, tekken3c,  tekken3,  coh700,   namcos12,  namcos12_state,          init_namcos12, ROT0, "Namco",           "Tekken 3 (World, TET2/VER.C)", 0 ) /* KC006 */
GAME( 1996, tekken3b,  tekken3,  coh700,   namcos12,  namcos12_state,          init_namcos12, ROT0, "Namco",           "Tekken 3 (World, TET2/VER.B)", 0 ) /* KC006 */
GAME( 1996, tekken3a,  tekken3,  coh700,   namcos12,  namcos12_state,          init_namcos12, ROT0, "Namco",           "Tekken 3 (World, TET2/VER.A)", 0 ) /* KC006 */
GAME( 1996, tekken3ud, tekken3,  coh700,   namcos12,  namcos12_state,          init_namcos12, ROT0, "Namco",           "Tekken 3 (US, TET3/VER.D)", 0 ) /* KC006 */
GAME( 1996, tekken3ua, tekken3,  coh700,   namcos12,  namcos12_state,          init_namcos12, ROT0, "Namco",           "Tekken 3 (US, TET3/VER.A)", 0 ) /* KC006 */
GAME( 1996, tekken3je1,tekken3,  coh700,   namcos12,  namcos12_state,          init_namcos12, ROT0, "Namco",           "Tekken 3 (Japan, TET1/VER.E1)", 0 ) /* KC006 */
GAME( 1996, tekken3ja, tekken3,  coh700,   namcos12,  namcos12_state,          init_namcos12, ROT0, "Namco",           "Tekken 3 (Japan, TET1/VER.A)", 0 ) /* KC006 */
GAME( 1997, lbgrande,  0,        coh700,   namcos12,  namcos12_state,          init_namcos12, ROT0, "Namco",           "Libero Grande (World, LG2/VER.A)", 0 ) /* KC014 */
GAME( 1997, toukon3,   0,        coh700,   namcos12,  namcos12_state,          init_namcos12, ROT0, "Namco / Tomy",    "Shin Nihon Pro Wrestling Toukon Retsuden 3 Arcade Edition (Japan, TR1/VER.A)", MACHINE_IMPERFECT_GRAPHICS ) /* KC019 */
GAME( 1998, soulclbr,  0,        coh700,   namcos12,  namcos12_state,          init_namcos12, ROT0, "Namco",           "Soul Calibur (Asia, SOC14/VER.C)", 0 )
GAME( 1998, soulclbrab,soulclbr, coh700,   namcos12,  namcos12_state,          init_namcos12, ROT0, "Namco",           "Soul Calibur (Asia, SOC14/VER.B)", 0 )
GAME( 1998, soulclbra, soulclbr, coh700,   namcos12,  namcos12_state,          init_namcos12, ROT0, "Namco",           "Soul Calibur (World, SOC12/VER.A2)", 0 ) /* KC020 */
GAME( 1998, soulclbruc,soulclbr, coh700,   namcos12,  namcos12_state,          init_namcos12, ROT0, "Namco",           "Soul Calibur (US, SOC13/VER.C)", 0 )
GAME( 1998, soulclbrub,soulclbr, coh700,   namcos12,  namcos12_state,          init_namcos12, ROT0, "Namco",           "Soul Calibur (US, SOC13/VER.B)", 0 ) /* KC020 */
GAME( 1998, soulclbrjc,soulclbr, coh700,   namcos12,  namcos12_state,          init_namcos12, ROT0, "Namco",           "Soul Calibur (Japan, SOC11/VER.C)", 0 ) /* KC020 */
GAME( 1998, soulclbrjb,soulclbr, coh700,   namcos12,  namcos12_state,          init_namcos12, ROT0, "Namco",           "Soul Calibur (Japan, SOC11/VER.B)", 0 ) /* KC020 */
GAME( 1998, soulclbrja,soulclbr, coh700,   namcos12,  namcos12_state,          init_namcos12, ROT0, "Namco",           "Soul Calibur (Japan, SOC11/VER.A2)", 0 ) /* KC020 */
GAME( 1998, ehrgeiz,   0,        coh700,   namcos12,  namcos12_state,          init_namcos12, ROT0, "Square / Namco",  "Ehrgeiz (World, EG2/VER.A)", 0 ) /* KC021 */
GAME( 1998, ehrgeizua, ehrgeiz,  coh700,   namcos12,  namcos12_state,          init_namcos12, ROT0, "Square / Namco",  "Ehrgeiz (US, EG3/VER.A)", 0 ) /* KC021 */
GAME( 1998, ehrgeizja, ehrgeiz,  coh700,   namcos12,  namcos12_state,          init_namcos12, ROT0, "Square / Namco",  "Ehrgeiz (Japan, EG1/VER.A)", 0 ) /* KC021 */
GAME( 1998, mdhorse,   0,        coh700,   namcos12,  namcos12_state,          init_alt_bank1,ROT0, "MOSS / Namco",    "Derby Quiz My Dream Horse (Japan, MDH1/VER.A2)", 0 ) /* KC035 */
GAME( 1998, aplarail,  0,        aplarail, aplarail,  namcos12_boothack_state, init_namcos12, ROT0, "Namco / Tomy",    "Attack Pla Rail (Japan, AP1/VER.A)", 0 ) /* KC032 */
GAME( 1998, sws98,     0,        coh700,   namcos12,  namcos12_state,          init_namcos12, ROT0, "Namco",           "Super World Stadium '98 (Japan, SS81/VER.A)", 0 ) /* KC0?? */
GAME( 1998, technodr,  0,        technodr, technodr,  namcos12_boothack_state, init_technodr, ROT0, "Namco",           "Techno Drive (Japan, TH1/VER.B)", MACHINE_NODEVICE_PRINTER ) /* KC056 */
GAME( 1998, tenkomor,  0,        coh700,   namcos12,  namcos12_boothack_state, init_namcos12, ROT90,"Namco",           "Tenkomori Shooting (World, TKM2/VER.A1)", 0 ) /* KC036 */
GAME( 1998, tenkomorja,tenkomor, coh700,   namcos12,  namcos12_boothack_state, init_namcos12, ROT90,"Namco",           "Tenkomori Shooting (Japan, TKM1/VER.A1)", 0 ) /* KC036 */
GAME( 1998, fgtlayer,  0,        coh700,   namcos12,  namcos12_boothack_state, init_namcos12, ROT0, "Arika / Namco",   "Fighting Layer (Asia, FTL3/VER.A)", 0 ) /* KC037 */
GAME( 1998, fgtlayerj, fgtlayer, coh700,   namcos12,  namcos12_boothack_state, init_namcos12, ROT0, "Arika / Namco",   "Fighting Layer (Japan, FTL0/VER.A)", 0 ) /* KC037 */
GAME( 1998, pacapp,    0,        coh700,   namcos12,  namcos12_boothack_state, init_namcos12, ROT0, "Produce / Namco", "Paca Paca Passion (Japan, PPP1/VER.A2)", 0 ) /* KC038 */
GAME( 1999, ptblank2,  0,        ptblank2, ptblank2,  namcos12_boothack_state, init_ptblank2, ROT0, "Namco",           "Point Blank 2 (World, GNB5/VER.A)", 0 ) /* KC042 */
GAME( 1999, gunbarl,   ptblank2, ptblank2, ptblank2,  namcos12_boothack_state, init_ptblank2, ROT0, "Namco",           "Gunbarl (Japan, GNB4/VER.A)", 0 ) /* KC042 */
GAME( 1999, sws99,     0,        coh700,   namcos12,  namcos12_state,          init_namcos12, ROT0, "Namco",           "Super World Stadium '99 (Japan, SS91/VER.A3)", 0 ) /* KC043 */
GAME( 1999, tektagt,   0,        tektagt,  namcos12,  namcos12_boothack_state, init_namcos12, ROT0, "Namco",           "Tekken Tag Tournament (World, TEG2/VER.C1, set 1)", 0 ) /* KC044 */
GAME( 1999, tektagtc1, tektagt,  tektagt,  namcos12,  namcos12_boothack_state, init_namcos12, ROT0, "Namco",           "Tekken Tag Tournament (World, TEG2/VER.C1, set 2)", 0 ) /* KC044 */
GAME( 1999, tektagtuc1,tektagt,  tektagt,  namcos12,  namcos12_boothack_state, init_namcos12, ROT0, "Namco",           "Tekken Tag Tournament (US, TEG3/VER.C1)", 0 ) /* KC044 */
GAME( 1999, tektagtub, tektagt,  tektagt,  namcos12,  namcos12_boothack_state, init_namcos12, ROT0, "Namco",           "Tekken Tag Tournament (US, TEG3/VER.B)", 0 ) /* KC044 */
GAME( 1999, tektagtjc1,tektagt,  tektagt,  namcos12,  namcos12_boothack_state, init_namcos12, ROT0, "Namco",           "Tekken Tag Tournament (Japan, TEG1/VER.C1)", 0 ) /* KC044 */
GAME( 1999, tektagtjb, tektagt,  tektagt,  namcos12,  namcos12_boothack_state, init_namcos12, ROT0, "Namco",           "Tekken Tag Tournament (Japan, TEG1/VER.B)", MACHINE_NOT_WORKING ) /* KC044 */
GAME( 1999, tektagtja, tektagt,  tektagt,  namcos12,  namcos12_boothack_state, init_namcos12, ROT0, "Namco",           "Tekken Tag Tournament (Japan, TEG1/VER.A3)", MACHINE_NOT_WORKING ) /* KC044 */
GAME( 1999, ghlpanic,  0,        ptblank2, ghlpanic,  namcos12_boothack_state, init_namcos12, ROT0, "Eighting / Raizing / Namco", "Ghoul Panic (World, OB2/VER.A)", 0 ) /* KC045 */
GAME( 1999, ohbakyuun, ghlpanic, ptblank2, ghlpanic,  namcos12_boothack_state, init_namcos12, ROT0, "Eighting / Raizing / Namco", "Oh! Bakyuuun (Japan, OB1/VER.A)", 0 ) /* KC045 */
GAME( 1999, pacapp2,   0,        coh700,   namcos12,  namcos12_boothack_state, init_namcos12, ROT0, "Produce / Namco", "Paca Paca Passion 2 (Japan, PKS1/VER.A)", 0 ) /* KC046 */
GAME( 1999, mrdrillr,  0,        coh700,   namcos124w,namcos12_boothack_state, init_namcos12, ROT0, "Namco",           "Mr. Driller (US, DRI3/VER.A2)", 0 ) /* KC048 */
GAME( 1999, mrdrillrj, mrdrillr, coh700,   namcos124w,namcos12_boothack_state, init_namcos12, ROT0, "Namco",           "Mr. Driller (Japan, DRI1/VER.A2)", 0 ) /* KC048 */
GAME( 1999, ujlnow,    0,        cdxa_pcb, ujlnow,    namcos12_cdxa_state,     init_alt_bank1,ROT0, "Namco",           "Um Jammer Lammy NOW! (Japan, UL1/VER.A)", 0 ) /* KC049 */
GAME( 1999, kaiunqz,   0,        coh700,   namcos12,  namcos12_state,          init_alt_bank1,ROT0, "Namco",           "Kaiun Quiz (Japan, KW1/VER.A)", 0 ) /* KC050 */
GAME( 1999, pacappsp,  0,        coh700,   namcos12,  namcos12_boothack_state, init_namcos12, ROT0, "Produce / Namco", "Paca Paca Passion Special (Japan, PSP1/VER.A)", 0 ) /* KC052 */
GAME( 1999, aquarush,  0,        coh700,   namcos12,  namcos12_state,          init_namcos12, ROT0, "Namco",           "Aqua Rush (Japan, AQ1/VER.A1)", 0 ) /* KC053 */
GAME( 1999, golgo13,   0,        golgo13,  golgo13,   namcos12_boothack_state, init_alt_bank1,ROT0, "Eighting / Raizing / Namco", "Golgo 13 (Japan, GLG1/VER.A)", 0 ) /* KC054 */
GAME( 2000, g13knd,    0,        golgo13,  golgo13,   namcos12_boothack_state, init_alt_bank1,ROT0, "Eighting / Raizing / Namco", "Golgo 13 Kiseki no Dandou (Japan, GLS1/VER.A)", 0 ) /* KC059 */
GAME( 2000, sws2000,   0,        coh700,   namcos12,  namcos12_boothack_state, init_namcos12, ROT0, "Namco",           "Super World Stadium 2000 (Japan, SS01/VER.A)", MACHINE_NOT_WORKING ) /* KC055 */
GAME( 2000, truckk,    0,        truckk,   truckk,    namcos12_cdxa_state,     init_truckk,   ROT0, "Metro / Namco",   "Truck Kyosokyoku (Japan, TKK2/VER.A)", MACHINE_IMPERFECT_SOUND ) /* KC056 */
GAME( 2000, kartduel,  0,        kartduel, kartduel,  namcos12_boothack_state, init_namcos12, ROT0, "Gaps / Namco",    "Kart Duel (World, KTD2/VER.A)", MACHINE_NOT_WORKING ) /* KC057 */
GAME( 2000, kartduelj, kartduel, kartduel, kartduel,  namcos12_boothack_state, init_namcos12, ROT0, "Gaps / Namco",    "Kart Duel (Japan, KTD1/VER.A)", MACHINE_NOT_WORKING ) /* KC057 */
GAME( 2001, sws2001,   sws2000,  coh716,   namcos12,  namcos12_boothack_state, init_alt_bank1,ROT0, "Namco",           "Super World Stadium 2001 (Japan, SS11/VER.A)", MACHINE_NOT_WORKING ) /* KC061 */
