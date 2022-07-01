// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

Namco System 10 - Arcade PSX Hardware
=====================================
Driver by smf. Board notes by Guru


----------------------------------------
Guru Readme for Namco System 10 Hardware
----------------------------------------
Note! This document is a Work-In-Progress and will be updated from time to time when more dumps are available.

This document covers all the known Namco System 10 games, including....
*Drum Master                                                     (C) Namco, 2001
*Drum Master 2                                                   (C) Namco, 2001
*Drum Master 3                                                   (C) Namco, 2002
*Drum Master 4                                                   (C) Namco, 2003
*Drum Master 5                                                   (C) Namco, 2003
*Drum Master 6                                                   (C) Namco, 2004
*Drum Master 7                                                   (C) Namco, 2005
GAHAHA Ippatsu-dou (GID2 Ver.A)                                  (C) Namco/Metro, 2000
GAHAHA Ippatsu-dou 2 (GIS1 Ver.A)                                (C) Namco/Metro, 2001
Gamshara (10021 Ver.A)                                           (C) Mitchell, 2003
Gegege no Kitarō Yōkai Yokochō Matsuri De Batoru Ja (GYM1 Ver.A) (C) Namco, 2007
Gekitoride-Jong Space (10011 Ver.A)                              (C) Namco/Metro, 2001
Golgo 13 Juusei no Chinkonka (GLT1 Ver.A)                        (C) Namco/8ing/Raizing, 2001
Gunbalina (GNN1 Ver. A)                                          (C) Namco, 2000
*Hard Puncher Hajime no Ippo 2 Ouja e no Chousen                 (C) Namco/Taito, 2002
*Honne Hakkenki                                                  (C) Namco, 2001
Keroro Gunsō Chikyū Shinryaku Shirei Dearimasu! (KRG1 Ver.A)     (C) Namco, 2006
**Knock Down 2001                                                (C) Namco, 2001
Kotoba no Puzzle Mojipittan (KPM1 Ver.A)                         (C) Namco, 2001
*Kurukuru Food                                                   (C) Namco, 2002
Mr Driller 2 (DR21 Ver.A)                                        (C) Namco, 2000
Mr Driller 2 (DR22 Ver.A)                                        (C) Namco, 2000
Mr Driller G (DRG1 Ver.A)                                        (C) Namco, 2001
NFL Classic Football (NCF3 Ver.A)                                (C) Namco, 2003
Pacman Ball (PMB2 Ver.A)                                         (C) Namco, 2003
Panicuru Panekuru (PPA1 Ver.A)                                   (C) Namco, 2001
*Photo Battle                                                    (C) Namco, 2001
Point Blank 3 (GNN2 Ver. A)                                      (C) Namco, 2000
Puzz Ball (PZB1 Ver. A)                                          (C) Namco, 2002
*Ren-ai Quiz High School Angel                                   (C) Namco, 2002
Seishun Quiz Colorful High School (CHS1 Ver.A)                   (C) Namco, 2002
Sekai Kaseki Hakken (Japan, SKH1 Ver.A)                          (C) Namco, 2004
Shamisen Brothers                                                (C) Kato/Konami, 2003
Star Trigon (STT1 Ver.A)                                         (C) Namco, 2002
*Taiko no Tatsujin                                               (C) Namco, 2001
Taiko no Tatsujin 2 (TK21 Ver.C)                                 (C) Namco, 2001
Taiko no Tatsujin 3 (TK31 Ver.A)                                 (C) Namco, 2002
Taiko no Tatsujin 4 (TK41 Ver.A)                                 (C) Namco, 2003
Taiko no Tatsujin 5 (TK51 Ver.A)                                 (C) Namco, 2003
Taiko no Tatsujin 6 (TK61 Ver.A)                                 (C) Namco, 2004
Tsukkomi Yousei Gips Nice Tsukkomi (NTK1 Ver.A)                  (C) Namco/Metro, 2002
Uchuu Daisakusen Chocovader Contactee (CVC1 Ver.A)               (C) Namco, 2002
Unknown medal (?) game (MTL1 SPR0B)                              (C) ?,     200?
Unknown medal (?) game (peeled off sticker)                      (C) ?,     2005

* - denotes not dumped yet.
** - denotes incomplete dump.

The Namco System 10 system comprises 2 or 3 PCB's....
MAIN PCB - This is the mother board PCB. It holds the main CPU/GPU & SPU and all sound circuitry, program & video RAM,
           controller/input logic and video output circuitry. Basically everything except the ROMs.
           There are three known revisions of this PCB so far. The differences seem very minor. The 2nd and 3rd revision
           have an updated CPLD revision.
           The 3rd revision has an updated model Sony chip. The only other _noticeable_ difference is some component
           shuffling in the sound amplification section to accommodate two extra 1000uF capacitors and one 470uF capacitor
           has been replaced by a 1000uF capacitor. Everything else, including all the PLDs appears to be identical.
           Note there are no ROMs on the Main PCB and also no custom Namco chips on System10, which seem to have been
           phased out. Instead, they have been replaced by (custom programmed) CPLDs, probably due to cost-cutting
           measures within the company, or to reduce the cost of System10 to an entry-level no-frills development platform.
MEM PCB  - There are three known revisions of this PCB (so far). They're mostly identical except for the type/number of ROMs
           used and the contents of the CPLD.  The 2nd revision also has a RAM chip on it. The 3rd revision has some extra
           hardware present to decode MP3 data.
           Each game has a multi-letter code assigned to it which is printed on a small sticker and placed on the top side
           of the MEM PCB.
           This code is then proceeded by a number (only '1' & '2' seen so far), then 'Ver.' then A/B/C/D/E (only 'A' seen so far)
           which denotes the software revision, and in some cases a sub-revision such as 1 or 2 (usually only listed in the
           test mode).
           The first 1 denotes a Japanese version. Other numbers denote a World version.
           For World versions, only the main program changes, the rest of the (graphics) ROMs use the Japanese version ROMs.
           If the version sticker has a red dot it means nothing as several identical versions of the same games exist with and
           without the red dot. A similar red dot has also been seen on Namco System 246 security carts and means nothing.
           Speculation about a red dot on a sticker is pointless and has no impact on the dumps or the emulation.
           Any System 10 MEM PCB can be swapped to run on any System 10 Main PCB regardless of the main board revision.
           The high scores are stored on the MEM PCB (probably inside the main program EEPROMs/FlashROMs).
           There are no "alt" versions with the same code, this simply means the game was dumped without first resetting the
           high score records and coinage/play statistics info to factory defaults.
           Also, on all System 10 games, there is a sticker with a serial number on it and the program ROMs also contain
           that same serial number. I'm not sure why, they're not exactly _easily_ traceable and no one cares either way ;-)
EXIO PCB - Optional I/O & Extra Controls PCB

           See the Main PCB, ROM Daughterboard PCB and Expansion PCB below for more details.


Main PCB Layout
---------------

Revision 1
SYSTEM10 MAIN PCB 8906960103 (8906970103)

Revision 2
SYSTEM10 MAIN PCB 8906960104 (8906970104)

Revision 3
SYSTEM10 MAIN PCB 8906962400 (8906972400)
  |----------------------------------------------------------|
  |   LA4705    VR1                     J201                 |
  |                           |----------------------|       |
  |           NJM3414         |----------------------|       |
|-|     J10                                                  |
|       BA3121                                               |
|             NJM3414            54V25632      54V25632    J1|
|                     CXD1178Q                               |
|J JP4                          |---------|   |-------|      |
|A                              |         |   |       |      |
|M         CXA2067AS            |CXD8561CQ|   |CY37128|      |
|M                 53.693175MHz |         |   |VP160  |      |
|A                              |         |   |       |      |
|                               |---------|   |-------|      |
|                                                            |
|                            101.4912MHz                     |
|-|          MAX734  IS41LV16100                             |
  |                              |---------|                 |
  |   DSW1           IS41LV16100 |         |                 |
  |                              |CXD8606BQ|                 |
  |        GAL16V8D              |         |                 |
  |J5      |-|           *       |         |          PST592 |
  |        | |                   |---------| |--------|      |
  |        | |                               |        |      |
  |        | |           *                   |CXD2938Q|      |
  |J4      | |J202             IS41LV16256   |        |      |
  |        | |                               |        |      |
  |        | |      EPM3064                  |--------|      |
  |        | |                                               |
  |        |-|                 PQ30RV21                      |
  |                                                  J103    |
  |----------------------------------------------------------|
