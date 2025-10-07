// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, ElSemi, Andrew Gardner
/* Hyper NeoGeo 64

Driver by David Haywood, ElSemi, Andrew Gardner and Angelo Salese


Notes:
  * The main board is identical for all revisions and all "versions" of the hardware.
    It contains the main MIPS CPU and a secondary communications KL5C80.

  * The I/O board is what changes between hardware "versions".  It has a Toshiba MCU with a protected
    internal ROM which has been dumped. This MCU controls (at least) the inputs per game and communicates
    with the main board through dualport RAM.

  * I believe that this secondary board is used as a protection device.
    The "board type" code comes from it in dualport RAM, and each game reads its inputs differently through dualport.
    It's capable of changing the input ports dynamically.
    It has nothing to do with the network. The network connector and all network hardware is located on the main board.

  * The Toshiba CPU datasheet is here : http://kr.ic-on-line.cn/IOL/viewpdf/TMP87CH40N_1029113.htm

  * From the Roads Edge manual : "The Network Check screen will be displayed for about 40 seconds whether
                                  the cabinet is connected for communication competition or not.  After this,
                                  the game screen will then appear.  At the same time the Network Check screen
                                  is displayed, the steering wheel will automatically straighten itself out."
                                 "During the Network Check, absolutely do not touch or try to use the steering wheel,
                                  pedal, shift lever, and switches.  This will cause the cabinet to malfunction."

  * The Japanese text on the Roads Edge network screen says : "waiting to connect network... please wait without touching machine"

------------------------------------------------------------------------------
Hyper NeoGeo 64, SNK 1997-1999
Hardware info by Guru

This is a 3D system comprising....
- One large PCB with many custom surface-mounted components on both sides.
- One separate interface PCB with JAMMA and/or several other connectors, I/O microcontroller, dualport RAM, video DAC and sound amp circuitry.
- One game cartridge inside a metal case.

There are only 7 games on this system. In some cases the game name changes depending on the BIOS region.
The games in order of release are....
001 Roads Edge / Round Trip RV
002 Samurai Shodown 64 / Samurai Spirits ～侍魂～ / 覇王傳說 64 (Paewang Jeonseol 64)
003 Xtreme Rally / Off Beat Racer!
004 Beast Busters: Second Nightmare
005 Samurai Shodown 64: Warriors Rage / Samurai Spirits 2: Asura Zanmaden
006 Fatal Fury: Wild Ambition / Garou Densetsu: Wild Ambition
007 Buriki One: World Grapple Tournament '99 in Tokyo

When powering a Hyper Neogeo 64 board, all four +5v pins on the JAMMA connector should have +5v going to them and ALL of the grounds should be
connected, not just a couple. This is due to the main board requiring several amps to operate. Additionally, the power connector on the I/O
board must be connected to the main board 5-pin connector otherwise the main board will not power up. The I/O board power connector is not
optional. It is recommended to use a 15-Amp PSU to power the game.

A correctly powered board will display a blue screen with white text as the game boots up.
However the two Samurai Showdown games show a solid blue screen for a few seconds then a black screen with white text....
'NOW I/O INITIALIZING SEQUENCE 1'
and after a few seconds...
'NOW I/O INITIALIZING SEQUENCE 2'

The main board has one version. Any main board will play any game. The only thing that changes between main boards is the BIOS, either
Japan, US or Export regions and the BIOS is easily changed by simply reflashing the ROM. There is another BIOS just for Korea which is said
to only play the Samurai Showdown games. If you have this version and want to play other games, simply reflash the ROM to one of the more
useful regions. For the three main BIOS dumps (excluding Korea), all of them will run any game with in-game differences like a changed title
screen and some text shown in either English or a different language. The language can be changed between Japanese and English in the test
mode but it doesn't change all texts. To have a fully English version of a game use a USA or World region BIOS. All chips on the main board
are identical regardless of the region. The communication flashROM has been dumped from driving, fighting and gun main boards and was
identical.
Several BIOS ROMs were dumped from fighting, driving and the gun boards and matched existing archives.
There are only 4 known BIOS regions.... Japan, USA, Export and Korea.


Main Board PCB Layout (Top)
---------------------

LVS-MAC SNK 1997.06.02
|--------------------------------------------------------------|
|CON4          CON9                           CON1         IC2 |
|                                               DPRAM1     LED2|
|   ASIC1        ASIC3       U4   CPU1               U28   LED1|
|                                                       ROM1   |
|                                                              |
|                  OSC2 OSC1      ASIC5  U24        FPGA1      |
|   FSRAM1                                                     |
|   FSRAM2                                                 CON8|
|   FSRAM3                        ASIC10            OSC4       |
|                                                              |
|                                                 CPU3  IC4    |
|   PSRAM1  ASIC7   ASIC8         DSP1            SRAM5     IC3|
|   PSRAM2                           OSC3               FROM1  |
|                                                              |
|                                                              |
|              CON10                           CON6            |
|--------------------------------------------------------------|
Note All ICs shown.

No.  PCB Label  IC Markings               IC Package   Use
----------------------------------------------------------------------------------------------------------------------------------------
01   ASIC1      NEO64-REN                 QFP304       3D Render Engine
02   ASIC3      NEO64-GTE                 QFP208       2D Transform Engine
03   ASIC5      NEO64-SYS                 QFP208       System Bus Controller
04   ASIC7      NEO64-BGC                 QFP240       Background GFX Controller
05   ASIC8      NEO64-SPR                 QFP208       Sprite Generator
06   ASIC10     NEO64-SCC                 QFP208       Scroll Character Controller
07   CPU1       NEC D30200GD-100 VR4300   QFP120       NEC VR4300 CPU; Clock Input 33.333MHz on pin 16. DivMode0=1, DivMode1=1; MasterClock=33.333MHz; PClock (Internal)=99.999MHz (i.e. 100MHz), TClock=33.333MHz
08   CPU3       KL5C80A12CFP              QFP80        Communication CPU (Z80-based) with on-board peripherals and 512b RAM. Clock input 10.000MHz on pin 24 [40/4]
09   DPRAM1*    IDT7133 LA35J             PLCC68       IDT7133 Dual-Port RAM
10   DSP1       L7A1045 L6028 DSP-A       QFP120       L7A1045 DSP-A
11   FPGA1      ALTERA EPF10K10QC208-4    QFP208       Altera FPGA, programmed as a network controller
12   FROM1      MBM29F400B-12             TSOP48       Communication program, same on all versions of the main board. Dumped as FROM1.BIN
13   FSRAM1     TC55V1664AJ-15            SOJ44        Static RAM
14   FSRAM2     TC55V1664AJ-15            SOJ44        Static RAM
15   FSRAM3     TC55V1664AJ-15            SOJ44        Static RAM
16   IC4        SMC COM20020-5ILJ         PLCC28       5Mbps ARCNet Controller
17   OSC1       M33.333 KDS 7M            7050         33.333MHz Oscillator, connected to the VR4300 CPU and NEO64-GTE
18   OSC2       M50.113 KDS 7L            7050         50.113MHz Oscillator, connected to NEO64-BGC
19   OSC3       A33.868 KDS 7M            7050         33.868MHz Oscillator, connected directly to the DSP-A
20   OSC4       A40.000 KDS 7L            7050         40MHz Oscillator, connected to the Altera FPGA
21   PSRAM1     TC551001BFL-70L           SOP32        Static RAM
22   PSRAM2     TC551001BFL-70L           SOP32        Static RAM
23   ROM1       ALTERA EPC1PC8            DIP8         FPGA Configuration IC. Dumped as ROM1.BIN (130817 bytes)
24   SRAM5      TC55257DFL-85L            SOP28        Static RAM
25   U4         LVX245                    TSSOP20      Logic
26   IC3        75ALS181                  SOIC14       EIA 422/EIA 485 Differential Driver/Receiver
27   U28        LCX16245                  TSSOP48      Logic
28   U24        LCX157                    TSSOP16      Logic
29   IC2        PQ3DF53                   TO-3P        3.3V 5A Regulator
30   LED1                                              Lights up for 1/2 second at power-on then goes off
31   LED2                                              Lights up when the power-up communication test passes and stays on
32   CON9                                              \
33   CON10                                             / Game cart plugs in here
34   CON4                                              Power from I/O board plugs in here
35   CON1                                              Unknown connector (accessible from the top for an option board)
36   CON6                                              Unknown connector (accessible from the top for an option board)
37   CON8                                              15 pin JST NH connector for network data (connects to 75ALS181)

   * The IDT 7133 / 7143 lacks interrupts and just acts as 0x1000 bytes (2x 0x800 16-bit words) of RAM
     IDT 7133 - 32K (2K X 16 Bit) MASTER Dual-Port SRAM
     IDT 7143 - 32K (2K X 16 Bit) SLAVE Dual-Port SRAM


Main Board PCB Layout (Bottom)
---------------------

LVS-MAC SNK 1997.06.02
|--------------------------------------------------------------|
|                                                              |
|   U5  U6  U7  U8  U9  U10  U11 U12                           |
|                                                              |
|   PSRAM4  ASIC9            CON2   SRAM4     CPU2  Y1         |
|   PSRAM3             SRAM1  CON3  SRAM3                      |
|                      SRAM2                                   |
|                                 U26         U23              |
|   FSRAM6                        DRAM3  U21  U22              |
|   FSRAM5                                       CON5          |
|   FSRAM4                        DRAM1                        |
|               U31    BROM1      DRAM2  U20  U27              |
|                                        U19                   |
|                                        U17                   |
|   ASIC2              ASIC4      ASIC6  U16                   |
|                                        U18                   |
|                 U3 U2 U1          IC1                        |
|--------------------------------------------------------------|
Note All ICs shown.

No.  PCB Label  IC Markings               IC Package   Use
----------------------------------------------------------------------------------------------------------------------------------------
01   ASIC2      NEO64-REN                 QFP304       3D Render Engine
02   ASIC4      NEO64-TRI2                QFP208       Triangle Engine
03   ASIC6      NEO64-CVR                 QFP120       Possible RAM Controller (wired to the CPU)
04   ASIC9      NEO64-CAL                 QFP208       I/O <-> Main board System Arbitrator (connected to I/O PCB via CON2/CON3)
05   BROM1      MBM29F400B-12             TSOP48       BIOS, dumped as BROM1.BIN
06   CPU2       NEC D70236AGJ-16 V53A     QFP120       NEC V53A CPU (code compatible with V20/V30). Clock input 32MHz. Internal Clock=Input Clock/2 (i.e. 16MHz)
07   DRAM1      HY51V18164BJC-60          SOJ42        Dynamic RAM
08   DRAM2      HY51V18164BJC-60          SOJ42        Dynamic RAM
09   DRAM3      HY51V18164BJC-60          SOJ42        Dynamic RAM
10   FSRAM4     TC55V1664AJ-15            SOJ44        Static RAM
11   FSRAM5     TC55V1664AJ-15            SOJ44        Static RAM
12   FSRAM6     TC55V1664AJ-15            SOJ44        Static RAM
13   PSRAM3     TC551001BFL-70L           SOP32        Static RAM
14   PSRAM4     TC551001BFL-70L           SOP32        Static RAM
15   SRAM1      TC55257DFL-85L            SOP28        Static RAM
16   SRAM2      TC55257DFL-85L            SOP28        Static RAM
17   SRAM3      TC551001BFL-70L           SOP32        Static RAM
18   SRAM4      TC551001BFL-70L           SOP32        Static RAM
19   Y1         D320L7                    XTAL         32MHz Crystal tied directly to the V53A
20   U5         LCX16374                  TSSOP48      Logic
21   U6         LCX16374                  TSSOP48      Logic
22   U7         LCX16374                  TSSOP48      Logic
23   U8         LCX16374                  TSSOP48      Logic
24   U9         LCX16374                  TSSOP48      Logic
25   U10        LCX16374                  TSSOP48      Logic
26   U11        LCX16374                  TSSOP48      Logic
27   U12        LCX16374                  TSSOP48      Logic
28   U26        S32X245                   TSSOP40      Logic
29   CON2                                              \
30   CON3                                              / I/O board plugs in here
31   IC1        PST573J                   MMP-3A       System Reset, trigger 2.7V
32   U16        LCX16245                  TSSOP48      Logic
33   U17        LCX16245                  TSSOP48      Logic
34   U18        LCX16245                  TSSOP48      Logic
35   U19        LCX16245                  TSSOP48      Logic
36   U20        LCX16245                  TSSOP48      Logic
37   U21        LCX16245                  TSSOP48      Logic
38   U22        LCX16245                  TSSOP48      Logic
39   U23        LCX16245                  TSSOP48      Logic
40   U27        LCX16245                  TSSOP48      Logic
41   U31        S32X245                   TSSOP40      Logic
42   U1         S32X245                   TSSOP40      Logic
43   U2         S32X245                   TSSOP40      Logic
44   U3         S32X245                   TSSOP40      Logic
45   CON5       -                                      Unknown connector (accessible from the bottom for an option board)

It seems that although there are extra connectors for option boards, none were actually made or used on any game.
It is possible the option connectors were for factory testing.


I/O Boards
----------

A special I/O board is required to boot the system which plugs into two custom connectors on the bottom of the main board.
There are 3 types:
LVS-IOJ runs the driving games and the Samurai Showdown games.
LVS-JAM runs all of the fighting games.
LVS-IGX runs the gun game (Beast Busters: Second Nightmare).
Note using an incompatible game and I/O board combination will result in an error at bootup 'MACHINE CODE ERROR'
For the driving games the network is also checked.
As a work-around, to satisfy the network check on driving games simply join CON8 pins 1, 3, 5, 6, 7 & 8 then the
network check will pass and the game will start.

All of the I/O boards look completely different. Two boards are JAMMA. The third type doesn't have an edge connector and instead uses
several JST connectors. The main control chip on this board is a Toshiba TMP87PH40AN (or TMP87CH40AN) microcontroller with an internal 32kb
program. The MCU from any I/O board can be swapped across different I/O boards and works fine. Several chips have been documented and were
all marked the same so there is only one revision of the control chip and it is identical on all versions of the I/O boards. The
microcontroller has an external serial EEPROM connected to it to configure the control mode 'ID Code' which it puts into the dualport RAM on
the I/O board and the game software checks it. I (Guru) confirmed that swapping the EEPROM from the LVS-JAM board onto a LVS-IOJ board
allows the other fighting games to boot and run. An EEPROM from a driving I/O board was re-programmed with the LVS-JAM EEPROM dump and put
onto the LVS-JAM I/O board and it worked fine. So to summarise, the driving I/O board can be converted to run all of the fighting games
simply by re-programming the EEPROM using the LVS-JAM dump in MAME.

The first I/O board 'LVS-IOJ' dated 6-6-1997 has a JAMMA edge connector and runs the driving games and the two Samurai Showdown games. Note
the first game on this system was Roads Edge, so while this has been known as the 1st rev fighting I/O board, it is actually the driving I/O
board that just happens to play the two Samurai Showdown games. The two custom connectors on the I/O board that plug into the main board do
not have power running to them. There is a 6 pin JST VH connector on the back of the I/O board directly below the JAMMA power pins which
must be plugged into the main board. This cable powers the main board so without it the game will not boot up. There is a small volume pot
accessible on the front and two JST XH connectors for connection of extra buttons and other controls for the driving games. The JST
connectors join with wires to some JST VL connectors in a bracket mounted to the bottom of the main board metal frame for MVS cabinet
wiring hookups. The volume pot does nothing when the JAMMA connector is used. The output of the volume pot is connected to one of the JST
connectors and only affects the volume when set to the MVS 3-Channel mode. Located just above the DPRAM1 chip, there are resistor pads
labelled 'MONO' and '3ch' with a small 2.2k-ohm surface-mounted resistor (marked 222). The default factory position is '3ch'. When the
resistor is moved to the mono position, amplified audio comes out through the JAMMA connector. However note the volume pot does not work, it
is only for the 3-channel MVS audio. To change the JAMMA volume use the volume pot in the cabinet.

LVS-IOJ SNK 1997.6.6
|---------------------------------------------|
|     VOL   CON2  J A M M A                   |
|                                     OPAMP1  |
| IC2                                         |
|                                        IC1  |
| IC3               U2                        |
|                           IC8               |
| IC4            IOCTR1          IC7  OPAMP2  |
|                    8MHz                  IC6|
|                      U17  U19    CON3       |
|SW1   DPRAM1   BT1              CON1  IC5    |
|---------------------------------------------|

No.  PCB Label  IC Markings               IC Package   Use
----------------------------------------------------------------------------------------------------------------------------------------
01   DPRAM1     IDT 71321 LA55PF *        QFP64        IDT 71321 High Speed 2k x8-bit Dual-Port Static Ram with Interrupts
02   IC5        MC44200FT                 QFP44        Triple 8-bit Video DAC
03   IOCTR1     TOSHIBA TMP87CH40N-4828   SDIP64       I/O Microcontroller with 32kb internal ROM. Clock Input 8.000MHz. Marked 'SNK-IOJ1.00A'. Dumped as TMP87PH40AN.BIN
04   U17        EPSON RTC62423            SOP24        Real-Time Clock. Clock input 32.768kHz
05   U19        TC55257DFL-85L            SOP28        Static RAM
06   IC1        NEC C1891ACY              DIP20-400mil NEC uPC1891 Matrix Surround Sound Processor
07   BT1        2430                                   3V Coin Battery with Solder Tags
08   SW1                                               4 position DIPSW; Appears to be unused
09   U2         BR9020F                   SOP8         2k-bit (128 words x 16-bit) Serial EEPROM. Dumped as LVS-IOJ-BR9020F.U2
10   OPAMP1     AD8044                    SOIC14       Quad 150MHz Rail to Rail Amplifier
11   IC3        TA8201                                 Audio Power AMP
12   IC4        TA8201                                 Audio Power Amp
13   OPAMP3     uPC844                    SOIC14       Quad Operational Amplifier
14   OPAMP2     uPC844                    SOIC14       Quad Operational Amplifier
15   IC2        TA8201                                 16-bit Stereo D/A converter
16   IC5        BU9480F                   SOP8         16-bit Stereo D/A converter
17   IC6        78D05                                  5V regulator
18   IC7        78D05                                  5V regulator
19   IC8        BU9480F                   SOP8         16-bit Stereo D/A converter
20   CON1                                              \
21   CON3                                              / Plugs into main board here
22   CON2                                              6 pin JST VH connector for power. Tied to the main board CON4
                                                       Pinout and wiring for CON2 to CON4:
                                                       CON2 -> CON4 (Main Board)
                                                       1    ->  1 (pink wire)
                                                       2    ->  2 (orange wire)
                                                       3    ->  3 (white wire)
                                                       4 no connection
                                                       5 -> tied to 4 pin connector on front bracket (red wire +5V)
                                                       6 -> tied to 4 pin connector on front bracket (red wire +5V)
                                                               CON4 pin 4 tied to 4 pin connector on front bracket (black wire GND)
                                                               CON4 pin 5 tied to 4 pin connector on front bracket (black wire GND)
Notes:
       1. The game cart plugs into the main PCB on the TOP side into CONN9 & CONN10
       2. If the game cart is not plugged in, the hardware shows nothing on screen or shows a black or blue screen filled with vertical lines.
       3. The IOCTR I/O MCU runs at 8 MHz. 87CH and 87PH types exist but they are functionally equivalent.
       4. Syncs measured at the JAMMA connector using Samurai Spirits 2 and the 1st I/O board. During gameplay and attract the syncs
          definitely change. For example the Hyper Neogeo logo that shows at the start causes my WG D9200 monitor to emit a noise when the sync
          changes and in-game the sync meter can't lock onto the frequency because it is changing. Meaning measuring the syncs is not all that
          helpful, but anyway....
          HSync - 16.28kHz
          VSync - ~60Hz


The second I/O board 'LVS-IGX' dated 10-11-1997 does not have a JAMMA edge connector. It has the same 6-pin JST VH power connector on the
bottom of the board. There are several JST XA connectors for hooking up the controls. There is no audio power AMP or volume pot on the
board. This board only runs the gun game 'Beast Busters: Second Nightmare'.

LVS-IGX SNK 1997.11.10
|---------------------------------------------|
| CON9  CON2 CON11   CON10     CON4  CON12 IC1|
|                          IC3 IC4 OPAMP1     |
|          U3      CON13                      |
|                                      OPAMP3 |
|      CON7   CON8  CON5    CON6  CON14       |
|                              OPAMP2         |
|    IOCTR1                        IC5 IC6 IC7|
|       8MHz                       IC2        |
|                      U21  U19    CON3       |
|SW1   DPRAM1   BT1              CON1         |
|---------------------------------------------|

No.  PCB Label  IC Markings               IC Package   Use
----------------------------------------------------------------------------------------------------------------------------------------
01   DPRAM1     IDT 71321 LA55PF *        QFP64        IDT 71321 High Speed 2k x8-bit Dual-Port Static Ram with Interrupts
02   IC2        MC44200FT                 QFP44        Triple 8-bit Video DAC
03   IOCTR1     TOSHIBA TMP87CH40N-4828   SDIP64       I/O Microcontroller with 32kb internal ROM. Clock Input 8.000MHz. Marked 'SNK-IOJ1.00A'. Dumped as TMP87PH40AN.BIN
04   U19        EPSON RTC62423            SOP24        Real-Time Clock. Clock input 32.768kHz
05   U21        W24258S-70LE              SOP28        Static RAM
06   IC1        NEC C1891ACY              DIP20-400mil NEC uPC1891 Matrix Surround Sound Processor
07   BT1        2430                                   3V Coin Battery with Solder Tags
08   SW1                                               4 position DIPSW; Appears to be unused
09   OPAMP2     AD8044                    SOIC14       Quad 150MHz Rail to Rail Amplifier
10   OPAMP3     uPC844                    SOIC14       Quad Operational Amplifier
11   OPAMP1     uPC844                    SOIC14       Quad Operational Amplifier
12   IC5        BU9480F                   SOP8         16-bit Stereo D/A converter
13   IC6        BU9480F                   SOP8         16-bit Stereo D/A converter
14   IC7        BU9480F                   SOP8         16-bit Stereo D/A converter
15   IC3        78D05                                  5V regulator
16   IC4        78D05                                  5V regulator
17   U3         BR9020F                   SOP8         2k-bit (128 words x 16-bit) Serial EEPROM. Dumped as LVS-IGX-BR9020F.U3
18   CON11                                             6 pin JST XA connector
19   CON12                                             12 pin JST XA connector
20   CON4                                              4 pin JST VH connector
21   CON10                                             13 pin JST XA connector
22   CON13                                             20 pin JST NH connector
23   CON7                                              15 pin JST XA connector
24   CON8                                              4 pin JST XA connector
25   CON5                                              20 pin JST XA connector
26   CON6                                              11 pin JST XA connector
27   CON14                                             5 pin JST XA connector
28   CON1                                              \
29   CON3                                              / Plugs into main board here
30   CON2                                              6 pin JST VH connector for power. Tied to the main board CON4
                                                       Pinout and wiring for CON2 to CON4:
                                                       CON2 -> CON4 (Main Board)
                                                       1    ->  1 (pink wire)
                                                       2    ->  2 (orange wire)
                                                       3    ->  3 (white wire)
                                                       4 no connection
                                                       5 -> tied to 4 pin connector on front bracket (red wire +5V)
                                                       6 -> tied to 4 pin connector on front bracket (red wire +5V)
                                                               CON4 pin 4 tied to 4 pin connector on front bracket (black wire GND)
                                                               CON4 pin 5 tied to 4 pin connector on front bracket (black wire GND)
Notes:
       1. The game cart plugs into the main PCB on the TOP side into CONN9 & CONN10
       2. If the game cart is not plugged in, the hardware shows nothing on screen or shows a black or blue screen filled with vertical lines.
       3. The IOCTR I/O MCU runs at 8 MHz. 87CH and 87PH types exist but they are functionally equivalent.


The third and final I/O board 'LVS-JAM' dated 1-20-1999 has a JAMMA edge connector. There is a 5 pin JST VH connector on the back of the I/O
board directly below the JAMMA power pins which must be plugged into the main board. Note it has 5 pins, not 6 pins like the first I/O
board. This board has several JST XA connectors on the top and front of the board. The JST XA connectors are used when the board is set to
MVS mode using a switch labelled MVS/JAMMA. In JAMMA mode the JST connectors are not used. Those JST connectors join with wires to a bracket
mounted to the bottom of the main board metal frame which contains several JST VL connectors. There is a thumb-wheel type potentiometer to
adjust the volume. There are two toggle switches. One selects between mono/audio or MVS 2-channel audio. The other one selects between JAMMA
controls and MVS controls. This board can run all of the fighting games.

LVS-JAM SNK 1999.1.20
|---------------------------------------------|
| VOL CON5 CON3    J A M M A            CON9  |
|                                             |
| IC3                                         |
|                     SW1       U3            |
|     SW3                                     |
| IC4                       IOCTR1    OPAMP1  |
| IC6  OPAMP3               BACKUP            |
|     IC5                   BKRAM1            |
| IC2 CON10                     CON4          |
|     SW2   BT1  DPRAM1       CON2   IC1      |
|---------------------------------------------|

No.  PCB Label  IC Markings               IC Package   Use
----------------------------------------------------------------------------------------------------------------------------------------
01   DPRAM1     IDT 71321 LA55PF *        QFP64        IDT 71321 High Speed 2k x8-bit Dual-Port Static Ram with Interrupts
02   IC1        MC44200FT                 QFP44        Triple 8-bit Video DAC
03   IOCTR1     TOSHIBA TMP87CH40N-4828   SDIP64       I/O Microcontroller with 32kb internal ROM. Clock Input 8.000MHz. Marked 'SNK-IOJ1.00A'. Dumped as TMP87PH40AN.BIN
04   BACKUP     EPSON RTC62423            SOP24        Real-Time Clock. Clock input 32.768kHz
05   BKRAM1     W24258S-70LE              SOP28        Static RAM
06   IC6        NEC C1891ACY              DIP20-400mil NEC uPC1891 Matrix Surround Sound Processor
07   BT1        2430                                   3V Coin Battery with Solder Tags
08   SW1                                               2 position DIPSW for controls; OFF = JAMMA, ON = MVS
09   SW2                                               4 position DIPSW; Appears to be unused
10   SW3                                               2 position DIPSW for audio output. OFF = MONO/JAMMA, ON = 2CH MVS
11   OPAMP1     AD8044                    SOIC14       Quad 150MHz Rail to Rail Amplifier
12   IC3        TA7252                                 Audio Power AMP
13   IC4        TA7252                                 Audio Power Amp
14   OPAMP3     uPC844                    SOIC14       Quad Operational Amplifier
15   OPAMP2     uPC844                    SOIC14       Quad Operational Amplifier
16   IC2        BU9480F                   SOP8         16-bit Stereo D/A converter
17   IC5        BU9480F                   SOP8         16-bit Stereo D/A converter
18   U3         BR9020F                   SOP8         2k-bit (128 words x 16-bit) Serial EEPROM. Dumped as LVS-JAM-BR9020F.U3
19   CON4                                              \
20   CON2                                              / Plugs into main board here
21   CON10                                             8 pin JST XA connector
22   CON6                                              3 pin JST XA connector
23   CON7                                              15 pin JST XA connector
24   CON1                                              12 pin JST XA connector
25   CON8                                              9 pin JST XA connector
26   CON5                                              4 pin JST XA connector
27   CON9                                              2x 10-pin Hirose HIF3BA-20PA-2.54DS(71) IDC flat cable connector
28   CON3                                              5 pin JST VH connector for power. Tied to the main board CON4
                                                       Pinout and wiring for CON3 to CON4:
                                                       CON3 -> CON4 (Main Board)
                                                       1    ->  1 (red wire)
                                                       2    ->  2 (red wire)
                                                       3    ->  3 (white wire)
                                                       4    ->  4 (black wire)
                                                       5    ->  5 (black wire)
Notes:
       1. The game cart plugs into the main PCB on the TOP side into CONN9 & CONN10
       2. If the game cart is not plugged in, the hardware shows nothing on screen or shows a black or blue screen filled with vertical lines.
       3. The IOCTR I/O MCU runs at 8 MHz. TMP87CH and TMP87PH types exist but they are functionally equivalent.

       *"IDT71321 is function-compatible (but not pin-compatible) with MB8421" ( src\devices\machine\mb8421.cpp )
        The INTL & INTR pins are not connected to anything on the PCB.
        There aren't any accesses to 7ff / 7fe outside of the RAM testing, commands are put at byte 0 by the MIPS


Hyper Neogeo game cartridges
----------------------------

The game carts contain a large amount of surface mounted ROMs on both sides of the PCB.
On a DG1 cart all the roms are 32Mbits, for the DG2 cart the SC and SP roms are 64Mbit.
The DG1 cart can accept a maximum of 96 ROMs
The DG2 cart can accept a maximum of 84 ROMs

The actual carts are only about 1/4 to 1/3rd populated.
Some of the IC locations between DG1 and DG2 are different also. See the source code below
for the exact number of ROMs used per game and ROM placements.

Games that use the LVS-DG1 cart: Road's Edge / Round Trip RV
                                 Xtreme Rally / Off Beat Racer!
                                 Beast Busters: Second Nightmare
                                 Samurai Shodown 64 / Samurai Spirits / Paewang Jeonseol 64

Games that use the LVS-DG2 cart: Fatal Fury: Wild Ambition / Garou Densetsu: Wild Ambition
                                 Buriki One: World Grapple Tournament '99 in Tokyo
                                 Samurai Shodown 64: Warriors Rage / Samurai Spirits 2: Asura Zanmaden

There might be a Rev.A program for Buriki One and Round Trip RV, we have Rev. B dumps.

pr = program
sc = scroll characters
sd = sound
tx = textures
sp = sprites
vt = vertex (3D data)

Top
---
LVS-DG1
(C) SNK 1997
1997.5.20
|----------------------------------------------------------------------------|
|                                                                            |
|                                                                            |
|                                                                            |
|                                                                            |
| TX01A.5    TX01A.13        VT03A.19   VT02A.18   VT01A.17   PR01B.81       |
|                                                                            |
|                                                                            |
|                                                                            |
| TX02A.6    TX02A.14        VT06A.22   VT05A.21   VT04A.20   PR03A.83       |
|                                                                            |
|                                                                            |
|                                                                            |
| TX03A.7    TX03A.15        VT09A.25   VT08A.24   VT07A.23   PR05A.85       |
|                                                                            |
|                                                                            |
|                                                                            |
| TX04A.8    TX04A.16        VT12A.28   VT11A.27   VT10A.26   PR07A.87       |
|                                                                            |
|                                                                            |
|                                                                            |
|  PQ3TZ53                   PR15A.95   PR13A.93   PR11A.91   PR09A.89       |
|                                                                            |
|                                                                            |
|                                                                            |
| SC09A.49  SC10A.50   SP09A.61   SP10A.62   SP11A.63   SP12A.64    SD02A.78 |
|                                                                            |
|                                                                            |
|                                                                            |
| SC05A.45  SC06A.46   SP05A.57   SP06A.58   SP07A.59   SP08A.60             |
|                                                                            |
|                                                                            |
|                                                                            |
| SC01A.41  SC02A.42   SP01A.53   SP02A.54   SP03A.55   SP04A.56    SD01A.77 |
|                                                                            |
|                                                                            |
|                                                                            |
|                                                                            |
|----------------------------------------------------------------------------|
Notes:
      PQ3TZ53 - 3.3V 0.5A Voltage Regulator

Bottom
------
LVS-DG1
|----------------------------------------------------------------------------|
|                         |----------------------|                           |
|                         |----------------------|                           |
|                                                                            |
|                                                                            |
| SC03A.43  SC04A.44   SP13A.65   SP14A.66   SP15A.67   SP16A.68    SD03A.79 |
|                                                                            |
|                                                                            |
|                                                                            |
| SC07A.47  SC08A.48   SP17A.69   SP18A.70   SP19A.71   SP20A.72             |
|                                                                            |
|                                                                            |
|                                                                            |
| SC11A.51  SC12A.52   SP21A.73   SP22A.74   SP23A.75   SP24A.76    SD04A.80 |
|                                                                            |
|               LCX138 LCX138 LCX138                                         |
|                                                                            |
|                            PR16A.96   PR14A.94   PR12A.92   PR10A.90       |
|                                                                            |
| LCX138 LCX138 LCX138 LCX138 LCX138 LCX138                                  |
|                                                                            |
| TX04A.4    TX04A.12        VT24A.40   VT23A.39   VT22A.38   PR08A.88       |
|                                                                            |
|                                                                            |
|                                                                            |
| TX03A.3    TX03A.11        VT21A.37   VT20A.36   VT19A.35   PR06A.86       |
|                                                                            |
|                                                                            |
|                                                                            |
| TX02A.2    TX02A.10        VT18A.34   VT17A.33   VT16A.32   PR04A.84       |
|                                                                            |
|                                                                            |
|                                                                            |
| TX01A.1    TX01A.9         VT15A.31   VT14A.30   VT13A.29   PR02B.82       |
|                                                                            |
|                                                                            |
|                         |----------------------|                           |
|                         |----------------------|                           |
|----------------------------------------------------------------------------|
Notes:
      LCX138 - 3.3V Logic. Powered by the PQ3TZ53 on the other side of the PCB.

Top
---
LVS-DG2
(C) SNK 1998
1998.6.5
|----------------------------------------------------------------------------|
|                                                                            |
|                                                                            |
|                                                                            |
|                                                                            |
| TX01A.5    TX01A.13        VT03A.19   VT02A.18   VT01A.17   PR01B.81       |
|                                                                            |
|                                                                            |
|                                                                            |
| TX02A.6    TX02A.14        VT06A.22   VT05A.21   VT04A.20   PR03A.83       |
|                                                                            |
|                                                                            |
|                                                                            |
| TX03A.7    TX03A.15        VT09A.25   VT08A.24   VT07A.23   PR05A.85       |
|                                                                            |
|                                                                            |
|                                                                            |
| TX04A.8    TX04A.16        VT12A.28   VT11A.27   VT10A.26   PR07A.87       |
|                                                                            |
|                                                                            |
|                                                                            |
|  PQ3TZ53                   PR15A.95   PR13A.93   PR11A.91   PR09A.89       |
|                                                                            |
|                                                                            |
|                                                                            |
| SC05A.98  SC06A.100  SP09A.107  SP10A.111  SP11A.115  SP12A.119   SD02A.78 |
|                                                                            |
|                                                                            |
|                                                                            |
|                                                                            |
|                                                                            |
|                                                                            |
|                                                                            |
| SC01A.97  SC02A.99   SP01A.105  SP02A.109  SP03A.113  SP04A.117   SD01A.77 |
|                                                                            |
|                                                                            |
|                                                                            |
|                                                                            |
|----------------------------------------------------------------------------|
Notes:
      PQ3TZ53 - 3.3V 0.5A Voltage Regulator

Bottom
------
LVS-DG2
|----------------------------------------------------------------------------|
|                         |----------------------|                           |
|                         |----------------------|                           |
|                                                                            |
|                                                                            |
| SC03A.101 SC04A.103  SP13A.108  SP14A.112  SP15A.116  SP16A.120   SD03A.79 |
|                                                                            |
|                                                                            |
|                                                                            |
|                                                                            |
|                                                                            |
|                                                                            |
|                                                                            |
| SC07A.102  SC08A.104  SP05A.106  SP06A.110  SP07A.114  SP08A.118  SD04A.80 |
|                                                                            |
|               LCX138 LCX138 LCX138                                         |
|                                                                            |
|                            PR16A.96   PR14A.94   PR12A.92   PR10A.90       |
|                                                                            |
| LCX138 LCX138 LCX138 LCX138 LCX138 LCX138                                  |
|                                                                            |
| TX04A.4    TX04A.12        VT24A.40   VT23A.39   VT22A.38   PR08A.88       |
|                                                                            |
|                                                                            |
|                                                                            |
| TX03A.3    TX03A.11        VT21A.37   VT20A.36   VT19A.35   PR06A.86       |
|                                                                            |
|                                                                            |
|                                                                            |
| TX02A.2    TX02A.10        VT18A.34   VT17A.33   VT16A.32   PR04A.84       |
|                                                                            |
|                                                                            |
|                                                                            |
| TX01A.1    TX01A.9         VT15A.31   VT14A.30   VT13A.29   PR02B.82       |
|                                                                            |
|                                                                            |
|                         |----------------------|                           |
|                         |----------------------|                           |
|----------------------------------------------------------------------------|
 Notes:
      LCX138 - 3.3V Logic. Powered by the PQ3TZ53 on the other side of the PCB.

      Not all ROM positions are populated, check the source for exact ROM usage.
      ROMs are mirrored. i.e. TX/PR/SP/SC etc ROMs line up on both sides of the PCB.
      There are 4 copies of each TX ROM on the PCB.

------------------------------------------------------------------------------
*/


