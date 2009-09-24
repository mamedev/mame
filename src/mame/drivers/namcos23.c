/*
    Namco System 22.5 and (Super) System 23
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
    - Text layer is (almost?) identical to System 22 & Super System 22.

    TODO:
    - Palette is not right.

    - H8/3002 now communicates with the H8/3334 on the I/O board, but it's not reliable yet.

    - Hook up actual inputs (?) via the 2 serial latches at d00004 and d00006.
      Works like this: write to d00004, then read d00004 12 times.  Ditto at
      d00006.  This gives 24 bits of inputs (?) from the I/O board (?).

    - The entire 3D subsystem.  Is there a DSP living down there?  If not, why the 300k
      download on initial startup?

    - There are currently no differences seen between System 23 (Time Crisis 2) and
      Super System 23 (GP500, Final Furlong 2).  These will presumably appear when
      the 3D hardware is emulated.

    - Serial number data is at offset 0x201 in the BIOS.  Until the games are running
      and displaying it I'm not going to meddle with it though.

*/

/*

Namco System 23 and Super System 23 Hardware Overview (last updated 27th February, 2006 at 5.26pm)
Namco, 1997 - 2000

Note! This document is a Work-In-Progress and will be updated from time to time when more dumps are available.

This document covers all the known Namco System 23 / Super System 23 games, including....
*Angler King      Namco, 1999    System 23
*Final Furlong    Namco, 1997    System 22.5/Gorgon
Gunmen Wars       Namco, 1998    System 23 [not dumped, but have]
Motocross Go!     Namco, 1997    System 23
Panic Park        Namco, 1998    System 23
Rapid River       Namco, 1997    System 22.5/Gorgon
Time Crisis II    Namco, 1997    System 23
*Underground King Namco, 1998    System 23
Downhill Bikers   Namco, 199?    System 23 [not dumped, but have]
500 GP            Namco, 1999    Super System 23
Crisis Zone       Namco, 2000    Super System 23 Evolution 2 [not dumped, but have]
Final Furlong 2   Namco, 1999    Super System 23
*Guitar Jam       Namco, 1999    Super System 23
*Race On!         Namco, 1998    Super System 23

* - denotes not dumped yet (and hardware type not confirmed). If you can help with the remaining undumped S22.5/S23/SS23 games,
    please contact me at http://www.mameworld.net/gurudumps/

The system comprises the following main boards....
- EMI PCB                          Small PCB with some connectors, including power in, video out, network in/out, sound out (to AMP PCB)
- BASS AMP PCB                     Power AMP PCB for general sounds and bass
- SYSTEM23 MAIN PCB                Main PCB for System 23        \
  or SystemSuper23 MAIN(1) PCB     Main PCB for Super System 23  / Note the 2 main boards are similar, but not exactly the same.
- MSPM(FRA) PCB                    A small plug-in daughterboard that holds FLASHROMs containing Main CPU and Sub CPU programs
- FCA PCB                          Controls & I/O interface board. Contains mostly transistors, caps, resistors, several connectors and an MCU.
                                   The MCU is different for EACH game and the FCA PCBs are not interchangeable between different games.
                                   If the FCA PCB is not connected, the game will not advance past the 3rd screen shown below.
- SYSTEM23 MEM(M) PCB              Holds MASKROMs for GFX/Sound and associated logic
                                   Note that in Super System23, the MEM(M) PCB is re-used from System23.
                                   On Super System23, there is a sticker over the System23 part labelled 'SystemSuper23'

The metal box housing these PCB's is approximately the same size as Super System 22. However, the box is mostly
empty. All of the CPU/Video/DSP hardware is located on the main PCB which is the same size as the
Super System 22 CPU board. The ROM PCB is half the size of the Super System22 ROM PCB. The ROM positions on it
can be configured for either 32MBit or 64MBit SOP44 MASK ROMs with a maximum capacity of 1664MBits.
The system also uses a dual pipeline graphics bus similar to Super System 22 and has two copies of the graphics ROMs
on the PCB.
The System 23 hardware is the first NAMCO system to require an external 3.3V power source. Previously the 3.3volts
was derived from a 5v to 3.3v regulator on systems such as System10/11/12 etc.
The KEYCUS chip is the familiar MACH211 PLCC44 IC as used on System12. The sound system is also taken from System12.

On bootup, the following happens (on GP500)...

1st screen - Black screen with white text
                               "SYSTEM 23 BOOTING     "
                               "SDRAM CHECKING A0xx000" (xx = slowly counts up to 3F, from 00), then OK ('CHECKING' is in yellow text, 'OK' is in green text)
   As the SDRAM is being checked, the LEDS 1 to 8 turn off in sequence from 8 to 1.

2nd screen - Black screen with white text
                               "S.S.23 POWER ON TEST      xxxx"  (xxxx = numbers count up rapidly from 0000)
                               "(C) NAMCO                     "
                               "                     VER. 1.16"
   As these checks happen, the LEDs 1 to 8 flash on/off

3rd screen - Black screen with white text
                               "S.S.23 POWER ON TEST      xxxx"  (xxxx = numbers count up rapidly from 0000)
                               "SUBCPU INITIALIZING ....      "
                               "SUBCPU PROGRAM Ver. 0211      "

   and a PACMAN eating dots along the bottom of the screen from left to right.
   As these checks happen, the LEDs 1 to 8 simultaneously flash on/off in various patterns.

   - On System23, the bootup sequence is shorter. The screen remains blank while the SDRAM is being checked. LEDS 1-8 turn off in sequence 8-1
     After that, the bootup sequence is mostly the same as SS23.


PCB Layouts
-----------

SYSTEM23 MAIN PCB 8660962302 8660971105 (8660961105)
|----------------------------------------------------------------------------|
|       J5                    3V_BATT                                        |
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
|       |NR4650-138|    |   C361  |           CY2291       |    C412     |   |
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
          KM416S1020 - Samsung 1M x16 SDRAM (x2, TSOP48)
          M5M4V4265  - Mitsubishi 256k x16 DRAM (x2, TSOP40/44)
          LC321664   - Sanyo 64k x16 EDO DRAM (SOJ40)
          HM5216165  - Hitachi 1M x16 SDRAM (x2, TSOP48)
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
               XC95108  - Xilinx XC95108 In-System Programmable CPLD (QFP100, labelled 'S23MA9B')
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
          J18   - Connector for MSPM(FRA) PCB
          J5    - Connector for EMI PCB
          J12   - 6-pin connector for In-System Programming of the XC95108 IC
          J14 \
          J15 \
          J16 \
          J17 \ - Connectors for MEM(M) PCB
          SW2   - 2 position DIP Switch
          SW3   - 2 position DIP Switch
          SW4   - 8 position DIP Switch


SystemSuper23 MAIN(1) PCB 8672960904 8672960104 (8672970104)
|----------------------------------------------------------------------------|
|       J5                    3V_BATT                                        |
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
          N341256    - NKK 32K x8 SRAM (x5, SOJ28)
          LC35256    - Sanyo 32K x8 SRAM (SOP28)
          KM416S1020 - Samsung 16M SDRAM (x4, TSOP48)
          KM416V2540 - Samsung 256K x16 EDO DRAM (x2, TSOP40/44)
          LC321664   - Sanyo 64K x16 EDO DRAM (SOJ40)
          71V124     - IDT 128K x8 SRAM (x2, SOJ32)
          CY7C1399   - Cypress 32K x8 SRAM (x4, SOJ28)
          CY7C182    - Cypress 8K x9 SRAM (SOJ28)
          M5M4V4265  - Mitsubishi 256K x16 DRAM (x2, TSOP40/44)
          HM5216165  - Hitachi 1M x16 SDRAM (x2, TSOP48)
          KM681000   - Samsung 128K x8 SRAM (x2, SOP32)

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
                    C444 (QFP136)
                    C447 (QFP256)

      Other ICs
      ---------
               XC95108  - Xilinx XC95108 In-System Programmable CPLD (QFP100, labelled 'S23MA9B')
               EPM7064  - Altera MAX EPM7064STC100-10 CPLD (TQFP100, labelled 'SS23MA4A')
               DS8921   - National RS422/423 Differential Line Driver and Receiver Pair (SOIC8)
               CXD1178Q - SONY CXD1178Q  8-bit RGB 3-channel D/A converter (QFP48)
               PAL(1)   - PALCE16V8H (PLCC20, stamped 'PAD23')
               PAL(2)   - PALCE22V10H (PLCC28, stamped 'S23MA5')
               PAL(3)   - PALCE22V10H (PLCC28, stamped 'SS23MA6B')
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
               IDT7200  - Integrated Devices Technology IDT7200 256 x9 CMOS Asynchronous FIFO

      Misc
      ----
          J18   - Connector for MSPM(FRA) PCB
          J5    - Connector for EMI PCB
          J14 \
          J15 \
          J16 \
          J17 \ - Connectors for MEM(M) PCB
          SW2   - 2 position DIP Switch
          SW3   - 2 position DIP Switch
          SW4   - 8 position DIP Switch


Program ROM PCB
---------------

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

      Game           Code and revision
      --------------------------------
      Time Crisis 2  TSS3 Ver.B


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

      Game           Code and revision
      --------------------------------
      GP500          5GP3 Ver.C
Other games dumps with unknown PCB information....

      Game             Code and revision
      ----------------------------------
      Final Furlong 2  FFS1 Ver.?
      Final Furlong 2  FFS2 Ver.?


ROM PCB
-------

Printed on the PCB        - 8660960601 (8660970601) SYSTEM23 MEM(M) PCB
Sticker (GP500)           - 8672961100
Sticker (Time Crisis 2)   - 8660962302
Sticker (Final Furlong 2) - ??????????
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
      J2   \
      J3   \
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
      Note: ROMs at locations 7M, 7K, 5M, 5K, 5J & 5F are not included in the archive since they're copies of
            other ROMs which are included in the archive. However, they are populated on the PCB.
            Each ROM is stamped with the Namco Game Code, then the ROM-use code (such as CCRL, CCRH, PT* or MT*).

                            Game
            Game            Code     Keycus    Notes
            ---------------------------------------------------------
            GP500           5GP1     KC029     -
            Time Crisis 2   TSS1     KC010     3A and 3C not populated. i.e. PT3L & PT3H is not used.
            Final Furlong 2 FFS1     KC???     -


I/O PCBs
--------

FCA PCB  8662969102 (8662979102)
(Used with GP500)
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
      PIC16F84 - Microchip PIC16F84 Programmable Interrupt Controller stamped 'CAP10' (SOIC20)
      MCU      - Fujitsu MB90F574 Microcontroller, stamped 'FCAF10' (QFP120)
      ADM485   - Analog Devices ADM485 +5V Low Power EIA RS-485 Transceiver (SOIC8)
*/