Notes:
------
      CXD8606BQ   : SONY CXD8606BQ Central Processing Unit / GTE     (QFP208)
                     - replaced by CXD8606CQ on Revision 3 Main PCB
      CXD8561CQ   : SONY CXD8561CQ Graphics Processor Unit           (QFP208)
      CXD2938Q    : SONY CXD2938Q  Sound Processor Unit              (QFP208)
      CXD1178Q    : SONY CXD1178Q  8-bit RGB 3-channel D/A converter (QFP48)
      CXA2067AS   : SONY CXA2067AS TV/Video circuit RGB Pre-Driver   (SDIP30)
      CY37128VP160: CYPRESS CY37128VP160 Complex Programmable Logic Device (TQFP160, stamped 'S10MA1')
                     - replaced by an updated revision on Revision 2 & 3 Main PCB and stamped 'S10MA1B'
      EPM3064     : Altera MAX EPM3064ATC100-10 Complex Programmable Logic Device (TQFP100, stamped 'S10MA2A')
      GAL16V8D    : GAL16V8D PAL (PLCC20, stamped 'S10MA3A')
      IS41LV16100 : ISSI IS41LV16100S-50T 1M x16 EDO DRAM (x2, TSOP50(44) Type II)
      IS41LV16256 : ISSI IS41LV16256-50T 256k x16 EDO DRAM (TSOP44(40) Type II)
      54V25632    : OKI 54V25632 256K x32 SGRAM (x2, QFP100)
      PQ30RV31    : Sharp PQ30RV31 5 Volt to 3.3 Volt Voltage Regulator
      LA4705      : LA4705 15W 2-channel Power Amplifier (SIP18)
      MAX734      : MAX734 +12V 120mA Flash Memory Programming Supply Switching Regulator (SOIC8)
      PST592      : PST592J System Reset IC with 2.7V detection circuit (MMP-4A)
      BA3121      : Rohm BA3121 Dual Channel Ground Isolation Amplifier & Noise Eliminator (SOIC8)
      JRC3414     : New Japan Radio Co. Ltd. JRC3414 Single-Supply Dual High Current Operational Amplifier (x2, SOIC8)
      DSW1        : 8 position DIP switch
      JP4         : 2 position jumper, set to NC, alt. position labelled SYNC (Note: changing the jumper position has no visual effect)
      J1          : 40 Pin IDC connector for a flat 40-wire cable, used for games that have a DVDROM Drive
      J4          : 10 pin header for extra controls etc  \ (note: custom Namco 48 pin edge connector is not on System10 PCBs)
      J5          : 4 pin header for stereo sound out     /
      J10         : 4 pin header for audio input from ROM board type 3. This audio is mixed with the other main board audio.
      J201        : 100 pin custom Namco connector for mounting of MEM PCB. This connector is surface-mounted, not a thru-hole type.
      J202        : 80 pin custom Namco connector for mounting of another board. This connector is surface-mounted, not a thru-hole type.
                    There are additional boards that plug in here and provide extra functionality. See below for the details.
      J103        : 6-pin JAMMA2 power plug (Note none of the other JAMMA2 standard connectors are present)
      VR1         : Volume potentiometer
      *           : Unpopulated position for IS41LV16100 1M x16 EDO DRAM

Additional Notes:
                1. In test mode (Display Test) the screen can be set to interlace or non-interlace mode. The graphics in
                   interlace mode are visually much smoother with noticeable screen flickering. Non-interlace modes gives
                   a much blockier graphic display (i.e. lower resolution) but without screen flickering.
                2. There is no dedicated highscore/options EEPROM present on the PCB, the game stores it's settings on the
                   game board (probably in the program EEPROMs/FlashROMs).

