// license:BSD-3-Clause
// copyright-holders:R. Belmont, Phil Stroffolino, Olivier Galibert
/*
    Namco System 22.5 and (Super) System 23 (Evolution 2)
    Extremely preliminary driver by R. Belmont, thanks to Phil Stroffolino & Olivier Galibert

    Hardware: * R4650 (MIPS III with IDT special instructions) main CPU.
                133 MHz for Gorgon, 166 MHz for System 23 and Super System 23, and
                200 MHz for Super System 23 Evolution 2.
              * H8/3002 MCU for sound/inputs
              * Custom polygon hardware
              * 1 text tilemap

    Gorgon and System 23 use an I/O board based on the Namco C78, which is a Renesas H8/3334 MCU
    (8-bit version of the H8/3002).

    Super System 23 uses a PIC16Cxx-based I/O board.  In both cases the I/O boards' MCUs apparently are connected
    to the H8/3002's serial port, similar to System 22 where one 37702 reads the I/O and communicates serially
    with the second 37702 which is the traditional "subcpu".

    NOTES:
    - First 128k of main program ROM is the BIOS, and after that is a 64-bit MIPS ELF image.
    - Text layer is identical to System 22 & Super System 22.

    TODO:
    - There are currently no differences seen between System 23 (Time Crisis 2) and
      Super System 23 (GP500, Final Furlong 2).  These will presumably appear when
      the 3D hardware is emulated.

    - Serial number data is at offset 0x201 in the BIOS.  Until the games are running
      and displaying it I'm not going to meddle with it though.

    - Add the sh2 in Gunmen Wars (no rom, controls the camera)

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


****************************************************************************

Namco System 23 and Super System 23 Hardware Overview (last updated 7th April 2013 at 12.49am)
Namco, 1997 - 2000

Note! This document is a Work-In-Progress and will be updated from time to time when more games are available.

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
Final Furlong 2   Namco, 1999    Super System 23
*Guitar Jam       Namco, 1999    Super System 23
Crisis Zone       Namco, 2000    System 23 Evolution 2

* - Guitar Jam is not dumped yet and the hardware type is not confirmed. It might not be on System 23 hardware.

A System 23 unit is comprised of some of the following pieces....
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
- V185 I/O PCB                     Gun I/O board used with Time Crisis II
- V221 MIU PCB                     Gun I/O board used with Crisis Zone (System 23 Evolution 2) and Time Crisis 3 (on System 246)
- SYSTEM23 MEM(M) PCB              Holds MASKROMs for GFX/Sound and associated logic
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
can be configured for either 32MBit or 64MBit SOP44 MASK ROMs with a maximum capacity of 1664MBits.
The system also uses a dual pipeline graphics bus similar to Super System 22 and has two copies of the graphics ROMs
on the PCB.
The System 23 hardware is the first NAMCO system to require an external 3.3V power source. Previously the 3.3V
was derived from a 5V to 3.3V regulator on systems such as System10/11/12 etc.
The KEYCUS chip is the familiar MACH211 PLCC44 IC as used on System12. The sound system is also taken from System12.

On bootup, the following happens (on GP500)...

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
                    C444 (QFP136)
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
                    C444 (QFP136)
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
      VXP3220A   - Micronis Intermetall VXP3220A Video Pixel Decoder IC (PLCC44)
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
      GP500            5GP3 Ver.C (for Super System 23)
      Time Crisis 2    TSS4 Ver.A (for Super System 23)
      Final Furlong 2  FFS1 Ver.? (for Super System 23)
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

FCA PCB  8662969102 (8662979102)
(Used with 500GP and Angler King. Another identical board is used with Ridge Racer V on System 246)
|---------------------------------------------------|
| J101                J106                          |
|            4.9152MHz                              |
|    DSW(6)                                         |
| LED2              |-----|                         |
|                   | MCU |                         |
|     LEDS3-10      |     |                         |
|  PIC16F84         |-----|                         |
|   JP1 LED1                           ADM485       |
|                                                   |
|                     J102              J104        |
|---------------------------------------------------|
Notes:
      J101     - 6 pin connector for power input
      J102     - 60 pin flat cable connector
      J104     - 5 pin connector
      J106     - 30 pin flat cable connector
      JP1      - 3 pin jumper, set to 'NORM'. Alt setting 'WR'
      3771     - Fujitsu MB3771 System Reset IC (SOIC8)
      PIC16F84 - Microchip PIC16F84 PIC (SOIC20)
                  - For 500GP and Angler King stamped 'CAP10'
                  - For Ridge Racer V (on System 246) stamped 'CAP11'
      MCU      - Fujitsu MB90F574 Microcontroller (QFP120)
                  - For 500 GP and Angler King stamped 'FCAF10'
                  - For Ridge Racer V (on System 246) stamped 'FCAF11'
      ADM485   - Analog Devices ADM485 +5V Low Power EIA RS-485 Transceiver (SOIC8)


ASCA-3A PCB 8662968301 (8662978301)
This is used with Motocross Go!
It's identical to the one shown below (ASCA-4A) but the ROM is a surface mounted
PLCC32 instead of a socketed DIP32 chip. Everything else is identical.

ASCA-4A PCB  8662968401 (8662978401)
(Used with most games and [for testing purposes] is able to boot all S23/SS23/Evolution2 games)
Sticker for Gunmen Wars: ASCA5 PCB 86629615
|---------------------------------------------------|
|                           PST592           62256  |
|                     14.7460MHz  |-------|         |
|                                 |  C78  |         |
|         LB1235       DSW(4)     |       | 27C1001 |
| LB1233  LB1235                  |-------|         |
|                                                   |
|                |------|            ADM485         |
|                |ALTERA|                           |
|                |EPM7096                           |
|      J101      |------|          J102  J103       |
|---------------------------------------------------|
Notes:
      J101     - 64 pin connector for power + inputs. This joins to another PCB at 90 degrees containing
                 a bunch of connectors named 'ASCA 4B PCB 8662968702 (8662978702)'. This is where ALL of the
                 inputs/outputs for game control and inter-PCB communication are connected
      PST592   - System Reset IC (SOIC4)
      C78      - Hitachi HD643334 H8/3334 Microcontroller rebadged as 'C78'. Clock input 14.746MHz (PLCC84)
      27C1001  - 128k x8 EPROM (DIP32)
                  - For Downhill Bikers labelled 'ASC3 IO-C'
                  - For Panic Park labelled 'ASC3 IO-C'
                  - For Race On! labelled 'ASC5 IO-A'
                  - For Gunmen Wars labelled 'ASC5 IO-A'
      62256    - 32k x8 SRAM (SOP28)
      EPM7096  - Altera EPM7096 CPLD with sticker 'ASCA,DR1' (PLCC68)
      ADM485   - Analog Devices ADM485 +5V Low Power EIA RS-485 Transceiver (SOIC8)
      J102/J103- Standard USB A and B connectors. These are not populated on most games, but are populated for
                 use with Motocross Go! on the ASCA-3A PCB.


I/O Boards for gun games
------------------------

Type 1:

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
      SLA4060   - Sanken Electric SLA4060 NPN general purpose darlington transistor (used to drive the kick-back solenoid in the gun)
      PST592    - System Reset IC (SOIC4)
      J1        - 12 position connector for power and I/O communication
      J3        - 12 position connector for gun connection (trigger/buttons/optical signal/power)
      J4        - not used?
      J5        - 6 position connector for network
      J601      - not used?
      JP1       - jumper set to 1-2 (lower position), labelled 'WR'
      DSW       - 4 position dipswitch block, all off

This board is used only on Time Crisis II.
Note the gun is a standard light gun.


Type 2:

V221 MIU PCB
2512960101 (2512970101)
additional sticker for Time Crisis 3 says '2591961001 V291 XMIU PCB'
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
      2267    - JRC2267 Current limiting diode array? (SOIC8)
      R305526 - Some kind of mini transformer or regulator?
      LC35256 - Sanyo LC35256 32k x8 SRAM (SOP28)
      LM1881  - National Semiconductor LM1881 Video Sync Separator (SOIC8)
      M0105   - Matsushita Panasonic 0105 = ? (SOIC16)
      T082    - Texas Instruments T082 (=TL082) JFET-Input operational amplifier (SOIC8)
      6393    - Sanyo 6393 (LA6393) High Performance Dual Comparator (SOIC8)
      ADM485  - Analog Devices ADM485 5V Low Power EIA RS-485 Transceiver (SOIC8)
      3771    - Fujitsu MB3771 Power Supply Monitor and Master Reset IC (SOIC8)
      EPM7128 - Altera MAX EPM7128SLC84-15 PLD labelled 'TMIU1/PLD0' (Time Crisis 3)
      29C020  - location for 29C020 PLCC32 Flash/EP ROM (not populated)
      ZUW1R51212 - Cosel ZUW1R51212 DC to DC Power Supply Module (input 9-18VDC, output +-12VDC or +24VDC)
      DSW     - 4 position dipswitch block, all off
      J1      - 6-pin power input connector
      J2      - 12-pin connector (cabinet buttons UP/DOWN/ENTER plus TEST/SERVICE/COIN etc)
      J3      - 4 pin connector (not used)
      J4      - 9 pin Namco female plug connector for gun (solenoid +24V/trigger/pedal/sensor)
      J5      - 5 pin connector used for I/O --> S246 communications (connects to USB link connection on main unit)
      J6      - 7-pin connector (not used)
      J9      - 6-pin connector (not used)
      J10     - 2-pin Namco female plug connector (not used)
      J11     - 6-pin Namco female plug connector (video input from CCD camera)
      PRG.8F  - 27C1001 EPROM with label...
                                           'XMIU1 PRG0' (I/O program for Time Crisis 3)
                                           'CSZ1 PRG0A' (I/O program for Crisis Zone)

This board is used on Crisis Zone (System 23 Evolution2) and Time Crisis 3 (System 246)
Note both games use a CCD camera for the gun sensor.


Drive/Feedback PCB
------------------

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


Namco Gorgon-based games
------------------------

Rapid River, Final Furlong (both Namco, 1997)

These games run on hardware called "GORGON". It appears to have similar
capabilities to System 23 but like Super System 22 has sprites also.
(System 23 doesn't have sprites). The PCBs are about two times larger
than System 23.

The system comprises Main PCB, ROM PCB and I/O PCB all located inside
a metal box with 3 separate power supplies for 5V, 12V and 3.3V. Main
input power is 115V.
Rapid River is controlled by rotating a paddle (for thrust) and turning it
sideways (moves left/right).
The rotation action is done with a 5K potentiometer whereby the thrust
is achieved by moving the pot from full left to full right continuously.
The left/right turning movement is just another 5K potentiometer connected
to the column of the paddle center shaft.
There are also some buttons just for test mode, including SELECT, UP & DOWN
The player's seat has movement controlled by a compressor and several
potentiometers. On bootup, the system tests the seat movement and displays
a warning if it's not working. Pressing START allows the game to continue
and function normally without the seat movement.

The other game on this system (Final Furlong) and is a horse racing game.
To control the horse you rock it forwards and backwards continually (it's very tiring
to play this game).
This would activate possibly one or two 5K potentiometers inside the horse body.
Just like a real horse you need to control the speed so your horse lasts the entire race.
If you rock too much, a message on screen says 'Too Fast'. To steer the horse turn the
head sideways using the reins. There would be another 5K potentiometer in the head
to activate the turning direction.


Main PCB
--------

8664960102 (8664970102) GORGON MAIN PCB
|------------------------------------------------------------------------------------------------------|
|                                   J4                       J5                         J6             |
|                              |---------|           |---------| |------| |---------|                  |
|         |---------| |------| |         |           |         | |C401  | |         |HM534251 HM534251 |
| CXD1178Q|         | |C381  | |  C374   |  |------| |  C417   | |      | |  304    |HM534251 HM534251 |
|         |  C404   | |      | |         |  |C435  | |         | |------| |         |HM534251 HM534251 |
|         |         | |------| |         |  |      | |         | |------| |         |                  |
|         |         |          |---------|  |------| |---------| |C400  | |---------|                  |
|         |---------|     |---------|       |------|             |      | |---------|                  |
|                         |         |       |C435  |    341256   |------| |         |HM534251 HM534251 |
|                         |  C397   |       |      |             |------| |  304    |HM534251 HM534251 |
|  341256 341256  341256  |         |       |------|    341256   |C401  | |         |HM534251 HM534251 |
|  M5M51008       341256  |         |     |---------|            |      | |         |                  |
|                         |---------|     |         | |------|   |------| |---------|                  |
|  M5M51008       341256         |------| |  C403   | |C406  |   |------| |---------|                  |
|ADM485              |---------| |C379  | |         | |      |   |C400  | |         |HM534251 HM534251 |
|                    |         | |      | |         | |------|   |      | |  304    |HM534251 HM534251 |
|    M5M51008        |  C300   | |------| |---------|            |------| |         |HM534251 HM534251 |
|                    |         | LH540204  LH540204              |------| |         |                  |
|    M5M51008        |         |341256                 |------|  |C401  | |---------|                  |
|J1   HCPL0611       |---------|341256                 |C407  |  |      | |---------|                  |
|         DS8921                  PST575  PST575       |      |  |------| |         |                  |
|  DS8921                                              |------|  |------| |  304    |HM534251 HM534251 |
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
|                |3002 |               |      |LC321664                       |         | |         |  |
|  J10           |-----|               |------|    J8                         |---------| |---------|  |
|------------------------------------------------------------------------------------------------------|
Notes:
     NKK NR4650 - R4600-based 64bit RISC CPU (Main CPU, QFP208, clock input source = CY2291S)
     H8/3002  - Hitachi H8/3002 HD6413002F17 (Sound CPU, QFP100, running at 14.7456MHz)
     EPM7128  - Altera EPM7128 FPGA labelled 'GOR-M1' (PLCC84)
     PAL      - PALCE16V8H stamped 'GOR-M3' (PLCC20)
     HM534251 - Hitachi HM534251 256k x4 Dynamic Video RAM (SOJ28)
     N341256  - NKK 32k x8 SRAM (SOJ28)
     M5M5256  - Mitsubishi 32k x8 SRAM (SOP28)
     D4516161 - NEC uPD4516161AG5-A80 1M x16 (16MBit) SDRAM (SSOP50)
     LC321664 - Sanyo 64k x16 EDO DRAM (SOJ40)
     M5M51008 - Mitsubishi 128k x8 SRAM (SOP32)
     CY7C128  - Cypress 2k x8 SRAM (SOJ28)
     LH540204 - Sharp CMOS 4096 x 9 Asynchronous FIFO (PLCC32)
     2061ASC-1- IC Designs 2061ASC-1 programmable clock generator (SOIC16)
     DS8921   - Dallas Semiconductor DS8921 RS-422/423 Differential Line Driver and Receiver Pair (SOIC8)
     HCPL0611 - Fairchild HCPL0611 High Speed 10MBits/sec Logic Gate Optocoupler (SOIC8)
     ADM485   - Analog Devices ADM485 5V Low Power EIA RS-485 Transceiver (SOIC8)
     PST575   - System Reset IC (SOIC8)
     CXD1178Q - Sony CXD1178Q 8-bit RGB 3-channel D/A converter (QFP48)
     J1       - 64 pin connector for connection of I/O board
     J4/J5/J6 \
     J8/J9    / Custom NAMCO connectors for connection of MEM(M1) PCB
     J10      - Custom NAMCO connector for MSPM(FR) PCB


     Namco Custom ICs
     ----------------
     C300 (QFP160)
     304  (x4, QFP120)
     C352 (QFP100)
     C361 (QFP120)
     C374 (QFP160)
     C379 (QFP64)
     C381 (QFP144)
     C397 (QFP160)
     C399 (QFP160)
     C400 (QFP100)
     C401 (x4, QFP64)
     C403 (QFP136)
     C404 (QFP208)
     C406 (QFP120)
     C407 (QFP64)
     C413 (QFP208)
     C416 (QFP176)
     C417 (QFP208)
     C422 (QFP64)
     C435 (x2, TQFP144)


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
     J1 -  Connector to plug into Main PCB
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
          PT*  - Point ROMs, sizes configurable to either 16M or 32M (SOP44)
          MT*  - Motion ROMs, sizes configurable to either 32M or 64M (SOP44)
          CG*  - Texture ROMs, sizes configurable to either 32M or 64M (SOP44)
          CCR* - Texture Tilemap ROMs, sizes fixed at 16M (SOP44)
          SPR* - Sprite ROMs, sizes configurable to either 32M or 64M (SOP44)
          WAVE*- Wave ROMs, sizes configurable to either 32M or 64M (SOP44)

I/O PCB
-------

V187 ASCA-2A PCB
2477960102 (2477970102)
|--------------------------------------------------------|
|                   J105                                 |
|                           |-------|        14.7456MHz  |
|   J104                    |ALTERA |    ADM485   PST592 |
|                           |EPM7064|     |-------|      |
|                           |       |     |       |      |
|                           |-------|     | C78   |      |
|     LC78815                             |       |      |
|                                         |-------|      |
|     MB87078                              |---|         |
| LA4705                                   |IC1| 62256   |
|                                          |---|         |
|         J101                J102                       |
|--------------------------------------------------------|
Notes:
     IC1  - Atmel AT29C020 2MBit EEPROM labelled 'ASCA1 I/O-A' (PLCC32)
     C78  - Namco Custom MCU, positively identified as a Hitachi H8/3334 (PLCC84)
     EPM7064 - Altera EPM7064LC68-15 PLD, labelled 'ASCA DR0' (PLCC68)
     PST592 - System Reset IC (SOIC4)
     ADM485 - Analog Devices +ADM485 5V Low Power EIA RS-485 Transceiver (SOIC8)
     MB87078 - Fujitsu MB87078 Electronic Volume Control IC (SOIC24)
     LC78815 - Sanyo LM78815 2-Channel 16-Bit D/A Converter (SOIC20)
     J101 - 34 pin flat cable connector for filter board
     J102 - 50 pin flat cable connector for filter board
     J104 - 8 pin power connector (+5V, +12V, +3.3V)
     J105 - 64 pin connector for connection of Main PCB

*/

#include "emu.h"
#include <float.h>
#include "video/poly.h"
#include "cpu/mips/mips3.h"
#include "cpu/h8/h83002.h"
#include "cpu/h8/h83337.h"
#include "cpu/sh2/sh2.h"
#include "sound/c352.h"
#include "machine/nvram.h"
#include "machine/rtc4543.h"
#include "machine/namco_settings.h"

#define JVSCLOCK    (XTAL_14_7456MHz)
#define H8CLOCK     (16737350)      /* from 2061 */
#define BUSCLOCK    (16737350*2)    /* 33MHz CPU bus clock / input */
#define C352CLOCK   (25992000)  /* measured at 25.992MHz from 2061 pin 9  */
#define C352DIV     (296)
#define VSYNC1      (59.8824)
#define VSYNC2      (59.915)
#define HSYNC       (16666150)
#define MODECLOCK   (130205)