/*
Rapid River
Namco, 1997

This game runs on hardware called "GORGON". It appears to be similar to
System 23 but the PCBs are slightly larger.

The system comprises Main PCB, ROM PCB and I/O PCB all located inside
a metal box with 3 separate power supplies for 5V, 12V and 3.3V. Main
input power is 115V.
The game is controlled by rotating a paddle (for thrust) and turning it
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
|                |H8/  |               |C361  |                   D4516161    |         | |LQF-133  |  |
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
      Rapid River    RD3 Ver.C


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
      KEYCUS - MACH211 CPLD stamped 'KC012' (PLCC44)
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
      EPM7064 - Altera EPM7064LC68-15 FPGA, labelled 'ASCA DR0' (PLCC68)
      PST592 - System Reset IC (SOIC4)
      ADM485 - Analog Devices +ADM485 5V Low Power EIA RS-485 Transceiver (SOIC8)
      MB87078 - Fujitsu MB87078 Electronic Volume Control IC (SOIC24)
      LC78815 - Sanyo LM78815 2-Channel 16-Bit D/A Converter (SOIC20)
      J101 - 34 pin flat cable connector for filter board
      J102 - 50 pin flat cable connector for filter board
      J104 - 8 pin power connector (+5V, +12V, +3.3V)
      J105 - 64 pin connector for connection of Main PCB
*/

#include "driver.h"
#include "cpu/mips/mips3.h"
#include "cpu/h83002/h8.h"
#include "sound/c352.h"

static int ss23_vstat = 0, hstat = 0, vstate = 0;
static tilemap *bgtilemap;
static UINT32 *namcos23_textram, *namcos23_shared_ram;
static UINT32 *namcos23_charram;
static UINT8 namcos23_jvssense;

static UINT16 nthword( const UINT32 *pSource, int offs )
{
	pSource += offs/2;
	return (pSource[0]<<((offs&1)*16))>>16;
}

static TILE_GET_INFO( TextTilemapGetInfo )
{
	UINT16 data = nthword( namcos23_textram,tile_index );
  /**
    * x---.----.----.---- blend
    * xxxx.----.----.---- palette select
    * ----.xx--.----.---- flip
    * ----.--xx.xxxx.xxxx code
    */
	SET_TILE_INFO( 0, data&0x03ff, data>>12, TILE_FLIPYX((data&0x0c00)>>10) );
} /* TextTilemapGetInfo */

static READ32_HANDLER( namcos23_textram_r )
{
	return namcos23_textram[offset];
}

static WRITE32_HANDLER( namcos23_textram_w )
{
	COMBINE_DATA( &namcos23_textram[offset] );
}

static VIDEO_START( ss23 )
{
	gfx_element_set_source(machine->gfx[0], (UINT8 *)namcos23_charram);
	bgtilemap = tilemap_create(machine, TextTilemapGetInfo, tilemap_scan_rows, 16, 16, 64, 64);
	tilemap_set_transparent_pen(bgtilemap, 0xf);
}

#if 0
static double
Normalize( UINT32 data )
{
	data &=  0xffffff;
	if( data&0x800000 )
	{
		data |= 0xff000000;
	}
	return (INT32)data;
}

static void
DrawLine( bitmap_t *bitmap, int x0, int y0, int x1, int y1 )
{
	if( x0>=0 && x0<bitmap->width &&
		x1>=0 && x1<bitmap->width &&
		y0>=0 && y0<bitmap->height &&
		y1>=0 && y1<bitmap->height )
	{
		int sx,sy,dy;
		if( x0>x1 )
		{
			int temp = x0;
			x0 = x1;
			x1 = temp;

			temp = y0;
			y0 = y1;
			y1 = temp;
		}

		if( x1>x0 )
		{
			sy = y0<<16;
			dy = ((y1-y0)<<16)/(x1-x0);
			for( sx=x0; sx<x1; sx++ )
			{
				UINT16 *pDest = BITMAP_ADDR16(bitmap, sy>>16, 0);
				pDest[sx] = 1;
				sy += dy;
			}
		}
	}
} /* DrawLine */

static void
DrawPoly( bitmap_t *bitmap, const UINT32 *pSource, int n, int bNew )
{
	UINT32 flags = *pSource++;
	UINT32 unk = *pSource++;
	UINT32 intensity = *pSource++;
	double x[4],y[4],z[4];
	int i;
	if( bNew )
	{
		mame_printf_debug( "polydata: 0x%08x 0x%08x 0x%08x\n", flags, unk, intensity );
	}
	for( i=0; i<n; i++ )
	{
		x[i] = Normalize(*pSource++);
		y[i] = Normalize(*pSource++);
		z[i] = Normalize(*pSource++);

		if( bNew )
		{
			mame_printf_debug( "\t(%f,%f,%f)\n", x[i], y[i], z[i] );
		}
	}
	for( i=0; i<n; i++ )
	{
		#define KDIST 0x400
		int j = (i+1)%n;
		double z0 = z[i]+KDIST;
		double z1 = z[j]+KDIST;

		if( z0>0 && z1>0 )
		{
			int x0 = bitmap->width*x[i]/z0 + bitmap->width/2;
			int y0 = bitmap->width*y[i]/z0 + bitmap->height/2;
			int x1 = bitmap->width*x[j]/z1 + bitmap->width/2;
			int y1 = bitmap->width*y[j]/z1 + bitmap->height/2;
			if( bNew )
			{
				mame_printf_debug( "[%d,%d]..[%d,%d]\n", x0,y0,x1,y1 );
			}
			DrawLine( bitmap, x0,y0,x1,y1 );
		}
	}
}
#endif

static VIDEO_UPDATE( ss23 )
{
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	tilemap_mark_all_tiles_dirty(bgtilemap);
	tilemap_draw( bitmap, cliprect, bgtilemap, 0/*flags*/, 0/*priority*/ ); /* opaque */

#if 0
	static int bNew = 1;
	static int code = 0x80;
	const UINT32 *pSource = (UINT32 *)memory_region(machine, "pointrom");

	pSource = pSource + pSource[code];

	bitmap_fill( bitmap, 0 , 0);
	for(;;)
	{
		UINT32 opcode = *pSource++;
		int bDone = (opcode&0x10000);

		switch( opcode&0x0f00 )
		{
		case 0x0300:
			DrawPoly( bitmap, pSource, 3, bNew );
			pSource += 3 + 3*3;
			break;

		case 0x0400:
			DrawPoly( bitmap, pSource, 4, bNew );
			pSource += 3 + 4*3;
			break;

		default:
			mame_printf_debug( "unk opcode: 0x%x\n", opcode );
			bDone = 1;
			break;
		}


		if( bDone )
		{
			break;
		}
	}

	bNew = 0;

	if( keyboard_pressed(KEYCODE_SPACE) )
	{
		while( keyboard_pressed(KEYCODE_SPACE) ){}
		code++;
		bNew = 1;
	}
#endif
	return 0;
}

static VIDEO_UPDATE( gorgon )
{
	return 0;
}

static WRITE32_HANDLER( s23_txtchar_w )
{
	COMBINE_DATA(&namcos23_charram[offset]);
	gfx_element_mark_dirty(space->machine->gfx[0], offset/32);
}

static READ32_HANDLER( ss23_vstat_r )
{
	if (offset == 1)
	{
		hstat ^= 0xffffffff;
		return hstat;
	}

	vstate++;
	if (vstate >= 2)
	{
	ss23_vstat ^= 0xffffffff;
		vstate = 0;
	}
	return ss23_vstat;
}

static UINT8 nthbyte( const UINT32 *pSource, int offs )
{
	pSource += offs/4;
	return (pSource[0]<<((offs&3)*8))>>24;
}

INLINE void UpdatePalette( running_machine *machine, int entry )
{
         int j;

	for( j=0; j<1; j++ )
	{
		int which = (entry*2)+(j*2);
		int r = nthbyte(paletteram32,which+0x00001);
		int g = nthbyte(paletteram32,which+0x08001);
		int b = nthbyte(paletteram32,which+0x18001);
		palette_set_color( machine,which,MAKE_RGB(r,g,b) );
	}
}

static READ32_HANDLER( namcos23_paletteram_r )
{
	return paletteram32[offset];
}

/* each LONGWORD is 2 colors.  each OFFSET is 2 colors */

static WRITE32_HANDLER( namcos23_paletteram_w )
{
	COMBINE_DATA( &paletteram32[offset] );

	UpdatePalette(space->machine, (offset % (0x8000/4))*2);
}

// must return this magic number
static UINT32 s23_vbl = 0;

static INTERRUPT_GEN(s23_interrupt)
{
	s23_vbl ^= 0x80008000;
}

static READ32_HANDLER(s23_unk_r)
{
	return 0x008e008e | s23_vbl;
}

// this & 8 and this & 4 are checked
// offset = 1 for magic latch
static READ32_HANDLER(sysctl_stat_r)
{
	if (offset == 1) return 0x0000ffff;	// all inputs in

	return 0xffffffff;
}

// as with System 22, we need to halt the MCU while checking shared RAM
static WRITE32_HANDLER( s23_mcuen_w )
{
	mame_printf_debug("mcuen_w: mask %08x, data %08x\n", mem_mask, data);
	if (mem_mask == 0x0000ffff)
	{
		if (data)
		{
			logerror("S23: booting H8/3002\n");
			cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_RESET, CLEAR_LINE);
		}
		else
		{
			logerror("S23: stopping H8/3002\n");
			cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_RESET, ASSERT_LINE);
		}
	}
}

static UINT32 gorgon_vbl = 0;
static READ32_HANDLER( gorgon_vbl_r )
{
	gorgon_vbl ^= 0xffffffff;

	return gorgon_vbl;
}