ROM Daughterboard PCBs
----------------------
This PCB holds all the ROMs.
There are three known types of ROM daughterboards used on S10 games (so far).
All of the PCBs are the same size (approx 5" x 5") containing one custom connector surface-mounted to the underside of
the PCB, some mask ROMs/flash ROMs, a CPLD (which seems to be the customary 'KEYCUS' chip. On the 2nd type a RAM
chip is also present. The 3rd type has additional hardware to decode MP3 audio and a ROMless Microcontroller.

********
*Type 1*
********
System10 MEM(M) PCB 8906961000 (8906970700)
|-------------------------------------|
|                                     |
|                       |-------|     +-
|                       |       |     +-
|                       |CY37128|   J1+-
|                       |VP160  |     +-
|                       |       |     +-
|     7E     7D         |-------|     +-
|                                     |
|     6E     6D                       |
|                                     |
|     5E     5D                 5A    |
|                                     |
|     4E     4D                 4A    |
|                                     |
|     3E     3D                       |
|                                     |
|     2E     2D                 2A    |
|                                     |
|     1E     1D                 1A    |
|                                     |
|-------------------------------------|
Notes:
      CY37128VP160: CY37128VP160 Cypress Complex Programmable Logic Device (TQFP160)
      1A - 5A     : Intel Flash DA28F640J5 64MBit Flash EEPROM (SSOP56)
      1D - 7E     : Samsung Electronics K3N9V1000A-YC 128MBit mask ROM (TSOP48) (see note 3)
      J1          : 6 pin header for programming the CPLD via JTAG

This PCB is used on:

              Software     MEM PCB
Game          Revision     Sticker      KEYCUS   ROMs Populated
------------------------------------------------------------------------------------
Mr Driller 2  DR21/VER.A3  DR21 Ver.A   KC001A   DR21VERA.1A, DR21MA1.1D, DR21MA2.2D
Mr Driller 2  DR22/VER.A3  DR22 Ver.A   KC001A   DR22VERA.1A, DR21MA1.1D, DR21MA2.2D

      Note
      1. The ROM PCB has locations for 4x 64MBit program ROMs, but only 1A is populated.
      2. The ROM PCB has locations for 14x 128MBit GFX ROMs (Total capacity = 2048MBits) but only 1D and 2D are populated.
      3. These ROMs are only 18mm long, dumping them requires a special custom adapter

********
*Type 2*
********
System10 MEM(N) PCB 8906961402 (8906971402)
|-------------------------------------|
|                                     |
|                    |---------|      +-
|                    |         |      +-
|                    |CY37256  |    J1+-
|     8E     8D      |VP208    |      +-
|                    |         |      +-
|     7E     7D      |---------|      +-
|                                     |
|     6E     6D                       |
|                                     |
|     5E     5D       CY7C1019        |
|                                     |
|     4E     4D                       |
|                                     |
|     3E     3D                       |
|                                     |
|     2E     2D                       |
|                                     |
|     1E     1D                       |
|                                     |
|-------------------------------------|
Notes:
      CY37256VP208: Cypress CY37256VP208 Complex Programmable Logic Device (TQFP208)
      CY7C1019    : Cypress CY7C1019BV33-15VC or Samsung Electronics K6R1008V1C-JC15 128k x8 bit 3.3V High Speed CMOS Static Ram (SOJ32)
      1D - 8E     : Samsung Electronics K9F2808U0B-YCBO 128MBit NAND Flash EEPROM (TSOP48)
      J1          : 6 pin header for programming the CPLD via JTAG

This PCB is used on:

                                                    MEM PCB
Game                                                Sticker       KEYCUS   ROMs Populated       CD            Notes
----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Gamshara                                            10021 Ver.A   KC020A   8E, 8D               N/A
Gegege no Kitarō Yōkai Yokochō Matsuri De Batoru Ja GYM1  Ver.A   KC052A   8E, 8D               N/A           also has a Namco S10 MGEX10 (8681960200) PCB
Gekitoride-Jong Space                               10011 Ver.A   KC003A   8E, 8D, 7E, 7D       N/A
Gunbalina                                           GNN1  Ver.A   KC002A   8E, 8D               N/A           see note 3
Keroro Gunsō Chikyū Shinryaku Shirei Dearimasu!     KRG1  Ver.A   KC047A1  8E, 8D               N/A           also has a Namco S10 MGEX10 (8681960200) PCB
Knock Down 2001                                     KD11  Ver.B   KC011A   8E, 8D               N/A           also has a Namco P-DRIVE PCB 1908961101 (1908971101) with an H8/3002
Kono Tako                                           10021 Ver.A   KC034A   8E, 8D               N/A
Kotoba no Puzzle Mojipittan                         KPM1  Ver.A   KC012A   8E, 8D, 7E           N/A
Mr Driller G                                        DRG1  Ver.A   KC007A   8E, 8D, 7E           N/A
NFL Classic Football                                NCF3  Ver.A   KC027A   8E, 8D, 7E, 7D       N/A
Pacman Ball                                         PMB2  Ver.A   KC026A   8E, 8D               N/A
Panicuru Panekuru                                   PPA1  Ver.A   KC017A   8E, 8D, 7E           N/A
Point Blank 3                                       GNN2  Ver.A   KC002A   8E, 8D               N/A           see note 3
Puzz Ball                                           PZB1  Ver.A   KC013A   8E, 8D               N/A           also has a Namco S10 MGEX10 (8681960201) PCB, unverified title
Sekai Kaseki Hakken                                 SKH1  Ver.A   KC035A   8E, 8D               N/A           also has a Namco S10 MGEX10 (8681960201) PCB, unverified title
Star Trigon                                         STT1  Ver.A   KC019A   8E, 8D               N/A
Taiko no Tatsujin 2                                 TK21  Ver.C   KC010A   8E, 8D, 7E           TK21-A        KEYCUS is marked KC007A, KC010A is a sticker
Taiko no Tatsujin 3                                 TK31  Ver.A   KC016A   8E, 8D, 7E           not dumped    For all TK* games see note 2
Taiko no Tatsujin 4                                 TK41  Ver.A   KC024A   8E, 8D, 7E           TK-4
Taiko no Tatsujin 5                                 TK51  Ver.A   KC031A   8E, 8D, 7E           not dumped
Taiko no Tatsujin 6                                 TK61  Ver.A   KC036A   8E, 8D, 7E           TK-6
Utyuu Daisakusen Chocovader Contactee               CVC1  Ver.A   KC022A   8E, 8D, 7E, 7D, 6E   N/A
unknown medal (?) game                              MTL1  SPR0B   KC043A   8E, 8D               N/A           also has a Namco System10 EXFINAL PCB 8906962603 (8906962703)
unknown medal (?) game                              peeled off    KC039A   8E, 8D               N/A           also has a Namco S10 MGEX10 (8681960201) PCB

      Notes:
      1. The ROM PCB has locations for 16x 128MBit FlashROMs (Total capacity = 2048MBits) but usually only a few are populated.
      2. All of the Taiko no Tatsujin games require a CDROM disc. The game will not show anything on screen
         if the CD drive & disc is not present and working. The disc contains binary data.
      3. Some kind of block locking or protection issues in the NAND FlashROM prevents the last NAND block being dumped.

********
*Type 3*
********
System10 MEM(P3) PCB 8906962201 (8906972201)
|-------------------------------------|
|TMP95C061       J101     L   K6R1008 |
|                PST575D              +-
|      LLLL   |-------|               +-
|VHCT245      |       |    |-------|  +-
|      LCX245 |CY37256|    |       |J1+-
|  07VZ5M     |VP208  |    |CY37256|  +-
|  07VZ5M     |(2)    |    |VP208  |  +-
|             |-------|    |(1)    |  |
|J3                        |-------|  |
|           HY57V641620    DSW(4)     |
|                        LCX245 LCX245|
|         LC82310        LCX245 LCX245|
|   3414      16.9344MHz              |
|                VHC14    L           |
|                         L           |
|                         L           |
|                         L           |
|     0  2  4  6  8  10  12  14       |
|J2                                   |
|     1  3  5  7  9  11  13  15       |
|-------------------------------------|
Notes:
      TMP95C061      : Toshiba TMP95C061 TLCS-900 Series CMOS 16-bit Microcontroller; No internal ROM or RAM (QFP100)
      CY37256VP208(1): Cypress CY37256VP208 Complex Programmable Logic Device, marked with code 'KC' and a number.
                       This is the Namco KEYCUS chip which is unique to each game (TQFP208)
      CY37256VP208(2): Cypress CY37256VP208 Complex Programmable Logic Device, marked 'S10MEP2A' (TQFP208)
      K6R1008        : Samsung Electronics K6R1008V1C-JC15 128k x8-bit 3.3V High Speed CMOS Static Ram (SOJ32)
      HY57V641620    : Hyundai HY57V641620 4 Banks x1M x16-bit Synchronous DRAM (TSOP54 Type II)
      0-15           : Samsung Electronics K9F2808U0A-YCBO 16Mx8-bit (128M-bit) NAND Flash ROM (TSOP48)
                       Note! These ROMs also hold data for high scores and play time and coin history.
                       They must be reset to factory defaults before dumping so the dump is clean.
      LC82310        : Sanyo LC82310 MP3 decoder IC (QFP64)
      3414           : New Japan Radio Co. Ltd. JRC3414 Single-Supply Dual High Current Operational Amplifier (SOIC8)
      07VZ5M         : Sharp 07VZ5M Variable Voltage Regulator
      PST575D        : Mitsumi PST575D System Reset IC. Available in voltage detection C through L with voltages 4.5V-2.3V
                       This D version triggers a reset at 4.2V (MMP-4A)
      J1             : 6 pin header for programming the CPLDs via JTAG
      J2             : 4 pin connector joined to main board for MP3 audio output from ROM board
      J3             : 6 pin connector joined to V278 EMI PCB (filter board on outside of metal box) via 16-pin IDC connector
                       This connector is probably for extra controls
      L              : LED (SMD 0603)

      Note
      1. The ROM PCB has locations for 16x Flash ROMs (Total capacity = 2048M-bits) but usually only a few are populated.

This PCB is used on:

                                   MEM PCB
Game                               Sticker      KEYCUS   ROMs Populated
-----------------------------------------------------------------------------------------------------
Golgo 13 Juusei no Chinkonka       GLT1 Ver.A   KC009A   0, 1, 2, 3, 4, 5
Seishun Quiz Colorful High School  CHS1 Ver.A   KC025A   0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
Tsukkomi Yousei Gips Nice Tsukkomi NTK1 Ver.A   KC018A   0, 1, 2, 3, 4, 5, 6, 7


Expansion Daughterboard PCB
---------------------------
This PCB provides input/output capabilities for JVS hook-ups and allows extra controls to be connected.
There is actually only one PCB design but there are several variations where some of the connectors and
ICs are not populated if the game does not need that capability. However the PCB contains all the
locations/pads/holes etc to mount those parts. In most cases seen so far this PCB is optional but on some
games (e.g. Point Blank 3) it is required for proper game play.

System10 EXIO PCB 8906960602 (8906970602)
|--------------------------------------------------------|
|         J4         J5                  J3       J2     |
|                                       ADM485  MC14052  |-|
|     LT1181A                                              |
|                                             VHCT245      |
|                  CY37128VP100               VHCT245      |
|J6                                  VHCT245  VHCT245    J1|
|                                    VHCT245               |
|                                    VHCT245               |
|   VHC14      61C256                                      |
|     TMP95C061                                            |
|    22.1184MHz                   VHCT244                |-|
|            LLLL        J7                   VHCT574  J8|
|--------------------------------------------------------|
Notes:
      TMP95C061    : Toshiba TMP95C061 TLCS-900 Series CMOS 16-bit Microcontroller; No internal ROM or RAM (QFP100)
      CY37128VP160 : CY37128VP100 Cypress Complex Programmable Logic Device, marked 'S10XIO1A' or 'S10XIO1B' or 'S10XIO1C' (TQFP100)
      VHC*         : Common 3.3v logic chips
      ADM485       : Analog Devices ADM485 Low Power EIA RS485 transceiver (SOIC8)
      61C256       : ISSI IS61C256AH-15J 32k x8-bit SRAM (SOJ28)
      LT1181A      : Linear Technology LT1181A or Analog Devices ADM202EARW Low Power 5V RS232 Dual Driver/Receiver (SOIC16W)
      MC14052      : OnSemi MC14052 Analog DP4T Multiplexers/Demultiplexer (SOIC16)
      L            : LED (SMD 0603)
      J1           : 48-Way Card Edge Connector
      J2           : USB Connector for JVS External I/O board
      J3           : Dual RCA Jacks marked 'AUDIO', for audio output
      J4/J5        : HD15F DSUB Connector marked 'CRT1/CRT2', for video output
      J6           : DB9 DSUB Connector marked 'RS232C'. Possibly for networking several PCBs together
      J7           : 6 pin header for programming the CPLDs via JTAG
      J8           : 2 pin header for connection of gun. Pin 1:Player 1 Gun Opto. Pin2:Player 2 Gun Opto

This PCB is required by Point Blank 3 since it controls the gun opto signal. Only the CPLD and J8 and some minor
logic and other small support parts are populated.
This PCB has been found almost fully populated (minus J6) on some Taiko no Tatsujin games (TK51/TK61) (and on Knock Down 2001) but not
earlier TK games, so it appears to be optional or is only used by the later TK51 and TK61 games.
*/

#include "emu.h"
#include "ns10crypt.h"

#include "cpu/psx/psx.h"
#include "cpu/tlcs900/tmp95c061.h"
#include "machine/ram.h"
//#include "sound/spu.h"
#include "video/psx.h"
#include "screen.h"
#include "speaker.h"


namespace {

class namcos10_state : public driver_device
{
public:
	namcos10_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_exio_mcu(*this, "exio_mcu")
		, m_memp3_mcu(*this, "memp3_mcu")
		, decrypter(*this, "decrypter")
	{ }

	void namcos10_base(machine_config &config);
	void namcos10_memm(machine_config &config);
	void namcos10_memn(machine_config &config);
	void namcos10_memn_exio(machine_config &config);
	void namcos10_memp3(machine_config &config);
	void ns10_konotako(machine_config &config);
	void ns10_mrdrilr2(machine_config &config);
	void ns10_knpuzzle(machine_config &config);
	void ns10_chocovdr(machine_config &config);
	void ns10_startrgn(machine_config &config);
	void ns10_gjspace(machine_config &config);
	void ns10_nflclsfb(machine_config &config);
	void ns10_gamshara(machine_config &config);

	void init_knpuzzle();
	void init_panikuru();
	void init_mrdrilr2();
	void init_startrgn();
	void init_gunbalna();
	void init_nflclsfb();
	void init_gjspace();
	void init_gamshara();
	void init_mrdrilrg();
	void init_chocovdr();
	void init_konotako();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	// memm variant interface
	void crypto_switch_w(uint16_t data);
	uint16_t range_r(offs_t offset);
	void bank_w(offs_t offset, uint16_t data);

	// memn variant interface
	uint16_t nand_status_r();
	void nand_address1_w(uint8_t data);
	void nand_address2_w(uint8_t data);
	void nand_address3_w(uint8_t data);
	void nand_address4_w(uint8_t data);
	uint16_t nand_data_r();
	void nand_block_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t nand_block_r(offs_t offset);

	uint16_t control_r(offs_t offset);
	void control_w(offs_t offset, uint16_t data);

	uint16_t i2c_clock_r();
	void i2c_clock_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t i2c_data_r();
	void i2c_data_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t sprot_r();
	void sprot_w(uint16_t data);

	uint8_t *nand_base = nullptr;
	void nand_copy( uint32_t *dst, uint32_t address, int len );

	void namcos10_map(address_map &map);
	void namcos10_memm_map(address_map &map);
	void namcos10_memn_map(address_map &map);

	enum {
		I2CP_IDLE,
		I2CP_RECIEVE_BYTE,
		I2CP_RECIEVE_ACK_1,
		I2CP_RECIEVE_ACK_0
	};

	uint32_t bank_base = 0;
	uint32_t nand_address = 0;
	uint16_t block[0x1ff]{};

	uint16_t i2c_host_clock = 0, i2c_host_data = 0, i2c_dev_clock = 0, i2c_dev_data = 0, i2c_prev_clock = 0, i2c_prev_data = 0;
	int i2cp_mode = 0;
	uint8_t i2c_byte = 0;
	int i2c_bit = 0;

	int sprot_bit = 0, sprot_byte = 0;
	uint16_t nand_read( uint32_t address );
	uint16_t nand_read2( uint32_t address );

	void i2c_update();

	void memn_driver_init(  );
	required_device<psxcpu_device> m_maincpu;
	optional_device<tmp95c061_device> m_exio_mcu;
	optional_device<tmp95c061_device> m_memp3_mcu;
	optional_device<ns10_decrypter_device> decrypter;
};


void namcos10_state::machine_start()
{
	nand_address = 0;
}


void namcos10_state::namcos10_map(address_map &map)
{
	map(0x1f500000, 0x1f501fff).ram().share("share3"); /* ram? stores block numbers */
	map(0x9f500000, 0x9f501fff).ram().share("share3"); /* ram? stores block numbers */
	map(0xbf500000, 0xbf501fff).ram().share("share3"); /* ram? stores block numbers */

	map(0x1fba0000, 0x1fba000f).rw(FUNC(namcos10_state::control_r), FUNC(namcos10_state::control_w));
	map(0x1fba0002, 0x1fba0003).rw(FUNC(namcos10_state::sprot_r), FUNC(namcos10_state::sprot_w));
	map(0x1fba0008, 0x1fba0009).rw(FUNC(namcos10_state::i2c_clock_r), FUNC(namcos10_state::i2c_clock_w));
	map(0x1fba000a, 0x1fba000b).rw(FUNC(namcos10_state::i2c_data_r), FUNC(namcos10_state::i2c_data_w));
}


// memm variant interface
//
// banked view with dynamic decryption over the flash.  Access to the
// nand is unknown, but may just be using the memn interface.  Won't
// know until the decryption is done.
//
// bios copies 62000-37ffff from the flash to 80012000 in ram through the
// decryption in range_r then jumps there

void namcos10_state::crypto_switch_w(uint16_t data)
{
	printf("crypto_switch_w: %04x\n", data);
	if (!decrypter.found())
		return;

	if (BIT(data, 15) != 0)
		decrypter->activate(data & 0xf);
	else
		decrypter->deactivate();
}

void namcos10_state::bank_w(offs_t offset, uint16_t data)
{
	bank_base = 0x100000 * offset;
}

uint16_t namcos10_state::range_r(offs_t offset)
{
	uint16_t data = ((const uint16_t *)(memregion("maincpu:rom")->base()))[bank_base+offset];

	if (!decrypter.found())
		return data;

	if (decrypter->is_active())
		return decrypter->decrypt(data);
	else
		return data;
}

uint16_t namcos10_state::control_r(offs_t offset)
{
	logerror("%s: control_r %d (%x)\n", machine().describe_context(), offset);
	if(offset == 2)
		return 1^0xffff;
	return 0;
}

void namcos10_state::control_w(offs_t offset, uint16_t data)
{
	logerror("%s: control_w %d, %04x (%x)\n", machine().describe_context(), offset, data);
}

void namcos10_state::sprot_w(uint16_t data)
{
	logerror("%s: sprot_w %04x (%x)\n", machine().describe_context(), data);
	sprot_bit = 7;
	sprot_byte = 0;
}

//   startrgn:
// 8004b6f8: jal 4b730, dies if v0!=0 (answers 1)
// flash access, unhappy with the results

// 800128d8: jal 37b58 (flash death)
// 800128e0: jal 1649c
// 800128e8: jal 2c47c

uint16_t namcos10_state::sprot_r()
{
	// If line 3 has 0x30/0x31 in it, something happens.  That
	// something currently kills the system though.

	const static uint8_t prot[0x40] = {
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
		0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
		0x50, 0x51, 0x50, 0x51, 0x50, 0x51, 0x50, 0x51, 0x50, 0x51, 0x50, 0x51, 0x50, 0x51, 0x50, 0x51,
		0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	};
	uint16_t res = sprot_byte >= 0x20 ? 0x3 :
		(((prot[sprot_byte     ] >> sprot_bit) & 1) ? 1 : 0) |
		(((prot[sprot_byte+0x20] >> sprot_bit) & 1) ? 2 : 0);

	sprot_bit--;
	if(sprot_bit == -1) {
		sprot_bit = 7;
		sprot_byte++;
	}
	return res;
}

uint16_t namcos10_state::i2c_clock_r()
{
	uint16_t res = i2c_dev_clock & i2c_host_clock & 1;
	//  logerror("i2c_clock_r %d (%x)\n", res, m_maincpu->pc());
	return res;
}


void namcos10_state::i2c_clock_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&i2c_host_clock);
	//  logerror("i2c_clock_w %d (%x)\n", data, m_maincpu->pc());
	i2c_update();
}