#define MAIN_VBLANK_IRQ 0x01
#define MAIN_C361_IRQ   0x02
#define MAIN_SUBCPU_IRQ 0x04
#define MAIN_C435_IRQ   0x08
#define MAIN_C422_IRQ   0x10
#define MAIN_C450_IRQ   0x20
#define MAIN_C451_IRQ   0x40

enum { MODEL, FLUSH };

enum { RENDER_MAX_ENTRIES = 1000, POLY_MAX_ENTRIES = 10000 };

struct namcos23_render_entry
{
	int type;

	union
	{
		struct
		{
			UINT16 model;
			INT16 m[9];
			INT32 v[3];
			float scaling;
		} model;
	};
};

struct namcos23_render_data
{
	running_machine *machine;
	const pen_t *pens;
	bitmap_rgb32 *bitmap;
	UINT32 (*texture_lookup)(running_machine &machine, const pen_t *pens, float x, float y);
};

class namcos23_state;

class namcos23_renderer : public poly_manager<float, namcos23_render_data, 4, POLY_MAX_ENTRIES>
{
public:
	namcos23_renderer(namcos23_state &state);
	void render_flush(bitmap_rgb32& bitmap);
	void render_scanline(INT32 scanline, const extent_t& extent, const namcos23_render_data& object, int threadid);
	float* zBuffer() { return m_zBuffer; }

private:
	namcos23_state& m_state;
	float* m_zBuffer;
};

typedef namcos23_renderer::vertex_t poly_vertex;

struct namcos23_poly_entry
{
	namcos23_render_data rd;
	int front;
	int vertex_count;
	float zkey;
	poly_vertex pv[16];
};


struct c417_t
{
	UINT16 ram[0x10000];
	UINT16 adr;
	UINT32 pointrom_adr;
};

struct c412_t
{
	UINT16 sdram_a[0x100000]; // Framebuffers, probably
	UINT16 sdram_b[0x100000];
	UINT16 sram[0x20000];     // Ram-based tiles for rendering
	UINT16 pczram[0x200];     // Ram-based tilemap for rendering, or something else
	UINT32 adr;
	UINT16 status_c;
};

struct c421_t
{
	UINT16 dram_a[0x40000];
	UINT16 dram_b[0x40000];
	UINT16 sram[0x8000];
	UINT32 adr;
};

struct c422_t
{
	INT16 regs[0x10];
};

struct c361_t
{
	emu_timer *timer;
	int scanline;
};

struct c404_t
{
	rgb_t bgcolor;
	UINT16 palbase;
	UINT8 layer;
};

