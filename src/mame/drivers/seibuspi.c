/*
      Seibu SPI Hardware
      Seibu SYS386

      Driver by Ville Linde


      Games on this hardware:

          Raiden Fighters
          Raiden Fighters 2
          Raiden Fighters Jet
          Senkyu / Battle Balls
          E-Jan High School
          Viper Phase 1

      Hardware:

          Intel 386 DX 25MHz
          Z80 8MHz (sound)
          YMF271F Sound chip
          Seibu Custom GFX chip

      SYS386 seems like a lower-cost version of single-board SPI.
      It has a 40MHz AMD 386 and a considerably weaker sound system (dual MSM6295).

NOTES:

    rjetus:
    - If you get a blank screen on startup you need to press F2 and choose "Reset settings",
      followed by "Exit". This will create a proper NVRAM

TODO:
- Alpha blending. Screen shot on www.system16.com show that during attract mode
  in Viper Phase 1 the "Viper" part of the logo (the red part) should be partially
  transparent. Same thing with the blu "Viper" logo when on the "push 1 or 2
  players button" screen. Note that the red logo is tiles, the blue logo is sprites.
  Same thing with the lights on the ground at the beginning of the game. They are
  opaque now, you should see the background tiles through.

*/

/*
Information from Guru

Seibu Kaihatsu SPI Hardware Overview
1995-1998 Seibu Kaihatsu Inc.

This system (known as 'SPI') consists of a main board and a plug-in
cartridge containing the game software. The games on SPI hardware can
be swapped by changing the top cartridge and then moving a jumper
to the alternative position. This re-flashes some ROMs for a few minutes
(accompanied by a techno music track). Afterwards, a message tells you to
put the jumper back to the original position and reboot the PCB. The new
game then plays.

There were a few revisions of this hardware, though most are the same with only
minor changes such as different IC revisions etc.

Games on this system include.....
Raiden Fighters
Raiden Fighters 2
Raiden Fighters Jet
Senkyu / Battle Balls
E-Jan High School
Viper Phase 1


Main Board Layout
-----------------
Revision 1
(C)1995 SXX2C-MAIN V2.0

Revision 2
(C)1998 SXX2C-MAIN V2.1
|-----------------------------------------------------------------------------|
|                  CN121       E28F008SA           XC7336(1)        Z80       |
|                                    E28F008SA                                |
|                      JP072                                                  |
|  VOL_L  VOL_R                                                               |
|                4560      |--------|            LH5496D-50                   |
|            4560    4560  |YMF271-F|                              XC7336(2)  |
|                          |        |16.9344MHz  LH5496D-50                   |
|              YAC513-F    |        |                                         |
|                          |--------|                                         |
|            JP121                                           PAL3  TC551001   |
|                                                                             |
|--|                                   |-------------------------| TC551001   |
   |                                   |-------------------------|            |
|--|       |------|                                                           |
|          |SIE150|       61256                                               |
|          |      |                                                           |
|          |------|       61256               |---------|     TC551664BJ-15   |
|                                             |SEI400   |                     |
|                                             |SB07-3460|      *              |
|J                                            |         |                     |
|A                                            |         |                     |
|M                                            |---------|     TC551664BJ-15   |
|M         JP071                                                              |
|A                                        PAL1        61256                   |
|                                                              *              |
|                                         PAL2        61256                   |
|                                                                             |
|                          28.63636MHz |-------------------------|            |
|                                      |-------------------------|            |
|--|                                                                          |
   |      BATT_3V              |---------|        |-------|        PAL4       |
|--|                           |SEI600   |        |AM386DX| 50MHz             |
|                              |SB08-1513|        |       |                   |
|            32.768kHz         |         |        |-------|                   |
|           DS2404S            |         |                                    |
|                              |---------|                                    |
|                                                                             |
|-----------------------------------------------------------------------------|
Notes:
     AM386DX   - Advanced Micro Devices AM386DX/DXL-25 (QFP132), running at 25.000MHz [50/2]
                   - Replaced with Intel NG80386DX25 on Revision 1 PCB
     Z80       - Zilog Z84C0008PEC (DIP40), running at 7.15909MHz (28.63636/4)
     YMF271-F  - Yamaha YMF271-F running at 16.9344MHz
     E28F008SA - Intel E28F008SA 8MBit FlashROM (TSOP40)
                   - Replaced with Sharp LH28F008 on Revision 1 PCB
     XC7336(1) - Xilinx XC7336 CPLD (PLCC44, stamped 'MCTL02')
     XC7336(2) - Xilinx XC7336 CPLD (PLCC44, stamped 'MCTL03')
     TC551001  - Toshiba TC551001 128k x8 SRAM (SOP32)
                   - Replaced with Toshiba TC518128 on Revision 1 PCB
     61256     - 32k x8 SRAM (DIP28)
                   - Replaced with Sony CXK5863BP-30 on Revision 1 PCB
     TC551664  - Toshiba TC551664BJ-15 64k x16 SRAM (SOJ44)
     LH5496D-50- Sharp LH5496D-50 Asynchronous FIFO (DIP28)
     DS2404S   - Dallas DS2404S EconoRAM Time Chip (SOIC16)
     PAL1      - ICT PEEL18CV8 (DIP20, stamped 'SXX005-5')
     PAL2      - ICT PEEL18CV8 (DIP20, stamped 'SXX011B')
     PAL3      - Lattice GAL16V8D (DIP20)
                   - Stamped 'MCTL01' on Revision 1 PCB
                   - no Seibu markings on Revision 2 PCB
     PAL4      - Advanced Micro Devices PALCE20V8H (no Seibu markings, DIP24)
                   - Replaced with ICT PEEL18CV8 (DIP20, stamped 'SXX010B') on Revision 1 PCB
     3V_BATT   - 3 Volt coin battery CR2032 for use with DS2404S
     JRC4560   - Japan Radio Co. JRC4560 Op Amp IC (DIP8)
                   - All 3 IC's replaced with a custom ceramic SIL module stamped 'HB-46A1' on Revision 1 PCB
     YAC513-M  - Yamaha YAC513-M DAC (SOIC16)
     JP072     - Jumper used when swapping game board cartridges
     JP071     - Slide Switch to flip screen
     JP121     - Jumper to set sound output to mono or stereo
     CN121     - Output connector for left/right speakers
     *         - Unpopulated position for Toshiba TC551664BJ-15 64k x16 SRAM


(C)1996 SXX2D V2.1
|----------------------------------------------------------|
|HA13118   CN121                                      Z80  |
|   HA13118                     |-------|  TC518128        |
|          4560  4560           |SEI155 |                  |
|                               |SB09   |  TC518128   PAL3 |
|   VOL_L  4650  YAC513-W       |-------|                  |
|--|    VOL_R    JP121      |-------------------------|    |
   |                        |-------------------------|    |
|--|                 |--------|                            |
|         16.9344MHz |YMF271-F|  61256  |---------|  61256 |
|                    |        |         |SEI400   |        |
|       |------|     |        |  61256  |SB07-3460|  61256 |
|J      |SIE150|     |--------|         |         |        |
|A      |      |                        |         |        |
|M      |------|                        |---------|        |
|M               LH28F008                    TC551664BJ-15 |
|A               LH28F008     PAL2   PAL1    TC551664BJ-15 |
|                           |-------------------------|    |
|                           |-------------------------|    |
|--|             28.63636MHz                               |
   |       BATT_3V           |---------|  |-------|  PAL4  |
|--|JP051                    |SEI600   |  |AM386DX|        |
|                            |SB08-1513|  |       | 50MHz  |
|                32.768kHz   |         |  |-------|        |
|EXCN1           DS2404S     |         |                   |
|EXCN2                       |---------|                   |
|----------------------------------------------------------|
Notes:
      This is a smaller (and perhaps cheaper) version of the SXX2C hardware.
      AM386DX   - Advanced Micro Devices AM386DX/DXL-25 (QFP132), running at 25.000MHz [50/2]
      Z80       - Zilog Z84C0008PEC (DIP40), running at 7.15909MHz (28.63636/4)
      YMF271-F  - Yamaha YMF271-F running at 16.9344MHz
      LH28F008  - Sharp LH28F008SAT-85 8MBit FlashROM (TSOP40)
      TC518128  - Toshiba TC528128 128k x8 SRAM (SOP32)
      61256     - SBT SB61L256AS-12 32k x8 SRAM (SOJ28)
      TC551664  - Toshiba TC551664BJ-15 64k x16 SRAM (SOJ44)
      DS2404S   - Dallas DS2404S EconoRAM Time Chip (SOIC16)
      PAL1      - AMD PALCE16V8 (SOIC20, stamped 'SXX2002')
      PAL2      - AMD PALCE16V8 (SOIC20, no markings)
      PAL3      - AMD PALCE16V8 (SOIC20, no markings)
      PAL4      - Lattice GAL20V8B (DIP24, stamped 'SXX2D01')
      3V_BATT   - 3 Volt coin battery CR2032 for use with DS2404S
      4560      - Japan Radio Co. JRC4560 Op Amp IC (DIP8)
      YAC513-W  - Yamaha YAC513-M DAC (SOIC16)
      HA13118   - Audio Power AMP IC (ZIP15)
      JP121     - Jumper used when swapping game board cartridges
      JP051     - Slide Switch to flip screen
      CN121     - Output connector for left/right speakers
      EXCN1/2   - Connectors for player 3 & 4 controls


ROM Board Layouts
-----------------

SXX2C ROM SUB
-------------------------------------
|     *                       *     |
|+BG2-P.U049    5.U048  BG-1D.U0415 |
|                                   |
|      *                       *    |
| BG-1P.U0410  6.U0413  +BG2-D.U0416|
|                                   |
|    |-------------------------|    |
|    |-------------------------|    |
|                                   |
| 5816                              |
|         |------------|            |
|         |            |     *      |
|         |            |OBJ-3.U0323 |
| 5816    |  SEI252    |            |
|         | SB05-106   |            |
|         |            |            |
|         |            |            |
| 5816    |------------|            |
|                                   |
|                *           *      |
|           OBJ-2.U0324 OBJ-1.U0322 |
|                                   |
|                                   |
|    |-------------------------|    |
|    |-------------------------|    |
|                                   |
|                                   |
| 3.U0210     2.U0212       1.U0211 |
|                                   |
|                *                  |
| 4.U029    PCM-1.U0215     7.U0216 |
|                                   |
-------------------------------------
Notes:
*     : These ROMs are surface mounted
+     : These ROMs not populated on some games
5816  : SONY CXD5816SP-12L SRAM (x3)
SEI252: SEIBU custom stamped 'SEI252 SB05-106' (QFP208)

This board is used by...

Battle Balls/SenKyu      (All Mask ROMs stamped 'F-BALLS')
                         Filename   ROM Type
                         ----------------------------------
                         OBJ-1.322  23C3210 (SOP44 MaskROM)
                         OBJ-2.324  23C3210 (SOP44 MaskROM)
                         OBJ-3.323  23C3210 (SOP44 MaskROM)
                         BG-1D.415  23C1610 (SOP44 MaskROM)
                         BG-1P.410  538100  (SOP32 MaskROM)
                         PCM-1.215  538100  (SOP32 MaskROM)
                         1.211      27C2001 (DIP32 EPROM)
                         2.212      27C020  (DIP32 EPROM)
                         3.210      27C020  (DIP32 EPROM)
                         4.029      27C2001 (DIP32 EPROM)
                         5.048      27C512  (DIP28 EPROM)
                         6.413      27C1024 (DIP40 EPROM)
                         7.216      27C040  (DIP32 EPROM)

E-Jan High School        (All Mask ROMs stamped 'EJAN')
                         Filename     ROM Type
                         ------------------------------------
                         OBJ-1.U0322  23C3210 (SOP44 MaskROM)
                         OBJ-2.U0324  23C3210 (SOP44 MaskROM)
                         OBJ-3.U0323  23C3210 (SOP44 MaskROM)
                         BG-1D.U0415  23C1610 (SOP44 MaskROM)
                         BG-2D.U0416  538000  (SOP32 MaskROM)
                         BG-1P.U0410  538000  (SOP32 MaskROM)
                         BG-2P.U049   534000  (SOP32 MaskROM)
                         PCM-1.U0215  538000  (SOP32 MaskROM)
                         1.U0211      27C2001 (DIP32 EPROM)
                         2.U0212      27C020  (DIP32 EPROM)
                         3.U0210      27C020  (DIP32 EPROM)
                         4.U029       27C2001 (DIP32 EPROM)
                         5.U048       27C512  (DIP28 EPROM)
                         6.U0413      27C1024 (DIP40 EPROM)
                         7.U0216      27C040  (DIP32 EPROM)

Viper Phase 1            (All Mask ROMs stamped 'VIPER')
(Old and New Versions)   Filename     ROM Type
                         ------------------------------------
                         OBJ-1.U0322  23C3210 (SOP44 MaskROM)
                         OBJ-2.U0324  23C3210 (SOP44 MaskROM)
                         OBJ-3.U0323  23C3210 (SOP44 MaskROM)
                         BG-11.U0415  23C1610 (SOP44 MaskROM)
                         BG21.U0416   538000  (SOP32 MaskROM)
                         BG-12.U0410  538000  (SOP32 MaskROM)
                         BG22.U049    534000  (SOP32 MaskROM)
                         PCM.U0215    538000  (SOP32 MaskROM)
                         1.U0211      27C040  (DIP32 EPROM)
                         2.U0212      27C040  (DIP32 EPROM)
                         3.U0210      27C040  (DIP32 EPROM)
                         4.U029       27C040  (DIP32 EPROM)
                         5.U048       27C512  (DIP28 EPROM)
                         6.U0413      27C1024 (DIP40 EPROM)


SXX2C ROM SUB2
-------------------------------------
|    *                        *     |
|BG2-P.U049    7.U048   BG1-D.U0415 |
|                                   |
|    *     5.U0423  6.U0424     *   |
|BG1-P.U0410            BG2-D.U0416 |
|                                   |
|    |-------------------------|    |
|    |-------------------------|    |
|                                   |
| 6116                              |
|         |------------|            |
|         |            |     *      |
|         |            |OBJ-3.U0323 |
| 6116    |  SEI252    |            |
|         | SB05-106   |            |
|         |            |            |
|         |            |            |
| 6116    |------------|            |
|                                   |
|                *           *      |
|           OBJ-2.U0324 OBJ-1.U0322 |
|                                   |
|                                   |
|    |-------------------------|    |
|    |-------------------------|    |
|                                   |
|                                   |
| 3.U0210     2.U0212       1.U0211 |
|                                   |
|                *                  |
| 4.U029     PCM.U0217      8.U0216 |
|                                   |
-------------------------------------
Notes:
*     : These ROMs are surface mounted
6116  : HM6116LK-70 SRAM (x3)
SEI252: SEIBU custom stamped 'SEI252 SB05-106' (QFP208)

This board is used by...
Raiden Fighters       (All Mask ROMs stamped 'GUN DOGS')
                      Filename     ROM Type
                      ----------------------------------
                      OBJ-1.U0322  23C3210 (SOP44 MaskROM)
                      OBJ-2.U0324  23C3210 (SOP44 MaskROM)
                      OBJ-3.U0323  23C3210 (SOP44 MaskROM)
                      BG1-D.U0415  23C1610 (SOP44 MaskROM)
                      BG2-D.U0416  23C1610 (SOP44 MaskROM)
                      BG1-P.U0410  538100  (SOP32 MaskROM)
                      BG2-P.U0049  538100  (SOP32 MaskROM)
                      PCM.U0217    538100  (SOP32 MaskROM)
                      1.U0211      27C040  (DIP32 EPROM)
                      2.U0212      27C040  (DIP32 EPROM)
                      3.U0210      27C040  (DIP32 EPROM)
                      4.U0029      27C040  (DIP32 EPROM)
                      5.U0423      27C512  (DIP28 EPROM)
                      6.U0424      27C512  (DIP28 EPROM)
                      7.U048       27C512  (DIP28 EPROM)
                      8.U0216      27C040  (DIP32 EPROM)


SXX2C ROM SUB4 (C)1996
-------------------------------------
|    *   BG1-P.U0410  *BG2-P.U049   |
|BG1-D.U0415   *          *FIX.U0425|
|                                   |
|    *        7.U048                |
|BG2-D.U0424                        |
|                                   |
|    |-------------------------|    |
|    |-------------------------|    |
|                                   |
| 6216                              |
|         |------------|            |
|         |            |     *      |
| 6216    |            |OBJ-3.U0323 |
|         |  SEI252    |            |
|         | SB05-106   |     *      |
|         |            |OBJ-2.U0324 |
|         |            |            |
|         |------------|     *      |
| 6216                  OBJ-1.U0322 |
|                                   |
|                                   |
|                                   |
|                                   |
|    |-------------------------|    |
|    |-------------------------|    |
|                                   |
|                              *    |
| 1.U0211        *        PCM2.U0217|
|            PRG34.U0219            |
|                                   |
| PRG2.U021                         |
|                                   |
-------------------------------------
Notes:
*     : These ROMs are surface mounted
6216  : BR6216C-10LL SRAM (x3)
SEI252: SEIBU custom stamped 'SEI252 SB05-106' (QFP208)

This board is used by...
Raiden Fighters (Asia)
                      Filename     ROM Type
                      ----------------------------------
                      OBJ-1.U0322  23C3210 (SOP44 MaskROM, stamped 'GUN DOGS')
                      OBJ-2.U0324  23C3210 (SOP44 MaskROM, stamped 'GUN DOGS')
                      OBJ-3.U0323  23C3210 (SOP44 MaskROM, stamped 'GUN DOGS')
                      BG1-D.U0415  23C1610 (SOP44 MaskROM, stamped 'GUN DOGS')
                      BG2-D.U0424  23C1610 (SOP44 MaskROM, stamped 'GUN DOGS')
                      BG1-P.U0410  538100  (SOP32 MaskROM, stamped 'GUN DOGS')
                      BG2-P.U0049  538100  (SOP32 MaskROM, stamped 'GUN DOGS')
                      FIX.U0425    LH531024(SOP40 MaskROM, stamped 'RAIDEN-F')
                      PCM2.U0217   23C1610 (SOP44 MaskROM, stamped 'RAIDEN-F')
                      PRG34.U0219  23C1610 (SOP44 MaskROM, stamped 'RAIDEN-F')
                      PRG2.U0212   534000  (DIP32 MaskROM, stamped 'RAIDEN-F')
                      1.U0211      27C040  (DIP32 EPROM)
                      7.U048       27C512  (DIP28 EPROM)


SXX2C ROM SUB8 (C)1997
-------------------------------------
|                                   |
| 7.U0514     6.U0518      5.U0524  |
|                                   |
|        *BG-2D.U0536    BG-1D.U0535|
|BG-2P.U0538    BG-1P.U0537     *   |
|    *              *               |
|    |-------------------------|    |
|    |-------------------------|    |
|                                   |
|    *                        PAL1  |
|OBJ-1.U0429 |------------|         |
|    *       |            |         |
|OBJ-1B.U0430|            | N341256 |
|    *       |   RISE10   |         |
|OBJ-2.U0431 |            | N341256 |
|    *       |            |         |
|OBJ-2B.U0432|            |         |
|    *       |------------|         |
|OBJ-3.U0434                  PAL2  |
|    *                              |
|OBJ-3B.U0433                       |
|                                   |
|PAL3                               |
|    |-------------------------|    |
|    |-------------------------|    |
|                                   |
|                              *    |
|                          PCM.U0217|
|1.U0211   3.U0221   8.U0222        |
|                                   |
|     2.U0212   4.U0220             |
|                                   |
-------------------------------------
Notes:
*      : These ROMs are surface mounted
N341256: NKK N341256SJ-15 32k x8 SRAM (x2)
RISE10 : SEIBU custom stamped 'RISE10' (QFP240)
PAL1   : PALCE 16V8 stamped 'RM83'
PAL2   : PALCE 16V8 stamped 'RM81'
PAL3   : PALCE 16V8 stamped 'RM82'

This board is used by...

Raiden Fighters 2     (All Mask ROMs stamped 'RAIDEN-F2')
                       Filename     ROM Type
                       --------------------------------------
                       BG-1D.U0535  MX23C3210 (SOP44 MaskROM)
                       BG-2D.U0536  MX23C3210 (SOP44 MaskROM)
                       BG-1P.U0537  MX23C1610 (SOP44 MaskROM)
                       BG-2P.U0538  MX23C1610 (SOP44 MaskROM)
                       OBJ1.U0429   MX23C3210 (TSOP48 MaskROM)
                       OBJ2.U0431   MX23C3210 (TSOP48 MaskROM)
                       OBJ3.U0434   MX23C3210 (TSOP48 MaskROM)
                       OBJ1B.U0430  MX23C1610 (TSOP48 MaskROM)
                       OBJ2B.U0432  MX23C1610 (TSOP48 MaskROM)
                       OBJ3B.U0433  MX23C1610 (TSOP48 MaskROM)
                       PCM.U0217    MX23C1610 (SOP44 MaskROM)
                       1.U0211      27C040    (DIP32 EPROM)
                       2.U0212      27C040    (DIP32 EPROM)
                       3.U0221      27C040    (DIP32 EPROM)
                       4.U0220      27C040    (DIP32 EPROM)
                       5.U0524      27C512    (DIP28 EPROM)
                       6.U0518      27C512    (DIP28 EPROM)
                       7.U0514      27C512    (DIP28 EPROM)
                       8.U0222      27C040    (DIP32 EPROM)


SXX2C ROM SUB10 (C)1998
-------------------------------------
|                                   |
|FIXP.U0514  FIX1.U0518  FIX0.U0524 |
|                                   |
|        *BG-2D.U0545    BG-1D.U0543|
|BG-2P.U0546    BG-1P.U0544     *   |
|    *              *               |
|    |-------------------------|    |
|    |-------------------------|    |
|                                   |
|    *                              |
|OBJ-1.U0442 |------------|         |
|            |            |         |
|            |            |   61256 |
|    *       |   RISE11   |         |
|OBJ-2.U0443 |            |   61256 |
|            |            |         |
|            |            |         |
|    *       |------------|         |
|OBJ-3.U0444                        |
|                             PAL1  |
|                             PAL2  |
|                                   |
|                                   |
|    |-------------------------|    |
|    |-------------------------|    |
|                                   |
|                             *     |
|PRG2.U0221  PRG0.U0211  PCM-D.U0227|
|                                   |
|                                   |
|PRG3.U0220  PRG1.U0212 SOUND1.U0222|
|                                   |
-------------------------------------
Notes:
*     : These ROMs are surface mounted
61256 : 32k x8 SRAM (x2)
RISE11: SEIBU custom stamped 'RISE11' (QFP240)
PAL1  : PALCE 16V8 stamped 'SPI ROM 10-2'
PAL2  : PALCE 16V8 stamped 'SPI ROM 10-1'

This board is used by...

Raiden Fighters Jet    (All Mask ROMs stamped 'RAIDEN-FJET')
                       Filename     ROM Type
                       --------------------------------------
                       FIXP.U0514   27C512    (DIP28 EPROM)
                       FIX1.U0518   27C512    (DIP28 EPROM)
                       FIX0.U0524   27C512    (DIP28 EPROM)
                       BG-2D.U0545  MX23C1610 (SOP44 MaskROM)
                       BG-1D.U0543  MX23C3210 (SOP44 MaskROM)
                       BG-1P.U0544  MX23C1610 (SOP44 MaskROM)
                       BG-2P.U0546  MX23C8000 (SOP32 MaskROM)
                       OBJ-1.U0442  MX23C6410 (SOP44 MaskROM)
                       OBJ-2.U0443  MX23C6410 (SOP44 MaskROM)
                       OBJ-3.U0444  MX23C6410 (SOP44 MaskROM)
                       PRG0.U0211   MX27C4000 (DIP32 EPROM)
                       PRG1.U0212   MX27C4000 (DIP32 EPROM)
                       PRG2.U0221   MX27C4000 (DIP32 EPROM)
                       PRG3.U0220   MX27C4000 (DIP32 EPROM)
                       PCM-D.U0227  MX23C1610 (SOP44 MaskROM)
                       SOUND1.U0222 MX27C4000 (DIP32 EPROM)


There were some single PCBs made that run just one game. These are shown below.


SXX2F V1.2
|-----------------------------------------------------------------------------|
|HA13118                |--------|           PCM   SOUND1                     |
|                       |YMF271-F|                 ZPRG     OBJ1  OBJ2  OBJ3  |
|                       |        |16.9344MHz                OBJ1B OBJ2B OBJ3B |
|  VOL                  |        |                                            |
|          YAC516-M     |--------|                                            |
|                                    Z84C0006PCS  AE8256AJ-12                 |
|--|            LH5496D-50                                                    |
   |                   PAL                                                    |
|--|       |------|    PAL   PAL   PAL   PAL                                  |
|          |SIE150|                                            |---------|    |
|          |      |                                            |RISE10   |  X |
|          |------| AE8256AJ-12                                |9741 GBZ1|  0 | X051 = 28.63636MHz OSC
|J                               |---------|     AE8256AJ-12   |         |  5 |
|A                               |SEI400   |     AE8256AJ-12   |         |  1 |
|M          ST93C46 AE8256AJ-12  |SB07-3460|                   |---------|    |
|M                               |         |                                  |
|A                               |         |                                  |
|    JP031          AE8256AJ-12  |---------|                                  |
|                                                  TC551664J-20     PRG0.U0259|
|                                                  TC551664J-20               |
|--|                AE8256AJ-12                                     PRG1.U0258|
   |                                 |---------|     |--------|               |
|--|                                 |SEI600   |     | Intel  |     PRG2.U0265|
|                                    |SB08-1513|     | i386DX |               |
|    FIX0   BG-1P.U0537  BG-1D.U0535 |         |     |--------|     PRG3.U0264|
|    FIX1                            |         |                              |
|    FIXP   BG-2P.U0545  BG-2D.U0536 |---------|          50MHz      PAL      |
|                                                                             |
|-----------------------------------------------------------------------------|
Notes:
      i386DX    - Intel i386DX (QFP132), running at 25.000MHz [50/2]
      Z80       - Zilog Z84C0006PCS (DIP40) - Uknown clock
      YMF271-F  - Yamaha YMF271-F running at 16.9344MHz
      TC551664  - Toshiba TC551664J-20 64k x16 SRAM (SOJ44)
      YAC516-M  - Yamaha YAC516-M DAC (SOP28)
      JP031     - Slide Switch to flip screen (unpopulated)
      ST93C46   - EEPROM (SOIC8)

Name         Size    CRC32
-------------------------------
fix0.bin    65536    0x6fdf4cf6
fix1.bin    65536    0x69b7899b
fixp.bin    65536    0x99a5fece
prg0.bin   524288    0xff3eeec1
prg1.bin   524288    0xe2cf77d6
prg2.bin   524288    0xcae87e1f
prg3.bin   524288    0x83f4fb5f
sound1.bin 524288    0x20384b0e
zprg.bin   131072    0xcc543c4f


SXX2G
|-----------------------------------------------------------------------------|
|HA13118                |--------|          PCM-D  RFJ-04                     |
|                       |YMF271-F|                 RFJ-05   OBJ-3 OBJ-2 OBJ-1 |
|                       |        |16.3840MHz                U075  U074  U073  |
|  VOL                  |        |                                            |
|          YAC516-M     |--------|                                            |
|                                    Z84C0004PCS  D43256BGU-70L               |
|--|            CY7C421-65PC                                                  |
   |                   PAL                                                    |
|--|       |------|    PAL   PAL   PAL   PAL                                  |
|          |SIE150|              4.9152MHz                     |---------|    |
|          |      |                                            |RISE11   |    |
|          |------|   61256                                    |9823 GAX1|    |
|J                               |---------|    D43256BGU-70L  |         |    |
|A                               |SEI400   |    D43256BGU-70L  |         |    |
|M          ST93C46   61256      |SB07-3460|                   |---------|    |
|M                               |         |                                  |
|A                               |         |                    28.63636MHz   |
|    JP031            61256      |---------|                                  |
|                                                  TC551664BJ-20  RFJ-06.U0259|
|                                                  TC551664BJ-20              |
|--|                  61256                                       RFJ-07.U0258|
   |                                 |---------|     |-------|                |
|--|                                 |SEI600   |     |AM386DX|    RFJ-08.U0265|
|                                    |SB08-1513|     |  40   |                |
|   RFJ-01  BG-1P.U0537  BG-1D.U0535 |         |     |-------|    RFJ-09.U0264|
|   RFJ-02                           |         |                              |
|   RFJ-03  BG-2P.U0545  BG-2D.U0536 |---------|       28.63636MHz            |
|                                                                    SW1  PAL |
|-----------------------------------------------------------------------------|
Notes:
      AM386DX   - Advanced Micro Devices AM386DX/DX-40 running at 28.63636MHz (QFP132)
      Z80       - Zilog Z84C0004PCS running at 4.9152MHz (DIP40)
      YMF271-F  - Yamaha YMF271-F running at 16.3840MHz
      TC551664  - Toshiba TC551664BJ-15 64k x16 SRAM (SOJ44)
      YAC516-M  - Yamaha YAC516-M DAC (SOP28)
      JP031     - Slide Switch to flip screen
      ST93C46   - EEPROM (SOIC8)

      ROMs
      ----
      RFJ-01 - FIX0   27C512
      RFJ-02 - FIX1   27C512
      RFJ-03 - FIXP   27C512
      RFJ-04 - SOUND1 27C040
      RFJ-05 - ZPRG   27C020
      RFJ-06 - PRG0   27C040
      RFJ-07 - PRG1   27C040
      RFJ-08 - PRG2   27C040
      RFJ-09 - PRG3   27C040
      Other ROMs not listed are surface mounted SOP44 except BG-2P
      which is SOP32
      All surface mounted ROMs are stamped 'RAIDEN-FJET' and match
      the same named ROMs from the SPI version.


E-Jan Sakurasou
Seibu Kaihatsu Inc. 1999

PCB Layout
----------

SYS386F V2.0
|-----------------------------------------------------|
|TA7252 4560D                                         |
| VOL   YAC516        SOUND1                          |
|                              |------------|         |
|  16.384MHz YMZ280B  SOUND2   |   RISE11   |  CHR3   |
|                              |            |         |
|                 SB61L256     |            |  CHR4   |
|                 SB61L256     |            |         |
|M                SB61L256     |------------|  CHR2   |
|A                SB61L256                28.63636MHz |
|H                                             CHR1   |
|J                       XC9536                       |
|O     PAL16(1)                                       |
|N           PAL16(2)  GAL20V8        W26010A         |
|G                                                    |
|    93C46   |---------|                              |
|            | SEI600  |   PRG3                       |
|  50MHz     |SB08-1513|              W26010A         |
|            |         |   PRG2                       |
|            |---------|                              |
|                          PRG1                       |
|                i386DX                               |
|                          PRG0                       |
|-----------------------------------------------------|
Notes:
      i386DX   - Intel i386DX-25MHz (QFP132), clock 25.000MHz [50/2]
      YMZ280B  - Yamaha YMZ280B-F, clock input 16.384MHz
      YAC516-M - Yamaha YAC516-M Delta Sigma Modulation D/A Converter with 8 Times Over Sampling Filter (SOP28)
      4560D    - JRC 4560D OP Amp (DIP8)
      TA7252   - Toshiba TA7252AP Power Amp IC (SIL7)
      GAL20V8  - Lattice GAL20V8B stamped 'S386F1' at location U0170
      PAL16(1) - AMD PAL16V8H stamped 'S386F2' at location U0069
      PAL16(2) - AMD PAL16V8H stamped 'S386F4' at location U0341
      XC9536   - Xilinx XC9536 CPLD (PLCC44) at location U0339
      W26010A  - Winbond W26010AJ-15 64kx16 SRAM (SOJ44)
      SB61L256 - Silicon-Based Technology Corporation SB61L256AS-12 32kx8 SRAM (SOJ28)
      Custom   - SEI600 SB08-1513 (QFP208)
                 RISE11 (QFP240)
      ROMs     - PRG0.U0211 \
                 PRG1.U0212  |
                 PRG2.U0221  | 27C020 EPROM
                 PRG3.U0220 /
                 CHR1.U0442 \
                 CHR2.U0443  |
                 CHR3.U0444  | 32MBit SOP44 mask ROM
                 CHR4.U0445 /
                 SOUND1.U083 \
                 SOUND2.U084 / 64MBit SOP44 mask ROM

     Measurements
     ------------
     OSC1 - 50.0003MHz
     OSC2 - 28.6368MHz
     OSC3 - 16.3837MHz
     VSync - 57.5943Hz
     HSync - 15.6656kHz

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/i386/i386.h"
#include "machine/ds2404.h"
#include "machine/eeprom.h"
#include "machine/intelfsh.h"
#include "sound/okim6295.h"
#include "sound/ymf271.h"
#include "sound/ymz280b.h"
#include "includes/seibuspi.h"

UINT32 *spimainram;

static UINT8 *z80_rom;

/********************************************************************/
static int z80_prg_fifo_pos = 0;
static int z80_lastbank;