uint16_t namcos10_state::i2c_data_r()
{
	uint16_t res = i2c_dev_data & i2c_host_data & 1;
	//  logerror("i2c_data_r %d (%x)\n", res, m_maincpu->pc());
	return res;
}


void namcos10_state::i2c_data_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&i2c_host_data);
	//  logerror("i2c_data_w %d (%x)\n", data, m_maincpu->pc());
	i2c_update();
}

void namcos10_state::i2c_update()
{
	uint16_t clock = i2c_dev_clock & i2c_host_clock & 1;
	uint16_t data = i2c_dev_data & i2c_host_data & 1;

	if(i2c_prev_data == data && i2c_prev_clock == clock)
		return;

	switch(i2cp_mode) {
	case I2CP_IDLE:
		if(clock && !data) {
			logerror("i2c: start bit\n");
			i2c_byte = 0;
			i2c_bit = 7;
			i2cp_mode = I2CP_RECIEVE_BYTE;
		}
		break;
	case I2CP_RECIEVE_BYTE:
		if(clock && data && !i2c_prev_data) {
			logerror("i2c stop bit\n");
			i2cp_mode = I2CP_IDLE;
		} else if(clock && !i2c_prev_clock) {
			i2c_byte |= (data << i2c_bit);
			//          logerror("i2c_byte = %02x (%d)\n", i2c_byte, i2c_bit);
			i2c_bit--;
			if(i2c_bit < 0) {
				i2cp_mode = I2CP_RECIEVE_ACK_1;
				logerror("i2c received byte %02x\n", i2c_byte);
				i2c_dev_data = 0;
				data = 0;
			}
		}
		break;
	case I2CP_RECIEVE_ACK_1:
		if(clock && !i2c_prev_clock) {
			//          logerror("i2c ack on\n");
			i2cp_mode = I2CP_RECIEVE_ACK_0;
		}
		break;
	case I2CP_RECIEVE_ACK_0:
		if(!clock && i2c_prev_clock) {
			//          logerror("i2c ack off\n");
			i2c_dev_data = 1;
			data = i2c_host_data & 1;
			i2c_byte = 0;
			i2c_bit = 7;
			i2cp_mode = I2CP_RECIEVE_BYTE;
		}
		break;
	}
	i2c_prev_data = data;
	i2c_prev_clock = clock;
}

void namcos10_state::namcos10_memm_map(address_map &map)
{
	namcos10_map(map);

	map(0x1f300000, 0x1f300001).w(FUNC(namcos10_state::crypto_switch_w));
	map(0x1f400000, 0x1f5fffff).r(FUNC(namcos10_state::range_r));
	map(0x1fb40000, 0x1fb4000f).w(FUNC(namcos10_state::bank_w));
}


// memn variant interface
//
// Block access to the nand.  Something strange is going on with the
// status port.  Interaction with the decryption is unclear at best.

uint16_t namcos10_state::nand_status_r()
{
	return 0;
}

void namcos10_state::nand_address1_w(uint8_t data)
{
	logerror("%s: nand_a1_w %08x (%08x)\n", machine().describe_context(), data);
	//  nand_address = ( nand_address & 0x00ffffff ) | ( data << 24 );
}

void namcos10_state::nand_address2_w(uint8_t data)
{
	logerror("%s: nand_a2_w %08x (%08x)\n", machine().describe_context(), data);
	nand_address = ( nand_address & 0xffffff00 ) | ( data << 0 );
}

void namcos10_state::nand_address3_w(uint8_t data)
{
	logerror("%s: nand_a3_w %08x (%08x)\n", machine().describe_context(), data);
	nand_address = ( nand_address & 0xffff00ff ) | ( data <<  8 );
}

void namcos10_state::nand_address4_w(uint8_t data)
{
	nand_address = ( nand_address & 0xff00ffff ) | ( data << 16 );
	logerror("%s: nand_a4_w %08x (%08x) -> %08x\n", machine().describe_context(), data, nand_address*2);
}

uint16_t namcos10_state::nand_read( uint32_t address )
{
	int index = ( ( address / 512 ) * 528 ) + ( address % 512 );
	return nand_base[ index ] | ( nand_base[ index + 1 ] << 8 );
}

uint16_t namcos10_state::nand_read2( uint32_t address )
{
	int index = ( ( address / 512 ) * 528 ) + ( address % 512 );
	return nand_base[ index + 1 ] | ( nand_base[ index ] << 8 );
}

uint16_t namcos10_state::nand_data_r()
{
	uint16_t data = nand_read2( nand_address * 2 );

	//  logerror("read %08x = %04x\n", nand_address*2, data);
	// printf("read %08x = %04x\n", nand_address*2, data);


/*  printf( "data<-%08x (%08x)\n", data, nand_address ); */
	nand_address++;

	if (!decrypter.found())
		return data;

	if (decrypter->is_active())
		return decrypter->decrypt(data);
	else
		return data;
}

void namcos10_state::nand_copy( uint32_t *dst, uint32_t address, int len )
{
	while( len > 0 )
	{
		*( dst++ ) = nand_read( address ) | ( nand_read( address + 2 ) << 16 );
		address += 4;
		len -= 4;
	}
}

void namcos10_state::nand_block_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA( &block[offset] );
}

uint16_t namcos10_state::nand_block_r(offs_t offset)
{
	return block[ offset ];
}

