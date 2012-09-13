/***************************************************************************

  Konami System 573
  ===========================================================
  Driver by R. Belmont & smf

  NOTE: The first time you run each game, it will go through a special initialization
  procedure.  This can be quite lengthy (in the case of Dark Horse Legend).  Let it
  complete all the way before exiting MAME and you will not have to do it again!

  NOTE 2: The first time you run Konami 80's Gallery, it will dump you on a clock
  setting screen.  Press DOWN to select "SAVE AND EXIT" then press player 1 START
  to continue.

  Note 3: If you are asked to insert a different cartridge then use the fake dip
  switch to change it.

  Note 4: Some games require you to press f2 to skip the rtc cleared note.

  TODO:
  * integrate ATAPI code with Aaron's ATA/IDE code
  * emulate memory card board GE885-PWB(A)A ( contains Toshiba tmpr3904af, ram, rom, tranceiver and glue ).

  -----------------------------------------------------------------------------------------

  System 573 Hardware Overview
  Konami, 1998-2001

  This system uses Konami PSX-based hardware with an ATAPI CDROM drive.
  Gun Mania (and probably Gun Mania Zone Plus) has no CDROM drive.
  There is a slot for a security cart (cart is installed in CN14) and also a PCMCIA card slot.
  The main board and CDROM drive are housed in a black metal box. Gun Mania doesn't have that box.
  The games can be swapped by exchanging the CDROM disc and the security cart, whereby the main-board
  FlashROMs are re-programmed after a small wait. On subsequent power-ups, there is a check to test if the
  contents of the FlashROMs matches the CDROM, then the game boots up immediately.

  PCMCIA card slot is used by Dance Dance Revolution (2ndMIX link ver. and later),
  GUITARFREAKS (2ndMIX link ver. and later), drummania (7thMIX and later), and Gun Mania.
  DDR and GF: 32M flash card is used to store edit data (players can edit data by PlayStation
  console, and send data to arcade machine via PS memory card).
  DM and GF: network PCB unit (for e-AMUSEMENT) is connected via PCMCIA slot.
  GM: unknown (program data is stored in flash card?)

  The games that run on this system include...

  Game                                         Year       Hardware Code     CD Code
  ---------------------------------------------------------------------------------
P *Anime Champ                                  2000.12
P Bass Angler                                   1998.03    GE765 JA          765 JA A02
P Bass Angler 2                                 1998.07    GC865 JA          865 JA A02
P *DAM-DDR Dance Dance Revolution for DAM       1999.11
P *DAM-DDR Dance Dance Revolution for DAM 2nd   2000.07
A Dance Dance Revolution                        1998.09    GC845 JA          845 JA(missing)/UA A01 / 845 JA A02
A Dance Dance Revolution Internet Ranking ver.  1998.11    GC845 JB          845 JB A01 / 845 JA/UA A02
A Dance Dance Revolution 2ndMIX                 1999.01    GC895 JA          895 JA A02
A Dance Dance Revolution 2ndMIX LINK version    1999.04    GE885 JA          885 JA A02
A DDR 2ndMIX with bmIIDX CLUB ver.              1999.05    GN896 JA          896 JA A01
A DDR 2ndMIX AND bmIIDX substream CLUB ver. 2   1999.05    GE984 JA          984 JA A01
D Dance Dance Revolution Solo BASS MIX          1999.08    GQ894 JA          894 JA A02
D Dance Dance Revolution 3rdMIX                 1999.10    GN887 AA/JA/KA    887 AA/JA/KA A02
D Dance Dance Revolution Solo 2000              1999.12    GC905 AA/JA       905 AA/JA A02
D Dance Dance Revolution 3rdMIX PLUS            2000.06    GCA22 JA          A22 JA A02
D Dance Dance Revolution 4thMIX                 2000.08    GCA33 JA          A33 JA A02
D Dance Dance Revolution 4thMIX PLUS            2000.12    GCA34 JA          A34 JA A02
? *Dance Dance Revolution Kids                  2000.12
D Dance Dance Revolution 5thMIX                 2001.03    GCA27 JA          A27 JA A02
D DDRMAX Dance Dance Revolution 6thMIX          2001.10    GCB19 JA          B19 JA A02
D DDRMAX2 Dance Dance Revolution 7thMIX         2002.03    GCB20 JA          B20 JA A02
D Dance Dance Revolution EXTREME                2002.12    GCC36 JA          C36 JA A02
D Dance Maniax                                  2000.06    GE874 JA          874 JA A(needs redump)
D Dance Maniax 2ndMIX                           2000.12    GCA39 JA          A39 JA A02
D *Dance Maniax 2ndMIX APPEND J PARADISE        2001.04
A Dancing Stage                                 1999.08    GN845 EA          845 EA(needs redump)
D Dancing Stage Euro Mix                        2000       GE936 EA          936 EA A(needs redump)
D Dancing Stage Euro Mix 2                      2000       G*C23 EA          C23 EA A02
D Dancing Stage featuring Disney's Rave         2000.11    GCA37 JA          A37 JA A02
D Dancing Stage featuring DREAMS COME TRUE      1999.12    GC910 JA          910 JA/JC A02
A Dancing Stage featuring TRUE KiSS DESTiNATiON 1999.07    G*884 JA          884 JA A02
P Dark Horse Legend                             1998.03    GX706 JA          706 JA A02
A drummania                                     1999.07    GQ881 JA          881 JA D01 / 881 JA A02(missing, audio CD)
D drummania 2ndMIX                              2000.03    GE912 JA          912 JA B02
D drummania 3rdMIX                              2000.09    GCA23 JA          A23 JA A02
D drummania 4thMIX                              2001.03    GEA25 JA          A25 JA A02
D drummania 5thMIX                              2001.09    GCB05 JA          B05 JA A02
D drummania 6thMIX                              2002.02    GCB16 JA          B16 JA A02
N drummania 7thMIX                              2002.08    GCC07 JA          C07 JA A02
N drummania 7thMIX power-up ver.                2002.08    GEC07 JB          C07 JC A02
N drummania 8thMIX                              2003.04    GCC38 JA          C38 JA A02
N drummania 9thMIX                              2003.10    GCD09 JA          D09 JA A02
N *drummania 10thMIX                            2004.04
? *Fighting Mania                               2000
P Fisherman's Bait                              1998.06    GE765 UA          765 UA B02
P Fisherman's Bait 2                            1998       GC865 UA          865 UA B02
P Fisherman's Bait Marlin Challenge             1999       GX889             889 AA/EA/JA/UA(needs redump)
P Gachagachamp                                  1999.01    GQ877 JA          GE877-JA(PCMCIA card)
A GUITARFREAKS                                  1999.02    GQ886 EA/JA/UA    886 ** C02
A GUITARFREAKS 2ndMIX                           1999.07    GQ883 JA          929 JB B02(needs redump)
A *GUITARFREAKS 2ndMIX Link ver.                1999.09
D GUITARFREAKS 3rdMIX                           2000.04    GE949 JA          949 JA C01 / 949 JA C02
D GUITARFREAKS 4thMIX                           2000.08    GEA24 JA          A24 JA A02
D GUITARFREAKS 5thMIX                           2001.03    GCA26 JA          A26 JA A02
D GUITARFREAKS 6thMIX                           2001.09    GCB06 JA          B06 JA A02
D GUITARFREAKS 7thMIX                           2002.02    GCB17 JA          B17 JA A02
N GUITARFREAKS 8thMIX                           2002.08    GCC08 JA          C08 JA A02
N GUITARFREAKS 8thMIX power-up ver.             2002.11    GEC08 JB          C08 JB A02
N GUITARFREAKS 9thMIX                           2003.04    GCC39 JA          C39 JA A02
N GUITARFREAKS 10thMIX                          2003.10    GCD10 JA          D10 JA A02
N *GUITARFREAKS 11thMIX                         2004.04
G *Gun Mania                                    2000.07    G?906 JA          (no CD)
? *Gun Mania Zone Plus                          2000.10
P Handle Champ                                  1997.12    GQ710 JA          (no CD)
P Hyper Bishi Bashi Champ                       1998.07    GC876 EA          (no CD)
P Hyper Bishi Bashi Champ - 2 Player            1999.08    GC908 JA          908    A02
P Jikkyou Powerful Pro Yakyuu EX                1998.04    GX802 JA          802 JA B02
P *Jikkyou Powerful Pro Yakyuu EX 98            1998.08
? *Kick & Kick                                  2001
P Konami 80's Arcade Gallery                    1998.11    GC826 JA          826 JA A01
P Konami 80's AC Special                        1998       GC826 UA          826 UA A01
D *Mambo a GoGo                                 2001.06
D Punchmania Hokuto no Ken                      2000.03                      918 JA B02
D Punchmania Hokuto no Ken 2                    2000.12                      A09 JA A02
P Salary Man Champ                              2001.02    GCA18 JA          A18 JA(needs redump)
P *Step Champ                                   1999.12

P: plain System573
A: uses ext. analog I/O board
D: uses ext. digital sound and I/O board
N: uses network PCB unit + ext. digital sound and I/O board
G: gun mania only, drives air soft gun (this game uses real BB bullet)

  Note:
       Not all games listed above are confirmed to run on System 573.
       * - denotes not dumped yet. If you can help with the remaining undumped System 573 games,
       please contact http://guru.mameworld.info/


  Main PCB Layout
  ---------------
                                                     External controls port
  GX700-PWB(A)B                                               ||
  (C)1997 KONAMI CO. LTD.                                     \/
  |-----------------------------------------------------==============-------|
  |   CN15            CNA                     CN10                           |
  |        CN16                                                              |
  |                                                 |------------------------|
  | PQ30RV21                                        |                        |
  |                         |-------|               |                        |
  |             KM416V256   |SONY   |               |     PCMCIA SLOT        |
  |                         |CXD2925|               |                        |
  |                         |-------|               |                        |
  |                                                 |                        |
  |                                                 |------------------------|
  | |-----|                                        CN21                      |
  | |32M  |  |---------|     |---------|                                     |
  | |-----|  |SONY     |     |SONY     |                                     |
  |          |CXD8561Q |     |CXD8530CQ|           29F016   29F016   |--|    |
  | |-----|  |         |     |         |                             |  |    |
  | |32M  |  |         |     |         |                             |  |    |
  | |-----|  |---------|     |---------|           29F016   29F016   |  |    |
  |      53.693175MHz    67.7376MHz                                  |  |    |
  |                                     |-----|                      |  |CN14|
  |      KM48V514      KM48V514         |9536 |    29F016   29F016   |  |    |
  |            KM48V514       KM48V514  |     |                      |  |    |
  |      KM48V514      KM48V514         |-----|                      |  |    |
  |            KM48V514      KM48V514              29F016   29F016   |--|    |
  | MC44200FT                          M48T58Y-70PC1                         |
  |                                                                      CN12|
  |                                    700A01.22                             |
  |                             14.7456MHz                                   |
  |                  |-------|                                               |
  |                  |KONAMI |    |----|                               LA4705|
  |   058232         |056879 |    |3644|                            SM5877   |
  |                  |       |    |----|         ADC0834                LM358|
  |                  |-------|            ADM485           CN4               |
  |                              CN5                        CN3      CN17    |
  |                                TEST_SW  DIP4 USB   CN8     RCA-L/R   CN9 |
  |--|          JAMMA            |-------------------------------------------|
     |---------------------------|
  Notes:
  CNA       - 40-pin IDE cable connector
  CN3       - 10-pin connector labelled 'ANALOG', connected to a 9-pin DSUB connector mounted in the
              front face of the housing, labelled 'OPTION1'
  CN4       - 12-pin connector labelled 'EXT-OUT'
  CN5       - 10-pin connector labelled 'EXT-IN', connected to a 9-pin DSUB connector mounted in the
              front face of the housing, labelled 'OPTION2'
  CN8       - 15-pin DSUB plug labelled 'VGA-DSUB15' extending from the front face of the housing
              labelled 'RGB'. Use of this connector is optional because the video is output via the
              standard JAMMA connector
  CN9       - 4-pin connector for amplified stereo sound output to 2 speakers
  CN10      - Custom 80-pin connector (for mounting an additional plug-in board for extra controls,
              possibly with CN21 also)
  CN12      - 4-pin CD-DA input connector (for Red-Book audio from CDROM drive to main board)
  CN14      - 44-pin security cartridge connector. The cartridge only contains a small PCB labelled
              'GX700-PWB(D) (C)1997 KONAMI' and has locations for 2 ICs only
              IC1 - Small SOIC8 chip, identified as a XICOR X76F041 security supervisor containing 4X
              128 x8 secureFLASH arrays, stamped '0038323 E9750'
              IC2 - Solder pads for mounting of a PLCC68 or QFP68 packaged IC (not populated)
  CN15      - 4-pin CDROM power connector
  CN16      - 2-pin fan connector
  CN17      - 6-pin power connector, connected to an 8-pin power plug mounted in the front face
              of the housing. This can be left unused because the JAMMA connector supplies all power
              requirements to the PCB
  CN21      - Custom 30-pin connector (purpose unknown, but probably for mounting an additional
              plug-in board with CN10 also)
  TEST_SW   - Push-button test switch
  DIP4      - 4-position DIP switch
  USB       - USB connector extended from the front face of the housing labelled 'I/O'
  RCA-L/R   - RCA connectors for left/right audio output
  PQ30RV21  - Sharp PQ30RV21 low-power voltage regulator (5 Volt to 3 Volt)
  LA4705    - Sanyo LA4705 15W 2-channel power amplifier (SIP18)
  LM358     - National Semiconductor LM358 low power dual operational amplifier (SOIC8, @ 33C)
  CXD2925Q  - Sony CXD2925Q SPU (QFP100, @ 15Q)
  CXD8561Q  - Sony CXD8561Q GPU (QFP208, @ 10M) Also found CXD8561BQ in some units
  CXD8530CQ - Sony CXD8530CQ R3000-based CPU (QFP208, @ 17M)
  9536      - Xilinx XC9536 in-system-programmable CPLD (PLCC44, @ 22J)
  3644      - Hitachi H8/3644 HD6473644H microcontroller with 32k ROM & 1k RAM (QFP64, @ 18E,
              labelled '700 02 38920')
  056879    - Konami 056879 custom IC (QFP120, @ 13E)
  MC44200FT - Motorola MC44200FT Triple 8-bit Video DAC (QFP44)
  058232    - Konami 058232 custom ceramic IC (SIP14, @ 6C)
  SM5877    - Nippon Precision Circuits SM5877 2-channel D/A convertor (SSOP24, @32D)
  ADM485    - Analog Devices ADM485 low power EIA RS-485 transceiver (SOIC8, @ 20C)
  ADC0834   - National Semiconductor ADC0834 8-Bit Serial I/O A/D Converter with Multiplexer
              Option (SOIC14, @ 24D)
  M48T58Y-70- STMicroelectronics M48T58Y-70PC1 8k x8 Timekeeper RAM (DIP32, @ 22H)
              Note that this is not used for protection. If you put in a new blank Timekeeper RAM
              it will be programmed with some data on power-up. If you swap games, the Timekeeper
              is updated with the new game data
  29F016      Fujitsu 29F016A-90PFTN 2M x8 FlashROM (TSOP48, @ 27H/J/L/M & 31H/J/L/M)
              Also found Sharp LH28F016S (2M x8 TSOP40) in some units
  KM416V256 - Samsung Electronics KM416V256BT-7 256k x 16 DRAM (TSOP44/40, @ 11Q labelled 'SPUDR4M')
  KM48V514  - Samsung Electronics KM48V514BJ-6 512k x 8 EDO DRAM (SOJ28, @ 16G/H, 14G/H, 12G/H, 9G/H labelled 'HDR4M8SJ')
              Also found NEC 424805AL-A60 in some units
  32M       - NEC D481850GF-A12 128k x 32Bit x 2 Banks SGRAM (QFP100, @ 4P & 4L)
              Also found Samsung KM4132G271Q-12 in some units
  Software  -
              - 700A01.22G 4M MaskROM (DIP32, @ 22G). AMD 27C040 is also used
              - SONY ATAPI CDROM drive, with CDROM disc containing program + graphics + sound
                Some System 573 units contain a CR-583 drive dated October 1997, some contain a
                CR-587 drive dated March 1998. Note that the CR-587 will not read CDR discs ;-)


  Auxillary Controls PCB
  ----------------------

  GE765-PWB(B)A (C)1998 KONAMI CO. LTD.
  |-----------------------------|
  |          CN33     C2242     |
  |                   C2242     |
  |       NRPS11-G1A            |
  |                         CN35|
  |  D4701                      |
  |        74LS14     PC817     |-----------------|
  |                                               |
  |  PAL         PAL                              |
  | (E765B1)    (E765B2)         LCX245           |
  |                                               |
  |  74LS174     PAL                              |
  |             (E765B1)                          |
  |                                               |
  |              74LS174       CN31               |
  |-----------------------------------------------|
  Notes: (all IC's shown)
        This PCB is known to be used for the fishing reel controls on all of the fishing games (at least).

        CN31       - Connector joining this PCB to the MAIN PCB
        CN33       - Connector used to join the external controls connector mounted on the outside of the
                     metal case to this PCB.
        CN35       - Power connector
        NRPS11-G1A - Relay?
        D4701      - NEC uPD4701 Encoder (SOP24)
        C2242      - 2SC2242 Transistor
        PC817      - Sharp PC817 Photo-coupler IC (DIP4)
        PAL        - AMD PALCE16V8Q, stamped 'E765Bx' (DIP20)


 GE877-PWB(C) (C)1998 KONAMI
  |----------------------|
|--       JAMMA OUT      --|
|                          |
|      CN6                 |
|     CN5  CN4  CN3  CN2   |
|                          |
|                          |
|         JAMMA IN         |
|--------------------------|
 Notes: This PCB is used for Gachagachamp. No ICs.

        CN5 - To control lever unit (1P). uses 9 pins out of 15 pins of B15P-SHF-1AA
        CN6 - To control lever unit (2P). uses 9 pins out of 14 pins of B14P-SHF-1AA
        (CN4, CN3, CN2 is printed pattern only, no actual connector)



  Digital I/O PCB
  ---------------

  GX894-PWB(B)A (C)1999 KONAMI CO. LTD.

             |-------------|
             |        CN12 |
             |             |
             | PC847 PC847 |
             |             |
             |        CN11 |
             |             |
             | PC847 PC847 |
             |             |
             | DS2401 CN10 |
             |             |
             | PC847 PC847 |
             |             |
             |  CN14  CN13 |
  |----------|             |----------|
  |                  PC847            |
  | ADM232 CN17              XC9536   |
  |                                   |
  |                    19.6608MHz     |-----------|
  | ADM232 CN15  CY7C109                          |
  |                       HY51V65164A HY51V65164A |
  |                            HY51V65164A        |
  |      CN16    XCS40XL                          |
  |                                               |
  | AK4309B   CN18         29.450MHz  MAS3507D    |
  |                                               |
  |                           CN3                 |
  | HYC24855  RCA-L/R                             |
  |-----------------------------------------------|

  Notes:

  PC847       - High Density Mounting Type Photocoupler
  CN12        - 13 pin connector with 8 wires to external connectors
  CN11        - 12 pin connector with 8 wires to external connectors
  DS2401      - DS2401 911C2  Silicon serial number
  CN10        - 10 pin connector with 8 wires to external connectors
  CN14        - 7 pin connector
  CN13        - 5 pin connector with 2 wires to external connectors
  ADM232      - ADM232AARN 9933 H48475  High Speed, 5 V, 0.1 uF CMOS RS-232 Drivers/Receivers
  CN17        - 3 pin connector
  XC9536      - XILINX XC9536 PC44AEM9933 F1096429A 15C
  CN15        - 8 pin connector
  CY7C109     - CY7C109-25VC 931 H 04 404825  128k x 8 Static RAM
  HY51V65164A - 64M bit dynamic EDO RAM
  CN16        - 4 pin connector joining this PCB to the CD-DA IN on the MAIN PCB.
  XCS40XL     - XILINX XCS40XL PQ208AKP9929 A2033251A 4C
  AK4309B     - AKM AK4309B 3N932N  16bit SCF DAC
  CN18        - 6 pin connector
  MAS3507D    - IM MAS3507D D8 9173 51 HM U 072953.000 ES  MPEG 1/2 Layer 2/3 Audio Decoder
  CN3         - Connector joining this PCB to the MAIN PCB
  HYC24855    - ?
  RCA-L/R     - RCA connectors for left/right audio output

  Drummania 10th Mix Multisession
  -------------------------------

  This box is used with multi-session System 573 games.

  Main board is standard GX700 PCB with CDROM (Drummania 10th Mix Multisession)
  and Digital I/O Board GX894-PWB(B)A
  BIOS is on a small plug-in daughterboard.
  Daughterboard contains one EPROM, one PAL22V10, 2 logic chips and a PIC16F84.
  The dumps provided are the EPROM dumped separately and a dump of the 'board'
  with it plugged in (reading may be affected by the PIC)


  PCB Layout of External Multisession Box
  ---------------------------------------

  GXA25-PWB(A)(C)2000 KONAMI
  |--------------------------------------------------------------------------|
  |CN9  ADM232  LS273        PC16552          PC16552         XC9536(1)  CN13|
  |DSW(8)  LS245   LS273            18.432MHz                        DS2401  |
  |         |-------|      |-------|       |-------|      |-------|          |
  | MB3793  |TOSHIBA|      |TOSHIBA|       |TOSHIBA|      |TOSHIBA|M48T58Y.6T|
  |         |TC9446F|      |TC9446F|       |TC9446F|      |TC9446F|          |
  |         |-016   |      |-016   |       |-016   |      |-016   |      CN12|
  |         |-------|      |-------|       |-------|      |-------|          |
  |       LV14                    XC9572XL                                   |
  | CN16                 CN17                 CN18             CN19 XC9536(2)|
  |PQ30RV21        LCX245   LCX245                                       CN11|
  |                                  33.8688MHz              PQ30RV21        |
  |    8.25MHz   HY57V641620                                                 |
  |  |------------|     HY57V641620   XC2S200                                |
  |  |TOSHIBA     |                                          FLASH.20T       |
  |  |TMPR3927AF  |                                                      CN10|
  |  |            |                                                          |
  |  |            |                                     LS245   F245  F245   |
  |  |            |HY57V641620  LCX245     DIP40                             |
  |  |------------|     HY57V641620  LCX245                   ATAPI44        |
  |                             LCX245              LED(HDD)  ATAPI40        |
  |    CN7                      LCX245      CN14    LED(CD)           CN5    |
  |--------------------------------------------------------------------------|
  Notes: (all IC's shown)
          TMPR3927     - Toshiba TMPR3927AF Risc Microprocessor (QFP240)
          FLASH.20T    - Fujitsu 29F400TC Flash ROM (TSOP48)
          ATAPI44      - IDE44 44-pin laptop type HDD connector (not used)
          ATAPI40      - IDE40 40-pin flat cable HDD connector used for connection of CDROM drive
          XC9572XL     - XILINX XC9572XL In-system Programmable CPLD stamped 'XA25A1' (TQFP100)
          XC9536(1)    - XILINX CPLD stamped 'XA25A3' (PLCC44)
          XC9536(2)    - XILINX CPLD stamped 'XA25A2' (PLCC44)
          XC2S200      - XILINX XC2S200 SPARTAN FPGA (QFP208)
          DS2401       - MAXIM Dallas DS2401 Silicon Serial Number (SOIC6)
          M48T58Y      - ST M48T58Y Timekeeper NVRAM 8k bytes x8-bit (DIP28). Chip appears empty (0x04 fill) or unused
          MB3793       - Fujitsu MB3793 Power-Voltage Monitoring IC with Watchdog Timer (SOIC8)
          DIP40        - Empty DIP40 socket
          HY57V641620  - Hyundai/Hynix HY57V641620 4 Banks x 1M x 16Bit Synchronous DRAM
          PC16552D     - National PC16552D Dual Universal Asynchronous Receiver/Transmitter with FIFO's
          TC9446F      - Toshiba TC9446F-016 Audio Digital Processor for Decode of Dolby Digital (AC-3) MPEG2 Audio
          CN16-CN19    - Connector for sub board (3 of them are present). One board connects via a thin cable from
                         CN1 to the main board to a connector on the security board labelled 'AMP BOX'.

  Sub Board Layout
  ----------------

  GXA25-PWB(B) (C) 2000 KONAMI
  |---------------------------------|
  | TLP2630  LV14          ADM232   |
  |CN2                           CN1|
  |A2430         AK5330             |
  |                                 |
  |                          RCA L/R|
  |ZUS1R50505        6379A          |
  |                          LM358  |
  |---------------------------------|

  */

#include "emu.h"
#include "cdrom.h"
#include "cpu/psx/psx.h"
#include "video/psx.h"
#include "includes/psx.h"
#include "machine/intelfsh.h"
#include "machine/cr589.h"
#include "machine/timekpr.h"
#include "machine/adc083x.h"
#include "machine/ds2401.h"
#include "machine/upd4701.h"
#include "machine/x76f041.h"
#include "machine/x76f100.h"
#include "machine/zs01.h"
#include "sound/spu.h"
#include "sound/cdda.h"
#include "sound/mas3507d.h"

#define VERBOSE_LEVEL ( 0 )

#define ATAPI_CYCLES_PER_SECTOR (5000)	// plenty of time to allow DMA setup etc.  BIOS requires this be at least 2000, individual games may vary.

#define ATAPI_STAT_BSY	   0x80
#define ATAPI_STAT_DRDY    0x40
#define ATAPI_STAT_DMARDDF 0x20
#define ATAPI_STAT_SERVDSC 0x10
#define ATAPI_STAT_DRQ     0x08
#define ATAPI_STAT_CORR    0x04
#define ATAPI_STAT_CHECK   0x01

#define ATAPI_INTREASON_COMMAND 0x01
#define ATAPI_INTREASON_IO      0x02
#define ATAPI_INTREASON_RELEASE 0x04

#define ATAPI_REG_DATA		0
#define ATAPI_REG_ERRFEAT	1
#define ATAPI_REG_INTREASON	2
#define ATAPI_REG_SAMTAG	3
#define ATAPI_REG_COUNTLOW	4
#define ATAPI_REG_COUNTHIGH	5
#define ATAPI_REG_DRIVESEL	6
#define ATAPI_REG_CMDSTATUS	7
#define ATAPI_REG_MAX 16

#define ATAPI_DATA_SIZE ( 64 * 1024 )

#define MAX_TRANSFER_SIZE ( 63488 )


class ksys573_state : public psx_state
{
public:
	ksys573_state(const machine_config &mconfig, device_type type, const char *tag)
		: psx_state(mconfig, type, tag) { }

	int m_flash_bank;
	fujitsu_29f016a_device *m_flash_device[5][16];
	int m_security_cart_number;

	UINT32 m_control;

	emu_timer *m_atapi_timer;
	scsidev_device *m_inserted_cdrom;
	scsidev_device *m_available_cdroms[ 2 ];
	int m_atapi_data_ptr;
	int m_atapi_data_len;
	int m_atapi_xferlen;
	int m_atapi_xferbase;
	int m_atapi_cdata_wait;
	int m_atapi_xfermod;

	UINT32 m_n_security_control;
	void (*m_security_callback)( running_machine &machine, int data );

	UINT8 m_gx700pwbf_output_data[ 4 ];
	void (*m_gx700pwfbf_output_callback)( running_machine &machine, int offset, int data );
	UINT16 m_gx894pwbba_output_data[ 8 ];
	void (*m_gx894pwbba_output_callback)( running_machine &machine, int offset, int data );

	UINT32 m_stage_mask;
	struct
	{
		int DO;
		int clk;
		int shift;
		int state;
		int bit;
	} m_stage[ 2 ];

	UINT32 m_gx894_ram_write_offset;
	UINT32 m_gx894_ram_read_offset;
	UINT16 *m_gx894_ram;
	int m_f;
	int m_o;
	int m_s;
	UINT32 m_a;
	UINT32 m_b;
	UINT32 m_c;
	UINT32 m_d;

	int m_salarymc_lamp_bits;
	int m_salarymc_lamp_shift;
	int m_salarymc_lamp_clk;

	int m_hyperbbc_lamp_strobe1;
	int m_hyperbbc_lamp_strobe2;

	/* memory */
	UINT8 m_atapi_regs[ATAPI_REG_MAX];
	UINT8 m_atapi_data[ATAPI_DATA_SIZE];