struct render_t
{
	namcos23_renderer *polymgr;
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
	namcos23_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_adc(*this, "subcpu:adc"),
		m_iocpu(*this, "iocpu"),
		m_rtc(*this, "rtc"),
		m_settings(*this, "namco_settings"),
		m_mainram(*this, "mainram"),
		m_shared_ram(*this, "shared_ram"),
		m_gammaram(*this, "gammaram"),
		m_charram(*this, "charram"),
		m_textram(*this, "textram"),
		m_czattr(*this, "czattr"),
		m_gmen_sh2(*this, "gmen_sh2"),
		m_gmen_sh2_shared(*this, "gmen_sh2_shared"),
		m_gfxdecode(*this, "gfxdecode"),
		m_lightx(*this, "LIGHTX"),
		m_lighty(*this, "LIGHTY"),
		m_p1(*this, "P1"),
		m_p2(*this, "P2"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_generic_paletteram_32(*this, "paletteram"),
		m_adc_ports(*this, "ADC")
	{ }

	required_device<mips3_device> m_maincpu;
	required_device<h83002_device> m_subcpu;
	required_device<h8_adc_device> m_adc;
	optional_device<h83334_device> m_iocpu;
	required_device<rtc4543_device> m_rtc;
	required_device<namco_settings_device> m_settings;
	required_shared_ptr<UINT32> m_mainram;
	required_shared_ptr<UINT32> m_shared_ram;
	required_shared_ptr<UINT32> m_gammaram;
	required_shared_ptr<UINT32> m_charram;
	required_shared_ptr<UINT32> m_textram;
	optional_shared_ptr<UINT32> m_czattr;
	optional_device<cpu_device> m_gmen_sh2;
	optional_shared_ptr<UINT32> m_gmen_sh2_shared;
	required_device<gfxdecode_device> m_gfxdecode;
	optional_ioport m_lightx;
	optional_ioport m_lighty;
	required_ioport m_p1;
	required_ioport m_p2;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<UINT32> m_generic_paletteram_32;
	optional_ioport_array<4> m_adc_ports;

	c404_t m_c404;
	c361_t m_c361;
	c417_t m_c417;
	c412_t m_c412;
	c421_t m_c421;
	c422_t m_c422;
	render_t m_render;

	tilemap_t *m_bgtilemap;
	UINT8 m_jvssense;
	INT32 m_has_jvsio;
	UINT32 m_main_irqcause;
	bool m_ctl_vbl_active;
	UINT8 m_ctl_led;
	UINT16 m_ctl_inp_buffer[2];
	bool m_subcpu_running;
	UINT32 m_c435_address;
	UINT32 m_c435_size;
	const UINT32 *m_ptrom;
	const UINT16 *m_tmlrom;
	const UINT8 *m_tmhrom;
	const UINT8 *m_texrom;
	UINT32 m_tileid_mask;
	UINT32 m_tile_mask;
	UINT32 m_ptrom_limit;

	int m_vblank_count;

// It may only be 128
// At 0x1e bytes per slot, rounded up to 0x20, that's 0x1000 to 0x2000 bytes.
// That fits pretty much anywhere, including inside a IC
// No idea at that point if it's CPU-reachable.  DMA's probably more efficient anyway.

// Matrices are stored in signed 2.14 fixed point
// Vectors are stored in signed 10.14 fixed point

	INT16 m_matrices[256][9];
	INT32 m_vectors[256][3];
	INT32 m_light_vector[3];
	UINT16 m_scaling;
	INT32 m_spv[3];
	INT16 m_spm[3];

	UINT16 m_c435_buffer[256];
	int m_c435_buffer_pos;

	UINT8 m_sub_porta;
	UINT8 m_sub_portb;
	UINT8 m_tssio_port_4;

	void update_main_interrupts(UINT32 cause);
	void update_mixer();

	DECLARE_WRITE32_MEMBER(textram_w);
	DECLARE_WRITE32_MEMBER(textchar_w);
	DECLARE_WRITE32_MEMBER(paletteram_w);
	DECLARE_READ16_MEMBER(c417_r);
	DECLARE_WRITE16_MEMBER(c417_w);
	DECLARE_READ16_MEMBER(c412_ram_r);
	DECLARE_WRITE16_MEMBER(c412_ram_w);
	DECLARE_READ16_MEMBER(c412_r);
	DECLARE_WRITE16_MEMBER(c412_w);
	DECLARE_READ16_MEMBER(c421_ram_r);
	DECLARE_WRITE16_MEMBER(c421_ram_w);
	DECLARE_READ16_MEMBER(c421_r);
	DECLARE_WRITE16_MEMBER(c421_w);
	DECLARE_WRITE16_MEMBER(ctl_w);
	DECLARE_READ16_MEMBER(ctl_r);
	DECLARE_WRITE16_MEMBER(c361_w);
	DECLARE_READ16_MEMBER(c361_r);
	DECLARE_READ16_MEMBER(c422_r);
	DECLARE_WRITE16_MEMBER(c422_w);
	DECLARE_WRITE16_MEMBER(mcuen_w);
	DECLARE_READ16_MEMBER(sub_comm_r);
	DECLARE_WRITE16_MEMBER(sub_comm_w);
	DECLARE_READ32_MEMBER(c435_r);
	DECLARE_WRITE32_MEMBER(c435_w);
	DECLARE_READ32_MEMBER(gmen_trigger_sh2);
	DECLARE_READ32_MEMBER(sh2_shared_r);
	DECLARE_WRITE32_MEMBER(sh2_shared_w);
	DECLARE_WRITE16_MEMBER(sharedram_sub_w);
	DECLARE_READ16_MEMBER(sharedram_sub_r);
	DECLARE_WRITE16_MEMBER(sub_interrupt_main_w);
	DECLARE_READ16_MEMBER(mcu_p8_r);
	DECLARE_WRITE16_MEMBER(mcu_p8_w);
	DECLARE_READ16_MEMBER(mcu_pa_r);
	DECLARE_WRITE16_MEMBER(mcu_pa_w);
	DECLARE_READ16_MEMBER(mcu_rtc_r);
	DECLARE_READ16_MEMBER(mcu_pb_r);
	DECLARE_WRITE16_MEMBER(mcu_pb_w);
	DECLARE_READ16_MEMBER(mcu_p6_r);
	DECLARE_WRITE16_MEMBER(mcu_p6_w);
	DECLARE_READ16_MEMBER(iob_p4_r);
	DECLARE_WRITE16_MEMBER(iob_p4_w);
	DECLARE_READ16_MEMBER(iob_p6_r);
	DECLARE_WRITE16_MEMBER(iob_p6_w);
	DECLARE_READ8_MEMBER(iob_gun_r);
	DECLARE_READ16_MEMBER(iob_analog_r);
	DECLARE_DRIVER_INIT(s23);
	TILE_GET_INFO_MEMBER(TextTilemapGetInfo);
	DECLARE_VIDEO_START(s23);
	DECLARE_MACHINE_RESET(gmen);
	virtual void machine_start();
	virtual void machine_reset();
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(interrupt);
	TIMER_CALLBACK_MEMBER(c361_timer_cb);
	void sub_irq(screen_device &screen, bool vblank_state);
	UINT8 nthbyte(const UINT32 *pSource, int offs);
	UINT16 nthword(const UINT32 *pSource, int offs);
	inline INT32 u32_to_s24(UINT32 v);
	inline INT32 u32_to_s10(UINT32 v);
	INT32 *c435_getv(UINT16 id);
	INT16 *c435_getm(UINT16 id);

	void c435_matrix_matrix_mul();
	void c435_matrix_set();
	void c435_vector_set();
	void c435_matrix_vector_mul();
	void c435_vector_matrix_mul();
	void c435_scaling_set();
	void c435_state_set_interrupt();
	void c435_state_set();
	void c435_render();
	void c435_flush();

	void c435_pio_w(UINT16 data);
	void c435_dma(address_space &space, UINT32 adr, UINT32 size);

	void render_apply_transform(INT32 xi, INT32 yi, INT32 zi, const namcos23_render_entry *re, poly_vertex &pv);
	void render_apply_matrot(INT32 xi, INT32 yi, INT32 zi, const namcos23_render_entry *re, INT32 &x, INT32 &y, INT32 &z);
	void render_project(poly_vertex &pv);
	void render_one_model(const namcos23_render_entry *re);
	void render_run(bitmap_rgb32 &bitmap);
};


UINT8 namcos23_state::nthbyte(const UINT32 *pSource, int offs)
{
	pSource += offs/4;
	return (pSource[0]<<((offs&3)*8))>>24;
}

UINT16 namcos23_state::nthword(const UINT32 *pSource, int offs)
{
	pSource += offs/2;
	return (pSource[0]<<((offs&1)*16))>>16;
}





/***************************************************************************

  Video

***************************************************************************/

namcos23_renderer::namcos23_renderer(namcos23_state &state)
	: poly_manager<float, namcos23_render_data, 4, POLY_MAX_ENTRIES>(state.machine()),
	  m_state(state)
{
}

// 3D hardware, to throw at least in part in video/namcos23.c

inline INT32 namcos23_state::u32_to_s24(UINT32 v)
{
	return v & 0x800000 ? v | 0xff000000 : v & 0xffffff;
}

inline INT32 namcos23_state::u32_to_s10(UINT32 v)
{
	return v & 0x200 ? v | 0xfffffe00 : v & 0x1ff;
}


INLINE UINT8 light(UINT8 c, float l)
{
	if(l < 1)
		l = l*c;
	else
		l = 255 - (255-c)/l;
	return UINT8(l);
}

INT32 *namcos23_state::c435_getv(UINT16 id)
{
	if(id == 0x8000)
		return m_light_vector;
	if(id >= 0x100) {
		memset(m_spv, 0, sizeof(m_spv));
		return m_spv;
	}
	return m_vectors[id];
}

INT16 *namcos23_state::c435_getm(UINT16 id)
{
	if(id >= 0x100) {
		memset(m_spm, 0, sizeof(m_spm));
		return m_spm;
	}
	return m_matrices[id];
}

void namcos23_state::c435_matrix_matrix_mul() // 0.0
{
	if((m_c435_buffer[0] & 0xf) != 4) {
		logerror("WARNING: c435_matrix_matrix_mul with size %d\n", m_c435_buffer[0] & 0xf);
		return;
	}
	if(m_c435_buffer[3] != 0xffff)
		logerror("WARNING: c435_matrix_matrix_mul with +2=%04x\n", m_c435_buffer[3]);

	INT16 *t        = c435_getm(m_c435_buffer[1]);
	const INT16 *m2 = c435_getm(m_c435_buffer[2]);
	const INT16 *m1 = c435_getm(m_c435_buffer[4]);

	t[0] = INT16((m1[0]*m2[0] + m1[1]*m2[1] + m1[2]*m2[2]) >> 14);
	t[1] = INT16((m1[0]*m2[3] + m1[1]*m2[4] + m1[2]*m2[5]) >> 14);
	t[2] = INT16((m1[0]*m2[6] + m1[1]*m2[7] + m1[2]*m2[8]) >> 14);
	t[3] = INT16((m1[3]*m2[0] + m1[4]*m2[1] + m1[5]*m2[2]) >> 14);
	t[4] = INT16((m1[3]*m2[3] + m1[4]*m2[4] + m1[5]*m2[5]) >> 14);
	t[5] = INT16((m1[3]*m2[6] + m1[4]*m2[7] + m1[5]*m2[8]) >> 14);
	t[6] = INT16((m1[6]*m2[0] + m1[7]*m2[1] + m1[8]*m2[2]) >> 14);
	t[7] = INT16((m1[6]*m2[3] + m1[7]*m2[4] + m1[8]*m2[5]) >> 14);
	t[8] = INT16((m1[6]*m2[6] + m1[7]*m2[7] + m1[8]*m2[8]) >> 14);
}

void namcos23_state::c435_matrix_vector_mul() // 0.1
{
	if((m_c435_buffer[0] & 0xf) != 4) {
		logerror("WARNING: c435_matrix_vector_mul with size %d\n", m_c435_buffer[0] & 0xf);
		return;
	}

	if(m_c435_buffer[3] != 0xffff) {
		INT32 *t        = c435_getv(m_c435_buffer[1]);
		const INT16 *m  = c435_getm(m_c435_buffer[2]);
		const INT32 *vt = c435_getv(m_c435_buffer[3]);
		const INT32 *v  = c435_getv(m_c435_buffer[4]);

		t[0] = INT32((m[0]*INT64(v[0]) + m[1]*INT64(v[1]) + m[2]*INT64(v[2])) >> 14) + vt[0];
		t[1] = INT32((m[3]*INT64(v[0]) + m[4]*INT64(v[1]) + m[5]*INT64(v[2])) >> 14) + vt[1];
		t[2] = INT32((m[6]*INT64(v[0]) + m[7]*INT64(v[1]) + m[8]*INT64(v[2])) >> 14) + vt[2];

	} else {
		INT32 *t       = c435_getv(m_c435_buffer[1]);
		const INT16 *m = c435_getm(m_c435_buffer[2]);
		const INT32 *v = c435_getv(m_c435_buffer[4]);

		t[0] = INT32((m[0]*INT64(v[0]) + m[1]*INT64(v[1]) + m[2]*INT64(v[2])) >> 14);
		t[1] = INT32((m[3]*INT64(v[0]) + m[4]*INT64(v[1]) + m[5]*INT64(v[2])) >> 14);
		t[2] = INT32((m[6]*INT64(v[0]) + m[7]*INT64(v[1]) + m[8]*INT64(v[2])) >> 14);
	}
}

void namcos23_state::c435_matrix_set() // 0.4
{
	if((m_c435_buffer[0] & 0xf) != 10) {
		logerror("WARNING: c435_matrix_set with size %d\n", m_c435_buffer[0] & 0xf);
		return;
	}
	INT16 *t = c435_getm(m_c435_buffer[1]);
	for(int i=0; i<9; i++)
		t[i] = m_c435_buffer[i+2];
}

void namcos23_state::c435_vector_set() // 0.5
{
	if((m_c435_buffer[0] & 0xf) != 7) {
		logerror("WARNING: c435_vector_set with size %d\n", m_c435_buffer[0] & 0xf);
		return;
	}
	INT32 *t = c435_getv(m_c435_buffer[1]);
	for(int i=0; i<3; i++)
		t[i] = u32_to_s24((m_c435_buffer[2*i+2] << 16) | m_c435_buffer[2*i+3]);
}

void namcos23_state::c435_scaling_set() // 4.4
{
	if((m_c435_buffer[0] & 0xff) != 1) {
		logerror("WARNING: c435_scaling_set with size %d\n", m_c435_buffer[0] & 0xff);
		return;
	}
	m_scaling = m_c435_buffer[1];
}

void namcos23_state::c435_state_set_interrupt() // 4.f.0001
{
	if(m_c435_buffer[0] != 0x4f02) {
		logerror("WARNING: c435_state_set_interrupt with size %d\n", m_c435_buffer[0] & 0xff);
		return;
	}
	if(m_c435_buffer[2] & 1)
		update_main_interrupts(m_main_irqcause | MAIN_C435_IRQ);
	else
		update_main_interrupts(m_main_irqcause & ~MAIN_C435_IRQ);
}

void namcos23_state::c435_state_set() // 4.f
{
	if((m_c435_buffer[0] & 0xff) == 0) {
		logerror("WARNING: c435_state_set with size %d\n", m_c435_buffer[0] & 0xff);
		return;
	}
	switch(m_c435_buffer[1]) {
	case 0x0001: c435_state_set_interrupt(); break;
	default:
		logerror("WARNING: c435_state_set(%04x, ...)\n", m_c435_buffer[1]);
		break;
	}
}

void namcos23_state::c435_render() // 8
{
	if((m_c435_buffer[0] & 0xf) != 3) {
		logerror("WARNING: c435_render with size %d, header %04x", m_c435_buffer[0] & 0xf, m_c435_buffer[0]);
		return;
	}

	render_t &render = m_render;
	bool use_scaling = m_c435_buffer[0] & 0x0080;

	logerror("render model %x %swith matrix %x and vector %x\n", m_c435_buffer[1], use_scaling ? "scaled " : "", m_c435_buffer[2], m_c435_buffer[3]);

	if(render.count[render.cur] >= RENDER_MAX_ENTRIES) {
		logerror("WARNING: render buffer full\n");
		return;
	}

	// Vector and matrix may be inverted
	const INT16 *m = c435_getm(m_c435_buffer[2]);
	const INT32 *v = c435_getv(m_c435_buffer[3]);

	namcos23_render_entry *re = render.entries[render.cur] + render.count[render.cur];
	re->type = MODEL;
	re->model.model = m_c435_buffer[1];
	re->model.scaling = use_scaling ? m_scaling / 16384.0 : 1.0;
	memcpy(re->model.m, m, sizeof(re->model.m));
	memcpy(re->model.v, v, sizeof(re->model.v));
	if(0)
		fprintf(stderr, "Render %04x (%f %f %f %f %f %f %f %f %f) (%f %f %f)\n",
				re->model.model,
				re->model.m[0]/16384.0, re->model.m[1]/16384.0, re->model.m[2]/16384.0,
				re->model.m[3]/16384.0, re->model.m[4]/16384.0, re->model.m[5]/16384.0,
				re->model.m[6]/16384.0, re->model.m[7]/16384.0, re->model.m[8]/16384.0,
				re->model.v[0]/16384.0, re->model.v[1]/16384.0, re->model.v[2]/16384.0);

	render.count[render.cur]++;
}

void namcos23_state::c435_flush() // c
{
	if((m_c435_buffer[0] & 0xf) != 0) {
		logerror("WARNING: c435_flush with size %d\n", m_c435_buffer[0] & 0xf);
		return;
	}

	render_t &render = m_render;
	namcos23_render_entry *re = render.entries[render.cur] + render.count[render.cur];
	re->type = FLUSH;
	render.count[render.cur]++;
}


void namcos23_state::c435_pio_w(UINT16 data)
{
	m_c435_buffer[m_c435_buffer_pos++] = data;
	UINT16 h = m_c435_buffer[0];
	int psize;
	if((h & 0x4000) == 0x4000)
		psize = h & 0xff;
	else
		psize = h & 0xf;
	if(m_c435_buffer_pos < psize+1)
		return;

	bool known = true;
	switch(h & 0xc000) {
	case 0x0000:
		switch(h & 0xf0) {
		case 0x00: c435_matrix_matrix_mul(); break;
		case 0x10: c435_matrix_vector_mul(); break;
		case 0x40: c435_matrix_set(); break;
		case 0x50: c435_vector_set(); break;
		default: known = false; break;
		}
		break;

	case 0x4000:
		switch(h & 0x3f00) {
		case 0x0400: c435_scaling_set(); break;
		case 0x0f00: c435_state_set(); break;
		default: known = false; break;
		}
		break;

	case 0x8000: c435_render(); break;
	case 0xc000: c435_flush(); break;
	}

	if(!known) {
		logerror("c435 -");
		for(int i=0; i<m_c435_buffer_pos; i++)
			logerror(" %04x", m_c435_buffer[i]);
		logerror("\n");
	}

	m_c435_buffer_pos = 0;
}

void namcos23_state::c435_dma(address_space &space, UINT32 adr, UINT32 size)
{
	adr &= 0x1fffffff;

	for(int pos=0; pos < size; pos += 2)
		c435_pio_w(space.read_word(adr+pos));
}

READ32_MEMBER(namcos23_state::c435_r)
{
	switch(offset) {
	case 0xa:
		return 1; // Busy flag
	}

	logerror("c435_r %02x @ %08x (%08x, %08x)\n", offset, mem_mask, space.device().safe_pc(), (unsigned int)space.device().state().state_int(MIPS3_R31));
	return 0;
}

WRITE32_MEMBER(namcos23_state::c435_w)
{
	switch(offset) {
	case 0x7:
		COMBINE_DATA(&m_c435_address);
		break;
	case 0x8:
		COMBINE_DATA(&m_c435_size);
		break;
	case 0x9:
		if(data & 1)
			c435_dma(space, m_c435_address, m_c435_size);
		break;
	default:
		logerror("c435_w %02x, %08x @ %08x (%08x, %08x)\n", offset, data, mem_mask, space.device().safe_pc(), (unsigned int)space.device().state().state_int(MIPS3_R31));
		break;
	}
}




void namcos23_renderer::render_scanline(INT32 scanline, const extent_t& extent, const namcos23_render_data& object, int threadid)
{
	const namcos23_render_data& rd = object;

	float w = extent.param[0].start;
	float u = extent.param[1].start;
	float v = extent.param[2].start;
	float l = extent.param[3].start;
	float dw = extent.param[0].dpdx;
	float du = extent.param[1].dpdx;
	float dv = extent.param[2].dpdx;
	float dl = extent.param[3].dpdx;
	
	UINT32 *img = &object.bitmap->pix32(scanline, extent.startx);
	
	for(int x = extent.startx; x < extent.stopx; x++) {
		float z = w ? 1/w : 0;
		UINT32 pcol = rd.texture_lookup(*rd.machine, rd.pens, u*z, v*z);
		float ll = l*z;
		*img = (light(pcol >> 16, ll) << 16) | (light(pcol >> 8, ll) << 8) | light(pcol, ll);
		
		w += dw;
		u += du;
		v += dv;
		l += dl;
		img++;
	}
}

void namcos23_state::render_apply_transform(INT32 xi, INT32 yi, INT32 zi, const namcos23_render_entry *re, poly_vertex &pv)
{
	pv.x =    (INT32((re->model.m[0]*INT64(xi) + re->model.m[1]*INT64(yi) + re->model.m[2]*INT64(zi)) >> 14)*re->model.scaling + re->model.v[0])/16384.0f;
	pv.y =    (INT32((re->model.m[3]*INT64(xi) + re->model.m[4]*INT64(yi) + re->model.m[5]*INT64(zi)) >> 14)*re->model.scaling + re->model.v[1])/16384.0f;
	pv.p[0] = (INT32((re->model.m[6]*INT64(xi) + re->model.m[7]*INT64(yi) + re->model.m[8]*INT64(zi)) >> 14)*re->model.scaling + re->model.v[2])/16384.0f;
}

void namcos23_state::render_apply_matrot(INT32 xi, INT32 yi, INT32 zi, const namcos23_render_entry *re, INT32 &x, INT32 &y, INT32 &z)
{
	x = (re->model.m[0]*xi + re->model.m[3]*yi + re->model.m[6]*zi) >> 14;
	y = (re->model.m[1]*xi + re->model.m[4]*yi + re->model.m[7]*zi) >> 14;
	z = (re->model.m[2]*xi + re->model.m[5]*yi + re->model.m[8]*zi) >> 14;
}

void namcos23_state::render_project(poly_vertex &pv)
{
	// 768 validated by the title screen size on tc2:
	// texture is 640x480, x range is 3.125, y range is 2.34375, z is 3.75
	// 640/(3.125/3.75) = 768
	// 480/(2.34375/3.75) = 768

	pv.x = 320 + 768 * pv.x;
	pv.y = 240 - 768 * pv.y;
	pv.p[0] = 1.0f / pv.p[0];
}

static UINT32 render_texture_lookup_nocache_point(running_machine &machine, const pen_t *pens, float x, float y)
{
	namcos23_state *state = machine.driver_data<namcos23_state>();
	UINT32 xx = UINT32(x);
	UINT32 yy = UINT32(y);
	UINT32 tileid = ((xx >> 4) & 0xff) | ((yy << 4) & state->m_tileid_mask);
	UINT8 attr = state->m_tmhrom[tileid >> 1];
	if(tileid & 1)
		attr &= 15;
	else
		attr >>= 4;
	UINT32 tile = (state->m_tmlrom[tileid] | (attr << 16)) & state->m_tile_mask;

	// Probably swapx/swapy to add on bits 2-3 of attr
	// Bits used by motoxgo at least
	UINT8 color = state->m_texrom[(tile << 8) | ((yy << 4) & 0xf0) | (xx & 0x0f)];
	return pens[color];
}

void namcos23_state::render_one_model(const namcos23_render_entry *re)
{
	render_t &render = m_render;
	UINT32 adr = m_ptrom[re->model.model];
	if(adr >= m_ptrom_limit) {
		logerror("WARNING: model %04x base address %08x out-of-bounds - pointram?\n", re->model.model, adr);
		return;
	}

	while(adr < m_ptrom_limit) {
		poly_vertex pv[15];

		UINT32 type = m_ptrom[adr++];
		UINT32 h    = m_ptrom[adr++];

		float tbase = (type >> 24) << 12;
		UINT8 color = (h >> 24) & 0x7f;
		int lmode = (type >> 19) & 3;
		int ne = (type >> 8) & 15;

		// Something to do with Z-sorting at least?
		if(type & 0x00001000)
			adr++;

		UINT32 light = 0;
		UINT32 extptr = 0;

		if(lmode == 3) {
			extptr = adr;
			adr += ne;
		}
		else
			light = m_ptrom[adr++];

		float minz = FLT_MAX;
		float maxz = FLT_MIN;

		for(int i=0; i<ne; i++) {
			UINT32 v1 = m_ptrom[adr++];
			UINT32 v2 = m_ptrom[adr++];
			UINT32 v3 = m_ptrom[adr++];

			render_apply_transform(u32_to_s24(v1), u32_to_s24(v2), u32_to_s24(v3), re, pv[i]);
			pv[i].p[1] = (((v1 >> 20) & 0xf00) | ((v2 >> 24 & 0xff))) + 0.5;
			pv[i].p[2] = (((v1 >> 16) & 0xf00) | ((v3 >> 24 & 0xff))) + 0.5f + tbase;

			if(pv[i].p[0] > maxz)
				maxz = pv[i].p[0];
			if(pv[i].p[0] < minz)
				minz = pv[i].p[0];

			switch(lmode) {
			case 0:
				pv[i].p[3] = ((light >> (8*(3-i))) & 0xff) / 64.0;
				break;
			case 1:
				pv[i].p[3] = ((light >> (8*(3-i))) & 0xff) / 64.0;
				break;
			case 2:
				pv[i].p[3] = 1.0;
				break;
			case 3: {
				UINT32 norm = m_ptrom[extptr++];
				INT32 nx = u32_to_s10(norm >> 20);
				INT32 ny = u32_to_s10(norm >> 10);
				INT32 nz = u32_to_s10(norm);
				INT32 nrx, nry, nrz;
				render_apply_matrot(nx, ny, nz, re, nrx, nry, nrz);
				float lsi = float(nrx*m_light_vector[0] + nry*m_light_vector[1] + nrz*m_light_vector[2])/4194304.0f;
				if(lsi < 0)
					lsi = 0;

				// Mapping taken out of a hat
				pv[i].p[3] = 0.25f+1.5f*lsi;
				break;
			}
			}
		}

		namcos23_poly_entry *p = render.polys + render.poly_count;

        // Should be unnecessary now that frustum clipping happens, but this still culls polys behind the camera
		p->vertex_count = render.polymgr->zclip_if_less(ne, pv, p->pv, 4, 0.00001f);
        
        // Project if you don't clip on the near plane
		if(p->vertex_count >= 3) {
            // Project the eye points
            frustum_clip_vertex<float, 3> clipVerts[10];
            for(int i=0; i<p->vertex_count; i++) {
                // Construct a frustum clipping vert from the NDCoords
                const float Z = p->pv[i].p[0];
                clipVerts[i].x = p->pv[i].x / Z;
                clipVerts[i].y = p->pv[i].y / Z;
                clipVerts[i].z = Z;
                clipVerts[i].w = Z;
                clipVerts[i].p[0] = p->pv[i].p[1];
                clipVerts[i].p[1] = p->pv[i].p[2];
                clipVerts[i].p[2] = p->pv[i].p[3];
            }

            // Clip against all edges of the view frustum
            int num_vertices = frustum_clip_all<float, 3>(clipVerts, p->vertex_count, clipVerts);
            
            if (num_vertices != 0)
            {
                // Push the results back into the main vertices
                for (int i=0; i < num_vertices; i++)
                {
                    p->pv[i].x = clipVerts[i].x;
                    p->pv[i].y = clipVerts[i].y;
                    p->pv[i].p[0] = clipVerts[i].w;
                    p->pv[i].p[1] = clipVerts[i].p[0];
                    p->pv[i].p[2] = clipVerts[i].p[1];
                    p->pv[i].p[3] = clipVerts[i].p[2];
                }
                p->vertex_count = num_vertices;
                
                // This is our poor-man's projection matrix
                for(int i=0; i<p->vertex_count; i++) 
                {
                    render_project(p->pv[i]);
                    
                    float w = p->pv[i].p[0];
                    p->pv[i].p[1] *= w;
                    p->pv[i].p[2] *= w;
                    p->pv[i].p[3] *= w;
                }
                
                // Compute an odd sorta'-Z thing that can situate the polygon wherever you want in Z-depth
                p->zkey = 0.5f*(minz+maxz);
                p->front = !(h & 0x00000001);
                p->rd.machine = &machine();
                p->rd.texture_lookup = render_texture_lookup_nocache_point;
                p->rd.pens = m_palette->pens() + (color << 8);
                render.poly_count++;
            }
		}

		if(type & 0x000010000)
			break;
	}
}

static int render_poly_compare(const void *i1, const void *i2)
{
	const namcos23_poly_entry *p1 = *(const namcos23_poly_entry **)i1;
	const namcos23_poly_entry *p2 = *(const namcos23_poly_entry **)i2;

	if(p1->front != p2->front)
		return p1->front ? 1 : -1;

	return p1->zkey < p2->zkey ? 1 : p1->zkey > p2->zkey ? -1 : 0;
}

void namcos23_renderer::render_flush(bitmap_rgb32& bitmap)
{
	render_t &render = m_state.m_render;

	if(!render.poly_count)
		return;

	for(int i=0; i<render.poly_count; i++)
		render.poly_order[i] = &render.polys[i];

	qsort(render.poly_order, render.poly_count, sizeof(namcos23_poly_entry *), render_poly_compare);

	const static rectangle scissor(0, 639, 0, 479);

	for(int i=0; i<render.poly_count; i++) {
		const namcos23_poly_entry *p = render.poly_order[i];
		namcos23_render_data& extra = render.polymgr->object_data_alloc();
		extra = p->rd;
		extra.bitmap = &bitmap;

		// We should probably split the polygons into triangles ourselves to insure everything is being rendered properly
		if (p->vertex_count == 3)
			render_triangle(scissor, render_delegate(FUNC(namcos23_renderer::render_scanline), this), 4, p->pv[0], p->pv[1], p->pv[2]);
		else if (p->vertex_count == 4)
			render_polygon<4>(scissor, render_delegate(FUNC(namcos23_renderer::render_scanline), this), 4, p->pv);
		else if (p->vertex_count == 5)
			render_polygon<5>(scissor, render_delegate(FUNC(namcos23_renderer::render_scanline), this), 4, p->pv);
		else if (p->vertex_count == 6)
			render_polygon<6>(scissor, render_delegate(FUNC(namcos23_renderer::render_scanline), this), 4, p->pv);
	}
	render.poly_count = 0;
}

void namcos23_state::render_run(bitmap_rgb32 &bitmap)
{
	render_t &render = m_render;
	const namcos23_render_entry *re = render.entries[!render.cur];

	render.poly_count = 0;
	for(int i=0; i<render.count[!render.cur]; i++) {
		switch(re->type) {
		case MODEL:
			render_one_model(re);
			break;
		case FLUSH:
			render.polymgr->render_flush(bitmap);
			break;
		}
		re++;
	}
	render.polymgr->render_flush(bitmap);
	render.polymgr->wait();
}



// C404 (gamma/palette)

void namcos23_state::update_mixer()
{
	// should be similar to Super System 22 C404
	// 08 - background color red
	// 09 - background color green
	// 0a - background color blue
	// 1b - text layer palette base
	// 1f - layer enable (d0: polygons, d1: sprites, d2: text)
	m_c404.bgcolor = rgb_t(nthword(m_gammaram,0x08), nthword(m_gammaram,0x09), nthword(m_gammaram,0x0a));
	m_c404.palbase = nthword(m_gammaram, 0x1b) << 8 & 0x7f00;
	m_c404.layer = nthword(m_gammaram, 0x1f) & 0xff;
}

WRITE32_MEMBER(namcos23_state::paletteram_w)
{
	COMBINE_DATA(&m_generic_paletteram_32[offset]);

	// each LONGWORD is 2 colors, each OFFSET is 2 colors
	for(int i = 0; i < 2; i++) {
		int which = (offset << 2 | i << 1) & 0xfffe;
		int r = nthbyte(m_generic_paletteram_32, which|0x00001);
		int g = nthbyte(m_generic_paletteram_32, which|0x10001);
		int b = nthbyte(m_generic_paletteram_32, which|0x20001);
		m_palette->set_pen_color(which/2, rgb_t(r,g,b));
	}
}



// C361 (text)

TILE_GET_INFO_MEMBER(namcos23_state::TextTilemapGetInfo)
{
	UINT16 data = nthword( m_textram,tile_index );
	/**
	* xxxx.----.----.---- palette select
	* ----.xx--.----.---- flip
	* ----.--xx.xxxx.xxxx code
	*/
	SET_TILE_INFO_MEMBER(0, data&0x03ff, data>>12, TILE_FLIPYX((data&0x0c00)>>10));
}

WRITE32_MEMBER(namcos23_state::textram_w)
{
	COMBINE_DATA( &m_textram[offset] );
	m_bgtilemap->mark_tile_dirty(offset*2);
	m_bgtilemap->mark_tile_dirty((offset*2)+1);
}

WRITE32_MEMBER(namcos23_state::textchar_w)
{
	COMBINE_DATA(&m_charram[offset]);
	m_gfxdecode->gfx(0)->mark_dirty(offset/32);
}



// Video start/update callbacks

VIDEO_START_MEMBER(namcos23_state,s23)
{
	m_gfxdecode->gfx(0)->set_source(reinterpret_cast<UINT8 *>(m_charram.target()));
	m_bgtilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(namcos23_state::TextTilemapGetInfo),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_bgtilemap->set_transparent_pen(0xf);
	m_bgtilemap->set_scrolldx(860, 860);
	m_render.polymgr = auto_alloc(machine(), namcos23_renderer(*this));
}


UINT32 namcos23_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	update_mixer();
	bitmap.fill(m_c404.bgcolor, cliprect);

	render_run(bitmap);

	m_bgtilemap->set_palette_offset(m_c404.palbase);
	if(m_c404.layer & 4)
		m_bgtilemap->draw(screen, bitmap, cliprect, 0, 0);

	m_vblank_count++;

	return 0;
}





/***************************************************************************

  Main CPU I/O + Memory Map
  (some cpu->video I/O handled above)

***************************************************************************/