void namcos10_state::namcos10_memn_map(address_map &map)
{
	namcos10_map(map);

	map(0x1f300000, 0x1f300001).w(FUNC(namcos10_state::crypto_switch_w));
	map(0x1f380000, 0x1f380001).w(FUNC(namcos10_state::crypto_switch_w));
	map(0x1f400000, 0x1f400001).r(FUNC(namcos10_state::nand_status_r));
	map(0x1f410000, 0x1f410000).w(FUNC(namcos10_state::nand_address1_w));
	map(0x1f420000, 0x1f420000).w(FUNC(namcos10_state::nand_address2_w));
	map(0x1f430000, 0x1f430000).w(FUNC(namcos10_state::nand_address3_w));
	map(0x1f440000, 0x1f440000).w(FUNC(namcos10_state::nand_address4_w));
	map(0x1f450000, 0x1f450001).r(FUNC(namcos10_state::nand_data_r));
	map(0x1fb60000, 0x1fb60001).rw(FUNC(namcos10_state::nand_block_r), FUNC(namcos10_state::nand_block_w));
}

void namcos10_state::memn_driver_init(  )
{
	uint8_t *BIOS = (uint8_t *)memregion( "maincpu:rom" )->base();
	nand_base = (uint8_t *)memregion( "user2" )->base();

	nand_copy( (uint32_t *)( BIOS + 0x0000000 ), 0x08000, 0x001c000 );
	nand_copy( (uint32_t *)( BIOS + 0x0020000 ), 0x24000, 0x03e0000 );
}

static void decrypt_bios( running_machine &machine, const char *regionName, int start, int end, int b15, int b14, int b13, int b12, int b11, int b10, int b9, int b8,
	int b7, int b6, int b5, int b4, int b3, int b2, int b1, int b0 )
{
	memory_region *region = machine.root_device().memregion( regionName );
	uint16_t *BIOS = (uint16_t *)( region->base() + start );
	int len = (end - start) / 2;

	for( int i = 0; i < len; i++ )
	{
		BIOS[ i ] = bitswap<16>( BIOS[ i ] ^ 0xaaaa,
			b15, b14, b13, b12, b11, b10, b9, b8, b7, b6, b5, b4, b3, b2, b1, b0 );
	}
}

void namcos10_state::init_mrdrilr2()
{
	int regSize = memregion("maincpu:rom")->bytes();

	decrypt_bios(machine(), "maincpu:rom", 0, 0x62000, 0xc, 0xd, 0xf, 0xe, 0xb, 0xa, 0x9, 0x8, 0x7, 0x6, 0x4, 0x1, 0x2, 0x5, 0x0, 0x3);
	decrypt_bios(machine(), "maincpu:rom", 0x380000, regSize, 0xc, 0xd, 0xf, 0xe, 0xb, 0xa, 0x9, 0x8, 0x7, 0x6, 0x4, 0x1, 0x2, 0x5, 0x0, 0x3);
}

void namcos10_state::init_gjspace()
{
	int regSize = memregion("user2")->bytes();
	decrypt_bios(machine(), "user2", 0x0008400, 0x0029400, 0x0, 0x2, 0xe, 0xd, 0xf, 0x6, 0xc, 0x7, 0x5, 0x1, 0x9, 0x8, 0xa, 0x3, 0x4, 0xb);
	decrypt_bios(machine(), "user2", 0x0210000, 0x104e800, 0x0, 0x2, 0xe, 0xd, 0xf, 0x6, 0xc, 0x7, 0x5, 0x1, 0x9, 0x8, 0xa, 0x3, 0x4, 0xb);
	decrypt_bios(machine(), "user2", 0x1077c00, regSize, 0x0, 0x2, 0xe, 0xd, 0xf, 0x6, 0xc, 0x7, 0x5, 0x1, 0x9, 0x8, 0xa, 0x3, 0x4, 0xb);
	memn_driver_init();
}

void namcos10_state::init_mrdrilrg()
{
	int regSize = memregion("user2")->bytes();
	decrypt_bios(machine(), "user2", 0x8400, regSize, 0x6, 0x4, 0x7, 0x5, 0x2, 0x1, 0x0, 0x3, 0xc, 0xd, 0xe, 0xf, 0x8, 0x9, 0xb, 0xa);
	memn_driver_init();
}

void namcos10_state::init_knpuzzle()
{
	int regSize = memregion("user2")->bytes();
	decrypt_bios(machine(), "user2", 0x0008400, 0x0029400, 0x6, 0x7, 0x4, 0x5, 0x2, 0x0, 0x3, 0x1, 0xc, 0xd, 0xe, 0xf, 0x9, 0xb, 0x8, 0xa);
	decrypt_bios(machine(), "user2", 0x047ac00, 0x1042200, 0x6, 0x7, 0x4, 0x5, 0x2, 0x0, 0x3, 0x1, 0xc, 0xd, 0xe, 0xf, 0x9, 0xb, 0x8, 0xa);
	decrypt_bios(machine(), "user2", 0x104a600, regSize  , 0x6, 0x7, 0x4, 0x5, 0x2, 0x0, 0x3, 0x1, 0xc, 0xd, 0xe, 0xf, 0x9, 0xb, 0x8, 0xa);
	memn_driver_init();
}

void namcos10_state::init_startrgn()
{
	int regSize = memregion("user2")->bytes();
	decrypt_bios(machine(), "user2", 0x0008400, 0x0029400, 0x6, 0x5, 0x4, 0x7, 0x1, 0x3, 0x0, 0x2, 0xc, 0xd, 0xe, 0xf, 0x8, 0xb, 0xa, 0x9);
	decrypt_bios(machine(), "user2", 0x00b9a00, 0x105ae00, 0x6, 0x5, 0x4, 0x7, 0x1, 0x3, 0x0, 0x2, 0xc, 0xd, 0xe, 0xf, 0x8, 0xb, 0xa, 0x9);
	decrypt_bios(machine(), "user2", 0x1080000, regSize  , 0x6, 0x7, 0x4, 0x5, 0x0, 0x1, 0x3, 0x2, 0xd, 0xc, 0xf, 0xe, 0x8, 0x9, 0xb, 0xa);
	memn_driver_init();
}

void namcos10_state::init_gamshara()
{
	int regSize = memregion("user2")->bytes();
	decrypt_bios(machine(), "user2", 0x0008400, 0x0029400, 0x5, 0x4, 0x7, 0x6, 0x0, 0x1, 0x3, 0x2, 0xd, 0xf, 0xc, 0xe, 0x8, 0x9, 0xa, 0xb);
	decrypt_bios(machine(), "user2", 0x014e200, 0x105ae00, 0x5, 0x4, 0x7, 0x6, 0x0, 0x1, 0x3, 0x2, 0xd, 0xf, 0xc, 0xe, 0x8, 0x9, 0xa, 0xb);
	decrypt_bios(machine(), "user2", 0x1080000, regSize  , 0x5, 0x4, 0x7, 0x6, 0x0, 0x1, 0x3, 0x2, 0xd, 0xf, 0xc, 0xe, 0x8, 0x9, 0xa, 0xb);
	memn_driver_init();
}

void namcos10_state::init_gunbalna()
{
	int regSize = memregion("user2")->bytes();
	decrypt_bios(machine(), "user2", 0x8400, regSize, 0x5, 0x4, 0x7, 0x6, 0x0, 0x1, 0x3, 0x2, 0xd, 0xf, 0xc, 0xe, 0x9, 0x8, 0xa, 0xb);
	memn_driver_init();
}

void namcos10_state::init_chocovdr()
{
	int regSize = memregion("user2")->bytes();
	decrypt_bios(machine(), "user2", 0x0008400, 0x0029400, 0x5, 0x4, 0x6, 0x7, 0x1, 0x0, 0x2, 0x3, 0xc, 0xf, 0xe, 0xd, 0x8, 0xb, 0xa, 0x9);
	decrypt_bios(machine(), "user2", 0x01eae00, 0x105ae00, 0x5, 0x4, 0x6, 0x7, 0x1, 0x0, 0x2, 0x3, 0xc, 0xf, 0xe, 0xd, 0x8, 0xb, 0xa, 0x9);
	decrypt_bios(machine(), "user2", 0x1080000, regSize  , 0x5, 0x4, 0x6, 0x7, 0x1, 0x0, 0x2, 0x3, 0xc, 0xf, 0xe, 0xd, 0x8, 0xb, 0xa, 0x9);
	memn_driver_init();
}

void namcos10_state::init_panikuru()
{
	int regSize = memregion("user2")->bytes();
	decrypt_bios(machine(), "user2", 0x8400, regSize, 0x6, 0x4, 0x7, 0x5, 0x0, 0x1, 0x2, 0x3, 0xc, 0xf, 0xe, 0xd, 0x9, 0x8, 0xb, 0xa);
	memn_driver_init();
}

void namcos10_state::init_nflclsfb()
{
	int regSize = memregion("user2")->bytes();
	decrypt_bios(machine(), "user2", 0x0008400, 0x0029400, 0x6, 0x5, 0x4, 0x7, 0x1, 0x3, 0x0, 0x2, 0xc, 0xd, 0xe, 0xf, 0x8, 0xb, 0xa, 0x9);
	decrypt_bios(machine(), "user2", 0x0214200, 0x105ae00, 0x6, 0x5, 0x4, 0x7, 0x1, 0x3, 0x0, 0x2, 0xc, 0xd, 0xe, 0xf, 0x8, 0xb, 0xa, 0x9);
	decrypt_bios(machine(), "user2", 0x1080000, regSize  , 0x6, 0x5, 0x4, 0x7, 0x1, 0x3, 0x0, 0x2, 0xc, 0xd, 0xe, 0xf, 0x8, 0xb, 0xa, 0x9);
	memn_driver_init();
}

