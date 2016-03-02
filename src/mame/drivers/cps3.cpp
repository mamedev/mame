// license:BSD-3-Clause
// copyright-holders:David Haywood, Andreas Naive, Tomasz Slanina, ElSemi
/*

CPS3 Driver (preliminary)

Decryption by Andreas Naive

Driver by David Haywood
 with help from Tomasz Slanina and ElSemi

Sound emulation by Philip Bennett

SCSI code by ElSemi

To-Do/Issues:

Street Fighter 3 2nd Impact uses flipped tilemaps during flashing, emulate this.

Figure out proper IRQ10 generation:
    If we generate on DMA operations only then Warzard is OK, otherwise it hangs during attract
    HOWEVER, SFIII2 sometimes has messed up character profiles unless we also generate it periodically.
    I think the corrupt background on some of the lighting effects may be related to this + the DMA
    status flags.

Alpha Blending Effects
    These are actually palette manipulation effects, not true blending.  How the values are used is
    not currently 100% understood.  They are incorrect if you use player 2 in Warzard

Linezoom
    Is it used anywhere??

Palette DMA effects
    Verify them, they might not be 100% accurate at the moment

Verify Full Screen Zoom on real hardware
    Which is which, x & y registers, how far can it zoom etc.

Verify CRT registers
    Only SFIII2 changes them, for widescreen mode.  What other modes are possible?

Sprite positioning glitches
    Some sprites are still in the wrong places, seems the placement of zooming sprites is imperfect
    eg. warzard intro + cutscenes leave the left most 16 pixels uncovered because the sprite is positioned incorrectly,
    the same occurs in the sf games.  doesn't look like the origin is correct when zooming in all cases.

Gaps in Sprite Zooming
    probably caused by use of drawgfx instead of processing as a single large sprite, but could also be due to the
    positioning of each part of the sprite.  Warzard is confirmed to have gaps during some cut-scenes on real hardware.

---

Capcom CP SYSTEM III Hardware Overview
Capcom, 1996-1999

From late 1996 to 1999 Capcom developed another hardware platform to rival the CPS2 System and called
it CP SYSTEM III. Only 6 games were produced. Here's a detailed table of all known releases until now:

                                                           |--------------- Cart ----------------|  |-------------- CD ---------------|
Game                                                 Year  Part #     Label      Region  CD  NO CD  Part #      Catalog #   Label        Revision
---------------------------------------------------  ----  ---------  ---------  ------  --  -----  ----------  ----------  -----------  --------
Warzard                                              1996  WZD96a00F  CP300000G  JAPAN   X          CAP-WZD000  CAP-WZD-3   CAP-WZD-3    961023
Warzard                                                    WZD96a00F  CP300000G  JAPAN   X          CAP-WZD000  CAP-WZD-5   CAP-WZD-5    961121
Red Earth                                                  WZD96aA0F  CP3000B0G  EUROPE  X          CAP-WZD0A0  CAP-WZD-3   CAP-WZD-3    961023
Red Earth                                                  WZD96aA0F  CP3000B0G  EUROPE  X          CAP-WZD0A0  CAP-WZD-5   CAP-WZD-5    961121
Red Earth                                                  WZD96aA0F  CP3000H0G  MEXICO* X          CAP-WZD0A0  CAP-WZD-3   CAP-WZD-3    961023
Red Earth                                                  WZD96aA0F  CP3000H0G  MEXICO* X          CAP-WZD0A0  CAP-WZD-5   CAP-WZD-5    961121
Red Earth                                                  WZD96aA0F  CP3000U0G  USA*    X          CAP-WZD0A0  CAP-WZD-3   CAP-WZD-3    961023
Red Earth                                                  WZD96aA0F  CP3000U0G  USA*    X          CAP-WZD0A0  CAP-WZD-5   CAP-WZD-5    961121

Street Fighter III: New Generation                   1997  SF397200F  CP300000G  JAPAN   X          CAP-SF3000  CAP-SF3-3   CAP-SF3-3    970204
Street Fighter III: New Generation                         SF397200F  CP300000G  JAPAN   X          CAP-SF3000  ?           ?            970312*
Street Fighter III: New Generation                         SF397200F  CP300000G  JAPAN   X          CAP-SF3000  ?           ?            970403*
Street Fighter III: New Generation                         SF3972A0F  CP3000B0G  EUROPE  X          CAP-SF30A0  CAP-SF3-3   CAP-SF3-3    970204
Street Fighter III: New Generation                         SF3972A0F  CP3000B0G  EUROPE  X          CAP-SF30A0  ?           ?            970312*
Street Fighter III: New Generation                         SF3972A0F  CP3000B0G  EUROPE  X          CAP-SF30A0  ?           ?            970403*
Street Fighter III: New Generation                         SF3972A0F  CP3000C0G  ASIA        X                                           970204
Street Fighter III: New Generation                         SF3972A0F  CP3000C0G  ASIA        X                                           970312*
Street Fighter III: New Generation                         SF3972A0F  CP3000C0G  ASIA        X                                           970403*
Street Fighter III: New Generation                         SF3972A0F  CP3000H0G  MEXICO  X          CAP-SF30A0  CAP-SF3-3   CAP-SF3-3    970204
Street Fighter III: New Generation                         SF3972A0F  CP3000H0G  MEXICO  X          CAP-SF30A0  ?           ?            970312*
Street Fighter III: New Generation                         SF3972A0F  CP3000H0G  MEXICO  X          CAP-SF30A0  ?           ?            970403*
Street Fighter III: New Generation                         SF3972A0F  CP3000U0G  USA     X          CAP-SF30A0  CAP-SF3-3   CAP-SF3-3    970204
Street Fighter III: New Generation                         SF3972A0F  CP3000U0G  USA     X          CAP-SF30A0  ?           ?            970312*
Street Fighter III: New Generation                         SF3972A0F  CP3000U0G  USA     X          CAP-SF30A0  ?           ?            970403*

Street Fighter III 2nd Impact: Giant Attack          1997  3GA97a00F  CP300000G  JAPAN   X          CAP-3GA000  CAP-3GA000  CAP-3GA-1    970930
Street Fighter III 2nd Impact: Giant Attack                3GA97a00F  CP300000G  JAPAN   X          CAP-3GA000  ?           ?            971016*
Street Fighter III 2nd Impact: Giant Attack                3GA97aA0F  CP3000C0G  ASIA        X                                           970930
Street Fighter III 2nd Impact: Giant Attack                3GA97aA0F  CP3000C0G  ASIA        X                                           971016*
Street Fighter III 2nd Impact: Giant Attack                3GA97aA0F  CP3000U0G  USA     X          CAP-3GA0A0  CAP-3GA000  CAP-3GA-1    970930
Street Fighter III 2nd Impact: Giant Attack                3GA97aA0F  CP3000U0G  USA     X          CAP-3GA0A0  ?           ?            971016*

JoJo no Kimyou na Bouken                             1998  JJK98c00F  CP300000G  JAPAN   X          CAP-JJK000  CAP-JJK000  CAP-JJK-140  981202
JoJo no Kimyou na Bouken                                   JJK98c00F  CP300000G  JAPAN   X          CAP-JJK000  CAP-JJK-2   CAP-JJK-160  990108
JoJo no Kimyou na Bouken                                   JJK98c00F  CP300000G  JAPAN   X          CAP-JJK000  CAP-JJK-3   CAP-JJK-161  990128
JoJo's Venture                                             JJK98cA0F  CP3000C0G  ASIA        X                                           981202
JoJo's Venture                                             JJK98cA0F  CP3000C0G  ASIA        X                                           990108
JoJo's Venture                                             JJK98cA0F  CP3000C0G  ASIA        X                                           990128
JoJo's Venture                                             JJK98cA0F  CP3000U0G  USA     X          CAP-JJK0A0  CAP-JJK000  CAP-JJK-140  981202
JoJo's Venture                                             JJK98cA0F  CP3000U0G  USA     X          CAP-JJK0A0  CAP-JJK-2   CAP-JJK-160  990108
JoJo's Venture                                             JJK98cA0F  CP3000U0G  USA     X          CAP-JJK0A0  CAP-JJK-3   CAP-JJK-161  990128

Street Fighter III 3rd Strike: Fight for the Future  1999  33S99400F  CP300000G  JAPAN*  X          CAP-33S000  CAP-33S-1   CAP-33S-1    990512
Street Fighter III 3rd Strike: Fight for the Future        33S99400F  CP300000G  JAPAN*  X          CAP-33S000  CAP-33S-2   CAP-33S-2    990608
Street Fighter III 3rd Strike: Fight for the Future        33S99400F  CP300000G  JAPAN       X                                           990512
Street Fighter III 3rd Strike: Fight for the Future        33S99400F  CP300000G  JAPAN       X                                           990608
Street Fighter III 3rd Strike: Fight for the Future        33S994A0F  CP3000B0G  EUROPE  X          CAP-33S0A0  CAP-33S-1   CAP-33S-1    990512
Street Fighter III 3rd Strike: Fight for the Future        33S994A0F  CP3000B0G  EUROPE  X          CAP-33S0A0  CAP-33S-2   CAP-33S-2    990608
Street Fighter III 3rd Strike: Fight for the Future        33S994A0F  CP3000C0G  ASIA*   X          CAP-33S0A0  CAP-33S-1   CAP-33S-1    990512
Street Fighter III 3rd Strike: Fight for the Future        33S994A0F  CP3000C0G  ASIA*   X          CAP-33S0A0  CAP-33S-2   CAP-33S-2    990608
Street Fighter III 3rd Strike: Fight for the Future        33S994A0F  CP3000C0G  ASIA*       X                                           990512
Street Fighter III 3rd Strike: Fight for the Future        33S994A0F  CP3000C0G  ASIA*       X                                           990608
Street Fighter III 3rd Strike: Fight for the Future        33S994A0F  CP3000H0G  MEXICO* ?          CAP-33S0A0  CAP-33S-1   CAP-33S-1    990512
Street Fighter III 3rd Strike: Fight for the Future        33S994A0F  CP3000H0G  MEXICO* ?          CAP-33S0A0  CAP-33S-2   CAP-33S-2    990608
Street Fighter III 3rd Strike: Fight for the Future        33S994A0F  CP3000U0G  USA     X          CAP-33S0A0  CAP-33S-1   CAP-33S-1    990512
Street Fighter III 3rd Strike: Fight for the Future        33S994A0F  CP3000U0G  USA     X          CAP-33S0A0  CAP-33S-2   CAP-33S-2    990608

JoJo no Kimyou na Bouken: Mirai e no Isan            1999  JJM99900F  CP300000G  JAPAN   X          CAP-JJM000  CAP-JJM-0   CAP-JJM-110  990913
JoJo no Kimyou na Bouken: Mirai e no Isan                  JJM99900F  CP300000G  JAPAN   X          CAP-JJM000  CAP-JJM-1   CAP-JJM-120  990927
JoJo no Kimyou na Bouken: Mirai e no Isan                  JJM99900F  CP300000G  JAPAN       X                                           990913
JoJo no Kimyou na Bouken: Mirai e no Isan                  JJM99900F  CP300000G  JAPAN       X                                           990927
JoJo's Bizarre Adventure                                   JJM999A0F  CP3000B0G  EUROPE      X                                           990913
JoJo's Bizarre Adventure                                   JJM999A0F  CP3000B0G  EUROPE      X                                           990927

* NOT DUMPED but known to exist

Each game consists of a cart and a CD having various codes needed to identify them. Carts and CDs have
both a different Part # printed on their front/top side that includes the game cart/CD code and ends
respectively with 00F/000 for all Japan releases and with A0F/0A0 for all the other ones. Therefore,
the part # can be used only to identify Japan releases and further parameters need to be introduced.
The cart is responsible for the game region that can be identified by a label with colored characters
and a code printed on the back side. The antepenultimate character of the label code and the colour of the
sticker vary by region, exactly as happens on the boot screen when the board is powered on. There are two
types of carts. Some require the CD to boot, some don't since the game is already loaded into the SIMMs.
Both types are externally identical and use the same codes, so the only way to distinguish them is to dump
the flashROMs. The game region and CD/NO CD flags are controlled by two different bytes in the flashROM.
The CD (and SIMMs too if the cart is of type NO CD) contains the game revision that can be identified
by two codes, the catalog # and the label. The catalog # is the identifying code printed in the mirror
ring on the top side close to the CD's center while the label is the code appearing on the CD icon when
it's inserted into a PC CD drive. It has been verified that the catalog # and label are the same for
some games but quite different for some others, so it's better to check both to avoid confusion. It
has also been verified that the catalog # and label (and the data on CDs) don't change between regions,
only between revisions. However, knowing one of them and comparing it with the table above will help
to understand if a new game revision has been discovered. Current CD dumps have been documented using
the catalog # as name, since the label is already included into the images used to generate CHDs.

The CP SYSTEM III comprises a main board with several custom ASICs, custom 72-pin SIMMs for program
and graphics storage (the same SIMMs are also used in some CPS2 titles), SCSI CDROM and CDROM disc,
and a plug-in security cart containing a boot flashROM, an NVRAM and a custom Capcom CPU containing
battery-backed decryption keys.

Not much is known about the actual CPU used in this system due to the extensive use of encryption and the volatile
nature of the security information. It is known that the CPU inside the security cart is the main CPU. It is known to
be a Hitachi SH-2 derivative thought to be based on a Hitachi HD6417099 SH2 variant with built-in encryption.
Tests were done by decrypting the security cart flashROM code and running it on the PCB with a dead cart with a zero
key and it didn't run so it is known that the custom CPU will not run standard (i.e. unencrypted) SH2 code.

The flashROM in the cart contains an encrypted program which is decrypted by the CPU in the cart. The CPU has built-in
decryption and the key is held in some static RAM on the CPU die and kept there by a battery. The code is executed by
the CPU to boot the system. Even though the code in the flashROM is encrypted, the cart can run it even if it is
dead/suicided because it has been discovered that the program contains a hidden security menu allowing the cart to be
loaded with the security data. This proves the cart runs the code even if the battery is dead. The special security
menu is not normally available but is likely accessed with a special key/button combination which is currently unknown.

Because the CPU in the cart is always powered by the battery, it has stealth capability that allows it to continually
monitor the situation. If the custom CPU detects any tampering (generally things such as voltage fluctuation or voltage
dropping or even removal of the cart with the power on), it immediately erases the SRAM (i.e. the decryption key)
inside the CPU which effectively kills the security cart. This also suggests that the custom Capcom CPU contains some
additional internal code to initiate the boot process because in order to re-program a cart using the hidden security
menu the CPU must execute some working code. It is known (from decapping it) that the CPU in the security cart contains
an amount of static RAM for data storage and a SH2 core based on the Hitachi SH7010-series (SH7014) SuperH RISC engine
family of Microprocessors.

It is thought that when a cartridge dies it will set the decryption keys identical to the ones of SFIII-2nd Impact, so
removing the battery and changing the content of the flashROM (if it's not a 2nd Impact) will make it run as a normal
SFIII-2nd Impact cartridge (is this verified on real hardware?)

The main board uses the familiar Capcom SIMM modules to hold the data from the CDROM so that the life of the CD drive
is maximized. The SIMMs don't contain RAM, but instead TSOP48 surface mounted flashROMs that can be updated with
different games on bootup using a built-in software updating system.
The SIMMs that hold the program code are located in positions 1 & 2 and are 64MBit.
The SIMMs that hold the graphics and sound data are located in positions 3, 4, 5, 6 & 7 and are 128MBit.
The data in the SIMMs is not decrypted, it is merely taken directly from the CDROM and shuffled slightly then
programmed to the flashROMs. The SIMMs hold the entire contents of the CDROM.

To swap games requires the security cart for the game, it's CDROM disc and the correctly populated type and number of
SIMMs on the main board.
On first power-up after switching the cart and CD, you're presented with a screen asking if you want to re-program the
SIMMs with the new game. Pressing player 1 button 2 cancels it. Pressing player 1 button 1 allows it to proceed whereby
you wait about 25-30 minutes then the game boots up almost immediately. On subsequent power-ups, the game boots
immediately.
If the CDROM is not present in the drive on a normal bootup, a message tells you to insert the CDROM.
Then you press button 1 to continue and the game boots immediately.
Note that not all of the SIMMs are populated on the PCB for each game. Some games have more, some less, depending on
game requirements, so flash times can vary per game. See the table below for details.

                                                     |----------- Required SIMM Locations & Types -----------|
Game                                                 1       2       3        4        5         6         7
--------------------------------------------------------------------------------------------------------------
Red Earth / Warzard                                  64MBit  -       128MBit  128MBit  32MBit*   -         -
Street Fighter III: New Generation                   64MBit  -       128MBit  128MBit  32MBit*   -         -
Street Fighter III 2nd Impact: Giant Attack          64MBit  64MBit  128MBit  128MBit  128MBit   -         -
JoJo's Venture                                       64MBit  64MBit  128MBit  128MBit  32MBit*   -         -
Street Fighter III 3rd Strike: Fight for the Future  64MBit  64MBit  128MBit  128MBit  128MBit   128MBit   -
JoJo's Bizarre Adventure                             64MBit  64MBit  128MBit  128MBit  128MBit   -         -

                                                     Notes:
                                                           - denotes not populated
                                                           * 32MBit SIMMs have only 2 FlashROMs populated on them.
                                                             128MBit SIMMs can also be used.
                                                           No game uses a SIMM at 7
                                                           See main board diagram below for SIMM locations.

Due to the built-in upgradability of the hardware, and the higher frame-rates the hardware seems to have, it appears
Capcom had big plans for this system and possibly intended to create many games on it, as they did with CPS2.
Unfortunately for Capcom, CP SYSTEM III was an absolute flop in the arcades so those plans were cancelled. Possible
reasons include:
- the games were essentially just 2D, and already there were many 3D games coming out onto the market that interested
  operators more than this.
- the cost of the system was quite expensive when compared to other games on the market.
- it is rumoured that the system was difficult to program for developers.
- these PCBs were not popular with operators because the security carts are extremely static-sensitive and most of them
  failed due to the decryption information being zapped by simple handling of the PCBs or by touching the security cart
  edge connector underneath the PCB while the security cart was plugged in, or by power fluctuations while flashing the
  SIMMs. You will know if your cart has been zapped because on bootup, you get a screen full of garbage coloured pixels
  instead of the game booting up, or just a black or single-colored screen. You should also not touch the inside of the
  security cart. The PCB can detect the presence of the security cart and if it is removed on a working game, the game
  will freeze immediately and it will also erase the security cart battery-backed decryption data.


PCB Layouts
-----------

CAPCOM
CP SYSTEM III
95682A-4 (older rev 95682A-3)
   |----------------------------------------------------------------------|
  |= J1             HM514260(2)  |------------|      |  |  |  |  |        |
   |                             |CAPCOM      |      |  |  |  |  |        |
  |= J2     TA8201  TC5118160    |DL-2729 PPU |      |  |  |  |  |        |
   |                             |(QFP304)    |      |  |  |  |  |        |
|--|          VOL   TC5118160    |            |      |  |  |  |  |        |
|    LM833N                      |            |      S  S  S  S  S        |
|    LM833N         TC5118160    |------------|      I  I  I  I  I        |
|           TDA1306T                      |--------| M  M  M  M  M        |
|                   TC5118160  60MHz      |CAPCOM  | M  M  M  M  M       |-|
|                              42.9545MHz |DL-3329 | 7  6  5  4  3       | |
|           LM385                         |SSU     | |  |  |  |  |       | |
|J                         KM681002       |--------| |  |  |  |  |       | |
|A                         KM681002  62256 |-------| |  |  |  |  |       | |
|M                                         |DL3529 | |  |  |  |  |       | |
|M          MC44200FU                      |GLL2   | |  |  |  |  |       | |
|A                              3.6864MHz  |-------|                  CN6| |
|                                                             |  |       | |
|                               |--------|   |-|              |  |       | |
|                               |CAPCOM  |   | |   |-------|  |  |       | |
|        TD62064                |DL-2929 |   | |   |CAPCOM |  |  |       | |
|                               |IOU     |   | |   |DL-3429|  |  |       | |
|        TD62064                |--------|   | |   |GLL1   |  S  S       | |
|--|                            *HA16103FPJ  | |   |-------|  I  I       |-|
   |                                         | |CN5           M  M        |
   |                                         | |   |-------|  M  M        |
  |-|                        93C46           | |   |CAPCOM |  2  1        |
  | |      PS2501                            | |   |DL-2829|  |  | |-----||
  | |CN1                                     | |   |CCU    |  |  | |AMD  ||
  | |      PS2501                            | |   |-------|  |  | |33C93||
  |-|                                        |-|              |  | |-----||
   |   SW1                                         HM514260   |  |        |
   |----------------------------------------------------------------------|
Notes:
      TA8201     - Toshiba TA8201 18W BTL x 2-Channel Audio Power Amplifier
      PS2501     - NEC PS2501 High Isolation Voltage Single Transistor Type Multi Photocoupler (DIP16)
      TDA1306T   - Philips TDA1306T Noise Shaping Filter DAC (SOIC24). The clock (on pin 12) measures
                   14.3181667MHz (42.9545/3)
      MC44200FU  - Motorola MC44200FU Triple 8-bit Video DAC (QFP44)
      LM833N     - ST Microelectronics LM833N Low Noise Audio Dual Op-Amp (DIP8)
      TD62064    - Toshiba TD62064AP NPN 50V 1.5A Quad Darlington Driver (DIP16)
      HA16103FPJ - Hitachi HA16103FPJ Watchdog Timer (SOIC20)
                   *Note this IC is not populated on the rev -4 board
      93C46      - National Semiconductor NM93C46A 128bytes x8 Serial EEPROM (SOIC8)
                   Note this IC is covered by a plastic housing on the PCB. The chip is just a normal
                   (unsecured) EEPROM so why it was covered is not known.
      LM385      - National Semiconductor LM385 Adjustable Micropower Voltage Reference Diode (SOIC8)
      33C93      - AMD 33C93A-16 SCSI Controller (PLCC44)
      KM681002   - Samsung Electronics KM681002 128k x8 SRAM (SOJ32). This is the 'Color RAM' in the test mode memory
                   test
      62256      - 8k x8 SRAM (SOJ28). This is the 'SS RAM' in the test mode memory test and is connected to the custom
                   SSU chip.
      HM514260(1)- Hitachi HM514260CJ7 1M x16 DRAM (SOJ40). This is the 'Work RAM' in the test mode memory test and is
                   connected to the custom CCU chip.
      HM514260(2)- Hitachi HM514260CJ7 1M x16 DRAM (SOJ40). This is the 'Sprite RAM' in the test mode memory test
      TC5118160  - Toshiba TC5118160BJ-60 or NEC 4218160-60 256k x16 DRAM (SOJ42). This is the 'Character RAM' in the
                   test mode memory test
      SW1        - Push-button Test Switch
      VOL        - Master Volume Potentiometer
      J1/J2      - Optional RCA Left/Right Audio Out Connectors
      CN1        - 34-Pin Capcom Kick Button Harness Connector
      CN5        - Security Cartridge Slot
      CN6        - 4-Pin Power Connector and 50-pin SCSI Data Cable Connector
                   CDROM Drive is a CR504-KCM 4X SCSI drive manufactured By Panasonic / Matsushita
      SIMM 1-2   - 72-Pin SIMM Connector, holds single sided SIMMs containing 4x Fujitsu 29F016A
                   surface mounted TSOP48 FlashROMs
      SIMM 3-7   - 72-Pin SIMM Connector, holds double sided SIMMs containing 8x Fujitsu 29F016A
                   surface mounted TSOP48 FlashROMs

                   SIMM Layout -
                          |----------------------------------------------------|
                          |                                                    |
                          |   |-------|   |-------|   |-------|   |-------|    |
                          |   |Flash_A|   |Flash_B|   |Flash_C|   |Flash_D|    |
                          |   |-------|   |-------|   |-------|   |-------|    |
                          |-                                                   |
                           |-------------------------/\------------------------|
                           Notes:
                                  For SIMMs 1-2, Flash_A & Flash_C and regular pinout (Fujitsu 29F016A-90PFTN)
                                  Flash_B & Flash_D are reverse pinout (Fujitsu 29F016A-90PFTR)
                                  and are mounted upside down also so that pin1 lines up with
                                  the normal pinout of FlashROMs A & C.
                                  For SIMMs 3-7, the 8 FlashROMs are populated on both sides using a similar layout.

      Capcom Custom ASICs -
                           DL-2729 PPU SD10-505   (QFP304) - Graphics chip.
                           DL-2829 CCU SD07-1514  (QFP208) - Probably a companion CPU or co-processor. Decapping
                                                             reveals it is manufactured by Toshiba. The 'Work RAM' is
                                                             connected to it.
                           DL-2929 IOU SD08-1513  (QFP208) - I/O controller.
                           DL-3329 SSU SD04-1536  (QFP144) - Sound chip, clocked at 21.47725MHz (42.9545/2). It has 32k
                                                             SRAM connected to it.
                           DL-3429 GLL1 SD06-1537 (QFP144) - DMA memory/bus controller.
                           DL-3529 GLL2 SD11-1755 (QFP80)  - ROM/SIMM bank selection chip (via 3x FCT162244 logic ICs).


Connector Pinouts
-----------------

                       JAMMA Connector                                       Extra Button Connector
                       ---------------                                       ----------------------
                    PART SIDE    SOLDER SIDE                                       TOP    BOTTOM
                ----------------------------                               --------------------------
                      GND  01    A  GND                                        GND  01    02  GND
                      GND  02    B  GND                                        +5V  03    04  +5V
                      +5V  03    C  +5V                                       +12V  05    06  +12V
                      +5V  04    D  +5V                                             07    08
                       NC  05    E  NC                           Player 2 Button 4  09    10
                     +12V  06    F  +12V                                            11    12
                           07    H                                                  13    14
           Coin Counter 1  08    J  NC                           Player 1 Button 4  15    16
             Coin Lockout  09    K  Coin Lockout                 Player 1 Button 5  17    18
               Speaker (+) 10    L  Speaker (-)                  Player 1 Button 6  19    20
                       NC  11    M  NC                           Player 2 Button 5  21    22
                Video Red  12    N  Video Green                  Player 2 Button 6  23    24
               Video Blue  13    P  Video Composite Sync                            25    26
             Video Ground  14    R  Service Switch                                  27    28
                     Test  15    S  NC                                 Volume Down  29    30  Volume UP
                   Coin A  16    T  Coin B                                     GND  31    32  GND
           Player 1 Start  17    U  Player 2 Start                             GND  33    34  GND
              Player 1 Up  18    V  Player 2 Up
            Player 1 Down  19    W  Player 2 Down
            Player 1 Left  20    X  Player 2 Left
           Player 1 Right  21    Y  Player 2 Right
        Player 1 Button 1  22    Z  Player 2 Button 1
        Player 1 Button 2  23    a  Player 2 Button 2
        Player 1 Button 3  24    b  Player 2 Button 3
                       NC  25    c  NC
                       NC  26    d  NC
                      GND  27    e  GND
                      GND  28    f  GND


Security Cartridge PCB Layout
-----------------------------

There are 4 types of CPS3 security carts. They have a label on the custom CPU that can be either A, B, C or D.
Cartridge types A/B are identical and cartridge types C/D are identical.
Type A/B have extra space on the back side to solder a 28F400 SOP44 flashROM which shares all electrical connections
with the 29F400 TSOP48 flashROM on the front side of the PCB. Either chip can be used to store the 512k cart program,
but no cart has been seen with a SOP44 flashROM populated, nor with both SOP44 and TSOP48 populated on one cart.
A and B cartridges also contain a FM1208S NVRAM which holds game settings or other per-game data. It is definitely
used. If the NVRAM data is not present when the game boots or the NVRAM is not working or inaccessible a message is
displayed 'EEPROM ERROR' and the game halts. This error can also occur if the security cart edge connector is dirty
and not contacting properly.
C and D cartridges lack the extra space to solder a SOP44 flashROM. A space is available on the back side for a FM1208S
NVRAM but it is not populated. A MACH111 CPLD is present on the back side and stamped 'CP3B1A'


Type A and Type B
-----------------

CAPCOM 95682B-3 TORNADE
|------------------------------------------------|
|      BATTERY                                   |
|                          |-------|             |
|                          |CAPCOM |   29F400    |
|                          |DL-3229|   *28F400   |
|                          |SCU    |     *FM1208S|
| 74HC00                   |-------|             |
|               6.25MHz                    74F00 |
|---|     |-|                             |------|
    |     | |                             |
    |-----| |-----------------------------|
Notes:
      74F00        - 74F00 Quad 2-Input NAND Gate (SOIC14)
      74HC00       - Philips 74HC00N Quad 2-Input NAND Gate (DIP14)
      29F400       - Fujitsu 29F400TA-90PFTN 512k x8 FlashROM (TSOP48)
      Custom ASIC  - CAPCOM DL-3229 SCU (QFP144). Decapping reveals this is a Hitachi HD6417099 SH2 variant
                     with built-in encryption, clocked at 6.250MHz
      FM1208S      - RAMTRON FM1208S 4k (512bytes x8) Nonvolatile Ferroelectric RAM (SOIC24)
      28F400       - 28F400 SOP44 FlashROM (not populated)
      *            - These components located on the other side of the PCB

      Note: The battery powers the CPU only. A small board containing some transistors/resistors is wired to the 74HC00
      to switch the CPU from battery power to main power to increase the life of the battery.


Type C and Type D
-----------------

CAPCOM 95682B-4 CP SYSTEM III
|------------------------------------------------|
|      BATTERY                                   |
|                          |-------|             |
|                          |CAPCOM |   29F400    |
|                          |DL-3229|   *MACH111  |
|                          |SCU    |     *FM1208S|
| 74HC00                   |-------|             |
|               6.25MHz                    74F00 |
|---|     |-|                             |------|
    |     | |                             |
    |-----| |-----------------------------|
Notes:
      74F00        - 74F00 Quad 2-Input NAND Gate (SOIC14)
      74HC00       - Philips 74HC00N Quad 2-Input NAND Gate (DIP14)
      29F400       - Fujitsu 29F400TA-90PFTN 512k x8 FlashROM (TSOP48)
      Custom ASIC  - CAPCOM DL-3229 SCU (QFP144). Decapping reveals this is a Hitachi HD6417099 SH2 variant
                     with built-in encryption, clocked at 6.250MHz
      FM1208S      - RAMTRON FM1208S 4k (512bytes x8) Nonvolatile Ferroelectric RAM (not populated)
      MACH111      - AMD MACH111 CPLD stamped 'CP3B1A' (PLCC44)
      *            - These components located on the other side of the PCB

      Note: The battery powers the CPU only. Some transistors/resistors present on the PCB and wired to the 74HC00
      switch the CPU from battery power to main power to increase the life of the battery.


Security cart resurrection info
-------------------------------

When the security cart dies the game no longer functions. The PCB can be brought back to life by doing the following
hardware modification to the security cart.....

1. Remove the custom QFP144 CPU and replace it with a standard Hitachi HD6417095 SH-2 CPU
2. Remove the 29F400 TSOP48 flashROM and re-program it with the decrypted and modified main program ROM from set
   'cps3boot' in MAME. A 28F400 SOP44 flashROM can be used instead and mounted to the back side of the security cart
   PCB. Do not mount both SOP44 and TSOP48 flashROMs, use only one TSOP48 flashROM or one SOP44 flashROM.
3. Power on the PCB and using the built-in cart flashROM menu re-program the SIMMs for your chosen game using the CD
   from set 'cps3boot' in MAME.
4. That is all. Enjoy your working PCB.

*/