#include "emu.h"
#include "hng64.h"

#include "cpu/mips/mips3.h"
#include "cpu/z80/kl5c80a12.h"
#include "cpu/z80/z80.h"
#include "machine/nvram.h"

#define LOG_COMRW           (1U << 1)
#define LOG_SNDCOM_UNKNWN   (1U << 2)
#define LOG_DMA             (1U << 3)
#define LOG_VREGS           (1U << 4)
#define LOG_SYSREGS         (1U << 5)

#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"


// this is driven by a KL5C80A12CFP which is basically a super-charged Z80
// MMU handling has been moved to the core; the rest is not implemented yet.

// I believe this CPU is used for network only (the racing games) and not related to the I/O MCU

/*
0x6010: tests RAM at [3]8000

*/

void hng64_state::comm_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom().region("comm", 0);
	map(0x20000, 0x3ffff).bankr(m_com_bank);
	map(0x40000, 0x47fff).mirror(0xb8000).ram();
}

void hng64_state::comm_io_map(address_map &map)
{
	map.global_mask(0xff);
	/* Reserved for the KL5C80 internal hardware */
//  map(0x00, 0x07).rw(FUNC(hng64_state::comm_mmu_r), FUNC(hng64_state::comm_mmu_w));
//  map(0x08, 0x1f).noprw();              /* Reserved */
//  map(0x20, 0x25).rw(hng64_state::));   /* Timer/Counter B */           /* hng64 writes here */
//  map(0x27, 0x27).noprw();              /* Reserved */
//  map(0x28, 0x2b).rw(hng64_state::)); /* Timer/Counter A */           /* hng64 writes here */
//  map(0x2c, 0x2f).rw(hng64_state::)); /* Parallel port A */
//  map(0x30, 0x33).rw(hng64_state::)); /* Parallel port B */
//  map(0x34, 0x37).rw(hng64_state::)); /* Interrupt controller */      /* hng64 writes here */
//  map(0x38, 0x39).rw(hng64_state::)); /* Serial port */               /* hng64 writes here */
//  map(0x3a, 0x3b).rw(hng64_state::)); /* System control register */   /* hng64 writes here */
//  map(0x3c, 0x3f).noprw();              /* Reserved */

	/* General IO */
//  map(0x40, 0x47).rw("ulanc", FUNC(com20020_device::read), FUNC(com20020_device::write));
	map(0x50, 0x57).rw(FUNC(hng64_state::com_share_r), FUNC(hng64_state::com_share_w));
	map(0x72, 0x72).w(FUNC(hng64_state::com_bank_w));
//  map(0x73, 0x73).w(hng64_state::));            /* dunno yet */
}


void hng64_state::reset_net()
{
//  m_comm->pulse_input_line(INPUT_LINE_NMI, attotime::zero); // reset the CPU and let 'er rip
//  m_comm->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);     // hold on there pardner...
}

void hng64_state::hng64_network(machine_config &config)
{
	KL5C80A12(config, m_comm, HNG64_MASTER_CLOCK / 4);        /* KL5C80A12CFP - binary compatible with Z80. */
	m_comm->set_addrmap(AS_PROGRAM, &hng64_state::comm_map);
	m_comm->set_addrmap(AS_IO, &hng64_state::comm_io_map);
}


u32 hng64_state::com_r(offs_t offset, u32 mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_COMRW, "com read  (PC=%08x): %08x %08x = %08x\n", m_maincpu->pc(), (offset*4)+0xc0000000, mem_mask, m_idt7133_dpram[offset]);
	return m_idt7133_dpram[offset];
}

void hng64_state::com_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_COMRW, "com write (PC=%08x): %08x %08x = %08x\n", m_maincpu->pc(), (offset*4)+0xc0000000, mem_mask, data);
	COMBINE_DATA(&m_idt7133_dpram[offset]);
}

/* TODO: fully understand this */
void hng64_state::com_share_mips_w(offs_t offset, u8 data)
{
	m_com_shared[offset ^ 3] = data;
}

u8 hng64_state::com_share_mips_r(offs_t offset)
{
	return m_com_shared[offset];
}

void hng64_state::com_share_w(offs_t offset, u8 data)
{
	m_com_shared[offset] = data;
}

u8 hng64_state::com_share_r(offs_t offset)
{
	if (offset == 4)
		return m_com_shared[offset] | 1; // some busy flag?

	return m_com_shared[offset];
}

void hng64_state::com_bank_w(u8 data)
{
	m_com_bank->set_entry(data & 0x03);
}


u32 hng64_state::rtc_r(offs_t offset, u32 mem_mask)
{
	if (offset & 1)
	{
		// RTC is mapped to 1 byte (4-bits used) in every 8 bytes so we can't even install this with a umask
		const int rtc_addr = offset >> 1;

		// bit 4 disables "system log reader" (the device is 4-bit? so this bit is not from the device?)
		if ((rtc_addr & 0xf) == 0xd)
			return m_rtc->read((rtc_addr) & 0xf) | 0x10;

		return m_rtc->read((rtc_addr) & 0xf);
	}
	else
	{
		// shouldn't happen unless something else is mapped here too
		if (!machine().side_effects_disabled())
			LOG("%s: unhandled rtc_r (%04x) (%08x)\n", machine().describe_context(), offset*4, mem_mask);
		return 0xffffffff;
	}
}

/* preliminary dma code, dma is used to copy program code -> ram */
void hng64_state::do_dma(address_space &space)
{
	// check if this determines how long the crosshatch is visible for, we might need to put it on a timer.

	LOGMASKED(LOG_DMA, "Performing DMA Start %08x Len %08x Dst %08x\n", m_dma_start, m_dma_len, m_dma_dst);
	while (m_dma_len >= 0)
	{
		space.write_dword(m_dma_dst, space.read_dword(m_dma_start));
		m_dma_start += 4;
		m_dma_dst += 4;
		m_dma_len--;
	}
}