void namcos10_state::init_konotako()
{
	int regSize = memregion("user2")->bytes();
	decrypt_bios(machine(), "user2", 0x0008400, 0x0029400, 0x6, 0x7, 0x4, 0x5, 0x0, 0x1, 0x3, 0x2, 0xd, 0xc, 0xf, 0xe, 0x8, 0x9, 0xb, 0xa);
	decrypt_bios(machine(), "user2", 0x00b9a00, 0x105ae00, 0x6, 0x7, 0x4, 0x5, 0x0, 0x1, 0x3, 0x2, 0xd, 0xc, 0xf, 0xe, 0x8, 0x9, 0xb, 0xa);
	decrypt_bios(machine(), "user2", 0x1080000, regSize  , 0x6, 0x7, 0x4, 0x5, 0x0, 0x1, 0x3, 0x2, 0xd, 0xc, 0xf, 0xe, 0x8, 0x9, 0xb, 0xa);
	memn_driver_init();
}


void namcos10_state::machine_reset()
{
	i2c_dev_clock = i2c_dev_data = 1;
	i2c_host_clock = i2c_host_data = 1;
	i2c_prev_clock = i2c_prev_data = 1;
	i2cp_mode = I2CP_IDLE;
	i2c_byte = 0x00;
	i2c_bit = 0;
}

void namcos10_state::namcos10_base(machine_config &config)
{
	/* basic machine hardware */
	CXD8606BQ(config, m_maincpu, XTAL(101'491'200));
	m_maincpu->set_disable_rom_berr(true);
	m_maincpu->subdevice<ram_device>("ram")->set_default_size("16M"); // ->set_default_size("4M"); 2 IS41LV16100s
	// The bios first configures the ROM window as 80000-big, then
	// switches to 400000.  If berr is active, the first configuration
	// wipes all handlers after 1fc80000, which kills the system
	// afterwards

	/* video hardware */
	CXD8561CQ(config, "gpu", XTAL(53'693'175), 0x200000, subdevice<psxcpu_device>("maincpu")).set_screen("screen"); // 2 54V25632s

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	// CXD2938Q; SPU with CD-ROM controller - also seen in PSone, 101.4912MHz / 3?
}

void namcos10_state::namcos10_memm(machine_config &config)
{
	namcos10_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &namcos10_state::namcos10_memm_map);
}

void namcos10_state::namcos10_memn(machine_config &config)
{
	namcos10_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &namcos10_state::namcos10_memn_map);
}

void namcos10_state::namcos10_memn_exio(machine_config &config)
{
	namcos10_memn(config);
	TMP95C061(config, m_exio_mcu, XTAL(22'118'400)).set_disable(); // not hooked up
}

void namcos10_state::namcos10_memp3(machine_config &config)
{
	namcos10_memn(config);
	TMP95C061(config, m_memp3_mcu, XTAL(16'934'400)).set_disable(); // not hooked up
	// LC82310 16.9344MHz
}

void namcos10_state::ns10_mrdrilr2(machine_config &config)
{
	namcos10_memm(config);
	/* decrypter device (CPLD in hardware?) */
	MRDRILR2_DECRYPTER(config, "decrypter", 0);
}

void namcos10_state::ns10_chocovdr(machine_config &config)
{
	namcos10_memn(config);
	/* decrypter device (CPLD in hardware?) */
	CHOCOVDR_DECRYPTER(config, "decrypter", 0);
}

void namcos10_state::ns10_gamshara(machine_config &config)
{
	namcos10_memn(config);
	/* decrypter device (CPLD in hardware?) */
	GAMSHARA_DECRYPTER(config, "decrypter", 0);
}

void namcos10_state::ns10_gjspace(machine_config &config)
{
	namcos10_memn(config);
	/* decrypter device (CPLD in hardware?) */
	GJSPACE_DECRYPTER(config, "decrypter", 0);
}

void namcos10_state::ns10_knpuzzle(machine_config &config)
{
	namcos10_memn(config);
	/* decrypter device (CPLD in hardware?) */
	KNPUZZLE_DECRYPTER(config, "decrypter", 0);
}

void namcos10_state::ns10_konotako(machine_config &config)
{
	namcos10_memn(config);
	/* decrypter device (CPLD in hardware?) */
	KONOTAKO_DECRYPTER(config, "decrypter", 0);
}

void namcos10_state::ns10_nflclsfb(machine_config &config)
{
	namcos10_memn(config);
	/* decrypter device (CPLD in hardware?) */
	NFLCLSFB_DECRYPTER(config, "decrypter", 0);
}

void namcos10_state::ns10_startrgn(machine_config &config)
{
	namcos10_memn(config);
	/* decrypter device (CPLD in hardware?) */
	STARTRGN_DECRYPTER(config, "decrypter", 0);
}

static INPUT_PORTS_START( namcos10 )
	/* IN 0 */
	PORT_START("SYSTEM")
	PORT_SERVICE_NO_TOGGLE( 0x8000, IP_ACTIVE_HIGH )
	PORT_BIT( 0x7fff, IP_ACTIVE_HIGH, IPT_UNUSED )

	/* IN 1 */
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON5 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON6 )
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )

	/* IN 2 */
	PORT_START("P2")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_SERVICE2 )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )

	/* IN 3 */
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
INPUT_PORTS_END


ROM_START( mrdrilr2 )
	ROM_REGION32_LE( 0x800000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD( "dr21vera.1a",  0x000000, 0x800000, CRC(f93532a2) SHA1(8b72f2868978be1f0e0abd11425a3c8b2b0c4e99) )

	ROM_REGION( 0x4000000, "user2", 0 ) /* main prg */
	ROM_LOAD( "dr21ma1.1d", 0x0000000, 0x1000000, CRC(26dc6f55) SHA1(a9cedf547fa7a4d5850b9b3b867d46e577a035e0) )
	ROM_LOAD( "dr21ma2.2d", 0x1000000, 0x1000000, CRC(702556ff) SHA1(c95defd5fd6a9b406fc8d8f28ecfab732ef1ff42) )
ROM_END

ROM_START( mrdrlr2a )
	ROM_REGION32_LE( 0x800000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD( "dr22vera.1a",  0x000000, 0x800000, CRC(f2633388) SHA1(42e56c9758ee833390003d4b41956f75f5a22760) )

	ROM_REGION( 0x4000000, "user2", 0 ) /* main prg */
	ROM_LOAD( "dr21ma1.1d", 0x0000000, 0x1000000, CRC(26dc6f55) SHA1(a9cedf547fa7a4d5850b9b3b867d46e577a035e0) )
	ROM_LOAD( "dr21ma2.2d", 0x1000000, 0x1000000, CRC(702556ff) SHA1(c95defd5fd6a9b406fc8d8f28ecfab732ef1ff42) )
ROM_END

ROM_START( gjspace )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x4200000, "user2", 0 ) /* main prg */
	ROM_LOAD( "10011a_0.bin", 0x0000000, 0x1080000, CRC(df862033) SHA1(4141357ed315adb4de636d7bf752354e953e8cbf) )
	ROM_LOAD( "10011a_1.bin", 0x1080000, 0x1080000, CRC(734c7ac0) SHA1(2f325236a4e4f2dba886682e9a7e8e243b5fbb3d) )
	ROM_LOAD( "10011a_2.bin", 0x2100000, 0x1080000, CRC(3bbbc0b7) SHA1(ad02ec2e5f401f0f5d40a413038649ebd25d5343) )
	ROM_LOAD( "10011a_3.bin", 0x3180000, 0x1080000, CRC(fb0de5ca) SHA1(50a462a52ff4a0bc112b9d89f2b2d032c60cf59c) )
ROM_END


ROM_START( g13jnc )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x6300000, "user2", 0 ) /* main prg */
	ROM_LOAD( "glt1_ver.a.0", 0x0000000, 0x1080000, CRC(e60f78d3) SHA1(5c876ac7366b5c46b5229a6b6f694ad222f36195) )
	ROM_LOAD( "glt1_ver.a.1", 0x1080000, 0x1080000, CRC(c3f31dd9) SHA1(05e6d39f33191979bcc00a585b64904a077000dc) )
	ROM_LOAD( "glt1_ver.a.2", 0x2100000, 0x1080000, CRC(e464e03a) SHA1(751f6bd753dacbb881fb47bc1b146ef59245bd10) )
	ROM_LOAD( "glt1_ver.a.3", 0x3180000, 0x1080000, CRC(f7486979) SHA1(a44c33ae7004e79fe66c6d2cba3d11671ce2582c) )
	ROM_LOAD( "glt1_ver.a.4", 0x4200000, 0x1080000, CRC(e39969b4) SHA1(3348839c0cc4a4bcaa7803ef22981420c527e1a4) )
	ROM_LOAD( "glt1_ver.a.5", 0x5280000, 0x1080000, CRC(a82800b4) SHA1(ce4cc479acdf7ac5a7237d07422ea3ee580d899a) )
ROM_END

ROM_START( mrdrilrg )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x3180000, "user2", 0 ) /* main prg */
	ROM_LOAD( "drg1a_0.bin",  0x0000000, 0x1080000, CRC(e0801878) SHA1(fbb771c1e76e0690f6dffed2287eb470b561ec20) )
	ROM_LOAD( "drg1a_1.bin",  0x1080000, 0x1080000, CRC(4d8cde73) SHA1(62a5fab8be8fd0a6bfeb101020d4cf58866a757c) )
	ROM_LOAD( "drg1a_2.bin",  0x2100000, 0x1080000, CRC(ccfabf7b) SHA1(0cbd91ce8abd6efca5d427b52279ce265f685aa9) )
