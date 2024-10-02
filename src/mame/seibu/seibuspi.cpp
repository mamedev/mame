// license:BSD-3-Clause
// copyright-holders:Ville Linde, hap, Nicola Salmoria
/*
      Seibu SPI Hardware
      Seibu SYS386I
      Seibu SYS386F

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


      The SPI mainboard is region locked. You can only play cartridges that are
      from the same region, otherwise the updater will give a checksum error.
      The region code is the 1st byte of flashrom u1053. If it is erased somehow,
      or power goes off during updating, it will show hardware error 81.

      This code is also in the main PRG ROM at offset 0x1ffffc, even on single
      board games. Known regions are:

          0x01 Japan
          0x10 US
          0x20 Taiwan
          0x22 Asia / Hong Kong
          0x24 Korea
          0x28 China
          0x80 Europe / Germany
          0x82 Austria
          0x8c Great Britain
          0x8e Greece
          0x90 Holland
          0x92 Italy
          0x96 Portugal
          0x9c Switzerland
          0x9e Australia
          0xbe World?


      SYS386I seems like a lower-cost version of single-board SPI.
      It has a 40MHz AMD 386 and a considerably weaker sound system (dual MSM6295).

TODO:
- Improve alpha blending. In Viper Phase 1, see the blue "Viper" logo when on the
  "push 1 or 2 players button" screen. Note that the alpha blended red logo on the
  title screen is tiles(that effect is simulated), this blue logo is sprites.
  The current implementation is a crude hack.
  * DMA table? can't find any
  * data in transparent pen? nope
  * color bit 15? nope
  * writes to $100/104/108 might be interesting...
- not sure if layer priorities are completely right

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
      JP121     - Jumper to set sound output to mono or stereo
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
61256 : 32k x8 SRAM (x2). These are tied to the RISE11 chip with RAM A11-A14 tied to vcc/gnd so
        these RAMs are configured as 2kB each and the RISE11 has a total of 4kB connected to it.
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



Mahjong Adapter Layout
----------------------

(C)SXX2C MAHJANG IF SEIBU KAIHATSU INC.
|------------|  |---------------------------------------------|  |------------|
|            |--|                  J A M M A                  |--|            |
|   E  E                                                               E  E   |
|   X  X                                                               X  X   |
|   C  C     74LS393   16.9344MHz    jumpers        74LS174  74LS174   C  C   |
|   N  N                                                               N  N   |
|   4  3     74LS393       *                        74LS174  74LS174   2  1   |
|                                                                             |
|   74LS138   74LS04       **           ***         74LS174  74LS174          |
|                                                                             |
|   74LS161   SN7406            resistor package     ****    74LS148          |
|                                                                             |
|          |---|                   5 6 P M J                    |---|         |
|----------|   |------------------------------------------------|   |---------|
Notes:
*     : Unpopulated location for ULN2003
**    : Unpopulated location for 16V8-25 GAL
***   : Unpopulated location for 74LS161
****  : Unpopulated location for 74LS148

This board is used by E Jong High School to encode the mahjong inputs onto the
SPI motherboard's JAMMA connector.


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
      Z80       - Zilog Z84C0006PCS (DIP40) - Unknown clock
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
#include "seibuspi.h"

#include "cpu/i386/i386.h"
#include "cpu/z80/z80.h"
#include "machine/ds2404.h"
#include "machine/intelfsh.h"
#include "seibuspi_m.h"
#include "sound/ymf271.h"
#include "sound/ymz280b.h"
#include "seibu_crtc.h"

#include "screen.h"
#include "speaker.h"


// default values written to CRTC (note: SYS386F does not have this chip)
#define PIXEL_CLOCK  (XTAL(28'636'363)/4)

#define SPI_HTOTAL   (448)
#define SPI_HBEND    (0)
#define SPI_HBSTART  (320)

#define SPI_VTOTAL   (296)
#define SPI_VBEND    (0)
#define SPI_VBSTART  (240) /* actually 253, but visible area is 240 lines */


#define ENABLE_SPEEDUP_HACKS 1 /* speed up at idle loops */


/*****************************************************************************/

u8 seibuspi_state::sound_fifo_status_r()
{
	// d0: fifo full flag (z80)
	// d1: fifo empty flag (main)
	// other bits: unused?
	const u8 d1 = (m_soundfifo[1] != nullptr) ? m_soundfifo[1]->ef_r() << 1 : 0;
	return d1 | m_soundfifo[0]->ff_r();
}

u8 seibuspi_state::spi_status_r()
{
	// d0: unknown status, waits for it to be set, video/dma related?
	// other bits: unused?
	return 0x01;
}

u8 seibuspi_state::spi_ds2404_unknown_r()
{
	// d0, d1, d2: unknown, waits for it to be cleared
	return 0x00;
}

void seibuspi_state::eeprom_w(u8 data)
{
	m_eeprom->di_write((data & 0x80) ? 1 : 0);
	m_eeprom->clk_write((data & 0x40) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->cs_write((data & 0x20) ? ASSERT_LINE : CLEAR_LINE);
}

void seibuspi_state::spi_layerbanks_eeprom_w(u8 data)
{
	// low bits: tile banks
	rf2_layer_bank_w(data);

	// high bits: eeprom
	eeprom_w(data);
}

void seibuspi_state::oki_bank_w(u8 data)
{
	m_oki[1]->set_rom_bank((data >> 2) & 1);
}

void seibuspi_state::z80_prg_transfer_w(u8 data)
{
	if (m_z80_prg_transfer_pos < m_z80_rom->bytes())
	{
		m_z80_rom->base()[m_z80_prg_transfer_pos] = data;
		m_z80_prg_transfer_pos++;
	}
}

void seibuspi_state::z80_enable_w(u8 data)
{
	// d0: reset z80
	// other bits: unused
	m_z80_prg_transfer_pos = 0;
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 0x01) ? CLEAR_LINE : ASSERT_LINE);
}

u8 seibuspi_state::sb_coin_r()
{
	const u8 ret = m_sb_coin_latch;
	if (!machine().side_effects_disabled())
		m_sb_coin_latch = 0;
	return ret;
}

u32 seibuspi_state::ejsakura_keyboard_r()
{
	// coins/eeprom data
	u32 ret = m_special->read();

	// multiplexed inputs
	for (int i = 0; i < 5; i++)
		if (m_ejsakura_input_port >> i & 1)
			ret &= m_key[i]->read();

	return ret;
}

void seibuspi_state::ejsakura_input_select_w(u32 data)
{
	m_ejsakura_input_port = data;
}


void seibuspi_state::base_map(address_map &map)
{
	map(0x00000000, 0x0003ffff).ram().share("mainram");
	map(0x00000400, 0x0000043f).rw("crtc", FUNC(seibu_crtc_device::read), FUNC(seibu_crtc_device::write));
	map(0x00000480, 0x00000483).w(FUNC(seibuspi_state::tilemap_dma_start_w));
	map(0x00000484, 0x00000487).w(FUNC(seibuspi_state::palette_dma_start_w));
	map(0x00000490, 0x00000493).w(FUNC(seibuspi_state::video_dma_length_w));
	map(0x00000494, 0x00000497).w(FUNC(seibuspi_state::video_dma_address_w));
	map(0x00000498, 0x0000049b).nopw(); // ? dma address high bits? (always writes 0)
	map(0x00000600, 0x00000600).r(FUNC(seibuspi_state::spi_status_r));
	map(0x00000604, 0x00000607).portr("INPUTS");
	map(0x00000608, 0x0000060b).portr("EXCH");
	map(0x0000060c, 0x0000060f).portr("SYSTEM");
	map(0x00200000, 0x003fffff).rom().region("maincpu", 0);
	map(0xffe00000, 0xffffffff).rom().region("maincpu", 0); // ROM location in real-mode
}

void seibuspi_state::sei252_map(address_map &map)
{
	//map(0x00000500, 0x0000057f).rw("obj", FUNC(sei252_device::read_xor), FUNC(sei252_device::write_xor));
	map(0x0000050e, 0x0000050f).w(FUNC(seibuspi_state::sprite_dma_start_w));
	map(0x00000524, 0x00000527).nopw(); // SEI252 sprite decryption key, see machine/spisprit.c
	map(0x00000528, 0x0000052b).nopw(); // SEI252 sprite decryption unknown
	map(0x00000530, 0x00000533).nopw(); // SEI252 sprite decryption table key, see machine/spisprit.c
	map(0x00000534, 0x00000537).nopw(); // SEI252 sprite decryption unknown
	map(0x0000053c, 0x0000053f).nopw(); // SEI252 sprite decryption table index, see machine/spisprit.c
}

void seibuspi_state::rise_map(address_map &map)
{
	//map(0x00000500, 0x0000057f).rw("obj", FUNC(seibu_encrypted_sprite_device::read), FUNC(seibu_encrypted_sprite_device::write));
	map(0x0000054c, 0x0000054f).nopw(); // RISE10/11 sprite decryption key, see machine/seibuspi.c
	map(0x00000562, 0x00000563).w(FUNC(seibuspi_state::sprite_dma_start_w));
}

void seibuspi_state::spi_map(address_map &map)
{
	base_map(map);
	sei252_map(map);
	map(0x00000600, 0x00000603).nopw(); // ?
	map(0x00000680, 0x00000680).r("soundfifo2", FUNC(fifo7200_device::data_byte_r));
	map(0x00000680, 0x00000680).w("soundfifo1", FUNC(fifo7200_device::data_byte_w));
	map(0x00000684, 0x00000684).r(FUNC(seibuspi_state::sound_fifo_status_r));
	map(0x00000688, 0x00000688).w(FUNC(seibuspi_state::z80_prg_transfer_w));
	map(0x0000068c, 0x0000068c).w(FUNC(seibuspi_state::z80_enable_w));
	map(0x0000068e, 0x0000068e).w(FUNC(seibuspi_state::rf2_layer_bank_w));
	map(0x000006d0, 0x000006d0).w("ds2404", FUNC(ds2404_device::_1w_reset_w));
	map(0x000006d4, 0x000006d4).w("ds2404", FUNC(ds2404_device::data_w));
	map(0x000006d8, 0x000006d8).w("ds2404", FUNC(ds2404_device::clk_w));
	map(0x000006dc, 0x000006dc).r("ds2404", FUNC(ds2404_device::data_r));
	map(0x000006dd, 0x000006dd).r(FUNC(seibuspi_state::spi_ds2404_unknown_r));
	map(0x00a00000, 0x013fffff).rom().region("sound01", 0);
}

void seibuspi_state::rdft2_map(address_map &map)
{
	base_map(map);
	rise_map(map);
	map(0x00000600, 0x00000603).nopw(); // ?
	map(0x00000680, 0x00000680).r("soundfifo2", FUNC(fifo7200_device::data_byte_r));
	map(0x00000680, 0x00000680).w("soundfifo1", FUNC(fifo7200_device::data_byte_w));
	map(0x00000684, 0x00000684).r(FUNC(seibuspi_state::sound_fifo_status_r));
	map(0x00000688, 0x00000688).w(FUNC(seibuspi_state::z80_prg_transfer_w));
	map(0x0000068c, 0x0000068c).w(FUNC(seibuspi_state::z80_enable_w));
	map(0x0000068e, 0x0000068e).w(FUNC(seibuspi_state::rf2_layer_bank_w));
	map(0x000006d0, 0x000006d0).w("ds2404", FUNC(ds2404_device::_1w_reset_w));
	map(0x000006d4, 0x000006d4).w("ds2404", FUNC(ds2404_device::data_w));
	map(0x000006d8, 0x000006d8).w("ds2404", FUNC(ds2404_device::clk_w));
	map(0x000006dc, 0x000006dc).r("ds2404", FUNC(ds2404_device::data_r));
	map(0x000006dd, 0x000006dd).r(FUNC(seibuspi_state::spi_ds2404_unknown_r));
	map(0x00a00000, 0x013fffff).rom().region("sound01", 0);
}

void seibuspi_state::sxx2e_map(address_map &map)
{
	base_map(map);
	sei252_map(map);
	map(0x00000680, 0x00000680).r(FUNC(seibuspi_state::sb_coin_r));
	map(0x00000680, 0x00000680).w("soundfifo1", FUNC(fifo7200_device::data_byte_w));
	map(0x00000684, 0x00000684).r(FUNC(seibuspi_state::sound_fifo_status_r));
	map(0x00000688, 0x0000068b).noprw(); // ?
	map(0x0000068c, 0x0000068f).nopw();
	map(0x000006d0, 0x000006d0).w("ds2404", FUNC(ds2404_device::_1w_reset_w));
	map(0x000006d4, 0x000006d4).w("ds2404", FUNC(ds2404_device::data_w));
	map(0x000006d8, 0x000006d8).w("ds2404", FUNC(ds2404_device::clk_w));
	map(0x000006dc, 0x000006dc).r("ds2404", FUNC(ds2404_device::data_r));
	map(0x000006dd, 0x000006dd).r(FUNC(seibuspi_state::spi_ds2404_unknown_r));
}

void seibuspi_state::sxx2f_map(address_map &map)
{
	base_map(map);
	rise_map(map);
	map(0x00000680, 0x00000680).r(FUNC(seibuspi_state::sb_coin_r));
	map(0x00000680, 0x00000680).w("soundfifo1", FUNC(fifo7200_device::data_byte_w));
	map(0x00000684, 0x00000684).r(FUNC(seibuspi_state::sound_fifo_status_r));
	map(0x00000688, 0x0000068b).noprw(); // ?
	map(0x0000068e, 0x0000068e).w(FUNC(seibuspi_state::spi_layerbanks_eeprom_w));
	map(0x00000690, 0x00000693).nopw(); // ?
}

void seibuspi_state::sys386i_map(address_map &map)
{
	base_map(map);
	rise_map(map);
	map(0x0000068e, 0x0000068e).w(FUNC(seibuspi_state::spi_layerbanks_eeprom_w));
	map(0x0000068f, 0x0000068f).w(FUNC(seibuspi_state::oki_bank_w));
	map(0x01200000, 0x01200000).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x01200004, 0x01200004).rw("oki2", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

void seibuspi_state::sys386f_map(address_map &map)
{
	map(0x00000000, 0x0003ffff).ram().share("mainram");
	rise_map(map);
	map(0x00000010, 0x00000010).r(FUNC(seibuspi_state::spi_status_r));
	map(0x00000400, 0x00000403).portr("SYSTEM").w(FUNC(seibuspi_state::ejsakura_input_select_w));
	map(0x00000404, 0x00000404).w(FUNC(seibuspi_state::eeprom_w));
	map(0x00000408, 0x0000040f).w("ymz", FUNC(ymz280b_device::write)).umask32(0x000000ff);
	map(0x00000484, 0x00000487).w(FUNC(seibuspi_state::palette_dma_start_w));
	map(0x00000490, 0x00000493).w(FUNC(seibuspi_state::video_dma_length_w));
	map(0x00000494, 0x00000497).w(FUNC(seibuspi_state::video_dma_address_w));
	map(0x00000600, 0x00000607).r("ymz", FUNC(ymz280b_device::read)).umask32(0x000000ff);
	map(0x0000060c, 0x0000060f).r(FUNC(seibuspi_state::ejsakura_keyboard_r));
	map(0x00200000, 0x003fffff).rom().region("maincpu", 0);
	map(0xffe00000, 0xffffffff).rom().region("maincpu", 0); // ROM location in real-mode
}


/*****************************************************************************/

u8 seibuspi_state::z80_soundfifo_status_r()
{
	// d0: fifo full flag (main)
	// d1: fifo empty flag (z80)
	// other bits: unused?
	const u8 d0 = (m_soundfifo[1] != nullptr) ? m_soundfifo[1]->ff_r() : 0;
	return d0 | m_soundfifo[0]->ef_r() << 1;
}

void seibuspi_state::z80_bank_w(u8 data)
{
	// d0-d2: bank @ 8000
	const u8 bank = data & 7;

	if (bank != m_z80_lastbank)
	{
		m_z80_lastbank = bank;
		m_z80_bank->set_entry(bank);
	}

	// d3: watchdog?
}

void seibuspi_state::spi_coin_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, data & 1);
	machine().bookkeeping().coin_counter_w(1, data & 2);

	// coin latch used by single boards
	if (data)
		m_sb_coin_latch = 0xa0 | data;
	else
		m_sb_coin_latch = 0;
}


void seibuspi_state::sxx2e_soundmap(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x3fff).ram();
	map(0x4002, 0x4002).nopw(); // ?
	map(0x4003, 0x4003).nopw(); // ?
	map(0x4004, 0x4004).w(FUNC(seibuspi_state::spi_coin_w));
	map(0x4008, 0x4008).r("soundfifo1", FUNC(fifo7200_device::data_byte_r));
	map(0x4008, 0x4008).nopw(); // ?
	map(0x4009, 0x4009).r(FUNC(seibuspi_state::z80_soundfifo_status_r));
	map(0x400b, 0x400b).nopw(); // ?
	map(0x4013, 0x4013).portr("COIN");
	map(0x401b, 0x401b).w(FUNC(seibuspi_state::z80_bank_w));
	map(0x6000, 0x600f).rw("ymf", FUNC(ymf271_device::read), FUNC(ymf271_device::write));
	map(0x8000, 0xffff).bankr("z80_bank");
}

void seibuspi_state::spi_soundmap(address_map &map)
{
	sxx2e_soundmap(map);
	map(0x4008, 0x4008).w("soundfifo2", FUNC(fifo7200_device::data_byte_w));
	map(0x400a, 0x400a).portr("JUMPERS"); // TODO: get these to actually work (only on SXX2C)
}

void seibuspi_state::spi_ymf271_map(address_map &map)
{
	map.global_mask(0x1fffff);
	map(0x000000, 0x0fffff).rw("soundflash1", FUNC(intel_e28f008sa_device::read), FUNC(intel_e28f008sa_device::write));
	map(0x100000, 0x1fffff).rw("soundflash2", FUNC(intel_e28f008sa_device::read), FUNC(intel_e28f008sa_device::write));
}


/*****************************************************************************/

void seibuspi_state::ymf_irqhandler(int state)
{
	if (state)
		m_audiocpu->set_input_line_and_vector(0, ASSERT_LINE, 0xd7); // Z80 - IRQ is RST10
	else
		m_audiocpu->set_input_line(0, CLEAR_LINE);
}

template <int N>
ioport_value seibuspi_state::ejanhs_encode()
{
	/* E-Jan Highschool has a keyboard with the following keys
	The keys are encoded with 3 bits for each input port
	A     - 010 port A
	B     - 010 port B
	C     - 010 port C
	D     - 010 port D
	E     - 011 port A
	F     - 011 port B
	G     - 011 port C
	H     - 011 port D
	I     - 100 port A
	J     - 100 port B
	K     - 100 port C
	L     - 100 port D
	M     - 101 port A
	N     - 101 port B
	CHI   - 101 port C
	PON   - 101 port D
	KAN   - 110 port A
	REACH - 110 port B
	RON   - 110 port C
	Start - 111 port A
	*/
	static const u8 encoding[] = { 6, 5, 4, 3, 2, 7 };
	ioport_value state = ~m_key[N]->read();

	for (int bit = 0; bit < std::size(encoding); bit++)
		if (state & (1 << bit))
			return encoding[bit];
	return 0;
}


/*****************************************************************************/

// JP1 is for SXX2C only
static INPUT_PORTS_START( sxx2c )
	PORT_START("JUMPERS")
	PORT_DIPNAME( 0x03, 0x03, "JP1" ) // "Only used when game-board is changed with a new game" in manual
	PORT_DIPSETTING(    0x03, "Update" ) // "Changing game" in manual
	PORT_DIPSETTING(    0x00, "Normal" )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( sxx2e )
	PORT_START("INPUTS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_SPECIAL_ONOFF_DIPLOC( 0x00008000, 0x00000000, Flip_Screen, "SW1:1" )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE_NO_TOGGLE( 0x00000004, IP_ACTIVE_LOW)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNUSED )

	// Some sets still read unused 3P and 4P inputs from here as in Zero Team
	PORT_START("EXCH")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( spi_3button )
	PORT_INCLUDE( sxx2e )
	PORT_INCLUDE( sxx2c )
INPUT_PORTS_END


static INPUT_PORTS_START( spi_2button )
	PORT_INCLUDE( spi_3button )

	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( sxx2f )
	PORT_INCLUDE( sxx2e )

	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
INPUT_PORTS_END


static INPUT_PORTS_START( sys386i )
	PORT_INCLUDE( sxx2f )

	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x00008000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_MODIFY("COIN")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( spi_mahjong_keyboard )
	PORT_START("KEY.0")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0xffffffe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY.1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0xffffffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY.2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0xffffffe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY.3")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xffffffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY.4")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0xffffffc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( spi_ejanhs )
	PORT_INCLUDE( spi_mahjong_keyboard )
	PORT_INCLUDE( sxx2c )

	PORT_START("INPUTS")
	PORT_BIT( 0x00000007, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(seibuspi_state, ejanhs_encode<3>)
	PORT_BIT( 0x00000038, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(seibuspi_state, ejanhs_encode<4>)
	PORT_BIT( 0x00000700, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(seibuspi_state, ejanhs_encode<2>)
	PORT_BIT( 0x00003800, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(seibuspi_state, ejanhs_encode<0>)
	PORT_SPECIAL_ONOFF_DIPLOC( 0x00008000, 0x00000000, Flip_Screen, "SW1:1" )
	PORT_BIT( 0xffff4000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	// These need a noncontiguous encoding, but are nonfunctional in any case
	//PORT_BIT( 0x00000013, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(seibuspi_state, ejanhs_encode<1>)
	PORT_SERVICE_NO_TOGGLE( 0x00000004, IP_ACTIVE_LOW )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x000000f3, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("KEY.1")
	PORT_BIT( 0x0000003f, IP_ACTIVE_LOW, IPT_UNUSED ) // Decoded but not recognized

	PORT_MODIFY("KEY.4")
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNUSED ) // Decoded but not recognized

	PORT_START("EXCH") // Another set of mahjong inputs is decoded from here but not used
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( ejsakura )
	PORT_INCLUDE( spi_mahjong_keyboard )

	PORT_MODIFY("KEY.4")
	PORT_SERVICE_NO_TOGGLE( 0x00000200, IP_ACTIVE_LOW)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Payout") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0xfffff5c0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SPECIAL")
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00004000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0xffffbf3f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/*****************************************************************************/

#define PLANE_CHAR 0
#define PLANE_TILE 0
#define PLANE_SPRITE 0

static const gfx_layout spi_charlayout =
{
	8,8,        /* 8*8 characters */
	RGN_FRAC(1,1),       /* 4096 characters */
	5,          /* 6 bits per pixel */
	{ 4, 8, 12, 16, 20 },
	{ STEP4(3,-1), STEP4(4*6+3,-1) },
	{ STEP8(0,4*6*2) },
	6*8*8
};

#if PLANE_CHAR
static const gfx_layout spi_charlayout0 =
{
	8,8,        /* 8*8 characters */
	RGN_FRAC(1,1),       /* 4096 characters */
	1,          /* 6 bits per pixel */
	{ 0 },
	{ STEP4(3,-1), STEP4(4*6+3,-1) },
	{ STEP8(0,4*6*2) },
	6*8*8
};

static const gfx_layout spi_charlayout1 =
{
	8,8,        /* 8*8 characters */
	RGN_FRAC(1,1),       /* 4096 characters */
	1,          /* 6 bits per pixel */
	{ 4 },
	{ STEP4(3,-1), STEP4(4*6+3,-1) },
	{ STEP8(0,4*6*2) },
	6*8*8
};

static const gfx_layout spi_charlayout2 =
{
	8,8,        /* 8*8 characters */
	RGN_FRAC(1,1),       /* 4096 characters */
	1,          /* 6 bits per pixel */
	{ 8 },
	{ STEP4(3,-1), STEP4(4*6+3,-1) },
	{ STEP8(0,4*6*2) },
	6*8*8
};

static const gfx_layout spi_charlayout3 =
{
	8,8,        /* 8*8 characters */
	RGN_FRAC(1,1),       /* 4096 characters */
	1,          /* 6 bits per pixel */
	{ 12 },
	{ STEP4(3,-1), STEP4(4*6+3,-1) },
	{ STEP8(0,4*6*2) },
	6*8*8
};

static const gfx_layout spi_charlayout4 =
{
	8,8,        /* 8*8 characters */
	RGN_FRAC(1,1),       /* 4096 characters */
	1,          /* 6 bits per pixel */
	{ 16 },
	{ STEP4(3,-1), STEP4(4*6+3,-1) },
	{ STEP8(0,4*6*2) },
	6*8*8
};

static const gfx_layout spi_charlayout5 =
{
	8,8,        /* 8*8 characters */
	RGN_FRAC(1,1),       /* 4096 characters */
	1,          /* 6 bits per pixel */
	{ 20 },
	{ STEP4(3,-1), STEP4(4*6+3,-1) },
	{ STEP8(0,4*6*2) },
	6*8*8
};
#endif

static const gfx_layout spi_tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	6,
	{ 0, 4, 8, 12, 16, 20 },
	{ STEP4(3,-1), STEP4(4*6+3,-1), STEP4(4*6*2+3,-1), STEP4(4*6*3+3,-1) },
	{ STEP16(0,4*6*4) },
	6*16*16
};

#if PLANE_TILE
static const gfx_layout spi_tilelayout0 =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP4(3,-1), STEP4(4*6+3,-1), STEP4(4*6*2+3,-1), STEP4(4*6*3+3,-1) },
	{ STEP16(0,4*6*4) },
	6*16*16
};

static const gfx_layout spi_tilelayout1 =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 4 },
	{ STEP4(3,-1), STEP4(4*6+3,-1), STEP4(4*6*2+3,-1), STEP4(4*6*3+3,-1) },
	{ STEP16(0,4*6*4) },
	6*16*16
};

static const gfx_layout spi_tilelayout2 =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 8 },
	{ STEP4(3,-1), STEP4(4*6+3,-1), STEP4(4*6*2+3,-1), STEP4(4*6*3+3,-1) },
	{ STEP16(0,4*6*4) },
	6*16*16
};

static const gfx_layout spi_tilelayout3 =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 12 },
	{ STEP4(3,-1), STEP4(4*6+3,-1), STEP4(4*6*2+3,-1), STEP4(4*6*3+3,-1) },
	{ STEP16(0,4*6*4) },
	6*16*16
};

static const gfx_layout spi_tilelayout4 =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 16 },
	{ STEP4(3,-1), STEP4(4*6+3,-1), STEP4(4*6*2+3,-1), STEP4(4*6*3+3,-1) },
	{ STEP16(0,4*6*4) },
	6*16*16
};

static const gfx_layout spi_tilelayout5 =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 20 },
	{ STEP4(3,-1), STEP4(4*6+3,-1), STEP4(4*6*2+3,-1), STEP4(4*6*3+3,-1) },
	{ STEP16(0,4*6*4) },
	6*16*16
};
#endif

static const gfx_layout spi_spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	6,
	{ 0,8, RGN_FRAC(1,3)+0,RGN_FRAC(1,3)+8,RGN_FRAC(2,3)+0,RGN_FRAC(2,3)+8  },
	{ STEP8(7,-1), STEP8(8*2+7,-1) },
	{ STEP16(0,8*4) },
	16*32
};

#if PLANE_SPRITE
static const gfx_layout spi_spritelayout0 =
{
	16,16,
	RGN_FRAC(1,3),
	1,
	{ 0 },
	{ STEP8(7,-1), STEP8(8*2+7,-1) },
	{ STEP16(0,8*4) },
	16*32
};

static const gfx_layout spi_spritelayout1 =
{
	16,16,
	RGN_FRAC(1,3),
	1,
	{ 8 },
	{ STEP8(7,-1), STEP8(8*2+7,-1) },
	{ STEP16(0,8*4) },
	16*32
};

static const gfx_layout spi_spritelayout2 =
{
	16,16,
	RGN_FRAC(1,3),
	1,
	{ RGN_FRAC(1,3)+0 },
	{ STEP8(7,-1), STEP8(8*2+7,-1) },
	{ STEP16(0,8*4) },
	16*32
};

static const gfx_layout spi_spritelayout3 =
{
	16,16,
	RGN_FRAC(1,3),
	1,
	{ RGN_FRAC(1,3)+8 },
	{ STEP8(7,-1), STEP8(8*2+7,-1) },
	{ STEP16(0,8*4) },
	16*32
};

static const gfx_layout spi_spritelayout4 =
{
	16,16,
	RGN_FRAC(1,3),
	1,
	{ RGN_FRAC(2,3)+0 },
	{ STEP8(7,-1), STEP8(8*2+7,-1) },
	{ STEP16(0,8*4) },
	16*32
};

static const gfx_layout spi_spritelayout5 =
{
	16,16,
	RGN_FRAC(1,3),
	1,
	{ RGN_FRAC(2,3)+8 },
	{ STEP8(7,-1), STEP8(8*2+7,-1) },
	{ STEP16(0,8*4) },
	16*32
};
#endif

static GFXDECODE_START( gfx_spi )
	GFXDECODE_ENTRY( "sprites", 0, spi_spritelayout,  0, 64 )
	GFXDECODE_ENTRY( "tiles",   0, spi_tilelayout, 4096, 24 )
	GFXDECODE_ENTRY( "chars",   0, spi_charlayout, 5632, 16 )
#if PLANE_SPRITE
	GFXDECODE_ENTRY( "sprites", 0, spi_spritelayout0, 0, 6144/2 )
	GFXDECODE_ENTRY( "sprites", 0, spi_spritelayout1, 0, 6144/2 )
	GFXDECODE_ENTRY( "sprites", 0, spi_spritelayout2, 0, 6144/2 )
	GFXDECODE_ENTRY( "sprites", 0, spi_spritelayout3, 0, 6144/2 )
	GFXDECODE_ENTRY( "sprites", 0, spi_spritelayout4, 0, 6144/2 )
	GFXDECODE_ENTRY( "sprites", 0, spi_spritelayout5, 0, 6144/2 )
#endif
#if PLANE_TILE
	GFXDECODE_ENTRY( "tiles",   0, spi_tilelayout0,   0, 6144/2 )
	GFXDECODE_ENTRY( "tiles",   0, spi_tilelayout1,   0, 6144/2 )
	GFXDECODE_ENTRY( "tiles",   0, spi_tilelayout2,   0, 6144/2 )
	GFXDECODE_ENTRY( "tiles",   0, spi_tilelayout3,   0, 6144/2 )
	GFXDECODE_ENTRY( "tiles",   0, spi_tilelayout4,   0, 6144/2 )
	GFXDECODE_ENTRY( "tiles",   0, spi_tilelayout5,   0, 6144/2 )
#endif
#if PLANE_CHAR
	GFXDECODE_ENTRY( "chars",   0, spi_charlayout0,   0, 6144/2 )
	GFXDECODE_ENTRY( "chars",   0, spi_charlayout1,   0, 6144/2 )
	GFXDECODE_ENTRY( "chars",   0, spi_charlayout2,   0, 6144/2 )
	GFXDECODE_ENTRY( "chars",   0, spi_charlayout3,   0, 6144/2 )
	GFXDECODE_ENTRY( "chars",   0, spi_charlayout4,   0, 6144/2 )
	GFXDECODE_ENTRY( "chars",   0, spi_charlayout5,   0, 6144/2 )
#endif
GFXDECODE_END

static const gfx_layout sys386f_spritelayout =
{
	16,16,
	RGN_FRAC(1,4),
	8,
	{ 0, 8, RGN_FRAC(1,4)+0, RGN_FRAC(1,4)+8, RGN_FRAC(2,4)+0, RGN_FRAC(2,4)+8, RGN_FRAC(3,4)+0, RGN_FRAC(3,4)+8 },
	{ STEP8(7,-1), STEP8(8*2+7,-1) },
	{ STEP16(0,8*4) },
	16*32
};

static GFXDECODE_START( gfx_sys386f )
	GFXDECODE_ENTRY( "sprites", 0, sys386f_spritelayout, 0, 32 )
GFXDECODE_END


/*****************************************************************************/

INTERRUPT_GEN_MEMBER(seibuspi_state::spi_interrupt)
{
	device.execute().set_input_line(0, HOLD_LINE); // where is ack?
}

IRQ_CALLBACK_MEMBER(seibuspi_state::spi_irq_callback)
{
	return 0x20;
}


/* SPI */

void seibuspi_state::init_spi_common()
{
	if (m_z80_rom != nullptr)
		m_z80_bank->configure_entries(0, 8, m_z80_rom->base(), 0x8000);
}

void seibuspi_state::init_sei252()
{
	text_decrypt(memregion("chars")->base());
	bg_decrypt(memregion("tiles")->base(), memregion("tiles")->bytes());
	seibuspi_sprite_decrypt(memregion("sprites")->base(), 0x400000);
	init_spi_common();
}


void seibuspi_state::machine_start()
{
	// use this to determine the region code when adding a new SPI cartridge clone set
	logerror("Game region code: %02X\n", memregion("maincpu")->base()[0x1ffffc]);

	// savestates
	save_item(NAME(m_z80_prg_transfer_pos));
	save_item(NAME(m_z80_lastbank));
	save_item(NAME(m_sb_coin_latch));
	save_item(NAME(m_ejsakura_input_port));
	if (m_z80_rom != nullptr) save_pointer(NAME(m_z80_rom->base()), m_z80_rom->bytes());
}

MACHINE_RESET_MEMBER(seibuspi_state,spi)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	m_z80_bank->set_entry(0);
	m_z80_lastbank = 0;
	m_z80_prg_transfer_pos = 0;

	// fix the magic ID byte so users can't "brick" the machine
	if (m_soundflash1 && m_soundflash1_region)
	{
		m_soundflash1->write_raw(0, m_soundflash1_region[0]);
	}
}