#include "emu.h"
#include "cdrom.h"
#include "cpu/sh2/sh2.h"
#include "machine/intelfsh.h"
#include "machine/nvram.h"
#include "includes/cps3.h"
#include "bus/scsi/scsi.h"
#include "bus/scsi/scsicd.h"
#include "machine/wd33c93.h"

#include "sfiii2.lh"

#define MASTER_CLOCK    42954500

#define DEBUG_PRINTF 0



#define DMA_XOR(a)      ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(1,2))

#define USER4REGION_LENGTH 0x800000*2
#define USER5REGION_LENGTH 0x800000*10

#define CPS3_TRANSPARENCY_NONE 0
#define CPS3_TRANSPARENCY_PEN 1
#define CPS3_TRANSPARENCY_PEN_INDEX 2
#define CPS3_TRANSPARENCY_PEN_INDEX_BLEND 3

inline void cps3_state::cps3_drawgfxzoom(bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx,
		unsigned int code,unsigned int color,int flipx,int flipy,int sx,int sy,
		int transparency,int transparent_color,
		int scalex, int scaley,bitmap_ind8 *pri_buffer,UINT32 pri_mask)
{
	rectangle myclip;

//  UINT8 al;

//  al = (pdrawgfx_shadow_lowpri) ? 0 : 0x80;

	if (!scalex || !scaley) return;

// todo: reimplement this optimization!!
//  if (scalex == 0x10000 && scaley == 0x10000)
//  {
//      common_drawgfx(dest_bmp,gfx,code,color,flipx,flipy,sx,sy,clip,transparency,transparent_color,pri_buffer,pri_mask);
//      return;
//  }

	/*
	scalex and scaley are 16.16 fixed point numbers
	1<<15 : shrink to 50%
	1<<16 : uniform scale
	1<<17 : double to 200%
	*/


	/* force clip to bitmap boundary */
	myclip = clip;
	myclip &= dest_bmp.cliprect();

	/* 32-bit ONLY */
	{
		if( gfx )
		{
//          const pen_t *pal = &gfx->colortable[gfx->granularity() * (color % gfx->colors())];
			UINT32 palbase = (gfx->granularity() * color) & 0x1ffff;
			const pen_t *pal = &m_mame_colours[palbase];
			const UINT8 *source_base = gfx->get_data(code % gfx->elements());

			int sprite_screen_height = (scaley*gfx->height()+0x8000)>>16;
			int sprite_screen_width = (scalex*gfx->width()+0x8000)>>16;

			if (sprite_screen_width && sprite_screen_height)
			{
				/* compute sprite increment per screen pixel */
				int dx = (gfx->width()<<16)/sprite_screen_width;
				int dy = (gfx->height()<<16)/sprite_screen_height;

				int ex = sx+sprite_screen_width;
				int ey = sy+sprite_screen_height;

				int x_index_base;
				int y_index;

				if( flipx )
				{
					x_index_base = (sprite_screen_width-1)*dx;
					dx = -dx;
				}
				else
				{
					x_index_base = 0;
				}

				if( flipy )
				{
					y_index = (sprite_screen_height-1)*dy;
					dy = -dy;
				}
				else
				{
					y_index = 0;
				}

				if( sx < myclip.min_x)
				{ /* clip left */
					int pixels = myclip.min_x-sx;
					sx += pixels;
					x_index_base += pixels*dx;
				}
				if( sy < myclip.min_y )
				{ /* clip top */
					int pixels = myclip.min_y-sy;
					sy += pixels;
					y_index += pixels*dy;
				}
				if( ex > myclip.max_x+1 )
				{ /* clip right */
					int pixels = ex-myclip.max_x-1;
					ex -= pixels;
				}
				if( ey > myclip.max_y+1 )
				{ /* clip bottom */
					int pixels = ey-myclip.max_y-1;
					ey -= pixels;
				}

				if( ex>sx )
				{ /* skip if inner loop doesn't draw anything */
					int y;

					/* case 0: no transparency */
					if (transparency == CPS3_TRANSPARENCY_NONE)
					{
						{
							for( y=sy; y<ey; y++ )
							{
								const UINT8 *source = source_base + (y_index>>16) * gfx->rowbytes();
								UINT32 *dest = &dest_bmp.pix32(y);

								int x, x_index = x_index_base;
								for( x=sx; x<ex; x++ )
								{
									dest[x] = pal[source[x_index>>16]];
									x_index += dx;
								}

								y_index += dy;
							}
						}
					}
					else if (transparency == CPS3_TRANSPARENCY_PEN)
					{
						{
							for( y=sy; y<ey; y++ )
							{
								const UINT8 *source = source_base + (y_index>>16) * gfx->rowbytes();
								UINT32 *dest = &dest_bmp.pix32(y);

								int x, x_index = x_index_base;
								for( x=sx; x<ex; x++ )
								{
									int c = source[x_index>>16];
									if( c != transparent_color ) dest[x] = pal[c];
									x_index += dx;
								}

								y_index += dy;
							}
						}
					}
					else if (transparency == CPS3_TRANSPARENCY_PEN_INDEX)
					{
						{
							for( y=sy; y<ey; y++ )
							{
								const UINT8 *source = source_base + (y_index>>16) * gfx->rowbytes();
								UINT32 *dest = &dest_bmp.pix32(y);

								int x, x_index = x_index_base;
								for( x=sx; x<ex; x++ )
								{
									int c = source[x_index>>16];
									if( c != transparent_color ) dest[x] = c | palbase;
									x_index += dx;
								}

								y_index += dy;
							}
						}
					}
					else if (transparency == CPS3_TRANSPARENCY_PEN_INDEX_BLEND)
					{
						{
							for( y=sy; y<ey; y++ )
							{
								const UINT8 *source = source_base + (y_index>>16) * gfx->rowbytes();
								UINT32 *dest = &dest_bmp.pix32(y);

								int x, x_index = x_index_base;
								for( x=sx; x<ex; x++ )
								{
									int c = source[x_index>>16];
									if( c != transparent_color )
									{
										/* blending isn't 100% understood */
										if (gfx->granularity() == 64)
										{
											// OK for sfiii2 spotlight
											if (c&0x01) dest[x] |= 0x2000;
											if (c&0x02) dest[x] |= 0x4000;
											if (c&0x04) dest[x] |= 0x8000;
											if (c&0x08) dest[x] |= 0x10000;
											if (c&0xf0) dest[x] |= machine().rand(); // ?? not used?
										}
										else
										{
											// OK for jojo intro, and warzard swords, and various shadows in sf games
											if (c&0x01) dest[x] |= 0x8000;
											if (color&0x100) dest[x]|=0x10000;
										}
									}


									x_index += dx;
								}

								y_index += dy;
							}
						}
					}
				}
			}
		}
	}
}



/* Encryption */


UINT16 cps3_state::rotate_left(UINT16 value, int n)
{
	int aux = value>>(16-n);
	return ((value<<n)|aux)%0x10000;
}

UINT16 cps3_state::rotxor(UINT16 val, UINT16 xorval)
{
	UINT16 res;

	res = val + rotate_left(val,2);

	res = rotate_left(res,4) ^ (res & (val ^ xorval));

	return res;
}

UINT32 cps3_state::cps3_mask(UINT32 address, UINT32 key1, UINT32 key2)
{
	// ignore all encryption
	if (m_altEncryption == 2)
		return 0;

	UINT16 val;

	address ^= key1;

	val = (address & 0xffff) ^ 0xffff;

	val = rotxor(val, key2 & 0xffff);

	val ^= (address >> 16) ^ 0xffff;

	val = rotxor(val, key2 >> 16);

	val ^= (address & 0xffff) ^ (key2 & 0xffff);

	return val | (val << 16);
}

void cps3_state::cps3_decrypt_bios()
{
	int i;
	UINT32 *coderegion = (UINT32*)memregion("bios")->base();

	for (i=0;i<0x80000;i+=4)
	{
		UINT32 dword = coderegion[i/4];
		UINT32 xormask = cps3_mask(i, m_key1, m_key2);
		coderegion[i/4] = dword ^ xormask;
	}
#if 0
	/* Dump to file */
	{
		FILE *fp;
		const char *gamename = machine().system().name;
		char filename[256];
		sprintf(filename, "%s_bios.dump", gamename);

		fp=fopen(filename, "w+b");
		if (fp)
		{
			fwrite(m_decrypted_bios, 0x080000, 1, fp);
			fclose(fp);
		}
	}
#endif
}

void cps3_state::init_common(void)
{
	// flash roms
	for (int simmnum = 0; simmnum < 7; simmnum++)
		for (int chipnum = 0; chipnum < 8; chipnum++)
			m_simm[simmnum][chipnum] = machine().device<fujitsu_29f016a_device>(string_format("simm%d.%d", simmnum + 1, chipnum).c_str());

	m_eeprom = std::make_unique<UINT32[]>(0x400/4);
	machine().device<nvram_device>("eeprom")->set_base(m_eeprom.get(), 0x400);
}


void cps3_state::init_crypt(UINT32 key1, UINT32 key2, int altEncryption)
{
	m_key1 = key1;
	m_key2 = key2;
	m_altEncryption = altEncryption;

	// cache pointers to regions
	if (m_user4_region)
	{
		m_user4 = m_user4_region->base();
	}
	else
	{
		m_user4 = auto_alloc_array(machine(), UINT8, USER4REGION_LENGTH);
	}

	if (m_user5_region)
	{
		m_user5 = m_user5_region->base();
	}
	else
	{
		m_user5 = auto_alloc_array(machine(), UINT8, USER5REGION_LENGTH);
	}

	m_cps3sound->set_base((INT8*)m_user5);

	// set strict verify
	m_maincpu->sh2drc_set_options(SH2DRC_STRICT_VERIFY);
	m_maincpu->sh2drc_add_fastram(0x02000000, 0x0207ffff, 0, &m_mainram[0]);
	m_maincpu->sh2drc_add_fastram(0x04000000, 0x0407ffff, 0, &m_spriteram[0]);
	m_maincpu->sh2drc_add_fastram(0x040C0020, 0x040C002b, 0, &m_tilemap20_regs_base[0]);
	m_maincpu->sh2drc_add_fastram(0x040C0030, 0x040C003b, 0, &m_tilemap30_regs_base[0]);

	cps3_decrypt_bios();

	init_common();
}

DRIVER_INIT_MEMBER(cps3_state,redearth)  { init_crypt(0x9e300ab1, 0xa175b82c, 0); }
DRIVER_INIT_MEMBER(cps3_state,sfiii)     { init_crypt(0xb5fe053e, 0xfc03925a, 0); }
DRIVER_INIT_MEMBER(cps3_state,sfiii2)    { init_crypt(0x00000000, 0x00000000, 1); } // sfiii2 runs off a 'dead' cart
DRIVER_INIT_MEMBER(cps3_state,jojo)      { init_crypt(0x02203ee3, 0x01301972, 0); }
DRIVER_INIT_MEMBER(cps3_state,sfiii3)    { init_crypt(0xa55432b4, 0x0c129981, 0); }
DRIVER_INIT_MEMBER(cps3_state,jojoba)    { init_crypt(0x23323ee3, 0x03021972, 0); }
DRIVER_INIT_MEMBER(cps3_state,cps3boot)  { init_crypt((UINT32)-1,(UINT32)-1,2); }



/* GFX decodes */


static const gfx_layout cps3_tiles16x16_layout =
{
	16,16,
	0x8000,
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 3*8,2*8,1*8,0*8,7*8,6*8,5*8,4*8,
		11*8,10*8,9*8,8*8,15*8,14*8,13*8,12*8 },
	{ 0*128,1*128,2*128,3*128,4*128,5*128,6*128,7*128,8*128,9*128,10*128,11*128,12*128,13*128,14*128,15*128},
	8*256
};



static const gfx_layout cps3_tiles8x8_layout =
{
	8,8,
	0x400,
	4,
	{ /*8,9,10,11,*/ 0,1,2,3 },
	{ 20,16,4,0,52,48,36,32 },

	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	64*8
};


void cps3_state::cps3_set_mame_colours(int colournum, UINT16 data, UINT32 fadeval)
{
	int r,g,b;
	UINT16* dst = (UINT16*)m_colourram.target();


	r = (data >> 0) & 0x1f;
	g = (data >> 5) & 0x1f;
	b = (data >> 10) & 0x1f;

	/* is this 100% correct? */
	if (fadeval!=0)
	{
		int fade;
		//printf("fadeval %08x\n",fadeval);

		fade = (fadeval & 0x3f000000)>>24;
		r = (r*fade)>>5;
		if (r>0x1f) r = 0x1f;

		fade = (fadeval & 0x003f0000)>>16;
		g = (g*fade)>>5;
		if (g>0x1f) g = 0x1f;

		fade = (fadeval & 0x0000003f)>>0;
		b = (b*fade)>>5;
		if (b>0x1f) b = 0x1f;

		data = (r <<0) | (g << 5) | (b << 10);
	}

	dst[colournum] = data;

	m_mame_colours[colournum] = (r << (16+3)) | (g << (8+3)) | (b << (0+3));

	if (colournum<0x10000) m_palette->set_pen_color(colournum,m_mame_colours[colournum]/* rgb_t(r<<3,g<<3,b<<3)*/);//m_mame_colours[colournum]);
}


void cps3_state::video_start()
{
	m_ss_ram       = std::make_unique<UINT32[]>(0x10000/4);
	memset(m_ss_ram.get(), 0x00, 0x10000);
	save_pointer(NAME(m_ss_ram.get()), 0x10000/4);

	m_char_ram = std::make_unique<UINT32[]>(0x800000/4);
	memset(m_char_ram.get(), 0x00, 0x800000);
	save_pointer(NAME(m_char_ram.get()), 0x800000 /4);

	/* create the char set (gfx will then be updated dynamically from RAM) */
	m_gfxdecode->set_gfx(0, std::make_unique<gfx_element>(m_palette, cps3_tiles8x8_layout, (UINT8 *)m_ss_ram.get(), 0, m_palette->entries() / 16, 0));

	//decode_ssram();

	/* create the char set (gfx will then be updated dynamically from RAM) */
	m_gfxdecode->set_gfx(1, std::make_unique<gfx_element>(m_palette, cps3_tiles16x16_layout, (UINT8 *)m_char_ram.get(), 0, m_palette->entries() / 64, 0));
	m_gfxdecode->gfx(1)->set_granularity(64);

	//decode_charram();

	m_mame_colours = std::make_unique<UINT32[]>(0x80000/4);
	memset(m_mame_colours.get(), 0x00, 0x80000);

	m_screenwidth = 384;

	// the renderbuffer can be twice the size of the screen, this allows us to handle framebuffer zoom values
	// between 0x00 and 0x80 (0x40 is normal, 0x80 would be 'view twice as much', 0x20 is 'view half as much')
	m_renderbuffer_bitmap.allocate(512*2,224*2);

	m_renderbuffer_clip.set(0, m_screenwidth-1, 0, 224-1);

	m_renderbuffer_bitmap.fill(0x3f, m_renderbuffer_clip);

}

// the 0x400 bit in the tilemap regs is "draw it upside-down"  (bios tilemap during flashing, otherwise capcom logo is flipped)

void cps3_state::cps3_draw_tilemapsprite_line(int tmnum, int drawline, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	UINT32* tmapregs[4] = { m_tilemap20_regs_base, m_tilemap30_regs_base, m_tilemap40_regs_base, m_tilemap50_regs_base };
	UINT32* regs;
	int line;
	int scrolly;
	if (tmnum>3)
	{
		printf("cps3_draw_tilemapsprite_line Illegal tilemap number %d\n",tmnum);
		return;
	}
	regs = tmapregs[tmnum];

	scrolly =  ((regs[0]&0x0000ffff)>>0)+4;
	line = drawline+scrolly;
	line&=0x3ff;


	if (!(regs[1]&0x00008000)) return;

	{
		UINT32 mapbase =  (regs[2]&0x007f0000)>>16;
		UINT32 linebase=  (regs[2]&0x7f000000)>>24;
		int linescroll_enable = (regs[1]&0x00004000);

		int scrollx;
		int x;
		int tileline = (line/16)+1;
		int tilesubline = line % 16;
		rectangle clip;

		mapbase = mapbase << 10;
		linebase = linebase << 10;

		if (!linescroll_enable)
		{
			scrollx =  (regs[0]&0xffff0000)>>16;
		}
		else
		{
		//  printf("linebase %08x\n", linebase);

			scrollx =  (regs[0]&0xffff0000)>>16;
			scrollx+= (m_spriteram[linebase+((line+16-4)&0x3ff)]>>16)&0x3ff;

		}

//  zoombase    =  (layerregs[1]&0xffff0000)>>16;

		drawline&=0x3ff;

		if (drawline>cliprect.max_y+4) return;

		clip.set(cliprect.min_x, cliprect.max_x, drawline, drawline);

		for (x=0;x<(cliprect.max_x/16)+2;x++)
		{
			UINT32 dat;
			int tileno;
			int colour;
			int bpp;
			int xflip,yflip;

			dat = m_spriteram[mapbase+((tileline&63)*64)+((x+scrollx/16)&63)];
			tileno = (dat & 0xffff0000)>>17;
			colour = (dat & 0x000001ff)>>0;
			bpp = (dat & 0x0000200)>>9;
			yflip  = (dat & 0x00000800)>>11;
			xflip  = (dat & 0x00001000)>>12;

			if (!bpp) m_gfxdecode->gfx(1)->set_granularity(256);
			else m_gfxdecode->gfx(1)->set_granularity(64);

			cps3_drawgfxzoom(bitmap,clip,m_gfxdecode->gfx(1),tileno,colour,xflip,yflip,(x*16)-scrollx%16,drawline-tilesubline,CPS3_TRANSPARENCY_PEN_INDEX,0, 0x10000, 0x10000, nullptr, 0);
		}
	}
}

UINT32 cps3_state::screen_update_cps3(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int y,x, count;
	attoseconds_t period = screen.frame_period().attoseconds();
	rectangle visarea = screen.visible_area();

	int bg_drawn[4] = { 0, 0, 0, 0 };

	UINT32 fullscreenzoomx, fullscreenzoomy;
	UINT32 fszx, fszy;

//  decode_ssram();
//  decode_charram();

	/* registers are normally 002a006f 01ef01c6
	        widescreen mode = 00230076 026501c6
	  only SFIII2 uses widescreen, I don't know exactly which register controls it */
	if (((m_fullscreenzoom[1]&0xffff0000)>>16)==0x0265)
	{
		if (m_screenwidth!=496)
		{
			m_screenwidth = 496;
			visarea.set(0, 496-1, 0, 224-1);
			screen.configure(496, 224, visarea, period);
		}
	}
	else
	{
		if (m_screenwidth!=384)
		{
			m_screenwidth = 384;
			visarea.set(0, 384-1, 0, 224-1);
			screen.configure(384, 224, visarea, period);
		}
	}

	fullscreenzoomx = m_fullscreenzoom[3] & 0x000000ff;
	fullscreenzoomy = m_fullscreenzoom[3] & 0x000000ff;
	/* clamp at 0x80, I don't know if this is accurate */
	if (fullscreenzoomx>0x80) fullscreenzoomx = 0x80;
	if (fullscreenzoomy>0x80) fullscreenzoomy = 0x80;

	fszx = (fullscreenzoomx<<16)/0x40;
	fszy = (fullscreenzoomy<<16)/0x40;

	m_renderbuffer_clip.min_x = 0;
	m_renderbuffer_clip.max_x = ((m_screenwidth*fszx)>>16)-1;
	m_renderbuffer_clip.min_y = 0;
	m_renderbuffer_clip.max_y = ((224*fszx)>>16)-1;

	m_renderbuffer_bitmap.fill(0, m_renderbuffer_clip);

	/* Sprites */
	{
		int i;

		//printf("Spritelist start:\n");
		for (i=0x00000/4;i<0x2000/4;i+=4)
		{
			int xpos =      (m_spriteram[i+1]&0x03ff0000)>>16;
			int ypos =      m_spriteram[i+1]&0x000003ff;
			int j;
			int gscroll =      (m_spriteram[i+0]&0x70000000)>>28;
			int length =    (m_spriteram[i+0]&0x01ff0000)>>16; // how many entries in the sprite table
			UINT32 start  =    (m_spriteram[i+0]&0x00007ff0)>>4;

			int whichbpp =     (m_spriteram[i+2]&0x40000000)>>30; // not 100% sure if this is right, jojo title / characters
			int whichpal =     (m_spriteram[i+2]&0x20000000)>>29;
			int global_xflip = (m_spriteram[i+2]&0x10000000)>>28;
			int global_yflip = (m_spriteram[i+2]&0x08000000)>>27;
			int global_alpha = (m_spriteram[i+2]&0x04000000)>>26; // alpha / shadow? set on sfiii2 shadows, and big black image in jojo intro
			int global_bpp =   (m_spriteram[i+2]&0x02000000)>>25;
			int global_pal =   (m_spriteram[i+2]&0x01ff0000)>>16;

			int gscrollx = (m_unk_vidregs[gscroll]&0x03ff0000)>>16;
			int gscrolly = (m_unk_vidregs[gscroll]&0x000003ff)>>0;
			start = (start * 0x100) >> 2;

			if ((m_spriteram[i+0]&0xf0000000) == 0x80000000)
				break;

			for (j=0;j<(length)*4;j+=4)
			{
				UINT32 value1 =     (m_spriteram[start+j+0]);
				UINT32 value2 =     (m_spriteram[start+j+1]);
				UINT32 value3 =     (m_spriteram[start+j+2]);


				//UINT8* srcdata = (UINT8*)m_char_ram;
				//UINT32 sourceoffset = (value1 >>14)&0x7fffff;
				int count;

				UINT32 tileno = (value1&0xfffe0000)>>17;

				int xpos2 = (value2 & 0x03ff0000)>>16;
				int ypos2 = (value2 & 0x000003ff)>>0;
				int flipx = (value1 & 0x00001000)>>12;
				int flipy = (value1 & 0x00000800)>>11;
				int alpha = (value1 & 0x00000400)>>10; //? this one is used for alpha effects on warzard
				int bpp =   (value1 & 0x00000200)>>9;
				int pal =   (value1 & 0x000001ff);


				/* these are the sizes to actually draw */
				int ysizedraw2 = ((value3 & 0x7f000000)>>24);
				int xsizedraw2 = ((value3 & 0x007f0000)>>16);
				int xx,yy;

				static const int tilestable[4] = { 8,1,2,4 };
				int ysize2 = ((value3 & 0x0000000c)>>2);
				int xsize2 = ((value3 & 0x00000003)>>0);
				UINT32 xinc,yinc;

				if (ysize2==0)
				{
				//  printf("invalid sprite ysize of 0 tiles\n");
					continue;
				}

				if (xsize2==0) // xsize of 0 tiles seems to be a special command to draw tilemaps
				{
					int tilemapnum = ((value3 & 0x00000030)>>4);
					//int startline;// = value2 & 0x3ff;
					//int endline;
					//int height = (value3 & 0x7f000000)>>24;
					int uu;
//                  UINT32* tmapregs[4] = { m_tilemap20_regs_base, m_tilemap30_regs_base, m_tilemap40_regs_base, m_tilemap50_regs_base };
//                  UINT32* regs;
//                  regs = tmapregs[tilemapnum];
					//endline = value2;
					//startline = endline - height;

					//startline &=0x3ff;
					//endline &=0x3ff;

					//printf("tilemap draw %01x %02x %02x %02x\n",tilemapnum, value2, height, regs[0]&0x000003ff );

					//printf("tilemap draw %01x %d %d\n",tilemapnum, startline, endline );


					/* Urgh, the startline / endline seem to be direct screen co-ordinates regardless of fullscreen zoom
					   which probably means the fullscreen zoom is applied when rendering everything, not aftewards */
					//for (uu=startline;uu<endline+1;uu++)

					if (bg_drawn[tilemapnum]==0)
					{
						for (uu=0;uu<1023;uu++)
						{
							cps3_draw_tilemapsprite_line(tilemapnum, uu, m_renderbuffer_bitmap, m_renderbuffer_clip );
						}
					}
					bg_drawn[tilemapnum] = 1;
				}
				else
				{
					ysize2 = tilestable[ysize2];
					xsize2 = tilestable[xsize2];

					xinc = ((xsizedraw2+1)<<16) / ((xsize2*0x10));
					yinc = ((ysizedraw2+1)<<16) / ((ysize2*0x10));

					xsize2-=1;
					ysize2-=1;

					flipx ^= global_xflip;
					flipy ^= global_yflip;

					if (!flipx) xpos2+=((xsizedraw2+1)/2);
					else xpos2-=((xsizedraw2+1)/2);

					ypos2+=((ysizedraw2+1)/2);

					if (!flipx) xpos2-= ((xsize2+1)*16*xinc)>>16;
					else  xpos2+= (xsize2*16*xinc)>>16;

					if (flipy) ypos2-= (ysize2*16*yinc)>>16;

					{
						count = 0;
						for (xx=0;xx<xsize2+1;xx++)
						{
							int current_xpos;

							if (!flipx) current_xpos = (xpos+xpos2+((xx*16*xinc)>>16));
							else current_xpos = (xpos+xpos2-((xx*16*xinc)>>16));
							//current_xpos +=  rand()&0x3ff;
							current_xpos += gscrollx;
							current_xpos += 1;
							current_xpos &=0x3ff;
							if (current_xpos&0x200) current_xpos-=0x400;

							for (yy=0;yy<ysize2+1;yy++)
							{
								int current_ypos;
								int actualpal;

								if (flipy) current_ypos = (ypos+ypos2+((yy*16*yinc)>>16));
								else current_ypos = (ypos+ypos2-((yy*16*yinc)>>16));

								current_ypos += gscrolly;
								current_ypos = 0x3ff-current_ypos;
								current_ypos -= 17;
								current_ypos &=0x3ff;

								if (current_ypos&0x200) current_ypos-=0x400;

								//if ( (whichbpp) && (m_screen->frame_number() & 1)) continue;

								/* use the palette value from the main list or the sublists? */
								if (whichpal)
								{
									actualpal = global_pal;
								}
								else
								{
									actualpal = pal;
								}

								/* use the bpp value from the main list or the sublists? */
								if (whichbpp)
								{
									if (!global_bpp) m_gfxdecode->gfx(1)->set_granularity(256);
									else m_gfxdecode->gfx(1)->set_granularity(64);
								}
								else
								{
									if (!bpp) m_gfxdecode->gfx(1)->set_granularity(256);
									else m_gfxdecode->gfx(1)->set_granularity(64);
								}

								{
									int realtileno = tileno+count;

									if (global_alpha || alpha)
									{
										cps3_drawgfxzoom(m_renderbuffer_bitmap,m_renderbuffer_clip,m_gfxdecode->gfx(1),realtileno,actualpal,0^flipx,0^flipy,current_xpos,current_ypos,CPS3_TRANSPARENCY_PEN_INDEX_BLEND,0,xinc,yinc, nullptr, 0);
									}
									else
									{
										cps3_drawgfxzoom(m_renderbuffer_bitmap,m_renderbuffer_clip,m_gfxdecode->gfx(1),realtileno,actualpal,0^flipx,0^flipy,current_xpos,current_ypos,CPS3_TRANSPARENCY_PEN_INDEX,0,xinc,yinc, nullptr, 0);
									}
									count++;
								}
							}
						}
					}
	//              */

				//  printf("cell %08x %08x %08x\n",value1, value2, value3);
				}
			}
		}
	}

	/* copy render bitmap with zoom */
	{
		UINT32 renderx,rendery;
		UINT32 srcx, srcy;
		UINT32* srcbitmap;
		UINT32* dstbitmap;


		srcy=0;
		for (rendery=0;rendery<224;rendery++)
		{
			dstbitmap = &bitmap.pix32(rendery);
			srcbitmap = &m_renderbuffer_bitmap.pix32(srcy>>16);
			srcx=0;

			for (renderx=0;renderx<m_screenwidth;renderx++)
			{
				dstbitmap[renderx] = m_mame_colours[srcbitmap[srcx>>16]&0x1ffff];
				srcx += fszx;
			}

			srcy += fszy;
		}
	}

	/* Draw the text layer */
	/* Copy the first 0x800 colours to be used for fg layer rendering */
//  for (offset=0;offset<0x200;offset++)
//  {
//      int palreadbase = (m_ss_pal_base << 9);
//      m_palette->set_pen_color(offset,m_mame_colours[palreadbase+offset]);
//  }

	// fg layer
	{
		// bank select? (sfiii2 intro)
		if (m_ss_bank_base & 0x01000000) count = 0x000;
		else count = 0x800;

		for (y=0;y<32;y++)
		{
			for (x=0;x<64;x++)
			{
				UINT32 data = m_ss_ram[count]; // +0x800 = 2nd bank, used on sfiii2 intro..
				UINT32 tile = (data >> 16) & 0x1ff;
				int pal = (data&0x003f) >> 1;
				int flipx = (data & 0x0080) >> 7;
				int flipy = (data & 0x0040) >> 6;
				pal += m_ss_pal_base << 5;
				tile+=0x200;

				cps3_drawgfxzoom(bitmap, cliprect, m_gfxdecode->gfx(0),tile,pal,flipx,flipy,x*8,y*8,CPS3_TRANSPARENCY_PEN,0,0x10000,0x10000,nullptr,0);
				count++;
			}
		}
	}
	return 0;
}