#define FIFO_SIZE 512
static int fifoin_rpos, fifoin_wpos;
static UINT8 fifoin_data[FIFO_SIZE];
static int fifoin_read_request = 0;

static int fifoout_rpos, fifoout_wpos;
static UINT8 fifoout_data[FIFO_SIZE];
static int fifoout_read_request = 0;

static UINT8 sb_coin_latch = 0;

static UINT8 ejsakura_input_port = 0;

static UINT8 z80_fifoout_pop(const address_space *space)
{
	UINT8 r;
	if (fifoout_wpos == fifoout_rpos)
	{
		logerror("Sound FIFOOUT underflow at %08X\n", cpu_get_pc(space->cpu));
	}
	r = fifoout_data[fifoout_rpos++];
	if(fifoout_rpos == FIFO_SIZE)
	{
		fifoout_rpos = 0;
	}

	if (fifoout_wpos == fifoout_rpos)
	{
		fifoout_read_request = 0;
	}

	return r;
}

static void z80_fifoout_push(const address_space *space, UINT8 data)
{
	fifoout_data[fifoout_wpos++] = data;
	if (fifoout_wpos == FIFO_SIZE)
	{
		fifoout_wpos = 0;
	}
	if(fifoout_wpos == fifoout_rpos)
	{
		fatalerror("Sound FIFOOUT overflow at %08X", cpu_get_pc(space->cpu));
	}

	fifoout_read_request = 1;
}

static UINT8 z80_fifoin_pop(const address_space *space)
{
	UINT8 r;
	if (fifoin_wpos == fifoin_rpos)
	{
		fatalerror("Sound FIFOIN underflow at %08X", cpu_get_pc(space->cpu));
	}
	r = fifoin_data[fifoin_rpos++];
	if(fifoin_rpos == FIFO_SIZE)
	{
		fifoin_rpos = 0;
	}

	if (fifoin_wpos == fifoin_rpos)
	{
		fifoin_read_request = 0;
	}

	return r;
}

static void z80_fifoin_push(const address_space *space, UINT8 data)
{
	fifoin_data[fifoin_wpos++] = data;
	if(fifoin_wpos == FIFO_SIZE)
	{
		fifoin_wpos = 0;
	}
	if(fifoin_wpos == fifoin_rpos)
	{
		fatalerror("Sound FIFOIN overflow at %08X", cpu_get_pc(space->cpu));
	}

	fifoin_read_request = 1;
}

static READ32_HANDLER( sb_coin_r )
{
	UINT8 r = sb_coin_latch;

	sb_coin_latch = 0;
	return r;
}

static WRITE8_HANDLER( sb_coin_w )
{
	if (data)
		sb_coin_latch = 0xa0 | data;
	else
		sb_coin_latch = 0;
}

static READ32_HANDLER( sound_fifo_r )
{
	UINT8 r = z80_fifoout_pop(space);

	return r;
}

static WRITE32_HANDLER( sound_fifo_w )
{
	if( ACCESSING_BITS_0_7 ) {
		z80_fifoin_push(space, data & 0xff);
	}
}

static READ32_HANDLER( sound_fifo_status_r )
{
	UINT32 r = 0;
	if (fifoout_read_request)
	{
		r |= 2;
	}
	return r | 1;
}

static READ32_HANDLER( spi_int_r )
{
	cputag_set_input_line(space->machine, "maincpu", 0,CLEAR_LINE );
	return 0xffffffff;
}

static READ32_HANDLER( spi_unknown_r )
{
	return 0xffffffff;
}

static WRITE32_DEVICE_HANDLER( eeprom_w )
{
	okim6295_device *oki2 = device->machine->device<okim6295_device>("oki2");

	// tile banks
	if( ACCESSING_BITS_16_23 ) {
		rf2_set_layer_banks(data >> 16);
		eeprom_write_bit(device, (data & 0x800000) ? 1 : 0);
		eeprom_set_clock_line(device, (data & 0x400000) ? ASSERT_LINE : CLEAR_LINE);
		eeprom_set_cs_line(device, (data & 0x200000) ? CLEAR_LINE : ASSERT_LINE);
	}

	// oki banking
	if (oki2 != NULL)
		oki2->set_bank_base((data & 0x4000000) ? 0x40000 : 0);
}

static WRITE32_HANDLER( z80_prg_fifo_w )
{
	if( ACCESSING_BITS_0_7 ) {
		if (z80_prg_fifo_pos<0x40000) z80_rom[z80_prg_fifo_pos] = data & 0xff;
		z80_prg_fifo_pos++;
	}
}

static WRITE32_HANDLER( z80_enable_w )
{
	// tile banks
	if( ACCESSING_BITS_16_23 ) {
		rf2_set_layer_banks(data >> 16);
	}

logerror("z80 data = %08x mask = %08x\n",data,mem_mask);
	if( ACCESSING_BITS_0_7 ) {
		if( data & 0x1 ) {
			z80_prg_fifo_pos = 0;
			cputag_set_input_line(space->machine, "soundcpu", INPUT_LINE_RESET, CLEAR_LINE );
		} else {
			cputag_set_input_line(space->machine, "soundcpu", INPUT_LINE_RESET, ASSERT_LINE );
		}
	}
}

static READ32_HANDLER( spi_controls1_r )
{
	if( ACCESSING_BITS_0_7 )
	{
		return input_port_read(space->machine, "INPUTS");
	}
	return 0xffffffff;
}

static READ32_HANDLER( spi_controls2_r )
{
	if( ACCESSING_BITS_0_7 )
	{
		return input_port_read(space->machine, "SYSTEM");
	}
	return 0xffffffff;
}

static CUSTOM_INPUT( ejsakura_keyboard_r )
{
	switch(ejsakura_input_port)
	{
		case 0x01:
			return input_port_read(field->port->machine, "INPUT01");
		case 0x02:
			return input_port_read(field->port->machine, "INPUT02");
		case 0x04:
			return input_port_read(field->port->machine, "INPUT04");
		case 0x08:
			return input_port_read(field->port->machine, "INPUT08");
		case 0x10:
			return input_port_read(field->port->machine, "INPUT10");
		default:
			return input_port_read(field->port->machine, "SYSTEM");
	}
	return 0xffffffff;
}
/********************************************************************/

static READ8_HANDLER( z80_soundfifo_r )
{
	UINT8 r = z80_fifoin_pop(space);

	return r;
}

static WRITE8_HANDLER( z80_soundfifo_w )
{
	z80_fifoout_push(space, data);
}

static READ8_HANDLER( z80_soundfifo_status_r )
{
	UINT8 r = 0;
	if (fifoin_read_request)
	{
		r |= 2;
	}
	return r | 1;
}

static WRITE8_HANDLER( z80_bank_w )
{
	if ((data & 7) != z80_lastbank)
	{
		z80_lastbank = (data & 7);
		memory_set_bankptr(space->machine, "bank4", z80_rom + (0x8000 * z80_lastbank));
	}
}

static READ8_HANDLER( z80_jp1_r )
{
	return input_port_read(space->machine, "JP1");
}

static READ8_HANDLER( z80_coin_r )
{
	return input_port_read(space->machine, "COIN");
}

static READ32_HANDLER( soundrom_r )
{
	UINT8 *sound = (UINT8*)memory_region(space->machine, "user2");
	UINT16 *sound16 = (UINT16*)memory_region(space->machine, "user2");

	if (mem_mask == 0x000000ff)
	{
		return sound[offset];
	}
	else if (mem_mask == 0xffffffff)
	{
		if (offset < 0x100000)
		{
			return sound16[offset];
		}
		else if (offset < 0x200000)
		{
			return sound16[0x80000 + (offset & 0x7ffff)];
		}
		else
		{
			return sound[offset];
		}
	}


	fatalerror("soundrom_r: %08X, %08X", offset, mem_mask);
}

/********************************************************************/

static ADDRESS_MAP_START( spi_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x00000417) AM_RAM
	AM_RANGE(0x00000418, 0x0000041b) AM_READWRITE(spi_layer_bank_r, spi_layer_bank_w)
	AM_RANGE(0x0000041c, 0x0000041f) AM_READNOP
	AM_RANGE(0x0000041c, 0x0000041f) AM_WRITE(spi_layer_enable_w)
	AM_RANGE(0x00000420, 0x0000042b) AM_RAM AM_BASE(&spi_scrollram)
	AM_RANGE(0x00000480, 0x00000483) AM_WRITE(tilemap_dma_start_w)
	AM_RANGE(0x00000484, 0x00000487) AM_WRITE(palette_dma_start_w)
	AM_RANGE(0x00000490, 0x00000493) AM_WRITE(video_dma_length_w)
	AM_RANGE(0x00000494, 0x00000497) AM_WRITE(video_dma_address_w)
	AM_RANGE(0x0000050c, 0x0000050f) AM_WRITE(sprite_dma_start_w)
	AM_RANGE(0x00000600, 0x00000603) AM_READ(spi_int_r)				/* Clear Interrupt */
	AM_RANGE(0x00000600, 0x00000603) AM_WRITENOP				/* Unknown */
	AM_RANGE(0x00000604, 0x00000607) AM_READ(spi_controls1_r)	/* Player controls */
	AM_RANGE(0x00000608, 0x0000060b) AM_READ(spi_unknown_r)		/* Unknown */
	AM_RANGE(0x0000060c, 0x0000060f) AM_READ(spi_controls2_r)	/* Player controls (start) */
	AM_RANGE(0x00000680, 0x00000683) AM_WRITE(sound_fifo_w)
	AM_RANGE(0x00000684, 0x00000687) AM_READ(sound_fifo_status_r)
	AM_RANGE(0x00000684, 0x00000687) AM_WRITENOP				/* Unknown */
	AM_RANGE(0x000006d0, 0x000006d3) AM_DEVWRITE8("ds2404", ds2404_1w_reset_w, 0x000000ff)
	AM_RANGE(0x000006d4, 0x000006d7) AM_DEVWRITE8("ds2404", ds2404_data_w, 0x000000ff)
	AM_RANGE(0x000006d8, 0x000006db) AM_DEVWRITE8("ds2404", ds2404_clk_w, 0x000000ff)
	AM_RANGE(0x000006dc, 0x000006df) AM_DEVREAD8("ds2404", ds2404_data_r, 0x000000ff)
	AM_RANGE(0x00000800, 0x0003ffff) AM_RAM AM_BASE(&spimainram)
	AM_RANGE(0x00200000, 0x003fffff) AM_ROM AM_SHARE("share2")
	AM_RANGE(0x00a00000, 0x013fffff) AM_READ(soundrom_r)
	AM_RANGE(0xffe00000, 0xffffffff) AM_ROM AM_REGION("user1", 0) AM_SHARE("share2")		/* ROM location in real-mode */
ADDRESS_MAP_END

static ADDRESS_MAP_START( spisound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_RAMBANK("bank5")
	AM_RANGE(0x4002, 0x4002) AM_WRITENOP			/* ack RST 10 */
	AM_RANGE(0x4003, 0x4003) AM_WRITENOP			/* Unknown */
	AM_RANGE(0x4004, 0x4004) AM_WRITE(sb_coin_w)	/* single board systems */
	AM_RANGE(0x4008, 0x4008) AM_READWRITE(z80_soundfifo_r, z80_soundfifo_w)
	AM_RANGE(0x4009, 0x4009) AM_READ(z80_soundfifo_status_r)
	AM_RANGE(0x400a, 0x400a) AM_READ(z80_jp1_r)
	AM_RANGE(0x400b, 0x400b) AM_WRITENOP			/* Unknown */
	AM_RANGE(0x4013, 0x4013) AM_READ(z80_coin_r)
	AM_RANGE(0x401b, 0x401b) AM_WRITE(z80_bank_w)		/* control register: bits 0-2 = bank @ 8000, bit 3 = watchdog? */
	AM_RANGE(0x6000, 0x600f) AM_DEVREADWRITE("ymf", ymf271_r, ymf271_w)
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank4")
ADDRESS_MAP_END

static READ8_DEVICE_HANDLER( flashrom_read )
{
	logerror("Flash Read: %08X\n", offset);
	if( offset < 0x100000 )
	{
		return intelflash_read(0, offset);
	}
	else if( offset < 0x200000 )
	{
		return intelflash_read(1, offset - 0x100000 );
	}
	return 0;
}

static WRITE8_DEVICE_HANDLER( flashrom_write )
{
	logerror("Flash Write: %08X, %02X\n", offset, data);
	if( offset < 0x100000 )
	{
		intelflash_write(0, offset + 1, data);
	}
	else if( offset < 0x200000 )
	{
		intelflash_write(1, offset - 0x100000 + 1, data);
	}
}

static void irqhandler(running_device *device, int state)
{
	if (state)
		cputag_set_input_line_and_vector(device->machine, "soundcpu", 0, ASSERT_LINE, 0xd7);	// IRQ is RST10
	else
		cputag_set_input_line(device->machine, "soundcpu", 0, CLEAR_LINE);
}

static WRITE32_DEVICE_HANDLER(sys386f2_eeprom_w)
{
	eeprom_write_bit(device, (data & 0x80) ? 1 : 0);
	eeprom_set_clock_line(device, (data & 0x40) ? ASSERT_LINE : CLEAR_LINE);
	eeprom_set_cs_line(device, (data & 0x20) ? CLEAR_LINE : ASSERT_LINE);
}

static const ymf271_interface ymf271_config =
{
	DEVCB_HANDLER(flashrom_read),
	DEVCB_HANDLER(flashrom_write),
	irqhandler
};

/********************************************************************/

static ADDRESS_MAP_START( seibu386_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x00000417) AM_RAM
	AM_RANGE(0x00000418, 0x0000041b) AM_READWRITE(spi_layer_bank_r, spi_layer_bank_w)
	AM_RANGE(0x0000041c, 0x0000041f) AM_READNOP
	AM_RANGE(0x0000041c, 0x0000041f) AM_WRITE(spi_layer_enable_w)
	AM_RANGE(0x00000420, 0x0000042b) AM_RAM AM_BASE(&spi_scrollram)
	AM_RANGE(0x00000480, 0x00000483) AM_WRITE(tilemap_dma_start_w)
	AM_RANGE(0x00000484, 0x00000487) AM_WRITE(palette_dma_start_w)
	AM_RANGE(0x00000490, 0x00000493) AM_WRITE(video_dma_length_w)
	AM_RANGE(0x00000494, 0x00000497) AM_WRITE(video_dma_address_w)
	AM_RANGE(0x0000050c, 0x0000050f) AM_WRITE(sprite_dma_start_w)
	AM_RANGE(0x00000600, 0x00000603) AM_READ(spi_int_r)				/* Unknown */
	AM_RANGE(0x00000604, 0x00000607) AM_READ(spi_controls1_r)	/* Player controls */
	AM_RANGE(0x00000608, 0x0000060b) AM_READ(spi_unknown_r)
	AM_RANGE(0x0000060c, 0x0000060f) AM_READ(spi_controls2_r)	/* Player controls (start) */
	AM_RANGE(0x0000068c, 0x0000068f) AM_DEVWRITE("eeprom", eeprom_w)
	AM_RANGE(0x00000800, 0x0003ffff) AM_RAM AM_BASE(&spimainram)
	AM_RANGE(0x00200000, 0x003fffff) AM_ROM AM_SHARE("share2")
	AM_RANGE(0x01200000, 0x01200003) AM_DEVREADWRITE8("oki1", okim6295_r, okim6295_w, 0x000000ff)
	AM_RANGE(0x01200004, 0x01200007) AM_DEVREADWRITE8("oki2", okim6295_r, okim6295_w, 0x000000ff)
	AM_RANGE(0xffe00000, 0xffffffff) AM_ROM AM_REGION("user1", 0) AM_SHARE("share2")		/* ROM location in real-mode */
ADDRESS_MAP_END

static WRITE32_HANDLER(input_select_w)
{
	ejsakura_input_port = data;
}

static ADDRESS_MAP_START( sys386f2_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x0000000f) AM_RAM
	AM_RANGE(0x00000010, 0x00000013) AM_READ(spi_int_r)				/* Unknown */
	AM_RANGE(0x00000090, 0x00000097) AM_RAM /* Unknown */
	AM_RANGE(0x00000400, 0x00000403) AM_READNOP AM_WRITE(input_select_w)
	AM_RANGE(0x00000404, 0x00000407) AM_DEVWRITE("eeprom", sys386f2_eeprom_w)
	AM_RANGE(0x00000408, 0x0000040f) AM_DEVWRITE8("ymz", ymz280b_w, 0x000000ff)
	AM_RANGE(0x00000484, 0x00000487) AM_WRITE(palette_dma_start_w)
	AM_RANGE(0x00000490, 0x00000493) AM_WRITE(video_dma_length_w)
	AM_RANGE(0x00000494, 0x00000497) AM_WRITE(video_dma_address_w)
	AM_RANGE(0x00000500, 0x0000054f) AM_RAM /* Unknown */
	AM_RANGE(0x00000560, 0x00000563) AM_WRITE(sprite_dma_start_w)
	AM_RANGE(0x00000600, 0x00000607) AM_DEVREAD8("ymz", ymz280b_r, 0x000000ff)
	AM_RANGE(0x00000608, 0x0000060b) AM_READ(spi_unknown_r)
	AM_RANGE(0x0000060c, 0x0000060f) AM_READ(spi_controls1_r)	/* Player controls */
	AM_RANGE(0x00000800, 0x0003ffff) AM_RAM AM_BASE(&spimainram)
	AM_RANGE(0x00200000, 0x003fffff) AM_ROM AM_SHARE("share2")
	AM_RANGE(0xffe00000, 0xffffffff) AM_ROM AM_REGION("user1", 0) AM_SHARE("share2")		/* ROM location in real-mode */
ADDRESS_MAP_END


/********************************************************************/

static INPUT_PORTS_START( spi_2button )
	PORT_START("INPUTS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE_NO_TOGGLE( 0x00000004, IP_ACTIVE_LOW) /* Test Button */
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service Coin") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)
	PORT_BIT( 0x000000b0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("JP1")
	PORT_DIPNAME( 0x03, 0x03, "JP1" )
	PORT_DIPSETTING(	0x03, "Update"  )
	PORT_DIPSETTING(	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( spi_3button )
	PORT_INCLUDE( spi_2button )

	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( seibu386_2button )
	PORT_START("INPUTS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE_NO_TOGGLE( 0x00000004, IP_ACTIVE_LOW) /* Test Button */
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service Coin") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("JP1")
	PORT_DIPNAME( 0x03, 0x03, "JP1" )
	PORT_DIPSETTING(	0x03, "Update"  )
	PORT_DIPSETTING(	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_BUTTON4 )
INPUT_PORTS_END

/* E-Jan Highschool has a keyboard with the following keys
   The keys are encoded with 6 bits
   A - 000010 port 0
   B - 010000 port 0
   C - 000010 port 1
   D - 010000 port 1
   E - 000011 port 0
   F - 011000 port 0
   G - 000011 port 1
   H - 011000 port 1
   I - 000100 port 0
   J - 100000 port 0
   K - 000100 port 1
   L - 100000 port 1
   M - 000101 port 0
   N - 101000 port 0
   CHI - 000101 port 1
   PON - 101000 port 1
   KAN - 000110 port 0
   REACH - 110000 port 0
   RON - 000110 port 1
   Start - 000111 port 0
*/

static CUSTOM_INPUT( ejanhs_encode )
{
	static const UINT8 encoding[] = { 0x02, 0x10, 0x03, 0x18, 0x04, 0x20, 0x05, 0x28, 0x06, 0x30, 0x07 };
	input_port_value state = input_port_read(field->port->machine, (const char *)param);
	int bit;

	for (bit = 0; bit < ARRAY_LENGTH(encoding); bit++)
		if (state & (1 << bit))
			return encoding[bit];
	return 0;
}

static INPUT_PORTS_START( spi_ejanhs )
	PORT_START("INPUTS")
	PORT_BIT( 0x0000003f, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(ejanhs_encode, "IN0BITS")
	PORT_BIT( 0x00003f00, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(ejanhs_encode, "IN1BITS")
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_SERVICE_NO_TOGGLE( 0x00000004, IP_ACTIVE_LOW )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service Coin") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x000000f3, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("JP1")
	PORT_DIPNAME( 0x03, 0x03, "JP1" )
	PORT_DIPSETTING(	0x03, "Update"  )
	PORT_DIPSETTING(	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0BITS")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_MAHJONG_A ) PORT_PLAYER(1)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_MAHJONG_B ) PORT_PLAYER(1)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_MAHJONG_E ) PORT_PLAYER(1)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_MAHJONG_F ) PORT_PLAYER(1)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_MAHJONG_I ) PORT_PLAYER(1)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_MAHJONG_J ) PORT_PLAYER(1)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_MAHJONG_M ) PORT_PLAYER(1)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_MAHJONG_N ) PORT_PLAYER(1)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_MAHJONG_KAN ) PORT_PLAYER(1)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_MAHJONG_REACH ) PORT_PLAYER(1)
	PORT_BIT( 0x400, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_START("IN1BITS")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_MAHJONG_C ) PORT_PLAYER(1)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_MAHJONG_D ) PORT_PLAYER(1)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_MAHJONG_G ) PORT_PLAYER(1)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_MAHJONG_H ) PORT_PLAYER(1)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_MAHJONG_K ) PORT_PLAYER(1)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_MAHJONG_L ) PORT_PLAYER(1)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_MAHJONG_CHI ) PORT_PLAYER(1)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_MAHJONG_PON ) PORT_PLAYER(1)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_MAHJONG_RON ) PORT_PLAYER(1)