	int m_tank_shutter_position;
	int m_cable_holder_release;
	double m_pad_position[ 6 ];
	DECLARE_CUSTOM_INPUT_MEMBER(gn845pwbb_read);
	DECLARE_CUSTOM_INPUT_MEMBER(gunmania_tank_shutter_sensor);
	DECLARE_CUSTOM_INPUT_MEMBER(gunmania_cable_holder_sensor);
	DECLARE_WRITE32_MEMBER(mb89371_w);
	DECLARE_READ32_MEMBER(mb89371_r);
	DECLARE_READ32_MEMBER(jamma_r);
	DECLARE_READ32_MEMBER(control_r);
	DECLARE_WRITE32_MEMBER(control_w);
	DECLARE_READ32_MEMBER(atapi_r);
	DECLARE_WRITE32_MEMBER(atapi_w);
	DECLARE_WRITE32_MEMBER(atapi_reset_w);
	DECLARE_WRITE32_MEMBER(security_w);
	DECLARE_READ32_MEMBER(security_r);
	DECLARE_READ32_MEMBER(flash_r);
	DECLARE_WRITE32_MEMBER(flash_w);
	DECLARE_READ32_MEMBER(ge765pwbba_r);
	DECLARE_WRITE32_MEMBER(ge765pwbba_w);
	DECLARE_READ32_MEMBER(gx700pwbf_io_r);
	DECLARE_WRITE32_MEMBER(gx700pwbf_io_w);
	DECLARE_READ32_MEMBER(gtrfrks_io_r);
	DECLARE_WRITE32_MEMBER(gtrfrks_io_w);
	DECLARE_READ32_MEMBER(gx894pwbba_r);
	DECLARE_WRITE32_MEMBER(gx894pwbba_w);
	DECLARE_WRITE32_MEMBER(dmx_io_w);
	DECLARE_WRITE32_MEMBER(mamboagg_io_w);
	DECLARE_WRITE32_MEMBER(gunmania_w);
	DECLARE_READ32_MEMBER(gunmania_r);
	DECLARE_DRIVER_INIT(gtrfrkdigital);
	DECLARE_DRIVER_INIT(salarymc);
	DECLARE_DRIVER_INIT(dmx);
	DECLARE_DRIVER_INIT(gtrfrks);
	DECLARE_DRIVER_INIT(drmndigital);
	DECLARE_DRIVER_INIT(punchmania);
	DECLARE_DRIVER_INIT(ddr);
	DECLARE_DRIVER_INIT(mamboagg);
	DECLARE_DRIVER_INIT(ge765pwbba);
	DECLARE_DRIVER_INIT(gunmania);
	DECLARE_DRIVER_INIT(hyperbbc);
	DECLARE_DRIVER_INIT(drmn);
	DECLARE_DRIVER_INIT(ddrsolo);
	DECLARE_DRIVER_INIT(ddrdigital);
	DECLARE_DRIVER_INIT(konami573);
	DECLARE_MACHINE_RESET(konami573);
};

INLINE void ATTR_PRINTF(3,4) verboselog( running_machine &machine, int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: %s", machine.describe_context(), buf );
	}
}

WRITE32_MEMBER(ksys573_state::mb89371_w)
{
	verboselog( machine(), 2, "mb89371_w %08x %08x %08x\n", offset, mem_mask, data );
}

READ32_MEMBER(ksys573_state::mb89371_r)
{
	UINT32 data = 0xffffffff;
	verboselog( machine(), 2, "mb89371_r %08x %08x %08x\n", offset, mem_mask, data );
	return data;
}

READ32_MEMBER(ksys573_state::jamma_r)
{
	int security_cart_number = m_security_cart_number;
	UINT32 data = ioport("IN1")->read();
	data |= 0x000000c0;

	ds2401_device *ds2401 = machine().device<ds2401_device>(security_cart_number ? "game_id" : "install_id");
	device_secure_serial_flash *secflash = machine().device<device_secure_serial_flash>(security_cart_number ? "game_eeprom" : "install_eeprom");

	if( ds2401 )
	{
		data |= ds2401->read() << 14;
	}

	if( secflash )
		data |= secflash->sda_r() << 18;

	if( m_flash_device[1][0] == NULL )
	{
		data |= ( 1 << 26 );
	}
	if( m_flash_device[2][0] == NULL )
	{
		data |= ( 1 << 27 );
	}

	verboselog( machine(), 2, "jamma_r( %08x, %08x ) %08x\n", offset, mem_mask, data );

	return data;
}

READ32_MEMBER(ksys573_state::control_r)
{

	verboselog( machine(), 2, "control_r( %08x, %08x ) %08x\n", offset, mem_mask, m_control );

	return m_control;
}

WRITE32_MEMBER(ksys573_state::control_w)
{
	UINT32 control;
	int old_bank = m_flash_bank;

	COMBINE_DATA(&m_control);
	control = m_control;

	verboselog( machine(), 2, "control_w( %08x, %08x, %08x )\n", offset, mem_mask, data );

	m_flash_bank = -1;

	// zs01 only, others are reached through security_w
	device_secure_serial_flash *secflash = machine().device<device_secure_serial_flash>(m_security_cart_number ? "game_eeprom" : "install_eeprom");
	if( dynamic_cast<zs01_device *>(secflash) )
		secflash->sda_w( !( ( control >> 6 ) & 1 ) ); /* 0x40 */

	if( m_flash_device[0][0] != NULL && ( control & ~0x43 ) == 0x00 )
	{
		m_flash_bank = (0 << 8) + ( ( control & 3 ) * 2 );
		if( m_flash_bank != old_bank ) verboselog( machine(), 1, "onboard %d\n", control & 3 );
	}
	else if( m_flash_device[1][0] != NULL && ( control & ~0x47 ) == 0x10 )
	{
		m_flash_bank = (1 << 8) + ( ( control & 7 ) * 2 );
		if( m_flash_bank != old_bank ) verboselog( machine(), 1, "pccard1 %d\n", control & 7 );
	}
	else if( m_flash_device[2][0] != NULL && ( control & ~0x47 ) == 0x20 )
	{
		m_flash_bank = (2 << 8) + ( ( control & 7 ) * 2 );
		if( m_flash_bank != old_bank ) verboselog( machine(), 1, "pccard2 %d\n", control & 7 );
	}
	else if( m_flash_device[3][0] != NULL && ( control & ~0x47 ) == 0x20 )
	{
		m_flash_bank = (3 << 8) + ( ( control & 7 ) * 2 );
		if( m_flash_bank != old_bank ) verboselog( machine(), 1, "pccard3 %d\n", control & 7 );
	}
	else if( m_flash_device[4][0] != NULL && ( control & ~0x47 ) == 0x28 )
	{
		m_flash_bank = (4 << 8) + ( ( control & 7 ) * 2 );
		if( m_flash_bank != old_bank ) verboselog( machine(), 1, "pccard4 %d\n", control & 7 );
	}
}

static TIMER_CALLBACK( atapi_xfer_end )
{
	ksys573_state *state = machine.driver_data<ksys573_state>();
	UINT32 *p_n_psxram = state->m_p_n_psxram;
	UINT8 *atapi_regs = state->m_atapi_regs;
	int i, n_this;
	UINT8 sector_buffer[ 4096 ];

	state->m_atapi_timer->adjust(attotime::never);

//  verboselog( machine, 2, "atapi_xfer_end( %d ) atapi_xferlen = %d, atapi_xfermod=%d\n", x, atapi_xfermod, atapi_xferlen );

//  mame_printf_debug("ATAPI: xfer_end.  xferlen = %d, atapi_xfermod = %d\n", atapi_xferlen, atapi_xfermod);

	while( state->m_atapi_xferlen > 0 )
	{
		// get a sector from the SCSI device
		state->m_inserted_cdrom->ReadData( sector_buffer, 2048 );

		state->m_atapi_xferlen -= 2048;

		i = 0;
		n_this = 2048 / 4;
		while( n_this > 0 )
		{
			p_n_psxram[ state->m_atapi_xferbase / 4 ] =
				( sector_buffer[ i + 0 ] << 0 ) |
				( sector_buffer[ i + 1 ] << 8 ) |
				( sector_buffer[ i + 2 ] << 16 ) |
				( sector_buffer[ i + 3 ] << 24 );
			state->m_atapi_xferbase += 4;
			i += 4;
			n_this--;
		}
	}

	if (state->m_atapi_xfermod > MAX_TRANSFER_SIZE)
	{
		state->m_atapi_xferlen = MAX_TRANSFER_SIZE;
		state->m_atapi_xfermod = state->m_atapi_xfermod - MAX_TRANSFER_SIZE;
	}
	else
	{
		state->m_atapi_xferlen = state->m_atapi_xfermod;
		state->m_atapi_xfermod = 0;
	}


	if (state->m_atapi_xferlen > 0)
	{
		//mame_printf_debug("ATAPI: starting next piece of multi-part transfer\n");
		atapi_regs[ATAPI_REG_COUNTLOW] = state->m_atapi_xferlen & 0xff;
		atapi_regs[ATAPI_REG_COUNTHIGH] = (state->m_atapi_xferlen>>8)&0xff;

		state->m_atapi_timer->adjust(machine.device<cpu_device>("maincpu")->cycles_to_attotime((ATAPI_CYCLES_PER_SECTOR * (state->m_atapi_xferlen/2048))));
	}
	else
	{
		//mame_printf_debug("ATAPI: Transfer completed, dropping DRQ\n");
		atapi_regs[ATAPI_REG_CMDSTATUS] = ATAPI_STAT_DRDY;
		atapi_regs[ATAPI_REG_INTREASON] = ATAPI_INTREASON_IO | ATAPI_INTREASON_COMMAND;
	}

	psx_irq_set(machine, 0x400);

	verboselog( machine, 2, "atapi_xfer_end: %d %d\n", state->m_atapi_xferlen, state->m_atapi_xfermod );
}

