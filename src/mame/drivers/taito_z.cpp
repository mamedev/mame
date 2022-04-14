// license:BSD-3-Clause
// copyright-holders:David Graves
// thanks-to:Richard Bush
/***************************************************************************

Taito Z System [twin 68K with optional Z80]
-------------------------------------------

David Graves

(this is based on the F2 driver by Bryan McPhail, Brad Oliver, Andrew Prime,
Nicola Salmoria. Thanks to Richard Bush and the Raine team, whose open
source was very helpful in many areas particularly the sprites.)


The Taito Z system has a number of similarities with the Taito F2 system,
and uses some of the same custom Taito components.

Taito Z supports 5 separate layers of graphics - one 64x64 tiled scrolling
background plane of 8x8 tiles, a similar foreground plane, another optional
plane used for drawing a road (e.g. Chasehq), a sprite plane [with varying
properties], and a text plane with character definitions held in ram.

(Double Axle has four rather than two background planes, and they contain
32x32 16x16 tiles. This is because it uses a TC0480SCP rather than the
older TC0100SCN tilemap generator used in previous Taito Z games. The
hardware for Taito's Super Chase was a further development of this, with a
68020 for main CPU and Ensoniq sound - standard features of Taito's F3
system. Taito's F3 system superseded both Taito B and F2 systems, but the
Taito Z system was enhanced with F3 features and continued in games like
Super Chase and Under Fire up to the mid 1990s.)

Each Taito Z game used one of the following sprite systems - allowing the
use of big sprites with minimal CPU overhead [*]:

(i)  16x8 tiles aggregated through a spritemap rom into 128x128 sprites
(ii) 16x16 tiles aggregated through a spritemap rom into three sprite sizes:
      128 x 128
       64 x 128
       32 x 128
(iii) 16x8 tiles aggregated through a spritemap rom into 64x64 sprites

[* in Taito B/F2/F3 the CPU has to keep track of all the 16x16 tiles within
a big sprite]

The Z system has twin 68K CPUs which communicate via shared ram.
Typically they share $4000 bytes, but Spacegun / Dbleaxle share $10000.

The first 68000 handles screen, palette and sprites, and sometimes other
jobs [e.g. inputs; in one game it also handles the road].

The second 68000 may handle functions such as:
    (i)  inputs/dips, sound (through a YM2610) and/or
    (ii) the "road" that's in every TaitoZ game except Spacegun.

Most Z system games have a Z80 as well, which takes over sound duties.
Commands are written to it by the one of the 68000s.

The memory map for the Taito Z games is similar in outline but usually
shuffled around: some games have different I/O because of analogue
sticks, light guns, cockpit hardware etc.


****************************************************************************

Contcirc board (B.Troha)
--------------

Taito Sound PCB J1100137A K1100314A:

  Zilog Z0840004PSC     XTAL OSC          Yamaha
  Z80 CPU               16.000 MHz        YM2610

  TC0060DCA              B33-30
  TC0060DCA
                                            TC0140SYT

                                          B33-08
                                          B33-09
                                          B33-10

Notes: B33-30 is a OKI M27512-15


Taito Video Board PCB J1100139A K1100316A:

 B33-03     TC0050VDZ     TC0050VDZ                       TC0050VDZ
 B33-04
 B33-05
 B33-06                      TC0020VAR

     B14-31

 B33-07

                      B14-30

Notes: B14-31 is 27HC64 (Sharp LH5763J-70)
       B14-30 is OKI M27512-15
DG:    TC0020VAR + 3xTC0050VDZ may be precursor to 370MSO/300FLA combo


Taito CPU Board J110138A K1100315A:

                                            XTAL OSC  XTAL OSC
                                            24.000MHz 26.686MHz

                                                 B33-02
B33-01
                  TC0150ROD                 TC0100SCN        NEC D43256C-10L
                                                             NEC D43256C-10L

                                                      TC0110PCR

                                          TC0070RGB
 MC6800P12 IC-25 MC68000P12 IC-35
           IC-26            IC 36           TC0040IOC
                                              DSWA  DSWB

Notes: IC-41 Is 271001 Listed as JH1 (unsocketed / unused)
       IC-42 Is 271001 Listed as JL1 (unsocketed / unused)



****************************************************************************

Aquajack
Taito, 1990

This game runs on Taito Z hardware

Main PCB Layout
---------------

J1100196A
K1100456A
K1100457A AQUA JACK (sticker)
|--------------------------------------------------------------------------|
|B77-17.1 2063  B77-07.33           B77-05.105         DSWA(8)  DSWB(8)    |
|         2063                                        |------|             |
| |---------|  |---------|               |---------|  |TAITO | 2063      |-|
| |  TAITO  |  |  TAITO  |               |  TAITO  |  |TC0110| TC0070RGB |
| |TC0050VDZ|  |TC0150ROD| B77-19.46     |TC0100SCN|  |PCR   | 2063      |-|
| |(QFP100) |  |(QFP160) |               |(QFP160) |  |------|             |
| |         |  |         |               |         |          MB3771       |
| |---------|  |---------|               |---------|  |------|             |
|                                     58257  58257    |TAITO |        (G) 2|
| 2018         |---------|                            |TC0220| TC0060DCA  8|
| 2018         |  TAITO  |   |---------|              |IOC   | TC0060DCA  W|
| 2018         |TC0020VAR|   |  TAITO  |              |------|            A|
| 2018         |(QFP124) |   |TC0320OBR| 2063        TL074    MB3735      Y|
|              |         |   |(QFP144) |             TL074       VOL       |
| 2018         |---------|   |         | B77_20.54   YM3016                |
| 2018         B77-18.37     |---------| Z80         YM2610              |-|
| 2018         B77-06.39                                      MB3735     |
| 2018      |---------|                              |---------| VOL     |-|
|           |  TAITO  |                  16MHz  TL074|  TAITO  |           |
|           |TC0050VDZ|                              |TC0100SYT| B77-15.89 |
| B77-01.13 |(QFP100) |       2018      B77-08.57    |(QFP120) |          |-|
|           |         |       2018      B77-09.58    |         |          | |
| B77-02.14 |---------|      |--------------|        |---------|          | |
|           |---------|      |  MC68000P12  |                          (M)| |
| B77-03.15 |  TAITO  |      |--------------|        2063                 | |
|           |TC0050VDZ|                              2063                 |-|
| B77-04.16 |(QFP100) |   B77-14.60       B77_23.67|---------|             |
|           |         |   B77-13.51       2063     |  TAITO  | B77-16.94   |
|           |---------|  |--------------|          |TC0170ABT|             |
|                        |  MC68000P12  | B77_24.69|(QFP120) |        24MHz|
| B77_25.17   B77_22.31  |--------------| 2063     |         | 26.686MHz   |
| 2063        2063                                 |---------|             |
|--------------------------------------------------------------------------|
Notes:
      68000 - Motorola MC68000P12 CPUs, running at 12.000MHz [24/2]
        Z80 - Zilog Z0840004PSC Z80 CPU, running at 4.000MHz [16/4]
     YM2610 - Yamaha YM2610 sound chip, running at 8.000MHz [16/2]
       2063 - Toshiba TMM2063 8K x8 SRAM (DIP28)
       2018 - Toshiba TMM2018 2K x8 SRAM (DIP24)
      58257 - Sony CXK58257 32K x8 SRAM (DIP28)
     MB3771 - Fujitsu MB3771 System Reset IC (DIP8)
        (G) - 28-Way Connector (Not JAMMA)
        (M) - 50-pin Flat Cable Connector Joining Main PCB To Analog Control PCB

        OSC: 26.686, 24.000, 16.000

     Taito custom ICs -
                       TC0070RGB - RGB/Video Mixer (Ceramic Flat Pack SIP25)
                       TC0060DCA - Digital to Analog Conversion for Audio (Ceramic Flat Pack SIP20)
                       TC0100SYT - Sound Communication
                       TC0220IOC - Input/Output. This chip also provides the master reset via the MB3771. It probably does more things too,
                                   including video output. For example, if the harness is connected backwards, this chip blows and kills
                                   the PCB. Even manually resetting the 68000's cannot restart the PCB, and it just shows a wavey pattern
                                   on screen.
                       TC0110PCR - Palette Generator
                       TC0100SCN - Tilemap Generator
                       TC0150ROD - Road Generator
                       TC0050VDZ - \ Motion Object Generator Combo?
                       TC0170ABT - /
                       TC0020VAR - ?
                       TC0320OBR - Road Object Generator? (tied to TC0150ROD)

    ROMs -


Analog Control PCB
------------------
J9100175A
K9100227A ADII PCB
K9100227A AQUA JACK (sticker)
|------------------|
|                  |
|    74LS244      |-|
|                 | |
|                 | |
|    ADC0809      | |
|              (M)| |
|                 | |
|    74LS245      | |
|(H)              | |
|                 | |
|    74HC74       | |
|                 |-|
|        4.9152MHz |
|------------------|
Notes:
      All components listed

      (H) - 6 pin connector for attachment of Analog Controls
      (M) - 50-pin Flat Cable Connector For Joining Analog Control PCB to Main PCB
  ADC0809 - Texas Instruments ADC0809N Analog To Digital Convertor IC (DIP28)



****************************************************************************
Guru-Readme for Chase HQ (Taito 1988)

CPU Board
---------

K1100357A
J1100157A CPU PCB
K1100357A CHASE HQ UP (sticker for upright)
M4300099A CHASE HQ DX (sticker for DX cabinet)
|------------------------------------------------------------------------|
|  B52-113.IC73     |---------|                     B52-119.IC20(PAL20L8)|
|                   |TAITO    |           68000-12  B52-118.IC21(PAL20L8)|
|  B52-114.IC72     |TC0140SYT|                                          |-|
|M                  |         |      B52-131.IC37 B52-129.IC30           | |
|  B52-115.IC71     |---------|      B52-130.IC36 B52-136.IC29           | |
|                                       TMM2063     TMM2063              | |
|  B52-116.IC70      Z80                                                 | |
|                    B52-137.IC51         B52-29.IC27         26.686MHz  | |
| TD62003                                  |---------|        24MHz      | |
|V           YM2610                        |TAITO    |                   | |
|                                          |TC0100SCN|      43256        |-|
|             16MHz                        |         |                   |
|                           TMM2063        |---------|      43256        |
|          TC0050DCA                   |---------|                       |-|
|            Y3016-F        TMM2063    |TAITO    | |------|              | |
|                                      |TC0170ABT| |TAITO |              | |
|        TL074 TL074                   |         | |TC0110|   B52-01.IC7 | |
|  MB3735  VOLUME B52-121.IC57(PAL20L8)|---------| |PCR   |              | |
|G    PC050CM     B52-120.IC56(PAL16L8)            |------|              | |
|  |---------|       68000-12       |---------|  B52-06.IC24 B52-28.IC4  | |
|  |TCOO40IOC|    B52-133.IC55      |TAITO    |       TMM2063   TMM2063  | |
|  |---------|        B52-132.IC39  |TC0150ROD|       TMM2063   TMM2063  |-|
| MB3771                            |         |                          |
| DIPSWB DIPSWA   TMM2063  TMM2063  |---------|           TC0070RGB      |
|------------------------------------------------------------------------|
Notes:
      68000 - Motorola MC68000P12 CPUs, clock input 12.000MHz [24/2]
        Z80 - Zilog Z0840004PSC Z80 CPU, clock input 4.000MHz [16/4]
     YM2610 - Yamaha YM2610 sound chip, clock input 8.000MHz [16/2]
    Y3016-F - Yamaha Y3016-F 2-Channel Serial & Binary Input Floating D/A Converter (SOIC16)
              Clock input 2.66666MHz (16/2/3, source = pin 64 of YM2610)
    TMM2063 - Toshiba TMM2063AP-70 8k x8-bit SRAM (DIP28)
      43256 - NEC D43256AC-10L 32k x8-bit SRAM (DIP28)
     MB3771 - Fujitsu MB3771 System Reset IC (DIP8)
     MB3735 - Fujitsu MB3735 20w BTL Mono Power AMP. Note this amp chip has one input and one output, so sound
              is mono. Where two speakers are present (cockpit/DX) the same sounds are output to both speakers
              because the edge connector pins 10+L and 11+M are hard-wired together.
      TL074 - Texas Instruments TL074 JFET Low-Noise Quad OP Amp
    TD62003 - Toshiba TD62003 PNP 50V 0.5A Quad Darlington Switch (for driving CPU-controlled lamps via 2x NEC 2SB1150 PNP Darlington Transistors)
          G - 28-Way Connector (Not JAMMA, but power/ground pins are the same as JAMMA)
          V - 5-pin video connector
          M - 50-pin flat cable connector. Only populated on the moving cabinet version. This is used to connect the motor PCB to the
              main PCB. This connector and some adjacent 74LS244/245 logic chips are not populated on the upright PCB version.

      Syncs - Horizontal: 15.675kHz
              Vertical: 60.0554Hz

       ROMs -
              B52-137.IC51 - Z80 Sound Program

              B52-131.IC37 \
              B52-129.IC30 | Main 68k Program (27C1001 EPROMs)
              B52-130.IC36 |
              B52-136.IC29 /

              B52-133.IC55 \ Sub 68k Program (27C512 EPROMs)
              B52-132.IC39 /

              B52-29.IC27  - Tilemaps (23C4000 mask ROM)

              B52-01.IC7   - Road/Sprite Priority & Palette Select (MMI63S141 BI-POLAR PROM)

              B52-06.IC24  - Road A/B Internal Priority (MMI63S141 BI-POLAR PROM)

              B52-28.IC4   - TC0150ROD ROM for Road data (23C4000 mask ROM)

              Note: These ROMs match the World ROM-set in MAME ('chasehq'). These are the same ROMs found on a Chase HQ DX cabinet
                    board-set which suggests any/all Chase HQ ROM-sets will work with the deluxe motion cabinet providing the Motor
                    PCB is present and working.

Taito custom ICs -
                  TC0070RGB - 5-bit RGB Video Mixer/RGB DAC (Ceramic Flat Pack SIL25)
                  PC050CM   - Coin/Counter/Lockout Functions
                  TC0060DCA - 2-Channel Digital to Analog Conversion for Audio (Ceramic Flat Pack SIL20)
                              Note: 2 channel audio from the YM2610 is input and output separately here but on the output it is
                              merged via 2x 2.7k resistors to a single audio signal and fed to the MB3735 AMP chip on pin 1.
                              The output from the MB3735 is mono.
                  TC0140SYT - Sound Communication
                  TC0040IOC - I/O Controls (start/shift/wheel/accelerator etc) and DIP switch management.
                              This chip provides the master reset output on pin 2.
                              On the upright cabinet, the accelerate pedal is a normally open switch (it is either off or on).
                              The shift lever is a normally open switch (off/on). When open it is set to low gear and when closed
                              (i.e. when pin 20 of connector G is grounded) it is set to high gear.
                              The steering wheel hardware on Chase HQ uses one half of a trackball mechanism (left/right only)
                              which is basically like a spinner. There is a segmented disc with optical sensor and another optical
                              sensor to detect the wheel center position. The wheel returns to the center position with the help of
                              a centering spring. The cockpit & DX cabs use 5k pots for the steering and accelerator, although
                              micro-switches will also work fine with the correct control mechanisms.
                  TC0110PCR - Palette Generator
                  TC0100SCN - Tilemap Generator
                  TC0150ROD - Road Generator
                  TC0170ABT - Motion Object Generator


Edge Connector Pinouts (NOTE! This is correct as per schematics. Pinout found on the internet is not accurate)
----------------------

            COMPONENT  |  SOLDER                       Video Connector (NOTE: when facing the edge connector, pin 5 is on the left)
           ------------+------------                   ---------------
               GND | 1 | A | GND                       1 GROUND
               GND | 2 | B | GND                       2 RED
               +5V | 3 | C | +5V                       3 GREEN
               +5V | 4 | D | +5V                       4 BLUE
               -5V | 5 | E | -5V                       5 SYNC
              +12V | 6 | F | +12V
               KEY |   |   | KEY
         Counter 1 | 8 | J | Counter 2
         Lockout 1 | 9 | K | Lockout 2                 Connector "H" (on video board. Note the PCB will work without this connector wired up)
       Speaker 1 + |10---L | Speaker 2 + (cockpit/DX   -------------
       Speaker 1 - |11---M | Speaker 2 - (cockpit/DX)  1  GND
          Volume 1 |12 | N | Volume 2 (cockpit/DX)     2  GND
               N/C |13 | P |                           3  GND
               GND |14 | R | Service Coin              4  GND
               GND |15 | S | Brake                     5  +5V
            Coin 1 |16 | T | Coin 2                    6  +5V
                   |17 | U |                           7  +5V
             Nitro |18 | V | Tilt                      8  +5V
Wheel Center Sense |19 | W | Start                     9  N/C
             Shift |20 | X | Accelerate                10 N/C
                   |21 | Y |                           11 N/C
            Lamp 1 |22 | Z | Lamp 2                    12 N/C
               N/C |23 | a |
                   |24 | b |
                   |25 | c |
     Steering Left |26 | d | Steering Right
               GND |27 | e | GND
               GND |28 | f | GND

Note:
      - 22 & Z are used to drive CPU-controlled lamps in the top header.
      - N/C means there is no trace connected to this edge connector pad.
      - Speaker 1 and 2 are hard-wired together at the edge connector and
        the PCB has a single mono amp chip so sound is mono regardless
        of how many speakers are in the cabinet.


Connector "M" (to Motor Control PCB, using standard 50-pin flat cable)
Note the labels shown are the signals.

  ------------+--------------
   /MTRES | A | 1 | MTA0  \
      GND | B | 2 | MTA1  |
      GND | C | 3 | MTA2  |
      GND | D | 4 | MTA3  |
      GND | E | 5 | MTA4  |
      GND | F | 6 | MTA5  | 11-bit Address Bus to/from Motor PCB
      GND | H | 7 | MTA6  |
      GND | J | 8 | MTA7  |
      GND | K | 9 | MTA8  |
      GND | L | 10| MTA9  |
      GND | M | 11| MTA10 /
      GND | N | 12| MTD0  \
      GND | P | 13| MTD1  |
      GND | R | 14| MTD2  |
      GND | S | 15| MTD3  |
      GND | T | 16| MTD4  | 8-bit Data Bus to/from Motor PCB
      GND | U | 17| MTD5  |
      GND | V | 18| MTD6  |
      GND | W | 19| MTD7  /
      GND | X | 20| MTR/W   Read/Write to/from Motor PCB
      GND | Y | 21| MOTOR   Motor On/Off Signal?
      GND | Z | 22| MTDTA   Acknowledge Signal?
      GND | a | 23| GND
      GND | b | 24| MTCK    Z80 clock from main board = 3.000MHz (24/8)
      GND | c | 25| GND

/MTRES is the Z80 reset signal. This stays low in upright cabinet mode, but when the
cabinet DIPs are set to deluxe this will go high to reset the Z80 on the motor PCB.
If the motor PCB is not present the signal will toggle low then high trying to reset
the Z80 until an acknowledge/ready signal (possibly MTDTA) comes back from the motor PCB.
Note none of these signals are present at the edge connector "M" if the 4 IC's at
IC78, IC79, IC80 & IC81 are not populated (i.e. if it's an upright PCB version).
Also note all these signals are present somewhere on the main PCB even if those logic chips
are not populated and regardless of the ROM set used. Most likely any ROM set will work
as a Deluxe version if the Motor PCB is present and working.


DIP Switches
------------

DIPSWA
|--------------------------------|---------------|-------------------------------|
| 1   2   3   4   5   6   7   8  | Function      | Option                        |
|--------------------------------|---------------|-------------------------------|
|OFF OFF                         | Cabinet       | Upright / Steering Lock*      |
|ON  OFF                         |               | Upright / Free Steering       |
|OFF ON                          |               | Cockpit / Steering Lock       |
|ON  ON                          |               | Deluxe  / Free Steering       |
|--------------------------------|---------------|-------------------------------|
|        OFF                     | Test Mode     | Game*                         |
|        ON                      |               | Test                          |
|--------------------------------|---------------|-------------------------------|
|            OFF                 | Demo Sounds   | On*                           |
|            ON                  |               | Off                           |
|--------------------------------|---------------|-------------------------------|
|                OFF OFF         | Coin A        | 1 Coin 1 Credit*              |
|                ON  OFF         |               | 2 Coins 1 Credit              |
|                OFF ON          |               | 3 Coins 1 Credit              |
|                ON  ON          |               | 4 Coins 1 Credit              |
|--------------------------------|---------------|-------------------------------|
|                        OFF OFF | Coin B        | 1 Coin 2 Credits*             |
|                        ON  OFF |               | 1 Coin 3 Credits              |
|                        OFF ON  |               | 1 Coin 4 Credits              |
|                        ON  ON  |               | 1 Coin 6 Credits              |
|--------------------------------|---------------|-------------------------------|
Notes:
* = Factory default setting

Coinage varies by region. Coinage shown is World Coinage.

DIPSWA 1 & 2 must be set to upright/cockpit cabinets otherwise there is an error on boot-up.
If both SW1 & SW2 are on, it enables the deluxe motion cabinet and the Motor PCB must be
present and working otherwise the game will complain with an error on boot-up "DIPSW A INITIAL ERROR!"

Upright uses digital pedals. Digital Pedals means pedals are micro-switch (on/off only)
Cockpit/Deluxe uses analog pedals. Analog Pedals means pedals use 5k-ohm potentiometers.
Steering Lock means steering wheel uses 5k-ohm potentiometers and restricted wheel movement.
Free Steering means steering wheel uses spinner-like mechanism with additional sensor for center position and spring to re-center the wheel.

DIPSWB
|--------------------------------|----------------|-------------------------------|
| 1   2   3   4   5   6   7   8  | Function       | Option                        |
|--------------------------------|----------------|-------------------------------|
|OFF OFF                         | Difficulty     | Medium*                       |
|ON  OFF                         |                | Easy                          |
|OFF ON                          |                | Hard                          |
|ON  ON                          |                | Hardest                       |
|--------------------------------|----------------|-------------------------------|
|        OFF OFF                 | Timer          | 60 Seconds*                   |
|        ON  OFF                 |                | 70 Seconds                    |
|        OFF ON                  |                | 65 Seconds                    |
|        ON  ON                  |                | 55 Seconds                    |
|--------------------------------|----------------|-------------------------------|
|                OFF             | Number of      | 3*                            |
|                ON              | Nitros         | 5                             |
|--------------------------------|----------------|-------------------------------|
|                    OFF         | Discount On    | Yes. 1 Coin To Continue*      |
|                    ON          | Continue       | No Discount To Continue       |
|--------------------------------|----------------|-------------------------------|
|                        OFF     | Criminal Damage| Clear*                        |
|                        ON      | On Continue    | Carry Over To Continued Game  |
|--------------------------------|----------------|-------------------------------|
|                            OFF | Allow Continue | Yes*                          |
|                            ON  |                | No                            |
|--------------------------------|----------------|-------------------------------|
* = Factory default setting


Video Board
-----------

K1100358A
J1100158A VIDEO PCB
K1100358A CHASE HQ CP (sticker)
|------------------------------------------------------------------------|
|         B52-26.IC15 B52-26.IC52                                        |
|         B52-17.IC16 B52-17.IC53                                        |
| H       B52-26.IC17 B52-26.IC54                                        |-|
|         B52-17.IC18 B52-17.IC55                                        | |
|                                                                        | |
|                     62256 62256 62256 62256 62256 62256 62256 62256    | |
|                                                                        | |
|                     62256 62256 62256 62256 62256 62256 62256 62256    | |
|                                                                        | |
|  B52-30.IC4                          |---------|     B52-127.IC156     | |
|                                      |TAITO    |                       |-|
|  B52-34.IC5                          |TC0020VAR|                       |
|                                      |         | B52-03.IC135          |
|  B52-31.IC6                          |---------| B52-126.IC136         |-|
|                          B52-27.IC64                      B52-124.IC180| |
|  B52-35.IC7              B52-51.IC65                                   | |
|                          B52-50.IC66            B52-25.IC123           | |
|  B52-32.IC8                       B52-125.IC112 B52-122.IC124          | |
|                          B52-49.IC68            B52-123.IC125          | |
|  B52-36.IC9                                                            | |
|                                B52-16.IC92                             | |
|  B52-33.IC10 B52-19.IC33       B52-18.IC93                             |-|
|              B52-38.IC34       B52-16.IC94                       2018  |
|  B52-37.IC11 B52-20.IC35 B52-21.IC51                             2018  |
|------------------------------------------------------------------------|
Notes:
      2018                       - 2k x8-bit SRAM
      62256                      - 32k x8-bit SRAM
      IC4/5/6                    - MB834100 Mask ROM
      IC7/8/9/10/11/34           - 23C4000 Mask ROM
      IC15/16/17/18/52/53/54/55  - PAL20R8
      IC33/35/51/112/124/125/180 - PAL16L8
      IC92/93/94                 - 63S141 Bi-Polar PROM
      IC64/IC123                 - PAL20L8
      IC135/136/156              - 63S441 Bi-Polar PROM
      IC68                       - 27C64 EPROM
      IC65/66                    - 27C512 EPROM
      TC0020VAR                  - Taito custom IC Sprite Generator


Motor PCB (only for full motion DX cabinet)
---------

J9100093A
K9100120A
DRIVE LOGIC P.C.B.
|-----------------------------------------------|
|                       BRT BRY T1 T2 Y1 Y2 OPT | <-- 7 LEDs
|                                               |
|             B14-32.IC27  HC14                 |
|     4584    B14-33.IC26  LS07   TL081 TL081   |
|1    4584                 LS02               b1|
|8              D4701      LS32   TL081 TL081   |
|W    TLP521-4             LS139                |
|A              D4701      LS74   TL081 TL081   |
|Y                         LS393              b2|
|           LS273          LS393                |
|           LS273 LS161    LS30                 |
|           LS273 LS161    LS04                 |
|   LS244   LS273 LS161    LS138                |
|   LS245   LS245 LS161                         |
|   LS157   2016                                |
|                    2016                       |
|R  LS157   LS138  27C256.IC17                  |
|                                               |
|   LS157  B14-34.IC6  Z80                      |
|                                               |
|-----------------------------------------------|
Notes: (all IC's shown)
         R        - 50 pin flat cable connector joining to main board connector M
         18WAY    - 36 pin card edge connector joining to numerous sensors and switches on the DX cabinet
         b1/b2    - 20 pin flat cable joining 2x Motor Power PCBs to Drive Logic PCB
         Z80      - Z80A CPU. Clock input 3.000MHz from main board (24/8)
         27C256   - 32k x8-bit EPROM
         TLP521-4 - Toshiba TLP521-4 Photocoupler With 4 Isolated Channels
         D4701    - NEC D4701 Incremental Encoder Counter
         TL081    - Texas Instruments TL081 JFET-Input Operational Amplifier
         4584     - Hex Schmitt Trigger Logic IC
         2016     - 2k x8-bit SRAM
         B14-*    - PAL16L8. Note the PALs and this Drive Logic PCB come from a Top Speed DX.
                    The Chase HQ DX cabs are just converted Top Speed DX cabs.


****************************************************************************

ChaseHQ2(SCI) custom chips (Guru) (DG: same as Bshark except 0140SYT?)
--------------------------

CPU PCB:
TC0170ABT
TC0150ROD
TC0140SYT
TC0220IOC

c09-23.rom is a
PROM type AM27S21PC, location looks like this...

-------------
|   68000   |
-------------

c09-25    c09-26
c09-24

|-------|
|       |
| ABT   |
|       |
|-------|

c09-23     c09-07

|-------|
|       |
| ROD   |
|       |
|-------|

c09-32   c09-33
-------------
|   68000   |
-------------

c09-21  c09-22

Lower PCB:
TC0270MOD
TC0300FLA
TC0260DAR
TC0370MSO
TC0100SCN
TC0380BSH

c09-16.rom is located next to
c09-05, which is located next to Taito TC0370MSO.

SCI (Guru)
Taito, 1989

Controls for this game are one wheel, one switch for shift lever (used for high gear)
and one switch each for accelerate, brake, gun and nitro.

Note that the gear is low by default and is shifted to high gear by a lever which
holds the switch closed. The lever is not self-centering or spring-loaded to go back to
low. The lever must be physically shifted back to low when required.


PCB Layout
----------

CPU PCB  K1100490A  J1100209A
|----------------------------------------------------|
| 24MHz  C09-14.42  TC0140SYT   C09-22.3   C09-21.2  |
|        C09-13.43                    68000          |
|        C09-12.44              C09-33.6   C09-32.5  |
|        YM2610     C09-15.29   6264       6264      |
| YM3016 TL074 TL074  Z80                            |
|                   C09-34.31   6264       TC0150ROD |
|D             VOL    6264      6264                 |
|                                                    |
|        MB3735                 C09-07.15  C09-23.14 |
|          D633        16MHz                         |
|   62064             6264      6264       TC0170ABT |
|                    C09-28.37  6264                 |
|G  62003            C09-36.38           C09-24.22   |
|        TC0220IOC    6264     C09-26.26 C09-25.25   |
|                    C09-30.40        68000          |
|        DSWB DSWA   C09-31.41                       |
|----------------------------------------------------|

Notes:
      Clocks:
             68000 : 16.000MHz (both)
             Z80   : 4.000MHz
             YM2610: 8.000MHz

      Vsync: 60Hz

      Misc parts:
                 MB3735: 15w Power AMP with dual output (used for stereo sounds ; CH1/CH2)
                 TL074 : JFET Lo Noise Quad OP Amp
                 6264  : 8k x8 SRAM
               TD62064 : NPN 50V 1.5A Quad Darlinton Switch (for driving coin meters)
               TD62003 : PNP 50V 0.5A Quad Darlinton Switch (for driving coin meters)
                  D633 : Si NPN POWER transistor used in 68k reset circuit (TIP122 compatible)
      ROMs:
            C09-12 to C09-14   - MB834100
            C09-07             - HN62404
            C09-32 to C09-33   - AM27C512
            C09-30 to C09-31   - TC571000
            C09-38 and C09-36  - TC571000
            C09-23             - AM27S21
            C09-22 and C09-26  - MMI PAL16L8B
            C09-21 and
            C09-24 to C09-25   - MMI PAL20L8B

PINOUT CONNECTOR D (Note: All pinouts typed from an original Taito document)
------------------

1  +24V
2  +24V
3  GND
4  GND
5  D OUT

Question: +24V and D OUT are for?


PINOUT CONNECTOR G (the meanings of some of these is a bit vague - PTL OUTx, DRV0, HANDLE CENTER SW etc)

         PARTS     |    SOLDER
    ---------------|---------------
             GND  1|A GND
             GND  2|B GND
             +5V  3|C +5V
             +5V  4|D +5V
             -5V  5|E -5V
            +12V  6|F +12V
             KEY  7|H KEY
       COUNTER A  8|J COUNTER B
     C LOCKOUT A  9|K C LOCKOUT B
        SPK CH1+ 10|L SPK CH2+
        SPK CH1- 11|M SPK CH2-
         VOLUME2 12|N VOLUME1
         VOLUME3 13|P MUTE
             GND 14|R SERVICE SW
             GND 15|S BRAKE SW0
          COIN A 16|T COIN B
       BRAKE SW1 17|U BRAKE SW2
        NITRO SW 18|V TILT
HANDLE CENTER SW 19|W START SW
        SHIFT SW 20|X ACCEL SW0
       ACCEL SW1 21|Y ACCEL SW2
        PTL OUT1 22|Z PTL OUT2
            DRV0 23|a GUN SW
         PADL X1 24|b PADL X2
         PADL Y1 25|c PADL Y2
       HANDLE Z1 26|d HANDLE Z2
             GND 27|e GND
             GND 28|f GND

Question: What hardware is used for steering and where is it connected? It doesn't seem to use
          a regular potentiometer for the steering??


PCB Layout
----------

VIDEO PCB  K1100491A  J1100210A
|-----------------------------------------------------|
|          TC0370MSO  C09-17.24  43256                |
|H                    C09-18.25  43256                |
|          C09-05.16                                  |
|          C09-16.17  26.686MHz  TC0100SCN  6264      |
|   C1815                                             |
|V  C1815                                   6264      |
|   C1815  TC0260DAR             C09-06.37            |
|6264                                                 |
|                                                     |
|                                TC0380BSH   C09-19.67|
|TC0270MOD TC0300FLA                                  |
|                                                     |
|43256    43256   43256   43256   C09-04.52  C09-20.71|
|43256    43256   43256   43256   C09-03.53           |
|43256    43256   43256   43256   C09-02.54           |
|43256    43256   43256   43256   C09-01.55           |
|-----------------------------------------------------|

Notes:
      ROMs:
            C09-01 to C09-05   - 234000
            C09-06             - HN62404
            C09-17 to C09-18   - MMI 63S441
            C09-19             - MMI PAL16L8B
            C09-20             - AM27S21

      Misc parts:
                6264: 8k x8 SRAM
               43256: 32k x8 SRAM
               C1815: transistor used for driving RGB

PINOUT CONNECTOR H
------------------

1  GND
2  GND
3  GND
4  GND
5  +5V
6  +5V
7  +5V
8  +5V
9  -5V
10 POST
11 +12V
12 NC


PINOUT CONNECTOR V
------------------

1  GND
2  RED
3  GREEN
4  BLUE
5  SYNC
6  NC
7  NC



****************************************************************************

Enforce PCB info
------------------

Taito, 198?/199?

Taito Z hardware
PCB No: K1100406A J1100175A (CPU PCB)
        K1100407A J1100176A (VIDEO PCB)

CPU: MC68000P12 (x2)
SND: Z80, YM2610, TC0040IOC, YM3016F
OSC: 26.686MHz, 24.000MHz, 16.000MHz
DIPs: 8 Position (x2)

Taito Chips:
CPU board - TC0100SCN, TC0140SYT, TC0170ABT, TC0110PCR
Video Board - TC0150ROD, TC0050VDZ (x3), TC0020VAR

Ram: CPU BOARD - 6264 (x9), 43256 (x2),
     VIDEO BOARD - 2018 (x10), 6264 (x2)

PALs/PROMs:
CPU BOARD - All located near/around the 68000's
b58-15 (PAL20L8)
b58-16 (PAL20L8)
b58-14 (PAL20L8)
b58-13 (PAL16L8)
b58-11 (PAL16L8)
b58-12 (PAL16L8)

VIDEO BOARD -
b58-22 (PAL16L8) \
b58-23 (63s141)   |  near TC0150ROD
b58-24 (63s141)  /

b58-20 (PAL16L8) \
b58-21 (PAL16R4)  |
b58-17 (PAL16L8)  |  near TC0020VAR
b58-18 (PAL16R4) /

b58-25 (63S141)      near b58-27
b58-19 (PAL16R4)     near b58-04/03/02/01

ROMs:
CPU BOARD - b58-18 , 27C010  \
        b58-19 , 27C010   |  68k Program
        b58-26 , 27C010   |
        b58-27 , 27C010  /

        b58-07 , 27C4096 \
        b58-08 , 27C4096  |
        b58-09 , 27C4100  |  near TC0100SCN & TC0140SYT
        b58-10 , 27C4096 /

        b58-32 , 27C512      z80 program

VIDEO PCB - b58-06 , 27C4100     near TC0150ROD

        b58-26a, 27C512      ?
        b58-27 , LH5763      ?

        b58-01 , 27C4100 \
        b58-02 , 27C4100  |
        b58-03 , 27C4100  |  near TC0050VDZ's
        b58-04 , 27C4100 /



****************************************************************************

BShark custom chips
-------------------

TC0220IOC (known io chip)
TC0260DAR (known palette chip)
TC0400YSC  substitute for TC0140SYT when 68K writes directly to YM2610 ??
TC0170ABT  = same in Dblaxle
TC0100SCN (known tilemap chip)
TC0370MSO  = same in Dblaxle, Motion Objects ?
TC0300FLA  = same in Dblaxle
TC0270MOD  ???
TC0380BSH  ???
TC0150ROD (known road generator chip)



****************************************************************************

DblAxle custom chip info
------------------------

TC0150ROD is next to road lines gfx chip [c78-09] but also
c78-15, an unused 256 byte rom. Perhaps this contains color
info for the road lines? Raine makes an artificial "pal map"
for the road, AFAICS.

TC0170ABT is between 68000 CPUA and the TC0140SYT. Next to
that is the Z80A, the YM2610, and the three adpcm roms.

On the graphics board we have the TC0480SCP next to its two
scr gfx roms: c78-10 & 11.

The STY object mapping rom is next to c78-25, an unused
0x10000 byte rom which compresses by 98%. To right of this
are TC0370MSO (motion objects?), then TC0300FLA.

Below c78-25 are two unused 1K roms: c84-10 and c84-11.
Below right is another unused 256 byte rom, c78-21.
(At the bottom are the 5 obj gfx roms.)

K11000635A
----------
 43256   c78-11 SCN1 CHR
 43256   c78-10 SCN0 CHR   TC0480SCP

 c78-04
 STY ROM
            c78-25   TC0370MSO   TC0300FLA
            c84-10
            c84-11                                      c78-21

                       43256 43256 43256 43256
                 43256 43256 43256 43256 43256
                 43256 43256 43256 43256 43256
                                   43256 43256

                             c78-05L
            c78-06 OBJ1
                             c78-05H

            c78-08 OBJ3      c78-07 OBJ2

Power Wheels
------------

Cpu PCB

CPU:    68000-16 x2
Sound:  Z80-A
    YM2610
OSC:    32.000MHz
Chips:  TC0140SYT
    TC0150ROD
    TC0170ABT
    TC0310FAM
    TC0510NIO


Video PCB

OSC:    26.686MHz
Chips:  TC0260DAR
    TC0270MOD
    TC0300FLA
    TC0370MSO
    TC0380BSH
    TC0480SCP


LAN interface board

OSC:    40.000MHz
    16.000MHz
Chips:  uPD72105C



****************************************************************************

Racing Beat
-------------

M43E0227A
K11E0674A
K1100650A J1100264A CPU PCB
|-------------------------------------------------------------|
|6264       62256        32MHz          DSWA   DSWB           |
|           62256                                             |
|C84-104.2                                                    |
|C84-110.3  TC0170ABT                          TC0510NIO      |
|C84-103.4                                                    |
|C84-111.5                                     MB3771         |
|                                   C84_101.42                |
|6264                 TC0140SYT                               |
|                                    6264                     |
|                                                             |
|                                                             |
|                         C84-85.31         Z80               |
|68000                                                        |
|                                                             |
|                                                             |
|PAL     PAL                                YM2610            |
|                         C84-86.33                           |
|PAL                                                          |
|                             6264          C84-87.46         |
|                                                             |
|                                                             |
|                   PAL   C84-99.35         YM3016            |
|6264   6264                                                  |
|                                                             |
|                   PAL   C84-100.36        TL074             |
|           TC0150ROD                                         |
|C84-84.12                    6264                            |
|                   PAL                                       |
|                                                TL074        |
|    C84-07.22                                        MB3735  |
|                  68000                                      |
|-------------------------------------------------------------|
Notes:
      68000s running at 16MHz
      Z80 running at 4MHz
      YM2610 running at 8MHz


K11X0675A
K1100635A
J1100256A VIDEO PCB
|-------------------------------------------------------------|
|                        26.686MHz       6264                 |
|62256    C84-89.11                              TC0260DAR    |
|                                                             |
|62256    C84-90.12                                           |
|                        TC0480SCP       6264                 |
|                                                             |
|                                        6264                 |
|C84-88.3                                                     |
|                                                             |
|                                                             |
|         C84-19.15                                           |
|                        TC0370MSO     TC0300FLA    PAL       |
|         C84-10.16                                           |
|         C84-11.17                                           |
|                                                    C84-09.74|
|                                                             |
|                    62256   62256   62256   62256            |
|                                                             |
|                                                             |
|            62256   62256   62256   62256   62256            |
|                                                             |
|                                                             |
|            62256   62256   62256   62256   62256            |
|                                                             |
|                                                             |
|                                    62256   62256            |
|  C84-91.23    C84-93.31                                     |
|                                                             |
|                              TC0380BSH           TC0270MOD  |
|  C84-92.25    C84-94.33                                     |
|                                                             |
|-------------------------------------------------------------|


TODO Lists
==========

Is the no-Z80 sound handling correct: some voices in Bshark
aren't that clear.

Make taitosnd cpu-independent so we can restore Z80 to CPU3.

Cockpit hardware

DIPs - e.g. coinage

Sprite zooming - dimensions may be got from the unused 64K rom
on the video board (it's in the right place for it, both with
Contcirc video chips and the chips used on later boards). These
64K roms compare as follows - makes sense as these groups
comprise the three sprite layout types used in TaitoZ games:

   Contcirc / Enforce                =IDENTICAL
   ChaseHQ / Nightstr                =IDENTICAL
   Bshark / SCI / Dblaxle / Racingb  =IDENTICAL

   Missing from Aquajack / Spacegun dumps (I would bet they are
   the same as Bshark). Can anyone dump these two along with any
   proms on the video board?


Continental Circus
------------------

Road priority incompletely understood - e.g. start barrier should
be darkening LH edge of road as well as RH edge.

Junk (?) stuff often written in high byte of sound word.

Speculative YM2610 a/b/c channel filtering as these may be
outputs to subwoofer (vibration). They sound better, anyway.


Chasehq
-------

Motor CPU: appears to be identical to one in Topspeed.

[Used to have junk sprites when you reach criminal car (the 'criminals
here' sprite): two bits above tile number are used. Are these
meaningless, or is some effect missing?]


Enforce
-------

Test mode - SHIFT: LO/HI is not understood (appears to depend on Demo
Sound DSW)

Landscape in the background can be made to scroll rapidly with DSW.
True to original?

Some layer offsets are out a little.


Battle Shark
------------

Is road on the road stages correct? Hard to tell.

Does the original have the "seeking" crosshair effect, making it a
challenge to control?


SCI
---

Road seems ok, but are the green bushes in round 1 a little too far
to the edge of the interroad verge?

Sprite frames were plotted in opposite order so flickered. Reversing
this has lost us alternate frames: probably need to buffer sprite
ram by one frame to solve this?


Night Striker
-------------

Road A/B priority problems will manifest in the choice tunnels with,
one or the other having higher priority in top and bottom halves. Can
someone provide a sequence of screenshots showing exactly what happens
at the road split point.

Strange page in test mode which lets you alter all sorts of settings,
may relate to sit-in cockpit version. Can't find a dip that disables
this. <- Test Mode 1? That's used for lamps and motor testing... -AS

Motors (located at the 0xe000**) are mirrored, they use both bytes of a
word, the high one is used during gameplay and the other one is used on service
mode. The gameplay port is xor'ed (!).
It works like this:
--xx xx-- Force Feedback power
---- --x- "Reverse" motor
---- ---x "Turn" motor

TC0220IOC offset 3 is used for lamps, both upright and cockpit version afaik:
x--- ---- spot 2 lamp (right)
-x-- ---- spot 1 lamp (left)
--x- ---- motor lamp 3 (right)
---x ---- motor lamp 2 (center)
---- x--- motor lamp 1 (left)
---- -x-- trigger lamp
---- --x- start lamp
---- ---x shot lamp
The two spot lamps are big red lamps located at the sides of the screen (at least
in the upright version), they lights when the player gets hit and/or if he's dying.


Aqua Jack
---------

Some wrong colors. Hovercraft body should be red. 1st level sky/water
should be blue.

Sprites left on screen under hiscore table. Deliberate? Or is there
a sprite disable bit somewhere.

Should road body be largely transparent as I've implemented it?

Sprite/sprite priorities often look bad. Sprites go to max size for
a frame before they explode - surely a bug.

Hangs briefly fairly often without massive cpu interleaving (500).
Keys aren't very responsive in test mode.

The problem code is this:

CPUA
$1fe02 hangs waiting for ($6002,A5) in shared ram to be zero.

CPUB
$1056 calls $11ea routine which starts by setting ($6002,A5) non-
zero. At end (after $1218 waiting for a bit from sound comm port)
it alters ($6002,A5) to zero (but this value lasts briefly!).

Unless context rapidly switches back to cpua this change is missed
because $11ea gets called again *very* rapidly at times when sounds
are being written [that's when the problem manifested].

$108a-c2 reads 0x20 bytes from unmapped area, not sure
what it's doing. Perhaps this machine had some optional
exotic input device...


Spacegun
--------

Problem with the zoomed sprites not matching up very well
when forming the background. They jerk a bit relative to
each other... probably a cpu sync thing, perhaps also some
fine-tuning required on the zoomed sprite dimension calcs.

ADC clock unknown.


Double Axle
-----------

Road occasionally has incorrectly unclipped line appearing at top
(ice stage). Also road 'ghost' often remains on screen - also an
interrupt issue I presume.

Double Axle has poor sound: one ADPCM rom should be twice as long?
[In log we saw stuff like this, suggesting extra ADPCM rom needed:
YM2610: ADPCM-A end out of range: $001157ff
YM2610: ADPCM-A start out of range: $00111f00]

Various sprites go missing e.g. mountains half way through cross
country course. Fall off the ledge and crash and you will see
the explosion sprites make other mountain sprites vanish, as
though their entries in spriteram are being overwritten. (Perhaps
an int6 timing/number issue: sprites seem to be ChaseHQ2ish with
a spriteframe toggle - currently this never changes which may be
wrong.)

Double Axle seems to keep only 1 sprite frame in sprite ram,
which is probably wrong. Game seems to work with no int 6's
at all. Cpu control byte has 0,4,8,c poked into 2nd nibble
and it seems possible this should be causing int6's ?


Racing Beat
-----------

Graphics problems:
- tearing in the main road (tile layer 3 offset?)
  likely cause is mame/video/tc0480scp.c in bg23_draw:
      ** flawed calc ?? **
      x_index -= (m_x_offset - 0x1f + layer * 4) * ((row_zoom & 0xff) << 8);
- car sprites palette flickering
- layer missing sometimes (random?) ie. motor block sprite after inserting coin

LAN board is unemulated

DIP switches are not verified


***************************************************************************/

#include "emu.h"
#include "includes/taito_z.h"
#include "includes/taitoipt.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/adc0808.h"
#include "machine/eepromser.h"
#include "sound/ymopn.h"
#include "speaker.h"

#include "chasehq.lh"
#include "contcirc.lh"
#include "dblaxle.lh"
#include "enforceja.lh"


void taitoz_state::parse_cpu_control()
{
	/* bit 0 enables cpu B */
	m_subcpu->set_input_line(INPUT_LINE_RESET, (m_cpua_ctrl & 0x1) ? CLEAR_LINE : ASSERT_LINE);
}

void taitoz_state::cpua_ctrl_w(offs_t offset, u16 data, u16 mem_mask)
{
	//logerror("CPU #0 PC %06x: write %04x to cpu control\n", m_maincpu->pc(), data);

	if (mem_mask == 0xff00) data >>= 8;
	data &= 0xff;

	m_cpua_ctrl = data;
	parse_cpu_control();
}

void chasehq_state::chasehq_cpua_ctrl_w(offs_t offset, u16 data, u16 mem_mask)
{
	cpua_ctrl_w(offset, data, mem_mask);

	m_lamps[0] = BIT(m_cpua_ctrl, 5);
	m_lamps[1] = BIT(m_cpua_ctrl, 6);
}

void dblaxle_state::dblaxle_cpua_ctrl_w(offs_t offset, u16 data, u16 mem_mask)
{
	cpua_ctrl_w(offset, data, mem_mask);

	m_wheel_vibration = BIT(data, 2);
}


/***********************************************************
                        INTERRUPTS
***********************************************************/

void sci_state::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
	case TIMER_TAITOZ_INTERRUPT6:
		/* 68000 A */
		m_maincpu->set_input_line(6, HOLD_LINE);
		break;
	default:
		throw emu_fatalerror("Unknown id in taitoz_state::device_timer");
	}
}

/***** Routines for particular games *****/

INTERRUPT_GEN_MEMBER(sci_state::sci_interrupt)
{
	/* Need 2 int4's per int6 else (-$6b63,A5) never set to 1 which
	   causes all sprites to vanish! Spriteram has areas for 2 frames
	   so in theory only needs updating every other frame. */

	m_sci_int6 = !m_sci_int6;

	if (m_sci_int6)
		timer_set(downcast<cpu_device *>(&device)->cycles_to_attotime(200000 - 500), TIMER_TAITOZ_INTERRUPT6);

	device.execute().set_input_line(4, HOLD_LINE);
}


/******************************************************************
                              EEPROM
******************************************************************/

static const u16 spacegun_default_eeprom[64]=
{
	0x0000,0x00ff,0x0001,0x4141,0x0000,0x00ff,0x0000,0xf0f0,
	0x0000,0x00ff,0x0001,0x4141,0x0000,0x00ff,0x0000,0xf0f0,
	0x0080,0x0080,0x0080,0x0080,0x0001,0x4000,0x0000,0xf000,
	0x0001,0x4285,0x0000,0xf1e3,0x0001,0x4000,0x0000,0xf000,
	0x0001,0x4285,0x0000,0xf1e3,0xcccb,0xffff,0xffff,0xffff,
	0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,
	0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,
	0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff
};


#if 0
u16 taitoz_state::eep_latch_r()
{
	return m_eep_latch;
}
#endif

void spacegun_state::spacegun_eeprom_w(u8 data)
{
/*          0000xxxx    (unused)
            000x0000    eeprom reset (active low)
            00x00000    eeprom clock
            0x000000    eeprom data
            x0000000    (unused)                  */

	m_eep_latch = data;
	m_io_eepromout->write(data, 0xff);
}


/**********************************************************
                       GAME INPUTS
**********************************************************/

CUSTOM_INPUT_MEMBER(taitoz_state::gas_pedal_r)
{
	static const u8 retval[8] = { 0,1,3,2,6,7,5,4 };
	return retval[m_gas.read_safe(0) & 7];
}

CUSTOM_INPUT_MEMBER(taitoz_state::brake_pedal_r)
{
	static const u8 retval[8] = { 0,1,3,2,6,7,5,4 };
	return retval[m_brake.read_safe(0) & 7];
}

// enforceja only, 3 bits applied on both pots
template <int axis> CUSTOM_INPUT_MEMBER(taitoz_state::adstick_r)
{
	static const u8 retval[8] = { 0,1,3,2,6,7,5,4 };
	u8 raw_value = ((axis == 0 ? m_stickx : m_sticky).read_safe(0) >> 5) & 7;
	return retval[raw_value];
}

u8 contcirc_state::contcirc_input_bypass_r()
{
	/* Bypass TC0040IOC controller for analog input */

	u8 port = m_tc0040ioc->port_r();   /* read port number */
	u16 steer = 0xff80 + m_steer.read_safe(0x80);

	switch (port)
	{
		case 0x08:
			return steer & 0xff;

		case 0x09:
			return steer >> 8;

		default:
			return m_tc0040ioc->portreg_r();
	}
}


u8 chasehq_state::chasehq_input_bypass_r()
{
	/* Bypass TC0040IOC controller for extra inputs */

	u8 port = m_tc0040ioc->port_r();   /* read port number */
	u16 steer = 0xff80 + m_steer.read_safe(0x80);

	switch (port)
	{
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
			return m_unknown_io[port - 0x08]->read();

		case 0x0c:
			return steer & 0xff;

		case 0x0d:
			return steer >> 8;

		default:
			return m_tc0040ioc->portreg_r();
	}
}


u16 sci_state::sci_steer_input_r(offs_t offset)
{
	u16 steer = 0xff80 + m_steer.read_safe(0x80);

	switch (offset)
	{
		case 0x04:
			return (steer & 0xff);

		case 0x05:
			return (steer & 0xff00) >> 8;
	}

	logerror("CPU #0 PC %06x: warning - read unmapped steer input offset %06x\n", m_maincpu->pc(), offset);

	return 0xff;
}


void spacegun_state::spacegun_gun_output_w(u16 data)
{
	m_recoil[0] = BIT(data, 0);
	m_recoil[1] = BIT(data, 1);
}


u16 taitoz_z80_sound_state::dblaxle_steer_input_r(offs_t offset)
{
	u16 steer = 0xff80 + m_steer.read_safe(0x80);

	switch (offset)
	{
		case 0x04:
			return steer >> 8;

		case 0x05:
			return steer & 0xff;
	}

	logerror("CPU #0 PC %06x: warning - read unmapped steer input offset %02x\n", m_maincpu->pc(), offset);

	return 0x00;
}

// TODO: proper motorcpu hook-up


u16 chasehq_state::chasehq_motor_r(offs_t offset)
{
	switch (offset)
	{
		case 0x0:
			return (machine().rand() &0xff);    /* motor status ?? */

		case 0x101:
			return 0x55;    /* motor cpu status ? */

		default:
			logerror("CPU #0 PC %06x: warning - read motor cpu %03x\n",m_maincpu->pc(),offset);
			return 0;
	}
}

void chasehq_state::chasehq_motor_w(offs_t offset, u16 data)
{
	/* Writes $e00000-25 and $e00200-219 */
	switch (offset)
	{
		case 0x0:
			break;

		case 0x101:
			/* outputs will go here, but driver is still broken */
			break;
	}

	logerror("CPU #0 PC %06x: warning - write %04x to motor cpu %03x\n",m_maincpu->pc(),data,offset);
}


void nightstr_state::nightstr_motor_w(offs_t offset, u16 data)
{
	/* Despite the informative notes at the top, the high end of the word doesn't seem to output any useful data. */
	/* I've added this so someone else can finish it.  */
	switch (offset)
	{
	case 0:
		if (BIT(data, 0))
			m_motor_dir[0] = 1;
		else if (BIT(data, 1))
			m_motor_dir[0] = 2;
		else
			m_motor_dir[0] = 0;
		m_motor_speed[0] = (data & 0x3c) >> 2;
		break;

	case 4:
		if (BIT(data, 0))
			m_motor_dir[1] = 1;
		else if (BIT(data, 1))
			m_motor_dir[1] = 2;
		else
			m_motor_dir[1] = 0;
		m_motor_speed[1] = (data & 0x3c) >> 2;
		break;

	case 8:
		if (BIT(data, 0))
			m_motor_dir[2] = 1;
		else if (BIT(data, 1))
			m_motor_dir[2] = 2;
		else
			m_motor_dir[2] = 0;
		m_motor_speed[2] = (data & 0x3c) >> 2;
		break;

	default:
		m_motor_debug = data;
		break;
	}

}


void taitoz_state::coin_control_w(u8 data)
{
	machine().bookkeeping().coin_lockout_w(0, BIT(~data, 0));
	machine().bookkeeping().coin_lockout_w(1, BIT(~data, 1));
	machine().bookkeeping().coin_counter_w(0, BIT( data, 2));
	machine().bookkeeping().coin_counter_w(1, BIT( data, 3));
}


u16 taitoz_z80_sound_state::aquajack_unknown_r()
{
	return 0xff;
}


/*****************************************************
                        SOUND
*****************************************************/

void taitoz_z80_sound_state::sound_bankswitch_w(u8 data)
{
	m_z80bank->set_entry(data & 7);
}


/**** sound pan control ****/
void taitoz_state::pancontrol_w(offs_t offset, u8 data)
{
	m_filter[offset & 3]->flt_volume_set_volume(data / 255.0f);
}


/***********************************************************
                   MEMORY STRUCTURES
***********************************************************/


void contcirc_state::contcirc_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x080000, 0x083fff).ram();
	map(0x084000, 0x087fff).ram().share("share1");
	map(0x090001, 0x090001).w(FUNC(contcirc_state::contcirc_out_w));    /* road palette bank, sub CPU reset, 3d glasses control */
	map(0x100000, 0x100007).rw(m_tc0110pcr, FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::step1_rbswap_word_w));   /* palette */
	map(0x200000, 0x20ffff).rw(m_tc0100scn, FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0x220000, 0x22000f).rw(m_tc0100scn, FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x300000, 0x301fff).rw(m_tc0150rod, FUNC(tc0150rod_device::word_r), FUNC(tc0150rod_device::word_w));    /* "root ram" */
	map(0x400000, 0x4006ff).ram().share("spriteram");
}

void contcirc_state::contcirc_cpub_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x080000, 0x083fff).ram();
	map(0x084000, 0x087fff).ram().share("share1");
	map(0x100001, 0x100001).r(FUNC(contcirc_state::contcirc_input_bypass_r)).w(m_tc0040ioc, FUNC(tc0040ioc_device::portreg_w)).umask16(0x00ff);
	map(0x100003, 0x100003).rw(m_tc0040ioc, FUNC(tc0040ioc_device::watchdog_r), FUNC(tc0040ioc_device::port_w));
	map(0x200001, 0x200001).w(m_tc0140syt, FUNC(tc0140syt_device::master_port_w));
	map(0x200003, 0x200003).rw(m_tc0140syt, FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
}


void chasehq_state::chasehq_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x107fff).ram();
	map(0x108000, 0x10bfff).ram().share("share1");
	map(0x10c000, 0x10ffff).ram();
	map(0x400001, 0x400001).r(FUNC(chasehq_state::chasehq_input_bypass_r)).w(m_tc0040ioc, FUNC(tc0040ioc_device::portreg_w)).umask16(0x00ff);
	map(0x400003, 0x400003).rw(m_tc0040ioc, FUNC(tc0040ioc_device::watchdog_r), FUNC(tc0040ioc_device::port_w));
	map(0x800000, 0x800001).w(FUNC(chasehq_state::chasehq_cpua_ctrl_w));
	map(0x820001, 0x820001).w(m_tc0140syt, FUNC(tc0140syt_device::master_port_w));
	map(0x820003, 0x820003).rw(m_tc0140syt, FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0xa00000, 0xa00007).rw(m_tc0110pcr, FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::step1_word_w));  /* palette */
	map(0xc00000, 0xc0ffff).rw(m_tc0100scn, FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0xc20000, 0xc2000f).rw(m_tc0100scn, FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0xd00000, 0xd007ff).ram().share("spriteram");
	map(0xe00000, 0xe003ff).rw(FUNC(chasehq_state::chasehq_motor_r), FUNC(chasehq_state::chasehq_motor_w)); /* motor cpu */
}

void chasehq_state::chasehq_cpub_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x100000, 0x103fff).ram();
	map(0x108000, 0x10bfff).ram().share("share1");
	map(0x800000, 0x801fff).rw(m_tc0150rod, FUNC(tc0150rod_device::word_r), FUNC(tc0150rod_device::word_w));
}


void contcirc_state::enforce_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x103fff).ram();
	map(0x104000, 0x107fff).ram().share("share1");
	map(0x200001, 0x200001).w(FUNC(contcirc_state::contcirc_out_w));
	map(0x300000, 0x3006ff).ram().share("spriteram");
	map(0x400000, 0x401fff).rw(m_tc0150rod, FUNC(tc0150rod_device::word_r), FUNC(tc0150rod_device::word_w));    /* "root ram" ??? */
	map(0x500000, 0x500007).rw(m_tc0110pcr, FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::step1_rbswap_word_w));   /* palette */
	map(0x600000, 0x60ffff).rw(m_tc0100scn, FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0x620000, 0x62000f).rw(m_tc0100scn, FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
}

void contcirc_state::enforce_cpub_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x103fff).ram();
	map(0x104000, 0x107fff).ram().share("share1");
	map(0x200001, 0x200001).w(m_tc0140syt, FUNC(tc0140syt_device::master_port_w));
	map(0x200003, 0x200003).rw(m_tc0140syt, FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x300000, 0x300003).rw(m_tc0040ioc, FUNC(tc0040ioc_device::read), FUNC(tc0040ioc_device::write)).umask16(0x00ff);
}


void taitoz_state::bshark_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x110000, 0x113fff).ram().share("share1");
	map(0x400000, 0x40000f).rw(m_tc0220ioc, FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write)).umask16(0x00ff);
	map(0x600000, 0x600001).w(FUNC(taitoz_state::cpua_ctrl_w));
	map(0x800000, 0x80000f).rw("adc", FUNC(adc0808_device::data_r), FUNC(adc0808_device::address_offset_start_w)).umask16(0x00ff);
	map(0xa00000, 0xa01fff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0xc00000, 0xc00fff).ram().share("spriteram");
	map(0xd00000, 0xd0ffff).rw(m_tc0100scn, FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0xd20000, 0xd2000f).rw(m_tc0100scn, FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
}

void taitoz_state::bsharkjjs_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x110000, 0x113fff).ram().share("share1");
	map(0x400000, 0x40000f).rw(m_tc0220ioc, FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write)).umask16(0x00ff);
	map(0x600000, 0x600001).w(FUNC(taitoz_state::cpua_ctrl_w));
//  map(0x800000, 0x80000f) // No analog stick, this is the Joystick version
	map(0xa00000, 0xa01fff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0xc00000, 0xc00fff).ram().share("spriteram");
	map(0xd00000, 0xd0ffff).rw(m_tc0100scn, FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0xd20000, 0xd2000f).rw(m_tc0100scn, FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
}

void taitoz_state::bshark_cpub_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x108000, 0x10bfff).ram();
	map(0x110000, 0x113fff).ram().share("share1");
	map(0x400000, 0x400007).w(FUNC(taitoz_state::pancontrol_w)).umask16(0x00ff);  /* pan */
//  map(0x40000a, 0x40000b).r(FUNC(taitoz_state::taitoz_unknown_r));  // ???
	map(0x600000, 0x600007).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write)).umask16(0x00ff);
	map(0x60000c, 0x60000d).noprw(); // interrupt controller?
	map(0x60000e, 0x60000f).noprw();
	map(0x800000, 0x801fff).rw(m_tc0150rod, FUNC(tc0150rod_device::word_r), FUNC(tc0150rod_device::word_w));
}


void sci_state::sci_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x107fff).ram();
	map(0x108000, 0x10bfff).ram().share("share1");
	map(0x10c000, 0x10ffff).ram();
	map(0x200000, 0x20000f).rw(m_tc0220ioc, FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write)).umask16(0x00ff);
	map(0x200010, 0x20001f).r(FUNC(sci_state::sci_steer_input_r));
//  map(0x400000, 0x400001).w(FUNC(sci_state::cpua_ctrl_w));  // ?? doesn't seem to fit what's written
	map(0x420001, 0x420001).w(m_tc0140syt, FUNC(tc0140syt_device::master_port_w));
	map(0x420003, 0x420003).rw(m_tc0140syt, FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x800000, 0x801fff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0xa00000, 0xa0ffff).rw(m_tc0100scn, FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0xa20000, 0xa2000f).rw(m_tc0100scn, FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0xc00000, 0xc03fff).ram().share("spriteram");
	map(0xc08000, 0xc08001).rw(FUNC(sci_state::sci_spriteframe_r), FUNC(sci_state::sci_spriteframe_w));
}

void sci_state::sci_cpub_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x200000, 0x203fff).ram();
	map(0x208000, 0x20bfff).ram().share("share1");
	map(0xa00000, 0xa01fff).rw(m_tc0150rod, FUNC(tc0150rod_device::word_r), FUNC(tc0150rod_device::word_w));
}


void nightstr_state::nightstr_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x110000, 0x113fff).ram().share("share1");
	map(0x400000, 0x40000f).rw(m_tc0220ioc, FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write)).umask16(0x00ff);
	map(0x800000, 0x800001).w(FUNC(nightstr_state::cpua_ctrl_w));
	map(0x820001, 0x820001).w(m_tc0140syt, FUNC(tc0140syt_device::master_port_w));
	map(0x820003, 0x820003).rw(m_tc0140syt, FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0xa00000, 0xa00007).rw(m_tc0110pcr, FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::step1_word_w));  /* palette */
	map(0xc00000, 0xc0ffff).rw(m_tc0100scn, FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0xc20000, 0xc2000f).rw(m_tc0100scn, FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0xd00000, 0xd007ff).ram().share("spriteram");
	map(0xe00000, 0xe00011).w(FUNC(nightstr_state::nightstr_motor_w));    /* Motor outputs */
	map(0xe40000, 0xe4000f).rw("adc", FUNC(adc0808_device::data_r), FUNC(adc0808_device::address_offset_start_w)).umask16(0x00ff);
}

void nightstr_state::nightstr_cpub_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x103fff).ram();
	map(0x104000, 0x107fff).ram().share("share1");
	map(0x800000, 0x801fff).rw(m_tc0150rod, FUNC(tc0150rod_device::word_r), FUNC(tc0150rod_device::word_w));
}


void taitoz_z80_sound_state::aquajack_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x103fff).ram();
	map(0x104000, 0x107fff).ram().share("share1");
	map(0x200000, 0x200001).w(FUNC(taitoz_z80_sound_state::cpua_ctrl_w));  // not needed, but it's probably like the others
	map(0x300000, 0x300007).rw(m_tc0110pcr, FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::step1_word_w));  /* palette */
	map(0x800000, 0x801fff).rw(m_tc0150rod, FUNC(tc0150rod_device::word_r), FUNC(tc0150rod_device::word_w));
	map(0xa00000, 0xa0ffff).rw(m_tc0100scn, FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0xa20000, 0xa2000f).rw(m_tc0100scn, FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0xc40000, 0xc403ff).ram().share("spriteram");
}

void taitoz_z80_sound_state::aquajack_cpub_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x103fff).ram();
	map(0x104000, 0x107fff).ram().share("share1");
	map(0x200000, 0x20000f).rw(m_tc0220ioc, FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write)).umask16(0x00ff);
	map(0x300001, 0x300001).w(m_tc0140syt, FUNC(tc0140syt_device::master_port_w));
	map(0x300003, 0x300003).rw(m_tc0140syt, FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x800800, 0x80083f).r(FUNC(taitoz_z80_sound_state::aquajack_unknown_r)); // Read regularly after write to 800800...
//  map(0x800800, 0x800801).w(FUNC(taitoz_z80_sound_state::taitoz_unknown_w));
//  map(0x900000, 0x900007).rw(FUNC(taitoz_z80_sound_state::taitoz_unknown_r), FUNC(taitoz_z80_sound_state::taitoz_unknown_w));
}


void spacegun_state::spacegun_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x30c000, 0x30ffff).ram();
	map(0x310000, 0x31ffff).ram().share("share1");
	map(0x500000, 0x5005ff).ram().share("spriteram");
	map(0x900000, 0x90ffff).rw(m_tc0100scn, FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0x920000, 0x92000f).rw(m_tc0100scn, FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0xb00000, 0xb00007).rw(m_tc0110pcr, FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::step1_rbswap_word_w));   /* palette */
}

void spacegun_state::spacegun_cpub_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x20c000, 0x20ffff).ram();
	map(0x210000, 0x21ffff).ram().share("share1");
	map(0x800000, 0x80000f).rw(m_tc0510nio, FUNC(tc0510nio_device::read), FUNC(tc0510nio_device::write)).umask16(0x00ff).cswidth(16);
	map(0xc00000, 0xc00007).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write)).umask16(0x00ff);
	map(0xc0000c, 0xc0000d).noprw(); // interrupt controller?
	map(0xc0000e, 0xc0000f).noprw();
	map(0xc20000, 0xc20007).w(FUNC(spacegun_state::pancontrol_w)).umask16(0x00ff);  /* pan */
	map(0xe00000, 0xe00001).w(FUNC(spacegun_state::spacegun_gun_output_w));    /* gun outputs */
	map(0xf00000, 0xf0000f).rw("adc", FUNC(adc0808_device::data_r), FUNC(adc0808_device::address_offset_start_w)).umask16(0x00ff);
}


void dblaxle_state::dblaxle_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x200000, 0x203fff).ram();
	map(0x210000, 0x21ffff).ram().share("share1");
	map(0x400000, 0x40000f).rw(m_tc0510nio, FUNC(tc0510nio_device::halfword_wordswap_r), FUNC(tc0510nio_device::halfword_wordswap_w));
	map(0x400010, 0x40001f).r(FUNC(dblaxle_state::dblaxle_steer_input_r));
	map(0x600000, 0x600001).w(FUNC(dblaxle_state::dblaxle_cpua_ctrl_w));  /* could this be causing int6 ? */
	map(0x620001, 0x620001).w(m_tc0140syt, FUNC(tc0140syt_device::master_port_w));
	map(0x620003, 0x620003).rw(m_tc0140syt, FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x800000, 0x801fff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x900000, 0x90ffff).rw(m_tc0480scp, FUNC(tc0480scp_device::ram_r), FUNC(tc0480scp_device::ram_w));      /* tilemap mirror */
	map(0xa00000, 0xa0ffff).rw(m_tc0480scp, FUNC(tc0480scp_device::ram_r), FUNC(tc0480scp_device::ram_w));      /* tilemaps */
	map(0xa30000, 0xa3002f).rw(m_tc0480scp, FUNC(tc0480scp_device::ctrl_r), FUNC(tc0480scp_device::ctrl_w));
	map(0xc00000, 0xc03fff).ram().share("spriteram"); /* mostly unused ? */
	//map(0xc08000, 0xc08001).rw(FUNC(dblaxle_state::sci_spriteframe_r), FUNC(dblaxle_state::sci_spriteframe_w)); /* set in int6, seems to stay zero */
}

void dblaxle_state::dblaxle_cpub_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x103fff).ram();
	map(0x110000, 0x11ffff).ram().share("share1");
	map(0x300000, 0x301fff).rw(m_tc0150rod, FUNC(tc0150rod_device::word_r), FUNC(tc0150rod_device::word_w));
	map(0x500000, 0x503fff).ram(); /* network ram ? (see Gunbustr) */
}


void sci_state::racingb_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x103fff).ram();
	map(0x110000, 0x11ffff).ram().share("share1");
	map(0x300000, 0x30000f).rw(m_tc0510nio, FUNC(tc0510nio_device::halfword_wordswap_r), FUNC(tc0510nio_device::halfword_wordswap_w));
	map(0x300010, 0x30001f).r(FUNC(sci_state::dblaxle_steer_input_r));
	map(0x500002, 0x500003).w(FUNC(sci_state::cpua_ctrl_w));
	map(0x520001, 0x520001).w(m_tc0140syt, FUNC(tc0140syt_device::master_port_w));
	map(0x520003, 0x520003).rw(m_tc0140syt, FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x700000, 0x701fff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x900000, 0x90ffff).rw(m_tc0480scp, FUNC(tc0480scp_device::ram_r), FUNC(tc0480scp_device::ram_w));      /* tilemaps */
	map(0x930000, 0x93002f).rw(m_tc0480scp, FUNC(tc0480scp_device::ctrl_r), FUNC(tc0480scp_device::ctrl_w));
	map(0xb00000, 0xb03fff).ram().share("spriteram"); /* mostly unused ? */
	map(0xb08000, 0xb08001).rw(FUNC(sci_state::sci_spriteframe_r), FUNC(sci_state::sci_spriteframe_w)); /* alternates 0/0x100 */
}

void sci_state::racingb_cpub_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x400000, 0x403fff).ram();
	map(0x410000, 0x41ffff).ram().share("share1");
	map(0xa00000, 0xa01fff).rw(m_tc0150rod, FUNC(tc0150rod_device::word_r), FUNC(tc0150rod_device::word_w));
	map(0xd00000, 0xd03fff).ram(); /* network ram ? */
}


/***************************************************************************/

void taitoz_z80_sound_state::z80_sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).bankr("z80bank");
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe003).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write));
	map(0xe200, 0xe200).nopr().w(m_tc0140syt, FUNC(tc0140syt_device::slave_port_w));
	map(0xe201, 0xe201).rw(m_tc0140syt, FUNC(tc0140syt_device::slave_comm_r), FUNC(tc0140syt_device::slave_comm_w));
	map(0xe400, 0xe403).w(FUNC(taitoz_z80_sound_state::pancontrol_w)); /* pan */
	map(0xea00, 0xea00).nopr();
	map(0xee00, 0xee00).nopw(); /* ? */
	map(0xf000, 0xf000).nopw(); /* ? */
	map(0xf200, 0xf200).w(FUNC(taitoz_z80_sound_state::sound_bankswitch_w));
}


/***********************************************************
                   INPUT PORTS, DIPs
***********************************************************/

#define TAITO_Z_ANALOG_ADJUST( str ) \
	PORT_DIPNAME( 0xff, 0x80, str ) \
	PORT_DIPSETTING(    0x00, "-80" ) \
	PORT_DIPSETTING(    0x01, "-7F" ) \
	PORT_DIPSETTING(    0x02, "-7E" ) \
	PORT_DIPSETTING(    0x03, "-7D" ) \
	PORT_DIPSETTING(    0x04, "-7C" ) \
	PORT_DIPSETTING(    0x05, "-7B" ) \
	PORT_DIPSETTING(    0x06, "-7A" ) \
	PORT_DIPSETTING(    0x07, "-79" ) \
	PORT_DIPSETTING(    0x08, "-78" ) \
	PORT_DIPSETTING(    0x09, "-77" ) \
	PORT_DIPSETTING(    0x0a, "-76" ) \
	PORT_DIPSETTING(    0x0b, "-75" ) \
	PORT_DIPSETTING(    0x0c, "-74" ) \
	PORT_DIPSETTING(    0x0d, "-73" ) \
	PORT_DIPSETTING(    0x0e, "-72" ) \
	PORT_DIPSETTING(    0x0f, "-71" ) \
	PORT_DIPSETTING(    0x10, "-70" ) \
	PORT_DIPSETTING(    0x11, "-6F" ) \
	PORT_DIPSETTING(    0x12, "-6E" ) \
	PORT_DIPSETTING(    0x13, "-6D" ) \
	PORT_DIPSETTING(    0x14, "-6C" ) \
	PORT_DIPSETTING(    0x15, "-6B" ) \
	PORT_DIPSETTING(    0x16, "-6A" ) \
	PORT_DIPSETTING(    0x17, "-69" ) \
	PORT_DIPSETTING(    0x18, "-68" ) \
	PORT_DIPSETTING(    0x19, "-67" ) \
	PORT_DIPSETTING(    0x1a, "-66" ) \
	PORT_DIPSETTING(    0x1b, "-65" ) \
	PORT_DIPSETTING(    0x1c, "-64" ) \
	PORT_DIPSETTING(    0x1d, "-63" ) \
	PORT_DIPSETTING(    0x1e, "-62" ) \
	PORT_DIPSETTING(    0x1f, "-61" ) \
	PORT_DIPSETTING(    0x20, "-60" ) \
	PORT_DIPSETTING(    0x21, "-5F" ) \
	PORT_DIPSETTING(    0x22, "-5E" ) \
	PORT_DIPSETTING(    0x23, "-5D" ) \
	PORT_DIPSETTING(    0x24, "-5C" ) \
	PORT_DIPSETTING(    0x25, "-5B" ) \
	PORT_DIPSETTING(    0x26, "-5A" ) \
	PORT_DIPSETTING(    0x27, "-59" ) \
	PORT_DIPSETTING(    0x28, "-58" ) \
	PORT_DIPSETTING(    0x29, "-57" ) \
	PORT_DIPSETTING(    0x2a, "-56" ) \
	PORT_DIPSETTING(    0x2b, "-55" ) \
	PORT_DIPSETTING(    0x2c, "-54" ) \
	PORT_DIPSETTING(    0x2d, "-53" ) \
	PORT_DIPSETTING(    0x2e, "-52" ) \
	PORT_DIPSETTING(    0x2f, "-51" ) \
	PORT_DIPSETTING(    0x30, "-50" ) \
	PORT_DIPSETTING(    0x31, "-4F" ) \
	PORT_DIPSETTING(    0x32, "-4E" ) \
	PORT_DIPSETTING(    0x33, "-4D" ) \
	PORT_DIPSETTING(    0x34, "-4C" ) \
	PORT_DIPSETTING(    0x35, "-4B" ) \
	PORT_DIPSETTING(    0x36, "-4A" ) \
	PORT_DIPSETTING(    0x37, "-49" ) \
	PORT_DIPSETTING(    0x38, "-48" ) \
	PORT_DIPSETTING(    0x39, "-47" ) \
	PORT_DIPSETTING(    0x3a, "-46" ) \
	PORT_DIPSETTING(    0x3b, "-45" ) \
	PORT_DIPSETTING(    0x3c, "-44" ) \
	PORT_DIPSETTING(    0x3d, "-43" ) \
	PORT_DIPSETTING(    0x3e, "-42" ) \
	PORT_DIPSETTING(    0x3f, "-41" ) \
	PORT_DIPSETTING(    0x40, "-40" ) \
	PORT_DIPSETTING(    0x41, "-3F" ) \
	PORT_DIPSETTING(    0x42, "-3E" ) \
	PORT_DIPSETTING(    0x43, "-3D" ) \
	PORT_DIPSETTING(    0x44, "-3C" ) \
	PORT_DIPSETTING(    0x45, "-3B" ) \
	PORT_DIPSETTING(    0x46, "-3A" ) \
	PORT_DIPSETTING(    0x47, "-39" ) \
	PORT_DIPSETTING(    0x48, "-38" ) \
	PORT_DIPSETTING(    0x49, "-37" ) \
	PORT_DIPSETTING(    0x4a, "-36" ) \
	PORT_DIPSETTING(    0x4b, "-35" ) \
	PORT_DIPSETTING(    0x4c, "-34" ) \
	PORT_DIPSETTING(    0x4d, "-33" ) \
	PORT_DIPSETTING(    0x4e, "-32" ) \
	PORT_DIPSETTING(    0x4f, "-31" ) \
	PORT_DIPSETTING(    0x50, "-30" ) \
	PORT_DIPSETTING(    0x51, "-2F" ) \
	PORT_DIPSETTING(    0x52, "-2E" ) \
	PORT_DIPSETTING(    0x53, "-2D" ) \
	PORT_DIPSETTING(    0x54, "-2C" ) \
	PORT_DIPSETTING(    0x55, "-2B" ) \
	PORT_DIPSETTING(    0x56, "-2A" ) \
	PORT_DIPSETTING(    0x57, "-29" ) \
	PORT_DIPSETTING(    0x58, "-28" ) \
	PORT_DIPSETTING(    0x59, "-27" ) \
	PORT_DIPSETTING(    0x5a, "-26" ) \
	PORT_DIPSETTING(    0x5b, "-25" ) \
	PORT_DIPSETTING(    0x5c, "-24" ) \
	PORT_DIPSETTING(    0x5d, "-23" ) \
	PORT_DIPSETTING(    0x5e, "-22" ) \
	PORT_DIPSETTING(    0x5f, "-21" ) \
	PORT_DIPSETTING(    0x60, "-20" ) \
	PORT_DIPSETTING(    0x61, "-1F" ) \
	PORT_DIPSETTING(    0x62, "-1E" ) \
	PORT_DIPSETTING(    0x63, "-1D" ) \
	PORT_DIPSETTING(    0x64, "-1C" ) \
	PORT_DIPSETTING(    0x65, "-1B" ) \
	PORT_DIPSETTING(    0x66, "-1A" ) \
	PORT_DIPSETTING(    0x67, "-19" ) \
	PORT_DIPSETTING(    0x68, "-18" ) \
	PORT_DIPSETTING(    0x69, "-17" ) \
	PORT_DIPSETTING(    0x6a, "-16" ) \
	PORT_DIPSETTING(    0x6b, "-15" ) \
	PORT_DIPSETTING(    0x6c, "-14" ) \
	PORT_DIPSETTING(    0x6d, "-13" ) \
	PORT_DIPSETTING(    0x6e, "-12" ) \
	PORT_DIPSETTING(    0x6f, "-11" ) \
	PORT_DIPSETTING(    0x70, "-10" ) \
	PORT_DIPSETTING(    0x71, "-0F" ) \
	PORT_DIPSETTING(    0x72, "-0E" ) \
	PORT_DIPSETTING(    0x73, "-0D" ) \
	PORT_DIPSETTING(    0x74, "-0C" ) \
	PORT_DIPSETTING(    0x75, "-0B" ) \
	PORT_DIPSETTING(    0x76, "-0A" ) \
	PORT_DIPSETTING(    0x77, "-09" ) \
	PORT_DIPSETTING(    0x78, "-08" ) \
	PORT_DIPSETTING(    0x79, "-07" ) \
	PORT_DIPSETTING(    0x7a, "-06" ) \
	PORT_DIPSETTING(    0x7b, "-05" ) \
	PORT_DIPSETTING(    0x7c, "-04" ) \
	PORT_DIPSETTING(    0x7d, "-03" ) \
	PORT_DIPSETTING(    0x7e, "-02" ) \
	PORT_DIPSETTING(    0x7f, "-01" ) \
	PORT_DIPSETTING(    0x80, "+00" ) \
	PORT_DIPSETTING(    0x81, "+01" ) \
	PORT_DIPSETTING(    0x82, "+02" ) \
	PORT_DIPSETTING(    0x83, "+03" ) \
	PORT_DIPSETTING(    0x84, "+04" ) \
	PORT_DIPSETTING(    0x85, "+05" ) \
	PORT_DIPSETTING(    0x86, "+06" ) \
	PORT_DIPSETTING(    0x87, "+07" ) \
	PORT_DIPSETTING(    0x88, "+08" ) \
	PORT_DIPSETTING(    0x89, "+09" ) \
	PORT_DIPSETTING(    0x8a, "+0A" ) \
	PORT_DIPSETTING(    0x8b, "+0B" ) \
	PORT_DIPSETTING(    0x8c, "+0C" ) \
	PORT_DIPSETTING(    0x8d, "+0D" ) \
	PORT_DIPSETTING(    0x8e, "+0E" ) \
	PORT_DIPSETTING(    0x8f, "+0F" ) \
	PORT_DIPSETTING(    0x90, "+10" ) \
	PORT_DIPSETTING(    0x91, "+11" ) \
	PORT_DIPSETTING(    0x92, "+12" ) \
	PORT_DIPSETTING(    0x93, "+13" ) \
	PORT_DIPSETTING(    0x94, "+14" ) \
	PORT_DIPSETTING(    0x95, "+15" ) \
	PORT_DIPSETTING(    0x96, "+16" ) \
	PORT_DIPSETTING(    0x97, "+17" ) \
	PORT_DIPSETTING(    0x98, "+18" ) \
	PORT_DIPSETTING(    0x99, "+19" ) \
	PORT_DIPSETTING(    0x9a, "+1A" ) \
	PORT_DIPSETTING(    0x9b, "+1B" ) \
	PORT_DIPSETTING(    0x9c, "+1C" ) \
	PORT_DIPSETTING(    0x9d, "+1D" ) \
	PORT_DIPSETTING(    0x9e, "+1E" ) \
	PORT_DIPSETTING(    0x9f, "+1F" ) \
	PORT_DIPSETTING(    0xa0, "+20" ) \
	PORT_DIPSETTING(    0xa1, "+21" ) \
	PORT_DIPSETTING(    0xa2, "+22" ) \
	PORT_DIPSETTING(    0xa3, "+23" ) \
	PORT_DIPSETTING(    0xa4, "+24" ) \
	PORT_DIPSETTING(    0xa5, "+25" ) \
	PORT_DIPSETTING(    0xa6, "+26" ) \
	PORT_DIPSETTING(    0xa7, "+27" ) \
	PORT_DIPSETTING(    0xa8, "+28" ) \
	PORT_DIPSETTING(    0xa9, "+29" ) \
	PORT_DIPSETTING(    0xaa, "+2A" ) \
	PORT_DIPSETTING(    0xab, "+2B" ) \
	PORT_DIPSETTING(    0xac, "+2C" ) \
	PORT_DIPSETTING(    0xad, "+2D" ) \
	PORT_DIPSETTING(    0xae, "+2E" ) \
	PORT_DIPSETTING(    0xaf, "+2F" ) \
	PORT_DIPSETTING(    0xb0, "+30" ) \
	PORT_DIPSETTING(    0xb1, "+31" ) \
	PORT_DIPSETTING(    0xb2, "+32" ) \
	PORT_DIPSETTING(    0xb3, "+33" ) \
	PORT_DIPSETTING(    0xb4, "+34" ) \
	PORT_DIPSETTING(    0xb5, "+35" ) \
	PORT_DIPSETTING(    0xb6, "+36" ) \
	PORT_DIPSETTING(    0xb7, "+37" ) \
	PORT_DIPSETTING(    0xb8, "+38" ) \
	PORT_DIPSETTING(    0xb9, "+39" ) \
	PORT_DIPSETTING(    0xba, "+3A" ) \
	PORT_DIPSETTING(    0xbb, "+3B" ) \
	PORT_DIPSETTING(    0xbc, "+3C" ) \
	PORT_DIPSETTING(    0xbd, "+3D" ) \
	PORT_DIPSETTING(    0xbe, "+3E" ) \
	PORT_DIPSETTING(    0xbf, "+3F" ) \
	PORT_DIPSETTING(    0xc0, "+40" ) \
	PORT_DIPSETTING(    0xc1, "+41" ) \
	PORT_DIPSETTING(    0xc2, "+42" ) \
	PORT_DIPSETTING(    0xc3, "+43" ) \
	PORT_DIPSETTING(    0xc4, "+44" ) \
	PORT_DIPSETTING(    0xc5, "+45" ) \
	PORT_DIPSETTING(    0xc6, "+46" ) \
	PORT_DIPSETTING(    0xc7, "+47" ) \
	PORT_DIPSETTING(    0xc8, "+48" ) \
	PORT_DIPSETTING(    0xc9, "+49" ) \
	PORT_DIPSETTING(    0xca, "+4A" ) \
	PORT_DIPSETTING(    0xcb, "+4B" ) \
	PORT_DIPSETTING(    0xcc, "+4C" ) \
	PORT_DIPSETTING(    0xcd, "+4D" ) \
	PORT_DIPSETTING(    0xce, "+4E" ) \
	PORT_DIPSETTING(    0xcf, "+4F" ) \
	PORT_DIPSETTING(    0xd0, "+50" ) \
	PORT_DIPSETTING(    0xd1, "+51" ) \
	PORT_DIPSETTING(    0xd2, "+52" ) \
	PORT_DIPSETTING(    0xd3, "+53" ) \
	PORT_DIPSETTING(    0xd4, "+54" ) \
	PORT_DIPSETTING(    0xd5, "+55" ) \
	PORT_DIPSETTING(    0xd6, "+56" ) \
	PORT_DIPSETTING(    0xd7, "+57" ) \
	PORT_DIPSETTING(    0xd8, "+58" ) \
	PORT_DIPSETTING(    0xd9, "+59" ) \
	PORT_DIPSETTING(    0xda, "+5A" ) \
	PORT_DIPSETTING(    0xdb, "+5B" ) \
	PORT_DIPSETTING(    0xdc, "+5C" ) \
	PORT_DIPSETTING(    0xdd, "+5D" ) \
	PORT_DIPSETTING(    0xde, "+5E" ) \
	PORT_DIPSETTING(    0xdf, "+5F" ) \
	PORT_DIPSETTING(    0xe0, "+60" ) \
	PORT_DIPSETTING(    0xe1, "+61" ) \
	PORT_DIPSETTING(    0xe2, "+62" ) \
	PORT_DIPSETTING(    0xe3, "+63" ) \
	PORT_DIPSETTING(    0xe4, "+64" ) \
	PORT_DIPSETTING(    0xe5, "+65" ) \
	PORT_DIPSETTING(    0xe6, "+66" ) \
	PORT_DIPSETTING(    0xe7, "+67" ) \
	PORT_DIPSETTING(    0xe8, "+68" ) \
	PORT_DIPSETTING(    0xe9, "+69" ) \
	PORT_DIPSETTING(    0xea, "+6A" ) \
	PORT_DIPSETTING(    0xeb, "+6B" ) \
	PORT_DIPSETTING(    0xec, "+6C" ) \
	PORT_DIPSETTING(    0xed, "+6D" ) \
	PORT_DIPSETTING(    0xee, "+6E" ) \
	PORT_DIPSETTING(    0xef, "+6F" ) \
	PORT_DIPSETTING(    0xf0, "+70" ) \
	PORT_DIPSETTING(    0xf1, "+71" ) \
	PORT_DIPSETTING(    0xf2, "+72" ) \
	PORT_DIPSETTING(    0xf3, "+73" ) \
	PORT_DIPSETTING(    0xf4, "+74" ) \
	PORT_DIPSETTING(    0xf5, "+75" ) \
	PORT_DIPSETTING(    0xf6, "+76" ) \
	PORT_DIPSETTING(    0xf7, "+77" ) \
	PORT_DIPSETTING(    0xf8, "+78" ) \
	PORT_DIPSETTING(    0xf9, "+79" ) \
	PORT_DIPSETTING(    0xfa, "+7A" ) \
	PORT_DIPSETTING(    0xfb, "+7B" ) \
	PORT_DIPSETTING(    0xfc, "+7C" ) \
	PORT_DIPSETTING(    0xfd, "+7D" ) \
	PORT_DIPSETTING(    0xfe, "+7E" ) \
	PORT_DIPSETTING(    0xff, "+7F" )


static INPUT_PORTS_START( contcirc )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW A:1")
	PORT_DIPSETTING(    0x01, "Cockpit" )           // analog pedals
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )  // digital pedals, no brake?, allow free steering wheel
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Continue_Price ) )   PORT_DIPLOCATION("SW A:2")
	PORT_DIPSETTING(    0x02, "Same as Start" )
	PORT_DIPSETTING(    0x00, "Discount" )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW A:3" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW A:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	TAITO_COINAGE_WORLD_LOC(SW A)

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x03, "Difficulty 1 (time/speed)" ) PORT_DIPLOCATION("SW B:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Difficulty 2 (other cars)" ) PORT_DIPLOCATION("SW B:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x00, "Steering Wheel" )            PORT_DIPLOCATION("SW B:5") // no function in Cockpit cabinet?
	PORT_DIPSETTING(    0x10, "Free" )
	PORT_DIPSETTING(    0x00, "Locked" )
	PORT_DIPNAME( 0x20, 0x00, "3D Effect" )                 PORT_DIPLOCATION("SW B:6") // unlisted in manual
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW B:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW B:8" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(taitoz_state, gas_pedal_r) PORT_CONDITION("DSWA", 0x01, EQUALS, 0x01)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_NAME("Gas Switch") PORT_CONDITION("DSWA", 0x01, EQUALS, 0x00)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_NAME("Shifter") PORT_TOGGLE
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(taitoz_state, brake_pedal_r) PORT_CONDITION("DSWA", 0x01, EQUALS, 0x01)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_NAME("Brake Switch") PORT_CONDITION("DSWA", 0x01, EQUALS, 0x00) // no function?

	PORT_START("IN2")   /* unused */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("STEER")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x20, 0xe0) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_REVERSE PORT_NAME("Steering Wheel") PORT_CONDITION("DSWB", 0x10, EQUALS, 0x00)
	PORT_BIT( 0xffff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(2) PORT_REVERSE PORT_NAME("Steering Wheel") PORT_CONDITION("DSWB", 0x10, EQUALS, 0x10)

	PORT_START("GAS")
	PORT_BIT( 0x07, 0x00, IPT_PEDAL )  PORT_SENSITIVITY(25) PORT_KEYDELTA(1) PORT_NAME("Gas Pedal") PORT_CONDITION("DSWA", 0x01, EQUALS, 0x01)

	PORT_START("BRAKE")
	PORT_BIT( 0x07, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(25) PORT_KEYDELTA(1) PORT_NAME("Brake Pedal") PORT_CONDITION("DSWA", 0x01, EQUALS, 0x01)
INPUT_PORTS_END

static INPUT_PORTS_START( contcrcu )
	PORT_INCLUDE(contcirc)

	PORT_MODIFY("DSWA")     // confirmed
	TAITO_COINAGE_JAPAN_OLD_LOC(SW A)
INPUT_PORTS_END

static INPUT_PORTS_START( contcrcj )
	PORT_INCLUDE(contcirc)

	PORT_MODIFY("DSWA")     // confirmed correct for US set as well
	TAITO_COINAGE_JAPAN_OLD_LOC(SW A)

	PORT_MODIFY("DSWB")
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW B:6" ) // Japan & US set 2 CANNOT turn off 3D effect!!
INPUT_PORTS_END

static INPUT_PORTS_START( chasehq ) // IN3-6 perhaps used with cockpit setup? //
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW A:1,2") /* US Manual states DIPS 1 & 2 "MUST REMAIN OFF" */
	PORT_DIPSETTING(    0x03, "Upright / Steering Lock" )           // digital pedals, locked steering wheel
	PORT_DIPSETTING(    0x02, "Upright / No Steering Lock" )        // digital pedals, free steering wheel
	PORT_DIPSETTING(    0x01, "Full Throttle Convert, Cockpit" )    // analog pedals, locked steering wheel
	PORT_DIPSETTING(    0x00, "Full Throttle Convert, Deluxe" )     // analog pedals, locked steering wheel, motor
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW A:3" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW A:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	TAITO_COINAGE_WORLD_LOC(SW A)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW B)
	PORT_DIPNAME( 0x0c, 0x0c, "Timer Setting" )             PORT_DIPLOCATION("SW B:3,4")
	PORT_DIPSETTING(    0x08, "70 Seconds" )
	PORT_DIPSETTING(    0x04, "65 Seconds" )
	PORT_DIPSETTING(    0x0c, "60 Seconds" )
	PORT_DIPSETTING(    0x00, "55 Seconds" )
	PORT_DIPNAME( 0x10, 0x10, "Turbos Stocked" )            PORT_DIPLOCATION("SW B:5")
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Continue_Price ) )   PORT_DIPLOCATION("SW B:6")
	PORT_DIPSETTING(    0x20, "Same as Start" )
	PORT_DIPSETTING(    0x00, "Discount" )
	PORT_DIPNAME( 0x40, 0x40, "Clear Damage on Continue" )  PORT_DIPLOCATION("SW B:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW B:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(taitoz_state, brake_pedal_r) PORT_CONDITION("DSWA", 0x02, EQUALS, 0x00)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_NAME("Brake Switch") PORT_CONDITION("DSWA", 0x02, EQUALS, 0x02)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_NAME("Turbo")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SERVICE2 ) PORT_NAME("Calibrate") // ?
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON4 ) PORT_NAME("Shifter") PORT_TOGGLE
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(taitoz_state, gas_pedal_r) PORT_CONDITION("DSWA", 0x02, EQUALS, 0x00)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_NAME("Gas Switch") PORT_CONDITION("DSWA", 0x02, EQUALS, 0x02)

	PORT_START("IN2")   /* unused */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("UNK1")  /* ??? */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK2")  /* ??? */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK3")  /* ??? */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK4")  /* ??? */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("STEER")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x20, 0xe0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_NAME("Steering Wheel") PORT_CONDITION("DSWA", 0x03, NOTEQUALS, 0x02)
	PORT_BIT( 0xffff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(2) PORT_NAME("Steering Wheel") PORT_CONDITION("DSWA", 0x03, EQUALS, 0x02)

	PORT_START("GAS")
	PORT_BIT( 0x07, 0x00, IPT_PEDAL )  PORT_SENSITIVITY(25) PORT_KEYDELTA(1) PORT_NAME("Gas Pedal") PORT_CONDITION("DSWA", 0x02, EQUALS, 0x00)

	PORT_START("BRAKE")
	PORT_BIT( 0x07, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(25) PORT_KEYDELTA(1) PORT_NAME("Brake Pedal") PORT_CONDITION("DSWA", 0x02, EQUALS, 0x00)
INPUT_PORTS_END

static INPUT_PORTS_START( chasehqj )
	PORT_INCLUDE(chasehq)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SW A)
INPUT_PORTS_END

static INPUT_PORTS_START( enforce )
	PORT_START("DSWA")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW A:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW A:2" )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW A:3" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW A:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )      // Says SHIFT HI in test mode !?
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )       // Says SHIFT LO in test mode !?
	TAITO_COINAGE_WORLD_LOC(SW A)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW B)
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW B:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW B:4" )
	PORT_DIPNAME( 0x10, 0x00, "Background scenery" ) PORT_DIPLOCATION("SW B:5")
	PORT_DIPSETTING(    0x10, "Crazy scrolling" )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW B:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW B:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW B:8" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Gatling Gun") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Laser") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)

	PORT_START("IN2")   /* unused */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( enforcej )
	PORT_INCLUDE(enforce)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SW A)
INPUT_PORTS_END

static INPUT_PORTS_START( enforceja )
	PORT_INCLUDE(enforce)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(taitoz_state, adstick_r<0>);

	PORT_MODIFY("IN1")
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(taitoz_state, adstick_r<1>);

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Shifter") PORT_TOGGLE PORT_PLAYER(1)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SW A)

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x30, 0x30, "3D Effects" )                 PORT_DIPLOCATION("SW B:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, "In Game Only" )
	PORT_DIPSETTING(    0x10, "In Game Only (duplicate)" )

	PORT_START("STICKX")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(20) PORT_KEYDELTA(4) PORT_PLAYER(1)

	PORT_START("STICKY")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(20) PORT_KEYDELTA(4) PORT_PLAYER(1)
INPUT_PORTS_END

static INPUT_PORTS_START( bshark )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "Mirror screen" ) PORT_DIPLOCATION("SW A:1")  // manual says it must be off
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW A:2" )   // manual says it must be off
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW A:3" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW A:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	TAITO_COINAGE_WORLD_LOC(SW A)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW B)
	PORT_DIPNAME( 0x0c, 0x04, "Speed of Sight" ) PORT_DIPLOCATION("SW B:3,4")
	PORT_DIPSETTING(    0x0c, "Slow" )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, "Fast" )
	PORT_DIPSETTING(    0x00, "Fastest" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW B:5" )   // manual says all these must be off
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW B:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW B:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW B:8" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")   /* unused */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")   /* b2-5 affect sound num in service mode but otherwise useless (?) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) /* "Fire" */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) /* same as "Fire" */

	PORT_START("STICKX")    /* values chosen to match allowed crosshair area */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x35, 0xcc) PORT_SENSITIVITY(20) PORT_KEYDELTA(4) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("X_ADJUST")  /* declare as DIP SWITCH instead of VARIABLE REGISTER */
	TAITO_Z_ANALOG_ADJUST( "Adjust Stick H (VARIABLE REGISTER)" )

	PORT_START("STICKY")    /* values chosen to match allowed crosshair area */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x32, 0xd5) PORT_SENSITIVITY(20) PORT_KEYDELTA(4) PORT_PLAYER(1)

	PORT_START("Y_ADJUST")  /* declare as DIP SWITCH instead of VARIABLE REGISTER */
	TAITO_Z_ANALOG_ADJUST( "Adjust Stick V (VARIABLE REGISTER)" )
INPUT_PORTS_END

static INPUT_PORTS_START( bsharku )
	PORT_INCLUDE(bshark)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_US_LOC(SW A)
INPUT_PORTS_END

static INPUT_PORTS_START( bsharkj )
	PORT_INCLUDE(bshark)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SW A)
INPUT_PORTS_END

static INPUT_PORTS_START( bsharkjjs )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "Mirror screen" ) PORT_DIPLOCATION("SW A:1")  // manual says it must be off
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW A:2" )   // manual says it must be off
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW A:3" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW A:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	TAITO_COINAGE_JAPAN_OLD_LOC(SW A)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW B)
	PORT_DIPNAME( 0x0c, 0x04, "Speed of Sight" ) PORT_DIPLOCATION("SW B:3,4")
	PORT_DIPSETTING(    0x0c, "Slow" )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, "Fast" )
	PORT_DIPSETTING(    0x00, "Fastest" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW B:5" )   // manual says all these must be off
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW B:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW B:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW B:8" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")   /* unused */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Fire") PORT_PLAYER(1)   /* "Fire" */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Warp") PORT_PLAYER(1)   /* Same as Fire, but called "Warp" in Service Mode */
INPUT_PORTS_END

static INPUT_PORTS_START( sci )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW A:1")
	PORT_DIPSETTING(    0x01, "Cockpit" )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW A:2" ) /* Manual states "MUST REMAIN OFF" */
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW A:3" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW A:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	TAITO_COINAGE_WORLD_LOC(SW A)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW B)
	PORT_DIPNAME( 0x0c, 0x0c, "Timer Setting" )             PORT_DIPLOCATION("SW B:3,4")
	PORT_DIPSETTING(    0x08, "70 Seconds" )
	PORT_DIPSETTING(    0x04, "65 Seconds" )
	PORT_DIPSETTING(    0x0c, "60 Seconds" )
	PORT_DIPSETTING(    0x00, "55 Seconds" )
	PORT_DIPNAME( 0x10, 0x10, "Turbos Stocked" )            PORT_DIPLOCATION("SW B:5")
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x20, 0x20, "Steering Radius" )           PORT_DIPLOCATION("SW B:6")
	PORT_DIPSETTING(    0x00, "270 Degree" )
	PORT_DIPSETTING(    0x20, "360 Degree" )
	PORT_DIPNAME( 0x40, 0x40, "Clear Damage on Continue" )  PORT_DIPLOCATION("SW B:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Siren Volume" )              PORT_DIPLOCATION("SW B:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Gun Trigger")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Brake Switch")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_NAME("Turbo")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SERVICE2 ) PORT_NAME("Center")
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON4 ) PORT_NAME("Shifter") PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_NAME("Gas Switch")
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("IN2")   /* unused */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("STEER") /* steering */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x20, 0xe0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_NAME("Steering Wheel")
INPUT_PORTS_END

static INPUT_PORTS_START( sciu )
	PORT_INCLUDE(sci)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_US_LOC(SW A)
INPUT_PORTS_END

static INPUT_PORTS_START( scij )
	PORT_INCLUDE(sci)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SW A)
INPUT_PORTS_END

static INPUT_PORTS_START( nightstr )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW A:1")
	PORT_DIPSETTING(    0x01, "Cockpit" )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW A:2" )   // manual says it must be off
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW A:3" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW A:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	TAITO_COINAGE_WORLD_LOC(SW A)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW B)
	PORT_DIPNAME( 0x0c, 0x0c, "Bonus Shields" ) PORT_DIPLOCATION("SW B:3,4")
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x30, "Shields" ) PORT_DIPLOCATION("SW B:5,6")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x30, "5" )
	PORT_DIPSETTING(    0x20, "6" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW B:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Trigger Turbo" ) PORT_DIPLOCATION("SW B:8")
	PORT_DIPSETTING(    0x80, "7 Shots / Second" )
	PORT_DIPSETTING(    0x00, "10 Shots / Second" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_TILT )

	PORT_START("IN1")   /* unused */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)

	PORT_START("STICKX")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(60) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("STICKY")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(60) PORT_KEYDELTA(15) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("X_ADJUST")  /* declare as DIP SWITCH instead of VARIABLE REGISTER */
	TAITO_Z_ANALOG_ADJUST( "Adjust Stick H (VARIABLE REGISTER)" )

	PORT_START("Y_ADJUST")  /* declare as DIP SWITCH instead of VARIABLE REGISTER */
	TAITO_Z_ANALOG_ADJUST( "Adjust Stick V (VARIABLE REGISTER)" )
INPUT_PORTS_END

static INPUT_PORTS_START( nghtstrj )
	PORT_INCLUDE( nightstr )

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SW A)
INPUT_PORTS_END

static INPUT_PORTS_START( nghtstru )
	PORT_INCLUDE( nightstr )

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_US_LOC(SW A)
INPUT_PORTS_END

static INPUT_PORTS_START( aquajack )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW A:1")
	PORT_DIPSETTING(    0x80, "Cockpit" )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW A:2" ) /* Dip 2 shown as "Must Remain in "Off" Position" in manual */
	PORT_SERVICE_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW A:3" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW A:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW A:6,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW A:8,7")  /* The Romstar (US version) manual list this as "Continue Pricing" */
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )    /* Same pricing as Coin A */
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )    /* 1 Coin to Continue  */
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_4C ) )    /* 2 Coins to Continue */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )    /* 3 Coins to Continue */

	PORT_START("DSWB")
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW B:2,1")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW B:4,3")
	PORT_DIPSETTING(    0x00, "30k" )
	PORT_DIPSETTING(    0x30, "50k" )
	PORT_DIPSETTING(    0x10, "80k" )
	PORT_DIPSETTING(    0x20, "100k" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW B:6,5")
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW B:7" ) /* Dip 7 shown as "Do Not Touch" in manuals */
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW B:8" ) /* Dip 8 */
	/* The previous taito_z.c said ---
	    The Romstar (US version) manual list this as "Endless Game" - Has no effect on "World" version
	   --- , and declared it as unused switch.
	*/
	/* PORT_DIPNAME( 0x01, 0x01, "Endless Game" ) PORT_DIPLOCATION("SW B:8") */
	/* PORT_DIPSETTING(    0x01, "Normal Game" ) */
	/* PORT_DIPSETTING(    0x00, "Endless Game" ) */

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")   /* what is it ??? */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1)
INPUT_PORTS_END

static INPUT_PORTS_START( aquajckj )
	PORT_INCLUDE(aquajack)

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW A:6,5")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW A:8,7")
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
INPUT_PORTS_END

static INPUT_PORTS_START( spacegun )
	PORT_START("DSWA")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW A:1" )   // Manual says Always Off
	PORT_DIPNAME( 0x02, 0x02, "Always have gunsight power up" ) PORT_DIPLOCATION("SW A:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW A:3" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW A:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	TAITO_COINAGE_WORLD_LOC(SW A)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW B)
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW B:3" )   // Manual lists dips 3 through 6 and 8 as Always off
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW B:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW B:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW B:6" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW B:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Disable Pedal (?)" ) PORT_DIPLOCATION("SW B:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2)

	PORT_START("IN1")   /* unused */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)

	PORT_START("STICKX1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(13) PORT_CENTERDELTA(0) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("STICKY1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(13) PORT_CENTERDELTA(0) PORT_PLAYER(1)

	PORT_START("STICKX2")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(13) PORT_CENTERDELTA(0) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("STICKY2")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(13) PORT_CENTERDELTA(0) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( spacegnj )
	PORT_INCLUDE( spacegun )

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SW A)
INPUT_PORTS_END

static INPUT_PORTS_START( spacegnu )
	PORT_INCLUDE( spacegun )

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_US_LOC(SW A)
INPUT_PORTS_END

static INPUT_PORTS_START( dblaxle ) // Side by Side linkable versions
	PORT_START("DSWA")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW A:1" )
	PORT_DIPNAME( 0x02, 0x02, "Gear shift" )            PORT_DIPLOCATION("SW A:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Inverted" )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW A:3" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW A:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	TAITO_COINAGE_US_LOC(SW A)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW B)
	PORT_DIPNAME( 0x04, 0x00, "Network?" )              PORT_DIPLOCATION("SW B:3") // doesn't boot if on
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Player Truck" )          PORT_DIPLOCATION("SW B:4")
	PORT_DIPSETTING(    0x08, "Red" )
	PORT_DIPSETTING(    0x00, "Blue" )
	PORT_DIPNAME( 0x10, 0x10, "Back button" )           PORT_DIPLOCATION("SW B:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Inverted" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW B:6" ) // causes "Root CPU Error" on "Icy Road" (Tourniquet)
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW B:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW B:8" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Shifter") PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Brake Switch")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Reverse")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Nitro")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Center")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Gas Switch")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")   /* unused */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("STEER")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x40, 0xc0) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_NAME("Steering Wheel")
INPUT_PORTS_END

static INPUT_PORTS_START( dblaxles ) // Single player versions
	PORT_INCLUDE(dblaxle)

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "Handle Pulse" ) PORT_DIPLOCATION("SW A:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Fast" )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x04, 0x00, "Back Gear" )           PORT_DIPLOCATION("SW B:3") // If set to NORMAL you need to keep REVERSE pressed to go forward
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "No Back Gear" )
	PORT_DIPNAME( 0x08, 0x08, "Vibration Mode" ) PORT_DIPLOCATION("SW B:4")
	PORT_DIPSETTING(    0x08, "Partial Vibration" )
	PORT_DIPSETTING(    0x00, "All The Time Vibration" )
	PORT_DIPNAME( 0x10, 0x10, "Steering Wheel Vibration" ) PORT_DIPLOCATION("SW B:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Select Round" ) PORT_DIPLOCATION("SW B:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW B:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Buy-In" ) PORT_DIPLOCATION("SW B:8") // manual states "In countries except North America this setting should be ON"
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) ) // You buy truck upgrades like Big Tires, Engine and Nitro canisters before the race
INPUT_PORTS_END

static INPUT_PORTS_START( pwheelsj )
	PORT_INCLUDE(dblaxle)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SW A)
INPUT_PORTS_END

static INPUT_PORTS_START( racingb )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW A:1") // don't know what is what
	PORT_DIPSETTING(    0x00, "Type 0" ) // free steering wheel
	PORT_DIPSETTING(    0x01, "Type 1" ) // locked steering wheel
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW A:2" )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW A:3" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW A:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	TAITO_COINAGE_WORLD_LOC(SW A)

	PORT_START("DSWB")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW B:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW B:2" )
	PORT_DIPNAME( 0x04, 0x04, "Steering Wheel Range" )  PORT_DIPLOCATION("SW B:3") // no function in Type 0 cabinet?
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x08, 0x08, "Steering Wheel Type" )   PORT_DIPLOCATION("SW B:4") // no function in Type 0 cabinet?
	PORT_DIPSETTING(    0x00, "Free" )
	PORT_DIPSETTING(    0x08, "Locked" )
	PORT_DIPNAME( 0x10, 0x10, "Network" )               PORT_DIPLOCATION("SW B:5") // gives a LAN error
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Player Car" )            PORT_DIPLOCATION("SW B:6,7")
	PORT_DIPSETTING(    0x60, "Red" )
	PORT_DIPSETTING(    0x40, "Blue" )
	PORT_DIPSETTING(    0x20, "Green" )
	PORT_DIPSETTING(    0x00, "Yellow" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW B:8" ) // affects car color too?

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Shifter") PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Brake Switch")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Pit In")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Center")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Gas Switch")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")   /* unused */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("STEER")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_NAME("Steering Wheel") PORT_CONDITION("DSWB", 0x08, EQUALS, 0x08)
	PORT_BIT( 0xffff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_NAME("Steering Wheel") PORT_CONDITION("DSWB", 0x08, EQUALS, 0x00)
INPUT_PORTS_END


/***********************************************************
                       GFX DECODING
***********************************************************/

static const gfx_layout tile16x8_layout =
{
	16,8,   /* 16*8 sprites */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ STEP4(0,16) },
	{ STEP16(0,1) },
	{ STEP8(0,16*4) },
	64*8    /* every sprite takes 64 consecutive bytes */
};

static const gfx_layout tile16x16_layout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ STEP4(0,16) },
	{ STEP16(0,1) },
	{ STEP16(0,16*4) },
	64*16   /* every sprite takes 128 consecutive bytes */
};

static GFXDECODE_START( gfx_taitoz )
	GFXDECODE_ENTRY( "sprites", 0x0, tile16x8_layout, 0, 256 )    /* sprite parts */
GFXDECODE_END

static GFXDECODE_START( gfx_chasehq )
	GFXDECODE_ENTRY( "sprites",  0x0, tile16x16_layout, 0, 256 )   /* sprite parts */
	GFXDECODE_ENTRY( "sprites2", 0x0, tile16x16_layout, 0, 256 )   /* sprite parts */
GFXDECODE_END


/***********************************************************
                      MACHINE DRIVERS

CPU Interleaving
----------------

Chasehq2 needs high interleaving to have sound (not checked
since May 2001 - may have changed).

Enforce with interleave of 1 sometimes lets you take over from
the demo game when you coin up! Set to 10 seems to cure this.

Bshark needs the high cpu interleaving to run test mode.

Nightstr needs the high cpu interleaving to get through init.

Aquajack has it VERY high to cure frequent sound-related
hangs.

Dblaxle has 10 to boot up reliably but very occasionally gets
a "root cpu error" still.

Racingb inherited interleave from Dblaxle - other values not
tested!

Mostly it's the 2nd 68K which writes to road chip, so syncing
between it and the master 68K may be important. Contcirc
and ChaseHQ have interleave of only 1 - possible cause of
Contcirc road glitchiness in attract?

***********************************************************/

/***********************************************************
                   SAVE STATES
***********************************************************/

void taitoz_state::device_post_load()
{
	parse_cpu_control();
}

void taitoz_state::machine_start()
{
	save_item(NAME(m_cpua_ctrl));
}

void taitoz_z80_sound_state::machine_start()
{
	taitoz_state::machine_start();

	int banks = memregion("audiocpu")->bytes() / 0x4000;
	m_z80bank->configure_entries(0, banks, memregion("audiocpu")->base(), 0x4000);
}

void contcirc_state::machine_start()
{
	taitoz_z80_sound_state::machine_start();

	m_shutter_out.resolve();

	m_shutter_toggle = 0;
	m_shutter_control = 0;

	save_item(NAME(m_shutter_toggle));
	save_item(NAME(m_shutter_control));
}

void chasehq_state::machine_start()
{
	taitoz_z80_sound_state::machine_start();

	m_lamps.resolve();
}

void sci_state::machine_start()
{
	taitoz_z80_sound_state::machine_start();

	save_item(NAME(m_sci_int6));
}

void nightstr_state::machine_start()
{
	taitoz_z80_sound_state::machine_start();

	m_motor_dir.resolve();
	m_motor_speed.resolve();
	m_motor_debug.resolve();
}

void spacegun_state::machine_start()
{
	taitoz_state::machine_start();

	m_recoil.resolve();

	m_eep_latch = 0;

	save_item(NAME(m_eep_latch));
}

void dblaxle_state::machine_start()
{
	taitoz_z80_sound_state::machine_start();

	m_wheel_vibration.resolve();
}

void taitoz_state::machine_reset()
{
	m_cpua_ctrl = 0xff;
}

void sci_state::machine_reset()
{
	taitoz_z80_sound_state::machine_reset();

	m_sci_int6 = 0;
}

void taitoz_state::screen_config(machine_config &config, int vdisp_start, int vdisp_end)
{
//  26.860 MHz comes from the video board, assume /4 pixel clock
//  contcirc and enforce uses a narrower visible area (checked via service mode),
//  confirm if they outputs the same syncs.
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(26'686'000)/4, 424, 0, 320, 262, vdisp_start, vdisp_end);
}

void contcirc_state::contcirc(machine_config &config) //OSC: 26.686, 24.000, 16.000
{
	M68000(config, m_maincpu, XTAL(24'000'000)/2);   // MC68000P12 @ 12 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &contcirc_state::contcirc_map);
	m_maincpu->set_vblank_int("screen", FUNC(contcirc_state::irq6_line_hold));

	Z80(config, m_audiocpu, XTAL(16'000'000)/4);    // Z0840004PSC @ 4 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &contcirc_state::z80_sound_map);

	M68000(config, m_subcpu, XTAL(24'000'000)/2);   // MC68000P12 @ 12 MHz
	m_subcpu->set_addrmap(AS_PROGRAM, &contcirc_state::contcirc_cpub_map);
	m_subcpu->set_vblank_int("screen", FUNC(contcirc_state::irq6_line_hold));

	TC0040IOC(config, m_tc0040ioc, 0);
	m_tc0040ioc->read_0_callback().set_ioport("DSWA");
	m_tc0040ioc->read_1_callback().set_ioport("DSWB");
	m_tc0040ioc->read_2_callback().set_ioport("IN0");
	m_tc0040ioc->read_3_callback().set_ioport("IN1");
	m_tc0040ioc->write_4_callback().set(FUNC(contcirc_state::coin_control_w));
	m_tc0040ioc->read_7_callback().set_ioport("IN2");

	screen_config(config, 24, 248);
	m_screen->set_screen_update(FUNC(contcirc_state::screen_update_contcirc));
	m_screen->set_palette(m_tc0110pcr);
	m_screen->screen_vblank().set(FUNC(contcirc_state::scope_vblank));

	GFXDECODE(config, m_gfxdecode, m_tc0110pcr, gfx_taitoz);

	TC0100SCN(config, m_tc0100scn, 0);
	m_tc0100scn->set_palette(m_tc0110pcr);

	TC0150ROD(config, m_tc0150rod, 0);

	TC0110PCR(config, m_tc0110pcr, 0);

	/* sound hardware */
	SPEAKER(config, "front").front_center();
	SPEAKER(config, "rear").rear_center();
	SPEAKER(config, "subwoofer").set_position(0.0, 0.0, 0.0); // FIXME: where is this speaker located?

	ym2610_device &ymsnd(YM2610(config, "ymsnd", XTAL(16'000'000)/2));    // 8 MHz
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "subwoofer", 0.20);
	ymsnd.add_route(1, "2610.1.l", 2.0);
	ymsnd.add_route(1, "2610.1.r", 2.0);
	ymsnd.add_route(2, "2610.2.l", 2.0);
	ymsnd.add_route(2, "2610.2.r", 2.0);

	FILTER_VOLUME(config, "2610.1.r").add_route(ALL_OUTPUTS, "rear", 1.0);
	FILTER_VOLUME(config, "2610.1.l").add_route(ALL_OUTPUTS, "front", 1.0);
	FILTER_VOLUME(config, "2610.2.r").add_route(ALL_OUTPUTS, "rear", 1.0);
	FILTER_VOLUME(config, "2610.2.l").add_route(ALL_OUTPUTS, "front", 1.0);

	TC0140SYT(config, m_tc0140syt, 0);
	m_tc0140syt->set_master_tag(m_subcpu);
	m_tc0140syt->set_slave_tag(m_audiocpu);
}

void chasehq_state::chasehq(machine_config &config) //OSC: 26.686, 24.000, 16.000
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000)/2);   // MC68000P12 @ 12 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &chasehq_state::chasehq_map);
	m_maincpu->set_vblank_int("screen", FUNC(chasehq_state::irq4_line_hold));

	Z80(config, m_audiocpu, XTAL(16'000'000)/4);    // Z0840004PSC @ 4 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &chasehq_state::z80_sound_map);

	M68000(config, m_subcpu, XTAL(24'000'000)/2);   // MC68000P12 @ 12 MHz
	m_subcpu->set_addrmap(AS_PROGRAM, &chasehq_state::chasehq_cpub_map);
	m_subcpu->set_vblank_int("screen", FUNC(chasehq_state::irq4_line_hold));

	TC0040IOC(config, m_tc0040ioc, 0);
	m_tc0040ioc->read_0_callback().set_ioport("DSWA");
	m_tc0040ioc->read_1_callback().set_ioport("DSWB");
	m_tc0040ioc->read_2_callback().set_ioport("IN0");
	m_tc0040ioc->read_3_callback().set_ioport("IN1");
	m_tc0040ioc->write_4_callback().set(FUNC(chasehq_state::coin_control_w));
	m_tc0040ioc->read_7_callback().set_ioport("IN2");

	/* video hardware */
	screen_config(config, 16, 256);
	m_screen->set_screen_update(FUNC(chasehq_state::screen_update_chasehq));
	m_screen->set_palette(m_tc0110pcr);

	GFXDECODE(config, m_gfxdecode, m_tc0110pcr, gfx_chasehq);

	TC0100SCN(config, m_tc0100scn, 0);
	m_tc0100scn->set_palette(m_tc0110pcr);

	TC0150ROD(config, m_tc0150rod, 0);

	TC0110PCR(config, m_tc0110pcr, 0);

	/* sound hardware */
	SPEAKER(config, "front").front_center();
	SPEAKER(config, "rear").rear_center();
	SPEAKER(config, "subwoofer").set_position(0.0, 0.0, 0.0); // FIXME: where is this speaker located?

	ym2610_device &ymsnd(YM2610(config, "ymsnd", XTAL(16'000'000)/2));    // 8 MHz
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "subwoofer", 0.20);
	ymsnd.add_route(1, "2610.1.l", 1.0);
	ymsnd.add_route(1, "2610.1.r", 1.0);
	ymsnd.add_route(2, "2610.2.l", 1.0);
	ymsnd.add_route(2, "2610.2.r", 1.0);

	FILTER_VOLUME(config, "2610.1.r").add_route(ALL_OUTPUTS, "rear", 1.0);
	FILTER_VOLUME(config, "2610.1.l").add_route(ALL_OUTPUTS, "front", 1.0);
	FILTER_VOLUME(config, "2610.2.r").add_route(ALL_OUTPUTS, "rear", 1.0);
	FILTER_VOLUME(config, "2610.2.l").add_route(ALL_OUTPUTS, "front", 1.0);

	TC0140SYT(config, m_tc0140syt, 0);
	m_tc0140syt->set_master_tag(m_subcpu);
	m_tc0140syt->set_slave_tag(m_audiocpu);
}

void contcirc_state::enforce(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000)/2);   // MC68000P12 @ 12 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &contcirc_state::enforce_map);
	m_maincpu->set_vblank_int("screen", FUNC(contcirc_state::irq6_line_hold));

	Z80(config, m_audiocpu, XTAL(16'000'000)/4);    // Z0840004PSC @ 4 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &contcirc_state::z80_sound_map);

	M68000(config, m_subcpu, XTAL(24'000'000)/2);   // MC68000P12 @ 12 MHz
	m_subcpu->set_addrmap(AS_PROGRAM, &contcirc_state::enforce_cpub_map);
	m_subcpu->set_vblank_int("screen", FUNC(contcirc_state::irq6_line_hold));

	config.set_maximum_quantum(attotime::from_hz(600));

	TC0040IOC(config, m_tc0040ioc, 0);
	m_tc0040ioc->read_0_callback().set_ioport("DSWA");
	m_tc0040ioc->read_1_callback().set_ioport("DSWB");
	m_tc0040ioc->read_2_callback().set_ioport("IN0");
	m_tc0040ioc->read_3_callback().set_ioport("IN1");
	m_tc0040ioc->write_4_callback().set(FUNC(contcirc_state::coin_control_w));
	m_tc0040ioc->read_7_callback().set_ioport("IN2");

	/* video hardware */
	screen_config(config, 24, 248);
	m_screen->set_screen_update(FUNC(contcirc_state::screen_update_contcirc));
	m_screen->set_palette(m_tc0110pcr);
	m_screen->screen_vblank().set(FUNC(contcirc_state::scope_vblank));

	GFXDECODE(config, m_gfxdecode, m_tc0110pcr, gfx_taitoz);

	TC0100SCN(config, m_tc0100scn, 0);
	m_tc0100scn->set_palette(m_tc0110pcr);

	TC0150ROD(config, m_tc0150rod, 0);

	TC0110PCR(config, m_tc0110pcr, 0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ym2610_device &ymsnd(YM2610(config, "ymsnd", XTAL(16'000'000)/2));    // 8 MHz
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "lspeaker", 0.25);
	ymsnd.add_route(0, "rspeaker", 0.25);
	ymsnd.add_route(1, "2610.1.l", 20.0);
	ymsnd.add_route(1, "2610.1.r", 20.0);
	ymsnd.add_route(2, "2610.2.l", 20.0);
	ymsnd.add_route(2, "2610.2.r", 20.0);

	FILTER_VOLUME(config, "2610.1.r").add_route(ALL_OUTPUTS, "rspeaker", 1.0);
	FILTER_VOLUME(config, "2610.1.l").add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	FILTER_VOLUME(config, "2610.2.r").add_route(ALL_OUTPUTS, "rspeaker", 1.0);
	FILTER_VOLUME(config, "2610.2.l").add_route(ALL_OUTPUTS, "lspeaker", 1.0);

	TC0140SYT(config, m_tc0140syt, 0);
	m_tc0140syt->set_master_tag(m_subcpu);
	m_tc0140syt->set_slave_tag(m_audiocpu);
}

void taitoz_state::bshark_base(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 12000000);   /* 12 MHz ??? */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitoz_state::bshark_map);
	m_maincpu->set_vblank_int("screen", FUNC(taitoz_state::irq4_line_hold));

	M68000(config, m_subcpu, 12000000);   /* 12 MHz ??? */
	m_subcpu->set_addrmap(AS_PROGRAM, &taitoz_state::bshark_cpub_map);
	m_subcpu->set_vblank_int("screen", FUNC(taitoz_state::irq4_line_hold));

	config.set_maximum_quantum(attotime::from_hz(6000));

	TC0220IOC(config, m_tc0220ioc, 0);
	m_tc0220ioc->read_0_callback().set_ioport("DSWA");
	m_tc0220ioc->read_1_callback().set_ioport("DSWB");
	m_tc0220ioc->read_2_callback().set_ioport("IN0");
	m_tc0220ioc->read_3_callback().set_ioport("IN1");
	m_tc0220ioc->write_4_callback().set(FUNC(taitoz_state::coin_control_w));
	m_tc0220ioc->read_7_callback().set_ioport("IN2");

	screen_config(config, 16, 256);
	m_screen->set_screen_update(FUNC(taitoz_state::screen_update_bshark));
	m_screen->set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_taitoz);
	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 4096);

	TC0100SCN(config, m_tc0100scn, 0);
	m_tc0100scn->set_palette("palette");

	TC0150ROD(config, m_tc0150rod, 0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 16000000/2));
	//ymsnd.irq_handler().set_inputline(m_audiocpu, 0); // DG: this is probably specific to Z80 and wrong?
	ymsnd.add_route(0, "lspeaker", 0.25);
	ymsnd.add_route(0, "rspeaker", 0.25);
	ymsnd.add_route(1, "2610.1.l", 28.0);
	ymsnd.add_route(1, "2610.1.r", 28.0);
	ymsnd.add_route(2, "2610.2.l", 28.0);
	ymsnd.add_route(2, "2610.2.r", 28.0);

	FILTER_VOLUME(config, "2610.1.r").add_route(ALL_OUTPUTS, "rspeaker", 1.0);
	FILTER_VOLUME(config, "2610.1.l").add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	FILTER_VOLUME(config, "2610.2.r").add_route(ALL_OUTPUTS, "rspeaker", 1.0);
	FILTER_VOLUME(config, "2610.2.l").add_route(ALL_OUTPUTS, "lspeaker", 1.0);
}

void taitoz_state::bshark(machine_config &config)
{
	bshark_base(config);

	adc0809_device &adc(ADC0809(config, "adc", 500000)); // clock unknown
	adc.eoc_ff_callback().set_inputline("maincpu", 6);
	adc.in_callback<0>().set_ioport("STICKX");
	adc.in_callback<1>().set_ioport("X_ADJUST");
	adc.in_callback<2>().set_ioport("STICKY");
	adc.in_callback<3>().set_ioport("Y_ADJUST");
}

void taitoz_state::bsharkjjs(machine_config &config)
{
	bshark_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &taitoz_state::bsharkjjs_map);
}


void sci_state::sci(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(32'000'000)/2);   // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &sci_state::sci_map);
	m_maincpu->set_vblank_int("screen", FUNC(sci_state::sci_interrupt));

	Z80(config, m_audiocpu, XTAL(32'000'000)/8);    // 4 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &sci_state::z80_sound_map);

	M68000(config, m_subcpu, XTAL(32'000'000)/2);   // 16 MHz
	m_subcpu->set_addrmap(AS_PROGRAM, &sci_state::sci_cpub_map);
	m_subcpu->set_vblank_int("screen", FUNC(sci_state::irq4_line_hold));

	config.set_maximum_quantum(attotime::from_hz(3000));

	TC0220IOC(config, m_tc0220ioc, 0);
	m_tc0220ioc->read_0_callback().set_ioport("DSWA");
	m_tc0220ioc->read_1_callback().set_ioport("DSWB");
	m_tc0220ioc->read_2_callback().set_ioport("IN0");
	m_tc0220ioc->read_3_callback().set_ioport("IN1");
	m_tc0220ioc->write_4_callback().set(FUNC(sci_state::coin_control_w));
	m_tc0220ioc->read_7_callback().set_ioport("IN2");

	/* video hardware */
	screen_config(config, 16, 256);
	m_screen->set_screen_update(FUNC(sci_state::screen_update_sci));
	m_screen->set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_taitoz);
	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 4096);

	TC0100SCN(config, m_tc0100scn, 0);
	m_tc0100scn->set_palette("palette");

	TC0150ROD(config, m_tc0150rod, 0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ym2610_device &ymsnd(YM2610(config, "ymsnd", XTAL(32'000'000)/4));    // 8 MHz
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "lspeaker", 0.25);
	ymsnd.add_route(0, "rspeaker", 0.25);
	ymsnd.add_route(1, "2610.1.l", 2.0);
	ymsnd.add_route(1, "2610.1.r", 2.0);
	ymsnd.add_route(2, "2610.2.l", 2.0);
	ymsnd.add_route(2, "2610.2.r", 2.0);

	FILTER_VOLUME(config, "2610.1.r").add_route(ALL_OUTPUTS, "rspeaker", 1.0);
	FILTER_VOLUME(config, "2610.1.l").add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	FILTER_VOLUME(config, "2610.2.r").add_route(ALL_OUTPUTS, "rspeaker", 1.0);
	FILTER_VOLUME(config, "2610.2.l").add_route(ALL_OUTPUTS, "lspeaker", 1.0);

	TC0140SYT(config, m_tc0140syt, 0);
	m_tc0140syt->set_master_tag(m_subcpu);
	m_tc0140syt->set_slave_tag(m_audiocpu);
}

void nightstr_state::nightstr(machine_config &config) //OSC: 26.686, 24.000, 16.000
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000)/2);   // MC68000P12 @ 12 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &nightstr_state::nightstr_map);
	m_maincpu->set_vblank_int("screen", FUNC(nightstr_state::irq4_line_hold));

	Z80(config, m_audiocpu, XTAL(16'000'000)/4);    // Z0840004PSC @ 4 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &nightstr_state::z80_sound_map);

	M68000(config, m_subcpu, XTAL(24'000'000)/2);   // MC68000P12 @ 12 MHz
	m_subcpu->set_addrmap(AS_PROGRAM, &nightstr_state::nightstr_cpub_map);
	m_subcpu->set_vblank_int("screen", FUNC(nightstr_state::irq4_line_hold));

	config.set_maximum_quantum(attotime::from_hz(6000));

	adc0809_device &adc(ADC0809(config, "adc", 500000)); // clock unknown
	adc.eoc_ff_callback().set_inputline("maincpu", 6);
	adc.in_callback<0>().set_ioport("STICKX");
	adc.in_callback<1>().set_ioport("STICKY");
	adc.in_callback<2>().set_ioport("X_ADJUST");
	adc.in_callback<3>().set_ioport("Y_ADJUST");

	TC0220IOC(config, m_tc0220ioc, 0);
	m_tc0220ioc->read_0_callback().set_ioport("DSWA");
	m_tc0220ioc->read_1_callback().set_ioport("DSWB");
	m_tc0220ioc->read_2_callback().set_ioport("IN0");
	m_tc0220ioc->read_3_callback().set_ioport("IN1");
	m_tc0220ioc->write_4_callback().set(FUNC(nightstr_state::coin_control_w));
	m_tc0220ioc->read_7_callback().set_ioport("IN2");

	/* video hardware */
	screen_config(config, 16, 256);
	m_screen->set_screen_update(FUNC(nightstr_state::screen_update_chasehq));
	m_screen->set_palette(m_tc0110pcr);

	GFXDECODE(config, m_gfxdecode, m_tc0110pcr, gfx_chasehq);

	TC0100SCN(config, m_tc0100scn, 0);
	m_tc0100scn->set_palette(m_tc0110pcr);

	TC0150ROD(config, m_tc0150rod, 0);

	TC0110PCR(config, m_tc0110pcr, 0);

	/* sound hardware */
	SPEAKER(config, "front").front_center();
	SPEAKER(config, "rear").rear_center();
	SPEAKER(config, "subwoofer").set_position(0.0, 0.0, 0.0); // FIXME: where is this located in the cabinet?

	ym2610_device &ymsnd(YM2610(config, "ymsnd", XTAL(16'000'000)/2));    // 8 MHz
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "subwoofer", 0.20);
	ymsnd.add_route(1, "2610.1.l", 2.0);
	ymsnd.add_route(1, "2610.1.r", 2.0);
	ymsnd.add_route(2, "2610.2.l", 2.0);
	ymsnd.add_route(2, "2610.2.r", 2.0);

	FILTER_VOLUME(config, "2610.1.r").add_route(ALL_OUTPUTS, "rear", 1.0);
	FILTER_VOLUME(config, "2610.1.l").add_route(ALL_OUTPUTS, "front", 1.0);
	FILTER_VOLUME(config, "2610.2.r").add_route(ALL_OUTPUTS, "rear", 1.0);
	FILTER_VOLUME(config, "2610.2.l").add_route(ALL_OUTPUTS, "front", 1.0);

	TC0140SYT(config, m_tc0140syt, 0);
	m_tc0140syt->set_master_tag(m_subcpu);
	m_tc0140syt->set_slave_tag(m_audiocpu);
}

void taitoz_z80_sound_state::aquajack(machine_config &config) //OSC: 26.686, 24.000, 16.000
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000)/2);   // MC68000P12 @ 12 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &taitoz_z80_sound_state::aquajack_map);
	m_maincpu->set_vblank_int("screen", FUNC(taitoz_z80_sound_state::irq4_line_hold));

	Z80(config, m_audiocpu, XTAL(16'000'000)/4);    // Z0840004PSC @ 4 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &taitoz_z80_sound_state::z80_sound_map);

	M68000(config, m_subcpu, XTAL(24'000'000)/2);   // MC68000P12 @ 12 MHz
	m_subcpu->set_addrmap(AS_PROGRAM, &taitoz_z80_sound_state::aquajack_cpub_map);
	m_subcpu->set_vblank_int("screen", FUNC(taitoz_z80_sound_state::irq4_line_hold));

	config.set_maximum_quantum(attotime::from_hz(30000));

	TC0220IOC(config, m_tc0220ioc, 0);
	m_tc0220ioc->read_0_callback().set_ioport("DSWA");
	m_tc0220ioc->read_1_callback().set_ioport("DSWB");
	m_tc0220ioc->read_2_callback().set_ioport("IN0");
	m_tc0220ioc->read_3_callback().set_ioport("IN1");
	m_tc0220ioc->write_4_callback().set(FUNC(taitoz_z80_sound_state::coin_control_w));
	m_tc0220ioc->read_7_callback().set_ioport("IN2");

	/* video hardware */
	screen_config(config, 16, 256);
	m_screen->set_screen_update(FUNC(taitoz_z80_sound_state::screen_update_aquajack));
	m_screen->set_palette(m_tc0110pcr);

	GFXDECODE(config, m_gfxdecode, m_tc0110pcr, gfx_taitoz);

	TC0100SCN(config, m_tc0100scn, 0);
	m_tc0100scn->set_palette(m_tc0110pcr);

	TC0150ROD(config, m_tc0150rod, 0);

	TC0110PCR(config, m_tc0110pcr, 0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ym2610_device &ymsnd(YM2610(config, "ymsnd", XTAL(16'000'000)/2));    // 8 MHz
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "lspeaker", 0.25);
	ymsnd.add_route(0, "rspeaker", 0.25);
	ymsnd.add_route(1, "2610.1.l", 2.0);
	ymsnd.add_route(1, "2610.1.r", 2.0);
	ymsnd.add_route(2, "2610.2.l", 2.0);
	ymsnd.add_route(2, "2610.2.r", 2.0);

	FILTER_VOLUME(config, "2610.1.r").add_route(ALL_OUTPUTS, "rspeaker", 1.0);
	FILTER_VOLUME(config, "2610.1.l").add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	FILTER_VOLUME(config, "2610.2.r").add_route(ALL_OUTPUTS, "rspeaker", 1.0);
	FILTER_VOLUME(config, "2610.2.l").add_route(ALL_OUTPUTS, "lspeaker", 1.0);

	TC0140SYT(config, m_tc0140syt, 0);
	m_tc0140syt->set_master_tag(m_subcpu);
	m_tc0140syt->set_slave_tag(m_audiocpu);
}

void spacegun_state::spacegun(machine_config &config) //OSC: 26.686, 24.000, 16.000
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000)/2);   // MC68000P12 @ 12 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &spacegun_state::spacegun_map);
	m_maincpu->set_vblank_int("screen", FUNC(spacegun_state::irq4_line_hold));

	M68000(config, m_subcpu, XTAL(24'000'000)/2);   // MC68000P12 @ 12 MHz
	m_subcpu->set_addrmap(AS_PROGRAM, &spacegun_state::spacegun_cpub_map);
	m_subcpu->set_vblank_int("screen", FUNC(spacegun_state::irq4_line_hold));

	EEPROM_93C46_16BIT(config, m_eeprom).default_data(spacegun_default_eeprom, 128);

	adc0809_device &adc(ADC0809(config, "adc", 500000)); // clock unknown
	adc.eoc_ff_callback().set_inputline("sub", 5);
	adc.in_callback<0>().set_ioport("STICKX1");
	adc.in_callback<1>().set_ioport("STICKY1");
	adc.in_callback<2>().set_ioport("STICKX2");
	adc.in_callback<3>().set_ioport("STICKY2");

	TC0510NIO(config, m_tc0510nio, 0);
	m_tc0510nio->read_0_callback().set_ioport("DSWA");
	m_tc0510nio->read_1_callback().set_ioport("DSWB");
	m_tc0510nio->read_2_callback().set_ioport("IN0");
	m_tc0510nio->read_3_callback().set(m_eeprom, FUNC(eeprom_serial_93cxx_device::do_read)).lshift(7);
	m_tc0510nio->write_3_callback().set(FUNC(spacegun_state::spacegun_eeprom_w));
	m_tc0510nio->write_4_callback().set(FUNC(spacegun_state::coin_control_w));
	m_tc0510nio->read_7_callback().set_ioport("IN2");

	/* video hardware */
	screen_config(config, 16, 256);
	m_screen->set_screen_update(FUNC(spacegun_state::screen_update_spacegun));
	m_screen->set_palette(m_tc0110pcr);

	GFXDECODE(config, m_gfxdecode, m_tc0110pcr, gfx_taitoz);

	TC0100SCN(config, m_tc0100scn, 0);
	m_tc0100scn->set_offsets(4, 0);
	m_tc0100scn->set_palette(m_tc0110pcr);

	TC0110PCR(config, m_tc0110pcr, 0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ym2610_device &ymsnd(YM2610(config, "ymsnd", XTAL(16'000'000)/2));    // 8 MHz
	//ymsnd.irq_handler().set_inputline(m_audiocpu, 0); // DG: this is probably specific to Z80 and wrong?
	ymsnd.add_route(0, "lspeaker", 0.25);
	ymsnd.add_route(0, "rspeaker", 0.25);
	ymsnd.add_route(1, "2610.1.l", 8.0);
	ymsnd.add_route(1, "2610.1.r", 8.0);
	ymsnd.add_route(2, "2610.2.l", 8.0);
	ymsnd.add_route(2, "2610.2.r", 8.0);

	FILTER_VOLUME(config, "2610.1.r").add_route(ALL_OUTPUTS, "rspeaker", 1.0);
	FILTER_VOLUME(config, "2610.1.l").add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	FILTER_VOLUME(config, "2610.2.r").add_route(ALL_OUTPUTS, "rspeaker", 1.0);
	FILTER_VOLUME(config, "2610.2.l").add_route(ALL_OUTPUTS, "lspeaker", 1.0);
}

void dblaxle_state::dblaxle(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(32'000'000)/2);   // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &dblaxle_state::dblaxle_map);
	m_maincpu->set_vblank_int("screen", FUNC(dblaxle_state::irq4_line_hold));

	Z80(config, m_audiocpu, XTAL(32'000'000)/8);   // 4 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &dblaxle_state::z80_sound_map);

	M68000(config, m_subcpu, XTAL(32'000'000)/2);   // 16 MHz
	m_subcpu->set_addrmap(AS_PROGRAM, &dblaxle_state::dblaxle_cpub_map);
	m_subcpu->set_vblank_int("screen", FUNC(dblaxle_state::irq4_line_hold));

	// make quantum time to be a multiple of the xtal (fixes road layer stuck on continue)
	config.set_maximum_quantum(attotime::from_hz(XTAL(32'000'000)/1024));

	TC0510NIO(config, m_tc0510nio, 0);
	m_tc0510nio->read_0_callback().set_ioport("DSWA");
	m_tc0510nio->read_1_callback().set_ioport("DSWB");
	m_tc0510nio->read_2_callback().set_ioport("IN0");
	m_tc0510nio->read_3_callback().set_ioport("IN1");
	m_tc0510nio->write_4_callback().set(FUNC(dblaxle_state::coin_control_w));
	m_tc0510nio->read_7_callback().set_ioport("IN2");

	/* video hardware */
	screen_config(config, 16, 256);
	m_screen->set_screen_update(FUNC(dblaxle_state::screen_update_dblaxle));
	m_screen->set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_taitoz);
	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 4096);

	TC0480SCP(config, m_tc0480scp, 0);
	m_tc0480scp->set_palette("palette");
	m_tc0480scp->set_offsets(0x1f, 0x08);

	TC0150ROD(config, m_tc0150rod, 0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ym2610_device &ymsnd(YM2610(config, "ymsnd", XTAL(32'000'000)/4));   // 8 MHz
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "lspeaker", 0.25);
	ymsnd.add_route(0, "rspeaker", 0.25);
	ymsnd.add_route(1, "2610.1.l", 8.0);
	ymsnd.add_route(1, "2610.1.r", 8.0);
	ymsnd.add_route(2, "2610.2.l", 8.0);
	ymsnd.add_route(2, "2610.2.r", 8.0);

	FILTER_VOLUME(config, "2610.1.r").add_route(ALL_OUTPUTS, "rspeaker", 1.0);
	FILTER_VOLUME(config, "2610.1.l").add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	FILTER_VOLUME(config, "2610.2.r").add_route(ALL_OUTPUTS, "rspeaker", 1.0);
	FILTER_VOLUME(config, "2610.2.l").add_route(ALL_OUTPUTS, "lspeaker", 1.0);

	TC0140SYT(config, m_tc0140syt, 0);
	m_tc0140syt->set_master_tag(m_subcpu);
	m_tc0140syt->set_slave_tag(m_audiocpu);
}

void sci_state::racingb(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(32'000'000)/2);   // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &sci_state::racingb_map);
	m_maincpu->set_vblank_int("screen", FUNC(sci_state::sci_interrupt));

	Z80(config, m_audiocpu, XTAL(32'000'000)/8);   // 4 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &sci_state::z80_sound_map);

	M68000(config, m_subcpu, XTAL(32'000'000)/2);   // 16 MHz
	m_subcpu->set_addrmap(AS_PROGRAM, &sci_state::racingb_cpub_map);
	m_subcpu->set_vblank_int("screen", FUNC(sci_state::irq4_line_hold));

	config.set_maximum_quantum(attotime::from_hz(600));

	TC0510NIO(config, m_tc0510nio, 0);
	m_tc0510nio->read_0_callback().set_ioport("DSWA");
	m_tc0510nio->read_1_callback().set_ioport("DSWB");
	m_tc0510nio->read_2_callback().set_ioport("IN0");
	m_tc0510nio->read_3_callback().set_ioport("IN1");
	m_tc0510nio->write_4_callback().set(FUNC(sci_state::coin_control_w));
	m_tc0510nio->read_7_callback().set_ioport("IN2");

	/* video hardware */
	screen_config(config, 16, 256);
	m_screen->set_screen_update(FUNC(sci_state::screen_update_racingb));
	m_screen->set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_taitoz);
	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 4096);

	TC0480SCP(config, m_tc0480scp, 0);
	m_tc0480scp->set_palette("palette");
	m_tc0480scp->set_offsets(0x1f, 0x08);

	TC0150ROD(config, m_tc0150rod, 0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ym2610_device &ymsnd(YM2610(config, "ymsnd", XTAL(32'000'000)/4));   // 8 MHz
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "lspeaker", 0.25);
	ymsnd.add_route(0, "rspeaker", 0.25);
	ymsnd.add_route(1, "2610.1.l", 8.0);
	ymsnd.add_route(1, "2610.1.r", 8.0);
	ymsnd.add_route(2, "2610.2.l", 8.0);
	ymsnd.add_route(2, "2610.2.r", 8.0);

	FILTER_VOLUME(config, "2610.1.r").add_route(ALL_OUTPUTS, "rspeaker", 1.0);
	FILTER_VOLUME(config, "2610.1.l").add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	FILTER_VOLUME(config, "2610.2.r").add_route(ALL_OUTPUTS, "rspeaker", 1.0);
	FILTER_VOLUME(config, "2610.2.l").add_route(ALL_OUTPUTS, "lspeaker", 1.0);

	TC0140SYT(config, m_tc0140syt, 0);
	m_tc0140syt->set_master_tag(m_subcpu);
	m_tc0140syt->set_slave_tag(m_audiocpu);
}


/***************************************************************************
                                 DRIVERS

Contcirc, Dblaxle sound sample rom order is uncertain as sound imperfect
***************************************************************************/

ROM_START( contcirc ) /* 3D Effects controlled via dipswitch, when on can toggle effect with START1 button */
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 256K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "b33-ww.ic25", 0x00000, 0x20000, CRC(f5c92e42) SHA1(42dfa1895e601df76d7022b83f05c4e5c843fd12) ) /* Needs actual Taito ID number here */
	ROM_LOAD16_BYTE( "b33-xx.ic26", 0x00001, 0x20000, CRC(e7c1d1fa) SHA1(75e851629a54facb8804ee8a953ab3265633bbf4) ) /* Needs actual Taito ID number here */

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "b33-yy.ic35", 0x00000, 0x20000, CRC(16522f2d) SHA1(1d2823d61518936d342df3ed712da5bdfdf6e55a) ) /* Needs actual Taito ID number here */
	ROM_LOAD16_BYTE( "cc_36.bin",   0x00001, 0x20000, CRC(a1732ea5) SHA1(b773add433c20633e7acbc99d5cfeb7ccde83371) ) /* Needs actual Taito ID number here */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "b33-30.11", 0x00000, 0x10000, CRC(d8746234) SHA1(39132eedfe2ff4e3133f8020304da0d04dd757db) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b33-02.57", 0x00000, 0x80000, CRC(f6fb3ba2) SHA1(19b7c4cf33c4737405ebe53e7342578454e6ef95) ) /* SCR 8x8 */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "b33-06", 0x000000, 0x080000, CRC(2cb40599) SHA1(48b269610f80a42608f563742e5266dcf11638d1) )   /* OBJ 16x8 */
	ROM_LOAD64_WORD_SWAP( "b33-05", 0x000002, 0x080000, CRC(bddf9eea) SHA1(284f4ba3dc107b4e26424963d8206c5ec4882983) )
	ROM_LOAD64_WORD_SWAP( "b33-04", 0x000004, 0x080000, CRC(8df866a2) SHA1(6b87d8e683fe7d31070b16620ebfee4edf7711b8) )
	ROM_LOAD64_WORD_SWAP( "b33-03", 0x000006, 0x080000, CRC(4f6c36d9) SHA1(18b15a991c3daf22b7f3f144edf3bd2abb3917eb) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "b33-01.3", 0x00000, 0x80000, CRC(f11f2be8) SHA1(72ae08dc5bf5f6901fbb52d3b1dabcba90929b38) )  /* ROD, road lines */

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "b33-07.64", 0x00000, 0x80000, CRC(151e1f52) SHA1(118c673d74f27c4e76b321cc0e84f166d9f0d412) )  /* STY spritemap */

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "b33-09.18", 0x00000, 0x80000, CRC(1e6724b5) SHA1(48bb96b648605a9ceb88ff3b175a87226583c3d6) )
	ROM_LOAD( "b33-10.17", 0x80000, 0x80000, CRC(e9ce03ab) SHA1(17324e8f0422118bc0912eba5750d80469f40b78) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "b33-08.19", 0x00000, 0x80000, CRC(caa1c4c8) SHA1(15ef4f36e56fab793d2249252c456677ca6a85c9) )

	ROM_REGION( 0x10000, "user2", 0 )   /* unused ROMs */
	ROM_LOAD( "b14-30.97",   0x00000, 0x10000, CRC(dccb0c7f) SHA1(42f0af72f559133b74912a4478e1323062be4b77) )   // sprite vertical zoom
	ROM_LOAD( "b14-31.50",   0x00000, 0x02000, CRC(5c6b013d) SHA1(6d02d4560076213b6fb6fe856143bb533090603e) )   // sprite horizontal zoom
	ROM_LOAD( "b33-17.16",   0x00000, 0x00100, CRC(7b7d8ff4) SHA1(18842ed8160739cd2e2ccc2db605153dbed6cc0a) )   // road/sprite priority and palette select
	ROM_LOAD( "b33-18.17",   0x00000, 0x00100, CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) )   // road A/B internal priority
ROM_END

ROM_START( contcircu ) /* 3D Effects controlled via dipswitch, when on can toggle effect with START1 button */
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 256K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "b33-ww.ic25", 0x00000, 0x20000, CRC(f5c92e42) SHA1(42dfa1895e601df76d7022b83f05c4e5c843fd12) ) /* Needs actual Taito ID number here */
	ROM_LOAD16_BYTE( "b33-xx.ic26", 0x00001, 0x20000, CRC(e7c1d1fa) SHA1(75e851629a54facb8804ee8a953ab3265633bbf4) ) /* Needs actual Taito ID number here */

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "b33-yy.ic35", 0x00000, 0x20000, CRC(16522f2d) SHA1(1d2823d61518936d342df3ed712da5bdfdf6e55a) ) /* Needs actual Taito ID number here */
	ROM_LOAD16_BYTE( "b33-zz.ic36", 0x00001, 0x20000, CRC(d6741e33) SHA1(8e86789e1664a34ceed85434fd3186f2571f0c4a) ) /* Needs actual Taito ID number here */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "b33-30.11", 0x00000, 0x10000, CRC(d8746234) SHA1(39132eedfe2ff4e3133f8020304da0d04dd757db) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b33-02.57", 0x00000, 0x80000, CRC(f6fb3ba2) SHA1(19b7c4cf33c4737405ebe53e7342578454e6ef95) ) /* SCR 8x8 */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "b33-06", 0x000000, 0x080000, CRC(2cb40599) SHA1(48b269610f80a42608f563742e5266dcf11638d1) )   /* OBJ 16x8 */
	ROM_LOAD64_WORD_SWAP( "b33-05", 0x000002, 0x080000, CRC(bddf9eea) SHA1(284f4ba3dc107b4e26424963d8206c5ec4882983) )
	ROM_LOAD64_WORD_SWAP( "b33-04", 0x000004, 0x080000, CRC(8df866a2) SHA1(6b87d8e683fe7d31070b16620ebfee4edf7711b8) )
	ROM_LOAD64_WORD_SWAP( "b33-03", 0x000006, 0x080000, CRC(4f6c36d9) SHA1(18b15a991c3daf22b7f3f144edf3bd2abb3917eb) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "b33-01.3", 0x00000, 0x80000, CRC(f11f2be8) SHA1(72ae08dc5bf5f6901fbb52d3b1dabcba90929b38) )  /* ROD, road lines */

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "b33-07.64", 0x00000, 0x80000, CRC(151e1f52) SHA1(118c673d74f27c4e76b321cc0e84f166d9f0d412) )  /* STY spritemap */

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "b33-09.18", 0x00000, 0x80000, CRC(1e6724b5) SHA1(48bb96b648605a9ceb88ff3b175a87226583c3d6) )
	ROM_LOAD( "b33-10.17", 0x80000, 0x80000, CRC(e9ce03ab) SHA1(17324e8f0422118bc0912eba5750d80469f40b78) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "b33-08.19", 0x00000, 0x80000, CRC(caa1c4c8) SHA1(15ef4f36e56fab793d2249252c456677ca6a85c9) )

	ROM_REGION( 0x10000, "user2", 0 )   /* unused ROMs */
	ROM_LOAD( "b14-30.97",   0x00000, 0x10000, CRC(dccb0c7f) SHA1(42f0af72f559133b74912a4478e1323062be4b77) )   // sprite vertical zoom
	ROM_LOAD( "b14-31.50",   0x00000, 0x02000, CRC(5c6b013d) SHA1(6d02d4560076213b6fb6fe856143bb533090603e) )   // sprite horizontal zoom
	ROM_LOAD( "b33-17.16",   0x00000, 0x00100, CRC(7b7d8ff4) SHA1(18842ed8160739cd2e2ccc2db605153dbed6cc0a) )   // road/sprite priority and palette select
	ROM_LOAD( "b33-18.17",   0x00000, 0x00100, CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) )   // road A/B internal priority
ROM_END

ROM_START( contcircua )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 256K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "b33-34.ic25", 0x00000, 0x20000, CRC(e1e016c1) SHA1(d6ca3bcf03828dc296eab73185f773860bbaaae6) ) /* 3D Effects ALWAYS ON */
	ROM_LOAD16_BYTE( "b33-33.ic26", 0x00001, 0x20000, CRC(f539d44b) SHA1(1b77d97376f9bf3bbd728d459f0a0afbadc6d756) )

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "b33-21-2.ic35", 0x00000, 0x20000, CRC(2723f9e3) SHA1(18a86e352bb0aeec6ad6c537294ddd0d33823ea6) )
	ROM_LOAD16_BYTE( "b33-31-1.ic36", 0x00001, 0x20000, CRC(438431f7) SHA1(9be4ac6526d5aee01c3691f189583a2cfdad0e45) ) /* Is this really B33 31-2 ?? */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "b33-30.11", 0x00000, 0x10000, CRC(d8746234) SHA1(39132eedfe2ff4e3133f8020304da0d04dd757db) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b33-02.57", 0x00000, 0x80000, CRC(f6fb3ba2) SHA1(19b7c4cf33c4737405ebe53e7342578454e6ef95) ) /* SCR 8x8 */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "b33-06", 0x000000, 0x080000, CRC(2cb40599) SHA1(48b269610f80a42608f563742e5266dcf11638d1) )   /* OBJ 16x8 */
	ROM_LOAD64_WORD_SWAP( "b33-05", 0x000002, 0x080000, CRC(bddf9eea) SHA1(284f4ba3dc107b4e26424963d8206c5ec4882983) )
	ROM_LOAD64_WORD_SWAP( "b33-04", 0x000004, 0x080000, CRC(8df866a2) SHA1(6b87d8e683fe7d31070b16620ebfee4edf7711b8) )
	ROM_LOAD64_WORD_SWAP( "b33-03", 0x000006, 0x080000, CRC(4f6c36d9) SHA1(18b15a991c3daf22b7f3f144edf3bd2abb3917eb) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "b33-01.3", 0x00000, 0x80000, CRC(f11f2be8) SHA1(72ae08dc5bf5f6901fbb52d3b1dabcba90929b38) )  /* ROD, road lines */

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "b33-07.64", 0x00000, 0x80000, CRC(151e1f52) SHA1(118c673d74f27c4e76b321cc0e84f166d9f0d412) )  /* STY spritemap */

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "b33-09.18", 0x00000, 0x80000, CRC(1e6724b5) SHA1(48bb96b648605a9ceb88ff3b175a87226583c3d6) )
	ROM_LOAD( "b33-10.17", 0x80000, 0x80000, CRC(e9ce03ab) SHA1(17324e8f0422118bc0912eba5750d80469f40b78) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "b33-08.19", 0x00000, 0x80000, CRC(caa1c4c8) SHA1(15ef4f36e56fab793d2249252c456677ca6a85c9) )

	ROM_REGION( 0x10000, "user2", 0 )   /* unused ROMs */
	ROM_LOAD( "b14-30.97",   0x00000, 0x10000, CRC(dccb0c7f) SHA1(42f0af72f559133b74912a4478e1323062be4b77) )   // sprite vertical zoom
	ROM_LOAD( "b14-31.50",   0x00000, 0x02000, CRC(5c6b013d) SHA1(6d02d4560076213b6fb6fe856143bb533090603e) )   // sprite horizontal zoom
	ROM_LOAD( "b33-17.16",   0x00000, 0x00100, CRC(7b7d8ff4) SHA1(18842ed8160739cd2e2ccc2db605153dbed6cc0a) )   // road/sprite priority and palette select
	ROM_LOAD( "b33-18.17",   0x00000, 0x00100, CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) )   // road A/B internal priority
ROM_END

ROM_START( contcircj )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 256K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "b33-19.ic25", 0x00000, 0x20000, CRC(b85360c8) SHA1(a52550c0889b99453b845dcfab2ed9581f9fdbe8) ) /* 3D Effects ALWAYS ON */
	ROM_LOAD16_BYTE( "b33-20.ic26", 0x00001, 0x20000, CRC(9f88378b) SHA1(dc4f3dbeb98031ced0591623a2ba7a2653cb6ff4) )

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "b33-21-2.ic35", 0x00000, 0x20000, CRC(2723f9e3) SHA1(18a86e352bb0aeec6ad6c537294ddd0d33823ea6) )
	ROM_LOAD16_BYTE( "b33-22-2.ic36", 0x00001, 0x20000, CRC(da8d604d) SHA1(31a4b686d12511a2522c7047a39aa09c0778f230) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "b33-30.11", 0x00000, 0x10000, CRC(d8746234) SHA1(39132eedfe2ff4e3133f8020304da0d04dd757db) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b33-02.57", 0x00000, 0x80000, CRC(f6fb3ba2) SHA1(19b7c4cf33c4737405ebe53e7342578454e6ef95) ) /* SCR 8x8 */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "b33-06", 0x000000, 0x080000, CRC(2cb40599) SHA1(48b269610f80a42608f563742e5266dcf11638d1) )   /* OBJ 16x8 */
	ROM_LOAD64_WORD_SWAP( "b33-05", 0x000002, 0x080000, CRC(bddf9eea) SHA1(284f4ba3dc107b4e26424963d8206c5ec4882983) )
	ROM_LOAD64_WORD_SWAP( "b33-04", 0x000004, 0x080000, CRC(8df866a2) SHA1(6b87d8e683fe7d31070b16620ebfee4edf7711b8) )
	ROM_LOAD64_WORD_SWAP( "b33-03", 0x000006, 0x080000, CRC(4f6c36d9) SHA1(18b15a991c3daf22b7f3f144edf3bd2abb3917eb) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "b33-01.3", 0x00000, 0x80000, CRC(f11f2be8) SHA1(72ae08dc5bf5f6901fbb52d3b1dabcba90929b38) )  /* ROD, road lines */

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "b33-07.64", 0x00000, 0x80000, CRC(151e1f52) SHA1(118c673d74f27c4e76b321cc0e84f166d9f0d412) )  /* STY spritemap */

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "b33-09.18", 0x00000, 0x80000, CRC(1e6724b5) SHA1(48bb96b648605a9ceb88ff3b175a87226583c3d6) )
	ROM_LOAD( "b33-10.17", 0x80000, 0x80000, CRC(e9ce03ab) SHA1(17324e8f0422118bc0912eba5750d80469f40b78) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "b33-08.19", 0x00000, 0x80000, CRC(caa1c4c8) SHA1(15ef4f36e56fab793d2249252c456677ca6a85c9) )

	ROM_REGION( 0x10000, "user2", 0 )   /* unused ROMs */
	ROM_LOAD( "b14-30.97",   0x00000, 0x10000, CRC(dccb0c7f) SHA1(42f0af72f559133b74912a4478e1323062be4b77) )   // sprite vertical zoom
	ROM_LOAD( "b14-31.50",   0x00000, 0x02000, CRC(5c6b013d) SHA1(6d02d4560076213b6fb6fe856143bb533090603e) )   // sprite horizontal zoom
	ROM_LOAD( "b33-17.16",   0x00000, 0x00100, CRC(7b7d8ff4) SHA1(18842ed8160739cd2e2ccc2db605153dbed6cc0a) )   // road/sprite priority and palette select
	ROM_LOAD( "b33-18.17",   0x00000, 0x00100, CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) )   // road A/B internal priority
ROM_END

ROM_START( chasehq )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "b52-130.36", 0x00000, 0x20000, CRC(4e7beb46) SHA1(b8890c4a2121aa93cfc3a41ddbb3b840d0804cfa) )
	ROM_LOAD16_BYTE( "b52-136.29", 0x00001, 0x20000, CRC(2f414df0) SHA1(0daad8b1f7512a5af0722983751841b5b18064ac) )
	ROM_LOAD16_BYTE( "b52-131.37", 0x40000, 0x20000, CRC(aa945d83) SHA1(9d8a8186a199cacc0e24cf1ee75d81ab8b056406) )
	ROM_LOAD16_BYTE( "b52-129.30", 0x40001, 0x20000, CRC(0eaebc08) SHA1(1dde3304b251ddeb52f1378ef3845269c3667169) )

	ROM_REGION( 0x20000, "sub", 0 ) /* 128K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "b52-132.39", 0x00000, 0x10000, CRC(a2f54789) SHA1(941a6470e3a5ae35d079657260a8d7d6a9fca122) )
	ROM_LOAD16_BYTE( "b52-133.55", 0x00001, 0x10000, CRC(12232f95) SHA1(2894b95fc1d0a6e5b323bf3e7f1968f02b30a845) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "b52-137.51",   0x00000, 0x10000, CRC(37abb74a) SHA1(1feb1e49102c13a90e02c150472545cd9f6334da) )

	ROM_REGION( 0x8000, "motorcpu", 0 )
	ROM_LOAD( "27c256.ic17",   0x0000, 0x8000, CRC(e52dfee1) SHA1(6e58e18eb2de3c899b950a4307ea21cd23683657) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b52-29.27", 0x00000, 0x80000, CRC(8366d27c) SHA1(d7c5f588b39742927228ce73e5d69bda1e903df6) ) /* SCR 8x8 */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "b52-34.5",  0x000000, 0x080000, CRC(7d8dce36) SHA1(ca082e647d10378144c05a70a8e4fe352d95eeaf) )
	ROM_LOAD64_WORD_SWAP( "b52-35.7",  0x000002, 0x080000, CRC(78eeec0d) SHA1(2e82186ca17c579816865ef21c52aef9e133fbf5) )    /* OBJ A 16x16 */
	ROM_LOAD64_WORD_SWAP( "b52-36.9",  0x000004, 0x080000, CRC(61e89e91) SHA1(f655b3caa37a8835c2eb11f4d72e985636ac5379) )
	ROM_LOAD64_WORD_SWAP( "b52-37.11", 0x000006, 0x080000, CRC(f02e47b9) SHA1(093864bd18bd58dafa57990e999f394ca3124452) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "b52-28.4", 0x00000, 0x80000, CRC(963bc82b) SHA1(e3558aecd1b82ddbf10ab2b71843a3664705f1f1) )  /* ROD, road lines */

	ROM_REGION( 0x200000, "sprites2", 0 )
	ROM_LOAD64_WORD_SWAP( "b52-30.4",  0x000000, 0x080000, CRC(1b8cc647) SHA1(8807fe01b6804507564fc179adf995bf86521fda) )
	ROM_LOAD64_WORD_SWAP( "b52-31.6",  0x000002, 0x080000, CRC(f1998e20) SHA1(b03d4e373e88933391f3533b885817edfca4cfdf) )    /* OBJ B 16x16 */
	ROM_LOAD64_WORD_SWAP( "b52-32.8",  0x000004, 0x080000, CRC(8620780c) SHA1(2545fd8fb03dcddc3da86d5ea06a6dc915acd1a1) )
	ROM_LOAD64_WORD_SWAP( "b52-33.10", 0x000006, 0x080000, CRC(e6f4b8c4) SHA1(8d15c75a16953aa56fb3dc6fd3b691e227bef622) )

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "b52-38.34", 0x00000, 0x80000, CRC(5b5bf7f6) SHA1(71dd5b40b83870d351c9ecaccc4fb98c3a6740ae) )  /* STY spritemap */

	ROM_REGION( 0x180000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "b52-115.71", 0x000000, 0x080000, CRC(4e117e93) SHA1(51d893fa21793335878c76f6d5987d99da60be04) )
	ROM_LOAD( "b52-114.72", 0x080000, 0x080000, CRC(3a73d6b1) SHA1(419f02a875b30913331db207e344d0eaa275297e) )
	ROM_LOAD( "b52-113.73", 0x100000, 0x080000, CRC(2c6a3a05) SHA1(f2f0dfbbbb6930bf53025064ebae9c07a95c6deb) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "b52-116.70", 0x00000, 0x80000, CRC(ad46983c) SHA1(6fcad67456fbd8c967cd4786815f70b57a24a969) )

	ROM_REGION( 0x10000, "user2", 0 )   /* unused ROMs */
	ROM_LOAD( "b52-01.7",    0x00000, 0x00100, CRC(89719d17) SHA1(50181b8172b0fc08b149db18caf10659be9c517f) )   // road/sprite priority and palette select
	ROM_LOAD( "b52-03.135",  0x00000, 0x00400, CRC(a3f8490d) SHA1(349b8c9ba914603f72f800a3fc8e8277d756deb1) )
	ROM_LOAD( "b52-06.24",   0x00000, 0x00100, CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) )   // road A/B internal priority
	ROM_LOAD( "b52-18.93",   0x00000, 0x00100, CRC(60bdaf1a) SHA1(0cb9c6b821de9ccc1f38336608dd7ead46cb8d24) )   // identical to b52-18b
	ROM_LOAD( "b52-18a",     0x00000, 0x00100, CRC(6271be0d) SHA1(84282af98fc0de10e88282f7187cd865133ed6ce) )
	ROM_LOAD( "b52-49.68",   0x00000, 0x02000, CRC(60dd2ed1) SHA1(8673b6b3355975fb91cd1491e0ac7c0f590e3824) )
	ROM_LOAD( "b52-50.66",   0x00000, 0x10000, CRC(c189781c) SHA1(af3904ce51f715970965d110313491dbacf188b8) )
	ROM_LOAD( "b52-51.65",   0x00000, 0x10000, CRC(30cc1f79) SHA1(3b0e3e6e8bce7a7d04a5b0103e2ce4e18e52a68e) )
	ROM_LOAD( "b52-126.136", 0x00000, 0x00400, CRC(fa2f840e) SHA1(dd61ee6833bd43bbf619d36ec46f2bfa00880f40) )
	ROM_LOAD( "b52-127.156", 0x00000, 0x00400, CRC(77682a4f) SHA1(da2b3143f1c8688a22d8ec47bbb73b2f2e578480) )

	ROM_REGION( 0x02a00, "plds", 0 )
	ROM_LOAD( "pal20l8b-b52-17.ic18",   0x0000, 0x0144, CRC(4851316d) SHA1(c58537b4b1ded471b7edcb94518757916a207bc0) )
	ROM_LOAD( "pal20l8b-b52-17.ic16",   0x0200, 0x0144, CRC(4851316d) SHA1(c58537b4b1ded471b7edcb94518757916a207bc0) )
	ROM_LOAD( "pal20l8b-b52-17.ic53",   0x0400, 0x0144, CRC(4851316d) SHA1(c58537b4b1ded471b7edcb94518757916a207bc0) )
	ROM_LOAD( "pal20l8b-b52-17.ic55",   0x0600, 0x0144, CRC(4851316d) SHA1(c58537b4b1ded471b7edcb94518757916a207bc0) )
	ROM_LOAD( "pal16l8b-b52-19.ic33",   0x0800, 0x0104, CRC(3ba292dc) SHA1(ce904e92e9c0d73802e6f53dc747204c194281c5) )
	ROM_LOAD( "pal16l8b-b52-20.ic35",   0x0a00, 0x0104, CRC(bd39ad73) SHA1(03120f3d7d0b9b9105990e226388f9cdd53e5789) )
	ROM_LOAD( "pal16l8b-b52-21.ic51",   0x0c00, 0x0104, CRC(2fe76aa4) SHA1(9ea187cff3e0edb58b019400cfc4bf342d4304c3) )
	ROM_LOAD( "pal20l8b-b52-25.ic123",  0x0e00, 0x0144, CRC(372b632d) SHA1(84631489c9f56907a97d686c35b15284fab09e8d) )
	ROM_LOAD( "pal20l8b-b52-26.ic15",   0x1000, 0x0144, CRC(d94f2bc2) SHA1(5ef92f824549add1a115ec25a10567bd4fe65c41) )
	ROM_LOAD( "pal20l8b-b52-26.ic18",   0x1200, 0x0144, CRC(d94f2bc2) SHA1(5ef92f824549add1a115ec25a10567bd4fe65c41) )
	ROM_LOAD( "pal20l8b-b52-26.ic52",   0x1400, 0x0144, CRC(d94f2bc2) SHA1(5ef92f824549add1a115ec25a10567bd4fe65c41) )
	ROM_LOAD( "pal20l8b-b52-26.ic54",   0x1600, 0x0144, CRC(d94f2bc2) SHA1(5ef92f824549add1a115ec25a10567bd4fe65c41) )
	ROM_LOAD( "pal20l8b-b52-27.ic64",   0x1800, 0x0144, CRC(61c2ab26) SHA1(8540d452a21581739b1ab08708b0c799dd2d2393) )
	ROM_LOAD( "pal20l8b-b52-118.ic20",  0x1a00, 0x0144, CRC(9c5fe4af) SHA1(1fbc6461f067eeac9679ca32c126563951a99c09) )
	ROM_LOAD( "pal20l8b-b52-119.ic21",  0x1c00, 0x0144, CRC(8b8e2106) SHA1(be59baae5cada9901c2ea8891f99731f010364db) )
	ROM_LOAD( "pal16l8b-b52-120.ic56",  0x1e00, 0x0104, CRC(3e7effa0) SHA1(0e8c09613b6a9311261aa8fa8bafab0f27081741) )
	ROM_LOAD( "pal20l8b-b52-121.ic57",  0x2000, 0x0144, CRC(7056fd1d) SHA1(fc032f24b95ff7a8bf5a7badddef51c5447c445d) )
	ROM_LOAD( "pal16l8b-b52-122.ic124", 0x2200, 0x0104, CRC(04c0fb04) SHA1(53dfeb747e213c67a78c1407e43cdad4cec2cb7e) )
	ROM_LOAD( "pal16l8b-b52-123.ic125", 0x2400, 0x0104, CRC(3865d1c8) SHA1(2d77326be4fb047243c3d5c33c442b009bf6fc04) )
	ROM_LOAD( "pal16l8b-b52-124.ic180", 0x2600, 0x0104, CRC(d448a25a) SHA1(9339a7969418af30493f4c14cd9bce47d030d1ad) )   /* is this read protected? - taken from chasehqju */
	ROM_LOAD( "pal16l8b-b52-125.ic112", 0x2800, 0x0104, CRC(7628c557) SHA1(11bf628e091dc02e0c2e17ae726061ac04705a54) )
ROM_END

ROM_START( chasehqj )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "b52-140.36", 0x00000, 0x20000, CRC(c1298a4b) SHA1(41981b72c9ebbea8f8a4aa32e74b9ed46dd71e32) )
	ROM_LOAD16_BYTE( "b52-139.29", 0x00001, 0x20000, CRC(997f732e) SHA1(0f7bd4b3c53e1f14830b3c288f2175e7c125c2cc) )
	ROM_LOAD16_BYTE( "b52-131.37", 0x40000, 0x20000, CRC(aa945d83) SHA1(9d8a8186a199cacc0e24cf1ee75d81ab8b056406) )
	ROM_LOAD16_BYTE( "b52-129.30", 0x40001, 0x20000, CRC(0eaebc08) SHA1(1dde3304b251ddeb52f1378ef3845269c3667169) )

	ROM_REGION( 0x20000, "sub", 0 ) /* 128K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "b52-132.39", 0x00000, 0x10000, CRC(a2f54789) SHA1(941a6470e3a5ae35d079657260a8d7d6a9fca122) )
	ROM_LOAD16_BYTE( "b52-133.55", 0x00001, 0x10000, CRC(12232f95) SHA1(2894b95fc1d0a6e5b323bf3e7f1968f02b30a845) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "b52-134.51",    0x00000, 0x10000, CRC(91faac7f) SHA1(05f00e0909444566877d0ef678bae49f107e1628) )

	ROM_REGION( 0x8000, "motorcpu", 0 )
	ROM_LOAD( "27c256.ic17",   0x0000, 0x8000, CRC(e52dfee1) SHA1(6e58e18eb2de3c899b950a4307ea21cd23683657) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b52-29.27", 0x00000, 0x80000, CRC(8366d27c) SHA1(d7c5f588b39742927228ce73e5d69bda1e903df6) ) /* SCR 8x8*/

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "b52-34.5",  0x000000, 0x080000, CRC(7d8dce36) SHA1(ca082e647d10378144c05a70a8e4fe352d95eeaf) )
	ROM_LOAD64_WORD_SWAP( "b52-35.7",  0x000002, 0x080000, CRC(78eeec0d) SHA1(2e82186ca17c579816865ef21c52aef9e133fbf5) )    /* OBJ A 16x16 */
	ROM_LOAD64_WORD_SWAP( "b52-36.9",  0x000004, 0x080000, CRC(61e89e91) SHA1(f655b3caa37a8835c2eb11f4d72e985636ac5379) )
	ROM_LOAD64_WORD_SWAP( "b52-37.11", 0x000006, 0x080000, CRC(f02e47b9) SHA1(093864bd18bd58dafa57990e999f394ca3124452) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "b52-28.4", 0x00000, 0x80000, CRC(963bc82b) SHA1(e3558aecd1b82ddbf10ab2b71843a3664705f1f1) )  /* ROD, road lines */

	ROM_REGION( 0x200000, "sprites2", 0 )
	ROM_LOAD64_WORD_SWAP( "b52-30.4",  0x000000, 0x080000, CRC(1b8cc647) SHA1(8807fe01b6804507564fc179adf995bf86521fda) )
	ROM_LOAD64_WORD_SWAP( "b52-31.6",  0x000002, 0x080000, CRC(f1998e20) SHA1(b03d4e373e88933391f3533b885817edfca4cfdf) )    /* OBJ B 16x16 */
	ROM_LOAD64_WORD_SWAP( "b52-32.8",  0x000004, 0x080000, CRC(8620780c) SHA1(2545fd8fb03dcddc3da86d5ea06a6dc915acd1a1) )
	ROM_LOAD64_WORD_SWAP( "b52-33.10", 0x000006, 0x080000, CRC(e6f4b8c4) SHA1(8d15c75a16953aa56fb3dc6fd3b691e227bef622) )

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "b52-38.34", 0x00000, 0x80000, CRC(5b5bf7f6) SHA1(71dd5b40b83870d351c9ecaccc4fb98c3a6740ae) )  /* STY spritemap */

	ROM_REGION( 0x180000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "b52-41.71", 0x000000, 0x80000, CRC(8204880c) SHA1(4dfd6454b4a4c04db3593e98648afbfe8d1f59ed) )
	ROM_LOAD( "b52-40.72", 0x080000, 0x80000, CRC(f0551055) SHA1(4498cd058a52d5e87c6d502e844908a5df3abf2a) )
	ROM_LOAD( "b52-39.73", 0x100000, 0x80000, CRC(ac9cbbd3) SHA1(792f41fef37ff35067fd0173d944f90279176649) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "b52-42.70", 0x00000, 0x80000, CRC(6e617df1) SHA1(e3d1678132130c66506f2e1419db2f6b5b062f74) )

	ROM_REGION( 0x10000, "user2", 0 )   /* unused ROMs */
	ROM_LOAD( "b52-01.7",    0x00000, 0x00100, CRC(89719d17) SHA1(50181b8172b0fc08b149db18caf10659be9c517f) )   // road/sprite priority and palette select
	ROM_LOAD( "b52-03.135",  0x00000, 0x00400, CRC(a3f8490d) SHA1(349b8c9ba914603f72f800a3fc8e8277d756deb1) )
	ROM_LOAD( "b52-06.24",   0x00000, 0x00100, CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) )   // road A/B internal priority
	ROM_LOAD( "b52-18.93",   0x00000, 0x00100, CRC(60bdaf1a) SHA1(0cb9c6b821de9ccc1f38336608dd7ead46cb8d24) )   // identical to b52-18b
	ROM_LOAD( "b52-18a",     0x00000, 0x00100, CRC(6271be0d) SHA1(84282af98fc0de10e88282f7187cd865133ed6ce) )
	ROM_LOAD( "b52-49.68",   0x00000, 0x02000, CRC(60dd2ed1) SHA1(8673b6b3355975fb91cd1491e0ac7c0f590e3824) )
	ROM_LOAD( "b52-50.66",   0x00000, 0x10000, CRC(c189781c) SHA1(af3904ce51f715970965d110313491dbacf188b8) )
	ROM_LOAD( "b52-51.65",   0x00000, 0x10000, CRC(30cc1f79) SHA1(3b0e3e6e8bce7a7d04a5b0103e2ce4e18e52a68e) )
	ROM_LOAD( "b52-126.136", 0x00000, 0x00400, CRC(fa2f840e) SHA1(dd61ee6833bd43bbf619d36ec46f2bfa00880f40) )
	ROM_LOAD( "b52-127.156", 0x00000, 0x00400, CRC(77682a4f) SHA1(da2b3143f1c8688a22d8ec47bbb73b2f2e578480) )

	ROM_REGION( 0x02a00, "plds", 0 )
	ROM_LOAD( "pal20l8b-b52-17.ic18",   0x0000, 0x0144, CRC(4851316d) SHA1(c58537b4b1ded471b7edcb94518757916a207bc0) )
	ROM_LOAD( "pal20l8b-b52-17.ic16",   0x0200, 0x0144, CRC(4851316d) SHA1(c58537b4b1ded471b7edcb94518757916a207bc0) )
	ROM_LOAD( "pal20l8b-b52-17.ic53",   0x0400, 0x0144, CRC(4851316d) SHA1(c58537b4b1ded471b7edcb94518757916a207bc0) )
	ROM_LOAD( "pal20l8b-b52-17.ic55",   0x0600, 0x0144, CRC(4851316d) SHA1(c58537b4b1ded471b7edcb94518757916a207bc0) )
	ROM_LOAD( "pal16l8b-b52-19.ic33",   0x0800, 0x0104, CRC(3ba292dc) SHA1(ce904e92e9c0d73802e6f53dc747204c194281c5) )
	ROM_LOAD( "pal16l8b-b52-20.ic35",   0x0a00, 0x0104, CRC(bd39ad73) SHA1(03120f3d7d0b9b9105990e226388f9cdd53e5789) )
	ROM_LOAD( "pal16l8b-b52-21.ic51",   0x0c00, 0x0104, CRC(2fe76aa4) SHA1(9ea187cff3e0edb58b019400cfc4bf342d4304c3) )
	ROM_LOAD( "pal20l8b-b52-25.ic123",  0x0e00, 0x0144, CRC(372b632d) SHA1(84631489c9f56907a97d686c35b15284fab09e8d) )
	ROM_LOAD( "pal20l8b-b52-26.ic15",   0x1000, 0x0144, CRC(d94f2bc2) SHA1(5ef92f824549add1a115ec25a10567bd4fe65c41) )
	ROM_LOAD( "pal20l8b-b52-26.ic18",   0x1200, 0x0144, CRC(d94f2bc2) SHA1(5ef92f824549add1a115ec25a10567bd4fe65c41) )
	ROM_LOAD( "pal20l8b-b52-26.ic52",   0x1400, 0x0144, CRC(d94f2bc2) SHA1(5ef92f824549add1a115ec25a10567bd4fe65c41) )
	ROM_LOAD( "pal20l8b-b52-26.ic54",   0x1600, 0x0144, CRC(d94f2bc2) SHA1(5ef92f824549add1a115ec25a10567bd4fe65c41) )
	ROM_LOAD( "pal20l8b-b52-27.ic64",   0x1800, 0x0144, CRC(61c2ab26) SHA1(8540d452a21581739b1ab08708b0c799dd2d2393) )
	ROM_LOAD( "pal20l8b-b52-118.ic20",  0x1a00, 0x0144, CRC(9c5fe4af) SHA1(1fbc6461f067eeac9679ca32c126563951a99c09) )
	ROM_LOAD( "pal20l8b-b52-119.ic21",  0x1c00, 0x0144, CRC(8b8e2106) SHA1(be59baae5cada9901c2ea8891f99731f010364db) )
	ROM_LOAD( "pal16l8b-b52-120.ic56",  0x1e00, 0x0104, CRC(3e7effa0) SHA1(0e8c09613b6a9311261aa8fa8bafab0f27081741) )
	ROM_LOAD( "pal20l8b-b52-121.ic57",  0x2000, 0x0144, CRC(7056fd1d) SHA1(fc032f24b95ff7a8bf5a7badddef51c5447c445d) )
	ROM_LOAD( "pal16l8b-b52-122.ic124", 0x2200, 0x0104, CRC(04c0fb04) SHA1(53dfeb747e213c67a78c1407e43cdad4cec2cb7e) )
	ROM_LOAD( "pal16l8b-b52-123.ic125", 0x2400, 0x0104, CRC(3865d1c8) SHA1(2d77326be4fb047243c3d5c33c442b009bf6fc04) )
	ROM_LOAD( "pal16l8b-b52-124.ic180", 0x2600, 0x0104, CRC(d448a25a) SHA1(9339a7969418af30493f4c14cd9bce47d030d1ad) )   /* is this read protected? - taken from chasehqju */
	ROM_LOAD( "pal16l8b-b52-125.ic112", 0x2800, 0x0104, CRC(7628c557) SHA1(11bf628e091dc02e0c2e17ae726061ac04705a54) )
ROM_END

ROM_START( chasehqju )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "b52-130.36", 0x00000, 0x20000, CRC(4e7beb46) SHA1(b8890c4a2121aa93cfc3a41ddbb3b840d0804cfa) ) // ==  b52-130.36            chasehq    Chase H.Q. (World)
	ROM_LOAD16_BYTE( "b52-128.29", 0x00001, 0x20000, CRC(c14f2cdc) SHA1(7136da9211d02534109fc1aa678b77b950c07942) )
	ROM_LOAD16_BYTE( "b52-131.37", 0x40000, 0x20000, CRC(aa945d83) SHA1(9d8a8186a199cacc0e24cf1ee75d81ab8b056406) )
	ROM_LOAD16_BYTE( "b52-129.30", 0x40001, 0x20000, CRC(0eaebc08) SHA1(1dde3304b251ddeb52f1378ef3845269c3667169) )

	ROM_REGION( 0x20000, "sub", 0 ) /* 128K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "b52-132.39", 0x00000, 0x10000, CRC(a2f54789) SHA1(941a6470e3a5ae35d079657260a8d7d6a9fca122) )
	ROM_LOAD16_BYTE( "b52-133.55", 0x00001, 0x10000, CRC(12232f95) SHA1(2894b95fc1d0a6e5b323bf3e7f1968f02b30a845) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "b52-134.51",    0x00000, 0x10000, CRC(91faac7f) SHA1(05f00e0909444566877d0ef678bae49f107e1628) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b52-29.27", 0x00000, 0x80000, CRC(8366d27c) SHA1(d7c5f588b39742927228ce73e5d69bda1e903df6) ) /* SCR 8x8*/

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "b52-34.5",  0x000000, 0x080000, CRC(7d8dce36) SHA1(ca082e647d10378144c05a70a8e4fe352d95eeaf) )
	ROM_LOAD64_WORD_SWAP( "b52-35.7",  0x000002, 0x080000, CRC(78eeec0d) SHA1(2e82186ca17c579816865ef21c52aef9e133fbf5) )    /* OBJ A 16x16 */
	ROM_LOAD64_WORD_SWAP( "b52-36.9",  0x000004, 0x080000, CRC(61e89e91) SHA1(f655b3caa37a8835c2eb11f4d72e985636ac5379) )
	ROM_LOAD64_WORD_SWAP( "b52-37.11", 0x000006, 0x080000, CRC(f02e47b9) SHA1(093864bd18bd58dafa57990e999f394ca3124452) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "b52-28.4", 0x00000, 0x80000, CRC(963bc82b) SHA1(e3558aecd1b82ddbf10ab2b71843a3664705f1f1) )  /* ROD, road lines */

	ROM_REGION( 0x200000, "sprites2", 0 )
	ROM_LOAD64_WORD_SWAP( "b52-30.4",  0x000000, 0x080000, CRC(1b8cc647) SHA1(8807fe01b6804507564fc179adf995bf86521fda) )
	ROM_LOAD64_WORD_SWAP( "b52-31.6",  0x000002, 0x080000, CRC(f1998e20) SHA1(b03d4e373e88933391f3533b885817edfca4cfdf) )    /* OBJ B 16x16 */
	ROM_LOAD64_WORD_SWAP( "b52-32.8",  0x000004, 0x080000, CRC(8620780c) SHA1(2545fd8fb03dcddc3da86d5ea06a6dc915acd1a1) )
	ROM_LOAD64_WORD_SWAP( "b52-33.10", 0x000006, 0x080000, CRC(e6f4b8c4) SHA1(8d15c75a16953aa56fb3dc6fd3b691e227bef622) )

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "b52-38.34", 0x00000, 0x80000, CRC(5b5bf7f6) SHA1(71dd5b40b83870d351c9ecaccc4fb98c3a6740ae) )  /* STY spritemap */

	ROM_REGION( 0x180000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "b52-41.71", 0x000000, 0x80000, CRC(8204880c) SHA1(4dfd6454b4a4c04db3593e98648afbfe8d1f59ed) )
	ROM_LOAD( "b52-40.72", 0x080000, 0x80000, CRC(f0551055) SHA1(4498cd058a52d5e87c6d502e844908a5df3abf2a) )
	ROM_LOAD( "b52-39.73", 0x100000, 0x80000, CRC(ac9cbbd3) SHA1(792f41fef37ff35067fd0173d944f90279176649) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "b52-42.70", 0x00000, 0x80000, CRC(6e617df1) SHA1(e3d1678132130c66506f2e1419db2f6b5b062f74) )

	ROM_REGION( 0x10000, "user2", 0 )   /* unused ROMs */
	ROM_LOAD( "b52-01.7",    0x00000, 0x00100, CRC(89719d17) SHA1(50181b8172b0fc08b149db18caf10659be9c517f) )   // road/sprite priority and palette select
	ROM_LOAD( "b52-03.135",  0x00000, 0x00400, CRC(a3f8490d) SHA1(349b8c9ba914603f72f800a3fc8e8277d756deb1) )
	ROM_LOAD( "b52-06.24",   0x00000, 0x00100, CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) )   // road A/B internal priority
	ROM_LOAD( "b52-18.93",   0x00000, 0x00100, CRC(60bdaf1a) SHA1(0cb9c6b821de9ccc1f38336608dd7ead46cb8d24) )   // identical to b52-18b
	ROM_LOAD( "b52-18a",     0x00000, 0x00100, CRC(6271be0d) SHA1(84282af98fc0de10e88282f7187cd865133ed6ce) )
	ROM_LOAD( "b52-49.68",   0x00000, 0x02000, CRC(60dd2ed1) SHA1(8673b6b3355975fb91cd1491e0ac7c0f590e3824) )
	ROM_LOAD( "b52-50.66",   0x00000, 0x10000, CRC(c189781c) SHA1(af3904ce51f715970965d110313491dbacf188b8) )
	ROM_LOAD( "b52-51.65",   0x00000, 0x10000, CRC(30cc1f79) SHA1(3b0e3e6e8bce7a7d04a5b0103e2ce4e18e52a68e) )
	ROM_LOAD( "b52-126.136", 0x00000, 0x00400, CRC(fa2f840e) SHA1(dd61ee6833bd43bbf619d36ec46f2bfa00880f40) )
	ROM_LOAD( "b52-127.156", 0x00000, 0x00400, CRC(77682a4f) SHA1(da2b3143f1c8688a22d8ec47bbb73b2f2e578480) )

	ROM_REGION( 0x02a00, "plds", 0 )
	ROM_LOAD( "pal20l8b-b52-17.ic18",   0x0000, 0x0144, CRC(4851316d) SHA1(c58537b4b1ded471b7edcb94518757916a207bc0) )
	ROM_LOAD( "pal20l8b-b52-17.ic16",   0x0200, 0x0144, CRC(4851316d) SHA1(c58537b4b1ded471b7edcb94518757916a207bc0) )
	ROM_LOAD( "pal20l8b-b52-17.ic53",   0x0400, 0x0144, CRC(4851316d) SHA1(c58537b4b1ded471b7edcb94518757916a207bc0) )
	ROM_LOAD( "pal20l8b-b52-17.ic55",   0x0600, 0x0144, CRC(4851316d) SHA1(c58537b4b1ded471b7edcb94518757916a207bc0) )
	ROM_LOAD( "pal16l8b-b52-19.ic33",   0x0800, 0x0104, CRC(3ba292dc) SHA1(ce904e92e9c0d73802e6f53dc747204c194281c5) )
	ROM_LOAD( "pal16l8b-b52-20.ic35",   0x0a00, 0x0104, CRC(bd39ad73) SHA1(03120f3d7d0b9b9105990e226388f9cdd53e5789) )
	ROM_LOAD( "pal16l8b-b52-21.ic51",   0x0c00, 0x0104, CRC(2fe76aa4) SHA1(9ea187cff3e0edb58b019400cfc4bf342d4304c3) )
	ROM_LOAD( "pal20l8b-b52-25.ic123",  0x0e00, 0x0144, CRC(372b632d) SHA1(84631489c9f56907a97d686c35b15284fab09e8d) )
	ROM_LOAD( "pal20l8b-b52-26.ic15",   0x1000, 0x0144, CRC(d94f2bc2) SHA1(5ef92f824549add1a115ec25a10567bd4fe65c41) )
	ROM_LOAD( "pal20l8b-b52-26.ic18",   0x1200, 0x0144, CRC(d94f2bc2) SHA1(5ef92f824549add1a115ec25a10567bd4fe65c41) )
	ROM_LOAD( "pal20l8b-b52-26.ic52",   0x1400, 0x0144, CRC(d94f2bc2) SHA1(5ef92f824549add1a115ec25a10567bd4fe65c41) )
	ROM_LOAD( "pal20l8b-b52-26.ic54",   0x1600, 0x0144, CRC(d94f2bc2) SHA1(5ef92f824549add1a115ec25a10567bd4fe65c41) )
	ROM_LOAD( "pal20l8b-b52-27.ic64",   0x1800, 0x0144, CRC(61c2ab26) SHA1(8540d452a21581739b1ab08708b0c799dd2d2393) )
	ROM_LOAD( "pal20l8b-b52-118.ic20",  0x1a00, 0x0144, CRC(9c5fe4af) SHA1(1fbc6461f067eeac9679ca32c126563951a99c09) )
	ROM_LOAD( "pal20l8b-b52-119.ic21",  0x1c00, 0x0144, CRC(8b8e2106) SHA1(be59baae5cada9901c2ea8891f99731f010364db) )
	ROM_LOAD( "pal16l8b-b52-120.ic56",  0x1e00, 0x0104, CRC(3e7effa0) SHA1(0e8c09613b6a9311261aa8fa8bafab0f27081741) )
	ROM_LOAD( "pal20l8b-b52-121.ic57",  0x2000, 0x0144, CRC(7056fd1d) SHA1(fc032f24b95ff7a8bf5a7badddef51c5447c445d) )
	ROM_LOAD( "pal16l8b-b52-122.ic124", 0x2200, 0x0104, CRC(04c0fb04) SHA1(53dfeb747e213c67a78c1407e43cdad4cec2cb7e) )
	ROM_LOAD( "pal16l8b-b52-123.ic125", 0x2400, 0x0104, CRC(3865d1c8) SHA1(2d77326be4fb047243c3d5c33c442b009bf6fc04) )
	ROM_LOAD( "pal16l8b-b52-124.ic180", 0x2600, 0x0104, CRC(d448a25a) SHA1(9339a7969418af30493f4c14cd9bce47d030d1ad) )   /* is this read protected? */
	ROM_LOAD( "pal16l8b-b52-125.ic112", 0x2800, 0x0104, CRC(7628c557) SHA1(11bf628e091dc02e0c2e17ae726061ac04705a54) )
ROM_END

ROM_START( chasehqu )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "b52-138.36", 0x00000, 0x20000, CRC(8b71fe51) SHA1(6f2352aa2112dd18d328acddacf412b54c896ec0) )
	ROM_LOAD16_BYTE( "b52-135.29", 0x00001, 0x20000, CRC(5ba56a7c) SHA1(3af6f1008181d5a5951fbd6a48dd7592a9e38f96) )
	ROM_LOAD16_BYTE( "b52-131.37", 0x40000, 0x20000, CRC(aa945d83) SHA1(9d8a8186a199cacc0e24cf1ee75d81ab8b056406) )
	ROM_LOAD16_BYTE( "b52-129.30", 0x40001, 0x20000, CRC(0eaebc08) SHA1(1dde3304b251ddeb52f1378ef3845269c3667169) )

	ROM_REGION( 0x20000, "sub", 0 ) /* 128K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "b52-132.39", 0x00000, 0x10000, CRC(a2f54789) SHA1(941a6470e3a5ae35d079657260a8d7d6a9fca122) )
	ROM_LOAD16_BYTE( "b52-133.55", 0x00001, 0x10000, CRC(12232f95) SHA1(2894b95fc1d0a6e5b323bf3e7f1968f02b30a845) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "b52-137.51",   0x00000, 0x10000, CRC(37abb74a) SHA1(1feb1e49102c13a90e02c150472545cd9f6334da) )

	ROM_REGION( 0x8000, "motorcpu", 0 )
	ROM_LOAD( "27c256.ic17",   0x0000, 0x8000, CRC(e52dfee1) SHA1(6e58e18eb2de3c899b950a4307ea21cd23683657) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b52-29.27", 0x00000, 0x80000, CRC(8366d27c) SHA1(d7c5f588b39742927228ce73e5d69bda1e903df6) ) /* SCR 8x8*/

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "b52-34.5",  0x000000, 0x080000, CRC(7d8dce36) SHA1(ca082e647d10378144c05a70a8e4fe352d95eeaf) )
	ROM_LOAD64_WORD_SWAP( "b52-35.7",  0x000002, 0x080000, CRC(78eeec0d) SHA1(2e82186ca17c579816865ef21c52aef9e133fbf5) )    /* OBJ A 16x16 */
	ROM_LOAD64_WORD_SWAP( "b52-36.9",  0x000004, 0x080000, CRC(61e89e91) SHA1(f655b3caa37a8835c2eb11f4d72e985636ac5379) )
	ROM_LOAD64_WORD_SWAP( "b52-37.11", 0x000006, 0x080000, CRC(f02e47b9) SHA1(093864bd18bd58dafa57990e999f394ca3124452) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "b52-28.4", 0x00000, 0x80000, CRC(963bc82b) SHA1(e3558aecd1b82ddbf10ab2b71843a3664705f1f1) )  /* ROD, road lines */

	ROM_REGION( 0x200000, "sprites2", 0 )
	ROM_LOAD64_WORD_SWAP( "b52-30.4",  0x000000, 0x080000, CRC(1b8cc647) SHA1(8807fe01b6804507564fc179adf995bf86521fda) )
	ROM_LOAD64_WORD_SWAP( "b52-31.6",  0x000002, 0x080000, CRC(f1998e20) SHA1(b03d4e373e88933391f3533b885817edfca4cfdf) )    /* OBJ B 16x16 */
	ROM_LOAD64_WORD_SWAP( "b52-32.8",  0x000004, 0x080000, CRC(8620780c) SHA1(2545fd8fb03dcddc3da86d5ea06a6dc915acd1a1) )
	ROM_LOAD64_WORD_SWAP( "b52-33.10", 0x000006, 0x080000, CRC(e6f4b8c4) SHA1(8d15c75a16953aa56fb3dc6fd3b691e227bef622) )

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "b52-38.34", 0x00000, 0x80000, CRC(5b5bf7f6) SHA1(71dd5b40b83870d351c9ecaccc4fb98c3a6740ae) )  /* STY spritemap */

	ROM_REGION( 0x180000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "b52-115.71", 0x000000, 0x080000, CRC(4e117e93) SHA1(51d893fa21793335878c76f6d5987d99da60be04) )
	ROM_LOAD( "b52-114.72", 0x080000, 0x080000, CRC(3a73d6b1) SHA1(419f02a875b30913331db207e344d0eaa275297e) )
	ROM_LOAD( "b52-113.73", 0x100000, 0x080000, CRC(2c6a3a05) SHA1(f2f0dfbbbb6930bf53025064ebae9c07a95c6deb) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "b52-116.70", 0x00000, 0x80000, CRC(ad46983c) SHA1(6fcad67456fbd8c967cd4786815f70b57a24a969) )

	ROM_REGION( 0x10000, "user2", 0 )   /* unused ROMs */
	ROM_LOAD( "b52-01.7",    0x00000, 0x00100, CRC(89719d17) SHA1(50181b8172b0fc08b149db18caf10659be9c517f) )   // road/sprite priority and palette select
	ROM_LOAD( "b52-03.135",  0x00000, 0x00400, CRC(a3f8490d) SHA1(349b8c9ba914603f72f800a3fc8e8277d756deb1) )
	ROM_LOAD( "b52-06.24",   0x00000, 0x00100, CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) )   // road A/B internal priority
	ROM_LOAD( "b52-18.93",   0x00000, 0x00100, CRC(60bdaf1a) SHA1(0cb9c6b821de9ccc1f38336608dd7ead46cb8d24) )   // identical to b52-18b
	ROM_LOAD( "b52-18a",     0x00000, 0x00100, CRC(6271be0d) SHA1(84282af98fc0de10e88282f7187cd865133ed6ce) )
	ROM_LOAD( "b52-49.68",   0x00000, 0x02000, CRC(60dd2ed1) SHA1(8673b6b3355975fb91cd1491e0ac7c0f590e3824) )
	ROM_LOAD( "b52-50.66",   0x00000, 0x10000, CRC(c189781c) SHA1(af3904ce51f715970965d110313491dbacf188b8) )
	ROM_LOAD( "b52-51.65",   0x00000, 0x10000, CRC(30cc1f79) SHA1(3b0e3e6e8bce7a7d04a5b0103e2ce4e18e52a68e) )
	ROM_LOAD( "b52-126.136", 0x00000, 0x00400, CRC(fa2f840e) SHA1(dd61ee6833bd43bbf619d36ec46f2bfa00880f40) )
	ROM_LOAD( "b52-127.156", 0x00000, 0x00400, CRC(77682a4f) SHA1(da2b3143f1c8688a22d8ec47bbb73b2f2e578480) )

	ROM_REGION( 0x02a00, "plds", 0 )
	ROM_LOAD( "pal20l8b-b52-17.ic18",   0x0000, 0x0144, CRC(4851316d) SHA1(c58537b4b1ded471b7edcb94518757916a207bc0) )
	ROM_LOAD( "pal20l8b-b52-17.ic16",   0x0200, 0x0144, CRC(4851316d) SHA1(c58537b4b1ded471b7edcb94518757916a207bc0) )
	ROM_LOAD( "pal20l8b-b52-17.ic53",   0x0400, 0x0144, CRC(4851316d) SHA1(c58537b4b1ded471b7edcb94518757916a207bc0) )
	ROM_LOAD( "pal20l8b-b52-17.ic55",   0x0600, 0x0144, CRC(4851316d) SHA1(c58537b4b1ded471b7edcb94518757916a207bc0) )
	ROM_LOAD( "pal16l8b-b52-19.ic33",   0x0800, 0x0104, CRC(3ba292dc) SHA1(ce904e92e9c0d73802e6f53dc747204c194281c5) )
	ROM_LOAD( "pal16l8b-b52-20.ic35",   0x0a00, 0x0104, CRC(bd39ad73) SHA1(03120f3d7d0b9b9105990e226388f9cdd53e5789) )
	ROM_LOAD( "pal16l8b-b52-21.ic51",   0x0c00, 0x0104, CRC(2fe76aa4) SHA1(9ea187cff3e0edb58b019400cfc4bf342d4304c3) )
	ROM_LOAD( "pal20l8b-b52-25.ic123",  0x0e00, 0x0144, CRC(372b632d) SHA1(84631489c9f56907a97d686c35b15284fab09e8d) )
	ROM_LOAD( "pal20l8b-b52-26.ic15",   0x1000, 0x0144, CRC(d94f2bc2) SHA1(5ef92f824549add1a115ec25a10567bd4fe65c41) )
	ROM_LOAD( "pal20l8b-b52-26.ic18",   0x1200, 0x0144, CRC(d94f2bc2) SHA1(5ef92f824549add1a115ec25a10567bd4fe65c41) )
	ROM_LOAD( "pal20l8b-b52-26.ic52",   0x1400, 0x0144, CRC(d94f2bc2) SHA1(5ef92f824549add1a115ec25a10567bd4fe65c41) )
	ROM_LOAD( "pal20l8b-b52-26.ic54",   0x1600, 0x0144, CRC(d94f2bc2) SHA1(5ef92f824549add1a115ec25a10567bd4fe65c41) )
	ROM_LOAD( "pal20l8b-b52-27.ic64",   0x1800, 0x0144, CRC(61c2ab26) SHA1(8540d452a21581739b1ab08708b0c799dd2d2393) )
	ROM_LOAD( "pal20l8b-b52-118.ic20",  0x1a00, 0x0144, CRC(9c5fe4af) SHA1(1fbc6461f067eeac9679ca32c126563951a99c09) )
	ROM_LOAD( "pal20l8b-b52-119.ic21",  0x1c00, 0x0144, CRC(8b8e2106) SHA1(be59baae5cada9901c2ea8891f99731f010364db) )
	ROM_LOAD( "pal16l8b-b52-120.ic56",  0x1e00, 0x0104, CRC(3e7effa0) SHA1(0e8c09613b6a9311261aa8fa8bafab0f27081741) )
	ROM_LOAD( "pal20l8b-b52-121.ic57",  0x2000, 0x0144, CRC(7056fd1d) SHA1(fc032f24b95ff7a8bf5a7badddef51c5447c445d) )
	ROM_LOAD( "pal16l8b-b52-122.ic124", 0x2200, 0x0104, CRC(04c0fb04) SHA1(53dfeb747e213c67a78c1407e43cdad4cec2cb7e) )
	ROM_LOAD( "pal16l8b-b52-123.ic125", 0x2400, 0x0104, CRC(3865d1c8) SHA1(2d77326be4fb047243c3d5c33c442b009bf6fc04) )
	ROM_LOAD( "pal16l8b-b52-124.ic180", 0x2600, 0x0104, CRC(d448a25a) SHA1(9339a7969418af30493f4c14cd9bce47d030d1ad) )   /* is this read protected? - taken from chasehqju */
	ROM_LOAD( "pal16l8b-b52-125.ic112", 0x2800, 0x0104, CRC(7628c557) SHA1(11bf628e091dc02e0c2e17ae726061ac04705a54) )
ROM_END

ROM_START( enforce )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 256K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "b58-38.27", 0x00000, 0x20000, CRC(a1aa0191) SHA1(193d936e1bfe0da4ac984aba65d3e4e6c93a4c11) )
	ROM_LOAD16_BYTE( "b58-36.19", 0x00001, 0x20000, CRC(40f43da3) SHA1(bb3d6c6db8df77674bb76c16992d05c297d97c9f) )

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "b58-37.26", 0x00000, 0x20000, CRC(e823c85c) SHA1(199b19e81c76eb936f4cf31957ae08bed1395bda) )
	ROM_LOAD16_BYTE( "b58-35.18", 0x00001, 0x20000, CRC(8b3ceb12) SHA1(c3f7d1ae5082715f202435c13e6d6f7ac4048750) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "b58-32.41",   0x00000, 0x10000, CRC(f3fd8eca) SHA1(3b1ab64984ea43805b6494f8add26210ed1175c5) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b58-09.13", 0x00000, 0x80000, CRC(9ffd5b31) SHA1(0214fb32012a48560ca9c6ed5ee969d3c41cf95c) ) /* SCR 8x8 */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "b58-04.7",  0x000000, 0x080000, CRC(9482f08d) SHA1(3fc74b9bebca1d82b300ba72c7297c3bcd69cfa9) )
	ROM_LOAD64_WORD_SWAP( "b58-03.6",  0x000002, 0x080000, CRC(158bc440) SHA1(ceab296146363a2e9a48f62118fba6123b4b5a1b) )    /* OBJ 16x8 */
	ROM_LOAD64_WORD_SWAP( "b58-02.2",  0x000004, 0x080000, CRC(6a6e307c) SHA1(fc4a68220e0dd0e64d75ba7c7af0c1ac97dc7fd9) )
	ROM_LOAD64_WORD_SWAP( "b58-01.1",  0x000006, 0x080000, CRC(01e9f0a8) SHA1(0d3a4dc81702e3c57c790eb8a45caca36cb47d4c) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "b58-06.116", 0x00000, 0x80000, CRC(b3495d70) SHA1(ead4c2fd20b8f103a849201c7344cded013eb8bb) )    /* ROD, road lines */

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "b58-05.71", 0x00000, 0x80000, CRC(d1f4991b) SHA1(f1c5a9b8dce994d013290e98fda7bedf73e95900) )  /* STY spritemap */

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "b58-07.11", 0x000000, 0x080000, CRC(eeb5ba08) SHA1(fe40333e09339c76e503ce87b42a89b48d487016) )
	ROM_LOAD( "b58-08.12", 0x080000, 0x080000, CRC(049243cf) SHA1(1f3099b6d764114dc4161ed308369d0f3148dc4e) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples ??? */
	ROM_LOAD( "b58-10.14", 0x00000, 0x80000, CRC(edce0cc1) SHA1(1f6cbc60502b8b12b349e48446ce3a4a1f76bccd) ) /* ??? */

	ROM_REGION( 0x10000, "user2", 0 )   /* unused ROMs */
	ROM_LOAD( "b58-26.104", 0x00000, 0x10000, CRC(dccb0c7f) SHA1(42f0af72f559133b74912a4478e1323062be4b77) )    // sprite vertical zoom
	ROM_LOAD( "b58-27.56",  0x00000, 0x02000, CRC(5c6b013d) SHA1(6d02d4560076213b6fb6fe856143bb533090603e) )    // sprite horizontal zoom
	ROM_LOAD( "b58-23.52",  0x00000, 0x00100, CRC(7b7d8ff4) SHA1(18842ed8160739cd2e2ccc2db605153dbed6cc0a) )    // road/sprite priority and palette select
	ROM_LOAD( "b58-24.51",  0x00000, 0x00100, CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) )    // road A/B internal priority
	ROM_LOAD( "b58-25.75",  0x00000, 0x00100, CRC(de547342) SHA1(3b2b116d4016ddbf46c41c625c7fcfd76129baa7) )
// Add pals...
ROM_END

ROM_START( enforcej )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 256K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "b58-17.27", 0x00000, 0x20000, CRC(a1aa0191) SHA1(193d936e1bfe0da4ac984aba65d3e4e6c93a4c11) )
	ROM_LOAD16_BYTE( "b58-19.19", 0x00001, 0x20000, CRC(40f43da3) SHA1(bb3d6c6db8df77674bb76c16992d05c297d97c9f) )

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "b58-16.26", 0x00000, 0x20000, CRC(e823c85c) SHA1(199b19e81c76eb936f4cf31957ae08bed1395bda) )
	ROM_LOAD16_BYTE( "b58-18.18", 0x00001, 0x20000, CRC(65328a3e) SHA1(f51ca107910629e030678e183cc8fd06d2569098) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "b58-32.41",   0x00000, 0x10000, CRC(f3fd8eca) SHA1(3b1ab64984ea43805b6494f8add26210ed1175c5) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b58-09.13", 0x00000, 0x80000, CRC(9ffd5b31) SHA1(0214fb32012a48560ca9c6ed5ee969d3c41cf95c) ) /* SCR 8x8 */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "b58-04.7",  0x000000, 0x080000, CRC(9482f08d) SHA1(3fc74b9bebca1d82b300ba72c7297c3bcd69cfa9) )
	ROM_LOAD64_WORD_SWAP( "b58-03.6",  0x000002, 0x080000, CRC(158bc440) SHA1(ceab296146363a2e9a48f62118fba6123b4b5a1b) )    /* OBJ 16x8 */
	ROM_LOAD64_WORD_SWAP( "b58-02.2",  0x000004, 0x080000, CRC(6a6e307c) SHA1(fc4a68220e0dd0e64d75ba7c7af0c1ac97dc7fd9) )
	ROM_LOAD64_WORD_SWAP( "b58-01.1",  0x000006, 0x080000, CRC(01e9f0a8) SHA1(0d3a4dc81702e3c57c790eb8a45caca36cb47d4c) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "b58-06.116", 0x00000, 0x80000, CRC(b3495d70) SHA1(ead4c2fd20b8f103a849201c7344cded013eb8bb) )    /* ROD, road lines */

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "b58-05.71", 0x00000, 0x80000, CRC(d1f4991b) SHA1(f1c5a9b8dce994d013290e98fda7bedf73e95900) )  /* STY spritemap */

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "b58-07.11", 0x000000, 0x080000, CRC(eeb5ba08) SHA1(fe40333e09339c76e503ce87b42a89b48d487016) )
	ROM_LOAD( "b58-08.12", 0x080000, 0x080000, CRC(049243cf) SHA1(1f3099b6d764114dc4161ed308369d0f3148dc4e) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples ??? */
	ROM_LOAD( "b58-10.14", 0x00000, 0x80000, CRC(edce0cc1) SHA1(1f6cbc60502b8b12b349e48446ce3a4a1f76bccd) ) /* ??? */

	ROM_REGION( 0x10000, "user2", 0 )   /* unused ROMs */
	ROM_LOAD( "b58-26.104", 0x00000, 0x10000, CRC(dccb0c7f) SHA1(42f0af72f559133b74912a4478e1323062be4b77) )    // sprite vertical zoom
	ROM_LOAD( "b58-27.56",  0x00000, 0x02000, CRC(5c6b013d) SHA1(6d02d4560076213b6fb6fe856143bb533090603e) )    // sprite horizontal zoom
	ROM_LOAD( "b58-23.52",  0x00000, 0x00100, CRC(7b7d8ff4) SHA1(18842ed8160739cd2e2ccc2db605153dbed6cc0a) )    // road/sprite priority and palette select
	ROM_LOAD( "b58-24.51",  0x00000, 0x00100, CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) )    // road A/B internal priority
	ROM_LOAD( "b58-25.75",  0x00000, 0x00100, CRC(de547342) SHA1(3b2b116d4016ddbf46c41c625c7fcfd76129baa7) )
// Add pals...
ROM_END

ROM_START( enforceja )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 256K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "b58-31.27", 0x00000, 0x20000, CRC(d686e371) SHA1(d2db6c093cec8211c2be1b78d7815aeef5d91fca) )
	ROM_LOAD16_BYTE( "b58-30.19", 0x00001, 0x20000, CRC(cd73c0d8) SHA1(8e1e95272f11b3be7b896e06baf1d3efa9b4c8c7) )

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "b58-29.26", 0x00000, 0x20000, CRC(8482a4e4) SHA1(32c4dd66b2062c62830c2ca2abbd3e23f1883de9) )
	ROM_LOAD16_BYTE( "b58-28.18", 0x00001, 0x20000, CRC(9735e2b1) SHA1(21e718a1a3d005d022b4aaab2da8350767f72a65) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "b58-32.41",   0x00000, 0x10000, CRC(f3fd8eca) SHA1(3b1ab64984ea43805b6494f8add26210ed1175c5) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b58-09.13", 0x00000, 0x80000, CRC(9ffd5b31) SHA1(0214fb32012a48560ca9c6ed5ee969d3c41cf95c) ) /* SCR 8x8 */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "b58-04.7",  0x000000, 0x080000, CRC(9482f08d) SHA1(3fc74b9bebca1d82b300ba72c7297c3bcd69cfa9) )
	ROM_LOAD64_WORD_SWAP( "b58-03.6",  0x000002, 0x080000, CRC(158bc440) SHA1(ceab296146363a2e9a48f62118fba6123b4b5a1b) )    /* OBJ 16x8 */
	ROM_LOAD64_WORD_SWAP( "b58-02.2",  0x000004, 0x080000, CRC(6a6e307c) SHA1(fc4a68220e0dd0e64d75ba7c7af0c1ac97dc7fd9) )
	ROM_LOAD64_WORD_SWAP( "b58-01.1",  0x000006, 0x080000, CRC(01e9f0a8) SHA1(0d3a4dc81702e3c57c790eb8a45caca36cb47d4c) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "b58-06.116", 0x00000, 0x80000, CRC(b3495d70) SHA1(ead4c2fd20b8f103a849201c7344cded013eb8bb) )    /* ROD, road lines */

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "b58-05.71", 0x00000, 0x80000, CRC(d1f4991b) SHA1(f1c5a9b8dce994d013290e98fda7bedf73e95900) )  /* STY spritemap */

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "b58-07.11", 0x000000, 0x080000, CRC(eeb5ba08) SHA1(fe40333e09339c76e503ce87b42a89b48d487016) )
	ROM_LOAD( "b58-08.12", 0x080000, 0x080000, CRC(049243cf) SHA1(1f3099b6d764114dc4161ed308369d0f3148dc4e) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples ??? */
	ROM_LOAD( "b58-10.14", 0x00000, 0x80000, CRC(edce0cc1) SHA1(1f6cbc60502b8b12b349e48446ce3a4a1f76bccd) ) /* ??? */

	ROM_REGION( 0x10000, "user2", 0 )   /* unused ROMs */
	ROM_LOAD( "b58-26.104", 0x00000, 0x10000, CRC(dccb0c7f) SHA1(42f0af72f559133b74912a4478e1323062be4b77) )    // sprite vertical zoom
	ROM_LOAD( "b58-27.56",  0x00000, 0x02000, CRC(5c6b013d) SHA1(6d02d4560076213b6fb6fe856143bb533090603e) )    // sprite horizontal zoom
	ROM_LOAD( "b58-23.52",  0x00000, 0x00100, CRC(7b7d8ff4) SHA1(18842ed8160739cd2e2ccc2db605153dbed6cc0a) )    // road/sprite priority and palette select
	ROM_LOAD( "b58-24.51",  0x00000, 0x00100, CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) )    // road A/B internal priority
	ROM_LOAD( "b58-25.75",  0x00000, 0x00100, CRC(de547342) SHA1(3b2b116d4016ddbf46c41c625c7fcfd76129baa7) )
// Add pals...
ROM_END

ROM_START( bshark )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "c34_71.98", 0x00000, 0x20000, CRC(df1fa629) SHA1(6cb207e577fac85da654f3dc56e2f9f25c38a76d) )
	ROM_LOAD16_BYTE( "c34_69.75", 0x00001, 0x20000, CRC(a54c137a) SHA1(632bf2d65f54035de2ecb87648dafa877c45e428) )
	ROM_LOAD16_BYTE( "c34_70.97", 0x40000, 0x20000, CRC(d77d81e2) SHA1(d60e586cefd9001e87cae583ca25bf5a8a461d8d) )
	ROM_LOAD16_BYTE( "c34_68.74", 0x40001, 0x20000, CRC(4e374ce2) SHA1(bf0436d40cfed75dcbd3e40a7c6aa45eeff6666e) )

	ROM_REGION( 0x80000, "sub", 0 ) /* 512K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "c34_74.128", 0x00000, 0x20000, CRC(6869fa99) SHA1(16221f25c865a81ca4f6a987b6de02a3ccf3208c) )
	ROM_LOAD16_BYTE( "c34_72.112", 0x00001, 0x20000, CRC(c09c0f91) SHA1(32c78924617328abb11c094f89a90a92e72ed5e6) )
	ROM_LOAD16_BYTE( "c34_75.129", 0x40000, 0x20000, CRC(6ba65542) SHA1(9ba5af9dd240a198dfa760ca14b0f0c84eb307c9) )
	ROM_LOAD16_BYTE( "c34_73.113", 0x40001, 0x20000, CRC(f2fe62b5) SHA1(e31b5989b747de451ee6c2a5e15ec75235d84e0d) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c34_05.3", 0x00000, 0x80000, CRC(596b83da) SHA1(826cf1e48a017a0cbfcc4a4f507dfb285594178b) )  /* SCR 8x8 */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "c34_04.17", 0x000000, 0x080000, CRC(2446b0da) SHA1(bce5c73533e2bb7dfa7f18fad510f818cf1a542a) )    /* OBJ 16x8 */
	ROM_LOAD64_WORD_SWAP( "c34_03.16", 0x000002, 0x080000, CRC(a18eab78) SHA1(155f0efbfe73e18355804477d4b8954bb47bf1ef) )
	ROM_LOAD64_WORD_SWAP( "c34_02.15", 0x000004, 0x080000, CRC(8488ba10) SHA1(60f8f0dc9d4bc6bc452527250221c9915e9dfe6e) )
	ROM_LOAD64_WORD_SWAP( "c34_01.14", 0x000006, 0x080000, CRC(3ebe8c63) SHA1(fa7403bf895c041cb64234209c944683ae372e57) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "c34_07.42", 0x00000, 0x80000, CRC(edb07808) SHA1(f32b4b93e9125536376d96fbca76c2b2f5f78656) ) /* ROD, road lines */

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "c34_06.12", 0x00000, 0x80000, CRC(d200b6eb) SHA1(6bfe3a7dde8d4e983521877d2bb176f5d126b763) )  /* STY spritemap */

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c34_08.127", 0x00000, 0x80000, CRC(89a30450) SHA1(96b96ca5a3e20cdceb9ac5ddf377fb21a9a529fb) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c34_09.126", 0x00000, 0x80000, CRC(39d12b50) SHA1(5c5d1369597604376943e4825f6c09cc28d66047) )

	ROM_REGION( 0x10000, "user2", 0 )   /* unused ROMs */
	ROM_LOAD( "c34_18.22", 0x00000, 0x10000, CRC(7245a6f6) SHA1(5bdde4e3bcde8c59dc84478c3cc079d7ef8ee9c5) )
	ROM_LOAD( "c34_19.72", 0x00000, 0x00100, CRC(2ee9c404) SHA1(3a2ddaaaf7abe9f47f7e062b002fd3a61c80f60b) ) // road/sprite priority and palette select
	ROM_LOAD( "c34_20.89", 0x00000, 0x00100, CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) ) // road A/B internal priority
	ROM_LOAD( "c34_21.7",  0x00000, 0x00400, CRC(10728853) SHA1(45d7cc8e06fbe01295cc2194bca9586f0ef8b12b) )
	ROM_LOAD( "c34_22.8",  0x00000, 0x00400, CRC(643e8bfc) SHA1(a6e6086fb8fbd102e01ec72fe60a4232f5909565) )
ROM_END

ROM_START( bsharku )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "c34_71.98", 0x00000, 0x20000, CRC(df1fa629) SHA1(6cb207e577fac85da654f3dc56e2f9f25c38a76d) )
	ROM_LOAD16_BYTE( "c34_69.75", 0x00001, 0x20000, CRC(a54c137a) SHA1(632bf2d65f54035de2ecb87648dafa877c45e428) )
	ROM_LOAD16_BYTE( "c34_70.97", 0x40000, 0x20000, CRC(d77d81e2) SHA1(d60e586cefd9001e87cae583ca25bf5a8a461d8d) )
	ROM_LOAD16_BYTE( "c34_67.74", 0x40001, 0x20000, CRC(39307c74) SHA1(65d1cb6b0baee29c1439180b8b4c6907e20b2921) )

	ROM_REGION( 0x80000, "sub", 0 ) /* 512K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "c34_74.128", 0x00000, 0x20000, CRC(6869fa99) SHA1(16221f25c865a81ca4f6a987b6de02a3ccf3208c) )
	ROM_LOAD16_BYTE( "c34_72.112", 0x00001, 0x20000, CRC(c09c0f91) SHA1(32c78924617328abb11c094f89a90a92e72ed5e6) )
	ROM_LOAD16_BYTE( "c34_75.129", 0x40000, 0x20000, CRC(6ba65542) SHA1(9ba5af9dd240a198dfa760ca14b0f0c84eb307c9) )
	ROM_LOAD16_BYTE( "c34_73.113", 0x40001, 0x20000, CRC(f2fe62b5) SHA1(e31b5989b747de451ee6c2a5e15ec75235d84e0d) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c34_05.3", 0x00000, 0x80000, CRC(596b83da) SHA1(826cf1e48a017a0cbfcc4a4f507dfb285594178b) )  /* SCR 8x8 */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "c34_04.17", 0x000000, 0x080000, CRC(2446b0da) SHA1(bce5c73533e2bb7dfa7f18fad510f818cf1a542a) )    /* OBJ 16x8 */
	ROM_LOAD64_WORD_SWAP( "c34_03.16", 0x000002, 0x080000, CRC(a18eab78) SHA1(155f0efbfe73e18355804477d4b8954bb47bf1ef) )
	ROM_LOAD64_WORD_SWAP( "c34_02.15", 0x000004, 0x080000, CRC(8488ba10) SHA1(60f8f0dc9d4bc6bc452527250221c9915e9dfe6e) )
	ROM_LOAD64_WORD_SWAP( "c34_01.14", 0x000006, 0x080000, CRC(3ebe8c63) SHA1(fa7403bf895c041cb64234209c944683ae372e57) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "c34_07.42", 0x00000, 0x80000, CRC(edb07808) SHA1(f32b4b93e9125536376d96fbca76c2b2f5f78656) ) /* ROD, road lines */

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "c34_06.12", 0x00000, 0x80000, CRC(d200b6eb) SHA1(6bfe3a7dde8d4e983521877d2bb176f5d126b763) )  /* STY spritemap */

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c34_08.127", 0x00000, 0x80000, CRC(89a30450) SHA1(96b96ca5a3e20cdceb9ac5ddf377fb21a9a529fb) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c34_09.126", 0x00000, 0x80000, CRC(39d12b50) SHA1(5c5d1369597604376943e4825f6c09cc28d66047) )

	ROM_REGION( 0x10000, "user2", 0 )   /* unused ROMs */
	ROM_LOAD( "c34_18.22", 0x00000, 0x10000, CRC(7245a6f6) SHA1(5bdde4e3bcde8c59dc84478c3cc079d7ef8ee9c5) )
	ROM_LOAD( "c34_19.72", 0x00000, 0x00100, CRC(2ee9c404) SHA1(3a2ddaaaf7abe9f47f7e062b002fd3a61c80f60b) ) // road/sprite priority and palette select
	ROM_LOAD( "c34_20.89", 0x00000, 0x00100, CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) ) // road A/B internal priority
	ROM_LOAD( "c34_21.7",  0x00000, 0x00400, CRC(10728853) SHA1(45d7cc8e06fbe01295cc2194bca9586f0ef8b12b) )
	ROM_LOAD( "c34_22.8",  0x00000, 0x00400, CRC(643e8bfc) SHA1(a6e6086fb8fbd102e01ec72fe60a4232f5909565) )
ROM_END

ROM_START( bsharkj )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "c34_71.98", 0x00000, 0x20000, CRC(df1fa629) SHA1(6cb207e577fac85da654f3dc56e2f9f25c38a76d) )
	ROM_LOAD16_BYTE( "c34_69.75", 0x00001, 0x20000, CRC(a54c137a) SHA1(632bf2d65f54035de2ecb87648dafa877c45e428) )
	ROM_LOAD16_BYTE( "c34_70.97", 0x40000, 0x20000, CRC(d77d81e2) SHA1(d60e586cefd9001e87cae583ca25bf5a8a461d8d) )
	ROM_LOAD16_BYTE( "c34_66.74", 0x40001, 0x20000, CRC(a0392dce) SHA1(5d20f39b75e921fda82c33990463cec73879d113) )

	ROM_REGION( 0x80000, "sub", 0 ) /* 512K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "c34_74.128", 0x00000, 0x20000, CRC(6869fa99) SHA1(16221f25c865a81ca4f6a987b6de02a3ccf3208c) )
	ROM_LOAD16_BYTE( "c34_72.112", 0x00001, 0x20000, CRC(c09c0f91) SHA1(32c78924617328abb11c094f89a90a92e72ed5e6) )
	ROM_LOAD16_BYTE( "c34_75.129", 0x40000, 0x20000, CRC(6ba65542) SHA1(9ba5af9dd240a198dfa760ca14b0f0c84eb307c9) )
	ROM_LOAD16_BYTE( "c34_73.113", 0x40001, 0x20000, CRC(f2fe62b5) SHA1(e31b5989b747de451ee6c2a5e15ec75235d84e0d) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c34_05.3", 0x00000, 0x80000, CRC(596b83da) SHA1(826cf1e48a017a0cbfcc4a4f507dfb285594178b) )  /* SCR 8x8 */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "c34_04.17", 0x000000, 0x080000, CRC(2446b0da) SHA1(bce5c73533e2bb7dfa7f18fad510f818cf1a542a) )    /* OBJ 16x8 */
	ROM_LOAD64_WORD_SWAP( "c34_03.16", 0x000002, 0x080000, CRC(a18eab78) SHA1(155f0efbfe73e18355804477d4b8954bb47bf1ef) )
	ROM_LOAD64_WORD_SWAP( "c34_02.15", 0x000004, 0x080000, CRC(8488ba10) SHA1(60f8f0dc9d4bc6bc452527250221c9915e9dfe6e) )
	ROM_LOAD64_WORD_SWAP( "c34_01.14", 0x000006, 0x080000, CRC(3ebe8c63) SHA1(fa7403bf895c041cb64234209c944683ae372e57) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "c34_07.42", 0x00000, 0x80000, CRC(edb07808) SHA1(f32b4b93e9125536376d96fbca76c2b2f5f78656) ) /* ROD, road lines */

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "c34_06.12", 0x00000, 0x80000, CRC(d200b6eb) SHA1(6bfe3a7dde8d4e983521877d2bb176f5d126b763) )  /* STY spritemap */

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c34_08.127", 0x00000, 0x80000, CRC(89a30450) SHA1(96b96ca5a3e20cdceb9ac5ddf377fb21a9a529fb) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c34_09.126", 0x00000, 0x80000, CRC(39d12b50) SHA1(5c5d1369597604376943e4825f6c09cc28d66047) )

	ROM_REGION( 0x10000, "user2", 0 )   /* unused ROMs */
	ROM_LOAD( "c34_18.22", 0x00000, 0x10000, CRC(7245a6f6) SHA1(5bdde4e3bcde8c59dc84478c3cc079d7ef8ee9c5) )
	ROM_LOAD( "c34_19.72", 0x00000, 0x00100, CRC(2ee9c404) SHA1(3a2ddaaaf7abe9f47f7e062b002fd3a61c80f60b) ) // road/sprite priority and palette select
	ROM_LOAD( "c34_20.89", 0x00000, 0x00100, CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) ) // road A/B internal priority
	ROM_LOAD( "c34_21.7",  0x00000, 0x00400, CRC(10728853) SHA1(45d7cc8e06fbe01295cc2194bca9586f0ef8b12b) )
	ROM_LOAD( "c34_22.8",  0x00000, 0x00400, CRC(643e8bfc) SHA1(a6e6086fb8fbd102e01ec72fe60a4232f5909565) )
ROM_END

ROM_START( bsharkjjs )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "c34_79.98", 0x00000, 0x20000, CRC(bc3f2e93) SHA1(a03778fb8c8fb91956005cab0f2050262bc8f306) )
	ROM_LOAD16_BYTE( "c34_77.75", 0x00001, 0x20000, CRC(917916d0) SHA1(86db550737a20fd5aa2862f7a6be0d47da5fc74e) )
	ROM_LOAD16_BYTE( "c34_78.97", 0x40000, 0x20000, CRC(f2fcc880) SHA1(6d8530056bd2e0e54061d95048b3b5e0b1eb76ef) )
	ROM_LOAD16_BYTE( "c34_76.74", 0x40001, 0x20000, CRC(de97fac0) SHA1(53baf70bbf0102ff965921330e2d7a918318acff) )

	ROM_REGION( 0x80000, "sub", 0 ) /* 512K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "c34_82.128", 0x00000, 0x20000, CRC(6869fa99) SHA1(16221f25c865a81ca4f6a987b6de02a3ccf3208c) )
	ROM_LOAD16_BYTE( "c34_80.112", 0x00001, 0x20000, CRC(e1783eb4) SHA1(02aaaf117f258625052734064692d2c1679b80b6) )
	ROM_LOAD16_BYTE( "c34_83.129", 0x40000, 0x20000, CRC(eec0b364) SHA1(17010b19570ee65020ae09e5734b48a763a12e3f) )
	ROM_LOAD16_BYTE( "c34_81.113", 0x40001, 0x20000, CRC(23ce6bcf) SHA1(b084209f809793d8f0f11ddabee217ba1abd6038) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c34_05.3", 0x00000, 0x80000, CRC(596b83da) SHA1(826cf1e48a017a0cbfcc4a4f507dfb285594178b) )  /* SCR 8x8 */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "c34_04.17", 0x000000, 0x080000, CRC(2446b0da) SHA1(bce5c73533e2bb7dfa7f18fad510f818cf1a542a) )    /* OBJ 16x8 */
	ROM_LOAD64_WORD_SWAP( "c34_03.16", 0x000002, 0x080000, CRC(a18eab78) SHA1(155f0efbfe73e18355804477d4b8954bb47bf1ef) )
	ROM_LOAD64_WORD_SWAP( "c34_02.15", 0x000004, 0x080000, CRC(8488ba10) SHA1(60f8f0dc9d4bc6bc452527250221c9915e9dfe6e) )
	ROM_LOAD64_WORD_SWAP( "c34_01.14", 0x000006, 0x080000, CRC(3ebe8c63) SHA1(fa7403bf895c041cb64234209c944683ae372e57) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "c34_07.42", 0x00000, 0x80000, CRC(edb07808) SHA1(f32b4b93e9125536376d96fbca76c2b2f5f78656) ) /* ROD, road lines */

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "c34_06.12", 0x00000, 0x80000, CRC(d200b6eb) SHA1(6bfe3a7dde8d4e983521877d2bb176f5d126b763) )  /* STY spritemap */

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c34_08.127", 0x00000, 0x80000, CRC(89a30450) SHA1(96b96ca5a3e20cdceb9ac5ddf377fb21a9a529fb) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c34_09.126", 0x00000, 0x80000, CRC(39d12b50) SHA1(5c5d1369597604376943e4825f6c09cc28d66047) )

	ROM_REGION( 0x10000, "user2", 0 )   /* unused ROMs */
	ROM_LOAD( "c34_18.22", 0x00000, 0x10000, CRC(7245a6f6) SHA1(5bdde4e3bcde8c59dc84478c3cc079d7ef8ee9c5) )
	ROM_LOAD( "c34_19.72", 0x00000, 0x00100, CRC(2ee9c404) SHA1(3a2ddaaaf7abe9f47f7e062b002fd3a61c80f60b) ) // road/sprite priority and palette select
	ROM_LOAD( "c34_20.89", 0x00000, 0x00100, CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) ) // road A/B internal priority
	ROM_LOAD( "c34_21.7",  0x00000, 0x00400, CRC(10728853) SHA1(45d7cc8e06fbe01295cc2194bca9586f0ef8b12b) )
	ROM_LOAD( "c34_22.8",  0x00000, 0x00400, CRC(643e8bfc) SHA1(a6e6086fb8fbd102e01ec72fe60a4232f5909565) )
ROM_END

ROM_START( sci )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "c09-37.43", 0x00000, 0x20000, CRC(0fecea17) SHA1(0ad4454eee6646b0f978b1ba83206d64c1f6d081) )
	ROM_LOAD16_BYTE( "c09-38.40", 0x00001, 0x20000, CRC(e46ebd9b) SHA1(52b0c1f95e8a664076d8fbc0f6204ca55893e281) )
	ROM_LOAD16_BYTE( "c09-42.38", 0x40000, 0x20000, CRC(f4404f87) SHA1(8f051f1ffbf323cb3d613bc22afa53676590f29c) )
	ROM_LOAD16_BYTE( "c09-39.41", 0x40001, 0x20000, CRC(de87bcb9) SHA1(b5537a25871ea90294f3b6f0b6386a883cfdf991) )

	ROM_REGION( 0x20000, "sub", 0 ) /* 128K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "c09-33.6", 0x00000, 0x10000, CRC(cf4e6c5b) SHA1(8d6720b605b8e0c7f0473ba452c79bf5efc2615d) ) /* Actual label is "C09 33*" */
	ROM_LOAD16_BYTE( "c09-32.5", 0x00001, 0x10000, CRC(a4713719) SHA1(b1110e397d3407ec63975cdd92a23cbb16348200) ) /* Actual label is "C09 32*" */

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "c09-34.31",   0x00000, 0x20000, CRC(a21b3151) SHA1(f59c7b1ba5edf97d72670ee194ce9fdc5c5b9a58) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c09-05.16", 0x00000, 0x80000, CRC(890b38f0) SHA1(b478c96214ce027926346a4653250c8ee8a98bdc) ) /* SCR 8x8 */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "c09-04.52", 0x000000, 0x080000, CRC(2cbb3c9b) SHA1(9e3d95f76f5f5d385b6a9516af781aefef1eb0ca) )    /* OBJ 16x8 */
	ROM_LOAD64_WORD_SWAP( "c09-02.53", 0x000002, 0x080000, CRC(a83a0389) SHA1(932788b5b5f01326d0fbb2b9fdb94a8c7c004db3) )
	ROM_LOAD64_WORD_SWAP( "c09-03.54", 0x000004, 0x080000, CRC(a31d0e80) SHA1(dfeff1b89dd7b3f19b26e77f2d66f6448cb00553) )
	ROM_LOAD64_WORD_SWAP( "c09-01.55", 0x000006, 0x080000, CRC(64bfea10) SHA1(15ea43092027b1717d0f24fbe6ac2cdf11a7ddc6) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "c09-07.15", 0x00000, 0x80000, CRC(963bc82b) SHA1(e3558aecd1b82ddbf10ab2b71843a3664705f1f1) ) /* ROD, road lines */

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "c09-06.37", 0x00000, 0x80000, CRC(12df6d7b) SHA1(8ce742eb3f7eb6283b5ca32bb520d1cc7684d515) )  /* STY spritemap */

	ROM_REGION( 0x180000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c09-14.42", 0x000000, 0x080000, CRC(ad78bf46) SHA1(4020744bbdc4b9ec3dee1a9d7b5ffa8def43d7b2) )
	ROM_LOAD( "c09-13.43", 0x080000, 0x080000, CRC(d57c41d3) SHA1(3375a1fc6389840544b9fdb96b2fafbc8e3276e2) )
	ROM_LOAD( "c09-12.44", 0x100000, 0x080000, CRC(56c99fa5) SHA1(3f9a6bc89d847cc4c99d35f98157ea3f187c0f98) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c09-15.29", 0x00000, 0x80000, CRC(e63b9095) SHA1(c6ea670b5a90ab39429259ec1fefb2bde5d0213f) )

	ROM_REGION( 0x10000, "user2", 0 )   /* unused ROMs */
	ROM_LOAD( "c09-16.17", 0x00000, 0x10000, CRC(7245a6f6) SHA1(5bdde4e3bcde8c59dc84478c3cc079d7ef8ee9c5) )
	ROM_LOAD( "c09-17.24", 0x00000, 0x00400, CRC(10728853) SHA1(45d7cc8e06fbe01295cc2194bca9586f0ef8b12b) ) // same data also found on Racing Beat, Double Axle & Battle Shark
	ROM_LOAD( "c09-18.25", 0x00000, 0x00400, CRC(643e8bfc) SHA1(a6e6086fb8fbd102e01ec72fe60a4232f5909565) ) // same data also found on Racing Beat, Double Axle & Battle Shark
	ROM_LOAD( "c09-20.71", 0x00000, 0x00100, CRC(cd8ffd80) SHA1(133bcd291a3751bce5293cb6b685f87258e8db19) ) // road/sprite priority and palette select
	ROM_LOAD( "c09-23.14", 0x00000, 0x00100, CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) ) // road A/B internal priority
	ROM_LOAD( "c09-19_pal16l8b.ic67", 0x00000, 0x00104, CRC(a0608442) SHA1(bcda5ef6582e82b2b369c24474bb14bd29e5d537) ) // MMI PAL16L8B
	ROM_LOAD( "c09-21_pal20l8b.ic2",  0x00000, 0x00144, CRC(583f9214) SHA1(1a5f3e9619bf6ba95f92ce9d12917014db8853a1) ) // MMI PAL20L8B
	ROM_LOAD( "c09-22_pal16l8b.ic3",  0x00000, 0x00104, CRC(b506d7a7) SHA1(603eb14d2703b94ae80894daa7f9f25890b7918c) ) // MMI PAL16L8B
	ROM_LOAD( "c09-24_pal20l8b.ic22", 0x00000, 0x00144, CRC(2ff83694) SHA1(c6594b1cfd3958d9b4ddb8d68d506929cfa0a759) ) // MMI PAL20L8B
	ROM_LOAD( "c09-25_pal20l8b.ic25", 0x00000, 0x00144, CRC(c69bf3fc) SHA1(83f1cc9f475aed47e969b77059f3332ef5068981) ) // MMI PAL20L8B
	ROM_LOAD( "c09-26_pal16l8b.ic26", 0x00000, 0x00104, CRC(36a8eb27) SHA1(8ec25c860e6568a3d1a7e7a8ed6a6397f135455c) ) // MMI PAL16L8B
ROM_END

ROM_START( scia )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "c09-28.43",  0x00000, 0x20000, CRC(630dbaad) SHA1(090f6a97007ac04f64d92ae5823b7254152144af) )
	ROM_LOAD16_BYTE( "c09-30.40",  0x00001, 0x20000, CRC(68b1a97d) SHA1(c377f7880154b38fe25dc0ec420ca0cd7228fbad) )
	ROM_LOAD16_BYTE( "c09-36.38",  0x40000, 0x20000, CRC(59e47cba) SHA1(313302bc62ff02b437b1091d394d2010ce66c7e7) )
	ROM_LOAD16_BYTE( "c09-31.41",  0x40001, 0x20000, CRC(962b1fbf) SHA1(62181a289dfc6d1da674ba4bcbefeb16a67a55e3) )

	ROM_REGION( 0x20000, "sub", 0 ) /* 128K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "c09-33.6", 0x00000, 0x10000, CRC(cf4e6c5b) SHA1(8d6720b605b8e0c7f0473ba452c79bf5efc2615d) ) /* Actual label is "C09 33*" */
	ROM_LOAD16_BYTE( "c09-32.5", 0x00001, 0x10000, CRC(a4713719) SHA1(b1110e397d3407ec63975cdd92a23cbb16348200) ) /* Actual label is "C09 32*" */

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "c09-34.31",   0x00000, 0x20000, CRC(a21b3151) SHA1(f59c7b1ba5edf97d72670ee194ce9fdc5c5b9a58) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c09-05.16", 0x00000, 0x80000, CRC(890b38f0) SHA1(b478c96214ce027926346a4653250c8ee8a98bdc) ) /* SCR 8x8 */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "c09-04.52", 0x000000, 0x080000, CRC(2cbb3c9b) SHA1(9e3d95f76f5f5d385b6a9516af781aefef1eb0ca) )    /* OBJ 16x8 */
	ROM_LOAD64_WORD_SWAP( "c09-02.53", 0x000002, 0x080000, CRC(a83a0389) SHA1(932788b5b5f01326d0fbb2b9fdb94a8c7c004db3) )
	ROM_LOAD64_WORD_SWAP( "c09-03.54", 0x000004, 0x080000, CRC(a31d0e80) SHA1(dfeff1b89dd7b3f19b26e77f2d66f6448cb00553) )
	ROM_LOAD64_WORD_SWAP( "c09-01.55", 0x000006, 0x080000, CRC(64bfea10) SHA1(15ea43092027b1717d0f24fbe6ac2cdf11a7ddc6) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "c09-07.15", 0x00000, 0x80000, CRC(963bc82b) SHA1(e3558aecd1b82ddbf10ab2b71843a3664705f1f1) ) /* ROD, road lines */

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "c09-06.37", 0x00000, 0x80000, CRC(12df6d7b) SHA1(8ce742eb3f7eb6283b5ca32bb520d1cc7684d515) )  /* STY spritemap */

	ROM_REGION( 0x180000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c09-14.42", 0x000000, 0x080000, CRC(ad78bf46) SHA1(4020744bbdc4b9ec3dee1a9d7b5ffa8def43d7b2) )
	ROM_LOAD( "c09-13.43", 0x080000, 0x080000, CRC(d57c41d3) SHA1(3375a1fc6389840544b9fdb96b2fafbc8e3276e2) )
	ROM_LOAD( "c09-12.44", 0x100000, 0x080000, CRC(56c99fa5) SHA1(3f9a6bc89d847cc4c99d35f98157ea3f187c0f98) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c09-15.29", 0x00000, 0x80000, CRC(e63b9095) SHA1(c6ea670b5a90ab39429259ec1fefb2bde5d0213f) )

	ROM_REGION( 0x10000, "user2", 0 )   /* unused ROMs */
	ROM_LOAD( "c09-16.17", 0x00000, 0x10000, CRC(7245a6f6) SHA1(5bdde4e3bcde8c59dc84478c3cc079d7ef8ee9c5) )
	ROM_LOAD( "c09-17.24", 0x00000, 0x00400, CRC(10728853) SHA1(45d7cc8e06fbe01295cc2194bca9586f0ef8b12b) ) // AMD AM27S33 BPROM - same data also found on Racing Beat, Double Axle & Battle Shark
	ROM_LOAD( "c09-18.25", 0x00000, 0x00400, CRC(643e8bfc) SHA1(a6e6086fb8fbd102e01ec72fe60a4232f5909565) ) // AMD AM27S33 BPROM - same data also found on Racing Beat, Double Axle & Battle Shark
	ROM_LOAD( "c09-20.71", 0x00000, 0x00100, CRC(cd8ffd80) SHA1(133bcd291a3751bce5293cb6b685f87258e8db19) ) // AMD AM27S21 BPROM - road/sprite priority and palette select
	ROM_LOAD( "c09-23.14", 0x00000, 0x00100, CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) ) // AMD AM27S21 BPROM - road A/B internal priority
	ROM_LOAD( "c09-19_pal16l8b.ic67", 0x00000, 0x00104, CRC(a0608442) SHA1(bcda5ef6582e82b2b369c24474bb14bd29e5d537) ) // MMI PAL16L8B
	ROM_LOAD( "c09-21_pal20l8b.ic2",  0x00000, 0x00144, CRC(583f9214) SHA1(1a5f3e9619bf6ba95f92ce9d12917014db8853a1) ) // MMI PAL20L8B
	ROM_LOAD( "c09-22_pal16l8b.ic3",  0x00000, 0x00104, CRC(b506d7a7) SHA1(603eb14d2703b94ae80894daa7f9f25890b7918c) ) // MMI PAL16L8B
	ROM_LOAD( "c09-24_pal20l8b.ic22", 0x00000, 0x00144, CRC(2ff83694) SHA1(c6594b1cfd3958d9b4ddb8d68d506929cfa0a759) ) // MMI PAL20L8B
	ROM_LOAD( "c09-25_pal20l8b.ic25", 0x00000, 0x00144, CRC(c69bf3fc) SHA1(83f1cc9f475aed47e969b77059f3332ef5068981) ) // MMI PAL20L8B
	ROM_LOAD( "c09-26_pal16l8b.ic26", 0x00000, 0x00104, CRC(36a8eb27) SHA1(8ec25c860e6568a3d1a7e7a8ed6a6397f135455c) ) // MMI PAL16L8B
ROM_END

ROM_START( scij )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "c09-37.43", 0x00000, 0x20000, CRC(0fecea17) SHA1(0ad4454eee6646b0f978b1ba83206d64c1f6d081) )
	ROM_LOAD16_BYTE( "c09-38.40", 0x00001, 0x20000, CRC(e46ebd9b) SHA1(52b0c1f95e8a664076d8fbc0f6204ca55893e281) )
	ROM_LOAD16_BYTE( "c09-40.38", 0x40000, 0x20000, CRC(1a4e2eab) SHA1(7c95ba516d164b7b4e6eaf80e3dacf7c35d8123d) )
	ROM_LOAD16_BYTE( "c09-39.41", 0x40001, 0x20000, CRC(de87bcb9) SHA1(b5537a25871ea90294f3b6f0b6386a883cfdf991) )

	ROM_REGION( 0x20000, "sub", 0 ) /* 128K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "c09-33.6", 0x00000, 0x10000, CRC(cf4e6c5b) SHA1(8d6720b605b8e0c7f0473ba452c79bf5efc2615d) ) /* Actual label is "C09 33*" */
	ROM_LOAD16_BYTE( "c09-32.5", 0x00001, 0x10000, CRC(a4713719) SHA1(b1110e397d3407ec63975cdd92a23cbb16348200) ) /* Actual label is "C09 32*" */

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "c09-27.31",   0x00000, 0x20000, CRC(cd161dca) SHA1(2e0632f290f8efae5e479c67ca8808a90e0f4afd) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c09-05.16", 0x00000, 0x80000, CRC(890b38f0) SHA1(b478c96214ce027926346a4653250c8ee8a98bdc) ) /* SCR 8x8 */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "c09-04.52", 0x000000, 0x080000, CRC(2cbb3c9b) SHA1(9e3d95f76f5f5d385b6a9516af781aefef1eb0ca) )    /* OBJ 16x8 */
	ROM_LOAD64_WORD_SWAP( "c09-02.53", 0x000002, 0x080000, CRC(a83a0389) SHA1(932788b5b5f01326d0fbb2b9fdb94a8c7c004db3) )
	ROM_LOAD64_WORD_SWAP( "c09-03.54", 0x000004, 0x080000, CRC(a31d0e80) SHA1(dfeff1b89dd7b3f19b26e77f2d66f6448cb00553) )
	ROM_LOAD64_WORD_SWAP( "c09-01.55", 0x000006, 0x080000, CRC(64bfea10) SHA1(15ea43092027b1717d0f24fbe6ac2cdf11a7ddc6) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "c09-07.15", 0x00000, 0x80000, CRC(963bc82b) SHA1(e3558aecd1b82ddbf10ab2b71843a3664705f1f1) ) /* ROD, road lines */

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "c09-06.37", 0x00000, 0x80000, CRC(12df6d7b) SHA1(8ce742eb3f7eb6283b5ca32bb520d1cc7684d515) )  /* STY spritemap */

	ROM_REGION( 0x180000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c09-10.42", 0x000000, 0x080000, CRC(ad78bf46) SHA1(4020744bbdc4b9ec3dee1a9d7b5ffa8def43d7b2) )
	ROM_LOAD( "c09-09.43", 0x080000, 0x080000, CRC(6a655c00) SHA1(5ae1ee422226e386550b69a1f35668c10d3bdcc2) )
	ROM_LOAD( "c09-08.44", 0x100000, 0x080000, CRC(7ddfc316) SHA1(47f0ed8eecd4719b4c5cb8762ee6b8bb01686812) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c09-11.29", 0x00000, 0x80000, CRC(6b1a11e1) SHA1(4304d029ecf91fa5b779057f195f75ebdd0a7c1c) )

	ROM_REGION( 0x10000, "user2", 0 )   /* unused ROMs */
	ROM_LOAD( "c09-16.17", 0x00000, 0x10000, CRC(7245a6f6) SHA1(5bdde4e3bcde8c59dc84478c3cc079d7ef8ee9c5) )
	ROM_LOAD( "c09-17.24", 0x00000, 0x00400, CRC(10728853) SHA1(45d7cc8e06fbe01295cc2194bca9586f0ef8b12b) ) // AMD AM27S33 BPROM - same data also found on Racing Beat, Double Axle & Battle Shark
	ROM_LOAD( "c09-18.25", 0x00000, 0x00400, CRC(643e8bfc) SHA1(a6e6086fb8fbd102e01ec72fe60a4232f5909565) ) // AMD AM27S33 BPROM - same data also found on Racing Beat, Double Axle & Battle Shark
	ROM_LOAD( "c09-20.71", 0x00000, 0x00100, CRC(cd8ffd80) SHA1(133bcd291a3751bce5293cb6b685f87258e8db19) ) // AMD AM27S21 BPROM - road/sprite priority and palette select
	ROM_LOAD( "c09-23.14", 0x00000, 0x00100, CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) ) // AMD AM27S21 BPROM - road A/B internal priority
	ROM_LOAD( "c09-19_pal16l8b.ic67", 0x00000, 0x00104, CRC(a0608442) SHA1(bcda5ef6582e82b2b369c24474bb14bd29e5d537) ) // MMI PAL16L8B
	ROM_LOAD( "c09-21_pal20l8b.ic2",  0x00000, 0x00144, CRC(583f9214) SHA1(1a5f3e9619bf6ba95f92ce9d12917014db8853a1) ) // MMI PAL20L8B
	ROM_LOAD( "c09-22_pal16l8b.ic3",  0x00000, 0x00104, CRC(b506d7a7) SHA1(603eb14d2703b94ae80894daa7f9f25890b7918c) ) // MMI PAL16L8B
	ROM_LOAD( "c09-24_pal20l8b.ic22", 0x00000, 0x00144, CRC(2ff83694) SHA1(c6594b1cfd3958d9b4ddb8d68d506929cfa0a759) ) // MMI PAL20L8B
	ROM_LOAD( "c09-25_pal20l8b.ic25", 0x00000, 0x00144, CRC(c69bf3fc) SHA1(83f1cc9f475aed47e969b77059f3332ef5068981) ) // MMI PAL20L8B
	ROM_LOAD( "c09-26_pal16l8b.ic26", 0x00000, 0x00104, CRC(36a8eb27) SHA1(8ec25c860e6568a3d1a7e7a8ed6a6397f135455c) ) // MMI PAL16L8B
ROM_END

ROM_START( sciu )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "c09-43.43",  0x00000, 0x20000, CRC(20a9343e) SHA1(b0185ddbda827236b7b41687f18c92e10c2dbd3a) )
	ROM_LOAD16_BYTE( "c09-44.40",  0x00001, 0x20000, CRC(7524338a) SHA1(f4e68a4d09f843f4697b4b4a4e94b5759a14fd01) )
	ROM_LOAD16_BYTE( "c09-41.38",  0x40000, 0x20000, CRC(83477f11) SHA1(f6dba2137a182dae215cf212bf85f4528e3d006d) )
	ROM_LOAD16_BYTE( "c09-39.41",  0x40001, 0x20000, CRC(de87bcb9) SHA1(b5537a25871ea90294f3b6f0b6386a883cfdf991) )

	ROM_REGION( 0x20000, "sub", 0 ) /* 128K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "c09-33.6", 0x00000, 0x10000, CRC(cf4e6c5b) SHA1(8d6720b605b8e0c7f0473ba452c79bf5efc2615d) ) /* Actual label is "C09 33*" */
	ROM_LOAD16_BYTE( "c09-32.5", 0x00001, 0x10000, CRC(a4713719) SHA1(b1110e397d3407ec63975cdd92a23cbb16348200) ) /* Actual label is "C09 32*" */

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "c09-34.31",   0x00000, 0x20000, CRC(a21b3151) SHA1(f59c7b1ba5edf97d72670ee194ce9fdc5c5b9a58) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c09-05.16", 0x00000, 0x80000, CRC(890b38f0) SHA1(b478c96214ce027926346a4653250c8ee8a98bdc) ) /* SCR 8x8 */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "c09-04.52", 0x000000, 0x080000, CRC(2cbb3c9b) SHA1(9e3d95f76f5f5d385b6a9516af781aefef1eb0ca) )    /* OBJ 16x8 */
	ROM_LOAD64_WORD_SWAP( "c09-02.53", 0x000002, 0x080000, CRC(a83a0389) SHA1(932788b5b5f01326d0fbb2b9fdb94a8c7c004db3) )
	ROM_LOAD64_WORD_SWAP( "c09-03.54", 0x000004, 0x080000, CRC(a31d0e80) SHA1(dfeff1b89dd7b3f19b26e77f2d66f6448cb00553) )
	ROM_LOAD64_WORD_SWAP( "c09-01.55", 0x000006, 0x080000, CRC(64bfea10) SHA1(15ea43092027b1717d0f24fbe6ac2cdf11a7ddc6) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "c09-07.15", 0x00000, 0x80000, CRC(963bc82b) SHA1(e3558aecd1b82ddbf10ab2b71843a3664705f1f1) ) /* ROD, road lines */

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "c09-06.37", 0x00000, 0x80000, CRC(12df6d7b) SHA1(8ce742eb3f7eb6283b5ca32bb520d1cc7684d515) )  /* STY spritemap */

	ROM_REGION( 0x180000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c09-14.42", 0x000000, 0x080000, CRC(ad78bf46) SHA1(4020744bbdc4b9ec3dee1a9d7b5ffa8def43d7b2) )
	ROM_LOAD( "c09-13.43", 0x080000, 0x080000, CRC(d57c41d3) SHA1(3375a1fc6389840544b9fdb96b2fafbc8e3276e2) )
	ROM_LOAD( "c09-12.44", 0x100000, 0x080000, CRC(56c99fa5) SHA1(3f9a6bc89d847cc4c99d35f98157ea3f187c0f98) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c09-15.29", 0x00000, 0x80000, CRC(e63b9095) SHA1(c6ea670b5a90ab39429259ec1fefb2bde5d0213f) )

	ROM_REGION( 0x10000, "user2", 0 )   /* unused ROMs */
	ROM_LOAD( "c09-16.17", 0x00000, 0x10000, CRC(7245a6f6) SHA1(5bdde4e3bcde8c59dc84478c3cc079d7ef8ee9c5) )
	ROM_LOAD( "c09-17.24", 0x00000, 0x00400, CRC(10728853) SHA1(45d7cc8e06fbe01295cc2194bca9586f0ef8b12b) ) // AMD AM27S33 BPROM - same data also found on Racing Beat, Double Axle & Battle Shark
	ROM_LOAD( "c09-18.25", 0x00000, 0x00400, CRC(643e8bfc) SHA1(a6e6086fb8fbd102e01ec72fe60a4232f5909565) ) // AMD AM27S33 BPROM - same data also found on Racing Beat, Double Axle & Battle Shark
	ROM_LOAD( "c09-20.71", 0x00000, 0x00100, CRC(cd8ffd80) SHA1(133bcd291a3751bce5293cb6b685f87258e8db19) ) // AMD AM27S21 BPROM - road/sprite priority and palette select
	ROM_LOAD( "c09-23.14", 0x00000, 0x00100, CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) ) // AMD AM27S21 BPROM - road A/B internal priority
	ROM_LOAD( "c09-19_pal16l8b.ic67", 0x00000, 0x00104, CRC(a0608442) SHA1(bcda5ef6582e82b2b369c24474bb14bd29e5d537) ) // MMI PAL16L8B
	ROM_LOAD( "c09-21_pal20l8b.ic2",  0x00000, 0x00144, CRC(583f9214) SHA1(1a5f3e9619bf6ba95f92ce9d12917014db8853a1) ) // MMI PAL20L8B
	ROM_LOAD( "c09-22_pal16l8b.ic3",  0x00000, 0x00104, CRC(b506d7a7) SHA1(603eb14d2703b94ae80894daa7f9f25890b7918c) ) // MMI PAL16L8B
	ROM_LOAD( "c09-24_pal20l8b.ic22", 0x00000, 0x00144, CRC(2ff83694) SHA1(c6594b1cfd3958d9b4ddb8d68d506929cfa0a759) ) // MMI PAL20L8B
	ROM_LOAD( "c09-25_pal20l8b.ic25", 0x00000, 0x00144, CRC(c69bf3fc) SHA1(83f1cc9f475aed47e969b77059f3332ef5068981) ) // MMI PAL20L8B
	ROM_LOAD( "c09-26_pal16l8b.ic26", 0x00000, 0x00104, CRC(36a8eb27) SHA1(8ec25c860e6568a3d1a7e7a8ed6a6397f135455c) ) // MMI PAL16L8B
ROM_END

ROM_START( scin )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "ic37.37", 0x00000, 0x20000, CRC(33fb159c) SHA1(b004a5249414f69768d8a951ded3c104ea107b32) )
	ROM_LOAD16_BYTE( "ic40.38", 0x00001, 0x20000, CRC(657df3f2) SHA1(80e30961e2cdcb834d2cbd48803ace68acaab422) )
	ROM_LOAD16_BYTE( "ic38.42", 0x40000, 0x20000, CRC(0a09b90b) SHA1(0970644a0d79ab3898187849bec6c7cf598652f3) )
	ROM_LOAD16_BYTE( "ic41.39", 0x40001, 0x20000, CRC(43167b2a) SHA1(1e6f2a113deb57df869ec2ceb99d6a7ecfa5d7e5) )

	ROM_REGION( 0x20000, "sub", 0 ) /* 128K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "c09-33.6", 0x00000, 0x10000, CRC(cf4e6c5b) SHA1(8d6720b605b8e0c7f0473ba452c79bf5efc2615d) ) /* Actual label is "C09 33*" */
	ROM_LOAD16_BYTE( "c09-32.5", 0x00001, 0x10000, CRC(a4713719) SHA1(b1110e397d3407ec63975cdd92a23cbb16348200) ) /* Actual label is "C09 32*" */

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "c09-34.31",   0x00000, 0x20000, CRC(a21b3151) SHA1(f59c7b1ba5edf97d72670ee194ce9fdc5c5b9a58) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c09-05.16", 0x00000, 0x80000, CRC(890b38f0) SHA1(b478c96214ce027926346a4653250c8ee8a98bdc) ) /* SCR 8x8 */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "c09-04.52", 0x000000, 0x080000, CRC(2cbb3c9b) SHA1(9e3d95f76f5f5d385b6a9516af781aefef1eb0ca) )    /* OBJ 16x8 */
	ROM_LOAD64_WORD_SWAP( "c09-02.53", 0x000002, 0x080000, CRC(a83a0389) SHA1(932788b5b5f01326d0fbb2b9fdb94a8c7c004db3) )
	ROM_LOAD64_WORD_SWAP( "c09-03.54", 0x000004, 0x080000, CRC(a31d0e80) SHA1(dfeff1b89dd7b3f19b26e77f2d66f6448cb00553) )
	ROM_LOAD64_WORD_SWAP( "c09-01.55", 0x000006, 0x080000, CRC(64bfea10) SHA1(15ea43092027b1717d0f24fbe6ac2cdf11a7ddc6) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "c09-07.15", 0x00000, 0x80000, CRC(963bc82b) SHA1(e3558aecd1b82ddbf10ab2b71843a3664705f1f1) ) /* ROD, road lines */

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "c09-06.37", 0x00000, 0x80000, CRC(12df6d7b) SHA1(8ce742eb3f7eb6283b5ca32bb520d1cc7684d515) )  /* STY spritemap */

	ROM_REGION( 0x180000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c09-14.42", 0x000000, 0x080000, CRC(ad78bf46) SHA1(4020744bbdc4b9ec3dee1a9d7b5ffa8def43d7b2) )
	ROM_LOAD( "c09-13.43", 0x080000, 0x080000, CRC(d57c41d3) SHA1(3375a1fc6389840544b9fdb96b2fafbc8e3276e2) )
	ROM_LOAD( "c09-12.44", 0x100000, 0x080000, CRC(56c99fa5) SHA1(3f9a6bc89d847cc4c99d35f98157ea3f187c0f98) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c09-15.29", 0x00000, 0x80000, CRC(e63b9095) SHA1(c6ea670b5a90ab39429259ec1fefb2bde5d0213f) )

	ROM_REGION( 0x10000, "user2", 0 )   /* unused ROMs */
	ROM_LOAD( "c09-16.17", 0x00000, 0x10000, CRC(7245a6f6) SHA1(5bdde4e3bcde8c59dc84478c3cc079d7ef8ee9c5) )
	ROM_LOAD( "c09-17.24", 0x00000, 0x00400, CRC(10728853) SHA1(45d7cc8e06fbe01295cc2194bca9586f0ef8b12b) ) // AMD AM27S33 BPROM - same data also found on Racing Beat, Double Axle & Battle Shark
	ROM_LOAD( "c09-18.25", 0x00000, 0x00400, CRC(643e8bfc) SHA1(a6e6086fb8fbd102e01ec72fe60a4232f5909565) ) // AMD AM27S33 BPROM - same data also found on Racing Beat, Double Axle & Battle Shark
	ROM_LOAD( "c09-20.71", 0x00000, 0x00100, CRC(cd8ffd80) SHA1(133bcd291a3751bce5293cb6b685f87258e8db19) ) // AMD AM27S21 BPROM - road/sprite priority and palette select
	ROM_LOAD( "c09-23.14", 0x00000, 0x00100, CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) ) // AMD AM27S21 BPROM - road A/B internal priority
	ROM_LOAD( "c09-19_pal16l8b.ic67", 0x00000, 0x00104, CRC(a0608442) SHA1(bcda5ef6582e82b2b369c24474bb14bd29e5d537) ) // MMI PAL16L8B
	ROM_LOAD( "c09-21_pal20l8b.ic2",  0x00000, 0x00144, CRC(583f9214) SHA1(1a5f3e9619bf6ba95f92ce9d12917014db8853a1) ) // MMI PAL20L8B
	ROM_LOAD( "c09-22_pal16l8b.ic3",  0x00000, 0x00104, CRC(b506d7a7) SHA1(603eb14d2703b94ae80894daa7f9f25890b7918c) ) // MMI PAL16L8B
	ROM_LOAD( "c09-24_pal20l8b.ic22", 0x00000, 0x00144, CRC(2ff83694) SHA1(c6594b1cfd3958d9b4ddb8d68d506929cfa0a759) ) // MMI PAL20L8B
	ROM_LOAD( "c09-25_pal20l8b.ic25", 0x00000, 0x00144, CRC(c69bf3fc) SHA1(83f1cc9f475aed47e969b77059f3332ef5068981) ) // MMI PAL20L8B
	ROM_LOAD( "c09-26_pal16l8b.ic26", 0x00000, 0x00104, CRC(36a8eb27) SHA1(8ec25c860e6568a3d1a7e7a8ed6a6397f135455c) ) // MMI PAL16L8B

ROM_END

ROM_START( nightstr )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "b91-45.bin", 0x00000, 0x20000, CRC(7ad63421) SHA1(4ecfc3c8cd691d878e5d9212ccff0d225bb06bd9) )
	ROM_LOAD16_BYTE( "b91-44.bin", 0x00001, 0x20000, CRC(4bc30adf) SHA1(531d6ee9c8ff0d4ed07c15465ec7cb78cf976115) )
	ROM_LOAD16_BYTE( "b91-43.bin", 0x40000, 0x20000, CRC(3e6f727a) SHA1(ae837131a4c0c9bc5deba155c2a5b7ae72f1d070) )
	ROM_LOAD16_BYTE( "b91-47.bin", 0x40001, 0x20000, CRC(9f778e03) SHA1(37888c3f4c52b5a714678f0f1e39f6a4f19beef9) )

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "b91-39.bin", 0x00000, 0x20000, CRC(725b23ae) SHA1(d4b4335863d32b9a81f7461240e960bf345c9835) )
	ROM_LOAD16_BYTE( "b91-40.bin", 0x00001, 0x20000, CRC(81fb364d) SHA1(f02733509039cde2c1de616e0a7969e31de1007a) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "b91-41.bin",   0x00000, 0x20000, CRC(2694bb42) SHA1(ee770472655ac0ef55eeff04037457dbf6744e4f) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b91-11.bin", 0x00000, 0x80000, CRC(fff8ce31) SHA1(fc729de92937a805d79379228d7a30041594c0df) )    /* SCR 8x8 */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "b91-04.bin", 0x000000, 0x080000, CRC(8ca1970d) SHA1(d8504298a38a95f1d8f3a2fba479ec75fe4d5de7) )   /* OBJ A 16x16 */
	ROM_LOAD64_WORD_SWAP( "b91-03.bin", 0x000002, 0x080000, CRC(cd5fed39) SHA1(c16c67cc998889288e6e96535fd8e61afc93bc78) )
	ROM_LOAD64_WORD_SWAP( "b91-02.bin", 0x000004, 0x080000, CRC(457c64b8) SHA1(443f13d56d53ca6a7750ec974da675bad3f34a38) )
	ROM_LOAD64_WORD_SWAP( "b91-01.bin", 0x000006, 0x080000, CRC(3731d94f) SHA1(2978d3eb1f44595681e84f3aa8dc03d34a191455) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "b91-10.bin", 0x00000, 0x80000, CRC(1d8f05b4) SHA1(04caa6a0887b90860c426a973dc3c3270e996818) )    /* ROD, road lines */

	ROM_REGION( 0x200000, "sprites2", 0 )
	ROM_LOAD64_WORD_SWAP( "b91-08.bin", 0x000000, 0x080000, CRC(66f35c34) SHA1(9040390fa9c626a54076a9461e0e198f059e2cb1) )   /* OBJ B 16x16 */
	ROM_LOAD64_WORD_SWAP( "b91-07.bin", 0x000002, 0x080000, CRC(4d8ec6cf) SHA1(2b7c10b459dc45313c4c90899a73c42c55b6c5c9) )
	ROM_LOAD64_WORD_SWAP( "b91-06.bin", 0x000004, 0x080000, CRC(a34dc839) SHA1(e1fcb763dbc562a62e862297458bde66d691606c) )
	ROM_LOAD64_WORD_SWAP( "b91-05.bin", 0x000006, 0x080000, CRC(5e72ac90) SHA1(c28c2718e873be5a254992ef8db256a394ca03ff) )

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "b91-09.bin", 0x00000, 0x80000, CRC(5f247ca2) SHA1(3b89e5d035f27f62a14c5c7a976c804f9bb5c04d) ) /* STY spritemap */

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "b91-13.bin", 0x00000, 0x80000, CRC(8c7bf0f5) SHA1(6e18531991225c24a9722c9fbe1af6ae6e9b866b) )
	ROM_LOAD( "b91-12.bin", 0x80000, 0x80000, CRC(da77c7af) SHA1(49662a69b83739e2e0209cabff83995a951383f4) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "b91-14.bin", 0x00000, 0x80000, CRC(6bc314d3) SHA1(ae3e9c6b853bab4ec81a6bd951b39a4bc883f456) )

	ROM_REGION( 0x10000, "user2", 0 )   /* unused ROMs */
	ROM_LOAD( "b91-26.bin", 0x00000, 0x0400,  CRC(77682a4f) SHA1(da2b3143f1c8688a22d8ec47bbb73b2f2e578480) )
	ROM_LOAD( "b91-27.bin", 0x00000, 0x0400,  CRC(a3f8490d) SHA1(349b8c9ba914603f72f800a3fc8e8277d756deb1) )
	ROM_LOAD( "b91-28.bin", 0x00000, 0x0400,  CRC(fa2f840e) SHA1(dd61ee6833bd43bbf619d36ec46f2bfa00880f40) )
	ROM_LOAD( "b91-29.bin", 0x00000, 0x2000,  CRC(ad685be8) SHA1(e7681d76fa216c124c54544393c4f6a08fd7d74d) )
	ROM_LOAD( "b91-30.bin", 0x00000, 0x10000, CRC(30cc1f79) SHA1(3b0e3e6e8bce7a7d04a5b0103e2ce4e18e52a68e) )
	ROM_LOAD( "b91-31.bin", 0x00000, 0x10000, CRC(c189781c) SHA1(af3904ce51f715970965d110313491dbacf188b8) )
	ROM_LOAD( "b91-32.bin", 0x00000, 0x0100,  CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) )    // road A/B internal priority
	ROM_LOAD( "b91-33.bin", 0x00000, 0x0100,  CRC(89719d17) SHA1(50181b8172b0fc08b149db18caf10659be9c517f) )    // road/sprite priority and palette select
ROM_END

ROM_START( nightstru )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "b91-45.bin", 0x00000, 0x20000, CRC(7ad63421) SHA1(4ecfc3c8cd691d878e5d9212ccff0d225bb06bd9) )
	ROM_LOAD16_BYTE( "b91-44.bin", 0x00001, 0x20000, CRC(4bc30adf) SHA1(531d6ee9c8ff0d4ed07c15465ec7cb78cf976115) )
	ROM_LOAD16_BYTE( "b91-43.bin", 0x40000, 0x20000, CRC(3e6f727a) SHA1(ae837131a4c0c9bc5deba155c2a5b7ae72f1d070) )
	ROM_LOAD16_BYTE( "b91-46.bin", 0x40001, 0x20000, CRC(e870be95) SHA1(9a83df2c88a029bc40f5ce074143778ea555a2ba) )

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "b91-39.bin", 0x00000, 0x20000, CRC(725b23ae) SHA1(d4b4335863d32b9a81f7461240e960bf345c9835) )
	ROM_LOAD16_BYTE( "b91-40.bin", 0x00001, 0x20000, CRC(81fb364d) SHA1(f02733509039cde2c1de616e0a7969e31de1007a) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "b91-41.bin",   0x00000, 0x20000, CRC(2694bb42) SHA1(ee770472655ac0ef55eeff04037457dbf6744e4f) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b91-11.bin", 0x00000, 0x80000, CRC(fff8ce31) SHA1(fc729de92937a805d79379228d7a30041594c0df) )    /* SCR 8x8 */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "b91-04.bin", 0x000000, 0x080000, CRC(8ca1970d) SHA1(d8504298a38a95f1d8f3a2fba479ec75fe4d5de7) )   /* OBJ A 16x16 */
	ROM_LOAD64_WORD_SWAP( "b91-03.bin", 0x000002, 0x080000, CRC(cd5fed39) SHA1(c16c67cc998889288e6e96535fd8e61afc93bc78) )
	ROM_LOAD64_WORD_SWAP( "b91-02.bin", 0x000004, 0x080000, CRC(457c64b8) SHA1(443f13d56d53ca6a7750ec974da675bad3f34a38) )
	ROM_LOAD64_WORD_SWAP( "b91-01.bin", 0x000006, 0x080000, CRC(3731d94f) SHA1(2978d3eb1f44595681e84f3aa8dc03d34a191455) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "b91-10.bin", 0x00000, 0x80000, CRC(1d8f05b4) SHA1(04caa6a0887b90860c426a973dc3c3270e996818) )    /* ROD, road lines */

	ROM_REGION( 0x200000, "sprites2", 0 )
	ROM_LOAD64_WORD_SWAP( "b91-08.bin", 0x000000, 0x080000, CRC(66f35c34) SHA1(9040390fa9c626a54076a9461e0e198f059e2cb1) )   /* OBJ B 16x16 */
	ROM_LOAD64_WORD_SWAP( "b91-07.bin", 0x000002, 0x080000, CRC(4d8ec6cf) SHA1(2b7c10b459dc45313c4c90899a73c42c55b6c5c9) )
	ROM_LOAD64_WORD_SWAP( "b91-06.bin", 0x000004, 0x080000, CRC(a34dc839) SHA1(e1fcb763dbc562a62e862297458bde66d691606c) )
	ROM_LOAD64_WORD_SWAP( "b91-05.bin", 0x000006, 0x080000, CRC(5e72ac90) SHA1(c28c2718e873be5a254992ef8db256a394ca03ff) )

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "b91-09.bin", 0x00000, 0x80000, CRC(5f247ca2) SHA1(3b89e5d035f27f62a14c5c7a976c804f9bb5c04d) ) /* STY spritemap */

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "b91-13.bin", 0x00000, 0x80000, CRC(8c7bf0f5) SHA1(6e18531991225c24a9722c9fbe1af6ae6e9b866b) )
	ROM_LOAD( "b91-12.bin", 0x80000, 0x80000, CRC(da77c7af) SHA1(49662a69b83739e2e0209cabff83995a951383f4) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "b91-14.bin", 0x00000, 0x80000, CRC(6bc314d3) SHA1(ae3e9c6b853bab4ec81a6bd951b39a4bc883f456) )

	ROM_REGION( 0x10000, "user2", 0 )   /* unused ROMs */
	ROM_LOAD( "b91-26.bin", 0x00000, 0x0400,  CRC(77682a4f) SHA1(da2b3143f1c8688a22d8ec47bbb73b2f2e578480) )
	ROM_LOAD( "b91-27.bin", 0x00000, 0x0400,  CRC(a3f8490d) SHA1(349b8c9ba914603f72f800a3fc8e8277d756deb1) )
	ROM_LOAD( "b91-28.bin", 0x00000, 0x0400,  CRC(fa2f840e) SHA1(dd61ee6833bd43bbf619d36ec46f2bfa00880f40) )
	ROM_LOAD( "b91-29.bin", 0x00000, 0x2000,  CRC(ad685be8) SHA1(e7681d76fa216c124c54544393c4f6a08fd7d74d) )
	ROM_LOAD( "b91-30.bin", 0x00000, 0x10000, CRC(30cc1f79) SHA1(3b0e3e6e8bce7a7d04a5b0103e2ce4e18e52a68e) )
	ROM_LOAD( "b91-31.bin", 0x00000, 0x10000, CRC(c189781c) SHA1(af3904ce51f715970965d110313491dbacf188b8) )
	ROM_LOAD( "b91-32.bin", 0x00000, 0x0100,  CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) )    // road A/B internal priority
	ROM_LOAD( "b91-33.bin", 0x00000, 0x0100,  CRC(89719d17) SHA1(50181b8172b0fc08b149db18caf10659be9c517f) )    // road/sprite priority and palette select
ROM_END

ROM_START( nightstrj )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "b91-45.bin", 0x00000, 0x20000, CRC(7ad63421) SHA1(4ecfc3c8cd691d878e5d9212ccff0d225bb06bd9) )
	ROM_LOAD16_BYTE( "b91-44.bin", 0x00001, 0x20000, CRC(4bc30adf) SHA1(531d6ee9c8ff0d4ed07c15465ec7cb78cf976115) )
	ROM_LOAD16_BYTE( "b91-43.bin", 0x40000, 0x20000, CRC(3e6f727a) SHA1(ae837131a4c0c9bc5deba155c2a5b7ae72f1d070) )
	ROM_LOAD16_BYTE( "b91-42.bin", 0x40001, 0x20000, CRC(7179ef2f) SHA1(4c45f0c4dfcf16665d7eca4fdcd6a959d9b6fc01) )

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "b91-39.bin", 0x00000, 0x20000, CRC(725b23ae) SHA1(d4b4335863d32b9a81f7461240e960bf345c9835) )
	ROM_LOAD16_BYTE( "b91-40.bin", 0x00001, 0x20000, CRC(81fb364d) SHA1(f02733509039cde2c1de616e0a7969e31de1007a) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "b91-41.bin",   0x00000, 0x20000, CRC(2694bb42) SHA1(ee770472655ac0ef55eeff04037457dbf6744e4f) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b91-11.bin", 0x00000, 0x80000, CRC(fff8ce31) SHA1(fc729de92937a805d79379228d7a30041594c0df) )    /* SCR 8x8 */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "b91-04.bin", 0x000000, 0x080000, CRC(8ca1970d) SHA1(d8504298a38a95f1d8f3a2fba479ec75fe4d5de7) )   /* OBJ A 16x16 */
	ROM_LOAD64_WORD_SWAP( "b91-03.bin", 0x000002, 0x080000, CRC(cd5fed39) SHA1(c16c67cc998889288e6e96535fd8e61afc93bc78) )
	ROM_LOAD64_WORD_SWAP( "b91-02.bin", 0x000004, 0x080000, CRC(457c64b8) SHA1(443f13d56d53ca6a7750ec974da675bad3f34a38) )
	ROM_LOAD64_WORD_SWAP( "b91-01.bin", 0x000006, 0x080000, CRC(3731d94f) SHA1(2978d3eb1f44595681e84f3aa8dc03d34a191455) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "b91-10.bin", 0x00000, 0x80000, CRC(1d8f05b4) SHA1(04caa6a0887b90860c426a973dc3c3270e996818) )    /* ROD, road lines */

	ROM_REGION( 0x200000, "sprites2", 0 )
	ROM_LOAD64_WORD_SWAP( "b91-08.bin", 0x000000, 0x080000, CRC(66f35c34) SHA1(9040390fa9c626a54076a9461e0e198f059e2cb1) )   /* OBJ B 16x16 */
	ROM_LOAD64_WORD_SWAP( "b91-07.bin", 0x000002, 0x080000, CRC(4d8ec6cf) SHA1(2b7c10b459dc45313c4c90899a73c42c55b6c5c9) )
	ROM_LOAD64_WORD_SWAP( "b91-06.bin", 0x000004, 0x080000, CRC(a34dc839) SHA1(e1fcb763dbc562a62e862297458bde66d691606c) )
	ROM_LOAD64_WORD_SWAP( "b91-05.bin", 0x000006, 0x080000, CRC(5e72ac90) SHA1(c28c2718e873be5a254992ef8db256a394ca03ff) )

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "b91-09.bin", 0x00000, 0x80000, CRC(5f247ca2) SHA1(3b89e5d035f27f62a14c5c7a976c804f9bb5c04d) ) /* STY spritemap */

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "b91-13.bin", 0x00000, 0x80000, CRC(8c7bf0f5) SHA1(6e18531991225c24a9722c9fbe1af6ae6e9b866b) )
	ROM_LOAD( "b91-12.bin", 0x80000, 0x80000, CRC(da77c7af) SHA1(49662a69b83739e2e0209cabff83995a951383f4) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "b91-14.bin", 0x00000, 0x80000, CRC(6bc314d3) SHA1(ae3e9c6b853bab4ec81a6bd951b39a4bc883f456) )

	ROM_REGION( 0x10000, "user2", 0 )   /* unused ROMs */
	ROM_LOAD( "b91-26.bin", 0x00000, 0x0400,  CRC(77682a4f) SHA1(da2b3143f1c8688a22d8ec47bbb73b2f2e578480) )
	ROM_LOAD( "b91-27.bin", 0x00000, 0x0400,  CRC(a3f8490d) SHA1(349b8c9ba914603f72f800a3fc8e8277d756deb1) )
	ROM_LOAD( "b91-28.bin", 0x00000, 0x0400,  CRC(fa2f840e) SHA1(dd61ee6833bd43bbf619d36ec46f2bfa00880f40) )
	ROM_LOAD( "b91-29.bin", 0x00000, 0x2000,  CRC(ad685be8) SHA1(e7681d76fa216c124c54544393c4f6a08fd7d74d) )
	ROM_LOAD( "b91-30.bin", 0x00000, 0x10000, CRC(30cc1f79) SHA1(3b0e3e6e8bce7a7d04a5b0103e2ce4e18e52a68e) )
	ROM_LOAD( "b91-31.bin", 0x00000, 0x10000, CRC(c189781c) SHA1(af3904ce51f715970965d110313491dbacf188b8) )
	ROM_LOAD( "b91-32.bin", 0x00000, 0x0100,  CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) )    // road A/B internal priority
	ROM_LOAD( "b91-33.bin", 0x00000, 0x0100,  CRC(89719d17) SHA1(50181b8172b0fc08b149db18caf10659be9c517f) )    // road/sprite priority and palette select
ROM_END

ROM_START( aquajack )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 256K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "b77-22.ic31", 0x00000, 0x20000, CRC(67400dde) SHA1(1e47c4fbd4449f2d973ac962ad58f22502d59198) )
	ROM_LOAD16_BYTE( "b77-26.ic17", 0x00001, 0x20000, CRC(cd4d0969) SHA1(d610e7847a09f1ca892007440fa1b431bb0c41d2) )

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "b77-24.ic69", 0x00000, 0x20000, CRC(95e643ed) SHA1(d47ddd50c744f33b3cbd5ef90880ca577977f5ca) )
	ROM_LOAD16_BYTE( "b77-23.ic67", 0x00001, 0x20000, CRC(395a7d1c) SHA1(22cbbabb07f43e72a6139b6b9d68d6c1146d727f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "b77-20.ic54",  0x00000, 0x10000, CRC(84ba54b7) SHA1(84e51c1a6a5b4eb2a65f4a6d9d54037323348f50) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b77-05.ic105", 0x00000, 0x80000, CRC(7238f0ff) SHA1(95e2d6815e99392358bbeabf1afbf237673f2e24) )  /* SCR 8x8 */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "b77-04.ic16", 0x000000, 0x80000, CRC(bed0be6c) SHA1(2b11824f741b7f6755bd78f594af19b63a29092f) )   /* OBJ 16x8 */
	ROM_LOAD64_WORD_SWAP( "b77-03.ic15", 0x000002, 0x80000, CRC(9a3030a7) SHA1(7b60fd066eccd04d9fcc131d9d06f151334ccab2) )
	ROM_LOAD64_WORD_SWAP( "b77-02.ic14", 0x000004, 0x80000, CRC(daea0d2e) SHA1(10640651824234a589838e8f017964b79de79cb4) )
	ROM_LOAD64_WORD_SWAP( "b77-01.ic13", 0x000006, 0x80000, CRC(cdab000d) SHA1(d83ee7f1dc17ab113bac38d0d062bb1519ff69f7) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "b77-07.ic33", 0x000000, 0x80000, CRC(7db1fc5e) SHA1(fbc88c2179b881d34d3a33d0a901d8da3445f9a8) )  /* ROD, road lines */

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "b77-06.ic39", 0x00000, 0x80000, CRC(ce2aed00) SHA1(9c992717914b13eb271122ecf7cca3634b013e56) )    /* STY spritemap */

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "b77-09.ic58", 0x00000, 0x80000, CRC(948e5ad9) SHA1(35cd6706470f01b5a244817d10fc65c075ff29b1) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "b77-08.ic57", 0x00000, 0x80000, CRC(119b9485) SHA1(2c9cd90be20df769e09016abccf59c8f119da286) )

	ROM_REGION( 0x00200, "user2", 0 )   /* unused PROMs */
	ROM_LOAD( "b77-17.ic1",  0x00000, 0x0100,  CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) )   // road A/B internal priority
	ROM_LOAD( "b77-18.ic37", 0x00100, 0x0100,  CRC(7b7d8ff4) SHA1(18842ed8160739cd2e2ccc2db605153dbed6cc0a) )   // road/sprite priority and palette select

/*  (no unused roms in my set, there should be an 0x10000 one like the rest) */
ROM_END

ROM_START( aquajacku )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 256K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "b77-22.ic31", 0x00000, 0x20000, CRC(67400dde) SHA1(1e47c4fbd4449f2d973ac962ad58f22502d59198) )
	ROM_LOAD16_BYTE( "b77-25.ic17", 0x00001, 0x20000, CRC(ba4a39ff) SHA1(89527c7e3106ae77c85372117fea24a8553ab377) )

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "b77-24.ic69", 0x00000, 0x20000, CRC(95e643ed) SHA1(d47ddd50c744f33b3cbd5ef90880ca577977f5ca) )
	ROM_LOAD16_BYTE( "b77-23.ic67", 0x00001, 0x20000, CRC(395a7d1c) SHA1(22cbbabb07f43e72a6139b6b9d68d6c1146d727f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "b77-20.ic54",  0x00000, 0x10000, CRC(84ba54b7) SHA1(84e51c1a6a5b4eb2a65f4a6d9d54037323348f50) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b77-05.ic105", 0x00000, 0x80000, CRC(7238f0ff) SHA1(95e2d6815e99392358bbeabf1afbf237673f2e24) )  /* SCR 8x8 */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "b77-04.ic16", 0x000000, 0x80000, CRC(bed0be6c) SHA1(2b11824f741b7f6755bd78f594af19b63a29092f) )   /* OBJ 16x8 */
	ROM_LOAD64_WORD_SWAP( "b77-03.ic15", 0x000002, 0x80000, CRC(9a3030a7) SHA1(7b60fd066eccd04d9fcc131d9d06f151334ccab2) )
	ROM_LOAD64_WORD_SWAP( "b77-02.ic14", 0x000004, 0x80000, CRC(daea0d2e) SHA1(10640651824234a589838e8f017964b79de79cb4) )
	ROM_LOAD64_WORD_SWAP( "b77-01.ic13", 0x000006, 0x80000, CRC(cdab000d) SHA1(d83ee7f1dc17ab113bac38d0d062bb1519ff69f7) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "b77-07.ic33", 0x000000, 0x80000, CRC(7db1fc5e) SHA1(fbc88c2179b881d34d3a33d0a901d8da3445f9a8) )  /* ROD, road lines */

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "b77-06.ic39", 0x00000, 0x80000, CRC(ce2aed00) SHA1(9c992717914b13eb271122ecf7cca3634b013e56) )    /* STY spritemap */

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "b77-09.ic58", 0x00000, 0x80000, CRC(948e5ad9) SHA1(35cd6706470f01b5a244817d10fc65c075ff29b1) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "b77-08.ic57", 0x00000, 0x80000, CRC(119b9485) SHA1(2c9cd90be20df769e09016abccf59c8f119da286) )

	ROM_REGION( 0x00200, "user2", 0 )   /* unused PROMs */
	ROM_LOAD( "b77-17.ic1",  0x00000, 0x0100,  CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) )   // road A/B internal priority
	ROM_LOAD( "b77-18.ic37", 0x00100, 0x0100,  CRC(7b7d8ff4) SHA1(18842ed8160739cd2e2ccc2db605153dbed6cc0a) )   // road/sprite priority and palette select

/*  (no unused roms in my set, there should be an 0x10000 one like the rest) */
ROM_END

ROM_START( aquajackj )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 256K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "b77-22.ic31", 0x00000, 0x20000, CRC(67400dde) SHA1(1e47c4fbd4449f2d973ac962ad58f22502d59198) )
	ROM_LOAD16_BYTE( "b77-21.ic17", 0x00001, 0x20000, CRC(23436845) SHA1(e62111c902453e1b655c7f25bcea938a6f13aed2) )

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "b77-24.ic69", 0x00000, 0x20000, CRC(95e643ed) SHA1(d47ddd50c744f33b3cbd5ef90880ca577977f5ca) )
	ROM_LOAD16_BYTE( "b77-23.ic67", 0x00001, 0x20000, CRC(395a7d1c) SHA1(22cbbabb07f43e72a6139b6b9d68d6c1146d727f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "b77-20.ic54",  0x00000, 0x10000, CRC(84ba54b7) SHA1(84e51c1a6a5b4eb2a65f4a6d9d54037323348f50) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b77-05.ic105", 0x00000, 0x80000, CRC(7238f0ff) SHA1(95e2d6815e99392358bbeabf1afbf237673f2e24) )  /* SCR 8x8 */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "b77-04.ic16", 0x000000, 0x80000, CRC(bed0be6c) SHA1(2b11824f741b7f6755bd78f594af19b63a29092f) )   /* OBJ 16x8 */
	ROM_LOAD64_WORD_SWAP( "b77-03.ic15", 0x000002, 0x80000, CRC(9a3030a7) SHA1(7b60fd066eccd04d9fcc131d9d06f151334ccab2) )
	ROM_LOAD64_WORD_SWAP( "b77-02.ic14", 0x000004, 0x80000, CRC(daea0d2e) SHA1(10640651824234a589838e8f017964b79de79cb4) )
	ROM_LOAD64_WORD_SWAP( "b77-01.ic13", 0x000006, 0x80000, CRC(cdab000d) SHA1(d83ee7f1dc17ab113bac38d0d062bb1519ff69f7) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "b77-07.ic33", 0x000000, 0x80000, CRC(7db1fc5e) SHA1(fbc88c2179b881d34d3a33d0a901d8da3445f9a8) )  /* ROD, road lines */

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "b77-06.ic39", 0x00000, 0x80000, CRC(ce2aed00) SHA1(9c992717914b13eb271122ecf7cca3634b013e56) )    /* STY spritemap */

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "b77-09.ic58", 0x00000, 0x80000, CRC(948e5ad9) SHA1(35cd6706470f01b5a244817d10fc65c075ff29b1) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "b77-08.ic57", 0x00000, 0x80000, CRC(119b9485) SHA1(2c9cd90be20df769e09016abccf59c8f119da286) )

	ROM_REGION( 0x00200, "user2", 0 )   /* unused PROMs */
	ROM_LOAD( "b77-17.ic1",  0x00000, 0x0100,  CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) )   // road A/B internal priority
	ROM_LOAD( "b77-18.ic37", 0x00100, 0x0100,  CRC(7b7d8ff4) SHA1(18842ed8160739cd2e2ccc2db605153dbed6cc0a) )   // road/sprite priority and palette select

/*  (no unused roms in my set, there should be an 0x10000 one like the rest) */
ROM_END

ROM_START( spacegun )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "c57-18.62", 0x00000, 0x20000, CRC(19d7d52e) SHA1(4361929a43f911864ece4dcd06995ea6b6156c59) )
	ROM_LOAD16_BYTE( "c57-20.74", 0x00001, 0x20000, CRC(2e58253f) SHA1(36fb52ce1c6cf9f537cf500ba330b167871969b9) )
	ROM_LOAD16_BYTE( "c57-17.59", 0x40000, 0x20000, CRC(e197edb8) SHA1(2ffd000aac1825ecd564c273f0cc055710ba4050) )
	ROM_LOAD16_BYTE( "c57-22.73", 0x40001, 0x20000, CRC(5855fde3) SHA1(fcd6d7ed16b61b9023596f0efb7f6971060a2e0b) )

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "c57-15+.27", 0x00000, 0x20000, CRC(b36eb8f1) SHA1(e6e9fb844fd9acc6ee8a515a964d5df8de088a8c) ) /* Actual label is "C57 15*" */
	ROM_LOAD16_BYTE( "c57-16+.29", 0x00001, 0x20000, CRC(bfb5d1e7) SHA1(cbf22e9043aac54e08c5da74d973da27844170ef) ) /* Actual label is "C57 16*" */

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c57-06.52", 0x00000, 0x80000, CRC(4ebadd5b) SHA1(d32a52b4d7dd19b0fa2551f93ce3d5cbcf2bc158) )     /* SCR 8x8 */

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "c57-01.25", 0x000000, 0x100000, CRC(f901b04e) SHA1(24bac1c3a0c585966a7cbeeebd9b2dd3acf45a67) )    /* OBJ 16x8 */
	ROM_LOAD64_WORD_SWAP( "c57-02.24", 0x000002, 0x100000, CRC(21ee4633) SHA1(ddb948b165127c8fb1a988b5a0f17f92117f1b66) )
	ROM_LOAD64_WORD_SWAP( "c57-03.12", 0x000004, 0x100000, CRC(fafca86f) SHA1(dc6ea78f0deafef632d8bd3677ec74e797dc69a2) )
	ROM_LOAD64_WORD_SWAP( "c57-04.11", 0x000006, 0x100000, CRC(a9787090) SHA1(8c05c4c0d14a9f60defb37225da37aadf946c563) )

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "c57-05.36", 0x00000, 0x80000, CRC(6a70eb2e) SHA1(307dd876af65204e86e094b4015ffb4a655824f8) )  /* STY spritemap */

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c57-07.76", 0x00000, 0x80000, CRC(ad653dc1) SHA1(2ec440f793b0a686233fbe61c9462f8365c42b65) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c57-08.75", 0x00000, 0x80000, CRC(22593550) SHA1(e802e947e6947d146e1b57dbff7ac021e19e7b2b) )

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "pal16l8-c57-09.9",  0x0000, 0x0104, CRC(ea93161e) SHA1(c83c6ce3696b2754abb861d41a8d587a8e82aa1e) )
	ROM_LOAD( "pal20l8-c57-10.47", 0x0200, 0x0144, CRC(3ee56888) SHA1(030efb0903d919686748b1ff86a327990832d0fa) )
	ROM_LOAD( "pal16l8-c57-11.48", 0x0400, 0x0104, CRC(6bb4372e) SHA1(513bf6f032a9043303b8660c106753ae214d28ff) )
	ROM_LOAD( "pal20l8-c57-12.61", 0x0600, 0x0144, CRC(debddb13) SHA1(47be25b3bb157d37b9813737544a56a2090f85ba) )
	ROM_LOAD( "pal16l8-c57-13.72", 0x0800, 0x0104, CRC(1369f23e) SHA1(bbc960cfc3edd07e89134e1b876aa7a6c0cba5ac) )
	ROM_LOAD( "pal16r4-c57-14.96", 0x0a00, 0x0104, CRC(75e1bf61) SHA1(e8358329a78ec0ab87641b2ecaec0b2b67c6ca30) )
ROM_END

ROM_START( spacegunu )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "c57-18.62", 0x00000, 0x20000, CRC(19d7d52e) SHA1(4361929a43f911864ece4dcd06995ea6b6156c59) )
	ROM_LOAD16_BYTE( "c57-20.74", 0x00001, 0x20000, CRC(2e58253f) SHA1(36fb52ce1c6cf9f537cf500ba330b167871969b9) )
	ROM_LOAD16_BYTE( "c57-17.59", 0x40000, 0x20000, CRC(e197edb8) SHA1(2ffd000aac1825ecd564c273f0cc055710ba4050) )
	ROM_LOAD16_BYTE( "c57-21.73", 0x40001, 0x20000, CRC(2f52cd75) SHA1(7dfd1f57925a0993608055247d565af810753189) )

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "c57-15+.27", 0x00000, 0x20000, CRC(b36eb8f1) SHA1(e6e9fb844fd9acc6ee8a515a964d5df8de088a8c) ) /* Actual label is "C57 15*" */
	ROM_LOAD16_BYTE( "c57-16+.29", 0x00001, 0x20000, CRC(bfb5d1e7) SHA1(cbf22e9043aac54e08c5da74d973da27844170ef) ) /* Actual label is "C57 16*" */

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c57-06.52", 0x00000, 0x80000, CRC(4ebadd5b) SHA1(d32a52b4d7dd19b0fa2551f93ce3d5cbcf2bc158) )     /* SCR 8x8 */

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "c57-01.25", 0x000000, 0x100000, CRC(f901b04e) SHA1(24bac1c3a0c585966a7cbeeebd9b2dd3acf45a67) )    /* OBJ 16x8 */
	ROM_LOAD64_WORD_SWAP( "c57-02.24", 0x000002, 0x100000, CRC(21ee4633) SHA1(ddb948b165127c8fb1a988b5a0f17f92117f1b66) )
	ROM_LOAD64_WORD_SWAP( "c57-03.12", 0x000004, 0x100000, CRC(fafca86f) SHA1(dc6ea78f0deafef632d8bd3677ec74e797dc69a2) )
	ROM_LOAD64_WORD_SWAP( "c57-04.11", 0x000006, 0x100000, CRC(a9787090) SHA1(8c05c4c0d14a9f60defb37225da37aadf946c563) )

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "c57-05.36", 0x00000, 0x80000, CRC(6a70eb2e) SHA1(307dd876af65204e86e094b4015ffb4a655824f8) )  /* STY spritemap */

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c57-07.76", 0x00000, 0x80000, CRC(ad653dc1) SHA1(2ec440f793b0a686233fbe61c9462f8365c42b65) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c57-08.75", 0x00000, 0x80000, CRC(22593550) SHA1(e802e947e6947d146e1b57dbff7ac021e19e7b2b) )

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "pal16l8-c57-09.9",  0x0000, 0x0104, CRC(ea93161e) SHA1(c83c6ce3696b2754abb861d41a8d587a8e82aa1e) )
	ROM_LOAD( "pal20l8-c57-10.47", 0x0200, 0x0144, CRC(3ee56888) SHA1(030efb0903d919686748b1ff86a327990832d0fa) )
	ROM_LOAD( "pal16l8-c57-11.48", 0x0400, 0x0104, CRC(6bb4372e) SHA1(513bf6f032a9043303b8660c106753ae214d28ff) )
	ROM_LOAD( "pal20l8-c57-12.61", 0x0600, 0x0144, CRC(debddb13) SHA1(47be25b3bb157d37b9813737544a56a2090f85ba) )
	ROM_LOAD( "pal16l8-c57-13.72", 0x0800, 0x0104, CRC(1369f23e) SHA1(bbc960cfc3edd07e89134e1b876aa7a6c0cba5ac) )
	ROM_LOAD( "pal16r4-c57-14.96", 0x0a00, 0x0104, CRC(75e1bf61) SHA1(e8358329a78ec0ab87641b2ecaec0b2b67c6ca30) )
ROM_END

ROM_START( spacegunj )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "c57-18+.62", 0x00000, 0x20000, CRC(c648c093) SHA1(136baf474036a64c6c19e1c3ced36f796ca47f0e) ) /* Actual label is "C57 18*" */
	ROM_LOAD16_BYTE( "c57-20+.74", 0x00001, 0x20000, CRC(4de524f6) SHA1(ae4557cb17ad434939174a8092f117da90178320) ) /* Actual label is "C57 20*" */
	ROM_LOAD16_BYTE( "c57-17.59",  0x40000, 0x20000, CRC(e197edb8) SHA1(2ffd000aac1825ecd564c273f0cc055710ba4050) )
	ROM_LOAD16_BYTE( "c57-19.73",  0x40001, 0x20000, CRC(c15cac59) SHA1(62d7ce8f15032d215c6ca3d65605195958758ea6) )

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "c57-15+.27", 0x00000, 0x20000, CRC(b36eb8f1) SHA1(e6e9fb844fd9acc6ee8a515a964d5df8de088a8c) ) /* Actual label is "C57 15*" */
	ROM_LOAD16_BYTE( "c57-16+.29", 0x00001, 0x20000, CRC(bfb5d1e7) SHA1(cbf22e9043aac54e08c5da74d973da27844170ef) ) /* Actual label is "C57 16*" */

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c57-06.52", 0x00000, 0x80000, CRC(4ebadd5b) SHA1(d32a52b4d7dd19b0fa2551f93ce3d5cbcf2bc158) )     /* SCR 8x8 */

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "c57-01.25", 0x000000, 0x100000, CRC(f901b04e) SHA1(24bac1c3a0c585966a7cbeeebd9b2dd3acf45a67) )    /* OBJ 16x8 */
	ROM_LOAD64_WORD_SWAP( "c57-02.24", 0x000002, 0x100000, CRC(21ee4633) SHA1(ddb948b165127c8fb1a988b5a0f17f92117f1b66) )
	ROM_LOAD64_WORD_SWAP( "c57-03.12", 0x000004, 0x100000, CRC(fafca86f) SHA1(dc6ea78f0deafef632d8bd3677ec74e797dc69a2) )
	ROM_LOAD64_WORD_SWAP( "c57-04.11", 0x000006, 0x100000, CRC(a9787090) SHA1(8c05c4c0d14a9f60defb37225da37aadf946c563) )

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "c57-05.36", 0x00000, 0x80000, CRC(6a70eb2e) SHA1(307dd876af65204e86e094b4015ffb4a655824f8) )  /* STY spritemap */

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c57-07.76", 0x00000, 0x80000, CRC(ad653dc1) SHA1(2ec440f793b0a686233fbe61c9462f8365c42b65) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c57-08.75", 0x00000, 0x80000, CRC(22593550) SHA1(e802e947e6947d146e1b57dbff7ac021e19e7b2b) )

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "pal16l8-c57-09.9",  0x0000, 0x0104, CRC(ea93161e) SHA1(c83c6ce3696b2754abb861d41a8d587a8e82aa1e) )
	ROM_LOAD( "pal20l8-c57-10.47", 0x0200, 0x0144, CRC(3ee56888) SHA1(030efb0903d919686748b1ff86a327990832d0fa) )
	ROM_LOAD( "pal16l8-c57-11.48", 0x0400, 0x0104, CRC(6bb4372e) SHA1(513bf6f032a9043303b8660c106753ae214d28ff) )
	ROM_LOAD( "pal20l8-c57-12.61", 0x0600, 0x0144, CRC(debddb13) SHA1(47be25b3bb157d37b9813737544a56a2090f85ba) )
	ROM_LOAD( "pal16l8-c57-13.72", 0x0800, 0x0104, CRC(1369f23e) SHA1(bbc960cfc3edd07e89134e1b876aa7a6c0cba5ac) )
	ROM_LOAD( "pal16r4-c57-14.96", 0x0a00, 0x0104, CRC(75e1bf61) SHA1(e8358329a78ec0ab87641b2ecaec0b2b67c6ca30) )
ROM_END

ROM_START( dblaxle ) /* Manual refers to this version as the "Version Without Communication" */
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "c78_49-1.2",  0x00000, 0x20000, CRC(a6f0c631) SHA1(371cb7807d2350ceca36fc0fb6a65d3179c011b0) )
	ROM_LOAD16_BYTE( "c78_51-1.4",  0x00001, 0x20000, CRC(ef24e83b) SHA1(a0bc1d2192bccfcb6f859aa0a27f43cc92080e1e) )
	ROM_LOAD16_BYTE( "c78_50-1.3",  0x40000, 0x20000, CRC(8b0440f4) SHA1(31f7fcb8acfac13bbf2036670b665744acd37d25) )
	ROM_LOAD16_BYTE( "c78_53-1.5",  0x40001, 0x20000, CRC(2bb91763) SHA1(a7cd2ac9f3937d88194d7c994d76abc89cc30f4d) )

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "c78-30-1.35", 0x00000, 0x20000, CRC(026aac18) SHA1(f50873982b4dc0fc822060f4c20c635efdd75d7e) )
	ROM_LOAD16_BYTE( "c78-31-1.36", 0x00001, 0x20000, CRC(67ce23e8) SHA1(983e998a79e3d4376b005c92ded050be236d37cc) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c78-34.c42",         0x00000, 0x20000, CRC(f2186943) SHA1(2e9aed39fddf3aa1db7e20f8a709b6b82cc3e7df) )

	ROM_REGION( 0x100000, "tc0480scp", 0 )
	ROM_LOAD32_WORD( "c78-10.12", 0x00000, 0x80000, CRC(44b1897c) SHA1(7ad179db6d7dfeb139ea13cb4a231f99d177f2b1) )  /* SCR 16x16 */
	ROM_LOAD32_WORD( "c78-11.11", 0x00002, 0x80000, CRC(7db3d4a3) SHA1(fc3c44ed36b212688a5bd8dc61321a994578258e) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "c78-08.25", 0x000000, 0x100000, CRC(6c725211) SHA1(3c1765f44fe57b496d305e994516674f71bd4c3c) )    /* OBJ 16x8 */
	ROM_LOAD64_WORD_SWAP( "c78-07.33", 0x000002, 0x100000, CRC(9da00d5b) SHA1(f6b664c7495b936ce1b99852da45ec92cb37062a) )
	ROM_LOAD64_WORD_SWAP( "c78-06.23", 0x000004, 0x100000, CRC(8309e91b) SHA1(3f27557bc82bf42cc77e3c7e363b51a0b119144d) )
	ROM_LOAD64_WORD_SWAP( "c78-05.31", 0x000006, 0x100000, CRC(90001f68) SHA1(5c08dfe6a2e12e6ca84035815563f38fc2c2c029) )
//  ROMX_LOAD      ( "c78-05l.1", 0x000007, 0x080000, CRC(f24bf972) , ROM_SKIP(7) )
//  ROMX_LOAD      ( "c78-05h.2", 0x000006, 0x080000, CRC(c01039b5) , ROM_SKIP(7) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "c78-09.12", 0x000000, 0x80000, CRC(0dbde6f5) SHA1(4049271e3738b54e0c56d191889b1aea5664d49f) )    /* ROD, road lines */

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "c78-04.3", 0x00000, 0x80000, CRC(cc1aa37c) SHA1(cfa2eb338dc81c98c637c2f0b14d2baea8b115f5) )   /* STY spritemap */

	ROM_REGION( 0x180000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c78-12.33", 0x000000, 0x100000, CRC(b0267404) SHA1(ffd337336ff9b096e3725f733364762f6e6d3fab) )
	ROM_LOAD( "c78-13.46", 0x100000, 0x080000, CRC(1b363aa2) SHA1(0aae3988024654e98cc0c784307b1c329c8f0783) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c78-14.31",  0x00000, 0x80000, CRC(9cad4dfb) SHA1(9187ef827a3f1bc9233d0e45e72c72c0956c5912) )

	ROM_REGION( 0x10000, "user2", 0 )   /* unused ROMs */
	ROM_LOAD( "c78-25.15",  0x00000, 0x10000, CRC(7245a6f6) SHA1(5bdde4e3bcde8c59dc84478c3cc079d7ef8ee9c5) )    // 98% compression
	ROM_LOAD( "c78-15.22",  0x00000, 0x00100, CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) )    // road A/B internal priority
	ROM_LOAD( "c78-21.74",  0x00000, 0x00100, CRC(2926bf27) SHA1(bfbbe6c71bb29a05959f3de0d940816139f9ebfe) )    // road/sprite priority and palette select
	ROM_LOAD( "c84-10.16",  0x00000, 0x00400, CRC(643e8bfc) SHA1(a6e6086fb8fbd102e01ec72fe60a4232f5909565) )
	ROM_LOAD( "c84-11.17",  0x00000, 0x00400, CRC(10728853) SHA1(45d7cc8e06fbe01295cc2194bca9586f0ef8b12b) )
ROM_END

ROM_START( dblaxleu ) /* Manual refers to this version as the "Version Without Communication" */
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "c78_49+.2",  0x00000, 0x20000, CRC(3bb0344a) SHA1(dad031033838bf65c83fb3715a9727f2d165909b) ) /* Actual label is C78 49* */
	ROM_LOAD16_BYTE( "c78_51+.4",  0x00001, 0x20000, CRC(918176cb) SHA1(b78bce351e240b447fbb9a1f44c8efa3e6b98cbe) ) /* Actual label is C78 51* */
	ROM_LOAD16_BYTE( "c78_50+.3",  0x40000, 0x20000, CRC(5a12e2bb) SHA1(53a91cc8fcf42934aa282f5a1bb286461dc2a421) ) /* Actual label is C78 50* */
	ROM_LOAD16_BYTE( "c78_53+.5",  0x40001, 0x20000, CRC(62f910d4) SHA1(3d952ffcb30a264751b4b282ae8c26ecea09c05c) ) /* Actual label is C78 53* */

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "c78-30+.35", 0x00000, 0x20000, CRC(f73b3ce1) SHA1(1794d1c74599d58302c6dbfabcc1b3110d19b1fb) ) /* Label missing, it's either C78 30 or C78 30* */
	ROM_LOAD16_BYTE( "c78-31+.36", 0x00001, 0x20000, CRC(4639adee) SHA1(24569dd4801c622758e60d3e526480ac6b5f85d2) ) /* Label missing, it's either C78 31 or C78 31* */

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c78-34.c42",        0x00000, 0x20000, CRC(f2186943) SHA1(2e9aed39fddf3aa1db7e20f8a709b6b82cc3e7df) )

	ROM_REGION( 0x100000, "tc0480scp", 0 )
	ROM_LOAD32_WORD( "c78-10.12", 0x00000, 0x80000, CRC(44b1897c) SHA1(7ad179db6d7dfeb139ea13cb4a231f99d177f2b1) )  /* SCR 16x16 */
	ROM_LOAD32_WORD( "c78-11.11", 0x00002, 0x80000, CRC(7db3d4a3) SHA1(fc3c44ed36b212688a5bd8dc61321a994578258e) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "c78-08.25", 0x000000, 0x100000, CRC(6c725211) SHA1(3c1765f44fe57b496d305e994516674f71bd4c3c) )    /* OBJ 16x8 */
	ROM_LOAD64_WORD_SWAP( "c78-07.33", 0x000002, 0x100000, CRC(9da00d5b) SHA1(f6b664c7495b936ce1b99852da45ec92cb37062a) )
	ROM_LOAD64_WORD_SWAP( "c78-06.23", 0x000004, 0x100000, CRC(8309e91b) SHA1(3f27557bc82bf42cc77e3c7e363b51a0b119144d) )
	ROM_LOAD64_WORD_SWAP( "c78-05.31", 0x000006, 0x100000, CRC(90001f68) SHA1(5c08dfe6a2e12e6ca84035815563f38fc2c2c029) )
//  ROMX_LOAD      ( "c78-05l.1", 0x000007, 0x080000, CRC(f24bf972) , ROM_SKIP(7) )
//  ROMX_LOAD      ( "c78-05h.2", 0x000006, 0x080000, CRC(c01039b5) , ROM_SKIP(7) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "c78-09.12", 0x000000, 0x80000, CRC(0dbde6f5) SHA1(4049271e3738b54e0c56d191889b1aea5664d49f) )    /* ROD, road lines */

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "c78-04.3", 0x00000, 0x80000, CRC(cc1aa37c) SHA1(cfa2eb338dc81c98c637c2f0b14d2baea8b115f5) )   /* STY spritemap */

	ROM_REGION( 0x180000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c78-12.33", 0x000000, 0x100000, CRC(b0267404) SHA1(ffd337336ff9b096e3725f733364762f6e6d3fab) )
	ROM_LOAD( "c78-13.46", 0x100000, 0x080000, CRC(1b363aa2) SHA1(0aae3988024654e98cc0c784307b1c329c8f0783) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c78-14.31",  0x00000, 0x80000, CRC(9cad4dfb) SHA1(9187ef827a3f1bc9233d0e45e72c72c0956c5912) )

	ROM_REGION( 0x10000, "user2", 0 )   /* unused ROMs */
	ROM_LOAD( "c78-25.15",  0x00000, 0x10000, CRC(7245a6f6) SHA1(5bdde4e3bcde8c59dc84478c3cc079d7ef8ee9c5) )    // 98% compression
	ROM_LOAD( "c78-15.22",  0x00000, 0x00100, CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) )    // road A/B internal priority
	ROM_LOAD( "c78-21.74",  0x00000, 0x00100, CRC(2926bf27) SHA1(bfbbe6c71bb29a05959f3de0d940816139f9ebfe) )    // road/sprite priority and palette select
	ROM_LOAD( "c84-10.16",  0x00000, 0x00400, CRC(643e8bfc) SHA1(a6e6086fb8fbd102e01ec72fe60a4232f5909565) )
	ROM_LOAD( "c84-11.17",  0x00000, 0x00400, CRC(10728853) SHA1(45d7cc8e06fbe01295cc2194bca9586f0ef8b12b) )
ROM_END

ROM_START( dblaxleul ) /* Side by side linkable version */
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "c78_41-1.2",  0x00000, 0x20000, CRC(cf297fe4) SHA1(4875de63e8336062c27d83b55938bcb3d08a24a3) )
	ROM_LOAD16_BYTE( "c78_43-1.4",  0x00001, 0x20000, CRC(38a8bad6) SHA1(50977a6a364893549d2f7899bbc4e0c67086697e) )
	ROM_LOAD16_BYTE( "c78_42-1.3",  0x40000, 0x20000, CRC(4124ab2b) SHA1(96c3b6e01a1823259b3d7ca43e0a8631bfe33d0e) )
	ROM_LOAD16_BYTE( "c78_44-1.5",  0x40001, 0x20000, CRC(50a55b6e) SHA1(62a72d33030d50c157a5cf05f6bdc1b02c9b9ff1) )

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "c78-30-1.35", 0x00000, 0x20000, CRC(026aac18) SHA1(f50873982b4dc0fc822060f4c20c635efdd75d7e) )
	ROM_LOAD16_BYTE( "c78-31-1.36", 0x00001, 0x20000, CRC(67ce23e8) SHA1(983e998a79e3d4376b005c92ded050be236d37cc) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c78-34.c42",         0x00000, 0x20000, CRC(f2186943) SHA1(2e9aed39fddf3aa1db7e20f8a709b6b82cc3e7df) )

	ROM_REGION( 0x100000, "tc0480scp", 0 )
	ROM_LOAD32_WORD( "c78-10.12", 0x00000, 0x80000, CRC(44b1897c) SHA1(7ad179db6d7dfeb139ea13cb4a231f99d177f2b1) )  /* SCR 16x16 */
	ROM_LOAD32_WORD( "c78-11.11", 0x00002, 0x80000, CRC(7db3d4a3) SHA1(fc3c44ed36b212688a5bd8dc61321a994578258e) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "c78-08.25", 0x000000, 0x100000, CRC(6c725211) SHA1(3c1765f44fe57b496d305e994516674f71bd4c3c) )    /* OBJ 16x8 */
	ROM_LOAD64_WORD_SWAP( "c78-07.33", 0x000002, 0x100000, CRC(9da00d5b) SHA1(f6b664c7495b936ce1b99852da45ec92cb37062a) )
	ROM_LOAD64_WORD_SWAP( "c78-06.23", 0x000004, 0x100000, CRC(8309e91b) SHA1(3f27557bc82bf42cc77e3c7e363b51a0b119144d) )
	ROM_LOAD64_WORD_SWAP( "c78-05.31", 0x000006, 0x100000, CRC(90001f68) SHA1(5c08dfe6a2e12e6ca84035815563f38fc2c2c029) )
//  ROMX_LOAD      ( "c78-05l.1", 0x000007, 0x080000, CRC(f24bf972) , ROM_SKIP(7) )
//  ROMX_LOAD      ( "c78-05h.2", 0x000006, 0x080000, CRC(c01039b5) , ROM_SKIP(7) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "c78-09.12", 0x000000, 0x80000, CRC(0dbde6f5) SHA1(4049271e3738b54e0c56d191889b1aea5664d49f) )    /* ROD, road lines */

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "c78-04.3", 0x00000, 0x80000, CRC(cc1aa37c) SHA1(cfa2eb338dc81c98c637c2f0b14d2baea8b115f5) )   /* STY spritemap */

	ROM_REGION( 0x180000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c78-12.33", 0x000000, 0x100000, CRC(b0267404) SHA1(ffd337336ff9b096e3725f733364762f6e6d3fab) )
	ROM_LOAD( "c78-13.46", 0x100000, 0x080000, CRC(1b363aa2) SHA1(0aae3988024654e98cc0c784307b1c329c8f0783) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c78-14.31",  0x00000, 0x80000, CRC(9cad4dfb) SHA1(9187ef827a3f1bc9233d0e45e72c72c0956c5912) )

	ROM_REGION( 0x10000, "user2", 0 )   /* unused ROMs */
	ROM_LOAD( "c78-25.15",  0x00000, 0x10000, CRC(7245a6f6) SHA1(5bdde4e3bcde8c59dc84478c3cc079d7ef8ee9c5) )    // 98% compression
	ROM_LOAD( "c78-15.22",  0x00000, 0x00100, CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) )    // road A/B internal priority
	ROM_LOAD( "c78-21.74",  0x00000, 0x00100, CRC(2926bf27) SHA1(bfbbe6c71bb29a05959f3de0d940816139f9ebfe) )    // road/sprite priority and palette select
	ROM_LOAD( "c84-10.16",  0x00000, 0x00400, CRC(643e8bfc) SHA1(a6e6086fb8fbd102e01ec72fe60a4232f5909565) )
	ROM_LOAD( "c84-11.17",  0x00000, 0x00400, CRC(10728853) SHA1(45d7cc8e06fbe01295cc2194bca9586f0ef8b12b) )
ROM_END

ROM_START( pwheelsj ) /* Side by side linkable version */
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "c78_26-2.2",  0x00000, 0x20000, CRC(25c8eb2e) SHA1(a526b886c76a19c9ce1abc25cf433574564605a3) )
	ROM_LOAD16_BYTE( "c78_28-2.4",  0x00001, 0x20000, CRC(a9500eb1) SHA1(ad300add3439515512003703df46e2f9317f2ee8) )
	ROM_LOAD16_BYTE( "c78_27-2.3",  0x40000, 0x20000, CRC(08d2cffb) SHA1(a4f117a15499c0df85bf8036f00871caa6723082) )
	ROM_LOAD16_BYTE( "c78_29-2.5",  0x40001, 0x20000, CRC(e1608004) SHA1(c4863264074de09ab38e7b73214f4271728e30aa) )

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "c78-30-1.35", 0x00000, 0x20000, CRC(026aac18) SHA1(f50873982b4dc0fc822060f4c20c635efdd75d7e) )
	ROM_LOAD16_BYTE( "c78-31-1.36", 0x00001, 0x20000, CRC(67ce23e8) SHA1(983e998a79e3d4376b005c92ded050be236d37cc) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c78-32.42",          0x00000, 0x20000, CRC(1494199c) SHA1(f6b6ccaadbc5440f9342750a79ebc00c019ef355) )

	ROM_REGION( 0x100000, "tc0480scp", 0 )
	ROM_LOAD32_WORD( "c78-10.12", 0x00000, 0x80000, CRC(44b1897c) SHA1(7ad179db6d7dfeb139ea13cb4a231f99d177f2b1) )  /* SCR 16x16 */
	ROM_LOAD32_WORD( "c78-11.11", 0x00002, 0x80000, CRC(7db3d4a3) SHA1(fc3c44ed36b212688a5bd8dc61321a994578258e) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "c78-08.25", 0x000000, 0x100000, CRC(6c725211) SHA1(3c1765f44fe57b496d305e994516674f71bd4c3c) )    /* OBJ 16x8 */
	ROM_LOAD64_WORD_SWAP( "c78-07.33", 0x000002, 0x100000, CRC(9da00d5b) SHA1(f6b664c7495b936ce1b99852da45ec92cb37062a) )
	ROM_LOAD64_WORD_SWAP( "c78-06.23", 0x000004, 0x100000, CRC(8309e91b) SHA1(3f27557bc82bf42cc77e3c7e363b51a0b119144d) )
	ROM_LOAD64_WORD_SWAP( "c78-05.31", 0x000006, 0x100000, CRC(90001f68) SHA1(5c08dfe6a2e12e6ca84035815563f38fc2c2c029) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "c78-09.12", 0x000000, 0x80000, CRC(0dbde6f5) SHA1(4049271e3738b54e0c56d191889b1aea5664d49f) )    /* ROD, road lines */

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "c78-04.3", 0x00000, 0x80000, CRC(cc1aa37c) SHA1(cfa2eb338dc81c98c637c2f0b14d2baea8b115f5) )   /* STY spritemap */

	ROM_REGION( 0x180000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c78-01.33", 0x000000, 0x100000, CRC(90ff1e72) SHA1(6115e3683bc701922953b644427d1ddb471bf037) )
	ROM_LOAD( "c78-02.46", 0x100000, 0x080000, CRC(8882d2b7) SHA1(4d3abac1e50cd5ae79a562f430563032a11e8390) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c78-03.31",  0x00000, 0x80000, CRC(9b926a2f) SHA1(cc2d612441a5cc587e097bb8380b56753b9a4f7c) )

	ROM_REGION( 0x10000, "user2", 0 )   /* unused ROMs */
	ROM_LOAD( "c78-25.15",  0x00000, 0x10000, CRC(7245a6f6) SHA1(5bdde4e3bcde8c59dc84478c3cc079d7ef8ee9c5) )    // 98% compression
	ROM_LOAD( "c78-15.22",  0x00000, 0x00100, CRC(fbf81f30) SHA1(c868452c334792345dcced075f6df69cff9e31ca) )    // road A/B internal priority
	ROM_LOAD( "c78-21.74",  0x00000, 0x00100, CRC(2926bf27) SHA1(bfbbe6c71bb29a05959f3de0d940816139f9ebfe) )    // road/sprite priority and palette select
	ROM_LOAD( "c84-10.16",  0x00000, 0x00400, CRC(643e8bfc) SHA1(a6e6086fb8fbd102e01ec72fe60a4232f5909565) )
	ROM_LOAD( "c84-11.17",  0x00000, 0x00400, CRC(10728853) SHA1(45d7cc8e06fbe01295cc2194bca9586f0ef8b12b) )
ROM_END

ROM_START( racingb )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "c84-110.3",  0x00000, 0x20000, CRC(119a8d3b) SHA1(bcda256730c4427c25aab17d2178814289361a78) )
	ROM_LOAD16_BYTE( "c84-111.5",  0x00001, 0x20000, CRC(1f095692) SHA1(6a36f3a62de9fc24724e68a23de782bc21c01734) )
	ROM_LOAD16_BYTE( "c84-104.2",  0x40000, 0x20000, CRC(37077fc6) SHA1(3498db29936f806e1cb624031940fda2e7e601fe) )
	ROM_LOAD16_BYTE( "c84-103.4",  0x40001, 0x20000, CRC(4ca1d1c2) SHA1(cd526db226362b7d4429a29392dee40bcc519556) )

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "c84-99.35",  0x00000, 0x20000, CRC(24778f40) SHA1(5a588be1774af4e179bdc0e16cd118e74bb9f6ff) )
	ROM_LOAD16_BYTE( "c84-100.36", 0x00001, 0x20000, CRC(2b99258a) SHA1(ff2da0f3a0391f55e20655554d72b82cc29fbc87) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c84-101.42",        0x00000, 0x20000, CRC(9322106e) SHA1(6c42ee7b9c76483fec2e397ec2737c030a082267) )

	ROM_REGION( 0x100000, "tc0480scp", 0 )
	ROM_LOAD32_WORD( "c84-90.12",  0x00000, 0x80000, CRC(83ee0e8d) SHA1(a3b6067913f15656e1f74b30b4c0364a50d1846a) ) /* SCR 16x16 */
	ROM_LOAD32_WORD( "c84-89.11",  0x00002, 0x80000, CRC(aae43c87) SHA1(cfc05553f7a18132127ae5f1d181fcc582432b56) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "c84-92.25", 0x000000, 0x100000, CRC(56e8fd55) SHA1(852446d4069a446dd9b88b29e461b83b8d626b2c) )    /* OBJ 16x8 */
	ROM_LOAD64_WORD_SWAP( "c84-94.33", 0x000002, 0x100000, CRC(6117c19b) SHA1(6b9587fb864a325aec17a73046ba5b7be08a8dd2) )
	ROM_LOAD64_WORD_SWAP( "c84-91.23", 0x000004, 0x100000, CRC(b1b0146c) SHA1(d01f08085d644b17445d904a4684c00f133f7bae) )
	ROM_LOAD64_WORD_SWAP( "c84-93.31", 0x000006, 0x100000, CRC(8837bb4e) SHA1(c41fff198a3c87c6e1672174ede589434374c1b3) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "c84-84.12", 0x000000, 0x80000, CRC(34dc486b) SHA1(2f503be67adbc5293f2d1218c838416fd931796c) )    /* ROD, road lines */

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "c84-88.3", 0x00000, 0x80000, CRC(edd1f49c) SHA1(f11c419dcc7da03ef1f1665c1344c27ff35fe867) )   /* STY spritemap */

	ROM_REGION( 0x180000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c84-86.33", 0x000000, 0x100000, CRC(98d9771e) SHA1(0cbb6b08e1fa5e632309962d7ad7dca448ef4d78) )
	ROM_LOAD( "c84-87.46", 0x100000, 0x080000, CRC(9c1dd80c) SHA1(e1bae4e02fd94413fac4683e39e530f9d508d658) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c84-85.31",  0x00000, 0x80000, CRC(24cd838d) SHA1(18139f7df191ff2d005d76b3a85a6fafb630ea42) )

	ROM_REGION( 0x10000, "user2", 0 )   /* unused ROMs */
	ROM_LOAD( "c84-19.15",  0x00000, 0x10000, CRC(7245a6f6) SHA1(5bdde4e3bcde8c59dc84478c3cc079d7ef8ee9c5) )
	ROM_LOAD( "c84-07.22",  0x00000, 0x00100, CRC(95a15c77) SHA1(10246020776cf23c0659f41db66ae2c86db09ed2) )    // road A/B internal priority? bad dump?
	ROM_LOAD( "c84-09.74",  0x00000, 0x00100, CRC(71217472) SHA1(69352cd484b4d5b41b37697aea24107dff8f1b24) )    // road/sprite priority and palette select?
	ROM_LOAD( "c84-10.16",  0x00000, 0x00400, CRC(643e8bfc) SHA1(a6e6086fb8fbd102e01ec72fe60a4232f5909565) )
	ROM_LOAD( "c84-11.17",  0x00000, 0x00400, CRC(10728853) SHA1(45d7cc8e06fbe01295cc2194bca9586f0ef8b12b) )
ROM_END

ROM_START( racingbj )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "c84-107.ic3",  0x00000, 0x20000, CRC(520aa110) SHA1(bc524ce35bcefe5877b5f6e0cae4f6550027673e) )
	ROM_LOAD16_BYTE( "c84-109.ic5",  0x00001, 0x20000, CRC(7ec710de) SHA1(bc66a695fb0f63b819a77f2133b423368bbec6b1) )
	ROM_LOAD16_BYTE( "c84-104.2",    0x40000, 0x20000, CRC(37077fc6) SHA1(3498db29936f806e1cb624031940fda2e7e601fe) )
	ROM_LOAD16_BYTE( "c84-108.ic4",  0x40001, 0x20000, CRC(a2afb0ee) SHA1(82493d97e522bffe511f62dffaba0fc71936c61d) )

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "c84-99.35",  0x00000, 0x20000, CRC(24778f40) SHA1(5a588be1774af4e179bdc0e16cd118e74bb9f6ff) )
	ROM_LOAD16_BYTE( "c84-100.36", 0x00001, 0x20000, CRC(2b99258a) SHA1(ff2da0f3a0391f55e20655554d72b82cc29fbc87) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c84-101.42",        0x00000, 0x20000, CRC(9322106e) SHA1(6c42ee7b9c76483fec2e397ec2737c030a082267) )

	ROM_REGION( 0x100000, "tc0480scp", 0 )
	ROM_LOAD32_WORD( "c84-90.12",  0x00000, 0x80000, CRC(83ee0e8d) SHA1(a3b6067913f15656e1f74b30b4c0364a50d1846a) ) /* SCR 16x16 */
	ROM_LOAD32_WORD( "c84-89.11",  0x00002, 0x80000, CRC(aae43c87) SHA1(cfc05553f7a18132127ae5f1d181fcc582432b56) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "c84-92.25", 0x000000, 0x100000, CRC(56e8fd55) SHA1(852446d4069a446dd9b88b29e461b83b8d626b2c) )    /* OBJ 16x8 */
	ROM_LOAD64_WORD_SWAP( "c84-94.33", 0x000002, 0x100000, CRC(6117c19b) SHA1(6b9587fb864a325aec17a73046ba5b7be08a8dd2) )
	ROM_LOAD64_WORD_SWAP( "c84-91.23", 0x000004, 0x100000, CRC(b1b0146c) SHA1(d01f08085d644b17445d904a4684c00f133f7bae) )
	ROM_LOAD64_WORD_SWAP( "c84-93.31", 0x000006, 0x100000, CRC(8837bb4e) SHA1(c41fff198a3c87c6e1672174ede589434374c1b3) )

	ROM_REGION16_LE( 0x80000, "tc0150rod", 0 )
	ROM_LOAD16_WORD( "c84-84.12", 0x000000, 0x80000, CRC(34dc486b) SHA1(2f503be67adbc5293f2d1218c838416fd931796c) )    /* ROD, road lines */

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "c84-88.3", 0x00000, 0x80000, CRC(edd1f49c) SHA1(f11c419dcc7da03ef1f1665c1344c27ff35fe867) )   /* STY spritemap */

	ROM_REGION( 0x180000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c84-86.33", 0x000000, 0x100000, CRC(98d9771e) SHA1(0cbb6b08e1fa5e632309962d7ad7dca448ef4d78) )
	ROM_LOAD( "c84-87.46", 0x100000, 0x080000, CRC(9c1dd80c) SHA1(e1bae4e02fd94413fac4683e39e530f9d508d658) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c84-85.31",  0x00000, 0x80000, CRC(24cd838d) SHA1(18139f7df191ff2d005d76b3a85a6fafb630ea42) )

	ROM_REGION( 0x10000, "user2", 0 )   /* unused ROMs */
	ROM_LOAD( "c84-19.15",  0x00000, 0x10000, CRC(7245a6f6) SHA1(5bdde4e3bcde8c59dc84478c3cc079d7ef8ee9c5) )
	ROM_LOAD( "c84-07.22",  0x00000, 0x00100, CRC(95a15c77) SHA1(10246020776cf23c0659f41db66ae2c86db09ed2) )    // road A/B internal priority? bad dump?
	ROM_LOAD( "c84-09.74",  0x00000, 0x00100, CRC(71217472) SHA1(69352cd484b4d5b41b37697aea24107dff8f1b24) )    // road/sprite priority and palette select?
	ROM_LOAD( "c84-10.16",  0x00000, 0x00400, CRC(643e8bfc) SHA1(a6e6086fb8fbd102e01ec72fe60a4232f5909565) )
	ROM_LOAD( "c84-11.17",  0x00000, 0x00400, CRC(10728853) SHA1(45d7cc8e06fbe01295cc2194bca9586f0ef8b12b) )
ROM_END


GAMEL(1987, contcirc,   0,        contcirc,  contcirc,  contcirc_state,         empty_init,  ROT0,               "Taito Corporation Japan",   "Continental Circus (World)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_contcirc )
GAMEL(1987, contcircu,  contcirc, contcirc,  contcrcu,  contcirc_state,         empty_init,  ROT0,               "Taito America Corporation", "Continental Circus (US set 1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_contcirc )
GAMEL(1987, contcircua, contcirc, contcirc,  contcrcj,  contcirc_state,         empty_init,  ROT0,               "Taito America Corporation", "Continental Circus (US set 2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_contcirc )
GAMEL(1987, contcircj,  contcirc, contcirc,  contcrcj,  contcirc_state,         empty_init,  ROT0,               "Taito Corporation",         "Continental Circus (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_contcirc )

GAMEL(1988, chasehq,    0,        chasehq,   chasehq,   chasehq_state,          empty_init,  ROT0,               "Taito Corporation Japan",   "Chase H.Q. (World)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_chasehq )
GAMEL(1988, chasehqj,   chasehq,  chasehq,   chasehqj,  chasehq_state,          empty_init,  ROT0,               "Taito Corporation",         "Chase H.Q. (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_chasehq )
GAMEL(1988, chasehqju,  chasehq,  chasehq,   chasehq,   chasehq_state,          empty_init,  ROT0,               "Taito Corporation",         "Chase H.Q. (Japan, upright?)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_chasehq ) // same code rev as Chase H.Q. (World)
GAMEL(1988, chasehqu,   chasehq,  chasehq,   chasehq,   chasehq_state,          empty_init,  ROT0,               "Taito America Corporation", "Chase H.Q. (US)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_chasehq )

GAME( 1988, enforce,    0,        enforce,   enforce,   contcirc_state,         empty_init,  ROT0,               "Taito Corporation Japan",   "Enforce (World)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1988, enforcej,   enforce,  enforce,   enforcej,  contcirc_state,         empty_init,  ROT0,               "Taito Corporation",         "Enforce (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAMEL(1988, enforceja,  enforce,  enforce,   enforceja, contcirc_state,         empty_init,  ROT0,               "Taito Corporation",         "Enforce (Japan, Analog Controls)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_enforceja )

GAME( 1989, bshark,     0,        bshark,    bshark,    taitoz_state,           empty_init,  ORIENTATION_FLIP_X, "Taito Corporation Japan",   "Battle Shark (World)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1989, bsharku,    bshark,   bshark,    bsharku,   taitoz_state,           empty_init,  ORIENTATION_FLIP_X, "Taito America Corporation", "Battle Shark (US)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1989, bsharkj,    bshark,   bshark,    bsharkj,   taitoz_state,           empty_init,  ORIENTATION_FLIP_X, "Taito Corporation",         "Battle Shark (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1989, bsharkjjs,  bshark,   bsharkjjs, bsharkjjs, taitoz_state,           empty_init,  ORIENTATION_FLIP_X, "Taito Corporation",         "Battle Shark (Japan, Joystick)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

GAMEL(1989, sci,        0,        sci,       sci,       sci_state,              empty_init,  ROT0,               "Taito Corporation Japan",   "Special Criminal Investigation (World set 1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_chasehq )
GAMEL(1989, scia,       sci,      sci,       sci,       sci_state,              empty_init,  ROT0,               "Taito Corporation Japan",   "Special Criminal Investigation (World set 2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_chasehq )
GAMEL(1989, scij,       sci,      sci,       scij,      sci_state,              empty_init,  ROT0,               "Taito Corporation",         "Special Criminal Investigation (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_chasehq )
GAMEL(1989, sciu,       sci,      sci,       sciu,      sci_state,              empty_init,  ROT0,               "Taito America Corporation", "Special Criminal Investigation (US)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_chasehq )
GAMEL(1991, scin,       sci,      sci,       sci,       sci_state,              empty_init,  ROT0,               "hack (Negro Torino)",       "Super Special Criminal Investigation (Negro Torino hack)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_chasehq )

GAME( 1989, nightstr,   0,        nightstr,  nightstr,  nightstr_state,         empty_init,  ROT0,               "Taito Corporation Japan",   "Night Striker (World)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1989, nightstrj,  nightstr, nightstr,  nghtstrj,  nightstr_state,         empty_init,  ROT0,               "Taito Corporation",         "Night Striker (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1989, nightstru,  nightstr, nightstr,  nghtstru,  nightstr_state,         empty_init,  ROT0,               "Taito America Corporation", "Night Striker (US)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

GAME( 1990, aquajack,   0,        aquajack,  aquajack,  taitoz_z80_sound_state, empty_init,  ROT0,               "Taito Corporation Japan",   "Aqua Jack (World)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1990, aquajacku,  aquajack, aquajack,  aquajack,  taitoz_z80_sound_state, empty_init,  ROT0,               "Taito America Corporation", "Aqua Jack (US)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1990, aquajackj,  aquajack, aquajack,  aquajckj,  taitoz_z80_sound_state, empty_init,  ROT0,               "Taito Corporation",         "Aqua Jack (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

GAME( 1990, spacegun,   0,        spacegun,  spacegun,  spacegun_state,         empty_init,  ORIENTATION_FLIP_X, "Taito Corporation Japan",   "Space Gun (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, spacegunj,  spacegun, spacegun,  spacegnj,  spacegun_state,         empty_init,  ORIENTATION_FLIP_X, "Taito Corporation",         "Space Gun (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, spacegunu,  spacegun, spacegun,  spacegnu,  spacegun_state,         empty_init,  ORIENTATION_FLIP_X, "Taito America Corporation", "Space Gun (US)", MACHINE_SUPPORTS_SAVE )

GAMEL(1991, dblaxle,    0,        dblaxle,   dblaxles,  dblaxle_state,          empty_init,  ROT0,               "Taito America Corporation", "Double Axle (US, Rev 1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_dblaxle )
GAMEL(1991, dblaxleu,   dblaxle,  dblaxle,   dblaxles,  dblaxle_state,          empty_init,  ROT0,               "Taito America Corporation", "Double Axle (US)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_dblaxle )
GAMEL(1991, dblaxleul,  dblaxle,  dblaxle,   dblaxle,   dblaxle_state,          empty_init,  ROT0,               "Taito America Corporation", "Double Axle (US, Rev 1, Linkable)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN, layout_dblaxle )
GAMEL(1991, pwheelsj,   dblaxle,  dblaxle,   pwheelsj,  dblaxle_state,          empty_init,  ROT0,               "Taito Corporation",         "Power Wheels (Japan, Rev 2, Linkable)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN, layout_dblaxle )

GAMEL(1991, racingb,    0,        racingb,   racingb,   sci_state,              empty_init,  ROT0,               "Taito Corporation Japan",   "Racing Beat (World)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN, layout_dblaxle )
GAMEL(1991, racingbj,   racingb,  racingb,   racingb,   sci_state,              empty_init,  ROT0,               "Taito Corporation",         "Racing Beat (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN, layout_dblaxle )