INPUT_PORTS_END

static INPUT_PORTS_START( spi_ejsakura )
	PORT_START("INPUTS")
	PORT_BIT( 0xffffffff, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM(ejsakura_keyboard_r, NULL)

	PORT_START("INPUT01")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(1)
	PORT_BIT( 0xffffffe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INPUT02")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )  PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )  PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(1)
	PORT_BIT( 0xffffffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INPUT04")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(1)
	PORT_BIT( 0xffffffe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INPUT08")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xffffffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INPUT10")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(1)
	PORT_SERVICE_NO_TOGGLE( 0x00000200, IP_ACTIVE_LOW) /* Test Button */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Payout") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0xfffff5c0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00004000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)
	PORT_BIT( 0xffffbf3f, IP_ACTIVE_LOW, IPT_UNUSED )

INPUT_PORTS_END
/********************************************************************/

#define PLANE_CHAR 0
#define PLANE_TILE 0
#define PLANE_SPRITE 0

static const gfx_layout spi_charlayout =
{
	8,8,		/* 8*8 characters */
	4096,		/* 4096 characters */
	5,			/* 6 bits per pixel */
	{ 4, 8, 12, 16, 20 },
	{ 3, 2, 1, 0, 27, 26, 25, 24 },
	{ 0*48, 1*48, 2*48, 3*48, 4*48, 5*48, 6*48, 7*48 },
	6*8*8
};

#if PLANE_CHAR
static const gfx_layout spi_charlayout0 =
{
	8,8,		/* 8*8 characters */
	4096,		/* 4096 characters */
	1,			/* 6 bits per pixel */
	{ 0 },
	{ 3, 2, 1, 0, 27, 26, 25, 24 },
	{ 0*48, 1*48, 2*48, 3*48, 4*48, 5*48, 6*48, 7*48 },
	6*8*8
};

static const gfx_layout spi_charlayout1 =
{
	8,8,		/* 8*8 characters */
	4096,		/* 4096 characters */
	1,			/* 6 bits per pixel */
	{ 4 },
	{ 3, 2, 1, 0, 27, 26, 25, 24 },
	{ 0*48, 1*48, 2*48, 3*48, 4*48, 5*48, 6*48, 7*48 },
	6*8*8
};

static const gfx_layout spi_charlayout2 =
{
	8,8,		/* 8*8 characters */
	4096,		/* 4096 characters */
	1,			/* 6 bits per pixel */
	{ 8 },
	{ 3, 2, 1, 0, 27, 26, 25, 24 },
	{ 0*48, 1*48, 2*48, 3*48, 4*48, 5*48, 6*48, 7*48 },
	6*8*8
};

static const gfx_layout spi_charlayout3 =
{
	8,8,		/* 8*8 characters */
	4096,		/* 4096 characters */
	1,			/* 6 bits per pixel */
	{ 12 },
	{ 3, 2, 1, 0, 27, 26, 25, 24 },
	{ 0*48, 1*48, 2*48, 3*48, 4*48, 5*48, 6*48, 7*48 },
	6*8*8
};

static const gfx_layout spi_charlayout4 =
{
	8,8,		/* 8*8 characters */
	4096,		/* 4096 characters */
	1,			/* 6 bits per pixel */
	{ 16 },
	{ 3, 2, 1, 0, 27, 26, 25, 24 },
	{ 0*48, 1*48, 2*48, 3*48, 4*48, 5*48, 6*48, 7*48 },
	6*8*8
};

static const gfx_layout spi_charlayout5 =
{
	8,8,		/* 8*8 characters */
	4096,		/* 4096 characters */
	1,			/* 6 bits per pixel */
	{ 20 },
	{ 3, 2, 1, 0, 27, 26, 25, 24 },
	{ 0*48, 1*48, 2*48, 3*48, 4*48, 5*48, 6*48, 7*48 },
	6*8*8
};
#endif

static const gfx_layout spi_tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	6,
	{ 0, 4, 8, 12, 16, 20 },
	{
		 3, 2, 1, 0,
	    27,26,25,24,
		51,50,49,48,
		75,74,73,72
	},
	{ 0*96, 1*96, 2*96, 3*96, 4*96, 5*96, 6*96, 7*96,
	  8*96, 9*96, 10*96, 11*96, 12*96, 13*96, 14*96, 15*96
	},
	6*16*16
};

#if PLANE_TILE
static const gfx_layout spi_tilelayout0 =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{
		 3, 2, 1, 0,
	    27,26,25,24,
		51,50,49,48,
		75,74,73,72
	},
	{ 0*96, 1*96, 2*96, 3*96, 4*96, 5*96, 6*96, 7*96,
	  8*96, 9*96, 10*96, 11*96, 12*96, 13*96, 14*96, 15*96
	},
	6*16*16
};

static const gfx_layout spi_tilelayout1 =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 4 },
	{
		 3, 2, 1, 0,
	    27,26,25,24,
		51,50,49,48,
		75,74,73,72
	},
	{ 0*96, 1*96, 2*96, 3*96, 4*96, 5*96, 6*96, 7*96,
	  8*96, 9*96, 10*96, 11*96, 12*96, 13*96, 14*96, 15*96
	},
	6*16*16
};

static const gfx_layout spi_tilelayout2 =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 8 },
	{
		 3, 2, 1, 0,
	    27,26,25,24,
		51,50,49,48,
		75,74,73,72
	},
	{ 0*96, 1*96, 2*96, 3*96, 4*96, 5*96, 6*96, 7*96,
	  8*96, 9*96, 10*96, 11*96, 12*96, 13*96, 14*96, 15*96
	},
	6*16*16
};

static const gfx_layout spi_tilelayout3 =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 12 },
	{
		 3, 2, 1, 0,
	    27,26,25,24,
		51,50,49,48,
		75,74,73,72
	},
	{ 0*96, 1*96, 2*96, 3*96, 4*96, 5*96, 6*96, 7*96,
	  8*96, 9*96, 10*96, 11*96, 12*96, 13*96, 14*96, 15*96
	},
	6*16*16
};

static const gfx_layout spi_tilelayout4 =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 16 },
	{
		 3, 2, 1, 0,
	    27,26,25,24,
		51,50,49,48,
		75,74,73,72
	},
	{ 0*96, 1*96, 2*96, 3*96, 4*96, 5*96, 6*96, 7*96,
	  8*96, 9*96, 10*96, 11*96, 12*96, 13*96, 14*96, 15*96
	},
	6*16*16
};

static const gfx_layout spi_tilelayout5 =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 20 },
	{
		 3, 2, 1, 0,
	    27,26,25,24,
		51,50,49,48,
		75,74,73,72
	},
	{ 0*96, 1*96, 2*96, 3*96, 4*96, 5*96, 6*96, 7*96,
	  8*96, 9*96, 10*96, 11*96, 12*96, 13*96, 14*96, 15*96
	},
	6*16*16
};
#endif

static const gfx_layout spi_spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	6,
	{ 0,8, RGN_FRAC(1,3)+0,RGN_FRAC(1,3)+8,RGN_FRAC(2,3)+0,RGN_FRAC(2,3)+8  },
	{
		7,6,5,4,3,2,1,0,23,22,21,20,19,18,17,16
	},
	{
		0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32,8*32,9*32,10*32,11*32,12*32,13*32,14*32,15*32
	},
	16*32
};

#if PLANE_SPRITE
static const gfx_layout spi_spritelayout0 =
{
	16,16,
	RGN_FRAC(1,3),
	1,
	{ 0 },
	{
		7,6,5,4,3,2,1,0,23,22,21,20,19,18,17,16
	},
	{
		0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32,8*32,9*32,10*32,11*32,12*32,13*32,14*32,15*32
	},
	16*32
};

static const gfx_layout spi_spritelayout1 =
{
	16,16,
	RGN_FRAC(1,3),
	1,
	{ 8 },
	{
		7,6,5,4,3,2,1,0,23,22,21,20,19,18,17,16
	},
	{
		0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32,8*32,9*32,10*32,11*32,12*32,13*32,14*32,15*32
	},
	16*32
};

static const gfx_layout spi_spritelayout2 =
{
	16,16,
	RGN_FRAC(1,3),
	1,
	{ RGN_FRAC(1,3)+0 },
	{
		7,6,5,4,3,2,1,0,23,22,21,20,19,18,17,16
	},
	{
		0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32,8*32,9*32,10*32,11*32,12*32,13*32,14*32,15*32
	},
	16*32
};

static const gfx_layout spi_spritelayout3 =
{
	16,16,
	RGN_FRAC(1,3),
	1,
	{ RGN_FRAC(1,3)+8 },
	{
		7,6,5,4,3,2,1,0,23,22,21,20,19,18,17,16
	},
	{
		0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32,8*32,9*32,10*32,11*32,12*32,13*32,14*32,15*32
	},
	16*32
};

static const gfx_layout spi_spritelayout4 =
{
	16,16,
	RGN_FRAC(1,3),
	1,
	{ RGN_FRAC(2,3)+0 },
	{
		7,6,5,4,3,2,1,0,23,22,21,20,19,18,17,16
	},
	{
		0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32,8*32,9*32,10*32,11*32,12*32,13*32,14*32,15*32
	},
	16*32
};

static const gfx_layout spi_spritelayout5 =
{
	16,16,
	RGN_FRAC(1,3),
	1,
	{ RGN_FRAC(2,3)+8 },
	{
		7,6,5,4,3,2,1,0,23,22,21,20,19,18,17,16
	},
	{
		0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32,8*32,9*32,10*32,11*32,12*32,13*32,14*32,15*32
	},
	16*32
};
#endif

static GFXDECODE_START( spi )
	GFXDECODE_ENTRY( "gfx1", 0, spi_charlayout,   5632, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, spi_tilelayout,   4096, 24 )
	GFXDECODE_ENTRY( "gfx3", 0, spi_spritelayout,    0, 96 )
#if PLANE_CHAR
	GFXDECODE_ENTRY( "gfx1", 0, spi_charlayout0,   0x3ff, 1 )
	GFXDECODE_ENTRY( "gfx1", 0, spi_charlayout1,   0x3ff, 1 )
	GFXDECODE_ENTRY( "gfx1", 0, spi_charlayout2,   0x3ff, 1 )
	GFXDECODE_ENTRY( "gfx1", 0, spi_charlayout3,   0x3ff, 1 )
	GFXDECODE_ENTRY( "gfx1", 0, spi_charlayout4,   0x3ff, 1 )
	GFXDECODE_ENTRY( "gfx1", 0, spi_charlayout5,   0x3ff, 1 )
#endif
#if PLANE_TILE
	GFXDECODE_ENTRY( "gfx2", 0, spi_tilelayout0,   0x3ff, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, spi_tilelayout1,   0x3ff, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, spi_tilelayout2,   0x3ff, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, spi_tilelayout3,   0x3ff, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, spi_tilelayout4,   0x3ff, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, spi_tilelayout5,   0x3ff, 1 )
#endif
#if PLANE_SPRITE
	GFXDECODE_ENTRY( "gfx3", 0, spi_spritelayout0, 0x3ff, 1 )
	GFXDECODE_ENTRY( "gfx3", 0, spi_spritelayout1, 0x3ff, 1 )
	GFXDECODE_ENTRY( "gfx3", 0, spi_spritelayout2, 0x3ff, 1 )
	GFXDECODE_ENTRY( "gfx3", 0, spi_spritelayout3, 0x3ff, 1 )
	GFXDECODE_ENTRY( "gfx3", 0, spi_spritelayout4, 0x3ff, 1 )
	GFXDECODE_ENTRY( "gfx3", 0, spi_spritelayout5, 0x3ff, 1 )
#endif
GFXDECODE_END

static const gfx_layout sys386f2_spritelayout =
{
	16,16,
	RGN_FRAC(1,4),
	8,
	{ 0, 8, RGN_FRAC(1,4)+0, RGN_FRAC(1,4)+8, RGN_FRAC(2,4)+0, RGN_FRAC(2,4)+8, RGN_FRAC(3,4)+0, RGN_FRAC(3,4)+8 },
	{
		7,6,5,4,3,2,1,0,23,22,21,20,19,18,17,16
	},
	{
		0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32,8*32,9*32,10*32,11*32,12*32,13*32,14*32,15*32
	},
	16*32
};

static GFXDECODE_START( sys386f2)
	GFXDECODE_ENTRY( "gfx1", 0, spi_charlayout,          5632, 16 ) // Not used
	GFXDECODE_ENTRY( "gfx2", 0, spi_tilelayout,          4096, 24 ) // Not used
	GFXDECODE_ENTRY( "gfx3", 0, sys386f2_spritelayout,      0, 96 )
GFXDECODE_END

/********************************************************************************/

static NVRAM_HANDLER( spi )
{
	nvram_handler_intelflash(machine, 0, file, read_or_write);
	nvram_handler_intelflash(machine, 1, file, read_or_write);
}

/* this is a 93C46 but with reset delay */
static const eeprom_interface eeprom_intf =
{
	6,				/* address bits */
	16,				/* data bits */
	"*110",			/*  read command */
	"*101",			/* write command */
	"*111",			/* erase command */
	"*10000xxxx",	/* lock command */
	"*10011xxxx",	/* unlock command */
	1,				/* enable_multi_read */
	1				/* reset_delay */
};

static INTERRUPT_GEN( spi_interrupt )
{
	cpu_set_input_line(device, 0, ASSERT_LINE );
}

static IRQ_CALLBACK(spi_irq_callback)
{
	return 0x20;
}

/* SPI */

static MACHINE_START( spi )
{
	z80_rom = auto_alloc_array(machine, UINT8, 0x40000);
}

static MACHINE_RESET( spi )
{
	int i;
	UINT8 *sound = memory_region(machine, "ymf");

	UINT8 *rombase = memory_region(machine, "user1");
	UINT8 flash_data = rombase[0x1ffffc];

	cputag_set_input_line(machine, "soundcpu", INPUT_LINE_RESET, ASSERT_LINE );
	cpu_set_irq_callback(devtag_get_device(machine, "maincpu"), spi_irq_callback);

	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x00000680, 0x00000683, 0, 0, sound_fifo_r);
	memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x00000688, 0x0000068b, 0, 0, z80_prg_fifo_w);
	memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x0000068c, 0x0000068f, 0, 0, z80_enable_w);

	memory_set_bankptr(machine, "bank4", z80_rom);
	memory_set_bankptr(machine, "bank5", z80_rom);

	/* If the first value doesn't match, the game shows a checksum error */
	/* If any of the other values are wrong, the game goes to update mode */
	intelflash_write(0, 0, 0xff);
	intelflash_write(0, 0, 0x10);
	intelflash_write(0, 0, flash_data);			/* country code */

	for (i=0; i < 0x100000; i++)
	{
		intelflash_write(0, 0, 0xff);
		sound[i] = intelflash_read(0, i);
	}
	for (i=0; i < 0x100000; i++)
	{
		intelflash_write(1, 0, 0xff);
		sound[0x100000+i] = intelflash_read(1, i);
	}
}

static MACHINE_DRIVER_START( spi )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", I386, 50000000/2)	/* Intel 386DX, 25MHz */
	MDRV_CPU_PROGRAM_MAP(spi_map)
	MDRV_CPU_VBLANK_INT("screen", spi_interrupt)

	MDRV_CPU_ADD("soundcpu", Z80, 28636360/4)
	MDRV_CPU_PROGRAM_MAP(spisound_map)

	MDRV_QUANTUM_TIME(HZ(12000))

	MDRV_MACHINE_START(spi)
	MDRV_MACHINE_RESET(spi)
	MDRV_NVRAM_HANDLER(spi)

	MDRV_EEPROM_ADD("eeprom", eeprom_intf)

	MDRV_DS2404_ADD("ds2404", 1995, 1, 1)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(54)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 30*8-1)

	MDRV_GFXDECODE(spi)
	MDRV_PALETTE_LENGTH(6144)

	MDRV_VIDEO_START(spi)
	MDRV_VIDEO_UPDATE(spi)

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymf", YMF271, 16934400)
	MDRV_SOUND_CONFIG(ymf271_config)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END

static MACHINE_START( sxx2f )
{
	z80_rom = auto_alloc_array(machine, UINT8, 0x40000);
}

static MACHINE_RESET( sxx2f )
{
	UINT8 *rom = memory_region(machine, "soundcpu");

	memory_set_bankptr(machine, "bank4", z80_rom);
	memory_set_bankptr(machine, "bank5", z80_rom);

	memcpy(z80_rom, rom, 0x40000);

	memory_install_write32_device_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), devtag_get_device(machine, "eeprom"), 0x0000068c, 0x0000068f, 0, 0, eeprom_w);
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x00000680, 0x00000683, 0, 0, sb_coin_r);

	cpu_set_irq_callback(devtag_get_device(machine, "maincpu"), spi_irq_callback);

	sb_coin_latch = 0;
}

static MACHINE_DRIVER_START( sxx2f ) /* Intel i386DX @ 25MHz, YMF271 @ 16.9344MHz, Z80 @ 7.159MHz(?) */

	MDRV_IMPORT_FROM(spi)

	MDRV_MACHINE_START(sxx2f)
	MDRV_MACHINE_RESET(sxx2f)

	MDRV_NVRAM_HANDLER(0)

MACHINE_DRIVER_END

static MACHINE_DRIVER_START( sxx2g ) /* single board version using measured clocks */

	MDRV_IMPORT_FROM(spi)

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_CLOCK(28636360) /* AMD AM386DX/DX-40, 28.63636MHz */

	MDRV_CPU_MODIFY("soundcpu")
	MDRV_CPU_CLOCK(4915200) /* 4.9152MHz */

	MDRV_SOUND_REPLACE("ymf", YMF271, 16384000) /* 16.3840MHz */
	MDRV_SOUND_CONFIG(ymf271_config)

	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)

	MDRV_MACHINE_START(sxx2f)
	MDRV_MACHINE_RESET(sxx2f)

	MDRV_NVRAM_HANDLER(0)

MACHINE_DRIVER_END

static READ32_HANDLER ( senkyu_speedup_r )
{
	if (cpu_get_pc(space->cpu)==0x00305bb2) cpu_spinuntil_int(space->cpu); // idle
	return spimainram[(0x0018cb4-0x800)/4];
}

static READ32_HANDLER( senkyua_speedup_r )
{
	if (cpu_get_pc(space->cpu)== 0x30582e) cpu_spinuntil_int(space->cpu); // idle
	return spimainram[(0x0018c9c-0x800)/4];
}

static READ32_HANDLER ( batlball_speedup_r )
{
//  printf("cpu_get_pc(space->cpu) %06x\n", cpu_get_pc(space->cpu));

	/* batlbalu */
	if (cpu_get_pc(space->cpu)==0x00305996) cpu_spinuntil_int(space->cpu); // idle

	/* batlball */
	if (cpu_get_pc(space->cpu)==0x003058aa) cpu_spinuntil_int(space->cpu); // idle

	return spimainram[(0x0018db4-0x800)/4];
}

static READ32_HANDLER ( rdft_speedup_r )
{
	/* rdft */
	if (cpu_get_pc(space->cpu)==0x0203f0a) cpu_spinuntil_int(space->cpu); // idle

	/* rdftau */
	if (cpu_get_pc(space->cpu)==0x0203f16) cpu_spinuntil_int(space->cpu); // idle

	/* rdftj */
	if (cpu_get_pc(space->cpu)==0x0203f22) cpu_spinuntil_int(space->cpu); // idle

	/* rdftdi */
	if (cpu_get_pc(space->cpu)==0x0203f46) cpu_spinuntil_int(space->cpu); // idle

	/* rdftu */
	if (cpu_get_pc(space->cpu)==0x0203f3a) cpu_spinuntil_int(space->cpu); // idle

//  mame_printf_debug("%08x\n",cpu_get_pc(space->cpu));

	return spimainram[(0x00298d0-0x800)/4];
}

static READ32_HANDLER ( viprp1_speedup_r )
{
	/* viprp1 */
	if (cpu_get_pc(space->cpu)==0x0202769) cpu_spinuntil_int(space->cpu); // idle

	/* viprp1s */
	if (cpu_get_pc(space->cpu)==0x02027e9) cpu_spinuntil_int(space->cpu); // idle

	/* viprp1ot */
	if (cpu_get_pc(space->cpu)==0x02026bd) cpu_spinuntil_int(space->cpu); // idle

//  mame_printf_debug("%08x\n",cpu_get_pc(space->cpu));

	return spimainram[(0x001e2e0-0x800)/4];
}

static READ32_HANDLER ( viprp1o_speedup_r )
{
	/* viperp1o */
	if (cpu_get_pc(space->cpu)==0x0201f99) cpu_spinuntil_int(space->cpu); // idle
//  mame_printf_debug("%08x\n",cpu_get_pc(space->cpu));
	return spimainram[(0x001d49c-0x800)/4];
}

#ifdef UNUSED_FUNCTION
// causes input problems?
READ32_HANDLER ( ejanhs_speedup_r )
{
// mame_printf_debug("%08x\n",cpu_get_pc(space->cpu));
 if (cpu_get_pc(space->cpu)==0x03032c7) cpu_spinuntil_int(space->cpu); // idle
 return spimainram[(0x002d224-0x800)/4];
}
#endif

static READ32_HANDLER ( rf2_speedup_r )
{

	/* rdft22kc */
	if (cpu_get_pc(space->cpu)==0x0203926) cpu_spinuntil_int(space->cpu); // idle

	/* rdft2, rdft2j */
	if (cpu_get_pc(space->cpu)==0x0204372) cpu_spinuntil_int(space->cpu); // idle

	/* rdft2us */
	if (cpu_get_pc(space->cpu)==0x020420e) cpu_spinuntil_int(space->cpu); // idle

	/* rdft2a */
	if (cpu_get_pc(space->cpu)==0x0204366) cpu_spinuntil_int(space->cpu); // idle

//  mame_printf_debug("%08x\n",cpu_get_pc(space->cpu));

	return spimainram[(0x0282AC-0x800)/4];
}

static READ32_HANDLER ( rfjet_speedup_r )
{
	/* rfjet, rfjetu, rfjeta */
	if (cpu_get_pc(space->cpu)==0x0206082) cpu_spinuntil_int(space->cpu); // idle

	/* rfjetus */
	if (cpu_get_pc(space->cpu)==0x0205b39)
	{
		UINT32 r;
		cpu_spinuntil_int(space->cpu); // idle
		// Hack to enter test mode
		r = spimainram[(0x002894c-0x800)/4] & (~0x400);
		return r | (((input_port_read(space->machine, "SYSTEM") ^ 0xff)<<8) & 0x400);
	}

	/* rfjetj */
	if (cpu_get_pc(space->cpu)==0x0205f2e) cpu_spinuntil_int(space->cpu); // idle

//  mame_printf_debug("%08x\n",cpu_get_pc(space->cpu));


	return spimainram[(0x002894c-0x800)/4];
}

static void init_spi(running_machine *machine)
{
	intelflash_init( machine, 0, FLASH_INTEL_E28F008SA, NULL );
	intelflash_init( machine, 1, FLASH_INTEL_E28F008SA, NULL );

	seibuspi_text_decrypt(memory_region(machine, "gfx1"));
	seibuspi_bg_decrypt(memory_region(machine, "gfx2"), memory_region_length(machine, "gfx2"));
	seibuspi_sprite_decrypt(memory_region(machine, "gfx3"), 0x400000);
}

static DRIVER_INIT( rdft )
{
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x00298d0, 0x00298d3, 0, 0, rdft_speedup_r );

	init_spi(machine);
}

static DRIVER_INIT( senkyu )
{
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x0018cb4, 0x0018cb7, 0, 0, senkyu_speedup_r );

	init_spi(machine);
}

static DRIVER_INIT( senkyua )
{
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x0018c9c, 0x0018c9f, 0, 0, senkyua_speedup_r );

	init_spi(machine);
}

static DRIVER_INIT( batlball )
{
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x0018db4, 0x0018db7, 0, 0, batlball_speedup_r );

	init_spi(machine);
}

static DRIVER_INIT( ejanhs )
{
//  idle skip doesn't work properly?
//  memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x002d224, 0x002d227, 0, 0, ejanhs_speedup_r );

	init_spi(machine);
}

static DRIVER_INIT( viprp1 )
{
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x001e2e0, 0x001e2e3, 0, 0, viprp1_speedup_r );

	init_spi(machine);
}

static DRIVER_INIT( viprp1o )
{
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x001d49c, 0x001d49f, 0, 0, viprp1o_speedup_r );

	init_spi(machine);
}