READ32_MEMBER(ksys573_state::atapi_r)
{
	UINT8 *atapi_regs = m_atapi_regs;
	int reg, data;

	if (mem_mask == 0x0000ffff)	// word-wide command read
	{
//      mame_printf_debug("ATAPI: packet read = %04x\n", atapi_data[atapi_data_ptr]);

		// assert IRQ and drop DRQ
		if (m_atapi_data_ptr == 0 && m_atapi_data_len == 0)
		{
			// get the data from the device
			if( m_atapi_xferlen > 0 )
			{
				m_inserted_cdrom->ReadData( m_atapi_data, m_atapi_xferlen );
				m_atapi_data_len = m_atapi_xferlen;
			}

			if (m_atapi_xfermod > MAX_TRANSFER_SIZE)
			{
				m_atapi_xferlen = MAX_TRANSFER_SIZE;
				m_atapi_xfermod = m_atapi_xfermod - MAX_TRANSFER_SIZE;
			}
			else
			{
				m_atapi_xferlen = m_atapi_xfermod;
				m_atapi_xfermod = 0;
			}

			verboselog( machine(), 2, "atapi_r: atapi_xferlen=%d\n", m_atapi_xferlen );
			if( m_atapi_xferlen != 0 )
			{
				atapi_regs[ATAPI_REG_CMDSTATUS] = ATAPI_STAT_DRQ | ATAPI_STAT_SERVDSC;
				atapi_regs[ATAPI_REG_INTREASON] = ATAPI_INTREASON_IO;
			}
			else
			{
				//mame_printf_debug("ATAPI: dropping DRQ\n");
				atapi_regs[ATAPI_REG_CMDSTATUS] = 0;
				atapi_regs[ATAPI_REG_INTREASON] = ATAPI_INTREASON_IO;
			}

			atapi_regs[ATAPI_REG_COUNTLOW] = m_atapi_xferlen & 0xff;
			atapi_regs[ATAPI_REG_COUNTHIGH] = (m_atapi_xferlen>>8)&0xff;

			psx_irq_set(machine(), 0x400);
		}

		if( m_atapi_data_ptr < m_atapi_data_len )
		{
			data = m_atapi_data[m_atapi_data_ptr++];
			data |= ( m_atapi_data[m_atapi_data_ptr++] << 8 );
			if( m_atapi_data_ptr >= m_atapi_data_len )
			{
//              verboselog( machine(), 2, "atapi_r: read all bytes\n" );
				m_atapi_data_ptr = 0;
				m_atapi_data_len = 0;

				if( m_atapi_xferlen == 0 )
				{
					atapi_regs[ATAPI_REG_CMDSTATUS] = 0;
					atapi_regs[ATAPI_REG_INTREASON] = ATAPI_INTREASON_IO;
					psx_irq_set(machine(), 0x400);
				}
			}
		}
		else
		{
			data = 0;
		}
	}
	else
	{
		int shift;
		reg = offset<<1;
		shift = 0;
		if (mem_mask == 0x00ff0000)
		{
			reg += 1;
			shift = 16;
		}

		data = atapi_regs[reg];

		switch( reg )
		{
		case ATAPI_REG_DATA:
			verboselog( machine(), 1, "atapi_r: data=%02x\n", data );
			break;
		case ATAPI_REG_ERRFEAT:
			verboselog( machine(), 1, "atapi_r: errfeat=%02x\n", data );
			break;
		case ATAPI_REG_INTREASON:
			verboselog( machine(), 1, "atapi_r: intreason=%02x\n", data );
			break;
		case ATAPI_REG_SAMTAG:
			verboselog( machine(), 1, "atapi_r: samtag=%02x\n", data );
			break;
		case ATAPI_REG_COUNTLOW:
			verboselog( machine(), 1, "atapi_r: countlow=%02x\n", data );
			break;
		case ATAPI_REG_COUNTHIGH:
			verboselog( machine(), 1, "atapi_r: counthigh=%02x\n", data );
			break;
		case ATAPI_REG_DRIVESEL:
			verboselog( machine(), 1, "atapi_r: drivesel=%02x\n", data );
			break;
		case ATAPI_REG_CMDSTATUS:
			verboselog( machine(), 1, "atapi_r: cmdstatus=%02x\n", data );
			break;
		}

//      mame_printf_debug("ATAPI: read reg %d = %x (PC=%x)\n", reg, data, space.device().safe_pc());

		data <<= shift;
	}

	verboselog( machine(), 2, "atapi_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
	return data;
}

WRITE32_MEMBER(ksys573_state::atapi_w)
{
	UINT8 *atapi_regs = m_atapi_regs;
	UINT8 *atapi_data = m_atapi_data;
	int reg;

	verboselog( machine(), 2, "atapi_w( %08x, %08x, %08x )\n", offset, mem_mask, data );

	if (mem_mask == 0x0000ffff)	// word-wide command write
	{
		verboselog( machine(), 2, "atapi_w: data=%04x\n", data );

//      mame_printf_debug("ATAPI: packet write %04x\n", data);
		atapi_data[m_atapi_data_ptr++] = data & 0xff;
		atapi_data[m_atapi_data_ptr++] = data >> 8;

		if (m_atapi_cdata_wait)
		{
//          mame_printf_debug("ATAPI: waiting, ptr %d wait %d\n", m_atapi_data_ptr, m_atapi_cdata_wait);
			if (m_atapi_data_ptr == m_atapi_cdata_wait)
			{
				// send it to the device
				m_inserted_cdrom->WriteData( atapi_data, m_atapi_cdata_wait );

				// assert IRQ
				psx_irq_set(machine(), 0x400);

				// not sure here, but clear DRQ at least?
				atapi_regs[ATAPI_REG_CMDSTATUS] = 0;
			}
		}

		else if ( m_atapi_data_ptr == 12 )
		{
			int phase;

			verboselog( machine(), 2, "atapi_w: command %02x\n", atapi_data[0]&0xff );

			// reset data pointer for reading SCSI results
			m_atapi_data_ptr = 0;
			m_atapi_data_len = 0;

			// send it to the SCSI device
			m_inserted_cdrom->SetCommand( m_atapi_data, 12 );
			m_inserted_cdrom->ExecCommand( &m_atapi_xferlen );
			m_inserted_cdrom->GetPhase( &phase );

			if (m_atapi_xferlen != -1)
			{
//              mame_printf_debug("ATAPI: SCSI command %02x returned %d bytes from the device\n", atapi_data[0]&0xff, m_atapi_xferlen);

				// store the returned command length in the ATAPI regs, splitting into
				// multiple transfers if necessary
				m_atapi_xfermod = 0;
				if (m_atapi_xferlen > MAX_TRANSFER_SIZE)
				{
					m_atapi_xfermod = m_atapi_xferlen - MAX_TRANSFER_SIZE;
					m_atapi_xferlen = MAX_TRANSFER_SIZE;
				}

				atapi_regs[ATAPI_REG_COUNTLOW] = m_atapi_xferlen & 0xff;
				atapi_regs[ATAPI_REG_COUNTHIGH] = (m_atapi_xferlen>>8)&0xff;

				if (m_atapi_xferlen == 0)
				{
					// if no data to return, set the registers properly
					atapi_regs[ATAPI_REG_CMDSTATUS] = ATAPI_STAT_DRDY;
					atapi_regs[ATAPI_REG_INTREASON] = ATAPI_INTREASON_IO|ATAPI_INTREASON_COMMAND;
				}
				else
				{
					// indicate data ready: set DRQ and DMA ready, and IO in INTREASON
					atapi_regs[ATAPI_REG_CMDSTATUS] = ATAPI_STAT_DRQ | ATAPI_STAT_SERVDSC;
					atapi_regs[ATAPI_REG_INTREASON] = ATAPI_INTREASON_IO;
				}

				switch( phase )
				{
				case SCSI_PHASE_DATAOUT:
					m_atapi_cdata_wait = m_atapi_xferlen;
					break;
				}

				// perform special ATAPI processing of certain commands
				switch (atapi_data[0]&0xff)
				{
					case 0x00: // BUS RESET / TEST UNIT READY
					case 0xbb: // SET CDROM SPEED
						atapi_regs[ATAPI_REG_CMDSTATUS] = 0;
						break;

					case 0x45: // PLAY
						atapi_regs[ATAPI_REG_CMDSTATUS] = ATAPI_STAT_BSY;
						m_atapi_timer->adjust( downcast<cpu_device *>(&space.device())->cycles_to_attotime( ATAPI_CYCLES_PER_SECTOR ) );
						break;
				}

				// assert IRQ
				psx_irq_set(machine(), 0x400);
			}
			else
			{
//              mame_printf_debug("ATAPI: SCSI device returned error!\n");

				atapi_regs[ATAPI_REG_CMDSTATUS] = ATAPI_STAT_DRQ | ATAPI_STAT_CHECK;
				atapi_regs[ATAPI_REG_ERRFEAT] = 0x50;	// sense key = ILLEGAL REQUEST
				atapi_regs[ATAPI_REG_COUNTLOW] = 0;
				atapi_regs[ATAPI_REG_COUNTHIGH] = 0;
			}
		}
	}
	else
	{
		reg = offset<<1;
		if (mem_mask == 0x00ff0000)
		{
			reg += 1;
			data >>= 16;
		}

		switch( reg )
		{
		case ATAPI_REG_DATA:
			verboselog( machine(), 1, "atapi_w: data=%02x\n", data );
			break;
		case ATAPI_REG_ERRFEAT:
			verboselog( machine(), 1, "atapi_w: errfeat=%02x\n", data );
			break;
		case ATAPI_REG_INTREASON:
			verboselog( machine(), 1, "atapi_w: intreason=%02x\n", data );
			break;
		case ATAPI_REG_SAMTAG:
			verboselog( machine(), 1, "atapi_w: samtag=%02x\n", data );
			break;
		case ATAPI_REG_COUNTLOW:
			verboselog( machine(), 1, "atapi_w: countlow=%02x\n", data );
			break;
		case ATAPI_REG_COUNTHIGH:
			verboselog( machine(), 1, "atapi_w: counthigh=%02x\n", data );
			break;
		case ATAPI_REG_DRIVESEL:
			verboselog( machine(), 1, "atapi_w: drivesel=%02x\n", data );
			break;
		case ATAPI_REG_CMDSTATUS:
			verboselog( machine(), 1, "atapi_w: cmdstatus=%02x\n", data );
			break;
		}

		atapi_regs[reg] = data;
//      mame_printf_debug("ATAPI: reg %d = %x (offset %x mask %x PC=%x)\n", reg, data, offset, mem_mask, space.device().safe_pc());

		if (reg == ATAPI_REG_CMDSTATUS)
		{
//          mame_printf_debug("ATAPI command %x issued! (PC=%x)\n", data, space.device().safe_pc());

			switch (data)
			{
				case 0xa0:	// PACKET
					atapi_regs[ATAPI_REG_CMDSTATUS] = ATAPI_STAT_DRQ;
					atapi_regs[ATAPI_REG_INTREASON] = ATAPI_INTREASON_COMMAND;

					m_atapi_data_ptr = 0;
					m_atapi_data_len = 0;

					/* we have no data */
					m_atapi_xferlen = 0;
					m_atapi_xfermod = 0;

					m_atapi_cdata_wait = 0;
					break;

				case 0xa1:	// IDENTIFY PACKET DEVICE
					atapi_regs[ATAPI_REG_CMDSTATUS] = ATAPI_STAT_DRQ;

					m_atapi_data_ptr = 0;
					m_atapi_data_len = 512;

					/* we have no data */
					m_atapi_xferlen = 0;
					m_atapi_xfermod = 0;

					memset( atapi_data, 0, m_atapi_data_len );

					atapi_data[ 0 ^ 1 ] = 0x85;	// ATAPI device, cmd set 5 compliant, DRQ within 3 ms of PACKET command
					atapi_data[ 1 ^ 1 ] = 0x00;

					memset( &atapi_data[ 46 ], ' ', 8 );
					atapi_data[ 46 ^ 1 ] = '1';
					atapi_data[ 47 ^ 1 ] = '.';
					atapi_data[ 48 ^ 1 ] = '0';

					memset( &atapi_data[ 54 ], ' ', 40 );
					atapi_data[ 54 ^ 1 ] = 'M';
					atapi_data[ 55 ^ 1 ] = 'A';
					atapi_data[ 56 ^ 1 ] = 'T';
					atapi_data[ 57 ^ 1 ] = 'S';
					atapi_data[ 58 ^ 1 ] = 'H';
					atapi_data[ 59 ^ 1 ] = 'I';
					atapi_data[ 60 ^ 1 ] = 'T';
					atapi_data[ 61 ^ 1 ] = 'A';
					atapi_data[ 62 ^ 1 ] = ' ';
					atapi_data[ 63 ^ 1 ] = 'C';
					atapi_data[ 64 ^ 1 ] = 'R';
					atapi_data[ 65 ^ 1 ] = '-';
					atapi_data[ 66 ^ 1 ] = '5';
					atapi_data[ 67 ^ 1 ] = '8';
					atapi_data[ 68 ^ 1 ] = '9';
					atapi_data[ 69 ^ 1 ] = ' ';

					atapi_data[ 98 ^ 1 ] = 0x04; // IORDY may be disabled
					atapi_data[ 99 ^ 1 ] = 0x00;

					atapi_regs[ATAPI_REG_COUNTLOW] = 0;
					atapi_regs[ATAPI_REG_COUNTHIGH] = 2;

					psx_irq_set(machine(), 0x400);
					break;

				case 0xef:	// SET FEATURES
					atapi_regs[ATAPI_REG_CMDSTATUS] = 0;

					m_atapi_data_ptr = 0;
					m_atapi_data_len = 0;

					psx_irq_set(machine(), 0x400);
					break;

				default:
					mame_printf_debug("ATAPI: Unknown IDE command %x\n", data);
					break;
			}
		}
	}
}

static void atapi_init(running_machine &machine)
{
	ksys573_state *state = machine.driver_data<ksys573_state>();

	state->m_atapi_regs[ATAPI_REG_CMDSTATUS] = 0;
	state->m_atapi_regs[ATAPI_REG_ERRFEAT] = 1;
	state->m_atapi_regs[ATAPI_REG_COUNTLOW] = 0x14;
	state->m_atapi_regs[ATAPI_REG_COUNTHIGH] = 0xeb;

	state->m_atapi_data_ptr = 0;
	state->m_atapi_data_len = 0;
	state->m_atapi_cdata_wait = 0;

	state->m_atapi_timer = machine.scheduler().timer_alloc(FUNC(atapi_xfer_end));
	state->m_atapi_timer->adjust(attotime::never);

	state->m_available_cdroms[ 0 ] = machine.device<scsidev_device>( ":cdrom0" );
	if( get_disk_handle( machine, ":cdrom1" ) != NULL )
	{
		state->m_available_cdroms[ 1 ] = machine.device<scsidev_device>( ":cdrom1" );
	}
	else
	{
		state->m_available_cdroms[ 1 ] = NULL;
	}

	state->save_item( NAME(state->m_atapi_regs) );
	state->save_item( NAME(state->m_atapi_data) );
	state->save_item( NAME(state->m_atapi_data_ptr) );
	state->save_item( NAME(state->m_atapi_data_len) );
	state->save_item( NAME(state->m_atapi_xferlen) );
	state->save_item( NAME(state->m_atapi_xferbase) );
	state->save_item( NAME(state->m_atapi_cdata_wait) );
	state->save_item( NAME(state->m_atapi_xfermod) );
}

WRITE32_MEMBER(ksys573_state::atapi_reset_w)
{
	UINT8 *atapi_regs = m_atapi_regs;

	verboselog( machine(), 2, "atapi_reset_w( %08x, %08x, %08x )\n", offset, mem_mask, data );

	if (data)
	{
		verboselog( machine(), 2, "atapi_reset_w: reset\n" );

//      mame_printf_debug("ATAPI reset\n");

		atapi_regs[ATAPI_REG_CMDSTATUS] = 0;
		atapi_regs[ATAPI_REG_ERRFEAT] = 1;
		atapi_regs[ATAPI_REG_COUNTLOW] = 0x14;
		atapi_regs[ATAPI_REG_COUNTHIGH] = 0xeb;

		m_atapi_data_ptr = 0;
		m_atapi_data_len = 0;
		m_atapi_cdata_wait = 0;

		m_atapi_xferlen = 0;
		m_atapi_xfermod = 0;
	}
}

static void cdrom_dma_read( ksys573_state *state, UINT32 n_address, INT32 n_size )
{
	verboselog( state->machine(), 2, "cdrom_dma_read( %08x, %08x )\n", n_address, n_size );
//  mame_printf_debug("DMA read: address %08x size %08x\n", n_address, n_size);
}

static void cdrom_dma_write( ksys573_state *state, UINT32 n_address, INT32 n_size )
{
	verboselog( state->machine(), 2, "cdrom_dma_write( %08x, %08x )\n", n_address, n_size );
//  mame_printf_debug("DMA write: address %08x size %08x\n", n_address, n_size);

	state->m_atapi_xferbase = n_address;

	verboselog( state->machine(), 2, "atapi_xfer_end: %d %d\n", state->m_atapi_xferlen, state->m_atapi_xfermod );

	// set a transfer complete timer (Note: CYCLES_PER_SECTOR can't be lower than 2000 or the BIOS ends up "out of order")
	state->m_atapi_timer->adjust(state->machine().device<cpu_device>("maincpu")->cycles_to_attotime((ATAPI_CYCLES_PER_SECTOR * (state->m_atapi_xferlen/2048))));
}

WRITE32_MEMBER(ksys573_state::security_w)
{
	int security_cart_number = m_security_cart_number;
	COMBINE_DATA( &m_n_security_control );

	verboselog( machine(), 2, "security_w( %08x, %08x, %08x )\n", offset, mem_mask, data );

	if( ACCESSING_BITS_0_15 )
	{
		ds2401_device *ds2401 = machine().device<ds2401_device>(security_cart_number ? "game_id" : "install_id");
		device_secure_serial_flash *secflash = machine().device<device_secure_serial_flash>(security_cart_number ? "game_eeprom" : "install_eeprom");

		if( secflash ) {
			if( !dynamic_cast<zs01_device *>(secflash) )
				secflash->sda_w(( data >> 0 ) & 1);
			secflash->scl_w(( data >> 1 ) & 1);
			secflash->cs_w(( data >> 2 ) & 1);
			secflash->rst_w(( data >> 3 ) & 1);
		}

		if(ds2401)
		{
			ds2401->write(!( ( data >> 4 ) & 1 ));
		}

		if( m_security_callback != NULL )
		{
			(*m_security_callback)( machine(), data & 0xff );
		}
	}

	machine().root_device().ioport("OUT1")->write_safe( data, mem_mask );
}

READ32_MEMBER(ksys573_state::security_r)
{

	UINT32 data = m_n_security_control;
	verboselog( machine(), 2, "security_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
	return data;
}

READ32_MEMBER(ksys573_state::flash_r)
{
	UINT32 data = 0;

	if( m_flash_bank < 0 )
	{
		mame_printf_debug( "%08x: flash_r( %08x, %08x ) no bank selected %08x\n", space.device().safe_pc(), offset, mem_mask, m_control );
		data = 0xffffffff;
	}
	else
	{
		fujitsu_29f016a_device **flash_base = &m_flash_device[m_flash_bank >> 8][m_flash_bank & 0xff];
		int adr = offset * 2;

		if( ACCESSING_BITS_0_7 )
		{
			data |= ( flash_base[0]->read( adr + 0 ) & 0xff ) << 0; // 31m/31l/31j/31h
		}
		if( ACCESSING_BITS_8_15 )
		{
			data |= ( flash_base[1]->read( adr + 0 ) & 0xff ) << 8; // 27m/27l/27j/27h
		}
		if( ACCESSING_BITS_16_23 )
		{
			data |= ( flash_base[0]->read( adr + 1 ) & 0xff ) << 16; // 31m/31l/31j/31h
		}
		if( ACCESSING_BITS_24_31 )
		{
			data |= ( flash_base[1]->read( adr + 1 ) & 0xff ) << 24; // 27m/27l/27j/27h
		}
	}

	verboselog( machine(), 2, "flash_r( %08x, %08x, %08x) bank = %08x\n", offset, mem_mask, data, m_flash_bank );

	return data;
}

WRITE32_MEMBER(ksys573_state::flash_w)
{

	verboselog( machine(), 2, "flash_w( %08x, %08x, %08x\n", offset, mem_mask, data );

	if( m_flash_bank < 0 )
	{
		mame_printf_debug( "%08x: flash_w( %08x, %08x, %08x ) no bank selected %08x\n", space.device().safe_pc(), offset, mem_mask, data, m_control );
	}
	else
	{
		fujitsu_29f016a_device **flash_base = &m_flash_device[m_flash_bank >> 8][m_flash_bank & 0xff];
		int adr = offset * 2;

		if( ACCESSING_BITS_0_7 )
		{
			flash_base[0]->write( adr + 0, ( data >> 0 ) & 0xff );
		}
		if( ACCESSING_BITS_8_15 )
		{
			flash_base[1]->write( adr + 0, ( data >> 8 ) & 0xff );
		}
		if( ACCESSING_BITS_16_23 )
		{
			flash_base[0]->write( adr + 1, ( data >> 16 ) & 0xff );
		}
		if( ACCESSING_BITS_24_31 )
		{
			flash_base[1]->write( adr + 1, ( data >> 24 ) & 0xff );
		}
	}
}

static ADDRESS_MAP_START( konami573_map, AS_PROGRAM, 32, ksys573_state )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM	AM_SHARE("share1") /* ram */
	AM_RANGE(0x1f000000, 0x1f3fffff) AM_READWRITE(flash_r, flash_w )
	AM_RANGE(0x1f400000, 0x1f400003) AM_READ_PORT( "IN0" ) AM_WRITE_PORT( "OUT0" )
	AM_RANGE(0x1f400004, 0x1f400007) AM_READ(jamma_r )
	AM_RANGE(0x1f400008, 0x1f40000b) AM_READ_PORT( "IN2" )
	AM_RANGE(0x1f40000c, 0x1f40000f) AM_READ_PORT( "IN3" )
	AM_RANGE(0x1f480000, 0x1f48000f) AM_READWRITE(atapi_r, atapi_w )	// IDE controller, used mostly in ATAPI mode (only 3 pure IDE commands seen so far)
	AM_RANGE(0x1f500000, 0x1f500003) AM_READWRITE(control_r, control_w )	// Konami can't make a game without a "control" register.
	AM_RANGE(0x1f560000, 0x1f560003) AM_WRITE(atapi_reset_w )
	AM_RANGE(0x1f5c0000, 0x1f5c0003) AM_WRITENOP				// watchdog?
	AM_RANGE(0x1f620000, 0x1f623fff) AM_DEVREADWRITE8_LEGACY("m48t58", timekeeper_r, timekeeper_w, 0x00ff00ff)
	AM_RANGE(0x1f680000, 0x1f68001f) AM_READWRITE(mb89371_r, mb89371_w)
	AM_RANGE(0x1f6a0000, 0x1f6a0003) AM_READWRITE(security_r, security_w )
	AM_RANGE(0x1fc00000, 0x1fc7ffff) AM_ROM AM_SHARE("share2") AM_REGION("bios", 0)
	AM_RANGE(0x80000000, 0x803fffff) AM_RAM AM_SHARE("share1") /* ram mirror */
	AM_RANGE(0x9fc00000, 0x9fc7ffff) AM_ROM AM_SHARE("share2") /* bios mirror */
	AM_RANGE(0xa0000000, 0xa03fffff) AM_RAM AM_SHARE("share1") /* ram mirror */
	AM_RANGE(0xbfc00000, 0xbfc7ffff) AM_ROM AM_SHARE("share2") /* bios mirror */
	AM_RANGE(0xfffe0130, 0xfffe0133) AM_WRITENOP
ADDRESS_MAP_END



static void flash_init( running_machine &machine )
{
	// find onboard flash devices
	ksys573_state *state = machine.driver_data<ksys573_state>();
	astring tempstr;
	for (int index = 0; index < 8; index++)
		state->m_flash_device[0][index] = machine.device<fujitsu_29f016a_device>(tempstr.format("onboard.%d", index));

	// find pccard flash devices
	for (int card = 1; card <= 4; card++)
		for (int index = 0; index < 16; index++)
			state->m_flash_device[card][index] = machine.device<fujitsu_29f016a_device>(tempstr.format("pccard%d.%d", card, index));

	state->save_item( NAME(state->m_flash_bank) );
	state->save_item( NAME(state->m_control) );
}

static void *atapi_get_device(running_machine &machine)
{
	ksys573_state *state = machine.driver_data<ksys573_state>();
	void *ret;
	state->m_inserted_cdrom->GetDevice( &ret );
	return ret;
}

static void update_mode( running_machine &machine )
{
	ksys573_state *state = machine.driver_data<ksys573_state>();
	int cart = state->ioport("CART")->read();
	int cd = state->ioport( "CD" )->read();
	scsidev_device *new_cdrom;

	if( state->machine().device<device_secure_serial_flash>("game_eeprom") )
	{
		state->m_security_cart_number = cart;
	}
	else
	{
		state->m_security_cart_number = 0;
	}

	if( state->m_available_cdroms[ 1 ] != NULL )
	{
		new_cdrom = state->m_available_cdroms[ cd ];
	}
	else
	{
		new_cdrom = state->m_available_cdroms[ 0 ];
	}

	if( state->m_inserted_cdrom != new_cdrom )
	{
		state->m_inserted_cdrom = new_cdrom;
		cdda_set_cdrom(machine.device("cdda"), atapi_get_device(machine));
	}
}

DRIVER_INIT_MEMBER(ksys573_state,konami573)
{

	psx_driver_init(machine());
	atapi_init(machine());

	save_item( NAME(m_n_security_control) );

	flash_init(machine());
}

MACHINE_RESET_MEMBER(ksys573_state,konami573)
{

	if( machine().device<device_secure_serial_flash>("install_eeprom") )
	{
		/* security cart */
		psx_sio_input( machine(), 1, PSX_SIO_IN_DSR, PSX_SIO_IN_DSR );
	}

	m_flash_bank = -1;

	update_mode(machine());
}

static void spu_irq(device_t *device, UINT32 data)
{
	if (data)
	{
		psx_irq_set(device->machine(), 1<<9);
	}
}

void sys573_vblank(ksys573_state *state, screen_device &screen, bool vblank_state)
{
	UINT32 *p_n_psxram = state->m_p_n_psxram;

	update_mode(state->machine());

	if( strcmp( state->machine().system().name, "ddr2ml" ) == 0 )
	{
		/* patch out security-plate error */

		/* install cd */

		/* 801e1540: jal $801e1f7c */
		if( p_n_psxram[ 0x1e1540 / 4 ] == 0x0c0787df )
		{
			/* 801e1540: j $801e1560 */
			p_n_psxram[ 0x1e1540 / 4 ] = 0x08078558;
		}

		/* flash */

		/* 8001f850: jal $80031fd8 */
		if( p_n_psxram[ 0x1f850 / 4 ] == 0x0c00c7f6 )
		{
			/* 8001f850: j $8001f888 */
			p_n_psxram[ 0x1f850 / 4 ] = 0x08007e22;
		}
	}
	else if( strcmp( state->machine().system().name, "ddr2mla" ) == 0 )
	{
		/* patch out security-plate error */

		/* 8001f850: jal $8003221c */
		if( p_n_psxram[ 0x1f850 / 4 ] == 0x0c00c887 )
		{
			/* 8001f850: j $8001f888 */
			p_n_psxram[ 0x1f850 / 4 ] = 0x08007e22;
		}
	}
}

/*
GE765-PWB(B)A

todo:
  find out what offset 4 is
  fix reel type detection
  find adc0834 SARS

*/

READ32_MEMBER(ksys573_state::ge765pwbba_r)
{
	device_t *upd4701 = machine().device("upd4701");
	UINT32 data = 0;

	switch (offset)
	{
	case 0x26:
		upd4701_y_add(upd4701, 0, ioport("uPD4701_y")->read_safe(0), 0xffff);
		upd4701_switches_set(upd4701, 0, ioport("uPD4701_switches")->read_safe(0));

		upd4701_cs_w(upd4701, 0, 0);
		upd4701_xy_w(upd4701, 0, 1);

		if (ACCESSING_BITS_0_7)
		{
			upd4701_ul_w(upd4701, 0, 0);
			data |= upd4701_d_r(upd4701, 0, 0xffff) << 0;
		}

		if (ACCESSING_BITS_16_23)
		{
			upd4701_ul_w(upd4701, 0, 1);
			data |= upd4701_d_r(upd4701, 0, 0xffff) << 16;
		}

		upd4701_cs_w(upd4701, 0, 1);
		break;

	default:
		verboselog(machine(), 0, "ge765pwbba_r: unhandled offset %08x %08x\n", offset, mem_mask);
		break;
	}

	verboselog(machine(), 2, "ge765pwbba_r( %08x, %08x ) %08x\n", offset, mem_mask, data);
	return data;
}

WRITE32_MEMBER(ksys573_state::ge765pwbba_w)
{
	device_t *upd4701 = machine().device("upd4701");
	switch (offset)
	{
	case 0x04:
		break;

	case 0x20:
		if (ACCESSING_BITS_0_7)
		{
			output_set_value("motor", data & 0xff);
		}
		break;

	case 0x22:
		if (ACCESSING_BITS_0_7)
		{
			output_set_value("brake", data & 0xff);
		}
		break;

	case 0x28:
		if (ACCESSING_BITS_0_7)
		{
			upd4701_resety_w(upd4701, 0, 1);
			upd4701_resety_w(upd4701, 0, 0);
		}
		break;

	default:
		verboselog(machine(), 0, "ge765pwbba_w: unhandled offset %08x %08x %08x\n", offset, mem_mask, data);
		break;
	}

	verboselog(machine(), 2, "ge765pwbba_w( %08x, %08x, %08x )\n", offset, mem_mask, data);
}

DRIVER_INIT_MEMBER(ksys573_state,ge765pwbba)
{
	DRIVER_INIT_CALL(konami573);
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_readwrite_handler( 0x1f640000, 0x1f6400ff, read32_delegate(FUNC(ksys573_state::ge765pwbba_r),this), write32_delegate(FUNC(ksys573_state::ge765pwbba_w),this));
}

/*

GX700-PWB(F)

Analogue I/O board

*/

READ32_MEMBER(ksys573_state::gx700pwbf_io_r)
{
	UINT32 data = 0;
	switch( offset )
	{
	case 0x20:
		/* result not used? */
		break;

	case 0x22:
		/* result not used? */
		break;

	case 0x24:
		/* result not used? */
		break;

	case 0x26:
		/* result not used? */
		break;

	default:
//      printf( "gx700pwbf_io_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
		break;
	}

	verboselog( machine(), 2, "gx700pwbf_io_r( %08x, %08x ) %08x\n", offset, mem_mask, data );

	return data;
}

static void gx700pwbf_output( running_machine &machine, int offset, UINT8 data )
{
	ksys573_state *state = machine.driver_data<ksys573_state>();

	if( state->m_gx700pwfbf_output_callback != NULL )
	{
		int i;
		static const int shift[] = { 7, 6, 1, 0, 5, 4, 3, 2 };
		for( i = 0; i < 8; i++ )
		{
			int oldbit = ( state->m_gx700pwbf_output_data[ offset ] >> shift[ i ] ) & 1;
			int newbit = ( data >> shift[ i ] ) & 1;
			if( oldbit != newbit )
			{
				(*state->m_gx700pwfbf_output_callback)( machine, ( offset * 8 ) + i, newbit );
			}
		}
	}
	state->m_gx700pwbf_output_data[ offset ] = data;
}

WRITE32_MEMBER(ksys573_state::gx700pwbf_io_w)
{
	verboselog( machine(), 2, "gx700pwbf_io_w( %08x, %08x, %08x )\n", offset, mem_mask, data );

	switch( offset )
	{
	case 0x20:

		if( ACCESSING_BITS_0_15 )
		{
			gx700pwbf_output( machine(), 0, data & 0xff );
		}
		break;

	case 0x22:
		if( ACCESSING_BITS_0_15 )
		{
			gx700pwbf_output( machine(), 1, data & 0xff );
		}
		break;

	case 0x24:
		if( ACCESSING_BITS_0_15 )
		{
			gx700pwbf_output( machine(), 2, data & 0xff );
		}
		break;

	case 0x26:
		if( ACCESSING_BITS_0_15 )
		{
			gx700pwbf_output( machine(), 3, data & 0xff );
		}
		break;

	default:
//      printf( "gx700pwbf_io_w( %08x, %08x, %08x )\n", offset, mem_mask, data );
		break;
	}
}

static void gx700pwfbf_init( running_machine &machine, void (*output_callback_func)( running_machine &machine, int offset, int data ) )
{
	ksys573_state *state = machine.driver_data<ksys573_state>();

	memset( state->m_gx700pwbf_output_data, 0, sizeof( state->m_gx700pwbf_output_data ) );

	state->m_gx700pwfbf_output_callback = output_callback_func;

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_readwrite_handler( 0x1f640000, 0x1f6400ff, read32_delegate(FUNC(ksys573_state::gx700pwbf_io_r),state), write32_delegate(FUNC(ksys573_state::gx700pwbf_io_w),state));

	state->save_item( NAME(state->m_gx700pwbf_output_data) );
}

/*

GN845-PWB(B)

DDR Stage Multiplexor

*/

#define DDR_STAGE_IDLE ( 0 )
#define DDR_STAGE_INIT ( 1 )

static const int mask[] =
{
	0, 6, 2, 4,
	0, 4, 0, 4,
	0, 4, 0, 4,
	0, 4, 0, 4,
	0, 4, 0, 4,
	0, 4, 0, 6
};

static void gn845pwbb_do_w( running_machine &machine, int offset, int data )
{
	ksys573_state *state = machine.driver_data<ksys573_state>();

	state->m_stage[ offset ].DO = !data;
}

static void gn845pwbb_clk_w( running_machine &machine, int offset, int data )
{
	ksys573_state *state = machine.driver_data<ksys573_state>();
	int clk = !data;

	if( clk != state->m_stage[ offset ].clk )
	{
		state->m_stage[ offset ].clk = clk;

		if( clk )
		{
			state->m_stage[ offset ].shift = ( state->m_stage[ offset ].shift >> 1 ) | ( state->m_stage[ offset ].DO << 12 );

			switch( state->m_stage[ offset ].state )
			{
			case DDR_STAGE_IDLE:
				if( state->m_stage[ offset ].shift == 0xc90 )
				{
					state->m_stage[ offset ].state = DDR_STAGE_INIT;
					state->m_stage[ offset ].bit = 0;
					state->m_stage_mask = 0xfffff9f9;
				}
				break;

			case DDR_STAGE_INIT:
				state->m_stage[ offset ].bit++;
				if( state->m_stage[ offset ].bit < 22 )
				{
					int a = ( ( ( ( ~0x06 ) | mask[ state->m_stage[ 0 ].bit ] ) & 0xff ) << 8 );
					int b = ( ( ( ( ~0x06 ) | mask[ state->m_stage[ 1 ].bit ] ) & 0xff ) << 0 );

					state->m_stage_mask = 0xffff0000 | a | b;
				}
				else
				{
					state->m_stage[ offset ].bit = 0;
					state->m_stage[ offset ].state = DDR_STAGE_IDLE;

					state->m_stage_mask = 0xffffffff;
				}
				break;
			}
		}
	}

	verboselog( machine, 2, "stage: %dp data clk=%d state=%d d0=%d shift=%08x bit=%d stage_mask=%08x\n", offset + 1, clk,
		state->m_stage[ offset ].state, state->m_stage[ offset ].DO, state->m_stage[ offset ].shift, state->m_stage[ offset ].bit, state->m_stage_mask );
}

CUSTOM_INPUT_MEMBER(ksys573_state::gn845pwbb_read)
{

	return ioport("STAGE")->read() & m_stage_mask;
}

static void gn845pwbb_output_callback( running_machine &machine, int offset, int data )
{
	switch( offset )
	{
	case 0:
		output_set_value( "foot 1p up", !data );
		break;

	case 1:
		output_set_value( "foot 1p left", !data );
		break;

	case 2:
		output_set_value( "foot 1p right", !data );
		break;

	case 3:
		output_set_value( "foot 1p down", !data );
		break;

	case 4:
		gn845pwbb_do_w( machine, 0, !data );
		break;

	case 7:
		gn845pwbb_clk_w( machine, 0, !data );
		break;

	case 8:
		output_set_value( "foot 2p up", !data );
		break;

	case 9:
		output_set_value( "foot 2p left", !data );
		break;

	case 10:
		output_set_value( "foot 2p right", !data );
		break;

	case 11:
		output_set_value( "foot 2p down", !data );
		break;

	case 12:
		gn845pwbb_do_w( machine, 1, !data );
		break;

	case 15:
		gn845pwbb_clk_w( machine, 1, !data );
		break;

	case 17:
		output_set_led_value( 0, !data ); // start 1
		break;

	case 18:
		output_set_led_value( 1, !data ); // start 2
		break;

	case 20:
		output_set_value( "body right low", !data );
		break;

	case 21:
		output_set_value( "body left low", !data );
		break;

	case 22:
		output_set_value( "body left high", !data );
		break;

	case 23:
		output_set_value( "body right high", !data );
		break;

	case 28: // digital
	case 30: // analogue
		output_set_value( "speaker", !data );
		break;

	default:
//        printf( "%d=%d\n", offset, data );
		break;
	}
}

DRIVER_INIT_MEMBER(ksys573_state,ddr)
{

	DRIVER_INIT_CALL(konami573);

	m_stage_mask = 0xffffffff;
	gx700pwfbf_init( machine(), gn845pwbb_output_callback );

	save_item( NAME(m_stage_mask) );
}

/*

Guitar Freaks

todo:
  find out what offset 4 is
  find out the pcb id

*/

READ32_MEMBER(ksys573_state::gtrfrks_io_r)
{
	UINT32 data = 0;
	switch( offset )
	{
	case 0:
		break;

	default:
		verboselog( machine(), 0, "gtrfrks_io_r: unhandled offset %08x, %08x\n", offset, mem_mask );
		break;
	}

	verboselog( machine(), 2, "gtrfrks_io_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
	return data;
}

WRITE32_MEMBER(ksys573_state::gtrfrks_io_w)
{
	verboselog( machine(), 2, "gtrfrks_io_w( %08x, %08x ) %08x\n", offset, mem_mask, data );

	switch( offset )
	{
	case 0:
		output_set_value( "spot left", !( ( data >> 7 ) & 1 ) );
		output_set_value( "spot right", !( ( data >> 6 ) & 1 ) );
		output_set_led_value( 0, !( ( data >> 5 ) & 1 ) ); // start left
		output_set_led_value( 1, !( ( data >> 4 ) & 1 ) ); // start right
		break;

	case 4:
		break;

	default:
		verboselog( machine(), 0, "gtrfrks_io_w: unhandled offset %08x, %08x\n", offset, mem_mask );
		break;
	}
}

DRIVER_INIT_MEMBER(ksys573_state,gtrfrks)
{
	DRIVER_INIT_CALL(konami573);
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_readwrite_handler( 0x1f600000, 0x1f6000ff, read32_delegate(FUNC(ksys573_state::gtrfrks_io_r),this), write32_delegate(FUNC(ksys573_state::gtrfrks_io_w),this));
}

/* GX894 digital i/o */

READ32_MEMBER(ksys573_state::gx894pwbba_r)
{
	UINT32 data = 0;

	switch( offset )
	{
	case 0x00:
		data |= 0x10000;
		break;
	case 0x20:
		if( ACCESSING_BITS_0_15 )
		{
			data |= 0x00001234;
		}
		break;
	case 0x2b:
		if( ACCESSING_BITS_0_15 )
		{
			mas3507d_device *mas3507d = machine().device<mas3507d_device>("mpeg");
			data |= mas3507d->i2c_scl_r() << 13;
			data |= mas3507d->i2c_sda_r() << 12;
		}
		if( ACCESSING_BITS_16_31 )
		{
//          data |= 0x10000000; /* rdy??? */
		}
		break;
	case 0x2d:
		if( ACCESSING_BITS_0_15 )
		{
			data |= m_gx894_ram[ m_gx894_ram_read_offset / 2 ];
			m_gx894_ram_read_offset += 2;
		}
		if( ACCESSING_BITS_16_31 )
		{
//          printf( "read offset 2d msw32\n" );
		}
		break;
	case 0x30:
		/* mp3? */
		if( ACCESSING_BITS_0_15 )
		{
			/* unknown data word */
		}
		if( ACCESSING_BITS_16_31 )
		{
			/* 0x000-0x1ff */
			data |= 0x1ff0000;
		}
		break;
	case 0x31:
		/* mp3? */
		if( ACCESSING_BITS_0_15 )
		{
			/* unknown data word count */
			data |= 0x0000;
		}
		if( ACCESSING_BITS_16_31 )
		{
//          printf( "read offset 31 msw32\n" );
		}
		break;
	case 0x32:
		if( ACCESSING_BITS_16_31 )
		{
			data |= 0 & 0xffff0000;
		}
		/* todo */
		break;
	case 0x33:
		if( ACCESSING_BITS_0_15 )
		{
			data |= 0 & 0x0000ffff;
		}
		/* todo */
		break;
	case 0x3b:
		if( ACCESSING_BITS_16_31 )
		{
			data |= machine().device<ds2401_device>("digital_id")->read() << 28;
		}
		break;
	case 0x3d:
		if( ACCESSING_BITS_16_31 )
		{
			// fpga/digital board status checks
			// wants & c000 = 8000 (just after program upload?)
			// write 0000 to +f4.w
			// write 8000 to +f6.w

			/* fails if !8000 */
			/* fails if  4000 */
			/* fails if !2000 */
			/* fails if !1000 */
			data |= ( 0x8000 | 0x2000 | 0x1000 ) << 16;
		}
		break;
	default:
//      printf( "read offset %08x\n", offset );
		break;
	}

	verboselog( machine(), 2, "gx894pwbba_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
//  printf( "%08x: gx894pwbba_r( %08x, %08x ) %08x\n", space.device().safe_pc(), offset, mem_mask, data );
	return data;
}

static char *binary( char *s, UINT32 data )
{
	int i;
	for( i = 0; i < 32; i++ )
	{
		s[ i ] = '0' + ( ( data >> ( 31 - i ) ) & 1 );
	}
	s[ i ] = 0;
	return s;
}

static void gx894pwbba_output( running_machine &machine, int offset, UINT8 data )
{
	ksys573_state *state = machine.driver_data<ksys573_state>();

	if( state->m_gx894pwbba_output_callback != NULL )
	{
		int i;
		static const int shift[] = { 0, 2, 3, 1 };
		for( i = 0; i < 4; i++ )
		{
			int oldbit = ( state->m_gx894pwbba_output_data[ offset ] >> shift[ i ] ) & 1;
			int newbit = ( data >> shift[ i ] ) & 1;
			if( oldbit != newbit )
			{
				(*state->m_gx894pwbba_output_callback)( machine, ( offset * 4 ) + i, newbit );
			}
		}
	}
	state->m_gx894pwbba_output_data[ offset ] = data;
}

WRITE32_MEMBER(ksys573_state::gx894pwbba_w)
{
	char buff[33];
	UINT32 olda=m_a,oldb=m_b,oldc=m_c,oldd=m_d;

//  printf( "gx894pwbba_w( %08x, %08x, %08x )\n", offset, mem_mask, data );

	if( offset == 4 )
	{
		return;
	}

	verboselog( machine(), 2, "gx894pwbba_w( %08x, %08x, %08x) %s\n", offset, mem_mask, data, binary( buff, data ) );

	switch( offset )
	{
	case 0x28:
		if(ACCESSING_BITS_0_15)
			logerror("FPGA MPEG start address high %04x\n", data);
		if(ACCESSING_BITS_16_31)
			logerror("FPGA MPEG start address low %04x\n", data >> 16);
		break;
	case 0x29:
		if(ACCESSING_BITS_0_15)
			logerror("FPGA MPEG end address high %04x\n", data);
		if(ACCESSING_BITS_16_31)
			logerror("FPGA MPEG end address low %04x\n", data >> 16);
		break;
	case 0x2a:
		if(ACCESSING_BITS_0_15)
			logerror("FPGA MPEG key 1/3 %04x\n", data);
		break;
	case 0x2b:
		if(ACCESSING_BITS_0_15) {
			mas3507d_device *mas3507d = machine().device<mas3507d_device>("mpeg");
			mas3507d->i2c_scl_w(data & 0x2000);
			mas3507d->i2c_sda_w(data & 0x1000);
		}
		if( ACCESSING_BITS_16_31 )
		{
			logerror("FPGA MPEG control %c%c%c\n",
					 data & 0x80000000 ? '#' : '.',
					 data & 0x40000000 ? '#' : '.',
					 data & 0x20000000 ? '#' : '.');
		}
		break;
	case 0x2c:
		if( ACCESSING_BITS_0_15 )
		{
			m_gx894_ram_write_offset &= 0x0000ffff;
			m_gx894_ram_write_offset |= ( data & 0x0000ffff ) << 16;
		}
		if( ACCESSING_BITS_16_31 )
		{
			m_gx894_ram_write_offset &= 0xffff0000;
			m_gx894_ram_write_offset |= ( data & 0xffff0000 ) >> 16;
		}
		break;
	case 0x2d:
		if( ACCESSING_BITS_0_15 )
		{
			m_gx894_ram[ m_gx894_ram_write_offset / 2 ] = data & 0xffff;
			m_gx894_ram_write_offset += 2;
		}
		if( ACCESSING_BITS_16_31 )
		{
			m_gx894_ram_read_offset &= 0x0000ffff;
			m_gx894_ram_read_offset |= ( data & 0xffff0000 ) << 0;
		}
		break;
	case 0x2e:
		if( ACCESSING_BITS_0_15 )
		{
			m_gx894_ram_read_offset &= 0xffff0000;
			m_gx894_ram_read_offset |= ( data & 0x0000ffff ) >> 0;
		}
		if( ACCESSING_BITS_16_31 )
		{
//          printf( "write offset 2e msw32\n" );
		}
		break;
	case 0x38:
		if( ACCESSING_BITS_16_31 )
		{
			gx894pwbba_output( machine(), 0, ( data >> 28 ) & 0xf );
		}
		if( ACCESSING_BITS_0_15 )
		{
			gx894pwbba_output( machine(), 1, ( data >> 12 ) & 0xf );
		}
		COMBINE_DATA( &m_a );
		break;
	case 0x39:
		if( ACCESSING_BITS_16_31 )
		{
			gx894pwbba_output( machine(), 7, ( data >> 28 ) & 0xf );
		}
		if( ACCESSING_BITS_0_15 )
		{
			gx894pwbba_output( machine(), 3, ( data >> 12 ) & 0xf );
		}
		COMBINE_DATA( &m_b );
		break;
	case 0x3a:
		if( ACCESSING_BITS_16_31 )
			logerror("FPGA MPEG key 2/3 %04x\n", data >> 16);
		break;
	case 0x3b:
		if( ACCESSING_BITS_0_15 )
			logerror("FPGA MPEG key 3/3 %02x\n", data & 0xff);
		if( ACCESSING_BITS_16_31 )
		{
			machine().device<ds2401_device>("digital_id")->write( !( ( data >> 28 ) & 1 ) );
		}
		break;
	case 0x3e:
		if( ACCESSING_BITS_0_15 )
		{
			/* 12 */
			/* 13 */
			/* 14 */
			/* 15 */

			/* fpga */
			m_s = ( m_s >> 1 ) | ( ( data & 0x8000 ) >> 8 );
			m_f++;
			if( m_f == 8 )
			{
//              printf( "%04x %02x\n", o, s );
				m_c = 0;
				m_f = 0;
				m_o++;
			}
		}

		if( ACCESSING_BITS_16_31 )
		{
			gx894pwbba_output( machine(), 4, ( data >> 28 ) & 0xf );
		}
		COMBINE_DATA( &m_c );
		break;
	case 0x3f:
		if( ACCESSING_BITS_16_31 )
		{
			gx894pwbba_output( machine(), 2, ( data >> 28 ) & 0xf );
		}
		if( ACCESSING_BITS_0_15 )
		{
			gx894pwbba_output( machine(), 5, ( data >> 12 ) & 0xf );
		}
		COMBINE_DATA( &m_d );
		break;
	default:
//      printf( "write offset %08x\n", offset );
		break;
	}
	if( m_a != olda || m_b != oldb || m_c != oldc || m_d != oldd )
	{
//      printf( "%08x %08x %08x %08x\n", a, b, c, d );
	}
}

static void gx894pwbba_init( running_machine &machine, void (*output_callback_func)( running_machine &machine, int offset, int data ) )
{
	ksys573_state *state = machine.driver_data<ksys573_state>();
	int gx894_ram_size = 24 * 1024 * 1024;

	state->m_gx894pwbba_output_callback = output_callback_func;

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_readwrite_handler( 0x1f640000, 0x1f6400ff, read32_delegate(FUNC(ksys573_state::gx894pwbba_r),state), write32_delegate(FUNC(ksys573_state::gx894pwbba_w),state));

	state->m_gx894_ram_write_offset = 0;
	state->m_gx894_ram_read_offset = 0;
	state->m_gx894_ram = auto_alloc_array( machine, UINT16, gx894_ram_size/2 );

	state->save_item( NAME(state->m_gx894pwbba_output_data) );
	state->save_pointer( NAME(state->m_gx894_ram), gx894_ram_size / 4 );
}

/* ddr digital */

DRIVER_INIT_MEMBER(ksys573_state,ddrdigital)
{
	DRIVER_INIT_CALL(konami573);

	gx894pwbba_init( machine(), gn845pwbb_output_callback );
}

/* guitar freaks digital */

DRIVER_INIT_MEMBER(ksys573_state,gtrfrkdigital)
{
	DRIVER_INIT_CALL(konami573);

	gx894pwbba_init( machine(), NULL );
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_readwrite_handler( 0x1f600000, 0x1f6000ff, read32_delegate(FUNC(ksys573_state::gtrfrks_io_r),this), write32_delegate(FUNC(ksys573_state::gtrfrks_io_w),this) );
}

/* ddr solo */

static void ddrsolo_output_callback( running_machine &machine, int offset, int data )
{
	switch( offset )
	{
	case 4:
	case 7:
	case 12:
	case 15:
		/* DDR stage i/o */
		break;

	case 8:
		output_set_value( "extra 4", !data );
		break;

	case 9:
		output_set_value( "extra 2", !data );
		break;

	case 10:
		output_set_value( "extra 1", !data );
		break;

	case 11:
		output_set_value( "extra 3", !data );
		break;

	case 16:
		output_set_value( "speaker", !data );
		break;

	case 20:
		output_set_led_value( 0, !data ); // start
		break;

	case 21:
		output_set_value( "body center", !data );
		break;

	case 22:
		output_set_value( "body right", !data );
		break;

	case 23:
		output_set_value( "body left", !data );
		break;

	default:
//      printf( "%d=%d\n", offset, data );
		break;
	}
}

DRIVER_INIT_MEMBER(ksys573_state,ddrsolo)
{
	DRIVER_INIT_CALL(konami573);

	gx894pwbba_init( machine(), ddrsolo_output_callback );
}

/* drummania */

static void drmn_output_callback( running_machine &machine, int offset, int data )
{
	switch( offset )
	{
	case 0: // drmn2+
	case 16: // drmn
		output_set_value( "hi-hat", !data );
		break;

	case 1: // drmn2+
	case 17: // drmn
		output_set_value( "high tom", !data );
		break;

	case 2: // drmn2+
	case 18: // drmn
		output_set_value( "low tom", !data );
		break;

	case 3: // drmn2+
	case 19: // drmn
		output_set_value( "snare", !data );
		break;

	case 8: // drmn2+
	case 30: // drmn
		output_set_value( "spot left & right", !data );
		break;

	case 9: // drmn2+
	case 31: // drmn
		output_set_value( "neon top", data );
		break;

	case 11: // drmn2+
	case 27: // drmn
		output_set_value( "neon woofer", data );
		break;

	case 12: // drmn2+
	case 20: // drmn
		output_set_value( "cymbal", !data );
		break;

	case 13: // drmn2+
	case 21: // drmn
		output_set_led_value( 0, data ); // start
		break;

	case 14: // drmn2+
	case 22: // drmn
		output_set_value( "select button", data );
		break;

	case 23: // drmn
	case 26: // drmn
		break;

	default:
//      printf( "%d=%d\n", offset, data );
		break;
	}
}

DRIVER_INIT_MEMBER(ksys573_state,drmn)
{
	DRIVER_INIT_CALL(konami573);

	gx700pwfbf_init( machine(), drmn_output_callback );
}

DRIVER_INIT_MEMBER(ksys573_state,drmndigital)
{
	DRIVER_INIT_CALL(konami573);

	gx894pwbba_init( machine(), drmn_output_callback );
}

/* dance maniax */

static void dmx_output_callback( running_machine &machine, int offset, int data )
{
	switch( offset )
	{
	case 0:
		output_set_value( "blue io 8", !data );
		break;

	case 1:
		output_set_value( "blue io 9", !data );
		break;

	case 2:
		output_set_value( "red io 9", !data );
		break;

	case 3:
		output_set_value( "red io 8", !data );
		break;

	case 4:
		output_set_value( "blue io 6", !data );
		break;

	case 5:
		output_set_value( "blue io 7", !data );
		break;

	case 6:
		output_set_value( "red io 7", !data );
		break;

	case 7:
		output_set_value( "red io 6", !data );
		break;

	case 8:
		output_set_value( "blue io 4", !data );
		break;

	case 9:
		output_set_value( "blue io 5", !data );
		break;

	case 10:
		output_set_value( "red io 5", !data );
		break;

	case 11:
		output_set_value( "red io 4", !data );
		break;

	case 12:
		output_set_value( "blue io 10", !data );
		break;

	case 13:
		output_set_value( "blue io 11", !data );
		break;

	case 14:
		output_set_value( "red io 11", !data );
		break;

	case 15:
		output_set_value( "red io 10", !data );
		break;

	case 16:
		output_set_value( "blue io 0", !data );
		break;

	case 17:
		output_set_value( "blue io 1", !data );
		break;

	case 18:
		output_set_value( "red io 1", !data );
		break;

	case 19:
		output_set_value( "red io 0", !data );
		break;

	case 20:
		output_set_value( "blue io 2", !data );
		break;

	case 21:
		output_set_value( "blue io 3", !data );
		break;

	case 22:
		output_set_value( "red io 3", !data );
		break;

	case 23:
		output_set_value( "red io 2", !data );
		break;

	case 28:
		output_set_value( "yellow spot light", !data );
		break;

	case 29:
		output_set_value( "blue spot light", !data );
		break;

	case 31:
		output_set_value( "pink spot light", !data );
		break;

	default:
//      printf( "%d=%d\n", offset, data );
		break;
	}
}

WRITE32_MEMBER(ksys573_state::dmx_io_w)
{
	verboselog( machine(), 2, "dmx_io_w( %08x, %08x ) %08x\n", offset, mem_mask, data );

	switch( offset )
	{
	case 0:
		output_set_value( "left 2p", !( ( data >> 0 ) & 1 ) );
		output_set_led_value( 1, !( ( data >> 1 ) & 1 ) ); // start 1p
		output_set_value( "right 2p", !( ( data >> 2 ) & 1 ) );

		output_set_value( "left 1p", !( ( data >> 3 ) & 1 ) );
		output_set_led_value( 0, !( ( data >> 4 ) & 1 ) ); // start 2p
		output_set_value( "right 1p", !( ( data >> 5 ) & 1 ) );
		break;

	default:
		verboselog( machine(), 0, "dmx_io_w: unhandled offset %08x, %08x\n", offset, mem_mask );
		break;
	}
}

DRIVER_INIT_MEMBER(ksys573_state,dmx)
{
	DRIVER_INIT_CALL(konami573);

	gx894pwbba_init( machine(), dmx_output_callback );
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_write_handler(0x1f600000, 0x1f6000ff, write32_delegate(FUNC(ksys573_state::dmx_io_w),this) );
}

/* salary man champ */

static void salarymc_lamp_callback( running_machine &machine, int data )
{
	ksys573_state *state = machine.driver_data<ksys573_state>();
	int d = ( data >> 7 ) & 1;
	int rst = ( data >> 6 ) & 1;
	int clk = ( data >> 5 ) & 1;

	if( rst )
	{
		state->m_salarymc_lamp_bits = 0;
		state->m_salarymc_lamp_shift = 0;
	}

	if( state->m_salarymc_lamp_clk != clk )
	{
		state->m_salarymc_lamp_clk = clk;

		if( state->m_salarymc_lamp_clk )
		{
			state->m_salarymc_lamp_shift <<= 1;

			state->m_salarymc_lamp_shift |= d;

			state->m_salarymc_lamp_bits++;
			if( state->m_salarymc_lamp_bits == 16 )
			{
				if( ( state->m_salarymc_lamp_shift & ~0xe38 ) != 0 )
				{
					verboselog( machine, 0, "unknown bits in salarymc_lamp_shift %08x\n", state->m_salarymc_lamp_shift & ~0xe38 );
				}

				output_set_value( "player 1 red", ( state->m_salarymc_lamp_shift >> 11 ) & 1 );
				output_set_value( "player 1 green", ( state->m_salarymc_lamp_shift >> 10 ) & 1 );
				output_set_value( "player 1 blue", ( state->m_salarymc_lamp_shift >> 9 ) & 1 );

				output_set_value( "player 2 red", ( state->m_salarymc_lamp_shift >> 5 ) & 1 );
				output_set_value( "player 2 green", ( state->m_salarymc_lamp_shift >> 4 ) & 1 );
				output_set_value( "player 2 blue", ( state->m_salarymc_lamp_shift >> 3 ) & 1 );

				state->m_salarymc_lamp_bits = 0;
				state->m_salarymc_lamp_shift = 0;
			}
		}
	}
}

DRIVER_INIT_MEMBER(ksys573_state,salarymc)
{

	DRIVER_INIT_CALL(konami573);

	m_security_callback = salarymc_lamp_callback;

	save_item( NAME(m_salarymc_lamp_bits) );
	save_item( NAME(m_salarymc_lamp_shift) );
	save_item( NAME(m_salarymc_lamp_clk) );
}

/* Hyper Bishi Bashi Champ */

static void hyperbbc_lamp_callback( running_machine &machine, int data )
{
	ksys573_state *state = machine.driver_data<ksys573_state>();
	int red = ( data >> 6 ) & 1;
	int blue = ( data >> 5 ) & 1;
	int green = ( data >> 4 ) & 1;
	int strobe1 = ( data >> 3 ) & 1;
	int strobe2 = ( data >> 0 ) & 1;

	if( strobe1 && !state->m_hyperbbc_lamp_strobe1 )
	{
		output_set_value( "player 1 red", red );
		output_set_value( "player 1 green", green );
		output_set_value( "player 1 blue", blue );
	}

	state->m_hyperbbc_lamp_strobe1 = strobe1;

	if( strobe2 && !state->m_hyperbbc_lamp_strobe2 )
	{
		output_set_value( "player 2 red", red );
		output_set_value( "player 2 green", green );
		output_set_value( "player 2 blue", blue );
	}

	state->m_hyperbbc_lamp_strobe2 = strobe2;
}

DRIVER_INIT_MEMBER(ksys573_state,hyperbbc)
{

	DRIVER_INIT_CALL(konami573);

	m_security_callback = hyperbbc_lamp_callback;

	save_item( NAME(m_hyperbbc_lamp_strobe1) );
	save_item( NAME(m_hyperbbc_lamp_strobe2) );
}

/* Mambo A Go Go */

static void mamboagg_output_callback( running_machine &machine, int offset, int data )
{
	switch( offset )
	{
	case 4:
		output_set_value( "fire lamp left", !data );
		break;
	case 5:
		output_set_value( "fire fan left", !data );
		break;
	case 6:
		output_set_value( "fire fan right", !data );
		break;
	case 7:
		output_set_value( "fire lamp right", !data );
		break;
	case 28:
		output_set_value( "conga left", !data );
		break;
	case 29:
		output_set_value( "conga right", !data );
		break;
	case 31:
		output_set_value( "conga centre", !data );
		break;
	}
}

WRITE32_MEMBER(ksys573_state::mamboagg_io_w)
{
	verboselog( machine(), 2, "mamboagg_io_w( %08x, %08x ) %08x\n", offset, mem_mask, data );

	switch( offset )
	{
	case 0:
		output_set_led_value( 0, !( ( data >> 3 ) & 1 ) ); // start 1p
		output_set_value( "select right", !( ( data >> 4 ) & 1 ) );
		output_set_value( "select left", !( ( data >> 5 ) & 1 ) );
		break;

	default:
		verboselog( machine(), 0, "mamboagg_io_w: unhandled offset %08x, %08x\n", offset, mem_mask );
		break;
	}
}

DRIVER_INIT_MEMBER(ksys573_state,mamboagg)
{
	DRIVER_INIT_CALL(konami573);

	gx894pwbba_init( machine(), mamboagg_output_callback );
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_write_handler(0x1f600000, 0x1f6000ff, write32_delegate(FUNC(ksys573_state::mamboagg_io_w),this));
}


/* punch mania */


static double punchmania_inputs_callback( device_t *device, UINT8 input )
{
	ksys573_state *state = device->machine().driver_data<ksys573_state>();
	double *pad_position = state->m_pad_position;
	int pads = state->ioport("PADS")->read();
	for( int i = 0; i < 6; i++ )
	{
		if( ( pads & ( 1 << i ) ) != 0 )
		{
			pad_position[ i ] = 5;
		}
	}

	switch( input )
	{
	case ADC083X_CH0:
		return pad_position[ 0 ]; /* Left Top */
	case ADC083X_CH1:
		return pad_position[ 1 ]; /* Left Middle */
	case ADC083X_CH2:
		return pad_position[ 2 ]; /* Left Bottom */
	case ADC083X_CH3:
		return pad_position[ 3 ]; /* Right Top */
	case ADC083X_CH4:
		return pad_position[ 4 ]; /* Right Middle */
	case ADC083X_CH5:
		return pad_position[ 5 ]; /* Right Bottom */
	case ADC083X_COM:
		return 0;
	case ADC083X_VREF:
		return 5;
	}
	return 5;
}

int pad_light[ 6 ];

static void punchmania_output_callback( running_machine &machine, int offset, int data )
{
	ksys573_state *state = machine.driver_data<ksys573_state>();
	double *pad_position = state->m_pad_position;
	char pad[ 7 ];

	switch( offset )
	{
	case 8:
		output_set_value( "select_left_right", !data );
		break;
	case 9:
		pad_light[ 2 ] = !data;
		output_set_value( "left_bottom_lamp", !data );
		break;
	case 10:
		pad_light[ 1 ] = !data;
		output_set_value( "left_middle_lamp", !data );
		break;
	case 11:
		output_set_value( "start_lamp", !data );
		break;
	case 12:
		pad_light[ 0 ] = !data;
		output_set_value( "left_top_lamp", !data );
		break;
	case 13:
		pad_light[ 4 ] = !data;
		output_set_value( "right_middle_lamp", !data );
		break;
	case 14:
		pad_light[ 3 ] = !data;
		output_set_value( "right_top_lamp", !data );
		break;
	case 15:
		pad_light[ 5 ] = !data;
		output_set_value( "right_bottom_lamp", !data );
		break;
	case 16:
		if( data )
		{
			pad_position[ 0 ] = 0; // left top motor +
		}
		break;
	case 17:
		if( data )
		{
			pad_position[ 1 ] = 0; // left middle motor +
		}
		break;
	case 18:
		if( data )
		{
			pad_position[ 1 ] = 5; // left middle motor -
		}
		break;
	case 19:
		if( data )
		{
			pad_position[ 0 ] = 5; // left top motor -
		}
		break;
	case 20:
		if( data )
		{
			pad_position[ 2 ] = 0; // left bottom motor +
		}
		break;
	case 21:
		if( data )
		{
			pad_position[ 3 ] = 5; // right top motor -
		}
		break;
	case 22:
		if( data )
		{
			pad_position[ 3 ] = 0; // right top motor +
		}
		break;
	case 23:
		if( data )
		{
			pad_position[ 2 ] = 5; // left bottom motor -
		}
		break;
	case 26:
		if( data )
		{
			pad_position[ 5 ] = 0; // right bottom motor +
		}
		break;
	case 27:
		if( data )
		{
			pad_position[ 4 ] = 0; // right middle motor +
		}
		break;
	case 30:
		if( data )
		{
			pad_position[ 4 ] = 5; // right middle motor -
		}
		break;
	case 31:
		if( data )
		{
			pad_position[ 5 ] = 5; // right bottom motor -
		}
		break;
	}
	sprintf( pad, "%d%d%d%d%d%d",
		(int)pad_position[ 0 ], (int)pad_position[ 1 ], (int)pad_position[ 2 ],
		(int)pad_position[ 3 ], (int)pad_position[ 4 ], (int)pad_position[ 5 ] );

	if( pad_light[ 0 ] ) pad[ 0 ] = '*';
	if( pad_light[ 1 ] ) pad[ 1 ] = '*';
	if( pad_light[ 2 ] ) pad[ 2 ] = '*';
	if( pad_light[ 3 ] ) pad[ 3 ] = '*';
	if( pad_light[ 4 ] ) pad[ 4 ] = '*';
	if( pad_light[ 5 ] ) pad[ 5 ] = '*';

	popmessage( "%s", pad );
}

static const adc083x_interface punchmania_adc_interface = {
	punchmania_inputs_callback
};

DRIVER_INIT_MEMBER(ksys573_state,punchmania)
{
	DRIVER_INIT_CALL(konami573);

	gx700pwfbf_init( machine(), punchmania_output_callback );
}

/* GunMania */

WRITE32_MEMBER(ksys573_state::gunmania_w)
{
	char s[ 1024 ] = "";
	ds2401_device *ds2401 = machine().device<ds2401_device>("gunmania_id");

	switch( offset )
	{
	case 0x26:
		ds2401->write( ( data >> 5 ) & 1 );
		break;

	case 0x2a:
		switch( data & 0xa0 )
		{
		case 0x20:
			strcat( s, "cable holder motor release " );

			m_cable_holder_release = 1;
			break;

		case 0x80:
			strcat( s, "cable holder motor catch " );

			m_cable_holder_release = 0;
			break;

		case 0xa0:
			strcat( s, "cable holder motor stop " );
			break;
		}

		switch( data & 0x50 )
		{
		case 0x10:
			strcat( s, "bullet supply motor rotate " );
			break;

		case 0x40:
			strcat( s, "bullet supply motor reverse " );
			break;

		case 0x50:
			strcat( s, "bullet shutter motor unknown ");
			break;
		}

		switch( data & 0x0a )
		{
		case 0x02:
			strcat( s, "tank shutter motor close " );

			if( m_tank_shutter_position > 0 )
			{
				m_tank_shutter_position--;
			}

			break;

		case 0x08:
			strcat( s, "tank shutter motor open " );

			if( m_tank_shutter_position < 100 )
			{
				m_tank_shutter_position++;
			}

			break;

		case 0x0a:
			strcat( s, "tank shutter motor unknown ");
			break;
		}

		if( ( data & ~0xfa ) != 0 )
		{
			char unknown[ 128 ];
			sprintf( unknown, "unknown bits %08x", data & ~0xfa );
			strcat( s, unknown );
		}

		if( s[ 0 ] != 0 )
		{
//          popmessage( "%s", s );
		}

		break;
	}

	verboselog( machine(), 2, "gunmania_w %08x %08x %08x\n", offset, mem_mask, data );
}

CUSTOM_INPUT_MEMBER(ksys573_state::gunmania_tank_shutter_sensor)
{

	if( m_tank_shutter_position == 0 )
	{
		return 1;
	}

	return 0;
}

CUSTOM_INPUT_MEMBER(ksys573_state::gunmania_cable_holder_sensor)
{

	return m_cable_holder_release;
}

READ32_MEMBER(ksys573_state::gunmania_r)
{
	UINT32 data = 0;
	ds2401_device *ds2401 = machine().device<ds2401_device>("gunmania_id");

	switch( offset )
	{
	case 0x20:
		data = ioport( "GUNX" )->read() | ds2401->read() << 7;
		break;

	case 0x22:
		data = ioport( "SENSOR" )->read();
		break;

	case 0x34:
		data = ioport( "ENCODER" )->read();
		popmessage( "encoder %04x", data );
		break;
	}

	verboselog( machine(), 2, "gunmania_r %08x %08x %08x\n", offset, mem_mask, data );
	return data;
}

DRIVER_INIT_MEMBER(ksys573_state,gunmania)
{
	DRIVER_INIT_CALL(konami573);
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_readwrite_handler( 0x1f640000, 0x1f6400ff, read32_delegate(FUNC(ksys573_state::gunmania_r),this), write32_delegate(FUNC(ksys573_state::gunmania_w),this));
}

/* ADC0834 Interface */

static double analogue_inputs_callback( device_t *device, UINT8 input )
{
	switch (input)
	{
	case ADC083X_CH0:
		return (double)( 5 * device->machine().root_device().ioport( "analog0" )->read_safe( 0 ) ) / 255.0;
	case ADC083X_CH1:
		return (double)( 5 * device->machine().root_device().ioport( "analog1" )->read_safe( 0 ) ) / 255.0;
	case ADC083X_CH2:
		return (double)( 5 * device->machine().root_device().ioport( "analog2" )->read_safe( 0 ) ) / 255.0;
	case ADC083X_CH3:
		return (double)( 5 * device->machine().root_device().ioport( "analog3" )->read_safe( 0 ) ) / 255.0;
	case ADC083X_AGND:
		return 0;
	case ADC083X_VREF:
		return 5;
	}
	return 0;
}


static const adc083x_interface konami573_adc_interface = {
	analogue_inputs_callback
};

static MACHINE_CONFIG_START( konami573, ksys573_state )
	/* basic machine hardware */
	MCFG_CPU_ADD( "maincpu", CXD8530CQ, XTAL_67_7376MHz )
	MCFG_CPU_PROGRAM_MAP( konami573_map )

	MCFG_PSX_DMA_CHANNEL_READ( "maincpu", 5, psx_dma_read_delegate( FUNC( cdrom_dma_read ), (ksys573_state *) owner ) )
	MCFG_PSX_DMA_CHANNEL_WRITE( "maincpu", 5, psx_dma_write_delegate( FUNC( cdrom_dma_write ), (ksys573_state *) owner ) )

	MCFG_MACHINE_RESET_OVERRIDE(ksys573_state, konami573 )

	// multiple cd's are handled by switching drives instead of discs.
	MCFG_DEVICE_ADD("cdrom0", CR589, 0)
	MCFG_DEVICE_ADD("cdrom1", CR589, 0)

	// onboard flash
	MCFG_FUJITSU_29F016A_ADD("onboard.0")
	MCFG_FUJITSU_29F016A_ADD("onboard.1")
	MCFG_FUJITSU_29F016A_ADD("onboard.2")
	MCFG_FUJITSU_29F016A_ADD("onboard.3")
	MCFG_FUJITSU_29F016A_ADD("onboard.4")
	MCFG_FUJITSU_29F016A_ADD("onboard.5")
	MCFG_FUJITSU_29F016A_ADD("onboard.6")
	MCFG_FUJITSU_29F016A_ADD("onboard.7")

	/* video hardware */
	MCFG_PSXGPU_ADD( "maincpu", "gpu", CXD8561Q, 0x200000, XTAL_53_693175MHz )
	MCFG_PSXGPU_VBLANK_CALLBACK( vblank_state_delegate( FUNC( sys573_vblank ), (ksys573_state *) owner ) )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SPU_ADD( "spu", XTAL_67_7376MHz/2, &spu_irq )
	MCFG_SOUND_ROUTE( 0, "lspeaker", 1.0 )
	MCFG_SOUND_ROUTE( 1, "rspeaker", 1.0 )

	MCFG_SOUND_ADD( "cdda", CDDA, 0 )
	MCFG_SOUND_ROUTE( 0, "lspeaker", 1.0 )
	MCFG_SOUND_ROUTE( 1, "rspeaker", 1.0 )

	MCFG_M48T58_ADD( "m48t58" )

	MCFG_ADC0834_ADD( "adc0834", konami573_adc_interface )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pccard1, konami573 )
	// flash for pccard 1
	MCFG_FUJITSU_29F016A_ADD("pccard1.0")
	MCFG_FUJITSU_29F016A_ADD("pccard1.1")
	MCFG_FUJITSU_29F016A_ADD("pccard1.2")
	MCFG_FUJITSU_29F016A_ADD("pccard1.3")
	MCFG_FUJITSU_29F016A_ADD("pccard1.4")
	MCFG_FUJITSU_29F016A_ADD("pccard1.5")
	MCFG_FUJITSU_29F016A_ADD("pccard1.6")
	MCFG_FUJITSU_29F016A_ADD("pccard1.7")
	MCFG_FUJITSU_29F016A_ADD("pccard1.8")
	MCFG_FUJITSU_29F016A_ADD("pccard1.9")
	MCFG_FUJITSU_29F016A_ADD("pccard1.10")
	MCFG_FUJITSU_29F016A_ADD("pccard1.11")
	MCFG_FUJITSU_29F016A_ADD("pccard1.12")
	MCFG_FUJITSU_29F016A_ADD("pccard1.13")
	MCFG_FUJITSU_29F016A_ADD("pccard1.14")
	MCFG_FUJITSU_29F016A_ADD("pccard1.15")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pccard2, konami573 )
	// flash for pccard 2
	MCFG_FUJITSU_29F016A_ADD("pccard2.0")
	MCFG_FUJITSU_29F016A_ADD("pccard2.1")
	MCFG_FUJITSU_29F016A_ADD("pccard2.2")
	MCFG_FUJITSU_29F016A_ADD("pccard2.3")
	MCFG_FUJITSU_29F016A_ADD("pccard2.4")
	MCFG_FUJITSU_29F016A_ADD("pccard2.5")
	MCFG_FUJITSU_29F016A_ADD("pccard2.6")
	MCFG_FUJITSU_29F016A_ADD("pccard2.7")
	MCFG_FUJITSU_29F016A_ADD("pccard2.8")
	MCFG_FUJITSU_29F016A_ADD("pccard2.9")
	MCFG_FUJITSU_29F016A_ADD("pccard2.10")
	MCFG_FUJITSU_29F016A_ADD("pccard2.11")
	MCFG_FUJITSU_29F016A_ADD("pccard2.12")
	MCFG_FUJITSU_29F016A_ADD("pccard2.13")
	MCFG_FUJITSU_29F016A_ADD("pccard2.14")
	MCFG_FUJITSU_29F016A_ADD("pccard2.15")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( k573bait, konami573 )

	/* Additional NEC Encoder */
	MCFG_UPD4701_ADD( "upd4701" )
MACHINE_CONFIG_END

// Variants with additional digital sound board
static MACHINE_CONFIG_DERIVED( k573d, konami573 )
	MCFG_MAS3507D_ADD( "mpeg" )
	MCFG_DS2401_ADD( "digital_id" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pccard1d, pccard1 )
	MCFG_MAS3507D_ADD( "mpeg" )
	MCFG_DS2401_ADD( "digital_id" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pccard2d, pccard2 )
	MCFG_MAS3507D_ADD( "mpeg" )
	MCFG_DS2401_ADD( "digital_id" )
MACHINE_CONFIG_END

// Security eeprom variants
//
// Suffixes are used to select them
//  x = x76f041
//  y = x76f100
//  z = zs01
//
//  i = also use one or two ds2401
//
// Up to two carts can be used

static MACHINE_CONFIG_DERIVED( konami573x, konami573 )
	MCFG_X76F041_ADD( "install_eeprom" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( konami573y, konami573 )
	MCFG_X76F100_ADD( "install_eeprom" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( konami573yi, konami573 )
	MCFG_X76F100_ADD( "install_eeprom" )
	MCFG_DS2401_ADD(  "install_id" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( konami573zi, konami573 )
	MCFG_ZS01_ADD(    "install_eeprom", "install_id" )
	MCFG_DS2401_ADD(  "install_id" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( k573baitx, k573bait )
	MCFG_X76F041_ADD( "install_eeprom" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( k573dx, k573d )
	MCFG_X76F041_ADD( "install_eeprom" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( k573dxi, k573d )
	MCFG_X76F041_ADD( "install_eeprom" )
	MCFG_DS2401_ADD(  "install_id" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( k573dxzi, k573d )
	MCFG_X76F041_ADD( "install_eeprom" )
	MCFG_DS2401_ADD(  "install_id" )
	MCFG_ZS01_ADD(    "game_eeprom", "game_id" )
	MCFG_DS2401_ADD(  "game_id" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( k573dyi, k573d )
	MCFG_X76F100_ADD( "install_eeprom" )
	MCFG_DS2401_ADD(  "install_id" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( k573dyyi, k573d )
	MCFG_X76F100_ADD( "install_eeprom" )
	MCFG_DS2401_ADD(  "install_id" )
	MCFG_X76F100_ADD( "game_eeprom" )
	MCFG_DS2401_ADD(  "game_id" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( k573dzi, k573d )
	MCFG_ZS01_ADD(    "install_eeprom", "install_id" )
	MCFG_DS2401_ADD(  "install_id" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pccard1x, pccard1 )
	MCFG_X76F041_ADD( "install_eeprom" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pccard1xi, pccard1 )
	MCFG_X76F041_ADD( "install_eeprom" )
	MCFG_DS2401_ADD(  "install_id" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pccard1yi, pccard1 )
	MCFG_X76F100_ADD( "install_eeprom" )
	MCFG_DS2401_ADD(  "install_id" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pccard1dxzi, pccard1d )
	MCFG_X76F041_ADD( "install_eeprom" )
	MCFG_DS2401_ADD(  "install_id" )
	MCFG_ZS01_ADD(    "game_eeprom", "game_id" )
	MCFG_DS2401_ADD(  "game_id" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pccard1dzi, pccard1d )
	MCFG_ZS01_ADD(    "install_eeprom", "install_id" )
	MCFG_DS2401_ADD(  "install_id" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pccard2yyi, pccard2 )
	MCFG_X76F100_ADD( "install_eeprom" )
	MCFG_DS2401_ADD(  "install_id" )
	MCFG_X76F100_ADD( "game_eeprom" )
	MCFG_DS2401_ADD(  "game_id" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pccard2dxzi, pccard2d )
	MCFG_X76F041_ADD( "install_eeprom" )
	MCFG_DS2401_ADD(  "install_id" )
	MCFG_ZS01_ADD(    "game_eeprom", "game_id" )
	MCFG_DS2401_ADD(  "game_id" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pccard2dyyi, pccard2d )
	MCFG_X76F100_ADD( "install_eeprom" )
	MCFG_DS2401_ADD(  "install_id" )
	MCFG_X76F100_ADD( "game_eeprom" )
	MCFG_DS2401_ADD(  "game_id" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pccard2dzi, pccard2d )
	MCFG_ZS01_ADD(    "install_eeprom", "install_id" )
	MCFG_DS2401_ADD(  "install_id" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( punchmania, pccard1xi )
	MCFG_ADC0838_ADD( "adc0838", punchmania_adc_interface )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( punchmania2, punchmania )
	// flash for pccard 3
	MCFG_FUJITSU_29F016A_ADD("pccard3.0")
	MCFG_FUJITSU_29F016A_ADD("pccard3.1")
	MCFG_FUJITSU_29F016A_ADD("pccard3.2")
	MCFG_FUJITSU_29F016A_ADD("pccard3.3")
	MCFG_FUJITSU_29F016A_ADD("pccard3.4")
	MCFG_FUJITSU_29F016A_ADD("pccard3.5")
	MCFG_FUJITSU_29F016A_ADD("pccard3.6")
	MCFG_FUJITSU_29F016A_ADD("pccard3.7")
	MCFG_FUJITSU_29F016A_ADD("pccard3.8")
	MCFG_FUJITSU_29F016A_ADD("pccard3.9")
	MCFG_FUJITSU_29F016A_ADD("pccard3.10")
	MCFG_FUJITSU_29F016A_ADD("pccard3.11")
	MCFG_FUJITSU_29F016A_ADD("pccard3.12")
	MCFG_FUJITSU_29F016A_ADD("pccard3.13")
	MCFG_FUJITSU_29F016A_ADD("pccard3.14")
	MCFG_FUJITSU_29F016A_ADD("pccard3.15")

		// flash for pccard 4
	MCFG_FUJITSU_29F016A_ADD("pccard4.0")
	MCFG_FUJITSU_29F016A_ADD("pccard4.1")
	MCFG_FUJITSU_29F016A_ADD("pccard4.2")
	MCFG_FUJITSU_29F016A_ADD("pccard4.3")
	MCFG_FUJITSU_29F016A_ADD("pccard4.4")
	MCFG_FUJITSU_29F016A_ADD("pccard4.5")
	MCFG_FUJITSU_29F016A_ADD("pccard4.6")
	MCFG_FUJITSU_29F016A_ADD("pccard4.7")
	MCFG_FUJITSU_29F016A_ADD("pccard4.8")
	MCFG_FUJITSU_29F016A_ADD("pccard4.9")
	MCFG_FUJITSU_29F016A_ADD("pccard4.10")
	MCFG_FUJITSU_29F016A_ADD("pccard4.11")
	MCFG_FUJITSU_29F016A_ADD("pccard4.12")
	MCFG_FUJITSU_29F016A_ADD("pccard4.13")
	MCFG_FUJITSU_29F016A_ADD("pccard4.14")
	MCFG_FUJITSU_29F016A_ADD("pccard4.15")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( gunmania, pccard2 )
	MCFG_DS2401_ADD( "gunmania_id" )
MACHINE_CONFIG_END

static INPUT_PORTS_START( konami573 )
	PORT_START("IN0")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("OUT0")
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("adc0834", adc083x_cs_write)
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("adc0834", adc083x_clk_write)
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("adc0834", adc083x_di_write)

	PORT_START("IN1")
	PORT_DIPNAME( 0x00000001, 0x00000001, "Unused 1" ) PORT_DIPLOCATION( "DIP SW:1" )
	PORT_DIPSETTING(          0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, "Screen Flip" ) PORT_DIPLOCATION( "DIP SW:2" )
	PORT_DIPSETTING(          0x00000002, DEF_STR( Normal ) )
	PORT_DIPSETTING(          0x00000000, "V-Flip" )
	PORT_DIPNAME( 0x00000004, 0x00000004, "Unused 2") PORT_DIPLOCATION( "DIP SW:3" )
	PORT_DIPSETTING(          0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000000, "Start Up Device" ) PORT_DIPLOCATION( "DIP SW:4" )
	PORT_DIPSETTING(          0x00000008, "CD-ROM Drive" )
	PORT_DIPSETTING(          0x00000000, "Flash ROM" )
	PORT_BIT( 0x000000f0, IP_ACTIVE_HIGH, IPT_SPECIAL ) /* 0xc0 */
	PORT_BIT( 0x00000100, IP_ACTIVE_HIGH, IPT_SPECIAL )
	PORT_BIT( 0x00000200, IP_ACTIVE_HIGH, IPT_SPECIAL )
//  PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNKNOWN )
//  PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_CONFNAME( 0x00001000, 0x00001000, "Network?" )
	PORT_CONFSETTING(          0x00001000, DEF_STR( Off ) )
	PORT_CONFSETTING(          0x00000000, DEF_STR( On ) )
//  PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNKNOWN )
//  PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
//  PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("adc0834", adc083x_do_read)
//  PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00040000, IP_ACTIVE_HIGH, IPT_SPECIAL ) /* x76f041/zs01 sda */
    PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00100000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* skip hang at startup */
	PORT_BIT( 0x00200000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* skip hang at startup */
//  PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
//  PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04000000, IP_ACTIVE_HIGH, IPT_SPECIAL ) /* PCCARD 1 */
	PORT_BIT( 0x08000000, IP_ACTIVE_HIGH, IPT_SPECIAL ) /* PCCARD 2 */
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_SERVICE1 )
//  PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
//  PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
//  PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) /* skip init? */
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_START1 ) /* skip init? */
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) /* skip init? */
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_START2 ) /* skip init? */

	PORT_START("IN3")
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_SERVICE_NO_TOGGLE( 0x00000400, IP_ACTIVE_LOW )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
//  PORT_BIT( 0xf0fff0ff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("CART")
	PORT_CONFNAME( 1, 0, "Security Cart" )
	PORT_CONFSETTING( 0, "Install" )
	PORT_CONFSETTING( 1, "Game" )

	PORT_START("CD")
	PORT_CONFNAME( 1, 0, "CD" )
	PORT_CONFSETTING( 0, "1" )
	PORT_CONFSETTING( 1, "2" )
INPUT_PORTS_END

static INPUT_PORTS_START( fbaitbc )
	PORT_INCLUDE( konami573 )

	PORT_START( "uPD4701_y" )
	PORT_BIT( 0x0fff, 0, IPT_MOUSE_Y ) PORT_MINMAX( 0, 0xfff ) PORT_SENSITIVITY( 15 ) PORT_KEYDELTA( 8 ) PORT_RESET

	PORT_START( "uPD4701_switches" )
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(1)
INPUT_PORTS_END

static INPUT_PORTS_START( fbaitmc )
	PORT_INCLUDE( fbaitbc )

	PORT_START( "analog0" )
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x20,0xdf) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(1) PORT_REVERSE

	PORT_START( "analog1" )
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x20,0xdf) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(1)
INPUT_PORTS_END

static INPUT_PORTS_START( ddr )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x00000f0f, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, ksys573_state,gn845pwbb_read, NULL)

	PORT_START( "STAGE" )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_PLAYER(1) /* multiplexor */
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_16WAY PORT_PLAYER(1)    /* multiplexor */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_PLAYER(2) /* multiplexor */
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_16WAY PORT_PLAYER(2)    /* multiplexor */
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( ddrsolo )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Left 1" )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Right 1" )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Up 1" )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Down 1" )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME( "P1 Up-Left 2" ) /* P1 BUTTON 1 */
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Left 2" ) /* P1 BUTTON 2 */
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Down 2" ) /* P1 BUTTON 3 */
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Up-Left 1" ) /* P2 LEFT */
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Up-Right 1" ) /* P2 RIGHT */
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 UP */
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 DOWN */
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_NAME( "P1 Up 2" ) /* P2 BUTTON1 */
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Right 2" ) /* P2 BUTTON2 */
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME( "P1 Up-Right 2" ) /* P2 BUTTON3 */
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_SPECIAL ) /* P2 START */

	PORT_MODIFY("IN3")
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON4 */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME( "P1 Select L" ) /* P1 BUTTON5 */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON6 */
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON4 */
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME( "P1 Select R" ) /* P2 BUTTON5 */
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON6 */
INPUT_PORTS_END

static INPUT_PORTS_START( gtrfrks )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* SERVICE1 */

	PORT_MODIFY("IN2")
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 Effect 1")
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("P1 Effect 2")
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("P1 Pick")
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME("P1 Wailing")
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Button R")
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Button G")
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Button B")
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Effect 1")
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("P2 Effect 2")
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("P2 Pick")
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(2) PORT_NAME("P2 Wailing")
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Button R")
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Button G")
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Button B")
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_START2 )

	PORT_MODIFY("IN3")
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_SERVICE1 ) /* P1 BUTTON4 */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON5 */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON6 */
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_SERVICE2 ) /* P1 BUTTON4 */
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON5 */
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON6 */
INPUT_PORTS_END

static INPUT_PORTS_START( dmx )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME( "D-Sensor D1 L" ) /* P1 LEFT */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME( "D-Sensor D1 R" ) /* P1 RIGHT */
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Select L" ) /* P1 UP */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Select R" ) /* P1 DOWN */
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME( "D-Sensor U L" ) /* P1 BUTTON1 */
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME( "D-Sensor U R" ) /* P1 BUTTON2 */
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON3 */
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME( "D-Sensor D1 L" ) /* P2 LEFT */
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME( "D-Sensor D1 R" ) /* P2 RIGHT */
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_PLAYER(2) PORT_NAME( "P2 Select L" ) /* P2 UP */
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_PLAYER(2) PORT_NAME( "P2 Select R" ) /* P2 DOWN */
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME( "D-Sensor U L" ) /* P2 BUTTON1 */
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME( "D-Sensor U R" ) /* P2 BUTTON2 */
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON3 */

	PORT_MODIFY("IN3")
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME( "D-Sensor D0 L" ) /* P1 BUTTON4 */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME( "D-Sensor D0 R" ) /* P1 BUTTON5 */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON6 */
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME( "D-Sensor D0 L" ) /* P2 BUTTON4 */
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME( "D-Sensor D0 R" ) /* P2 BUTTON5 */
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON6 */
INPUT_PORTS_END