ROM_END

ROM_START( mrdrilrga )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x3180000, "user2", 0 ) /* main prg */
	ROM_LOAD( "0.8e",         0x0000000, 0x1080000, CRC(def72bcd) SHA1(e243b7ef23b2b00612c185e01493cd01be51f154) )
	ROM_LOAD( "1.8d",         0x1080000, 0x1080000, CRC(c87b5e86) SHA1(b034210da30e1f2f7d04f77e00bf7724437e2024) )
	ROM_LOAD( "2.7e",         0x2100000, 0x1080000, CRC(e0a9339f) SHA1(4284e7233876cfaf8021440d78ccc8c70d00cc00) )
ROM_END

ROM_START( knpuzzle )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x3180000, "user2", 0 ) /* main prg */
	ROM_LOAD( "kpm1a_0.bin",  0x0000000, 0x1080000, CRC(b2947eb8) SHA1(fa941bf3598bb25d2c8f0a93154e32bf78a6507c) )
	ROM_LOAD( "kpm1a_1.bin",  0x1080000, 0x1080000, CRC(f3aa855a) SHA1(87b94e22db4bc4169324bbff93c4ea19c1d99b40) )
	ROM_LOAD( "kpm1a_2.bin",  0x2100000, 0x1080000, CRC(b297cc8d) SHA1(c3494e7a8a0b4e0c8c40b99121373effbfe848eb) )
ROM_END

ROM_START( startrgn )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x2100000, "user2", 0 ) /* main prg */
	ROM_LOAD( "stt1a_0.bin",  0x0000000, 0x1080000, CRC(1e090644) SHA1(a7a293e2bd9eea2eb64a492a47272d9d9ee2c724) )
	ROM_LOAD( "stt1a_1.bin",  0x1080000, 0x1080000, CRC(aa527694) SHA1(a25dcbeca58a1443070848b3487a24d51d41a34b) )
ROM_END

ROM_START( gamshara )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x2100000, "user2", 0 ) /* main prg */
	ROM_LOAD( "10021a_e.8e",  0x0000000, 0x1080000, CRC(684ab324) SHA1(95c2e0a04c4f33039535fc451c5559d239b8fbc6) )
	ROM_LOAD( "10021a.8d",    0x1080000, 0x1080000, CRC(73669ff7) SHA1(eb8bbf931f1f8a049208d081d040512a3ffa9c00) )
ROM_END

ROM_START( gamsharaj )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x2100000, "user2", 0 ) /* main prg */
	ROM_LOAD( "10021a.8e",    0x0000000, 0x1080000, CRC(6c0361fc) SHA1(7debf1f2e6bed31d59fb224a78a17a94fc573785) )
	ROM_LOAD( "10021a.8d",    0x1080000, 0x1080000, CRC(73669ff7) SHA1(eb8bbf931f1f8a049208d081d040512a3ffa9c00) )
ROM_END

ROM_START( ptblank3 )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x2100000, "user2", 0 ) /* main prg */
	// protection issues preventing the last NAND block to be dumped
	ROM_LOAD("gnn2a.8e", 0x0000000, 0x1080000, BAD_DUMP CRC(31b39221) SHA1(7fcb14aaa26c531928a6cd704e746d0e3ae3e031))
	ROM_LOAD( "gnn2a.8d",         0x1080000, 0x1080000, BAD_DUMP CRC(82d2cfb5) SHA1(4b5e713a55e74a7b32b1b9b5811892df2df86256) )
ROM_END

ROM_START( gunbalina )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x2100000, "user2", 0 ) /* main prg */
	// protection issues preventing the last NAND block to be dumped
	ROM_LOAD( "gnn1a.8e",         0x0000000, 0x1080000, BAD_DUMP CRC(981b03d4) SHA1(1c55458f1b2964afe2cf4e9d84548c0699808e9f) )
	ROM_LOAD( "gnn1a.8d",         0x1080000, 0x1080000, BAD_DUMP CRC(6cd343e0) SHA1(dcec44abae1504025895f42fe574549e5010f7d5) )
ROM_END

ROM_START( chocovdr )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x5280000, "user2", 0 ) /* main prg */
	ROM_LOAD( "0.8e",         0x0000000, 0x1080000, CRC(f36eebb5) SHA1(a0464186b247b28f37005ffd9e9b7370145f67ef) )
	ROM_LOAD( "1.8d",         0x1080000, 0x1080000, CRC(4aecd6fc) SHA1(31fe8f36e38020a92f15c44fd1a4b486636b40ce) )
	ROM_LOAD( "2.7e",         0x2100000, 0x1080000, CRC(ac212e5a) SHA1(f2d2e65a3249992730b8b90561b9bcf5eaaafb88) )
	ROM_LOAD( "3.7d",         0x3180000, 0x1080000, CRC(907d3d15) SHA1(20519d1f8bd9c6bc45b65e2d7444d588e922611d) )
	ROM_LOAD( "4.6e",         0x4200000, 0x1080000, CRC(1ed957dd) SHA1(bc8ce9f249fe496c130c6fe67b2260c4d0734ab9) )
ROM_END

ROM_START( panikuru )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x3180000, "user2", 0 ) /* main prg */
	ROM_LOAD( "0.8e",         0x0000000, 0x1080000, CRC(3aa66da4) SHA1(3f6ff164981e2c1825766c775442608fbf86d702) )
	ROM_LOAD( "1.8d",         0x1080000, 0x1080000, CRC(18e5135d) SHA1(a7b1533a1df71be5498718e301d1c9c548551fb4) )
	ROM_LOAD( "2.7e",         0x2100000, 0x1080000, CRC(cd3b25e0) SHA1(39dfebc59e71b8f1c28e718ee71032620f11440c) )
ROM_END

ROM_START( nflclsfb )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x4200000, "user2", 0 ) /* main prg */
	ROM_LOAD( "0.8e",         0x0000000, 0x1080000, CRC(b08d4270) SHA1(5f5dc1c2862292a9e597f6a21c0f9db2e5796ded) )
	ROM_LOAD( "1.8d",         0x1080000, 0x1080000, CRC(d3f519d8) SHA1(60d5f2fafd700e39245bed17e3cc6d608cc2c088) )
	ROM_LOAD( "2.7e",         0x2100000, 0x1080000, CRC(0c65fdc2) SHA1(fa5d41a7b10ae8f8d312b61cc6d34408123bda97) )
	ROM_LOAD( "3.7d",         0x3180000, 0x1080000, CRC(0a4e601d) SHA1(9c302a0b5aaf7046390982e62092b867c3a534a5) )
ROM_END

ROM_START( konotako )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x4200000, "user2", 0 ) /* main prg */
	ROM_LOAD( "0.8e",         0x0000000, 0x1080000, CRC(63d23a0c) SHA1(31b54119f20827ff13ecf0cd87803a5e27eaafe7) )
	ROM_LOAD( "1.8d",         0x1080000, 0x1080000, CRC(bdbed53c) SHA1(5773069c43642e6f334cee185a6fb6908eedcf4a) )
ROM_END

ROM_START( sekaikh )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x2100000, "user2", 0 ) /* main prg */
	ROM_LOAD( "0.8e",  0x0000000, 0x1080000, CRC(b0cc4a4f) SHA1(41974931901090811e07f18c04e2af853c308f88) )
	ROM_LOAD( "1.8d",  0x1080000, 0x1080000, CRC(65c4a8b4) SHA1(c7fefc32604bb47519a05cdb6c8b0f50034e0efd) )

	ROM_REGION( 0x8000, "mgexio", 0 )
	ROM_LOAD( "m48z35y.ic11", 0x0000, 0x8000, CRC(e0e52ffc) SHA1(557490e2f286773a945851f44ed0214de731cd75) )
ROM_END

ROM_START( sekaikha )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x2100000, "user2", 0 ) /* main prg */
	ROM_LOAD( "0.8e",  0x0000000, 0x1080000, CRC(e32c36ac) SHA1(d762723b6ecf65c8cb7c85c25d9a1fbbcdcfd27a) )
	ROM_LOAD( "1.8d",  0x1080000, 0x1080000, CRC(7cb38ece) SHA1(e21fbc9ff09ca51e1857e32318b95107ae4b3f0b) )

	ROM_REGION( 0x8000, "mgexio", 0 )
	ROM_LOAD( "m48z35y.ic11", 0x0000, 0x8000, CRC(e0e52ffc) SHA1(557490e2f286773a945851f44ed0214de731cd75) )
ROM_END

ROM_START( taiko6 )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x3180000, "user2", 0 ) /* main prg */
	ROM_LOAD( "k9f2808u0c.8e",         0x0000000, 0x1080000, CRC(a87352c9) SHA1(1816fabecfaa140da2cd46334701c3e2fb93258e) )
	ROM_LOAD( "k9f2808u0c.8d",         0x1080000, 0x1080000, CRC(e89aa7a3) SHA1(c34d693f4715ce930dbd105eda1ffc8379991c22) )
	ROM_LOAD( "k9f2808u0c.7e",         0x2100000, 0x1080000, CRC(098920ef) SHA1(06a689d8abb8454ed62dda92d93a8f5d756a6166) )

	DISK_REGION("cd")
	DISK_IMAGE_READONLY( "tk-6", 0, SHA1(ca8b8dfccc2022094c428b5e0b6391a77ec351f4) )
ROM_END

ROM_START( unks10md )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x2100000, "user2", 0 ) /* main prg */
	ROM_LOAD( "k9f2808u0c.8e",  0x0000000, 0x1080000, CRC(b8ce45c6) SHA1(cfc85e796e32f5f3cc16e12ce902f0ae088eea31) )
	ROM_LOAD( "k9f2808u0c.8d",  0x1080000, 0x1080000, CRC(49a2a732) SHA1(1a473177827a6d0e58c289d9af064665b941519b) )