u32 hng64_state::dmac_r(offs_t offset, u32 mem_mask)
{
	// DMAC seems to be mapped as 4 bytes in every 8
	if ((offset * 4) == 0x54)
		return 0x00000000; //dma status, 0x800

	if (!machine().side_effects_disabled())
		LOG("%s: unhandled dmac_r (%04x) (%08x)\n", machine().describe_context(), offset*4, mem_mask);

	return 0xffffffff;
}

void hng64_state::dmac_w(address_space &space, offs_t offset, u32 data, u32 mem_mask)
{
	// DMAC seems to be mapped as 4 bytes in every 8
	switch (offset * 4)
	{
	case 0x04: COMBINE_DATA(&m_dma_start); break;
	case 0x14: COMBINE_DATA(&m_dma_dst); break;
	case 0x24: COMBINE_DATA(&m_dma_len);
		do_dma(space);
		break;

	// these are touched during startup when setting up the DMA, maybe mode selection?
	case 0x34: // (0x0075)
	case 0x44: // (0x0000)

	// written immediately after length, maybe one of these is the actual trigger?, 4c is explicitly set to 0 after all operations are complete
	case 0x4c: // (0x0101 - trigger) (0x0000 - after DMA)
	case 0x5c: // (0x0008 - trigger?) after 0x4c
	default:
		LOG("%s: unhandled dmac_w (%04x) %08x (%08x)\n", machine().describe_context(), offset*4, data, mem_mask);
		break;
	}
}

void hng64_state::rtc_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (offset & 1)
	{
		// RTC is mapped to 1 byte (4-bits used) in every 8 bytes so we can't even install this with a umask
		m_rtc->write((offset >> 1) & 0xf, data);
	}
	else
	{
		// shouldn't happen unless something else is mapped here too
		LOG("%s: unhandled rtc_w (%04x) %08x (%08x)\n", machine().describe_context(), offset*4, data, mem_mask);
	}
}

void hng64_state::mips_to_iomcu_irq_w(offs_t offset, u32 data, u32 mem_mask)
{
	// guess, written after a write to 0x00 in dpram, which is where the command goes, and the IRQ onthe MCU reads the command
	LOG("%s: HNG64 writing to SYSTEM Registers %08x (%08x) (IO MCU IRQ TRIGGER?)\n", machine().describe_context(), data, mem_mask);
	if (mem_mask & 0xffff0000) m_tempio_irqon_timer->adjust(attotime::zero);
}

u32 hng64_state::irqc_r(offs_t offset, u32 mem_mask)
{
	if ((offset * 4) == 0x04)
	{
		if (!machine().side_effects_disabled())
			LOG("%s: irq level READ %04x\n", machine().describe_context(), m_irq_level);
		return m_irq_level;
	}
	else
	{
		if (!machine().side_effects_disabled())
			LOG("%s: unhandled irqc_r (%04x) (%08x)\n", machine().describe_context(), offset*4, mem_mask);
	}

	return 0xffffffff;
}

void hng64_state::irqc_w(offs_t offset, u32 data, u32 mem_mask)
{
	switch (offset * 4)
	{
		//case 0x0c: // global irq mask? (probably not)
	case 0x1c:
		// IRQ ack
		m_irq_pending &= ~(data&mem_mask);
		set_irq(0x0000);
		break;

	default:
		LOG("%s: unhandled irqc_w (%04x) %08x (%08x)\n", machine().describe_context(), offset * 4, data, mem_mask);
		break;
	}
}

/*
  These 'sysregs' seem to be multiple sets of the same thing
  (based on xrally)

  the 0x1084 addresses appear to be related to the IO MCU, but neither sending commands to the MCU, not controlling lines directly
  0x20 is written to 0x1084 in the MIPS IRQ handlers for the IO MCU (both 0x11 and 0x17 irq levels)

  the 0x1074 address seems to be the same thing but for the network CPU
  0x20 is written to 0x1074 in the MIPS IRQ handlers that seem to be associated with communication (levels 0x09, 0x0a, 0x0b, 0x0c)


  -----
  the following notes are taken from the old 'fake IO' function, in reality it turned out that these 'commands' were not needed
  with the real IO MCU hooked up, although we still use the 0x0c one as a hack in order to provide the 'm_no_machine_error_code' value
  in order to bypass a startup check, in reality it looks like that should be written by the MCU after reading it via serial.

  ---- OUTDATED NOTES ----

  I'm not really convinced these are commands in this sense based on code analysis, probably just a non-standard way of controlling the lines

    command table:
    0x0b = ? mode input polling (sams64, bbust2, sams64_2 & roadedge) (*)
    0x0c = cut down connections, treats the dualport to be normal RAM
    0x11 = ? mode input polling (fatfurwa, xrally, buriki) (*)
    0x20 = asks for MCU machine code (probably not, this is also written in the function after the TLCS870 requests an interrupt on the MIPS)

    (*) 0x11 is followed by 0x0b if the latter is used, JVS-esque indirect/direct mode?
  ----
*/

u32 hng64_state::sysregs_r(offs_t offset, u32 mem_mask)
{
	//LOG("%s: sysregs_r (%04x) (%08x)\n", machine().describe_context(), offset * 4, mem_mask);

	switch (offset * 4)
	{
		case 0x001c: return 0x00000000; // 0x00000040 must not be set or games won't boot
		//case 0x106c:
		//case 0x107c:
		case 0x1084:
			if (!machine().side_effects_disabled())
				LOG("%s: HNG64 reading MCU status port (%08x)\n", machine().describe_context(), mem_mask);
			return 0x00000002; //MCU->MIPS latch port
	}

	return m_sysregs[offset];
}

void hng64_state::raster_irq_pos_w(u32 data)
{
	m_raster_irq_pos[m_irq_pos_half] = data;

	m_irq_pos_half ^= 1;
}

void hng64_state::sysregs_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_sysregs[offset]);

	if (((offset * 4) & 0xff00) == 0x1100)
		LOGMASKED(LOG_SYSREGS, "%s: HNG64 writing to SYSTEM Registers 0x%08x == 0x%08x. (PC=%08x)\n", machine().describe_context(), offset*4, m_sysregs[offset], m_maincpu->pc());

	const int scanline = m_screen->vpos();

	switch (offset * 4)
	{
		/*
		case 0x0014:
		    break;
		case 0x001c:
		    break;
		*/
		case 0x100c:
			raster_irq_pos_w(data);
			break;

		/*
		case 0x1014:
		    break;
		case 0x101c:
		    break;
		case 0x106c: // also reads from this one when writing 100c regs
		    break;
		case 0x1074:
		    break;
		*/
		case 0x1084: //MIPS->MCU latch port
			m_mcu_en = (data & 0xff); //command-based, i.e. doesn't control halt line and such?
			LOG("%s: HNG64 writing to MCU control port %08x (%08x)\n", machine().describe_context(), data, mem_mask);
			break;
		/*
		case 0x108c:
		    break;
		*/
		default:
			LOGMASKED(LOG_SYSREGS, "%s: HNG64 writing to SYSTEM Registers %08x %08x (%08x) on scanline %d\n", machine().describe_context(), offset * 4, data, mem_mask, scanline);
	}
}


/**************************************
* MIPS side Dual Port RAM hookup for MCU
**************************************/

u8 hng64_state::dualport_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		LOG("%s: dualport R %04x\n", machine().describe_context(), offset);

	// hack, this should just be put in ram at 0x600 by the MCU.
	if (!(m_mcu_en == 0x0c))
	{
		switch (offset)
		{
		case 0x600: return m_no_machine_error_code;
		}
	}

	return m_dt71321_dpram->right_r(offset);
}

/*
Beast Busters 2 outputs (all at offset == 0x1c):
0x00000001 start #1
0x00000002 start #2
0x00000004 start #3
0x00001000 gun #1
0x00002000 gun #2
0x00004000 gun #3
*/


/*
    MIPS clearly writes commands for the TLCS870 MCU at 00 here
    first command it writes after the startup checks is 0x0a, it should also trigger an EXTINT0 on the TLCS870
    around that time, as the EXTINT0 reads the command.

    call at CBB0 in the MCU is to read the command from shared RAM
    value is used in the jump table at CBC5
    command 0x0a points at ccbd
    which starts with a call to copy 0x40 bytes of data from 0x200 in shared RAM to the internal RAM of the MCU
    the MIPS (at least in Fatal Fury) uploads this data to shared RAM prior to the call.

    need to work out what triggers the interrupt, as a write to 0 wouldn't as the Dual Port RAM interrupts
    are on addresses 0x7fe and 0x7ff (we're using an address near the system regs, based on code analysis
    it seems correct, see mips_to_iomcu_irq_w )
*/

void hng64_state::dualport_w(offs_t offset, u8 data)
{
	m_dt71321_dpram->right_w(offset, data);
	LOG("%s: dualport WRITE %04x %02x\n", machine().describe_context(), offset, data);
}

/************************************************************************************************************/

/* The following is guesswork, needs confirmation with a test on the real board. */
// every sprite is 0x20 bytes
//
void hng64_state::sprite_clear_even_w(offs_t offset, u32 data, u32 mem_mask)
{
	auto &mspace = m_maincpu->space(AS_PROGRAM);
	const u32 spr_offs = (offset) * 0x40;
	// for one sprite
	if (ACCESSING_BITS_16_31)
	{
		mspace.write_dword(0x20000000+0x00+0x00+spr_offs, 0x00000000);
		mspace.write_dword(0x20000000+0x08+0x00+spr_offs, 0x00000000);
		mspace.write_dword(0x20000000+0x10+0x00+spr_offs, 0x00000000);
		mspace.write_dword(0x20000000+0x18+0x00+spr_offs, 0x00000000);
	}
	// for another sprite
	if (ACCESSING_BITS_8_15)
	{
		mspace.write_dword(0x20000000+0x00+0x20+spr_offs, 0x00000000);
		mspace.write_dword(0x20000000+0x08+0x20+spr_offs, 0x00000000);
		mspace.write_dword(0x20000000+0x10+0x20+spr_offs, 0x00000000);
		mspace.write_dword(0x20000000+0x18+0x20+spr_offs, 0x00000000);
	}
}

void hng64_state::sprite_clear_odd_w(offs_t offset, u32 data, u32 mem_mask)
{
	auto &mspace = m_maincpu->space(AS_PROGRAM);
	const u32 spr_offs = (offset) * 0x40;
	// for one sprite
	if (ACCESSING_BITS_16_31)
	{
		mspace.write_dword(0x20000000+0x04+0x00+spr_offs, 0x00000000);
	//  mspace.write_dword(0x20000000+0x0c+0x00+spr_offs, 0x00000000); // erases part of the slash palette in the sams64 2nd intro when we don't want it to! (2nd slash)
		mspace.write_dword(0x20000000+0x14+0x00+spr_offs, 0x00000000);
		mspace.write_dword(0x20000000+0x1c+0x00+spr_offs, 0x00000000);
	}
	// for another sprite
	if (ACCESSING_BITS_0_15)
	{
		mspace.write_dword(0x20000000+0x04+0x20+spr_offs, 0x00000000);
	//  mspace.write_dword(0x20000000+0x0c+0x20+spr_offs, 0x00000000); // erases part of the slash palette in the sams64 2nd intro when we don't want it to! (1st slash)
		mspace.write_dword(0x20000000+0x14+0x20+spr_offs, 0x00000000);
		mspace.write_dword(0x20000000+0x1c+0x20+spr_offs, 0x00000000);
	}
}

void hng64_state::vregs_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_VREGS, "vregs_w %02x, %08x %08x\n", offset * 4, data, mem_mask);

	u32 newval = m_videoregs[offset];
	COMBINE_DATA(&newval);

	if (newval != m_videoregs[offset])
	{
		const int vpos = m_screen->vpos();

		if (vpos > 0)
			m_screen->update_partial(vpos - 1);

		m_videoregs[offset] = newval;
	}
}

u16 hng64_state::main_sound_comms_r(offs_t offset)
{
	switch (offset * 2)
	{
		case 0x00:
			return main_latch[0];
		case 0x02:
			return main_latch[1];
		case 0x04:
			return sound_latch[0];
		case 0x06:
			return sound_latch[1];
		default:
			if (!machine().side_effects_disabled())
				LOGMASKED(LOG_SNDCOM_UNKNWN, "%08x R\n",offset*2);
			break;
	}
	return 0;
}

void hng64_state::main_sound_comms_w(offs_t offset, u16 data, u16 mem_mask)
{
	switch (offset * 2)
	{
		case 0x00:
			COMBINE_DATA(&main_latch[0]);
			break;
		case 0x02:
			COMBINE_DATA(&main_latch[1]);
			break;
		case 0x08:
			if (data & 1)
			{
				m_audiocpu->set_input_line(5, ASSERT_LINE);
				// let the V53 catch up
				m_maincpu->spin_until_time(attotime::from_usec(5));
			}
			break;
		default:
			LOGMASKED(LOG_SNDCOM_UNKNWN, "%02x %04x\n",offset*2,data);
			break;
	}
}


void hng64_state::main_map(address_map &map)
{
	// main RAM / ROM
	map(0x00000000, 0x00ffffff).ram().share(m_mainram);
	map(0x04000000, 0x05ffffff).nopw().rom().region("gameprg", 0);

	// Misc Peripherals
	map(0x1f700000, 0x1f7010ff).rw(FUNC(hng64_state::sysregs_r), FUNC(hng64_state::sysregs_w)).share(m_sysregs); // various things

	map(0x1f701100, 0x1f70111f).rw(FUNC(hng64_state::irqc_r), FUNC(hng64_state::irqc_w));
	map(0x1f701200, 0x1f70127f).rw(FUNC(hng64_state::dmac_r), FUNC(hng64_state::dmac_w));
	// 1f702004 used (rarely writes 01 or a random looking value as part of init sequences)
	map(0x1f702100, 0x1f70217f).rw(FUNC(hng64_state::rtc_r), FUNC(hng64_state::rtc_w));
	map(0x1f7021c4, 0x1f7021c7).w(FUNC(hng64_state::mips_to_iomcu_irq_w));

	// SRAM.  Coin data, Player Statistics, etc.
	map(0x1f800000, 0x1f803fff).ram().share("nvram");

	// Dualport RAM (shared with IO MCU)
	map(0x1f808000, 0x1f8087ff).rw(FUNC(hng64_state::dualport_r), FUNC(hng64_state::dualport_w)).umask32(0xffffffff);

	// BIOS ROM
	map(0x1fc00000, 0x1fc7ffff).nopw().rom().region("bios", 0);

	// Sprites
	map(0x20000000, 0x2000bfff).ram().share(m_spriteram);
	map(0x2000d800, 0x2000e3ff).w(FUNC(hng64_state::sprite_clear_even_w));
	map(0x2000e400, 0x2000efff).w(FUNC(hng64_state::sprite_clear_odd_w));
	map(0x20010000, 0x20010013).ram().share(m_spriteregs);

	// Backgrounds
	map(0x20100000, 0x2017ffff).ram().w(FUNC(hng64_state::videoram_w)).share(m_videoram);    // Tilemap
	map(0x20190000, 0x20190037).ram().w(FUNC(hng64_state::vregs_w)).share(m_videoregs);

	// Mixing
	map(0x20200000, 0x20203fff).ram().w(FUNC(hng64_state::pal_w)).share(m_paletteram);
	map(0x20208000, 0x2020805f).w(FUNC(hng64_state::tcram_w)).share(m_tcram);   // Transition Control
	map(0x20208000, 0x2020805f).r(FUNC(hng64_state::tcram_r));

	// 3D display list control
	map(0x20300000, 0x203001ff).w(FUNC(hng64_state::dl_w)); // 3d Display List
	map(0x20300200, 0x20300203).w(FUNC(hng64_state::dl_upload_w));  // 3d Display List Upload
	map(0x20300210, 0x20300213).w(FUNC(hng64_state::dl_unk_w)); // once, on startup
	map(0x20300214, 0x20300217).w(FUNC(hng64_state::dl_control_w));
	map(0x20300218, 0x2030021b).r(FUNC(hng64_state::dl_vreg_r));

	// 3D framebuffer
	map(0x30000000, 0x30000003).rw(FUNC(hng64_state::fbcontrol_r), FUNC(hng64_state::fbcontrol_w)).umask32(0xffffffff);
	map(0x30000004, 0x30000007).w(FUNC(hng64_state::fbscale_w)).share(m_fbscale);
	map(0x30000008, 0x3000000b).w(FUNC(hng64_state::fbscroll_w)).share(m_fbscroll);
	map(0x3000000c, 0x3000000f).w(FUNC(hng64_state::fbunkbyte_w)).share(m_fbunk);
	// as the below registers are used at render time, not mixing time, it's possible the above scroll/scale registers are too
	map(0x30000010, 0x3000002f).rw(FUNC(hng64_state::texture_wrapsize_table_r), FUNC(hng64_state::texture_wrapsize_table_w)).umask32(0xffffffff);

	map(0x30100000, 0x3015ffff).rw(FUNC(hng64_state::fbram1_r), FUNC(hng64_state::fbram1_w)).share(m_fbram1);  // 3D Display Buffer A
	map(0x30200000, 0x3025ffff).rw(FUNC(hng64_state::fbram2_r), FUNC(hng64_state::fbram2_w)).share(m_fbram2);  // 3D Display Buffer B

	// Sound
	map(0x60000000, 0x601fffff).rw(FUNC(hng64_state::soundram2_r), FUNC(hng64_state::soundram2_w)); // actually seems unmapped, see note in audio/hng64.c
	map(0x60200000, 0x603fffff).rw(FUNC(hng64_state::soundram_r), FUNC(hng64_state::soundram_w));   // program + data for V53A gets uploaded here

	// These are sound ports of some sort
	map(0x68000000, 0x6800000f).rw(FUNC(hng64_state::main_sound_comms_r), FUNC(hng64_state::main_sound_comms_w));
	map(0x6f000000, 0x6f000003).w(FUNC(hng64_state::soundcpu_enable_w));

	// Dualport RAM (shared with Communications CPU)
	map(0xc0000000, 0xc0000fff).rw(FUNC(hng64_state::com_r), FUNC(hng64_state::com_w)).share("com_ram");
	map(0xc0001000, 0xc0001007).ram().share(m_comhack);//.rw(FUNC(hng64_state::com_share_mips_r), FUNC(hng64_state::com_share_mips_w));
}


static INPUT_PORTS_START( hng64 ) // base port, for debugging
	PORT_START("VBLANK")
	PORT_BIT( 0xffffffff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "IN0" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x01, "IN3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN4")
	PORT_DIPNAME( 0x01, 0x01, "IN4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN5")
	PORT_DIPNAME( 0x01, 0x01, "IN5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN6")
	PORT_DIPNAME( 0x01, 0x01, "IN6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN7")
	PORT_DIPNAME( 0x01, 0x01, "IN7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("AN0")
	PORT_START("AN1")
	PORT_START("AN2")
	PORT_START("AN3")
	PORT_START("AN4")
	PORT_START("AN5")
	PORT_START("AN6")
	PORT_START("AN7")
INPUT_PORTS_END


static INPUT_PORTS_START( hng64_fight )
	PORT_INCLUDE( hng64 )

	PORT_MODIFY("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_MODIFY("IN5") // why is this shifted, is it a bug in the TLCS870 emulation or intentional?
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_MODIFY("IN6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) // Service
	PORT_SERVICE_NO_TOGGLE(0x02, IP_ACTIVE_LOW)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )
INPUT_PORTS_END


static INPUT_PORTS_START( hng64_drive )
	PORT_INCLUDE( hng64 )

	PORT_MODIFY("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN5")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("BGM 1")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("BGM 2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("BGM 3")

	PORT_MODIFY("IN6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("BGM 4")
	PORT_BIT( 0x06, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("View 1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("View 2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Shift Down")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Shift Up")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) // Service
	PORT_SERVICE_NO_TOGGLE(0x02, IP_ACTIVE_LOW)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("AN0")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(60) PORT_PLAYER(1) PORT_NAME("Handle")

	PORT_MODIFY("AN1")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(60) PORT_PLAYER(1) PORT_NAME("Accelerator")

	PORT_MODIFY("AN2")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(60) PORT_PLAYER(1) PORT_NAME("Brake")
INPUT_PORTS_END


static INPUT_PORTS_START( hng64_shoot )
	PORT_INCLUDE( hng64 )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) //trigger
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) //pump
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) //bomb
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) //trigger
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) //pump
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) //bomb
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3) //trigger
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3) //pump
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3) //bomb
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_SERVICE_NO_TOGGLE(0x80, IP_ACTIVE_LOW)

	PORT_MODIFY("IN3") // Debug Port? - there are inputs to pause game, bring up a test menu, move the camera around etc.
	PORT_DIPNAME( 0x01, 0x01, "DEBUG" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("IN4") // usual inputs are disconnected
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN5") // usual inputs are disconnected
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN6") // usual inputs are disconnected
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN7") // usual inputs are disconnected
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("AN0")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(7) PORT_REVERSE PORT_PLAYER(1)

	PORT_MODIFY("AN1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(7) PORT_REVERSE PORT_PLAYER(1)

	PORT_MODIFY("AN2")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(7) PORT_REVERSE PORT_PLAYER(2)

	PORT_MODIFY("AN3")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(7) PORT_REVERSE PORT_PLAYER(2)

	PORT_MODIFY("AN4")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(7) PORT_REVERSE PORT_PLAYER(3)

	PORT_MODIFY("AN5")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(7) PORT_REVERSE PORT_PLAYER(3)
INPUT_PORTS_END


static const gfx_layout hng64_8x8x4_tilelayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 24, 28, 8, 12, 16, 20, 0, 4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

static const gfx_layout hng64_8x8x8_tilelayout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 24,     8,     16,     0,
		256+24, 256+8, 256+16, 256+0 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	16*32
};

static const gfx_layout hng64_16x16x4_tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 24,     28,     8,     12,     16,     20,     0,     4,
		256+24, 256+28, 256+8, 256+12, 256+16, 256+20, 256+0, 256+4 },
	{ 0*32,  1*32,  2*32,  3*32,  4*32,  5*32,  6*32,  7*32,
		16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32 },
	32*32
};