static INPUT_PORTS_START( drmn )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* COIN2 */

	PORT_MODIFY("IN2")
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME( "High Tom" ) /* P1 LEFT */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME( "Low Tom" ) /* P1 RIGHT */
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME( "Hi-Hat" ) /* P1 UP */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME( "Snare" ) /* P1 DOWN */
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME( "Cymbal" ) /* P1 BUTTON 1 */
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON 2 */
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME( "Bass Drum" ) /* P1 BUTTON 3 */
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "Select L" ) /* P2 LEFT */
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "Select R" ) /* P2 RIGHT */
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 UP */
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 DOWN */
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON1 */
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON2 */
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON3 */
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 START */

	PORT_MODIFY("IN3")
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON4 */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON5 */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON6 */
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON4 */
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON5 */
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON6 */
INPUT_PORTS_END

static INPUT_PORTS_START( gunmania )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x000000ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME( "Bullet Tube-1 Sensor" )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME( "Bullet Tube-2 Sensor" )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME( "Safety Sensor Under" )
	PORT_BIT( 0x00000100, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF,ksys573_state,gunmania_tank_shutter_sensor, NULL )

	PORT_MODIFY("IN3")
	PORT_BIT( 0x0d000b00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02000000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF,ksys573_state,gunmania_cable_holder_sensor, NULL )

	PORT_START("GUNX")
	PORT_BIT( 0x7f, 0x2f, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0x00,0x5f) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("GUNY")
	PORT_BIT( 0x7f, 0x1f, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0x00,0x3f) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START( "SENSOR" )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_PLAYER(1) PORT_NAME( "Safety Sensor Front" )

	PORT_START( "ENCODER" )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(4) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(4) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(4) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(4) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(4) PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x00000020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(4) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(4) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_PLAYER(4) PORT_CODE(KEYCODE_I)