// Interrupts

void namcos23_state::update_main_interrupts(UINT32 cause)
{
	UINT32 changed = cause ^ m_main_irqcause;
	m_main_irqcause = cause;

	// level 2: vblank
	if(changed & MAIN_VBLANK_IRQ)
		m_maincpu->set_input_line(MIPS3_IRQ0, (cause & MAIN_VBLANK_IRQ) ? ASSERT_LINE : CLEAR_LINE);

	// level 3: C361/subcpu
	if(changed & (MAIN_C361_IRQ | MAIN_SUBCPU_IRQ))
		m_maincpu->set_input_line(MIPS3_IRQ1, (cause & (MAIN_C361_IRQ | MAIN_SUBCPU_IRQ)) ? ASSERT_LINE : CLEAR_LINE);

	// level 4: C435
	if(changed & MAIN_C435_IRQ)
		m_maincpu->set_input_line(MIPS3_IRQ2, (cause & MAIN_C435_IRQ) ? ASSERT_LINE : CLEAR_LINE);

	// level 5: C422
	if(changed & MAIN_C422_IRQ)
		m_maincpu->set_input_line(MIPS3_IRQ3, (cause & MAIN_C422_IRQ) ? ASSERT_LINE : CLEAR_LINE);

	// crszone(sys23ev2) has a different configuration, are they hardwired or configured by software? (where?)..
	// level 3: C422/subcpu
	// level 4: vblank
	// level 5: C451/C361
	// level 6: C450
}

INTERRUPT_GEN_MEMBER(namcos23_state::interrupt)
{
	if(!m_ctl_vbl_active) {
		m_ctl_vbl_active = true;
		update_main_interrupts(m_main_irqcause | MAIN_VBLANK_IRQ);
	}

	m_render.cur = !m_render.cur;
	m_render.count[m_render.cur] = 0;
}

void namcos23_state::sub_irq(screen_device &screen, bool vblank_state)
{
	m_subcpu->set_input_line(1, vblank_state ? ASSERT_LINE : CLEAR_LINE);
	m_adc->adtrg_w(vblank_state);
	m_sub_portb = (m_sub_portb & 0x7f) | (vblank_state << 7);
}


// C417

READ16_MEMBER(namcos23_state::c417_r)
{
	switch(offset) {
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
	case 0:
		return 0x8e | (m_screen->vblank() ? 0x0000 : 0x8000);
	case 1:
		return m_c417.adr;
	case 4:
		//logerror("c417_r %04x = %04x (%08x, %08x)\n", c417.adr, c417.ram[c417.adr], space.device().safe_pc(), (unsigned int)space.device().state().state_int(MIPS3_R31));
		return m_c417.ram[m_c417.adr];
	case 5:
		if(m_c417.pointrom_adr >= m_ptrom_limit)
			return 0xffff;
		return m_ptrom[m_c417.pointrom_adr] >> 16;
	case 6:
		if(m_c417.pointrom_adr >= m_ptrom_limit)
			return 0xffff;
		return m_ptrom[m_c417.pointrom_adr];
	}

	logerror("c417_r %x @ %04x (%08x, %08x)\n", offset, mem_mask, space.device().safe_pc(), (unsigned int)space.device().state().state_int(MIPS3_R31));
	return 0;
}

WRITE16_MEMBER(namcos23_state::c417_w)
{
	switch(offset) {
	case 0:
		c435_pio_w(data);
		break;
	case 1:
		COMBINE_DATA(&m_c417.adr);
		break;
	case 2:
		m_c417.pointrom_adr = (m_c417.pointrom_adr << 16) | data;
		break;
	case 3:
		m_c417.pointrom_adr = 0;
		break;
	case 4:
		//logerror("c417_w %04x = %04x (%08x, %08x)\n", m_c417.adr, data, space.device().safe_pc(), (unsigned int)space.device().state().state_int(MIPS3_R31));
		COMBINE_DATA(m_c417.ram + m_c417.adr);
		break;
	case 7:
		logerror("c417_w: ack IRQ 2 (%x)\n", data);
		update_main_interrupts(m_main_irqcause & ~MAIN_C435_IRQ);
		break;
	default:
		logerror("c417_w %x, %04x @ %04x (%08x, %08x)\n", offset, data, mem_mask, space.device().safe_pc(), (unsigned int)space.device().state().state_int(MIPS3_R31));
		break;
	}
}



// C412

READ16_MEMBER(namcos23_state::c412_ram_r)
{
	//  logerror("c412_ram_r %06x (%08x, %08x)\n", offset, space.device().safe_pc(), (unsigned int)space.device().state().state_int(MIPS3_R31));
	if(offset < 0x100000)
		return m_c412.sdram_a[offset & 0xfffff];
	else if(offset < 0x200000)
		return m_c412.sdram_b[offset & 0xfffff];
	else if(offset < 0x220000)
		return m_c412.sram   [offset & 0x1ffff];
	else if(offset < 0x220200)
		return m_c412.pczram [offset & 0x001ff];

	return 0xffff;
}

WRITE16_MEMBER(namcos23_state::c412_ram_w)
{
	//  logerror("c412_ram_w %06x = %04x (%08x, %08x)\n", offset, data, space.device().safe_pc(), (unsigned int)space.device().state().state_int(MIPS3_R31));
	if(offset < 0x100000)
		COMBINE_DATA(m_c412.sdram_a + (offset & 0xfffff));
	else if(offset < 0x200000)
		COMBINE_DATA(m_c412.sdram_b + (offset & 0xfffff));
	else if(offset < 0x220000)
		COMBINE_DATA(m_c412.sram    + (offset & 0x1ffff));
	else if(offset < 0x220200)
		COMBINE_DATA(m_c412.pczram  + (offset & 0x001ff));
}

READ16_MEMBER(namcos23_state::c412_r)
{
	switch(offset) {
	case 0x3:
		return 0x0002; // 0001 = busy, 0002 = game uploads things
	case 0x8:
		return m_c412.adr;
	case 0x9:
		return m_c412.adr >> 16;
	case 0xa:
		return c412_ram_r(space, m_c412.adr, mem_mask);
	case 0xc:
		// unknown status, 500gp reads it and waits for a transition
		// no other games use it?
		m_c412.status_c ^= 1;
		return m_c412.status_c;
	}

	logerror("c412_r %x @ %04x (%08x, %08x)\n", offset, mem_mask, space.device().safe_pc(), (unsigned int)space.device().state().state_int(MIPS3_R31));
	return 0;
}

WRITE16_MEMBER(namcos23_state::c412_w)
{
	switch(offset) {
	case 0x2:
		// d0: cz on
		// other bits: no function?
		break;
	case 0x8:
		m_c412.adr = (data & mem_mask) | (m_c412.adr & (0xffffffff ^ mem_mask));
		break;
	case 0x9:
		m_c412.adr = ((data & mem_mask) << 16) | (m_c412.adr & (0xffffffff ^ (mem_mask << 16)));
		break;
	case 0xa:
		c412_ram_w(space, m_c412.adr, data, mem_mask);
		m_c412.adr += 2;
		break;
	default:
		logerror("c412_w %x, %04x @ %04x (%08x, %08x)\n", offset, data, mem_mask, space.device().safe_pc(), (unsigned int)space.device().state().state_int(MIPS3_R31));
		break;
	}
}



// C421

READ16_MEMBER(namcos23_state::c421_ram_r)
{
	//  logerror("c421_ram_r %06x (%08x, %08x)\n", offset, space.device().safe_pc(), (unsigned int)space.device().state().state_int(MIPS3_R31));
	if(offset < 0x40000)
		return m_c421.dram_a[offset & 0x3ffff];
	else if(offset < 0x80000)
		return m_c421.dram_b[offset & 0x3ffff];
	else if(offset < 0x88000)
		return m_c421.sram  [offset & 0x07fff];

	return 0xffff;
}

WRITE16_MEMBER(namcos23_state::c421_ram_w)
{
	//  logerror("c421_ram_w %06x = %04x (%08x, %08x)\n", offset, data, space.device().safe_pc(), (unsigned int)space.device().state().state_int(MIPS3_R31));
	if(offset < 0x40000)
		COMBINE_DATA(m_c421.dram_a + (offset & 0x3ffff));
	else if(offset < 0x80000)
		COMBINE_DATA(m_c421.dram_b + (offset & 0x3ffff));
	else if(offset < 0x88000)
		COMBINE_DATA(m_c421.sram   + (offset & 0x07fff));
}

READ16_MEMBER(namcos23_state::c421_r)
{
	switch(offset) {
	case 0:
		return c421_ram_r(space, m_c421.adr & 0xfffff, mem_mask);

	case 2:
		return m_c421.adr >> 16;
	case 3:
		return m_c421.adr;
	}

	logerror("c421_r %x @ %04x (%08x, %08x)\n", offset, mem_mask, space.device().safe_pc(), (unsigned int)space.device().state().state_int(MIPS3_R31));
	return 0;
}

WRITE16_MEMBER(namcos23_state::c421_w)
{
	switch(offset) {
	case 0:
		c421_ram_w(space, m_c421.adr & 0xfffff, data, mem_mask);
		m_c421.adr += 2;
		break;
	case 2:
		m_c421.adr = ((data & mem_mask) << 16) | (m_c421.adr & (0xffffffff ^ (mem_mask << 16)));
		break;
	case 3:
		m_c421.adr = (data & mem_mask) | (m_c421.adr & (0xffffffff ^ mem_mask));
		break;
	default:
		logerror("c421_w %x, %04x @ %04x (%08x, %08x)\n", offset, data, mem_mask, space.device().safe_pc(), (unsigned int)space.device().state().state_int(MIPS3_R31));
		break;
	}
}



// C422

READ16_MEMBER(namcos23_state::c422_r)
{
	return m_c422.regs[offset];
}

WRITE16_MEMBER(namcos23_state::c422_w)
{
	switch(offset) {
	case 1:
		if(data == 0xfffb) {
			logerror("c422_w: raise IRQ 3\n");
			update_main_interrupts(m_main_irqcause | MAIN_C422_IRQ);
		}
		else if(data == 0x000f) {
			logerror("c422_w: ack IRQ 3\n");
			update_main_interrupts(m_main_irqcause & ~MAIN_C422_IRQ);
		}
		break;

	default:
		logerror("c422_w: %04x @ %x\n", data, offset);
		break;
	}

	COMBINE_DATA(&m_c422.regs[offset]);
}



// C361 (text)

TIMER_CALLBACK_MEMBER(namcos23_state::c361_timer_cb)
{
	if(m_c361.scanline != 0x1ff) {
		// need to do a partial update here, but doesn't work properly yet
		//m_screen->update_partial(m_screen->vpos());
		update_main_interrupts(m_main_irqcause | MAIN_C361_IRQ);

		// TC2 indicates it's probably one-shot since it resets it each VBL...
		//c361.timer->adjust(m_screen->time_until_pos(c361.scanline));
	}
	else
		update_main_interrupts(m_main_irqcause & ~MAIN_C361_IRQ);
}

WRITE16_MEMBER(namcos23_state::c361_w)
{
	switch(offset) {
	case 0:
		m_bgtilemap->set_scrollx(0, data&0xfff);
		break;

	case 1:
		m_bgtilemap->set_scrolly(0, data&0xfff);
		break;

	case 4: // interrupt control
		m_c361.scanline = data & 0x1ff;
		m_c361.timer->adjust(m_screen->time_until_pos(m_c361.scanline));
		break;

	default:
		logerror("c361_w %x, %04x @ %04x (%08x, %08x)\n", offset, data, mem_mask, space.device().safe_pc(), (unsigned int)space.device().state().state_int(MIPS3_R31));
		break;
	}
}

READ16_MEMBER(namcos23_state::c361_r)
{
	switch(offset) {
	// current raster position
	// how does it work exactly? it's not understood in namcos22 either (also has a c361)
	case 5:
		update_main_interrupts(m_main_irqcause & ~MAIN_C361_IRQ);
		return (m_screen->vpos()*2) | (m_screen->vblank() ? 1 : 0);
	case 6:
		update_main_interrupts(m_main_irqcause & ~MAIN_C361_IRQ);
		return m_screen->vblank() ? 1 : 0;
	}

	logerror("c361_r %x @ %04x (%08x, %08x)\n", offset, mem_mask, space.device().safe_pc(), (unsigned int)space.device().state().state_int(MIPS3_R31));
	return 0xffff;
}



// C?? (control)

WRITE16_MEMBER(namcos23_state::ctl_w)
{
	switch(offset) {
	case 0:
		if(m_ctl_led != (data & 0xff)) {
			m_ctl_led = data & 0xff;
			for(int i = 0; i < 8; i++)
				output_set_lamp_value(i, (~data<<i & 0x80) ? 0 : 1);
		}
		break;

	case 2: case 3:
		// These may be coming from another CPU, in particular the I/O one
		m_ctl_inp_buffer[offset-2] = (offset == 2 ? m_p1 : m_p2)->read();
		break;
	case 5:
		if(m_ctl_vbl_active) {
			m_ctl_vbl_active = false;
			update_main_interrupts(m_main_irqcause & ~MAIN_VBLANK_IRQ);
		}
		break;

	case 6: // gmen wars spams this heavily with 0 prior to starting the GMEN board test
		if(data != 0)
			logerror("ctl_w %x, %04x @ %04x (%08x, %08x)\n", offset, data, mem_mask, space.device().safe_pc(), (unsigned int)space.device().state().state_int(MIPS3_R31));
		break;

	default:
		logerror("ctl_w %x, %04x @ %04x (%08x, %08x)\n", offset, data, mem_mask, space.device().safe_pc(), (unsigned int)space.device().state().state_int(MIPS3_R31));
		break;
	}
}

READ16_MEMBER(namcos23_state::ctl_r)
{
	switch(offset) {
	// 0100 set freezes gorgon (polygon fifo flag)
	case 1:
		return 0x0000 | ioport("DSW")->read() | ((m_main_irqcause & MAIN_C361_IRQ) ? 0x400 : 0);
	case 2: case 3: {
		UINT16 res = m_ctl_inp_buffer[offset-2] & 0x800 ? 0xffff : 0x0000;
		m_ctl_inp_buffer[offset-2] = (m_ctl_inp_buffer[offset-2] << 1) | 1;
		return res;
	}
	}

	logerror("ctl_r %x @ %04x (%08x, %08x)\n", offset, mem_mask, space.device().safe_pc(), (unsigned int)space.device().state().state_int(MIPS3_R31));
	return 0xffff;
}



// C?? (MCU enable)

WRITE16_MEMBER(namcos23_state::mcuen_w)
{
	switch(offset) {
	case 2:
		// subcpu irq ack
		update_main_interrupts(m_main_irqcause & ~MAIN_SUBCPU_IRQ);
		break;

	case 5:
		// boot/start the audio mcu
		if(data) {
			logerror("mcuen_w: booting H8/3002\n");

			// Panic Park: writing 1 when it's already running means reboot?
			if(m_subcpu_running) {
				m_subcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
			}

			m_subcpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
			m_subcpu_running = true;
		} else {
			logerror("mcuen_w: stopping H8/3002\n");
			m_subcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
			m_subcpu_running = false;
		}
		break;

	default:
		logerror("mcuen_w: mask %04x, data %04x @ %x\n", mem_mask, data, offset);
		break;
	}
}



// C?? (unknown comms)

// while getting the subcpu to be ready, panicprk sits in a tight loop waiting for this AND 0002 to be non-zero (at PC=BFC02F00)
// timecrs2 locks up in a similar way as panicprk, at the beginning of the 2nd level, by reading/writing to this register a couple of times
READ16_MEMBER(namcos23_state::sub_comm_r)
{
	return 2;
}

WRITE16_MEMBER(namcos23_state::sub_comm_w)
{
	;
}


// System Gorgon
static ADDRESS_MAP_START( gorgon_map, AS_PROGRAM, 32, namcos23_state )
	ADDRESS_MAP_GLOBAL_MASK(0xfffffff)
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM AM_SHARE("mainram")
	AM_RANGE(0x01000000, 0x010000ff) AM_READWRITE(c435_r, c435_w)
	AM_RANGE(0x02000000, 0x0200000f) AM_READWRITE16(c417_r, c417_w, 0xffffffff)
	AM_RANGE(0x04400000, 0x0440ffff) AM_RAM AM_SHARE("shared_ram") // Communication RAM (C416)
	AM_RANGE(0x04c3ff00, 0x04c3ff0f) AM_WRITE16(mcuen_w, 0xffffffff)
	AM_RANGE(0x06080000, 0x0608000f) AM_RAM AM_SHARE("czattr")
	AM_RANGE(0x06080200, 0x060803ff) AM_RAM // PCZ Convert RAM (C406) (should be banked)
	AM_RANGE(0x06108000, 0x061087ff) AM_RAM AM_SHARE("gammaram") // Gamma RAM (C404)
	AM_RANGE(0x06110000, 0x0613ffff) AM_RAM_WRITE(paletteram_w) AM_SHARE("paletteram") // Palette RAM (C404)
	AM_RANGE(0x06400000, 0x0641dfff) AM_RAM_WRITE(textchar_w) AM_SHARE("charram") // Text CGRAM (C361)
	AM_RANGE(0x0641e000, 0x0641ffff) AM_RAM_WRITE(textram_w) AM_SHARE("textram") // Text VRAM (C361)
	AM_RANGE(0x06420000, 0x0642000f) AM_READWRITE16(c361_r, c361_w, 0xffffffff) // C361
	AM_RANGE(0x08000000, 0x087fffff) AM_ROM AM_REGION("data", 0) // data ROMs
	AM_RANGE(0x0c000000, 0x0c00ffff) AM_RAM AM_SHARE("nvram") // Backup RAM
	AM_RANGE(0x0d000000, 0x0d00000f) AM_READWRITE16(ctl_r, ctl_w, 0xffffffff) // write for LEDs at d000000, watchdog at d000004
	AM_RANGE(0x0e000000, 0x0e007fff) AM_RAM // C405 RAM - what is this?
	AM_RANGE(0x0f000000, 0x0f000003) AM_READWRITE16(sub_comm_r, sub_comm_w, 0xffffffff) // not sure
	AM_RANGE(0x0f200000, 0x0f203fff) AM_RAM // C422 RAM
	AM_RANGE(0x0f300000, 0x0f30000f) AM_READWRITE16(c422_r, c422_w, 0xffffffff) // C422 registers
	AM_RANGE(0x0fc00000, 0x0fffffff) AM_WRITENOP AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END