READ32_MEMBER(cps3_state::cps3_ssram_r)
{
	if (offset>0x8000/4)
		return LITTLE_ENDIANIZE_INT32(m_ss_ram[offset]);
	else
		return m_ss_ram[offset];
}

WRITE32_MEMBER(cps3_state::cps3_ssram_w)
{
	if (offset>0x8000/4)
	{
		// we only want to endian-flip the character data, the tilemap info is fine
		data = LITTLE_ENDIANIZE_INT32(data);
		mem_mask = LITTLE_ENDIANIZE_INT32(mem_mask);
		m_gfxdecode->gfx(0)->mark_dirty(offset/16);
	}

	COMBINE_DATA(&m_ss_ram[offset]);
}

WRITE32_MEMBER(cps3_state::cps3_0xc0000000_ram_w)
{
	COMBINE_DATA( &m_0xc0000000_ram[offset] );
	// store a decrypted copy
	m_0xc0000000_ram_decrypted[offset] = m_0xc0000000_ram[offset]^cps3_mask(offset*4+0xc0000000, m_key1, m_key2);
}

WRITE32_MEMBER(cps3_state::cram_bank_w)
{
	if (ACCESSING_BITS_0_7)
	{
		// this seems to be related to accesses to the 0x04100000 region
		if (m_cram_bank != data)
		{
			m_cram_bank = data;
		//if(data&0xfffffff0)
		//bank_w 00000000, ffff0000
		//bank_w 00000001, ffff0000
		//bank_w 00000002, ffff0000
		//bank_w 00000003, ffff0000
		//bank_w 00000004, ffff0000
		//bank_w 00000005, ffff0000
		//bank_w 00000006, ffff0000
		//bank_w 00000007, ffff0000
		// during CHARACTER RAM test..
			if(DEBUG_PRINTF) printf("bank_w %08x, %08x\n",data,mem_mask);

		}
	}
	else
	{
		if(DEBUG_PRINTF) printf("bank_w LSB32 %08x, %08x\n",data,mem_mask);

	}
}

READ32_MEMBER(cps3_state::cram_data_r)
{
	UINT32 fulloffset = (((m_cram_bank&0x7)*0x100000)/4) + offset;

	return LITTLE_ENDIANIZE_INT32(m_char_ram[fulloffset]);
}

WRITE32_MEMBER(cps3_state::cram_data_w)
{
	UINT32 fulloffset = (((m_cram_bank&0x7)*0x100000)/4) + offset;
	mem_mask = LITTLE_ENDIANIZE_INT32(mem_mask);
	data = LITTLE_ENDIANIZE_INT32(data);
	COMBINE_DATA(&m_char_ram[fulloffset]);
	m_gfxdecode->gfx(1)->mark_dirty(fulloffset/0x40);
}

/* FLASH ROM ACCESS */

READ32_MEMBER(cps3_state::cps3_gfxflash_r)
{
	UINT32 result = 0;
	if (m_cram_gfxflash_bank&1) offset += 0x200000/4;

	fujitsu_29f016a_device *chip0 = m_simm[2 + m_cram_gfxflash_bank/8][(m_cram_gfxflash_bank % 8) & ~1];
	fujitsu_29f016a_device *chip1 = m_simm[2 + m_cram_gfxflash_bank/8][(m_cram_gfxflash_bank % 8) |  1];
	if (chip0 == nullptr || chip1 == nullptr)
		return 0xffffffff;

	if(DEBUG_PRINTF) printf("gfxflash_r\n");

	if (ACCESSING_BITS_24_31)   // GFX Flash 1
	{
		logerror("read GFX flash chip %s addr %02x\n", chip0->tag(), (offset<<1));
		result |= chip0->read( (offset<<1) ) << 24;
	}
	if (ACCESSING_BITS_16_23)   // GFX Flash 2
	{
		logerror("read GFX flash chip %s addr %02x\n", chip1->tag(), (offset<<1));
		result |= chip1->read( (offset<<1) ) << 16;
	}
	if (ACCESSING_BITS_8_15)    // GFX Flash 1
	{
		logerror("read GFX flash chip %s addr %02x\n", chip0->tag(), (offset<<1)+1);
		result |= chip0->read( (offset<<1)+0x1 ) << 8;
	}
	if (ACCESSING_BITS_0_7) // GFX Flash 2
	{
		logerror("read GFX flash chip %s addr %02x\n", chip1->tag(), (offset<<1)+1);
		result |= chip1->read( (offset<<1)+0x1 ) << 0;
	}

	//printf("read GFX flash chips addr %02x returning %08x mem_mask %08x crambank %08x gfxbank %08x\n", offset*2, result,mem_mask,  m_cram_bank, m_cram_gfxflash_bank  );

	return result;
}

WRITE32_MEMBER(cps3_state::cps3_gfxflash_w)
{
	int command;
	if (m_cram_gfxflash_bank&1) offset += 0x200000/4;

	fujitsu_29f016a_device *chip0 = m_simm[2 + m_cram_gfxflash_bank/8][(m_cram_gfxflash_bank % 8) & ~1];
	fujitsu_29f016a_device *chip1 = m_simm[2 + m_cram_gfxflash_bank/8][(m_cram_gfxflash_bank % 8) |  1];
	if (chip0 == nullptr || chip1 == nullptr)
		return;

//  if(DEBUG_PRINTF) printf("cps3_gfxflash_w %08x %08x %08x\n", offset *2, data, mem_mask);


	if (ACCESSING_BITS_24_31)   // GFX Flash 1
	{
		command = (data >> 24) & 0xff;
		logerror("write to GFX flash chip %s addr %02x cmd %02x\n", chip0->tag(), (offset<<1), command);
		chip0->write( (offset<<1), command);
	}
	if (ACCESSING_BITS_16_23)   // GFX Flash 2
	{
		command = (data >> 16) & 0xff;
		logerror("write to GFX flash chip %s addr %02x cmd %02x\n", chip1->tag(), (offset<<1), command);
		chip1->write( (offset<<1), command);
	}
	if (ACCESSING_BITS_8_15)    // GFX Flash 1
	{
		command = (data >> 8) & 0xff;
		logerror("write to GFX flash chip %s addr %02x cmd %02x\n", chip0->tag(), (offset<<1)+1, command);
		chip0->write( (offset<<1)+0x1, command);
	}
	if (ACCESSING_BITS_0_7) // GFX Flash 2
	{
		command = (data >> 0) & 0xff;
		//if ( ((offset<<1)+1) != 0x555) printf("write to GFX flash chip %s addr %02x cmd %02x\n", chip1->tag(), (offset<<1)+1, command);
		chip1->write( (offset<<1)+0x1, command);
	}

	/* make a copy in the linear memory region we actually use for drawing etc.  having it stored in interleaved flash roms isnt' very useful */
	{
		UINT32* romdata = (UINT32*)m_user5;
		int real_offset = 0;
		UINT32 newdata;

		real_offset = ((m_cram_gfxflash_bank&0x3e) * 0x200000) + offset*4;

		newdata =((chip0->read_raw(((offset*2)&0xfffffffe)+0)<<8) |
					(chip0->read_raw(((offset*2)&0xfffffffe)+1)<<24) |
					(chip1->read_raw(((offset*2)&0xfffffffe)+0)<<0)  |
					(chip1->read_raw(((offset*2)&0xfffffffe)+1)<<16));

//      printf("flashcrap %08x %08x %08x\n", offset *2, romdata[real_offset/4], newdata);
		romdata[real_offset/4] = newdata;
	}
}



UINT32 cps3_state::cps3_flashmain_r(address_space &space, int which, UINT32 offset, UINT32 mem_mask)
{
	UINT32 result = 0;

	if (m_simm[which][0] == nullptr || m_simm[which][1] == nullptr || m_simm[which][2] == nullptr || m_simm[which][3] == nullptr)
		return 0xffffffff;

	if (ACCESSING_BITS_24_31)   // Flash 1
	{
//      logerror("read flash chip %d addr %02x\n", base+0, offset*4 );
		result |= (m_simm[which][0]->read(offset)<<24);
	}
	if (ACCESSING_BITS_16_23)   // Flash 1
	{
//      logerror("read flash chip %d addr %02x\n", base+1, offset*4 );
		result |= (m_simm[which][1]->read(offset)<<16);
	}
	if (ACCESSING_BITS_8_15)    // Flash 1
	{
//      logerror("read flash chip %d addr %02x\n", base+2, offset*4 );
		result |= (m_simm[which][2]->read(offset)<<8);
	}
	if (ACCESSING_BITS_0_7) // Flash 1
	{
//      logerror("read flash chip %d addr %02x\n", base+3, offset*4 );
		result |= (m_simm[which][3]->read(offset)<<0);
	}

//  if (base==4) logerror("read flash chips addr %02x returning %08x\n", offset*4, result );

	return result;
}



READ32_MEMBER(cps3_state::cps3_flash1_r)
{
	UINT32 retvalue = cps3_flashmain_r(space, 0, offset,mem_mask);

	if (m_altEncryption) return retvalue;

	retvalue = retvalue ^ cps3_mask(0x6000000+offset*4, m_key1, m_key2);
	return retvalue;
}

READ32_MEMBER(cps3_state::cps3_flash2_r)
{
	UINT32 retvalue = cps3_flashmain_r(space, 1, offset,mem_mask);

	if (m_altEncryption) return retvalue;

	retvalue = retvalue ^ cps3_mask(0x6800000+offset*4, m_key1, m_key2);
	return retvalue;
}

void cps3_state::cps3_flashmain_w(int which, UINT32 offset, UINT32 data, UINT32 mem_mask)
{
	int command;

	if (m_simm[which][0] == nullptr || m_simm[which][1] == nullptr || m_simm[which][2] == nullptr || m_simm[which][3] == nullptr)
		return;

	if (ACCESSING_BITS_24_31)   // Flash 1
	{
		command = (data >> 24) & 0xff;
		logerror("write to flash chip %s addr %02x cmd %02x\n", m_simm[which][0]->tag(), offset, command);
		m_simm[which][0]->write(offset, command);
	}
	if (ACCESSING_BITS_16_23)   // Flash 2
	{
		command = (data >> 16) & 0xff;
		logerror("write to flash chip %s addr %02x cmd %02x\n", m_simm[which][1]->tag(), offset, command);
		m_simm[which][1]->write(offset, command);
	}
	if (ACCESSING_BITS_8_15)    // Flash 2
	{
		command = (data >> 8) & 0xff;
		logerror("write to flash chip %s addr %02x cmd %02x\n", m_simm[which][2]->tag(), offset, command);
		m_simm[which][2]->write(offset, command);
	}
	if (ACCESSING_BITS_0_7) // Flash 2
	{
		command = (data >> 0) & 0xff;
		logerror("write to flash chip %s addr %02x cmd %02x\n", m_simm[which][3]->tag(), offset, command);
		m_simm[which][3]->write(offset, command);
	}

	/* copy data into regions to execute from */
	{
		UINT32* romdata =  (UINT32*)m_user4;
		UINT32* romdata2 = (UINT32*)m_decrypted_gamerom;
		int real_offset = 0;
		UINT32 newdata;

		real_offset = offset * 4;

		if (which==1)
		{
			romdata+=0x800000/4;
			romdata2+=0x800000/4;
			real_offset += 0x800000;
		}

		newdata = (m_simm[which][0]->read_raw(offset)<<24) |
					(m_simm[which][1]->read_raw(offset)<<16) |
					(m_simm[which][2]->read_raw(offset)<<8) |
					(m_simm[which][3]->read_raw(offset)<<0);

		//printf("%08x %08x %08x %08x %08x\n",offset, romdata2[offset], romdata[offset], newdata,  newdata^cps3_mask(0x6000000+real_offset, m_key1, m_key2)  );

		romdata[offset] = newdata;
		romdata2[offset] = newdata^cps3_mask(0x6000000+real_offset, m_key1, m_key2);
	}
}

WRITE32_MEMBER(cps3_state::cps3_flash1_w)
{
	cps3_flashmain_w(0,offset,data,mem_mask);
}

WRITE32_MEMBER(cps3_state::cps3_flash2_w)
{
	cps3_flashmain_w(1,offset,data,mem_mask);
}

WRITE32_MEMBER(cps3_state::cram_gfxflash_bank_w)
{
	if (ACCESSING_BITS_24_31)
	{
		//printf("cram_gfxflash_bank_w MSB32 %08x\n",data);
/*
    SIMM 3 (Rom 30/31)
    cram_gfxflash_bank_w MSB32 00020000  // first half of gfx 0 + 1
    cram_gfxflash_bank_w MSB32 00030000  // 2nd half of gfx 0 + 1
    cram_gfxflash_bank_w MSB32 00040000  // first half of gfx 2 + 3
    cram_gfxflash_bank_w MSB32 00050000
    cram_gfxflash_bank_w MSB32 00060000  // first half of gfx 4 + 5
    cram_gfxflash_bank_w MSB32 00070000
    cram_gfxflash_bank_w MSB32 00080000  // first half of gfx 6 + 7
    cram_gfxflash_bank_w MSB32 00090000
    SIMM 4 (Rom 40/41)
    cram_gfxflash_bank_w MSB32 000a0000  // first half of gfx 8 + 9
    cram_gfxflash_bank_w MSB32 000b0000
    cram_gfxflash_bank_w MSB32 000c0000  // first half of gfx 10 + 11
    cram_gfxflash_bank_w MSB32 000d0000
    cram_gfxflash_bank_w MSB32 000e0000  // first half of gfx 12 + 13
    cram_gfxflash_bank_w MSB32 000f0000
    cram_gfxflash_bank_w MSB32 00100000  // first half of gfx 14 + 15
    cram_gfxflash_bank_w MSB32 00110000
    SIMM 5 (Rom 50/51)
    cram_gfxflash_bank_w MSB32 00120000  // first half of gfx 16 + 17
    cram_gfxflash_bank_w MSB32 00130000
    cram_gfxflash_bank_w MSB32 00140000  // first half of gfx 18 + 19
    cram_gfxflash_bank_w MSB32 00150000
    cram_gfxflash_bank_w MSB32 00160000  // first half of gfx 20 + 21
    cram_gfxflash_bank_w MSB32 00170000
    cram_gfxflash_bank_w MSB32 00180000  // first half of gfx 22 + 23
    cram_gfxflash_bank_w MSB32 00190000
    SIMM 6 (Rom 60/61)
    cram_gfxflash_bank_w MSB32 001a0000  // first half of gfx 24 + 25
    cram_gfxflash_bank_w MSB32 001b0000
    cram_gfxflash_bank_w MSB32 001c0000  // first half of gfx 26 + 27
    cram_gfxflash_bank_w MSB32 001d0000
    cram_gfxflash_bank_w MSB32 001e0000  // first half of gfx 28 + 29
    cram_gfxflash_bank_w MSB32 001f0000
    cram_gfxflash_bank_w MSB32 00200000  // first half of gfx 30 + 31
    cram_gfxflash_bank_w MSB32 00210000
    SIMM 7 (Rom 70/71) ** NOT USED (would follow on in sequence tho)

    */
		m_cram_gfxflash_bank = (data & 0xffff0000) >> 16;
		m_cram_gfxflash_bank-= 0x0002;// as with sound access etc. first 4 meg is 'special' and skipped
	}

	if (ACCESSING_BITS_0_7)
	{
		// writes 0 during boot
		//printf("cram_gfxflash_bank_LSB_w LSB32 %08x\n",data);
	}
}

// this seems to be dma active flags, and maybe vblank... not if it is anything else
READ32_MEMBER(cps3_state::cps3_vbl_r)
{
	return 0x00000000;
}

READ32_MEMBER(cps3_state::cps3_unk_io_r)
{
	//  warzard will crash before booting if you return anything here
	return 0xffffffff;
}

READ32_MEMBER(cps3_state::cps3_40C0000_r)
{
	return 0x00000000;
}

READ32_MEMBER(cps3_state::cps3_40C0004_r)
{
	return 0x00000000;
}

/* EEPROM access is a little odd, I think it accesses eeprom through some kind of
   additional interface, as these writes aren't normal for the type of eeprom we have */

READ32_MEMBER(cps3_state::cps3_eeprom_r)
{
	int addr = offset*4;

	if (addr>=0x100 && addr<=0x17f)
	{
		if (ACCESSING_BITS_24_31) m_current_eeprom_read = (m_eeprom[offset-0x100/4] & 0xffff0000)>>16;
		else m_current_eeprom_read = (m_eeprom[offset-0x100/4] & 0x0000ffff)>>0;
		// read word to latch...
		return 0x00000000;
	}
	else if (addr == 0x200)
	{
		// busy flag / read data..
		if (ACCESSING_BITS_24_31) return 0;
		else
		{
			//if(DEBUG_PRINTF) printf("reading %04x from eeprom\n", m_current_eeprom_read);
			return m_current_eeprom_read;
		}
	}
	else
	{
	//  if(DEBUG_PRINTF) printf("unk read eeprom addr %04x, mask %08x\n", addr, mem_mask);
		return 0x00000000;
	}
}

WRITE32_MEMBER(cps3_state::cps3_eeprom_w)
{
	int addr = offset*4;

	if (addr>=0x080 && addr<=0x0ff)
	{
		offset -= 0x80/4;
		COMBINE_DATA(&m_eeprom[offset]);
		// write word to storage

	}
	else if (addr>=0x180 && addr<=0x1ff)
	{
		// always 00000000 ? incrememnt access?
	}
	else
	{
	//  if(DEBUG_PRINTF) printf("unk write eeprom addr %04x, data %08x, mask %08x\n", addr, data, mem_mask);
	}

}

WRITE32_MEMBER(cps3_state::cps3_ss_bank_base_w)
{
	// might be scroll registers or something else..
	// used to display bank with 'insert coin' on during sfiii2 attract intro
	COMBINE_DATA(&m_ss_bank_base);

//  printf("cps3_ss_bank_base_w %08x %08x\n", data, mem_mask);
}

WRITE32_MEMBER(cps3_state::cps3_ss_pal_base_w)
{
		if(DEBUG_PRINTF) printf ("cps3_ss_pal_base_w %08x %08x\n", data, mem_mask);

	if(ACCESSING_BITS_24_31)
	{
		m_ss_pal_base = (data & 0x00ff0000)>>16;

		if (data & 0xff000000) printf("ss_pal_base MSB32 upper bits used %04x \n", data);
	}
	else
	{
	//  printf("ss_pal_base LSB32 used %04x \n", data);
	}
}

//<ElSemi> +0 X  +2 Y +4 unknown +6 enable (&0x8000) +8 low part tilemap base, high part linescroll base
//<ElSemi> (a word each)


WRITE32_MEMBER(cps3_state::cps3_palettedma_w)
{
	if (offset==0)
	{
		COMBINE_DATA(&m_paldma_source);
		m_paldma_realsource = (m_paldma_source<<1)-0x400000;
	}
	else if (offset==1)
	{
		COMBINE_DATA(&m_paldma_dest);
	}
	else if (offset==2)
	{
		COMBINE_DATA(&m_paldma_fade);
	}
	else if (offset==3)
	{
		COMBINE_DATA(&m_paldma_other2);

		if (ACCESSING_BITS_24_31)
		{
			m_paldma_length = (data & 0xffff0000)>>16;
		}
		if (ACCESSING_BITS_0_7)
		{
			if (data & 0x0002)
			{
				int i;
				UINT16* src = (UINT16*)m_user5;
			//  if(DEBUG_PRINTF) printf("CPS3 pal dma start %08x (real: %08x) dest %08x fade %08x other2 %08x (length %04x)\n", m_paldma_source, m_paldma_realsource, m_paldma_dest, m_paldma_fade, m_paldma_other2, m_paldma_length);

				for (i=0;i<m_paldma_length;i++)
				{
					UINT16 coldata = src[BYTE_XOR_BE(((m_paldma_realsource>>1)+i))];

					//if (m_paldma_fade!=0) printf("%08x\n",m_paldma_fade);

					cps3_set_mame_colours((m_paldma_dest+i)^1, coldata, m_paldma_fade);
				}


				m_maincpu->set_input_line(10, ASSERT_LINE);


			}
		}
	}

}


//static UINT8* current_table;




UINT32 cps3_state::process_byte( UINT8 real_byte, UINT32 destination, int max_length )
{
	UINT8* dest       = (UINT8*)m_char_ram.get();

	//printf("process byte for destination %08x\n", destination);

	destination&=0x7fffff;

	if (real_byte&0x40)
	{
		int tranfercount = 0;

		//printf("Set RLE Mode\n");
		m_rle_length = (real_byte&0x3f)+1;

		//printf("RLE Operation (length %08x\n", m_rle_length );

		while (m_rle_length)
		{
			dest[((destination+tranfercount)&0x7fffff)^3] = (m_last_normal_byte&0x3f);
			m_gfxdecode->gfx(1)->mark_dirty(((destination+tranfercount)&0x7fffff)/0x100);
			//printf("RLE WRite Byte %08x, %02x\n", destination+tranfercount, real_byte);

			tranfercount++;
			m_rle_length--;
			max_length--;

			if ((destination+tranfercount) > 0x7fffff)  return max_length;


	//      if (max_length==0) return max_length; // this is meant to abort the transfer if we exceed dest length,, not working
		}
		return tranfercount;
	}
	else
	{
		//printf("Write Normal Data\n");
		dest[(destination&0x7fffff)^3] = real_byte;
		m_last_normal_byte = real_byte;
		m_gfxdecode->gfx(1)->mark_dirty((destination&0x7fffff)/0x100);
		return 1;
	}
}

void cps3_state::cps3_do_char_dma( UINT32 real_source, UINT32 real_destination, UINT32 real_length )
{
	UINT8* sourcedata = (UINT8*)m_user5;
	int length_remaining;

	m_last_normal_byte = 0;
	m_rle_length = 0;
	length_remaining = real_length;
	while (length_remaining)
	{
		UINT8 current_byte;

		current_byte = sourcedata[DMA_XOR(real_source)];
		real_source++;

		if (current_byte & 0x80)
		{
			UINT8 real_byte;
			UINT32 length_processed;
			current_byte &= 0x7f;

			real_byte = sourcedata[DMA_XOR((m_current_table_address+current_byte*2+0))];
			//if (real_byte&0x80) return;
			length_processed = process_byte(real_byte, real_destination, length_remaining );
			length_remaining-=length_processed; // subtract the number of bytes the operation has taken
			real_destination+=length_processed; // add it onto the destination
			if (real_destination>0x7fffff) return;
			if (length_remaining<=0) return; // if we've expired, exit

			real_byte = sourcedata[DMA_XOR((m_current_table_address+current_byte*2+1))];
			//if (real_byte&0x80) return;
			length_processed = process_byte(real_byte, real_destination, length_remaining );
			length_remaining-=length_processed; // subtract the number of bytes the operation has taken
			real_destination+=length_processed; // add it onto the destination
			if (real_destination>0x7fffff) return;
			if (length_remaining<=0) return;  // if we've expired, exit
		}
		else
		{
			UINT32 length_processed;
			length_processed = process_byte(current_byte, real_destination, length_remaining );
			length_remaining-=length_processed; // subtract the number of bytes the operation has taken
			real_destination+=length_processed; // add it onto the destination
			if (real_destination>0x7fffff) return;
			if (length_remaining<=0) return;  // if we've expired, exit
		}

//      length_remaining--;
	}
}

UINT32 cps3_state::ProcessByte8(UINT8 b,UINT32 dst_offset)
{
	UINT8* destRAM = (UINT8*)m_char_ram.get();
	int l=0;

	if(m_lastb==m_lastb2) //rle
	{
		int i;
		int rle=(b+1)&0xff;

		for(i=0;i<rle;++i)
		{
			destRAM[(dst_offset&0x7fffff)^3] = m_lastb;
			m_gfxdecode->gfx(1)->mark_dirty((dst_offset&0x7fffff)/0x100);

			dst_offset++;
			++l;
		}
		m_lastb2=0xffff;

		return l;
	}
	else
	{
		m_lastb2=m_lastb;
		m_lastb=b;
		destRAM[(dst_offset&0x7fffff)^3] = b;
		m_gfxdecode->gfx(1)->mark_dirty((dst_offset&0x7fffff)/0x100);
		return 1;
	}
}

void cps3_state::cps3_do_alt_char_dma( UINT32 src, UINT32 real_dest, UINT32 real_length )
{
	UINT8* px = (UINT8*)m_user5;
	UINT32 start = real_dest;
	UINT32 ds = real_dest;

	m_lastb=0xfffe;
	m_lastb2=0xffff;

	while(1)
	{
		int i;
		UINT8 ctrl=px[DMA_XOR(src)];
		++src;

		for(i=0;i<8;++i)
		{
			UINT8 p=px[DMA_XOR(src)];

			if(ctrl&0x80)
			{
				UINT8 real_byte;
				p&=0x7f;
				real_byte = px[DMA_XOR((m_current_table_address+p*2+0))];
				ds+=ProcessByte8(real_byte,ds);
				real_byte = px[DMA_XOR((m_current_table_address+p*2+1))];
				ds+=ProcessByte8(real_byte,ds);
			}
			else
			{
				ds+=ProcessByte8(p,ds);
			}
			++src;
			ctrl<<=1;

			if((ds-start)>=real_length)
				return;
		}
	}
}

void cps3_state::cps3_process_character_dma(UINT32 address)
{
	int i;

	//printf("charDMA start:\n");

	for (i = 0; i < 0x1000; i += 3)
	{
		UINT32 dat1 = LITTLE_ENDIANIZE_INT32(m_char_ram[i + 0 + (address)]);
		UINT32 dat2 = LITTLE_ENDIANIZE_INT32(m_char_ram[i + 1 + (address)]);
		UINT32 dat3 = LITTLE_ENDIANIZE_INT32(m_char_ram[i + 2 + (address)]);
		UINT32 real_source      = (dat3 << 1) - 0x400000;
		UINT32 real_destination =  dat2 << 3;
		UINT32 real_length      = (((dat1 & 0x001fffff) + 1) << 3);

		/* 0x01000000 is the end of list marker, 0x13131313 is our default fill */
		if ((dat1 == 0x01000000) || (dat1 == 0x13131313)) break;

		//printf("%08x %08x %08x real_source %08x (rom %d offset %08x) real_destination %08x, real_length %08x\n", dat1, dat2, dat3, real_source, real_source/0x800000, real_source%0x800000, real_destination, real_length);

		if  ((dat1 & 0x00e00000) == 0x00800000)
		{
			/* Sets a table used by the decompression routines */
			{
				/* We should probably copy this, but a pointer to it is fine for our purposes as the data doesn't change */
				m_current_table_address = real_source;
			}
			m_maincpu->set_input_line(10, ASSERT_LINE);
		}
		else if  ((dat1 & 0x00e00000) == 0x00400000)
		{
			/* 6bpp DMA decompression
			  - this is used for the majority of sprites and backgrounds */
			cps3_do_char_dma(real_source, real_destination, real_length );
			m_maincpu->set_input_line(10, ASSERT_LINE);

		}
		else if  ((dat1 & 0x00e00000) == 0x00600000)
		{
			/* 8bpp DMA decompression
			  - this is used on SFIII NG Sean's Stage ONLY */
			cps3_do_alt_char_dma(real_source, real_destination, real_length);
			m_maincpu->set_input_line(10, ASSERT_LINE);
		}
		else
		{
			// warzard uses command 0, uncompressed? but for what?
			//printf("Unknown DMA List Command Type\n");
		}

	}
}