INPUT_PORTS_END

static INPUT_PORTS_START( hndlchmp )
	PORT_INCLUDE( konami573 )

	PORT_START( "analog0" )
	PORT_BIT( 0xff, 0xc0, IPT_PEDAL ) PORT_MINMAX( 0xc0, 0xf0 ) PORT_SENSITIVITY( 100 ) PORT_KEYDELTA( 20 ) PORT_PLAYER( 2 )

	PORT_START( "analog1" )
	PORT_BIT( 0xff, 0xc0, IPT_PEDAL ) PORT_MINMAX( 0xc0, 0xf0 ) PORT_SENSITIVITY( 100 ) PORT_KEYDELTA( 20 ) PORT_PLAYER( 1 )

	PORT_START( "analog2" )
	PORT_BIT( 0xff, 0x7f, IPT_PADDLE ) PORT_MINMAX( 0x48, 0xb7 ) PORT_SENSITIVITY( 25 ) PORT_KEYDELTA( 30 ) PORT_PLAYER( 2 )

	PORT_START( "analog3" )
	PORT_BIT( 0xff, 0x7f, IPT_PADDLE ) PORT_MINMAX( 0x48, 0xb7 ) PORT_SENSITIVITY( 25 ) PORT_KEYDELTA( 30 ) PORT_PLAYER( 1 )
INPUT_PORTS_END

static INPUT_PORTS_START( hyperbbc )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY("IN2")

	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3) /* P1 LEFT */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3) /* P1 RIGHT */
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_START3 ) /* P1 UP */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3) /* P1 DOWN */
INPUT_PORTS_END

static INPUT_PORTS_START( hypbbc2p )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_START2 ) /* P1 UP */
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 START */
INPUT_PORTS_END

static INPUT_PORTS_START( mamboagg )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME( "Right Pad 1 (Top Right)" ) /* COIN2 */

	PORT_MODIFY("IN2")
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(1) PORT_NAME( "Centre Pad 3 (Middle Right)" ) /* P1 UP */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME( "Centre Pad 1 (Top Right)" ) /* P1 DOWN */
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME( "Left Pad 2 (Bottom Left)" ) /* P1 BUTTON 1 */
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME( "Left Pad 1 (Top Left)" ) /* P1 BUTTON 2 */
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME( "Left Pad 3 (Bottom Right)" ) /* P1 BUTTON 3 */
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME( "Centre Pad 2 (Bottom Left)" ) /* P2 LEFT */
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(1) PORT_NAME( "Centre Pad 3 (Bottom Right)" ) /* P2 RIGHT */
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME( "Centre Pad 1 (Top Left)" ) /* P2 UP */
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME( "Centre Pad 2 (Middle Left)" ) /* P2 DOWN */
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME( "Right Pad 2 (Bottom Left)" ) /* P2 BUTTON1 */
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME( "Right Pad 1 (Top Left)" ) /* P2 BUTTON2 */
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_PLAYER(1) PORT_NAME( "Right Pad 3 (Bottom Right)" ) /* P2 BUTTON3 */
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME( "Left Pad 1 (Top Right)" ) /* P2 START */

	PORT_MODIFY("IN3")
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME( "Left Pad 3 (Middle Right)" ) /* P1 BUTTON4 */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME( "Left Pad 2 (Middle Left)" ) /* P1 BUTTON5 */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON6 */
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_PLAYER(1) PORT_NAME( "Right Pad 3 (Middle Right)" ) /* P2 BUTTON4 */
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME( "Right Pad 2 (Middle Left)" ) /* P2 BUTTON5 */
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON6 */
INPUT_PORTS_END

static INPUT_PORTS_START( punchmania )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x00000100, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("adc0838", adc083x_do_read)
	PORT_BIT( 0x00000200, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("adc0838", adc083x_sars_read)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "OUT1" )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("adc0838", adc083x_cs_write)
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("adc0838", adc083x_clk_write)
	PORT_BIT( 0x00000020, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("adc0838", adc083x_di_write)

	PORT_MODIFY("IN2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "Select L" ) /* P2 LEFT */
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "Select R" ) /* P2 RIGHT */
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME( "Skip Check" )
	PORT_BIT( 0x00005ffc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN3")
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON4 */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON5 */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON6 */
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON4 */
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON5 */
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON6 */

	PORT_START( "PADS" )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME( "Top Left" )
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME( "Middle Left" )
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME( "Bottom Left" )
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME( "Top Right" )
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME( "Middle Right" )
	PORT_BIT( 0x00000020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME( "Bottom Right" )
INPUT_PORTS_END

#define SYS573_BIOS_A \
	ROM_REGION32_LE( 0x080000, "bios", 0 ) \
	ROM_SYSTEM_BIOS( 0, "std",        "Standard" ) \
	ROMX_LOAD( "700a01.22g",   0x0000000, 0x080000, CRC(11812ef8) SHA1(e1284add4aaddd5337bd7f4e27614460d52b5b48), ROM_BIOS(1) ) \
	ROM_SYSTEM_BIOS( 1, "gchgchmp",   "Found on Gachagachamp" ) \
	ROMX_LOAD( "700_a01.22g",  0x000000,  0x080000, CRC(39ebb0ca) SHA1(9aab8c637dd2be84d79007e52f108abe92bf29dd), ROM_BIOS(2) ) \
	ROM_SYSTEM_BIOS( 2, "dsem2",      "Found on Dancing Stage Euro Mix 2" ) \
	ROMX_LOAD( "700b01.22g",   0x0000000, 0x080000, CRC(6cf852af) NO_DUMP, ROM_BIOS(3) )

#define SYS573_DIGITAL_ID \
	ROM_REGION( 0x000008, "digital_id", 0 ) /* digital board id */		\
	ROM_LOAD( "digital-id.bin",   0x000000, 0x000008, CRC(2b977f4d) SHA1(2b108a56653f91cb3351718c45dfcf979bc35ef1) )

// BIOS
ROM_START( sys573 )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID
ROM_END

// Games
ROM_START( bassangl )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "ge765ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(ee1b32a7) SHA1(c0f6b14b054f5a95ce474e794a3e0ca78faac681) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "765jaa02", 0, BAD_DUMP SHA1(4291711b1025733cb97f6da5dc3b03c189fcc37c) )
ROM_END

ROM_START( bassang2 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gc865ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(095cbfb5) SHA1(529ce0a7b0986cf7e64c37f466d6c2dac95cea7f) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "865jaa02", 0, BAD_DUMP SHA1(b98d9aa54f13aa73bea580d6494cb6a7f3217be3) )
ROM_END

ROM_START( cr589fw )
	SYS573_BIOS_A

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "700b04", 0, BAD_DUMP SHA1(2f65f62eb7ae202153a8544989675989ed33316f) )
ROM_END

ROM_START( cr589fwa )
	SYS573_BIOS_A

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "700a04", 0, BAD_DUMP SHA1(554481f48eeb5daf8b4e7be2d66840d6c8454a52) )
ROM_END

ROM_START( darkhleg )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gx706ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(72b42574) SHA1(79dc959f0ce95ccb9ac0dbf0a72aec973e91bc56) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "706jaa02", 0, BAD_DUMP SHA1(58bd06855988250028086cba6b3670372b9d96a0) )
ROM_END

ROM_START( ddrextrm )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gcc36ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(c1601287) SHA1(929691a78f7bb6dd830f832f301116df0da1619b) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gcc36ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "c36jaa02", 0, BAD_DUMP SHA1(edeb45fff0e66151b1ba2fd67542064ccddb031e) )
ROM_END

ROM_START( ddru )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gn845ua.u1",   0x000000, 0x000224, BAD_DUMP CRC(c9e7fced) SHA1(aac4dde100091bc64d397f53484a0ffbf68b8101) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "845uaa02", 0, SHA1(d3f9290d4dadb5e9b82ebe77abf7b99d1a89f716) )
ROM_END

ROM_START( ddrj )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gc845jb.u1",   0x000000, 0x000224, BAD_DUMP CRC(a16f42b8) SHA1(da4f1eb3eb2b28cb3a0bc74bb9b9945970f56ac2) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "845jba02", 0, BAD_DUMP SHA1(2d10378c89fe85682f262f0987f8366b9ea72f11) )
ROM_END

ROM_START( ddrja )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gc845ja.u1",   0x000000, 0x000224, NO_DUMP )

	ROM_REGION( 0x200000, "onboard.0", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jaa.31m",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "onboard.1", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jaa.27m",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "onboard.2", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jaa.31l",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "onboard.3", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jaa.27l",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "onboard.4", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jaa.31j",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "onboard.5", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jaa.27j",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "onboard.6", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jaa.31h",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "onboard.7", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jaa.27h",  0x000000, 0x200000, NO_DUMP )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "845jaa02", 0, BAD_DUMP SHA1(37ca16be25bee39a5692dee2fa5f0fa0addfaaca) )

	DISK_REGION( "cdrom1" )
	DISK_IMAGE_READONLY( "845jaa01", 1, NO_DUMP ) // if this even exists
ROM_END

ROM_START( ddrjb )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gc845ja.u1",   0x000000, 0x000224, NO_DUMP )

	ROM_REGION( 0x200000, "onboard.0", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.31m",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "onboard.1", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.27m",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "onboard.2", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.31l",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "onboard.3", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.27l",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "onboard.4", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.31j",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "onboard.5", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.27j",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "onboard.6", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.31h",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "onboard.7", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.27h",  0x000000, 0x200000, NO_DUMP )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "845jab02", 0, BAD_DUMP SHA1(7bdcef37bf376c23153dfd1580de5666cc681335) )

	DISK_REGION( "cdrom1" )
	DISK_IMAGE_READONLY( "845jab01", 1, NO_DUMP ) // if this even exists
ROM_END

ROM_START( ddra )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gn845aa.u1",   0x000000, 0x000224, BAD_DUMP CRC(327c4851) SHA1(f0939224af706fd103a67aae9c96518c1db90ac9) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "845aaa02", 0, BAD_DUMP SHA1(839e2f8698a1561ac364998b8b3158ef0dee6998) )
ROM_END

ROM_START( ddr2m )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gn895jaa.u1",  0x000000, 0x000224, BAD_DUMP CRC(363f427e) SHA1(adec886a07b9bd91f142f286b04fc6582205f037) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "895jaa02", 0, BAD_DUMP SHA1(cfe3a6f3ed62ba388b07045e29e22472d17dcfe4) )
ROM_END

ROM_START( ddr2mc )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gn896ja.u1",  0x000000, 0x000224, BAD_DUMP CRC(cbc984c5) SHA1(6c0cd78a41000999b4ffbd9fb3707738b50a9b50) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "896jaa01", 0, BAD_DUMP SHA1(f802a0e2ba0147eb71c54d92af409c3010a5715f) )

	DISK_REGION( "cdrom1" )
	DISK_IMAGE_READONLY( "895jaa02", 1, BAD_DUMP SHA1(cfe3a6f3ed62ba388b07045e29e22472d17dcfe4) )
ROM_END

ROM_START( ddr2mc2 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "ge984ja.u1",  0x000000, 0x000224, BAD_DUMP CRC(cbc984c5) SHA1(6c0cd78a41000999b4ffbd9fb3707738b50a9b50) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "984jaa01", 0, BAD_DUMP SHA1(5505c28be27bfa9648060fd799bcf0c2c5f608ed) )

	DISK_REGION( "cdrom1" )
	DISK_IMAGE_READONLY( "895jaa02", 1, BAD_DUMP SHA1(cfe3a6f3ed62ba388b07045e29e22472d17dcfe4) )
ROM_END

ROM_START( ddr2ml )
	SYS573_BIOS_A

	ROM_REGION( 0x080000, "cpu2", 0 ) /* memory card reader */
	ROM_LOAD( "885a01.bin",   0x000000, 0x080000, CRC(e22d093f) SHA1(927f62f63b5caa7899392decacd12fea0e6fdbea) )

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "ge885jaa.u1",  0x000000, 0x000224, BAD_DUMP CRC(cbc984c5) SHA1(6c0cd78a41000999b4ffbd9fb3707738b50a9b50) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "885jab01", 0, SHA1(c2bbb9e2e6f34e07f57e7076726af81df39f55c9) )

	DISK_REGION( "cdrom1" )
	DISK_IMAGE_READONLY( "885jaa02", 0, BAD_DUMP SHA1(5d187aea247eefc5c065566ab277acd8c942ba27) )
ROM_END

ROM_START( ddr2mla )
	SYS573_BIOS_A

	ROM_REGION( 0x080000, "cpu2", 0 ) /* memory card reader */
	ROM_LOAD( "885a01.bin",   0x000000, 0x080000, CRC(e22d093f) SHA1(927f62f63b5caa7899392decacd12fea0e6fdbea) )

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "ge885jaa.u1",  0x000000, 0x000224, BAD_DUMP CRC(cbc984c5) SHA1(6c0cd78a41000999b4ffbd9fb3707738b50a9b50) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "885jaa02", 0, BAD_DUMP SHA1(5d187aea247eefc5c065566ab277acd8c942ba27) )
ROM_END