static void init_rf2(running_machine *machine)
{
	intelflash_init( machine, 0, FLASH_INTEL_E28F008SA, NULL );
	intelflash_init( machine, 1, FLASH_INTEL_E28F008SA, NULL );

	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x0282AC, 0x0282AF, 0, 0, rf2_speedup_r );
	seibuspi_rise10_text_decrypt(memory_region(machine, "gfx1"));
	seibuspi_rise10_bg_decrypt(memory_region(machine, "gfx2"), memory_region_length(machine, "gfx2"));
	seibuspi_rise10_sprite_decrypt(memory_region(machine, "gfx3"), 0x600000);

	memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x560, 0x563, 0, 0, sprite_dma_start_w);
}

static DRIVER_INIT( rdft2 )
{
	init_rf2(machine);
}

static DRIVER_INIT( rdft2us )
{
	init_rf2(machine);
}


static DRIVER_INIT( rfjet )
{
	intelflash_init( machine, 0, FLASH_INTEL_E28F008SA, NULL );
	intelflash_init( machine, 1, FLASH_INTEL_E28F008SA, NULL );

	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x002894c, 0x002894f, 0, 0, rfjet_speedup_r );
	seibuspi_rise11_text_decrypt(memory_region(machine, "gfx1"));
	seibuspi_rise11_bg_decrypt(memory_region(machine, "gfx2"), memory_region_length(machine, "gfx2"));
	seibuspi_rise11_sprite_decrypt_rfjet(memory_region(machine, "gfx3"), 0x800000);

	memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x560, 0x563, 0, 0, sprite_dma_start_w);
}

/* SYS386 */

static DRIVER_INIT( rdft22kc )
{
	init_rf2(machine);
}

static MACHINE_RESET( seibu386 )
{
	cpu_set_irq_callback(devtag_get_device(machine, "maincpu"), spi_irq_callback);
}