static const gfx_layout hng64_16x16x8_tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 24,      8,      16,      0,
		256+24,  256+8,  256+16,  256+0,
		1024+24, 1024+8, 1024+16, 1024+0,
		1280+24, 1280+8, 1280+16, 1280+0, },
	{ 0*32,  1*32,  2*32,  3*32,  4*32,  5*32,  6*32,  7*32,
		16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32 },
	64*32
};

static const gfx_layout hng64_16x16x4_spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 56, 60, 24, 28, 48, 52, 16, 20, 40, 44, 8, 12, 32, 36, 0, 4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64, 8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	16*64
};

static const gfx_layout hng64_16x16x8_spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 56,      24,      48,      16,      40,      8,      32,      0,
		1024+56, 1024+24, 1024+48, 1024+16, 1024+40, 1024+8, 1024+32, 1024+0 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64, 8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	32*64
};

static const u32 texlayout_xoffset[1024] = { STEP1024(0,8) };
static const u32 texlayout_yoffset[1024] = { STEP1024(0,8192) };

template <u32... Values>
static auto const &texlayout_xoffset_4(std::integer_sequence<u32, Values...>)
{
	static constexpr u32 const s_values[sizeof...(Values)] = { ((Values * 4) ^ 4)... };
	return s_values;
}

template <u32... Values>
static auto const &texlayout_yoffset_4(std::integer_sequence<u32, Values...>)
{
	static constexpr u32 const s_values[sizeof...(Values)] = { (Values * 4096)... };
	return s_values;
}

static const gfx_layout hng64_1024x1024x8_texlayout =
{
	1024, 1024,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	1024*1024*8,
	texlayout_xoffset,
	texlayout_yoffset
};

// it appears that 4bpp tiles can only be in the top 1024 part of this as indexing is the same as 8bpp?
static const gfx_layout hng64_1024x1024x4_texlayout =
{
	1024, 2048,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	1024*2048*4,
	texlayout_xoffset_4(std::make_integer_sequence<u32, 1024>()),
	texlayout_yoffset_4(std::make_integer_sequence<u32, 2048>())
};


static GFXDECODE_START( gfx_hng64 )
	/* tilemap tiles */
	GFXDECODE_ENTRY( "scrtile", 0, hng64_8x8x4_tilelayout,  0x0, 0x100 )
	GFXDECODE_ENTRY( "scrtile", 0, hng64_8x8x8_tilelayout,  0x0, 0x10 )
	GFXDECODE_ENTRY( "scrtile", 0, hng64_16x16x4_tilelayout,0x0, 0x100 )
	GFXDECODE_ENTRY( "scrtile", 0, hng64_16x16x8_tilelayout,0x0, 0x10 )

	/* sprite tiles */
	GFXDECODE_ENTRY( "sprtile", 0, hng64_16x16x4_spritelayout, 0x0, 0x100 )
	GFXDECODE_ENTRY( "sprtile", 0, hng64_16x16x8_spritelayout, 0x0, 0x10 )

	/* texture pages (not used by rendering code) */
	GFXDECODE_ENTRY( "textures0", 0, hng64_1024x1024x4_texlayout, 0x0, 0x100 )
	GFXDECODE_ENTRY( "textures0", 0, hng64_1024x1024x8_texlayout, 0x0, 0x10 )

GFXDECODE_END

static void hng64_reorder(u8* gfxregion, size_t gfxregionsize)
{
	// by default 2 4bpp tiles are stored in each 8bpp tile, this makes decoding in MAME harder than it needs to be
	// reorder them
	const u8 tilesize = 4*8; // 4 bytes per line, 8 lines

	std::vector<u8> buffer(gfxregionsize);

	for (int i = 0; i < gfxregionsize/2; i += tilesize)
	{
		memcpy(&buffer[i*2+tilesize], gfxregion+i,                   tilesize);
		memcpy(&buffer[i*2],          gfxregion+i+(gfxregionsize/2), tilesize);
	}

	memcpy(gfxregion, &buffer[0], gfxregionsize);
}

void hng64_state::init_reorder_gfx()
{
	hng64_reorder(memregion("scrtile")->base(), memregion("scrtile")->bytes());
}

void hng64_state::init_hng64()
{
	init_reorder_gfx();
}

void hng64_state::init_hng64_fght()
{
	m_no_machine_error_code = 0x01;
	init_hng64();
}

void hng64_state::init_ss64()
{
	init_hng64_fght();
	m_samsho64_3d_hack = true;
}

void hng64_state::init_hng64_drive()
{
	m_no_machine_error_code = 0x02;
	init_hng64();
}

void hng64_state::init_roadedge()
{
	init_hng64_drive();
	m_roadedge_3d_hack = true;
}

void hng64_state::init_hng64_shoot()
{
	m_no_machine_error_code = 0x03;
	init_hng64();
}

void hng64_state::set_irq(u32 irq_vector)
{
	/*
	    TODO:
	    - irq sources;
	    - irq priority;
	    - is there an irq mask mechanism?
	    - is irq level cleared too when the irq acks?

	    IRQ level read at 0x80008cac
	    IO RAM is at bf808000 on the MIPS

	    -- irq table in Fatal Fury WA - 'empty' entries just do minimum 'interrupt service' with no real function.
	    80000400: 80039F20         irq00 vblank irq
	    80000404: 80039F84         1rq01 jump based on ram content
	    80000408: 8003A08C         irq02 'empty'
	    8000040C: 8006FF04         irq03 3d FIFO?
	    80000410: A0000410         irq04 INVALID
	    80000414: A0000414         irq05 INVALID
	    80000418: A0000418         irq06 INVALID
	    8000041C: A000041C         irq07 INVALID
	    80000420: A0000420         irq08 INVALID
	    80000424: 8003A00C         irq09 'empty'                       writes to sysreg 1074 instead of loading/storing regs tho
	    80000428: 80039FD0         irq0a 'empty'                       writes to sysreg 1074 instead of loading/storing regs tho
	    8000042C: 8003A0C0         irq0b 'empty'(network on xrally?)   writes to sysreg 1074 instead of loading/storing regs tho
	    80000430: 8003A050         irq0c 'empty'                       writes to sysreg 1074 instead of loading/storing regs tho
	    80000434: A0000434         irq0d INVALID
	    80000438: A0000438         irq0e INVALID
	    8000043C: A000043C         irq0f INVALID
	    80000440: A0000440         irq10 INVALID
	    80000444: 8003A0FC         irq11 IO MCU related?               write to sysreg 1084 instead of loading/storing regs, accesses dualport RAM
	    80000448: A0000448         irq12 INVALID
	    8000044C: A000044C         irq13 INVALID
	    80000450: A0000450         irq14 INVALID
	    80000454: A0000454         irq15 INVALID
	    80000458: A0000458         irq16 INVALID
	    8000045C: 8003A1D4         irq17 'empty'                       write to sysreg 1084 instead of loading/storing regs tho (like irq 0x11)
	    80000460: A0000460         irq18 INVALID
	    (all other entries, invalid)

	    Xrally (invalid IRQs are more obviously invalid, pointing at 0)
	    80000400: 80016ED0         irq00
	    80000404: 80016F58         irq01
	    80000408: 80017048         irq02
	    8000040C: 80013484         irq03
	    80000410: 00000000         irq04 INVALID
	    80000414: 00000000         irq05 INVALID
	    80000418: 00000000         irq06 INVALID
	    8000041C: 00000000         irq07 INVALID
	    80000420: 00000000         irq08 INVALID
	    80000424: 80016FC8         irq09
	    80000428: 80016F8C         irq0a
	    8000042C: 8001707C         irq0b
	    80000430: 8001700C         irq0c
	    80000434: 00000000         irq0d INVALID
	    80000438: 00000000         irq0e INVALID
	    8000043C: 00000000         irq0f INVALID
	    80000440: 00000000         irq10 INVALID
	    80000444: 800170C0         irq11
	    80000448: 00000000         irq12 INVALID
	    8000044C: 00000000         irq13 INVALID
	    80000450: 00000000         irq14 INVALID
	    80000454: 00000000         irq15 INVALID
	    80000458: 00000000         irq16 INVALID
	    8000045C: 80017198         irq17
	    80000460: 00000000         irq18 INVALID
	    (all other entries, invalid)

	    Buriki
	    80000400: 800C49C4
	    80000404: 800C4748
	    80000408: 800C4828
	    8000040C: 800C4B80
	    80000410: 00000000
	    80000414: 00000000
	    80000418: 00000000
	    8000041C: 00000000
	    80000420: 00000000
	    80000424: 800C47B0
	    80000428: 800C4778
	    8000042C: 800C4858
	    80000430: 800C47F0
	    80000434: 00000000
	    80000438: 00000000
	    8000043C: 00000000
	    80000440: 00000000
	    80000444: 800C4890
	    80000448: 00000000
	    8000044C: 00000000
	    80000450: 00000000
	    80000454: 00000000
	    80000458: 00000000
	    8000045C: 800C498C
	    80000460: 00000000

	    Beast Busters 2
	    80000400: 8000E9D8
	    80000404: 8000EAFC
	    80000408: 8000EBFC
	    8000040C: 80012D90
	    80000410: FFFFFFFF
	    80000414: FFFFFFFF
	    80000418: FFFFFFFF
	    8000041C: FFFFFFFF
	    80000420: FFFFFFFF
	    80000424: 8000EB74
	    80000428: 8000EB34
	    8000042C: 8000EC34
	    80000430: 8000EBBC
	    80000434: FFFFFFFF
	    80000438: FFFFFFFF
	    8000043C: FFFFFFFF
	    80000440: FFFFFFFF
	    80000444: 8000E508
	    80000448: FFFFFFFF
	    8000044C: FFFFFFFF
	    80000450: FFFFFFFF
	    80000454: FFFFFFFF
	    80000458: FFFFFFFF
	    8000045C: FFFFFFFF irq17 INVALID (not even a stub routine here)
	    80000460: FFFFFFFF

	    Roads Edge
	    80000400: 80028B04
	    80000404: 80028B88
	    80000408: 80028C68
	    8000040C: 80036FAC
	    80000410: 00000000
	    80000414: 00000000
	    80000418: 00000000
	    8000041C: 00000000
	    80000420: 00000000
	    80000424: 80028BF0
	    80000428: 80028BB8
	    8000042C: 80028C98
	    80000430: 80028C30
	    80000434: 00000000
	    80000438: 00000000
	    8000043C: 00000000
	    80000440: 00000000
	    80000444: 80027340
	    80000448: 00000000
	    8000044C: 00000000
	    80000450: 00000000
	    80000454: 00000000
	    80000458: 00000000
	    8000045C: 00000000 irq17 INVALID (not even a stub routine here)
	    80000460: 00000000

	    SamSho 64 code is more complex, irqs point to functions that get a jump address from a fixed ram location for each IRQ, most are invalid tho?
	    the ingame table is copied from 80005DD0
	                                      bootup   ingame
	    80000400: 800C03E0 irq00 80005dd0 800c02e0 800cfcc8
	    80000404: 800C041C irq01 80005dd4 800c0000
	    80000408: 800C0458 irq02 80005dd8 800c0000
	    8000040C: 800C0494 irq03 80005ddc 800c3054 800cfd58
	    80000410: 800C04D0 irq04 80005de0 800c3070 800cfdf8 - interesting because this level is invalid on other games
	    80000414: 800C032C irq05 80000478 00000000
	    80000418: 800C0368 irq06 80000478 00000000
	    8000041C: 800C03A4 irq07 80000478 00000000
	    80000420: 800C050C irq08 80005df0 800c0000
	    80000424: 800C0548 irq09 80005df4 800c0000
	    80000428: 800C0584 irq0a 80005df8 800c0000
	    8000042C: 800C05C0 irq0b 80005dfc 800c0000
	    80000430: 800C05FC irq0c 80005e00 800c0000
	    80000434: 800C02F0 irq0d 80000478 00000000
	    80000438: 800C02F0 irq0e 80000478 00000000
	    8000043C: 800C02F0 irq0f 80000478 00000000
	    80000440: 800C0638 irq10 80005e10 800c0000
	    80000444: 800C0674 irq11 80005e14 800c0000
	    80000448: 800C06B0 irq12 80005e18 800c0000
	    8000044C: 800C06EC irq13 80005e1c 800c0000
	    80000450: 800C0728 irq14 80005e20 800c0000
	    80000454: 800C0764 irq15 80005e24 800c0000
	    80000458: 800C07A0 irq16 80005e28 800c0000
	    8000045C: 800C07DC irq17 80005e2c 800c0000
	    80000460: 00000000 (invalid)

	    SamSho 64 2 is the same types as SamSho 64
	                                      bootup   ingame
	    80000400: 801008DC irq00 802011e0 801007e0 8011f6b4
	    80000404: 80100918 irq01 802011e4 80100500
	    80000408: 80100954 irq02 802011e8 80100500
	    8000040C: 80100990 irq03 802011ec 80101b38 8011f7b8
	    80000410: 801009CC irq04 802011f0 80101b54 80101b54
	    80000414: 80100828 irq05 80000478 0000000b
	    80000418: 80100864 irq06 80000478 0000000b
	    8000041C: 801008A0 irq07 80000478 0000000b
	    80000420: 80100A08 irq08 80201200 80100500
	    80000424: 80100A44 irq09 80201204 80100500
	    80000428: 80100A80 irq0a 80201208 80100500
	    8000042C: 80100ABC irq0b 8020120c 80100500
	    80000430: 80100AF8 irq0c 80201210 80100500
	    80000434: 801007EC irq0d 80000478 0000000b
	    80000438: 801007EC irq0e 80000478 0000000b
	    8000043C: 801007EC irq0f 80000478 0000000b
	    80000440: 80100B34 irq10 80201220 80100500
	    80000444: 80100B70 irq11 80201224 80100500
	    80000448: 80100BAC irq12 80201228 80100500
	    8000044C: 80100BE8 irq13 8020122c 80100500
	    80000450: 80100C24 irq14 80201230 80100500
	    80000454: 80100C60 irq15 80201234 80100500
	    80000458: 80100C9C irq16 80201238 80100500
	    8000045C: 80100CD8 irq17 8020123c 80100500
	    80000460: 00000000 (invalid)

	    Register 111c is connected to the interrupts and written in each one (IRQ ack / latch clear?)

	    HNG64 writing to SYSTEM Registers 0x0000111c == 0x00000001. (PC=80009b54) 0x00 vblank irq
	    HNG64 writing to SYSTEM Registers 0x0000111c == 0x00000002. (PC=80009b5c) 0x01 <empty> (not empty of ffwa)
	    HNG64 writing to SYSTEM Registers 0x0000111c == 0x00000004. (PC=80009b64) 0x02 <empty>
	    HNG64 writing to SYSTEM Registers 0x0000111c == 0x00000008. (PC=80009b6c) 0x03 3d fifo processed irq
	                                                         00010
	                                                         00020
	                                                         00040
	                                                         00080
	                                                         00100
	    HNG64 writing to SYSTEM Registers 0x0000111c == 0x00000200. (PC=80009b70) 0x09
	    HNG64 writing to SYSTEM Registers 0x0000111c == 0x00000400. (PC=80009b78) 0x0a
	    HNG64 writing to SYSTEM Registers 0x0000111c == 0x00000800. (PC=80009b88) 0x0b network irq, needed by xrally and roadedge
	                                                         01000
	                                                         02000
	                                                         04000
	                                                         08000
	                                                         10000
	    HNG64 writing to SYSTEM Registers 0x0000111c == 0x00020000. (PC=80009b80) 0x11 MCU related irq?
	                                                         40000
	                                                         80000
	                                                        100000
	                                                        200000
	                                                        400000
	                                                        800000 0x17 MCU related irq?

	    samsho64 / samsho64_2 does this during running:
	    HNG64 writing to SYSTEM Registers 0x0000111c == 0x00000000. (PC=800008fc) just checking?
	    HNG64 writing to SYSTEM Registers 0x0000111c == 0x00000040. (PC=800008fc) <- most notably causes TLBL error in fatfurwa
	*/

	m_irq_pending |= irq_vector;

	if (m_irq_pending)
	{
		for (int i = 0; i < 31; i++)
		{
			if (m_irq_pending & 1 << i)
			{
				m_irq_level = i;
				break;
			}
		}

		m_maincpu->set_input_line(0, ASSERT_LINE);
	}
	else
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(hng64_state::hng64_irq)
{
	const int scanline = param;

	if (!(scanline & 1)) // in reality there are half as many scanlines, as we're running in interlace mode
	{
		const int scanline_shifted = scanline >> 1;

		switch (scanline_shifted)
		{
		case 224: set_irq(0x0001);  break; // lv 0 vblank irq
//      case 32:  set_irq(0x0008);  break; // lv 2
//      case 64:  set_irq(0x0008);  break; // lv 2
		case 240: set_irq(0x0800);  break; // lv 11 network irq?

		default:
		{
			// raster / timer irq, used to split tilemaps for fatfurwa floor
			if (scanline_shifted == m_raster_irq_pos[0]+8)
				if (scanline_shifted <224)
					set_irq(0x0002);
			break;
		}

		}
	}

}

void hng64_state::machine_start()
{
	/* 1 meg of virtual address space for the com cpu */
	m_com_virtual_mem = std::make_unique<u8[]>(0x100000);
	m_com_op_base = std::make_unique<u8[]>(0x10000);

	m_soundram = std::make_unique<u16[]>(0x200000 / 2);
	m_soundram2 = std::make_unique<u16[]>(0x200000 / 2);

	/* set the fastest DRC options */
	m_maincpu->mips3drc_set_options(MIPS3DRC_FASTEST_OPTIONS + MIPS3DRC_STRICT_VERIFY);

	/* configure fast RAM regions */
	m_maincpu->add_fastram(0x00000000, 0x00ffffff, false, m_mainram);
	m_maincpu->add_fastram(0x04000000, 0x05ffffff, true, m_cart);
	m_maincpu->add_fastram(0x1fc00000, 0x1fc7ffff, true, m_rombase);

	for (int i = 0; i < 0x38 / 4; i++)
	{
		m_videoregs[i] = 0xdeadbeef;
	}

	for (int i = 0; i < 0x20; i++)
	{
		m_texture_wrapsize_table[i] = 0x08;
	}

	m_videoregs[0] = 0x00000000;

	m_irq_pending = 0;

	m_3dfifo_timer = timer_alloc(FUNC(hng64_state::_3dfifo_processed), this);
	m_comhack_timer = timer_alloc(FUNC(hng64_state::comhack_callback), this);

	m_com_bank->configure_entries(0, 4, memregion("comm")->base(), 0x20000);
	m_com_bank->set_entry(0);

	init_sound();
	init_io();

	save_pointer(NAME(m_com_virtual_mem), 0x100000);
	save_pointer(NAME(m_com_op_base), 0x10000);
	save_pointer(NAME(m_soundram), 0x200000 / 2);
	save_pointer(NAME(m_soundram2), 0x200000 / 2);

	save_item(NAME(m_fbcontrol));

	save_item(NAME(m_dma_start));
	save_item(NAME(m_dma_dst));
	save_item(NAME(m_dma_len));

	save_item(NAME(m_mcu_en));

	save_item(NAME(m_activeDisplayList));
	save_item(NAME(m_no_machine_error_code));

	save_item(NAME(m_unk_vreg_toggle));
	save_item(NAME(m_p1_trig));

	save_item(NAME(m_screen_dis));

	save_item(NAME(m_old_animmask));
	save_item(NAME(m_old_animbits));
	save_item(NAME(m_old_tileflags));

	save_item(NAME(m_port7));
	save_item(NAME(m_port1));
	save_item(NAME(m_ex_ramaddr));
	save_item(NAME(m_ex_ramaddr_upper));

	save_item(NAME(m_irq_pending));

	save_item(NAME(m_irq_level));

	save_item(NAME(main_latch));
	save_item(NAME(sound_latch));

	save_item(NAME(m_irq_pos_half));
	save_item(NAME(m_raster_irq_pos));

	save_item(NAME(m_texture_wrapsize_table));
}

TIMER_CALLBACK_MEMBER(hng64_state::comhack_callback)
{
	LOG("comhack_callback %04x\n\n", m_comhack[0]);

	// different network IDs give different default colours for the cars in roadedge
	u8 network_id = 0x01;

	// this fixes the stuck scroller text in the xrally intro (largest pink text) but prevents the inputs from working.
	// It's probably trying to sync the scroller with another unit? however the original machines can run as singles
	// if you loop some of the pins on the network connector back, so maybe MAME is just confused about the mode it's
	// running in.
	// network_id |= 0x08;


	m_comhack[0] = m_comhack[0] | network_id;
}


void hng64_state::machine_reset()
{
	/* For simulate MCU stepping */
	m_mcu_en = 0;

	reset_net();
	reset_sound();

	// on real hardware, even with no network, it takes until the counter reaches about 37 (Xtreme Rally) to boot, this kicks in at around 7
	m_comhack_timer->adjust(m_maincpu->cycles_to_attotime(400000000));

	// does the HW init these to anything?
	m_fbcontrol[0] = 0x00;
	m_fbcontrol[1] = 0x00;
	m_fbcontrol[2] = 0x00;
	m_fbcontrol[3] = 0x00;

	m_irq_pos_half = 0;
	m_raster_irq_pos[0] = 0xffffffff;
	m_raster_irq_pos[1] = 0xffffffff;

	clear3d();
}

/***********************************************

  Control / Lamp etc. access from MCU side?

  this is probably 8 multiplexed 8-bit input / output ports (probably joysticks, coins etc.)

***********************************************/

void hng64_state::ioport1_w(u8 data)
{
	//LOG("%s: ioport1_w %02x\n", machine().describe_context(), data);

	/* Port bits

	  aaac w-?-

	  a = external port number / address?
	  c = toggled during read / write accesses, probably clocking byte from/to latch

	  ? = toggled at the start of extint 0 , set during reads?

	  w = set during writes?

	*/

	m_port1 = data;
}

// it does write 0xff here before each set of reading, but before setting a new output address?
void hng64_state::ioport3_w(u8 data)
{

	if (m_port1 & 0x08) // 0x08 in port1 enables write? otherwise it writes 0xff to port 7 all the time, when port 7 is also lamps
	{
		const int addr = (m_port1 & 0xe0) >> 5;
		m_lamps->lamps_w(addr, data);
	}
}


u8 hng64_state::ioport3_r()
{
	const int addr = (m_port1 & 0xe0) >> 5;

	//LOG("%s: ioport3_r (from address %02x) (other bits of m_port1 %02x)\n", machine().describe_context(), addr, m_port1 & 0x1f);
	return m_in[addr]->read();
}

DEFINE_DEVICE_TYPE(HNG64_LAMPS, hng64_lamps_device, "hng64_lamps", "HNG64 Lamps")

hng64_lamps_device::hng64_lamps_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, HNG64_LAMPS, tag, owner, clock)
	, m_lamps_out_cb(*this)
{
}