// (Super) System 23
static ADDRESS_MAP_START( s23_map, AS_PROGRAM, 32, namcos23_state )
	ADDRESS_MAP_GLOBAL_MASK(0xfffffff)
	AM_RANGE(0x00000000, 0x00ffffff) AM_RAM AM_SHARE("mainram")
	AM_RANGE(0x01000000, 0x010000ff) AM_READWRITE(c435_r, c435_w)
	AM_RANGE(0x02000000, 0x0200000f) AM_READWRITE16(c417_r, c417_w, 0xffffffff)
	AM_RANGE(0x04400000, 0x0440ffff) AM_RAM AM_SHARE("shared_ram") // Communication RAM (C416)
	AM_RANGE(0x04c3ff00, 0x04c3ff0f) AM_WRITE16(mcuen_w, 0xffffffff)
	AM_RANGE(0x06000000, 0x0600ffff) AM_RAM AM_SHARE("nvram") // Backup RAM
	AM_RANGE(0x06200000, 0x06203fff) AM_RAM // C422 RAM
	AM_RANGE(0x06400000, 0x0640000f) AM_READWRITE16(c422_r, c422_w, 0xffffffff) // C422 registers
	AM_RANGE(0x06800000, 0x0681dfff) AM_RAM_WRITE(textchar_w) AM_SHARE("charram") // Text CGRAM (C361)
	AM_RANGE(0x0681e000, 0x0681ffff) AM_RAM_WRITE(textram_w) AM_SHARE("textram") // Text VRAM (C361)
	AM_RANGE(0x06820000, 0x0682000f) AM_READWRITE16(c361_r, c361_w, 0xffffffff) // C361
	AM_RANGE(0x06a08000, 0x06a087ff) AM_RAM AM_SHARE("gammaram") // Gamma RAM (C404)
	AM_RANGE(0x06a10000, 0x06a3ffff) AM_RAM_WRITE(paletteram_w) AM_SHARE("paletteram") // Palette RAM (C404)
	AM_RANGE(0x08000000, 0x08ffffff) AM_ROM AM_REGION("data", 0x0000000) AM_MIRROR(0x1000000) // data ROMs
	AM_RANGE(0x0a000000, 0x0affffff) AM_ROM AM_REGION("data", 0x1000000) AM_MIRROR(0x1000000)
	AM_RANGE(0x0c000000, 0x0c00001f) AM_READWRITE16(c412_r, c412_w, 0xffffffff)
	AM_RANGE(0x0c400000, 0x0c400007) AM_READWRITE16(c421_r, c421_w, 0xffffffff)
	AM_RANGE(0x0d000000, 0x0d00000f) AM_READWRITE16(ctl_r, ctl_w, 0xffffffff)
	AM_RANGE(0x0e800000, 0x0e800003) AM_READWRITE16(sub_comm_r, sub_comm_w, 0xffffffff) // not sure
	AM_RANGE(0x0fc00000, 0x0fffffff) AM_WRITENOP AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END



// GMEN interface

READ32_MEMBER(namcos23_state::gmen_trigger_sh2)
{
	logerror("gmen_trigger_sh2: booting SH-2\n");
	m_gmen_sh2->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);

	return 0;
}

READ32_MEMBER(namcos23_state::sh2_shared_r)
{
	return m_gmen_sh2_shared[offset];
}

WRITE32_MEMBER(namcos23_state::sh2_shared_w)
{
	COMBINE_DATA(&m_gmen_sh2_shared[offset]);
}

static ADDRESS_MAP_START( gmen_mips_map, AS_PROGRAM, 32, namcos23_state )
	AM_IMPORT_FROM(s23_map)
	AM_RANGE(0x0e400000, 0x0e400003) AM_READ(gmen_trigger_sh2)
	AM_RANGE(0x0e700000, 0x0e70ffff) AM_READWRITE(sh2_shared_r, sh2_shared_w)
ADDRESS_MAP_END


// SH2 memmap
/* TODO: of course, I believe that area 0x008***** is actually a bank of some sort ... */
static ADDRESS_MAP_START( gmen_sh2_map, AS_PROGRAM, 32, namcos23_state )
	AM_RANGE(0x00000000, 0x0000ffff) AM_RAM AM_SHARE("gmen_sh2_shared")
	AM_RANGE(0x00800000, 0x008fffff) AM_ROM AM_REGION("data", 0xc00000) //c00000 "data" for final furlong 2. 0x1b6bc0 "user1" for gunmen wars
	AM_RANGE(0x01800000, 0x0183ffff) AM_RAM // ???
	//AM_RANGE(0x02800000, 0x02800003) AM_RAM // probably transfer status related, reads/writes after each end of flash transfer, TBD
	AM_RANGE(0x04000000, 0x043fffff) AM_RAM // SH-2 main work RAM (SDRAM)
	AM_RANGE(0x06000000, 0x06000003) AM_NOP // serial port for camera?
ADDRESS_MAP_END





/***************************************************************************

  Sub CPU (H8/3002 MCU) I/O + Memory Map

***************************************************************************/

WRITE16_MEMBER(namcos23_state::sharedram_sub_w)
{
	UINT16 *shared16 = reinterpret_cast<UINT16 *>(m_shared_ram.target());

	// fake that an I/O board is connected for games w/o a dump or that aren't properly communicating with it yet
	if(!m_has_jvsio) {
		if((offset == 0x4052/2) && (data == 0x78)) {
			data = 0;
		}
	}

	COMBINE_DATA(&shared16[BYTE_XOR_BE(offset)]);
}

READ16_MEMBER(namcos23_state::sharedram_sub_r)
{
	UINT16 *shared16 = reinterpret_cast<UINT16 *>(m_shared_ram.target());

	return shared16[BYTE_XOR_BE(offset)];
}


WRITE16_MEMBER(namcos23_state::sub_interrupt_main_w)
{
	if((mem_mask == 0xffff) && (data == 0x3170)) {
		update_main_interrupts(m_main_irqcause | MAIN_SUBCPU_IRQ);
	} else {
		logerror("Unknown write %x to sub_interrupt_main_w!\n", data);
	}
}



// Port 6

READ16_MEMBER(namcos23_state::mcu_p6_r)
{
	// bit 1 = JVS cable present sense (1 = I/O board plugged in)
	return (m_jvssense << 1) | 0xfd;
}

WRITE16_MEMBER(namcos23_state::mcu_p6_w)
{
	//printf("%02x to port 6\n", data);
}



// Port 8, looks like serial comms, where to/from?

READ16_MEMBER(namcos23_state::mcu_p8_r)
{
	return 0x02;
}

WRITE16_MEMBER(namcos23_state::mcu_p8_w)
{
	;
}



// Port A

READ16_MEMBER(namcos23_state::mcu_pa_r)
{
	return m_sub_porta;
}

WRITE16_MEMBER(namcos23_state::mcu_pa_w)
{
	m_rtc->ce_w(data & 1);
	m_sub_porta = data;
	m_rtc->ce_w((m_sub_portb & 0x20) && (m_sub_porta & 1));
	m_settings->ce_w((m_sub_portb & 0x20) && !(m_sub_porta & 1));
}



// Port B

READ16_MEMBER(namcos23_state::mcu_pb_r)
{
	return m_sub_portb;
}

WRITE16_MEMBER(namcos23_state::mcu_pb_w)
{
	m_sub_portb = (m_sub_portb & 0x80) | (data & 0x7f);
	m_rtc->ce_w((m_sub_portb & 0x20) && (m_sub_porta & 1));
	m_settings->ce_w((m_sub_portb & 0x20) && !(m_sub_porta & 1));
}


static ADDRESS_MAP_START( s23h8rwmap, AS_PROGRAM, 16, namcos23_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x08ffff) AM_READWRITE(sharedram_sub_r, sharedram_sub_w )
	AM_RANGE(0x280000, 0x287fff) AM_DEVREADWRITE("c352", c352_device, read, write)
	AM_RANGE(0x300000, 0x300003) AM_NOP // seems to be more inputs, maybe false leftover code from System 12?
	AM_RANGE(0x300010, 0x300011) AM_NOP
	AM_RANGE(0x300020, 0x300021) AM_WRITE(sub_interrupt_main_w )
	AM_RANGE(0x300030, 0x300031) AM_WRITENOP    // timecrs2 writes this when writing to the sync shared ram location, motoxgo doesn't
ADDRESS_MAP_END


static ADDRESS_MAP_START( s23h8iomap, AS_IO, 16, namcos23_state )
	AM_RANGE(h8_device::PORT_6, h8_device::PORT_6) AM_READWRITE(mcu_p6_r, mcu_p6_w )
	AM_RANGE(h8_device::PORT_8, h8_device::PORT_8) AM_READWRITE(mcu_p8_r, mcu_p8_w )
	AM_RANGE(h8_device::PORT_A, h8_device::PORT_A) AM_READWRITE(mcu_pa_r, mcu_pa_w )
	AM_RANGE(h8_device::PORT_B, h8_device::PORT_B) AM_READWRITE(mcu_pb_r, mcu_pb_w )
	AM_RANGE(h8_device::ADC_0, h8_device::ADC_0) AM_NOP
	AM_RANGE(h8_device::ADC_1, h8_device::ADC_1) AM_NOP
	AM_RANGE(h8_device::ADC_2, h8_device::ADC_2) AM_NOP
	AM_RANGE(h8_device::ADC_3, h8_device::ADC_3) AM_NOP
ADDRESS_MAP_END





/***************************************************************************

  I/O Board (H8/3334 MCU "Namco C78") I/O + Memory Map

***************************************************************************/

// Port 4

READ16_MEMBER(namcos23_state::iob_p4_r)
{
	return m_tssio_port_4;
}

WRITE16_MEMBER(namcos23_state::iob_p4_w)
{
	m_tssio_port_4 = data;

	// bit 2 = SENSE line back to main (0 = asserted, 1 = dropped)
	m_jvssense = (data & 0x04) ? 0 : 1;
}



// Port 6

READ16_MEMBER(namcos23_state::iob_p6_r)
{
	// d4 is service button
	UINT8 sb = (ioport("SERVICE")->read() & 1) << 4;
	// other bits: unknown

	return sb | 0;
}

WRITE16_MEMBER(namcos23_state::iob_p6_w)
{
	//printf("iob %02x to port 6\n", data);
}


// Analog Ports

READ16_MEMBER(namcos23_state::iob_analog_r)
{
	return m_adc_ports[offset]->read_safe(0);
}