WRITE32_MEMBER(cps3_state::cps3_characterdma_w)
{
	if(DEBUG_PRINTF) printf("chardma_w %08x %08x %08x\n", offset, data, mem_mask);

	if (offset==0)
	{
		//COMBINE_DATA(&m_chardma_source);
		if (ACCESSING_BITS_0_7)
		{
			m_chardma_source = data & 0x0000ffff;
		}
		if (ACCESSING_BITS_24_31)
		{
			if(DEBUG_PRINTF) printf("chardma_w accessing MSB32 of offset 0");
		}
	}
	else if (offset==1)
	{
		COMBINE_DATA(&m_chardma_other);

		if (ACCESSING_BITS_24_31)
		{
			if ((data>>16) & 0x0040)
			{
				UINT32 list_address;
				list_address = (m_chardma_source | ((m_chardma_other&0x003f0000)));

				//printf("chardma_w activated %08x %08x (address = cram %08x)\n", m_chardma_source, m_chardma_other, list_address*4 );
				cps3_process_character_dma(list_address);
			}
			else
			{
				if(DEBUG_PRINTF) printf("chardma_w NOT activated %08x %08x\n", m_chardma_source, m_chardma_other );
			}

			if ((data>>16) & 0xff80)
				if(DEBUG_PRINTF) printf("chardma_w unknown bits in activate command %08x %08x\n", m_chardma_source, m_chardma_other );
		}
		else
		{
			if(DEBUG_PRINTF) printf("chardma_w LSB32 write to activate command %08x %08x\n", m_chardma_source, m_chardma_other );
		}
	}
}

WRITE32_MEMBER(cps3_state::cps3_irq10_ack_w)
{
	m_maincpu->set_input_line(10, CLEAR_LINE); return;
}

WRITE32_MEMBER(cps3_state::cps3_irq12_ack_w)
{
	m_maincpu->set_input_line(12, CLEAR_LINE); return;
}

WRITE32_MEMBER(cps3_state::cps3_unk_vidregs_w)
{
	COMBINE_DATA(&m_unk_vidregs[offset]);
}

READ32_MEMBER(cps3_state::cps3_colourram_r)
{
	UINT16* src = (UINT16*)m_colourram.target();

	return src[offset*2+1] | (src[offset*2+0]<<16);
}

WRITE32_MEMBER(cps3_state::cps3_colourram_w)
{
//  COMBINE_DATA(&m_colourram[offset]);

	if (ACCESSING_BITS_24_31)
	{
		cps3_set_mame_colours(offset*2, (data & 0xffff0000) >> 16, 0);
	}

	if (ACCESSING_BITS_0_7)
	{
		cps3_set_mame_colours(offset*2+1, (data & 0x0000ffff) >> 0, 0);
	}
}


/* there are more unknown writes, but you get the idea */
static ADDRESS_MAP_START( cps3_map, AS_PROGRAM, 32, cps3_state )
	AM_RANGE(0x00000000, 0x0007ffff) AM_ROM AM_REGION("bios", 0) // Bios ROM
	AM_RANGE(0x02000000, 0x0207ffff) AM_RAM AM_SHARE("mainram") // Main RAM

	AM_RANGE(0x03000000, 0x030003ff) AM_RAM // 'FRAM' (SFIII memory test mode ONLY)

//  AM_RANGE(0x04000000, 0x0407dfff) AM_RAM AM_SHARE("spriteram")//AM_WRITEONLY // Sprite RAM (jojoba tests this size)
	AM_RANGE(0x04000000, 0x0407ffff) AM_RAM AM_SHARE("spriteram")//AM_WRITEONLY // Sprite RAM

	AM_RANGE(0x04080000, 0x040bffff) AM_READWRITE(cps3_colourram_r, cps3_colourram_w) AM_SHARE("colourram")  // Colour RAM (jojoba tests this size) 0x20000 colours?!

	// video registers of some kind probably
	AM_RANGE(0x040C0000, 0x040C0003) AM_READ(cps3_40C0000_r)//?? every frame
	AM_RANGE(0x040C0004, 0x040C0007) AM_READ(cps3_40C0004_r)//AM_READ(cps3_40C0004_r) // warzard reads this!
//  AM_RANGE(0x040C0008, 0x040C000b) AM_WRITENOP//??
	AM_RANGE(0x040C000c, 0x040C000f) AM_READ(cps3_vbl_r)// AM_WRITENOP/

	AM_RANGE(0x040C0000, 0x040C001f) AM_WRITE(cps3_unk_vidregs_w)
	AM_RANGE(0x040C0020, 0x040C002b) AM_WRITEONLY AM_SHARE("tmap20_regs")
	AM_RANGE(0x040C0030, 0x040C003b) AM_WRITEONLY AM_SHARE("tmap30_regs")
	AM_RANGE(0x040C0040, 0x040C004b) AM_WRITEONLY AM_SHARE("tmap40_regs")
	AM_RANGE(0x040C0050, 0x040C005b) AM_WRITEONLY AM_SHARE("tmap50_regs")

	AM_RANGE(0x040C0060, 0x040C007f) AM_RAM AM_SHARE("fullscreenzoom")


	AM_RANGE(0x040C0094, 0x040C009b) AM_WRITE(cps3_characterdma_w)


	AM_RANGE(0x040C00a0, 0x040C00af) AM_WRITE(cps3_palettedma_w)


	AM_RANGE(0x040C0084, 0x040C0087) AM_WRITE(cram_bank_w)
	AM_RANGE(0x040C0088, 0x040C008b) AM_WRITE(cram_gfxflash_bank_w)

	AM_RANGE(0x040e0000, 0x040e02ff) AM_DEVREADWRITE("cps3sound", cps3_sound_device, cps3_sound_r, cps3_sound_w)

	AM_RANGE(0x04100000, 0x041fffff) AM_READWRITE(cram_data_r, cram_data_w)
	AM_RANGE(0x04200000, 0x043fffff) AM_READWRITE(cps3_gfxflash_r, cps3_gfxflash_w) // GFX Flash ROMS

	AM_RANGE(0x05000000, 0x05000003) AM_READ_PORT("INPUTS")
	AM_RANGE(0x05000004, 0x05000007) AM_READ_PORT("EXTRA")

	AM_RANGE(0x05000008, 0x0500000b) AM_WRITENOP // ?? every frame

	AM_RANGE(0x05000a00, 0x05000a1f) AM_READ(cps3_unk_io_r ) // ?? every frame

	AM_RANGE(0x05001000, 0x05001203) AM_READWRITE(cps3_eeprom_r, cps3_eeprom_w )

	AM_RANGE(0x05040000, 0x0504ffff) AM_READWRITE(cps3_ssram_r,cps3_ssram_w) // 'SS' RAM (Score Screen) (text tilemap + toles)
	//0x25050020
	AM_RANGE(0x05050020, 0x05050023) AM_WRITE(cps3_ss_bank_base_w )
	AM_RANGE(0x05050024, 0x05050027) AM_WRITE(cps3_ss_pal_base_w )

	AM_RANGE(0x05100000, 0x05100003) AM_WRITE(cps3_irq12_ack_w )
	AM_RANGE(0x05110000, 0x05110003) AM_WRITE(cps3_irq10_ack_w )

	AM_RANGE(0x05140000, 0x05140003) AM_DEVREADWRITE8("wd33c93", wd33c93_device, read, write, 0x00ff00ff )

	AM_RANGE(0x06000000, 0x067fffff) AM_READWRITE(cps3_flash1_r, cps3_flash1_w ) /* Flash ROMs simm 1 */
	AM_RANGE(0x06800000, 0x06ffffff) AM_READWRITE(cps3_flash2_r, cps3_flash2_w ) /* Flash ROMs simm 2 */

	AM_RANGE(0xc0000000, 0xc00003ff) AM_RAM_WRITE(cps3_0xc0000000_ram_w ) AM_SHARE("0xc0000000_ram") /* Executes code from here */
ADDRESS_MAP_END

static ADDRESS_MAP_START( decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 32, cps3_state )
	AM_RANGE(0x00000000, 0x0007ffff) AM_ROM AM_REGION("bios", 0) // Bios ROM
	AM_RANGE(0x06000000, 0x06ffffff) AM_ROM AM_SHARE("decrypted_gamerom")
	AM_RANGE(0xc0000000, 0xc00003ff) AM_ROM AM_SHARE("0xc0000000_ram_decrypted")
ADDRESS_MAP_END

static INPUT_PORTS_START( cps3 )
	PORT_START("INPUTS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Jab Punch") PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Strong Punch") PORT_PLAYER(1)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Fierce Punch") PORT_PLAYER(1)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED ) // nothing here?
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Jab Punch") PORT_PLAYER(2)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Strong Punch") PORT_PLAYER(2)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2 Fierce Punch") PORT_PLAYER(2)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNUSED ) // nothing here?
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x00020000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00fc0000, IP_ACTIVE_LOW, IPT_UNUSED ) // nothing here?
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P2 Roundhouse Kick") PORT_PLAYER(2)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNUSED ) // nothing here?
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0000000, IP_ACTIVE_LOW, IPT_UNUSED ) // nothing here?

	PORT_START("EXTRA")
	PORT_BIT( 0x0001ffff, IP_ACTIVE_LOW, IPT_UNUSED ) // nothing here?
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P1 Roundhouse Kick") PORT_PLAYER(1)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P1 Forward Kick") PORT_PLAYER(1)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 Short Kick") PORT_PLAYER(1)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P2 Short Kick") PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P2 Forward Kick") PORT_PLAYER(2)
	PORT_BIT( 0xffc00000, IP_ACTIVE_LOW, IPT_UNUSED ) // nothing here?
INPUT_PORTS_END

/* Red Earth game inputs */
static INPUT_PORTS_START ( cps3_re )
	PORT_INCLUDE ( cps3 )

	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("P1 Start / Change Orb")
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("P2 Start / Change Orb")
INPUT_PORTS_END

/* Jojo game inputs */
static INPUT_PORTS_START( cps3_jojo)
	PORT_INCLUDE( cps3 )

	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Light") PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Medium") PORT_PLAYER(1)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Strong") PORT_PLAYER(1)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Light") PORT_PLAYER(2)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Medium") PORT_PLAYER(2)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2 Strong") PORT_PLAYER(2)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(2)

	PORT_MODIFY("EXTRA")
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(1)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(1)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 Stand") PORT_PLAYER(1)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P2 Stand") PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(2)
INPUT_PORTS_END

INTERRUPT_GEN_MEMBER(cps3_state::cps3_vbl_interrupt)
{
	device.execute().set_input_line(12, ASSERT_LINE);
}

INTERRUPT_GEN_MEMBER(cps3_state::cps3_other_interrupt)
{
	// this seems to need to be periodic (see the life bar portraits in sfiii2
	// but also triggered on certain dma events (or warzard locks up in attract)
	// what is the REAL source of IRQ10??
	device.execute().set_input_line(10, ASSERT_LINE);
}


void cps3_state::machine_reset()
{
	m_current_table_address = -1;

	// copy data from flashroms back into user regions + decrypt into regions we execute/draw from.
	copy_from_nvram();
}



// make a copy in the regions we execute code / draw gfx from
void cps3_state::copy_from_nvram()
{
	UINT32* romdata = (UINT32*)m_user4;
	UINT32* romdata2 = (UINT32*)m_decrypted_gamerom;
	int i;
	/* copy + decrypt program roms which have been loaded from flashroms/nvram */
	for (i=0;i<0x800000;i+=4)
	{
		UINT32 data;

		data = ((m_simm[0][0]->read_raw(i/4)<<24) | (m_simm[0][1]->read_raw(i/4)<<16) | (m_simm[0][2]->read_raw(i/4)<<8) | (m_simm[0][3]->read_raw(i/4)<<0));

	//  printf("%08x %08x %08x %08x\n",romdata[i/4],data, romdata2[i/4], data ^ cps3_mask(i+0x6000000, m_key1, m_key2));
		romdata[i/4] = data;
		romdata2[i/4] = data ^ cps3_mask(i+0x6000000, m_key1, m_key2);

	}

	romdata  += 0x800000/4;
	romdata2 += 0x800000/4;

	if (m_simm[1][0] != nullptr)
		for (i=0;i<0x800000;i+=4)
		{
			UINT32 data;

			data = ((m_simm[1][0]->read_raw(i/4)<<24) | (m_simm[1][1]->read_raw(i/4)<<16) | (m_simm[1][2]->read_raw(i/4)<<8) | (m_simm[1][3]->read_raw(i/4)<<0));

		//  printf("%08x %08x %08x %08x\n",romdata[i/4],data, romdata2[i/4],  data ^ cps3_mask(i+0x6800000, m_key1, m_key2) );
			romdata[i/4] = data;
			romdata2[i/4] = data ^ cps3_mask(i+0x6800000, m_key1, m_key2);
		}

	/* copy gfx from loaded flashroms to user reigon 5, where it's used */
	{
		UINT32 thebase, len = USER5REGION_LENGTH;
		int flashnum = 0;
		int countoffset = 0;

		romdata = (UINT32*)m_user5;
		for (thebase = 0;thebase < len/2; thebase+=0x200000)
		{
		//  printf("flashnums %d. %d\n",flashnum, flashnum+1);

			fujitsu_29f016a_device *flash0 = m_simm[2 + flashnum/8][flashnum % 8 + 0];
			fujitsu_29f016a_device *flash1 = m_simm[2 + flashnum/8][flashnum % 8 + 1];
			if (flash0 == nullptr || flash1 == nullptr)
				continue;
			if (flash0 != nullptr && flash1 != nullptr)
			{
				for (i=0;i<0x200000;i+=2)
				{
					UINT32 dat = (flash0->read_raw(i+0)<<8) |
									(flash0->read_raw(i+1)<<24) |
									(flash1->read_raw(i+0)<<0) |
									(flash1->read_raw(i+1)<<16);

					//printf("%08x %08x\n",romdata[countoffset],dat);
					romdata[countoffset] = dat;

					countoffset++;
				}
			}
			flashnum+=2;
		}
	}


	/*
	{
	    FILE *fp;
	    const char *gamename = machine().system().name;
	    char filename[256];
	    sprintf(filename, "%s_bios.dump", gamename);

	    fp=fopen(filename, "w+b");
	    if (fp)
	    {
	        fwrite(rom, 0x080000, 1, fp);
	        fclose(fp);
	    }
	}
	*/

}


SH2_DMA_KLUDGE_CB(cps3_state::dma_callback)
{
	/*
	  on the actual CPS3 hardware the SH2 DMA bypasses the encryption.

	  to handle this in MAME we use this callback, and reverse the effect of the
	  encryption that would otherwise be applied.  this allows us to avoid per-game,
	  per-PC hacks.  this approach is however still a little messy.

	*/

	/* I doubt this is endian safe.. needs checking / fixing */
	if (size==0)
	{
		if ((src&3)==0) data <<=24;
		if ((src&3)==1) data <<=16;
		if ((src&3)==2) data <<=8;
		if ((src&3)==3) data <<=0;
	}


	if (src<0x80000)
	{
		int offs = (src & 0x07ffff) >> 2;
		data = data ^ cps3_mask(offs * 4, m_key1, m_key2);
	}
	else if (src>=0x6000000 && src<0x6800000)
	{
		int offs = (src & 0x07fffff) >> 2;
		if (!m_altEncryption) data = data ^ cps3_mask(0x6000000 + offs * 4, m_key1, m_key2);
	}
	else if (src>=0x6800000 && src<0x7000000)
	{
		int offs = (src & 0x07fffff) >> 2;
		if (!m_altEncryption) data = data ^ cps3_mask(0x6800000 + offs * 4, m_key1, m_key2);
	}
	else
	{
		//printf("%s :src %08x, dst %08x, returning %08x\n", machine().describe_context(), src, dst, data);
	}

	/* I doubt this is endian safe.. needs checking / fixing */
	if (size==0)
	{
		if ((src&3)==0) data >>=24;
		if ((src&3)==1) data >>=16;
		if ((src&3)==2) data >>=8;
		if ((src&3)==3) data >>=0;

		data &=0x000000ff;
	}

	return data;
}


static MACHINE_CONFIG_FRAGMENT( simm1_64mbit )
	MCFG_FUJITSU_29F016A_ADD("simm1.0")
	MCFG_FUJITSU_29F016A_ADD("simm1.1")
	MCFG_FUJITSU_29F016A_ADD("simm1.2")
	MCFG_FUJITSU_29F016A_ADD("simm1.3")
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( simm2_64mbit )
	MCFG_FUJITSU_29F016A_ADD("simm2.0")
	MCFG_FUJITSU_29F016A_ADD("simm2.1")
	MCFG_FUJITSU_29F016A_ADD("simm2.2")
	MCFG_FUJITSU_29F016A_ADD("simm2.3")
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( simm3_128mbit )
	MCFG_FUJITSU_29F016A_ADD("simm3.0")
	MCFG_FUJITSU_29F016A_ADD("simm3.1")
	MCFG_FUJITSU_29F016A_ADD("simm3.2")
	MCFG_FUJITSU_29F016A_ADD("simm3.3")
	MCFG_FUJITSU_29F016A_ADD("simm3.4")
	MCFG_FUJITSU_29F016A_ADD("simm3.5")
	MCFG_FUJITSU_29F016A_ADD("simm3.6")
	MCFG_FUJITSU_29F016A_ADD("simm3.7")
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( simm4_128mbit )
	MCFG_FUJITSU_29F016A_ADD("simm4.0")
	MCFG_FUJITSU_29F016A_ADD("simm4.1")
	MCFG_FUJITSU_29F016A_ADD("simm4.2")
	MCFG_FUJITSU_29F016A_ADD("simm4.3")
	MCFG_FUJITSU_29F016A_ADD("simm4.4")
	MCFG_FUJITSU_29F016A_ADD("simm4.5")
	MCFG_FUJITSU_29F016A_ADD("simm4.6")
	MCFG_FUJITSU_29F016A_ADD("simm4.7")
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( simm5_128mbit )
	MCFG_FUJITSU_29F016A_ADD("simm5.0")
	MCFG_FUJITSU_29F016A_ADD("simm5.1")
	MCFG_FUJITSU_29F016A_ADD("simm5.2")
	MCFG_FUJITSU_29F016A_ADD("simm5.3")
	MCFG_FUJITSU_29F016A_ADD("simm5.4")
	MCFG_FUJITSU_29F016A_ADD("simm5.5")
	MCFG_FUJITSU_29F016A_ADD("simm5.6")
	MCFG_FUJITSU_29F016A_ADD("simm5.7")
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( simm5_32mbit )
	MCFG_FUJITSU_29F016A_ADD("simm5.0")
	MCFG_FUJITSU_29F016A_ADD("simm5.1")
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( simm6_128mbit )
	MCFG_FUJITSU_29F016A_ADD("simm6.0")
	MCFG_FUJITSU_29F016A_ADD("simm6.1")
	MCFG_FUJITSU_29F016A_ADD("simm6.2")
	MCFG_FUJITSU_29F016A_ADD("simm6.3")
	MCFG_FUJITSU_29F016A_ADD("simm6.4")
	MCFG_FUJITSU_29F016A_ADD("simm6.5")
	MCFG_FUJITSU_29F016A_ADD("simm6.6")
	MCFG_FUJITSU_29F016A_ADD("simm6.7")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( cps3, cps3_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SH2, 6250000*4) // external clock is 6.25 Mhz, it sets the internal multiplier to 4x (this should probably be handled in the core..)
	MCFG_CPU_PROGRAM_MAP(cps3_map)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cps3_state,  cps3_vbl_interrupt)
	MCFG_CPU_PERIODIC_INT_DRIVER(cps3_state, cps3_other_interrupt, 80) /* ?source? */
	MCFG_SH2_DMA_KLUDGE_CB(cps3_state, dma_callback)

	MCFG_DEVICE_ADD("scsi", SCSI_PORT, 0)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE1, "cdrom", SCSICD, SCSI_ID_1)

	MCFG_DEVICE_ADD("wd33c93", WD33C93, 0)
	MCFG_LEGACY_SCSI_PORT("scsi")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_60MHz/8, 486, 0, 384, 259, 0, 224)
	MCFG_SCREEN_UPDATE_DRIVER(cps3_state, screen_update_cps3)
/*
    Measured clocks:
        V = 59.5992Hz
        H = 15.4335kHz
        H/V = 258.955 ~ 259 lines

    Possible video clocks:
        60MHz       / 15.4335kHz = 3887.647 / 8 = 485.956 ~ 486 -> likely
         42.9545MHz / 15.4445kHz = 2781.217 / 6 = 463.536 -> unlikely
*/

	MCFG_NVRAM_ADD_0FILL("eeprom")
	MCFG_PALETTE_ADD("palette", 0x10000) // actually 0x20000 ...

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", empty)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("cps3sound", CPS3, MASTER_CLOCK / 3)
	MCFG_SOUND_ROUTE(1, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(0, "rspeaker", 1.0)
MACHINE_CONFIG_END


/* individual configs for each machine, depending on the SIMMs installed */
MACHINE_CONFIG_DERIVED( redearth, cps3 )
	MCFG_FRAGMENT_ADD(simm1_64mbit)
	MCFG_FRAGMENT_ADD(simm3_128mbit)
	MCFG_FRAGMENT_ADD(simm4_128mbit)
	MCFG_FRAGMENT_ADD(simm5_32mbit)
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( sfiii, cps3 )
	MCFG_FRAGMENT_ADD(simm1_64mbit)
	MCFG_FRAGMENT_ADD(simm3_128mbit)
	MCFG_FRAGMENT_ADD(simm4_128mbit)
	MCFG_FRAGMENT_ADD(simm5_32mbit)
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( sfiii2, cps3 )
	MCFG_FRAGMENT_ADD(simm1_64mbit)
	MCFG_FRAGMENT_ADD(simm2_64mbit)
	MCFG_FRAGMENT_ADD(simm3_128mbit)
	MCFG_FRAGMENT_ADD(simm4_128mbit)
	MCFG_FRAGMENT_ADD(simm5_128mbit)
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( jojo, cps3 )
	MCFG_FRAGMENT_ADD(simm1_64mbit)
	MCFG_FRAGMENT_ADD(simm2_64mbit)
	MCFG_FRAGMENT_ADD(simm3_128mbit)
	MCFG_FRAGMENT_ADD(simm4_128mbit)
	MCFG_FRAGMENT_ADD(simm5_32mbit)
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( sfiii3, cps3 )
	MCFG_FRAGMENT_ADD(simm1_64mbit)
	MCFG_FRAGMENT_ADD(simm2_64mbit)
	MCFG_FRAGMENT_ADD(simm3_128mbit)
	MCFG_FRAGMENT_ADD(simm4_128mbit)
	MCFG_FRAGMENT_ADD(simm5_128mbit)
	MCFG_FRAGMENT_ADD(simm6_128mbit)
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( jojoba, cps3 )
	MCFG_FRAGMENT_ADD(simm1_64mbit)
	MCFG_FRAGMENT_ADD(simm2_64mbit)
	MCFG_FRAGMENT_ADD(simm3_128mbit)
	MCFG_FRAGMENT_ADD(simm4_128mbit)
	MCFG_FRAGMENT_ADD(simm5_128mbit)
MACHINE_CONFIG_END


/* CD sets - use CD BIOS roms */

ROM_START( redearth )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "redearth_euro.29f400.u2", 0x000000, 0x080000, CRC(02e0f336) SHA1(acc37e830dfeb9674f5a0fb24f4cc23217ae4ff5) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "cap-wzd-5", 0, BAD_DUMP SHA1(e5676752b08283dc4a98c3d7b759e8aa6dcd0679) )
ROM_END

ROM_START( redearthr1 )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "redearth_euro.29f400.u2", 0x000000, 0x080000, CRC(02e0f336) SHA1(acc37e830dfeb9674f5a0fb24f4cc23217ae4ff5) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "cap-wzd-3", 0, SHA1(a6ff67093db6bc80ee5fc46e4300e0177b213a52) )
ROM_END

ROM_START( warzard )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "warzard_japan.29f400.u2", 0x000000, 0x080000, CRC(f8e2f0c6) SHA1(93d6a986f44c211fff014e55681eca4d2a2774d6) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "cap-wzd-5", 0, BAD_DUMP SHA1(e5676752b08283dc4a98c3d7b759e8aa6dcd0679) )
ROM_END

ROM_START( warzardr1 )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "warzard_japan.29f400.u2", 0x000000, 0x080000, CRC(f8e2f0c6) SHA1(93d6a986f44c211fff014e55681eca4d2a2774d6) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "cap-wzd-3", 0, SHA1(a6ff67093db6bc80ee5fc46e4300e0177b213a52) )
ROM_END


ROM_START( sfiii )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "sfiii_euro.29f400.u2", 0x000000, 0x080000, CRC(27699ddc) SHA1(d8b525cd27e584560b129598df31fd2c5b2a682a) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "cap-sf3-3", 0, BAD_DUMP SHA1(606e62cc5f46275e366e7dbb412dbaeb7e54cd0c) )
ROM_END

ROM_START( sfiiiu )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "sfiii_usa_region_b1.29f400.u2", 0x000000, 0x080000, CRC(fb172a8e) SHA1(48ebf59910f246835f7dc0c588da30f7a908072f) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "cap-sf3-3", 0, BAD_DUMP SHA1(606e62cc5f46275e366e7dbb412dbaeb7e54cd0c) )
ROM_END

ROM_START( sfiiia )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "sfiii_asia_region_bd.29f400.u2", 0x000000, 0x080000,  CRC(cbd28de7) SHA1(9c15ecb73b9587d20850e62e8683930a45caa01b) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "cap-sf3-3", 0, BAD_DUMP SHA1(606e62cc5f46275e366e7dbb412dbaeb7e54cd0c) )
ROM_END

ROM_START( sfiiij )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "sfiii_japan.29f400.u2", 0x000000, 0x080000, CRC(74205250) SHA1(c3e83ace7121d32da729162662ec6b5285a31211) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "cap-sf3-3", 0, BAD_DUMP SHA1(606e62cc5f46275e366e7dbb412dbaeb7e54cd0c) )
ROM_END

ROM_START( sfiiih )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "sfiii_hispanic.29f400.u2", 0x000000, 0x080000, CRC(d2b3cd48) SHA1(00ebb270c24a66515c97e35331de54ff5358000e) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "cap-sf3-3", 0, BAD_DUMP SHA1(606e62cc5f46275e366e7dbb412dbaeb7e54cd0c) )
ROM_END


ROM_START( sfiii2 )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "sfiii2_usa.29f400.u2", 0x000000, 0x080000, CRC(75dd72e0) SHA1(5a12d6ea6734df5de00ecee6f9ef470749d2f242) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "cap-3ga000", 0, BAD_DUMP SHA1(4e162885b0b3265a56e0265037bcf247e820f027) )
ROM_END

ROM_START( sfiii2j )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "sfiii2_japan.29f400.u2", 0x000000, 0x080000, CRC(faea0a3e) SHA1(a03cd63bcf52e4d57f7a598c8bc8e243694624ec) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "cap-3ga000", 0, BAD_DUMP SHA1(4e162885b0b3265a56e0265037bcf247e820f027) )
ROM_END


ROM_START( jojo )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "jojo_usa.29f400.u2", 0x000000, 0x080000, CRC(8d40f7be) SHA1(2a4bd83db2f959c33b071e517941aa55a0f919c0) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "cap-jjk-3", 0, SHA1(dc6e74b5e02e13f62cb8c4e234dd6061501e49c1) )
ROM_END

ROM_START( jojor1 )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "jojo_usa.29f400.u2", 0x000000, 0x080000, CRC(8d40f7be) SHA1(2a4bd83db2f959c33b071e517941aa55a0f919c0) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "cap-jjk-2", 0, BAD_DUMP SHA1(0f5c09171409213e191a607ee89ca3a91fe9c96a) )
ROM_END

ROM_START( jojor2 )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "jojo_usa.29f400.u2", 0x000000, 0x080000, CRC(8d40f7be) SHA1(2a4bd83db2f959c33b071e517941aa55a0f919c0) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "cap-jjk000", 0, BAD_DUMP SHA1(09869f6d8c032b527e02d815749dc8fab1289e86) )
ROM_END

ROM_START( jojoj )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "jojo_japan.29f400.u2", 0x000000, 0x080000, CRC(02778f60) SHA1(a167f9ebe030592a0cdb0c6a3c75835c6a43be4c) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "cap-jjk-3", 0, SHA1(dc6e74b5e02e13f62cb8c4e234dd6061501e49c1) )
ROM_END

ROM_START( jojojr1 )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "jojo_japan.29f400.u2", 0x000000, 0x080000, CRC(02778f60) SHA1(a167f9ebe030592a0cdb0c6a3c75835c6a43be4c) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "cap-jjk-2", 0, BAD_DUMP SHA1(0f5c09171409213e191a607ee89ca3a91fe9c96a) )
ROM_END

ROM_START( jojojr2 )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "jojo_japan.29f400.u2", 0x000000, 0x080000, CRC(02778f60) SHA1(a167f9ebe030592a0cdb0c6a3c75835c6a43be4c) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "cap-jjk000", 0, BAD_DUMP SHA1(09869f6d8c032b527e02d815749dc8fab1289e86) )
ROM_END


ROM_START( sfiii3 )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "sfiii3_euro.29f400.u2", 0x000000, 0x080000, CRC(30bbf293) SHA1(f094c2eeaf4f6709060197aca371a4532346bf78) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "cap-33s-2", 0, BAD_DUMP SHA1(41b0e246db91cbfc3f8f0f62d981734feb4b4ab5) )
ROM_END