void seibuspi_state::spi(machine_config &config)
{
	/* basic machine hardware */
	I386(config, m_maincpu, 50_MHz_XTAL / 2); // AMD or Intel 386DX, 25MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &seibuspi_state::spi_map);
	m_maincpu->set_vblank_int("screen", FUNC(seibuspi_state::spi_interrupt));
	m_maincpu->set_irq_acknowledge_callback(FUNC(seibuspi_state::spi_irq_callback));

	Z80(config, m_audiocpu, 28.636363_MHz_XTAL / 4); // Z84C0008PEC, 7.159MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &seibuspi_state::spi_soundmap);

	config.set_maximum_quantum(attotime::from_hz(12000));

	MCFG_MACHINE_RESET_OVERRIDE(seibuspi_state, spi)

	ds2404_device &rtc(DS2404(config, "ds2404", 32.768_kHz_XTAL));
	rtc.ref_year(1995);
	rtc.ref_month(1);
	rtc.ref_day(1);

	INTEL_E28F008SA(config, "soundflash1"); // Sharp LH28F008 on newer mainboard revision
	INTEL_E28F008SA(config, "soundflash2"); // "

	IDT7201(config, m_soundfifo[0]); // LH5496D, but on single board hw it's one CY7C421
	IDT7201(config, m_soundfifo[1]); // "

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(PIXEL_CLOCK, SPI_HTOTAL, SPI_HBEND, SPI_HBSTART, SPI_VTOTAL, SPI_VBEND, SPI_VBSTART);
	screen.set_screen_update(FUNC(seibuspi_state::screen_update_spi));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_spi);

	PALETTE(config, m_palette, palette_device::BLACK, 6144);

	seibu_crtc_device &crtc(SEIBU_CRTC(config, "crtc", 0));
	crtc.decrypt_key_callback().set(FUNC(seibuspi_state::tile_decrypt_key_w));
	crtc.layer_en_callback().set(FUNC(seibuspi_state::spi_layer_enable_w));
	crtc.reg_1a_callback().set(FUNC(seibuspi_state::spi_layer_bank_w));
	crtc.layer_scroll_callback().set(FUNC(seibuspi_state::scroll_w));

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ymf271_device &ymf(YMF271(config, "ymf", 16.9344_MHz_XTAL));
	ymf.irq_handler().set(FUNC(seibuspi_state::ymf_irqhandler));
	ymf.set_addrmap(0, &seibuspi_state::spi_ymf271_map);

	ymf.add_route(0, "lspeaker", 1.0);
	ymf.add_route(1, "rspeaker", 1.0);
//  ymf.add_route(2, "lspeaker", 1.0); Output 2/3 not used?
//  ymf.add_route(3, "rspeaker", 1.0);
}

void seibuspi_state::ejanhs(machine_config &config)
{
	spi(config);

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(seibuspi_state, ejanhs)
}

void seibuspi_state::rdft2(machine_config &config)
{
	spi(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &seibuspi_state::rdft2_map);
}


/* single boards */

MACHINE_RESET_MEMBER(seibuspi_state,sxx2e)
{
	m_z80_bank->set_entry(0);
	m_z80_lastbank = 0;
	m_sb_coin_latch = 0;
}

void seibuspi_state::sxx2e(machine_config &config)
{
	spi(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &seibuspi_state::sxx2e_map);

	m_audiocpu->set_addrmap(AS_PROGRAM, &seibuspi_state::sxx2e_soundmap);

	MCFG_MACHINE_RESET_OVERRIDE(seibuspi_state, sxx2e)

	config.device_remove("soundflash1");
	config.device_remove("soundflash2");

	config.device_remove("soundfifo2");

	/* sound hardware */
	// Single PCBs only output mono sound, SXX2E : unverified
	config.device_remove("lspeaker");
	config.device_remove("rspeaker");
	SPEAKER(config, "mono").front_center();

	ymf271_device &ymf(YMF271(config.replace(), "ymf", 16.9344_MHz_XTAL));
	ymf.irq_handler().set(FUNC(seibuspi_state::ymf_irqhandler));
	ymf.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void seibuspi_state::sxx2f(machine_config &config)
{
	sxx2e(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &seibuspi_state::sxx2f_map);

	config.device_remove("ds2404");

	EEPROM_93C46_16BIT(config, "eeprom");

	// Z80 is Z84C0006PCS instead of Z84C0008PEC
	// clock is unknown, possibly slower than 7.159MHz
}

void seibuspi_state::sxx2g(machine_config &config) // clocks differ, but otherwise same hw as sxx2f
{
	sxx2f(config);

	/* basic machine hardware */
	m_maincpu->set_clock(28.636363_MHz_XTAL); // AMD AM386DX/DX-40, 28.63636MHz
	m_audiocpu->set_clock(4.9512_MHz_XTAL); // Z84C0004PCS, 4.9152MHz

	/* sound hardware */
	ymf271_device &ymf(YMF271(config.replace(), "ymf", 16.384_MHz_XTAL)); // 16.384MHz(!)
	ymf.irq_handler().set(FUNC(seibuspi_state::ymf_irqhandler));
	ymf.add_route(ALL_OUTPUTS, "mono", 1.0);
}


/* SYS386I */

void seibuspi_state::sys386i(machine_config &config)
{
	/* basic machine hardware */
	I386(config, m_maincpu, 40_MHz_XTAL); // AMD 386DX, 40MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &seibuspi_state::sys386i_map);
	m_maincpu->set_vblank_int("screen", FUNC(seibuspi_state::spi_interrupt));
	m_maincpu->set_irq_acknowledge_callback(FUNC(seibuspi_state::spi_irq_callback));

	EEPROM_93C46_16BIT(config, "eeprom");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(PIXEL_CLOCK, SPI_HTOTAL, SPI_HBEND, SPI_HBSTART, SPI_VTOTAL, SPI_VBEND, SPI_VBSTART);
	screen.set_screen_update(FUNC(seibuspi_state::screen_update_spi));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_spi);

	PALETTE(config, m_palette, palette_device::BLACK, 6144);

	seibu_crtc_device &crtc(SEIBU_CRTC(config, "crtc", 0));
	crtc.decrypt_key_callback().set(FUNC(seibuspi_state::tile_decrypt_key_w));
	crtc.layer_en_callback().set(FUNC(seibuspi_state::spi_layer_enable_w));
	crtc.reg_1a_callback().set(FUNC(seibuspi_state::spi_layer_bank_w));
	crtc.layer_scroll_callback().set(FUNC(seibuspi_state::scroll_w));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki[0], 28.636363_MHz_XTAL / 20, okim6295_device::PIN7_HIGH);
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 0.50);

	OKIM6295(config, m_oki[1], 28.636363_MHz_XTAL / 20, okim6295_device::PIN7_HIGH);
	m_oki[1]->add_route(ALL_OUTPUTS, "mono", 0.50);
}


/* SYS386F */

void seibuspi_state::init_sys386f()
{
	u16 *src = (u16 *)memregion("sprites")->base();
	u16 tmp[0x40 / 2], offset;

	// sprite_reorder() only
	for (int i = 0; i < memregion("sprites")->bytes() / 0x40; i++)
	{
		memcpy(tmp, src, 0x40);

		for (int j = 0; j < 0x40 / 2; j++)
		{
			offset = (j >> 1) | (j << 4 & 0x10);
			*src++ = tmp[offset];
		}
	}
}

void seibuspi_state::sys386f(machine_config &config)
{
	/* basic machine hardware */
	I386(config, m_maincpu, XTAL(50'000'000)/2); // Intel i386DX, 25MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &seibuspi_state::sys386f_map);
	m_maincpu->set_vblank_int("screen", FUNC(seibuspi_state::spi_interrupt));
	m_maincpu->set_irq_acknowledge_callback(FUNC(seibuspi_state::spi_irq_callback));

	EEPROM_93C46_16BIT(config, "eeprom");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(57.59);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 0*8, 30*8-1);
	screen.set_screen_update(FUNC(seibuspi_state::screen_update_sys386f));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sys386f);

	PALETTE(config, m_palette, palette_device::BLACK, 8192);

	MCFG_VIDEO_START_OVERRIDE(seibuspi_state, sys386f)

	/* sound hardware */
	// Single PCBs only output mono sound
	SPEAKER(config, "mono").front_center();

	YMZ280B(config, "ymz", XTAL(16'384'000)).add_route(ALL_OUTPUTS, "mono", 1.0);
}


/*****************************************************************************/

void seibuspi_state::init_senkyu()
{
	if (ENABLE_SPEEDUP_HACKS) m_maincpu->space(AS_PROGRAM).install_read_handler(0x0018cb4, 0x0018cb7, read32smo_delegate(*this, FUNC(seibuspi_state::senkyu_speedup_r)));
	init_sei252();
}

void seibuspi_state::init_senkyua()
{
	if (ENABLE_SPEEDUP_HACKS) m_maincpu->space(AS_PROGRAM).install_read_handler(0x0018c9c, 0x0018c9f, read32smo_delegate(*this, FUNC(seibuspi_state::senkyua_speedup_r)));
	init_sei252();
}

void seibuspi_state::init_batlball()
{
	if (ENABLE_SPEEDUP_HACKS) m_maincpu->space(AS_PROGRAM).install_read_handler(0x0018db4, 0x0018db7, read32smo_delegate(*this, FUNC(seibuspi_state::batlball_speedup_r)));
	init_sei252();
}

void seibuspi_state::init_viprp1()
{
	if (ENABLE_SPEEDUP_HACKS) m_maincpu->space(AS_PROGRAM).install_read_handler(0x001e2e0, 0x001e2e3, read32smo_delegate(*this, FUNC(seibuspi_state::viprp1_speedup_r)));
	init_sei252();
}

void seibuspi_state::init_viprp1o()
{
	if (ENABLE_SPEEDUP_HACKS) m_maincpu->space(AS_PROGRAM).install_read_handler(0x001d49c, 0x001d49f, read32smo_delegate(*this, FUNC(seibuspi_state::viprp1o_speedup_r)));
	init_sei252();
}

void seibuspi_state::init_ejanhs()
{
	// idle skip doesn't work properly?
	if (false && ENABLE_SPEEDUP_HACKS) m_maincpu->space(AS_PROGRAM).install_read_handler(0x002d224, 0x002d227, read32smo_delegate(*this, FUNC(seibuspi_state::ejanhs_speedup_r)));
	init_sei252();
}

void seibuspi_state::init_rdft()
{
	if (ENABLE_SPEEDUP_HACKS) m_maincpu->space(AS_PROGRAM).install_read_handler(0x00298d0, 0x00298d3, read32smo_delegate(*this, FUNC(seibuspi_state::rdft_speedup_r)));
	init_sei252();
}

void seibuspi_state::init_rdft2()
{
	if (ENABLE_SPEEDUP_HACKS) m_maincpu->space(AS_PROGRAM).install_read_handler(0x00282ac, 0x00282af, read32smo_delegate(*this, FUNC(seibuspi_state::rf2_speedup_r)));

	rdft2_text_decrypt(memregion("chars")->base());
	rdft2_bg_decrypt(memregion("tiles")->base(), memregion("tiles")->bytes());
	seibuspi_rise10_sprite_decrypt(memregion("sprites")->base(), 0x600000);
	init_spi_common();
}

void seibuspi_state::init_rfjet()
{
	if (ENABLE_SPEEDUP_HACKS) m_maincpu->space(AS_PROGRAM).install_read_handler(0x002894c, 0x002894f, read32smo_delegate(*this, FUNC(seibuspi_state::rfjet_speedup_r)));

	rfjet_text_decrypt(memregion("chars")->base());
	rfjet_bg_decrypt(memregion("tiles")->base(), memregion("tiles")->bytes());
	seibuspi_rise11_sprite_decrypt_rfjet(memregion("sprites")->base(), 0x800000);
	init_spi_common();
}


u32 seibuspi_state::senkyu_speedup_r()
{
	if (m_maincpu->pc()==0x00305bb2) m_maincpu->spin_until_interrupt(); // idle

	return m_mainram[0x0018cb4/4];
}

u32 seibuspi_state::senkyua_speedup_r()
{
	if (m_maincpu->pc()== 0x30582e) m_maincpu->spin_until_interrupt(); // idle

	return m_mainram[0x0018c9c/4];
}

u32 seibuspi_state::batlball_speedup_r()
{
//  printf("m_maincpu->pc() %06x\n", m_maincpu->pc());

	/* batlbalu */
	if (m_maincpu->pc()==0x00305996) m_maincpu->spin_until_interrupt(); // idle

	/* batlball */
	if (m_maincpu->pc()==0x003058aa) m_maincpu->spin_until_interrupt(); // idle

	return m_mainram[0x0018db4/4];
}

u32 seibuspi_state::viprp1_speedup_r()
{
	/* viprp1 */
	if (m_maincpu->pc()==0x0202769) m_maincpu->spin_until_interrupt(); // idle

	/* viprp1s */
	if (m_maincpu->pc()==0x02027e9) m_maincpu->spin_until_interrupt(); // idle

	/* viprp1ot */
	if (m_maincpu->pc()==0x02026bd) m_maincpu->spin_until_interrupt(); // idle

//  osd_printf_debug("%08x\n",m_maincpu->pc());

	return m_mainram[0x001e2e0/4];
}

u32 seibuspi_state::viprp1o_speedup_r()
{
	/* viperp1o */
	if (m_maincpu->pc()==0x0201f99) m_maincpu->spin_until_interrupt(); // idle
//  osd_printf_debug("%08x\n",m_maincpu->pc());
	return m_mainram[0x001d49c/4];
}

// causes input problems?
u32 seibuspi_state::ejanhs_speedup_r()
{
	if (m_maincpu->pc()==0x03032c7) m_maincpu->spin_until_interrupt(); // idle
//  osd_printf_debug("%08x\n",m_maincpu->pc());
	return m_mainram[0x002d224/4];
}

u32 seibuspi_state::rdft_speedup_r()
{
	/* rdft */
	if (m_maincpu->pc()==0x0203f06) m_maincpu->spin_until_interrupt(); // idle

	/* rdftj? */
	if (m_maincpu->pc()==0x0203f0a) m_maincpu->spin_until_interrupt(); // idle

	/* rdftau */
	if (m_maincpu->pc()==0x0203f16) m_maincpu->spin_until_interrupt(); // idle

	/* rdftja? */
	if (m_maincpu->pc()==0x0203f22) m_maincpu->spin_until_interrupt(); // idle

	/* rdfta, rdftadi, rdftam, rdftit */
	if (m_maincpu->pc()==0x0203f46) m_maincpu->spin_until_interrupt(); // idle

	/* rdftu */
	if (m_maincpu->pc()==0x0203f3a) m_maincpu->spin_until_interrupt(); // idle

	/* rdftauge */
	if (m_maincpu->pc()==0x0203f6e) m_maincpu->spin_until_interrupt(); // idle

//  osd_printf_debug("%08x\n",m_maincpu->pc());

	return m_mainram[0x00298d0/4];
}

u32 seibuspi_state::rf2_speedup_r()
{
	/* rdft22kc */
	if (m_maincpu->pc()==0x0203926) m_maincpu->spin_until_interrupt(); // idle

	/* rdft2, rdft2j */
	if (m_maincpu->pc()==0x0204372) m_maincpu->spin_until_interrupt(); // idle

	/* rdft2us */
	if (m_maincpu->pc()==0x020420e) m_maincpu->spin_until_interrupt(); // idle

	/* rdft2a */
	if (m_maincpu->pc()==0x0204366) m_maincpu->spin_until_interrupt(); // idle

//  osd_printf_debug("%08x\n",m_maincpu->pc());

	return m_mainram[0x0282ac/4];
}

u32 seibuspi_state::rfjet_speedup_r()
{
	/* rfjet, rfjetu, rfjeta */
	if (m_maincpu->pc()==0x0206082) m_maincpu->spin_until_interrupt(); // idle

	/* rfjetus */
	if (m_maincpu->pc()==0x0205b39)
	{
		u32 r;
		m_maincpu->spin_until_interrupt(); // idle
		// Hack to enter test mode
		r = m_mainram[0x002894c/4] & (~0x400);
		return r | (((ioport("SYSTEM")->read() ^ 0xff)<<8) & 0x400);
	}

	/* rfjetj */
	if (m_maincpu->pc()==0x0205f2e) m_maincpu->spin_until_interrupt(); // idle

//  osd_printf_debug("%08x\n",m_maincpu->pc());

	return m_mainram[0x002894c/4];
}


/*****************************************************************************/

#define ROM_LOAD24_BYTE(name,offset,length,hash)        ROMX_LOAD(name, offset, length, hash, ROM_SKIP(2))
#define ROM_LOAD24_WORD(name,offset,length,hash)        ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_SKIP(1) | ROM_REVERSE)
#define ROM_LOAD24_WORD_SWAP(name,offset,length,hash)   ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_SKIP(1))

/* SPI games */