void hng64_lamps_device::device_start()
{
}

void hng64_state::drive_lamps7_w(u8 data)
{
	/*
	   0x80 - BGM Select #2 (Active High)
	   0x40 - BGM Select #1 (Active High)
	   0x20
	   0x10
	   0x08
	   0x04
	   0x02
	   0x01
	*/
}

void hng64_state::drive_lamps6_w(u8 data)
{
	/*
	   0x80 - BGM Select #4 (Active High)
	   0x40 - BGM Select #3 (Active High)
	   0x20 - Winning Lamp (0x00 = ON, 0x10 = Blink 1, 0x20 = Blink 2, 0x30 = OFF)
	   0x10 -  ^^
	   0x08 - Breaking Lamp (Active Low?)
	   0x04 - Start Lamp (Active High)
	   0x02
	   0x01 - Coin Counter #1
	*/
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
}

void hng64_state::drive_lamps5_w(u8 data)
{
	// force feedback steering position
}

void hng64_state::shoot_lamps7_w(u8 data)
{
	/*
	   0x80
	   0x40 - Gun #3
	   0x20 - Gun #2
	   0x10 - Gun #1
	   0x08
	   0x04
	   0x02
	   0x01
	*/
}

/*
    Beast Busters 2 outputs (all written to offset 0x1c in dualport ram):
    0x00000001 start #1
    0x00000002 start #2
    0x00000004 start #3
    0x00001000 gun #1
    0x00002000 gun #2
    0x00004000 gun #3
*/

void hng64_state::shoot_lamps6_w(u8 data)
{
	// Start Lamp #1 / #2 don't get written to the output port, is this a TLCS870 bug or are they not connected to the 'lamp' outputs, they do get written to the DP ram, see above notes
	/*
	   0x80
	   0x40
	   0x20
	   0x10
	   0x08
	   0x04 - Start Lamp #3
	   0x02
	   0x01
	*/
}

void hng64_state::fight_lamps6_w(u8 data)
{
	/*
	   0x80
	   0x40
	   0x20
	   0x10
	   0x08
	   0x04
	   0x02 - Coin Counter #2
	   0x01 - Coin Counter #1
	*/
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
}


/***********************************************

 Dual Port RAM access from MCU side

***********************************************/

void hng64_state::ioport7_w(u8 data)
{
	/* Port bits

	 i?xR Aacr

	 a = 0x200 of address bit to external RAM (direct?)
	 A = 0x400 of address bit to external RAM (direct?)
	 R = read / write mode? (if 1, write, if 0, read?)

	 r = counter reset? ( 1->0 ?)
	 c = clock address? ( 1->0 ?)

	 x = written with clock bits, might be latch related?
	 ? = written before some operations

	 i = generate interrupt on MIPS? (written after the MCU has completed writing 'results' of some operations to shared ram, before executing more code to write another result, so needs to be processed quickly by the MIPS?)

	*/

	//LOG("%s: ioport7_w %02x\n", machine().describe_context(), data);

	m_ex_ramaddr_upper = (data & 0x0c) >> 2;

	if ((!(data & 0x80)) && (m_port7 & 0x80))
	{
		LOG("%s: MCU request MIPS IRQ?\n", machine().describe_context());
		set_irq(0x00020000);
	}

	if ((!(data & 0x01)) && (m_port7 & 0x01))
	{
		m_ex_ramaddr = 0;
	}

	if ((!(data & 0x02)) && (m_port7 & 0x02))
	{
		m_ex_ramaddr++;
		m_ex_ramaddr &= 0x1ff;
	}

	m_port7 = data;
}

u8 hng64_state::ioport0_r()
{
	const u16 addr = (m_ex_ramaddr | (m_ex_ramaddr_upper << 9)) & 0x7ff;
	const u8 ret = m_dt71321_dpram->left_r(addr);

	if (!machine().side_effects_disabled())
		LOG("%s: ioport0_r %02x (from address %04x)\n", machine().describe_context(), ret, addr);
	return ret;
}

void hng64_state::ioport0_w(u8 data)
{
	const u16 addr = (m_ex_ramaddr | (m_ex_ramaddr_upper << 9)) & 0x7ff;
	m_dt71321_dpram->left_w(addr, data);

	LOG("%s: ioport0_w %02x (to address %04x)\n", machine().describe_context(), data, addr);
}


/***********************************************

 Unknown (LED?) access from MCU side

***********************************************/

/* This port is dual purpose, with the upper pins being used as a serial input / output / clock etc. and the output latch (written data) being configured appropriately however the lower 2 bits also seem to be used
   maybe these lower 2 bits were intended for serial comms LEDs, although none are documented in the PCB layouts.
*/
void hng64_state::ioport4_w(u8 data)
{
	LOG("%s: ioport4_w %02x\n", machine().describe_context(), data);
}

/***********************************************

 Serial Accesses from MCU side

***********************************************/

/* I think the serial reads / writes actually go to the network hardware, and the IO MCU is acting as an interface between the actual network and the KL5C80A12CFP
   because the network connectors are on the IO board.  This might also be related to the 'm_no_machine_error_code' value required which differs per IO board
   type as the game startup sequences read that from the 0x6xx region of shared RAM, which also seems to be where a lot of the serial stuff is stored.
*/

// there are also serial reads, TLCS870 core doesn't support them yet

void hng64_state::sio0_w(int state)
{
	// tlcs870 core provides better logging than anything we could put here at the moment
}




TIMER_CALLBACK_MEMBER(hng64_state::tempio_irqon_callback)
{
	LOG("timer_hack_on\n");
	m_iomcu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
	m_tempio_irqoff_timer->adjust(m_maincpu->cycles_to_attotime(1000));
}

TIMER_CALLBACK_MEMBER(hng64_state::tempio_irqoff_callback)
{
	LOG("timer_hack_off\n");
	m_iomcu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}


void hng64_state::init_io()
{
	m_tempio_irqon_timer = timer_alloc(FUNC(hng64_state::tempio_irqon_callback), this);
	m_tempio_irqoff_timer = timer_alloc(FUNC(hng64_state::tempio_irqoff_callback), this);

	m_port7 = 0x00;
	m_port1 = 0x00;
	m_ex_ramaddr = 0;
	m_ex_ramaddr_upper = 0;
}