ROM_START( sfiii3r1 )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "sfiii3_euro.29f400.u2", 0x000000, 0x080000, CRC(30bbf293) SHA1(f094c2eeaf4f6709060197aca371a4532346bf78) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "cap-33s-1", 0, BAD_DUMP SHA1(2f4a9006a31903114f9f9dc09465ae253e565c51) )
ROM_END

ROM_START( sfiii3u )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "sfiii3_usa.29f400.u2", 0x000000, 0x080000, CRC(ecc545c1) SHA1(e39083820aae914fd8b80c9765129bedb745ceba) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "cap-33s-2", 0, BAD_DUMP SHA1(41b0e246db91cbfc3f8f0f62d981734feb4b4ab5) )
ROM_END

ROM_START( sfiii3ur1 )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "sfiii3_usa.29f400.u2", 0x000000, 0x080000, CRC(ecc545c1) SHA1(e39083820aae914fd8b80c9765129bedb745ceba) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "cap-33s-1", 0, BAD_DUMP SHA1(2f4a9006a31903114f9f9dc09465ae253e565c51) )
ROM_END


ROM_START( jojoba )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "jojoba_japan.29f400.u2", 0x000000, 0x080000, CRC(3085478c) SHA1(055eab1fc42816f370a44b17fd7e87ffcb10e8b7) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "cap-jjm-1", 0, SHA1(8628d3fa555fbd5f4121082e925c1834b76c5e65) )
ROM_END

ROM_START( jojobar1 )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "jojoba_japan.29f400.u2", 0x000000, 0x080000, CRC(3085478c) SHA1(055eab1fc42816f370a44b17fd7e87ffcb10e8b7) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "cap-jjm-0", 0, BAD_DUMP SHA1(0678a0baeb853dcff1d230c14f0873cc9f143d7b) )
ROM_END





/* NO CD sets - use NO CD BIOS roms - don't require the CD image to boot */

ROM_START( sfiiin )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "sfiii_asia_nocd.29f400.u2", 0x000000, 0x080000, CRC(ca2b715f) SHA1(86319987f9af4afd272a2488e73de8382743cb37) ) // this is a different VERSION of the bios compared to all other sets, not just an alt region code

	ROM_REGION( 0x200000, "simm1.0", 0 )
	ROM_LOAD( "sfiii-simm1.0", 0x00000, 0x200000, CRC(cfc9e45a) SHA1(5d9061f76680642e730373e3ac29b24926dc5c0c) )
	ROM_REGION( 0x200000, "simm1.1", 0 )
	ROM_LOAD( "sfiii-simm1.1", 0x00000, 0x200000, CRC(57920546) SHA1(c8452e7e101b8888fb806d1c9874c6be49fc7dbd) )
	ROM_REGION( 0x200000, "simm1.2", 0 )
	ROM_LOAD( "sfiii-simm1.2", 0x00000, 0x200000, CRC(0d8f2680) SHA1(ade7b28acd11023696c4b20136f3d2f34da6b1be) )
	ROM_REGION( 0x200000, "simm1.3", 0 )
	ROM_LOAD( "sfiii-simm1.3", 0x00000, 0x200000, CRC(ea4ca054) SHA1(f91c55c4e4fc428ce15d27be38aeed3a483d028c) )

	ROM_REGION( 0x200000, "simm3.0", 0 )
	ROM_LOAD( "sfiii-simm3.0", 0x00000, 0x200000, CRC(080b3bd3) SHA1(f51bc5de95ab22b87ba09ea721285b308afd0bda) )
	ROM_REGION( 0x200000, "simm3.1", 0 )
	ROM_LOAD( "sfiii-simm3.1", 0x00000, 0x200000, CRC(5c356f2f) SHA1(e969ce388f6e565d9612e65b0895560c7bb472e6) )
	ROM_REGION( 0x200000, "simm3.2", 0 )
	ROM_LOAD( "sfiii-simm3.2", 0x00000, 0x200000, CRC(f9c97a45) SHA1(58a9691696c3f26a1150a451567c501f55cf1874) )
	ROM_REGION( 0x200000, "simm3.3", 0 )
	ROM_LOAD( "sfiii-simm3.3", 0x00000, 0x200000, CRC(09de3ead) SHA1(2f41d84a96cb5e0d169200a4e9358ad5f407a2b7) )
	ROM_REGION( 0x200000, "simm3.4", 0 )
	ROM_LOAD( "sfiii-simm3.4", 0x00000, 0x200000, CRC(7dd7e1f3) SHA1(bcf1023287457d97f09d9f6e9c93fdf24cc24a07) )
	ROM_REGION( 0x200000, "simm3.5", 0 )
	ROM_LOAD( "sfiii-simm3.5", 0x00000, 0x200000, CRC(47a03a3a) SHA1(2509e5737059251888e4e1efbcdfac86a89ff1a1) )
	ROM_REGION( 0x200000, "simm3.6", 0 )
	ROM_LOAD( "sfiii-simm3.6", 0x00000, 0x200000, CRC(e9eb7a26) SHA1(b8547edb7085e9149aa59d5226ad2d1976cab2bd) )
	ROM_REGION( 0x200000, "simm3.7", 0 )
	ROM_LOAD( "sfiii-simm3.7", 0x00000, 0x200000, CRC(7f44395c) SHA1(f4d2e283cb3a4aad4eae4e13963a74e20be7c181) )

	ROM_REGION( 0x200000, "simm4.0", 0 )
	ROM_LOAD( "sfiii-simm4.0", 0x00000, 0x200000, CRC(9ac080fc) SHA1(2e5024b35b147513ee42eda8748df9d669410377) )
	ROM_REGION( 0x200000, "simm4.1", 0 )
	ROM_LOAD( "sfiii-simm4.1", 0x00000, 0x200000, CRC(6e2c4c94) SHA1(5a185cb76b5999bd826bc9b5ea584a5c3498f69d) )
	ROM_REGION( 0x200000, "simm4.2", 0 )
	ROM_LOAD( "sfiii-simm4.2", 0x00000, 0x200000, CRC(8afc22d4) SHA1(04a419a3092c98fc4a7693e6acf30ae5a849e5c1) )
	ROM_REGION( 0x200000, "simm4.3", 0 )
	ROM_LOAD( "sfiii-simm4.3", 0x00000, 0x200000, CRC(9f3873b8) SHA1(33499d6f02bc84c80acb56be078aaed7f8d1300d) )
	ROM_REGION( 0x200000, "simm4.4", 0 )
	ROM_LOAD( "sfiii-simm4.4", 0x00000, 0x200000, CRC(166b3c97) SHA1(40e6e9d43cbbd8496b430931b8ab7db01dc1c6d5) )
	ROM_REGION( 0x200000, "simm4.5", 0 )
	ROM_LOAD( "sfiii-simm4.5", 0x00000, 0x200000, CRC(e5ea2547) SHA1(a823c689098f37a3054d728bddb0033a4b8396f1) )
	ROM_REGION( 0x200000, "simm4.6", 0 )
	ROM_LOAD( "sfiii-simm4.6", 0x00000, 0x200000, CRC(e85b9fdd) SHA1(264cb10fe9b3ede384c7db42bfc58ed5c21ea8f8) )
	ROM_REGION( 0x200000, "simm4.7", 0 )
	ROM_LOAD( "sfiii-simm4.7", 0x00000, 0x200000, CRC(362c01b7) SHA1(9c404312a6aabe8e91e68dde193e3972bc1636cd) )

	ROM_REGION( 0x200000, "simm5.0", 0 )
	ROM_LOAD( "sfiii-simm5.0", 0x00000, 0x200000, CRC(9bc108b2) SHA1(894dadab7957044bf877029c7f8e556d5d6e85d3) )
	ROM_REGION( 0x200000, "simm5.1", 0 )
	ROM_LOAD( "sfiii-simm5.1", 0x00000, 0x200000, CRC(c6f1c066) SHA1(00de492dd1ef7aef05027a8c501c296b6602e917) )
ROM_END


ROM_START( sfiiina )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "sfiii_asia_nocd.29f400.u2", 0x000000, 0x080000, CRC(73e32463) SHA1(45d144e533e4b20cc5a744ca4f618e288430c601) ) // sldh

	ROM_REGION( 0x200000, "simm1.0", 0 )
	ROM_LOAD( "sfiii-simm1.0", 0x00000, 0x200000, CRC(cfc9e45a) SHA1(5d9061f76680642e730373e3ac29b24926dc5c0c) )
	ROM_REGION( 0x200000, "simm1.1", 0 )
	ROM_LOAD( "sfiii-simm1.1", 0x00000, 0x200000, CRC(57920546) SHA1(c8452e7e101b8888fb806d1c9874c6be49fc7dbd) )
	ROM_REGION( 0x200000, "simm1.2", 0 )
	ROM_LOAD( "sfiii-simm1.2", 0x00000, 0x200000, CRC(0d8f2680) SHA1(ade7b28acd11023696c4b20136f3d2f34da6b1be) )
	ROM_REGION( 0x200000, "simm1.3", 0 )
	ROM_LOAD( "sfiii-simm1.3", 0x00000, 0x200000, CRC(ea4ca054) SHA1(f91c55c4e4fc428ce15d27be38aeed3a483d028c) )

	ROM_REGION( 0x200000, "simm3.0", 0 )
	ROM_LOAD( "sfiii-simm3.0", 0x00000, 0x200000, CRC(080b3bd3) SHA1(f51bc5de95ab22b87ba09ea721285b308afd0bda) )
	ROM_REGION( 0x200000, "simm3.1", 0 )
	ROM_LOAD( "sfiii-simm3.1", 0x00000, 0x200000, CRC(5c356f2f) SHA1(e969ce388f6e565d9612e65b0895560c7bb472e6) )
	ROM_REGION( 0x200000, "simm3.2", 0 )
	ROM_LOAD( "sfiii-simm3.2", 0x00000, 0x200000, CRC(f9c97a45) SHA1(58a9691696c3f26a1150a451567c501f55cf1874) )
	ROM_REGION( 0x200000, "simm3.3", 0 )
	ROM_LOAD( "sfiii-simm3.3", 0x00000, 0x200000, CRC(09de3ead) SHA1(2f41d84a96cb5e0d169200a4e9358ad5f407a2b7) )
	ROM_REGION( 0x200000, "simm3.4", 0 )
	ROM_LOAD( "sfiii-simm3.4", 0x00000, 0x200000, CRC(7dd7e1f3) SHA1(bcf1023287457d97f09d9f6e9c93fdf24cc24a07) )
	ROM_REGION( 0x200000, "simm3.5", 0 )
	ROM_LOAD( "sfiii-simm3.5", 0x00000, 0x200000, CRC(47a03a3a) SHA1(2509e5737059251888e4e1efbcdfac86a89ff1a1) )
	ROM_REGION( 0x200000, "simm3.6", 0 )
	ROM_LOAD( "sfiii-simm3.6", 0x00000, 0x200000, CRC(e9eb7a26) SHA1(b8547edb7085e9149aa59d5226ad2d1976cab2bd) )
	ROM_REGION( 0x200000, "simm3.7", 0 )
	ROM_LOAD( "sfiii-simm3.7", 0x00000, 0x200000, CRC(7f44395c) SHA1(f4d2e283cb3a4aad4eae4e13963a74e20be7c181) )

	ROM_REGION( 0x200000, "simm4.0", 0 )
	ROM_LOAD( "sfiii-simm4.0", 0x00000, 0x200000, CRC(9ac080fc) SHA1(2e5024b35b147513ee42eda8748df9d669410377) )
	ROM_REGION( 0x200000, "simm4.1", 0 )
	ROM_LOAD( "sfiii-simm4.1", 0x00000, 0x200000, CRC(6e2c4c94) SHA1(5a185cb76b5999bd826bc9b5ea584a5c3498f69d) )
	ROM_REGION( 0x200000, "simm4.2", 0 )
	ROM_LOAD( "sfiii-simm4.2", 0x00000, 0x200000, CRC(8afc22d4) SHA1(04a419a3092c98fc4a7693e6acf30ae5a849e5c1) )
	ROM_REGION( 0x200000, "simm4.3", 0 )
	ROM_LOAD( "sfiii-simm4.3", 0x00000, 0x200000, CRC(9f3873b8) SHA1(33499d6f02bc84c80acb56be078aaed7f8d1300d) )
	ROM_REGION( 0x200000, "simm4.4", 0 )
	ROM_LOAD( "sfiii-simm4.4", 0x00000, 0x200000, CRC(166b3c97) SHA1(40e6e9d43cbbd8496b430931b8ab7db01dc1c6d5) )
	ROM_REGION( 0x200000, "simm4.5", 0 )
	ROM_LOAD( "sfiii-simm4.5", 0x00000, 0x200000, CRC(e5ea2547) SHA1(a823c689098f37a3054d728bddb0033a4b8396f1) )
	ROM_REGION( 0x200000, "simm4.6", 0 )
	ROM_LOAD( "sfiii-simm4.6", 0x00000, 0x200000, CRC(e85b9fdd) SHA1(264cb10fe9b3ede384c7db42bfc58ed5c21ea8f8) )
	ROM_REGION( 0x200000, "simm4.7", 0 )
	ROM_LOAD( "sfiii-simm4.7", 0x00000, 0x200000, CRC(362c01b7) SHA1(9c404312a6aabe8e91e68dde193e3972bc1636cd) )

	ROM_REGION( 0x200000, "simm5.0", 0 )
	ROM_LOAD( "sfiii-simm5.0", 0x00000, 0x200000, CRC(9bc108b2) SHA1(894dadab7957044bf877029c7f8e556d5d6e85d3) )
	ROM_REGION( 0x200000, "simm5.1", 0 )
	ROM_LOAD( "sfiii-simm5.1", 0x00000, 0x200000, CRC(c6f1c066) SHA1(00de492dd1ef7aef05027a8c501c296b6602e917) )
ROM_END

ROM_START( sfiii2n )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "sfiii2_asia_nocd.29f400.u2", 0x000000, 0x080000, CRC(fd297c0d) SHA1(4323deda2789f104b53f32a663196ec16de73215) )

	ROM_REGION( 0x200000, "simm1.0", 0 )
	ROM_LOAD( "sfiii2-simm1.0", 0x00000, 0x200000, CRC(2d666f0b) SHA1(68de034b3a3aeaf4b26122a84ad48b0b763e4122) )
	ROM_REGION( 0x200000, "simm1.1", 0 )
	ROM_LOAD( "sfiii2-simm1.1", 0x00000, 0x200000, CRC(2a3a8ef6) SHA1(31fb58fd1360ed8c951e2c4ac898a5a7104528d6) )
	ROM_REGION( 0x200000, "simm1.2", 0 )
	ROM_LOAD( "sfiii2-simm1.2", 0x00000, 0x200000, CRC(161d2206) SHA1(58999f876e64c1a088e8765962a9cd504f22a706) )
	ROM_REGION( 0x200000, "simm1.3", 0 )
	ROM_LOAD( "sfiii2-simm1.3", 0x00000, 0x200000, CRC(87ded8a3) SHA1(4ccef64f80d2ee63940b0958b500364ee515db51) )

	ROM_REGION( 0x200000, "simm2.0", 0 )
	ROM_LOAD( "sfiii2-simm2.0", 0x00000, 0x200000, CRC(94a4ce0f) SHA1(2c8e26a66d1dcd17c22c70baa2a3ff5a54511514) )
	ROM_REGION( 0x200000, "simm2.1", 0 )
	ROM_LOAD( "sfiii2-simm2.1", 0x00000, 0x200000, CRC(67585033) SHA1(24df9968a54c330fbe95f8e4dfe6e7dfd144ed0c) )
	ROM_REGION( 0x200000, "simm2.2", 0 )
	ROM_LOAD( "sfiii2-simm2.2", 0x00000, 0x200000, CRC(fabffcd5) SHA1(9399f64c42f63a64e44a21a2690e44779943a2b2) )
	ROM_REGION( 0x200000, "simm2.3", 0 )
	ROM_LOAD( "sfiii2-simm2.3", 0x00000, 0x200000, CRC(623c09ca) SHA1(dc9618a08bb7f44e569ac17605d268511155a14e) )

	ROM_REGION( 0x200000, "simm3.0", 0 )
	ROM_LOAD( "sfiii2-simm3.0", 0x00000, 0x200000, CRC(dab2d766) SHA1(d265cc8b1b497eb4bedd63b3f1de60eb1c1db0df) )
	ROM_REGION( 0x200000, "simm3.1", 0 )
	ROM_LOAD( "sfiii2-simm3.1", 0x00000, 0x200000, CRC(1f2aa34b) SHA1(38b224d34c4550f1f33c2c368e2a252d0d176cc0) )
	ROM_REGION( 0x200000, "simm3.2", 0 )
	ROM_LOAD( "sfiii2-simm3.2", 0x00000, 0x200000, CRC(6f1a04eb) SHA1(980ca929114075d1920e2da44f9a22087cc92e55) )
	ROM_REGION( 0x200000, "simm3.3", 0 )
	ROM_LOAD( "sfiii2-simm3.3", 0x00000, 0x200000, CRC(e05ef205) SHA1(e604e3832549740f953581fc91e850beda6a73c8) )
	ROM_REGION( 0x200000, "simm3.4", 0 )
	ROM_LOAD( "sfiii2-simm3.4", 0x00000, 0x200000, CRC(affb074f) SHA1(0e76973807039bc66fd0f3233401cea8d2c45f84) )
	ROM_REGION( 0x200000, "simm3.5", 0 )
	ROM_LOAD( "sfiii2-simm3.5", 0x00000, 0x200000, CRC(6962872e) SHA1(f16b2d0792697345145d0e9d950e912a2ffabe0d) )
	ROM_REGION( 0x200000, "simm3.6", 0 )
	ROM_LOAD( "sfiii2-simm3.6", 0x00000, 0x200000, CRC(6eed87de) SHA1(5d5067ad36234c5efd57a2baebeffa2f44f2caec) )
	ROM_REGION( 0x200000, "simm3.7", 0 )
	ROM_LOAD( "sfiii2-simm3.7", 0x00000, 0x200000, CRC(e18f479e) SHA1(cd4c1812ab422336bfa414e0b2098b472d2f9251) )

	ROM_REGION( 0x200000, "simm4.0", 0 )
	ROM_LOAD( "sfiii2-simm4.0", 0x00000, 0x200000, CRC(764c2503) SHA1(cad3f20ade2e1d3ac52f8c318443da20062ae943) )
	ROM_REGION( 0x200000, "simm4.1", 0 )
	ROM_LOAD( "sfiii2-simm4.1", 0x00000, 0x200000, CRC(3e16af6e) SHA1(afde2ed4bf3a3e95035fc02c572c5b83178a9467) )
	ROM_REGION( 0x200000, "simm4.2", 0 )
	ROM_LOAD( "sfiii2-simm4.2", 0x00000, 0x200000, CRC(215705e6) SHA1(42d3849f8a9242a89ba465dbc205f310186c67cd) )
	ROM_REGION( 0x200000, "simm4.3", 0 )
	ROM_LOAD( "sfiii2-simm4.3", 0x00000, 0x200000, CRC(e30cbd9c) SHA1(c205101ada86154921e09fed4f6908d15ec60761) )
	ROM_REGION( 0x200000, "simm4.4", 0 )
	ROM_LOAD( "sfiii2-simm4.4", 0x00000, 0x200000, CRC(4185ded9) SHA1(24bf9b5f25d7753f1feb09b82611f7482f30d304) )
	ROM_REGION( 0x200000, "simm4.5", 0 )
	ROM_LOAD( "sfiii2-simm4.5", 0x00000, 0x200000, CRC(4e8db013) SHA1(6816df2b6c60005fb375530ea93bb30a960c9b01) )
	ROM_REGION( 0x200000, "simm4.6", 0 )
	ROM_LOAD( "sfiii2-simm4.6", 0x00000, 0x200000, CRC(08df48ce) SHA1(e8a3b68ebeab193539446c3f6e0a19b37f1f3495) )
	ROM_REGION( 0x200000, "simm4.7", 0 )
	ROM_LOAD( "sfiii2-simm4.7", 0x00000, 0x200000, CRC(bb8f80a5) SHA1(35d9e86637d54405c97fdb7da9c42cc53907cae3) )

	ROM_REGION( 0x200000, "simm5.0", 0 )
	ROM_LOAD( "sfiii2-simm5.0", 0x00000, 0x200000, CRC(ebdc4787) SHA1(f86e8ebf4b2214be166dbe4ea921058a552364ea) )
	ROM_REGION( 0x200000, "simm5.1", 0 )
	ROM_LOAD( "sfiii2-simm5.1", 0x00000, 0x200000, CRC(6b7c550e) SHA1(77cdabccf3ecebf142ac86dffe6e24052941e3a1) )
	ROM_REGION( 0x200000, "simm5.2", 0 )
	ROM_LOAD( "sfiii2-simm5.2", 0x00000, 0x200000, CRC(56ff8c50) SHA1(16f7602a4549a5b724e3fcdb75b0f3c397077b81) )
	ROM_REGION( 0x200000, "simm5.3", 0 )
	ROM_LOAD( "sfiii2-simm5.3", 0x00000, 0x200000, CRC(3f2ac3e9) SHA1(a7b631f18ce572a42f46314f37a01d9840abc765) )
	ROM_REGION( 0x200000, "simm5.4", 0 )
	ROM_LOAD( "sfiii2-simm5.4", 0x00000, 0x200000, CRC(48cda50e) SHA1(35e9f27fb8b69e3b3a313ea33dc53b1102e5f66e) )
	ROM_REGION( 0x200000, "simm5.5", 0 )
	ROM_LOAD( "sfiii2-simm5.5", 0x00000, 0x200000, CRC(520c0af6) SHA1(7bed1b6707974eafbfb62ccb84a51df8a100e070) )
	ROM_REGION( 0x200000, "simm5.6", 0 )
	ROM_LOAD( "sfiii2-simm5.6", 0x00000, 0x200000, CRC(2edc5986) SHA1(761ab2c67d0d873ffd74158eb77f7722c076f3e3) )
	ROM_REGION( 0x200000, "simm5.7", 0 )
	ROM_LOAD( "sfiii2-simm5.7", 0x00000, 0x200000, CRC(93ffa199) SHA1(33ec2379f30c6fdf47ba72c1d0cad8bdd02f17df) )
ROM_END

ROM_START( jojon )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "jojo_asia_nocd.29f400.u2", 0x000000, 0x080000, CRC(05b4f953) SHA1(c746c7bb5359acc9adced817cb4870b1912eaefd) )

	ROM_REGION( 0x200000, "simm1.0", 0 )
	ROM_LOAD( "jojo-simm1.0", 0x00000, 0x200000, CRC(9516948b) SHA1(4d7e6c1eb7d1bebff2a5069bcd186070a9105474) ) // sldh
	ROM_REGION( 0x200000, "simm1.1", 0 )
	ROM_LOAD( "jojo-simm1.1", 0x00000, 0x200000, CRC(a847848d) SHA1(4df70309395f1d2a2e8f85bc34e17453d4a76f81) ) // sldh
	ROM_REGION( 0x200000, "simm1.2", 0 )
	ROM_LOAD( "jojo-simm1.2", 0x00000, 0x200000, CRC(853e8846) SHA1(d120b7e2de9502e3261e8dd101f97589b2ed1c38) ) // sldh
	ROM_REGION( 0x200000, "simm1.3", 0 )
	ROM_LOAD( "jojo-simm1.3", 0x00000, 0x200000, CRC(c04fe00e) SHA1(d09409b77460d19b56aaaf4a64356f3d37a1ee41) ) // sldh

	ROM_REGION( 0x200000, "simm2.0", 0 )
	ROM_LOAD( "jojo-simm2.0", 0x00000, 0x200000, CRC(e1a4b3c8) SHA1(5dc298431644e1ca470aaab752a7d74f2f9dc7a1) ) // sldh
	ROM_REGION( 0x200000, "simm2.1", 0 )
	ROM_LOAD( "jojo-simm2.1", 0x00000, 0x200000, CRC(189cef95) SHA1(ebe42a019358461557f69fb17d65d84d0f733415) ) // sldh
	ROM_REGION( 0x200000, "simm2.2", 0 )
	ROM_LOAD( "jojo-simm2.2", 0x00000, 0x200000, CRC(47db5ec6) SHA1(e80271e4013e4391c2cc4229ff1fbd4a2b7c6f04) ) // sldh
	ROM_REGION( 0x200000, "simm2.3", 0 )
	ROM_LOAD( "jojo-simm2.3", 0x00000, 0x200000, CRC(e3d3a155) SHA1(75e9b5da93dd8894cf70fa4dac56f3958be4c766) ) // sldh

	ROM_REGION( 0x200000, "simm3.0", 0 )
	ROM_LOAD( "jojo-simm3.0", 0x00000, 0x200000, CRC(de7fc9c1) SHA1(662b85a990b04c855773506c936317e62fab4a05) )
	ROM_REGION( 0x200000, "simm3.1", 0 )
	ROM_LOAD( "jojo-simm3.1", 0x00000, 0x200000, CRC(43d053d3) SHA1(54ff0e9c164e0d1649522c330ccc7e5d79e0bc85) )
	ROM_REGION( 0x200000, "simm3.2", 0 )
	ROM_LOAD( "jojo-simm3.2", 0x00000, 0x200000, CRC(2ffd7fa5) SHA1(9018c8e2b286a333ba606208e90caa764951ea3f) )
	ROM_REGION( 0x200000, "simm3.3", 0 )
	ROM_LOAD( "jojo-simm3.3", 0x00000, 0x200000, CRC(4da4985b) SHA1(2552b1730a21ce17d58b69a79ad212a6a5829439) )
	ROM_REGION( 0x200000, "simm3.4", 0 )
	ROM_LOAD( "jojo-simm3.4", 0x00000, 0x200000, CRC(fde98d72) SHA1(654563e12d033e8656dc74a268a08b15b171470d) )
	ROM_REGION( 0x200000, "simm3.5", 0 )
	ROM_LOAD( "jojo-simm3.5", 0x00000, 0x200000, CRC(edb2a266) SHA1(19ebada8422c7f4bf70d0c9ad42b84268967b316) )
	ROM_REGION( 0x200000, "simm3.6", 0 )
	ROM_LOAD( "jojo-simm3.6", 0x00000, 0x200000, CRC(be7cf319) SHA1(7893f5907992e6b903b2683980bba6d3d003bb06) )
	ROM_REGION( 0x200000, "simm3.7", 0 )
	ROM_LOAD( "jojo-simm3.7", 0x00000, 0x200000, CRC(56fe1a9f) SHA1(01741fe1256f4e682f687e94040f4e8bbb8bedb2) )

	ROM_REGION( 0x200000, "simm4.0", 0 )
	ROM_LOAD( "jojo-simm4.0", 0x00000, 0x200000, CRC(c4e7bf68) SHA1(a4d1ddea58a3d42db82a63a5e974cbf38d9b792a) )
	ROM_REGION( 0x200000, "simm4.1", 0 )
	ROM_LOAD( "jojo-simm4.1", 0x00000, 0x200000, CRC(b62b2719) SHA1(cb577b89e9e14fda67715716fefd47a782d518ab) )
	ROM_REGION( 0x200000, "simm4.2", 0 )
	ROM_LOAD( "jojo-simm4.2", 0x00000, 0x200000, CRC(18d15809) SHA1(2b406cd1aaa4799a436213dcaa65473eacb4c6d7) )
	ROM_REGION( 0x200000, "simm4.3", 0 )
	ROM_LOAD( "jojo-simm4.3", 0x00000, 0x200000, CRC(9af0ad79) SHA1(075ee048e17b50188876f25d7a6571d6ace84d7d) )
	ROM_REGION( 0x200000, "simm4.4", 0 )
	ROM_LOAD( "jojo-simm4.4", 0x00000, 0x200000, CRC(4124c1f0) SHA1(e4946a8029adc5d0bacead8d766521b4ccd1722b) )
	ROM_REGION( 0x200000, "simm4.5", 0 )
	ROM_LOAD( "jojo-simm4.5", 0x00000, 0x200000, CRC(5e001fd1) SHA1(6457a39f336381b46e587aa2f5f719810ee5bcf9) )
	ROM_REGION( 0x200000, "simm4.6", 0 )
	ROM_LOAD( "jojo-simm4.6", 0x00000, 0x200000, CRC(9affa23b) SHA1(e3d77e777c47277d841a9dadc1dd6e3157706a2e) )
	ROM_REGION( 0x200000, "simm4.7", 0 )
	ROM_LOAD( "jojo-simm4.7", 0x00000, 0x200000, CRC(2511572a) SHA1(725adcf71bcee5c8bb839d2d1c5e3456b8c6886b) )

	ROM_REGION( 0x200000, "simm5.0", 0 )
	ROM_LOAD( "jojo-simm5.0", 0x00000, 0x200000, CRC(797615fc) SHA1(29874be9f1da5515c90f5d601aa5924c263f8feb) )
	ROM_REGION( 0x200000, "simm5.1", 0 )
	ROM_LOAD( "jojo-simm5.1", 0x00000, 0x200000, CRC(734fd162) SHA1(16cdfac74d18a6c2216afb1ce6afbd7f15297c32) )
ROM_END

