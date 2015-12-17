// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega System 32/Multi 32 hardware

****************************************************************************

    Still to do:
        * fix protection
        * fix jpark correctly
        * priorities in multi32 appear wrong - stadium cross map screen
                                           and title fight ingame backgrounds
        * should f1lap be set up as a twin cabinet / direct link or can it
          be operated as a single screen, unlike f1en/air rescue

****************************************************************************


Slip Stream (Hispanic 950515)
Sega, 1995

This game runs on Sega System 32 hardware.

PCB Layout
----------
837-7428  171-5964E (C) SEGA 1990
|-------------------------------------------------------------------------------------------|
| TDA1518                                                               8464      8464      |
|             HM65256                                                   8464      8464      |
|  TL064      HM65256             |---------|                             |---------|       |
|   LC7881  |-----|               |SEGA     |                             |SEGA     |       |
|           |ASSP |               |315-5385 |                             |315-5388 |       |
|CND        |5C105|               |         |                             |         |       |
|           |1FD07|               |         |             |---------|     |         |       |
|           |-----|               |---------|        CNF  |SEGA     |     |---------|       |
|                                                         |315-5242 |                       |
|-|  TL062   TL064                  84256                 |         |                       |
  |                                 84256                 |         |                    CNI|
|-|                                                       |---------|      |---------|      |
|             YM3438                                                       |SEGA     |      |
|                             |-----------|                                |315-5387 |      |
|             YM3438          |NEC        |       315-5441                 |         |      |
|                             |D70616R-16 |                                |         |      |
|J              BAT_5.5V      |9105M9 V60 |                                |---------|      |
|A   BAT_3.3V                 |(C)NEC 1986|                                                 |
|M                            |           |                                  D42264         |
|M            8464            |-----------|                                  D42264         |
|A                                                                           D42264         |
|             Z80                                                            D42264         |
|                            HM53461ZP-12   HM53461ZP-12                                    |
|                            HM53461ZP-12   HM53461ZP-12                                    |
|                            HM53461ZP-12   HM53461ZP-12                                    |
|                            HM53461ZP-12   HM53461ZP-12                |-----------|       |
|-|                          HM53461ZP-12   HM53461ZP-12                | SEGA      |    CNE|
  |      |-------|           HM53461ZP-12   HM53461ZP-12                | 315-5386A |       |
|-|      |SEGA   |           HM53461ZP-12   HM53461ZP-12                |           |       |
|        |315-   |           HM53461ZP-12   HM53461ZP-12                |           |       |
|        |5296   |                                                      |           |       |
|        |-------|        3771   3771                                   |-----------|       |
|         93C46                                                              D42264         |
|CNB                         SW2                                             D42264         |
|                            SW1                                             D42264         |
|       CNA                  DSW1    32MHz  50MHz                            D42264         |
|-------------------------------------------------------------------------------------------|

Notes:
      V60 CPU running at 16.00MHz [32/2]
      Z80 CPU running at 8.000MHz [32/4]
      YM3438 running at 8.000MHz [32/4]
      CNE/F/I - Multi-pin connectors for connection of ROM Board
      CND     - 4 pin connector for 2nd Speaker for Stereo Output
      CNA     - 30 pin connector for extra controls PCB
      CNB     - 26 pin connector (purpose unknown)
      SW1     - push-button TEST switch
      SW2     - push button SERVICE switch
      DSW1    - 4-position DIP Switch
      HM53461 - Hitachi 256k Dual Port RAM
      D42264  - NEC 256k Dual Port RAM

      SEGA Custom ICs:
                      315-5441  Lattice GAL16V8A (DIP20)
                      315-5386A (QFP184)
                      315-5388  (QFP160)
                      315-5387  (QFP160)
                      315-5242  (custom ceramic DIP32 containing a small QFP IC and some capacitors/resistors etc)
                      315-5296  (QFP100)
                      ASSP 5C105 (QFP80)
                      315-5385  (QFP128)


ROM Board
---------
837-7429-01
|-------------------------------------------------------------------------------------------|
|JP10-JP27                     CNJ                    CNH                      JP1-JP9      |
|              IC6                                                                          |
| IC7                                                                                       |
|              IC13                                                                         |
| IC14                                                                                      |
|            315-5442                                                                       |
| IC22                                 IC25                IC24               IC23          |
|                                                                                           |
| IC26                                 IC29                IC28               IC27          |
|                                                                                           |
| IC31            CNK                  IC34                IC23               IC32          |
|                                                                                           |
| IC35                                 IC38                IC37               IC36          |
|             CNG       JP28-JP32                                                           |
|-------------------------------------------------------------------------------------------|

Notes:
CNG/H/J  - Multi-pin connectors (below PCB) for connection of ROM Board to Main Board
CNK      - Custom Sega connector for connection of protection daughterboard (not used)
315-5442 - MMI PAL16L8ACN (DIP20)

Jumpers: (to configure ROM sizes used on Slip Stream. Other games)
         (may vary depending on ROM types used)
JP1: 2-3           JP17: 1-2
JP2: 2-3           JP18: 2-3
JP3: 2-3           JP19: 1-2
JP4: 1-2           JP20: 2-3
JP5: 2-3           JP21: 1-2
JP6: 2-3           JP22: 2-3
JP7: 1-2           JP23: 1-2
JP8: 2-3           JP24: 1-2
JP9: 1-2           JP25: 2-3
JP10: 2-3          JP26: 1-2
JP11: 1-2          JP27: 1-2
JP12: 1-2          JP28: 2-3
JP13: 1-2          JP29: 1-2
JP14: 2-3          JP30: 2-3
JP15: 2-3          JP31: 1-2
JP16: 2-3          JP32: 1-2

ROM Locations
-------------
PCB Label  CPU P0 CPU P1    CPU D0   CPU D1   SND0     SND1     SND2 SND3 SCR0     SCR1     SCR2     SCR3     OBJ0     OBJ1     OBJ2     OBJ3     OBJ4     OBJ5     OBJ6     OBJ7
IC#        IC13   IC6       IC7      IC14     IC35     IC31     -    -    IC38     IC34     IC29     IC25     IC36     IC32     IC27     IC23     IC37     IC33     IC28     IC24
IC Type    -      27C4002   27C4000  27C4000  27C010   27C4000  -    -    27C4000  27C4000  27C4000  27C4000  27C4000  27C4000  27C4000  27C4000  27C4000  27C4000  27C4000  27C4000
ROM Label  -      S32HPRG01 S32DAT00 S32DAT01 S32SND00 S32SND01 -    -    S32SCR00 S32SCR01 S32SCR02 S32SCR03 S32OBJ00 S32OBJ01 S32OBJ02 S32OBJ03 S32OBJ04 S32OBJ05 S32OBJ06 S32OBJ07


Extra Controls PCB
------------------
837-7536
837-7536-91 (sticker)
|------------------------|
|                        |
|                 74HC74 |
|         OKI_M6253      |
|  74HC4053              |
| DAP601          74HC139|
| DAN601                 |
|CN2              CN1    |
|------------------------|
Notes:
CN2 - Multi-pin connector for controls
CN1 - Connector joining to CNA on main PCB


*********************************************************************************************


Title Fight (Export Revision)
Sega, 1992

This game runs on Sega System Multi32 hardware.

PCB Layout
----------
837-8676  171-6253C (C) SEGA 1992
834-9324-02 (STICKER)
|-------------------------------------------------------------------------------------------|
|             CNF                                     CNH                          8464     |
|8464   HM53461ZP-12 (x16)      |-----------|    HM53461ZP-12 (x16)                8464     |
|8464                           | SEGA      |                                      8464     |
|8464                           | 315-5386A |                                      8464     |
|8464                           |           |                                               |
|                               |           |                                               |
|                               |           |                                               |
|   |---------|  |---------|    |-----------|                    |---------|  |---------|   |
|   |SEGA     |  |SEGA     |                                     |SEGA     |  |SEGA     |   |
|   |315-5388 |  |315-5387 |  HM53461ZP-12 (x8)                  |315-5388 |  |315-5242 |   |
|   |         |  |         |                                     |         |  |         |   |
|   |         |  |         |                                     |         |  |         |   |
|   |---------|  |---------|                                     |---------|  |---------|   |
|             CND                                     CNE                                   |
| |---------| MB3771 MB3771                      315-5596              |-------| M5M5278P-25|
| |SEGA     |                                                          |SEGA   |            |
| |315-5242 |      8464                                                |315-   | M5M5278P-25|
| |         |BAT_3.3V                                                  |5591   |            |
| |         |BAT_5.5V                     |---------|                  |-------| M5M5278P-25|
| |---------|         Z80    50MHz        |SEGA     |     |-----------|                 DSW1|
|                            32MHz        |315-5385 |     |NEC        |          M5M5278P-25|
| |------|                                |         |     |D70632R-20 |                 SW4 |
| |SEGA  |                   |-------|    |         |     |9314X9 V70 |    40MHz        SW3 |
| |315-  |                   |SEGA   |    |---------|     |(C)NEC 1986|                     |
| |5560  | YM3438            |315-   |                    |           |                     |
| |------| LC78820           |5296   |                    |-----------|              93C45  |
|TDA1518                     |-------|                                   |-------|          |
|                                       SW1    SW2                       |SEGA   |       CNM|
|  VOL                                                                   |315-   |          |
|                                                                        |5296   |          |
| UPC844 UPC4064                                                         |-------|          |
|                                                                                           |
|CNC                                                      LED                               |
--|            JAMMA             |----|     CNJ      |----|            JAMMA             |--|
  |------------------------------|    |--------------|    |------------------------------|
Notes:
      V70 CPU running at 20.00MHz [40/2]
      Z80 CPU running at 8.000MHz [32/4]
      YM3438 running at 8.000MHz [32/4]
      315-5560 running at 8.000MHz [32/4]
      CND/E/F/H: Multi-pin connectors for connection of ROM Board
      CNC      : 4 pin connector for 2nd Speaker for Stereo Output
      CNJ      : 32 pin connector (purpose unknown)
      CNM      : 30 pin connector (purpose unknown)
      SW1      : push-button TEST switch
      SW2      : push button SERVICE switch
      SW3/4    : push button switches (purpose unknown)
      DSW1     : 4-position DIP Switch

      SEGA Custom ICs:
                      315-5596  Lattice GAL16V8A (DIP20)
                      315-5386A (QFP184)
                      315-5388  (x2, QFP160)
                      315-5387  (QFP160)
                      315-5242  (x2, custom ceramic DIP32 containing a small QFP IC and some capacitors/resistors etc)
                      315-5296  (x2, QFP100)
                      315-5560  (QFP80)
                      315-5385  (QFP128)
                      315-5591  (QFP100)


ROM Board
---------
837-8890
|-------------------------------------------------------------------------------------------|
|             CN3                                     CN4                                   |
|                                                                                           |
|             IC3                          IC14            IC15              IC10           |
| IC1   IC2                   JP1-5                                                         |
|                                                                                           |
|             IC11            JP6-10       IC22            IC23              IC18           |
|                                                                                           |
|                                                                                           |
|                                          IC36            IC37                             |
|             315-5598                                                       IC38           |
|                  CN5                                                                      |
| IC30  IC31                               IC39            IC40                             |
|                                                                CN6         IC41           |
|JP11-16      CN1                                     CN2                                   |
|-------------------------------------------------------------------------------------------|

Notes:
CN1/2/3/4 : Multi-pin connectors (below PCB) for connection of ROM Board to Main Board
CN5       : Custom Sega connector for connection of protection daughterboard (not used)
CN6       : Custom Sega connector (purpose unknown)
315-5598  : Lattice GAL16V8A (DIP20)

Jumpers: (to configure ROM sizes used on Title Fight. Other games may vary depending on ROM types used.)
JP1: 1-2
JP2: 1-2
JP3: 2-3
JP4: 2-3
JP5: 2-3
JP6: 1-2
JP7: 2-3
JP8: 2-3
JP9: 1-2
JP10: 1-2
JP11: OPEN
JP12: OPEN
JP13: OPEN
JP14: OPEN
JP15: 1-2
JP16: 2-3

ROM Locations (All known System Multi32 games shown for reference)
-------------
PCB Label     CPU P0    CPU P1    CPU D0    CPU D1    SOUND     SCR0      SCR1      OBJ0      OBJ1      OBJ2      OBJ3      OBJ4      OBJ5      OBJ6      OBJ7      PCM D0    PCM D1    PCM EP/S
IC#           IC37      IC40      IC36      IC39      IC31      IC3       IC11      IC14      IC22      IC15      IC23      IC10      IC18      IC38      IC41      IC1       IC2       IC30

IC Type       27C2048   27C2048   -         -         27C1000   8316200   8316200   8316200   8316200   8316200   8316200   8316200   8316200   8316200   8316200   8316200   -         -
Title Fight   EPR15388  EPR15389  -         -         EPR15384  MPR15381  MPR15382  MPR15379  MPR15380  MPR15375  MPR15376  MPR15371  MPR15372  MPR15373  MPR15374  MPR15385  -         -

IC Type       27C1024   27C1024   27C4200   27C4200   27C040    8316200   8316200   8316200   8316200   8316200   8316200   8316200   8316200   8316200   8316200   8316200   8316200   -
OutRunners    EPR15618  EPR15619  EPR15538  EPR15539  EPR15550  MPR15548  MPR15549  MPR15540  MPR15541  MPR15542  MPR15543  MPR15544  MPR15545  MPR15546  MPR15547  MPR15551  MPR15552  -

IC Type       27C2048   27C2048   -         -         27C1000   534200    534200    8316200   8316200   8316200   8316200   8316200   8316200   8316200   8316200   8316200   8316200   -
Hard Dunk     EPR16512  EPR16509  -         -         EPR16505  MPR16503  MPR16504  MPR16495  MPR16496  MPR16497  MPR16498  MPR16499  MPR16500  MPR16501  MPR16502  MPR16506  MPR16507  -
Hard Dunk (J) EPR16508  EPR16509  -         -         EPR16505  MPR16503  MPR16504  MPR16495  MPR16496  MPR16497  MPR16498  MPR16499  MPR16500  MPR16501  MPR16502  MPR16506  MPR16507  -

IC Type       27C2048   27C2048   27C4200   27C4200   27C1000   27C160    27C160    27C160    27C160    27C160    27C160    27C160    27C160    27C160    27C160    27C160    27C160    -
Stadium Cross EPR15093  EPR15094  EPR15018  EPR15019  EPR15192  EPR15020  EPR15021  EPR15022  EPR15023  EPR15024  EPR15025  EPR15026  EPR15027  EPR15028  EPR15029  EPR15031  EPR15032  -


System 32 Multi COMM board
--------------------------

SEGA 1992 837-8792
sticker: 837-9016-03
|--------------------------------------------------------|
|           CN3                           CN4            |
|                                                        |
|   32MHz   JP1(1-2)                                     |
|        Z80        MB89374      MB89237A    MB8421      |
|                                                        |
|  JP2(1-2)                                              |
|     EPR-15033.IC17                                     |
|  JP3(2-3)                                              |
|  JP4(1-2)                                              |
|     8464            GAL16V8                GAL16V8     |
|                     (315-5610)             (315-5506)  |
|           CN1                           CN2            |
|                      CN8  CN9                          |
|--------------------------------------------------------|
EPR-15033 - AM27C100 EPROM found on Stadium Cross
  MB89374 - Fujitsu MB89374 Data Link Controller. Clock input 8MHz [32/4]
 MB89237A - Fujitsu MB89237A DMA Controller
     8464 - Fujitsu MB8464 8k x8 SRAM
      CN8 - TX connector
      CN9 - RX connector
    CN1-4 - Joins to Main Board. Connections on top allow plugging in the ROM PCB


*********************************************************************************************


Alien 3 The Gun

