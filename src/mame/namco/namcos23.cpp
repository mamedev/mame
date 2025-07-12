// license:BSD-3-Clause
// copyright-holders:R. Belmont, Ryan Holtz, Phil Stroffolino, Olivier Galibert
/*
    Namco System 22.5 and (Super) System 23 (Evolution 2)
    Preliminary driver by R. Belmont and Ryan Holtz, thanks to Phil Stroffolino & Olivier Galibert

    Hardware: * R4650 (MIPS III with IDT special instructions) main CPU.
                133 MHz for Gorgon, 166 MHz for System 23 and Super System 23, and
                200 MHz for Super System 23 Evolution 2.
              * H8/3002 MCU for sound/inputs
              * Custom polygon hardware
              * 1 text tilemap

    All games use a JVS I/O board connected to the H8/3002's serial port #0 and requires an I/O board to
    get past the subcpu check. It's similar to System 22 where one 37702 reads the I/O and communicates
    serially with the second 37702 which is the traditional "subcpu". Several I/O boards are accepted
    including TSS-I/O, FCA, ASCA3, ASCA5 and the common JVS I/O boards manufactured by Sega.

    NOTES:
    - First 128k of main program ROM is the BIOS, and after that is a 64-bit MIPS ELF image.
    - Text layer is identical to System 22 & Super System 22.

    TODO:
    - There are currently no differences seen between System 23 (Time Crisis 2) and
      Super System 23 (500GP, Final Furlong 2). These will presumably appear when
      the 3D hardware is emulated.

    - Serial number data is at offset 0x201 in the BIOS.  Until the games are running
      and displaying it I'm not going to meddle with it though.  Some newer Namco SS22 games
      have similar data there.
      The only difference between motoxgov2a and motoxgov2a2, motoxgov1a and motoxgov1a2,
      panicprkj and panicprkj2 is this data.

    - Improve GMEN hookups/comms.

    - Super System 23 tests irqs in the post.  timecrs2v4a's code can
      potentially test 7 sources, but only actually test 5.  With each
      source there is code to clear the interrupt and code to raise it.
      Levels 0 and 1 are not connected to anything according to the code.

      VBlank (level 2):
        clear: ad00000a.h = 0
        raise: just wait for it

      C361   (level 3):
        clear: a6820008.h = 1ff
               a100005c.w = 0
               a100005c.w = 1
               a4c3ff04.w = 0
        raise: a6820008.h = c8
               a100005c.w = 1

      Subcpu (level 3, same as C361):
        clear: same as C361
        raise: a4405002.h = 3170

      C435   (level 4):
        clear: a200000e.h = 1
               a200000e.h = 0
        raise: a2000000.h = 4f02 (c435 pio, state_set)
               a2000000.h = 1    (          interrupt)
               a2000000.h = 1    (          raise)

      C422   (level 5):
        clear: a6400002.h = f
               ad000008.h = 0
        raise: a640000e.h = 0
               a6400006.h = 1
               a640000a.h = 1
               a6400006.h = fffb
               a6400006.h = 0

      RS232  (level 6, not tested by timecrs2v4a):
        clear: nothing
        raise: nothing

      Timer  (level 7, not tested by timecrs2v4a):
        clear: c0.Compare = 10d880
        raise: c0.Count   = 10c8e0
               c0.Compare = 10d880

   Downhill bikers irq ack on level 3:
        check ad000000 & 0400
          if not, a4c3ff04 = 0
        check ad000000 & 0800
          if not, read a682000a, wait until it stops changing (?)
        return


c8000000:
  8011e384:
   if ((a2000000.w & 0xfff0 != 0x0080) (c417_r, 808e or 008e)
     +10.w = 2
     +16.w = 42
     +16.w = 23c0
     +10.w = 3
     801deaf0.w *0x28 -> +12.w (fixed)




':maincpu' (801142FC): unmapped program memory write to 0C800010 = 00020000 & FFFF0000
':maincpu' (801143A8): unmapped program memory write to 0C800010 = 00020000 & FFFF0000
':maincpu' (801143B4): unmapped program memory write to 0C800014 = 00000042 & 0000FFFF
':maincpu' (801143C0): unmapped program memory write to 0C800014 = 000023C0 & 0000FFFF
':maincpu' (801143CC): unmapped program memory write to 0C800010 = 00030000 & FFFF0000
':maincpu' (801143E0): unmapped program memory write to 0C800010 = 00000000 & 0000FFFF
':maincpu' (801143E0): unmapped program memory write to 0C800010 = 00000000 & 0000FFFF
':maincpu' (801143E0): unmapped program memory write to 0C800010 = 00000000 & 0000FFFF
':maincpu' (801143E0): unmapped program memory write to 0C800010 = 00000000 & 0000FFFF

    Game status:
        all games:          Glitchy graphics.
        downhill/u          Extremely glitchy graphics.
        panicprk/j/j2       Some mini-games are not playable due to Z-sorting issues.
        gunwars/a           Extremely glitchy graphics.
        raceon              Locks up after POST.
        aking               Inputs unresponsive.
        500gp               Occasional lockups.
        crszone(all)        Input issues.

****************************************************************************

Namco System 23 and Super System 23 Hardware Overview (last updated 7th April 2013 at 12.49am)
Namco, 1997 - 2000

Note! This document is a Work-In-Progress and will be updated from time to time as more information becomes available.

This document covers all the known Namco Gorgon / System 23 / Super System 23 games, including....
Final Furlong     Namco, 1997    System 22.5/Gorgon
Rapid River       Namco, 1997    System 22.5/Gorgon
Motocross Go!     Namco, 1997    System 23
Time Crisis II    Namco, 1997    System 23 and Super System 23
Downhill Bikers   Namco, 1997    System 23
Panic Park        Namco, 1998    System 23
Angler King       Namco, 1998    Super System 23
Gunmen Wars       Namco, 1998    Super System 23
Race On!          Namco, 1998    Super System 23
500 GP            Namco, 1998    Super System 23
Final Furlong 2   Namco, 1998    Super System 23
*Guitar Jam       Namco, 1999    Super System 23
Crisis Zone       Namco, 1999    System 23 Evolution 2

* - Guitar Jam is not dumped yet and the hardware type is not confirmed.
    According to Bandai Namco's website it is indeed SS23, and includes an extra sound board with Zoom Corp. DSP.

A System 23 unit is comprised of some of the following pieces....
- SYSTEM23 POWER(A) PCB            Small PCB bolted to the metal box only consisting of power in and network in/out. Only Motocross Go! used this as its
                                   video and sound connectors are mounted on the main board.
- V185B EMI PCB                    Small PCB bolted to the metal box with several connectors including power in, video out, network in/out, sound out
                                   (to AMP PCB) used with most of the S23/SS23 games that use a single main PCB and no other control PCBs.
- V198 EMI PCB                     Small PCB bolted to the metal box with several connectors (power/video/sound etc) plus a couple of extra
                                   ones for the CCD camera and feedback board. It is connected to the main and GMEN boards on the other side via
                                   2 multi-pin connectors (used only with games that use the GMEN board)
- BASS AMP PCB                     Power AMP PCB for general sounds and bass
- SYSTEM23 MAIN PCB                Main PCB for System 23               \
  or SystemSuper23 MAIN(1) PCB     Main PCB for Super System 23         / Note the 3 main boards are similar, but not exactly the same.
  or System23Evolution2 PCB        Main PCB for System 23 Evolution 2  /
- MSPM(FR*) PCB                    A small plug-in daughterboard that holds FLASHROMs containing Main CPU and Sound CPU programs
- FCA PCB                          Controls & I/O interface board used with a few Super System 23 games only. Contains mostly transistors,
                                   caps, resistors, several connectors, an MCU and a PIC16F84.
                                   The PIC is different for EACH game and the FCA PCBs are not interchangeable between different games.
                                   If the FCA PCB is not connected, the game will not advance past the 3rd screen shown below.
- ASCA-3A PCB / ASCA-4A PCB        This is the standard I/O board used with most of the S23/SS23 games with support for digital and
                                   analog controls (buttons/joysticks/pots etc).
- V183 AMC PCB                     I/O board only in Motocross Go that controls handlebar and seat force feedback. It's connected as a slave
                                   I/O board. Half of the board recycles a v145 motor board used in Rave Racer, Ace Driver and Dirt Dash.
- V185 I/O PCB                     Gun I/O board used with Time Crisis II
- V221 MIU PCB                     Gun I/O board used with Crisis Zone (System 23 Evolution 2) and Time Crisis 3 (on System 246)
- SYSTEM23 MEM(M) PCB              Holds mask ROMs for GFX/Sound and associated logic
                                   Note that in Super System23, the MEM(M) PCB is re-used from System23.
                                   On Super System23, there is a sticker over the System23 part labelled 'SystemSuper23' and one
                                   PAL is not populated.
- GMEN PCB                         A large board that sits on top of the main board containing a SH2 CPU, some RAM and a few CPLDs/FPGAs.
                                   This controls the video overlay from the CCD camera in Gunmen Wars, Race On! and Final Furlong 2.
                                   The ROM board plugs in on top of this board.
- V194 STR PCB                     Used with Race On! to control the steering feed-back. An identical re-labelled PCB (V257) with
                                   different SOP44 ROMs is used with Wangan Midnight (Chihiro) and Ridge Racer V (System 246)

The metal box housing these PCB's is approximately the same size as Super System 22. However, the box is mostly
empty. All of the CPU/Video/DSP hardware is located on the main PCB which is the same size as the
Super System 22 CPU board. The ROM PCB is half the size of the Super System22 ROM PCB. The ROM positions on it
can be configured for either 32MBit or 64MBit SOP44 mask ROMs with a maximum capacity of 1664MBits.
The system also uses a dual pipeline graphics bus similar to Super System 22 and has two copies of the graphics ROMs
on the PCB.
The System 23 hardware is the first NAMCO system to require an external 3.3V power source. Previously the 3.3V
was derived from a 5V to 3.3V regulator on systems such as System10/11/12 etc.
The KEYCUS chip is the familiar MACH211 PLCC44 IC as used on System12. The sound system is also taken from System12.

On bootup, the following happens (on 500GP)...

1st screen - Grey screen with white text
                               "SYSTEM 23 BOOTING     "
                               "SDRAM CHECKING A0xx000" (xx = slowly counts up to 3F, from 00), then OK ('CHECKING' is in yellow text, 'OK' is in green text)
   As the SDRAM is being checked, the LEDS 1 to 8 turn off in sequence from 8 to 1.

2nd screen - Grey screen with white text
                               "S.S.23 POWER ON TEST      xxxx"  (xxxx = numbers count up rapidly from 0000)
                               "(C) NAMCO                     "
                               "                     VER. 1.16"
   As these checks happen, the LEDs 1 to 8 flash on/off

3rd screen - Grey screen with white text
                               "S.S.23 POWER ON TEST      xxxx"  (xxxx = numbers count up rapidly from 0000)
                               "SUBCPU INITIALIZING ....      "
                               "SUBCPU PROGRAM Ver. 0211      "

   and a PACMAN eating dots along the bottom of the screen from left to right.
   As these checks happen, the LEDs 1 to 8 simultaneously flash on/off in various patterns.
   The Sub CPU will initialize before the Pacman reaches half-way across the screen.

When the SUB-CPU connects there are numerous POST screens that test almost all of the main components.

On System23, the bootup sequence is shorter. The screen remains blank while the SDRAM is being checked (i.e. 1st screen mentioned above is not shown).
LEDS 1-8 turn off in sequence 8-1. The bank of 8 LEDs on the main board cycles from left to right to left repeatedly (almost all S23/SS23 games seem
to do this in fact). After that, the bootup sequence is mostly the same as SS23.
When the game is running some games just cycle the 8 LEDs left/right/left etc. Others cycle the LEDs in pairs just one LED position
left/right/left etc. Those crazy Namco guys *really* like LEDs.

To skip the (long) POST, set DIP switch #2 of the 8-position DIP switch block to ON. As soon as the SUB-CPU connects the game will boot.
However this only works if the program supports it. Most games just ignore the DIPs.


PCB Layouts
-----------
Rev 1
SYSTEM23 MAIN PCB 8660961103 (8660971103) This rev has J7/J8/J9/J10/J11 populated
Rev 2
SYSTEM23 MAIN PCB 8660961105 (8660971105)
|----------------------------------------------------------------------------|
|       J5       J7  J8       3V_BATT       J9   J10                J11      |
|                     LED1-8         *R4543     *MAX734          ADM485JR    |
|  |-------| LED10-11         LC35256                 CXA1779P  *3414 *3414  |
|  |H8/3002|          *2061ASC-1                                             |
|  |       |       SW4                                *LM358                 |
|  |-------|                    DS8921                *MB88347  *MB87078     |
|J18                                                  *LC78832  *LC78832     |
|              SW3  14.7456MHz                  |----|   |----|  CXD1178Q    |
|             |------| |----| |----||---------| |C435|   |C435|              |
|    N341256  | C416 | |C422| |IDT ||         | |----|   |----|              |
|             |      | |----| |7200||   C403  |                              |
|    N341256  |      |        |----||         |                 |---------|  |
|             |------|        |----||         | PAL(2)  N341256 |         |  |
|   |----| *PST575            |IDT ||---------|                 |  C417   |  |
|   |C352|           CY7C182  |7200|                            |         |  |
|   |----| LED9               |----|                    N341256 |         |  |
|                            J12                                |---------|  |
|     KM416S1020            |-------|   PAL(3)  M5M4V4265                    |
|                           |XILINX |                                        |
|J16                        |XC95108|                                     J17|
|     KM416S1020            |-------|       |---------|  |---------| N341256 |
|                                           |         |  |         |         |
|                                           |   C421  |  |   C404  | N341256 |
|       |---------|              N341256    |         |  |         |         |
|       |         |                         |         |  |         | N341256 |
|       |   C413  |              N341256    |---------|  |---------|         |
|       |         |                                                          |
|       |         |                           M5M4V4265                      |
|       |---------| SW2    LC321664                                          |
|               *PST575                                                      |
|                                                             *KM681000      |
|       |----------|    |---------|                        |-------------|   |
|       |NKK       |    |         |                        |             |   |
|       |NR4650-13B|    |   C361  |           CY2291       |    C412     |   |
|J14    |          |    |         |                        |             |J15|
|       |          |    |         |           14.31818MHz  |             |   |
|       |----------|    |---------|  PAL(1)                |-------------|   |
|                                                             *KM681000      |
|                                                       HM5216165  HM5216165 |
|----------------------------------------------------------------------------|
Notes:
      * - These parts are underneath the PCB.

      Main Parts List:

      CPU
      ---
          NKK NR4650 - R4600-based 64bit RISC CPU (Main CPU, QFP208, clock input source = CY2291)
          H8/3002    - Hitachi H8/3002 HD6413002F17 (Sound CPU, QFP100, running at 14.7456MHz)

      RAM
      ---
          N341256    - NKK 32k x8 SRAM (x9, SOJ28)
          LC35256    - Sanyo 32k x8 SRAM (SOP28)
          KM416S1020 - Samsung 16MBit SDRAM (x2, TSSOP50)
          M5M4V4265  - Mitsubishi 256k x16 DRAM (x2, TSOP40/44)
          LC321664   - Sanyo 64k x16 EDO DRAM (SOJ40)
          HM5216165  - Hitachi 16MBit SDRAM (x2, TSSOP50)
          KM681000   - Samsung 128k x8 SRAM (x2, SOP32)
          CY7C182    - Cypress 8k x9 SRAM (SOJ28)

      Namco Customs
      -------------
                    C352 (QFP100)
                    C361 (QFP120)
                    C403 (QFP136)
                    C404 (QFP208)
                    C412 (QFP256)
                    C413 (QFP208)
                    C416 (QFP176)
                    C417 (QFP208)
                    C421 (QFP208)
                    C422 (QFP64)
                    C435 (x2, QFP144)

      Other ICs
      ---------
               XC95108  - Xilinx XC95108 In-System Programmable CPLD (QFP100)
                            - labelled 'S23MA9' on Rev 1
                            - labelled 'S23MA9B' on Rev 2
               DS8921   - National RS422/423 Differential Line Driver and Receiver Pair (SOIC8)
               CXD1178Q - SONY CXD1178Q  8-bit RGB 3-channel D/A converter (QFP48)
               PAL(1)   - PALCE16V8H (PLCC20, stamped 'PAD23')
               PAL(2)   - PALCE22V10H (PLCC28, stamped 'S23MA5')
               PAL(3)   - PALCE22V10H (PLCC28, stamped 'SS23MA6B')
               MAX734   - MAX734 +12V 120mA Flash Memory Programming Supply Switching Regulator (SOIC8)
               PST575   - PST575 System Reset IC (SOIC4)
               3414     - NJM3414 70mA Dual Op Amp (x2, SOIC8)
               LM358    - National LM358 Low Power Dual Operational Amplifier (SOIC8)
               MB87078  - Fujitsu MB87078 Electronic Volume Control IC (SOIC24)
               MB88347  - Fujitsu MB88347 8bit 8 channel D/A converter with OP AMP output buffers (SOIC16)
               ADM485   - Analog Devices Low Power EIA RS485 transceiver (SOIC8)
               CXA1779P - SONY CXA1779P TV/Video circuit RGB Pre-Driver (DIP28)
               CY2291   - Cypress CY2291 Three-PLL General Purpose EPROM Programmable Clock Generator (SOIC20)
               2061ASC-1- IC Designs 2061ASC-1 clock Generator IC (SOIC16, also found on Namco System 11 PCBs)
               R4543    - EPSON Real Time Clock Module (SOIC14)
               IDT7200  - Integrated Devices Technology IDT7200 256 x9 CMOS Asynchronous FIFO

      Misc
      ----
          J5    - Connector for EMI PCB
          J7/J8 - 15-pin VGA output connectors                        -\
          J9/J10- Red/White Stereo Audio Output RCA connectors          \
          J11   - Standard USB connector (used on Motocross Go!)       -/  Not populated on most PCBs
          J12   - 6-pin connector for In-System Programming of the XC95108 IC
          J14 \
          J15 |
          J16 |
          J17 \ - Connectors for MEM(M) PCB
          J18   - Connector for MSPM(FRA) PCB
          SW2   - 2 position DIP Switch
          SW3   - 2 position DIP Switch
          SW4   - 8 position DIP Switch


SystemSuper23 MAIN(1) PCB 8672960904 8672960104 (8672970104)
|----------------------------------------------------------------------------|
|       J5       J7  J8       3V_BATT       J9   J10                J11      |
|                     LED1-8         *R4543     *MAX734          ADM485JR    |
|  |-------| LED10-11         LC35256                 CXA1779P   3414  3414  |
|  |H8/3002|          *2061ASC-1                                             |
|  |       |       SW4                                *LM358                 |
|  |-------|                    DS8921                *MB88347  *MB87078     |
|J18                                                  *LC78832  *LC78832     |
|               SW3  14.7456MHz                |----|    |----|  CXD1178Q    |
|              |------|  |----|   |---------|  |C435|    |C435|              |
|    N341256   | C416 |  |C422|   |         |  |----|    |----|              |
|              |      |  |----|   |   C444  |                                |
|    N341256   |      |           |         |                   |---------|  |
|              |------|           |         |  PAL(2) CY7C1399  |         |  |
|   |----| *PST575                |---------|                   |  C417   |  |
|   |C352|           CY7C182                                    |         |  |
|   |----| LED9                                       CY7C1399  |         |  |
|                                                               |---------|  |
|     KM416S1020           EPM7064      PAL(3)  KM416V2540                   |
|                                                                            |
|J16                                                                      J17|
|     KM416S1020                            |---------|  |---------| N341256 |
|                                CY7C1399   |         |  |         |         |
|                                           |   C421  |  |   C404  | N341256 |
|       |---------|              CY7C1399   |         |  |         |         |
|       |         |                         |         |  |         | N341256 |
|       |   C413  |                         |---------|  |---------|         |
|       |         |                                                          |
|       |         |                             KM416V2540                   |
|       |---------| SW2    LC321664                                          |
|               *PST575                                                      |
|                                                           *KM416S1020      |
|       |----------|    |---------|                        |-------------|   |
|       |NKK       |    |         |                        |             |   |
|       |NR4650-167|    |   C361  |           CY2291       |    C447     |   |
|J14    |          |    |         |                        |             |J15|
|       |          |    |         |           14.31818MHz  |             |   |
|       |----------|    |---------|  PAL(1)                |-------------|   |
|                                                           *KM416S1020      |
|                                                          71V124   71V124   |
|----------------------------------------------------------------------------|
Notes:
      * - These parts are underneath the PCB.

      Main Parts List:

      CPU
      ---
          NKK NR4650 - R4600-based 64bit RISC CPU (Main CPU, QFP208, clock input source = CY2291)
          H8/3002    - Hitachi H8/3002 HD6413002F17 (Sound CPU, QFP100, running at 14.7456MHz)

      RAM
      ---
          N341256    - NKK 32k x8 SRAM (x5, SOJ28)
          LC35256    - Sanyo 32k x8 SRAM (SOP28)
          KM416S1020 - Samsung 16MBit SDRAM (x4, TSSOP50)
          KM416V2540 - Samsung 256k x16 EDO DRAM (x2, TSOP40/44)
          LC321664   - Sanyo 64k x16 EDO DRAM (SOJ40)
          71V124     - IDT 128k x8 SRAM (x2, SOJ32)
          CY7C1399   - Cypress 32k x8 SRAM (x4, SOJ28)
          CY7C182    - Cypress 8k x9 SRAM (SOJ28)
          M5M4V4265  - Mitsubishi 256k x16 DRAM (x2, TSOP40/44)

      Namco Customs
      -------------
                    C352 (QFP100)
                    C361 (QFP120)
                    C404 (QFP208)
                    C413 (QFP208)
                    C416 (QFP176)
                    C417 (QFP208)
                    C421 (QFP208)
                    C422 (QFP64)
                    C435 (x2, QFP144)
                    C444 (QFP136) This replaces C403 from System 23 but likely integrates the 2x IDT7200 chips used on System 23 inside this custom chip.
                    C447 (QFP256)

      Other ICs
      ---------
               EPM7064  - Altera MAX EPM7064STC100-10 CPLD (TQFP100, labelled 'SS23MA4A')
               DS8921   - National RS422/423 Differential Line Driver and Receiver Pair (SOIC8)
               CXD1178Q - SONY CXD1178Q  8-bit RGB 3-channel D/A converter (QFP48)
               PAL(1)   - PALCE16V8H (PLCC20, stamped 'SS23MA1B')
               PAL(2)   - PALCE22V10H (PLCC28, stamped 'SS23MA2A')
               PAL(3)   - PALCE22V10H (PLCC28, stamped 'SS23MA3A')
               MAX734   - MAX734 +12V 120mA Flash Memory Programming Supply Switching Regulator (SOIC8)
               PST575   - PST575 System Reset IC (SOIC4)
               3414     - NJM3414 70mA Dual Op Amp (x2, SOIC8)
               LM358    - National LM358 Low Power Dual Operational Amplifier (SOIC8)
               MB87078  - Fujitsu MB87078 Electronic Volume Control IC (SOIC24)
               MB88347  - Fujitsu MB88347 8bit 8 channel D/A converter with OP AMP output buffers (SOIC16)
               ADM485   - Analog Devices Low Power EIA RS485 transceiver (SOIC8)
               CXA1779P - SONY CXA1779P TV/Video circuit RGB Pre-Driver (DIP28)
               CY2291   - Cypress CY2291 Three-PLL General Purpose EPROM Programmable Clock Generator (SOIC20)
               2061ASC-1- IC Designs 2061ASC-1 clock generator IC (SOIC16, also found on Namco System 11 PCBs)
               R4543    - EPSON Real Time Clock Module (SOIC14)

      Misc
      ----
          J5    - Connector for EMI PCB
          J7/J8 - 15-pin VGA output connectors                 -\
          J9/J10- Red/White Stereo Audio Output RCA connectors   \
          J11   - Standard USB connector                        -/  Not populated on most PCBs
          J14 \
          J15 |
          J16 |
          J17 \ - Connectors for MEM(M) PCB
          J18   - Connector for MSPM(FRA) PCB
          SW2   - 2 position DIP Switch
          SW3   - 2 position DIP Switch
          SW4   - 8 position DIP Switch


System23Evolution2 MAIN PCB 8902960103 (8902970103)
|----------------------------------------------------------------------------|
|       J5          J7                           3V_BATT                     |
|                     LED1-8     ADM485               *R4543                 |
|  |-------| LED10-11                                  LC35256   CXD1178Q    |
|  |H8/3002|          *2061ASC-1                                             |
|  |       |       SW4                  UDA1320 UDA1320                      |
|  |-------|                    DS8921                                       |
|J18                                                                         |
|               SW3  14.7456MHz                |----|    |----|              |
|              |------|  |----|   |---------|  |C435|    |C435|              |
|    N341256   | C452 |  |C422|   |         |  |----|    |----|              |
|              |      |  |----|   |   C444  |                                |
|    N341256   |      |           |         | XILINX    XILINX  |---------|  |
|              |------|           |         | XC9572XL  XC9536XL|         |  |
|   |----| *PST575                |---------|                   |  C451   |  |
|   |C352|           CY7C182                                    |         |  |
|   |----| LED9                                                 |         |  |
|                                                               |---------|  |
|     KM432S2030                                KM416V2540                   |
|                                                                            |
|J16                                                                      J17|
|     KM432S2030                            |---------|  |---------| N341256 |
|                                IS61LV256  |         |  |         |         |
|                                           |   C421  |  |   C404  | N341256 |
|       |---------|              IS61LV256  |         |  |         |         |
|       |         |                         |         |  |         | N341256 |
|       |   C450  |                         |---------|  |---------|         |
|       |         |                                                          |
|       |         |                             KM416V2540                   |
|       |---------| SW2    LC321664                                          |
|               *PST575                                                      |
|                                                           *KM416S1120      |
|       |----------|    |---------|                        |-------------|   |
|       |IDT       |    |         |                        |             |   |
|       |NR4650-200|    |   C361  |           CY2291       |    C447     |   |
|J14    |          |    |         |                        |             |J15|
|       |          |    |         |           14.31818MHz  |             |   |
|       |----------|    |---------|  PAL                   |-------------|   |
|                                                           *KM416S1120      |
|                                                          71V124   71V124   |
|----------------------------------------------------------------------------|
Notes:
      * - These parts are underneath the PCB.

      Main Parts List:

      CPU
      ---
          NKK NR4650 - R4600-based 64bit RISC CPU (Main CPU, QFP208, clock input source = CY2291)
          H8/3002    - Hitachi H8/3002 HD6413002F17 (Sound CPU, QFP100, running at 14.7456MHz)

      RAM
      ---
          N341256    - NKK 32k x8 SRAM (x5, SOJ28)
          LC35256    - Sanyo 32k x8 SRAM (SOP28)
          KM432S2030 - Samsung 32MBit SDRAM (x2, TSSOP86)
          KM416S1120 - Samsung 16MBit SDRAM (x2, TSSOP50)
          KM416V2540 - Samsung 256k x16 EDO DRAM (x2, TSOP40/44)
          LC321664   - Sanyo 64k x16 EDO DRAM (SOJ40)
          71V124     - IDT 128k x8 SRAM (x2, SOJ32)
          ISS61LV256 - ISSI 32k x8 SRAM (x2, SOJ28)
          CY7C182    - Cypress 8k x9 SRAM (SOJ28)
          M5M4V4265  - Mitsubishi 256k x16 DRAM (x2, TSOP40/44)

      Namco Customs
      -------------
                    C352 (QFP100)
                    C361 (QFP120)
                    C404 (QFP208)
                    C421 (QFP208)
                    C422 (QFP64)
                    C435 (x2, QFP144)
                    C444 (QFP136) This replaces C403 from System 23 but likely integrates the 2x IDT7200 chips used on System 23 inside this custom chip.
                    C447 (QFP256)
                    C450 (BGAxxx)
                    C451 (QFP208)
                    C452 (QFP176)

      Other ICs
      ---------
               XC9536   - XILINX XC9536XL CPLD (TQFP64, labelled 'S23EV2.1')
               XC9572   - XILINX XC9536XL CPLD (TQFP100, labelled 'S23EV2.2')
               DS8921   - National RS422/423 Differential Line Driver and Receiver Pair (SOIC8)
               CXD1178Q - SONY CXD1178Q  8-bit RGB 3-channel D/A converter (QFP48)
               PAL      - PALCE16V8H (PLCC20, stamped 'PAD23')
               PST575   - PST575 System Reset IC (SOIC4)
               ADM485   - Analog Devices Low Power EIA RS485 transceiver (SOIC8)
               CY2291   - Cypress CY2291 Three-PLL General Purpose EPROM Programmable Clock Generator (SOIC20)
               2061ASC-1- IC Designs 2061ASC-1 clock generator IC (SOIC16, also found on Namco System 11 PCBs)
               R4543    - EPSON Real Time Clock Module (SOIC14)
               UDA1320  - Philips UDA1320 Low-cost stereo filter DAC (SOIC16)

      Misc
      ----
          J5    - Connector for EMI PCB
          J7    - 15-pin VGA output connector
          J14 \
          J15 |
          J16 |
          J17 \ - Connectors for MEM(M) PCB
          J18   - Connector for MSPM(FRC) PCB
          SW2   - 2 position DIP Switch
          SW3   - 2 position DIP Switch
          SW4   - 8 position DIP Switch


SystemSuper23 Gmen PCB 8672960502 (8672970502)
Sticker: Gmen (GTR) PCB
                         |---------------------------------------------------|
                         |        J8              LED4    J6        J5       |
                         |          20.25MHz          82AF          LT1635   |
                         |     VXP3220A            611_803        24.576MHz  |
                         |                                      |-----|      |
                         |                                      |FUJI |      |
                         |                           |-----|    |MD8402A     |
                         |                           |FUJI |    |-----|      |
                         |                           |MD8412A                |
                         |                           |-----|                 |
                         |                                                   |
                         |                                                   |
                         |                                                   |
                         |                       TSOP48                      |
|------------------------|                                                   |
|  N341256 N341256                                             N341024       |
|                     HM5118165 HM5118165                                    |
|                                                                            |
|     |---------|                  |---(1)--|                                |
|J10  |         |                  |ALTERA  |                  N341024    J17|
|     |  C446   |      |--------|  |MAX     |                                |
|     |         |      |ALTERA  |  |EPM7064 |                                |
|     |         |      |FLEX    |  |--------|                                |
|     |---------|      |EPM8282 | LED3                     |----------|      |
|                      |--------|                          |ALTERA    |      |
|                                                          |FLEX      |      |
|                                                          |EPM8452   |      |
|                                                          |          |      |
|                         |-------|                        |----------|      |
|                         | SH2   |                           LED2           |
|                         |       |                                          |
|              28.7MHz    |-------|                                          |
|J11                3771         KM416S1020                    N341024    J16|
|    HD63B50                                                                 |
|              40MHz                                                         |
|                                KM416S1020          SW3                     |
| |--(2)--|                                                    N341024       |
| |ALTERA |                                                                  |
| |EPM7064|                                                                  |
| |-------|    LT1180                               LED1    LED5-12          |
|----------------------------------------------------------------------------|
Notes:
      This board controls the video overlay from the CCD camera in Gunmen Wars, Race ON! and Final Furlong 2.
      The main board does it's usual POST then the board fires up, LED1 lights red and the LEDs 5-12 go crazy pulsing left to right from
      the middle outwards. The main board uploads a loader program, then the Main SH2 program. After completion LED 3 lights green and then
      LED 2 lights orange for a second then extinguishes then the main board resets :-/
      It doesn't boot up for unknown reasons but likely another PCB is missing from somewhere in the cabinet or the GMEN PCB is faulty.

      SH2        - Hitachi HD6417604 SH2 CPU (QFP144, clock input 28.7MHz)
      N341024    - NKK 128k x8 SRAM (x4, SOP32)
      N341256    - NKK 32k x8 SRAM (x2, SOP28)
      KM416S1020 - Samsung 16MBit SDRAM (x2, TSSOP50). Also compatible with MB811181622
      HM5118165  - Hitachi 16MBit SDRAM (x2, TSSOP44/50)
      C446       - Namco Custom IC (QFP160)
      TSOP48     - Not-populated position for a 29F400 512k x8 TSOP48 flash ROM
      3771       - Fujitsu MB3771 System Reset IC (SOIC8)
      MD8402     - Fuji MD8402A IEEE 1394 'Firewire' Physical Channel Interface IC (TQFP100)
      MD8412     - Fuji MD8412A IEEE 1394 'Firewire' Link Layer Controller IC (TQFP100)
      VPX3220A   - Micronas Intermetall VPX3220A Video Pixel Decoder IC (PLCC44)
      HD63B50    - Hitachi HD63B50 Asynchronous Communications Interface Adapter IC (SOP24)
      LT1180     - Linear Technology LT1180A Low Power 5V RS232 Dual Driver/Receiver Pair with 0.1mF Capacitors
      EPM7064(1) - Altera MAX EPM7064 CPLD labelled 'SS23GM1A' (TQFP100)
      EPM7064(2) - Altera EPM7064 CPLD labelled 'SS23GM2A' (TQFP44)
      EPM8452    - Altera FLEX EPM8452 CPLD (no label, QFP160)
      EPM8282    - Altera FLEX EPM8282 CPLD (no label, TQFP100)
      82AF       - National 82AF General Purpose EMI Reduction IC (SOIC8)
      LT1635     - Linear Technology LT1635 Micropower Rail-to-Rail Op Amp and Reference Buffer (SOIC8)
      611_803    - ? (SOIC8)
      J5/J6      - IEEE 1394 'Firewire' connectors
      J8         - Connector for EMI PCB
      J10        - Labelled 'WAVE'       \
      J11        - Labelled 'SYS23BUS'   | Connectors for MEM(M) PCB
      J16        - Labelled 'TEXTURE'    | i.e. The ROM board sits on top of this board
      J17        - Labelled 'POINT'      /
      SW3        - 8 position DIP Switch


Namco NamCam
------------

/------------------------\
|   NAMCO          /---\ |
|                  | O | |
|  NAMCAM          \---/ |
\------------------------/

This is a CCD camera used on Gunmen Wars and Race-On and is
mounted into the upper bezel above the monitor in front of the
player. Both games run on Namco Super System 23 hardware.
The camera has a 5 pin connector. That connector is wired to
a 6 pin connector J6 on the V198 EMI PCB on the outside of the metal
box housing the Super System 23 game boards.
The pinout of J6 is...
1 - +5V
2 - GND
3 - VIDEO (color composite video)
4 - GND
5 - NO CONNECT
6 - SCREEN (just tied to GND on the PCB)
Connector J6 eventually connects to the GMEN board to the VPX-3220 IC.

When the game is started the game takes a snapshot of the
player's face which is added on top of the game graphics by
the GMEN PCB.
If the NamCam is not connected the game will hang near the end of
the SH2 initialisation/loading and will not go in-game. If the
camera works enough to get the game booted but doesn't work in-game
the board will hang on the 'taking the photo' screen then eventually
reboot. The camera PCB is made by Toshiba.
The PCB is mounted to the underside of the NamCam.

PCB Top Side
------------

TOSHIBA
PB7408
70177778A
TW10794V-0
767 13604 (sticker)
|--------------------------------|
|        |-------|19.069MHz      |
|        |TOSHIBA|       CCD_CAM |
|        |TC90A22AF              |
|P001    |-------|  93C45A       |
|--------------------------------|
Notes:
TC90A22AF - Toshiba TA90A22AF CCD Camera Digital Signal Processing IC (TQFP80)
   93C45A - 93C45A (TSSOP8) EEPROM, connected to TC90A22AF on pins 59-64.
            pin 58 is NTPA and is grounded so the camera is set to NTSC mode.
            The eeprom holds camera setting data, brightness, contrast etc.
  CCD_CAM - This is the actual CCD sensor and lens
     P001 - 5-pin connector that joins to the V198 EMI PCB


PCB bottom Side
---------------
                    Note: This area has many tiny diodes, inductors, transistors,
TOSHIBA             capacitors etc, most can't be identified to period-correct
PB7408              manufacturers due to undocumented SMD codes.
70177778A            /----/\----\
TW10794V-0          /            \
|--------------------------------|
|             5W01  1R33  GY     |
|        TA1283FN        3800    |
| D16510             00DM   LY   |
|              X2 LY        800mA|
|--------------------------------|
Notes:
  D16510 - NEC uPD16510 Vertical Driver for CCD Sensor (TSSOP20)
TA1283FN - ? Made by Toshiba (TSSOP24)
    3800 - ? possibly an op-amp made by New Japan Radio Co. (i.e. JRC3800?) (TSSOP8)
    1R33 - Sharp PQ1R33 Voltage Regulator (SOIC6)
      X2 - ROHM EMX2 Dual General-Purpose NPN Transistor
      LY - Probably a transistor (SOT23)
      GY - Probably a transistor (SOT89)
    00DM - ? (SOT523)
    5W01 - ?. Possibly ST Microelectronics ST25W01 128 x8-bit Serial EEPROM (connected to TA1283FN) (TSSOP8)


Program ROM PCB
---------------
Type 1:
MSPM(FRA) PCB 8699017500 (8699017400)
|--------------------------|
|            J1            |
|                          |
|  IC3               IC1   |
|                          |
|                          |
|                    IC2   |
|--------------------------|
Notes:
      J1 -  Connector to plug into Main PCB
      IC1 \
      IC2 / Main Program  (Fujitsu 29F016 16MBit FlashROM, TSOP48)
      IC3 - Sound Program (Fujitsu 29F400T 4MBit FlashROM, TSOP48)

      Games that use this PCB include...

      Game             Code and revision
      ----------------------------------
      Time Crisis 2    TSS2 Ver.B (for System 23)
      Time Crisis 2    TSS3 Ver.B (for System 23)
      Gunmen Wars      GM1  Ver.A (for Super System 23)
      Downhill Bikers  DH3  Ver.A (for System 23)
      Motocross Go!    MG3  Ver.A (for System 23)
      Panic Park       PNP2 Ver.A (for System 23)
      Race On!         RO2  Ver.A (for Super System 23)

Type 2:
MSPM(FRA) PCB 8699017501 (8699017401)
|--------------------------|
|            J1            |
|                          |
|  IC2               IC3   |
|                          |
|                          |
|  IC1                     |
|--------------------------|
Notes:
      J1 -  Connector to plug into Main PCB
      IC1 \
      IC2 / Main Program  (Fujitsu 29F016 16MBit FlashROM, TSOP48)
      IC3 - Sound Program (ST M29F400T 4MBit FlashROM, TSOP48)

      Games that use this PCB include...

      Game             Code and revision
      ----------------------------------
      Angler King      AG1  Ver.A (for Super System 23)
      500GP            5GP3 Ver.C (for Super System 23)
      Time Crisis 2    TSS4 Ver.A (for Super System 23)
      Final Furlong 2  FFS1 Ver.A (for Super System 23)
      Final Furlong 2  FFS2 Ver.? (for Super System 23)

Type 3:
MSPM(FRC) PCB 8699019800 (8699019700)
|--------------------------|
|            J1            |
|                          |
|  IC1         IC2   IC4   |
|                          |
|                          |
|                 IC3      |
|--------------------------|
Notes:
      J1 -  Connector to plug into Main PCB
      IC1 - Sound Program (Fujitsu 29F400T 4MBit FlashROM, TSOP48)
      IC3 - a *very tiny* transistor marked 'H5'
      IC2 \
      IC4 / Main Program  (Intel DA28F640J5 64MBit FlashROM, SSOP56)

      Games that use this PCB include...

      Game             Code and revision    Notes
      -----------------------------------------------------------------------
      Crisis Zone      CSZO4 Ver.B          Serialsed ROMs, IC2 not populated
      Crisis Zone      CSZO3 Ver.B          Serialsed ROMs, IC2 not populated
      Crisis Zone      CSZO2 Ver.A          Serialsed ROMs, IC2 not populated


ROM PCB
-------

Printed on the PCB        - 8660960601 (8660970601) SYSTEM23 MEM(M) PCB
Sticker (500GP)           - 8672961100
Sticker (Time Crisis 2)   - 8660962302
Sticker (Crisis Zone)     - 8672961100 .... same as 500GP
Sticker (Race On!)        - 8672961100 .... same as 500GP
Sticker (Angler King)     - 8672961100 .... same as 500GP

|----------------------------------------------------------------------------|
| KEYCUS    MTBH.2M      CGLL.4M        CGLL.5M         CCRL.7M       PAL(3) |
|                                                                            |
|                                                                            |
|J1         MTAH.2J      CGLM.4K        CGLM.5K         CCRH.7K            J4|
|                                                                            |
|   PAL(4)                                            JP5                    |
|                        CGUM.4J        CGUM.5J       JP4                    |
|           MTAL.2H                                   JP3                    |
|                                                     JP2                    |
|                        CGUU.4F        CGUU.5F                              |
|                                                       CCRL.7F              |
|           MTBL.2F                                                          |
|                            PAL(1)    PAL(2)                                |
|                                                       CCRH.7E              |
|                                                                            |
|         JP1                                                                |
|                                                                            |
|       WAVEL.2C      PT3L.3C      PT2L.4C      PT1L.5C      PT0L.7C         |
|J2                                                                        J3|
|                                                                            |
|       WAVEH.2A      PT3H.3A      PT2H.4A      PT1H.5A      PT0H.7A         |
|                                                                            |
|                                                                            |
|----------------------------------------------------------------------------|
Notes:
      J1   \
      J2   |
      J3   |
      J4   \   - Connectors to main PCB
      JP1      - ROM size configuration jumper for WAVE ROMs. Set to 64M, alt. setting 32M
      JP2      - ROM size configuration jumper for CG ROMs. Set to 64M, alt. setting 32M
      JP3      - ROM size configuration jumper for CG ROMs. Set to 64M, alt. setting 32M
      JP4      - ROM size configuration jumper for CG ROMs. Set to 64M, alt. setting 32M
      JP5      - ROM size configuration jumper for CG ROMs. Set to 64M, alt. setting 32M
                 Other ROMs
                           CCRL - size fixed at 32M
                           CCRH - size fixed at 16M
                           PT*  - size fixed at 32M
                           MT*  - size fixed at 64M

      KEYCUS   - Mach211 CPLD (PLCC44)
      PAL(1)   - PALCE20V8H  (PLCC28, stamped 'SS22M2')  \ Both identical
      PAL(2)   - PALCE20V8H  (PLCC28, stamped 'SS22M2')  /
      PAL(3)   - PALCE16V8H  (PLCC20, stamped 'SS22M1')
      PAL(4)   - PALCE16V8H  (PLCC20, stamped 'SS23MM1')
                 Note this PAL is not populated when used on Super System 23

      All ROMs are SOP44 MaskROMs
      Note: ROMs at locations 7M, 7K, 5M, 5K, 5J & 5F are copies of identical ROMs on the PCB at locations 7F, 7E, 4M, 4K, 4J, 4F
            Each ROM is stamped with the Namco Game Code, then the ROM-use code (such as CCRL, CCRH, PT* or MT*).

                            Game
            Game            Code     Keycus    Notes
            -----------------------------------------------------------------------
            500GP           5GP1     KC029     -
            Angler King     AG1      KC028     -
            Crisis Zone     CSZ1     KC039     -
            Downhill Bikers DH1      KC016     3A, 3C, 2M and 2F not populated.
            Final Furlong 2 FFS1     KC???     -
            Gunmen Wars     GM1      KC018     3A, 3C, 4A, 4C, 2A, 2F and 2M not populated.
            Motocross Go!   MG1      KC009     3A, 3C, 4A, 4C, 4F and 7F not populated.
            Panic Park      PNP1     KC015     3A, 3C, 4A, 4C, 2M and 2F not populated.
            Race On!        RO1      KC017     2M and 2F not populated.
            Time Crisis 2   TSS1     KC010     3A and 3C not populated.

I/O PCBs
--------

V194 STR PCB
2487960102 (2487970102)
|----------------------------------------------------------|
|         RO1_STR-0A.IC16      TRANSFORMER        J105     |
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
|       LED   JP1 O O-O                                    |
|       LED                            HP3150         K2682|
|                                                          |
|                                      HP3150              |
|                                                          |
|J103                                                      |
|                UPC358  LM393   UPC358               K2682|
|            J102                            J106          |
|----------------------------------------------------------|
Notes:
      This board is used with Race On! (and Wangan Midnight on Chihiro and Ridge Racer V on System 246) to control the
      steering feed-back motor. It may be used with other System 23/Super System 23 driving/racing games too but no
      other games are confirmed at the moment.

      RO1_STR-0A.IC16 - Fujitsu MB29F400TC 512k x8 flash ROM (SOP44)
                        - Labelled 'RO1 STR-0A' for Race On!
                        - Labelled 'RR3 STR-0A' for Ridge Racer V (on System 246)
      EPM7064         - Altera EPM7064 CPLD labelled 'STR-DR1' (PLCC44)
      N341256         - NKK 32k x8 SRAM (SOP28)
      K2682           - 2SK2682 N-Channel Silicon MOSFET
      BF150G8E        - Large power transistor(?) connected to the transformer
      UPC358          - NEC uPC358 Dual operational amplifier (SOIC8)
      LM393           - National LM393 Low Power Low Offset Voltage Dual Comparator (SOIC8)
      MAX232          - Maxim MAX232 dual serial to TTL logic level driver/receiver (SOIC16)
      HP3150          - ? (DIP8)
      MB3773          - Fujitsu MB3773 Power Supply Monitor with Watch Dog Timer and Reset (SOIC8)
      MB3771          - Fujitsu MB3771 System Reset IC (SOIC8)
      DIP42           - Unpopulated DIP42 socket for 27C4096 EPROM
      MB90242A        - Fujitsu MB90242A 16-Bit CISC ROM-less F2MC-16F Family Microcontroller optimized for mechatronics control applications (TQFP80)
      J101            - 8 pin connector (purpose unknown)
      J102            - 3 pin connector input from potentiometer connected to the steering wheel mechanism
      J103            - Power input connector (5v/GND/12v)
      J104            - 6 pin connector joined with a cable to J6 on the V198 EMI PCB. This cable is the I/O connection to/from the main board.
      J105            - 110VAC power input
      J106            - DC variable power output to feed-back motor


********************************************************************************************

Namco System 22.5 GORgON-based games
Hardware info by Guru
---------------------

Games on this system include....
Rapid River   (Namco, 1997)
Final Furlong (Namco, 1997)

These games run on hardware called "GORgON". This is half-way hardware after Namco Super System 22 and
before System 23. It has similar capabilities to System 23 but like Super System 22 it has sprites also, whereas
System 23 is a full 3D system and doesn't have sprites. The PCBs are about two times larger than System 23.

For Final Furlong (the first game on this system) the system comprises TWO Main PCBs each with ROM PCB on top.
There are 2x GORgON AV PCBs plugged directly into each main board providing connectors for power, audio,
video and comms with 3 separate power supplies for 5V, 12V and 3.3V all located inside a metal box. There is a
separate external I/O PCB ASCA-1A. This connects to the filter board via RS485 using the USB connector.
Main input power is 115VAC.
Final Furlong is a horse racing game.
To control the horse you rock it forwards and backwards continually (it's very tiring to play this game).
This activates one 5K-ohm potentiometer inside the horse body. Essentially the pot just moves from one extreme
to the other. Just like a real horse you need to control the speed so your horse lasts the entire race.
If you rock too much, a message on screen says 'Too Fast'. To steer the horse turn the head sideways using the reins.
There is another 5K-ohm potentiometer in the head to activate the turning direction. The head of the horse also has
two buttons for left and right. This is used to select items and activate the whip. When riding the horse, pressing either
button activates the whip (i.e. they both do the same thing). To select a different track or different horse turn the head.

For Rapid River there is only one main board and ROM board (linking/networking machines is not possible). The I/O
board is ASCA-2A and is different to the I/O board used with Final Furlong and plugs directly into the main board.
This board has the same connectors as the Final Furlong GORgON AV PCB but also has an audio amp and a standard 8 pin
JVS power connector. The I/O board is located inside the metal box. There are two flat cables on the I/O board joining
it to the filter board which is bolted to the outside of the metal box.
Rapid River is controlled by rotating a paddle (for thrust) and turning it sideways (moves left/right).
The rotation action is done with a 5K-ohm potentiometer whereby the thrust is achieved by moving the pot from full left to
full right continuously. The left/right turning movement is just another 5K-ohm potentiometer connected to the column of the paddle
center shaft. There are also some buttons just for test mode, including SELECT, UP & DOWN. The player's seat has movement
controlled by a compressor and several potentiometers. On bootup, the system tests the seat movement and displays a warning
if it's not working. Pressing START allows the game to continue and function normally without the seat movement.


Main PCB
--------

8664960102 (8664970102) GORGON MAIN PCB
|------------------------------------------------------------------------------------------------------|
|                                   J4                       J5                         J6             |
|                              |---------|           |---------| |------| |---------|                  |
|         |---------| |------| |         |           |         | |C401  | |         |HM534251 HM534251 |
| CXD1178Q|         | |C381  | |  C374   |  |------| |  C417   | |      | |  C304   |HM534251 HM534251 |
|         |  C404   | |      | |         |  |C435  | |         | |------| |         |HM534251 HM534251 |
|         |         | |------| |         |  |      | |         | |------| |         |                  |
|         |         |          |---------|  |------| |---------| |C400  | |---------|                  |
|         |---------|     |---------|       |------|             |      | |---------|                  |
|                         |         |       |C435  |    341256   |------| |         |HM534251 HM534251 |
|                         |  C397   |       |      |             |------| |  C304   |HM534251 HM534251 |
|  341256 341256  341256  |         |       |------|    341256   |C401  | |         |HM534251 HM534251 |
|  M5M51008       341256  |         |     |---------|            |      | |         |                  |
|                         |---------|     |         | |------|   |------| |---------|                  |
|  M5M51008       341256         |------| |  C403   | |C406  |   |------| |---------|                  |
|ADM485              |---------| |C379  | |         | |      |   |C400  | |         |HM534251 HM534251 |
|                    |         | |      | |         | |------|   |      | |  C304   |HM534251 HM534251 |
|    M5M51008        |  C300   | |------| |---------|            |------| |         |HM534251 HM534251 |
|                    |         | LH540204  LH540204              |------| |         |                  |
|    M5M51008        |         |341256                 |------|  |C401  | |---------|                  |
|J1   HCPL0611       |---------|341256                 |C407  |  |      | |---------|                  |
|         DS8921                  PST575  PST575       |      |  |------| |         |                  |
|  DS8921                                              |------|  |------| |  C304   |HM534251 HM534251 |
|                 M5M51008                                       |C400  | |         |HM534251 HM534251 |
|       CY7C128             CY2291S                              |      | |         |                  |
|         |------|M5M51008  14.31818MHz                          |------| |---------|                  |
|         |C422  |          J9           M5M5256                 |------| |---------|         3V_BATT  |
|         |      |341256                                         |C400  | |         |                  |
|         |------|341256                                         |      | |  C399   |341256  LEDS(8)   |
|                                   |------|      |--------|     |------| |         |341256            |
|                                   |C352  |      |ALTERA  |     |------| |         |                  |
|  ADM485        DSW1(2)   |------| |      |      |EPM7128 |     |C401  | |---------| DSW3(2)   DSW5(8)|
|    2061ASC               |C416  | |------|      |        |     |      |     |---------| |---------|  |
|      14.7456MHz          |      |               |--------|     |------|     |         | |NKK      |  |
|PAL             |-----|   |------|    |------|                   D4516161    |  C413   | |NR4650   |  |
|                |H8/  |               |C361  |                   D4516161    |         | |LQF-13B  |  |
|                |3002 |               |      |                               |         | |         |  |
|  J10           |-----|      LC321664 |------|    J8                         |---------| |---------|  |
|------------------------------------------------------------------------------------------------------|
Notes:
       NR4650 - NKK NR4650 R4600-based 64-bit RISC CPU (Main CPU, QFP208). Clock input source = CY2291S pin 10
                R4650 master clock input on pin 185 is 33.3333MHz [66.6666/2; CY2291S output = 66.6666]
      H8/3002 - Hitachi H8/3002 HD6413002F17 (Sound CPU, QFP100). Clock input 16.74115MHz. Clock source = ASC2061 MCLKOUT/2
      EPM7128 - Altera EPM7128 CPLD labelled 'GOR-M1' (PLCC84). This chip controls *MANY* chip-enable and clock signals.
          PAL - PALCE16V8H stamped 'GOR-M3' (PLCC20)
     HM534251 - Hitachi HM534251 256kB x4 Dynamic Video RAM (SOJ28)
      N341256 - NKK 32kB x8 SRAM (SOJ28)
      M5M5256 - Mitsubishi 32kB x8 SRAM (SOP28)
     D4516161 - NEC uPD4516161AG5-A80 1M x16 (16MBit) SDRAM (SSOP50)
     LC321664 - Sanyo 64kB x16 EDO DRAM (SOJ40)
     M5M51008 - Mitsubishi 128kB x8 SRAM (SOP32)
      CY7C128 - Cypress 2kB x8 SRAM (SOJ28)
     LH540204 - Sharp CMOS 4096 x 9 Asynchronous FIFO (PLCC32)
    2061ASC-1 - IC Designs 2061ASC-1 Programmable Clock Generator (SOIC16). XTAL input = 14.7456MHz
                This chip uses some internal ROM tables and formulas and is programmed with several registers to generate 2 output clocks.
                The measured values below can vary depending on the input clock frequency and accuracy tolerance.
                Measured Outputs: VCLKOUT - 25.9282MHz, MCLKOUT - 33.4823MHz
      CY2291S - General Purpose EPROM Programmable Clock Generator. Clock input 14.31818MHz
                Full part number is CY2291SC-221. SC="Special Customer". This is custom-programmed at the factory per Namco specifications.
                This chip uses some internal ROM tables to generate 6 output clocks. Not all of the outputs are actually used on the PCB.
                The measured values below can vary depending on the input clock frequency and accuracy tolerance.
                Measured Outputs: CPUCLK - 66.6666MHz, CLKB - 51.200MHz, CLKA - 20.000MHz , CLKF - none, CLKD - none, CLKC - 40.000MHz
                The outputs CLKA/C/D/F are not connected.
       DS8921 - Dallas Semiconductor DS8921 RS-422/423 Differential Line Driver and Receiver Pair (SOIC8)
     HCPL0611 - Fairchild HCPL0611 High Speed 10MBits/sec Logic Gate Optocoupler (SOIC8)
       ADM485 - Analog Devices ADM485 5V Low Power EIA RS-485 Transceiver (SOIC8)
                This is used for I/O PCB communication via H8/3002 signals PB0, P90 and P92.
       PST575 - System Reset IC (SOIC8)
     CXD1178Q - Sony CXD1178Q 8-bit RGB 3-channel D/A converter (QFP48). R,G,B Clock inputs 12.800MHz. Source clock is CY2291S CLKB [51.200/4]
           J1 - 64 pin connector for connection of I/O board
     J4/J5/J6 \
        J8/J9 / Custom NAMCO connectors for connection of MEM(M1) PCB
          J10 - Custom NAMCO connector for MSPM(FR) PCB


     Namco Custom ICs
     ----------------
         C300 (QFP160) - Sprite-related functions
     C304 (x4, QFP120) - Texture-related functions. Grouped with a C400 and C401 for each chip (4 sets).
         C352 (QFP100) - 32-Voice 4-channel 8-bit PCM Sound. Clock input 25.9282MHz (source = 2061ASC-1 pin 9)
         C361 (QFP120) - Text / Character Generator + HSync / VSync Generator
         C374 (QFP160) - Sprite-related functions / Sprite Zoom
          C379 (QFP64) - Sprite-related functions
         C381 (QFP144) - Sprite-related functions
         C397 (QFP160) - Sprite-related functions
         C399 (QFP160) \ This chip ties all the texture outputs from C304, C400 & C401 together and probably does CPU <> 3D System Communication.
     C400 (x4, QFP100) | Texture-related functions (these run burning hot then fail.... result = 3D objects all white and no textures ;-)
      C401 (x4, QFP64) /
         C403 (QFP136) - Polygon-related functions + FIFO data supply source
         C404 (QFP208) - GAMMA, Palette, Pixel Mixer, 24-bit RGB output directly to CXD1178Q (8-bits per color)
         C406 (QFP120) - Polygon-related functions
          C407 (QFP64) - Polygon-related functions
         C413 (QFP208) - Memory Controller
         C416 (QFP176) - CPU <> CPU Communication (R4650 <> H8/3002)
         C417 (QFP208) - Polygon Generator
          C422 (QFP64) - RS422 Networking (Twin Cabinet) Communication Controller
    C435 (x2, TQFP144) - Polygon-related functions


Program ROM PCB
---------------

MSPM(FR) PCB 8699015200 (8699015100)
|--------------------------|
|            J1            |
|                          |
|  IC3               IC1   |
|                          |
|                          |
|                    IC2   |
|--------------------------|
Notes:
      J1 - Connector to plug into Main PCB
     IC1 \
     IC2 / Main Program  (Fujitsu 29F016 16MBit FlashROM, TSOP48)
     IC3 - Sound Program (Fujitsu 29F400T 4MBit FlashROM, TSOP48)

     Games that use this PCB include...

     Game           Code and revision
     --------------------------------
     Rapid River    RD2 Ver.C
     Rapid River    RD3 Ver.C
     Final Furlong  FF2 Ver.A


ROM PCB
-------

MEM(M1) PCB
8664960202 (8664970202)
|--------------------------------------------------------|
|    J2(TEXTURE)        J3(POINT)           J5(SPRITE)   |
| PAL1                                                   |
|                                                        |
|                                                        |
|                                                        |
| CCRL.11A                                               |
|      CCRL.11E  PT3L.12J PT3H.12L  SPRLL.12P SPRLL.12T  |
| CCRH.11B                                               |
|      CCRH.11F                                          |
|                PT2L.11J PT2H.11L  SPRLM.11P SPRLM.11T  |
|                                                        |
|                                                        |
|                PT1L.10J PT1H.10L  SPRUM.10P SPRUM.10T  |
|   PAL2        PAL3                                     |
|                                                        |
|                PT0L.9J  PT0H.9L   SPRUU.9P  SPRUU.9T   |
|                                   JP7       JP9        |
|                                   JP6       JP8        |
| CGLL.8B     CGLL.8F                                    |
|                                                        |
|                                                        |
| CGLM.7B     CGLM.7F                                    |
|      JP2    JP4                                        |
|      JP1    JP3                                        |
| CGUM.6B     CGUM.6F                                    |
|                                          J1(WAVE)      |
|                                                        |
| CGUU.5B     CGUU.5F                      WAVEH.3S      |
|                                                        |
|                    MTBH.5J               WAVEL.2S      |
|                    MTAH.3J                      JP5    |
|                    MTBL.2J                             |
|                    MTAL.1J    KEYCUS                   |
|                                                        |
|                    J4(MOTION)                          |
|--------------------------------------------------------|
Notes:
        PAL1 - PALCE16V8H stamped 'SS22M1' (PLCC20)
        PAL2 - PALCE20V8H stamped 'SS22M2' (PLCC32)
        PAL3 - PALCE20V8H stamped 'SS22M2' (PLCC32)
      KEYCUS - for Rapid River: MACH211 CPLD stamped 'KC012' (PLCC44)
      KEYCUS - for Final Furlong: MACH211 CPLD stamped 'KC011' (PLCC44)
      J1->J5 - Custom NAMCO connectors for joining ROM PCB to Main PCB
     JP1/JP2 \
     JP3/JP4 |
     JP5     | Jumpers to set ROM sizes (32M/64M)
     JP6/JP7 |
     JP8/JP9 /

     ROMs
     ----
           PT* - Point ROMs, sizes configurable to either 16M or 32M (SOP44)
           MT* - Motion ROMs, sizes configurable to either 32M or 64M (SOP44)
           CG* - Texture ROMs, sizes configurable to either 32M or 64M (SOP44)
          CCR* - Texture Tilemap ROMs, sizes fixed at 16M (SOP44)
          SPR* - Sprite ROMs, sizes configurable to either 32M or 64M (SOP44)
          WAVE*- Wave ROMs, sizes configurable to either 32M or 64M (SOP44)

Other Boards
------------

GORgON AV PCB
8664960301 (8664970301)
|------------------------------------|
|J2   J3     J4        J5       J6   |
|            BD-8                    |
|  PC410  74AC244 NJM2100*           |
|    74AC00  LC78815       NJM2100*  |
|             J1    LC78815          |
|------------------------------------|
Notes: (* = these parts on bottom side of PCB)
     J1 - 64 pin connector for connection to Main PCB
     J2 - 10 pin connector
     J3 - 15 pin HD15 DSUB connector
     J4 - Dual Red/White RCA Jacks (Twin Stereo Audio)
     J5 - USB-A connector
     J6 - 6 pin power input connector (GND, GND, GND, 5V, 5V, 12V)
   BD-8 - TDK ZBDS5101-8 Ferrite Bead SMD Array
  PC410 - Sharp PC410 Photocoupler
 74AC00 - 74AC00 Quad 2-Input NAND Gate
74AC244 - 74AC244 Octal Buffer/Line Driver with Tri-state Outputs
NJM2100 - New Japan Radio Co. NJM2100 Dual Operational Amplifier
LC78815 - Sanyo LC78815 2-Channel 16-Bit D/A Converter

This board plugs into the mainboard used for Final Furlong and connects to ASCA-1A I/O PCB.


V187 ASCA-2B PCB
2477960201 (2477970201)
|-----------------------------------------------|
|                                               |
|  J207      J204     J206       J205     J203  |
|                                               |
|                                               |
|                                               |
|          J202*                    J201*       |
|-----------------------------------------------|
Notes: (* = these parts on bottom side of PCB)
      J201 - 34 pin flat cable connector for connection to ASCA-2A I/O PCB
      J202 - 50 pin flat cable connector for connection to ASCA-2A I/O PCB
      J203 - 9 pin connector. Pinout: 1 RED, 2 GREEN, 3 BLUE, 4 GND, 5 CSYNC, 6 SPK L+, 7 SPK R-, 8 SPK R+, 9 SPK L-
      J204 - 15 pin connector. Pinout: 1 GND, 2 12V, 3 GND, 4 5K-POT, 5 5K-POT, 6 SELECT, 7 NC, 8 NC, 9 UP, 10 5V, 11 NC, 12 NC, 13 LAMP, 14 START, 15 DOWN
             When wired to Final Furlong changes are: 11 RIGHT, 14 LEFT
      J205 - 6 pin connector. Pinout: 1 SERVICE, 2 TEST, 3 COIN, 4 GND, 5 NC, 6 NC
      J206 - 12 pin connector
      J207 - 12 pin connector. Pinout: 1 SOL FR, 2 SOL FL, 3 SOL RR, 4 SOL RL, 5 12V, 6 NC, 7 5K POT RL, 8 GND, 9 5K POT FR, 10 5K POT FL, 11 5K POT RR, 12 5V
             FR/FL/RR/RL means Front Left, Front Right, Rear Right, Rear Left. SOL means Solenoid.

This is the filter board bolted to the outside of the metal box for Rapid River. It plugs into V187 ASCA-2A I/O PCB with 2 flat cables.
It can also be used with Final Furlong when wired correctly.


*/

#include "emu.h"
#include "bus/jvs/namcoio.h"
#include "bus/rs232/rs232.h"
#include "cpu/h8/h83002.h"
#include "cpu/mips/mips3.h"
#include "cpu/sh/sh7604.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/nvram.h"
#include "machine/rtc4543.h"
#include "sound/c352.h"
#include "video/rgbutil.h"
#include "video/poly.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "md8412b.h"
#include "namco_settings.h"
#include "vpx3220a.h"

#include <cfloat>

#define LOG_PROJ_MAT        (1ULL << 1)
#define LOG_3D_STATE_ERR    (1ULL << 2)
#define LOG_3D_STATE_UNK    (1ULL << 3)
#define LOG_MATRIX_ERR      (1ULL << 4)
#define LOG_MATRIX_UNK      (1ULL << 5)
#define LOG_VEC_ERR         (1ULL << 6)
#define LOG_VEC_UNK         (1ULL << 7)
#define LOG_RENDER_ERR      (1ULL << 8)
#define LOG_RENDER_INFO     (1ULL << 9)
#define LOG_MODEL_ERR       (1ULL << 10)
#define LOG_MODEL_INFO      (1ULL << 11)
#define LOG_MODELS          (1ULL << 12)
#define LOG_C435_PIO_UNK    (1ULL << 13)
#define LOG_C435_UNK        (1ULL << 14)
#define LOG_C417_UNK        (1ULL << 15)
#define LOG_C417_ACK        (1ULL << 16)
#define LOG_C412_UNK        (1ULL << 17)
#define LOG_C421_UNK        (1ULL << 18)
#define LOG_C422_IRQ        (1ULL << 19)
#define LOG_C422_UNK        (1ULL << 20)
#define LOG_C361_UNK        (1ULL << 21)
#define LOG_CTL_UNK         (1ULL << 22)
#define LOG_MCU             (1ULL << 23)
#define LOG_SH2             (1ULL << 24)
#define LOG_SUBIRQ          (1ULL << 25)
#define LOG_SPRITES         (1ULL << 26)
#define LOG_IOMCU           (1ULL << 27)
#define LOG_ADC_RD          (1ULL << 28)
#define LOG_ADC_WR          (1ULL << 29)
#define LOG_C417_IRQ        (1ULL << 30)
#define LOG_C361_IRQ        (1ULL << 31)
#define LOG_MATRIX_INFO     (1ULL << 32)
#define LOG_VEC_INFO        (1ULL << 33)
#define LOG_CTL_REG         (1ULL << 34)
#define LOG_C435_REG        (1ULL << 35)
#define LOG_C361_REG        (1ULL << 36)
#define LOG_C417_REG        (1ULL << 37)
#define LOG_C412_RAM        (1ULL << 38)
#define LOG_C421_RAM        (1ULL << 39)
#define LOG_C404_REGS       (1ULL << 40)
#define LOG_C404_RAM        (1ULL << 41)
#define LOG_GMEN            (1ULL << 42)
#define LOG_MCU_PORTS       (1ULL << 43)
#define LOG_RS232           (1ULL << 44)
#define LOG_IRQ_STATUS      (1ULL << 45)
#define LOG_C451            (1ULL << 46)
#define LOG_ALL ( LOG_PROJ_MAT | LOG_3D_STATE_ERR | LOG_3D_STATE_UNK | LOG_VEC_ERR | LOG_VEC_UNK | LOG_RENDER_ERR | LOG_RENDER_INFO | LOG_MODEL_ERR | \
				LOG_MODEL_INFO | LOG_MODELS | LOG_C435_PIO_UNK | LOG_C435_UNK | LOG_C417_UNK | LOG_C417_ACK | LOG_C412_UNK | LOG_C421_UNK | \
				LOG_C422_IRQ | LOG_C422_UNK | LOG_C361_UNK | LOG_CTL_UNK | LOG_C417_IRQ | LOG_C361_IRQ | LOG_MATRIX_INFO | LOG_VEC_INFO | \
				LOG_CTL_REG | LOG_C435_REG | LOG_C361_REG | LOG_C417_REG | LOG_C412_RAM | LOG_C421_RAM | LOG_C404_REGS | LOG_C404_RAM | LOG_GMEN | \
				LOG_GENERAL | LOG_RS232 | LOG_IRQ_STATUS | LOG_C451 | LOG_MATRIX_UNK | LOG_VEC_UNK | LOG_MCU_PORTS )

#define VERBOSE ( 0 )
#include "logmacro.h"

namespace
{

#define JVSCLOCK    (XTAL(14'745'600))

#define H8CLOCK     (16934400)      /* based on research (superctr) */
#define BUSCLOCK    (16934400*2)
#define C352CLOCK   (25401600)
#define C352DIV     (288)

#define PIXEL_CLOCK (51.2_MHz_XTAL/4*2)

#define HTOTAL      (814)
#define HBEND       (0)
#define HBSTART     (640)

#define VTOTAL      (525)
#define VBEND       (0)
#define VBSTART     (480)

#define MAIN_VBLANK_IRQ 0x01
#define MAIN_C361_IRQ   0x02
#define MAIN_SUBCPU_IRQ 0x04
#define MAIN_C435_IRQ   0x08
#define MAIN_C422_IRQ   0x10
#define MAIN_C450_IRQ   0x20
#define MAIN_C451_IRQ   0x40
#define MAIN_RS232_IRQ  0x100

#define DUMP_MODELS 0

static s32 u32_to_s24(u32 v);
static s32 u32_to_s10(u32 v);

enum
{
	MODEL,
	IMMEDIATE,
	DIRECT,
	SPRITE
};

enum
{
	RENDER_MAX_ENTRIES = 4000,
	POLY_MAX_ENTRIES = 40000
};

struct c404_mixer_regs_t
{
	u8 poly_fade_r;
	u8 poly_fade_g;
	u8 poly_fade_b;
	u8 fog_r;
	u8 fog_g;
	u8 fog_b;
	u32 bgcolor;
	u16 spot_factor;
	u8 poly_alpha_color;
	u8 poly_alpha_pen;
	u8 poly_alpha;
	u8 alpha_check12;
	u8 alpha_check13;
	u8 alpha_mask;
	u8 alpha_factor;
	u8 screen_fade_r;
	u8 screen_fade_g;
	u8 screen_fade_b;
	u8 screen_fade_factor;
	u8 fade_flags;
	u16 palbase;
	u8 layer_flags;
};

struct namcos23_model_data
{
	u16 model;
	u16 model2;
	s16 m[9];
	s32 v[3];
	float scaling;
	bool transpose;
	s32 light_vector[3];
};

struct namcos23_direct_data
{
	u16 d[28];
};

struct namcos23_immediate_data
{
	u32 type;
	u32 h;
	u32 pal;
	u32 zbias;
	u32 i[4];
	u32 u[4];
	u32 v[4];
	u32 x[4];
	u32 y[4];
	u32 z[4];
};

struct namcos23_sprite_data
{
	s16 xpos;
	s16 ypos;
	s16 xsize;
	s16 ysize;
	u8 rows;
	u8 cols;
	u8 linktype;
	u8 alpha;
	u16 code;
	u8 xflip;
	u8 yflip;
	u8 cz;
	u8 prioverchar;
	u32 zcoord;
	u8 color;
	u8 fade_enabled;
};

struct namcos23_render_entry
{
	int type;
	u16 absolute_priority;
	u16 model_blend_factor;
	u16 camera_power;
	u16 camera_ambient;
	u8 poly_fade_r;
	u8 poly_fade_g;
	u8 poly_fade_b;
	u8 poly_alpha_color;
	u8 poly_alpha_pen;
	u8 poly_alpha;
	u8 screen_fade_r;
	u8 screen_fade_g;
	u8 screen_fade_b;
	u8 screen_fade_factor;
	u8 fade_flags;

	union
	{
		namcos23_model_data model;
		namcos23_direct_data direct;
		namcos23_immediate_data immediate;
		namcos23_sprite_data sprite;
	};
};

struct namcos23_render_data
{
	running_machine *machine;
	u32 rgb;
	const pen_t *pens;
	bitmap_rgb32 *bitmap;
	bitmap_ind8 *primap;
	u32 flags;
	rgbaint_t polycolor;
	u16 model;
	bool direct;
	bool immediate;
	bool sprite;
	int tbase;
	u8 cmode;
	u16 cz_value;
	u8 cz_type;
	const u8 *sprite_source;
	u32 h;
	u32 type;
	u8 sprite_line_modulo;
	u8 sprite_xflip;
	u8 sprite_yflip;
	u8 fogfactor;
	bool pfade_enabled;
	u8 fadefactor;
	bool shade_enabled;
	bool alpha_enabled;
	bool blend_enabled;
	u8 alphafactor;
	u8 alpha;
	u8 poly_alpha_pen;
	u8 prioverchar;

	rgbaint_t fogcolor;
	rgbaint_t fadecolor;
	u32 (*texture_lookup)(running_machine &machine, const pen_t *pens, int penshift, int penmask, float x, float y, u8 &pen);
	bool (*stencil_lookup)(running_machine &machine, float x, float y);
};

class namcos23_state;

class namcos23_renderer : public poly_manager<float, namcos23_render_data, 4>
{
public:
	namcos23_renderer(namcos23_state &state);
	void render_flush(screen_device &screen, bitmap_rgb32 &bitmap);
	void render_scanline(s32 scanline, const extent_t& extent, const namcos23_render_data& object, int threadid);
	void render_sprite_scanline(s32 scanline, const extent_t& extent, const namcos23_render_data& object, int threadid);
	float* zBuffer() { return m_zBuffer; }

private:
	namcos23_state& m_state;
	float* m_zBuffer = nullptr;
};

typedef namcos23_renderer::vertex_t poly_vertex;

struct namcos23_poly_entry
{
	namcos23_render_data rd;
	int vertex_count;
	int zkey;
	c404_mixer_regs_t c404;
	poly_vertex pv[16];
};


struct c417_t
{
	u16 ram[0x10000];
	u16 adr;
	u32 pointrom_adr;
	bool test_mode;
};

struct c412_t
{
	u16 sdram_a[0x100000]; // Framebuffers, probably
	u16 sdram_b[0x100000];
	u16 sram[0x20000];     // Ram-based tiles for alpha-cutout drawing
	u16 pczram[0x200];     // PCZ Convert RAM
	u32 adr;
	u16 status_c;
};

struct c421_t
{
	u16 dram_a[0x40000];
	u16 dram_b[0x40000];
	u16 sram[0x8000];
	u32 adr;
};

struct c422_t
{
	s16 regs[0x10];
};

struct c361_t
{
	emu_timer *timer;
	int scanline;
};

struct c404_t
{
	u8 poly_fade_r;
	u8 poly_fade_g;
	u8 poly_fade_b;
	u8 fog_r;
	u8 fog_g;
	u8 fog_b;
	u32 bgcolor;
	u16 spot_factor;
	u8 poly_alpha_color;
	u8 poly_alpha_pen;
	u8 poly_alpha;
	u8 alpha_check12;
	u8 alpha_check13;
	u8 alpha_mask;
	u8 alpha_factor;
	u8 screen_fade_r;
	u8 screen_fade_g;
	u8 screen_fade_b;
	u8 screen_fade_factor;
	u8 fade_flags;
	u16 palbase;
	u8 layer_flags;
	u16 ram[0x400];
	u16 spritedata_idx;
	u64 rowscroll_frame;
	u16 rowscroll[480];
	u16 lastrow;
	u16 xscroll;
	u16 yscroll;
	struct
	{
		u16 d[4];
	} sprites[0x280];
};

struct c435_t
{
	u32 address;
	u32 size;
	u16 buffer[256];
	int buffer_pos;
	u16 direct_buf[28];
	u16 direct_vertex[6];
	u16 direct_vert_pos;
	u16 direct_buf_pos;
	bool direct_buf_nonempty;
	bool direct_buf_open;
	u16 pio_mode;
	u16 sprite_target;
	u16 spritedata[0x10000];
};

struct render_t
{
	std::unique_ptr<namcos23_renderer> polymgr;
	int cur;
	int poly_count;
	int count[2];
	namcos23_render_entry entries[2][RENDER_MAX_ENTRIES];
	namcos23_poly_entry polys[POLY_MAX_ENTRIES];
	namcos23_poly_entry *poly_order[POLY_MAX_ENTRIES];
};

class namcos23_state : public driver_device
{
public:
	namcos23_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_adc(*this, "subcpu:adc"),
		m_jvs(*this, "jvs"),
		m_rtc(*this, "rtc"),
		m_settings(*this, "namco_settings"),
		m_mainram(*this, "mainram"),
		m_shared_ram(*this, "shared_ram", 0x10000, ENDIANNESS_BIG),
		m_charram(*this, "charram"),
		m_textram(*this, "textram"),
		m_czattr(*this, "czattr", 0x10, ENDIANNESS_BIG),
		m_gfxdecode(*this, "gfxdecode"),
		m_lightx(*this, "LIGHTX"),
		m_lighty(*this, "LIGHTY"),
		m_p1(*this, "P1"),
		m_p2(*this, "P2"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_generic_paletteram_32(*this, "paletteram"),
		m_jvs_sense(jvs_port_device::sense::None),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void s23(machine_config &config);
	void timecrs2(machine_config &config);
	void downhill(machine_config &config);
	void panicprk(machine_config &config);

	render_t m_render;
	const u16 *m_tmlrom;
	const u8 *m_tmhrom;
	const u8 *m_texrom;
	const u8 *m_sprrom;
	const u16 *m_texram;
	u32 m_tileid_mask;
	u32 m_tile_mask;

	u16 *m_texture_tilemap;
	u8 *m_texture_tiledata;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void mips_base_map(address_map &map) ATTR_COLD;
	void mips_map(address_map &map) ATTR_COLD;
	void s23h8rwmap(address_map &map) ATTR_COLD;

	void c361_map(address_map &map, const u32 addr);
	void c404_map(address_map &map, const u32 addr);

	virtual void configure_jvs(device_jvs_interface &io);

	void irq_update_common(u32 cause);
	virtual void irq_update(u32 cause);
	void subcpu_irq1_update(int state);

	void textram_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void textchar_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void paletteram_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void sprites_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	u16 c404_ram_r(offs_t offset);
	void c404_ram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void c404_poly_fade_red_w(offs_t offset, u16 data);
	void c404_poly_fade_green_w(offs_t offset, u16 data);
	void c404_poly_fade_blue_w(offs_t offset, u16 data);
	void c404_fog_red_w(offs_t offset, u16 data);
	void c404_fog_green_w(offs_t offset, u16 data);
	void c404_fog_blue_w(offs_t offset, u16 data);
	void c404_bg_red_w(offs_t offset, u16 data);
	void c404_bg_green_w(offs_t offset, u16 data);
	void c404_bg_blue_w(offs_t offset, u16 data);
	void c404_spot_lsb_w(offs_t offset, u16 data);
	void c404_spot_msb_w(offs_t offset, u16 data);
	void c404_poly_alpha_color_w(offs_t offset, u16 data);
	void c404_poly_alpha_pen_w(offs_t offset, u16 data);
	void c404_poly_alpha_w(offs_t offset, u16 data);
	void c404_alpha_check12_w(offs_t offset, u16 data);
	void c404_alpha_check13_w(offs_t offset, u16 data);
	void c404_text_alpha_mask_w(offs_t offset, u16 data);
	void c404_text_alpha_factor_w(offs_t offset, u16 data);
	void c404_screen_fade_red_w(offs_t offset, u16 data);
	void c404_screen_fade_green_w(offs_t offset, u16 data);
	void c404_screen_fade_blue_w(offs_t offset, u16 data);
	void c404_screen_fade_factor_w(offs_t offset, u16 data);
	void c404_fade_flags_w(offs_t offset, u16 data);
	void c404_palette_base_w(offs_t offset, u16 data);
	void c404_layer_flags_w(offs_t offset, u16 data);

	virtual u16 c417_status_r();
	u16 c417_addr_r();
	void c417_addr_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void c417_ptrom_addr_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 c417_test_done_r();
	void c417_ptrom_addr_clear_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 c417_ram_r();
	void c417_ram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 c417_ptrom_msw_r();
	u16 c417_ptrom_lsw_r();
	void c417_irq_ack_w(offs_t offset, u16 data);

	u16 c412_ram_r(offs_t offset);
	void c412_ram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 c412_r(offs_t offset, u16 mem_mask = ~0);
	void c412_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	u16 c421_ram_r();
	void c421_ram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 c421_addr_msw_r();
	void c421_addr_msw_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 c421_addr_lsw_r();
	void c421_addr_lsw_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void direct_buf_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void ctl_leds_w(offs_t offset, u16 data);
	u16 ctl_status_r();
	u16 ctl_input1_r();
	void ctl_input1_w(offs_t offset, u16 data);
	u16 ctl_input2_r();
	void ctl_input2_w(offs_t offset, u16 data);
	void ctl_vbl_ack_w(offs_t offset, u16 data);
	void ctl_direct_poly_w(offs_t offset, u16 data);

	void c361_xscroll_w(offs_t offset, u16 data);
	void c361_yscroll_w(offs_t offset, u16 data);
	void c361_irq_scanline_w(offs_t offset, u16 data);
	u16 c361_vpos_r();
	u16 c361_vblank_r();
	TIMER_CALLBACK_MEMBER(c361_timer_cb);

	u16 c422_r(offs_t offset);
	void c422_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void mcuen_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	u32 c435_busy_flag_r();
	void c435_dma_addr_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void c435_dma_size_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void c435_dma_start_w(address_space &space, offs_t offset, u32 data, u32 mem_mask = ~0);
	void c435_clear_bufpos_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void c435_state_pio_w(u16 data);
	void c435_state_reset_w(u16 data);
	void c435_pio_w(offs_t offset, u16 data);
	void c435_sprite_w(u16 data);

	void c435_dma(address_space &space, u32 adr, u32 size);

	s32 *c435_getv(u16 id);
	s16 *c435_getm(u16 id);

	void c435_state_set_interrupt(const u16 *param);
	void c435_state_set_projection_matrix_line(const u16 *param);
	void c435_state_set(u16 type, const u16 *param);
	int c435_get_state_entry_size(u16 type);

	void c435_matrix_matrix_mul();
	void c435_matrix_vector_mul();
	void c435_matrix_matrix_immed_mul();
	void c435_matrix_vector_immed_mul();
	void c435_matrix_set();
	void c435_vector_set();
	void c435_state_set();
	void c435_scaling_set();
	void c435_model_blend_factor_set();
	void c435_absolute_priority_set();
	void c435_camera_power_set();
	void c435_camera_ambient_set();
	void c435_render();
	void c435_flush();

	void sharedram_sub_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 sharedram_sub_r(offs_t offset);

	void sub_interrupt_main_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 sub_comm_r(offs_t offset);
	void sub_comm_w(offs_t offset, u8 data);

	u8 mcu_p8_r();
	void mcu_p8_w(u8 data);
	u8 mcu_pa_r();
	void mcu_pa_w(u8 data);
	u8 mcu_pb_r();
	void mcu_pb_w(u8 data);
	u8 mcu_p6_r();
	void mcu_p6_w(u8 data);

	TILE_GET_INFO_MEMBER(text_tilemap_get_info);
	virtual u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(subcpu_scanline_on_tick);
	TIMER_CALLBACK_MEMBER(subcpu_scanline_off_tick);
	virtual void vblank(int state);

	u8 nthbyte(const u32 *pSource, int offs);
	u16 nthword(const u32 *pSource, int offs);
	float f24_to_f32(u32 v);

	void render_apply_transform(s32 xi, s32 yi, s32 zi, const namcos23_render_entry *re, float &x, float &y, float &z);
	void render_apply_matrot(s32 xi, s32 yi, s32 zi, const namcos23_render_entry *re, float &x, float &y, float &z);
	void render_project(poly_vertex &v);
	void render_model(const namcos23_render_entry *re);
	void render_direct_poly(const namcos23_render_entry *re);
	void render_sprite(const namcos23_render_entry *re);
	void render_sprite_tile(u32 code_offset, const namcos23_render_entry *re, int row, int col);
	void render_immediate(const namcos23_render_entry *re);
	virtual void render_run(screen_device &screen, bitmap_rgb32 &bitmap);

	void update_text_rowscroll();
	void apply_text_scroll();
	void mix_text_layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int prival);

	required_device<mips3_device> m_maincpu;
	required_device<h83002_device> m_subcpu;
	required_device<h8_adc_device> m_adc;
	required_device<jvs_port_device> m_jvs;
	required_device<rtc4543_device> m_rtc;
	required_device<namco_settings_device> m_settings;
	required_shared_ptr<u32> m_mainram;
	memory_share_creator<u32> m_shared_ram;
	required_shared_ptr<u32> m_charram;
	required_shared_ptr<u32> m_textram;
	memory_share_creator<u32> m_czattr;
	required_device<gfxdecode_device> m_gfxdecode;
	optional_ioport m_lightx;
	optional_ioport m_lighty;
	required_ioport m_p1;
	required_ioport m_p2;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<u32> m_generic_paletteram_32;

	c404_t m_c404;
	c361_t m_c361;
	c417_t m_c417;
	c412_t m_c412;
	c421_t m_c421;
	c422_t m_c422;
	c435_t m_c435;
	std::unique_ptr<bitmap_ind16> m_mix_bitmap;

	tilemap_t *m_bgtilemap;

	u8 m_jvs_sense;

	u32 m_main_irqcause;

	bool m_ctl_vbl_active;
	u8 m_ctl_led;
	u16 m_ctl_inp_buffer[2];

	bool m_subcpu_running;

	const u32 *m_ptrom;
	u32 m_ptrom_limit;

	emu_timer *m_subcpu_scanline_on_timer;
	emu_timer *m_subcpu_scanline_off_timer;

	u16 m_absolute_priority;
	u16 m_model_blend_factor;
	u16 m_camera_power;
	u16 m_camera_ambient;
	float m_proj_matrix[8*3];
	u8 m_proj_matrix_line;

	// There may only be 128 matrix and vector slots.
	// At 0x1e bytes per slot, rounded up to 0x20, that's 0x1000 to 0x2000 bytes.
	// That fits pretty much anywhere, including inside an IC.
	// Unknown right now if it's directly CPU-accessible. Command packets via DMA are probably more efficient.
	s16 m_matrices[256][9]; // Matrices are stored in signed 2.14 fixed point
	s32 m_vectors[256][3];  // Vectors are stored in signed 10.14 fixed point
	s32 m_light_vector[3];
	u16 m_scaling;
	s32 m_spv[3];
	s16 m_spm[9];

	int m_c361_irqnum;
	int m_c422_irqnum;
	int m_c435_irqnum;
	int m_vbl_irqnum;
	int m_sub_irqnum;
	int m_rs232_irqnum;

	u8 m_sub_port8;
	u8 m_sub_porta;
	u8 m_sub_portb;
	output_finder<8> m_lamps;
};

class gorgon_state : public namcos23_state
{
public:
	gorgon_state(const machine_config &mconfig, device_type type, const char *tag) :
		namcos23_state(mconfig, type, tag)
	{
	}

	void gorgon(machine_config &config);
	void finfurl(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void mips_map(address_map &map) ATTR_COLD;

	void nvram_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u16 czattr_r(offs_t offset, u16 mem_mask = ~0);
	void czattr_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 czram_r(offs_t offset, u16 mem_mask = ~0);
	void czram_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void c435_pio_mode_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	virtual u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;
	virtual void render_run(screen_device &screen, bitmap_rgb32 &bitmap) override;
	virtual void vblank(int state) override;

	void recalc_czram();

	std::unique_ptr<u16[]> m_banked_czram[4];
	std::unique_ptr<u8[]> m_recalc_czram[4];
	u32 m_cz_was_written[4];
};

class motoxgo_state : public namcos23_state
{
public:
	using namcos23_state::namcos23_state;

	void motoxgo(machine_config &config)
	{
		s23(config);
		m_jvs->set_default_option("namco_asca3a");
	}

protected:
	virtual void configure_jvs(device_jvs_interface &io) override
	{
		namcos23_state::configure_jvs(io);

		io.analog_input<4>().set_constant(0x200); // FREEZE/RELAY? < 0x200 = BANK ERR
		if (io.jvs())
		{
			io.jvs()->set_default_option("namco_amc");
			io.jvs()->set_option_machine_config("namco_amc", [](device_t *device)
			{
				auto &io = dynamic_cast<device_jvs_interface &>(*device);
				io.player<0>().set_constant(0);
				// motors?
				io.output().set_nop();
				io.analog_output<0>().set_nop();
				io.analog_output<1>().set_nop();
			});
		}
	}
};

class rapidrvr_state : public gorgon_state
{
public:
	rapidrvr_state(const machine_config &mconfig, device_type type, const char *tag) :
		gorgon_state(mconfig, type, tag),
		m_paddle(*this, "JVS_ANALOG_INPUT%u", 1U),
		m_output{},
		m_sensor{}
	{
	}

	void rapidrvr(machine_config &config)
	{
		gorgon(config);
		m_jvs->set_default_option("namco_asca1");
	}

protected:
	virtual void driver_start() override
	{
		gorgon_state::driver_start();

		m_sensor_timer = timer_alloc(FUNC(rapidrvr_state::sensor_timer_callback), this);
		m_sensor_timer->adjust(attotime::zero, 0, attotime::from_hz(200));

		save_item(NAME(m_sensor));
		save_item(NAME(m_output));
	}

	virtual void configure_jvs(device_jvs_interface &io) override
	{
		namcos23_state::configure_jvs(io);

		io.analog_input<0>().set(*this, FUNC(rapidrvr_state::paddle_r<0>)); // yaw
		io.analog_input<1>().set(*this, FUNC(rapidrvr_state::paddle_r<1>)); // pitch
		io.analog_input<2>().set(*this, FUNC(rapidrvr_state::sensor_r<0>)); // fr
		io.analog_input<3>().set(*this, FUNC(rapidrvr_state::sensor_r<1>)); // fl
		io.analog_input<4>().set(*this, FUNC(rapidrvr_state::sensor_r<2>)); // rr
		io.analog_input<5>().set(*this, FUNC(rapidrvr_state::sensor_r<3>)); // rl
		io.output().set(*this, FUNC(rapidrvr_state::output_w));
	}

	TIMER_CALLBACK_MEMBER(sensor_timer_callback)
	{
		for (int i = 0; i < 4; i++)
			if (BIT(m_output, 7 - i))
				m_sensor[i] = std::min(0x1100, m_sensor[i] + 0x280);
			else
				m_sensor[i] = std::max(0x40, m_sensor[i] - 0x280);
	}

	template<int N>
	uint16_t paddle_r()
	{
		return m_paddle[N]->read() - 0x8000;
	}

	template<int N>
	uint16_t sensor_r()
	{
		return (N == 1 || N == 2) ? -m_sensor[N] : m_sensor[N];
	}

	void output_w(offs_t offset, uint64_t data, uint64_t mem_mask)
	{
		COMBINE_DATA(&m_output);
	}

	emu_timer *m_sensor_timer;
	required_ioport_array<2> m_paddle;
	u64 m_output;
	u16 m_sensor[4];
};

class namcoss23_state : public namcos23_state
{
public:
	namcoss23_state(const machine_config &mconfig, device_type type, const char *tag) :
		namcos23_state(mconfig, type, tag)
	{ }

	void ss23(machine_config &config);
	void timecrs2v4a(machine_config &config);
	void _500gp(machine_config &config);
	void aking(machine_config &config);

protected:
	void mips_map(address_map &map) ATTR_COLD;

private:
	void c435_c361_ack_w(offs_t offset, u32 data);
};

class namcoss23_gmen_state : public namcoss23_state
{
public:
	namcoss23_gmen_state(const machine_config &mconfig, device_type type, const char *tag) :
		namcoss23_state(mconfig, type, tag),
		m_sh2(*this, "sh2"),
		m_vpx(*this, "vpx"),
		m_firewire(*this, "firewire"),
		m_sh2_shared(*this, "sh2_shared", 0x10000, ENDIANNESS_BIG),
		m_dsw(*this, "GMENDSW")
	{ }

	void gmen(machine_config &config);
	void gunwars(machine_config &config);
	void raceon(machine_config &config);
	void finfurl2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	u32 sh2_trigger_r();
	u32 sh2_shared_r(offs_t offset, u32 mem_mask = ~0);
	void sh2_shared_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 sh2_dsw_r(offs_t offset, u32 mem_mask = ~0);
	u32 mips_sh2_unk_r(offs_t offset, u32 mem_mask = ~0);
	u32 sh2_unk_r(offs_t offset, u32 mem_mask = ~0);
	void sh2_unk_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 sh2_unk6200000_r(offs_t offset, u32 mem_mask = ~0);
	u32 sh2_kludge_r();
	void vpx_i2c_sdao_w(int state);
	u8 vpx_i2c_r();
	void vpx_i2c_w(u8 data);

	TIMER_CALLBACK_MEMBER(sh2_irq_off);
	int m_sh2_irq;
	emu_timer *m_sh2_irq_timer;

private:
	void mips_map(address_map &map) ATTR_COLD;
	void sh2_map(address_map &map) ATTR_COLD;

	required_device<sh7604_device> m_sh2;
	required_device<vpx3220a_device> m_vpx;
	required_device<md8412b_device> m_firewire;
	memory_share_creator<u32> m_sh2_shared;
	required_ioport m_dsw;

	int m_vpx_sdao;
	u32 m_sh2_unk;
};

class crszone_state : public namcoss23_state
{
public:
	crszone_state(const machine_config &mconfig, device_type type, const char *tag) :
		namcoss23_state(mconfig, type, tag),
		m_acia(*this, "acia")
	{ }

	void crszone(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	void mips_map(address_map &map) ATTR_COLD;

	virtual void irq_update(u32 cause) override;

	virtual u16 c417_status_r() override;

	void irq_vbl_ack_w(offs_t offset, u32 data);
	u32 irq_lv3_status_r();
	u32 irq_lv5_status_r();
	u32 irq_lv6_status_r();

	void acia_irq_w(int state);
	u8 acia_r(offs_t offset);
	void acia_w(offs_t offset, u8 data);

	u32 c450_irq_status_r(offs_t offset, u32 mem_mask);
	void c450_dma_addr_w(address_space &space, offs_t offset, u32 data, u32 mem_mask = ~0);
	void c450_dma_size_w(address_space &space, offs_t offset, u32 data, u32 mem_mask = ~0);

	required_device<acia6850_device> m_acia;

	int m_c450_irqnum;
	int m_c451_irqnum;
};

u8 namcos23_state::nthbyte(const u32 *pSource, int offs)
{
	pSource += offs/4;
	return (pSource[0]<<((offs&3)*8))>>24;
}

u16 namcos23_state::nthword(const u32 *pSource, int offs)
{
	pSource += offs/2;
	return (pSource[0]<<((offs&1)*16))>>16;
}



/***************************************************************************

  Video

***************************************************************************/

namcos23_renderer::namcos23_renderer(namcos23_state &state)
	: poly_manager<float, namcos23_render_data, 4>(state.machine()),
		m_state(state)
{
}

// 3D hardware

s32 u32_to_s24(u32 v)
{
	return v & 0x800000 ? v | 0xff000000 : v & 0xffffff;
}

s32 u32_to_s10(u32 v)
{
	return v & 0x200 ? (0x3ff - (v & 0x3ff)) | 0xfffffe00 : v & 0x1ff;
}

float namcos23_state::f24_to_f32(u32 v)
{
	// 8 bits exponent, 16 mantissa
	// mantissa is 16-bits signed, 2-complement
	// value is m * 2**(e-46)
	// 1 is e=32, m=0x4000, -1 is e=31, m=0x8000

	// This code turns it into a standard float
	if (!(v & 0x0000ffff))
		return 0;

	u32 r = v & 0x8000 ? 0x80000000 : 0;
	u16 m = r ? -v : v;
	u8 e = (v >> 16) + 0x60;
	while(!(m & 0x8000))
	{
		m <<= 1;
		e--;
	}

	r = r | (e << 23) | ((m & 0x7fff) << 8);
	return *(float *)&r;
}

s32 *namcos23_state::c435_getv(u16 id)
{
	if (id == 0x8000)
		return m_light_vector;
	if (id >= 0x100)
	{
		memset(m_spv, 0, sizeof(m_spv));
		return m_spv;
	}
	return m_vectors[id];
}

s16 *namcos23_state::c435_getm(u16 id)
{
	if (id >= 0x100)
	{
		memset(m_spm, 0, sizeof(m_spm));
		return m_spm;
	}
	return m_matrices[id];
}

void namcos23_state::c435_state_set_interrupt(const u16 *param)
{
	if (param[0] & 1)
		irq_update(m_main_irqcause | MAIN_C435_IRQ);
	else
		irq_update(m_main_irqcause & ~MAIN_C435_IRQ);
}

void namcos23_state::c435_state_set_projection_matrix_line(const u16 *param)
{
	// timecrs2:
	//   sx = 640/2, sy = 480/2, t = tan(fov/2) (fov=45 degrees)
	//   line 1: 1 0 -(sx-a)/(sx/t) 0 -1  0 -(sx+a)/(sx/t) 0
	//   line 2: 0 1 -(sy-b)/(sx/t) 0  0 -1 -(sy+b)/(sx/t) 0
	//   line 3: 0 0 -1             c  0  0              0 sx/t

	std::ostringstream buf;
	buf << "projection matrix line:";
	for (int i = 0; i < 8; i++)
	{
		util::stream_format(buf, " %f", f24_to_f32((param[2 * i + 1] << 16) | param[2 * i + 2]));
		m_proj_matrix[m_proj_matrix_line * 8 + i] = f24_to_f32((param[2 * i + 1] << 16) | param[2 * i + 2]);
	}
	m_proj_matrix_line = (m_proj_matrix_line + 1) % 3;
	buf << "\n";
	LOGMASKED(LOG_PROJ_MAT, "%s: %s", machine().describe_context(), std::move(buf).str());

	std::ostringstream buf2;
	buf2 << "projection matrix line:";
	for (int i = 0; i < 8; i++)
	{
		util::stream_format(buf2, " %08x", (param[2 * i + 1] << 16) | param[2 * i + 2]);
	}
	buf2 << "\n";
	LOGMASKED(LOG_PROJ_MAT, "%s: %s", machine().describe_context(), std::move(buf2).str());
}

void namcos23_state::c435_state_reset_w(u16 data)
{
	m_c435.buffer_pos = 0;
}

void namcos23_state::c435_state_pio_w(u16 data)
{
	m_c435.buffer[m_c435.buffer_pos++] = data;
	int psize = c435_get_state_entry_size(m_c435.buffer[0]);
	if (m_c435.buffer_pos < psize+1)
		return;
	c435_state_set(m_c435.buffer[0], m_c435.buffer + 1);

	m_c435.buffer_pos = 0;
}

int namcos23_state::c435_get_state_entry_size(u16 type)
{
	switch (type)
	{
	case 0x0000: return 43;
	case 0x0001: return 1;
	case 0x0002: return 53;
	case 0x0009: return 19;
	case 0x000a: return 47;
	case 0x0042: return 41;
	case 0x0046: return 13;
	case 0x00c0: return 33;
	case 0x00c6: return 13;
	case 0x00c8: return 17;
	default:
		LOGMASKED(LOG_3D_STATE_UNK, "%s: WARNING: Unknown size for state type %04x\n", machine().describe_context(), type);
		return -1;
	}
}

static void transpose_matrix(s16 *m1)
{
	s16 m2[9];
	memcpy(m2, m1, sizeof(m2));
	m1[0] = m2[0];  m1[1] = m2[3];  m1[2] = m2[6];
	m1[3] = m2[1];  m1[4] = m2[4];  m1[5] = m2[7];
	m1[6] = m2[2];  m1[7] = m2[5];  m1[8] = m2[8];
}

void namcos23_state::c435_matrix_matrix_mul() // 0.0
{
	if ((m_c435.buffer[0] & 0xf) != 4)
	{
		LOGMASKED(LOG_MATRIX_ERR, "%s: WARNING: c435_matrix_matrix_mul with size %d\n", machine().describe_context(), m_c435.buffer[0] & 0xf);
		return;
	}
	bool transpose = BIT(m_c435.buffer[0], 10);
	bool identity = BIT(m_c435.buffer[0], 8);
	if ((m_c435.buffer[0] & ~0x500) != 0x0004)
		LOGMASKED(LOG_MATRIX_UNK, "%s: WARNING: c435_matrix_matrix_mul header %04x\n", machine().describe_context(), m_c435.buffer[0]);
	if (m_c435.buffer[3] != 0xffff)
		LOGMASKED(LOG_MATRIX_UNK, "%s: WARNING: c435_matrix_matrix_mul with +2=%04x\n", machine().describe_context(), m_c435.buffer[3]);

	LOGMASKED(LOG_MATRIX_INFO, "c435_matrix_matrix_mul (%04x): Matrix %d = Matrix %d * Matrix %d\n", m_c435.buffer[0], m_c435.buffer[1], m_c435.buffer[2], m_c435.buffer[4]);

	s16 *t = c435_getm(m_c435.buffer[1]);
	s16 m1[9];
	s16 m2[9];

	memcpy(m2, c435_getm(m_c435.buffer[2]), sizeof(s16) * 9);
	if (transpose)
		transpose_matrix(m2);
	if (identity)
	{
		memset(m1, 0, sizeof(s16) * 9);
		m1[0] = m1[4] = m1[8] = 0x4000;
	}
	else
	{
		memcpy(m1, c435_getm(m_c435.buffer[4]), sizeof(s16) * 9);
	}
	t[0] = s16((m1[0] * m2[0] + m1[1] * m2[1] + m1[2] * m2[2]) >> 14);
	t[1] = s16((m1[0] * m2[3] + m1[1] * m2[4] + m1[2] * m2[5]) >> 14);
	t[2] = s16((m1[0] * m2[6] + m1[1] * m2[7] + m1[2] * m2[8]) >> 14);
	t[3] = s16((m1[3] * m2[0] + m1[4] * m2[1] + m1[5] * m2[2]) >> 14);
	t[4] = s16((m1[3] * m2[3] + m1[4] * m2[4] + m1[5] * m2[5]) >> 14);
	t[5] = s16((m1[3] * m2[6] + m1[4] * m2[7] + m1[5] * m2[8]) >> 14);
	t[6] = s16((m1[6] * m2[0] + m1[7] * m2[1] + m1[8] * m2[2]) >> 14);
	t[7] = s16((m1[6] * m2[3] + m1[7] * m2[4] + m1[8] * m2[5]) >> 14);
	t[8] = s16((m1[6] * m2[6] + m1[7] * m2[7] + m1[8] * m2[8]) >> 14);

	LOGMASKED(LOG_MATRIX_INFO, "result: %04x    %04x    %04x\n", (u16)t[0], (u16)t[1], (u16)t[2]);
	LOGMASKED(LOG_MATRIX_INFO, "        %04x    %04x    %04x\n", (u16)t[3], (u16)t[4], (u16)t[5]);
	LOGMASKED(LOG_MATRIX_INFO, "        %04x    %04x    %04x\n", (u16)t[6], (u16)t[7], (u16)t[8]);
}

void namcos23_state::c435_matrix_vector_mul() // 0.1
{
	if ((m_c435.buffer[0] & 0xf) != 4)
	{
		LOGMASKED(LOG_VEC_ERR, "%s: WARNING: c435_matrix_vector_mul with size %d\n", machine().describe_context(), m_c435.buffer[0] & 0xf);
		return;
	}

	bool extra_logging = false;
	if (m_c435.buffer[0] != 0x0814 && m_c435.buffer[0] != 0x1014 && m_c435.buffer[0] != 0x0414)
	{
		LOGMASKED(LOG_VEC_UNK, "%s: WARNING: c435_matrix_vector_mul header %04x %04x %04x %04x %04x\n", machine().describe_context(), m_c435.buffer[0], m_c435.buffer[1], m_c435.buffer[2], m_c435.buffer[3], m_c435.buffer[4]);
		extra_logging = true;
	}

	s32 *t = c435_getv(m_c435.buffer[1]);
	s16 m[9];
	s32 v[3];
	memcpy(m, c435_getm(m_c435.buffer[2]), sizeof(s16) * 9);
	memcpy(v, c435_getv(m_c435.buffer[4]), sizeof(s32) * 3);

	if (BIT(m_c435.buffer[0], 10))
		transpose_matrix(m);

	if (m_c435.buffer[3] != 0xffff)
	{
		s32 vt[3];
		memcpy(vt, c435_getv(m_c435.buffer[3]), sizeof(s32) * 3);

		LOGMASKED(LOG_MATRIX_INFO, "c435_matrix_vector_mul (%04x): Vector %d = Matrix %d * Vector %d + Vector %d\n", m_c435.buffer[0], m_c435.buffer[1], m_c435.buffer[2], m_c435.buffer[4], m_c435.buffer[3]);
		if (BIT(m_c435.buffer[0], 12))
		{
			t[0] = s32((m[0] * s64(v[0]) + m[1] * s64(v[1]) + m[2] * s64(v[2])) >> 14) - vt[0];
			t[1] = s32((m[3] * s64(v[0]) + m[4] * s64(v[1]) + m[5] * s64(v[2])) >> 14) - vt[1];
			t[2] = s32((m[6] * s64(v[0]) + m[7] * s64(v[1]) + m[8] * s64(v[2])) >> 14) - vt[2];
		}
		else
		{
			t[0] = s32((m[0] * s64(v[0]) + m[1] * s64(v[1]) + m[2] * s64(v[2])) >> 14) + vt[0];
			t[1] = s32((m[3] * s64(v[0]) + m[4] * s64(v[1]) + m[5] * s64(v[2])) >> 14) + vt[1];
			t[2] = s32((m[6] * s64(v[0]) + m[7] * s64(v[1]) + m[8] * s64(v[2])) >> 14) + vt[2];
		}
		if (extra_logging)
		{
			LOGMASKED(LOG_MATRIX_UNK, "    %04x %04x %04x   %08x   %08x\n", m[0], m[1], m[2], (u32)v[0], (u32)vt[0]);
			LOGMASKED(LOG_MATRIX_UNK, "    %04x %04x %04x * %08x + %08x\n", m[3], m[4], m[5], (u32)v[1], (u32)vt[1]);
			LOGMASKED(LOG_MATRIX_UNK, "    %04x %04x %04x   %08x   %08x\n", m[6], m[7], m[8], (u32)v[2], (u32)vt[2]);
		}
		LOGMASKED(LOG_MATRIX_INFO, "result: %08x %08x %08x\n", (u32)t[0], (u32)t[1], (u32)t[2]);
	}
	else
	{
		LOGMASKED(LOG_MATRIX_INFO, "c435_matrix_vector_mul (%04x): Vector %d = Matrix %d * Vector %d\n", m_c435.buffer[0], m_c435.buffer[1], m_c435.buffer[2], m_c435.buffer[4]);

		t[0] = s32((m[0]*s64(v[0]) + m[1]*s64(v[1]) + m[2]*s64(v[2])) >> 14);
		t[1] = s32((m[3]*s64(v[0]) + m[4]*s64(v[1]) + m[5]*s64(v[2])) >> 14);
		t[2] = s32((m[6]*s64(v[0]) + m[7]*s64(v[1]) + m[8]*s64(v[2])) >> 14);

		if (extra_logging)
		{
			LOGMASKED(LOG_MATRIX_UNK, "    %04x %04x %04x   %08x\n", m[0], m[1], m[2], (u32)v[0]);
			LOGMASKED(LOG_MATRIX_UNK, "    %04x %04x %04x * %08x\n", m[3], m[4], m[5], (u32)v[1]);
			LOGMASKED(LOG_MATRIX_UNK, "    %04x %04x %04x   %08x\n", m[6], m[7], m[8], (u32)v[2]);
		}
		LOGMASKED(LOG_MATRIX_INFO, "result: %08x %08x %08x\n", (u32)t[0], (u32)t[1], (u32)t[2]);
	}
}

void namcos23_state::c435_matrix_matrix_immed_mul() // 0.2
{
	if ((m_c435.buffer[0] & 0xf) != 12)
	{
		LOGMASKED(LOG_MATRIX_ERR, "%s: WARNING: c435_matrix_matrix_immed_mul with size %d\n", machine().describe_context(), m_c435.buffer[0] & 0xf);
		return;
	}
	bool transpose = BIT(m_c435.buffer[0], 10);
	if ((m_c435.buffer[0] & ~0x400) != 0x000c)
		LOGMASKED(LOG_MATRIX_UNK, "%s: WARNING: c435_matrix_matrix_immed_mul header %04x\n", machine().describe_context(), m_c435.buffer[0]);
	if (m_c435.buffer[3] != 0xffff)
		LOGMASKED(LOG_MATRIX_UNK, "%s: WARNING: c435_matrix_matrix_immed_mul with +2=%04x\n", machine().describe_context(), m_c435.buffer[3]);

	LOGMASKED(LOG_MATRIX_INFO, "c435_matrix_matrix_immed_mul (%04x): Matrix %d = Immediate Matrix * Matrix %d\n", m_c435.buffer[0], m_c435.buffer[1], m_c435.buffer[2]);
	LOGMASKED(LOG_MATRIX_INFO, "    %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x\n",
		m_c435.buffer[0], m_c435.buffer[1], m_c435.buffer[2], m_c435.buffer[3],
		m_c435.buffer[4], m_c435.buffer[5], m_c435.buffer[6],
		m_c435.buffer[7], m_c435.buffer[8], m_c435.buffer[9],
		m_c435.buffer[10], m_c435.buffer[11], m_c435.buffer[12]);

	s16 *t = c435_getm(m_c435.buffer[1]);
	s16 m1[9];
	s16 m2[9];

	memcpy(m1, &m_c435.buffer[4], sizeof(s16) * 9);
	memcpy(m2, c435_getm(m_c435.buffer[2]), sizeof(s16) * 9);
	LOGMASKED(LOG_MATRIX_INFO, "    m1: %04x    %04x    %04x\n",   (u16)m1[0], (u16)m1[1], (u16)m1[2]);
	LOGMASKED(LOG_MATRIX_INFO, "        %04x    %04x    %04x\n",   (u16)m1[3], (u16)m1[4], (u16)m1[5]);
	LOGMASKED(LOG_MATRIX_INFO, "        %04x    %04x    %04x\n\n", (u16)m1[6], (u16)m1[7], (u16)m1[8]);
	LOGMASKED(LOG_MATRIX_INFO, "    m2: %04x    %04x    %04x\n",   (u16)m2[0], (u16)m2[1], (u16)m2[2]);
	LOGMASKED(LOG_MATRIX_INFO, "        %04x    %04x    %04x\n",   (u16)m2[3], (u16)m2[4], (u16)m2[5]);
	LOGMASKED(LOG_MATRIX_INFO, "        %04x    %04x    %04x\n\n", (u16)m2[6], (u16)m2[7], (u16)m2[8]);

	if (transpose)
		transpose_matrix(m2);
	t[0] = s16((m1[0] * m2[0] + m1[1] * m2[3] + m1[2] * m2[6]) >> 14);
	t[1] = s16((m1[0] * m2[1] + m1[1] * m2[4] + m1[2] * m2[7]) >> 14);
	t[2] = s16((m1[0] * m2[2] + m1[1] * m2[5] + m1[2] * m2[8]) >> 14);

	t[3] = s16((m1[3] * m2[0] + m1[4] * m2[3] + m1[5] * m2[6]) >> 14);
	t[4] = s16((m1[3] * m2[1] + m1[4] * m2[4] + m1[5] * m2[7]) >> 14);
	t[5] = s16((m1[3] * m2[2] + m1[4] * m2[5] + m1[5] * m2[8]) >> 14);

	t[6] = s16((m1[6] * m2[0] + m1[7] * m2[3] + m1[8] * m2[6]) >> 14);
	t[7] = s16((m1[6] * m2[1] + m1[7] * m2[4] + m1[8] * m2[7]) >> 14);
	t[8] = s16((m1[6] * m2[2] + m1[7] * m2[5] + m1[8] * m2[8]) >> 14);

	LOGMASKED(LOG_MATRIX_INFO, "result: %04x    %04x    %04x\n", (u16)t[0], (u16)t[1], (u16)t[2]);
	LOGMASKED(LOG_MATRIX_INFO, "        %04x    %04x    %04x\n", (u16)t[3], (u16)t[4], (u16)t[5]);
	LOGMASKED(LOG_MATRIX_INFO, "        %04x    %04x    %04x\n", (u16)t[6], (u16)t[7], (u16)t[8]);
}

void namcos23_state::c435_matrix_vector_immed_mul() // 0.3
{
	if ((m_c435.buffer[0] & 0xf) != 9)
	{
		LOGMASKED(LOG_VEC_ERR, "%s: WARNING: c435_matrix_vector_immed_mul with size %d\n", machine().describe_context(), m_c435.buffer[0] & 0xf);
		return;
	}

	bool extra_logging = false;
	if (m_c435.buffer[0] != 0x0839 && m_c435.buffer[0] != 0x1039 && m_c435.buffer[0] != 0x0439)
	{
		LOGMASKED(LOG_VEC_UNK, "%s: WARNING: c435_matrix_vector_immed_mul header %04x %04x %04x %04x\n", machine().describe_context(), m_c435.buffer[0], m_c435.buffer[1], m_c435.buffer[2], m_c435.buffer[3]);
		extra_logging = true;
	}

	s32 *t = c435_getv(m_c435.buffer[1]);
	s16 m[9];
	s32 v[3];
	memcpy(m, c435_getm(m_c435.buffer[2]), sizeof(s16) * 9);
	v[0] = ((s32)m_c435.buffer[4] << 16) | (u16)m_c435.buffer[5];
	v[1] = ((s32)m_c435.buffer[6] << 16) | (u16)m_c435.buffer[7];
	v[2] = ((s32)m_c435.buffer[8] << 16) | (u16)m_c435.buffer[9];

	if (BIT(m_c435.buffer[0], 10))
		transpose_matrix(m);

	if (m_c435.buffer[3] != 0xffff)
	{
		s32 vt[3];
		memcpy(vt, c435_getv(m_c435.buffer[3]), sizeof(s32) * 3);

		LOGMASKED(LOG_MATRIX_INFO, "c435_matrix_vector_immed_mul (%04x): Vector %d = Matrix %d * Immediate Vector + Vector %d\n", m_c435.buffer[0], m_c435.buffer[1], m_c435.buffer[2], m_c435.buffer[3]);
		if (BIT(m_c435.buffer[0], 12))
		{
			t[0] = s32((m[0] * s64(v[0]) + m[1] * s64(v[1]) + m[2] * s64(v[2])) >> 14) - vt[0];
			t[1] = s32((m[3] * s64(v[0]) + m[4] * s64(v[1]) + m[5] * s64(v[2])) >> 14) - vt[1];
			t[2] = s32((m[6] * s64(v[0]) + m[7] * s64(v[1]) + m[8] * s64(v[2])) >> 14) - vt[2];
		}
		else
		{
			t[0] = s32((m[0] * s64(v[0]) + m[1] * s64(v[1]) + m[2] * s64(v[2])) >> 14) + vt[0];
			t[1] = s32((m[3] * s64(v[0]) + m[4] * s64(v[1]) + m[5] * s64(v[2])) >> 14) + vt[1];
			t[2] = s32((m[6] * s64(v[0]) + m[7] * s64(v[1]) + m[8] * s64(v[2])) >> 14) + vt[2];
		}
		if (extra_logging)
		{
			LOGMASKED(LOG_MATRIX_UNK, "    %04x %04x %04x   %08x   %08x\n", m[0], m[1], m[2], (u32)v[0], (u32)vt[0]);
			LOGMASKED(LOG_MATRIX_UNK, "    %04x %04x %04x * %08x + %08x\n", m[3], m[4], m[5], (u32)v[1], (u32)vt[1]);
			LOGMASKED(LOG_MATRIX_UNK, "    %04x %04x %04x   %08x   %08x\n", m[6], m[7], m[8], (u32)v[2], (u32)vt[2]);
		}
		LOGMASKED(LOG_MATRIX_INFO, "result: %08x %08x %08x\n", (u32)t[0], (u32)t[1], (u32)t[2]);
	}
	else
	{
		LOGMASKED(LOG_MATRIX_INFO, "c435_matrix_vector_immed_mul (%04x): Vector %d = Matrix %d * Immediate Vector\n", m_c435.buffer[0], m_c435.buffer[1], m_c435.buffer[2]);

		t[0] = s32((m[0] * s64(v[0]) + m[1] * s64(v[1]) + m[2] * s64(v[2])) >> 14);
		t[1] = s32((m[3] * s64(v[0]) + m[4] * s64(v[1]) + m[5] * s64(v[2])) >> 14);
		t[2] = s32((m[6] * s64(v[0]) + m[7] * s64(v[1]) + m[8] * s64(v[2])) >> 14);

		if (extra_logging)
		{
			LOGMASKED(LOG_MATRIX_UNK, "    %04x %04x %04x   %08x\n", m[0], m[1], m[2], (u32)v[0]);
			LOGMASKED(LOG_MATRIX_UNK, "    %04x %04x %04x * %08x\n", m[3], m[4], m[5], (u32)v[1]);
			LOGMASKED(LOG_MATRIX_UNK, "    %04x %04x %04x   %08x\n", m[6], m[7], m[8], (u32)v[2]);
		}
		LOGMASKED(LOG_MATRIX_INFO, "result: %08x %08x %08x\n", (u32)t[0], (u32)t[1], (u32)t[2]);
	}
}

void namcos23_state::c435_matrix_set() // 0.4
{
	if ((m_c435.buffer[0] & 0xf) != 10)
	{
		LOGMASKED(LOG_MATRIX_ERR, "%s: WARNING: c435_matrix_set with size %d\n", machine().describe_context(), m_c435.buffer[0] & 0xf);
		return;
	}

	if (m_c435.buffer[0] != 0x004a)
		LOGMASKED(LOG_MATRIX_UNK, "%s: WARNING: c435_matrix_set header %04x\n", machine().describe_context(), m_c435.buffer[0]);

	s16 *t = c435_getm(m_c435.buffer[1]);
	for (int i = 0; i < 9; i++)
		t[i] = m_c435.buffer[i+2];
	LOGMASKED(LOG_MATRIX_INFO, "c435_matrix_set (%04x): Matrix %d:\n", m_c435.buffer[0], m_c435.buffer[1]);
	LOGMASKED(LOG_MATRIX_INFO, "    %04x %04x %04x\n", t[0], t[1], t[2]);
	LOGMASKED(LOG_MATRIX_INFO, "    %04x %04x %04x\n", t[3], t[4], t[5]);
	LOGMASKED(LOG_MATRIX_INFO, "    %04x %04x %04x\n", t[6], t[7], t[8]);
}

void namcos23_state::c435_vector_set() // 0.5
{
	if ((m_c435.buffer[0] & 0xf) != 7)
	{
		LOGMASKED(LOG_VEC_ERR, "%s: WARNING: c435_vector_set with size %d\n", machine().describe_context(), m_c435.buffer[0] & 0xf);
		return;
	}
	if (m_c435.buffer[0] != 0x057)
		LOGMASKED(LOG_VEC_UNK, "%s: WARNING: c435_vector_set header %04x\n", machine().describe_context(), m_c435.buffer[0]);

	s32 *t = c435_getv(m_c435.buffer[1]);
	for (int i = 0; i < 3; i++)
		t[i] = u32_to_s24((m_c435.buffer[2 * i + 2] << 16) | m_c435.buffer[2 * i + 3]);

	LOGMASKED(LOG_VEC_INFO, "c435_vector_set (%04x): Vector %d = %08x %08x %08x\n", m_c435.buffer[0], m_c435.buffer[1], t[0], t[1], t[2]);
}

void namcos23_state::c435_scaling_set() // 4.4
{
	if ((m_c435.buffer[0] & 0xff) != 1)
	{
		LOGMASKED(LOG_VEC_ERR, "%s: WARNING: c435_scaling_set with size %d\n", machine().describe_context(), m_c435.buffer[0] & 0xff);
		return;
	}

	LOGMASKED(LOG_MATRIX_INFO, "c435_scaling_set (%04x): %04x\n", m_c435.buffer[0], m_c435.buffer[1]);
	m_scaling = m_c435.buffer[1];
}

void namcos23_state::c435_state_set(u16 type, const u16 *param)
{
	LOGMASKED(LOG_3D_STATE_UNK, "%s: c435_state_set, type %04x, header %04x\n", machine().describe_context(), type, m_c435.buffer[0]);
	switch (type)
	{
	case 0x0000:
	{
		render_t &render = m_render;
		namcos23_render_entry *re = render.entries[render.cur] + render.count[render.cur];
		re->type = IMMEDIATE;
		re->poly_fade_r = m_c404.poly_fade_r;
		re->poly_fade_g = m_c404.poly_fade_g;
		re->poly_fade_b = m_c404.poly_fade_b;
		re->poly_alpha_color = m_c404.poly_alpha_color;
		re->poly_alpha_pen = m_c404.poly_alpha_pen;
		re->poly_alpha = m_c404.poly_alpha;
		re->screen_fade_r = m_c404.screen_fade_r;
		re->screen_fade_g = m_c404.screen_fade_g;
		re->screen_fade_b = m_c404.screen_fade_b;
		re->screen_fade_factor = m_c404.screen_fade_factor;
		re->fade_flags = m_c404.fade_flags;
		re->absolute_priority = m_absolute_priority;
		re->model_blend_factor = 0;
		re->camera_power = m_camera_power;
		re->camera_ambient = m_camera_ambient;
		if (m_c435.buffer[0] == 0x4f38)
		{
			re->immediate.type  =  param[ 0];
			re->immediate.h     = (param[ 1] << 16) | param[ 2];
			re->immediate.pal   = (param[ 3] << 16) | param[ 4];
			re->immediate.zbias = (param[ 5] << 16) | param[ 6];
			re->immediate.i[0]  = (param[ 7] << 16) | param[ 8];
			re->immediate.i[1]  = (param[ 9] << 16) | param[10];
			re->immediate.i[2]  = (param[11] << 16) | param[12];
			re->immediate.i[3]  = (param[13] << 16) | param[14];
			re->immediate.u[0]  = (param[15] << 16) | param[16];
			re->immediate.v[0]  = (param[17] << 16) | param[18];
			re->immediate.u[1]  = (param[19] << 16) | param[20];
			re->immediate.v[1]  = (param[21] << 16) | param[22];
			re->immediate.u[2]  = (param[23] << 16) | param[24];
			re->immediate.v[2]  = (param[25] << 16) | param[26];
			re->immediate.u[3]  = (param[27] << 16) | param[28];
			re->immediate.v[3]  = (param[29] << 16) | param[30];
			re->immediate.x[0]  = (param[31] << 16) | param[32];
			re->immediate.y[0]  = (param[33] << 16) | param[34];
			re->immediate.z[0]  = (param[35] << 16) | param[36];
			re->immediate.x[1]  = (param[37] << 16) | param[38];
			re->immediate.y[1]  = (param[39] << 16) | param[40];
			re->immediate.z[1]  = (param[41] << 16) | param[42];
			re->immediate.x[2]  = (param[43] << 16) | param[44];
			re->immediate.y[2]  = (param[45] << 16) | param[46];
			re->immediate.z[2]  = (param[47] << 16) | param[48];
			re->immediate.x[3]  = (param[49] << 16) | param[50];
			re->immediate.y[3]  = (param[51] << 16) | param[52];
			re->immediate.z[3]  = (param[53] << 16) | param[54];
		}
		else
		{
			re->immediate.type  =  param[ 0];
			re->immediate.h     = (param[ 1] << 16) | param[ 2];
			re->immediate.pal   = (param[ 3] << 16) | param[ 4];
			re->immediate.zbias = (param[ 5] << 16) | param[ 6];
			re->immediate.i[0]  = (param[ 7] << 16) | param[ 8];
			re->immediate.i[1]  = (param[ 9] << 16) | param[10];
			re->immediate.i[2]  = (param[11] << 16) | param[12];
			re->immediate.u[0]  = (param[13] << 16) | param[14];
			re->immediate.v[0]  = (param[15] << 16) | param[16];
			re->immediate.u[1]  = (param[17] << 16) | param[18];
			re->immediate.v[1]  = (param[19] << 16) | param[20];
			re->immediate.u[2]  = (param[21] << 16) | param[22];
			re->immediate.v[2]  = (param[23] << 16) | param[24];
			re->immediate.x[0]  = (param[25] << 16) | param[26];
			re->immediate.y[0]  = (param[27] << 16) | param[28];
			re->immediate.z[0]  = (param[29] << 16) | param[30];
			re->immediate.x[1]  = (param[31] << 16) | param[32];
			re->immediate.y[1]  = (param[33] << 16) | param[34];
			re->immediate.z[1]  = (param[35] << 16) | param[36];
			re->immediate.x[2]  = (param[37] << 16) | param[38];
			re->immediate.y[2]  = (param[39] << 16) | param[40];
			re->immediate.z[2]  = (param[41] << 16) | param[42];
		}
		render.count[render.cur]++;
		break;
	}
	case 0x0002:
	{
		render_t &render = m_render;
		namcos23_render_entry *re = render.entries[render.cur] + render.count[render.cur];
		re->poly_fade_r = m_c404.poly_fade_r;
		re->poly_fade_g = m_c404.poly_fade_g;
		re->poly_fade_b = m_c404.poly_fade_b;
		re->poly_alpha_color = m_c404.poly_alpha_color;
		re->poly_alpha_pen = m_c404.poly_alpha_pen;
		re->poly_alpha = m_c404.poly_alpha;
		re->screen_fade_r = m_c404.screen_fade_r;
		re->screen_fade_g = m_c404.screen_fade_g;
		re->screen_fade_b = m_c404.screen_fade_b;
		re->screen_fade_factor = m_c404.screen_fade_factor;
		re->fade_flags = m_c404.fade_flags;
		re->type = IMMEDIATE;
		re->absolute_priority = m_absolute_priority;
		re->model_blend_factor = 0;
		re->camera_power = m_camera_power;
		re->camera_ambient = m_camera_ambient;
		/*
		3-e0: 1110 0000, has shade+tex+pos
		3-a0: 1010 0000, has tex+pos
		[:] Word 00: 000a
		[:] Word 01: 34a0       type
		[:] Word 02: 00003242   h
		[:] Word 04: 0040ff00   pal?
		[:] Word 06: 00000000   zbias?
		[:] Word 08: 0000000f   u0
		[:] Word 0a: 0000060f   v0
		[:] Word 0c: 0000000f   u1
		[:] Word 0e: 00000600   v1
		[:] Word 10: 00000000   u2
		[:] Word 12: 00000600   v2
		[:] Word 14: 00000000   u3
		[:] Word 16: 0000060f   v3
		[:] Word 18: 000000c8   x0
		[:] Word 1a: ffffff38   y0
		[:] Word 1c: 00000500   z0
		[:] Word 1e: ffffff38   x1
		[:] Word 20: ffffff38   y1
		[:] Word 22: 00000500   z1
		[:] Word 24: ffffff38   x2
		[:] Word 26: 000000c8   y2
		[:] Word 28: 00000500   z2
		[:] Word 2a: 000000c8   x3
		[:] Word 2c: 000000c8   y3
		[:] Word 2e: 00000500   z3
		*/
		re->immediate.type  =  param[ 0];
		re->immediate.h     = (param[ 1] << 16) | param[ 2];
		re->immediate.pal   = (param[ 3] << 16) | param[ 4];
		re->immediate.zbias = 0;
		re->immediate.i[0]  = (param[ 5] << 16) | param[ 6];
		re->immediate.i[1]  = (param[ 7] << 16) | param[ 8];
		re->immediate.i[2]  = (param[ 9] << 16) | param[10];
		re->immediate.i[3]  = (param[11] << 16) | param[12];
		re->immediate.u[0]  = (param[13] << 16) | param[14];
		re->immediate.v[0]  = (param[15] << 16) | param[16];
		re->immediate.u[1]  = (param[17] << 16) | param[18];
		re->immediate.v[1]  = (param[19] << 16) | param[20];
		re->immediate.u[2]  = (param[21] << 16) | param[22];
		re->immediate.v[2]  = (param[23] << 16) | param[24];
		re->immediate.u[3]  = (param[25] << 16) | param[26];
		re->immediate.v[3]  = (param[27] << 16) | param[28];
		re->immediate.x[0]  = (param[29] << 16) | param[30];
		re->immediate.y[0]  = (param[31] << 16) | param[32];
		re->immediate.z[0]  = (param[33] << 16) | param[34];
		re->immediate.x[1]  = (param[35] << 16) | param[36];
		re->immediate.y[1]  = (param[37] << 16) | param[38];
		re->immediate.z[1]  = (param[39] << 16) | param[40];
		re->immediate.x[2]  = (param[41] << 16) | param[42];
		re->immediate.y[2]  = (param[43] << 16) | param[44];
		re->immediate.z[2]  = (param[45] << 16) | param[46];
		re->immediate.x[3]  = (param[47] << 16) | param[48];
		re->immediate.y[3]  = (param[49] << 16) | param[50];
		re->immediate.z[3]  = (param[51] << 16) | param[52];
		render.count[render.cur]++;
		break;
	}
	case 0x000a:
	{
		render_t &render = m_render;
		namcos23_render_entry *re = render.entries[render.cur] + render.count[render.cur];
		re->poly_fade_r = m_c404.poly_fade_r;
		re->poly_fade_g = m_c404.poly_fade_g;
		re->poly_fade_b = m_c404.poly_fade_b;
		re->poly_alpha_color = m_c404.poly_alpha_color;
		re->poly_alpha_pen = m_c404.poly_alpha_pen;
		re->poly_alpha = m_c404.poly_alpha;
		re->screen_fade_r = m_c404.screen_fade_r;
		re->screen_fade_g = m_c404.screen_fade_g;
		re->screen_fade_b = m_c404.screen_fade_b;
		re->screen_fade_factor = m_c404.screen_fade_factor;
		re->fade_flags = m_c404.fade_flags;
		re->type = IMMEDIATE;
		re->absolute_priority = m_absolute_priority;
		re->model_blend_factor = 0;
		re->camera_power = m_camera_power;
		re->camera_ambient = m_camera_ambient;
		re->immediate.type  =  param[ 0];
		re->immediate.h     = (param[ 1] << 16) | param[ 2];
		re->immediate.pal   = (param[ 3] << 16) | param[ 4];
		re->immediate.zbias = (param[ 5] << 16) | param[ 6];
		re->immediate.u[0]  = (param[ 7] << 16) | param[ 8];
		re->immediate.v[0]  = (param[ 9] << 16) | param[10];
		re->immediate.u[1]  = (param[11] << 16) | param[12];
		re->immediate.v[1]  = (param[13] << 16) | param[14];
		re->immediate.u[2]  = (param[15] << 16) | param[16];
		re->immediate.v[2]  = (param[17] << 16) | param[18];
		re->immediate.u[3]  = (param[19] << 16) | param[20];
		re->immediate.v[3]  = (param[21] << 16) | param[22];
		re->immediate.x[0]  = (param[23] << 16) | param[24];
		re->immediate.y[0]  = (param[25] << 16) | param[26];
		re->immediate.z[0]  = (param[27] << 16) | param[28];
		re->immediate.x[1]  = (param[29] << 16) | param[30];
		re->immediate.y[1]  = (param[31] << 16) | param[32];
		re->immediate.z[1]  = (param[33] << 16) | param[34];
		re->immediate.x[2]  = (param[35] << 16) | param[36];
		re->immediate.y[2]  = (param[37] << 16) | param[38];
		re->immediate.z[2]  = (param[39] << 16) | param[40];
		re->immediate.x[3]  = (param[41] << 16) | param[42];
		re->immediate.y[3]  = (param[43] << 16) | param[44];
		re->immediate.z[3]  = (param[45] << 16) | param[46];
		render.count[render.cur]++;
		break;
	}
	case 0x0001:
		c435_state_set_interrupt(param);
		break;
	case 0x0036:
		LOGMASKED(LOG_3D_STATE_UNK, "%s: unknown state set (%04x)\n", machine().describe_context(), m_c435.buffer[0]);
		for (int i = 0; i < (m_c435.buffer[0] & 0xff); i++)
			LOGMASKED(LOG_3D_STATE_UNK, "%s: Word %02x: %04x\n", machine().describe_context(), i, m_c435.buffer[1 + i]);
		break;
	case 0x0046:
		LOGMASKED(LOG_3D_STATE_UNK, "%s: unknown matrix(?) set (type 46) (%04x)\n", machine().describe_context(), m_c435.buffer[0]);
		for (int i = 0; i < (m_c435.buffer[0] & 0xff); i++)
			LOGMASKED(LOG_3D_STATE_UNK, "%s: Word %02x: %04x\n", machine().describe_context(), i, m_c435.buffer[1 + i]);
		break;
	case 0x00c8:
		c435_state_set_projection_matrix_line(param);
		break;
	default:
		LOGMASKED(LOG_3D_STATE_UNK, "%s: unknown state type (%04x, %04x)\n", machine().describe_context(), m_c435.buffer[0], m_c435.buffer[1]);
		for (int i = 0; i < (m_c435.buffer[0] & 0xff); i++)
			LOGMASKED(LOG_3D_STATE_UNK, "%s: Word %02x: %04x\n", machine().describe_context(), i, m_c435.buffer[1 + i]);
		break;
	}
}

void namcos23_state::c435_state_set() // 4.f
{
	if ((m_c435.buffer[0] & 0xff) == 0)
	{
		LOGMASKED(LOG_3D_STATE_ERR, "%s: WARNING: c435_state_set with zero size\n", machine().describe_context());
		return;
	}
	int size = c435_get_state_entry_size(m_c435.buffer[1]);
	if (size != ((m_c435.buffer[0] & 0xff) - 1) && m_c435.buffer[0] != 0x4f38)
	{
		LOGMASKED(LOG_3D_STATE_ERR, "%s: WARNING: c435_state_set size disagreement (type=%04x, got %d, expected %d)\n", machine().describe_context(), m_c435.buffer[1], (m_c435.buffer[0] & 0xff) - 1, size);
		LOGMASKED(LOG_3D_STATE_ERR, "Header: %04x\n", m_c435.buffer[0]);
		for (int i = 0; i < (m_c435.buffer[0] & 0xff); i++)
		{
			LOGMASKED(LOG_3D_STATE_ERR, "Word %02x: %04x\n", i, m_c435.buffer[i+1]);
		}
		return;
	}

	c435_state_set(m_c435.buffer[1], m_c435.buffer + 2);
}

void namcos23_state::c435_absolute_priority_set() // 4.1
{
	m_absolute_priority = m_c435.buffer[1];
}

void namcos23_state::c435_model_blend_factor_set() // 4.5
{
	m_model_blend_factor = m_c435.buffer[1];
}

void namcos23_state::c435_camera_power_set() // 4.7
{
	m_camera_power = m_c435.buffer[1];
}

void namcos23_state::c435_camera_ambient_set() // 4.8
{
	m_camera_ambient = m_c435.buffer[1];
}

void namcos23_state::c435_render() // 8
{
	const int size = m_c435.buffer[0] & 0xf;
	if (size != 3)
		LOGMASKED(LOG_RENDER_ERR, "%04x %04x %04x %04x %04x\n", m_c435.buffer[0], m_c435.buffer[1], m_c435.buffer[2], m_c435.buffer[3], m_c435.buffer[4]);

	render_t &render = m_render;
	bool use_scaling = BIT(m_c435.buffer[0], 7);
	bool transpose = BIT(m_c435.buffer[0], 6);

	if (render.count[render.cur] >= RENDER_MAX_ENTRIES)
	{
		LOGMASKED(LOG_RENDER_ERR, "%s: WARNING: render buffer full\n", machine().describe_context());
		return;
	}

	const s16 *m = c435_getm(m_c435.buffer[size - 1]);
	const s32 *v = c435_getv(m_c435.buffer[size]);

	namcos23_render_entry *re = render.entries[render.cur] + render.count[render.cur];
	re->type = MODEL;
	re->model.model = m_c435.buffer[1];
	re->model.model2 = (size == 4 ? m_c435.buffer[2] : 0);
	re->model.scaling = use_scaling ? m_scaling / 16384.0 : 1.0;
	re->model.transpose = transpose;
	re->absolute_priority = m_absolute_priority;
	re->model_blend_factor = m_model_blend_factor;
	re->camera_power = m_camera_power;
	re->camera_ambient = m_camera_ambient;
	re->model.light_vector[0] = m_light_vector[0];
	re->model.light_vector[1] = m_light_vector[1];
	re->model.light_vector[2] = m_light_vector[2];
	re->poly_fade_r = m_c404.poly_fade_r;
	re->poly_fade_g = m_c404.poly_fade_g;
	re->poly_fade_b = m_c404.poly_fade_b;
	re->poly_alpha_color = m_c404.poly_alpha_color;
	re->poly_alpha_pen = m_c404.poly_alpha_pen;
	re->poly_alpha = m_c404.poly_alpha;
	re->screen_fade_r = m_c404.screen_fade_r;
	re->screen_fade_g = m_c404.screen_fade_g;
	re->screen_fade_b = m_c404.screen_fade_b;
	re->screen_fade_factor = m_c404.screen_fade_factor;
	re->fade_flags = m_c404.fade_flags;
	memcpy(re->model.m, m, sizeof(re->model.m));
	memcpy(re->model.v, v, sizeof(re->model.v));

	LOGMASKED(LOG_MODEL_INFO, "%s: Render %04x (%04x) (%04x) %f (%d)\n", machine().describe_context(),
			re->model.model, m_c435.buffer[size - 1], m_c435.buffer[size], re->model.scaling, render.count[render.cur] + 1);

	render.count[render.cur]++;
}

void namcos23_state::c435_flush() // c
{
	if ((m_c435.buffer[0] & 0xf) != 0)
	{
		LOGMASKED(LOG_RENDER_ERR, "%s: WARNING: c435_flush with size %d\n", machine().describe_context(), m_c435.buffer[0] & 0xf);
		return;
	}

	if (BIT(m_c435.buffer[0], 13))
		irq_update(m_main_irqcause | MAIN_C451_IRQ);
	else
		irq_update(m_main_irqcause & ~MAIN_C451_IRQ);
}

void namcos23_state::c435_sprite_w(u16 data)
{
	// Ignore the first two values
	if (m_c435.buffer_pos >= 2)
	{
		if (m_c435.buffer_pos < 4)
		{
			m_c435.sprite_target = data;
		}
		else
		{
			u16 buf_idx = m_c435.buffer_pos - 4;
			LOGMASKED(LOG_SPRITES, "c435_sprite_w: Sprite data area %04x write: %04x\n", m_c435.sprite_target + buf_idx, data);
			m_c435.spritedata[m_c435.sprite_target + buf_idx] = data;
		}
	}
	m_c435.buffer_pos++;
}

void namcos23_state::c435_pio_w(offs_t offset, u16 data)
{
	LOGMASKED(LOG_C435_PIO_UNK, "%s: C435 PIO: %x\n", machine().describe_context(), data);
	m_c435.buffer[m_c435.buffer_pos++] = data;
	u16 h = m_c435.buffer[0];
	int psize;
	if ((h & 0x4000) == 0x4000)
		psize = h & 0xff;
	else
		psize = h & 0xf;
	if (m_c435.buffer_pos < psize + 1)
		return;

	bool known = true;
	switch (h & 0xc000)
	{
	case 0x0000:
		switch (h & 0xf0)
		{
		case 0x00:
			c435_matrix_matrix_mul();
			break;
		case 0x10:
			c435_matrix_vector_mul();
			break;
		case 0x20:
			c435_matrix_matrix_immed_mul();
			break;
		case 0x30:
			c435_matrix_vector_immed_mul();
			break;
		case 0x40:
			c435_matrix_set();
			break;
		case 0x50:
			c435_vector_set();
			break;
		default:
			known = false;
			break;
		}
		break;

	case 0x4000:
		switch (h & 0x3f00)
		{
		case 0x0100:
			c435_absolute_priority_set();
			break;
		case 0x0400:
			c435_scaling_set();
			break;
		case 0x0500:
			c435_model_blend_factor_set();
			break;
		case 0x0700:
			c435_camera_power_set();
			break;
		case 0x0800:
			c435_camera_ambient_set();
			break;
		case 0x0f00:
			c435_state_set();
			break;
		default:
			known = false;
			break;
		}
		break;

	case 0x8000:
		c435_render();
		break;
	case 0xc000:
		c435_flush();
		break;
	}

	if (!known)
	{
		std::ostringstream buf;
		buf << "Unknown c435 -";
		for (int i = 0; i < m_c435.buffer_pos; i++)
			util::stream_format(buf, " %04x", m_c435.buffer[i]);
		buf << "\n";
		LOGMASKED(LOG_C435_PIO_UNK, "%s: %s", machine().describe_context(), std::move(buf).str());
	}

	m_c435.buffer_pos = 0;
}

void namcos23_state::c435_dma(address_space &space, u32 adr, u32 size)
{
	adr &= 0x1fffffff;

	if (BIT(m_c435.pio_mode, 0))
	{
		for (u32 pos = 0; pos < size; pos += 2)
			c435_sprite_w(space.read_word(adr + pos));
	}
	else
	{
		for (u32 pos = 0; pos < size; pos += 2)
			c435_pio_w(0, space.read_word(adr + pos));
	}
}

void gorgon_state::nvram_w(offs_t offset, u32 data, u32 mem_mask)
{
	// TODO
}

u16 gorgon_state::czattr_r(offs_t offset, u16 mem_mask)
{
	u16 *czattr = (u16 *)m_czattr.target();
	LOGMASKED(LOG_C435_REG, "%s: czattr[%d] read: %04x & %04x\n", machine().describe_context(), offset, czattr[offset], mem_mask);
	return czattr[offset];
}

void gorgon_state::czattr_w(offs_t offset, u16 data, u16 mem_mask)
{
	u16 *czattr = (u16 *)m_czattr.target();
	u16 prev = czattr[offset];
	LOGMASKED(LOG_C435_REG, "%s: czattr[%d] write: %04x & %04x\n", machine().describe_context(), offset, czattr[offset], mem_mask);
	COMBINE_DATA(&czattr[offset]);

	if (offset == 4)
	{
		// invalidate if compare function changed
		u16 changed = prev ^ czattr[offset];
		for (int bank = 0; bank < 4; bank++)
		{
			m_cz_was_written[bank] |= changed >> (bank * 4) & 2;
		}
	}
}

u16 gorgon_state::czram_r(offs_t offset, u16 mem_mask)
{
	u16 *czattr = (u16 *)m_czattr.target();
	int bank = czattr[5] & 3;
	return m_banked_czram[bank][offset & 0xff];
}

void gorgon_state::czram_w(offs_t offset, u16 data, u16 mem_mask)
{
	u16 *czattr = (u16 *)m_czattr.target();
	for (int bank = 0; bank < 4; bank++)
	{
		// write enable bit
		if (~czattr[4] >> (bank * 4) & 1)
		{
			const u16 old = m_banked_czram[bank][offset & 0xff];
			COMBINE_DATA(&m_banked_czram[bank][offset & 0xff]);
			m_cz_was_written[bank] |= (old ^ m_banked_czram[bank][offset & 0xff]);
		}
	}
}

void gorgon_state::recalc_czram()
{
	u16 *czattr = (u16 *)m_czattr.target();
	for (int bank = 0; bank < 4; bank++)
	{
		// gorgon czram is 'just' a big compare table
		// this is very slow when emulating, so let's recalculate it to a simpler lookup table
		if (m_cz_was_written[bank])
		{
			int reverse = (czattr[4] >> (bank * 4) & 2) ? 0xff : 0;
			int small_val = 0x2000;
			int small_offset = reverse;
			int large_val = 0;
			int large_offset = reverse ^ 0xff;
			int prev = 0;

			for (int i = 0; i < 0x100; i++)
			{
				int factor = i ^ reverse;
				int val = std::min<u16>(m_banked_czram[bank][factor], 0x2000);
				int start = prev;
				int end = val;

				if (i > 0)
				{
					// discard if compare function doesn't match
					if (start >= end)
						continue;

					// fill range
					for (int j = start; j < end; j++)
						m_recalc_czram[bank][j] = factor;
				}

				// remember largest/smallest for later
				if (val < small_val)
				{
					small_val = val;
					small_offset = factor;
				}
				if (val > large_val)
				{
					large_val = val;
					large_offset = factor;
				}

				prev = val;
			}

			// fill possible leftover ranges
			for (int j = 0; j < small_val; j++)
				m_recalc_czram[bank][j] = small_offset;
			for (int j = large_val; j < 0x2000; j++)
				m_recalc_czram[bank][j] = large_offset;

			m_cz_was_written[bank] = 0;
		}
	}
}

u32 namcos23_state::c435_busy_flag_r()
{
	LOGMASKED(LOG_C435_REG, "%s: c435 read busy flag: %08x\n", machine().describe_context(), 1);
	return 1;
}

void namcoss23_state::c435_c361_ack_w(offs_t offset, u32 data)
{
	irq_update(m_main_irqcause & ~MAIN_C361_IRQ);
}

void namcos23_state::c435_dma_addr_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_C435_REG, "%s: c435 write address: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_c435.address);
}

void namcos23_state::c435_dma_size_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_C435_REG, "%s: c435 write size: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_c435.size);
}

void namcos23_state::c435_dma_start_w(address_space &space, offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_C435_REG, "%s: c435 write DMA: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	if (data & 1)
		c435_dma(space, m_c435.address, m_c435.size);
}

void gorgon_state::c435_pio_mode_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_C435_REG, "%s: PIO mode write: %08x & %08x (%s mode)\n", machine().describe_context(), data, mem_mask, BIT(data, 0) ? "sprite" : "command");
	COMBINE_DATA(&m_c435.pio_mode);
	m_c435.buffer_pos = 0;
}

void namcos23_state::c435_clear_bufpos_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_C435_REG, "%s: clear buffer pos: %08x & %08x\n", machine().describe_context(), data, mem_mask);
	m_c435.buffer_pos = 0;
}

void namcos23_renderer::render_sprite_scanline(s32 scanline, const extent_t& extent, const namcos23_render_data& object, int threadid)
{
	const namcos23_render_data& rd = object;

	int y_index = extent.param[1].start - rd.sprite_yflip;
	float x_index = extent.param[0].start - rd.sprite_xflip;
	float dx = extent.param[0].dpdx;
	const pen_t *pal = rd.pens;
	u8 *const source = (u8 *)rd.sprite_source + y_index * rd.sprite_line_modulo;
	u32 *const dest = &object.bitmap->pix(scanline);
	u8 *primap = &object.primap->pix(scanline);
	int prioverchar = object.prioverchar;

	int alphafactor = object.alpha;
	bool alpha_enabled = object.alpha_enabled;
	u8 alpha_pen = object.poly_alpha_pen;
	int fadefactor = 0xff - object.fadefactor;
	rgbaint_t fadecolor(object.fadecolor);

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		int pen = source[(int)x_index];
		if (pen != 0xff)
		{
			rgbaint_t rgb(pal[pen]);

			if (fadefactor != 0xff)
			{
				rgb.blend(fadecolor, fadefactor);
			}

			if (alphafactor != 0xff && (alpha_enabled || pen == alpha_pen))
			{
				rgb.blend(rgbaint_t(dest[x]), alphafactor);
			}

			dest[x] = 0xff000000 | rgb.to_rgba();
			primap[x] = (primap[x] & ~1) | prioverchar;
		}
		x_index += dx;
	}
}

void namcos23_renderer::render_scanline(s32 scanline, const extent_t& extent, const namcos23_render_data& object, int threadid)
{
	const namcos23_render_data& rd = object;

	float z = extent.param[0].start;
	float u = extent.param[1].start;
	float v = extent.param[2].start;
	float i = extent.param[3].start;
	const float dz = extent.param[0].dpdx;
	const float du = extent.param[1].dpdx;
	const float dv = extent.param[2].dpdx;
	const float di = extent.param[3].dpdx;

	int fadefactor = 0xff - rd.fadefactor;
	int alphafactor = 0xff - rd.alpha;
	bool alpha_enabled = rd.alpha_enabled;
	u8 alpha_pen = rd.poly_alpha_pen;
	bool polyfade_enabled = rd.pfade_enabled;
	rgbaint_t fadecolor = rd.fadecolor;
	rgbaint_t polycolor = rd.polycolor;
	bool shade_enabled = rd.shade_enabled;
	bool blend_enabled = rd.blend_enabled;

	u32 *dest = &rd.bitmap->pix(scanline);
	u8 *primap = &rd.primap->pix(scanline);

	const pen_t *pens = rd.pens;
	int prioverchar = rd.prioverchar;
	int penmask = 0xff;
	int penshift = 0;

	if (rd.cmode & 4)
	{
		pens += 0xec + ((rd.cmode & 8) << 1);
		penmask = 0x03;
		penshift = 2 * (~rd.cmode & 3);
	}
	else if (rd.cmode & 2)
	{
		pens += 0xe0 + ((rd.cmode & 8) << 1);
		penmask = 0x0f;
		penshift = 4 * (~rd.cmode & 1);
	}

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		float ooz = 1.0f / z;
		int tx = int(u * ooz);
		int ty = int(v * ooz);
		u8 pen = 0;
		bool reject = rd.stencil_lookup(*rd.machine, tx, ty);
		if (!reject)
		{
			ty += rd.tbase;
			u32 tex_rgb = rd.texture_lookup(*rd.machine, pens, penshift, penmask, tx, ty, pen);
			rgbaint_t rgb(tex_rgb);

			if (shade_enabled)
			{
				int shade = i * ooz;
				rgb.scale_imm_and_clamp(shade << 2);
			}

			// fade
			if (polyfade_enabled)
			{
				rgb.scale_and_clamp(polycolor);
			}

			if (fadefactor != 0xff)
			{
				rgb.blend(fadecolor, fadefactor);
			}

			// alpha
			if (alphafactor != 0xff && (alpha_enabled || pen == alpha_pen))
			{
				rgb.blend(rgbaint_t(dest[x]), alphafactor);
			}

			if (blend_enabled)
			{
				rgb.blend(rgbaint_t(dest[x]), 0x80);
			}

			u32 src = rgb.to_rgba();
			dest[x] = 0xff000000 | src;
			primap[x] = (primap[x] & ~1) | prioverchar;
		}

		z += dz;
		u += du;
		v += dv;
		i += di;
	}
}

void namcos23_state::render_apply_transform(s32 xi, s32 yi, s32 zi, const namcos23_render_entry *re, float &x, float &y, float &z)
{
	s16 m[9];
	memcpy(m, re->model.m, sizeof(s16) * 9);
	if (re->model.transpose)
		transpose_matrix(m);
	x = (((m[0] * s64(xi) + m[1] * s64(yi) + m[2] * s64(zi)) >> 14) * re->model.scaling + re->model.v[0]) / 16384.f;
	y = (((m[3] * s64(xi) + m[4] * s64(yi) + m[5] * s64(zi)) >> 14) * re->model.scaling + re->model.v[1]) / 16384.f;
	z = (((m[6] * s64(xi) + m[7] * s64(yi) + m[8] * s64(zi)) >> 14) * re->model.scaling + re->model.v[2]) / 16384.f;
}

void namcos23_state::render_apply_matrot(s32 xi, s32 yi, s32 zi, const namcos23_render_entry *re, float &x, float &y, float &z)
{
	x = (re->model.m[0] * xi + re->model.m[1] * yi + re->model.m[2] * zi) / 4194304.f;
	y = (re->model.m[3] * xi + re->model.m[4] * yi + re->model.m[5] * zi) / 4194304.f;
	z = (re->model.m[6] * xi + re->model.m[7] * yi + re->model.m[8] * zi) / 4194304.f;
}

void namcos23_state::render_project(poly_vertex &pv)
{
	// 768 validated by the title screen size on tc2:
	// texture is 640x480, x range is 3.125, y range is 2.34375, z is 3.75
	// 640/(3.125/3.75) = 768
	// 480/(2.34375/3.75) = 768

	pv.x = 320 + m_proj_matrix[23]*pv.x;
	pv.y = 240 - m_proj_matrix[23]*pv.y;

	pv.p[0] = 1.0f / pv.p[0];
}

static bool render_stencil_lookup(running_machine &machine, float x, float y)
{
	namcos23_state *state = machine.driver_data<namcos23_state>();
	u32 xx = u32(x);
	u32 yy = u32(y);
	u32 bit = (xx & 15) ^ 15;
	u32 offs = ((yy << 6) | (xx >> 4)) & 0x1ffff;
	if (!BIT(state->m_texram[offs], bit))
	{
		return true;
	}
	return false;
}

static bool render_stencil_lookup_always(running_machine &machine, float x, float y)
{
	return false;
}

static u32 render_texture_lookup(running_machine &machine, const pen_t *pens, int penshift, int penmask, float x, float y, u8 &pen)
{
	namcos23_state *state = machine.driver_data<namcos23_state>();
	u32 xx = u32(x);
	u32 yy = u32(y);
	u32 tileid = ((xx >> 4) & 0xff) | ((yy << 4) & state->m_tileid_mask);
	u8 attr = state->m_tmhrom[tileid >> 1];
	if(tileid & 1)
		attr &= 15;
	else
		attr >>= 4;
	u32 tile = (state->m_tmlrom[tileid] | (attr << 16)) & state->m_tile_mask;

	// Probably swapx/swapy to add on bits 2-3 of attr
	// Bits used by motoxgo at least
	u8 color = state->m_texrom[(tile << 8) | ((yy << 4) & 0xf0) | (xx & 0x0f)];
	pen = color;
	return pens[(pen >> penshift) & penmask];
}

void namcos23_state::render_direct_poly(const namcos23_render_entry *re)
{
	render_t &render = m_render;

	u32 polyshift = ((re->direct.d[1] & 0x1ff) << 12) | (re->direct.d[0] & 0xfff);
	u32 cztype = re->direct.d[3] & 3;
	u32 flags = ((re->direct.d[3] << 6) & 0x1fff) | cztype;

	static const int indices[2][3] =
	{
		{ 0, 1, 2 },
		{ 0, 2, 3 }
	};
	for (int i = 0; i < 2; i++)
	{
		namcos23_poly_entry *p = render.polys + render.poly_count;
		p->vertex_count = 3;

		for (int j = 0; j < 3; j++)
		{
			int index = indices[i][j];
			u16 const *src = &re->direct.d[4 + index * 6];

			int mantissa = src[5];
			int exponent = src[4] & 0x3f;

			if (mantissa)
			{
				p->pv[j].p[0] = mantissa;
				while (exponent < 0x2e)
				{
					p->pv[j].p[0] /= 2.0f;
					exponent++;
				}
			}
			else
				p->pv[j].p[0] = 1.f;

			p->pv[j].p[1] = ((src[0] >> 4) + 0.5) * p->pv[j].p[0];
			p->pv[j].p[2] = ((src[1] >> 4) + 0.5) * p->pv[j].p[0];
			p->pv[j].p[3] = (src[4] >> 8) * p->pv[j].p[0];
			p->pv[j].x = ((s16)src[2] + 320);
			p->pv[j].y = ((s16)src[3] + 240);
		}

		p->zkey = polyshift | (re->absolute_priority << 21);
		p->rd.machine = &machine();
		p->rd.texture_lookup = render_texture_lookup;
		p->rd.stencil_lookup = render_stencil_lookup_always;
		p->rd.pens = m_palette->pens() + (re->direct.d[2] & 0x7f00);
		p->rd.direct = true;
		p->rd.sprite = false;
		p->rd.immediate = false;
		p->rd.shade_enabled = true;
		p->rd.rgb = 0x00ffffff;
		p->rd.flags = flags;
		p->rd.tbase = ((re->direct.d[2] & 0x000f) << 12) | (BIT(re->direct.d[2], 7) << 16);
		p->rd.cmode = (re->direct.d[2] & 0x0070) >> 4;
		p->rd.cz_value = (re->direct.d[3] >> 2) & 0x1fff;
		p->rd.cz_type = re->direct.d[3] & 3;
		p->rd.prioverchar = ((p->rd.cmode & 7) == 1) ? 7 : 0;

		p->rd.fogfactor = 0;
		p->rd.fadefactor = 0;
		p->rd.alphafactor = re->poly_alpha;

		// global fade
		if (re->fade_flags & 1)
		{
			p->rd.fadefactor = re->screen_fade_factor;
			p->rd.fadecolor.set(0, re->screen_fade_r, re->screen_fade_g, re->screen_fade_b);
		}

		// poly fade
		p->rd.pfade_enabled = re->poly_fade_r != 0 || re->poly_fade_g != 0 || re->poly_fade_b != 0;
		p->rd.polycolor.set(0, re->poly_fade_r, re->poly_fade_g, re->poly_fade_b);

		// alpha
		p->rd.alpha = re->poly_alpha;
		p->rd.alpha_enabled = ((re->direct.d[2] >> 8) & 0x7f) != re->poly_alpha_color;
		p->rd.poly_alpha_pen = re->poly_alpha_pen;
		p->rd.type = re->type;

		// blend
		p->rd.blend_enabled = false;

		render.poly_count++;
	}
}

void namcos23_state::render_sprite(const namcos23_render_entry *re)
{
	int offset = 0;

	for (int row = 0; row < re->sprite.rows; row++)
	{
		for (int col = 0; col < re->sprite.cols; col++)
		{
			render_sprite_tile(offset, re, row, col);
			offset++;
		}
	}
}

void namcos23_state::render_sprite_tile(u32 code_offset, const namcos23_render_entry *re, int row, int col)
{
	const namcos23_sprite_data &sprite = re->sprite;
	render_t &render = m_render;

	u32 code = sprite.code + code_offset;

	gfx_element *gfx = m_gfxdecode->gfx(2);
	s32 sprite_screen_height = (((sprite.ysize << 16) >> 5) * gfx->height() + 0x8000) >> 16;
	s32 sprite_screen_width = (((sprite.xsize << 16) >> 5) * gfx->width() + 0x8000) >> 16;
	if (sprite_screen_width && sprite_screen_height)
	{
		float fsx = sprite.xpos + col * sprite.xsize;
		float fsy = sprite.ypos + row * sprite.ysize;
		float fwidth = gfx->width();
		float fheight = gfx->height();
		float fsw = sprite_screen_width;
		float fsh = sprite_screen_height;

		namcos23_poly_entry *p = render.polys + render.poly_count;
		p->vertex_count = 4;

		p->pv[0].x = fsx;
		p->pv[0].y = fsy;
		p->pv[0].p[0] = 0;
		p->pv[0].p[1] = 0;
		p->pv[1].x = fsx + fsw;
		p->pv[1].y = fsy;
		p->pv[1].p[0] = fwidth;
		p->pv[1].p[1] = 0;
		p->pv[2].x = fsx + fsw;
		p->pv[2].y = fsy + fsh;
		p->pv[2].p[0] = fwidth;
		p->pv[2].p[1] = fheight;
		p->pv[3].x = fsx;
		p->pv[3].y = fsy + fsh;
		p->pv[3].p[0] = 0;
		p->pv[3].p[1] = fheight;
		p->pv[0].p[2] = p->pv[1].p[2] = p->pv[2].p[2] = p->pv[3].p[2] = 1.f;
		p->pv[0].p[3] = p->pv[1].p[3] = p->pv[2].p[3] = p->pv[3].p[3] = 64.f;

		p->rd.pens = m_palette->pens() + gfx->granularity() * (sprite.color & 0x7f);
		p->zkey = sprite.zcoord;
		p->rd.machine = &machine();
		p->rd.sprite = true;
		p->rd.immediate = false;
		p->rd.shade_enabled = false;
		p->rd.sprite_source = gfx->get_data(code % gfx->elements());
		p->rd.sprite_line_modulo = gfx->rowbytes();
		p->rd.sprite_xflip = sprite.xflip;
		p->rd.sprite_yflip = sprite.yflip;

		p->rd.fadefactor = 0;

		// global fade
		if (re->fade_flags & 2 || sprite.fade_enabled)
		{
			p->rd.fadefactor = re->screen_fade_factor;
			p->rd.fadecolor.set(0, re->screen_fade_r, re->screen_fade_g, re->screen_fade_b);
		}

		// sprite fog
		p->rd.fogfactor = 0;

		p->rd.alpha = 0xff - sprite.alpha;
		p->rd.poly_alpha_pen = re->poly_alpha_pen;
		p->rd.alpha_enabled = (sprite.color & 0x7f) != re->poly_alpha_color;

		p->rd.blend_enabled = false;
		render.poly_count++;
	}
}

void namcos23_state::render_immediate(const namcos23_render_entry *re)
{
	render_t &render = m_render;
	poly_vertex pv[16];

	u32 type = re->immediate.type;
	u32 h    = re->immediate.h;
	u32 polyshift = re->immediate.zbias;
	u32 ne   = (type >> 8) & 0xf;
	bool stencil_enabled = BIT(h, 11);

	float minz = FLT_MAX;
	float maxz = FLT_MIN;

	for (int i = 0; i < ne; i++)
	{
		pv[i].x = (s32)re->immediate.x[i] / 16384.f;
		pv[i].y = (s32)re->immediate.y[i] / 16384.f;
		pv[i].p[0] = (s32)re->immediate.z[i] / 16384.f;
		pv[i].p[1] = (s32)re->immediate.u[i];
		pv[i].p[2] = (s32)re->immediate.v[i];
		pv[i].p[3] = (s32)re->immediate.i[i];
	}

	namcos23_poly_entry *p = render.polys + render.poly_count;

	// Should be unnecessary once frustum clipping happens correctly, but this will at least cull polys behind the camera
	p->vertex_count = render.polymgr->zclip_if_less<4>(ne, pv, p->pv, 0.0001f);

	// Project if you don't clip on the near plane
	if (p->vertex_count >= 3)
	{
		for (int i = 0; i < p->vertex_count; i++)
		{
			float z = p->pv[i].p[0] != 0 ? p->pv[i].p[0] : 1.f;
			p->pv[i].x /= z;
			p->pv[i].y /= z;

			z *= 16384.f;
			if (z > maxz)
				maxz = z;
			if (z < minz)
				minz = z;

			render_project(p->pv[i]);

			float w = p->pv[i].p[0];
			p->pv[i].p[1] *= w;
			p->pv[i].p[2] *= w;
			p->pv[i].p[3] *= w;
		}

		// Compute an odd sorta'-Z thing that can situate the polygon wherever you want in Z-depth
		int zsort = 0.5f * (minz + maxz) + 0.5f;
		if (zsort > 0x1fffff) zsort = 0x1fffff;

		int absolute_priority = re->absolute_priority & 7;
		if (BIT(polyshift, 21))
			zsort = polyshift & 0x1fffff;
		else
		{
			zsort += BIT(polyshift, 16) ? (polyshift | 0xfffe0000) : (polyshift & 0x0000ffff);
			absolute_priority += (polyshift & 0x1c0000) >> 18;
		}

		p->zkey = zsort | (absolute_priority << 21);
		p->rd.machine = &machine();
		p->rd.texture_lookup = render_texture_lookup;
		p->rd.stencil_lookup = stencil_enabled ? render_stencil_lookup : render_stencil_lookup_always;
		p->rd.pens = m_palette->pens() + (re->immediate.pal & 0x7f00);
		p->rd.rgb = 0x00ffffff;
		p->rd.direct = false;
		p->rd.sprite = false;
		p->rd.immediate = true;
		p->rd.shade_enabled = true;
		p->rd.h = h;
		p->rd.type = type;
		p->rd.tbase = 0;

		// global fade
		if (re->fade_flags & 1)
		{
			p->rd.fadefactor = re->screen_fade_factor;
			p->rd.fadecolor.set(0, re->screen_fade_r, re->screen_fade_g, re->screen_fade_b);
		}

		// poly fade
		p->rd.pfade_enabled = re->poly_fade_r != 0 || re->poly_fade_g != 0 || re->poly_fade_b != 0;
		p->rd.polycolor.set(0, re->poly_fade_r, re->poly_fade_g, re->poly_fade_b);

		// alpha
		p->rd.alpha = re->poly_alpha;
		p->rd.alpha_enabled = ((re->immediate.pal >> 8) & 0x7f) != re->poly_alpha_color;
		p->rd.poly_alpha_pen = re->poly_alpha_pen;
		p->rd.blend_enabled = BIT(h, 10);
		p->rd.type = re->type;

		render.poly_count++;
	}
}

void namcos23_state::render_model(const namcos23_render_entry *re)
{
	render_t &render = m_render;

	LOGMASKED(LOG_MODELS, "%s: Drawing model %04x\n", machine().describe_context(), re->model.model);

	const bool model_blend = (re->model.model2 != 0);
	u32 adr = m_ptrom[re->model.model];
	u32 adr2 = m_ptrom[re->model.model2];
	if (adr >= m_ptrom_limit || adr2 >= m_ptrom_limit)
	{
		LOGMASKED(LOG_MODEL_ERR, "%s: WARNING: model %04x base address %08x out-of-bounds - pointram?\n", machine().describe_context(), re->model.model, adr);
		return;
	}

	u32 offs = 0;
	const u32 *data = &m_ptrom[adr];
	const u32 *data2 = &m_ptrom[adr2];
	while ((adr + offs) < m_ptrom_limit && (adr2 + offs) < m_ptrom_limit)
	{
		poly_vertex pv[15];

		u32 type = data[offs++];
		u32 h    = data[offs++];

		u32 cmode = (type & 0x70000000) >> 29;
		u32 tbase = (((type & 0x0f000000) >> 24) << 12) | (BIT(type, 31) << 16);
		u8 color = (h >> 24) & 0x7f;
		int lmode = (type >> 19) & 3;
		int ne = (type >> 8) & 15;
		bool stencil_enabled = BIT(h, 11);

		u32 polyshift = 0;
		if (type & 0x00001000)
		{
			polyshift = data[offs++];
		}
		u8 alpha = 0xff;

		u32 light = 0;
		u32 extptr = 0;

		if (lmode == 3)
		{
			extptr = offs;
			offs += ne;
		}
		else
		{
			light = data[offs++];
		}

		for (int i = 0; i < ne; i++)
		{
			u32 v1 = data[offs + 0];
			u32 v2 = data[offs + 1];
			u32 v3 = data[offs + 2];

			float x, y, z;
			render_apply_transform(u32_to_s24(v1), u32_to_s24(v2), u32_to_s24(v3), re, x, y, z);

			float factor_b = re->model_blend_factor / 16384.f;
			float factor_a = 1.f - factor_b;
			if (model_blend)
			{
				u32 v12 = data2[offs + 0];
				u32 v22 = data2[offs + 1];
				u32 v32 = data2[offs + 2];

				float x2, y2, z2;
				render_apply_transform(u32_to_s24(v12), u32_to_s24(v22), u32_to_s24(v32), re, x2, y2, z2);

				x = x * factor_a + x2 * factor_b;
				y = y * factor_a + y2 * factor_b;
				z = z * factor_a + z2 * factor_b;
			}

			offs += 3;

			pv[i].x = x;
			pv[i].y = y;
			pv[i].p[0] = z;
			pv[i].p[1] = (((v1 >> 20) & 0xf00) | ((v2 >> 24) & 0xff)) + (stencil_enabled ? 0 : 0.5);
			pv[i].p[2] = (((v1 >> 16) & 0xf00) | ((v3 >> 24) & 0xff)) + (stencil_enabled ? 0 : 0.5);
			pv[i].p[3] = 64;

			static const u8 LIGHT_SHIFTS[4] = { 24, 16,  8,  0 };
			switch (lmode)
			{
			case 0:
				pv[i].p[3] = (float)((light >> LIGHT_SHIFTS[i]) & 0xff);
				break;
			case 1:
				pv[i].p[3] = (float)((light >> LIGHT_SHIFTS[i]) & 0xff);
				break;
			case 2:
				pv[i].p[3] = 64.f;
				break;
			case 3:
			{
				u32 norm = data[extptr];
				s32 nx = u32_to_s10((norm >> 20) & 0x3ff);
				s32 ny = u32_to_s10((norm >> 10) & 0x3ff);
				s32 nz = u32_to_s10(norm & 0x3ff);

				if (model_blend)
				{
					u32 norm2 = data2[extptr];
					s32 nx2 = u32_to_s10((norm2 >> 20) & 0x3ff);
					s32 ny2 = u32_to_s10((norm2 >> 10) & 0x3ff);
					s32 nz2 = u32_to_s10(norm2 & 0x3ff);

					nx = (s32)(nx * factor_a + nx2 * factor_b);
					ny = (s32)(ny * factor_a + ny2 * factor_b);
					nz = (s32)(nz * factor_a + nz2 * factor_b);
				}
				extptr++;

				float nrx, nry, nrz;
				render_apply_matrot(nx, ny, nz, re, nrx, nry, nrz);
				float length = sqrtf(nrx * nrx + nry * nry + nrz * nrz);
				if (length != 0.0f)
				{
					nrx /= length;
					nry /= length;
					nrz /= length;
				}

				float cx, cy, cz;
				render_apply_matrot(re->model.light_vector[0], re->model.light_vector[1], re->model.light_vector[2], re, cx, cy, cz);
				length = sqrtf(cx * cx + cy * cy + cz * cz);
				if (length != 0.0f)
				{
					cx /= length;
					cy /= length;
					cz /= length;
				}

				float lsi = nrx * cx + nry * cy + nrz * cz;
				if (lsi < 0)
					lsi = 0;

				pv[i].p[3] = std::clamp(re->camera_ambient + re->camera_power * lsi, 0.f, 64.f);
			}   break;
			}
		}
		namcos23_poly_entry *p = render.polys + render.poly_count;

		if (ne >= 3)
		{
			if (BIT(h, 5))
			{
				const float z0 = pv[0].p[0];
				const float z1 = pv[1].p[0];
				const float z2 = pv[2].p[0];
				const float z3 = pv[3].p[0];
				float c1 =
					(pv[2].x * (z0 * pv[1].y - pv[0].y * z1)) +
					(pv[2].y * (pv[0].x * z1 - z0 * pv[1].x)) +
					(z2 * (pv[0].y * pv[1].x - pv[0].x * pv[1].y));
				float c2 =
					(pv[0].x * (z2 * pv[3].y - pv[2].y * z3))+
					(pv[0].y * (pv[2].x * z3 - z2 * pv[3].x))+
					(z0 * (pv[2].y * pv[3].x - pv[2].x * pv[3].y));

				if (c1 >= 0.f && c2 >= 0.f)
				{
					if (type & 0x00010000)
						break;
					continue;
				}
			}
		}
		else
		{
			if (type & 0x00010000)
				break;
			continue;
		}

		float minz = FLT_MAX;
		float maxz = FLT_MIN;

		// Should be unnecessary once frustum clipping happens correctly, but this will at least cull polys behind the camera
		p->vertex_count = render.polymgr->zclip_if_less<4>(ne, pv, p->pv, 0.0001f);

		// Project if you don't clip on the near plane
		if (p->vertex_count >= 3)
		{
			// This is our bad excuse for a projection matrix
			for (int i = 0; i < p->vertex_count; i++)
			{
				float z = (p->pv[i].p[0] != 0) ? p->pv[i].p[0] : 1.f;
				p->pv[i].x /= z;
				p->pv[i].y /= z;

				z *= 16384.f;
				if (z > maxz)
					maxz = z;
				if (z < minz)
					minz = z;

				render_project(p->pv[i]);

				float w = p->pv[i].p[0];
				p->pv[i].p[1] *= w;
				p->pv[i].p[2] *= w;
				p->pv[i].p[3] *= w;
			}

			if (maxz < 0)
			{
				if (type & 0x00010000)
					break;
				continue;
			}

			int zsort = 0.5f * (minz + maxz) + 0.5f;
			if (zsort > 0x1fffff) zsort = 0x1fffff;

			int absolute_priority = re->absolute_priority & 7;
			if (BIT(polyshift, 21))
				zsort = polyshift & 0x1fffff;
			else
			{
				zsort += BIT(polyshift, 17) ? (polyshift | 0xfffc0000) : (polyshift & 0x0001ffff);
				absolute_priority += (polyshift & 0x1c0000) >> 18;
			}

			zsort = std::clamp(zsort, 0, 0x1fffff);
			zsort |= (absolute_priority << 21);
			p->zkey = zsort;

			p->rd.machine = &machine();
			p->rd.texture_lookup = render_texture_lookup;
			p->rd.stencil_lookup = stencil_enabled ? render_stencil_lookup : render_stencil_lookup_always;
			p->rd.pens = m_palette->pens() + (color << 8);
			p->rd.rgb = (alpha << 24) | 0x00ffffff;
			p->rd.model = re->model.model;
			p->rd.direct = false;
			p->rd.sprite = false;
			p->rd.immediate = false;
			p->rd.shade_enabled = true;
			p->rd.h = h;
			p->rd.type = type;
			p->rd.tbase = tbase;
			p->rd.cmode = cmode;
			p->rd.cz_value = 0;
			p->rd.cz_type = 0;
			p->rd.type = re->type;

			p->rd.fogfactor = 0;
			p->rd.fadefactor = 0;
			p->rd.alphafactor = re->poly_alpha;

			// global fade
			if (re->fade_flags & 1)
			{
				p->rd.fadefactor = re->screen_fade_factor;
				p->rd.fadecolor.set(0, re->screen_fade_r, re->screen_fade_g, re->screen_fade_b);
			}

			// poly fade
			p->rd.pfade_enabled = re->poly_fade_r != 0 || re->poly_fade_g != 0 || re->poly_fade_b != 0;
			p->rd.polycolor.set(0, re->poly_fade_r, re->poly_fade_g, re->poly_fade_b);

			// alpha
			p->rd.alpha = re->poly_alpha;
			p->rd.alpha_enabled = ((color & 0x7f) != re->poly_alpha_color) || BIT(type, 21);
			p->rd.poly_alpha_pen = re->poly_alpha_pen;
			p->rd.blend_enabled = BIT(h, 10);

			render.poly_count++;
		}

		if (type & 0x000010000)
			break;
	}
}

static int render_poly_compare(const void *i1, const void *i2)
{
	const namcos23_poly_entry *p1 = *(const namcos23_poly_entry **)i1;
	const namcos23_poly_entry *p2 = *(const namcos23_poly_entry **)i2;

	return p1->zkey <= p2->zkey ? 1 : p1->zkey > p2->zkey ? -1 : 0;
}

void namcos23_renderer::render_flush(screen_device &screen, bitmap_rgb32 &bitmap)
{
	render_t &render = m_state.m_render;

	if (!render.poly_count)
		return;

	for (int i = 0; i < render.poly_count; i++)
		render.poly_order[i] = &render.polys[i];

	qsort(render.poly_order, render.poly_count, sizeof(namcos23_poly_entry *), render_poly_compare);

	const static rectangle scissor(0, 639, 0, 479);

	for (int i = 0; i < render.poly_count; i++)
	{
		const namcos23_poly_entry *p = render.poly_order[i];
		namcos23_render_data& extra = render.polymgr->object_data().next();
		extra = p->rd;
		extra.bitmap = &bitmap;
		extra.primap = &screen.priority();
		extra.prioverchar = 2;

		// We should probably split the polygons into triangles ourselves to insure everything is being rendered properly
		if (p->rd.sprite)
			render_triangle_fan<4>(scissor, render_delegate(&namcos23_renderer::render_sprite_scanline, this), 4, p->pv);
		else if (p->vertex_count == 3)
			render_triangle<4>(scissor, render_delegate(&namcos23_renderer::render_scanline, this), p->pv[0], p->pv[1], p->pv[2]);
		else if (p->vertex_count == 4)
			render_triangle_fan<4>(scissor, render_delegate(&namcos23_renderer::render_scanline, this), 4, p->pv);
		else if (p->vertex_count == 5)
			render_triangle_fan<4>(scissor, render_delegate(&namcos23_renderer::render_scanline, this), 5, p->pv);
		else if (p->vertex_count == 6)
			render_triangle_fan<4>(scissor, render_delegate(&namcos23_renderer::render_scanline, this), 6, p->pv);
	}

	render.poly_count = 0;
}

void gorgon_state::render_run(screen_device &screen, bitmap_rgb32 &bitmap)
{
	render_t &render = m_render;

	if (BIT(m_c404.layer_flags, 1) && !BIT(m_c435.spritedata[0], 0))
	{
		bool y_lowres = !BIT(m_c435.spritedata[0], 2);
		s16 deltax = (s16)((s16)m_c435.spritedata[6] + (s16)m_c435.spritedata[10] + 0x50);
		s16 deltay = (s16)((s16)m_c435.spritedata[12] + (0x2a >> (y_lowres ? 1 : 0)));
		u16 base = m_c435.spritedata[2];
		u16 sprite_count = (m_c435.spritedata[4] - base) + 1;
		for (int i = 0; i < sprite_count; i++)
		{
			namcos23_render_entry *re = render.entries[!render.cur] + render.count[!render.cur];

			u16 *data = &m_c435.spritedata[0x4000 + i * 16];
			re->type = SPRITE;
			re->poly_fade_r = m_c404.poly_fade_r;
			re->poly_fade_g = m_c404.poly_fade_g;
			re->poly_fade_b = m_c404.poly_fade_b;
			re->poly_alpha_color = m_c404.poly_alpha_color;
			re->poly_alpha_pen = m_c404.poly_alpha_pen;
			re->poly_alpha = m_c404.poly_alpha;
			re->screen_fade_r = m_c404.screen_fade_r;
			re->screen_fade_g = m_c404.screen_fade_g;
			re->screen_fade_b = m_c404.screen_fade_b;
			re->screen_fade_factor = m_c404.screen_fade_factor;
			re->fade_flags = m_c404.fade_flags;
			re->absolute_priority = m_absolute_priority;
			re->model_blend_factor = 0;
			re->sprite.zcoord = ((m_c404.sprites[i].d[0] << 16) | m_c404.sprites[i].d[1]) & 0x00ffffff;
			re->sprite.xpos = (s16)data[0] - deltax;
			re->sprite.ypos = (s16)data[2] - deltay;
			re->sprite.xsize = (s16)data[4];
			re->sprite.ysize = (s16)data[6];
			re->sprite.rows = data[10] & 7;
			re->sprite.cols = (data[10] >> 4) & 7;
			re->sprite.linktype = data[8] & 0xff;
			re->sprite.alpha = data[14] >> 8;
			re->sprite.code = data[12];
			re->sprite.xflip = BIT(data[10], 7);
			re->sprite.yflip = BIT(data[10], 3);
			re->sprite.color = m_c404.sprites[i].d[2] & 0xff;
			re->sprite.cz = m_c404.sprites[i].d[3] & 0xff;
			re->sprite.fade_enabled = BIT(m_c404.sprites[i].d[3], 15);
			re->sprite.prioverchar = (re->sprite.cz == 0xfe) ? 1 : 0;

			if (re->sprite.rows == 0)
				re->sprite.rows = 8;
			if (re->sprite.cols == 0)
				re->sprite.cols = 8;

			// right justify
			if (BIT(data[10], 9))
				re->sprite.xpos -= re->sprite.xsize * re->sprite.cols - 1;

			// bottom justify
			if (BIT(data[10], 8))
				re->sprite.ypos -= re->sprite.ysize * re->sprite.rows - 1;

			if (re->sprite.yflip)
			{
				re->sprite.ypos += re->sprite.ysize * re->sprite.rows - 1;
				re->sprite.ysize = -re->sprite.ysize;
			}

			if (re->sprite.xflip)
			{
				re->sprite.xpos += re->sprite.xsize * re->sprite.cols - 1;
				re->sprite.xsize = -re->sprite.xsize;
			}

			if (re->sprite.xsize && re->sprite.ysize)
			{
				render.count[!render.cur]++;
			}
		}
	}

	namcos23_state::render_run(screen, bitmap);
}

void namcos23_state::render_run(screen_device &screen, bitmap_rgb32 &bitmap)
{
	render_t &render = m_render;
	const namcos23_render_entry *re = render.entries[!render.cur];

	render.poly_count = 0;
	for (int i = 0; i < render.count[!render.cur]; i++)
	{
		switch (re->type)
		{
		case MODEL:
			if (m_c404.layer_flags & 1)
				render_model(re);
			break;
		case DIRECT:
			if (m_c404.layer_flags & 1)
				render_direct_poly(re);
			break;
		case IMMEDIATE:
			if (m_c404.layer_flags & 1)
				render_immediate(re);
			break;
		case SPRITE:
			render_sprite(re);
			break;
		}
		re++;
	}

	render.polymgr->render_flush(screen, bitmap);
	render.polymgr->wait();

	render.cur = !render.cur;
	render.count[render.cur] = 0;
}



// C404 (gamma/palette/sprites)

void namcos23_state::sprites_w(offs_t offset, u16 data, u16 mem_mask)
{
	switch (offset)
	{
		case 0:
			m_c404.spritedata_idx = data >> 1;
			break;
		case 1:
			m_c404.sprites[(m_c404.spritedata_idx >> 2) % 0x280].d[m_c404.spritedata_idx & 3] = data;
			m_c404.spritedata_idx++;
			break;
	}
	LOGMASKED(LOG_SPRITES, "%s: sprites_w: %d = %04x\n", machine().describe_context(), offset, data);
}

void namcos23_state::paletteram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_generic_paletteram_32[offset]);

	// each LONGWORD is 2 colors, each OFFSET is 2 colors
	for (int i = 0; i < 2; i++)
	{
		int which = (offset << 2 | i << 1) & 0xfffe;
		int r = nthbyte(m_generic_paletteram_32, which|0x00001);
		int g = nthbyte(m_generic_paletteram_32, which|0x10001);
		int b = nthbyte(m_generic_paletteram_32, which|0x20001);
		m_palette->set_pen_color(which/2, rgb_t(r,g,b));
	}
}



// C361 (text)

TILE_GET_INFO_MEMBER(namcos23_state::text_tilemap_get_info)
{
	u16 data = nthword(m_textram, tile_index);
	/**
	* xxxx.----.----.---- palette select
	* ----.xx--.----.---- flip
	* ----.--xx.xxxx.xxxx code
	*/
	tileinfo.set(0, data & 0x03ff, data >> 12, TILE_FLIPYX((data & 0x0c00) >> 10));
}

void namcos23_state::textram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_textram[offset]);
	m_bgtilemap->mark_tile_dirty(offset*2);
	m_bgtilemap->mark_tile_dirty((offset*2)+1);
}

void namcos23_state::textchar_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_charram[offset]);
	m_gfxdecode->gfx(0)->mark_dirty(offset/32);
}

// C404 (mixing, gamma RAM)

u16 namcos23_state::c404_ram_r(offs_t offset)
{
	LOGMASKED(LOG_C404_RAM, "%s: c404_ram_r[%04x]: %04x\n", machine().describe_context(), offset, m_c404.ram[offset]);
	return m_c404.ram[offset];
}

void namcos23_state::c404_ram_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOGMASKED(LOG_C404_RAM, "%s: c404_ram_w[%04x] = %04x & %04x\n", machine().describe_context(), offset, data, mem_mask);
	COMBINE_DATA(&m_c404.ram[offset]);
}

void namcos23_state::c404_poly_fade_red_w(offs_t offset, u16 data) // 0
{
	LOGMASKED(LOG_C404_REGS, "%s: c404_poly_fade_red_w: %04x\n", machine().describe_context(), data);
	m_c404.poly_fade_r = m_c404.ram[0x00] = data;
}

void namcos23_state::c404_poly_fade_green_w(offs_t offset, u16 data) // 1
{
	LOGMASKED(LOG_C404_REGS, "%s: c404_poly_fade_green_w: %04x\n", machine().describe_context(), data);
	m_c404.poly_fade_g = m_c404.ram[0x01] = data;
}

void namcos23_state::c404_poly_fade_blue_w(offs_t offset, u16 data) // 2
{
	LOGMASKED(LOG_C404_REGS, "%s: c404_poly_fade_blue_w: %04x\n", machine().describe_context(), data);
	m_c404.poly_fade_b = m_c404.ram[0x02] = data;
}

void namcos23_state::c404_fog_red_w(offs_t offset, u16 data) // 5
{
	LOGMASKED(LOG_C404_REGS, "%s: c404_fog_red_w: %04x\n", machine().describe_context(), data);
	m_c404.fog_r = m_c404.ram[0x05] = data;
}

void namcos23_state::c404_fog_green_w(offs_t offset, u16 data) // 6
{
	LOGMASKED(LOG_C404_REGS, "%s: c404_fog_green_w: %04x\n", machine().describe_context(), data);
	m_c404.fog_g = m_c404.ram[0x06] = data;
}

void namcos23_state::c404_fog_blue_w(offs_t offset, u16 data) // 7
{
	LOGMASKED(LOG_C404_REGS, "%s: c404_fog_blue_w: %04x\n", machine().describe_context(), data);
	m_c404.fog_b = m_c404.ram[0x07] = data;
}

void namcos23_state::c404_bg_red_w(offs_t offset, u16 data) // 8
{
	LOGMASKED(LOG_C404_REGS, "%s: c404_bg_red_w: %04x\n", machine().describe_context(), data);
	m_c404.ram[0x08] = data;
	m_c404.bgcolor &= 0xff00ffff;
	m_c404.bgcolor |= (data & 0x00ff) << 16;
}

void namcos23_state::c404_bg_green_w(offs_t offset, u16 data) // 9
{
	LOGMASKED(LOG_C404_REGS, "%s: c404_bg_green_w: %04x\n", machine().describe_context(), data);
	m_c404.ram[0x09] = data;
	m_c404.bgcolor &= 0xffff00ff;
	m_c404.bgcolor |= (data & 0x00ff) << 8;
}

void namcos23_state::c404_bg_blue_w(offs_t offset, u16 data) // a
{
	LOGMASKED(LOG_C404_REGS, "%s: c404_bg_blue_w: %04x\n", machine().describe_context(), data);
	m_c404.ram[0x0a] = data;
	m_c404.bgcolor &= 0xffffff00;
	m_c404.bgcolor |= data & 0x00ff;
}

void namcos23_state::c404_spot_lsb_w(offs_t offset, u16 data) // d
{
	LOGMASKED(LOG_C404_REGS, "%s: c404_spot_lsb_w: %04x\n", machine().describe_context(), data);
	m_c404.ram[0x0d] = data;
	m_c404.spot_factor &= 0xff00;
	m_c404.spot_factor |= data & 0x00ff;
}

void namcos23_state::c404_spot_msb_w(offs_t offset, u16 data) // e
{
	LOGMASKED(LOG_C404_REGS, "%s: c404_spot_msb_w: %04x\n", machine().describe_context(), data);
	m_c404.ram[0x0e] = data;
	m_c404.spot_factor &= 0x00ff;
	m_c404.spot_factor |= (data & 0x00ff) << 8;
}

void namcos23_state::c404_poly_alpha_color_w(offs_t offset, u16 data) // f
{
	LOGMASKED(LOG_C404_REGS, "%s: c404_poly_alpha_color_w: %04x\n", machine().describe_context(), data);
	m_c404.poly_alpha_color = m_c404.ram[0x0f] = data;
}

void namcos23_state::c404_poly_alpha_pen_w(offs_t offset, u16 data) // 10
{
	LOGMASKED(LOG_C404_REGS, "%s: c404_poly_alpha_pen_w: %04x\n", machine().describe_context(), data);
	m_c404.poly_alpha_pen = m_c404.ram[0x10] = data;
}

void namcos23_state::c404_poly_alpha_w(offs_t offset, u16 data) // 11
{
	LOGMASKED(LOG_C404_REGS, "%s: c404_poly_alpha_w: %04x\n", machine().describe_context(), data);
	m_c404.poly_alpha = m_c404.ram[0x11] = data;
}

void namcos23_state::c404_alpha_check12_w(offs_t offset, u16 data) // 12
{
	LOGMASKED(LOG_C404_REGS, "%s: c404_alpha_check12_w: %04x\n", machine().describe_context(), data);
	m_c404.alpha_check12 = m_c404.ram[0x12] = data;
}

void namcos23_state::c404_alpha_check13_w(offs_t offset, u16 data) // 13
{
	LOGMASKED(LOG_C404_REGS, "%s: c404_alpha_check13_w: %04x\n", machine().describe_context(), data);
	m_c404.alpha_check13 = m_c404.ram[0x13] = data;
}

void namcos23_state::c404_text_alpha_mask_w(offs_t offset, u16 data) // 14
{
	LOGMASKED(LOG_C404_REGS, "%s: c404_text_alpha_mask_w: %04x\n", machine().describe_context(), data);
	m_c404.alpha_mask = m_c404.ram[0x14] = data;
}

void namcos23_state::c404_text_alpha_factor_w(offs_t offset, u16 data) // 15
{
	LOGMASKED(LOG_C404_REGS, "%s: c404_text_alpha_factor_w: %04x\n", machine().describe_context(), data);
	m_c404.alpha_factor = m_c404.ram[0x15] = data;
}

void namcos23_state::c404_screen_fade_red_w(offs_t offset, u16 data) // 16
{
	LOGMASKED(LOG_C404_REGS, "%s: c404_screen_fade_red_w: %04x\n", machine().describe_context(), data);
	m_c404.screen_fade_r = m_c404.ram[0x16] = data;
}

void namcos23_state::c404_screen_fade_green_w(offs_t offset, u16 data) // 17
{
	LOGMASKED(LOG_C404_REGS, "%s: c404_screen_fade_green_w: %04x\n", machine().describe_context(), data);
	m_c404.screen_fade_g = m_c404.ram[0x17] = data;
}

void namcos23_state::c404_screen_fade_blue_w(offs_t offset, u16 data) // 18
{
	LOGMASKED(LOG_C404_REGS, "%s: c404_screen_fade_blue_w: %04x\n", machine().describe_context(), data);
	m_c404.screen_fade_b = m_c404.ram[0x18] = data;
}

void namcos23_state::c404_screen_fade_factor_w(offs_t offset, u16 data) // 19
{
	LOGMASKED(LOG_C404_REGS, "%s: c404_screen_fade_factor_w: %04x\n", machine().describe_context(), data);
	m_c404.screen_fade_factor = m_c404.ram[0x19] = data;
}

void namcos23_state::c404_fade_flags_w(offs_t offset, u16 data) // 1a
{
	LOGMASKED(LOG_C404_REGS, "%s: c404_fade_flags_w: %04x\n", machine().describe_context(), data);
	m_c404.fade_flags = m_c404.ram[0x1a] = data;
}

void namcos23_state::c404_palette_base_w(offs_t offset, u16 data) // 1b
{
	LOGMASKED(LOG_C404_REGS, "%s: c404_palette_base_w: %04x\n", machine().describe_context(), data);
	m_c404.ram[0x1b] = data;
	m_c404.palbase = (data << 8) & 0x7f00;
}

void namcos23_state::c404_layer_flags_w(offs_t offset, u16 data) // 1f
{
	LOGMASKED(LOG_C404_REGS, "%s: c404_layer_flags_w: %04x\n", machine().describe_context(), data);
	m_c404.layer_flags = m_c404.ram[0x1f] = data;
}


// Video start/update callbacks

void namcos23_state::video_start()
{
	m_gfxdecode->gfx(0)->set_source(reinterpret_cast<u8 *>(m_charram.target()));
	m_mix_bitmap = std::make_unique<bitmap_ind16>(640, 480);
	m_bgtilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(namcos23_state::text_tilemap_get_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_bgtilemap->set_scroll_rows(64 * 16); // fake
	m_bgtilemap->set_transparent_pen(0xf);
	m_render.polymgr = std::make_unique<namcos23_renderer>(*this);

	m_ptrom  = (const u32 *)memregion("pointrom")->base();
	m_tmlrom = (const u16 *)memregion("textilemapl")->base();
	m_tmhrom = memregion("textilemaph")->base();
	m_texrom = memregion("textile")->base();

	m_tileid_mask = (memregion("textilemapl")->bytes()/2 - 1) & ~0xff; // Used for y masking
	m_tile_mask = memregion("textile")->bytes()/256 - 1;
	m_ptrom_limit = memregion("pointrom")->bytes()/4;

	for (int i = 0; i < m_gfxdecode->gfx(1)->elements(); i++)
		m_gfxdecode->gfx(1)->get_data(i);

	m_texture_tilemap = (u16 *)memregion("textile")->base();
	m_texture_tiledata = (u8 *)m_gfxdecode->gfx(1)->get_data(0);
}

void gorgon_state::video_start()
{
	namcos23_state::video_start();
	m_sprrom = memregion("sprites")->base();
}

void namcos23_state::mix_text_layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int prival)
{
	const pen_t *pens = m_palette->pens();
	u8 pen = 0;
	rgbaint_t rgb;

	// prepare fader
	bool fade_enabled = (m_c404.fade_flags & 2) && m_c404.screen_fade_factor;
	int fade_factor = 0xff - m_c404.screen_fade_factor;
	rgbaint_t fade_color(0, m_c404.screen_fade_r, m_c404.screen_fade_g, m_c404.screen_fade_b);

	// mix textlayer with poly/sprites
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		u16 const *const src = &m_mix_bitmap->pix(y);
		u8 const *const pri = &screen.priority().pix(y);
		u32 *dst = &bitmap.pix(y);
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++, dst++)
		{
			// skip if transparent or under poly/sprite
			if (pri[x] == prival)
			{
				rgb.set(pens[src[x]]);
				pen = src[x];

				// apply fade
				if (fade_enabled)
					rgb.blend(fade_color, fade_factor);

				// apply alpha
				if (m_c404.alpha_factor && ((pen & 0xf) == m_c404.alpha_mask || (pen >= m_c404.alpha_check12 && pen <= m_c404.alpha_check13)))
					rgb.blend(rgbaint_t(*dst), 0xff - m_c404.alpha_factor);

				*dst = rgb.to_rgba();
			}
		}
	}
}

u32 gorgon_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_c404.layer_flags & 2 && !machine().video().skip_this_frame())
		recalc_czram();

	return namcos23_state::screen_update(screen, bitmap, cliprect);
}

u32 namcos23_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (machine().video().skip_this_frame())
	{
		m_render.cur = !m_render.cur;
		m_render.count[m_render.cur] = 0;
		m_render.poly_count = 0;

		return UPDATE_HAS_NOT_CHANGED;
	}

	m_bgtilemap->set_palette_offset(m_c404.palbase);
	screen.priority().fill(0, cliprect);

	// background color
	rgbaint_t bg_color(m_c404.bgcolor);
	if (m_c404.fade_flags & 1 && m_c404.screen_fade_factor)
	{
		rgbaint_t fade_color(0, m_c404.screen_fade_r, m_c404.screen_fade_g, m_c404.screen_fade_b);
		bg_color.blend(fade_color, 0xff - m_c404.screen_fade_factor);
	}
	bitmap.fill(bg_color.to_rgba(), cliprect);

	if (m_c404.layer_flags & 4)
	{
		apply_text_scroll();
		m_bgtilemap->set_palette_offset(m_c404.palbase);

		m_bgtilemap->draw(screen, *m_mix_bitmap, cliprect, 0, 4, 4);
		mix_text_layer(screen, bitmap, cliprect, 4);
	}

	render_run(screen, bitmap);

	if (m_c404.layer_flags & 4)
	{
		mix_text_layer(screen, bitmap, cliprect, 6);
	}

	return 0;
}





/***************************************************************************

  Main CPU I/O + Memory Map
  (some cpu->video I/O handled above)

***************************************************************************/

// Interrupts

void namcos23_state::irq_update_common(u32 cause)
{
	const u32 old = m_main_irqcause;
	const u32 changed = cause ^ m_main_irqcause;
	m_main_irqcause = cause;

	// vblank
	if (changed & MAIN_VBLANK_IRQ)
	{
		if (m_main_irqcause & MAIN_VBLANK_IRQ)
			LOGMASKED(LOG_IRQ_STATUS, "Raising IRQ%d, main VBL\n", m_vbl_irqnum - MIPS3_IRQ0);
		else if (old & MAIN_VBLANK_IRQ)
			LOGMASKED(LOG_IRQ_STATUS, "Lowering IRQ%d, main VBL\n", m_vbl_irqnum - MIPS3_IRQ0);
		m_maincpu->set_input_line(m_vbl_irqnum, (cause & MAIN_VBLANK_IRQ) ? ASSERT_LINE : CLEAR_LINE);
	}

	// C422
	if (changed & MAIN_C422_IRQ)
	{
		if (m_main_irqcause & MAIN_C422_IRQ)
			LOGMASKED(LOG_IRQ_STATUS, "Raising IRQ%d, C422\n", m_c422_irqnum - MIPS3_IRQ0);
		else if (old & MAIN_C422_IRQ)
			LOGMASKED(LOG_IRQ_STATUS, "Lowering IRQ%d, C422\n", m_c422_irqnum - MIPS3_IRQ0);
		m_maincpu->set_input_line(m_c422_irqnum, (cause & MAIN_C422_IRQ) ? ASSERT_LINE : CLEAR_LINE);
	}
}

void namcos23_state::irq_update(u32 cause)
{
	const u32 old = m_main_irqcause;
	const u32 changed = cause ^ m_main_irqcause;

	irq_update_common(cause);

	// C361/subcpu
	if (changed & (MAIN_C361_IRQ | MAIN_SUBCPU_IRQ))
	{
		if (m_main_irqcause & (MAIN_C361_IRQ | MAIN_SUBCPU_IRQ))
			LOGMASKED(LOG_IRQ_STATUS, "Raising IRQ%d, C361(%d) || SubCPU(%d)\n", m_c361_irqnum - MIPS3_IRQ0, (m_main_irqcause & MAIN_C361_IRQ) ? 1 : 0, (m_main_irqcause & MAIN_SUBCPU_IRQ) ? 1 : 0);
		else if (old & (MAIN_C361_IRQ | MAIN_SUBCPU_IRQ))
			LOGMASKED(LOG_IRQ_STATUS, "Lowering IRQ%d\n", m_c361_irqnum - MIPS3_IRQ0);
		m_maincpu->set_input_line(m_c361_irqnum, (cause & (MAIN_C361_IRQ | MAIN_SUBCPU_IRQ)) ? ASSERT_LINE : CLEAR_LINE);
	}

	// C435
	if (changed & MAIN_C435_IRQ && m_c435_irqnum >= 0)
	{
		if (m_main_irqcause & MAIN_C435_IRQ)
			LOGMASKED(LOG_IRQ_STATUS, "Raising IRQ%d, C435\n", m_c435_irqnum - MIPS3_IRQ0);
		else if (old & MAIN_C435_IRQ)
			LOGMASKED(LOG_IRQ_STATUS, "Lowering IRQ%d, C435\n", m_c435_irqnum - MIPS3_IRQ0);
		m_maincpu->set_input_line(m_c435_irqnum, (cause & MAIN_C435_IRQ) ? ASSERT_LINE : CLEAR_LINE);
	}
}

void crszone_state::irq_update(u32 cause)
{
	const u32 old = m_main_irqcause;
	const u32 changed = cause ^ m_main_irqcause;

	irq_update_common(cause);

	// RS232
	if (changed & MAIN_RS232_IRQ)
	{
		if (m_main_irqcause & MAIN_RS232_IRQ)
			LOGMASKED(LOG_IRQ_STATUS, "Raising IRQ%d, RS232\n", m_rs232_irqnum - MIPS3_IRQ0);
		else if (old & MAIN_RS232_IRQ)
			LOGMASKED(LOG_IRQ_STATUS, "Lowering IRQ%d, RS232\n", m_rs232_irqnum - MIPS3_IRQ0);
		m_maincpu->set_input_line(m_rs232_irqnum, (cause & MAIN_RS232_IRQ) ? ASSERT_LINE : CLEAR_LINE);
	}

	// C361
	if (changed & MAIN_C361_IRQ)
	{
		if (m_main_irqcause & MAIN_C361_IRQ)
			LOGMASKED(LOG_IRQ_STATUS, "Raising IRQ%d, C361\n", m_c361_irqnum - MIPS3_IRQ0);
		else if (old & MAIN_C361_IRQ)
			LOGMASKED(LOG_IRQ_STATUS, "Lowering IRQ%d, C361\n", m_c361_irqnum - MIPS3_IRQ0);
		m_maincpu->set_input_line(m_c361_irqnum, (cause & MAIN_C361_IRQ) ? ASSERT_LINE : CLEAR_LINE);
	}

	// SubCPU
	if (changed & MAIN_SUBCPU_IRQ)
	{
		if (m_main_irqcause & MAIN_SUBCPU_IRQ)
			LOGMASKED(LOG_IRQ_STATUS, "Raising IRQ%d, SubCPU\n", m_sub_irqnum - MIPS3_IRQ0);
		else if (old & MAIN_SUBCPU_IRQ)
			LOGMASKED(LOG_IRQ_STATUS, "Lowering IRQ%d, SubCPU\n", m_sub_irqnum - MIPS3_IRQ0);
		m_maincpu->set_input_line(m_sub_irqnum, (cause & MAIN_SUBCPU_IRQ) ? ASSERT_LINE : CLEAR_LINE);
	}

	// C450
	if (changed & MAIN_C450_IRQ && m_c450_irqnum >= 0)
	{
		if (m_main_irqcause & MAIN_C450_IRQ)
			LOGMASKED(LOG_IRQ_STATUS, "Raising IRQ%d, C450\n", m_c450_irqnum - MIPS3_IRQ0);
		else if (old & MAIN_C450_IRQ)
			LOGMASKED(LOG_IRQ_STATUS, "Lowering IRQ%d, C450\n", m_c450_irqnum - MIPS3_IRQ0);
		m_maincpu->set_input_line(m_c450_irqnum, (cause & MAIN_C450_IRQ) ? ASSERT_LINE : CLEAR_LINE);
	}

	// C451
	if (changed & MAIN_C451_IRQ && m_c451_irqnum >= 0)
	{
		if (m_main_irqcause & MAIN_C451_IRQ)
			LOGMASKED(LOG_IRQ_STATUS, "Raising IRQ%d, C451\n", m_c451_irqnum - MIPS3_IRQ0);
		else if (old & MAIN_C451_IRQ)
			LOGMASKED(LOG_IRQ_STATUS, "Lowering IRQ%d, C451\n", m_c451_irqnum - MIPS3_IRQ0);
		m_maincpu->set_input_line(m_c451_irqnum, (cause & MAIN_C451_IRQ) ? ASSERT_LINE : CLEAR_LINE);
	}
}

void namcos23_state::subcpu_irq1_update(int state)
{
	m_subcpu->set_input_line(INPUT_LINE_IRQ1, state ? ASSERT_LINE : CLEAR_LINE);
	m_sub_port8 = state ? (m_sub_port8 & ~0x02) : (m_sub_port8 | 0x02); // IRQ1 pin
}

void namcos23_state::vblank(int state)
{
	if (state)
	{
		m_ctl_vbl_active = true;
		irq_update(m_main_irqcause | MAIN_VBLANK_IRQ);
	}
	else
	{
		irq_update(m_main_irqcause & ~MAIN_VBLANK_IRQ);
	}
	m_sub_portb = (m_sub_portb & 0x7f) | (state << 7);
	m_adc->adtrg_w(state);
}

void gorgon_state::vblank(int state)
{
	namcos23_state::vblank(state);
	subcpu_irq1_update(state);
}

TIMER_CALLBACK_MEMBER(namcos23_state::subcpu_scanline_on_tick)
{
	if (m_screen->vpos() < 72)
		subcpu_irq1_update(ASSERT_LINE);
}

TIMER_CALLBACK_MEMBER(namcos23_state::subcpu_scanline_off_tick)
{
	subcpu_irq1_update(CLEAR_LINE);
}


// C417

u16 namcos23_state::c417_status_r()
{
	/* According to timecrs2v4a, +0 is the status word with bits being:
	   15: test mode flag (huh?)
	   10: fifo data ready
	   9:  cmd ram data ready
	   8:  matrix busy
	   7:  output unit busy (inverted)
	   6:  hokan/tenso unit busy
	   5:  point unit busy
	   4:  access unit busy
	   3:  c403 busy, called c444 in 500gp (inverted)
	   2:  2nd c435 busy (inverted)
	   1:  1st c435 busy (inverted)
	   0:  xcpreq
	*/

	u16 data = 0x008e | (m_c417.test_mode << 15);
	LOGMASKED(LOG_C417_REG, "%s: c417_status_r: %04x\n", machine().describe_context(), data);
	return data;
}

u16 crszone_state::c417_status_r()
{
	u16 data = m_c417.test_mode << 15;
	LOGMASKED(LOG_C417_REG, "%s: c417_status_r: %04x\n", machine().describe_context(), data);
	return data;
}

u16 namcos23_state::c417_addr_r()
{
	LOGMASKED(LOG_C417_REG, "%s: c417_addr_r: %04x\n", machine().describe_context(), m_c417.adr);
	return m_c417.adr;
}

void namcos23_state::c417_addr_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOGMASKED(LOG_C417_REG, "%s: c417_addr_w: %04x & %04x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_c417.adr);
}

void namcos23_state::c417_ptrom_addr_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_c417.pointrom_adr = (m_c417.pointrom_adr << 16) | data;
	LOGMASKED(LOG_C417_REG, "%s: c417_ptrom_addr_w: %04x & %04x (now %08x)\n", machine().describe_context(), data, mem_mask, m_c417.pointrom_adr);
}

u16 namcos23_state::c417_test_done_r()
{
	LOGMASKED(LOG_C417_REG, "%s: c417_test_done_r: %04x\n", machine().describe_context(), 0);
	m_c417.test_mode = false;
	return 0;
}

void namcos23_state::c417_ptrom_addr_clear_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOGMASKED(LOG_C417_REG, "%s: c417_ptrom_addr_clear_w: %04x & %04x\n", machine().describe_context(), data, mem_mask);
	m_c417.pointrom_adr = 0;
	m_c417.test_mode = true;
}

u16 namcos23_state::c417_ram_r()
{
	LOGMASKED(LOG_C417_REG, "%s: c417 ram[%04x] read: %04x\n", machine().describe_context(), m_c417.adr, m_c417.ram[m_c417.adr]);
	return m_c417.ram[m_c417.adr];
}

void namcos23_state::c417_ram_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (!m_c417.test_mode)
		LOGMASKED(LOG_C417_REG, "%s: c417_ram_w: %04x = %04x & %04x\n", machine().describe_context(), m_c417.adr, data, mem_mask);
	COMBINE_DATA(m_c417.ram + m_c417.adr);
}

u16 namcos23_state::c417_ptrom_msw_r()
{
	if (m_c417.pointrom_adr >= m_ptrom_limit)
	{
		LOGMASKED(LOG_C417_REG, "%s: c417 point rom (over-limit) msw read: %04x\n", machine().describe_context(), 0xffff);
		return 0xffff;
	}
	LOGMASKED(LOG_C417_REG, "%s: c417 point rom[%06x] msw read: %04x\n", machine().describe_context(), m_c417.pointrom_adr, m_ptrom[m_c417.pointrom_adr] >> 16);
	return m_ptrom[m_c417.pointrom_adr] >> 16;
}

u16 namcos23_state::c417_ptrom_lsw_r()
{
	if (m_c417.pointrom_adr >= m_ptrom_limit)
	{
		LOGMASKED(LOG_C417_REG, "%s: c417 point rom (over-limit) lsw read: %04x\n", machine().describe_context(), 0xffff);
		return 0xffff;
	}
	// TODO: rapid river wants auto-inc in some way here (NGs point ROM self test otherwise)
	LOGMASKED(LOG_C417_REG, "%s: c417 point rom[%06x] lsw read: %04x\n", machine().describe_context(), m_c417.pointrom_adr, (u16)m_ptrom[m_c417.pointrom_adr]);
	return m_ptrom[m_c417.pointrom_adr];
}

void namcos23_state::c417_irq_ack_w(offs_t offset, u16 data)
{
	LOGMASKED(LOG_C417_ACK, "%s: c417_w: ack IRQ 2 (%x)\n", machine().describe_context(), data);
	irq_update(m_main_irqcause & ~MAIN_C435_IRQ);
}


// C412

u32 crszone_state::c450_irq_status_r(offs_t offset, u32 mem_mask)
{
	return (m_main_irqcause & MAIN_C450_IRQ) ? 1 : 0;
}

void crszone_state::c450_dma_addr_w(address_space &space, offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_C435_REG, "%s: c450 write DMA address @%08x: %08x & %08x\n", machine().describe_context(), 0x10400000, data, mem_mask);
	COMBINE_DATA(&m_c435.address);
}

void crszone_state::c450_dma_size_w(address_space &space, offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_C435_REG, "%s: c450 write DMA size @%08x: %08x & %08x (kicking DMA from %08x)\n", machine().describe_context(), 0x1040000c, data, mem_mask, m_c435.address);
	COMBINE_DATA(&m_c435.size);
	c435_dma(space, m_c435.address, m_c435.size);
	irq_update(m_main_irqcause | MAIN_C450_IRQ);
}

// C412

u16 namcos23_state::c412_ram_r(offs_t offset)
{
	//  logerror("c412_ram_r %06x (%08x, %08x)\n", offset, m_maincpu->pc(), (unsigned int)m_maincpu->state_int(MIPS3_R31));
	if (offset < 0x100000)
		return m_c412.sdram_a[offset & 0xfffff];
	else if (offset < 0x200000)
		return m_c412.sdram_b[offset & 0xfffff];
	else if (offset < 0x220000)
		return m_c412.sram[offset & 0x1ffff];
	else if (offset < 0x220200)
		return m_c412.pczram[offset & 0x1ff];

	return 0xffff;
}

void namcos23_state::c412_ram_w(offs_t offset, u16 data, u16 mem_mask)
{
	//  logerror("c412_ram_w %06x = %04x (%08x, %08x)\n", offset, data, m_maincpu->pc(), (unsigned int)m_maincpu->state_int(MIPS3_R31));
	if (offset < 0x100000)
	{
		//if (!m_c417.test_mode) LOGMASKED(LOG_C412_RAM, "C412 SDRAM A write: %08x = %04x & %04x\n", (offset << 1), data, mem_mask);
		COMBINE_DATA(m_c412.sdram_a + (offset & 0xfffff));
	}
	else if (offset < 0x200000)
	{
		//if (!m_c417.test_mode) LOGMASKED(LOG_C412_RAM, "C412 SDRAM B write: %08x = %04x & %04x\n", (offset << 1), data, mem_mask);
		COMBINE_DATA(m_c412.sdram_b + (offset & 0xfffff));
	}
	else if (offset < 0x220000)
	{
		if (!m_c417.test_mode) LOGMASKED(LOG_C412_RAM, "C412 SRAM write: %08x = %04x & %04x\n", (offset << 1), data, mem_mask);
		COMBINE_DATA(m_c412.sram    + (offset & 0x1ffff));
	}
	else if (offset < 0x220200)
	{
		if (!m_c417.test_mode) LOGMASKED(LOG_C412_RAM, "C412 PCZRAM write: %08x = %04x & %04x\n", (offset << 1), data, mem_mask);
		COMBINE_DATA(m_c412.pczram  + (offset & 0x1ff));
	}
	else
	{
		if (!m_c417.test_mode) LOGMASKED(LOG_C412_RAM, "C412 Unknown RAM write: %08x = %04x & %04x\n", (offset << 1), data, mem_mask);
	}
}

u16 namcos23_state::c412_r(offs_t offset, u16 mem_mask)
{
	switch (offset)
	{
	case 0x3:
		LOGMASKED(LOG_C412_UNK, "%s: read c412 offset %x & %04x: %04x\n", machine().describe_context(), offset, mem_mask, 0x0002);
		return 0x0002; // 0001 = busy, 0002 = game uploads things
	case 0x8:
		LOGMASKED(LOG_C412_UNK, "%s: read c412 offset %x & %04x: %04x\n", machine().describe_context(), offset, mem_mask, (u16)m_c412.adr);
		return m_c412.adr;
	case 0x9:
		LOGMASKED(LOG_C412_UNK, "%s: read c412 offset %x & %04x: %04x\n", machine().describe_context(), offset, mem_mask, (u16)(m_c412.adr >> 16));
		return m_c412.adr >> 16;
	case 0xa: {
		u16 data = c412_ram_r(m_c412.adr);
		return data;
	}
	case 0xc:
		// unknown status, 500gp reads it and waits for a transition
		// no other games use it?
		m_c412.status_c ^= 1;
		return m_c412.status_c;
	}

	LOGMASKED(LOG_C412_UNK, "%s: c412 unknown read %x & %04x\n", machine().describe_context(), offset, mem_mask);
	return 0;
}

void namcos23_state::c412_w(offs_t offset, u16 data, u16 mem_mask)
{
	switch (offset)
	{
	case 0x2:
		// d0: cz on
		// other bits: no function?
		LOGMASKED(LOG_C412_UNK, "%s: c412 write %x = %04x & %04x\n", machine().describe_context(), offset, data, mem_mask);
		break;
	case 0x8:
		m_c412.adr = (data & mem_mask) | (m_c412.adr & (0xffffffff ^ mem_mask));
		break;
	case 0x9:
		m_c412.adr = ((data & mem_mask) << 16) | (m_c412.adr & (0xffffffff ^ (mem_mask << 16)));
		break;
	case 0xa:
		c412_ram_w(m_c412.adr, data, mem_mask);
		m_c412.adr++;
		break;
	default:
		LOGMASKED(LOG_C412_UNK, "%s: c412 unknown write %x = %04x & %04x\n", machine().describe_context(), offset, data, mem_mask);
		break;
	}
}



// C421

u16 namcos23_state::c421_ram_r()
{
	offs_t offset = m_c421.adr & 0xfffff;
	if (offset < 0x40000)
		return m_c421.dram_a[offset & 0x3ffff];
	else if (offset < 0x80000)
		return m_c421.dram_b[offset & 0x3ffff];
	else if (offset < 0x88000)
		return m_c421.sram  [offset & 0x07fff];

	return 0xffff;
}

void namcos23_state::c421_ram_w(offs_t offset, u16 data, u16 mem_mask)
{
	offset = m_c421.adr & 0xfffff;
	if (offset < 0x40000)
		COMBINE_DATA(m_c421.dram_a + (offset & 0x3ffff));
	else if (offset < 0x80000)
		COMBINE_DATA(m_c421.dram_b + (offset & 0x3ffff));
	else if (offset < 0x88000)
		COMBINE_DATA(m_c421.sram   + (offset & 0x07fff));
	else
		LOGMASKED(LOG_C421_RAM, "C421 Unknown RAM write: %08x = %04x & %04x\n", (offset << 1), data, mem_mask);
	m_c421.adr += 2;
}

u16 namcos23_state::c421_addr_msw_r()
{
	LOGMASKED(LOG_C421_UNK, "%s: c421 RAM address MSW read: %04x\n", machine().describe_context(), (u16)(m_c421.adr >> 16));
	return (u16)(m_c421.adr >> 16);
}

void namcos23_state::c421_addr_msw_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_c421.adr = ((data & mem_mask) << 16) | (m_c421.adr & ~(mem_mask << 16));
}

u16 namcos23_state::c421_addr_lsw_r()
{
	LOGMASKED(LOG_C421_UNK, "%s: c421 RAM address LSW read: %04x\n", machine().describe_context(), (u16)m_c421.adr);
	return (u16)m_c421.adr;
}

void namcos23_state::c421_addr_lsw_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_c421.adr = (data & mem_mask) | (m_c421.adr & ~(u32)mem_mask);
}


// C422

u16 namcos23_state::c422_r(offs_t offset)
{
	LOGMASKED(LOG_C422_UNK, "%s: c422 reg[%d] read: %04x\n", machine().describe_context(), offset, m_c422.regs[offset]);
	return m_c422.regs[offset];
}

void namcos23_state::c422_w(offs_t offset, u16 data, u16 mem_mask)
{
	switch (offset)
	{
	case 1:
		LOGMASKED(LOG_C422_IRQ, "%s: c422_w: offset 1: %04x\n", machine().describe_context(), data);
		if (data == 0xfffb)
		{
			LOGMASKED(LOG_C422_IRQ, "%s: c422_w: raise IRQ 3\n", machine().describe_context());
			irq_update(m_main_irqcause | MAIN_C422_IRQ);
		}
		else if (data == 0x000f)
		{
			LOGMASKED(LOG_C422_IRQ, "%s: c422_w: ack IRQ 3\n", machine().describe_context());
			irq_update(m_main_irqcause & ~MAIN_C422_IRQ);
		}
		break;
	default:
		LOGMASKED(LOG_C422_UNK, "%s: c422 unknown write %x = %04x & %04x\n", machine().describe_context(), offset, data, mem_mask);
		break;
	}

	COMBINE_DATA(&m_c422.regs[offset]);
}



// C361 (text)

TIMER_CALLBACK_MEMBER(namcos23_state::c361_timer_cb)
{
	if (m_c361.scanline != 0x1ff)
	{
		LOGMASKED(LOG_C361_IRQ, "c361 timer callback, scanline %d\n", m_c361.scanline);
		// need to do a partial update here, but doesn't work properly yet
		irq_update(m_main_irqcause | MAIN_C361_IRQ);

		// TC2 indicates it's probably one-shot since it resets it each VBL...
		//c361.timer->adjust(m_screen->time_until_pos(c361.scanline));
	}
	else
	{
		LOGMASKED(LOG_C361_IRQ, "c361 timer callback, clearing irq\n", m_c361.scanline);
		irq_update(m_main_irqcause & ~MAIN_C361_IRQ);
	}
}

void namcos23_state::update_text_rowscroll()
{
	u64 frame = m_screen->frame_number();
	if (frame != m_c404.rowscroll_frame)
	{
		m_c404.rowscroll_frame = frame;
		m_c404.lastrow = 0;
	}

	int scroll_x = (m_c404.xscroll - 0x35c) & 0x3ff;
	int y = std::min(m_screen->vpos() + 4, 480); // +4 is a fudge factor, otherwise rowscrolls are 4 lines too early

	// save x scroll value until current scanline
	for (int i = m_c404.lastrow; i < y; i++)
		m_c404.rowscroll[i] = scroll_x;
	m_c404.lastrow = y;
}

void namcos23_state::apply_text_scroll()
{
	update_text_rowscroll();
	int scroll_y = m_c404.yscroll & 0x3ff;

	m_bgtilemap->set_scrolly(0, scroll_y);

	for (int i = 0; i < 0x400; i++)
		m_bgtilemap->set_scrollx(i, m_c404.rowscroll[0]);

	// apply current frame x scroll updates to tilemap
	for (int i = 0; i < 480; i++)
	{
		m_bgtilemap->set_scrollx((i + scroll_y + 4) & 0x3ff, m_c404.rowscroll[i]);
	}
}

void namcos23_state::c361_xscroll_w(offs_t offset, u16 data)
{
	LOGMASKED(LOG_C361_REG, "%s: c361_xscroll_w: %04x (scanline %d)\n", machine().describe_context(), data, m_screen->vpos());
	update_text_rowscroll();
	m_c404.xscroll = data;
}

void namcos23_state::c361_yscroll_w(offs_t offset, u16 data)
{
	LOGMASKED(LOG_C361_REG, "%s: c361_yscroll_w: %04x (scanline %d)\n", machine().describe_context(), data, m_screen->vpos());
	m_c404.yscroll = data;
}

void namcos23_state::c361_irq_scanline_w(offs_t offset, u16 data)
{
	LOGMASKED(LOG_C361_IRQ, "%s: c361_irq_scanline_w: %04x\n", machine().describe_context(), data);
	m_c361.scanline = (data & 0x1ff);
	m_c361.timer->adjust(m_screen->time_until_pos(m_c361.scanline));
}

u16 namcos23_state::c361_vpos_r()
{
	irq_update(m_main_irqcause & ~MAIN_C361_IRQ);
	return (m_screen->vpos() * 2) | (m_screen->vblank() ? 1 : 0);
}

u16 namcos23_state::c361_vblank_r()
{
	const u16 data = ((m_main_irqcause & MAIN_C361_IRQ) || m_screen->vblank()) ? 1 : 0;
	irq_update(m_main_irqcause & ~MAIN_C361_IRQ);
	return data;
}


// C?? (control)

void namcos23_state::direct_buf_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (offset == 1)
	{
		m_c435.direct_buf_open = (bool)data;
		m_c435.direct_buf_pos = 0;
		return;
	}

	if (!m_c435.direct_buf_open)
		return;

	if (m_c435.direct_buf_pos == 0)
	{
		m_c435.direct_buf_pos++;
		return;
	}
	m_c435.direct_buf[m_c435.direct_buf_pos - 1] = data;
	m_c435.direct_buf_pos++;
	m_c435.direct_buf_nonempty = true;

	if (m_c435.direct_buf_pos >= 29)
	{
		render_t &render = m_render;
		namcos23_render_entry *re = render.entries[render.cur] + render.count[render.cur];
		re->type = DIRECT;
		re->poly_fade_r = m_c404.poly_fade_r;
		re->poly_fade_g = m_c404.poly_fade_g;
		re->poly_fade_b = m_c404.poly_fade_b;
		re->poly_alpha_color = m_c404.poly_alpha_color;
		re->poly_alpha_pen = m_c404.poly_alpha_pen;
		re->poly_alpha = m_c404.poly_alpha;
		re->screen_fade_r = m_c404.screen_fade_r;
		re->screen_fade_g = m_c404.screen_fade_g;
		re->screen_fade_b = m_c404.screen_fade_b;
		re->screen_fade_factor = m_c404.screen_fade_factor;
		re->fade_flags = m_c404.fade_flags;
		re->absolute_priority = m_absolute_priority;
		re->model_blend_factor = 0;
		memcpy(re->direct.d, m_c435.direct_buf, sizeof(m_c435.direct_buf));
		render.count[render.cur]++;

		m_c435.direct_buf_nonempty = false;
		m_c435.direct_buf_pos = 0;
	}
}

void namcos23_state::ctl_leds_w(offs_t offset, u16 data)
{
	if (m_ctl_led != (data & 0xff))
	{
		m_ctl_led = data & 0xff;
		for (int i = 0; i < 8; i++)
			m_lamps[i] = BIT(data, 7 - i);
	}
}

u16 namcos23_state::ctl_status_r()
{
	// 0100 set freezes gorgon (polygon fifo flag)
	const u16 data = 0x0000 | ioport("DSW")->read() | ((m_main_irqcause & MAIN_C361_IRQ) ? 0x400 : 0);
	LOGMASKED(LOG_CTL_REG, "%s: ctl_status_r: %04x\n", machine().describe_context(), data);
	return data;
}

u16 namcos23_state::ctl_input1_r()
{
	const u16 data = m_ctl_inp_buffer[0] & 0x800 ? 0xffff : 0x0000;
	m_ctl_inp_buffer[0] = (m_ctl_inp_buffer[0] << 1) | 1;
	return data;
}

void namcos23_state::ctl_input1_w(offs_t offset, u16 data)
{
	// These may be coming from another CPU, in particular the I/O one
	m_ctl_inp_buffer[0] = m_p1->read();
}

u16 namcos23_state::ctl_input2_r()
{
	const u16 data = m_ctl_inp_buffer[1] & 0x800 ? 0xffff : 0x0000;
	m_ctl_inp_buffer[1] = (m_ctl_inp_buffer[1] << 1) | 1;
	return data;
}

void namcos23_state::ctl_input2_w(offs_t offset, u16 data)
{
	// These may be coming from another CPU, in particular the I/O one
	m_ctl_inp_buffer[1] = m_p2->read();
}

void namcos23_state::ctl_vbl_ack_w(offs_t offset, u16 data)
{
	if (m_ctl_vbl_active)
	{
		m_ctl_vbl_active = false;
		irq_update(m_main_irqcause & ~MAIN_VBLANK_IRQ);
	}
}

void namcos23_state::ctl_direct_poly_w(offs_t offset, u16 data)
{
	// gmen wars spams this heavily with 0 prior to starting the GMEN board test
	m_c435.direct_buf[m_c435.direct_buf_pos++] = data;
	if (data)
		m_c435.direct_buf_nonempty = true;
	if (m_c435.direct_buf_pos >= 28)
	{
		if (m_c435.direct_buf_nonempty)
		{
			render_t &render = m_render;
			namcos23_render_entry *re = render.entries[render.cur] + render.count[render.cur];
			if (!BIT(m_c435.direct_buf[0], 15))
			{
				re->type = DIRECT;
				re->poly_fade_r = m_c404.poly_fade_r;
				re->poly_fade_g = m_c404.poly_fade_g;
				re->poly_fade_b = m_c404.poly_fade_b;
				re->poly_alpha_color = m_c404.poly_alpha_color;
				re->poly_alpha_pen = m_c404.poly_alpha_pen;
				re->poly_alpha = m_c404.poly_alpha;
				re->screen_fade_r = m_c404.screen_fade_r;
				re->screen_fade_g = m_c404.screen_fade_g;
				re->screen_fade_b = m_c404.screen_fade_b;
				re->screen_fade_factor = m_c404.screen_fade_factor;
				re->fade_flags = m_c404.fade_flags;
				re->absolute_priority = m_absolute_priority;
				re->model_blend_factor = 0;
				memcpy(re->direct.d, m_c435.direct_buf, sizeof(m_c435.direct_buf));
				render.count[render.cur]++;
			}
		}
		m_c435.direct_buf_nonempty = false;
		m_c435.direct_buf_pos = 0;
	}
}


// C?? (MCU enable)

void namcos23_state::mcuen_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (offset != 7)
	{
		LOGMASKED(LOG_MCU, "%s: mcuen_w: %08x = %04x & %04x\n", machine().describe_context(), offset, data, mem_mask);
	}
	switch (offset)
	{
	case 2:
		// subcpu irq ack
		irq_update(m_main_irqcause & ~MAIN_SUBCPU_IRQ);
		break;

	case 5:
		// boot/start the audio mcu
		if (data)
		{
			// Panic Park: writing 1 when it's already running means reboot?
			if (m_subcpu_running)
			{
				LOGMASKED(LOG_MCU, "%s: mcuen_w: resetting H8/3002\n", machine().describe_context());
				m_subcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
			}

			m_subcpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
			m_subcpu_running = true;
		}
		else
		{
			LOGMASKED(LOG_MCU, "%s: mcuen_w: stopping H8/3002\n", machine().describe_context());
			m_subcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
			m_subcpu_running = false;
		}
		break;

	default:
		// For some reason, the main program writes the high 16bits of the first 0x28000
		// 32-bits words of itself to offset 7
		//logerror("mcuen_w: mask %04x, data %04x @ %x\n", mem_mask, data, offset);
		break;
	}
}


// C?? (unknown comms)

// 6850 ACIA comms info:
// while getting the subcpu to be ready, panicprk sits in a tight loop waiting for status AND 0002 to be non-zero (at PC=BFC02F00)
// bit 1 tx fifo empty
// bit 0 rx fifo ready
// bit 4-6: error (reads data port and discards it) (PC=0xbfc03698)
// data ready is signalled thru MIPS irq 4
// 3f0fa8 (phys address) is where data pops up in TC2 & PP
// PC=0xbfc03838 bit 7 high => fail (data loaded must be parsed somehow)
u16 namcos23_state::sub_comm_r(offs_t offset)
{
	// status register
	m_maincpu->set_input_line(m_rs232_irqnum, CLEAR_LINE);
	if (offset == 0)
	{
		LOGMASKED(LOG_MCU, "%s: sub_comm_r status read: %04x\n", machine().describe_context(), 3);
		return 1 | 2;
	}

	// data rx, TBD
	LOGMASKED(LOG_MCU, "%s: sub_comm_r data read: %04x\n", machine().describe_context(), 0);
	return 0;
}

void namcos23_state::sub_comm_w(offs_t offset, u8 data)
{
	LOGMASKED(LOG_MCU, "%s: sub_comm_w data write to %d: %02x\n", machine().describe_context(), data);
	if (offset == 1)
	{
		// data tx
		m_maincpu->set_input_line(m_rs232_irqnum, ASSERT_LINE);
	}
}

void crszone_state::irq_vbl_ack_w(offs_t offset, u32 data)
{
	irq_update(m_main_irqcause & ~MAIN_VBLANK_IRQ);
}

u32 crszone_state::irq_lv3_status_r()
{
	u32 data = 0;
	if (m_main_irqcause & MAIN_SUBCPU_IRQ)
	{
		data |= 1;
		irq_update(m_main_irqcause & ~MAIN_SUBCPU_IRQ);
	}
	if (m_main_irqcause & MAIN_C422_IRQ)
	{
		data |= 2;
		irq_update(m_main_irqcause & ~MAIN_C422_IRQ);
	}
	LOGMASKED(LOG_IRQ_STATUS, "%s: LV3 IRQ status read: %08x\n", machine().describe_context(), data);
	return data;
}

u32 crszone_state::irq_lv5_status_r()
{
	u32 data = 0;
	if (m_main_irqcause & MAIN_C451_IRQ)
	{
		data |= 1;
		irq_update(m_main_irqcause & ~MAIN_C451_IRQ);
	}
	if (m_main_irqcause & MAIN_C361_IRQ)
	{
		data |= 2;
		irq_update(m_main_irqcause & ~MAIN_C361_IRQ);
	}
	LOGMASKED(LOG_IRQ_STATUS, "%s: LV5 IRQ status read: %08x\n", machine().describe_context(), data);
	return data;
}

u32 crszone_state::irq_lv6_status_r()
{
	u32 data = 0;
	if (m_main_irqcause & MAIN_C450_IRQ)
	{
		data |= 2;
		irq_update(m_main_irqcause & ~MAIN_C450_IRQ);
	}
	LOGMASKED(LOG_IRQ_STATUS, "%s: LV6 IRQ status read: %08x\n", machine().describe_context(), data);
	return data;
}



/***************************************************************************

  MIPS CPU Memory Maps

***************************************************************************/

void namcos23_state::mips_base_map(address_map &map)
{
	map.global_mask(0xfffffff);
	map(0x00000000, 0x00ffffff).ram().share("mainram");

	map(0x0100001c, 0x0100001f).w(FUNC(namcos23_state::c435_dma_addr_w));
	map(0x01000020, 0x01000023).w(FUNC(namcos23_state::c435_dma_size_w));
	map(0x01000024, 0x01000027).w(FUNC(namcos23_state::c435_dma_start_w));
	map(0x01000028, 0x0100002b).r(FUNC(namcos23_state::c435_busy_flag_r));
	map(0x01000060, 0x01000063).w(FUNC(namcos23_state::c435_clear_bufpos_w));

	map(0x02000000, 0x02000001).rw(FUNC(namcos23_state::c417_status_r), FUNC(namcos23_state::c435_pio_w));
	map(0x02000002, 0x02000003).rw(FUNC(namcos23_state::c417_addr_r), FUNC(namcos23_state::c417_addr_w));
	map(0x02000004, 0x02000005).w(FUNC(namcos23_state::c417_ptrom_addr_w));
	map(0x02000006, 0x02000007).rw(FUNC(namcos23_state::c417_test_done_r), FUNC(namcos23_state::c417_ptrom_addr_clear_w));
	map(0x02000008, 0x02000009).rw(FUNC(namcos23_state::c417_ram_r), FUNC(namcos23_state::c417_ram_w));
	map(0x0200000a, 0x0200000b).r(FUNC(namcos23_state::c417_ptrom_msw_r));
	map(0x0200000c, 0x0200000d).r(FUNC(namcos23_state::c417_ptrom_lsw_r));
	map(0x0200000e, 0x0200000f).w(FUNC(namcos23_state::c417_irq_ack_w));

	map(0x04400000, 0x0440ffff).ram().share("shared_ram"); // Communication RAM (C416)
	map(0x04c3ff00, 0x04c3ff0f).w(FUNC(namcos23_state::mcuen_w));

	map(0x0d000000, 0x0d000001).w(FUNC(namcos23_state::ctl_leds_w));
	map(0x0d000002, 0x0d000003).r(FUNC(namcos23_state::ctl_status_r));
	map(0x0d000004, 0x0d000005).rw(FUNC(namcos23_state::ctl_input1_r), FUNC(namcos23_state::ctl_input1_w));
	map(0x0d000006, 0x0d000007).rw(FUNC(namcos23_state::ctl_input2_r), FUNC(namcos23_state::ctl_input2_w));
	map(0x0d00000a, 0x0d00000b).w(FUNC(namcos23_state::ctl_vbl_ack_w));
	map(0x0d00000c, 0x0d00000d).w(FUNC(namcos23_state::ctl_direct_poly_w));

	map(0x0fc00000, 0x0fffffff).nopw().rom().region("user1", 0);
}

void gorgon_state::mips_map(address_map &map)
{
	namcos23_state::mips_base_map(map);
	map(0x0100005c, 0x0100005f).w(FUNC(gorgon_state::c435_pio_mode_w));

	map(0x06000000, 0x06000003).w(FUNC(gorgon_state::nvram_w));
	map(0x06080000, 0x0608000f).rw(FUNC(gorgon_state::czattr_r), FUNC(gorgon_state::czattr_w)); // CZ Attribute RAM
	map(0x06080200, 0x060803ff).rw(FUNC(gorgon_state::czram_r), FUNC(gorgon_state::czram_w));

	c404_map(map, 0x06108000);

	map(0x06110000, 0x0613ffff).ram().w(FUNC(gorgon_state::paletteram_w)).share("paletteram"); // Palette RAM (C404)
	map(0x06300000, 0x06300007).w(FUNC(gorgon_state::sprites_w));
	map(0x06400000, 0x0641dfff).ram().w(FUNC(gorgon_state::textchar_w)).share("charram"); // Text CGRAM (C361)
	map(0x0641e000, 0x0641ffff).ram().w(FUNC(gorgon_state::textram_w)).share("textram"); // Text VRAM (C361)

	c361_map(map, 0x06420000);

	map(0x08000000, 0x087fffff).rom().region("data", 0x000000); // data ROMs

	map(0x0c000000, 0x0c00ffff).ram().share("nvram"); // Backup RAM

	map(0x0f000000, 0x0f000003).rw(FUNC(gorgon_state::sub_comm_r), FUNC(gorgon_state::sub_comm_w));
	map(0x0f200000, 0x0f203fff).ram(); // C422 RAM
	map(0x0f300000, 0x0f30000f).rw(FUNC(gorgon_state::c422_r), FUNC(gorgon_state::c422_w)); // C422 registers
}

// (Super) System 23
void namcos23_state::mips_map(address_map &map)
{
	namcos23_state::mips_base_map(map);

	map(0x06000000, 0x0600ffff).ram().share("nvram"); // Backup RAM
	map(0x06200000, 0x06203fff).ram(); // C422 RAM
	map(0x06400000, 0x0640000f).rw(FUNC(namcos23_state::c422_r), FUNC(namcos23_state::c422_w)); // C422 registers
	map(0x06800000, 0x0681dfff).ram().w(FUNC(namcos23_state::textchar_w)).share("charram"); // Text CGRAM (C361)
	map(0x0681e000, 0x0681ffff).ram().w(FUNC(namcos23_state::textram_w)).share("textram"); // Text VRAM (C361)

	c361_map(map, 0x06820000);
	c404_map(map, 0x06a08000);

	map(0x06a10000, 0x06a3ffff).ram().w(FUNC(namcos23_state::paletteram_w)).share("paletteram"); // Palette RAM (C404)
	map(0x08000000, 0x08ffffff).rom().region("data", 0x0000000).mirror(0x1000000); // data ROMs
	map(0x0a000000, 0x0affffff).rom().region("data", 0x1000000).mirror(0x1000000);
	map(0x0c000000, 0x0c00001f).rw(FUNC(namcos23_state::c412_r), FUNC(namcos23_state::c412_w));

	map(0x0c400000, 0x0c400001).rw(FUNC(namcos23_state::c421_ram_r), FUNC(namcos23_state::c421_ram_w));
	map(0x0c400004, 0x0c400005).rw(FUNC(namcos23_state::c421_addr_msw_r), FUNC(namcos23_state::c421_addr_msw_w));
	map(0x0c400006, 0x0c400007).rw(FUNC(namcos23_state::c421_addr_lsw_r), FUNC(namcos23_state::c421_addr_lsw_w));

	map(0x0c800010, 0x0c800011).w(FUNC(namcos23_state::c435_state_reset_w));
	map(0x0c800016, 0x0c800017).w(FUNC(namcos23_state::c435_state_pio_w));
	map(0x0cc00000, 0x0cc00003).w(FUNC(namcos23_state::direct_buf_w));

	map(0x0e800000, 0x0e800003).rw(FUNC(namcos23_state::sub_comm_r), FUNC(namcos23_state::sub_comm_w)); // not sure
}

void crszone_state::mips_map(address_map &map)
{
	map.global_mask(0x1fffffff);
	map(0x00000000, 0x00ffffff).ram().share("mainram");

	map(0x10000058, 0x1000005b).nopw();
	map(0x10000080, 0x10000083).w(FUNC(crszone_state::irq_vbl_ack_w));
	map(0x100000a8, 0x100000ab).r(FUNC(crszone_state::irq_lv3_status_r));
	map(0x100000b0, 0x100000b3).r(FUNC(crszone_state::irq_lv5_status_r));
	map(0x100000b4, 0x100000b7).r(FUNC(crszone_state::irq_lv6_status_r));

	map(0x10400000, 0x10400003).w(FUNC(crszone_state::c450_dma_addr_w));
	map(0x1040000c, 0x1040000f).w(FUNC(crszone_state::c450_dma_size_w));
	map(0x1040001c, 0x1040001f).r(FUNC(crszone_state::c450_irq_status_r));

	map(0x12000000, 0x12000001).rw(FUNC(crszone_state::c417_status_r), FUNC(crszone_state::c435_pio_w));
	map(0x12000002, 0x12000003).rw(FUNC(crszone_state::c417_addr_r), FUNC(crszone_state::c417_addr_w));
	map(0x12000004, 0x12000005).w(FUNC(crszone_state::c417_ptrom_addr_w));
	map(0x12000006, 0x12000007).rw(FUNC(crszone_state::c417_test_done_r), FUNC(crszone_state::c417_ptrom_addr_clear_w));
	map(0x12000008, 0x12000009).rw(FUNC(crszone_state::c417_ram_r), FUNC(crszone_state::c417_ram_w));
	map(0x1200000a, 0x1200000b).r(FUNC(crszone_state::c417_ptrom_msw_r));
	map(0x1200000c, 0x1200000d).rw(FUNC(crszone_state::c417_ptrom_lsw_r), FUNC(crszone_state::c417_irq_ack_w));

	map(0x14400000, 0x1440ffff).ram().share("shared_ram"); // Communication RAM (C416)
	map(0x14c3ff00, 0x14c3ff0f).w(FUNC(crszone_state::mcuen_w));
	map(0x16000000, 0x1600ffff).ram().share("nvram"); // Backup RAM
	map(0x16200000, 0x16203fff).ram(); // C422 RAM
	map(0x16400000, 0x1640000f).rw(FUNC(crszone_state::c422_r), FUNC(crszone_state::c422_w)); // C422 registers
	map(0x16800000, 0x1681dfff).ram().w(FUNC(crszone_state::textchar_w)).share("charram"); // Text CGRAM (C361)
	map(0x1681e000, 0x1681ffff).ram().w(FUNC(crszone_state::textram_w)).share("textram"); // Text VRAM (C361)

	c361_map(map, 0x16820000);
	c404_map(map, 0x16a08000);

	map(0x16a10000, 0x16a3ffff).ram().w(FUNC(crszone_state::paletteram_w)).share("paletteram"); // Palette RAM (C404)
	map(0x18000000, 0x18ffffff).rom().region("data", 0x0000000).mirror(0x1000000); // data ROMs
	map(0x1a000000, 0x1affffff).rom().region("data", 0x1000000).mirror(0x1000000);
	map(0x1c000000, 0x1c00001f).rw(FUNC(crszone_state::c412_r), FUNC(crszone_state::c412_w));

	map(0x1c400000, 0x1c400001).rw(FUNC(crszone_state::c421_ram_r), FUNC(crszone_state::c421_ram_w));
	map(0x1c400004, 0x1c400005).rw(FUNC(crszone_state::c421_addr_msw_r), FUNC(crszone_state::c421_addr_msw_w));
	map(0x1c400006, 0x1c400007).rw(FUNC(crszone_state::c421_addr_lsw_r), FUNC(crszone_state::c421_addr_lsw_w));

	map(0x1c800010, 0x1c800011).w(FUNC(crszone_state::c435_state_reset_w));
	map(0x1c800016, 0x1c800017).w(FUNC(crszone_state::c435_state_pio_w));
	// map(0x1cc00000, 0x1cc00003).w(FUNC(crszone_state::direct_buf_w)); // Currently commented out due to obstructing other geometry

	map(0x1d000000, 0x1d000001).w(FUNC(crszone_state::ctl_leds_w));
	map(0x1d000002, 0x1d000003).r(FUNC(crszone_state::ctl_status_r));
	map(0x1d000004, 0x1d000005).rw(FUNC(crszone_state::ctl_input1_r), FUNC(crszone_state::ctl_input1_w));
	map(0x1d000006, 0x1d000007).rw(FUNC(crszone_state::ctl_input2_r), FUNC(crszone_state::ctl_input2_w));
	map(0x1d00000a, 0x1d00000b).w(FUNC(crszone_state::ctl_vbl_ack_w));
	map(0x1d00000c, 0x1d00000d).w(FUNC(crszone_state::ctl_direct_poly_w));

	map(0x1f000000, 0x1f000003).rw(FUNC(crszone_state::acia_r), FUNC(crszone_state::acia_w));
	map(0x1f800000, 0x1fffffff).nopw().rom().region("user1", 0);
}

void namcos23_state::c361_map(address_map &map, const u32 addr)
{
	map(addr + 0x0, addr + 0x1).w(FUNC(namcos23_state::c361_xscroll_w));
	map(addr + 0x2, addr + 0x3).w(FUNC(namcos23_state::c361_yscroll_w));
	map(addr + 0x8, addr + 0x9).w(FUNC(namcos23_state::c361_irq_scanline_w));
	map(addr + 0xa, addr + 0xb).r(FUNC(namcos23_state::c361_vpos_r));
	map(addr + 0xc, addr + 0xd).r(FUNC(namcos23_state::c361_vblank_r));
}

void namcos23_state::c404_map(address_map &map, const u32 addr)
{
	map(addr + 0x000, addr + 0x7ff).rw(FUNC(namcos23_state::c404_ram_r), FUNC(namcos23_state::c404_ram_w));
	map(addr + 0x000, addr + 0x001).w(FUNC(namcos23_state::c404_poly_fade_red_w));
	map(addr + 0x002, addr + 0x003).w(FUNC(namcos23_state::c404_poly_fade_green_w));
	map(addr + 0x004, addr + 0x005).w(FUNC(namcos23_state::c404_poly_fade_blue_w));
	map(addr + 0x00a, addr + 0x00b).w(FUNC(namcos23_state::c404_fog_red_w));
	map(addr + 0x00c, addr + 0x00d).w(FUNC(namcos23_state::c404_fog_green_w));
	map(addr + 0x00e, addr + 0x00f).w(FUNC(namcos23_state::c404_fog_blue_w));
	map(addr + 0x010, addr + 0x011).w(FUNC(namcos23_state::c404_bg_red_w));
	map(addr + 0x012, addr + 0x013).w(FUNC(namcos23_state::c404_bg_green_w));
	map(addr + 0x014, addr + 0x015).w(FUNC(namcos23_state::c404_bg_blue_w));
	map(addr + 0x01a, addr + 0x01b).w(FUNC(namcos23_state::c404_spot_lsb_w));
	map(addr + 0x01c, addr + 0x01d).w(FUNC(namcos23_state::c404_spot_msb_w));
	map(addr + 0x01e, addr + 0x01f).w(FUNC(namcos23_state::c404_poly_alpha_color_w));
	map(addr + 0x020, addr + 0x021).w(FUNC(namcos23_state::c404_poly_alpha_pen_w));
	map(addr + 0x022, addr + 0x023).w(FUNC(namcos23_state::c404_poly_alpha_w));
	map(addr + 0x024, addr + 0x025).w(FUNC(namcos23_state::c404_alpha_check12_w));
	map(addr + 0x026, addr + 0x027).w(FUNC(namcos23_state::c404_alpha_check13_w));
	map(addr + 0x028, addr + 0x029).w(FUNC(namcos23_state::c404_text_alpha_mask_w));
	map(addr + 0x02a, addr + 0x02b).w(FUNC(namcos23_state::c404_text_alpha_factor_w));
	map(addr + 0x02c, addr + 0x02d).w(FUNC(namcos23_state::c404_screen_fade_red_w));
	map(addr + 0x02e, addr + 0x02f).w(FUNC(namcos23_state::c404_screen_fade_green_w));
	map(addr + 0x030, addr + 0x031).w(FUNC(namcos23_state::c404_screen_fade_blue_w));
	map(addr + 0x032, addr + 0x033).w(FUNC(namcos23_state::c404_screen_fade_factor_w));
	map(addr + 0x034, addr + 0x035).w(FUNC(namcos23_state::c404_fade_flags_w));
	map(addr + 0x036, addr + 0x037).w(FUNC(namcos23_state::c404_palette_base_w));
	map(addr + 0x03e, addr + 0x03f).w(FUNC(namcos23_state::c404_layer_flags_w));
}

void namcoss23_state::mips_map(address_map &map)
{
	namcos23_state::mips_map(map);
	map(0x0100005c, 0x0100005f).w(FUNC(namcoss23_state::c435_c361_ack_w));
}

// GMEN interface

u32 namcoss23_gmen_state::sh2_trigger_r()
{
	LOGMASKED(LOG_SH2, "%s: sh2_trigger_r: booting SH-2\n", machine().describe_context());
	m_sh2->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);

	return 0;
}

u32 namcoss23_gmen_state::sh2_shared_r(offs_t offset, u32 mem_mask)
{
	return m_sh2_shared[offset];
}

void namcoss23_gmen_state::sh2_shared_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_sh2_shared[offset]);
}

u32 namcoss23_gmen_state::sh2_dsw_r(offs_t offset, u32 mem_mask)
{
	return m_dsw->read() << 24;
}

u32 namcoss23_gmen_state::mips_sh2_unk_r(offs_t offset, u32 mem_mask)
{
	logerror("%s: mips_sh2_unk_r: %08x & %08x\n", machine().describe_context().c_str(), m_sh2_unk, mem_mask);
	return m_sh2_unk;
}

u32 namcoss23_gmen_state::sh2_unk_r(offs_t offset, u32 mem_mask)
{
	return 0;
}

void namcoss23_gmen_state::sh2_unk_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_sh2_unk);
}

u32 namcoss23_gmen_state::sh2_unk6200000_r(offs_t offset, u32 mem_mask)
{
	return 0x04000000;
}

void namcoss23_gmen_state::vpx_i2c_sdao_w(int state)
{
	m_vpx_sdao = state;
}

u8 namcoss23_gmen_state::vpx_i2c_r() {
	//LOGMASKED(LOG_GMEN, "%s: vpx_i2c_r: %02x\n", machine().describe_context().c_str(), m_vpx_sdao);
	return m_vpx_sdao;
}

void namcoss23_gmen_state::vpx_i2c_w(u8 data) {
	//LOGMASKED(LOG_GMEN, "%s: vpx_i2c_w: %02x\n", machine().describe_context().c_str(), data);
	m_vpx->sda_write(BIT(data, 0));
	m_vpx->scl_write(BIT(data, 1));
}

u32 namcoss23_gmen_state::sh2_kludge_r()
{
	return 0x22115566;
}

void namcoss23_gmen_state::mips_map(address_map &map)
{
	namcoss23_state::mips_map(map);
	map(0x0e400000, 0x0e400003).r(FUNC(namcoss23_gmen_state::sh2_trigger_r));
	map(0x0e600000, 0x0e600003).r(FUNC(namcoss23_gmen_state::mips_sh2_unk_r));
	map(0x0e700000, 0x0e70ffff).rw(FUNC(namcoss23_gmen_state::sh2_shared_r), FUNC(namcoss23_gmen_state::sh2_shared_w));
	map(0x0e70c000, 0x0e70c003).r(FUNC(namcoss23_gmen_state::sh2_kludge_r));
}

void namcoss23_gmen_state::sh2_map(address_map &map)
{
	map(0x00000000, 0x0000ffff).mirror(0x01000000).rw(FUNC(namcoss23_gmen_state::sh2_shared_r), FUNC(namcoss23_gmen_state::sh2_shared_w));
	map(0x04000000, 0x043fffff).ram(); // SH-2 main work RAM (SDRAM)
	map(0x01800000, 0x01bfffff).ram();
	map(0x02800000, 0x02800003).rw(FUNC(namcoss23_gmen_state::sh2_unk_r), FUNC(namcoss23_gmen_state::sh2_unk_w));
	map(0x03000000, 0x03000003).r(FUNC(namcoss23_gmen_state::sh2_dsw_r));
	map(0x06000000, 0x06000003).umask32(0xff000000).rw(FUNC(namcoss23_gmen_state::vpx_i2c_r), FUNC(namcoss23_gmen_state::vpx_i2c_w));
	map(0x06200000, 0x06200003).r(FUNC(namcoss23_gmen_state::sh2_unk6200000_r));
	//map(0x06600000, 0x06600003).nopw();
	map(0x00c00000, 0x00c0006b).m(m_firewire, FUNC(md8412b_device::map));
}





/***************************************************************************

  Sub CPU (H8/3002 MCU) I/O + Memory Map

***************************************************************************/

void namcos23_state::sharedram_sub_w(offs_t offset, u16 data, u16 mem_mask)
{
	u16 *shared16 = reinterpret_cast<u16 *>(m_shared_ram.target());
	COMBINE_DATA(&shared16[BYTE_XOR_BE(offset)]);
}

u16 namcos23_state::sharedram_sub_r(offs_t offset)
{
	u16 *shared16 = reinterpret_cast<u16 *>(m_shared_ram.target());
	return shared16[BYTE_XOR_BE(offset)];
}

void namcos23_state::sub_interrupt_main_w(offs_t offset, u16 data, u16 mem_mask)
{
	if ((mem_mask == 0xffff) && (data == 0x3170))
		irq_update(m_main_irqcause | MAIN_SUBCPU_IRQ);
	else
		LOGMASKED(LOG_SUBIRQ, "%s: Unknown write %x to sub_interrupt_main_w!\n", machine().describe_context(), data);
}

void crszone_state::acia_irq_w(int state)
{
	if (state)
		irq_update(m_main_irqcause | MAIN_RS232_IRQ);
	else
		irq_update(m_main_irqcause & ~MAIN_RS232_IRQ);
}

u8 crszone_state::acia_r(offs_t offset)
{
	u8 data = 0;
	if (offset == 1)
	{
		data = m_acia->status_r();
		LOGMASKED(LOG_RS232, "%s: ACIA S23 status read: %02x\n", machine().describe_context(), data);
		return data;
	}
	if (offset == 3)
	{
		data = m_acia->data_r();
		LOGMASKED(LOG_RS232, "%s: ACIA S23 data read: %02x\n", machine().describe_context(), data);
		return data;
	}
	LOGMASKED(LOG_RS232, "%s: ACIA S23 read %d: %02x\n", machine().describe_context(), offset, data);
	return data;
}

void crszone_state::acia_w(offs_t offset, u8 data)
{
	if (offset == 1)
	{
		LOGMASKED(LOG_RS232, "%s: ACIA control write = %02x\n", machine().describe_context(), data);
		m_acia->control_w(data);
		return;
	}
	LOGMASKED(LOG_RS232, "%s: ACIA S23 write %d = %02x\n", machine().describe_context(), offset, data);
}

// Port 6

u8 namcos23_state::mcu_p6_r()
{
	// bit 1 = JVS cable present sense (1 = I/O board plugged in)
	const u8 data = ((m_jvs_sense != jvs_port_device::sense::Initialized) << 1) | 0xfd;
	LOGMASKED(LOG_MCU_PORTS, "%s: mcu_p6_r: %02x\n", machine().describe_context(), data);
	return data;
}

void namcos23_state::mcu_p6_w(u8 data)
{
	LOGMASKED(LOG_MCU_PORTS, "%s: mcu_p6_w: %02x\n", machine().describe_context(), data);
	static u8 old_data = 0;
	if (data != old_data)
	{
		const u8 changed = data ^ old_data;
		for (int i = 0; i < 8; i++)
		{
			if (!BIT(changed, i))
				continue;
			LOGMASKED(LOG_MCU_PORTS, "%s:           Bit %d changed (now %d)\n", machine().describe_context(), i, BIT(data, i));
		}
		old_data = data;
	}
}



// Port 8

u8 namcos23_state::mcu_p8_r()
{
	LOGMASKED(LOG_MCU_PORTS, "%s: mcu_p8_r: %02x\n", machine().describe_context(), m_sub_port8);
	return m_sub_port8;
}

void namcos23_state::mcu_p8_w(u8 data)
{
	LOGMASKED(LOG_MCU_PORTS, "%s: mcu_p8_w: %02x\n", machine().describe_context(), data);
	static u8 old_data = 0;
	if (data != old_data)
	{
		const u8 changed = data ^ old_data;
		for (int i = 0; i < 8; i++)
		{
			if (!BIT(changed, i))
				continue;
			LOGMASKED(LOG_MCU_PORTS, "%s:           Bit %d changed (now %d)\n", machine().describe_context(), i, BIT(data, i));
		}
		old_data = data;
	}
	m_sub_port8 = (data & ~0x02) | (m_sub_port8 & 0x02);
}



// Port A

u8 namcos23_state::mcu_pa_r()
{
	LOGMASKED(LOG_MCU_PORTS, "%s: mcu_pa_r: %02x\n", machine().describe_context(), m_sub_porta);
	return m_sub_porta;
}

void namcos23_state::mcu_pa_w(u8 data)
{
	LOGMASKED(LOG_MCU_PORTS, "%s: mcu_pa_w: %02x\n", machine().describe_context(), data);
	static u8 old_data = 0;
	if (data != old_data)
	{
		const u8 changed = data ^ old_data;
		for (int i = 0; i < 8; i++)
		{
			if (!BIT(changed, i))
				continue;
			LOGMASKED(LOG_MCU_PORTS, "%s:           Bit %d changed (now %d)\n", machine().describe_context(), i, BIT(data, i));
		}
		old_data = data;
	}
	m_sub_porta = data;
	m_rtc->ce_w((m_sub_portb & 0x20) && (m_sub_porta & 1));
	m_settings->ce_w((m_sub_portb & 0x20) && !(m_sub_porta & 1));
}



// Port B

u8 namcos23_state::mcu_pb_r()
{
	LOGMASKED(LOG_MCU_PORTS, "%s: mcu_pb_r: %02x\n", machine().describe_context(), m_sub_portb);
	return m_sub_portb;
}

void namcos23_state::mcu_pb_w(u8 data)
{
	LOGMASKED(LOG_MCU_PORTS, "%s: mcu_pb_w: %02x\n", machine().describe_context(), data);
	static u8 old_data = 0;
	if (data != old_data)
	{
		const u8 changed = data ^ old_data;
		for (int i = 0; i < 8; i++)
		{
			if (!BIT(changed, i))
				continue;
			LOGMASKED(LOG_MCU_PORTS, "%s:           Bit %d changed (now %d)\n", machine().describe_context(), i, BIT(data, i));
		}
		old_data = data;
	}
	m_sub_portb = (m_sub_portb & 0xc0) | (data & 0x3f);
	m_rtc->ce_w((m_sub_portb & 0x20) && (m_sub_porta & 1));
	m_settings->ce_w((m_sub_portb & 0x20) && !(m_sub_porta & 1));
	m_jvs->rts(BIT(m_sub_portb, 0));
}


void namcos23_state::s23h8rwmap(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x080000, 0x08ffff).rw(FUNC(namcos23_state::sharedram_sub_r), FUNC(namcos23_state::sharedram_sub_w));
	map(0x280000, 0x287fff).rw("c352", FUNC(c352_device::read), FUNC(c352_device::write));
	map(0x300000, 0x300003).noprw(); // seems to be more inputs, maybe false leftover code from System 12?
	map(0x300010, 0x300011).noprw();
	map(0x300020, 0x300021).w(FUNC(namcos23_state::sub_interrupt_main_w));
	map(0x300030, 0x300031).nopw(); // timecrs2 writes this when writing to the sync shared ram location, motoxgo doesn't
}


/***************************************************************************

  Machine Drivers

***************************************************************************/

void namcos23_state::machine_start()
{
	save_item(NAME(m_c404.poly_fade_r));
	save_item(NAME(m_c404.poly_fade_g));
	save_item(NAME(m_c404.poly_fade_b));
	save_item(NAME(m_c404.fog_r));
	save_item(NAME(m_c404.fog_g));
	save_item(NAME(m_c404.fog_b));
	save_item(NAME(m_c404.bgcolor));
	save_item(NAME(m_c404.spot_factor));
	save_item(NAME(m_c404.poly_alpha_color));
	save_item(NAME(m_c404.poly_alpha_pen));
	save_item(NAME(m_c404.poly_alpha));
	save_item(NAME(m_c404.alpha_check12));
	save_item(NAME(m_c404.alpha_check13));
	save_item(NAME(m_c404.alpha_mask));
	save_item(NAME(m_c404.alpha_factor));
	save_item(NAME(m_c404.screen_fade_r));
	save_item(NAME(m_c404.screen_fade_g));
	save_item(NAME(m_c404.screen_fade_b));
	save_item(NAME(m_c404.screen_fade_factor));
	save_item(NAME(m_c404.fade_flags));
	save_item(NAME(m_c404.palbase));
	save_item(NAME(m_c404.layer_flags));
	save_item(NAME(m_c404.ram));
	save_item(NAME(m_c404.spritedata_idx));
	save_item(NAME(m_c404.rowscroll_frame));
	save_item(NAME(m_c404.rowscroll));
	save_item(NAME(m_c404.lastrow));
	save_item(NAME(m_c404.xscroll));
	save_item(NAME(m_c404.yscroll));
	save_item(STRUCT_MEMBER(m_c404.sprites, d));

	save_item(NAME(m_c361.scanline));

	save_item(NAME(m_c417.ram));
	save_item(NAME(m_c417.adr));
	save_item(NAME(m_c417.pointrom_adr));
	save_item(NAME(m_c417.test_mode));

	save_item(NAME(m_c412.sdram_a));
	save_item(NAME(m_c412.sdram_b));
	save_item(NAME(m_c412.sram));
	save_item(NAME(m_c412.pczram));
	save_item(NAME(m_c412.adr));
	save_item(NAME(m_c412.status_c));

	save_item(NAME(m_c421.dram_a));
	save_item(NAME(m_c421.dram_b));
	save_item(NAME(m_c421.sram));
	save_item(NAME(m_c421.adr));

	save_item(NAME(m_c422.regs));

	save_item(NAME(m_jvs_sense));
	save_item(NAME(m_main_irqcause));

	save_item(NAME(m_ctl_vbl_active));
	save_item(NAME(m_ctl_led));
	save_item(NAME(m_ctl_inp_buffer));

	save_item(NAME(m_subcpu_running));

	save_item(NAME(m_c435.address));
	save_item(NAME(m_c435.size));
	save_item(NAME(m_c435.buffer));
	save_item(NAME(m_c435.buffer_pos));
	save_item(NAME(m_c435.direct_buf));
	save_item(NAME(m_c435.direct_vertex));
	save_item(NAME(m_c435.direct_vert_pos));
	save_item(NAME(m_c435.direct_buf_pos));
	save_item(NAME(m_c435.direct_buf_nonempty));
	save_item(NAME(m_c435.direct_buf_open));
	save_item(NAME(m_c435.pio_mode));
	save_item(NAME(m_c435.sprite_target));
	save_item(NAME(m_c435.spritedata));

	save_item(NAME(m_ptrom_limit));
	save_item(NAME(m_proj_matrix));
	save_item(NAME(m_proj_matrix_line));

	save_item(NAME(m_absolute_priority));
	save_item(NAME(m_model_blend_factor));
	save_item(NAME(m_camera_power));
	save_item(NAME(m_camera_ambient));
	save_item(NAME(m_matrices));
	save_item(NAME(m_vectors));
	save_item(NAME(m_light_vector));
	save_item(NAME(m_scaling));
	save_item(NAME(m_spv));
	save_item(NAME(m_spm));

	save_item(NAME(m_sub_port8));
	save_item(NAME(m_sub_porta));
	save_item(NAME(m_sub_portb));

	m_lamps.resolve();

	m_c361.timer = timer_alloc(FUNC(namcos23_state::c361_timer_cb), this);
	m_c361.timer->adjust(attotime::never);

	m_maincpu->add_fastram(0, m_mainram.bytes()-1, false, reinterpret_cast<u32 *>(memshare("mainram")->ptr()));

	m_subcpu_scanline_on_timer = timer_alloc(FUNC(namcos23_state::subcpu_scanline_on_tick), this);
	m_subcpu_scanline_off_timer = timer_alloc(FUNC(namcos23_state::subcpu_scanline_off_tick), this);

	m_texram = m_c412.sram;

	m_rs232_irqnum = -1;
	m_vbl_irqnum = MIPS3_IRQ0;
	m_c361_irqnum = MIPS3_IRQ1;
	m_sub_irqnum = MIPS3_IRQ1;
	m_c435_irqnum = MIPS3_IRQ2;
	m_c422_irqnum = MIPS3_IRQ3;
	m_rs232_irqnum = MIPS3_IRQ5;
}

void gorgon_state::machine_start()
{
	namcos23_state::machine_start();

	save_item(NAME(m_cz_was_written));

	for (int bank = 0; bank < 4; bank++)
	{
		m_banked_czram[bank] = make_unique_clear<u16[]>(0x100);
		m_recalc_czram[bank] = make_unique_clear<u8[]>(0x2000);
		m_cz_was_written[bank] = 1;

		save_pointer(NAME(m_banked_czram[bank]), 0x100, bank);
		save_pointer(NAME(m_recalc_czram[bank]), 0x2000, bank);
	}
}

void crszone_state::machine_start()
{
	namcos23_state::machine_start();

	m_rs232_irqnum = MIPS3_IRQ0;
	m_sub_irqnum = MIPS3_IRQ1;
	m_c422_irqnum = MIPS3_IRQ1;
	m_vbl_irqnum = MIPS3_IRQ2;
	m_c361_irqnum = MIPS3_IRQ3;
	m_c435_irqnum = -1;
	m_c451_irqnum = MIPS3_IRQ3;
	m_c450_irqnum = MIPS3_IRQ4;
}

void namcos23_state::machine_reset()
{
	m_c435.buffer_pos = 0;
	m_subcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_subcpu_running = false;

	m_c435.direct_buf_pos = 0;
	m_c435.direct_buf_nonempty = false;
	m_c435.direct_buf_open = false;
	memset(m_proj_matrix, 0, sizeof(float) * 24);
	m_proj_matrix_line = 0;

	memset(m_c404.rowscroll, 0, sizeof(m_c404.rowscroll));
	m_c404.lastrow = 0;

	m_subcpu_scanline_on_timer->adjust(attotime::zero, 0, m_screen->scan_period());
	m_subcpu_scanline_off_timer->adjust(m_screen->time_until_pos(0, 32), 0, m_screen->time_until_pos(0, 32) + m_screen->scan_period());

	m_jvs_sense = 1;
	m_main_irqcause = 0;
	m_ctl_vbl_active = false;
	m_sub_port8 = 0x02;
	m_sub_porta = 0;
	m_sub_portb = 0x50;
	m_subcpu_running = false;
	m_render.count[0] = m_render.count[1] = 0;
	m_render.cur = 0;
}

void gorgon_state::machine_reset()
{
	namcos23_state::machine_reset();

	m_subcpu_scanline_on_timer->adjust(attotime::never);
	m_subcpu_scanline_off_timer->adjust(attotime::never);
}

void namcoss23_gmen_state::machine_start()
{
	namcos23_state::machine_start();

	m_sh2_irq_timer = timer_alloc(FUNC(namcoss23_gmen_state::sh2_irq_off), this);
	save_item(NAME(m_vpx_sdao));
}

void namcoss23_gmen_state::machine_reset()
{
	namcos23_state::machine_reset();

	// halt the SH-2 until we need it
	m_sh2->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_sh2_irq_timer->adjust(attotime::never);

	m_sh2_irq = -1;
	m_sh2_unk = 0;
	m_vpx_sdao = 0;
}

TIMER_CALLBACK_MEMBER(namcoss23_gmen_state::sh2_irq_off)
{
	m_sh2->set_input_line(m_sh2_irq, CLEAR_LINE);
	m_sh2_irq = -1;
}



#define XOR(a) WORD2_XOR_BE(a)
static const gfx_layout namcos23_cg_layout =
{
	16,16,
	0x400, /* 0x3c0 */
	4,
	{ 0,1,2,3 },
	{ XOR(0)*4, XOR(1)*4,  XOR(2)*4,  XOR(3)*4,  XOR(4)*4,  XOR(5)*4,  XOR(6)*4,  XOR(7)*4,
		XOR(8)*4, XOR(9)*4, XOR(10)*4, XOR(11)*4, XOR(12)*4, XOR(13)*4, XOR(14)*4, XOR(15)*4 },
	{ 64*0,64*1,64*2,64*3,64*4,64*5,64*6,64*7,64*8,64*9,64*10,64*11,64*12,64*13,64*14,64*15 },
	64*16
}; /* cg_layout */

#undef XOR

static GFXLAYOUT_RAW(namcos23_sprite_layout, 32, 32, 32*8, 32*32*8)

static GFXDECODE_START( gfx_namcos23 )
	GFXDECODE_RAM(   nullptr,   0, namcos23_cg_layout, 0, 0x800 )
	GFXDECODE_ENTRY( "textile", 0, gfx_16x16x8_raw,    0, 0x80 )
GFXDECODE_END

static GFXDECODE_START( gfx_gorgon )
	GFXDECODE_RAM(   nullptr,   0, namcos23_cg_layout,     0, 0x800 )
	GFXDECODE_ENTRY( "textile", 0, gfx_16x16x8_raw,        0, 0x80 )
	GFXDECODE_ENTRY( "sprites", 0, namcos23_sprite_layout, 0, 0x80 )
GFXDECODE_END

void gorgon_state::gorgon(machine_config &config)
{
	/* basic machine hardware */
	R4650BE(config, m_maincpu, BUSCLOCK*4);
	m_maincpu->set_icache_size(8192);   // VERIFIED
	m_maincpu->set_dcache_size(8192);   // VERIFIED
	m_maincpu->set_addrmap(AS_PROGRAM, &gorgon_state::mips_map);

	H83002(config, m_subcpu, H8CLOCK);
	m_subcpu->set_addrmap(AS_PROGRAM, &gorgon_state::s23h8rwmap);
	m_subcpu->read_adc<0>().set_constant(0);
	m_subcpu->read_adc<1>().set_constant(0);
	m_subcpu->read_adc<2>().set_constant(0);
	m_subcpu->read_adc<3>().set_constant(0);
	m_subcpu->read_port6().set(FUNC(gorgon_state::mcu_p6_r));
	m_subcpu->write_port6().set(FUNC(gorgon_state::mcu_p6_w));
	m_subcpu->read_port7().set_constant(0);
	m_subcpu->read_port8().set(FUNC(gorgon_state::mcu_p8_r));
	m_subcpu->write_port8().set(FUNC(gorgon_state::mcu_p8_w));
	m_subcpu->read_porta().set(FUNC(gorgon_state::mcu_pa_r));
	m_subcpu->write_porta().set(FUNC(gorgon_state::mcu_pa_w));
	m_subcpu->read_portb().set(FUNC(gorgon_state::mcu_pb_r));
	m_subcpu->write_portb().set(FUNC(gorgon_state::mcu_pb_w));

	// Timer at 115200*16 for the jvs serial clock
	m_subcpu->sci_set_external_clock_period(0, attotime::from_hz(JVSCLOCK/8));

	NAMCO_SETTINGS(config, m_settings, 0);

	RTC4543(config, m_rtc, XTAL(32'768));
	m_rtc->data_cb().set(m_subcpu, FUNC(h8_device::sci_rx_w<1>));

	m_subcpu->write_sci_tx<1>().set(m_settings, FUNC(namco_settings_device::data_w));
	m_subcpu->write_sci_clk<1>().set(m_rtc, FUNC(rtc4543_device::clk_w)).invert();
	m_subcpu->write_sci_clk<1>().append(m_settings, FUNC(namco_settings_device::clk_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	m_screen->set_screen_update(FUNC(gorgon_state::screen_update));
	m_screen->screen_vblank().set(FUNC(gorgon_state::vblank));
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE);

	PALETTE(config, m_palette).set_entries(0x8000);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gorgon);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	c352_device &c352(C352(config, "c352", C352CLOCK, C352DIV));
	c352.add_route(0, "speaker", 1.00, 0);
	c352.add_route(1, "speaker", 1.00, 1);
	c352.add_route(2, "speaker", 1.00, 0);
	c352.add_route(3, "speaker", 1.00, 1);

	JVS_PORT(config, m_jvs, jvs_port_devices, nullptr);
	m_jvs->rxd().set(m_subcpu, FUNC(h8_device::sci_rx_w<0>));
	m_subcpu->write_sci_tx<0>().set(m_jvs, FUNC(jvs_port_device::txd));
	m_jvs->sense().set([this](u8 state) { m_jvs_sense = state; });

	for (const auto &option : m_jvs->option_list())
		m_jvs->set_option_machine_config(option.first.c_str(), [this](device_t *device)
	{
		configure_jvs(dynamic_cast<device_jvs_interface &>(*device));
	});
}

void gorgon_state::finfurl(machine_config &config)
{
	gorgon(config);
	m_jvs->set_default_option("namco_asca1");
}

void namcos23_state::s23(machine_config &config)
{
	/* basic machine hardware */
	R4650BE(config, m_maincpu, BUSCLOCK*5);
	m_maincpu->set_icache_size(8192);   // VERIFIED
	m_maincpu->set_dcache_size(8192);   // VERIFIED
	m_maincpu->set_addrmap(AS_PROGRAM, &namcos23_state::mips_map);

	H83002(config, m_subcpu, H8CLOCK);
	m_subcpu->set_addrmap(AS_PROGRAM, &namcos23_state::s23h8rwmap);
	m_subcpu->read_adc<0>().set_constant(0);
	m_subcpu->read_adc<1>().set_constant(0);
	m_subcpu->read_adc<2>().set_constant(0);
	m_subcpu->read_adc<3>().set_constant(0);
	m_subcpu->read_port6().set(FUNC(namcos23_state::mcu_p6_r));
	m_subcpu->write_port6().set(FUNC(namcos23_state::mcu_p6_w));
	m_subcpu->read_port7().set_constant(0);
	m_subcpu->read_port8().set(FUNC(namcos23_state::mcu_p8_r));
	m_subcpu->write_port8().set(FUNC(namcos23_state::mcu_p8_w));
	m_subcpu->read_porta().set(FUNC(namcos23_state::mcu_pa_r));
	m_subcpu->write_porta().set(FUNC(namcos23_state::mcu_pa_w));
	m_subcpu->read_portb().set(FUNC(namcos23_state::mcu_pb_r));
	m_subcpu->write_portb().set(FUNC(namcos23_state::mcu_pb_w));

	// Timer at 115200*16 for the jvs serial clock
	m_subcpu->sci_set_external_clock_period(0, attotime::from_hz(JVSCLOCK/8));

	NAMCO_SETTINGS(config, m_settings, 0);

	RTC4543(config, m_rtc, XTAL(32'768));
	m_rtc->data_cb().set(m_subcpu, FUNC(h8_device::sci_rx_w<1>));

	m_subcpu->write_sci_tx<1>().set(m_settings, FUNC(namco_settings_device::data_w));
	m_subcpu->write_sci_clk<1>().set(m_rtc, FUNC(rtc4543_device::clk_w)).invert();
	m_subcpu->write_sci_clk<1>().append(m_settings, FUNC(namco_settings_device::clk_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	m_screen->set_screen_update(FUNC(namcos23_state::screen_update));
	m_screen->screen_vblank().set(FUNC(namcos23_state::vblank));
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE);

	PALETTE(config, m_palette).set_entries(0x8000);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_namcos23);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	c352_device &c352(C352(config, "c352", C352CLOCK, C352DIV));
	c352.add_route(0, "speaker", 1.00, 0);
	c352.add_route(1, "speaker", 1.00, 1);
	c352.add_route(2, "speaker", 1.00, 0);
	c352.add_route(3, "speaker", 1.00, 1);

	JVS_PORT(config, m_jvs, jvs_port_devices, nullptr);
	m_jvs->rxd().set(m_subcpu, FUNC(h8_device::sci_rx_w<0>));
	m_subcpu->write_sci_tx<0>().set(m_jvs, FUNC(jvs_port_device::txd));
	m_jvs->sense().set([this](u8 state) { m_jvs_sense = state; });

	for (const auto &option : m_jvs->option_list())
		m_jvs->set_option_machine_config(option.first.c_str(), [this](device_t *device)
	{
		configure_jvs(dynamic_cast<device_jvs_interface &>(*device));
	});
}

void namcos23_state::configure_jvs(device_jvs_interface &io)
{
	io.system().set_ioport("^^JVS_SYSTEM");
	io.player<0>().set_ioport("^^JVS_PLAYER1");
	io.coin<0>().set_ioport("^^JVS_COIN1");
	io.analog_input<0>().set_ioport("^^JVS_ANALOG_INPUT1");
	io.analog_input<1>().set_ioport("^^JVS_ANALOG_INPUT2");
	io.analog_input<2>().set_ioport("^^JVS_ANALOG_INPUT3");
	io.analog_input<3>().set_ioport("^^JVS_ANALOG_INPUT4");
	io.analog_input<4>().set_ioport("^^JVS_ANALOG_INPUT5");
	io.analog_input<5>().set_ioport("^^JVS_ANALOG_INPUT6");
	io.analog_input<6>().set_ioport("^^JVS_ANALOG_INPUT7");
	io.analog_input<7>().set_ioport("^^JVS_ANALOG_INPUT8");
	io.rotary_input<0>().set_ioport("^^JVS_ROTARY_INPUT1");
	io.screen_position_x<0>().set_ioport("^^JVS_SCREEN_POSITION_INPUT_X1");
	io.screen_position_y<0>().set_ioport("^^JVS_SCREEN_POSITION_INPUT_Y1");
}

void namcos23_state::downhill(machine_config &config)
{
	s23(config);
	m_jvs->set_default_option("namco_asca3");
}

void namcos23_state::panicprk(machine_config &config)
{
	s23(config);
	m_jvs->set_default_option("namco_asca3a");
}

void namcos23_state::timecrs2(machine_config &config)
{
	s23(config);
	m_jvs->set_default_option("namco_tssio");
}

void namcoss23_state::ss23(machine_config &config)
{
	s23(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &namcoss23_state::mips_map);
}

void namcoss23_state::timecrs2v4a(machine_config &config)
{
	ss23(config);
	m_jvs->set_default_option("namco_tssio");
}

void namcoss23_state::_500gp(machine_config &config)
{
	ss23(config);
	m_jvs->set_default_option("namco_fca10");
}

void namcoss23_state::aking(machine_config &config)
{
	ss23(config);
	m_jvs->set_default_option("namco_fca10");
}

void namcoss23_gmen_state::gmen(machine_config &config)
{
	ss23(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &namcoss23_gmen_state::mips_map);

	SH7604(config, m_sh2, XTAL(28'700'000));
	m_sh2->set_addrmap(AS_PROGRAM, &namcoss23_gmen_state::sh2_map);

	VPX3220A(config, m_vpx, 0);
	m_vpx->sda_callback().set(FUNC(namcoss23_gmen_state::vpx_i2c_sdao_w));

	MD8412B(config, m_firewire, 0);
}

void namcoss23_gmen_state::gunwars(machine_config &config)
{
	gmen(config);
	m_jvs->set_default_option("namco_asca5");
}

void namcoss23_gmen_state::raceon(machine_config &config)
{
	gmen(config);
	m_jvs->set_default_option("namco_asca5");
}

void namcoss23_gmen_state::finfurl2(machine_config &config)
{
	gmen(config);
	m_jvs->set_default_option("namco_asca3a");
}

void crszone_state::crszone(machine_config &config)
{
	ss23(config);

	/* basic machine hardware */
	m_maincpu->set_clock(BUSCLOCK * 6);
	m_maincpu->set_addrmap(AS_PROGRAM, &crszone_state::mips_map);

	m_jvs->set_default_option("namco_csz1");

	/* debug hardware */
	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_acia->irq_handler().set(FUNC(crszone_state::acia_irq_w));

	clock_device &acia_clock(CLOCK(config, "acia_clock", 1'843'200));
	acia_clock.signal_handler().set("acia", FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append("acia", FUNC(acia6850_device::write_rxc));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
}


/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START(s23)
	// You can go to the pcb test mode by pressing P1-A in some games.
	// Use P1-A to select, P1-Sel+P1-A to exit, up/down to navigate
	PORT_START("P1")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_PLAYER(2) PORT_NAME("Dev Service P1-D")
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_PLAYER(2) PORT_NAME("Dev Service P1-E")
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_BUTTON7) PORT_PLAYER(2) PORT_NAME("Dev Service P1-F")
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2) PORT_NAME("Dev Service P1-A")
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_BUTTON8) PORT_PLAYER(2) PORT_NAME("Dev Service P1-G")
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_BUTTON9) PORT_PLAYER(2) PORT_NAME("Dev Service P1-H")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_PLAYER(2) PORT_NAME("Dev Service Down")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_PLAYER(2) PORT_NAME("Dev Service Up")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_START2) PORT_NAME("Dev Service Start")
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2) PORT_NAME("Dev Service P1-Sel")
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(2) PORT_NAME("Dev Service P1-B")
	PORT_BIT(0x800, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_PLAYER(2) PORT_NAME("Dev Service P1-C")

	PORT_START("P2")
	PORT_BIT(0xfff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("DSW")
	PORT_DIPNAME(0x01, 0x01, "Service Mode DIP") PORT_DIPLOCATION("DIP:8")
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, "Skip POST") PORT_DIPLOCATION("DIP:7")
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x04, "Freeze?") PORT_DIPLOCATION("DIP:6")
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x08, DEF_STR(Unknown)) PORT_DIPLOCATION("DIP:5")
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x10, DEF_STR(Unknown)) PORT_DIPLOCATION("DIP:4")
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x20, DEF_STR(Unknown)) PORT_DIPLOCATION("DIP:3")
	PORT_DIPSETTING(0x20, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x40, DEF_STR(Unknown)) PORT_DIPLOCATION("DIP:2")
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, DEF_STR(Unknown)) PORT_DIPLOCATION("DIP:1")
	PORT_DIPSETTING(0x80, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))

	PORT_START("JVS_SYSTEM")
	PORT_SERVICE(0x80, IP_ACTIVE_HIGH)

	PORT_START("JVS_PLAYER1")
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_SERVICE1)
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_BUTTON1)
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_BUTTON2)
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_BUTTON3)
	PORT_BIT(0x00004000, IP_ACTIVE_HIGH, IPT_BUTTON4)
	PORT_BIT(0x00002000, IP_ACTIVE_HIGH, IPT_BUTTON5)
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_BUTTON6)
	PORT_BIT(0x00000800, IP_ACTIVE_HIGH, IPT_BUTTON7)
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_BUTTON8)
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_BUTTON9)
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_BUTTON10)
	PORT_BIT(0x00800000, IP_ACTIVE_HIGH, IPT_BUTTON11)
	PORT_BIT(0x00400000, IP_ACTIVE_HIGH, IPT_BUTTON12)
	PORT_BIT(0x00200000, IP_ACTIVE_HIGH, IPT_BUTTON13)

	PORT_START("JVS_COIN1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_TILT2) PORT_NAME("Counter disconnected")

	/* Dummy so we can easily get the analog ch # */
	PORT_START("JVS_ANALOG_INPUT1")
	PORT_BIT(0x80ff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("JVS_ANALOG_INPUT2")
	PORT_BIT(0x81ff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("JVS_ANALOG_INPUT3")
	PORT_BIT(0x82ff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("JVS_ANALOG_INPUT4")
	PORT_BIT(0x83ff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("JVS_ANALOG_INPUT5")
	PORT_BIT(0x84ff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("JVS_ANALOG_INPUT6")
	PORT_BIT(0x85ff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("JVS_ANALOG_INPUT7")
	PORT_BIT(0x86ff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("JVS_ANALOG_INPUT8")
	PORT_BIT(0x87ff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("JVS_ROTARY_INPUT1")
	PORT_BIT(0xffff, 0x0000, IPT_UNUSED)

	PORT_START("JVS_SCREEN_POSITION_INPUT_X1")
	PORT_BIT(0xffff, 0x0000, IPT_UNUSED)

	PORT_START("JVS_SCREEN_POSITION_INPUT_Y1")
	PORT_BIT(0xffff, 0x0000, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START(gorgon)
	PORT_INCLUDE(s23)

	PORT_MODIFY("P1")
	PORT_BIT(0xfff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_MODIFY("P2")
	PORT_BIT(0xfff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_MODIFY("DSW")
	PORT_DIPNAME(0x01, 0x01, DEF_STR(Unknown)) PORT_DIPLOCATION("DIP:8")
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, DEF_STR(Unknown)) PORT_DIPLOCATION("DIP:7")
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x04, DEF_STR(Unknown)) PORT_DIPLOCATION("DIP:6")
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x08, DEF_STR(Unknown)) PORT_DIPLOCATION("DIP:5")
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x10, DEF_STR(Unknown)) PORT_DIPLOCATION("DIP:4")
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x20, DEF_STR(Unknown)) PORT_DIPLOCATION("DIP:3")
	PORT_DIPSETTING(0x20, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x40, DEF_STR(Unknown)) PORT_DIPLOCATION("DIP:2")
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, "Service Mode DIP") PORT_DIPLOCATION("DIP:1")
	PORT_DIPSETTING(0x80, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
INPUT_PORTS_END

static INPUT_PORTS_START(gmen)
	PORT_INCLUDE(s23)

	PORT_START("GMENDSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
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
INPUT_PORTS_END

static INPUT_PORTS_START(500gp)
	PORT_INCLUDE(s23)

	PORT_MODIFY("JVS_PLAYER1")
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_NAME("Select Up")
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_NAME("Select Down")
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_START1) // BUTTON1
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Tank") // BUTTON6
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("View Button") // BUTTON9
	PORT_CONFNAME(0x00000100, 0x00000100, DEF_STR(Cabinet)) // BUTTON10
	PORT_CONFSETTING(0x00000100, DEF_STR(Standard))
	PORT_CONFSETTING(0x00000000, "Deluxe")
	PORT_BIT(0x00e0ec01, IP_ACTIVE_HIGH, IPT_UNUSED) // BUTTON2/BUTTON3/BUTTON4/BUTTON5/BUTTON7/BUTTON8/BUTTON11/BUTTON12/BUTTON13

	PORT_MODIFY("JVS_ANALOG_INPUT1")
	PORT_BIT(0xffff, 0x8000, IPT_PEDAL) PORT_MINMAX(0x0100, 0xffff) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_NAME("Throttle")

	PORT_MODIFY("JVS_ANALOG_INPUT2")
	PORT_BIT(0xffff, 0x8000, IPT_PEDAL2) PORT_MINMAX(0x0100, 0xffff) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_NAME("Brake")

	PORT_MODIFY("JVS_ANALOG_INPUT3")
	PORT_BIT(0xffff, 0x8000, IPT_AD_STICK_X) PORT_MINMAX(0x0100, 0xffff) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_NAME("Bank")
INPUT_PORTS_END

static INPUT_PORTS_START(aking)
	PORT_INCLUDE(s23)
INPUT_PORTS_END

static INPUT_PORTS_START(downhill)
	PORT_INCLUDE(s23)

	PORT_MODIFY("JVS_PLAYER1")
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_NAME("Select Up")
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_NAME("Select Down")
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Enter") // BUTTON1
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Right Brake") // BUTTON8
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Left Brake") // BUTTON9
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_START1) // BUTTON10
	PORT_BIT(0x00e0f801, IP_ACTIVE_HIGH, IPT_BUTTON2) // BUTTON2/BUTTON3/BUTTON4/BUTTON5/BUTTON6/BUTTON7/BUTTON11/BUTTON12/BUTTON13

	PORT_MODIFY("JVS_ANALOG_INPUT7")
	PORT_BIT(0xffff, 0x8000, IPT_PADDLE) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_NAME("Steering")

	PORT_MODIFY("JVS_ROTARY_INPUT1")
	PORT_BIT(0xffff, 0, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(8) PORT_NAME("Encoder")
INPUT_PORTS_END

static INPUT_PORTS_START(finfurl)
	PORT_INCLUDE(gorgon)

	PORT_MODIFY("JVS_PLAYER1")
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Enter") // BUTTON1
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Whip Button L") // BUTTON2
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Whip Button R") // BUTTON3
	PORT_BIT(0x00e07f00, IP_ACTIVE_HIGH, IPT_UNUSED) // BUTTON4/BUTTON5/BUTTON6/BUTTON7/BUTTON8/BUTTON9/BUTTON10/BUTTON11/BUTTON12/BUTTON13

	PORT_MODIFY("JVS_ANALOG_INPUT1")
	PORT_BIT(0xffff, 0x8000, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_NAME("Swing")

	PORT_MODIFY("JVS_ANALOG_INPUT2")
	PORT_BIT(0xffff, 0x8000, IPT_AD_STICK_X) PORT_MINMAX(0x5000, 0xb000) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_NAME("Handle") PORT_REVERSE
INPUT_PORTS_END

static INPUT_PORTS_START(finfurl2)
	PORT_INCLUDE(gmen)

	PORT_MODIFY("JVS_PLAYER1")
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Enter") // BUTTON1
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Whip Button L") // BUTTON2
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Whip Button R") // BUTTON3
	PORT_BIT(0x00e07f00, IP_ACTIVE_HIGH, IPT_UNUSED) // BUTTON4/BUTTON5/BUTTON6/BUTTON7/BUTTON8/BUTTON9/BUTTON10/BUTTON11/BUTTON12/BUTTON13

	PORT_MODIFY("JVS_ANALOG_INPUT1")
	PORT_BIT(0xffff, 0x8000, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_NAME("Swing")

	PORT_MODIFY("JVS_ANALOG_INPUT2")
	PORT_BIT(0xffff, 0x8000, IPT_AD_STICK_X) PORT_MINMAX(0x5000, 0xb000) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_NAME("Handle") PORT_REVERSE
INPUT_PORTS_END

static INPUT_PORTS_START(gunwars)
	PORT_INCLUDE(gmen)

	PORT_MODIFY("JVS_PLAYER1")
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_NAME("Select Up")
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_NAME("Select Down")
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_START) PORT_NAME("Enter") // BUTTON1
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Gun Trigger") // BUTTON8
	PORT_BIT(0x00e0fb01, IP_ACTIVE_HIGH, IPT_UNUSED) // BUTTON2/BUTTON3/BUTTON4/BUTTON5/BUTTON6/BUTTON7/BUTTON9/BUTTON10/BUTTON11/BUTTON12/BUTTON13/

	PORT_MODIFY("JVS_ANALOG_INPUT1")
	PORT_BIT(0xffff, 0x8000, IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_NAME("Slide Left/Right")

	PORT_MODIFY("JVS_ANALOG_INPUT2")
	PORT_BIT(0xffff, 0x8000, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_NAME("Slide Forward/Back")

	PORT_MODIFY("JVS_ANALOG_INPUT3")
	PORT_BIT(0xffff, 0x8000, IPT_AD_STICK_Z) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_NAME("Gun Yawning Left/Right")
INPUT_PORTS_END

static INPUT_PORTS_START(motoxgo)
	PORT_INCLUDE(s23)

	PORT_MODIFY("JVS_PLAYER1")
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_NAME("Select Up")
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_NAME("Select Down")
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_START1) // BUTTON1
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("View") // BUTTON2
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_CUSTOM) // BUTTON3 // FREEZE/RELAY? motors ignored if pressed
	PORT_BIT(0x00e07f00, IP_ACTIVE_HIGH, IPT_BUTTON4) // BUTTON4/BUTTON5/BUTTON6/BUTTON7/BUTTON8/BUTTON9/BUTTON10/BUTTON11/BUTTON12/BUTTON13

	PORT_MODIFY("JVS_ANALOG_INPUT1")
	PORT_BIT(0xffff, 0x8000, IPT_AD_STICK_Y) PORT_MINMAX(0x000, 0xffff) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_NAME("Steering")

	PORT_MODIFY("JVS_ANALOG_INPUT2")
	PORT_BIT(0xffff, 0x8000, IPT_PEDAL) PORT_MINMAX(0x8000, 0xffff) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_NAME("Throttle")

	PORT_MODIFY("JVS_ANALOG_INPUT3")
	PORT_BIT(0xffff, 0x8000, IPT_PEDAL2) PORT_MINMAX(0x0000, 0x8000) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_NAME("Brake")

	PORT_MODIFY("JVS_ANALOG_INPUT4")
	PORT_BIT(0xffff, 0x8000, IPT_AD_STICK_X) PORT_MINMAX(0x0000, 0xffff) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_NAME("Bank")
INPUT_PORTS_END

static INPUT_PORTS_START(panicprk)
	PORT_INCLUDE(s23)

	PORT_MODIFY("JVS_PLAYER1")
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_NAME("Up Select")
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_NAME("Down Select")
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Enter") // BUTTON1
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_START1) // BUTTON2
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_START2) // BUTTON3
	PORT_BIT(0x00e07f00, IP_ACTIVE_HIGH, IPT_UNUSED) // BUTTON4/BUTTON5/BUTTON6/BUTTON7/BUTTON8/BUTTON9/BUTTON10/BUTTON11/BUTTON12/BUTTON13

	PORT_MODIFY("JVS_ANALOG_INPUT1")
	PORT_BIT(0xffff, 0x8000, IPT_AD_STICK_X) PORT_MINMAX(0x000, 0xffff) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_CENTERDELTA(0)

	PORT_MODIFY("JVS_ANALOG_INPUT2")
	PORT_BIT(0xffff, 0x8000, IPT_AD_STICK_X) PORT_PLAYER(2) PORT_MINMAX(0x000, 0xffff) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_CENTERDELTA(0)
INPUT_PORTS_END

static INPUT_PORTS_START(raceon)
	PORT_INCLUDE(gmen)
INPUT_PORTS_END

static INPUT_PORTS_START(rapidrvr)
	PORT_INCLUDE(gorgon)

	PORT_MODIFY("JVS_PLAYER1")
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_NAME("Service Up")
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_NAME("Service Down")
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Service Enter")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_START1) // BUTTON2
	PORT_BIT(0x00e0ff00, IP_ACTIVE_HIGH, IPT_UNUSED) // BUTTON3/BUTTON4/BUTTON5/BUTTON6/BUTTON7/BUTTON8/BUTTON9/BUTTON10/BUTTON11/BUTTON12/BUTTON13

	PORT_MODIFY("JVS_ANALOG_INPUT1")
	PORT_BIT(0xffff, 0x8000, IPT_AD_STICK_Y) PORT_MINMAX(0x4c80, 0xb340) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_NAME("Yaw") PORT_REVERSE

	PORT_MODIFY("JVS_ANALOG_INPUT2")
	PORT_BIT(0xffff, 0x8000, IPT_AD_STICK_X) PORT_MINMAX(0x4c80, 0xb340) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_NAME("Pitch")
INPUT_PORTS_END

static INPUT_PORTS_START(rapidrvrp)
	PORT_INCLUDE(rapidrvr)

	// To fully use test mode, both Service Mode dipswitches need to be enabled.
	// Some of the developer menus require you to navigate with the Dev keys,
	// but usually the User keys work fine too.
	PORT_MODIFY("P1")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_NAME("Dev Service D")
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_NAME("Dev Service E") // I/O Unknown Status
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_NAME("Dev Service F") // I/O Air Damper FR
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2) PORT_NAME("Dev Service A") // + I/O Air Damper RR
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_BUTTON7) PORT_NAME("Dev Service G")
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_BUTTON8) PORT_NAME("Dev Service H")
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_PLAYER(2) PORT_NAME("Dev Service Down")
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_PLAYER(2) PORT_NAME("Dev Service Up")
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_START2) PORT_NAME("Dev Start")
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("Dev Service C") // + I/O Air Damper FL
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2) PORT_NAME("Dev Service B") // + I/O Air Damper RL
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_BUTTON9)
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_BUTTON10)
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_BUTTON11)
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_BUTTON12)

	PORT_MODIFY("JVS_PLAYER1")
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_NAME("User Service Up")
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_NAME("User Service Down")
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("User Service Enter")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("User Start") // BUTTON2
	PORT_BIT(0x00e0ff00, IP_ACTIVE_HIGH, IPT_UNUSED) // BUTTON3/BUTTON4/BUTTON5/BUTTON6/BUTTON7/BUTTON8/BUTTON9/BUTTON10/BUTTON11/BUTTON12/BUTTON13

	PORT_MODIFY("DSW")
	PORT_DIPNAME(0x08, 0x08, "Debug Messages") PORT_DIPLOCATION("DIP:5")
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x40, "Dev Service Mode") PORT_DIPLOCATION("DIP:2")
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, "User Service Mode") PORT_DIPLOCATION("DIP:1")
	PORT_DIPSETTING(0x80, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
INPUT_PORTS_END

static INPUT_PORTS_START(timecrs2)
	PORT_INCLUDE(s23)

	PORT_MODIFY("JVS_PLAYER1")
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_NAME("User Service Down")
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_NAME("User Service Up")
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("User Enter") // BUTTON1
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Gun Trigger") // BUTTON2
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Foot Pedal") // BUTTON3
	PORT_CONFNAME(0x00004000, 0x00000000, "Link ID" ) // BUTTON4
	PORT_CONFSETTING(         0x00004000, "Right/Blue" )
	PORT_CONFSETTING(         0x00000000, "Left/Red" )
	PORT_BIT(0x00e03f00, IP_ACTIVE_HIGH, IPT_UNUSED) // BUTTON5/BUTTON6/BUTTON7/BUTTON8/BUTTON9/BUTTON10/BUTTON11/BUTTON12/BUTTON13

	PORT_MODIFY("JVS_SCREEN_POSITION_INPUT_X1") // tuned for CRT
	PORT_BIT(0xfff, 91 + 733 / 2, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(91, 91 + 733) PORT_SENSITIVITY(50) PORT_KEYDELTA(20)

	PORT_MODIFY("JVS_SCREEN_POSITION_INPUT_Y1") // tuned for CRT - can't shoot below the statusbar?
	PORT_BIT(0xfff, 38 + 247 / 2, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(38, 38 + 247) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)
INPUT_PORTS_END

static INPUT_PORTS_START(crszone)
	PORT_INCLUDE(timecrs2)

	PORT_MODIFY("JVS_PLAYER1")
	PORT_BIT(0x00002000, IP_ACTIVE_LOW, IPT_UNUSED) // BUTTON5 (Motor test shows NG if this is not pressed)

	PORT_MODIFY("JVS_SCREEN_POSITION_INPUT_X1")
	PORT_BIT(0xfff, 0x1bf, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0x040, 0x33f) PORT_SENSITIVITY(50) PORT_KEYDELTA(20)

	PORT_MODIFY("JVS_SCREEN_POSITION_INPUT_Y1")
	PORT_BIT(0xfff, 0x08f, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0x020, 0x0ff) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)
INPUT_PORTS_END

// a note about "user1" ROMs:
// serial number data is at offset 0x201 (interleaved) and it's the only difference between sets marked as 'set 1' and 'set 2'
// TODO: verify if it's better to just keep one set and note the alternate serial numbers

ROM_START( rapidrvr )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "rd3verc.ic2",  0x000000, 0x200000, CRC(c15c0f30) SHA1(9f529232818f3e184f81f62408a5cad615b05613) )
	ROM_LOAD16_BYTE( "rd3verc.ic1",  0x000001, 0x200000, CRC(9d7f4411) SHA1(d049efaa539d36ed0f73ca3f50a8f7112e67f865) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "rd3verc.ic3",  0x000000, 0x080000, CRC(6e26fbaf) SHA1(4ab6637d22f0d26f7e1d10e9c80059c56f64303d) )

	ROM_REGION32_BE( 0x800000, "data", 0 )  /* data */
	ROM_LOAD16_BYTE( "rd1mtah.3j",   0x000000, 0x400000, CRC(d8fa0f3d) SHA1(0d5bdb3a2e7be1dffe11b74baa2c10bfe011ae92) )
	ROM_LOAD16_BYTE( "rd1mtal.1j",   0x000001, 0x400000, CRC(8f0efa86) SHA1(9953461c258f2a96be275a7b18d6518ddfac3860) )

	ROM_REGION( 0x2000000, "textile", 0 )   /* texture tiles */
	ROM_LOAD( "rd1cgll.8b",   0x0000000, 0x800000, CRC(b58b92ac) SHA1(70ee6e0e5347e05817aa30d53d766b8ce0fc44e4) )
	ROM_LOAD( "rd1cglm.7b",   0x0800000, 0x800000, CRC(447067fa) SHA1(e2052373773594feb303e1924a4a820cf34ab55b) )
	ROM_LOAD( "rd1cgum.6b",   0x1000000, 0x800000, CRC(c50de2ef) SHA1(24758a72b3569ce6a643a5786fce7c34b8aa692d) )
	ROM_LOAD( "rd1cguu.5b",   0x1800000, 0x800000, CRC(611bab41) SHA1(84cddb2b63bf8336e92aecb06eddf1b34af73540) )

	ROM_REGION( 0x1000000, "sprites", 0 )   /* sprites tiles */
	ROM_LOAD( "rd1sprll.12t", 0x0000000, 0x400000, CRC(8d450259) SHA1(27cccd1e7dad8880147bb85185982d8d27076e69) )
	ROM_LOAD( "rd1sprlm.11p", 0x0400000, 0x400000, CRC(6c8db3a5) SHA1(24d81fa11e9c835cddadec4cbd530738e258346c) )
	ROM_LOAD( "rd1sprum.10p", 0x0800000, 0x400000, CRC(8e08b2c6) SHA1(a17331a4e41f677f604d1b74e7694cf920b03b66) )
	ROM_LOAD( "rd1spruu.9p",  0x0c00000, 0x400000, CRC(f20a9673) SHA1(e5f1d552b0c42e102593ab578ff0b9ff814f8650) )

	ROM_REGION16_LE( 0x200000, "textilemapl", 0 )   /* texture tilemap 0-15 */
	ROM_LOAD( "rd1ccrl.11a",  0x000000, 0x200000, CRC(b0ea2b32) SHA1(0dc45846725b0de619bc6bae69e3eb166ed21bf0) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "rd1ccrh.11b",  0x000000, 0x200000, CRC(fafffb86) SHA1(15b0ba0252b99d0cac29fcb374fb895643f528fe) )

	ROM_REGION32_BE( 0x2000000, "pointrom", 0 ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "rd1pt0h.9l",   0x0000000, 0x400000, CRC(6f280eff) SHA1(9dd8c8903581d7a412146e50f4009e1d2b743f06) )
	ROM_LOAD32_WORD_SWAP( "rd1pt0l.9j",   0x0000002, 0x400000, CRC(47b1c5a5) SHA1(021d4ca7b8674d8ed5daa701bf41b4a7164d992a) )
	ROM_LOAD32_WORD_SWAP( "rd1pt1h.10l",  0x0800000, 0x400000, CRC(37bd9bdf) SHA1(b26c284024ea4ad4c67b2eefbfdd5ebb35a0118e) )
	ROM_LOAD32_WORD_SWAP( "rd1pt1l.10j",  0x0800002, 0x400000, CRC(91131cb3) SHA1(e42c5e190c719f1cf2d6e91444062ab901be0e73) )
	ROM_LOAD32_WORD_SWAP( "rd1pt2h.11l",  0x1000000, 0x400000, CRC(fa601e83) SHA1(45c420538910f566e75d668306735f54c901669f) )
	ROM_LOAD32_WORD_SWAP( "rd1pt2l.11j",  0x1000002, 0x400000, CRC(3423ff9f) SHA1(73823c179c866cbb601a23417acbbf5b3dc97213) )
	ROM_LOAD32_WORD_SWAP( "rd1pt3h.12l",  0x1800000, 0x400000, CRC(e82ff66a) SHA1(9e2c951136b26d969d2c9d030b7e0bad8bbbe3fb) )
	ROM_LOAD32_WORD_SWAP( "rd1pt3l.12j",  0x1800002, 0x400000, CRC(7216d63e) SHA1(77088ff05c2630996f4bdc87fe466f9b97611467) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
	ROM_LOAD( "rd1wavel.2s",  0x000000, 0x800000, CRC(bf52c08c) SHA1(6745062e078e520484390fad1f723124aa4076d0) )
	ROM_LOAD( "rd1waveh.3s",  0x800000, 0x800000, CRC(ef0136b5) SHA1(a6d923ededca168fe555e0b86a72f53bec5424cc) )

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate ROMs */
	ROM_LOAD( "rd1cgll.8f",   0x000000, 0x800000, CRC(b58b92ac) SHA1(70ee6e0e5347e05817aa30d53d766b8ce0fc44e4) )
	ROM_LOAD( "rd1cglm.7f",   0x000000, 0x800000, CRC(447067fa) SHA1(e2052373773594feb303e1924a4a820cf34ab55b) )
	ROM_LOAD( "rd1cgum.6f",   0x000000, 0x800000, CRC(c50de2ef) SHA1(24758a72b3569ce6a643a5786fce7c34b8aa692d) )
	ROM_LOAD( "rd1cguu.5f",   0x000000, 0x800000, CRC(611bab41) SHA1(84cddb2b63bf8336e92aecb06eddf1b34af73540) )
	ROM_LOAD( "rd1sprll.12p", 0x000000, 0x400000, CRC(8d450259) SHA1(27cccd1e7dad8880147bb85185982d8d27076e69) )
	ROM_LOAD( "rd1sprlm.11t", 0x000000, 0x400000, CRC(6c8db3a5) SHA1(24d81fa11e9c835cddadec4cbd530738e258346c) )
	ROM_LOAD( "rd1sprum.10t", 0x000000, 0x400000, CRC(8e08b2c6) SHA1(a17331a4e41f677f604d1b74e7694cf920b03b66) )
	ROM_LOAD( "rd1spruu.9t",  0x000000, 0x400000, CRC(f20a9673) SHA1(e5f1d552b0c42e102593ab578ff0b9ff814f8650) )
	ROM_LOAD( "rd1ccrl.11e",  0x000000, 0x200000, CRC(b0ea2b32) SHA1(0dc45846725b0de619bc6bae69e3eb166ed21bf0) )
	ROM_LOAD( "rd1ccrh.11f",  0x000000, 0x200000, CRC(fafffb86) SHA1(15b0ba0252b99d0cac29fcb374fb895643f528fe) )
ROM_END


ROM_START( rapidrvrv2c )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "rd2verc.ic2",  0x000000, 0x200000, CRC(234fc2f4) SHA1(64374f4de19855f1980d8e088049b0c112107f43) )
	ROM_LOAD16_BYTE( "rd2verc.ic1",  0x000001, 0x200000, CRC(651c5da4) SHA1(0e73e2cfafda626597d2ce08bf07458509fb79de) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "rd2verc.ic3",  0x000000, 0x080000, CRC(6e26fbaf) SHA1(4ab6637d22f0d26f7e1d10e9c80059c56f64303d) )

	ROM_REGION32_BE( 0x800000, "data", 0 )  /* data */
	ROM_LOAD16_BYTE( "rd1mtah.3j",   0x000000, 0x400000, CRC(d8fa0f3d) SHA1(0d5bdb3a2e7be1dffe11b74baa2c10bfe011ae92) )
	ROM_LOAD16_BYTE( "rd1mtal.1j",   0x000001, 0x400000, CRC(8f0efa86) SHA1(9953461c258f2a96be275a7b18d6518ddfac3860) )

	ROM_REGION( 0x2000000, "textile", 0 )   /* texture tiles */
	ROM_LOAD( "rd1cgll.8b",   0x0000000, 0x800000, CRC(b58b92ac) SHA1(70ee6e0e5347e05817aa30d53d766b8ce0fc44e4) )
	ROM_LOAD( "rd1cglm.7b",   0x0800000, 0x800000, CRC(447067fa) SHA1(e2052373773594feb303e1924a4a820cf34ab55b) )
	ROM_LOAD( "rd1cgum.6b",   0x1000000, 0x800000, CRC(c50de2ef) SHA1(24758a72b3569ce6a643a5786fce7c34b8aa692d) )
	ROM_LOAD( "rd1cguu.5b",   0x1800000, 0x800000, CRC(611bab41) SHA1(84cddb2b63bf8336e92aecb06eddf1b34af73540) )

	ROM_REGION( 0x1000000, "sprites", 0 )   /* sprites tiles */
	ROM_LOAD( "rd1sprll.12t", 0x0000000, 0x400000, CRC(8d450259) SHA1(27cccd1e7dad8880147bb85185982d8d27076e69) )
	ROM_LOAD( "rd1sprlm.11p", 0x0400000, 0x400000, CRC(6c8db3a5) SHA1(24d81fa11e9c835cddadec4cbd530738e258346c) )
	ROM_LOAD( "rd1sprum.10p", 0x0800000, 0x400000, CRC(8e08b2c6) SHA1(a17331a4e41f677f604d1b74e7694cf920b03b66) )
	ROM_LOAD( "rd1spruu.9p",  0x0c00000, 0x400000, CRC(f20a9673) SHA1(e5f1d552b0c42e102593ab578ff0b9ff814f8650) )

	ROM_REGION16_LE( 0x200000, "textilemapl", 0 )   /* texture tilemap 0-15 */
	ROM_LOAD( "rd1ccrl.11a",  0x000000, 0x200000, CRC(b0ea2b32) SHA1(0dc45846725b0de619bc6bae69e3eb166ed21bf0) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "rd1ccrh.11b",  0x000000, 0x200000, CRC(fafffb86) SHA1(15b0ba0252b99d0cac29fcb374fb895643f528fe) )

	ROM_REGION32_BE( 0x2000000, "pointrom", 0 ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "rd1pt0h.9l",   0x0000000, 0x400000, CRC(6f280eff) SHA1(9dd8c8903581d7a412146e50f4009e1d2b743f06) )
	ROM_LOAD32_WORD_SWAP( "rd1pt0l.9j",   0x0000002, 0x400000, CRC(47b1c5a5) SHA1(021d4ca7b8674d8ed5daa701bf41b4a7164d992a) )
	ROM_LOAD32_WORD_SWAP( "rd1pt1h.10l",  0x0800000, 0x400000, CRC(37bd9bdf) SHA1(b26c284024ea4ad4c67b2eefbfdd5ebb35a0118e) )
	ROM_LOAD32_WORD_SWAP( "rd1pt1l.10j",  0x0800002, 0x400000, CRC(91131cb3) SHA1(e42c5e190c719f1cf2d6e91444062ab901be0e73) )
	ROM_LOAD32_WORD_SWAP( "rd1pt2h.11l",  0x1000000, 0x400000, CRC(fa601e83) SHA1(45c420538910f566e75d668306735f54c901669f) )
	ROM_LOAD32_WORD_SWAP( "rd1pt2l.11j",  0x1000002, 0x400000, CRC(3423ff9f) SHA1(73823c179c866cbb601a23417acbbf5b3dc97213) )
	ROM_LOAD32_WORD_SWAP( "rd1pt3h.12l",  0x1800000, 0x400000, CRC(e82ff66a) SHA1(9e2c951136b26d969d2c9d030b7e0bad8bbbe3fb) )
	ROM_LOAD32_WORD_SWAP( "rd1pt3l.12j",  0x1800002, 0x400000, CRC(7216d63e) SHA1(77088ff05c2630996f4bdc87fe466f9b97611467) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
	ROM_LOAD( "rd1wavel.2s",  0x000000, 0x800000, CRC(bf52c08c) SHA1(6745062e078e520484390fad1f723124aa4076d0) )
	ROM_LOAD( "rd1waveh.3s",  0x800000, 0x800000, CRC(ef0136b5) SHA1(a6d923ededca168fe555e0b86a72f53bec5424cc) )

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate ROMs */
	ROM_LOAD( "rd1cgll.8f",   0x000000, 0x800000, CRC(b58b92ac) SHA1(70ee6e0e5347e05817aa30d53d766b8ce0fc44e4) )
	ROM_LOAD( "rd1cglm.7f",   0x000000, 0x800000, CRC(447067fa) SHA1(e2052373773594feb303e1924a4a820cf34ab55b) )
	ROM_LOAD( "rd1cgum.6f",   0x000000, 0x800000, CRC(c50de2ef) SHA1(24758a72b3569ce6a643a5786fce7c34b8aa692d) )
	ROM_LOAD( "rd1cguu.5f",   0x000000, 0x800000, CRC(611bab41) SHA1(84cddb2b63bf8336e92aecb06eddf1b34af73540) )
	ROM_LOAD( "rd1sprll.12p", 0x000000, 0x400000, CRC(8d450259) SHA1(27cccd1e7dad8880147bb85185982d8d27076e69) )
	ROM_LOAD( "rd1sprlm.11t", 0x000000, 0x400000, CRC(6c8db3a5) SHA1(24d81fa11e9c835cddadec4cbd530738e258346c) )
	ROM_LOAD( "rd1sprum.10t", 0x000000, 0x400000, CRC(8e08b2c6) SHA1(a17331a4e41f677f604d1b74e7694cf920b03b66) )
	ROM_LOAD( "rd1spruu.9t",  0x000000, 0x400000, CRC(f20a9673) SHA1(e5f1d552b0c42e102593ab578ff0b9ff814f8650) )
	ROM_LOAD( "rd1ccrl.11e",  0x000000, 0x200000, CRC(b0ea2b32) SHA1(0dc45846725b0de619bc6bae69e3eb166ed21bf0) )
	ROM_LOAD( "rd1ccrh.11f",  0x000000, 0x200000, CRC(fafffb86) SHA1(15b0ba0252b99d0cac29fcb374fb895643f528fe) )
ROM_END


ROM_START( rapidrvrp ) // prototype board
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "29f016.ic2",  0x000000, 0x200000, CRC(9f72a7cd) SHA1(06245f1d3cc6ffb5b0123a8eea0dc8338bdfc0d6) )
	ROM_LOAD16_BYTE( "29f016.ic1",  0x000001, 0x200000, CRC(d395a244) SHA1(7f7b7b75b4bf9ac8808a27afed87f503df28e49f) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "29f400.ic3",  0x000000, 0x080000, CRC(f194c942) SHA1(b581c97327dea092e30ba46ad630d10477343a39) )

	ROM_REGION32_BE( 0x800000, "data", 0 )  /* data */
	ROM_LOAD16_BYTE( "rd1mtah.3j",   0x000000, 0x400000, CRC(d8fa0f3d) SHA1(0d5bdb3a2e7be1dffe11b74baa2c10bfe011ae92) )
	ROM_LOAD16_BYTE( "rd1mtal.1j",   0x000001, 0x400000, CRC(8f0efa86) SHA1(9953461c258f2a96be275a7b18d6518ddfac3860) )

	ROM_REGION( 0x2000000, "textile", 0 )   /* texture tiles */
	ROM_LOAD( "rd1cgll.8b",   0x0000000, 0x800000, CRC(b58b92ac) SHA1(70ee6e0e5347e05817aa30d53d766b8ce0fc44e4) )
	ROM_LOAD( "rd1cglm.7b",   0x0800000, 0x800000, CRC(447067fa) SHA1(e2052373773594feb303e1924a4a820cf34ab55b) )
	ROM_LOAD( "rd1cgum.6b",   0x1000000, 0x800000, CRC(c50de2ef) SHA1(24758a72b3569ce6a643a5786fce7c34b8aa692d) )
	ROM_LOAD( "rd1cguu.5b",   0x1800000, 0x800000, CRC(611bab41) SHA1(84cddb2b63bf8336e92aecb06eddf1b34af73540) )

	ROM_REGION( 0x1000000, "sprites", 0 )   /* sprites tiles */
	ROM_LOAD( "rd1sprll.12t", 0x0000000, 0x400000, CRC(8d450259) SHA1(27cccd1e7dad8880147bb85185982d8d27076e69) )
	ROM_LOAD( "rd1sprlm.11p", 0x0400000, 0x400000, CRC(6c8db3a5) SHA1(24d81fa11e9c835cddadec4cbd530738e258346c) )
	ROM_LOAD( "rd1sprum.10p", 0x0800000, 0x400000, CRC(8e08b2c6) SHA1(a17331a4e41f677f604d1b74e7694cf920b03b66) )
	ROM_LOAD( "rd1spruu.9p",  0x0c00000, 0x400000, CRC(f20a9673) SHA1(e5f1d552b0c42e102593ab578ff0b9ff814f8650) )

	ROM_REGION16_LE( 0x200000, "textilemapl", 0 )   /* texture tilemap 0-15 */
	ROM_LOAD( "rd1ccrl.11a",  0x000000, 0x200000, CRC(b0ea2b32) SHA1(0dc45846725b0de619bc6bae69e3eb166ed21bf0) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "rd1ccrh.11b",  0x000000, 0x200000, CRC(fafffb86) SHA1(15b0ba0252b99d0cac29fcb374fb895643f528fe) )

	ROM_REGION32_BE( 0x2000000, "pointrom", 0 ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "rd1pt0h.9l",   0x0000000, 0x400000, CRC(6f280eff) SHA1(9dd8c8903581d7a412146e50f4009e1d2b743f06) )
	ROM_LOAD32_WORD_SWAP( "rd1pt0l.9j",   0x0000002, 0x400000, CRC(47b1c5a5) SHA1(021d4ca7b8674d8ed5daa701bf41b4a7164d992a) )
	ROM_LOAD32_WORD_SWAP( "rd1pt1h.10l",  0x0800000, 0x400000, CRC(37bd9bdf) SHA1(b26c284024ea4ad4c67b2eefbfdd5ebb35a0118e) )
	ROM_LOAD32_WORD_SWAP( "rd1pt1l.10j",  0x0800002, 0x400000, CRC(91131cb3) SHA1(e42c5e190c719f1cf2d6e91444062ab901be0e73) )
	ROM_LOAD32_WORD_SWAP( "rd1pt2h.11l",  0x1000000, 0x400000, CRC(fa601e83) SHA1(45c420538910f566e75d668306735f54c901669f) )
	ROM_LOAD32_WORD_SWAP( "rd1pt2l.11j",  0x1000002, 0x400000, CRC(3423ff9f) SHA1(73823c179c866cbb601a23417acbbf5b3dc97213) )
	ROM_LOAD32_WORD_SWAP( "rd1pt3h.12l",  0x1800000, 0x400000, CRC(e82ff66a) SHA1(9e2c951136b26d969d2c9d030b7e0bad8bbbe3fb) )
	ROM_LOAD32_WORD_SWAP( "rd1pt3l.12j",  0x1800002, 0x400000, CRC(7216d63e) SHA1(77088ff05c2630996f4bdc87fe466f9b97611467) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
	ROM_LOAD( "rd1wavel.2s",  0x000000, 0x800000, CRC(bf52c08c) SHA1(6745062e078e520484390fad1f723124aa4076d0) )
	ROM_LOAD( "rd1waveh.3s",  0x800000, 0x800000, CRC(ef0136b5) SHA1(a6d923ededca168fe555e0b86a72f53bec5424cc) )

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate ROMs */
	ROM_LOAD( "rd1cgll.8f",   0x000000, 0x800000, CRC(b58b92ac) SHA1(70ee6e0e5347e05817aa30d53d766b8ce0fc44e4) )
	ROM_LOAD( "rd1cglm.7f",   0x000000, 0x800000, CRC(447067fa) SHA1(e2052373773594feb303e1924a4a820cf34ab55b) )
	ROM_LOAD( "rd1cgum.6f",   0x000000, 0x800000, CRC(c50de2ef) SHA1(24758a72b3569ce6a643a5786fce7c34b8aa692d) )
	ROM_LOAD( "rd1cguu.5f",   0x000000, 0x800000, CRC(611bab41) SHA1(84cddb2b63bf8336e92aecb06eddf1b34af73540) )
	ROM_LOAD( "rd1sprll.12p", 0x000000, 0x400000, CRC(8d450259) SHA1(27cccd1e7dad8880147bb85185982d8d27076e69) )
	ROM_LOAD( "rd1sprlm.11t", 0x000000, 0x400000, CRC(6c8db3a5) SHA1(24d81fa11e9c835cddadec4cbd530738e258346c) )
	ROM_LOAD( "rd1sprum.10t", 0x000000, 0x400000, CRC(8e08b2c6) SHA1(a17331a4e41f677f604d1b74e7694cf920b03b66) )
	ROM_LOAD( "rd1spruu.9t",  0x000000, 0x400000, CRC(f20a9673) SHA1(e5f1d552b0c42e102593ab578ff0b9ff814f8650) )
	ROM_LOAD( "rd1ccrl.11e",  0x000000, 0x200000, CRC(b0ea2b32) SHA1(0dc45846725b0de619bc6bae69e3eb166ed21bf0) )
	ROM_LOAD( "rd1ccrh.11f",  0x000000, 0x200000, CRC(fafffb86) SHA1(15b0ba0252b99d0cac29fcb374fb895643f528fe) )
ROM_END


ROM_START( finfurl )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "ff2vera.ic2",  0x000000, 0x200000, CRC(e10f9dfa) SHA1(6f6989cd722fec5e3ed3ad1bb4866c5831041ae1) )
	ROM_LOAD16_BYTE( "ff2vera.ic1",  0x000001, 0x200000, CRC(5a90ffbf) SHA1(e22dc0ae2d3c3b3a521369fe3f63412ae2ae0a12) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "ff2vera.ic3",  0x000000, 0x080000, CRC(ab681078) SHA1(ec8367404458a54893ab6bea29c8a2ba3272b816) )

	ROM_REGION32_BE( 0x800000, "data", 0 )  /* data */
	ROM_LOAD16_BYTE( "ff2mtah.3j",   0x000000, 0x400000, CRC(161003cd) SHA1(04409333a4776b17700fc6d1aa06a39560132e03) )
	ROM_LOAD16_BYTE( "ff2mtal.1j",   0x000001, 0x400000, CRC(ed1a5bf2) SHA1(bd05388a125a0201a41af95fb2aa5fe1c8b0f270) )

	ROM_REGION( 0x1000000, "textile", 0 )   /* texture tiles */
	ROM_LOAD( "ff2cgll.8b",   0x0000000, 0x400000, CRC(8e6c34eb) SHA1(795631c8019011246ed1e5546de4433dc22dd9e7) )
	ROM_LOAD( "ff2cglm.7b",   0x0400000, 0x400000, CRC(406f321b) SHA1(41a2b0229d5370f141b9d6a4e1801e2f9973f660) )
	ROM_LOAD( "ff2cgum.6b",   0x0800000, 0x400000, CRC(b808be59) SHA1(906bfbb5d34feef9697da545a93930fe6e56685c) )
	ROM_LOAD( "ff2cguu.5b",   0x0c00000, 0x400000, CRC(595deee4) SHA1(b29ff9c6ba17737f1f87c05b2d899d80b0b72dbb) )

	ROM_REGION( 0x1000000, "sprites", 0 )   /* texture tiles bank 2? */
	ROM_LOAD( "ff2sprll.12t", 0x0000000, 0x400000, CRC(1b305a13) SHA1(3d213a77b7a019fe4511097e7a27aa0688a3a586) )
	ROM_LOAD( "ff2sprlm.11p", 0x0400000, 0x400000, CRC(421a8fbf) SHA1(8bd6f3e1ac9c7b0ac9d25dfbce35f5b7a5d5bcc7) )
	ROM_LOAD( "ff2sprum.10p", 0x0800000, 0x400000, CRC(cb53c03e) SHA1(c39a44cad240c5b77c235c07ea700f9847ab9482) )
	ROM_LOAD( "ff2spruu.9p",  0x0c00000, 0x400000, CRC(c134b0de) SHA1(cea9d9f4ce2f45a93c797ed467d8458521db9b3d) )

	ROM_REGION16_LE( 0x200000, "textilemapl", 0 )   /* texture tilemap 0-15 */
	ROM_LOAD( "ff2ccrl.11a",  0x000000, 0x200000, CRC(f1f9e77c) SHA1(adf659a4671ea066817e6620b7d7d5f60f6e01e5) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "ff2ccrh.11b",  0x000000, 0x200000, CRC(71228c61) SHA1(b39d0b51f36c0d00a6144ae20613bebee3ed22bc) )

	ROM_REGION32_BE( 0x800000, "pointrom", 0 )  /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "ff2pt0h.9l",   0x000000, 0x400000, CRC(344ce7a5) SHA1(79d2c4495b47592be4dee6e39294dd3194eb1d5f) )
	ROM_LOAD32_WORD_SWAP( "ff2pt0l.9j",   0x000002, 0x400000, CRC(7eeda441) SHA1(78648559abec5e1f04622cd1cfd5d94bddda7dbf) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
	ROM_LOAD( "ff2wavel.2s",  0x000000, 0x800000, CRC(6235c605) SHA1(521eaee80ac17c0936877d49394e5390fa0ff8a0) )
	ROM_LOAD( "ff2waveh.3s",  0x800000, 0x800000, CRC(2a59492a) SHA1(886ec0a4a71048d65f93c52df96416e74d23b3ec) )

	ROM_REGION( 0x400000, "dups", 0 )   /* duplicate ROMs */
	ROM_LOAD( "ff2cguu.5f",   0x000000, 0x400000, CRC(595deee4) SHA1(b29ff9c6ba17737f1f87c05b2d899d80b0b72dbb) )
	ROM_LOAD( "ff2cgum.6f",   0x000000, 0x400000, CRC(b808be59) SHA1(906bfbb5d34feef9697da545a93930fe6e56685c) )
	ROM_LOAD( "ff2cgll.8f",   0x000000, 0x400000, CRC(8e6c34eb) SHA1(795631c8019011246ed1e5546de4433dc22dd9e7) )
	ROM_LOAD( "ff2cglm.7f",   0x000000, 0x400000, CRC(406f321b) SHA1(41a2b0229d5370f141b9d6a4e1801e2f9973f660) )
	ROM_LOAD( "ff2spruu.9t",  0x000000, 0x400000, CRC(c134b0de) SHA1(cea9d9f4ce2f45a93c797ed467d8458521db9b3d) )
	ROM_LOAD( "ff2sprum.10t", 0x000000, 0x400000, CRC(cb53c03e) SHA1(c39a44cad240c5b77c235c07ea700f9847ab9482) )
	ROM_LOAD( "ff2sprll.12p", 0x000000, 0x400000, CRC(1b305a13) SHA1(3d213a77b7a019fe4511097e7a27aa0688a3a586) )
	ROM_LOAD( "ff2sprlm.11t", 0x000000, 0x400000, CRC(421a8fbf) SHA1(8bd6f3e1ac9c7b0ac9d25dfbce35f5b7a5d5bcc7) )
	ROM_LOAD( "ff2ccrl.11e",  0x000000, 0x200000, CRC(f1f9e77c) SHA1(adf659a4671ea066817e6620b7d7d5f60f6e01e5) )
	ROM_LOAD( "ff2ccrh.11f",  0x000000, 0x200000, CRC(71228c61) SHA1(b39d0b51f36c0d00a6144ae20613bebee3ed22bc) )
ROM_END


ROM_START( motoxgo )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "mg3vera.ic2",  0x000000, 0x200000, CRC(1bf06f00) SHA1(e9d04e9f19bff7a58cb280dd1d5db12801b68ba0) )
	ROM_LOAD16_BYTE( "mg3vera.ic1",  0x000001, 0x200000, CRC(f5e6e25b) SHA1(1de30e8e831be66987112645a9db3a3001b89fe6) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "mg3vera.ic3",  0x000000, 0x080000, CRC(9e3d46a8) SHA1(9ffa5b91ea51cc0fb97def25ce47efa3441f3c6f) )

	ROM_REGION( 0x20000, "exioboard", 0 )   /* "extra" I/O board (uses Fujitsu MB90611A MCU) */
	ROM_LOAD( "mg1prog0a.3a", 0x000000, 0x020000, CRC(b2b5be8f) SHA1(803652b7b8fde2196b7fb742ba8b9843e4fcd2de) )

	ROM_REGION32_BE( 0x2000000, "data", ROMREGION_ERASEFF ) /* data ROMs */
	ROM_LOAD16_BYTE( "mg1mtah.2j",   0x000000, 0x800000, CRC(845f4768) SHA1(9c03b1f6dcd9d1f43c2958d855221be7f9415c47) )
	ROM_LOAD16_BYTE( "mg1mtal.2h",   0x000001, 0x800000, CRC(fdad0f0a) SHA1(420d50f012af40f80b196d3aae320376e6c32367) )

	ROM_REGION( 0x2000000, "textile", ROMREGION_ERASEFF )   /* texture tiles */
	ROM_LOAD( "mg1cgll.4m",   0x0000000, 0x800000, CRC(175dfe34) SHA1(66ae35b0084159aea1afeb1a6486fffa635992b5) )
	ROM_LOAD( "mg1cglm.4k",   0x0800000, 0x800000, CRC(b3e648e7) SHA1(98018ae2276f905a7f74e1dab540a44247524436) )
	ROM_LOAD( "mg1cgum.4j",   0x1000000, 0x800000, CRC(46a77d73) SHA1(132ce2452ee68ba374e98b59032ac0a1a277078d) )

	ROM_REGION16_LE( 0x400000, "textilemapl", 0 )   /* texture tilemap 0-15 */
	ROM_LOAD( "mg1ccrl.7f",   0x000000, 0x400000, CRC(5372e300) SHA1(63a49782289ed93a321ca7d193241fb83ca97e6b) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "mg1ccrh.7e",   0x000000, 0x200000, CRC(2e77597d) SHA1(58dd83c1b0c08115e728c5e7dea5e62135b821ba) )

	ROM_REGION32_BE( 0x1000000, "pointrom", ROMREGION_ERASEFF ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "mg1pt0h.7a",   0x000000, 0x400000, CRC(c9ba1b47) SHA1(42ec0638edb4c502ff0a340c4cf590bdd767cfe2) )
	ROM_LOAD32_WORD_SWAP( "mg1pt0l.7c",   0x000002, 0x400000, CRC(3b9e95d3) SHA1(d7823ed6c590669ccd4098ed439599a3eb814ed1) )
	ROM_LOAD32_WORD_SWAP( "mg1pt1h.5a",   0x800000, 0x400000, CRC(8d4f7097) SHA1(004e9ed0b5d6ce83ffadb9bd429fa7560abdb598) )
	ROM_LOAD32_WORD_SWAP( "mg1pt1l.5c",   0x800002, 0x400000, CRC(0dd2f358) SHA1(3537e6be3fec9fec8d5a8dd02d9cf67b3805f8f0) )

	ROM_REGION( 0x1000000, "c352", ROMREGION_ERASEFF ) /* C352 PCM samples */
	ROM_LOAD( "mg1wavel.2c",  0x000000, 0x800000, CRC(f78b1b4d) SHA1(47cd654ec0a69de0dc81b8d83692eebf5611228b) )
	ROM_LOAD( "mg1waveh.2a",  0x800000, 0x800000, CRC(8cb73877) SHA1(2e2b170c7ff889770c13b4ab7ac316b386ada153) )

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate ROMs */
	ROM_LOAD( "mg1cgll.5m",   0x000000, 0x800000, CRC(175dfe34) SHA1(66ae35b0084159aea1afeb1a6486fffa635992b5) )
	ROM_LOAD( "mg1cglm.5k",   0x000000, 0x800000, CRC(b3e648e7) SHA1(98018ae2276f905a7f74e1dab540a44247524436) )
	ROM_LOAD( "mg1cgum.5j",   0x000000, 0x800000, CRC(46a77d73) SHA1(132ce2452ee68ba374e98b59032ac0a1a277078d) )
	ROM_LOAD( "mg1ccrl.7m",   0x000000, 0x400000, CRC(5372e300) SHA1(63a49782289ed93a321ca7d193241fb83ca97e6b) )
	ROM_LOAD( "mg1ccrh.7k",   0x400000, 0x200000, CRC(2e77597d) SHA1(58dd83c1b0c08115e728c5e7dea5e62135b821ba) )
ROM_END


ROM_START( motoxgov2a )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "mg2vera.ic2",  0x000000, 0x200000, CRC(66093336) SHA1(c87874245a70a1642fb9ecfc94cbbc89f0fd633f) )
	ROM_LOAD16_BYTE( "mg2vera.ic1",  0x000001, 0x200000, CRC(3dc7736f) SHA1(c5137aa449918a124415f8ea5581e037f841129c) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "mg3vera.ic3",  0x000000, 0x080000, CRC(9e3d46a8) SHA1(9ffa5b91ea51cc0fb97def25ce47efa3441f3c6f) )

	ROM_REGION( 0x20000, "exioboard", 0 )   /* "extra" I/O board (uses Fujitsu MB90611A MCU) */
	ROM_LOAD( "mg1prog0a.3a", 0x000000, 0x020000, CRC(b2b5be8f) SHA1(803652b7b8fde2196b7fb742ba8b9843e4fcd2de) )

	ROM_REGION32_BE( 0x2000000, "data", ROMREGION_ERASEFF ) /* data ROMs */
	ROM_LOAD16_BYTE( "mg1mtah.2j",   0x000000, 0x800000, CRC(845f4768) SHA1(9c03b1f6dcd9d1f43c2958d855221be7f9415c47) )
	ROM_LOAD16_BYTE( "mg1mtal.2h",   0x000001, 0x800000, CRC(fdad0f0a) SHA1(420d50f012af40f80b196d3aae320376e6c32367) )

	ROM_REGION( 0x2000000, "textile", ROMREGION_ERASEFF )   /* texture tiles */
	ROM_LOAD( "mg1cgll.4m",   0x0000000, 0x800000, CRC(175dfe34) SHA1(66ae35b0084159aea1afeb1a6486fffa635992b5) )
	ROM_LOAD( "mg1cglm.4k",   0x0800000, 0x800000, CRC(b3e648e7) SHA1(98018ae2276f905a7f74e1dab540a44247524436) )
	ROM_LOAD( "mg1cgum.4j",   0x1000000, 0x800000, CRC(46a77d73) SHA1(132ce2452ee68ba374e98b59032ac0a1a277078d) )

	ROM_REGION16_LE( 0x400000, "textilemapl", 0 )   /* texture tilemap 0-15 */
	ROM_LOAD( "mg1ccrl.7f",   0x000000, 0x400000, CRC(5372e300) SHA1(63a49782289ed93a321ca7d193241fb83ca97e6b) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "mg1ccrh.7e",   0x000000, 0x200000, CRC(2e77597d) SHA1(58dd83c1b0c08115e728c5e7dea5e62135b821ba) )

	ROM_REGION32_BE( 0x1000000, "pointrom", ROMREGION_ERASEFF ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "mg1pt0h.7a",   0x000000, 0x400000, CRC(c9ba1b47) SHA1(42ec0638edb4c502ff0a340c4cf590bdd767cfe2) )
	ROM_LOAD32_WORD_SWAP( "mg1pt0l.7c",   0x000002, 0x400000, CRC(3b9e95d3) SHA1(d7823ed6c590669ccd4098ed439599a3eb814ed1) )
	ROM_LOAD32_WORD_SWAP( "mg1pt1h.5a",   0x800000, 0x400000, CRC(8d4f7097) SHA1(004e9ed0b5d6ce83ffadb9bd429fa7560abdb598) )
	ROM_LOAD32_WORD_SWAP( "mg1pt1l.5c",   0x800002, 0x400000, CRC(0dd2f358) SHA1(3537e6be3fec9fec8d5a8dd02d9cf67b3805f8f0) )

	ROM_REGION( 0x1000000, "c352", ROMREGION_ERASEFF ) /* C352 PCM samples */
	ROM_LOAD( "mg1wavel.2c",  0x000000, 0x800000, CRC(f78b1b4d) SHA1(47cd654ec0a69de0dc81b8d83692eebf5611228b) )
	ROM_LOAD( "mg1waveh.2a",  0x800000, 0x800000, CRC(8cb73877) SHA1(2e2b170c7ff889770c13b4ab7ac316b386ada153) )

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate ROMs */
	ROM_LOAD( "mg1cgll.5m",   0x000000, 0x800000, CRC(175dfe34) SHA1(66ae35b0084159aea1afeb1a6486fffa635992b5) )
	ROM_LOAD( "mg1cglm.5k",   0x000000, 0x800000, CRC(b3e648e7) SHA1(98018ae2276f905a7f74e1dab540a44247524436) )
	ROM_LOAD( "mg1cgum.5j",   0x000000, 0x800000, CRC(46a77d73) SHA1(132ce2452ee68ba374e98b59032ac0a1a277078d) )
	ROM_LOAD( "mg1ccrl.7m",   0x000000, 0x400000, CRC(5372e300) SHA1(63a49782289ed93a321ca7d193241fb83ca97e6b) )
	ROM_LOAD( "mg1ccrh.7k",   0x400000, 0x200000, CRC(2e77597d) SHA1(58dd83c1b0c08115e728c5e7dea5e62135b821ba) )
ROM_END

ROM_START( motoxgov2a2 )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "mg2vera1.ic2",  0x000000, 0x200000, CRC(0deab179) SHA1(9b45c0db3921bdfcd0eb220939a1197d8dd2860b) )
	ROM_LOAD16_BYTE( "mg2vera1.ic1",  0x000001, 0x200000, CRC(4b1bc511) SHA1(884c08a0d09e885582283c0b3a5e56a184ccd1b5) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "mg3vera.ic3",  0x000000, 0x080000, CRC(9e3d46a8) SHA1(9ffa5b91ea51cc0fb97def25ce47efa3441f3c6f) )

	ROM_REGION( 0x20000, "exioboard", 0 )   /* "extra" I/O board (uses Fujitsu MB90611A MCU) */
	ROM_LOAD( "mg1prog0a.3a", 0x000000, 0x020000, CRC(b2b5be8f) SHA1(803652b7b8fde2196b7fb742ba8b9843e4fcd2de) )

	ROM_REGION32_BE( 0x2000000, "data", ROMREGION_ERASEFF ) /* data ROMs */
	ROM_LOAD16_BYTE( "mg1mtah.2j",   0x000000, 0x800000, CRC(845f4768) SHA1(9c03b1f6dcd9d1f43c2958d855221be7f9415c47) )
	ROM_LOAD16_BYTE( "mg1mtal.2h",   0x000001, 0x800000, CRC(fdad0f0a) SHA1(420d50f012af40f80b196d3aae320376e6c32367) )

	ROM_REGION( 0x2000000, "textile", ROMREGION_ERASEFF )   /* texture tiles */
	ROM_LOAD( "mg1cgll.4m",   0x0000000, 0x800000, CRC(175dfe34) SHA1(66ae35b0084159aea1afeb1a6486fffa635992b5) )
	ROM_LOAD( "mg1cglm.4k",   0x0800000, 0x800000, CRC(b3e648e7) SHA1(98018ae2276f905a7f74e1dab540a44247524436) )
	ROM_LOAD( "mg1cgum.4j",   0x1000000, 0x800000, CRC(46a77d73) SHA1(132ce2452ee68ba374e98b59032ac0a1a277078d) )

	ROM_REGION16_LE( 0x400000, "textilemapl", 0 )   /* texture tilemap 0-15 */
	ROM_LOAD( "mg1ccrl.7f",   0x000000, 0x400000, CRC(5372e300) SHA1(63a49782289ed93a321ca7d193241fb83ca97e6b) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "mg1ccrh.7e",   0x000000, 0x200000, CRC(2e77597d) SHA1(58dd83c1b0c08115e728c5e7dea5e62135b821ba) )

	ROM_REGION32_BE( 0x1000000, "pointrom", ROMREGION_ERASEFF ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "mg1pt0h.7a",   0x000000, 0x400000, CRC(c9ba1b47) SHA1(42ec0638edb4c502ff0a340c4cf590bdd767cfe2) )
	ROM_LOAD32_WORD_SWAP( "mg1pt0l.7c",   0x000002, 0x400000, CRC(3b9e95d3) SHA1(d7823ed6c590669ccd4098ed439599a3eb814ed1) )
	ROM_LOAD32_WORD_SWAP( "mg1pt1h.5a",   0x800000, 0x400000, CRC(8d4f7097) SHA1(004e9ed0b5d6ce83ffadb9bd429fa7560abdb598) )
	ROM_LOAD32_WORD_SWAP( "mg1pt1l.5c",   0x800002, 0x400000, CRC(0dd2f358) SHA1(3537e6be3fec9fec8d5a8dd02d9cf67b3805f8f0) )

	ROM_REGION( 0x1000000, "c352", ROMREGION_ERASEFF ) /* C352 PCM samples */
	ROM_LOAD( "mg1wavel.2c",  0x000000, 0x800000, CRC(f78b1b4d) SHA1(47cd654ec0a69de0dc81b8d83692eebf5611228b) )
	ROM_LOAD( "mg1waveh.2a",  0x800000, 0x800000, CRC(8cb73877) SHA1(2e2b170c7ff889770c13b4ab7ac316b386ada153) )

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate ROMs */
	ROM_LOAD( "mg1cgll.5m",   0x000000, 0x800000, CRC(175dfe34) SHA1(66ae35b0084159aea1afeb1a6486fffa635992b5) )
	ROM_LOAD( "mg1cglm.5k",   0x000000, 0x800000, CRC(b3e648e7) SHA1(98018ae2276f905a7f74e1dab540a44247524436) )
	ROM_LOAD( "mg1cgum.5j",   0x000000, 0x800000, CRC(46a77d73) SHA1(132ce2452ee68ba374e98b59032ac0a1a277078d) )
	ROM_LOAD( "mg1ccrl.7m",   0x000000, 0x400000, CRC(5372e300) SHA1(63a49782289ed93a321ca7d193241fb83ca97e6b) )
	ROM_LOAD( "mg1ccrh.7k",   0x400000, 0x200000, CRC(2e77597d) SHA1(58dd83c1b0c08115e728c5e7dea5e62135b821ba) )
ROM_END

ROM_START( motoxgov1a )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "mg1vera.ic2",  0x000000, 0x200000, CRC(5ba13d9e) SHA1(7f6484df644772f2478155c05844532f8abbd196) )
	ROM_LOAD16_BYTE( "mg1vera.ic1",  0x000001, 0x200000, CRC(193b463e) SHA1(f62eed49f7f8bf01b8b4deb1578ddee1d4a54ca3) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "mg3vera.ic3",  0x000000, 0x080000, CRC(9e3d46a8) SHA1(9ffa5b91ea51cc0fb97def25ce47efa3441f3c6f) )

	ROM_REGION( 0x20000, "exioboard", 0 )   /* "extra" I/O board (uses Fujitsu MB90611A MCU) */
	ROM_LOAD( "mg1prog0a.3a", 0x000000, 0x020000, CRC(b2b5be8f) SHA1(803652b7b8fde2196b7fb742ba8b9843e4fcd2de) )

	ROM_REGION32_BE( 0x2000000, "data", ROMREGION_ERASEFF ) /* data ROMs */
	ROM_LOAD16_BYTE( "mg1mtah.2j",   0x000000, 0x800000, CRC(845f4768) SHA1(9c03b1f6dcd9d1f43c2958d855221be7f9415c47) )
	ROM_LOAD16_BYTE( "mg1mtal.2h",   0x000001, 0x800000, CRC(fdad0f0a) SHA1(420d50f012af40f80b196d3aae320376e6c32367) )

	ROM_REGION( 0x2000000, "textile", ROMREGION_ERASEFF )   /* texture tiles */
	ROM_LOAD( "mg1cgll.4m",   0x0000000, 0x800000, CRC(175dfe34) SHA1(66ae35b0084159aea1afeb1a6486fffa635992b5) )
	ROM_LOAD( "mg1cglm.4k",   0x0800000, 0x800000, CRC(b3e648e7) SHA1(98018ae2276f905a7f74e1dab540a44247524436) )
	ROM_LOAD( "mg1cgum.4j",   0x1000000, 0x800000, CRC(46a77d73) SHA1(132ce2452ee68ba374e98b59032ac0a1a277078d) )

	ROM_REGION16_LE( 0x400000, "textilemapl", 0 )   /* texture tilemap 0-15 */
	ROM_LOAD( "mg1ccrl.7f",   0x000000, 0x400000, CRC(5372e300) SHA1(63a49782289ed93a321ca7d193241fb83ca97e6b) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "mg1ccrh.7e",   0x000000, 0x200000, CRC(2e77597d) SHA1(58dd83c1b0c08115e728c5e7dea5e62135b821ba) )

	ROM_REGION32_BE( 0x1000000, "pointrom", ROMREGION_ERASEFF ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "mg1pt0h.7a",   0x000000, 0x400000, CRC(c9ba1b47) SHA1(42ec0638edb4c502ff0a340c4cf590bdd767cfe2) )
	ROM_LOAD32_WORD_SWAP( "mg1pt0l.7c",   0x000002, 0x400000, CRC(3b9e95d3) SHA1(d7823ed6c590669ccd4098ed439599a3eb814ed1) )
	ROM_LOAD32_WORD_SWAP( "mg1pt1h.5a",   0x800000, 0x400000, CRC(8d4f7097) SHA1(004e9ed0b5d6ce83ffadb9bd429fa7560abdb598) )
	ROM_LOAD32_WORD_SWAP( "mg1pt1l.5c",   0x800002, 0x400000, CRC(0dd2f358) SHA1(3537e6be3fec9fec8d5a8dd02d9cf67b3805f8f0) )

	ROM_REGION( 0x1000000, "c352", ROMREGION_ERASEFF ) /* C352 PCM samples */
	ROM_LOAD( "mg1wavel.2c",  0x000000, 0x800000, CRC(f78b1b4d) SHA1(47cd654ec0a69de0dc81b8d83692eebf5611228b) )
	ROM_LOAD( "mg1waveh.2a",  0x800000, 0x800000, CRC(8cb73877) SHA1(2e2b170c7ff889770c13b4ab7ac316b386ada153) )

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate ROMs */
	ROM_LOAD( "mg1cgll.5m",   0x000000, 0x800000, CRC(175dfe34) SHA1(66ae35b0084159aea1afeb1a6486fffa635992b5) )
	ROM_LOAD( "mg1cglm.5k",   0x000000, 0x800000, CRC(b3e648e7) SHA1(98018ae2276f905a7f74e1dab540a44247524436) )
	ROM_LOAD( "mg1cgum.5j",   0x000000, 0x800000, CRC(46a77d73) SHA1(132ce2452ee68ba374e98b59032ac0a1a277078d) )
	ROM_LOAD( "mg1ccrl.7m",   0x000000, 0x400000, CRC(5372e300) SHA1(63a49782289ed93a321ca7d193241fb83ca97e6b) )
	ROM_LOAD( "mg1ccrh.7k",   0x400000, 0x200000, CRC(2e77597d) SHA1(58dd83c1b0c08115e728c5e7dea5e62135b821ba) )
ROM_END


ROM_START( motoxgov1a2 )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "mg1vera1.ic2",  0x000000, 0x200000, CRC(532ec687) SHA1(1e822b9afa00a897c0ad2341e33ebc93962a8244) )
	ROM_LOAD16_BYTE( "mg1vera1.ic1",  0x000001, 0x200000, CRC(3154b80a) SHA1(ecec56dfd594f5fc651478fa3ae8963182cb94c3) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "mg3vera.ic3",  0x000000, 0x080000, CRC(9e3d46a8) SHA1(9ffa5b91ea51cc0fb97def25ce47efa3441f3c6f) )

	ROM_REGION( 0x20000, "exioboard", 0 )   /* "extra" I/O board (uses Fujitsu MB90611A MCU) */
	ROM_LOAD( "mg1prog0a.3a", 0x000000, 0x020000, CRC(b2b5be8f) SHA1(803652b7b8fde2196b7fb742ba8b9843e4fcd2de) )

	ROM_REGION32_BE( 0x2000000, "data", ROMREGION_ERASEFF ) /* data ROMs */
	ROM_LOAD16_BYTE( "mg1mtah.2j",   0x000000, 0x800000, CRC(845f4768) SHA1(9c03b1f6dcd9d1f43c2958d855221be7f9415c47) )
	ROM_LOAD16_BYTE( "mg1mtal.2h",   0x000001, 0x800000, CRC(fdad0f0a) SHA1(420d50f012af40f80b196d3aae320376e6c32367) )

	ROM_REGION( 0x2000000, "textile", ROMREGION_ERASEFF )   /* texture tiles */
	ROM_LOAD( "mg1cgll.4m",   0x0000000, 0x800000, CRC(175dfe34) SHA1(66ae35b0084159aea1afeb1a6486fffa635992b5) )
	ROM_LOAD( "mg1cglm.4k",   0x0800000, 0x800000, CRC(b3e648e7) SHA1(98018ae2276f905a7f74e1dab540a44247524436) )
	ROM_LOAD( "mg1cgum.4j",   0x1000000, 0x800000, CRC(46a77d73) SHA1(132ce2452ee68ba374e98b59032ac0a1a277078d) )

	ROM_REGION16_LE( 0x400000, "textilemapl", 0 )   /* texture tilemap 0-15 */
	ROM_LOAD( "mg1ccrl.7f",   0x000000, 0x400000, CRC(5372e300) SHA1(63a49782289ed93a321ca7d193241fb83ca97e6b) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "mg1ccrh.7e",   0x000000, 0x200000, CRC(2e77597d) SHA1(58dd83c1b0c08115e728c5e7dea5e62135b821ba) )

	ROM_REGION32_BE( 0x1000000, "pointrom", ROMREGION_ERASEFF ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "mg1pt0h.7a",   0x000000, 0x400000, CRC(c9ba1b47) SHA1(42ec0638edb4c502ff0a340c4cf590bdd767cfe2) )
	ROM_LOAD32_WORD_SWAP( "mg1pt0l.7c",   0x000002, 0x400000, CRC(3b9e95d3) SHA1(d7823ed6c590669ccd4098ed439599a3eb814ed1) )
	ROM_LOAD32_WORD_SWAP( "mg1pt1h.5a",   0x800000, 0x400000, CRC(8d4f7097) SHA1(004e9ed0b5d6ce83ffadb9bd429fa7560abdb598) )
	ROM_LOAD32_WORD_SWAP( "mg1pt1l.5c",   0x800002, 0x400000, CRC(0dd2f358) SHA1(3537e6be3fec9fec8d5a8dd02d9cf67b3805f8f0) )

	ROM_REGION( 0x1000000, "c352", ROMREGION_ERASEFF ) /* C352 PCM samples */
	ROM_LOAD( "mg1wavel.2c",  0x000000, 0x800000, CRC(f78b1b4d) SHA1(47cd654ec0a69de0dc81b8d83692eebf5611228b) )
	ROM_LOAD( "mg1waveh.2a",  0x800000, 0x800000, CRC(8cb73877) SHA1(2e2b170c7ff889770c13b4ab7ac316b386ada153) )

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate ROMs */
	ROM_LOAD( "mg1cgll.5m",   0x000000, 0x800000, CRC(175dfe34) SHA1(66ae35b0084159aea1afeb1a6486fffa635992b5) )
	ROM_LOAD( "mg1cglm.5k",   0x000000, 0x800000, CRC(b3e648e7) SHA1(98018ae2276f905a7f74e1dab540a44247524436) )
	ROM_LOAD( "mg1cgum.5j",   0x000000, 0x800000, CRC(46a77d73) SHA1(132ce2452ee68ba374e98b59032ac0a1a277078d) )
	ROM_LOAD( "mg1ccrl.7m",   0x000000, 0x400000, CRC(5372e300) SHA1(63a49782289ed93a321ca7d193241fb83ca97e6b) )
	ROM_LOAD( "mg1ccrh.7k",   0x400000, 0x200000, CRC(2e77597d) SHA1(58dd83c1b0c08115e728c5e7dea5e62135b821ba) )
ROM_END


ROM_START( timecrs2 )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "tss3verb.2",   0x000000, 0x200000, CRC(c7be691f) SHA1(5e2e7a0db3d8ce6dfeb6c0d99e9fe6a9f9cab467) )
	ROM_LOAD16_BYTE( "tss3verb.1",   0x000001, 0x200000, CRC(6e3f232b) SHA1(8007d8f31a605a5df89938d7c9f9d3d209c934be) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "tss1vera.3",   0x000000, 0x080000, CRC(41e41994) SHA1(eabc1a307c329070bfc6486cb68169c94ff8a162) ) /* Flash ROM type 29F400TC - Common code throughout all versions */

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data ROMs */
	ROM_LOAD16_BYTE( "tss1mtah.2j",  0x0000000, 0x800000, CRC(697c26ed) SHA1(72f6f69e89496ba0c6183b35c3bde71f5a3c721f) )
	ROM_LOAD16_BYTE( "tss1mtal.2h",  0x0000001, 0x800000, CRC(bfc79190) SHA1(04bda00c4cc5660d27af4f3b0ee3550dea8d3805) )
	ROM_LOAD16_BYTE( "tss1mtbh.2m",  0x1000000, 0x800000, CRC(82582776) SHA1(7c790d09bac660ea1c62da3ffb21ab43f2461594) )
	ROM_LOAD16_BYTE( "tss1mtbl.2f",  0x1000001, 0x800000, CRC(e648bea4) SHA1(3803d03e72b25fbcc124d5b25066d25629b76b94) )

	ROM_REGION( 0x2000000, "textile", 0 )   /* texture tiles */
	ROM_LOAD( "tss1cgll.4m",  0x0000000, 0x800000, CRC(18433aaa) SHA1(08539beb2e66ec4e41062621fc098b121c669546) )
	ROM_LOAD( "tss1cglm.4k",  0x0800000, 0x800000, CRC(669974c2) SHA1(cfebe199631e38f547b38fcd35f1645b74e8dd0a) )
	ROM_LOAD( "tss1cgum.4j",  0x1000000, 0x800000, CRC(c22739e1) SHA1(8671ee047bb248033656c50befd1c35e5e478e1a) )
	ROM_LOAD( "tss1cguu.4f",  0x1800000, 0x800000, CRC(76924e04) SHA1(751065d6ce658cbbcd88f854f6937ebd2204ec68) )

	ROM_REGION16_LE( 0x400000, "textilemapl", 0 )   /* texture tilemap 0-15 */
	ROM_LOAD( "tss1ccrl.7f",  0x000000, 0x400000, CRC(3a325fe7) SHA1(882735dce7aeb36f9e88a983498360f5de901e9d) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "tss1ccrh.7e",  0x000000, 0x200000, CRC(f998de1a) SHA1(371f540f505608297c5ffcfb623b983ca8310afb) )

	ROM_REGION32_BE( 0x2000000, "pointrom", 0 ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "tss1pt0h.7a",  0x0000000, 0x400000, CRC(cdbe0ba8) SHA1(f8c6da31654c0a2a8024888ffb7fc1c783b2d629) )
	ROM_LOAD32_WORD_SWAP( "tss1pt0l.7c",  0x0000002, 0x400000, CRC(896f0fb4) SHA1(bdfa99eb21ce4fc8021f9d95a5558a34f9942c57) )
	ROM_LOAD32_WORD_SWAP( "tss1pt1h.5a",  0x0800000, 0x400000, CRC(63647596) SHA1(833412be8f61686bd7e06c2738df740e0e585d0f) )
	ROM_LOAD32_WORD_SWAP( "tss1pt1l.5c",  0x0800002, 0x400000, CRC(5a09921f) SHA1(c23885708c7adf0b81c2c9346e21b869634a5b35) )
	ROM_LOAD32_WORD_SWAP( "tss1pt2h.4a",  0x1000000, 0x400000, CRC(9b06e22d) SHA1(cff5ed098112a4f0a2bc8937e226f50066e605b1) )
	ROM_LOAD32_WORD_SWAP( "tss1pt2l.4c",  0x1000002, 0x400000, CRC(4b230d79) SHA1(794cee0a19993e90913f58507c53224f361e9663) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
	ROM_LOAD( "tss1wavel.2c", 0x000000, 0x800000, CRC(deaead26) SHA1(72dac0c3f41d4c3c290f9eb1b50236ae3040a472) )
	ROM_LOAD( "tss1waveh.2a", 0x800000, 0x800000, CRC(5c8758b4) SHA1(b85c8f6869900224ef83a2340b17f5bbb2801af9) )
ROM_END


ROM_START( timecrs2v2b )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "tss2verb.2", 0x000000, 0x200000, CRC(fb129049) SHA1(c975ea022b3a2a249a6ab60e2e0358f9dc507775) )
	ROM_LOAD16_BYTE( "tss2verb.1", 0x000001, 0x200000, CRC(2d6a1d3e) SHA1(2b6bc54427c1ae2fcdb57a33b2b2b00bd2065109) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "tss1vera.3",   0x000000, 0x080000, CRC(41e41994) SHA1(eabc1a307c329070bfc6486cb68169c94ff8a162) ) /* Flash ROM type 29F400TC - Common code throughout all versions */

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data ROMs */
	ROM_LOAD16_BYTE( "tss1mtah.2j",  0x0000000, 0x800000, CRC(697c26ed) SHA1(72f6f69e89496ba0c6183b35c3bde71f5a3c721f) )
	ROM_LOAD16_BYTE( "tss1mtal.2h",  0x0000001, 0x800000, CRC(bfc79190) SHA1(04bda00c4cc5660d27af4f3b0ee3550dea8d3805) )
	ROM_LOAD16_BYTE( "tss1mtbh.2m",  0x1000000, 0x800000, CRC(82582776) SHA1(7c790d09bac660ea1c62da3ffb21ab43f2461594) )
	ROM_LOAD16_BYTE( "tss1mtbl.2f",  0x1000001, 0x800000, CRC(e648bea4) SHA1(3803d03e72b25fbcc124d5b25066d25629b76b94) )

	ROM_REGION( 0x2000000, "textile", 0 )   /* texture tiles */
	ROM_LOAD( "tss1cgll.4m",  0x0000000, 0x800000, CRC(18433aaa) SHA1(08539beb2e66ec4e41062621fc098b121c669546) )
	ROM_LOAD( "tss1cglm.4k",  0x0800000, 0x800000, CRC(669974c2) SHA1(cfebe199631e38f547b38fcd35f1645b74e8dd0a) )
	ROM_LOAD( "tss1cgum.4j",  0x1000000, 0x800000, CRC(c22739e1) SHA1(8671ee047bb248033656c50befd1c35e5e478e1a) )
	ROM_LOAD( "tss1cguu.4f",  0x1800000, 0x800000, CRC(76924e04) SHA1(751065d6ce658cbbcd88f854f6937ebd2204ec68) )

	ROM_REGION16_LE( 0x400000, "textilemapl", 0 )   /* texture tilemap 0-15 */
	ROM_LOAD( "tss1ccrl.7f",  0x000000, 0x400000, CRC(3a325fe7) SHA1(882735dce7aeb36f9e88a983498360f5de901e9d) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "tss1ccrh.7e",  0x000000, 0x200000, CRC(f998de1a) SHA1(371f540f505608297c5ffcfb623b983ca8310afb) )

	ROM_REGION32_BE( 0x2000000, "pointrom", 0 ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "tss1pt0h.7a",  0x0000000, 0x400000, CRC(cdbe0ba8) SHA1(f8c6da31654c0a2a8024888ffb7fc1c783b2d629) )
	ROM_LOAD32_WORD_SWAP( "tss1pt0l.7c",  0x0000002, 0x400000, CRC(896f0fb4) SHA1(bdfa99eb21ce4fc8021f9d95a5558a34f9942c57) )
	ROM_LOAD32_WORD_SWAP( "tss1pt1h.5a",  0x0800000, 0x400000, CRC(63647596) SHA1(833412be8f61686bd7e06c2738df740e0e585d0f) )
	ROM_LOAD32_WORD_SWAP( "tss1pt1l.5c",  0x0800002, 0x400000, CRC(5a09921f) SHA1(c23885708c7adf0b81c2c9346e21b869634a5b35) )
	ROM_LOAD32_WORD_SWAP( "tss1pt2h.4a",  0x1000000, 0x400000, CRC(9b06e22d) SHA1(cff5ed098112a4f0a2bc8937e226f50066e605b1) )
	ROM_LOAD32_WORD_SWAP( "tss1pt2l.4c",  0x1000002, 0x400000, CRC(4b230d79) SHA1(794cee0a19993e90913f58507c53224f361e9663) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
	ROM_LOAD( "tss1wavel.2c", 0x000000, 0x800000, CRC(deaead26) SHA1(72dac0c3f41d4c3c290f9eb1b50236ae3040a472) )
	ROM_LOAD( "tss1waveh.2a", 0x800000, 0x800000, CRC(5c8758b4) SHA1(b85c8f6869900224ef83a2340b17f5bbb2801af9) )
ROM_END


ROM_START( timecrs2v1b )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "tss1verb.2", 0x000000, 0x200000, CRC(10ebcb71) SHA1(9aeb9bf70f8a5949927ed84490b9212b19fe57ab) ) /* Flash ROM type 29F016A */
	ROM_LOAD16_BYTE( "tss1verb.1", 0x000001, 0x200000, CRC(4f9a38ef) SHA1(7e38dae63b68f7ab43355b22247938d6e8f43dee) ) /* Flash ROM type 29F016A */

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "tss1vera.3",   0x000000, 0x080000, CRC(41e41994) SHA1(eabc1a307c329070bfc6486cb68169c94ff8a162) ) /* Flash ROM type 29F400TC - Common code throughout all versions */

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data ROMs */
	ROM_LOAD16_BYTE( "tss1mtah.2j",  0x0000000, 0x800000, CRC(697c26ed) SHA1(72f6f69e89496ba0c6183b35c3bde71f5a3c721f) )
	ROM_LOAD16_BYTE( "tss1mtal.2h",  0x0000001, 0x800000, CRC(bfc79190) SHA1(04bda00c4cc5660d27af4f3b0ee3550dea8d3805) )
	ROM_LOAD16_BYTE( "tss1mtbh.2m",  0x1000000, 0x800000, CRC(82582776) SHA1(7c790d09bac660ea1c62da3ffb21ab43f2461594) )
	ROM_LOAD16_BYTE( "tss1mtbl.2f",  0x1000001, 0x800000, CRC(e648bea4) SHA1(3803d03e72b25fbcc124d5b25066d25629b76b94) )

	ROM_REGION( 0x2000000, "textile", 0 )   /* texture tiles */
	ROM_LOAD( "tss1cgll.4m",  0x0000000, 0x800000, CRC(18433aaa) SHA1(08539beb2e66ec4e41062621fc098b121c669546) )
	ROM_LOAD( "tss1cglm.4k",  0x0800000, 0x800000, CRC(669974c2) SHA1(cfebe199631e38f547b38fcd35f1645b74e8dd0a) )
	ROM_LOAD( "tss1cgum.4j",  0x1000000, 0x800000, CRC(c22739e1) SHA1(8671ee047bb248033656c50befd1c35e5e478e1a) )
	ROM_LOAD( "tss1cguu.4f",  0x1800000, 0x800000, CRC(76924e04) SHA1(751065d6ce658cbbcd88f854f6937ebd2204ec68) )

	ROM_REGION16_LE( 0x400000, "textilemapl", 0 )   /* texture tilemap 0-15 */
	ROM_LOAD( "tss1ccrl.7f",  0x000000, 0x400000, CRC(3a325fe7) SHA1(882735dce7aeb36f9e88a983498360f5de901e9d) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "tss1ccrh.7e",  0x000000, 0x200000, CRC(f998de1a) SHA1(371f540f505608297c5ffcfb623b983ca8310afb) )

	ROM_REGION32_BE( 0x2000000, "pointrom", 0 ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "tss1pt0h.7a",  0x0000000, 0x400000, CRC(cdbe0ba8) SHA1(f8c6da31654c0a2a8024888ffb7fc1c783b2d629) )
	ROM_LOAD32_WORD_SWAP( "tss1pt0l.7c",  0x0000002, 0x400000, CRC(896f0fb4) SHA1(bdfa99eb21ce4fc8021f9d95a5558a34f9942c57) )
	ROM_LOAD32_WORD_SWAP( "tss1pt1h.5a",  0x0800000, 0x400000, CRC(63647596) SHA1(833412be8f61686bd7e06c2738df740e0e585d0f) )
	ROM_LOAD32_WORD_SWAP( "tss1pt1l.5c",  0x0800002, 0x400000, CRC(5a09921f) SHA1(c23885708c7adf0b81c2c9346e21b869634a5b35) )
	ROM_LOAD32_WORD_SWAP( "tss1pt2h.4a",  0x1000000, 0x400000, CRC(9b06e22d) SHA1(cff5ed098112a4f0a2bc8937e226f50066e605b1) )
	ROM_LOAD32_WORD_SWAP( "tss1pt2l.4c",  0x1000002, 0x400000, CRC(4b230d79) SHA1(794cee0a19993e90913f58507c53224f361e9663) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
	ROM_LOAD( "tss1wavel.2c", 0x000000, 0x800000, CRC(deaead26) SHA1(72dac0c3f41d4c3c290f9eb1b50236ae3040a472) )
	ROM_LOAD( "tss1waveh.2a", 0x800000, 0x800000, CRC(5c8758b4) SHA1(b85c8f6869900224ef83a2340b17f5bbb2801af9) )
ROM_END


ROM_START( timecrs2v4a )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "tss4vera.2",   0x000000, 0x200000, CRC(c84edd3b) SHA1(0b577a8ef6e74afa991dd81c2db19041787724da) )
	ROM_LOAD16_BYTE( "tss4vera.1",   0x000001, 0x200000, CRC(26f57c83) SHA1(c8983c26b7524a35257a242b66a9413eb354ca0d) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "tss1vera.3",   0x000000, 0x080000, CRC(41e41994) SHA1(eabc1a307c329070bfc6486cb68169c94ff8a162) ) /* Flash ROM type 29F400TC - Common code throughout all versions */

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data ROMs */
	ROM_LOAD16_BYTE( "tss1mtah.2j",  0x0000000, 0x800000, CRC(697c26ed) SHA1(72f6f69e89496ba0c6183b35c3bde71f5a3c721f) )
	ROM_LOAD16_BYTE( "tss1mtal.2h",  0x0000001, 0x800000, CRC(bfc79190) SHA1(04bda00c4cc5660d27af4f3b0ee3550dea8d3805) )
	ROM_LOAD16_BYTE( "tss1mtbh.2m",  0x1000000, 0x800000, CRC(82582776) SHA1(7c790d09bac660ea1c62da3ffb21ab43f2461594) )
	ROM_LOAD16_BYTE( "tss1mtbl.2f",  0x1000001, 0x800000, CRC(e648bea4) SHA1(3803d03e72b25fbcc124d5b25066d25629b76b94) )

	ROM_REGION( 0x2000000, "textile", 0 )   /* texture tiles */
	ROM_LOAD( "tss1cgll.4m",  0x0000000, 0x800000, CRC(18433aaa) SHA1(08539beb2e66ec4e41062621fc098b121c669546) )
	ROM_LOAD( "tss1cglm.4k",  0x0800000, 0x800000, CRC(669974c2) SHA1(cfebe199631e38f547b38fcd35f1645b74e8dd0a) )
	ROM_LOAD( "tss1cgum.4j",  0x1000000, 0x800000, CRC(c22739e1) SHA1(8671ee047bb248033656c50befd1c35e5e478e1a) )
	ROM_LOAD( "tss1cguu.4f",  0x1800000, 0x800000, CRC(76924e04) SHA1(751065d6ce658cbbcd88f854f6937ebd2204ec68) )

	ROM_REGION16_LE( 0x400000, "textilemapl", 0 )   /* texture tilemap 0-15 */
	ROM_LOAD( "tss1ccrl.7f",  0x000000, 0x400000, CRC(3a325fe7) SHA1(882735dce7aeb36f9e88a983498360f5de901e9d) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "tss1ccrh.7e",  0x000000, 0x200000, CRC(f998de1a) SHA1(371f540f505608297c5ffcfb623b983ca8310afb) )

	ROM_REGION32_BE( 0x2000000, "pointrom", 0 ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "tss1pt0h.7a",  0x0000000, 0x400000, CRC(cdbe0ba8) SHA1(f8c6da31654c0a2a8024888ffb7fc1c783b2d629) )
	ROM_LOAD32_WORD_SWAP( "tss1pt0l.7c",  0x0000002, 0x400000, CRC(896f0fb4) SHA1(bdfa99eb21ce4fc8021f9d95a5558a34f9942c57) )
	ROM_LOAD32_WORD_SWAP( "tss1pt1h.5a",  0x0800000, 0x400000, CRC(63647596) SHA1(833412be8f61686bd7e06c2738df740e0e585d0f) )
	ROM_LOAD32_WORD_SWAP( "tss1pt1l.5c",  0x0800002, 0x400000, CRC(5a09921f) SHA1(c23885708c7adf0b81c2c9346e21b869634a5b35) )
	ROM_LOAD32_WORD_SWAP( "tss1pt2h.4a",  0x1000000, 0x400000, CRC(9b06e22d) SHA1(cff5ed098112a4f0a2bc8937e226f50066e605b1) )
	ROM_LOAD32_WORD_SWAP( "tss1pt2l.4c",  0x1000002, 0x400000, CRC(4b230d79) SHA1(794cee0a19993e90913f58507c53224f361e9663) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
	ROM_LOAD( "tss1wavel.2c", 0x000000, 0x800000, CRC(deaead26) SHA1(72dac0c3f41d4c3c290f9eb1b50236ae3040a472) )
	ROM_LOAD( "tss1waveh.2a", 0x800000, 0x800000, CRC(5c8758b4) SHA1(b85c8f6869900224ef83a2340b17f5bbb2801af9) )
ROM_END


ROM_START( timecrs2v5a )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "tss5vera.2",   0x000000, 0x200000, CRC(71ef4821) SHA1(03999c2c3219725f8716b7d32efbf810b6401806) )
	ROM_LOAD16_BYTE( "tss5vera.1",   0x000001, 0x200000, CRC(8dfcdd76) SHA1(ba8b2e7070814a225205010ab44ca73628e2913c) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "tss5vera.3",   0x000000, 0x080000, CRC(41e41994) SHA1(eabc1a307c329070bfc6486cb68169c94ff8a162) ) /* Flash ROM type 29F400TC - Common code throughout all versions */

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data ROMs */
	ROM_LOAD16_BYTE( "tss1mtah.2j",  0x0000000, 0x800000, CRC(697c26ed) SHA1(72f6f69e89496ba0c6183b35c3bde71f5a3c721f) )
	ROM_LOAD16_BYTE( "tss1mtal.2h",  0x0000001, 0x800000, CRC(bfc79190) SHA1(04bda00c4cc5660d27af4f3b0ee3550dea8d3805) )
	ROM_LOAD16_BYTE( "tss1mtbh.2m",  0x1000000, 0x800000, CRC(82582776) SHA1(7c790d09bac660ea1c62da3ffb21ab43f2461594) )
	ROM_LOAD16_BYTE( "tss1mtbl.2f",  0x1000001, 0x800000, CRC(e648bea4) SHA1(3803d03e72b25fbcc124d5b25066d25629b76b94) )

	ROM_REGION( 0x2000000, "textile", 0 )   /* texture tiles */
	ROM_LOAD( "tss1cgll.4m",  0x0000000, 0x800000, CRC(18433aaa) SHA1(08539beb2e66ec4e41062621fc098b121c669546) )
	ROM_LOAD( "tss1cglm.4k",  0x0800000, 0x800000, CRC(669974c2) SHA1(cfebe199631e38f547b38fcd35f1645b74e8dd0a) )
	ROM_LOAD( "tss1cgum.4j",  0x1000000, 0x800000, CRC(c22739e1) SHA1(8671ee047bb248033656c50befd1c35e5e478e1a) )
	ROM_LOAD( "tss1cguu.4f",  0x1800000, 0x800000, CRC(76924e04) SHA1(751065d6ce658cbbcd88f854f6937ebd2204ec68) )

	ROM_REGION16_LE( 0x400000, "textilemapl", 0 )   /* texture tilemap 0-15 */
	ROM_LOAD( "tss1ccrl.7f",  0x000000, 0x400000, CRC(3a325fe7) SHA1(882735dce7aeb36f9e88a983498360f5de901e9d) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "tss1ccrh.7e",  0x000000, 0x200000, CRC(f998de1a) SHA1(371f540f505608297c5ffcfb623b983ca8310afb) )

	ROM_REGION32_BE( 0x2000000, "pointrom", 0 ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "tss1pt0h.7a",  0x0000000, 0x400000, CRC(cdbe0ba8) SHA1(f8c6da31654c0a2a8024888ffb7fc1c783b2d629) )
	ROM_LOAD32_WORD_SWAP( "tss1pt0l.7c",  0x0000002, 0x400000, CRC(896f0fb4) SHA1(bdfa99eb21ce4fc8021f9d95a5558a34f9942c57) )
	ROM_LOAD32_WORD_SWAP( "tss1pt1h.5a",  0x0800000, 0x400000, CRC(63647596) SHA1(833412be8f61686bd7e06c2738df740e0e585d0f) )
	ROM_LOAD32_WORD_SWAP( "tss1pt1l.5c",  0x0800002, 0x400000, CRC(5a09921f) SHA1(c23885708c7adf0b81c2c9346e21b869634a5b35) )
	ROM_LOAD32_WORD_SWAP( "tss1pt2h.4a",  0x1000000, 0x400000, CRC(9b06e22d) SHA1(cff5ed098112a4f0a2bc8937e226f50066e605b1) )
	ROM_LOAD32_WORD_SWAP( "tss1pt2l.4c",  0x1000002, 0x400000, CRC(4b230d79) SHA1(794cee0a19993e90913f58507c53224f361e9663) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
	ROM_LOAD( "tss1wavel.2c", 0x000000, 0x800000, CRC(deaead26) SHA1(72dac0c3f41d4c3c290f9eb1b50236ae3040a472) )
	ROM_LOAD( "tss1waveh.2a", 0x800000, 0x800000, CRC(5c8758b4) SHA1(b85c8f6869900224ef83a2340b17f5bbb2801af9) )
ROM_END


ROM_START( aking )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "ag1vera.ic2",   0x000000, 0x200000, CRC(dc98fefb) SHA1(d173c5c6d23f1dae61d448bb6fae27daca525221) )
	ROM_LOAD16_BYTE( "ag1vera.ic1",   0x000001, 0x200000, CRC(f1a08d5c) SHA1(f11bee1093b237067b84ddec8e1bca0b70fc6678) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "ag1vera.ic3",   0x000000, 0x080000, CRC(266ac71c) SHA1(648a64adc0e4a2cefd71c31a6a71359b6c196430) )

	ROM_REGION( 0x40000, "iocpu2", 0 ) // I/O board MB90F574 MCU code
	ROM_LOAD( "fcaf10.bin", 0x000000, 0x040000, NO_DUMP ) // 256KB internal flash ROM

	ROM_REGION( 0x10000, "iocpu3", 0 ) // I/O board PIC16F84 code
	ROM_LOAD( "fcap10.ic2", 0x000000, 0x004010, NO_DUMP )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data ROMs */
	ROM_LOAD16_BYTE( "ag1mtah.2j",  0x0000000, 0x800000, CRC(f2d8ca9d) SHA1(8158d13d74f2aae7c0d1238619ce1ad3a17d8047) )
	ROM_LOAD16_BYTE( "ag1mtal.2h",  0x0000001, 0x800000, CRC(7facbfd4) SHA1(c42988e274a1b4f40f4b4379e94653ef07429c58) )
	ROM_LOAD16_BYTE( "ag1mtbh.2m",  0x1000000, 0x800000, CRC(890bdb52) SHA1(a38f039187448ee328547582eab22813ce625615) )
	ROM_LOAD16_BYTE( "ag1mtbl.2f",  0x1000001, 0x800000, CRC(62d771c9) SHA1(69a47af1366d351157131472756fd05e0fdbf87f) )

	ROM_REGION( 0x2000000, "textile", 0 )   /* texture tiles */
	ROM_LOAD( "ag1cgll.4m",  0x0000000, 0x800000, CRC(9db7e939) SHA1(7be8d6f6d1e236f2655784493bdf4f9869ecd6eb) )
	ROM_LOAD( "ag1cglm.4k",  0x0800000, 0x800000, CRC(17792dba) SHA1(367676870820e44b0092d5ff6d4ee4e80bbf91d2) )
	ROM_LOAD( "ag1cgum.4j",  0x1000000, 0x800000, CRC(5dfa863d) SHA1(a1cde62f00dd8b70538a8eba2aa7ec497cdcaa5c) )
	ROM_LOAD( "ag1cguu.4f",  0x1800000, 0x800000, CRC(86396786) SHA1(d20121eb7d595567cd3438c66ae4c07dbaaaaeb8) )

	ROM_REGION16_LE( 0x400000, "textilemapl", 0 )   /* texture tilemap 0-15 */
	ROM_LOAD( "ag1ccrl.7f",  0x000000, 0x400000, CRC(86bbe1f9) SHA1(3d8484aadc48638ad2b6806118416ac69345e35a) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "ag1ccrh.7e",  0x000000, 0x200000, CRC(abe2aab1) SHA1(b43ddf9b0f4a7ac75dc16fa5b2ed86ac5a273a50) )

	ROM_REGION32_BE( 0x2000000, "pointrom", 0 ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "ag1pt0h.7a",  0x0000000, 0x400000, CRC(b5582ca7) SHA1(0e48b7e3595f9be4e9403a2db939ec140726a880) )
	ROM_LOAD32_WORD_SWAP( "ag1pt0l.7c",  0x0000002, 0x400000, CRC(10e7f54f) SHA1(caf1d28991a9d082b5ddc5def62586b09fa8aff2) )
	ROM_LOAD32_WORD_SWAP( "ag1pt1h.5a",  0x0800000, 0x400000, CRC(25e4776a) SHA1(31e7c9dd3aba01e425839a0ffe1eb0001ac16770) )
	ROM_LOAD32_WORD_SWAP( "ag1pt1l.5c",  0x0800002, 0x400000, CRC(5d3d7099) SHA1(d80d6b692c513945857bcd2c8cfc12b8ec0f3be5) )
	ROM_LOAD32_WORD_SWAP( "ag1pt2h.4a",  0x1000000, 0x400000, CRC(f0eb9012) SHA1(a867f09162e0b0a4eead0bd212df76ba1abb2c19) )
	ROM_LOAD32_WORD_SWAP( "ag1pt2l.4c",  0x1000002, 0x400000, CRC(bf92c054) SHA1(9d676c3bb63bf29d7b18fe5d7e6912a922f06350) )
	ROM_LOAD32_WORD_SWAP( "ag1pt3h.3a",  0x1800000, 0x400000, CRC(e11a12ce) SHA1(4ee78a4d7ada9c26734132baac47b0cbede3d4fd) )
	ROM_LOAD32_WORD_SWAP( "ag1pt3l.3c",  0x1800002, 0x400000, CRC(04b475db) SHA1(3ea28e51185dc2c2bfa50a87031580524eaacc4a) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
	ROM_LOAD( "ag1wavel.2c", 0x000000, 0x800000, CRC(d7fefbd4) SHA1(2cf31661feb6aef40621621897be8e0bc248c1d9) )
	ROM_LOAD( "ag1waveh.2a", 0x800000, 0x800000, CRC(37a61daa) SHA1(34632809f49975d9dc4c76b09ef896df0bc03a52) )
ROM_END


ROM_START( 500gp )
	/* r4650-generic-xrom-generic: NMON 1.0.8-sys23-19990105 P for SYSTEM23 P1 */
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "5gp3verc.2",   0x000000, 0x200000, CRC(e2d43468) SHA1(5e861dd223c7fa177febed9803ac353cba18e19d) )
	ROM_LOAD16_BYTE( "5gp3verc.1",   0x000001, 0x200000, CRC(f6efc94a) SHA1(785eee2bec5080d4e8ef836f28d446328c942b0e) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "5gp3verc.3",   0x000000, 0x080000, CRC(b323abdf) SHA1(8962e39b48a7074a2d492afb5db3f5f3e5ae2389) )

	ROM_REGION( 0x40000, "iocpu2", 0 ) // I/O board MB90F574 MCU code
	ROM_LOAD( "fcaf10.bin", 0x000000, 0x040000, NO_DUMP ) // 256KB internal flash ROM

	ROM_REGION( 0x10000, "iocpu3", 0 ) // I/O board PIC16F84 code
	ROM_LOAD( "fcap10.ic2", 0x000000, 0x004010, NO_DUMP )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data ROMs */
	ROM_LOAD16_BYTE( "5gp1mtah.2j",  0x0000000, 0x800000, CRC(246e4b7a) SHA1(75743294b8f48bffb84f062febfbc02230d49ce9) )
	ROM_LOAD16_BYTE( "5gp1mtal.2h",  0x0000001, 0x800000, CRC(1bb00c7b) SHA1(922be45d57330c31853b2dc1642c589952b09188) )
	ROM_LOAD16_BYTE( "5gp1mtbh.2m",  0x1000000, 0x800000, CRC(352360e8) SHA1(d621dfac3385059c52d215f6623901589a8658a3) )
	ROM_LOAD16_BYTE( "5gp1mtbl.2f",  0x1000001, 0x800000, CRC(66640606) SHA1(c69a0219748241c49315d7464f8156f8068e9cf5) )

	ROM_REGION( 0x2000000, "textile", 0 )   /* texture tiles */
	ROM_LOAD( "5gp1cgll.4m",  0x0000000, 0x800000, CRC(0cc5bf35) SHA1(b75510a94fa6b6d2ed43566e6e84c7ae62f68194) )
	ROM_LOAD( "5gp1cglm.4k",  0x0800000, 0x800000, CRC(31557d48) SHA1(b85c3db20b101ba6bdd77487af67c3324bea29d5) )
	ROM_LOAD( "5gp1cgum.4j",  0x1000000, 0x800000, CRC(0265b701) SHA1(497a4c33311d3bb315100a78400cf2fa726f1483) )
	ROM_LOAD( "5gp1cguu.4f",  0x1800000, 0x800000, CRC(c411163b) SHA1(ae644d62357b8b806b160774043e41908fba5d05) )

	ROM_REGION16_LE( 0x400000, "textilemapl", 0 )   /* texture tilemap 0-15 */
	ROM_LOAD( "5gp1ccrl.7f",  0x000000, 0x400000, CRC(e7c77e1f) SHA1(0231ddbe2afb880099dfe2657c41236c74c730bb) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "5gp1ccrh.7e",  0x000000, 0x200000, CRC(b2eba764) SHA1(5e09d1171f0afdeb9ed7337df1dbc924f23d3a0b) )

	ROM_REGION32_BE( 0x2000000, "pointrom", 0 ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "5gp1pt0h.7a",  0x0000000, 0x400000, CRC(5746a8cd) SHA1(e70fc596ab9360f474f716c73d76cb9851370c76) )
	ROM_LOAD32_WORD_SWAP( "5gp1pt0l.7c",  0x0000002, 0x400000, CRC(a0ece0a1) SHA1(b7aab2d78e1525f865214c7de387ccd585de5d34) )
	ROM_LOAD32_WORD_SWAP( "5gp1pt1h.5a",  0x0800000, 0x400000, CRC(b1feb5df) SHA1(45db259215511ac3e472895956f70204d4575482) )
	ROM_LOAD32_WORD_SWAP( "5gp1pt1l.5c",  0x0800002, 0x400000, CRC(80b25ad2) SHA1(e9a03fe5bb4ce925f7218ab426ed2a1ca1a26a62) )
	ROM_LOAD32_WORD_SWAP( "5gp1pt2h.4a",  0x1000000, 0x400000, CRC(9a693771) SHA1(c988e04cd91c3b7e75b91376fd73be4a7da543e7) )
	ROM_LOAD32_WORD_SWAP( "5gp1pt2l.4c",  0x1000002, 0x400000, CRC(9289dbeb) SHA1(ec546ad3b1c90609591e599c760c70049ba3b581) )
	ROM_LOAD32_WORD_SWAP( "5gp1pt3h.3a",  0x1800000, 0x400000, CRC(26eaa400) SHA1(0157b76fffe81b40eb970e84c98398807ced92c4) )
	ROM_LOAD32_WORD_SWAP( "5gp1pt3l.3c",  0x1800002, 0x400000, CRC(480b120d) SHA1(6c703550faa412095d9633cf508050614e15fbae) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
	ROM_LOAD( "5gp1wavel.2c", 0x000000, 0x800000, CRC(aa634cc2) SHA1(e96f5c682039bc6ef22bf90e98f4da78486bd2b1) )
	ROM_LOAD( "5gp1waveh.2a", 0x800000, 0x800000, CRC(1e3523e8) SHA1(cb3d0d389fcbfb728fad29cfc36ef654d28d553a) )
ROM_END


ROM_START( raceon )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "ro2vera.ic2",  0x000000, 0x200000, CRC(08b94548) SHA1(6363f1724540c2671555bc5bb11e22611614baf5) )
	ROM_LOAD16_BYTE( "ro2vera.ic1",  0x000001, 0x200000, CRC(4270884b) SHA1(82e4d4376907ee5dbabe047b9d2279f08cff5f71) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "ro2vera.ic3",  0x000000, 0x080000, CRC(a763ecb7) SHA1(6b1ab63bb56342abbf7ddd7d17d413779fbafce1) )

	ROM_REGION( 0x80000, "ffb", 0 ) /* STR steering force-feedback board code */
	ROM_LOAD( "ro1_str-0a.ic16", 0x000000, 0x080000, CRC(27d39e1f) SHA1(6161cbb27c964ffab1db3b3c1f073ec514876e61) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data ROMs */
	ROM_LOAD16_BYTE( "ro1mtah.2j",   0x000000, 0x800000, CRC(216abfb1) SHA1(8db7b17dc6441adc7a4ec8b941d5a84d73c735d6) )
	ROM_LOAD16_BYTE( "ro1mtal.2h",   0x000001, 0x800000, CRC(17646306) SHA1(8d1af777f8e884b650efee8e4c26e032e1c088b7) )

	ROM_REGION( 0x2000000, "textile", 0 )   /* texture tiles */
	ROM_LOAD( "ro1cgll.4m",   0x0000000, 0x800000, CRC(12c64936) SHA1(14a0d3d336f2fbe7992eedb3900748763368bc6b) )
	ROM_LOAD( "ro1cglm.4k",   0x0800000, 0x800000, CRC(7e8bb4fc) SHA1(46a7940989576239a720fde8ec4e4b623b0b6fe6) )
	ROM_LOAD( "ro1cgum.4j",   0x1000000, 0x800000, CRC(b9767735) SHA1(87fec452998a782db2cf00d369149b200a00d163) )
	ROM_LOAD( "ro1cguu.4f",   0x1800000, 0x800000, CRC(8fef8bd4) SHA1(6870590f585dc8d87ebe5181da870715c9c4fee3) )

	ROM_REGION16_LE( 0x400000, "textilemapl", 0 )   /* texture tilemap 0-15*/
	ROM_LOAD( "ro1ccrl.7f",   0x000000, 0x400000, CRC(fe50e424) SHA1(8317c998db687e1c40398e0005a037dcded19c25) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "ro1ccrh.7e",   0x000000, 0x200000, CRC(1c958de2) SHA1(4893350999d5d377e68b9577187828de7a4c77c2) )

	ROM_REGION32_LE( 0x2000000, "pointrom", 0 ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "ro1pt0h.7a",   0x0000000, 0x400000, CRC(b052c0dc) SHA1(ed60e2df68315da02a48a701841cd861e8dd46c2) )
	ROM_LOAD32_WORD_SWAP( "ro1pt0l.7c",   0x0000002, 0x400000, CRC(4f09e946) SHA1(4c0e1edb0e807b580849510aa45313ceb700b8b9) )
	ROM_LOAD32_WORD_SWAP( "ro1pt1h.5a",   0x0800000, 0x400000, CRC(94c6be22) SHA1(27fb80975a3332fe2ec91996d2c74b2f4fb5a2de) )
	ROM_LOAD32_WORD_SWAP( "ro1pt1l.5c",   0x0800002, 0x400000, CRC(307b30f2) SHA1(24e0e5d392751e1e4126a604c7a18f90be1e6771) )
	ROM_LOAD32_WORD_SWAP( "ro1pt2h.4a",   0x1000000, 0x400000, CRC(b77886f2) SHA1(efdeb11c9783dbd7eb776c2a99204fb2e55ecb6d) )
	ROM_LOAD32_WORD_SWAP( "ro1pt2l.4c",   0x1000002, 0x400000, CRC(6eaec66b) SHA1(d96d96d3b11a5109fed07e455e2abc9a8450080c) )
	ROM_LOAD32_WORD_SWAP( "ro1pt3h.3a",   0x1800000, 0x400000, CRC(87b8f7c5) SHA1(51f996199f7fc020f12c4ebd3e47acbc04341ce3) )
	ROM_LOAD32_WORD_SWAP( "ro1pt3l.3c",   0x1800002, 0x400000, CRC(ee1a4d1d) SHA1(4fa71edd26edcb3efc56b71f603fabcafcc2d3f3) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
	ROM_LOAD( "ro1wavel.2c",  0x000000, 0x800000, CRC(c6aca840) SHA1(09a021459b6326fe161ffcee36376648a5bf0e00) )
	ROM_LOAD( "ro2waveh.2a",  0x800000, 0x800000, CRC(ceecbf0d) SHA1(f0a5e57c04b661685833b209bd5e072666068391) )

	ROM_REGION( 0x800000, "spares", 0 ) /* duplicate ROMs for the second texel pipeline on the PCB, not used for emulation */
	ROM_LOAD( "ro1ccrl.7m",   0x000000, 0x400000, CRC(fe50e424) SHA1(8317c998db687e1c40398e0005a037dcded19c25) )
	ROM_LOAD( "ro1ccrh.7k",   0x000000, 0x200000, CRC(1c958de2) SHA1(4893350999d5d377e68b9577187828de7a4c77c2) )
	ROM_LOAD( "ro1cgll.5m",   0x000000, 0x800000, CRC(12c64936) SHA1(14a0d3d336f2fbe7992eedb3900748763368bc6b) )
	ROM_LOAD( "ro1cglm.5k",   0x000000, 0x800000, CRC(7e8bb4fc) SHA1(46a7940989576239a720fde8ec4e4b623b0b6fe6) )
	ROM_LOAD( "ro1cgum.5j",   0x000000, 0x800000, CRC(b9767735) SHA1(87fec452998a782db2cf00d369149b200a00d163) )
	ROM_LOAD( "ro1cguu.5f",   0x000000, 0x800000, CRC(8fef8bd4) SHA1(6870590f585dc8d87ebe5181da870715c9c4fee3) )
ROM_END


ROM_START( raceonj )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "ro1verb.ic2",  0x000000, 0x200000, CRC(92952b85) SHA1(4a360479546fccd6932415fb9deae664355e4879) )
	ROM_LOAD16_BYTE( "ro1verb.ic1",  0x000001, 0x200000, CRC(92fb212c) SHA1(8d22c3fcf5e928a8fc1ea97422021e42a5eedc8b) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "ro1verb.ic3",  0x000000, 0x080000, CRC(a763ecb7) SHA1(6b1ab63bb56342abbf7ddd7d17d413779fbafce1) )

	ROM_REGION( 0x80000, "ffb", 0 ) /* STR steering force-feedback board code */
	ROM_LOAD( "ro1_str-0a.ic16", 0x000000, 0x080000, CRC(27d39e1f) SHA1(6161cbb27c964ffab1db3b3c1f073ec514876e61) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data ROMs */
	ROM_LOAD16_BYTE( "ro1mtah.2j",   0x000000, 0x800000, CRC(216abfb1) SHA1(8db7b17dc6441adc7a4ec8b941d5a84d73c735d6) )
	ROM_LOAD16_BYTE( "ro1mtal.2h",   0x000001, 0x800000, CRC(17646306) SHA1(8d1af777f8e884b650efee8e4c26e032e1c088b7) )

	ROM_REGION( 0x2000000, "textile", 0 )   /* texture tiles */
	ROM_LOAD( "ro1cgll.4m",   0x0000000, 0x800000, CRC(12c64936) SHA1(14a0d3d336f2fbe7992eedb3900748763368bc6b) )
	ROM_LOAD( "ro1cglm.4k",   0x0800000, 0x800000, CRC(7e8bb4fc) SHA1(46a7940989576239a720fde8ec4e4b623b0b6fe6) )
	ROM_LOAD( "ro1cgum.4j",   0x1000000, 0x800000, CRC(b9767735) SHA1(87fec452998a782db2cf00d369149b200a00d163) )
	ROM_LOAD( "ro1cguu.4f",   0x1800000, 0x800000, CRC(8fef8bd4) SHA1(6870590f585dc8d87ebe5181da870715c9c4fee3) )

	ROM_REGION16_LE( 0x400000, "textilemapl", 0 )   /* texture tilemap 0-15*/
	ROM_LOAD( "ro1ccrl.7f",   0x000000, 0x400000, CRC(fe50e424) SHA1(8317c998db687e1c40398e0005a037dcded19c25) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "ro1ccrh.7e",   0x000000, 0x200000, CRC(1c958de2) SHA1(4893350999d5d377e68b9577187828de7a4c77c2) )

	ROM_REGION32_LE( 0x2000000, "pointrom", 0 ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "ro1pt0h.7a",   0x0000000, 0x400000, CRC(b052c0dc) SHA1(ed60e2df68315da02a48a701841cd861e8dd46c2) )
	ROM_LOAD32_WORD_SWAP( "ro1pt0l.7c",   0x0000002, 0x400000, CRC(4f09e946) SHA1(4c0e1edb0e807b580849510aa45313ceb700b8b9) )
	ROM_LOAD32_WORD_SWAP( "ro1pt1h.5a",   0x0800000, 0x400000, CRC(94c6be22) SHA1(27fb80975a3332fe2ec91996d2c74b2f4fb5a2de) )
	ROM_LOAD32_WORD_SWAP( "ro1pt1l.5c",   0x0800002, 0x400000, CRC(307b30f2) SHA1(24e0e5d392751e1e4126a604c7a18f90be1e6771) )
	ROM_LOAD32_WORD_SWAP( "ro1pt2h.4a",   0x1000000, 0x400000, CRC(b77886f2) SHA1(efdeb11c9783dbd7eb776c2a99204fb2e55ecb6d) )
	ROM_LOAD32_WORD_SWAP( "ro1pt2l.4c",   0x1000002, 0x400000, CRC(6eaec66b) SHA1(d96d96d3b11a5109fed07e455e2abc9a8450080c) )
	ROM_LOAD32_WORD_SWAP( "ro1pt3h.3a",   0x1800000, 0x400000, CRC(87b8f7c5) SHA1(51f996199f7fc020f12c4ebd3e47acbc04341ce3) )
	ROM_LOAD32_WORD_SWAP( "ro1pt3l.3c",   0x1800002, 0x400000, CRC(ee1a4d1d) SHA1(4fa71edd26edcb3efc56b71f603fabcafcc2d3f3) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
	ROM_LOAD( "ro1wavel.2c",  0x000000, 0x800000, CRC(c6aca840) SHA1(09a021459b6326fe161ffcee36376648a5bf0e00) )
	ROM_LOAD( "ro1waveh.2a",  0x800000, 0x800000, CRC(24ba72e8) SHA1(c9ad8be152918758f3bdbc21c2b83fbf70d16a5d) )

	ROM_REGION( 0x800000, "spares", 0 ) /* duplicate ROMs for the second texel pipeline on the PCB, not used for emulation */
	ROM_LOAD( "ro1ccrl.7m",   0x000000, 0x400000, CRC(fe50e424) SHA1(8317c998db687e1c40398e0005a037dcded19c25) )
	ROM_LOAD( "ro1ccrh.7k",   0x000000, 0x200000, CRC(1c958de2) SHA1(4893350999d5d377e68b9577187828de7a4c77c2) )
	ROM_LOAD( "ro1cgll.5m",   0x000000, 0x800000, CRC(12c64936) SHA1(14a0d3d336f2fbe7992eedb3900748763368bc6b) )
	ROM_LOAD( "ro1cglm.5k",   0x000000, 0x800000, CRC(7e8bb4fc) SHA1(46a7940989576239a720fde8ec4e4b623b0b6fe6) )
	ROM_LOAD( "ro1cgum.5j",   0x000000, 0x800000, CRC(b9767735) SHA1(87fec452998a782db2cf00d369149b200a00d163) )
	ROM_LOAD( "ro1cguu.5f",   0x000000, 0x800000, CRC(8fef8bd4) SHA1(6870590f585dc8d87ebe5181da870715c9c4fee3) )
ROM_END


ROM_START( finfurl2 )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "29f016.ic2",   0x000000, 0x200000, CRC(13cbc545) SHA1(3e67a7bfbb1c1374e8e3996a0c09e4861b0dca14) )
	ROM_LOAD16_BYTE( "29f016.ic1",   0x000001, 0x200000, CRC(5b04e4f2) SHA1(8099fc3deab9ed14a2484a774666fbd928330de8) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "ffs1vera.ic3",  0x000000, 0x080000, CRC(9fd69bbd) SHA1(53a9bf505de70495dcccc43fdc722b3381aad97c) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data ROMs */
	ROM_LOAD16_BYTE( "ffs1mtah.2j",  0x0000000, 0x800000, CRC(f336d81d) SHA1(a9177091e1412dea1b6ea6c53530ae31361b32d0) )
	ROM_LOAD16_BYTE( "ffs1mtal.2h",  0x0000001, 0x800000, CRC(98730ad5) SHA1(9ba276ad88ec8730edbacab80cdacc34a99593e4) )
	ROM_LOAD16_BYTE( "ffs1mtbh.2m",  0x1000000, 0x800000, CRC(0f42c93b) SHA1(26b313fc5c33afb0a1ee42243486e38f052c95c2) )
	ROM_LOAD16_BYTE( "ffs1mtbl.2f",  0x1000001, 0x800000, CRC(0abc9e50) SHA1(be5e5e2b637811c59804ef9442c6da5a5a1315e2) )

	ROM_REGION( 0x2000000, "textile", 0 )   /* texture tiles */
	ROM_LOAD( "ffs1cgll.4m",  0x0000000, 0x800000, CRC(171bba76) SHA1(4a63a1f34de8f341a0ef9b499a21e8fec758e1cd) )
	ROM_LOAD( "ffs1cglm.4k",  0x0800000, 0x800000, CRC(48acf207) SHA1(ea902efdd94aba34dadb20762219d2d25441d199) )
	ROM_LOAD( "ffs1cgum.4j",  0x1000000, 0x800000, CRC(77447199) SHA1(1eeae30b3dd1ac467bdbbdfe4be36ca0f0816496) )
	ROM_LOAD( "ffs1cguu.4f",  0x1800000, 0x800000, CRC(52c0a19f) SHA1(e6b4b90ff88da09cb2e653e450e7ae66942a719e) )

	ROM_REGION16_LE( 0x200000, "textilemapl", 0 )   /* texture tilemap 0-15*/
	ROM_LOAD( "ffs1ccrl.7f",  0x000000, 0x200000, CRC(ffbcfec1) SHA1(9ab25f1543da4b72784eec93985abaa2e1dafc83) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "ffs1ccrh.7e",  0x000000, 0x200000, CRC(8be4aeb4) SHA1(ec344f6fba42092083e737e436451f5d7be12c15) )

	ROM_REGION32_LE( 0x2000000, "pointrom", 0 ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "ffs1pt0h.7a",  0x0000000, 0x400000, CRC(6f9da966) SHA1(97ed7eba833d6c6065bb8cbb4673dd09abe20b21) )
	ROM_LOAD32_WORD_SWAP( "ffs1pt0l.7c",  0x0000002, 0x400000, CRC(63d80064) SHA1(d4c08fc0baca9b1d9f1203673acb6830a2d5e4b0) )
	ROM_LOAD32_WORD_SWAP( "ffs1pt1h.5a",  0x0800000, 0x400000, CRC(180edc67) SHA1(7c8e0266cf3481eea5416100a913eda046d30185) )
	ROM_LOAD32_WORD_SWAP( "ffs1pt1l.5c",  0x0800002, 0x400000, CRC(21948286) SHA1(e4185eeb78195f2f691bad7c54b30a40718bf3b2) )
	ROM_LOAD32_WORD_SWAP( "ffs1pt2h.4a",  0x1000000, 0x400000, CRC(f5664030) SHA1(0444ec9780417cff35404b961a75a49461fc1c50) )
	ROM_LOAD32_WORD_SWAP( "ffs1pt2l.4c",  0x1000002, 0x400000, CRC(8cf96442) SHA1(1ccbe5cf700a63adc06e8c6afe4527d27942207d) )
	ROM_LOAD32_WORD_SWAP( "ffs1pt3h.3a",  0x1800000, 0x400000, CRC(81c675ee) SHA1(4417862f481ace6152762634a3a012161829501e) )
	ROM_LOAD32_WORD_SWAP( "ffs1pt3l.3c",  0x1800002, 0x400000, CRC(a3541604) SHA1(dffec3756051773bd1110d98463eb3282834b00c) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
	ROM_LOAD( "ffs1wavel.2c", 0x000000, 0x800000, CRC(67ba16cf) SHA1(00b38617c2185b9a3bf279962ad0c21a7287256f) )
	ROM_LOAD( "ffs1waveh.2a", 0x800000, 0x800000, CRC(178e8bd3) SHA1(8ab1a97003914f70b09e96c5924f3a839fe634c7) )
ROM_END


ROM_START( finfurl2j )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "ffs1vera.ic2", 0x000000, 0x200000, CRC(0215125d) SHA1(a99f601441c152b0b00f4811e5752c71897b1ed4) )
	ROM_LOAD16_BYTE( "ffs1vera.ic1", 0x000001, 0x200000, CRC(38c9ae96) SHA1(b50afc7276662267ff6460f82d0e5e8b00b341ea) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "ffs1vera.ic3",  0x000000, 0x080000, CRC(9fd69bbd) SHA1(53a9bf505de70495dcccc43fdc722b3381aad97c) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data ROMs */
	ROM_LOAD16_BYTE( "ffs1mtah.2j",  0x0000000, 0x800000, CRC(f336d81d) SHA1(a9177091e1412dea1b6ea6c53530ae31361b32d0) )
	ROM_LOAD16_BYTE( "ffs1mtal.2h",  0x0000001, 0x800000, CRC(98730ad5) SHA1(9ba276ad88ec8730edbacab80cdacc34a99593e4) )
	ROM_LOAD16_BYTE( "ffs1mtbh.2m",  0x1000000, 0x800000, CRC(0f42c93b) SHA1(26b313fc5c33afb0a1ee42243486e38f052c95c2) )
	ROM_LOAD16_BYTE( "ffs1mtbl.2f",  0x1000001, 0x800000, CRC(0abc9e50) SHA1(be5e5e2b637811c59804ef9442c6da5a5a1315e2) )

	ROM_REGION( 0x2000000, "textile", 0 )   /* texture tiles */
	ROM_LOAD( "ffs1cgll.4m",  0x0000000, 0x800000, CRC(171bba76) SHA1(4a63a1f34de8f341a0ef9b499a21e8fec758e1cd) )
	ROM_LOAD( "ffs1cglm.4k",  0x0800000, 0x800000, CRC(48acf207) SHA1(ea902efdd94aba34dadb20762219d2d25441d199) )
	ROM_LOAD( "ffs1cgum.4j",  0x1000000, 0x800000, CRC(77447199) SHA1(1eeae30b3dd1ac467bdbbdfe4be36ca0f0816496) )
	ROM_LOAD( "ffs1cguu.4f",  0x1800000, 0x800000, CRC(52c0a19f) SHA1(e6b4b90ff88da09cb2e653e450e7ae66942a719e) )

	ROM_REGION16_LE( 0x200000, "textilemapl", 0 )   /* texture tilemap 0-15*/
	ROM_LOAD( "ffs1ccrl.7f",  0x000000, 0x200000, CRC(ffbcfec1) SHA1(9ab25f1543da4b72784eec93985abaa2e1dafc83) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "ffs1ccrh.7e",  0x000000, 0x200000, CRC(8be4aeb4) SHA1(ec344f6fba42092083e737e436451f5d7be12c15) )

	ROM_REGION32_LE( 0x2000000, "pointrom", 0 ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "ffs1pt0h.7a",  0x0000000, 0x400000, CRC(6f9da966) SHA1(97ed7eba833d6c6065bb8cbb4673dd09abe20b21) )
	ROM_LOAD32_WORD_SWAP( "ffs1pt0l.7c",  0x0000002, 0x400000, CRC(63d80064) SHA1(d4c08fc0baca9b1d9f1203673acb6830a2d5e4b0) )
	ROM_LOAD32_WORD_SWAP( "ffs1pt1h.5a",  0x0800000, 0x400000, CRC(180edc67) SHA1(7c8e0266cf3481eea5416100a913eda046d30185) )
	ROM_LOAD32_WORD_SWAP( "ffs1pt1l.5c",  0x0800002, 0x400000, CRC(21948286) SHA1(e4185eeb78195f2f691bad7c54b30a40718bf3b2) )
	ROM_LOAD32_WORD_SWAP( "ffs1pt2h.4a",  0x1000000, 0x400000, CRC(f5664030) SHA1(0444ec9780417cff35404b961a75a49461fc1c50) )
	ROM_LOAD32_WORD_SWAP( "ffs1pt2l.4c",  0x1000002, 0x400000, CRC(8cf96442) SHA1(1ccbe5cf700a63adc06e8c6afe4527d27942207d) )
	ROM_LOAD32_WORD_SWAP( "ffs1pt3h.3a",  0x1800000, 0x400000, CRC(81c675ee) SHA1(4417862f481ace6152762634a3a012161829501e) )
	ROM_LOAD32_WORD_SWAP( "ffs1pt3l.3c",  0x1800002, 0x400000, CRC(a3541604) SHA1(dffec3756051773bd1110d98463eb3282834b00c) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
	ROM_LOAD( "ffs1wavel.2c", 0x000000, 0x800000, CRC(67ba16cf) SHA1(00b38617c2185b9a3bf279962ad0c21a7287256f) )
	ROM_LOAD( "ffs1waveh.2a", 0x800000, 0x800000, CRC(178e8bd3) SHA1(8ab1a97003914f70b09e96c5924f3a839fe634c7) )
ROM_END


ROM_START( panicprk )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "pnp2vera.ic2", 0x000000, 0x200000, CRC(cd528597) SHA1(cf390e78228eb10d5f50ff7e7e37063a2d87f469) )
	ROM_LOAD16_BYTE( "pnp2vera.ic1", 0x000001, 0x200000, CRC(80fea853) SHA1(b18003bde060ebb3c892a6d7fa4abf868cadc777) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "pnp1vera.ic3", 0x000000, 0x080000, CRC(fe4bc6f4) SHA1(2114dc4bc63d589e6c3b26a73dbc60924f3b1765) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data ROMs */
	ROM_LOAD16_BYTE( "pnp1mtah.2j",  0x000000, 0x800000, CRC(37addddd) SHA1(3032989653304417df80606bc3fde6e9425d8cbb) )
	ROM_LOAD16_BYTE( "pnp1mtal.2h",  0x000001, 0x800000, CRC(6490faaa) SHA1(03443746009b434e5d4074ea6314910418907360) )

	ROM_REGION( 0x2000000, "textile", 0 )   /* texture tiles */
	ROM_LOAD( "pnp1cgll.4m",  0x0000000, 0x800000, CRC(d03932cf) SHA1(49240e44923cc6e815e9457b6290fd18466658af) )
	ROM_LOAD( "pnp1cglm.5k",  0x0800000, 0x800000, CRC(abf4ccf2) SHA1(3848e26d0ba6c872bbc6d5e0eb23a9d4b34152d5) )
	ROM_LOAD( "pnp1cgum.4j",  0x1000000, 0x800000, CRC(206217ca) SHA1(9c095bba7764f3405c3fab10513b9b78981ec44d) )
	ROM_LOAD( "pnp1cguu.5f",  0x1800000, 0x800000, CRC(cd64f57f) SHA1(8780270298e0823db1acbbf79396788df0c3c19c) )

	ROM_REGION16_LE( 0x200000, "textilemapl", 0 )   /* texture tilemap 0-15 */
	ROM_LOAD( "pnp1ccrl.7f",  0x000000, 0x200000, CRC(b7bc43c2) SHA1(f4b470540194486ca6822f438fc1d4700cfb2ab1) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "pnp1ccrh.7e",  0x000000, 0x200000, CRC(caaf1b73) SHA1(b436992817ab4e4dad05e7429eb102d4fb57fa6a) )

	ROM_REGION32_BE( 0x2000000, "pointrom", 0 ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "pnp1pt0h.7a",  0x000000, 0x400000, CRC(43fc2246) SHA1(301d321cd4a01ebd7ccfa6f295d6c3daf0a19efe) )
	ROM_LOAD32_WORD_SWAP( "pnp1pt0l.7c",  0x000002, 0x400000, CRC(26af5fa1) SHA1(12fcf98c2a59643e0fdfdd7186f9f16baf54a9cf) )
	ROM_LOAD32_WORD_SWAP( "pnp1pt1h.5a",  0x800000, 0x400000, CRC(1ff470c0) SHA1(ca8fad90743589744939d681b0ce94f368337b3f) )
	ROM_LOAD32_WORD_SWAP( "pnp1pt1l.5c",  0x800002, 0x400000, CRC(15c6f236) SHA1(e8c393359a91cdce6e9110a48c0a80708f8fc132) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
	ROM_LOAD( "pnp1wavel.2c", 0x000000, 0x800000, CRC(35c6a9bd) SHA1(4b56fdc37525c15e57d93091e6609d6a6905fc5c) )
	ROM_LOAD( "pnp1waveh.2a", 0x800000, 0x800000, CRC(6fa1826a) SHA1(20a5af49e65ae2bc57c016b5cd9bafa5a5220d35) )

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate ROMs */
	ROM_LOAD( "pnp1cguu.4f",  0x000000, 0x800000, CRC(cd64f57f) SHA1(8780270298e0823db1acbbf79396788df0c3c19c) )
	ROM_LOAD( "pnp1cgum.5j",  0x000000, 0x800000, CRC(206217ca) SHA1(9c095bba7764f3405c3fab10513b9b78981ec44d) )
	ROM_LOAD( "pnp1cgll.5m",  0x000000, 0x800000, CRC(d03932cf) SHA1(49240e44923cc6e815e9457b6290fd18466658af) )
	ROM_LOAD( "pnp1cglm.4k",  0x000000, 0x800000, CRC(abf4ccf2) SHA1(3848e26d0ba6c872bbc6d5e0eb23a9d4b34152d5) )
	ROM_LOAD( "pnp1ccrl.7m",  0x000000, 0x200000, CRC(b7bc43c2) SHA1(f4b470540194486ca6822f438fc1d4700cfb2ab1) )
	ROM_LOAD( "pnp1ccrh.7k",  0x000000, 0x200000, CRC(caaf1b73) SHA1(b436992817ab4e4dad05e7429eb102d4fb57fa6a) )

	ROM_REGION( 0x010000, "nvram", 0 )
	ROM_LOAD( "nvram",        0x000000, 0x010000, CRC(10fb7b12) SHA1(32b12dfaa52a1c4b2df2002489b7b4040f361a6c) )
ROM_END


ROM_START( panicprkj )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "pnp1verb.ic2", 0x000000, 0x200000, CRC(a46e34f8) SHA1(c84eb701a1e01e706dea515acaaf6d98ad53f453) )
	ROM_LOAD16_BYTE( "pnp1verb.ic1", 0x000001, 0x200000, CRC(4de52a64) SHA1(7edd974d52e17bcbdb029f718c06d603ed558d90) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "pnp1vera.ic3", 0x000000, 0x080000, CRC(fe4bc6f4) SHA1(2114dc4bc63d589e6c3b26a73dbc60924f3b1765) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data ROMs */
	ROM_LOAD16_BYTE( "pnp1mtah.2j",  0x000000, 0x800000, CRC(37addddd) SHA1(3032989653304417df80606bc3fde6e9425d8cbb) )
	ROM_LOAD16_BYTE( "pnp1mtal.2h",  0x000001, 0x800000, CRC(6490faaa) SHA1(03443746009b434e5d4074ea6314910418907360) )

	ROM_REGION( 0x2000000, "textile", 0 )   /* texture tiles */
	ROM_LOAD( "pnp1cgll.4m",  0x0000000, 0x800000, CRC(d03932cf) SHA1(49240e44923cc6e815e9457b6290fd18466658af) )
	ROM_LOAD( "pnp1cglm.5k",  0x0800000, 0x800000, CRC(abf4ccf2) SHA1(3848e26d0ba6c872bbc6d5e0eb23a9d4b34152d5) )
	ROM_LOAD( "pnp1cgum.4j",  0x1000000, 0x800000, CRC(206217ca) SHA1(9c095bba7764f3405c3fab10513b9b78981ec44d) )
	ROM_LOAD( "pnp1cguu.5f",  0x1800000, 0x800000, CRC(cd64f57f) SHA1(8780270298e0823db1acbbf79396788df0c3c19c) )

	ROM_REGION16_LE( 0x200000, "textilemapl", 0 )   /* texture tilemap 0-15 */
	ROM_LOAD( "pnp1ccrl.7f",  0x000000, 0x200000, CRC(b7bc43c2) SHA1(f4b470540194486ca6822f438fc1d4700cfb2ab1) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "pnp1ccrh.7e",  0x000000, 0x200000, CRC(caaf1b73) SHA1(b436992817ab4e4dad05e7429eb102d4fb57fa6a) )

	ROM_REGION32_BE( 0x2000000, "pointrom", 0 ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "pnp1pt0h.7a",  0x000000, 0x400000, CRC(43fc2246) SHA1(301d321cd4a01ebd7ccfa6f295d6c3daf0a19efe) )
	ROM_LOAD32_WORD_SWAP( "pnp1pt0l.7c",  0x000002, 0x400000, CRC(26af5fa1) SHA1(12fcf98c2a59643e0fdfdd7186f9f16baf54a9cf) )
	ROM_LOAD32_WORD_SWAP( "pnp1pt1h.5a",  0x800000, 0x400000, CRC(1ff470c0) SHA1(ca8fad90743589744939d681b0ce94f368337b3f) )
	ROM_LOAD32_WORD_SWAP( "pnp1pt1l.5c",  0x800002, 0x400000, CRC(15c6f236) SHA1(e8c393359a91cdce6e9110a48c0a80708f8fc132) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
	ROM_LOAD( "pnp1wavel.2c", 0x000000, 0x800000, CRC(35c6a9bd) SHA1(4b56fdc37525c15e57d93091e6609d6a6905fc5c) )
	ROM_LOAD( "pnp1waveh.2a", 0x800000, 0x800000, CRC(6fa1826a) SHA1(20a5af49e65ae2bc57c016b5cd9bafa5a5220d35) )

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate ROMs */
	ROM_LOAD( "pnp1cguu.4f",  0x000000, 0x800000, CRC(cd64f57f) SHA1(8780270298e0823db1acbbf79396788df0c3c19c) )
	ROM_LOAD( "pnp1cgum.5j",  0x000000, 0x800000, CRC(206217ca) SHA1(9c095bba7764f3405c3fab10513b9b78981ec44d) )
	ROM_LOAD( "pnp1cgll.5m",  0x000000, 0x800000, CRC(d03932cf) SHA1(49240e44923cc6e815e9457b6290fd18466658af) )
	ROM_LOAD( "pnp1cglm.4k",  0x000000, 0x800000, CRC(abf4ccf2) SHA1(3848e26d0ba6c872bbc6d5e0eb23a9d4b34152d5) )
	ROM_LOAD( "pnp1ccrl.7m",  0x000000, 0x200000, CRC(b7bc43c2) SHA1(f4b470540194486ca6822f438fc1d4700cfb2ab1) )
	ROM_LOAD( "pnp1ccrh.7k",  0x000000, 0x200000, CRC(caaf1b73) SHA1(b436992817ab4e4dad05e7429eb102d4fb57fa6a) )

	ROM_REGION( 0x010000, "nvram", 0 )
	ROM_LOAD( "nvram",        0x000000, 0x010000, CRC(22362b2a) SHA1(144956188e3cba78015e78cdcb52142b72a5aa1e) )
ROM_END


ROM_START( panicprkj2 )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) // 4 megs for main R4650 code
	ROM_LOAD16_BYTE( "pnp1verb.ic2", 0x000000, 0x200000, CRC(59748d7d) SHA1(f7677ec9027439a7352c88494873861e78ae9a41) )
	ROM_LOAD16_BYTE( "pnp1verb.ic1", 0x000001, 0x200000, CRC(718ff346) SHA1(0f631fe32e864623455d9bc39cdfa80b34c6dbb0) )

	ROM_REGION( 0x80000, "subcpu", 0 )  // Hitachi H8/3002 MCU code
	ROM_LOAD16_WORD_SWAP( "pnp1vera.ic3", 0x000000, 0x080000, CRC(fe4bc6f4) SHA1(2114dc4bc63d589e6c3b26a73dbc60924f3b1765) )

	ROM_REGION32_BE( 0x2000000, "data", 0 )
	ROM_LOAD16_BYTE( "pnp1mtah.2j",  0x000000, 0x800000, CRC(37addddd) SHA1(3032989653304417df80606bc3fde6e9425d8cbb) )
	ROM_LOAD16_BYTE( "pnp1mtal.2h",  0x000001, 0x800000, CRC(6490faaa) SHA1(03443746009b434e5d4074ea6314910418907360) )

	ROM_REGION( 0x2000000, "textile", 0 )
	ROM_LOAD( "pnp1cgll.4m",  0x0000000, 0x800000, CRC(d03932cf) SHA1(49240e44923cc6e815e9457b6290fd18466658af) )
	ROM_LOAD( "pnp1cglm.5k",  0x0800000, 0x800000, CRC(abf4ccf2) SHA1(3848e26d0ba6c872bbc6d5e0eb23a9d4b34152d5) )
	ROM_LOAD( "pnp1cgum.4j",  0x1000000, 0x800000, CRC(206217ca) SHA1(9c095bba7764f3405c3fab10513b9b78981ec44d) )
	ROM_LOAD( "pnp1cguu.5f",  0x1800000, 0x800000, CRC(cd64f57f) SHA1(8780270298e0823db1acbbf79396788df0c3c19c) )

	ROM_REGION16_LE( 0x200000, "textilemapl", 0 )
	ROM_LOAD( "pnp1ccrl.7f",  0x000000, 0x200000, CRC(b7bc43c2) SHA1(f4b470540194486ca6822f438fc1d4700cfb2ab1) )

	ROM_REGION( 0x200000, "textilemaph", 0 )
	ROM_LOAD( "pnp1ccrh.7e",  0x000000, 0x200000, CRC(caaf1b73) SHA1(b436992817ab4e4dad05e7429eb102d4fb57fa6a) )

	ROM_REGION32_BE( 0x2000000, "pointrom", 0 )
	ROM_LOAD32_WORD_SWAP( "pnp1pt0h.7a",  0x000000, 0x400000, CRC(43fc2246) SHA1(301d321cd4a01ebd7ccfa6f295d6c3daf0a19efe) )
	ROM_LOAD32_WORD_SWAP( "pnp1pt0l.7c",  0x000002, 0x400000, CRC(26af5fa1) SHA1(12fcf98c2a59643e0fdfdd7186f9f16baf54a9cf) )
	ROM_LOAD32_WORD_SWAP( "pnp1pt1h.5a",  0x800000, 0x400000, CRC(1ff470c0) SHA1(ca8fad90743589744939d681b0ce94f368337b3f) )
	ROM_LOAD32_WORD_SWAP( "pnp1pt1l.5c",  0x800002, 0x400000, CRC(15c6f236) SHA1(e8c393359a91cdce6e9110a48c0a80708f8fc132) )

	ROM_REGION( 0x1000000, "c352", 0 )
	ROM_LOAD( "pnp1wavel.2c", 0x000000, 0x800000, CRC(35c6a9bd) SHA1(4b56fdc37525c15e57d93091e6609d6a6905fc5c) )
	ROM_LOAD( "pnp1waveh.2a", 0x800000, 0x800000, CRC(6fa1826a) SHA1(20a5af49e65ae2bc57c016b5cd9bafa5a5220d35) )

	ROM_REGION( 0x800000, "dups", 0 )   // duplicate ROMs
	ROM_LOAD( "pnp1cguu.4f",  0x000000, 0x800000, CRC(cd64f57f) SHA1(8780270298e0823db1acbbf79396788df0c3c19c) )
	ROM_LOAD( "pnp1cgum.5j",  0x000000, 0x800000, CRC(206217ca) SHA1(9c095bba7764f3405c3fab10513b9b78981ec44d) )
	ROM_LOAD( "pnp1cgll.5m",  0x000000, 0x800000, CRC(d03932cf) SHA1(49240e44923cc6e815e9457b6290fd18466658af) )
	ROM_LOAD( "pnp1cglm.4k",  0x000000, 0x800000, CRC(abf4ccf2) SHA1(3848e26d0ba6c872bbc6d5e0eb23a9d4b34152d5) )
	ROM_LOAD( "pnp1ccrl.7m",  0x000000, 0x200000, CRC(b7bc43c2) SHA1(f4b470540194486ca6822f438fc1d4700cfb2ab1) )
	ROM_LOAD( "pnp1ccrh.7k",  0x000000, 0x200000, CRC(caaf1b73) SHA1(b436992817ab4e4dad05e7429eb102d4fb57fa6a) )

	ROM_REGION( 0x010000, "nvram", 0 )
	ROM_LOAD( "nvram",        0x000000, 0x010000, CRC(e49584d5) SHA1(2f49a3d83fcfb929edb29df1f4feff3aa582c3f8) )
ROM_END


ROM_START( gunwars )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "gm1verb.ic2",  0x000000, 0x200000, CRC(401f8264) SHA1(281f245ae0fbc2b82248c7aacaa5dfcdb114e2ee) )
	ROM_LOAD16_BYTE( "gm1verb.ic1",  0x000001, 0x200000, CRC(f9fd0f2b) SHA1(53dadd49d0d43f0693c84853ba3de1b5faa9e1d8) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "gm1vera.ic3",  0x000000, 0x080000, CRC(5582fdd4) SHA1(8aae8bc6688d531888f2de509c07502ee355b3ab) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data ROMs */
	ROM_LOAD16_BYTE( "gm1mtah.2j",   0x000000, 0x800000, CRC(3cea9094) SHA1(497395425e409de47e1114de9aeeaf05e4f6a9a1) )
	ROM_LOAD16_BYTE( "gm1mtal.2h",   0x000001, 0x800000, CRC(d531dfcd) SHA1(9f7cbe9a03c1f7649bf05a7a30d47511573b50ba) )

	ROM_REGION( 0x2000000, "textile", 0 )   /* texture tiles */
	ROM_LOAD( "gm1cgll.4m",   0x0000000, 0x800000, CRC(936c0079) SHA1(3aec8caada35b7ed790bb3a8bcf6e01cad068fcd) )
	ROM_LOAD( "gm1cglm.4k",   0x0800000, 0x800000, CRC(e2ee5493) SHA1(1ffd74646796ad554d7967ba9fc18deab4fedadf) )
	ROM_LOAD( "gm1cgum.4j",   0x1000000, 0x800000, CRC(a7728944) SHA1(c187c6d66128554fcecc96e81d4f5396197e8280) )
	ROM_LOAD( "gm1cguu.5f",   0x1800000, 0x800000, CRC(26a74698) SHA1(3f07d273abb3f2552dc6a29300f5dc2f2744c852) )

	ROM_REGION16_LE( 0x400000, "textilemapl", 0 )   /* texture tilemap 0-15 */
	ROM_LOAD( "gm1ccrl.7f",   0x000000, 0x400000, CRC(2c54c182) SHA1(538dfb04653f8d86f976e702456bf4da97e3fda9) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "gm1ccrh.7e",   0x000000, 0x200000, CRC(8563ef01) SHA1(59f09a08008a71a4bb12bd43a1b5dbe633d3061d) )

	ROM_REGION32_BE( 0x2000000, "pointrom", 0 ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "gm1pt0h.7a",   0x000000, 0x400000, CRC(5ebd658c) SHA1(9e7b89a726b11b6da3327d72ec6adcc30fbb384d) )
	ROM_LOAD32_WORD_SWAP( "gm1pt0l.7c",   0x000002, 0x400000, CRC(62e9bedb) SHA1(7043c5e6f26139c9e6e18d4f35fac6a16d4dabd1) )
	ROM_LOAD32_WORD_SWAP( "gm1pt1h.5a",   0x800000, 0x400000, CRC(5f6cebab) SHA1(95bd30d30ea25509b66a107fb255d0af1e6a357e) )
	ROM_LOAD32_WORD_SWAP( "gm1pt1l.5c",   0x800002, 0x400000, CRC(f44c149f) SHA1(9f995de02ea6ac35ccbabbba5bb473a10e1ec667) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
	ROM_LOAD( "gm1wave.2c",   0x000000, 0x800000, CRC(7d5c79a4) SHA1(b800a46bcca10cb0d0d9e0acfa68af63ae64dcaf) )

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate ROMs */
	ROM_LOAD( "gm1cguu.4f",   0x000000, 0x800000, CRC(26a74698) SHA1(3f07d273abb3f2552dc6a29300f5dc2f2744c852) )
	ROM_LOAD( "gm1cgum.5j",   0x000000, 0x800000, CRC(a7728944) SHA1(c187c6d66128554fcecc96e81d4f5396197e8280) )
	ROM_LOAD( "gm1cgll.5m",   0x000000, 0x800000, CRC(936c0079) SHA1(3aec8caada35b7ed790bb3a8bcf6e01cad068fcd) )
	ROM_LOAD( "gm1cglm.5k",   0x000000, 0x800000, CRC(e2ee5493) SHA1(1ffd74646796ad554d7967ba9fc18deab4fedadf) )
	ROM_LOAD( "gm1ccrl.7m",   0x000000, 0x400000, CRC(2c54c182) SHA1(538dfb04653f8d86f976e702456bf4da97e3fda9) )
	ROM_LOAD( "gm1ccrh.7k",   0x000000, 0x200000, CRC(8563ef01) SHA1(59f09a08008a71a4bb12bd43a1b5dbe633d3061d) )
ROM_END


ROM_START( gunwarsa )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "gm1vera.ic2",  0x000000, 0x200000, CRC(cf61467f) SHA1(eae79e4e540340cba7d576a36085f802b8032f4f) )
	ROM_LOAD16_BYTE( "gm1vera.ic1",  0x000001, 0x200000, CRC(abc9ffe6) SHA1(d833b9b9d8bb0cc4b53f30507c9603df9e63fa2f) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "gm1vera.ic3",  0x000000, 0x080000, CRC(5582fdd4) SHA1(8aae8bc6688d531888f2de509c07502ee355b3ab) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data ROMs */
	ROM_LOAD16_BYTE( "gm1mtah.2j",   0x000000, 0x800000, CRC(3cea9094) SHA1(497395425e409de47e1114de9aeeaf05e4f6a9a1) )
	ROM_LOAD16_BYTE( "gm1mtal.2h",   0x000001, 0x800000, CRC(d531dfcd) SHA1(9f7cbe9a03c1f7649bf05a7a30d47511573b50ba) )

	ROM_REGION( 0x2000000, "textile", 0 )   /* texture tiles */
	ROM_LOAD( "gm1cgll.4m",   0x0000000, 0x800000, CRC(936c0079) SHA1(3aec8caada35b7ed790bb3a8bcf6e01cad068fcd) )
	ROM_LOAD( "gm1cglm.4k",   0x0800000, 0x800000, CRC(e2ee5493) SHA1(1ffd74646796ad554d7967ba9fc18deab4fedadf) )
	ROM_LOAD( "gm1cgum.4j",   0x1000000, 0x800000, CRC(a7728944) SHA1(c187c6d66128554fcecc96e81d4f5396197e8280) )
	ROM_LOAD( "gm1cguu.5f",   0x1800000, 0x800000, CRC(26a74698) SHA1(3f07d273abb3f2552dc6a29300f5dc2f2744c852) )

	ROM_REGION16_LE( 0x400000, "textilemapl", 0 )   /* texture tilemap 0-15 */
	ROM_LOAD( "gm1ccrl.7f",   0x000000, 0x400000, CRC(2c54c182) SHA1(538dfb04653f8d86f976e702456bf4da97e3fda9) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "gm1ccrh.7e",   0x000000, 0x200000, CRC(8563ef01) SHA1(59f09a08008a71a4bb12bd43a1b5dbe633d3061d) )

	ROM_REGION32_BE( 0x2000000, "pointrom", 0 ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "gm1pt0h.7a",   0x000000, 0x400000, CRC(5ebd658c) SHA1(9e7b89a726b11b6da3327d72ec6adcc30fbb384d) )
	ROM_LOAD32_WORD_SWAP( "gm1pt0l.7c",   0x000002, 0x400000, CRC(62e9bedb) SHA1(7043c5e6f26139c9e6e18d4f35fac6a16d4dabd1) )
	ROM_LOAD32_WORD_SWAP( "gm1pt1h.5a",   0x800000, 0x400000, CRC(5f6cebab) SHA1(95bd30d30ea25509b66a107fb255d0af1e6a357e) )
	ROM_LOAD32_WORD_SWAP( "gm1pt1l.5c",   0x800002, 0x400000, CRC(f44c149f) SHA1(9f995de02ea6ac35ccbabbba5bb473a10e1ec667) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
	ROM_LOAD( "gm1wave.2c",   0x000000, 0x800000, CRC(7d5c79a4) SHA1(b800a46bcca10cb0d0d9e0acfa68af63ae64dcaf) )

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate ROMs */
	ROM_LOAD( "gm1cguu.4f",   0x000000, 0x800000, CRC(26a74698) SHA1(3f07d273abb3f2552dc6a29300f5dc2f2744c852) )
	ROM_LOAD( "gm1cgum.5j",   0x000000, 0x800000, CRC(a7728944) SHA1(c187c6d66128554fcecc96e81d4f5396197e8280) )
	ROM_LOAD( "gm1cgll.5m",   0x000000, 0x800000, CRC(936c0079) SHA1(3aec8caada35b7ed790bb3a8bcf6e01cad068fcd) )
	ROM_LOAD( "gm1cglm.5k",   0x000000, 0x800000, CRC(e2ee5493) SHA1(1ffd74646796ad554d7967ba9fc18deab4fedadf) )
	ROM_LOAD( "gm1ccrl.7m",   0x000000, 0x400000, CRC(2c54c182) SHA1(538dfb04653f8d86f976e702456bf4da97e3fda9) )
	ROM_LOAD( "gm1ccrh.7k",   0x000000, 0x200000, CRC(8563ef01) SHA1(59f09a08008a71a4bb12bd43a1b5dbe633d3061d) )
ROM_END


ROM_START( downhill ) // Dump has been reprogrammed on blank flash ROMs and tested working on real PCB
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "dh2vera.ic2",  0x000000, 0x200000, CRC(81bca744) SHA1(0335960126e41f02442828213990f0d30af86696) )
	ROM_LOAD16_BYTE( "dh2vera.ic1",  0x000001, 0x200000, CRC(ea7dcf68) SHA1(fd5110e3dab04f8c8503fd6fe9edbfc7c5a22aaf) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "dh3vera.ic3",  0x000000, 0x080000, CRC(98f9fc8b) SHA1(5152b9e11773033a26da11d1f3774a261e61a2c5) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data ROMs */
	ROM_LOAD16_BYTE( "dh1mtah.2j",   0x000000, 0x800000, CRC(3b56faa7) SHA1(861db7f549bedbb2b837516fcc966ad5890007ce) )
	ROM_LOAD16_BYTE( "dh1mtal.2h",   0x000001, 0x800000, CRC(9fa07bfe) SHA1(a6b847ff7d5eadbf60b434a0d905051ea4227113) )

	ROM_REGION( 0x2000000, "textile", 0 )   /* texture tiles */
	ROM_LOAD( "dh1cgll.4m",   0x0000000, 0x800000, CRC(c0d5ad87) SHA1(bc1992516c63aebdae0322def77f082d799a327a) )
	ROM_LOAD( "dh1cglm.4k",   0x0800000, 0x800000, CRC(5d9a5e35) SHA1(d746abb45f04aa4eb9d43d9c79051e71bf024e38) )
	ROM_LOAD( "dh1cgum.4j",   0x1000000, 0x800000, CRC(1044d0a0) SHA1(e0bf843616e166495fcdc76f076eb53a28287d30) )
	ROM_LOAD( "dh1cguu.5f",   0x1800000, 0x800000, CRC(66cb0dd7) SHA1(1f67320f150f1b55c97eae4b9fe4890fabc8dc7e) )

	ROM_REGION16_LE( 0x400000, "textilemapl", 0 )   /* texture tilemap 0-15 */
	ROM_LOAD( "dh1ccrl.7f",   0x000000, 0x400000, CRC(65c857df) SHA1(5d67b17cf272f042b4264d9871d6e4088c20b788) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "dh1ccrh.7e",   0x000000, 0x200000, CRC(f21c482d) SHA1(bfcead2ff3d10f996ac0bf81470d050bd6374156) )

	ROM_REGION32_BE( 0x2000000, "pointrom", 0 ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "dh1pt0h.7a", 0x0000000, 0x400000, CRC(0e84a5d8) SHA1(28559f978b86d88bb18c3e58e28a97ecfb5f7fa9) )
	ROM_LOAD32_WORD_SWAP( "dh1pt0l.7c", 0x0000002, 0x400000, CRC(d120eee5) SHA1(fa1269d891f4e0510491aa70c4abd5f36852e691) )
	ROM_LOAD32_WORD_SWAP( "dh1pt1h.5a", 0x0800000, 0x400000, CRC(88cd4c90) SHA1(94016c72a9da983e55c74cbdd3691b596ea50c31) )
	ROM_LOAD32_WORD_SWAP( "dh1pt1l.5c", 0x0800002, 0x400000, CRC(dee2f2bf) SHA1(258f9a6e324502550d27b8feaf36244766fa19da) )
	ROM_LOAD32_WORD_SWAP( "dh1pt2h.4a", 0x1000000, 0x400000, CRC(7e167c65) SHA1(018bf6aea4c1640ef728cf7b8e491f11742ede0d) )
	ROM_LOAD32_WORD_SWAP( "dh1pt2l.4c", 0x1000002, 0x400000, CRC(714e3090) SHA1(39827f645dacbb57c7c40193f3f58e879899a4f3) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
	ROM_LOAD( "dh1wavel.2c",  0x000000, 0x800000, CRC(10954726) SHA1(50ee0346c46194dada7b5c0d8b1efe9a7f211b90) )
	ROM_LOAD( "dh1waveh.2a",  0x800000, 0x800000, CRC(2adfa312) SHA1(d01a46af2c95d1ea64e9778979ae147298d921e3) )

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate ROMs */
	ROM_LOAD( "dh1cguu.4f",   0x000000, 0x800000, CRC(66cb0dd7) SHA1(1f67320f150f1b55c97eae4b9fe4890fabc8dc7e) )
	ROM_LOAD( "dh1cgum.5j",   0x000000, 0x800000, CRC(1044d0a0) SHA1(e0bf843616e166495fcdc76f076eb53a28287d30) )
	ROM_LOAD( "dh1cgll.5m",   0x000000, 0x800000, CRC(c0d5ad87) SHA1(bc1992516c63aebdae0322def77f082d799a327a) )
	ROM_LOAD( "dh1cglm.5k",   0x000000, 0x800000, CRC(5d9a5e35) SHA1(d746abb45f04aa4eb9d43d9c79051e71bf024e38) )
	ROM_LOAD( "dh1ccrl.7m",   0x000000, 0x400000, CRC(65c857df) SHA1(5d67b17cf272f042b4264d9871d6e4088c20b788) )
	ROM_LOAD( "dh1ccrh.7k",   0x000000, 0x200000, CRC(f21c482d) SHA1(bfcead2ff3d10f996ac0bf81470d050bd6374156) )
ROM_END


ROM_START( downhillu )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "dh3vera.ic2",  0x000000, 0x200000, CRC(5d9952e9) SHA1(d38422330bd708c247b9968429fbff36fe706598) )
	ROM_LOAD16_BYTE( "dh3vera.ic1",  0x000001, 0x200000, CRC(64a236f3) SHA1(aac59e0db5cfefc4b442e6c3a5189a8418742201) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "dh3vera.ic3",  0x000000, 0x080000, CRC(98f9fc8b) SHA1(5152b9e11773033a26da11d1f3774a261e61a2c5) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data ROMs */
	ROM_LOAD16_BYTE( "dh1mtah.2j",   0x000000, 0x800000, CRC(3b56faa7) SHA1(861db7f549bedbb2b837516fcc966ad5890007ce) )
	ROM_LOAD16_BYTE( "dh1mtal.2h",   0x000001, 0x800000, CRC(9fa07bfe) SHA1(a6b847ff7d5eadbf60b434a0d905051ea4227113) )

	ROM_REGION( 0x2000000, "textile", 0 )   /* texture tiles */
	ROM_LOAD( "dh1cgll.4m",   0x0000000, 0x800000, CRC(c0d5ad87) SHA1(bc1992516c63aebdae0322def77f082d799a327a) )
	ROM_LOAD( "dh1cglm.4k",   0x0800000, 0x800000, CRC(5d9a5e35) SHA1(d746abb45f04aa4eb9d43d9c79051e71bf024e38) )
	ROM_LOAD( "dh1cgum.4j",   0x1000000, 0x800000, CRC(1044d0a0) SHA1(e0bf843616e166495fcdc76f076eb53a28287d30) )
	ROM_LOAD( "dh1cguu.5f",   0x1800000, 0x800000, CRC(66cb0dd7) SHA1(1f67320f150f1b55c97eae4b9fe4890fabc8dc7e) )

	ROM_REGION16_LE( 0x400000, "textilemapl", 0 )   /* texture tilemap 0-15 */
	ROM_LOAD( "dh1ccrl.7f",   0x000000, 0x400000, CRC(65c857df) SHA1(5d67b17cf272f042b4264d9871d6e4088c20b788) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "dh1ccrh.7e",   0x000000, 0x200000, CRC(f21c482d) SHA1(bfcead2ff3d10f996ac0bf81470d050bd6374156) )

	ROM_REGION32_BE( 0x2000000, "pointrom", 0 ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "dh1pt0h.7a", 0x0000000, 0x400000, CRC(0e84a5d8) SHA1(28559f978b86d88bb18c3e58e28a97ecfb5f7fa9) )
	ROM_LOAD32_WORD_SWAP( "dh1pt0l.7c", 0x0000002, 0x400000, CRC(d120eee5) SHA1(fa1269d891f4e0510491aa70c4abd5f36852e691) )
	ROM_LOAD32_WORD_SWAP( "dh1pt1h.5a", 0x0800000, 0x400000, CRC(88cd4c90) SHA1(94016c72a9da983e55c74cbdd3691b596ea50c31) )
	ROM_LOAD32_WORD_SWAP( "dh1pt1l.5c", 0x0800002, 0x400000, CRC(dee2f2bf) SHA1(258f9a6e324502550d27b8feaf36244766fa19da) )
	ROM_LOAD32_WORD_SWAP( "dh1pt2h.4a", 0x1000000, 0x400000, CRC(7e167c65) SHA1(018bf6aea4c1640ef728cf7b8e491f11742ede0d) )
	ROM_LOAD32_WORD_SWAP( "dh1pt2l.4c", 0x1000002, 0x400000, CRC(714e3090) SHA1(39827f645dacbb57c7c40193f3f58e879899a4f3) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
	ROM_LOAD( "dh1wavel.2c",  0x000000, 0x800000, CRC(10954726) SHA1(50ee0346c46194dada7b5c0d8b1efe9a7f211b90) )
	ROM_LOAD( "dh1waveh.2a",  0x800000, 0x800000, CRC(2adfa312) SHA1(d01a46af2c95d1ea64e9778979ae147298d921e3) )

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate ROMs */
	ROM_LOAD( "dh1cguu.4f",   0x000000, 0x800000, CRC(66cb0dd7) SHA1(1f67320f150f1b55c97eae4b9fe4890fabc8dc7e) )
	ROM_LOAD( "dh1cgum.5j",   0x000000, 0x800000, CRC(1044d0a0) SHA1(e0bf843616e166495fcdc76f076eb53a28287d30) )
	ROM_LOAD( "dh1cgll.5m",   0x000000, 0x800000, CRC(c0d5ad87) SHA1(bc1992516c63aebdae0322def77f082d799a327a) )
	ROM_LOAD( "dh1cglm.5k",   0x000000, 0x800000, CRC(5d9a5e35) SHA1(d746abb45f04aa4eb9d43d9c79051e71bf024e38) )
	ROM_LOAD( "dh1ccrl.7m",   0x000000, 0x400000, CRC(65c857df) SHA1(5d67b17cf272f042b4264d9871d6e4088c20b788) )
	ROM_LOAD( "dh1ccrh.7k",   0x000000, 0x200000, CRC(f21c482d) SHA1(bfcead2ff3d10f996ac0bf81470d050bd6374156) )
ROM_END


ROM_START( crszone )
	ROM_REGION32_BE( 0x800000, "user1", 0 ) /* 8 megs for main R4650 code */
	ROM_LOAD16_WORD_SWAP( "cszo4verb.ic4", 0x000000, 0x800000, CRC(6192533d) SHA1(d102b91fe193bf255ea4e57a2bd964aa1cdfd21d) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "cszo3verb.ic1", 0x000000, 0x080000, CRC(c790743b) SHA1(5fa7b83a7a1b1105a3aa0870b782cf2741b7d11c) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data ROMs */
	ROM_LOAD16_BYTE( "csz1mtah.2j",  0x0000000, 0x800000, CRC(66b076ad) SHA1(edd32e0b380f01a9626d32f5eec860f841c8be8a) )
	ROM_LOAD16_BYTE( "csz1mtal.2h",  0x0000001, 0x800000, CRC(38dc639a) SHA1(aa9b5b35174c1b007a57a4bd7a53bc3f479b5b71) )
	ROM_LOAD16_BYTE( "csz1mtbh.2m",  0x1000000, 0x800000, CRC(bdec4188) SHA1(a098651fbd8a69a0afc17f4b6c93350926cacd6b) )
	ROM_LOAD16_BYTE( "csz1mtbl.2f",  0x1000001, 0x800000, CRC(9c8f8d7a) SHA1(f61bcc9763df15428c82931a605ee40334d5ad98) )

	ROM_REGION( 0x2000000, "textile", 0 )   /* texture tiles */
	ROM_LOAD( "csz1cgll.4m",  0x0000000, 0x800000, CRC(0bcd41f2) SHA1(80b74f9398e8bd074f79a14490d06cfeb875c874) )
	ROM_LOAD( "csz1cglm.4k",  0x0800000, 0x800000, CRC(d4af93d1) SHA1(0df37b793ce8da02d14f714722382786ae5d3ce2) )
	ROM_LOAD( "csz1cgum.4j",  0x1000000, 0x800000, CRC(913c98b5) SHA1(b952dbc19053796077d4f33e8da836893e933b12) )
	ROM_LOAD( "csz1cguu.5f",  0x1800000, 0x800000, CRC(e1d1bf24) SHA1(daf2c68e2d9a8f313d262d221cc990c93dfdf22f) )

	ROM_REGION16_LE( 0x400000, "textilemapl", 0 )   /* texture tilemap 0-15 */
	ROM_LOAD( "csz1ccrl.7f",  0x000000, 0x400000, CRC(1c20768d) SHA1(6cf4280e26f3625d6f750837bf344163e7e93c3d) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "csz1ccrh.7e",  0x000000, 0x200000, CRC(bc2fa03c) SHA1(e63d8e75494a383bf9a213edfa9c472a010f8efe) )

	ROM_REGION32_BE( 0x2000000, "pointrom", 0 ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "csz1pt0h.7a",  0x0000000, 0x400000, CRC(e82f1abb) SHA1(b1c57152cc27835e06e429fd1659fe0973638142) )
	ROM_LOAD32_WORD_SWAP( "csz1pt0l.7c",  0x0000002, 0x400000, CRC(b0d66afe) SHA1(7cda4eebf1bb1191d17e4b5e616be2fbe4ae9328) )
	ROM_LOAD32_WORD_SWAP( "csz1pt1h.5a",  0x0800000, 0x400000, CRC(e54f80ad) SHA1(3b3fbb3001e630d800b02ec8e653d74878ac5116) )
	ROM_LOAD32_WORD_SWAP( "csz1pt1l.5c",  0x0800002, 0x400000, CRC(527171c8) SHA1(0b2ce3858f40bdedf1543309a6bc28d780415250) )
	ROM_LOAD32_WORD_SWAP( "csz1pt2h.4a",  0x1000000, 0x400000, CRC(e295137a) SHA1(37b18af1b3d9f0e69b45135f89b49a1ceec79127) )
	ROM_LOAD32_WORD_SWAP( "csz1pt2l.4c",  0x1000002, 0x400000, CRC(c87d6dbd) SHA1(686f39073c521d6b21ef8bc1161b41b680697c63) )
	ROM_LOAD32_WORD_SWAP( "csz1pt3h.3a",  0x1800000, 0x400000, CRC(05f65bdf) SHA1(0c349fe5381fe7aeb81f9365a2b44a212f6bd33e) )
	ROM_LOAD32_WORD_SWAP( "csz1pt3l.3c",  0x1800002, 0x400000, CRC(5d077c0f) SHA1(a4fd0167d89bf9417766405726e0334e7c7eaec3) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
	ROM_LOAD( "csz1wavel.2c", 0x000000, 0x800000, CRC(d0d74132) SHA1(a293d93bca8e12e388a088a592cfa7bcb9a976f7) )
	ROM_LOAD( "csz1waveh.2a", 0x800000, 0x800000, CRC(de9d14a8) SHA1(e5006861928bb1d29bf80c7304f1a6d044b094fd) )

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate ROMs */
	ROM_LOAD( "csz1cguu.4f",  0x000000, 0x800000, CRC(e1d1bf24) SHA1(daf2c68e2d9a8f313d262d221cc990c93dfdf22f) )
	ROM_LOAD( "csz1cgum.5j",  0x000000, 0x800000, CRC(913c98b5) SHA1(b952dbc19053796077d4f33e8da836893e933b12) )
	ROM_LOAD( "csz1cgll.5m",  0x000000, 0x800000, CRC(0bcd41f2) SHA1(80b74f9398e8bd074f79a14490d06cfeb875c874) )
	ROM_LOAD( "csz1cglm.5k",  0x000000, 0x800000, CRC(d4af93d1) SHA1(0df37b793ce8da02d14f714722382786ae5d3ce2) )
	ROM_LOAD( "csz1ccrl.7m",  0x000000, 0x400000, CRC(1c20768d) SHA1(6cf4280e26f3625d6f750837bf344163e7e93c3d) )
	ROM_LOAD( "csz1ccrh.7k",  0x000000, 0x200000, CRC(bc2fa03c) SHA1(e63d8e75494a383bf9a213edfa9c472a010f8efe) )
ROM_END


ROM_START( crszonev4a )
	ROM_REGION32_BE( 0x800000, "user1", 0 ) /* 8 megs for main R4650 code */
	ROM_LOAD16_WORD_SWAP( "cszo4vera.ic4", 0x000000, 0x800000, CRC(cabee8c3) SHA1(4887b8550038c072f988c5999d57ec40e82e4072) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "cszo3verb.ic1", 0x000000, 0x080000, CRC(c790743b) SHA1(5fa7b83a7a1b1105a3aa0870b782cf2741b7d11c) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data ROMs */
	ROM_LOAD16_BYTE( "csz1mtah.2j",  0x0000000, 0x800000, CRC(66b076ad) SHA1(edd32e0b380f01a9626d32f5eec860f841c8be8a) )
	ROM_LOAD16_BYTE( "csz1mtal.2h",  0x0000001, 0x800000, CRC(38dc639a) SHA1(aa9b5b35174c1b007a57a4bd7a53bc3f479b5b71) )
	ROM_LOAD16_BYTE( "csz1mtbh.2m",  0x1000000, 0x800000, CRC(bdec4188) SHA1(a098651fbd8a69a0afc17f4b6c93350926cacd6b) )
	ROM_LOAD16_BYTE( "csz1mtbl.2f",  0x1000001, 0x800000, CRC(9c8f8d7a) SHA1(f61bcc9763df15428c82931a605ee40334d5ad98) )

	ROM_REGION( 0x2000000, "textile", 0 )   /* texture tiles */
	ROM_LOAD( "csz1cgll.4m",  0x0000000, 0x800000, CRC(0bcd41f2) SHA1(80b74f9398e8bd074f79a14490d06cfeb875c874) )
	ROM_LOAD( "csz1cglm.4k",  0x0800000, 0x800000, CRC(d4af93d1) SHA1(0df37b793ce8da02d14f714722382786ae5d3ce2) )
	ROM_LOAD( "csz1cgum.4j",  0x1000000, 0x800000, CRC(913c98b5) SHA1(b952dbc19053796077d4f33e8da836893e933b12) )
	ROM_LOAD( "csz1cguu.5f",  0x1800000, 0x800000, CRC(e1d1bf24) SHA1(daf2c68e2d9a8f313d262d221cc990c93dfdf22f) )

	ROM_REGION16_LE( 0x400000, "textilemapl", 0 )   /* texture tilemap 0-15 */
	ROM_LOAD( "csz1ccrl.7f",  0x000000, 0x400000, CRC(1c20768d) SHA1(6cf4280e26f3625d6f750837bf344163e7e93c3d) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "csz1ccrh.7e",  0x000000, 0x200000, CRC(bc2fa03c) SHA1(e63d8e75494a383bf9a213edfa9c472a010f8efe) )

	ROM_REGION32_BE( 0x2000000, "pointrom", 0 ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "csz1pt0h.7a",  0x0000000, 0x400000, CRC(e82f1abb) SHA1(b1c57152cc27835e06e429fd1659fe0973638142) )
	ROM_LOAD32_WORD_SWAP( "csz1pt0l.7c",  0x0000002, 0x400000, CRC(b0d66afe) SHA1(7cda4eebf1bb1191d17e4b5e616be2fbe4ae9328) )
	ROM_LOAD32_WORD_SWAP( "csz1pt1h.5a",  0x0800000, 0x400000, CRC(e54f80ad) SHA1(3b3fbb3001e630d800b02ec8e653d74878ac5116) )
	ROM_LOAD32_WORD_SWAP( "csz1pt1l.5c",  0x0800002, 0x400000, CRC(527171c8) SHA1(0b2ce3858f40bdedf1543309a6bc28d780415250) )
	ROM_LOAD32_WORD_SWAP( "csz1pt2h.4a",  0x1000000, 0x400000, CRC(e295137a) SHA1(37b18af1b3d9f0e69b45135f89b49a1ceec79127) )
	ROM_LOAD32_WORD_SWAP( "csz1pt2l.4c",  0x1000002, 0x400000, CRC(c87d6dbd) SHA1(686f39073c521d6b21ef8bc1161b41b680697c63) )
	ROM_LOAD32_WORD_SWAP( "csz1pt3h.3a",  0x1800000, 0x400000, CRC(05f65bdf) SHA1(0c349fe5381fe7aeb81f9365a2b44a212f6bd33e) )
	ROM_LOAD32_WORD_SWAP( "csz1pt3l.3c",  0x1800002, 0x400000, CRC(5d077c0f) SHA1(a4fd0167d89bf9417766405726e0334e7c7eaec3) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
	ROM_LOAD( "csz1wavel.2c", 0x000000, 0x800000, CRC(d0d74132) SHA1(a293d93bca8e12e388a088a592cfa7bcb9a976f7) )
	ROM_LOAD( "csz1waveh.2a", 0x800000, 0x800000, CRC(de9d14a8) SHA1(e5006861928bb1d29bf80c7304f1a6d044b094fd) )

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate ROMs */
	ROM_LOAD( "csz1cguu.4f",  0x000000, 0x800000, CRC(e1d1bf24) SHA1(daf2c68e2d9a8f313d262d221cc990c93dfdf22f) )
	ROM_LOAD( "csz1cgum.5j",  0x000000, 0x800000, CRC(913c98b5) SHA1(b952dbc19053796077d4f33e8da836893e933b12) )
	ROM_LOAD( "csz1cgll.5m",  0x000000, 0x800000, CRC(0bcd41f2) SHA1(80b74f9398e8bd074f79a14490d06cfeb875c874) )
	ROM_LOAD( "csz1cglm.5k",  0x000000, 0x800000, CRC(d4af93d1) SHA1(0df37b793ce8da02d14f714722382786ae5d3ce2) )
	ROM_LOAD( "csz1ccrl.7m",  0x000000, 0x400000, CRC(1c20768d) SHA1(6cf4280e26f3625d6f750837bf344163e7e93c3d) )
	ROM_LOAD( "csz1ccrh.7k",  0x000000, 0x200000, CRC(bc2fa03c) SHA1(e63d8e75494a383bf9a213edfa9c472a010f8efe) )
ROM_END


ROM_START( crszonev3b )
	ROM_REGION32_BE( 0x800000, "user1", 0 ) /* 8 megs for main R4650 code */
	ROM_LOAD16_WORD_SWAP( "cszo3verb.ic4", 0x000000, 0x800000, CRC(4cb26465) SHA1(078dfd0d8c920707df14e9a26658fa63421fcb0b) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "cszo3verb.ic1", 0x000000, 0x080000, CRC(c790743b) SHA1(5fa7b83a7a1b1105a3aa0870b782cf2741b7d11c) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data ROMs */
	ROM_LOAD16_BYTE( "csz1mtah.2j",  0x0000000, 0x800000, CRC(66b076ad) SHA1(edd32e0b380f01a9626d32f5eec860f841c8be8a) )
	ROM_LOAD16_BYTE( "csz1mtal.2h",  0x0000001, 0x800000, CRC(38dc639a) SHA1(aa9b5b35174c1b007a57a4bd7a53bc3f479b5b71) )
	ROM_LOAD16_BYTE( "csz1mtbh.2m",  0x1000000, 0x800000, CRC(bdec4188) SHA1(a098651fbd8a69a0afc17f4b6c93350926cacd6b) )
	ROM_LOAD16_BYTE( "csz1mtbl.2f",  0x1000001, 0x800000, CRC(9c8f8d7a) SHA1(f61bcc9763df15428c82931a605ee40334d5ad98) )

	ROM_REGION( 0x2000000, "textile", 0 )   /* texture tiles */
	ROM_LOAD( "csz1cgll.4m",  0x0000000, 0x800000, CRC(0bcd41f2) SHA1(80b74f9398e8bd074f79a14490d06cfeb875c874) )
	ROM_LOAD( "csz1cglm.4k",  0x0800000, 0x800000, CRC(d4af93d1) SHA1(0df37b793ce8da02d14f714722382786ae5d3ce2) )
	ROM_LOAD( "csz1cgum.4j",  0x1000000, 0x800000, CRC(913c98b5) SHA1(b952dbc19053796077d4f33e8da836893e933b12) )
	ROM_LOAD( "csz1cguu.5f",  0x1800000, 0x800000, CRC(e1d1bf24) SHA1(daf2c68e2d9a8f313d262d221cc990c93dfdf22f) )

	ROM_REGION16_LE( 0x400000, "textilemapl", 0 )   /* texture tilemap 0-15 */
	ROM_LOAD( "csz1ccrl.7f",  0x000000, 0x400000, CRC(1c20768d) SHA1(6cf4280e26f3625d6f750837bf344163e7e93c3d) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "csz1ccrh.7e",  0x000000, 0x200000, CRC(bc2fa03c) SHA1(e63d8e75494a383bf9a213edfa9c472a010f8efe) )

	ROM_REGION32_BE( 0x2000000, "pointrom", 0 ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "csz1pt0h.7a",  0x0000000, 0x400000, CRC(e82f1abb) SHA1(b1c57152cc27835e06e429fd1659fe0973638142) )
	ROM_LOAD32_WORD_SWAP( "csz1pt0l.7c",  0x0000002, 0x400000, CRC(b0d66afe) SHA1(7cda4eebf1bb1191d17e4b5e616be2fbe4ae9328) )
	ROM_LOAD32_WORD_SWAP( "csz1pt1h.5a",  0x0800000, 0x400000, CRC(e54f80ad) SHA1(3b3fbb3001e630d800b02ec8e653d74878ac5116) )
	ROM_LOAD32_WORD_SWAP( "csz1pt1l.5c",  0x0800002, 0x400000, CRC(527171c8) SHA1(0b2ce3858f40bdedf1543309a6bc28d780415250) )
	ROM_LOAD32_WORD_SWAP( "csz1pt2h.4a",  0x1000000, 0x400000, CRC(e295137a) SHA1(37b18af1b3d9f0e69b45135f89b49a1ceec79127) )
	ROM_LOAD32_WORD_SWAP( "csz1pt2l.4c",  0x1000002, 0x400000, CRC(c87d6dbd) SHA1(686f39073c521d6b21ef8bc1161b41b680697c63) )
	ROM_LOAD32_WORD_SWAP( "csz1pt3h.3a",  0x1800000, 0x400000, CRC(05f65bdf) SHA1(0c349fe5381fe7aeb81f9365a2b44a212f6bd33e) )
	ROM_LOAD32_WORD_SWAP( "csz1pt3l.3c",  0x1800002, 0x400000, CRC(5d077c0f) SHA1(a4fd0167d89bf9417766405726e0334e7c7eaec3) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
	ROM_LOAD( "csz1wavel.2c", 0x000000, 0x800000, CRC(d0d74132) SHA1(a293d93bca8e12e388a088a592cfa7bcb9a976f7) )
	ROM_LOAD( "csz1waveh.2a", 0x800000, 0x800000, CRC(de9d14a8) SHA1(e5006861928bb1d29bf80c7304f1a6d044b094fd) )

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate ROMs */
	ROM_LOAD( "csz1cguu.4f",  0x000000, 0x800000, CRC(e1d1bf24) SHA1(daf2c68e2d9a8f313d262d221cc990c93dfdf22f) )
	ROM_LOAD( "csz1cgum.5j",  0x000000, 0x800000, CRC(913c98b5) SHA1(b952dbc19053796077d4f33e8da836893e933b12) )
	ROM_LOAD( "csz1cgll.5m",  0x000000, 0x800000, CRC(0bcd41f2) SHA1(80b74f9398e8bd074f79a14490d06cfeb875c874) )
	ROM_LOAD( "csz1cglm.5k",  0x000000, 0x800000, CRC(d4af93d1) SHA1(0df37b793ce8da02d14f714722382786ae5d3ce2) )
	ROM_LOAD( "csz1ccrl.7m",  0x000000, 0x400000, CRC(1c20768d) SHA1(6cf4280e26f3625d6f750837bf344163e7e93c3d) )
	ROM_LOAD( "csz1ccrh.7k",  0x000000, 0x200000, CRC(bc2fa03c) SHA1(e63d8e75494a383bf9a213edfa9c472a010f8efe) )
ROM_END


ROM_START( crszonev3b2 )
	ROM_REGION32_BE( 0x800000, "user1", 0 ) /* 8 megs for main R4650 code */
	ROM_LOAD16_WORD_SWAP( "cszo3verb.ic4", 0x000000, 0x800000, CRC(3755b402) SHA1(e169fded9d136af7ce6997868629eed5196b8cdd) ) // sldh

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "cszo3verb.ic1", 0x000000, 0x080000, CRC(c790743b) SHA1(5fa7b83a7a1b1105a3aa0870b782cf2741b7d11c) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data ROMs */
	ROM_LOAD16_BYTE( "csz1mtah.2j",  0x0000000, 0x800000, CRC(66b076ad) SHA1(edd32e0b380f01a9626d32f5eec860f841c8be8a) )
	ROM_LOAD16_BYTE( "csz1mtal.2h",  0x0000001, 0x800000, CRC(38dc639a) SHA1(aa9b5b35174c1b007a57a4bd7a53bc3f479b5b71) )
	ROM_LOAD16_BYTE( "csz1mtbh.2m",  0x1000000, 0x800000, CRC(bdec4188) SHA1(a098651fbd8a69a0afc17f4b6c93350926cacd6b) )
	ROM_LOAD16_BYTE( "csz1mtbl.2f",  0x1000001, 0x800000, CRC(9c8f8d7a) SHA1(f61bcc9763df15428c82931a605ee40334d5ad98) )

	ROM_REGION( 0x2000000, "textile", 0 )   /* texture tiles */
	ROM_LOAD( "csz1cgll.4m",  0x0000000, 0x800000, CRC(0bcd41f2) SHA1(80b74f9398e8bd074f79a14490d06cfeb875c874) )
	ROM_LOAD( "csz1cglm.4k",  0x0800000, 0x800000, CRC(d4af93d1) SHA1(0df37b793ce8da02d14f714722382786ae5d3ce2) )
	ROM_LOAD( "csz1cgum.4j",  0x1000000, 0x800000, CRC(913c98b5) SHA1(b952dbc19053796077d4f33e8da836893e933b12) )
	ROM_LOAD( "csz1cguu.5f",  0x1800000, 0x800000, CRC(e1d1bf24) SHA1(daf2c68e2d9a8f313d262d221cc990c93dfdf22f) )

	ROM_REGION16_LE( 0x400000, "textilemapl", 0 )   /* texture tilemap 0-15 */
	ROM_LOAD( "csz1ccrl.7f",  0x000000, 0x400000, CRC(1c20768d) SHA1(6cf4280e26f3625d6f750837bf344163e7e93c3d) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "csz1ccrh.7e",  0x000000, 0x200000, CRC(bc2fa03c) SHA1(e63d8e75494a383bf9a213edfa9c472a010f8efe) )

	ROM_REGION32_BE( 0x2000000, "pointrom", 0 ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "csz1pt0h.7a",  0x0000000, 0x400000, CRC(e82f1abb) SHA1(b1c57152cc27835e06e429fd1659fe0973638142) )
	ROM_LOAD32_WORD_SWAP( "csz1pt0l.7c",  0x0000002, 0x400000, CRC(b0d66afe) SHA1(7cda4eebf1bb1191d17e4b5e616be2fbe4ae9328) )
	ROM_LOAD32_WORD_SWAP( "csz1pt1h.5a",  0x0800000, 0x400000, CRC(e54f80ad) SHA1(3b3fbb3001e630d800b02ec8e653d74878ac5116) )
	ROM_LOAD32_WORD_SWAP( "csz1pt1l.5c",  0x0800002, 0x400000, CRC(527171c8) SHA1(0b2ce3858f40bdedf1543309a6bc28d780415250) )
	ROM_LOAD32_WORD_SWAP( "csz1pt2h.4a",  0x1000000, 0x400000, CRC(e295137a) SHA1(37b18af1b3d9f0e69b45135f89b49a1ceec79127) )
	ROM_LOAD32_WORD_SWAP( "csz1pt2l.4c",  0x1000002, 0x400000, CRC(c87d6dbd) SHA1(686f39073c521d6b21ef8bc1161b41b680697c63) )
	ROM_LOAD32_WORD_SWAP( "csz1pt3h.3a",  0x1800000, 0x400000, CRC(05f65bdf) SHA1(0c349fe5381fe7aeb81f9365a2b44a212f6bd33e) )
	ROM_LOAD32_WORD_SWAP( "csz1pt3l.3c",  0x1800002, 0x400000, CRC(5d077c0f) SHA1(a4fd0167d89bf9417766405726e0334e7c7eaec3) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
	ROM_LOAD( "csz1wavel.2c", 0x000000, 0x800000, CRC(d0d74132) SHA1(a293d93bca8e12e388a088a592cfa7bcb9a976f7) )
	ROM_LOAD( "csz1waveh.2a", 0x800000, 0x800000, CRC(de9d14a8) SHA1(e5006861928bb1d29bf80c7304f1a6d044b094fd) )

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate ROMs */
	ROM_LOAD( "csz1cguu.4f",  0x000000, 0x800000, CRC(e1d1bf24) SHA1(daf2c68e2d9a8f313d262d221cc990c93dfdf22f) )
	ROM_LOAD( "csz1cgum.5j",  0x000000, 0x800000, CRC(913c98b5) SHA1(b952dbc19053796077d4f33e8da836893e933b12) )
	ROM_LOAD( "csz1cgll.5m",  0x000000, 0x800000, CRC(0bcd41f2) SHA1(80b74f9398e8bd074f79a14490d06cfeb875c874) )
	ROM_LOAD( "csz1cglm.5k",  0x000000, 0x800000, CRC(d4af93d1) SHA1(0df37b793ce8da02d14f714722382786ae5d3ce2) )
	ROM_LOAD( "csz1ccrl.7m",  0x000000, 0x400000, CRC(1c20768d) SHA1(6cf4280e26f3625d6f750837bf344163e7e93c3d) )
	ROM_LOAD( "csz1ccrh.7k",  0x000000, 0x200000, CRC(bc2fa03c) SHA1(e63d8e75494a383bf9a213edfa9c472a010f8efe) )
ROM_END


ROM_START( crszonev3a )
	ROM_REGION32_BE( 0x800000, "user1", 0 ) /* 8 megs for main R4650 code */
	ROM_LOAD16_WORD_SWAP( "cszo3vera.ic4", 0x000000, 0x800000, CRC(09b0c91e) SHA1(226c3788d6a50272e2544d04d9ca20df81014fb6) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "cszo3verb.ic1", 0x000000, 0x080000, CRC(c790743b) SHA1(5fa7b83a7a1b1105a3aa0870b782cf2741b7d11c) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data ROMs */
	ROM_LOAD16_BYTE( "csz1mtah.2j",  0x0000000, 0x800000, CRC(66b076ad) SHA1(edd32e0b380f01a9626d32f5eec860f841c8be8a) )
	ROM_LOAD16_BYTE( "csz1mtal.2h",  0x0000001, 0x800000, CRC(38dc639a) SHA1(aa9b5b35174c1b007a57a4bd7a53bc3f479b5b71) )
	ROM_LOAD16_BYTE( "csz1mtbh.2m",  0x1000000, 0x800000, CRC(bdec4188) SHA1(a098651fbd8a69a0afc17f4b6c93350926cacd6b) )
	ROM_LOAD16_BYTE( "csz1mtbl.2f",  0x1000001, 0x800000, CRC(9c8f8d7a) SHA1(f61bcc9763df15428c82931a605ee40334d5ad98) )

	ROM_REGION( 0x2000000, "textile", 0 )   /* texture tiles */
	ROM_LOAD( "csz1cgll.4m",  0x0000000, 0x800000, CRC(0bcd41f2) SHA1(80b74f9398e8bd074f79a14490d06cfeb875c874) )
	ROM_LOAD( "csz1cglm.4k",  0x0800000, 0x800000, CRC(d4af93d1) SHA1(0df37b793ce8da02d14f714722382786ae5d3ce2) )
	ROM_LOAD( "csz1cgum.4j",  0x1000000, 0x800000, CRC(913c98b5) SHA1(b952dbc19053796077d4f33e8da836893e933b12) )
	ROM_LOAD( "csz1cguu.5f",  0x1800000, 0x800000, CRC(e1d1bf24) SHA1(daf2c68e2d9a8f313d262d221cc990c93dfdf22f) )

	ROM_REGION16_LE( 0x400000, "textilemapl", 0 )   /* texture tilemap 0-15 */
	ROM_LOAD( "csz1ccrl.7f",  0x000000, 0x400000, CRC(1c20768d) SHA1(6cf4280e26f3625d6f750837bf344163e7e93c3d) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "csz1ccrh.7e",  0x000000, 0x200000, CRC(bc2fa03c) SHA1(e63d8e75494a383bf9a213edfa9c472a010f8efe) )

	ROM_REGION32_BE( 0x2000000, "pointrom", 0 ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "csz1pt0h.7a",  0x0000000, 0x400000, CRC(e82f1abb) SHA1(b1c57152cc27835e06e429fd1659fe0973638142) )
	ROM_LOAD32_WORD_SWAP( "csz1pt0l.7c",  0x0000002, 0x400000, CRC(b0d66afe) SHA1(7cda4eebf1bb1191d17e4b5e616be2fbe4ae9328) )
	ROM_LOAD32_WORD_SWAP( "csz1pt1h.5a",  0x0800000, 0x400000, CRC(e54f80ad) SHA1(3b3fbb3001e630d800b02ec8e653d74878ac5116) )
	ROM_LOAD32_WORD_SWAP( "csz1pt1l.5c",  0x0800002, 0x400000, CRC(527171c8) SHA1(0b2ce3858f40bdedf1543309a6bc28d780415250) )
	ROM_LOAD32_WORD_SWAP( "csz1pt2h.4a",  0x1000000, 0x400000, CRC(e295137a) SHA1(37b18af1b3d9f0e69b45135f89b49a1ceec79127) )
	ROM_LOAD32_WORD_SWAP( "csz1pt2l.4c",  0x1000002, 0x400000, CRC(c87d6dbd) SHA1(686f39073c521d6b21ef8bc1161b41b680697c63) )
	ROM_LOAD32_WORD_SWAP( "csz1pt3h.3a",  0x1800000, 0x400000, CRC(05f65bdf) SHA1(0c349fe5381fe7aeb81f9365a2b44a212f6bd33e) )
	ROM_LOAD32_WORD_SWAP( "csz1pt3l.3c",  0x1800002, 0x400000, CRC(5d077c0f) SHA1(a4fd0167d89bf9417766405726e0334e7c7eaec3) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
	ROM_LOAD( "csz1wavel.2c", 0x000000, 0x800000, CRC(d0d74132) SHA1(a293d93bca8e12e388a088a592cfa7bcb9a976f7) )
	ROM_LOAD( "csz1waveh.2a", 0x800000, 0x800000, CRC(de9d14a8) SHA1(e5006861928bb1d29bf80c7304f1a6d044b094fd) )

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate ROMs */
	ROM_LOAD( "csz1cguu.4f",  0x000000, 0x800000, CRC(e1d1bf24) SHA1(daf2c68e2d9a8f313d262d221cc990c93dfdf22f) )
	ROM_LOAD( "csz1cgum.5j",  0x000000, 0x800000, CRC(913c98b5) SHA1(b952dbc19053796077d4f33e8da836893e933b12) )
	ROM_LOAD( "csz1cgll.5m",  0x000000, 0x800000, CRC(0bcd41f2) SHA1(80b74f9398e8bd074f79a14490d06cfeb875c874) )
	ROM_LOAD( "csz1cglm.5k",  0x000000, 0x800000, CRC(d4af93d1) SHA1(0df37b793ce8da02d14f714722382786ae5d3ce2) )
	ROM_LOAD( "csz1ccrl.7m",  0x000000, 0x400000, CRC(1c20768d) SHA1(6cf4280e26f3625d6f750837bf344163e7e93c3d) )
	ROM_LOAD( "csz1ccrh.7k",  0x000000, 0x200000, CRC(bc2fa03c) SHA1(e63d8e75494a383bf9a213edfa9c472a010f8efe) )
ROM_END


ROM_START( crszonev2a )
	ROM_REGION32_BE( 0x800000, "user1", 0 ) /* 8 megs for main R4650 code */
	ROM_LOAD16_WORD_SWAP( "cszo2vera.ic4", 0x000000, 0x800000, CRC(1426d8d0) SHA1(e8049df1b2db1180f9edf6e5fa9fe8692ae81086) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "cszo3verb.ic1", 0x000000, 0x080000, CRC(c790743b) SHA1(5fa7b83a7a1b1105a3aa0870b782cf2741b7d11c) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data ROMs */
	ROM_LOAD16_BYTE( "csz1mtah.2j",  0x0000000, 0x800000, CRC(66b076ad) SHA1(edd32e0b380f01a9626d32f5eec860f841c8be8a) )
	ROM_LOAD16_BYTE( "csz1mtal.2h",  0x0000001, 0x800000, CRC(38dc639a) SHA1(aa9b5b35174c1b007a57a4bd7a53bc3f479b5b71) )
	ROM_LOAD16_BYTE( "csz1mtbh.2m",  0x1000000, 0x800000, CRC(bdec4188) SHA1(a098651fbd8a69a0afc17f4b6c93350926cacd6b) )
	ROM_LOAD16_BYTE( "csz1mtbl.2f",  0x1000001, 0x800000, CRC(9c8f8d7a) SHA1(f61bcc9763df15428c82931a605ee40334d5ad98) )

	ROM_REGION( 0x2000000, "textile", 0 )   /* texture tiles */
	ROM_LOAD( "csz1cgll.4m",  0x0000000, 0x800000, CRC(0bcd41f2) SHA1(80b74f9398e8bd074f79a14490d06cfeb875c874) )
	ROM_LOAD( "csz1cglm.4k",  0x0800000, 0x800000, CRC(d4af93d1) SHA1(0df37b793ce8da02d14f714722382786ae5d3ce2) )
	ROM_LOAD( "csz1cgum.4j",  0x1000000, 0x800000, CRC(913c98b5) SHA1(b952dbc19053796077d4f33e8da836893e933b12) )
	ROM_LOAD( "csz1cguu.5f",  0x1800000, 0x800000, CRC(e1d1bf24) SHA1(daf2c68e2d9a8f313d262d221cc990c93dfdf22f) )

	ROM_REGION16_LE( 0x400000, "textilemapl", 0 )   /* texture tilemap 0-15 */
	ROM_LOAD( "csz1ccrl.7f",  0x000000, 0x400000, CRC(1c20768d) SHA1(6cf4280e26f3625d6f750837bf344163e7e93c3d) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "csz1ccrh.7e",  0x000000, 0x200000, CRC(bc2fa03c) SHA1(e63d8e75494a383bf9a213edfa9c472a010f8efe) )

	ROM_REGION32_BE( 0x2000000, "pointrom", 0 ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "csz1pt0h.7a",  0x0000000, 0x400000, CRC(e82f1abb) SHA1(b1c57152cc27835e06e429fd1659fe0973638142) )
	ROM_LOAD32_WORD_SWAP( "csz1pt0l.7c",  0x0000002, 0x400000, CRC(b0d66afe) SHA1(7cda4eebf1bb1191d17e4b5e616be2fbe4ae9328) )
	ROM_LOAD32_WORD_SWAP( "csz1pt1h.5a",  0x0800000, 0x400000, CRC(e54f80ad) SHA1(3b3fbb3001e630d800b02ec8e653d74878ac5116) )
	ROM_LOAD32_WORD_SWAP( "csz1pt1l.5c",  0x0800002, 0x400000, CRC(527171c8) SHA1(0b2ce3858f40bdedf1543309a6bc28d780415250) )
	ROM_LOAD32_WORD_SWAP( "csz1pt2h.4a",  0x1000000, 0x400000, CRC(e295137a) SHA1(37b18af1b3d9f0e69b45135f89b49a1ceec79127) )
	ROM_LOAD32_WORD_SWAP( "csz1pt2l.4c",  0x1000002, 0x400000, CRC(c87d6dbd) SHA1(686f39073c521d6b21ef8bc1161b41b680697c63) )
	ROM_LOAD32_WORD_SWAP( "csz1pt3h.3a",  0x1800000, 0x400000, CRC(05f65bdf) SHA1(0c349fe5381fe7aeb81f9365a2b44a212f6bd33e) )
	ROM_LOAD32_WORD_SWAP( "csz1pt3l.3c",  0x1800002, 0x400000, CRC(5d077c0f) SHA1(a4fd0167d89bf9417766405726e0334e7c7eaec3) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
	ROM_LOAD( "csz1wavel.2c", 0x000000, 0x800000, CRC(d0d74132) SHA1(a293d93bca8e12e388a088a592cfa7bcb9a976f7) )
	ROM_LOAD( "csz1waveh.2a", 0x800000, 0x800000, CRC(de9d14a8) SHA1(e5006861928bb1d29bf80c7304f1a6d044b094fd) )

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate ROMs */
	ROM_LOAD( "csz1cguu.4f",  0x000000, 0x800000, CRC(e1d1bf24) SHA1(daf2c68e2d9a8f313d262d221cc990c93dfdf22f) )
	ROM_LOAD( "csz1cgum.5j",  0x000000, 0x800000, CRC(913c98b5) SHA1(b952dbc19053796077d4f33e8da836893e933b12) )
	ROM_LOAD( "csz1cgll.5m",  0x000000, 0x800000, CRC(0bcd41f2) SHA1(80b74f9398e8bd074f79a14490d06cfeb875c874) )
	ROM_LOAD( "csz1cglm.5k",  0x000000, 0x800000, CRC(d4af93d1) SHA1(0df37b793ce8da02d14f714722382786ae5d3ce2) )
	ROM_LOAD( "csz1ccrl.7m",  0x000000, 0x400000, CRC(1c20768d) SHA1(6cf4280e26f3625d6f750837bf344163e7e93c3d) )
	ROM_LOAD( "csz1ccrh.7k",  0x000000, 0x200000, CRC(bc2fa03c) SHA1(e63d8e75494a383bf9a213edfa9c472a010f8efe) )
ROM_END

ROM_START( crszonev2b )
	ROM_REGION32_BE( 0x800000, "user1", 0 ) /* 8 megs for main R4650 code */
	ROM_LOAD16_WORD_SWAP( "cszo2verb.ic4", 0x000000, 0x800000, CRC(9a1a456d) SHA1(7f3a10bb6cc1de0613c9bec292eda54c8e95ff8a) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "cszo3verb.ic1", 0x000000, 0x080000, CRC(c790743b) SHA1(5fa7b83a7a1b1105a3aa0870b782cf2741b7d11c) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data ROMs */
	ROM_LOAD16_BYTE( "csz1mtah.2j",  0x0000000, 0x800000, CRC(66b076ad) SHA1(edd32e0b380f01a9626d32f5eec860f841c8be8a) )
	ROM_LOAD16_BYTE( "csz1mtal.2h",  0x0000001, 0x800000, CRC(38dc639a) SHA1(aa9b5b35174c1b007a57a4bd7a53bc3f479b5b71) )
	ROM_LOAD16_BYTE( "csz1mtbh.2m",  0x1000000, 0x800000, CRC(bdec4188) SHA1(a098651fbd8a69a0afc17f4b6c93350926cacd6b) )
	ROM_LOAD16_BYTE( "csz1mtbl.2f",  0x1000001, 0x800000, CRC(9c8f8d7a) SHA1(f61bcc9763df15428c82931a605ee40334d5ad98) )

	ROM_REGION( 0x2000000, "textile", 0 )   /* texture tiles */
	ROM_LOAD( "csz1cgll.4m",  0x0000000, 0x800000, CRC(0bcd41f2) SHA1(80b74f9398e8bd074f79a14490d06cfeb875c874) )
	ROM_LOAD( "csz1cglm.4k",  0x0800000, 0x800000, CRC(d4af93d1) SHA1(0df37b793ce8da02d14f714722382786ae5d3ce2) )
	ROM_LOAD( "csz1cgum.4j",  0x1000000, 0x800000, CRC(913c98b5) SHA1(b952dbc19053796077d4f33e8da836893e933b12) )
	ROM_LOAD( "csz1cguu.5f",  0x1800000, 0x800000, CRC(e1d1bf24) SHA1(daf2c68e2d9a8f313d262d221cc990c93dfdf22f) )

	ROM_REGION16_LE( 0x400000, "textilemapl", 0 )   /* texture tilemap 0-15 */
	ROM_LOAD( "csz1ccrl.7f",  0x000000, 0x400000, CRC(1c20768d) SHA1(6cf4280e26f3625d6f750837bf344163e7e93c3d) )

	ROM_REGION( 0x200000, "textilemaph", 0 )        /* texture tilemap 16-17 + attr */
	ROM_LOAD( "csz1ccrh.7e",  0x000000, 0x200000, CRC(bc2fa03c) SHA1(e63d8e75494a383bf9a213edfa9c472a010f8efe) )

	ROM_REGION32_BE( 0x2000000, "pointrom", 0 ) /* 3D model data */
	ROM_LOAD32_WORD_SWAP( "csz1pt0h.7a",  0x0000000, 0x400000, CRC(e82f1abb) SHA1(b1c57152cc27835e06e429fd1659fe0973638142) )
	ROM_LOAD32_WORD_SWAP( "csz1pt0l.7c",  0x0000002, 0x400000, CRC(b0d66afe) SHA1(7cda4eebf1bb1191d17e4b5e616be2fbe4ae9328) )
	ROM_LOAD32_WORD_SWAP( "csz1pt1h.5a",  0x0800000, 0x400000, CRC(e54f80ad) SHA1(3b3fbb3001e630d800b02ec8e653d74878ac5116) )
	ROM_LOAD32_WORD_SWAP( "csz1pt1l.5c",  0x0800002, 0x400000, CRC(527171c8) SHA1(0b2ce3858f40bdedf1543309a6bc28d780415250) )
	ROM_LOAD32_WORD_SWAP( "csz1pt2h.4a",  0x1000000, 0x400000, CRC(e295137a) SHA1(37b18af1b3d9f0e69b45135f89b49a1ceec79127) )
	ROM_LOAD32_WORD_SWAP( "csz1pt2l.4c",  0x1000002, 0x400000, CRC(c87d6dbd) SHA1(686f39073c521d6b21ef8bc1161b41b680697c63) )
	ROM_LOAD32_WORD_SWAP( "csz1pt3h.3a",  0x1800000, 0x400000, CRC(05f65bdf) SHA1(0c349fe5381fe7aeb81f9365a2b44a212f6bd33e) )
	ROM_LOAD32_WORD_SWAP( "csz1pt3l.3c",  0x1800002, 0x400000, CRC(5d077c0f) SHA1(a4fd0167d89bf9417766405726e0334e7c7eaec3) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
	ROM_LOAD( "csz1wavel.2c", 0x000000, 0x800000, CRC(d0d74132) SHA1(a293d93bca8e12e388a088a592cfa7bcb9a976f7) )
	ROM_LOAD( "csz1waveh.2a", 0x800000, 0x800000, CRC(de9d14a8) SHA1(e5006861928bb1d29bf80c7304f1a6d044b094fd) )

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate ROMs */
	ROM_LOAD( "csz1cguu.4f",  0x000000, 0x800000, CRC(e1d1bf24) SHA1(daf2c68e2d9a8f313d262d221cc990c93dfdf22f) )
	ROM_LOAD( "csz1cgum.5j",  0x000000, 0x800000, CRC(913c98b5) SHA1(b952dbc19053796077d4f33e8da836893e933b12) )
	ROM_LOAD( "csz1cgll.5m",  0x000000, 0x800000, CRC(0bcd41f2) SHA1(80b74f9398e8bd074f79a14490d06cfeb875c874) )
	ROM_LOAD( "csz1cglm.5k",  0x000000, 0x800000, CRC(d4af93d1) SHA1(0df37b793ce8da02d14f714722382786ae5d3ce2) )
	ROM_LOAD( "csz1ccrl.7m",  0x000000, 0x400000, CRC(1c20768d) SHA1(6cf4280e26f3625d6f750837bf344163e7e93c3d) )
	ROM_LOAD( "csz1ccrh.7k",  0x000000, 0x200000, CRC(bc2fa03c) SHA1(e63d8e75494a383bf9a213edfa9c472a010f8efe) )
ROM_END

} // anonymous namespace


/* Games */
#define GAME_FLAGS ( MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
//    YEAR, NAME,        PARENT,   MACHINE,     INPUT,     CLASS,                INIT,        MNTR, COMPANY, FULLNAME,                                   FLAGS
GAME( 1997, rapidrvr,    0,        rapidrvr,    rapidrvr,  rapidrvr_state,       empty_init,  ROT0, "Namco", "Rapid River (US, RD3 Ver. C)",             GAME_FLAGS ) // 97/11/27, USA
GAME( 1997, rapidrvrv2c, rapidrvr, rapidrvr,    rapidrvr,  rapidrvr_state,       empty_init,  ROT0, "Namco", "Rapid River (World, RD2 Ver. C)",          GAME_FLAGS ) // 97/11/27, Europe
GAME( 1997, rapidrvrp,   rapidrvr, rapidrvr,    rapidrvrp, rapidrvr_state,       empty_init,  ROT0, "Namco", "Rapid River (prototype)",                  GAME_FLAGS ) // 97/11/10, USA
GAME( 1997, finfurl,     0,        finfurl,     finfurl,   gorgon_state,         empty_init,  ROT0, "Namco", "Final Furlong (World, FF2 Ver. A)",        GAME_FLAGS | MACHINE_NODEVICE_LAN )
GAME( 1997, downhill,    0,        downhill,    downhill,  namcos23_state,       empty_init,  ROT0, "Namco", "Downhill Bikers (World, DH2 Ver. A)",      GAME_FLAGS | MACHINE_NODEVICE_LAN )
GAME( 1997, downhillu,   downhill, downhill,    downhill,  namcos23_state,       empty_init,  ROT0, "Namco", "Downhill Bikers (US, DH3 Ver. A)",         GAME_FLAGS | MACHINE_NODEVICE_LAN )
GAME( 1997, motoxgo,     0,        motoxgo,     motoxgo,   motoxgo_state,        empty_init,  ROT0, "Namco", "Motocross Go! (US, MG3 Ver. A)",           GAME_FLAGS | MACHINE_NODEVICE_LAN )
GAME( 1997, motoxgov2a,  motoxgo,  motoxgo,     motoxgo,   motoxgo_state,        empty_init,  ROT0, "Namco", "Motocross Go! (World, MG2 Ver. A, set 1)", GAME_FLAGS | MACHINE_NODEVICE_LAN )
GAME( 1997, motoxgov2a2, motoxgo,  motoxgo,     motoxgo,   motoxgo_state,        empty_init,  ROT0, "Namco", "Motocross Go! (World, MG2 Ver. A, set 2)", GAME_FLAGS | MACHINE_NODEVICE_LAN )
GAME( 1997, motoxgov1a,  motoxgo,  motoxgo,     motoxgo,   motoxgo_state,        empty_init,  ROT0, "Namco", "Motocross Go! (Japan, MG1 Ver. A, set 1)", GAME_FLAGS | MACHINE_NODEVICE_LAN )
GAME( 1997, motoxgov1a2, motoxgo,  motoxgo,     motoxgo,   motoxgo_state,        empty_init,  ROT0, "Namco", "Motocross Go! (Japan, MG1 Ver. A, set 2)", GAME_FLAGS | MACHINE_NODEVICE_LAN )
GAME( 1997, timecrs2,    0,        timecrs2,    timecrs2,  namcos23_state,       empty_init,  ROT0, "Namco", "Time Crisis II (US, TSS3 Ver. B)",         GAME_FLAGS | MACHINE_NODEVICE_LAN )
GAME( 1997, timecrs2v2b, timecrs2, timecrs2,    timecrs2,  namcos23_state,       empty_init,  ROT0, "Namco", "Time Crisis II (World, TSS2 Ver. B)",      GAME_FLAGS | MACHINE_NODEVICE_LAN )
GAME( 1997, timecrs2v1b, timecrs2, timecrs2,    timecrs2,  namcos23_state,       empty_init,  ROT0, "Namco", "Time Crisis II (Japan, TSS1 Ver. B)",      GAME_FLAGS | MACHINE_NODEVICE_LAN )
GAME( 1997, timecrs2v4a, timecrs2, timecrs2v4a, timecrs2,  namcoss23_state,      empty_init,  ROT0, "Namco", "Time Crisis II (World, TSS4 Ver. A)",      GAME_FLAGS | MACHINE_NODEVICE_LAN )
GAME( 1997, timecrs2v5a, timecrs2, timecrs2v4a, timecrs2,  namcoss23_state,      empty_init,  ROT0, "Namco", "Time Crisis II (US, TSS5 Ver. A)",         GAME_FLAGS | MACHINE_NODEVICE_LAN )
GAME( 1997, panicprk,    0,        panicprk,    panicprk,  namcos23_state,       empty_init,  ROT0, "Namco", "Panic Park (World, PNP2 Ver. A)",          GAME_FLAGS )
GAME( 1997, panicprkj,   panicprk, panicprk,    panicprk,  namcos23_state,       empty_init,  ROT0, "Namco", "Panic Park (Japan, PNP1 Ver. B, set 1)",   GAME_FLAGS )
GAME( 1997, panicprkj2,  panicprk, panicprk,    panicprk,  namcos23_state,       empty_init,  ROT0, "Namco", "Panic Park (Japan, PNP1 Ver. B, set 2)",   GAME_FLAGS )
GAME( 1998, gunwars,     0,        gunwars,     gunwars,   namcoss23_gmen_state, empty_init,  ROT0, "Namco", "Gunmen Wars (Japan, GM1 Ver. B)",          GAME_FLAGS | MACHINE_NODEVICE_LAN )
GAME( 1998, gunwarsa,    gunwars,  gunwars,     gunwars,   namcoss23_gmen_state, empty_init,  ROT0, "Namco", "Gunmen Wars (Japan, GM1 Ver. A)",          GAME_FLAGS | MACHINE_NODEVICE_LAN )
GAME( 1998, raceon,      0,        raceon,      raceon,    namcoss23_gmen_state, empty_init,  ROT0, "Namco", "Race On! (World, RO2 Ver. A)",             GAME_FLAGS | MACHINE_NODEVICE_LAN )
GAME( 1998, raceonj,     raceon,   raceon,      raceon,    namcoss23_gmen_state, empty_init,  ROT0, "Namco", "Race On! (Japan, RO1 Ver. B)",             GAME_FLAGS | MACHINE_NODEVICE_LAN )
GAME( 1998, 500gp,       0,        _500gp,      500gp,     namcoss23_state,      empty_init,  ROT0, "Namco", "500 GP (US, 5GP3 Ver. C)",                 GAME_FLAGS | MACHINE_NODEVICE_LAN )
GAME( 1998, aking,       0,        aking,       aking,     namcoss23_state,      empty_init,  ROT0, "Namco", "Angler King (Japan, AG1 Ver. A)",          GAME_FLAGS )
GAME( 1998, finfurl2,    0,        finfurl2,    finfurl2,  namcoss23_gmen_state, empty_init,  ROT0, "Namco", "Final Furlong 2 (World)",                  GAME_FLAGS | MACHINE_NODEVICE_LAN ) // 99/02/26  15:08:47 Overseas
GAME( 1998, finfurl2j,   finfurl2, finfurl2,    finfurl2,  namcoss23_gmen_state, empty_init,  ROT0, "Namco", "Final Furlong 2 (Japan, FFS1 Ver.A)",      GAME_FLAGS | MACHINE_NODEVICE_LAN ) // 99/02/26  15:03:14 Japanese
GAME( 1999, crszone,     0,        crszone,     crszone,   crszone_state,        empty_init,  ROT0, "Namco", "Crisis Zone (World, CSZO4 Ver. B)",        GAME_FLAGS )
GAME( 1999, crszonev4a,  crszone,  crszone,     crszone,   crszone_state,        empty_init,  ROT0, "Namco", "Crisis Zone (World, CSZO4 Ver. A)",        GAME_FLAGS )
GAME( 1999, crszonev3b,  crszone,  crszone,     crszone,   crszone_state,        empty_init,  ROT0, "Namco", "Crisis Zone (US, CSZO3 Ver. B, set 1)",    GAME_FLAGS )
GAME( 1999, crszonev3b2, crszone,  crszone,     crszone,   crszone_state,        empty_init,  ROT0, "Namco", "Crisis Zone (US, CSZO3 Ver. B, set 2)",    GAME_FLAGS )
GAME( 1999, crszonev3a,  crszone,  crszone,     crszone,   crszone_state,        empty_init,  ROT0, "Namco", "Crisis Zone (US, CSZO3 Ver. A)",           GAME_FLAGS )
GAME( 1999, crszonev2a,  crszone,  crszone,     crszone,   crszone_state,        empty_init,  ROT0, "Namco", "Crisis Zone (World, CSZO2 Ver. A)",        GAME_FLAGS )
GAME( 1999, crszonev2b,  crszone,  crszone,     crszone,   crszone_state,        empty_init,  ROT0, "Namco", "Crisis Zone (World, CSZO2 Ver. B)",        GAME_FLAGS )