ROM_START( jojonr1 )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "jojo_asia_nocd.29f400.u2", 0x000000, 0x080000, CRC(05b4f953) SHA1(c746c7bb5359acc9adced817cb4870b1912eaefd) )

	ROM_REGION( 0x200000, "simm1.0", 0 )
	ROM_LOAD( "jojo-simm1.0", 0x00000, 0x200000, CRC(cfbc38d6) SHA1(c33e3a51fe8ab54e0912a1d6e662fe1ade73cee7) ) // sldh
	ROM_REGION( 0x200000, "simm1.1", 0 )
	ROM_LOAD( "jojo-simm1.1", 0x00000, 0x200000, CRC(42578d94) SHA1(fa46f92ac1a6716430adec9ab27214a11fa61749) ) // sldh
	ROM_REGION( 0x200000, "simm1.2", 0 )
	ROM_LOAD( "jojo-simm1.2", 0x00000, 0x200000, CRC(1b40c566) SHA1(9833799e9b4fecf7f9ce14bca64936646b3fdbde) ) // sldh
	ROM_REGION( 0x200000, "simm1.3", 0 )
	ROM_LOAD( "jojo-simm1.3", 0x00000, 0x200000, CRC(bba709b4) SHA1(0dd71e575f2193505f2ab960568ac1eccf40d53f) ) // sldh

	ROM_REGION( 0x200000, "simm2.0", 0 )
	ROM_LOAD( "jojo-simm2.0", 0x00000, 0x200000, CRC(417e5dc1) SHA1(54ee9596c1c51811f3bdef7dbe77b44b34f230ca) ) // sldh
	ROM_REGION( 0x200000, "simm2.1", 0 )
	ROM_LOAD( "jojo-simm2.1", 0x00000, 0x200000, CRC(d3b3267d) SHA1(eb2cff347880f1489fb5b1b8bd16df8f50c7f494) ) // sldh
	ROM_REGION( 0x200000, "simm2.2", 0 )
	ROM_LOAD( "jojo-simm2.2", 0x00000, 0x200000, CRC(c66d96b1) SHA1(909d5aac165748b549b6056a6091c41df012f5df) ) // sldh
	ROM_REGION( 0x200000, "simm2.3", 0 )
	ROM_LOAD( "jojo-simm2.3", 0x00000, 0x200000, CRC(aa34cc85) SHA1(7677cc6fa913755fc699691b350698bbe8904118) ) // sldh

	ROM_REGION( 0x200000, "simm3.0", 0 )
	ROM_LOAD( "jojo-simm3.0", 0x00000, 0x200000, CRC(de7fc9c1) SHA1(662b85a990b04c855773506c936317e62fab4a05) )
	ROM_REGION( 0x200000, "simm3.1", 0 )
	ROM_LOAD( "jojo-simm3.1", 0x00000, 0x200000, CRC(43d053d3) SHA1(54ff0e9c164e0d1649522c330ccc7e5d79e0bc85) )
	ROM_REGION( 0x200000, "simm3.2", 0 )
	ROM_LOAD( "jojo-simm3.2", 0x00000, 0x200000, CRC(2ffd7fa5) SHA1(9018c8e2b286a333ba606208e90caa764951ea3f) )
	ROM_REGION( 0x200000, "simm3.3", 0 )
	ROM_LOAD( "jojo-simm3.3", 0x00000, 0x200000, CRC(4da4985b) SHA1(2552b1730a21ce17d58b69a79ad212a6a5829439) )
	ROM_REGION( 0x200000, "simm3.4", 0 )
	ROM_LOAD( "jojo-simm3.4", 0x00000, 0x200000, CRC(fde98d72) SHA1(654563e12d033e8656dc74a268a08b15b171470d) )
	ROM_REGION( 0x200000, "simm3.5", 0 )
	ROM_LOAD( "jojo-simm3.5", 0x00000, 0x200000, CRC(edb2a266) SHA1(19ebada8422c7f4bf70d0c9ad42b84268967b316) )
	ROM_REGION( 0x200000, "simm3.6", 0 )
	ROM_LOAD( "jojo-simm3.6", 0x00000, 0x200000, CRC(be7cf319) SHA1(7893f5907992e6b903b2683980bba6d3d003bb06) )
	ROM_REGION( 0x200000, "simm3.7", 0 )
	ROM_LOAD( "jojo-simm3.7", 0x00000, 0x200000, CRC(56fe1a9f) SHA1(01741fe1256f4e682f687e94040f4e8bbb8bedb2) )

	ROM_REGION( 0x200000, "simm4.0", 0 )
	ROM_LOAD( "jojo-simm4.0", 0x00000, 0x200000, CRC(c4e7bf68) SHA1(a4d1ddea58a3d42db82a63a5e974cbf38d9b792a) )
	ROM_REGION( 0x200000, "simm4.1", 0 )
	ROM_LOAD( "jojo-simm4.1", 0x00000, 0x200000, CRC(b62b2719) SHA1(cb577b89e9e14fda67715716fefd47a782d518ab) )
	ROM_REGION( 0x200000, "simm4.2", 0 )
	ROM_LOAD( "jojo-simm4.2", 0x00000, 0x200000, CRC(18d15809) SHA1(2b406cd1aaa4799a436213dcaa65473eacb4c6d7) )
	ROM_REGION( 0x200000, "simm4.3", 0 )
	ROM_LOAD( "jojo-simm4.3", 0x00000, 0x200000, CRC(9af0ad79) SHA1(075ee048e17b50188876f25d7a6571d6ace84d7d) )
	ROM_REGION( 0x200000, "simm4.4", 0 )
	ROM_LOAD( "jojo-simm4.4", 0x00000, 0x200000, CRC(4124c1f0) SHA1(e4946a8029adc5d0bacead8d766521b4ccd1722b) )
	ROM_REGION( 0x200000, "simm4.5", 0 )
	ROM_LOAD( "jojo-simm4.5", 0x00000, 0x200000, CRC(5e001fd1) SHA1(6457a39f336381b46e587aa2f5f719810ee5bcf9) )
	ROM_REGION( 0x200000, "simm4.6", 0 )
	ROM_LOAD( "jojo-simm4.6", 0x00000, 0x200000, CRC(9affa23b) SHA1(e3d77e777c47277d841a9dadc1dd6e3157706a2e) )
	ROM_REGION( 0x200000, "simm4.7", 0 )
	ROM_LOAD( "jojo-simm4.7", 0x00000, 0x200000, CRC(2511572a) SHA1(725adcf71bcee5c8bb839d2d1c5e3456b8c6886b) )

	ROM_REGION( 0x200000, "simm5.0", 0 )
	ROM_LOAD( "jojo-simm5.0", 0x00000, 0x200000, CRC(797615fc) SHA1(29874be9f1da5515c90f5d601aa5924c263f8feb) )
	ROM_REGION( 0x200000, "simm5.1", 0 )
	ROM_LOAD( "jojo-simm5.1", 0x00000, 0x200000, CRC(734fd162) SHA1(16cdfac74d18a6c2216afb1ce6afbd7f15297c32) )
ROM_END

ROM_START( jojonr2 )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "jojo_asia_nocd.29f400.u2", 0x000000, 0x080000, CRC(05b4f953) SHA1(c746c7bb5359acc9adced817cb4870b1912eaefd) )

	ROM_REGION( 0x200000, "simm1.0", 0 )
	ROM_LOAD( "jojo-simm1.0", 0x00000, 0x200000, CRC(e06ba886) SHA1(4defd5e8e1e6d0c439fed8a6454e89a59e24ea4c) ) // sldh
	ROM_REGION( 0x200000, "simm1.1", 0 )
	ROM_LOAD( "jojo-simm1.1", 0x00000, 0x200000, CRC(6dd177c8) SHA1(c39db980f6fcca9c221e9be6f777eaf38f1b136b) ) // sldh
	ROM_REGION( 0x200000, "simm1.2", 0 )
	ROM_LOAD( "jojo-simm1.2", 0x00000, 0x200000, CRC(d35a15e0) SHA1(576b92a94505764a10b9bcf82c02335e7ef62014) ) // sldh
	ROM_REGION( 0x200000, "simm1.3", 0 )
	ROM_LOAD( "jojo-simm1.3", 0x00000, 0x200000, CRC(66d865ac) SHA1(5248c3f124af62b4a672d954ef15f86629feeacb) ) // sldh

	ROM_REGION( 0x200000, "simm2.0", 0 )
	ROM_LOAD( "jojo-simm2.0", 0x00000, 0x200000, CRC(417e5dc1) SHA1(54ee9596c1c51811f3bdef7dbe77b44b34f230ca) ) // sldh
	ROM_REGION( 0x200000, "simm2.1", 0 )
	ROM_LOAD( "jojo-simm2.1", 0x00000, 0x200000, CRC(c891c887) SHA1(42e84f774ee655e9a39b016a3cfe94262ed2e9f1) ) // sldh
	ROM_REGION( 0x200000, "simm2.2", 0 )
	ROM_LOAD( "jojo-simm2.2", 0x00000, 0x200000, CRC(1e101f30) SHA1(56518c1646bb9452334856bb8bcc58892f9f93b9) ) // sldh
	ROM_REGION( 0x200000, "simm2.3", 0 )
	ROM_LOAD( "jojo-simm2.3", 0x00000, 0x200000, CRC(1fd1d3e4) SHA1(bed2b77d58f1fdf7ba5ca7126d3db1dd0f8c80b4) ) // sldh

	ROM_REGION( 0x200000, "simm3.0", 0 )
	ROM_LOAD( "jojo-simm3.0",  0x00000, 0x200000, CRC(de7fc9c1) SHA1(662b85a990b04c855773506c936317e62fab4a05) )
	ROM_REGION( 0x200000, "simm3.1", 0 )
	ROM_LOAD( "jojo-simm3.1",  0x00000, 0x200000, CRC(43d053d3) SHA1(54ff0e9c164e0d1649522c330ccc7e5d79e0bc85) )
	ROM_REGION( 0x200000, "simm3.2", 0 )
	ROM_LOAD( "jojo-simm3.2",  0x00000, 0x200000, CRC(2ffd7fa5) SHA1(9018c8e2b286a333ba606208e90caa764951ea3f) )
	ROM_REGION( 0x200000, "simm3.3", 0 )
	ROM_LOAD( "jojo-simm3.3",  0x00000, 0x200000, CRC(4da4985b) SHA1(2552b1730a21ce17d58b69a79ad212a6a5829439) )
	ROM_REGION( 0x200000, "simm3.4", 0 )
	ROM_LOAD( "jojo-simm3.4",  0x00000, 0x200000, CRC(fde98d72) SHA1(654563e12d033e8656dc74a268a08b15b171470d) )
	ROM_REGION( 0x200000, "simm3.5", 0 )
	ROM_LOAD( "jojo-simm3.5",  0x00000, 0x200000, CRC(edb2a266) SHA1(19ebada8422c7f4bf70d0c9ad42b84268967b316) )
	ROM_REGION( 0x200000, "simm3.6", 0 )
	ROM_LOAD( "jojo-simm3.6",  0x00000, 0x200000, CRC(be7cf319) SHA1(7893f5907992e6b903b2683980bba6d3d003bb06) )
	ROM_REGION( 0x200000, "simm3.7", 0 )
	ROM_LOAD( "jojo-simm3.7",  0x00000, 0x200000, CRC(56fe1a9f) SHA1(01741fe1256f4e682f687e94040f4e8bbb8bedb2) )

	ROM_REGION( 0x200000, "simm4.0", 0 )
	ROM_LOAD( "jojo-simm4.0",  0x00000, 0x200000, CRC(c4e7bf68) SHA1(a4d1ddea58a3d42db82a63a5e974cbf38d9b792a) )
	ROM_REGION( 0x200000, "simm4.1", 0 )
	ROM_LOAD( "jojo-simm4.1",  0x00000, 0x200000, CRC(b62b2719) SHA1(cb577b89e9e14fda67715716fefd47a782d518ab) )
	ROM_REGION( 0x200000, "simm4.2", 0 )
	ROM_LOAD( "jojo-simm4.2",  0x00000, 0x200000, CRC(18d15809) SHA1(2b406cd1aaa4799a436213dcaa65473eacb4c6d7) )
	ROM_REGION( 0x200000, "simm4.3", 0 )
	ROM_LOAD( "jojo-simm4.3",  0x00000, 0x200000, CRC(9af0ad79) SHA1(075ee048e17b50188876f25d7a6571d6ace84d7d) )
	ROM_REGION( 0x200000, "simm4.4", 0 )
	ROM_LOAD( "jojo-simm4.4",  0x00000, 0x200000, CRC(4124c1f0) SHA1(e4946a8029adc5d0bacead8d766521b4ccd1722b) )
	ROM_REGION( 0x200000, "simm4.5", 0 )
	ROM_LOAD( "jojo-simm4.5",  0x00000, 0x200000, CRC(5e001fd1) SHA1(6457a39f336381b46e587aa2f5f719810ee5bcf9) )
	ROM_REGION( 0x200000, "simm4.6", 0 )
	ROM_LOAD( "jojo-simm4.6",  0x00000, 0x200000, CRC(9affa23b) SHA1(e3d77e777c47277d841a9dadc1dd6e3157706a2e) )
	ROM_REGION( 0x200000, "simm4.7", 0 )
	ROM_LOAD( "jojo-simm4.7",  0x00000, 0x200000, CRC(2511572a) SHA1(725adcf71bcee5c8bb839d2d1c5e3456b8c6886b) )

	ROM_REGION( 0x200000, "simm5.0", 0 )
	ROM_LOAD( "jojo-simm5.0",  0x00000, 0x200000, CRC(797615fc) SHA1(29874be9f1da5515c90f5d601aa5924c263f8feb) )
	ROM_REGION( 0x200000, "simm5.1", 0 )
	ROM_LOAD( "jojo-simm5.1",  0x00000, 0x200000, CRC(734fd162) SHA1(16cdfac74d18a6c2216afb1ce6afbd7f15297c32) )
ROM_END

ROM_START( sfiii3n )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "sfiii3_japan_nocd.29f400.u2", 0x000000, 0x080000, CRC(1edc6366) SHA1(60b4b9adeb030a33059d74fdf03873029e465b52) )

	ROM_REGION( 0x200000, "simm1.0", 0 )
	ROM_LOAD( "sfiii3-simm1.0", 0x00000, 0x200000, CRC(11dfd3cd) SHA1(dba1f77c46e80317e3279298411154dfb6db2309) ) // sldh
	ROM_REGION( 0x200000, "simm1.1", 0 )
	ROM_LOAD( "sfiii3-simm1.1", 0x00000, 0x200000, CRC(c50585e6) SHA1(a289237957ea1c7f58b1c65e24c54ceb34cb1712) ) // sldh
	ROM_REGION( 0x200000, "simm1.2", 0 )
	ROM_LOAD( "sfiii3-simm1.2", 0x00000, 0x200000, CRC(8e011d9b) SHA1(e0861bcd3c4f865474d7ce47aa9eeec7b3d28da6) ) // sldh
	ROM_REGION( 0x200000, "simm1.3", 0 )
	ROM_LOAD( "sfiii3-simm1.3", 0x00000, 0x200000, CRC(dca8d92f) SHA1(7cd241641c943df446e2c75b88b5cf2d2ebf7b2e) ) // sldh

	ROM_REGION( 0x200000, "simm2.0", 0 )
	ROM_LOAD( "sfiii3-simm2.0", 0x00000, 0x200000, CRC(06eb969e) SHA1(d89f6a6585b76692d57d337f0f8186398fb056da) )
	ROM_REGION( 0x200000, "simm2.1", 0 )
	ROM_LOAD( "sfiii3-simm2.1", 0x00000, 0x200000, CRC(e7039f82) SHA1(8e81e66b5a4f45ae14b070a491bde47a6a74499f) )
	ROM_REGION( 0x200000, "simm2.2", 0 )
	ROM_LOAD( "sfiii3-simm2.2", 0x00000, 0x200000, CRC(645c96f7) SHA1(06d5a54874d4bf100b776131ec9060da209ad037) )
	ROM_REGION( 0x200000, "simm2.3", 0 )
	ROM_LOAD( "sfiii3-simm2.3", 0x00000, 0x200000, CRC(610efab1) SHA1(bbc21ed6ff6220ff6017a3f02ebd9a341fbc9040) )

	ROM_REGION( 0x200000, "simm3.0", 0 )
	ROM_LOAD( "sfiii3-simm3.0", 0x00000, 0x200000, CRC(7baa1f79) SHA1(3f409df28c24dd7221966b5340d59898ea756b6f) )
	ROM_REGION( 0x200000, "simm3.1", 0 )
	ROM_LOAD( "sfiii3-simm3.1", 0x00000, 0x200000, CRC(234bf8fe) SHA1(2191781ae4d726cab28de97f27efa4a13f3bdd69) )
	ROM_REGION( 0x200000, "simm3.2", 0 )
	ROM_LOAD( "sfiii3-simm3.2", 0x00000, 0x200000, CRC(d9ebc308) SHA1(af6a0dca77e5181c9f20533a06760a782c5fd51d) )
	ROM_REGION( 0x200000, "simm3.3", 0 )
	ROM_LOAD( "sfiii3-simm3.3", 0x00000, 0x200000, CRC(293cba77) SHA1(294604cacdc24261aec4d39e489de91c41fa1758) )
	ROM_REGION( 0x200000, "simm3.4", 0 )
	ROM_LOAD( "sfiii3-simm3.4", 0x00000, 0x200000, CRC(6055e747) SHA1(3813852c5a4a5355ef739ca8f0913bbd390b984b) )
	ROM_REGION( 0x200000, "simm3.5", 0 )
	ROM_LOAD( "sfiii3-simm3.5", 0x00000, 0x200000, CRC(499aa6fc) SHA1(5b9b6eab3e99ff3e1d7c1f50b9d8bc6a81f3f8a9) )
	ROM_REGION( 0x200000, "simm3.6", 0 )
	ROM_LOAD( "sfiii3-simm3.6", 0x00000, 0x200000, CRC(6c13879e) SHA1(de189b0b8f42bc7dd89983e62bc2ecb4237b3277) )
	ROM_REGION( 0x200000, "simm3.7", 0 )
	ROM_LOAD( "sfiii3-simm3.7", 0x00000, 0x200000, CRC(cf4f8ede) SHA1(e0fb68fcb0e445f824c62fa828d6e1dcd7e3683a) )

	ROM_REGION( 0x200000, "simm4.0", 0 )
	ROM_LOAD( "sfiii3-simm4.0", 0x00000, 0x200000, CRC(091fd5ba) SHA1(3327ad7c2623c119bf728af717ea2ce3b74673a9) )
	ROM_REGION( 0x200000, "simm4.1", 0 )
	ROM_LOAD( "sfiii3-simm4.1", 0x00000, 0x200000, CRC(0bca8917) SHA1(b7b284e2f16f46d46bcfaae779b232c5b980924f) )
	ROM_REGION( 0x200000, "simm4.2", 0 )
	ROM_LOAD( "sfiii3-simm4.2", 0x00000, 0x200000, CRC(a0fd578b) SHA1(100c9db9f00ecd88d518076f5a0822e6ac3695b3) )
	ROM_REGION( 0x200000, "simm4.3", 0 )
	ROM_LOAD( "sfiii3-simm4.3", 0x00000, 0x200000, CRC(4bf8c699) SHA1(2c0b4288b5ebc5e54d9e782dfc39eb8c78fd4c21) )
	ROM_REGION( 0x200000, "simm4.4", 0 )
	ROM_LOAD( "sfiii3-simm4.4", 0x00000, 0x200000, CRC(137b8785) SHA1(56a579520a8ce2abbf36be57777f024e80474eee) )
	ROM_REGION( 0x200000, "simm4.5", 0 )
	ROM_LOAD( "sfiii3-simm4.5", 0x00000, 0x200000, CRC(4fb70671) SHA1(9aba83c18cfc099a5ce18793119bff0c2b9c777f) )
	ROM_REGION( 0x200000, "simm4.6", 0 )
	ROM_LOAD( "sfiii3-simm4.6", 0x00000, 0x200000, CRC(832374a4) SHA1(c84629e32fbf47cb7b5b4ee7555bfc2ac9b3857f) )
	ROM_REGION( 0x200000, "simm4.7", 0 )
	ROM_LOAD( "sfiii3-simm4.7", 0x00000, 0x200000, CRC(1c88576d) SHA1(0f039944d0c2305999ed5dbd351c3eb87812dc3b) )

	ROM_REGION( 0x200000, "simm5.0", 0 )
	ROM_LOAD( "sfiii3-simm5.0", 0x00000, 0x200000, CRC(c67d9190) SHA1(d265475244099d0ec153059986f3445c7bd910a3) )
	ROM_REGION( 0x200000, "simm5.1", 0 )
	ROM_LOAD( "sfiii3-simm5.1", 0x00000, 0x200000, CRC(6cb79868) SHA1(c94237f30e05bfcb2e23945530c812d9e4c73416) )
	ROM_REGION( 0x200000, "simm5.2", 0 )
	ROM_LOAD( "sfiii3-simm5.2", 0x00000, 0x200000, CRC(df69930e) SHA1(c76b7c559a1d5558138afbc796249efa2f49f6a8) )
	ROM_REGION( 0x200000, "simm5.3", 0 )
	ROM_LOAD( "sfiii3-simm5.3", 0x00000, 0x200000, CRC(333754e0) SHA1(4c18a569c26524a492ecd6f4c8b3c8e803a077d3) )
	ROM_REGION( 0x200000, "simm5.4", 0 )
	ROM_LOAD( "sfiii3-simm5.4", 0x00000, 0x200000, CRC(78f6d417) SHA1(a69577cc5399fcf0a24548661168f27f3e7e8e40) )
	ROM_REGION( 0x200000, "simm5.5", 0 )
	ROM_LOAD( "sfiii3-simm5.5", 0x00000, 0x200000, CRC(8ccad9b1) SHA1(f8bda399f87be2497b7ac39e9661f9863bf4f873) )
	ROM_REGION( 0x200000, "simm5.6", 0 )
	ROM_LOAD( "sfiii3-simm5.6", 0x00000, 0x200000, CRC(85de59e5) SHA1(748b5c91f15777b85d8c1d35b685cd90d3185ec6) )
	ROM_REGION( 0x200000, "simm5.7", 0 )
	ROM_LOAD( "sfiii3-simm5.7", 0x00000, 0x200000, CRC(ee7e29b3) SHA1(63dc30c6904ca2f58d229249bee5eef51fafa158) )

	ROM_REGION( 0x200000, "simm6.0", 0 )
	ROM_LOAD( "sfiii3-simm6.0", 0x00000, 0x200000, CRC(8da69042) SHA1(fd3d08295342635b2136e48d543c9350d287bb22) )
	ROM_REGION( 0x200000, "simm6.1", 0 )
	ROM_LOAD( "sfiii3-simm6.1", 0x00000, 0x200000, CRC(1c8c7ac4) SHA1(ac9f8353a4c356ef98aa7c226baba00b01f5a80f) )
	ROM_REGION( 0x200000, "simm6.2", 0 )
	ROM_LOAD( "sfiii3-simm6.2", 0x00000, 0x200000, CRC(a671341d) SHA1(636f4c04962bc1e1ddb29d2e01244b00389b234f) )
	ROM_REGION( 0x200000, "simm6.3", 0 )
	ROM_LOAD( "sfiii3-simm6.3", 0x00000, 0x200000, CRC(1a990249) SHA1(2acc639e2c0c53bf24096b8620eab090bc25d03b) )
	ROM_REGION( 0x200000, "simm6.4", 0 )
	ROM_LOAD( "sfiii3-simm6.4", 0x00000, 0x200000, CRC(20cb39ac) SHA1(7d13a0fea1ef719dd2ff77dfb547d53c6023cc9e) )
	ROM_REGION( 0x200000, "simm6.5", 0 )
	ROM_LOAD( "sfiii3-simm6.5", 0x00000, 0x200000, CRC(5f844b2f) SHA1(564e4934f89ed3b92a4c4874519f8f00f3b48696) )
	ROM_REGION( 0x200000, "simm6.6", 0 )
	ROM_LOAD( "sfiii3-simm6.6", 0x00000, 0x200000, CRC(450e8d28) SHA1(885db658132aa27926df617ec2d2a1f38abdbb60) )
	ROM_REGION( 0x200000, "simm6.7", 0 )
	ROM_LOAD( "sfiii3-simm6.7", 0x00000, 0x200000, CRC(cc5f4187) SHA1(248ddace21ed4736a56e92f77cc6ad219d7fef0b) )
ROM_END