void hng64_state::hng64(machine_config &config)
{
	/* basic machine hardware */
	VR4300BE(config, m_maincpu, HNG64_MASTER_CLOCK);     // actually R4300
	m_maincpu->set_icache_size(16384);
	m_maincpu->set_dcache_size(16384);
	m_maincpu->set_addrmap(AS_PROGRAM, &hng64_state::main_map);

	TIMER(config, "scantimer", 0).configure_scanline(FUNC(hng64_state::hng64_irq), "screen", 0, 1);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	RTC62423(config, m_rtc, XTAL(32'768));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_hng64);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	m_screen->set_screen_update(FUNC(hng64_state::screen_update));
	m_screen->screen_vblank().set(FUNC(hng64_state::screen_vblank));

	PALETTE(config, m_palette).set_format(palette_device::xRGB_888, 0x1000);
	PALETTE(config, m_palette_fade0).set_format(palette_device::xRGB_888, 0x1000);
	PALETTE(config, m_palette_fade1).set_format(palette_device::xRGB_888, 0x1000);
	PALETTE(config, m_palette_3d).set_format(palette_device::xRGB_888, 0x1000 * 0x10);

	hng64_audio(config);
	hng64_network(config);

	tmp87ph40an_device &iomcu(TMP87PH40AN(config, m_iomcu, 8_MHz_XTAL));
	iomcu.p0_in_cb().set(FUNC(hng64_state::ioport0_r)); // reads from shared ram
	//iomcu.p1_in_cb().set(FUNC(hng64_state::ioport1_r)); // the IO MCU code uses opcodes that only access the output latch, never read from the port
	//iomcu.p2_in_cb().set(FUNC(hng64_state::ioport2_r)); // the IO MCU uses EXTINT0 which shares one of the pins on this port, but the port is not used for IO
	iomcu.p3_in_cb().set(FUNC(hng64_state::ioport3_r)); // probably reads input ports?
	//iomcu.p4_in_cb().set(FUNC(hng64_state::ioport4_r)); // the IO MCU code uses opcodes that only access the output latch, never read from the port
	//iomcu.p5_in_cb().set(FUNC(hng64_state::ioport5_r)); // simply seems to be unused, neither used for an IO port, nor any of the other features
	//iomcu.p6_in_cb().set(FUNC(hng64_state::ioport6_r)); // the IO MCU code uses the ADC which shares pins with port 6, meaning port 6 isn't used as an IO port
	//iomcu.p7_in_cb().set(FUNC(hng64_state::ioport7_r)); // the IO MCU code uses opcodes that only access the output latch, never read from the port
	iomcu.p0_out_cb().set(FUNC(hng64_state::ioport0_w)); // writes to shared ram
	iomcu.p1_out_cb().set(FUNC(hng64_state::ioport1_w));  // configuration / clocking for input port (port 3) accesses
	//iomcu.p2_out_cb().set(FUNC(hng64_state::ioport2_w)); // the IO MCU uses EXTINT0 which shares one of the pins on this port, but the port is not used for IO
	iomcu.p3_out_cb().set(FUNC(hng64_state::ioport3_w)); // writes to ports for lamps, coin counters, force feedback etc.
	iomcu.p4_out_cb().set(FUNC(hng64_state::ioport4_w)); // unknown, lower 2 IO bits accessed along with serial accesses
	//iomcu.p5_out_cb().set(FUNC(hng64_state::ioport5_w));  // simply seems to be unused, neither used for an IO port, nor any of the other features
	//iomcu.p6_out_cb().set(FUNC(hng64_state::ioport6_w)); // the IO MCU code uses the ADC which shares pins with port 6, meaning port 6 isn't used as an IO port
	iomcu.p7_out_cb().set(FUNC(hng64_state::ioport7_w)); // configuration / clocking for shared ram (port 0) accesses
	// most likely the analog inputs, up to a maximum of 8
	iomcu.an0_in_cb().set_ioport("AN0");
	iomcu.an1_in_cb().set_ioport("AN1");
	iomcu.an2_in_cb().set_ioport("AN2");
	iomcu.an3_in_cb().set_ioport("AN3");
	iomcu.an4_in_cb().set_ioport("AN4");
	iomcu.an5_in_cb().set_ioport("AN5");
	iomcu.an6_in_cb().set_ioport("AN6");
	iomcu.an7_in_cb().set_ioport("AN7");
	// network related?
	iomcu.serial0_out_cb().set(FUNC(hng64_state::sio0_w));
	//iomcu.serial1_out_cb().set(FUNC(hng64_state::sio1_w)); // not initialized / used

	IDT71321(config, "dt71321_dpram", 0);
	//MCFG_MB8421_INTL_AN0R(INPUTLINE("xxx", 0)) // I don't think the IRQs are connected
}

void hng64_state::hng64_default(machine_config &config)
{
	hng64(config);

	hng64_lamps_device &lamps(HNG64_LAMPS(config, m_lamps, 0));
	lamps.lamps_out_cb<0>().set(FUNC(hng64_state::default_lamps_w<0>));
	lamps.lamps_out_cb<1>().set(FUNC(hng64_state::default_lamps_w<1>));
	lamps.lamps_out_cb<2>().set(FUNC(hng64_state::default_lamps_w<2>));
	lamps.lamps_out_cb<3>().set(FUNC(hng64_state::default_lamps_w<3>));
	lamps.lamps_out_cb<4>().set(FUNC(hng64_state::default_lamps_w<4>));
	lamps.lamps_out_cb<5>().set(FUNC(hng64_state::default_lamps_w<5>));
	lamps.lamps_out_cb<6>().set(FUNC(hng64_state::default_lamps_w<6>));
	lamps.lamps_out_cb<7>().set(FUNC(hng64_state::default_lamps_w<7>));
}

void hng64_state::hng64_drive(machine_config &config)
{
	hng64(config);

	hng64_lamps_device &lamps(HNG64_LAMPS(config, m_lamps, 0));
	lamps.lamps_out_cb<5>().set(FUNC(hng64_state::drive_lamps5_w)); // force feedback steering
	lamps.lamps_out_cb<6>().set(FUNC(hng64_state::drive_lamps6_w)); // lamps + coin counter
	lamps.lamps_out_cb<7>().set(FUNC(hng64_state::drive_lamps7_w)); // lamps
}

void hng64_state::hng64_shoot(machine_config &config)
{
	hng64(config);

	hng64_lamps_device &lamps(HNG64_LAMPS(config, m_lamps, 0));
	lamps.lamps_out_cb<6>().set(FUNC(hng64_state::shoot_lamps6_w)); // start lamps (some missing?!)
	lamps.lamps_out_cb<7>().set(FUNC(hng64_state::shoot_lamps7_w)); // gun lamps
}

void hng64_state::hng64_fight(machine_config &config)
{
	hng64(config);

	hng64_lamps_device &lamps(HNG64_LAMPS(config, m_lamps, 0));
	lamps.lamps_out_cb<6>().set(FUNC(hng64_state::fight_lamps6_w)); // coin counters
}


#define ROM_LOAD_HNG64_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash,  ROM_BIOS(bios))

/* All main BIOS roms are said to be from 'fighting' type PCB, it is unknown if the actual MIPS BIOS differs on the others, but it appears unlikely.

  The IO MCU was dumped from a TMP87PH40AN type chip taken from an unknown IO board type.

  Some boards instead use a TMP87CH40N but in all cases they're stickered SNK-IOJ1.00A so the content is possibly the same on all types.

  This needs further studying of the MCU code as it is known that the different IO boards return a different ident value.
*/

#define HNG64_BIOS \
	/* R4300 BIOS code (main CPU) */ \
	ROM_REGION32_BE( 0x0100000, "bios", 0 ) \
	ROM_SYSTEM_BIOS( 0, "japan", "Japan" ) \
	ROM_LOAD_HNG64_BIOS( 0, "brom1.bin",         0x00000, 0x080000, CRC(a30dd3de) SHA1(3e2fd0a56214e6f5dcb93687e409af13d065ea30) ) \
	ROM_SYSTEM_BIOS( 1, "us", "USA" ) \
	ROM_LOAD_HNG64_BIOS( 1, "bios_us.bin",       0x00000, 0x080000,  CRC(ab5948d6) SHA1(f8b940c1ae5ce2d3b2cd0c9bfaf6e5b063cec06e) ) \
	ROM_SYSTEM_BIOS( 2, "export", "Export" ) \
	ROM_LOAD_HNG64_BIOS( 2, "bios_export.bin",   0x00000, 0x080000, CRC(bbf07ec6) SHA1(5656aa077f6a6d43953f15b5123eea102a9d5313) ) \
	ROM_SYSTEM_BIOS( 3, "korea", "Korea" ) \
	ROM_LOAD_HNG64_BIOS( 3, "bios_korea.bin",    0x00000, 0x080000, CRC(ac953e2e) SHA1(f502188ef252b7c9d04934c4b525730a116de48b) ) \
	/* KL5C80 BIOS (network CPU) */ \
	ROM_REGION( 0x080000, "comm", 0 ) \
	ROM_LOAD ( "from1.bin", 0x000000, 0x080000,  CRC(6b933005) SHA1(e992747f46c48b66e5509fe0adf19c91250b00c7) ) \
	/* FPGA (unknown) */ \
	ROM_REGION( 0x0100000, "fpga", 0 ) /* FPGA data  */ \
	ROM_LOAD ( "rom1.bin",  0x000000, 0x01ff32,  CRC(4a6832dc) SHA1(ae504f7733c2f40450157cd1d3b85bc83fac8569) ) \
	/* TMP87PH40AN (I/O MCU) */ \
	ROM_REGION( 0x10000, "iomcu", 0 ) /* "64Bit I/O Controller Ver 1.0 1997.06.29(C)SNK" internal ID string */ \
	ROM_LOAD ( "tmp87ph40an.bin",  0x8000, 0x8000,  CRC(b70df21f) SHA1(5b742e8a0bbf4c0ae4f4398d34c7058fb24acc92) ) \
	/* BR9020F (EEPROM) */ \
	ROM_REGION( 0x100, "eeprom", 0 ) /* EEPROMs on the I/O boards, mostly empty, currently not used by the emulation */ \
	ROM_LOAD( "lvs-ioj-br9020f.u2", 0x000, 0x100, CRC(78b7020d) SHA1(2b8549532ef5e1e8102dbe71af55fdfb27ccbba6) ) \
	ROM_LOAD( "lvs-igx-br9020f.u3", 0x000, 0x100, CRC(af9f4287) SHA1(6df0e35c77dbfee2fab7ff490dcd651db420e367) ) \
	ROM_LOAD( "lvs-jam-br9020f.u3", 0x000, 0x100, CRC(dabec5d2) SHA1(19c5be89c57387d6ea563b3dc55674d0692af98e) ) \
	ROM_DEFAULT_BIOS( "export" )

ROM_START( hng64 )
	/* BIOS */
	HNG64_BIOS

	/* To placate MAME */
	ROM_REGION32_BE( 0x2000000, "gameprg", ROMREGION_ERASEFF )
	ROM_REGION( 0x4000, "scrtile", ROMREGION_ERASEFF )
	ROM_REGION( 0x4000, "sprtile", ROMREGION_ERASEFF )
	ROM_REGION( 0x1000000, "textures0", ROMREGION_ERASEFF )
	ROM_REGION( 0x1000000, "textures1", ROMREGION_ERASEFF )
	ROM_REGION( 0x1000000, "textures2", ROMREGION_ERASEFF )
	ROM_REGION( 0x1000000, "textures3", ROMREGION_ERASEFF )
	ROM_REGION16_LE( 0x0c00000, "verts", ROMREGION_ERASEFF )
	ROM_REGION( 0x1000000, "l7a1045", ROMREGION_ERASEFF ) /* Sound Samples */
ROM_END


ROM_START( roadedge )
	HNG64_BIOS

	ROM_REGION32_BE( 0x2000000, "gameprg", 0 )
	ROM_LOAD32_WORD_SWAP( "001pr01b.81", 0x0000002, 0x400000, CRC(effbac30) SHA1(c1bddf3e511a8950f65ac7e452f81dbc4b7fd977) )
	ROM_LOAD32_WORD_SWAP( "001pr02b.82", 0x0000000, 0x400000, CRC(b9aa4ad3) SHA1(9ab3c896dbdc45560b7127486e2db6ca3b15a057) )

	/* Scroll Characters 8x8x8 / 16x16x8 */
	ROM_REGION( 0x1000000, "scrtile", 0 )
	ROM_LOAD16_BYTE( "001sc01a.41", 0x0000000, 0x400000, CRC(084395a1) SHA1(8bfea8fd3981fd45dcc04bd74840a5948aaf06a8) )
	ROM_LOAD16_BYTE( "001sc02a.42", 0x0000001, 0x400000, CRC(51dd19e3) SHA1(eeb3634294a049a357a75ee00aa9fce65b737395) )
	ROM_LOAD16_BYTE( "001sc03a.43", 0x0800000, 0x400000, CRC(0b6f3e19) SHA1(3b6dfd0f0633b0d8b629815920edfa39d92336bf) )
	ROM_LOAD16_BYTE( "001sc04a.44", 0x0800001, 0x400000, CRC(256c8c1c) SHA1(85935eea3722ec92f8d922f527c2e049c4185aa3) )

	/* Sprite Characters - 8x8x8 / 16x16x8 */
	ROM_REGION( 0x1000000, "sprtile", 0 )
	ROM_LOAD32_BYTE( "001sp01a.53",0x0000000, 0x400000, CRC(7a469453) SHA1(3738ca76f538243bb23ffd23a42b2a0558882889) )
	ROM_LOAD32_BYTE( "001sp02a.54",0x0000001, 0x400000, CRC(6b9a3de0) SHA1(464c652f7b193326e3a871dfe751dd83c14284eb) )
	ROM_LOAD32_BYTE( "001sp03a.55",0x0000002, 0x400000, CRC(efbbd391) SHA1(7447c481ba6f9ba154d48a4b160dd24157891d35) )
	ROM_LOAD32_BYTE( "001sp04a.56",0x0000003, 0x400000, CRC(1a0eb173) SHA1(a69b786a9957197d1cc950ab046c57c18ca07ea7) )

	/* Textures
	   NOTE: same roms are at different positions on the board, repeated a total of 4 times as there are 4 rendering units
	   We only use the first copy, as the ROMs are always identical
	*/
	ROM_REGION( 0x1000000, "textures0", 0 )
	ROM_LOAD( "001tx01a.1", 0x0000000, 0x400000, CRC(f6539bb9) SHA1(57fc5583d56846be93d6f5784acd20fc149c70a5) )
	ROM_LOAD( "001tx02a.2", 0x0400000, 0x400000, CRC(f1d139d3) SHA1(f120243f4d55f38b10bf8d1aa861cdc546a24c80) )
	ROM_LOAD( "001tx03a.3", 0x0800000, 0x400000, CRC(22a375bd) SHA1(d55b62843d952930db110bcf3056a98a04a7adf4) )
	ROM_LOAD( "001tx04a.4", 0x0c00000, 0x400000, CRC(288a5bd5) SHA1(24e05db681894eb31cdc049cf42c1f9d7347bd0c) )

	ROM_REGION( 0x1000000, "textures1", 0 )
	ROM_LOAD( "001tx01a.5", 0x0000000, 0x400000, CRC(f6539bb9) SHA1(57fc5583d56846be93d6f5784acd20fc149c70a5) )
	ROM_LOAD( "001tx02a.6", 0x0400000, 0x400000, CRC(f1d139d3) SHA1(f120243f4d55f38b10bf8d1aa861cdc546a24c80) )
	ROM_LOAD( "001tx03a.7", 0x0800000, 0x400000, CRC(22a375bd) SHA1(d55b62843d952930db110bcf3056a98a04a7adf4) )
	ROM_LOAD( "001tx04a.8", 0x0c00000, 0x400000, CRC(288a5bd5) SHA1(24e05db681894eb31cdc049cf42c1f9d7347bd0c) )

	ROM_REGION( 0x1000000, "textures2", 0 )
	ROM_LOAD( "001tx01a.9", 0x0000000, 0x400000, CRC(f6539bb9) SHA1(57fc5583d56846be93d6f5784acd20fc149c70a5) )
	ROM_LOAD( "001tx02a.10",0x0400000, 0x400000, CRC(f1d139d3) SHA1(f120243f4d55f38b10bf8d1aa861cdc546a24c80) )
	ROM_LOAD( "001tx03a.11",0x0800000, 0x400000, CRC(22a375bd) SHA1(d55b62843d952930db110bcf3056a98a04a7adf4) )
	ROM_LOAD( "001tx04a.12",0x0c00000, 0x400000, CRC(288a5bd5) SHA1(24e05db681894eb31cdc049cf42c1f9d7347bd0c) )

	ROM_REGION( 0x1000000, "textures3", 0 )
	ROM_LOAD( "001tx01a.13",0x0000000, 0x400000, CRC(f6539bb9) SHA1(57fc5583d56846be93d6f5784acd20fc149c70a5) )
	ROM_LOAD( "001tx02a.14",0x0400000, 0x400000, CRC(f1d139d3) SHA1(f120243f4d55f38b10bf8d1aa861cdc546a24c80) )
	ROM_LOAD( "001tx03a.15",0x0800000, 0x400000, CRC(22a375bd) SHA1(d55b62843d952930db110bcf3056a98a04a7adf4) )
	ROM_LOAD( "001tx04a.16",0x0c00000, 0x400000, CRC(288a5bd5) SHA1(24e05db681894eb31cdc049cf42c1f9d7347bd0c) )

	/* X,Y,Z Vertex ROMs */
	ROM_REGION16_LE( 0x0c00000, "verts", 0 )
	ROMX_LOAD( "001vt01a.17", 0x0000000, 0x400000, CRC(1a748e1b) SHA1(376d40baa3b94890d4740045d053faf208fe43db), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "001vt02a.18", 0x0000002, 0x400000, CRC(449f94d0) SHA1(2228690532d82d2661285aeb4260689b027597cb), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "001vt03a.19", 0x0000004, 0x400000, CRC(50ac8639) SHA1(dd2d3689466990a7c479bb8f11bd930ea45e47b5), ROM_GROUPWORD | ROM_SKIP(4) )

	ROM_REGION( 0x1000000, "l7a1045", 0 ) /* Sound Samples */
	ROM_LOAD( "001sd01a.77", 0x0000000, 0x400000, CRC(a851da99) SHA1(2ba24feddafc5fadec155cdb7af305fdffcf6690) )
	ROM_LOAD( "001sd02a.78", 0x0400000, 0x400000, CRC(ca5cec15) SHA1(05e91a602728a048d61bf86aa8d43bb4186aeac1) )
ROM_END


ROM_START( sams64 )
	HNG64_BIOS

	ROM_REGION32_BE( 0x2000000, "gameprg", 0 )
	ROM_LOAD32_WORD_SWAP( "002-pro1a.81", 0x0000002, 0x400000, CRC(e5b907c5) SHA1(83637ffaa9031d41a5bed3397a519d1dfa8052cb) )
	ROM_LOAD32_WORD_SWAP( "002-pro2a.82", 0x0000000, 0x400000, CRC(803ed2eb) SHA1(666db47886a316e68b911311e5db3bc0f5b8a34d) )
	ROM_LOAD32_WORD_SWAP( "002-pro3a.83", 0x0800002, 0x400000, CRC(582156a7) SHA1(a7bbbd472a53072cbfaed5d41d4265123c9e3f3d) )
	ROM_LOAD32_WORD_SWAP( "002-pro4a.84", 0x0800000, 0x400000, CRC(5a8291e9) SHA1(ec1e5a5a0ba37393e8b93d78b4ac855109d45ec9) )

	/* Scroll Characters 8x8x8 / 16x16x8 */
	ROM_REGION( 0x2000000, "scrtile", 0 )
	ROM_LOAD16_BYTE( "002-sc01a.41", 0x0000000, 0x400000, CRC(77c3df69) SHA1(813d57814acccd2c04c951e58ac87cf7413bdf58) )
	ROM_LOAD16_BYTE( "002-sc02a.42", 0x0000001, 0x400000, CRC(60065174) SHA1(624c2e20abb53b2466df4ce2ffa9e20273798e92) )
	ROM_LOAD16_BYTE( "002-sc05a.45", 0x0800000, 0x400000, CRC(fd242bee) SHA1(b1fad97987da21c77d6c460bbed6f0dd18905ed4) )
	ROM_LOAD16_BYTE( "002-sc06a.46", 0x0800001, 0x400000, CRC(87afc297) SHA1(47d5eaae88ce501fbbd5a2d7305c1d6acadfb13e) )
	ROM_LOAD16_BYTE( "002-sc03a.43", 0x1000000, 0x400000, CRC(5d4a5289) SHA1(7a1576fdd344825cb05866c156d17b18f562a336) )
	ROM_LOAD16_BYTE( "002-sc04a.44", 0x1000001, 0x400000, CRC(aa5536fa) SHA1(09a50a29561ac97c564243da879bd7c4cf8c3cee) )
	ROM_LOAD16_BYTE( "002-sc07a.47", 0x1800000, 0x400000, CRC(e01e8a95) SHA1(b132214ef2b33a46cb605ea8f2193e77d9464881) )
	ROM_LOAD16_BYTE( "002-sc08a.48", 0x1800001, 0x400000, CRC(a17464d0) SHA1(2e6b73b1e0983b2b01455b0f4d6dc7c3845adb69) )

	/* Sprite Characters - 8x8x8 / 16x16x8 */
	ROM_REGION( 0x2000000, "sprtile", 0 )
	ROM_LOAD32_BYTE( "002-sp01a.53",0x0000000, 0x400000, CRC(c73cf9b4) SHA1(7c34fa1bc03cd366d473dbf3e316a6434ee5ec60) )
	ROM_LOAD32_BYTE( "002-sp02a.54",0x0000001, 0x400000, CRC(04b0ecc8) SHA1(893e522324dd41dfcd2217974a6740e6bc3ea1d3) )
	ROM_LOAD32_BYTE( "002-sp03a.55",0x0000002, 0x400000, CRC(13c80b74) SHA1(ad6c1690ebcde0d8237201ea43eb162cd5308ccb) )
	ROM_LOAD32_BYTE( "002-sp04a.56",0x0000003, 0x400000, CRC(b1a6a06d) SHA1(1b11ee7cec46d0c99dc6310ee8221fa2de33c359) )
	ROM_LOAD32_BYTE( "002-sp05a.57",0x1000000, 0x400000, CRC(fa71e825) SHA1(adfa8b5a8ec703d4f04285c47f2618a294c90ec5) )
	ROM_LOAD32_BYTE( "002-sp06a.58",0x1000001, 0x400000, CRC(1bcfe48e) SHA1(8d85b1eb33fea48e5c6597d2fcbec903ecdad9d9) )
	ROM_LOAD32_BYTE( "002-sp07a.59",0x1000002, 0x400000, CRC(a5049bd7) SHA1(123e32c22f53d6e55ee1d1deb4ab40891004c6fd) )
	ROM_LOAD32_BYTE( "002-sp08a.60",0x1000003, 0x400000, CRC(c2e57813) SHA1(e7a21df1f94ed959a53da9dc4667863ee77bf676) )

	/* Textures
	   NOTE: same roms are at different positions on the board, repeated a total of 4 times as there are 4 rendering units
	   We only use the first copy, as the ROMs are always identical
	*/
	ROM_REGION( 0x1000000, "textures0", 0 )
	ROM_LOAD( "002-tx01a.1", 0x0000000, 0x400000, CRC(233749b5) SHA1(7c93681bbd5f4246e0dc50d26108f04e9b248d0d) )
	ROM_LOAD( "002-tx02a.2", 0x0400000, 0x400000, CRC(d5074be2) SHA1(c33e9b9f0d21ad5ad31d8f988b3c7378d374fc1b) )
	ROM_LOAD( "002-tx03a.3", 0x0800000, 0x400000, CRC(68c313f7) SHA1(90ce8d0d19a994647c7167e3b256ff31647e575a) )
	ROM_LOAD( "002-tx04a.4", 0x0c00000, 0x400000, CRC(f7dac24f) SHA1(1215354f28cbeb9fc38f6a7acae450ad5f34bb6a) )

	ROM_REGION( 0x1000000, "textures1", 0 )
	ROM_LOAD( "002-tx01a.5", 0x0000000, 0x400000, CRC(233749b5) SHA1(7c93681bbd5f4246e0dc50d26108f04e9b248d0d) )
	ROM_LOAD( "002-tx02a.6", 0x0400000, 0x400000, CRC(d5074be2) SHA1(c33e9b9f0d21ad5ad31d8f988b3c7378d374fc1b) )
	ROM_LOAD( "002-tx03a.7", 0x0800000, 0x400000, CRC(68c313f7) SHA1(90ce8d0d19a994647c7167e3b256ff31647e575a) )
	ROM_LOAD( "002-tx04a.8", 0x0c00000, 0x400000, CRC(f7dac24f) SHA1(1215354f28cbeb9fc38f6a7acae450ad5f34bb6a) )

	ROM_REGION( 0x1000000, "textures2", 0 )
	ROM_LOAD( "002-tx01a.9",  0x0000000, 0x400000, CRC(233749b5) SHA1(7c93681bbd5f4246e0dc50d26108f04e9b248d0d) )
	ROM_LOAD( "002-tx02a.10", 0x0400000, 0x400000, CRC(d5074be2) SHA1(c33e9b9f0d21ad5ad31d8f988b3c7378d374fc1b) )
	ROM_LOAD( "002-tx03a.11", 0x0800000, 0x400000, CRC(68c313f7) SHA1(90ce8d0d19a994647c7167e3b256ff31647e575a) )
	ROM_LOAD( "002-tx04a.12", 0x0c00000, 0x400000, CRC(f7dac24f) SHA1(1215354f28cbeb9fc38f6a7acae450ad5f34bb6a) )

	ROM_REGION( 0x1000000, "textures3", 0 )
	ROM_LOAD( "002-tx01a.13", 0x0000000, 0x400000, CRC(233749b5) SHA1(7c93681bbd5f4246e0dc50d26108f04e9b248d0d) )
	ROM_LOAD( "002-tx02a.14", 0x0400000, 0x400000, CRC(d5074be2) SHA1(c33e9b9f0d21ad5ad31d8f988b3c7378d374fc1b) )
	ROM_LOAD( "002-tx03a.15", 0x0800000, 0x400000, CRC(68c313f7) SHA1(90ce8d0d19a994647c7167e3b256ff31647e575a) )
	ROM_LOAD( "002-tx04a.16", 0x0c00000, 0x400000, CRC(f7dac24f) SHA1(1215354f28cbeb9fc38f6a7acae450ad5f34bb6a) )

	/* X,Y,Z Vertex ROMs */
	ROM_REGION16_LE( 0x1800000, "verts", 0 )
	ROMX_LOAD( "002-vt01a.17", 0x0000000, 0x400000, CRC(403fd7fd) SHA1(9bdadbeb4cd13c4c4e89a1c233af9eaaa46f8fdf), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "002-vt02a.18", 0x0000002, 0x400000, CRC(e1885905) SHA1(6b16083c50e887aebe2baf95bf56697c239970f2), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "002-vt03a.19", 0x0000004, 0x400000, CRC(2074a6a6) SHA1(9a5e8259d1e19d2b43878c24ca06afba5ee5e316), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "002-vt04a.20", 0x0c00000, 0x400000, CRC(aefc4d94) SHA1(f9d8222d4320ccf9f3c7c0ef307e03c8f34ea530), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "002-vt05a.21", 0x0c00002, 0x400000, CRC(d32ee9cb) SHA1(a768dfc15899924eb05eccbf8e85cb29c7b60396), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "002-vt06a.22", 0x0c00004, 0x400000, CRC(13bf3636) SHA1(7c704bf66b571350207bccc7a2d6ed1ec9de4cd5), ROM_GROUPWORD | ROM_SKIP(4) )

	ROM_REGION( 0x1000000, "l7a1045", 0 ) /* Sound Samples */
	ROM_LOAD( "002-sd01a.77", 0x0000000, 0x400000, CRC(6215036b) SHA1(ded71dce98b7f7ef78ef32d966a292bbf0d15332) )
	ROM_LOAD( "002-sd02a.78", 0x0400000, 0x400000, CRC(32b28310) SHA1(5b80750a66c12b035b493d06e3842741a3334d0f) )
	ROM_LOAD( "002-sd03a.79", 0x0800000, 0x400000, CRC(53591413) SHA1(36c7efa1aced0ca38b3ce7b95af28755973698f3) )
ROM_END


ROM_START( xrally )
	HNG64_BIOS

	ROM_REGION32_BE( 0x2000000, "gameprg", 0 )
	ROM_LOAD32_WORD_SWAP( "003-pr01a.81", 0x0000002, 0x400000, CRC(4e160388) SHA1(08fba66d0f0dab47f7db5bc7d411f4fc0e8219c8) )
	ROM_LOAD32_WORD_SWAP( "003-pr02a.82", 0x0000000, 0x400000, CRC(c4dd4f18) SHA1(4db0e6d5cabd9e4f82d5905556174b9eff8ad4d9) )

	/* Scroll Characters 8x8x8 / 16x16x8 */
	ROM_REGION( 0x1000000, "scrtile", 0 )
	ROM_LOAD16_BYTE( "003-sc01a.41", 0x0000000, 0x400000, CRC(bc608584) SHA1(fa4b618eb36f302f58cefea7c50618a8318927d6) )
	ROM_LOAD16_BYTE( "003-sc02a.42", 0x0000001, 0x400000, CRC(c810e9e2) SHA1(4f0d35d9b0af2a4b66253e467c0d30a519c904b6) )
	ROM_LOAD16_BYTE( "003-sc03a.43", 0x0800000, 0x400000, CRC(12724653) SHA1(5e40947086883d64db84ac51a1b29efa2f173f58) )
	ROM_LOAD16_BYTE( "003-sc04a.44", 0x0800001, 0x400000, CRC(b0062c4d) SHA1(73c75b59dc1463ad80f805191f4605a6b4b1c321) )

	/* Sprite Characters - 8x8x8 / 16x16x8 */
	ROM_REGION( 0x1000000, "sprtile", 0 )
	ROM_LOAD32_BYTE( "003-sp01a.53",0x0000000, 0x400000, CRC(12a329dc) SHA1(00929f3c460cce5a3657dec73d467731e59de564) )
	ROM_LOAD32_BYTE( "003-sp02a.54",0x0000001, 0x400000, CRC(ee9e5338) SHA1(681c2f34a2f292ce14fcbef4447ede7b949c7117) )
	ROM_LOAD32_BYTE( "003-sp03a.55",0x0000002, 0x400000, CRC(6fa8dff9) SHA1(500bd128e6568e9491e52676775e9239adc332fe) )
	ROM_LOAD32_BYTE( "003-sp04a.56",0x0000003, 0x400000, CRC(a98eec07) SHA1(de0c7db56b851daa369f37088bd536933372346f) )

	/* Textures
	   NOTE: same roms are at different positions on the board, repeated a total of 4 times as there are 4 rendering units
	   We only use the first copy, as the ROMs are always identical
	*/
	ROM_REGION( 0x1000000, "textures0", 0 )
	ROM_LOAD( "003-tx01a.1", 0x0000000, 0x400000, CRC(83ea2178) SHA1(931898f57564b8b9975e06df5ccfd8c84fc2fbe3) )
	ROM_LOAD( "003-tx02a.2", 0x0400000, 0x400000, CRC(7912f4be) SHA1(bca44c1415a25f2349857b2246e3ee7abe709a84) )
	ROM_LOAD( "003-tx03a.3", 0x0800000, 0x400000, CRC(a319c94e) SHA1(14d720cdd8b9411fd82a7b4b33ee5dbfdd01c9f8) )
	ROM_LOAD( "003-tx04a.4", 0x0c00000, 0x400000, CRC(16d7805b) SHA1(4cc7b2375832c2f9f20fe882e604a2a52bf07f6f) )

	ROM_REGION( 0x1000000, "textures1", 0 )
	ROM_LOAD( "003-tx01a.5", 0x0000000, 0x400000, CRC(83ea2178) SHA1(931898f57564b8b9975e06df5ccfd8c84fc2fbe3) )
	ROM_LOAD( "003-tx02a.6", 0x0400000, 0x400000, CRC(7912f4be) SHA1(bca44c1415a25f2349857b2246e3ee7abe709a84) )
	ROM_LOAD( "003-tx03a.7", 0x0800000, 0x400000, CRC(a319c94e) SHA1(14d720cdd8b9411fd82a7b4b33ee5dbfdd01c9f8) )
	ROM_LOAD( "003-tx04a.8", 0x0c00000, 0x400000, CRC(16d7805b) SHA1(4cc7b2375832c2f9f20fe882e604a2a52bf07f6f) )

	ROM_REGION( 0x1000000, "textures2", 0 )
	ROM_LOAD( "003-tx01a.9",  0x0000000, 0x400000, CRC(83ea2178) SHA1(931898f57564b8b9975e06df5ccfd8c84fc2fbe3) )
	ROM_LOAD( "003-tx02a.10", 0x0400000, 0x400000, CRC(7912f4be) SHA1(bca44c1415a25f2349857b2246e3ee7abe709a84) )
	ROM_LOAD( "003-tx03a.11", 0x0800000, 0x400000, CRC(a319c94e) SHA1(14d720cdd8b9411fd82a7b4b33ee5dbfdd01c9f8) )
	ROM_LOAD( "003-tx04a.12", 0x0c00000, 0x400000, CRC(16d7805b) SHA1(4cc7b2375832c2f9f20fe882e604a2a52bf07f6f) )

	ROM_REGION( 0x1000000, "textures3", 0 )
	ROM_LOAD( "003-tx01a.13", 0x0000000, 0x400000, CRC(83ea2178) SHA1(931898f57564b8b9975e06df5ccfd8c84fc2fbe3) )
	ROM_LOAD( "003-tx02a.14", 0x0400000, 0x400000, CRC(7912f4be) SHA1(bca44c1415a25f2349857b2246e3ee7abe709a84) )
	ROM_LOAD( "003-tx03a.15", 0x0800000, 0x400000, CRC(a319c94e) SHA1(14d720cdd8b9411fd82a7b4b33ee5dbfdd01c9f8) )
	ROM_LOAD( "003-tx04a.16", 0x0c00000, 0x400000, CRC(16d7805b) SHA1(4cc7b2375832c2f9f20fe882e604a2a52bf07f6f) )

	/* X,Y,Z Vertex ROMs */
	ROM_REGION16_LE( 0x0c00000, "verts", 0 )
	ROMX_LOAD( "003-vt01a.17", 0x0000000, 0x400000, CRC(3e5e275d) SHA1(74f5ec88c258bc224e271f7abeb02d6485e27d8c), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "003-vt02a.18", 0x0000002, 0x400000, CRC(da7b956e) SHA1(c57cbb8c51145ae224faba5b6a1a7e61cb2bee64), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "003-vt03a.19", 0x0000004, 0x400000, CRC(4fe72cb7) SHA1(9f8e662f0656f201924834d1ee78498d4223745e), ROM_GROUPWORD | ROM_SKIP(4) )

	ROM_REGION( 0x1000000, "l7a1045", 0 ) /* Sound Samples */
	ROM_LOAD( "003-sd01a.77", 0x0000000, 0x400000, CRC(c43898ff) SHA1(0e49b87181b56c62a674d255d326f761942b99b1) )
	ROM_LOAD( "003-sd02a.78", 0x0400000, 0x400000, CRC(079a3d5a) SHA1(a97b052de69fee7d605cae30f5a228e6ffeabb26) )
	ROM_LOAD( "003-sd03a.79", 0x0800000, 0x400000, CRC(96c0991a) SHA1(01be872b3e307258236fe96a544417dd8a0bc8bd) )
ROM_END


ROM_START( bbust2 )
	HNG64_BIOS

	ROM_REGION32_BE( 0x2000000, "gameprg", 0 )
	ROM_LOAD32_WORD_SWAP( "004-pr01a.81", 0x0000002, 0x400000, CRC(7b836ece) SHA1(7a4a08251f1dd66c368ac203f5a006266e77f73d) )
	ROM_LOAD32_WORD_SWAP( "004-pr02a.82", 0x0000000, 0x400000, CRC(8c55a988) SHA1(d9a61ac3d8550ce0ee6aab374c9f024912163180) )
	ROM_LOAD32_WORD_SWAP( "004-pr03a.83", 0x0800002, 0x400000, CRC(f25a82dd) SHA1(74c0a03021ef424e0b9c3c818be297d2967b3012) )
	ROM_LOAD32_WORD_SWAP( "004-pr04a.84", 0x0800000, 0x400000, CRC(9258312b) SHA1(fabac42c8a033e85d503be56f266f9386adff10b) )

	/* Scroll Characters 8x8x8 / 16x16x8 */
	ROM_REGION( 0x1000000, "scrtile", 0 )
	ROM_LOAD16_BYTE( "004-sc01a.41", 0x0000000, 0x400000, CRC(0b52987e) SHA1(3c7b0ce9416dea8db4cf63431166fcfa7c3bb168) )
	ROM_LOAD16_BYTE( "004-sc02a.42", 0x0000001, 0x400000, CRC(6b55309d) SHA1(87761deed6d842075bbe13abc444ac502274eeba) )
	ROM_LOAD16_BYTE( "004-sc03a.43", 0x0800000, 0x400000, CRC(17302f01) SHA1(5b6a927c520e421aa31b9162d3e47b06069b4bd0) )
	ROM_LOAD16_BYTE( "004-sc04a.44", 0x0800001, 0x400000, CRC(db31d73c) SHA1(8a6847e367e87a081cd1499294935c45f1fb4794) )

	/* Sprite Characters - 8x8x8 / 16x16x8 */
	ROM_REGION( 0x2000000, "sprtile", 0 )
	ROM_LOAD32_BYTE( "004-sp01a.53",0x0000000, 0x400000, CRC(72fe73c3) SHA1(82825705076c40558d414653386e3bf1d0693008) )
	ROM_LOAD32_BYTE( "004-sp02a.54",0x0000001, 0x400000, CRC(1ece1cff) SHA1(78d88e96df979a834b5af091d3feda8b9cd466e0) )
	ROM_LOAD32_BYTE( "004-sp03a.55",0x0000002, 0x400000, CRC(9049ab14) SHA1(0a19ccbd82f000eba19a0b407fa5765db0464cca) )
	ROM_LOAD32_BYTE( "004-sp04a.56",0x0000003, 0x400000, CRC(8f7fb914) SHA1(dd1709881bf1d9e233b4e794c0e2ce28d265f855) )
	ROM_LOAD32_BYTE( "004-sp05a.57",0x1000000, 0x400000, CRC(440ce760) SHA1(f6f256334c32fe7d25448fba73f8966c4c5b1cba) )
	ROM_LOAD32_BYTE( "004-sp06a.58",0x1000001, 0x400000, CRC(fc24d2e5) SHA1(073dcb21ec6cf9c6a81987a54c0e27a2db499341) )
	ROM_LOAD32_BYTE( "004-sp07a.59",0x1000002, 0x400000, CRC(bc580b81) SHA1(c668d0524fdc53c6ba2f3e5120f2dee7ce4279bb) )
	ROM_LOAD32_BYTE( "004-sp08a.60",0x1000003, 0x400000, CRC(d6c69bea) SHA1(24508c0ed0ca135316aec1c8239e8b755070384a) )

	/* Textures
	   NOTE: same roms are at different positions on the board, repeated a total of 4 times as there are 4 rendering units
	   We only use the first copy, as the ROMs are always identical
	*/
	ROM_REGION( 0x1000000, "textures0", 0 )
	ROM_LOAD( "004-tx01a.1", 0x0000000, 0x400000, CRC(12a78a20) SHA1(a5c1c8841cd0cb5efbf7408d908fa10a743e5c6f) )
	ROM_LOAD( "004-tx02a.2", 0x0400000, 0x400000, CRC(a36c6c34) SHA1(3e4ad293b064a7c05aa23447ff5f17010cae2863) )
	ROM_LOAD( "004-tx03a.3", 0x0800000, 0x400000, CRC(f46377c0) SHA1(bfa6fc3ab89599a4443577d18578569ad55774bd) )
	ROM_LOAD( "004-tx04a.4", 0x0c00000, 0x400000, CRC(b5f0ef01) SHA1(646bfb17b9e81aecf8db33d3a021f7769b262eda) )

	ROM_REGION( 0x1000000, "textures1", 0 )
	ROM_LOAD( "004-tx01a.5", 0x0000000, 0x400000, CRC(12a78a20) SHA1(a5c1c8841cd0cb5efbf7408d908fa10a743e5c6f) )
	ROM_LOAD( "004-tx02a.6", 0x0400000, 0x400000, CRC(a36c6c34) SHA1(3e4ad293b064a7c05aa23447ff5f17010cae2863) )
	ROM_LOAD( "004-tx03a.7", 0x0800000, 0x400000, CRC(f46377c0) SHA1(bfa6fc3ab89599a4443577d18578569ad55774bd) )
	ROM_LOAD( "004-tx04a.8", 0x0c00000, 0x400000, CRC(b5f0ef01) SHA1(646bfb17b9e81aecf8db33d3a021f7769b262eda) )

	ROM_REGION( 0x1000000, "textures2", 0 )
	ROM_LOAD( "004-tx01a.9" , 0x0000000, 0x400000, CRC(12a78a20) SHA1(a5c1c8841cd0cb5efbf7408d908fa10a743e5c6f) )
	ROM_LOAD( "004-tx02a.10", 0x0400000, 0x400000, CRC(a36c6c34) SHA1(3e4ad293b064a7c05aa23447ff5f17010cae2863) )
	ROM_LOAD( "004-tx03a.11", 0x0800000, 0x400000, CRC(f46377c0) SHA1(bfa6fc3ab89599a4443577d18578569ad55774bd) )
	ROM_LOAD( "004-tx04a.12", 0x0c00000, 0x400000, CRC(b5f0ef01) SHA1(646bfb17b9e81aecf8db33d3a021f7769b262eda) )

	ROM_REGION( 0x1000000, "textures3", 0 )
	ROM_LOAD( "004-tx01a.13", 0x0000000, 0x400000, CRC(12a78a20) SHA1(a5c1c8841cd0cb5efbf7408d908fa10a743e5c6f) )
	ROM_LOAD( "004-tx02a.14", 0x0400000, 0x400000, CRC(a36c6c34) SHA1(3e4ad293b064a7c05aa23447ff5f17010cae2863) )
	ROM_LOAD( "004-tx03a.15", 0x0800000, 0x400000, CRC(f46377c0) SHA1(bfa6fc3ab89599a4443577d18578569ad55774bd) )
	ROM_LOAD( "004-tx04a.16", 0x0c00000, 0x400000, CRC(b5f0ef01) SHA1(646bfb17b9e81aecf8db33d3a021f7769b262eda) )

	/* X,Y,Z Vertex ROMs */
	ROM_REGION16_LE( 0x0c00000, "verts", 0 )
	ROMX_LOAD( "004-vt01a.17", 0x0000000, 0x400000, CRC(25ebbf9b) SHA1(b7c3fb9ee9cf75824d908e7a94970282f1845d5d), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "004-vt02a.18", 0x0000002, 0x400000, CRC(279fc216) SHA1(eb90cc347745491c1d1b1fb611fd6e227310731c), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "004-vt03a.19", 0x0000004, 0x400000, CRC(e0cf6a42) SHA1(dd09b3d05739cf030c820cd7dbaea2e7262764ab), ROM_GROUPWORD | ROM_SKIP(4) )

	ROM_REGION( 0x1000000, "l7a1045", 0 ) /* Sound Samples */
	ROM_LOAD( "004-sd01a.77", 0x0000000, 0x400000, CRC(2ef868bd) SHA1(0a1ef002efe6738698ebe98a1c3695b151fdd282) )
	ROM_LOAD( "004-sd02a.78", 0x0400000, 0x400000, CRC(07fb3135) SHA1(56cc8e29ba9b13f82a4c9248bff02e2b7a0c49b0) )
	ROM_LOAD( "004-sd03a.79", 0x0800000, 0x400000, CRC(42571f1d) SHA1(425cbd3f7c8aea1c0f057ea8f186acffb0091dc0) )
ROM_END


ROM_START( sams64_2 )
	HNG64_BIOS

	ROM_REGION32_BE( 0x2000000, "gameprg", 0 )
	ROM_LOAD32_WORD_SWAP( "005pr01a.81", 0x0000002, 0x400000, CRC(a69d7700) SHA1(a580783a109bc3e24248d70bcd67f62dd7d8a5dd) )
	ROM_LOAD32_WORD_SWAP( "005pr02a.82", 0x0000000, 0x400000, CRC(38b9e6b3) SHA1(d1dad8247d920cc66854a0096e1c7845842d2e1c) )
	ROM_LOAD32_WORD_SWAP( "005pr03a.83", 0x0800002, 0x400000, CRC(0bc738a8) SHA1(79893b0e1c4a31e02ab385c4382684245975ae8f) )
	ROM_LOAD32_WORD_SWAP( "005pr04a.84", 0x0800000, 0x400000, CRC(6b504852) SHA1(fcdcab432162542d249818a6cd15b8f2e8230f97) )
	ROM_LOAD32_WORD_SWAP( "005pr05a.85", 0x1000002, 0x400000, CRC(32a743d3) SHA1(4088b930a1a4d6224a0939ef3942af1bf605cdb5) )
	ROM_LOAD32_WORD_SWAP( "005pr06a.86", 0x1000000, 0x400000, CRC(c09fa615) SHA1(697d6769c16b3c8f73a6df4a1e268ec40cb30d51) )
	ROM_LOAD32_WORD_SWAP( "005pr07a.87", 0x1800002, 0x400000, CRC(44286ad3) SHA1(1f890c74c0da0d34940a880468e68f7fb1417813) )
	ROM_LOAD32_WORD_SWAP( "005pr08a.88", 0x1800000, 0x400000, CRC(d094eb67) SHA1(3edc8d608c631a05223e1d05157cd3daf2d6597a) )

	/* Scroll Characters 8x8x8 / 16x16x8 */
	ROM_REGION( 0x4000000, "scrtile", 0 )
	ROM_LOAD16_BYTE( "005sc01a.97",  0x0000000, 0x800000, CRC(7f11cda9) SHA1(5fbdabd8423e9723a6ec38f8503e6ca7f4f69fdd) )
	ROM_LOAD16_BYTE( "005sc02a.99",  0x0000001, 0x800000, CRC(87d1e1a7) SHA1(00f2ef46ce64ab715add8cd47745c57944286f81) )
	ROM_LOAD16_BYTE( "005sc05a.98",  0x1000000, 0x800000, CRC(4475a3f8) SHA1(f099baf766ee00d166cfa8402baa0b6ea25a0010) )
	ROM_LOAD16_BYTE( "005sc06a.100", 0x1000001, 0x800000, CRC(41c0fbbd) SHA1(1d9ac01c9499a6202ee59d15d498ec34edc05888) )
	ROM_LOAD16_BYTE( "005sc03a.101", 0x2000000, 0x800000, CRC(a5d4c535) SHA1(089a3cd07701f025024ce73b7b4d38063c33a59f) )
	ROM_LOAD16_BYTE( "005sc04a.103", 0x2000001, 0x800000, CRC(14930d77) SHA1(b4c613a8896e21fe2cac0595dd1ea30dc7fce0bd) )
	ROM_LOAD16_BYTE( "005sc07a.102", 0x3000000, 0x800000, CRC(3505b198) SHA1(2fdfdd5a1f6f31f5fb1c0af70047108d1df44af2) )
	ROM_LOAD16_BYTE( "005sc08a.104", 0x3000001, 0x800000, CRC(3139e413) SHA1(38210541379ddeba8c0b9ef8fa5430c0090db7c7) )

	/* Sprite Characters - 8x8x8 / 16x16x8 */
	ROM_REGION( 0x4000000, "sprtile", 0 )
	ROM_LOAD32_BYTE( "005sp01a.105",0x0000000, 0x800000, CRC(68eefee5) SHA1(d95bd7b549900500633af07544423b0062ac07ce) )
	ROM_LOAD32_BYTE( "005sp02a.109",0x0000001, 0x800000, CRC(5d9a49b9) SHA1(50768c496a3e0b4379e121349f32edec4f18652f) )
	ROM_LOAD32_BYTE( "005sp03a.113",0x0000002, 0x800000, CRC(9b6530fe) SHA1(398433b98578a6b4b950afc4d6318916376e0760) )
	ROM_LOAD32_BYTE( "005sp04a.117",0x0000003, 0x800000, CRC(d4e422ce) SHA1(9bfaa533ab3d014cdb0c535cf6952e01925cc30b) )
	ROM_LOAD32_BYTE( "005sp05a.106",0x2000000, 0x400000, CRC(d8b1fb26) SHA1(7da767d8e817c52afc416ccfe8caf30f66c233ef) )
	ROM_LOAD32_BYTE( "005sp06a.110",0x2000001, 0x400000, CRC(87ed72a0) SHA1(0d7db4dc9f15a0377a83f020ffbe81621ca77cff) )
	ROM_LOAD32_BYTE( "005sp07a.114",0x2000002, 0x400000, CRC(8eb3c173) SHA1(d5763c19a3e2fd93f7784d957e7401c9152c40de) )
	ROM_LOAD32_BYTE( "005sp08a.118",0x2000003, 0x400000, CRC(05486fbc) SHA1(747d9ae03ce999be4ab697753e93c90ea85b7d44) )

	/* Textures
	   NOTE: same roms are at different positions on the board, repeated a total of 4 times as there are 4 rendering units
	   We only use the first copy, as the ROMs are always identical
	*/
	ROM_REGION( 0x1000000, "textures0", 0 )
	ROM_LOAD( "005tx01a.1", 0x0000000, 0x400000, CRC(05a4ceb7) SHA1(2dfc46a70c0a957ed0931a4c4df90c341aafff70) )
	ROM_LOAD( "005tx02a.2", 0x0400000, 0x400000, CRC(b7094c69) SHA1(aed9a624166f6f1a2eb4e746c61f9f46f1929283) )
	ROM_LOAD( "005tx03a.3", 0x0800000, 0x400000, CRC(34764891) SHA1(cd6ea663ae28b7f6ac1ede2f9922afbb35b915b4) )
	ROM_LOAD( "005tx04a.4", 0x0c00000, 0x400000, CRC(6be50882) SHA1(1f99717cfa69076b258a0c52d66be007fd820374) )

	ROM_REGION( 0x1000000, "textures1", 0 )
	ROM_LOAD( "005tx01a.5", 0x0000000, 0x400000, CRC(05a4ceb7) SHA1(2dfc46a70c0a957ed0931a4c4df90c341aafff70) )
	ROM_LOAD( "005tx02a.6", 0x0400000, 0x400000, CRC(b7094c69) SHA1(aed9a624166f6f1a2eb4e746c61f9f46f1929283) )
	ROM_LOAD( "005tx03a.7", 0x0800000, 0x400000, CRC(34764891) SHA1(cd6ea663ae28b7f6ac1ede2f9922afbb35b915b4) )
	ROM_LOAD( "005tx04a.8", 0x0c00000, 0x400000, CRC(6be50882) SHA1(1f99717cfa69076b258a0c52d66be007fd820374) )

	ROM_REGION( 0x1000000, "textures2", 0 )
	ROM_LOAD( "005tx01a.9", 0x0000000, 0x400000, CRC(05a4ceb7) SHA1(2dfc46a70c0a957ed0931a4c4df90c341aafff70) )
	ROM_LOAD( "005tx02a.10",0x0400000, 0x400000, CRC(b7094c69) SHA1(aed9a624166f6f1a2eb4e746c61f9f46f1929283) )
	ROM_LOAD( "005tx03a.11",0x0800000, 0x400000, CRC(34764891) SHA1(cd6ea663ae28b7f6ac1ede2f9922afbb35b915b4) )
	ROM_LOAD( "005tx04a.12",0x0c00000, 0x400000, CRC(6be50882) SHA1(1f99717cfa69076b258a0c52d66be007fd820374) )

	ROM_REGION( 0x1000000, "textures3", 0 )
	ROM_LOAD( "005tx01a.13",0x0000000, 0x400000, CRC(05a4ceb7) SHA1(2dfc46a70c0a957ed0931a4c4df90c341aafff70) )
	ROM_LOAD( "005tx02a.14",0x0400000, 0x400000, CRC(b7094c69) SHA1(aed9a624166f6f1a2eb4e746c61f9f46f1929283) )
	ROM_LOAD( "005tx03a.15",0x0800000, 0x400000, CRC(34764891) SHA1(cd6ea663ae28b7f6ac1ede2f9922afbb35b915b4) )
	ROM_LOAD( "005tx04a.16",0x0c00000, 0x400000, CRC(6be50882) SHA1(1f99717cfa69076b258a0c52d66be007fd820374) )

	/* X,Y,Z Vertex ROMs */
	ROM_REGION16_LE( 0x1800000, "verts", 0 )
	ROMX_LOAD( "005vt01a.17", 0x0000000, 0x400000, CRC(48a61479) SHA1(ef982b1ecc6dfca2ad989391afcc1b3d1e7fe652), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "005vt02a.18", 0x0000002, 0x400000, CRC(ba9100c8) SHA1(f7704fb8e5310ea7d0e6ae6b8935717ec9119b6d), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "005vt03a.19", 0x0000004, 0x400000, CRC(f54a28de) SHA1(c445cf7fee71a516065cf37e05b898208f48b17e), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "005vt04a.20", 0x0c00000, 0x400000, CRC(57ad79c7) SHA1(bc382317323c1f8a31b69ae3100d3bba6b5d0838), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "005vt05a.21", 0x0c00002, 0x400000, CRC(49c82bec) SHA1(09255279edb9a204bbe1cce8cef58d5c81e86d1f), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "005vt06a.22", 0x0c00004, 0x400000, CRC(7ba05b6c) SHA1(729c1d182d74998dd904b587a2405f55af9825e0), ROM_GROUPWORD | ROM_SKIP(4) )

	ROM_REGION( 0x1000000, "l7a1045", 0 ) /* Sound Samples */
	ROM_LOAD( "005sd01a.77", 0x0000000, 0x400000, CRC(8f68150f) SHA1(a1e5efdfd1ed29f81e25c8da669851ddb7b0c826) )
	ROM_LOAD( "005sd02a.78", 0x0400000, 0x400000, CRC(6b4da6a0) SHA1(8606c413c129635bdaaa37254edbfd19b10426bb) )
	ROM_LOAD( "005sd03a.79", 0x0800000, 0x400000, CRC(a529fab3) SHA1(8559d402c8f66f638590b8b57ec9efa775010c96) )
	ROM_LOAD( "005sd04a.80", 0x0c00000, 0x400000, CRC(dca95ead) SHA1(39afdfba0e5262b524f25706a96be00e5d14548e) )
ROM_END


ROM_START( fatfurwa )
	HNG64_BIOS

	ROM_REGION32_BE( 0x2000000, "gameprg", 0 )
	ROM_LOAD32_WORD_SWAP( "006pr01a.81", 0x0000002, 0x400000, CRC(3830efa1) SHA1(9d8c941ccb6cbe8d138499cf9d335db4ac7a9ec0) )
	ROM_LOAD32_WORD_SWAP( "006pr02a.82", 0x0000000, 0x400000, CRC(8d5de84e) SHA1(e3ae014263f370c2836f62ab323f1560cb3a9cf0) )
	ROM_LOAD32_WORD_SWAP( "006pr03a.83", 0x0800002, 0x400000, CRC(c811b458) SHA1(7d94e0df501fb086b2e5cf08905d7a3adc2c6472) )
	ROM_LOAD32_WORD_SWAP( "006pr04a.84", 0x0800000, 0x400000, CRC(de708d6c) SHA1(2c9848e7bbf61c574370f9ecab5f5a6ba63339fd) )

	/* Scroll Characters 8x8x8 / 16x16x8 */
	ROM_REGION( 0x4000000, "scrtile", 0 )
	ROM_LOAD16_BYTE( "006sc01a.97", 0x0000000, 0x800000, CRC(f13dffad) SHA1(86363aeae176fd4204e446c13a028da919dc2069) )
	ROM_LOAD16_BYTE( "006sc02a.99", 0x0000001, 0x800000, CRC(be79d42a) SHA1(f3eb950a62e2df1de116af9434027439f1305e1f) )
	ROM_LOAD16_BYTE( "006sc05a.98", 0x1000000, 0x800000, CRC(0487297b) SHA1(d3fa4d691559327739c96717312faf09b498001d) )
	ROM_LOAD16_BYTE( "006sc06a.100",0x1000001, 0x800000, CRC(34a76c31) SHA1(be05dc75afb7cde65ba5d29c0e66a7b1b62c41cb) )
	ROM_LOAD16_BYTE( "006sc03a.101",0x2000000, 0x800000, CRC(16918b73) SHA1(ad0c751a301fe3c95fca19473869dfd55fb6b0de) )
	ROM_LOAD16_BYTE( "006sc04a.103",0x2000001, 0x800000, CRC(9b63cd98) SHA1(62519a3a531c4493a5a85dc01ca69413977120ca) )
	ROM_LOAD16_BYTE( "006sc07a.102",0x3000000, 0x800000, CRC(7a1c371e) SHA1(1cd4ad66dd007adc9ab0c29720cbf9955c7337e0) )
	ROM_LOAD16_BYTE( "006sc08a.104",0x3000001, 0x800000, CRC(88232ade) SHA1(4ae2a572c3525087f77c95185e8697a1fc720512) )

	/* Sprite Characters - 8x8x8 / 16x16x8 */
	ROM_REGION( 0x4000000, "sprtile", 0 )
	ROM_LOAD32_BYTE( "006sp01a.105",0x0000000, 0x800000, CRC(087b8c49) SHA1(bb1eb2baef7da91f904bf45414f21dd6bac30749) )
	ROM_LOAD32_BYTE( "006sp02a.109",0x0000001, 0x800000, CRC(da28631e) SHA1(ea7e2d9195cfa4f954f4d542296eec1323223653) )
	ROM_LOAD32_BYTE( "006sp03a.113",0x0000002, 0x800000, CRC(bb87b55b) SHA1(8644ebb356ae158244a6e03254b0212cb359e167) )
	ROM_LOAD32_BYTE( "006sp04a.117",0x0000003, 0x800000, CRC(2367a536) SHA1(304b5b7f7e5d41e69fbd4ac2a938c42f3766630e) )
	ROM_LOAD32_BYTE( "006sp05a.106",0x2000000, 0x800000, CRC(0eb8fd06) SHA1(c2b6fab1b0104910d7bb39d0a496ada39c5cc122) )
	ROM_LOAD32_BYTE( "006sp06a.110",0x2000001, 0x800000, CRC(dccc3f75) SHA1(fef8d259c17a78e2266fed965fba1e15f1cd01dd) )
	ROM_LOAD32_BYTE( "006sp07a.114",0x2000002, 0x800000, CRC(cd7baa1b) SHA1(4084f3a73aae623d69bd9de87cecf4a33b628b7f) )
	ROM_LOAD32_BYTE( "006sp08a.118",0x2000003, 0x800000, CRC(9c3044ac) SHA1(24b28bcc6be51ab3ff59c2894094cd03ec377d84) )

	/* Textures
	   NOTE: same roms are at different positions on the board, repeated a total of 4 times as there are 4 rendering units
	   We only use the first copy, as the ROMs are always identical
	*/
	ROM_REGION( 0x1000000, "textures0", 0 )
	ROM_LOAD( "006tx01a.1", 0x0000000, 0x400000, CRC(ab4c1747) SHA1(2c097bd38f1a92c4b6534992f6bf29fd6dc2d265) )
	ROM_LOAD( "006tx02a.2", 0x0400000, 0x400000, CRC(7854a229) SHA1(dba23c1b793dd0308ac1088c819543fff334a57e) )
	ROM_LOAD( "006tx03a.3", 0x0800000, 0x400000, CRC(94edfbd1) SHA1(d4004bb1273e6091608856cb4b151e9d81d5ed30) )
	ROM_LOAD( "006tx04a.4", 0x0c00000, 0x400000, CRC(82d61652) SHA1(28303ae9e2545a4cb0b5843f9e73407754f41e9e) )

	ROM_REGION( 0x1000000, "textures1", 0 )
	ROM_LOAD( "006tx01a.5", 0x0000000, 0x400000, CRC(ab4c1747) SHA1(2c097bd38f1a92c4b6534992f6bf29fd6dc2d265) )
	ROM_LOAD( "006tx02a.6", 0x0400000, 0x400000, CRC(7854a229) SHA1(dba23c1b793dd0308ac1088c819543fff334a57e) )
	ROM_LOAD( "006tx03a.7", 0x0800000, 0x400000, CRC(94edfbd1) SHA1(d4004bb1273e6091608856cb4b151e9d81d5ed30) )
	ROM_LOAD( "006tx04a.8", 0x0c00000, 0x400000, CRC(82d61652) SHA1(28303ae9e2545a4cb0b5843f9e73407754f41e9e) )

	ROM_REGION( 0x1000000, "textures2", 0 )
	ROM_LOAD( "006tx01a.9", 0x0000000, 0x400000, CRC(ab4c1747) SHA1(2c097bd38f1a92c4b6534992f6bf29fd6dc2d265) )
	ROM_LOAD( "006tx02a.10",0x0400000, 0x400000, CRC(7854a229) SHA1(dba23c1b793dd0308ac1088c819543fff334a57e) )
	ROM_LOAD( "006tx03a.11",0x0800000, 0x400000, CRC(94edfbd1) SHA1(d4004bb1273e6091608856cb4b151e9d81d5ed30) )
	ROM_LOAD( "006tx04a.12",0x0c00000, 0x400000, CRC(82d61652) SHA1(28303ae9e2545a4cb0b5843f9e73407754f41e9e) )

	ROM_REGION( 0x1000000, "textures3", 0 )
	ROM_LOAD( "006tx01a.13",0x0000000, 0x400000, CRC(ab4c1747) SHA1(2c097bd38f1a92c4b6534992f6bf29fd6dc2d265) )
	ROM_LOAD( "006tx02a.14",0x0400000, 0x400000, CRC(7854a229) SHA1(dba23c1b793dd0308ac1088c819543fff334a57e) )
	ROM_LOAD( "006tx03a.15",0x0800000, 0x400000, CRC(94edfbd1) SHA1(d4004bb1273e6091608856cb4b151e9d81d5ed30) )
	ROM_LOAD( "006tx04a.16",0x0c00000, 0x400000, CRC(82d61652) SHA1(28303ae9e2545a4cb0b5843f9e73407754f41e9e) )

	/* X,Y,Z Vertex ROMs */
	ROM_REGION16_LE( 0x0c00000, "verts", 0 )
	ROMX_LOAD( "006vt01a.17", 0x0000000, 0x400000, CRC(5c20ed4c) SHA1(df679f518292d70b9f23d2bddabf975d56b96910), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "006vt02a.18", 0x0000002, 0x400000, CRC(150eb717) SHA1(9acb067346eb386256047c0f1d24dc8fcc2118ca), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "006vt03a.19", 0x0000004, 0x400000, CRC(021cfcaf) SHA1(fb8b5f50d3490b31f0a4c3e6d3ae1b98bae41c97), ROM_GROUPWORD | ROM_SKIP(4) )

	ROM_REGION( 0x1000000, "l7a1045", 0 ) /* Sound Samples */
	ROM_LOAD( "006sd01a.77", 0x0000000, 0x400000, CRC(790efb6d) SHA1(23ddd3ee8ae808e58cbcaf92a9ef56d3ca6289b5) )
	ROM_LOAD( "006sd02a.78", 0x0400000, 0x400000, CRC(f7f020c7) SHA1(b72fde4ff6384b80166a3cb67d31bf7afda750bc) )
	ROM_LOAD( "006sd03a.79", 0x0800000, 0x400000, CRC(1a678084) SHA1(f52efb6145102d289f332d8341d89a5d231ba003) )
	ROM_LOAD( "006sd04a.80", 0x0c00000, 0x400000, CRC(3c280a5c) SHA1(9d3fc78e18de45382878268db47ff9d9716f1505) )

	/* this game does not initialize EEPROM automatically otherwise (each region requires different defaults) */
	ROM_REGION( 0x4000, "nvram", 0 )
	ROMX_LOAD( "default_nvram_japan",  0x00000, 0x4000, CRC(1f618d44) SHA1(c007c5f94b28b8c56c8c539d2f82336515c0ed84), ROM_BIOS(0) )
	ROMX_LOAD( "default_nvram_usa",    0x00000, 0x4000, CRC(f139f4a5) SHA1(6f7e2fc5d902c1499f3c55f9ca2ef7becc49103b), ROM_BIOS(1) )
	ROMX_LOAD( "default_nvram_others", 0x00000, 0x4000, CRC(bf1c3e4a) SHA1(454c6e5e505293bfdeb87d08e72420bba84c3b7b), ROM_BIOS(2) )
	ROMX_LOAD( "default_nvram_korea",  0x00000, 0x4000, CRC(e8fb68df) SHA1(3170e7465b93319c0550f35f8906b5bdc5332eec), ROM_BIOS(3) )
ROM_END


ROM_START( buriki )
	HNG64_BIOS

	ROM_REGION32_BE( 0x2000000, "gameprg", 0 )
	ROM_LOAD32_WORD_SWAP( "007pr01b.81", 0x0000002, 0x400000, CRC(a31202f5) SHA1(c657729b292d394ced021a0201a1c5608a7118ba) )
	ROM_LOAD32_WORD_SWAP( "007pr02b.82", 0x0000000, 0x400000, CRC(a563fed6) SHA1(9af9a021beb814e35df968abe5a99225a124b5eb) )
	ROM_LOAD32_WORD_SWAP( "007pr03a.83", 0x0800002, 0x400000, CRC(da5f6105) SHA1(5424cf5289cef66e301e968b4394e551918fe99b) )
	ROM_LOAD32_WORD_SWAP( "007pr04a.84", 0x0800000, 0x400000, CRC(befc7bce) SHA1(83d9ecf944e03a40cf25ee288077c2265d6a588a) )
	ROM_LOAD32_WORD_SWAP( "007pr05a.85", 0x1000002, 0x400000, CRC(013e28bc) SHA1(45e5ac45b42b26957c2877ac1042472c4b5ec914) )
	ROM_LOAD32_WORD_SWAP( "007pr06a.86", 0x1000000, 0x400000, CRC(0620fccc) SHA1(e0bffc56b019c79276a4ef5ec7354edda15b0889) )

	/* Scroll Characters 8x8x8 / 16x16x8 */
	ROM_REGION( 0x4000000, "scrtile", 0 )
	ROM_LOAD16_BYTE( "007sc01a.97", 0x0000000, 0x800000, CRC(4e8300db) SHA1(f1c9e6fddc10efc8f2a530027cca062f48b8c8d4) )
	ROM_LOAD16_BYTE( "007sc02a.99", 0x0000001, 0x800000, CRC(d5855944) SHA1(019c0bd2f8de7ffddd53df6581b40940262f0053) )
	ROM_LOAD16_BYTE( "007sc05a.98", 0x1000000, 0x400000, CRC(27f848c1) SHA1(2ee9cca4e68e56c7c17c8e2d7e0f55a34a5960bd) )
	ROM_LOAD16_BYTE( "007sc06a.100",0x1000001, 0x400000, CRC(c39e9b4c) SHA1(3c8a0494c2a6866ecc0df2c551619c57ee072440) )
	ROM_LOAD16_BYTE( "007sc03a.101",0x2000000, 0x800000, CRC(ff45c9b5) SHA1(ddcc2a10ccac62eb1f3671172ad1a4d163714fca) )
	ROM_LOAD16_BYTE( "007sc04a.103",0x2000001, 0x800000, CRC(e4cb59e9) SHA1(4e07ff374890217466a53d5bfb1fa99eb7402360) )
	ROM_LOAD16_BYTE( "007sc07a.102",0x3000000, 0x400000, CRC(753e7e3d) SHA1(39b2e9fd23878d8fc4f98fe88b466e963d8fc959) )
	ROM_LOAD16_BYTE( "007sc08a.104",0x3000001, 0x400000, CRC(b605928e) SHA1(558042b84115273fa581606daafba0e9688fa002) )

	/* Sprite Characters - 8x8x8 / 16x16x8 */
	ROM_REGION( 0x4000000, "sprtile", 0 )
	ROM_LOAD32_BYTE( "007sp01a.105",0x0000000, 0x800000, CRC(160acae6) SHA1(37c15e1d2544ec6f3b61d06200345d6abdd28edf) )
	ROM_LOAD32_BYTE( "007sp02a.109",0x0000001, 0x800000, CRC(1a55331d) SHA1(0b03d5c7312e01874365b31f1ff3d9766abd00f1) )
	ROM_LOAD32_BYTE( "007sp03a.113",0x0000002, 0x800000, CRC(3f308444) SHA1(0acd52312c15a2ed3bacf60a2fd820cb09ebbb55) )
	ROM_LOAD32_BYTE( "007sp04a.117",0x0000003, 0x800000, CRC(6b81aa51) SHA1(55f7702e1d7a2bef7f050d0358de9036a0139877) )
	ROM_LOAD32_BYTE( "007sp05a.106",0x2000000, 0x400000, CRC(32d2fa41) SHA1(b16a0bbd397be2a8d532c85951b924e2e086a189) )
	ROM_LOAD32_BYTE( "007sp06a.110",0x2000001, 0x400000, CRC(b6f8d7f3) SHA1(70ce94f2193ee39218022da617413c42f6753574) )
	ROM_LOAD32_BYTE( "007sp07a.114",0x2000002, 0x400000, CRC(5caa1cc9) SHA1(3e40b10ea3bcf1239d0015da4be869632b805ddd) )
	ROM_LOAD32_BYTE( "007sp08a.118",0x2000003, 0x400000, CRC(7a158c67) SHA1(d66f4920a513208d45b908a1934d9afb894debf1) )

	/* Textures
	   NOTE: same roms are at different positions on the board, repeated a total of 4 times as there are 4 rendering units
	   We only use the first copy, as the ROMs are always identical
	*/
	ROM_REGION( 0x1000000, "textures0", 0 )
	ROM_LOAD( "007tx01a.1", 0x0000000, 0x400000, CRC(a7774075) SHA1(4f3da9af131a7efb0f0a5180da57c19c65fffb82) )
	ROM_LOAD( "007tx02a.2", 0x0400000, 0x400000, CRC(bc05d5fd) SHA1(84e3fafcebdeb1e2ffae80785949c973a14055d8) )
	ROM_LOAD( "007tx03a.3", 0x0800000, 0x400000, CRC(da9484fb) SHA1(f54b669a66400df00bf25436e5fd5c9bf68dbd55) )
	ROM_LOAD( "007tx04a.4", 0x0c00000, 0x400000, CRC(02aa3f46) SHA1(1fca89c70586f8ebcdf669ecac121afa5cdf623f) )

	ROM_REGION( 0x1000000, "textures1", 0 )
	ROM_LOAD( "007tx01a.5", 0x0000000, 0x400000, CRC(a7774075) SHA1(4f3da9af131a7efb0f0a5180da57c19c65fffb82) )
	ROM_LOAD( "007tx02a.6", 0x0400000, 0x400000, CRC(bc05d5fd) SHA1(84e3fafcebdeb1e2ffae80785949c973a14055d8) )
	ROM_LOAD( "007tx03a.7", 0x0800000, 0x400000, CRC(da9484fb) SHA1(f54b669a66400df00bf25436e5fd5c9bf68dbd55) )
	ROM_LOAD( "007tx04a.8", 0x0c00000, 0x400000, CRC(02aa3f46) SHA1(1fca89c70586f8ebcdf669ecac121afa5cdf623f) )

	ROM_REGION( 0x1000000, "textures2", 0 )
	ROM_LOAD( "007tx01a.9", 0x0000000, 0x400000, CRC(a7774075) SHA1(4f3da9af131a7efb0f0a5180da57c19c65fffb82) )
	ROM_LOAD( "007tx02a.10",0x0400000, 0x400000, CRC(bc05d5fd) SHA1(84e3fafcebdeb1e2ffae80785949c973a14055d8) )
	ROM_LOAD( "007tx03a.11",0x0800000, 0x400000, CRC(da9484fb) SHA1(f54b669a66400df00bf25436e5fd5c9bf68dbd55) )
	ROM_LOAD( "007tx04a.12",0x0c00000, 0x400000, CRC(02aa3f46) SHA1(1fca89c70586f8ebcdf669ecac121afa5cdf623f) )

	ROM_REGION( 0x1000000, "textures3", 0 )
	ROM_LOAD( "007tx01a.13",0x0000000, 0x400000, CRC(a7774075) SHA1(4f3da9af131a7efb0f0a5180da57c19c65fffb82) )
	ROM_LOAD( "007tx02a.14",0x0400000, 0x400000, CRC(bc05d5fd) SHA1(84e3fafcebdeb1e2ffae80785949c973a14055d8) )
	ROM_LOAD( "007tx03a.15",0x0800000, 0x400000, CRC(da9484fb) SHA1(f54b669a66400df00bf25436e5fd5c9bf68dbd55) )
	ROM_LOAD( "007tx04a.16",0x0c00000, 0x400000, CRC(02aa3f46) SHA1(1fca89c70586f8ebcdf669ecac121afa5cdf623f) )

	/* X,Y,Z Vertex ROMs */
	ROM_REGION16_LE( 0x0c00000, "verts", 0 )
	ROMX_LOAD( "007vt01a.17", 0x0000000, 0x400000, CRC(f78a0376) SHA1(fde4ddd4bf326ae5f1ed10311c237b13b62e060c), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "007vt02a.18", 0x0000002, 0x400000, CRC(f365f608) SHA1(035fd9b829b7720c4aee6fdf204c080e6157994f), ROM_GROUPWORD | ROM_SKIP(4) )
	ROMX_LOAD( "007vt03a.19", 0x0000004, 0x400000, CRC(ba05654d) SHA1(b7fe532732c0af7860c8eded3c5abd304d74e08e), ROM_GROUPWORD | ROM_SKIP(4) )

	ROM_REGION( 0x1000000, "l7a1045", 0 ) /* Sound Samples */
	ROM_LOAD( "007sd01a.77", 0x0000000, 0x400000, CRC(1afb48c6) SHA1(b072d4fe72d6c5267864818d300b32e85b426213) )
	ROM_LOAD( "007sd02a.78", 0x0400000, 0x400000, CRC(c65f1dd5) SHA1(7f504c585a10c1090dbd1ac31a3a0db920c992a0) )
	ROM_LOAD( "007sd03a.79", 0x0800000, 0x400000, CRC(356f25c8) SHA1(5250865900894232960686f40c5da35b3868b78c) )
	ROM_LOAD( "007sd04a.80", 0x0c00000, 0x400000, CRC(dabfbbad) SHA1(7d58d5181705618e0e2d69c6fdb81b9b3d2b9e0f) )
ROM_END

/* BIOS */
GAME( 1997, hng64,    0,     hng64_default, hng64,          hng64_state, init_hng64,       ROT0, "SNK",       "Hyper NeoGeo 64 BIOS", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_IS_BIOS_ROOT )

/* Games */
GAME( 1997, roadedge, hng64, hng64_drive,   hng64_drive,    hng64_state, init_roadedge,    ROT0, "SNK",       "Roads Edge / Round Trip RV (rev.B)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NODEVICE_LAN )  /* 001 */
GAME( 1998, sams64,   hng64, hng64_fight,   hng64_fight,    hng64_state, init_ss64,        ROT0, "SNK",       "Samurai Shodown 64 / Samurai Spirits / Paewang Jeonseol 64", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) /* 002 */
GAME( 1998, xrally,   hng64, hng64_drive,   hng64_drive,    hng64_state, init_hng64_drive, ROT0, "SNK",       "Xtreme Rally / Off Beat Racer!", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NODEVICE_LAN )  /* 003 */
GAME( 1998, bbust2,   hng64, hng64_shoot,   hng64_shoot,    hng64_state, init_hng64_shoot, ROT0, "SNK / ADK", "Beast Busters: Second Nightmare", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )  /* 004 */ // ADK credited in the ending sequence
GAME( 1998, sams64_2, hng64, hng64_fight,   hng64_fight,    hng64_state, init_ss64,        ROT0, "SNK",       "Samurai Shodown 64: Warriors Rage / Samurai Spirits 2: Asura Zanmaden", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) /* 005 */
GAME( 1998, fatfurwa, hng64, hng64_fight,   hng64_fight,    hng64_state, init_hng64_fght,  ROT0, "SNK",       "Fatal Fury: Wild Ambition / Garou Densetsu: Wild Ambition (rev.A)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )  /* 006 */
GAME( 1999, buriki,   hng64, hng64_fight,   hng64_fight,    hng64_state, init_hng64_fght,  ROT0, "SNK",       "Buriki One: World Grapple Tournament '99 in Tokyo (rev.B)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )  /* 007 */