static MACHINE_DRIVER_START( seibu386 )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", I386, 40000000)	/* AMD 386DX, 40MHz */
	MDRV_CPU_PROGRAM_MAP(seibu386_map)
	MDRV_CPU_VBLANK_INT("screen", spi_interrupt)

	MDRV_MACHINE_RESET(seibu386)

	MDRV_EEPROM_ADD("eeprom", eeprom_intf)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(54)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 30*8-1)

	MDRV_GFXDECODE(spi)
	MDRV_PALETTE_LENGTH(6144)

	MDRV_VIDEO_START(spi)
	MDRV_VIDEO_UPDATE(spi)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_OKIM6295_ADD("oki1", 1431815, OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_OKIM6295_ADD("oki2", 1431815, OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

/* SYS386-F V2.0 */
static DRIVER_INIT( sys386f2 )
{
	int i, j;
	UINT16 *src = (UINT16 *)memory_region(machine, "gfx3");
	UINT16 tmp[0x40 / 2], Offset;

	// sprite_reorder() only
	for(i = 0; i < memory_region_length(machine, "gfx3") / 0x40; i++)
	{
		memcpy(tmp, src, 0x40);

		for(j = 0; j < 0x40 / 2; j++)
		{
			Offset = (j >> 1) | (j << 4 & 0x10);
			*src++ = tmp[Offset];
		}
	}
}

static MACHINE_DRIVER_START( sys386f2 )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", I386, 25000000)	/* 25mhz */
	MDRV_CPU_PROGRAM_MAP(sys386f2_map)
	MDRV_CPU_VBLANK_INT("screen", spi_interrupt)

	/* no z80? */

	MDRV_MACHINE_RESET(seibu386)

	MDRV_EEPROM_ADD("eeprom", eeprom_intf)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(54)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 30*8-1)

	MDRV_GFXDECODE(sys386f2)
	MDRV_PALETTE_LENGTH(8192)

	MDRV_VIDEO_START(sys386f2)
	MDRV_VIDEO_UPDATE(sys386f2)

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymz", YMZ280B, 16934400)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


/*******************************************************************/
#define ROM_LOAD24_BYTE(name,offset,length,hash)		ROMX_LOAD(name, offset, length, hash, ROM_SKIP(2))
#define ROM_LOAD24_WORD(name,offset,length,hash)		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_SKIP(1) | ROM_REVERSE)
#define ROM_LOAD24_WORD_SWAP(name,offset,length,hash)	ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_SKIP(1))

/* SPI games */

ROM_START( senkyu )
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("fb_1.211", 0x100000, 0x40000, CRC(20a3e5db) SHA1(f1109aeceac7993abc9093d09429718ffc292c77) )
	ROM_LOAD32_BYTE("fb_2.212", 0x100001, 0x40000, CRC(38e90619) SHA1(451ab5f4a5935bb779f9c245c1c4358e80d93c15) )
	ROM_LOAD32_BYTE("fb_3.210", 0x100002, 0x40000, CRC(226f0429) SHA1(69d0fe6671278d7fe215e455bb50abf631cdb484) )
	ROM_LOAD32_BYTE("fb_4.29",  0x100003, 0x40000, CRC(b46d66b7) SHA1(1acd0fea9384e1488b44661e0c99b9672a3f9803) )

	ROM_REGION( 0x30000, "gfx1", 0)
	ROM_LOAD24_WORD("fb_6.413", 0x000000, 0x20000, CRC(b57115c9) SHA1(eb95f416f522032ca949bfb6348f1ff824101f2d) )
	ROM_LOAD24_BYTE("fb_5.48",  0x000002, 0x10000, CRC(440a9ae3) SHA1(3f57e6da91f0dac2d816c873759f1e1d3259caf1) )

	ROM_REGION( 0x300000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("fb_bg-1d.415", 0x000000, 0x200000, CRC(eae7a1fc) SHA1(26d8a9f4e554848977ec1f6a8aad8751b558a8d4) )
	ROM_LOAD24_BYTE("fb_bg-1p.410", 0x000002, 0x100000, CRC(b46e774e) SHA1(00b6c1d0b0ea37f4354acab543b270c0bf45896d) )

	ROM_REGION( 0xc00000, "gfx3", 0)	/* sprites */
	ROM_LOAD("fb_obj-1.322", 0x000000, 0x400000, CRC(29f86f68) SHA1(1afe809ce00a25f8b27543e4188edc3e3e604951) )
	ROM_LOAD("fb_obj-2.324", 0x400000, 0x400000, CRC(c9e3130b) SHA1(12b5d5363142e8efb3b7fc44289c0afffa5011c6) )
	ROM_LOAD("fb_obj-3.323", 0x800000, 0x400000, CRC(f6c3bc49) SHA1(d0eb9c6aa3954d94e3a442a48e0fe6cc279f5513) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("fb_pcm-1.215",  0x000000, 0x080000, CRC(1d83891c) SHA1(09502437562275c14c0f3a0e62b19e91bedb4693) )
	ROM_CONTINUE(0x100000,0x080000)
	ROM_LOAD("fb_7.216",      0x200000, 0x080000, CRC(874d7b59) SHA1(0236753636c9a818780b23f5f506697b9f6d93c7) )
ROM_END

ROM_START( senkyua )
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("1.bin", 0x100000, 0x40000, CRC(6102c3fb) SHA1(4a55b41d916768f9601513db973b82077bca47c5) )
	ROM_LOAD32_BYTE("2.bin", 0x100001, 0x40000, CRC(d5b8ce46) SHA1(f6e4b8f51146179efb52ecb2b72fdeaee10b7282) )
	ROM_LOAD32_BYTE("3.bin", 0x100002, 0x40000, CRC(e27ceccd) SHA1(3d6b8e97e89939c72d1a5a4a3856025b5f548645) )
	ROM_LOAD32_BYTE("4.bin", 0x100003, 0x40000, CRC(7c6d4549) SHA1(efc6920a2e518afe849fb6fe191e7cd0bc483be5) )

	ROM_REGION( 0x30000, "gfx1", 0)
	ROM_LOAD24_WORD("fb_6.413", 0x000000, 0x20000, CRC(b57115c9) SHA1(eb95f416f522032ca949bfb6348f1ff824101f2d) )
	ROM_LOAD24_BYTE("fb_5.48",  0x000002, 0x10000, CRC(440a9ae3) SHA1(3f57e6da91f0dac2d816c873759f1e1d3259caf1) )

	ROM_REGION( 0x300000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("fb_bg-1d.415", 0x000000, 0x200000, CRC(eae7a1fc) SHA1(26d8a9f4e554848977ec1f6a8aad8751b558a8d4) )
	ROM_LOAD24_BYTE("fb_bg-1p.410", 0x000002, 0x100000, CRC(b46e774e) SHA1(00b6c1d0b0ea37f4354acab543b270c0bf45896d) )

	ROM_REGION( 0xc00000, "gfx3", 0)	/* sprites */
	ROM_LOAD("fb_obj-1.322", 0x000000, 0x400000, CRC(29f86f68) SHA1(1afe809ce00a25f8b27543e4188edc3e3e604951) )
	ROM_LOAD("fb_obj-2.324", 0x400000, 0x400000, CRC(c9e3130b) SHA1(12b5d5363142e8efb3b7fc44289c0afffa5011c6) )
	ROM_LOAD("fb_obj-3.323", 0x800000, 0x400000, CRC(f6c3bc49) SHA1(d0eb9c6aa3954d94e3a442a48e0fe6cc279f5513) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("fb_pcm-1.215",  0x000000, 0x080000, CRC(1d83891c) SHA1(09502437562275c14c0f3a0e62b19e91bedb4693) )
	ROM_CONTINUE(0x100000,0x080000)
	ROM_LOAD("fb_7.216",      0x200000, 0x080000, CRC(874d7b59) SHA1(0236753636c9a818780b23f5f506697b9f6d93c7) )
ROM_END

ROM_START( batlball )
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("1.211", 0x100000, 0x40000, CRC(d4e48f89) SHA1(10e43a9ff3f6f169de6352280a8a06e7f482271a) )
	ROM_LOAD32_BYTE("2.212", 0x100001, 0x40000, CRC(3077720b) SHA1(b65c3d02ac75eb56e0c5dc1bf6bb6a4e445a41cf) )
	ROM_LOAD32_BYTE("3.210", 0x100002, 0x40000, CRC(520d31e1) SHA1(998ae968113ab5b2891044187d93793903c13452) )
	ROM_LOAD32_BYTE("4.029", 0x100003, 0x40000, CRC(22419b78) SHA1(67475a654d4ad94e5dfda88cbe2f9c1b5ba6d2cc) )

	ROM_REGION( 0x30000, "gfx1", 0)
	ROM_LOAD24_WORD("fb_6.413", 0x000000, 0x20000, CRC(b57115c9) SHA1(eb95f416f522032ca949bfb6348f1ff824101f2d) )
	ROM_LOAD24_BYTE("fb_5.48",  0x000002, 0x10000, CRC(440a9ae3) SHA1(3f57e6da91f0dac2d816c873759f1e1d3259caf1) )

	ROM_REGION( 0x300000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("fb_bg-1d.415", 0x000000, 0x200000, CRC(eae7a1fc) SHA1(26d8a9f4e554848977ec1f6a8aad8751b558a8d4) )
	ROM_LOAD24_BYTE("fb_bg-1p.410", 0x000002, 0x100000, CRC(b46e774e) SHA1(00b6c1d0b0ea37f4354acab543b270c0bf45896d) )

	ROM_REGION( 0xc00000, "gfx3", 0)	/* sprites */
	ROM_LOAD("fb_obj-1.322", 0x000000, 0x400000, CRC(29f86f68) SHA1(1afe809ce00a25f8b27543e4188edc3e3e604951) )
	ROM_LOAD("fb_obj-2.324", 0x400000, 0x400000, CRC(c9e3130b) SHA1(12b5d5363142e8efb3b7fc44289c0afffa5011c6) )
	ROM_LOAD("fb_obj-3.323", 0x800000, 0x400000, CRC(f6c3bc49) SHA1(d0eb9c6aa3954d94e3a442a48e0fe6cc279f5513) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("fb_pcm-1.215",  0x000000, 0x080000, CRC(1d83891c) SHA1(09502437562275c14c0f3a0e62b19e91bedb4693) )
	ROM_CONTINUE(0x100000,0x080000)
	ROM_LOAD("fb_7.216",      0x200000, 0x080000, CRC(874d7b59) SHA1(0236753636c9a818780b23f5f506697b9f6d93c7) )
ROM_END

ROM_START( batlballa )
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("senkyua1.bin", 0x100000, 0x40000, CRC(ec3c4d4d) SHA1(6c57b8fbb77ce1615850842d06c054e88e240eef) )
	ROM_LOAD32_BYTE("2.212", 0x100001, 0x40000, CRC(3077720b) SHA1(b65c3d02ac75eb56e0c5dc1bf6bb6a4e445a41cf) )
	ROM_LOAD32_BYTE("3.210", 0x100002, 0x40000, CRC(520d31e1) SHA1(998ae968113ab5b2891044187d93793903c13452) )
	ROM_LOAD32_BYTE("4.029", 0x100003, 0x40000, CRC(22419b78) SHA1(67475a654d4ad94e5dfda88cbe2f9c1b5ba6d2cc) )

	ROM_REGION( 0x30000, "gfx1", 0)
	ROM_LOAD24_WORD("fb_6.413", 0x000000, 0x20000, CRC(b57115c9) SHA1(eb95f416f522032ca949bfb6348f1ff824101f2d) )
	ROM_LOAD24_BYTE("fb_5.48",  0x000002, 0x10000, CRC(440a9ae3) SHA1(3f57e6da91f0dac2d816c873759f1e1d3259caf1) )

	ROM_REGION( 0x300000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("fb_bg-1d.415", 0x000000, 0x200000, CRC(eae7a1fc) SHA1(26d8a9f4e554848977ec1f6a8aad8751b558a8d4) )
	ROM_LOAD24_BYTE("fb_bg-1p.410", 0x000002, 0x100000, CRC(b46e774e) SHA1(00b6c1d0b0ea37f4354acab543b270c0bf45896d) )

	ROM_REGION( 0xc00000, "gfx3", 0)	/* sprites */
	ROM_LOAD("fb_obj-1.322", 0x000000, 0x400000, CRC(29f86f68) SHA1(1afe809ce00a25f8b27543e4188edc3e3e604951) )
	ROM_LOAD("fb_obj-2.324", 0x400000, 0x400000, CRC(c9e3130b) SHA1(12b5d5363142e8efb3b7fc44289c0afffa5011c6) )
	ROM_LOAD("fb_obj-3.323", 0x800000, 0x400000, CRC(f6c3bc49) SHA1(d0eb9c6aa3954d94e3a442a48e0fe6cc279f5513) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("fb_pcm-1.215",  0x000000, 0x080000, CRC(1d83891c) SHA1(09502437562275c14c0f3a0e62b19e91bedb4693) )
	ROM_CONTINUE(0x100000,0x080000)
	ROM_LOAD("fb_7.216",      0x200000, 0x080000, CRC(874d7b59) SHA1(0236753636c9a818780b23f5f506697b9f6d93c7) )
ROM_END

ROM_START( batlballu )
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("sen1.bin", 0x100000, 0x40000, CRC(13849bf0) SHA1(ffa829a8b8a05a8fbaf883a30759f2ad8071a85b) )
	ROM_LOAD32_BYTE("sen2.bin", 0x100001, 0x40000, CRC(2ae5f7e2) SHA1(cef9ddea8b1d21f20a48c2523c9420c1800720c8) )
	ROM_LOAD32_BYTE("sen3.bin", 0x100002, 0x40000, CRC(98e6f19f) SHA1(433f8463e63bba32730d3c098354f8c95257df3f) )
	ROM_LOAD32_BYTE("sen4.bin", 0x100003, 0x40000, CRC(1343ec56) SHA1(8ecc8d7b425ff6512ffa969a7f26423fa50ad258) )

	ROM_REGION( 0x30000, "gfx1", 0)
	ROM_LOAD24_WORD("fb_6.413", 0x000000, 0x20000, CRC(b57115c9) SHA1(eb95f416f522032ca949bfb6348f1ff824101f2d) )
	ROM_LOAD24_BYTE("fb_5.48",  0x000002, 0x10000, CRC(440a9ae3) SHA1(3f57e6da91f0dac2d816c873759f1e1d3259caf1) )

	ROM_REGION( 0x300000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("fb_bg-1d.415", 0x000000, 0x200000, CRC(eae7a1fc) SHA1(26d8a9f4e554848977ec1f6a8aad8751b558a8d4) )
	ROM_LOAD24_BYTE("fb_bg-1p.410", 0x000002, 0x100000, CRC(b46e774e) SHA1(00b6c1d0b0ea37f4354acab543b270c0bf45896d) )

	ROM_REGION( 0xc00000, "gfx3", 0)	/* sprites */
	ROM_LOAD("fb_obj-1.322", 0x000000, 0x400000, CRC(29f86f68) SHA1(1afe809ce00a25f8b27543e4188edc3e3e604951) )
	ROM_LOAD("fb_obj-2.324", 0x400000, 0x400000, CRC(c9e3130b) SHA1(12b5d5363142e8efb3b7fc44289c0afffa5011c6) )
	ROM_LOAD("fb_obj-3.323", 0x800000, 0x400000, CRC(f6c3bc49) SHA1(d0eb9c6aa3954d94e3a442a48e0fe6cc279f5513) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("fb_pcm-1.215",  0x000000, 0x080000, CRC(1d83891c) SHA1(09502437562275c14c0f3a0e62b19e91bedb4693) )
	ROM_CONTINUE(0x100000,0x080000)
	ROM_LOAD("fb_7.216",      0x200000, 0x080000, CRC(874d7b59) SHA1(0236753636c9a818780b23f5f506697b9f6d93c7) )
ROM_END

ROM_START( ejanhs )
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("ejan3_1.211", 0x100000, 0x40000, CRC(e626d3d2) SHA1(d23cb5e218a85e09de98fa966afbfd43090b396e) )
	ROM_LOAD32_BYTE("ejan3_2.212", 0x100001, 0x40000, CRC(83c39da2) SHA1(9526ffb5d5becccf0aa2e338ab4a3c873d575e6f) )
	ROM_LOAD32_BYTE("ejan3_3.210", 0x100002, 0x40000, CRC(46897b7d) SHA1(a22e0467c016e72bf99df2c1e6ecc792b2151b15) )
	ROM_LOAD32_BYTE("ejan3_4.29",  0x100003, 0x40000, CRC(b3187a2b) SHA1(7fc11ed5ceb2e45f784e75307fef8b850a981a2e) )

	ROM_REGION( 0x30000, "gfx1", 0)
	ROM_LOAD24_WORD("ejan3_6.413", 0x000000, 0x20000, CRC(837e012c) SHA1(815452083b65885d6e66dfc058ceec81bb3e6678) )
	ROM_LOAD24_BYTE("ejan3_5.48",  0x000002, 0x10000, CRC(d62db7bf) SHA1(c88f1bb6106c59179b914962ed8cdd4095fd9ce8) )

	ROM_REGION( 0x600000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("ej3_bg1d.415", 0x000000, 0x200000, CRC(bcacabe0) SHA1(b73581cf923196326b5b0b99e6aedb915bab0880) )
	ROM_LOAD24_BYTE("ej3_bg1p.410", 0x000002, 0x100000, CRC(1fd0eb5e) SHA1(ca64c8020b246128232f4f6c0a0a2dd9cd3efeae) )
	ROM_LOAD24_WORD("ej3_bg2d.416", 0x300000, 0x100000, CRC(ea2acd69) SHA1(b796e9e4b7342bf452f5ffdbce32cfefc603ba0f) )
	ROM_LOAD24_BYTE("ej3_bg2p.49",  0x300002, 0x080000, CRC(a4a9cb0f) SHA1(da177d13bb95bf6b987d3ca13bcdc86570807b2c) )

	ROM_REGION( 0xc00000, "gfx3", 0)	/* sprites */
	ROM_LOAD("ej3_obj1.322", 0x000000, 0x400000, CRC(852f180e) SHA1(d4845dace45c05a68f3b38ccb301c5bf5dce4174) )
	ROM_LOAD("ej3_obj2.324", 0x400000, 0x400000, CRC(1116ad08) SHA1(d5c81383b3f9ede7dd03e6be35487b40740b1f8f) )
	ROM_LOAD("ej3_obj3.323", 0x800000, 0x400000, CRC(ccfe02b6) SHA1(368bc8efe9d6677ba3d0cfc0f450a4bda32988be) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("ej3_pcm1.215",  0x000000, 0x080000, CRC(a92a3a82) SHA1(b86c27c5a2831ddd2a1c2b071018a99afec14018) )
	ROM_CONTINUE(0x100000,0x080000)
	ROM_LOAD("ejan3_7.216",   0x200000, 0x080000, CRC(c6fc6bcf) SHA1(d4d8c06d295f8eacfa10c21dbab5858f936121f3) )
ROM_END


ROM_START( viprp1 )
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("seibu1.211", 0x000000, 0x80000, CRC(e5caf4ff) SHA1(7c87a4e8e8dacfb7cc0be8f778352bce2801e59b) )
	ROM_LOAD32_BYTE("seibu2.212", 0x000001, 0x80000, CRC(688a998e) SHA1(0c48374b6800cd00e3ee96c0fb12119a680b091d) )
	ROM_LOAD32_BYTE("seibu3.210", 0x000002, 0x80000, CRC(990fa76a) SHA1(7619a631d6f83b3677eb47f984aff684e9518d6d) )
	ROM_LOAD32_BYTE("seibu4.29",  0x000003, 0x80000, CRC(13e3e343) SHA1(aac0c7450059847f53b5081e4abf26303a50f999) )

	ROM_REGION( 0x30000, "gfx1", 0)
	ROM_LOAD24_WORD("seibu5.u0413", 0x000000, 0x20000, CRC(5ece677c) SHA1(b782cf3296f866f79fafa69ff719211c9d4026df) )
	ROM_LOAD24_BYTE("seibu6.u048",  0x000002, 0x10000, CRC(44844ef8) SHA1(bcbe24d2ffb64f9165ba4ab7de27f44b99b5ff5a) )

	ROM_REGION( 0x600000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("v_bg-11.415",  0x000000, 0x200000, CRC(6fc96736) SHA1(12df47d8af2c1febc1bce5bcf3218766447885bd) )
	ROM_LOAD24_BYTE("v_bg-12.415",  0x000002, 0x100000, CRC(d3c7281c) SHA1(340bca1f31486609b3c34dd7830362a216ff648e) )
	ROM_LOAD24_WORD("v_bg-21.410",  0x300000, 0x100000, CRC(d65b4318) SHA1(6522970d95ffa7fa2f32e0b5b4f0eb69e0286b36) )
	ROM_LOAD24_BYTE("v_bg-22.416",  0x300002, 0x080000, CRC(24a0a23a) SHA1(0b0330717620e3f3274a25845d9edaf8023b9db2) )

	ROM_REGION( 0xc00000, "gfx3", 0)	/* sprites */
	ROM_LOAD("v_obj-1.322",  0x000000, 0x400000, CRC(3be5b631) SHA1(fd1064428d28ca166a9267b968c0ba846cfed656) )
	ROM_LOAD("v_obj-2.324",  0x400000, 0x400000, CRC(924153b4) SHA1(db5dadcfb4cd5e6efe9d995085936ce4f4eb4254) )
	ROM_LOAD("v_obj-3.323",  0x800000, 0x400000, CRC(e9fb9062) SHA1(18e97b4c5cced2b529e6e72d8041c6f78fcec76e) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("v_pcm.215",  0x000000, 0x080000, CRC(e3111b60) SHA1(f7a7747f29c392876e43efcb4e6c0741454082f2) )
	ROM_CONTINUE(0x100000,0x80000) /* stops reading around 00ee8a6, rom is empty at this point, countdown continues anyway */
ROM_END

ROM_START( viprp1u )
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("seibu1.u0211", 0x000000, 0x80000, CRC(3f412b80) SHA1(ccffce101d20971278c0f6c5f4efcf3ab687aba6) ) /* New version, "=U.S.A=" seems part of title */
	ROM_LOAD32_BYTE("seibu2.u0212", 0x000001, 0x80000, CRC(2e6c2376) SHA1(b6e660dc7c89cf565c6e055683e84ffcf8179709) )
	ROM_LOAD32_BYTE("seibu3.u0210", 0x000002, 0x80000, CRC(c38f7b4e) SHA1(d5bf2c7f2f6c812c65005facfd40ac6d3b61f29d) )
	ROM_LOAD32_BYTE("seibu4.u029",  0x000003, 0x80000, CRC(523cbef3) SHA1(5d15261b8fb108e0ba4dfd14d259984ef81ce877) )

	ROM_REGION( 0x30000, "gfx1", 0)
	ROM_LOAD24_WORD("seibu5.u0413", 0x000000, 0x20000, CRC(5ece677c) SHA1(b782cf3296f866f79fafa69ff719211c9d4026df) )
	ROM_LOAD24_BYTE("seibu6.u048",  0x000002, 0x10000, CRC(44844ef8) SHA1(bcbe24d2ffb64f9165ba4ab7de27f44b99b5ff5a) )

	ROM_REGION( 0x600000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("v_bg-11.415",  0x000000, 0x200000, CRC(6fc96736) SHA1(12df47d8af2c1febc1bce5bcf3218766447885bd) )
	ROM_LOAD24_BYTE("v_bg-12.415",  0x000002, 0x100000, CRC(d3c7281c) SHA1(340bca1f31486609b3c34dd7830362a216ff648e) )
	ROM_LOAD24_WORD("v_bg-21.410",  0x300000, 0x100000, CRC(d65b4318) SHA1(6522970d95ffa7fa2f32e0b5b4f0eb69e0286b36) )
	ROM_LOAD24_BYTE("v_bg-22.416",  0x300002, 0x080000, CRC(24a0a23a) SHA1(0b0330717620e3f3274a25845d9edaf8023b9db2) )

	ROM_REGION( 0xc00000, "gfx3", 0)	/* sprites */
	ROM_LOAD("v_obj-1.322",  0x000000, 0x400000, CRC(3be5b631) SHA1(fd1064428d28ca166a9267b968c0ba846cfed656) )
	ROM_LOAD("v_obj-2.324",  0x400000, 0x400000, CRC(924153b4) SHA1(db5dadcfb4cd5e6efe9d995085936ce4f4eb4254) )
	ROM_LOAD("v_obj-3.323",  0x800000, 0x400000, CRC(e9fb9062) SHA1(18e97b4c5cced2b529e6e72d8041c6f78fcec76e) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("v_pcm.215",  0x000000, 0x080000, CRC(e3111b60) SHA1(f7a7747f29c392876e43efcb4e6c0741454082f2) )
	ROM_CONTINUE(0x100000,0x80000) /* stops reading around 00ee8a6, rom is empty at this point, countdown continues anyway */
ROM_END

ROM_START( viprp1j )
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("v_1-n.211", 0x000000, 0x80000, CRC(55f10b72) SHA1(2a1ebaa969f346bf3659ed8b0f469dce9eaf3b4b) )
	ROM_LOAD32_BYTE("v_2-n.212", 0x000001, 0x80000, CRC(0f888283) SHA1(7e5ac81279b9c7a06f07cb8ae76938cdd5c9beee) )
	ROM_LOAD32_BYTE("v_3-n.210", 0x000002, 0x80000, CRC(842434ac) SHA1(982d219c1d329122789c552208db2f4aaa4af7e4) )
	ROM_LOAD32_BYTE("v_4-n.29",  0x000003, 0x80000, CRC(a3948824) SHA1(fe076951427126c8b7fe81be84ecf0699597225b) )

	ROM_REGION( 0x30000, "gfx1", 0)
	ROM_LOAD24_WORD("seibu5.u0413", 0x000000, 0x20000, CRC(5ece677c) SHA1(b782cf3296f866f79fafa69ff719211c9d4026df) )
	ROM_LOAD24_BYTE("seibu6.u048",  0x000002, 0x10000, CRC(44844ef8) SHA1(bcbe24d2ffb64f9165ba4ab7de27f44b99b5ff5a) )

	ROM_REGION( 0x600000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("v_bg-11.415",  0x000000, 0x200000, CRC(6fc96736) SHA1(12df47d8af2c1febc1bce5bcf3218766447885bd) )
	ROM_LOAD24_BYTE("v_bg-12.415",  0x000002, 0x100000, CRC(d3c7281c) SHA1(340bca1f31486609b3c34dd7830362a216ff648e) )
	ROM_LOAD24_WORD("v_bg-21.410",  0x300000, 0x100000, CRC(d65b4318) SHA1(6522970d95ffa7fa2f32e0b5b4f0eb69e0286b36) )
	ROM_LOAD24_BYTE("v_bg-22.416",  0x300002, 0x080000, CRC(24a0a23a) SHA1(0b0330717620e3f3274a25845d9edaf8023b9db2) )

	ROM_REGION( 0xc00000, "gfx3", 0)	/* sprites */
	ROM_LOAD("v_obj-1.322",  0x000000, 0x400000, CRC(3be5b631) SHA1(fd1064428d28ca166a9267b968c0ba846cfed656) )
	ROM_LOAD("v_obj-2.324",  0x400000, 0x400000, CRC(924153b4) SHA1(db5dadcfb4cd5e6efe9d995085936ce4f4eb4254) )
	ROM_LOAD("v_obj-3.323",  0x800000, 0x400000, CRC(e9fb9062) SHA1(18e97b4c5cced2b529e6e72d8041c6f78fcec76e) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("v_pcm.215",  0x000000, 0x080000, CRC(e3111b60) SHA1(f7a7747f29c392876e43efcb4e6c0741454082f2) )
	ROM_CONTINUE(0x100000,0x80000) /* stops reading around 00ee8a6, rom is empty at this point, countdown continues anyway */
ROM_END

ROM_START( viprp1s )
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("viper_prg0.bin", 0x000000, 0x80000, CRC(ed9980b8) SHA1(bc324e9121ee1e55237bd91681f163ec7790de4c) )
	ROM_LOAD32_BYTE("viper_prg1.bin", 0x000001, 0x80000, CRC(9d4d3486) SHA1(ded6fa32b973046e50c40c40c446590b5f6d0b76) )
	ROM_LOAD32_BYTE("viper_prg2.bin", 0x000002, 0x80000, CRC(d7ea460b) SHA1(aed10adacd073f7d2b35f12ba4b7876e5c99d142) )
	ROM_LOAD32_BYTE("viper_prg3.bin", 0x000003, 0x80000, CRC(ca6df094) SHA1(921eec141ce2d449047172fa9cdf39d459b5cc7b) )

	ROM_REGION( 0x30000, "gfx1", 0)
	ROM_LOAD24_WORD("seibu5.u0413", 0x000000, 0x20000, CRC(5ece677c) SHA1(b782cf3296f866f79fafa69ff719211c9d4026df) )
	ROM_LOAD24_BYTE("seibu6.u048",  0x000002, 0x10000, CRC(44844ef8) SHA1(bcbe24d2ffb64f9165ba4ab7de27f44b99b5ff5a) )

	ROM_REGION( 0x600000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("v_bg-11.415",  0x000000, 0x200000, CRC(6fc96736) SHA1(12df47d8af2c1febc1bce5bcf3218766447885bd) )
	ROM_LOAD24_BYTE("v_bg-12.415",  0x000002, 0x100000, CRC(d3c7281c) SHA1(340bca1f31486609b3c34dd7830362a216ff648e) )
	ROM_LOAD24_WORD("v_bg-21.410",  0x300000, 0x100000, CRC(d65b4318) SHA1(6522970d95ffa7fa2f32e0b5b4f0eb69e0286b36) )
	ROM_LOAD24_BYTE("v_bg-22.416",  0x300002, 0x080000, CRC(24a0a23a) SHA1(0b0330717620e3f3274a25845d9edaf8023b9db2) )

	ROM_REGION( 0xc00000, "gfx3", 0)	/* sprites */
	ROM_LOAD("v_obj-1.322",  0x000000, 0x400000, CRC(3be5b631) SHA1(fd1064428d28ca166a9267b968c0ba846cfed656) )
	ROM_LOAD("v_obj-2.324",  0x400000, 0x400000, CRC(924153b4) SHA1(db5dadcfb4cd5e6efe9d995085936ce4f4eb4254) )
	ROM_LOAD("v_obj-3.323",  0x800000, 0x400000, CRC(e9fb9062) SHA1(18e97b4c5cced2b529e6e72d8041c6f78fcec76e) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("v_pcm.215",  0x000000, 0x080000, CRC(e3111b60) SHA1(f7a7747f29c392876e43efcb4e6c0741454082f2) )
	ROM_CONTINUE(0x100000,0x80000)
ROM_END

ROM_START( viprp1hk )
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("seibu_1", 0x000000, 0x80000, CRC(283ba7b7) SHA1(28122e04b72f1163c69f3f845f6a493fdb6ed652) ) /* Old Version, "=HONG KONG=" seems part of title */
	ROM_LOAD32_BYTE("seibu_2", 0x000001, 0x80000, CRC(2c4db249) SHA1(a6372c9a3cde5f262ec5ef446945f6d3ad506e88) )
	ROM_LOAD32_BYTE("seibu_3", 0x000002, 0x80000, CRC(91989503) SHA1(8c215fac200cc693396dbd57e0939e7efe883342) )
	ROM_LOAD32_BYTE("seibu_4", 0x000003, 0x80000, CRC(12c9582d) SHA1(a79e26514e5ab8703a7a8c3ac39b359cfa4117c1) )

	ROM_REGION( 0x30000, "gfx1", 0)
	ROM_LOAD24_WORD("seibu_5", 0x000000, 0x20000, CRC(80920fed) SHA1(b35ed080925f6d0a0b6d2d1ab4fa919f625b1e6a) ) /* Different from both "new" & "old" versions */
	ROM_LOAD24_BYTE("seibu_6", 0x000002, 0x10000, CRC(e71a8722) SHA1(3e0133fe1f85058ca6d9ac59d731f342c6b50e92) )

	ROM_REGION( 0x600000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("v_bg-11.415",  0x000000, 0x200000, CRC(6fc96736) SHA1(12df47d8af2c1febc1bce5bcf3218766447885bd) )
	ROM_LOAD24_BYTE("v_bg-12.415",  0x000002, 0x100000, CRC(d3c7281c) SHA1(340bca1f31486609b3c34dd7830362a216ff648e) )
	ROM_LOAD24_WORD("v_bg-21.410",  0x300000, 0x100000, CRC(d65b4318) SHA1(6522970d95ffa7fa2f32e0b5b4f0eb69e0286b36) )
	ROM_LOAD24_BYTE("v_bg-22.416",  0x300002, 0x080000, CRC(24a0a23a) SHA1(0b0330717620e3f3274a25845d9edaf8023b9db2) )

	ROM_REGION( 0xc00000, "gfx3", 0)	/* sprites */
	ROM_LOAD("v_obj-1.322",  0x000000, 0x400000, CRC(3be5b631) SHA1(fd1064428d28ca166a9267b968c0ba846cfed656) )
	ROM_LOAD("v_obj-2.324",  0x400000, 0x400000, CRC(924153b4) SHA1(db5dadcfb4cd5e6efe9d995085936ce4f4eb4254) )
	ROM_LOAD("v_obj-3.323",  0x800000, 0x400000, CRC(e9fb9062) SHA1(18e97b4c5cced2b529e6e72d8041c6f78fcec76e) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("v_pcm.215",  0x000000, 0x080000, CRC(e3111b60) SHA1(f7a7747f29c392876e43efcb4e6c0741454082f2) )
	ROM_CONTINUE(0x100000,0x80000)
ROM_END

ROM_START( viprp1oj )
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("v_1-o.211", 0x000000, 0x80000, CRC(4430be64) SHA1(96501a490042c289060d8510f6f79fbf64f79c1a) )
	ROM_LOAD32_BYTE("v_2-o.212", 0x000001, 0x80000, CRC(ffbd88f7) SHA1(cd7f291117dd18bd80fb1130eb87936ff7517ee3) )
	ROM_LOAD32_BYTE("v_3-o.210", 0x000002, 0x80000, CRC(6146db39) SHA1(04e68bfff320a3ffcb47686fa012a038538adc1a) )
	ROM_LOAD32_BYTE("v_4-o.29",  0x000003, 0x80000, CRC(dc8dd2b6) SHA1(20970706240c38c54084b4ae24b7ad23b31aa3de) )

	ROM_REGION( 0x30000, "gfx1", 0)
	ROM_LOAD24_WORD("v_5-o.413", 0x000000, 0x20000, CRC(6d863acc) SHA1(3e3e14f51b9394b24d7cbf562f1cfffc9ec2216d) )
	ROM_LOAD24_BYTE("v_6-o.48",  0x000002, 0x10000, CRC(fe7cb8f7) SHA1(55c7ab977c3666c8770deb62718d535673ffd4f8) )

	ROM_REGION( 0x600000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("v_bg-11.415",  0x000000, 0x200000, CRC(6fc96736) SHA1(12df47d8af2c1febc1bce5bcf3218766447885bd) )
	ROM_LOAD24_BYTE("v_bg-12.415",  0x000002, 0x100000, CRC(d3c7281c) SHA1(340bca1f31486609b3c34dd7830362a216ff648e) )
	ROM_LOAD24_WORD("v_bg-21.410",  0x300000, 0x100000, CRC(d65b4318) SHA1(6522970d95ffa7fa2f32e0b5b4f0eb69e0286b36) )
	ROM_LOAD24_BYTE("v_bg-22.416",  0x300002, 0x080000, CRC(24a0a23a) SHA1(0b0330717620e3f3274a25845d9edaf8023b9db2) )

	ROM_REGION( 0xc00000, "gfx3", 0)	/* sprites */
	ROM_LOAD("v_obj-1.322",  0x000000, 0x400000, CRC(3be5b631) SHA1(fd1064428d28ca166a9267b968c0ba846cfed656) )
	ROM_LOAD("v_obj-2.324",  0x400000, 0x400000, CRC(924153b4) SHA1(db5dadcfb4cd5e6efe9d995085936ce4f4eb4254) )
	ROM_LOAD("v_obj-3.323",  0x800000, 0x400000, CRC(e9fb9062) SHA1(18e97b4c5cced2b529e6e72d8041c6f78fcec76e) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("v_pcm.215",  0x000000, 0x080000, CRC(e3111b60) SHA1(f7a7747f29c392876e43efcb4e6c0741454082f2) )
	ROM_CONTINUE(0x100000,0x80000)
ROM_END

ROM_START( viprp1ot )
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("ov1.bin", 0x000000, 0x80000, CRC(cbad0e28) SHA1(fbc9b3b243ae0d556f41e8bef5f09489bb9e302b) )
	ROM_LOAD32_BYTE("ov2.bin", 0x000001, 0x80000, CRC(0e2bbcb5) SHA1(5e53d60357fb0f9efa441261fac79e153eb35f3d) )
	ROM_LOAD32_BYTE("ov3.bin", 0x000002, 0x80000, CRC(0e86686b) SHA1(0af207ea77ef378364d80d20ecbfba2f043f2405) )
	ROM_LOAD32_BYTE("ov4.bin", 0x000003, 0x80000, CRC(9d7dd325) SHA1(550a8b5ed60e7ac50c40ec3eaa2cd6462be4a619) )

	ROM_REGION( 0x30000, "gfx1", 0)
	ROM_LOAD24_WORD("v_5-o.413", 0x000000, 0x20000, CRC(6d863acc) SHA1(3e3e14f51b9394b24d7cbf562f1cfffc9ec2216d) )
	ROM_LOAD24_BYTE("v_6-o.48",  0x000002, 0x10000, CRC(fe7cb8f7) SHA1(55c7ab977c3666c8770deb62718d535673ffd4f8) )

	ROM_REGION( 0x600000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("v_bg-11.415",  0x000000, 0x200000, CRC(6fc96736) SHA1(12df47d8af2c1febc1bce5bcf3218766447885bd) )
	ROM_LOAD24_BYTE("v_bg-12.415",  0x000002, 0x100000, CRC(d3c7281c) SHA1(340bca1f31486609b3c34dd7830362a216ff648e) )
	ROM_LOAD24_WORD("v_bg-21.410",  0x300000, 0x100000, CRC(d65b4318) SHA1(6522970d95ffa7fa2f32e0b5b4f0eb69e0286b36) )
	ROM_LOAD24_BYTE("v_bg-22.416",  0x300002, 0x080000, CRC(24a0a23a) SHA1(0b0330717620e3f3274a25845d9edaf8023b9db2) )

	ROM_REGION( 0xc00000, "gfx3", 0)	/* sprites */
	ROM_LOAD("v_obj-1.322",  0x000000, 0x400000, CRC(3be5b631) SHA1(fd1064428d28ca166a9267b968c0ba846cfed656) )
	ROM_LOAD("v_obj-2.324",  0x400000, 0x400000, CRC(924153b4) SHA1(db5dadcfb4cd5e6efe9d995085936ce4f4eb4254) )
	ROM_LOAD("v_obj-3.323",  0x800000, 0x400000, CRC(e9fb9062) SHA1(18e97b4c5cced2b529e6e72d8041c6f78fcec76e) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("v_pcm.215",  0x000000, 0x080000, CRC(e3111b60) SHA1(f7a7747f29c392876e43efcb4e6c0741454082f2) )
	ROM_CONTINUE(0x100000,0x80000)
ROM_END


ROM_START( rdft )
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("gd_1.211", 0x000000, 0x80000, CRC(f6b2cbdc) SHA1(040c4ff961c8be388c8279b06b777d528c2acc1b) )
	ROM_LOAD32_BYTE("gd_2.212", 0x000001, 0x80000, CRC(1982f812) SHA1(4f12fc3fd7f7a4beda4d29cc81e3a58d255e441f) )
	ROM_LOAD32_BYTE("gd_3.210", 0x000002, 0x80000, CRC(b0f59f44) SHA1(d44fe074ddab35cd0190535cd9fbd7f9e49312a4) )
	ROM_LOAD32_BYTE("gd_4.29",  0x000003, 0x80000, CRC(cd8705bd) SHA1(b19a1486d6b899a134d7b518863ddc8f07967e8b) )

	ROM_REGION( 0x30000, "gfx1", 0)	/* text layer roms */
	ROM_LOAD24_BYTE("gd_5.423", 0x000000, 0x10000, CRC(8f8d4e14) SHA1(06c803975767ae98f40ba7ac5764a5bc8baa3a30) )
	ROM_LOAD24_BYTE("gd_6.424", 0x000001, 0x10000, CRC(6ac64968) SHA1(ec395205c24c4f864a1f805bb0d4641562d4faa9) )
	ROM_LOAD24_BYTE("gd_7.48",  0x000002, 0x10000, CRC(4d87e1ea) SHA1(3230e9b643fad773e61ab8ce09c0cd7d4d0558e3) )

	ROM_REGION( 0x600000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("gd_bg1-d.415", 0x000000, 0x200000, CRC(6a68054c) SHA1(5cbfc4ac90045f1401c2dda7a51936558c9de07e) )
	ROM_LOAD24_BYTE("gd_bg1-p.410", 0x000002, 0x100000, CRC(3400794a) SHA1(719808f7442bac612cefd7b7fffcd665e6337ad0) )
	ROM_LOAD24_WORD("gd_bg2-d.416", 0x300000, 0x200000, CRC(61cd2991) SHA1(bb608e3948bf9ea35b5e1615d2ba6858d029dcbe) )
	ROM_LOAD24_BYTE("gd_bg2-p.49",  0x300002, 0x100000, CRC(502d5799) SHA1(c3a0e1a4f5a7b35572ae1ff31315da4ed08aa2fe) )

	ROM_REGION( 0xc00000, "gfx3", 0)	/* sprites */
	ROM_LOAD("gd_obj-1.322", 0x000000, 0x400000, CRC(59d86c99) SHA1(d3c9241e7b51fe21f8351051b063f91dc69bf905) )
	ROM_LOAD("gd_obj-2.324", 0x400000, 0x400000, CRC(1ceb0b6f) SHA1(97225a9b3e7be18080aa52f6570af2cce8f25c06) )
	ROM_LOAD("gd_obj-3.323", 0x800000, 0x400000, CRC(36e93234) SHA1(51917a80b7da5c32a9434a1076fc2916d62e6a3e) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("gd_pcm.217", 0x000000, 0x200000, CRC(31253ad7) SHA1(c81c8d50f8f287f5cbfaec77b30d969b01ce11a9) )
	ROM_LOAD("gd_8.216",   0x200000, 0x080000, CRC(f88cb6e4) SHA1(fb35b41307b490d5d08e4b8a70f8ff4ce2ca8105) )
ROM_END

ROM_START( rdftu )
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("rdftu_gd_1.211", 0x000000, 0x80000, CRC(47810c48) SHA1(8dc8848d3e7467ea887c50fd5675fba2cc741121) )
	ROM_LOAD32_BYTE("rdftu_gd_2.212", 0x000001, 0x80000, CRC(13911750) SHA1(8899accb059ed84170924750bb39ae7383ebd959) )
	ROM_LOAD32_BYTE("rdftu_gd_3.210", 0x000002, 0x80000, CRC(10761b03) SHA1(e67db2e7c2176987419158fc4cee00fd9b99d03f) )
	ROM_LOAD32_BYTE("rdftu_gd_4.29",  0x000003, 0x80000, CRC(e5a3f01d) SHA1(5ca338f85a020d43d2618f88e798a076d13a5c7f) )

	ROM_REGION( 0x30000, "gfx1", 0)	/* text layer roms */
	ROM_LOAD24_BYTE("gd_5.423", 0x000000, 0x10000, CRC(8f8d4e14) SHA1(06c803975767ae98f40ba7ac5764a5bc8baa3a30) )
	ROM_LOAD24_BYTE("gd_6.424", 0x000001, 0x10000, CRC(6ac64968) SHA1(ec395205c24c4f864a1f805bb0d4641562d4faa9) )
	ROM_LOAD24_BYTE("gd_7.48",  0x000002, 0x10000, CRC(4d87e1ea) SHA1(3230e9b643fad773e61ab8ce09c0cd7d4d0558e3) )

	ROM_REGION( 0x600000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("gd_bg1-d.415", 0x000000, 0x200000, CRC(6a68054c) SHA1(5cbfc4ac90045f1401c2dda7a51936558c9de07e) )
	ROM_LOAD24_BYTE("gd_bg1-p.410", 0x000002, 0x100000, CRC(3400794a) SHA1(719808f7442bac612cefd7b7fffcd665e6337ad0) )
	ROM_LOAD24_WORD("gd_bg2-d.416", 0x300000, 0x200000, CRC(61cd2991) SHA1(bb608e3948bf9ea35b5e1615d2ba6858d029dcbe) )
	ROM_LOAD24_BYTE("gd_bg2-p.49",  0x300002, 0x100000, CRC(502d5799) SHA1(c3a0e1a4f5a7b35572ae1ff31315da4ed08aa2fe) )

	ROM_REGION( 0xc00000, "gfx3", 0)	/* sprites */
	ROM_LOAD("gd_obj-1.322", 0x000000, 0x400000, CRC(59d86c99) SHA1(d3c9241e7b51fe21f8351051b063f91dc69bf905) )
	ROM_LOAD("gd_obj-2.324", 0x400000, 0x400000, CRC(1ceb0b6f) SHA1(97225a9b3e7be18080aa52f6570af2cce8f25c06) )
	ROM_LOAD("gd_obj-3.323", 0x800000, 0x400000, CRC(36e93234) SHA1(51917a80b7da5c32a9434a1076fc2916d62e6a3e) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("gd_pcm.217", 0x000000, 0x200000, CRC(31253ad7) SHA1(c81c8d50f8f287f5cbfaec77b30d969b01ce11a9) )
	ROM_LOAD("gd_8.216",   0x200000, 0x080000, CRC(f88cb6e4) SHA1(fb35b41307b490d5d08e4b8a70f8ff4ce2ca8105) )
ROM_END


ROM_START( rdftj )
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("rf1.bin", 0x000000, 0x80000, CRC(46861b75) SHA1(079c589c490d49f7ec97a7e68c5b6e7e37872827) )
	ROM_LOAD32_BYTE("rf2.bin", 0x000001, 0x80000, CRC(6388ed11) SHA1(aebbccfb0f704cdceb45ea71216275dd83880e15) )
	ROM_LOAD32_BYTE("rf3.bin", 0x000002, 0x80000, CRC(beafcd24) SHA1(2dbc47ecef6f898a371a841df2c72151da9c5a8d) )
	ROM_LOAD32_BYTE("rf4.bin", 0x000003, 0x80000, CRC(5236f45f) SHA1(8b05d977d3d07796007a00a52d2396475dc2f7dc) )

	ROM_REGION( 0x30000, "gfx1", 0)	/* text layer roms */
	ROM_LOAD24_BYTE("gd_5.423", 0x000000, 0x10000, CRC(8f8d4e14) SHA1(06c803975767ae98f40ba7ac5764a5bc8baa3a30) )
	ROM_LOAD24_BYTE("gd_6.424", 0x000001, 0x10000, CRC(6ac64968) SHA1(ec395205c24c4f864a1f805bb0d4641562d4faa9) )
	ROM_LOAD24_BYTE("gd_7.48",  0x000002, 0x10000, CRC(4d87e1ea) SHA1(3230e9b643fad773e61ab8ce09c0cd7d4d0558e3) )

	ROM_REGION( 0x600000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("gd_bg1-d.415", 0x000000, 0x200000, CRC(6a68054c) SHA1(5cbfc4ac90045f1401c2dda7a51936558c9de07e) )
	ROM_LOAD24_BYTE("gd_bg1-p.410", 0x000002, 0x100000, CRC(3400794a) SHA1(719808f7442bac612cefd7b7fffcd665e6337ad0) )
	ROM_LOAD24_WORD("gd_bg2-d.416", 0x300000, 0x200000, CRC(61cd2991) SHA1(bb608e3948bf9ea35b5e1615d2ba6858d029dcbe) )
	ROM_LOAD24_BYTE("gd_bg2-p.49",  0x300002, 0x100000, CRC(502d5799) SHA1(c3a0e1a4f5a7b35572ae1ff31315da4ed08aa2fe) )

	ROM_REGION( 0xc00000, "gfx3", 0)	/* sprites */
	ROM_LOAD("gd_obj-1.322", 0x000000, 0x400000, CRC(59d86c99) SHA1(d3c9241e7b51fe21f8351051b063f91dc69bf905) )
	ROM_LOAD("gd_obj-2.324", 0x400000, 0x400000, CRC(1ceb0b6f) SHA1(97225a9b3e7be18080aa52f6570af2cce8f25c06) )
	ROM_LOAD("gd_obj-3.323", 0x800000, 0x400000, CRC(36e93234) SHA1(51917a80b7da5c32a9434a1076fc2916d62e6a3e) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("gd_pcm.217", 0x000000, 0x200000, CRC(31253ad7) SHA1(c81c8d50f8f287f5cbfaec77b30d969b01ce11a9) )
	ROM_LOAD("gd_8.216",   0x200000, 0x080000, CRC(f88cb6e4) SHA1(fb35b41307b490d5d08e4b8a70f8ff4ce2ca8105) )
ROM_END

ROM_START( rdftau )
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("1.u0211", 0x000000, 0x80000, CRC(6339c60d) SHA1(871d5bc9fc695651ceb6fcfdab32084320fe239d) )
	ROM_LOAD32_BYTE("2.u0212", 0x000001, 0x80000, CRC(a88bda02) SHA1(27dc720d28f56cf443a4eb0bbaaf4bf3b194056d) )
	ROM_LOAD32_BYTE("3.u0210", 0x000002, 0x80000, CRC(a73e337e) SHA1(93323875c676f38eca3298fcf4a34911db2d78a8) )
	ROM_LOAD32_BYTE("4.u029",  0x000003, 0x80000, CRC(8cc628f0) SHA1(7534eae8a1ea461adad483002b3cecf132e0e325) )

	ROM_REGION( 0x30000, "gfx1", 0)	/* text layer roms */
	ROM_LOAD24_BYTE("gd_5.423", 0x000000, 0x10000, CRC(8f8d4e14) SHA1(06c803975767ae98f40ba7ac5764a5bc8baa3a30) )
	ROM_LOAD24_BYTE("gd_6.424", 0x000001, 0x10000, CRC(6ac64968) SHA1(ec395205c24c4f864a1f805bb0d4641562d4faa9) )
	ROM_LOAD24_BYTE("gd_7.48",  0x000002, 0x10000, CRC(4d87e1ea) SHA1(3230e9b643fad773e61ab8ce09c0cd7d4d0558e3) )

	ROM_REGION( 0x600000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("gd_bg1-d.415", 0x000000, 0x200000, CRC(6a68054c) SHA1(5cbfc4ac90045f1401c2dda7a51936558c9de07e) )
	ROM_LOAD24_BYTE("gd_bg1-p.410", 0x000002, 0x100000, CRC(3400794a) SHA1(719808f7442bac612cefd7b7fffcd665e6337ad0) )
	ROM_LOAD24_WORD("gd_bg2-d.416", 0x300000, 0x200000, CRC(61cd2991) SHA1(bb608e3948bf9ea35b5e1615d2ba6858d029dcbe) )
	ROM_LOAD24_BYTE("gd_bg2-p.49",  0x300002, 0x100000, CRC(502d5799) SHA1(c3a0e1a4f5a7b35572ae1ff31315da4ed08aa2fe) )

	ROM_REGION( 0xc00000, "gfx3", 0)	/* sprites */
	ROM_LOAD("gd_obj-1.322", 0x000000, 0x400000, CRC(59d86c99) SHA1(d3c9241e7b51fe21f8351051b063f91dc69bf905) )
	ROM_LOAD("gd_obj-2.324", 0x400000, 0x400000, CRC(1ceb0b6f) SHA1(97225a9b3e7be18080aa52f6570af2cce8f25c06) )
	ROM_LOAD("gd_obj-3.323", 0x800000, 0x400000, CRC(36e93234) SHA1(51917a80b7da5c32a9434a1076fc2916d62e6a3e) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("gd_pcm.217", 0x000000, 0x200000, CRC(31253ad7) SHA1(c81c8d50f8f287f5cbfaec77b30d969b01ce11a9) )
	ROM_LOAD("gd_8.216",   0x200000, 0x080000, CRC(f88cb6e4) SHA1(fb35b41307b490d5d08e4b8a70f8ff4ce2ca8105) )
ROM_END

ROM_START( rdftdi )
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("u211.bin", 0x000000, 0x80000, CRC(fc0e2885) SHA1(79621155d992d504e993bd3ee0d6ff3903bd5415) )
	ROM_LOAD32_BYTE("u212.bin", 0x000001, 0x80000, CRC(58ccb10c) SHA1(0cce4057bfada78121d9586574b98d46cdd7dd46) )
	ROM_LOAD32_BYTE("u210.bin", 0x000002, 0x80000, CRC(47fc3c96) SHA1(7378f8caa847f89f235b5be6779118721076873b) )
	ROM_LOAD32_BYTE("u29.bin",  0x000003, 0x80000, CRC(271bdd4b) SHA1(0a805568cbd6a9c18bdb755a41972ff6bba9e6eb) )

	ROM_REGION( 0x30000, "gfx1", 0)	/* text layer roms */
	ROM_LOAD24_BYTE("gd_5.423", 0x000000, 0x10000, CRC(8f8d4e14) SHA1(06c803975767ae98f40ba7ac5764a5bc8baa3a30) )
	ROM_LOAD24_BYTE("gd_6.424", 0x000001, 0x10000, CRC(6ac64968) SHA1(ec395205c24c4f864a1f805bb0d4641562d4faa9) )
	ROM_LOAD24_BYTE("gd_7.48",  0x000002, 0x10000, CRC(4d87e1ea) SHA1(3230e9b643fad773e61ab8ce09c0cd7d4d0558e3) )

	ROM_REGION( 0x600000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("gd_bg1-d.415", 0x000000, 0x200000, CRC(6a68054c) SHA1(5cbfc4ac90045f1401c2dda7a51936558c9de07e) )
	ROM_LOAD24_BYTE("gd_bg1-p.410", 0x000002, 0x100000, CRC(3400794a) SHA1(719808f7442bac612cefd7b7fffcd665e6337ad0) )
	ROM_LOAD24_WORD("gd_bg2-d.416", 0x300000, 0x200000, CRC(61cd2991) SHA1(bb608e3948bf9ea35b5e1615d2ba6858d029dcbe) )
	ROM_LOAD24_BYTE("gd_bg2-p.49",  0x300002, 0x100000, CRC(502d5799) SHA1(c3a0e1a4f5a7b35572ae1ff31315da4ed08aa2fe) )

	ROM_REGION( 0xc00000, "gfx3", 0)	/* sprites */
	ROM_LOAD("gd_obj-1.322", 0x000000, 0x400000, CRC(59d86c99) SHA1(d3c9241e7b51fe21f8351051b063f91dc69bf905) )
	ROM_LOAD("gd_obj-2.324", 0x400000, 0x400000, CRC(1ceb0b6f) SHA1(97225a9b3e7be18080aa52f6570af2cce8f25c06) )
	ROM_LOAD("gd_obj-3.323", 0x800000, 0x400000, CRC(36e93234) SHA1(51917a80b7da5c32a9434a1076fc2916d62e6a3e) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("gd_pcm.217", 0x000000, 0x200000, CRC(31253ad7) SHA1(c81c8d50f8f287f5cbfaec77b30d969b01ce11a9) )
	ROM_LOAD("gd_8.216",   0x200000, 0x080000, CRC(f88cb6e4) SHA1(fb35b41307b490d5d08e4b8a70f8ff4ce2ca8105) )
ROM_END

ROM_START( rdftit )
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("seibu1",   0x000000, 0x80000, CRC(de0c3e3c) SHA1(b00225bad282e46b5825608f76eea6670bfe5527) )
	ROM_LOAD32_BYTE("u212.bin", 0x000001, 0x80000, CRC(58ccb10c) SHA1(0cce4057bfada78121d9586574b98d46cdd7dd46) )
	ROM_LOAD32_BYTE("u210.bin", 0x000002, 0x80000, CRC(47fc3c96) SHA1(7378f8caa847f89f235b5be6779118721076873b) )
	ROM_LOAD32_BYTE("u29.bin",  0x000003, 0x80000, CRC(271bdd4b) SHA1(0a805568cbd6a9c18bdb755a41972ff6bba9e6eb) )

	ROM_REGION( 0x30000, "gfx1", 0)	/* text layer roms */
	ROM_LOAD24_BYTE("gd_5.423", 0x000000, 0x10000, CRC(8f8d4e14) SHA1(06c803975767ae98f40ba7ac5764a5bc8baa3a30) )
	ROM_LOAD24_BYTE("gd_6.424", 0x000001, 0x10000, CRC(6ac64968) SHA1(ec395205c24c4f864a1f805bb0d4641562d4faa9) )
	ROM_LOAD24_BYTE("gd_7.48",  0x000002, 0x10000, CRC(4d87e1ea) SHA1(3230e9b643fad773e61ab8ce09c0cd7d4d0558e3) )

	ROM_REGION( 0x600000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("gd_bg1-d.415", 0x000000, 0x200000, CRC(6a68054c) SHA1(5cbfc4ac90045f1401c2dda7a51936558c9de07e) )
	ROM_LOAD24_BYTE("gd_bg1-p.410", 0x000002, 0x100000, CRC(3400794a) SHA1(719808f7442bac612cefd7b7fffcd665e6337ad0) )
	ROM_LOAD24_WORD("gd_bg2-d.416", 0x300000, 0x200000, CRC(61cd2991) SHA1(bb608e3948bf9ea35b5e1615d2ba6858d029dcbe) )
	ROM_LOAD24_BYTE("gd_bg2-p.49",  0x300002, 0x100000, CRC(502d5799) SHA1(c3a0e1a4f5a7b35572ae1ff31315da4ed08aa2fe) )

	ROM_REGION( 0xc00000, "gfx3", 0)	/* sprites */
	ROM_LOAD("gd_obj-1.322", 0x000000, 0x400000, CRC(59d86c99) SHA1(d3c9241e7b51fe21f8351051b063f91dc69bf905) )
	ROM_LOAD("gd_obj-2.324", 0x400000, 0x400000, CRC(1ceb0b6f) SHA1(97225a9b3e7be18080aa52f6570af2cce8f25c06) )
	ROM_LOAD("gd_obj-3.323", 0x800000, 0x400000, CRC(36e93234) SHA1(51917a80b7da5c32a9434a1076fc2916d62e6a3e) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("gd_pcm.217", 0x000000, 0x200000, CRC(31253ad7) SHA1(c81c8d50f8f287f5cbfaec77b30d969b01ce11a9) )
	ROM_LOAD("gd_8.216",   0x200000, 0x080000, CRC(f88cb6e4) SHA1(fb35b41307b490d5d08e4b8a70f8ff4ce2ca8105) )
ROM_END

ROM_START( rdfta )
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("seibu1a",  0x000000, 0x80000, CRC(c3bb2e58) SHA1(399ac4b387ba38f5fdad5c4172b2d3baeafd8773) )
	ROM_LOAD32_BYTE("u212.bin", 0x000001, 0x80000, CRC(58ccb10c) SHA1(0cce4057bfada78121d9586574b98d46cdd7dd46) )
	ROM_LOAD32_BYTE("u210.bin", 0x000002, 0x80000, CRC(47fc3c96) SHA1(7378f8caa847f89f235b5be6779118721076873b) )
	ROM_LOAD32_BYTE("u29.bin",  0x000003, 0x80000, CRC(271bdd4b) SHA1(0a805568cbd6a9c18bdb755a41972ff6bba9e6eb) )

	ROM_REGION( 0x30000, "gfx1", 0)	/* text layer roms */
	ROM_LOAD24_BYTE("gd_5.423", 0x000000, 0x10000, CRC(8f8d4e14) SHA1(06c803975767ae98f40ba7ac5764a5bc8baa3a30) )
	ROM_LOAD24_BYTE("gd_6.424", 0x000001, 0x10000, CRC(6ac64968) SHA1(ec395205c24c4f864a1f805bb0d4641562d4faa9) )
	ROM_LOAD24_BYTE("gd_7.48",  0x000002, 0x10000, CRC(4d87e1ea) SHA1(3230e9b643fad773e61ab8ce09c0cd7d4d0558e3) )

	ROM_REGION( 0x600000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("gd_bg1-d.415", 0x000000, 0x200000, CRC(6a68054c) SHA1(5cbfc4ac90045f1401c2dda7a51936558c9de07e) )
	ROM_LOAD24_BYTE("gd_bg1-p.410", 0x000002, 0x100000, CRC(3400794a) SHA1(719808f7442bac612cefd7b7fffcd665e6337ad0) )
	ROM_LOAD24_WORD("gd_bg2-d.416", 0x300000, 0x200000, CRC(61cd2991) SHA1(bb608e3948bf9ea35b5e1615d2ba6858d029dcbe) )
	ROM_LOAD24_BYTE("gd_bg2-p.49",  0x300002, 0x100000, CRC(502d5799) SHA1(c3a0e1a4f5a7b35572ae1ff31315da4ed08aa2fe) )

	ROM_REGION( 0xc00000, "gfx3", 0)	/* sprites */
	ROM_LOAD("gd_obj-1.322", 0x000000, 0x400000, CRC(59d86c99) SHA1(d3c9241e7b51fe21f8351051b063f91dc69bf905) )
	ROM_LOAD("gd_obj-2.324", 0x400000, 0x400000, CRC(1ceb0b6f) SHA1(97225a9b3e7be18080aa52f6570af2cce8f25c06) )
	ROM_LOAD("gd_obj-3.323", 0x800000, 0x400000, CRC(36e93234) SHA1(51917a80b7da5c32a9434a1076fc2916d62e6a3e) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("gd_pcm.217", 0x000000, 0x200000, CRC(31253ad7) SHA1(c81c8d50f8f287f5cbfaec77b30d969b01ce11a9) )
	ROM_LOAD("gd_8.216",   0x200000, 0x080000, CRC(f88cb6e4) SHA1(fb35b41307b490d5d08e4b8a70f8ff4ce2ca8105) )
ROM_END

ROM_START( rdft2us )	/* Single board version SXX2F */
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("prg0.u0259", 0x000000, 0x80000, CRC(ff3eeec1) SHA1(88c1741e4936db9a5b13e562061b0f1cc6fa6b36) )
	ROM_LOAD32_BYTE("prg1.u0258", 0x000001, 0x80000, CRC(e2cf77d6) SHA1(173cc0e304c9dadea4ed0812ebb64c6c83549912) )
	ROM_LOAD32_BYTE("prg2.u0265", 0x000002, 0x80000, CRC(cae87e1f) SHA1(e460aad693eb2702ae11f758b11d37f852d00790) )
	ROM_LOAD32_BYTE("prg3.u0264", 0x000003, 0x80000, CRC(83f4fb5f) SHA1(73f58daa1aae0c4978db409cedd736fb49b15f1c) )

	ROM_REGION( 0x30000, "gfx1", 0)
	ROM_LOAD24_BYTE("fix0.u0524", 0x000001, 0x10000, CRC(6fdf4cf6) SHA1(7e9d4a49e829dfdc373c0f5acfbe8c7a91ac115b) )
	ROM_LOAD24_BYTE("fix1.u0518", 0x000000, 0x10000, CRC(69b7899b) SHA1(d3cacd4ef4d2c95d803403101beb9d4be75fae61) )
	ROM_LOAD24_BYTE("fixp.u0514", 0x000002, 0x10000, CRC(99a5fece) SHA1(44ae95d650ed6e00202d3438f5f91a5e52e319cb) )

	ROM_REGION( 0xc00000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0535", 0x000000, 0x400000, CRC(6143f576) SHA1(c034923d0663d9ef24357a03098b8cb81dbab9f8) )
	ROM_LOAD24_BYTE("bg-1p.u0537", 0x000002, 0x200000, CRC(55e64ef7) SHA1(aae991268948d07342ee8ba1b3761bd180aab8ec) )
	ROM_LOAD24_WORD("bg-2d.u0536", 0x600000, 0x400000, CRC(c607a444) SHA1(dc1aa96a42e9394ca6036359670a4ec6f830c96d) )
	ROM_LOAD24_BYTE("bg-2p.u0538", 0x600002, 0x200000, CRC(f0830248) SHA1(6075df96b49e70d2243fef691e096119e7a4d044) )

	ROM_REGION( 0x1200000, "gfx3", 0)	/* sprites */
	ROM_LOAD("obj3.u075",  0x0000000, 0x400000, CRC(e08f42dc) SHA1(5188d71d4355eaf43ea8893b4cfc4fe80cc24f41) )
	ROM_LOAD("obj3b.u078", 0x0400000, 0x200000, CRC(1b6a523c) SHA1(99a420dbc8e22e7832ccda7cec9fa661a2a2687a) )
	ROM_LOAD("obj2.u074",  0x0600000, 0x400000, CRC(7aeadd8e) SHA1(47103c0579240c5b1add4d0b164eaf76f5fa97f0) )
	ROM_LOAD("obj2b.u077", 0x0a00000, 0x200000, CRC(5d790a5d) SHA1(1ed5d4ad4c9a7e505ce35dcc90d184c26ce891dc) )
	ROM_LOAD("obj1.u073",  0x0c00000, 0x400000, CRC(c2c50f02) SHA1(b81397b5800c6d49f58b7ac7ff6eac56da3c5257) )
	ROM_LOAD("obj1b.u076", 0x1000000, 0x200000, CRC(5259321f) SHA1(3c70c1147e49f81371d0f60f7108d9718d56faf4) )

	ROM_REGION(0x40000, "soundcpu", 0)		/* 256k for the Z80 */
	ROM_LOAD("zprg.u091", 0x000000, 0x20000, CRC(cc543c4f) SHA1(6e5c93fd3d21c594571b071d4a830211e1f162b2) )

	ROM_REGION(0x280000, "ymf", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("pcm.u0103",    0x000000, 0x200000, CRC(2edc30b5) SHA1(c25d690d633657fc3687636b9070f36bd305ae06) )
	ROM_LOAD("sound1.u0107", 0x200000, 0x080000, CRC(20384b0e) SHA1(9c5d725418543df740f9145974ed6ffbbabee1d0) ) /* Different sound1 then SPI carts */
ROM_END

ROM_START( rdft2 ) /* SPI Cart, Europe */
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("prg0.tun", 0x000000, 0x80000, CRC(3cb3fdca) SHA1(4b472dfd65c7bbbcb92a295aa73b0fa70581455b) )
	ROM_LOAD32_BYTE("prg1.bin", 0x000001, 0x80000, CRC(cab55d88) SHA1(246e13880d34b6c7c3f4ab5e18fa8a0547c03d9d) )
	ROM_LOAD32_BYTE("prg2.bin", 0x000002, 0x80000, CRC(83758b0e) SHA1(63adb2d09e7bd7dba47a55b3b579d543dfb553e3) )
	ROM_LOAD32_BYTE("prg3.bin", 0x000003, 0x80000, CRC(084fb5e4) SHA1(588bfe091662b88f02f528181a2f1d9c67c7b280) )

	ROM_REGION( 0x30000, "gfx1", 0)
	ROM_LOAD24_BYTE("fix0.u0524", 0x000001, 0x10000, CRC(6fdf4cf6) SHA1(7e9d4a49e829dfdc373c0f5acfbe8c7a91ac115b) )
	ROM_LOAD24_BYTE("fix1.u0518", 0x000000, 0x10000, CRC(69b7899b) SHA1(d3cacd4ef4d2c95d803403101beb9d4be75fae61) )
	ROM_LOAD24_BYTE("fixp.u0514", 0x000002, 0x10000, CRC(99a5fece) SHA1(44ae95d650ed6e00202d3438f5f91a5e52e319cb) )

	ROM_REGION( 0xc00000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0535", 0x000000, 0x400000, CRC(6143f576) SHA1(c034923d0663d9ef24357a03098b8cb81dbab9f8) )
	ROM_LOAD24_BYTE("bg-1p.u0537", 0x000002, 0x200000, CRC(55e64ef7) SHA1(aae991268948d07342ee8ba1b3761bd180aab8ec) )
	ROM_LOAD24_WORD("bg-2d.u0536", 0x600000, 0x400000, CRC(c607a444) SHA1(dc1aa96a42e9394ca6036359670a4ec6f830c96d) )
	ROM_LOAD24_BYTE("bg-2p.u0538", 0x600002, 0x200000, CRC(f0830248) SHA1(6075df96b49e70d2243fef691e096119e7a4d044) )

	ROM_REGION( 0x1200000, "gfx3", 0)	/* sprites */
	ROM_LOAD("obj3.u0434",  0x0000000, 0x400000, CRC(e08f42dc) SHA1(5188d71d4355eaf43ea8893b4cfc4fe80cc24f41) )
	ROM_LOAD("obj3b.u0433", 0x0400000, 0x200000, CRC(1b6a523c) SHA1(99a420dbc8e22e7832ccda7cec9fa661a2a2687a) )
	ROM_LOAD("obj2.u0431",  0x0600000, 0x400000, CRC(7aeadd8e) SHA1(47103c0579240c5b1add4d0b164eaf76f5fa97f0) )
	ROM_LOAD("obj2b.u0432", 0x0a00000, 0x200000, CRC(5d790a5d) SHA1(1ed5d4ad4c9a7e505ce35dcc90d184c26ce891dc) )
	ROM_LOAD("obj1.u0429",  0x0c00000, 0x400000, CRC(c2c50f02) SHA1(b81397b5800c6d49f58b7ac7ff6eac56da3c5257) )
	ROM_LOAD("obj1b.u0430", 0x1000000, 0x200000, CRC(5259321f) SHA1(3c70c1147e49f81371d0f60f7108d9718d56faf4) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("pcm.u0217",    0x000000, 0x200000, CRC(2edc30b5) SHA1(c25d690d633657fc3687636b9070f36bd305ae06) )
	ROM_LOAD("sound1.u0222", 0x200000, 0x080000, CRC(b7bd3703) SHA1(6427a7e6de10d6743d6e64b984a1d1c647f5643a) )
ROM_END

ROM_START( rdft2u ) /* SPI Cart, USA */
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("1.bin", 0x000000, 0x80000, CRC(b7d6c866) SHA1(eefe63dfc641c3904dd150a10ffeb68137068725) )
	ROM_LOAD32_BYTE("2.bin", 0x000001, 0x80000, CRC(ff7747c5) SHA1(7481d0484001ff7367af56e8ea99f985cce405f2) )
	ROM_LOAD32_BYTE("3.bin", 0x000002, 0x80000, CRC(86e3d1a8) SHA1(2757cfda57c82dd0f66427caf54eb1f40e85740d) )
	ROM_LOAD32_BYTE("4.bin", 0x000003, 0x80000, CRC(2e409a76) SHA1(cf90aa14a07b5aa861f6f7cc9b1968171e532557) )

	ROM_REGION( 0x30000, "gfx1", 0)
	ROM_LOAD24_BYTE("fix0.u0524", 0x000001, 0x10000, CRC(6fdf4cf6) SHA1(7e9d4a49e829dfdc373c0f5acfbe8c7a91ac115b) )
	ROM_LOAD24_BYTE("fix1.u0518", 0x000000, 0x10000, CRC(69b7899b) SHA1(d3cacd4ef4d2c95d803403101beb9d4be75fae61) )
	ROM_LOAD24_BYTE("fixp.u0514", 0x000002, 0x10000, CRC(99a5fece) SHA1(44ae95d650ed6e00202d3438f5f91a5e52e319cb) )

	ROM_REGION( 0xc00000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0535", 0x000000, 0x400000, CRC(6143f576) SHA1(c034923d0663d9ef24357a03098b8cb81dbab9f8) )
	ROM_LOAD24_BYTE("bg-1p.u0537", 0x000002, 0x200000, CRC(55e64ef7) SHA1(aae991268948d07342ee8ba1b3761bd180aab8ec) )
	ROM_LOAD24_WORD("bg-2d.u0536", 0x600000, 0x400000, CRC(c607a444) SHA1(dc1aa96a42e9394ca6036359670a4ec6f830c96d) )
	ROM_LOAD24_BYTE("bg-2p.u0538", 0x600002, 0x200000, CRC(f0830248) SHA1(6075df96b49e70d2243fef691e096119e7a4d044) )

	ROM_REGION( 0x1200000, "gfx3", 0)	/* sprites */
	ROM_LOAD("obj3.u0434",  0x0000000, 0x400000, CRC(e08f42dc) SHA1(5188d71d4355eaf43ea8893b4cfc4fe80cc24f41) )
	ROM_LOAD("obj3b.u0433", 0x0400000, 0x200000, CRC(1b6a523c) SHA1(99a420dbc8e22e7832ccda7cec9fa661a2a2687a) )
	ROM_LOAD("obj2.u0431",  0x0600000, 0x400000, CRC(7aeadd8e) SHA1(47103c0579240c5b1add4d0b164eaf76f5fa97f0) )
	ROM_LOAD("obj2b.u0432", 0x0a00000, 0x200000, CRC(5d790a5d) SHA1(1ed5d4ad4c9a7e505ce35dcc90d184c26ce891dc) )
	ROM_LOAD("obj1.u0429",  0x0c00000, 0x400000, CRC(c2c50f02) SHA1(b81397b5800c6d49f58b7ac7ff6eac56da3c5257) )
	ROM_LOAD("obj1b.u0430", 0x1000000, 0x200000, CRC(5259321f) SHA1(3c70c1147e49f81371d0f60f7108d9718d56faf4) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("pcm.u0217",    0x000000, 0x200000, CRC(2edc30b5) SHA1(c25d690d633657fc3687636b9070f36bd305ae06) )
	ROM_LOAD("sound1.u0222", 0x200000, 0x080000, CRC(b7bd3703) SHA1(6427a7e6de10d6743d6e64b984a1d1c647f5643a) )
ROM_END

ROM_START( rdft2j ) /* SPI Cart, Japan */
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("prg0.sei", 0x000000, 0x80000, CRC(a60c4e7c) SHA1(7789b029d0ac084c7e5e662a7168edaed8f11633) )
	ROM_LOAD32_BYTE("prg1.bin", 0x000001, 0x80000, CRC(cab55d88) SHA1(246e13880d34b6c7c3f4ab5e18fa8a0547c03d9d) )
	ROM_LOAD32_BYTE("prg2.bin", 0x000002, 0x80000, CRC(83758b0e) SHA1(63adb2d09e7bd7dba47a55b3b579d543dfb553e3) )
	ROM_LOAD32_BYTE("prg3.bin", 0x000003, 0x80000, CRC(084fb5e4) SHA1(588bfe091662b88f02f528181a2f1d9c67c7b280) )

	ROM_REGION( 0x30000, "gfx1", 0)
	ROM_LOAD24_BYTE("fix0.u0524", 0x000001, 0x10000, CRC(6fdf4cf6) SHA1(7e9d4a49e829dfdc373c0f5acfbe8c7a91ac115b) )
	ROM_LOAD24_BYTE("fix1.u0518", 0x000000, 0x10000, CRC(69b7899b) SHA1(d3cacd4ef4d2c95d803403101beb9d4be75fae61) )
	ROM_LOAD24_BYTE("fixp.u0514", 0x000002, 0x10000, CRC(99a5fece) SHA1(44ae95d650ed6e00202d3438f5f91a5e52e319cb) )

	ROM_REGION( 0xc00000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0535", 0x000000, 0x400000, CRC(6143f576) SHA1(c034923d0663d9ef24357a03098b8cb81dbab9f8) )
	ROM_LOAD24_BYTE("bg-1p.u0537", 0x000002, 0x200000, CRC(55e64ef7) SHA1(aae991268948d07342ee8ba1b3761bd180aab8ec) )
	ROM_LOAD24_WORD("bg-2d.u0536", 0x600000, 0x400000, CRC(c607a444) SHA1(dc1aa96a42e9394ca6036359670a4ec6f830c96d) )
	ROM_LOAD24_BYTE("bg-2p.u0538", 0x600002, 0x200000, CRC(f0830248) SHA1(6075df96b49e70d2243fef691e096119e7a4d044) )

	ROM_REGION( 0x1200000, "gfx3", 0)	/* sprites */
	ROM_LOAD("obj3.u0434",  0x0000000, 0x400000, CRC(e08f42dc) SHA1(5188d71d4355eaf43ea8893b4cfc4fe80cc24f41) )
	ROM_LOAD("obj3b.u0433", 0x0400000, 0x200000, CRC(1b6a523c) SHA1(99a420dbc8e22e7832ccda7cec9fa661a2a2687a) )
	ROM_LOAD("obj2.u0431",  0x0600000, 0x400000, CRC(7aeadd8e) SHA1(47103c0579240c5b1add4d0b164eaf76f5fa97f0) )
	ROM_LOAD("obj2b.u0432", 0x0a00000, 0x200000, CRC(5d790a5d) SHA1(1ed5d4ad4c9a7e505ce35dcc90d184c26ce891dc) )
	ROM_LOAD("obj1.u0429",  0x0c00000, 0x400000, CRC(c2c50f02) SHA1(b81397b5800c6d49f58b7ac7ff6eac56da3c5257) )
	ROM_LOAD("obj1b.u0430", 0x1000000, 0x200000, CRC(5259321f) SHA1(3c70c1147e49f81371d0f60f7108d9718d56faf4) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("pcm.u0217",    0x000000, 0x200000, CRC(2edc30b5) SHA1(c25d690d633657fc3687636b9070f36bd305ae06) )
	ROM_LOAD("sound1.u0222", 0x200000, 0x080000, CRC(b7bd3703) SHA1(6427a7e6de10d6743d6e64b984a1d1c647f5643a) )
ROM_END

ROM_START( rdft2j2 ) /* SPI Cart, Japan */
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("rf2.1",    0x000000, 0x80000, CRC(391d5057) SHA1(a1849142cbf7344ac1279781597e27b3b8ae6127) )
	ROM_LOAD32_BYTE("rf2_2.bin", 0x000001, 0x80000, CRC(ec73a767) SHA1(83f3905afe49401793c0ea0193cb31d3ba1e1739) )
	ROM_LOAD32_BYTE("rf2_3.bin", 0x000002, 0x80000, CRC(e66243b2) SHA1(54e67af37a4586fd1afc79085ed433d599e1bb87) )
	ROM_LOAD32_BYTE("rf2_4.bin", 0x000003, 0x80000, CRC(92b7b73e) SHA1(128649b2a6a0616113bd0f9846fb6cf814ae326d) )

	ROM_REGION( 0x30000, "gfx1", 0)
	ROM_LOAD24_BYTE("rf2_5.bin", 0x000001, 0x10000, CRC(377cac2f) SHA1(f7c9323d79b77f6c8c02ba2c6cdca127d6e5cb5c) )
	ROM_LOAD24_BYTE("rf2_6.bin", 0x000000, 0x10000, CRC(42bd5372) SHA1(c38df85b25070db9640eac541f71c0511bab0c98) )
	ROM_LOAD24_BYTE("rf2_7.bin", 0x000002, 0x10000, CRC(1efaac7e) SHA1(8252af56dcb7a6306dc3422070176778e3c511c2) )

	ROM_REGION( 0xc00000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0535", 0x000000, 0x400000, CRC(6143f576) SHA1(c034923d0663d9ef24357a03098b8cb81dbab9f8) )
	ROM_LOAD24_BYTE("bg-1p.u0537", 0x000002, 0x200000, CRC(55e64ef7) SHA1(aae991268948d07342ee8ba1b3761bd180aab8ec) )
	ROM_LOAD24_WORD("bg-2d.u0536", 0x600000, 0x400000, CRC(c607a444) SHA1(dc1aa96a42e9394ca6036359670a4ec6f830c96d) )
	ROM_LOAD24_BYTE("bg-2p.u0538", 0x600002, 0x200000, CRC(f0830248) SHA1(6075df96b49e70d2243fef691e096119e7a4d044) )

	ROM_REGION( 0x1200000, "gfx3", 0)	/* sprites */
	ROM_LOAD("obj3.u0434",  0x0000000, 0x400000, CRC(e08f42dc) SHA1(5188d71d4355eaf43ea8893b4cfc4fe80cc24f41) )
	ROM_LOAD("obj3b.u0433", 0x0400000, 0x200000, CRC(1b6a523c) SHA1(99a420dbc8e22e7832ccda7cec9fa661a2a2687a) )
	ROM_LOAD("obj2.u0431",  0x0600000, 0x400000, CRC(7aeadd8e) SHA1(47103c0579240c5b1add4d0b164eaf76f5fa97f0) )
	ROM_LOAD("obj2b.u0432", 0x0a00000, 0x200000, CRC(5d790a5d) SHA1(1ed5d4ad4c9a7e505ce35dcc90d184c26ce891dc) )
	ROM_LOAD("obj1.u0429",  0x0c00000, 0x400000, CRC(c2c50f02) SHA1(b81397b5800c6d49f58b7ac7ff6eac56da3c5257) )
	ROM_LOAD("obj1b.u0430", 0x1000000, 0x200000, CRC(5259321f) SHA1(3c70c1147e49f81371d0f60f7108d9718d56faf4) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("pcm.u0217",    0x000000, 0x200000, CRC(2edc30b5) SHA1(c25d690d633657fc3687636b9070f36bd305ae06) )
	ROM_LOAD("sound1.u0222", 0x200000, 0x080000, CRC(b7bd3703) SHA1(6427a7e6de10d6743d6e64b984a1d1c647f5643a) )
ROM_END

ROM_START( rdft2a ) /* SPI Cart, Asia (Metrotainment license) */
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("prg0.met", 0x000000, 0x80000, CRC(046b3f0e) SHA1(033898f658d6007f891828835734422d4af36321) )
	ROM_LOAD32_BYTE("prg1.bin", 0x000001, 0x80000, CRC(cab55d88) SHA1(246e13880d34b6c7c3f4ab5e18fa8a0547c03d9d) )
	ROM_LOAD32_BYTE("prg2.bin", 0x000002, 0x80000, CRC(83758b0e) SHA1(63adb2d09e7bd7dba47a55b3b579d543dfb553e3) )
	ROM_LOAD32_BYTE("prg3.bin", 0x000003, 0x80000, CRC(084fb5e4) SHA1(588bfe091662b88f02f528181a2f1d9c67c7b280) )

	ROM_REGION( 0x30000, "gfx1", 0)
	ROM_LOAD24_BYTE("fix0.u0524", 0x000001, 0x10000, CRC(6fdf4cf6) SHA1(7e9d4a49e829dfdc373c0f5acfbe8c7a91ac115b) )
	ROM_LOAD24_BYTE("fix1.u0518", 0x000000, 0x10000, CRC(69b7899b) SHA1(d3cacd4ef4d2c95d803403101beb9d4be75fae61) )
	ROM_LOAD24_BYTE("fixp.u0514", 0x000002, 0x10000, CRC(99a5fece) SHA1(44ae95d650ed6e00202d3438f5f91a5e52e319cb) )

	ROM_REGION( 0xc00000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0535", 0x000000, 0x400000, CRC(6143f576) SHA1(c034923d0663d9ef24357a03098b8cb81dbab9f8) )
	ROM_LOAD24_BYTE("bg-1p.u0537", 0x000002, 0x200000, CRC(55e64ef7) SHA1(aae991268948d07342ee8ba1b3761bd180aab8ec) )
	ROM_LOAD24_WORD("bg-2d.u0536", 0x600000, 0x400000, CRC(c607a444) SHA1(dc1aa96a42e9394ca6036359670a4ec6f830c96d) )
	ROM_LOAD24_BYTE("bg-2p.u0538", 0x600002, 0x200000, CRC(f0830248) SHA1(6075df96b49e70d2243fef691e096119e7a4d044) )

	ROM_REGION( 0x1200000, "gfx3", 0)	/* sprites */
	ROM_LOAD("obj3.u0434",  0x0000000, 0x400000, CRC(e08f42dc) SHA1(5188d71d4355eaf43ea8893b4cfc4fe80cc24f41) )
	ROM_LOAD("obj3b.u0433", 0x0400000, 0x200000, CRC(1b6a523c) SHA1(99a420dbc8e22e7832ccda7cec9fa661a2a2687a) )
	ROM_LOAD("obj2.u0431",  0x0600000, 0x400000, CRC(7aeadd8e) SHA1(47103c0579240c5b1add4d0b164eaf76f5fa97f0) )
	ROM_LOAD("obj2b.u0432", 0x0a00000, 0x200000, CRC(5d790a5d) SHA1(1ed5d4ad4c9a7e505ce35dcc90d184c26ce891dc) )
	ROM_LOAD("obj1.u0429",  0x0c00000, 0x400000, CRC(c2c50f02) SHA1(b81397b5800c6d49f58b7ac7ff6eac56da3c5257) )
	ROM_LOAD("obj1b.u0430", 0x1000000, 0x200000, CRC(5259321f) SHA1(3c70c1147e49f81371d0f60f7108d9718d56faf4) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("pcm.u0217",    0x000000, 0x200000, CRC(2edc30b5) SHA1(c25d690d633657fc3687636b9070f36bd305ae06) )
	ROM_LOAD("sound1.u0222", 0x200000, 0x080000, CRC(b7bd3703) SHA1(6427a7e6de10d6743d6e64b984a1d1c647f5643a) )
ROM_END

ROM_START( rdft2a2 ) /* SPI Cart, Asia (Dream Island license) */
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("rf2_1.bin", 0x000000, 0x80000, CRC(72198410) SHA1(ca4bc858f6bf247a343b0fdae1d1a3cdabc4a3c3) )
	ROM_LOAD32_BYTE("rf2_2.bin", 0x000001, 0x80000, CRC(ec73a767) SHA1(83f3905afe49401793c0ea0193cb31d3ba1e1739) )
	ROM_LOAD32_BYTE("rf2_3.bin", 0x000002, 0x80000, CRC(e66243b2) SHA1(54e67af37a4586fd1afc79085ed433d599e1bb87) )
	ROM_LOAD32_BYTE("rf2_4.bin", 0x000003, 0x80000, CRC(92b7b73e) SHA1(128649b2a6a0616113bd0f9846fb6cf814ae326d) )

	ROM_REGION( 0x30000, "gfx1", 0)
	ROM_LOAD24_BYTE("rf2_5.bin", 0x000001, 0x10000, CRC(377cac2f) SHA1(f7c9323d79b77f6c8c02ba2c6cdca127d6e5cb5c) )
	ROM_LOAD24_BYTE("rf2_6.bin", 0x000000, 0x10000, CRC(42bd5372) SHA1(c38df85b25070db9640eac541f71c0511bab0c98) )
	ROM_LOAD24_BYTE("rf2_7.bin", 0x000002, 0x10000, CRC(1efaac7e) SHA1(8252af56dcb7a6306dc3422070176778e3c511c2) )

	ROM_REGION( 0xc00000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0535", 0x000000, 0x400000, CRC(6143f576) SHA1(c034923d0663d9ef24357a03098b8cb81dbab9f8) )
	ROM_LOAD24_BYTE("bg-1p.u0537", 0x000002, 0x200000, CRC(55e64ef7) SHA1(aae991268948d07342ee8ba1b3761bd180aab8ec) )
	ROM_LOAD24_WORD("bg-2d.u0536", 0x600000, 0x400000, CRC(c607a444) SHA1(dc1aa96a42e9394ca6036359670a4ec6f830c96d) )
	ROM_LOAD24_BYTE("bg-2p.u0538", 0x600002, 0x200000, CRC(f0830248) SHA1(6075df96b49e70d2243fef691e096119e7a4d044) )

	ROM_REGION( 0x1200000, "gfx3", 0)	/* sprites */
	ROM_LOAD("obj3.u0434",  0x0000000, 0x400000, CRC(e08f42dc) SHA1(5188d71d4355eaf43ea8893b4cfc4fe80cc24f41) )
	ROM_LOAD("obj3b.u0433", 0x0400000, 0x200000, CRC(1b6a523c) SHA1(99a420dbc8e22e7832ccda7cec9fa661a2a2687a) )
	ROM_LOAD("obj2.u0431",  0x0600000, 0x400000, CRC(7aeadd8e) SHA1(47103c0579240c5b1add4d0b164eaf76f5fa97f0) )
	ROM_LOAD("obj2b.u0432", 0x0a00000, 0x200000, CRC(5d790a5d) SHA1(1ed5d4ad4c9a7e505ce35dcc90d184c26ce891dc) )
	ROM_LOAD("obj1.u0429",  0x0c00000, 0x400000, CRC(c2c50f02) SHA1(b81397b5800c6d49f58b7ac7ff6eac56da3c5257) )
	ROM_LOAD("obj1b.u0430", 0x1000000, 0x200000, CRC(5259321f) SHA1(3c70c1147e49f81371d0f60f7108d9718d56faf4) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("pcm.u0217",    0x000000, 0x200000, CRC(2edc30b5) SHA1(c25d690d633657fc3687636b9070f36bd305ae06) )
	ROM_LOAD("sound1.u0222", 0x200000, 0x080000, CRC(b7bd3703) SHA1(6427a7e6de10d6743d6e64b984a1d1c647f5643a) )
ROM_END

ROM_START( rdft2t ) /* SPI Cart, Asia */
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("prg0", 0x000000, 0x80000, CRC(7e8c3acc) SHA1(63f4f9f7df7fa028737d9f7dfae96795cde58541) )
	ROM_LOAD32_BYTE("prg1", 0x000001, 0x80000, CRC(22cb5b68) SHA1(35f86ad7771fe9aaac3904ed34a96d0cc10ef21c) )
	ROM_LOAD32_BYTE("prg2", 0x000002, 0x80000, CRC(3eca68dd) SHA1(98378654adf055d72ae685f90e36643c9d6419d7) )
	ROM_LOAD32_BYTE("prg3", 0x000003, 0x80000, CRC(4124daa4) SHA1(42f225c0328df59ffeacc215d37010f825bf507e) )

	ROM_REGION( 0x30000, "gfx1", 0)
	ROM_LOAD24_BYTE("rf2_5.bin", 0x000001, 0x10000, CRC(377cac2f) SHA1(f7c9323d79b77f6c8c02ba2c6cdca127d6e5cb5c) )
	ROM_LOAD24_BYTE("rf2_6.bin", 0x000000, 0x10000, CRC(42bd5372) SHA1(c38df85b25070db9640eac541f71c0511bab0c98) )
	ROM_LOAD24_BYTE("rf2_7.bin", 0x000002, 0x10000, CRC(1efaac7e) SHA1(8252af56dcb7a6306dc3422070176778e3c511c2) )

	ROM_REGION( 0xc00000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0535", 0x000000, 0x400000, CRC(6143f576) SHA1(c034923d0663d9ef24357a03098b8cb81dbab9f8) )
	ROM_LOAD24_BYTE("bg-1p.u0537", 0x000002, 0x200000, CRC(55e64ef7) SHA1(aae991268948d07342ee8ba1b3761bd180aab8ec) )
	ROM_LOAD24_WORD("bg-2d.u0536", 0x600000, 0x400000, CRC(c607a444) SHA1(dc1aa96a42e9394ca6036359670a4ec6f830c96d) )
	ROM_LOAD24_BYTE("bg-2p.u0538", 0x600002, 0x200000, CRC(f0830248) SHA1(6075df96b49e70d2243fef691e096119e7a4d044) )

	ROM_REGION( 0x1200000, "gfx3", 0)	/* sprites */
	ROM_LOAD("obj3.u0434",  0x0000000, 0x400000, CRC(e08f42dc) SHA1(5188d71d4355eaf43ea8893b4cfc4fe80cc24f41) )
	ROM_LOAD("obj3b.u0433", 0x0400000, 0x200000, CRC(1b6a523c) SHA1(99a420dbc8e22e7832ccda7cec9fa661a2a2687a) )
	ROM_LOAD("obj2.u0431",  0x0600000, 0x400000, CRC(7aeadd8e) SHA1(47103c0579240c5b1add4d0b164eaf76f5fa97f0) )
	ROM_LOAD("obj2b.u0432", 0x0a00000, 0x200000, CRC(5d790a5d) SHA1(1ed5d4ad4c9a7e505ce35dcc90d184c26ce891dc) )
	ROM_LOAD("obj1.u0429",  0x0c00000, 0x400000, CRC(c2c50f02) SHA1(b81397b5800c6d49f58b7ac7ff6eac56da3c5257) )
	ROM_LOAD("obj1b.u0430", 0x1000000, 0x200000, CRC(5259321f) SHA1(3c70c1147e49f81371d0f60f7108d9718d56faf4) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("pcm.u0217",    0x000000, 0x200000, CRC(2edc30b5) SHA1(c25d690d633657fc3687636b9070f36bd305ae06) )
	ROM_LOAD("sound1.u0222", 0x200000, 0x080000, CRC(b7bd3703) SHA1(6427a7e6de10d6743d6e64b984a1d1c647f5643a) )
ROM_END

ROM_START( rfjet ) /* SPI Cart, Europe */
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("prg0.u0211", 0x000000, 0x80000, CRC(e5a3b304) SHA1(f7285f9c69c589fcc71082dc0b9225fdccec855f) )
	ROM_LOAD32_BYTE("prg1.u0212", 0x000001, 0x80000, CRC(395e6da7) SHA1(736f777cb1b6bf5541832b8ea89594738ca6d829) )
	ROM_LOAD32_BYTE("prg2.u0221", 0x000002, 0x80000, CRC(82f7a57e) SHA1(5300015e25d5f2f82eda3ed54bc105d645518498) )
	ROM_LOAD32_BYTE("prg3.u0220", 0x000003, 0x80000, CRC(cbdf100d) SHA1(c9efd11103429f7f36c1652cadb5384d925cb767) )

	ROM_REGION( 0x30000, "gfx1", ROMREGION_ERASEFF)
	ROM_LOAD24_BYTE("fix0.u0524", 0x000001, 0x10000, CRC(8bc080be) SHA1(ad296fb98242c963072346a8de289e704b445ad4) )
	ROM_LOAD24_BYTE("fix1.u0518", 0x000000, 0x10000, CRC(bded85e7) SHA1(ccb8c438ce6b9a742e3ab15be970b1e636783626) )
	ROM_LOAD24_BYTE("fixp.u0514", 0x000002, 0x10000, CRC(015d0748) SHA1(b1e8eaeba63a7914f1dc27d7e3ca5d0b6db202ed) )

	ROM_REGION( 0x900000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0543", 0x000000, 0x400000, CRC(edfd96da) SHA1(4813267f104619f569e5777e75b75304321abb49) )
	ROM_LOAD24_BYTE("bg-1p.u0544", 0x000002, 0x200000, CRC(a4cc4631) SHA1(cc1c4f4de8a078ca774f5a328a9a58291949b1fb) )
	ROM_LOAD24_WORD("bg-2d.u0545", 0x600000, 0x200000, CRC(731fbb59) SHA1(13cd29ec4d4c73582c5fb363218e737886826e5f) )
	ROM_LOAD24_BYTE("bg-2p.u0546", 0x600002, 0x100000, CRC(03652c25) SHA1(c0d77285111bc84e008362981ac02a246678ed0a) )

	ROM_REGION( 0x1800000, "gfx3", 0)	/* sprites */
	ROM_LOAD("obj-1.u0442", 0x0000000, 0x800000, CRC(58a59896) SHA1(edeaaa69987bd5d08c47ed9bf47a3901e2dcc892) )
	ROM_LOAD("obj-2.u0443", 0x0800000, 0x800000, CRC(a121d1e3) SHA1(1851ae81f2ae9d3404aadd9fbc0ed7f9230290b9) )
	ROM_LOAD("obj-3.u0444", 0x1000000, 0x800000, CRC(bc2c0c63) SHA1(c8d395722f7012c3be366a0fc9b224c537afabae) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("pcm-d.u0227",  0x000000, 0x200000, CRC(8ee3ff45) SHA1(2801b23495866c91c8f8bebd37d5fcae7a625838) )
	ROM_LOAD("sound1.u0222", 0x200000, 0x080000, CRC(d4fc3da1) SHA1(a03bd97e36a21d27a834b9691b27a7eb7ac51ff2) )
ROM_END

ROM_START( rfjetj ) /* SPI Cart, Japan */
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("prg0.bin", 0x000000, 0x80000, CRC(d82fb71f) SHA1(63a458fd007c353f4fae54a4882f5c565fe1efa4) )
	ROM_LOAD32_BYTE("prg1.bin", 0x000001, 0x80000, CRC(7e21c669) SHA1(731852e5925dccc9d0d1ae4bcafa238f157f4079) )
	ROM_LOAD32_BYTE("prg2.bin", 0x000002, 0x80000, CRC(2f402d55) SHA1(d0d852239abb6f4d73e263de5544fc0893e7a7ab) )
	ROM_LOAD32_BYTE("prg3.bin", 0x000003, 0x80000, CRC(d619e2ad) SHA1(9dbff1babf62c3c5478a84d2a82a428de5949154) )

	ROM_REGION( 0x30000, "gfx1", ROMREGION_ERASEFF)
	ROM_LOAD24_BYTE("fix0.u0524", 0x000001, 0x10000, CRC(8bc080be) SHA1(ad296fb98242c963072346a8de289e704b445ad4) )
	ROM_LOAD24_BYTE("fix1.u0518", 0x000000, 0x10000, CRC(bded85e7) SHA1(ccb8c438ce6b9a742e3ab15be970b1e636783626) )
	ROM_LOAD24_BYTE("fixp.u0514", 0x000002, 0x10000, CRC(015d0748) SHA1(b1e8eaeba63a7914f1dc27d7e3ca5d0b6db202ed) )

	ROM_REGION( 0x900000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0543", 0x000000, 0x400000, CRC(edfd96da) SHA1(4813267f104619f569e5777e75b75304321abb49) )
	ROM_LOAD24_BYTE("bg-1p.u0544", 0x000002, 0x200000, CRC(a4cc4631) SHA1(cc1c4f4de8a078ca774f5a328a9a58291949b1fb) )
	ROM_LOAD24_WORD("bg-2d.u0545", 0x600000, 0x200000, CRC(731fbb59) SHA1(13cd29ec4d4c73582c5fb363218e737886826e5f) )
	ROM_LOAD24_BYTE("bg-2p.u0546", 0x600002, 0x100000, CRC(03652c25) SHA1(c0d77285111bc84e008362981ac02a246678ed0a) )

	ROM_REGION( 0x1800000, "gfx3", 0)	/* sprites */
	ROM_LOAD("obj-1.u0442", 0x0000000, 0x800000, CRC(58a59896) SHA1(edeaaa69987bd5d08c47ed9bf47a3901e2dcc892) )
	ROM_LOAD("obj-2.u0443", 0x0800000, 0x800000, CRC(a121d1e3) SHA1(1851ae81f2ae9d3404aadd9fbc0ed7f9230290b9) )
	ROM_LOAD("obj-3.u0444", 0x1000000, 0x800000, CRC(bc2c0c63) SHA1(c8d395722f7012c3be366a0fc9b224c537afabae) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("pcm-d.u0227",  0x000000, 0x200000, CRC(8ee3ff45) SHA1(2801b23495866c91c8f8bebd37d5fcae7a625838) )
	ROM_LOAD("sound1.u0222", 0x200000, 0x080000, CRC(d4fc3da1) SHA1(a03bd97e36a21d27a834b9691b27a7eb7ac51ff2) )
ROM_END


ROM_START( rfjetu ) /* SPI Cart, US */
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("prg0u.u0211", 0x000000, 0x80000, CRC(15ac2040) SHA1(7309a9dd9c91fef0e761dcf8639f421ce7abc97f) )
	ROM_LOAD32_BYTE("prg1.u0212",  0x000001, 0x80000, CRC(395e6da7) SHA1(736f777cb1b6bf5541832b8ea89594738ca6d829) )
	ROM_LOAD32_BYTE("prg2.u0221",  0x000002, 0x80000, CRC(82f7a57e) SHA1(5300015e25d5f2f82eda3ed54bc105d645518498) )
	ROM_LOAD32_BYTE("prg3.u0220",  0x000003, 0x80000, CRC(cbdf100d) SHA1(c9efd11103429f7f36c1652cadb5384d925cb767) )

	ROM_REGION( 0x30000, "gfx1", ROMREGION_ERASEFF)
	ROM_LOAD24_BYTE("fix0.u0524", 0x000001, 0x10000, CRC(8bc080be) SHA1(ad296fb98242c963072346a8de289e704b445ad4) )
	ROM_LOAD24_BYTE("fix1.u0518", 0x000000, 0x10000, CRC(bded85e7) SHA1(ccb8c438ce6b9a742e3ab15be970b1e636783626) )
	ROM_LOAD24_BYTE("fixp.u0514", 0x000002, 0x10000, CRC(015d0748) SHA1(b1e8eaeba63a7914f1dc27d7e3ca5d0b6db202ed) )

	ROM_REGION( 0x900000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0543", 0x000000, 0x400000, CRC(edfd96da) SHA1(4813267f104619f569e5777e75b75304321abb49) )
	ROM_LOAD24_BYTE("bg-1p.u0544", 0x000002, 0x200000, CRC(a4cc4631) SHA1(cc1c4f4de8a078ca774f5a328a9a58291949b1fb) )
	ROM_LOAD24_WORD("bg-2d.u0545", 0x600000, 0x200000, CRC(731fbb59) SHA1(13cd29ec4d4c73582c5fb363218e737886826e5f) )
	ROM_LOAD24_BYTE("bg-2p.u0546", 0x600002, 0x100000, CRC(03652c25) SHA1(c0d77285111bc84e008362981ac02a246678ed0a) )

	ROM_REGION( 0x1800000, "gfx3", 0)	/* sprites */
	ROM_LOAD("obj-1.u0442", 0x0000000, 0x800000, CRC(58a59896) SHA1(edeaaa69987bd5d08c47ed9bf47a3901e2dcc892) )
	ROM_LOAD("obj-2.u0443", 0x0800000, 0x800000, CRC(a121d1e3) SHA1(1851ae81f2ae9d3404aadd9fbc0ed7f9230290b9) )
	ROM_LOAD("obj-3.u0444", 0x1000000, 0x800000, CRC(bc2c0c63) SHA1(c8d395722f7012c3be366a0fc9b224c537afabae) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("pcm-d.u0227",  0x000000, 0x200000, CRC(8ee3ff45) SHA1(2801b23495866c91c8f8bebd37d5fcae7a625838) )
	ROM_LOAD("sound1.u0222", 0x200000, 0x080000, CRC(d4fc3da1) SHA1(a03bd97e36a21d27a834b9691b27a7eb7ac51ff2) )
ROM_END

ROM_START( rfjeta ) /* SPI Cart, Asia */
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("prg0a.u0211", 0x000000, 0x80000, CRC(3418d4f5) SHA1(f8766d7b3708a196de417ee757787220b2a9ced1) )
	ROM_LOAD32_BYTE("prg1.u0212",  0x000001, 0x80000, CRC(395e6da7) SHA1(736f777cb1b6bf5541832b8ea89594738ca6d829) )
	ROM_LOAD32_BYTE("prg2.u0221",  0x000002, 0x80000, CRC(82f7a57e) SHA1(5300015e25d5f2f82eda3ed54bc105d645518498) )
	ROM_LOAD32_BYTE("prg3.u0220",  0x000003, 0x80000, CRC(cbdf100d) SHA1(c9efd11103429f7f36c1652cadb5384d925cb767) )

	ROM_REGION( 0x30000, "gfx1", ROMREGION_ERASEFF)
	ROM_LOAD24_BYTE("fix0.u0524", 0x000001, 0x10000, CRC(8bc080be) SHA1(ad296fb98242c963072346a8de289e704b445ad4) )
	ROM_LOAD24_BYTE("fix1.u0518", 0x000000, 0x10000, CRC(bded85e7) SHA1(ccb8c438ce6b9a742e3ab15be970b1e636783626) )
	ROM_LOAD24_BYTE("fixp.u0514", 0x000002, 0x10000, CRC(015d0748) SHA1(b1e8eaeba63a7914f1dc27d7e3ca5d0b6db202ed) )

	ROM_REGION( 0x900000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0543", 0x000000, 0x400000, CRC(edfd96da) SHA1(4813267f104619f569e5777e75b75304321abb49) )
	ROM_LOAD24_BYTE("bg-1p.u0544", 0x000002, 0x200000, CRC(a4cc4631) SHA1(cc1c4f4de8a078ca774f5a328a9a58291949b1fb) )
	ROM_LOAD24_WORD("bg-2d.u0545", 0x600000, 0x200000, CRC(731fbb59) SHA1(13cd29ec4d4c73582c5fb363218e737886826e5f) )
	ROM_LOAD24_BYTE("bg-2p.u0546", 0x600002, 0x100000, CRC(03652c25) SHA1(c0d77285111bc84e008362981ac02a246678ed0a) )

	ROM_REGION( 0x1800000, "gfx3", 0)	/* sprites */
	ROM_LOAD("obj-1.u0442", 0x0000000, 0x800000, CRC(58a59896) SHA1(edeaaa69987bd5d08c47ed9bf47a3901e2dcc892) )
	ROM_LOAD("obj-2.u0443", 0x0800000, 0x800000, CRC(a121d1e3) SHA1(1851ae81f2ae9d3404aadd9fbc0ed7f9230290b9) )
	ROM_LOAD("obj-3.u0444", 0x1000000, 0x800000, CRC(bc2c0c63) SHA1(c8d395722f7012c3be366a0fc9b224c537afabae) )

	ROM_REGION(0x200000, "ymf", ROMREGION_ERASE00)

	ROM_REGION(0x280000, "user2", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("pcm-d.u0227",  0x000000, 0x200000, CRC(8ee3ff45) SHA1(2801b23495866c91c8f8bebd37d5fcae7a625838) )
	ROM_LOAD("sound1.u0222", 0x200000, 0x080000, CRC(d4fc3da1) SHA1(a03bd97e36a21d27a834b9691b27a7eb7ac51ff2) )
ROM_END

ROM_START( rfjetus )	/* Single board version SXX2G */
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("rfj-06.u0259", 0x000000, 0x80000, CRC(c835aa7a) SHA1(291eada97ceb907dfea15688ce6055e63b3aa675) ) /* PRG0 */
	ROM_LOAD32_BYTE("rfj-07.u0258", 0x000001, 0x80000, CRC(3b6ca1ca) SHA1(9db019c0ddecfb58e2be5c345d78352f700035bf) ) /* PRG1 */
	ROM_LOAD32_BYTE("rfj-08.u0265", 0x000002, 0x80000, CRC(1f5dd06c) SHA1(6f5a8c9035971a470212cd0a89b94181011602c3) ) /* PRG2 */
	ROM_LOAD32_BYTE("rfj-09.u0264", 0x000003, 0x80000, CRC(cc71c402) SHA1(b040e600744e7b3f52de0fa852ce3ae08ae49985) ) /* PRG3 */

	ROM_REGION( 0x30000, "gfx1", ROMREGION_ERASEFF)
	ROM_LOAD24_BYTE("fix0.u0524", 0x000001, 0x10000, CRC(8bc080be) SHA1(ad296fb98242c963072346a8de289e704b445ad4) ) /* rfj-01 */
	ROM_LOAD24_BYTE("fix1.u0518", 0x000000, 0x10000, CRC(bded85e7) SHA1(ccb8c438ce6b9a742e3ab15be970b1e636783626) ) /* rfj-02 */
	ROM_LOAD24_BYTE("fixp.u0514", 0x000002, 0x10000, CRC(015d0748) SHA1(b1e8eaeba63a7914f1dc27d7e3ca5d0b6db202ed) ) /* rfj-03 */

	ROM_REGION( 0x900000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0535", 0x000000, 0x400000, CRC(edfd96da) SHA1(4813267f104619f569e5777e75b75304321abb49) )
	ROM_LOAD24_BYTE("bg-1p.u0537", 0x000002, 0x200000, CRC(a4cc4631) SHA1(cc1c4f4de8a078ca774f5a328a9a58291949b1fb) )
	ROM_LOAD24_WORD("bg-2d.u0536", 0x600000, 0x200000, CRC(731fbb59) SHA1(13cd29ec4d4c73582c5fb363218e737886826e5f) )
	ROM_LOAD24_BYTE("bg-2p.u0545", 0x600002, 0x100000, CRC(03652c25) SHA1(c0d77285111bc84e008362981ac02a246678ed0a) )

	ROM_REGION( 0x1800000, "gfx3", 0)	/* sprites */
	ROM_LOAD("obj-1.u073", 0x0000000, 0x800000, CRC(58a59896) SHA1(edeaaa69987bd5d08c47ed9bf47a3901e2dcc892) )
	ROM_LOAD("obj-2.u074", 0x0800000, 0x800000, CRC(a121d1e3) SHA1(1851ae81f2ae9d3404aadd9fbc0ed7f9230290b9) )
	ROM_LOAD("obj-3.u075", 0x1000000, 0x800000, CRC(bc2c0c63) SHA1(c8d395722f7012c3be366a0fc9b224c537afabae) )

	ROM_REGION(0x40000, "soundcpu", 0)		/* 256k for the Z80 */
	ROM_LOAD("rfj-05.u091", 0x000000, 0x40000, CRC(a55e8799) SHA1(5d4ca9ae920ab54e23ee3b1b33db72711e744516) ) /* ZPRG */

	ROM_REGION(0x280000, "ymf", ROMREGION_ERASE00)	/* sound roms */
	ROM_LOAD("pcm-d.u0103",  0x000000, 0x200000, CRC(8ee3ff45) SHA1(2801b23495866c91c8f8bebd37d5fcae7a625838) )
	ROM_LOAD("rfj-04.u0107", 0x200000, 0x080000, CRC(c050da03) SHA1(1002dac51a3a4932c4f0074c1f3d97a597d98755) ) /* SOUND1 */
ROM_END



/*******************************************************************/
/* SYS386 games */

/*
Raiden Fighters 2 - 2000 Operation Hell Dive
Seibu Kaihatsu Inc., 2000

This game runs on a single PCB, not the usual SPI hardware that the previous Raiden
Fighters games ran on.

PCB Layout
----------

SYS386I
|-----------------------------------------------------|
|HA13118   6295   PCM0       OBJ4    OBJ5    OBJ6     |
|    4560D 6295   PCM1   71256   OBJ1    OBJ2    OBJ3 |
|                                                     |
|              PAL  PAL  71256      |----------|      |
|                                   |          |      |
|        *     PAL  PAL    71256    |          |      |
|                                   |  RISE10  |      |
|                          71256    | (QFP240) |      |
|J |--------|                       |          |      |
|  |SIE150  |       |---------|     |----------|      |
|A |(QFP100)|       |         |                       |
|  |--------| 71256 |SEI400   |            28.6363MHz |
|M                  |SB07-3460|                       |
|             71256 |(QFP208) |                       |
|M                  |---------|   TC551664            |
|                                                     |
|A   93C46            |---------| TC551664            |
|            PAL      |         |                     |
|                     |SEI600   |             PRG0-1  |
|                     |SB08-1513| |--------|          |
|                     |(QFP208) | | AM386  |          |
|                     |---------| | DX40   |          |
| FIX0   BG-1P. BG-1D             |(QFP132)|  PRG2-3  |
| FIX1                            |--------|          |
| FIX2   BG-2P  BG-2D                                 |
|                                 40MHz        PAL    |
|-----------------------------------------------------|
Notes:
      ROMs
      ----
           OBJ1, OBJ2, OBJ3 - Objects,  MX23C3210TC surface mounted 32MBit MaskROM (TSOP48)
           OBJ4, OBJ5, OBJ6 - Objects,  MX23C1610TC surface mounted 16MBit MaskROM (TSOP48)
                              Note - The PCB is wired to accept MX32C3210 32MBit MaskROMs in all OBJ positions.

           PRG0-1, PRG2-3   - Main program,  LH28F800SU surface mounted 8MBit FlashROM (TSOP56)
                              Note - The PCB is wired to accept DIP32 27C040 4MBit EPROMs here also with positions
                              labelled PRG0, PRG1, PRG2 & PRG3

           BG-1P, BG-2P     - Backgrounds, MX29F1610MC surface mounted 16MBit FlashROM (SOP44)
           BG-1D, BG-2D     - Backgrounds, MX23C3210MC surface mounted 32MBit MaskROM (SOP44)

           PCM0, PCM1       - PCM sound samples, 27C4001 4MBit EPROM (DIP32)

           FIX0, FIX1, FIX2 - 27C512 EPROM (DIP28)

      Clocks
      ------
            M6295 clock -   1.431815MHz (both, 28.6363MHz / 20),  sample rate = M6295 clock / 132 (both)
            AM386 clock -   40.000MHz
            VSync       -   54Hz

      RAM
      ---
         TC551664 - Toshiba TC551664J-15 1MBit SRAM (64k x16, SOJ44)
         71256    - 256k SRAM (32k x8, SOJ28)

      Custom IC's
      -----------
                 RISE10 (QFP240)
                 SEI400 (QFP208)
                 SEI600 (QFP208)
                 SIE150 (QFP100)

      Other
      -----
           93C46   - 128bytes EEPROM (SOIC8, not dumped)
           HA13118 - 18W audio power AMP
           4560D   - Op AMP (DIP8)
           *       - Unpopulated position for Xilinx XC9572 CPLD
*/

ROM_START( rdft22kc )
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_WORD("prg0-1.267", 0x000000, 0x100000, CRC(0d7d6eb8) SHA1(3a71e1e0ba5bb500dc026debbb6189723c0c2890) )
	ROM_LOAD32_WORD("prg2-3.266", 0x000002, 0x100000, CRC(ead53e69) SHA1(b0e2e06f403317054ecb48d2747034424245f129) )

	ROM_REGION( 0x30000, "gfx1", 0)	/* text layer roms */
	ROM_LOAD24_BYTE("fix0.524", 0x000001, 0x10000, CRC(ed11d043) SHA1(fd3a5a33aa4d795941d64c0d23f9d6f8222843e3) )
	ROM_LOAD24_BYTE("fix1.518", 0x000000, 0x10000, CRC(7036d70a) SHA1(3535b52c0fa1a1158cacc041f8aba2b9a1b43af5) )
	ROM_LOAD24_BYTE("fix2.514", 0x000002, 0x10000, CRC(29b465da) SHA1(644454ab5e0dc1028e9512f85adfe5d8adb757de) )

	ROM_REGION( 0xc00000, "gfx2", 0)	/* background layer roms */
	ROM_LOAD24_WORD("bg-1d.535", 0x000000, 0x400000, CRC(6143f576) SHA1(c034923d0663d9ef24357a03098b8cb81dbab9f8) )
	ROM_LOAD24_BYTE("bg-1p.544", 0x000002, 0x200000, CRC(55e64ef7) SHA1(aae991268948d07342ee8ba1b3761bd180aab8ec) )
	ROM_LOAD24_WORD("bg-2d.536", 0x600000, 0x400000, CRC(c607a444) SHA1(dc1aa96a42e9394ca6036359670a4ec6f830c96d) )
	ROM_LOAD24_BYTE("bg-2p.545", 0x600002, 0x200000, CRC(f0830248) SHA1(6075df96b49e70d2243fef691e096119e7a4d044) )

	ROM_REGION( 0x1200000, "gfx3", 0)	/* sprites */
	ROM_LOAD("obj3.075",  0x0000000, 0x400000, CRC(e08f42dc) SHA1(5188d71d4355eaf43ea8893b4cfc4fe80cc24f41) )
	ROM_LOAD("obj6.078",  0x0400000, 0x200000, CRC(1b6a523c) SHA1(99a420dbc8e22e7832ccda7cec9fa661a2a2687a) )
	ROM_LOAD("obj2.074",  0x0600000, 0x400000, CRC(7aeadd8e) SHA1(47103c0579240c5b1add4d0b164eaf76f5fa97f0) )
	ROM_LOAD("obj5.077",  0x0a00000, 0x200000, CRC(5d790a5d) SHA1(1ed5d4ad4c9a7e505ce35dcc90d184c26ce891dc) )
	ROM_LOAD("obj1.073",  0x0c00000, 0x400000, CRC(c2c50f02) SHA1(b81397b5800c6d49f58b7ac7ff6eac56da3c5257) )
	ROM_LOAD("obj4.076",  0x1000000, 0x200000, CRC(5259321f) SHA1(3c70c1147e49f81371d0f60f7108d9718d56faf4) )

	ROM_REGION( 0x80000, "oki1", 0)	/* sound data for MSM6295 */
	ROM_LOAD("pcm0.1022", 0x000000, 0x80000, CRC(fd599b35) SHA1(00c0307d1b503bd5ce02d7960ce5e1ad600a7290) )

	ROM_REGION( 0x80000, "oki2", 0)	/* sound data for MSM6295 */
	ROM_LOAD("pcm1.1023", 0x000000, 0x80000, CRC(8b716356) SHA1(42ee1896c02518cd1e9cb0dc130321834665a79e) )
ROM_END

/*

E-Jan Sakurasou
(c)1999 Seibu

SYS386F V2.0

CPU: intel i386DX-25MHz
Sound: YMZ280B-F YAC516-M
OSC: 28.3751(H)-28.6363(L) 16.384MHz 50.000MHz
Custom: SEI600 RISE11

ROMs:
PRG0.BIN 211
PRG1.BIN 212
PRG2.BIN 221
PRG3.BIN 220

CHR1.BIN 442
CHR2.BIN 443
CHR3.BIN 444
CHR4.BIN 445

SOUND1.BIN 83
SOUND2.BIN 84

*/

ROM_START( ejsakura ) /* SYS386F V2.0 */
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("prg0.211",  0x100000, 0x40000, CRC(199f2f08) SHA1(096afb23f2763b9aee5e8de3870fe47116a8d134) )
	ROM_LOAD32_BYTE("prg1.212",  0x100001, 0x40000, CRC(2cb636e6) SHA1(3524231a336de5acc93dff20b0b65ade31e27116) )
	ROM_LOAD32_BYTE("prg2.221",  0x100002, 0x40000, CRC(98a7b615) SHA1(ea34d8f3e9200a6d84efe9168e2f573ec5c2afd2) )
	ROM_LOAD32_BYTE("prg3.220",  0x100003, 0x40000, CRC(9c3c037a) SHA1(a802e13a0b827896342d9d34dbb00d1c36cabaff) )

	ROM_REGION( 0x30000, "gfx1", ROMREGION_ERASEFF)

	ROM_REGION( 0x900000, "gfx2", ROMREGION_ERASEFF)	/* background layer roms */

	ROM_REGION( 0x1000000, "gfx3", 0)	/* sprites */
	ROM_LOAD16_WORD_SWAP("chr4.445", 0x000000, 0x400000, CRC(40c6c238) SHA1(0d07b59e25632feb070ce0e572ae75f9bb939893) )
	ROM_LOAD16_WORD_SWAP("chr3.444", 0x400000, 0x400000, CRC(8e5d1de5) SHA1(c1ccb6b4341ee1e939958ec9e68280c6faa2ef1f) )
	ROM_LOAD16_WORD_SWAP("chr2.443", 0x800000, 0x400000, CRC(638dc9ae) SHA1(0c11b1e688733fbaeabf83b33410714c22ae53cd) )
	ROM_LOAD16_WORD_SWAP("chr1.442", 0xc00000, 0x400000, CRC(177e3139) SHA1(0385a831c141d59ec4e9c6d6fae9436dca123764) )

	ROM_REGION(0x1000000, "ymz", 0)
	ROM_LOAD("sound1.83",  0x000000, 0x800000, CRC(98783cfc) SHA1(f142429e0658a36e908cc135fe0e01ce853d071d) )
	ROM_LOAD("sound2.84",  0x800000, 0x800000, CRC(ff37e769) SHA1(eb6d260cbc4e4a925a5d8f604ec695e567ac6bb5) )
ROM_END

ROM_START( ejsakura12 ) /* SYS386F V1.2 */
	ROM_REGION32_LE(0x200000, "user1", 0)	/* i386 program */
	ROM_LOAD32_BYTE("prg0v1.2.u0211",  0x100000, 0x40000, CRC(c734fde6) SHA1(d4256f0d2be624fc0e5340ae14679679e5e184c8) )
	ROM_LOAD32_BYTE("prg1v1.2.u0212",  0x100001, 0x40000, CRC(fb7a9e38) SHA1(5a2e02e1b8ed71ffc96dbda871618f5f9cccc8c6) )
	ROM_LOAD32_BYTE("prg2v1.2.u0221",  0x100002, 0x40000, CRC(e13098ad) SHA1(abf471afd25a08ba1848964c988112c86d1dcfaa) )
	ROM_LOAD32_BYTE("prg3v1.2.u0220",  0x100003, 0x40000, CRC(29b5460f) SHA1(c9cb0eb421a79b722bf5a0dc428d0f5f8499e170) )

	ROM_REGION( 0x30000, "gfx1", ROMREGION_ERASEFF)

	ROM_REGION( 0x900000, "gfx2", ROMREGION_ERASEFF)	/* background layer roms */

	ROM_REGION( 0x1000000, "gfx3", 0)	/* sprites */
	ROM_LOAD16_WORD_SWAP("chr4.445", 0x000000, 0x400000, CRC(40c6c238) SHA1(0d07b59e25632feb070ce0e572ae75f9bb939893) )
	ROM_LOAD16_WORD_SWAP("chr3.444", 0x400000, 0x400000, CRC(8e5d1de5) SHA1(c1ccb6b4341ee1e939958ec9e68280c6faa2ef1f) )
	ROM_LOAD16_WORD_SWAP("chr2.443", 0x800000, 0x400000, CRC(638dc9ae) SHA1(0c11b1e688733fbaeabf83b33410714c22ae53cd) )
	ROM_LOAD16_WORD_SWAP("chr1.442", 0xc00000, 0x400000, CRC(177e3139) SHA1(0385a831c141d59ec4e9c6d6fae9436dca123764) )

	ROM_REGION(0x1000000, "ymz", 0)
	ROM_LOAD("sound1.83",  0x000000, 0x800000, CRC(98783cfc) SHA1(f142429e0658a36e908cc135fe0e01ce853d071d) )
	ROM_LOAD("sound2.84",  0x800000, 0x800000, CRC(ff37e769) SHA1(eb6d260cbc4e4a925a5d8f604ec695e567ac6bb5) )
ROM_END

/*******************************************************************/


/* SPI */
GAME( 1995, senkyu,    0,       spi,      spi_3button, senkyu,   ROT0,   "Seibu Kaihatsu", "Senkyu (Japan, set 1)",  GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1995, senkyua,   senkyu,  spi,      spi_3button, senkyua,  ROT0,   "Seibu Kaihatsu", "Senkyu (Japan, set 2)",  GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1995, batlball,  senkyu,  spi,      spi_3button, batlball, ROT0,   "Seibu Kaihatsu (Tuning license)", "Battle Balls (Germany)",  GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1995, batlballa, senkyu,  spi,      spi_3button, batlball, ROT0,   "Seibu Kaihatsu (Metrotainment license)", "Battle Balls (Asia)",  GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1995, batlballu, senkyu,  spi,      spi_3button, batlball, ROT0,   "Seibu Kaihatsu (Fabtek license)", "Battle Balls (US)",  GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )

GAME( 1995, viprp1,    0,       spi,      spi_3button, viprp1,  ROT270, "Seibu Kaihatsu", "Viper Phase 1 (World, New Version)",  GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1995, viprp1u,   viprp1,  spi,      spi_3button, viprp1o, ROT270, "Seibu Kaihatsu (Fabtek license)", "Viper Phase 1 (USA, New Version)",  GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND ) /* New version, "=U.S.A=" seems part of title */
GAME( 1995, viprp1j,   viprp1,  spi,      spi_3button, viprp1,  ROT270, "Seibu Kaihatsu", "Viper Phase 1 (Japan, New Version)",  GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1995, viprp1s,   viprp1,  spi,      spi_3button, viprp1,  ROT270, "Seibu Kaihatsu", "Viper Phase 1 (Switzerland, New Version)",  GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )

GAME( 1995, viprp1oj,  viprp1,  spi,      spi_3button, viprp1o, ROT270, "Seibu Kaihatsu", "Viper Phase 1 (Japan)",  GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1995, viprp1ot,  viprp1,  spi,      spi_3button, viprp1,  ROT270, "Seibu Kaihatsu (Tuning license)", "Viper Phase 1 (Germany)",  GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1995, viprp1hk,  viprp1,  spi,      spi_3button, viprp1,  ROT270, "Seibu Kaihatsu (Metrotainment license)", "Viper Phase 1 (Hong Kong)",  GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND ) /* "=HONG KONG=" seems part of title */

GAME( 1996, ejanhs,    0,       spi,      spi_ejanhs,  ejanhs,   ROT0,   "Seibu Kaihatsu", "E-Jan High School (Japan)",  GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )

GAME( 1996, rdft,      0,       spi,      spi_3button, rdft, ROT270, "Seibu Kaihatsu", "Raiden Fighters (Japan set 1)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1996, rdftu,     rdft,    spi,      spi_3button, rdft, ROT270, "Seibu Kaihatsu (Fabtek license)", "Raiden Fighters (US)",  GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1996, rdftau,    rdft,    spi,      spi_3button, rdft, ROT270, "Seibu Kaihatsu", "Raiden Fighters (Australia)",  GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1996, rdftj,     rdft,    spi,      spi_3button, rdft, ROT270, "Seibu Kaihatsu", "Raiden Fighters (Japan set 2)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1996, rdftdi,    rdft,    spi,      spi_3button, rdft, ROT270, "Seibu Kaihatsu (Dream Island license)", "Raiden Fighters (Dream Island Co. license)",  GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1996, rdftit,    rdft,    spi,      spi_3button, rdft, ROT270, "Seibu Kaihatsu", "Raiden Fighters (Italy)",  GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1996, rdfta,     rdft,    spi,      spi_3button, rdft, ROT270, "Seibu Kaihatsu", "Raiden Fighters (Austria)",  GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )

GAME( 1997, rdft2,     0,       spi,      spi_2button, rdft2,  ROT270, "Seibu Kaihatsu (Tuning license)", "Raiden Fighters 2",  GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1997, rdft2a2,   rdft2,   spi,      spi_2button, rdft2,  ROT270, "Seibu Kaihatsu (Dream Island license)", "Raiden Fighters 2 (Asia, Dream Island license, SPI)",  GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1997, rdft2a,    rdft2,   spi,      spi_2button, rdft2,  ROT270, "Seibu Kaihatsu (Metrotainment license)", "Raiden Fighters 2 (Asia, Metrotainment license, SPI)",  GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1997, rdft2j,    rdft2,   spi,      spi_2button, rdft2,  ROT270, "Seibu Kaihatsu", "Raiden Fighters 2 (Japan, SPI)",  GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1997, rdft2j2,   rdft2,   spi,      spi_2button, rdft2,  ROT270, "Seibu Kaihatsu", "Raiden Fighters 2 (Japan set 2, SPI)",  GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1997, rdft2t,    rdft2,   spi,      spi_2button, rdft2,  ROT270, "Seibu Kaihatsu", "Raiden Fighters 2 (Taiwan, SPI)",  GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1997, rdft2u,    rdft2,   spi,      spi_2button, rdft2,  ROT270, "Seibu Kaihatsu (Fabtek license)", "Raiden Fighters 2 (USA, SPI)",  GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )

GAME( 1998, rfjet,     0,       spi,      spi_2button, rfjet,    ROT270, "Seibu Kaihatsu (Tuning license)", "Raiden Fighters Jet",  GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1998, rfjetu,    rfjet,   spi,      spi_2button, rfjet,    ROT270, "Seibu Kaihatsu (Fabtek license)", "Raiden Fighters Jet (US)",  GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1998, rfjeta,    rfjet,   spi,      spi_2button, rfjet,    ROT270, "Seibu Kaihatsu (Dream Island license)", "Raiden Fighters Jet (Asia)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
GAME( 1998, rfjetj,    rfjet,   spi,      spi_2button, rfjet,    ROT270, "Seibu Kaihatsu", "Raiden Fighters Jet (Japan)",  GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )

/* there is another rf dump rf_spi_asia.zip but it seems strange, 1 program rom, cart pic seems to show others as a different type of rom */

/* SXX2F */
GAME( 1997, rdft2us,   rdft2,   sxx2f,    spi_2button, rdft2us,  ROT270, "Seibu Kaihatsu (Fabtek license)", "Raiden Fighters 2.1 (US, Single Board)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND ) // title screen shows '2.1'

/* SXX2G */
GAME( 1999, rfjetus,   rfjet,   sxx2g,    spi_2button, rfjet,    ROT270, "Seibu Kaihatsu (Fabtek license)", "Raiden Fighters Jet (US, Single Board)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND  ) // has 1998-99 copyright + planes unlocked

/* SYS386 */
GAME( 2000, rdft22kc,  rdft2,   seibu386, seibu386_2button, rdft22kc, ROT270, "Seibu Kaihatsu", "Raiden Fighters 2 - 2000 (China)", GAME_IMPERFECT_GRAPHICS )

/* SYS386F V2.0 */
GAME( 1999, ejsakura,   0,        sys386f2, spi_ejsakura, sys386f2, ROT0, "Seibu Kaihatsu", "E-Jan Sakurasou (v2.0)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, ejsakura12, ejsakura, sys386f2, spi_ejsakura, sys386f2, ROT0, "Seibu Kaihatsu", "E-Jan Sakurasou (v1.2)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