ROM_START( sfiii3nr1 )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "sfiii3_japan_nocd.29f400.u2", 0x000000, 0x080000, CRC(1edc6366) SHA1(60b4b9adeb030a33059d74fdf03873029e465b52) )

	ROM_REGION( 0x200000, "simm1.0", 0 )
	ROM_LOAD( "sfiii3-simm1.0", 0x00000, 0x200000, CRC(66e66235) SHA1(0a98038721d176458d4f85dbd76c5edb93a65322) ) // sldh
	ROM_REGION( 0x200000, "simm1.1", 0 )
	ROM_LOAD( "sfiii3-simm1.1", 0x00000, 0x200000, CRC(186e8c5f) SHA1(a63040201a660b56217a8cbab32f5c2c466ee5dd) ) // sldh
	ROM_REGION( 0x200000, "simm1.2", 0 )
	ROM_LOAD( "sfiii3-simm1.2", 0x00000, 0x200000, CRC(bce18cab) SHA1(a5c28063d98c22403756fc926a20631456fb7dcc) ) // sldh
	ROM_REGION( 0x200000, "simm1.3", 0 )
	ROM_LOAD( "sfiii3-simm1.3", 0x00000, 0x200000, CRC(129dc2c9) SHA1(c1e634d94b1c8f7f02a47703622de5cab3d0da3f) ) // sldh

	ROM_REGION( 0x200000, "simm2.0", 0 )
	ROM_LOAD( "sfiii3-simm2.0",  0x00000, 0x200000, CRC(06eb969e) SHA1(d89f6a6585b76692d57d337f0f8186398fb056da) )
	ROM_REGION( 0x200000, "simm2.1", 0 )
	ROM_LOAD( "sfiii3-simm2.1",  0x00000, 0x200000, CRC(e7039f82) SHA1(8e81e66b5a4f45ae14b070a491bde47a6a74499f) )
	ROM_REGION( 0x200000, "simm2.2", 0 )
	ROM_LOAD( "sfiii3-simm2.2",  0x00000, 0x200000, CRC(645c96f7) SHA1(06d5a54874d4bf100b776131ec9060da209ad037) )
	ROM_REGION( 0x200000, "simm2.3", 0 )
	ROM_LOAD( "sfiii3-simm2.3",  0x00000, 0x200000, CRC(610efab1) SHA1(bbc21ed6ff6220ff6017a3f02ebd9a341fbc9040) )

	ROM_REGION( 0x200000, "simm3.0", 0 )
	ROM_LOAD( "sfiii3-simm3.0",  0x00000, 0x200000, CRC(7baa1f79) SHA1(3f409df28c24dd7221966b5340d59898ea756b6f) )
	ROM_REGION( 0x200000, "simm3.1", 0 )
	ROM_LOAD( "sfiii3-simm3.1",  0x00000, 0x200000, CRC(234bf8fe) SHA1(2191781ae4d726cab28de97f27efa4a13f3bdd69) )
	ROM_REGION( 0x200000, "simm3.2", 0 )
	ROM_LOAD( "sfiii3-simm3.2",  0x00000, 0x200000, CRC(d9ebc308) SHA1(af6a0dca77e5181c9f20533a06760a782c5fd51d) )
	ROM_REGION( 0x200000, "simm3.3", 0 )
	ROM_LOAD( "sfiii3-simm3.3",  0x00000, 0x200000, CRC(293cba77) SHA1(294604cacdc24261aec4d39e489de91c41fa1758) )
	ROM_REGION( 0x200000, "simm3.4", 0 )
	ROM_LOAD( "sfiii3-simm3.4",  0x00000, 0x200000, CRC(6055e747) SHA1(3813852c5a4a5355ef739ca8f0913bbd390b984b) )
	ROM_REGION( 0x200000, "simm3.5", 0 )
	ROM_LOAD( "sfiii3-simm3.5",  0x00000, 0x200000, CRC(499aa6fc) SHA1(5b9b6eab3e99ff3e1d7c1f50b9d8bc6a81f3f8a9) )
	ROM_REGION( 0x200000, "simm3.6", 0 )
	ROM_LOAD( "sfiii3-simm3.6",  0x00000, 0x200000, CRC(6c13879e) SHA1(de189b0b8f42bc7dd89983e62bc2ecb4237b3277) )
	ROM_REGION( 0x200000, "simm3.7", 0 )
	ROM_LOAD( "sfiii3-simm3.7",  0x00000, 0x200000, CRC(cf4f8ede) SHA1(e0fb68fcb0e445f824c62fa828d6e1dcd7e3683a) )

	ROM_REGION( 0x200000, "simm4.0", 0 )
	ROM_LOAD( "sfiii3-simm4.0",  0x00000, 0x200000, CRC(091fd5ba) SHA1(3327ad7c2623c119bf728af717ea2ce3b74673a9) )
	ROM_REGION( 0x200000, "simm4.1", 0 )
	ROM_LOAD( "sfiii3-simm4.1",  0x00000, 0x200000, CRC(0bca8917) SHA1(b7b284e2f16f46d46bcfaae779b232c5b980924f) )
	ROM_REGION( 0x200000, "simm4.2", 0 )
	ROM_LOAD( "sfiii3-simm4.2",  0x00000, 0x200000, CRC(a0fd578b) SHA1(100c9db9f00ecd88d518076f5a0822e6ac3695b3) )
	ROM_REGION( 0x200000, "simm4.3", 0 )
	ROM_LOAD( "sfiii3-simm4.3",  0x00000, 0x200000, CRC(4bf8c699) SHA1(2c0b4288b5ebc5e54d9e782dfc39eb8c78fd4c21) )
	ROM_REGION( 0x200000, "simm4.4", 0 )
	ROM_LOAD( "sfiii3-simm4.4",  0x00000, 0x200000, CRC(137b8785) SHA1(56a579520a8ce2abbf36be57777f024e80474eee) )
	ROM_REGION( 0x200000, "simm4.5", 0 )
	ROM_LOAD( "sfiii3-simm4.5",  0x00000, 0x200000, CRC(4fb70671) SHA1(9aba83c18cfc099a5ce18793119bff0c2b9c777f) )
	ROM_REGION( 0x200000, "simm4.6", 0 )
	ROM_LOAD( "sfiii3-simm4.6",  0x00000, 0x200000, CRC(832374a4) SHA1(c84629e32fbf47cb7b5b4ee7555bfc2ac9b3857f) )
	ROM_REGION( 0x200000, "simm4.7", 0 )
	ROM_LOAD( "sfiii3-simm4.7",  0x00000, 0x200000, CRC(1c88576d) SHA1(0f039944d0c2305999ed5dbd351c3eb87812dc3b) )

	ROM_REGION( 0x200000, "simm5.0", 0 )
	ROM_LOAD( "sfiii3-simm5.0",  0x00000, 0x200000, CRC(c67d9190) SHA1(d265475244099d0ec153059986f3445c7bd910a3) )
	ROM_REGION( 0x200000, "simm5.1", 0 )
	ROM_LOAD( "sfiii3-simm5.1",  0x00000, 0x200000, CRC(6cb79868) SHA1(c94237f30e05bfcb2e23945530c812d9e4c73416) )
	ROM_REGION( 0x200000, "simm5.2", 0 )
	ROM_LOAD( "sfiii3-simm5.2",  0x00000, 0x200000, CRC(df69930e) SHA1(c76b7c559a1d5558138afbc796249efa2f49f6a8) )
	ROM_REGION( 0x200000, "simm5.3", 0 )
	ROM_LOAD( "sfiii3-simm5.3",  0x00000, 0x200000, CRC(333754e0) SHA1(4c18a569c26524a492ecd6f4c8b3c8e803a077d3) )
	ROM_REGION( 0x200000, "simm5.4", 0 )
	ROM_LOAD( "sfiii3-simm5.4",  0x00000, 0x200000, CRC(78f6d417) SHA1(a69577cc5399fcf0a24548661168f27f3e7e8e40) )
	ROM_REGION( 0x200000, "simm5.5", 0 )
	ROM_LOAD( "sfiii3-simm5.5",  0x00000, 0x200000, CRC(8ccad9b1) SHA1(f8bda399f87be2497b7ac39e9661f9863bf4f873) )
	ROM_REGION( 0x200000, "simm5.6", 0 )
	ROM_LOAD( "sfiii3-simm5.6",  0x00000, 0x200000, CRC(85de59e5) SHA1(748b5c91f15777b85d8c1d35b685cd90d3185ec6) )
	ROM_REGION( 0x200000, "simm5.7", 0 )
	ROM_LOAD( "sfiii3-simm5.7",  0x00000, 0x200000, CRC(ee7e29b3) SHA1(63dc30c6904ca2f58d229249bee5eef51fafa158) )

	ROM_REGION( 0x200000, "simm6.0", 0 )
	ROM_LOAD( "sfiii3-simm6.0",  0x00000, 0x200000, CRC(8da69042) SHA1(fd3d08295342635b2136e48d543c9350d287bb22) )
	ROM_REGION( 0x200000, "simm6.1", 0 )
	ROM_LOAD( "sfiii3-simm6.1",  0x00000, 0x200000, CRC(1c8c7ac4) SHA1(ac9f8353a4c356ef98aa7c226baba00b01f5a80f) )
	ROM_REGION( 0x200000, "simm6.2", 0 )
	ROM_LOAD( "sfiii3-simm6.2",  0x00000, 0x200000, CRC(a671341d) SHA1(636f4c04962bc1e1ddb29d2e01244b00389b234f) )
	ROM_REGION( 0x200000, "simm6.3", 0 )
	ROM_LOAD( "sfiii3-simm6.3",  0x00000, 0x200000, CRC(1a990249) SHA1(2acc639e2c0c53bf24096b8620eab090bc25d03b) )
	ROM_REGION( 0x200000, "simm6.4", 0 )
	ROM_LOAD( "sfiii3-simm6.4",  0x00000, 0x200000, CRC(20cb39ac) SHA1(7d13a0fea1ef719dd2ff77dfb547d53c6023cc9e) )
	ROM_REGION( 0x200000, "simm6.5", 0 )
	ROM_LOAD( "sfiii3-simm6.5",  0x00000, 0x200000, CRC(5f844b2f) SHA1(564e4934f89ed3b92a4c4874519f8f00f3b48696) )
	ROM_REGION( 0x200000, "simm6.6", 0 )
	ROM_LOAD( "sfiii3-simm6.6",  0x00000, 0x200000, CRC(450e8d28) SHA1(885db658132aa27926df617ec2d2a1f38abdbb60) )
	ROM_REGION( 0x200000, "simm6.7", 0 )
	ROM_LOAD( "sfiii3-simm6.7",  0x00000, 0x200000, CRC(cc5f4187) SHA1(248ddace21ed4736a56e92f77cc6ad219d7fef0b) )
ROM_END

ROM_START( jojoban )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "jojoba_japan_nocd.29f400.u2", 0x000000, 0x080000, CRC(4dab19f5) SHA1(ba07190e7662937fc267f07285c51e99a45c061e) )

	ROM_REGION( 0x200000, "simm1.0", 0 )
	ROM_LOAD( "jojoba-simm1.0", 0x00000, 0x200000, CRC(adcd8377) SHA1(f1aacbe061e3bcade5cca34435c3f86aec5f1499) ) // sldh
	ROM_REGION( 0x200000, "simm1.1", 0 )
	ROM_LOAD( "jojoba-simm1.1", 0x00000, 0x200000, CRC(d7590b59) SHA1(bfee627ebb7cb7b28216527b17e1b06a4e6f19f4) ) // sldh
	ROM_REGION( 0x200000, "simm1.2", 0 )
	ROM_LOAD( "jojoba-simm1.2", 0x00000, 0x200000, CRC(e62e240b) SHA1(70468cae67c009a80b45954c2a30794577343c77) ) // sldh
	ROM_REGION( 0x200000, "simm1.3", 0 )
	ROM_LOAD( "jojoba-simm1.3", 0x00000, 0x200000, CRC(c95450c3) SHA1(55616e009b007180d1ac6290c8da44b0d864a494) ) // sldh

	ROM_REGION( 0x200000, "simm2.0", 0 )
	ROM_LOAD( "jojoba-simm2.0", 0x00000, 0x200000, CRC(535f2eba) SHA1(167bec0dccfc2f91cb10cb1e2631ee619b3eb9fe) ) // sldh
	ROM_REGION( 0x200000, "simm2.1", 0 )
	ROM_LOAD( "jojoba-simm2.1", 0x00000, 0x200000, CRC(01dd3a01) SHA1(08c462219796baa3ec28d78d038a18187cd838bb) ) // sldh
	ROM_REGION( 0x200000, "simm2.2", 0 )
	ROM_LOAD( "jojoba-simm2.2", 0x00000, 0x200000, CRC(61432672) SHA1(d0416a75d395926041f90a3d34edb96a080acfd6) ) // sldh
	ROM_REGION( 0x200000, "simm2.3", 0 )
	ROM_LOAD( "jojoba-simm2.3", 0x00000, 0x200000, CRC(acdc9aca) SHA1(89f77ddd6286709182a676fd9bd6c333a3b16271) ) // sldh

	ROM_REGION( 0x200000, "simm3.0", 0 )
	ROM_LOAD( "jojoba-simm3.0", 0x00000, 0x200000, CRC(4d16e111) SHA1(f198007375be65e89856d64ee2b3857a18b4eab8) )
	ROM_REGION( 0x200000, "simm3.1", 0 )
	ROM_LOAD( "jojoba-simm3.1", 0x00000, 0x200000, CRC(9b3406d3) SHA1(54e90cd334d13e2c74305c6b87ebce1365ef3d59) )
	ROM_REGION( 0x200000, "simm3.2", 0 )
	ROM_LOAD( "jojoba-simm3.2", 0x00000, 0x200000, CRC(f2414997) SHA1(fb89d5784250538ad17fd527267b513afb6eca20) )
	ROM_REGION( 0x200000, "simm3.3", 0 )
	ROM_LOAD( "jojoba-simm3.3", 0x00000, 0x200000, CRC(954b9c7d) SHA1(0d64d97167d4e669d7e4f3a388f9d5ec1e18ed42) )
	ROM_REGION( 0x200000, "simm3.4", 0 )
	ROM_LOAD( "jojoba-simm3.4", 0x00000, 0x200000, CRC(625adc1d) SHA1(533d62759ecece10c711d99bfca403e5cba279b5) )
	ROM_REGION( 0x200000, "simm3.5", 0 )
	ROM_LOAD( "jojoba-simm3.5", 0x00000, 0x200000, CRC(20a70bb4) SHA1(3bd8376304ffc974fb8031eac8bebff27969538c) )
	ROM_REGION( 0x200000, "simm3.6", 0 )
	ROM_LOAD( "jojoba-simm3.6", 0x00000, 0x200000, CRC(a10ec5af) SHA1(9b403260e8fbdacaa5369ab79fc05855cc6a6bdb) )
	ROM_REGION( 0x200000, "simm3.7", 0 )
	ROM_LOAD( "jojoba-simm3.7", 0x00000, 0x200000, CRC(0bd0de7a) SHA1(1debecda5f282f2a1dd17e887e522a4d00c5dc9d) )

	ROM_REGION( 0x200000, "simm4.0", 0 )
	ROM_LOAD( "jojoba-simm4.0", 0x00000, 0x200000, CRC(6ea14adc) SHA1(696b2ec66f3c197817a60f507a1b4c78db37f488) )
	ROM_REGION( 0x200000, "simm4.1", 0 )
	ROM_LOAD( "jojoba-simm4.1", 0x00000, 0x200000, CRC(8f4c42fb) SHA1(363d769b0b066ce139125426d2da6dfa15d1eb28) )
	ROM_REGION( 0x200000, "simm4.2", 0 )
	ROM_LOAD( "jojoba-simm4.2", 0x00000, 0x200000, CRC(ef0586d1) SHA1(8fcc350da20e3e59fa76fa14e10f2c47233ba9dc) )
	ROM_REGION( 0x200000, "simm4.3", 0 )
	ROM_LOAD( "jojoba-simm4.3", 0x00000, 0x200000, CRC(93ccc470) SHA1(5d267679e61c0fb592ad5f696d3c06ec1746d0b3) )
	ROM_REGION( 0x200000, "simm4.4", 0 )
	ROM_LOAD( "jojoba-simm4.4", 0x00000, 0x200000, CRC(3d9ec7d2) SHA1(665b867bab928be183c2006527e55f9b8ec4a271) )
	ROM_REGION( 0x200000, "simm4.5", 0 )
	ROM_LOAD( "jojoba-simm4.5", 0x00000, 0x200000, CRC(03e66850) SHA1(8478662dc9db20d9a186d315a883bd1cbb5e5000) )
	ROM_REGION( 0x200000, "simm4.6", 0 )
	ROM_LOAD( "jojoba-simm4.6", 0x00000, 0x200000, CRC(01606ac3) SHA1(ccc74edeca6abdd86fc1cf42ececa1ea393b3261) )
	ROM_REGION( 0x200000, "simm4.7", 0 )
	ROM_LOAD( "jojoba-simm4.7", 0x00000, 0x200000, CRC(36392b87) SHA1(e62080c8461775c1e180400dfb44414679fd0fc1) )

	ROM_REGION( 0x200000, "simm5.0", 0 )
	ROM_LOAD( "jojoba-simm5.0", 0x00000, 0x200000, CRC(2ef8c60c) SHA1(dea87a73a11b8edd27c3c9c5ab2af295cb5508f9) )
	ROM_REGION( 0x200000, "simm5.1", 0 )
	ROM_LOAD( "jojoba-simm5.1", 0x00000, 0x200000, CRC(cf7d7ca6) SHA1(b347707b1e5bc71d28b282273f893592e5f9e333) )
	ROM_REGION( 0x200000, "simm5.2", 0 )
	ROM_LOAD( "jojoba-simm5.2", 0x00000, 0x200000, CRC(b7815bfa) SHA1(0b5a3a2ffe1b3c0ca765dcedc297e78e5928302b) )
	ROM_REGION( 0x200000, "simm5.3", 0 )
	ROM_LOAD( "jojoba-simm5.3", 0x00000, 0x200000, CRC(9bfec049) SHA1(62cc9a1920047863205544b77344ee18f310f084) )
	ROM_REGION( 0x200000, "simm5.4", 0 )
	ROM_LOAD( "jojoba-simm5.4", 0x00000, 0x200000, CRC(d167536b) SHA1(e2637d3486f168ce44e0a00413d38960cb86db4c) )
	ROM_REGION( 0x200000, "simm5.5", 0 )
	ROM_LOAD( "jojoba-simm5.5", 0x00000, 0x200000, CRC(55e7a042) SHA1(c18bda61fa005d9174a27b7b7d324004262a4525) )
	ROM_REGION( 0x200000, "simm5.6", 0 )
	ROM_LOAD( "jojoba-simm5.6", 0x00000, 0x200000, CRC(4fb32906) SHA1(3a5965b3197517932c8aa4c07a6ea6a190a338d7) )
	ROM_REGION( 0x200000, "simm5.7", 0 )
	ROM_LOAD( "jojoba-simm5.7", 0x00000, 0x200000, CRC(8c8be520) SHA1(c461f3f76a83592b36b29afb316679a7c8972404) )
ROM_END

ROM_START( jojobanr1 )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "jojoba_japan_nocd.29f400.u2", 0x000000, 0x080000, CRC(4dab19f5) SHA1(ba07190e7662937fc267f07285c51e99a45c061e) )

	ROM_REGION( 0x200000, "simm1.0", 0 )
	ROM_LOAD( "jojoba-simm1.0", 0x00000, 0x200000, CRC(76976231) SHA1(90adde7e5983ec6a4e02789d5cefe9e85c9c52d5) ) // sldh
	ROM_REGION( 0x200000, "simm1.1", 0 )
	ROM_LOAD( "jojoba-simm1.1", 0x00000, 0x200000, CRC(cedd78e7) SHA1(964988b90a2f14c1da2cfc48d943e16e54da3fd3) ) // sldh
	ROM_REGION( 0x200000, "simm1.2", 0 )
	ROM_LOAD( "jojoba-simm1.2", 0x00000, 0x200000, CRC(2955b77f) SHA1(2a907a5cd91448bfc420c318584e5ef4bbe55a91) ) // sldh
	ROM_REGION( 0x200000, "simm1.3", 0 )
	ROM_LOAD( "jojoba-simm1.3", 0x00000, 0x200000, CRC(280139d7) SHA1(b7c28f6f0218688fb873a3106d2f95ea2e1e927c) ) // sldh

	ROM_REGION( 0x200000, "simm2.0", 0 )
	ROM_LOAD( "jojoba-simm2.0", 0x00000, 0x200000, CRC(305c4914) SHA1(c3a73ffe58f61ab8f1cd9e3f0891037638dc5a9b) ) // sldh
	ROM_REGION( 0x200000, "simm2.1", 0 )
	ROM_LOAD( "jojoba-simm2.1", 0x00000, 0x200000, CRC(18af4f3b) SHA1(04b8fdf23a782b10c203b111cc634a6d3474044a) ) // sldh
	ROM_REGION( 0x200000, "simm2.2", 0 )
	ROM_LOAD( "jojoba-simm2.2", 0x00000, 0x200000, CRC(397e5c9e) SHA1(021d86ee66bf951fb6a1dd90fb7007c6865cbb8b) ) // sldh
	ROM_REGION( 0x200000, "simm2.3", 0 )
	ROM_LOAD( "jojoba-simm2.3", 0x00000, 0x200000, CRC(a9d0a7d7) SHA1(b2cfc0661f8903ddbeea8a604ee8b42097e10ab8) ) // sldh

	ROM_REGION( 0x200000, "simm3.0", 0 )
	ROM_LOAD( "jojoba-simm3.0",  0x00000, 0x200000, CRC(4d16e111) SHA1(f198007375be65e89856d64ee2b3857a18b4eab8) )
	ROM_REGION( 0x200000, "simm3.1", 0 )
	ROM_LOAD( "jojoba-simm3.1",  0x00000, 0x200000, CRC(9b3406d3) SHA1(54e90cd334d13e2c74305c6b87ebce1365ef3d59) )
	ROM_REGION( 0x200000, "simm3.2", 0 )
	ROM_LOAD( "jojoba-simm3.2",  0x00000, 0x200000, CRC(f2414997) SHA1(fb89d5784250538ad17fd527267b513afb6eca20) )
	ROM_REGION( 0x200000, "simm3.3", 0 )
	ROM_LOAD( "jojoba-simm3.3",  0x00000, 0x200000, CRC(954b9c7d) SHA1(0d64d97167d4e669d7e4f3a388f9d5ec1e18ed42) )
	ROM_REGION( 0x200000, "simm3.4", 0 )
	ROM_LOAD( "jojoba-simm3.4",  0x00000, 0x200000, CRC(625adc1d) SHA1(533d62759ecece10c711d99bfca403e5cba279b5) )
	ROM_REGION( 0x200000, "simm3.5", 0 )
	ROM_LOAD( "jojoba-simm3.5",  0x00000, 0x200000, CRC(20a70bb4) SHA1(3bd8376304ffc974fb8031eac8bebff27969538c) )
	ROM_REGION( 0x200000, "simm3.6", 0 )
	ROM_LOAD( "jojoba-simm3.6",  0x00000, 0x200000, CRC(a10ec5af) SHA1(9b403260e8fbdacaa5369ab79fc05855cc6a6bdb) )
	ROM_REGION( 0x200000, "simm3.7", 0 )
	ROM_LOAD( "jojoba-simm3.7",  0x00000, 0x200000, CRC(0bd0de7a) SHA1(1debecda5f282f2a1dd17e887e522a4d00c5dc9d) )

	ROM_REGION( 0x200000, "simm4.0", 0 )
	ROM_LOAD( "jojoba-simm4.0",  0x00000, 0x200000, CRC(6ea14adc) SHA1(696b2ec66f3c197817a60f507a1b4c78db37f488) )
	ROM_REGION( 0x200000, "simm4.1", 0 )
	ROM_LOAD( "jojoba-simm4.1",  0x00000, 0x200000, CRC(8f4c42fb) SHA1(363d769b0b066ce139125426d2da6dfa15d1eb28) )
	ROM_REGION( 0x200000, "simm4.2", 0 )
	ROM_LOAD( "jojoba-simm4.2",  0x00000, 0x200000, CRC(ef0586d1) SHA1(8fcc350da20e3e59fa76fa14e10f2c47233ba9dc) )
	ROM_REGION( 0x200000, "simm4.3", 0 )
	ROM_LOAD( "jojoba-simm4.3",  0x00000, 0x200000, CRC(93ccc470) SHA1(5d267679e61c0fb592ad5f696d3c06ec1746d0b3) )
	ROM_REGION( 0x200000, "simm4.4", 0 )
	ROM_LOAD( "jojoba-simm4.4",  0x00000, 0x200000, CRC(3d9ec7d2) SHA1(665b867bab928be183c2006527e55f9b8ec4a271) )
	ROM_REGION( 0x200000, "simm4.5", 0 )
	ROM_LOAD( "jojoba-simm4.5",  0x00000, 0x200000, CRC(03e66850) SHA1(8478662dc9db20d9a186d315a883bd1cbb5e5000) )
	ROM_REGION( 0x200000, "simm4.6", 0 )
	ROM_LOAD( "jojoba-simm4.6",  0x00000, 0x200000, CRC(01606ac3) SHA1(ccc74edeca6abdd86fc1cf42ececa1ea393b3261) )
	ROM_REGION( 0x200000, "simm4.7", 0 )
	ROM_LOAD( "jojoba-simm4.7",  0x00000, 0x200000, CRC(36392b87) SHA1(e62080c8461775c1e180400dfb44414679fd0fc1) )

	ROM_REGION( 0x200000, "simm5.0", 0 )
	ROM_LOAD( "jojoba-simm5.0",  0x00000, 0x200000, CRC(2ef8c60c) SHA1(dea87a73a11b8edd27c3c9c5ab2af295cb5508f9) )
	ROM_REGION( 0x200000, "simm5.1", 0 )
	ROM_LOAD( "jojoba-simm5.1",  0x00000, 0x200000, CRC(cf7d7ca6) SHA1(b347707b1e5bc71d28b282273f893592e5f9e333) )
	ROM_REGION( 0x200000, "simm5.2", 0 )
	ROM_LOAD( "jojoba-simm5.2",  0x00000, 0x200000, CRC(b7815bfa) SHA1(0b5a3a2ffe1b3c0ca765dcedc297e78e5928302b) )
	ROM_REGION( 0x200000, "simm5.3", 0 )
	ROM_LOAD( "jojoba-simm5.3",  0x00000, 0x200000, CRC(9bfec049) SHA1(62cc9a1920047863205544b77344ee18f310f084) )
	ROM_REGION( 0x200000, "simm5.4", 0 )
	ROM_LOAD( "jojoba-simm5.4",  0x00000, 0x200000, CRC(d167536b) SHA1(e2637d3486f168ce44e0a00413d38960cb86db4c) )
	ROM_REGION( 0x200000, "simm5.5", 0 )
	ROM_LOAD( "jojoba-simm5.5",  0x00000, 0x200000, CRC(55e7a042) SHA1(c18bda61fa005d9174a27b7b7d324004262a4525) )
	ROM_REGION( 0x200000, "simm5.6", 0 )
	ROM_LOAD( "jojoba-simm5.6",  0x00000, 0x200000, CRC(4fb32906) SHA1(3a5965b3197517932c8aa4c07a6ea6a190a338d7) )
	ROM_REGION( 0x200000, "simm5.7", 0 )
	ROM_LOAD( "jojoba-simm5.7",  0x00000, 0x200000, CRC(8c8be520) SHA1(c461f3f76a83592b36b29afb316679a7c8972404) )
ROM_END

ROM_START( jojobane )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "jojoba_euro_nocd.29f400.u2", 0x000000, 0x080000, CRC(1ee2d679) SHA1(9e129b454a376606b3f7e8aec64de425cf9c635c) )

	ROM_REGION( 0x200000, "simm1.0", 0 )
	ROM_LOAD( "jojoba-simm1.0", 0x00000, 0x200000, CRC(adcd8377) SHA1(f1aacbe061e3bcade5cca34435c3f86aec5f1499) ) // sldh
	ROM_REGION( 0x200000, "simm1.1", 0 )
	ROM_LOAD( "jojoba-simm1.1", 0x00000, 0x200000, CRC(d7590b59) SHA1(bfee627ebb7cb7b28216527b17e1b06a4e6f19f4) ) // sldh
	ROM_REGION( 0x200000, "simm1.2", 0 )
	ROM_LOAD( "jojoba-simm1.2", 0x00000, 0x200000, CRC(e62e240b) SHA1(70468cae67c009a80b45954c2a30794577343c77) ) // sldh
	ROM_REGION( 0x200000, "simm1.3", 0 )
	ROM_LOAD( "jojoba-simm1.3", 0x00000, 0x200000, CRC(c95450c3) SHA1(55616e009b007180d1ac6290c8da44b0d864a494) ) // sldh

	ROM_REGION( 0x200000, "simm2.0", 0 )
	ROM_LOAD( "jojoba-simm2.0", 0x00000, 0x200000, CRC(535f2eba) SHA1(167bec0dccfc2f91cb10cb1e2631ee619b3eb9fe) ) // sldh
	ROM_REGION( 0x200000, "simm2.1", 0 )
	ROM_LOAD( "jojoba-simm2.1", 0x00000, 0x200000, CRC(01dd3a01) SHA1(08c462219796baa3ec28d78d038a18187cd838bb) ) // sldh
	ROM_REGION( 0x200000, "simm2.2", 0 )
	ROM_LOAD( "jojoba-simm2.2", 0x00000, 0x200000, CRC(61432672) SHA1(d0416a75d395926041f90a3d34edb96a080acfd6) ) // sldh
	ROM_REGION( 0x200000, "simm2.3", 0 )
	ROM_LOAD( "jojoba-simm2.3", 0x00000, 0x200000, CRC(acdc9aca) SHA1(89f77ddd6286709182a676fd9bd6c333a3b16271) ) // sldh

	ROM_REGION( 0x200000, "simm3.0", 0 )
	ROM_LOAD( "jojoba-simm3.0", 0x00000, 0x200000, CRC(4d16e111) SHA1(f198007375be65e89856d64ee2b3857a18b4eab8) )
	ROM_REGION( 0x200000, "simm3.1", 0 )
	ROM_LOAD( "jojoba-simm3.1", 0x00000, 0x200000, CRC(9b3406d3) SHA1(54e90cd334d13e2c74305c6b87ebce1365ef3d59) )
	ROM_REGION( 0x200000, "simm3.2", 0 )
	ROM_LOAD( "jojoba-simm3.2", 0x00000, 0x200000, CRC(f2414997) SHA1(fb89d5784250538ad17fd527267b513afb6eca20) )
	ROM_REGION( 0x200000, "simm3.3", 0 )
	ROM_LOAD( "jojoba-simm3.3", 0x00000, 0x200000, CRC(954b9c7d) SHA1(0d64d97167d4e669d7e4f3a388f9d5ec1e18ed42) )
	ROM_REGION( 0x200000, "simm3.4", 0 )
	ROM_LOAD( "jojoba-simm3.4", 0x00000, 0x200000, CRC(625adc1d) SHA1(533d62759ecece10c711d99bfca403e5cba279b5) )
	ROM_REGION( 0x200000, "simm3.5", 0 )
	ROM_LOAD( "jojoba-simm3.5", 0x00000, 0x200000, CRC(20a70bb4) SHA1(3bd8376304ffc974fb8031eac8bebff27969538c) )
	ROM_REGION( 0x200000, "simm3.6", 0 )
	ROM_LOAD( "jojoba-simm3.6", 0x00000, 0x200000, CRC(a10ec5af) SHA1(9b403260e8fbdacaa5369ab79fc05855cc6a6bdb) )
	ROM_REGION( 0x200000, "simm3.7", 0 )
	ROM_LOAD( "jojoba-simm3.7", 0x00000, 0x200000, CRC(0bd0de7a) SHA1(1debecda5f282f2a1dd17e887e522a4d00c5dc9d) )

	ROM_REGION( 0x200000, "simm4.0", 0 )
	ROM_LOAD( "jojoba-simm4.0", 0x00000, 0x200000, CRC(6ea14adc) SHA1(696b2ec66f3c197817a60f507a1b4c78db37f488) )
	ROM_REGION( 0x200000, "simm4.1", 0 )
	ROM_LOAD( "jojoba-simm4.1", 0x00000, 0x200000, CRC(8f4c42fb) SHA1(363d769b0b066ce139125426d2da6dfa15d1eb28) )
	ROM_REGION( 0x200000, "simm4.2", 0 )
	ROM_LOAD( "jojoba-simm4.2", 0x00000, 0x200000, CRC(ef0586d1) SHA1(8fcc350da20e3e59fa76fa14e10f2c47233ba9dc) )
	ROM_REGION( 0x200000, "simm4.3", 0 )
	ROM_LOAD( "jojoba-simm4.3", 0x00000, 0x200000, CRC(93ccc470) SHA1(5d267679e61c0fb592ad5f696d3c06ec1746d0b3) )
	ROM_REGION( 0x200000, "simm4.4", 0 )
	ROM_LOAD( "jojoba-simm4.4", 0x00000, 0x200000, CRC(3d9ec7d2) SHA1(665b867bab928be183c2006527e55f9b8ec4a271) )
	ROM_REGION( 0x200000, "simm4.5", 0 )
	ROM_LOAD( "jojoba-simm4.5", 0x00000, 0x200000, CRC(03e66850) SHA1(8478662dc9db20d9a186d315a883bd1cbb5e5000) )
	ROM_REGION( 0x200000, "simm4.6", 0 )
	ROM_LOAD( "jojoba-simm4.6", 0x00000, 0x200000, CRC(01606ac3) SHA1(ccc74edeca6abdd86fc1cf42ececa1ea393b3261) )
	ROM_REGION( 0x200000, "simm4.7", 0 )
	ROM_LOAD( "jojoba-simm4.7", 0x00000, 0x200000, CRC(36392b87) SHA1(e62080c8461775c1e180400dfb44414679fd0fc1) )

	ROM_REGION( 0x200000, "simm5.0", 0 )
	ROM_LOAD( "jojoba-simm5.0", 0x00000, 0x200000, CRC(2ef8c60c) SHA1(dea87a73a11b8edd27c3c9c5ab2af295cb5508f9) )
	ROM_REGION( 0x200000, "simm5.1", 0 )
	ROM_LOAD( "jojoba-simm5.1", 0x00000, 0x200000, CRC(cf7d7ca6) SHA1(b347707b1e5bc71d28b282273f893592e5f9e333) )
	ROM_REGION( 0x200000, "simm5.2", 0 )
	ROM_LOAD( "jojoba-simm5.2", 0x00000, 0x200000, CRC(b7815bfa) SHA1(0b5a3a2ffe1b3c0ca765dcedc297e78e5928302b) )
	ROM_REGION( 0x200000, "simm5.3", 0 )
	ROM_LOAD( "jojoba-simm5.3", 0x00000, 0x200000, CRC(9bfec049) SHA1(62cc9a1920047863205544b77344ee18f310f084) )
	ROM_REGION( 0x200000, "simm5.4", 0 )
	ROM_LOAD( "jojoba-simm5.4", 0x00000, 0x200000, CRC(d167536b) SHA1(e2637d3486f168ce44e0a00413d38960cb86db4c) )
	ROM_REGION( 0x200000, "simm5.5", 0 )
	ROM_LOAD( "jojoba-simm5.5", 0x00000, 0x200000, CRC(55e7a042) SHA1(c18bda61fa005d9174a27b7b7d324004262a4525) )
	ROM_REGION( 0x200000, "simm5.6", 0 )
	ROM_LOAD( "jojoba-simm5.6", 0x00000, 0x200000, CRC(4fb32906) SHA1(3a5965b3197517932c8aa4c07a6ea6a190a338d7) )
	ROM_REGION( 0x200000, "simm5.7", 0 )
	ROM_LOAD( "jojoba-simm5.7", 0x00000, 0x200000, CRC(8c8be520) SHA1(c461f3f76a83592b36b29afb316679a7c8972404) )