static ADDRESS_MAP_START( gorgon_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM
	AM_RANGE(0x02000000, 0x02000003) AM_READ( gorgon_vbl_r )
	AM_RANGE(0x04400000, 0x0440ffff) AM_RAM AM_BASE(&namcos23_shared_ram)
	AM_RANGE(0x04c3ff0c, 0x04c3ff0f) AM_RAM				// 3d FIFO
	AM_RANGE(0x0d000000, 0x0d000007) AM_READ(sysctl_stat_r)
	AM_RANGE(0x0fc00000, 0x0fffffff) AM_WRITENOP AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0x1fc00000, 0x1fffffff) AM_WRITENOP AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ss23_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM
	AM_RANGE(0x02000000, 0x02000003) AM_READ( s23_unk_r )
	AM_RANGE(0x04400000, 0x0440ffff) AM_RAM AM_BASE(&namcos23_shared_ram)
	AM_RANGE(0x04c3ff08, 0x04c3ff0b) AM_WRITE( s23_mcuen_w )
	AM_RANGE(0x04c3ff0c, 0x04c3ff0f) AM_RAM				// 3d FIFO
	AM_RANGE(0x06800000, 0x06800fff) AM_RAM 			// text layer palette
	AM_RANGE(0x06800000, 0x06803fff) AM_WRITE( s23_txtchar_w ) AM_BASE(&namcos23_charram)	// text layer characters
	AM_RANGE(0x06804000, 0x0681dfff) AM_RAM
	AM_RANGE(0x0681e000, 0x0681ffff) AM_READ(namcos23_textram_r) AM_WRITE(namcos23_textram_w) AM_BASE(&namcos23_textram)
	AM_RANGE(0x06a08000, 0x06a2ffff) AM_READ(namcos23_paletteram_r) AM_WRITE(namcos23_paletteram_w) AM_BASE(&paletteram32)
	AM_RANGE(0x06a30000, 0x06a3ffff) AM_RAM
	AM_RANGE(0x06820008, 0x0682000f) AM_READ( ss23_vstat_r )	// vblank status?
	AM_RANGE(0x08000000, 0x08017fff) AM_RAM
	AM_RANGE(0x0d000000, 0x0d000007) AM_READ(sysctl_stat_r) AM_WRITENOP
	AM_RANGE(0x0fc00000, 0x0fffffff) AM_WRITENOP AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0x1fc00000, 0x1fffffff) AM_WRITENOP AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END

static WRITE16_HANDLER( sharedram_sub_w )
{
	UINT16 *shared16 = (UINT16 *)namcos23_shared_ram;

	// fake that an I/O board is connected
	if ((offset == 0x4052/2) && (data == 0x78))
	{
		data = 0;
	}

	COMBINE_DATA(&shared16[BYTE_XOR_BE(offset)]);
}

static READ16_HANDLER( sharedram_sub_r )
{
	UINT16 *shared16 = (UINT16 *)namcos23_shared_ram;

	return shared16[BYTE_XOR_BE(offset)];
}