ROM_START( senkyu )
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("fb_1.211", 0x100000, 0x40000, CRC(20a3e5db) SHA1(f1109aeceac7993abc9093d09429718ffc292c77) )
	ROM_LOAD32_BYTE("fb_2.212", 0x100001, 0x40000, CRC(38e90619) SHA1(451ab5f4a5935bb779f9c245c1c4358e80d93c15) )
	ROM_LOAD32_BYTE("fb_3.210", 0x100002, 0x40000, CRC(226f0429) SHA1(69d0fe6671278d7fe215e455bb50abf631cdb484) )
	ROM_LOAD32_BYTE("fb_4.29",  0x100003, 0x40000, CRC(b46d66b7) SHA1(1acd0fea9384e1488b44661e0c99b9672a3f9803) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_WORD("fb_6.413", 0x000000, 0x20000, CRC(b57115c9) SHA1(eb95f416f522032ca949bfb6348f1ff824101f2d) )
	ROM_LOAD24_BYTE("fb_5.48",  0x000002, 0x10000, CRC(440a9ae3) SHA1(3f57e6da91f0dac2d816c873759f1e1d3259caf1) )

	ROM_REGION( 0x300000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("fb_bg-1d.415", 0x000000, 0x200000, CRC(eae7a1fc) SHA1(26d8a9f4e554848977ec1f6a8aad8751b558a8d4) )
	ROM_LOAD24_BYTE("fb_bg-1p.410", 0x000002, 0x100000, CRC(b46e774e) SHA1(00b6c1d0b0ea37f4354acab543b270c0bf45896d) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("fb_obj-1.322", 0x000000, 0x400000, CRC(29f86f68) SHA1(1afe809ce00a25f8b27543e4188edc3e3e604951) )
	ROM_LOAD("fb_obj-2.324", 0x400000, 0x400000, CRC(c9e3130b) SHA1(12b5d5363142e8efb3b7fc44289c0afffa5011c6) )
	ROM_LOAD("fb_obj-3.323", 0x800000, 0x400000, CRC(f6c3bc49) SHA1(d0eb9c6aa3954d94e3a442a48e0fe6cc279f5513) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_BYTE("fb_pcm-1.215",  0x000000, 0x080000, CRC(1d83891c) SHA1(09502437562275c14c0f3a0e62b19e91bedb4693) )
	ROM_CONTINUE(                    0x400000, 0x080000 )
	ROM_LOAD32_BYTE("fb_7.216",      0x800000, 0x080000, CRC(874d7b59) SHA1(0236753636c9a818780b23f5f506697b9f6d93c7) )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region01.u1053", 0x000000, 0x100000, CRC(7ae7ab76) SHA1(a2b196f470bf64af94002fc4e2640fadad00418f) )
ROM_END

ROM_START( senkyua )
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("1.bin", 0x100000, 0x40000, CRC(6102c3fb) SHA1(4a55b41d916768f9601513db973b82077bca47c5) )
	ROM_LOAD32_BYTE("2.bin", 0x100001, 0x40000, CRC(d5b8ce46) SHA1(f6e4b8f51146179efb52ecb2b72fdeaee10b7282) )
	ROM_LOAD32_BYTE("3.bin", 0x100002, 0x40000, CRC(e27ceccd) SHA1(3d6b8e97e89939c72d1a5a4a3856025b5f548645) )
	ROM_LOAD32_BYTE("4.bin", 0x100003, 0x40000, CRC(7c6d4549) SHA1(efc6920a2e518afe849fb6fe191e7cd0bc483be5) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_WORD("fb_6.413", 0x000000, 0x20000, CRC(b57115c9) SHA1(eb95f416f522032ca949bfb6348f1ff824101f2d) )
	ROM_LOAD24_BYTE("fb_5.48",  0x000002, 0x10000, CRC(440a9ae3) SHA1(3f57e6da91f0dac2d816c873759f1e1d3259caf1) )

	ROM_REGION( 0x300000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("fb_bg-1d.415", 0x000000, 0x200000, CRC(eae7a1fc) SHA1(26d8a9f4e554848977ec1f6a8aad8751b558a8d4) )
	ROM_LOAD24_BYTE("fb_bg-1p.410", 0x000002, 0x100000, CRC(b46e774e) SHA1(00b6c1d0b0ea37f4354acab543b270c0bf45896d) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("fb_obj-1.322", 0x000000, 0x400000, CRC(29f86f68) SHA1(1afe809ce00a25f8b27543e4188edc3e3e604951) )
	ROM_LOAD("fb_obj-2.324", 0x400000, 0x400000, CRC(c9e3130b) SHA1(12b5d5363142e8efb3b7fc44289c0afffa5011c6) )
	ROM_LOAD("fb_obj-3.323", 0x800000, 0x400000, CRC(f6c3bc49) SHA1(d0eb9c6aa3954d94e3a442a48e0fe6cc279f5513) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_BYTE("fb_pcm-1.215",  0x000000, 0x080000, CRC(1d83891c) SHA1(09502437562275c14c0f3a0e62b19e91bedb4693) )
	ROM_CONTINUE(                    0x400000, 0x080000 )
	ROM_LOAD32_BYTE("fb_7.216",      0x800000, 0x080000, CRC(874d7b59) SHA1(0236753636c9a818780b23f5f506697b9f6d93c7) )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region01.u1053", 0x000000, 0x100000, CRC(7ae7ab76) SHA1(a2b196f470bf64af94002fc4e2640fadad00418f) )
ROM_END

ROM_START( batlball )
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("1.211", 0x100000, 0x40000, CRC(d4e48f89) SHA1(10e43a9ff3f6f169de6352280a8a06e7f482271a) )
	ROM_LOAD32_BYTE("2.212", 0x100001, 0x40000, CRC(3077720b) SHA1(b65c3d02ac75eb56e0c5dc1bf6bb6a4e445a41cf) )
	ROM_LOAD32_BYTE("3.210", 0x100002, 0x40000, CRC(520d31e1) SHA1(998ae968113ab5b2891044187d93793903c13452) )
	ROM_LOAD32_BYTE("4.029", 0x100003, 0x40000, CRC(22419b78) SHA1(67475a654d4ad94e5dfda88cbe2f9c1b5ba6d2cc) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_WORD("fb_6.413", 0x000000, 0x20000, CRC(b57115c9) SHA1(eb95f416f522032ca949bfb6348f1ff824101f2d) )
	ROM_LOAD24_BYTE("fb_5.48",  0x000002, 0x10000, CRC(440a9ae3) SHA1(3f57e6da91f0dac2d816c873759f1e1d3259caf1) )

	ROM_REGION( 0x300000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("fb_bg-1d.415", 0x000000, 0x200000, CRC(eae7a1fc) SHA1(26d8a9f4e554848977ec1f6a8aad8751b558a8d4) )
	ROM_LOAD24_BYTE("fb_bg-1p.410", 0x000002, 0x100000, CRC(b46e774e) SHA1(00b6c1d0b0ea37f4354acab543b270c0bf45896d) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("fb_obj-1.322", 0x000000, 0x400000, CRC(29f86f68) SHA1(1afe809ce00a25f8b27543e4188edc3e3e604951) )
	ROM_LOAD("fb_obj-2.324", 0x400000, 0x400000, CRC(c9e3130b) SHA1(12b5d5363142e8efb3b7fc44289c0afffa5011c6) )
	ROM_LOAD("fb_obj-3.323", 0x800000, 0x400000, CRC(f6c3bc49) SHA1(d0eb9c6aa3954d94e3a442a48e0fe6cc279f5513) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_BYTE("fb_pcm-1.215",  0x000000, 0x080000, CRC(1d83891c) SHA1(09502437562275c14c0f3a0e62b19e91bedb4693) )
	ROM_CONTINUE(                    0x400000, 0x080000 )
	ROM_LOAD32_BYTE("fb_7.216",      0x800000, 0x080000, CRC(874d7b59) SHA1(0236753636c9a818780b23f5f506697b9f6d93c7) )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region80.u1053", 0x000000, 0x100000, CRC(e2adaff5) SHA1(9297afaf78209724515d8f78de8cee7bc7cb796b) )
ROM_END

ROM_START( batlballo ) // Board has a low serial number 000014 and PCB date is 95.10.02
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("seibu_1a.211", 0x100000, 0x40000, CRC(90340e8c) SHA1(303d3c5ffc1a64e1e4aa614105119d9d7768f516) )
	ROM_LOAD32_BYTE("seibu_2a.212", 0x100001, 0x40000, CRC(db655d3e) SHA1(bfd873e0d0daf3759778c76fe72fcf96e84250a4) )
	ROM_LOAD32_BYTE("seibu_3a.210", 0x100002, 0x40000, CRC(659a54a2) SHA1(a6c024e42b104a6198829f0f75baaa294fe9de6c) )
	ROM_LOAD32_BYTE("seibu_4a.029", 0x100003, 0x40000, CRC(51183421) SHA1(d97da1693e429cb7a061f08274070eb3e6966ec0) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_WORD("seibu_6.413", 0x000000, 0x20000, CRC(338556f9) SHA1(dfab6e1562dd9c373aa094a3791ecd4cd3c9b6f5) )
	ROM_LOAD24_BYTE("seibu_5.48",  0x000002, 0x10000, CRC(6ccfb72e) SHA1(825b917ecd8495de23d55d2d2902d9d7c54ce4ed) )

	ROM_REGION( 0x300000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("fb_bg-1d.415", 0x000000, 0x200000, CRC(eae7a1fc) SHA1(26d8a9f4e554848977ec1f6a8aad8751b558a8d4) )
	ROM_LOAD24_BYTE("fb_bg-1p.410", 0x000002, 0x100000, CRC(b46e774e) SHA1(00b6c1d0b0ea37f4354acab543b270c0bf45896d) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("fb_obj-1.322", 0x000000, 0x400000, CRC(29f86f68) SHA1(1afe809ce00a25f8b27543e4188edc3e3e604951) )
	ROM_LOAD("fb_obj-2.324", 0x400000, 0x400000, CRC(c9e3130b) SHA1(12b5d5363142e8efb3b7fc44289c0afffa5011c6) )
	ROM_LOAD("fb_obj-3.323", 0x800000, 0x400000, CRC(f6c3bc49) SHA1(d0eb9c6aa3954d94e3a442a48e0fe6cc279f5513) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_BYTE("fb_pcm-1.215",  0x000000, 0x080000, CRC(1d83891c) SHA1(09502437562275c14c0f3a0e62b19e91bedb4693) )
	ROM_CONTINUE(                    0x400000, 0x080000 )
	ROM_LOAD32_BYTE("seibu_7.216",   0x800000, 0x080000, CRC(874d7b59) SHA1(0236753636c9a818780b23f5f506697b9f6d93c7) )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region80.u1053", 0x000000, 0x100000, CRC(e2adaff5) SHA1(9297afaf78209724515d8f78de8cee7bc7cb796b) )
ROM_END

ROM_START( batlballa )
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("senkyua1.bin", 0x100000, 0x40000, CRC(ec3c4d4d) SHA1(6c57b8fbb77ce1615850842d06c054e88e240eef) )
	ROM_LOAD32_BYTE("2.212",        0x100001, 0x40000, CRC(3077720b) SHA1(b65c3d02ac75eb56e0c5dc1bf6bb6a4e445a41cf) )
	ROM_LOAD32_BYTE("3.210",        0x100002, 0x40000, CRC(520d31e1) SHA1(998ae968113ab5b2891044187d93793903c13452) )
	ROM_LOAD32_BYTE("4.029",        0x100003, 0x40000, CRC(22419b78) SHA1(67475a654d4ad94e5dfda88cbe2f9c1b5ba6d2cc) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_WORD("fb_6.413", 0x000000, 0x20000, CRC(b57115c9) SHA1(eb95f416f522032ca949bfb6348f1ff824101f2d) )
	ROM_LOAD24_BYTE("fb_5.48",  0x000002, 0x10000, CRC(440a9ae3) SHA1(3f57e6da91f0dac2d816c873759f1e1d3259caf1) )

	ROM_REGION( 0x300000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("fb_bg-1d.415", 0x000000, 0x200000, CRC(eae7a1fc) SHA1(26d8a9f4e554848977ec1f6a8aad8751b558a8d4) )
	ROM_LOAD24_BYTE("fb_bg-1p.410", 0x000002, 0x100000, CRC(b46e774e) SHA1(00b6c1d0b0ea37f4354acab543b270c0bf45896d) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("fb_obj-1.322", 0x000000, 0x400000, CRC(29f86f68) SHA1(1afe809ce00a25f8b27543e4188edc3e3e604951) )
	ROM_LOAD("fb_obj-2.324", 0x400000, 0x400000, CRC(c9e3130b) SHA1(12b5d5363142e8efb3b7fc44289c0afffa5011c6) )
	ROM_LOAD("fb_obj-3.323", 0x800000, 0x400000, CRC(f6c3bc49) SHA1(d0eb9c6aa3954d94e3a442a48e0fe6cc279f5513) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_BYTE("fb_pcm-1.215",  0x000000, 0x080000, CRC(1d83891c) SHA1(09502437562275c14c0f3a0e62b19e91bedb4693) )
	ROM_CONTINUE(                    0x400000, 0x080000 )
	ROM_LOAD32_BYTE("fb_7.216",      0x800000, 0x080000, CRC(874d7b59) SHA1(0236753636c9a818780b23f5f506697b9f6d93c7) )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region22.u1053", 0x000000, 0x100000, CRC(5fee8413) SHA1(6d6a62fa01293b4ba4b349a39820d024add6ea22) )
ROM_END

ROM_START( batlballe ) /* Early version, PCB serial number of 19, hand written labels dated 10/16 (Oct 16, 1995) */
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("1_10-16", 0x100000, 0x40000, CRC(6b1baa07) SHA1(29b8f4016e9bffdcdb6ec405cd443ca0a80de5d5) )
	ROM_LOAD32_BYTE("2_10-16", 0x100001, 0x40000, CRC(3c890639) SHA1(968c4a5efc5ebbe4e4cc81f834c286c02596c24e) )
	ROM_LOAD32_BYTE("3_10-16", 0x100002, 0x40000, CRC(8c30180e) SHA1(47b99b04e2e74f1ee5095aed3f45aba66cd3da3f) )
	ROM_LOAD32_BYTE("4_10-16", 0x100003, 0x40000, CRC(048c7aaa) SHA1(5eca2cdf4e6f077988509c3c42c86408b21ffbf1) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_WORD("fb_6.413", 0x000000, 0x20000, CRC(b57115c9) SHA1(eb95f416f522032ca949bfb6348f1ff824101f2d) )
	ROM_LOAD24_BYTE("fb_5.48",  0x000002, 0x10000, CRC(440a9ae3) SHA1(3f57e6da91f0dac2d816c873759f1e1d3259caf1) )

	ROM_REGION( 0x300000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("fb_bg-1d.415", 0x000000, 0x200000, CRC(eae7a1fc) SHA1(26d8a9f4e554848977ec1f6a8aad8751b558a8d4) )
	ROM_LOAD24_BYTE("fb_bg-1p.410", 0x000002, 0x100000, CRC(b46e774e) SHA1(00b6c1d0b0ea37f4354acab543b270c0bf45896d) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("fb_obj-1.322", 0x000000, 0x400000, CRC(29f86f68) SHA1(1afe809ce00a25f8b27543e4188edc3e3e604951) )
	ROM_LOAD("fb_obj-2.324", 0x400000, 0x400000, CRC(c9e3130b) SHA1(12b5d5363142e8efb3b7fc44289c0afffa5011c6) )
	ROM_LOAD("fb_obj-3.323", 0x800000, 0x400000, CRC(f6c3bc49) SHA1(d0eb9c6aa3954d94e3a442a48e0fe6cc279f5513) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_BYTE("fb_pcm-1.215",  0x000000, 0x080000, CRC(1d83891c) SHA1(09502437562275c14c0f3a0e62b19e91bedb4693) )
	ROM_CONTINUE(                    0x400000, 0x080000 )
	ROM_LOAD32_BYTE("fb_7.216",      0x800000, 0x080000, CRC(874d7b59) SHA1(0236753636c9a818780b23f5f506697b9f6d93c7) )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region22.u1053", 0x000000, 0x100000, CRC(5fee8413) SHA1(6d6a62fa01293b4ba4b349a39820d024add6ea22) )
ROM_END

ROM_START( batlballu )
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("sen1.bin", 0x100000, 0x40000, CRC(13849bf0) SHA1(ffa829a8b8a05a8fbaf883a30759f2ad8071a85b) )
	ROM_LOAD32_BYTE("sen2.bin", 0x100001, 0x40000, CRC(2ae5f7e2) SHA1(cef9ddea8b1d21f20a48c2523c9420c1800720c8) )
	ROM_LOAD32_BYTE("sen3.bin", 0x100002, 0x40000, CRC(98e6f19f) SHA1(433f8463e63bba32730d3c098354f8c95257df3f) )
	ROM_LOAD32_BYTE("sen4.bin", 0x100003, 0x40000, CRC(1343ec56) SHA1(8ecc8d7b425ff6512ffa969a7f26423fa50ad258) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_WORD("fb_6.413", 0x000000, 0x20000, CRC(b57115c9) SHA1(eb95f416f522032ca949bfb6348f1ff824101f2d) )
	ROM_LOAD24_BYTE("fb_5.48",  0x000002, 0x10000, CRC(440a9ae3) SHA1(3f57e6da91f0dac2d816c873759f1e1d3259caf1) )

	ROM_REGION( 0x300000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("fb_bg-1d.415", 0x000000, 0x200000, CRC(eae7a1fc) SHA1(26d8a9f4e554848977ec1f6a8aad8751b558a8d4) )
	ROM_LOAD24_BYTE("fb_bg-1p.410", 0x000002, 0x100000, CRC(b46e774e) SHA1(00b6c1d0b0ea37f4354acab543b270c0bf45896d) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("fb_obj-1.322", 0x000000, 0x400000, CRC(29f86f68) SHA1(1afe809ce00a25f8b27543e4188edc3e3e604951) )
	ROM_LOAD("fb_obj-2.324", 0x400000, 0x400000, CRC(c9e3130b) SHA1(12b5d5363142e8efb3b7fc44289c0afffa5011c6) )
	ROM_LOAD("fb_obj-3.323", 0x800000, 0x400000, CRC(f6c3bc49) SHA1(d0eb9c6aa3954d94e3a442a48e0fe6cc279f5513) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_BYTE("fb_pcm-1.215",  0x000000, 0x080000, CRC(1d83891c) SHA1(09502437562275c14c0f3a0e62b19e91bedb4693) )
	ROM_CONTINUE(                    0x400000, 0x080000 )
	ROM_LOAD32_BYTE("fb_7.216",      0x800000, 0x080000, CRC(874d7b59) SHA1(0236753636c9a818780b23f5f506697b9f6d93c7) )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region10.u1053", 0x000000, 0x100000, CRC(4319d998) SHA1(a064ce647453a9b3bccf7f1d6d0d52b5a72e09dd) )
ROM_END

ROM_START( batlballpt ) /* SXX2C ROM SUB cart - only U0211 labeled with the date 171195 */
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("senkyu_prog0_171195.u0211", 0x100000, 0x40000, CRC(dcc227b6) SHA1(10d72ebdd218887521226c82a61c2cdebcd9af99) )
	ROM_LOAD32_BYTE("2.u0212", 0x100001, 0x40000, CRC(03ab203f) SHA1(de5771883624440a9842b109b764b52811facfad) )
	ROM_LOAD32_BYTE("3.u0210", 0x100002, 0x40000, CRC(9eb9c8b4) SHA1(fc4f8feac3840776f842d03d86b37a669bba4f12) )
	ROM_LOAD32_BYTE("4.u029",  0x100003, 0x40000, CRC(c37ae2a5) SHA1(ee33f7baf852ef36b4f7f71a08d82acf1e7460b8) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_WORD("fb_6.413", 0x000000, 0x20000, CRC(b57115c9) SHA1(eb95f416f522032ca949bfb6348f1ff824101f2d) )
	ROM_LOAD24_BYTE("fb_5.48",  0x000002, 0x10000, CRC(440a9ae3) SHA1(3f57e6da91f0dac2d816c873759f1e1d3259caf1) )

	ROM_REGION( 0x300000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("fb_bg-1d.415", 0x000000, 0x200000, CRC(eae7a1fc) SHA1(26d8a9f4e554848977ec1f6a8aad8751b558a8d4) )
	ROM_LOAD24_BYTE("fb_bg-1p.410", 0x000002, 0x100000, CRC(b46e774e) SHA1(00b6c1d0b0ea37f4354acab543b270c0bf45896d) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("fb_obj-1.322", 0x000000, 0x400000, CRC(29f86f68) SHA1(1afe809ce00a25f8b27543e4188edc3e3e604951) )
	ROM_LOAD("fb_obj-2.324", 0x400000, 0x400000, CRC(c9e3130b) SHA1(12b5d5363142e8efb3b7fc44289c0afffa5011c6) )
	ROM_LOAD("fb_obj-3.323", 0x800000, 0x400000, CRC(f6c3bc49) SHA1(d0eb9c6aa3954d94e3a442a48e0fe6cc279f5513) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_BYTE("fb_pcm-1.215",  0x000000, 0x080000, CRC(1d83891c) SHA1(09502437562275c14c0f3a0e62b19e91bedb4693) )
	ROM_CONTINUE(                    0x400000, 0x080000 )
	ROM_LOAD32_BYTE("fb_7.216",      0x800000, 0x080000, CRC(874d7b59) SHA1(0236753636c9a818780b23f5f506697b9f6d93c7) )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region96.u1053", 0x000000, 0x100000, CRC(a0ebae75) SHA1(31f7955a529a4e2492b530e54878ed7a13f49c94) )
ROM_END


ROM_START( ejanhs )
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("ejan3_1.211", 0x100000, 0x40000, CRC(e626d3d2) SHA1(d23cb5e218a85e09de98fa966afbfd43090b396e) )
	ROM_LOAD32_BYTE("ejan3_2.212", 0x100001, 0x40000, CRC(83c39da2) SHA1(9526ffb5d5becccf0aa2e338ab4a3c873d575e6f) )
	ROM_LOAD32_BYTE("ejan3_3.210", 0x100002, 0x40000, CRC(46897b7d) SHA1(a22e0467c016e72bf99df2c1e6ecc792b2151b15) )
	ROM_LOAD32_BYTE("ejan3_4.29",  0x100003, 0x40000, CRC(b3187a2b) SHA1(7fc11ed5ceb2e45f784e75307fef8b850a981a2e) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_WORD("ejan3_6.413", 0x000000, 0x20000, CRC(837e012c) SHA1(815452083b65885d6e66dfc058ceec81bb3e6678) )
	ROM_LOAD24_BYTE("ejan3_5.48",  0x000002, 0x10000, CRC(d62db7bf) SHA1(c88f1bb6106c59179b914962ed8cdd4095fd9ce8) )

	ROM_REGION( 0x600000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("ej3_bg1d.415", 0x000000, 0x200000, CRC(bcacabe0) SHA1(b73581cf923196326b5b0b99e6aedb915bab0880) )
	ROM_LOAD24_BYTE("ej3_bg1p.410", 0x000002, 0x100000, CRC(1fd0eb5e) SHA1(ca64c8020b246128232f4f6c0a0a2dd9cd3efeae) )
	ROM_LOAD24_WORD("ej3_bg2d.416", 0x300000, 0x100000, CRC(ea2acd69) SHA1(b796e9e4b7342bf452f5ffdbce32cfefc603ba0f) )
	ROM_LOAD24_BYTE("ej3_bg2p.49",  0x300002, 0x080000, CRC(a4a9cb0f) SHA1(da177d13bb95bf6b987d3ca13bcdc86570807b2c) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("ej3_obj1.322", 0x000000, 0x400000, CRC(852f180e) SHA1(d4845dace45c05a68f3b38ccb301c5bf5dce4174) )
	ROM_LOAD("ej3_obj2.324", 0x400000, 0x400000, CRC(1116ad08) SHA1(d5c81383b3f9ede7dd03e6be35487b40740b1f8f) )
	ROM_LOAD("ej3_obj3.323", 0x800000, 0x400000, CRC(ccfe02b6) SHA1(368bc8efe9d6677ba3d0cfc0f450a4bda32988be) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_BYTE("ej3_pcm1.215",  0x000000, 0x080000, CRC(a92a3a82) SHA1(b86c27c5a2831ddd2a1c2b071018a99afec14018) )
	ROM_CONTINUE(                    0x400000, 0x080000 )
	ROM_LOAD32_BYTE("ejan3_7.216",   0x800000, 0x080000, CRC(c6fc6bcf) SHA1(d4d8c06d295f8eacfa10c21dbab5858f936121f3) )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region01.u1053", 0x000000, 0x100000, CRC(7ae7ab76) SHA1(a2b196f470bf64af94002fc4e2640fadad00418f) )
ROM_END


ROM_START( viprp1 )
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("seibu1.211", 0x000000, 0x80000, CRC(e5caf4ff) SHA1(7c87a4e8e8dacfb7cc0be8f778352bce2801e59b) )
	ROM_LOAD32_BYTE("seibu2.212", 0x000001, 0x80000, CRC(688a998e) SHA1(0c48374b6800cd00e3ee96c0fb12119a680b091d) )
	ROM_LOAD32_BYTE("seibu3.210", 0x000002, 0x80000, CRC(990fa76a) SHA1(7619a631d6f83b3677eb47f984aff684e9518d6d) )
	ROM_LOAD32_BYTE("seibu4.29",  0x000003, 0x80000, CRC(13e3e343) SHA1(aac0c7450059847f53b5081e4abf26303a50f999) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_WORD("seibu5.u0413", 0x000000, 0x20000, CRC(5ece677c) SHA1(b782cf3296f866f79fafa69ff719211c9d4026df) )
	ROM_LOAD24_BYTE("seibu6.u048",  0x000002, 0x10000, CRC(44844ef8) SHA1(bcbe24d2ffb64f9165ba4ab7de27f44b99b5ff5a) )

	ROM_REGION( 0x600000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("v_bg-11.415",  0x000000, 0x200000, CRC(6fc96736) SHA1(12df47d8af2c1febc1bce5bcf3218766447885bd) )
	ROM_LOAD24_BYTE("v_bg-12.415",  0x000002, 0x100000, CRC(d3c7281c) SHA1(340bca1f31486609b3c34dd7830362a216ff648e) )
	ROM_LOAD24_WORD("v_bg-21.410",  0x300000, 0x100000, CRC(d65b4318) SHA1(6522970d95ffa7fa2f32e0b5b4f0eb69e0286b36) )
	ROM_LOAD24_BYTE("v_bg-22.416",  0x300002, 0x080000, CRC(24a0a23a) SHA1(0b0330717620e3f3274a25845d9edaf8023b9db2) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("v_obj-1.322",  0x000000, 0x400000, CRC(3be5b631) SHA1(fd1064428d28ca166a9267b968c0ba846cfed656) )
	ROM_LOAD("v_obj-2.324",  0x400000, 0x400000, CRC(924153b4) SHA1(db5dadcfb4cd5e6efe9d995085936ce4f4eb4254) )
	ROM_LOAD("v_obj-3.323",  0x800000, 0x400000, CRC(e9fb9062) SHA1(18e97b4c5cced2b529e6e72d8041c6f78fcec76e) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_BYTE("v_pcm.215",  0x000000, 0x080000, CRC(e3111b60) SHA1(f7a7747f29c392876e43efcb4e6c0741454082f2) )
	ROM_CONTINUE(                 0x400000, 0x080000 )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_regionbe.u1053", 0x000000, 0x100000, CRC(a4c181d0) SHA1(0aeea4cac4030f60ee77d62deca6b67c318c0866) )
ROM_END

ROM_START( viprp1k )
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("seibu1.211", 0x000000, 0x80000, CRC(5495c930) SHA1(056237965aefa4c0ea7782e0ee5ba1b58a045d7a) ) // sldh
	ROM_LOAD32_BYTE("seibu2.212", 0x000001, 0x80000, CRC(e0ad22ae) SHA1(1911d17f0b462a9bada9efee85e531f2445e4ac6) ) // sldh
	ROM_LOAD32_BYTE("seibu3.210", 0x000002, 0x80000, CRC(db7bcb90) SHA1(45f0a44e24d7b4b996d833e579f405bcf7584563) ) // sldh
	ROM_LOAD32_BYTE("seibu4.29",  0x000003, 0x80000, CRC(c6188bf9) SHA1(746a205c428a080c36d0daf1d4f6b2b0f8efb977) ) // sldh

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_WORD("seibu5.u0413", 0x000000, 0x20000, CRC(1a35f2d8) SHA1(cd9b140f144a8c72756e18913eaef121963be341) ) // sldh
	ROM_LOAD24_BYTE("seibu6.u048",  0x000002, 0x10000, CRC(e88bf049) SHA1(62f35840dc90b505118ed81ac0d75397a689783b) ) // sldh

	ROM_REGION( 0x600000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("v_bg-11.415",  0x000000, 0x200000, CRC(6fc96736) SHA1(12df47d8af2c1febc1bce5bcf3218766447885bd) )
	ROM_LOAD24_BYTE("v_bg-12.415",  0x000002, 0x100000, CRC(d3c7281c) SHA1(340bca1f31486609b3c34dd7830362a216ff648e) )
	ROM_LOAD24_WORD("v_bg-21.410",  0x300000, 0x100000, CRC(d65b4318) SHA1(6522970d95ffa7fa2f32e0b5b4f0eb69e0286b36) )
	ROM_LOAD24_BYTE("v_bg-22.416",  0x300002, 0x080000, CRC(24a0a23a) SHA1(0b0330717620e3f3274a25845d9edaf8023b9db2) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("v_obj-1.322",  0x000000, 0x400000, CRC(3be5b631) SHA1(fd1064428d28ca166a9267b968c0ba846cfed656) )
	ROM_LOAD("v_obj-2.324",  0x400000, 0x400000, CRC(924153b4) SHA1(db5dadcfb4cd5e6efe9d995085936ce4f4eb4254) )
	ROM_LOAD("v_obj-3.323",  0x800000, 0x400000, CRC(e9fb9062) SHA1(18e97b4c5cced2b529e6e72d8041c6f78fcec76e) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_BYTE("v_pcm.215",  0x000000, 0x080000, CRC(e3111b60) SHA1(f7a7747f29c392876e43efcb4e6c0741454082f2) )
	ROM_CONTINUE(                 0x400000, 0x080000 )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region24.u1053", 0x000000, 0x100000, CRC(72a33dc4) SHA1(65a52f576ca4d240418fedd9a4922edcd6c0c8d1) )
ROM_END

ROM_START( viprp1u )
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("seibu1.u0211", 0x000000, 0x80000, CRC(3f412b80) SHA1(ccffce101d20971278c0f6c5f4efcf3ab687aba6) ) /* New version, "=U.S.A=" seems part of title */
	ROM_LOAD32_BYTE("seibu2.u0212", 0x000001, 0x80000, CRC(2e6c2376) SHA1(b6e660dc7c89cf565c6e055683e84ffcf8179709) )
	ROM_LOAD32_BYTE("seibu3.u0210", 0x000002, 0x80000, CRC(c38f7b4e) SHA1(d5bf2c7f2f6c812c65005facfd40ac6d3b61f29d) )
	ROM_LOAD32_BYTE("seibu4.u029",  0x000003, 0x80000, CRC(523cbef3) SHA1(5d15261b8fb108e0ba4dfd14d259984ef81ce877) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_WORD("seibu5.u0413", 0x000000, 0x20000, CRC(5ece677c) SHA1(b782cf3296f866f79fafa69ff719211c9d4026df) )
	ROM_LOAD24_BYTE("seibu6.u048",  0x000002, 0x10000, CRC(44844ef8) SHA1(bcbe24d2ffb64f9165ba4ab7de27f44b99b5ff5a) )

	ROM_REGION( 0x600000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("v_bg-11.415",  0x000000, 0x200000, CRC(6fc96736) SHA1(12df47d8af2c1febc1bce5bcf3218766447885bd) )
	ROM_LOAD24_BYTE("v_bg-12.415",  0x000002, 0x100000, CRC(d3c7281c) SHA1(340bca1f31486609b3c34dd7830362a216ff648e) )
	ROM_LOAD24_WORD("v_bg-21.410",  0x300000, 0x100000, CRC(d65b4318) SHA1(6522970d95ffa7fa2f32e0b5b4f0eb69e0286b36) )
	ROM_LOAD24_BYTE("v_bg-22.416",  0x300002, 0x080000, CRC(24a0a23a) SHA1(0b0330717620e3f3274a25845d9edaf8023b9db2) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("v_obj-1.322",  0x000000, 0x400000, CRC(3be5b631) SHA1(fd1064428d28ca166a9267b968c0ba846cfed656) )
	ROM_LOAD("v_obj-2.324",  0x400000, 0x400000, CRC(924153b4) SHA1(db5dadcfb4cd5e6efe9d995085936ce4f4eb4254) )
	ROM_LOAD("v_obj-3.323",  0x800000, 0x400000, CRC(e9fb9062) SHA1(18e97b4c5cced2b529e6e72d8041c6f78fcec76e) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_BYTE("v_pcm.215",  0x000000, 0x080000, CRC(e3111b60) SHA1(f7a7747f29c392876e43efcb4e6c0741454082f2) )
	ROM_CONTINUE(                 0x400000, 0x080000 )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region10.u1053", 0x000000, 0x100000, CRC(4319d998) SHA1(a064ce647453a9b3bccf7f1d6d0d52b5a72e09dd) )
ROM_END

ROM_START( viprp1ua )
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("seibus_1", 0x000000, 0x80000, CRC(882c299c) SHA1(36309b99764c684bd17eb512e661bafd3f3298e2) ) /* New version, "=U.S.A=" seems part of title */
	ROM_LOAD32_BYTE("seibus_2", 0x000001, 0x80000, CRC(6ce586e9) SHA1(511731996638666cbe81a1d97affce855e255bf7) )
	ROM_LOAD32_BYTE("seibus_3", 0x000002, 0x80000, CRC(f9dd9128) SHA1(ff7460699424de9e9d953343c42e0ef0fa1f0e30) )
	ROM_LOAD32_BYTE("seibus_4", 0x000003, 0x80000, CRC(cb06440c) SHA1(c73647fb72c1579f05298fd884d8aeb3765bfff4) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_WORD("seibu5.u0413", 0x000000, 0x20000, CRC(5ece677c) SHA1(b782cf3296f866f79fafa69ff719211c9d4026df) )
	ROM_LOAD24_BYTE("seibu6.u048",  0x000002, 0x10000, CRC(44844ef8) SHA1(bcbe24d2ffb64f9165ba4ab7de27f44b99b5ff5a) )

	ROM_REGION( 0x600000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("v_bg-11.415",  0x000000, 0x200000, CRC(6fc96736) SHA1(12df47d8af2c1febc1bce5bcf3218766447885bd) )
	ROM_LOAD24_BYTE("v_bg-12.415",  0x000002, 0x100000, CRC(d3c7281c) SHA1(340bca1f31486609b3c34dd7830362a216ff648e) )
	ROM_LOAD24_WORD("v_bg-21.410",  0x300000, 0x100000, CRC(d65b4318) SHA1(6522970d95ffa7fa2f32e0b5b4f0eb69e0286b36) )
	ROM_LOAD24_BYTE("v_bg-22.416",  0x300002, 0x080000, CRC(24a0a23a) SHA1(0b0330717620e3f3274a25845d9edaf8023b9db2) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("v_obj-1.322",  0x000000, 0x400000, CRC(3be5b631) SHA1(fd1064428d28ca166a9267b968c0ba846cfed656) )
	ROM_LOAD("v_obj-2.324",  0x400000, 0x400000, CRC(924153b4) SHA1(db5dadcfb4cd5e6efe9d995085936ce4f4eb4254) )
	ROM_LOAD("v_obj-3.323",  0x800000, 0x400000, CRC(e9fb9062) SHA1(18e97b4c5cced2b529e6e72d8041c6f78fcec76e) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_BYTE("v_pcm.215",  0x000000, 0x080000, CRC(e3111b60) SHA1(f7a7747f29c392876e43efcb4e6c0741454082f2) )
	ROM_CONTINUE(                 0x400000, 0x080000 )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region10.u1053", 0x000000, 0x100000, CRC(4319d998) SHA1(a064ce647453a9b3bccf7f1d6d0d52b5a72e09dd) )
ROM_END

ROM_START( viprp1j )
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("v_1-n.211", 0x000000, 0x80000, CRC(55f10b72) SHA1(2a1ebaa969f346bf3659ed8b0f469dce9eaf3b4b) )
	ROM_LOAD32_BYTE("v_2-n.212", 0x000001, 0x80000, CRC(0f888283) SHA1(7e5ac81279b9c7a06f07cb8ae76938cdd5c9beee) )
	ROM_LOAD32_BYTE("v_3-n.210", 0x000002, 0x80000, CRC(842434ac) SHA1(982d219c1d329122789c552208db2f4aaa4af7e4) )
	ROM_LOAD32_BYTE("v_4-n.29",  0x000003, 0x80000, CRC(a3948824) SHA1(fe076951427126c8b7fe81be84ecf0699597225b) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_WORD("seibu5.u0413", 0x000000, 0x20000, CRC(5ece677c) SHA1(b782cf3296f866f79fafa69ff719211c9d4026df) )
	ROM_LOAD24_BYTE("seibu6.u048",  0x000002, 0x10000, CRC(44844ef8) SHA1(bcbe24d2ffb64f9165ba4ab7de27f44b99b5ff5a) )

	ROM_REGION( 0x600000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("v_bg-11.415",  0x000000, 0x200000, CRC(6fc96736) SHA1(12df47d8af2c1febc1bce5bcf3218766447885bd) )
	ROM_LOAD24_BYTE("v_bg-12.415",  0x000002, 0x100000, CRC(d3c7281c) SHA1(340bca1f31486609b3c34dd7830362a216ff648e) )
	ROM_LOAD24_WORD("v_bg-21.410",  0x300000, 0x100000, CRC(d65b4318) SHA1(6522970d95ffa7fa2f32e0b5b4f0eb69e0286b36) )
	ROM_LOAD24_BYTE("v_bg-22.416",  0x300002, 0x080000, CRC(24a0a23a) SHA1(0b0330717620e3f3274a25845d9edaf8023b9db2) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("v_obj-1.322",  0x000000, 0x400000, CRC(3be5b631) SHA1(fd1064428d28ca166a9267b968c0ba846cfed656) )
	ROM_LOAD("v_obj-2.324",  0x400000, 0x400000, CRC(924153b4) SHA1(db5dadcfb4cd5e6efe9d995085936ce4f4eb4254) )
	ROM_LOAD("v_obj-3.323",  0x800000, 0x400000, CRC(e9fb9062) SHA1(18e97b4c5cced2b529e6e72d8041c6f78fcec76e) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_BYTE("v_pcm.215",  0x000000, 0x080000, CRC(e3111b60) SHA1(f7a7747f29c392876e43efcb4e6c0741454082f2) )
	ROM_CONTINUE(                 0x400000, 0x080000 )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region01.u1053", 0x000000, 0x100000, CRC(7ae7ab76) SHA1(a2b196f470bf64af94002fc4e2640fadad00418f) )
ROM_END

ROM_START( viprp1s )
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("viper_prg0.bin", 0x000000, 0x80000, CRC(ed9980b8) SHA1(bc324e9121ee1e55237bd91681f163ec7790de4c) )
	ROM_LOAD32_BYTE("viper_prg1.bin", 0x000001, 0x80000, CRC(9d4d3486) SHA1(ded6fa32b973046e50c40c40c446590b5f6d0b76) )
	ROM_LOAD32_BYTE("viper_prg2.bin", 0x000002, 0x80000, CRC(d7ea460b) SHA1(aed10adacd073f7d2b35f12ba4b7876e5c99d142) )
	ROM_LOAD32_BYTE("viper_prg3.bin", 0x000003, 0x80000, CRC(ca6df094) SHA1(921eec141ce2d449047172fa9cdf39d459b5cc7b) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_WORD("seibu5.u0413", 0x000000, 0x20000, CRC(5ece677c) SHA1(b782cf3296f866f79fafa69ff719211c9d4026df) )
	ROM_LOAD24_BYTE("seibu6.u048",  0x000002, 0x10000, CRC(44844ef8) SHA1(bcbe24d2ffb64f9165ba4ab7de27f44b99b5ff5a) )

	ROM_REGION( 0x600000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("v_bg-11.415",  0x000000, 0x200000, CRC(6fc96736) SHA1(12df47d8af2c1febc1bce5bcf3218766447885bd) )
	ROM_LOAD24_BYTE("v_bg-12.415",  0x000002, 0x100000, CRC(d3c7281c) SHA1(340bca1f31486609b3c34dd7830362a216ff648e) )
	ROM_LOAD24_WORD("v_bg-21.410",  0x300000, 0x100000, CRC(d65b4318) SHA1(6522970d95ffa7fa2f32e0b5b4f0eb69e0286b36) )
	ROM_LOAD24_BYTE("v_bg-22.416",  0x300002, 0x080000, CRC(24a0a23a) SHA1(0b0330717620e3f3274a25845d9edaf8023b9db2) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("v_obj-1.322",  0x000000, 0x400000, CRC(3be5b631) SHA1(fd1064428d28ca166a9267b968c0ba846cfed656) )
	ROM_LOAD("v_obj-2.324",  0x400000, 0x400000, CRC(924153b4) SHA1(db5dadcfb4cd5e6efe9d995085936ce4f4eb4254) )
	ROM_LOAD("v_obj-3.323",  0x800000, 0x400000, CRC(e9fb9062) SHA1(18e97b4c5cced2b529e6e72d8041c6f78fcec76e) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_BYTE("v_pcm.215",  0x000000, 0x080000, CRC(e3111b60) SHA1(f7a7747f29c392876e43efcb4e6c0741454082f2) )
	ROM_CONTINUE(                 0x400000, 0x080000 )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region9c.u1053", 0x000000, 0x100000, CRC(d73d640c) SHA1(61a99af2a153de9d53e28872a2493e2ba797a325) )
ROM_END

ROM_START( viprp1h )
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("viper_prg0_010995.u0211", 0x000000, 0x80000, CRC(e42fcc93) SHA1(5b2848a1da0e5d37e04ac646e67bbb84678c0292) ) /* same code as viprp1s, different region byte value */
	ROM_LOAD32_BYTE("viper_prg1_010995.u0212", 0x000001, 0x80000, CRC(9d4d3486) SHA1(ded6fa32b973046e50c40c40c446590b5f6d0b76) )
	ROM_LOAD32_BYTE("viper_prg2_010995.u0210", 0x000002, 0x80000, CRC(d7ea460b) SHA1(aed10adacd073f7d2b35f12ba4b7876e5c99d142) )
	ROM_LOAD32_BYTE("viper_prg3_010995.u029",  0x000003, 0x80000, CRC(ca6df094) SHA1(921eec141ce2d449047172fa9cdf39d459b5cc7b) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_WORD("viper_fix_010995.u0413", 0x000000, 0x20000, CRC(5ece677c) SHA1(b782cf3296f866f79fafa69ff719211c9d4026df) )
	ROM_LOAD24_BYTE("viper_fixp_010995.u048", 0x000002, 0x10000, CRC(44844ef8) SHA1(bcbe24d2ffb64f9165ba4ab7de27f44b99b5ff5a) )

	ROM_REGION( 0x600000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("v_bg-11.415",  0x000000, 0x200000, CRC(6fc96736) SHA1(12df47d8af2c1febc1bce5bcf3218766447885bd) )
	ROM_LOAD24_BYTE("v_bg-12.415",  0x000002, 0x100000, CRC(d3c7281c) SHA1(340bca1f31486609b3c34dd7830362a216ff648e) )
	ROM_LOAD24_WORD("v_bg-21.410",  0x300000, 0x100000, CRC(d65b4318) SHA1(6522970d95ffa7fa2f32e0b5b4f0eb69e0286b36) )
	ROM_LOAD24_BYTE("v_bg-22.416",  0x300002, 0x080000, CRC(24a0a23a) SHA1(0b0330717620e3f3274a25845d9edaf8023b9db2) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("v_obj-1.322",  0x000000, 0x400000, CRC(3be5b631) SHA1(fd1064428d28ca166a9267b968c0ba846cfed656) )
	ROM_LOAD("v_obj-2.324",  0x400000, 0x400000, CRC(924153b4) SHA1(db5dadcfb4cd5e6efe9d995085936ce4f4eb4254) )
	ROM_LOAD("v_obj-3.323",  0x800000, 0x400000, CRC(e9fb9062) SHA1(18e97b4c5cced2b529e6e72d8041c6f78fcec76e) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_BYTE("v_pcm.215",  0x000000, 0x080000, CRC(e3111b60) SHA1(f7a7747f29c392876e43efcb4e6c0741454082f2) )
	ROM_CONTINUE(                 0x400000, 0x080000 )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region90.u1053", 0x000000, 0x100000, CRC(8da617a2) SHA1(29c6ee05ed1c9a428a89d625b72692296c38424b) )
ROM_END

ROM_START( viprp1t )
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("viper_prg0_010995.u0211", 0x000000, 0x80000, CRC(f998dcf7) SHA1(c2dc876e4dc51062caf3d0df7c3c9cc9a5201760) ) /* same code as viprp1s and viprp1h, different region byte value */
	ROM_LOAD32_BYTE("viper_prg1_010995.u0212", 0x000001, 0x80000, CRC(9d4d3486) SHA1(ded6fa32b973046e50c40c40c446590b5f6d0b76) )
	ROM_LOAD32_BYTE("viper_prg2_010995.u0210", 0x000002, 0x80000, CRC(d7ea460b) SHA1(aed10adacd073f7d2b35f12ba4b7876e5c99d142) )
	ROM_LOAD32_BYTE("viper_prg3_010995.u029",  0x000003, 0x80000, CRC(ca6df094) SHA1(921eec141ce2d449047172fa9cdf39d459b5cc7b) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_WORD("viper_fix_010995.u0413", 0x000000, 0x20000, CRC(5ece677c) SHA1(b782cf3296f866f79fafa69ff719211c9d4026df) )
	ROM_LOAD24_BYTE("viper_fixp_010995.u048", 0x000002, 0x10000, CRC(44844ef8) SHA1(bcbe24d2ffb64f9165ba4ab7de27f44b99b5ff5a) )

	ROM_REGION( 0x600000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("v_bg-11.415",  0x000000, 0x200000, CRC(6fc96736) SHA1(12df47d8af2c1febc1bce5bcf3218766447885bd) )
	ROM_LOAD24_BYTE("v_bg-12.415",  0x000002, 0x100000, CRC(d3c7281c) SHA1(340bca1f31486609b3c34dd7830362a216ff648e) )
	ROM_LOAD24_WORD("v_bg-21.410",  0x300000, 0x100000, CRC(d65b4318) SHA1(6522970d95ffa7fa2f32e0b5b4f0eb69e0286b36) )
	ROM_LOAD24_BYTE("v_bg-22.416",  0x300002, 0x080000, CRC(24a0a23a) SHA1(0b0330717620e3f3274a25845d9edaf8023b9db2) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("v_obj-1.322",  0x000000, 0x400000, CRC(3be5b631) SHA1(fd1064428d28ca166a9267b968c0ba846cfed656) )
	ROM_LOAD("v_obj-2.324",  0x400000, 0x400000, CRC(924153b4) SHA1(db5dadcfb4cd5e6efe9d995085936ce4f4eb4254) )
	ROM_LOAD("v_obj-3.323",  0x800000, 0x400000, CRC(e9fb9062) SHA1(18e97b4c5cced2b529e6e72d8041c6f78fcec76e) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_BYTE("v_pcm.215",  0x000000, 0x080000, CRC(e3111b60) SHA1(f7a7747f29c392876e43efcb4e6c0741454082f2) )
	ROM_CONTINUE(                 0x400000, 0x080000 )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region80.u1053", 0x000000, 0x100000, CRC(e2adaff5) SHA1(9297afaf78209724515d8f78de8cee7bc7cb796b) )
ROM_END

ROM_START( viprp1pt ) /* SXX2C ROM SUB cart */
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("viper_prg0_010995.u0211", 0x000000, 0x80000, CRC(0d4c69a6) SHA1(d46706419c280838050a44cc27d6e469c021f295) ) /* same code as viprp1s, viprp1h and viprp1t , different region byte value */
	ROM_LOAD32_BYTE("viper_prg1_010995.u0212", 0x000001, 0x80000, CRC(9d4d3486) SHA1(ded6fa32b973046e50c40c40c446590b5f6d0b76) )
	ROM_LOAD32_BYTE("viper_prg2_010995.u0210", 0x000002, 0x80000, CRC(d7ea460b) SHA1(aed10adacd073f7d2b35f12ba4b7876e5c99d142) )
	ROM_LOAD32_BYTE("viper_prg3_010995.u029",  0x000003, 0x80000, CRC(ca6df094) SHA1(921eec141ce2d449047172fa9cdf39d459b5cc7b) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_WORD("viper_fix_010995.u0413", 0x000000, 0x20000, CRC(5ece677c) SHA1(b782cf3296f866f79fafa69ff719211c9d4026df) )
	ROM_LOAD24_BYTE("viper_fixp_010995.u048", 0x000002, 0x10000, CRC(44844ef8) SHA1(bcbe24d2ffb64f9165ba4ab7de27f44b99b5ff5a) )

	ROM_REGION( 0x600000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("v_bg-11.415",  0x000000, 0x200000, CRC(6fc96736) SHA1(12df47d8af2c1febc1bce5bcf3218766447885bd) )
	ROM_LOAD24_BYTE("v_bg-12.415",  0x000002, 0x100000, CRC(d3c7281c) SHA1(340bca1f31486609b3c34dd7830362a216ff648e) )
	ROM_LOAD24_WORD("v_bg-21.410",  0x300000, 0x100000, CRC(d65b4318) SHA1(6522970d95ffa7fa2f32e0b5b4f0eb69e0286b36) )
	ROM_LOAD24_BYTE("v_bg-22.416",  0x300002, 0x080000, CRC(24a0a23a) SHA1(0b0330717620e3f3274a25845d9edaf8023b9db2) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("v_obj-1.322",  0x000000, 0x400000, CRC(3be5b631) SHA1(fd1064428d28ca166a9267b968c0ba846cfed656) )
	ROM_LOAD("v_obj-2.324",  0x400000, 0x400000, CRC(924153b4) SHA1(db5dadcfb4cd5e6efe9d995085936ce4f4eb4254) )
	ROM_LOAD("v_obj-3.323",  0x800000, 0x400000, CRC(e9fb9062) SHA1(18e97b4c5cced2b529e6e72d8041c6f78fcec76e) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_BYTE("v_pcm.215",  0x000000, 0x080000, CRC(e3111b60) SHA1(f7a7747f29c392876e43efcb4e6c0741454082f2) )
	ROM_CONTINUE(                 0x400000, 0x080000 )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region96.u1053", 0x000000, 0x100000, CRC(a0ebae75) SHA1(31f7955a529a4e2492b530e54878ed7a13f49c94) )
ROM_END

ROM_START( viprp1hk )
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("seibu_1", 0x000000, 0x80000, CRC(283ba7b7) SHA1(28122e04b72f1163c69f3f845f6a493fdb6ed652) ) /* Old Version, "=HONG KONG=" seems part of title */
	ROM_LOAD32_BYTE("seibu_2", 0x000001, 0x80000, CRC(2c4db249) SHA1(a6372c9a3cde5f262ec5ef446945f6d3ad506e88) )
	ROM_LOAD32_BYTE("seibu_3", 0x000002, 0x80000, CRC(91989503) SHA1(8c215fac200cc693396dbd57e0939e7efe883342) )
	ROM_LOAD32_BYTE("seibu_4", 0x000003, 0x80000, CRC(12c9582d) SHA1(a79e26514e5ab8703a7a8c3ac39b359cfa4117c1) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_WORD("seibu_5", 0x000000, 0x20000, CRC(80920fed) SHA1(b35ed080925f6d0a0b6d2d1ab4fa919f625b1e6a) ) /* Different from both "new" & "old" versions */
	ROM_LOAD24_BYTE("seibu_6", 0x000002, 0x10000, CRC(e71a8722) SHA1(3e0133fe1f85058ca6d9ac59d731f342c6b50e92) )

	ROM_REGION( 0x600000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("v_bg-11.415",  0x000000, 0x200000, CRC(6fc96736) SHA1(12df47d8af2c1febc1bce5bcf3218766447885bd) )
	ROM_LOAD24_BYTE("v_bg-12.415",  0x000002, 0x100000, CRC(d3c7281c) SHA1(340bca1f31486609b3c34dd7830362a216ff648e) )
	ROM_LOAD24_WORD("v_bg-21.410",  0x300000, 0x100000, CRC(d65b4318) SHA1(6522970d95ffa7fa2f32e0b5b4f0eb69e0286b36) )
	ROM_LOAD24_BYTE("v_bg-22.416",  0x300002, 0x080000, CRC(24a0a23a) SHA1(0b0330717620e3f3274a25845d9edaf8023b9db2) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("v_obj-1.322",  0x000000, 0x400000, CRC(3be5b631) SHA1(fd1064428d28ca166a9267b968c0ba846cfed656) )
	ROM_LOAD("v_obj-2.324",  0x400000, 0x400000, CRC(924153b4) SHA1(db5dadcfb4cd5e6efe9d995085936ce4f4eb4254) )
	ROM_LOAD("v_obj-3.323",  0x800000, 0x400000, CRC(e9fb9062) SHA1(18e97b4c5cced2b529e6e72d8041c6f78fcec76e) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_BYTE("v_pcm.215",  0x000000, 0x080000, CRC(e3111b60) SHA1(f7a7747f29c392876e43efcb4e6c0741454082f2) )
	ROM_CONTINUE(                 0x400000, 0x080000 )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region22.u1053", 0x000000, 0x100000, CRC(5fee8413) SHA1(6d6a62fa01293b4ba4b349a39820d024add6ea22) )
ROM_END

ROM_START( viprp1oj )
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("v_1-o.211", 0x000000, 0x80000, CRC(4430be64) SHA1(96501a490042c289060d8510f6f79fbf64f79c1a) )
	ROM_LOAD32_BYTE("v_2-o.212", 0x000001, 0x80000, CRC(ffbd88f7) SHA1(cd7f291117dd18bd80fb1130eb87936ff7517ee3) )
	ROM_LOAD32_BYTE("v_3-o.210", 0x000002, 0x80000, CRC(6146db39) SHA1(04e68bfff320a3ffcb47686fa012a038538adc1a) )
	ROM_LOAD32_BYTE("v_4-o.29",  0x000003, 0x80000, CRC(dc8dd2b6) SHA1(20970706240c38c54084b4ae24b7ad23b31aa3de) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_WORD("v_5-o.413", 0x000000, 0x20000, CRC(6d863acc) SHA1(3e3e14f51b9394b24d7cbf562f1cfffc9ec2216d) )
	ROM_LOAD24_BYTE("v_6-o.48",  0x000002, 0x10000, CRC(fe7cb8f7) SHA1(55c7ab977c3666c8770deb62718d535673ffd4f8) )

	ROM_REGION( 0x600000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("v_bg-11.415",  0x000000, 0x200000, CRC(6fc96736) SHA1(12df47d8af2c1febc1bce5bcf3218766447885bd) )
	ROM_LOAD24_BYTE("v_bg-12.415",  0x000002, 0x100000, CRC(d3c7281c) SHA1(340bca1f31486609b3c34dd7830362a216ff648e) )
	ROM_LOAD24_WORD("v_bg-21.410",  0x300000, 0x100000, CRC(d65b4318) SHA1(6522970d95ffa7fa2f32e0b5b4f0eb69e0286b36) )
	ROM_LOAD24_BYTE("v_bg-22.416",  0x300002, 0x080000, CRC(24a0a23a) SHA1(0b0330717620e3f3274a25845d9edaf8023b9db2) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("v_obj-1.322",  0x000000, 0x400000, CRC(3be5b631) SHA1(fd1064428d28ca166a9267b968c0ba846cfed656) )
	ROM_LOAD("v_obj-2.324",  0x400000, 0x400000, CRC(924153b4) SHA1(db5dadcfb4cd5e6efe9d995085936ce4f4eb4254) )
	ROM_LOAD("v_obj-3.323",  0x800000, 0x400000, CRC(e9fb9062) SHA1(18e97b4c5cced2b529e6e72d8041c6f78fcec76e) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_BYTE("v_pcm.215",  0x000000, 0x080000, CRC(e3111b60) SHA1(f7a7747f29c392876e43efcb4e6c0741454082f2) )
	ROM_CONTINUE(                 0x400000, 0x080000 )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region01.u1053", 0x000000, 0x100000, CRC(7ae7ab76) SHA1(a2b196f470bf64af94002fc4e2640fadad00418f) )
ROM_END

ROM_START( viprp1ot )
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("ov1.bin", 0x000000, 0x80000, CRC(cbad0e28) SHA1(fbc9b3b243ae0d556f41e8bef5f09489bb9e302b) )
	ROM_LOAD32_BYTE("ov2.bin", 0x000001, 0x80000, CRC(0e2bbcb5) SHA1(5e53d60357fb0f9efa441261fac79e153eb35f3d) )
	ROM_LOAD32_BYTE("ov3.bin", 0x000002, 0x80000, CRC(0e86686b) SHA1(0af207ea77ef378364d80d20ecbfba2f043f2405) )
	ROM_LOAD32_BYTE("ov4.bin", 0x000003, 0x80000, CRC(9d7dd325) SHA1(550a8b5ed60e7ac50c40ec3eaa2cd6462be4a619) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_WORD("v_5-o.413", 0x000000, 0x20000, CRC(6d863acc) SHA1(3e3e14f51b9394b24d7cbf562f1cfffc9ec2216d) )
	ROM_LOAD24_BYTE("v_6-o.48",  0x000002, 0x10000, CRC(fe7cb8f7) SHA1(55c7ab977c3666c8770deb62718d535673ffd4f8) )

	ROM_REGION( 0x600000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("v_bg-11.415",  0x000000, 0x200000, CRC(6fc96736) SHA1(12df47d8af2c1febc1bce5bcf3218766447885bd) )
	ROM_LOAD24_BYTE("v_bg-12.415",  0x000002, 0x100000, CRC(d3c7281c) SHA1(340bca1f31486609b3c34dd7830362a216ff648e) )
	ROM_LOAD24_WORD("v_bg-21.410",  0x300000, 0x100000, CRC(d65b4318) SHA1(6522970d95ffa7fa2f32e0b5b4f0eb69e0286b36) )
	ROM_LOAD24_BYTE("v_bg-22.416",  0x300002, 0x080000, CRC(24a0a23a) SHA1(0b0330717620e3f3274a25845d9edaf8023b9db2) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("v_obj-1.322",  0x000000, 0x400000, CRC(3be5b631) SHA1(fd1064428d28ca166a9267b968c0ba846cfed656) )
	ROM_LOAD("v_obj-2.324",  0x400000, 0x400000, CRC(924153b4) SHA1(db5dadcfb4cd5e6efe9d995085936ce4f4eb4254) )
	ROM_LOAD("v_obj-3.323",  0x800000, 0x400000, CRC(e9fb9062) SHA1(18e97b4c5cced2b529e6e72d8041c6f78fcec76e) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_BYTE("v_pcm.215",  0x000000, 0x080000, CRC(e3111b60) SHA1(f7a7747f29c392876e43efcb4e6c0741454082f2) )
	ROM_CONTINUE(                 0x400000, 0x080000 )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region80.u1053", 0x000000, 0x100000, CRC(e2adaff5) SHA1(9297afaf78209724515d8f78de8cee7bc7cb796b) )
ROM_END


ROM_START( rdft ) /* SXX2C ROM SUB2 cart */
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("raiden-fi_prg0_121196.u0211", 0x000000, 0x80000, CRC(adcb5dbc) SHA1(3831becd1e052d81dd00ee098ee630fe35164df8) )
	ROM_LOAD32_BYTE("raiden-fi_prg1_121196.u0212", 0x000001, 0x80000, CRC(60c5b92e) SHA1(ca67f97f9e7d8a21667dc59e7d390dff91179b08) )
	ROM_LOAD32_BYTE("raiden-fi_prg2_121196.u0210", 0x000002, 0x80000, CRC(44b86db5) SHA1(bb05c6d27af86084cd3e17a189826c836f229c8b) )
	ROM_LOAD32_BYTE("raiden-fi_prg3_121196.u029",  0x000003, 0x80000, CRC(e70727ce) SHA1(2e4e896d50cf086e682054cb2e9c223af04cd0cb) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_BYTE("seibu_5.u0423", 0x000000, 0x10000, CRC(8f8d4e14) SHA1(06c803975767ae98f40ba7ac5764a5bc8baa3a30) )
	ROM_LOAD24_BYTE("seibu_6.u0424", 0x000001, 0x10000, CRC(6ac64968) SHA1(ec395205c24c4f864a1f805bb0d4641562d4faa9) )
	ROM_LOAD24_BYTE("seibu_7.u048",  0x000002, 0x10000, CRC(4d87e1ea) SHA1(3230e9b643fad773e61ab8ce09c0cd7d4d0558e3) )

	ROM_REGION( 0x600000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("gun_dogs_bg1-d.u0415", 0x000000, 0x200000, CRC(6a68054c) SHA1(5cbfc4ac90045f1401c2dda7a51936558c9de07e) ) // some mask ROMs might be labeled GD BG1-D ect
	ROM_LOAD24_BYTE("gun_dogs_bg1-p.u0410", 0x000002, 0x100000, CRC(3400794a) SHA1(719808f7442bac612cefd7b7fffcd665e6337ad0) )
	ROM_LOAD24_WORD("gun_dogs_bg2-d.u0424", 0x300000, 0x200000, CRC(61cd2991) SHA1(bb608e3948bf9ea35b5e1615d2ba6858d029dcbe) )
	ROM_LOAD24_BYTE("gun_dogs_bg2-p.u049",  0x300002, 0x100000, CRC(502d5799) SHA1(c3a0e1a4f5a7b35572ae1ff31315da4ed08aa2fe) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("gun_dogs_obj-1.u0322", 0x000000, 0x400000, CRC(59d86c99) SHA1(d3c9241e7b51fe21f8351051b063f91dc69bf905) )
	ROM_LOAD("gun_dogs_obj-2.u0324", 0x400000, 0x400000, CRC(1ceb0b6f) SHA1(97225a9b3e7be18080aa52f6570af2cce8f25c06) )
	ROM_LOAD("gun_dogs_obj-3.u0323", 0x800000, 0x400000, CRC(36e93234) SHA1(51917a80b7da5c32a9434a1076fc2916d62e6a3e) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_WORD("gun_dogs_pcm.u0217",  0x000000, 0x100000, CRC(31253ad7) SHA1(c81c8d50f8f287f5cbfaec77b30d969b01ce11a9) )
	ROM_CONTINUE(                          0x400000, 0x100000 )
	ROM_LOAD32_BYTE("seibu_8.u0216",       0x800000, 0x080000, CRC(f88cb6e4) SHA1(fb35b41307b490d5d08e4b8a70f8ff4ce2ca8105) )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region80.u1053", 0x000000, 0x100000, CRC(e2adaff5) SHA1(9297afaf78209724515d8f78de8cee7bc7cb796b) )
ROM_END

ROM_START( rdftu )
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("rdftu_gd_1.211", 0x000000, 0x80000, CRC(47810c48) SHA1(8dc8848d3e7467ea887c50fd5675fba2cc741121) )
	ROM_LOAD32_BYTE("rdftu_gd_2.212", 0x000001, 0x80000, CRC(13911750) SHA1(8899accb059ed84170924750bb39ae7383ebd959) )
	ROM_LOAD32_BYTE("rdftu_gd_3.210", 0x000002, 0x80000, CRC(10761b03) SHA1(e67db2e7c2176987419158fc4cee00fd9b99d03f) )
	ROM_LOAD32_BYTE("rdftu_gd_4.29",  0x000003, 0x80000, CRC(e5a3f01d) SHA1(5ca338f85a020d43d2618f88e798a076d13a5c7f) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_BYTE("seibu_5.u0423", 0x000000, 0x10000, CRC(8f8d4e14) SHA1(06c803975767ae98f40ba7ac5764a5bc8baa3a30) )
	ROM_LOAD24_BYTE("seibu_6.u0424", 0x000001, 0x10000, CRC(6ac64968) SHA1(ec395205c24c4f864a1f805bb0d4641562d4faa9) )
	ROM_LOAD24_BYTE("seibu_7.u048",  0x000002, 0x10000, CRC(4d87e1ea) SHA1(3230e9b643fad773e61ab8ce09c0cd7d4d0558e3) )

	ROM_REGION( 0x600000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("gun_dogs_bg1-d.u0415", 0x000000, 0x200000, CRC(6a68054c) SHA1(5cbfc4ac90045f1401c2dda7a51936558c9de07e) ) // some mask ROMs might be labeled GD BG1-D ect
	ROM_LOAD24_BYTE("gun_dogs_bg1-p.u0410", 0x000002, 0x100000, CRC(3400794a) SHA1(719808f7442bac612cefd7b7fffcd665e6337ad0) )
	ROM_LOAD24_WORD("gun_dogs_bg2-d.u0424", 0x300000, 0x200000, CRC(61cd2991) SHA1(bb608e3948bf9ea35b5e1615d2ba6858d029dcbe) )
	ROM_LOAD24_BYTE("gun_dogs_bg2-p.u049",  0x300002, 0x100000, CRC(502d5799) SHA1(c3a0e1a4f5a7b35572ae1ff31315da4ed08aa2fe) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("gun_dogs_obj-1.u0322", 0x000000, 0x400000, CRC(59d86c99) SHA1(d3c9241e7b51fe21f8351051b063f91dc69bf905) )
	ROM_LOAD("gun_dogs_obj-2.u0324", 0x400000, 0x400000, CRC(1ceb0b6f) SHA1(97225a9b3e7be18080aa52f6570af2cce8f25c06) )
	ROM_LOAD("gun_dogs_obj-3.u0323", 0x800000, 0x400000, CRC(36e93234) SHA1(51917a80b7da5c32a9434a1076fc2916d62e6a3e) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_WORD("gun_dogs_pcm.u0217",  0x000000, 0x100000, CRC(31253ad7) SHA1(c81c8d50f8f287f5cbfaec77b30d969b01ce11a9) )
	ROM_CONTINUE(                          0x400000, 0x100000 )
	ROM_LOAD32_BYTE("seibu_8.u0216",       0x800000, 0x080000, CRC(f88cb6e4) SHA1(fb35b41307b490d5d08e4b8a70f8ff4ce2ca8105) )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region10.u1053", 0x000000, 0x100000, CRC(4319d998) SHA1(a064ce647453a9b3bccf7f1d6d0d52b5a72e09dd) )
ROM_END

ROM_START( rdftj )
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("gd_1.211", 0x000000, 0x80000, CRC(f6b2cbdc) SHA1(040c4ff961c8be388c8279b06b777d528c2acc1b) )
	ROM_LOAD32_BYTE("gd_2.212", 0x000001, 0x80000, CRC(1982f812) SHA1(4f12fc3fd7f7a4beda4d29cc81e3a58d255e441f) )
	ROM_LOAD32_BYTE("gd_3.210", 0x000002, 0x80000, CRC(b0f59f44) SHA1(d44fe074ddab35cd0190535cd9fbd7f9e49312a4) )
	ROM_LOAD32_BYTE("gd_4.29",  0x000003, 0x80000, CRC(cd8705bd) SHA1(b19a1486d6b899a134d7b518863ddc8f07967e8b) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_BYTE("seibu_5.u0423", 0x000000, 0x10000, CRC(8f8d4e14) SHA1(06c803975767ae98f40ba7ac5764a5bc8baa3a30) )
	ROM_LOAD24_BYTE("seibu_6.u0424", 0x000001, 0x10000, CRC(6ac64968) SHA1(ec395205c24c4f864a1f805bb0d4641562d4faa9) )
	ROM_LOAD24_BYTE("seibu_7.u048",  0x000002, 0x10000, CRC(4d87e1ea) SHA1(3230e9b643fad773e61ab8ce09c0cd7d4d0558e3) )

	ROM_REGION( 0x600000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("gun_dogs_bg1-d.u0415", 0x000000, 0x200000, CRC(6a68054c) SHA1(5cbfc4ac90045f1401c2dda7a51936558c9de07e) ) // some mask ROMs might be labeled GD BG1-D ect
	ROM_LOAD24_BYTE("gun_dogs_bg1-p.u0410", 0x000002, 0x100000, CRC(3400794a) SHA1(719808f7442bac612cefd7b7fffcd665e6337ad0) )
	ROM_LOAD24_WORD("gun_dogs_bg2-d.u0424", 0x300000, 0x200000, CRC(61cd2991) SHA1(bb608e3948bf9ea35b5e1615d2ba6858d029dcbe) )
	ROM_LOAD24_BYTE("gun_dogs_bg2-p.u049",  0x300002, 0x100000, CRC(502d5799) SHA1(c3a0e1a4f5a7b35572ae1ff31315da4ed08aa2fe) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("gun_dogs_obj-1.u0322", 0x000000, 0x400000, CRC(59d86c99) SHA1(d3c9241e7b51fe21f8351051b063f91dc69bf905) )
	ROM_LOAD("gun_dogs_obj-2.u0324", 0x400000, 0x400000, CRC(1ceb0b6f) SHA1(97225a9b3e7be18080aa52f6570af2cce8f25c06) )
	ROM_LOAD("gun_dogs_obj-3.u0323", 0x800000, 0x400000, CRC(36e93234) SHA1(51917a80b7da5c32a9434a1076fc2916d62e6a3e) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_WORD("gun_dogs_pcm.u0217",  0x000000, 0x100000, CRC(31253ad7) SHA1(c81c8d50f8f287f5cbfaec77b30d969b01ce11a9) )
	ROM_CONTINUE(                          0x400000, 0x100000 )
	ROM_LOAD32_BYTE("seibu_8.u0216",       0x800000, 0x080000, CRC(f88cb6e4) SHA1(fb35b41307b490d5d08e4b8a70f8ff4ce2ca8105) )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region01.u1053", 0x000000, 0x100000, CRC(7ae7ab76) SHA1(a2b196f470bf64af94002fc4e2640fadad00418f) )
ROM_END

ROM_START( rdftja )
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("rf1.bin", 0x000000, 0x80000, CRC(46861b75) SHA1(079c589c490d49f7ec97a7e68c5b6e7e37872827) )
	ROM_LOAD32_BYTE("rf2.bin", 0x000001, 0x80000, CRC(6388ed11) SHA1(aebbccfb0f704cdceb45ea71216275dd83880e15) )
	ROM_LOAD32_BYTE("rf3.bin", 0x000002, 0x80000, CRC(beafcd24) SHA1(2dbc47ecef6f898a371a841df2c72151da9c5a8d) )
	ROM_LOAD32_BYTE("rf4.bin", 0x000003, 0x80000, CRC(5236f45f) SHA1(8b05d977d3d07796007a00a52d2396475dc2f7dc) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_BYTE("seibu_5.u0423", 0x000000, 0x10000, CRC(8f8d4e14) SHA1(06c803975767ae98f40ba7ac5764a5bc8baa3a30) )
	ROM_LOAD24_BYTE("seibu_6.u0424", 0x000001, 0x10000, CRC(6ac64968) SHA1(ec395205c24c4f864a1f805bb0d4641562d4faa9) )
	ROM_LOAD24_BYTE("seibu_7.u048",  0x000002, 0x10000, CRC(4d87e1ea) SHA1(3230e9b643fad773e61ab8ce09c0cd7d4d0558e3) )

	ROM_REGION( 0x600000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("gun_dogs_bg1-d.u0415", 0x000000, 0x200000, CRC(6a68054c) SHA1(5cbfc4ac90045f1401c2dda7a51936558c9de07e) ) // some mask ROMs might be labeled GD BG1-D ect
	ROM_LOAD24_BYTE("gun_dogs_bg1-p.u0410", 0x000002, 0x100000, CRC(3400794a) SHA1(719808f7442bac612cefd7b7fffcd665e6337ad0) )
	ROM_LOAD24_WORD("gun_dogs_bg2-d.u0424", 0x300000, 0x200000, CRC(61cd2991) SHA1(bb608e3948bf9ea35b5e1615d2ba6858d029dcbe) )
	ROM_LOAD24_BYTE("gun_dogs_bg2-p.u049",  0x300002, 0x100000, CRC(502d5799) SHA1(c3a0e1a4f5a7b35572ae1ff31315da4ed08aa2fe) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("gun_dogs_obj-1.u0322", 0x000000, 0x400000, CRC(59d86c99) SHA1(d3c9241e7b51fe21f8351051b063f91dc69bf905) )
	ROM_LOAD("gun_dogs_obj-2.u0324", 0x400000, 0x400000, CRC(1ceb0b6f) SHA1(97225a9b3e7be18080aa52f6570af2cce8f25c06) )
	ROM_LOAD("gun_dogs_obj-3.u0323", 0x800000, 0x400000, CRC(36e93234) SHA1(51917a80b7da5c32a9434a1076fc2916d62e6a3e) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_WORD("gun_dogs_pcm.u0217",  0x000000, 0x100000, CRC(31253ad7) SHA1(c81c8d50f8f287f5cbfaec77b30d969b01ce11a9) )
	ROM_CONTINUE(                          0x400000, 0x100000 )
	ROM_LOAD32_BYTE("seibu_8.u0216",       0x800000, 0x080000, CRC(f88cb6e4) SHA1(fb35b41307b490d5d08e4b8a70f8ff4ce2ca8105) )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region01.u1053", 0x000000, 0x100000, CRC(7ae7ab76) SHA1(a2b196f470bf64af94002fc4e2640fadad00418f) )
ROM_END

ROM_START( rdftau )
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("1.u0211", 0x000000, 0x80000, CRC(6339c60d) SHA1(871d5bc9fc695651ceb6fcfdab32084320fe239d) )
	ROM_LOAD32_BYTE("2.u0212", 0x000001, 0x80000, CRC(a88bda02) SHA1(27dc720d28f56cf443a4eb0bbaaf4bf3b194056d) )
	ROM_LOAD32_BYTE("3.u0210", 0x000002, 0x80000, CRC(a73e337e) SHA1(93323875c676f38eca3298fcf4a34911db2d78a8) )
	ROM_LOAD32_BYTE("4.u029",  0x000003, 0x80000, CRC(8cc628f0) SHA1(7534eae8a1ea461adad483002b3cecf132e0e325) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_BYTE("seibu_5.u0423", 0x000000, 0x10000, CRC(8f8d4e14) SHA1(06c803975767ae98f40ba7ac5764a5bc8baa3a30) )
	ROM_LOAD24_BYTE("seibu_6.u0424", 0x000001, 0x10000, CRC(6ac64968) SHA1(ec395205c24c4f864a1f805bb0d4641562d4faa9) )
	ROM_LOAD24_BYTE("seibu_7.u048",  0x000002, 0x10000, CRC(4d87e1ea) SHA1(3230e9b643fad773e61ab8ce09c0cd7d4d0558e3) )

	ROM_REGION( 0x600000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("gun_dogs_bg1-d.u0415", 0x000000, 0x200000, CRC(6a68054c) SHA1(5cbfc4ac90045f1401c2dda7a51936558c9de07e) ) // some mask ROMs might be labeled GD BG1-D ect
	ROM_LOAD24_BYTE("gun_dogs_bg1-p.u0410", 0x000002, 0x100000, CRC(3400794a) SHA1(719808f7442bac612cefd7b7fffcd665e6337ad0) )
	ROM_LOAD24_WORD("gun_dogs_bg2-d.u0424", 0x300000, 0x200000, CRC(61cd2991) SHA1(bb608e3948bf9ea35b5e1615d2ba6858d029dcbe) )
	ROM_LOAD24_BYTE("gun_dogs_bg2-p.u049",  0x300002, 0x100000, CRC(502d5799) SHA1(c3a0e1a4f5a7b35572ae1ff31315da4ed08aa2fe) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("gun_dogs_obj-1.u0322", 0x000000, 0x400000, CRC(59d86c99) SHA1(d3c9241e7b51fe21f8351051b063f91dc69bf905) )
	ROM_LOAD("gun_dogs_obj-2.u0324", 0x400000, 0x400000, CRC(1ceb0b6f) SHA1(97225a9b3e7be18080aa52f6570af2cce8f25c06) )
	ROM_LOAD("gun_dogs_obj-3.u0323", 0x800000, 0x400000, CRC(36e93234) SHA1(51917a80b7da5c32a9434a1076fc2916d62e6a3e) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_WORD("gun_dogs_pcm.u0217",  0x000000, 0x100000, CRC(31253ad7) SHA1(c81c8d50f8f287f5cbfaec77b30d969b01ce11a9) )
	ROM_CONTINUE(                          0x400000, 0x100000 )
	ROM_LOAD32_BYTE("seibu_8.u0216",       0x800000, 0x080000, CRC(f88cb6e4) SHA1(fb35b41307b490d5d08e4b8a70f8ff4ce2ca8105) )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region9e.u1053", 0x000000, 0x100000, CRC(7ad6f17e) SHA1(9a2cc77a4f86c00208f739bd53aca4f55adf7ea7) )
ROM_END

ROM_START( rdftauge )  /* SPI Cart "SXX2C ROM SUB2", Australia region (Tuning license - Evaluation Software For Show, Germany) ~~ SPI PCB "(C)1995 SXX2C-MAIN V2.0" */
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("seibu.1.u0211", 0x000000, 0x80000, CRC(3e79da3c) SHA1(8d33da1dadb791ff97b353532ca647eb462c2ae4) )
	ROM_LOAD32_BYTE("seibu.2.u0212", 0x000001, 0x80000, CRC(a6fbf98c) SHA1(ce0f5f6f1f5656dbac0a6b977795026276c8fa86) )
	ROM_LOAD32_BYTE("seibu.3.u0210", 0x000002, 0x80000, CRC(ad31cc17) SHA1(12b735519cad190887cdbc6680d879f791ab3726) )
	ROM_LOAD32_BYTE("seibu.4.u029",  0x000003, 0x80000, CRC(756d99ae) SHA1(38c234acf3e8204a29a9adb077328d37d97cfd6d) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_BYTE("seibu_5.u0423", 0x000000, 0x10000, CRC(8f8d4e14) SHA1(06c803975767ae98f40ba7ac5764a5bc8baa3a30) )
	ROM_LOAD24_BYTE("seibu_6.u0424", 0x000001, 0x10000, CRC(6ac64968) SHA1(ec395205c24c4f864a1f805bb0d4641562d4faa9) )
	ROM_LOAD24_BYTE("seibu_7.u048",  0x000002, 0x10000, CRC(4d87e1ea) SHA1(3230e9b643fad773e61ab8ce09c0cd7d4d0558e3) )

	ROM_REGION( 0x600000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("gun_dogs_bg1-d.u0415", 0x000000, 0x200000, CRC(6a68054c) SHA1(5cbfc4ac90045f1401c2dda7a51936558c9de07e) ) // some mask ROMs might be labeled GD BG1-D ect
	ROM_LOAD24_BYTE("gun_dogs_bg1-p.u0410", 0x000002, 0x100000, CRC(3400794a) SHA1(719808f7442bac612cefd7b7fffcd665e6337ad0) )
	ROM_LOAD24_WORD("gun_dogs_bg2-d.u0416", 0x300000, 0x200000, CRC(61cd2991) SHA1(bb608e3948bf9ea35b5e1615d2ba6858d029dcbe) )
	ROM_LOAD24_BYTE("gun_dogs_bg2-p.u049",  0x300002, 0x100000, CRC(502d5799) SHA1(c3a0e1a4f5a7b35572ae1ff31315da4ed08aa2fe) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("gun_dogs_obj-1.u0322", 0x000000, 0x400000, CRC(59d86c99) SHA1(d3c9241e7b51fe21f8351051b063f91dc69bf905) )
	ROM_LOAD("gun_dogs_obj-2.u0324", 0x400000, 0x400000, CRC(1ceb0b6f) SHA1(97225a9b3e7be18080aa52f6570af2cce8f25c06) )
	ROM_LOAD("gun_dogs_obj-3.u0323", 0x800000, 0x400000, CRC(36e93234) SHA1(51917a80b7da5c32a9434a1076fc2916d62e6a3e) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_WORD("gun_dogs_pcm.u0217", 0x000000, 0x100000, CRC(31253ad7) SHA1(c81c8d50f8f287f5cbfaec77b30d969b01ce11a9) )
	ROM_CONTINUE(                         0x400000, 0x100000 )
	ROM_LOAD32_BYTE("seibu_8.u0216",      0x800000, 0x080000, CRC(f88cb6e4) SHA1(fb35b41307b490d5d08e4b8a70f8ff4ce2ca8105) )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region9e.u1053", 0x000000, 0x100000, CRC(7ad6f17e) SHA1(9a2cc77a4f86c00208f739bd53aca4f55adf7ea7) )
ROM_END

// The rest of the following Raiden Fighters sets are based on the same code revision
ROM_START( rdfta ) // SXX2C ROM SUB2 cart
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("seibu_1.u0211", 0x000000, 0x80000, CRC(c3bb2e58) SHA1(399ac4b387ba38f5fdad5c4172b2d3baeafd8773) ) // sldh
	ROM_LOAD32_BYTE("seibu_2.u0212", 0x000001, 0x80000, CRC(58ccb10c) SHA1(0cce4057bfada78121d9586574b98d46cdd7dd46) )
	ROM_LOAD32_BYTE("seibu_3.u0210", 0x000002, 0x80000, CRC(47fc3c96) SHA1(7378f8caa847f89f235b5be6779118721076873b) )
	ROM_LOAD32_BYTE("seibu_4.u029",  0x000003, 0x80000, CRC(271bdd4b) SHA1(0a805568cbd6a9c18bdb755a41972ff6bba9e6eb) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_BYTE("seibu_5.u0423", 0x000000, 0x10000, CRC(8f8d4e14) SHA1(06c803975767ae98f40ba7ac5764a5bc8baa3a30) )
	ROM_LOAD24_BYTE("seibu_6.u0424", 0x000001, 0x10000, CRC(6ac64968) SHA1(ec395205c24c4f864a1f805bb0d4641562d4faa9) )
	ROM_LOAD24_BYTE("seibu_7.u048",  0x000002, 0x10000, CRC(4d87e1ea) SHA1(3230e9b643fad773e61ab8ce09c0cd7d4d0558e3) )

	ROM_REGION( 0x600000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("gun_dogs_bg1-d.u0415", 0x000000, 0x200000, CRC(6a68054c) SHA1(5cbfc4ac90045f1401c2dda7a51936558c9de07e) ) // some mask ROMs might be labeled GD BG1-D ect
	ROM_LOAD24_BYTE("gun_dogs_bg1-p.u0410", 0x000002, 0x100000, CRC(3400794a) SHA1(719808f7442bac612cefd7b7fffcd665e6337ad0) )
	ROM_LOAD24_WORD("gun_dogs_bg2-d.u0424", 0x300000, 0x200000, CRC(61cd2991) SHA1(bb608e3948bf9ea35b5e1615d2ba6858d029dcbe) )
	ROM_LOAD24_BYTE("gun_dogs_bg2-p.u049",  0x300002, 0x100000, CRC(502d5799) SHA1(c3a0e1a4f5a7b35572ae1ff31315da4ed08aa2fe) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("gun_dogs_obj-1.u0322", 0x000000, 0x400000, CRC(59d86c99) SHA1(d3c9241e7b51fe21f8351051b063f91dc69bf905) )
	ROM_LOAD("gun_dogs_obj-2.u0324", 0x400000, 0x400000, CRC(1ceb0b6f) SHA1(97225a9b3e7be18080aa52f6570af2cce8f25c06) )
	ROM_LOAD("gun_dogs_obj-3.u0323", 0x800000, 0x400000, CRC(36e93234) SHA1(51917a80b7da5c32a9434a1076fc2916d62e6a3e) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_WORD("gun_dogs_pcm.u0217",  0x000000, 0x100000, CRC(31253ad7) SHA1(c81c8d50f8f287f5cbfaec77b30d969b01ce11a9) )
	ROM_CONTINUE(                          0x400000, 0x100000 )
	ROM_LOAD32_BYTE("seibu_8.u0216",       0x800000, 0x080000, CRC(f88cb6e4) SHA1(fb35b41307b490d5d08e4b8a70f8ff4ce2ca8105) )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region82.u1053", 0x000000, 0x100000, CRC(4f463a87) SHA1(0e27904745da61a3ba7c48c5b4c7d45989bbd05b) )
ROM_END

ROM_START( rdftit ) // SXX2C ROM SUB2 cart
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("seibu_1.u0211", 0x000000, 0x80000, CRC(de0c3e3c) SHA1(b00225bad282e46b5825608f76eea6670bfe5527) ) // sldh
	ROM_LOAD32_BYTE("seibu_2.u0212", 0x000001, 0x80000, CRC(58ccb10c) SHA1(0cce4057bfada78121d9586574b98d46cdd7dd46) )
	ROM_LOAD32_BYTE("seibu_3.u0210", 0x000002, 0x80000, CRC(47fc3c96) SHA1(7378f8caa847f89f235b5be6779118721076873b) )
	ROM_LOAD32_BYTE("seibu_4.u029",  0x000003, 0x80000, CRC(271bdd4b) SHA1(0a805568cbd6a9c18bdb755a41972ff6bba9e6eb) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_BYTE("seibu_5.u0423", 0x000000, 0x10000, CRC(8f8d4e14) SHA1(06c803975767ae98f40ba7ac5764a5bc8baa3a30) )
	ROM_LOAD24_BYTE("seibu_6.u0424", 0x000001, 0x10000, CRC(6ac64968) SHA1(ec395205c24c4f864a1f805bb0d4641562d4faa9) )
	ROM_LOAD24_BYTE("seibu_7.u048",  0x000002, 0x10000, CRC(4d87e1ea) SHA1(3230e9b643fad773e61ab8ce09c0cd7d4d0558e3) )

	ROM_REGION( 0x600000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("gun_dogs_bg1-d.u0415", 0x000000, 0x200000, CRC(6a68054c) SHA1(5cbfc4ac90045f1401c2dda7a51936558c9de07e) ) // some mask ROMs might be labeled GD BG1-D ect
	ROM_LOAD24_BYTE("gun_dogs_bg1-p.u0410", 0x000002, 0x100000, CRC(3400794a) SHA1(719808f7442bac612cefd7b7fffcd665e6337ad0) )
	ROM_LOAD24_WORD("gun_dogs_bg2-d.u0424", 0x300000, 0x200000, CRC(61cd2991) SHA1(bb608e3948bf9ea35b5e1615d2ba6858d029dcbe) )
	ROM_LOAD24_BYTE("gun_dogs_bg2-p.u049",  0x300002, 0x100000, CRC(502d5799) SHA1(c3a0e1a4f5a7b35572ae1ff31315da4ed08aa2fe) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("gun_dogs_obj-1.u0322", 0x000000, 0x400000, CRC(59d86c99) SHA1(d3c9241e7b51fe21f8351051b063f91dc69bf905) )
	ROM_LOAD("gun_dogs_obj-2.u0324", 0x400000, 0x400000, CRC(1ceb0b6f) SHA1(97225a9b3e7be18080aa52f6570af2cce8f25c06) )
	ROM_LOAD("gun_dogs_obj-3.u0323", 0x800000, 0x400000, CRC(36e93234) SHA1(51917a80b7da5c32a9434a1076fc2916d62e6a3e) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_WORD("gun_dogs_pcm.u0217",  0x000000, 0x100000, CRC(31253ad7) SHA1(c81c8d50f8f287f5cbfaec77b30d969b01ce11a9) )
	ROM_CONTINUE(                          0x400000, 0x100000 )
	ROM_LOAD32_BYTE("seibu_8.u0216",       0x800000, 0x080000, CRC(f88cb6e4) SHA1(fb35b41307b490d5d08e4b8a70f8ff4ce2ca8105) )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region92.u1053", 0x000000, 0x100000, CRC(204d82d0) SHA1(444f4aefa27d8f5d1a2f7f08f826ea84b0ccbd02) )
ROM_END

ROM_START( rdftgb ) // SXX2C ROM SUB2 cart
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("seibu_1.u0211", 0x000000, 0x80000, CRC(2403035f) SHA1(9763cca3864d6127050e1507b572efa68f664b3c) )
	ROM_LOAD32_BYTE("seibu_2.u0212", 0x000001, 0x80000, CRC(58ccb10c) SHA1(0cce4057bfada78121d9586574b98d46cdd7dd46) )
	ROM_LOAD32_BYTE("seibu_3.u0210", 0x000002, 0x80000, CRC(47fc3c96) SHA1(7378f8caa847f89f235b5be6779118721076873b) )
	ROM_LOAD32_BYTE("seibu_4.u029",  0x000003, 0x80000, CRC(271bdd4b) SHA1(0a805568cbd6a9c18bdb755a41972ff6bba9e6eb) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_BYTE("seibu_5.u0423", 0x000000, 0x10000, CRC(8f8d4e14) SHA1(06c803975767ae98f40ba7ac5764a5bc8baa3a30) )
	ROM_LOAD24_BYTE("seibu_6.u0424", 0x000001, 0x10000, CRC(6ac64968) SHA1(ec395205c24c4f864a1f805bb0d4641562d4faa9) )
	ROM_LOAD24_BYTE("seibu_7.u048",  0x000002, 0x10000, CRC(4d87e1ea) SHA1(3230e9b643fad773e61ab8ce09c0cd7d4d0558e3) )

	ROM_REGION( 0x600000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("gun_dogs_bg1-d.u0415", 0x000000, 0x200000, CRC(6a68054c) SHA1(5cbfc4ac90045f1401c2dda7a51936558c9de07e) ) // some mask ROMs might be labeled GD BG1-D ect
	ROM_LOAD24_BYTE("gun_dogs_bg1-p.u0410", 0x000002, 0x100000, CRC(3400794a) SHA1(719808f7442bac612cefd7b7fffcd665e6337ad0) )
	ROM_LOAD24_WORD("gun_dogs_bg2-d.u0424", 0x300000, 0x200000, CRC(61cd2991) SHA1(bb608e3948bf9ea35b5e1615d2ba6858d029dcbe) )
	ROM_LOAD24_BYTE("gun_dogs_bg2-p.u049",  0x300002, 0x100000, CRC(502d5799) SHA1(c3a0e1a4f5a7b35572ae1ff31315da4ed08aa2fe) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("gun_dogs_obj-1.u0322", 0x000000, 0x400000, CRC(59d86c99) SHA1(d3c9241e7b51fe21f8351051b063f91dc69bf905) )
	ROM_LOAD("gun_dogs_obj-2.u0324", 0x400000, 0x400000, CRC(1ceb0b6f) SHA1(97225a9b3e7be18080aa52f6570af2cce8f25c06) )
	ROM_LOAD("gun_dogs_obj-3.u0323", 0x800000, 0x400000, CRC(36e93234) SHA1(51917a80b7da5c32a9434a1076fc2916d62e6a3e) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_WORD("gun_dogs_pcm.u0217",  0x000000, 0x100000, CRC(31253ad7) SHA1(c81c8d50f8f287f5cbfaec77b30d969b01ce11a9) )
	ROM_CONTINUE(                          0x400000, 0x100000 )
	ROM_LOAD32_BYTE("seibu_8.u0216",       0x800000, 0x080000, CRC(f88cb6e4) SHA1(fb35b41307b490d5d08e4b8a70f8ff4ce2ca8105) )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region8c.u1053", 0x000000, 0x100000, CRC(b836dc5b) SHA1(80400429970f8997978ee723d3067a39ebc0e126) )
ROM_END

ROM_START( rdftgr ) // SXX2C ROM SUB2 cart
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("seibu_1.u0211", 0x000000, 0x80000, CRC(ca0d6273) SHA1(f761d68ec9a49d66d9f3adc663663f716cdd3735) ) // sldh
	ROM_LOAD32_BYTE("seibu_2.u0212", 0x000001, 0x80000, CRC(58ccb10c) SHA1(0cce4057bfada78121d9586574b98d46cdd7dd46) )
	ROM_LOAD32_BYTE("seibu_3.u0210", 0x000002, 0x80000, CRC(47fc3c96) SHA1(7378f8caa847f89f235b5be6779118721076873b) )
	ROM_LOAD32_BYTE("seibu_4.u029",  0x000003, 0x80000, CRC(271bdd4b) SHA1(0a805568cbd6a9c18bdb755a41972ff6bba9e6eb) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_BYTE("seibu_5.u0423", 0x000000, 0x10000, CRC(8f8d4e14) SHA1(06c803975767ae98f40ba7ac5764a5bc8baa3a30) )
	ROM_LOAD24_BYTE("seibu_6.u0424", 0x000001, 0x10000, CRC(6ac64968) SHA1(ec395205c24c4f864a1f805bb0d4641562d4faa9) )
	ROM_LOAD24_BYTE("seibu_7.u048",  0x000002, 0x10000, CRC(4d87e1ea) SHA1(3230e9b643fad773e61ab8ce09c0cd7d4d0558e3) )

	ROM_REGION( 0x600000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("gun_dogs_bg1-d.u0415", 0x000000, 0x200000, CRC(6a68054c) SHA1(5cbfc4ac90045f1401c2dda7a51936558c9de07e) ) // some mask ROMs might be labeled GD BG1-D ect
	ROM_LOAD24_BYTE("gun_dogs_bg1-p.u0410", 0x000002, 0x100000, CRC(3400794a) SHA1(719808f7442bac612cefd7b7fffcd665e6337ad0) )
	ROM_LOAD24_WORD("gun_dogs_bg2-d.u0424", 0x300000, 0x200000, CRC(61cd2991) SHA1(bb608e3948bf9ea35b5e1615d2ba6858d029dcbe) )
	ROM_LOAD24_BYTE("gun_dogs_bg2-p.u049",  0x300002, 0x100000, CRC(502d5799) SHA1(c3a0e1a4f5a7b35572ae1ff31315da4ed08aa2fe) )

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("gun_dogs_obj-1.u0322", 0x000000, 0x400000, CRC(59d86c99) SHA1(d3c9241e7b51fe21f8351051b063f91dc69bf905) )
	ROM_LOAD("gun_dogs_obj-2.u0324", 0x400000, 0x400000, CRC(1ceb0b6f) SHA1(97225a9b3e7be18080aa52f6570af2cce8f25c06) )
	ROM_LOAD("gun_dogs_obj-3.u0323", 0x800000, 0x400000, CRC(36e93234) SHA1(51917a80b7da5c32a9434a1076fc2916d62e6a3e) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_WORD("gun_dogs_pcm.u0217",  0x000000, 0x100000, CRC(31253ad7) SHA1(c81c8d50f8f287f5cbfaec77b30d969b01ce11a9) )
	ROM_CONTINUE(                          0x400000, 0x100000 )
	ROM_LOAD32_BYTE("seibu_8.u0216",       0x800000, 0x080000, CRC(f88cb6e4) SHA1(fb35b41307b490d5d08e4b8a70f8ff4ce2ca8105) )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region8e.u1053", 0x000000, 0x100000, CRC(15dd4929) SHA1(e0f68e9e4d775d70e85ea6b5dd29beeb0e940b1c) )
ROM_END

ROM_START( rdftjb ) // SXX2C ROM SUB4 cart
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("seibu_1.u0211",        0x000000, 0x080000, CRC(b70afcc2) SHA1(70ac545a9fc30df310254997674878fbc2c2d718) ) // socket is silkscreened on pcb PRG0 - sldh
	ROM_LOAD32_BYTE("raiden-f_prg2.u0212",  0x000001, 0x080000, CRC(58ccb10c) SHA1(0cce4057bfada78121d9586574b98d46cdd7dd46) ) // socket is silkscreened on pcb PRG1
	ROM_LOAD32_WORD("raiden-f_prg34.u0219", 0x000002, 0x100000, CRC(63f01d17) SHA1(74dbd0417b974583da87fc6c7a081b03fd4e16b8) ) // socket is silkscreened on pcb PRG23

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_WORD("raiden-f_fix.u0425", 0x000000, 0x20000, CRC(2be2936b) SHA1(9e719f7328a52af220b6f084c1e0990ca6e2d533) ) // socket is silkscreened on pcb FIX01
	ROM_LOAD24_BYTE("seibu_7.u048",       0x000002, 0x10000, CRC(4d87e1ea) SHA1(3230e9b643fad773e61ab8ce09c0cd7d4d0558e3) ) // socket is silkscreened on pcb FIXP

	ROM_REGION( 0x600000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("gun_dogs_bg1-d.u0415", 0x000000, 0x200000, CRC(6a68054c) SHA1(5cbfc4ac90045f1401c2dda7a51936558c9de07e) ) // pads are silkscreened on pcb BG12
	ROM_LOAD24_BYTE("gun_dogs_bg1-p.u0410", 0x000002, 0x100000, CRC(3400794a) SHA1(719808f7442bac612cefd7b7fffcd665e6337ad0) ) // pads are silkscreened on pcb BG12P
	ROM_LOAD24_WORD("gun_dogs_bg2-d.u0424", 0x300000, 0x200000, CRC(61cd2991) SHA1(bb608e3948bf9ea35b5e1615d2ba6858d029dcbe) ) // pads are silkscreened on pcb BG3
	ROM_LOAD24_BYTE("gun_dogs_bg2-p.u049",  0x300002, 0x100000, CRC(502d5799) SHA1(c3a0e1a4f5a7b35572ae1ff31315da4ed08aa2fe) ) // pads are silkscreened on pcb BG3P

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("gun_dogs_obj-1.u0322", 0x000000, 0x400000, CRC(59d86c99) SHA1(d3c9241e7b51fe21f8351051b063f91dc69bf905) ) // pads are silkscreened on pcb OBJ1
	ROM_LOAD("gun_dogs_obj-2.u0324", 0x400000, 0x400000, CRC(1ceb0b6f) SHA1(97225a9b3e7be18080aa52f6570af2cce8f25c06) ) // pads are silkscreened on pcb OBJ2
	ROM_LOAD("gun_dogs_obj-3.u0323", 0x800000, 0x400000, CRC(36e93234) SHA1(51917a80b7da5c32a9434a1076fc2916d62e6a3e) ) // pads are silkscreened on pcb OBJ3

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_WORD("raiden-f_pcm2.u0217", 0x000000, 0x100000, CRC(3f8d4a48) SHA1(30664a2908daaeaee58f7e157516b522c952e48d) ) // pads are silkscreened SOUND0
	ROM_CONTINUE(                          0x400000, 0x100000 )
	/* SOUND1 socket is unpopulated */

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region01.u1053", 0x000000, 0x100000, CRC(7ae7ab76) SHA1(a2b196f470bf64af94002fc4e2640fadad00418f) )
ROM_END

ROM_START( rdftua ) // SXX2C ROM SUB4 cart
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("seibu_1.u0211",        0x000000, 0x080000, CRC(ddbadc30) SHA1(d419847db8b7f4ca6f161bfa309314eafeea8b40) ) // socket is silkscreened on pcb PRG0 - sldh
	ROM_LOAD32_BYTE("raiden-f_prg2.u0212",  0x000001, 0x080000, CRC(58ccb10c) SHA1(0cce4057bfada78121d9586574b98d46cdd7dd46) ) // socket is silkscreened on pcb PRG1
	ROM_LOAD32_WORD("raiden-f_prg34.u0219", 0x000002, 0x100000, CRC(63f01d17) SHA1(74dbd0417b974583da87fc6c7a081b03fd4e16b8) ) // socket is silkscreened on pcb PRG23

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_WORD("raiden-f_fix.u0425", 0x000000, 0x20000, CRC(2be2936b) SHA1(9e719f7328a52af220b6f084c1e0990ca6e2d533) ) // socket is silkscreened on pcb FIX01
	ROM_LOAD24_BYTE("seibu_7.u048",       0x000002, 0x10000, CRC(4d87e1ea) SHA1(3230e9b643fad773e61ab8ce09c0cd7d4d0558e3) ) // socket is silkscreened on pcb FIXP

	ROM_REGION( 0x600000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("gun_dogs_bg1-d.u0415", 0x000000, 0x200000, CRC(6a68054c) SHA1(5cbfc4ac90045f1401c2dda7a51936558c9de07e) ) // pads are silkscreened on pcb BG12
	ROM_LOAD24_BYTE("gun_dogs_bg1-p.u0410", 0x000002, 0x100000, CRC(3400794a) SHA1(719808f7442bac612cefd7b7fffcd665e6337ad0) ) // pads are silkscreened on pcb BG12P
	ROM_LOAD24_WORD("gun_dogs_bg2-d.u0424", 0x300000, 0x200000, CRC(61cd2991) SHA1(bb608e3948bf9ea35b5e1615d2ba6858d029dcbe) ) // pads are silkscreened on pcb BG3
	ROM_LOAD24_BYTE("gun_dogs_bg2-p.u049",  0x300002, 0x100000, CRC(502d5799) SHA1(c3a0e1a4f5a7b35572ae1ff31315da4ed08aa2fe) ) // pads are silkscreened on pcb BG3P

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("gun_dogs_obj-1.u0322", 0x000000, 0x400000, CRC(59d86c99) SHA1(d3c9241e7b51fe21f8351051b063f91dc69bf905) ) // pads are silkscreened on pcb OBJ1
	ROM_LOAD("gun_dogs_obj-2.u0324", 0x400000, 0x400000, CRC(1ceb0b6f) SHA1(97225a9b3e7be18080aa52f6570af2cce8f25c06) ) // pads are silkscreened on pcb OBJ2
	ROM_LOAD("gun_dogs_obj-3.u0323", 0x800000, 0x400000, CRC(36e93234) SHA1(51917a80b7da5c32a9434a1076fc2916d62e6a3e) ) // pads are silkscreened on pcb OBJ3

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_WORD("raiden-f_pcm2.u0217", 0x000000, 0x100000, CRC(3f8d4a48) SHA1(30664a2908daaeaee58f7e157516b522c952e48d) ) // pads are silkscreened SOUND0
	ROM_CONTINUE(                          0x400000, 0x100000 )
	/* SOUND1 socket is unpopulated */

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region10.u1053", 0x000000, 0x100000, CRC(4319d998) SHA1(a064ce647453a9b3bccf7f1d6d0d52b5a72e09dd) )
ROM_END

ROM_START( rdftadi ) // Dream Island license - SXX2C ROM SUB4 cart
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("seibu_1.u0211",        0x000000, 0x080000, CRC(fc0e2885) SHA1(79621155d992d504e993bd3ee0d6ff3903bd5415) ) // socket is silkscreened on pcb PRG0 - sldh
	ROM_LOAD32_BYTE("raiden-f_prg2.u0212",  0x000001, 0x080000, CRC(58ccb10c) SHA1(0cce4057bfada78121d9586574b98d46cdd7dd46) ) // socket is silkscreened on pcb PRG1
	ROM_LOAD32_WORD("raiden-f_prg34.u0219", 0x000002, 0x100000, CRC(63f01d17) SHA1(74dbd0417b974583da87fc6c7a081b03fd4e16b8) ) // socket is silkscreened on pcb PRG23

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_WORD("raiden-f_fix.u0425", 0x000000, 0x20000, CRC(2be2936b) SHA1(9e719f7328a52af220b6f084c1e0990ca6e2d533) ) // socket is silkscreened on pcb FIX01
	ROM_LOAD24_BYTE("seibu_7.u048",       0x000002, 0x10000, CRC(4d87e1ea) SHA1(3230e9b643fad773e61ab8ce09c0cd7d4d0558e3) ) // socket is silkscreened on pcb FIXP

	ROM_REGION( 0x600000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("gun_dogs_bg1-d.u0415", 0x000000, 0x200000, CRC(6a68054c) SHA1(5cbfc4ac90045f1401c2dda7a51936558c9de07e) ) // pads are silkscreened on pcb BG12
	ROM_LOAD24_BYTE("gun_dogs_bg1-p.u0410", 0x000002, 0x100000, CRC(3400794a) SHA1(719808f7442bac612cefd7b7fffcd665e6337ad0) ) // pads are silkscreened on pcb BG12P
	ROM_LOAD24_WORD("gun_dogs_bg2-d.u0424", 0x300000, 0x200000, CRC(61cd2991) SHA1(bb608e3948bf9ea35b5e1615d2ba6858d029dcbe) ) // pads are silkscreened on pcb BG3
	ROM_LOAD24_BYTE("gun_dogs_bg2-p.u049",  0x300002, 0x100000, CRC(502d5799) SHA1(c3a0e1a4f5a7b35572ae1ff31315da4ed08aa2fe) ) // pads are silkscreened on pcb BG3P

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("gun_dogs_obj-1.u0322", 0x000000, 0x400000, CRC(59d86c99) SHA1(d3c9241e7b51fe21f8351051b063f91dc69bf905) ) // pads are silkscreened on pcb OBJ1
	ROM_LOAD("gun_dogs_obj-2.u0324", 0x400000, 0x400000, CRC(1ceb0b6f) SHA1(97225a9b3e7be18080aa52f6570af2cce8f25c06) ) // pads are silkscreened on pcb OBJ2
	ROM_LOAD("gun_dogs_obj-3.u0323", 0x800000, 0x400000, CRC(36e93234) SHA1(51917a80b7da5c32a9434a1076fc2916d62e6a3e) ) // pads are silkscreened on pcb OBJ3

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_WORD("raiden-f_pcm2.u0217", 0x000000, 0x100000, CRC(3f8d4a48) SHA1(30664a2908daaeaee58f7e157516b522c952e48d) ) // pads are silkscreened SOUND0
	ROM_CONTINUE(                          0x400000, 0x100000 )
	/* SOUND1 socket is unpopulated */

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region24.u1053", 0x000000, 0x100000, CRC(72a33dc4) SHA1(65a52f576ca4d240418fedd9a4922edcd6c0c8d1) )
ROM_END

ROM_START( rdftam ) // Metrotainment license - SXX2C ROM SUB4 cart
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("seibu_1.u0211",        0x000000, 0x080000, CRC(156d8db0) SHA1(93662b3ee494e37a56428a7aa3dad7a957835950) ) // socket is silkscreened on pcb PRG0 - sldh
	ROM_LOAD32_BYTE("raiden-f_prg2.u0212",  0x000001, 0x080000, CRC(58ccb10c) SHA1(0cce4057bfada78121d9586574b98d46cdd7dd46) ) // socket is silkscreened on pcb PRG1
	ROM_LOAD32_WORD("raiden-f_prg34.u0219", 0x000002, 0x100000, CRC(63f01d17) SHA1(74dbd0417b974583da87fc6c7a081b03fd4e16b8) ) // socket is silkscreened on pcb PRG23

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_WORD("raiden-f_fix.u0425", 0x000000, 0x20000, CRC(2be2936b) SHA1(9e719f7328a52af220b6f084c1e0990ca6e2d533) ) // socket is silkscreened on pcb FIX01
	ROM_LOAD24_BYTE("seibu_7.u048",       0x000002, 0x10000, CRC(4d87e1ea) SHA1(3230e9b643fad773e61ab8ce09c0cd7d4d0558e3) ) // socket is silkscreened on pcb FIXP

	ROM_REGION( 0x600000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("gun_dogs_bg1-d.u0415", 0x000000, 0x200000, CRC(6a68054c) SHA1(5cbfc4ac90045f1401c2dda7a51936558c9de07e) ) // pads are silkscreened on pcb BG12
	ROM_LOAD24_BYTE("gun_dogs_bg1-p.u0410", 0x000002, 0x100000, CRC(3400794a) SHA1(719808f7442bac612cefd7b7fffcd665e6337ad0) ) // pads are silkscreened on pcb BG12P
	ROM_LOAD24_WORD("gun_dogs_bg2-d.u0424", 0x300000, 0x200000, CRC(61cd2991) SHA1(bb608e3948bf9ea35b5e1615d2ba6858d029dcbe) ) // pads are silkscreened on pcb BG3
	ROM_LOAD24_BYTE("gun_dogs_bg2-p.u049",  0x300002, 0x100000, CRC(502d5799) SHA1(c3a0e1a4f5a7b35572ae1ff31315da4ed08aa2fe) ) // pads are silkscreened on pcb BG3P

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("gun_dogs_obj-1.u0322", 0x000000, 0x400000, CRC(59d86c99) SHA1(d3c9241e7b51fe21f8351051b063f91dc69bf905) ) // pads are silkscreened on pcb OBJ1
	ROM_LOAD("gun_dogs_obj-2.u0324", 0x400000, 0x400000, CRC(1ceb0b6f) SHA1(97225a9b3e7be18080aa52f6570af2cce8f25c06) ) // pads are silkscreened on pcb OBJ2
	ROM_LOAD("gun_dogs_obj-3.u0323", 0x800000, 0x400000, CRC(36e93234) SHA1(51917a80b7da5c32a9434a1076fc2916d62e6a3e) ) // pads are silkscreened on pcb OBJ3

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_WORD("raiden-f_pcm2.u0217", 0x000000, 0x100000, CRC(3f8d4a48) SHA1(30664a2908daaeaee58f7e157516b522c952e48d) ) // pads are silkscreened SOUND0
	ROM_CONTINUE(                          0x400000, 0x100000 )
	/* SOUND1 socket is unpopulated */

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region22.u1053", 0x000000, 0x100000, CRC(5fee8413) SHA1(6d6a62fa01293b4ba4b349a39820d024add6ea22) )
ROM_END


ROM_START( rdft2 ) /* SPI Cart, Europe */
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("prg0.tun", 0x000000, 0x80000, CRC(3cb3fdca) SHA1(4b472dfd65c7bbbcb92a295aa73b0fa70581455b) )
	ROM_LOAD32_BYTE("prg1.bin", 0x000001, 0x80000, CRC(cab55d88) SHA1(246e13880d34b6c7c3f4ab5e18fa8a0547c03d9d) )
	ROM_LOAD32_BYTE("prg2.bin", 0x000002, 0x80000, CRC(83758b0e) SHA1(63adb2d09e7bd7dba47a55b3b579d543dfb553e3) )
	ROM_LOAD32_BYTE("prg3.bin", 0x000003, 0x80000, CRC(084fb5e4) SHA1(588bfe091662b88f02f528181a2f1d9c67c7b280) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_BYTE("fix0.u0524", 0x000001, 0x10000, CRC(6fdf4cf6) SHA1(7e9d4a49e829dfdc373c0f5acfbe8c7a91ac115b) )
	ROM_LOAD24_BYTE("fix1.u0518", 0x000000, 0x10000, CRC(69b7899b) SHA1(d3cacd4ef4d2c95d803403101beb9d4be75fae61) )
	ROM_LOAD24_BYTE("fixp.u0514", 0x000002, 0x10000, CRC(99a5fece) SHA1(44ae95d650ed6e00202d3438f5f91a5e52e319cb) )

	ROM_REGION( 0xc00000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0535", 0x000000, 0x400000, CRC(6143f576) SHA1(c034923d0663d9ef24357a03098b8cb81dbab9f8) )
	ROM_LOAD24_BYTE("bg-1p.u0537", 0x000002, 0x200000, CRC(55e64ef7) SHA1(aae991268948d07342ee8ba1b3761bd180aab8ec) )
	ROM_LOAD24_WORD("bg-2d.u0536", 0x600000, 0x400000, CRC(c607a444) SHA1(dc1aa96a42e9394ca6036359670a4ec6f830c96d) )
	ROM_LOAD24_BYTE("bg-2p.u0538", 0x600002, 0x200000, CRC(f0830248) SHA1(6075df96b49e70d2243fef691e096119e7a4d044) )

	ROM_REGION( 0x1200000, "sprites", 0 ) /* sprites */
	ROM_LOAD("obj3.u0434",  0x0000000, 0x400000, CRC(e08f42dc) SHA1(5188d71d4355eaf43ea8893b4cfc4fe80cc24f41) )
	ROM_LOAD("obj3b.u0433", 0x0400000, 0x200000, CRC(1b6a523c) SHA1(99a420dbc8e22e7832ccda7cec9fa661a2a2687a) )
	ROM_LOAD("obj2.u0431",  0x0600000, 0x400000, CRC(7aeadd8e) SHA1(47103c0579240c5b1add4d0b164eaf76f5fa97f0) )
	ROM_LOAD("obj2b.u0432", 0x0a00000, 0x200000, CRC(5d790a5d) SHA1(1ed5d4ad4c9a7e505ce35dcc90d184c26ce891dc) )
	ROM_LOAD("obj1.u0429",  0x0c00000, 0x400000, CRC(c2c50f02) SHA1(b81397b5800c6d49f58b7ac7ff6eac56da3c5257) )
	ROM_LOAD("obj1b.u0430", 0x1000000, 0x200000, CRC(5259321f) SHA1(3c70c1147e49f81371d0f60f7108d9718d56faf4) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_WORD("pcm.u0217",    0x000000, 0x100000, CRC(2edc30b5) SHA1(c25d690d633657fc3687636b9070f36bd305ae06) )
	ROM_CONTINUE(                   0x400000, 0x100000 )
	ROM_LOAD32_BYTE("sound1.u0222", 0x800000, 0x080000, CRC(b7bd3703) SHA1(6427a7e6de10d6743d6e64b984a1d1c647f5643a) )

	ROM_REGION( 0x0345, "pals", 0 ) /* pals */
	ROM_LOAD("rm81.u0529.bin", 0x0000, 0x0117, CRC(acd55c8e) SHA1(b965e828fecd61b836aca337637e53d7360d9dc4) ) // AMD PALCE16V8H-15SC/4
	ROM_LOAD("rm82.u0330.bin", 0x0117, 0x0117, CRC(64c71423) SHA1(1da3502bec0c843b7198d1d9ab60f9fd4b110a8e) ) // AMD PALCE16V8H-15SC/4
	ROM_LOAD("rm83.u0331.bin", 0x022e, 0x0117, CRC(6e10d66b) SHA1(995d2a0da680ec19ee253098c91a4780dd8403c6) ) // AMD PALCE16V8H-15SC/4

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region80.u1053", 0x000000, 0x100000, CRC(e2adaff5) SHA1(9297afaf78209724515d8f78de8cee7bc7cb796b) )
ROM_END

ROM_START( rdft2u ) /* SPI Cart, USA */
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("1.bin", 0x000000, 0x80000, CRC(b7d6c866) SHA1(eefe63dfc641c3904dd150a10ffeb68137068725) )
	ROM_LOAD32_BYTE("2.bin", 0x000001, 0x80000, CRC(ff7747c5) SHA1(7481d0484001ff7367af56e8ea99f985cce405f2) )
	ROM_LOAD32_BYTE("3.bin", 0x000002, 0x80000, CRC(86e3d1a8) SHA1(2757cfda57c82dd0f66427caf54eb1f40e85740d) )
	ROM_LOAD32_BYTE("4.bin", 0x000003, 0x80000, CRC(2e409a76) SHA1(cf90aa14a07b5aa861f6f7cc9b1968171e532557) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_BYTE("fix0.u0524", 0x000001, 0x10000, CRC(6fdf4cf6) SHA1(7e9d4a49e829dfdc373c0f5acfbe8c7a91ac115b) )
	ROM_LOAD24_BYTE("fix1.u0518", 0x000000, 0x10000, CRC(69b7899b) SHA1(d3cacd4ef4d2c95d803403101beb9d4be75fae61) )
	ROM_LOAD24_BYTE("fixp.u0514", 0x000002, 0x10000, CRC(99a5fece) SHA1(44ae95d650ed6e00202d3438f5f91a5e52e319cb) )

	ROM_REGION( 0xc00000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0535", 0x000000, 0x400000, CRC(6143f576) SHA1(c034923d0663d9ef24357a03098b8cb81dbab9f8) )
	ROM_LOAD24_BYTE("bg-1p.u0537", 0x000002, 0x200000, CRC(55e64ef7) SHA1(aae991268948d07342ee8ba1b3761bd180aab8ec) )
	ROM_LOAD24_WORD("bg-2d.u0536", 0x600000, 0x400000, CRC(c607a444) SHA1(dc1aa96a42e9394ca6036359670a4ec6f830c96d) )
	ROM_LOAD24_BYTE("bg-2p.u0538", 0x600002, 0x200000, CRC(f0830248) SHA1(6075df96b49e70d2243fef691e096119e7a4d044) )

	ROM_REGION( 0x1200000, "sprites", 0 ) /* sprites */
	ROM_LOAD("obj3.u0434",  0x0000000, 0x400000, CRC(e08f42dc) SHA1(5188d71d4355eaf43ea8893b4cfc4fe80cc24f41) )
	ROM_LOAD("obj3b.u0433", 0x0400000, 0x200000, CRC(1b6a523c) SHA1(99a420dbc8e22e7832ccda7cec9fa661a2a2687a) )
	ROM_LOAD("obj2.u0431",  0x0600000, 0x400000, CRC(7aeadd8e) SHA1(47103c0579240c5b1add4d0b164eaf76f5fa97f0) )
	ROM_LOAD("obj2b.u0432", 0x0a00000, 0x200000, CRC(5d790a5d) SHA1(1ed5d4ad4c9a7e505ce35dcc90d184c26ce891dc) )
	ROM_LOAD("obj1.u0429",  0x0c00000, 0x400000, CRC(c2c50f02) SHA1(b81397b5800c6d49f58b7ac7ff6eac56da3c5257) )
	ROM_LOAD("obj1b.u0430", 0x1000000, 0x200000, CRC(5259321f) SHA1(3c70c1147e49f81371d0f60f7108d9718d56faf4) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_WORD("pcm.u0217",    0x000000, 0x100000, CRC(2edc30b5) SHA1(c25d690d633657fc3687636b9070f36bd305ae06) )
	ROM_CONTINUE(                   0x400000, 0x100000 )
	ROM_LOAD32_BYTE("sound1.u0222", 0x800000, 0x080000, CRC(b7bd3703) SHA1(6427a7e6de10d6743d6e64b984a1d1c647f5643a) )

	ROM_REGION( 0x0345, "pals", 0 ) /* pals */
	ROM_LOAD("rm81.u0529.bin", 0x0000, 0x0117, CRC(acd55c8e) SHA1(b965e828fecd61b836aca337637e53d7360d9dc4) ) // AMD PALCE16V8H-15SC/4
	ROM_LOAD("rm82.u0330.bin", 0x0117, 0x0117, CRC(64c71423) SHA1(1da3502bec0c843b7198d1d9ab60f9fd4b110a8e) ) // AMD PALCE16V8H-15SC/4
	ROM_LOAD("rm83.u0331.bin", 0x022e, 0x0117, CRC(6e10d66b) SHA1(995d2a0da680ec19ee253098c91a4780dd8403c6) ) // AMD PALCE16V8H-15SC/4

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region10.u1053", 0x000000, 0x100000, CRC(4319d998) SHA1(a064ce647453a9b3bccf7f1d6d0d52b5a72e09dd) )
ROM_END

ROM_START( rdft2j ) /* SPI Cart, Japan */
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("prg0.sei", 0x000000, 0x80000, CRC(a60c4e7c) SHA1(7789b029d0ac084c7e5e662a7168edaed8f11633) )
	ROM_LOAD32_BYTE("prg1.bin", 0x000001, 0x80000, CRC(cab55d88) SHA1(246e13880d34b6c7c3f4ab5e18fa8a0547c03d9d) )
	ROM_LOAD32_BYTE("prg2.bin", 0x000002, 0x80000, CRC(83758b0e) SHA1(63adb2d09e7bd7dba47a55b3b579d543dfb553e3) )
	ROM_LOAD32_BYTE("prg3.bin", 0x000003, 0x80000, CRC(084fb5e4) SHA1(588bfe091662b88f02f528181a2f1d9c67c7b280) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_BYTE("fix0.u0524", 0x000001, 0x10000, CRC(6fdf4cf6) SHA1(7e9d4a49e829dfdc373c0f5acfbe8c7a91ac115b) )
	ROM_LOAD24_BYTE("fix1.u0518", 0x000000, 0x10000, CRC(69b7899b) SHA1(d3cacd4ef4d2c95d803403101beb9d4be75fae61) )
	ROM_LOAD24_BYTE("fixp.u0514", 0x000002, 0x10000, CRC(99a5fece) SHA1(44ae95d650ed6e00202d3438f5f91a5e52e319cb) )

	ROM_REGION( 0xc00000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0535", 0x000000, 0x400000, CRC(6143f576) SHA1(c034923d0663d9ef24357a03098b8cb81dbab9f8) )
	ROM_LOAD24_BYTE("bg-1p.u0537", 0x000002, 0x200000, CRC(55e64ef7) SHA1(aae991268948d07342ee8ba1b3761bd180aab8ec) )
	ROM_LOAD24_WORD("bg-2d.u0536", 0x600000, 0x400000, CRC(c607a444) SHA1(dc1aa96a42e9394ca6036359670a4ec6f830c96d) )
	ROM_LOAD24_BYTE("bg-2p.u0538", 0x600002, 0x200000, CRC(f0830248) SHA1(6075df96b49e70d2243fef691e096119e7a4d044) )

	ROM_REGION( 0x1200000, "sprites", 0 ) /* sprites */
	ROM_LOAD("obj3.u0434",  0x0000000, 0x400000, CRC(e08f42dc) SHA1(5188d71d4355eaf43ea8893b4cfc4fe80cc24f41) )
	ROM_LOAD("obj3b.u0433", 0x0400000, 0x200000, CRC(1b6a523c) SHA1(99a420dbc8e22e7832ccda7cec9fa661a2a2687a) )
	ROM_LOAD("obj2.u0431",  0x0600000, 0x400000, CRC(7aeadd8e) SHA1(47103c0579240c5b1add4d0b164eaf76f5fa97f0) )
	ROM_LOAD("obj2b.u0432", 0x0a00000, 0x200000, CRC(5d790a5d) SHA1(1ed5d4ad4c9a7e505ce35dcc90d184c26ce891dc) )
	ROM_LOAD("obj1.u0429",  0x0c00000, 0x400000, CRC(c2c50f02) SHA1(b81397b5800c6d49f58b7ac7ff6eac56da3c5257) )
	ROM_LOAD("obj1b.u0430", 0x1000000, 0x200000, CRC(5259321f) SHA1(3c70c1147e49f81371d0f60f7108d9718d56faf4) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_WORD("pcm.u0217",    0x000000, 0x100000, CRC(2edc30b5) SHA1(c25d690d633657fc3687636b9070f36bd305ae06) )
	ROM_CONTINUE(                   0x400000, 0x100000 )
	ROM_LOAD32_BYTE("sound1.u0222", 0x800000, 0x080000, CRC(b7bd3703) SHA1(6427a7e6de10d6743d6e64b984a1d1c647f5643a) )

	ROM_REGION( 0x0345, "pals", 0 ) /* pals */
	ROM_LOAD("rm81.u0529.bin", 0x0000, 0x0117, CRC(acd55c8e) SHA1(b965e828fecd61b836aca337637e53d7360d9dc4) ) // AMD PALCE16V8H-15SC/4
	ROM_LOAD("rm82.u0330.bin", 0x0117, 0x0117, CRC(64c71423) SHA1(1da3502bec0c843b7198d1d9ab60f9fd4b110a8e) ) // AMD PALCE16V8H-15SC/4
	ROM_LOAD("rm83.u0331.bin", 0x022e, 0x0117, CRC(6e10d66b) SHA1(995d2a0da680ec19ee253098c91a4780dd8403c6) ) // AMD PALCE16V8H-15SC/4

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region01.u1053", 0x000000, 0x100000, CRC(7ae7ab76) SHA1(a2b196f470bf64af94002fc4e2640fadad00418f) )
ROM_END

ROM_START( rdft2ja ) /* SPI Cart, Japan */
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("rf2.1",     0x000000, 0x80000, CRC(391d5057) SHA1(a1849142cbf7344ac1279781597e27b3b8ae6127) )
	ROM_LOAD32_BYTE("rf2_2.bin", 0x000001, 0x80000, CRC(ec73a767) SHA1(83f3905afe49401793c0ea0193cb31d3ba1e1739) )
	ROM_LOAD32_BYTE("rf2_3.bin", 0x000002, 0x80000, CRC(e66243b2) SHA1(54e67af37a4586fd1afc79085ed433d599e1bb87) )
	ROM_LOAD32_BYTE("rf2_4.bin", 0x000003, 0x80000, CRC(92b7b73e) SHA1(128649b2a6a0616113bd0f9846fb6cf814ae326d) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_BYTE("rf2_5.bin", 0x000001, 0x10000, CRC(377cac2f) SHA1(f7c9323d79b77f6c8c02ba2c6cdca127d6e5cb5c) )
	ROM_LOAD24_BYTE("rf2_6.bin", 0x000000, 0x10000, CRC(42bd5372) SHA1(c38df85b25070db9640eac541f71c0511bab0c98) )
	ROM_LOAD24_BYTE("rf2_7.bin", 0x000002, 0x10000, CRC(1efaac7e) SHA1(8252af56dcb7a6306dc3422070176778e3c511c2) )

	ROM_REGION( 0xc00000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0535", 0x000000, 0x400000, CRC(6143f576) SHA1(c034923d0663d9ef24357a03098b8cb81dbab9f8) )
	ROM_LOAD24_BYTE("bg-1p.u0537", 0x000002, 0x200000, CRC(55e64ef7) SHA1(aae991268948d07342ee8ba1b3761bd180aab8ec) )
	ROM_LOAD24_WORD("bg-2d.u0536", 0x600000, 0x400000, CRC(c607a444) SHA1(dc1aa96a42e9394ca6036359670a4ec6f830c96d) )
	ROM_LOAD24_BYTE("bg-2p.u0538", 0x600002, 0x200000, CRC(f0830248) SHA1(6075df96b49e70d2243fef691e096119e7a4d044) )

	ROM_REGION( 0x1200000, "sprites", 0 ) /* sprites */
	ROM_LOAD("obj3.u0434",  0x0000000, 0x400000, CRC(e08f42dc) SHA1(5188d71d4355eaf43ea8893b4cfc4fe80cc24f41) )
	ROM_LOAD("obj3b.u0433", 0x0400000, 0x200000, CRC(1b6a523c) SHA1(99a420dbc8e22e7832ccda7cec9fa661a2a2687a) )
	ROM_LOAD("obj2.u0431",  0x0600000, 0x400000, CRC(7aeadd8e) SHA1(47103c0579240c5b1add4d0b164eaf76f5fa97f0) )
	ROM_LOAD("obj2b.u0432", 0x0a00000, 0x200000, CRC(5d790a5d) SHA1(1ed5d4ad4c9a7e505ce35dcc90d184c26ce891dc) )
	ROM_LOAD("obj1.u0429",  0x0c00000, 0x400000, CRC(c2c50f02) SHA1(b81397b5800c6d49f58b7ac7ff6eac56da3c5257) )
	ROM_LOAD("obj1b.u0430", 0x1000000, 0x200000, CRC(5259321f) SHA1(3c70c1147e49f81371d0f60f7108d9718d56faf4) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_WORD("pcm.u0217",    0x000000, 0x100000, CRC(2edc30b5) SHA1(c25d690d633657fc3687636b9070f36bd305ae06) )
	ROM_CONTINUE(                   0x400000, 0x100000 )
	ROM_LOAD32_BYTE("sound1.u0222", 0x800000, 0x080000, CRC(b7bd3703) SHA1(6427a7e6de10d6743d6e64b984a1d1c647f5643a) )

	ROM_REGION( 0x0345, "pals", 0 ) /* pals */
	ROM_LOAD("rm81.u0529.bin", 0x0000, 0x0117, CRC(acd55c8e) SHA1(b965e828fecd61b836aca337637e53d7360d9dc4) ) // AMD PALCE16V8H-15SC/4
	ROM_LOAD("rm82.u0330.bin", 0x0117, 0x0117, CRC(64c71423) SHA1(1da3502bec0c843b7198d1d9ab60f9fd4b110a8e) ) // AMD PALCE16V8H-15SC/4
	ROM_LOAD("rm83.u0331.bin", 0x022e, 0x0117, CRC(6e10d66b) SHA1(995d2a0da680ec19ee253098c91a4780dd8403c6) ) // AMD PALCE16V8H-15SC/4

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region01.u1053", 0x000000, 0x100000, CRC(7ae7ab76) SHA1(a2b196f470bf64af94002fc4e2640fadad00418f) )
ROM_END

ROM_START( rdft2jb ) /* SPI Cart, Japan */
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("prg0.rom", 0x000000, 0x80000, CRC(fc42cab8) SHA1(2e33fc8d77fdc4ee58e93fc191d12f2fe9cc3c65) )
	ROM_LOAD32_BYTE("prg1.rom", 0x000001, 0x80000, CRC(a0f09dc5) SHA1(e8ad20be1f04752b0884571384d4490813ed82d9) )
	ROM_LOAD32_BYTE("prg2.rom", 0x000002, 0x80000, CRC(368580e0) SHA1(184036a0cbddbf79b62e388b93cb93b885faee88) )
	ROM_LOAD32_BYTE("prg3.rom", 0x000003, 0x80000, CRC(7ad45c01) SHA1(d782057658dd000c1cf0a4726a6ed821e6f2be67) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_BYTE("rf2_5.bin", 0x000001, 0x10000, CRC(377cac2f) SHA1(f7c9323d79b77f6c8c02ba2c6cdca127d6e5cb5c) )
	ROM_LOAD24_BYTE("rf2_6.bin", 0x000000, 0x10000, CRC(42bd5372) SHA1(c38df85b25070db9640eac541f71c0511bab0c98) )
	ROM_LOAD24_BYTE("rf2_7.bin", 0x000002, 0x10000, CRC(1efaac7e) SHA1(8252af56dcb7a6306dc3422070176778e3c511c2) )

	ROM_REGION( 0xc00000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0535", 0x000000, 0x400000, CRC(6143f576) SHA1(c034923d0663d9ef24357a03098b8cb81dbab9f8) )
	ROM_LOAD24_BYTE("bg-1p.u0537", 0x000002, 0x200000, CRC(55e64ef7) SHA1(aae991268948d07342ee8ba1b3761bd180aab8ec) )
	ROM_LOAD24_WORD("bg-2d.u0536", 0x600000, 0x400000, CRC(c607a444) SHA1(dc1aa96a42e9394ca6036359670a4ec6f830c96d) )
	ROM_LOAD24_BYTE("bg-2p.u0538", 0x600002, 0x200000, CRC(f0830248) SHA1(6075df96b49e70d2243fef691e096119e7a4d044) )

	ROM_REGION( 0x1200000, "sprites", 0 ) /* sprites */
	ROM_LOAD("obj3.u0434",  0x0000000, 0x400000, CRC(e08f42dc) SHA1(5188d71d4355eaf43ea8893b4cfc4fe80cc24f41) )
	ROM_LOAD("obj3b.u0433", 0x0400000, 0x200000, CRC(1b6a523c) SHA1(99a420dbc8e22e7832ccda7cec9fa661a2a2687a) )
	ROM_LOAD("obj2.u0431",  0x0600000, 0x400000, CRC(7aeadd8e) SHA1(47103c0579240c5b1add4d0b164eaf76f5fa97f0) )
	ROM_LOAD("obj2b.u0432", 0x0a00000, 0x200000, CRC(5d790a5d) SHA1(1ed5d4ad4c9a7e505ce35dcc90d184c26ce891dc) )
	ROM_LOAD("obj1.u0429",  0x0c00000, 0x400000, CRC(c2c50f02) SHA1(b81397b5800c6d49f58b7ac7ff6eac56da3c5257) )
	ROM_LOAD("obj1b.u0430", 0x1000000, 0x200000, CRC(5259321f) SHA1(3c70c1147e49f81371d0f60f7108d9718d56faf4) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_WORD("pcm.u0217",    0x000000, 0x100000, CRC(2edc30b5) SHA1(c25d690d633657fc3687636b9070f36bd305ae06) )
	ROM_CONTINUE(                   0x400000, 0x100000 )
	ROM_LOAD32_BYTE("sound1.u0222", 0x800000, 0x080000, CRC(b7bd3703) SHA1(6427a7e6de10d6743d6e64b984a1d1c647f5643a) )

	ROM_REGION( 0x0345, "pals", 0 ) /* pals */
	ROM_LOAD("rm81.u0529.bin", 0x0000, 0x0117, CRC(acd55c8e) SHA1(b965e828fecd61b836aca337637e53d7360d9dc4) ) // AMD PALCE16V8H-15SC/4
	ROM_LOAD("rm82.u0330.bin", 0x0117, 0x0117, CRC(64c71423) SHA1(1da3502bec0c843b7198d1d9ab60f9fd4b110a8e) ) // AMD PALCE16V8H-15SC/4
	ROM_LOAD("rm83.u0331.bin", 0x022e, 0x0117, CRC(6e10d66b) SHA1(995d2a0da680ec19ee253098c91a4780dd8403c6) ) // AMD PALCE16V8H-15SC/4

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region01.u1053", 0x000000, 0x100000, CRC(7ae7ab76) SHA1(a2b196f470bf64af94002fc4e2640fadad00418f) )
ROM_END

ROM_START( rdft2jc ) /* SPI SXX2C ROM SUB8 Cart, Japan */
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("seibu_1.u0211", 0x000000, 0x80000, CRC(36b6407c) SHA1(b3f5bb3c582aca71e0c7d9da5951b30c7389cc24) )
	ROM_LOAD32_BYTE("seibu_2.u0212", 0x000001, 0x80000, CRC(65ee556e) SHA1(13311850aabba9fc373adfd5cd590c114505933f) )
	ROM_LOAD32_BYTE("seibu_3.u0221", 0x000002, 0x80000, CRC(d2458358) SHA1(18a9cfee77a6a09584bc3fb0073c822d12de5bf1) )
	ROM_LOAD32_BYTE("seibu_4.u0220", 0x000003, 0x80000, CRC(5c4412f9) SHA1(c72603bee3ce14f40d4bf5e3ae3f041b923edd57) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_BYTE("fix0.u0524", 0x000001, 0x10000, CRC(6fdf4cf6) SHA1(7e9d4a49e829dfdc373c0f5acfbe8c7a91ac115b) )
	ROM_LOAD24_BYTE("fix1.u0518", 0x000000, 0x10000, CRC(69b7899b) SHA1(d3cacd4ef4d2c95d803403101beb9d4be75fae61) )
	ROM_LOAD24_BYTE("fixp.u0514", 0x000002, 0x10000, CRC(99a5fece) SHA1(44ae95d650ed6e00202d3438f5f91a5e52e319cb) )

	ROM_REGION( 0xc00000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0535", 0x000000, 0x400000, CRC(6143f576) SHA1(c034923d0663d9ef24357a03098b8cb81dbab9f8) )
	ROM_LOAD24_BYTE("bg-1p.u0537", 0x000002, 0x200000, CRC(55e64ef7) SHA1(aae991268948d07342ee8ba1b3761bd180aab8ec) )
	ROM_LOAD24_WORD("bg-2d.u0536", 0x600000, 0x400000, CRC(c607a444) SHA1(dc1aa96a42e9394ca6036359670a4ec6f830c96d) )
	ROM_LOAD24_BYTE("bg-2p.u0538", 0x600002, 0x200000, CRC(f0830248) SHA1(6075df96b49e70d2243fef691e096119e7a4d044) )

	ROM_REGION( 0x1200000, "sprites", 0 ) /* sprites */
	ROM_LOAD("obj3.u0434",  0x0000000, 0x400000, CRC(e08f42dc) SHA1(5188d71d4355eaf43ea8893b4cfc4fe80cc24f41) )
	ROM_LOAD("obj3b.u0433", 0x0400000, 0x200000, CRC(1b6a523c) SHA1(99a420dbc8e22e7832ccda7cec9fa661a2a2687a) )
	ROM_LOAD("obj2.u0431",  0x0600000, 0x400000, CRC(7aeadd8e) SHA1(47103c0579240c5b1add4d0b164eaf76f5fa97f0) )
	ROM_LOAD("obj2b.u0432", 0x0a00000, 0x200000, CRC(5d790a5d) SHA1(1ed5d4ad4c9a7e505ce35dcc90d184c26ce891dc) )
	ROM_LOAD("obj1.u0429",  0x0c00000, 0x400000, CRC(c2c50f02) SHA1(b81397b5800c6d49f58b7ac7ff6eac56da3c5257) )
	ROM_LOAD("obj1b.u0430", 0x1000000, 0x200000, CRC(5259321f) SHA1(3c70c1147e49f81371d0f60f7108d9718d56faf4) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_WORD("pcm.u0217",    0x000000, 0x100000, CRC(2edc30b5) SHA1(c25d690d633657fc3687636b9070f36bd305ae06) )
	ROM_CONTINUE(                   0x400000, 0x100000 )
	ROM_LOAD32_BYTE("sound1.u0222", 0x800000, 0x080000, CRC(b7bd3703) SHA1(6427a7e6de10d6743d6e64b984a1d1c647f5643a) )

	ROM_REGION( 0x0345, "pals", 0 ) /* pals */
	ROM_LOAD("rm81.u0529.bin", 0x0000, 0x0117, CRC(acd55c8e) SHA1(b965e828fecd61b836aca337637e53d7360d9dc4) ) // AMD PALCE16V8H-15SC/4
	ROM_LOAD("rm82.u0330.bin", 0x0117, 0x0117, CRC(64c71423) SHA1(1da3502bec0c843b7198d1d9ab60f9fd4b110a8e) ) // AMD PALCE16V8H-15SC/4
	ROM_LOAD("rm83.u0331.bin", 0x022e, 0x0117, CRC(6e10d66b) SHA1(995d2a0da680ec19ee253098c91a4780dd8403c6) ) // AMD PALCE16V8H-15SC/4

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region01.u1053", 0x000000, 0x100000, CRC(7ae7ab76) SHA1(a2b196f470bf64af94002fc4e2640fadad00418f) )
ROM_END

ROM_START( rdft2it ) /* SPI Cart, Italy */
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("seibu1.bin",0x000000, 0x80000, CRC(501b92a9) SHA1(3e1c5cc63906ec7b97a3478557ec2638c515d726) )
	ROM_LOAD32_BYTE("seibu2.bin",0x000001, 0x80000, CRC(ec73a767) SHA1(83f3905afe49401793c0ea0193cb31d3ba1e1739) )
	ROM_LOAD32_BYTE("seibu3.bin",0x000002, 0x80000, CRC(e66243b2) SHA1(54e67af37a4586fd1afc79085ed433d599e1bb87) )
	ROM_LOAD32_BYTE("seibu4.bin",0x000003, 0x80000, CRC(92b7b73e) SHA1(128649b2a6a0616113bd0f9846fb6cf814ae326d) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_BYTE("seibu5.bin", 0x000001, 0x10000, CRC(377cac2f) SHA1(f7c9323d79b77f6c8c02ba2c6cdca127d6e5cb5c) )
	ROM_LOAD24_BYTE("seibu6.bin", 0x000000, 0x10000, CRC(42bd5372) SHA1(c38df85b25070db9640eac541f71c0511bab0c98) )
	ROM_LOAD24_BYTE("seibu7.bin", 0x000002, 0x10000, CRC(1efaac7e) SHA1(8252af56dcb7a6306dc3422070176778e3c511c2) )

	ROM_REGION( 0xc00000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0535", 0x000000, 0x400000, CRC(6143f576) SHA1(c034923d0663d9ef24357a03098b8cb81dbab9f8) )
	ROM_LOAD24_BYTE("bg-1p.u0537", 0x000002, 0x200000, CRC(55e64ef7) SHA1(aae991268948d07342ee8ba1b3761bd180aab8ec) )
	ROM_LOAD24_WORD("bg-2d.u0536", 0x600000, 0x400000, CRC(c607a444) SHA1(dc1aa96a42e9394ca6036359670a4ec6f830c96d) )
	ROM_LOAD24_BYTE("bg-2p.u0538", 0x600002, 0x200000, CRC(f0830248) SHA1(6075df96b49e70d2243fef691e096119e7a4d044) )

	ROM_REGION( 0x1200000, "sprites", 0 ) /* sprites */
	ROM_LOAD("obj3.u0434",  0x0000000, 0x400000, CRC(e08f42dc) SHA1(5188d71d4355eaf43ea8893b4cfc4fe80cc24f41) )
	ROM_LOAD("obj3b.u0433", 0x0400000, 0x200000, CRC(1b6a523c) SHA1(99a420dbc8e22e7832ccda7cec9fa661a2a2687a) )
	ROM_LOAD("obj2.u0431",  0x0600000, 0x400000, CRC(7aeadd8e) SHA1(47103c0579240c5b1add4d0b164eaf76f5fa97f0) )
	ROM_LOAD("obj2b.u0432", 0x0a00000, 0x200000, CRC(5d790a5d) SHA1(1ed5d4ad4c9a7e505ce35dcc90d184c26ce891dc) )
	ROM_LOAD("obj1.u0429",  0x0c00000, 0x400000, CRC(c2c50f02) SHA1(b81397b5800c6d49f58b7ac7ff6eac56da3c5257) )
	ROM_LOAD("obj1b.u0430", 0x1000000, 0x200000, CRC(5259321f) SHA1(3c70c1147e49f81371d0f60f7108d9718d56faf4) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_WORD("pcm.u0217",    0x000000, 0x100000, CRC(2edc30b5) SHA1(c25d690d633657fc3687636b9070f36bd305ae06) )
	ROM_CONTINUE(                   0x400000, 0x100000 )
	ROM_LOAD32_BYTE("seibu8.bin", 0x800000, 0x080000, CRC(b7bd3703) SHA1(6427a7e6de10d6743d6e64b984a1d1c647f5643a) )

	ROM_REGION( 0x0345, "pals", 0 ) /* pals */
	ROM_LOAD("rm81.u0529.bin", 0x0000, 0x0117, CRC(acd55c8e) SHA1(b965e828fecd61b836aca337637e53d7360d9dc4) ) // AMD PALCE16V8H-15SC/4
	ROM_LOAD("rm82.u0330.bin", 0x0117, 0x0117, CRC(64c71423) SHA1(1da3502bec0c843b7198d1d9ab60f9fd4b110a8e) ) // AMD PALCE16V8H-15SC/4
	ROM_LOAD("rm83.u0331.bin", 0x022e, 0x0117, CRC(6e10d66b) SHA1(995d2a0da680ec19ee253098c91a4780dd8403c6) ) // AMD PALCE16V8H-15SC/4

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region92.u1053", 0x000000, 0x100000, CRC(204d82d0) SHA1(444f4aefa27d8f5d1a2f7f08f826ea84b0ccbd02) )
ROM_END

ROM_START( rdft2a ) /* SPI Cart, Asia (Metrotainment license); SPI PCB is marked "(C)1997 SXX2C ROM SUB8" */
	// The SUB8 board is also capable of having two 23C8100 roms at U0223 and U0219 for PRG instead of the four roms below.
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program, all are 27C040 */
	ROM_LOAD32_BYTE("seibu__1.u0211", 0x000000, 0x80000, CRC(046b3f0e) SHA1(033898f658d6007f891828835734422d4af36321) ) // socket is silkscreened on pcb PRG1
	ROM_LOAD32_BYTE("seibu__2.u0212", 0x000001, 0x80000, CRC(cab55d88) SHA1(246e13880d34b6c7c3f4ab5e18fa8a0547c03d9d) ) // socket is silkscreened on pcb PRG2
	ROM_LOAD32_BYTE("seibu__3.u0221", 0x000002, 0x80000, CRC(83758b0e) SHA1(63adb2d09e7bd7dba47a55b3b579d543dfb553e3) ) // socket is silkscreened on pcb PRG3
	ROM_LOAD32_BYTE("seibu__4.u0220", 0x000003, 0x80000, CRC(084fb5e4) SHA1(588bfe091662b88f02f528181a2f1d9c67c7b280) ) // socket is silkscreened on pcb PRG4

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms - all are 27C512 */
	ROM_LOAD24_BYTE("seibu__5.u0524", 0x000001, 0x10000, CRC(6fdf4cf6) SHA1(7e9d4a49e829dfdc373c0f5acfbe8c7a91ac115b) ) // socket is silkscreened on pcb FIX0
	ROM_LOAD24_BYTE("seibu__6.u0518", 0x000000, 0x10000, CRC(69b7899b) SHA1(d3cacd4ef4d2c95d803403101beb9d4be75fae61) ) // socket is silkscreened on pcb FIX1
	ROM_LOAD24_BYTE("seibu__7.u0514", 0x000002, 0x10000, CRC(99a5fece) SHA1(44ae95d650ed6e00202d3438f5f91a5e52e319cb) ) // socket is silkscreened on pcb FIXP

	ROM_REGION( 0xc00000, "tiles", ROMREGION_ERASEFF ) /* background layer roms - half are MX semiconductor MX23C3210MC, half are some sort of 23C1610 equivalent with no visible manufacturer name */
	ROM_LOAD24_WORD("raiden-f2bg-1d.u0535",   0x000000, 0x400000, CRC(6143f576) SHA1(c034923d0663d9ef24357a03098b8cb81dbab9f8) ) // pads are silkscreened on pcb BG12
	ROM_LOAD24_BYTE("raiden-f2__bg-1p.u0537", 0x000002, 0x200000, CRC(55e64ef7) SHA1(aae991268948d07342ee8ba1b3761bd180aab8ec) ) // pads are silkscreened on pcb BG12P
	ROM_LOAD24_WORD("raiden-f2bg-2d.u0536",   0x600000, 0x400000, CRC(c607a444) SHA1(dc1aa96a42e9394ca6036359670a4ec6f830c96d) ) // pads are silkscreened on pcb BG3
	ROM_LOAD24_BYTE("raiden-f2__bg-2p.u0538", 0x600002, 0x200000, CRC(f0830248) SHA1(6075df96b49e70d2243fef691e096119e7a4d044) ) // pads are silkscreened on pcb BG3P

	ROM_REGION( 0x1200000, "sprites", 0 ) /* sprites - all are paired MX semconductor MX23C3210TC and MX23C1610TC mask roms */
	ROM_LOAD("raiden-f2obj-3.u0434", 0x0000000, 0x400000, CRC(e08f42dc) SHA1(5188d71d4355eaf43ea8893b4cfc4fe80cc24f41) ) // pads are silkscreened on pcb OBJ3
	ROM_LOAD("raiden-f2obj-6.u0433", 0x0400000, 0x200000, CRC(1b6a523c) SHA1(99a420dbc8e22e7832ccda7cec9fa661a2a2687a) ) // pads are silkscreened on pcb OBJ3B
	ROM_LOAD("raiden-f2obj-2.u0431", 0x0600000, 0x400000, CRC(7aeadd8e) SHA1(47103c0579240c5b1add4d0b164eaf76f5fa97f0) ) // pads are silkscreened on pcb OBJ2
	ROM_LOAD("raiden-f2obj-5.u0432", 0x0a00000, 0x200000, CRC(5d790a5d) SHA1(1ed5d4ad4c9a7e505ce35dcc90d184c26ce891dc) ) // pads are silkscreened on pcb OBJ2B
	ROM_LOAD("raiden-f2obj-1.u0429", 0x0c00000, 0x400000, CRC(c2c50f02) SHA1(b81397b5800c6d49f58b7ac7ff6eac56da3c5257) ) // pads are silkscreened on pcb OBJ1
	ROM_LOAD("raiden-f2obj-4.u0430", 0x1000000, 0x200000, CRC(5259321f) SHA1(3c70c1147e49f81371d0f60f7108d9718d56faf4) ) // pads are silkscreened on pcb OBJ1B

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms - sound0 is some sort of 23C1610 equivalent with no visible manufacturer name, sound1 is a 27C040 */
	ROM_LOAD32_WORD("raiden-f2__pcm.u0217", 0x000000, 0x100000, CRC(2edc30b5) SHA1(c25d690d633657fc3687636b9070f36bd305ae06) ) // pads are silkscreened on pcb SOUND0
	ROM_CONTINUE(                           0x400000, 0x100000 )
	ROM_LOAD32_BYTE("seibu__8.u0222",       0x800000, 0x080000, CRC(b7bd3703) SHA1(6427a7e6de10d6743d6e64b984a1d1c647f5643a) ) // socket is silkscreened on pcb SOUND1

	ROM_REGION( 0x0345, "pals", 0 ) /* pals */
	ROM_LOAD("rm81.u0529.bin", 0x0000, 0x0117, CRC(acd55c8e) SHA1(b965e828fecd61b836aca337637e53d7360d9dc4) ) // AMD PALCE16V8H-15SC/4
	ROM_LOAD("rm82.u0330.bin", 0x0117, 0x0117, CRC(64c71423) SHA1(1da3502bec0c843b7198d1d9ab60f9fd4b110a8e) ) // AMD PALCE16V8H-15SC/4
	ROM_LOAD("rm83.u0331.bin", 0x022e, 0x0117, CRC(6e10d66b) SHA1(995d2a0da680ec19ee253098c91a4780dd8403c6) ) // AMD PALCE16V8H-15SC/4

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region22.u1053", 0x000000, 0x100000, CRC(5fee8413) SHA1(6d6a62fa01293b4ba4b349a39820d024add6ea22) )
ROM_END

ROM_START( rdft2aa ) /* SPI Cart, Asia (Dream Island license) */
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("rf2_1.bin", 0x000000, 0x80000, CRC(72198410) SHA1(ca4bc858f6bf247a343b0fdae1d1a3cdabc4a3c3) )
	ROM_LOAD32_BYTE("rf2_2.bin", 0x000001, 0x80000, CRC(ec73a767) SHA1(83f3905afe49401793c0ea0193cb31d3ba1e1739) )
	ROM_LOAD32_BYTE("rf2_3.bin", 0x000002, 0x80000, CRC(e66243b2) SHA1(54e67af37a4586fd1afc79085ed433d599e1bb87) )
	ROM_LOAD32_BYTE("rf2_4.bin", 0x000003, 0x80000, CRC(92b7b73e) SHA1(128649b2a6a0616113bd0f9846fb6cf814ae326d) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_BYTE("rf2_5.bin", 0x000001, 0x10000, CRC(377cac2f) SHA1(f7c9323d79b77f6c8c02ba2c6cdca127d6e5cb5c) )
	ROM_LOAD24_BYTE("rf2_6.bin", 0x000000, 0x10000, CRC(42bd5372) SHA1(c38df85b25070db9640eac541f71c0511bab0c98) )
	ROM_LOAD24_BYTE("rf2_7.bin", 0x000002, 0x10000, CRC(1efaac7e) SHA1(8252af56dcb7a6306dc3422070176778e3c511c2) )

	ROM_REGION( 0xc00000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0535", 0x000000, 0x400000, CRC(6143f576) SHA1(c034923d0663d9ef24357a03098b8cb81dbab9f8) )
	ROM_LOAD24_BYTE("bg-1p.u0537", 0x000002, 0x200000, CRC(55e64ef7) SHA1(aae991268948d07342ee8ba1b3761bd180aab8ec) )
	ROM_LOAD24_WORD("bg-2d.u0536", 0x600000, 0x400000, CRC(c607a444) SHA1(dc1aa96a42e9394ca6036359670a4ec6f830c96d) )
	ROM_LOAD24_BYTE("bg-2p.u0538", 0x600002, 0x200000, CRC(f0830248) SHA1(6075df96b49e70d2243fef691e096119e7a4d044) )

	ROM_REGION( 0x1200000, "sprites", 0 ) /* sprites */
	ROM_LOAD("obj3.u0434",  0x0000000, 0x400000, CRC(e08f42dc) SHA1(5188d71d4355eaf43ea8893b4cfc4fe80cc24f41) )
	ROM_LOAD("obj3b.u0433", 0x0400000, 0x200000, CRC(1b6a523c) SHA1(99a420dbc8e22e7832ccda7cec9fa661a2a2687a) )
	ROM_LOAD("obj2.u0431",  0x0600000, 0x400000, CRC(7aeadd8e) SHA1(47103c0579240c5b1add4d0b164eaf76f5fa97f0) )
	ROM_LOAD("obj2b.u0432", 0x0a00000, 0x200000, CRC(5d790a5d) SHA1(1ed5d4ad4c9a7e505ce35dcc90d184c26ce891dc) )
	ROM_LOAD("obj1.u0429",  0x0c00000, 0x400000, CRC(c2c50f02) SHA1(b81397b5800c6d49f58b7ac7ff6eac56da3c5257) )
	ROM_LOAD("obj1b.u0430", 0x1000000, 0x200000, CRC(5259321f) SHA1(3c70c1147e49f81371d0f60f7108d9718d56faf4) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_WORD("pcm.u0217",    0x000000, 0x100000, CRC(2edc30b5) SHA1(c25d690d633657fc3687636b9070f36bd305ae06) )
	ROM_CONTINUE(                   0x400000, 0x100000 )
	ROM_LOAD32_BYTE("sound1.u0222", 0x800000, 0x080000, CRC(b7bd3703) SHA1(6427a7e6de10d6743d6e64b984a1d1c647f5643a) )

	ROM_REGION( 0x0345, "pals", 0 ) /* pals */
	ROM_LOAD("rm81.u0529.bin", 0x0000, 0x0117, CRC(acd55c8e) SHA1(b965e828fecd61b836aca337637e53d7360d9dc4) ) // AMD PALCE16V8H-15SC/4
	ROM_LOAD("rm82.u0330.bin", 0x0117, 0x0117, CRC(64c71423) SHA1(1da3502bec0c843b7198d1d9ab60f9fd4b110a8e) ) // AMD PALCE16V8H-15SC/4
	ROM_LOAD("rm83.u0331.bin", 0x022e, 0x0117, CRC(6e10d66b) SHA1(995d2a0da680ec19ee253098c91a4780dd8403c6) ) // AMD PALCE16V8H-15SC/4

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region24.u1053", 0x000000, 0x100000, CRC(72a33dc4) SHA1(65a52f576ca4d240418fedd9a4922edcd6c0c8d1) )
ROM_END

ROM_START( rdft2t ) /* SPI Cart, Taiwan */
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("prg0", 0x000000, 0x80000, CRC(7e8c3acc) SHA1(63f4f9f7df7fa028737d9f7dfae96795cde58541) )
	ROM_LOAD32_BYTE("prg1", 0x000001, 0x80000, CRC(22cb5b68) SHA1(35f86ad7771fe9aaac3904ed34a96d0cc10ef21c) )
	ROM_LOAD32_BYTE("prg2", 0x000002, 0x80000, CRC(3eca68dd) SHA1(98378654adf055d72ae685f90e36643c9d6419d7) )
	ROM_LOAD32_BYTE("prg3", 0x000003, 0x80000, CRC(4124daa4) SHA1(42f225c0328df59ffeacc215d37010f825bf507e) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_BYTE("rf2_5.bin", 0x000001, 0x10000, CRC(377cac2f) SHA1(f7c9323d79b77f6c8c02ba2c6cdca127d6e5cb5c) )
	ROM_LOAD24_BYTE("rf2_6.bin", 0x000000, 0x10000, CRC(42bd5372) SHA1(c38df85b25070db9640eac541f71c0511bab0c98) )
	ROM_LOAD24_BYTE("rf2_7.bin", 0x000002, 0x10000, CRC(1efaac7e) SHA1(8252af56dcb7a6306dc3422070176778e3c511c2) )

	ROM_REGION( 0xc00000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0535", 0x000000, 0x400000, CRC(6143f576) SHA1(c034923d0663d9ef24357a03098b8cb81dbab9f8) )
	ROM_LOAD24_BYTE("bg-1p.u0537", 0x000002, 0x200000, CRC(55e64ef7) SHA1(aae991268948d07342ee8ba1b3761bd180aab8ec) )
	ROM_LOAD24_WORD("bg-2d.u0536", 0x600000, 0x400000, CRC(c607a444) SHA1(dc1aa96a42e9394ca6036359670a4ec6f830c96d) )
	ROM_LOAD24_BYTE("bg-2p.u0538", 0x600002, 0x200000, CRC(f0830248) SHA1(6075df96b49e70d2243fef691e096119e7a4d044) )

	ROM_REGION( 0x1200000, "sprites", 0 ) /* sprites */
	ROM_LOAD("obj3.u0434",  0x0000000, 0x400000, CRC(e08f42dc) SHA1(5188d71d4355eaf43ea8893b4cfc4fe80cc24f41) )
	ROM_LOAD("obj3b.u0433", 0x0400000, 0x200000, CRC(1b6a523c) SHA1(99a420dbc8e22e7832ccda7cec9fa661a2a2687a) )
	ROM_LOAD("obj2.u0431",  0x0600000, 0x400000, CRC(7aeadd8e) SHA1(47103c0579240c5b1add4d0b164eaf76f5fa97f0) )
	ROM_LOAD("obj2b.u0432", 0x0a00000, 0x200000, CRC(5d790a5d) SHA1(1ed5d4ad4c9a7e505ce35dcc90d184c26ce891dc) )
	ROM_LOAD("obj1.u0429",  0x0c00000, 0x400000, CRC(c2c50f02) SHA1(b81397b5800c6d49f58b7ac7ff6eac56da3c5257) )
	ROM_LOAD("obj1b.u0430", 0x1000000, 0x200000, CRC(5259321f) SHA1(3c70c1147e49f81371d0f60f7108d9718d56faf4) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_WORD("pcm.u0217",    0x000000, 0x100000, CRC(2edc30b5) SHA1(c25d690d633657fc3687636b9070f36bd305ae06) )
	ROM_CONTINUE(                   0x400000, 0x100000 )
	ROM_LOAD32_BYTE("sound1.u0222", 0x800000, 0x080000, CRC(b7bd3703) SHA1(6427a7e6de10d6743d6e64b984a1d1c647f5643a) )

	ROM_REGION( 0x0345, "pals", 0 ) /* pals */
	ROM_LOAD("rm81.u0529.bin", 0x0000, 0x0117, CRC(acd55c8e) SHA1(b965e828fecd61b836aca337637e53d7360d9dc4) ) // AMD PALCE16V8H-15SC/4
	ROM_LOAD("rm82.u0330.bin", 0x0117, 0x0117, CRC(64c71423) SHA1(1da3502bec0c843b7198d1d9ab60f9fd4b110a8e) ) // AMD PALCE16V8H-15SC/4
	ROM_LOAD("rm83.u0331.bin", 0x022e, 0x0117, CRC(6e10d66b) SHA1(995d2a0da680ec19ee253098c91a4780dd8403c6) ) // AMD PALCE16V8H-15SC/4

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region20.u1053", 0x000000, 0x100000, CRC(f2051161) SHA1(45cbd5fd9ae0ca0c5c3450bca5f6806ddce3c56f) )
ROM_END

ROM_START( rdft2s ) /* SPI Cart, Switzerland; SPI PCB is marked "(C)1997 SXX2C ROM SUB8" */
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program, all are 27C040 */
	ROM_LOAD32_BYTE("seibu__1.u0211", 0x000000, 0x80000, CRC(28b2a185) SHA1(a42adc166992629e96b1ad2fa0859643689fb5c4) ) // socket is silkscreened on pcb PRG1
	ROM_LOAD32_BYTE("seibu__2.u0212", 0x000001, 0x80000, CRC(cab55d88) SHA1(246e13880d34b6c7c3f4ab5e18fa8a0547c03d9d) ) // socket is silkscreened on pcb PRG2
	ROM_LOAD32_BYTE("seibu__3.u0221", 0x000002, 0x80000, CRC(83758b0e) SHA1(63adb2d09e7bd7dba47a55b3b579d543dfb553e3) ) // socket is silkscreened on pcb PRG3
	ROM_LOAD32_BYTE("seibu__4.u0220", 0x000003, 0x80000, CRC(084fb5e4) SHA1(588bfe091662b88f02f528181a2f1d9c67c7b280) ) // socket is silkscreened on pcb PRG4

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms - all are 27C512 */
	ROM_LOAD24_BYTE("seibu__5.u0524", 0x000001, 0x10000, CRC(6fdf4cf6) SHA1(7e9d4a49e829dfdc373c0f5acfbe8c7a91ac115b) ) // socket is silkscreened on pcb FIX0
	ROM_LOAD24_BYTE("seibu__6.u0518", 0x000000, 0x10000, CRC(69b7899b) SHA1(d3cacd4ef4d2c95d803403101beb9d4be75fae61) ) // socket is silkscreened on pcb FIX1
	ROM_LOAD24_BYTE("seibu__7.u0514", 0x000002, 0x10000, CRC(99a5fece) SHA1(44ae95d650ed6e00202d3438f5f91a5e52e319cb) ) // socket is silkscreened on pcb FIXP

	ROM_REGION( 0xc00000, "tiles", ROMREGION_ERASEFF ) /* background layer roms - half are MX semiconductor MX23C3210MC, half are some sort of 23C1610 equivalent with no visible manufacturer name */
	ROM_LOAD24_WORD("raiden-f2bg-1d.u0535",   0x000000, 0x400000, CRC(6143f576) SHA1(c034923d0663d9ef24357a03098b8cb81dbab9f8) ) // pads are silkscreened on pcb BG12
	ROM_LOAD24_BYTE("raiden-f2__bg-1p.u0537", 0x000002, 0x200000, CRC(55e64ef7) SHA1(aae991268948d07342ee8ba1b3761bd180aab8ec) ) // pads are silkscreened on pcb BG12P
	ROM_LOAD24_WORD("raiden-f2bg-2d.u0536",   0x600000, 0x400000, CRC(c607a444) SHA1(dc1aa96a42e9394ca6036359670a4ec6f830c96d) ) // pads are silkscreened on pcb BG3
	ROM_LOAD24_BYTE("raiden-f2__bg-2p.u0538", 0x600002, 0x200000, CRC(f0830248) SHA1(6075df96b49e70d2243fef691e096119e7a4d044) ) // pads are silkscreened on pcb BG3P

	ROM_REGION( 0x1200000, "sprites", 0 ) /* sprites - all are paired MX semconductor MX23C3210TC and MX23C1610TC mask roms */
	ROM_LOAD("raiden-f2obj-3.u0434", 0x0000000, 0x400000, CRC(e08f42dc) SHA1(5188d71d4355eaf43ea8893b4cfc4fe80cc24f41) ) // pads are silkscreened on pcb OBJ3
	ROM_LOAD("raiden-f2obj-6.u0433", 0x0400000, 0x200000, CRC(1b6a523c) SHA1(99a420dbc8e22e7832ccda7cec9fa661a2a2687a) ) // pads are silkscreened on pcb OBJ3B
	ROM_LOAD("raiden-f2obj-2.u0431", 0x0600000, 0x400000, CRC(7aeadd8e) SHA1(47103c0579240c5b1add4d0b164eaf76f5fa97f0) ) // pads are silkscreened on pcb OBJ2
	ROM_LOAD("raiden-f2obj-5.u0432", 0x0a00000, 0x200000, CRC(5d790a5d) SHA1(1ed5d4ad4c9a7e505ce35dcc90d184c26ce891dc) ) // pads are silkscreened on pcb OBJ2B
	ROM_LOAD("raiden-f2obj-1.u0429", 0x0c00000, 0x400000, CRC(c2c50f02) SHA1(b81397b5800c6d49f58b7ac7ff6eac56da3c5257) ) // pads are silkscreened on pcb OBJ1
	ROM_LOAD("raiden-f2obj-4.u0430", 0x1000000, 0x200000, CRC(5259321f) SHA1(3c70c1147e49f81371d0f60f7108d9718d56faf4) ) // pads are silkscreened on pcb OBJ1B

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms - sound0 is some sort of 23C1610 equivalent with no visible manufacturer name, sound1 is a 27C040 */
	ROM_LOAD32_WORD("raiden-f2__pcm.u0217", 0x000000, 0x100000, CRC(2edc30b5) SHA1(c25d690d633657fc3687636b9070f36bd305ae06) ) // pads are silkscreened on pcb SOUND0
	ROM_CONTINUE(                           0x400000, 0x100000 )
	ROM_LOAD32_BYTE("seibu__8.u0222",       0x800000, 0x080000, CRC(b7bd3703) SHA1(6427a7e6de10d6743d6e64b984a1d1c647f5643a) ) // socket is silkscreened on pcb SOUND1

	ROM_REGION( 0x0345, "pals", 0 ) /* pals */
	ROM_LOAD("rm81.u0529.bin", 0x0000, 0x0117, CRC(acd55c8e) SHA1(b965e828fecd61b836aca337637e53d7360d9dc4) ) // AMD PALCE16V8H-15SC/4
	ROM_LOAD("rm82.u0330.bin", 0x0117, 0x0117, CRC(64c71423) SHA1(1da3502bec0c843b7198d1d9ab60f9fd4b110a8e) ) // AMD PALCE16V8H-15SC/4
	ROM_LOAD("rm83.u0331.bin", 0x022e, 0x0117, CRC(6e10d66b) SHA1(995d2a0da680ec19ee253098c91a4780dd8403c6) ) // AMD PALCE16V8H-15SC/4

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region9c.u1053", 0x000000, 0x100000, CRC(d73d640c) SHA1(61a99af2a153de9d53e28872a2493e2ba797a325) )
ROM_END


ROM_START( rfjet ) /* SPI Cart, Europe */
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("prg0.u0211", 0x000000, 0x80000, CRC(e5a3b304) SHA1(f7285f9c69c589fcc71082dc0b9225fdccec855f) )
	ROM_LOAD32_BYTE("prg1.u0212", 0x000001, 0x80000, CRC(395e6da7) SHA1(736f777cb1b6bf5541832b8ea89594738ca6d829) )
	ROM_LOAD32_BYTE("prg2.u0221", 0x000002, 0x80000, CRC(82f7a57e) SHA1(5300015e25d5f2f82eda3ed54bc105d645518498) )
	ROM_LOAD32_BYTE("prg3.u0220", 0x000003, 0x80000, CRC(cbdf100d) SHA1(c9efd11103429f7f36c1652cadb5384d925cb767) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_BYTE("fix0.u0524", 0x000001, 0x10000, CRC(8bc080be) SHA1(ad296fb98242c963072346a8de289e704b445ad4) )
	ROM_LOAD24_BYTE("fix1.u0518", 0x000000, 0x10000, CRC(bded85e7) SHA1(ccb8c438ce6b9a742e3ab15be970b1e636783626) )
	ROM_LOAD24_BYTE("fixp.u0514", 0x000002, 0x10000, CRC(015d0748) SHA1(b1e8eaeba63a7914f1dc27d7e3ca5d0b6db202ed) )

	ROM_REGION( 0x900000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0543", 0x000000, 0x400000, CRC(edfd96da) SHA1(4813267f104619f569e5777e75b75304321abb49) )
	ROM_LOAD24_BYTE("bg-1p.u0544", 0x000002, 0x200000, CRC(a4cc4631) SHA1(cc1c4f4de8a078ca774f5a328a9a58291949b1fb) )
	ROM_LOAD24_WORD("bg-2d.u0545", 0x600000, 0x200000, CRC(731fbb59) SHA1(13cd29ec4d4c73582c5fb363218e737886826e5f) )
	ROM_LOAD24_BYTE("bg-2p.u0546", 0x600002, 0x100000, CRC(03652c25) SHA1(c0d77285111bc84e008362981ac02a246678ed0a) )

	ROM_REGION( 0x1800000, "sprites", 0 ) /* sprites */
	ROM_LOAD("obj-1.u0442", 0x0000000, 0x800000, CRC(58a59896) SHA1(edeaaa69987bd5d08c47ed9bf47a3901e2dcc892) )
	ROM_LOAD("obj-2.u0443", 0x0800000, 0x800000, CRC(a121d1e3) SHA1(1851ae81f2ae9d3404aadd9fbc0ed7f9230290b9) )
	ROM_LOAD("obj-3.u0444", 0x1000000, 0x800000, CRC(bc2c0c63) SHA1(c8d395722f7012c3be366a0fc9b224c537afabae) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_WORD("pcm-d.u0227",  0x000000, 0x100000, CRC(8ee3ff45) SHA1(2801b23495866c91c8f8bebd37d5fcae7a625838) )
	ROM_CONTINUE(                   0x400000, 0x100000 )
	ROM_LOAD32_BYTE("sound1.u0222", 0x800000, 0x080000, CRC(d4fc3da1) SHA1(a03bd97e36a21d27a834b9691b27a7eb7ac51ff2) )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region80.u1053", 0x000000, 0x100000, CRC(e2adaff5) SHA1(9297afaf78209724515d8f78de8cee7bc7cb796b) )
ROM_END

ROM_START( rfjetj ) /* SPI Cart, Japan */
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("prg0.bin", 0x000000, 0x80000, CRC(d82fb71f) SHA1(63a458fd007c353f4fae54a4882f5c565fe1efa4) )
	ROM_LOAD32_BYTE("prg1.bin", 0x000001, 0x80000, CRC(7e21c669) SHA1(731852e5925dccc9d0d1ae4bcafa238f157f4079) )
	ROM_LOAD32_BYTE("prg2.bin", 0x000002, 0x80000, CRC(2f402d55) SHA1(d0d852239abb6f4d73e263de5544fc0893e7a7ab) )
	ROM_LOAD32_BYTE("prg3.bin", 0x000003, 0x80000, CRC(d619e2ad) SHA1(9dbff1babf62c3c5478a84d2a82a428de5949154) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_BYTE("fix0.u0524", 0x000001, 0x10000, CRC(8bc080be) SHA1(ad296fb98242c963072346a8de289e704b445ad4) )
	ROM_LOAD24_BYTE("fix1.u0518", 0x000000, 0x10000, CRC(bded85e7) SHA1(ccb8c438ce6b9a742e3ab15be970b1e636783626) )
	ROM_LOAD24_BYTE("fixp.u0514", 0x000002, 0x10000, CRC(015d0748) SHA1(b1e8eaeba63a7914f1dc27d7e3ca5d0b6db202ed) )

	ROM_REGION( 0x900000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0543", 0x000000, 0x400000, CRC(edfd96da) SHA1(4813267f104619f569e5777e75b75304321abb49) )
	ROM_LOAD24_BYTE("bg-1p.u0544", 0x000002, 0x200000, CRC(a4cc4631) SHA1(cc1c4f4de8a078ca774f5a328a9a58291949b1fb) )
	ROM_LOAD24_WORD("bg-2d.u0545", 0x600000, 0x200000, CRC(731fbb59) SHA1(13cd29ec4d4c73582c5fb363218e737886826e5f) )
	ROM_LOAD24_BYTE("bg-2p.u0546", 0x600002, 0x100000, CRC(03652c25) SHA1(c0d77285111bc84e008362981ac02a246678ed0a) )

	ROM_REGION( 0x1800000, "sprites", 0 ) /* sprites */
	ROM_LOAD("obj-1.u0442", 0x0000000, 0x800000, CRC(58a59896) SHA1(edeaaa69987bd5d08c47ed9bf47a3901e2dcc892) )
	ROM_LOAD("obj-2.u0443", 0x0800000, 0x800000, CRC(a121d1e3) SHA1(1851ae81f2ae9d3404aadd9fbc0ed7f9230290b9) )
	ROM_LOAD("obj-3.u0444", 0x1000000, 0x800000, CRC(bc2c0c63) SHA1(c8d395722f7012c3be366a0fc9b224c537afabae) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_WORD("pcm-d.u0227",  0x000000, 0x100000, CRC(8ee3ff45) SHA1(2801b23495866c91c8f8bebd37d5fcae7a625838) )
	ROM_CONTINUE(                   0x400000, 0x100000 )
	ROM_LOAD32_BYTE("sound1.u0222", 0x800000, 0x080000, CRC(d4fc3da1) SHA1(a03bd97e36a21d27a834b9691b27a7eb7ac51ff2) )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region01.u1053", 0x000000, 0x100000, CRC(7ae7ab76) SHA1(a2b196f470bf64af94002fc4e2640fadad00418f) )
ROM_END

ROM_START( rfjetu ) /* SPI Cart, US */
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("prg0u.u0211", 0x000000, 0x80000, CRC(15ac2040) SHA1(7309a9dd9c91fef0e761dcf8639f421ce7abc97f) )
	ROM_LOAD32_BYTE("prg1.u0212",  0x000001, 0x80000, CRC(395e6da7) SHA1(736f777cb1b6bf5541832b8ea89594738ca6d829) )
	ROM_LOAD32_BYTE("prg2.u0221",  0x000002, 0x80000, CRC(82f7a57e) SHA1(5300015e25d5f2f82eda3ed54bc105d645518498) )
	ROM_LOAD32_BYTE("prg3.u0220",  0x000003, 0x80000, CRC(cbdf100d) SHA1(c9efd11103429f7f36c1652cadb5384d925cb767) )

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_BYTE("fix0.u0524", 0x000001, 0x10000, CRC(8bc080be) SHA1(ad296fb98242c963072346a8de289e704b445ad4) )
	ROM_LOAD24_BYTE("fix1.u0518", 0x000000, 0x10000, CRC(bded85e7) SHA1(ccb8c438ce6b9a742e3ab15be970b1e636783626) )
	ROM_LOAD24_BYTE("fixp.u0514", 0x000002, 0x10000, CRC(015d0748) SHA1(b1e8eaeba63a7914f1dc27d7e3ca5d0b6db202ed) )

	ROM_REGION( 0x900000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0543", 0x000000, 0x400000, CRC(edfd96da) SHA1(4813267f104619f569e5777e75b75304321abb49) )
	ROM_LOAD24_BYTE("bg-1p.u0544", 0x000002, 0x200000, CRC(a4cc4631) SHA1(cc1c4f4de8a078ca774f5a328a9a58291949b1fb) )
	ROM_LOAD24_WORD("bg-2d.u0545", 0x600000, 0x200000, CRC(731fbb59) SHA1(13cd29ec4d4c73582c5fb363218e737886826e5f) )
	ROM_LOAD24_BYTE("bg-2p.u0546", 0x600002, 0x100000, CRC(03652c25) SHA1(c0d77285111bc84e008362981ac02a246678ed0a) )

	ROM_REGION( 0x1800000, "sprites", 0 ) /* sprites */
	ROM_LOAD("obj-1.u0442", 0x0000000, 0x800000, CRC(58a59896) SHA1(edeaaa69987bd5d08c47ed9bf47a3901e2dcc892) )
	ROM_LOAD("obj-2.u0443", 0x0800000, 0x800000, CRC(a121d1e3) SHA1(1851ae81f2ae9d3404aadd9fbc0ed7f9230290b9) )
	ROM_LOAD("obj-3.u0444", 0x1000000, 0x800000, CRC(bc2c0c63) SHA1(c8d395722f7012c3be366a0fc9b224c537afabae) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_WORD("pcm-d.u0227",  0x000000, 0x100000, CRC(8ee3ff45) SHA1(2801b23495866c91c8f8bebd37d5fcae7a625838) )
	ROM_CONTINUE(                   0x400000, 0x100000 )
	ROM_LOAD32_BYTE("sound1.u0222", 0x800000, 0x080000, CRC(d4fc3da1) SHA1(a03bd97e36a21d27a834b9691b27a7eb7ac51ff2) )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region10.u1053", 0x000000, 0x100000, CRC(4319d998) SHA1(a064ce647453a9b3bccf7f1d6d0d52b5a72e09dd) )
ROM_END

ROM_START( rfjeta ) /* SPI Cart, Asia */
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("prg0a.u0211", 0x000000, 0x80000, CRC(3418d4f5) SHA1(f8766d7b3708a196de417ee757787220b2a9ced1) )
	ROM_LOAD32_BYTE("prg1.u0212",  0x000001, 0x80000, CRC(395e6da7) SHA1(736f777cb1b6bf5541832b8ea89594738ca6d829) ) // sldh
	ROM_LOAD32_BYTE("prg2.u0221",  0x000002, 0x80000, CRC(82f7a57e) SHA1(5300015e25d5f2f82eda3ed54bc105d645518498) ) // sldh
	ROM_LOAD32_BYTE("prg3.u0220",  0x000003, 0x80000, CRC(cbdf100d) SHA1(c9efd11103429f7f36c1652cadb5384d925cb767) ) // sldh

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_BYTE("fix0.u0524", 0x000001, 0x10000, CRC(8bc080be) SHA1(ad296fb98242c963072346a8de289e704b445ad4) )
	ROM_LOAD24_BYTE("fix1.u0518", 0x000000, 0x10000, CRC(bded85e7) SHA1(ccb8c438ce6b9a742e3ab15be970b1e636783626) )
	ROM_LOAD24_BYTE("fixp.u0514", 0x000002, 0x10000, CRC(015d0748) SHA1(b1e8eaeba63a7914f1dc27d7e3ca5d0b6db202ed) )

	ROM_REGION( 0x900000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0543", 0x000000, 0x400000, CRC(edfd96da) SHA1(4813267f104619f569e5777e75b75304321abb49) )
	ROM_LOAD24_BYTE("bg-1p.u0544", 0x000002, 0x200000, CRC(a4cc4631) SHA1(cc1c4f4de8a078ca774f5a328a9a58291949b1fb) )
	ROM_LOAD24_WORD("bg-2d.u0545", 0x600000, 0x200000, CRC(731fbb59) SHA1(13cd29ec4d4c73582c5fb363218e737886826e5f) )
	ROM_LOAD24_BYTE("bg-2p.u0546", 0x600002, 0x100000, CRC(03652c25) SHA1(c0d77285111bc84e008362981ac02a246678ed0a) )

	ROM_REGION( 0x1800000, "sprites", 0 ) /* sprites */
	ROM_LOAD("obj-1.u0442", 0x0000000, 0x800000, CRC(58a59896) SHA1(edeaaa69987bd5d08c47ed9bf47a3901e2dcc892) )
	ROM_LOAD("obj-2.u0443", 0x0800000, 0x800000, CRC(a121d1e3) SHA1(1851ae81f2ae9d3404aadd9fbc0ed7f9230290b9) )
	ROM_LOAD("obj-3.u0444", 0x1000000, 0x800000, CRC(bc2c0c63) SHA1(c8d395722f7012c3be366a0fc9b224c537afabae) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_WORD("pcm-d.u0227",  0x000000, 0x100000, CRC(8ee3ff45) SHA1(2801b23495866c91c8f8bebd37d5fcae7a625838) )
	ROM_CONTINUE(                   0x400000, 0x100000 )
	ROM_LOAD32_BYTE("sound1.u0222", 0x800000, 0x080000, CRC(d4fc3da1) SHA1(a03bd97e36a21d27a834b9691b27a7eb7ac51ff2) )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region24.u1053", 0x000000, 0x100000, CRC(72a33dc4) SHA1(65a52f576ca4d240418fedd9a4922edcd6c0c8d1) )
ROM_END

ROM_START( rfjett ) /* SPI Cart, Taiwan */
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE( "prg0.u0211",   0x000000, 0x080000, CRC(a4734579) SHA1(dfbd8e2a3178c7cfd7bd3698999f14bc80f5212f) ) // sldh
	ROM_LOAD32_BYTE( "prg1.u0212",   0x000001, 0x080000, CRC(5e4ad3a4) SHA1(ff66e16f48978b88b298c78e21309208ccb3ff15) ) // sldh
	ROM_LOAD32_BYTE( "prg2.u0221",   0x000002, 0x080000, CRC(21c9942e) SHA1(ededa05a4b5dae2dec5c4409f22e9a66d2c8e98e) ) // sldh
	ROM_LOAD32_BYTE( "prg3.u0220",   0x000003, 0x080000, CRC(ea3657f4) SHA1(2291e31243af7d2e79ae727d9b5484e8d49cc7d9) ) // sldh

	ROM_REGION( 0x40000, "audiocpu", ROMREGION_ERASE00 ) /* 256K RAM, ROM from Z80 point-of-view */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_BYTE("fix0.u0524", 0x000001, 0x10000, CRC(8bc080be) SHA1(ad296fb98242c963072346a8de289e704b445ad4) )
	ROM_LOAD24_BYTE("fix1.u0518", 0x000000, 0x10000, CRC(bded85e7) SHA1(ccb8c438ce6b9a742e3ab15be970b1e636783626) )
	ROM_LOAD24_BYTE("fixp.u0514", 0x000002, 0x10000, CRC(015d0748) SHA1(b1e8eaeba63a7914f1dc27d7e3ca5d0b6db202ed) )

	ROM_REGION( 0x900000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0543", 0x000000, 0x400000, CRC(edfd96da) SHA1(4813267f104619f569e5777e75b75304321abb49) )
	ROM_LOAD24_BYTE("bg-1p.u0544", 0x000002, 0x200000, CRC(a4cc4631) SHA1(cc1c4f4de8a078ca774f5a328a9a58291949b1fb) )
	ROM_LOAD24_WORD("bg-2d.u0545", 0x600000, 0x200000, CRC(731fbb59) SHA1(13cd29ec4d4c73582c5fb363218e737886826e5f) )
	ROM_LOAD24_BYTE("bg-2p.u0546", 0x600002, 0x100000, CRC(03652c25) SHA1(c0d77285111bc84e008362981ac02a246678ed0a) )

	ROM_REGION( 0x1800000, "sprites", 0 ) /* sprites */
	ROM_LOAD("obj-1.u0442", 0x0000000, 0x800000, CRC(58a59896) SHA1(edeaaa69987bd5d08c47ed9bf47a3901e2dcc892) )
	ROM_LOAD("obj-2.u0443", 0x0800000, 0x800000, CRC(a121d1e3) SHA1(1851ae81f2ae9d3404aadd9fbc0ed7f9230290b9) )
	ROM_LOAD("obj-3.u0444", 0x1000000, 0x800000, CRC(bc2c0c63) SHA1(c8d395722f7012c3be366a0fc9b224c537afabae) )

	ROM_REGION32_LE( 0xa00000, "sound01", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD32_WORD("pcm-d.u0227",  0x000000, 0x100000, CRC(8ee3ff45) SHA1(2801b23495866c91c8f8bebd37d5fcae7a625838) )
	ROM_CONTINUE(                   0x400000, 0x100000 )
	ROM_LOAD32_BYTE("sound1.u0222", 0x800000, 0x080000, CRC(d4fc3da1) SHA1(a03bd97e36a21d27a834b9691b27a7eb7ac51ff2) )

	ROM_REGION( 0x100000, "soundflash1", 0 ) /* on SPI motherboard */
	ROM_LOAD("flash0_blank_region20.u1053", 0x000000, 0x100000, CRC(f2051161) SHA1(45cbd5fd9ae0ca0c5c3450bca5f6806ddce3c56f) )
ROM_END


/*****************************************************************************/
/* SXX2E/F/G games */

ROM_START( rdfts ) /* Single board version SXX2E Ver3.0 */
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("seibu_1.u0259",        0x000000, 0x080000, CRC(e278dddd) SHA1(fe54a0d0f9e8596268f7f37e85d71c5c2d8b2846) ) // socket is silkscreened on pcb PRG0
	ROM_LOAD32_BYTE("raiden-f_prg2.u0258",  0x000001, 0x080000, CRC(58ccb10c) SHA1(0cce4057bfada78121d9586574b98d46cdd7dd46) ) // socket is silkscreened on pcb PRG1
	ROM_LOAD32_WORD("raiden-f_prg34.u0262", 0x000002, 0x100000, CRC(63f01d17) SHA1(74dbd0417b974583da87fc6c7a081b03fd4e16b8) ) // socket is silkscreened on pcb PRG23

	ROM_REGION( 0x40000, "audiocpu", 0 ) /* 256K ROM for the Z80 */
	ROM_LOAD("seibu_zprg.u1139", 0x000000, 0x20000, CRC(c1fda3e8) SHA1(c1d3a7ba0601a80534ec32249de71d33a828a162) ) // pads are silkscreened ZPRG

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_WORD("raiden-f_fix.u0535", 0x000000, 0x20000, CRC(2be2936b) SHA1(9e719f7328a52af220b6f084c1e0990ca6e2d533) ) // socket is silkscreened on pcb FIX01
	ROM_LOAD24_BYTE("seibu_fix2.u0528",   0x000002, 0x10000, CRC(4d87e1ea) SHA1(3230e9b643fad773e61ab8ce09c0cd7d4d0558e3) ) // socket is silkscreened on pcb FIX2

	ROM_REGION( 0x600000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("gun_dogs_bg1-d.u0526", 0x000000, 0x200000, CRC(6a68054c) SHA1(5cbfc4ac90045f1401c2dda7a51936558c9de07e) ) // pads are silkscreened on pcb BG12
	ROM_LOAD24_BYTE("gun_dogs_bg1-p.u0531", 0x000002, 0x100000, CRC(3400794a) SHA1(719808f7442bac612cefd7b7fffcd665e6337ad0) ) // pads are silkscreened on pcb BG12P
	ROM_LOAD24_WORD("gun_dogs_bg2-d.u0534", 0x300000, 0x200000, CRC(61cd2991) SHA1(bb608e3948bf9ea35b5e1615d2ba6858d029dcbe) ) // pads are silkscreened on pcb BG3
	ROM_LOAD24_BYTE("gun_dogs_bg2-p.u0530", 0x300002, 0x100000, CRC(502d5799) SHA1(c3a0e1a4f5a7b35572ae1ff31315da4ed08aa2fe) ) // pads are silkscreened on pcb BG3P

	ROM_REGION( 0xc00000, "sprites", 0 ) /* sprites */
	ROM_LOAD("gun_dogs_obj-1.u0322", 0x000000, 0x400000, CRC(59d86c99) SHA1(d3c9241e7b51fe21f8351051b063f91dc69bf905) ) // pads are silkscreened on pcb OBJ1
	ROM_LOAD("gun_dogs_obj-2.u0324", 0x400000, 0x400000, CRC(1ceb0b6f) SHA1(97225a9b3e7be18080aa52f6570af2cce8f25c06) ) // pads are silkscreened on pcb OBJ2
	ROM_LOAD("gun_dogs_obj-3.u0323", 0x800000, 0x400000, CRC(36e93234) SHA1(51917a80b7da5c32a9434a1076fc2916d62e6a3e) ) // pads are silkscreened on pcb OBJ3

	ROM_REGION( 0x200000, "ymf", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD("raiden-f_pcm2.u0975", 0x000000, 0x200000, CRC(3f8d4a48) SHA1(30664a2908daaeaee58f7e157516b522c952e48d) ) // pads are silkscreened SOUND0
	/* SOUND1 socket is unpopulated */
ROM_END


ROM_START( rdft2us ) /* Single board version SXX2F */
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("prg0.u0259", 0x000000, 0x80000, CRC(ff3eeec1) SHA1(88c1741e4936db9a5b13e562061b0f1cc6fa6b36) )
	ROM_LOAD32_BYTE("prg1.u0258", 0x000001, 0x80000, CRC(e2cf77d6) SHA1(173cc0e304c9dadea4ed0812ebb64c6c83549912) )
	ROM_LOAD32_BYTE("prg2.u0265", 0x000002, 0x80000, CRC(cae87e1f) SHA1(e460aad693eb2702ae11f758b11d37f852d00790) )
	ROM_LOAD32_BYTE("prg3.u0264", 0x000003, 0x80000, CRC(83f4fb5f) SHA1(73f58daa1aae0c4978db409cedd736fb49b15f1c) )

	ROM_REGION( 0x40000, "audiocpu", 0 ) /* 256K ROM for the Z80 */
	ROM_LOAD("zprg.u091", 0x000000, 0x20000, CRC(cc543c4f) SHA1(6e5c93fd3d21c594571b071d4a830211e1f162b2) )

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_BYTE("fix0.u0524", 0x000001, 0x10000, CRC(6fdf4cf6) SHA1(7e9d4a49e829dfdc373c0f5acfbe8c7a91ac115b) )
	ROM_LOAD24_BYTE("fix1.u0518", 0x000000, 0x10000, CRC(69b7899b) SHA1(d3cacd4ef4d2c95d803403101beb9d4be75fae61) )
	ROM_LOAD24_BYTE("fixp.u0514", 0x000002, 0x10000, CRC(99a5fece) SHA1(44ae95d650ed6e00202d3438f5f91a5e52e319cb) )

	ROM_REGION( 0xc00000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0535", 0x000000, 0x400000, CRC(6143f576) SHA1(c034923d0663d9ef24357a03098b8cb81dbab9f8) )
	ROM_LOAD24_BYTE("bg-1p.u0537", 0x000002, 0x200000, CRC(55e64ef7) SHA1(aae991268948d07342ee8ba1b3761bd180aab8ec) )
	ROM_LOAD24_WORD("bg-2d.u0536", 0x600000, 0x400000, CRC(c607a444) SHA1(dc1aa96a42e9394ca6036359670a4ec6f830c96d) )
	ROM_LOAD24_BYTE("bg-2p.u0538", 0x600002, 0x200000, CRC(f0830248) SHA1(6075df96b49e70d2243fef691e096119e7a4d044) )

	ROM_REGION( 0x1200000, "sprites", 0 ) /* sprites */
	ROM_LOAD("obj3.u075",  0x0000000, 0x400000, CRC(e08f42dc) SHA1(5188d71d4355eaf43ea8893b4cfc4fe80cc24f41) )
	ROM_LOAD("obj3b.u078", 0x0400000, 0x200000, CRC(1b6a523c) SHA1(99a420dbc8e22e7832ccda7cec9fa661a2a2687a) )
	ROM_LOAD("obj2.u074",  0x0600000, 0x400000, CRC(7aeadd8e) SHA1(47103c0579240c5b1add4d0b164eaf76f5fa97f0) )
	ROM_LOAD("obj2b.u077", 0x0a00000, 0x200000, CRC(5d790a5d) SHA1(1ed5d4ad4c9a7e505ce35dcc90d184c26ce891dc) )
	ROM_LOAD("obj1.u073",  0x0c00000, 0x400000, CRC(c2c50f02) SHA1(b81397b5800c6d49f58b7ac7ff6eac56da3c5257) )
	ROM_LOAD("obj1b.u076", 0x1000000, 0x200000, CRC(5259321f) SHA1(3c70c1147e49f81371d0f60f7108d9718d56faf4) )

	ROM_REGION( 0x280000, "ymf", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD("pcm.u0103",    0x000000, 0x200000, CRC(2edc30b5) SHA1(c25d690d633657fc3687636b9070f36bd305ae06) )
	ROM_LOAD("sound1.u0107", 0x200000, 0x080000, CRC(20384b0e) SHA1(9c5d725418543df740f9145974ed6ffbbabee1d0) ) /* Different sound1 than SPI carts */
ROM_END


ROM_START( rfjets ) /* Single board version SXX2G */
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("rfj-06.u0259", 0x000000, 0x80000, CRC(c835aa7a) SHA1(291eada97ceb907dfea15688ce6055e63b3aa675) ) /* PRG0 */
	ROM_LOAD32_BYTE("rfj-07.u0258", 0x000001, 0x80000, CRC(3b6ca1ca) SHA1(9db019c0ddecfb58e2be5c345d78352f700035bf) ) /* PRG1 */
	ROM_LOAD32_BYTE("rfj-08.u0265", 0x000002, 0x80000, CRC(1f5dd06c) SHA1(6f5a8c9035971a470212cd0a89b94181011602c3) ) /* PRG2 */
	ROM_LOAD32_BYTE("rfj-09.u0264", 0x000003, 0x80000, CRC(cc71c402) SHA1(b040e600744e7b3f52de0fa852ce3ae08ae49985) ) /* PRG3 */

	ROM_REGION( 0x40000, "audiocpu", 0 ) /* 256K ROM for the Z80 */
	ROM_LOAD("rfj-05.u091", 0x000000, 0x40000, CRC(a55e8799) SHA1(5d4ca9ae920ab54e23ee3b1b33db72711e744516) ) /* ZPRG */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_BYTE("rfj-01.u0524", 0x000001, 0x10000, CRC(8bc080be) SHA1(ad296fb98242c963072346a8de289e704b445ad4) ) /* FIX0 */
	ROM_LOAD24_BYTE("rfj-02.u0518", 0x000000, 0x10000, CRC(bded85e7) SHA1(ccb8c438ce6b9a742e3ab15be970b1e636783626) ) /* FIX1 */
	ROM_LOAD24_BYTE("rfj-03.u0514", 0x000002, 0x10000, CRC(015d0748) SHA1(b1e8eaeba63a7914f1dc27d7e3ca5d0b6db202ed) ) /* FIXP */

	ROM_REGION( 0x900000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0535", 0x000000, 0x400000, CRC(edfd96da) SHA1(4813267f104619f569e5777e75b75304321abb49) )
	ROM_LOAD24_BYTE("bg-1p.u0537", 0x000002, 0x200000, CRC(a4cc4631) SHA1(cc1c4f4de8a078ca774f5a328a9a58291949b1fb) )
	ROM_LOAD24_WORD("bg-2d.u0536", 0x600000, 0x200000, CRC(731fbb59) SHA1(13cd29ec4d4c73582c5fb363218e737886826e5f) )
	ROM_LOAD24_BYTE("bg-2p.u0545", 0x600002, 0x100000, CRC(03652c25) SHA1(c0d77285111bc84e008362981ac02a246678ed0a) )

	ROM_REGION( 0x1800000, "sprites", 0 ) /* sprites */
	ROM_LOAD("obj-1.u073", 0x0000000, 0x800000, CRC(58a59896) SHA1(edeaaa69987bd5d08c47ed9bf47a3901e2dcc892) )
	ROM_LOAD("obj-2.u074", 0x0800000, 0x800000, CRC(a121d1e3) SHA1(1851ae81f2ae9d3404aadd9fbc0ed7f9230290b9) )
	ROM_LOAD("obj-3.u075", 0x1000000, 0x800000, CRC(bc2c0c63) SHA1(c8d395722f7012c3be366a0fc9b224c537afabae) )

	ROM_REGION( 0x280000, "ymf", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD("pcm-d.u0103",  0x000000, 0x200000, CRC(8ee3ff45) SHA1(2801b23495866c91c8f8bebd37d5fcae7a625838) )
	ROM_LOAD("rfj-04.u0107", 0x200000, 0x080000, CRC(c050da03) SHA1(1002dac51a3a4932c4f0074c1f3d97a597d98755) ) /* SOUND1 */

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "st93c46.bin", 0x0000, 0x0080, CRC(8fe8063b) SHA1(afb0141580e1b2bd149092a9cc9e8b4072b1ef10) )
ROM_END

/* Notes on rfjetsa:

 - Will initialize the EEPROM on 1st boot and continue (rfjets requires it to be done manually in testmode)
 - Default game cost 2 credits for Solo & 4 credits for Dual (rfjets is 1 credit for Solo & 2 credits for Dual)
 - Has a Parental Advisory warning screen for acceptance in the US arcade market
 - Adds Sound Test and EEPROM Test to the Test Mode menu
 - Misc. debug strings and bugs (see MT 5211)
*/
ROM_START( rfjetsa ) /* Single board version SXX2G */
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("rfj-06.u0259", 0x000000, 0x80000, CRC(b0c8d47e) SHA1(1dde30d25f9e8eaa301343ae1d272b5c0044bc1f) ) /* PRG0 */ // sldh
	ROM_LOAD32_BYTE("rfj-07.u0258", 0x000001, 0x80000, CRC(17189b39) SHA1(6471170ae770d762e15f1503ef9a6832c202da6c) ) /* PRG1 */ // sldh
	ROM_LOAD32_BYTE("rfj-08.u0265", 0x000002, 0x80000, CRC(ab6d724b) SHA1(ef7e42b1bf649a354fe22b0edd00475ced4151be) ) /* PRG2 */ // sldh
	ROM_LOAD32_BYTE("rfj-09.u0264", 0x000003, 0x80000, CRC(b119a67c) SHA1(4fa7dd0e86a3f7c6efa6ae9cf72991b652c877b9) ) /* PRG3 */ // sldh

	ROM_REGION( 0x40000, "audiocpu", 0 ) /* 256K ROM for the Z80 */
	ROM_LOAD("rfj-05.u091", 0x000000, 0x40000, CRC(a55e8799) SHA1(5d4ca9ae920ab54e23ee3b1b33db72711e744516) ) /* ZPRG */

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_BYTE("rfj-01.u0524", 0x000001, 0x10000, CRC(8bc080be) SHA1(ad296fb98242c963072346a8de289e704b445ad4) ) /* FIX0 */
	ROM_LOAD24_BYTE("rfj-02.u0518", 0x000000, 0x10000, CRC(bded85e7) SHA1(ccb8c438ce6b9a742e3ab15be970b1e636783626) ) /* FIX1 */
	ROM_LOAD24_BYTE("rfj-03.u0514", 0x000002, 0x10000, CRC(015d0748) SHA1(b1e8eaeba63a7914f1dc27d7e3ca5d0b6db202ed) ) /* FIXP */

	ROM_REGION( 0x900000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0535", 0x000000, 0x400000, CRC(edfd96da) SHA1(4813267f104619f569e5777e75b75304321abb49) )
	ROM_LOAD24_BYTE("bg-1p.u0537", 0x000002, 0x200000, CRC(a4cc4631) SHA1(cc1c4f4de8a078ca774f5a328a9a58291949b1fb) )
	ROM_LOAD24_WORD("bg-2d.u0536", 0x600000, 0x200000, CRC(731fbb59) SHA1(13cd29ec4d4c73582c5fb363218e737886826e5f) )
	ROM_LOAD24_BYTE("bg-2p.u0545", 0x600002, 0x100000, CRC(03652c25) SHA1(c0d77285111bc84e008362981ac02a246678ed0a) )

	ROM_REGION( 0x1800000, "sprites", 0 ) /* sprites */
	ROM_LOAD("obj-1.u073", 0x0000000, 0x800000, CRC(58a59896) SHA1(edeaaa69987bd5d08c47ed9bf47a3901e2dcc892) )
	ROM_LOAD("obj-2.u074", 0x0800000, 0x800000, CRC(a121d1e3) SHA1(1851ae81f2ae9d3404aadd9fbc0ed7f9230290b9) )
	ROM_LOAD("obj-3.u075", 0x1000000, 0x800000, CRC(bc2c0c63) SHA1(c8d395722f7012c3be366a0fc9b224c537afabae) )

	ROM_REGION( 0x280000, "ymf", ROMREGION_ERASE00 ) /* sound roms */
	ROM_LOAD("pcm-d.u0103",  0x000000, 0x200000, CRC(8ee3ff45) SHA1(2801b23495866c91c8f8bebd37d5fcae7a625838) )
	ROM_LOAD("rfj-04.u0107", 0x200000, 0x080000, CRC(c050da03) SHA1(1002dac51a3a4932c4f0074c1f3d97a597d98755) ) /* SOUND1 */
ROM_END


/*****************************************************************************/
/* SYS386 games */

ROM_START( rdft22kc ) /* SYS386I */
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_WORD("prg0-1.267", 0x000000, 0x100000, CRC(0d7d6eb8) SHA1(3a71e1e0ba5bb500dc026debbb6189723c0c2890) )
	ROM_LOAD32_WORD("prg2-3.266", 0x000002, 0x100000, CRC(ead53e69) SHA1(b0e2e06f403317054ecb48d2747034424245f129) )

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_BYTE("fix0.524", 0x000001, 0x10000, CRC(ed11d043) SHA1(fd3a5a33aa4d795941d64c0d23f9d6f8222843e3) )
	ROM_LOAD24_BYTE("fix1.518", 0x000000, 0x10000, CRC(7036d70a) SHA1(3535b52c0fa1a1158cacc041f8aba2b9a1b43af5) )
	ROM_LOAD24_BYTE("fix2.514", 0x000002, 0x10000, CRC(29b465da) SHA1(644454ab5e0dc1028e9512f85adfe5d8adb757de) )

	ROM_REGION( 0xc00000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("bg-1d.535", 0x000000, 0x400000, CRC(6143f576) SHA1(c034923d0663d9ef24357a03098b8cb81dbab9f8) )
	ROM_LOAD24_BYTE("bg-1p.544", 0x000002, 0x200000, CRC(55e64ef7) SHA1(aae991268948d07342ee8ba1b3761bd180aab8ec) )
	ROM_LOAD24_WORD("bg-2d.536", 0x600000, 0x400000, CRC(c607a444) SHA1(dc1aa96a42e9394ca6036359670a4ec6f830c96d) )
	ROM_LOAD24_BYTE("bg-2p.545", 0x600002, 0x200000, CRC(f0830248) SHA1(6075df96b49e70d2243fef691e096119e7a4d044) )

	ROM_REGION( 0x1200000, "sprites", 0 ) /* sprites */
	ROM_LOAD("obj3.075",  0x0000000, 0x400000, CRC(e08f42dc) SHA1(5188d71d4355eaf43ea8893b4cfc4fe80cc24f41) )
	ROM_LOAD("obj6.078",  0x0400000, 0x200000, CRC(1b6a523c) SHA1(99a420dbc8e22e7832ccda7cec9fa661a2a2687a) )
	ROM_LOAD("obj2.074",  0x0600000, 0x400000, CRC(7aeadd8e) SHA1(47103c0579240c5b1add4d0b164eaf76f5fa97f0) )
	ROM_LOAD("obj5.077",  0x0a00000, 0x200000, CRC(5d790a5d) SHA1(1ed5d4ad4c9a7e505ce35dcc90d184c26ce891dc) )
	ROM_LOAD("obj1.073",  0x0c00000, 0x400000, CRC(c2c50f02) SHA1(b81397b5800c6d49f58b7ac7ff6eac56da3c5257) )
	ROM_LOAD("obj4.076",  0x1000000, 0x200000, CRC(5259321f) SHA1(3c70c1147e49f81371d0f60f7108d9718d56faf4) )

	ROM_REGION( 0x80000, "oki1", 0 ) /* sound data for MSM6295 */
	ROM_LOAD("pcm0.1022", 0x000000, 0x80000, CRC(fd599b35) SHA1(00c0307d1b503bd5ce02d7960ce5e1ad600a7290) )

	ROM_REGION( 0x80000, "oki2", 0 ) /* sound data for MSM6295 */
	ROM_LOAD("pcm1.1023", 0x000000, 0x80000, CRC(8b716356) SHA1(42ee1896c02518cd1e9cb0dc130321834665a79e) )
ROM_END


ROM_START( rfjet2kc ) /* SYS386I */
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_WORD("prg01.u267", 0x000000, 0x100000, CRC(36019fa8) SHA1(28baf0ed4a53b818c1e6986d5d3491373524eca1) )
	ROM_LOAD32_WORD("prg23.u266", 0x000002, 0x100000, CRC(65695dde) SHA1(1b25dde03bc9319414144fc13b34c455112f4076) )

	ROM_REGION( 0x30000, "chars", ROMREGION_ERASEFF ) /* text layer roms */
	ROM_LOAD24_BYTE("rfj-01.524", 0x000001, 0x10000, CRC(e9d53007) SHA1(29aa7b70d5d5eb5e31426ac84143be44bc0597aa) )
	ROM_LOAD24_BYTE("rfj-02.518", 0x000000, 0x10000, CRC(dd3eabd3) SHA1(31c8f7a0cd262096a77673b040326605db542ab8) )
	ROM_LOAD24_BYTE("rfj-03.514", 0x000002, 0x10000, CRC(0daa8aac) SHA1(08a98fb3079ea9f78aa5b950bfeb30b0a805bab7) )

	ROM_REGION( 0xc00000, "tiles", ROMREGION_ERASEFF ) /* background layer roms */
	ROM_LOAD24_WORD("bg-1d.u0535", 0x000000, 0x400000, CRC(edfd96da) SHA1(4813267f104619f569e5777e75b75304321abb49) )
	ROM_LOAD24_BYTE("bg-1p.u0537", 0x000002, 0x200000, CRC(a4cc4631) SHA1(cc1c4f4de8a078ca774f5a328a9a58291949b1fb) )
	ROM_LOAD24_WORD("bg-2d.u0536", 0x600000, 0x200000, CRC(731fbb59) SHA1(13cd29ec4d4c73582c5fb363218e737886826e5f) )
	ROM_LOAD24_BYTE("bg-2p.u0547", 0x600002, 0x100000, CRC(03652c25) SHA1(c0d77285111bc84e008362981ac02a246678ed0a) )

	ROM_REGION( 0x1800000, "sprites", 0 ) /* sprites */
	ROM_LOAD("obj-1.u073",  0x0000000, 0x800000, CRC(58a59896) SHA1(edeaaa69987bd5d08c47ed9bf47a3901e2dcc892) )
	ROM_LOAD("obj-2.u074",  0x0800000, 0x800000, CRC(a121d1e3) SHA1(1851ae81f2ae9d3404aadd9fbc0ed7f9230290b9) )
	ROM_LOAD("obj-3.u0749", 0x1000000, 0x800000, CRC(bc2c0c63) SHA1(c8d395722f7012c3be366a0fc9b224c537afabae) )

	ROM_REGION( 0x80000, "oki1", 0 ) /* sound data for MSM6295 */
	ROM_LOAD("rfj-05.u1022", 0x000000, 0x80000, CRC(fd599b35) SHA1(00c0307d1b503bd5ce02d7960ce5e1ad600a7290) )

	ROM_REGION( 0x80000, "oki2", 0 ) /* sound data for MSM6295 */
	ROM_LOAD("rfj-04.u1023", 0x000000, 0x80000, CRC(1d10cd08) SHA1(c431d3f1a7b580024b083dafb76c53b771c88726) )
ROM_END


ROM_START( ejsakura ) /* SYS386F V2.0 */
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("prg0.211",  0x100000, 0x40000, CRC(199f2f08) SHA1(096afb23f2763b9aee5e8de3870fe47116a8d134) )
	ROM_LOAD32_BYTE("prg1.212",  0x100001, 0x40000, CRC(2cb636e6) SHA1(3524231a336de5acc93dff20b0b65ade31e27116) )
	ROM_LOAD32_BYTE("prg2.221",  0x100002, 0x40000, CRC(98a7b615) SHA1(ea34d8f3e9200a6d84efe9168e2f573ec5c2afd2) )
	ROM_LOAD32_BYTE("prg3.220",  0x100003, 0x40000, CRC(9c3c037a) SHA1(a802e13a0b827896342d9d34dbb00d1c36cabaff) )

	ROM_REGION( 0x1000000, "sprites", 0 ) /* sprites */
	ROM_LOAD16_WORD_SWAP("chr4.445", 0x000000, 0x400000, CRC(40c6c238) SHA1(0d07b59e25632feb070ce0e572ae75f9bb939893) )
	ROM_LOAD16_WORD_SWAP("chr3.444", 0x400000, 0x400000, CRC(8e5d1de5) SHA1(c1ccb6b4341ee1e939958ec9e68280c6faa2ef1f) )
	ROM_LOAD16_WORD_SWAP("chr2.443", 0x800000, 0x400000, CRC(638dc9ae) SHA1(0c11b1e688733fbaeabf83b33410714c22ae53cd) )
	ROM_LOAD16_WORD_SWAP("chr1.442", 0xc00000, 0x400000, CRC(177e3139) SHA1(0385a831c141d59ec4e9c6d6fae9436dca123764) )

	ROM_REGION( 0x1000000, "ymz", 0 )
	ROM_LOAD("sound1.83",  0x000000, 0x800000, CRC(98783cfc) SHA1(f142429e0658a36e908cc135fe0e01ce853d071d) )
	ROM_LOAD("sound2.84",  0x800000, 0x800000, CRC(ff37e769) SHA1(eb6d260cbc4e4a925a5d8f604ec695e567ac6bb5) )
ROM_END

ROM_START( ejsakura12 ) /* SYS386F V1.2 */
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) /* i386 program */
	ROM_LOAD32_BYTE("prg0v1.2.u0211",  0x100000, 0x40000, CRC(c734fde6) SHA1(d4256f0d2be624fc0e5340ae14679679e5e184c8) )
	ROM_LOAD32_BYTE("prg1v1.2.u0212",  0x100001, 0x40000, CRC(fb7a9e38) SHA1(5a2e02e1b8ed71ffc96dbda871618f5f9cccc8c6) )
	ROM_LOAD32_BYTE("prg2v1.2.u0221",  0x100002, 0x40000, CRC(e13098ad) SHA1(abf471afd25a08ba1848964c988112c86d1dcfaa) )
	ROM_LOAD32_BYTE("prg3v1.2.u0220",  0x100003, 0x40000, CRC(29b5460f) SHA1(c9cb0eb421a79b722bf5a0dc428d0f5f8499e170) )

	ROM_REGION( 0x1000000, "sprites", 0 ) /* sprites */
	ROM_LOAD16_WORD_SWAP("chr4.445", 0x000000, 0x400000, CRC(40c6c238) SHA1(0d07b59e25632feb070ce0e572ae75f9bb939893) )
	ROM_LOAD16_WORD_SWAP("chr3.444", 0x400000, 0x400000, CRC(8e5d1de5) SHA1(c1ccb6b4341ee1e939958ec9e68280c6faa2ef1f) )
	ROM_LOAD16_WORD_SWAP("chr2.443", 0x800000, 0x400000, CRC(638dc9ae) SHA1(0c11b1e688733fbaeabf83b33410714c22ae53cd) )
	ROM_LOAD16_WORD_SWAP("chr1.442", 0xc00000, 0x400000, CRC(177e3139) SHA1(0385a831c141d59ec4e9c6d6fae9436dca123764) )

	ROM_REGION( 0x1000000, "ymz", 0 )
	ROM_LOAD("sound1.83",  0x000000, 0x800000, CRC(98783cfc) SHA1(f142429e0658a36e908cc135fe0e01ce853d071d) )
	ROM_LOAD("sound2.84",  0x800000, 0x800000, CRC(ff37e769) SHA1(eb6d260cbc4e4a925a5d8f604ec695e567ac6bb5) )
ROM_END


/*****************************************************************************/

/* SPI */
GAME( 1995, senkyu,     0,        spi,     spi_3button, seibuspi_state, init_senkyu,   ROT0,   "Seibu Kaihatsu",                         "Senkyu (Japan, newer)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, senkyua,    senkyu,   spi,     spi_3button, seibuspi_state, init_senkyua,  ROT0,   "Seibu Kaihatsu",                         "Senkyu (Japan, earlier)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, batlball,   senkyu,   spi,     spi_3button, seibuspi_state, init_batlball, ROT0,   "Seibu Kaihatsu (Tuning license)",        "Battle Balls (Germany, newer)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, batlballo,  senkyu,   spi,     spi_3button, seibuspi_state, init_batlball, ROT0,   "Seibu Kaihatsu (Tuning license)",        "Battle Balls (Germany, earlier)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, batlballu,  senkyu,   spi,     spi_3button, seibuspi_state, init_batlball, ROT0,   "Seibu Kaihatsu (Fabtek license)",        "Battle Balls (US)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, batlballa,  senkyu,   spi,     spi_3button, seibuspi_state, init_batlball, ROT0,   "Seibu Kaihatsu (Metrotainment license)", "Battle Balls (Hong Kong)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, batlballe,  senkyu,   spi,     spi_3button, seibuspi_state, init_batlball, ROT0,   "Seibu Kaihatsu (Metrotainment license)", "Battle Balls (Hong Kong, earlier)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, batlballpt, senkyu,   spi,     spi_3button, seibuspi_state, init_senkyua,  ROT0,   "Seibu Kaihatsu",                         "Battle Balls (Portugal)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS ) // 1 program ROM dated 171195

// these are unique
GAME( 1995, viprp1,     0,        spi,     spi_3button, seibuspi_state, init_viprp1,   ROT270, "Seibu Kaihatsu",                         "Viper Phase 1 (New Version, World)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, viprp1k,    viprp1,   spi,     spi_3button, seibuspi_state, init_viprp1,   ROT270, "Seibu Kaihatsu (Dream Island license)",  "Viper Phase 1 (New Version, Korea)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, viprp1u,    viprp1,   spi,     spi_3button, seibuspi_state, init_viprp1o,  ROT270, "Seibu Kaihatsu (Fabtek license)",        "Viper Phase 1 (New Version, US set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS ) /* New version, "=U.S.A=" seems part of title */
GAME( 1995, viprp1ua,   viprp1,   spi,     spi_3button, seibuspi_state, init_viprp1o,  ROT270, "Seibu Kaihatsu (Fabtek license)",        "Viper Phase 1 (New Version, US set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS ) /* New version, "=U.S.A=" seems part of title */
GAME( 1995, viprp1j,    viprp1,   spi,     spi_3button, seibuspi_state, init_viprp1,   ROT270, "Seibu Kaihatsu",                         "Viper Phase 1 (New Version, Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
// same code revision (010995 code base) - SXX2C ROM SUB cart
// counterintuitively this seems to be the oldest code base of the game despite playing with the 'new version' rules, it has various typos not present in other sets eg. 'UPDATEING'
GAME( 1995, viprp1s,    viprp1,   spi,     spi_3button, seibuspi_state, init_viprp1,   ROT270, "Seibu Kaihatsu",                         "Viper Phase 1 (New Version, Switzerland)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, viprp1h,    viprp1,   spi,     spi_3button, seibuspi_state, init_viprp1,   ROT270, "Seibu Kaihatsu",                         "Viper Phase 1 (New Version, Holland)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, viprp1t,    viprp1,   spi,     spi_3button, seibuspi_state, init_viprp1,   ROT270, "Seibu Kaihatsu (Tuning license)",        "Viper Phase 1 (New Version, Germany)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, viprp1pt,   viprp1,   spi,     spi_3button, seibuspi_state, init_viprp1,   ROT270, "Seibu Kaihatsu",                         "Viper Phase 1 (New Version, Portugal)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
// these are unique
GAME( 1995, viprp1ot,   viprp1,   spi,     spi_3button, seibuspi_state, init_viprp1,   ROT270, "Seibu Kaihatsu (Tuning license)",        "Viper Phase 1 (Germany)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, viprp1oj,   viprp1,   spi,     spi_3button, seibuspi_state, init_viprp1o,  ROT270, "Seibu Kaihatsu",                         "Viper Phase 1 (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, viprp1hk,   viprp1,   spi,     spi_3button, seibuspi_state, init_viprp1,   ROT270, "Seibu Kaihatsu (Metrotainment license)", "Viper Phase 1 (Hong Kong)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS ) /* "=HONG KONG=" seems part of title */

GAME( 1996, ejanhs,     0,        ejanhs,  spi_ejanhs,  seibuspi_state, init_ejanhs,   ROT0,   "Seibu Kaihatsu",                         "E Jong High School (Japan)", MACHINE_SUPPORTS_SAVE )

// these are unique
GAME( 1996, rdft,       0,        spi,     spi_3button, seibuspi_state, init_rdft,     ROT270, "Seibu Kaihatsu (Tuning license)",        "Raiden Fighters (Germany)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, rdftj,      rdft,     spi,     spi_3button, seibuspi_state, init_rdft,     ROT270, "Seibu Kaihatsu",                         "Raiden Fighters (Japan, earlier)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, rdftja,     rdft,     spi,     spi_3button, seibuspi_state, init_rdft,     ROT270, "Seibu Kaihatsu",                         "Raiden Fighters (Japan, earliest)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, rdftu,      rdft,     spi,     spi_3button, seibuspi_state, init_rdft,     ROT270, "Seibu Kaihatsu (Fabtek license)",        "Raiden Fighters (US, earlier)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, rdftauge,   rdft,     spi,     spi_3button, seibuspi_state, init_rdft,     ROT270, "Seibu Kaihatsu (Tuning license)",        "Raiden Fighters (Evaluation Software For Show, Germany)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
// this is one revision - SXX2C ROM SUB4 cart
GAME( 1996, rdftua,     rdft,     spi,     spi_3button, seibuspi_state, init_rdft,     ROT270, "Seibu Kaihatsu (Fabtek license)",        "Raiden Fighters (US, newer)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, rdftjb,     rdft,     spi,     spi_3button, seibuspi_state, init_rdft,     ROT270, "Seibu Kaihatsu",                         "Raiden Fighters (Japan, newer)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, rdftau,     rdft,     spi,     spi_3button, seibuspi_state, init_rdft,     ROT270, "Seibu Kaihatsu",                         "Raiden Fighters (Australia)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, rdftam,     rdft,     spi,     spi_3button, seibuspi_state, init_rdft,     ROT270, "Seibu Kaihatsu (Metrotainment license)", "Raiden Fighters (Hong Kong)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, rdftadi,    rdft,     spi,     spi_3button, seibuspi_state, init_rdft,     ROT270, "Seibu Kaihatsu (Dream Island license)",  "Raiden Fighters (Korea)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
// same code revision - SXX2C ROM SUB2 cart
GAME( 1996, rdfta,      rdft,     spi,     spi_3button, seibuspi_state, init_rdft,     ROT270, "Seibu Kaihatsu",                         "Raiden Fighters (Austria)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, rdftgb,     rdft,     spi,     spi_3button, seibuspi_state, init_rdft,     ROT270, "Seibu Kaihatsu",                         "Raiden Fighters (Great Britain)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, rdftgr,     rdft,     spi,     spi_3button, seibuspi_state, init_rdft,     ROT270, "Seibu Kaihatsu",                         "Raiden Fighters (Greece)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, rdftit,     rdft,     spi,     spi_3button, seibuspi_state, init_rdft,     ROT270, "Seibu Kaihatsu",                         "Raiden Fighters (Italy)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )

// this is one revision
GAME( 1997, rdft2,      0,        rdft2,   spi_2button, seibuspi_state, init_rdft2,    ROT270, "Seibu Kaihatsu (Tuning license)",        "Raiden Fighters 2 - Operation Hell Dive (Germany)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, rdft2j,     rdft2,    rdft2,   spi_2button, seibuspi_state, init_rdft2,    ROT270, "Seibu Kaihatsu",                         "Raiden Fighters 2 - Operation Hell Dive (Japan set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, rdft2a,     rdft2,    rdft2,   spi_2button, seibuspi_state, init_rdft2,    ROT270, "Seibu Kaihatsu (Metrotainment license)", "Raiden Fighters 2 - Operation Hell Dive (Hong Kong)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, rdft2s,     rdft2,    rdft2,   spi_2button, seibuspi_state, init_rdft2,    ROT270, "Seibu Kaihatsu",                         "Raiden Fighters 2 - Operation Hell Dive (Switzerland)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
// this is another
GAME( 1997, rdft2ja,    rdft2,    rdft2,   spi_2button, seibuspi_state, init_rdft2,    ROT270, "Seibu Kaihatsu",                         "Raiden Fighters 2 - Operation Hell Dive (Japan set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, rdft2aa,    rdft2,    rdft2,   spi_2button, seibuspi_state, init_rdft2,    ROT270, "Seibu Kaihatsu (Dream Island license)",  "Raiden Fighters 2 - Operation Hell Dive (Korea)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, rdft2it,    rdft2,    rdft2,   spi_2button, seibuspi_state, init_rdft2,    ROT270, "Seibu Kaihatsu",                         "Raiden Fighters 2 - Operation Hell Dive (Italy)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
// these are unique
GAME( 1997, rdft2jb,    rdft2,    rdft2,   spi_2button, seibuspi_state, init_rdft2,    ROT270, "Seibu Kaihatsu",                         "Raiden Fighters 2 - Operation Hell Dive (Japan set 3)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, rdft2jc,    rdft2,    rdft2,   spi_2button, seibuspi_state, init_rdft2,    ROT270, "Seibu Kaihatsu",                         "Raiden Fighters 2 - Operation Hell Dive (Japan set 4)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, rdft2t,     rdft2,    rdft2,   spi_2button, seibuspi_state, init_rdft2,    ROT270, "Seibu Kaihatsu",                         "Raiden Fighters 2 - Operation Hell Dive (Taiwan)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, rdft2u,     rdft2,    rdft2,   spi_2button, seibuspi_state, init_rdft2,    ROT270, "Seibu Kaihatsu (Fabtek license)",        "Raiden Fighters 2 - Operation Hell Dive (US)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )

GAME( 1998, rfjet,      0,        rdft2,   spi_2button, seibuspi_state, init_rfjet,    ROT270, "Seibu Kaihatsu (Tuning license)",        "Raiden Fighters Jet (Germany)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, rfjetu,     rfjet,    rdft2,   spi_2button, seibuspi_state, init_rfjet,    ROT270, "Seibu Kaihatsu (Fabtek license)",        "Raiden Fighters Jet (US)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, rfjetj,     rfjet,    rdft2,   spi_2button, seibuspi_state, init_rfjet,    ROT270, "Seibu Kaihatsu",                         "Raiden Fighters Jet (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, rfjeta,     rfjet,    rdft2,   spi_2button, seibuspi_state, init_rfjet,    ROT270, "Seibu Kaihatsu (Dream Island license)",  "Raiden Fighters Jet (Korea)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, rfjett,     rfjet,    rdft2,   spi_2button, seibuspi_state, init_rfjet,    ROT270, "Seibu Kaihatsu",                         "Raiden Fighters Jet (Taiwan)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )

/* SXX2E */
GAME( 1996, rdfts,      rdft,     sxx2e,   sxx2e,       seibuspi_state, init_rdft,     ROT270, "Seibu Kaihatsu (Explorer System Corp. license)", "Raiden Fighters (Taiwan, single board)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )

/* SXX2F */
GAME( 1997, rdft2us,    rdft2,    sxx2f,   sxx2f,       seibuspi_state, init_rdft2,    ROT270, "Seibu Kaihatsu (Fabtek license)", "Raiden Fighters 2 - Operation Hell Dive (US, single board)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS ) // title screen shows small '.1'

/* SXX2G */
GAME( 1999, rfjets,     rfjet,    sxx2g,   sxx2f,       seibuspi_state, init_rfjet,    ROT270, "Seibu Kaihatsu", "Raiden Fighters Jet (US, single board)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS ) // has 1998-99 copyright + planes unlocked
GAME( 1999, rfjetsa,    rfjet,    sxx2g,   sxx2f,       seibuspi_state, init_rfjet,    ROT270, "Seibu Kaihatsu", "Raiden Fighters Jet (US, single board, test version?)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS ) // maybe test/proto? see notes at romdefs

/* SYS386I */
GAME( 2000, rdft22kc,   rdft2,    sys386i, sys386i,     seibuspi_state, init_rdft2,    ROT270, "Seibu Kaihatsu", "Raiden Fighters 2 - Operation Hell Dive 2000 (China, SYS386I)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2000, rfjet2kc,   rfjet,    sys386i, sys386i,     seibuspi_state, init_rfjet,    ROT270, "Seibu Kaihatsu", "Raiden Fighters Jet 2000 (China, SYS386I)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )

/* SYS386F */
GAME( 1999, ejsakura,   0,        sys386f, ejsakura,    seibuspi_state, init_sys386f,  ROT0,   "Seibu Kaihatsu", "E-Jan Sakurasou (Japan, SYS386F V2.0)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, ejsakura12, ejsakura, sys386f, ejsakura,    seibuspi_state, init_sys386f,  ROT0,   "Seibu Kaihatsu", "E-Jan Sakurasou (Japan, SYS386F V1.2)", MACHINE_SUPPORTS_SAVE )