The PCB number is 837-7428-03 171-5964-02B
The board matches the documented one for Slipstream in all aspects except
the Sega PCM sound chip is numbered 315-5476A (on Slipstream it's ASSP 5C105)

The EEPROM is a 93C45 at location IC76

The ROM PCB number is 'SYSTEM32 837-8393-01 16M ROM BD'
and the sticker number is '837-9878-02'

The PAL on the ROM board is a GAL16V8 at IC23 and marked '315-5552'

The correct ROM names are....

EPR-15943.IC17  - Main Program
EPR-15942.IC8   |
MPR-15855.IC18  |
MPR-15854.IC9  /

EPR-15859.IC36  - Sound Program
MPR-15858.IC35  - Sound Data
MPR-15857.IC34  |
MPR-15856.IC24 /

MPR-15863.IC14 - Tiles
MPR-15862.IC5  /

MPR-15864.IC32  - Sprites
MPR-15866.IC30  |
MPR-15868.IC28  |
MPR-15870.IC26  |
MPR-15865.IC31  |
MPR-15867.IC29  |
MPR-15869.IC27  |
MPR-15871.IC25 /

93C45_EEPROM.IC76

Alien 3 The Gun Jumpers:

JP1: 2-3
JP2: 2-3
JP3: 1-2
JP4: 1-2
JP5: 1-2
JP6: 1-2
JP7: 1-2
JP8: 1-2
JP9: 2-3
JP10: 2-3
JP11: 2-3
JP12: 1-2
JP13: 2-3
JP14: 1-2
JP15: 2-3
JP16: 1-2
JP17: 2-3
JP18: 1-2
JP19: 1-2
JP20: 2-3
JP21: 1-2
JP22: 2-3
JP23: 2-3
JP24: 1-2

The main board uses the standard I/O board 837-7536 for the gun controls.


*********************************************************************************************

Dark Edge

The PCB number is 171-5964-02B-K (i.e. manufactured in Korea)
The board matches the documented one for Slipstream in all aspects except
the Sega PCM sound chip is numbered 315-5476A (on Slipstream it's ASSP 5C105)

The EEPROM is a 93C45 at location IC76

The ROM PCB number is 'SYSTEM32 837-8393 16M ROM BD'
No game sticker is present.
The PAL on the ROM board is a GAL16V8 at IC23 and marked '315-5552'
The only difference between this board and Alien 3 The Gun is the sockets at
IC9, IC17 & IC18 are not populated. And of course there's a FD1149 security
module 317-0204.
Note the battery is easily accessible from underneath the module. The module is not
sealed and the lower cover snaps off easily.
The battery can not be changed but it is easy to solder wires to the plus/minus terminals
and then wire in a new 3V coin battery and the module will keep going for a few more years.
If the battery needs updating again do not remove any of the existing batteries otherwise
the PCB will die instantly. Just solder in a new battery to the same wires.
Always solder the battery plus to plus and minus to minus. If it's wired in plus to minus
the voltage will double to 6V.
If you have a System 32 board with an FD1149 add another new battery now!

Rom Types:
Main program ROM at IC8 is 27C240
Sound Program at IC36 is 27C100
Sound Data at IC35,34,24 are 838000
Tiles are 834200
Sprites are 8316200

Dark Edge Jumpers:

JP1: 2-3
JP2: 2-3
JP3: 1-2
JP4: 1-2
JP5: 2-3
JP6: 2-3
JP7: 1-2
JP8: 1-2
JP9: 1-2
JP10: 1-2
JP11: 2-3
JP12: 1-2
JP13: 2-3
JP14: 1-2
JP15: 2-3
JP16: 1-2
JP17: 1-2
JP18: 2-3
JP19: 1-2
JP20: 2-3
JP21: 1-2
JP22: 2-3
JP23: 2-3
JP24: 2-3


Extra Controls PCB
------------------
837-7968
|------------------------|
|CN2  PC817(x6)          |
|                        |
|   CN3          MB89255B|
|                        |
|                  74F139|
|CN4       A1603C  JP1234|
|CN5              CN1    |
|------------------------|
Notes:
CN3      - Multi-pin connector for extra controls (most likely for buttons only)
CN2/4/5  - Purpose unknown (not used on Dark Edge)
CN1      - Connector joining to CNA on main PCB
MB89255B - Fujitsu MB89255B Parallel Data I/O Interface (8-bit data bus & 3x 8-bit parallel I/O ports)
           This chip is very small and is a SSOP40 package. The chip is functionally compatible with Intel 8255A
A1603C   - NEC uPA1603C Quad Monolithic N-Channel Power MOS FET Array
JP1234   - Four 2-pin jumpers. JP3 is shorted, the others are not shorted
           JP1/2/3/4 are tied to the 74F139 pins 4,5,6,7 respectively. The 74F139 is tied to CN1 and the MB89255B
           Pin 1 of the jumpers is common to all and is tied to pin 6 of the MB89255B (Chip Select)


*********************************************************************************************


On "Super Visual Football: European Sega Cup" and "JLEAGUE" :

JLEAGUE was the original code developed and released in early
1994 in Japan which than got translated and updated to be
released as the European Sega Cup.

So, JLEAGUE is the Original.
and
"Super Visual Football: European Sega Cup" is the clone.

My source of information?
I wrote the simulation ( in assembly ) while working in the SEGA
Haneda ( Tokyo ) office in 1993. We even filed a patent on the
team/individual player task synchronization. After I finished JLEAGE,
we handed it off to the satellite groups for localization.

Cheers,

MIB.42


***************************************************************************

Output Notes:
All outputs are hooked up properly with the following exceptions:
radm:  Motors aren't hooked up, as the board isn't emulated. Also the 2nd and 3rd lamps "die" when the cabinet dip is set to deluxe.
    They probably get moved over to the motor driver board.

radr:  See radm

kokoroj: This driver isn't finished enough to flesh out the outputs, but a space has been reserved in the output functions.

jpark:  Since the piston driver board isn't fully emulated, they aren't hooked up.  offset 0c of the common chip function seems to have something to do with it.

orunners:  Interleaved with the dj and << >> buttons is the data the drives the lcd display.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/v60/v60.h"
#include "cpu/nec/v25.h"
#include "rendlay.h"
#include "includes/segas32.h"
#include "machine/eepromser.h"
#include "sound/2612intf.h"
#include "sound/rf5c68.h"

#include "radr.lh"

const device_type SEGA_S32_PCB = &device_creator<segas32_state>;

segas32_state::segas32_state(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, SEGA_S32_PCB, "Sega System 32 PCB", tag, owner, clock, "segas32_pcb", __FILE__),
		m_z80_shared_ram(*this,"z80_shared_ram"),
		m_ga2_dpram(*this,"ga2_dpram"),
		m_system32_workram(*this,"workram"),
		m_system32_videoram(*this,"videoram", 0),
		m_system32_spriteram(*this,"spriteram", 0),
		m_system32_paletteram(*this,"paletteram", 0) ,
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_multipcm(*this, "sega"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_irq_timer_0(*this, "v60_irq0"),
		m_irq_timer_1(*this, "v60_irq1"),
		m_s32comm(*this, "s32comm")
{
}


/*************************************
 *
 *  Constants
 *
 *************************************/

#define MASTER_CLOCK        32215900
#define RFC_CLOCK           XTAL_50MHz
#define MULTI32_CLOCK       XTAL_40MHz

#define TIMER_0_CLOCK       ((MASTER_CLOCK/2)/2048) /* confirmed */
#define TIMER_1_CLOCK       ((RFC_CLOCK/16)/256)    /* confirmed */

#define MAIN_IRQ_VBSTART    0
#define MAIN_IRQ_VBSTOP     1
#define MAIN_IRQ_SOUND      2
#define MAIN_IRQ_TIMER0     3
#define MAIN_IRQ_TIMER1     4

#define SOUND_IRQ_YM3438    0
#define SOUND_IRQ_V60       1



/*************************************
 *
 *  Machine init
 *
 *************************************/

void segas32_state::device_start()
{
	common_start(0);
}

void segas32_v25_state::device_start()
{
	common_start(0);
}

void sega_multi32_state::device_start()
{
	common_start(1);
}

void segas32_state::device_reset()
{
	/* initialize the interrupt controller */
	memset(m_v60_irq_control, 0xff, sizeof(m_v60_irq_control));

	/* allocate timers */
	m_v60_irq_timer[0] = m_irq_timer_0;
	m_v60_irq_timer[1] = m_irq_timer_1;

	/* clear IRQ lines */
	m_maincpu->set_input_line(0, CLEAR_LINE);
}


/*************************************
 *
 *  Interrupt controller
 *
 *************************************/

void segas32_state::update_irq_state()
{
	UINT8 effirq = m_v60_irq_control[7] & ~m_v60_irq_control[6] & 0x1f;
	int vector;

	/* loop over interrupt vectors, finding the highest priority one with */
	/* an unmasked interrupt pending */
	for (vector = 0; vector < 5; vector++)
		if (effirq & (1 << vector))
		{
			m_maincpu->set_input_line_and_vector(0, ASSERT_LINE, vector);
			break;
		}

	/* if we didn't find any, clear the interrupt line */
	if (vector == 5)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}


void segas32_state::signal_v60_irq(int which)
{
	int i;

	/* see if this interrupt input is mapped to any vectors; if so, mark them */
	for (i = 0; i < 5; i++)
		if (m_v60_irq_control[i] == which)
			m_v60_irq_control[7] |= 1 << i;
	update_irq_state();
}


TIMER_DEVICE_CALLBACK_MEMBER(segas32_state::signal_v60_irq_callback)
{
	signal_v60_irq(param);
}


void segas32_state::int_control_w(address_space &space, int offset, UINT8 data)
{
	int duration;

//  logerror("%06X:int_control_w(%X) = %02X\n", space.device().safe_pc(), offset, data);
	switch (offset)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:         /* vectors */
			m_v60_irq_control[offset] = data;
			break;

		case 5:         /* unknown */
			m_v60_irq_control[offset] = data;
			break;

		case 6:         /* mask */
			m_v60_irq_control[offset] = data;
			update_irq_state();
			break;

		case 7:         /* acknowledge */
			m_v60_irq_control[offset] &= data;
			update_irq_state();
			break;

		case 8:
		case 9:         /* timer 0 count */
			m_v60_irq_control[offset] = data;
			duration = m_v60_irq_control[8] + ((m_v60_irq_control[9] << 8) & 0xf00);
			if (duration)
			{
				attotime period = attotime::from_hz(TIMER_0_CLOCK) * duration;
				m_v60_irq_timer[0]->adjust(period, MAIN_IRQ_TIMER0);
			}
			break;

		case 10:
		case 11:        /* timer 1 count */
			m_v60_irq_control[offset] = data;
			duration = m_v60_irq_control[10] + ((m_v60_irq_control[11] << 8) & 0xf00);
			if (duration)
			{
				attotime period = attotime::from_hz(TIMER_1_CLOCK) * duration;
				m_v60_irq_timer[1]->adjust(period, MAIN_IRQ_TIMER1);
			}
			break;

		case 12:
		case 13:
		case 14:
		case 15:        /* signal IRQ to sound CPU */
			signal_sound_irq(SOUND_IRQ_V60);
			break;
	}
}


READ16_MEMBER(segas32_state::interrupt_control_16_r)
{
	switch (offset)
	{
		case 8/2:
			/* fix me - should return timer count down value */
			break;

		case 10/2:
			/* fix me - should return timer count down value */
			break;
	}

	/* return all F's for everything except timer values */
	return 0xffff;
}


WRITE16_MEMBER(segas32_state::interrupt_control_16_w)
{
	if (ACCESSING_BITS_0_7)
		int_control_w(space, offset*2+0, data);
	if (ACCESSING_BITS_8_15)
		int_control_w(space, offset*2+1, data >> 8);
}


READ32_MEMBER(segas32_state::interrupt_control_32_r)
{
	switch (offset)
	{
		case 8/4:
			/* fix me - should return timer count down value */
			break;
	}

	/* return all F's for everything except timer values */
	return 0xffffffff;
}


WRITE32_MEMBER(segas32_state::interrupt_control_32_w)
{
	if (ACCESSING_BITS_0_7)
		int_control_w(space, offset*4+0, data);
	if (ACCESSING_BITS_8_15)
		int_control_w(space, offset*4+1, data >> 8);
	if (ACCESSING_BITS_16_23)
		int_control_w(space, offset*4+2, data >> 16);
	if (ACCESSING_BITS_24_31)
		int_control_w(space, offset*4+3, data >> 24);
}


TIMER_CALLBACK_MEMBER(segas32_state::end_of_vblank_int)
{
	signal_v60_irq(MAIN_IRQ_VBSTOP);
	system32_set_vblank(0);
}


INTERRUPT_GEN_MEMBER(segas32_state::start_of_vblank_int)
{
	signal_v60_irq(MAIN_IRQ_VBSTART);
	system32_set_vblank(1);
	machine().scheduler().timer_set(m_screen->time_until_pos(0), timer_expired_delegate(FUNC(segas32_state::end_of_vblank_int),this));
	if (m_system32_prot_vblank)
		(this->*m_system32_prot_vblank)();
	if (m_s32comm != nullptr)
		m_s32comm->check_vint_irq();
}



/*************************************
 *
 *  I/O chip
 *
 *************************************/

UINT16 segas32_state::common_io_chip_r(address_space &space, int which, offs_t offset, UINT16 mem_mask)
{
	static const char *const portnames[2][8] =
			{
				{ "P1_A", "P2_A", "PORTC_A", "PORTD_A", "SERVICE12_A", "SERVICE34_A", "PORTG_A", "PORTH_A" },
				{ "P1_B", "P2_B", "PORTC_B", "PORTD_B", "SERVICE12_B", "SERVICE34_B", "PORTG_B", "PORTH_B" },
			};
	offset &= 0x1f/2;

	switch (offset)
	{
		/* I/O ports */
		case 0x00/2:
		case 0x02/2:
		case 0x04/2:
		case 0x06/2:
		case 0x08/2:
		case 0x0a/2:
		case 0x0c/2:
		case 0x0e/2:
			/* if the port is configured as an output, return the last thing written */
			if (m_misc_io_data[which][0x1e/2] & (1 << offset))
				return m_misc_io_data[which][offset];

			/* otherwise, return an input port */
			return read_safe(ioport(portnames[which][offset]), 0xffff);

		/* 'SEGA' protection */
		case 0x10/2:
			return 'S';
		case 0x12/2:
			return 'E';
		case 0x14/2:
			return 'G';
		case 0x16/2:
			return 'A';

		/* CNT register & mirror */
		case 0x18/2:
		case 0x1c/2:
			return m_misc_io_data[which][0x1c/2];

		/* port direction register & mirror */
		case 0x1a/2:
		case 0x1e/2:
			return m_misc_io_data[which][0x1e/2];
	}
	return 0xffff;
}


void segas32_state::common_io_chip_w(address_space &space, int which, offs_t offset, UINT16 data, UINT16 mem_mask)
{
//  UINT8 old;

	/* only LSB matters */
	if (!ACCESSING_BITS_0_7)
		return;

	/* generic implementation */
	offset &= 0x1f/2;
//  old = m_misc_io_data[which][offset];
	m_misc_io_data[which][offset] = data;

	switch (offset)
	{
		/* I/O ports */
		case 0x00/2:
		case 0x02/2:
		case 0x04/2:
		case 0x08/2:
		case 0x0a/2:
		case 0x0c/2:
			if (m_sw2_output)
				(this->*m_sw2_output)(which, data);
			break;

		/* miscellaneous output */
		case 0x06/2:
			if (m_sw1_output)
				(this->*m_sw1_output)(which, data);

			if (which == 0)
			{
				m_eeprom->di_write((data & 0x80) >> 7);
				m_eeprom->cs_write((data & 0x20) ? ASSERT_LINE : CLEAR_LINE);
				m_eeprom->clk_write((data & 0x40) ? ASSERT_LINE : CLEAR_LINE);
			}
/*            coin_lockout_w(machine(), 1 + 2*which, data & 0x08);
            coin_lockout_w(machine(), 0 + 2*which, data & 0x04);*/
			coin_counter_w(machine(), 1 + 2*which, data & 0x02);
			coin_counter_w(machine(), 0 + 2*which, data & 0x01);
			break;

		/* tile banking */
		case 0x0e/2:
			if (which == 0)
				m_system32_tilebank_external = data;
			else
			{
				/* multi-32 EEPROM access */
				m_eeprom->di_write((data & 0x80) >> 7);
				m_eeprom->cs_write((data & 0x20) ? ASSERT_LINE : CLEAR_LINE);
				m_eeprom->clk_write((data & 0x40) ? ASSERT_LINE : CLEAR_LINE);
			}
			break;

		/* CNT register */
		case 0x1c/2:
			m_system32_displayenable[which] = (data & 0x02);
			if (which == 0)
				m_soundcpu->set_input_line(INPUT_LINE_RESET, (data & 0x04) ? CLEAR_LINE : ASSERT_LINE);
			break;
	}
}


READ16_MEMBER(segas32_state::io_chip_r)
{
	return common_io_chip_r(space, 0, offset, mem_mask);
}


WRITE16_MEMBER(segas32_state::io_chip_w)
{
	common_io_chip_w(space, 0, offset, data, mem_mask);
}


READ32_MEMBER(segas32_state::io_chip_0_r)
{
	return common_io_chip_r(space, 0, offset*2+0, mem_mask) |
			(common_io_chip_r(space, 0, offset*2+1, mem_mask >> 16) << 16);
}


WRITE32_MEMBER(segas32_state::io_chip_0_w)
{
	if (ACCESSING_BITS_0_15)
		common_io_chip_w(space, 0, offset*2+0, data, mem_mask);
	if (ACCESSING_BITS_16_31)
		common_io_chip_w(space, 0, offset*2+1, data >> 16, mem_mask >> 16);
}


READ32_MEMBER(segas32_state::io_chip_1_r)
{
	return common_io_chip_r(space, 1, offset*2+0, mem_mask) |
			(common_io_chip_r(space, 1, offset*2+1, mem_mask >> 16) << 16);
}


WRITE32_MEMBER(segas32_state::io_chip_1_w)
{
	if (ACCESSING_BITS_0_15)
		common_io_chip_w(space, 1, offset*2+0, data, mem_mask);
	if (ACCESSING_BITS_16_31)
		common_io_chip_w(space, 1, offset*2+1, data >> 16, mem_mask >> 16);
}



/*************************************
 *
 *  I/O expansion range
 *
 *************************************/

READ16_MEMBER(segas32_state::io_expansion_r)
{
	if (!m_custom_io_r[0].isnull())
		return (m_custom_io_r[0])(space, offset, mem_mask);
	else
		logerror("%06X:io_expansion_r(%X)\n", space.device().safe_pc(), offset);
	return 0xffff;
}


WRITE16_MEMBER(segas32_state::io_expansion_w)
{
	/* only LSB matters */
	if (!ACCESSING_BITS_0_7)
	return;

	if (!m_custom_io_w[0].isnull())
		(m_custom_io_w[0])(space, offset, data, mem_mask);
	else
		logerror("%06X:io_expansion_w(%X) = %02X\n", space.device().safe_pc(), offset, data & 0xff);
}


READ32_MEMBER(segas32_state::io_expansion_0_r)
{
	if (!m_custom_io_r[0].isnull())
		return (m_custom_io_r[0])(space, offset*2+0, mem_mask) |
				((m_custom_io_r[0])(space, offset*2+1, mem_mask >> 16) << 16);
	else
		logerror("%06X:io_expansion_r(%X)\n", space.device().safe_pc(), offset);
	return 0xffffffff;
}


WRITE32_MEMBER(segas32_state::io_expansion_0_w)
{
	/* only LSB matters */


	if (ACCESSING_BITS_0_7)
	{
		/* harddunk uses bits 4,5 for output lamps */
		if (m_sw3_output)
			(this->*m_sw3_output)(0, data & 0xff);

		if (!m_custom_io_w[0].isnull())
			(m_custom_io_w[0])(space, offset*2+0, data, mem_mask);
		else
			logerror("%06X:io_expansion_w(%X) = %02X\n", space.device().safe_pc(), offset, data & 0xff);

	}
	if (ACCESSING_BITS_16_23)
	{
		if (!m_custom_io_w[0].isnull())
			(m_custom_io_w[0])(space, offset*2+1, data >> 16, mem_mask >> 16);
		else
			logerror("%06X:io_expansion_w(%X) = %02X\n", space.device().safe_pc(), offset, data & 0xff);
	}
}


READ32_MEMBER(segas32_state::io_expansion_1_r)
{
	if (!m_custom_io_r[1].isnull())
		return (m_custom_io_r[1])(space, offset*2+0, mem_mask) |
				((m_custom_io_r[1])(space, offset*2+1, mem_mask >> 16) << 16);
	else
		logerror("%06X:io_expansion_r(%X)\n", space.device().safe_pc(), offset);
	return 0xffffffff;
}


WRITE32_MEMBER(segas32_state::io_expansion_1_w)
{
	/* only LSB matters */
	if (ACCESSING_BITS_0_7)
	{
		if (!m_custom_io_w[1].isnull())
			(m_custom_io_w[1])(space, offset*2+0, data, mem_mask);
		else
			logerror("%06X:io_expansion_w(%X) = %02X\n", space.device().safe_pc(), offset, data & 0xff);
	}
	if (ACCESSING_BITS_16_23)
	{
		if (!m_custom_io_w[1].isnull())
			(m_custom_io_w[1])(space, offset*2+1, data >> 16, mem_mask >> 16);
		else
			logerror("%06X:io_expansion_w(%X) = %02X\n", space.device().safe_pc(), offset, data & 0xff);
	}
}



/*************************************
 *
 *  Game-specific custom I/O
 *
 *************************************/

READ16_MEMBER(segas32_state::analog_custom_io_r)
{
	UINT16 result;
	switch (offset)
	{
		case 0x10/2:
		case 0x12/2:
		case 0x14/2:
		case 0x16/2:
			result = m_analog_value[offset & 3] | 0x7f;
			m_analog_value[offset & 3] <<= 1;
			return result;
	}
	logerror("%06X:unknown analog_custom_io_r(%X) & %04X\n", space.device().safe_pc(), offset*2, mem_mask);
	return 0xffff;
}


WRITE16_MEMBER(segas32_state::analog_custom_io_w)
{
	static const char *const names[] = { "ANALOG1", "ANALOG2", "ANALOG3", "ANALOG4" };
	switch (offset)
	{
		case 0x10/2:
		case 0x12/2:
		case 0x14/2:
		case 0x16/2:
			m_analog_value[offset & 3] = read_safe(ioport(names[offset & 3]), 0);
			return;
	}
	logerror("%06X:unknown analog_custom_io_w(%X) = %04X & %04X\n", space.device().safe_pc(), offset*2, data, mem_mask);
}


READ16_MEMBER(segas32_state::extra_custom_io_r)
{
	static const char *const names[] = { "EXTRA1", "EXTRA2", "EXTRA3", "EXTRA4" };
	switch (offset)
	{
		case 0x20/2:
		case 0x22/2:
		case 0x24/2:
		case 0x26/2:
			return read_safe(ioport(names[offset & 3]), 0xffff);
	}

	logerror("%06X:unknown extra_custom_io_r(%X) & %04X\n", space.device().safe_pc(), offset*2, mem_mask);
	return 0xffff;
}


WRITE16_MEMBER(segas32_state::orunners_custom_io_w)
{
	static const char *const names[] = { "ANALOG1", "ANALOG2", "ANALOG3", "ANALOG4", "ANALOG5", "ANALOG6", "ANALOG7", "ANALOG8" };
	switch (offset)
	{
		case 0x10/2:
		case 0x12/2:
		case 0x14/2:
		case 0x16/2:
			m_analog_value[offset & 3] = read_safe(ioport(names[m_analog_bank * 4 + (offset & 3)]), 0);
			return;

		case 0x20/2:
			m_analog_bank = data & 1;
			return;
	}
	logerror("%06X:unknown orunners_custom_io_w(%X) = %04X & %04X\n", space.device().safe_pc(), offset*2, data, mem_mask);
}


READ16_MEMBER(segas32_state::sonic_custom_io_r)
{
	static const char *const names[] = { "TRACKX1", "TRACKY1", "TRACKX2", "TRACKY2", "TRACKX3", "TRACKY3" };

	switch (offset)
	{
		case 0x00/2:
		case 0x04/2:
		case 0x08/2:
		case 0x0c/2:
		case 0x10/2:
		case 0x14/2:
			return (UINT8)(ioport(names[offset/2])->read() - m_sonic_last[offset/2]);
	}

	logerror("%06X:unknown sonic_custom_io_r(%X) & %04X\n", space.device().safe_pc(), offset*2, mem_mask);
	return 0xffff;
}


WRITE16_MEMBER(segas32_state::sonic_custom_io_w)
{
	static const char *const names[] = { "TRACKX1", "TRACKY1", "TRACKX2", "TRACKY2", "TRACKX3", "TRACKY3" };

	switch (offset)
	{
		case 0x00/2:
		case 0x08/2:
		case 0x10/2:
			m_sonic_last[offset/2 + 0] = ioport(names[offset/2 + 0])->read();
			m_sonic_last[offset/2 + 1] = ioport(names[offset/2 + 1])->read();
			return;
	}

	logerror("%06X:unknown sonic_custom_io_w(%X) = %04X & %04X\n", space.device().safe_pc(), offset*2, data, mem_mask);
}



/*************************************
 *
 *  Random number generator
 *
 *************************************/

WRITE16_MEMBER(segas32_state::random_number_16_w)
{
//  osd_printf_debug("%06X:random_seed_w(%04X) = %04X & %04X\n", space.device().safe_pc(), offset*2, data, mem_mask);
}

READ16_MEMBER(segas32_state::random_number_16_r)
{
	return machine().rand();
}

WRITE32_MEMBER(segas32_state::random_number_32_w)
{
//  osd_printf_debug("%06X:random_seed_w(%04X) = %04X & %04X\n", space.device().safe_pc(), offset*2, data, mem_mask);
}

READ32_MEMBER(segas32_state::random_number_32_r)
{
	return machine().rand() ^ (machine().rand() << 16);
}



/*************************************
 *
 *  Sound communications
 *
 *************************************/

READ16_MEMBER(segas32_state::shared_ram_16_r)
{
	return m_z80_shared_ram[offset*2+0] | (m_z80_shared_ram[offset*2+1] << 8);
}


WRITE16_MEMBER(segas32_state::shared_ram_16_w)
{
	if (ACCESSING_BITS_0_7)
		m_z80_shared_ram[offset*2+0] = data;
	if (ACCESSING_BITS_8_15)
		m_z80_shared_ram[offset*2+1] = data >> 8;
}


READ32_MEMBER(segas32_state::shared_ram_32_r)
{
	return m_z80_shared_ram[offset*4+0] | (m_z80_shared_ram[offset*4+1] << 8) |
			(m_z80_shared_ram[offset*4+2] << 16) | (m_z80_shared_ram[offset*4+3] << 24);
}


WRITE32_MEMBER(segas32_state::shared_ram_32_w)
{
	if (ACCESSING_BITS_0_7)
		m_z80_shared_ram[offset*4+0] = data;
	if (ACCESSING_BITS_8_15)
		m_z80_shared_ram[offset*4+1] = data >> 8;
	if (ACCESSING_BITS_16_23)
		m_z80_shared_ram[offset*4+2] = data >> 16;
	if (ACCESSING_BITS_24_31)
		m_z80_shared_ram[offset*4+3] = data >> 24;
}



/*************************************
 *
 *  Sound interrupt controller
 *
 *************************************/

void segas32_state::update_sound_irq_state()
{
	UINT8 effirq = m_sound_irq_input & ~m_sound_irq_control[3] & 0x07;
	int vector;

	/* loop over interrupt vectors, finding the highest priority one with */
	/* an unmasked interrupt pending */
	for (vector = 0; vector < 3; vector++)
		if (effirq & (1 << vector))
		{
			m_soundcpu->set_input_line_and_vector(0, ASSERT_LINE, 2 * vector);
			break;
		}

	/* if we didn't find any, clear the interrupt line */
	if (vector == 3)
		m_soundcpu->set_input_line(0, CLEAR_LINE);
}


void segas32_state::signal_sound_irq(int which)
{
	/* see if this interrupt input is mapped to any vectors; if so, mark them */
	for (int i = 0; i < 3; i++)
		if (m_sound_irq_control[i] == which)
			m_sound_irq_input |= 1 << i;
	update_sound_irq_state();
}


void segas32_state::clear_sound_irq(int which)
{
	for (int i = 0; i < 3; i++)
		if (m_sound_irq_control[i] == which)
			m_sound_irq_input &= ~(1 << i);
	update_sound_irq_state();
}


WRITE8_MEMBER(segas32_state::sound_int_control_lo_w)
{
	/* odd offsets are interrupt acks */
	if (offset & 1)
	{
		m_sound_irq_input &= data;
		update_sound_irq_state();
	}

	/* high offsets signal an IRQ to the v60 */
	if (offset & 4)
		signal_v60_irq(MAIN_IRQ_SOUND);
}


WRITE8_MEMBER(segas32_state::sound_int_control_hi_w)
{
	m_sound_irq_control[offset] = data;
	update_sound_irq_state();
}


WRITE_LINE_MEMBER(segas32_state::ym3438_irq_handler)
{
	if (state)
		signal_sound_irq(SOUND_IRQ_YM3438);
	else
		clear_sound_irq(SOUND_IRQ_YM3438);
}



/*************************************
 *
 *  Sound banking
 *
 *************************************/

WRITE8_MEMBER(segas32_state::sound_bank_lo_w)
{
	m_sound_bank = (m_sound_bank & ~0x3f) | (data & 0x3f);
	membank("bank1")->set_base(memregion("soundcpu")->base() + 0x100000 + 0x2000 * m_sound_bank);
}


WRITE8_MEMBER(segas32_state::sound_bank_hi_w)
{
	m_sound_bank = (m_sound_bank & 0x3f) | ((data & 0x04) << 4) | ((data & 0x03) << 7);
	membank("bank1")->set_base(memregion("soundcpu")->base() + 0x100000 + 0x2000 * m_sound_bank);
}


WRITE8_MEMBER(segas32_state::multipcm_bank_w)
{
	m_multipcm->set_bank(0x80000 * ((data >> 3) & 7), 0x80000 * (data & 7));
}


WRITE8_MEMBER(segas32_state::scross_bank_w)
{
	m_multipcm->set_bank(0x80000 * (data & 7), 0x80000 * (data & 7));
}



/*************************************
 *
 *  Sound hack (not protection)
 *
 *************************************/

READ8_MEMBER(segas32_state::sound_dummy_r)
{
	return m_sound_dummy_value;
}


WRITE8_MEMBER(segas32_state::sound_dummy_w)
{
	m_sound_dummy_value = data;
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( system32_map, AS_PROGRAM, 16, segas32_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x200000, 0x20ffff) AM_MIRROR(0x0f0000) AM_RAM AM_SHARE("workram")
	AM_RANGE(0x300000, 0x31ffff) AM_MIRROR(0x0e0000) AM_READWRITE(system32_videoram_r, system32_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x400000, 0x41ffff) AM_MIRROR(0x0e0000) AM_READWRITE(system32_spriteram_r, system32_spriteram_w) AM_SHARE("spriteram")
	AM_RANGE(0x500000, 0x50000f) AM_MIRROR(0x0ffff0) AM_READWRITE(system32_sprite_control_r, system32_sprite_control_w)
	AM_RANGE(0x600000, 0x60ffff) AM_MIRROR(0x0e0000) AM_READWRITE(system32_paletteram_r, system32_paletteram_w) AM_SHARE("paletteram.0")
	AM_RANGE(0x610000, 0x61007f) AM_MIRROR(0x0eff80) AM_READWRITE(system32_mixer_r, system32_mixer_w)
	AM_RANGE(0x700000, 0x701fff) AM_MIRROR(0x0fe000) AM_READWRITE(shared_ram_16_r, shared_ram_16_w)
	AM_RANGE(0x800000, 0x800fff) AM_DEVREADWRITE8("s32comm", s32comm_device, share_r, share_w, 0x00ff)
	AM_RANGE(0x801000, 0x801001) AM_DEVREADWRITE8("s32comm", s32comm_device, cn_r, cn_w, 0x00ff)
	AM_RANGE(0x801002, 0x801003) AM_DEVREADWRITE8("s32comm", s32comm_device, fg_r, fg_w, 0x00ff)
	AM_RANGE(0xc00000, 0xc0001f) AM_MIRROR(0x0fff80) AM_READWRITE(io_chip_r, io_chip_w)
	AM_RANGE(0xc00040, 0xc0007f) AM_MIRROR(0x0fff80) AM_READWRITE(io_expansion_r, io_expansion_w)
	AM_RANGE(0xd00000, 0xd0000f) AM_MIRROR(0x07fff0) AM_READWRITE(interrupt_control_16_r, interrupt_control_16_w)
	AM_RANGE(0xd80000, 0xdfffff) AM_READWRITE(random_number_16_r, random_number_16_w)
	AM_RANGE(0xf00000, 0xffffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END


static ADDRESS_MAP_START( multi32_map, AS_PROGRAM, 32, segas32_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xffffff)
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x200000, 0x21ffff) AM_MIRROR(0x0e0000) AM_RAM
	AM_RANGE(0x300000, 0x31ffff) AM_MIRROR(0x0e0000) AM_READWRITE(multi32_videoram_r, multi32_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x400000, 0x41ffff) AM_MIRROR(0x0e0000) AM_READWRITE(multi32_spriteram_r, multi32_spriteram_w) AM_SHARE("spriteram")
	AM_RANGE(0x500000, 0x50000f) AM_MIRROR(0x0ffff0) AM_READWRITE(multi32_sprite_control_r, multi32_sprite_control_w)
	AM_RANGE(0x600000, 0x60ffff) AM_MIRROR(0x060000) AM_READWRITE(multi32_paletteram_0_r, multi32_paletteram_0_w) AM_SHARE("paletteram.0")
	AM_RANGE(0x610000, 0x61007f) AM_MIRROR(0x06ff80) AM_WRITE(multi32_mixer_0_w)
	AM_RANGE(0x680000, 0x68ffff) AM_MIRROR(0x060000) AM_READWRITE(multi32_paletteram_1_r, multi32_paletteram_1_w) AM_SHARE("paletteram.1")
	AM_RANGE(0x690000, 0x69007f) AM_MIRROR(0x06ff80) AM_WRITE(multi32_mixer_1_w)
	AM_RANGE(0x700000, 0x701fff) AM_MIRROR(0x0fe000) AM_READWRITE(shared_ram_32_r, shared_ram_32_w)
	AM_RANGE(0x800000, 0x800fff) AM_DEVREADWRITE8("s32comm", s32comm_device, share_r, share_w, 0x00ff00ff)
	AM_RANGE(0x801000, 0x801003) AM_DEVREADWRITE8("s32comm", s32comm_device, cn_r, cn_w, 0x000000ff)
	AM_RANGE(0x801000, 0x801003) AM_DEVREADWRITE8("s32comm", s32comm_device, fg_r, fg_w, 0x00ff0000)
	AM_RANGE(0xc00000, 0xc0001f) AM_MIRROR(0x07ff80) AM_READWRITE(io_chip_0_r, io_chip_0_w)
	AM_RANGE(0xc00040, 0xc0007f) AM_MIRROR(0x07ff80) AM_READWRITE(io_expansion_0_r, io_expansion_0_w)
	AM_RANGE(0xc80000, 0xc8001f) AM_MIRROR(0x07ff80) AM_READWRITE(io_chip_1_r, io_chip_1_w)
	AM_RANGE(0xc80040, 0xc8007f) AM_MIRROR(0x07ff80) AM_READWRITE(io_expansion_1_r, io_expansion_1_w)
	AM_RANGE(0xd00000, 0xd0000f) AM_MIRROR(0x07fff0) AM_READWRITE(interrupt_control_32_r, interrupt_control_32_w)
	AM_RANGE(0xd80000, 0xdfffff) AM_READWRITE(random_number_32_r, random_number_32_w)
	AM_RANGE(0xf00000, 0xffffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END



/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( system32_sound_map, AS_PROGRAM, 8, segas32_state )
	AM_RANGE(0x0000, 0x9fff) AM_ROM AM_REGION("soundcpu", 0x100000)
	AM_RANGE(0xa000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xc00f) AM_MIRROR(0x0ff0) AM_DEVWRITE("rfsnd", rf5c68_device, rf5c68_w)
	AM_RANGE(0xd000, 0xdfff) AM_DEVREADWRITE("rfsnd", rf5c68_device, rf5c68_mem_r, rf5c68_mem_w)
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_SHARE("z80_shared_ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( system32_sound_portmap, AS_IO, 8, segas32_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x80, 0x83) AM_MIRROR(0x0c) AM_DEVREADWRITE("ym1", ym3438_device, read, write)
	AM_RANGE(0x90, 0x93) AM_MIRROR(0x0c) AM_DEVREADWRITE("ym2", ym3438_device, read, write)
	AM_RANGE(0xa0, 0xaf) AM_WRITE(sound_bank_lo_w)
	AM_RANGE(0xb0, 0xbf) AM_WRITE(sound_bank_hi_w)
	AM_RANGE(0xc0, 0xcf) AM_WRITE(sound_int_control_lo_w)
	AM_RANGE(0xd0, 0xd3) AM_MIRROR(0x04) AM_WRITE(sound_int_control_hi_w)
	AM_RANGE(0xf1, 0xf1) AM_READWRITE(sound_dummy_r, sound_dummy_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( multi32_sound_map, AS_PROGRAM, 8, segas32_state )
	AM_RANGE(0x0000, 0x9fff) AM_ROM AM_REGION("soundcpu", 0x100000)
	AM_RANGE(0xa000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xdfff) AM_DEVREADWRITE("sega", multipcm_device, read, write)
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_SHARE("z80_shared_ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( multi32_sound_portmap, AS_IO, 8, segas32_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x80, 0x83) AM_MIRROR(0x0c) AM_DEVREADWRITE("ymsnd", ym3438_device, read, write)
	AM_RANGE(0xa0, 0xaf) AM_WRITE(sound_bank_lo_w)
	AM_RANGE(0xb0, 0xbf) AM_WRITE(multipcm_bank_w)
	AM_RANGE(0xc0, 0xcf) AM_WRITE(sound_int_control_lo_w)
	AM_RANGE(0xd0, 0xd3) AM_MIRROR(0x04) AM_WRITE(sound_int_control_hi_w)
	AM_RANGE(0xf1, 0xf1) AM_READWRITE(sound_dummy_r, sound_dummy_w)
ADDRESS_MAP_END



/*************************************
 *
 *  GA2 Protection CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( ga2_v25_map, AS_PROGRAM, 8, segas32_state )
	AM_RANGE(0x00000, 0x0ffff) AM_ROM AM_REGION("mcu", 0)
	AM_RANGE(0x10000, 0x1ffff) AM_RAM AM_SHARE("ga2_dpram")
	AM_RANGE(0xf0000, 0xfffff) AM_ROM AM_REGION("mcu", 0)
ADDRESS_MAP_END



/*************************************
 *
 *  Generic port definitions
 *
 *************************************/


static INPUT_PORTS_START( system32_generic )
	PORT_START("mainpcb:P1_A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY

	PORT_START("mainpcb:P2_A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("mainpcb:PORTC_A")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("mainpcb:PORTD_A")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("mainpcb:SERVICE12_A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("mainpcb:SERVICE34_A")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 )   /* sometimes mirrors SERVICE1 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE4 )   /* tends to also work as a test switch */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("mainpcb:eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START("mainpcb:PORTG_A")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("mainpcb:PORTH_A")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( system32_generic_slave )
	PORT_START("slavepcb:P1_A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY

	PORT_START("slavepcb:P2_A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("slavepcb:PORTC_A")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("slavepcb:PORTD_A")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("slavepcb:SERVICE12_A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("slavepcb:SERVICE34_A")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 )   /* sometimes mirrors SERVICE1 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE4 )   /* tends to also work as a test switch */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("slavepcb:eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START("slavepcb:PORTG_A")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("slavepcb:PORTH_A")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( multi32_generic )
	PORT_INCLUDE( system32_generic )

	PORT_START("mainpcb:P1_B")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("mainpcb:P2_B")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("mainpcb:PORTC_B")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("mainpcb:PORTD_B")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("mainpcb:SERVICE12_B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("mainpcb:SERVICE34_B")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("mainpcb:eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START("mainpcb:PORTG_B")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("mainpcb:PORTH_B")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Game-specific port definitions
 *
 *************************************/

static INPUT_PORTS_START( arescue )
	PORT_INCLUDE( system32_generic )

	PORT_MODIFY("mainpcb:P1_A")
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("mainpcb:P2_A")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("mainpcb:SERVICE12_A")
	PORT_BIT( 0x38, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("mainpcb:SERVICE34_A")
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("mainpcb:ANALOG1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_REVERSE

	PORT_START("mainpcb:ANALOG2")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)

	PORT_START("mainpcb:ANALOG3")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Z ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)

	PORT_INCLUDE( system32_generic_slave )

	PORT_MODIFY("slavepcb:P1_A")
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)

	PORT_MODIFY("slavepcb:P2_A")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("slavepcb:SERVICE12_A")
	PORT_BIT( 0x38, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("slavepcb:SERVICE34_A")
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("slavepcb:ANALOG1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_REVERSE  PORT_PLAYER(2)

	PORT_START("slavepcb:ANALOG2")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)  PORT_PLAYER(2)

	PORT_START("slavepcb:ANALOG3")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Z ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)  PORT_PLAYER(2)

INPUT_PORTS_END


static INPUT_PORTS_START( alien3 )
	PORT_INCLUDE( system32_generic )

	PORT_MODIFY("mainpcb:P1_A")
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("mainpcb:P2_A")
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("mainpcb:SERVICE12_A")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("mainpcb:ANALOG1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5)

	PORT_START("mainpcb:ANALOG2")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5)

	PORT_START("mainpcb:ANALOG3")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_PLAYER(2)

	PORT_START("mainpcb:ANALOG4")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( arabfgt )
	PORT_INCLUDE( system32_generic )

	PORT_MODIFY("mainpcb:P1_A")
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("mainpcb:P2_A")
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("mainpcb:EXTRA1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)

	PORT_START("mainpcb:EXTRA2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)

	PORT_START("mainpcb:EXTRA3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( arabfgtu )
	PORT_INCLUDE( arabfgt )

	PORT_MODIFY("mainpcb:SERVICE12_A")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_MODIFY("mainpcb:EXTRA3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END


static INPUT_PORTS_START( brival )
	PORT_INCLUDE( system32_generic )

	PORT_MODIFY("mainpcb:P1_A")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("mainpcb:P2_A")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("mainpcb:EXTRA2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
INPUT_PORTS_END


static INPUT_PORTS_START( darkedge )
	PORT_INCLUDE( system32_generic )

	PORT_MODIFY("mainpcb:P1_A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_MODIFY("mainpcb:P2_A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)

	PORT_START("mainpcb:EXTRA2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
INPUT_PORTS_END


static INPUT_PORTS_START( dbzvrvs )
	PORT_INCLUDE( system32_generic )
INPUT_PORTS_END


static INPUT_PORTS_START( f1en )
	PORT_INCLUDE( system32_generic )

	PORT_MODIFY("mainpcb:P1_A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("mainpcb:Gear Up")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("mainpcb:Gear Down")
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("mainpcb:P2_A")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("mainpcb:SERVICE12_A")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("mainpcb:SERVICE34_A")
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
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("mainpcb:ANALOG1")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_NAME("mainpcb:Steering Wheel")

	PORT_START("mainpcb:ANALOG2")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL  ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_NAME("mainpcb:Gas Pedal")

	PORT_START("mainpcb:ANALOG3")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_NAME("mainpcb:Brake Pedal")

	PORT_INCLUDE( system32_generic_slave )

	PORT_MODIFY("slavepcb:P1_A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("slavepcb:Gear Up")  PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("slavepcb:Gear Down") PORT_PLAYER(2)
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("slavepcb:P2_A")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("slavepcb:SERVICE12_A")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("slavepcb:SERVICE34_A")
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
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("slavepcb:ANALOG1")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_NAME("slavepcb:Steering Wheel")  PORT_PLAYER(2)

	PORT_START("slavepcb:ANALOG2")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL  ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_NAME("slavepcb:Gas Pedal")  PORT_PLAYER(2)

	PORT_START("slavepcb:ANALOG3")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_NAME("slavepcb:Brake Pedal")  PORT_PLAYER(2)

INPUT_PORTS_END


static INPUT_PORTS_START( f1lap )
	PORT_INCLUDE( system32_generic )

	PORT_MODIFY("mainpcb:P1_A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("mainpcb:Gear Up")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("mainpcb:Gear Down")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_Z) PORT_NAME("mainpcb:Overtake")
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("mainpcb:P2_A")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("mainpcb:SERVICE12_A")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("mainpcb:SERVICE34_A")
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
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) ) // service coin mirror
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) ) // seems to be a service switch mirror
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("mainpcb:ANALOG1")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(50) PORT_KEYDELTA(20) PORT_NAME("mainpcb:Steering Wheel")

	PORT_START("mainpcb:ANALOG2")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL  ) PORT_SENSITIVITY(50) PORT_KEYDELTA(20) PORT_NAME("mainpcb:Gas Pedal")

	PORT_START("mainpcb:ANALOG3")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(50) PORT_KEYDELTA(20) PORT_NAME("mainpcb:Brake Pedal")
INPUT_PORTS_END


static INPUT_PORTS_START( ga2 )
	PORT_INCLUDE( system32_generic )

	PORT_START("mainpcb:EXTRA1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)

	PORT_START("mainpcb:EXTRA2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)

	PORT_START("mainpcb:EXTRA3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( ga2u )
	PORT_INCLUDE( ga2 )

	PORT_MODIFY("mainpcb:SERVICE12_A")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_MODIFY("mainpcb:EXTRA3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END


static INPUT_PORTS_START( harddunk )
	PORT_INCLUDE( multi32_generic )

	PORT_MODIFY("mainpcb:P1_A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)

	PORT_MODIFY("mainpcb:P2_A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)

	PORT_MODIFY("mainpcb:SERVICE12_A")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("mainpcb:P1_B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)

	PORT_MODIFY("mainpcb:P2_B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(5)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(5)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(5)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(5)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(5)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(5)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(5)

	PORT_MODIFY("mainpcb:SERVICE12_B")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START5 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("mainpcb:EXTRA1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)

	PORT_START("mainpcb:EXTRA2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(6)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(6)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(6)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(6)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(6)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(6)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(6)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(6)

	PORT_START("mainpcb:EXTRA3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START6 )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( holo )
	PORT_INCLUDE( system32_generic )

	PORT_MODIFY("mainpcb:P1_A")
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("mainpcb:P2_A")
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( jpark )
	PORT_INCLUDE( system32_generic )

	PORT_MODIFY("mainpcb:P1_A")
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("mainpcb:P2_A")
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("mainpcb:ANALOG1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5)

	PORT_START("mainpcb:ANALOG2")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5)

	PORT_START("mainpcb:ANALOG3")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_PLAYER(2)

	PORT_START("mainpcb:ANALOG4")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( orunners )
	PORT_INCLUDE( multi32_generic )

	PORT_MODIFY("mainpcb:P1_A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_CODE(KEYCODE_SPACE)    /* shift up */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_CODE(KEYCODE_LSHIFT)   /* shift down */
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("mainpcb:P2_A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_CODE(KEYCODE_Z)        /* DJ/music */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_CODE(KEYCODE_X)        /* << */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_CODE(KEYCODE_C)        /* >> */
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("mainpcb:P1_B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_CODE(KEYCODE_Q)        /* shift up */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_CODE(KEYCODE_W)        /* shift down */
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("mainpcb:P2_B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)                             /* DJ/music */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)                             /* << */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)                             /* >> */
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("mainpcb:ANALOG1")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("mainpcb:ANALOG2")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("mainpcb:ANALOG3")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("mainpcb:ANALOG4")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("mainpcb:ANALOG7")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("mainpcb:ANALOG8")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( radm )
	PORT_INCLUDE( system32_generic )

	PORT_MODIFY("mainpcb:P1_A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("mainpcb:Light")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("mainpcb:Wiper")
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("mainpcb:P2_A")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("mainpcb:SERVICE12_A")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("mainpcb:SERVICE34_A")
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
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("mainpcb:ANALOG1")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)

	PORT_START("mainpcb:ANALOG2")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)

	PORT_START("mainpcb:ANALOG3")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)
INPUT_PORTS_END


static INPUT_PORTS_START( radr )
	PORT_INCLUDE( system32_generic )

	PORT_MODIFY("mainpcb:P1_A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("mainpcb:Gear Change") PORT_TOGGLE
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("mainpcb:P2_A")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("mainpcb:SERVICE12_A")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("mainpcb:SERVICE34_A")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Transmission" )
	PORT_DIPSETTING(    0x08, "Manual" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("mainpcb:ANALOG1")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)

	PORT_START("mainpcb:ANALOG2")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)

	PORT_START("mainpcb:ANALOG3")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)
INPUT_PORTS_END


static INPUT_PORTS_START( scross )
	PORT_INCLUDE( multi32_generic )

	PORT_MODIFY("mainpcb:P1_A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_CODE(KEYCODE_SPACE)        /* P1 Attack */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_CODE(KEYCODE_LSHIFT)      /* P1 Wheelie */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_CODE(KEYCODE_LALT)     /* P1 Brake */
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("mainpcb:P1_B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_CODE(KEYCODE_Q)        /* P2 Attack */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_CODE(KEYCODE_W)       /* P2 Wheelie */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_CODE(KEYCODE_S)        /* P2 Brake */
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("mainpcb:P2_A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("mainpcb:ANALOG1")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("mainpcb:ANALOG2")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("mainpcb:ANALOG3")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("mainpcb:ANALOG4")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( slipstrm )
	PORT_INCLUDE( system32_generic )

	PORT_MODIFY("mainpcb:P1_A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("mainpcb:Gear Change") PORT_TOGGLE
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("mainpcb:P2_A")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("mainpcb:SERVICE12_A")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("mainpcb:SERVICE34_A")
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
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("mainpcb:ANALOG1")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)

	PORT_START("mainpcb:ANALOG2")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)

	PORT_START("mainpcb:ANALOG3")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)
INPUT_PORTS_END


static INPUT_PORTS_START( sonic )
	PORT_INCLUDE( system32_generic )

	PORT_MODIFY("mainpcb:P1_A")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("mainpcb:P2_A")
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("mainpcb:SERVICE12_A")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START3 )

	PORT_START("mainpcb:TRACKX1")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(30) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("mainpcb:TRACKY1")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(30) PORT_PLAYER(1)

	PORT_START("mainpcb:TRACKX2")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(30) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("mainpcb:TRACKY2")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(30) PORT_PLAYER(2)

	PORT_START("mainpcb:TRACKX3")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(30) PORT_REVERSE PORT_PLAYER(3)

	PORT_START("mainpcb:TRACKY3")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(30) PORT_PLAYER(3)
INPUT_PORTS_END


static INPUT_PORTS_START( spidman )
	PORT_INCLUDE( system32_generic )

	PORT_MODIFY("mainpcb:P1_A")
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("mainpcb:P2_A")
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("mainpcb:EXTRA1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)

	PORT_START("mainpcb:EXTRA2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)

	PORT_START("mainpcb:EXTRA3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( spidmanu )
	PORT_INCLUDE( spidman )

	PORT_MODIFY("mainpcb:SERVICE12_A")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_MODIFY("mainpcb:EXTRA3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END


static INPUT_PORTS_START( svf )
	PORT_INCLUDE( system32_generic )
INPUT_PORTS_END


static INPUT_PORTS_START( titlef )
	PORT_INCLUDE( multi32_generic )

	PORT_MODIFY("mainpcb:P1_A")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_PLAYER(1)

	PORT_MODIFY("mainpcb:P2_A")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_PLAYER(1)

	PORT_MODIFY("mainpcb:SERVICE12_A")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("mainpcb:P1_B")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_PLAYER(2)

	PORT_MODIFY("mainpcb:P2_B")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_PLAYER(2)

	PORT_MODIFY("mainpcb:SERVICE12_B")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout bgcharlayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 0, 4, 16, 20, 8, 12, 24, 28, 32, 36, 48, 52, 40, 44, 56, 60  },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
		8*64, 9*64,10*64,11*64,12*64,13*64,14*64,15*64 },
	16*64
};


static GFXDECODE_START( segas32 )
	GFXDECODE_ENTRY( "gfx1", 0, bgcharlayout,   0x00, 0x3ff  )
GFXDECODE_END





/*************************************
 *
 *  Machine driver
 *
 *************************************/


static MACHINE_CONFIG_FRAGMENT( system32 )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V60, MASTER_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(system32_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", segas32_state,  start_of_vblank_int)

	MCFG_CPU_ADD("soundcpu", Z80, MASTER_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(system32_sound_map)
	MCFG_CPU_IO_MAP(system32_sound_portmap)


	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	MCFG_TIMER_DRIVER_ADD("v60_irq0", segas32_state, signal_v60_irq_callback)
	MCFG_TIMER_DRIVER_ADD("v60_irq1", segas32_state, signal_v60_irq_callback)

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", segas32)
	MCFG_PALETTE_ADD("palette", 0x4000)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(52*8, 262)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 52*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(segas32_state, screen_update_system32)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ym1", YM3438, MASTER_CLOCK/4)
	MCFG_YM2612_IRQ_HANDLER(WRITELINE(segas32_state, ym3438_irq_handler))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.40)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.40)

	MCFG_SOUND_ADD("ym2", YM3438, MASTER_CLOCK/4)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.40)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.40)

	MCFG_RF5C68_ADD("rfsnd", RFC_CLOCK/4)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.55)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.55)

	MCFG_S32COMM_ADD("s32comm")
MACHINE_CONFIG_END

const device_type SEGA_S32_REGULAR_DEVICE = &device_creator<segas32_regular_state>;

segas32_regular_state::segas32_regular_state(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: segas32_state(mconfig, tag, owner, clock)
{
}

machine_config_constructor segas32_regular_state::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( system32 );
}




static MACHINE_CONFIG_FRAGMENT( system32_v25 )
	MCFG_FRAGMENT_ADD( system32 )

	/* add a V25 for protection */
	MCFG_CPU_ADD("mcu", V25, 10000000)
	MCFG_CPU_PROGRAM_MAP(ga2_v25_map)
	MCFG_V25_CONFIG(ga2_v25_opcode_table)
MACHINE_CONFIG_END

const device_type SEGA_S32_V25_DEVICE = &device_creator<segas32_v25_state>;

segas32_v25_state::segas32_v25_state(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: segas32_state(mconfig, tag, owner, clock)
{
}

machine_config_constructor segas32_v25_state::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( system32_v25 );
}


static MACHINE_CONFIG_FRAGMENT( multi32 )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V70, MULTI32_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(multi32_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", segas32_state,  start_of_vblank_int)

	MCFG_CPU_ADD("soundcpu", Z80, MASTER_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(multi32_sound_map)
	MCFG_CPU_IO_MAP(multi32_sound_portmap)


	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	MCFG_TIMER_DRIVER_ADD("v60_irq0", segas32_state, signal_v60_irq_callback)
	MCFG_TIMER_DRIVER_ADD("v60_irq1", segas32_state, signal_v60_irq_callback)

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", segas32)
	MCFG_PALETTE_ADD("palette", 0x8000)
	MCFG_DEFAULT_LAYOUT(layout_dualhsxs)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(52*8, 262)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 52*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(segas32_state, screen_update_multi32_left)

	MCFG_SCREEN_ADD("screen2", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(52*8, 262)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 52*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(segas32_state, screen_update_multi32_right)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM3438, MASTER_CLOCK/4)
	MCFG_YM2612_IRQ_HANDLER(WRITELINE(segas32_state, ym3438_irq_handler))
	MCFG_SOUND_ROUTE(1, "lspeaker", 0.40)
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.40)

	MCFG_SOUND_ADD("sega", MULTIPCM, MASTER_CLOCK/4)
	MCFG_SOUND_ROUTE(1, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(0, "rspeaker", 1.0)

	MCFG_S32COMM_ADD("s32comm")
MACHINE_CONFIG_END


const device_type SEGA_MULTI32_DEVICE = &device_creator<sega_multi32_state>;

sega_multi32_state::sega_multi32_state(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: segas32_state(mconfig, tag, owner, clock)
{
}

machine_config_constructor sega_multi32_state::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( multi32 );
}


class segas32_new_state : public driver_device
{
public:
	segas32_new_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_mainpcb(*this, "mainpcb"),
	m_slavepcb(*this, "slavepcb")
	{ }

	required_device<segas32_state> m_mainpcb;
	optional_device<segas32_state> m_slavepcb;

	DECLARE_DRIVER_INIT(titlef);
	DECLARE_DRIVER_INIT(slipstrm);
	DECLARE_DRIVER_INIT(radm);
	DECLARE_DRIVER_INIT(holo);
	DECLARE_DRIVER_INIT(svf);
	DECLARE_DRIVER_INIT(jleague);
	DECLARE_DRIVER_INIT(arescue);
	DECLARE_DRIVER_INIT(jpark);
	DECLARE_DRIVER_INIT(ga2);
	DECLARE_DRIVER_INIT(scross);
	DECLARE_DRIVER_INIT(spidman);
	DECLARE_DRIVER_INIT(sonicp);
	DECLARE_DRIVER_INIT(f1en);
	DECLARE_DRIVER_INIT(dbzvrvs);
	DECLARE_DRIVER_INIT(brival);
	DECLARE_DRIVER_INIT(harddunk);
	DECLARE_DRIVER_INIT(arabfgt);
	DECLARE_DRIVER_INIT(sonic);
	DECLARE_DRIVER_INIT(alien3);
	DECLARE_DRIVER_INIT(darkedge);
	DECLARE_DRIVER_INIT(radr);
	DECLARE_DRIVER_INIT(f1lap);
	DECLARE_DRIVER_INIT(orunners);

	std::unique_ptr<UINT16[]> m_dual_pcb_comms;
	DECLARE_WRITE16_MEMBER(dual_pcb_comms_w);
	DECLARE_READ16_MEMBER(dual_pcb_comms_r);
	DECLARE_READ16_MEMBER(dual_pcb_masterslave);
	DECLARE_READ16_MEMBER(dual_pcb_slave);


};



static MACHINE_CONFIG_START( sega_system32, segas32_new_state )
	MCFG_DEVICE_ADD("mainpcb", SEGA_S32_REGULAR_DEVICE, 0)
MACHINE_CONFIG_END

// for air rescue & f1en where there is a sub-board containing shared ram sitting underneath the ROM board bridging 2 PCBs (not a network link)
static MACHINE_CONFIG_START( sega_system32_dual_direct, segas32_new_state )
	MCFG_DEVICE_ADD("mainpcb", SEGA_S32_REGULAR_DEVICE, 0)
	MCFG_DEVICE_ADD("slavepcb", SEGA_S32_REGULAR_DEVICE, 0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( sega_system32_v25, segas32_new_state )
	MCFG_DEVICE_ADD("mainpcb", SEGA_S32_V25_DEVICE, 0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( sega_multi32, segas32_new_state )
	MCFG_DEVICE_ADD("mainpcb", SEGA_MULTI32_DEVICE, 0)
MACHINE_CONFIG_END

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/


#define ROM_LOAD_x2(name, base, length, crc) \
	ROM_LOAD( name, base + 0 * length, length, crc ) \
	ROM_RELOAD(     base + 1 * length, length )

#define ROM_LOAD_x4(name, base, length, crc) \
	ROM_LOAD( name, base + 0 * length, length, crc ) \
	ROM_RELOAD(     base + 1 * length, length ) \
	ROM_RELOAD(     base + 2 * length, length ) \
	ROM_RELOAD(     base + 3 * length, length )

#define ROM_LOAD_x8(name, base, length, crc) \
	ROM_LOAD( name, base + 0 * length, length, crc ) \
	ROM_RELOAD(     base + 1 * length, length ) \
	ROM_RELOAD(     base + 2 * length, length ) \
	ROM_RELOAD(     base + 3 * length, length ) \
	ROM_RELOAD(     base + 4 * length, length ) \
	ROM_RELOAD(     base + 5 * length, length ) \
	ROM_RELOAD(     base + 6 * length, length ) \
	ROM_RELOAD(     base + 7 * length, length )

#define ROM_LOAD_x16(name, base, length, crc) \
	ROM_LOAD( name, base + 0 * length, length, crc ) \
	ROM_RELOAD(     base + 1 * length, length ) \
	ROM_RELOAD(     base + 2 * length, length ) \
	ROM_RELOAD(     base + 3 * length, length ) \
	ROM_RELOAD(     base + 4 * length, length ) \
	ROM_RELOAD(     base + 5 * length, length ) \
	ROM_RELOAD(     base + 6 * length, length ) \
	ROM_RELOAD(     base + 7 * length, length ) \
	ROM_RELOAD(     base + 8 * length, length ) \
	ROM_RELOAD(     base + 9 * length, length ) \
	ROM_RELOAD(     base + 10 * length, length ) \
	ROM_RELOAD(     base + 11 * length, length ) \
	ROM_RELOAD(     base + 12 * length, length ) \
	ROM_RELOAD(     base + 13 * length, length ) \
	ROM_RELOAD(     base + 14 * length, length ) \
	ROM_RELOAD(     base + 15 * length, length )

#define ROM_LOAD16_BYTE_x2(name, base, length, crc) \
	ROM_LOAD16_BYTE( name, base + 0 * length, length, crc ) \
	ROM_RELOAD(     base + 2 * length, length )

#define ROM_LOAD16_BYTE_x4(name, base, length, crc) \
	ROM_LOAD16_BYTE( name, base + 0 * length, length, crc ) \
	ROM_RELOAD(     base + 2 * length, length ) \
	ROM_RELOAD(     base + 4 * length, length ) \
	ROM_RELOAD(     base + 6 * length, length )

#define ROM_LOAD32_WORD_x2(name, base, length, crc) \
	ROM_LOAD32_WORD( name, base + 0 * length, length, crc ) \
	ROM_RELOAD(     base + 2 * length, length )

#define ROM_LOAD32_WORD_x4(name, base, length, crc) \
	ROM_LOAD32_WORD( name, base + 0 * length, length, crc ) \
	ROM_RELOAD(     base + 2 * length, length ) \
	ROM_RELOAD(     base + 4 * length, length ) \
	ROM_RELOAD(     base + 6 * length, length )



/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Air Rescue (US)

    Sega Game ID codes:
       Game BD: 833-8508-01 AIR RESCUE (US)
                833-8508-02 AIR RESCUE (Export)
                833-8508-03 AIR RESCUE (Japan)
    Rom PCB No: 834-8526-01 (US)
                834-8526-02 (Export)
                834-8526-03 (Japan)
   Link PCB No: 837-8223-01
     A/D BD No: 837-7536 (one for each mainboard)
     DSP BD No: 837-8341

    requires 2 linked system32 pcbs
    requires additional math DSP to be emulated

    The link PCB attaches 2 System 32 mainboards together, then ROM boards for each mainboard attaches to the link PCB.
    This provides a direct connection between the PCBs (NOT a network link) so they effectively operate as a single boardset
    sharing RAM (we should emulate it as such)

    Link PCB is a single sparsely populated romless PCB but contains

    Left side
    1x MB8431-12LP (IC2)
    2x HD74LS74AP (IC6, IC7)
    2x GAL16V8A-25LP (stamped 315-5545) (IC3)

    Right side
    1x MB8421-12LP (IC1)
    1x GAL16V8A-25LP (stamped xxx-xxxx) (IC5)
    1x HD74LS74AP (IC8)
    1x GAL16V8A-25LP (stamped 315-5545) (IC4)

    (todo, full layout)

    The left Rom PCB (master?) contains a sub-board on the ROM board with the math DSP, the right Rom PCB does not have this.

*/
ROM_START( arescue )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x4( "epr-14540.ic13",     0x000000, 0x020000, CRC(c2b4e5d0) SHA1(69f8ddded5095df9012663d0ded61b78f1692a8d) )
	ROM_LOAD_x4( "epr-14539.ic6",      0x080000, 0x020000, CRC(1a1b5532) SHA1(f3651470222036703b7ecedb6e91e4cdb3d20df7) )
	ROM_LOAD16_BYTE( "epr-14509.ic14", 0x100000, 0x080000, CRC(daa5a356) SHA1(ca87242c59de5ab5f9406635bee758a855fe20bc) )
	ROM_LOAD16_BYTE( "epr-14508.ic7",  0x100001, 0x080000, CRC(6702c14d) SHA1(dc9324f16a3e3238f5ccdade9451d6823a50b563) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x4( "epr-14513.ic35", 0x100000, 0x40000, CRC(f9a884cd) SHA1(73010fff5e0257355e08e78838c74af86ed364ce) )
	ROM_LOAD_x2( "mpr-14512.ic31", 0x200000, 0x80000, CRC(9da48051) SHA1(2d41148d089a75362ed0fde577eca919213ac666) )
	ROM_LOAD_x2( "mpr-14511.ic26", 0x300000, 0x80000, CRC(074c53cc) SHA1(9c89843bbe8058123c25b7f8f86de754ddbca2bb) )
	ROM_LOAD_x2( "mpr-14510.ic22", 0x400000, 0x80000, CRC(5ea6d52d) SHA1(d424082468940bb91ab424ac7812839792ed4e88) )

	ROM_REGION( 0x200000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD32_BYTE( "mpr-14496.ic25", 0x000003, 0x080000, CRC(737da16c) SHA1(52247d9bc2924e90d040bef484a541b1f4a9026f) )
	ROM_LOAD32_BYTE( "mpr-14497.ic29", 0x000001, 0x080000, CRC(ebd7ed17) SHA1(2307dc28501965432d2ff55a21698efdce014401) )
	ROM_LOAD32_BYTE( "mpr-14498.ic34", 0x000002, 0x080000, CRC(d4a764bd) SHA1(8434a9225ed1e01e8b1cfe169268e42cd3ce6ee3) )
	ROM_LOAD32_BYTE( "mpr-14499.ic38", 0x000000, 0x080000, CRC(fadc4b2b) SHA1(01c02a4dfad1ab19bac8b81b61d37fdc035bc5c5) )

	ROM_REGION32_BE( 0x800000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-14500.ic24", 0x000007, 0x100000, CRC(0a064e9b) SHA1(264761f4aacaeeac9426528caf180404cd7f6e18) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14501.ic28", 0x000006, 0x100000, CRC(4662bb41) SHA1(80774e680468e9ba9c5dd5eeaa4791fa3b3722fd) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14502.ic33", 0x000005, 0x100000, CRC(988555a9) SHA1(355e44319fd51358329cc7cd226e4c4725e045cb) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14503.ic37", 0x000004, 0x100000, CRC(90556aca) SHA1(24df62af55048db66d50c7034c5460330d231bf5) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14504.ic23", 0x000003, 0x100000, CRC(46dd038d) SHA1(9530a52e2e7388437c20ebcb19bf84c8b3b5086b) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14505.ic27", 0x000002, 0x100000, CRC(be142c1f) SHA1(224631e00c2458c39c6a2ef7978c2b1131fb4da2) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14506.ic32", 0x000001, 0x100000, CRC(5dd8fb6b) SHA1(7d21cacb2c9dba5db2547b6d8e89397e0424ee8e) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14507.ic36", 0x000000, 0x100000, CRC(db3f59ec) SHA1(96dcb3827354773fc2911c62260a27e90dcbe96a) , ROM_SKIP(7) )

	ROM_REGION( 0x20000, "user2", 0 ) /* NEC uPD77P25 DSP Internal ROM */ // ONLY PRESENT ON ONE PCB STACK
	ROM_LOAD( "d7725.01", 0x000000, 0x002800, CRC(a7ec5644) SHA1(e9b05c70b639ee289e557dfd9a6c724b36338e2b) )

	ROM_REGION( 0x200000, "slavepcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x4( "epr-14540.ic13",     0x000000, 0x020000, CRC(c2b4e5d0) SHA1(69f8ddded5095df9012663d0ded61b78f1692a8d) )
	ROM_LOAD_x4( "epr-14539.ic6",      0x080000, 0x020000, CRC(1a1b5532) SHA1(f3651470222036703b7ecedb6e91e4cdb3d20df7) )
	ROM_LOAD16_BYTE( "epr-14509.ic14", 0x100000, 0x080000, CRC(daa5a356) SHA1(ca87242c59de5ab5f9406635bee758a855fe20bc) )
	ROM_LOAD16_BYTE( "epr-14508.ic7",  0x100001, 0x080000, CRC(6702c14d) SHA1(dc9324f16a3e3238f5ccdade9451d6823a50b563) )

	ROM_REGION( 0x500000, "slavepcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x4( "epr-14513.ic35", 0x100000, 0x40000, CRC(f9a884cd) SHA1(73010fff5e0257355e08e78838c74af86ed364ce) )
	ROM_LOAD_x2( "mpr-14512.ic31", 0x200000, 0x80000, CRC(9da48051) SHA1(2d41148d089a75362ed0fde577eca919213ac666) )
	ROM_LOAD_x2( "mpr-14511.ic26", 0x300000, 0x80000, CRC(074c53cc) SHA1(9c89843bbe8058123c25b7f8f86de754ddbca2bb) )
	ROM_LOAD_x2( "mpr-14510.ic22", 0x400000, 0x80000, CRC(5ea6d52d) SHA1(d424082468940bb91ab424ac7812839792ed4e88) )

	ROM_REGION( 0x200000, "slavepcb:gfx1", 0 ) /* tiles */
	ROM_LOAD32_BYTE( "mpr-14496.ic25", 0x000003, 0x080000, CRC(737da16c) SHA1(52247d9bc2924e90d040bef484a541b1f4a9026f) )
	ROM_LOAD32_BYTE( "mpr-14497.ic29", 0x000001, 0x080000, CRC(ebd7ed17) SHA1(2307dc28501965432d2ff55a21698efdce014401) )
	ROM_LOAD32_BYTE( "mpr-14498.ic34", 0x000002, 0x080000, CRC(d4a764bd) SHA1(8434a9225ed1e01e8b1cfe169268e42cd3ce6ee3) )
	ROM_LOAD32_BYTE( "mpr-14499.ic38", 0x000000, 0x080000, CRC(fadc4b2b) SHA1(01c02a4dfad1ab19bac8b81b61d37fdc035bc5c5) )

	ROM_REGION32_BE( 0x800000, "slavepcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-14500.ic24", 0x000007, 0x100000, CRC(0a064e9b) SHA1(264761f4aacaeeac9426528caf180404cd7f6e18) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14501.ic28", 0x000006, 0x100000, CRC(4662bb41) SHA1(80774e680468e9ba9c5dd5eeaa4791fa3b3722fd) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14502.ic33", 0x000005, 0x100000, CRC(988555a9) SHA1(355e44319fd51358329cc7cd226e4c4725e045cb) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14503.ic37", 0x000004, 0x100000, CRC(90556aca) SHA1(24df62af55048db66d50c7034c5460330d231bf5) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14504.ic23", 0x000003, 0x100000, CRC(46dd038d) SHA1(9530a52e2e7388437c20ebcb19bf84c8b3b5086b) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14505.ic27", 0x000002, 0x100000, CRC(be142c1f) SHA1(224631e00c2458c39c6a2ef7978c2b1131fb4da2) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14506.ic32", 0x000001, 0x100000, CRC(5dd8fb6b) SHA1(7d21cacb2c9dba5db2547b6d8e89397e0424ee8e) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14507.ic36", 0x000000, 0x100000, CRC(db3f59ec) SHA1(96dcb3827354773fc2911c62260a27e90dcbe96a) , ROM_SKIP(7) )

ROM_END

/**************************************************************************************************************************
    Air Rescue (Japan)

    Sega Game ID codes:
       Game BD: 833-8508-03 AIR RESCUE
    Rom PCB No: 834-8526-03
   Link PCB No: 837-8223-01
     A/D BD No: 837-7536 (one for each mainboard)
     DSP BD No: 837-8341

    requires 2 linked system32 pcbs
    requires additional math DSP to be emulated
*/
ROM_START( arescuej )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x4( "epr-14515.ic13",     0x000000, 0x020000, CRC(fb5eefbd) SHA1(f2739ad2e168843fe992d7fb546ffd859fa6c17a) )
	ROM_LOAD_x4( "epr-14514.ic6",      0x080000, 0x020000, CRC(ebf6dfc5) SHA1(2146dc23f1268124b6ad3cd00416a71fc56130bf) )
	ROM_LOAD16_BYTE( "epr-14509.ic14", 0x100000, 0x080000, CRC(daa5a356) SHA1(ca87242c59de5ab5f9406635bee758a855fe20bc) )
	ROM_LOAD16_BYTE( "epr-14508.ic7",  0x100001, 0x080000, CRC(6702c14d) SHA1(dc9324f16a3e3238f5ccdade9451d6823a50b563) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x4( "epr-14513.ic35", 0x100000, 0x40000, CRC(f9a884cd) SHA1(73010fff5e0257355e08e78838c74af86ed364ce) )
	ROM_LOAD_x2( "mpr-14512.ic31", 0x200000, 0x80000, CRC(9da48051) SHA1(2d41148d089a75362ed0fde577eca919213ac666) )
	ROM_LOAD_x2( "mpr-14511.ic26", 0x300000, 0x80000, CRC(074c53cc) SHA1(9c89843bbe8058123c25b7f8f86de754ddbca2bb) )
	ROM_LOAD_x2( "mpr-14510.ic22", 0x400000, 0x80000, CRC(5ea6d52d) SHA1(d424082468940bb91ab424ac7812839792ed4e88) )

	ROM_REGION( 0x200000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD32_BYTE( "mpr-14496.ic25", 0x000003, 0x080000, CRC(737da16c) SHA1(52247d9bc2924e90d040bef484a541b1f4a9026f) )
	ROM_LOAD32_BYTE( "mpr-14497.ic29", 0x000001, 0x080000, CRC(ebd7ed17) SHA1(2307dc28501965432d2ff55a21698efdce014401) )
	ROM_LOAD32_BYTE( "mpr-14498.ic34", 0x000002, 0x080000, CRC(d4a764bd) SHA1(8434a9225ed1e01e8b1cfe169268e42cd3ce6ee3) )
	ROM_LOAD32_BYTE( "mpr-14499.ic38", 0x000000, 0x080000, CRC(fadc4b2b) SHA1(01c02a4dfad1ab19bac8b81b61d37fdc035bc5c5) )

	ROM_REGION32_BE( 0x800000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-14500.ic24", 0x000007, 0x100000, CRC(0a064e9b) SHA1(264761f4aacaeeac9426528caf180404cd7f6e18) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14501.ic28", 0x000006, 0x100000, CRC(4662bb41) SHA1(80774e680468e9ba9c5dd5eeaa4791fa3b3722fd) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14502.ic33", 0x000005, 0x100000, CRC(988555a9) SHA1(355e44319fd51358329cc7cd226e4c4725e045cb) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14503.ic37", 0x000004, 0x100000, CRC(90556aca) SHA1(24df62af55048db66d50c7034c5460330d231bf5) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14504.ic23", 0x000003, 0x100000, CRC(46dd038d) SHA1(9530a52e2e7388437c20ebcb19bf84c8b3b5086b) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14505.ic27", 0x000002, 0x100000, CRC(be142c1f) SHA1(224631e00c2458c39c6a2ef7978c2b1131fb4da2) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14506.ic32", 0x000001, 0x100000, CRC(5dd8fb6b) SHA1(7d21cacb2c9dba5db2547b6d8e89397e0424ee8e) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14507.ic36", 0x000000, 0x100000, CRC(db3f59ec) SHA1(96dcb3827354773fc2911c62260a27e90dcbe96a) , ROM_SKIP(7) )

	ROM_REGION( 0x20000, "user2", 0 ) /* NEC uPD77P25 DSP Internal ROM */ // ONLY PRESENT ON ONE PCB STACK
	ROM_LOAD( "d7725.01", 0x000000, 0x002800, CRC(a7ec5644) SHA1(e9b05c70b639ee289e557dfd9a6c724b36338e2b) )

	ROM_REGION( 0x200000, "slavepcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x4( "epr-14515.ic13",     0x000000, 0x020000, CRC(fb5eefbd) SHA1(f2739ad2e168843fe992d7fb546ffd859fa6c17a) )
	ROM_LOAD_x4( "epr-14514.ic6",      0x080000, 0x020000, CRC(ebf6dfc5) SHA1(2146dc23f1268124b6ad3cd00416a71fc56130bf) )
	ROM_LOAD16_BYTE( "epr-14509.ic14", 0x100000, 0x080000, CRC(daa5a356) SHA1(ca87242c59de5ab5f9406635bee758a855fe20bc) )
	ROM_LOAD16_BYTE( "epr-14508.ic7",  0x100001, 0x080000, CRC(6702c14d) SHA1(dc9324f16a3e3238f5ccdade9451d6823a50b563) )

	ROM_REGION( 0x500000, "slavepcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x4( "epr-14513.ic35", 0x100000, 0x40000, CRC(f9a884cd) SHA1(73010fff5e0257355e08e78838c74af86ed364ce) )
	ROM_LOAD_x2( "mpr-14512.ic31", 0x200000, 0x80000, CRC(9da48051) SHA1(2d41148d089a75362ed0fde577eca919213ac666) )
	ROM_LOAD_x2( "mpr-14511.ic26", 0x300000, 0x80000, CRC(074c53cc) SHA1(9c89843bbe8058123c25b7f8f86de754ddbca2bb) )
	ROM_LOAD_x2( "mpr-14510.ic22", 0x400000, 0x80000, CRC(5ea6d52d) SHA1(d424082468940bb91ab424ac7812839792ed4e88) )

	ROM_REGION( 0x200000, "slavepcb:gfx1", 0 ) /* tiles */
	ROM_LOAD32_BYTE( "mpr-14496.ic25", 0x000003, 0x080000, CRC(737da16c) SHA1(52247d9bc2924e90d040bef484a541b1f4a9026f) )
	ROM_LOAD32_BYTE( "mpr-14497.ic29", 0x000001, 0x080000, CRC(ebd7ed17) SHA1(2307dc28501965432d2ff55a21698efdce014401) )
	ROM_LOAD32_BYTE( "mpr-14498.ic34", 0x000002, 0x080000, CRC(d4a764bd) SHA1(8434a9225ed1e01e8b1cfe169268e42cd3ce6ee3) )
	ROM_LOAD32_BYTE( "mpr-14499.ic38", 0x000000, 0x080000, CRC(fadc4b2b) SHA1(01c02a4dfad1ab19bac8b81b61d37fdc035bc5c5) )

	ROM_REGION32_BE( 0x800000, "slavepcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-14500.ic24", 0x000007, 0x100000, CRC(0a064e9b) SHA1(264761f4aacaeeac9426528caf180404cd7f6e18) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14501.ic28", 0x000006, 0x100000, CRC(4662bb41) SHA1(80774e680468e9ba9c5dd5eeaa4791fa3b3722fd) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14502.ic33", 0x000005, 0x100000, CRC(988555a9) SHA1(355e44319fd51358329cc7cd226e4c4725e045cb) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14503.ic37", 0x000004, 0x100000, CRC(90556aca) SHA1(24df62af55048db66d50c7034c5460330d231bf5) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14504.ic23", 0x000003, 0x100000, CRC(46dd038d) SHA1(9530a52e2e7388437c20ebcb19bf84c8b3b5086b) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14505.ic27", 0x000002, 0x100000, CRC(be142c1f) SHA1(224631e00c2458c39c6a2ef7978c2b1131fb4da2) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14506.ic32", 0x000001, 0x100000, CRC(5dd8fb6b) SHA1(7d21cacb2c9dba5db2547b6d8e89397e0424ee8e) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14507.ic36", 0x000000, 0x100000, CRC(db3f59ec) SHA1(96dcb3827354773fc2911c62260a27e90dcbe96a) , ROM_SKIP(7) )

ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Alien 3: The Gun (Export)
    not protected

    Sega Game ID codes:
       Game BD: 834-9877-02
    Rom PCB No: 837-9878-02
      Main PCB: 837-7428-03 (SYSTEM 32 COM)
     A/D BD NO. 837-7536
*/
ROM_START( alien3 )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x2( "epr-15943.ic17",     0x000000, 0x040000, CRC(ac4591aa) SHA1(677155a3ebdac6602525e06adb25d287eaf9e089) )
	ROM_LOAD_x2( "epr-15942.ic8",      0x080000, 0x040000, CRC(a1e1d0ec) SHA1(10d8d2235a67a4ba475fe98124c6a4a5311592b5) )
	ROM_LOAD16_BYTE( "mpr-15855.ic18", 0x100000, 0x080000, CRC(a6fadabe) SHA1(328bbb54651eef197ba13f1bd9228f3f4de7ee5e) )
	ROM_LOAD16_BYTE( "mpr-15854.ic9",  0x100001, 0x080000, CRC(d1aec392) SHA1(f48804fe0151e83ad45e912b55db8ae8ddebd2ad) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x4( "epr-15859.ic36", 0x100000, 0x040000, CRC(91b55bd0) SHA1(23b85a006a91c2a5eb1cee14172fd0d8b7732518) )
	ROM_LOAD( "mpr-15858.ic35",    0x200000, 0x100000, CRC(2eb64c10) SHA1(b2dbe86b82e889f4a9850cf4aa6596a139c1c3d6) )
	ROM_LOAD( "mpr-15857.ic34",    0x300000, 0x100000, CRC(915c56df) SHA1(7031f937c826af17caf7ec8cbb6155d0a55bd38a) )
	ROM_LOAD( "mpr-15856.ic24",    0x400000, 0x100000, CRC(a5ef4f1f) SHA1(e8da7a995955e80872a25bd75465c590b649cfab) )

	ROM_REGION( 0x400000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-15863.ic14", 0x000000, 0x200000, CRC(9d36b645) SHA1(2977047780b615b64c3b4aec78fef0643d40490e) )
	ROM_LOAD16_BYTE( "mpr-15862.ic5",  0x000001, 0x200000, CRC(9e277d25) SHA1(9f191484a42391268306a8d2d95c340ce8b2d6cd) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-15864.ic32", 0x000000, 0x200000, CRC(58207157) SHA1(d1b0c7edac8b89b1322398d4cd3a976a88bc0b56) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15866.ic30", 0x000002, 0x200000, CRC(9c53732c) SHA1(9aa5103cc10b4927c16e0cf102b64a15dd038756) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15868.ic28", 0x000004, 0x200000, CRC(62d556e8) SHA1(d70cab0881784a3d4dd06d0c99587ca6054c2dc4) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15870.ic26", 0x000006, 0x200000, CRC(d31c0400) SHA1(44c1b2e5236d894d31ff72552a7ad50270dd2fad) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15865.ic31", 0x800000, 0x200000, CRC(dd64f87b) SHA1(cfa96c5f2b1221706552f5cef4aa7c61ebe21e39) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15867.ic29", 0x800002, 0x200000, CRC(8cf9cb11) SHA1(a77399fccee3f258a5716721edd69a33f94f8daf) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15869.ic27", 0x800004, 0x200000, CRC(dd4b137f) SHA1(7316dce32d35bf468defae5e6ed86910a37a2457) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15871.ic25", 0x800006, 0x200000, CRC(58eb10ae) SHA1(23f2a72dc7b2d7b5c8a979952f81608296805745) , ROM_SKIP(6)|ROM_GROUPWORD )

	ROM_REGION16_BE( 0x80, "mainpcb:eeprom", 0 )
	ROM_LOAD16_WORD( "93c45_eeprom.ic76", 0x0000, 0x0080, CRC(6e1d9df3) SHA1(2fd818bc393fb96e945fa37a63c8a3c4aff2f79f) )
ROM_END

/**************************************************************************************************************************
    Alien 3: The Gun (US)
    not protected

    Sega Game ID codes:
       Game BD: 834-9877-01
    Rom PCB No: 837-9878-01
      Main PCB: 837-7428-03 (SYSTEM 32 COM)
     A/D BD NO. 837-7536
*/
ROM_START( alien3u )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x2( "epr-15941.ic17",     0x000000, 0x040000, CRC(bf8c257f) SHA1(d08b77d2e3f5af0da7e7d8727fbe7fc0eb1153ff) )
	ROM_LOAD_x2( "epr-15940a.ic8",     0x080000, 0x040000, CRC(8840b51e) SHA1(0aa6945000676b1adc535b1557a1455d62aed9f5) )
	ROM_LOAD16_BYTE( "mpr-15855.ic18", 0x100000, 0x080000, CRC(a6fadabe) SHA1(328bbb54651eef197ba13f1bd9228f3f4de7ee5e) )
	ROM_LOAD16_BYTE( "mpr-15854.ic9",  0x100001, 0x080000, CRC(d1aec392) SHA1(f48804fe0151e83ad45e912b55db8ae8ddebd2ad) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x4( "epr-15859.ic36", 0x100000, 0x040000, CRC(91b55bd0) SHA1(23b85a006a91c2a5eb1cee14172fd0d8b7732518) )
	ROM_LOAD( "mpr-15858.ic35",    0x200000, 0x100000, CRC(2eb64c10) SHA1(b2dbe86b82e889f4a9850cf4aa6596a139c1c3d6) )
	ROM_LOAD( "mpr-15857.ic34",    0x300000, 0x100000, CRC(915c56df) SHA1(7031f937c826af17caf7ec8cbb6155d0a55bd38a) )
	ROM_LOAD( "mpr-15856.ic24",    0x400000, 0x100000, CRC(a5ef4f1f) SHA1(e8da7a995955e80872a25bd75465c590b649cfab) )

	ROM_REGION( 0x400000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-15863.ic14", 0x000000, 0x200000, CRC(9d36b645) SHA1(2977047780b615b64c3b4aec78fef0643d40490e) )
	ROM_LOAD16_BYTE( "mpr-15862.ic5",  0x000001, 0x200000, CRC(9e277d25) SHA1(9f191484a42391268306a8d2d95c340ce8b2d6cd) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-15864.ic32", 0x000000, 0x200000, CRC(58207157) SHA1(d1b0c7edac8b89b1322398d4cd3a976a88bc0b56) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15866.ic30", 0x000002, 0x200000, CRC(9c53732c) SHA1(9aa5103cc10b4927c16e0cf102b64a15dd038756) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15868.ic28", 0x000004, 0x200000, CRC(62d556e8) SHA1(d70cab0881784a3d4dd06d0c99587ca6054c2dc4) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15870.ic26", 0x000006, 0x200000, CRC(d31c0400) SHA1(44c1b2e5236d894d31ff72552a7ad50270dd2fad) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15865.ic31", 0x800000, 0x200000, CRC(dd64f87b) SHA1(cfa96c5f2b1221706552f5cef4aa7c61ebe21e39) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15867.ic29", 0x800002, 0x200000, CRC(8cf9cb11) SHA1(a77399fccee3f258a5716721edd69a33f94f8daf) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15869.ic27", 0x800004, 0x200000, CRC(dd4b137f) SHA1(7316dce32d35bf468defae5e6ed86910a37a2457) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15871.ic25", 0x800006, 0x200000, CRC(58eb10ae) SHA1(23f2a72dc7b2d7b5c8a979952f81608296805745) , ROM_SKIP(6)|ROM_GROUPWORD )

	ROM_REGION16_BE( 0x80, "mainpcb:eeprom", 0 )
	ROM_LOAD16_WORD( "93c45_eeprom.ic76", 0x0000, 0x0080, CRC(6e1d9df3) SHA1(2fd818bc393fb96e945fa37a63c8a3c4aff2f79f) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Arabian Fight (Export)
    protected via a custom V25 with encrypted code
*/
ROM_START( arabfgt )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x8( "epr-14609.ic8",         0x000000, 0x020000, CRC(6a43c7fb) SHA1(70e9f9fa5f867f0455d62ff2690ad19055d79363) )
	ROM_LOAD16_BYTE_x2( "epr-14592.ic18", 0x100000, 0x040000, CRC(f7dff316) SHA1(338690a1404dde6e7e66067f23605a247c7d0f5b) )
	ROM_LOAD16_BYTE_x2( "epr-14591.ic9",  0x100001, 0x040000, CRC(bbd940fb) SHA1(99c17aba890935eaf7ea468492da03103288eb1b) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU + banks */
	ROM_LOAD_x8( "epr-14596.ic36",  0x100000, 0x020000, CRC(bd01faec) SHA1(c909dcb8ef2672c4b0060d911d295e445ca311eb) )
	ROM_LOAD( "mpr-14595f.ic35",    0x200000, 0x100000, CRC(5173d1af) SHA1(dccda644488d0c561c8ff7fa9619bd9504d8d9c6) )
	ROM_LOAD( "mpr-14594f.ic34",    0x300000, 0x100000, CRC(01777645) SHA1(7bcbe7687bd80b94bd3b2b3099cdd036bf7e0cd3) )
	ROM_LOAD( "mpr-14593f.ic24",    0x400000, 0x100000, CRC(aa037047) SHA1(5cb1cfb235bbbf875d2b07ac4a9130ba13d47e57) )

	ROM_REGION( 0x100000, "cpu2", 0 ) /* Protection CPU */
	ROM_LOAD( "epr-14468-01.u3", 0x00000, 0x10000, CRC(c3c591e4) SHA1(53e48066e85b61d0c456618d14334a509b354cb3) )
	ROM_RELOAD(                  0xf0000, 0x10000)

	ROM_REGION( 0x400000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-14599f.ic14", 0x000000, 0x200000, CRC(94f1cf10) SHA1(34ec86487bcb6726c025149c319f00a854eb7a1d) )
	ROM_LOAD16_BYTE( "mpr-14598f.ic5",  0x000001, 0x200000, CRC(010656f3) SHA1(31619c022cba4f250ce174f186d3e34444f60faf) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-14600f.ic32", 0x000000, 0x200000, CRC(e860988a) SHA1(328581877c0890519c854f75f0976b0e9c4560f8) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14602.ic30",  0x000002, 0x200000, CRC(64524e4d) SHA1(86246185ab5ab638a73991c9e3aeb07c6d51be4f) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14604.ic28",  0x000004, 0x200000, CRC(5f8d5167) SHA1(1b08495e5a4cc2530c2895e47abd0e0b75496c68) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14606.ic26",  0x000006, 0x200000, CRC(7047f437) SHA1(e806a1cd73c96b33e8edc64e41d99bf7798103e0) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14601f.ic31", 0x800000, 0x200000, CRC(a2f3bb32) SHA1(1a60975dead5faf08ad4e9a96a00f98664d5e5ec) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14603.ic29",  0x800002, 0x200000, CRC(f6ce494b) SHA1(b3117e34913e855c035ebe37fbfbe0f7466f94f0) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14605.ic27",  0x800004, 0x200000, CRC(aaf52697) SHA1(b502a37ae68fc08b60cdf0e2b744898b3474d3b9) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14607.ic25",  0x800006, 0x200000, CRC(b70b0735) SHA1(9ef2da6f710bc5c2c7ee30dc144409a61dbe6646) , ROM_SKIP(6)|ROM_GROUPWORD )
ROM_END

/**************************************************************************************************************************
    Arabian Fight (US)
    protected via a custom V25 with encrypted code
*/
ROM_START( arabfgtu )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x8( "epr-14608.ic8",         0x000000, 0x020000, CRC(cd5efba9) SHA1(a7daf8e95d31359753c984c447e93d40f43a179d) )
	ROM_LOAD16_BYTE_x2( "epr-14592.ic18", 0x100000, 0x040000, CRC(f7dff316) SHA1(338690a1404dde6e7e66067f23605a247c7d0f5b) )
	ROM_LOAD16_BYTE_x2( "epr-14591.ic9",  0x100001, 0x040000, CRC(bbd940fb) SHA1(99c17aba890935eaf7ea468492da03103288eb1b) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU + banks */
	ROM_LOAD_x8( "epr-14596.ic36",  0x100000, 0x020000, CRC(bd01faec) SHA1(c909dcb8ef2672c4b0060d911d295e445ca311eb) )
	ROM_LOAD( "mpr-14595f.ic35",    0x200000, 0x100000, CRC(5173d1af) SHA1(dccda644488d0c561c8ff7fa9619bd9504d8d9c6) )
	ROM_LOAD( "mpr-14594f.ic34",    0x300000, 0x100000, CRC(01777645) SHA1(7bcbe7687bd80b94bd3b2b3099cdd036bf7e0cd3) )
	ROM_LOAD( "mpr-14593f.ic24",    0x400000, 0x100000, CRC(aa037047) SHA1(5cb1cfb235bbbf875d2b07ac4a9130ba13d47e57) )

	ROM_REGION( 0x100000, "cpu2", 0 ) /* Protection CPU */
	ROM_LOAD( "epr-14468-01.u3", 0x00000, 0x10000, CRC(c3c591e4) SHA1(53e48066e85b61d0c456618d14334a509b354cb3) )
	ROM_RELOAD(                  0xf0000, 0x10000)

	ROM_REGION( 0x400000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-14599f.ic14", 0x000000, 0x200000, CRC(94f1cf10) SHA1(34ec86487bcb6726c025149c319f00a854eb7a1d) )
	ROM_LOAD16_BYTE( "mpr-14598f.ic5",  0x000001, 0x200000, CRC(010656f3) SHA1(31619c022cba4f250ce174f186d3e34444f60faf) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-14600f.ic32", 0x000000, 0x200000, CRC(e860988a) SHA1(328581877c0890519c854f75f0976b0e9c4560f8) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14602.ic30",  0x000002, 0x200000, CRC(64524e4d) SHA1(86246185ab5ab638a73991c9e3aeb07c6d51be4f) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14604.ic28",  0x000004, 0x200000, CRC(5f8d5167) SHA1(1b08495e5a4cc2530c2895e47abd0e0b75496c68) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14606.ic26",  0x000006, 0x200000, CRC(7047f437) SHA1(e806a1cd73c96b33e8edc64e41d99bf7798103e0) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14601f.ic31", 0x800000, 0x200000, CRC(a2f3bb32) SHA1(1a60975dead5faf08ad4e9a96a00f98664d5e5ec) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14603.ic29",  0x800002, 0x200000, CRC(f6ce494b) SHA1(b3117e34913e855c035ebe37fbfbe0f7466f94f0) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14605.ic27",  0x800004, 0x200000, CRC(aaf52697) SHA1(b502a37ae68fc08b60cdf0e2b744898b3474d3b9) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14607.ic25",  0x800006, 0x200000, CRC(b70b0735) SHA1(9ef2da6f710bc5c2c7ee30dc144409a61dbe6646) , ROM_SKIP(6)|ROM_GROUPWORD )
ROM_END

/**************************************************************************************************************************
    Arabian Fight (Japan)
    protected via a custom V25 with encrypted code
*/
ROM_START( arabfgtj )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x8( "epr-14597.ic8",         0x000000, 0x020000, CRC(7a6fe222) SHA1(f730e9d44ad20dbaa59f6311a077c889e0aec8e4) )
	ROM_LOAD16_BYTE_x2( "epr-14592.ic18", 0x100000, 0x040000, CRC(f7dff316) SHA1(338690a1404dde6e7e66067f23605a247c7d0f5b) )
	ROM_LOAD16_BYTE_x2( "epr-14591.ic9",  0x100001, 0x040000, CRC(bbd940fb) SHA1(99c17aba890935eaf7ea468492da03103288eb1b) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU + banks */
	ROM_LOAD_x8( "epr-14596.ic36",  0x100000, 0x020000, CRC(bd01faec) SHA1(c909dcb8ef2672c4b0060d911d295e445ca311eb) )
	ROM_LOAD( "mpr-14595f.ic35",    0x200000, 0x100000, CRC(5173d1af) SHA1(dccda644488d0c561c8ff7fa9619bd9504d8d9c6) )
	ROM_LOAD( "mpr-14594f.ic34",    0x300000, 0x100000, CRC(01777645) SHA1(7bcbe7687bd80b94bd3b2b3099cdd036bf7e0cd3) )
	ROM_LOAD( "mpr-14593f.ic24",    0x400000, 0x100000, CRC(aa037047) SHA1(5cb1cfb235bbbf875d2b07ac4a9130ba13d47e57) )

	ROM_REGION( 0x100000, "cpu2", 0 ) /* Protection CPU */
	ROM_LOAD( "epr-14468-01.u3", 0x00000, 0x10000, CRC(c3c591e4) SHA1(53e48066e85b61d0c456618d14334a509b354cb3) )
	ROM_RELOAD(                  0xf0000, 0x10000)

	ROM_REGION( 0x400000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-14599f.ic14", 0x000000, 0x200000, CRC(94f1cf10) SHA1(34ec86487bcb6726c025149c319f00a854eb7a1d) )
	ROM_LOAD16_BYTE( "mpr-14598f.ic5",  0x000001, 0x200000, CRC(010656f3) SHA1(31619c022cba4f250ce174f186d3e34444f60faf) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-14600f.ic32", 0x000000, 0x200000, CRC(e860988a) SHA1(328581877c0890519c854f75f0976b0e9c4560f8) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14602.ic30",  0x000002, 0x200000, CRC(64524e4d) SHA1(86246185ab5ab638a73991c9e3aeb07c6d51be4f) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14604.ic28",  0x000004, 0x200000, CRC(5f8d5167) SHA1(1b08495e5a4cc2530c2895e47abd0e0b75496c68) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14606.ic26",  0x000006, 0x200000, CRC(7047f437) SHA1(e806a1cd73c96b33e8edc64e41d99bf7798103e0) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14601f.ic31", 0x800000, 0x200000, CRC(a2f3bb32) SHA1(1a60975dead5faf08ad4e9a96a00f98664d5e5ec) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14603.ic29",  0x800002, 0x200000, CRC(f6ce494b) SHA1(b3117e34913e855c035ebe37fbfbe0f7466f94f0) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14605.ic27",  0x800004, 0x200000, CRC(aaf52697) SHA1(b502a37ae68fc08b60cdf0e2b744898b3474d3b9) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14607.ic25",  0x800006, 0x200000, CRC(b70b0735) SHA1(9ef2da6f710bc5c2c7ee30dc144409a61dbe6646) , ROM_SKIP(6)|ROM_GROUPWORD )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Burning Rival (Export)
    protected via FD1149 317-0212
*/
ROM_START( brival )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x8( "epr-15722.ic8",      0x000000, 0x020000, CRC(138141c0) SHA1(aca2a46ee1008e91c65a09e79c76d5175e7df5e8) )
	ROM_LOAD16_BYTE( "epr-15723.ic18", 0x100000, 0x080000, CRC(4ff40d39) SHA1(b33a656f976ec7a1a2268e7b9a81d5b84f3d9ca3) )
	ROM_LOAD16_BYTE( "epr-15724.ic9",  0x100001, 0x080000, CRC(3ff8a052) SHA1(f484a8e15a022f9ff290e662ab27f96f9f0ad24e) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x8( "epr-15725.ic36", 0x100000, 0x020000, CRC(ea1407d7) SHA1(68b571341f032278e87a38739ba8084b7a6062d3) )
	ROM_LOAD( "mpr-15627.ic35",    0x200000, 0x100000, CRC(8a8388c5) SHA1(7ee03feb975cc576a3d8651fd41976ca87d60894) )
	ROM_LOAD( "mpr-15626.ic34",    0x300000, 0x100000, CRC(83306d1e) SHA1(feb08902b51c0013d9417832cdf198e36cdfc28c) )
	ROM_LOAD( "mpr-15625.ic24",    0x400000, 0x100000, CRC(3ce82932) SHA1(f2107bc2591f46a51c9f0d706933b1ae69db91f9) )

	ROM_REGION( 0x400000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-15629.ic14", 0x000000, 0x200000, CRC(2c8dd96d) SHA1(4a42a30485c19eb4f4a9d518a3dff3ae11911d01) )
	ROM_LOAD16_BYTE( "mpr-15628.ic5",  0x000001, 0x200000, CRC(58d4ca40) SHA1(b1633acc803bba7e8283a9663b49abeda662a74d) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-15637.ic32", 0x000000, 0x200000, CRC(b6cf2f05) SHA1(a308d40ce5165e03fccf7fcd615ee111f7840fdc) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15635.ic30", 0x000002, 0x200000, CRC(70f2eb2b) SHA1(9868c8b0dd8ce810a0e32f51e702eee7e1c9a967) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15633.ic28", 0x000004, 0x200000, CRC(005dfed5) SHA1(f555620d75d3886a890307be9df9c0879bcda695) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15631.ic26", 0x000006, 0x200000, CRC(c35e2f21) SHA1(37935aa2eaa1769e57fb58f47f9797ae153d7496) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15636.ic31", 0x800000, 0x200000, CRC(d81ca97b) SHA1(a8e64e6cbe822f18ce20f50c8ddb8f1d5ed8b783) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15634.ic29", 0x800002, 0x200000, CRC(b0c6c52a) SHA1(04dd7344ca82e38f9d796a764c9e5a631a89aaac) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15632.ic27", 0x800004, 0x200000, CRC(8476e52b) SHA1(e89748d34febcaf362580cdae30a5c570e56899a) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15630.ic25", 0x800006, 0x200000, CRC(bf7dd2f6) SHA1(ab3fbe9e2b9b57424fb2a147f32b0f573c0b11b8) , ROM_SKIP(6)|ROM_GROUPWORD )
ROM_END

/**************************************************************************************************************************
    Burning Rival (Japan)
    protected via FD1149 317-0212
*/
ROM_START( brivalj )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x8( "epr-15720.ic8",      0x000000, 0x020000, CRC(0d182d78) SHA1(53e9e5898869ea4a354dc3e9a78d8b8e9a7274c9) )
	ROM_LOAD16_BYTE( "epr-15723.ic18", 0x100000, 0x080000, CRC(4ff40d39) SHA1(b33a656f976ec7a1a2268e7b9a81d5b84f3d9ca3) )
	ROM_LOAD16_BYTE( "epr-15724.ic9",  0x100001, 0x080000, CRC(3ff8a052) SHA1(f484a8e15a022f9ff290e662ab27f96f9f0ad24e) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x8( "epr-15725.ic36", 0x100000, 0x020000, CRC(ea1407d7) SHA1(68b571341f032278e87a38739ba8084b7a6062d3) )
	ROM_LOAD( "mpr-15627.ic35",    0x200000, 0x100000, CRC(8a8388c5) SHA1(7ee03feb975cc576a3d8651fd41976ca87d60894) )
	ROM_LOAD( "mpr-15626.ic34",    0x300000, 0x100000, CRC(83306d1e) SHA1(feb08902b51c0013d9417832cdf198e36cdfc28c) )
	ROM_LOAD( "mpr-15625.ic24",    0x400000, 0x100000, CRC(3ce82932) SHA1(f2107bc2591f46a51c9f0d706933b1ae69db91f9) )

	/* the 10 roms below may be bad dumps ... mp14598 / 99 have corrupt tiles when compared to the roms
	   in the parent set, but Sega did change the part numbers so they might be correct, the others
	   are suspicious, the changes are very similar but the part numbers haven't changed.  We really
	   need a 3rd board to verify */
	ROM_REGION( 0x400000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-14599f.ic14", 0x000000, 0x200000, CRC(1de17e83) SHA1(04ee14b863f93b42a5bd1b6da71cff54ef11d4b7) ) /* Rom # matches tile rom # from Arabian Fight ??? */
	ROM_LOAD16_BYTE( "mpr-14598f.ic5",  0x000001, 0x200000, CRC(cafb0de9) SHA1(94c6bfc7a4081dee373e9466a7b6f80889696087) ) /* Rom # matchrs tile rom # from Arabian Fight ??? */

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "brivalj_mp15637.32", 0x000000, 0x200000, CRC(f39844c0) SHA1(c48dc8cccdd9d3756cf99a983c6a89ed43fcda22) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "brivalj_mp15635.30", 0x000002, 0x200000, CRC(263cf6d1) SHA1(7accd214502fd050edc0901c9929d6069dae4d00) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "brivalj_mp15633.28", 0x000004, 0x200000, CRC(44e9a88b) SHA1(57a930b9c3b83c889df54de60c90f847c2dcb614) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "brivalj_mp15631.26", 0x000006, 0x200000, CRC(e93cf9c9) SHA1(17786cd3ccaef613216db724e923861841c52b45) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "brivalj_mp15636.31", 0x800000, 0x200000, CRC(079ff77f) SHA1(bdd41acef58c39ba58cf85d307229622877dbdf9) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "brivalj_mp15634.29", 0x800002, 0x200000, CRC(1edc14cd) SHA1(80a281c904560b364fe9f2b8987b7a254220a29f) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "brivalj_mp15632.27", 0x800004, 0x200000, CRC(796215f2) SHA1(d7b393781dbba59c9b1cd600d27e6d91e36ea771) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "brivalj_mp15630.25", 0x800006, 0x200000, CRC(8dabb501) SHA1(c5af2187d00e0b9732a82441f9758b303fecbb2c) , ROM_SKIP(6)|ROM_GROUPWORD )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Dark Edge (Export)
    protected via FD1149 317-0204
*/
ROM_START( darkedge )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x2( "epr-15246.ic8", 0x000000, 0x080000, CRC(c0bdceeb) SHA1(9cf670cf9a8691f259c75c1d9c6cb14e8a70bb72) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x8( "epr-15243.ic36", 0x100000, 0x020000, CRC(08ca5f11) SHA1(c2c48d2f02770941a93794f82cb407d6264904d2) )
	ROM_LOAD( "mpr-15242.ic35",    0x200000, 0x100000, CRC(ffb7d917) SHA1(bfeae1a2bd7250edb695b7034f6b1f851f6fd48a) )
	ROM_LOAD( "mpr-15241.ic34",    0x300000, 0x100000, CRC(8eccc4fe) SHA1(119724b9b6d2b51ad4f065ebf74d200960090e68) )
	ROM_LOAD( "mpr-15240.ic24",    0x400000, 0x100000, CRC(867d59e8) SHA1(fb1c0d26dbb1bde9d8bc86419cd911b8e37bf923) )

	ROM_REGION( 0x100000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-15248.ic14", 0x000000, 0x080000, CRC(185b308b) SHA1(a49c1b752b3c4355560e0cd712fb9a096140e37b) )
	ROM_LOAD16_BYTE( "mpr-15247.ic5",  0x000001, 0x080000, CRC(be21548c) SHA1(2e315aadc2a0b781c3ee3fe71c75eb1f43514eff) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-15249.ic32", 0x000000, 0x200000, CRC(2b4371a8) SHA1(47f448bfbc068f2d0cdedd81bcd280823d5758a3) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15251.ic30", 0x000002, 0x200000, CRC(efe2d689) SHA1(af22153ea3afdde3732f881087c642170f91d745) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15253.ic28", 0x000004, 0x200000, CRC(8356ed01) SHA1(a28747813807361c7d0c722a94e194caea8bfab6) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15255.ic26", 0x000006, 0x200000, CRC(ff04a5b0) SHA1(d4548f9da014ba5249c2f75d654a2a88c095aaf8) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15250.ic31", 0x800000, 0x200000, CRC(c5cab71a) SHA1(111c69c40a39c3fceef38f5876e1dcf7ac2fbee2) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15252.ic29", 0x800002, 0x200000, CRC(f8885fea) SHA1(ef944df5f6fd64813734056ad2a150f518c75459) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15254.ic27", 0x800004, 0x200000, CRC(7765424b) SHA1(7cd4c275f6333beeea62dd65a769e11650c68923) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15256.ic25", 0x800006, 0x200000, CRC(44c36b62) SHA1(4c7f2cc4347ef2126dcbf43e8dce8500e52b5f8e) , ROM_SKIP(6)|ROM_GROUPWORD )
ROM_END

/**************************************************************************************************************************
    Dark Edge (Japan)
    protected via FD1149 317-0204
*/
ROM_START( darkedgej )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x2( "epr-15244.ic8", 0x000000, 0x080000, CRC(0db138cb) SHA1(79ccb754e0d816b395b536a6d9c5a6e93168a913) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x8( "epr-15243.ic36", 0x100000, 0x020000, CRC(08ca5f11) SHA1(c2c48d2f02770941a93794f82cb407d6264904d2) )
	ROM_LOAD( "mpr-15242.ic35",    0x200000, 0x100000, CRC(ffb7d917) SHA1(bfeae1a2bd7250edb695b7034f6b1f851f6fd48a) )
	ROM_LOAD( "mpr-15241.ic34",    0x300000, 0x100000, CRC(8eccc4fe) SHA1(119724b9b6d2b51ad4f065ebf74d200960090e68) )
	ROM_LOAD( "mpr-15240.ic24",    0x400000, 0x100000, CRC(867d59e8) SHA1(fb1c0d26dbb1bde9d8bc86419cd911b8e37bf923) )

	ROM_REGION( 0x100000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-15248.ic14", 0x000000, 0x080000, CRC(185b308b) SHA1(a49c1b752b3c4355560e0cd712fb9a096140e37b) )
	ROM_LOAD16_BYTE( "mpr-15247.ic5",  0x000001, 0x080000, CRC(be21548c) SHA1(2e315aadc2a0b781c3ee3fe71c75eb1f43514eff) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-15249.ic32", 0x000000, 0x200000, CRC(2b4371a8) SHA1(47f448bfbc068f2d0cdedd81bcd280823d5758a3) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15251.ic30", 0x000002, 0x200000, CRC(efe2d689) SHA1(af22153ea3afdde3732f881087c642170f91d745) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15253.ic28", 0x000004, 0x200000, CRC(8356ed01) SHA1(a28747813807361c7d0c722a94e194caea8bfab6) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15255.ic26", 0x000006, 0x200000, CRC(ff04a5b0) SHA1(d4548f9da014ba5249c2f75d654a2a88c095aaf8) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15250.ic31", 0x800000, 0x200000, CRC(c5cab71a) SHA1(111c69c40a39c3fceef38f5876e1dcf7ac2fbee2) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15252.ic29", 0x800002, 0x200000, CRC(f8885fea) SHA1(ef944df5f6fd64813734056ad2a150f518c75459) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15254.ic27", 0x800004, 0x200000, CRC(7765424b) SHA1(7cd4c275f6333beeea62dd65a769e11650c68923) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15256.ic25", 0x800006, 0x200000, CRC(44c36b62) SHA1(4c7f2cc4347ef2126dcbf43e8dce8500e52b5f8e) , ROM_SKIP(6)|ROM_GROUPWORD )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Dragon Ball Z, VRVS
    protected via FD1149 317-0215/0217
*/
ROM_START( dbzvrvs )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD( "16543",   0x000000, 0x080000, CRC(7b9bc6f5) SHA1(556fd8471bf471e41fc6a50471c2be1bd6b98697) )
	ROM_LOAD( "16542.a", 0x080000, 0x080000, CRC(6449ab22) SHA1(03e6cdacf77f2ff80dd6798094deac5486f2c840) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x4( "16541", 0x100000, 0x040000, CRC(1d61d836) SHA1(c6b1b54d41d2650abeaf69a31aa76c4462531880) )
	ROM_LOAD( "16540",    0x200000, 0x100000, CRC(b6f9bb43) SHA1(823f29a2fc4b9315e8c58616dbd095d45d366c8b) )
	ROM_LOAD( "16539",    0x300000, 0x100000, CRC(38c26418) SHA1(2442933e13c83209e904c1dec677aeda91b75290) )
	ROM_LOAD( "16538",    0x400000, 0x100000, CRC(4d402c31) SHA1(2df160fd7e70f3d7b52fef2a2082e68966fd1535) )

	ROM_REGION( 0x200000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "16545", 0x000000, 0x100000, CRC(51748bac) SHA1(b1cae16b62a8d29117c0adb140eb09c1092f6c37) )
	ROM_LOAD16_BYTE( "16544", 0x000001, 0x100000, CRC(f6c93dfc) SHA1(a006cedb7d0151ccc8d22e6588b1c39e099da182) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "16546", 0x000000, 0x200000, CRC(96f4be31) SHA1(ce3281630180d91de7850e9b1062382817fe0b1d) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "16548", 0x000002, 0x200000, CRC(00377f59) SHA1(cf0f808d7730f334c5ac80d3171fa457be9ac88e) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "16550", 0x000004, 0x200000, CRC(168e8966) SHA1(a18ec30f1358b09bcde6d8d2dbe0a82bea3bdae9) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "16552", 0x000006, 0x200000, CRC(a31dae31) SHA1(2da2c391f29b5fdb87e3f95d9dabd50370fafa5a) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "16547", 0x800000, 0x200000, CRC(50d328ed) SHA1(c4795299f5d7c9f3a847d684d8cde7012d4486f0) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "16549", 0x800002, 0x200000, CRC(a5802e9f) SHA1(4cec3ed85a21aaf99b73013795721f212019e619) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "16551", 0x800004, 0x200000, CRC(dede05fc) SHA1(51e092579e2b81fb68a9cc54165f80026fe71796) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "16553", 0x800006, 0x200000, CRC(c0a43009) SHA1(e4f73768de512046b3e25c4238da811dcc2dde0b) , ROM_SKIP(6)|ROM_GROUPWORD )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    F1 Exhaust Note (Export)
    to display the title screen this will require 2 linked system32 boards to be emulated

    Sega Game ID codes:
     GAME BD NO. 833-8389-02 EXHAUST NOTE
         ROM BD. 834-8439-02
        MAIN BD. 837-7428 (SYSTEM 32 COM)
    Link PCB NO. 837-8223-01
      A/D BD NO. 837-7536
*/
ROM_START( f1en )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x8( "epr-14452a.ic6",        0x000000, 0x020000, CRC(b5b4a9d9) SHA1(6699c15dc1155c3cee33a06d320acbff0ab5ad11) )
	ROM_LOAD16_BYTE_x2( "epr-14445.ic14", 0x100000, 0x040000, CRC(d06261ab) SHA1(6e1c4ce4e49a142fd5b1ecac98145960d7afd567) )
	ROM_LOAD16_BYTE_x2( "epr-14444.ic7",  0x100001, 0x040000, CRC(07724354) SHA1(9d7f64a80553c4ae0e9cf716478fd5c4b8277470) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x8( "epr-14449.ic35", 0x100000, 0x020000, CRC(2d29699c) SHA1(cae02e5533a0edd3b3b4a54a1a43321285e06416) )
	ROM_LOAD_x2( "epr-14448.ic31", 0x200000, 0x080000, CRC(87ca1e8d) SHA1(739274171c13983a60d061176095645419dade49) )
	ROM_LOAD_x2( "epr-14447.ic26", 0x300000, 0x080000, CRC(db1cfcbd) SHA1(c76eb2ced5571a548ad00709097dd272747127a2) )
	ROM_LOAD_x2( "epr-14446.ic22", 0x400000, 0x080000, CRC(646ec2cb) SHA1(67e453f128ae227e22c68f55d0d3f5831fbeb2f9) )

	ROM_REGION( 0x100000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD32_BYTE( "mpr-14362", 0x000000, 0x040000, CRC(fb1c4e79) SHA1(38ee23763b9e5bb62bbc54cab95041415404f0c4) )
	ROM_LOAD32_BYTE( "mpr-14361", 0x000002, 0x040000, CRC(e3204bda) SHA1(34157e80edd6d685bd5a5e23b1e0130a5f3d138a) )
	ROM_LOAD32_BYTE( "mpr-14360", 0x000001, 0x040000, CRC(c5e8da79) SHA1(662a6c146fe3d0b8763d845379c06d0ee6ced1ed) )
	ROM_LOAD32_BYTE( "mpr-14359", 0x000003, 0x040000, CRC(70305c68) SHA1(7a6a1bf7381eba8cc1c3897497b32ca63316972a) )

	ROM_REGION32_BE( 0x800000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-14370", 0x000000, 0x080000, CRC(fda78289) SHA1(3740affdcc738c50d07ff3e5b592bdf8a8b6be15) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14369", 0x000001, 0x080000, CRC(7765116d) SHA1(9493148aa84adc90143cf638265d4c55bfb43990) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14368", 0x000002, 0x080000, CRC(5744a30e) SHA1(98544fb234a8e93716e951d5414a490845e213c5) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14367", 0x000003, 0x080000, CRC(77bb9003) SHA1(6370fdeab4967976840d752577cd860b9ce8efca) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14366", 0x000004, 0x080000, CRC(21078e83) SHA1(f35f643c28aad3bf18cb9906b114c4f49b7b4cd1) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14365", 0x000005, 0x080000, CRC(36913790) SHA1(4a447cffb44b023fe1441277db1e411d4cd119eb) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14364", 0x000006, 0x080000, CRC(0fa12ecd) SHA1(6a34c7718edffbeddded8786e11cac181b485ebd) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14363", 0x000007, 0x080000, CRC(f3427a56) SHA1(6a99d7432dfff35470ddcca5cfde36689a77e706) , ROM_SKIP(7) )

	ROM_REGION( 0x200000, "slavepcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x8( "epr-14452a.ic6",        0x000000, 0x020000, CRC(b5b4a9d9) SHA1(6699c15dc1155c3cee33a06d320acbff0ab5ad11) )
	ROM_LOAD16_BYTE_x2( "epr-14445.ic14", 0x100000, 0x040000, CRC(d06261ab) SHA1(6e1c4ce4e49a142fd5b1ecac98145960d7afd567) )
	ROM_LOAD16_BYTE_x2( "epr-14444.ic7",  0x100001, 0x040000, CRC(07724354) SHA1(9d7f64a80553c4ae0e9cf716478fd5c4b8277470) )

	ROM_REGION( 0x500000, "slavepcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x8( "epr-14449.ic35", 0x100000, 0x020000, CRC(2d29699c) SHA1(cae02e5533a0edd3b3b4a54a1a43321285e06416) )
	ROM_LOAD_x2( "epr-14448.ic31", 0x200000, 0x080000, CRC(87ca1e8d) SHA1(739274171c13983a60d061176095645419dade49) )
	ROM_LOAD_x2( "epr-14447.ic26", 0x300000, 0x080000, CRC(db1cfcbd) SHA1(c76eb2ced5571a548ad00709097dd272747127a2) )
	ROM_LOAD_x2( "epr-14446.ic22", 0x400000, 0x080000, CRC(646ec2cb) SHA1(67e453f128ae227e22c68f55d0d3f5831fbeb2f9) )

	ROM_REGION( 0x100000, "slavepcb:gfx1", 0 ) /* tiles */
	ROM_LOAD32_BYTE( "mpr-14362", 0x000000, 0x040000, CRC(fb1c4e79) SHA1(38ee23763b9e5bb62bbc54cab95041415404f0c4) )
	ROM_LOAD32_BYTE( "mpr-14361", 0x000002, 0x040000, CRC(e3204bda) SHA1(34157e80edd6d685bd5a5e23b1e0130a5f3d138a) )
	ROM_LOAD32_BYTE( "mpr-14360", 0x000001, 0x040000, CRC(c5e8da79) SHA1(662a6c146fe3d0b8763d845379c06d0ee6ced1ed) )
	ROM_LOAD32_BYTE( "mpr-14359", 0x000003, 0x040000, CRC(70305c68) SHA1(7a6a1bf7381eba8cc1c3897497b32ca63316972a) )

	ROM_REGION32_BE( 0x800000, "slavepcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-14370", 0x000000, 0x080000, CRC(fda78289) SHA1(3740affdcc738c50d07ff3e5b592bdf8a8b6be15) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14369", 0x000001, 0x080000, CRC(7765116d) SHA1(9493148aa84adc90143cf638265d4c55bfb43990) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14368", 0x000002, 0x080000, CRC(5744a30e) SHA1(98544fb234a8e93716e951d5414a490845e213c5) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14367", 0x000003, 0x080000, CRC(77bb9003) SHA1(6370fdeab4967976840d752577cd860b9ce8efca) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14366", 0x000004, 0x080000, CRC(21078e83) SHA1(f35f643c28aad3bf18cb9906b114c4f49b7b4cd1) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14365", 0x000005, 0x080000, CRC(36913790) SHA1(4a447cffb44b023fe1441277db1e411d4cd119eb) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14364", 0x000006, 0x080000, CRC(0fa12ecd) SHA1(6a34c7718edffbeddded8786e11cac181b485ebd) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14363", 0x000007, 0x080000, CRC(f3427a56) SHA1(6a99d7432dfff35470ddcca5cfde36689a77e706) , ROM_SKIP(7) )

ROM_END

/**************************************************************************************************************************
    F1 Exhaust Note (US)
    to display the title screen this will require 2 linked system32 boards to be emulated

    Sega Game ID codes:
     GAME BD NO. 833-8389-01 EXHAUST NOTE
         ROM BD. 834-8439-01 or 834-8439-04
        MAIN BD. 837-7428 (SYSTEM 32 COM)
    Link PCB NO. 837-8223-01
      A/D BD NO. 837-7536
*/
ROM_START( f1enu ) // ROM PCB number is 834-8439-04
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x8( "epr-14451a.ic6",        0x000000, 0x020000, CRC(e17259c9) SHA1(be789b7630b7265e19ea2c80f603caff9cec37f8) )
	ROM_LOAD16_BYTE_x2( "epr-14445.ic14", 0x100000, 0x040000, CRC(d06261ab) SHA1(6e1c4ce4e49a142fd5b1ecac98145960d7afd567) )
	ROM_LOAD16_BYTE_x2( "epr-14444.ic7",  0x100001, 0x040000, CRC(07724354) SHA1(9d7f64a80553c4ae0e9cf716478fd5c4b8277470) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x8( "epr-14449.ic35", 0x100000, 0x020000, CRC(2d29699c) SHA1(cae02e5533a0edd3b3b4a54a1a43321285e06416) )
	ROM_LOAD_x2( "epr-14448.ic31", 0x200000, 0x080000, CRC(87ca1e8d) SHA1(739274171c13983a60d061176095645419dade49) )
	ROM_LOAD_x2( "epr-14447.ic26", 0x300000, 0x080000, CRC(db1cfcbd) SHA1(c76eb2ced5571a548ad00709097dd272747127a2) )
	ROM_LOAD_x2( "epr-14446.ic22", 0x400000, 0x080000, CRC(646ec2cb) SHA1(67e453f128ae227e22c68f55d0d3f5831fbeb2f9) )

	ROM_REGION( 0x100000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD32_BYTE( "mpr-14362", 0x000000, 0x040000, CRC(fb1c4e79) SHA1(38ee23763b9e5bb62bbc54cab95041415404f0c4) )
	ROM_LOAD32_BYTE( "mpr-14361", 0x000002, 0x040000, CRC(e3204bda) SHA1(34157e80edd6d685bd5a5e23b1e0130a5f3d138a) )
	ROM_LOAD32_BYTE( "mpr-14360", 0x000001, 0x040000, CRC(c5e8da79) SHA1(662a6c146fe3d0b8763d845379c06d0ee6ced1ed) )
	ROM_LOAD32_BYTE( "mpr-14359", 0x000003, 0x040000, CRC(70305c68) SHA1(7a6a1bf7381eba8cc1c3897497b32ca63316972a) )

	ROM_REGION32_BE( 0x800000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-14370", 0x000000, 0x080000, CRC(fda78289) SHA1(3740affdcc738c50d07ff3e5b592bdf8a8b6be15) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14369", 0x000001, 0x080000, CRC(7765116d) SHA1(9493148aa84adc90143cf638265d4c55bfb43990) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14368", 0x000002, 0x080000, CRC(5744a30e) SHA1(98544fb234a8e93716e951d5414a490845e213c5) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14367", 0x000003, 0x080000, CRC(77bb9003) SHA1(6370fdeab4967976840d752577cd860b9ce8efca) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14366", 0x000004, 0x080000, CRC(21078e83) SHA1(f35f643c28aad3bf18cb9906b114c4f49b7b4cd1) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14365", 0x000005, 0x080000, CRC(36913790) SHA1(4a447cffb44b023fe1441277db1e411d4cd119eb) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14364", 0x000006, 0x080000, CRC(0fa12ecd) SHA1(6a34c7718edffbeddded8786e11cac181b485ebd) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14363", 0x000007, 0x080000, CRC(f3427a56) SHA1(6a99d7432dfff35470ddcca5cfde36689a77e706) , ROM_SKIP(7) )

	ROM_REGION( 0x200000, "slavepcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x8( "epr-14451a.ic6",        0x000000, 0x020000, CRC(e17259c9) SHA1(be789b7630b7265e19ea2c80f603caff9cec37f8) )
	ROM_LOAD16_BYTE_x2( "epr-14445.ic14", 0x100000, 0x040000, CRC(d06261ab) SHA1(6e1c4ce4e49a142fd5b1ecac98145960d7afd567) )
	ROM_LOAD16_BYTE_x2( "epr-14444.ic7",  0x100001, 0x040000, CRC(07724354) SHA1(9d7f64a80553c4ae0e9cf716478fd5c4b8277470) )

	ROM_REGION( 0x500000, "slavepcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x8( "epr-14449.ic35", 0x100000, 0x020000, CRC(2d29699c) SHA1(cae02e5533a0edd3b3b4a54a1a43321285e06416) )
	ROM_LOAD_x2( "epr-14448.ic31", 0x200000, 0x080000, CRC(87ca1e8d) SHA1(739274171c13983a60d061176095645419dade49) )
	ROM_LOAD_x2( "epr-14447.ic26", 0x300000, 0x080000, CRC(db1cfcbd) SHA1(c76eb2ced5571a548ad00709097dd272747127a2) )
	ROM_LOAD_x2( "epr-14446.ic22", 0x400000, 0x080000, CRC(646ec2cb) SHA1(67e453f128ae227e22c68f55d0d3f5831fbeb2f9) )

	ROM_REGION( 0x100000, "slavepcb:gfx1", 0 ) /* tiles */
	ROM_LOAD32_BYTE( "mpr-14362", 0x000000, 0x040000, CRC(fb1c4e79) SHA1(38ee23763b9e5bb62bbc54cab95041415404f0c4) )
	ROM_LOAD32_BYTE( "mpr-14361", 0x000002, 0x040000, CRC(e3204bda) SHA1(34157e80edd6d685bd5a5e23b1e0130a5f3d138a) )
	ROM_LOAD32_BYTE( "mpr-14360", 0x000001, 0x040000, CRC(c5e8da79) SHA1(662a6c146fe3d0b8763d845379c06d0ee6ced1ed) )
	ROM_LOAD32_BYTE( "mpr-14359", 0x000003, 0x040000, CRC(70305c68) SHA1(7a6a1bf7381eba8cc1c3897497b32ca63316972a) )

	ROM_REGION32_BE( 0x800000, "slavepcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-14370", 0x000000, 0x080000, CRC(fda78289) SHA1(3740affdcc738c50d07ff3e5b592bdf8a8b6be15) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14369", 0x000001, 0x080000, CRC(7765116d) SHA1(9493148aa84adc90143cf638265d4c55bfb43990) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14368", 0x000002, 0x080000, CRC(5744a30e) SHA1(98544fb234a8e93716e951d5414a490845e213c5) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14367", 0x000003, 0x080000, CRC(77bb9003) SHA1(6370fdeab4967976840d752577cd860b9ce8efca) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14366", 0x000004, 0x080000, CRC(21078e83) SHA1(f35f643c28aad3bf18cb9906b114c4f49b7b4cd1) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14365", 0x000005, 0x080000, CRC(36913790) SHA1(4a447cffb44b023fe1441277db1e411d4cd119eb) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14364", 0x000006, 0x080000, CRC(0fa12ecd) SHA1(6a34c7718edffbeddded8786e11cac181b485ebd) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14363", 0x000007, 0x080000, CRC(f3427a56) SHA1(6a99d7432dfff35470ddcca5cfde36689a77e706) , ROM_SKIP(7) )
ROM_END

/**************************************************************************************************************************
    F1 Exhaust Note (Japan)
    to display the title screen this will require 2 linked system32 boards to be emulated

    Sega Game ID codes:
     GAME BD NO. 833-8389-03 EXHAUST NOTE
         ROM BD. 834-8439-03
        MAIN BD. 837-7428 (SYSTEM 32 COM)
    Link PCB NO. 837-8223-01
      A/D BD NO. 837-7536
*/
ROM_START( f1enj ) // ROM PCB number is 834-8439-04
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x8( "epr-14450a.ic6",        0x000000, 0x020000, CRC(10f62723) SHA1(68fcd6655798f348678e3cc8857c6d1cb46e0987) )
	ROM_LOAD16_BYTE_x2( "epr-14445.ic14", 0x100000, 0x040000, CRC(d06261ab) SHA1(6e1c4ce4e49a142fd5b1ecac98145960d7afd567) )
	ROM_LOAD16_BYTE_x2( "epr-14444.ic7",  0x100001, 0x040000, CRC(07724354) SHA1(9d7f64a80553c4ae0e9cf716478fd5c4b8277470) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x8( "epr-14449.ic35", 0x100000, 0x020000, CRC(2d29699c) SHA1(cae02e5533a0edd3b3b4a54a1a43321285e06416) )
	ROM_LOAD_x2( "epr-14448.ic31", 0x200000, 0x080000, CRC(87ca1e8d) SHA1(739274171c13983a60d061176095645419dade49) )
	ROM_LOAD_x2( "epr-14447.ic26", 0x300000, 0x080000, CRC(db1cfcbd) SHA1(c76eb2ced5571a548ad00709097dd272747127a2) )
	ROM_LOAD_x2( "epr-14446.ic22", 0x400000, 0x080000, CRC(646ec2cb) SHA1(67e453f128ae227e22c68f55d0d3f5831fbeb2f9) )

	ROM_REGION( 0x100000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD32_BYTE( "mpr-14362", 0x000000, 0x040000, CRC(fb1c4e79) SHA1(38ee23763b9e5bb62bbc54cab95041415404f0c4) )
	ROM_LOAD32_BYTE( "mpr-14361", 0x000002, 0x040000, CRC(e3204bda) SHA1(34157e80edd6d685bd5a5e23b1e0130a5f3d138a) )
	ROM_LOAD32_BYTE( "mpr-14360", 0x000001, 0x040000, CRC(c5e8da79) SHA1(662a6c146fe3d0b8763d845379c06d0ee6ced1ed) )
	ROM_LOAD32_BYTE( "mpr-14359", 0x000003, 0x040000, CRC(70305c68) SHA1(7a6a1bf7381eba8cc1c3897497b32ca63316972a) )

	ROM_REGION32_BE( 0x800000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-14370", 0x000000, 0x080000, CRC(fda78289) SHA1(3740affdcc738c50d07ff3e5b592bdf8a8b6be15) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14369", 0x000001, 0x080000, CRC(7765116d) SHA1(9493148aa84adc90143cf638265d4c55bfb43990) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14368", 0x000002, 0x080000, CRC(5744a30e) SHA1(98544fb234a8e93716e951d5414a490845e213c5) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14367", 0x000003, 0x080000, CRC(77bb9003) SHA1(6370fdeab4967976840d752577cd860b9ce8efca) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14366", 0x000004, 0x080000, CRC(21078e83) SHA1(f35f643c28aad3bf18cb9906b114c4f49b7b4cd1) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14365", 0x000005, 0x080000, CRC(36913790) SHA1(4a447cffb44b023fe1441277db1e411d4cd119eb) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14364", 0x000006, 0x080000, CRC(0fa12ecd) SHA1(6a34c7718edffbeddded8786e11cac181b485ebd) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14363", 0x000007, 0x080000, CRC(f3427a56) SHA1(6a99d7432dfff35470ddcca5cfde36689a77e706) , ROM_SKIP(7) )

	ROM_REGION( 0x200000, "slavepcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x8( "epr-14450a.ic6",        0x000000, 0x020000, CRC(10f62723) SHA1(68fcd6655798f348678e3cc8857c6d1cb46e0987) )
	ROM_LOAD16_BYTE_x2( "epr-14445.ic14", 0x100000, 0x040000, CRC(d06261ab) SHA1(6e1c4ce4e49a142fd5b1ecac98145960d7afd567) )
	ROM_LOAD16_BYTE_x2( "epr-14444.ic7",  0x100001, 0x040000, CRC(07724354) SHA1(9d7f64a80553c4ae0e9cf716478fd5c4b8277470) )

	ROM_REGION( 0x500000, "slavepcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x8( "epr-14449.ic35", 0x100000, 0x020000, CRC(2d29699c) SHA1(cae02e5533a0edd3b3b4a54a1a43321285e06416) )
	ROM_LOAD_x2( "epr-14448.ic31", 0x200000, 0x080000, CRC(87ca1e8d) SHA1(739274171c13983a60d061176095645419dade49) )
	ROM_LOAD_x2( "epr-14447.ic26", 0x300000, 0x080000, CRC(db1cfcbd) SHA1(c76eb2ced5571a548ad00709097dd272747127a2) )
	ROM_LOAD_x2( "epr-14446.ic22", 0x400000, 0x080000, CRC(646ec2cb) SHA1(67e453f128ae227e22c68f55d0d3f5831fbeb2f9) )

	ROM_REGION( 0x100000, "slavepcb:gfx1", 0 ) /* tiles */
	ROM_LOAD32_BYTE( "mpr-14362", 0x000000, 0x040000, CRC(fb1c4e79) SHA1(38ee23763b9e5bb62bbc54cab95041415404f0c4) )
	ROM_LOAD32_BYTE( "mpr-14361", 0x000002, 0x040000, CRC(e3204bda) SHA1(34157e80edd6d685bd5a5e23b1e0130a5f3d138a) )
	ROM_LOAD32_BYTE( "mpr-14360", 0x000001, 0x040000, CRC(c5e8da79) SHA1(662a6c146fe3d0b8763d845379c06d0ee6ced1ed) )
	ROM_LOAD32_BYTE( "mpr-14359", 0x000003, 0x040000, CRC(70305c68) SHA1(7a6a1bf7381eba8cc1c3897497b32ca63316972a) )

	ROM_REGION32_BE( 0x800000, "slavepcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-14370", 0x000000, 0x080000, CRC(fda78289) SHA1(3740affdcc738c50d07ff3e5b592bdf8a8b6be15) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14369", 0x000001, 0x080000, CRC(7765116d) SHA1(9493148aa84adc90143cf638265d4c55bfb43990) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14368", 0x000002, 0x080000, CRC(5744a30e) SHA1(98544fb234a8e93716e951d5414a490845e213c5) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14367", 0x000003, 0x080000, CRC(77bb9003) SHA1(6370fdeab4967976840d752577cd860b9ce8efca) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14366", 0x000004, 0x080000, CRC(21078e83) SHA1(f35f643c28aad3bf18cb9906b114c4f49b7b4cd1) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14365", 0x000005, 0x080000, CRC(36913790) SHA1(4a447cffb44b023fe1441277db1e411d4cd119eb) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14364", 0x000006, 0x080000, CRC(0fa12ecd) SHA1(6a34c7718edffbeddded8786e11cac181b485ebd) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14363", 0x000007, 0x080000, CRC(f3427a56) SHA1(6a99d7432dfff35470ddcca5cfde36689a77e706) , ROM_SKIP(7) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    F1 Super Lap (Export)
    protected via FD1149 317-0210
*/
ROM_START( f1lap )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x4( "epr-15598.ic17",        0x000000, 0x020000, CRC(9feab7cd) SHA1(2a14c0df39e7bdae12a34679fabc6abb7618e27d) )
	ROM_LOAD_x4( "epr-15611.ic8",         0x080000, 0x020000, CRC(0d8c97c2) SHA1(863c606c58faddc2bdaeb69f9079266155ff9a96) )
	ROM_LOAD16_BYTE_x2( "epr-15596.ic18", 0x100000, 0x040000, CRC(20e92909) SHA1(b974c79e11bfbd1cee61f9041cf79971fd96db3a) )
	ROM_LOAD16_BYTE_x2( "epr-15597.ic9",  0x100001, 0x040000, CRC(cd1ccddb) SHA1(ff0371a8010141d1ab81b5eba555ae7c64e5da37) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x8( "epr-15592.ic36", 0x100000, 0x020000, CRC(7c055cc8) SHA1(169beb83dfae86dd408aa92b3c214b8f607825fc) )
	ROM_LOAD( "mpr-15593.ic35",    0x200000, 0x100000, CRC(e7300441) SHA1(33c264f0e6326689ba75026932c0932868e83b25) )
	ROM_LOAD( "mpr-15594.ic34",    0x300000, 0x100000, CRC(7f4ca3bb) SHA1(dc53a1857d619e574acb4c0587a6ba844df2d283) )
	ROM_LOAD( "mpr-15595.ic24",    0x400000, 0x100000, CRC(3fbdad9a) SHA1(573ea2242f79c7d3b6bf0e6745f6b07a621834ac) )

	ROM_REGION( 0x400000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-15608.ic14", 0x000000, 0x200000, CRC(64462c69) SHA1(9501e83c52e3e16f73b94cef975b5a31b2ee5476) )
	ROM_LOAD16_BYTE( "mpr-15609.ic5",  0x000001, 0x200000, CRC(d586e455) SHA1(aea190d31c590216eb19766ba749b1e9b710bdce) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-15600.ic32", 0x000000, 0x200000, CRC(d2698d23) SHA1(996fbcc1d0814e6f14fa7e4870ece077ecda54e6) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15602.ic30", 0x000002, 0x200000, CRC(1674764d) SHA1(bc39757a5d25df1a088f874ca2442854eb551e48) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15604.ic28", 0x000004, 0x200000, CRC(1552bbb9) SHA1(77edd3f9d8dec87fa0445d264309e6164eba9313) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15606.ic26", 0x000006, 0x200000, CRC(2b4f5265) SHA1(48b4ccdedb52fbf80661ff380e5a273201fc0a12) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15601.ic31", 0x800000, 0x200000, CRC(31a8f40a) SHA1(62798346750dea87e43c8a8b01c33bf886bb50f4) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15603.ic29", 0x800002, 0x200000, CRC(3805ecbc) SHA1(54d29250441160f282c70adfd515adb21d2cda33) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15605.ic27", 0x800004, 0x200000, CRC(cbdbf35e) SHA1(a1c0900ac3210e72f5848561a6c4a77c804782c6) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15607.ic25", 0x800006, 0x200000, CRC(6c8817c9) SHA1(f5d493ed4237caf5042e95373bf9abd1fd16f873) , ROM_SKIP(6)|ROM_GROUPWORD )

	ROM_REGION( 0x20000, "user2", 0 ) /*  comms board  */
	ROM_LOAD( "15612", 0x00000, 0x20000, CRC(9d204617) SHA1(8db57121065f5d1ac52fcfb88459bdbdc30e645b) )
ROM_END

/**************************************************************************************************************************
    F1 Super Lap (Japan)
    protected via FD1149 317-0210
*/
ROM_START( f1lapj )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x4( "epr-15598.ic17",        0x000000, 0x020000, CRC(9feab7cd) SHA1(2a14c0df39e7bdae12a34679fabc6abb7618e27d) )
	ROM_LOAD_x4( "epr-15599.ic8",         0x080000, 0x020000, CRC(5c5ac112) SHA1(2c071946e33f0700a832c7aad36f639acd35f555) )
	ROM_LOAD16_BYTE_x2( "epr-15596.ic18", 0x100000, 0x040000, CRC(20e92909) SHA1(b974c79e11bfbd1cee61f9041cf79971fd96db3a) )
	ROM_LOAD16_BYTE_x2( "epr-15597.ic9",  0x100001, 0x040000, CRC(cd1ccddb) SHA1(ff0371a8010141d1ab81b5eba555ae7c64e5da37) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x8( "epr-15592.ic36", 0x100000, 0x020000, CRC(7c055cc8) SHA1(169beb83dfae86dd408aa92b3c214b8f607825fc) )
	ROM_LOAD( "mpr-15593.ic35",    0x200000, 0x100000, CRC(e7300441) SHA1(33c264f0e6326689ba75026932c0932868e83b25) )
	ROM_LOAD( "mpr-15594.ic34",    0x300000, 0x100000, CRC(7f4ca3bb) SHA1(dc53a1857d619e574acb4c0587a6ba844df2d283) )
	ROM_LOAD( "mpr-15595.ic24",    0x400000, 0x100000, CRC(3fbdad9a) SHA1(573ea2242f79c7d3b6bf0e6745f6b07a621834ac) )

	ROM_REGION( 0x400000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-15608.ic14", 0x000000, 0x200000, CRC(64462c69) SHA1(9501e83c52e3e16f73b94cef975b5a31b2ee5476) )
	ROM_LOAD16_BYTE( "mpr-15609.ic5",  0x000001, 0x200000, CRC(d586e455) SHA1(aea190d31c590216eb19766ba749b1e9b710bdce) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-15600.ic32", 0x000000, 0x200000, CRC(d2698d23) SHA1(996fbcc1d0814e6f14fa7e4870ece077ecda54e6) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15602.ic30", 0x000002, 0x200000, CRC(1674764d) SHA1(bc39757a5d25df1a088f874ca2442854eb551e48) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15604.ic28", 0x000004, 0x200000, CRC(1552bbb9) SHA1(77edd3f9d8dec87fa0445d264309e6164eba9313) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15606.ic26", 0x000006, 0x200000, CRC(2b4f5265) SHA1(48b4ccdedb52fbf80661ff380e5a273201fc0a12) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15601.ic31", 0x800000, 0x200000, CRC(31a8f40a) SHA1(62798346750dea87e43c8a8b01c33bf886bb50f4) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15603.ic29", 0x800002, 0x200000, CRC(3805ecbc) SHA1(54d29250441160f282c70adfd515adb21d2cda33) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15605.ic27", 0x800004, 0x200000, CRC(cbdbf35e) SHA1(a1c0900ac3210e72f5848561a6c4a77c804782c6) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15607.ic25", 0x800006, 0x200000, CRC(6c8817c9) SHA1(f5d493ed4237caf5042e95373bf9abd1fd16f873) , ROM_SKIP(6)|ROM_GROUPWORD )

	ROM_REGION( 0x20000, "user2", 0 ) /*  comms board  */
	ROM_LOAD( "15612", 0x00000, 0x20000, CRC(9d204617) SHA1(8db57121065f5d1ac52fcfb88459bdbdc30e645b) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Golden Axe: The Return of Death Adder (Export)
    protected via a custom V25 with encrypted code
*/
ROM_START( ga2 )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x4( "epr-14961b.ic17",        0x000000, 0x020000, CRC(d9cd8885) SHA1(dc9d1f01770bd23ba5959e300badbc5093a149bc) )
	ROM_LOAD_x4( "epr-14958b.ic8",         0x080000, 0x020000, CRC(0be324a3) SHA1(5e5f457548906453eaa8d326c353b47353eab73d) )
	ROM_LOAD16_BYTE_x2( "epr-15148b.ic18", 0x100000, 0x040000, CRC(c477a9fd) SHA1(a9d60f801c12fd067e5ad1801a92c84edd13bd08) )
	ROM_LOAD16_BYTE_x2( "epr-15147b.ic9",  0x100001, 0x040000, CRC(1bb676ea) SHA1(125ffd13204f48be23e20b281c42c2307888c40b) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU + banks */
	ROM_LOAD_x16( "epr-14945.ic36", 0x100000, 0x010000, CRC(4781d4cb) SHA1(bd1b774b3cd0c3e0290c55e426f66d6820d21d0f) )
	ROM_LOAD( "mpr-14944.ic35",     0x200000, 0x100000, CRC(fd4d4b86) SHA1(e14b9cd6004bf9ecd902e37b433b828241361b46) )
	ROM_LOAD( "mpr-14943.ic34",     0x300000, 0x100000, CRC(24d40333) SHA1(38faf8f3eac317a163e93bd2247fe98189b13d2d) )
	ROM_LOAD( "mpr-14942.ic24",     0x400000, 0x100000, CRC(a89b0e90) SHA1(e14c62418eb7f9a2deb2a6dcf635bedc1c73c253) )

	ROM_REGION( 0x100000, "mainpcb:mcu", 0 ) /* Protection CPU */
	ROM_LOAD( "epr-14468-02.u3", 0x00000, 0x10000, CRC(77634daa) SHA1(339169d164b9ed7dc3787b084d33effdc8e9efc1) ) /* located on separate sub board */

	ROM_REGION( 0x400000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-14948.ic14", 0x000000, 0x200000, CRC(75050d4a) SHA1(51d6bc9935abcf30af438e69c2cf4e09f57a803f) )
	ROM_LOAD16_BYTE( "mpr-14947.ic5",  0x000001, 0x200000, CRC(b53e62f4) SHA1(5aa0f198e6eb070b77b0d180d30c0228a9bc691e) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-14949.ic32", 0x000000, 0x200000, CRC(152c716c) SHA1(448d16ea036b66e886119c00af543dfa5e53fd84) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14951.ic30", 0x000002, 0x200000, CRC(fdb1a534) SHA1(3126b595bf69bf9952fedf8f9c6743eb10489dc6) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14953.ic28", 0x000004, 0x200000, CRC(33bd1c15) SHA1(4e16562e3357d4db54b20543073e8f1fd6f74b1f) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14955.ic26", 0x000006, 0x200000, CRC(e42684aa) SHA1(12e0f18a11edb46f09e2e8c5c4ba14170d0cf00d) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14950.ic31", 0x800000, 0x200000, CRC(15fd0026) SHA1(e918984bd60ad63172fe273b31cc9019100228c8) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14952.ic29", 0x800002, 0x200000, CRC(96f96613) SHA1(4c9808866032dab0401de322c28242e8a8775457) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14954.ic27", 0x800004, 0x200000, CRC(39b2ac9e) SHA1(74f4c81d85ab9b4c5e8ae4b4d2c6dff766c482ca) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14956.ic25", 0x800006, 0x200000, CRC(298fca50) SHA1(16e09b19cc7be3dfc8e82b45348e6d1cf2ed5621) , ROM_SKIP(6)|ROM_GROUPWORD )
ROM_END

/**************************************************************************************************************************
    Golden Axe: The Return of Death Adder (US)
    protected via a custom V25 with encrypted code
    Sega Game ID codes:
     Game: 833-8932-02 GOLDEN AXE II AC USA
Rom board: 833-8933-01
Sub board: 834-8529-02

*/
ROM_START( ga2u )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x4( "epr-14960a.ic17",        0x000000, 0x020000, CRC(87182fea) SHA1(bb669ea7091f1ea34589a565490effa934ca44a3) )
	ROM_LOAD_x4( "epr-14957a.ic8",         0x080000, 0x020000, CRC(ab787cf4) SHA1(7e19bb3e5d587b5009efc9f9fa52aecaef0eedc4) )
	ROM_LOAD16_BYTE_x2( "epr-15146a.ic18", 0x100000, 0x040000, CRC(7293d5c3) SHA1(535a8b4b4a05546b321cee8de6733edfc1f71589) )
	ROM_LOAD16_BYTE_x2( "epr-15145a.ic9",  0x100001, 0x040000, CRC(0da61782) SHA1(f0302d747e5d55663095bb38732af423104c33ea) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU + banks */
	ROM_LOAD_x16( "epr-14945.ic36", 0x100000, 0x010000, CRC(4781d4cb) SHA1(bd1b774b3cd0c3e0290c55e426f66d6820d21d0f) )
	ROM_LOAD( "mpr-14944.ic35",     0x200000, 0x100000, CRC(fd4d4b86) SHA1(e14b9cd6004bf9ecd902e37b433b828241361b46) )
	ROM_LOAD( "mpr-14943.ic34",     0x300000, 0x100000, CRC(24d40333) SHA1(38faf8f3eac317a163e93bd2247fe98189b13d2d) )
	ROM_LOAD( "mpr-14942.ic24",     0x400000, 0x100000, CRC(a89b0e90) SHA1(e14c62418eb7f9a2deb2a6dcf635bedc1c73c253) )

	ROM_REGION( 0x100000, "mainpcb:mcu", 0 ) /* Protection CPU */
	ROM_LOAD( "epr-14468-02.u3", 0x00000, 0x10000, CRC(77634daa) SHA1(339169d164b9ed7dc3787b084d33effdc8e9efc1) ) /* located on separate sub board */

	ROM_REGION( 0x400000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-14948.ic14", 0x000000, 0x200000, CRC(75050d4a) SHA1(51d6bc9935abcf30af438e69c2cf4e09f57a803f) )
	ROM_LOAD16_BYTE( "mpr-14947.ic5",  0x000001, 0x200000, CRC(b53e62f4) SHA1(5aa0f198e6eb070b77b0d180d30c0228a9bc691e) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-14949.ic32", 0x000000, 0x200000, CRC(152c716c) SHA1(448d16ea036b66e886119c00af543dfa5e53fd84) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14951.ic30", 0x000002, 0x200000, CRC(fdb1a534) SHA1(3126b595bf69bf9952fedf8f9c6743eb10489dc6) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14953.ic28", 0x000004, 0x200000, CRC(33bd1c15) SHA1(4e16562e3357d4db54b20543073e8f1fd6f74b1f) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14955.ic26", 0x000006, 0x200000, CRC(e42684aa) SHA1(12e0f18a11edb46f09e2e8c5c4ba14170d0cf00d) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14950.ic31", 0x800000, 0x200000, CRC(15fd0026) SHA1(e918984bd60ad63172fe273b31cc9019100228c8) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14952.ic29", 0x800002, 0x200000, CRC(96f96613) SHA1(4c9808866032dab0401de322c28242e8a8775457) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14954.ic27", 0x800004, 0x200000, CRC(39b2ac9e) SHA1(74f4c81d85ab9b4c5e8ae4b4d2c6dff766c482ca) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14956.ic25", 0x800006, 0x200000, CRC(298fca50) SHA1(16e09b19cc7be3dfc8e82b45348e6d1cf2ed5621) , ROM_SKIP(6)|ROM_GROUPWORD )
ROM_END

/**************************************************************************************************************************
    Golden Axe: The Return of Death Adder (Japan)
    protected via a custom V25 with encrypted code
*/
ROM_START( ga2j )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x4( "epr-14956.ic17",        0x000000, 0x020000, CRC(f1929177) SHA1(7dc39c40eff9fb46c2e51d1e83478cd6970e3951) )
	ROM_LOAD_x4( "epr-14946.ic8",         0x080000, 0x020000, CRC(eacafe94) SHA1(d41a7e1ee2df9e053b559be0a1a6d2ae520fd3e4) )
	ROM_LOAD16_BYTE_x2( "epr-14941.ic18", 0x100000, 0x040000, CRC(0ffb8203) SHA1(b27dce634d203af8abb6ddfb656d4c48eb54af01) )
	ROM_LOAD16_BYTE_x2( "epr-14940.ic9",  0x100001, 0x040000, CRC(3b5b3084) SHA1(ea17f6b7fd413fe3808f822cec84c993c9b75aa2) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU + banks */
	ROM_LOAD_x16( "epr-14945.ic36", 0x100000, 0x010000, CRC(4781d4cb) SHA1(bd1b774b3cd0c3e0290c55e426f66d6820d21d0f) )
	ROM_LOAD( "mpr-14944.ic35",     0x200000, 0x100000, CRC(fd4d4b86) SHA1(e14b9cd6004bf9ecd902e37b433b828241361b46) )
	ROM_LOAD( "mpr-14943.ic34",     0x300000, 0x100000, CRC(24d40333) SHA1(38faf8f3eac317a163e93bd2247fe98189b13d2d) )
	ROM_LOAD( "mpr-14942.ic24",     0x400000, 0x100000, CRC(a89b0e90) SHA1(e14c62418eb7f9a2deb2a6dcf635bedc1c73c253) )

	ROM_REGION( 0x100000, "mainpcb:mcu", 0 ) /* Protection CPU */
	ROM_LOAD( "epr-14468-02.u3", 0x00000, 0x10000, CRC(77634daa) SHA1(339169d164b9ed7dc3787b084d33effdc8e9efc1) ) /* located on separate sub board */

	ROM_REGION( 0x400000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-14948.ic14", 0x000000, 0x200000, CRC(75050d4a) SHA1(51d6bc9935abcf30af438e69c2cf4e09f57a803f) )
	ROM_LOAD16_BYTE( "mpr-14947.ic5",  0x000001, 0x200000, CRC(b53e62f4) SHA1(5aa0f198e6eb070b77b0d180d30c0228a9bc691e) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-14949.ic32", 0x000000, 0x200000, CRC(152c716c) SHA1(448d16ea036b66e886119c00af543dfa5e53fd84) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14951.ic30", 0x000002, 0x200000, CRC(fdb1a534) SHA1(3126b595bf69bf9952fedf8f9c6743eb10489dc6) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14953.ic28", 0x000004, 0x200000, CRC(33bd1c15) SHA1(4e16562e3357d4db54b20543073e8f1fd6f74b1f) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14955.ic26", 0x000006, 0x200000, CRC(e42684aa) SHA1(12e0f18a11edb46f09e2e8c5c4ba14170d0cf00d) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14950.ic31", 0x800000, 0x200000, CRC(15fd0026) SHA1(e918984bd60ad63172fe273b31cc9019100228c8) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14952.ic29", 0x800002, 0x200000, CRC(96f96613) SHA1(4c9808866032dab0401de322c28242e8a8775457) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14954.ic27", 0x800004, 0x200000, CRC(39b2ac9e) SHA1(74f4c81d85ab9b4c5e8ae4b4d2c6dff766c482ca) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-14956.ic25", 0x800006, 0x200000, CRC(298fca50) SHA1(16e09b19cc7be3dfc8e82b45348e6d1cf2ed5621) , ROM_SKIP(6)|ROM_GROUPWORD )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Hard Dunk (Export) - Multi-32
    not protected
*/
ROM_START( harddunk )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code */
	ROM_LOAD32_WORD_x2( "epr-16512.ic37", 0x000000, 0x40000, CRC(1a7de085) SHA1(2e0dac1f7715089b7f6b1035c859ffe2d674932f) )
	/* the following is the same as 16509.ic40 but with a different name, unusual for Sega */
	ROM_LOAD32_WORD_x2( "epr-16513.ic40", 0x000002, 0x40000, CRC(603dee75) SHA1(32ae964a4b57d470b4900cca6e06329f1a75a6e6) )

	ROM_REGION( 0x180000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x4( "epr-16505.ic31", 0x100000, 0x20000, CRC(eeb90a07) SHA1(d1c2132897994b2e85fd5a97222b9fcd61bc421e) )

	ROM_REGION( 0x100000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-16503.ic3", 0x000000, 0x080000, CRC(ac1b6f1a) SHA1(56482931adf7fe551acf796b74cd8af3773d4fef) )
	ROM_LOAD16_BYTE( "mpr-16504.ic11", 0x000001, 0x080000, CRC(7c61fcd8) SHA1(ca4354f90fada752bf11ee22a7798a8aa22b1c61) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-16495.ic14", 0x000000, 0x200000, CRC(6e5f26be) SHA1(146761072bbed08f4a9df8a474b34fab61afaa4f) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16497.ic15", 0x000002, 0x200000, CRC(42ab5859) SHA1(f50c51eb81186aec5f747ecab4c5c928f8701afc) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16499.ic10", 0x000004, 0x200000, CRC(a290ea36) SHA1(2503b44174f23a9d323caab86553977d1d6d9c94) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16501.ic38", 0x000006, 0x200000, CRC(f1566620) SHA1(bcf31d11ee669d5afc7dc22c42fa59f4e48c1f50) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16496.ic22", 0x800000, 0x200000, CRC(d9d27247) SHA1(d211623478516ed1b89ab16a7fc7969954c5e353) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16498.ic23", 0x800002, 0x200000, CRC(c022a991) SHA1(a660a20692f4d9ba7be73577328f69f109be5e47) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16500.ic18", 0x800004, 0x200000, CRC(452c0be3) SHA1(af87ce4618bae2d791c1baed34ba7f853af664ff) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16502.ic41", 0x800006, 0x200000, CRC(ffc3147e) SHA1(12d882dec3098674d27058a8009e8778555f477a) , ROM_SKIP(6)|ROM_GROUPWORD )

	ROM_REGION( 0x400000, "mainpcb:sega", 0 ) /* Sega PCM sound data */
	ROM_LOAD( "mpr-16506.1", 0x000000, 0x200000, CRC(e779f5ed) SHA1(462d1bbe8bb12a0c5a6d6c613c720b26ec21cb25) )
	ROM_LOAD( "mpr-16507.2", 0x200000, 0x200000, CRC(31e068d3) SHA1(9ac88b15af441fb3b31ce759c565b60a09039571) )
ROM_END

/**************************************************************************************************************************
    Hard Dunk (Japan) - Multi-32
    not protected
*/
ROM_START( harddunkj )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code */
	ROM_LOAD32_WORD_x2( "epr-16508.ic37", 0x000000, 0x40000, CRC(b3713be5) SHA1(8123638a838e41fcc0d32e14382421b521eff94f) )
	ROM_LOAD32_WORD_x2( "epr-16509.ic40", 0x000002, 0x40000, CRC(603dee75) SHA1(32ae964a4b57d470b4900cca6e06329f1a75a6e6) )

	ROM_REGION( 0x180000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x4( "epr-16505.ic31", 0x100000, 0x20000, CRC(eeb90a07) SHA1(d1c2132897994b2e85fd5a97222b9fcd61bc421e) )

	ROM_REGION( 0x100000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-16503.ic3", 0x000000, 0x080000, CRC(ac1b6f1a) SHA1(56482931adf7fe551acf796b74cd8af3773d4fef) )
	ROM_LOAD16_BYTE( "mpr-16504.ic11", 0x000001, 0x080000, CRC(7c61fcd8) SHA1(ca4354f90fada752bf11ee22a7798a8aa22b1c61) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-16495.ic14", 0x000000, 0x200000, CRC(6e5f26be) SHA1(146761072bbed08f4a9df8a474b34fab61afaa4f) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16497.ic15", 0x000002, 0x200000, CRC(42ab5859) SHA1(f50c51eb81186aec5f747ecab4c5c928f8701afc) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16499.ic10", 0x000004, 0x200000, CRC(a290ea36) SHA1(2503b44174f23a9d323caab86553977d1d6d9c94) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16501.ic38", 0x000006, 0x200000, CRC(f1566620) SHA1(bcf31d11ee669d5afc7dc22c42fa59f4e48c1f50) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16496.ic22", 0x800000, 0x200000, CRC(d9d27247) SHA1(d211623478516ed1b89ab16a7fc7969954c5e353) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16498.ic23", 0x800002, 0x200000, CRC(c022a991) SHA1(a660a20692f4d9ba7be73577328f69f109be5e47) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16500.ic18", 0x800004, 0x200000, CRC(452c0be3) SHA1(af87ce4618bae2d791c1baed34ba7f853af664ff) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16502.ic41", 0x800006, 0x200000, CRC(ffc3147e) SHA1(12d882dec3098674d27058a8009e8778555f477a) , ROM_SKIP(6)|ROM_GROUPWORD )

	ROM_REGION( 0x400000, "mainpcb:sega", 0 ) /* Sega PCM sound data */
	ROM_LOAD( "mpr-16506.ic1", 0x000000, 0x200000, CRC(e779f5ed) SHA1(462d1bbe8bb12a0c5a6d6c613c720b26ec21cb25) )
	ROM_LOAD( "mpr-16507.ic2", 0x200000, 0x200000, CRC(31e068d3) SHA1(9ac88b15af441fb3b31ce759c565b60a09039571) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Holosseum (US)
    not protected
     Game: 833-8887-01 HOLOSSEUM
Rom board: 834-8888-01

*/
ROM_START( holo )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x4( "epr-14977a",       0x000000, 0x020000, CRC(e0d7e288) SHA1(3126041ba73f21fac0207bf5c63230c61180f564) )
	ROM_LOAD_x4( "epr-14976a",       0x080000, 0x020000, CRC(e56f13be) SHA1(3d9e7add8feaa35c4c2e8bda84ae251087bd5e40) )
	ROM_LOAD16_BYTE_x4( "epr-15011", 0x100000, 0x020000, CRC(b9f59f59) SHA1(f8c91fa877cf53153bec3d7850eab38227cc18ba) )
	ROM_LOAD16_BYTE_x4( "epr-15010", 0x100001, 0x020000, CRC(0c09c57b) SHA1(028a9fe1c625be218ba90906308d25d69d4de4c4) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x8( "epr-14965", 0x100000, 0x020000, CRC(3a918cfe) SHA1(f43ecbc9e774873e868bc921321541b308ea1a3c) )
	ROM_LOAD( "mpr-14964",    0x200000, 0x100000, CRC(7ff581d5) SHA1(ab81bd70937319e4edc8924bdb493d5ef1ec096a) )
	ROM_LOAD( "mpr-14963",    0x300000, 0x100000, CRC(0974a60e) SHA1(87d770edcee9c9e8f37d36ab28c5aa5d685ea849) )
	ROM_LOAD( "mpr-14962",    0x400000, 0x100000, CRC(6b2e694e) SHA1(7874bdfd534231c7756e0e0d9fc7a3d5bdba74d3) )

	ROM_REGION( 0x000100, "mainpcb:gfx1", ROMREGION_ERASEFF ) /* tiles */
	/* game doesn't use bg tilemaps */

	ROM_REGION32_BE( 0x800000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-14973", 0x000000, 0x100000, CRC(b3c3ff6b) SHA1(94e8dbfae37a5b122ee3d471aad1f758e4a39b9e) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14972", 0x000001, 0x100000, CRC(0c161374) SHA1(413ab45deb687ecdbdc06ae98aa32ad8a0d80e0c) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14971", 0x000002, 0x100000, CRC(dfcf6fdf) SHA1(417291b54010be20dd6738a70d372b580615a8bb) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14970", 0x000003, 0x100000, CRC(cae3a745) SHA1(b6cc1f4abb460cda4714967e880928dc727ecf0a) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14969", 0x000004, 0x100000, CRC(c06b7c15) SHA1(8b97a899e6eacf798b9f55af8df95e12ccacadec) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14968", 0x000005, 0x100000, CRC(f413894a) SHA1(d65f57b1e55199e901c7c2f701589c46eeab739a) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14967", 0x000006, 0x100000, CRC(5377fce0) SHA1(baf8f82ab851b24202938fc7213d72324b9b92c0) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14966", 0x000007, 0x100000, CRC(dffba2e9) SHA1(b97e47e78abb8302bc2c87681643382203bd76eb) , ROM_SKIP(7) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Jurassic Park (Export)
    not protected
     Game: 833-10544 JURASSIC PARK
   ROM BD: 834-10545
A/D BD NO: 837-7536-91
*/
ROM_START( jpark )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x2( "epr-16402a.ic8",     0x000000, 0x80000, CRC(c70db239) SHA1(fd79dfd1ce194fcc8ccb58117bc845cdfe9943b1) )
	ROM_LOAD16_BYTE( "epr-16395.ic18", 0x100000, 0x80000, CRC(ac5a01d6) SHA1(df6bffdf5723cb8790a9c1c0ab271989a758bdd8) )
	ROM_LOAD16_BYTE( "epr-16394.ic9",  0x100001, 0x80000, CRC(c08c3a8a) SHA1(923cf256d863656336401fa75103b42298cb3822) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x4( "epr-16399.ic36", 0x100000, 0x040000, CRC(b09b2fe3) SHA1(bf8d646bab65fcc4ece8c2bd9a3df389e5860ed6) )
	ROM_LOAD( "mpr-16398.ic35",    0x200000, 0x100000, CRC(fa710ca6) SHA1(1fd625070eef5f99d7be07606aeeff9282e32532) )
	ROM_LOAD( "mpr-16397.ic34",    0x300000, 0x100000, CRC(6e96e0be) SHA1(422b783b72127b80a23043b2dd1c04f5772f436e) )
	ROM_LOAD( "mpr-16396.ic24",    0x400000, 0x100000, CRC(f69a2dc4) SHA1(3f02b10976852916c58e852f3161a857784fe36b) )

	ROM_REGION( 0x400000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-16404.ic14", 0x000000, 0x200000, CRC(11283807) SHA1(99e465c3fc31e640740b8257a349e203f026754a) )
	ROM_LOAD16_BYTE( "mpr-16403.ic5",  0x000001, 0x200000, CRC(02530a9b) SHA1(b43e1b47f74c801bfc599cbe893fb8dc13453dd0) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-16405.ic32", 0x000000, 0x200000, CRC(b425f182) SHA1(66c6bd29dd3450db816b895c4c9c5208a66aae67) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16407.ic30", 0x000002, 0x200000, CRC(bc49ffd9) SHA1(a50ba7ddccfdfd7638c4041978b39c1559afbbb4) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16409.ic28", 0x000004, 0x200000, CRC(fe73660d) SHA1(ec1a3ea5303d2ccb9e327da18476969953626e1c) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16411.ic26", 0x000006, 0x200000, CRC(71cabbc5) SHA1(9760f57ef43eb251488dadd37711d5682d902434) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16406.ic31", 0x800000, 0x200000, CRC(b9ed73d6) SHA1(0dd22e7a21e95d84fc91acd742c737f96529f515) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16408.ic29", 0x800002, 0x200000, CRC(7b2f476b) SHA1(da99a9911982ba8aaef8c9aecc2977c9fd6da094) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16410.ic27", 0x800004, 0x200000, CRC(49c8f952) SHA1(f26b818711910b10bf520e5f849a1478a6b1d6e6) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16412.ic25", 0x800006, 0x200000, CRC(105dc26e) SHA1(fd2ef8c9fe1a78b4f9cc891a6fbd060184e58a1f) , ROM_SKIP(6)|ROM_GROUPWORD )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* unused */
	ROM_LOAD( "epr-13908.xx", 0x00000, 0x8000, CRC(6228c1d2) SHA1(bd37fe775534fb94c9af80546948ce5f9c47bbf5) ) /* cabinet movement */
ROM_END

/**************************************************************************************************************************
    Jurassic Park - Deluxe, Revision A (Japan)
    not protected
     Game: 833-10544-03 JURASSIC PARK DLX
   ROM BD: 834-10545-03
*/
ROM_START( jparkj )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x2( "epr-16400a.ic8",     0x000000, 0x80000, CRC(1e03dbfe) SHA1(b7c274769ff483e687749ff20b1dc0fc38e8ef82) )
	ROM_LOAD16_BYTE( "epr-16395.ic18", 0x100000, 0x80000, CRC(ac5a01d6) SHA1(df6bffdf5723cb8790a9c1c0ab271989a758bdd8) )
	ROM_LOAD16_BYTE( "epr-16394.ic9",  0x100001, 0x80000, CRC(c08c3a8a) SHA1(923cf256d863656336401fa75103b42298cb3822) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x4( "epr-16399.ic36", 0x100000, 0x040000, CRC(b09b2fe3) SHA1(bf8d646bab65fcc4ece8c2bd9a3df389e5860ed6) )
	ROM_LOAD( "mpr-16398.ic35",    0x200000, 0x100000, CRC(fa710ca6) SHA1(1fd625070eef5f99d7be07606aeeff9282e32532) )
	ROM_LOAD( "mpr-16397.ic34",    0x300000, 0x100000, CRC(6e96e0be) SHA1(422b783b72127b80a23043b2dd1c04f5772f436e) )
	ROM_LOAD( "mpr-16396.ic24",    0x400000, 0x100000, CRC(f69a2dc4) SHA1(3f02b10976852916c58e852f3161a857784fe36b) )

	ROM_REGION( 0x400000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-16404.ic14", 0x000000, 0x200000, CRC(11283807) SHA1(99e465c3fc31e640740b8257a349e203f026754a) )
	ROM_LOAD16_BYTE( "mpr-16403.ic5",  0x000001, 0x200000, CRC(02530a9b) SHA1(b43e1b47f74c801bfc599cbe893fb8dc13453dd0) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-16405.ic32", 0x000000, 0x200000, CRC(b425f182) SHA1(66c6bd29dd3450db816b895c4c9c5208a66aae67) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16407.ic30", 0x000002, 0x200000, CRC(bc49ffd9) SHA1(a50ba7ddccfdfd7638c4041978b39c1559afbbb4) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16409.ic28", 0x000004, 0x200000, CRC(fe73660d) SHA1(ec1a3ea5303d2ccb9e327da18476969953626e1c) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16411.ic26", 0x000006, 0x200000, CRC(71cabbc5) SHA1(9760f57ef43eb251488dadd37711d5682d902434) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16406.ic31", 0x800000, 0x200000, CRC(b9ed73d6) SHA1(0dd22e7a21e95d84fc91acd742c737f96529f515) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16408.ic29", 0x800002, 0x200000, CRC(7b2f476b) SHA1(da99a9911982ba8aaef8c9aecc2977c9fd6da094) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16410.ic27", 0x800004, 0x200000, CRC(49c8f952) SHA1(f26b818711910b10bf520e5f849a1478a6b1d6e6) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16412.ic25", 0x800006, 0x200000, CRC(105dc26e) SHA1(fd2ef8c9fe1a78b4f9cc891a6fbd060184e58a1f) , ROM_SKIP(6)|ROM_GROUPWORD )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* unused */
	ROM_LOAD( "epr-13908.xx", 0x00000, 0x8000, CRC(6228c1d2) SHA1(bd37fe775534fb94c9af80546948ce5f9c47bbf5) ) /* cabinet movement */
ROM_END

/**************************************************************************************************************************
    Jurassic Park - Deluxe (Japan)
    not protected
     Game: 833-10544-03 JURASSIC PARK DLX
   ROM BD: 834-10545-03
*/
ROM_START( jparkja )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x2( "epr-16400.ic8",      0x000000, 0x80000, CRC(321c3411) SHA1(c53e7ed5f2e523741a521c9cd271123ab557cc4a) )
	ROM_LOAD16_BYTE( "epr-16395.ic18", 0x100000, 0x80000, CRC(ac5a01d6) SHA1(df6bffdf5723cb8790a9c1c0ab271989a758bdd8) )
	ROM_LOAD16_BYTE( "epr-16394.ic9",  0x100001, 0x80000, CRC(c08c3a8a) SHA1(923cf256d863656336401fa75103b42298cb3822) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x4( "epr-16399.ic36", 0x100000, 0x040000, CRC(b09b2fe3) SHA1(bf8d646bab65fcc4ece8c2bd9a3df389e5860ed6) )
	ROM_LOAD( "mpr-16398.ic35",    0x200000, 0x100000, CRC(fa710ca6) SHA1(1fd625070eef5f99d7be07606aeeff9282e32532) )
	ROM_LOAD( "mpr-16397.ic34",    0x300000, 0x100000, CRC(6e96e0be) SHA1(422b783b72127b80a23043b2dd1c04f5772f436e) )
	ROM_LOAD( "mpr-16396.ic24",    0x400000, 0x100000, CRC(f69a2dc4) SHA1(3f02b10976852916c58e852f3161a857784fe36b) )

	ROM_REGION( 0x400000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-16404.ic14", 0x000000, 0x200000, CRC(11283807) SHA1(99e465c3fc31e640740b8257a349e203f026754a) )
	ROM_LOAD16_BYTE( "mpr-16403.ic5",  0x000001, 0x200000, CRC(02530a9b) SHA1(b43e1b47f74c801bfc599cbe893fb8dc13453dd0) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-16405.ic32", 0x000000, 0x200000, CRC(b425f182) SHA1(66c6bd29dd3450db816b895c4c9c5208a66aae67) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16407.ic30", 0x000002, 0x200000, CRC(bc49ffd9) SHA1(a50ba7ddccfdfd7638c4041978b39c1559afbbb4) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16409.ic28", 0x000004, 0x200000, CRC(fe73660d) SHA1(ec1a3ea5303d2ccb9e327da18476969953626e1c) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16411.ic26", 0x000006, 0x200000, CRC(71cabbc5) SHA1(9760f57ef43eb251488dadd37711d5682d902434) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16406.ic31", 0x800000, 0x200000, CRC(b9ed73d6) SHA1(0dd22e7a21e95d84fc91acd742c737f96529f515) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16408.ic29", 0x800002, 0x200000, CRC(7b2f476b) SHA1(da99a9911982ba8aaef8c9aecc2977c9fd6da094) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16410.ic27", 0x800004, 0x200000, CRC(49c8f952) SHA1(f26b818711910b10bf520e5f849a1478a6b1d6e6) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16412.ic25", 0x800006, 0x200000, CRC(105dc26e) SHA1(fd2ef8c9fe1a78b4f9cc891a6fbd060184e58a1f) , ROM_SKIP(6)|ROM_GROUPWORD )

	ROM_REGION( 0x10000, "cpu2", 0 ) /* unused */
	ROM_LOAD( "epr-13908.xx", 0x00000, 0x8000, CRC(6228c1d2) SHA1(bd37fe775534fb94c9af80546948ce5f9c47bbf5) ) /* cabinet movement */
ROM_END

/**************************************************************************************************************************
    Jurassic Park - Conversion (Japan)
    not protected
     Game: 833-10544-03 JURASSIC PARK CVT
   ROM BD: 834-10545-03
*/
ROM_START( jparkjc )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x2( "epr-16400a.ic8",     0x000000, 0x80000, CRC(1e03dbfe) SHA1(b7c274769ff483e687749ff20b1dc0fc38e8ef82) )
	ROM_LOAD16_BYTE( "epr-16395.ic18", 0x100000, 0x80000, CRC(ac5a01d6) SHA1(df6bffdf5723cb8790a9c1c0ab271989a758bdd8) )
	ROM_LOAD16_BYTE( "epr-16394.ic9",  0x100001, 0x80000, CRC(c08c3a8a) SHA1(923cf256d863656336401fa75103b42298cb3822) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x4( "epr-16630.ic36", 0x100000, 0x040000, CRC(955855eb) SHA1(c7a325ba0009139c2cc263cd457dbc0d527c7582) )
	ROM_LOAD( "mpr-16398.ic35",    0x200000, 0x100000, CRC(fa710ca6) SHA1(1fd625070eef5f99d7be07606aeeff9282e32532) )
	ROM_LOAD( "mpr-16397.ic34",    0x300000, 0x100000, CRC(6e96e0be) SHA1(422b783b72127b80a23043b2dd1c04f5772f436e) )
	ROM_LOAD( "mpr-16396.ic24",    0x400000, 0x100000, CRC(f69a2dc4) SHA1(3f02b10976852916c58e852f3161a857784fe36b) )

	ROM_REGION( 0x400000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-16404.ic14", 0x000000, 0x200000, CRC(11283807) SHA1(99e465c3fc31e640740b8257a349e203f026754a) )
	ROM_LOAD16_BYTE( "mpr-16403.ic5",  0x000001, 0x200000, CRC(02530a9b) SHA1(b43e1b47f74c801bfc599cbe893fb8dc13453dd0) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-16405.ic32", 0x000000, 0x200000, CRC(b425f182) SHA1(66c6bd29dd3450db816b895c4c9c5208a66aae67) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16407.ic30", 0x000002, 0x200000, CRC(bc49ffd9) SHA1(a50ba7ddccfdfd7638c4041978b39c1559afbbb4) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16409.ic28", 0x000004, 0x200000, CRC(fe73660d) SHA1(ec1a3ea5303d2ccb9e327da18476969953626e1c) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16411.ic26", 0x000006, 0x200000, CRC(71cabbc5) SHA1(9760f57ef43eb251488dadd37711d5682d902434) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16406.ic31", 0x800000, 0x200000, CRC(b9ed73d6) SHA1(0dd22e7a21e95d84fc91acd742c737f96529f515) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16408.ic29", 0x800002, 0x200000, CRC(7b2f476b) SHA1(da99a9911982ba8aaef8c9aecc2977c9fd6da094) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16410.ic27", 0x800004, 0x200000, CRC(49c8f952) SHA1(f26b818711910b10bf520e5f849a1478a6b1d6e6) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16412.ic25", 0x800006, 0x200000, CRC(105dc26e) SHA1(fd2ef8c9fe1a78b4f9cc891a6fbd060184e58a1f) , ROM_SKIP(6)|ROM_GROUPWORD )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Kokoroji 2
    Sega System32 + CD - Sega 1993

    Rom Board is 837-8393 16Mb ROM board (Same as godenaxe2 or Arabian Fight)

    SCSI CD board is 839-0572-01. It use a Fujitsu MB89352AP for SCSI + a Sony CXD1095Q for I/O + 8Mhz quartz
*/
ROM_START( kokoroj2 )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x8( "epr-16186.ic8",     0x000000,  0x20000, CRC(8c3afb6e) SHA1(68c3c066a943b1ea8e3bee06c7c8279b5a12e7f7) )
	ROM_LOAD16_BYTE( "epr-16183.ic18", 0x100000, 0x80000, CRC(4844432f) SHA1(b127169d0f108e0b99ec81814a5c3c45bb82e0b1) )
	ROM_LOAD16_BYTE( "epr-16182.ic9",  0x100001, 0x80000, CRC(a27f5f5f) SHA1(c2bbd1632bce0851cf8ab45d3ccbec1076e67f5e) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x4( "epr-16185.ic36", 0x100000, 0x020000, CRC(afb97c4d) SHA1(f6e77d932824f93d89559a9cb3b2d678d5fc6940) )
	ROM_LOAD( "mpr-16184.ic35",    0x200000, 0x080000, CRC(dbd44a85) SHA1(e7341d2ef27c580bff365b5c546da2adb72faee8) )

	ROM_REGION( 0x400000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-16188.ic14", 0x000000, 0x200000, CRC(83a450ab) SHA1(1d0b45512d784ed1d82135b84c7c540f92d789f7) )
	ROM_LOAD16_BYTE( "mpr-16187.ic5",  0x000001, 0x200000, CRC(98b62f8b) SHA1(eaf98efd9eac7b7c385138a8a4dbc94b0ca38df5) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-16189.ic32", 0x000000, 0x200000, CRC(0937f713) SHA1(4b2b09ec8ed97794ad3824d1c57eae7f7e01379c) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16191.ic30", 0x000002, 0x200000, CRC(cfef4aaa) SHA1(bc8a252dcbdb8facdd91eda7aed0f56fe7529d15) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16193.ic28", 0x000004, 0x200000, CRC(a0706e4e) SHA1(1f36d952971c05db4190b229aa4957db3e5224f1) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16195.ic26", 0x000006, 0x200000, CRC(a4ddcd61) SHA1(90ef40f1fc84d1e4d4f78c33b8f0d1f56e04bf90) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16190.ic31", 0x800000, 0x200000, CRC(528d408e) SHA1(89f8a2cfc8b59377d6a65555c3172e457b131502) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16192.ic29", 0x800002, 0x200000, CRC(efaa93d1) SHA1(2947eaf7fc358ced1c04e7abe7a3f3066c73f2d0) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16194.ic27", 0x800004, 0x200000, CRC(39b5efe7) SHA1(2039909a2dd46951d442f1b6377f365525f9f2f1) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16196.ic25", 0x800006, 0x200000, CRC(b8e22e05) SHA1(dd667e2c5d421cba356421825e6aca9b5ca0af45) , ROM_SKIP(6)|ROM_GROUPWORD )

	/* AUDIO CD */
	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "cdp-00146", 0, SHA1(0b37e0ea2380ecd9abef2ccd6a8096d76d2ba344) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Outrunners (Export) - Multi-32
    not protected

    Sega Game ID codes:
     GAME BD NO. 834-9559-02
         ROM BD. 837-9560-02
        MAIN BD. 837-8676 (SYSTEM MULTI)
      A/D BD NO. 837-7536
        COMM BD. 837-8792
*/
ROM_START( orunners )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code */
	ROM_LOAD32_WORD_x4( "epr15620.ic37", 0x000000, 0x020000, CRC(84f5ad92) SHA1(1f9cb04b42b2d450be93400d9979a7910eaf05d1) )
	ROM_LOAD32_WORD_x4( "epr15621.ic40", 0x000002, 0x020000, CRC(d98b765a) SHA1(b58567e976228267a86af53de2135bc0b247a44a) )
	ROM_LOAD32_WORD( "mpr15538.ic36",   0x100000, 0x080000, CRC(93958820) SHA1(e19b6f18a5707dbb64ae009d63c05eac5bac4a81) )
	ROM_LOAD32_WORD( "mpr15539.ic39",   0x100002, 0x080000, CRC(219760fa) SHA1(bd62a83de9c9542f6da454a87dc4947492f65c52) )

	ROM_REGION( 0x180000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr15550.ic31", 0x100000, 0x80000, CRC(0205d2ed) SHA1(3475479e1a45fe96eefbe53842758898db7accbf) )

	ROM_REGION( 0x400000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr15548.ic3", 0x000000, 0x200000, CRC(b6470a66) SHA1(e1544590c02d41f62f82a4d771b893fb0f2734c7) )
	ROM_LOAD16_BYTE( "mpr15549.ic11", 0x000001, 0x200000, CRC(81d12520) SHA1(1555893941e832f00ad3d0b3ad0c34a0d3a1c58a) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr15540.ic14", 0x000000, 0x200000, CRC(a10d72b4) SHA1(6d9d5e20be6721b53ce49df4d5a1bbd91f5b3aed) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr15542.ic15", 0x000002, 0x200000, CRC(40952374) SHA1(c669ef52508bc2f49cf812dc86ac98fb535471fa) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr15544.ic10", 0x000004, 0x200000, CRC(39e3df45) SHA1(38a7b21617b45613b05509dda388f8f7770b186c) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr15546.ic38", 0x000006, 0x200000, CRC(e3fcc12c) SHA1(1cf7e05c7873f68789a27a91cddf471df40d7907) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr15541.ic22", 0x800000, 0x200000, CRC(a2003c2d) SHA1(200a2c7d78d3f5f28909267fdcdbddd58c5f5fa2) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr15543.ic23", 0x800002, 0x200000, CRC(933e8e7b) SHA1(0d53286f524f47851a483569dc37e9f6d34cc5f4) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr15545.ic18", 0x800004, 0x200000, CRC(53dd0235) SHA1(4aee5ae1820ff933b6bd8a54bdbf989c0bc95c1a) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr15547.ic41", 0x800006, 0x200000, CRC(edcb2a43) SHA1(f0bcfcc749ca0267f85bf9838164869912944d00) , ROM_SKIP(6)|ROM_GROUPWORD )

	ROM_REGION( 0x400000, "mainpcb:sega", 0 ) /* Sega PCM sound data */
	ROM_LOAD( "mpr15551.ic1", 0x000000, 0x200000, CRC(4894bc73) SHA1(351f5c03fb430fd87df915dfe3a377b5ada622c4) )
	ROM_LOAD( "mpr15552.ic2", 0x200000, 0x200000, CRC(1c4b5e73) SHA1(50a8e9a200575a3522a51bf094aa0e87b90bb0a3) )
ROM_END

/**************************************************************************************************************************
    Outrunners (US) - Multi-32
    not protected
     GAME BD NO. 834-9559-01
         ROM BD. 837-9560-01
        MAIN BD. 837-8676 (SYSTEM MULTI)
      A/D BD NO. 837-7536
        COMM BD. 837-8792
*/
ROM_START( orunnersu )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code */
	ROM_LOAD32_WORD_x4( "epr15618.ic37", 0x000000, 0x020000, CRC(25647f76) SHA1(9f882921ebb2f078350295c322b263f75812c053) )
	ROM_LOAD32_WORD_x4( "epr15619.ic40", 0x000002, 0x020000, CRC(2a558f95) SHA1(616ec0a7b251da61a49b933c58895b1a4d39417a) )
	ROM_LOAD32_WORD( "mpr15538.ic36",   0x100000, 0x080000, CRC(93958820) SHA1(e19b6f18a5707dbb64ae009d63c05eac5bac4a81) )
	ROM_LOAD32_WORD( "mpr15539.ic39",   0x100002, 0x080000, CRC(219760fa) SHA1(bd62a83de9c9542f6da454a87dc4947492f65c52) )

	ROM_REGION( 0x180000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr15550.ic31", 0x100000, 0x80000, CRC(0205d2ed) SHA1(3475479e1a45fe96eefbe53842758898db7accbf) )

	ROM_REGION( 0x400000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr15548.ic3", 0x000000, 0x200000, CRC(b6470a66) SHA1(e1544590c02d41f62f82a4d771b893fb0f2734c7) )
	ROM_LOAD16_BYTE( "mpr15549.ic11", 0x000001, 0x200000, CRC(81d12520) SHA1(1555893941e832f00ad3d0b3ad0c34a0d3a1c58a) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr15540.ic14", 0x000000, 0x200000, CRC(a10d72b4) SHA1(6d9d5e20be6721b53ce49df4d5a1bbd91f5b3aed) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr15542.ic15", 0x000002, 0x200000, CRC(40952374) SHA1(c669ef52508bc2f49cf812dc86ac98fb535471fa) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr15544.ic10", 0x000004, 0x200000, CRC(39e3df45) SHA1(38a7b21617b45613b05509dda388f8f7770b186c) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr15546.ic38", 0x000006, 0x200000, CRC(e3fcc12c) SHA1(1cf7e05c7873f68789a27a91cddf471df40d7907) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr15541.ic22", 0x800000, 0x200000, CRC(a2003c2d) SHA1(200a2c7d78d3f5f28909267fdcdbddd58c5f5fa2) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr15543.ic23", 0x800002, 0x200000, CRC(933e8e7b) SHA1(0d53286f524f47851a483569dc37e9f6d34cc5f4) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr15545.ic18", 0x800004, 0x200000, CRC(53dd0235) SHA1(4aee5ae1820ff933b6bd8a54bdbf989c0bc95c1a) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr15547.ic41", 0x800006, 0x200000, CRC(edcb2a43) SHA1(f0bcfcc749ca0267f85bf9838164869912944d00) , ROM_SKIP(6)|ROM_GROUPWORD )

	ROM_REGION( 0x400000, "mainpcb:sega", 0 ) /* Sega PCM sound data */
	ROM_LOAD( "mpr15551.ic1", 0x000000, 0x200000, CRC(4894bc73) SHA1(351f5c03fb430fd87df915dfe3a377b5ada622c4) )
	ROM_LOAD( "mpr15552.ic2", 0x200000, 0x200000, CRC(1c4b5e73) SHA1(50a8e9a200575a3522a51bf094aa0e87b90bb0a3) )
ROM_END

/**************************************************************************************************************************
    Outrunners (Japan) - Multi-32
    not protected
     GAME BD NO. 834-9559-03
         ROM BD. 837-9560-03
        MAIN BD. 837-8676 (SYSTEM MULTI)
      A/D BD NO. 837-7536
        COMM BD. 837-8792
*/
ROM_START( orunnersj )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code */
	ROM_LOAD32_WORD_x4( "epr15616.ic37", 0x000000, 0x020000, CRC(fb550545) SHA1(2f2c36843b115f5417e1f2ccd4a34ebf91265190) ) /* Need to verify the EPR numbers */
	ROM_LOAD32_WORD_x4( "epr15617.ic40", 0x000002, 0x020000, CRC(6bb741e0) SHA1(d92087a2c0b6de4287e569eecf9758615a85d1eb) ) /* Need to verify the EPR numbers */
	ROM_LOAD32_WORD( "mpr15538.ic36",   0x100000, 0x080000, CRC(93958820) SHA1(e19b6f18a5707dbb64ae009d63c05eac5bac4a81) )
	ROM_LOAD32_WORD( "mpr15539.ic39",   0x100002, 0x080000, CRC(219760fa) SHA1(bd62a83de9c9542f6da454a87dc4947492f65c52) )

	ROM_REGION( 0x180000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr15550.ic31", 0x100000, 0x80000, CRC(0205d2ed) SHA1(3475479e1a45fe96eefbe53842758898db7accbf) )

	ROM_REGION( 0x400000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr15548.ic3", 0x000000, 0x200000, CRC(b6470a66) SHA1(e1544590c02d41f62f82a4d771b893fb0f2734c7) )
	ROM_LOAD16_BYTE( "mpr15549.ic11", 0x000001, 0x200000, CRC(81d12520) SHA1(1555893941e832f00ad3d0b3ad0c34a0d3a1c58a) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr15540.ic14", 0x000000, 0x200000, CRC(a10d72b4) SHA1(6d9d5e20be6721b53ce49df4d5a1bbd91f5b3aed) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr15542.ic15", 0x000002, 0x200000, CRC(40952374) SHA1(c669ef52508bc2f49cf812dc86ac98fb535471fa) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr15544.ic10", 0x000004, 0x200000, CRC(39e3df45) SHA1(38a7b21617b45613b05509dda388f8f7770b186c) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr15546.ic38", 0x000006, 0x200000, CRC(e3fcc12c) SHA1(1cf7e05c7873f68789a27a91cddf471df40d7907) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr15541.ic22", 0x800000, 0x200000, CRC(a2003c2d) SHA1(200a2c7d78d3f5f28909267fdcdbddd58c5f5fa2) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr15543.ic23", 0x800002, 0x200000, CRC(933e8e7b) SHA1(0d53286f524f47851a483569dc37e9f6d34cc5f4) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr15545.ic18", 0x800004, 0x200000, CRC(53dd0235) SHA1(4aee5ae1820ff933b6bd8a54bdbf989c0bc95c1a) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr15547.ic41", 0x800006, 0x200000, CRC(edcb2a43) SHA1(f0bcfcc749ca0267f85bf9838164869912944d00) , ROM_SKIP(6)|ROM_GROUPWORD )

	ROM_REGION( 0x400000, "mainpcb:sega", 0 ) /* Sega PCM sound data */
	ROM_LOAD( "mpr15551.ic1", 0x000000, 0x200000, CRC(4894bc73) SHA1(351f5c03fb430fd87df915dfe3a377b5ada622c4) )
	ROM_LOAD( "mpr15552.ic2", 0x200000, 0x200000, CRC(1c4b5e73) SHA1(50a8e9a200575a3522a51bf094aa0e87b90bb0a3) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Rad Mobile Deluxe Cabinet (Export)
    not protected

    Sega Game ID codes:
    GAME BD NO. 833-7738-01 RAD MOBILE (USA)
                833-7738-02 RAD MOBILE (Export)
                833-7738-03 RAD MOBILE (Japan)
        ROM BD. 834-7739-01 (USA)
                834-7739-02 (Export)
                834-7739-03 (Japan)
      MAIN BD. 837-7428
    A/D BD NO. 837-7536

    Upright Cabinet:
  USA:  EPR-13690.ic21 (dumped)
        EPR-13691.ic37 (not dumped)
        EPR-13692.ic38 (not dumped)

Export: EPR-13693.ic21 (dumped)
        EPR-13694.ic37 (not dumped)
        EPR-13695.ic38 (not dumped)

    Japanese version is undumped. There is likely a Japanese specific sound rom at IC20 (EPR-13524.ic20 ??)
*/
ROM_START( radm )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x8( "epr-13693.ic21",     0x000000, 0x020000, CRC(3f09a211) SHA1(e0e011d7069745e9bf0395bc1375d0f8b9c46dab) )
	ROM_LOAD16_BYTE( "epr-13525.ic37", 0x100000, 0x080000, CRC(62ad83a0) SHA1(b537176ebca15d91db04d5d7ab36aa967d41288e) )
	ROM_LOAD16_BYTE( "epr-13526.ic38", 0x100001, 0x080000, CRC(59ea372a) SHA1(e7a5d59586652c59c23e07e0a99ecc740fb6144d) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x8( "epr-13527.ic9",  0x100000, 0x020000, CRC(a2e3fbbe) SHA1(2787bbef696ab3f2b7855ac991867837d3de54cd) )
	ROM_LOAD_x2( "epr-13523.ic14", 0x200000, 0x080000, CRC(d5563697) SHA1(eb3fd3dbfea383ac1bb5d2e1552723994cb4693d) )
	ROM_LOAD_x2( "epr-13699.ic20", 0x300000, 0x080000, CRC(33fd2913) SHA1(60b664559b4989446b1c7d875432e53a36fe27df) )
	ROM_LOAD_x2( "epr-13523.ic22", 0x400000, 0x080000, CRC(d5563697) SHA1(eb3fd3dbfea383ac1bb5d2e1552723994cb4693d) ) /* Deluxe or Upright manuals don't show this rom */

	ROM_REGION( 0x200000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD32_BYTE( "mpr-13519.ic3",  0x000000, 0x080000, CRC(bedc9534) SHA1(7b3f7a47b6c0ca6707dc3c1167f3564d43adb32f) )
	ROM_LOAD32_BYTE( "mpr-13520.ic7",  0x000002, 0x080000, CRC(3532e91a) SHA1(669c8d27b4b48e1ab9d6d30b0994f5a4e5169118) )
	ROM_LOAD32_BYTE( "mpr-13521.ic12", 0x000001, 0x080000, CRC(e9bca903) SHA1(18a73c830b9755262a1c525e3ad5ae084117b64d) )
	ROM_LOAD32_BYTE( "mpr-13522.ic18", 0x000003, 0x080000, CRC(25e04648) SHA1(617e794e8f7aa2a435bac917b8968699fe88dafb) )

	ROM_REGION32_BE( 0x800000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-13511.ic1",  0x000000, 0x100000, CRC(f8f15b11) SHA1(da6c2b8c3a94c4c263583f046823eaea818aff7c) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13512.ic5",  0x000001, 0x100000, CRC(d0be34a6) SHA1(b42a63e30f0f7a94de8a825ca93cf8efdb7a7648) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13513.ic10", 0x000002, 0x100000, CRC(feef1982) SHA1(bdf906317079a12c48ef4fca5bef0d437e9bf050) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13514.ic16", 0x000003, 0x100000, CRC(d0f9ebd1) SHA1(510ebd3d7a52bcab2debea61591770d1dff172a1) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13515.ic2",  0x000004, 0x100000, CRC(77bf2387) SHA1(7215dde5618e238edbe16b3007ede790785fe25f) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13516.ic6",  0x000005, 0x100000, CRC(8c4bc62d) SHA1(3206f623ec0b7558413d063404103b183f26b488) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13517.ic11", 0x000006, 0x100000, CRC(1d7d84a7) SHA1(954cfccfc7250a5bead2eeba42e655d5ac82955f) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13518.ic17", 0x000007, 0x100000, CRC(9ea4b15d) SHA1(7dcfd6d42bb945beca8344cf92e7bd53903a824b) , ROM_SKIP(7) )

	ROM_REGION( 0x8000, "user2", 0 ) /* unused (cabinet motor?) */
	ROM_LOAD( "epr-13686.bin", 0x00000, 0x8000, CRC(317a2857) SHA1(e0788dc7a7d214d9c4d26b24e44c1a0dc9ae477c) ) /* cabinet movement */

	ROM_REGION16_BE( 0x80, "mainpcb:eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-radm.ic76", 0x0000, 0x0080, CRC(b1737c06) SHA1(29448a6effeb53322a93158feb9a62bc6ad31f21) )
ROM_END

/**************************************************************************************************************************
    Rad Mobile Deluxe Cabinet (US)
    not protected
*/
ROM_START( radmu )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x8( "epr-13690.ic21",     0x000000, 0x020000, CRC(21637dec) SHA1(b9921effb10a72f3bdca4d540149c7f46662b716) )
	ROM_LOAD16_BYTE( "epr-13525.ic37", 0x100000, 0x080000, CRC(62ad83a0) SHA1(b537176ebca15d91db04d5d7ab36aa967d41288e) )
	ROM_LOAD16_BYTE( "epr-13526.ic38", 0x100001, 0x080000, CRC(59ea372a) SHA1(e7a5d59586652c59c23e07e0a99ecc740fb6144d) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x8( "epr-13527.ic9",  0x100000, 0x020000, CRC(a2e3fbbe) SHA1(2787bbef696ab3f2b7855ac991867837d3de54cd) )
	ROM_LOAD_x2( "epr-13523.ic14", 0x200000, 0x080000, CRC(d5563697) SHA1(eb3fd3dbfea383ac1bb5d2e1552723994cb4693d) )
	ROM_LOAD_x2( "epr-13699.ic20", 0x300000, 0x080000, CRC(33fd2913) SHA1(60b664559b4989446b1c7d875432e53a36fe27df) )
	ROM_LOAD_x2( "epr-13523.ic22", 0x400000, 0x080000, CRC(d5563697) SHA1(eb3fd3dbfea383ac1bb5d2e1552723994cb4693d) ) /* Deluxe or Upright manuals don't show this rom */

	ROM_REGION( 0x200000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD32_BYTE( "mpr-13519.ic3",  0x000000, 0x080000, CRC(bedc9534) SHA1(7b3f7a47b6c0ca6707dc3c1167f3564d43adb32f) )
	ROM_LOAD32_BYTE( "mpr-13520.ic7",  0x000002, 0x080000, CRC(3532e91a) SHA1(669c8d27b4b48e1ab9d6d30b0994f5a4e5169118) )
	ROM_LOAD32_BYTE( "mpr-13521.ic12", 0x000001, 0x080000, CRC(e9bca903) SHA1(18a73c830b9755262a1c525e3ad5ae084117b64d) )
	ROM_LOAD32_BYTE( "mpr-13522.ic18", 0x000003, 0x080000, CRC(25e04648) SHA1(617e794e8f7aa2a435bac917b8968699fe88dafb) )

	ROM_REGION32_BE( 0x800000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-13511.ic1",  0x000000, 0x100000, CRC(f8f15b11) SHA1(da6c2b8c3a94c4c263583f046823eaea818aff7c) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13512.ic5",  0x000001, 0x100000, CRC(d0be34a6) SHA1(b42a63e30f0f7a94de8a825ca93cf8efdb7a7648) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13513.ic10", 0x000002, 0x100000, CRC(feef1982) SHA1(bdf906317079a12c48ef4fca5bef0d437e9bf050) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13514.ic16", 0x000003, 0x100000, CRC(d0f9ebd1) SHA1(510ebd3d7a52bcab2debea61591770d1dff172a1) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13515.ic2",  0x000004, 0x100000, CRC(77bf2387) SHA1(7215dde5618e238edbe16b3007ede790785fe25f) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13516.ic6",  0x000005, 0x100000, CRC(8c4bc62d) SHA1(3206f623ec0b7558413d063404103b183f26b488) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13517.ic11", 0x000006, 0x100000, CRC(1d7d84a7) SHA1(954cfccfc7250a5bead2eeba42e655d5ac82955f) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13518.ic17", 0x000007, 0x100000, CRC(9ea4b15d) SHA1(7dcfd6d42bb945beca8344cf92e7bd53903a824b) , ROM_SKIP(7) )

	ROM_REGION( 0x8000, "user2", 0 ) /* unused (cabinet motor?) */
	ROM_LOAD( "epr-13686.bin", 0x00000, 0x8000, CRC(317a2857) SHA1(e0788dc7a7d214d9c4d26b24e44c1a0dc9ae477c) ) /* cabinet movement */

	ROM_REGION16_BE( 0x80, "mainpcb:eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-radm.ic76", 0x0000, 0x0080, CRC(b1737c06) SHA1(29448a6effeb53322a93158feb9a62bc6ad31f21) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Rad Rally (Export)
    not protected

    Sega Game ID codes:
     Game: 833-8110-02 RAD RALLY
Rom board: 833-8111-02
A/D BD NO. 837-7536

*/
ROM_START( radr )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x8( "epr-14241.ic21",     0x000000, 0x020000, CRC(59a5f63d) SHA1(325a26a09475ddc828de71e71a1d3043f3959cec) )
	ROM_LOAD16_BYTE( "epr-14106.ic37", 0x100000, 0x080000, CRC(e73c63bf) SHA1(30fb68eaa7d02a232c873bd7751cac7d0fa08e44) )
	ROM_LOAD16_BYTE( "epr-14107.ic38", 0x100001, 0x080000, CRC(832f797a) SHA1(b0c16ef7bd8d37f592975052ba9da3da70a2fc79) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x8( "epr-14108.ic9",  0x100000, 0x020000, CRC(38a99b4d) SHA1(b6455e6b29bfef41c5e0ebe3a8064889b7e5f5fd) )
	ROM_LOAD_x2( "epr-14109.ic14", 0x200000, 0x080000, CRC(b42e5833) SHA1(da94ce7c1d7a581a1aa6b79b323c67a919918808) )
	ROM_LOAD_x2( "epr-14110.ic20", 0x300000, 0x080000, CRC(b495e7dc) SHA1(b4143fcee10e0649378fdb1e3f5a0a2c585414ec) )
	ROM_LOAD_x2( "epr-14237.ic22", 0x400000, 0x080000, CRC(0a4b4b29) SHA1(98447a587f903ba03e17d6a145b7c8bfddf25c4d) )

	ROM_REGION( 0x100000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD32_BYTE( "epr-14102.ic3",  0x000000, 0x040000, CRC(5626e80f) SHA1(9844817295a8cd8a9b09da6681b0c1fbfe82618e) )
	ROM_LOAD32_BYTE( "epr-14103.ic7",  0x000002, 0x040000, CRC(08c7e804) SHA1(cf45b1934edc43cb3a0ed72159949cb0dd00d701) )
	ROM_LOAD32_BYTE( "epr-14104.ic12", 0x000001, 0x040000, CRC(b0173646) SHA1(1ba4edc033e0e4f5a1e02987e9f6b8b1650b46d7) )
	ROM_LOAD32_BYTE( "epr-14105.ic18", 0x000003, 0x040000, CRC(614843b6) SHA1(d4f2cd3b024f7152d6e89237f0da06adea2efe57) )

	ROM_REGION32_BE( 0x800000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-13511.ic1",  0x000000, 0x100000, CRC(f8f15b11) SHA1(da6c2b8c3a94c4c263583f046823eaea818aff7c) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13512.ic5",  0x000001, 0x100000, CRC(d0be34a6) SHA1(b42a63e30f0f7a94de8a825ca93cf8efdb7a7648) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13513.ic10", 0x000002, 0x100000, CRC(feef1982) SHA1(bdf906317079a12c48ef4fca5bef0d437e9bf050) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13514.ic16", 0x000003, 0x100000, CRC(d0f9ebd1) SHA1(510ebd3d7a52bcab2debea61591770d1dff172a1) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13515.ic2",  0x000004, 0x100000, CRC(77bf2387) SHA1(7215dde5618e238edbe16b3007ede790785fe25f) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13516.ic6",  0x000005, 0x100000, CRC(8c4bc62d) SHA1(3206f623ec0b7558413d063404103b183f26b488) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13517.ic11", 0x000006, 0x100000, CRC(1d7d84a7) SHA1(954cfccfc7250a5bead2eeba42e655d5ac82955f) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13518.ic17", 0x000007, 0x100000, CRC(9ea4b15d) SHA1(7dcfd6d42bb945beca8344cf92e7bd53903a824b) , ROM_SKIP(7) )

	ROM_REGION( 0x8000, "user2", 0 ) /* unused */
	ROM_LOAD( "epr-14084.17", 0x00000, 0x8000, CRC(f14ed074) SHA1(e1bb23eac85e3236046527c5c7688f6f23d43aef) ) /* cabinet link */

	ROM_REGION16_BE( 0x80, "mainpcb:eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-radr.ic76", 0x0000, 0x0080, CRC(602032c6) SHA1(fecf14017e537fe870457d2a8d4f86ec6d442b90) )
ROM_END

/**************************************************************************************************************************
    Rad Rally (US)
    not protected

    Sega Game ID codes:
     Game: 833-8110-01 RAD RALLY
Rom board: 833-8111-01
A/D BD NO. 837-7536

*/
ROM_START( radru )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x8( "epr-14240.ic21",     0x000000, 0x020000, CRC(8473e7ab) SHA1(fbd883dc804d7de6ce239b68c6e6ae3a54e9e03c) )
	ROM_LOAD16_BYTE( "epr-14106.ic37", 0x100000, 0x080000, CRC(e73c63bf) SHA1(30fb68eaa7d02a232c873bd7751cac7d0fa08e44) )
	ROM_LOAD16_BYTE( "epr-14107.ic38", 0x100001, 0x080000, CRC(832f797a) SHA1(b0c16ef7bd8d37f592975052ba9da3da70a2fc79) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x8( "epr-14108.ic9",  0x100000, 0x020000, CRC(38a99b4d) SHA1(b6455e6b29bfef41c5e0ebe3a8064889b7e5f5fd) )
	ROM_LOAD_x2( "epr-14109.ic14", 0x200000, 0x080000, CRC(b42e5833) SHA1(da94ce7c1d7a581a1aa6b79b323c67a919918808) )
	ROM_LOAD_x2( "epr-14110.ic20", 0x300000, 0x080000, CRC(b495e7dc) SHA1(b4143fcee10e0649378fdb1e3f5a0a2c585414ec) )
	ROM_LOAD_x2( "epr-14237.ic22", 0x400000, 0x080000, CRC(0a4b4b29) SHA1(98447a587f903ba03e17d6a145b7c8bfddf25c4d) )

	ROM_REGION( 0x100000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD32_BYTE( "epr-14102.ic3",  0x000000, 0x040000, CRC(5626e80f) SHA1(9844817295a8cd8a9b09da6681b0c1fbfe82618e) )
	ROM_LOAD32_BYTE( "epr-14103.ic7",  0x000002, 0x040000, CRC(08c7e804) SHA1(cf45b1934edc43cb3a0ed72159949cb0dd00d701) )
	ROM_LOAD32_BYTE( "epr-14104.ic12", 0x000001, 0x040000, CRC(b0173646) SHA1(1ba4edc033e0e4f5a1e02987e9f6b8b1650b46d7) )
	ROM_LOAD32_BYTE( "epr-14105.ic16", 0x000003, 0x040000, CRC(614843b6) SHA1(d4f2cd3b024f7152d6e89237f0da06adea2efe57) )

	ROM_REGION32_BE( 0x800000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-13511.ic1",  0x000000, 0x100000, CRC(f8f15b11) SHA1(da6c2b8c3a94c4c263583f046823eaea818aff7c) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13512.ic5",  0x000001, 0x100000, CRC(d0be34a6) SHA1(b42a63e30f0f7a94de8a825ca93cf8efdb7a7648) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13513.ic10", 0x000002, 0x100000, CRC(feef1982) SHA1(bdf906317079a12c48ef4fca5bef0d437e9bf050) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13514.ic16", 0x000003, 0x100000, CRC(d0f9ebd1) SHA1(510ebd3d7a52bcab2debea61591770d1dff172a1) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13515.ic2",  0x000004, 0x100000, CRC(77bf2387) SHA1(7215dde5618e238edbe16b3007ede790785fe25f) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13516.ic6",  0x000005, 0x100000, CRC(8c4bc62d) SHA1(3206f623ec0b7558413d063404103b183f26b488) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13517.ic11", 0x000006, 0x100000, CRC(1d7d84a7) SHA1(954cfccfc7250a5bead2eeba42e655d5ac82955f) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13518.ic17", 0x000007, 0x100000, CRC(9ea4b15d) SHA1(7dcfd6d42bb945beca8344cf92e7bd53903a824b) , ROM_SKIP(7) )

	ROM_REGION( 0x8000, "user2", 0 ) /* unused */
	ROM_LOAD( "epr-14084.17", 0x00000, 0x8000, CRC(f14ed074) SHA1(e1bb23eac85e3236046527c5c7688f6f23d43aef) ) /* cabinet link */

	ROM_REGION16_BE( 0x80, "mainpcb:eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-radr.ic76", 0x0000, 0x0080, CRC(602032c6) SHA1(fecf14017e537fe870457d2a8d4f86ec6d442b90) )
ROM_END

/**************************************************************************************************************************
    Rad Rally (Japan)
    not protected

    Sega Game ID codes:
     Game: 833-8110-03 RAD RALLY
Rom board: 833-8111-03
A/D BD NO. 837-7536

*/
ROM_START( radrj )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x8( "epr-14111.ic21",     0x000000, 0x020000, CRC(7adc6d17) SHA1(fc312a30b077ba060b6d98ab6ecccd2e16b32fc2) )
	ROM_LOAD16_BYTE( "epr-14106.ic37", 0x100000, 0x080000, CRC(e73c63bf) SHA1(30fb68eaa7d02a232c873bd7751cac7d0fa08e44) )
	ROM_LOAD16_BYTE( "epr-14107.ic38", 0x100001, 0x080000, CRC(832f797a) SHA1(b0c16ef7bd8d37f592975052ba9da3da70a2fc79) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x8( "epr-14108.ic9",  0x100000, 0x020000, CRC(38a99b4d) SHA1(b6455e6b29bfef41c5e0ebe3a8064889b7e5f5fd) )
	ROM_LOAD_x2( "epr-14109.ic14", 0x200000, 0x080000, CRC(b42e5833) SHA1(da94ce7c1d7a581a1aa6b79b323c67a919918808) )
	ROM_LOAD_x2( "epr-14110.ic20", 0x300000, 0x080000, CRC(b495e7dc) SHA1(b4143fcee10e0649378fdb1e3f5a0a2c585414ec) )
	ROM_LOAD_x2( "epr-14237.ic22", 0x400000, 0x080000, CRC(0a4b4b29) SHA1(98447a587f903ba03e17d6a145b7c8bfddf25c4d) )

	ROM_REGION( 0x100000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD32_BYTE( "epr-14102.ic3",  0x000000, 0x040000, CRC(5626e80f) SHA1(9844817295a8cd8a9b09da6681b0c1fbfe82618e) )
	ROM_LOAD32_BYTE( "epr-14103.ic7",  0x000002, 0x040000, CRC(08c7e804) SHA1(cf45b1934edc43cb3a0ed72159949cb0dd00d701) )
	ROM_LOAD32_BYTE( "epr-14104.ic12", 0x000001, 0x040000, CRC(b0173646) SHA1(1ba4edc033e0e4f5a1e02987e9f6b8b1650b46d7) )
	ROM_LOAD32_BYTE( "epr-14105.ic16", 0x000003, 0x040000, CRC(614843b6) SHA1(d4f2cd3b024f7152d6e89237f0da06adea2efe57) )

	ROM_REGION32_BE( 0x800000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-13511.ic1",  0x000000, 0x100000, CRC(f8f15b11) SHA1(da6c2b8c3a94c4c263583f046823eaea818aff7c) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13512.ic5",  0x000001, 0x100000, CRC(d0be34a6) SHA1(b42a63e30f0f7a94de8a825ca93cf8efdb7a7648) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13513.ic10", 0x000002, 0x100000, CRC(feef1982) SHA1(bdf906317079a12c48ef4fca5bef0d437e9bf050) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13514.ic16", 0x000003, 0x100000, CRC(d0f9ebd1) SHA1(510ebd3d7a52bcab2debea61591770d1dff172a1) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13515.ic2",  0x000004, 0x100000, CRC(77bf2387) SHA1(7215dde5618e238edbe16b3007ede790785fe25f) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13516.ic6",  0x000005, 0x100000, CRC(8c4bc62d) SHA1(3206f623ec0b7558413d063404103b183f26b488) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13517.ic11", 0x000006, 0x100000, CRC(1d7d84a7) SHA1(954cfccfc7250a5bead2eeba42e655d5ac82955f) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-13518.ic17", 0x000007, 0x100000, CRC(9ea4b15d) SHA1(7dcfd6d42bb945beca8344cf92e7bd53903a824b) , ROM_SKIP(7) )

	ROM_REGION( 0x8000, "user2", 0 ) /* unused */
	ROM_LOAD( "epr-14084.17", 0x00000, 0x8000, CRC(f14ed074) SHA1(e1bb23eac85e3236046527c5c7688f6f23d43aef) ) /* cabinet link */

	ROM_REGION16_BE( 0x80, "mainpcb:eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-radr.ic76", 0x0000, 0x0080, CRC(602032c6) SHA1(fecf14017e537fe870457d2a8d4f86ec6d442b90) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Slipstream (Brazil)

    GAME BD NO. 833-7429-01
     1. ROM BD. 837-7429-01
     2. MAIN BD. 837-7428
    A/D BD NO. 837-7536
*/
ROM_START( slipstrm )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x2( "s32b_prg01.ic6",    0x000000, 0x080000, CRC(7d066307) SHA1(d87e04167263b435b77830db02ed58651ccc020c) )
	ROM_LOAD16_BYTE( "s32_dat00.ic14",0x100000, 0x080000, CRC(c3ff6309) SHA1(dcc857736fe0f15aa7909c3ee88a7e239c8f0228) )
	ROM_LOAD16_BYTE( "s32_dat01.ic7", 0x100001, 0x080000, CRC(0e605c81) SHA1(47c64195cab9a07b234d5a375d26168e53ffaa17) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x8( "s32_snd00.ic35", 0x100000, 0x020000, CRC(0fee2278) SHA1(7533a03c3fc46d65dfdd07bddf1e6e0bbc368752) )
	ROM_LOAD_x2( "s32_snd01.ic31", 0x200000, 0x080000, CRC(ae7be5f2) SHA1(ba089355e64864435bcc3b0c208e4bce1ea66295) )

	ROM_REGION( 0x200000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD32_BYTE( "s32_scr00.ic38", 0x000000, 0x080000, CRC(3cbb2d0b) SHA1(b94006347b72cd60a889b0e279f62f677cedfd2e) )
	ROM_LOAD32_BYTE( "s32_scr01.ic34", 0x000002, 0x080000, CRC(4167be55) SHA1(96b34d311b318c00c3fad917e341589a70ba0a15) )
	ROM_LOAD32_BYTE( "s32_scr02.ic29", 0x000001, 0x080000, CRC(52c4bb85) SHA1(4fbee1072a19c75c25b5fd269acc75640923d69c) )
	ROM_LOAD32_BYTE( "s32_scr03.ic25", 0x000003, 0x080000, CRC(4948604a) SHA1(d5a1b9781fef7976a59a0af9b755a04fcacf9381) )

	ROM_REGION32_BE( 0x400000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "s32_obj00.ic36", 0x000000, 0x80000, CRC(cffe9e0d) SHA1(5272d54ff142de927a9abd61f3646e963c7d22c4) , ROM_SKIP(7) )
	ROMX_LOAD( "s32_obj01.ic32", 0x000001, 0x80000, CRC(4ebd1383) SHA1(ce35f4d15e7904bfde55e58cdde925cba8002763) , ROM_SKIP(7) )
	ROMX_LOAD( "s32_obj02.ic27", 0x000002, 0x80000, CRC(b3cf4fe2) SHA1(e13199522e1e3e8b9cfe72cc29b33f25dad542ef) , ROM_SKIP(7) )
	ROMX_LOAD( "s32_obj03.ic23", 0x000003, 0x80000, CRC(c6345391) SHA1(155758097911ffca0c5c0b2a24a8033339dcfcbb) , ROM_SKIP(7) )
	ROMX_LOAD( "s32_obj04.ic37", 0x000004, 0x80000, CRC(2de4288e) SHA1(8e794f79f506293edb7609187a7908516ce76849) , ROM_SKIP(7) )
	ROMX_LOAD( "s32_obj05.ic33", 0x000005, 0x80000, CRC(6cfb74fb) SHA1(b74c886959910cd069427418525b23300a9b7b18) , ROM_SKIP(7) )
	ROMX_LOAD( "s32_obj06.ic28", 0x000006, 0x80000, CRC(53234bf4) SHA1(1eca538dcb86e44c31310ab1ab42a2b66b69c8fe) , ROM_SKIP(7) )
	ROMX_LOAD( "s32_obj07.ic24", 0x000007, 0x80000, CRC(22c129cf) SHA1(0f64680511a357038f6a556253c13fbb5417dd1a) , ROM_SKIP(7) )
ROM_END

/**************************************************************************************************************************
    Slipstream (Hispanic)

    GAME BD NO. 833-7429-01
     1. ROM BD. 837-7429-01
     2. MAIN BD. 837-7428
    A/D BD NO. 837-7536
*/
ROM_START( slipstrmh )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x2( "s32h_prg01.ic6",    0x000000, 0x080000, CRC(ab778297) SHA1(e440d44b20f2f7478ef7d86af90af5eb7b9a545a) )
	ROM_LOAD16_BYTE( "s32_dat00.ic14",0x100000, 0x080000, CRC(c3ff6309) SHA1(dcc857736fe0f15aa7909c3ee88a7e239c8f0228) )
	ROM_LOAD16_BYTE( "s32_dat01.ic7", 0x100001, 0x080000, CRC(0e605c81) SHA1(47c64195cab9a07b234d5a375d26168e53ffaa17) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x8( "s32_snd00.ic35", 0x100000, 0x020000, CRC(0fee2278) SHA1(7533a03c3fc46d65dfdd07bddf1e6e0bbc368752) )
	ROM_LOAD_x2( "s32_snd01.ic31", 0x200000, 0x080000, CRC(ae7be5f2) SHA1(ba089355e64864435bcc3b0c208e4bce1ea66295) )

	ROM_REGION( 0x200000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD32_BYTE( "s32_scr00.ic38", 0x000000, 0x080000, CRC(3cbb2d0b) SHA1(b94006347b72cd60a889b0e279f62f677cedfd2e) )
	ROM_LOAD32_BYTE( "s32_scr01.ic34", 0x000002, 0x080000, CRC(4167be55) SHA1(96b34d311b318c00c3fad917e341589a70ba0a15) )
	ROM_LOAD32_BYTE( "s32_scr02.ic29", 0x000001, 0x080000, CRC(52c4bb85) SHA1(4fbee1072a19c75c25b5fd269acc75640923d69c) )
	ROM_LOAD32_BYTE( "s32_scr03.ic25", 0x000003, 0x080000, CRC(4948604a) SHA1(d5a1b9781fef7976a59a0af9b755a04fcacf9381) )

	ROM_REGION32_BE( 0x400000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "s32_obj00.ic36", 0x000000, 0x80000, CRC(cffe9e0d) SHA1(5272d54ff142de927a9abd61f3646e963c7d22c4) , ROM_SKIP(7) )
	ROMX_LOAD( "s32_obj01.ic32", 0x000001, 0x80000, CRC(4ebd1383) SHA1(ce35f4d15e7904bfde55e58cdde925cba8002763) , ROM_SKIP(7) )
	ROMX_LOAD( "s32_obj02.ic27", 0x000002, 0x80000, CRC(b3cf4fe2) SHA1(e13199522e1e3e8b9cfe72cc29b33f25dad542ef) , ROM_SKIP(7) )
	ROMX_LOAD( "s32_obj03.ic23", 0x000003, 0x80000, CRC(c6345391) SHA1(155758097911ffca0c5c0b2a24a8033339dcfcbb) , ROM_SKIP(7) )
	ROMX_LOAD( "s32_obj04.ic37", 0x000004, 0x80000, CRC(2de4288e) SHA1(8e794f79f506293edb7609187a7908516ce76849) , ROM_SKIP(7) )
	ROMX_LOAD( "s32_obj05.ic33", 0x000005, 0x80000, CRC(6cfb74fb) SHA1(b74c886959910cd069427418525b23300a9b7b18) , ROM_SKIP(7) )
	ROMX_LOAD( "s32_obj06.ic28", 0x000006, 0x80000, CRC(53234bf4) SHA1(1eca538dcb86e44c31310ab1ab42a2b66b69c8fe) , ROM_SKIP(7) )
	ROMX_LOAD( "s32_obj07.ic24", 0x000007, 0x80000, CRC(22c129cf) SHA1(0f64680511a357038f6a556253c13fbb5417dd1a) , ROM_SKIP(7) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Sonic the Hedgehog - Protected (FD1149  SEGA 317-0213)

Sega ROM BD ID# 834-9496
        ROM BD. 837-8393-01
     I/O BD NO. 837-8685

*/
ROM_START( sonic )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x4( "epr-15787c.ic17",        0x000000, 0x020000, CRC(25e3c27e) SHA1(8f173cd5c7c817dcccdcad9be5781cfaa081d73e) )
	ROM_LOAD_x4( "epr-15786c.ic8",         0x080000, 0x020000, CRC(efe9524c) SHA1(8020e734704a8f989919ee5ad92f70035de717f0) )
	ROM_LOAD16_BYTE_x2( "epr-15781c.ic18", 0x100000, 0x040000, CRC(65b06c25) SHA1(9f524012a7adbc71737f90fc556f0ce9adc2bcf8) )
	ROM_LOAD16_BYTE_x2( "epr-15780c.ic9",  0x100001, 0x040000, CRC(2db66fd2) SHA1(54582c0d5977649a38fc3a2c0fe4d7b1959abc76) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x4( "epr-15785.ic36", 0x100000, 0x040000, CRC(0fe7422e) SHA1(b7eaf4736ba155965317bb4ef3b33fc122635151) )
	ROM_LOAD( "mpr-15784.ic35",    0x200000, 0x100000, CRC(42f06714) SHA1(30e45bb2d9b492f0c1acc4fbe1e5869f0559300b) )
	ROM_LOAD( "mpr-15783.ic34",    0x300000, 0x100000, CRC(e4220eea) SHA1(a546c8bfc24e0695cf79c49e1a867d2595a1ed7f) )
	ROM_LOAD( "mpr-15782.ic33",    0x400000, 0x100000, CRC(cf56b5a0) SHA1(5786228aab120c3361524ba93b418b24fd5b8ffb) ) // (this is the only rom unchanged from the prototype)

	ROM_REGION( 0x200000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-15789.ic14", 0x000000, 0x100000, CRC(4378f12b) SHA1(826e0550a3c5f2b6e59c6531ac03658a4f826651) )
	ROM_LOAD16_BYTE( "mpr-15788.ic5",  0x000001, 0x100000, CRC(a6ed5d7a) SHA1(d30f26b452d380e7657e044e144f7dbbc4dc13e5) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-15790.ic32", 0x000000, 0x200000, CRC(c69d51b1) SHA1(7644fb64457855f9ed87ca25ddc28c21bcb61fd9) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15792.ic30", 0x000002, 0x200000, CRC(1006bb67) SHA1(38c752e634aa94b1a23c09c4dba6388b7d0358af) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15794.ic28", 0x000004, 0x200000, CRC(8672b480) SHA1(61659e3856cdff0b2bca190a7e60c81b86ea2089) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15796.ic26", 0x000006, 0x200000, CRC(95b8011c) SHA1(ebc56ae49a76d04de60b0f81506769219a9885a7) , ROM_SKIP(6)|ROM_GROUPWORD )

	// NOTE: these last 4 are in fact 16 megabit ROMs,
	// but they were dumped as 8 because the top half
	// is "FF" in all of them.
	ROMX_LOAD( "mpr-15791.ic31", 0x800000, 0x100000, CRC(42217066) SHA1(46d14c632da1bed02bc0a637e34ab9cbf356c5de) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15793.ic29", 0x800002, 0x100000, CRC(75bafe55) SHA1(ad33dae062c4bdf8d17d3f6f7c333aa2e7da260e) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15795.ic27", 0x800004, 0x100000, CRC(7f3dad30) SHA1(84be1c31df35e1c7fef77e83d6d135378789a1ef) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15797.ic25", 0x800006, 0x100000, CRC(013c6cab) SHA1(eb9b77d28815d2e225b0882706084a52b11c48ea) , ROM_SKIP(6)|ROM_GROUPWORD )
ROM_END

/**************************************************************************************************************************
    Sonic the Hedgehog (prototype)
    not protected
*/
ROM_START( sonicp )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x4( "sonpg0.bin",        0x000000, 0x20000, CRC(da05dcbb) SHA1(c2ced1f3aee92b0e531d5cd7611d4811f2ae95e7) )
	ROM_LOAD_x4( "sonpg1.bin",        0x080000, 0x20000, CRC(c57dc5c5) SHA1(5741bdd52ee7181d883129885838b36f4af8a04c) )
	ROM_LOAD16_BYTE_x2( "sonpd0.bin", 0x100000, 0x40000, CRC(a7da7546) SHA1(0a10573b21cd38d58380698bc18b0256dbb24044) )
	ROM_LOAD16_BYTE_x2( "sonpd1.bin", 0x100001, 0x40000, CRC(c30e4c70) SHA1(897b6f62921694fe3c63677908f76eaf38b7b92f) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x4( "sonsnd0.bin", 0x100000, 0x040000, CRC(569c8d4b) SHA1(9f1f6da6adbea043cc5ad853806fcb7bf683c832) )
	ROM_LOAD( "sonsnd1.bin",    0x200000, 0x100000, CRC(f4fa5a21) SHA1(14a364ba7744ff0b44423d8d6bab990fe534ff29) )
	ROM_LOAD( "sonsnd2.bin",    0x300000, 0x100000, CRC(e1bd45a5) SHA1(b411757853d61588e5223b48b5124cc00b3d65dd) )
	ROM_LOAD( "sonsnd3.bin",    0x400000, 0x100000, CRC(cf56b5a0) SHA1(5786228aab120c3361524ba93b418b24fd5b8ffb) )

	ROM_REGION( 0x200000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD32_BYTE( "sonscl0.bin", 0x000000, 0x080000, CRC(445e31b9) SHA1(5678dfda74a09b5ac673448b222d11df4ca23aff) )
	ROM_LOAD32_BYTE( "sonscl1.bin", 0x000002, 0x080000, CRC(3d234181) SHA1(2e8c14ad36be76f5f5fc6a3ee152f1abc8bf0ddd) )
	ROM_LOAD32_BYTE( "sonscl2.bin", 0x000001, 0x080000, CRC(a5de28b2) SHA1(49a16ac10cf01b5b8802b8b015a2e403086c206a) )
	ROM_LOAD32_BYTE( "sonscl3.bin", 0x000003, 0x080000, CRC(7ce7554b) SHA1(8def3acae6baafbe9e350f18e245a9a833df5cc4) )

	ROM_REGION32_BE( 0x800000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "sonobj0.bin", 0x000000, 0x100000, CRC(ceea18e3) SHA1(f902a7e2f8e126fd7a7862c55de32ce6352a7716) , ROM_SKIP(7) )
	ROMX_LOAD( "sonobj1.bin", 0x000001, 0x100000, CRC(6bbc226b) SHA1(5ef4256b6a93891daf1349def6db3bc428e5f4f3) , ROM_SKIP(7) )
	ROMX_LOAD( "sonobj2.bin", 0x000002, 0x100000, CRC(fcd5ef0e) SHA1(e3e50d4838ac3cce41d69ee6cd31981fbe422a4b) , ROM_SKIP(7) )
	ROMX_LOAD( "sonobj3.bin", 0x000003, 0x100000, CRC(b99b42ab) SHA1(60d91dc4e8e0adc62809cd2e71833c658124fbfc) , ROM_SKIP(7) )
	ROMX_LOAD( "sonobj4.bin", 0x000004, 0x100000, CRC(c7ec1456) SHA1(d866b9dff546bd6feb43e317328ac0a2324303b9) , ROM_SKIP(7) )
	ROMX_LOAD( "sonobj5.bin", 0x000005, 0x100000, CRC(bd5da27f) SHA1(ab3043190a32b555513a29a70e01547daf698cf8) , ROM_SKIP(7) )
	ROMX_LOAD( "sonobj6.bin", 0x000006, 0x100000, CRC(313c92d1) SHA1(a5134750667502811fd755cc0974a744cdb785e1) , ROM_SKIP(7) )
	ROMX_LOAD( "sonobj7.bin", 0x000007, 0x100000, CRC(3784c507) SHA1(8ea58c52b09b84643218e26f1ec1fa0ea864346e) , ROM_SKIP(7) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Spiderman (Export)
    not protected
*/
ROM_START( spidman )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x4( "epr-14307.ic13",        0x000000, 0x020000, CRC(d900219c) SHA1(d59654db1fc0ec4d5f8cda9000ab4bd3bb36cdfc) )
	ROM_LOAD_x4( "epr-14306.ic7",         0x080000, 0x020000, CRC(64379dc6) SHA1(7efc7175351186c54f141161a395e63b1cc7e7a5) )
	ROM_LOAD16_BYTE_x4( "epr-14281.ic14", 0x100000, 0x020000, CRC(8a746c42) SHA1(fa3729ec3aa4b3c59322408146ce2cfbf5a11b98) )
	ROM_LOAD16_BYTE_x4( "epr-14280.ic6",  0x100001, 0x020000, CRC(3c8148f7) SHA1(072b7982bb95e7a9ab77844b59020146c262488d) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x4( "epr-14285.ic35", 0x100000, 0x040000, CRC(25aefad6) SHA1(10153f4e773a0f55378f869eb1d85156e85f893f) )
	ROM_LOAD_x2( "mpr-14284.ic31", 0x200000, 0x080000, CRC(760542d4) SHA1(dcac73869c02fefd328bd6bdbcbdb3b68b0647da) )
	ROM_LOAD_x2( "mpr-14283.ic26", 0x300000, 0x080000, CRC(c863a91c) SHA1(afdc76bbb9b207cfcb47d437248a757d03212f4e) )
	ROM_LOAD_x2( "mpr-14282.ic22", 0x400000, 0x080000, CRC(ea20979e) SHA1(9b70ef055da8c7c56da54b7edef2379678e7c50f) )

	ROM_REGION( 0x400000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD32_BYTE( "mpr-14291-s.ic38", 0x000000, 0x100000, CRC(490f95a1) SHA1(f220788670b76164ac414ed9b16a422f719be267) )
	ROM_LOAD32_BYTE( "mpr-14290-s.ic34", 0x000002, 0x100000, CRC(a144162d) SHA1(d43f12dd9f690cdfcebb6c7b515ff7dc7dcaa377) )
	ROM_LOAD32_BYTE( "mpr-14289-s.ic29", 0x000001, 0x100000, CRC(38570582) SHA1(a9d810a02a1f5a6849c79d65fbebff21a4b82b59) )
	ROM_LOAD32_BYTE( "mpr-14288-s.ic25", 0x000003, 0x100000, CRC(3188b636) SHA1(bc0adeeca5040caa563ee1e0eded9c323ca23446) )

	ROM_REGION32_BE( 0x800000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-14299-h.ic36", 0x000000, 0x100000, CRC(ce59231b) SHA1(bcb1f11b74935694d0617ec8df66db2cc57b6219) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14298-h.ic32", 0x000001, 0x100000, CRC(2745c84c) SHA1(5a0528c921cba7a1047d3a2ece79925103d719a1) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14297-h.ic27", 0x000002, 0x100000, CRC(29cb9450) SHA1(7dc38d23a2f0cee2f4edde05c1a6f0dc83f331db) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14296-h.ic23", 0x000003, 0x100000, CRC(9d8cbd31) SHA1(55a9f9ec9029157da033e69664b58e694a28db47) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14295-h.ic37", 0x000004, 0x100000, CRC(29591f50) SHA1(1ac4ceaf74892e30f210ad77024eb441c5e4a959) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14294-h.ic33", 0x000005, 0x100000, CRC(fa86b794) SHA1(7b6907e5734fbf2fba7bcc213a8551fec5e9f3d5) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14293-s.ic28", 0x000006, 0x100000, CRC(52899269) SHA1(ff809ff88701109e0ca79e785a61402d97335cec) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14292-s.ic24", 0x000007, 0x100000, CRC(41f71443) SHA1(351d40d6159cb5b792519bce5d16490965800cfb) , ROM_SKIP(7) )
ROM_END

/**************************************************************************************************************************
    Spiderman (U.S.)
    not protected

    Sega Game ID codes:
     Game: 833-8331-04 SPIDER-MAN
Rom board: 834-8332-01

 Rom board type: 837-7429-01
Input sub board: 837-7968

*/
ROM_START( spidmanu )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x4( "epr-14303a.ic13",       0x000000, 0x020000, CRC(7f1bd28f) SHA1(cff57e66d09682baf44aace99d698ad305f6a3d5) )
	ROM_LOAD_x4( "epr-14302a.ic7",        0x080000, 0x020000, CRC(d954c40a) SHA1(436c81779274861de79dc6ce2c0fcc65bfd52098) )
	ROM_LOAD16_BYTE_x4( "epr-14281.ic14", 0x100000, 0x020000, CRC(8a746c42) SHA1(fa3729ec3aa4b3c59322408146ce2cfbf5a11b98) )
	ROM_LOAD16_BYTE_x4( "epr-14280.ic6",  0x100001, 0x020000, CRC(3c8148f7) SHA1(072b7982bb95e7a9ab77844b59020146c262488d) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x4( "epr-14285.ic35", 0x100000, 0x040000, CRC(25aefad6) SHA1(10153f4e773a0f55378f869eb1d85156e85f893f) )
	ROM_LOAD_x2( "mpr-14284.ic31", 0x200000, 0x080000, CRC(760542d4) SHA1(dcac73869c02fefd328bd6bdbcbdb3b68b0647da) )
	ROM_LOAD_x2( "mpr-14283.ic26", 0x300000, 0x080000, CRC(c863a91c) SHA1(afdc76bbb9b207cfcb47d437248a757d03212f4e) )
	ROM_LOAD_x2( "mpr-14282.ic22", 0x400000, 0x080000, CRC(ea20979e) SHA1(9b70ef055da8c7c56da54b7edef2379678e7c50f) )

	ROM_REGION( 0x400000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD32_BYTE( "mpr-14291-s.ic38", 0x000000, 0x100000, CRC(490f95a1) SHA1(f220788670b76164ac414ed9b16a422f719be267) )
	ROM_LOAD32_BYTE( "mpr-14290-s.ic34", 0x000002, 0x100000, CRC(a144162d) SHA1(d43f12dd9f690cdfcebb6c7b515ff7dc7dcaa377) )
	ROM_LOAD32_BYTE( "mpr-14289-s.ic29", 0x000001, 0x100000, CRC(38570582) SHA1(a9d810a02a1f5a6849c79d65fbebff21a4b82b59) )
	ROM_LOAD32_BYTE( "mpr-14288-s.ic25", 0x000003, 0x100000, CRC(3188b636) SHA1(bc0adeeca5040caa563ee1e0eded9c323ca23446) )

	ROM_REGION32_BE( 0x800000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-14299-h.ic36", 0x000000, 0x100000, CRC(ce59231b) SHA1(bcb1f11b74935694d0617ec8df66db2cc57b6219) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14298-h.ic32", 0x000001, 0x100000, CRC(2745c84c) SHA1(5a0528c921cba7a1047d3a2ece79925103d719a1) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14297-h.ic27", 0x000002, 0x100000, CRC(29cb9450) SHA1(7dc38d23a2f0cee2f4edde05c1a6f0dc83f331db) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14296-h.ic23", 0x000003, 0x100000, CRC(9d8cbd31) SHA1(55a9f9ec9029157da033e69664b58e694a28db47) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14295-h.ic37", 0x000004, 0x100000, CRC(29591f50) SHA1(1ac4ceaf74892e30f210ad77024eb441c5e4a959) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14294-h.ic33", 0x000005, 0x100000, CRC(fa86b794) SHA1(7b6907e5734fbf2fba7bcc213a8551fec5e9f3d5) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14293-s.ic28", 0x000006, 0x100000, CRC(52899269) SHA1(ff809ff88701109e0ca79e785a61402d97335cec) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14292-s.ic24", 0x000007, 0x100000, CRC(41f71443) SHA1(351d40d6159cb5b792519bce5d16490965800cfb) , ROM_SKIP(7) )
ROM_END

/**************************************************************************************************************************
    Spiderman (Japan)
    not protected

     Game: 833-8331 SPIDER-MAN
Rom board: 834-8332

*/
ROM_START( spidmanj )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x4( "epr-14287.ic13",       0x000000, 0x020000, CRC(403ccdc9) SHA1(4e240c749e362dfa5c579ccbdff18ae3fab58dff) )
	ROM_LOAD_x4( "epr-14286.ic7",        0x080000, 0x020000, CRC(5c2b4e2c) SHA1(79229594710416510f85e6e53ea578789afd4091) )
	ROM_LOAD16_BYTE_x4( "epr-14281.ic14", 0x100000, 0x020000, CRC(8a746c42) SHA1(fa3729ec3aa4b3c59322408146ce2cfbf5a11b98) )
	ROM_LOAD16_BYTE_x4( "epr-14280.ic6",  0x100001, 0x020000, CRC(3c8148f7) SHA1(072b7982bb95e7a9ab77844b59020146c262488d) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x4( "epr-14285.ic35", 0x100000, 0x040000, CRC(25aefad6) SHA1(10153f4e773a0f55378f869eb1d85156e85f893f) )
	ROM_LOAD_x2( "mpr-14284.ic31", 0x200000, 0x080000, CRC(760542d4) SHA1(dcac73869c02fefd328bd6bdbcbdb3b68b0647da) )
	ROM_LOAD_x2( "mpr-14283.ic26", 0x300000, 0x080000, CRC(c863a91c) SHA1(afdc76bbb9b207cfcb47d437248a757d03212f4e) )
	ROM_LOAD_x2( "mpr-14282.ic22", 0x400000, 0x080000, CRC(ea20979e) SHA1(9b70ef055da8c7c56da54b7edef2379678e7c50f) )

	ROM_REGION( 0x400000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD32_BYTE( "mpr-14291-s.ic38", 0x000000, 0x100000, CRC(490f95a1) SHA1(f220788670b76164ac414ed9b16a422f719be267) )
	ROM_LOAD32_BYTE( "mpr-14290-s.ic34", 0x000002, 0x100000, CRC(a144162d) SHA1(d43f12dd9f690cdfcebb6c7b515ff7dc7dcaa377) )
	ROM_LOAD32_BYTE( "mpr-14289-s.ic29", 0x000001, 0x100000, CRC(38570582) SHA1(a9d810a02a1f5a6849c79d65fbebff21a4b82b59) )
	ROM_LOAD32_BYTE( "mpr-14288-s.ic25", 0x000003, 0x100000, CRC(3188b636) SHA1(bc0adeeca5040caa563ee1e0eded9c323ca23446) )

	ROM_REGION32_BE( 0x800000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-14299-h.ic36", 0x000000, 0x100000, CRC(ce59231b) SHA1(bcb1f11b74935694d0617ec8df66db2cc57b6219) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14298-h.ic32", 0x000001, 0x100000, CRC(2745c84c) SHA1(5a0528c921cba7a1047d3a2ece79925103d719a1) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14297-h.ic27", 0x000002, 0x100000, CRC(29cb9450) SHA1(7dc38d23a2f0cee2f4edde05c1a6f0dc83f331db) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14296-h.ic23", 0x000003, 0x100000, CRC(9d8cbd31) SHA1(55a9f9ec9029157da033e69664b58e694a28db47) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14295-h.ic37", 0x000004, 0x100000, CRC(29591f50) SHA1(1ac4ceaf74892e30f210ad77024eb441c5e4a959) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14294-h.ic33", 0x000005, 0x100000, CRC(fa86b794) SHA1(7b6907e5734fbf2fba7bcc213a8551fec5e9f3d5) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14293-s.ic28", 0x000006, 0x100000, CRC(52899269) SHA1(ff809ff88701109e0ca79e785a61402d97335cec) , ROM_SKIP(7) )
	ROMX_LOAD( "mpr-14292-s.ic24", 0x000007, 0x100000, CRC(41f71443) SHA1(351d40d6159cb5b792519bce5d16490965800cfb) , ROM_SKIP(7) )
ROM_END
/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Stadium Cross (Export) - Multi-32
    not protected
*/
ROM_START( scross )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code */
	ROM_LOAD32_WORD_x2( "epr-15093.ic37", 0x000000, 0x040000, CRC(2adc7a4b) SHA1(dca71f00d94898c0758394704d819e13482bf120) )
	ROM_LOAD32_WORD_x2( "epr-15094.ic40", 0x000002, 0x040000, CRC(bbb0ae73) SHA1(0d8837706405f301adf8fa85c8d4813d7600af98) )
	ROM_LOAD32_WORD( "mpr-15018.ic36", 0x100000, 0x080000, CRC(3a98385e) SHA1(8088d337655030c28e290da4bbf44cb647dab66c) )
	ROM_LOAD32_WORD( "mpr-15019.ic39", 0x100002, 0x080000, CRC(8bf4ac83) SHA1(e594d9d9b42d0765ed8a20a40b7dd92b75124d34) )

	ROM_REGION( 0x180000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x4( "epr-15192.ic31", 0x100000, 0x20000, CRC(7524290b) SHA1(ee58be2c0c4293ee19622b96ca493f4ce4da0038) )

	ROM_REGION( 0x400000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-15020.ic3", 0x000000, 0x100000, CRC(de47006a) SHA1(dbef7b9ff8c39992b8596d38985e65c627d6fa79) )
	ROM_LOAD16_BYTE( "mpr-15021.ic11", 0x000001, 0x100000, CRC(3677db02) SHA1(7aeeb85f1632253fcdc8f7881512066e97837e5e) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-15022.ic14", 0x000000, 0x100000, CRC(baee6fd5) SHA1(ddf022c61f0805af45a84c65eb5d01006c153c07) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15024.ic15", 0x000002, 0x100000, CRC(b9f339e2) SHA1(4b9a392459132a19d62928ef3939e1f2356e3994) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15026.ic10", 0x000004, 0x100000, CRC(b72e8df6) SHA1(a7a87f79814b022985121e163c7f88244c50e427) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15028.ic38", 0x000006, 0x100000, CRC(183f6eb0) SHA1(5a2172d5afd696af361ff9f8a92f5911e05c578d) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15023.ic22", 0x800000, 0x100000, CRC(230735ed) SHA1(36075504e27b8a32d63fa3b8abd7037d17440ebf) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15025.ic23", 0x800002, 0x100000, CRC(da4315cb) SHA1(3910c5654e34b17851d6d93615e3404b3b397fea) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15027.ic18", 0x800004, 0x100000, CRC(b765efb8) SHA1(61f3865f92b36ca4b3cd20c0716a7121755eea73) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15029.ic41", 0x800006, 0x100000, CRC(cf8e3b2b) SHA1(c158810d9d82b10a753bc739d1f56572042dac0b) , ROM_SKIP(6)|ROM_GROUPWORD )

	ROM_REGION( 0x400000, "mainpcb:sega", 0 ) /* Sega PCM sound data */
	ROM_LOAD( "mpr-15031.ic1", 0x000000, 0x100000, CRC(6af139dc) SHA1(2378c2ad0c52c114eb93206a6fbee723c038d030) )
	ROM_LOAD( "mpr-15032.ic2", 0x200000, 0x100000, CRC(915d6096) SHA1(e1f670949b1254f5a3c3131993ca9b3baa4d9f6b) )

	ROM_REGION( 0x20000, "user2", 0 ) /*  comms board? - might not belong to this game, just going based on epr number  */
	ROM_LOAD( "epr-15033.ic17", 0x00000, 0x20000, CRC(dc19ac00) SHA1(16bbb5af034e5419673e637be30283b73ab7b290) )
ROM_END

ROM_START( scrossa )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code */
	// 37/40 were missing labels
	ROM_LOAD32_WORD_x2( "ic37",         0x000000, 0x040000, CRC(240a7655) SHA1(7cfdce002fb4783e8c1debf206922d88647c106e) )
	ROM_LOAD32_WORD_x2( "ic40",         0x000002, 0x040000, CRC(3a073060) SHA1(c92c8d8921d94d85c8484c103cbf9cd6ad651333) )
	ROM_LOAD32_WORD( "mpr-15018.ic36", 0x100000, 0x080000, CRC(3a98385e) SHA1(8088d337655030c28e290da4bbf44cb647dab66c) )
	ROM_LOAD32_WORD( "mpr-15019.ic39", 0x100002, 0x080000, CRC(8bf4ac83) SHA1(e594d9d9b42d0765ed8a20a40b7dd92b75124d34) )

	ROM_REGION( 0x180000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x4( "epr-15192.ic31", 0x100000, 0x20000, CRC(7524290b) SHA1(ee58be2c0c4293ee19622b96ca493f4ce4da0038) )

	ROM_REGION( 0x400000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-15020.ic3", 0x000000, 0x100000, CRC(de47006a) SHA1(dbef7b9ff8c39992b8596d38985e65c627d6fa79) )
	ROM_LOAD16_BYTE( "mpr-15021.ic11", 0x000001, 0x100000, CRC(3677db02) SHA1(7aeeb85f1632253fcdc8f7881512066e97837e5e) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-15022.ic14", 0x000000, 0x100000, CRC(baee6fd5) SHA1(ddf022c61f0805af45a84c65eb5d01006c153c07) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15024.ic15", 0x000002, 0x100000, CRC(b9f339e2) SHA1(4b9a392459132a19d62928ef3939e1f2356e3994) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15026.ic10", 0x000004, 0x100000, CRC(b72e8df6) SHA1(a7a87f79814b022985121e163c7f88244c50e427) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15028.ic38", 0x000006, 0x100000, CRC(183f6eb0) SHA1(5a2172d5afd696af361ff9f8a92f5911e05c578d) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15023.ic22", 0x800000, 0x100000, CRC(230735ed) SHA1(36075504e27b8a32d63fa3b8abd7037d17440ebf) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15025.ic23", 0x800002, 0x100000, CRC(da4315cb) SHA1(3910c5654e34b17851d6d93615e3404b3b397fea) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15027.ic18", 0x800004, 0x100000, CRC(b765efb8) SHA1(61f3865f92b36ca4b3cd20c0716a7121755eea73) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15029.ic41", 0x800006, 0x100000, CRC(cf8e3b2b) SHA1(c158810d9d82b10a753bc739d1f56572042dac0b) , ROM_SKIP(6)|ROM_GROUPWORD )

	ROM_REGION( 0x400000, "mainpcb:sega", 0 ) /* Sega PCM sound data */
	ROM_LOAD( "mpr-15031.ic1", 0x000000, 0x100000, CRC(6af139dc) SHA1(2378c2ad0c52c114eb93206a6fbee723c038d030) )
	ROM_LOAD( "mpr-15032.ic2", 0x200000, 0x100000, CRC(915d6096) SHA1(e1f670949b1254f5a3c3131993ca9b3baa4d9f6b) )

	ROM_REGION( 0x20000, "user2", 0 ) /* comms board confirmed */
	ROM_LOAD( "epr-15033.ic17", 0x00000, 0x20000, CRC(dc19ac00) SHA1(16bbb5af034e5419673e637be30283b73ab7b290) )
ROM_END

/**************************************************************************************************************************
    Stadium Cross (US) - Multi-32
    not protected
*/

ROM_START( scrossu )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code */
	ROM_LOAD32_WORD_x2( "epr-15091.ic37", 0x000000, 0x040000, CRC(2c572293) SHA1(6377a6eb6084f7332ce6eeaaf0c37200da792d0c) )
	ROM_LOAD32_WORD_x2( "epr-15092.ic40", 0x000002, 0x040000, CRC(6e3e175a) SHA1(feaca0720646e2a4b78b376e99dc86788adb98e7) )
	ROM_LOAD32_WORD( "epr-15018.ic36",    0x100000, 0x080000, CRC(3a98385e) SHA1(8088d337655030c28e290da4bbf44cb647dab66c) )
	ROM_LOAD32_WORD( "epr-15019.ic39",    0x100002, 0x080000, CRC(8bf4ac83) SHA1(e594d9d9b42d0765ed8a20a40b7dd92b75124d34) )

	ROM_REGION( 0x180000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x4( "epr-15192.ic31", 0x100000, 0x20000, CRC(7524290b) SHA1(ee58be2c0c4293ee19622b96ca493f4ce4da0038) )

	ROM_REGION( 0x400000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-15020.ic3", 0x000000, 0x100000, CRC(de47006a) SHA1(dbef7b9ff8c39992b8596d38985e65c627d6fa79) )
	ROM_LOAD16_BYTE( "mpr-15021.ic11", 0x000001, 0x100000, CRC(3677db02) SHA1(7aeeb85f1632253fcdc8f7881512066e97837e5e) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-15022.ic14", 0x000000, 0x100000, CRC(baee6fd5) SHA1(ddf022c61f0805af45a84c65eb5d01006c153c07) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15024.ic15", 0x000002, 0x100000, CRC(b9f339e2) SHA1(4b9a392459132a19d62928ef3939e1f2356e3994) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15026.ic10", 0x000004, 0x100000, CRC(b72e8df6) SHA1(a7a87f79814b022985121e163c7f88244c50e427) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15028.ic38", 0x000006, 0x100000, CRC(183f6eb0) SHA1(5a2172d5afd696af361ff9f8a92f5911e05c578d) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15023.ic22", 0x800000, 0x100000, CRC(230735ed) SHA1(36075504e27b8a32d63fa3b8abd7037d17440ebf) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15025.ic23", 0x800002, 0x100000, CRC(da4315cb) SHA1(3910c5654e34b17851d6d93615e3404b3b397fea) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15027.ic18", 0x800004, 0x100000, CRC(b765efb8) SHA1(61f3865f92b36ca4b3cd20c0716a7121755eea73) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15029.ic41", 0x800006, 0x100000, CRC(cf8e3b2b) SHA1(c158810d9d82b10a753bc739d1f56572042dac0b) , ROM_SKIP(6)|ROM_GROUPWORD )

	ROM_REGION( 0x400000, "mainpcb:sega", 0 ) /* Sega PCM sound data */
	ROM_LOAD( "mpr-15031.ic1", 0x000000, 0x100000, CRC(6af139dc) SHA1(2378c2ad0c52c114eb93206a6fbee723c038d030) )
	ROM_LOAD( "mpr-15032.ic2", 0x200000, 0x100000, CRC(915d6096) SHA1(e1f670949b1254f5a3c3131993ca9b3baa4d9f6b) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Super Visual Football
    protected via FD1149 317-0222
*/
ROM_START( svf )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x4( "epr-16872a.ic17",    0x000000, 0x020000, CRC(1f383b00) SHA1(c3af01743de5ff09ada19879902842efdbceb595) )
	ROM_LOAD_x4( "epr-16871a.ic8",     0x080000, 0x020000, CRC(f7061bd7) SHA1(b46f4f2ecda8f521c0a91f2f2c2445b72cbc2874) )
	ROM_LOAD16_BYTE( "epr-16865.ic18", 0x100000, 0x080000, CRC(9198ca9f) SHA1(0f6271ce8a07e4ab7fdce38964055510f2ebfd4e) )
	ROM_LOAD16_BYTE( "epr-16864.ic9",  0x100001, 0x080000, CRC(201a940e) SHA1(e19d76141844dbdedee0698ea50edbb898ab55e9) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x8( "epr-16866.ic36", 0x100000, 0x020000, CRC(74431350) SHA1(d3208b595423b5b0f25ee90db213112a09906f8f) )
	ROM_LOAD( "mpr-16779.ic35",    0x200000, 0x100000, CRC(7055e859) SHA1(cde27fa4aaf0ee54063ee68794e9a6075581fff5) )
	ROM_LOAD( "mpr-16778.ic34",    0x300000, 0x100000, CRC(feedaecf) SHA1(25c14ccb85c467dc0c8e85b61f8f86f4396c0cc7) )
	ROM_LOAD( "mpr-16777.ic24",    0x400000, 0x100000, CRC(14b5d5df) SHA1(1b0b0a31294b1bbc16d2046b374d584a1b00a78c) )

	ROM_REGION( 0x200000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-16784.ic14", 0x000000, 0x100000, CRC(4608efe2) SHA1(9b41aa28f50af770e854ef9fdff1a55da7b7b131) )
	ROM_LOAD16_BYTE( "mpr-16783.ic5",  0x000001, 0x100000, CRC(042eabe7) SHA1(a11df5c21d85f0c96dbdcaf57be37a79658ad648) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-16785.ic32", 0x000000, 0x200000, CRC(51f775ce) SHA1(125b40bf47304d37b92e81df5081c81d9af6c8a2) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16787.ic30", 0x000002, 0x200000, CRC(dee7a204) SHA1(29acff4d5dd68609ac46853860788206d18262ab) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16789.ic28", 0x000004, 0x200000, CRC(6b6c8ad3) SHA1(97b0078c851845c31dcf0fe4b2a88393dcdf8988) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16791.ic26", 0x000006, 0x200000, CRC(4f7236da) SHA1(d1c29f6aa82d60a626217f1430bc8a76ef012007) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16860.ic31", 0x800000, 0x200000, CRC(578a7325) SHA1(75a066841fa24952d8ed5ac2d988609295f437a8) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16861.ic29", 0x800002, 0x200000, CRC(d79c3f73) SHA1(e4360efb0964a92cfad8c458a5568709fcc81339) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16862.ic27", 0x800004, 0x200000, CRC(00793354) SHA1(3b37a89c5100d5f92a3567fc8d2003bc9d6fe0cd) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16863.ic25", 0x800006, 0x200000, CRC(42338226) SHA1(106636408d5648fb95fbaee06074c57f6a535a82) , ROM_SKIP(6)|ROM_GROUPWORD )
ROM_END

/**************************************************************************************************************************
    Super Visual Soccer
    protected via FD1149 317-0222
*/
ROM_START( svs )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x4( "epr-16883a.ic17",    0x000000, 0x020000, CRC(e1c0c3ce) SHA1(12dd8d9d1a2c2c7bf1ab652a6a6f947384d79577) )
	ROM_LOAD_x4( "epr-16882a.ic8",     0x080000, 0x020000, CRC(1161bbbe) SHA1(3cfeed9ea947eed79aeb5674d54de45d15fb6e1f) )
	ROM_LOAD16_BYTE( "epr-16865.ic18", 0x100000, 0x080000, CRC(9198ca9f) SHA1(0f6271ce8a07e4ab7fdce38964055510f2ebfd4e) )
	ROM_LOAD16_BYTE( "epr-16864.ic9",  0x100001, 0x080000, CRC(201a940e) SHA1(e19d76141844dbdedee0698ea50edbb898ab55e9) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x4( "epr-16868.ic36", 0x100000, 0x040000, CRC(47aa4ec7) SHA1(baea18aaac0314f769f1e36fdbe8aedf62862544) ) /* same as jleague but with a different part number */
	ROM_LOAD( "mpr-16779.ic35",    0x200000, 0x100000, CRC(7055e859) SHA1(cde27fa4aaf0ee54063ee68794e9a6075581fff5) )
	ROM_LOAD( "mpr-16778.ic34",    0x300000, 0x100000, CRC(feedaecf) SHA1(25c14ccb85c467dc0c8e85b61f8f86f4396c0cc7) )
	ROM_LOAD( "mpr-16777.ic24",    0x400000, 0x100000, CRC(14b5d5df) SHA1(1b0b0a31294b1bbc16d2046b374d584a1b00a78c) )

	ROM_REGION( 0x200000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-16784.ic14", 0x000000, 0x100000, CRC(4608efe2) SHA1(9b41aa28f50af770e854ef9fdff1a55da7b7b131) )
	ROM_LOAD16_BYTE( "mpr-16783.ic5",  0x000001, 0x100000, CRC(042eabe7) SHA1(a11df5c21d85f0c96dbdcaf57be37a79658ad648) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-16785.ic32", 0x000000, 0x200000, CRC(51f775ce) SHA1(125b40bf47304d37b92e81df5081c81d9af6c8a2) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16787.ic30", 0x000002, 0x200000, CRC(dee7a204) SHA1(29acff4d5dd68609ac46853860788206d18262ab) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16789.ic28", 0x000004, 0x200000, CRC(6b6c8ad3) SHA1(97b0078c851845c31dcf0fe4b2a88393dcdf8988) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16791.ic26", 0x000006, 0x200000, CRC(4f7236da) SHA1(d1c29f6aa82d60a626217f1430bc8a76ef012007) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16860.ic31", 0x800000, 0x200000, CRC(578a7325) SHA1(75a066841fa24952d8ed5ac2d988609295f437a8) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16861.ic29", 0x800002, 0x200000, CRC(d79c3f73) SHA1(e4360efb0964a92cfad8c458a5568709fcc81339) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16862.ic27", 0x800004, 0x200000, CRC(00793354) SHA1(3b37a89c5100d5f92a3567fc8d2003bc9d6fe0cd) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16863.ic25", 0x800006, 0x200000, CRC(42338226) SHA1(106636408d5648fb95fbaee06074c57f6a535a82) , ROM_SKIP(6)|ROM_GROUPWORD )
ROM_END

/**************************************************************************************************************************
    The JLeague '94
    protected via FD1149 317-0222

    Sega Game ID codes:
     Game: 833-10851 J. LEAGUE 1994

*/
ROM_START( jleague )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code + data */
	ROM_LOAD_x4( "epr-16782.ic17",     0x000000, 0x020000, CRC(f0278944) SHA1(49e3842231ee5abdd6205b598309153d6b4ddc02) )
	ROM_LOAD_x4( "epr-16781.ic8",      0x080000, 0x020000, CRC(7df9529b) SHA1(de3633f4941ff3877c4cb8b53e080eccea19f22e) )
	ROM_LOAD16_BYTE( "epr-16776.ic18", 0x100000, 0x080000, CRC(e8694626) SHA1(d4318a9a6b1cc5c719bff9c25b7398dd2ea1e18b) )
	ROM_LOAD16_BYTE( "epr-16775.ic9",  0x100001, 0x080000, CRC(e81e2c3d) SHA1(2900710f1dec6cf71875c82a56584ba45ed3a545) )

	ROM_REGION( 0x500000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x4( "epr-16780.ic36", 0x100000, 0x040000, CRC(47aa4ec7) SHA1(baea18aaac0314f769f1e36fdbe8aedf62862544) )
	ROM_LOAD( "mpr-16779.ic35",    0x200000, 0x100000, CRC(7055e859) SHA1(cde27fa4aaf0ee54063ee68794e9a6075581fff5) )
	ROM_LOAD( "mpr-16778.ic34",    0x300000, 0x100000, CRC(feedaecf) SHA1(25c14ccb85c467dc0c8e85b61f8f86f4396c0cc7) )
	ROM_LOAD( "mpr-16777.ic24",    0x400000, 0x100000, CRC(14b5d5df) SHA1(1b0b0a31294b1bbc16d2046b374d584a1b00a78c) )

	ROM_REGION( 0x200000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-16784.ic14", 0x000000, 0x100000, CRC(4608efe2) SHA1(9b41aa28f50af770e854ef9fdff1a55da7b7b131) )
	ROM_LOAD16_BYTE( "mpr-16783.ic5",  0x000001, 0x100000, CRC(042eabe7) SHA1(a11df5c21d85f0c96dbdcaf57be37a79658ad648) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-16785.ic32", 0x000000, 0x200000, CRC(51f775ce) SHA1(125b40bf47304d37b92e81df5081c81d9af6c8a2) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16787.ic30", 0x000002, 0x200000, CRC(dee7a204) SHA1(29acff4d5dd68609ac46853860788206d18262ab) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16789.ic28", 0x000004, 0x200000, CRC(6b6c8ad3) SHA1(97b0078c851845c31dcf0fe4b2a88393dcdf8988) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16791.ic26", 0x000006, 0x200000, CRC(4f7236da) SHA1(d1c29f6aa82d60a626217f1430bc8a76ef012007) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16786.ic31", 0x800000, 0x200000, CRC(d08a2d32) SHA1(baac63cbacbe38e057684174040384a7152eb523) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16788.ic29", 0x800002, 0x200000, CRC(cd9d3605) SHA1(7c4402be1a1ddde6766cfdd5fe7e2a088c4a59e8) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16790.ic27", 0x800004, 0x200000, CRC(2ea75746) SHA1(c91e5d678917886cc23fbef7a577c5a70665c7b2) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-16792.ic25", 0x800006, 0x200000, CRC(9f416072) SHA1(18107cf64f918888aa5a54432f8e9137910101b8) , ROM_SKIP(6)|ROM_GROUPWORD )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Title Fight (Export) - Multi-32
    not protected

    Sega Game ID codes:
     Game: 834-9324-02 TITLE FIGHT
Rom board: 834-9413-02
  Main BD: 837-8676 (SYSTEM MULTI 32)

*/
ROM_START( titlef )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code */
	ROM_LOAD32_WORD_x2( "epr-15388.ic37", 0x000000, 0x40000, CRC(db1eefbd) SHA1(7059a1d5c9364d836c1d922071a108cbde661e0a) )
	ROM_LOAD32_WORD_x2( "epr-15389.ic40", 0x000002, 0x40000, CRC(da9f60a3) SHA1(87a7bea04e51e3c241871e83ff7322c6a07bd106) )

	ROM_REGION( 0x180000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x4( "epr-15384.ic31", 0x100000, 0x20000, CRC(0f7d208d) SHA1(5425120480f813210fae28951e8bfd5acb08ca53) )

	ROM_REGION( 0x400000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-15381.ic3",  0x000000, 0x200000, CRC(162cc4d6) SHA1(2369d3d76ab5ef8f033aa45530ab957f0e5ff028) )
	ROM_LOAD16_BYTE( "mpr-15382.ic11", 0x000001, 0x200000, CRC(fd03a130) SHA1(040c36383ef5d8298af714958cd5b0a4c7556ae7) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-15379.ic14", 0x000000, 0x200000, CRC(e5c74b11) SHA1(67e4460efe5dcd88ffc12024b255efc843e6a8b5) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15375.ic15", 0x000002, 0x200000, CRC(046a9b50) SHA1(2b4c53f2a0264835cb7197daa9b3461c212541e8) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15371.ic10", 0x000004, 0x200000, CRC(999046c6) SHA1(37ce4e8aaf537b5366eacabaf36e4477b5624121) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15373.ic38", 0x000006, 0x200000, CRC(9b3294d9) SHA1(19542f14ce09753385a44098dfd1aaf331e7af0e) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15380.ic22", 0x800000, 0x200000, CRC(6ea0e58d) SHA1(1c4b761522157b0b9d086181ba6f6994879d8fdf) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15376.ic23", 0x800002, 0x200000, CRC(de3e05c5) SHA1(cac0d04ecd37e5836d246c0809bcfc11430df591) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15372.ic18", 0x800004, 0x200000, CRC(c187c36a) SHA1(bb55c2a768a43ef19a7847a4aa113523fee26c20) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15374.ic41", 0x800006, 0x200000, CRC(e026aab0) SHA1(75dfaef6d50c3d1d7f27aa5e44fcbc0ff2173c6f) , ROM_SKIP(6)|ROM_GROUPWORD )

	ROM_REGION( 0x400000, "mainpcb:sega", 0 ) /* Sega PCM sound data */
	ROM_LOAD( "mpr-15385.ic1", 0x000000, 0x200000, CRC(5a9b0aa0) SHA1(d208aa165f9eea05e3b8c3f406ff44374e4f6887) )
ROM_END

/**************************************************************************************************************************
    Title Fight (US) - Multi-32
    not protected

    Sega Game ID codes:
     Game: 834-9324-01 TITLE FIGHT
Rom board: 834-9413-01
  Main BD: 837-8676 (SYSTEM MULTI 32)

*/
ROM_START( titlefu )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code */
	ROM_LOAD32_WORD_x4( "epr-15386.ic37", 0x000000, 0x40000, CRC(e36e2516) SHA1(b6a73d6136ff8a13831b6db0fbc8a585f2acf254) )
	ROM_LOAD32_WORD_x4( "epr-15387.ic40", 0x000002, 0x40000, CRC(e63406d3) SHA1(cd105862b2267d1d3af588cda70e6e4c1cca1da2) )

	ROM_REGION( 0x180000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x4( "epr-15384.ic31", 0x100000, 0x20000, CRC(0f7d208d) SHA1(5425120480f813210fae28951e8bfd5acb08ca53) )

	ROM_REGION( 0x400000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-15381.ic3",  0x000000, 0x200000, CRC(162cc4d6) SHA1(2369d3d76ab5ef8f033aa45530ab957f0e5ff028) )
	ROM_LOAD16_BYTE( "mpr-15382.ic11", 0x000001, 0x200000, CRC(fd03a130) SHA1(040c36383ef5d8298af714958cd5b0a4c7556ae7) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-15379.ic14", 0x000000, 0x200000, CRC(e5c74b11) SHA1(67e4460efe5dcd88ffc12024b255efc843e6a8b5) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15375.ic15", 0x000002, 0x200000, CRC(046a9b50) SHA1(2b4c53f2a0264835cb7197daa9b3461c212541e8) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15371.ic10", 0x000004, 0x200000, CRC(999046c6) SHA1(37ce4e8aaf537b5366eacabaf36e4477b5624121) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15373.ic38", 0x000006, 0x200000, CRC(9b3294d9) SHA1(19542f14ce09753385a44098dfd1aaf331e7af0e) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15380.ic22", 0x800000, 0x200000, CRC(6ea0e58d) SHA1(1c4b761522157b0b9d086181ba6f6994879d8fdf) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15376.ic23", 0x800002, 0x200000, CRC(de3e05c5) SHA1(cac0d04ecd37e5836d246c0809bcfc11430df591) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15372.ic18", 0x800004, 0x200000, CRC(c187c36a) SHA1(bb55c2a768a43ef19a7847a4aa113523fee26c20) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15374.ic41", 0x800006, 0x200000, CRC(e026aab0) SHA1(75dfaef6d50c3d1d7f27aa5e44fcbc0ff2173c6f) , ROM_SKIP(6)|ROM_GROUPWORD )

	ROM_REGION( 0x400000, "mainpcb:sega", 0 ) /* Sega PCM sound data */
	ROM_LOAD( "mpr-15385.ic1", 0x000000, 0x200000, CRC(5a9b0aa0) SHA1(d208aa165f9eea05e3b8c3f406ff44374e4f6887) )
ROM_END

/**************************************************************************************************************************
    Title Fight (Japan) - Multi-32
    not protected

    Sega Game ID codes:
     Game: 834-9324-03 TITLE FIGHT
Rom board: 834-9413-03
  Main BD: 837-8676 (SYSTEM MULTI 32)

*/
ROM_START( titlefj )
	ROM_REGION( 0x200000, "mainpcb:maincpu", 0 ) /* v60 code */
	ROM_LOAD32_WORD_x4( "epr-15377.ic37", 0x000000, 0x40000, CRC(1868403c) SHA1(d34a3a05e5a3bb2e6159f95d1e22d264bf553cda) )
	ROM_LOAD32_WORD_x4( "epr-15378.ic40", 0x000002, 0x40000, CRC(44487b0a) SHA1(4aefd063bf148334d5f43d69f497766d50ffca30) )

	ROM_REGION( 0x180000, "mainpcb:soundcpu", 0 ) /* sound CPU */
	ROM_LOAD_x4( "epr-15384.ic31", 0x100000, 0x20000, CRC(0f7d208d) SHA1(5425120480f813210fae28951e8bfd5acb08ca53) )

	ROM_REGION( 0x400000, "mainpcb:gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE( "mpr-15381.ic3",  0x000000, 0x200000, CRC(162cc4d6) SHA1(2369d3d76ab5ef8f033aa45530ab957f0e5ff028) )
	ROM_LOAD16_BYTE( "mpr-15382.ic11", 0x000001, 0x200000, CRC(fd03a130) SHA1(040c36383ef5d8298af714958cd5b0a4c7556ae7) )

	ROM_REGION32_BE( 0x1000000, "mainpcb:gfx2", 0 ) /* sprites */
	ROMX_LOAD( "mpr-15379.ic14", 0x000000, 0x200000, CRC(e5c74b11) SHA1(67e4460efe5dcd88ffc12024b255efc843e6a8b5) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15375.ic15", 0x000002, 0x200000, CRC(046a9b50) SHA1(2b4c53f2a0264835cb7197daa9b3461c212541e8) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15371.ic10", 0x000004, 0x200000, CRC(999046c6) SHA1(37ce4e8aaf537b5366eacabaf36e4477b5624121) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15373.ic38", 0x000006, 0x200000, CRC(9b3294d9) SHA1(19542f14ce09753385a44098dfd1aaf331e7af0e) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15380.ic22", 0x800000, 0x200000, CRC(6ea0e58d) SHA1(1c4b761522157b0b9d086181ba6f6994879d8fdf) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15376.ic23", 0x800002, 0x200000, CRC(de3e05c5) SHA1(cac0d04ecd37e5836d246c0809bcfc11430df591) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15372.ic18", 0x800004, 0x200000, CRC(c187c36a) SHA1(bb55c2a768a43ef19a7847a4aa113523fee26c20) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr-15374.ic41", 0x800006, 0x200000, CRC(e026aab0) SHA1(75dfaef6d50c3d1d7f27aa5e44fcbc0ff2173c6f) , ROM_SKIP(6)|ROM_GROUPWORD )

	ROM_REGION( 0x400000, "mainpcb:sega", 0 ) /* Sega PCM sound data */
	ROM_LOAD( "mpr-15385.ic1", 0x000000, 0x200000, CRC(5a9b0aa0) SHA1(d208aa165f9eea05e3b8c3f406ff44374e4f6887) )
ROM_END



/*************************************
 *
 *  Common driver inits
 *
 *************************************/

void segas32_state::segas32_common_init(read16_delegate custom_r, write16_delegate custom_w)
{
	/* reset the custom handlers and other pointers */
	m_custom_io_r[0] = custom_r;
	m_custom_io_w[0] = custom_w;
	m_system32_prot_vblank = nullptr;
	m_sw1_output = nullptr;
	m_sw2_output = nullptr;
	m_sw3_output = nullptr;
}


/*************************************
 *
 *  Output callbacks
 *
 *  TODO: kokoroj2 and jpark (SW2)
 *
 *  Additional notes:
 *    - about jpark: the compression switch is broken/inoperative
 *      and because of that all piston data, which is in this
 *      section is frozen. bits x01, x04 and x10 when which == 0
 *      (IO chip 0), seem to have something to do with the sensor
 *      switches we need to fix
 *************************************/

void segas32_state::radm_sw1_output( int which, UINT16 data )
{
	if (which == 0)
		output_set_value("Start_lamp", BIT(data, 2));
}

void segas32_state::radm_sw2_output( int which, UINT16 data )
{
	if (which == 0)
	{
		output_set_value("Wiper_lamp", BIT(data, 0));
		output_set_value("Lights_lamp", BIT(data, 1));
	}
}

void segas32_state::radr_sw2_output( int which, UINT16 data )
{
	if (which == 0)
	{
		output_set_value("Entry_lamp", BIT(data, 0));
		output_set_value("Winner_lamp", BIT(data, 1));
	}
}

void segas32_state::alien3_sw1_output( int which, UINT16 data )
{
	if (which == 0)
	{
		output_set_value("Player1_Gun_Recoil", BIT(data, 2));
		output_set_value("Player2_Gun_Recoil", BIT(data, 3));
	}
}

void segas32_state::arescue_sw1_output( int which, UINT16 data )
{
	if (which == 0)
	{
		output_set_value("Start_lamp", BIT(data, 2));
		output_set_value("Back_lamp", BIT(data, 4));
	}
}

void segas32_state::f1lap_sw1_output( int which, UINT16 data )
{
	if (which == 0)
	{
		output_set_value("lamp0", BIT(data, 2));
		output_set_value("lamp1", BIT(data, 3));
	}
}

void segas32_state::jpark_sw1_output( int which, UINT16 data )
{
	if (which == 0)
	{
		output_set_value("Left_lamp", BIT(data, 2));
		output_set_value("Right_lamp", BIT(data, 3));
	}
}

void segas32_state::orunners_sw1_output( int which, UINT16 data )
{
	/* note ma = monitor A and mb = Monitor B */
	if (which == 0)
	{
		output_set_value("MA_Check_Point_lamp", BIT(data, 1));
		output_set_value("MA_Race_Leader_lamp", BIT(data, 3));
		output_set_value("MA_Steering_Wheel_motor", BIT(data, 4));
	}
	else
	{
		output_set_value("MB_Check_Point_lamp", BIT(data, 1));
		output_set_value("MB_Race_Leader_lamp", BIT(data, 3));
		output_set_value("MB_Steering_Wheel_motor", BIT(data, 4));
	}
}

void segas32_state::orunners_sw2_output( int which, UINT16 data )
{
	/* note ma = monitor A and mb = Monitor B */
	/* also note that the remaining bits are for the game's lcd display */
	/* the bijokkoy driver might be used as an example for handling these outputs */
	if (which == 0)
	{
		output_set_value("MA_DJ_Music_lamp", BIT(data, 0));
		output_set_value("MA_<<_>>_lamp", BIT(data, 1));
	}
	else
	{
		output_set_value("MB_DJ_Music_lamp", BIT(data, 0));
		output_set_value("MB_<<_>>_lamp", BIT(data, 1));
	}
}

void segas32_state::harddunk_sw1_output( int which, UINT16 data )
{
	if (which == 0)
	{
		output_set_value("1P_Start_lamp", BIT(data, 2));
		output_set_value("2P_Start_lamp", BIT(data, 3));
	}
	else
	{
		output_set_value("4P_Start_lamp", BIT(data, 2));
		output_set_value("5P_Start_lamp", BIT(data, 3));
	}
}

void segas32_state::harddunk_sw2_output( int which, UINT16 data )
{
	if (which == 0)
		output_set_value("Left_Winner_lamp", BIT(data, 0));
	else
		output_set_value("Right_Winner_lamp", BIT(data, 0));
}

void segas32_state::harddunk_sw3_output( int which, UINT16 data )
{
	output_set_value("3P_Start_lamp", BIT(data, 4));
	output_set_value("6P_Start_lamp", BIT(data, 5));
}

void segas32_state::titlef_sw1_output( int which, UINT16 data )
{
	if (which == 0)
	{
		output_set_value("Blue_Button_1P_lamp", BIT(data, 2));
		output_set_value("Blue_Button_2P_lamp", BIT(data, 3));
	}
	else
	{
		output_set_value("Red_Button_1P_lamp", BIT(data, 2));
		output_set_value("Red_Button_2P_lamp", BIT(data, 3));
	}
}

void segas32_state::titlef_sw2_output( int which, UINT16 data )
{
	if (which == 0)
		output_set_value("Blue_Corner_lamp", BIT(data, 0));
	else
		output_set_value("Red_Corner_lamp", BIT(data, 0));
}

void segas32_state::scross_sw1_output( int which, UINT16 data )
{
	/* note ma = monitor A and mb = Monitor B */
	if (which == 0)
		output_set_value("MA_Start_lamp", BIT(data, 2));
	else
		output_set_value("MB_Start_lamp", BIT(data, 2));
}

void segas32_state::scross_sw2_output( int which, UINT16 data )
{
	/* Note:  I'm not an expert on digits, so I didn't know the right map to use, I just added it manually and it seems to work fine. */
	if (which == 0)
		output_set_value("MA_Digit", data);
	else
		output_set_value("MB_Digit", data);
}

/*************************************
 *
 *  Game-specific driver inits
 *
 *************************************/

WRITE16_MEMBER(segas32_new_state::dual_pcb_comms_w)
{
	COMBINE_DATA(&m_dual_pcb_comms[offset]);
}

READ16_MEMBER(segas32_new_state::dual_pcb_comms_r)
{
	return m_dual_pcb_comms[offset];
}


/* There must be something on the comms board for this?
   Probably not a dip/solder link/trace cut, but maybe
   just whichever way the cables are plugged in?
   Both f1en and arescue master units try to set bit 1... */
READ16_MEMBER(segas32_new_state::dual_pcb_masterslave)
{
	return 0; // 0/1 master/slave
}

READ16_MEMBER(segas32_new_state::dual_pcb_slave)
{
	return 1; // 0/1 master/slave
}




DRIVER_INIT_MEMBER(segas32_new_state,titlef) {  m_mainpcb->init_titlef(); }
DRIVER_INIT_MEMBER(segas32_new_state,slipstrm) {    m_mainpcb->init_slipstrm(); }
DRIVER_INIT_MEMBER(segas32_new_state,radm) {    m_mainpcb->init_radm(); }
DRIVER_INIT_MEMBER(segas32_new_state,holo) {    m_mainpcb->init_holo(); }
DRIVER_INIT_MEMBER(segas32_new_state,svf) { m_mainpcb->init_svf(); }
DRIVER_INIT_MEMBER(segas32_new_state,jleague) { m_mainpcb->init_jleague(); }
DRIVER_INIT_MEMBER(segas32_new_state,jpark) {   m_mainpcb->init_jpark(); }
DRIVER_INIT_MEMBER(segas32_new_state,ga2) { m_mainpcb->init_ga2(); }
DRIVER_INIT_MEMBER(segas32_new_state,scross) {  m_mainpcb->init_scross(); }
DRIVER_INIT_MEMBER(segas32_new_state,spidman) { m_mainpcb->init_spidman(); }
DRIVER_INIT_MEMBER(segas32_new_state,sonicp) {  m_mainpcb->init_sonicp(); }
DRIVER_INIT_MEMBER(segas32_new_state,dbzvrvs) { m_mainpcb->init_dbzvrvs(); }
DRIVER_INIT_MEMBER(segas32_new_state,brival) {  m_mainpcb->init_brival(); }
DRIVER_INIT_MEMBER(segas32_new_state,harddunk) {    m_mainpcb->init_harddunk(); }
DRIVER_INIT_MEMBER(segas32_new_state,arabfgt) { m_mainpcb->init_arabfgt(); }
DRIVER_INIT_MEMBER(segas32_new_state,sonic) {   m_mainpcb->init_sonic(); }
DRIVER_INIT_MEMBER(segas32_new_state,alien3) {  m_mainpcb->init_alien3(); }
DRIVER_INIT_MEMBER(segas32_new_state,darkedge) {    m_mainpcb->init_darkedge(); }
DRIVER_INIT_MEMBER(segas32_new_state,radr) {    m_mainpcb->init_radr(); }
DRIVER_INIT_MEMBER(segas32_new_state,orunners) {    m_mainpcb->init_orunners(); }

DRIVER_INIT_MEMBER(segas32_new_state, arescue)
{
	m_mainpcb->init_arescue(1);
	m_slavepcb->init_arescue(0);

	m_dual_pcb_comms = std::make_unique<UINT16[]>(0x1000/2);
	m_mainpcb->m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x810000, 0x810fff, read16_delegate(FUNC(segas32_new_state::dual_pcb_comms_r),this), write16_delegate(FUNC(segas32_new_state::dual_pcb_comms_w),this));
	m_mainpcb->m_maincpu->space(AS_PROGRAM).install_read_handler(0x818000, 0x818003, read16_delegate(FUNC(segas32_new_state::dual_pcb_masterslave),this));

	m_slavepcb->m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x810000, 0x810fff, read16_delegate(FUNC(segas32_new_state::dual_pcb_comms_r),this), write16_delegate(FUNC(segas32_new_state::dual_pcb_comms_w),this));
	m_slavepcb->m_maincpu->space(AS_PROGRAM).install_read_handler(0x818000, 0x818003, read16_delegate(FUNC(segas32_new_state::dual_pcb_slave),this));
}

DRIVER_INIT_MEMBER(segas32_new_state,f1en) {
	m_mainpcb->init_f1en();
	m_slavepcb->init_f1en();

	m_dual_pcb_comms = std::make_unique<UINT16[]>(0x1000/2);
	memset(m_dual_pcb_comms.get(), 0xff, 0x1000 / 2);

	m_mainpcb->m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x810000, 0x810fff, read16_delegate(FUNC(segas32_new_state::dual_pcb_comms_r),this), write16_delegate(FUNC(segas32_new_state::dual_pcb_comms_w),this));
	m_mainpcb->m_maincpu->space(AS_PROGRAM).install_read_handler(0x818000, 0x818003, read16_delegate(FUNC(segas32_new_state::dual_pcb_masterslave),this));

	m_slavepcb->m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x810000, 0x810fff, read16_delegate(FUNC(segas32_new_state::dual_pcb_comms_r),this), write16_delegate(FUNC(segas32_new_state::dual_pcb_comms_w),this));
	m_slavepcb->m_maincpu->space(AS_PROGRAM).install_read_handler(0x818000, 0x818003, read16_delegate(FUNC(segas32_new_state::dual_pcb_slave),this));
}

DRIVER_INIT_MEMBER(segas32_new_state,f1lap)
{
	m_mainpcb->init_f1lap();
}


void segas32_state::init_alien3(void)
{
	segas32_common_init(read16_delegate(FUNC(segas32_state::analog_custom_io_r), this), write16_delegate(FUNC(segas32_state::analog_custom_io_w), this));
	m_sw1_output = &segas32_state::alien3_sw1_output;
}

void segas32_state::init_arescue(int m_hasdsp)
{
	segas32_common_init(read16_delegate(FUNC(segas32_state::analog_custom_io_r),this), write16_delegate(FUNC(segas32_state::analog_custom_io_w),this));
	if (m_hasdsp) m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xa00000, 0xa00007, read16_delegate(FUNC(segas32_state::arescue_dsp_r),this), write16_delegate(FUNC(segas32_state::arescue_dsp_w),this));

	for (auto & elem : m_arescue_dsp_io)
		elem = 0x00;

	m_sw1_output = &segas32_state::arescue_sw1_output;
}


void segas32_state::init_arabfgt(void)
{
	segas32_common_init(read16_delegate(FUNC(segas32_state::extra_custom_io_r),this), write16_delegate());

	/* install protection handlers */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xa00100, 0xa0011f, read16_delegate(FUNC(segas32_state::arf_wakeup_protection_r),this));
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xa00000, 0xa00fff, read16_delegate(FUNC(segas32_state::arabfgt_protection_r),this), write16_delegate(FUNC(segas32_state::arabfgt_protection_w),this));
}


void segas32_state::init_brival(void)
{
	segas32_common_init(read16_delegate(FUNC(segas32_state::extra_custom_io_r),this), write16_delegate());

	/* install protection handlers */
	m_system32_protram = std::make_unique<UINT16[]>(0x1000/2);
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x20ba00, 0x20ba07, read16_delegate(FUNC(segas32_state::brival_protection_r),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xa00000, 0xa00fff, write16_delegate(FUNC(segas32_state::brival_protection_w),this));
}


void segas32_state::init_darkedge(void)
{
	segas32_common_init(read16_delegate(FUNC(segas32_state::extra_custom_io_r),this), write16_delegate());

	/* install protection handlers */
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xa00000, 0xa7ffff, read16_delegate(FUNC(segas32_state::darkedge_protection_r),this), write16_delegate(FUNC(segas32_state::darkedge_protection_w),this));
	m_system32_prot_vblank = &segas32_state::darkedge_fd1149_vblank;
}

void segas32_state::init_dbzvrvs(void)
{
	segas32_common_init(read16_delegate(), write16_delegate());

	/* install protection handlers */
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xa00000, 0xa7ffff, read16_delegate(FUNC(segas32_state::dbzvrvs_protection_r),this), write16_delegate(FUNC(segas32_state::dbzvrvs_protection_w),this));
}


void segas32_state::init_f1en(void)
{
	segas32_common_init(read16_delegate(FUNC(segas32_state::analog_custom_io_r),this), write16_delegate(FUNC(segas32_state::analog_custom_io_w),this));


	m_sw1_output = &segas32_state::radm_sw1_output;
}




void segas32_state::init_f1lap(void)
{
	segas32_common_init(read16_delegate(FUNC(segas32_state::analog_custom_io_r),this), write16_delegate(FUNC(segas32_state::analog_custom_io_w),this));
	m_system32_prot_vblank = &segas32_state::f1lap_fd1149_vblank;

	m_sw1_output = &segas32_state::f1lap_sw1_output;

	m_s32comm->set_linktype(15612); // EPR-15612
}


void segas32_state::init_ga2(void)
{
	segas32_common_init(read16_delegate(FUNC(segas32_state::extra_custom_io_r),this), write16_delegate());

	decrypt_ga2_protrom();
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xa00000, 0xa00fff, read16_delegate(FUNC(segas32_state::ga2_dpram_r),this), write16_delegate(FUNC(segas32_state::ga2_dpram_w),this));
}


void segas32_state::init_harddunk(void)
{
	segas32_common_init(read16_delegate(FUNC(segas32_state::extra_custom_io_r),this), write16_delegate());
	m_sw1_output = &segas32_state::harddunk_sw1_output;
	m_sw2_output = &segas32_state::harddunk_sw2_output;
	m_sw3_output = &segas32_state::harddunk_sw3_output;
}


void segas32_state::init_holo(void)
{
	segas32_common_init(read16_delegate(), write16_delegate());
}


void segas32_state::init_jpark(void)
{
	/* Temp. Patch until we emulate the 'Drive Board', thanks to Malice */
	UINT16 *pROM = (UINT16 *)memregion("maincpu")->base();

	segas32_common_init(read16_delegate(FUNC(segas32_state::analog_custom_io_r),this), write16_delegate(FUNC(segas32_state::analog_custom_io_w),this));

	pROM[0xC15A8/2] = 0xCD70;
	pROM[0xC15AA/2] = 0xD8CD;

	m_sw1_output = &segas32_state::jpark_sw1_output;
}


void segas32_state::init_orunners(void)
{
	segas32_common_init(read16_delegate(FUNC(segas32_state::analog_custom_io_r),this), write16_delegate(FUNC(segas32_state::orunners_custom_io_w),this));
	m_sw1_output = &segas32_state::orunners_sw1_output;
	m_sw2_output = &segas32_state::orunners_sw2_output;

	m_s32comm->set_linktype(15033); // EPR-15033
}


void segas32_state::init_radm(void)
{
	segas32_common_init(read16_delegate(FUNC(segas32_state::analog_custom_io_r),this), write16_delegate(FUNC(segas32_state::analog_custom_io_w),this));
	m_sw1_output = &segas32_state::radm_sw1_output;
	m_sw2_output = &segas32_state::radm_sw2_output;
}


void segas32_state::init_radr(void)
{
	segas32_common_init(read16_delegate(FUNC(segas32_state::analog_custom_io_r),this), write16_delegate(FUNC(segas32_state::analog_custom_io_w),this));
	m_sw1_output = &segas32_state::radm_sw1_output;
	m_sw2_output = &segas32_state::radr_sw2_output;

	m_s32comm->set_linktype(14084); // EPR-14084
}


void segas32_state::init_scross(void)
{
	segas32_common_init(read16_delegate(FUNC(segas32_state::analog_custom_io_r),this), write16_delegate(FUNC(segas32_state::analog_custom_io_w),this));
	m_soundcpu->space(AS_PROGRAM).install_write_handler(0xb0, 0xbf, write8_delegate(FUNC(segas32_state::scross_bank_w),this));

	m_sw1_output = &segas32_state::scross_sw1_output;
	m_sw2_output = &segas32_state::scross_sw2_output;

	m_s32comm->set_linktype(15033); // EPR-15033
}


void segas32_state::init_slipstrm(void)
{
	segas32_common_init(read16_delegate(FUNC(segas32_state::analog_custom_io_r),this), write16_delegate(FUNC(segas32_state::analog_custom_io_w),this));
}


void segas32_state::init_sonic(void)
{
	segas32_common_init(read16_delegate(FUNC(segas32_state::sonic_custom_io_r),this), write16_delegate(FUNC(segas32_state::sonic_custom_io_w),this));

	/* install protection handlers */
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x20E5C4, 0x20E5C5, write16_delegate(FUNC(segas32_state::sonic_level_load_protection),this));
}


void segas32_state::init_sonicp(void)
{
	segas32_common_init(read16_delegate(FUNC(segas32_state::sonic_custom_io_r),this), write16_delegate(FUNC(segas32_state::sonic_custom_io_w),this));
}


void segas32_state::init_spidman(void)
{
	segas32_common_init(read16_delegate(FUNC(segas32_state::extra_custom_io_r),this), write16_delegate());
}


void segas32_state::init_svf(void)
{
	segas32_common_init(read16_delegate(), write16_delegate());
}


void segas32_state::init_jleague(void)
{
	segas32_common_init(read16_delegate(), write16_delegate());
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x20F700, 0x20F705, write16_delegate(FUNC(segas32_state::jleague_protection_w),this));
}


void segas32_state::init_titlef(void)
{
	segas32_common_init(read16_delegate(), write16_delegate());
	m_sw1_output = &segas32_state::titlef_sw1_output;
	m_sw2_output = &segas32_state::titlef_sw2_output;
}


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1992, arescue,   0,        sega_system32_dual_direct,     arescue,  segas32_new_state, arescue,  ROT0, "Sega",   "Air Rescue (US)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, arescuej,  arescue,  sega_system32_dual_direct,     arescue,  segas32_new_state, arescue,  ROT0, "Sega",   "Air Rescue (Japan)", MACHINE_IMPERFECT_GRAPHICS )

GAME( 1993, alien3,    0,        sega_system32,     alien3,   segas32_new_state, alien3,   ROT0, "Sega",   "Alien3: The Gun (World)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1993, alien3u,   alien3,   sega_system32,     alien3,   segas32_new_state, alien3,   ROT0, "Sega",   "Alien3: The Gun (US)", MACHINE_IMPERFECT_GRAPHICS )

GAME( 1991, arabfgt,   0,        sega_system32,     arabfgt,  segas32_new_state, arabfgt,  ROT0, "Sega",   "Arabian Fight (World)", MACHINE_IMPERFECT_GRAPHICS ) /* Released in 03.1992 */
GAME( 1991, arabfgtu,  arabfgt,  sega_system32,     arabfgtu, segas32_new_state, arabfgt,  ROT0, "Sega",   "Arabian Fight (US)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1991, arabfgtj,  arabfgt,  sega_system32,     arabfgt,  segas32_new_state, arabfgt,  ROT0, "Sega",   "Arabian Fight (Japan)", MACHINE_IMPERFECT_GRAPHICS )

GAME( 1992, brival,    0,        sega_system32,     brival,   segas32_new_state, brival,   ROT0, "Sega",   "Burning Rival (World)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, brivalj,   brival,   sega_system32,     brival,   segas32_new_state, brival,   ROT0, "Sega",   "Burning Rival (Japan)", MACHINE_IMPERFECT_GRAPHICS )

GAME( 1992, darkedge,  0,        sega_system32,     darkedge, segas32_new_state, darkedge, ROT0, "Sega",   "Dark Edge (World)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, darkedgej, darkedge, sega_system32,     darkedge, segas32_new_state, darkedge, ROT0, "Sega",   "Dark Edge (Japan)", MACHINE_IMPERFECT_GRAPHICS )

GAME( 1994, dbzvrvs,   0,        sega_system32,     dbzvrvs,  segas32_new_state, dbzvrvs,  ROT0, "Sega / Banpresto", "Dragon Ball Z V.R.V.S. (Japan)", MACHINE_IMPERFECT_GRAPHICS)

GAME( 1991, f1en,      0,        sega_system32_dual_direct,     f1en,     segas32_new_state, f1en,     ROT0, "Sega",   "F1 Exhaust Note (World)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1991, f1enu,     f1en,     sega_system32_dual_direct,     f1en,     segas32_new_state, f1en,     ROT0, "Sega",   "F1 Exhaust Note (US)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1991, f1enj,     f1en,     sega_system32_dual_direct,     f1en,     segas32_new_state, f1en,     ROT0, "Sega",   "F1 Exhaust Note (Japan)", MACHINE_IMPERFECT_GRAPHICS )

GAME( 1993, f1lap,     0,        sega_system32,     f1lap,    segas32_new_state, f1lap,    ROT0, "Sega",   "F1 Super Lap (World)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1993, f1lapj,    f1lap,    sega_system32,     f1lap,    segas32_new_state, f1lap,    ROT0, "Sega",   "F1 Super Lap (Japan)", MACHINE_IMPERFECT_GRAPHICS )

GAME( 1992, ga2,       0,        sega_system32_v25, ga2,      segas32_new_state, ga2,      ROT0, "Sega",   "Golden Axe: The Revenge of Death Adder (World)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, ga2u,      ga2,      sega_system32_v25, ga2u,     segas32_new_state, ga2,      ROT0, "Sega",   "Golden Axe: The Revenge of Death Adder (US)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, ga2j,      ga2,      sega_system32_v25, ga2,      segas32_new_state, ga2,      ROT0, "Sega",   "Golden Axe: The Revenge of Death Adder (Japan)", MACHINE_IMPERFECT_GRAPHICS )

GAME( 1992, holo,      0,        sega_system32,     holo,     segas32_new_state, holo,     ORIENTATION_FLIP_Y, "Sega",   "Holosseum (US)", MACHINE_IMPERFECT_GRAPHICS )

GAME( 1993, jpark,     0,        sega_system32,     jpark,    segas32_new_state, jpark,    ROT0, "Sega",   "Jurassic Park (World)", MACHINE_IMPERFECT_GRAPHICS )  /* Released in 02.1994 */
GAME( 1993, jparkj,    jpark,    sega_system32,     jpark,    segas32_new_state, jpark,    ROT0, "Sega",   "Jurassic Park (Japan, Rev A, Deluxe)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1993, jparkja,   jpark,    sega_system32,     jpark,    segas32_new_state, jpark,    ROT0, "Sega",   "Jurassic Park (Japan, Deluxe)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1993, jparkjc,   jpark,    sega_system32,     jpark,    segas32_new_state, jpark,    ROT0, "Sega",   "Jurassic Park (Japan, Rev A, Conversion)", MACHINE_IMPERFECT_GRAPHICS )

GAME( 1994, kokoroj2,  0,        sega_system32,     radr,     segas32_new_state, radr,     ROT0, "Sega",   "Kokoroji 2", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING) /* uses an Audio CD */

GAME( 1990, radm,      0,        sega_system32,     radm,     segas32_new_state, radm,     ROT0, "Sega",   "Rad Mobile (World)", MACHINE_IMPERFECT_GRAPHICS )  /* Released in 02.1991 */
GAME( 1990, radmu,     radm,     sega_system32,     radm,     segas32_new_state, radm,     ROT0, "Sega",   "Rad Mobile (US)", MACHINE_IMPERFECT_GRAPHICS )

GAMEL(1991, radr,      0,        sega_system32,     radr,     segas32_new_state, radr,     ROT0, "Sega",   "Rad Rally (World)", MACHINE_IMPERFECT_GRAPHICS, layout_radr )
GAMEL(1991, radru,     radr,     sega_system32,     radr,     segas32_new_state, radr,     ROT0, "Sega",   "Rad Rally (US)", MACHINE_IMPERFECT_GRAPHICS, layout_radr )
GAMEL(1991, radrj,     radr,     sega_system32,     radr,     segas32_new_state, radr,     ROT0, "Sega",   "Rad Rally (Japan)", MACHINE_IMPERFECT_GRAPHICS, layout_radr )

GAMEL(1995, slipstrm,  0,        sega_system32,     slipstrm, segas32_new_state, slipstrm, ROT0, "Capcom", "Slip Stream (Brazil 950515)", MACHINE_IMPERFECT_GRAPHICS, layout_radr )
GAMEL(1995, slipstrmh, slipstrm, sega_system32,     slipstrm, segas32_new_state, slipstrm, ROT0, "Capcom", "Slip Stream (Hispanic 950515)", MACHINE_IMPERFECT_GRAPHICS, layout_radr )

GAME( 1992, sonic,     0,        sega_system32,     sonic,    segas32_new_state, sonic,    ROT0, "Sega",   "SegaSonic The Hedgehog (Japan, rev. C)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, sonicp,    sonic,    sega_system32,     sonic,    segas32_new_state, sonicp,   ROT0, "Sega",   "SegaSonic The Hedgehog (Japan, prototype)", MACHINE_IMPERFECT_GRAPHICS )

GAME( 1991, spidman,   0,        sega_system32,     spidman,  segas32_new_state, spidman,  ROT0, "Sega",   "Spider-Man: The Videogame (World)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1991, spidmanu,  spidman,  sega_system32,     spidmanu, segas32_new_state, spidman,  ROT0, "Sega",   "Spider-Man: The Videogame (US)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1991, spidmanj,  spidman,  sega_system32,     spidman,  segas32_new_state, spidman,  ROT0, "Sega",   "Spider-Man: The Videogame (Japan)", MACHINE_IMPERFECT_GRAPHICS )

GAME( 1994, svf,       0,        sega_system32,     svf,      segas32_new_state, svf,      ROT0, "Sega",   "Super Visual Football: European Sega Cup", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, svs,       svf,      sega_system32,     svf,      segas32_new_state, svf,      ROT0, "Sega",   "Super Visual Soccer: Sega Cup (US)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, jleague,   svf,      sega_system32,     svf,      segas32_new_state, jleague,  ROT0, "Sega",   "The J.League 1994 (Japan)", MACHINE_IMPERFECT_GRAPHICS )


GAME( 1994, harddunk,  0,        sega_multi32,      harddunk, segas32_new_state, harddunk, ROT0, "Sega",   "Hard Dunk (World)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, harddunkj, harddunk, sega_multi32,      harddunk, segas32_new_state, harddunk, ROT0, "Sega",   "Hard Dunk (Japan)", MACHINE_IMPERFECT_GRAPHICS )

GAME( 1992, orunners,  0,        sega_multi32,      orunners, segas32_new_state, orunners, ROT0, "Sega",   "OutRunners (World)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, orunnersu, orunners, sega_multi32,      orunners, segas32_new_state, orunners, ROT0, "Sega",   "OutRunners (US)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, orunnersj, orunners, sega_multi32,      orunners, segas32_new_state, orunners, ROT0, "Sega",   "OutRunners (Japan)", MACHINE_IMPERFECT_GRAPHICS )

GAME( 1992, scross,    0,        sega_multi32,      scross,   segas32_new_state, scross,   ROT0, "Sega",   "Stadium Cross (World)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, scrossa,   scross,   sega_multi32,      scross,   segas32_new_state, scross,   ROT0, "Sega",   "Stadium Cross (World, alt)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, scrossu,   scross,   sega_multi32,      scross,   segas32_new_state, scross,   ROT0, "Sega",   "Stadium Cross (US)", MACHINE_IMPERFECT_GRAPHICS )

GAME( 1992, titlef,    0,        sega_multi32,      titlef,   segas32_new_state, titlef,   ROT0, "Sega",   "Title Fight (World)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, titlefu,   titlef,   sega_multi32,      titlef,   segas32_new_state, titlef,   ROT0, "Sega",   "Title Fight (US)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, titlefj,   titlef,   sega_multi32,      titlef,   segas32_new_state, titlef,   ROT0, "Sega",   "Title Fight (Japan)", MACHINE_IMPERFECT_GRAPHICS )