ROM_END

ROM_START( jojobaner1 )
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "jojoba_euro_nocd.29f400.u2", 0x000000, 0x080000, CRC(1ee2d679) SHA1(9e129b454a376606b3f7e8aec64de425cf9c635c) )

	ROM_REGION( 0x200000, "simm1.0", 0 )
	ROM_LOAD( "jojoba-simm1.0", 0x00000, 0x200000, CRC(76976231) SHA1(90adde7e5983ec6a4e02789d5cefe9e85c9c52d5) ) // sldh
	ROM_REGION( 0x200000, "simm1.1", 0 )
	ROM_LOAD( "jojoba-simm1.1", 0x00000, 0x200000, CRC(cedd78e7) SHA1(964988b90a2f14c1da2cfc48d943e16e54da3fd3) ) // sldh
	ROM_REGION( 0x200000, "simm1.2", 0 )
	ROM_LOAD( "jojoba-simm1.2", 0x00000, 0x200000, CRC(2955b77f) SHA1(2a907a5cd91448bfc420c318584e5ef4bbe55a91) ) // sldh
	ROM_REGION( 0x200000, "simm1.3", 0 )
	ROM_LOAD( "jojoba-simm1.3", 0x00000, 0x200000, CRC(280139d7) SHA1(b7c28f6f0218688fb873a3106d2f95ea2e1e927c) ) // sldh

	ROM_REGION( 0x200000, "simm2.0", 0 )
	ROM_LOAD( "jojoba-simm2.0", 0x00000, 0x200000, CRC(305c4914) SHA1(c3a73ffe58f61ab8f1cd9e3f0891037638dc5a9b) ) // sldh
	ROM_REGION( 0x200000, "simm2.1", 0 )
	ROM_LOAD( "jojoba-simm2.1", 0x00000, 0x200000, CRC(18af4f3b) SHA1(04b8fdf23a782b10c203b111cc634a6d3474044a) ) // sldh
	ROM_REGION( 0x200000, "simm2.2", 0 )
	ROM_LOAD( "jojoba-simm2.2", 0x00000, 0x200000, CRC(397e5c9e) SHA1(021d86ee66bf951fb6a1dd90fb7007c6865cbb8b) ) // sldh
	ROM_REGION( 0x200000, "simm2.3", 0 )
	ROM_LOAD( "jojoba-simm2.3", 0x00000, 0x200000, CRC(a9d0a7d7) SHA1(b2cfc0661f8903ddbeea8a604ee8b42097e10ab8) ) // sldh

	ROM_REGION( 0x200000, "simm3.0", 0 )
	ROM_LOAD( "jojoba-simm3.0",  0x00000, 0x200000, CRC(4d16e111) SHA1(f198007375be65e89856d64ee2b3857a18b4eab8) )
	ROM_REGION( 0x200000, "simm3.1", 0 )
	ROM_LOAD( "jojoba-simm3.1",  0x00000, 0x200000, CRC(9b3406d3) SHA1(54e90cd334d13e2c74305c6b87ebce1365ef3d59) )
	ROM_REGION( 0x200000, "simm3.2", 0 )
	ROM_LOAD( "jojoba-simm3.2",  0x00000, 0x200000, CRC(f2414997) SHA1(fb89d5784250538ad17fd527267b513afb6eca20) )
	ROM_REGION( 0x200000, "simm3.3", 0 )
	ROM_LOAD( "jojoba-simm3.3",  0x00000, 0x200000, CRC(954b9c7d) SHA1(0d64d97167d4e669d7e4f3a388f9d5ec1e18ed42) )
	ROM_REGION( 0x200000, "simm3.4", 0 )
	ROM_LOAD( "jojoba-simm3.4",  0x00000, 0x200000, CRC(625adc1d) SHA1(533d62759ecece10c711d99bfca403e5cba279b5) )
	ROM_REGION( 0x200000, "simm3.5", 0 )
	ROM_LOAD( "jojoba-simm3.5",  0x00000, 0x200000, CRC(20a70bb4) SHA1(3bd8376304ffc974fb8031eac8bebff27969538c) )
	ROM_REGION( 0x200000, "simm3.6", 0 )
	ROM_LOAD( "jojoba-simm3.6",  0x00000, 0x200000, CRC(a10ec5af) SHA1(9b403260e8fbdacaa5369ab79fc05855cc6a6bdb) )
	ROM_REGION( 0x200000, "simm3.7", 0 )
	ROM_LOAD( "jojoba-simm3.7",  0x00000, 0x200000, CRC(0bd0de7a) SHA1(1debecda5f282f2a1dd17e887e522a4d00c5dc9d) )

	ROM_REGION( 0x200000, "simm4.0", 0 )
	ROM_LOAD( "jojoba-simm4.0",  0x00000, 0x200000, CRC(6ea14adc) SHA1(696b2ec66f3c197817a60f507a1b4c78db37f488) )
	ROM_REGION( 0x200000, "simm4.1", 0 )
	ROM_LOAD( "jojoba-simm4.1",  0x00000, 0x200000, CRC(8f4c42fb) SHA1(363d769b0b066ce139125426d2da6dfa15d1eb28) )
	ROM_REGION( 0x200000, "simm4.2", 0 )
	ROM_LOAD( "jojoba-simm4.2",  0x00000, 0x200000, CRC(ef0586d1) SHA1(8fcc350da20e3e59fa76fa14e10f2c47233ba9dc) )
	ROM_REGION( 0x200000, "simm4.3", 0 )
	ROM_LOAD( "jojoba-simm4.3",  0x00000, 0x200000, CRC(93ccc470) SHA1(5d267679e61c0fb592ad5f696d3c06ec1746d0b3) )
	ROM_REGION( 0x200000, "simm4.4", 0 )
	ROM_LOAD( "jojoba-simm4.4",  0x00000, 0x200000, CRC(3d9ec7d2) SHA1(665b867bab928be183c2006527e55f9b8ec4a271) )
	ROM_REGION( 0x200000, "simm4.5", 0 )
	ROM_LOAD( "jojoba-simm4.5",  0x00000, 0x200000, CRC(03e66850) SHA1(8478662dc9db20d9a186d315a883bd1cbb5e5000) )
	ROM_REGION( 0x200000, "simm4.6", 0 )
	ROM_LOAD( "jojoba-simm4.6",  0x00000, 0x200000, CRC(01606ac3) SHA1(ccc74edeca6abdd86fc1cf42ececa1ea393b3261) )
	ROM_REGION( 0x200000, "simm4.7", 0 )
	ROM_LOAD( "jojoba-simm4.7",  0x00000, 0x200000, CRC(36392b87) SHA1(e62080c8461775c1e180400dfb44414679fd0fc1) )

	ROM_REGION( 0x200000, "simm5.0", 0 )
	ROM_LOAD( "jojoba-simm5.0",  0x00000, 0x200000, CRC(2ef8c60c) SHA1(dea87a73a11b8edd27c3c9c5ab2af295cb5508f9) )
	ROM_REGION( 0x200000, "simm5.1", 0 )
	ROM_LOAD( "jojoba-simm5.1",  0x00000, 0x200000, CRC(cf7d7ca6) SHA1(b347707b1e5bc71d28b282273f893592e5f9e333) )
	ROM_REGION( 0x200000, "simm5.2", 0 )
	ROM_LOAD( "jojoba-simm5.2",  0x00000, 0x200000, CRC(b7815bfa) SHA1(0b5a3a2ffe1b3c0ca765dcedc297e78e5928302b) )
	ROM_REGION( 0x200000, "simm5.3", 0 )
	ROM_LOAD( "jojoba-simm5.3",  0x00000, 0x200000, CRC(9bfec049) SHA1(62cc9a1920047863205544b77344ee18f310f084) )
	ROM_REGION( 0x200000, "simm5.4", 0 )
	ROM_LOAD( "jojoba-simm5.4",  0x00000, 0x200000, CRC(d167536b) SHA1(e2637d3486f168ce44e0a00413d38960cb86db4c) )
	ROM_REGION( 0x200000, "simm5.5", 0 )
	ROM_LOAD( "jojoba-simm5.5",  0x00000, 0x200000, CRC(55e7a042) SHA1(c18bda61fa005d9174a27b7b7d324004262a4525) )
	ROM_REGION( 0x200000, "simm5.6", 0 )
	ROM_LOAD( "jojoba-simm5.6",  0x00000, 0x200000, CRC(4fb32906) SHA1(3a5965b3197517932c8aa4c07a6ea6a190a338d7) )
	ROM_REGION( 0x200000, "simm5.7", 0 )
	ROM_LOAD( "jojoba-simm5.7",  0x00000, 0x200000, CRC(8c8be520) SHA1(c461f3f76a83592b36b29afb316679a7c8972404) )
ROM_END

/* Bootlegs for use with modified security carts */

ROM_START( cps3boot ) // for cart with standard SH2
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "no-battery_bios_29f400_for_hd6417095_sh2.u2", 0x000000, 0x080000, CRC(cb9bd5b0) SHA1(ea7ecb3deb69f5307a62d8f0d7d8e68d49013d07))

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "UniCD-CPS3_for_standard_SH2_V4", 0, SHA1(099c52bd38753f0f4876243e7aa87ca482a2dcb7) )
ROM_END

ROM_START( cps3booto ) // for cart with standard SH2
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "no-battery_bios_29f400_for_hd6417095_sh2.u2", 0x000000, 0x080000, CRC(cb9bd5b0) SHA1(ea7ecb3deb69f5307a62d8f0d7d8e68d49013d07))

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "no-battery_multi-game_bootleg_cd_for_hd6417095_sh2", 0, SHA1(6057cc3ec7991c0c00a7ab9da6ac2f92c9fb1aed) )
ROM_END

ROM_START( cps3booto2 ) // for cart with standard SH2
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "no-battery_bios_29f400_for_hd6417095_sh2.u2", 0x000000, 0x080000, CRC(cb9bd5b0) SHA1(ea7ecb3deb69f5307a62d8f0d7d8e68d49013d07))

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "no-battery_multi-game_bootleg_cd_for_hd6417095_sh2_older", 0, SHA1(123f2fcb0f3dd3d6b859e82a51d0127e46763776) )
ROM_END

ROM_START( cps3bs32 ) // for cart with standard SH2
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "no-battery_bios_29f400_for_hd6417095_sh2.u2", 0x000000, 0x080000, CRC(cb9bd5b0) SHA1(ea7ecb3deb69f5307a62d8f0d7d8e68d49013d07))

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "sfiii_2nd_impact_converted_for_standard_sh2_v3", 0, SHA1(8f180d159e88042a1e819cefd39eef67f5e86e3d) )
ROM_END

ROM_START( cps3bs32a ) // for cart with standard SH2
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "no-battery_bios_29f400_for_hd6417095_sh2.u2", 0x000000, 0x080000, CRC(cb9bd5b0) SHA1(ea7ecb3deb69f5307a62d8f0d7d8e68d49013d07))

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "sfiii_2nd_impact_converted_for_standard_sh2_older", 0, SHA1(8a8e4138c3bf12435933ab9d9ace510513200843) ) // v1 or v2?
ROM_END

ROM_START( cps3boota ) // for cart with dead custom SH2 (or 2nd Impact CPU which is the same as a dead one)
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "no-battery_bios_29f400_for_dead_security_cart.u2", 0x000000, 0x080000, CRC(0fd56fb3) SHA1(5a8bffc07eb7da73cf4bca6718df72e471296bfd) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "UniCD-CPS3_for_custom_SH2_V5", 0, SHA1(50a5b2845d3dd3de3bce15c4f1b58500db80cabe) )
ROM_END

ROM_START( cps3bootao ) // for cart with dead custom SH2 (or 2nd Impact CPU which is the same as a dead one)
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "no-battery_bios_29f400_for_dead_security_cart.u2", 0x000000, 0x080000, CRC(0fd56fb3) SHA1(5a8bffc07eb7da73cf4bca6718df72e471296bfd) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "no-battery_multi-game_bootleg_cd_for_dead_security_cart", 0, SHA1(1ede2f1ba197ee787208358a13eae7185a5ae3b2) )
ROM_END


ROM_START( cps3bootao2 ) // for cart with dead custom SH2 (or 2nd Impact CPU which is the same as a dead one)
	ROM_REGION32_BE( 0x080000, "bios", 0 ) /* bios region */
	ROM_LOAD( "no-battery_bios_29f400_for_dead_security_cart.u2", 0x000000, 0x080000, CRC(0fd56fb3) SHA1(5a8bffc07eb7da73cf4bca6718df72e471296bfd) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "no-battery_multi-game_bootleg_cd_for_dead_security_cart_older", 0, SHA1(4b0b673b45dac94da018576c0a7f8644653fc564) )
ROM_END


/*****************************************************************************************
  CPS3 game region / special flag information
*****************************************************************************************/

/*****************************************************************************************

    Red Earth / Warzard

    JAPAN 1
    ASIA 2
    EURO 3
    USA 4
    HISPANIC 5
    BRAZIL 6
    OCEANIA 7
    ASIA NCD 8

    UINT32 *rom =  (UINT32*)machine.root_device().memregion ( "bios" )->base();
    rom[0x1fed8/4]^=0x00000001; // clear region to 0 (invalid)
    rom[0x1fed8/4]^=0x00000008; // region 8 - ASIA NO CD - doesn't actually skip the CD
                                // test on startup, only during game, must be another flag
                                // somewhere too, and we don't have any actual NCD dumps
                                // to compare (or it expects SCSI to report there being
                                // no cd drive?)

*****************************************************************************************/

/*****************************************************************************************

    Street Fighter III: New Generation

    JAPAN 1
    ASIA NCD 2
    EURO 3
    USA 4
    HISPANIC 5
    BRAZIL 6
    OCEANIA 7
    ASIA 8

    // bios rom also lists korea, but game rom does not.

    UINT32 *rom =  (UINT32*)machine.root_device().memregion ( "bios" )->base();
    rom[0x1fec8/4]^=0x00000001; // region (clear region)
    rom[0x1fec8/4]^=0x00000008; // region
    rom[0x1fecc/4]^=0x01000000; // nocd - this ONLY skips the cd check in the bios test
                                // menu is region is ASIA NCD, otherwise it will report
                                // NG, Asia was probably the only NCD region for this

*****************************************************************************************/

/*****************************************************************************************

    Street Fighter III 2nd Impact: Giant Attack

    JAPAN 1
    ASIA NCD 2
    EURO 3
    USA 4
    HISPANIC 5
    BRAZIL 6
    OCEANIA 7
    ASIA 8

    UINT32 *rom =  (UINT32*)machine.root_device().memregion ( "bios" )->base();
    rom[0x1fec8/4]^=0x00000001; // region (clear region)
    rom[0x1fec8/4]^=0x00000008; // region
    rom[0x1fecc/4]^=0x01000000; // nocd - this ONLY skips the cd check in the bios test
                                // menu is region is ASIA NCD, otherwise it will report
                                // NG, Asia was probably the only NCD region for this

*****************************************************************************************/

/*****************************************************************************************

    JoJo's Venture / JoJo no Kimyou na Bouken

    XXXXXX 0
    JAPAN 1
    ASIA 2
    EURO 3
    USA 4
    HISPANIC 5
    BRAZIL 6
    OCEANIA 7

    DEVELOPMENT VERSION add 0x70 mask!

    UINT32 *rom =  (UINT32*)machine.root_device().memregion ( "bios" )->base();
    rom[0x1fec8/4]^=0x00000001; // region hack (clear jpn)

    rom[0x1fec8/4]^=0x00000004; // region
    rom[0x1fec8/4]^=0x00000070; // DEV mode
    rom[0x1fecc/4]^=0x01000000; // nocd

*****************************************************************************************/

/*****************************************************************************************

    Street Fighter III 3rd Strike: Fight for the Future

    JAPAN 1
    ASIA 2
    EURO 3
    USA 4
    HISPANIC 5
    BRAZIL 6
    OCEANIA 7

    UINT32 *rom =  (UINT32*)machine.root_device().memregion ( "bios" )->base();
    rom[0x1fec8/4]^=0x00000004; // region (clear region)
    rom[0x1fec8/4]^=0x00000001; // region
    rom[0x1fecc/4]^=0x01000000; // nocd

*****************************************************************************************/

/*****************************************************************************************

    JoJo's Bizarre Adventure / JoJo no Kimyou na Bouken: Mirai e no Isan

    XXXXXX 0
    JAPAN 1
    ASIA 2
    EURO 3
    USA 4
    HISPANIC 5
    BRAZIL 6
    OCEANIA 7

    DEVELOPMENT VERSION add 0x70 mask!

    UINT32 *rom =  (UINT32*)machine.root_device().memregion ( "bios" )->base();
    rom[0x1fec8/4]^=0x00000001; // region (clear jpn)
    rom[0x1fec8/4]^=0x00000002; // region
    rom[0x1fec8/4]^=0x00000070; // DEV mode
    rom[0x1fecc/4]^=0x01000000; // nocd

*****************************************************************************************/

/* Red Earth / Warzard */

// 961121
GAME( 1996, redearth,  0,        redearth, cps3_re, cps3_state,   redearth, ROT0, "Capcom", "Red Earth (Euro 961121)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, warzard,   redearth, redearth, cps3_re, cps3_state,   redearth, ROT0, "Capcom", "Warzard (Japan 961121)", MACHINE_IMPERFECT_GRAPHICS )

// 961023
GAME( 1996, redearthr1,redearth, redearth, cps3_re, cps3_state,   redearth, ROT0, "Capcom", "Red Earth (Euro 961023)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, warzardr1, redearth, redearth, cps3_re, cps3_state,   redearth, ROT0, "Capcom", "Warzard (Japan 961023)", MACHINE_IMPERFECT_GRAPHICS )

/* Street Fighter III: New Generation */

// 970403
// not dumped

// 970312
// not dumped

// 970204
GAME( 1997, sfiii,     0,        sfiii,    cps3, cps3_state,      sfiii,    ROT0, "Capcom", "Street Fighter III: New Generation (Euro 970204)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, sfiiiu,    sfiii,    sfiii,    cps3, cps3_state,      sfiii,    ROT0, "Capcom", "Street Fighter III: New Generation (USA 970204)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, sfiiia,    sfiii,    sfiii,    cps3, cps3_state,      sfiii,    ROT0, "Capcom", "Street Fighter III: New Generation (Asia 970204)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, sfiiij,    sfiii,    sfiii,    cps3, cps3_state,      sfiii,    ROT0, "Capcom", "Street Fighter III: New Generation (Japan 970204)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, sfiiih,    sfiii,    sfiii,    cps3, cps3_state,      sfiii,    ROT0, "Capcom", "Street Fighter III: New Generation (Hispanic 970204)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, sfiiin,    sfiii,    sfiii,    cps3, cps3_state,      sfiii,    ROT0, "Capcom", "Street Fighter III: New Generation (Asia 970204, NO CD, bios set 1)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, sfiiina,   sfiii,    sfiii,    cps3, cps3_state,      sfiii,    ROT0, "Capcom", "Street Fighter III: New Generation (Asia 970204, NO CD, bios set 2)", MACHINE_IMPERFECT_GRAPHICS )

/* Street Fighter III 2nd Impact: Giant Attack */

// 971016
// not dumped

// 970930
GAMEL(1997, sfiii2,    0,        sfiii2,   cps3, cps3_state,      sfiii2,   ROT0, "Capcom", "Street Fighter III 2nd Impact: Giant Attack (USA 970930)", MACHINE_IMPERFECT_GRAPHICS, layout_sfiii2 ) // layout is for widescreen support
GAMEL(1997, sfiii2j,   sfiii2,   sfiii2,   cps3, cps3_state,      sfiii2,   ROT0, "Capcom", "Street Fighter III 2nd Impact: Giant Attack (Japan 970930)", MACHINE_IMPERFECT_GRAPHICS, layout_sfiii2 )
GAMEL(1997, sfiii2n,   sfiii2,   sfiii2,   cps3, cps3_state,      sfiii2,   ROT0, "Capcom", "Street Fighter III 2nd Impact: Giant Attack (Asia 970930, NO CD)", MACHINE_IMPERFECT_GRAPHICS, layout_sfiii2 )

/* JoJo's Venture / JoJo no Kimyou na Bouken */

// 990128
GAME( 1998, jojo,      0,        jojo,     cps3_jojo, cps3_state, jojo,     ROT0, "Capcom", "JoJo's Venture (USA 990128)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, jojoj,     jojo,     jojo,     cps3_jojo, cps3_state, jojo,     ROT0, "Capcom", "JoJo no Kimyou na Bouken (Japan 990128)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, jojon,     jojo,     jojo,     cps3_jojo, cps3_state, jojo,     ROT0, "Capcom", "JoJo's Venture (Asia 990128, NO CD)", MACHINE_IMPERFECT_GRAPHICS )

// 990108
GAME( 1998, jojor1,    jojo,     jojo,     cps3_jojo, cps3_state, jojo,     ROT0, "Capcom", "JoJo's Venture (USA 990108)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, jojojr1,   jojo,     jojo,     cps3_jojo, cps3_state, jojo,     ROT0, "Capcom", "JoJo no Kimyou na Bouken (Japan 990108)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, jojonr1,   jojo,     jojo,     cps3_jojo, cps3_state, jojo,     ROT0, "Capcom", "JoJo's Venture (Asia 990108, NO CD)", MACHINE_IMPERFECT_GRAPHICS )

// 981202
GAME( 1998, jojor2,    jojo,     jojo,     cps3_jojo, cps3_state, jojo,     ROT0, "Capcom", "JoJo's Venture (USA 981202)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, jojojr2,   jojo,     jojo,     cps3_jojo, cps3_state, jojo,     ROT0, "Capcom", "JoJo no Kimyou na Bouken (Japan 981202)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, jojonr2,   jojo,     jojo,     cps3_jojo, cps3_state, jojo,     ROT0, "Capcom", "JoJo's Venture (Asia 981202, NO CD)", MACHINE_IMPERFECT_GRAPHICS )

/* Street Fighter III 3rd Strike: Fight for the Future */

// 990608
GAME( 1999, sfiii3,    0,        sfiii3,   cps3, cps3_state,      sfiii3,   ROT0, "Capcom", "Street Fighter III 3rd Strike: Fight for the Future (Euro 990608)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, sfiii3u,   sfiii3,   sfiii3,   cps3, cps3_state,      sfiii3,   ROT0, "Capcom", "Street Fighter III 3rd Strike: Fight for the Future (USA 990608)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, sfiii3n,   sfiii3,   sfiii3,   cps3, cps3_state,      sfiii3,   ROT0, "Capcom", "Street Fighter III 3rd Strike: Fight for the Future (Japan 990608, NO CD)", MACHINE_IMPERFECT_GRAPHICS )

// 990512
GAME( 1999, sfiii3r1,  sfiii3,   sfiii3,   cps3, cps3_state,      sfiii3,   ROT0, "Capcom", "Street Fighter III 3rd Strike: Fight for the Future (Euro 990512)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, sfiii3ur1, sfiii3,   sfiii3,   cps3, cps3_state,      sfiii3,   ROT0, "Capcom", "Street Fighter III 3rd Strike: Fight for the Future (USA 990512)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, sfiii3nr1, sfiii3,   sfiii3,   cps3, cps3_state,      sfiii3,   ROT0, "Capcom", "Street Fighter III 3rd Strike: Fight for the Future (Japan 990512, NO CD)", MACHINE_IMPERFECT_GRAPHICS )

/* JoJo's Bizarre Adventure / JoJo no Kimyou na Bouken: Mirai e no Isan */

// 990927
GAME( 1999, jojoba,    0,        jojoba,   cps3_jojo, cps3_state, jojoba,   ROT0, "Capcom", "JoJo no Kimyou na Bouken: Mirai e no Isan (Japan 990927)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, jojoban,   jojoba,   jojoba,   cps3_jojo, cps3_state, jojoba,   ROT0, "Capcom", "JoJo no Kimyou na Bouken: Mirai e no Isan (Japan 990927, NO CD)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, jojobane,  jojoba,   jojoba,   cps3_jojo, cps3_state, jojoba,   ROT0, "Capcom", "JoJo's Bizarre Adventure (Euro 990927, NO CD)", MACHINE_IMPERFECT_GRAPHICS )

// 990913
GAME( 1999, jojobar1,  jojoba,   jojoba,   cps3_jojo, cps3_state, jojoba,   ROT0, "Capcom", "JoJo no Kimyou na Bouken: Mirai e no Isan (Japan 990913)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, jojobanr1, jojoba,   jojoba,   cps3_jojo, cps3_state, jojoba,   ROT0, "Capcom", "JoJo no Kimyou na Bouken: Mirai e no Isan (Japan 990913, NO CD)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, jojobaner1,jojoba,   jojoba,   cps3_jojo, cps3_state, jojoba,   ROT0, "Capcom", "JoJo's Bizarre Adventure (Euro 990913, NO CD)", MACHINE_IMPERFECT_GRAPHICS )

// bootlegs, hold START1 during bootup to change games

// newest revision, fixes some issues with Warzard decryption.
GAME( 1999, cps3boot,   0,        sfiii3,   cps3_jojo, cps3_state, cps3boot,   ROT0, "bootleg", "CPS3 Multi-game bootleg for HD6417095 type SH2 (V4)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, cps3boota,  cps3boot, sfiii3,   cps3_jojo, cps3_state, sfiii2,     ROT0, "bootleg", "CPS3 Multi-game bootleg for dead security cart (V5)", MACHINE_IMPERFECT_GRAPHICS )

GAME( 1999, cps3booto,  cps3boot, sfiii3,   cps3_jojo, cps3_state, cps3boot,   ROT0, "bootleg", "CPS3 Multi-game bootleg for HD6417095 type SH2 (older)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, cps3bootao, cps3boot, sfiii3,   cps3_jojo, cps3_state, sfiii2,     ROT0, "bootleg", "CPS3 Multi-game bootleg for dead security cart (older)", MACHINE_IMPERFECT_GRAPHICS )
// this doesn't play 2nd Impact despite it being listed.  2nd Impact uses separate data/code encryption and can't be decrypted cleanly for a standard SH2.  Selecting it just flashes in a copy of 3rd Strike with the 2nd Impact loading screen
GAME( 1999, cps3booto2, cps3boot, sfiii3,   cps3_jojo, cps3_state, cps3boot,   ROT0, "bootleg", "CPS3 Multi-game bootleg for HD6417095 type SH2 (oldest) (New Generation, 3rd Strike, JoJo's Venture, JoJo's Bizarre Adventure and Red Earth only)", MACHINE_IMPERFECT_GRAPHICS )
// this does not play Red Earth or the 2 Jojo games.  New Generation and 3rd Strike have been heavily modified to work with the separate code/data encryption a dead cart / 2nd Impact cart has.  Selecting the other games will give an 'invalid CD' message.
GAME( 1999, cps3bootao2, cps3boot, sfiii3,   cps3_jojo, cps3_state, sfiii2,     ROT0, "bootleg", "CPS3 Multi-game bootleg for dead security cart (oldest) (New Generation, 2nd Impact and 3rd Strike only)", MACHINE_IMPERFECT_GRAPHICS )
// these are test bootleg CDs for running 2nd Impact on a standard SH2
GAME( 1999, cps3bs32,  cps3boot, sfiii3,   cps3_jojo, cps3_state, cps3boot,   ROT0, "bootleg", "Street Fighter III 2nd Impact: Giant Attack (USA 970930, bootleg for HD6417095 type SH2, V3)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, cps3bs32a, cps3boot, sfiii3,   cps3_jojo, cps3_state, cps3boot,   ROT0, "bootleg", "Street Fighter III 2nd Impact: Giant Attack (USA 970930, bootleg for HD6417095 type SH2, older)", MACHINE_IMPERFECT_GRAPHICS ) // older / buggier hack