ROM_END

ROM_START( unks10md2 )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x2100000, "user2", 0 ) /* main prg */
	ROM_LOAD( "k9f2808u0c.8e",  0x0000000, 0x1080000, CRC(53b3e255) SHA1(6e5a3addb859023d8c7e53237acf9f028c85f57b) )
	ROM_LOAD( "k9f2808u0c.8d",  0x1080000, 0x1080000, CRC(a0ad9504) SHA1(43e9e83b0340dd2e0f28ff9ccd3667db4e70951a) )
ROM_END

ROM_START( kd2001 )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x2100000, "user2", 0 ) /* main prg */
	ROM_LOAD( "0.8e",  0x0000000, 0x1080000, NO_DUMP ) // broken flash ROM, couldn't be dumped
	ROM_LOAD( "1.8d",  0x1080000, 0x1080000, CRC(2b0d0e8c) SHA1(d679e7044e1f93bb7bd449e6d8fcb7737d154025) )

	ROM_REGION( 0x20000, "pdrivecpu", 0 )
	ROM_LOAD( "kd11-dr0-ic10.bin", 0x00000, 0x20000, CRC(59649293) SHA1(71c3a0e73d077398e7f3d95acedc47814e99fbc6) )
ROM_END

ROM_START( keroro )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x4200000, "user2", 0 ) /* main prg */
	ROM_LOAD( "krg1.8e",  0x0000000, 0x2100000, CRC(12e78c66) SHA1(83573f68f27ace345a3be16f29f874f14e593233) ) // K9F5608U0D, double sized wrt to the other games and PCB silkscreen
	ROM_LOAD( "krg1.8d",  0x2100000, 0x2100000, CRC(879a87b7) SHA1(fcef8eb9423b4825bf27fbfe8cae6d4018cb534f) ) // K9F5608U0D, double sized wrt to the other games and PCB silkscreen
ROM_END

ROM_START( gegemdb )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x4200000, "user2", 0 ) /* main prg */
	ROM_LOAD( "gym1.8e",  0x0000000, 0x2100000, CRC(ea740351) SHA1(4dc7ce256a2d60be512d04a992b2103602bcfaa9) ) // K9F5608U0D, double sized wrt to the other games and PCB silkscreen
	ROM_LOAD( "gym1.8d",  0x2100000, 0x2100000, CRC(0145a8c1) SHA1(a32dd944d022df14450bbcb01b4d1712683c0680) ) // K9F5608U0D, double sized wrt to the other games and PCB silkscreen

	ROM_REGION( 0x8000, "nvram", 0 )
	ROM_LOAD( "nvram.bin", 0x0000, 0x8000, CRC(c0c87c71) SHA1(263f7f3df772644bcf973413d3fac9ae305fda6c) )
ROM_END

ROM_START( pacmball )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x2100000, "user2", 0 ) /* main prg */
	ROM_LOAD( "k9f2808u0c.8e",  0x0000000, 0x1080000, CRC(7b6f814d) SHA1(728167866d9350150b5fd9ebcf8fe7280efedb91) )
	ROM_LOAD( "k9f2808u0c.8d",  0x1080000, 0x1080000, CRC(f79d7199) SHA1(4ef9b758ee778e12f7fef717e063597299fb8219) )
ROM_END

ROM_START( puzzball )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x2100000, "user2", 0 ) /* main prg */
	ROM_LOAD( "k9f2808u0c.8e",  0x0000000, 0x1080000, CRC(ca6642a7) SHA1(550891c80feaf2c1b262f420cf90946419319640) )
	ROM_LOAD( "k9f2808u0c.8d",  0x1080000, 0x1080000, CRC(b13f6f45) SHA1(66917476de5417596a9d3b9169ea74d93f3037fe) )
ROM_END

} // Anonymous namespace


GAME( 2000, mrdrilr2,  0,        ns10_mrdrilr2,      namcos10, namcos10_state, init_mrdrilr2, ROT0, "Namco", "Mr. Driller 2 (Japan, DR21 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // PORT_4WAY joysticks
GAME( 2000, mrdrlr2a,  mrdrilr2, ns10_mrdrilr2,      namcos10, namcos10_state, init_mrdrilr2, ROT0, "Namco", "Mr. Driller 2 (World, DR22 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // PORT_4WAY joysticks
GAME( 2000, ptblank3,  0,        namcos10_memn_exio, namcos10, namcos10_state, init_gunbalna, ROT0, "Namco", "Point Blank 3 (World, GNN2 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // needs to hookup gun IO
GAME( 2000, gunbalina, ptblank3, namcos10_memn_exio, namcos10, namcos10_state, init_gunbalna, ROT0, "Namco", "Gunbalina (Japan, GNN1 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // ""
GAME( 2001, gjspace,   0,        ns10_gjspace,       namcos10, namcos10_state, init_gjspace,  ROT0, "Namco / Metro", "Gekitoride-Jong Space (10011 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2001, g13jnc,    0,        namcos10_memp3,     namcos10, namcos10_state, empty_init,    ROT0, "Eighting / Raizing / Namco", "Golgo 13: Juusei no Chinkonka (Japan, GLT1 VER.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2001, mrdrilrg,  0,        namcos10_memn,      namcos10, namcos10_state, init_mrdrilrg, ROT0, "Namco", "Mr. Driller G (Japan, DRG1 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // PORT_4WAY joysticks
GAME( 2001, mrdrilrga, mrdrilrg, namcos10_memn,      namcos10, namcos10_state, init_mrdrilrg, ROT0, "Namco", "Mr. Driller G ALT (Japan, DRG1 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // PORT_4WAY joysticks
GAME( 2001, knpuzzle,  0,        ns10_knpuzzle,      namcos10, namcos10_state, init_knpuzzle, ROT0, "Namco", "Kotoba no Puzzle Mojipittan (Japan, KPM1 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2001, kd2001,    0,        namcos10_memn,      namcos10, namcos10_state, empty_init,    ROT0, "Namco", "Knock Down 2001 (Japan, KD11 Ver. B)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
GAME( 2002, chocovdr,  0,        ns10_chocovdr,      namcos10, namcos10_state, init_chocovdr, ROT0, "Namco", "Uchuu Daisakusen: Chocovader Contactee (Japan, CVC1 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2002, startrgn,  0,        ns10_startrgn,      namcos10, namcos10_state, init_startrgn, ROT0, "Namco", "Star Trigon (Japan, STT1 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
GAME( 2002, panikuru,  0,        namcos10_memn,      namcos10, namcos10_state, init_panikuru, ROT0, "Namco", "Panicuru Panekuru (Japan, PPA1 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2002, gamshara,  0,        ns10_gamshara,      namcos10, namcos10_state, init_gamshara, ROT0, "Mitchell", "Gamshara (World, 10021 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // Ver. 20020912A ETC
GAME( 2002, gamsharaj, gamshara, ns10_gamshara,      namcos10, namcos10_state, init_gamshara, ROT0, "Mitchell", "Gamshara (Japan, 10021 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2002, puzzball,  0,        namcos10_memn,      namcos10, namcos10_state, empty_init,    ROT0, "Namco", "Puzz Ball (Japan, PZB1 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // title guessed based on known game list and PCB sticker
GAME( 2003, nflclsfb,  0,        ns10_nflclsfb,      namcos10, namcos10_state, init_nflclsfb, ROT0, "Namco", "NFL Classic Football (US, NCF3 Ver.A.)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2003, pacmball,  0,        namcos10_memn,      namcos10, namcos10_state, empty_init,    ROT0, "Namco", "Pacman BALL (PMB2 Ver.A.)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2003, konotako,  0,        ns10_konotako,      namcos10, namcos10_state, init_konotako, ROT0, "Mitchell", "Kono Tako (10021 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
GAME( 2004, sekaikh,   0,        namcos10_memn,      namcos10, namcos10_state, empty_init,    ROT0, "Namco", "Sekai Kaseki Hakken (Japan, SKH1 Ver.B)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
GAME( 2004, sekaikha,  sekaikh,  namcos10_memn,      namcos10, namcos10_state, empty_init,    ROT0, "Namco", "Sekai Kaseki Hakken (Japan, SKH1 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
GAME( 2004, taiko6,    0,        namcos10_memn,      namcos10, namcos10_state, empty_init,    ROT0, "Namco", "Taiko no Tatsujin 6 (Japan, TK61 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
GAME( 2005, unks10md2, 0,        namcos10_memn,      namcos10, namcos10_state, empty_init,    ROT0, "Namco", "unknown Namco System 10 medal game (unknown code)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND) // ROM VER. B0 FEB 09 2005 15:29:02 in test mode
GAME( 2006, keroro,    0,        namcos10_memn,      namcos10, namcos10_state, empty_init,    ROT0, "Namco", "Keroro Gunso Chikyu Shinryaku Shirei Dearimasu! (KRG1 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND) // ケロロ軍曹　地球侵略指令…であります！
GAME( 2007, gegemdb,   0,        namcos10_memn,      namcos10, namcos10_state, empty_init,    ROT0, "Namco", "Gegege no Kitaro Yokai Yokocho Matsuri De Batoru Ja (GYM1 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND) // ゲゲゲの鬼太郎　妖怪横丁まつりでバトルじゃ
GAME( 200?, unks10md,  0,        namcos10_memn,      namcos10, namcos10_state, empty_init,    ROT0, "Namco", "unknown Namco System 10 medal game (MTL1 SPR0B)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