static ADDRESS_MAP_START( s23iobrdmap, AS_PROGRAM, 16, namcos23_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM AM_REGION("iocpu", 0)
	AM_RANGE(0x6000, 0x6001) AM_READ_PORT("IN01")
	AM_RANGE(0x6002, 0x6003) AM_READ_PORT("IN23")
	AM_RANGE(0x6004, 0x6005) AM_WRITENOP
	AM_RANGE(0x6006, 0x6007) AM_NOP
	AM_RANGE(0xc000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( s23iobrdiomap, AS_IO, 16, namcos23_state )
	AM_RANGE(h8_device::PORT_4,  h8_device::PORT_4)  AM_READWRITE(iob_p4_r, iob_p4_w)
	AM_RANGE(h8_device::PORT_5,  h8_device::PORT_5)  AM_NOP   // bit 2 = status LED to indicate transmitting packet to main
	AM_RANGE(h8_device::PORT_6,  h8_device::PORT_6)  AM_READWRITE(iob_p6_r, iob_p6_w)
	AM_RANGE(h8_device::PORT_8,  h8_device::PORT_8)  AM_NOP   // unknown - used on ASCA-5 only
	AM_RANGE(h8_device::PORT_9,  h8_device::PORT_9)  AM_NOP   // unknown - used on ASCA-5 only
	AM_RANGE(h8_device::ADC_0,   h8_device::ADC_3)   AM_READ(iob_analog_r)
ADDRESS_MAP_END


// Time Crisis lightgun

READ8_MEMBER(namcos23_state::iob_gun_r)
{
	UINT16 xpos = m_lightx->read();
	UINT16 ypos = m_lighty->read();
	// ypos is not completely understood yet, there should be a difference between case 1/4 and 2/5

	switch(offset) {
		case 0: return xpos&0xff;
		case 1: return ypos&0xff;
		case 2: return ypos&0xff;
		case 3: return xpos>>8;
		case 4: return ypos>>8;
		case 5: return ypos>>8;
	}

	return 0;
}

static ADDRESS_MAP_START( timecrs2iobrdmap, AS_PROGRAM, 16, namcos23_state )
	AM_RANGE(0x7000, 0x700f) AM_READ8(iob_gun_r, 0xffff)
	AM_IMPORT_FROM( s23iobrdmap )
ADDRESS_MAP_END





/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( rapidrvr )
	PORT_START("P1")
	PORT_BIT( 0xfff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0xfff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN01")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Test Button") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("Service Up")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Service Down")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Service Enter")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xe0ff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN23")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xf7ff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DIP:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DIP:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DIP:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DIP:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DIP:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DIP:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DIP:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(0x80, IP_ACTIVE_LOW, "DIP:1" )
INPUT_PORTS_END


static INPUT_PORTS_START( rapidrvrp )
	PORT_INCLUDE( rapidrvr )

	// To fully use test mode, both Service Mode dipswitches need to be enabled.
	// Some of the developer menus require you to navigate with the Dev keys,
	// but usually the User keys work fine too.
	PORT_MODIFY("P1")
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_UNKNOWN ) // I/O Unknown Status
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_UNKNOWN ) // I/O Air Dumper FR
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("Dev Service A") // + I/O Air Dumper RR
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_NAME("Dev Service Down")
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_NAME("Dev Service Up")
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Dev Start")
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x400, IP_ACTIVE_LOW, IPT_UNKNOWN ) // I/O Air Dumper FL
	PORT_BIT( 0x800, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("Dev Service B") // + I/O Air Dumper RL

	PORT_MODIFY("IN01")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Test Button") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("User Service Up")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("User Service Down")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("User Service Enter")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("User Start")
	PORT_BIT( 0xe0ff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x08, 0x08, "Debug Messages" )    PORT_DIPLOCATION("DIP:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Dev Service Mode" )  PORT_DIPLOCATION("DIP:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(0x80, IP_ACTIVE_LOW, "DIP:1" ) PORT_NAME("User Service Mode")

#if 0 // need to hook these up properly
	PORT_START("ADC.0")
	PORT_BIT( 0xffff, 0x8000, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_NAME("ADC0") // rear r
	PORT_START("ADC.1")
	PORT_BIT( 0xffff, 0x8000, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_NAME("ADC1") // rear l
	PORT_START("ADC.2")
	PORT_BIT( 0xffff, 0x8000, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_NAME("ADC2") // front r
	PORT_START("ADC.3")
	PORT_BIT( 0xffff, 0x8000, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_NAME("ADC3") // front l
#endif
INPUT_PORTS_END


static INPUT_PORTS_START( finfurl )
	PORT_INCLUDE( rapidrvr )

	PORT_MODIFY("IN01")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )

#if 0 // need to hook these up properly
	PORT_START("ADC.0")
	PORT_BIT( 0xffff, 0x8000, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_NAME("ADC0")
	PORT_START("ADC.1")
	PORT_BIT( 0xffff, 0x8000, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_NAME("ADC1")
	PORT_START("ADC.2")
	PORT_BIT( 0xffff, 0x8000, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_NAME("ADC2")
	PORT_START("ADC.3")
	PORT_BIT( 0xffff, 0x8000, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_NAME("ADC3")
#endif
INPUT_PORTS_END


static INPUT_PORTS_START( s23 )
	// No idea if start is actually there, but we need buttons to pass error screens
	// You can go to the pcb test mode by pressing P1-A, and it doesn't crash anymore somehow
	// Use P1-A to select, P1-Sel+P1-A to exit, up/down to navigate
	PORT_START("P1")
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("Dev Service P1-A")
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_NAME("Dev Service Down")
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_NAME("Dev Service Up")
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Dev Service Start")
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("Dev Service P1-Sel")
	PORT_BIT( 0xc37, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0xfff, IP_ACTIVE_LOW, IPT_UNKNOWN )   // 0x100 = freeze?

	PORT_START("IN01")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) // gun trigger
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) // foot pedal
	PORT_BIT(0x00fc, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN )    // this is the "coin acceptor connected" signal
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_START("IN23")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )

	PORT_START("DSW")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x02, 0x02, "Skip POST" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Freeze?" )
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


static INPUT_PORTS_START( timecrs2 )
	PORT_INCLUDE( s23 )

	PORT_START("LIGHTX") // tuned for CRT
	PORT_BIT( 0xfff, 91+733/2, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(91, 91+733) PORT_SENSITIVITY(48) PORT_KEYDELTA(12)
	PORT_START("LIGHTY") // tuned for CRT - can't shoot below the statusbar?
	PORT_BIT( 0xfff, 38+247/2, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(38, 38+247) PORT_SENSITIVITY(64) PORT_KEYDELTA(4)
INPUT_PORTS_END





/***************************************************************************

  Machine Drivers

***************************************************************************/

void namcos23_state::machine_start()
{
	m_c361.timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(namcos23_state::c361_timer_cb),this));
	m_c361.timer->adjust(attotime::never);

	m_maincpu->add_fastram(0, m_mainram.bytes()-1, FALSE, reinterpret_cast<UINT32 *>(memshare("mainram")->ptr()));
}


void namcos23_state::machine_reset()
{
	m_vblank_count = 0;
	m_c435_buffer_pos = 0;
	m_subcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

MACHINE_RESET_MEMBER(namcos23_state,gmen)
{
	machine_reset();

	// halt the SH-2 until we need it
	m_gmen_sh2->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}



DRIVER_INIT_MEMBER(namcos23_state,s23)
{
	m_ptrom  = (const UINT32 *)memregion("pointrom")->base();
	m_tmlrom = (const UINT16 *)memregion("textilemapl")->base();
	m_tmhrom = memregion("textilemaph")->base();
	m_texrom = memregion("textile")->base();

	m_tileid_mask = (memregion("textilemapl")->bytes()/2 - 1) & ~0xff; // Used for y masking
	m_tile_mask = memregion("textile")->bytes()/256 - 1;
	m_ptrom_limit = memregion("pointrom")->bytes()/4;

	m_jvssense = 1;
	m_main_irqcause = 0;
	m_ctl_vbl_active = false;
	m_sub_portb = 0x50;
	m_tssio_port_4 = 0;
	m_sub_porta = 0;
	m_subcpu_running = false;
	m_render.count[0] = m_render.count[1] = 0;
	m_render.cur = 0;

	if((!strcmp(machine().system().name, "motoxgo")) ||
		(!strcmp(machine().system().name, "panicprk")) ||
		(!strcmp(machine().system().name, "panicprkj")) ||
		(!strcmp(machine().system().name, "rapidrvr")) ||
		(!strcmp(machine().system().name, "rapidrvrv2c")) ||
		(!strcmp(machine().system().name, "rapidrvrp")) ||
		(!strcmp(machine().system().name, "finfurl")) ||
		(!strcmp(machine().system().name, "gunwars")) ||
		(!strcmp(machine().system().name, "gunwarsa")) ||
		(!strcmp(machine().system().name, "downhill")) ||
		(!strcmp(machine().system().name, "finfurl2")) ||
		(!strcmp(machine().system().name, "finfurl2j")) ||
		(!strcmp(machine().system().name, "raceon")) ||
		(!strcmp(machine().system().name, "crszone")) ||
		(!strcmp(machine().system().name, "crszonev4a")) ||
		(!strcmp(machine().system().name, "crszonev3b")) ||
		(!strcmp(machine().system().name, "crszonev3b2")) ||
		(!strcmp(machine().system().name, "crszonev3a")) ||
		(!strcmp(machine().system().name, "crszonev2a")) ||
		(!strcmp(machine().system().name, "timecrs2v2b")) ||
		(!strcmp(machine().system().name, "timecrs2"))) {
		m_has_jvsio = 1;
	} else {
		m_has_jvsio = 0;
	}
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

static GFXDECODE_START( namcos23 )
	GFXDECODE_ENTRY( NULL, 0, namcos23_cg_layout, 0, 0x800 )
GFXDECODE_END


static MACHINE_CONFIG_START( gorgon, namcos23_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", R4650BE, BUSCLOCK*4)
	MCFG_MIPS3_ICACHE_SIZE(8192)   // VERIFIED
	MCFG_MIPS3_DCACHE_SIZE(8192)   // VERIFIED
	MCFG_CPU_PROGRAM_MAP(gorgon_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", namcos23_state, interrupt)

	MCFG_CPU_ADD("subcpu", H83002, H8CLOCK )
	MCFG_CPU_PROGRAM_MAP( s23h8rwmap )
	MCFG_CPU_IO_MAP( s23h8iomap )

	// Timer at 115200*16 for the jvs serial clock
	MCFG_DEVICE_MODIFY(":subcpu:sci0")
	MCFG_H8_SCI_SET_EXTERNAL_CLOCK_PERIOD(attotime::from_hz(JVSCLOCK/8))

	MCFG_CPU_ADD("iocpu", H83334, JVSCLOCK )
	MCFG_CPU_PROGRAM_MAP( s23iobrdmap )
	MCFG_CPU_IO_MAP( s23iobrdiomap )

	MCFG_DEVICE_MODIFY("iocpu:sci0")
	MCFG_H8_SCI_TX_CALLBACK(DEVWRITELINE(":subcpu:sci0", h8_sci_device, rx_w))
	MCFG_DEVICE_MODIFY("subcpu:sci0")
	MCFG_H8_SCI_TX_CALLBACK(DEVWRITELINE(":iocpu:sci0", h8_sci_device, rx_w))

	MCFG_QUANTUM_TIME(attotime::from_hz(2*115200))

	MCFG_NAMCO_SETTINGS_ADD("namco_settings")

	MCFG_RTC4543_ADD("rtc", XTAL_32_768kHz)
	MCFG_RTC4543_DATA_CALLBACK(DEVWRITELINE("subcpu:sci1", h8_sci_device, rx_w))

	MCFG_LINE_DISPATCH_ADD("clk_dispatch", 2)
	MCFG_LINE_DISPATCH_FWD_CB(0, 2, DEVWRITELINE(":rtc", rtc4543_device, clk_w)) MCFG_DEVCB_INVERT
	MCFG_LINE_DISPATCH_FWD_CB(1, 2, DEVWRITELINE(":namco_settings", namco_settings_device, clk_w))

	MCFG_DEVICE_MODIFY("subcpu:sci1")
	MCFG_H8_SCI_TX_CALLBACK(DEVWRITELINE(":namco_settings", namco_settings_device, data_w))
	MCFG_H8_SCI_CLK_CALLBACK(DEVWRITELINE(":clk_dispatch", devcb_line_dispatch_device<2>, in_w))

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(VSYNC1)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // Not in any way accurate
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 479)
	MCFG_SCREEN_UPDATE_DRIVER(namcos23_state, screen_update)
	MCFG_SCREEN_VBLANK_DRIVER(namcos23_state, sub_irq)

	MCFG_PALETTE_ADD("palette", 0x8000)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", namcos23)

	MCFG_VIDEO_START_OVERRIDE(namcos23_state,s23)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_C352_ADD("c352", C352CLOCK, C352DIV)
	MCFG_SOUND_ROUTE(0, "rspeaker", 1.00)
	MCFG_SOUND_ROUTE(1, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(2, "rspeaker", 1.00)
	MCFG_SOUND_ROUTE(3, "lspeaker", 1.00)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( s23, namcos23_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", R4650BE, BUSCLOCK*4)
	MCFG_MIPS3_ICACHE_SIZE(8192)   // VERIFIED
	MCFG_MIPS3_DCACHE_SIZE(8192)   // VERIFIED
	MCFG_CPU_PROGRAM_MAP(s23_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", namcos23_state, interrupt)

	MCFG_CPU_ADD("subcpu", H83002, H8CLOCK )
	MCFG_CPU_PROGRAM_MAP( s23h8rwmap )
	MCFG_CPU_IO_MAP( s23h8iomap )

	// Timer at 115200*16 for the jvs serial clock
	MCFG_DEVICE_MODIFY(":subcpu:sci0")
	MCFG_H8_SCI_SET_EXTERNAL_CLOCK_PERIOD(attotime::from_hz(JVSCLOCK/8))

	MCFG_CPU_ADD("iocpu", H83334, JVSCLOCK )
	MCFG_CPU_PROGRAM_MAP( s23iobrdmap )
	MCFG_CPU_IO_MAP( s23iobrdiomap )

	MCFG_DEVICE_MODIFY("iocpu:sci0")
	MCFG_H8_SCI_TX_CALLBACK(DEVWRITELINE(":subcpu:sci0", h8_sci_device, rx_w))
	MCFG_DEVICE_MODIFY("subcpu:sci0")
	MCFG_H8_SCI_TX_CALLBACK(DEVWRITELINE(":iocpu:sci0", h8_sci_device, rx_w))

	MCFG_QUANTUM_TIME(attotime::from_hz(2*115200))

	MCFG_NAMCO_SETTINGS_ADD("namco_settings")

	MCFG_RTC4543_ADD("rtc", XTAL_32_768kHz)
	MCFG_RTC4543_DATA_CALLBACK(DEVWRITELINE("subcpu:sci1", h8_sci_device, rx_w))

	MCFG_LINE_DISPATCH_ADD("clk_dispatch", 2)
	MCFG_LINE_DISPATCH_FWD_CB(0, 2, DEVWRITELINE(":rtc", rtc4543_device, clk_w)) MCFG_DEVCB_INVERT
	MCFG_LINE_DISPATCH_FWD_CB(1, 2, DEVWRITELINE(":namco_settings", namco_settings_device, clk_w))

	MCFG_DEVICE_MODIFY("subcpu:sci1")
	MCFG_H8_SCI_TX_CALLBACK(DEVWRITELINE(":namco_settings", namco_settings_device, data_w))
	MCFG_H8_SCI_CLK_CALLBACK(DEVWRITELINE(":clk_dispatch", devcb_line_dispatch_device<2>, in_w))

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(VSYNC1)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // Not in any way accurate
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 479)
	MCFG_SCREEN_UPDATE_DRIVER(namcos23_state, screen_update)
	MCFG_SCREEN_VBLANK_DRIVER(namcos23_state, sub_irq)

	MCFG_PALETTE_ADD("palette", 0x8000)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", namcos23)

	MCFG_VIDEO_START_OVERRIDE(namcos23_state,s23)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_C352_ADD("c352", C352CLOCK, C352DIV)
	MCFG_SOUND_ROUTE(0, "rspeaker", 1.00)
	MCFG_SOUND_ROUTE(1, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(2, "rspeaker", 1.00)
	MCFG_SOUND_ROUTE(3, "lspeaker", 1.00)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( timecrs2, s23 )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("iocpu")
	MCFG_CPU_PROGRAM_MAP( timecrs2iobrdmap )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( gmen, s23 )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(BUSCLOCK*5)
	MCFG_CPU_PROGRAM_MAP(gmen_mips_map)

	MCFG_CPU_ADD("gmen_sh2", SH2, XTAL_28_7MHz)
	MCFG_CPU_PROGRAM_MAP(gmen_sh2_map)

	MCFG_MACHINE_RESET_OVERRIDE(namcos23_state,gmen)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( ss23, namcos23_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", R4650BE, BUSCLOCK*5)
	MCFG_MIPS3_ICACHE_SIZE(8192)   // VERIFIED
	MCFG_MIPS3_DCACHE_SIZE(8192)   // VERIFIED
	MCFG_CPU_PROGRAM_MAP(s23_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", namcos23_state, interrupt)

	MCFG_CPU_ADD("subcpu", H83002, H8CLOCK )
	MCFG_CPU_PROGRAM_MAP( s23h8rwmap )
	MCFG_CPU_IO_MAP( s23h8iomap )

	// Timer at 115200*16 for the jvs serial clock
	MCFG_DEVICE_MODIFY(":subcpu:sci0")
	MCFG_H8_SCI_SET_EXTERNAL_CLOCK_PERIOD(attotime::from_hz(JVSCLOCK/8))

	MCFG_QUANTUM_TIME(attotime::from_hz(2*115200))

	MCFG_NAMCO_SETTINGS_ADD("namco_settings")

	MCFG_RTC4543_ADD("rtc", XTAL_32_768kHz)
	MCFG_RTC4543_DATA_CALLBACK(DEVWRITELINE("subcpu:sci1", h8_sci_device, rx_w))

	MCFG_LINE_DISPATCH_ADD("clk_dispatch", 2)
	MCFG_LINE_DISPATCH_FWD_CB(0, 2, DEVWRITELINE(":rtc", rtc4543_device, clk_w)) MCFG_DEVCB_INVERT
	MCFG_LINE_DISPATCH_FWD_CB(1, 2, DEVWRITELINE(":namco_settings", namco_settings_device, clk_w))

	MCFG_DEVICE_MODIFY("subcpu:sci1")
	MCFG_H8_SCI_TX_CALLBACK(DEVWRITELINE(":namco_settings", namco_settings_device, data_w))
	MCFG_H8_SCI_CLK_CALLBACK(DEVWRITELINE(":clk_dispatch", devcb_line_dispatch_device<2>, in_w))

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(VSYNC1)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // Not in any way accurate
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 479)
	MCFG_SCREEN_UPDATE_DRIVER(namcos23_state, screen_update)
	MCFG_SCREEN_VBLANK_DRIVER(namcos23_state, sub_irq)

	MCFG_PALETTE_ADD("palette", 0x8000)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", namcos23)

	MCFG_VIDEO_START_OVERRIDE(namcos23_state,s23)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_C352_ADD("c352", C352CLOCK, C352DIV)
	MCFG_SOUND_ROUTE(0, "rspeaker", 1.00)
	MCFG_SOUND_ROUTE(1, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(2, "rspeaker", 1.00)
	MCFG_SOUND_ROUTE(3, "lspeaker", 1.00)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( timecrs2v4a, ss23 )
	/* basic machine hardware */
	MCFG_CPU_ADD("iocpu", H83334, JVSCLOCK )
	MCFG_CPU_PROGRAM_MAP( timecrs2iobrdmap )
	MCFG_CPU_IO_MAP( s23iobrdiomap )

	MCFG_DEVICE_MODIFY("iocpu:sci0")
	MCFG_H8_SCI_TX_CALLBACK(DEVWRITELINE(":subcpu:sci0", h8_sci_device, rx_w))
	MCFG_DEVICE_MODIFY("subcpu:sci0")
	MCFG_H8_SCI_TX_CALLBACK(DEVWRITELINE(":iocpu:sci0", h8_sci_device, rx_w))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ss23e2, ss23 )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(BUSCLOCK*6)

	MCFG_CPU_ADD("iocpu", H83334, JVSCLOCK )
	MCFG_CPU_PROGRAM_MAP( s23iobrdmap )
	MCFG_CPU_IO_MAP( s23iobrdiomap )

	MCFG_DEVICE_MODIFY("iocpu:sci0")
	MCFG_H8_SCI_TX_CALLBACK(DEVWRITELINE(":subcpu:sci0", h8_sci_device, rx_w))
	MCFG_DEVICE_MODIFY("subcpu:sci0")
	MCFG_H8_SCI_TX_CALLBACK(DEVWRITELINE(":iocpu:sci0", h8_sci_device, rx_w))
MACHINE_CONFIG_END



ROM_START( rapidrvr )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "rd3verc.ic2",  0x000000, 0x200000, CRC(c15c0f30) SHA1(9f529232818f3e184f81f62408a5cad615b05613) )
	ROM_LOAD16_BYTE( "rd3verc.ic1",  0x000001, 0x200000, CRC(9d7f4411) SHA1(d049efaa539d36ed0f73ca3f50a8f7112e67f865) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "rd3verc.ic3",  0x000000, 0x080000, CRC(6e26fbaf) SHA1(4ab6637d22f0d26f7e1d10e9c80059c56f64303d) )

	ROM_REGION( 0x40000, "iocpu", 0 )   /* I/O board HD643334 H8/3334 MCU code */
	ROM_LOAD( "asca1_io-a.ic2", 0x000000, 0x040000, CRC(77cdf69a) SHA1(497af1059f85c07bea2dd0d303481623f6019dcf) )

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

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate roms */
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

	ROM_REGION( 0x40000, "iocpu", 0 )   /* I/O board HD643334 H8/3334 MCU code */
	ROM_LOAD( "asca1_io-a.ic2", 0x000000, 0x040000, CRC(77cdf69a) SHA1(497af1059f85c07bea2dd0d303481623f6019dcf) )

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

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate roms */
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

	ROM_REGION( 0x40000, "iocpu", 0 )   /* I/O board HD643334 H8/3334 MCU code */
	ROM_LOAD( "asca1_io-a.ic2", 0x000000, 0x040000, CRC(77cdf69a) SHA1(497af1059f85c07bea2dd0d303481623f6019dcf) )

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

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate roms */
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

	ROM_REGION( 0x40000, "iocpu", 0 )   /* I/O board HD643334 H8/3334 MCU code */
	ROM_LOAD( "asca1_io-a.ic2", 0x000000, 0x040000, CRC(77cdf69a) SHA1(497af1059f85c07bea2dd0d303481623f6019dcf) )

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

	ROM_REGION( 0x400000, "dups", 0 )   /* duplicate roms */
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

	ROM_REGION( 0x40000, "iocpu", 0 )   /* I/O board HD643334 H8/3334 MCU code */
	ROM_LOAD( "asca-3a.ic14", 0x000000, 0x040000, CRC(8e9266e5) SHA1(ffa8782ca641d71d57df23ed1c5911db05d3df97) )

	ROM_REGION( 0x20000, "exioboard", 0 )   /* "extra" I/O board (uses Fujitsu MB90611A MCU) */
	ROM_LOAD( "mg1prog0a.3a", 0x000000, 0x020000, CRC(b2b5be8f) SHA1(803652b7b8fde2196b7fb742ba8b9843e4fcd2de) )

	ROM_REGION32_BE( 0x2000000, "data", ROMREGION_ERASEFF ) /* data roms */
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

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate roms */
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

	ROM_REGION( 0x40000, "iocpu", 0 )   /* I/O board HD643334 H8/3334 MCU code */
	ROM_LOAD( "asca-3a.ic14", 0x000000, 0x040000, CRC(8e9266e5) SHA1(ffa8782ca641d71d57df23ed1c5911db05d3df97) )

	ROM_REGION( 0x20000, "exioboard", 0 )   /* "extra" I/O board (uses Fujitsu MB90611A MCU) */
	ROM_LOAD( "mg1prog0a.3a", 0x000000, 0x020000, CRC(b2b5be8f) SHA1(803652b7b8fde2196b7fb742ba8b9843e4fcd2de) )

	ROM_REGION32_BE( 0x2000000, "data", ROMREGION_ERASEFF ) /* data roms */
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

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate roms */
	ROM_LOAD( "mg1cgll.5m",   0x000000, 0x800000, CRC(175dfe34) SHA1(66ae35b0084159aea1afeb1a6486fffa635992b5) )
	ROM_LOAD( "mg1cglm.5k",   0x000000, 0x800000, CRC(b3e648e7) SHA1(98018ae2276f905a7f74e1dab540a44247524436) )
	ROM_LOAD( "mg1cgum.5j",   0x000000, 0x800000, CRC(46a77d73) SHA1(132ce2452ee68ba374e98b59032ac0a1a277078d) )
	ROM_LOAD( "mg1ccrl.7m",   0x000000, 0x400000, CRC(5372e300) SHA1(63a49782289ed93a321ca7d193241fb83ca97e6b) )
	ROM_LOAD( "mg1ccrh.7k",   0x400000, 0x200000, CRC(2e77597d) SHA1(58dd83c1b0c08115e728c5e7dea5e62135b821ba) )
ROM_END


ROM_START( motoxgov1a )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "mg1vera.ic2",  0x000000, 0x200000, CRC(5ba13d9e) SHA1(7f6484df644772f2478155c05844532f8abbd196) )
	ROM_LOAD16_BYTE( "mg1vera.ic1",  0x000001, 0x200000, CRC(6b2bda52) SHA1(922ea739c8a62c7147126bf20ed3ffe8faec8842) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "mg3vera.ic3",  0x000000, 0x080000, CRC(9e3d46a8) SHA1(9ffa5b91ea51cc0fb97def25ce47efa3441f3c6f) )

	ROM_REGION( 0x40000, "iocpu", 0 )   /* I/O board HD643334 H8/3334 MCU code */
	ROM_LOAD( "asca-3a.ic14", 0x000000, 0x040000, CRC(8e9266e5) SHA1(ffa8782ca641d71d57df23ed1c5911db05d3df97) )

	ROM_REGION( 0x20000, "exioboard", 0 )   /* "extra" I/O board (uses Fujitsu MB90611A MCU) */
	ROM_LOAD( "mg1prog0a.3a", 0x000000, 0x020000, CRC(b2b5be8f) SHA1(803652b7b8fde2196b7fb742ba8b9843e4fcd2de) )

	ROM_REGION32_BE( 0x2000000, "data", ROMREGION_ERASEFF ) /* data roms */
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

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate roms */
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

	ROM_REGION( 0x40000, "iocpu", 0 )   /* I/O board HD643334 H8/3334 MCU code */
	ROM_LOAD( "asca-3a.ic14", 0x000000, 0x040000, CRC(8e9266e5) SHA1(ffa8782ca641d71d57df23ed1c5911db05d3df97) )

	ROM_REGION( 0x20000, "exioboard", 0 )   /* "extra" I/O board (uses Fujitsu MB90611A MCU) */
	ROM_LOAD( "mg1prog0a.3a", 0x000000, 0x020000, CRC(b2b5be8f) SHA1(803652b7b8fde2196b7fb742ba8b9843e4fcd2de) )

	ROM_REGION32_BE( 0x2000000, "data", ROMREGION_ERASEFF ) /* data roms */
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

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate roms */
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
	ROM_LOAD16_WORD_SWAP( "tss3verb.3",   0x000000, 0x080000, CRC(41e41994) SHA1(eabc1a307c329070bfc6486cb68169c94ff8a162) )

	ROM_REGION( 0x40000, "iocpu", 0 )   /* I/O board HD643334 H8/3334 MCU code */
	ROM_LOAD( "tssioprog.ic3", 0x000000, 0x040000, CRC(edad4538) SHA1(1330189184a636328d956c0e435f8d9ad2e96a80) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data roms */
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
	ROM_LOAD16_BYTE( "tss2verb.ic2", 0x000000, 0x200000, BAD_DUMP CRC(9f56a4df) SHA1(5ecb3cd93726ab6be02762853fd6a45266d6c0bc) )
	ROM_LOAD16_BYTE( "tss2verb.ic1", 0x000001, 0x200000, BAD_DUMP CRC(aa147f71) SHA1(e00267d1a8286942c83dc35289ad65bd3cb6d8db) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "tss3verb.3",   0x000000, 0x080000, CRC(41e41994) SHA1(eabc1a307c329070bfc6486cb68169c94ff8a162) )

	ROM_REGION( 0x40000, "iocpu", 0 )   /* I/O board HD643334 H8/3334 MCU code */
	ROM_LOAD( "tssioprog.ic3", 0x000000, 0x040000, CRC(edad4538) SHA1(1330189184a636328d956c0e435f8d9ad2e96a80) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data roms */
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
	ROM_LOAD16_WORD_SWAP( "tss4vera.3",   0x000000, 0x080000, CRC(41e41994) SHA1(eabc1a307c329070bfc6486cb68169c94ff8a162) )

	ROM_REGION( 0x40000, "iocpu", 0 )   /* I/O board HD643334 H8/3334 MCU code */
	ROM_LOAD( "tssioprog.ic3", 0x000000, 0x040000, CRC(edad4538) SHA1(1330189184a636328d956c0e435f8d9ad2e96a80) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data roms */
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

	ROM_REGION( 0x40000, "iocpu", 0 )   /* I/O board MB90F574 MCU code */
	ROM_LOAD( "fcaf10.bin", 0x000000, 0x040000, NO_DUMP ) // 256KB internal flashrom

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data roms */
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

	ROM_REGION( 0x40000, "iocpu", 0 )   /* I/O board MB90F574 MCU code */
	ROM_LOAD( "fcaf10.bin", 0x000000, 0x040000, NO_DUMP ) // 256KB internal flashrom

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data roms */
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

	ROM_REGION( 0x40000, "iocpu", 0 )   /* I/O board HD643334 H8/3334 MCU code */
	ROM_LOAD( "asc5_io-a.ic14", 0x000000, 0x020000, CRC(5964767f) SHA1(320db5e78ae23c5f94e368432d51573b409995db) )

	ROM_REGION( 0x80000, "ffb", 0 ) /* STR steering force-feedback board code */
	ROM_LOAD( "ro1_str-0a.ic16", 0x000000, 0x080000, CRC(27d39e1f) SHA1(6161cbb27c964ffab1db3b3c1f073ec514876e61) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data roms */
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
	ROM_LOAD32_WORD_SWAP( "ro1pt0h.7a",   0x0000000, 0x400000, CRC(6ef742ab) SHA1(500ce413b2463a555237de7bcc9627d1082c9b52) )
	ROM_LOAD32_WORD_SWAP( "ro1pt0l.7c",   0x0000002, 0x400000, CRC(f4b88bd0) SHA1(cc642d959645730b03ef01e6dbb5d0077bce7163) )
	ROM_LOAD32_WORD_SWAP( "ro1pt1h.5a",   0x0800000, 0x400000, CRC(428bf573) SHA1(6be159e1cf7ef38639610c347fd2322ab9911a70) )
	ROM_LOAD32_WORD_SWAP( "ro1pt1l.5c",   0x0800002, 0x400000, CRC(f3df1d13) SHA1(9f96c99bd3537940a532d3dccb69a1c7d8c6be63) )
	ROM_LOAD32_WORD_SWAP( "ro1pt2h.4a",   0x1000000, 0x400000, CRC(e1abdbc9) SHA1(91827af01cb83f4422d7329c8eea52bb57d7d57e) )
	ROM_LOAD32_WORD_SWAP( "ro1pt2l.4c",   0x1000002, 0x400000, CRC(c64f5cdc) SHA1(e7261f3a56718f304127cc85c08d0b32525dc1cd) )
	ROM_LOAD32_WORD_SWAP( "ro1pt3h.3a",   0x1800000, 0x400000, CRC(ef4685f6) SHA1(930037cac4aae9892278aa322844d03c773c70f7) )
	ROM_LOAD32_WORD_SWAP( "ro1pt3l.3c",   0x1800002, 0x400000, CRC(07d27009) SHA1(770001bee9d7ace337db8a42bf377678b2b5d5fb) )

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


ROM_START( finfurl2 )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "29f016.ic2",   0x000000, 0x200000, CRC(13cbc545) SHA1(3e67a7bfbb1c1374e8e3996a0c09e4861b0dca14) )
	ROM_LOAD16_BYTE( "29f016.ic1",   0x000001, 0x200000, CRC(5b04e4f2) SHA1(8099fc3deab9ed14a2484a774666fbd928330de8) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "m29f400.ic3",  0x000000, 0x080000, CRC(9fd69bbd) SHA1(53a9bf505de70495dcccc43fdc722b3381aad97c) )

	ROM_REGION( 0x40000, "iocpu", 0 )   /* I/O board HD643334 H8/3334 MCU code */
	ROM_LOAD( "asca-3a.ic14", 0x000000, 0x040000, CRC(8e9266e5) SHA1(ffa8782ca641d71d57df23ed1c5911db05d3df97) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data roms */
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
	ROM_LOAD32_WORD_SWAP( "ffs1pt0h.7a",  0x0000000, 0x400000, CRC(79b9b019) SHA1(ca2bbabd949fec91001a30b63f7343520028cde0) )
	ROM_LOAD32_WORD_SWAP( "ffs1pt0l.7c",  0x0000002, 0x400000, CRC(383cbfba) SHA1(0784ac2d709bee6653c95f80fedf7f98ca79357f) )
	ROM_LOAD32_WORD_SWAP( "ffs1pt1h.5a",  0x0800000, 0x400000, CRC(2dba59d0) SHA1(34d4c415b5635338511ff3578eb3c00e2b6cd7d4) )
	ROM_LOAD32_WORD_SWAP( "ffs1pt1l.5c",  0x0800002, 0x400000, CRC(ba0fff5b) SHA1(d5a6db4de60657d46228e85ed09ed7f0ecbc7975) )
	ROM_LOAD32_WORD_SWAP( "ffs1pt2h.4a",  0x1000000, 0x400000, CRC(26ea01e8) SHA1(9af096c99e6835e21b1b78dfce07040f50f8c922) )
	ROM_LOAD32_WORD_SWAP( "ffs1pt2l.4c",  0x1000002, 0x400000, CRC(c5199c1b) SHA1(8f1a70c8edb2791a099b4911353af6250a5d0e8a) )
	ROM_LOAD32_WORD_SWAP( "ffs1pt3h.3a",  0x1800000, 0x400000, CRC(48226e9f) SHA1(f099b2929d49903a33b4dab80972c3ce0ddb6ca2) )
	ROM_LOAD32_WORD_SWAP( "ffs1pt3l.3c",  0x1800002, 0x400000, CRC(2381611a) SHA1(a3d948bf910dcfd9f47c65c56b9920f58c42fed5) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
	ROM_LOAD( "ffs1wavel.2c", 0x000000, 0x800000, CRC(67ba16cf) SHA1(00b38617c2185b9a3bf279962ad0c21a7287256f) )
	ROM_LOAD( "ffs1waveh.2a", 0x800000, 0x800000, CRC(178e8bd3) SHA1(8ab1a97003914f70b09e96c5924f3a839fe634c7) )
ROM_END


ROM_START( finfurl2j )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "29f016_jap1.ic2", 0x000000, 0x200000, CRC(0215125d) SHA1(a99f601441c152b0b00f4811e5752c71897b1ed4) )
	ROM_LOAD16_BYTE( "29f016_jap1.ic1", 0x000001, 0x200000, CRC(38c9ae96) SHA1(b50afc7276662267ff6460f82d0e5e8b00b341ea) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "m29f400.ic3",  0x000000, 0x080000, CRC(9fd69bbd) SHA1(53a9bf505de70495dcccc43fdc722b3381aad97c) )

	ROM_REGION( 0x40000, "iocpu", 0 )   /* I/O board HD643334 H8/3334 MCU code */
	ROM_LOAD( "asca-3a.ic14", 0x000000, 0x040000, CRC(8e9266e5) SHA1(ffa8782ca641d71d57df23ed1c5911db05d3df97) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data roms */
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
	ROM_LOAD32_WORD_SWAP( "ffs1pt0h.7a",  0x0000000, 0x400000, CRC(79b9b019) SHA1(ca2bbabd949fec91001a30b63f7343520028cde0) )
	ROM_LOAD32_WORD_SWAP( "ffs1pt0l.7c",  0x0000002, 0x400000, CRC(383cbfba) SHA1(0784ac2d709bee6653c95f80fedf7f98ca79357f) )
	ROM_LOAD32_WORD_SWAP( "ffs1pt1h.5a",  0x0800000, 0x400000, CRC(2dba59d0) SHA1(34d4c415b5635338511ff3578eb3c00e2b6cd7d4) )
	ROM_LOAD32_WORD_SWAP( "ffs1pt1l.5c",  0x0800002, 0x400000, CRC(ba0fff5b) SHA1(d5a6db4de60657d46228e85ed09ed7f0ecbc7975) )
	ROM_LOAD32_WORD_SWAP( "ffs1pt2h.4a",  0x1000000, 0x400000, CRC(26ea01e8) SHA1(9af096c99e6835e21b1b78dfce07040f50f8c922) )
	ROM_LOAD32_WORD_SWAP( "ffs1pt2l.4c",  0x1000002, 0x400000, CRC(c5199c1b) SHA1(8f1a70c8edb2791a099b4911353af6250a5d0e8a) )
	ROM_LOAD32_WORD_SWAP( "ffs1pt3h.3a",  0x1800000, 0x400000, CRC(48226e9f) SHA1(f099b2929d49903a33b4dab80972c3ce0ddb6ca2) )
	ROM_LOAD32_WORD_SWAP( "ffs1pt3l.3c",  0x1800002, 0x400000, CRC(2381611a) SHA1(a3d948bf910dcfd9f47c65c56b9920f58c42fed5) )
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

	ROM_REGION( 0x40000, "iocpu", 0 )   /* I/O board HD643334 H8/3334 MCU code */
	ROM_LOAD( "asca-3a.ic14", 0x000000, 0x040000, CRC(8e9266e5) SHA1(ffa8782ca641d71d57df23ed1c5911db05d3df97) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data roms */
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

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate roms */
	ROM_LOAD( "pnp1cguu.4f",  0x000000, 0x800000, CRC(cd64f57f) SHA1(8780270298e0823db1acbbf79396788df0c3c19c) )
	ROM_LOAD( "pnp1cgum.5j",  0x000000, 0x800000, CRC(206217ca) SHA1(9c095bba7764f3405c3fab10513b9b78981ec44d) )
	ROM_LOAD( "pnp1cgll.5m",  0x000000, 0x800000, CRC(d03932cf) SHA1(49240e44923cc6e815e9457b6290fd18466658af) )
	ROM_LOAD( "pnp1cglm.4k",  0x000000, 0x800000, CRC(abf4ccf2) SHA1(3848e26d0ba6c872bbc6d5e0eb23a9d4b34152d5) )
	ROM_LOAD( "pnp1ccrl.7m",  0x000000, 0x200000, CRC(b7bc43c2) SHA1(f4b470540194486ca6822f438fc1d4700cfb2ab1) )
	ROM_LOAD( "pnp1ccrh.7k",  0x000000, 0x200000, CRC(caaf1b73) SHA1(b436992817ab4e4dad05e7429eb102d4fb57fa6a) )
ROM_END


ROM_START( panicprkj )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "pnp1verb.ic2", 0x000000, 0x200000, CRC(a46e34f8) SHA1(c84eb701a1e01e706dea515acaaf6d98ad53f453) )
	ROM_LOAD16_BYTE( "pnp1verb.ic1", 0x000001, 0x200000, CRC(4de52a64) SHA1(7edd974d52e17bcbdb029f718c06d603ed558d90) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "pnp1vera.ic3", 0x000000, 0x080000, CRC(fe4bc6f4) SHA1(2114dc4bc63d589e6c3b26a73dbc60924f3b1765) )

	ROM_REGION( 0x40000, "iocpu", 0 )   /* I/O board HD643334 H8/3334 MCU code */
	ROM_LOAD( "asca-3a.ic14", 0x000000, 0x040000, CRC(8e9266e5) SHA1(ffa8782ca641d71d57df23ed1c5911db05d3df97) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data roms */
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

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate roms */
	ROM_LOAD( "pnp1cguu.4f",  0x000000, 0x800000, CRC(cd64f57f) SHA1(8780270298e0823db1acbbf79396788df0c3c19c) )
	ROM_LOAD( "pnp1cgum.5j",  0x000000, 0x800000, CRC(206217ca) SHA1(9c095bba7764f3405c3fab10513b9b78981ec44d) )
	ROM_LOAD( "pnp1cgll.5m",  0x000000, 0x800000, CRC(d03932cf) SHA1(49240e44923cc6e815e9457b6290fd18466658af) )
	ROM_LOAD( "pnp1cglm.4k",  0x000000, 0x800000, CRC(abf4ccf2) SHA1(3848e26d0ba6c872bbc6d5e0eb23a9d4b34152d5) )
	ROM_LOAD( "pnp1ccrl.7m",  0x000000, 0x200000, CRC(b7bc43c2) SHA1(f4b470540194486ca6822f438fc1d4700cfb2ab1) )
	ROM_LOAD( "pnp1ccrh.7k",  0x000000, 0x200000, CRC(caaf1b73) SHA1(b436992817ab4e4dad05e7429eb102d4fb57fa6a) )
ROM_END


ROM_START( gunwars )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "gm1verb.ic2",  0x000000, 0x200000, CRC(401f8264) SHA1(281f245ae0fbc2b82248c7aacaa5dfcdb114e2ee) )
	ROM_LOAD16_BYTE( "gm1verb.ic1",  0x000001, 0x200000, CRC(f9fd0f2b) SHA1(53dadd49d0d43f0693c84853ba3de1b5faa9e1d8) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "gm1vera.ic3",  0x000000, 0x080000, CRC(5582fdd4) SHA1(8aae8bc6688d531888f2de509c07502ee355b3ab) )

	ROM_REGION( 0x40000, "iocpu", 0 )   /* I/O board HD643334 H8/3334 MCU code. "ASCA-5;Ver 2.09;JPN,Multipurpose" */
	ROM_LOAD( "asc5_io-a.ic14", 0x000000, 0x020000, CRC(5964767f) SHA1(320db5e78ae23c5f94e368432d51573b409995db) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data roms */
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

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate roms */
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

	ROM_REGION( 0x40000, "iocpu", 0 )   /* I/O board HD643334 H8/3334 MCU code. "ASCA-5;Ver 2.09;JPN,Multipurpose" */
	ROM_LOAD( "asc5_io-a.ic14", 0x000000, 0x020000, CRC(5964767f) SHA1(320db5e78ae23c5f94e368432d51573b409995db) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data roms */
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

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate roms */
	ROM_LOAD( "gm1cguu.4f",   0x000000, 0x800000, CRC(26a74698) SHA1(3f07d273abb3f2552dc6a29300f5dc2f2744c852) )
	ROM_LOAD( "gm1cgum.5j",   0x000000, 0x800000, CRC(a7728944) SHA1(c187c6d66128554fcecc96e81d4f5396197e8280) )
	ROM_LOAD( "gm1cgll.5m",   0x000000, 0x800000, CRC(936c0079) SHA1(3aec8caada35b7ed790bb3a8bcf6e01cad068fcd) )
	ROM_LOAD( "gm1cglm.5k",   0x000000, 0x800000, CRC(e2ee5493) SHA1(1ffd74646796ad554d7967ba9fc18deab4fedadf) )
	ROM_LOAD( "gm1ccrl.7m",   0x000000, 0x400000, CRC(2c54c182) SHA1(538dfb04653f8d86f976e702456bf4da97e3fda9) )
	ROM_LOAD( "gm1ccrh.7k",   0x000000, 0x200000, CRC(8563ef01) SHA1(59f09a08008a71a4bb12bd43a1b5dbe633d3061d) )
ROM_END


ROM_START( downhill )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_BYTE( "dh3vera.ic2",  0x000000, 0x200000, CRC(5d9952e9) SHA1(d38422330bd708c247b9968429fbff36fe706598) )
	ROM_LOAD16_BYTE( "dh3vera.ic1",  0x000001, 0x200000, CRC(64a236f3) SHA1(aac59e0db5cfefc4b442e6c3a5189a8418742201) )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "dh3vera.ic3",  0x000000, 0x080000, CRC(98f9fc8b) SHA1(5152b9e11773033a26da11d1f3774a261e61a2c5) )

	ROM_REGION( 0x40000, "iocpu", 0 )   /* I/O board HD643334 H8/3334 MCU code. "ASCA-3;Ver 2.04;JPN,Multipurpose + Rotary Encoder" */
	ROM_LOAD( "asc3_io-c.ic14", 0x000000, 0x020000, CRC(2f272a7b) SHA1(9d7ebe274c0d26f5f38747224d42d0375e2ed14c) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data roms */
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

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate roms */
	ROM_LOAD( "dh1cguu.4f",   0x000000, 0x800000, CRC(66cb0dd7) SHA1(1f67320f150f1b55c97eae4b9fe4890fabc8dc7e) )
	ROM_LOAD( "dh1cgum.5j",   0x000000, 0x800000, CRC(1044d0a0) SHA1(e0bf843616e166495fcdc76f076eb53a28287d30) )
	ROM_LOAD( "dh1cgll.5m",   0x000000, 0x800000, CRC(c0d5ad87) SHA1(bc1992516c63aebdae0322def77f082d799a327a) )
	ROM_LOAD( "dh1cglm.5k",   0x000000, 0x800000, CRC(5d9a5e35) SHA1(d746abb45f04aa4eb9d43d9c79051e71bf024e38) )
	ROM_LOAD( "dh1ccrl.7m",   0x000000, 0x400000, CRC(65c857df) SHA1(5d67b17cf272f042b4264d9871d6e4088c20b788) )
	ROM_LOAD( "dh1ccrh.7k",   0x000000, 0x200000, CRC(f21c482d) SHA1(bfcead2ff3d10f996ac0bf81470d050bd6374156) )
ROM_END


ROM_START( crszone )
	ROM_REGION32_BE( 0x800000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_WORD_SWAP( "cszo4verb.ic4", 0x400000, 0x400000, CRC(6192533d) SHA1(d102b91fe193bf255ea4e57a2bd964aa1cdfd21d) )
	ROM_CONTINUE( 0x000000, 0x400000 )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "cszo3verb.ic1", 0x000000, 0x080000, CRC(c790743b) SHA1(5fa7b83a7a1b1105a3aa0870b782cf2741b7d11c) )

	ROM_REGION( 0x40000, "iocpu", 0 )   /* I/O board HD643334 H8/3334 MCU code. "MIU-I/O;Ver2.05;JPN,GUN-EXTENTION" */
	ROM_LOAD( "csz1prg0a.8f", 0x000000, 0x020000, CRC(8edc36b3) SHA1(b5df211988d856572fcc313480e693c8561784e4) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data roms */
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

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate roms */
	ROM_LOAD( "csz1cguu.4f",  0x000000, 0x800000, CRC(e1d1bf24) SHA1(daf2c68e2d9a8f313d262d221cc990c93dfdf22f) )
	ROM_LOAD( "csz1cgum.5j",  0x000000, 0x800000, CRC(913c98b5) SHA1(b952dbc19053796077d4f33e8da836893e933b12) )
	ROM_LOAD( "csz1cgll.5m",  0x000000, 0x800000, CRC(0bcd41f2) SHA1(80b74f9398e8bd074f79a14490d06cfeb875c874) )
	ROM_LOAD( "csz1cglm.5k",  0x000000, 0x800000, CRC(d4af93d1) SHA1(0df37b793ce8da02d14f714722382786ae5d3ce2) )
	ROM_LOAD( "csz1ccrl.7m",  0x000000, 0x400000, CRC(1c20768d) SHA1(6cf4280e26f3625d6f750837bf344163e7e93c3d) )
	ROM_LOAD( "csz1ccrh.7k",  0x000000, 0x200000, CRC(bc2fa03c) SHA1(e63d8e75494a383bf9a213edfa9c472a010f8efe) )
ROM_END


ROM_START( crszonev4a )
	ROM_REGION32_BE( 0x800000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_WORD_SWAP( "cszo4vera.ic4", 0x400000, 0x400000, CRC(cabee8c3) SHA1(4887b8550038c072f988c5999d57ec40e82e4072) )
	ROM_CONTINUE( 0x000000, 0x400000 )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "cszo3verb.ic1", 0x000000, 0x080000, CRC(c790743b) SHA1(5fa7b83a7a1b1105a3aa0870b782cf2741b7d11c) )

	ROM_REGION( 0x40000, "iocpu", 0 )   /* I/O board HD643334 H8/3334 MCU code. "MIU-I/O;Ver2.05;JPN,GUN-EXTENTION" */
	ROM_LOAD( "csz1prg0a.8f", 0x000000, 0x020000, CRC(8edc36b3) SHA1(b5df211988d856572fcc313480e693c8561784e4) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data roms */
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

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate roms */
	ROM_LOAD( "csz1cguu.4f",  0x000000, 0x800000, CRC(e1d1bf24) SHA1(daf2c68e2d9a8f313d262d221cc990c93dfdf22f) )
	ROM_LOAD( "csz1cgum.5j",  0x000000, 0x800000, CRC(913c98b5) SHA1(b952dbc19053796077d4f33e8da836893e933b12) )
	ROM_LOAD( "csz1cgll.5m",  0x000000, 0x800000, CRC(0bcd41f2) SHA1(80b74f9398e8bd074f79a14490d06cfeb875c874) )
	ROM_LOAD( "csz1cglm.5k",  0x000000, 0x800000, CRC(d4af93d1) SHA1(0df37b793ce8da02d14f714722382786ae5d3ce2) )
	ROM_LOAD( "csz1ccrl.7m",  0x000000, 0x400000, CRC(1c20768d) SHA1(6cf4280e26f3625d6f750837bf344163e7e93c3d) )
	ROM_LOAD( "csz1ccrh.7k",  0x000000, 0x200000, CRC(bc2fa03c) SHA1(e63d8e75494a383bf9a213edfa9c472a010f8efe) )
ROM_END


ROM_START( crszonev3b )
	ROM_REGION32_BE( 0x800000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_WORD_SWAP( "cszo3verb.ic4", 0x400000, 0x400000, CRC(4cb26465) SHA1(078dfd0d8c920707df14e9a26658fa63421fcb0b) )
	ROM_CONTINUE( 0x000000, 0x400000 )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "cszo3verb.ic1", 0x000000, 0x080000, CRC(c790743b) SHA1(5fa7b83a7a1b1105a3aa0870b782cf2741b7d11c) )

	ROM_REGION( 0x40000, "iocpu", 0 )   /* I/O board HD643334 H8/3334 MCU code. "MIU-I/O;Ver2.05;JPN,GUN-EXTENTION" */
	ROM_LOAD( "csz1prg0a.8f", 0x000000, 0x020000, CRC(8edc36b3) SHA1(b5df211988d856572fcc313480e693c8561784e4) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data roms */
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

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate roms */
	ROM_LOAD( "csz1cguu.4f",  0x000000, 0x800000, CRC(e1d1bf24) SHA1(daf2c68e2d9a8f313d262d221cc990c93dfdf22f) )
	ROM_LOAD( "csz1cgum.5j",  0x000000, 0x800000, CRC(913c98b5) SHA1(b952dbc19053796077d4f33e8da836893e933b12) )
	ROM_LOAD( "csz1cgll.5m",  0x000000, 0x800000, CRC(0bcd41f2) SHA1(80b74f9398e8bd074f79a14490d06cfeb875c874) )
	ROM_LOAD( "csz1cglm.5k",  0x000000, 0x800000, CRC(d4af93d1) SHA1(0df37b793ce8da02d14f714722382786ae5d3ce2) )
	ROM_LOAD( "csz1ccrl.7m",  0x000000, 0x400000, CRC(1c20768d) SHA1(6cf4280e26f3625d6f750837bf344163e7e93c3d) )
	ROM_LOAD( "csz1ccrh.7k",  0x000000, 0x200000, CRC(bc2fa03c) SHA1(e63d8e75494a383bf9a213edfa9c472a010f8efe) )
ROM_END


ROM_START( crszonev3b2 )
	ROM_REGION32_BE( 0x800000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_WORD_SWAP( "cszo3verb.ic4", 0x400000, 0x400000, CRC(3755b402) SHA1(e169fded9d136af7ce6997868629eed5196b8cdd) ) // sldh
	ROM_CONTINUE( 0x000000, 0x400000 )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "cszo3verb.ic1", 0x000000, 0x080000, CRC(c790743b) SHA1(5fa7b83a7a1b1105a3aa0870b782cf2741b7d11c) )

	ROM_REGION( 0x40000, "iocpu", 0 )   /* I/O board HD643334 H8/3334 MCU code. "MIU-I/O;Ver2.05;JPN,GUN-EXTENTION" */
	ROM_LOAD( "csz1prg0a.8f", 0x000000, 0x020000, CRC(8edc36b3) SHA1(b5df211988d856572fcc313480e693c8561784e4) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data roms */
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

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate roms */
	ROM_LOAD( "csz1cguu.4f",  0x000000, 0x800000, CRC(e1d1bf24) SHA1(daf2c68e2d9a8f313d262d221cc990c93dfdf22f) )
	ROM_LOAD( "csz1cgum.5j",  0x000000, 0x800000, CRC(913c98b5) SHA1(b952dbc19053796077d4f33e8da836893e933b12) )
	ROM_LOAD( "csz1cgll.5m",  0x000000, 0x800000, CRC(0bcd41f2) SHA1(80b74f9398e8bd074f79a14490d06cfeb875c874) )
	ROM_LOAD( "csz1cglm.5k",  0x000000, 0x800000, CRC(d4af93d1) SHA1(0df37b793ce8da02d14f714722382786ae5d3ce2) )
	ROM_LOAD( "csz1ccrl.7m",  0x000000, 0x400000, CRC(1c20768d) SHA1(6cf4280e26f3625d6f750837bf344163e7e93c3d) )
	ROM_LOAD( "csz1ccrh.7k",  0x000000, 0x200000, CRC(bc2fa03c) SHA1(e63d8e75494a383bf9a213edfa9c472a010f8efe) )
ROM_END


ROM_START( crszonev3a )
	ROM_REGION32_BE( 0x800000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_WORD_SWAP( "cszo3vera.ic4", 0x400000, 0x400000, CRC(09b0c91e) SHA1(226c3788d6a50272e2544d04d9ca20df81014fb6) )
	ROM_CONTINUE( 0x000000, 0x400000 )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "cszo3verb.ic1", 0x000000, 0x080000, CRC(c790743b) SHA1(5fa7b83a7a1b1105a3aa0870b782cf2741b7d11c) )

	ROM_REGION( 0x40000, "iocpu", 0 )   /* I/O board HD643334 H8/3334 MCU code. "MIU-I/O;Ver2.05;JPN,GUN-EXTENTION" */
	ROM_LOAD( "csz1prg0a.8f", 0x000000, 0x020000, CRC(8edc36b3) SHA1(b5df211988d856572fcc313480e693c8561784e4) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data roms */
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

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate roms */
	ROM_LOAD( "csz1cguu.4f",  0x000000, 0x800000, CRC(e1d1bf24) SHA1(daf2c68e2d9a8f313d262d221cc990c93dfdf22f) )
	ROM_LOAD( "csz1cgum.5j",  0x000000, 0x800000, CRC(913c98b5) SHA1(b952dbc19053796077d4f33e8da836893e933b12) )
	ROM_LOAD( "csz1cgll.5m",  0x000000, 0x800000, CRC(0bcd41f2) SHA1(80b74f9398e8bd074f79a14490d06cfeb875c874) )
	ROM_LOAD( "csz1cglm.5k",  0x000000, 0x800000, CRC(d4af93d1) SHA1(0df37b793ce8da02d14f714722382786ae5d3ce2) )
	ROM_LOAD( "csz1ccrl.7m",  0x000000, 0x400000, CRC(1c20768d) SHA1(6cf4280e26f3625d6f750837bf344163e7e93c3d) )
	ROM_LOAD( "csz1ccrh.7k",  0x000000, 0x200000, CRC(bc2fa03c) SHA1(e63d8e75494a383bf9a213edfa9c472a010f8efe) )
ROM_END


ROM_START( crszonev2a )
	ROM_REGION32_BE( 0x800000, "user1", 0 ) /* 4 megs for main R4650 code */
	ROM_LOAD16_WORD_SWAP( "cszo2vera.ic4", 0x400000, 0x400000, CRC(1426d8d0) SHA1(e8049df1b2db1180f9edf6e5fa9fe8692ae81086) )
	ROM_CONTINUE( 0x000000, 0x400000 )

	ROM_REGION( 0x80000, "subcpu", 0 )  /* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "cszo3verb.ic1", 0x000000, 0x080000, CRC(c790743b) SHA1(5fa7b83a7a1b1105a3aa0870b782cf2741b7d11c) )

	ROM_REGION( 0x40000, "iocpu", 0 )   /* I/O board HD643334 H8/3334 MCU code. "MIU-I/O;Ver2.05;JPN,GUN-EXTENTION" */
	ROM_LOAD( "csz1prg0a.8f", 0x000000, 0x020000, CRC(8edc36b3) SHA1(b5df211988d856572fcc313480e693c8561784e4) )

	ROM_REGION32_BE( 0x2000000, "data", 0 ) /* data roms */
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

	ROM_REGION( 0x800000, "dups", 0 )   /* duplicate roms */
	ROM_LOAD( "csz1cguu.4f",  0x000000, 0x800000, CRC(e1d1bf24) SHA1(daf2c68e2d9a8f313d262d221cc990c93dfdf22f) )
	ROM_LOAD( "csz1cgum.5j",  0x000000, 0x800000, CRC(913c98b5) SHA1(b952dbc19053796077d4f33e8da836893e933b12) )
	ROM_LOAD( "csz1cgll.5m",  0x000000, 0x800000, CRC(0bcd41f2) SHA1(80b74f9398e8bd074f79a14490d06cfeb875c874) )
	ROM_LOAD( "csz1cglm.5k",  0x000000, 0x800000, CRC(d4af93d1) SHA1(0df37b793ce8da02d14f714722382786ae5d3ce2) )
	ROM_LOAD( "csz1ccrl.7m",  0x000000, 0x400000, CRC(1c20768d) SHA1(6cf4280e26f3625d6f750837bf344163e7e93c3d) )
	ROM_LOAD( "csz1ccrh.7k",  0x000000, 0x200000, CRC(bc2fa03c) SHA1(e63d8e75494a383bf9a213edfa9c472a010f8efe) )
ROM_END


/* Games */
#define GAME_FLAGS (MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND)
//    YEAR, NAME,        PARENT,   MACHINE,     INPUT,     INIT,                MNTR,  COMPANY, FULLNAME,                      FLAGS
GAME( 1997, rapidrvr,    0,        gorgon,      rapidrvr,  namcos23_state, s23, ROT0, "Namco", "Rapid River (RD3 Ver. C)",     GAME_FLAGS ) // 97/11/27, USA
GAME( 1997, rapidrvrv2c, rapidrvr, gorgon,      rapidrvr,  namcos23_state, s23, ROT0, "Namco", "Rapid River (RD2 Ver. C)",     GAME_FLAGS ) // 97/11/27, Europe
GAME( 1997, rapidrvrp,   rapidrvr, gorgon,      rapidrvrp, namcos23_state, s23, ROT0, "Namco", "Rapid River (prototype)",      GAME_FLAGS ) // 97/11/10, USA
GAME( 1997, finfurl,     0,        gorgon,      finfurl,   namcos23_state, s23, ROT0, "Namco", "Final Furlong (FF2 Ver. A)",   GAME_FLAGS )
GAME( 1997, downhill,    0,        s23,         s23,       namcos23_state, s23, ROT0, "Namco", "Downhill Bikers (DH3 Ver. A)", GAME_FLAGS )
GAME( 1997, motoxgo,     0,        s23,         s23,       namcos23_state, s23, ROT0, "Namco", "Motocross Go! (MG3 Ver. A)",   GAME_FLAGS )
GAME( 1997, motoxgov2a,  motoxgo,  s23,         s23,       namcos23_state, s23, ROT0, "Namco", "Motocross Go! (MG2 Ver. A)",   GAME_FLAGS )
GAME( 1997, motoxgov1a,  motoxgo,  s23,         s23,       namcos23_state, s23, ROT0, "Namco", "Motocross Go! (MG1 Ver. A, set 1)", GAME_FLAGS )
GAME( 1997, motoxgov1a2, motoxgo,  s23,         s23,       namcos23_state, s23, ROT0, "Namco", "Motocross Go! (MG1 Ver. A, set 2)", GAME_FLAGS )
GAME( 1997, timecrs2,    0,        timecrs2,    timecrs2,  namcos23_state, s23, ROT0, "Namco", "Time Crisis II (TSS3 Ver. B)", GAME_FLAGS )
GAME( 1997, timecrs2v2b, timecrs2, timecrs2,    timecrs2,  namcos23_state, s23, ROT0, "Namco", "Time Crisis II (TSS2 Ver. B)", GAME_FLAGS )
GAME( 1997, timecrs2v4a, timecrs2, timecrs2v4a, timecrs2,  namcos23_state, s23, ROT0, "Namco", "Time Crisis II (TSS4 Ver. A)", GAME_FLAGS )
GAME( 1998, panicprk,    0,        s23,         s23,       namcos23_state, s23, ROT0, "Namco", "Panic Park (PNP2 Ver. A)",     GAME_FLAGS )
GAME( 1998, panicprkj,   panicprk, s23,         s23,       namcos23_state, s23, ROT0, "Namco", "Panic Park (PNP1 Ver. B)",     GAME_FLAGS )
GAME( 1998, gunwars,     0,        gmen,        s23,       namcos23_state, s23, ROT0, "Namco", "Gunmen Wars (GM1 Ver. B)",     GAME_FLAGS )
GAME( 1998, gunwarsa,    gunwars,  gmen,        s23,       namcos23_state, s23, ROT0, "Namco", "Gunmen Wars (GM1 Ver. A)",     GAME_FLAGS )
GAME( 1998, raceon,      0,        gmen,        s23,       namcos23_state, s23, ROT0, "Namco", "Race On! (RO2 Ver. A)",        GAME_FLAGS )
GAME( 1998, 500gp,       0,        ss23,        s23,       namcos23_state, s23, ROT0, "Namco", "500 GP (5GP3 Ver. C)",         GAME_FLAGS )
GAME( 1998, aking,       0,        ss23,        s23,       namcos23_state, s23, ROT0, "Namco", "Angler King (AG1 Ver. A)",     GAME_FLAGS )
GAME( 1999, finfurl2,    0,        gmen,        s23,       namcos23_state, s23, ROT0, "Namco", "Final Furlong 2 (World)",      GAME_FLAGS )
GAME( 1999, finfurl2j,   finfurl2, gmen,        s23,       namcos23_state, s23, ROT0, "Namco", "Final Furlong 2 (Japan)",      GAME_FLAGS )
GAME( 2000, crszone,     0,        ss23e2,      s23,       namcos23_state, s23, ROT0, "Namco", "Crisis Zone (CSZO4 Ver. B)",   GAME_FLAGS )
GAME( 2000, crszonev4a,  crszone,  ss23e2,      s23,       namcos23_state, s23, ROT0, "Namco", "Crisis Zone (CSZO4 Ver. A)",   GAME_FLAGS )
GAME( 2000, crszonev3b,  crszone,  ss23e2,      s23,       namcos23_state, s23, ROT0, "Namco", "Crisis Zone (CSZO3 Ver. B, set 1)", GAME_FLAGS )
GAME( 2000, crszonev3b2, crszone,  ss23e2,      s23,       namcos23_state, s23, ROT0, "Namco", "Crisis Zone (CSZO3 Ver. B, set 2)", GAME_FLAGS )
GAME( 2000, crszonev3a,  crszone,  ss23e2,      s23,       namcos23_state, s23, ROT0, "Namco", "Crisis Zone (CSZO3 Ver. A)",   GAME_FLAGS )
GAME( 2000, crszonev2a,  crszone,  ss23e2,      s23,       namcos23_state, s23, ROT0, "Namco", "Crisis Zone (CSZO2 Ver. A)",   GAME_FLAGS )