/* H8/3002 MCU stuff */
static ADDRESS_MAP_START( s23h8rwmap, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_READ(SMH_ROM)
	AM_RANGE(0x080000, 0x08ffff) AM_READWRITE( sharedram_sub_r, sharedram_sub_w )
	AM_RANGE(0x280000, 0x287fff) AM_DEVREADWRITE( "c352", c352_r, c352_w )
	AM_RANGE(0x300000, 0x300001) AM_READNOP //AM_READ_PORT("IN1")
	AM_RANGE(0x300002, 0x300003) AM_READNOP //AM_READ_PORT("IN2")
	AM_RANGE(0x300010, 0x300011) AM_NOP
	AM_RANGE(0x300030, 0x300031) AM_WRITENOP	// timecrs2 writes this when writing to the sync shared ram location, motoxgo doesn't
ADDRESS_MAP_END

static READ8_HANDLER( s23_mcu_p8_r )
{
	return 0x02;
}

// emulation of the Epson R4543 real time clock
// in System 12, bit 0 of H8/3002 port A is connected to it's chip enable
// the actual I/O takes place through the H8/3002's serial port B.

static int s23_porta = 0, s23_rtcstate = 0;

static READ8_HANDLER( s23_mcu_pa_r )
{
	return s23_porta;
}

static WRITE8_HANDLER( s23_mcu_pa_w )
{
	// bit 0 = chip enable for the RTC
	// reset the state on the rising edge of the bit
	if ((!(s23_porta & 1)) && (data & 1))
	{
		s23_rtcstate = 0;
	}

	s23_porta = data;
}

INLINE UINT8 make_bcd(UINT8 data)
{
	return ((data / 10) << 4) | (data % 10);
}

static READ8_HANDLER( s23_mcu_rtc_r )
{
	UINT8 ret = 0;
	mame_system_time systime;
	static const int weekday[7] = { 7, 1, 2, 3, 4, 5, 6 };

	mame_get_current_datetime(space->machine, &systime);

	switch (s23_rtcstate)
	{
		case 0:
			ret = make_bcd(systime.local_time.second);	// seconds (BCD, 0-59) in bits 0-6, bit 7 = battery low
			break;
		case 1:
			ret = make_bcd(systime.local_time.minute);	// minutes (BCD, 0-59)
			break;
		case 2:
			ret = make_bcd(systime.local_time.hour);	// hour (BCD, 0-23)
			break;
		case 3:
			ret = make_bcd(weekday[systime.local_time.weekday]);	// low nibble = day of the week
			ret |= (make_bcd(systime.local_time.mday) & 0x0f)<<4;	// high nibble = low digit of day
			break;
		case 4:
			ret = (make_bcd(systime.local_time.mday) >> 4);			// low nibble = high digit of day
			ret |= (make_bcd(systime.local_time.month + 1) & 0x0f)<<4;	// high nibble = low digit of month
			break;
		case 5:
			ret = make_bcd(systime.local_time.month + 1) >> 4;	// low nibble = high digit of month
			ret |= (make_bcd(systime.local_time.year % 10) << 4);	// high nibble = low digit of year
			break;
		case 6:
			ret = make_bcd(systime.local_time.year % 100) >> 4;	// low nibble = tens digit of year (BCD, 0-9)
			break;
	}

	s23_rtcstate++;

	return ret;
}

static int s23_lastpB = 0x50, s23_setstate = 0, s23_setnum, s23_settings[8];

static READ8_HANDLER( s23_mcu_portB_r )
{
	s23_lastpB ^= 0x80;
	return s23_lastpB;
}

static WRITE8_HANDLER( s23_mcu_portB_w )
{
	// bit 7 = chip enable for the video settings controller
	if (data & 0x80)
	{
		s23_setstate = 0;
	}

	s23_lastpB = data;
}

static WRITE8_HANDLER( s23_mcu_settings_w )
{
	if (s23_setstate)
	{
		// data
		s23_settings[s23_setnum] = data;

		if (s23_setnum == 7)
		{
			logerror("S23 video settings: Contrast: %02x  R: %02x  G: %02x  B: %02x\n",
				BITSWAP8(s23_settings[0], 0, 1, 2, 3, 4, 5, 6, 7),
				BITSWAP8(s23_settings[1], 0, 1, 2, 3, 4, 5, 6, 7),
				BITSWAP8(s23_settings[2], 0, 1, 2, 3, 4, 5, 6, 7),
				BITSWAP8(s23_settings[3], 0, 1, 2, 3, 4, 5, 6, 7));
		}
	}
	else
	{	// setting number
		s23_setnum = (data >> 4)-1;
	}

	s23_setstate ^= 1;
}

static UINT8 maintoio[64], mi_rd, mi_wr;
static UINT8 iotomain[64], im_rd, im_wr;

static READ8_HANDLER( s23_mcu_iob_r )
{
	UINT8 ret = iotomain[im_rd];

	im_rd++;
	im_rd &= 0x3f;

	if (im_rd == im_wr)
	{
		cputag_set_input_line(space->machine, "audiocpu", H8_SCI_0_RX, CLEAR_LINE);
	}
	else
	{
		cputag_set_input_line(space->machine, "audiocpu", H8_SCI_0_RX, CLEAR_LINE);
		cputag_set_input_line(space->machine, "audiocpu", H8_SCI_0_RX, ASSERT_LINE);
	}

	return ret;
}

static WRITE8_HANDLER( s23_mcu_iob_w )
{
	maintoio[mi_wr++] = data;
	mi_wr &= 0x3f;

	cputag_set_input_line(space->machine, "ioboard", H8_SCI_0_RX, ASSERT_LINE);
}

static INPUT_PORTS_START( ss23 )
	PORT_START("H8PORT")
INPUT_PORTS_END


static READ8_HANDLER(s23_mcu_p6_r)
{
	// bit 1 = JVS cable present sense (1 = I/O board plugged in)
		return (namcos23_jvssense << 1) | 0xfd;
}

static WRITE8_HANDLER(s23_mcu_p6_w)
{
//  printf("%02x to port 6\n", data);
}

static ADDRESS_MAP_START( s23h8iomap, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(H8_PORT_6, H8_PORT_6) AM_READWRITE( s23_mcu_p6_r, s23_mcu_p6_w )
	AM_RANGE(H8_PORT_7, H8_PORT_7) AM_READ_PORT( "H8PORT" )
	AM_RANGE(H8_PORT_8, H8_PORT_8) AM_READ( s23_mcu_p8_r ) AM_WRITENOP
	AM_RANGE(H8_PORT_A, H8_PORT_A) AM_READWRITE( s23_mcu_pa_r, s23_mcu_pa_w )
	AM_RANGE(H8_PORT_B, H8_PORT_B) AM_READWRITE( s23_mcu_portB_r, s23_mcu_portB_w )
	AM_RANGE(H8_SERIAL_0, H8_SERIAL_0) AM_READWRITE( s23_mcu_iob_r, s23_mcu_iob_w )
	AM_RANGE(H8_SERIAL_1, H8_SERIAL_1) AM_READWRITE( s23_mcu_rtc_r, s23_mcu_settings_w )
	AM_RANGE(H8_ADC_0_H, H8_ADC_0_L) AM_NOP
	AM_RANGE(H8_ADC_1_H, H8_ADC_1_L) AM_NOP
	AM_RANGE(H8_ADC_2_H, H8_ADC_2_L) AM_NOP
	AM_RANGE(H8_ADC_3_H, H8_ADC_3_L) AM_NOP
ADDRESS_MAP_END

// version without serial hookup to I/O board for games where the PIC isn't dumped
static ADDRESS_MAP_START( s23h8ionoiobmap, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(H8_PORT_7, H8_PORT_7) AM_READ_PORT( "H8PORT" )
	AM_RANGE(H8_PORT_8, H8_PORT_8) AM_READ( s23_mcu_p8_r ) AM_WRITENOP
	AM_RANGE(H8_PORT_A, H8_PORT_A) AM_READWRITE( s23_mcu_pa_r, s23_mcu_pa_w )
	AM_RANGE(H8_PORT_B, H8_PORT_B) AM_READWRITE( s23_mcu_portB_r, s23_mcu_portB_w )
	AM_RANGE(H8_SERIAL_1, H8_SERIAL_1) AM_READWRITE( s23_mcu_rtc_r, s23_mcu_settings_w )
	AM_RANGE(H8_ADC_0_H, H8_ADC_0_L) AM_NOP
	AM_RANGE(H8_ADC_1_H, H8_ADC_1_L) AM_NOP
	AM_RANGE(H8_ADC_2_H, H8_ADC_2_L) AM_NOP
	AM_RANGE(H8_ADC_3_H, H8_ADC_3_L) AM_NOP
ADDRESS_MAP_END


static READ8_HANDLER( s23_iob_mcu_r )
{
	UINT8 ret = maintoio[mi_rd];

	mi_rd++;
	mi_rd &= 0x3f;

	if (mi_rd == mi_wr)
	{
		cputag_set_input_line(space->machine, "ioboard", H8_SCI_0_RX, CLEAR_LINE);
	}

	return ret;
}

static WRITE8_HANDLER( s23_iob_mcu_w )
{
	iotomain[im_wr++] = data;
	im_wr &= 0x3f;

	cputag_set_input_line(space->machine, "audiocpu", H8_SCI_0_RX, ASSERT_LINE);
}

static UINT8 s23_tssio_port_4 = 0;

static READ8_HANDLER( s23_iob_p4_r )
{
	return s23_tssio_port_4;
}

static WRITE8_HANDLER( s23_iob_p4_w )
{
	s23_tssio_port_4 = data;

	namcos23_jvssense = (data & 0x04) ? 0 : 1;
 }

/* H8/3334 (Namco C78) I/O board MCU */
static ADDRESS_MAP_START( s23iobrdmap, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_READ(SMH_ROM)
	AM_RANGE(0xc000, 0xdfff) AM_RAM
ADDRESS_MAP_END

/*
    port 5 bit 2 = LED to indicate transmitting packet to main
    port 4 bit 2 = SENSE line back to main (0 = asserted, 1 = dropped)
*/
static ADDRESS_MAP_START( s23iobrdiomap, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(H8_PORT_4, H8_PORT_4) AM_READWRITE( s23_iob_p4_r, s23_iob_p4_w )
	AM_RANGE(H8_SERIAL_0, H8_SERIAL_0) AM_READWRITE( s23_iob_mcu_r, s23_iob_mcu_w )
ADDRESS_MAP_END

static DRIVER_INIT(ss23)
{
	mi_rd = mi_wr = im_rd = im_wr = 0;
	namcos23_jvssense = 1;
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

#if 0
static const gfx_layout sprite_layout =
{
	32,32,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{
		0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,
		8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8,
		16*8,17*8,18*8,19*8,20*8,21*8,22*8,23*8,
		24*8,25*8,26*8,27*8,28*8,29*8,30*8,31*8 },
	{
		0*32*8,1*32*8,2*32*8,3*32*8,4*32*8,5*32*8,6*32*8,7*32*8,
		8*32*8,9*32*8,10*32*8,11*32*8,12*32*8,13*32*8,14*32*8,15*32*8,
		16*32*8,17*32*8,18*32*8,19*32*8,20*32*8,21*32*8,22*32*8,23*32*8,
		24*32*8,25*32*8,26*32*8,27*32*8,28*32*8,29*32*8,30*32*8,31*32*8 },
	32*32*8
};
#endif

static GFXDECODE_START( namcos23 )
	GFXDECODE_ENTRY( NULL, 0, namcos23_cg_layout,  0, 0x80 )
GFXDECODE_END

static const mips3_config config =
{
	8192,				/* code cache size - VERIFIED */
	8192				/* data cache size - VERIFIED */
};

static INTERRUPT_GEN( namcos23_interrupt )
{
}

static MACHINE_DRIVER_START( gorgon )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", R4650BE, 133000000)
	MDRV_CPU_CONFIG(config)
	MDRV_CPU_PROGRAM_MAP(gorgon_map)

	MDRV_CPU_ADD("audiocpu", H83002, 14745600 )
	MDRV_CPU_PROGRAM_MAP( s23h8rwmap)
	MDRV_CPU_IO_MAP( s23h8ionoiobmap)
	MDRV_CPU_VBLANK_INT("screen", irq1_line_pulse)

	MDRV_QUANTUM_TIME(HZ(60000))

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*16, 30*16)
	MDRV_SCREEN_VISIBLE_AREA(0, 64*16-1, 0, 30*16-1)

	MDRV_PALETTE_LENGTH(0x8000)

	MDRV_GFXDECODE(namcos23)

	MDRV_VIDEO_START(ss23)
	MDRV_VIDEO_UPDATE(gorgon)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("c352", C352, 14745600)
	MDRV_SOUND_ROUTE(0, "rspeaker", 1.00)
	MDRV_SOUND_ROUTE(1, "lspeaker", 1.00)
	MDRV_SOUND_ROUTE(2, "rspeaker", 1.00)
	MDRV_SOUND_ROUTE(3, "lspeaker", 1.00)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( s23 )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", R4650BE, 166000000)
	MDRV_CPU_CONFIG(config)
	MDRV_CPU_PROGRAM_MAP(ss23_map)
	MDRV_CPU_VBLANK_INT("screen", s23_interrupt)

	MDRV_CPU_ADD("audiocpu", H83002, 14745600 )
	MDRV_CPU_PROGRAM_MAP( s23h8rwmap)
	MDRV_CPU_IO_MAP( s23h8iomap)
	MDRV_CPU_VBLANK_INT("screen", irq1_line_pulse)

	MDRV_CPU_ADD("ioboard", H83334, 14745600 )
	MDRV_CPU_PROGRAM_MAP( s23iobrdmap)
	MDRV_CPU_IO_MAP( s23iobrdiomap)

	MDRV_QUANTUM_TIME(HZ(60*4000))

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*16, 30*16)
	MDRV_SCREEN_VISIBLE_AREA(0, 64*16-1, 0, 30*16-1)

	MDRV_PALETTE_LENGTH(0x8000)

	MDRV_GFXDECODE(namcos23)

	MDRV_VIDEO_START(ss23)
	MDRV_VIDEO_UPDATE(ss23)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("c352", C352, 14745600)
	MDRV_SOUND_ROUTE(0, "rspeaker", 1.00)
	MDRV_SOUND_ROUTE(1, "lspeaker", 1.00)
	MDRV_SOUND_ROUTE(2, "rspeaker", 1.00)
	MDRV_SOUND_ROUTE(3, "lspeaker", 1.00)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( ss23 )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", R4650BE, 166000000)
	MDRV_CPU_CONFIG(config)
	MDRV_CPU_PROGRAM_MAP(ss23_map)
	MDRV_CPU_VBLANK_INT("screen", namcos23_interrupt)

	MDRV_CPU_ADD("audiocpu", H83002, 14745600 )
	MDRV_CPU_PROGRAM_MAP( s23h8rwmap)
	MDRV_CPU_IO_MAP( s23h8ionoiobmap)
	MDRV_CPU_VBLANK_INT("screen", irq1_line_pulse)

	MDRV_QUANTUM_TIME(HZ(60000))

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(48*16, 30*16)
	MDRV_SCREEN_VISIBLE_AREA(0, 48*16-1, 0, 30*16-1)

	MDRV_PALETTE_LENGTH(0x8000)

	MDRV_GFXDECODE(namcos23)

	MDRV_VIDEO_START(ss23)
	MDRV_VIDEO_UPDATE(ss23)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("c352", C352, 14745600)
	MDRV_SOUND_ROUTE(0, "rspeaker", 1.00)
	MDRV_SOUND_ROUTE(1, "lspeaker", 1.00)
	MDRV_SOUND_ROUTE(2, "rspeaker", 1.00)
	MDRV_SOUND_ROUTE(3, "lspeaker", 1.00)
MACHINE_DRIVER_END

ROM_START( rapidrvr )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
        ROM_LOAD16_BYTE( "rd3verc.ic2",  0x000000, 0x200000, CRC(c15c0f30) SHA1(9f529232818f3e184f81f62408a5cad615b05613) )
        ROM_LOAD16_BYTE( "rd3verc.ic1",  0x000001, 0x200000, CRC(9d7f4411) SHA1(d049efaa539d36ed0f73ca3f50a8f7112e67f865) )

	ROM_REGION( 0x80000, "audiocpu", 0 )	/* Hitachi H8/3002 MCU code */
	ROM_LOAD16_WORD_SWAP( "rd3verc.ic3",  0x000000, 0x080000, CRC(6e26fbaf) SHA1(4ab6637d22f0d26f7e1d10e9c80059c56f64303d) )

	ROM_REGION( 0x800000, "sprite", 0 )	/* sprite? tilemap? tiles */
        ROM_LOAD16_BYTE( "rd1mtal.1j",   0x000000, 0x400000, CRC(8f0efa86) SHA1(9953461c258f2a96be275a7b18d6518ddfac3860) )
        ROM_LOAD16_BYTE( "rd1mtah.3j",   0x000001, 0x400000, CRC(d8fa0f3d) SHA1(0d5bdb3a2e7be1dffe11b74baa2c10bfe011ae92) )

	ROM_REGION( 0x2000000, "textile", 0 )	/* texture tiles */
        ROM_LOAD( "rd1cguu.5b",   0x0000000, 0x800000, CRC(611bab41) SHA1(84cddb2b63bf8336e92aecb06eddf1b34af73540) )
        ROM_LOAD( "rd1cgum.6b",   0x0800000, 0x800000, CRC(c50de2ef) SHA1(24758a72b3569ce6a643a5786fce7c34b8aa692d) )
        ROM_LOAD( "rd1cgll.8b",   0x1000000, 0x800000, CRC(b58b92ac) SHA1(70ee6e0e5347e05817aa30d53d766b8ce0fc44e4) )
        ROM_LOAD( "rd1cglm.7b",   0x1800000, 0x800000, CRC(447067fa) SHA1(e2052373773594feb303e1924a4a820cf34ab55b) )

	ROM_REGION( 0x2000000, "textile2", 0 )	/* texture tiles bank 2? */
        ROM_LOAD( "rd1spruu.9p",  0x0000000, 0x400000, CRC(f20a9673) SHA1(e5f1d552b0c42e102593ab578ff0b9ff814f8650) )
        ROM_LOAD( "rd1sprum.10p", 0x0800000, 0x400000, CRC(8e08b2c6) SHA1(a17331a4e41f677f604d1b74e7694cf920b03b66) )
        ROM_LOAD( "rd1sprll.12t", 0x1000000, 0x400000, CRC(8d450259) SHA1(27cccd1e7dad8880147bb85185982d8d27076e69) )
        ROM_LOAD( "rd1sprlm.11p", 0x1800000, 0x400000, CRC(6c8db3a5) SHA1(24d81fa11e9c835cddadec4cbd530738e258346c) )

	ROM_REGION( 0x2000000, "textiledup", 0 )	/* duplicate bank of texture tiles */
        ROM_LOAD( "rd1cguu.5f",   0x0800000, 0x800000, CRC(611bab41) SHA1(84cddb2b63bf8336e92aecb06eddf1b34af73540) )
        ROM_LOAD( "rd1cgum.6f",   0x1800000, 0x800000, CRC(c50de2ef) SHA1(24758a72b3569ce6a643a5786fce7c34b8aa692d) )
        ROM_LOAD( "rd1cgll.8f",   0x1000000, 0x800000, CRC(b58b92ac) SHA1(70ee6e0e5347e05817aa30d53d766b8ce0fc44e4) )
        ROM_LOAD( "rd1cglm.7f",   0x0000000, 0x800000, CRC(447067fa) SHA1(e2052373773594feb303e1924a4a820cf34ab55b) )

	ROM_REGION( 0x2000000, "textile2d", 0 )	/* duplicate of texture tiles bank 2? */
        ROM_LOAD( "rd1spruu.9t",  0x0000000, 0x400000, CRC(f20a9673) SHA1(e5f1d552b0c42e102593ab578ff0b9ff814f8650) )
        ROM_LOAD( "rd1sprum.10t", 0x0800000, 0x400000, CRC(8e08b2c6) SHA1(a17331a4e41f677f604d1b74e7694cf920b03b66) )
        ROM_LOAD( "rd1sprll.12p", 0x1000000, 0x400000, CRC(8d450259) SHA1(27cccd1e7dad8880147bb85185982d8d27076e69) )
        ROM_LOAD( "rd1sprlm.11t", 0x1800000, 0x400000, CRC(6c8db3a5) SHA1(24d81fa11e9c835cddadec4cbd530738e258346c) )

	ROM_REGION( 0x400000, "textilemap", 0 )	/* texture tilemap */
        ROM_LOAD( "rd1ccrl.11a",  0x000000, 0x200000, CRC(b0ea2b32) SHA1(0dc45846725b0de619bc6bae69e3eb166ed21bf0) )
        ROM_LOAD( "rd1ccrh.11b",  0x200000, 0x200000, CRC(fafffb86) SHA1(15b0ba0252b99d0cac29fcb374fb895643f528fe) )

	ROM_REGION( 0x400000, "textilemp2", 0 )	/* duplicate texture tilemap */
        ROM_LOAD( "rd1ccrl.11e",  0x000000, 0x200000, CRC(b0ea2b32) SHA1(0dc45846725b0de619bc6bae69e3eb166ed21bf0) )
        ROM_LOAD( "rd1ccrh.11f",  0x200000, 0x200000, CRC(fafffb86) SHA1(15b0ba0252b99d0cac29fcb374fb895643f528fe) )

	ROM_REGION32_LE( 0x2000000, "pointrom", 0 )	/* 3D model data */
        ROM_LOAD32_WORD( "rd1pt0l.9j",   0x0000000, 0x400000, CRC(47b1c5a5) SHA1(021d4ca7b8674d8ed5daa701bf41b4a7164d992a) )
        ROM_LOAD32_WORD( "rd1pt0h.9l",   0x0000002, 0x400000, CRC(6f280eff) SHA1(9dd8c8903581d7a412146e50f4009e1d2b743f06) )
        ROM_LOAD32_WORD( "rd1pt1l.10j",  0x0800000, 0x400000, CRC(91131cb3) SHA1(e42c5e190c719f1cf2d6e91444062ab901be0e73) )
        ROM_LOAD32_WORD( "rd1pt1h.10l",  0x0800002, 0x400000, CRC(37bd9bdf) SHA1(b26c284024ea4ad4c67b2eefbfdd5ebb35a0118e) )
        ROM_LOAD32_WORD( "rd1pt2l.11j",  0x1000000, 0x400000, CRC(3423ff9f) SHA1(73823c179c866cbb601a23417acbbf5b3dc97213) )
        ROM_LOAD32_WORD( "rd1pt2h.11l",  0x1000002, 0x400000, CRC(fa601e83) SHA1(45c420538910f566e75d668306735f54c901669f) )
        ROM_LOAD32_WORD( "rd1pt3l.12j",  0x1800000, 0x400000, CRC(7216d63e) SHA1(77088ff05c2630996f4bdc87fe466f9b97611467) )
        ROM_LOAD32_WORD( "rd1pt3h.12l",  0x1800002, 0x400000, CRC(e82ff66a) SHA1(9e2c951136b26d969d2c9d030b7e0bad8bbbe3fb) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
        ROM_LOAD( "rd1wavel.2s",  0x000000, 0x800000, CRC(bf52c08c) SHA1(6745062e078e520484390fad1f723124aa4076d0) )
        ROM_LOAD( "rd1waveh.3s",  0x800000, 0x800000, CRC(ef0136b5) SHA1(a6d923ededca168fe555e0b86a72f53bec5424cc) )
ROM_END

ROM_START( motoxgo )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
        ROM_LOAD16_BYTE( "mg3vera.ic2",  0x000000, 0x200000, CRC(1bf06f00) SHA1(e9d04e9f19bff7a58cb280dd1d5db12801b68ba0) )
        ROM_LOAD16_BYTE( "mg3vera.ic1",  0x000001, 0x200000, CRC(f5e6e25b) SHA1(1de30e8e831be66987112645a9db3a3001b89fe6) )

	ROM_REGION( 0x80000, "audiocpu", 0 )	/* Hitachi H8/3002 MCU code */
        ROM_LOAD16_WORD_SWAP( "mg3vera.ic3",  0x000000, 0x080000, CRC(9e3d46a8) SHA1(9ffa5b91ea51cc0fb97def25ce47efa3441f3c6f) )

	ROM_REGION( 0x40000, "ioboard", 0 )	/* I/O board HD643334 H8/3334 MCU code */
        ROM_LOAD( "asca-3a.ic14", 0x000000, 0x040000, CRC(8e9266e5) SHA1(ffa8782ca641d71d57df23ed1c5911db05d3df97) )

	ROM_REGION( 0x20000, "exioboard", 0 )	/* "extra" I/O board (uses Fujitsu MB90611A MCU) */
        ROM_LOAD( "mg1prog0a.3a", 0x000000, 0x020000, CRC(b2b5be8f) SHA1(803652b7b8fde2196b7fb742ba8b9843e4fcd2de) )

	ROM_REGION( 0x2000000, "sprite", ROMREGION_ERASEFF )	/* sprite? tilemap? tiles */
        ROM_LOAD16_BYTE( "mg1mtal.2h",   0x000000, 0x800000, CRC(fdad0f0a) SHA1(420d50f012af40f80b196d3aae320376e6c32367) )
        ROM_LOAD16_BYTE( "mg1mtah.2j",   0x000001, 0x800000, CRC(845f4768) SHA1(9c03b1f6dcd9d1f43c2958d855221be7f9415c47) )

	ROM_REGION( 0x2000000, "textile", ROMREGION_ERASEFF )	/* texture tiles */
        ROM_LOAD( "mg1cgum.4j",   0x000000, 0x800000, CRC(46a77d73) SHA1(132ce2452ee68ba374e98b59032ac0a1a277078d) )
        ROM_LOAD( "mg1cgll.4m",   0x000000, 0x800000, CRC(175dfe34) SHA1(66ae35b0084159aea1afeb1a6486fffa635992b5) )
        ROM_LOAD( "mg1cglm.4k",   0x000000, 0x800000, CRC(b3e648e7) SHA1(98018ae2276f905a7f74e1dab540a44247524436) )

	ROM_REGION( 0x2000000, "textile2", ROMREGION_ERASEFF )	/* second copy of texture tiles */
        ROM_LOAD( "mg1cgum.5j",   0x000000, 0x800000, CRC(46a77d73) SHA1(132ce2452ee68ba374e98b59032ac0a1a277078d) )
        ROM_LOAD( "mg1cgll.5m",   0x000000, 0x800000, CRC(175dfe34) SHA1(66ae35b0084159aea1afeb1a6486fffa635992b5) )
        ROM_LOAD( "mg1cglm.5k",   0x000000, 0x800000, CRC(b3e648e7) SHA1(98018ae2276f905a7f74e1dab540a44247524436) )

	ROM_REGION( 0x600000, "textilemap", ROMREGION_ERASEFF )	/* texture tilemap */
        ROM_LOAD( "mg1ccrl.7f",   0x000000, 0x400000, CRC(5372e300) SHA1(63a49782289ed93a321ca7d193241fb83ca97e6b) )
        ROM_LOAD( "mg1ccrh.7e",   0x400000, 0x200000, CRC(2e77597d) SHA1(58dd83c1b0c08115e728c5e7dea5e62135b821ba) )

	ROM_REGION( 0x600000, "textilemap2", ROMREGION_ERASEFF) /* second copy of texture tilemap */
        ROM_LOAD( "mg1ccrl.7m",   0x000000, 0x400000, CRC(5372e300) SHA1(63a49782289ed93a321ca7d193241fb83ca97e6b) )
        ROM_LOAD( "mg1ccrh.7k",   0x400000, 0x200000, CRC(2e77597d) SHA1(58dd83c1b0c08115e728c5e7dea5e62135b821ba) )

	ROM_REGION32_LE( 0x2000000, "pointrom", ROMREGION_ERASEFF )	/* 3D model data */
        ROM_LOAD32_WORD( "mg1pt0l.7c",   0x000000, 0x400000, CRC(3b9e95d3) SHA1(d7823ed6c590669ccd4098ed439599a3eb814ed1) )
        ROM_LOAD32_WORD( "mg1pt0h.7a",   0x000002, 0x400000, CRC(c9ba1b47) SHA1(42ec0638edb4c502ff0a340c4cf590bdd767cfe2) )
        ROM_LOAD32_WORD( "mg1pt1h.5a",   0x800000, 0x400000, CRC(8d4f7097) SHA1(004e9ed0b5d6ce83ffadb9bd429fa7560abdb598) )
        ROM_LOAD32_WORD( "mg1pt1l.5c",   0x800002, 0x400000, CRC(0dd2f358) SHA1(3537e6be3fec9fec8d5a8dd02d9cf67b3805f8f0) )

	ROM_REGION( 0x1000000, "c352", ROMREGION_ERASEFF ) /* C352 PCM samples */
        ROM_LOAD( "mg1wavel.2c",  0x000000, 0x800000, CRC(f78b1b4d) SHA1(47cd654ec0a69de0dc81b8d83692eebf5611228b) )
        ROM_LOAD( "mg1waveh.2a",  0x800000, 0x800000, CRC(8cb73877) SHA1(2e2b170c7ff889770c13b4ab7ac316b386ada153) )
ROM_END

ROM_START( timecrs2 )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
        ROM_LOAD16_BYTE( "tss3verb.2",   0x000000, 0x200000, CRC(c7be691f) SHA1(5e2e7a0db3d8ce6dfeb6c0d99e9fe6a9f9cab467) )
        ROM_LOAD16_BYTE( "tss3verb.1",   0x000001, 0x200000, CRC(6e3f232b) SHA1(8007d8f31a605a5df89938d7c9f9d3d209c934be) )

	ROM_REGION( 0x80000, "audiocpu", 0 )	/* Hitachi H8/3002 MCU code */
        ROM_LOAD16_WORD_SWAP( "tss3verb.3",   0x000000, 0x080000, CRC(41e41994) SHA1(eabc1a307c329070bfc6486cb68169c94ff8a162) )

	ROM_REGION( 0x40000, "ioboard", 0 )	/* I/O board HD643334 H8/3334 MCU code */
	ROM_LOAD( "tssioprog.ic3", 0x000000, 0x040000, CRC(edad4538) SHA1(1330189184a636328d956c0e435f8d9ad2e96a80) )

	ROM_REGION( 0x2000000, "sprite", 0 )	/* sprite? tilemap? tiles */
        ROM_LOAD16_BYTE( "tss1mtal.2h",  0x0000000, 0x800000, CRC(bfc79190) SHA1(04bda00c4cc5660d27af4f3b0ee3550dea8d3805) )
        ROM_LOAD16_BYTE( "tss1mtah.2j",  0x0000001, 0x800000, CRC(697c26ed) SHA1(72f6f69e89496ba0c6183b35c3bde71f5a3c721f) )
        ROM_LOAD16_BYTE( "tss1mtbl.2f",  0x1000000, 0x800000, CRC(e648bea4) SHA1(3803d03e72b25fbcc124d5b25066d25629b76b94) )
        ROM_LOAD16_BYTE( "tss1mtbh.2m",  0x1000001, 0x800000, CRC(82582776) SHA1(7c790d09bac660ea1c62da3ffb21ab43f2461594) )

	ROM_REGION( 0x2000000, "textile", 0 )	/* texture tiles */
        ROM_LOAD( "tss1cguu.4f",  0x0000000, 0x800000, CRC(76924e04) SHA1(751065d6ce658cbbcd88f854f6937ebd2204ec68) )
        ROM_LOAD( "tss1cgum.4j",  0x0800000, 0x800000, CRC(c22739e1) SHA1(8671ee047bb248033656c50befd1c35e5e478e1a) )
        ROM_LOAD( "tss1cgll.4m",  0x1000000, 0x800000, CRC(18433aaa) SHA1(08539beb2e66ec4e41062621fc098b121c669546) )
        ROM_LOAD( "tss1cglm.4k",  0x1800000, 0x800000, CRC(669974c2) SHA1(cfebe199631e38f547b38fcd35f1645b74e8dd0a) )

	ROM_REGION( 0x600000, "textilemap", 0 )	/* texture tilemap */
        ROM_LOAD( "tss1ccrl.7f",  0x000000, 0x400000, CRC(3a325fe7) SHA1(882735dce7aeb36f9e88a983498360f5de901e9d) )
        ROM_LOAD( "tss1ccrh.7e",  0x400000, 0x200000, CRC(f998de1a) SHA1(371f540f505608297c5ffcfb623b983ca8310afb) )

	ROM_REGION32_LE( 0x2000000, "pointrom", 0 )	/* 3D model data */
        ROM_LOAD32_WORD( "tss1pt0l.7c",  0x0000000, 0x400000, CRC(896f0fb4) SHA1(bdfa99eb21ce4fc8021f9d95a5558a34f9942c57) )
        ROM_LOAD32_WORD( "tss1pt0h.7a",  0x0000002, 0x400000, CRC(cdbe0ba8) SHA1(f8c6da31654c0a2a8024888ffb7fc1c783b2d629) )
        ROM_LOAD32_WORD( "tss1pt1l.5c",  0x0800000, 0x400000, CRC(5a09921f) SHA1(c23885708c7adf0b81c2c9346e21b869634a5b35) )
        ROM_LOAD32_WORD( "tss1pt1h.5a",  0x0800002, 0x400000, CRC(63647596) SHA1(833412be8f61686bd7e06c2738df740e0e585d0f) )
        ROM_LOAD32_WORD( "tss1pt2l.4c",  0x1000000, 0x400000, CRC(4b230d79) SHA1(794cee0a19993e90913f58507c53224f361e9663) )
        ROM_LOAD32_WORD( "tss1pt2h.4a",  0x1000002, 0x400000, CRC(9b06e22d) SHA1(cff5ed098112a4f0a2bc8937e226f50066e605b1) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
        ROM_LOAD( "tss1wavel.2c", 0x000000, 0x800000, CRC(deaead26) SHA1(72dac0c3f41d4c3c290f9eb1b50236ae3040a472) )
        ROM_LOAD( "tss1waveh.2a", 0x800000, 0x800000, CRC(5c8758b4) SHA1(b85c8f6869900224ef83a2340b17f5bbb2801af9) )
ROM_END

ROM_START( timecrs2b )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
        ROM_LOAD16_BYTE( "tss2verb.ic2", 0x000000, 0x200000, CRC(9f56a4df) SHA1(5ecb3cd93726ab6be02762853fd6a45266d6c0bc) )
        ROM_LOAD16_BYTE( "tss2verb.ic1", 0x000001, 0x200000, CRC(aa147f71) SHA1(e00267d1a8286942c83dc35289ad65bd3cb6d8db) )

	ROM_REGION( 0x80000, "audiocpu", 0 )	/* Hitachi H8/3002 MCU code */
        ROM_LOAD16_WORD_SWAP( "tss3verb.3",   0x000000, 0x080000, CRC(41e41994) SHA1(eabc1a307c329070bfc6486cb68169c94ff8a162) )

	ROM_REGION( 0x40000, "ioboard", 0 )	/* I/O board HD643334 H8/3334 MCU code */
	ROM_LOAD( "tssioprog.ic3", 0x000000, 0x040000, CRC(edad4538) SHA1(1330189184a636328d956c0e435f8d9ad2e96a80) )

	ROM_REGION( 0x2000000, "sprite", 0 )	/* sprite? tilemap? tiles */
        ROM_LOAD16_BYTE( "tss1mtal.2h",  0x0000000, 0x800000, CRC(bfc79190) SHA1(04bda00c4cc5660d27af4f3b0ee3550dea8d3805) )
        ROM_LOAD16_BYTE( "tss1mtah.2j",  0x0000001, 0x800000, CRC(697c26ed) SHA1(72f6f69e89496ba0c6183b35c3bde71f5a3c721f) )
        ROM_LOAD16_BYTE( "tss1mtbl.2f",  0x1000000, 0x800000, CRC(e648bea4) SHA1(3803d03e72b25fbcc124d5b25066d25629b76b94) )
        ROM_LOAD16_BYTE( "tss1mtbh.2m",  0x1000001, 0x800000, CRC(82582776) SHA1(7c790d09bac660ea1c62da3ffb21ab43f2461594) )

	ROM_REGION( 0x2000000, "textile", 0 )	/* texture tiles */
        ROM_LOAD( "tss1cguu.4f",  0x0000000, 0x800000, CRC(76924e04) SHA1(751065d6ce658cbbcd88f854f6937ebd2204ec68) )
        ROM_LOAD( "tss1cgum.4j",  0x0800000, 0x800000, CRC(c22739e1) SHA1(8671ee047bb248033656c50befd1c35e5e478e1a) )
        ROM_LOAD( "tss1cgll.4m",  0x1000000, 0x800000, CRC(18433aaa) SHA1(08539beb2e66ec4e41062621fc098b121c669546) )
        ROM_LOAD( "tss1cglm.4k",  0x1800000, 0x800000, CRC(669974c2) SHA1(cfebe199631e38f547b38fcd35f1645b74e8dd0a) )

	ROM_REGION( 0x600000, "textilemap", 0 )	/* texture tilemap */
        ROM_LOAD( "tss1ccrl.7f",  0x000000, 0x400000, CRC(3a325fe7) SHA1(882735dce7aeb36f9e88a983498360f5de901e9d) )
        ROM_LOAD( "tss1ccrh.7e",  0x400000, 0x200000, CRC(f998de1a) SHA1(371f540f505608297c5ffcfb623b983ca8310afb) )

	ROM_REGION32_LE( 0x2000000, "pointrom", 0 )	/* 3D model data */
        ROM_LOAD32_WORD( "tss1pt0l.7c",  0x0000000, 0x400000, CRC(896f0fb4) SHA1(bdfa99eb21ce4fc8021f9d95a5558a34f9942c57) )
        ROM_LOAD32_WORD( "tss1pt0h.7a",  0x0000002, 0x400000, CRC(cdbe0ba8) SHA1(f8c6da31654c0a2a8024888ffb7fc1c783b2d629) )
        ROM_LOAD32_WORD( "tss1pt1l.5c",  0x0800000, 0x400000, CRC(5a09921f) SHA1(c23885708c7adf0b81c2c9346e21b869634a5b35) )
        ROM_LOAD32_WORD( "tss1pt1h.5a",  0x0800002, 0x400000, CRC(63647596) SHA1(833412be8f61686bd7e06c2738df740e0e585d0f) )
        ROM_LOAD32_WORD( "tss1pt2l.4c",  0x1000000, 0x400000, CRC(4b230d79) SHA1(794cee0a19993e90913f58507c53224f361e9663) )
        ROM_LOAD32_WORD( "tss1pt2h.4a",  0x1000002, 0x400000, CRC(9b06e22d) SHA1(cff5ed098112a4f0a2bc8937e226f50066e605b1) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
        ROM_LOAD( "tss1wavel.2c", 0x000000, 0x800000, CRC(deaead26) SHA1(72dac0c3f41d4c3c290f9eb1b50236ae3040a472) )
        ROM_LOAD( "tss1waveh.2a", 0x800000, 0x800000, CRC(5c8758b4) SHA1(b85c8f6869900224ef83a2340b17f5bbb2801af9) )
ROM_END

ROM_START( 500gp )
	/* r4650-generic-xrom-generic: NMON 1.0.8-sys23-19990105 P for SYSTEM23 P1 */
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
        ROM_LOAD16_BYTE( "5gp3verc.2",   0x000000, 0x200000, CRC(e2d43468) SHA1(5e861dd223c7fa177febed9803ac353cba18e19d) )
        ROM_LOAD16_BYTE( "5gp3verc.1",   0x000001, 0x200000, CRC(f6efc94a) SHA1(785eee2bec5080d4e8ef836f28d446328c942b0e) )

	ROM_REGION( 0x80000, "audiocpu", 0 )	/* Hitachi H8/3002 MCU code */
        ROM_LOAD16_WORD_SWAP( "5gp3verc.3",   0x000000, 0x080000, CRC(b323abdf) SHA1(8962e39b48a7074a2d492afb5db3f5f3e5ae2389) )

	ROM_REGION( 0x2000000, "sprite", 0 )	/* sprite? tilemap? tiles */
		ROM_LOAD16_BYTE( "5gp1mtal.2h",  0x0000000, 0x800000, CRC(1bb00c7b) SHA1(922be45d57330c31853b2dc1642c589952b09188) )
        ROM_LOAD16_BYTE( "5gp1mtah.2j",  0x0000001, 0x800000, CRC(246e4b7a) SHA1(75743294b8f48bffb84f062febfbc02230d49ce9) )

		/* COMMON FUJII YASUI WAKAO KURE INOUE
         * 0x000000..0x57ffff: all 0xff
         */
        ROM_LOAD16_BYTE( "5gp1mtbl.2f",  0x1000000, 0x800000, CRC(66640606) SHA1(c69a0219748241c49315d7464f8156f8068e9cf5) )
        ROM_LOAD16_BYTE( "5gp1mtbh.2m",  0x1000001, 0x800000, CRC(352360e8) SHA1(d621dfac3385059c52d215f6623901589a8658a3) )

	ROM_REGION( 0x2000000, "textile", 0 )	/* texture tiles */
        ROM_LOAD( "5gp1cguu.4f",  0x0000000, 0x800000, CRC(c411163b) SHA1(ae644d62357b8b806b160774043e41908fba5d05) )
        ROM_LOAD( "5gp1cgum.4j",  0x0800000, 0x800000, CRC(0265b701) SHA1(497a4c33311d3bb315100a78400cf2fa726f1483) )
        ROM_LOAD( "5gp1cgll.4m",  0x1000000, 0x800000, CRC(0cc5bf35) SHA1(b75510a94fa6b6d2ed43566e6e84c7ae62f68194) )
        ROM_LOAD( "5gp1cglm.4k",  0x1800000, 0x800000, CRC(31557d48) SHA1(b85c3db20b101ba6bdd77487af67c3324bea29d5) )

	ROM_REGION( 0x600000, "textilemap", 0 )	/* texture tilemap */
        ROM_LOAD( "5gp1ccrl.7f",  0x000000, 0x400000, CRC(e7c77e1f) SHA1(0231ddbe2afb880099dfe2657c41236c74c730bb) )
        ROM_LOAD( "5gp1ccrh.7e",  0x400000, 0x200000, CRC(b2eba764) SHA1(5e09d1171f0afdeb9ed7337df1dbc924f23d3a0b) )

	ROM_REGION32_LE( 0x2000000, "pointrom", 0 )	/* 3D model data */
        ROM_LOAD32_WORD( "5gp1pt0l.7c",  0x0000000, 0x400000, CRC(a0ece0a1) SHA1(b7aab2d78e1525f865214c7de387ccd585de5d34) )
        ROM_LOAD32_WORD( "5gp1pt0h.7a",  0x0000002, 0x400000, CRC(5746a8cd) SHA1(e70fc596ab9360f474f716c73d76cb9851370c76) )
        ROM_LOAD32_WORD( "5gp1pt1l.5c",  0x0800000, 0x400000, CRC(80b25ad2) SHA1(e9a03fe5bb4ce925f7218ab426ed2a1ca1a26a62) )
        ROM_LOAD32_WORD( "5gp1pt1h.5a",  0x0800002, 0x400000, CRC(b1feb5df) SHA1(45db259215511ac3e472895956f70204d4575482) )
		ROM_LOAD32_WORD( "5gp1pt2l.4c",  0x1000000, 0x400000, CRC(9289dbeb) SHA1(ec546ad3b1c90609591e599c760c70049ba3b581) )
        ROM_LOAD32_WORD( "5gp1pt2h.4a",  0x1000002, 0x400000, CRC(9a693771) SHA1(c988e04cd91c3b7e75b91376fd73be4a7da543e7) )
		ROM_LOAD32_WORD( "5gp1pt3l.3c",  0x1800000, 0x400000, CRC(480b120d) SHA1(6c703550faa412095d9633cf508050614e15fbae) )
        ROM_LOAD32_WORD( "5gp1pt3h.3a",  0x1800002, 0x400000, CRC(26eaa400) SHA1(0157b76fffe81b40eb970e84c98398807ced92c4) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
        ROM_LOAD( "5gp1wavel.2c", 0x000000, 0x800000, CRC(aa634cc2) SHA1(e96f5c682039bc6ef22bf90e98f4da78486bd2b1) )
        ROM_LOAD( "5gp1waveh.2a", 0x800000, 0x800000, CRC(1e3523e8) SHA1(cb3d0d389fcbfb728fad29cfc36ef654d28d553a) )
ROM_END

ROM_START( finfurl2 )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
        ROM_LOAD16_BYTE( "29f016.ic2",   0x000000, 0x200000, CRC(13cbc545) SHA1(3e67a7bfbb1c1374e8e3996a0c09e4861b0dca14) )
        ROM_LOAD16_BYTE( "29f016.ic1",   0x000001, 0x200000, CRC(5b04e4f2) SHA1(8099fc3deab9ed14a2484a774666fbd928330de8) )

	ROM_REGION( 0x80000, "audiocpu", 0 )	/* Hitachi H8/3002 MCU code */
        ROM_LOAD16_WORD_SWAP( "m29f400.ic3",  0x000000, 0x080000, CRC(9fd69bbd) SHA1(53a9bf505de70495dcccc43fdc722b3381aad97c) )

	ROM_REGION( 0x2000000, "sprite", 0 )	/* sprite? tilemap? tiles */
        ROM_LOAD16_BYTE( "ffs1mtal.2h",  0x0000000, 0x800000, CRC(98730ad5) SHA1(9ba276ad88ec8730edbacab80cdacc34a99593e4) )
        ROM_LOAD16_BYTE( "ffs1mtah.2j",  0x0000001, 0x800000, CRC(f336d81d) SHA1(a9177091e1412dea1b6ea6c53530ae31361b32d0) )
        ROM_LOAD16_BYTE( "ffs1mtbl.2f",  0x1000000, 0x800000, CRC(0abc9e50) SHA1(be5e5e2b637811c59804ef9442c6da5a5a1315e2) )
        ROM_LOAD16_BYTE( "ffs1mtbh.2m",  0x1000001, 0x800000, CRC(0f42c93b) SHA1(26b313fc5c33afb0a1ee42243486e38f052c95c2) )

	ROM_REGION( 0x2000000, "textile", 0 )	/* texture tiles */
        ROM_LOAD( "ffs1cguu.4f",  0x0000000, 0x800000, CRC(52c0a19f) SHA1(e6b4b90ff88da09cb2e653e450e7ae66942a719e) )
        ROM_LOAD( "ffs1cgum.4j",  0x0800000, 0x800000, CRC(77447199) SHA1(1eeae30b3dd1ac467bdbbdfe4be36ca0f0816496) )
        ROM_LOAD( "ffs1cgll.4m",  0x1000000, 0x800000, CRC(171bba76) SHA1(4a63a1f34de8f341a0ef9b499a21e8fec758e1cd) )
        ROM_LOAD( "ffs1cglm.4k",  0x1800000, 0x800000, CRC(48acf207) SHA1(ea902efdd94aba34dadb20762219d2d25441d199) )

	ROM_REGION( 0x400000, "textilemap", 0 )	/* texture tilemap */
        ROM_LOAD( "ffs1ccrl.7f",  0x000000, 0x200000, CRC(ffbcfec1) SHA1(9ab25f1543da4b72784eec93985abaa2e1dafc83) )
        ROM_LOAD( "ffs1ccrh.7e",  0x200000, 0x200000, CRC(8be4aeb4) SHA1(ec344f6fba42092083e737e436451f5d7be12c15) )

	ROM_REGION32_LE( 0x2000000, "pointrom", 0 )	/* 3D model data */
        ROM_LOAD32_WORD( "ffs1pt0l.7c",  0x0000000, 0x400000, CRC(383cbfba) SHA1(0784ac2d709bee6653c95f80fedf7f98ca79357f) )
        ROM_LOAD32_WORD( "ffs1pt0h.7a",  0x0000002, 0x400000, CRC(79b9b019) SHA1(ca2bbabd949fec91001a30b63f7343520028cde0) )
        ROM_LOAD32_WORD( "ffs1pt1l.5c",  0x0800000, 0x400000, CRC(ba0fff5b) SHA1(d5a6db4de60657d46228e85ed09ed7f0ecbc7975) )
        ROM_LOAD32_WORD( "ffs1pt1h.5a",  0x0800002, 0x400000, CRC(2dba59d0) SHA1(34d4c415b5635338511ff3578eb3c00e2b6cd7d4) )
        ROM_LOAD32_WORD( "ffs1pt2l.4c",  0x1000000, 0x400000, CRC(c5199c1b) SHA1(8f1a70c8edb2791a099b4911353af6250a5d0e8a) )
        ROM_LOAD32_WORD( "ffs1pt2h.4a",  0x1000002, 0x400000, CRC(26ea01e8) SHA1(9af096c99e6835e21b1b78dfce07040f50f8c922) )
        ROM_LOAD32_WORD( "ffs1pt3l.3c",  0x1800000, 0x400000, CRC(2381611a) SHA1(a3d948bf910dcfd9f47c65c56b9920f58c42fed5) )
        ROM_LOAD32_WORD( "ffs1pt3h.3a",  0x1800002, 0x400000, CRC(48226e9f) SHA1(f099b2929d49903a33b4dab80972c3ce0ddb6ca2) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
        ROM_LOAD( "ffs1wavel.2c", 0x000000, 0x800000, CRC(67ba16cf) SHA1(00b38617c2185b9a3bf279962ad0c21a7287256f) )
        ROM_LOAD( "ffs1waveh.2a", 0x800000, 0x800000, CRC(178e8bd3) SHA1(8ab1a97003914f70b09e96c5924f3a839fe634c7) )
ROM_END

ROM_START( finfurl2j )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
        ROM_LOAD16_BYTE( "29f016_jap1.ic2", 0x000000, 0x200000, CRC(0215125d) SHA1(a99f601441c152b0b00f4811e5752c71897b1ed4) )
        ROM_LOAD16_BYTE( "29f016_jap1.ic1", 0x000001, 0x200000, CRC(38c9ae96) SHA1(b50afc7276662267ff6460f82d0e5e8b00b341ea) )

	ROM_REGION( 0x80000, "audiocpu", 0 )	/* Hitachi H8/3002 MCU code */
        ROM_LOAD16_WORD_SWAP( "m29f400.ic3",  0x000000, 0x080000, CRC(9fd69bbd) SHA1(53a9bf505de70495dcccc43fdc722b3381aad97c) )

	ROM_REGION( 0x2000000, "sprite", 0 )	/* sprite? tilemap? tiles */
        ROM_LOAD16_BYTE( "ffs1mtal.2h",  0x0000000, 0x800000, CRC(98730ad5) SHA1(9ba276ad88ec8730edbacab80cdacc34a99593e4) )
        ROM_LOAD16_BYTE( "ffs1mtah.2j",  0x0000001, 0x800000, CRC(f336d81d) SHA1(a9177091e1412dea1b6ea6c53530ae31361b32d0) )
        ROM_LOAD16_BYTE( "ffs1mtbl.2f",  0x1000000, 0x800000, CRC(0abc9e50) SHA1(be5e5e2b637811c59804ef9442c6da5a5a1315e2) )
        ROM_LOAD16_BYTE( "ffs1mtbh.2m",  0x1000001, 0x800000, CRC(0f42c93b) SHA1(26b313fc5c33afb0a1ee42243486e38f052c95c2) )

	ROM_REGION( 0x2000000, "textile", 0 )	/* texture tiles */
        ROM_LOAD( "ffs1cguu.4f",  0x0000000, 0x800000, CRC(52c0a19f) SHA1(e6b4b90ff88da09cb2e653e450e7ae66942a719e) )
        ROM_LOAD( "ffs1cgum.4j",  0x0800000, 0x800000, CRC(77447199) SHA1(1eeae30b3dd1ac467bdbbdfe4be36ca0f0816496) )
        ROM_LOAD( "ffs1cgll.4m",  0x1000000, 0x800000, CRC(171bba76) SHA1(4a63a1f34de8f341a0ef9b499a21e8fec758e1cd) )
        ROM_LOAD( "ffs1cglm.4k",  0x1800000, 0x800000, CRC(48acf207) SHA1(ea902efdd94aba34dadb20762219d2d25441d199) )

	ROM_REGION( 0x400000, "textilemap", 0 )	/* texture tilemap */
        ROM_LOAD( "ffs1ccrl.7f",  0x000000, 0x200000, CRC(ffbcfec1) SHA1(9ab25f1543da4b72784eec93985abaa2e1dafc83) )
        ROM_LOAD( "ffs1ccrh.7e",  0x200000, 0x200000, CRC(8be4aeb4) SHA1(ec344f6fba42092083e737e436451f5d7be12c15) )

	ROM_REGION32_LE( 0x2000000, "pointrom", 0 )	/* 3D model data */
        ROM_LOAD32_WORD( "ffs1pt0l.7c",  0x0000000, 0x400000, CRC(383cbfba) SHA1(0784ac2d709bee6653c95f80fedf7f98ca79357f) )
        ROM_LOAD32_WORD( "ffs1pt0h.7a",  0x0000002, 0x400000, CRC(79b9b019) SHA1(ca2bbabd949fec91001a30b63f7343520028cde0) )
        ROM_LOAD32_WORD( "ffs1pt1l.5c",  0x0800000, 0x400000, CRC(ba0fff5b) SHA1(d5a6db4de60657d46228e85ed09ed7f0ecbc7975) )
        ROM_LOAD32_WORD( "ffs1pt1h.5a",  0x0800002, 0x400000, CRC(2dba59d0) SHA1(34d4c415b5635338511ff3578eb3c00e2b6cd7d4) )
        ROM_LOAD32_WORD( "ffs1pt2l.4c",  0x1000000, 0x400000, CRC(c5199c1b) SHA1(8f1a70c8edb2791a099b4911353af6250a5d0e8a) )
        ROM_LOAD32_WORD( "ffs1pt2h.4a",  0x1000002, 0x400000, CRC(26ea01e8) SHA1(9af096c99e6835e21b1b78dfce07040f50f8c922) )
        ROM_LOAD32_WORD( "ffs1pt3l.3c",  0x1800000, 0x400000, CRC(2381611a) SHA1(a3d948bf910dcfd9f47c65c56b9920f58c42fed5) )
        ROM_LOAD32_WORD( "ffs1pt3h.3a",  0x1800002, 0x400000, CRC(48226e9f) SHA1(f099b2929d49903a33b4dab80972c3ce0ddb6ca2) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
        ROM_LOAD( "ffs1wavel.2c", 0x000000, 0x800000, CRC(67ba16cf) SHA1(00b38617c2185b9a3bf279962ad0c21a7287256f) )
        ROM_LOAD( "ffs1waveh.2a", 0x800000, 0x800000, CRC(178e8bd3) SHA1(8ab1a97003914f70b09e96c5924f3a839fe634c7) )
ROM_END

ROM_START( panicprk )
	ROM_REGION32_BE( 0x400000, "user1", 0 ) /* 4 megs for main R4650 code */
        ROM_LOAD16_BYTE( "pnp2ver-a.ic2", 0x000000, 0x200000, CRC(cd528597) SHA1(cf390e78228eb10d5f50ff7e7e37063a2d87f469) )
        ROM_LOAD16_BYTE( "pnp2ver-a.ic1", 0x000001, 0x200000, CRC(80fea853) SHA1(b18003bde060ebb3c892a6d7fa4abf868cadc777) )

	ROM_REGION( 0x80000, "audiocpu", 0 )	/* Hitachi H8/3002 MCU code */
        ROM_LOAD16_WORD_SWAP( "pnp2ver-a.ic3", 0x000000, 0x080000, CRC(fe4bc6f4) SHA1(2114dc4bc63d589e6c3b26a73dbc60924f3b1765) )

	ROM_REGION( 0x2000000, "sprite", 0 )	/* sprite? tilemap? tiles */
        ROM_LOAD16_BYTE( "pnp1mtal.2h",  0x000000, 0x800000, CRC(6490faaa) SHA1(03443746009b434e5d4074ea6314910418907360) )
        ROM_LOAD16_BYTE( "pnp1mtah.2j",  0x000001, 0x800000, CRC(37addddd) SHA1(3032989653304417df80606bc3fde6e9425d8cbb) )

	ROM_REGION( 0x4000000, "textile", 0 )	/* texture tiles */
        ROM_LOAD( "pnp1cguu.5f",  0x000000, 0x800000, CRC(cd64f57f) SHA1(8780270298e0823db1acbbf79396788df0c3c19c) )
        ROM_LOAD( "pnp1cgum.4j",  0x800000, 0x800000, CRC(206217ca) SHA1(9c095bba7764f3405c3fab10513b9b78981ec44d) )
        ROM_LOAD( "pnp1cgll.4m",  0x1000000, 0x800000, CRC(d03932cf) SHA1(49240e44923cc6e815e9457b6290fd18466658af) )
        ROM_LOAD( "pnp1cglm.5k",  0x180000, 0x800000, CRC(abf4ccf2) SHA1(3848e26d0ba6c872bbc6d5e0eb23a9d4b34152d5) )
        ROM_LOAD( "pnp1cguu.4f",  0x2000000, 0x800000, CRC(cd64f57f) SHA1(8780270298e0823db1acbbf79396788df0c3c19c) )
        ROM_LOAD( "pnp1cgum.5j",  0x2800000, 0x800000, CRC(206217ca) SHA1(9c095bba7764f3405c3fab10513b9b78981ec44d) )
        ROM_LOAD( "pnp1cgll.5m",  0x3000000, 0x800000, CRC(d03932cf) SHA1(49240e44923cc6e815e9457b6290fd18466658af) )
        ROM_LOAD( "pnp1cglm.4k",  0x3800000, 0x800000, CRC(abf4ccf2) SHA1(3848e26d0ba6c872bbc6d5e0eb23a9d4b34152d5) )

	ROM_REGION( 0x800000, "textilemap", 0 )	/* texture tilemap */
        ROM_LOAD( "pnp1ccrl.7f",  0x000000, 0x200000, CRC(b7bc43c2) SHA1(f4b470540194486ca6822f438fc1d4700cfb2ab1) )
        ROM_LOAD( "pnp1ccrh.7e",  0x200000, 0x200000, CRC(caaf1b73) SHA1(b436992817ab4e4dad05e7429eb102d4fb57fa6a) )
        ROM_LOAD( "pnp1ccrl.7m",  0x400000, 0x200000, CRC(b7bc43c2) SHA1(f4b470540194486ca6822f438fc1d4700cfb2ab1) )
        ROM_LOAD( "pnp1ccrh.7k",  0x600000, 0x200000, CRC(caaf1b73) SHA1(b436992817ab4e4dad05e7429eb102d4fb57fa6a) )

	ROM_REGION32_LE( 0x2000000, "pointrom", 0 )	/* 3D model data */
        ROM_LOAD32_WORD( "pnp1pt0l.7c",  0x000000, 0x400000, CRC(26af5fa1) SHA1(12fcf98c2a59643e0fdfdd7186f9f16baf54a9cf) )
        ROM_LOAD32_WORD( "pnp1pt0h.7a",  0x000002, 0x400000, CRC(43fc2246) SHA1(301d321cd4a01ebd7ccfa6f295d6c3daf0a19efe) )
        ROM_LOAD32_WORD( "pnp1pt1l.5c",  0x800000, 0x400000, CRC(15c6f236) SHA1(e8c393359a91cdce6e9110a48c0a80708f8fc132) )
        ROM_LOAD32_WORD( "pnp1pt1h.5a",  0x800002, 0x400000, CRC(1ff470c0) SHA1(ca8fad90743589744939d681b0ce94f368337b3f) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* C352 PCM samples */
        ROM_LOAD( "pnp1wavel.2c", 0x000000, 0x800000, CRC(35c6a9bd) SHA1(4b56fdc37525c15e57d93091e6609d6a6905fc5c) )
        ROM_LOAD( "pnp1waveh.2a", 0x800000, 0x800000, CRC(6fa1826a) SHA1(20a5af49e65ae2bc57c016b5cd9bafa5a5220d35) )
ROM_END


/* Games */
GAME( 1997, rapidrvr, 0,      gorgon, ss23, ss23, ROT0, "Namco", "Rapid River (RD3 Ver. C)", GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_IMPERFECT_SOUND )
GAME( 1997, motoxgo,  0,         s23, ss23, ss23, ROT0, "Namco", "Motocross Go! (MG3 Ver. A)", GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_IMPERFECT_SOUND )
GAME( 1997, timecrs2, 0,         s23, ss23, ss23, ROT0, "Namco", "Time Crisis 2 (TSS3 Ver. B)", GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_IMPERFECT_SOUND )
GAME( 1997, timecrs2b,timecrs2,  s23, ss23, ss23, ROT0, "Namco", "Time Crisis 2 (TSS2 Ver. B)", GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_IMPERFECT_SOUND )
GAME( 1998, panicprk, 0,        ss23, ss23, ss23, ROT0, "Namco", "Panic Park (PNP2 Ver. A)", GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_IMPERFECT_SOUND )
GAME( 1999, 500gp,    0,        ss23, ss23, ss23, ROT0, "Namco", "500GP", GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_IMPERFECT_SOUND )
GAME( 1999, finfurl2, 0,        ss23, ss23, ss23, ROT0, "Namco", "Final Furlong 2 (World)", GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_IMPERFECT_SOUND )
GAME( 1999, finfurl2j,finfurl2, ss23, ss23, ss23, ROT0, "Namco", "Final Furlong 2 (Japan)", GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_IMPERFECT_SOUND )
