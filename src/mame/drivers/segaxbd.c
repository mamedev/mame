// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega X-board hardware

    Special thanks to Charles MacDonald for his priceless assistance

****************************************************************************

    Known bugs:
        * gprider has a hack to make it work
        * smgp network and motor boards not hooked up
        * rachero doesn't like IC17/IC108 (divide chips) in self-test
          due to testing an out-of-bounds value
        * abcop doesn't like IC41/IC108 (divide chips) in self-test
          due to testing an out-of-bounds value
        * rascot is not working at all


Sega X-Board System Overview
Sega, 1987-1992

The following games are known to exist on the X-Board hardware...

AB Cop             (C) Sega 1990
After Burner       (C) Sega 1987
After Burner II    (C) Sega 1987
*Caribbean Boule   (C) Sega 1992
GP Rider           (C) Sega 1990
Last Survivor      (C) Sega 1989
Line of Fire       (C) Sega 1989
Racing Hero        (C) Sega 1990
Royal Ascot        (C) Sega 1991    dumped, but very likely incomplete
Super Monaco GP    (C) Sega 1989
Thunder Blade      (C) Sega 1987

* denotes not dumped. There are also several revisions of the above games not dumper either.

Main Board
----------
Top    : 834-6335
Bottom : 171-5494
Sticker: 834-7088-01 REV. B  SUPER MONACO GP
Sticker: 834-6335-02 AFTER BURNER
|-----------------------------------------------------------------------------|
|IC67 IC66 IC65 IC64 IC58 IC57 IC56 IC55  BATT  CNH  16MHz CNA    CNE     CNF |
|IC71 IC70 IC69 IC68                      IC107  IC15   IC11    IC7  IC5  IC1 |
|IC75 IC74 IC73 IC72                             IC16   IC12   IC8   IC6  IC2 |
|IC79 IC78 IC77 IC76 IC63 IC62 IC61 IC60  IC108  IC17   IC13   IC10 IC9   IC3 |
|                                                IC18   IC14                  |
|                                                                             |
|                                                                             |
|     IC84              IC81                                                  |
|                                                                IC23         |
|                                                                IC22         |
|                                       IC109                        IC21 IC20|
|                     IC125      IC118              IC28             IC30 IC29|
|IC93 IC92 IC91 IC90  IC126                                      IC31         |
|                                             IC53               IC32         |
|                     IC32*                              IC40  IC38           |
|                     IC33*                                    IC39        CNI|
|IC97 IC96 IC95 IC94        IC127       IC117                                 |
|                     IC134                                                   |
|                     IC135                   50MHz                           |
|                                                                     IC37    |
|IC101 IC100 IC99 IC98  IC148                                                 |
|                                        IC165                                |
|                                                              IC42           |
|                                 IC150        IC170                  IC41    |
|IC105 IC104 IC103 IC102                                                      |
|                    IC154  IC152                  IC160  IC159  DIPSWB DIPSWA|
|                       IC153     IC151  IC149   CNG      CNB      CNC    CND |
|-----------------------------------------------------------------------------|
Notes:
      ROMs: (ROM locations on the PCB not listed are not populated)
        Type (note 1) 27C1000    27C1000    27C1000    27C1000    27C1000    27C1000    27C1000    27C1000    27C512     27C512     27C512     831000     831000     831000     831000     831000     831000     831000     831000     831000     831000     831000     831000     27C1000    27C1000    27C1000    27C1000    27C512     27C512     831000     831000     831000
        Location      IC58       IC63       IC57       IC62       IC20       IC29       IC21       IC30       IC154      IC153      IC152      IC90       IC94       IC98       IC102      IC91       IC95       IC99       IC103      IC92       IC96       IC100      IC104      IC93       IC97       IC101      IC105      IC40       IC17       IC11       IC12       IC13
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
After Burner          EPR-10940  EPR-10941  -          -          EPR-10927  EPR-10928  -          -          EPR-10926  EPR-10925  EPR-10924  MPR-10932  MPR-10934  MPR-10936  MPR-10938  MPR-10933  MPR-10935  MPR-10937  MPR-10939  EPR-10942  EPR-10943  EPR-10944  EPR-10945  EPR-10946  EPR-10947  EPR-10948  EPR-10949  EPR-10922  MPR-10923  MPR-10930  MPR-10931  MPR-11102
After Burner 2        EPR-11107  EPR-11108  -          -          EPR-11109  EPR-11110  -          -          EPR-11115  EPR-11114  EPR-11113  MPR-10932  MPR-10934  MPR-10936  MPR-10938  MPR-10933  MPR-10935  MPR-10937  MPR-10939  MPR-11103  MPR-11104  MPR-11105  MPR-11106  EPR-11116  EPR-11117  EPR-11118  EPR-11119  EPR-10922  EPR-11112  MPR-10930  MPR-10931  EPR-11102
Line Of Fire (set 3)  EPR-12849  EPR-12850  -          -          EPR-12804  EPR-12805  EPR-12802  EPR-12803  OPR-12791  OPR-12792  OPR-12793  EPR-12787  EPR-12788  EPR-12789  EPR-12790  EPR-12783  EPR-12784  EPR-12785  EPR-12786  EPR-12779  EPR-12780  EPR-12781  EPR-12782  EPR-12775  EPR-12776  EPR-12777  EPR-12778  -          EPR-12798  EPR-12799  EPR-12800  EPR-12801
Line Of Fire (set 2)  EPR-12847A EPR-12848A -          -          EPR-12804  EPR-12805  EPR-12802  EPR-12803  OPR-12791  OPR-12792  OPR-12793  EPR-12787  EPR-12788  EPR-12789  EPR-12790  EPR-12783  EPR-12784  EPR-12785  EPR-12786  EPR-12779  EPR-12780  EPR-12781  EPR-12782  EPR-12775  EPR-12776  EPR-12777  EPR-12778  -          EPR-12798  EPR-12799  EPR-12800  EPR-12801
Line Of Fire (set 1)  EPR-12794  EPR-12795  -          -          EPR-12804  EPR-12805  EPR-12802  EPR-12803  OPR-12791  OPR-12792  OPR-12793  EPR-12787  EPR-12788  EPR-12789  EPR-12790  EPR-12783  EPR-12784  EPR-12785  EPR-12786  EPR-12779  EPR-12780  EPR-12781  EPR-12782  EPR-12775  EPR-12776  EPR-12777  EPR-12778  -          EPR-12798  EPR-12799  EPR-12800  EPR-12801
Thunder Blade (set 2) EPR-11405  EPR-11406  EPR-11306  EPR-11307  EPR-11390  EPR-11391  EPR-11310  EPR-11311  EPR-11314  EPR-11315  EPR-11316  EPR-11323  EPR-11322  EPR-11321  EPR-11320  EPR-11327  EPR-11326  EPR-11325  EPR-11324  EPR-11331  EPR-11330  EPR-11329  EPR-11328  EPR-11395  EPR-11394  EPR-11393  EPR-11392  EPR-11313  EPR-11396  EPR-11317  EPR-11318  EPR-11319
Thunder Blade (set 1) EPR-11304  EPR-11305  EPR-11306  EPR-11307  EPR-11308  EPR-11309  EPR-11310  EPR-11311  EPR-11314  EPR-11315  EPR-11316  EPR-11323  EPR-11322  EPR-11321  EPR-11320  EPR-11327  EPR-11326  EPR-11325  EPR-11324  EPR-11331  EPR-11330  EPR-11329  EPR-11328  EPR-11335  EPR-11334  EPR-11333  EPR-11332  EPR-11313  EPR-11312  EPR-11317  EPR-11318  EPR-11319
Racing Hero           EPR-13129  EPR-13130  EPR-12855  EPR-12856  EPR-12857  EPR-12858  -          -          EPR-12879  EPR-12880  EPR-12881  EPR-12872  EPR-12873  EPR-12874  EPR-12875  EPR-12868  EPR-12869  EPR-12870  EPR-12871  EPR-12864  EPR-12865  EPR-12866  EPR-12867  EPR-12860  EPR-12861  EPR-12862  EPR-12863  -          EPR-12859  EPR-12876  EPR-12877  EPR-12878
S.Monaco GP (set 9)   EPR-12563B EPR-12564B -          -          EPR-12576A EPR-12577A -          -          EPR-12429  EPR-12430  EPR-12431  MPR-12425  MPR-12426  MPR-12427  MPR-12428  MPR-12421  MPR-12422  MPR-12423  MPR-12424  MPR-12417  MPR-12418  MPR-12419  MPR-12420  EPR-12609  EPR-12610  EPR-12611  EPR-12612  -          EPR-12436  MPR-12437  MPR-12438  MPR-12439
S.Monaco GP (set 8)   EPR-12563A EPR-12564A -          -          EPR-12576A EPR-12577A -          -          EPR-12429  EPR-12430  EPR-12431  MPR-12425  MPR-12426  MPR-12427  MPR-12428  MPR-12421  MPR-12422  MPR-12423  MPR-12424  MPR-12417  MPR-12418  MPR-12419  MPR-12420  EPR-12609  EPR-12610  EPR-12611  EPR-12612  -          EPR-12436  MPR-12437  MPR-12438  MPR-12439
S.Monaco GP (set 7)   EPR-12563  EPR-12564  -          -          EPR-12576  EPR-12577  -          -          EPR-12429  EPR-12430  EPR-12431  MPR-12425  MPR-12426  MPR-12427  MPR-12428  MPR-12421  MPR-12422  MPR-12423  MPR-12424  MPR-12417  MPR-12418  MPR-12419  MPR-12420  EPR-12609  EPR-12610  EPR-12611  EPR-12612  -          EPR-12436  MPR-12437  MPR-12438  MPR-12439
S.Monaco GP (set 6)   EPR-12561C EPR-12562C -          -          EPR-12574A EPR-12575A -          -          EPR-12429  EPR-12430  EPR-12431  MPR-12425  MPR-12426  MPR-12427  MPR-12428  MPR-12421  MPR-12422  MPR-12423  MPR-12424  MPR-12417  MPR-12418  MPR-12419  MPR-12420  EPR-12609  EPR-12610  EPR-12611  EPR-12612  -          EPR-12436  MPR-12437  MPR-12438  MPR-12439
S.Monaco GP (set 5)   EPR-12561B EPR-12562B -          -          EPR-12574A EPR-12575A -          -          EPR-12429  EPR-12430  EPR-12431  MPR-12425  MPR-12426  MPR-12427  MPR-12428  MPR-12421  MPR-12422  MPR-12423  MPR-12424  MPR-12417  MPR-12418  MPR-12419  MPR-12420  EPR-12609  EPR-12610  EPR-12611  EPR-12612  -          EPR-12436  MPR-12437  MPR-12438  MPR-12439
S.Monaco GP (set 4)   EPR-12561A EPR-12562A -          -          EPR-12574A EPR-12575A -          -          EPR-12429  EPR-12430  EPR-12431  MPR-12425  MPR-12426  MPR-12427  MPR-12428  MPR-12421  MPR-12422  MPR-12423  MPR-12424  MPR-12417  MPR-12418  MPR-12419  MPR-12420  EPR-12609  EPR-12610  EPR-12611  EPR-12612  -          EPR-12436  MPR-12437  MPR-12438  MPR-12439
S.Monaco GP (set 3)   EPR-12561  EPR-12562  -          -          EPR-12574A EPR-12575A -          -          EPR-12429  EPR-12430  EPR-12431  MPR-12425  MPR-12426  MPR-12427  MPR-12428  MPR-12421  MPR-12422  MPR-12423  MPR-12424  MPR-12417  MPR-12418  MPR-12419  MPR-12420  EPR-12609  EPR-12610  EPR-12611  EPR-12612  -          EPR-12436  MPR-12437  MPR-12438  MPR-12439
S.Monaco GP (set 2)   EPR-12432B EPR-12433B -          -          EPR-12441A EPR-12442A -          -          EPR-12429  EPR-12430  EPR-12431  MPR-12425  MPR-12426  MPR-12427  MPR-12428  MPR-12421  MPR-12422  MPR-12423  MPR-12424  MPR-12417  MPR-12418  MPR-12419  MPR-12420  EPR-12413  EPR-12414  EPR-12415  EPR-12416  -          EPR-12436  MPR-12437  MPR-12438  MPR-12439
S.Monaco GP (set 1)   EPR-12432A EPR-12433A -          -          EPR-12441A EPR-12442A -          -          EPR-12429  EPR-12430  EPR-12431  MPR-12425  MPR-12426  MPR-12427  MPR-12428  MPR-12421  MPR-12422  MPR-12423  MPR-12424  MPR-12417  MPR-12418  MPR-12419  MPR-12420  EPR-12413  EPR-12414  EPR-12415  EPR-12416  -          EPR-12436  MPR-12437  MPR-12438  MPR-12439
AB Cop                EPR-13568B EPR-13556B EPR-13559  EPR-13558  EPR-13566  EPR-13565  -          -          OPR-13553  OPR-13554  OPR-13555  OPR-13552  OPR-13551  OPR-13550  OPR-13549  OPR-13548  OPR-13547  OPR-13546  OPR-13545  OPR-13544  OPR-13543  OPR-13542  OPR-13541  OPR-13540  OPR-13539  OPR-13538  OPR-13537  EPR-13564  EPR-13560  OPR-13563  OPR-13562  OPR-13561
GP Rider (set 2)      EPR-13408  EPR-13409  -          -          EPR-13395  EPR-13394  EPR-13393  EPR-13392  EPR-13383  EPR-13384  EPR-13385  EPR-13382  EPR-13381  EPR-13380  EPR-13379  EPR-13378  EPR-13377  EPR-13376  EPR-13375  EPR-13374  EPR-13373  EPR-13372  EPR-13371  EPR-13370  EPR-13369  EPR-13368  EPR-13367  -          EPR-13388  EPR-13391  EPR-13390  EPR-13389
GP Rider (set 1)      EPR-13406  EPR-13407  -          -          EPR-13395  EPR-13394  EPR-13393  EPR-13392  EPR-13383  EPR-13384  EPR-13385  EPR-13382  EPR-13381  EPR-13380  EPR-13379  EPR-13378  EPR-13377  EPR-13376  EPR-13375  EPR-13374  EPR-13373  EPR-13372  EPR-13371  EPR-13370  EPR-13369  EPR-13368  EPR-13367  -          EPR-13388  EPR-13391  EPR-13390  EPR-13389

Note 1 - PCB can handle 27C1000 / 27C010 / 831000 ROMs via jumpers
         If label = EPR, ROM is 32 pin 27C1000 or 27C010
         If label = MPR, ROM is 28 pin 831000
         For jumper settings, 27C1000 also means 831000 can be used
         S28 shorted, S26 open = ROMS 90-103 (Groups 1,2) use 831000
         S28 open, S26 shorted = ROMS 90-103 (Groups 1,2) use 27C010
         S29 shorted, S27 open = ROMS 90-103 (Groups 3,4) use 831000
         S29 open, S27 shorted = ROMS 92-105 (Groups 3,4) use 27C010
         For IC11/12/13, set jumpers S1 open, S2 resistor, S3 open, S4 resistor for 27C1000. Reverse them for 27C010
         For IC20/29 set jumpers S5 resistor, S6 open, S7 resistor, S8 open for 27C1000. Reverse them for 27C010
         For IC21/30 set jumpers S9 resistor, S10 open, S11 resistor, S12 open for 27C1000. Reverse them for 27C010
         For IC57/62 set jumpers S18 resistor, S19 open, S20 resistor, S21 open for 27C1000. Reverse them for 27C010
         For IC58/63 set jumpers S22 resistor, S23 open, S24 resistor, S25 open for 27C1000. Reverse them for 27C010
         For IC40 set jumpers S13 open, S14 resistor to set 27C512. Reverse them for 27C256
         For IC152/153/154 set jumpers S31 open, S32 resistor to set 27C512. Reverse them for 27C256

PALs: (Common to all games except where noted)
     IC18 : 315-5280 (= CK2605 == PLS153) - Z80 address decoding
     IC84 : 315-5278 (= PAL16L8) - Sprite ROM bank control
     IC109: 315-5290 (= PAL16L8) - Main CPU address decoding
     IC117: 315-5291 (= PAL16L8) - Main CPU address decoding
     IC127: After Burner - 315-5279 (= PAL16R6)
            S.Monaco GP  - 315-5304 (= PAL16R6)
            GP Rider     - 315-5304 (= PAL16R6)
            Line Of Fire - 315-5304 (= PAL16R6)
            There could be other different ones or maybe there's just 2 types?

RAM:
    IC9  : 6116    (2k x8 SRAM) - Sega PCM chip RAM. == TMM2115
    IC10 : 6116    (2k x8 SRAM) - Sega PCM chip RAM
    IC16 : 6116    (2k x8 SRAM) - Z80 program RAM
    IC22 : 6264    (8k x8 SRAM) - Sub CPU Program RAM. == Sony CXK5864 or Fujitsu MB8464 or NEC D4364
    IC23 : 6264    (8k x8 SRAM) - Sub CPU Program RAM
    IC31 : 6116    (2k x8 SRAM) - Sub CPU Program RAM
    IC32 : 6116    (2k x8 SRAM) - Sub CPU Program RAM
    IC38 : 6264    (8k x8 SRAM) - Road RAM
    IC39 : 6264    (8k x8 SRAM) - Road RAM
    IC55 : 6264    (8k x8 SRAM) - Main CPU Program RAM
    IC56 : 6264    (8k x8 SRAM) - Main CPU Program RAM
    IC60 : 6264    (8k x8 SRAM) - Main CPU Program RAM
    IC61 : 6264    (8k x8 SRAM) - Main CPU Program RAM
    IC64 : TC51832 (32k x8 SRAM) - Sprite GFX RAM. == NEC uPD42832
    IC65 : TC51832 (32k x8 SRAM) - Sprite GFX RAM
    IC66 : TC51832 (32k x8 SRAM) - Sprite GFX RAM
    IC67 : TC51832 (32k x8 SRAM) - Sprite GFX RAM
    IC68 : TC51832 (32k x8 SRAM) - Sprite GFX RAM
    IC69 : TC51832 (32k x8 SRAM) - Sprite GFX RAM
    IC70 : TC51832 (32k x8 SRAM) - Sprite GFX RAM
    IC71 : TC51832 (32k x8 SRAM) - Sprite GFX RAM
    IC72 : TC51832 (32k x8 SRAM) - Sprite GFX RAM
    IC73 : TC51832 (32k x8 SRAM) - Sprite GFX RAM
    IC74 : TC51832 (32k x8 SRAM) - Sprite GFX RAM
    IC75 : TC51832 (32k x8 SRAM) - Sprite GFX RAM
    IC76 : TC51832 (32k x8 SRAM) - Sprite GFX RAM
    IC77 : TC51832 (32k x8 SRAM) - Sprite GFX RAM
    IC78 : TC51832 (32k x8 SRAM) - Sprite GFX RAM
    IC79 : TC51832 (32k x8 SRAM) - Sprite GFX RAM
    IC125: MB81C78 (8k x8 SRAM) -
    IC126: MB81C78 (8k x8 SRAM) -
    IC132: 6264    (8k x8 SRAM) - Text RAM. \ * On this PCB these are mis-labelled as IC32 and IC33
    IC133: 6264    (8k x8 SRAM) - Text RAM  /
    IC134: 62256   (32k x8 SRAM) - Tile / Background GFX RAM
    IC135: 62256   (32k x8 SRAM) - Tile / Background GFX RAM
    IC150: 6264    (8k x8 SRAM) -
    IC151: 6264    (8k x8 SRAM) -

SEGA Custom ICs:
                IC8  : 315-5218  (QFP100) - Sega Stereo PCM Sound IC with 16 channels. Clock input 16.000MHz
                IC37 : 315-5248  (QFP100) - Hardware multiplier
                IC41 : 315-5249  (QFP120) - Hardware divider
                IC42 : 315-5275  (QFP100) - Road generator, located underneath the PCB
                IC53 : 315-5250  (QFP120) - 68000 / Z80 interface, hardware comparator
                IC81 : 315-5211A (PGA179) - Sprite Generator
                IC107: 315-5248  (QFP100) - Hardware multiplier
                IC108: 315-5249  (QFP120) - Hardware divider
                IC148: 315-5197  (PGA135) - Tilemap generator (for Backgrounds)
                IC149: 315-5242  (Custom) - Color Encoder. Custom ceramic DIP package. Contains
                                            a QFP44 and some smt resistors/caps etc

OTHER:
      IC14 : Z80 CPU, clock 4.000MHz [16/4] (DIP40)
      IC15 : YM2151, clock 4.000MHz [16/4] (DIP24)
      IC28 : 68000 CPU (sub), clock 12.5000MHz [50/4] (DIP64)
      IC118: Hitachi FD1094 Encrypted 68000 CPU or regular 68000 CPU (main), clock 12.5000MHz [50/4] (DIP64)
      IC159: SONY CXD1095 CMOS I/O Port Expander (QFP64)
      IC160: SONY CXD1095 CMOS I/O Port Expander (QFP64)
      IC165: ADC0804, for control of analog inputs (DIP20)
      IC170: Fujitsu MB3773 Reset IC (DIP8)
      IC1  : NEC uPC324 Low Power Quad Operational Amplifier (DIP14)
      IC2  : NEC uPC4082 J-FET Dual Input Operational Amplifier (DIP8)
      IC3  : Yamaha YM3012 Sound Digital to Analog Converter (DIP16)
      IC5  : M8736 MF6CN-50 (DIP14)
      IC6  : M8736 MF6CN-50 (DIP14)
      IC7  : Exar MP7633JN CMOS 10-Bit Multiplying Digital to Analog Converter (== AD7533 / AD7530 / AD7520) (DIP16)
      BATT : 5.5 volt 0.1uF Super Cap
      CNA  : 10 pin +5V / GND Power Connector
      CNB  : 20 pin Analog Controls Connector
      CNC  : 26 pin Connector for ?
      CND  : 50 pin Digital Controls/Buttons Connector
      CNE  : 6 pin Connector for ?
      CNF  : 4 pin Stereo Sound Output Connector
      CNG  : 6 pin RGB/Sync/GND Connector
      CNH  : 10 pin Connector for ?
      CNI  : 30 pin Expansion Connector (not populated)
      VSync: 59.6368Hz  \ (measured via EL4583 & TTi PFM1300)
      HSync: 15.5645kHz /


Add-on boards for Super Monaco GP
---------------------------------

Super Monaco GP was released as upright, twin, cockpit, and deluxe 'Air Drive'.
DIP switches determine the cabinet type. It is presumed that these extra boards can be interchanged.


Network Board (twin cabinet)
-------------

Top    : 834-6780
Bottom : 171-5729-01
Sticker: 834-7112
|---------| |--| |----------------------|
|         RX   TX            315-5336   |
|             315-5337                  |
|                                       |
|            16MHz      6264            |
|                    epr-12587.14       |
| MB89372P-SH     Z80E        MB8421    |
|---------------------------------------|
Notes:
      PALs     : 315-5337, 315-5336, both PAL16L8
      Z80 clock: 8.000MHz [16/2]
      6264     : 8k x8 SRAM
      MB8421   : Fujitsu 2k x8 Dual-Port SRAM (SDIP52)
      MB89372  : Fujitsu Multi-Protocol Controller (SDIP64)
      epr-12587: 27C256 EPROM


Sound Board (for 4-channel sound, cockpit and deluxe cabinets)
-------------

label: 837-7000
Z80 (assume 4MHz)
Sega 315-5218 sound IC
ROMs:
- epr-12535.8
- mpr-12437.20
- mpr-12438.21
- mpr-12439.22


Motor Board (deluxe cabinet)
-------------

label: ?
Z80 (unknown speed)
ROMs:
- epr-12505.8


***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/segaxbd.h"
#include "cpu/m68000/m68000.h"
#include "machine/segaic16.h"
#include "machine/nvram.h"
#include "sound/2151intf.h"
#include "sound/segapcm.h"
#include "includes/segaipt.h"

const device_type SEGA_XBD_PCB = &device_creator<segaxbd_state>;

segaxbd_state::segaxbd_state(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, SEGA_XBD_PCB, "Sega System 32 PCB", tag, owner, clock, "segas32_pcb", __FILE__),
			m_maincpu(*this, "maincpu"),
			m_subcpu(*this, "subcpu"),
			m_soundcpu(*this, "soundcpu"),
			m_soundcpu2(*this, "soundcpu2"),
			m_mcu(*this, "mcu"),
			m_cmptimer_1(*this, "cmptimer_main"),
			m_sprites(*this, "sprites"),
			m_segaic16vid(*this, "segaic16vid"),
			m_segaic16road(*this, "segaic16road"),
			m_road_priority(1),
			m_scanline_timer(NULL),
			m_timer_irq_state(0),
			m_vblank_irq_state(0),
			m_loffire_sync(NULL),
			m_lastsurv_mux(0),
			m_paletteram(*this, "paletteram"),
			m_gprider_hack(false),
			m_palette_entries(0),
			m_screen(*this, "screen"),
			m_palette(*this, "palette")
{
	memset(m_adc_reverse, 0, sizeof(m_adc_reverse));
	memset(m_iochip_regs, 0, sizeof(m_iochip_regs));
	palette_init();
	memset(m_latched_value, 0, sizeof(m_latched_value));
	memset(m_latch_read, 0, sizeof(m_latch_read));
}


void segaxbd_state::device_start()
{
	if(!m_segaic16road->started())
		throw device_missing_dependencies();

	// point globals to allocated memory regions
	m_segaic16road->segaic16_roadram_0 = reinterpret_cast<UINT16 *>(memshare("roadram")->ptr());

	video_start();

	// allocate a scanline timer
	m_scanline_timer = timer_alloc(TID_SCANLINE);

	// reset the custom handlers and other pointers
	m_iochip_custom_io_w[0][3] = iowrite_delegate(FUNC(segaxbd_state::generic_iochip0_lamps_w), this);


	// save state
	save_item(NAME(m_timer_irq_state));
	save_item(NAME(m_vblank_irq_state));
	save_item(NAME(m_iochip_regs[0]));
	save_item(NAME(m_iochip_regs[1]));
	save_item(NAME(m_lastsurv_mux));
}

void segaxbd_state::device_reset()
{
	m_segaic16vid->tilemap_reset(*m_screen);

	// hook the RESET line, which resets CPU #1
	m_maincpu->set_reset_callback(write_line_delegate(FUNC(segaxbd_state::m68k_reset_callback),this));

	// start timers to track interrupts
	m_scanline_timer->adjust(m_screen->time_until_pos(1), 1);
}


class segaxbd_new_state : public driver_device
{
public:
	segaxbd_new_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_mainpcb(*this, "mainpcb")
	{
	}

	required_device<segaxbd_state> m_mainpcb;

	// game-specific driver init
	DECLARE_DRIVER_INIT(generic);
	DECLARE_DRIVER_INIT(aburner2);
	DECLARE_DRIVER_INIT(lastsurv);
	DECLARE_DRIVER_INIT(loffire);
	DECLARE_DRIVER_INIT(smgp);
	DECLARE_DRIVER_INIT(rascot);
	DECLARE_DRIVER_INIT(gprider);

};

class segaxbd_new_state_double : public segaxbd_new_state
{
public:
	segaxbd_new_state_double(const machine_config &mconfig, device_type type, const char *tag)
		: segaxbd_new_state(mconfig, type, tag),
		m_subpcb(*this, "subpcb")
	{
		for (int i = 0; i < 0x800; i++)
		{
			shareram[i] = 0x0000;
		}
		rampage1 = 0x0000;
		rampage2 = 0x0000;
	}

	required_device<segaxbd_state> m_subpcb;

	DECLARE_READ16_MEMBER(shareram1_r) {
		if (offset < 0x10) {
			int address = (rampage1 << 4) + offset;
			return shareram[address];
		}
		return 0xffff;
	}
	DECLARE_WRITE16_MEMBER(shareram1_w) {
		if (offset < 0x10) {
			int address = (rampage1 << 4) + offset;
			COMBINE_DATA(&shareram[address]);
		} else if (offset == 0x10) {
			rampage1 = data & 0x00FF;
		}
	}
	DECLARE_READ16_MEMBER(shareram2_r) {
		if (offset < 0x10) {
			int address = (rampage2 << 4) + offset;
			return shareram[address];
		}
		return 0xffff;
	}
	DECLARE_WRITE16_MEMBER(shareram2_w) {
		if (offset < 0x10) {
			int address = (rampage2 << 4) + offset;
			COMBINE_DATA(&shareram[address]);
		} else if (offset == 0x10) {
			rampage2 = data & 0x007F;
		}
	}

	DECLARE_DRIVER_INIT(gprider_double);

	UINT16 shareram[0x800];
	UINT16 rampage1;
	UINT16 rampage2;
};

//**************************************************************************
//  CONSTANTS
//**************************************************************************

const UINT32 MASTER_CLOCK = XTAL_50MHz;
const UINT32 SOUND_CLOCK = XTAL_16MHz;



//**************************************************************************
//  COMPARE/TIMER CHIP CALLBACKS
//**************************************************************************

//-------------------------------------------------
//  timer_ack_callback - acknowledge a timer chip
//  interrupt signal
//-------------------------------------------------

void segaxbd_state::timer_ack_callback()
{
	// clear the timer IRQ
	m_timer_irq_state = 0;
	update_main_irqs();
}


//-------------------------------------------------
//  sound_data_w - write data to the sound CPU
//-------------------------------------------------

void segaxbd_state::sound_data_w(UINT8 data)
{
	synchronize(TID_SOUND_WRITE, data);
}



//**************************************************************************
//  MAIN CPU READ/WRITE CALLBACKS
//**************************************************************************

//-------------------------------------------------
//  adc_w - handle reads from the ADC
//-------------------------------------------------

READ16_MEMBER( segaxbd_state::adc_r )
{
	static const char *const ports[] = { "ADC0", "ADC1", "ADC2", "ADC3", "ADC4", "ADC5", "ADC6", "ADC7" };

	// on the write, latch the selected input port and stash the value
	int which = (m_iochip_regs[0][2] >> 2) & 7;
	int value = read_safe(ioport(ports[which]), 0x0010);

	// reverse some port values
	if (m_adc_reverse[which])
		value = 255 - value;

	// return the previously latched value
	return value;
}


//-------------------------------------------------
//  adc_w - handle writes to the ADC
//-------------------------------------------------

WRITE16_MEMBER( segaxbd_state::adc_w )
{
}


//-------------------------------------------------
//  iochip_r - helper to handle I/O chip reads
//-------------------------------------------------

inline UINT16 segaxbd_state::iochip_r(int which, int port, int inputval)
{
	UINT16 result = m_iochip_regs[which][port];

	// if there's custom I/O, do that to get the input value
	if (!m_iochip_custom_io_r[which][port].isnull())
		inputval = m_iochip_custom_io_r[which][port](inputval);

	// for ports 0-3, the direction is controlled 4 bits at a time by register 6
	if (port <= 3)
	{
		if ((m_iochip_regs[which][6] >> (2 * port + 0)) & 1)
			result = (result & ~0x0f) | (inputval & 0x0f);
		if ((m_iochip_regs[which][6] >> (2 * port + 1)) & 1)
			result = (result & ~0xf0) | (inputval & 0xf0);
	}

	// for port 4, the direction is controlled 1 bit at a time by register 7
	else
	{
		if ((m_iochip_regs[which][7] >> 0) & 1)
			result = (result & ~0x01) | (inputval & 0x01);
		if ((m_iochip_regs[which][7] >> 1) & 1)
			result = (result & ~0x02) | (inputval & 0x02);
		if ((m_iochip_regs[which][7] >> 2) & 1)
			result = (result & ~0x04) | (inputval & 0x04);
		if ((m_iochip_regs[which][7] >> 3) & 1)
			result = (result & ~0x08) | (inputval & 0x08);
		result &= 0x0f;
	}
	return result;
}


//-------------------------------------------------
//  iochip_0_r - handle reads from the first I/O
//  chip
//-------------------------------------------------

READ16_MEMBER( segaxbd_state::iochip_0_r )
{
	switch (offset)
	{
		case 0:
			// Input port:
			//  D7: (Not connected)
			//  D6: /INTR of ADC0804
			//  D5-D0: CN C pin 24-19 (switch state 0= open, 1= closed)
			return iochip_r(0, 0, ioport("IO0PORTA")->read());

		case 1:
			// I/O port: CN C pins 17,15,13,11,9,7,5,3
			return iochip_r(0, 1, ioport("IO0PORTB")->read());

		case 2:
			// Output port
			return iochip_r(0, 2, 0);

		case 3:
			// Output port
			return iochip_r(0, 3, 0);

		case 4:
			// Unused
			return iochip_r(0, 4, 0);
	}

	// everything else returns 0
	return 0;
}


//-------------------------------------------------
//  iochip_0_w - handle writes to the first I/O
//  chip
//-------------------------------------------------

WRITE16_MEMBER( segaxbd_state::iochip_0_w )
{
	// access is via the low 8 bits
	if (!ACCESSING_BITS_0_7)
		return;

	data &= 0xff;

	// swap in the new value and remember the previous value
	UINT8 oldval = m_iochip_regs[0][offset];
	m_iochip_regs[0][offset] = data;

	// certain offsets have common effects
	switch (offset)
	{
		case 2:
			// Output port:
			//  D7: (Not connected)
			//  D6: (/WDC) - watchdog reset
			//  D5: Screen display (1= blanked, 0= displayed)
			//  D4-D2: (ADC2-0)
			//  D1: (CONT) - affects sprite hardware
			//  D0: Sound section reset (1= normal operation, 0= reset)
			if (((oldval ^ data) & 0x40) && !(data & 0x40))
				machine().watchdog_reset();

			m_segaic16vid->set_display_enable(data & 0x20);

			m_soundcpu->set_input_line(INPUT_LINE_RESET, (data & 0x01) ? CLEAR_LINE : ASSERT_LINE);
			if (m_soundcpu2 != NULL)
				m_soundcpu2->set_input_line(INPUT_LINE_RESET, (data & 0x01) ? CLEAR_LINE : ASSERT_LINE);
			break;

		case 3:
			// Output port:
			//  D7: Amplifier mute control (1= sounding, 0= muted)
			//  D6-D0: CN D pin A17-A23 (output level 1= high, 0= low) - usually set up as lamps and coincounter
			machine().sound().system_enable(data & 0x80);
			break;

		default:
			break;
	}

	// if there's custom I/O, handle that as well
	if (!m_iochip_custom_io_w[0][offset].isnull())
		m_iochip_custom_io_w[0][offset](data);
	else if (offset <= 4)
		logerror("I/O chip 0, port %c write = %02X\n", 'A' + offset, data);
}


//-------------------------------------------------
//  iochip_1_r - handle reads from the second I/O
//  chip
//-------------------------------------------------

READ16_MEMBER( segaxbd_state::iochip_1_r )
{
	switch (offset)
	{
		case 0:
			// Input port: switches, CN D pin A1-8 (switch state 1= open, 0= closed)
			return iochip_r(1, 0, ioport("IO1PORTA")->read());

		case 1:
			// Input port: switches, CN D pin A9-16 (switch state 1= open, 0= closed)
			return iochip_r(1, 1, ioport("IO1PORTB")->read());

		case 2:
			// Input port: DIP switches (1= off, 0= on)
			return iochip_r(1, 2, ioport("IO1PORTC")->read());

		case 3:
			// Input port: DIP switches (1= off, 0= on)
			return iochip_r(1, 3, ioport("IO1PORTD")->read());

		case 4:
			// Unused
			return iochip_r(1, 4, 0);
	}

	// everything else returns 0
	return 0;
}


//-------------------------------------------------
//  iochip_1_w - handle writes to the second I/O
//  chip
//-------------------------------------------------

WRITE16_MEMBER( segaxbd_state::iochip_1_w )
{
	// access is via the low 8 bits
	if (!ACCESSING_BITS_0_7)
		return;

	data &= 0xff;
	m_iochip_regs[1][offset] = data;

	// if there's custom I/O, handle that as well
	if (!m_iochip_custom_io_w[1][offset].isnull())
		m_iochip_custom_io_w[1][offset](data);
	else if (offset <= 4)
		logerror("I/O chip 1, port %c write = %02X\n", 'A' + offset, data);
}


//-------------------------------------------------
//  iocontrol_w - handle writes to the I/O control
//  port
//-------------------------------------------------

WRITE16_MEMBER( segaxbd_state::iocontrol_w )
{
	if (ACCESSING_BITS_0_7)
	{
		logerror("I/O chip force input = %d\n", data & 1);
		// Racing Hero and ABCop set this and fouls up their output ports
		//iochip_force_input = data & 1;
	}
}



//**************************************************************************
//  GAME-SPECIFIC MAIN CPU READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  loffire_sync0_w - force synchronization on
//  writes to this address for Line of Fire
//-------------------------------------------------

WRITE16_MEMBER( segaxbd_state::loffire_sync0_w )
{
	COMBINE_DATA(&m_loffire_sync[offset]);
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(10));
}


//-------------------------------------------------
//  rascot_excs_r - /EXCS region reads for Rascot
//-------------------------------------------------

READ16_MEMBER( segaxbd_state::rascot_excs_r )
{
	//logerror("%06X:rascot_excs_r(%04X)\n", m_maincpu->pc(), offset*2);

	// probably receives commands from the server here
	//return space.machine().rand() & 0xff;

	return 0xff;
}


//-------------------------------------------------
//  rascot_excs_w - /EXCS region writes for Rascot
//-------------------------------------------------

WRITE16_MEMBER( segaxbd_state::rascot_excs_w )
{
	//logerror("%06X:rascot_excs_w(%04X) = %04X & %04X\n", m_maincpu->pc(), offset*2, data, mem_mask);
}


//-------------------------------------------------
//  smgp_excs_r - /EXCS region reads for
//  Super Monaco GP
//-------------------------------------------------

READ16_MEMBER( segaxbd_state::smgp_excs_r )
{
	//logerror("%06X:smgp_excs_r(%04X)\n", m_maincpu->pc(), offset*2);
	return 0xffff;
}


//-------------------------------------------------
//  smgp_excs_w - /EXCS region writes for
//  Super Monaco GP
//-------------------------------------------------

WRITE16_MEMBER( segaxbd_state::smgp_excs_w )
{
	//logerror("%06X:smgp_excs_w(%04X) = %04X & %04X\n", m_maincpu->pc(), offset*2, data, mem_mask);
}



//**************************************************************************
//  SOUND Z80 CPU READ/WRITE CALLBACKS
//**************************************************************************

//-------------------------------------------------
//  sound_data_r - read latched sound data
//-------------------------------------------------

READ8_MEMBER( segaxbd_state::sound_data_r )
{
	m_soundcpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	return soundlatch_read();
}



//**************************************************************************
//  DRIVER OVERRIDES
//**************************************************************************

//-------------------------------------------------
//  device_timer - handle device timers
//-------------------------------------------------

void segaxbd_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TID_SOUND_WRITE:
			soundlatch_write(param);
			m_soundcpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);

			// if an extra sound board is attached, do an nmi there as well
			if (m_soundcpu2 != NULL)
				m_soundcpu2->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
			break;

		case TID_SCANLINE:
		{
			int scanline = param;
			int next_scanline = (scanline + 2) % 262;
			bool update = false;

			// clock the timer and set the IRQ if something happened
			if ((scanline % 2) != 0 && m_cmptimer_1->clock())
				m_timer_irq_state = 1, update = true;

			// set VBLANK on scanline 223
			if (scanline == 223)
			{
				m_vblank_irq_state = 1;
				update = true;
				m_subcpu->set_input_line(4, ASSERT_LINE);
				next_scanline = scanline + 1;
			}

			// clear VBLANK on scanline 224
			else if (scanline == 224)
			{
				m_vblank_irq_state = 0;
				update = true;
				m_subcpu->set_input_line(4, CLEAR_LINE);
				next_scanline = scanline + 1;
			}

			// update IRQs on the main CPU
			if (update)
				update_main_irqs();

			// come back in 2 scanlines
			m_scanline_timer->adjust(m_screen->time_until_pos(next_scanline), next_scanline);
			break;
		}
	}
}



//**************************************************************************
//  CUSTOM I/O HANDLERS
//**************************************************************************

//-------------------------------------------------
//  generic_iochip0_lamps_w - shared handler for
//  coin counters and lamps
//-------------------------------------------------

void segaxbd_state::generic_iochip0_lamps_w(UINT8 data)
{
	// d0: ?
	// d3: always 0?
	// d4: coin counter
	// d7: mute audio (always handled above)
	// other bits: lamps
	coin_counter_w(machine(), 0, (data >> 4) & 0x01);

	//
	//    aburner2:
	// d1: altitude warning lamp
	// d2: start lamp
	// d5: lock on lamp
	// d6: danger lamp
	// in clone aburner, lamps work only in testmode?

	output_set_lamp_value(0, (data >> 5) & 0x01);
	output_set_lamp_value(1, (data >> 6) & 0x01);
	output_set_lamp_value(2, (data >> 1) & 0x01);
	output_set_lamp_value(3, (data >> 2) & 0x01);
}


//-------------------------------------------------
//  aburner2_iochip0_motor_r - motor I/O reads
//  for Afterburner II
//-------------------------------------------------

UINT8 segaxbd_state::aburner2_iochip0_motor_r(UINT8 data)
{
	data &= 0xc0;

	// TODO
	return data | 0x3f;
}


//-------------------------------------------------
//  aburner2_iochip0_motor_w - motor I/O writes
//  for Afterburner II
//-------------------------------------------------

void segaxbd_state::aburner2_iochip0_motor_w(UINT8 data)
{
	// TODO
}


//-------------------------------------------------
//  smgp_iochip0_motor_r - motor I/O reads
//  for Super Monaco GP
//-------------------------------------------------

UINT8 segaxbd_state::smgp_iochip0_motor_r(UINT8 data)
{
	data &= 0xc0;

	// TODO
	return data | 0x0;
}


//-------------------------------------------------
//  smgp_iochip0_motor_w - motor I/O reads
//  for Super Monaco GP
//-------------------------------------------------

void segaxbd_state::smgp_iochip0_motor_w(UINT8 data)
{
	// TODO
}


//-------------------------------------------------
//  lastsurv_iochip1_port_r - muxed I/O reads
//  for Last Survivor
//-------------------------------------------------

UINT8 segaxbd_state::lastsurv_iochip1_port_r(UINT8 data)
{
	static const char * const port_names[] = { "MUX0", "MUX1", "MUX2", "MUX3" };
	return read_safe(ioport(port_names[m_lastsurv_mux]), 0xff);
}


//-------------------------------------------------
//  lastsurv_iochip0_muxer_w - muxed I/O writes
//  for Last Survivor
//-------------------------------------------------

void segaxbd_state::lastsurv_iochip0_muxer_w(UINT8 data)
{
	m_lastsurv_mux = (data >> 5) & 3;
	generic_iochip0_lamps_w(data & 0x9f);
}



//**************************************************************************
//  INTERNAL HELPERS
//**************************************************************************

//-------------------------------------------------
//  update_main_irqs - flush IRQ state to the
//  CPU device
//-------------------------------------------------

void segaxbd_state::update_main_irqs()
{
	UINT8 irq = 0;

	if (m_timer_irq_state)
		irq |= 2;
	else
		m_maincpu->set_input_line(2, CLEAR_LINE);

	if (m_vblank_irq_state)
		irq |= 4;
	else
		m_maincpu->set_input_line(4, CLEAR_LINE);

	if (m_gprider_hack && irq > 4)
		irq = 4;

	if (irq != 6)
		m_maincpu->set_input_line(6, CLEAR_LINE);

	if (irq)
	{
		m_maincpu->set_input_line(irq, ASSERT_LINE);
		machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(100));
	}
}


//-------------------------------------------------
//  m68k_reset_callback - callback for when the
//  main 68000 is reset
//-------------------------------------------------

WRITE_LINE_MEMBER(segaxbd_state::m68k_reset_callback)
{
	m_subcpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(100));
}


//-------------------------------------------------
//  palette_init - precompute weighted RGB values
//  for each input value 0-31
//-------------------------------------------------

void segaxbd_state::palette_init()
{
	//
	//  Color generation details
	//
	//  Each color is made up of 5 bits, connected through one or more resistors like so:
	//
	//  Bit 0 = 1 x 3.9K ohm
	//  Bit 1 = 1 x 2.0K ohm
	//  Bit 2 = 1 x 1.0K ohm
	//  Bit 3 = 2 x 1.0K ohm
	//  Bit 4 = 4 x 1.0K ohm
	//
	//  Another data bit is connected by a tristate buffer to the color output through a
	//  470 ohm resistor. The buffer allows the resistor to have no effect (tristate),
	//  halve brightness (pull-down) or double brightness (pull-up). The data bit source
	//  is bit 15 of each color RAM entry.
	//

	// compute weight table for regular palette entries
	static const int resistances_normal[6] = { 3900, 2000, 1000, 1000/2, 1000/4, 0   };
	double weights_normal[6];
	compute_resistor_weights(0, 255, -1.0,
		6, resistances_normal, weights_normal, 0, 0,
		0, NULL, NULL, 0, 0,
		0, NULL, NULL, 0, 0);

	// compute weight table for shadow/hilight palette entries
	static const int resistances_sh[6]     = { 3900, 2000, 1000, 1000/2, 1000/4, 470 };
	double weights_sh[6];
	compute_resistor_weights(0, 255, -1.0,
		6, resistances_sh, weights_sh, 0, 0,
		0, NULL, NULL, 0, 0,
		0, NULL, NULL, 0, 0);

	// compute R, G, B for each weight
	for (int value = 0; value < 32; value++)
	{
		int i4 = (value >> 4) & 1;
		int i3 = (value >> 3) & 1;
		int i2 = (value >> 2) & 1;
		int i1 = (value >> 1) & 1;
		int i0 = (value >> 0) & 1;
		m_palette_normal[value] = combine_6_weights(weights_normal, i0, i1, i2, i3, i4, 0);
		m_palette_shadow[value] = combine_6_weights(weights_sh, i0, i1, i2, i3, i4, 0);
		m_palette_hilight[value] = combine_6_weights(weights_sh, i0, i1, i2, i3, i4, 1);
	}
}


//-------------------------------------------------
//  paletteram_w - handle writes to palette RAM
//-------------------------------------------------

WRITE16_MEMBER( segaxbd_state::paletteram_w )
{
	// compute the number of entries
	if (m_palette_entries == 0)
		m_palette_entries = memshare("paletteram")->bytes() / 2;

	// get the new value
	UINT16 newval = m_paletteram[offset];
	COMBINE_DATA(&newval);
	m_paletteram[offset] = newval;

	//     byte 0    byte 1
	//  sBGR BBBB GGGG RRRR
	//  x000 4321 4321 4321
	int r = ((newval >> 12) & 0x01) | ((newval << 1) & 0x1e);
	int g = ((newval >> 13) & 0x01) | ((newval >> 3) & 0x1e);
	int b = ((newval >> 14) & 0x01) | ((newval >> 7) & 0x1e);

	// normal colors
	m_palette->set_pen_color(offset + 0 * m_palette_entries, m_palette_normal[r],  m_palette_normal[g],  m_palette_normal[b]);
	m_palette->set_pen_color(offset + 1 * m_palette_entries, m_palette_shadow[r],  m_palette_shadow[g],  m_palette_shadow[b]);
	m_palette->set_pen_color(offset + 2 * m_palette_entries, m_palette_hilight[r], m_palette_hilight[g], m_palette_hilight[b]);
}

//**************************************************************************
//  MAIN CPU ADDRESS MAPS
//**************************************************************************

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, segaxbd_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x3fffff)
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x083fff) AM_MIRROR(0x01c000) AM_RAM AM_SHARE("backup1")
	AM_RANGE(0x0a0000, 0x0a3fff) AM_MIRROR(0x01c000) AM_RAM AM_SHARE("backup2")
	AM_RANGE(0x0c0000, 0x0cffff) AM_DEVREADWRITE("segaic16vid", segaic16_video_device, tileram_r, tileram_w) AM_SHARE("tileram")
	AM_RANGE(0x0d0000, 0x0d0fff) AM_MIRROR(0x00f000) AM_DEVREADWRITE("segaic16vid", segaic16_video_device, textram_r, textram_w) AM_SHARE("textram")
	AM_RANGE(0x0e0000, 0x0e0007) AM_MIRROR(0x003ff8) AM_DEVREADWRITE("multiplier_main", sega_315_5248_multiplier_device, read, write)
	AM_RANGE(0x0e4000, 0x0e401f) AM_MIRROR(0x003fe0) AM_DEVREADWRITE("divider_main", sega_315_5249_divider_device, read, write)
	AM_RANGE(0x0e8000, 0x0e801f) AM_MIRROR(0x003fe0) AM_DEVREADWRITE("cmptimer_main", sega_315_5250_compare_timer_device, read, write)
	AM_RANGE(0x100000, 0x100fff) AM_MIRROR(0x00f000) AM_RAM AM_SHARE("sprites")
	AM_RANGE(0x110000, 0x11ffff) AM_DEVWRITE("sprites", sega_xboard_sprite_device, draw_write)
	AM_RANGE(0x120000, 0x123fff) AM_MIRROR(0x00c000) AM_RAM_WRITE(paletteram_w) AM_SHARE("paletteram")
	AM_RANGE(0x130000, 0x13ffff) AM_READWRITE(adc_r, adc_w)
	AM_RANGE(0x140000, 0x14000f) AM_MIRROR(0x00fff0) AM_READWRITE(iochip_0_r, iochip_0_w)
	AM_RANGE(0x150000, 0x15000f) AM_MIRROR(0x00fff0) AM_READWRITE(iochip_1_r, iochip_1_w)
	AM_RANGE(0x160000, 0x16ffff) AM_WRITE(iocontrol_w)
	AM_RANGE(0x200000, 0x27ffff) AM_ROM AM_REGION("subcpu", 0x00000)
	AM_RANGE(0x280000, 0x283fff) AM_MIRROR(0x01c000) AM_RAM AM_SHARE("subram0")
	AM_RANGE(0x2a0000, 0x2a3fff) AM_MIRROR(0x01c000) AM_RAM AM_SHARE("subram1")
	AM_RANGE(0x2e0000, 0x2e0007) AM_MIRROR(0x003ff8) AM_DEVREADWRITE("multiplier_subx", sega_315_5248_multiplier_device, read, write)
	AM_RANGE(0x2e4000, 0x2e401f) AM_MIRROR(0x003fe0) AM_DEVREADWRITE("divider_subx", sega_315_5249_divider_device, read, write)
	AM_RANGE(0x2e8000, 0x2e800f) AM_MIRROR(0x003ff0) AM_DEVREADWRITE("cmptimer_subx", sega_315_5250_compare_timer_device, read, write)
	AM_RANGE(0x2ec000, 0x2ecfff) AM_MIRROR(0x001000) AM_RAM AM_SHARE("roadram")
	AM_RANGE(0x2ee000, 0x2effff) AM_DEVREADWRITE("segaic16road", segaic16_road_device, segaic16_road_control_0_r, segaic16_road_control_0_w)
//  AM_RANGE(0x2f0000, 0x2f3fff) AM_READWRITE(excs_r, excs_w)
	AM_RANGE(0x3f8000, 0x3fbfff) AM_RAM AM_SHARE("backup1")
	AM_RANGE(0x3fc000, 0x3fffff) AM_RAM AM_SHARE("backup2")
ADDRESS_MAP_END

static ADDRESS_MAP_START( decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 16, segaxbd_state )
	AM_RANGE(0x00000, 0xfffff) AM_ROMBANK("fd1094_decrypted_opcodes")
ADDRESS_MAP_END

//**************************************************************************
//  SUB CPU ADDRESS MAPS
//**************************************************************************

static ADDRESS_MAP_START( sub_map, AS_PROGRAM, 16, segaxbd_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xfffff)
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x083fff) AM_MIRROR(0x01c000) AM_RAM AM_SHARE("subram0")
	AM_RANGE(0x0a0000, 0x0a3fff) AM_MIRROR(0x01c000) AM_RAM AM_SHARE("subram1")
	AM_RANGE(0x0e0000, 0x0e0007) AM_MIRROR(0x003ff8) AM_DEVREADWRITE("multiplier_subx", sega_315_5248_multiplier_device, read, write)
	AM_RANGE(0x0e4000, 0x0e401f) AM_MIRROR(0x003fe0) AM_DEVREADWRITE("divider_subx", sega_315_5249_divider_device, read, write)
	AM_RANGE(0x0e8000, 0x0e800f) AM_MIRROR(0x003ff0) AM_DEVREADWRITE("cmptimer_subx", sega_315_5250_compare_timer_device, read, write)
	AM_RANGE(0x0ec000, 0x0ecfff) AM_MIRROR(0x001000) AM_RAM AM_SHARE("roadram")
	AM_RANGE(0x0ee000, 0x0effff) AM_DEVREADWRITE("segaic16road", segaic16_road_device, segaic16_road_control_0_r, segaic16_road_control_0_w)
//  AM_RANGE(0x0f0000, 0x0f3fff) AM_READWRITE(excs_r, excs_w)
ADDRESS_MAP_END



//**************************************************************************
//  Z80 SOUND CPU ADDRESS MAPS
//**************************************************************************

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, segaxbd_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf0ff) AM_MIRROR(0x0700) AM_DEVREADWRITE("pcm", segapcm_device, sega_pcm_r, sega_pcm_w)
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_portmap, AS_IO, 8, segaxbd_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_MIRROR(0x3e) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE(0x40, 0x40) AM_MIRROR(0x3f) AM_READ(sound_data_r)
ADDRESS_MAP_END



//**************************************************************************
//  SUPER MONACO GP 2ND SOUND CPU ADDRESS MAPS
//**************************************************************************

// Sound Board
// The extra sound is used when the cabinet is Deluxe(Air Drive), or Cockpit. The soundlatch is
// shared with the main board sound.
static ADDRESS_MAP_START( smgp_sound2_map, AS_PROGRAM, 8, segaxbd_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf0ff) AM_MIRROR(0x0700) AM_DEVREADWRITE("pcm2", segapcm_device, sega_pcm_r, sega_pcm_w)
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( smgp_sound2_portmap, AS_IO, 8, segaxbd_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x40, 0x40) AM_MIRROR(0x3f) AM_READ(sound_data_r)
ADDRESS_MAP_END



//**************************************************************************
//  SUPER MONACO GP MOTOR BOARD CPU ADDRESS MAPS
//**************************************************************************

// Motor Board, not yet emulated
static ADDRESS_MAP_START( smgp_airdrive_map, AS_PROGRAM, 8, segaxbd_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xafff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( smgp_airdrive_portmap, AS_IO, 8, segaxbd_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01, 0x01) AM_READNOP
	AM_RANGE(0x02, 0x03) AM_NOP
ADDRESS_MAP_END



//**************************************************************************
//  SUPER MONACO GP LINK BOARD CPU ADDRESS MAPS
//**************************************************************************

// Link Board, not yet emulated
static ADDRESS_MAP_START( smgp_comm_map, AS_PROGRAM, 8, segaxbd_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x3fff) AM_RAM
	AM_RANGE(0x4000, 0x47ff) AM_RAM // MB8421 Dual-Port SRAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( smgp_comm_portmap, AS_IO, 8, segaxbd_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END



//**************************************************************************
//  RASCOT UNKNOWN Z80 CPU ADDRESS MAPS
//**************************************************************************

// Z80, unknown function
static ADDRESS_MAP_START( rascot_z80_map, AS_PROGRAM, 8, segaxbd_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xafff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( rascot_z80_portmap, AS_IO, 8, segaxbd_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END



//**************************************************************************
//  GENERIC PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( xboard_generic )
	PORT_START("mainpcb:IO0PORTA")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL )    // /INTR of ADC0804
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("mainpcb:IO0PORTB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("mainpcb:IO1PORTA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    // button? not used by any game we have
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    // cannon trigger or shift down
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    // missile button or shift up
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("mainpcb:IO1PORTB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("mainpcb:IO1PORTC")
	SEGA_COINAGE_LOC(SWA)

	PORT_START("mainpcb:IO1PORTD")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SWB:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SWB:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SWB:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SWB:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SWB:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SWB:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SWB:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SWB:8" )
INPUT_PORTS_END



//**************************************************************************
//  GAME-SPECIFIC PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( aburner )
	PORT_INCLUDE( xboard_generic )

	PORT_MODIFY("mainpcb:IO1PORTA")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Vulcan")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Missile")

	PORT_MODIFY("mainpcb:IO1PORTD")
	PORT_DIPNAME( 0x03, 0x01, "Cabinet Type" ) PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x03, "Moving Deluxe" )
	PORT_DIPSETTING(    0x02, "Moving Standard" )
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Unused ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	// According to the manual, SWB:4 sets 3 or 4 lives, but it doesn't actually do that.
	// However, it does on Afterburner II.  Maybe there's another version of Afterburner
	// that behaves as the manual suggests.
	// In the Japanese manual "DIP SW B:4 / NOT USED"
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "3x Credits" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START("mainpcb:ADC0")  // stick X
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x20,0xe0) PORT_SENSITIVITY(100) PORT_KEYDELTA(4)

	PORT_START("mainpcb:ADC1")  // stick Y
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x40,0xc0) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_REVERSE

	PORT_START("mainpcb:ADC2")  // throttle
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Z ) PORT_SENSITIVITY(100) PORT_KEYDELTA(79)

	PORT_START("mainpcb:ADC3")  // motor Y
	PORT_BIT( 0xff, (0xb0+0x50)/2, IPT_SPECIAL )

	PORT_START("mainpcb:ADC4")  // motor X
	PORT_BIT( 0xff, (0xb0+0x50)/2, IPT_SPECIAL )
INPUT_PORTS_END


static INPUT_PORTS_START( aburner2 )
	PORT_INCLUDE( aburner )

	PORT_MODIFY("mainpcb:IO1PORTD")
	PORT_DIPNAME( 0x03, 0x01, "Cabinet Type" ) PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x03, "Moving Deluxe" )
	PORT_DIPSETTING(    0x02, "Moving Standard" )
	PORT_DIPSETTING(    0x01, "Upright 1" )
	PORT_DIPSETTING(    0x00, "Upright 2" )
	PORT_DIPNAME( 0x04, 0x04, "Throttle Lever" ) PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWB:4,5")
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x08, "3x Credits" )
	PORT_DIPSETTING(    0x00, "4x Credits" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END


static INPUT_PORTS_START( thndrbld )
	PORT_INCLUDE( xboard_generic )

	PORT_MODIFY("mainpcb:IO1PORTA")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Cannon")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Missile")

	PORT_MODIFY("mainpcb:IO1PORTD")
	PORT_DIPNAME( 0x01, 0x01, "Cabinet Type" ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x01, "Econ Upright" )
	PORT_DIPSETTING(    0x00, "Mini Upright" )  // see note about inputs below
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Time" ) PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x04, "30 sec" )
	PORT_DIPSETTING(    0x00, "0 sec" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWB:4,5")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	//  These inputs are valid for the "Econ Upright" and "Deluxe" cabinets.
	//  On the "Standing" cabinet, the joystick Y axis is reversed.
	//  On the "Mini Upright" cabinet, the inputs conform to After Burner II:
	//  the X axis is (un-)reversed, and the throttle and Y axis switch places
	PORT_START("mainpcb:ADC0")  // stick X
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x01,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_REVERSE

	PORT_START("mainpcb:ADC1")  // "slottle"
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Z ) PORT_SENSITIVITY(100) PORT_KEYDELTA(79)

	PORT_START("mainpcb:ADC2")  // stick Y
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x01,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(4)
INPUT_PORTS_END


static INPUT_PORTS_START( thndrbd1 )
	PORT_INCLUDE( thndrbld )

	PORT_MODIFY("mainpcb:IO1PORTD")
	PORT_DIPNAME( 0x01, 0x01, "Cabinet Type" ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x01, "Deluxe" )
	PORT_DIPSETTING(    0x00, "Standing" )  // see note about inputs above
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Time" ) PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x04, "30 sec" )
	PORT_DIPSETTING(    0x00, "0 sec" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWB:4,5")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END


static const ioport_value lastsurv_position_table[] =
{
	0x0f ^ 0x08 ^ 0x01,     // down + left
	0x0f ^ 0x01,            // left
	0x0f ^ 0x04 ^ 0x01,     // up + left
	0x0f ^ 0x04,            // up
	0x0f ^ 0x04 ^ 0x02,     // up + right
	0x0f ^ 0x02,            // right
	0x0f ^ 0x08 ^ 0x02,     // down + right
	0x0f ^ 0x08,            // down
};
static INPUT_PORTS_START( lastsurv )
	PORT_INCLUDE( xboard_generic )

	PORT_MODIFY("mainpcb:IO1PORTA")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )

	PORT_START("mainpcb:MUX0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0xf0, 0xf0 ^ 0x40, IPT_POSITIONAL ) PORT_PLAYER(2) PORT_POSITIONS(8) PORT_REMAP_TABLE(lastsurv_position_table) PORT_WRAPS PORT_SENSITIVITY(1) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_CODE_DEC(KEYCODE_Q) PORT_CODE_INC(KEYCODE_W)

	PORT_START("mainpcb:MUX1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0xf0, 0xf0 ^ 0x40, IPT_POSITIONAL ) PORT_PLAYER(1) PORT_POSITIONS(8) PORT_REMAP_TABLE(lastsurv_position_table) PORT_WRAPS PORT_SENSITIVITY(1) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X)

	PORT_START("mainpcb:MUX2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("mainpcb:MUX3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("mainpcb:IO1PORTD")
	PORT_DIPNAME( 0x03, 0x03, "I.D. No" ) PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x0c, 0x0c, "Network" ) PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x0c, "Off" )
	PORT_DIPSETTING(    0x08, "On (2)" )
	PORT_DIPSETTING(    0x04, "On (4)" )
//  PORT_DIPSETTING(    0x00, "No Use" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin Chute" ) PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, "Single" )
	PORT_DIPSETTING(    0x00, "Twin" )
INPUT_PORTS_END


static INPUT_PORTS_START( loffire )
	PORT_INCLUDE( xboard_generic )

	PORT_MODIFY("mainpcb:IO1PORTA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("mainpcb:IO1PORTB")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_MODIFY("mainpcb:IO1PORTD")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Japanese ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, "Cockpit" )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPNAME( 0x04, 0x04, "2 Credits to Start" ) PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:4,5")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin Chute" ) PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, "Twin" )

	PORT_START("mainpcb:ADC0")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(5)

	PORT_START("mainpcb:ADC1")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(5)

	PORT_START("mainpcb:ADC2")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_PLAYER(2)

	PORT_START("mainpcb:ADC3")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( rachero )
	PORT_INCLUDE( xboard_generic )

	PORT_MODIFY("mainpcb:IO1PORTA")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Move to Center")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Suicide")

	PORT_MODIFY("mainpcb:IO1PORTD")
	PORT_DIPNAME( 0x01, 0x01, "Credits" ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x01, "1 to Start, 1 to Continue" )
	PORT_DIPSETTING(    0x00, "2 to Start, 1 to Continue" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x30, 0x30, "Time" )  PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )

	PORT_START("mainpcb:ADC0")  // steering
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x20,0xe0) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_REVERSE

	PORT_START("mainpcb:ADC1")  // gas pedal
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20)

	PORT_START("mainpcb:ADC2")  // brake
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(100) PORT_KEYDELTA(40)
INPUT_PORTS_END


static INPUT_PORTS_START( smgp )
	PORT_INCLUDE( xboard_generic )

	PORT_MODIFY("mainpcb:IO1PORTA")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Shift Down") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Shift Up") PORT_CODE(KEYCODE_Z)

	PORT_MODIFY("mainpcb:IO1PORTD")
	PORT_DIPNAME( 0x07, 0x07, "Machine ID" ) PORT_DIPLOCATION("SWB:1,2,3")
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPNAME( 0x38, 0x38, "Number of Machines" ) PORT_DIPLOCATION("SWB:4,5,6")
	PORT_DIPSETTING(    0x38, "1" )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x28, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x10, "6" )
	PORT_DIPSETTING(    0x08, "7" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(    0xc0, "Deluxe" )
	PORT_DIPSETTING(    0x80, "Cockpit" )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
//  PORT_DIPSETTING(    0x00, "Deluxe" )

	PORT_START("mainpcb:ADC0")  // steering
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x38,0xc8) PORT_SENSITIVITY(100) PORT_KEYDELTA(4)

	PORT_START("mainpcb:ADC1")  // gas pedal
	PORT_BIT( 0xff, 0x38, IPT_PEDAL ) PORT_MINMAX(0x38,0xb8) PORT_SENSITIVITY(100) PORT_KEYDELTA(20)

	PORT_START("mainpcb:ADC2")  // brake
	PORT_BIT( 0xff, 0x28, IPT_PEDAL2 ) PORT_MINMAX(0x28,0xa8) PORT_SENSITIVITY(100) PORT_KEYDELTA(40)
INPUT_PORTS_END


static INPUT_PORTS_START( abcop )
	PORT_INCLUDE( xboard_generic )

	PORT_MODIFY("mainpcb:IO1PORTA")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Jump")

	PORT_MODIFY("mainpcb:IO1PORTD")
	PORT_DIPNAME( 0x01, 0x01, "Credits" ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x01, "1 to Start, 1 to Continue" )
	PORT_DIPSETTING(    0x00, "2 to Start, 1 to Continue" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x30, 0x30, "Time" ) PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START("mainpcb:ADC0")  // steering
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x20,0xe0) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_REVERSE

	PORT_START("mainpcb:ADC1")  // accelerator
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20)
INPUT_PORTS_END


static INPUT_PORTS_START( gprider )
	PORT_START("mainpcb:IO0PORTA")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL )    // /INTR of ADC0804
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("mainpcb:IO0PORTB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("mainpcb:IO1PORTA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    // button? not used by any game we have
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Shift Down") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Shift Up") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("mainpcb:IO1PORTB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("mainpcb:IO1PORTC")
	SEGA_COINAGE_LOC(SWA)

	PORT_START("mainpcb:IO1PORTD")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x03, "Ride On" )
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
//  PORT_DIPSETTING(    0x01, DEF_STR( Unused ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Unused ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SWB:3" )
	PORT_DIPNAME( 0x08, 0x08, "ID No." ) PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x08, "Main" ) // Player 1 (Blue)
	PORT_DIPSETTING(    0x00, "Slave" ) // Player 2 (Red)
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SWB:6" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START("mainpcb:ADC0")  // steering
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x01,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(4)

	PORT_START("mainpcb:ADC1")  // gas pedal
	PORT_BIT( 0xff, 0x10, IPT_PEDAL ) PORT_MINMAX(0x10,0xef) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_REVERSE

	PORT_START("mainpcb:ADC2")  // brake
	PORT_BIT( 0xff, 0x10, IPT_PEDAL2 ) PORT_MINMAX(0x10,0xef) PORT_SENSITIVITY(100) PORT_KEYDELTA(40) PORT_REVERSE
INPUT_PORTS_END

static INPUT_PORTS_START( gprider_double )
	PORT_INCLUDE( gprider )

	PORT_START("subpcb:IO0PORTA")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL )    // /INTR of ADC0804
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("subpcb:IO0PORTB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("subpcb:IO1PORTA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )    // button? not used by any game we have
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Shift Down") PORT_CODE(KEYCODE_W) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Shift Up") PORT_CODE(KEYCODE_S) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4 )

	PORT_START("subpcb:IO1PORTB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("subpcb:IO1PORTC")
	SEGA_COINAGE_LOC(SWA)

	PORT_START("subpcb:IO1PORTD")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x03, "Ride On" )
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
//  PORT_DIPSETTING(    0x01, DEF_STR( Unused ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Unused ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SWB:3" )
	PORT_DIPNAME( 0x08, 0x00, "ID No." ) PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x08, "Main" ) // Player 1 (Blue)
	PORT_DIPSETTING(    0x00, "Slave" ) // Player 2 (Red)
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SWB:6" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START("subpcb:ADC0")  // steering
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x01,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_PLAYER(2)

	PORT_START("subpcb:ADC1")  // gas pedal
	PORT_BIT( 0xff, 0x10, IPT_PEDAL ) PORT_MINMAX(0x10,0xef) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("subpcb:ADC2")  // brake
	PORT_BIT( 0xff, 0x10, IPT_PEDAL2 ) PORT_MINMAX(0x10,0xef) PORT_SENSITIVITY(100) PORT_KEYDELTA(40) PORT_REVERSE PORT_PLAYER(2)

INPUT_PORTS_END


static INPUT_PORTS_START( rascot )
	PORT_INCLUDE( xboard_generic )

INPUT_PORTS_END



//**************************************************************************
//  GRAPHICS DEFINITIONS
//**************************************************************************

static GFXDECODE_START( segaxbd )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x3_planar, 0, 1024 )
GFXDECODE_END



//**************************************************************************
//  GENERIC MACHINE DRIVERS
//**************************************************************************

static MACHINE_CONFIG_FRAGMENT( xboard )

	// basic machine hardware
	MCFG_CPU_ADD("maincpu", M68000, MASTER_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(main_map)

	MCFG_CPU_ADD("subcpu", M68000, MASTER_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(sub_map)

	MCFG_CPU_ADD("soundcpu", Z80, SOUND_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_portmap)

	MCFG_NVRAM_ADD_0FILL("backup1")
	MCFG_NVRAM_ADD_0FILL("backup2")
	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	MCFG_SEGA_315_5248_MULTIPLIER_ADD("multiplier_main")
	MCFG_SEGA_315_5248_MULTIPLIER_ADD("multiplier_subx")
	MCFG_SEGA_315_5249_DIVIDER_ADD("divider_main")
	MCFG_SEGA_315_5249_DIVIDER_ADD("divider_subx")

	MCFG_SEGA_315_5250_COMPARE_TIMER_ADD("cmptimer_main")
	MCFG_SEGA_315_5250_TIMER_ACK(segaxbd_state, timer_ack_callback)
	MCFG_SEGA_315_5250_SOUND_WRITE(segaxbd_state, sound_data_w)

	MCFG_SEGA_315_5250_COMPARE_TIMER_ADD("cmptimer_subx")

	// video hardware
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", segaxbd)
	MCFG_PALETTE_ADD("palette", 8192*3)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK/8, 400, 0, 320, 262, 0, 224)
	MCFG_SCREEN_UPDATE_DRIVER(segaxbd_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_SEGA_XBOARD_SPRITES_ADD("sprites")
	MCFG_SEGAIC16VID_ADD("segaic16vid")
	MCFG_SEGAIC16VID_GFXDECODE("gfxdecode")
	MCFG_VIDEO_SET_SCREEN("screen")

	MCFG_SEGAIC16_ROAD_ADD("segaic16road")

	// sound hardware
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", SOUND_CLOCK/4)
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("soundcpu", 0))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.43)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.43)

	MCFG_SEGAPCM_ADD("pcm", SOUND_CLOCK/4)
	MCFG_SEGAPCM_BANK(BANK_512)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

const device_type SEGA_XBD_REGULAR_DEVICE = &device_creator<segaxbd_regular_state>;

segaxbd_regular_state::segaxbd_regular_state(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: segaxbd_state(mconfig, tag, owner, clock)
{
}

machine_config_constructor segaxbd_regular_state::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( xboard );
}

static MACHINE_CONFIG_START( sega_xboard, segaxbd_new_state )
	MCFG_DEVICE_ADD("mainpcb", SEGA_XBD_REGULAR_DEVICE, 0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_FRAGMENT( xboard_fd1094 )

	MCFG_FRAGMENT_ADD( xboard )

	MCFG_CPU_REPLACE("maincpu", FD1094, MASTER_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
MACHINE_CONFIG_END

const device_type SEGA_XBD_FD1094_DEVICE = &device_creator<segaxbd_fd1094_state>;

segaxbd_fd1094_state::segaxbd_fd1094_state(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: segaxbd_state(mconfig, tag, owner, clock)
{
}

machine_config_constructor segaxbd_fd1094_state::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( xboard_fd1094 );
}

static MACHINE_CONFIG_START( sega_xboard_fd1094, segaxbd_new_state )
	MCFG_DEVICE_ADD("mainpcb", SEGA_XBD_FD1094_DEVICE, 0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( sega_xboard_fd1094_double, segaxbd_new_state_double )
	MCFG_DEVICE_ADD("mainpcb", SEGA_XBD_FD1094_DEVICE, 0)
	MCFG_DEVICE_ADD("subpcb", SEGA_XBD_FD1094_DEVICE, 0)

	//MCFG_QUANTUM_PERFECT_CPU("mainpcb:maincpu") // doesn't help..
MACHINE_CONFIG_END


//**************************************************************************
//  GAME-SPECIFIC MACHINE DRIVERS
//**************************************************************************

static MACHINE_CONFIG_FRAGMENT( lastsurv_fd1094 )

	MCFG_FRAGMENT_ADD( xboard_fd1094 )

	// basic machine hardware
	// TODO: network board

	// sound hardware - ym2151 stereo is reversed
	MCFG_SOUND_MODIFY("ymsnd")
	MCFG_SOUND_ROUTES_RESET()
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.43)
	MCFG_SOUND_ROUTE(1, "lspeaker", 0.43)
MACHINE_CONFIG_END

const device_type SEGA_XBD_LASTSURV_FD1094_DEVICE = &device_creator<segaxbd_lastsurv_fd1094_state>;

segaxbd_lastsurv_fd1094_state::segaxbd_lastsurv_fd1094_state(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: segaxbd_state(mconfig, tag, owner, clock)
{
}

machine_config_constructor segaxbd_lastsurv_fd1094_state::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( lastsurv_fd1094 );
}

static MACHINE_CONFIG_START( sega_lastsurv_fd1094, segaxbd_new_state )
	MCFG_DEVICE_ADD("mainpcb", SEGA_XBD_LASTSURV_FD1094_DEVICE, 0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( lastsurv )

	MCFG_FRAGMENT_ADD( xboard )

	// basic machine hardware
	// TODO: network board

	// sound hardware - ym2151 stereo is reversed
	MCFG_SOUND_MODIFY("ymsnd")
	MCFG_SOUND_ROUTES_RESET()
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.43)
	MCFG_SOUND_ROUTE(1, "lspeaker", 0.43)
MACHINE_CONFIG_END

const device_type SEGA_XBD_LASTSURV_DEVICE = &device_creator<segaxbd_lastsurv_state>;

segaxbd_lastsurv_state::segaxbd_lastsurv_state(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: segaxbd_state(mconfig, tag, owner, clock)
{
}

machine_config_constructor segaxbd_lastsurv_state::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( lastsurv );
}

static MACHINE_CONFIG_START( sega_lastsurv, segaxbd_new_state )
	MCFG_DEVICE_ADD("mainpcb", SEGA_XBD_LASTSURV_DEVICE, 0)
MACHINE_CONFIG_END



static MACHINE_CONFIG_FRAGMENT( smgp_fd1094 )
	MCFG_FRAGMENT_ADD( xboard_fd1094 )

	// basic machine hardware
	MCFG_CPU_ADD("soundcpu2", Z80, SOUND_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(smgp_sound2_map)
	MCFG_CPU_IO_MAP(smgp_sound2_portmap)

	MCFG_CPU_ADD("commcpu", Z80, XTAL_16MHz/2) // Z80E
	MCFG_CPU_PROGRAM_MAP(smgp_comm_map)
	MCFG_CPU_IO_MAP(smgp_comm_portmap)

	MCFG_CPU_ADD("motorcpu", Z80, XTAL_16MHz/2) // not verified
	MCFG_CPU_PROGRAM_MAP(smgp_airdrive_map)
	MCFG_CPU_IO_MAP(smgp_airdrive_portmap)

	// sound hardware
	MCFG_SPEAKER_STANDARD_STEREO("rearleft", "rearright")

	MCFG_SEGAPCM_ADD("pcm2", SOUND_CLOCK/4)
	MCFG_SEGAPCM_BANK(BANK_512)
	MCFG_SOUND_ROUTE(0, "rearleft", 1.0)
	MCFG_SOUND_ROUTE(1, "rearright", 1.0)
MACHINE_CONFIG_END

const device_type SEGA_XBD_SMGP_FD1094_DEVICE = &device_creator<segaxbd_smgp_fd1094_state>;

segaxbd_smgp_fd1094_state::segaxbd_smgp_fd1094_state(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: segaxbd_state(mconfig, tag, owner, clock)
{
}

machine_config_constructor segaxbd_smgp_fd1094_state::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( smgp_fd1094 );
}

static MACHINE_CONFIG_START( sega_smgp_fd1094, segaxbd_new_state )
	MCFG_DEVICE_ADD("mainpcb", SEGA_XBD_SMGP_FD1094_DEVICE, 0)
MACHINE_CONFIG_END



static MACHINE_CONFIG_FRAGMENT( smgp )
	MCFG_FRAGMENT_ADD( xboard )

	// basic machine hardware
	MCFG_CPU_ADD("soundcpu2", Z80, SOUND_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(smgp_sound2_map)
	MCFG_CPU_IO_MAP(smgp_sound2_portmap)

	MCFG_CPU_ADD("commcpu", Z80, XTAL_16MHz/2) // Z80E
	MCFG_CPU_PROGRAM_MAP(smgp_comm_map)
	MCFG_CPU_IO_MAP(smgp_comm_portmap)

	MCFG_CPU_ADD("motorcpu", Z80, XTAL_16MHz/2) // not verified
	MCFG_CPU_PROGRAM_MAP(smgp_airdrive_map)
	MCFG_CPU_IO_MAP(smgp_airdrive_portmap)

	// sound hardware
	MCFG_SPEAKER_STANDARD_STEREO("rearleft", "rearright")

	MCFG_SEGAPCM_ADD("pcm2", SOUND_CLOCK/4)
	MCFG_SEGAPCM_BANK(BANK_512)
	MCFG_SOUND_ROUTE(0, "rearleft", 1.0)
	MCFG_SOUND_ROUTE(1, "rearright", 1.0)
MACHINE_CONFIG_END

const device_type SEGA_XBD_SMGP_DEVICE = &device_creator<segaxbd_smgp_state>;

segaxbd_smgp_state::segaxbd_smgp_state(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: segaxbd_state(mconfig, tag, owner, clock)
{
}

machine_config_constructor segaxbd_smgp_state::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( smgp );
}

static MACHINE_CONFIG_START( sega_smgp, segaxbd_new_state )
	MCFG_DEVICE_ADD("mainpcb", SEGA_XBD_SMGP_DEVICE, 0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_FRAGMENT( rascot  )
	MCFG_FRAGMENT_ADD( xboard )

	// basic machine hardware
	MCFG_CPU_MODIFY("soundcpu")
	MCFG_CPU_PROGRAM_MAP(rascot_z80_map)
	MCFG_CPU_IO_MAP(rascot_z80_portmap)
MACHINE_CONFIG_END

const device_type SEGA_XBD_RASCOT_DEVICE = &device_creator<segaxbd_rascot_state>;

segaxbd_rascot_state::segaxbd_rascot_state(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: segaxbd_state(mconfig, tag, owner, clock)
{
}

machine_config_constructor segaxbd_rascot_state::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( rascot );
}

static MACHINE_CONFIG_START( sega_rascot, segaxbd_new_state )
	MCFG_DEVICE_ADD("mainpcb", SEGA_XBD_RASCOT_DEVICE, 0)
MACHINE_CONFIG_END


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************


//*************************************************************************************************************************
//*************************************************************************************************************************
//*************************************************************************************************************************
//  Afterburner, Sega X-board
//  CPU: 68000 (317-????)
//
//  Missing the Deluxe/Upright English (US?) version rom set
//   Program roms:
//     EPR-11092.58
//     EPR-11093.63
//     EPR-10950.57
//     EPR-10951.62
//   Sub-Program
//     EPR-11090.30
//     EPR-11091.20
//   Fix Scroll Character
//     EPR-11089.154
//     EPR-11088.153
//     EPR-11087.152
//   Object (Character & Scene Scenery)
//     EPR-11098.93
//     EPR-11099.97
//     EPR-11100.101
//     EPR-11101.105
//     EPR-11094.92--
//     EPR-11095.96  \ These 4 found in Afterburner II (German)??
//     EPR-11096.100 /
//     EPR-11097.104-
//   Sound Data
//     EPR-10929.13
//
ROM_START( aburner )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-10940.58", 0x00000, 0x20000, CRC(4d132c4e) SHA1(007af52167c369177b86fc0f8b007ebceba2a30c) )
	ROM_LOAD16_BYTE( "epr-10941.63", 0x00001, 0x20000, CRC(136ea264) SHA1(606ac67db53a6002ed1bd71287aed2e3e720cdf4) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-10927.20", 0x00000, 0x20000, CRC(66d36757) SHA1(c7f6d653fb6bfd629bb62057010d41f3ccfccc4d) )
	ROM_LOAD16_BYTE( "epr-10928.29", 0x00001, 0x20000, CRC(7c01d40b) SHA1(d95b4702a9c813db8bc24c8cd7e0933cbe54a573) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-10926.154", 0x00000, 0x10000, CRC(ed8bd632) SHA1(d5bbd5e257ebef8cfb3baf5fa530b189d9cddb57) )
	ROM_LOAD( "epr-10925.153", 0x10000, 0x10000, CRC(4ef048cc) SHA1(3b386b3bfa600f114dbc19796bb6864a88ff4562) )
	ROM_LOAD( "epr-10924.152", 0x20000, 0x10000, CRC(50c15a6d) SHA1(fc202cc583fc6804647abc884fdf332e72ea3100) )

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "mpr-10932.90",  0x000000, 0x20000, CRC(cc0821d6) SHA1(22e84419a585209bbda1466d2180504c316a9b7f) ) // First 8 roms are MPR, the rest are EPR
	ROM_LOAD32_BYTE( "mpr-10934.94",  0x000001, 0x20000, CRC(4a51b1fa) SHA1(2eed018a5a1e935bb72b6f440a794466a1397dc5) )
	ROM_LOAD32_BYTE( "mpr-10936.98",  0x000002, 0x20000, CRC(ada70d64) SHA1(ba6203b0fdb4c4998b7be5b446eb8354751d553a) )
	ROM_LOAD32_BYTE( "mpr-10938.102", 0x000003, 0x20000, CRC(e7675baf) SHA1(aa979319a44c0b18c462afb5ca9cdeed2292c76a) )
	ROM_LOAD32_BYTE( "mpr-10933.91",  0x080000, 0x20000, CRC(c8efb2c3) SHA1(ba31da93f929f2c457e60b2099d5a1ba6b5a9f48) )
	ROM_LOAD32_BYTE( "mpr-10935.95",  0x080001, 0x20000, CRC(c1e23521) SHA1(5e95f3b6ff9f4caca676eaa6c84f1200315218ea) )
	ROM_LOAD32_BYTE( "mpr-10937.99",  0x080002, 0x20000, CRC(f0199658) SHA1(cd67504fef53f637a3b1c723c4a04148f88028d2) )
	ROM_LOAD32_BYTE( "mpr-10939.103", 0x080003, 0x20000, CRC(a0d49480) SHA1(6c4234456bc09ae771beec284d7aa21ebe474f6f) )
	ROM_LOAD32_BYTE( "epr-10942.92",  0x100000, 0x20000, CRC(5ce10b8c) SHA1(c6c189143762b0ef473d5d31d66226820c5cf080) )
	ROM_LOAD32_BYTE( "epr-10943.96",  0x100001, 0x20000, CRC(b98294dc) SHA1(a4161af23f9a67b4ed81308c73e72e1797cce894) )
	ROM_LOAD32_BYTE( "epr-10944.100", 0x100002, 0x20000, CRC(17be8f67) SHA1(371f0dd1914a98695cb86f921fe8e82b49e69a4a) )
	ROM_LOAD32_BYTE( "epr-10945.104", 0x100003, 0x20000, CRC(df4d4c4f) SHA1(24075a6709869d9acf9082b6b4ad96bc6f8b1932) )
	ROM_LOAD32_BYTE( "epr-10946.93",  0x180000, 0x20000, CRC(d7d485f4) SHA1(d843aefb4d99e0dff8d62ee6bd0c3aa6aa6c941b) )
	ROM_LOAD32_BYTE( "epr-10947.97",  0x180001, 0x20000, CRC(08838392) SHA1(84f7ff3bff31c0738dead7bc00219ede834eb0e0) )
	ROM_LOAD32_BYTE( "epr-10948.101", 0x180002, 0x20000, CRC(64284761) SHA1(9594c671900f7f49d8fb965bc17b4380ce2c68d5) )
	ROM_LOAD32_BYTE( "epr-10949.105", 0x180003, 0x20000, CRC(d8437d92) SHA1(480291358c3d197645d7bd149bdfe5d41071d52d) )

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	ROM_LOAD( "epr-10922.40", 0x000000, 0x10000, CRC(b49183d4) SHA1(71d87bfbce858049ccde9597ab15575b3cdba892) )

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-10923.17", 0x00000, 0x10000, CRC(6888eb8f) SHA1(8f8fffb214842a5d356e33f5a97099bc6407384f) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "mpr-10931.11", 0x00000, 0x20000, CRC(9209068f) SHA1(01f3dda1c066d00080c55f2c86c506b6b2407f98) )
	ROM_LOAD( "mpr-10930.12", 0x20000, 0x20000, CRC(6493368b) SHA1(328aff19ff1d1344e9115f519d3962390c4e5ba4) )
	ROM_LOAD( "epr-10929.13", 0x40000, 0x20000, CRC(6c07c78d) SHA1(3868b1824f43e4f2b4fbcd9274bfb3000c889d12) )
ROM_END


//*************************************************************************************************************************
//*************************************************************************************************************************
//*************************************************************************************************************************
//  Afterburner II, Sega X-board
//  CPU: 68000 (317-????)
//
ROM_START( aburner2 )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-11107.58",  0x00000, 0x20000, CRC(6d87bab7) SHA1(ab34fe78f1f216037b3e3dca3e61f1b31c05cedf) )
	ROM_LOAD16_BYTE( "epr-11108.63", 0x00001, 0x20000, CRC(202a3e1d) SHA1(cf2018bbad366de4b222eae35942636ca68aa581) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-11109.20", 0x00000, 0x20000, CRC(85a0fe07) SHA1(5a3a8fda6cb4898cfece4ec865b81b9b60f9ad55) )
	ROM_LOAD16_BYTE( "epr-11110.29", 0x00001, 0x20000, CRC(f3d6797c) SHA1(17487b89ddbfbcc32a0b52268259f1c8d10fd0b2) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-11115.154", 0x00000, 0x10000, CRC(e8e32921) SHA1(30a96e6b514a475c778296228ba5b6fb96b211b0) )
	ROM_LOAD( "epr-11114.153", 0x10000, 0x10000, CRC(2e97f633) SHA1(074125c106dd00785903b2e10cd7e28d5036eb60) )
	ROM_LOAD( "epr-11113.152", 0x20000, 0x10000, CRC(36058c8c) SHA1(52befe6c6c53f10b6fd4971098abc8f8d3eef9d4) )

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "mpr-10932.90",  0x000000, 0x20000, CRC(cc0821d6) SHA1(22e84419a585209bbda1466d2180504c316a9b7f) ) // First 8 roms are MPR, the rest are EPR
	ROM_LOAD32_BYTE( "mpr-10934.94",  0x000001, 0x20000, CRC(4a51b1fa) SHA1(2eed018a5a1e935bb72b6f440a794466a1397dc5) )
	ROM_LOAD32_BYTE( "mpr-10936.98",  0x000002, 0x20000, CRC(ada70d64) SHA1(ba6203b0fdb4c4998b7be5b446eb8354751d553a) )
	ROM_LOAD32_BYTE( "mpr-10938.102", 0x000003, 0x20000, CRC(e7675baf) SHA1(aa979319a44c0b18c462afb5ca9cdeed2292c76a) )
	ROM_LOAD32_BYTE( "mpr-10933.91",  0x080000, 0x20000, CRC(c8efb2c3) SHA1(ba31da93f929f2c457e60b2099d5a1ba6b5a9f48) )
	ROM_LOAD32_BYTE( "mpr-10935.95",  0x080001, 0x20000, CRC(c1e23521) SHA1(5e95f3b6ff9f4caca676eaa6c84f1200315218ea) )
	ROM_LOAD32_BYTE( "mpr-10937.99",  0x080002, 0x20000, CRC(f0199658) SHA1(cd67504fef53f637a3b1c723c4a04148f88028d2) )
	ROM_LOAD32_BYTE( "mpr-10939.103", 0x080003, 0x20000, CRC(a0d49480) SHA1(6c4234456bc09ae771beec284d7aa21ebe474f6f) )
	ROM_LOAD32_BYTE( "epr-11103.92",  0x100000, 0x20000, CRC(bdd60da2) SHA1(01673837c5ad84fa087728a05549ac01542ef4e9) )
	ROM_LOAD32_BYTE( "epr-11104.96",  0x100001, 0x20000, CRC(06a35fce) SHA1(c39ae02fc8246e883c4f4c320f668ce6ca9c845a) )
	ROM_LOAD32_BYTE( "epr-11105.100", 0x100002, 0x20000, CRC(027b0689) SHA1(c704c79faadb5e445fd3bd9281683b09831782d2) )
	ROM_LOAD32_BYTE( "epr-11106.104", 0x100003, 0x20000, CRC(9e1fec09) SHA1(6cc47d86852b988bfcd64cb4ed7d832c683e3114) )
	ROM_LOAD32_BYTE( "epr-11116.93",  0x180000, 0x20000, CRC(49b4c1ba) SHA1(5419f49f091e386eead4ccf5e03f12769e278179) )
	ROM_LOAD32_BYTE( "epr-11117.97",  0x180001, 0x20000, CRC(821fbb71) SHA1(be2366d7b4a3a2543ba5024f0e258f1bc43caec8) )
	ROM_LOAD32_BYTE( "epr-11118.101", 0x180002, 0x20000, CRC(8f38540b) SHA1(1fdfb157d1aca96cb635bd3d64f94545eb88c133) )
	ROM_LOAD32_BYTE( "epr-11119.105", 0x180003, 0x20000, CRC(d0343a8e) SHA1(8c0c0addb6dfd0ea04c3900a9f7f7c731ca6e9ea) )

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	ROM_LOAD( "epr-10922.40", 0x000000, 0x10000, CRC(b49183d4) SHA1(71d87bfbce858049ccde9597ab15575b3cdba892) )

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-11112.17",    0x00000, 0x10000, CRC(d777fc6d) SHA1(46ce1c3875437044c0a172960d560d6acd6eaa92) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "mpr-10931.11", 0x00000, 0x20000, CRC(9209068f) SHA1(01f3dda1c066d00080c55f2c86c506b6b2407f98) )
	ROM_LOAD( "mpr-10930.12", 0x20000, 0x20000, CRC(6493368b) SHA1(328aff19ff1d1344e9115f519d3962390c4e5ba4) )
	ROM_LOAD( "epr-11102.13", 0x40000, 0x20000, CRC(6c07c78d) SHA1(3868b1824f43e4f2b4fbcd9274bfb3000c889d12) )
ROM_END

//*************************************************************************************************************************
//  Afterburner II (German), Sega X-board
//  CPU: 68000 (317-????)
//  Sega Game ID #: 834-6335-04 AFTER BURNER
//
ROM_START( aburner2g )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-11173a.58", 0x00000, 0x20000, CRC(cbf480f4) SHA1(f5bab7b2889cdd3f3f2a3e9edd3f17b4d2a5b8a9) )
	ROM_LOAD16_BYTE( "epr-11174a.63", 0x00001, 0x20000, CRC(ed7cba77) SHA1(e81f24fa93329ad25150eada7717cce55fa3887d) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-11109.20", 0x00000, 0x20000, CRC(85a0fe07) SHA1(5a3a8fda6cb4898cfece4ec865b81b9b60f9ad55) )
	ROM_LOAD16_BYTE( "epr-11110.29", 0x00001, 0x20000, CRC(f3d6797c) SHA1(17487b89ddbfbcc32a0b52268259f1c8d10fd0b2) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-11115.154", 0x00000, 0x10000, CRC(e8e32921) SHA1(30a96e6b514a475c778296228ba5b6fb96b211b0) )
	ROM_LOAD( "epr-11114.153", 0x10000, 0x10000, CRC(2e97f633) SHA1(074125c106dd00785903b2e10cd7e28d5036eb60) )
	ROM_LOAD( "epr-11113.152", 0x20000, 0x10000, CRC(36058c8c) SHA1(52befe6c6c53f10b6fd4971098abc8f8d3eef9d4) )

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "mpr-10932.90",  0x000000, 0x20000, CRC(cc0821d6) SHA1(22e84419a585209bbda1466d2180504c316a9b7f) ) // First 8 roms are MPR, the rest are EPR
	ROM_LOAD32_BYTE( "mpr-10934.94",  0x000001, 0x20000, CRC(4a51b1fa) SHA1(2eed018a5a1e935bb72b6f440a794466a1397dc5) )
	ROM_LOAD32_BYTE( "mpr-10936.98",  0x000002, 0x20000, CRC(ada70d64) SHA1(ba6203b0fdb4c4998b7be5b446eb8354751d553a) )
	ROM_LOAD32_BYTE( "mpr-10938.102", 0x000003, 0x20000, CRC(e7675baf) SHA1(aa979319a44c0b18c462afb5ca9cdeed2292c76a) )
	ROM_LOAD32_BYTE( "mpr-10933.91",  0x080000, 0x20000, CRC(c8efb2c3) SHA1(ba31da93f929f2c457e60b2099d5a1ba6b5a9f48) )
	ROM_LOAD32_BYTE( "mpr-10935.95",  0x080001, 0x20000, CRC(c1e23521) SHA1(5e95f3b6ff9f4caca676eaa6c84f1200315218ea) )
	ROM_LOAD32_BYTE( "mpr-10937.99",  0x080002, 0x20000, CRC(f0199658) SHA1(cd67504fef53f637a3b1c723c4a04148f88028d2) )
	ROM_LOAD32_BYTE( "mpr-10939.103", 0x080003, 0x20000, CRC(a0d49480) SHA1(6c4234456bc09ae771beec284d7aa21ebe474f6f) )
	ROM_LOAD32_BYTE( "epr-11094.92",  0x100000, 0x20000, CRC(bdd60da2) SHA1(01673837c5ad84fa087728a05549ac01542ef4e9) )
	ROM_LOAD32_BYTE( "epr-11095.96",  0x100001, 0x20000, CRC(06a35fce) SHA1(c39ae02fc8246e883c4f4c320f668ce6ca9c845a) )
	ROM_LOAD32_BYTE( "epr-11096.100", 0x100002, 0x20000, CRC(027b0689) SHA1(c704c79faadb5e445fd3bd9281683b09831782d2) )
	ROM_LOAD32_BYTE( "epr-11097.104", 0x100003, 0x20000, CRC(9e1fec09) SHA1(6cc47d86852b988bfcd64cb4ed7d832c683e3114) )
	ROM_LOAD32_BYTE( "epr-11116.93",  0x180000, 0x20000, CRC(49b4c1ba) SHA1(5419f49f091e386eead4ccf5e03f12769e278179) )
	ROM_LOAD32_BYTE( "epr-11117.97",  0x180001, 0x20000, CRC(821fbb71) SHA1(be2366d7b4a3a2543ba5024f0e258f1bc43caec8) )
	ROM_LOAD32_BYTE( "epr-11118.101", 0x180002, 0x20000, CRC(8f38540b) SHA1(1fdfb157d1aca96cb635bd3d64f94545eb88c133) )
	ROM_LOAD32_BYTE( "epr-11119.105", 0x180003, 0x20000, CRC(d0343a8e) SHA1(8c0c0addb6dfd0ea04c3900a9f7f7c731ca6e9ea) )

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	ROM_LOAD( "epr-10922.40", 0x000000, 0x10000, CRC(b49183d4) SHA1(71d87bfbce858049ccde9597ab15575b3cdba892) )

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-11112.17",    0x00000, 0x10000, CRC(d777fc6d) SHA1(46ce1c3875437044c0a172960d560d6acd6eaa92) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "mpr-10931.11", 0x00000, 0x20000, CRC(9209068f) SHA1(01f3dda1c066d00080c55f2c86c506b6b2407f98) ) // There is known to exist German Sample roms
	ROM_LOAD( "mpr-10930.12", 0x20000, 0x20000, CRC(6493368b) SHA1(328aff19ff1d1344e9115f519d3962390c4e5ba4) )
	ROM_LOAD( "epr-10929.13", 0x40000, 0x20000, CRC(6c07c78d) SHA1(3868b1824f43e4f2b4fbcd9274bfb3000c889d12) )
ROM_END


//*************************************************************************************************************************
//*************************************************************************************************************************
//*************************************************************************************************************************
//  Line of Fire, Sega X-board
//  CPU: FD1094 (317-0136)
//  Sega game ID# 834-7218-02
//
ROM_START( loffire )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-12849.58", 0x000000, 0x20000, CRC(61cfd2fe) SHA1(b47ae9cdf741574ab9128dd3556b1ef35e81a149) )
	ROM_LOAD16_BYTE( "epr-12850.63", 0x000001, 0x20000, CRC(14598f2a) SHA1(13a51529ed32acefd733d9f638621c3e023dbd6d) )

	//
	// It's not possible to determine the original value with just the available
	// ROM data. The choice was between 47, 56 and 57, which decrypt correctly all
	// the code at the affected addresses (2638, 6638 and so on).
	// I chose 57 because it's the only one that has only 1 bit different from the
	// bad value in the old dump (77).
	//
	// Nicola Salmoria
	//

	ROM_REGION( 0x2000, "mainpcb:maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0136.key", 0x0000, 0x2000, BAD_DUMP CRC(344bfe0c) SHA1(f6bb8045b46f90f8abadf1dc2e1ae1d7cef9c810) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-12804.20", 0x000000, 0x20000, CRC(b853480e) SHA1(de0889e99251da7ea50316282ebf6f434cc2db11) )
	ROM_LOAD16_BYTE( "epr-12805.29", 0x000001, 0x20000, CRC(4a7200c3) SHA1(3e6febed36a55438e0d24441b68f2b7952791584) )
	ROM_LOAD16_BYTE( "epr-12802.21", 0x040000, 0x20000, CRC(d746bb39) SHA1(08dc8cf565997c7e52329961bf7a229a15900cff) )
	ROM_LOAD16_BYTE( "epr-12803.30", 0x040001, 0x20000, CRC(c1d9e751) SHA1(98b3d0b3b31702f6234b5fea2b82d512fc5d3ad2) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "opr-12791.154", 0x00000, 0x10000, CRC(acfa69ba) SHA1(353c43dda6c2282a785646b0a58c90cfd173cd7b) )
	ROM_LOAD( "opr-12792.153", 0x10000, 0x10000, CRC(e506723c) SHA1(d04dc29686fe348f8f715d14c027de0e508c770f) )
	ROM_LOAD( "opr-12793.152", 0x20000, 0x10000, CRC(0ce8cce3) SHA1(1a6b1af2b0b9e8240e681f7b15e9d08595753fe6) )

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "epr-12787.90",  0x000000, 0x20000, CRC(6431a3a6) SHA1(63a732b7dfd2b83fe7684d47fea26063c4ece099) )
	ROM_LOAD32_BYTE( "epr-12788.94",  0x000001, 0x20000, CRC(1982a0ce) SHA1(e4756f31b0094e0e9ddb2df53a5c938ac5559230) )
	ROM_LOAD32_BYTE( "epr-12789.98",  0x000002, 0x20000, CRC(97d03274) SHA1(b4b9921db53949bc8e91f8a2992e89c172fe8893) )
	ROM_LOAD32_BYTE( "epr-12790.102", 0x000003, 0x20000, CRC(816e76e6) SHA1(34d2a662af96f40f40a77497cbc0a3374fe9a34f) )
	ROM_LOAD32_BYTE( "epr-12783.91",  0x080000, 0x20000, CRC(c13feea9) SHA1(c0c3097903079deec22b0f8de76927f7570ac0f6) )
	ROM_LOAD32_BYTE( "epr-12784.95",  0x080001, 0x20000, CRC(39b94c65) SHA1(4deae3bf7bb4e04b011d23292a0c68471758e7ec) )
	ROM_LOAD32_BYTE( "epr-12785.99",  0x080002, 0x20000, CRC(05ed0059) SHA1(b7404a0f4f15ffdbd08673683cea22340de3f5f9) )
	ROM_LOAD32_BYTE( "epr-12786.103", 0x080003, 0x20000, CRC(a4123165) SHA1(024597dcfbd3be932626b84dbd6e7d38a7a0195d) )
	ROM_LOAD32_BYTE( "epr-12779.92",  0x100000, 0x20000, CRC(ae58af7c) SHA1(8c57f2d0b6584dd606afc5ecff039479e5068420) )
	ROM_LOAD32_BYTE( "epr-12780.96",  0x100001, 0x20000, CRC(ee670c1e) SHA1(8a9e0808d40e210abf6c49ef5c0774d8c0d6602b) )
	ROM_LOAD32_BYTE( "epr-12781.100", 0x100002, 0x20000, CRC(538f6bc5) SHA1(4f294ef0aa9c7e2ac7e92518d938f0870f2e46d1) )
	ROM_LOAD32_BYTE( "epr-12782.104", 0x100003, 0x20000, CRC(5acc34f7) SHA1(ef27ab818f50e59a122b9fc65b13442d9fee307c) )
	ROM_LOAD32_BYTE( "epr-12775.93",  0x180000, 0x20000, CRC(693056ec) SHA1(82d10d960441811b9369295bbb60fa7bfc5457a3) )
	ROM_LOAD32_BYTE( "epr-12776.97",  0x180001, 0x20000, CRC(61efbdfd) SHA1(67f267e0673c64ce77669826ea1d11cb79d0ccc1) )
	ROM_LOAD32_BYTE( "epr-12777.101", 0x180002, 0x20000, CRC(29d5b953) SHA1(0c932a67e2aecffa7a1dbaa587c96214e1a2cc7f) )
	ROM_LOAD32_BYTE( "epr-12778.105", 0x180003, 0x20000, CRC(2fb68e07) SHA1(8685e72aed115cbc9c6c7511217996a573b30d16) )

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-12798.17", 0x00000, 0x10000, CRC(0587738d) SHA1(24c79b0c73616d5532a49a2c9121dfabe3a80c7d) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "epr-12799.11", 0x00000, 0x20000, CRC(bc60181c) SHA1(3c89161348db7cafb5636ab4eaba91fbd3541f90) )
	ROM_LOAD( "epr-12800.12", 0x20000, 0x20000, CRC(1158c1a3) SHA1(e1d664a203eed5a0130b39ced7bea8328f06f107) )
	ROM_LOAD( "epr-12801.13", 0x40000, 0x20000, CRC(2d6567c4) SHA1(542be9d8e91cf2df18d95f4e259cfda0560697cb) )
ROM_END

ROM_START( loffired )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bootleg_epr-12849.58", 0x000000, 0x20000, CRC(dfd1ab45) SHA1(dac358b6f50999deaed422578c2dcdfb492c81c9) )
	ROM_LOAD16_BYTE( "bootleg_epr-12850.63", 0x000001, 0x20000, CRC(90889ae9) SHA1(254f8934e8a0329e28a38c71c4bd628ef7237ca8) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-12804.20", 0x000000, 0x20000, CRC(b853480e) SHA1(de0889e99251da7ea50316282ebf6f434cc2db11) )
	ROM_LOAD16_BYTE( "epr-12805.29", 0x000001, 0x20000, CRC(4a7200c3) SHA1(3e6febed36a55438e0d24441b68f2b7952791584) )
	ROM_LOAD16_BYTE( "epr-12802.21", 0x040000, 0x20000, CRC(d746bb39) SHA1(08dc8cf565997c7e52329961bf7a229a15900cff) )
	ROM_LOAD16_BYTE( "epr-12803.30", 0x040001, 0x20000, CRC(c1d9e751) SHA1(98b3d0b3b31702f6234b5fea2b82d512fc5d3ad2) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "opr-12791.154", 0x00000, 0x10000, CRC(acfa69ba) SHA1(353c43dda6c2282a785646b0a58c90cfd173cd7b) )
	ROM_LOAD( "opr-12792.153", 0x10000, 0x10000, CRC(e506723c) SHA1(d04dc29686fe348f8f715d14c027de0e508c770f) )
	ROM_LOAD( "opr-12793.152", 0x20000, 0x10000, CRC(0ce8cce3) SHA1(1a6b1af2b0b9e8240e681f7b15e9d08595753fe6) )

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "epr-12787.90",  0x000000, 0x20000, CRC(6431a3a6) SHA1(63a732b7dfd2b83fe7684d47fea26063c4ece099) )
	ROM_LOAD32_BYTE( "epr-12788.94",  0x000001, 0x20000, CRC(1982a0ce) SHA1(e4756f31b0094e0e9ddb2df53a5c938ac5559230) )
	ROM_LOAD32_BYTE( "epr-12789.98",  0x000002, 0x20000, CRC(97d03274) SHA1(b4b9921db53949bc8e91f8a2992e89c172fe8893) )
	ROM_LOAD32_BYTE( "epr-12790.102", 0x000003, 0x20000, CRC(816e76e6) SHA1(34d2a662af96f40f40a77497cbc0a3374fe9a34f) )
	ROM_LOAD32_BYTE( "epr-12783.91",  0x080000, 0x20000, CRC(c13feea9) SHA1(c0c3097903079deec22b0f8de76927f7570ac0f6) )
	ROM_LOAD32_BYTE( "epr-12784.95",  0x080001, 0x20000, CRC(39b94c65) SHA1(4deae3bf7bb4e04b011d23292a0c68471758e7ec) )
	ROM_LOAD32_BYTE( "epr-12785.99",  0x080002, 0x20000, CRC(05ed0059) SHA1(b7404a0f4f15ffdbd08673683cea22340de3f5f9) )
	ROM_LOAD32_BYTE( "epr-12786.103", 0x080003, 0x20000, CRC(a4123165) SHA1(024597dcfbd3be932626b84dbd6e7d38a7a0195d) )
	ROM_LOAD32_BYTE( "epr-12779.92",  0x100000, 0x20000, CRC(ae58af7c) SHA1(8c57f2d0b6584dd606afc5ecff039479e5068420) )
	ROM_LOAD32_BYTE( "epr-12780.96",  0x100001, 0x20000, CRC(ee670c1e) SHA1(8a9e0808d40e210abf6c49ef5c0774d8c0d6602b) )
	ROM_LOAD32_BYTE( "epr-12781.100", 0x100002, 0x20000, CRC(538f6bc5) SHA1(4f294ef0aa9c7e2ac7e92518d938f0870f2e46d1) )
	ROM_LOAD32_BYTE( "epr-12782.104", 0x100003, 0x20000, CRC(5acc34f7) SHA1(ef27ab818f50e59a122b9fc65b13442d9fee307c) )
	ROM_LOAD32_BYTE( "epr-12775.93",  0x180000, 0x20000, CRC(693056ec) SHA1(82d10d960441811b9369295bbb60fa7bfc5457a3) )
	ROM_LOAD32_BYTE( "epr-12776.97",  0x180001, 0x20000, CRC(61efbdfd) SHA1(67f267e0673c64ce77669826ea1d11cb79d0ccc1) )
	ROM_LOAD32_BYTE( "epr-12777.101", 0x180002, 0x20000, CRC(29d5b953) SHA1(0c932a67e2aecffa7a1dbaa587c96214e1a2cc7f) )
	ROM_LOAD32_BYTE( "epr-12778.105", 0x180003, 0x20000, CRC(2fb68e07) SHA1(8685e72aed115cbc9c6c7511217996a573b30d16) )

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-12798.17", 0x00000, 0x10000, CRC(0587738d) SHA1(24c79b0c73616d5532a49a2c9121dfabe3a80c7d) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "epr-12799.11", 0x00000, 0x20000, CRC(bc60181c) SHA1(3c89161348db7cafb5636ab4eaba91fbd3541f90) )
	ROM_LOAD( "epr-12800.12", 0x20000, 0x20000, CRC(1158c1a3) SHA1(e1d664a203eed5a0130b39ced7bea8328f06f107) )
	ROM_LOAD( "epr-12801.13", 0x40000, 0x20000, CRC(2d6567c4) SHA1(542be9d8e91cf2df18d95f4e259cfda0560697cb) )
ROM_END

//*************************************************************************************************************************
//  Line of Fire, Sega X-board
//  CPU: FD1094 (317-0135)
//  Sega game ID# 834-7218-01
//
ROM_START( loffireu )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-12847a.58", 0x000000, 0x20000, CRC(c50eb4ed) SHA1(18a46c97aec2fefd160338c1760b6ee367dcb57f) )
	ROM_LOAD16_BYTE( "epr-12848a.63", 0x000001, 0x20000, CRC(f8ff8640) SHA1(193bb8f42f3c5011ad1fbf87215f012de5e950fb) )

	ROM_REGION( 0x2000, "mainpcb:maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0135.key", 0x0000, 0x2000, CRC(c53ad019) SHA1(7e6dc2b35ebfeefb507d4d03f5a59574944177d1) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-12804.20", 0x000000, 0x20000, CRC(b853480e) SHA1(de0889e99251da7ea50316282ebf6f434cc2db11) )
	ROM_LOAD16_BYTE( "epr-12805.29", 0x000001, 0x20000, CRC(4a7200c3) SHA1(3e6febed36a55438e0d24441b68f2b7952791584) )
	ROM_LOAD16_BYTE( "epr-12802.21", 0x040000, 0x20000, CRC(d746bb39) SHA1(08dc8cf565997c7e52329961bf7a229a15900cff) )
	ROM_LOAD16_BYTE( "epr-12803.30", 0x040001, 0x20000, CRC(c1d9e751) SHA1(98b3d0b3b31702f6234b5fea2b82d512fc5d3ad2) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "opr-12791.154", 0x00000, 0x10000, CRC(acfa69ba) SHA1(353c43dda6c2282a785646b0a58c90cfd173cd7b) )
	ROM_LOAD( "opr-12792.153", 0x10000, 0x10000, CRC(e506723c) SHA1(d04dc29686fe348f8f715d14c027de0e508c770f) )
	ROM_LOAD( "opr-12793.152", 0x20000, 0x10000, CRC(0ce8cce3) SHA1(1a6b1af2b0b9e8240e681f7b15e9d08595753fe6) )

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "epr-12787.90",  0x000000, 0x20000, CRC(6431a3a6) SHA1(63a732b7dfd2b83fe7684d47fea26063c4ece099) )
	ROM_LOAD32_BYTE( "epr-12788.94",  0x000001, 0x20000, CRC(1982a0ce) SHA1(e4756f31b0094e0e9ddb2df53a5c938ac5559230) )
	ROM_LOAD32_BYTE( "epr-12789.98",  0x000002, 0x20000, CRC(97d03274) SHA1(b4b9921db53949bc8e91f8a2992e89c172fe8893) )
	ROM_LOAD32_BYTE( "epr-12790.102", 0x000003, 0x20000, CRC(816e76e6) SHA1(34d2a662af96f40f40a77497cbc0a3374fe9a34f) )
	ROM_LOAD32_BYTE( "epr-12783.91",  0x080000, 0x20000, CRC(c13feea9) SHA1(c0c3097903079deec22b0f8de76927f7570ac0f6) )
	ROM_LOAD32_BYTE( "epr-12784.95",  0x080001, 0x20000, CRC(39b94c65) SHA1(4deae3bf7bb4e04b011d23292a0c68471758e7ec) )
	ROM_LOAD32_BYTE( "epr-12785.99",  0x080002, 0x20000, CRC(05ed0059) SHA1(b7404a0f4f15ffdbd08673683cea22340de3f5f9) )
	ROM_LOAD32_BYTE( "epr-12786.103", 0x080003, 0x20000, CRC(a4123165) SHA1(024597dcfbd3be932626b84dbd6e7d38a7a0195d) )
	ROM_LOAD32_BYTE( "epr-12779.92",  0x100000, 0x20000, CRC(ae58af7c) SHA1(8c57f2d0b6584dd606afc5ecff039479e5068420) )
	ROM_LOAD32_BYTE( "epr-12780.96",  0x100001, 0x20000, CRC(ee670c1e) SHA1(8a9e0808d40e210abf6c49ef5c0774d8c0d6602b) )
	ROM_LOAD32_BYTE( "epr-12781.100", 0x100002, 0x20000, CRC(538f6bc5) SHA1(4f294ef0aa9c7e2ac7e92518d938f0870f2e46d1) )
	ROM_LOAD32_BYTE( "epr-12782.104", 0x100003, 0x20000, CRC(5acc34f7) SHA1(ef27ab818f50e59a122b9fc65b13442d9fee307c) )
	ROM_LOAD32_BYTE( "epr-12775.93",  0x180000, 0x20000, CRC(693056ec) SHA1(82d10d960441811b9369295bbb60fa7bfc5457a3) )
	ROM_LOAD32_BYTE( "epr-12776.97",  0x180001, 0x20000, CRC(61efbdfd) SHA1(67f267e0673c64ce77669826ea1d11cb79d0ccc1) )
	ROM_LOAD32_BYTE( "epr-12777.101", 0x180002, 0x20000, CRC(29d5b953) SHA1(0c932a67e2aecffa7a1dbaa587c96214e1a2cc7f) )
	ROM_LOAD32_BYTE( "epr-12778.105", 0x180003, 0x20000, CRC(2fb68e07) SHA1(8685e72aed115cbc9c6c7511217996a573b30d16) )

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-12798.17", 0x00000, 0x10000, CRC(0587738d) SHA1(24c79b0c73616d5532a49a2c9121dfabe3a80c7d) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "epr-12799.11", 0x00000, 0x20000, CRC(bc60181c) SHA1(3c89161348db7cafb5636ab4eaba91fbd3541f90) )
	ROM_LOAD( "epr-12800.12", 0x20000, 0x20000, CRC(1158c1a3) SHA1(e1d664a203eed5a0130b39ced7bea8328f06f107) )
	ROM_LOAD( "epr-12801.13", 0x40000, 0x20000, CRC(2d6567c4) SHA1(542be9d8e91cf2df18d95f4e259cfda0560697cb) )
ROM_END

ROM_START( loffireud )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bootleg_epr-12847a.58", 0x000000, 0x20000, CRC(74d270d0) SHA1(88819b4a4b49e4f02fdb4a617e2548a82ce7e835) )
	ROM_LOAD16_BYTE( "bootleg_epr-12848a.63", 0x000001, 0x20000, CRC(7f27e058) SHA1(98401f992e4feb9141dc802edaaaa09eedfa8817) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-12804.20", 0x000000, 0x20000, CRC(b853480e) SHA1(de0889e99251da7ea50316282ebf6f434cc2db11) )
	ROM_LOAD16_BYTE( "epr-12805.29", 0x000001, 0x20000, CRC(4a7200c3) SHA1(3e6febed36a55438e0d24441b68f2b7952791584) )
	ROM_LOAD16_BYTE( "epr-12802.21", 0x040000, 0x20000, CRC(d746bb39) SHA1(08dc8cf565997c7e52329961bf7a229a15900cff) )
	ROM_LOAD16_BYTE( "epr-12803.30", 0x040001, 0x20000, CRC(c1d9e751) SHA1(98b3d0b3b31702f6234b5fea2b82d512fc5d3ad2) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "opr-12791.154", 0x00000, 0x10000, CRC(acfa69ba) SHA1(353c43dda6c2282a785646b0a58c90cfd173cd7b) )
	ROM_LOAD( "opr-12792.153", 0x10000, 0x10000, CRC(e506723c) SHA1(d04dc29686fe348f8f715d14c027de0e508c770f) )
	ROM_LOAD( "opr-12793.152", 0x20000, 0x10000, CRC(0ce8cce3) SHA1(1a6b1af2b0b9e8240e681f7b15e9d08595753fe6) )

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "epr-12787.90",  0x000000, 0x20000, CRC(6431a3a6) SHA1(63a732b7dfd2b83fe7684d47fea26063c4ece099) )
	ROM_LOAD32_BYTE( "epr-12788.94",  0x000001, 0x20000, CRC(1982a0ce) SHA1(e4756f31b0094e0e9ddb2df53a5c938ac5559230) )
	ROM_LOAD32_BYTE( "epr-12789.98",  0x000002, 0x20000, CRC(97d03274) SHA1(b4b9921db53949bc8e91f8a2992e89c172fe8893) )
	ROM_LOAD32_BYTE( "epr-12790.102", 0x000003, 0x20000, CRC(816e76e6) SHA1(34d2a662af96f40f40a77497cbc0a3374fe9a34f) )
	ROM_LOAD32_BYTE( "epr-12783.91",  0x080000, 0x20000, CRC(c13feea9) SHA1(c0c3097903079deec22b0f8de76927f7570ac0f6) )
	ROM_LOAD32_BYTE( "epr-12784.95",  0x080001, 0x20000, CRC(39b94c65) SHA1(4deae3bf7bb4e04b011d23292a0c68471758e7ec) )
	ROM_LOAD32_BYTE( "epr-12785.99",  0x080002, 0x20000, CRC(05ed0059) SHA1(b7404a0f4f15ffdbd08673683cea22340de3f5f9) )
	ROM_LOAD32_BYTE( "epr-12786.103", 0x080003, 0x20000, CRC(a4123165) SHA1(024597dcfbd3be932626b84dbd6e7d38a7a0195d) )
	ROM_LOAD32_BYTE( "epr-12779.92",  0x100000, 0x20000, CRC(ae58af7c) SHA1(8c57f2d0b6584dd606afc5ecff039479e5068420) )
	ROM_LOAD32_BYTE( "epr-12780.96",  0x100001, 0x20000, CRC(ee670c1e) SHA1(8a9e0808d40e210abf6c49ef5c0774d8c0d6602b) )
	ROM_LOAD32_BYTE( "epr-12781.100", 0x100002, 0x20000, CRC(538f6bc5) SHA1(4f294ef0aa9c7e2ac7e92518d938f0870f2e46d1) )
	ROM_LOAD32_BYTE( "epr-12782.104", 0x100003, 0x20000, CRC(5acc34f7) SHA1(ef27ab818f50e59a122b9fc65b13442d9fee307c) )
	ROM_LOAD32_BYTE( "epr-12775.93",  0x180000, 0x20000, CRC(693056ec) SHA1(82d10d960441811b9369295bbb60fa7bfc5457a3) )
	ROM_LOAD32_BYTE( "epr-12776.97",  0x180001, 0x20000, CRC(61efbdfd) SHA1(67f267e0673c64ce77669826ea1d11cb79d0ccc1) )
	ROM_LOAD32_BYTE( "epr-12777.101", 0x180002, 0x20000, CRC(29d5b953) SHA1(0c932a67e2aecffa7a1dbaa587c96214e1a2cc7f) )
	ROM_LOAD32_BYTE( "epr-12778.105", 0x180003, 0x20000, CRC(2fb68e07) SHA1(8685e72aed115cbc9c6c7511217996a573b30d16) )

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-12798.17", 0x00000, 0x10000, CRC(0587738d) SHA1(24c79b0c73616d5532a49a2c9121dfabe3a80c7d) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "epr-12799.11", 0x00000, 0x20000, CRC(bc60181c) SHA1(3c89161348db7cafb5636ab4eaba91fbd3541f90) )
	ROM_LOAD( "epr-12800.12", 0x20000, 0x20000, CRC(1158c1a3) SHA1(e1d664a203eed5a0130b39ced7bea8328f06f107) )
	ROM_LOAD( "epr-12801.13", 0x40000, 0x20000, CRC(2d6567c4) SHA1(542be9d8e91cf2df18d95f4e259cfda0560697cb) )
ROM_END

//*************************************************************************************************************************
//  Line of Fire, Sega X-board
//  CPU: FD1094 (317-0134)
//  Sega game ID# 834-7218
//
ROM_START( loffirej )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	// repaired using data from the loffire set since they are mostly identical
	// when decrypted, they pass the rom check so are assumed to be ok but double
	// checking them when possible never hurts
	ROM_LOAD16_BYTE( "epr-12794.58", 0x000000, 0x20000, CRC(1e588992) SHA1(fe7107e83c12643e7d22fd4b4cd0c7bcff0d84c3) )
	ROM_LOAD16_BYTE( "epr-12795.63", 0x000001, 0x20000, CRC(d43d7427) SHA1(ecbd425bab6aa65ffbd441d6a0936ac055d5f06d) )

	ROM_REGION( 0x2000, "mainpcb:maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0134.key", 0x0000, 0x2000, CRC(732626d4) SHA1(75ed7ca417758dd62afb4edbb9daee754932c392) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-12804.20", 0x000000, 0x20000, CRC(b853480e) SHA1(de0889e99251da7ea50316282ebf6f434cc2db11) )
	ROM_LOAD16_BYTE( "epr-12805.29", 0x000001, 0x20000, CRC(4a7200c3) SHA1(3e6febed36a55438e0d24441b68f2b7952791584) )
	ROM_LOAD16_BYTE( "epr-12802.21", 0x040000, 0x20000, CRC(d746bb39) SHA1(08dc8cf565997c7e52329961bf7a229a15900cff) )
	ROM_LOAD16_BYTE( "epr-12803.30", 0x040001, 0x20000, CRC(c1d9e751) SHA1(98b3d0b3b31702f6234b5fea2b82d512fc5d3ad2) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "opr-12791.154", 0x00000, 0x10000, CRC(acfa69ba) SHA1(353c43dda6c2282a785646b0a58c90cfd173cd7b) )
	ROM_LOAD( "opr-12792.153", 0x10000, 0x10000, CRC(e506723c) SHA1(d04dc29686fe348f8f715d14c027de0e508c770f) )
	ROM_LOAD( "opr-12793.152", 0x20000, 0x10000, CRC(0ce8cce3) SHA1(1a6b1af2b0b9e8240e681f7b15e9d08595753fe6) )

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "epr-12787.90",  0x000000, 0x20000, CRC(6431a3a6) SHA1(63a732b7dfd2b83fe7684d47fea26063c4ece099) )
	ROM_LOAD32_BYTE( "epr-12788.94",  0x000001, 0x20000, CRC(1982a0ce) SHA1(e4756f31b0094e0e9ddb2df53a5c938ac5559230) )
	ROM_LOAD32_BYTE( "epr-12789.98",  0x000002, 0x20000, CRC(97d03274) SHA1(b4b9921db53949bc8e91f8a2992e89c172fe8893) )
	ROM_LOAD32_BYTE( "epr-12790.102", 0x000003, 0x20000, CRC(816e76e6) SHA1(34d2a662af96f40f40a77497cbc0a3374fe9a34f) )
	ROM_LOAD32_BYTE( "epr-12783.91",  0x080000, 0x20000, CRC(c13feea9) SHA1(c0c3097903079deec22b0f8de76927f7570ac0f6) )
	ROM_LOAD32_BYTE( "epr-12784.95",  0x080001, 0x20000, CRC(39b94c65) SHA1(4deae3bf7bb4e04b011d23292a0c68471758e7ec) )
	ROM_LOAD32_BYTE( "epr-12785.99",  0x080002, 0x20000, CRC(05ed0059) SHA1(b7404a0f4f15ffdbd08673683cea22340de3f5f9) )
	ROM_LOAD32_BYTE( "epr-12786.103", 0x080003, 0x20000, CRC(a4123165) SHA1(024597dcfbd3be932626b84dbd6e7d38a7a0195d) )
	ROM_LOAD32_BYTE( "epr-12779.92",  0x100000, 0x20000, CRC(ae58af7c) SHA1(8c57f2d0b6584dd606afc5ecff039479e5068420) )
	ROM_LOAD32_BYTE( "epr-12780.96",  0x100001, 0x20000, CRC(ee670c1e) SHA1(8a9e0808d40e210abf6c49ef5c0774d8c0d6602b) )
	ROM_LOAD32_BYTE( "epr-12781.100", 0x100002, 0x20000, CRC(538f6bc5) SHA1(4f294ef0aa9c7e2ac7e92518d938f0870f2e46d1) )
	ROM_LOAD32_BYTE( "epr-12782.104", 0x100003, 0x20000, CRC(5acc34f7) SHA1(ef27ab818f50e59a122b9fc65b13442d9fee307c) )
	ROM_LOAD32_BYTE( "epr-12775.93",  0x180000, 0x20000, CRC(693056ec) SHA1(82d10d960441811b9369295bbb60fa7bfc5457a3) )
	ROM_LOAD32_BYTE( "epr-12776.97",  0x180001, 0x20000, CRC(61efbdfd) SHA1(67f267e0673c64ce77669826ea1d11cb79d0ccc1) )
	ROM_LOAD32_BYTE( "epr-12777.101", 0x180002, 0x20000, CRC(29d5b953) SHA1(0c932a67e2aecffa7a1dbaa587c96214e1a2cc7f) )
	ROM_LOAD32_BYTE( "epr-12778.105", 0x180003, 0x20000, CRC(2fb68e07) SHA1(8685e72aed115cbc9c6c7511217996a573b30d16) )

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-12798.17", 0x00000, 0x10000, CRC(0587738d) SHA1(24c79b0c73616d5532a49a2c9121dfabe3a80c7d) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "epr-12799.11", 0x00000, 0x20000, CRC(bc60181c) SHA1(3c89161348db7cafb5636ab4eaba91fbd3541f90) )
	ROM_LOAD( "epr-12800.12", 0x20000, 0x20000, CRC(1158c1a3) SHA1(e1d664a203eed5a0130b39ced7bea8328f06f107) )
	ROM_LOAD( "epr-12801.13", 0x40000, 0x20000, CRC(2d6567c4) SHA1(542be9d8e91cf2df18d95f4e259cfda0560697cb) )
ROM_END

ROM_START( loffirejd )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bootleg_epr-12794.58", 0x000000, 0x20000, CRC(795f110d) SHA1(1592c618d21932490555c5fdf376429dfae00a95) )
	ROM_LOAD16_BYTE( "bootleg_epr-12795.63", 0x000001, 0x20000, CRC(87c52aaa) SHA1(179d735966e46dc2e9d61047038224699c1956ed) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-12804.20", 0x000000, 0x20000, CRC(b853480e) SHA1(de0889e99251da7ea50316282ebf6f434cc2db11) )
	ROM_LOAD16_BYTE( "epr-12805.29", 0x000001, 0x20000, CRC(4a7200c3) SHA1(3e6febed36a55438e0d24441b68f2b7952791584) )
	ROM_LOAD16_BYTE( "epr-12802.21", 0x040000, 0x20000, CRC(d746bb39) SHA1(08dc8cf565997c7e52329961bf7a229a15900cff) )
	ROM_LOAD16_BYTE( "epr-12803.30", 0x040001, 0x20000, CRC(c1d9e751) SHA1(98b3d0b3b31702f6234b5fea2b82d512fc5d3ad2) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "opr-12791.154", 0x00000, 0x10000, CRC(acfa69ba) SHA1(353c43dda6c2282a785646b0a58c90cfd173cd7b) )
	ROM_LOAD( "opr-12792.153", 0x10000, 0x10000, CRC(e506723c) SHA1(d04dc29686fe348f8f715d14c027de0e508c770f) )
	ROM_LOAD( "opr-12793.152", 0x20000, 0x10000, CRC(0ce8cce3) SHA1(1a6b1af2b0b9e8240e681f7b15e9d08595753fe6) )

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "epr-12787.90",  0x000000, 0x20000, CRC(6431a3a6) SHA1(63a732b7dfd2b83fe7684d47fea26063c4ece099) )
	ROM_LOAD32_BYTE( "epr-12788.94",  0x000001, 0x20000, CRC(1982a0ce) SHA1(e4756f31b0094e0e9ddb2df53a5c938ac5559230) )
	ROM_LOAD32_BYTE( "epr-12789.98",  0x000002, 0x20000, CRC(97d03274) SHA1(b4b9921db53949bc8e91f8a2992e89c172fe8893) )
	ROM_LOAD32_BYTE( "epr-12790.102", 0x000003, 0x20000, CRC(816e76e6) SHA1(34d2a662af96f40f40a77497cbc0a3374fe9a34f) )
	ROM_LOAD32_BYTE( "epr-12783.91",  0x080000, 0x20000, CRC(c13feea9) SHA1(c0c3097903079deec22b0f8de76927f7570ac0f6) )
	ROM_LOAD32_BYTE( "epr-12784.95",  0x080001, 0x20000, CRC(39b94c65) SHA1(4deae3bf7bb4e04b011d23292a0c68471758e7ec) )
	ROM_LOAD32_BYTE( "epr-12785.99",  0x080002, 0x20000, CRC(05ed0059) SHA1(b7404a0f4f15ffdbd08673683cea22340de3f5f9) )
	ROM_LOAD32_BYTE( "epr-12786.103", 0x080003, 0x20000, CRC(a4123165) SHA1(024597dcfbd3be932626b84dbd6e7d38a7a0195d) )
	ROM_LOAD32_BYTE( "epr-12779.92",  0x100000, 0x20000, CRC(ae58af7c) SHA1(8c57f2d0b6584dd606afc5ecff039479e5068420) )
	ROM_LOAD32_BYTE( "epr-12780.96",  0x100001, 0x20000, CRC(ee670c1e) SHA1(8a9e0808d40e210abf6c49ef5c0774d8c0d6602b) )
	ROM_LOAD32_BYTE( "epr-12781.100", 0x100002, 0x20000, CRC(538f6bc5) SHA1(4f294ef0aa9c7e2ac7e92518d938f0870f2e46d1) )
	ROM_LOAD32_BYTE( "epr-12782.104", 0x100003, 0x20000, CRC(5acc34f7) SHA1(ef27ab818f50e59a122b9fc65b13442d9fee307c) )
	ROM_LOAD32_BYTE( "epr-12775.93",  0x180000, 0x20000, CRC(693056ec) SHA1(82d10d960441811b9369295bbb60fa7bfc5457a3) )
	ROM_LOAD32_BYTE( "epr-12776.97",  0x180001, 0x20000, CRC(61efbdfd) SHA1(67f267e0673c64ce77669826ea1d11cb79d0ccc1) )
	ROM_LOAD32_BYTE( "epr-12777.101", 0x180002, 0x20000, CRC(29d5b953) SHA1(0c932a67e2aecffa7a1dbaa587c96214e1a2cc7f) )
	ROM_LOAD32_BYTE( "epr-12778.105", 0x180003, 0x20000, CRC(2fb68e07) SHA1(8685e72aed115cbc9c6c7511217996a573b30d16) )

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-12798.17", 0x00000, 0x10000, CRC(0587738d) SHA1(24c79b0c73616d5532a49a2c9121dfabe3a80c7d) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "epr-12799.11", 0x00000, 0x20000, CRC(bc60181c) SHA1(3c89161348db7cafb5636ab4eaba91fbd3541f90) )
	ROM_LOAD( "epr-12800.12", 0x20000, 0x20000, CRC(1158c1a3) SHA1(e1d664a203eed5a0130b39ced7bea8328f06f107) )
	ROM_LOAD( "epr-12801.13", 0x40000, 0x20000, CRC(2d6567c4) SHA1(542be9d8e91cf2df18d95f4e259cfda0560697cb) )
ROM_END

//*************************************************************************************************************************
//*************************************************************************************************************************
//*************************************************************************************************************************
//  Thunder Blade, Sega X-board
//  CPU: FD1094 (317-0056)
//
//  GAME BD NO. 834-6493-03 (Uses "MPR" mask roms) or 834-6493-05 (Uses "EPR" eproms)
//
ROM_START( thndrbld )
	ROM_REGION( 0x100000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-11405.ic58", 0x000000, 0x20000, CRC(e057dd5a) SHA1(4c032db4752dfb44dba3def5ee5377fffd94b79c) )
	ROM_LOAD16_BYTE( "epr-11406.ic63", 0x000001, 0x20000, CRC(c6b994b8) SHA1(098b2ae30c4aafea35222369d60f8e89f87639eb) )
	ROM_LOAD16_BYTE( "epr-11306.ic57", 0x040000, 0x20000, CRC(4b95f2b4) SHA1(9e0ff898a2af05c35db3551e52c7485748698c28) )
	ROM_LOAD16_BYTE( "epr-11307.ic62", 0x040001, 0x20000, CRC(2d6833e4) SHA1(b39a744370014237121f0010d18897e63f7058cf) )

	ROM_REGION( 0x2000, "mainpcb:maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0056.key", 0x0000, 0x2000, CRC(b40cd2c5) SHA1(865e70bce4f55f6702960d6eaa780b7b1f880e41) )

	ROM_REGION( 0x100000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-11390.ic20", 0x000000, 0x20000, CRC(ed988fdb) SHA1(b809b0b7dabd5cb29f5387522c6dfb993d1d0271) )
	ROM_LOAD16_BYTE( "epr-11391.ic29", 0x000001, 0x20000, CRC(12523bc1) SHA1(54635d6c4cc97cf4148dcac3bb2056fc414252f7) )
	ROM_LOAD16_BYTE( "epr-11310.ic21", 0x040000, 0x20000, CRC(5d9fa02c) SHA1(0ca71e35cf9740e38a52960f7d1ef96e7e1dda94) )
	ROM_LOAD16_BYTE( "epr-11311.ic30", 0x040001, 0x20000, CRC(483de21b) SHA1(871f0e856dcc81dcef1d9846261b3c011fa26dde) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-11314.ic154", 0x00000, 0x10000, CRC(d4f954a9) SHA1(93ee8cf8fcf4e1d0dd58329bba9b594431193449) )
	ROM_LOAD( "epr-11315.ic153", 0x10000, 0x10000, CRC(35813088) SHA1(ea1ec982d1509efb26e7b6a150825a6a905efed9) )
	ROM_LOAD( "epr-11316.ic152", 0x20000, 0x10000, CRC(84290dff) SHA1(c13fb6ef12a991f79a95072f953a02b5c992aa2d) )

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "epr-11323.ic90",  0x000000, 0x20000, CRC(27e40735) SHA1(284ddb88efe741fb78199ea619c9b230ee689803) )
	ROM_LOAD32_BYTE( "epr-11322.ic94",  0x000001, 0x20000, CRC(10364d74) SHA1(393b19a972b5d8817ffd438f13ded73cd58ebe56) )
	ROM_LOAD32_BYTE( "epr-11321.ic98",  0x000002, 0x20000, CRC(8e738f58) SHA1(9f2dceebf01e582cf60f072ae411000d8503894b) )
	ROM_LOAD32_BYTE( "epr-11320.ic102", 0x000003, 0x20000, CRC(a95c76b8) SHA1(cda62f3c25b9414a523c2fc5d109031ed560069e) )
	ROM_LOAD32_BYTE( "epr-11327.ic91",  0x080000, 0x20000, CRC(deae90f1) SHA1(c73c23bab949041242302cec13d653dcc71bb944) )
	ROM_LOAD32_BYTE( "epr-11326.ic95",  0x080001, 0x20000, CRC(29198403) SHA1(3ecf315a0e6b3ed5005f8bdcb2e2a884c8b176c7) )
	ROM_LOAD32_BYTE( "epr-11325.ic99",  0x080002, 0x20000, CRC(b9e98ae9) SHA1(c4932e2590b10d54fa8ded94593dc4203fccc60d) )
	ROM_LOAD32_BYTE( "epr-11324.ic103", 0x080003, 0x20000, CRC(9742b552) SHA1(922032264d469e943dfbcaaf57464efc638fcf73) )
	ROM_LOAD32_BYTE( "epr-11331.ic92",  0x100000, 0x20000, CRC(3a2c042e) SHA1(c296ff222d156d3bdcb42bef321831f502830fd6) )
	ROM_LOAD32_BYTE( "epr-11330.ic96",  0x100001, 0x20000, CRC(aa7c70c5) SHA1(b6fea17392b7821b8b3bba78002f9c1604f09edc) )
	ROM_LOAD32_BYTE( "epr-11329.ic100", 0x100002, 0x20000, CRC(31b20257) SHA1(7ce10a94bce67b2d15d7b576b0f7d47389dc8948) )
	ROM_LOAD32_BYTE( "epr-11328.ic104", 0x100003, 0x20000, CRC(da39e89c) SHA1(526549ce9112754c82743552eeebec63fe7ad968) )
	ROM_LOAD32_BYTE( "epr-11395.ic93",  0x180000, 0x20000, CRC(90775579) SHA1(15a86071a105da40ec9c0c0074e342231fc030d0) ) //
	ROM_LOAD32_BYTE( "epr-11394.ic97",  0x180001, 0x20000, CRC(5f2783be) SHA1(424510153a91902901f321f39738a862d6fba8e7) ) // different numbers?
	ROM_LOAD32_BYTE( "epr-11393.ic101", 0x180002, 0x20000, CRC(525e2e1d) SHA1(6fd09f775e7e6cad8078513d1af0a8ff40fb1360) ) // replaced from original rev?
	ROM_LOAD32_BYTE( "epr-11392.ic105", 0x180003, 0x20000, CRC(b4a382f7) SHA1(c03a05ba521f654db1a9c5f5717b7a15e5a29d4e) ) //

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // Road Data
	ROM_LOAD( "epr-11313.ic29", 0x00000, 0x10000, CRC(6a56c4c3) SHA1(c1b8023cb2ba4e96be052031c24b6ae424225c71) )

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-11396.ic17", 0x00000, 0x10000, CRC(d37b54a4) SHA1(c230fe7241a1f13ca13506d1492f348f506c40a7) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "epr-11317.ic11", 0x00000, 0x20000, CRC(d4e7ac1f) SHA1(ec5d6e4949938adf56e5613801ae56ff2c3dede5) )
	ROM_LOAD( "epr-11318.ic12", 0x20000, 0x20000, CRC(70d3f02c) SHA1(391aac2bc5673e06150de27e19c7c6359da8ca82) )
	ROM_LOAD( "epr-11319.ic13", 0x40000, 0x20000, CRC(50d9242e) SHA1(a106371bf680c3088ec61f07fc5c4ce467973c15) )
ROM_END

ROM_START( thndrbldd )
	ROM_REGION( 0x100000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bootleg_epr-11405.ic58", 0x000000, 0x20000, CRC(1642fd59) SHA1(92b95d97b1eef770983c993d357e06ecf6a2b29c) )
	ROM_LOAD16_BYTE( "bootleg_epr-11406.ic63", 0x000001, 0x20000, CRC(aa87dd75) SHA1(4c61dfef69a68d9cab8fed0d2cbb28b751319049) )
	ROM_LOAD16_BYTE( "epr-11306.ic57", 0x040000, 0x20000, CRC(4b95f2b4) SHA1(9e0ff898a2af05c35db3551e52c7485748698c28) )
	ROM_LOAD16_BYTE( "epr-11307.ic62", 0x040001, 0x20000, CRC(2d6833e4) SHA1(b39a744370014237121f0010d18897e63f7058cf) )

	ROM_REGION( 0x100000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-11390.ic20", 0x000000, 0x20000, CRC(ed988fdb) SHA1(b809b0b7dabd5cb29f5387522c6dfb993d1d0271) )
	ROM_LOAD16_BYTE( "epr-11391.ic29", 0x000001, 0x20000, CRC(12523bc1) SHA1(54635d6c4cc97cf4148dcac3bb2056fc414252f7) )
	ROM_LOAD16_BYTE( "epr-11310.ic21", 0x040000, 0x20000, CRC(5d9fa02c) SHA1(0ca71e35cf9740e38a52960f7d1ef96e7e1dda94) )
	ROM_LOAD16_BYTE( "epr-11311.ic30", 0x040001, 0x20000, CRC(483de21b) SHA1(871f0e856dcc81dcef1d9846261b3c011fa26dde) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-11314.ic154", 0x00000, 0x10000, CRC(d4f954a9) SHA1(93ee8cf8fcf4e1d0dd58329bba9b594431193449) )
	ROM_LOAD( "epr-11315.ic153", 0x10000, 0x10000, CRC(35813088) SHA1(ea1ec982d1509efb26e7b6a150825a6a905efed9) )
	ROM_LOAD( "epr-11316.ic152", 0x20000, 0x10000, CRC(84290dff) SHA1(c13fb6ef12a991f79a95072f953a02b5c992aa2d) )

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "epr-11323.ic90",  0x000000, 0x20000, CRC(27e40735) SHA1(284ddb88efe741fb78199ea619c9b230ee689803) )
	ROM_LOAD32_BYTE( "epr-11322.ic94",  0x000001, 0x20000, CRC(10364d74) SHA1(393b19a972b5d8817ffd438f13ded73cd58ebe56) )
	ROM_LOAD32_BYTE( "epr-11321.ic98",  0x000002, 0x20000, CRC(8e738f58) SHA1(9f2dceebf01e582cf60f072ae411000d8503894b) )
	ROM_LOAD32_BYTE( "epr-11320.ic102", 0x000003, 0x20000, CRC(a95c76b8) SHA1(cda62f3c25b9414a523c2fc5d109031ed560069e) )
	ROM_LOAD32_BYTE( "epr-11327.ic91",  0x080000, 0x20000, CRC(deae90f1) SHA1(c73c23bab949041242302cec13d653dcc71bb944) )
	ROM_LOAD32_BYTE( "epr-11326.ic95",  0x080001, 0x20000, CRC(29198403) SHA1(3ecf315a0e6b3ed5005f8bdcb2e2a884c8b176c7) )
	ROM_LOAD32_BYTE( "epr-11325.ic99",  0x080002, 0x20000, CRC(b9e98ae9) SHA1(c4932e2590b10d54fa8ded94593dc4203fccc60d) )
	ROM_LOAD32_BYTE( "epr-11324.ic103", 0x080003, 0x20000, CRC(9742b552) SHA1(922032264d469e943dfbcaaf57464efc638fcf73) )
	ROM_LOAD32_BYTE( "epr-11331.ic92",  0x100000, 0x20000, CRC(3a2c042e) SHA1(c296ff222d156d3bdcb42bef321831f502830fd6) )
	ROM_LOAD32_BYTE( "epr-11330.ic96",  0x100001, 0x20000, CRC(aa7c70c5) SHA1(b6fea17392b7821b8b3bba78002f9c1604f09edc) )
	ROM_LOAD32_BYTE( "epr-11329.ic100", 0x100002, 0x20000, CRC(31b20257) SHA1(7ce10a94bce67b2d15d7b576b0f7d47389dc8948) )
	ROM_LOAD32_BYTE( "epr-11328.ic104", 0x100003, 0x20000, CRC(da39e89c) SHA1(526549ce9112754c82743552eeebec63fe7ad968) )
	ROM_LOAD32_BYTE( "epr-11395.ic93",  0x180000, 0x20000, CRC(90775579) SHA1(15a86071a105da40ec9c0c0074e342231fc030d0) ) //
	ROM_LOAD32_BYTE( "epr-11394.ic97",  0x180001, 0x20000, CRC(5f2783be) SHA1(424510153a91902901f321f39738a862d6fba8e7) ) // different numbers?
	ROM_LOAD32_BYTE( "epr-11393.ic101", 0x180002, 0x20000, CRC(525e2e1d) SHA1(6fd09f775e7e6cad8078513d1af0a8ff40fb1360) ) // replaced from original rev?
	ROM_LOAD32_BYTE( "epr-11392.ic105", 0x180003, 0x20000, CRC(b4a382f7) SHA1(c03a05ba521f654db1a9c5f5717b7a15e5a29d4e) ) //

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // Road Data
	ROM_LOAD( "epr-11313.ic29", 0x00000, 0x10000, CRC(6a56c4c3) SHA1(c1b8023cb2ba4e96be052031c24b6ae424225c71) )

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-11396.ic17", 0x00000, 0x10000, CRC(d37b54a4) SHA1(c230fe7241a1f13ca13506d1492f348f506c40a7) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "epr-11317.ic11", 0x00000, 0x20000, CRC(d4e7ac1f) SHA1(ec5d6e4949938adf56e5613801ae56ff2c3dede5) )
	ROM_LOAD( "epr-11318.ic12", 0x20000, 0x20000, CRC(70d3f02c) SHA1(391aac2bc5673e06150de27e19c7c6359da8ca82) )
	ROM_LOAD( "epr-11319.ic13", 0x40000, 0x20000, CRC(50d9242e) SHA1(a106371bf680c3088ec61f07fc5c4ce467973c15) )
ROM_END

//*************************************************************************************************************************
//  Thunder Blade (Japan), Sega X-board
//  CPU: MC68000
//
//  GAME BD NO. 834-6493-03 (Uses "MPR" mask roms) or 834-6493-05 (Uses "EPR" eproms)
//
ROM_START( thndrbld1 )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-11304.ic58", 0x000000, 0x20000, CRC(a90630ef) SHA1(8f29e020119b2243b1c95e15546af1773327ae85) ) // patched?
	ROM_LOAD16_BYTE( "epr-11305.ic63", 0x000001, 0x20000, CRC(9ba3ef61) SHA1(f75748b37ce35b0ef881804f73417643068dfbb2) ) // patched?
	ROM_LOAD16_BYTE( "epr-11306.ic57", 0x040000, 0x20000, CRC(4b95f2b4) SHA1(9e0ff898a2af05c35db3551e52c7485748698c28) )
	ROM_LOAD16_BYTE( "epr-11307.ic62", 0x040001, 0x20000, CRC(2d6833e4) SHA1(b39a744370014237121f0010d18897e63f7058cf) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-11308.ic20", 0x00000, 0x20000, CRC(7956c238) SHA1(4608225cfd6ea3d38317cbe970f26a5fc2f8e320) )
	ROM_LOAD16_BYTE( "epr-11309.ic29", 0x00001, 0x20000, CRC(c887f620) SHA1(644c47cc2cf75cbe489ea084c13c59d94631e83f) )
	ROM_LOAD16_BYTE( "epr-11310.ic21", 0x040000, 0x20000, CRC(5d9fa02c) SHA1(0ca71e35cf9740e38a52960f7d1ef96e7e1dda94) )
	ROM_LOAD16_BYTE( "epr-11311.ic30", 0x040001, 0x20000, CRC(483de21b) SHA1(871f0e856dcc81dcef1d9846261b3c011fa26dde) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-11314.ic154", 0x00000, 0x10000, CRC(d4f954a9) SHA1(93ee8cf8fcf4e1d0dd58329bba9b594431193449) )
	ROM_LOAD( "epr-11315.ic153", 0x10000, 0x10000, CRC(35813088) SHA1(ea1ec982d1509efb26e7b6a150825a6a905efed9) )
	ROM_LOAD( "epr-11316.ic152", 0x20000, 0x10000, CRC(84290dff) SHA1(c13fb6ef12a991f79a95072f953a02b5c992aa2d) )

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "epr-11323.ic90",  0x000000, 0x20000, CRC(27e40735) SHA1(284ddb88efe741fb78199ea619c9b230ee689803) )
	ROM_LOAD32_BYTE( "epr-11322.ic94",  0x000001, 0x20000, CRC(10364d74) SHA1(393b19a972b5d8817ffd438f13ded73cd58ebe56) )
	ROM_LOAD32_BYTE( "epr-11321.ic98",  0x000002, 0x20000, CRC(8e738f58) SHA1(9f2dceebf01e582cf60f072ae411000d8503894b) )
	ROM_LOAD32_BYTE( "epr-11320.ic102", 0x000003, 0x20000, CRC(a95c76b8) SHA1(cda62f3c25b9414a523c2fc5d109031ed560069e) )
	ROM_LOAD32_BYTE( "epr-11327.ic91",  0x080000, 0x20000, CRC(deae90f1) SHA1(c73c23bab949041242302cec13d653dcc71bb944) )
	ROM_LOAD32_BYTE( "epr-11326.ic95",  0x080001, 0x20000, CRC(29198403) SHA1(3ecf315a0e6b3ed5005f8bdcb2e2a884c8b176c7) )
	ROM_LOAD32_BYTE( "epr-11325.ic99",  0x080002, 0x20000, CRC(b9e98ae9) SHA1(c4932e2590b10d54fa8ded94593dc4203fccc60d) )
	ROM_LOAD32_BYTE( "epr-11324.ic103", 0x080003, 0x20000, CRC(9742b552) SHA1(922032264d469e943dfbcaaf57464efc638fcf73) )
	ROM_LOAD32_BYTE( "epr-11331.ic92",  0x100000, 0x20000, CRC(3a2c042e) SHA1(c296ff222d156d3bdcb42bef321831f502830fd6) )
	ROM_LOAD32_BYTE( "epr-11330.ic96",  0x100001, 0x20000, CRC(aa7c70c5) SHA1(b6fea17392b7821b8b3bba78002f9c1604f09edc) )
	ROM_LOAD32_BYTE( "epr-11329.ic100", 0x100002, 0x20000, CRC(31b20257) SHA1(7ce10a94bce67b2d15d7b576b0f7d47389dc8948) )
	ROM_LOAD32_BYTE( "epr-11328.ic104", 0x100003, 0x20000, CRC(da39e89c) SHA1(526549ce9112754c82743552eeebec63fe7ad968) )
	ROM_LOAD32_BYTE( "epr-11335.ic93",  0x180000, 0x20000, CRC(f19b3e86) SHA1(40e8ba10cd5020782b82279974d13330a9c015e5) )
	ROM_LOAD32_BYTE( "epr-11334.ic97",  0x180001, 0x20000, CRC(348f91c7) SHA1(03da6a4fee1fdea76058be4bc5ffcde7a79e5948) )
	ROM_LOAD32_BYTE( "epr-11333.ic101", 0x180002, 0x20000, CRC(05a2333f) SHA1(70f213945fa7fe056fe17a02558638e87f2c001e) )
	ROM_LOAD32_BYTE( "epr-11332.ic105", 0x180003, 0x20000, CRC(dc089ec6) SHA1(d72390c45138a507e79af112addbc015560fc248) )

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // Road Data
	ROM_LOAD( "epr-11313.ic29", 0x00000, 0x10000, CRC(6a56c4c3) SHA1(c1b8023cb2ba4e96be052031c24b6ae424225c71) )

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-11312.ic17",   0x00000, 0x10000, CRC(3b974ed2) SHA1(cf18a2d0f01643c747a884bf00e5b7037ba2e64a) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "epr-11317.ic11", 0x00000, 0x20000, CRC(d4e7ac1f) SHA1(ec5d6e4949938adf56e5613801ae56ff2c3dede5) )
	ROM_LOAD( "epr-11318.ic12", 0x20000, 0x20000, CRC(70d3f02c) SHA1(391aac2bc5673e06150de27e19c7c6359da8ca82) )
	ROM_LOAD( "epr-11319.ic13", 0x40000, 0x20000, CRC(50d9242e) SHA1(a106371bf680c3088ec61f07fc5c4ce467973c15) )
ROM_END


//*************************************************************************************************************************
//*************************************************************************************************************************
//*************************************************************************************************************************
//  Last Survivor, Sega X-board
//  CPU: FD1094 (317-0083)
//
ROM_START( lastsurv )
	ROM_REGION( 0x100000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-12046.ic58", 0x000000, 0x20000, CRC(f94f3a1a) SHA1(f509cbccb1f36ce52ed3e44d4d7b31a047050700) )
	ROM_LOAD16_BYTE( "epr-12047.ic63", 0x000001, 0x20000, CRC(1b45c116) SHA1(c46ad622a145baea52d918537fa43a2009ed0cca) )
	ROM_LOAD16_BYTE( "epr-12048.ic57", 0x040000, 0x20000, CRC(648e38ca) SHA1(e5f7fd42f49dbbddd1a812a04d8b95c1a73e640b) )
	ROM_LOAD16_BYTE( "epr-12049.ic62", 0x040001, 0x20000, CRC(6c5c4753) SHA1(6834542005bc8cad7918ae17d3764306d7f9a959) )

	ROM_REGION( 0x2000, "mainpcb:maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0083.key", 0x0000, 0x2000, CRC(dca0b9cc) SHA1(77510804d36d486ffa1e0bb5b0a36d43adc63415) )

	ROM_REGION( 0x100000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-12050.ic20", 0x000000, 0x20000, CRC(985a0f36) SHA1(bd0a93aa16565c8338db0c67b031bfa409bce5a9) )
	ROM_LOAD16_BYTE( "epr-12051.ic29", 0x000001, 0x20000, CRC(f967d5a8) SHA1(16d742da755b5b7c3c3a9f6b4baaf242e5e54441) )
	ROM_LOAD16_BYTE( "epr-12052.ic21", 0x040000, 0x20000, CRC(9f7a424d) SHA1(b8c2d3aa08ba71f08f2c1f403edac16bf4334184) )
	ROM_LOAD16_BYTE( "epr-12053.ic30", 0x040001, 0x20000, CRC(efcf30f6) SHA1(55cd42c78f117995a89844529386ae3d11c718c1) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-12055.ic154", 0x00000, 0x10000, CRC(150014a4) SHA1(9fbab916ee903c541f61014e137ccecd071b5c3a) )
	ROM_LOAD( "epr-12056.ic153", 0x10000, 0x10000, CRC(3cd4c306) SHA1(b0f178688870c67936a15383024c392072e3bc66) )
	ROM_LOAD( "epr-12057.ic152", 0x20000, 0x10000, CRC(37e91770) SHA1(69e26f4d3c4ebfaf0225a9b1c60038595929ef05) )

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "mpr-12064.ic90",  0x000000, 0x20000, CRC(84562a69) SHA1(815189a65065def213ef171fe40a44a455dfe75a) )
	ROM_LOAD32_BYTE( "mpr-12063.ic94",  0x000001, 0x20000, CRC(d163727c) SHA1(50ed2b401e107a359874dad5d86eec788f5504eb) )
	ROM_LOAD32_BYTE( "mpr-12062.ic98",  0x000002, 0x20000, CRC(6b57833b) SHA1(1d70894c81a4cd39f43067701a598d2c4fbffa58) )
	ROM_LOAD32_BYTE( "mpr-12061.ic102", 0x000003, 0x20000, CRC(8907d5ba) SHA1(f4f9a19f3c27ef02314e59294a9658e2b20d52e0) )
	ROM_LOAD32_BYTE( "epr-12068.ic91",  0x080000, 0x20000, CRC(8b12d342) SHA1(0356a413c2438e9c6c660454f03c0e24c6325f6b) )
	ROM_LOAD32_BYTE( "epr-12067.ic95",  0x080001, 0x20000, CRC(1a1cdd89) SHA1(cd725aa450efa60ecc7d4111d0690cb441633935) )
	ROM_LOAD32_BYTE( "epr-12066.ic99",  0x080002, 0x20000, CRC(a91d16b5) SHA1(501ddedf79130979c90c72882c2d96f5fd01adea) )
	ROM_LOAD32_BYTE( "epr-12065.ic103", 0x080003, 0x20000, CRC(f4ce14c6) SHA1(42221ee03f363e94bf7b6de0bd89172525500412) )
	ROM_LOAD32_BYTE( "epr-12072.ic92",  0x100000, 0x20000, CRC(222064c8) SHA1(a3914f8dabd8a3d99eaf4e03fa45e177c9f30666) )
	ROM_LOAD32_BYTE( "epr-12071.ic96",  0x100001, 0x20000, CRC(a329b78c) SHA1(33b1f27dcc5ac36fdfd7374e1edda4fc31421126) )
	ROM_LOAD32_BYTE( "epr-12070.ic100", 0x100002, 0x20000, CRC(97cc6706) SHA1(9160f100bd85f9c8b774e27a5d68e1c513111a61) )
	ROM_LOAD32_BYTE( "epr-12069.ic104", 0x100003, 0x20000, CRC(2c3ba66e) SHA1(087fbf9d17f38b06b134088d89965c8d17dd5846) )
	ROM_LOAD32_BYTE( "epr-12076.ic93",  0x180000, 0x20000, CRC(24f628e1) SHA1(abbc22282c7a9df203a8c589ddf08413d67392b1) )
	ROM_LOAD32_BYTE( "epr-12075.ic97",  0x180001, 0x20000, CRC(69b3507f) SHA1(c447ceb38b473a3f65847471ef6de559e6ecce4a) )
	ROM_LOAD32_BYTE( "epr-12074.ic101", 0x180002, 0x20000, CRC(ee6cbb73) SHA1(c68d825ded83dd06ba7b816622db3d57631b4fcc) )
	ROM_LOAD32_BYTE( "epr-12073.ic105", 0x180003, 0x20000, CRC(167e6342) SHA1(2f87074d6821a974cbb137ca2bec28fafc0df46f) )

	ROM_REGION( 0x20000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // Road Data
	// none

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-12054.ic17", 0x00000, 0x10000, CRC(e9b39216) SHA1(142764b40b4db69ff08d28338d1b12b1dd1ed0a0) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "epr-12058.ic11", 0x00000, 0x20000, CRC(4671cb46) SHA1(03ecaa4409a5b86a558313d4ccfb2334f79cff17) )
	ROM_LOAD( "epr-12059.ic12", 0x20000, 0x20000, CRC(8c99aff4) SHA1(818418e4e92f601b09fcaa0979802a2c2c85b435) )
	ROM_LOAD( "epr-12060.ic13", 0x40000, 0x20000, CRC(7ed382b3) SHA1(c87306d1b9edb8b4b97aee4af1317526750e2da2) )
ROM_END

ROM_START( lastsurvd )
	ROM_REGION( 0x100000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bootleg_epr-12046.ic58", 0x000000, 0x20000, CRC(ddef5278) SHA1(0efb4c6280f8127406d55461983137bac8f2a2c8) )
	ROM_LOAD16_BYTE( "bootleg_epr-12047.ic63", 0x000001, 0x20000, CRC(3981a891) SHA1(b25a37e2a3e55f1ee370ca99e406959fb1db13d6) )
	ROM_LOAD16_BYTE( "epr-12048.ic57", 0x040000, 0x20000, CRC(648e38ca) SHA1(e5f7fd42f49dbbddd1a812a04d8b95c1a73e640b) )
	ROM_LOAD16_BYTE( "epr-12049.ic62", 0x040001, 0x20000, CRC(6c5c4753) SHA1(6834542005bc8cad7918ae17d3764306d7f9a959) )

	ROM_REGION( 0x100000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-12050.ic20", 0x000000, 0x20000, CRC(985a0f36) SHA1(bd0a93aa16565c8338db0c67b031bfa409bce5a9) )
	ROM_LOAD16_BYTE( "epr-12051.ic29", 0x000001, 0x20000, CRC(f967d5a8) SHA1(16d742da755b5b7c3c3a9f6b4baaf242e5e54441) )
	ROM_LOAD16_BYTE( "epr-12052.ic21", 0x040000, 0x20000, CRC(9f7a424d) SHA1(b8c2d3aa08ba71f08f2c1f403edac16bf4334184) )
	ROM_LOAD16_BYTE( "epr-12053.ic30", 0x040001, 0x20000, CRC(efcf30f6) SHA1(55cd42c78f117995a89844529386ae3d11c718c1) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-12055.ic154", 0x00000, 0x10000, CRC(150014a4) SHA1(9fbab916ee903c541f61014e137ccecd071b5c3a) )
	ROM_LOAD( "epr-12056.ic153", 0x10000, 0x10000, CRC(3cd4c306) SHA1(b0f178688870c67936a15383024c392072e3bc66) )
	ROM_LOAD( "epr-12057.ic152", 0x20000, 0x10000, CRC(37e91770) SHA1(69e26f4d3c4ebfaf0225a9b1c60038595929ef05) )

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "mpr-12064.ic90",  0x000000, 0x20000, CRC(84562a69) SHA1(815189a65065def213ef171fe40a44a455dfe75a) )
	ROM_LOAD32_BYTE( "mpr-12063.ic94",  0x000001, 0x20000, CRC(d163727c) SHA1(50ed2b401e107a359874dad5d86eec788f5504eb) )
	ROM_LOAD32_BYTE( "mpr-12062.ic98",  0x000002, 0x20000, CRC(6b57833b) SHA1(1d70894c81a4cd39f43067701a598d2c4fbffa58) )
	ROM_LOAD32_BYTE( "mpr-12061.ic102", 0x000003, 0x20000, CRC(8907d5ba) SHA1(f4f9a19f3c27ef02314e59294a9658e2b20d52e0) )
	ROM_LOAD32_BYTE( "epr-12068.ic91",  0x080000, 0x20000, CRC(8b12d342) SHA1(0356a413c2438e9c6c660454f03c0e24c6325f6b) )
	ROM_LOAD32_BYTE( "epr-12067.ic95",  0x080001, 0x20000, CRC(1a1cdd89) SHA1(cd725aa450efa60ecc7d4111d0690cb441633935) )
	ROM_LOAD32_BYTE( "epr-12066.ic99",  0x080002, 0x20000, CRC(a91d16b5) SHA1(501ddedf79130979c90c72882c2d96f5fd01adea) )
	ROM_LOAD32_BYTE( "epr-12065.ic103", 0x080003, 0x20000, CRC(f4ce14c6) SHA1(42221ee03f363e94bf7b6de0bd89172525500412) )
	ROM_LOAD32_BYTE( "epr-12072.ic92",  0x100000, 0x20000, CRC(222064c8) SHA1(a3914f8dabd8a3d99eaf4e03fa45e177c9f30666) )
	ROM_LOAD32_BYTE( "epr-12071.ic96",  0x100001, 0x20000, CRC(a329b78c) SHA1(33b1f27dcc5ac36fdfd7374e1edda4fc31421126) )
	ROM_LOAD32_BYTE( "epr-12070.ic100", 0x100002, 0x20000, CRC(97cc6706) SHA1(9160f100bd85f9c8b774e27a5d68e1c513111a61) )
	ROM_LOAD32_BYTE( "epr-12069.ic104", 0x100003, 0x20000, CRC(2c3ba66e) SHA1(087fbf9d17f38b06b134088d89965c8d17dd5846) )
	ROM_LOAD32_BYTE( "epr-12076.ic93",  0x180000, 0x20000, CRC(24f628e1) SHA1(abbc22282c7a9df203a8c589ddf08413d67392b1) )
	ROM_LOAD32_BYTE( "epr-12075.ic97",  0x180001, 0x20000, CRC(69b3507f) SHA1(c447ceb38b473a3f65847471ef6de559e6ecce4a) )
	ROM_LOAD32_BYTE( "epr-12074.ic101", 0x180002, 0x20000, CRC(ee6cbb73) SHA1(c68d825ded83dd06ba7b816622db3d57631b4fcc) )
	ROM_LOAD32_BYTE( "epr-12073.ic105", 0x180003, 0x20000, CRC(167e6342) SHA1(2f87074d6821a974cbb137ca2bec28fafc0df46f) )

	ROM_REGION( 0x20000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // Road Data
	// none

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-12054.ic17", 0x00000, 0x10000, CRC(e9b39216) SHA1(142764b40b4db69ff08d28338d1b12b1dd1ed0a0) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "epr-12058.ic11", 0x00000, 0x20000, CRC(4671cb46) SHA1(03ecaa4409a5b86a558313d4ccfb2334f79cff17) )
	ROM_LOAD( "epr-12059.ic12", 0x20000, 0x20000, CRC(8c99aff4) SHA1(818418e4e92f601b09fcaa0979802a2c2c85b435) )
	ROM_LOAD( "epr-12060.ic13", 0x40000, 0x20000, CRC(7ed382b3) SHA1(c87306d1b9edb8b4b97aee4af1317526750e2da2) )
ROM_END

//*************************************************************************************************************************
//*************************************************************************************************************************
//*************************************************************************************************************************
//  Racing Hero, Sega X-board
//  CPU: FD1094 (317-0144)
//
ROM_START( rachero )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-13129.ic58", 0x00000, 0x20000,CRC(ad9f32e7) SHA1(dbcb3436782bee88dcac05d4f59c97f170a7387d) )
	ROM_LOAD16_BYTE( "epr-13130.ic63", 0x00001, 0x20000,CRC(6022777b) SHA1(965c76565d740be3355c4b403a1629cffb9fcd78) )
	ROM_LOAD16_BYTE( "epr-12855.ic57", 0x40000, 0x20000,CRC(cecf1e73) SHA1(3f8631379f32dbfda7720ef345276f9be23ada06) )
	ROM_LOAD16_BYTE( "epr-12856.ic62", 0x40001, 0x20000,CRC(da900ebb) SHA1(595ed65248185ddf8666b3f30ad6329162116448) )

	ROM_REGION( 0x2000, "mainpcb:maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0144.key", 0x0000, 0x2000, CRC(8740bbff) SHA1(de96e606c04a09258b966532fb01a6b4d4db86a6) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-12857.ic20", 0x00000, 0x20000, CRC(8a2328cc) SHA1(c34498428ddfb3eeb986f4153a6165a685d8fc8a) )
	ROM_LOAD16_BYTE( "epr-12858.ic29", 0x00001, 0x20000, CRC(38a248b7) SHA1(a17672123665403c1c56fedab6c8abf44b1131f9) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-12879.ic154", 0x00000, 0x10000, CRC(c1a9de7a) SHA1(2425456a9d4ba92e1f2da6c2f164a6d5a5dee7c7) )
	ROM_LOAD( "epr-12880.ic153", 0x10000, 0x10000, CRC(27ff04a5) SHA1(b554a6e060f4803100be8efa52977b503eb0f31d) )
	ROM_LOAD( "epr-12881.ic152", 0x20000, 0x10000, CRC(72f14491) SHA1(b7a6cbd08470a5edda77cdd0337abd502c4905fd) )

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "epr-12872.ic90",  0x000000, 0x20000, CRC(68d56139) SHA1(b5f32edbda10c31d52f90defea2bae226676069f) )
	ROM_LOAD32_BYTE( "epr-12873.ic94",  0x000001, 0x20000, CRC(3d3ec450) SHA1(ac96ad8c7b365478bd1e5826a073e242f1208247) )
	ROM_LOAD32_BYTE( "epr-12874.ic98",  0x000002, 0x20000, CRC(7d6bde23) SHA1(88b12ec6386cdad60b0028b72033a0037a0cdbdb) )
	ROM_LOAD32_BYTE( "epr-12875.ic102", 0x000003, 0x20000, CRC(e33092bf) SHA1(31e211e25adac0a98befb459093f23c905fbc1e6) )
	ROM_LOAD32_BYTE( "epr-12868.ic91",  0x080000, 0x20000, CRC(96289583) SHA1(4d37e67860bc0e6ef69f0a0775c28f6f2fd6875e) )
	ROM_LOAD32_BYTE( "epr-12869.ic95",  0x080001, 0x20000, CRC(2ef0de02) SHA1(11ee3d77df2cddd3156da52e50565505f95f4cd4) )
	ROM_LOAD32_BYTE( "epr-12870.ic99",  0x080002, 0x20000, CRC(c76630e1) SHA1(7b76e4819990e147639d6b930b17b6fa10df191c) )
	ROM_LOAD32_BYTE( "epr-12871.ic103", 0x080003, 0x20000, CRC(23401b1a) SHA1(eaf465ffda84bdb83cc85daf781275bada396aab) )
	ROM_LOAD32_BYTE( "epr-12864.ic92",  0x100000, 0x20000, CRC(77d6cff4) SHA1(1e625204801d03369311844efb26d22216253ac4) )
	ROM_LOAD32_BYTE( "epr-12865.ic96",  0x100001, 0x20000, CRC(1e7e685b) SHA1(532fe361357383aa9dada833cbe31716c58001e5) )
	ROM_LOAD32_BYTE( "epr-12866.ic100", 0x100002, 0x20000, CRC(fdf31329) SHA1(9c229a0f9d8b8114acfe4f17b45a9b8640560b3e) )
	ROM_LOAD32_BYTE( "epr-12867.ic104", 0x100003, 0x20000, CRC(b25e37fd) SHA1(fef5bfe4690b3203b83fd565d883b2c63f439633) )
	ROM_LOAD32_BYTE( "epr-12860.ic93",  0x180000, 0x20000, CRC(86b64119) SHA1(d39aedad0f05e500e33af888126bd2fc22539141) )
	ROM_LOAD32_BYTE( "epr-12861.ic97",  0x180001, 0x20000, CRC(bccff19b) SHA1(32c3f7802a12be02a114b78cd898c46fcb1c0a61) )
	ROM_LOAD32_BYTE( "epr-12862.ic101", 0x180002, 0x20000, CRC(7d4c3b05) SHA1(4e25a077b403549c681c5047912d0e28f4c07720) )
	ROM_LOAD32_BYTE( "epr-12863.ic105", 0x180003, 0x20000, CRC(85095053) SHA1(f93194ecc0300956280cc0515b3e3ba2c9f71364) )

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // ground data
	// none

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-12859.ic17",    0x00000, 0x10000, CRC(d57881da) SHA1(75b7f331ea8c2e33d6236e0c8fc8dabe5eef8160) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "epr-12876.ic11",    0x00000, 0x20000, CRC(f72a34a0) SHA1(28f7d077c24352557da3a91a7e49b0c5b79f2a2e) )
	ROM_LOAD( "epr-12877.ic12",    0x20000, 0x20000, CRC(18c1b6d2) SHA1(860cbb96999ab76c40ce96996bba70c42d845abc) )
	ROM_LOAD( "epr-12878.ic13",    0x40000, 0x20000, CRC(7c212c15) SHA1(360b332d2fb32d88949ff8b357a863ffaaca39c2) )
ROM_END

ROM_START( racherod )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bootleg_epr-13129.ic58", 0x00000, 0x20000, CRC(82ee7312) SHA1(4d011529b538885bbc3bb1cb23048b785d3be318) )
	ROM_LOAD16_BYTE( "bootleg_epr-13130.ic63", 0x00001, 0x20000, CRC(53fb8649) SHA1(8b66d6e2018f92c7c992944ed5d4a685d9f13a6d) )
	ROM_LOAD16_BYTE( "epr-12855.ic57", 0x40000, 0x20000,CRC(cecf1e73) SHA1(3f8631379f32dbfda7720ef345276f9be23ada06) )
	ROM_LOAD16_BYTE( "epr-12856.ic62", 0x40001, 0x20000,CRC(da900ebb) SHA1(595ed65248185ddf8666b3f30ad6329162116448) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-12857.ic20", 0x00000, 0x20000, CRC(8a2328cc) SHA1(c34498428ddfb3eeb986f4153a6165a685d8fc8a) )
	ROM_LOAD16_BYTE( "epr-12858.ic29", 0x00001, 0x20000, CRC(38a248b7) SHA1(a17672123665403c1c56fedab6c8abf44b1131f9) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-12879.ic154", 0x00000, 0x10000, CRC(c1a9de7a) SHA1(2425456a9d4ba92e1f2da6c2f164a6d5a5dee7c7) )
	ROM_LOAD( "epr-12880.ic153", 0x10000, 0x10000, CRC(27ff04a5) SHA1(b554a6e060f4803100be8efa52977b503eb0f31d) )
	ROM_LOAD( "epr-12881.ic152", 0x20000, 0x10000, CRC(72f14491) SHA1(b7a6cbd08470a5edda77cdd0337abd502c4905fd) )

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "epr-12872.ic90",  0x000000, 0x20000, CRC(68d56139) SHA1(b5f32edbda10c31d52f90defea2bae226676069f) )
	ROM_LOAD32_BYTE( "epr-12873.ic94",  0x000001, 0x20000, CRC(3d3ec450) SHA1(ac96ad8c7b365478bd1e5826a073e242f1208247) )
	ROM_LOAD32_BYTE( "epr-12874.ic98",  0x000002, 0x20000, CRC(7d6bde23) SHA1(88b12ec6386cdad60b0028b72033a0037a0cdbdb) )
	ROM_LOAD32_BYTE( "epr-12875.ic102", 0x000003, 0x20000, CRC(e33092bf) SHA1(31e211e25adac0a98befb459093f23c905fbc1e6) )
	ROM_LOAD32_BYTE( "epr-12868.ic91",  0x080000, 0x20000, CRC(96289583) SHA1(4d37e67860bc0e6ef69f0a0775c28f6f2fd6875e) )
	ROM_LOAD32_BYTE( "epr-12869.ic95",  0x080001, 0x20000, CRC(2ef0de02) SHA1(11ee3d77df2cddd3156da52e50565505f95f4cd4) )
	ROM_LOAD32_BYTE( "epr-12870.ic99",  0x080002, 0x20000, CRC(c76630e1) SHA1(7b76e4819990e147639d6b930b17b6fa10df191c) )
	ROM_LOAD32_BYTE( "epr-12871.ic103", 0x080003, 0x20000, CRC(23401b1a) SHA1(eaf465ffda84bdb83cc85daf781275bada396aab) )
	ROM_LOAD32_BYTE( "epr-12864.ic92",  0x100000, 0x20000, CRC(77d6cff4) SHA1(1e625204801d03369311844efb26d22216253ac4) )
	ROM_LOAD32_BYTE( "epr-12865.ic96",  0x100001, 0x20000, CRC(1e7e685b) SHA1(532fe361357383aa9dada833cbe31716c58001e5) )
	ROM_LOAD32_BYTE( "epr-12866.ic100", 0x100002, 0x20000, CRC(fdf31329) SHA1(9c229a0f9d8b8114acfe4f17b45a9b8640560b3e) )
	ROM_LOAD32_BYTE( "epr-12867.ic104", 0x100003, 0x20000, CRC(b25e37fd) SHA1(fef5bfe4690b3203b83fd565d883b2c63f439633) )
	ROM_LOAD32_BYTE( "epr-12860.ic93",  0x180000, 0x20000, CRC(86b64119) SHA1(d39aedad0f05e500e33af888126bd2fc22539141) )
	ROM_LOAD32_BYTE( "epr-12861.ic97",  0x180001, 0x20000, CRC(bccff19b) SHA1(32c3f7802a12be02a114b78cd898c46fcb1c0a61) )
	ROM_LOAD32_BYTE( "epr-12862.ic101", 0x180002, 0x20000, CRC(7d4c3b05) SHA1(4e25a077b403549c681c5047912d0e28f4c07720) )
	ROM_LOAD32_BYTE( "epr-12863.ic105", 0x180003, 0x20000, CRC(85095053) SHA1(f93194ecc0300956280cc0515b3e3ba2c9f71364) )

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // ground data
	// none

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-12859.ic17",    0x00000, 0x10000, CRC(d57881da) SHA1(75b7f331ea8c2e33d6236e0c8fc8dabe5eef8160) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "epr-12876.ic11",    0x00000, 0x20000, CRC(f72a34a0) SHA1(28f7d077c24352557da3a91a7e49b0c5b79f2a2e) )
	ROM_LOAD( "epr-12877.ic12",    0x20000, 0x20000, CRC(18c1b6d2) SHA1(860cbb96999ab76c40ce96996bba70c42d845abc) )
	ROM_LOAD( "epr-12878.ic13",    0x40000, 0x20000, CRC(7c212c15) SHA1(360b332d2fb32d88949ff8b357a863ffaaca39c2) )
ROM_END

//*************************************************************************************************************************
//*************************************************************************************************************************
//*************************************************************************************************************************
//  Super Monaco GP, Sega X-board
//  CPU: FD1094 (317-0126a)
//  This set is coming from a twin.
//
//  This set has an extra link board (834-7112) or 171-5729-01 under the main board with a Z80
//
//  Xtal is 16.000 Mhz.
//
//  It has also one eprom (Epr 12587.14) two pal 16L8 (315-5336 and 315-5337) and two
//  fujitsu IC MB89372P and MB8421-12LP
//
//  Main Board : (834-8180-02)
//
//  epr-12576A.20 (68000)
//  epr-12577A.29 (68000)
//  epr-12563B.58 FD1094 317-0126A
//  epr-12564B.63 FD1094 317-0126A
//  epr-12609.93
//  epr-12610.97
//  epr-12611.101
//  epr-12612.105
//  mpr-12417.92
//  mpr-12418.96
//  mpr-12419.100
//  mpr-12420.104
//  mpr-12421.91
//  mpr-12422.95
//  mpr-12423.99
//  mpr-12424.103
//  mpr-12425.90
//  mpr-12426.94
//  mpr-12427.98
//  mpr-12428.102
//  epr-12429.154
//  epr-12430.153
//  epr-12431.152
//  epr-12436.17
//  mpr-12437.11
//  mpr-12438.12
//  mpr-12439.13
//
//  Link Board :
//
//  Ep12587.14
//
ROM_START( smgp )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-12563b.58", 0x00000, 0x20000, CRC(baf1f333) SHA1(f91a7a311237b9940a44b815716d4226a7ae1e8b) )
	ROM_LOAD16_BYTE( "epr-12564b.63", 0x00001, 0x20000, CRC(b5191af0) SHA1(d6fb19552e4816eefe751907ec55a2e07ad24879) )

	ROM_REGION( 0x2000, "mainpcb:maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0126a.key", 0x0000, 0x2000, CRC(2abc1982) SHA1(cc4c36e6ba52431df17c6e36ba08d3a89be7b7e7) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-12576a.20", 0x00000, 0x20000, CRC(2c9599c1) SHA1(79206f38c2976bd9299ed37bf62ac26dd3fba801) )
	ROM_LOAD16_BYTE( "epr-12577a.29", 0x00001, 0x20000, CRC(abf9a50b) SHA1(e367b305cd45900aae4849af4904543f05456dc6) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-12429.154", 0x00000, 0x10000, CRC(5851e614) SHA1(3dc97237ede2c6125e92ea6efc68a748d0ec69be) )
	ROM_LOAD( "epr-12430.153", 0x10000, 0x10000, CRC(05e00134) SHA1(8baaa80815d5dabd38dc8600e357975b96d23b95) )
	ROM_LOAD( "epr-12431.152", 0x20000, 0x10000, CRC(35572f4a) SHA1(d66456ecf7b59f81736fb873c553926b56bb3977))

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "mpr-12425.90",  0x000000, 0x20000, CRC(14bf2a15) SHA1(84db3ac09e4a8fe470ac051d8d5de1814b48bc72) )
	ROM_LOAD32_BYTE( "mpr-12426.94",  0x000001, 0x20000, CRC(28b60dc0) SHA1(ad69d449434853445a076319a55a29014217a100) )
	ROM_LOAD32_BYTE( "mpr-12427.98",  0x000002, 0x20000, CRC(0a367928) SHA1(bcb558b7c23906397e66a7f046b09eb5036c0888) )
	ROM_LOAD32_BYTE( "mpr-12428.102", 0x000003, 0x20000, CRC(efa80ad5) SHA1(9bc7c3fb60cc076f29a0af487d58e5b48f1c4b06) )
	ROM_LOAD32_BYTE( "mpr-12421.91",  0x080000, 0x20000, CRC(25f46140) SHA1(ea75e364cf52636d100158f79be627e36da8c327) )
	ROM_LOAD32_BYTE( "mpr-12422.95",  0x080001, 0x20000, CRC(cb51c8f6) SHA1(5af56ae1916c3212b8d5b9e4bccbbe1916694f89) )
	ROM_LOAD32_BYTE( "mpr-12423.99",  0x080002, 0x20000, CRC(0be9818e) SHA1(637a8201416e73d53f7e2502ea0a5277e43c167d) )
	ROM_LOAD32_BYTE( "mpr-12424.103", 0x080003, 0x20000, CRC(0ce00dfc) SHA1(3b1990977ec7ad4c3bea66527707cff2cd8d5a98) )
	ROM_LOAD32_BYTE( "mpr-12417.92",  0x100000, 0x20000, CRC(a806eabf) SHA1(1a61a2135d92b42ee131fd3240bc8a17a96696ab) )
	ROM_LOAD32_BYTE( "mpr-12418.96",  0x100001, 0x20000, CRC(ed1a0f2b) SHA1(1aa87292ca0465fa129d6be81d95dbb77332ecab) )
	ROM_LOAD32_BYTE( "mpr-12419.100", 0x100002, 0x20000, CRC(ce4568cb) SHA1(1ed66e74ce94d41593b498827d9cc243f775d4ba) )
	ROM_LOAD32_BYTE( "mpr-12420.104", 0x100003, 0x20000, CRC(679442eb) SHA1(f88ef0219497f955d8db6783f3636dad52928f46) )
	ROM_LOAD32_BYTE( "epr-12609.93",  0x180000, 0x20000, CRC(a867812f) SHA1(f8950bf794b6c2ec767ffff837d28917b636dbe7) ) //
	ROM_LOAD32_BYTE( "epr-12610.97",  0x180001, 0x20000, CRC(53b99417) SHA1(ab72d35c88695c777d24c5557e5d3ea2d446e51b) ) //
	ROM_LOAD32_BYTE( "epr-12611.101", 0x180002, 0x20000, CRC(bd5c6ab0) SHA1(7632dc4daa8eabe74769369856a8ba451e5bd420) ) // these differ from japan set
	ROM_LOAD32_BYTE( "epr-12612.105", 0x180003, 0x20000, CRC(ac86e890) SHA1(7720c1c8df6de5de50254e97772c15161b796031) ) //

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none??

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-12436.17",    0x00000, 0x10000, CRC(16ec5f0a) SHA1(307b7388b5c36fd4bc2a61f7941db44858e03c5c) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "mpr-12437.11",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) )
	ROM_LOAD( "mpr-12438.12",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) )
	ROM_LOAD( "mpr-12439.13",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) ) // not used in deluxe

	ROM_REGION( 0x10000, "mainpcb:soundcpu2", 0 ) // z80 on extra sound board
	ROM_LOAD( "epr-12535.8",     0x00000, 0x10000, CRC(80453597) SHA1(d3fee7bb4a8964f5cf1cdae80fc3dde06c947839) ) // taken from deluxe cabinet dump

	ROM_REGION( 0x80000, "mainpcb:pcm2", ROMREGION_ERASEFF ) // Sega PCM sound data on extra sound board (same as on main board..)
	ROM_LOAD( "mpr-12437.20",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) ) // taken from deluxe cabinet dump
	ROM_LOAD( "mpr-12438.21",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) ) // "
	ROM_LOAD( "mpr-12439.22",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) ) // "

	ROM_REGION( 0x10000, "mainpcb:commcpu", 0 ) // z80 on network board
	ROM_LOAD( "epr-12587.14",    0x00000, 0x08000, CRC(2afe648b) SHA1(b5bf86f3acbcc23c136185110acecf2c971294fa) )

	ROM_REGION( 0x10000, "mainpcb:motorcpu", 0 ) // z80 on air board
	ROM_LOAD( "epr-12505.8",     0x00000, 0x08000, CRC(5020788a) SHA1(ed6d1dfb8b6a62d17469e3d09a5b5b864c6b486c) ) // taken from deluxe cabinet dump
ROM_END

ROM_START( smgpd )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bootleg_epr-12563b.58", 0x00000, 0x20000, CRC(af30e3cd) SHA1(b05a4f8be701fada6d55a042079f4b2067b52cb2) )
	ROM_LOAD16_BYTE( "bootleg_epr-12564b.63", 0x00001, 0x20000, CRC(eb7cadfe) SHA1(58c2d05cd21795c1d5d603179decc3b861ef438f) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-12576a.20", 0x00000, 0x20000, CRC(2c9599c1) SHA1(79206f38c2976bd9299ed37bf62ac26dd3fba801) )
	ROM_LOAD16_BYTE( "epr-12577a.29", 0x00001, 0x20000, CRC(abf9a50b) SHA1(e367b305cd45900aae4849af4904543f05456dc6) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-12429.154", 0x00000, 0x10000, CRC(5851e614) SHA1(3dc97237ede2c6125e92ea6efc68a748d0ec69be) )
	ROM_LOAD( "epr-12430.153", 0x10000, 0x10000, CRC(05e00134) SHA1(8baaa80815d5dabd38dc8600e357975b96d23b95) )
	ROM_LOAD( "epr-12431.152", 0x20000, 0x10000, CRC(35572f4a) SHA1(d66456ecf7b59f81736fb873c553926b56bb3977))

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "mpr-12425.90",  0x000000, 0x20000, CRC(14bf2a15) SHA1(84db3ac09e4a8fe470ac051d8d5de1814b48bc72) )
	ROM_LOAD32_BYTE( "mpr-12426.94",  0x000001, 0x20000, CRC(28b60dc0) SHA1(ad69d449434853445a076319a55a29014217a100) )
	ROM_LOAD32_BYTE( "mpr-12427.98",  0x000002, 0x20000, CRC(0a367928) SHA1(bcb558b7c23906397e66a7f046b09eb5036c0888) )
	ROM_LOAD32_BYTE( "mpr-12428.102", 0x000003, 0x20000, CRC(efa80ad5) SHA1(9bc7c3fb60cc076f29a0af487d58e5b48f1c4b06) )
	ROM_LOAD32_BYTE( "mpr-12421.91",  0x080000, 0x20000, CRC(25f46140) SHA1(ea75e364cf52636d100158f79be627e36da8c327) )
	ROM_LOAD32_BYTE( "mpr-12422.95",  0x080001, 0x20000, CRC(cb51c8f6) SHA1(5af56ae1916c3212b8d5b9e4bccbbe1916694f89) )
	ROM_LOAD32_BYTE( "mpr-12423.99",  0x080002, 0x20000, CRC(0be9818e) SHA1(637a8201416e73d53f7e2502ea0a5277e43c167d) )
	ROM_LOAD32_BYTE( "mpr-12424.103", 0x080003, 0x20000, CRC(0ce00dfc) SHA1(3b1990977ec7ad4c3bea66527707cff2cd8d5a98) )
	ROM_LOAD32_BYTE( "mpr-12417.92",  0x100000, 0x20000, CRC(a806eabf) SHA1(1a61a2135d92b42ee131fd3240bc8a17a96696ab) )
	ROM_LOAD32_BYTE( "mpr-12418.96",  0x100001, 0x20000, CRC(ed1a0f2b) SHA1(1aa87292ca0465fa129d6be81d95dbb77332ecab) )
	ROM_LOAD32_BYTE( "mpr-12419.100", 0x100002, 0x20000, CRC(ce4568cb) SHA1(1ed66e74ce94d41593b498827d9cc243f775d4ba) )
	ROM_LOAD32_BYTE( "mpr-12420.104", 0x100003, 0x20000, CRC(679442eb) SHA1(f88ef0219497f955d8db6783f3636dad52928f46) )
	ROM_LOAD32_BYTE( "epr-12609.93",  0x180000, 0x20000, CRC(a867812f) SHA1(f8950bf794b6c2ec767ffff837d28917b636dbe7) ) //
	ROM_LOAD32_BYTE( "epr-12610.97",  0x180001, 0x20000, CRC(53b99417) SHA1(ab72d35c88695c777d24c5557e5d3ea2d446e51b) ) //
	ROM_LOAD32_BYTE( "epr-12611.101", 0x180002, 0x20000, CRC(bd5c6ab0) SHA1(7632dc4daa8eabe74769369856a8ba451e5bd420) ) // these differ from japan set
	ROM_LOAD32_BYTE( "epr-12612.105", 0x180003, 0x20000, CRC(ac86e890) SHA1(7720c1c8df6de5de50254e97772c15161b796031) ) //

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none??

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-12436.17",    0x00000, 0x10000, CRC(16ec5f0a) SHA1(307b7388b5c36fd4bc2a61f7941db44858e03c5c) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "mpr-12437.11",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) )
	ROM_LOAD( "mpr-12438.12",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) )
	ROM_LOAD( "mpr-12439.13",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) ) // not used in deluxe

	ROM_REGION( 0x10000, "mainpcb:soundcpu2", 0 ) // z80 on extra sound board
	ROM_LOAD( "epr-12535.8",     0x00000, 0x10000, CRC(80453597) SHA1(d3fee7bb4a8964f5cf1cdae80fc3dde06c947839) ) // taken from deluxe cabinet dump

	ROM_REGION( 0x80000, "mainpcb:pcm2", ROMREGION_ERASEFF ) // Sega PCM sound data on extra sound board (same as on main board..)
	ROM_LOAD( "mpr-12437.20",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) ) // taken from deluxe cabinet dump
	ROM_LOAD( "mpr-12438.21",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) ) // "
	ROM_LOAD( "mpr-12439.22",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) ) // "

	ROM_REGION( 0x10000, "mainpcb:commcpu", 0 ) // z80 on network board
	ROM_LOAD( "epr-12587.14",    0x00000, 0x08000, CRC(2afe648b) SHA1(b5bf86f3acbcc23c136185110acecf2c971294fa) )

	ROM_REGION( 0x10000, "mainpcb:motorcpu", 0 ) // z80 on air board
	ROM_LOAD( "epr-12505.8",     0x00000, 0x08000, CRC(5020788a) SHA1(ed6d1dfb8b6a62d17469e3d09a5b5b864c6b486c) ) // taken from deluxe cabinet dump
ROM_END

//*************************************************************************************************************************
//  Super Monaco GP, Sega X-board
//  CPU: FD1094 (317-0126a)
//
// this set contained only prg roms
ROM_START( smgp6 )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-12563a.58", 0x00000, 0x20000, CRC(2e64b10e) SHA1(2be1ffb3120e4af6a61880e2a2602db07a73f373) )
	ROM_LOAD16_BYTE( "epr-12564a.63", 0x00001, 0x20000, CRC(5baba3e7) SHA1(37194d5a4d3ee48a276f6aeb35b2f20a7661caa2) )

	ROM_REGION( 0x2000, "mainpcb:maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0126a.key", 0x0000, 0x2000, CRC(2abc1982) SHA1(cc4c36e6ba52431df17c6e36ba08d3a89be7b7e7) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-12576a.20", 0x00000, 0x20000, CRC(2c9599c1) SHA1(79206f38c2976bd9299ed37bf62ac26dd3fba801) )
	ROM_LOAD16_BYTE( "epr-12577a.29", 0x00001, 0x20000, CRC(abf9a50b) SHA1(e367b305cd45900aae4849af4904543f05456dc6) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-12429.154", 0x00000, 0x10000, CRC(5851e614) SHA1(3dc97237ede2c6125e92ea6efc68a748d0ec69be) )
	ROM_LOAD( "epr-12430.153", 0x10000, 0x10000, CRC(05e00134) SHA1(8baaa80815d5dabd38dc8600e357975b96d23b95) )
	ROM_LOAD( "epr-12431.152", 0x20000, 0x10000, CRC(35572f4a) SHA1(d66456ecf7b59f81736fb873c553926b56bb3977))

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "mpr-12425.90",  0x000000, 0x20000, CRC(14bf2a15) SHA1(84db3ac09e4a8fe470ac051d8d5de1814b48bc72) )
	ROM_LOAD32_BYTE( "mpr-12426.94",  0x000001, 0x20000, CRC(28b60dc0) SHA1(ad69d449434853445a076319a55a29014217a100) )
	ROM_LOAD32_BYTE( "mpr-12427.98",  0x000002, 0x20000, CRC(0a367928) SHA1(bcb558b7c23906397e66a7f046b09eb5036c0888) )
	ROM_LOAD32_BYTE( "mpr-12428.102", 0x000003, 0x20000, CRC(efa80ad5) SHA1(9bc7c3fb60cc076f29a0af487d58e5b48f1c4b06) )
	ROM_LOAD32_BYTE( "mpr-12421.91",  0x080000, 0x20000, CRC(25f46140) SHA1(ea75e364cf52636d100158f79be627e36da8c327) )
	ROM_LOAD32_BYTE( "mpr-12422.95",  0x080001, 0x20000, CRC(cb51c8f6) SHA1(5af56ae1916c3212b8d5b9e4bccbbe1916694f89) )
	ROM_LOAD32_BYTE( "mpr-12423.99",  0x080002, 0x20000, CRC(0be9818e) SHA1(637a8201416e73d53f7e2502ea0a5277e43c167d) )
	ROM_LOAD32_BYTE( "mpr-12424.103", 0x080003, 0x20000, CRC(0ce00dfc) SHA1(3b1990977ec7ad4c3bea66527707cff2cd8d5a98) )
	ROM_LOAD32_BYTE( "mpr-12417.92",  0x100000, 0x20000, CRC(a806eabf) SHA1(1a61a2135d92b42ee131fd3240bc8a17a96696ab) )
	ROM_LOAD32_BYTE( "mpr-12418.96",  0x100001, 0x20000, CRC(ed1a0f2b) SHA1(1aa87292ca0465fa129d6be81d95dbb77332ecab) )
	ROM_LOAD32_BYTE( "mpr-12419.100", 0x100002, 0x20000, CRC(ce4568cb) SHA1(1ed66e74ce94d41593b498827d9cc243f775d4ba) )
	ROM_LOAD32_BYTE( "mpr-12420.104", 0x100003, 0x20000, CRC(679442eb) SHA1(f88ef0219497f955d8db6783f3636dad52928f46) )
	ROM_LOAD32_BYTE( "epr-12609.93",  0x180000, 0x20000, CRC(a867812f) SHA1(f8950bf794b6c2ec767ffff837d28917b636dbe7) ) //
	ROM_LOAD32_BYTE( "epr-12610.97",  0x180001, 0x20000, CRC(53b99417) SHA1(ab72d35c88695c777d24c5557e5d3ea2d446e51b) ) //
	ROM_LOAD32_BYTE( "epr-12611.101", 0x180002, 0x20000, CRC(bd5c6ab0) SHA1(7632dc4daa8eabe74769369856a8ba451e5bd420) ) // these differ from japan set
	ROM_LOAD32_BYTE( "epr-12612.105", 0x180003, 0x20000, CRC(ac86e890) SHA1(7720c1c8df6de5de50254e97772c15161b796031) ) //

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none??

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-12436.17",    0x00000, 0x10000, CRC(16ec5f0a) SHA1(307b7388b5c36fd4bc2a61f7941db44858e03c5c) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "mpr-12437.11",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) )
	ROM_LOAD( "mpr-12438.12",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) )
	ROM_LOAD( "mpr-12439.13",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) ) // not used in deluxe

	ROM_REGION( 0x10000, "mainpcb:soundcpu2", 0 ) // z80 on extra sound board
	ROM_LOAD( "epr-12535.8",     0x00000, 0x10000, CRC(80453597) SHA1(d3fee7bb4a8964f5cf1cdae80fc3dde06c947839) ) // taken from deluxe cabinet dump

	ROM_REGION( 0x80000, "mainpcb:pcm2", ROMREGION_ERASEFF ) // Sega PCM sound data on extra sound board (same as on main board..)
	ROM_LOAD( "mpr-12437.20",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) ) // taken from deluxe cabinet dump
	ROM_LOAD( "mpr-12438.21",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) ) // "
	ROM_LOAD( "mpr-12439.22",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) ) // "

	ROM_REGION( 0x10000, "mainpcb:commcpu", 0 ) // z80 on network board
	ROM_LOAD( "epr-12587.14",    0x00000, 0x08000, CRC(2afe648b) SHA1(b5bf86f3acbcc23c136185110acecf2c971294fa) ) // taken from twin cabinet dump

	ROM_REGION( 0x10000, "mainpcb:motorcpu", 0 ) // z80 on air board
	ROM_LOAD( "epr-12505.8",     0x00000, 0x08000, CRC(5020788a) SHA1(ed6d1dfb8b6a62d17469e3d09a5b5b864c6b486c) ) // taken from deluxe cabinet dump
ROM_END

ROM_START( smgp6d )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bootleg_epr-12563a.58", 0x00000, 0x20000, CRC(3ba5a1f0) SHA1(52ac3568f35a68afb458fe8d1f4c20029052100f) )
	ROM_LOAD16_BYTE( "bootleg_epr-12564a.63", 0x00001, 0x20000, CRC(05ce14e9) SHA1(abc65f85b9d8710ef88c336df6584c194364dce5) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-12576a.20", 0x00000, 0x20000, CRC(2c9599c1) SHA1(79206f38c2976bd9299ed37bf62ac26dd3fba801) )
	ROM_LOAD16_BYTE( "epr-12577a.29", 0x00001, 0x20000, CRC(abf9a50b) SHA1(e367b305cd45900aae4849af4904543f05456dc6) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-12429.154", 0x00000, 0x10000, CRC(5851e614) SHA1(3dc97237ede2c6125e92ea6efc68a748d0ec69be) )
	ROM_LOAD( "epr-12430.153", 0x10000, 0x10000, CRC(05e00134) SHA1(8baaa80815d5dabd38dc8600e357975b96d23b95) )
	ROM_LOAD( "epr-12431.152", 0x20000, 0x10000, CRC(35572f4a) SHA1(d66456ecf7b59f81736fb873c553926b56bb3977))

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "mpr-12425.90",  0x000000, 0x20000, CRC(14bf2a15) SHA1(84db3ac09e4a8fe470ac051d8d5de1814b48bc72) )
	ROM_LOAD32_BYTE( "mpr-12426.94",  0x000001, 0x20000, CRC(28b60dc0) SHA1(ad69d449434853445a076319a55a29014217a100) )
	ROM_LOAD32_BYTE( "mpr-12427.98",  0x000002, 0x20000, CRC(0a367928) SHA1(bcb558b7c23906397e66a7f046b09eb5036c0888) )
	ROM_LOAD32_BYTE( "mpr-12428.102", 0x000003, 0x20000, CRC(efa80ad5) SHA1(9bc7c3fb60cc076f29a0af487d58e5b48f1c4b06) )
	ROM_LOAD32_BYTE( "mpr-12421.91",  0x080000, 0x20000, CRC(25f46140) SHA1(ea75e364cf52636d100158f79be627e36da8c327) )
	ROM_LOAD32_BYTE( "mpr-12422.95",  0x080001, 0x20000, CRC(cb51c8f6) SHA1(5af56ae1916c3212b8d5b9e4bccbbe1916694f89) )
	ROM_LOAD32_BYTE( "mpr-12423.99",  0x080002, 0x20000, CRC(0be9818e) SHA1(637a8201416e73d53f7e2502ea0a5277e43c167d) )
	ROM_LOAD32_BYTE( "mpr-12424.103", 0x080003, 0x20000, CRC(0ce00dfc) SHA1(3b1990977ec7ad4c3bea66527707cff2cd8d5a98) )
	ROM_LOAD32_BYTE( "mpr-12417.92",  0x100000, 0x20000, CRC(a806eabf) SHA1(1a61a2135d92b42ee131fd3240bc8a17a96696ab) )
	ROM_LOAD32_BYTE( "mpr-12418.96",  0x100001, 0x20000, CRC(ed1a0f2b) SHA1(1aa87292ca0465fa129d6be81d95dbb77332ecab) )
	ROM_LOAD32_BYTE( "mpr-12419.100", 0x100002, 0x20000, CRC(ce4568cb) SHA1(1ed66e74ce94d41593b498827d9cc243f775d4ba) )
	ROM_LOAD32_BYTE( "mpr-12420.104", 0x100003, 0x20000, CRC(679442eb) SHA1(f88ef0219497f955d8db6783f3636dad52928f46) )
	ROM_LOAD32_BYTE( "epr-12609.93",  0x180000, 0x20000, CRC(a867812f) SHA1(f8950bf794b6c2ec767ffff837d28917b636dbe7) ) //
	ROM_LOAD32_BYTE( "epr-12610.97",  0x180001, 0x20000, CRC(53b99417) SHA1(ab72d35c88695c777d24c5557e5d3ea2d446e51b) ) //
	ROM_LOAD32_BYTE( "epr-12611.101", 0x180002, 0x20000, CRC(bd5c6ab0) SHA1(7632dc4daa8eabe74769369856a8ba451e5bd420) ) // these differ from japan set
	ROM_LOAD32_BYTE( "epr-12612.105", 0x180003, 0x20000, CRC(ac86e890) SHA1(7720c1c8df6de5de50254e97772c15161b796031) ) //

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none??

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-12436.17",    0x00000, 0x10000, CRC(16ec5f0a) SHA1(307b7388b5c36fd4bc2a61f7941db44858e03c5c) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "mpr-12437.11",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) )
	ROM_LOAD( "mpr-12438.12",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) )
	ROM_LOAD( "mpr-12439.13",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) ) // not used in deluxe

	ROM_REGION( 0x10000, "mainpcb:soundcpu2", 0 ) // z80 on extra sound board
	ROM_LOAD( "epr-12535.8",     0x00000, 0x10000, CRC(80453597) SHA1(d3fee7bb4a8964f5cf1cdae80fc3dde06c947839) ) // taken from deluxe cabinet dump

	ROM_REGION( 0x80000, "mainpcb:pcm2", ROMREGION_ERASEFF ) // Sega PCM sound data on extra sound board (same as on main board..)
	ROM_LOAD( "mpr-12437.20",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) ) // taken from deluxe cabinet dump
	ROM_LOAD( "mpr-12438.21",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) ) // "
	ROM_LOAD( "mpr-12439.22",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) ) // "

	ROM_REGION( 0x10000, "mainpcb:commcpu", 0 ) // z80 on network board
	ROM_LOAD( "epr-12587.14",    0x00000, 0x08000, CRC(2afe648b) SHA1(b5bf86f3acbcc23c136185110acecf2c971294fa) ) // taken from twin cabinet dump

	ROM_REGION( 0x10000, "mainpcb:motorcpu", 0 ) // z80 on air board
	ROM_LOAD( "epr-12505.8",     0x00000, 0x08000, CRC(5020788a) SHA1(ed6d1dfb8b6a62d17469e3d09a5b5b864c6b486c) ) // taken from deluxe cabinet dump
ROM_END

//*************************************************************************************************************************
//  Super Monaco GP, Sega X-board
//  CPU: FD1094 (317-0126)
//  This set is coming from a deluxe.
//
//  SEGA Monaco G.P. by SEGA 1989
//
//  This set is coming from a sitdown "air drive" version.
//
//  This set has an extra sound board (837-7000) under the main board with a Z80
//  and a few eproms, some of those eproms are already on the main board !
//
//  It has also an "air drive" board with a Z80 and one eprom.
//
//  Main Board : (834-7016-05)
//
//  epr-12576.20 (68000)
//  epr-12577.29 (68000)
//  epr-12563.58 FD1094 317-0126
//  epr-12564.63 FD1094 317-0126
//  epr-12413.93
//  epr-12414.97
//  epr-12415.101
//  epr-12416.105
//  mpr-12417.92
//  mpr-12418.96
//  mpr-12419.100
//  mpr-12420.104
//  mpr-12421.91
//  mpr-12422.95
//  mpr-12423.99
//  mpr-12424.103
//  mpr-12425.90
//  mpr-12426.94
//  mpr-12427.98
//  mpr-12428.102
//  epr-12429.154
//  epr-12430.153
//  epr-12431.152
//  epr-12436.17
//  mpr-12437.11
//  mpr-12438.12
//  IC 13 is not used !
//
//  Sound Board :
//
//  epr-12535.8
//  mpr-12437.20
//  mpr-12438.21
//  mpr-12439.22
//
//  Air Drive Board :
//
//  Ep12505.8
//
ROM_START( smgp5 )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-12563.58", 0x00000, 0x20000, CRC(6d7325ae) SHA1(bf88ceddc49dab5b439080d5bf0e7e084a79546c) )
	ROM_LOAD16_BYTE( "epr-12564.63", 0x00001, 0x20000, CRC(adfbf921) SHA1(f3321e03dc37b14db065b85d63e321810e4ea797) )

	ROM_REGION( 0x2000, "mainpcb:maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0126.key", 0x0000, 0x2000, CRC(4d917996) SHA1(17232c0e35d439a12db3d966064cf00104088903) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-12576.20", 0x00000, 0x20000, CRC(23266b26) SHA1(240b9bf198fd2975851e769766566ec4e8379f87) )
	ROM_LOAD16_BYTE( "epr-12577.29", 0x00001, 0x20000, CRC(d5b53211) SHA1(b11f5c5094eb7ea9578f15489b00d8bbac1edee6) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-12429.154", 0x00000, 0x10000, CRC(5851e614) SHA1(3dc97237ede2c6125e92ea6efc68a748d0ec69be) )
	ROM_LOAD( "epr-12430.153", 0x10000, 0x10000, CRC(05e00134) SHA1(8baaa80815d5dabd38dc8600e357975b96d23b95) )
	ROM_LOAD( "epr-12431.152", 0x20000, 0x10000, CRC(35572f4a) SHA1(d66456ecf7b59f81736fb873c553926b56bb3977))

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "mpr-12425.90",  0x000000, 0x20000, CRC(14bf2a15) SHA1(84db3ac09e4a8fe470ac051d8d5de1814b48bc72) )
	ROM_LOAD32_BYTE( "mpr-12426.94",  0x000001, 0x20000, CRC(28b60dc0) SHA1(ad69d449434853445a076319a55a29014217a100) )
	ROM_LOAD32_BYTE( "mpr-12427.98",  0x000002, 0x20000, CRC(0a367928) SHA1(bcb558b7c23906397e66a7f046b09eb5036c0888) )
	ROM_LOAD32_BYTE( "mpr-12428.102", 0x000003, 0x20000, CRC(efa80ad5) SHA1(9bc7c3fb60cc076f29a0af487d58e5b48f1c4b06) )
	ROM_LOAD32_BYTE( "mpr-12421.91",  0x080000, 0x20000, CRC(25f46140) SHA1(ea75e364cf52636d100158f79be627e36da8c327) )
	ROM_LOAD32_BYTE( "mpr-12422.95",  0x080001, 0x20000, CRC(cb51c8f6) SHA1(5af56ae1916c3212b8d5b9e4bccbbe1916694f89) )
	ROM_LOAD32_BYTE( "mpr-12423.99",  0x080002, 0x20000, CRC(0be9818e) SHA1(637a8201416e73d53f7e2502ea0a5277e43c167d) )
	ROM_LOAD32_BYTE( "mpr-12424.103", 0x080003, 0x20000, CRC(0ce00dfc) SHA1(3b1990977ec7ad4c3bea66527707cff2cd8d5a98) )
	ROM_LOAD32_BYTE( "mpr-12417.92",  0x100000, 0x20000, CRC(a806eabf) SHA1(1a61a2135d92b42ee131fd3240bc8a17a96696ab) )
	ROM_LOAD32_BYTE( "mpr-12418.96",  0x100001, 0x20000, CRC(ed1a0f2b) SHA1(1aa87292ca0465fa129d6be81d95dbb77332ecab) )
	ROM_LOAD32_BYTE( "mpr-12419.100", 0x100002, 0x20000, CRC(ce4568cb) SHA1(1ed66e74ce94d41593b498827d9cc243f775d4ba) )
	ROM_LOAD32_BYTE( "mpr-12420.104", 0x100003, 0x20000, CRC(679442eb) SHA1(f88ef0219497f955d8db6783f3636dad52928f46) )
	ROM_LOAD32_BYTE( "epr-12413.93",  0x180000, 0x20000, CRC(2f1693df) SHA1(ba1e654a1b5fae661b0dae4a8ed04ff50fb546a2) )
	ROM_LOAD32_BYTE( "epr-12414.97",  0x180001, 0x20000, CRC(c78f3d45) SHA1(665750907ed11c89c2ea5c410eac2808445131ae) )
	ROM_LOAD32_BYTE( "epr-12415.101", 0x180002, 0x20000, CRC(6080e9ed) SHA1(eb1b871453f76e6a65d20fa9d4bddc1c9f940b4d) )
	ROM_LOAD32_BYTE( "epr-12416.105", 0x180003, 0x20000, CRC(6f1f2769) SHA1(d00d26cd1052d4b46c432b6b69cb2d83179d52a6) )

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none??

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-12436.17",    0x00000, 0x10000, CRC(16ec5f0a) SHA1(307b7388b5c36fd4bc2a61f7941db44858e03c5c) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "mpr-12437.11",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) )
	ROM_LOAD( "mpr-12438.12",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) )
	ROM_LOAD( "mpr-12439.13",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) ) // not used in deluxe

	ROM_REGION( 0x10000, "mainpcb:soundcpu2", 0 ) // z80 on extra sound board
	ROM_LOAD( "epr-12535.8",     0x00000, 0x10000, CRC(80453597) SHA1(d3fee7bb4a8964f5cf1cdae80fc3dde06c947839) )

	ROM_REGION( 0x80000, "mainpcb:pcm2", ROMREGION_ERASEFF ) // Sega PCM sound data on extra sound board (same as on main board..)
	ROM_LOAD( "mpr-12437.20",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) )
	ROM_LOAD( "mpr-12438.21",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) )
	ROM_LOAD( "mpr-12439.22",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) )

	ROM_REGION( 0x10000, "mainpcb:commcpu", 0 ) // z80 on network board
	ROM_LOAD( "epr-12587.14",    0x00000, 0x08000, CRC(2afe648b) SHA1(b5bf86f3acbcc23c136185110acecf2c971294fa) ) // taken from twin cabinet dump

	ROM_REGION( 0x10000, "mainpcb:motorcpu", 0 ) // z80 on air board
	ROM_LOAD( "epr-12505.8",     0x00000, 0x08000, CRC(5020788a) SHA1(ed6d1dfb8b6a62d17469e3d09a5b5b864c6b486c) )
ROM_END

ROM_START( smgp5d )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bootleg_epr-12563.58", 0x00000, 0x20000, CRC(6c7f0549) SHA1(a9beed12e204acc5bf45dded9b3d4d1643b83a94) )
	ROM_LOAD16_BYTE( "bootleg_epr-12564.63", 0x00001, 0x20000, CRC(c2b3a219) SHA1(d9299d2a0a93404f18148e9d2bd2d57cd043b67b) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-12576.20", 0x00000, 0x20000, CRC(23266b26) SHA1(240b9bf198fd2975851e769766566ec4e8379f87) )
	ROM_LOAD16_BYTE( "epr-12577.29", 0x00001, 0x20000, CRC(d5b53211) SHA1(b11f5c5094eb7ea9578f15489b00d8bbac1edee6) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-12429.154", 0x00000, 0x10000, CRC(5851e614) SHA1(3dc97237ede2c6125e92ea6efc68a748d0ec69be) )
	ROM_LOAD( "epr-12430.153", 0x10000, 0x10000, CRC(05e00134) SHA1(8baaa80815d5dabd38dc8600e357975b96d23b95) )
	ROM_LOAD( "epr-12431.152", 0x20000, 0x10000, CRC(35572f4a) SHA1(d66456ecf7b59f81736fb873c553926b56bb3977))

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "mpr-12425.90",  0x000000, 0x20000, CRC(14bf2a15) SHA1(84db3ac09e4a8fe470ac051d8d5de1814b48bc72) )
	ROM_LOAD32_BYTE( "mpr-12426.94",  0x000001, 0x20000, CRC(28b60dc0) SHA1(ad69d449434853445a076319a55a29014217a100) )
	ROM_LOAD32_BYTE( "mpr-12427.98",  0x000002, 0x20000, CRC(0a367928) SHA1(bcb558b7c23906397e66a7f046b09eb5036c0888) )
	ROM_LOAD32_BYTE( "mpr-12428.102", 0x000003, 0x20000, CRC(efa80ad5) SHA1(9bc7c3fb60cc076f29a0af487d58e5b48f1c4b06) )
	ROM_LOAD32_BYTE( "mpr-12421.91",  0x080000, 0x20000, CRC(25f46140) SHA1(ea75e364cf52636d100158f79be627e36da8c327) )
	ROM_LOAD32_BYTE( "mpr-12422.95",  0x080001, 0x20000, CRC(cb51c8f6) SHA1(5af56ae1916c3212b8d5b9e4bccbbe1916694f89) )
	ROM_LOAD32_BYTE( "mpr-12423.99",  0x080002, 0x20000, CRC(0be9818e) SHA1(637a8201416e73d53f7e2502ea0a5277e43c167d) )
	ROM_LOAD32_BYTE( "mpr-12424.103", 0x080003, 0x20000, CRC(0ce00dfc) SHA1(3b1990977ec7ad4c3bea66527707cff2cd8d5a98) )
	ROM_LOAD32_BYTE( "mpr-12417.92",  0x100000, 0x20000, CRC(a806eabf) SHA1(1a61a2135d92b42ee131fd3240bc8a17a96696ab) )
	ROM_LOAD32_BYTE( "mpr-12418.96",  0x100001, 0x20000, CRC(ed1a0f2b) SHA1(1aa87292ca0465fa129d6be81d95dbb77332ecab) )
	ROM_LOAD32_BYTE( "mpr-12419.100", 0x100002, 0x20000, CRC(ce4568cb) SHA1(1ed66e74ce94d41593b498827d9cc243f775d4ba) )
	ROM_LOAD32_BYTE( "mpr-12420.104", 0x100003, 0x20000, CRC(679442eb) SHA1(f88ef0219497f955d8db6783f3636dad52928f46) )
	ROM_LOAD32_BYTE( "epr-12413.93",  0x180000, 0x20000, CRC(2f1693df) SHA1(ba1e654a1b5fae661b0dae4a8ed04ff50fb546a2) )
	ROM_LOAD32_BYTE( "epr-12414.97",  0x180001, 0x20000, CRC(c78f3d45) SHA1(665750907ed11c89c2ea5c410eac2808445131ae) )
	ROM_LOAD32_BYTE( "epr-12415.101", 0x180002, 0x20000, CRC(6080e9ed) SHA1(eb1b871453f76e6a65d20fa9d4bddc1c9f940b4d) )
	ROM_LOAD32_BYTE( "epr-12416.105", 0x180003, 0x20000, CRC(6f1f2769) SHA1(d00d26cd1052d4b46c432b6b69cb2d83179d52a6) )

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none??

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-12436.17",    0x00000, 0x10000, CRC(16ec5f0a) SHA1(307b7388b5c36fd4bc2a61f7941db44858e03c5c) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "mpr-12437.11",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) )
	ROM_LOAD( "mpr-12438.12",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) )
	ROM_LOAD( "mpr-12439.13",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) ) // not used in deluxe

	ROM_REGION( 0x10000, "mainpcb:soundcpu2", 0 ) // z80 on extra sound board
	ROM_LOAD( "epr-12535.8",     0x00000, 0x10000, CRC(80453597) SHA1(d3fee7bb4a8964f5cf1cdae80fc3dde06c947839) )

	ROM_REGION( 0x80000, "mainpcb:pcm2", ROMREGION_ERASEFF ) // Sega PCM sound data on extra sound board (same as on main board..)
	ROM_LOAD( "mpr-12437.20",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) )
	ROM_LOAD( "mpr-12438.21",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) )
	ROM_LOAD( "mpr-12439.22",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) )

	ROM_REGION( 0x10000, "mainpcb:commcpu", 0 ) // z80 on network board
	ROM_LOAD( "epr-12587.14",    0x00000, 0x08000, CRC(2afe648b) SHA1(b5bf86f3acbcc23c136185110acecf2c971294fa) ) // taken from twin cabinet dump

	ROM_REGION( 0x10000, "mainpcb:motorcpu", 0 ) // z80 on air board
	ROM_LOAD( "epr-12505.8",     0x00000, 0x08000, CRC(5020788a) SHA1(ed6d1dfb8b6a62d17469e3d09a5b5b864c6b486c) )
ROM_END

//*************************************************************************************************************************
//  Super Monaco GP, Sega X-board
//  CPU: FD1094 (317-0125a)
//
ROM_START( smgpu )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-12561c.58", 0x00000, 0x20000, CRC(a5b0f3fe) SHA1(17103e56f822fdb52e72f597c01415ed375aa102) )
	ROM_LOAD16_BYTE( "epr-12562c.63", 0x00001, 0x20000, CRC(799e55f4) SHA1(2e02cdc63bda47b087c81021018287cfa961c083) )

	ROM_REGION( 0x2000, "mainpcb:maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0125a.key", 0x0000, 0x2000, CRC(3ecdb120) SHA1(c484198e4509d79214e78d4a47e9a7e339f7a2ed) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-12574a.20", 0x00000, 0x20000, CRC(f8b5c38b) SHA1(0184d5a1b71fb42d33dbaaad99d2c0fbc5750e7e) )
	ROM_LOAD16_BYTE( "epr-12575a.29", 0x00001, 0x20000, CRC(248b1d17) SHA1(22f1e0d0d698abdf0cb1954f1f6382432a12c186) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-12429.154", 0x00000, 0x10000, CRC(5851e614) SHA1(3dc97237ede2c6125e92ea6efc68a748d0ec69be) )
	ROM_LOAD( "epr-12430.153", 0x10000, 0x10000, CRC(05e00134) SHA1(8baaa80815d5dabd38dc8600e357975b96d23b95) )
	ROM_LOAD( "epr-12431.152", 0x20000, 0x10000, CRC(35572f4a) SHA1(d66456ecf7b59f81736fb873c553926b56bb3977))

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "mpr-12425.90",  0x000000, 0x20000, CRC(14bf2a15) SHA1(84db3ac09e4a8fe470ac051d8d5de1814b48bc72) )
	ROM_LOAD32_BYTE( "mpr-12426.94",  0x000001, 0x20000, CRC(28b60dc0) SHA1(ad69d449434853445a076319a55a29014217a100) )
	ROM_LOAD32_BYTE( "mpr-12427.98",  0x000002, 0x20000, CRC(0a367928) SHA1(bcb558b7c23906397e66a7f046b09eb5036c0888) )
	ROM_LOAD32_BYTE( "mpr-12428.102", 0x000003, 0x20000, CRC(efa80ad5) SHA1(9bc7c3fb60cc076f29a0af487d58e5b48f1c4b06) )
	ROM_LOAD32_BYTE( "mpr-12421.91",  0x080000, 0x20000, CRC(25f46140) SHA1(ea75e364cf52636d100158f79be627e36da8c327) )
	ROM_LOAD32_BYTE( "mpr-12422.95",  0x080001, 0x20000, CRC(cb51c8f6) SHA1(5af56ae1916c3212b8d5b9e4bccbbe1916694f89) )
	ROM_LOAD32_BYTE( "mpr-12423.99",  0x080002, 0x20000, CRC(0be9818e) SHA1(637a8201416e73d53f7e2502ea0a5277e43c167d) )
	ROM_LOAD32_BYTE( "mpr-12424.103", 0x080003, 0x20000, CRC(0ce00dfc) SHA1(3b1990977ec7ad4c3bea66527707cff2cd8d5a98) )
	ROM_LOAD32_BYTE( "mpr-12417.92",  0x100000, 0x20000, CRC(a806eabf) SHA1(1a61a2135d92b42ee131fd3240bc8a17a96696ab) )
	ROM_LOAD32_BYTE( "mpr-12418.96",  0x100001, 0x20000, CRC(ed1a0f2b) SHA1(1aa87292ca0465fa129d6be81d95dbb77332ecab) )
	ROM_LOAD32_BYTE( "mpr-12419.100", 0x100002, 0x20000, CRC(ce4568cb) SHA1(1ed66e74ce94d41593b498827d9cc243f775d4ba) )
	ROM_LOAD32_BYTE( "mpr-12420.104", 0x100003, 0x20000, CRC(679442eb) SHA1(f88ef0219497f955d8db6783f3636dad52928f46) )
	ROM_LOAD32_BYTE( "epr-12609.93",  0x180000, 0x20000, CRC(a867812f) SHA1(f8950bf794b6c2ec767ffff837d28917b636dbe7) ) //
	ROM_LOAD32_BYTE( "epr-12610.97",  0x180001, 0x20000, CRC(53b99417) SHA1(ab72d35c88695c777d24c5557e5d3ea2d446e51b) ) //
	ROM_LOAD32_BYTE( "epr-12611.101", 0x180002, 0x20000, CRC(bd5c6ab0) SHA1(7632dc4daa8eabe74769369856a8ba451e5bd420) ) // these differ from japan set
	ROM_LOAD32_BYTE( "epr-12612.105", 0x180003, 0x20000, CRC(ac86e890) SHA1(7720c1c8df6de5de50254e97772c15161b796031) ) //

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none??

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-12436.17",    0x00000, 0x10000, CRC(16ec5f0a) SHA1(307b7388b5c36fd4bc2a61f7941db44858e03c5c) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "mpr-12437.11",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) )
	ROM_LOAD( "mpr-12438.12",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) )
	ROM_LOAD( "mpr-12439.13",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) ) // not used in deluxe

	ROM_REGION( 0x10000, "mainpcb:soundcpu2", 0 ) // z80 on extra sound board
	ROM_LOAD( "epr-12535.8",     0x00000, 0x10000, CRC(80453597) SHA1(d3fee7bb4a8964f5cf1cdae80fc3dde06c947839) ) // taken from deluxe cabinet dump

	ROM_REGION( 0x80000, "mainpcb:pcm2", ROMREGION_ERASEFF ) // Sega PCM sound data on extra sound board (same as on main board..)
	ROM_LOAD( "mpr-12437.20",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) ) // taken from deluxe cabinet dump
	ROM_LOAD( "mpr-12438.21",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) ) // "
	ROM_LOAD( "mpr-12439.22",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) ) // "

	ROM_REGION( 0x10000, "mainpcb:commcpu", 0 ) // z80 on network board
	ROM_LOAD( "epr-12587.14",    0x00000, 0x08000, CRC(2afe648b) SHA1(b5bf86f3acbcc23c136185110acecf2c971294fa) ) // taken from twin cabinet dump

	ROM_REGION( 0x10000, "mainpcb:motorcpu", 0 ) // z80 on air board
	ROM_LOAD( "epr-12505.8",     0x00000, 0x08000, CRC(5020788a) SHA1(ed6d1dfb8b6a62d17469e3d09a5b5b864c6b486c) ) // taken from deluxe cabinet dump
ROM_END

ROM_START( smgpud )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bootleg_epr-12561c.58", 0x00000, 0x20000, CRC(7053e379) SHA1(742e80e2ec2dedae1d6ba64fc563707790a30606) )
	ROM_LOAD16_BYTE( "bootleg_epr-12562c.63", 0x00001, 0x20000, CRC(db848e75) SHA1(0750c981a70a2cc5dfae6ce27598865847ff3156) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-12574a.20", 0x00000, 0x20000, CRC(f8b5c38b) SHA1(0184d5a1b71fb42d33dbaaad99d2c0fbc5750e7e) )
	ROM_LOAD16_BYTE( "epr-12575a.29", 0x00001, 0x20000, CRC(248b1d17) SHA1(22f1e0d0d698abdf0cb1954f1f6382432a12c186) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-12429.154", 0x00000, 0x10000, CRC(5851e614) SHA1(3dc97237ede2c6125e92ea6efc68a748d0ec69be) )
	ROM_LOAD( "epr-12430.153", 0x10000, 0x10000, CRC(05e00134) SHA1(8baaa80815d5dabd38dc8600e357975b96d23b95) )
	ROM_LOAD( "epr-12431.152", 0x20000, 0x10000, CRC(35572f4a) SHA1(d66456ecf7b59f81736fb873c553926b56bb3977))

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "mpr-12425.90",  0x000000, 0x20000, CRC(14bf2a15) SHA1(84db3ac09e4a8fe470ac051d8d5de1814b48bc72) )
	ROM_LOAD32_BYTE( "mpr-12426.94",  0x000001, 0x20000, CRC(28b60dc0) SHA1(ad69d449434853445a076319a55a29014217a100) )
	ROM_LOAD32_BYTE( "mpr-12427.98",  0x000002, 0x20000, CRC(0a367928) SHA1(bcb558b7c23906397e66a7f046b09eb5036c0888) )
	ROM_LOAD32_BYTE( "mpr-12428.102", 0x000003, 0x20000, CRC(efa80ad5) SHA1(9bc7c3fb60cc076f29a0af487d58e5b48f1c4b06) )
	ROM_LOAD32_BYTE( "mpr-12421.91",  0x080000, 0x20000, CRC(25f46140) SHA1(ea75e364cf52636d100158f79be627e36da8c327) )
	ROM_LOAD32_BYTE( "mpr-12422.95",  0x080001, 0x20000, CRC(cb51c8f6) SHA1(5af56ae1916c3212b8d5b9e4bccbbe1916694f89) )
	ROM_LOAD32_BYTE( "mpr-12423.99",  0x080002, 0x20000, CRC(0be9818e) SHA1(637a8201416e73d53f7e2502ea0a5277e43c167d) )
	ROM_LOAD32_BYTE( "mpr-12424.103", 0x080003, 0x20000, CRC(0ce00dfc) SHA1(3b1990977ec7ad4c3bea66527707cff2cd8d5a98) )
	ROM_LOAD32_BYTE( "mpr-12417.92",  0x100000, 0x20000, CRC(a806eabf) SHA1(1a61a2135d92b42ee131fd3240bc8a17a96696ab) )
	ROM_LOAD32_BYTE( "mpr-12418.96",  0x100001, 0x20000, CRC(ed1a0f2b) SHA1(1aa87292ca0465fa129d6be81d95dbb77332ecab) )
	ROM_LOAD32_BYTE( "mpr-12419.100", 0x100002, 0x20000, CRC(ce4568cb) SHA1(1ed66e74ce94d41593b498827d9cc243f775d4ba) )
	ROM_LOAD32_BYTE( "mpr-12420.104", 0x100003, 0x20000, CRC(679442eb) SHA1(f88ef0219497f955d8db6783f3636dad52928f46) )
	ROM_LOAD32_BYTE( "epr-12609.93",  0x180000, 0x20000, CRC(a867812f) SHA1(f8950bf794b6c2ec767ffff837d28917b636dbe7) ) //
	ROM_LOAD32_BYTE( "epr-12610.97",  0x180001, 0x20000, CRC(53b99417) SHA1(ab72d35c88695c777d24c5557e5d3ea2d446e51b) ) //
	ROM_LOAD32_BYTE( "epr-12611.101", 0x180002, 0x20000, CRC(bd5c6ab0) SHA1(7632dc4daa8eabe74769369856a8ba451e5bd420) ) // these differ from japan set
	ROM_LOAD32_BYTE( "epr-12612.105", 0x180003, 0x20000, CRC(ac86e890) SHA1(7720c1c8df6de5de50254e97772c15161b796031) ) //

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none??

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-12436.17",    0x00000, 0x10000, CRC(16ec5f0a) SHA1(307b7388b5c36fd4bc2a61f7941db44858e03c5c) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "mpr-12437.11",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) )
	ROM_LOAD( "mpr-12438.12",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) )
	ROM_LOAD( "mpr-12439.13",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) ) // not used in deluxe

	ROM_REGION( 0x10000, "mainpcb:soundcpu2", 0 ) // z80 on extra sound board
	ROM_LOAD( "epr-12535.8",     0x00000, 0x10000, CRC(80453597) SHA1(d3fee7bb4a8964f5cf1cdae80fc3dde06c947839) ) // taken from deluxe cabinet dump

	ROM_REGION( 0x80000, "mainpcb:pcm2", ROMREGION_ERASEFF ) // Sega PCM sound data on extra sound board (same as on main board..)
	ROM_LOAD( "mpr-12437.20",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) ) // taken from deluxe cabinet dump
	ROM_LOAD( "mpr-12438.21",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) ) // "
	ROM_LOAD( "mpr-12439.22",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) ) // "

	ROM_REGION( 0x10000, "mainpcb:commcpu", 0 ) // z80 on network board
	ROM_LOAD( "epr-12587.14",    0x00000, 0x08000, CRC(2afe648b) SHA1(b5bf86f3acbcc23c136185110acecf2c971294fa) ) // taken from twin cabinet dump

	ROM_REGION( 0x10000, "mainpcb:motorcpu", 0 ) // z80 on air board
	ROM_LOAD( "epr-12505.8",     0x00000, 0x08000, CRC(5020788a) SHA1(ed6d1dfb8b6a62d17469e3d09a5b5b864c6b486c) ) // taken from deluxe cabinet dump
ROM_END

//*************************************************************************************************************************
//  Super Monaco GP, Sega X-board
//  CPU: FD1094 (317-0125a)
//
// very first US version with demo sound on by default
ROM_START( smgpu1 )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-12561b.58", 0x00000, 0x20000, CRC(80a32655) SHA1(fe1ffa8af9f1ca175ba90b24a0853329b08d19af) )
	ROM_LOAD16_BYTE( "epr-12562b.63", 0x00001, 0x20000, CRC(d525f2a8) SHA1(f3241e11485c7428cd9f081ec6768fda39ae3250) )

	ROM_REGION( 0x2000, "mainpcb:maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0125a.key", 0x0000, 0x2000, CRC(3ecdb120) SHA1(c484198e4509d79214e78d4a47e9a7e339f7a2ed) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-12574a.20", 0x00000, 0x20000, CRC(f8b5c38b) SHA1(0184d5a1b71fb42d33dbaaad99d2c0fbc5750e7e) )
	ROM_LOAD16_BYTE( "epr-12575a.29", 0x00001, 0x20000, CRC(248b1d17) SHA1(22f1e0d0d698abdf0cb1954f1f6382432a12c186) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-12429.154", 0x00000, 0x10000, CRC(5851e614) SHA1(3dc97237ede2c6125e92ea6efc68a748d0ec69be) )
	ROM_LOAD( "epr-12430.153", 0x10000, 0x10000, CRC(05e00134) SHA1(8baaa80815d5dabd38dc8600e357975b96d23b95) )
	ROM_LOAD( "epr-12431.152", 0x20000, 0x10000, CRC(35572f4a) SHA1(d66456ecf7b59f81736fb873c553926b56bb3977))

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "mpr-12425.90",  0x000000, 0x20000, CRC(14bf2a15) SHA1(84db3ac09e4a8fe470ac051d8d5de1814b48bc72) )
	ROM_LOAD32_BYTE( "mpr-12426.94",  0x000001, 0x20000, CRC(28b60dc0) SHA1(ad69d449434853445a076319a55a29014217a100) )
	ROM_LOAD32_BYTE( "mpr-12427.98",  0x000002, 0x20000, CRC(0a367928) SHA1(bcb558b7c23906397e66a7f046b09eb5036c0888) )
	ROM_LOAD32_BYTE( "mpr-12428.102", 0x000003, 0x20000, CRC(efa80ad5) SHA1(9bc7c3fb60cc076f29a0af487d58e5b48f1c4b06) )
	ROM_LOAD32_BYTE( "mpr-12421.91",  0x080000, 0x20000, CRC(25f46140) SHA1(ea75e364cf52636d100158f79be627e36da8c327) )
	ROM_LOAD32_BYTE( "mpr-12422.95",  0x080001, 0x20000, CRC(cb51c8f6) SHA1(5af56ae1916c3212b8d5b9e4bccbbe1916694f89) )
	ROM_LOAD32_BYTE( "mpr-12423.99",  0x080002, 0x20000, CRC(0be9818e) SHA1(637a8201416e73d53f7e2502ea0a5277e43c167d) )
	ROM_LOAD32_BYTE( "mpr-12424.103", 0x080003, 0x20000, CRC(0ce00dfc) SHA1(3b1990977ec7ad4c3bea66527707cff2cd8d5a98) )
	ROM_LOAD32_BYTE( "mpr-12417.92",  0x100000, 0x20000, CRC(a806eabf) SHA1(1a61a2135d92b42ee131fd3240bc8a17a96696ab) )
	ROM_LOAD32_BYTE( "mpr-12418.96",  0x100001, 0x20000, CRC(ed1a0f2b) SHA1(1aa87292ca0465fa129d6be81d95dbb77332ecab) )
	ROM_LOAD32_BYTE( "mpr-12419.100", 0x100002, 0x20000, CRC(ce4568cb) SHA1(1ed66e74ce94d41593b498827d9cc243f775d4ba) )
	ROM_LOAD32_BYTE( "mpr-12420.104", 0x100003, 0x20000, CRC(679442eb) SHA1(f88ef0219497f955d8db6783f3636dad52928f46) )
	ROM_LOAD32_BYTE( "epr-12609.93",  0x180000, 0x20000, CRC(a867812f) SHA1(f8950bf794b6c2ec767ffff837d28917b636dbe7) ) //
	ROM_LOAD32_BYTE( "epr-12610.97",  0x180001, 0x20000, CRC(53b99417) SHA1(ab72d35c88695c777d24c5557e5d3ea2d446e51b) ) //
	ROM_LOAD32_BYTE( "epr-12611.101", 0x180002, 0x20000, CRC(bd5c6ab0) SHA1(7632dc4daa8eabe74769369856a8ba451e5bd420) ) // these differ from japan set
	ROM_LOAD32_BYTE( "epr-12612.105", 0x180003, 0x20000, CRC(ac86e890) SHA1(7720c1c8df6de5de50254e97772c15161b796031) ) //

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none??

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-12436.17",    0x00000, 0x10000, CRC(16ec5f0a) SHA1(307b7388b5c36fd4bc2a61f7941db44858e03c5c) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "mpr-12437.11",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) )
	ROM_LOAD( "mpr-12438.12",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) )
	ROM_LOAD( "mpr-12439.13",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) ) // not used in deluxe

	ROM_REGION( 0x10000, "mainpcb:soundcpu2", 0 ) // z80 on extra sound board
	ROM_LOAD( "epr-12535.8",     0x00000, 0x10000, CRC(80453597) SHA1(d3fee7bb4a8964f5cf1cdae80fc3dde06c947839) ) // taken from deluxe cabinet dump

	ROM_REGION( 0x80000, "mainpcb:pcm2", ROMREGION_ERASEFF ) // Sega PCM sound data on extra sound board (same as on main board..)
	ROM_LOAD( "mpr-12437.20",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) ) // taken from deluxe cabinet dump
	ROM_LOAD( "mpr-12438.21",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) ) // "
	ROM_LOAD( "mpr-12439.22",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) ) // "

	ROM_REGION( 0x10000, "mainpcb:commcpu", 0 ) // z80 on network board
	ROM_LOAD( "epr-12587.14",    0x00000, 0x08000, CRC(2afe648b) SHA1(b5bf86f3acbcc23c136185110acecf2c971294fa) ) // taken from twin cabinet dump

	ROM_REGION( 0x10000, "mainpcb:motorcpu", 0 ) // z80 on air board
	ROM_LOAD( "epr-12505.8",     0x00000, 0x08000, CRC(5020788a) SHA1(ed6d1dfb8b6a62d17469e3d09a5b5b864c6b486c) ) // taken from deluxe cabinet dump
ROM_END

ROM_START( smgpu1d )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bootleg_epr-12561.58", 0x00000, 0x20000, CRC(554036d2) SHA1(7f82c8e96342f5b01209e0fcf56bafbb06d06458) )
	ROM_LOAD16_BYTE( "bootleg_epr-12562.63", 0x00001, 0x20000, CRC(773f2929) SHA1(aff10b08ed4cf1505228d8f2a785e71eeaea4089) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-12574a.20", 0x00000, 0x20000, CRC(f8b5c38b) SHA1(0184d5a1b71fb42d33dbaaad99d2c0fbc5750e7e) )
	ROM_LOAD16_BYTE( "epr-12575a.29", 0x00001, 0x20000, CRC(248b1d17) SHA1(22f1e0d0d698abdf0cb1954f1f6382432a12c186) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-12429.154", 0x00000, 0x10000, CRC(5851e614) SHA1(3dc97237ede2c6125e92ea6efc68a748d0ec69be) )
	ROM_LOAD( "epr-12430.153", 0x10000, 0x10000, CRC(05e00134) SHA1(8baaa80815d5dabd38dc8600e357975b96d23b95) )
	ROM_LOAD( "epr-12431.152", 0x20000, 0x10000, CRC(35572f4a) SHA1(d66456ecf7b59f81736fb873c553926b56bb3977))

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "mpr-12425.90",  0x000000, 0x20000, CRC(14bf2a15) SHA1(84db3ac09e4a8fe470ac051d8d5de1814b48bc72) )
	ROM_LOAD32_BYTE( "mpr-12426.94",  0x000001, 0x20000, CRC(28b60dc0) SHA1(ad69d449434853445a076319a55a29014217a100) )
	ROM_LOAD32_BYTE( "mpr-12427.98",  0x000002, 0x20000, CRC(0a367928) SHA1(bcb558b7c23906397e66a7f046b09eb5036c0888) )
	ROM_LOAD32_BYTE( "mpr-12428.102", 0x000003, 0x20000, CRC(efa80ad5) SHA1(9bc7c3fb60cc076f29a0af487d58e5b48f1c4b06) )
	ROM_LOAD32_BYTE( "mpr-12421.91",  0x080000, 0x20000, CRC(25f46140) SHA1(ea75e364cf52636d100158f79be627e36da8c327) )
	ROM_LOAD32_BYTE( "mpr-12422.95",  0x080001, 0x20000, CRC(cb51c8f6) SHA1(5af56ae1916c3212b8d5b9e4bccbbe1916694f89) )
	ROM_LOAD32_BYTE( "mpr-12423.99",  0x080002, 0x20000, CRC(0be9818e) SHA1(637a8201416e73d53f7e2502ea0a5277e43c167d) )
	ROM_LOAD32_BYTE( "mpr-12424.103", 0x080003, 0x20000, CRC(0ce00dfc) SHA1(3b1990977ec7ad4c3bea66527707cff2cd8d5a98) )
	ROM_LOAD32_BYTE( "mpr-12417.92",  0x100000, 0x20000, CRC(a806eabf) SHA1(1a61a2135d92b42ee131fd3240bc8a17a96696ab) )
	ROM_LOAD32_BYTE( "mpr-12418.96",  0x100001, 0x20000, CRC(ed1a0f2b) SHA1(1aa87292ca0465fa129d6be81d95dbb77332ecab) )
	ROM_LOAD32_BYTE( "mpr-12419.100", 0x100002, 0x20000, CRC(ce4568cb) SHA1(1ed66e74ce94d41593b498827d9cc243f775d4ba) )
	ROM_LOAD32_BYTE( "mpr-12420.104", 0x100003, 0x20000, CRC(679442eb) SHA1(f88ef0219497f955d8db6783f3636dad52928f46) )
	ROM_LOAD32_BYTE( "epr-12609.93",  0x180000, 0x20000, CRC(a867812f) SHA1(f8950bf794b6c2ec767ffff837d28917b636dbe7) ) //
	ROM_LOAD32_BYTE( "epr-12610.97",  0x180001, 0x20000, CRC(53b99417) SHA1(ab72d35c88695c777d24c5557e5d3ea2d446e51b) ) //
	ROM_LOAD32_BYTE( "epr-12611.101", 0x180002, 0x20000, CRC(bd5c6ab0) SHA1(7632dc4daa8eabe74769369856a8ba451e5bd420) ) // these differ from japan set
	ROM_LOAD32_BYTE( "epr-12612.105", 0x180003, 0x20000, CRC(ac86e890) SHA1(7720c1c8df6de5de50254e97772c15161b796031) ) //

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none??

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-12436.17",    0x00000, 0x10000, CRC(16ec5f0a) SHA1(307b7388b5c36fd4bc2a61f7941db44858e03c5c) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "mpr-12437.11",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) )
	ROM_LOAD( "mpr-12438.12",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) )
	ROM_LOAD( "mpr-12439.13",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) ) // not used in deluxe

	ROM_REGION( 0x10000, "mainpcb:soundcpu2", 0 ) // z80 on extra sound board
	ROM_LOAD( "epr-12535.8",     0x00000, 0x10000, CRC(80453597) SHA1(d3fee7bb4a8964f5cf1cdae80fc3dde06c947839) ) // taken from deluxe cabinet dump

	ROM_REGION( 0x80000, "mainpcb:pcm2", ROMREGION_ERASEFF ) // Sega PCM sound data on extra sound board (same as on main board..)
	ROM_LOAD( "mpr-12437.20",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) ) // taken from deluxe cabinet dump
	ROM_LOAD( "mpr-12438.21",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) ) // "
	ROM_LOAD( "mpr-12439.22",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) ) // "

	ROM_REGION( 0x10000, "mainpcb:commcpu", 0 ) // z80 on network board
	ROM_LOAD( "epr-12587.14",    0x00000, 0x08000, CRC(2afe648b) SHA1(b5bf86f3acbcc23c136185110acecf2c971294fa) ) // taken from twin cabinet dump

	ROM_REGION( 0x10000, "mainpcb:motorcpu", 0 ) // z80 on air board
	ROM_LOAD( "epr-12505.8",     0x00000, 0x08000, CRC(5020788a) SHA1(ed6d1dfb8b6a62d17469e3d09a5b5b864c6b486c) ) // taken from deluxe cabinet dump
ROM_END

//*************************************************************************************************************************
//    Super Monaco GP, Sega X-board
//    CPU: FD1094 (317-0125a)
//    This set is coming from a twin.
//
//    ROMs:
//         IC58 : epr-12561A.58 (27C010 EPROM)
//         IC57 : not populated
//         IC63 : epr-12562A.63 (27C010 EPROM)
//         IC62 : not populated
//
//         IC11 : mpr-12437.11  (831000 MASKROM)
//         IC12 : mpr-12438.12  (831000 MASKROM)
//         IC13 : mpr-12439.13  (831000 MASKROM)
//         IC17 : epr-12436.17  (27C512 EPROM)
//
//         IC21 : not populated
//         IC20 : epr-12574A.20 (27C010 EPROM)
//         IC30 : not populated
//         IC29 : epr-12575A.29 (27C010 EPROM)
//
//         IC40 : not populated
//
//         IC90 : mpr-12425.90  (831000 MASKROM)
//         IC91 : mpr-12421.91  (831000 MASKROM)
//         IC92 : mpr-12417.92  (831000 MASKROM)
//         IC93 : epr-12609.93  (27C010 EPROM)
//
//         IC94 : mpr-12426.94  (831000 MASKROM)
//         IC95 : mpr-12422.95  (831000 MASKROM)
//         IC96 : mpr-12418.96  (831000 MASKROM)
//         IC97 : epr-12610.97  (27C010 EPROM)
//
//         IC98 : mpr-12427.98  (831000 MASKROM)
//         IC99 : mpr-12423.99  (831000 MASKROM)
//         IC100: mpr-12419.100 (831000 MASKROM)
//         IC101: epr-12611.101 (27C010 EPROM)
//
//         IC102: mpr-12428.102 (831000 MASKROM)
//         IC103: mpr-12424.103 (831000 MASKROM)
//         IC104: mpr-12420.104 (831000 MASKROM)
//         IC105: epr-12612.105 (27C010 EPROM)
//
//         IC154: epr-12429.154 (27C512 EPROM)
//         IC153: epr-12430.153 (27C512 EPROM)
//         IC152: epr-12431.152 (27C512 EPROM)
//
ROM_START( smgpu2 )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-12561a.58", 0x00000, 0x20000, CRC(e505eb89) SHA1(bfb9a7a8b13ae454a92349e57215562477cd2cd2) )
	ROM_LOAD16_BYTE( "epr-12562a.63", 0x00001, 0x20000, CRC(c3af4215) SHA1(c46829e08d5492515de5d3269b0e899705d0b108) )

	ROM_REGION( 0x2000, "mainpcb:maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0125a.key", 0x0000, 0x2000, CRC(3ecdb120) SHA1(c484198e4509d79214e78d4a47e9a7e339f7a2ed) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-12574a.20", 0x00000, 0x20000, CRC(f8b5c38b) SHA1(0184d5a1b71fb42d33dbaaad99d2c0fbc5750e7e) )
	ROM_LOAD16_BYTE( "epr-12575a.29", 0x00001, 0x20000, CRC(248b1d17) SHA1(22f1e0d0d698abdf0cb1954f1f6382432a12c186) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-12429.154", 0x00000, 0x10000, CRC(5851e614) SHA1(3dc97237ede2c6125e92ea6efc68a748d0ec69be) )
	ROM_LOAD( "epr-12430.153", 0x10000, 0x10000, CRC(05e00134) SHA1(8baaa80815d5dabd38dc8600e357975b96d23b95) )
	ROM_LOAD( "epr-12431.152", 0x20000, 0x10000, CRC(35572f4a) SHA1(d66456ecf7b59f81736fb873c553926b56bb3977))

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "mpr-12425.90",  0x000000, 0x20000, CRC(14bf2a15) SHA1(84db3ac09e4a8fe470ac051d8d5de1814b48bc72) )
	ROM_LOAD32_BYTE( "mpr-12426.94",  0x000001, 0x20000, CRC(28b60dc0) SHA1(ad69d449434853445a076319a55a29014217a100) )
	ROM_LOAD32_BYTE( "mpr-12427.98",  0x000002, 0x20000, CRC(0a367928) SHA1(bcb558b7c23906397e66a7f046b09eb5036c0888) )
	ROM_LOAD32_BYTE( "mpr-12428.102", 0x000003, 0x20000, CRC(efa80ad5) SHA1(9bc7c3fb60cc076f29a0af487d58e5b48f1c4b06) )
	ROM_LOAD32_BYTE( "mpr-12421.91",  0x080000, 0x20000, CRC(25f46140) SHA1(ea75e364cf52636d100158f79be627e36da8c327) )
	ROM_LOAD32_BYTE( "mpr-12422.95",  0x080001, 0x20000, CRC(cb51c8f6) SHA1(5af56ae1916c3212b8d5b9e4bccbbe1916694f89) )
	ROM_LOAD32_BYTE( "mpr-12423.99",  0x080002, 0x20000, CRC(0be9818e) SHA1(637a8201416e73d53f7e2502ea0a5277e43c167d) )
	ROM_LOAD32_BYTE( "mpr-12424.103", 0x080003, 0x20000, CRC(0ce00dfc) SHA1(3b1990977ec7ad4c3bea66527707cff2cd8d5a98) )
	ROM_LOAD32_BYTE( "mpr-12417.92",  0x100000, 0x20000, CRC(a806eabf) SHA1(1a61a2135d92b42ee131fd3240bc8a17a96696ab) )
	ROM_LOAD32_BYTE( "mpr-12418.96",  0x100001, 0x20000, CRC(ed1a0f2b) SHA1(1aa87292ca0465fa129d6be81d95dbb77332ecab) )
	ROM_LOAD32_BYTE( "mpr-12419.100", 0x100002, 0x20000, CRC(ce4568cb) SHA1(1ed66e74ce94d41593b498827d9cc243f775d4ba) )
	ROM_LOAD32_BYTE( "mpr-12420.104", 0x100003, 0x20000, CRC(679442eb) SHA1(f88ef0219497f955d8db6783f3636dad52928f46) )
	ROM_LOAD32_BYTE( "epr-12609.93",  0x180000, 0x20000, CRC(a867812f) SHA1(f8950bf794b6c2ec767ffff837d28917b636dbe7) ) //
	ROM_LOAD32_BYTE( "epr-12610.97",  0x180001, 0x20000, CRC(53b99417) SHA1(ab72d35c88695c777d24c5557e5d3ea2d446e51b) ) //
	ROM_LOAD32_BYTE( "epr-12611.101", 0x180002, 0x20000, CRC(bd5c6ab0) SHA1(7632dc4daa8eabe74769369856a8ba451e5bd420) ) // these differ from japan set
	ROM_LOAD32_BYTE( "epr-12612.105", 0x180003, 0x20000, CRC(ac86e890) SHA1(7720c1c8df6de5de50254e97772c15161b796031) ) //

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none??

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-12436.17",    0x00000, 0x10000, CRC(16ec5f0a) SHA1(307b7388b5c36fd4bc2a61f7941db44858e03c5c) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "mpr-12437.11",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) )
	ROM_LOAD( "mpr-12438.12",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) )
	ROM_LOAD( "mpr-12439.13",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) ) // not used in deluxe

	ROM_REGION( 0x10000, "mainpcb:soundcpu2", 0 ) // z80 on extra sound board
	ROM_LOAD( "epr-12535.8",     0x00000, 0x10000, CRC(80453597) SHA1(d3fee7bb4a8964f5cf1cdae80fc3dde06c947839) ) // taken from deluxe cabinet dump

	ROM_REGION( 0x80000, "mainpcb:pcm2", ROMREGION_ERASEFF ) // Sega PCM sound data on extra sound board (same as on main board..)
	ROM_LOAD( "mpr-12437.20",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) ) // taken from deluxe cabinet dump
	ROM_LOAD( "mpr-12438.21",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) ) // "
	ROM_LOAD( "mpr-12439.22",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) ) // "

	ROM_REGION( 0x10000, "mainpcb:commcpu", 0 ) // z80 on network board
	ROM_LOAD( "epr-12587.14",    0x00000, 0x08000, CRC(2afe648b) SHA1(b5bf86f3acbcc23c136185110acecf2c971294fa) )

	ROM_REGION( 0x10000, "mainpcb:motorcpu", 0 ) // z80 on air board
	ROM_LOAD( "epr-12505.8",     0x00000, 0x08000, CRC(5020788a) SHA1(ed6d1dfb8b6a62d17469e3d09a5b5b864c6b486c) ) // taken from deluxe cabinet dump
ROM_END

ROM_START( smgpu2d )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bootleg_epr-12561a.58", 0x00000, 0x20000, CRC(30e6fb0e) SHA1(7cb079f7a1160e08a01ceebbcb528fb19bd3e5fb) )
	ROM_LOAD16_BYTE( "bootleg_epr-12562a.63", 0x00001, 0x20000, CRC(61b59994) SHA1(e8bbac96321f85a170e4cddbe0c66bc7e21024f0) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-12574a.20", 0x00000, 0x20000, CRC(f8b5c38b) SHA1(0184d5a1b71fb42d33dbaaad99d2c0fbc5750e7e) )
	ROM_LOAD16_BYTE( "epr-12575a.29", 0x00001, 0x20000, CRC(248b1d17) SHA1(22f1e0d0d698abdf0cb1954f1f6382432a12c186) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-12429.154", 0x00000, 0x10000, CRC(5851e614) SHA1(3dc97237ede2c6125e92ea6efc68a748d0ec69be) )
	ROM_LOAD( "epr-12430.153", 0x10000, 0x10000, CRC(05e00134) SHA1(8baaa80815d5dabd38dc8600e357975b96d23b95) )
	ROM_LOAD( "epr-12431.152", 0x20000, 0x10000, CRC(35572f4a) SHA1(d66456ecf7b59f81736fb873c553926b56bb3977))

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "mpr-12425.90",  0x000000, 0x20000, CRC(14bf2a15) SHA1(84db3ac09e4a8fe470ac051d8d5de1814b48bc72) )
	ROM_LOAD32_BYTE( "mpr-12426.94",  0x000001, 0x20000, CRC(28b60dc0) SHA1(ad69d449434853445a076319a55a29014217a100) )
	ROM_LOAD32_BYTE( "mpr-12427.98",  0x000002, 0x20000, CRC(0a367928) SHA1(bcb558b7c23906397e66a7f046b09eb5036c0888) )
	ROM_LOAD32_BYTE( "mpr-12428.102", 0x000003, 0x20000, CRC(efa80ad5) SHA1(9bc7c3fb60cc076f29a0af487d58e5b48f1c4b06) )
	ROM_LOAD32_BYTE( "mpr-12421.91",  0x080000, 0x20000, CRC(25f46140) SHA1(ea75e364cf52636d100158f79be627e36da8c327) )
	ROM_LOAD32_BYTE( "mpr-12422.95",  0x080001, 0x20000, CRC(cb51c8f6) SHA1(5af56ae1916c3212b8d5b9e4bccbbe1916694f89) )
	ROM_LOAD32_BYTE( "mpr-12423.99",  0x080002, 0x20000, CRC(0be9818e) SHA1(637a8201416e73d53f7e2502ea0a5277e43c167d) )
	ROM_LOAD32_BYTE( "mpr-12424.103", 0x080003, 0x20000, CRC(0ce00dfc) SHA1(3b1990977ec7ad4c3bea66527707cff2cd8d5a98) )
	ROM_LOAD32_BYTE( "mpr-12417.92",  0x100000, 0x20000, CRC(a806eabf) SHA1(1a61a2135d92b42ee131fd3240bc8a17a96696ab) )
	ROM_LOAD32_BYTE( "mpr-12418.96",  0x100001, 0x20000, CRC(ed1a0f2b) SHA1(1aa87292ca0465fa129d6be81d95dbb77332ecab) )
	ROM_LOAD32_BYTE( "mpr-12419.100", 0x100002, 0x20000, CRC(ce4568cb) SHA1(1ed66e74ce94d41593b498827d9cc243f775d4ba) )
	ROM_LOAD32_BYTE( "mpr-12420.104", 0x100003, 0x20000, CRC(679442eb) SHA1(f88ef0219497f955d8db6783f3636dad52928f46) )
	ROM_LOAD32_BYTE( "epr-12609.93",  0x180000, 0x20000, CRC(a867812f) SHA1(f8950bf794b6c2ec767ffff837d28917b636dbe7) ) //
	ROM_LOAD32_BYTE( "epr-12610.97",  0x180001, 0x20000, CRC(53b99417) SHA1(ab72d35c88695c777d24c5557e5d3ea2d446e51b) ) //
	ROM_LOAD32_BYTE( "epr-12611.101", 0x180002, 0x20000, CRC(bd5c6ab0) SHA1(7632dc4daa8eabe74769369856a8ba451e5bd420) ) // these differ from japan set
	ROM_LOAD32_BYTE( "epr-12612.105", 0x180003, 0x20000, CRC(ac86e890) SHA1(7720c1c8df6de5de50254e97772c15161b796031) ) //

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none??

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-12436.17",    0x00000, 0x10000, CRC(16ec5f0a) SHA1(307b7388b5c36fd4bc2a61f7941db44858e03c5c) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "mpr-12437.11",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) )
	ROM_LOAD( "mpr-12438.12",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) )
	ROM_LOAD( "mpr-12439.13",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) ) // not used in deluxe

	ROM_REGION( 0x10000, "mainpcb:soundcpu2", 0 ) // z80 on extra sound board
	ROM_LOAD( "epr-12535.8",     0x00000, 0x10000, CRC(80453597) SHA1(d3fee7bb4a8964f5cf1cdae80fc3dde06c947839) ) // taken from deluxe cabinet dump

	ROM_REGION( 0x80000, "mainpcb:pcm2", ROMREGION_ERASEFF ) // Sega PCM sound data on extra sound board (same as on main board..)
	ROM_LOAD( "mpr-12437.20",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) ) // taken from deluxe cabinet dump
	ROM_LOAD( "mpr-12438.21",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) ) // "
	ROM_LOAD( "mpr-12439.22",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) ) // "

	ROM_REGION( 0x10000, "mainpcb:commcpu", 0 ) // z80 on network board
	ROM_LOAD( "epr-12587.14",    0x00000, 0x08000, CRC(2afe648b) SHA1(b5bf86f3acbcc23c136185110acecf2c971294fa) )

	ROM_REGION( 0x10000, "mainpcb:motorcpu", 0 ) // z80 on air board
	ROM_LOAD( "epr-12505.8",     0x00000, 0x08000, CRC(5020788a) SHA1(ed6d1dfb8b6a62d17469e3d09a5b5b864c6b486c) ) // taken from deluxe cabinet dump
ROM_END

//*************************************************************************************************************************
//  Super Monaco GP, Sega X-board
//  CPU: FD1094 (317-0124a)
//
ROM_START( smgpj )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-12432b.58", 0x00000, 0x20000, CRC(c1a29db1) SHA1(0122d366899f98f7a60b0c9bddeece7995cebf83) )
	ROM_LOAD16_BYTE( "epr-12433b.63", 0x00001, 0x20000, CRC(97199eb1) SHA1(3baccf8159821d4b4d5caedf5eb691f07372be93) )

	ROM_REGION( 0x2000, "mainpcb:maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0124a.key", 0x0000, 0x2000, CRC(022a8a16) SHA1(4fd80105cb85ccba77cf1e76a21d6e245d5d2e7d) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-12441a.20", 0x00000, 0x20000, CRC(2c9599c1) SHA1(79206f38c2976bd9299ed37bf62ac26dd3fba801) )
	ROM_LOAD16_BYTE( "epr-12442a.29", 0x00001, 0x20000, CRC(77a5ec16) SHA1(b8cf6a3f12689d89bbdd9fb39d1cb7d1a3c10602) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-12429.154", 0x00000, 0x10000, CRC(5851e614) SHA1(3dc97237ede2c6125e92ea6efc68a748d0ec69be) )
	ROM_LOAD( "epr-12430.153", 0x10000, 0x10000, CRC(05e00134) SHA1(8baaa80815d5dabd38dc8600e357975b96d23b95) )
	ROM_LOAD( "epr-12431.152", 0x20000, 0x10000, CRC(35572f4a) SHA1(d66456ecf7b59f81736fb873c553926b56bb3977))

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "mpr-12425.90",  0x000000, 0x20000, CRC(14bf2a15) SHA1(84db3ac09e4a8fe470ac051d8d5de1814b48bc72) )
	ROM_LOAD32_BYTE( "mpr-12426.94",  0x000001, 0x20000, CRC(28b60dc0) SHA1(ad69d449434853445a076319a55a29014217a100) )
	ROM_LOAD32_BYTE( "mpr-12427.98",  0x000002, 0x20000, CRC(0a367928) SHA1(bcb558b7c23906397e66a7f046b09eb5036c0888) )
	ROM_LOAD32_BYTE( "mpr-12428.102", 0x000003, 0x20000, CRC(efa80ad5) SHA1(9bc7c3fb60cc076f29a0af487d58e5b48f1c4b06) )
	ROM_LOAD32_BYTE( "mpr-12421.91",  0x080000, 0x20000, CRC(25f46140) SHA1(ea75e364cf52636d100158f79be627e36da8c327) )
	ROM_LOAD32_BYTE( "mpr-12422.95",  0x080001, 0x20000, CRC(cb51c8f6) SHA1(5af56ae1916c3212b8d5b9e4bccbbe1916694f89) )
	ROM_LOAD32_BYTE( "mpr-12423.99",  0x080002, 0x20000, CRC(0be9818e) SHA1(637a8201416e73d53f7e2502ea0a5277e43c167d) )
	ROM_LOAD32_BYTE( "mpr-12424.103", 0x080003, 0x20000, CRC(0ce00dfc) SHA1(3b1990977ec7ad4c3bea66527707cff2cd8d5a98) )
	ROM_LOAD32_BYTE( "mpr-12417.92",  0x100000, 0x20000, CRC(a806eabf) SHA1(1a61a2135d92b42ee131fd3240bc8a17a96696ab) )
	ROM_LOAD32_BYTE( "mpr-12418.96",  0x100001, 0x20000, CRC(ed1a0f2b) SHA1(1aa87292ca0465fa129d6be81d95dbb77332ecab) )
	ROM_LOAD32_BYTE( "mpr-12419.100", 0x100002, 0x20000, CRC(ce4568cb) SHA1(1ed66e74ce94d41593b498827d9cc243f775d4ba) )
	ROM_LOAD32_BYTE( "mpr-12420.104", 0x100003, 0x20000, CRC(679442eb) SHA1(f88ef0219497f955d8db6783f3636dad52928f46) )
	ROM_LOAD32_BYTE( "epr-12413.93",  0x180000, 0x20000, CRC(2f1693df) SHA1(ba1e654a1b5fae661b0dae4a8ed04ff50fb546a2) )
	ROM_LOAD32_BYTE( "epr-12414.97",  0x180001, 0x20000, CRC(c78f3d45) SHA1(665750907ed11c89c2ea5c410eac2808445131ae) )
	ROM_LOAD32_BYTE( "epr-12415.101", 0x180002, 0x20000, CRC(6080e9ed) SHA1(eb1b871453f76e6a65d20fa9d4bddc1c9f940b4d) )
	ROM_LOAD32_BYTE( "epr-12416.105", 0x180003, 0x20000, CRC(6f1f2769) SHA1(d00d26cd1052d4b46c432b6b69cb2d83179d52a6) )

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none??

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-12436.17",    0x00000, 0x10000, CRC(16ec5f0a) SHA1(307b7388b5c36fd4bc2a61f7941db44858e03c5c) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "mpr-12437.11",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) )
	ROM_LOAD( "mpr-12438.12",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) )
	ROM_LOAD( "mpr-12439.13",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) ) // not used in deluxe

	ROM_REGION( 0x10000, "mainpcb:soundcpu2", 0 ) // z80 on extra sound board
	ROM_LOAD( "epr-12535.8",     0x00000, 0x10000, CRC(80453597) SHA1(d3fee7bb4a8964f5cf1cdae80fc3dde06c947839) ) // taken from deluxe cabinet dump

	ROM_REGION( 0x80000, "mainpcb:pcm2", ROMREGION_ERASEFF ) // Sega PCM sound data on extra sound board (same as on main board..)
	ROM_LOAD( "mpr-12437.20",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) ) // taken from deluxe cabinet dump
	ROM_LOAD( "mpr-12438.21",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) ) // "
	ROM_LOAD( "mpr-12439.22",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) ) // "

	ROM_REGION( 0x10000, "mainpcb:commcpu", 0 ) // z80 on network board
	ROM_LOAD( "epr-12587.14",    0x00000, 0x08000, CRC(2afe648b) SHA1(b5bf86f3acbcc23c136185110acecf2c971294fa) ) // taken from twin cabinet dump

	ROM_REGION( 0x10000, "mainpcb:motorcpu", 0 ) // z80 on air board
	ROM_LOAD( "epr-12505.8",     0x00000, 0x08000, CRC(5020788a) SHA1(ed6d1dfb8b6a62d17469e3d09a5b5b864c6b486c) ) // taken from deluxe cabinet dump
ROM_END


ROM_START( smgpjd )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bootleg_epr-12432b.58", 0x00000, 0x20000, CRC(1c46c4de) SHA1(32711dfb8209317c6c2fc0fbb8f04b907123a4dc) )
	ROM_LOAD16_BYTE( "bootleg_epr-12433b.63", 0x00001, 0x20000, CRC(ae8a4942) SHA1(53f52e6431353d611ac6a3520af053f955b79b37))

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-12441a.20", 0x00000, 0x20000, CRC(2c9599c1) SHA1(79206f38c2976bd9299ed37bf62ac26dd3fba801) )
	ROM_LOAD16_BYTE( "epr-12442a.29", 0x00001, 0x20000, CRC(77a5ec16) SHA1(b8cf6a3f12689d89bbdd9fb39d1cb7d1a3c10602) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-12429.154", 0x00000, 0x10000, CRC(5851e614) SHA1(3dc97237ede2c6125e92ea6efc68a748d0ec69be) )
	ROM_LOAD( "epr-12430.153", 0x10000, 0x10000, CRC(05e00134) SHA1(8baaa80815d5dabd38dc8600e357975b96d23b95) )
	ROM_LOAD( "epr-12431.152", 0x20000, 0x10000, CRC(35572f4a) SHA1(d66456ecf7b59f81736fb873c553926b56bb3977))

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "mpr-12425.90",  0x000000, 0x20000, CRC(14bf2a15) SHA1(84db3ac09e4a8fe470ac051d8d5de1814b48bc72) )
	ROM_LOAD32_BYTE( "mpr-12426.94",  0x000001, 0x20000, CRC(28b60dc0) SHA1(ad69d449434853445a076319a55a29014217a100) )
	ROM_LOAD32_BYTE( "mpr-12427.98",  0x000002, 0x20000, CRC(0a367928) SHA1(bcb558b7c23906397e66a7f046b09eb5036c0888) )
	ROM_LOAD32_BYTE( "mpr-12428.102", 0x000003, 0x20000, CRC(efa80ad5) SHA1(9bc7c3fb60cc076f29a0af487d58e5b48f1c4b06) )
	ROM_LOAD32_BYTE( "mpr-12421.91",  0x080000, 0x20000, CRC(25f46140) SHA1(ea75e364cf52636d100158f79be627e36da8c327) )
	ROM_LOAD32_BYTE( "mpr-12422.95",  0x080001, 0x20000, CRC(cb51c8f6) SHA1(5af56ae1916c3212b8d5b9e4bccbbe1916694f89) )
	ROM_LOAD32_BYTE( "mpr-12423.99",  0x080002, 0x20000, CRC(0be9818e) SHA1(637a8201416e73d53f7e2502ea0a5277e43c167d) )
	ROM_LOAD32_BYTE( "mpr-12424.103", 0x080003, 0x20000, CRC(0ce00dfc) SHA1(3b1990977ec7ad4c3bea66527707cff2cd8d5a98) )
	ROM_LOAD32_BYTE( "mpr-12417.92",  0x100000, 0x20000, CRC(a806eabf) SHA1(1a61a2135d92b42ee131fd3240bc8a17a96696ab) )
	ROM_LOAD32_BYTE( "mpr-12418.96",  0x100001, 0x20000, CRC(ed1a0f2b) SHA1(1aa87292ca0465fa129d6be81d95dbb77332ecab) )
	ROM_LOAD32_BYTE( "mpr-12419.100", 0x100002, 0x20000, CRC(ce4568cb) SHA1(1ed66e74ce94d41593b498827d9cc243f775d4ba) )
	ROM_LOAD32_BYTE( "mpr-12420.104", 0x100003, 0x20000, CRC(679442eb) SHA1(f88ef0219497f955d8db6783f3636dad52928f46) )
	ROM_LOAD32_BYTE( "epr-12413.93",  0x180000, 0x20000, CRC(2f1693df) SHA1(ba1e654a1b5fae661b0dae4a8ed04ff50fb546a2) )
	ROM_LOAD32_BYTE( "epr-12414.97",  0x180001, 0x20000, CRC(c78f3d45) SHA1(665750907ed11c89c2ea5c410eac2808445131ae) )
	ROM_LOAD32_BYTE( "epr-12415.101", 0x180002, 0x20000, CRC(6080e9ed) SHA1(eb1b871453f76e6a65d20fa9d4bddc1c9f940b4d) )
	ROM_LOAD32_BYTE( "epr-12416.105", 0x180003, 0x20000, CRC(6f1f2769) SHA1(d00d26cd1052d4b46c432b6b69cb2d83179d52a6) )

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none??

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-12436.17",    0x00000, 0x10000, CRC(16ec5f0a) SHA1(307b7388b5c36fd4bc2a61f7941db44858e03c5c) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "mpr-12437.11",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) )
	ROM_LOAD( "mpr-12438.12",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) )
	ROM_LOAD( "mpr-12439.13",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) ) // not used in deluxe

	ROM_REGION( 0x10000, "mainpcb:soundcpu2", 0 ) // z80 on extra sound board
	ROM_LOAD( "epr-12535.8",     0x00000, 0x10000, CRC(80453597) SHA1(d3fee7bb4a8964f5cf1cdae80fc3dde06c947839) ) // taken from deluxe cabinet dump

	ROM_REGION( 0x80000, "mainpcb:pcm2", ROMREGION_ERASEFF ) // Sega PCM sound data on extra sound board (same as on main board..)
	ROM_LOAD( "mpr-12437.20",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) ) // taken from deluxe cabinet dump
	ROM_LOAD( "mpr-12438.21",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) ) // "
	ROM_LOAD( "mpr-12439.22",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) ) // "

	ROM_REGION( 0x10000, "mainpcb:commcpu", 0 ) // z80 on network board
	ROM_LOAD( "epr-12587.14",    0x00000, 0x08000, CRC(2afe648b) SHA1(b5bf86f3acbcc23c136185110acecf2c971294fa) ) // taken from twin cabinet dump

	ROM_REGION( 0x10000, "mainpcb:motorcpu", 0 ) // z80 on air board
	ROM_LOAD( "epr-12505.8",     0x00000, 0x08000, CRC(5020788a) SHA1(ed6d1dfb8b6a62d17469e3d09a5b5b864c6b486c) ) // taken from deluxe cabinet dump
ROM_END


//*************************************************************************************************************************
//  Super Monaco GP, Sega X-board
//  CPU: FD1094 (317-0124a)
//
ROM_START( smgpja )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-12432a.58", 0x00000, 0x20000, CRC(22517672) SHA1(db9ac40e83e9786bc9dad70f62c2080d3df694ee) )
	ROM_LOAD16_BYTE( "epr-12433a.63", 0x00001, 0x20000, CRC(a46b5d13) SHA1(3a7de5cb6f3e6d726f0ea886a87125dedc6f849f) )

	ROM_REGION( 0x2000, "mainpcb:maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0124a.key", 0x0000, 0x2000, CRC(022a8a16) SHA1(4fd80105cb85ccba77cf1e76a21d6e245d5d2e7d) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-12441a.20", 0x00000, 0x20000, CRC(2c9599c1) SHA1(79206f38c2976bd9299ed37bf62ac26dd3fba801) )
	ROM_LOAD16_BYTE( "epr-12442a.29", 0x00001, 0x20000, CRC(77a5ec16) SHA1(b8cf6a3f12689d89bbdd9fb39d1cb7d1a3c10602) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-12429.154", 0x00000, 0x10000, CRC(5851e614) SHA1(3dc97237ede2c6125e92ea6efc68a748d0ec69be) )
	ROM_LOAD( "epr-12430.153", 0x10000, 0x10000, CRC(05e00134) SHA1(8baaa80815d5dabd38dc8600e357975b96d23b95) )
	ROM_LOAD( "epr-12431.152", 0x20000, 0x10000, CRC(35572f4a) SHA1(d66456ecf7b59f81736fb873c553926b56bb3977))

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "mpr-12425.90",  0x000000, 0x20000, CRC(14bf2a15) SHA1(84db3ac09e4a8fe470ac051d8d5de1814b48bc72) )
	ROM_LOAD32_BYTE( "mpr-12426.94",  0x000001, 0x20000, CRC(28b60dc0) SHA1(ad69d449434853445a076319a55a29014217a100) )
	ROM_LOAD32_BYTE( "mpr-12427.98",  0x000002, 0x20000, CRC(0a367928) SHA1(bcb558b7c23906397e66a7f046b09eb5036c0888) )
	ROM_LOAD32_BYTE( "mpr-12428.102", 0x000003, 0x20000, CRC(efa80ad5) SHA1(9bc7c3fb60cc076f29a0af487d58e5b48f1c4b06) )
	ROM_LOAD32_BYTE( "mpr-12421.91",  0x080000, 0x20000, CRC(25f46140) SHA1(ea75e364cf52636d100158f79be627e36da8c327) )
	ROM_LOAD32_BYTE( "mpr-12422.95",  0x080001, 0x20000, CRC(cb51c8f6) SHA1(5af56ae1916c3212b8d5b9e4bccbbe1916694f89) )
	ROM_LOAD32_BYTE( "mpr-12423.99",  0x080002, 0x20000, CRC(0be9818e) SHA1(637a8201416e73d53f7e2502ea0a5277e43c167d) )
	ROM_LOAD32_BYTE( "mpr-12424.103", 0x080003, 0x20000, CRC(0ce00dfc) SHA1(3b1990977ec7ad4c3bea66527707cff2cd8d5a98) )
	ROM_LOAD32_BYTE( "mpr-12417.92",  0x100000, 0x20000, CRC(a806eabf) SHA1(1a61a2135d92b42ee131fd3240bc8a17a96696ab) )
	ROM_LOAD32_BYTE( "mpr-12418.96",  0x100001, 0x20000, CRC(ed1a0f2b) SHA1(1aa87292ca0465fa129d6be81d95dbb77332ecab) )
	ROM_LOAD32_BYTE( "mpr-12419.100", 0x100002, 0x20000, CRC(ce4568cb) SHA1(1ed66e74ce94d41593b498827d9cc243f775d4ba) )
	ROM_LOAD32_BYTE( "mpr-12420.104", 0x100003, 0x20000, CRC(679442eb) SHA1(f88ef0219497f955d8db6783f3636dad52928f46) )
	ROM_LOAD32_BYTE( "epr-12413.93",  0x180000, 0x20000, CRC(2f1693df) SHA1(ba1e654a1b5fae661b0dae4a8ed04ff50fb546a2) )
	ROM_LOAD32_BYTE( "epr-12414.97",  0x180001, 0x20000, CRC(c78f3d45) SHA1(665750907ed11c89c2ea5c410eac2808445131ae) )
	ROM_LOAD32_BYTE( "epr-12415.101", 0x180002, 0x20000, CRC(6080e9ed) SHA1(eb1b871453f76e6a65d20fa9d4bddc1c9f940b4d) )
	ROM_LOAD32_BYTE( "epr-12416.105", 0x180003, 0x20000, CRC(6f1f2769) SHA1(d00d26cd1052d4b46c432b6b69cb2d83179d52a6) )

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none??

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-12436.17",    0x00000, 0x10000, CRC(16ec5f0a) SHA1(307b7388b5c36fd4bc2a61f7941db44858e03c5c) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "mpr-12437.11",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) )
	ROM_LOAD( "mpr-12438.12",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) )
	ROM_LOAD( "mpr-12439.13",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) ) // not used in deluxe

	ROM_REGION( 0x10000, "mainpcb:soundcpu2", 0 ) // z80 on extra sound board
	ROM_LOAD( "epr-12535.8",     0x00000, 0x10000, CRC(80453597) SHA1(d3fee7bb4a8964f5cf1cdae80fc3dde06c947839) ) // taken from deluxe cabinet dump

	ROM_REGION( 0x80000, "mainpcb:pcm2", ROMREGION_ERASEFF ) // Sega PCM sound data on extra sound board (same as on main board..)
	ROM_LOAD( "mpr-12437.20",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) ) // taken from deluxe cabinet dump
	ROM_LOAD( "mpr-12438.21",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) ) // "
	ROM_LOAD( "mpr-12439.22",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) ) // "

	ROM_REGION( 0x10000, "mainpcb:commcpu", 0 ) // z80 on network board
	ROM_LOAD( "epr-12587.14",    0x00000, 0x08000, CRC(2afe648b) SHA1(b5bf86f3acbcc23c136185110acecf2c971294fa) ) // taken from twin cabinet dump

	ROM_REGION( 0x10000, "mainpcb:motorcpu", 0 ) // z80 on air board
	ROM_LOAD( "epr-12505.8",     0x00000, 0x08000, CRC(5020788a) SHA1(ed6d1dfb8b6a62d17469e3d09a5b5b864c6b486c) ) // taken from deluxe cabinet dump
ROM_END


//*************************************************************************************************************************
//*************************************************************************************************************************
//*************************************************************************************************************************
//  AB Cop (World), Sega X-board
//  CPU: FD1094 (317-0169b)
//
ROM_START( abcop )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-13568b.ic58", 0x00000, 0x20000, CRC(f88db35b) SHA1(7d85c1194a2aa08427333d2ffc2a8d4f7e1beff0) )
	ROM_LOAD16_BYTE( "epr-13556b.ic63", 0x00001, 0x20000, CRC(337bf32e) SHA1(dafb9d9b3baf79ca76355278e8a14294f186790a) )
	ROM_LOAD16_BYTE( "epr-13559.ic57",  0x40000, 0x20000, CRC(4588bf19) SHA1(6a8b3d4450ac0bc41b46e6a4e1b44d82112fcd64) )
	ROM_LOAD16_BYTE( "epr-13558.ic62",  0x40001, 0x20000, CRC(11259ed4) SHA1(e7de174a0bdb1d1111e5e419f1d501ab5be1d32d) )

	ROM_REGION( 0x2000, "mainpcb:maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0169b.key", 0x0000, 0x2000, CRC(058da36e) SHA1(ab3f68a90725063c68fc5d0f8dbece1f8940dc7d) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-13566.ic20", 0x00000, 0x20000, CRC(22e52f32) SHA1(c67a4ccb88becc58dddcbfea0a1ac2017f7b2929) )
	ROM_LOAD16_BYTE( "epr-13565.ic29", 0x00001, 0x20000, CRC(a21784bd) SHA1(b40ba0ef65bbfe514625253f6aeec14bf4bcf08c) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "opr-13553.ic154", 0x00000, 0x10000, CRC(8c418837) SHA1(e325db39fae768865e20d2cd1ee2b91a9b0165f5) )
	ROM_LOAD( "opr-13554.ic153", 0x10000, 0x10000, CRC(4e3df9f0) SHA1(8b481c2cd25c58612ac8ac3ffb7eeae9ca247d2e) )
	ROM_LOAD( "opr-13555.ic152", 0x20000, 0x10000, CRC(6c4a1d42) SHA1(6c37b045b21173f1e2f7bd19d01c00979b8107fb) )

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "opr-13552.ic90",  0x000000, 0x20000, CRC(cc2cf706) SHA1(ad39c22e652ebcd90ffb5e17ae35985645f93c71) )
	ROM_LOAD32_BYTE( "opr-13551.ic94",  0x000001, 0x20000, CRC(d6f276c1) SHA1(9ec68157ea460e09ef4b69aa8ea17687dc47ea59) )
	ROM_LOAD32_BYTE( "opr-13550.ic98",  0x000002, 0x20000, CRC(f16518dd) SHA1(a5f1785cd28f03069cb238ac92c6afb5a26cbd37) )
	ROM_LOAD32_BYTE( "opr-13549.ic102", 0x000003, 0x20000, CRC(cba407a7) SHA1(e7684d3b40baa6d832b887fd85ad67fbad8aa7de) )
	ROM_LOAD32_BYTE( "opr-13548.ic91",  0x080000, 0x20000, CRC(080fd805) SHA1(e729565815a3a37462cfee460b7392d2f08e96e5) )
	ROM_LOAD32_BYTE( "opr-13547.ic95",  0x080001, 0x20000, CRC(42d4dd68) SHA1(6ae1f3585ebb20fd2908456d6fa41a893261277e) )
	ROM_LOAD32_BYTE( "opr-13546.ic99",  0x080002, 0x20000, CRC(ca6fbf3d) SHA1(49c3516d87f1546fa7efe785fc5c064d90b1cb8e) )
	ROM_LOAD32_BYTE( "opr-13545.ic103", 0x080003, 0x20000, CRC(c9e58dd2) SHA1(ace2e1630d8df2454183ffdbe26d8cb6d199e940) )
	ROM_LOAD32_BYTE( "opr-13544.ic92",  0x100000, 0x20000, CRC(9c1436d9) SHA1(5156e1b5c7461f6dc0d449b86b6b72153b290a4c) )
	ROM_LOAD32_BYTE( "opr-13543.ic96",  0x100001, 0x20000, CRC(2c1c8f0e) SHA1(19c9fd4272a3db18381f435ed6cd01f994c655e7) )
	ROM_LOAD32_BYTE( "opr-13542.ic100", 0x100002, 0x20000, CRC(01fd52b8) SHA1(b4ab13c7b2b2ffcfdab37d8e4855d5ef8823f1cc) )
	ROM_LOAD32_BYTE( "opr-13541.ic104", 0x100003, 0x20000, CRC(a45c547b) SHA1(d93aaa850d14a7699a1b0411e823088a9bce7553) )
	ROM_LOAD32_BYTE( "opr-13540.ic93",  0x180000, 0x20000, CRC(84b42ab0) SHA1(d24ba7fe23463fc5813ef26e0395951559d6d162) )
	ROM_LOAD32_BYTE( "opr-13539.ic97",  0x180001, 0x20000, CRC(cd6e524f) SHA1(e6df2552a84b2da95301486379c78679b0297634) )
	ROM_LOAD32_BYTE( "opr-13538.ic101", 0x180002, 0x20000, CRC(bf9a4586) SHA1(6013dee83375d72d262c8c04c2e668afea2e216c) )
	ROM_LOAD32_BYTE( "opr-13537.ic105", 0x180003, 0x20000, CRC(fa14ed3e) SHA1(d684496ade2517696a56c1423dd4686d283c133f) )

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // ground data
	ROM_LOAD( "opr-13564.ic40",  0x00000, 0x10000, CRC(e70ba138) SHA1(85eb6618f408642227056d278f10dec8dcc5a80d) )

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-13560.ic17",    0x00000, 0x10000, CRC(83050925) SHA1(118710e5789c7999bb7326df4d7bd207cbffdfd4) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "opr-13563.ic11",    0x00000, 0x20000, CRC(4083e74f) SHA1(e48c7ce0aa3406af0bbf79c169a8157693c97041) )
	ROM_LOAD( "opr-13562.ic12",    0x20000, 0x20000, CRC(3cc3968f) SHA1(d25647f6a3fa939ba30e03e7334362ef0749b23a) )
	ROM_LOAD( "opr-13561.ic13",    0x40000, 0x20000, CRC(80a7c02a) SHA1(7e8c1b9ba270d8657dbe90ed8be2e4b6463e5928) )
ROM_END

ROM_START( abcopd )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bootleg_epr13568b.ic58", 0x00000, 0x20000, CRC(3c367a01) SHA1(ddb37a399a67818c5b14c4fac1f25d0e660a7b0f) )
	ROM_LOAD16_BYTE( "bootleg_epr13556b.ic63", 0x00001, 0x20000, CRC(1078246e) SHA1(3490f46c1f52d41f96fb449bdcee5fbec871aaca) )
	ROM_LOAD16_BYTE( "epr-13559.ic57",  0x40000, 0x20000, CRC(4588bf19) SHA1(6a8b3d4450ac0bc41b46e6a4e1b44d82112fcd64) )
	ROM_LOAD16_BYTE( "epr-13558.ic62",  0x40001, 0x20000, CRC(11259ed4) SHA1(e7de174a0bdb1d1111e5e419f1d501ab5be1d32d) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-13566.ic20", 0x00000, 0x20000, CRC(22e52f32) SHA1(c67a4ccb88becc58dddcbfea0a1ac2017f7b2929) )
	ROM_LOAD16_BYTE( "epr-13565.ic29", 0x00001, 0x20000, CRC(a21784bd) SHA1(b40ba0ef65bbfe514625253f6aeec14bf4bcf08c) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "opr-13553.ic154", 0x00000, 0x10000, CRC(8c418837) SHA1(e325db39fae768865e20d2cd1ee2b91a9b0165f5) )
	ROM_LOAD( "opr-13554.ic153", 0x10000, 0x10000, CRC(4e3df9f0) SHA1(8b481c2cd25c58612ac8ac3ffb7eeae9ca247d2e) )
	ROM_LOAD( "opr-13555.ic152", 0x20000, 0x10000, CRC(6c4a1d42) SHA1(6c37b045b21173f1e2f7bd19d01c00979b8107fb) )

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "opr-13552.ic90",  0x000000, 0x20000, CRC(cc2cf706) SHA1(ad39c22e652ebcd90ffb5e17ae35985645f93c71) )
	ROM_LOAD32_BYTE( "opr-13551.ic94",  0x000001, 0x20000, CRC(d6f276c1) SHA1(9ec68157ea460e09ef4b69aa8ea17687dc47ea59) )
	ROM_LOAD32_BYTE( "opr-13550.ic98",  0x000002, 0x20000, CRC(f16518dd) SHA1(a5f1785cd28f03069cb238ac92c6afb5a26cbd37) )
	ROM_LOAD32_BYTE( "opr-13549.ic102", 0x000003, 0x20000, CRC(cba407a7) SHA1(e7684d3b40baa6d832b887fd85ad67fbad8aa7de) )
	ROM_LOAD32_BYTE( "opr-13548.ic91",  0x080000, 0x20000, CRC(080fd805) SHA1(e729565815a3a37462cfee460b7392d2f08e96e5) )
	ROM_LOAD32_BYTE( "opr-13547.ic95",  0x080001, 0x20000, CRC(42d4dd68) SHA1(6ae1f3585ebb20fd2908456d6fa41a893261277e) )
	ROM_LOAD32_BYTE( "opr-13546.ic99",  0x080002, 0x20000, CRC(ca6fbf3d) SHA1(49c3516d87f1546fa7efe785fc5c064d90b1cb8e) )
	ROM_LOAD32_BYTE( "opr-13545.ic103", 0x080003, 0x20000, CRC(c9e58dd2) SHA1(ace2e1630d8df2454183ffdbe26d8cb6d199e940) )
	ROM_LOAD32_BYTE( "opr-13544.ic92",  0x100000, 0x20000, CRC(9c1436d9) SHA1(5156e1b5c7461f6dc0d449b86b6b72153b290a4c) )
	ROM_LOAD32_BYTE( "opr-13543.ic96",  0x100001, 0x20000, CRC(2c1c8f0e) SHA1(19c9fd4272a3db18381f435ed6cd01f994c655e7) )
	ROM_LOAD32_BYTE( "opr-13542.ic100", 0x100002, 0x20000, CRC(01fd52b8) SHA1(b4ab13c7b2b2ffcfdab37d8e4855d5ef8823f1cc) )
	ROM_LOAD32_BYTE( "opr-13541.ic104", 0x100003, 0x20000, CRC(a45c547b) SHA1(d93aaa850d14a7699a1b0411e823088a9bce7553) )
	ROM_LOAD32_BYTE( "opr-13540.ic93",  0x180000, 0x20000, CRC(84b42ab0) SHA1(d24ba7fe23463fc5813ef26e0395951559d6d162) )
	ROM_LOAD32_BYTE( "opr-13539.ic97",  0x180001, 0x20000, CRC(cd6e524f) SHA1(e6df2552a84b2da95301486379c78679b0297634) )
	ROM_LOAD32_BYTE( "opr-13538.ic101", 0x180002, 0x20000, CRC(bf9a4586) SHA1(6013dee83375d72d262c8c04c2e668afea2e216c) )
	ROM_LOAD32_BYTE( "opr-13537.ic105", 0x180003, 0x20000, CRC(fa14ed3e) SHA1(d684496ade2517696a56c1423dd4686d283c133f) )

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // ground data
	ROM_LOAD( "opr-13564.ic40",  0x00000, 0x10000, CRC(e70ba138) SHA1(85eb6618f408642227056d278f10dec8dcc5a80d) )

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-13560.ic17",    0x00000, 0x10000, CRC(83050925) SHA1(118710e5789c7999bb7326df4d7bd207cbffdfd4) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "opr-13563.ic11",    0x00000, 0x20000, CRC(4083e74f) SHA1(e48c7ce0aa3406af0bbf79c169a8157693c97041) )
	ROM_LOAD( "opr-13562.ic12",    0x20000, 0x20000, CRC(3cc3968f) SHA1(d25647f6a3fa939ba30e03e7334362ef0749b23a) )
	ROM_LOAD( "opr-13561.ic13",    0x40000, 0x20000, CRC(80a7c02a) SHA1(7e8c1b9ba270d8657dbe90ed8be2e4b6463e5928) )
ROM_END

//*************************************************************************************************************************
//  AB Cop (Japan), Sega X-board
//  CPU: FD1094 (317-0169b)
//
ROM_START( abcopj )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-13557b.ic58", 0x00000, 0x20000, CRC(554e106a) SHA1(3166957ded67c82d4710bdd20eb88006e14c6a3e) )
	ROM_LOAD16_BYTE( "epr-13556b.ic63", 0x00001, 0x20000, CRC(337bf32e) SHA1(dafb9d9b3baf79ca76355278e8a14294f186790a) )
	ROM_LOAD16_BYTE( "epr-13559.ic57",  0x40000, 0x20000, CRC(4588bf19) SHA1(6a8b3d4450ac0bc41b46e6a4e1b44d82112fcd64) )
	ROM_LOAD16_BYTE( "epr-13558.ic62",  0x40001, 0x20000, CRC(11259ed4) SHA1(e7de174a0bdb1d1111e5e419f1d501ab5be1d32d) )

	ROM_REGION( 0x2000, "mainpcb:maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0169b.key", 0x0000, 0x2000, CRC(058da36e) SHA1(ab3f68a90725063c68fc5d0f8dbece1f8940dc7d) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-13566.ic20", 0x00000, 0x20000, CRC(22e52f32) SHA1(c67a4ccb88becc58dddcbfea0a1ac2017f7b2929) )
	ROM_LOAD16_BYTE( "epr-13565.ic29", 0x00001, 0x20000, CRC(a21784bd) SHA1(b40ba0ef65bbfe514625253f6aeec14bf4bcf08c) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "opr-13553.ic154", 0x00000, 0x10000, CRC(8c418837) SHA1(e325db39fae768865e20d2cd1ee2b91a9b0165f5) )
	ROM_LOAD( "opr-13554.ic153", 0x10000, 0x10000, CRC(4e3df9f0) SHA1(8b481c2cd25c58612ac8ac3ffb7eeae9ca247d2e) )
	ROM_LOAD( "opr-13555.ic152", 0x20000, 0x10000, CRC(6c4a1d42) SHA1(6c37b045b21173f1e2f7bd19d01c00979b8107fb) )

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "opr-13552.ic90",  0x000000, 0x20000, CRC(cc2cf706) SHA1(ad39c22e652ebcd90ffb5e17ae35985645f93c71) )
	ROM_LOAD32_BYTE( "opr-13551.ic94",  0x000001, 0x20000, CRC(d6f276c1) SHA1(9ec68157ea460e09ef4b69aa8ea17687dc47ea59) )
	ROM_LOAD32_BYTE( "opr-13550.ic98",  0x000002, 0x20000, CRC(f16518dd) SHA1(a5f1785cd28f03069cb238ac92c6afb5a26cbd37) )
	ROM_LOAD32_BYTE( "opr-13549.ic102", 0x000003, 0x20000, CRC(cba407a7) SHA1(e7684d3b40baa6d832b887fd85ad67fbad8aa7de) )
	ROM_LOAD32_BYTE( "opr-13548.ic91",  0x080000, 0x20000, CRC(080fd805) SHA1(e729565815a3a37462cfee460b7392d2f08e96e5) )
	ROM_LOAD32_BYTE( "opr-13547.ic95",  0x080001, 0x20000, CRC(42d4dd68) SHA1(6ae1f3585ebb20fd2908456d6fa41a893261277e) )
	ROM_LOAD32_BYTE( "opr-13546.ic99",  0x080002, 0x20000, CRC(ca6fbf3d) SHA1(49c3516d87f1546fa7efe785fc5c064d90b1cb8e) )
	ROM_LOAD32_BYTE( "opr-13545.ic103", 0x080003, 0x20000, CRC(c9e58dd2) SHA1(ace2e1630d8df2454183ffdbe26d8cb6d199e940) )
	ROM_LOAD32_BYTE( "opr-13544.ic92",  0x100000, 0x20000, CRC(9c1436d9) SHA1(5156e1b5c7461f6dc0d449b86b6b72153b290a4c) )
	ROM_LOAD32_BYTE( "opr-13543.ic96",  0x100001, 0x20000, CRC(2c1c8f0e) SHA1(19c9fd4272a3db18381f435ed6cd01f994c655e7) )
	ROM_LOAD32_BYTE( "opr-13542.ic100", 0x100002, 0x20000, CRC(01fd52b8) SHA1(b4ab13c7b2b2ffcfdab37d8e4855d5ef8823f1cc) )
	ROM_LOAD32_BYTE( "opr-13541.ic104", 0x100003, 0x20000, CRC(a45c547b) SHA1(d93aaa850d14a7699a1b0411e823088a9bce7553) )
	ROM_LOAD32_BYTE( "opr-13540.ic93",  0x180000, 0x20000, CRC(84b42ab0) SHA1(d24ba7fe23463fc5813ef26e0395951559d6d162) )
	ROM_LOAD32_BYTE( "opr-13539.ic97",  0x180001, 0x20000, CRC(cd6e524f) SHA1(e6df2552a84b2da95301486379c78679b0297634) )
	ROM_LOAD32_BYTE( "opr-13538.ic101", 0x180002, 0x20000, CRC(bf9a4586) SHA1(6013dee83375d72d262c8c04c2e668afea2e216c) )
	ROM_LOAD32_BYTE( "opr-13537.ic105", 0x180003, 0x20000, CRC(fa14ed3e) SHA1(d684496ade2517696a56c1423dd4686d283c133f) )

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // ground data
	ROM_LOAD( "opr-13564.ic40",  0x00000, 0x10000, CRC(e70ba138) SHA1(85eb6618f408642227056d278f10dec8dcc5a80d) )

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-13560.ic17",    0x00000, 0x10000, CRC(83050925) SHA1(118710e5789c7999bb7326df4d7bd207cbffdfd4) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "opr-13563.ic11",    0x00000, 0x20000, CRC(4083e74f) SHA1(e48c7ce0aa3406af0bbf79c169a8157693c97041) )
	ROM_LOAD( "opr-13562.ic12",    0x20000, 0x20000, CRC(3cc3968f) SHA1(d25647f6a3fa939ba30e03e7334362ef0749b23a) )
	ROM_LOAD( "opr-13561.ic13",    0x40000, 0x20000, CRC(80a7c02a) SHA1(7e8c1b9ba270d8657dbe90ed8be2e4b6463e5928) )
ROM_END

ROM_START( abcopjd )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "bootleg_epr-13557b.ic58", 0x00000, 0x20000, CRC(91f5d930) SHA1(8e3a4645b64b71cafa43bba3fd167dea494d7748) )
	ROM_LOAD16_BYTE( "bootleg_epr-13556b.ic63", 0x00001, 0x20000, CRC(1078246e) SHA1(3490f46c1f52d41f96fb449bdcee5fbec871aaca) )
	ROM_LOAD16_BYTE( "epr-13559.ic57",  0x40000, 0x20000, CRC(4588bf19) SHA1(6a8b3d4450ac0bc41b46e6a4e1b44d82112fcd64) )
	ROM_LOAD16_BYTE( "epr-13558.ic62",  0x40001, 0x20000, CRC(11259ed4) SHA1(e7de174a0bdb1d1111e5e419f1d501ab5be1d32d) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-13566.ic20", 0x00000, 0x20000, CRC(22e52f32) SHA1(c67a4ccb88becc58dddcbfea0a1ac2017f7b2929) )
	ROM_LOAD16_BYTE( "epr-13565.ic29", 0x00001, 0x20000, CRC(a21784bd) SHA1(b40ba0ef65bbfe514625253f6aeec14bf4bcf08c) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "opr-13553.ic154", 0x00000, 0x10000, CRC(8c418837) SHA1(e325db39fae768865e20d2cd1ee2b91a9b0165f5) )
	ROM_LOAD( "opr-13554.ic153", 0x10000, 0x10000, CRC(4e3df9f0) SHA1(8b481c2cd25c58612ac8ac3ffb7eeae9ca247d2e) )
	ROM_LOAD( "opr-13555.ic152", 0x20000, 0x10000, CRC(6c4a1d42) SHA1(6c37b045b21173f1e2f7bd19d01c00979b8107fb) )

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "opr-13552.ic90",  0x000000, 0x20000, CRC(cc2cf706) SHA1(ad39c22e652ebcd90ffb5e17ae35985645f93c71) )
	ROM_LOAD32_BYTE( "opr-13551.ic94",  0x000001, 0x20000, CRC(d6f276c1) SHA1(9ec68157ea460e09ef4b69aa8ea17687dc47ea59) )
	ROM_LOAD32_BYTE( "opr-13550.ic98",  0x000002, 0x20000, CRC(f16518dd) SHA1(a5f1785cd28f03069cb238ac92c6afb5a26cbd37) )
	ROM_LOAD32_BYTE( "opr-13549.ic102", 0x000003, 0x20000, CRC(cba407a7) SHA1(e7684d3b40baa6d832b887fd85ad67fbad8aa7de) )
	ROM_LOAD32_BYTE( "opr-13548.ic91",  0x080000, 0x20000, CRC(080fd805) SHA1(e729565815a3a37462cfee460b7392d2f08e96e5) )
	ROM_LOAD32_BYTE( "opr-13547.ic95",  0x080001, 0x20000, CRC(42d4dd68) SHA1(6ae1f3585ebb20fd2908456d6fa41a893261277e) )
	ROM_LOAD32_BYTE( "opr-13546.ic99",  0x080002, 0x20000, CRC(ca6fbf3d) SHA1(49c3516d87f1546fa7efe785fc5c064d90b1cb8e) )
	ROM_LOAD32_BYTE( "opr-13545.ic103", 0x080003, 0x20000, CRC(c9e58dd2) SHA1(ace2e1630d8df2454183ffdbe26d8cb6d199e940) )
	ROM_LOAD32_BYTE( "opr-13544.ic92",  0x100000, 0x20000, CRC(9c1436d9) SHA1(5156e1b5c7461f6dc0d449b86b6b72153b290a4c) )
	ROM_LOAD32_BYTE( "opr-13543.ic96",  0x100001, 0x20000, CRC(2c1c8f0e) SHA1(19c9fd4272a3db18381f435ed6cd01f994c655e7) )
	ROM_LOAD32_BYTE( "opr-13542.ic100", 0x100002, 0x20000, CRC(01fd52b8) SHA1(b4ab13c7b2b2ffcfdab37d8e4855d5ef8823f1cc) )
	ROM_LOAD32_BYTE( "opr-13541.ic104", 0x100003, 0x20000, CRC(a45c547b) SHA1(d93aaa850d14a7699a1b0411e823088a9bce7553) )
	ROM_LOAD32_BYTE( "opr-13540.ic93",  0x180000, 0x20000, CRC(84b42ab0) SHA1(d24ba7fe23463fc5813ef26e0395951559d6d162) )
	ROM_LOAD32_BYTE( "opr-13539.ic97",  0x180001, 0x20000, CRC(cd6e524f) SHA1(e6df2552a84b2da95301486379c78679b0297634) )
	ROM_LOAD32_BYTE( "opr-13538.ic101", 0x180002, 0x20000, CRC(bf9a4586) SHA1(6013dee83375d72d262c8c04c2e668afea2e216c) )
	ROM_LOAD32_BYTE( "opr-13537.ic105", 0x180003, 0x20000, CRC(fa14ed3e) SHA1(d684496ade2517696a56c1423dd4686d283c133f) )

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // ground data
	ROM_LOAD( "opr-13564.ic40",  0x00000, 0x10000, CRC(e70ba138) SHA1(85eb6618f408642227056d278f10dec8dcc5a80d) )

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-13560.ic17",    0x00000, 0x10000, CRC(83050925) SHA1(118710e5789c7999bb7326df4d7bd207cbffdfd4) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "opr-13563.ic11",    0x00000, 0x20000, CRC(4083e74f) SHA1(e48c7ce0aa3406af0bbf79c169a8157693c97041) )
	ROM_LOAD( "opr-13562.ic12",    0x20000, 0x20000, CRC(3cc3968f) SHA1(d25647f6a3fa939ba30e03e7334362ef0749b23a) )
	ROM_LOAD( "opr-13561.ic13",    0x40000, 0x20000, CRC(80a7c02a) SHA1(7e8c1b9ba270d8657dbe90ed8be2e4b6463e5928) )
ROM_END


//*************************************************************************************************************************
//*************************************************************************************************************************
//*************************************************************************************************************************
//  GP Rider (World), Sega X-board
//  CPU: FD1094 (317-0163)
//  Custom Chip 315-5304 (IC 127)
//  IC BD Number: 834-7626-03 (roms are "MPR") / 834-7626-05 (roms are "EPR")
//
ROM_START( gpriders )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-13409.ic58", 0x00000, 0x20000, CRC(9abb81b6) SHA1(f6308f3ec99ee66677e86f6a915e4dff8557d25f) )
	ROM_LOAD16_BYTE( "epr-13408.ic63", 0x00001, 0x20000, CRC(8e410e97) SHA1(2021d738064e57d175b59ba053d9ee35ed4516c8) )

	ROM_REGION( 0x2000, "mainpcb:maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0163.key", 0x0000, 0x2000, CRC(c1d4d207) SHA1(c35b0a49fb6a1e0e9a1c087f0ccd190ad5c2bb2c) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-13395.ic20", 0x00000, 0x20000,CRC(d6ccfac7) SHA1(9287ab08600163a0d9bd33618c629f99391316bd) )
	ROM_LOAD16_BYTE( "epr-13394.ic29", 0x00001, 0x20000,CRC(914a55ec) SHA1(84fe1df12478990418b46b6800425e5599e9eff9) )
	ROM_LOAD16_BYTE( "epr-13393.ic21", 0x40000, 0x20000,CRC(08d023cc) SHA1(d008d57e494f484a1a84896065d53fb9b1d8d60e) )
	ROM_LOAD16_BYTE( "epr-13392.ic30", 0x40001, 0x20000,CRC(f927cd42) SHA1(67eab328c1fb878fe3d086d0639f5051b135a037) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-13383.ic154", 0x00000, 0x10000, CRC(24f897a7) SHA1(68ba17067d90f07bb5a549017be4773b33ae81d0) )
	ROM_LOAD( "epr-13384.ic153", 0x10000, 0x10000, CRC(fe8238bd) SHA1(601910bd86536e6b08f5308b298c8f01fa60f233) )
	ROM_LOAD( "epr-13385.ic152", 0x20000, 0x10000, CRC(6df1b995) SHA1(5aab19b87a9ef162c30ccf5974cb795e37dba91f) )

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "epr-13382.ic90",  0x000000, 0x20000, CRC(01dac209) SHA1(4c6b03308193c472f6cdbcede306f8ce6db0cc4b) )
	ROM_LOAD32_BYTE( "epr-13381.ic94",  0x000001, 0x20000, CRC(3a50d931) SHA1(9d9cb1793f3b8f562ce0ea49f2afeef099f20859) )
	ROM_LOAD32_BYTE( "epr-13380.ic98",  0x000002, 0x20000, CRC(ad1024c8) SHA1(86e941424b2e2e00940886e5daed640a78ed7403) )
	ROM_LOAD32_BYTE( "epr-13379.ic102", 0x000003, 0x20000, CRC(1ac17625) SHA1(7aefd382041dd3f97936ecb8738a3f2c9780c58f) )
	ROM_LOAD32_BYTE( "epr-13378.ic91",  0x080000, 0x20000, CRC(50c9b867) SHA1(dd9702b369ea8abd50da22ce721b7040428e9d4b) )
	ROM_LOAD32_BYTE( "epr-13377.ic95",  0x080001, 0x20000, CRC(9b12f5c0) SHA1(2060420611b3354974c49bc80f556f945512570b) )
	ROM_LOAD32_BYTE( "epr-13376.ic99",  0x080002, 0x20000, CRC(449ac518) SHA1(0438a72e53a7889d39ea7e2530e49a2594d97e90) )
	ROM_LOAD32_BYTE( "epr-13375.ic103", 0x080003, 0x20000, CRC(5489a9ff) SHA1(c458cb55d957edae340535f54189438296f3ec2f) )
	ROM_LOAD32_BYTE( "epr-13374.ic92",  0x100000, 0x20000, CRC(6a319e4f) SHA1(d9f92b15f4baa14745048073205add35b7d42d27) )
	ROM_LOAD32_BYTE( "epr-13373.ic96",  0x100001, 0x20000, CRC(eca5588b) SHA1(11def0c293868193d457958fe7459fd8c31dbd2b) )
	ROM_LOAD32_BYTE( "epr-13372.ic100", 0x100002, 0x20000, CRC(0b45a433) SHA1(82fa2b208eaf70b70524681fbc3ec70085e70d83) )
	ROM_LOAD32_BYTE( "epr-13371.ic104", 0x100003, 0x20000, CRC(b68f4cff) SHA1(166f2a685cbc230c098fdc1646b6e632dd2b09dd) )
	ROM_LOAD32_BYTE( "epr-13370.ic93",  0x180000, 0x20000, CRC(78276620) SHA1(2c4505c57a1e765f9cfd48fb1637d67d199a2f1d) )
	ROM_LOAD32_BYTE( "epr-13369.ic97",  0x180001, 0x20000, CRC(8625bf0f) SHA1(0ae70bc0d54e25eecf4a11cf0600225dca35914d) )
	ROM_LOAD32_BYTE( "epr-13368.ic101", 0x180002, 0x20000, CRC(0f50716c) SHA1(eb4c7f47e11c58fe0d58f67e6dafabc6291eabb8) )
	ROM_LOAD32_BYTE( "epr-13367.ic105", 0x180003, 0x20000, CRC(4b1bb51f) SHA1(17fd5ac9e18dd6097a015e9d7b6815826f9c53f1) )

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none??

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-13388.ic17",    0x00000, 0x10000, CRC(706581e4) SHA1(51c9dbf2bf0d6b8826de24cd33596f5c95136870) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "epr-13391.ic11",    0x00000, 0x20000, CRC(8c30c867) SHA1(0d735291b1311890938f8a1143fae6af9feb2a69) )
	ROM_LOAD( "epr-13390.ic12",    0x20000, 0x20000, CRC(8c93cd05) SHA1(bb08094abac6c104eddf14f634e9791f03122946) )
	ROM_LOAD( "epr-13389.ic13",    0x40000, 0x20000, CRC(4e4c758e) SHA1(181750dfcdd6d5b28b063c980c251991163d9474) )
ROM_END

// Twin setup
ROM_START( gprider )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-13409.ic58", 0x00000, 0x20000, CRC(9abb81b6) SHA1(f6308f3ec99ee66677e86f6a915e4dff8557d25f) )
	ROM_LOAD16_BYTE( "epr-13408.ic63", 0x00001, 0x20000, CRC(8e410e97) SHA1(2021d738064e57d175b59ba053d9ee35ed4516c8) )

	ROM_REGION( 0x2000, "mainpcb:maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0163.key", 0x0000, 0x2000, CRC(c1d4d207) SHA1(c35b0a49fb6a1e0e9a1c087f0ccd190ad5c2bb2c) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-13395.ic20", 0x00000, 0x20000,CRC(d6ccfac7) SHA1(9287ab08600163a0d9bd33618c629f99391316bd) )
	ROM_LOAD16_BYTE( "epr-13394.ic29", 0x00001, 0x20000,CRC(914a55ec) SHA1(84fe1df12478990418b46b6800425e5599e9eff9) )
	ROM_LOAD16_BYTE( "epr-13393.ic21", 0x40000, 0x20000,CRC(08d023cc) SHA1(d008d57e494f484a1a84896065d53fb9b1d8d60e) )
	ROM_LOAD16_BYTE( "epr-13392.ic30", 0x40001, 0x20000,CRC(f927cd42) SHA1(67eab328c1fb878fe3d086d0639f5051b135a037) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-13383.ic154", 0x00000, 0x10000, CRC(24f897a7) SHA1(68ba17067d90f07bb5a549017be4773b33ae81d0) )
	ROM_LOAD( "epr-13384.ic153", 0x10000, 0x10000, CRC(fe8238bd) SHA1(601910bd86536e6b08f5308b298c8f01fa60f233) )
	ROM_LOAD( "epr-13385.ic152", 0x20000, 0x10000, CRC(6df1b995) SHA1(5aab19b87a9ef162c30ccf5974cb795e37dba91f) )

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "epr-13382.ic90",  0x000000, 0x20000, CRC(01dac209) SHA1(4c6b03308193c472f6cdbcede306f8ce6db0cc4b) )
	ROM_LOAD32_BYTE( "epr-13381.ic94",  0x000001, 0x20000, CRC(3a50d931) SHA1(9d9cb1793f3b8f562ce0ea49f2afeef099f20859) )
	ROM_LOAD32_BYTE( "epr-13380.ic98",  0x000002, 0x20000, CRC(ad1024c8) SHA1(86e941424b2e2e00940886e5daed640a78ed7403) )
	ROM_LOAD32_BYTE( "epr-13379.ic102", 0x000003, 0x20000, CRC(1ac17625) SHA1(7aefd382041dd3f97936ecb8738a3f2c9780c58f) )
	ROM_LOAD32_BYTE( "epr-13378.ic91",  0x080000, 0x20000, CRC(50c9b867) SHA1(dd9702b369ea8abd50da22ce721b7040428e9d4b) )
	ROM_LOAD32_BYTE( "epr-13377.ic95",  0x080001, 0x20000, CRC(9b12f5c0) SHA1(2060420611b3354974c49bc80f556f945512570b) )
	ROM_LOAD32_BYTE( "epr-13376.ic99",  0x080002, 0x20000, CRC(449ac518) SHA1(0438a72e53a7889d39ea7e2530e49a2594d97e90) )
	ROM_LOAD32_BYTE( "epr-13375.ic103", 0x080003, 0x20000, CRC(5489a9ff) SHA1(c458cb55d957edae340535f54189438296f3ec2f) )
	ROM_LOAD32_BYTE( "epr-13374.ic92",  0x100000, 0x20000, CRC(6a319e4f) SHA1(d9f92b15f4baa14745048073205add35b7d42d27) )
	ROM_LOAD32_BYTE( "epr-13373.ic96",  0x100001, 0x20000, CRC(eca5588b) SHA1(11def0c293868193d457958fe7459fd8c31dbd2b) )
	ROM_LOAD32_BYTE( "epr-13372.ic100", 0x100002, 0x20000, CRC(0b45a433) SHA1(82fa2b208eaf70b70524681fbc3ec70085e70d83) )
	ROM_LOAD32_BYTE( "epr-13371.ic104", 0x100003, 0x20000, CRC(b68f4cff) SHA1(166f2a685cbc230c098fdc1646b6e632dd2b09dd) )
	ROM_LOAD32_BYTE( "epr-13370.ic93",  0x180000, 0x20000, CRC(78276620) SHA1(2c4505c57a1e765f9cfd48fb1637d67d199a2f1d) )
	ROM_LOAD32_BYTE( "epr-13369.ic97",  0x180001, 0x20000, CRC(8625bf0f) SHA1(0ae70bc0d54e25eecf4a11cf0600225dca35914d) )
	ROM_LOAD32_BYTE( "epr-13368.ic101", 0x180002, 0x20000, CRC(0f50716c) SHA1(eb4c7f47e11c58fe0d58f67e6dafabc6291eabb8) )
	ROM_LOAD32_BYTE( "epr-13367.ic105", 0x180003, 0x20000, CRC(4b1bb51f) SHA1(17fd5ac9e18dd6097a015e9d7b6815826f9c53f1) )

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none??

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-13388.ic17",    0x00000, 0x10000, CRC(706581e4) SHA1(51c9dbf2bf0d6b8826de24cd33596f5c95136870) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "epr-13391.ic11",    0x00000, 0x20000, CRC(8c30c867) SHA1(0d735291b1311890938f8a1143fae6af9feb2a69) )
	ROM_LOAD( "epr-13390.ic12",    0x20000, 0x20000, CRC(8c93cd05) SHA1(bb08094abac6c104eddf14f634e9791f03122946) )
	ROM_LOAD( "epr-13389.ic13",    0x40000, 0x20000, CRC(4e4c758e) SHA1(181750dfcdd6d5b28b063c980c251991163d9474) )

	ROM_REGION( 0x80000, "subpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-13409.ic58", 0x00000, 0x20000, CRC(9abb81b6) SHA1(f6308f3ec99ee66677e86f6a915e4dff8557d25f) )
	ROM_LOAD16_BYTE( "epr-13408.ic63", 0x00001, 0x20000, CRC(8e410e97) SHA1(2021d738064e57d175b59ba053d9ee35ed4516c8) )

	ROM_REGION( 0x2000, "subpcb:maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0163.key", 0x0000, 0x2000, CRC(c1d4d207) SHA1(c35b0a49fb6a1e0e9a1c087f0ccd190ad5c2bb2c) )

	ROM_REGION( 0x80000, "subpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-13395.ic20", 0x00000, 0x20000,CRC(d6ccfac7) SHA1(9287ab08600163a0d9bd33618c629f99391316bd) )
	ROM_LOAD16_BYTE( "epr-13394.ic29", 0x00001, 0x20000,CRC(914a55ec) SHA1(84fe1df12478990418b46b6800425e5599e9eff9) )
	ROM_LOAD16_BYTE( "epr-13393.ic21", 0x40000, 0x20000,CRC(08d023cc) SHA1(d008d57e494f484a1a84896065d53fb9b1d8d60e) )
	ROM_LOAD16_BYTE( "epr-13392.ic30", 0x40001, 0x20000,CRC(f927cd42) SHA1(67eab328c1fb878fe3d086d0639f5051b135a037) )

	ROM_REGION( 0x30000, "subpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-13383.ic154", 0x00000, 0x10000, CRC(24f897a7) SHA1(68ba17067d90f07bb5a549017be4773b33ae81d0) )
	ROM_LOAD( "epr-13384.ic153", 0x10000, 0x10000, CRC(fe8238bd) SHA1(601910bd86536e6b08f5308b298c8f01fa60f233) )
	ROM_LOAD( "epr-13385.ic152", 0x20000, 0x10000, CRC(6df1b995) SHA1(5aab19b87a9ef162c30ccf5974cb795e37dba91f) )

	ROM_REGION32_LE( 0x200000, "subpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "epr-13382.ic90",  0x000000, 0x20000, CRC(01dac209) SHA1(4c6b03308193c472f6cdbcede306f8ce6db0cc4b) )
	ROM_LOAD32_BYTE( "epr-13381.ic94",  0x000001, 0x20000, CRC(3a50d931) SHA1(9d9cb1793f3b8f562ce0ea49f2afeef099f20859) )
	ROM_LOAD32_BYTE( "epr-13380.ic98",  0x000002, 0x20000, CRC(ad1024c8) SHA1(86e941424b2e2e00940886e5daed640a78ed7403) )
	ROM_LOAD32_BYTE( "epr-13379.ic102", 0x000003, 0x20000, CRC(1ac17625) SHA1(7aefd382041dd3f97936ecb8738a3f2c9780c58f) )
	ROM_LOAD32_BYTE( "epr-13378.ic91",  0x080000, 0x20000, CRC(50c9b867) SHA1(dd9702b369ea8abd50da22ce721b7040428e9d4b) )
	ROM_LOAD32_BYTE( "epr-13377.ic95",  0x080001, 0x20000, CRC(9b12f5c0) SHA1(2060420611b3354974c49bc80f556f945512570b) )
	ROM_LOAD32_BYTE( "epr-13376.ic99",  0x080002, 0x20000, CRC(449ac518) SHA1(0438a72e53a7889d39ea7e2530e49a2594d97e90) )
	ROM_LOAD32_BYTE( "epr-13375.ic103", 0x080003, 0x20000, CRC(5489a9ff) SHA1(c458cb55d957edae340535f54189438296f3ec2f) )
	ROM_LOAD32_BYTE( "epr-13374.ic92",  0x100000, 0x20000, CRC(6a319e4f) SHA1(d9f92b15f4baa14745048073205add35b7d42d27) )
	ROM_LOAD32_BYTE( "epr-13373.ic96",  0x100001, 0x20000, CRC(eca5588b) SHA1(11def0c293868193d457958fe7459fd8c31dbd2b) )
	ROM_LOAD32_BYTE( "epr-13372.ic100", 0x100002, 0x20000, CRC(0b45a433) SHA1(82fa2b208eaf70b70524681fbc3ec70085e70d83) )
	ROM_LOAD32_BYTE( "epr-13371.ic104", 0x100003, 0x20000, CRC(b68f4cff) SHA1(166f2a685cbc230c098fdc1646b6e632dd2b09dd) )
	ROM_LOAD32_BYTE( "epr-13370.ic93",  0x180000, 0x20000, CRC(78276620) SHA1(2c4505c57a1e765f9cfd48fb1637d67d199a2f1d) )
	ROM_LOAD32_BYTE( "epr-13369.ic97",  0x180001, 0x20000, CRC(8625bf0f) SHA1(0ae70bc0d54e25eecf4a11cf0600225dca35914d) )
	ROM_LOAD32_BYTE( "epr-13368.ic101", 0x180002, 0x20000, CRC(0f50716c) SHA1(eb4c7f47e11c58fe0d58f67e6dafabc6291eabb8) )
	ROM_LOAD32_BYTE( "epr-13367.ic105", 0x180003, 0x20000, CRC(4b1bb51f) SHA1(17fd5ac9e18dd6097a015e9d7b6815826f9c53f1) )

	ROM_REGION( 0x10000, "subpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none??

	ROM_REGION( 0x10000, "subpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-13388.ic17",    0x00000, 0x10000, CRC(706581e4) SHA1(51c9dbf2bf0d6b8826de24cd33596f5c95136870) )

	ROM_REGION( 0x80000, "subpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "epr-13391.ic11",    0x00000, 0x20000, CRC(8c30c867) SHA1(0d735291b1311890938f8a1143fae6af9feb2a69) )
	ROM_LOAD( "epr-13390.ic12",    0x20000, 0x20000, CRC(8c93cd05) SHA1(bb08094abac6c104eddf14f634e9791f03122946) )
	ROM_LOAD( "epr-13389.ic13",    0x40000, 0x20000, CRC(4e4c758e) SHA1(181750dfcdd6d5b28b063c980c251991163d9474) )
ROM_END


//*************************************************************************************************************************
//  GP Rider (US), Sega X-board
//  CPU: FD1094 (317-0162)
//  Custom Chip 315-5304 (IC 127)
//  IC BD Number: 834-7626-01 (roms are "MPR") / 834-7626-04 (roms are "EPR")
//
ROM_START( gpriderus )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-13407.ic58", 0x00000, 0x20000, CRC(03553ebd) SHA1(041a71a2dce2ad56360f500cb11e29a629020160) )
	ROM_LOAD16_BYTE( "epr-13406.ic63", 0x00001, 0x20000, CRC(122c711f) SHA1(2bcc51347e771a7e7f770e68b24d82497d24aa2e) )

	ROM_REGION( 0x2000, "mainpcb:maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0162.key", 0x0000, 0x2000, CRC(8067de53) SHA1(e8cd1dfbad94856c6bd51569557667e72f0a5dd4) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-13395.ic20", 0x00000, 0x20000,CRC(d6ccfac7) SHA1(9287ab08600163a0d9bd33618c629f99391316bd) )
	ROM_LOAD16_BYTE( "epr-13394.ic29", 0x00001, 0x20000,CRC(914a55ec) SHA1(84fe1df12478990418b46b6800425e5599e9eff9) )
	ROM_LOAD16_BYTE( "epr-13393.ic21", 0x40000, 0x20000,CRC(08d023cc) SHA1(d008d57e494f484a1a84896065d53fb9b1d8d60e) )
	ROM_LOAD16_BYTE( "epr-13392.ic30", 0x40001, 0x20000,CRC(f927cd42) SHA1(67eab328c1fb878fe3d086d0639f5051b135a037) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-13383.ic154", 0x00000, 0x10000, CRC(24f897a7) SHA1(68ba17067d90f07bb5a549017be4773b33ae81d0) )
	ROM_LOAD( "epr-13384.ic153", 0x10000, 0x10000, CRC(fe8238bd) SHA1(601910bd86536e6b08f5308b298c8f01fa60f233) )
	ROM_LOAD( "epr-13385.ic152", 0x20000, 0x10000, CRC(6df1b995) SHA1(5aab19b87a9ef162c30ccf5974cb795e37dba91f) )

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "epr-13382.ic90",  0x000000, 0x20000, CRC(01dac209) SHA1(4c6b03308193c472f6cdbcede306f8ce6db0cc4b) )
	ROM_LOAD32_BYTE( "epr-13381.ic94",  0x000001, 0x20000, CRC(3a50d931) SHA1(9d9cb1793f3b8f562ce0ea49f2afeef099f20859) )
	ROM_LOAD32_BYTE( "epr-13380.ic98",  0x000002, 0x20000, CRC(ad1024c8) SHA1(86e941424b2e2e00940886e5daed640a78ed7403) )
	ROM_LOAD32_BYTE( "epr-13379.ic102", 0x000003, 0x20000, CRC(1ac17625) SHA1(7aefd382041dd3f97936ecb8738a3f2c9780c58f) )
	ROM_LOAD32_BYTE( "epr-13378.ic91",  0x080000, 0x20000, CRC(50c9b867) SHA1(dd9702b369ea8abd50da22ce721b7040428e9d4b) )
	ROM_LOAD32_BYTE( "epr-13377.ic95",  0x080001, 0x20000, CRC(9b12f5c0) SHA1(2060420611b3354974c49bc80f556f945512570b) )
	ROM_LOAD32_BYTE( "epr-13376.ic99",  0x080002, 0x20000, CRC(449ac518) SHA1(0438a72e53a7889d39ea7e2530e49a2594d97e90) )
	ROM_LOAD32_BYTE( "epr-13375.ic103", 0x080003, 0x20000, CRC(5489a9ff) SHA1(c458cb55d957edae340535f54189438296f3ec2f) )
	ROM_LOAD32_BYTE( "epr-13374.ic92",  0x100000, 0x20000, CRC(6a319e4f) SHA1(d9f92b15f4baa14745048073205add35b7d42d27) )
	ROM_LOAD32_BYTE( "epr-13373.ic96",  0x100001, 0x20000, CRC(eca5588b) SHA1(11def0c293868193d457958fe7459fd8c31dbd2b) )
	ROM_LOAD32_BYTE( "epr-13372.ic100", 0x100002, 0x20000, CRC(0b45a433) SHA1(82fa2b208eaf70b70524681fbc3ec70085e70d83) )
	ROM_LOAD32_BYTE( "epr-13371.ic104", 0x100003, 0x20000, CRC(b68f4cff) SHA1(166f2a685cbc230c098fdc1646b6e632dd2b09dd) )
	ROM_LOAD32_BYTE( "epr-13370.ic93",  0x180000, 0x20000, CRC(78276620) SHA1(2c4505c57a1e765f9cfd48fb1637d67d199a2f1d) )
	ROM_LOAD32_BYTE( "epr-13369.ic97",  0x180001, 0x20000, CRC(8625bf0f) SHA1(0ae70bc0d54e25eecf4a11cf0600225dca35914d) )
	ROM_LOAD32_BYTE( "epr-13368.ic101", 0x180002, 0x20000, CRC(0f50716c) SHA1(eb4c7f47e11c58fe0d58f67e6dafabc6291eabb8) )
	ROM_LOAD32_BYTE( "epr-13367.ic105", 0x180003, 0x20000, CRC(4b1bb51f) SHA1(17fd5ac9e18dd6097a015e9d7b6815826f9c53f1) )

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none??

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-13388.ic17",    0x00000, 0x10000, CRC(706581e4) SHA1(51c9dbf2bf0d6b8826de24cd33596f5c95136870) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "epr-13391.ic11",    0x00000, 0x20000, CRC(8c30c867) SHA1(0d735291b1311890938f8a1143fae6af9feb2a69) )
	ROM_LOAD( "epr-13390.ic12",    0x20000, 0x20000, CRC(8c93cd05) SHA1(bb08094abac6c104eddf14f634e9791f03122946) )
	ROM_LOAD( "epr-13389.ic13",    0x40000, 0x20000, CRC(4e4c758e) SHA1(181750dfcdd6d5b28b063c980c251991163d9474) )
ROM_END

// twin setup
ROM_START( gprideru )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-13407.ic58", 0x00000, 0x20000, CRC(03553ebd) SHA1(041a71a2dce2ad56360f500cb11e29a629020160) )
	ROM_LOAD16_BYTE( "epr-13406.ic63", 0x00001, 0x20000, CRC(122c711f) SHA1(2bcc51347e771a7e7f770e68b24d82497d24aa2e) )

	ROM_REGION( 0x2000, "mainpcb:maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0162.key", 0x0000, 0x2000, CRC(8067de53) SHA1(e8cd1dfbad94856c6bd51569557667e72f0a5dd4) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-13395.ic20", 0x00000, 0x20000,CRC(d6ccfac7) SHA1(9287ab08600163a0d9bd33618c629f99391316bd) )
	ROM_LOAD16_BYTE( "epr-13394.ic29", 0x00001, 0x20000,CRC(914a55ec) SHA1(84fe1df12478990418b46b6800425e5599e9eff9) )
	ROM_LOAD16_BYTE( "epr-13393.ic21", 0x40000, 0x20000,CRC(08d023cc) SHA1(d008d57e494f484a1a84896065d53fb9b1d8d60e) )
	ROM_LOAD16_BYTE( "epr-13392.ic30", 0x40001, 0x20000,CRC(f927cd42) SHA1(67eab328c1fb878fe3d086d0639f5051b135a037) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-13383.ic154", 0x00000, 0x10000, CRC(24f897a7) SHA1(68ba17067d90f07bb5a549017be4773b33ae81d0) )
	ROM_LOAD( "epr-13384.ic153", 0x10000, 0x10000, CRC(fe8238bd) SHA1(601910bd86536e6b08f5308b298c8f01fa60f233) )
	ROM_LOAD( "epr-13385.ic152", 0x20000, 0x10000, CRC(6df1b995) SHA1(5aab19b87a9ef162c30ccf5974cb795e37dba91f) )

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "epr-13382.ic90",  0x000000, 0x20000, CRC(01dac209) SHA1(4c6b03308193c472f6cdbcede306f8ce6db0cc4b) )
	ROM_LOAD32_BYTE( "epr-13381.ic94",  0x000001, 0x20000, CRC(3a50d931) SHA1(9d9cb1793f3b8f562ce0ea49f2afeef099f20859) )
	ROM_LOAD32_BYTE( "epr-13380.ic98",  0x000002, 0x20000, CRC(ad1024c8) SHA1(86e941424b2e2e00940886e5daed640a78ed7403) )
	ROM_LOAD32_BYTE( "epr-13379.ic102", 0x000003, 0x20000, CRC(1ac17625) SHA1(7aefd382041dd3f97936ecb8738a3f2c9780c58f) )
	ROM_LOAD32_BYTE( "epr-13378.ic91",  0x080000, 0x20000, CRC(50c9b867) SHA1(dd9702b369ea8abd50da22ce721b7040428e9d4b) )
	ROM_LOAD32_BYTE( "epr-13377.ic95",  0x080001, 0x20000, CRC(9b12f5c0) SHA1(2060420611b3354974c49bc80f556f945512570b) )
	ROM_LOAD32_BYTE( "epr-13376.ic99",  0x080002, 0x20000, CRC(449ac518) SHA1(0438a72e53a7889d39ea7e2530e49a2594d97e90) )
	ROM_LOAD32_BYTE( "epr-13375.ic103", 0x080003, 0x20000, CRC(5489a9ff) SHA1(c458cb55d957edae340535f54189438296f3ec2f) )
	ROM_LOAD32_BYTE( "epr-13374.ic92",  0x100000, 0x20000, CRC(6a319e4f) SHA1(d9f92b15f4baa14745048073205add35b7d42d27) )
	ROM_LOAD32_BYTE( "epr-13373.ic96",  0x100001, 0x20000, CRC(eca5588b) SHA1(11def0c293868193d457958fe7459fd8c31dbd2b) )
	ROM_LOAD32_BYTE( "epr-13372.ic100", 0x100002, 0x20000, CRC(0b45a433) SHA1(82fa2b208eaf70b70524681fbc3ec70085e70d83) )
	ROM_LOAD32_BYTE( "epr-13371.ic104", 0x100003, 0x20000, CRC(b68f4cff) SHA1(166f2a685cbc230c098fdc1646b6e632dd2b09dd) )
	ROM_LOAD32_BYTE( "epr-13370.ic93",  0x180000, 0x20000, CRC(78276620) SHA1(2c4505c57a1e765f9cfd48fb1637d67d199a2f1d) )
	ROM_LOAD32_BYTE( "epr-13369.ic97",  0x180001, 0x20000, CRC(8625bf0f) SHA1(0ae70bc0d54e25eecf4a11cf0600225dca35914d) )
	ROM_LOAD32_BYTE( "epr-13368.ic101", 0x180002, 0x20000, CRC(0f50716c) SHA1(eb4c7f47e11c58fe0d58f67e6dafabc6291eabb8) )
	ROM_LOAD32_BYTE( "epr-13367.ic105", 0x180003, 0x20000, CRC(4b1bb51f) SHA1(17fd5ac9e18dd6097a015e9d7b6815826f9c53f1) )

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none??

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-13388.ic17",    0x00000, 0x10000, CRC(706581e4) SHA1(51c9dbf2bf0d6b8826de24cd33596f5c95136870) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "epr-13391.ic11",    0x00000, 0x20000, CRC(8c30c867) SHA1(0d735291b1311890938f8a1143fae6af9feb2a69) )
	ROM_LOAD( "epr-13390.ic12",    0x20000, 0x20000, CRC(8c93cd05) SHA1(bb08094abac6c104eddf14f634e9791f03122946) )
	ROM_LOAD( "epr-13389.ic13",    0x40000, 0x20000, CRC(4e4c758e) SHA1(181750dfcdd6d5b28b063c980c251991163d9474) )

	ROM_REGION( 0x80000, "subpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-13407.ic58", 0x00000, 0x20000, CRC(03553ebd) SHA1(041a71a2dce2ad56360f500cb11e29a629020160) )
	ROM_LOAD16_BYTE( "epr-13406.ic63", 0x00001, 0x20000, CRC(122c711f) SHA1(2bcc51347e771a7e7f770e68b24d82497d24aa2e) )

	ROM_REGION( 0x2000, "subpcb:maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0162.key", 0x0000, 0x2000, CRC(8067de53) SHA1(e8cd1dfbad94856c6bd51569557667e72f0a5dd4) )

	ROM_REGION( 0x80000, "subpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-13395.ic20", 0x00000, 0x20000,CRC(d6ccfac7) SHA1(9287ab08600163a0d9bd33618c629f99391316bd) )
	ROM_LOAD16_BYTE( "epr-13394.ic29", 0x00001, 0x20000,CRC(914a55ec) SHA1(84fe1df12478990418b46b6800425e5599e9eff9) )
	ROM_LOAD16_BYTE( "epr-13393.ic21", 0x40000, 0x20000,CRC(08d023cc) SHA1(d008d57e494f484a1a84896065d53fb9b1d8d60e) )
	ROM_LOAD16_BYTE( "epr-13392.ic30", 0x40001, 0x20000,CRC(f927cd42) SHA1(67eab328c1fb878fe3d086d0639f5051b135a037) )

	ROM_REGION( 0x30000, "subpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-13383.ic154", 0x00000, 0x10000, CRC(24f897a7) SHA1(68ba17067d90f07bb5a549017be4773b33ae81d0) )
	ROM_LOAD( "epr-13384.ic153", 0x10000, 0x10000, CRC(fe8238bd) SHA1(601910bd86536e6b08f5308b298c8f01fa60f233) )
	ROM_LOAD( "epr-13385.ic152", 0x20000, 0x10000, CRC(6df1b995) SHA1(5aab19b87a9ef162c30ccf5974cb795e37dba91f) )

	ROM_REGION32_LE( 0x200000, "subpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "epr-13382.ic90",  0x000000, 0x20000, CRC(01dac209) SHA1(4c6b03308193c472f6cdbcede306f8ce6db0cc4b) )
	ROM_LOAD32_BYTE( "epr-13381.ic94",  0x000001, 0x20000, CRC(3a50d931) SHA1(9d9cb1793f3b8f562ce0ea49f2afeef099f20859) )
	ROM_LOAD32_BYTE( "epr-13380.ic98",  0x000002, 0x20000, CRC(ad1024c8) SHA1(86e941424b2e2e00940886e5daed640a78ed7403) )
	ROM_LOAD32_BYTE( "epr-13379.ic102", 0x000003, 0x20000, CRC(1ac17625) SHA1(7aefd382041dd3f97936ecb8738a3f2c9780c58f) )
	ROM_LOAD32_BYTE( "epr-13378.ic91",  0x080000, 0x20000, CRC(50c9b867) SHA1(dd9702b369ea8abd50da22ce721b7040428e9d4b) )
	ROM_LOAD32_BYTE( "epr-13377.ic95",  0x080001, 0x20000, CRC(9b12f5c0) SHA1(2060420611b3354974c49bc80f556f945512570b) )
	ROM_LOAD32_BYTE( "epr-13376.ic99",  0x080002, 0x20000, CRC(449ac518) SHA1(0438a72e53a7889d39ea7e2530e49a2594d97e90) )
	ROM_LOAD32_BYTE( "epr-13375.ic103", 0x080003, 0x20000, CRC(5489a9ff) SHA1(c458cb55d957edae340535f54189438296f3ec2f) )
	ROM_LOAD32_BYTE( "epr-13374.ic92",  0x100000, 0x20000, CRC(6a319e4f) SHA1(d9f92b15f4baa14745048073205add35b7d42d27) )
	ROM_LOAD32_BYTE( "epr-13373.ic96",  0x100001, 0x20000, CRC(eca5588b) SHA1(11def0c293868193d457958fe7459fd8c31dbd2b) )
	ROM_LOAD32_BYTE( "epr-13372.ic100", 0x100002, 0x20000, CRC(0b45a433) SHA1(82fa2b208eaf70b70524681fbc3ec70085e70d83) )
	ROM_LOAD32_BYTE( "epr-13371.ic104", 0x100003, 0x20000, CRC(b68f4cff) SHA1(166f2a685cbc230c098fdc1646b6e632dd2b09dd) )
	ROM_LOAD32_BYTE( "epr-13370.ic93",  0x180000, 0x20000, CRC(78276620) SHA1(2c4505c57a1e765f9cfd48fb1637d67d199a2f1d) )
	ROM_LOAD32_BYTE( "epr-13369.ic97",  0x180001, 0x20000, CRC(8625bf0f) SHA1(0ae70bc0d54e25eecf4a11cf0600225dca35914d) )
	ROM_LOAD32_BYTE( "epr-13368.ic101", 0x180002, 0x20000, CRC(0f50716c) SHA1(eb4c7f47e11c58fe0d58f67e6dafabc6291eabb8) )
	ROM_LOAD32_BYTE( "epr-13367.ic105", 0x180003, 0x20000, CRC(4b1bb51f) SHA1(17fd5ac9e18dd6097a015e9d7b6815826f9c53f1) )

	ROM_REGION( 0x10000, "subpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none??

	ROM_REGION( 0x10000, "subpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-13388.ic17",    0x00000, 0x10000, CRC(706581e4) SHA1(51c9dbf2bf0d6b8826de24cd33596f5c95136870) )

	ROM_REGION( 0x80000, "subpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "epr-13391.ic11",    0x00000, 0x20000, CRC(8c30c867) SHA1(0d735291b1311890938f8a1143fae6af9feb2a69) )
	ROM_LOAD( "epr-13390.ic12",    0x20000, 0x20000, CRC(8c93cd05) SHA1(bb08094abac6c104eddf14f634e9791f03122946) )
	ROM_LOAD( "epr-13389.ic13",    0x40000, 0x20000, CRC(4e4c758e) SHA1(181750dfcdd6d5b28b063c980c251991163d9474) )
ROM_END

//*************************************************************************************************************************
//  GP Rider (Japan), Sega X-board
//  CPU: FD1094 (317-0161)
//  Custom Chip 315-5304 (IC 127)
//  IC BD Number: 834-7626-01 (roms are "MPR") / 834-7626-04 (roms are "EPR")
//
ROM_START( gpriderjs )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-13387.ic58", 0x00000, 0x20000, CRC(a1e8b2c5) SHA1(22b70a9074263af808bb9dffee29cbcff7e304e3) )
	ROM_LOAD16_BYTE( "epr-13386.ic63", 0x00001, 0x20000, CRC(d8be9e66) SHA1(d81c03b08fd6b971554b94e0adac131a1dcf3248) )

	ROM_REGION( 0x2000, "mainpcb:maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0161.key", 0x0000, 0x2000, CRC(e38ddc16) SHA1(d1f7f261320cbc605b4f7e5a9c28f49af5471d87) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-13395.ic20", 0x00000, 0x20000,CRC(d6ccfac7) SHA1(9287ab08600163a0d9bd33618c629f99391316bd) )
	ROM_LOAD16_BYTE( "epr-13394.ic29", 0x00001, 0x20000,CRC(914a55ec) SHA1(84fe1df12478990418b46b6800425e5599e9eff9) )
	ROM_LOAD16_BYTE( "epr-13393.ic21", 0x40000, 0x20000,CRC(08d023cc) SHA1(d008d57e494f484a1a84896065d53fb9b1d8d60e) )
	ROM_LOAD16_BYTE( "epr-13392.ic30", 0x40001, 0x20000,CRC(f927cd42) SHA1(67eab328c1fb878fe3d086d0639f5051b135a037) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-13383.ic154", 0x00000, 0x10000, CRC(24f897a7) SHA1(68ba17067d90f07bb5a549017be4773b33ae81d0) )
	ROM_LOAD( "epr-13384.ic153", 0x10000, 0x10000, CRC(fe8238bd) SHA1(601910bd86536e6b08f5308b298c8f01fa60f233) )
	ROM_LOAD( "epr-13385.ic152", 0x20000, 0x10000, CRC(6df1b995) SHA1(5aab19b87a9ef162c30ccf5974cb795e37dba91f) )

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "epr-13382.ic90",  0x000000, 0x20000, CRC(01dac209) SHA1(4c6b03308193c472f6cdbcede306f8ce6db0cc4b) )
	ROM_LOAD32_BYTE( "epr-13381.ic94",  0x000001, 0x20000, CRC(3a50d931) SHA1(9d9cb1793f3b8f562ce0ea49f2afeef099f20859) )
	ROM_LOAD32_BYTE( "epr-13380.ic98",  0x000002, 0x20000, CRC(ad1024c8) SHA1(86e941424b2e2e00940886e5daed640a78ed7403) )
	ROM_LOAD32_BYTE( "epr-13379.ic102", 0x000003, 0x20000, CRC(1ac17625) SHA1(7aefd382041dd3f97936ecb8738a3f2c9780c58f) )
	ROM_LOAD32_BYTE( "epr-13378.ic91",  0x080000, 0x20000, CRC(50c9b867) SHA1(dd9702b369ea8abd50da22ce721b7040428e9d4b) )
	ROM_LOAD32_BYTE( "epr-13377.ic95",  0x080001, 0x20000, CRC(9b12f5c0) SHA1(2060420611b3354974c49bc80f556f945512570b) )
	ROM_LOAD32_BYTE( "epr-13376.ic99",  0x080002, 0x20000, CRC(449ac518) SHA1(0438a72e53a7889d39ea7e2530e49a2594d97e90) )
	ROM_LOAD32_BYTE( "epr-13375.ic103", 0x080003, 0x20000, CRC(5489a9ff) SHA1(c458cb55d957edae340535f54189438296f3ec2f) )
	ROM_LOAD32_BYTE( "epr-13374.ic92",  0x100000, 0x20000, CRC(6a319e4f) SHA1(d9f92b15f4baa14745048073205add35b7d42d27) )
	ROM_LOAD32_BYTE( "epr-13373.ic96",  0x100001, 0x20000, CRC(eca5588b) SHA1(11def0c293868193d457958fe7459fd8c31dbd2b) )
	ROM_LOAD32_BYTE( "epr-13372.ic100", 0x100002, 0x20000, CRC(0b45a433) SHA1(82fa2b208eaf70b70524681fbc3ec70085e70d83) )
	ROM_LOAD32_BYTE( "epr-13371.ic104", 0x100003, 0x20000, CRC(b68f4cff) SHA1(166f2a685cbc230c098fdc1646b6e632dd2b09dd) )
	ROM_LOAD32_BYTE( "epr-13370.ic93",  0x180000, 0x20000, CRC(78276620) SHA1(2c4505c57a1e765f9cfd48fb1637d67d199a2f1d) )
	ROM_LOAD32_BYTE( "epr-13369.ic97",  0x180001, 0x20000, CRC(8625bf0f) SHA1(0ae70bc0d54e25eecf4a11cf0600225dca35914d) )
	ROM_LOAD32_BYTE( "epr-13368.ic101", 0x180002, 0x20000, CRC(0f50716c) SHA1(eb4c7f47e11c58fe0d58f67e6dafabc6291eabb8) )
	ROM_LOAD32_BYTE( "epr-13367.ic105", 0x180003, 0x20000, CRC(4b1bb51f) SHA1(17fd5ac9e18dd6097a015e9d7b6815826f9c53f1) )

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none??

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-13388.ic17",    0x00000, 0x10000, CRC(706581e4) SHA1(51c9dbf2bf0d6b8826de24cd33596f5c95136870) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "epr-13391.ic11",    0x00000, 0x20000, CRC(8c30c867) SHA1(0d735291b1311890938f8a1143fae6af9feb2a69) )
	ROM_LOAD( "epr-13390.ic12",    0x20000, 0x20000, CRC(8c93cd05) SHA1(bb08094abac6c104eddf14f634e9791f03122946) )
	ROM_LOAD( "epr-13389.ic13",    0x40000, 0x20000, CRC(4e4c758e) SHA1(181750dfcdd6d5b28b063c980c251991163d9474) )
ROM_END

// twin setup
ROM_START( gpriderj )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-13387.ic58", 0x00000, 0x20000, CRC(a1e8b2c5) SHA1(22b70a9074263af808bb9dffee29cbcff7e304e3) )
	ROM_LOAD16_BYTE( "epr-13386.ic63", 0x00001, 0x20000, CRC(d8be9e66) SHA1(d81c03b08fd6b971554b94e0adac131a1dcf3248) )

	ROM_REGION( 0x2000, "mainpcb:maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0161.key", 0x0000, 0x2000, CRC(e38ddc16) SHA1(d1f7f261320cbc605b4f7e5a9c28f49af5471d87) )

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-13395.ic20", 0x00000, 0x20000,CRC(d6ccfac7) SHA1(9287ab08600163a0d9bd33618c629f99391316bd) )
	ROM_LOAD16_BYTE( "epr-13394.ic29", 0x00001, 0x20000,CRC(914a55ec) SHA1(84fe1df12478990418b46b6800425e5599e9eff9) )
	ROM_LOAD16_BYTE( "epr-13393.ic21", 0x40000, 0x20000,CRC(08d023cc) SHA1(d008d57e494f484a1a84896065d53fb9b1d8d60e) )
	ROM_LOAD16_BYTE( "epr-13392.ic30", 0x40001, 0x20000,CRC(f927cd42) SHA1(67eab328c1fb878fe3d086d0639f5051b135a037) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-13383.ic154", 0x00000, 0x10000, CRC(24f897a7) SHA1(68ba17067d90f07bb5a549017be4773b33ae81d0) )
	ROM_LOAD( "epr-13384.ic153", 0x10000, 0x10000, CRC(fe8238bd) SHA1(601910bd86536e6b08f5308b298c8f01fa60f233) )
	ROM_LOAD( "epr-13385.ic152", 0x20000, 0x10000, CRC(6df1b995) SHA1(5aab19b87a9ef162c30ccf5974cb795e37dba91f) )

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "epr-13382.ic90",  0x000000, 0x20000, CRC(01dac209) SHA1(4c6b03308193c472f6cdbcede306f8ce6db0cc4b) )
	ROM_LOAD32_BYTE( "epr-13381.ic94",  0x000001, 0x20000, CRC(3a50d931) SHA1(9d9cb1793f3b8f562ce0ea49f2afeef099f20859) )
	ROM_LOAD32_BYTE( "epr-13380.ic98",  0x000002, 0x20000, CRC(ad1024c8) SHA1(86e941424b2e2e00940886e5daed640a78ed7403) )
	ROM_LOAD32_BYTE( "epr-13379.ic102", 0x000003, 0x20000, CRC(1ac17625) SHA1(7aefd382041dd3f97936ecb8738a3f2c9780c58f) )
	ROM_LOAD32_BYTE( "epr-13378.ic91",  0x080000, 0x20000, CRC(50c9b867) SHA1(dd9702b369ea8abd50da22ce721b7040428e9d4b) )
	ROM_LOAD32_BYTE( "epr-13377.ic95",  0x080001, 0x20000, CRC(9b12f5c0) SHA1(2060420611b3354974c49bc80f556f945512570b) )
	ROM_LOAD32_BYTE( "epr-13376.ic99",  0x080002, 0x20000, CRC(449ac518) SHA1(0438a72e53a7889d39ea7e2530e49a2594d97e90) )
	ROM_LOAD32_BYTE( "epr-13375.ic103", 0x080003, 0x20000, CRC(5489a9ff) SHA1(c458cb55d957edae340535f54189438296f3ec2f) )
	ROM_LOAD32_BYTE( "epr-13374.ic92",  0x100000, 0x20000, CRC(6a319e4f) SHA1(d9f92b15f4baa14745048073205add35b7d42d27) )
	ROM_LOAD32_BYTE( "epr-13373.ic96",  0x100001, 0x20000, CRC(eca5588b) SHA1(11def0c293868193d457958fe7459fd8c31dbd2b) )
	ROM_LOAD32_BYTE( "epr-13372.ic100", 0x100002, 0x20000, CRC(0b45a433) SHA1(82fa2b208eaf70b70524681fbc3ec70085e70d83) )
	ROM_LOAD32_BYTE( "epr-13371.ic104", 0x100003, 0x20000, CRC(b68f4cff) SHA1(166f2a685cbc230c098fdc1646b6e632dd2b09dd) )
	ROM_LOAD32_BYTE( "epr-13370.ic93",  0x180000, 0x20000, CRC(78276620) SHA1(2c4505c57a1e765f9cfd48fb1637d67d199a2f1d) )
	ROM_LOAD32_BYTE( "epr-13369.ic97",  0x180001, 0x20000, CRC(8625bf0f) SHA1(0ae70bc0d54e25eecf4a11cf0600225dca35914d) )
	ROM_LOAD32_BYTE( "epr-13368.ic101", 0x180002, 0x20000, CRC(0f50716c) SHA1(eb4c7f47e11c58fe0d58f67e6dafabc6291eabb8) )
	ROM_LOAD32_BYTE( "epr-13367.ic105", 0x180003, 0x20000, CRC(4b1bb51f) SHA1(17fd5ac9e18dd6097a015e9d7b6815826f9c53f1) )

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none??

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-13388.ic17",    0x00000, 0x10000, CRC(706581e4) SHA1(51c9dbf2bf0d6b8826de24cd33596f5c95136870) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "epr-13391.ic11",    0x00000, 0x20000, CRC(8c30c867) SHA1(0d735291b1311890938f8a1143fae6af9feb2a69) )
	ROM_LOAD( "epr-13390.ic12",    0x20000, 0x20000, CRC(8c93cd05) SHA1(bb08094abac6c104eddf14f634e9791f03122946) )
	ROM_LOAD( "epr-13389.ic13",    0x40000, 0x20000, CRC(4e4c758e) SHA1(181750dfcdd6d5b28b063c980c251991163d9474) )

	ROM_REGION( 0x80000, "subpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-13387.ic58", 0x00000, 0x20000, CRC(a1e8b2c5) SHA1(22b70a9074263af808bb9dffee29cbcff7e304e3) )
	ROM_LOAD16_BYTE( "epr-13386.ic63", 0x00001, 0x20000, CRC(d8be9e66) SHA1(d81c03b08fd6b971554b94e0adac131a1dcf3248) )

	ROM_REGION( 0x2000, "subpcb:maincpu:key", 0 )  // decryption key
	ROM_LOAD( "317-0161.key", 0x0000, 0x2000, CRC(e38ddc16) SHA1(d1f7f261320cbc605b4f7e5a9c28f49af5471d87) )

	ROM_REGION( 0x80000, "subpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-13395.ic20", 0x00000, 0x20000,CRC(d6ccfac7) SHA1(9287ab08600163a0d9bd33618c629f99391316bd) )
	ROM_LOAD16_BYTE( "epr-13394.ic29", 0x00001, 0x20000,CRC(914a55ec) SHA1(84fe1df12478990418b46b6800425e5599e9eff9) )
	ROM_LOAD16_BYTE( "epr-13393.ic21", 0x40000, 0x20000,CRC(08d023cc) SHA1(d008d57e494f484a1a84896065d53fb9b1d8d60e) )
	ROM_LOAD16_BYTE( "epr-13392.ic30", 0x40001, 0x20000,CRC(f927cd42) SHA1(67eab328c1fb878fe3d086d0639f5051b135a037) )

	ROM_REGION( 0x30000, "subpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-13383.ic154", 0x00000, 0x10000, CRC(24f897a7) SHA1(68ba17067d90f07bb5a549017be4773b33ae81d0) )
	ROM_LOAD( "epr-13384.ic153", 0x10000, 0x10000, CRC(fe8238bd) SHA1(601910bd86536e6b08f5308b298c8f01fa60f233) )
	ROM_LOAD( "epr-13385.ic152", 0x20000, 0x10000, CRC(6df1b995) SHA1(5aab19b87a9ef162c30ccf5974cb795e37dba91f) )

	ROM_REGION32_LE( 0x200000, "subpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "epr-13382.ic90",  0x000000, 0x20000, CRC(01dac209) SHA1(4c6b03308193c472f6cdbcede306f8ce6db0cc4b) )
	ROM_LOAD32_BYTE( "epr-13381.ic94",  0x000001, 0x20000, CRC(3a50d931) SHA1(9d9cb1793f3b8f562ce0ea49f2afeef099f20859) )
	ROM_LOAD32_BYTE( "epr-13380.ic98",  0x000002, 0x20000, CRC(ad1024c8) SHA1(86e941424b2e2e00940886e5daed640a78ed7403) )
	ROM_LOAD32_BYTE( "epr-13379.ic102", 0x000003, 0x20000, CRC(1ac17625) SHA1(7aefd382041dd3f97936ecb8738a3f2c9780c58f) )
	ROM_LOAD32_BYTE( "epr-13378.ic91",  0x080000, 0x20000, CRC(50c9b867) SHA1(dd9702b369ea8abd50da22ce721b7040428e9d4b) )
	ROM_LOAD32_BYTE( "epr-13377.ic95",  0x080001, 0x20000, CRC(9b12f5c0) SHA1(2060420611b3354974c49bc80f556f945512570b) )
	ROM_LOAD32_BYTE( "epr-13376.ic99",  0x080002, 0x20000, CRC(449ac518) SHA1(0438a72e53a7889d39ea7e2530e49a2594d97e90) )
	ROM_LOAD32_BYTE( "epr-13375.ic103", 0x080003, 0x20000, CRC(5489a9ff) SHA1(c458cb55d957edae340535f54189438296f3ec2f) )
	ROM_LOAD32_BYTE( "epr-13374.ic92",  0x100000, 0x20000, CRC(6a319e4f) SHA1(d9f92b15f4baa14745048073205add35b7d42d27) )
	ROM_LOAD32_BYTE( "epr-13373.ic96",  0x100001, 0x20000, CRC(eca5588b) SHA1(11def0c293868193d457958fe7459fd8c31dbd2b) )
	ROM_LOAD32_BYTE( "epr-13372.ic100", 0x100002, 0x20000, CRC(0b45a433) SHA1(82fa2b208eaf70b70524681fbc3ec70085e70d83) )
	ROM_LOAD32_BYTE( "epr-13371.ic104", 0x100003, 0x20000, CRC(b68f4cff) SHA1(166f2a685cbc230c098fdc1646b6e632dd2b09dd) )
	ROM_LOAD32_BYTE( "epr-13370.ic93",  0x180000, 0x20000, CRC(78276620) SHA1(2c4505c57a1e765f9cfd48fb1637d67d199a2f1d) )
	ROM_LOAD32_BYTE( "epr-13369.ic97",  0x180001, 0x20000, CRC(8625bf0f) SHA1(0ae70bc0d54e25eecf4a11cf0600225dca35914d) )
	ROM_LOAD32_BYTE( "epr-13368.ic101", 0x180002, 0x20000, CRC(0f50716c) SHA1(eb4c7f47e11c58fe0d58f67e6dafabc6291eabb8) )
	ROM_LOAD32_BYTE( "epr-13367.ic105", 0x180003, 0x20000, CRC(4b1bb51f) SHA1(17fd5ac9e18dd6097a015e9d7b6815826f9c53f1) )

	ROM_REGION( 0x10000, "subpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none??

	ROM_REGION( 0x10000, "subpcb:soundcpu", 0 ) // sound CPU
	ROM_LOAD( "epr-13388.ic17",    0x00000, 0x10000, CRC(706581e4) SHA1(51c9dbf2bf0d6b8826de24cd33596f5c95136870) )

	ROM_REGION( 0x80000, "subpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	ROM_LOAD( "epr-13391.ic11",    0x00000, 0x20000, CRC(8c30c867) SHA1(0d735291b1311890938f8a1143fae6af9feb2a69) )
	ROM_LOAD( "epr-13390.ic12",    0x20000, 0x20000, CRC(8c93cd05) SHA1(bb08094abac6c104eddf14f634e9791f03122946) )
	ROM_LOAD( "epr-13389.ic13",    0x40000, 0x20000, CRC(4e4c758e) SHA1(181750dfcdd6d5b28b063c980c251991163d9474) )
ROM_END

//*************************************************************************************************************************
//*************************************************************************************************************************
//*************************************************************************************************************************
//  Royal Ascot - should be X-Board, or closely related, although it's a main display / terminal setup,
//  and we only have the ROMs for one of those parts..
//
ROM_START( rascot )
	ROM_REGION( 0x80000, "mainpcb:maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-13965a.ic58", 0x00000, 0x20000, CRC(7eacdfb3) SHA1(fad23352d9c5e266ad9f7fe3ccbd29b5b912b90b) )
	ROM_LOAD16_BYTE( "epr-13694a.ic63", 0x00001, 0x20000, CRC(15b86498) SHA1(ccb57063ca53347b5f771b0d7ceaeb9cd50d246a) ) // 13964a?

	ROM_REGION( 0x80000, "mainpcb:subcpu", 0 ) // 2nd 68000 code
	ROM_LOAD16_BYTE( "epr-13967.ic20", 0x00000, 0x20000, CRC(3b92e2b8) SHA1(5d456d7d6fa540709facda1fd8813707ebfd99d8) )
	ROM_LOAD16_BYTE( "epr-13966.ic29", 0x00001, 0x20000, CRC(eaa644e1) SHA1(b9cc171523995f5120ea7b9748af2f8de697b933) )

	ROM_REGION( 0x30000, "mainpcb:gfx1", 0 ) // tiles
	ROM_LOAD( "epr-13961", 0x00000, 0x10000, CRC(68038629) SHA1(fbe8605840331096c5156d695772e5f36b2e131a) )
	ROM_LOAD( "epr-13962", 0x10000, 0x10000, CRC(7d7605bc) SHA1(20d3a7116807db7c831e285233d8c67317980e4a) )
	ROM_LOAD( "epr-13963", 0x20000, 0x10000, CRC(f3376b65) SHA1(36b9292518a112409d03b97ea048b7ab22734841) )

	ROM_REGION32_LE( 0x200000, "mainpcb:sprites", 0 ) // sprites
	ROM_LOAD32_BYTE( "epr-13960",  0x000000, 0x20000, CRC(b974128d) SHA1(14450615b3a10b1de6d098a282f80f80c98c34b8) )
	ROM_LOAD32_BYTE( "epr-13959",  0x000001, 0x20000, CRC(db245b22) SHA1(301b7caea7a3b42ab1ab21894ad61b8b14ef1e7c) )
	ROM_LOAD32_BYTE( "epr-13958",  0x000002, 0x20000, CRC(7803a027) SHA1(ff659da334e4440a6de9be43dde9dfa21dae5f14) )
	ROM_LOAD32_BYTE( "epr-13957",  0x000003, 0x20000, CRC(6d50fb54) SHA1(d21462c30a5555980b964930ddef4dc1963e1d8e) )

	ROM_REGION( 0x10000, "mainpcb:gfx3", ROMREGION_ERASE00 ) // road gfx
	// none??

	ROM_REGION( 0x10000, "mainpcb:soundcpu", 0 ) // sound CPU
	// is this really a sound rom, or a terminal / link rom? accesses unexpected addresses
	ROM_LOAD( "epr-14221a",    0x00000, 0x10000, CRC(0d429ac4) SHA1(9cd4c7e858874f372eb3e409ba37964f1ebf07d5) )

	ROM_REGION( 0x80000, "mainpcb:pcm", ROMREGION_ERASEFF ) // Sega PCM sound data
	// none??
ROM_END



//**************************************************************************
//  CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  init_* - game-specific initialization
//-------------------------------------------------

void segaxbd_state::install_aburner2(void)
{
	m_road_priority = 0;
	m_iochip_custom_io_r[0][0] = ioread_delegate(FUNC(segaxbd_state::aburner2_iochip0_motor_r), this);
	m_iochip_custom_io_w[0][1] = iowrite_delegate(FUNC(segaxbd_state::aburner2_iochip0_motor_w), this);
}

DRIVER_INIT_MEMBER(segaxbd_new_state,aburner2)
{
	m_mainpcb->install_aburner2();
}

void segaxbd_state::install_lastsurv(void)
{
	m_iochip_custom_io_r[1][1] = ioread_delegate(FUNC(segaxbd_state::lastsurv_iochip1_port_r), this);
	m_iochip_custom_io_w[0][3] = iowrite_delegate(FUNC(segaxbd_state::lastsurv_iochip0_muxer_w), this);
}

DRIVER_INIT_MEMBER(segaxbd_new_state,lastsurv)
{
	m_mainpcb->install_lastsurv();
}

void segaxbd_state::install_loffire(void)
{
	m_adc_reverse[1] = m_adc_reverse[3] = true;

	// install sync hack on core shared memory
	m_loffire_sync = m_maincpu->space(AS_PROGRAM).install_write_handler(0x29c000, 0x29c011, write16_delegate(FUNC(segaxbd_state::loffire_sync0_w), this));
}


DRIVER_INIT_MEMBER(segaxbd_new_state,loffire)
{
	m_mainpcb->install_loffire();
}

void segaxbd_state::install_smgp(void)
{
	m_iochip_custom_io_r[0][0] = ioread_delegate(FUNC(segaxbd_state::smgp_iochip0_motor_r), this);
	m_iochip_custom_io_w[0][1] = iowrite_delegate(FUNC(segaxbd_state::smgp_iochip0_motor_w), this);

	// map /EXCS space
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x2f0000, 0x2f3fff, read16_delegate(FUNC(segaxbd_state::smgp_excs_r), this), write16_delegate(FUNC(segaxbd_state::smgp_excs_w), this));
}

DRIVER_INIT_MEMBER(segaxbd_new_state,smgp)
{
	m_mainpcb->install_smgp();
}

DRIVER_INIT_MEMBER(segaxbd_new_state,rascot)
{
	// patch out bootup link test
	UINT16 *rom = reinterpret_cast<UINT16 *>(memregion("mainpcb:subcpu")->base());
	rom[0xb78/2] = 0x601e; // subrom checksum test
	rom[0x57e/2] = 0x4e71;
	rom[0x5d0/2] = 0x6008;
	rom[0x606/2] = 0x4e71;

	// map /EXCS space
	m_mainpcb->m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x0f0000, 0x0f3fff, read16_delegate(FUNC(segaxbd_state::rascot_excs_r), (segaxbd_state*)m_mainpcb), write16_delegate(FUNC(segaxbd_state::rascot_excs_w), (segaxbd_state*)m_mainpcb));
}

void segaxbd_state::install_gprider(void)
{
	m_gprider_hack = true;

}

DRIVER_INIT_MEMBER(segaxbd_new_state,gprider)
{
	m_mainpcb->install_gprider();
}


DRIVER_INIT_MEMBER(segaxbd_new_state_double,gprider_double)
{
	m_mainpcb->install_gprider();
	m_subpcb->install_gprider();

	m_mainpcb->m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x2F0000, 0x2F003f, read16_delegate(FUNC(segaxbd_new_state_double::shareram1_r), this), write16_delegate(FUNC(segaxbd_new_state_double::shareram1_w), this));
	m_subpcb->m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x2F0000, 0x2F003f, read16_delegate(FUNC(segaxbd_new_state_double::shareram2_r), this), write16_delegate(FUNC(segaxbd_new_state_double::shareram2_w), this));
}


//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

//    YEAR, NAME,     PARENT,   MACHINE,        INPUT,    INIT,                    MONITOR,COMPANY,FULLNAME,FLAGS
GAME( 1987, aburner2, 0,        sega_xboard,         aburner2, segaxbd_new_state, aburner2, ROT0,   "Sega", "After Burner II", 0 )
GAME( 1987, aburner2g,aburner2, sega_xboard,         aburner2, segaxbd_new_state, aburner2, ROT0,   "Sega", "After Burner II (German)", 0 )

GAME( 1987, aburner,  aburner2, sega_xboard,         aburner,  segaxbd_new_state, aburner2, ROT0,   "Sega", "After Burner", 0 )

GAME( 1987, thndrbld, 0,        sega_xboard_fd1094,  thndrbld, driver_device,     0,  ROT0,   "Sega", "Thunder Blade (upright) (FD1094 317-0056)", 0 )
GAME( 1987, thndrbld1,thndrbld, sega_xboard,         thndrbd1, driver_device,     0,  ROT0,   "Sega", "Thunder Blade (deluxe/standing) (unprotected)", 0 )

GAME( 1989, lastsurv, 0,        sega_lastsurv_fd1094,lastsurv, segaxbd_new_state, lastsurv, ROT0,   "Sega", "Last Survivor (Japan) (FD1094 317-0083)", 0 )

GAME( 1989, loffire,  0,        sega_xboard_fd1094,  loffire,  segaxbd_new_state, loffire,  ROT0,   "Sega", "Line of Fire / Bakudan Yarou (World) (FD1094 317-0136)", 0 )
GAME( 1989, loffireu, loffire,  sega_xboard_fd1094,  loffire,  segaxbd_new_state, loffire,  ROT0,   "Sega", "Line of Fire / Bakudan Yarou (US) (FD1094 317-0135)", 0 )
GAME( 1989, loffirej, loffire,  sega_xboard_fd1094,  loffire,  segaxbd_new_state, loffire,  ROT0,   "Sega", "Line of Fire / Bakudan Yarou (Japan) (FD1094 317-0134)", 0 )

GAME( 1989, rachero,  0,        sega_xboard_fd1094,  rachero,  driver_device,     0,  ROT0,   "Sega", "Racing Hero (FD1094 317-0144)", 0 )

GAME( 1989, smgp,     0,        sega_smgp_fd1094,    smgp,     segaxbd_new_state, smgp,     ROT0,   "Sega", "Super Monaco GP (World, Rev B) (FD1094 317-0126a)", 0 )
GAME( 1989, smgp6,    smgp,     sega_smgp_fd1094,    smgp,     segaxbd_new_state, smgp,     ROT0,   "Sega", "Super Monaco GP (World, Rev A) (FD1094 317-0126a)", 0 )
GAME( 1989, smgp5,    smgp,     sega_smgp_fd1094,    smgp,     segaxbd_new_state, smgp,     ROT0,   "Sega", "Super Monaco GP (World) (FD1094 317-0126)", 0 )
GAME( 1989, smgpu,    smgp,     sega_smgp_fd1094,    smgp,     segaxbd_new_state, smgp,     ROT0,   "Sega", "Super Monaco GP (US, Rev C) (FD1094 317-0125a)", 0 )
GAME( 1989, smgpu1,   smgp,     sega_smgp_fd1094,    smgp,     segaxbd_new_state, smgp,     ROT0,   "Sega", "Super Monaco GP (US, Rev B) (FD1094 317-0125a)", 0 )
GAME( 1989, smgpu2,   smgp,     sega_smgp_fd1094,    smgp,     segaxbd_new_state, smgp,     ROT0,   "Sega", "Super Monaco GP (US, Rev A) (FD1094 317-0125a)", 0 )
GAME( 1989, smgpj,    smgp,     sega_smgp_fd1094,    smgp,     segaxbd_new_state, smgp,     ROT0,   "Sega", "Super Monaco GP (Japan, Rev B) (FD1094 317-0124a)", 0 )
GAME( 1989, smgpja,   smgp,     sega_smgp_fd1094,    smgp,     segaxbd_new_state, smgp,     ROT0,   "Sega", "Super Monaco GP (Japan, Rev A) (FD1094 317-0124a)", 0 )

GAME( 1990, abcop,    0,        sega_xboard_fd1094,  abcop,    driver_device,     0,  ROT0,   "Sega", "A.B. Cop (World) (FD1094 317-0169b)", 0 )
GAME( 1990, abcopj,   abcop,    sega_xboard_fd1094,  abcop,    driver_device,     0,  ROT0,   "Sega", "A.B. Cop (Japan) (FD1094 317-0169b)", 0 )

// wasn't officially available as a single PCB setup, but runs anyway albeit with messages suggesting you can compete against a rival that doesn't exist?
GAME( 1990, gpriders, gprider,  sega_xboard_fd1094,  gprider, segaxbd_new_state, gprider,  ROT0,   "Sega", "GP Rider (World, FD1094 317-0163)", 0 )
GAME( 1990, gpriderus,gprider,  sega_xboard_fd1094,  gprider, segaxbd_new_state, gprider,  ROT0,   "Sega", "GP Rider (US, FD1094 317-0162)", 0 )
GAME( 1990, gpriderjs,gprider,  sega_xboard_fd1094,  gprider, segaxbd_new_state, gprider,  ROT0,   "Sega", "GP Rider (Japan, FD1094 317-0161)", 0 )

// multi X-Board (2 stacks directly connected, shared RAM on bridge PCB - not networked)
GAME( 1990, gprider, 0,        sega_xboard_fd1094_double, gprider_double,  segaxbd_new_state_double, gprider_double,  ROT0,   "Sega", "GP Rider (World, FD1094 317-0163) (Twin setup)", 0 )
GAME( 1990, gprideru,gprider,  sega_xboard_fd1094_double, gprider_double,  segaxbd_new_state_double, gprider_double,  ROT0,   "Sega", "GP Rider (US, FD1094 317-0162) (Twin setup)", 0 )
GAME( 1990, gpriderj,gprider,  sega_xboard_fd1094_double, gprider_double,  segaxbd_new_state_double, gprider_double,  ROT0,   "Sega", "GP Rider (Japan, FD1094 317-0161) (Twin setup)", 0 )

// X-Board + other boards?
GAME( 1991, rascot,   0,        sega_rascot,         rascot,   segaxbd_new_state, rascot,   ROT0,   "Sega", "Royal Ascot (Japan, terminal?)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

// decrypted bootlegs

GAME( 1987, thndrbldd, thndrbld,sega_xboard,  thndrbld, driver_device,     0,  ROT0,   "Sega", "Thunder Blade (upright) (bootleg of FD1094 317-0056 set)", 0 )

GAME( 1989, racherod, rachero,  sega_xboard,  rachero,  driver_device,     0,        ROT0,   "bootleg", "Racing Hero (bootleg of FD1094 317-0144 set)", 0 )

GAME( 1989, smgpd,     smgp,     sega_smgp,    smgp,     segaxbd_new_state, smgp,     ROT0,   "bootleg", "Super Monaco GP (World, Rev B) (bootleg of FD1094 317-0126a set)", 0 )
GAME( 1989, smgp6d,    smgp,     sega_smgp,    smgp,     segaxbd_new_state, smgp,     ROT0,   "bootleg", "Super Monaco GP (World, Rev A) (bootleg of FD1094 317-0126a set)", 0 )
GAME( 1989, smgp5d,    smgp,     sega_smgp,    smgp,     segaxbd_new_state, smgp,     ROT0,   "bootleg", "Super Monaco GP (World) (bootleg of FD1094 317-0126 set)", 0 )
GAME( 1989, smgpud,    smgp,     sega_smgp,    smgp,     segaxbd_new_state, smgp,     ROT0,   "bootleg", "Super Monaco GP (US, Rev C) (bootleg of FD1094 317-0125a set)", 0 )
GAME( 1989, smgpu1d,   smgp,     sega_smgp,    smgp,     segaxbd_new_state, smgp,     ROT0,   "bootleg", "Super Monaco GP (US, Rev B) (bootleg of FD1094 317-0125a set)", 0 )
GAME( 1989, smgpu2d,   smgp,     sega_smgp,    smgp,     segaxbd_new_state, smgp,     ROT0,   "bootleg", "Super Monaco GP (US, Rev A) (bootleg of FD1094 317-0125a set)", 0 )
GAME( 1989, smgpjd,    smgp,     sega_smgp,    smgp,     segaxbd_new_state, smgp,     ROT0,   "bootleg", "Super Monaco GP (Japan, Rev B) (bootleg of FD1094 317-0124a set)", 0 )

GAME( 1989, lastsurvd,lastsurv, sega_lastsurv,lastsurv, segaxbd_new_state, lastsurv, ROT0,   "bootleg", "Last Survivor (Japan) (bootleg of FD1094 317-0083 set)", 0 )

GAME( 1990, abcopd,   abcop,    sega_xboard,  abcop,    driver_device,     0,        ROT0,   "bootleg", "A.B. Cop (World) (bootleg of FD1094 317-0169b set)", 0 )
GAME( 1990, abcopjd,  abcop,    sega_xboard,  abcop,    driver_device,     0,        ROT0,   "bootleg", "A.B. Cop (Japan) (bootleg of FD1094 317-0169b set)", 0 )

GAME( 1989, loffired,  loffire,  sega_xboard,  loffire,  segaxbd_new_state, loffire,  ROT0,   "bootleg", "Line of Fire / Bakudan Yarou (World) (bootleg of FD1094 317-0136 set)", 0 )
GAME( 1989, loffireud, loffire,  sega_xboard,  loffire,  segaxbd_new_state, loffire,  ROT0,   "bootleg", "Line of Fire / Bakudan Yarou (US) (bootleg of FD1094 317-0135 set)", 0 )
GAME( 1989, loffirejd, loffire,  sega_xboard,  loffire,  segaxbd_new_state, loffire,  ROT0,   "bootleg", "Line of Fire / Bakudan Yarou (Japan) (bootleg of FD1094 317-0134 set)", 0 )