ROM_START( ddr3ma )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0000084, "install_eeprom", 0 ) /* install security cart eeprom */
	ROM_LOAD( "ge887aa.u1",   0x000000, 0x000084, BAD_DUMP CRC(4ce86d32) SHA1(94cdb9873a7f7503acc3b763e9b49ec6af53533f) )

	ROM_REGION( 0x0000084, "game_eeprom", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gn887aa.u1",   0x000000, 0x000084, BAD_DUMP CRC(bb14f9bd) SHA1(9d0adf5a32d8bbcaaea2f701f5c7a5d51ee0b8bf) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* install security cart id */
	ROM_LOAD( "ge887aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "game_id", 0 ) /* game security cart id */
	ROM_LOAD( "gn887aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "887aaa02", 0, BAD_DUMP SHA1(c4136305b97123f5dfe3ecd34a10ddda0180da3d) )
ROM_END

ROM_START( ddr3mj )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0000084, "install_eeprom", 0 ) /* install security cart eeprom */
	ROM_LOAD( "ge887ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(3a377cec) SHA1(5bf3107a89547bd7697d9f0ab8f67240e101a559) )

	ROM_REGION( 0x0000084, "game_eeprom", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gn887ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(2f633432) SHA1(bce44f20a5a7318af6aea4fdfa8af64ddb76047c) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* install security cart id */
	ROM_LOAD( "ge887ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "game_id", 0 ) /* game security cart id */
	ROM_LOAD( "gn887ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "887jaa02", 0, BAD_DUMP SHA1(2d1bf2a1566292dc869afaa6486f5ecd3973ff62) )
ROM_END

ROM_START( ddr3mk )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0000084, "install_eeprom", 0 ) /* install security cart eeprom */
	ROM_LOAD( "ge887kb.u1",   0x000000, 0x000084, BAD_DUMP CRC(4ce86d32) SHA1(94cdb9873a7f7503acc3b763e9b49ec6af53533f) )

	ROM_REGION( 0x0000084, "game_eeprom", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gn887kb.u1",   0x000000, 0x000084, BAD_DUMP CRC(bb14f9bd) SHA1(9d0adf5a32d8bbcaaea2f701f5c7a5d51ee0b8bf) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* install security cart id */
	ROM_LOAD( "ge887kb.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "game_id", 0 ) /* game security cart id */
	ROM_LOAD( "gn887kb.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "887kba02", 0, BAD_DUMP SHA1(92a3844fab24f46c16dd96f9474d95fd001df603) )
ROM_END

ROM_START( ddr3mka )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0000084, "install_eeprom", 0 ) /* install security cart eeprom */
	ROM_LOAD( "ge887ka.u1",   0x000000, 0x000084, BAD_DUMP CRC(4ce86d32) SHA1(94cdb9873a7f7503acc3b763e9b49ec6af53533f) )

	ROM_REGION( 0x0000084, "game_eeprom", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gn887ka.u1",   0x000000, 0x000084, BAD_DUMP CRC(bb14f9bd) SHA1(9d0adf5a32d8bbcaaea2f701f5c7a5d51ee0b8bf) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* install security cart id */
	ROM_LOAD( "ge887ka.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "game_id", 0 ) /* game security cart id */
	ROM_LOAD( "gn887ka.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "887kaa02", 0, BAD_DUMP SHA1(a80930dd66c2e2326e8792f2e7cf9116d9cd752c) )
ROM_END

ROM_START( ddr3mp )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* install security cart eeprom */
	ROM_LOAD( "gea22ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(ef370ff7) SHA1(cb7a043f8bfa535e54ae9af728031d1018ed0734) )

	ROM_REGION( 0x0001014, "game_eeprom", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gca22ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(6883c82c) SHA1(6fef1dc7150066eee427db685b6c5fb350b7768d) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* install security cart id */
	ROM_LOAD( "gea22ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "game_id", 0 ) /* game security cart id */
	ROM_LOAD( "gca22ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a22jaa02", 0, BAD_DUMP SHA1(2bf07d08f6acee562024b418b453d654fc40f8dd) )
ROM_END

ROM_START( ddr4m )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* install security cart eeprom */
	ROM_LOAD( "gea33aa.u1",   0x000000, 0x000224, BAD_DUMP CRC(7bd2a24f) SHA1(62c73a54c4ed7697cf81ddbf3d13d4b0ca827be5) )

	ROM_REGION( 0x0001014, "game_eeprom", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gca33aa.u1",   0x000000, 0x001014, BAD_DUMP CRC(f6feb2bd) SHA1(dfd5bd532338849289e2e4c155c0ca86e79b9ae5) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* install security cart id */
	ROM_LOAD( "gea33aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "game_id", 0 ) /* game security cart id */
	ROM_LOAD( "gca33aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a33aaa02", 0, BAD_DUMP SHA1(cc7349cbee37bfb101480497e99f1f52acb4ffa1) )
ROM_END

ROM_START( ddr4mj )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* install security cart eeprom */
	ROM_LOAD( "a33jaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(10f1e9b8) SHA1(985bd26638964beebba5de4c7cda772b402d2e59) )

	ROM_REGION( 0x0001014, "game_eeprom", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gca33ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(e5230867) SHA1(44aea9ccc90d81e7f41e5e9a62b28fcbdd75363b) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* install security cart id */
	ROM_LOAD( "a33jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "game_id", 0 ) /* game security cart id */
	ROM_LOAD( "gca33ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a33jaa02", 0, BAD_DUMP SHA1(9d9fb5e65f1532f358e9c273c56d11389d11fd79) )
ROM_END

ROM_START( ddr4ms )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* install security cart eeprom */
	ROM_LOAD( "gea33ab.u1",   0x000000, 0x000224, BAD_DUMP CRC(32fb3d13) SHA1(3ca6c77438f96b13d2c05f13a10fcff89a1403a2) )

	ROM_REGION( 0x0001014, "game_eeprom", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gca33ab.u1",   0x000000, 0x001014, BAD_DUMP CRC(312ac13f) SHA1(05d733edc03cfc5ea03db6c683f59ed6ff860b5a) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* install security cart id */
	ROM_LOAD( "gea33ab.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "game_id", 0 ) /* game security cart id */
	ROM_LOAD( "gca33ab.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a33aba02", 0, BAD_DUMP SHA1(cc7349cbee37bfb101480497e99f1f52acb4ffa1) )
ROM_END

ROM_START( ddr4msj )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* install security cart eeprom */
	ROM_LOAD( "a33jba.u1",    0x000000, 0x000224, BAD_DUMP CRC(babf6fdb) SHA1(a2ef6b855d42072f0d3c72c8de9aff1f867de3f7) )

	ROM_REGION( 0x0001014, "game_eeprom", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gca33jb.u1",   0x000000, 0x001014, BAD_DUMP CRC(00e4b531) SHA1(f421fc33642c5a3cd89fb14dc8cd601bdddd1f55) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* install security cart id */
	ROM_LOAD( "a33jba.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "game_id", 0 ) /* game security cart id */
	ROM_LOAD( "gca33jb.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a33jba02", 0, BAD_DUMP SHA1(9d9fb5e65f1532f358e9c273c56d11389d11fd79) )
ROM_END

ROM_START( ddr4mp )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* install security cart eeprom */
	ROM_LOAD( "gea34ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(10f1e9b8) SHA1(985bd26638964beebba5de4c7cda772b402d2e59) )

	ROM_REGION( 0x0001014, "game_eeprom", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gca34ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(e9b6ce56) SHA1(f040fba2b2b446baa840026dcd10f9785f8cc0a3) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* install security cart id */
	ROM_LOAD( "gea34ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "game_id", 0 ) /* game security cart id */
	ROM_LOAD( "gca34ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	ROM_REGION( 0x002000, "m48t58", 0 ) /* timekeeper */
	ROM_LOAD( "gca34ja.22h",  0x000000, 0x002000, CRC(80575c1f) SHA1(a0594ca0f75bc7d49b645e835e9fa48a73c3c9c7) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a34jaa02", 0, BAD_DUMP SHA1(1d5f9eb633f054ddbf9fba55d53e4ee263ba91dd) )
ROM_END

ROM_START( ddr4mps )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* install security cart eeprom */
	ROM_LOAD( "gea34jb.u1",   0x000000, 0x000224, BAD_DUMP CRC(babf6fdb) SHA1(a2ef6b855d42072f0d3c72c8de9aff1f867de3f7) )

	ROM_REGION( 0x0001014, "game_eeprom", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gca34jb.u1",   0x000000, 0x001014, BAD_DUMP CRC(0c717300) SHA1(00d21f39fe90494ffec2f8799767cc46a9cd2b00) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* install security cart id */
	ROM_LOAD( "gea34jb.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "game_id", 0 ) /* game security cart id */
	ROM_LOAD( "gca34jb.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	ROM_REGION( 0x002000, "m48t58", 0 ) /* timekeeper */
	ROM_LOAD( "gca34jb.22h",  0x000000, 0x002000, CRC(bc6c8bd7) SHA1(10ceec5c7bc5ca9fca88f3c083a7d97012982079) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a34jba02", 0, BAD_DUMP SHA1(1d5f9eb633f054ddbf9fba55d53e4ee263ba91dd) )
ROM_END

ROM_START( ddr5m )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gca27ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(ec526036) SHA1(f47d94d19268fdfa3ae9d42db9f2e2f9be318f2b) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gca27ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a27jaa02", 0, BAD_DUMP SHA1(0324973c98b82b72b22d2f0cd43e1924b83be667) )
ROM_END

ROM_START( ddrbocd )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gn895jaa.u1",  0x000000, 0x000224, BAD_DUMP CRC(363f427e) SHA1(adec886a07b9bd91f142f286b04fc6582205f037) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "892jaa01", 0, BAD_DUMP SHA1(46ace0feef48a2a6643c3cb4ac9164ba0beeea94) )

	DISK_REGION( "cdrom1" )
	DISK_IMAGE_READONLY( "895jaa02", 1, BAD_DUMP SHA1(cfe3a6f3ed62ba388b07045e29e22472d17dcfe4) )
ROM_END

ROM_START( ddrs2k )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0000084, "install_eeprom", 0 ) /* install security cart eeprom */
	ROM_LOAD( "ge905aa.u1",   0x000000, 0x000084, BAD_DUMP CRC(36d18e2f) SHA1(e976047dfbee62de9ad9e5de8e7629a24c29d581) )

	ROM_REGION( 0x0000084, "game_eeprom", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gc905aa.u1",   0x000000, 0x000084, BAD_DUMP CRC(21073a3e) SHA1(afa12404ceb462b9016a41c40775da87aa09cfeb) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* install security cart id */
	ROM_LOAD( "ge905aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "game_id", 0 ) /* game security cart id */
	ROM_LOAD( "gc905aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "905aaa02", 0, BAD_DUMP SHA1(1fc0f3fcc7d5d23711967023ff02c1fc76479024) )
ROM_END

ROM_START( ddrs2kj )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0000084, "install_eeprom", 0 ) /* install security cart eeprom */
	ROM_LOAD( "ge905ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(a077b0a1) SHA1(8f247b38c933a104a325ebf1f1691ef260480e1a) )

	ROM_REGION( 0x0000084, "game_eeprom", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gc905ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(b7a104b0) SHA1(0f6901e41640f729f8a084a33148a9b900475594) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* install security cart id */
	ROM_LOAD( "ge905ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "game_id", 0 ) /* game security cart id */
	ROM_LOAD( "gc905aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "905jaa02", 0, BAD_DUMP SHA1(84931345611574afd53976a0807f4163348e3c15) )
ROM_END

ROM_START( ddrmax )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gcb19ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(2255626a) SHA1(cb70c4b551265ffc6cc41f7bd2678696e8067060) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gcb19ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "b19jaa02", 0, BAD_DUMP SHA1(a156ebdef395747c64e1829237e4e7932ae251a8) )
ROM_END

ROM_START( ddrmax2 )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gcb20ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(fb7e0f58) SHA1(e6da23257a2a2ba7c69e817a91a0a8864f009386) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gcb20ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "b20jaa02", 0, BAD_DUMP SHA1(3f378e922e3182f980d07d6b2b524e33c5a00549) )
ROM_END

ROM_START( ddrsbm )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0000084, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gq894ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(cc3a47de) SHA1(f6e2e101870370b1e295a4a9ed546aa2d8bc2010) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gq894ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "894jaa02", 0, BAD_DUMP SHA1(3b2e061996d12f0e7367a579208eb746d849e070) )
ROM_END

ROM_START( ddrusa )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gka44ua.u1",   0x000000, 0x001014, BAD_DUMP CRC(2ef7c4f1) SHA1(9004d27179ece86883d01b3e6bbfeebc1b478d57) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gka44ua.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a44uaa02", 0, BAD_DUMP SHA1(2cdbe1c62d16a2be65adb7e11331fce5c8e45504) )
ROM_END

ROM_START( drmn )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gq881ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(7dca0b3f) SHA1(db6d5c527e2a99133b516e01433024d3173848c6) )

	ROM_REGION( 0x200000, "onboard.6", 0 ) /* onboard flash */
	ROM_LOAD( "gq881ja.31h",  0x000000, 0x200000, CRC(a5b86ece) SHA1(9696f0c512501574bae6e436306675894bb2352e) )
	ROM_REGION( 0x200000, "onboard.7", 0 ) /* onboard flash */
	ROM_LOAD( "gq881ja.27h",  0x000000, 0x200000, CRC(fc0b94c1) SHA1(967d374288db757d161d0e9e8e396a1176071c5f) )

	ROM_REGION( 0x002000, "m48t58", 0 ) /* timekeeper */
	ROM_LOAD( "gq881ja.22h",  0x000000, 0x002000, CRC(e834d5ec) SHA1(1c845811e43d7dfec657da288b5a38b8bc9c8366) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "881jad01", 0, BAD_DUMP SHA1(7d9d47bef636dbaa8d578f34ea9489e349d3d6df) ) // upgrade or bootleg?

	DISK_REGION( "cdrom1" )
	DISK_IMAGE_READONLY( "881jaa02", 1, NO_DUMP )
ROM_END

ROM_START( drmn2m )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* install security cart eeprom */
	ROM_LOAD( "ge912ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(1246fe5b) SHA1(b58d4f4c95e13abf639d645223565544bd79a58a) )

	ROM_REGION( 0x0001014, "game_eeprom", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gn912ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(34deea99) SHA1(f179e22eaf30453bb94177ed9c25d7996f020c99) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* install security cart id */
	ROM_LOAD( "ge912ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "game_id", 0 ) /* game security cart id */
	ROM_LOAD( "gn912ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "912jab02", 0, BAD_DUMP SHA1(19dfae94b63468d3e16d3cc4a3eeae60d5dff1d7) )
ROM_END

ROM_START( drmn2mpu )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* install security cart eeprom */
	ROM_LOAD( "ge912ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(1246fe5b) SHA1(b58d4f4c95e13abf639d645223565544bd79a58a) )

	ROM_REGION( 0x0001014, "game_eeprom", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gn912ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(34deea99) SHA1(f179e22eaf30453bb94177ed9c25d7996f020c99) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* install security cart id */
	ROM_LOAD( "ge912ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "game_id", 0 ) /* game security cart id */
	ROM_LOAD( "gn912ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "912jab02", 0, BAD_DUMP SHA1(19dfae94b63468d3e16d3cc4a3eeae60d5dff1d7) )

	DISK_REGION( "cdrom1" )
	DISK_IMAGE_READONLY( "912za01",  1, BAD_DUMP SHA1(033a310006efe164cc6a8276de42a5d555f9fea9) )
ROM_END

ROM_START( drmn3m )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* install security cart eeprom */
	ROM_LOAD( "a23jaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(90e544fa) SHA1(1feb617c36bad41aa720a6e5d3ec9e5cb2030567) )

	ROM_REGION( 0x0001014, "game_eeprom", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gca23ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(5af1b5da) SHA1(cf862ef9ab60e8da89af96266943137827e4a261) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* install security cart id */
	ROM_LOAD( "a23jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "game_id", 0 ) /* game security cart id */
	ROM_LOAD( "gca23ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a23jaa02", 0, BAD_DUMP SHA1(89e365f61a4db889621d7d9d9917bcfa2c09704e) )
ROM_END

ROM_START( drmn4m )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gea25jaa.u1",   0x000000, 0x001014, BAD_DUMP CRC(356bbbf4) SHA1(a20a8fcaed2dce50451346b1683739c96067feb1) )

	ROM_REGION( 0x200000, "onboard.0", 0 ) /* onboard flash */
	ROM_LOAD( "gea25jaa.31m", 0x000000, 0x200000, CRC(a0dd0ef4) SHA1(be4c1d3f2eb3c484b515be12b692c30cc780c36c) )
	ROM_REGION( 0x200000, "onboard.1", 0 ) /* onboard flash */
	ROM_LOAD( "gea25jaa.27m", 0x000000, 0x200000, CRC(118fa45a) SHA1(6bc6129e328f6f97a27b9f524066297b29efff5a) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gea25jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a25jaa02", 0, BAD_DUMP SHA1(8a0b761d1c282d927e2daf92519654a1c91ee1ab) )

	DISK_REGION( "multisession" )
	DISK_IMAGE_READONLY( "a25jba02", 0, BAD_DUMP SHA1(5f4aae359da610352c1004cfa1a32064d8f55d0e) )
ROM_END

ROM_START( drmn5m )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gcb05jaa.u1",   0x000000, 0x001014, BAD_DUMP CRC(6b629d68) SHA1(d01ef0677cd72c05f5f354fc6c4d9022b3506c1e) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gcb05jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "b05jaa02", 0, BAD_DUMP SHA1(7a6e7940d1441cff1d9be1bc3affc029fe6dc9e4) )

	DISK_REGION( "multisession" )
	DISK_IMAGE_READONLY( "b05jba02", 0, BAD_DUMP SHA1(822149db553ca78ad8174719a657dbbd2776b922) )
ROM_END

ROM_START( drmn6m )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gcb16jaa.u1",   0x000000, 0x001014, BAD_DUMP CRC(f6933041) SHA1(1839bb99d2db9413c58a2ed95e9039d2c7dd62ba) )

	ROM_REGION( 0x200000, "onboard.0", 0 ) /* onboard flash */
	ROM_LOAD( "gcb16jaa.31m",  0x000000, 0x200000, CRC(19de3e53) SHA1(bbb7a247bdd617a124330a946c2e8dd565b2a09c) )
	ROM_REGION( 0x200000, "onboard.1", 0 ) /* onboard flash */
	ROM_LOAD( "gcb16jaa.27m",  0x000000, 0x200000, CRC(5696e133) SHA1(aad39cc25ce5279adac8a10fb10158f4f4418c0a) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gcb16jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "b16jaa02", 0, BAD_DUMP SHA1(fa0862a9bd3a48d4f6e7b44b11ad387acc05037e) )

	DISK_REGION( "multisession" )
	DISK_IMAGE_READONLY( "b16jba02", 0, BAD_DUMP SHA1(07de74a3ca384407d99c433110085208a458653e) )
ROM_END

ROM_START( drmn7m )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gcc07jba.u1",   0x000000, 0x001014, BAD_DUMP CRC(8d9bcf10) SHA1(3d486df924ba41669675d62982396aebf8d12052) )

	ROM_REGION( 0x200000, "onboard.0", 0 ) /* onboard flash */
	ROM_LOAD( "gcc07jba.31m",  0x000000, 0x200000, CRC(7120d1ce) SHA1(4df9828150120762b99c5b212bc7a91b0d525bce) )
	ROM_REGION( 0x200000, "onboard.1", 0 ) /* onboard flash */
	ROM_LOAD( "gcc07jba.27m",  0x000000, 0x200000, CRC(9393fe8e) SHA1(f60752e3e397121f3d4856a634e1c8ce5fc465b5) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gcc07jba.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "c07jca02", 0, BAD_DUMP SHA1(1f27e0a22c7f4f37ec3a09984ce197e390340d27) )

	DISK_REGION( "multisession" )
	DISK_IMAGE_READONLY( "c07jda02", 0, BAD_DUMP SHA1(7c22ebbda11bdaf85c3441d7a6f3497994cd957f) )
ROM_END

ROM_START( drmn7ma )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gcc07jaa.u1",   0x000000, 0x001014, BAD_DUMP CRC(b675b39b) SHA1(9639db913821641cee619d7cc520de5d0c3ae7fa) )

	ROM_REGION( 0x200000, "onboard.0", 0 ) /* onboard flash */
	ROM_LOAD( "gcc07jaa.31m",  0x000000, 0x200000, CRC(1e1cbfe3) SHA1(6c942820f915ea0e01f0e736d70780ad8408aa69) )
	ROM_REGION( 0x200000, "onboard.1", 0 ) /* onboard flash */
	ROM_LOAD( "gcc07jaa.27m",  0x000000, 0x200000, CRC(49d27b57) SHA1(e62737fe8665d837c2cebd1dcf4577a021d8cdb1) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gcc07jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "c07jaa02", 0, BAD_DUMP SHA1(96c410745d1fd14059bf11987655ed998a9b79dd) )

	DISK_REGION( "multisession" )
	DISK_IMAGE_READONLY( "c07jba02", 0, BAD_DUMP SHA1(25e1a3ff7886c409d16e40ca1798b01b11546755) )
ROM_END

ROM_START( drmn8m )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gcc38jaa.u1",   0x000000, 0x001014, BAD_DUMP CRC(aaa03630) SHA1(4976b0c2e1b4458840a165bd889861d62289ad89) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gcc38jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "c38jaa02", 0, BAD_DUMP SHA1(d19ae541557405a4484145f4237f3c868375c72e) )

	DISK_REGION( "multisession" )
	DISK_IMAGE_READONLY( "c38jba02", 0, BAD_DUMP SHA1(d963064678978d489474d1ca22c1f249c6f60232) )
ROM_END

ROM_START( drmn9m )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gcd09jaa.u1",   0x000000, 0x001014, BAD_DUMP CRC(a1201529) SHA1(4a82f2ee9b049a16c00b7dcd905c43c1a06d60ee) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gcd09jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "d09jaa02", 0, BAD_DUMP SHA1(33f3e48ed5a8becd8c4714413e454328d8d5baae) )

	DISK_REGION( "multisession" )
	DISK_IMAGE_READONLY( "d09jba02", 0, NO_DUMP )
ROM_END

ROM_START( drmn10m )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gcd40jaa.u1",   0x000000, 0x001014, BAD_DUMP CRC(ef0983a7) SHA1(06127b9fd786eca64eea50c40f7f73717b631e59) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gcd40jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "d40jaa02", 0, BAD_DUMP SHA1(68b2038f0cd2d461f608945d1e243f2b6979efaa) )

	DISK_REGION( "multisession" )
	DISK_IMAGE_READONLY( "d40jba02", 0, BAD_DUMP SHA1(0ded9e0a6c77b181e7b6beb1dbdfa17dee4acd90) )
ROM_END

ROM_START( dmx )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "ge874ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(c5536373) SHA1(1492221f7dd9485f7745ecb0a982a88c8e768e53) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "ge874ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "874jaa", 0, BAD_DUMP SHA1(3338a784efdca4f8bdcc83d2c9a6bbe7f7046d5c) )
ROM_END

ROM_START( dmx2m )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gca39ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(ecc75eb7) SHA1(af66ced28ba5e79ae32ae0ef12d2ebe4207f3822) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gca39ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a39jaa02", 0, BAD_DUMP SHA1(3d021448df857c12f6d46a20e14ae0fc6d342dcc) )
ROM_END

ROM_START( dmx2majp )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gca38ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(99a746b8) SHA1(333236e59a707ecaf840a66f9b947ceade2cf2c9) )

	ROM_REGION( 0x200000, "onboard.0", 0 ) /* onboard flash */
	ROM_LOAD( "gca38ja.31m",  0x000000, 0x200000, CRC(a0f54ab5) SHA1(a5ae67d7619393779c79a2e227cac0675eeef538) )
	ROM_REGION( 0x200000, "onboard.1", 0 ) /* onboard flash */
	ROM_LOAD( "gca38ja.27m",  0x000000, 0x200000, CRC(6c3934b8) SHA1(f0e4a692b6caaf60fefaec87fd23da577439f69d) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gca38ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a38jaa02", 0, BAD_DUMP SHA1(d26c481ef8a70bba75bcdf41f9ceb3a49c245986) )
ROM_END

ROM_START( dncfrks )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gk874ka.u1",   0x000000, 0x001014, BAD_DUMP CRC(7a6f4672) SHA1(2e009e57760e92f48070a69cff5597c37a4783a2) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gk874ka.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "874kaa", 0, BAD_DUMP SHA1(4d1e843417ea96635eeba0cef944e83fdb72565c) )
ROM_END

ROM_START( dsem )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "ge936ea.u1",   0x000000, 0x000224, BAD_DUMP CRC(0f5b7ae3) SHA1(646dd49da1216cc2d3d6920bc9b3447d55ebfbf0) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "ge936ea.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "936eaa", 0, BAD_DUMP SHA1(7cacc15ae065d47af31f1008374ec8241dba0d55) )
ROM_END

ROM_START( dsem2 )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gkc23ea.u1",   0x000000, 0x001014, BAD_DUMP CRC(aec2421a) SHA1(5ea9e9ce6161ebc99a50db0b7304385511bd4553) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gkc23ea.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "c23eaa02", 0, BAD_DUMP SHA1(46868c97530db5be1b43ffa32744e3e12495c243) )
ROM_END

ROM_START( dsfdct )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0000084, "install_eeprom", 0 ) /* install security cart eeprom */
	ROM_LOAD( "ge887ja_gn887ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(08a60147) SHA1(0d39dca5e9e17fff0e64f296c8416e4ca23fdc1b) )

	ROM_REGION( 0x0000084, "game_eeprom", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gc910jc.u1",   0x000000, 0x000084, BAD_DUMP CRC(3c1ca973) SHA1(32211a72e3ac88b2723f82dac0b26f93031b3a9c) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* install security cart id */
	ROM_LOAD( "ge887ja_gn887ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	ROM_REGION( 0x000008, "game_id", 0 ) /* game security cart id */
	ROM_LOAD( "gc910jc.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "910jca02", 0, BAD_DUMP SHA1(0c868f3c9f696d291e8f27687e3ad83e453a4894) )
ROM_END

ROM_START( dsfdcta )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "install_eeprom", 0 ) /* install security cart eeprom */
	ROM_LOAD( "gn884ja.u1",  0x000000, 0x000084, BAD_DUMP CRC(ce6b98ce) SHA1(75549d9470345ce06d2706d373b19416d97e5b9a) )

	ROM_REGION( 0x0000084, "game_eeprom", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gc910ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(59a23808) SHA1(fcff1c68ff6cfbd391ac997a40fb5253fc62de82) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* install security cart id */
	ROM_LOAD( "gn884ja.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	ROM_REGION( 0x000008, "game_id", 0 ) /* game security cart id */
	ROM_LOAD( "gc910ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "910jaa02", 0, BAD_DUMP SHA1(70851c383e3876c4a697a99706fbaae2dafcb0e0) )
ROM_END

ROM_START( dsfdr )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* install security cart eeprom */
	ROM_LOAD( "gea37ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(5321055e) SHA1(d06b0dca9caba8249d71340469ad9083b02fd087) )

	ROM_REGION( 0x0001014, "game_eeprom", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gca37ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(b6d9e7f9) SHA1(bc5f491de53a96d46745df09bc94e7853052296c) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* install security cart id */
	ROM_LOAD( "gea37ja.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "game_id", 0 ) /* game security cart id */
	ROM_LOAD( "gca37ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a37jaa02", 0, BAD_DUMP SHA1(c6a23b910e884aa0d4afc388dbc8379e0d09611a) )
ROM_END

ROM_START( dsftkd )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gn884ja.u1",  0x000000, 0x000084, BAD_DUMP CRC(ce6b98ce) SHA1(75549d9470345ce06d2706d373b19416d97e5b9a) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gn884ja.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "884jaa02", 0, BAD_DUMP SHA1(80f02fcb7ea5b6394a2a58f12b73d87a1826d7f4) )
ROM_END

ROM_START( dstage )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gn845ea.u1",   0x000000, 0x000224, BAD_DUMP CRC(db643af7) SHA1(881221da640b883302e657b906ea0a4e74555679) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "845ea", 0, BAD_DUMP SHA1(d3f9290d4dadb5e9b82ebe77abf7b99d1a89f716) )
ROM_END

ROM_START( fbait2bc )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gc865ua.u1", 0x000000, 0x000224, BAD_DUMP CRC(ea8f0b4b) SHA1(363b1ea1a520b239ba8bca867366bbe8a9977a43) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "865uab02", 0, BAD_DUMP SHA1(d14dc066d4c16fba1e9b31d5f042ad249c4b5137) )
ROM_END

ROM_START( fbaitbc )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "ge765ua.u1", 0x000000, 0x000224, BAD_DUMP CRC(588748c6) SHA1(ea1ead61e0dcb324ef7b6106cae00bcf6702d6c4) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "765uab02", 0, BAD_DUMP SHA1(07b09e763e4b90108aa924b518221b16667a7133) )
ROM_END

ROM_START( fbaitmc )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gx889ea.u1", 0x000000, 0x000224, BAD_DUMP CRC(753ad84e) SHA1(e024cefaaee7c9945ccc1f9a3d896b8560adce2e) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "889ea", 0, BAD_DUMP SHA1(0b567bf2f03ee8089e0b021ea502a53b3f6fe7ac) )
ROM_END

ROM_START( fbaitmca )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gx889aa.u1", 0x000000, 0x000224, BAD_DUMP CRC(9c22aae8) SHA1(c107b0bf7fa76708f2d4f6aaf2cf27b3858378a3) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "889aa", 0, BAD_DUMP SHA1(0b567bf2f03ee8089e0b021ea502a53b3f6fe7ac) )
ROM_END

ROM_START( fbaitmcj )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gx889ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(6278603c) SHA1(d6b59e270cfe4016e12565aedec8a4f0702e1a6f) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "889ja", 0, BAD_DUMP SHA1(0b567bf2f03ee8089e0b021ea502a53b3f6fe7ac) )
ROM_END

ROM_START( fbaitmcu )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gx889ua.u1", 0x000000, 0x000224, BAD_DUMP CRC(67b91e54) SHA1(4d94bfab08e2bf6e34ee606dd3c4e345d8e5d158) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "889ua", 0, BAD_DUMP SHA1(0b567bf2f03ee8089e0b021ea502a53b3f6fe7ac) )
ROM_END

ROM_START( fghtmn )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gq918eaa.u1",  0x000000, 0x000224, CRC(f3342ff5) SHA1(d3d6ecc22396f74b99ad7aab7908cd542c518977) )

	ROM_REGION( 0x200000, "onboard.0", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.31m", 0x000000, 0x200000, CRC(3653b5d7) SHA1(1deb44335b7a38506fb30da40e0ca61b96aea7bb) )
	ROM_REGION( 0x200000, "onboard.1", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.27m", 0x000000, 0x200000, CRC(27d48c97) SHA1(c140d4bdfa869fbcae1133bbfe73a346e6f46cb8) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gq918eaa.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "918xxb02", 0, BAD_DUMP SHA1(8ced8952fff3e70ce0621a491f0973af5a6ccd82) )
ROM_END

ROM_START( fghtmna )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gq918aaa.u1",  0x000000, 0x000224, CRC(1a2c5d53) SHA1(ab7e44a83e8cd271e2bf8580881a3050d35641df) )

	ROM_REGION( 0x200000, "onboard.0", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.31m", 0x000000, 0x200000, CRC(3653b5d7) SHA1(1deb44335b7a38506fb30da40e0ca61b96aea7bb) )
	ROM_REGION( 0x200000, "onboard.1", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.27m", 0x000000, 0x200000, CRC(27d48c97) SHA1(c140d4bdfa869fbcae1133bbfe73a346e6f46cb8) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gq918aaa.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "918xxb02", 0, BAD_DUMP SHA1(8ced8952fff3e70ce0621a491f0973af5a6ccd82) )
ROM_END

ROM_START( fghtmnk )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gq918kaa.u1",  0x000000, 0x000224, CRC(cf32990b) SHA1(bf49b8560f008696b45a3f7f03fa7b3395635b0f) )

	ROM_REGION( 0x200000, "onboard.0", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.31m", 0x000000, 0x200000, CRC(3653b5d7) SHA1(1deb44335b7a38506fb30da40e0ca61b96aea7bb) )
	ROM_REGION( 0x200000, "onboard.1", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.27m", 0x000000, 0x200000, CRC(27d48c97) SHA1(c140d4bdfa869fbcae1133bbfe73a346e6f46cb8) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gq918kaa.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "918xxb02", 0, BAD_DUMP SHA1(8ced8952fff3e70ce0621a491f0973af5a6ccd82) )
ROM_END

ROM_START( fghtmnu )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gq918uaa.u1",  0x000000, 0x000224, CRC(e1b7e9ef) SHA1(5767f47cb9a689601fb92c6a494563c5ffdde04c) )

	ROM_REGION( 0x200000, "onboard.0", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.31m", 0x000000, 0x200000, CRC(3653b5d7) SHA1(1deb44335b7a38506fb30da40e0ca61b96aea7bb) )
	ROM_REGION( 0x200000, "onboard.1", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.27m", 0x000000, 0x200000, CRC(27d48c97) SHA1(c140d4bdfa869fbcae1133bbfe73a346e6f46cb8) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gq918uaa.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "918xxb02", 0, BAD_DUMP SHA1(8ced8952fff3e70ce0621a491f0973af5a6ccd82) )
ROM_END

ROM_START( hndlchmp )
	SYS573_BIOS_A

	ROM_REGION( 0x200000, "onboard.0", 0 ) /* onboard flash */
	ROM_LOAD( "710ja.31m",    0x000000, 0x200000, CRC(f5f71b1d) SHA1(7d518e5333f44e6ec921a1e882df970953814b6e) )
	ROM_REGION( 0x200000, "onboard.1", 0 ) /* onboard flash */
	ROM_LOAD( "710ja.27m",    0x000000, 0x200000, CRC(b3d8c037) SHA1(678b88c37111d1fde8996c7d71b66ec1c4f161fe) )
	ROM_REGION( 0x200000, "onboard.2", 0 ) /* onboard flash */
	ROM_LOAD( "710ja.31l",    0x000000, 0x200000, CRC(78e8556c) SHA1(9f6bb651ddeb042ebf1ba057d4932494149f47d6) )
	ROM_REGION( 0x200000, "onboard.3", 0 ) /* onboard flash */
	ROM_LOAD( "710ja.27l",    0x000000, 0x200000, CRC(f6a87155) SHA1(269bfdf05ee4ab2e4b87b6e92045e56d0557a576) )
	ROM_REGION( 0x200000, "onboard.4", 0 ) /* onboard flash */
	ROM_LOAD( "710ja.31j",    0x000000, 0x200000, CRC(bdc05d16) SHA1(ee397950f7e7e910fdc05737f99604e43d288719) )
	ROM_REGION( 0x200000, "onboard.5", 0 ) /* onboard flash */
	ROM_LOAD( "710ja.27j",    0x000000, 0x200000, CRC(ad925ed3) SHA1(e3222308961851cccee2de9da804f74854907451) )
	ROM_REGION( 0x200000, "onboard.6", 0 ) /* onboard flash */
	ROM_LOAD( "710ja.31h",    0x000000, 0x200000, CRC(a0293108) SHA1(2e5651a4c1b8e021cc3060db138c9fe7c28caa3b) )
	ROM_REGION( 0x200000, "onboard.7", 0 ) /* onboard flash */
	ROM_LOAD( "710ja.27h",    0x000000, 0x200000, CRC(aed26efe) SHA1(20b6fccd0bc5495d8258b976f72d330d6315c6f6) )

	ROM_REGION( 0x002000, "m48t58", 0 ) /* timekeeper */
	ROM_LOAD( "710ja.22h",    0x000000, 0x002000, CRC(b784de91) SHA1(048157e9ad6df46656dbac6349b0c821254e1c37) )
ROM_END

ROM_START( gtrfrks )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gq886eac.u1",  0x000000, 0x000224, BAD_DUMP CRC(06bd6c4f) SHA1(61930e467ad135e2f31393ff5af981ed52f3bef9) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "886__c02", 0, BAD_DUMP SHA1(80293512c4b914ef98acb1bbc7e3a2ed944a0dad) )
ROM_END

ROM_START( gtrfrksu )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gq886uac.u1",  0x000000, 0x000224, BAD_DUMP CRC(143eaa55) SHA1(51a4fa3693f1cb1646a8986003f9b6cc1ae8b630) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "886__c02", 0, BAD_DUMP SHA1(80293512c4b914ef98acb1bbc7e3a2ed944a0dad) )
ROM_END

ROM_START( gtrfrksj )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gq886jac.u1",  0x000000, 0x000224, BAD_DUMP CRC(11ffd43d) SHA1(27f4f4d782604379254fb98c3c57e547aa4b321f) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "886__c02", 0, BAD_DUMP SHA1(80293512c4b914ef98acb1bbc7e3a2ed944a0dad) )
ROM_END

ROM_START( gtrfrksa )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gq886aac.u1",  0x000000, 0x000224, BAD_DUMP CRC(efa51ee9) SHA1(3374d936de69c287e0161bc526546441c2943555) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "886__c02", 0, BAD_DUMP SHA1(80293512c4b914ef98acb1bbc7e3a2ed944a0dad) )
ROM_END

ROM_START( gtrfrk2m )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gq883jad.u1",  0x000000, 0x000084, BAD_DUMP CRC(687868c4) SHA1(1230e74e4cf17953febe501df56d8bbec1de9356) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gq883jad.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "929jbb02", 0, BAD_DUMP SHA1(4f6bb0150ad6ed574dd7583ccd60604028663b2a) )
ROM_END

ROM_START( gtrfrk3m )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* install security cart eeprom */
	ROM_LOAD( "949jaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(96c21d71) SHA1(871f1f0429154a486e547e182534db1557008dd6) )

	ROM_REGION( 0x0001014, "game_eeprom", 0 ) /* game security cart eeprom */
	ROM_LOAD( "ge949jab.u1",  0x000000, 0x001014, BAD_DUMP CRC(8645e17f) SHA1(e8a833384cb6bdb05870fcd44e7c8ed48a03c852) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* install security cart id */
	ROM_LOAD( "949jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "game_id", 0 ) /* game security cart id */
	ROM_LOAD( "ge949jab.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "949jac01", 0, BAD_DUMP SHA1(ff017dd5c0ecbdb8935d0d4656a45e9fab10ef82) )

	DISK_REGION( "cdrom1" )
	DISK_IMAGE_READONLY( "949jab02", 1, BAD_DUMP SHA1(ad629c9bafbdc4bf6c679918a5fae2bcfdb39332) )
ROM_END

ROM_START( gtfrk3ma )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* install security cart eeprom */
	ROM_LOAD( "949jaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(96c21d71) SHA1(871f1f0429154a486e547e182534db1557008dd6) )

	ROM_REGION( 0x0001014, "game_eeprom", 0 ) /* game security cart eeprom */
	ROM_LOAD( "ge949jab.u1",  0x000000, 0x001014, BAD_DUMP CRC(8645e17f) SHA1(e8a833384cb6bdb05870fcd44e7c8ed48a03c852) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* install security cart id */
	ROM_LOAD( "949jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "game_id", 0 ) /* game security cart id */
	ROM_LOAD( "ge949jab.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "949jab02", 0, BAD_DUMP SHA1(ad629c9bafbdc4bf6c679918a5fae2bcfdb39332) )
ROM_END

ROM_START( gtfrk3mb )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* game security cart eeprom */
	ROM_LOAD( "ge949jaa.u1",  0x000000, 0x001014, BAD_DUMP CRC(61f35ee1) SHA1(0a2b66742364d76ec18647b2761590bd49229625) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* game security cart id */
	ROM_LOAD( "ge949jaa.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "949jaz02", 0, BAD_DUMP SHA1(b0c786ba420a34fcbd16bc36a137f6ae87b7dfa8) )
ROM_END

ROM_START( gtrfrk4m )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* install security cart eeprom */
	ROM_LOAD( "a24jaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(29e326fe) SHA1(41a600105b08accc9d7ebd2b8ae08c0863758aa0) )

	ROM_REGION( 0x0001014, "game_eeprom", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gea24ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(d1fccf11) SHA1(6dcd79f3171d6e4bd7e1149901638f8ea58ff623) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* install security cart id */
	ROM_LOAD( "a24jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "game_id", 0 ) /* game security cart id */
	ROM_LOAD( "gea24ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a24jaa02", 0, BAD_DUMP SHA1(bc0303f5a6a19484cd35890cc9934ee0bcabb2ad) )
ROM_END

ROM_START( gtrfrk5m )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gea26jaa.u1",  0x000000, 0x001014, BAD_DUMP CRC(c2725fca) SHA1(b70a3266c61af5cbe0478a6f3dd850ebcab980dc) )

	ROM_REGION( 0x200000, "onboard.0", 0 ) /* onboard flash */
	ROM_LOAD( "gea26jaa.31m", 0x000000, 0x200000, CRC(1a25e660) SHA1(dbd8fad0bac307723c70d00763cadf4261a7ed73) )
	ROM_REGION( 0x200000, "onboard.1", 0 ) /* onboard flash */
	ROM_LOAD( "gea26jaa.27m", 0x000000, 0x200000, CRC(345dc5f2) SHA1(61af3fcfe6119c1e8e18b92693855ab4fe708b30) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gea26jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a26jaa02", 0, BAD_DUMP SHA1(9909e08abff780db6fd7a5fbcc57ffbe14ae08ce) )
ROM_END

ROM_START( gtrfrk6m )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gcb06ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(673c98ab) SHA1(b1d889bf4fc5e425056acb6b72b2c563966fb7d7) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gcb06ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "b06jaa02", 0, BAD_DUMP SHA1(2ea53ef492da63183a28c54afde07fce323fe42e) )
ROM_END

ROM_START( gtrfrk7m )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gcb17jaa.u1",   0x000000, 0x001014, BAD_DUMP CRC(5a338c31) SHA1(0fd9ee306335858dd6bef680a62557a8bf055cc3) )

	ROM_REGION( 0x200000, "onboard.0", 0 ) /* onboard flash */
	ROM_LOAD( "gcb17jaa.31m", 0x000000, 0x200000, CRC(1e1cbfe3) SHA1(6c942820f915ea0e01f0e736d70780ad8408aa69) )
	ROM_REGION( 0x200000, "onboard.1", 0 ) /* onboard flash */
	ROM_LOAD( "gcb17jaa.27m", 0x000000, 0x200000, CRC(7e7da9a9) SHA1(1882418779a48b5aefd113895756116379a6a4f7) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gcb17jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "b17jaa02", 0, BAD_DUMP SHA1(d38dc22011b71b0e4167f1728a8794ea4b9c5396) )
ROM_END

ROM_START( gtrfrk8m )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gcc08jba.u1",   0x000000, 0x001014, BAD_DUMP CRC(db4b3027) SHA1(65ca32fcacda18954a4e8352dbb9bf583dfdd121) )

	ROM_REGION( 0x200000, "onboard.0", 0 ) /* onboard flash */
	ROM_LOAD( "gcc08jba.31m", 0x000000, 0x200000, CRC(ddef5efe) SHA1(7c3a219eacf63f55894e81cb0e41753176191708) )
	ROM_REGION( 0x200000, "onboard.1", 0 ) /* onboard flash */
	ROM_LOAD( "gcc08jba.27m", 0x000000, 0x200000, CRC(9393fe8e) SHA1(f60752e3e397121f3d4856a634e1c8ce5fc465b5) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gcc08jba.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "c08jba02", 0, BAD_DUMP SHA1(8e352ed8ade581b7c9bb579fc56003ea1831202c) )
ROM_END

ROM_START( gtrfrk8ma )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gcc08jaa.u1",   0x000000, 0x001014, BAD_DUMP CRC(9c58f22b) SHA1(41ade23bac86e437b1f12c5730b8cce292ffe4f8) )

	ROM_REGION( 0x200000, "onboard.0", 0 ) /* onboard flash */
	ROM_LOAD( "gcc08jaa.31m", 0x000000, 0x200000, CRC(aa723d4c) SHA1(5f55ddaf7f21b624deac99cc40b89989cd6f3a3d) )
	ROM_REGION( 0x200000, "onboard.1", 0 ) /* onboard flash */
	ROM_LOAD( "gcc08jaa.27m", 0x000000, 0x200000, CRC(49d27b57) SHA1(e62737fe8665d837c2cebd1dcf4577a021d8cdb1) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gcc08jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "c08jaa02", 0, BAD_DUMP SHA1(7a1d97f74ec4d643ff7d3981d66b551cbf9e57f0) )
ROM_END

ROM_START( gtrfrk9m )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gcc39jaa.u1",   0x000000, 0x001014, BAD_DUMP CRC(afb75814) SHA1(027dc2ae3444d10c14169f1f354ffcc928f62fb3) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gcc39jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "c39jaa02", 0, BAD_DUMP SHA1(d0696b29976a6bc01c3a1fefe09dbee721ff3ffb) )
ROM_END

ROM_START( gtfrk10m )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gcd10jab.u1",   0x000000, 0x001014, BAD_DUMP CRC(43520577) SHA1(a0749e766688032fe6558707b564288b95da9b8d) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gcd10jab.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "d10jab01", 0, BAD_DUMP SHA1(c84858b412f0798a65cf3059c743501f32ad7280) )

	DISK_REGION( "cdrom1" )
	DISK_IMAGE_READONLY( "d10jaa02", 1, BAD_DUMP SHA1(d4e4460ca3edc1b365af593757557c6cf5b7b3ec) )
ROM_END

ROM_START( gtfrk10ma )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gcd10jaa.u1",   0x000000, 0x001014, BAD_DUMP CRC(43520577) SHA1(a0749e766688032fe6558707b564288b95da9b8d) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gcd10jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "d10jaa02", 0, BAD_DUMP SHA1(d4e4460ca3edc1b365af593757557c6cf5b7b3ec) )
ROM_END

ROM_START( gtfrk10mb )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gcd10jab.u1",   0x000000, 0x001014, BAD_DUMP CRC(43520577) SHA1(a0749e766688032fe6558707b564288b95da9b8d) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gcd10jab.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "d10jba02", 0, BAD_DUMP SHA1(80893da422268cc1f89688289cdec981c4f9feb2) )
ROM_END

ROM_START( gtfrk11m )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gcd39ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(9bd81d0a) SHA1(c95f6d7317bf88177f7217de4ba4376485d5cdbf) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gcd39ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "d39jaa02", 0, BAD_DUMP SHA1(7a87ee331ba0301bb8724c398e6c77cfb9c172a7) )
ROM_END

ROM_START( gunmania )
	SYS573_BIOS_A

	ROM_REGION( 0x000008, "gunmania_id", 0 ) /* digital board id */		\
	ROM_LOAD( "ds2401",        0x000000, 0x000008, CRC(2b977f4d) SHA1(2b108a56653f91cb3351718c45dfcf979bc35ef1) )

	ROM_REGION( 0x200000, "onboard.0", 0 ) /* onboard flash */
	ROM_LOAD( "gl906jaa.31m",  0x000000, 0x200000, CRC(6c02d360) SHA1(961bd9df4928a3dead9da6a88641547cae4c4dbd) )
	ROM_REGION( 0x200000, "onboard.1", 0 ) /* onboard flash */
	ROM_LOAD( "gl906jaa.27m",  0x000000, 0x200000, CRC(057b5bce) SHA1(979e3fb5496920c3f9eb7111425c08d80c9076a5) )
	ROM_REGION( 0x200000, "onboard.2", 0 ) /* onboard flash */
	ROM_LOAD( "gl906jaa.31l",  0x000000, 0x200000, CRC(3f3abf8f) SHA1(9c93e58fad16ccbe4bc4499a1a15af134243c154) )
	ROM_REGION( 0x200000, "onboard.3", 0 ) /* onboard flash */
	ROM_LOAD( "gl906jaa.27l",  0x000000, 0x200000, CRC(f2be642d) SHA1(6c46197a0d114ac90824de1fc4df12db561844e5) )
	ROM_REGION( 0x200000, "onboard.4", 0 ) /* onboard flash */
	ROM_LOAD( "gl906jaa.31j",  0x000000, 0x200000, CRC(889a4733) SHA1(1f6578d95c0331fdf3235ef7d899d5bd083ff6a0) )
	ROM_REGION( 0x200000, "onboard.5", 0 ) /* onboard flash */
	ROM_LOAD( "gl906jaa.27j",  0x000000, 0x200000, CRC(984193a8) SHA1(1a310e22a80cb4854b138f737f679384c98b2e46) )
	ROM_REGION( 0x200000, "onboard.6", 0 ) /* onboard flash */
	ROM_LOAD( "gl906jaa.31h",  0x000000, 0x200000, CRC(202236c1) SHA1(ecd58f2b325fdefe2ac6cdd6f4edd212432e149a) )
	ROM_REGION( 0x200000, "onboard.7", 0 ) /* onboard flash */
	ROM_LOAD( "gl906jaa.27h",  0x000000, 0x200000, CRC(8861b858) SHA1(2a67d465786759a74162ebebc0a44ba9309ffa60) )

	ROM_REGION( 0x200000, "pccard2.0", 0 ) /* PCCARD2 */
	ROM_LOAD( "gl906jaa.h",   0x0000000, 0x200000, BAD_DUMP CRC(b2f3dc23) SHA1(65f7b986d2b12b26dfc364e6f990f7c504b5519f) )
	ROM_REGION( 0x200000, "pccard2.1", 0 ) /* PCCARD2 */
	ROM_LOAD( "gl906jaa.8",   0x0000000, 0x200000, BAD_DUMP CRC(5e40ed31) SHA1(5594b0c42b2ae8dd06259b93cc29bc3b44a85d44) )
	ROM_REGION( 0x200000, "pccard2.2", 0 ) /* PCCARD2 */
	ROM_LOAD( "gl906jaa.g",   0x0000000, 0x200000, BAD_DUMP CRC(8d89877e) SHA1(7d76d48d64d7ac5411d714a4bb83f37e3e5b8df6) )
	ROM_REGION( 0x200000, "pccard2.3", 0 ) /* PCCARD2 */
	ROM_LOAD( "gl906jaa.7",   0x0000000, 0x200000, BAD_DUMP CRC(8d89877e) SHA1(7d76d48d64d7ac5411d714a4bb83f37e3e5b8df6) )
	ROM_REGION( 0x200000, "pccard2.4", 0 ) /* PCCARD2 */
	ROM_LOAD( "gl906jaa.f",   0x0000000, 0x200000, BAD_DUMP CRC(8d89877e) SHA1(7d76d48d64d7ac5411d714a4bb83f37e3e5b8df6) )
	ROM_REGION( 0x200000, "pccard2.5", 0 ) /* PCCARD2 */
	ROM_LOAD( "gl906jaa.6",   0x0000000, 0x200000, BAD_DUMP CRC(8d89877e) SHA1(7d76d48d64d7ac5411d714a4bb83f37e3e5b8df6) )
	ROM_REGION( 0x200000, "pccard2.6", 0 ) /* PCCARD2 */
	ROM_LOAD( "gl906jaa.e",   0x0000000, 0x200000, BAD_DUMP CRC(074370b9) SHA1(2dabc3bebc52b3fe09a73ced4ccbe9a5065feb70) )
	ROM_REGION( 0x200000, "pccard2.7", 0 ) /* PCCARD2 */
	ROM_LOAD( "gl906jaa.5",   0x0000000, 0x200000, BAD_DUMP CRC(9645dd9e) SHA1(34a85d349496eaed124db3cd8f40724f92fa3600) )
	ROM_REGION( 0x200000, "pccard2.8", 0 ) /* PCCARD2 */
	ROM_LOAD( "gl906jaa.4",   0x0000000, 0x200000, BAD_DUMP CRC(0daf5c60) SHA1(fc507cb9bb746d217d4cccf393dc311d3e64a16f) )
	ROM_REGION( 0x200000, "pccard2.9", 0 ) /* PCCARD2 */
	ROM_LOAD( "gl906jaa.d",   0x0000000, 0x200000, BAD_DUMP CRC(e51dc4c2) SHA1(8214cbd329d68df1aa625801c5e8d6b1f30358aa) )
	ROM_REGION( 0x200000, "pccard2.10", 0 ) /* PCCARD2 */
	ROM_LOAD( "gl906jaa.3",   0x0000000, 0x200000, BAD_DUMP CRC(8d89877e) SHA1(7d76d48d64d7ac5411d714a4bb83f37e3e5b8df6) )
	ROM_REGION( 0x200000, "pccard2.11", 0 ) /* PCCARD2 */
	ROM_LOAD( "gl906jaa.c",   0x0000000, 0x200000, BAD_DUMP CRC(8d89877e) SHA1(7d76d48d64d7ac5411d714a4bb83f37e3e5b8df6) )
	ROM_REGION( 0x200000, "pccard2.12", 0 ) /* PCCARD2 */
	ROM_LOAD( "gl906jaa.2",   0x0000000, 0x200000, BAD_DUMP CRC(8d89877e) SHA1(7d76d48d64d7ac5411d714a4bb83f37e3e5b8df6) )
	ROM_REGION( 0x200000, "pccard2.13", 0 ) /* PCCARD2 */
	ROM_LOAD( "gl906jaa.b",   0x0000000, 0x200000, BAD_DUMP CRC(8d89877e) SHA1(7d76d48d64d7ac5411d714a4bb83f37e3e5b8df6) )
	ROM_REGION( 0x200000, "pccard2.14", 0 ) /* PCCARD2 */
	ROM_LOAD( "gl906jaa.1",   0x0000000, 0x200000, BAD_DUMP CRC(11d5630b) SHA1(26a94780eac653fb4912c533040356d79ba0fe94) )
	ROM_REGION( 0x200000, "pccard2.15", 0 ) /* PCCARD2 */
	ROM_LOAD( "gl906jaa.a",   0x0000000, 0x200000, BAD_DUMP CRC(e62d18e6) SHA1(b32b884fbd9b5a65efcdd1b50dd3b7a99fdceeb9) )
ROM_END

ROM_START( hyperbbc )
	SYS573_BIOS_A

	ROM_REGION( 0x200000, "onboard.0", 0 ) /* onboard flash */
	ROM_LOAD( "876ea.31m",    0x000000, 0x200000, CRC(a76043cb) SHA1(1c37034298abf3219d0bba29f4fcea8d83782926) )
	ROM_REGION( 0x200000, "onboard.1", 0 ) /* onboard flash */
	ROM_LOAD( "876ea.27m",    0x000000, 0x200000, CRC(689ddd94) SHA1(512ca1529695f4f79ca8c1b8f64bb0067137e430) )
	ROM_REGION( 0x200000, "onboard.2", 0 ) /* onboard flash */
	ROM_LOAD( "876ea.31l",    0x000000, 0x200000, CRC(d011c7a5) SHA1(8861b62c8b654b8e719600a37337ae44e6438899) )
	ROM_REGION( 0x200000, "onboard.3", 0 ) /* onboard flash */
	ROM_LOAD( "876ea.27l",    0x000000, 0x200000, CRC(950a5267) SHA1(373a7305a090d1e347bfeb62cc2db55cde2a106e) )
	ROM_REGION( 0x200000, "onboard.4", 0 ) /* onboard flash */
	ROM_LOAD( "876ea.31j",    0x000000, 0x200000, CRC(ae497ebc) SHA1(ef131e60726db94f0d9ceab70bce02c0de080ede) )
	ROM_REGION( 0x200000, "onboard.5", 0 ) /* onboard flash */
	ROM_LOAD( "876ea.27j",    0x000000, 0x200000, CRC(9c156b1b) SHA1(bf07d71cc1f7e9e14beb9f9dfb71667ef2b54f8d) )
	ROM_REGION( 0x200000, "onboard.6", 0 ) /* onboard flash */
	ROM_LOAD( "876ea.31h",    0x000000, 0x200000, CRC(368372fb) SHA1(5cc4cb72e182c9e4d0737352e029fd703ba2f516) )
	ROM_REGION( 0x200000, "onboard.7", 0 ) /* onboard flash */
	ROM_LOAD( "876ea.27h",    0x000000, 0x200000, CRC(49175f99) SHA1(0154f6332ed210b6f0af20ba622133cde0994b7f) )

	ROM_REGION( 0x002000, "m48t58", 0 ) /* timekeeper */
	ROM_LOAD( "876ea.22h",    0x000000, 0x002000, CRC(8e11d196) SHA1(e7442fdd611f4290d531b1ebdc9f487e323fd531) )
ROM_END

ROM_START( hyperbbca )
	SYS573_BIOS_A

	ROM_REGION( 0x200000, "onboard.0", 0 ) /* onboard flash */
	ROM_LOAD( "876aa.31m",    0x000000, 0x200000, CRC(677f8b0a) SHA1(a4c1029a70f5733f64a4f4dde4a568d2cb4dd11d) )
	ROM_REGION( 0x200000, "onboard.1", 0 ) /* onboard flash */
	ROM_LOAD( "876aa.27m",    0x000000, 0x200000, CRC(0af35a7d) SHA1(086ad70c8bf4bbe5d9748e4d47c639b4250270fc) )
	ROM_REGION( 0x200000, "onboard.2", 0 ) /* onboard flash */
	ROM_LOAD( "876ea.31l",    0x000000, 0x200000, CRC(d011c7a5) SHA1(8861b62c8b654b8e719600a37337ae44e6438899) )
	ROM_REGION( 0x200000, "onboard.3", 0 ) /* onboard flash */
	ROM_LOAD( "876ea.27l",    0x000000, 0x200000, CRC(950a5267) SHA1(373a7305a090d1e347bfeb62cc2db55cde2a106e) )
	ROM_REGION( 0x200000, "onboard.4", 0 ) /* onboard flash */
	ROM_LOAD( "876ea.31j",    0x000000, 0x200000, CRC(ae497ebc) SHA1(ef131e60726db94f0d9ceab70bce02c0de080ede) )
	ROM_REGION( 0x200000, "onboard.5", 0 ) /* onboard flash */
	ROM_LOAD( "876ea.27j",    0x000000, 0x200000, CRC(9c156b1b) SHA1(bf07d71cc1f7e9e14beb9f9dfb71667ef2b54f8d) )
	ROM_REGION( 0x200000, "onboard.6", 0 ) /* onboard flash */
	ROM_LOAD( "876ea.31h",    0x000000, 0x200000, CRC(368372fb) SHA1(5cc4cb72e182c9e4d0737352e029fd703ba2f516) )
	ROM_REGION( 0x200000, "onboard.7", 0 ) /* onboard flash */
	ROM_LOAD( "876ea.27h",    0x000000, 0x200000, CRC(49175f99) SHA1(0154f6332ed210b6f0af20ba622133cde0994b7f) )

	ROM_REGION( 0x002000, "m48t58", 0 ) /* timekeeper */
	ROM_LOAD( "876aa.22h",    0x000000, 0x002000, CRC(3c17f026) SHA1(8ed33aca99f5d09d5792e136e700e3ac628018e8) )
ROM_END

ROM_START( hypbbc2p )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gx908ja.u1",  0x000000, 0x000084, BAD_DUMP CRC(fb6c0635) SHA1(0d974462a0a244ffb1a651adb316242cde427756) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "908a02", 0, BAD_DUMP SHA1(573194ca9938c30415fc88dcc0c0152dd3024d71) )
ROM_END

ROM_START( hypbbc2pk )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gx908ka.u1",  0x000000, 0x000084, BAD_DUMP CRC(f4f37fe1) SHA1(30f90cdb2d092e4f8d6c14cfd4ca4945e6d352cb) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "908a02", 0, BAD_DUMP SHA1(573194ca9938c30415fc88dcc0c0152dd3024d71) )
ROM_END

ROM_START( konam80a )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gc826aa.u1", 0x000000, 0x000224, BAD_DUMP CRC(9b38b959) SHA1(6b4fca340a9b1c2ae21ad3903c1ac1e39ab08b1a) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "826aaa01", 0, BAD_DUMP SHA1(be5f8b31fd18ba631fe98c2132c56abf20193419) )
ROM_END

ROM_START( konam80j )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gc826ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(e9e861e8) SHA1(45841db0b42d096213d9539a8d076d39391dca6d) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "826jaa01", 0, SHA1(be5f8b31fd18ba631fe98c2132c56abf20193419) )
ROM_END

ROM_START( konam80k )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gc826ka.u1", 0x000000, 0x000224, BAD_DUMP CRC(d41f7e38) SHA1(73e2bb132e23be72e72ea5b0686ccad28e47574a) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "826kaa01", 0, BAD_DUMP SHA1(be5f8b31fd18ba631fe98c2132c56abf20193419) )
ROM_END

ROM_START( konam80s )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gc826ea.u1", 0x000000, 0x000224, BAD_DUMP CRC(6ce4c619) SHA1(d2be08c213c0a74e30b7ebdd93946374cc64457f) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "826eaa01", 0, BAD_DUMP SHA1(be5f8b31fd18ba631fe98c2132c56abf20193419) )
ROM_END

ROM_START( konam80u )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gc826ua.u1", 0x000000, 0x000224, BAD_DUMP CRC(0577379b) SHA1(3988a2a5ef1f1d5981c4767cbed05b73351be903) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "826uaa01", 0, SHA1(be5f8b31fd18ba631fe98c2132c56abf20193419) )
ROM_END

ROM_START( mamboagg )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gqa40jab.u1",  0x000000, 0x001014, BAD_DUMP CRC(fd9e7c1f) SHA1(6dd4790589d48803f58328d099c908f0565b2c01) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gqa40jab.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a40jab02", 0, SHA1(a914842442b2f63465e16f979f06da0b78a5f13e) )
ROM_END

ROM_START( mrtlbeat )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0001014, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "geb47jb.u1",   0x000000, 0x001014, BAD_DUMP CRC(90079ff5) SHA1(8273ee3349dd13207836b0ebf72ad5fa67fef68a) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "geb47jb.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "b47jxb02", 0, SHA1(6bbe8d6169ef692bd8995da564bd5a97b6bf0b31) )
ROM_END

ROM_START( pbballex )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gx802ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(ea8bdda3) SHA1(780034ab08871631ef0e3e9b779ca89e016c26a8) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "802jab02", 0, BAD_DUMP SHA1(bcc2b6c3515e2420eef9fdf8b28115368a428a92) )
ROM_END

ROM_START( pcnfrk3m )
	SYS573_BIOS_A
	SYS573_DIGITAL_ID

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* install security cart eeprom */
	ROM_LOAD( "a23kaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(d71c4b5c) SHA1(3911c5dd933c30e6e44c8cf417bb4c284ecb4b80) )

	ROM_REGION( 0x0001014, "game_eeprom", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gca23ka.u1",   0x000000, 0x001014, BAD_DUMP CRC(f392349c) SHA1(e7eb7979db276de560d5820163a70d97e6c023d8) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* install security cart id */
	ROM_LOAD( "a23kaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "game_id", 0 ) /* game security cart id */
	ROM_LOAD( "gca23ka.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a23kaa02", 0, BAD_DUMP SHA1(5b853cc25eb583ed36d8cd402235b4f5c9ce065a) )
ROM_END

ROM_START( pnchmn )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gq918jaa.u1",  0x000000, 0x000224, BAD_DUMP CRC(e4769787) SHA1(d60c6598c7c58b5cd8f86350ebf7f3f32c1ebe9b) )

	ROM_REGION( 0x200000, "onboard.0", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.31m", 0x000000, 0x200000, CRC(3653b5d7) SHA1(1deb44335b7a38506fb30da40e0ca61b96aea7bb) )
	ROM_REGION( 0x200000, "onboard.1", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.27m", 0x000000, 0x200000, CRC(27d48c97) SHA1(c140d4bdfa869fbcae1133bbfe73a346e6f46cb8) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gq918jaa.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "918xxb02", 0, BAD_DUMP SHA1(8ced8952fff3e70ce0621a491f0973af5a6ccd82) )
ROM_END

ROM_START( pnchmna )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gq918jab.u1",  0x000000, 0x000224, BAD_DUMP CRC(e4769787) SHA1(d60c6598c7c58b5cd8f86350ebf7f3f32c1ebe9b) )

	ROM_REGION( 0x200000, "onboard.0", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.31m", 0x000000, 0x200000, CRC(3653b5d7) SHA1(1deb44335b7a38506fb30da40e0ca61b96aea7bb) )
	ROM_REGION( 0x200000, "onboard.1", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.27m", 0x000000, 0x200000, CRC(27d48c97) SHA1(c140d4bdfa869fbcae1133bbfe73a346e6f46cb8) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gq918jab.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "918jab02", 0, BAD_DUMP SHA1(8e59b07f6f4b67afb1109200da01d177a537d1be) )
ROM_END

ROM_START( pnchmn2 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gqa09ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(e1e4108f) SHA1(0605e2c7a7dcb2f4928350e96d2ffcc2ede4a762) )

	ROM_REGION( 0x200000, "onboard.0", 0 ) /* onboard flash */
	ROM_LOAD( "gqa09ja.31m",  0x000000, 0x200000, CRC(b1043a91) SHA1(b474439c1a7da7855d9b6d2162d4a522f499d6ab) )
	ROM_REGION( 0x200000, "onboard.1", 0 ) /* onboard flash */
	ROM_LOAD( "gqa09ja.27m",  0x000000, 0x200000, CRC(09b1a70b) SHA1(0f3bcad879e05faaf8130133d774a2071031ee74) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gqa09ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a09jaa02", 0, BAD_DUMP SHA1(b085fbe76d5ef87578744b45b874b5f79147e586) )
ROM_END

ROM_START( salarymc )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "install_eeprom", 0 ) /* security cart eeprom */
	ROM_LOAD( "gca18jaa.u1",  0x000000, 0x000084, CRC(c9197f67) SHA1(8e95a89008f756a79295f2cb557c39efca1351e7) )

	ROM_REGION( 0x000008, "install_id", 0 ) /* security cart id */
	ROM_LOAD( "gca18jaa.u6",  0x000000, 0x000008, CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "gca18jaa", 0, SHA1(8adcc8ef76cbfb9f47fec5702b0b200565b5c561) )
ROM_END

GAME( 1997, sys573,   0,        konami573,    konami573, ksys573_state, konami573,  ROT0, "Konami", "System 573 BIOS", GAME_IS_BIOS_ROOT )

GAME( 1997, hndlchmp, sys573,   konami573,    hndlchmp, ksys573_state,  konami573,  ROT0, "Konami", "Handle Champ (GQ710 VER. JAB)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, darkhleg, sys573,   konami573x,   konami573, ksys573_state, konami573,  ROT0, "Konami", "Dark Horse Legend (GX706 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, fbaitbc,  sys573,   k573baitx,    fbaitbc, ksys573_state,   ge765pwbba, ROT0, "Konami", "Fisherman's Bait - A Bass Challenge (GE765 VER. UAB)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, bassangl, fbaitbc,  k573baitx,    fbaitbc, ksys573_state,   ge765pwbba, ROT0, "Konami", "Bass Angler (GE765 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, pbballex, sys573,   konami573x,   konami573, ksys573_state, konami573,  ROT0, "Konami", "Powerful Pro Baseball EX (GX802 VER. JAB)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, konam80s, sys573,   konami573x,   konami573, ksys573_state, konami573,  ROT90, "Konami", "Konami 80's AC Special (GC826 VER. EAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, konam80u, konam80s, konami573x,   konami573, ksys573_state, konami573,  ROT90, "Konami", "Konami 80's AC Special (GC826 VER. UAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, konam80j, konam80s, konami573x,   konami573, ksys573_state, konami573,  ROT90, "Konami", "Konami 80's Gallery (GC826 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, konam80a, konam80s, konami573x,   konami573, ksys573_state, konami573,  ROT90, "Konami", "Konami 80's AC Special (GC826 VER. AAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, konam80k, konam80s, konami573x,   konami573, ksys573_state, konami573,  ROT90, "Konami", "Konami 80's AC Special (GC826 VER. KAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, dstage,   sys573,   konami573x,   ddr, ksys573_state,       ddr,        ROT0, "Konami", "Dancing Stage (GN845 VER. EAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, ddru,     dstage,   konami573x,   ddr, ksys573_state,       ddr,        ROT0, "Konami", "Dance Dance Revolution (GN845 VER. UAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, ddrj,     dstage,   konami573x,   ddr, ksys573_state,       ddr,        ROT0, "Konami", "Dance Dance Revolution - Internet Ranking Ver (GC845 VER. JBA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, ddrja,    dstage,   konami573x,   ddr, ksys573_state,       ddr,        ROT0, "Konami", "Dance Dance Revolution (GC845 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )
GAME( 1998, ddrjb,    dstage,   konami573x,   ddr, ksys573_state,       ddr,        ROT0, "Konami", "Dance Dance Revolution (GC845 VER. JAB)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )
GAME( 1999, ddra,     dstage,   konami573x,   ddr, ksys573_state,       ddr,        ROT0, "Konami", "Dance Dance Revolution (GN845 VER. AAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, fbait2bc, sys573,   k573baitx,    fbaitbc, ksys573_state,   ge765pwbba, ROT0, "Konami", "Fisherman's Bait 2 - A Bass Challenge (GE865 VER. UAB)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, bassang2, fbait2bc, k573baitx,    fbaitbc, ksys573_state,   ge765pwbba, ROT0, "Konami", "Bass Angler 2 (GE865 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, hyperbbc, sys573,   konami573,    hyperbbc, ksys573_state,  konami573,  ROT0, "Konami", "Hyper Bishi Bashi Champ (GQ876 VER. EAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, hyperbbca,hyperbbc, konami573,    hyperbbc, ksys573_state,  konami573,  ROT0, "Konami", "Hyper Bishi Bashi Champ (GQ876 VER. AAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, drmn,     sys573,   konami573x,   drmn, ksys573_state,      drmn,       ROT0, "Konami", "DrumMania (GQ881 VER. JAD)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )
GAME( 1999, gtrfrks,  sys573,   konami573x,   gtrfrks, ksys573_state,   gtrfrks,    ROT0, "Konami", "Guitar Freaks (GQ886 VER. EAC)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, gtrfrksu, gtrfrks,  konami573x,   gtrfrks, ksys573_state,   gtrfrks,    ROT0, "Konami", "Guitar Freaks (GQ886 VER. UAC)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, gtrfrksj, gtrfrks,  konami573x,   gtrfrks, ksys573_state,   gtrfrks,    ROT0, "Konami", "Guitar Freaks (GQ886 VER. JAC)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, gtrfrksa, gtrfrks,  konami573x,   gtrfrks, ksys573_state,   gtrfrks,    ROT0, "Konami", "Guitar Freaks (GQ886 VER. AAC)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, fbaitmc,  sys573,   k573baitx,    fbaitmc, ksys573_state,   ge765pwbba, ROT0, "Konami", "Fisherman's Bait - Marlin Challenge (GX889 VER. EA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, fbaitmcu, fbaitmc,  k573baitx,    fbaitmc, ksys573_state,   ge765pwbba, ROT0, "Konami", "Fisherman's Bait - Marlin Challenge (GX889 VER. UA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, fbaitmcj, fbaitmc,  k573baitx,    fbaitmc, ksys573_state,   ge765pwbba, ROT0, "Konami", "Fisherman's Bait - Marlin Challenge (GX889 VER. JA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, fbaitmca, fbaitmc,  k573baitx,    fbaitmc, ksys573_state,   ge765pwbba, ROT0, "Konami", "Fisherman's Bait - Marlin Challenge (GX889 VER. AA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, ddr2m,    sys573,   konami573x,   ddr, ksys573_state,       ddr,        ROT0, "Konami", "Dance Dance Revolution 2nd Mix (GN895 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, ddr2ml,   ddr2m,    pccard1x,     ddr, ksys573_state,       ddr,        ROT0, "Konami", "Dance Dance Revolution 2nd Mix - Link Ver (GE885 VER. JAB)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, ddr2mla,  ddr2m,    pccard1x,     ddr, ksys573_state,       ddr,        ROT0, "Konami", "Dance Dance Revolution 2nd Mix - Link Ver (GE885 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, ddrbocd,  ddr2m,    pccard1x,     ddr, ksys573_state,       ddr,        ROT0, "Konami", "Dance Dance Revolution Best of Cool Dancers (GE892 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, ddr2mc,   ddr2m,    konami573x,   ddr, ksys573_state,       ddr,        ROT0, "Konami", "Dance Dance Revolution 2nd Mix with beatmaniaIIDX CLUB VERSiON (GE896 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, ddr2mc2,  ddr2m,    konami573x,   ddr, ksys573_state,       ddr,        ROT0, "Konami", "Dance Dance Revolution 2nd Mix with beatmaniaIIDX substream CLUB VERSiON 2 (GE984 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, gtrfrk2m, sys573,   pccard1yi,    gtrfrks, ksys573_state,   gtrfrks,    ROT0, "Konami", "Guitar Freaks 2nd Mix Ver 1.01 (GQ883 VER. JAD)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, dsftkd,   sys573,   konami573yi,  ddr, ksys573_state,       ddr,        ROT0, "Konami", "Dancing Stage featuring TRUE KiSS DESTiNATiON (G*884 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, cr589fw,  sys573,   konami573,    konami573, ksys573_state, konami573,  ROT0, "Konami", "CD-ROM Drive Updater 2.0 (700B04)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, cr589fwa, sys573,   konami573,    konami573, ksys573_state, konami573,  ROT0, "Konami", "CD-ROM Drive Updater (700A04)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 2000, ddr3mk,   sys573,   pccard2dyyi,  ddr, ksys573_state,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution 3rd Mix - Ver.Korea2 (GN887 VER. KBA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.3 */
GAME( 2000, ddr3mka,  ddr3mk,   pccard2dyyi,  ddr, ksys573_state,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution 3rd Mix - Ver.Korea (GN887 VER. KAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.3 */
GAME( 1999, ddr3ma,   ddr3mk,   pccard2dyyi,  ddr, ksys573_state,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution 3rd Mix (GN887 VER. AAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.1 */
GAME( 1999, ddr3mj,   ddr3mk,   pccard2dyyi,  ddr, ksys573_state,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution 3rd Mix (GN887 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.0 */
GAME( 1999, ddrsbm,   sys573,   k573dyi,      ddrsolo, ksys573_state,   ddrsolo,    ROT0, "Konami", "Dance Dance Revolution Solo Bass Mix (GQ894 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )
GAME( 1999, ddrs2k,   sys573,   k573dyyi,     ddrsolo, ksys573_state,   ddrsolo,    ROT0, "Konami", "Dance Dance Revolution Solo 2000 (GC905 VER. AAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.3 */
GAME( 1999, ddrs2kj,  ddrs2k,   k573dyyi,     ddrsolo, ksys573_state,   ddrsolo,    ROT0, "Konami", "Dance Dance Revolution Solo 2000 (GC905 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.2 */
GAME( 1999, hypbbc2p, sys573,   konami573y,   hypbbc2p, ksys573_state,  hyperbbc,   ROT0, "Konami", "Hyper Bishi Bashi Champ - 2 Player (GX908 1999/08/24 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, hypbbc2pk,hypbbc2p, konami573y,   hypbbc2p, ksys573_state,  hyperbbc,   ROT0, "Konami", "Hyper Bishi Bashi Champ - 2 Player (GX908 1999/08/24 VER. KAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, dsfdct,   sys573,   pccard2dyyi,  ddr, ksys573_state,       ddrdigital, ROT0, "Konami", "Dancing Stage featuring Dreams Come True (GC910 VER. JCA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )
GAME( 1999, dsfdcta,  dsfdct,   pccard2yyi,   ddr, ksys573_state,       ddr,        ROT0, "Konami", "Dancing Stage featuring Dreams Come True (GC910 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, drmn2m,   sys573,   k573dxzi,     drmn, ksys573_state,      drmndigital,ROT0, "Konami", "DrumMania 2nd Mix (GE912 VER. JAB)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.5 */
GAME( 1999, drmn2mpu, drmn2m,   k573dxzi,     drmn, ksys573_state,      drmndigital,ROT0, "Konami", "DrumMania 2nd Mix Session Power Up Kit (GE912 VER. JAB)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.5 */
GAME( 2000, dncfrks,  sys573,   k573dzi,      dmx, ksys573_state,       dmx,        ROT0, "Konami", "Dance Freaks (G*874 VER. KAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.6 */
GAME( 2000, dmx,      dncfrks,  k573dzi,      dmx, ksys573_state,       dmx,        ROT0, "Konami", "Dance Maniax (G*874 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.6 */
GAME( 2000, gunmania, sys573,   gunmania,     gunmania, ksys573_state,  gunmania,   ROT0, "Konami", "GunMania (GL906 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )
GAME( 2000, fghtmn,   sys573,   punchmania,   punchmania, ksys573_state,punchmania, ROT0, "Konami", "Fighting Mania (QG918 VER. EAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* motor/artwork/network */
GAME( 2000, fghtmna,  fghtmn,   punchmania,   punchmania, ksys573_state,punchmania, ROT0, "Konami", "Fighting Mania (QG918 VER. AAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* motor/artwork/network */
GAME( 2000, pnchmn,   fghtmn,   punchmania,   punchmania, ksys573_state,punchmania, ROT0, "Konami", "Punch Mania: Hokuto No Ken (GQ918 VER. JAB)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* motor/artwork/network */
GAME( 2000, pnchmna,  fghtmn,   punchmania,   punchmania, ksys573_state,punchmania, ROT0, "Konami", "Punch Mania: Hokuto No Ken (GQ918 VER. JAB ALT CD)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* motor/artwork/network */
GAME( 2000, fghtmnk,  fghtmn,   punchmania,   punchmania, ksys573_state,punchmania, ROT0, "Konami", "Fighting Mania (QG918 VER. KAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* motor/artwork/network */
GAME( 2000, fghtmnu,  fghtmn,   punchmania,   punchmania, ksys573_state,punchmania, ROT0, "Konami", "Fighting Mania (QG918 VER. UAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* motor/artwork/network */
GAME( 2000, dsem,     sys573,   k573dxi,      ddr, ksys573_state,       ddrdigital, ROT0, "Konami", "Dancing Stage Euro Mix (G*936 VER. EAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.7 */
GAME( 2000, gtrfrk3m, sys573,   pccard1dxzi,  gtrfrks, ksys573_state,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 3rd Mix (GE949 VER. JAC)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.4 */
GAME( 2000, gtfrk3ma, gtrfrk3m, pccard1dxzi,  gtrfrks, ksys573_state,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 3rd Mix (GE949 VER. JAB)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.4 */
GAME( 2000, gtfrk3mb, gtrfrk3m, pccard1dzi,   gtrfrks, ksys573_state,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 3rd Mix - security cassette versionup (949JAZ02)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.4 */
GAME( 2000, pnchmn2,  sys573,   punchmania2,  punchmania, ksys573_state,punchmania, ROT0, "Konami", "Punch Mania 2: Hokuto No Ken (GQA09 JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* motor/artwork/network */
GAME( 2000, salarymc, sys573,   konami573yi,  hypbbc2p, ksys573_state,  salarymc,   ROT0, "Konami", "Salary Man Champ (GCA18 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 2000, ddr3mp,   sys573,   pccard2dxzi,  ddr, ksys573_state,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution 3rd Mix Plus (G*A22 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.6 */
GAME( 2000, pcnfrk3m, sys573,   k573dxzi,     drmn, ksys573_state,      drmndigital,ROT0, "Konami", "Percussion Freaks 3rd Mix (G*A23 VER. KAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, drmn3m,   pcnfrk3m, k573dxzi,     drmn, ksys573_state,      drmndigital,ROT0, "Konami", "DrumMania 3rd Mix (G*A23 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, gtrfrk4m, sys573,   pccard1dxzi,  gtrfrks, ksys573_state,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 4th Mix (G*A24 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddr4m,    sys573,   pccard2dxzi,  ddr, ksys573_state,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution 4th Mix (G*A33 VER. AAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddr4mj,   ddr4m,    pccard2dxzi,  ddr, ksys573_state,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution 4th Mix (G*A33 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddr4ms,   sys573,   pccard2dxzi,  ddrsolo, ksys573_state,   ddrsolo,    ROT0, "Konami", "Dance Dance Revolution 4th Mix Solo (G*A33 VER. ABA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddr4msj,  ddr4ms,   pccard2dxzi,  ddrsolo, ksys573_state,   ddrsolo,    ROT0, "Konami", "Dance Dance Revolution 4th Mix Solo (G*A33 VER. JBA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, dsfdr,    sys573,   k573dxzi,     ddr, ksys573_state,       ddrdigital, ROT0, "Konami", "Dancing Stage Featuring Disney's Rave (GCA37JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddrusa,   sys573,   k573dx,       ddr, ksys573_state,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution USA (G*A44 VER. UAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddr4mp,   sys573,   pccard2dxzi,  ddr, ksys573_state,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution 4th Mix Plus (G*A34 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2000, ddr4mps,  sys573,   pccard2dxzi,  ddrsolo, ksys573_state,   ddrsolo,    ROT0, "Konami", "Dance Dance Revolution 4th Mix Plus Solo (G*A34 VER. JBA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2000, dmx2m,    sys573,   k573dzi,      dmx, ksys573_state,       dmx,        ROT0, "Konami", "Dance Maniax 2nd Mix (G*A39 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2000, drmn4m,   sys573,   k573dzi,      drmn, ksys573_state,      drmndigital,ROT0, "Konami", "DrumMania 4th Mix (G*A25 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2001, gtrfrk5m, sys573,   pccard1dzi,   gtrfrks, ksys573_state,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 5th Mix (G*A26 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, ddr5m,    sys573,   pccard2dzi,   ddr, ksys573_state,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution 5th Mix (G*A27 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, dmx2majp, sys573,   konami573zi,  dmx, ksys573_state,       dmx,        ROT0, "Konami", "Dance Maniax 2nd Mix Append J-Paradise (G*A38 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, mamboagg, sys573,   k573dzi,      mamboagg, ksys573_state,  mamboagg,   ROT0, "Konami", "Mambo A Go-Go (GQA40 VER. JAB)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2001, drmn5m,   sys573,   k573dzi,      drmn, ksys573_state,      drmndigital,ROT0, "Konami", "DrumMania 5th Mix (G*B05 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, gtrfrk6m, sys573,   pccard1dzi,   gtrfrks, ksys573_state,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 6th Mix (G*B06 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, drmn6m,   sys573,   k573dzi,      drmn, ksys573_state,      drmndigital,ROT0, "Konami", "DrumMania 6th Mix (G*B16 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2001, gtrfrk7m, sys573,   pccard1dzi,   gtrfrks, ksys573_state,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 7th Mix (G*B17 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2001, ddrmax,   sys573,   pccard2dzi,   ddr, ksys573_state,       ddrdigital, ROT0, "Konami", "DDR Max - Dance Dance Revolution 6th Mix (G*B19 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2002, ddrmax2,  sys573,   pccard2dzi,   ddr, ksys573_state,       ddrdigital, ROT0, "Konami", "DDR Max 2 - Dance Dance Revolution 7th Mix (G*B20 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2002, mrtlbeat, sys573,   pccard2dzi,   ddr, ksys573_state,       ddrdigital, ROT0, "Konami", "Martial Beat (G*B47 VER. JBA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2002, drmn7m,   sys573,   k573dzi,      drmn, ksys573_state,      drmndigital,ROT0, "Konami", "DrumMania 7th Mix power-up ver. (G*C07 VER. JBA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2002, drmn7ma,  drmn7m,   k573dzi,      drmn, ksys573_state,      drmndigital,ROT0, "Konami", "DrumMania 7th Mix (G*C07 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2002, gtrfrk8m, sys573,   pccard1dzi,   gtrfrks, ksys573_state,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 8th Mix power-up ver. (G*C08 VER. JBA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2002, gtrfrk8ma,gtrfrk8m, pccard1dzi,   gtrfrks, ksys573_state,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 8th Mix (G*C08 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2002, dsem2,    sys573,   pccard2dzi,   ddr, ksys573_state,       ddrdigital, ROT0, "Konami", "Dancing Stage Euro Mix 2 (G*C23 VER. EAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2002, ddrextrm, sys573,   pccard2dzi,   ddr, ksys573_state,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution Extreme (G*C36 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2003, drmn8m,   sys573,   k573dzi,      drmn, ksys573_state,      drmndigital,ROT0, "Konami", "DrumMania 8th Mix (G*C07 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2003, gtrfrk9m, sys573,   pccard1dzi,   gtrfrks, ksys573_state,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 9th Mix (G*C39 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2003, drmn9m,   sys573,   k573dzi,      drmn, ksys573_state,      drmndigital,ROT0, "Konami", "DrumMania 9th Mix (G*D09 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2003, gtfrk10m, sys573,   pccard1dzi,   gtrfrks, ksys573_state,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 10th Mix (G*D10 VER. JAB)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2003, gtfrk10ma,gtfrk10m, pccard1dzi,   gtrfrks, ksys573_state,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 10th Mix (G*D10 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2003, gtfrk10mb,gtfrk10m, pccard1dzi,   gtrfrks, ksys573_state,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 10th Mix eAmusement (G*D10 VER. JBA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2004, gtfrk11m, sys573,   pccard1dzi,   gtrfrks, ksys573_state,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 11th Mix (G*D39 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2004, drmn10m,  sys573,   k573dzi,      drmn, ksys573_state,      drmndigital,ROT0, "Konami", "DrumMania 10th Mix (G*D40 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.95 */
