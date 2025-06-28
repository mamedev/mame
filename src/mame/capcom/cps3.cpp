// license:BSD-3-Clause
// copyright-holders:David Haywood, Andreas Naive, Tomasz Slanina, ElSemi
/*

CPS3 Driver (preliminary)

Decryption by Andreas Naive

Driver by David Haywood
 with help from Tomasz Slanina and ElSemi

Sound emulation by Philip Bennett

SCSI code by ElSemi

Known Issues:

Whole screen flip not emulated

Miscellaneous TO-DOs:

DMA ack IRQ10 generation:
    Character and Palette DMAs speed is unknown, needs to be measured.

Alpha Blending Effects
    These are actually palette manipulation effects, not true blending.
    Verify them, current emulation might not be 100% accurate.

Tilemap Linezoom
    Seems unused in games. May be enabled in jojo/jojoba dev.menu BG test (P2 btn4)

Palette DMA effects
    Verify them, they might not be 100% accurate at the moment

Verify Full Screen Zoom on real hardware
    How far can it zoom etc.

Verify CRT registers
    Actual Pixel clock for H Start and H Blank registers is unknown. It is not known which is base pixel clock
    and how it affected by register 40C0080 bits.

Sprite positioning glitches
    Some sprites are still in the wrong places, seems the placement of zooming sprites is imperfect
    eg. warzard intro + cutscenes leave the left most 16 pixels uncovered because the sprite is positioned incorrectly,
    the same occurs in the sf games.  doesn't look like the origin is correct when zooming in all cases.

Gaps in Sprite Zooming
    Warzard is confirmed to have gaps during some cut-scenes on real hardware.

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
Red Earth                                                  WZD96aA0F  CP3000C0G  ASIA        X      CAP-WZD0A0  CAP-WZD-3   CAP-WZD-3    961023
Red Earth                                                  WZD96aA0F  CP3000C0G  ASIA        X      CAP-WZD0A0  CAP-WZD-5   CAP-WZD-5    961121
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
Street Fighter III 2nd Impact: Giant Attack                3GA97aA0F  CP3000H0G  MEXICO  X          CAP-3GA0A0  CAP-3GA000  CAP-3GA-1    970930
Street Fighter III 2nd Impact: Giant Attack                3GA97aA0F  CP3000H0G  MEXICO  X          CAP-3GA0A0  ?           ?            971016*
Street Fighter III 2nd Impact: Giant Attack                3GA97aA0F  CP3000U0G  USA     X          CAP-3GA0A0  CAP-3GA000  CAP-3GA-1    970930
Street Fighter III 2nd Impact: Giant Attack                3GA97aA0F  CP3000U0G  USA     X          CAP-3GA0A0  ?           ?            971016*

JoJo no Kimyou na Bouken                             1998  JJK98c00F  CP300000G  JAPAN   X          CAP-JJK000  CAP-JJK000  CAP-JJK-140  981202
JoJo no Kimyou na Bouken                                   JJK98c00F  CP300000G  JAPAN   X          CAP-JJK000  CAP-JJK-2   CAP-JJK-160  990108
JoJo no Kimyou na Bouken                                   JJK98c00F  CP300000G  JAPAN   X          CAP-JJK000  CAP-JJK-3   CAP-JJK-161  990128
JoJo's Venture                                             JJK98cA0F  CP3000C0G  ASIA        X                                           981202
JoJo's Venture                                             JJK98cA0F  CP3000C0G  ASIA        X                                           990108
JoJo's Venture                                             JJK98cA0F  CP3000C0G  ASIA        X                                           990128
JoJo's Venture                                             JJK98cA0F  CP3000C0G  ASIA    X          CAP-JJK0A0  CAP-JJK000  CAP-JJK-140  981202
JoJo's Venture                                             JJK98cA0F  CP3000C0G  ASIA    X          CAP-JJK0A0  CAP-JJK-2   CAP-JJK-160  990108
JoJo's Venture                                             JJK98cA0F  CP3000C0G  ASIA    X          CAP-JJK0A0  CAP-JJK-3   CAP-JJK-161  990128
JoJo's Venture                                             JJK98cA0F  CP3000U0G  USA     X          CAP-JJK0A0  CAP-JJK000  CAP-JJK-140  981202
JoJo's Venture                                             JJK98cA0F  CP3000U0G  USA     X          CAP-JJK0A0  CAP-JJK-2   CAP-JJK-160  990108
JoJo's Venture                                             JJK98cA0F  CP3000U0G  USA     X          CAP-JJK0A0  CAP-JJK-3   CAP-JJK-161  990128
JoJo's Venture                                             JJK98cA0F  CP3000B0G  EUROPE  X          CAP-JJK0A0  CAP-JJK000  CAP-JJK-140  981202
JoJo's Venture                                             JJK98cA0F  CP3000B0G  EUROPE  X          CAP-JJK0A0  CAP-JJK-2   CAP-JJK-160  990108
JoJo's Venture                                             JJK98cA0F  CP3000B0G  EUROPE  X          CAP-JJK0A0  CAP-JJK-3   CAP-JJK-161  990128

Street Fighter III 3rd Strike: Fight for the Future  1999  33S99400F  CP300000G  JAPAN   X          CAP-33S000  CAP-33S-1   CAP-33S-1    990512
Street Fighter III 3rd Strike: Fight for the Future        33S99400F  CP300000G  JAPAN   X          CAP-33S000  CAP-33S-2   CAP-33S-2    990608
Street Fighter III 3rd Strike: Fight for the Future        33S99400F  CP300000G  JAPAN       X                                           990512
Street Fighter III 3rd Strike: Fight for the Future        33S99400F  CP300000G  JAPAN       X                                           990608
Street Fighter III 3rd Strike: Fight for the Future        33S994A0F  CP3000B0G  EUROPE  X          CAP-33S0A0  CAP-33S-1   CAP-33S-1    990512
Street Fighter III 3rd Strike: Fight for the Future        33S994A0F  CP3000B0G  EUROPE  X          CAP-33S0A0  CAP-33S-2   CAP-33S-2    990608
Street Fighter III 3rd Strike: Fight for the Future        33S994A0F  CP3000C0G  ASIA*   X          CAP-33S0A0  CAP-33S-1   CAP-33S-1    990512
Street Fighter III 3rd Strike: Fight for the Future        33S994A0F  CP3000C0G  ASIA*   X          CAP-33S0A0  CAP-33S-2   CAP-33S-2    990608
Street Fighter III 3rd Strike: Fight for the Future        33S994A0F  CP3000C0G  ASIA        X                                           990512
Street Fighter III 3rd Strike: Fight for the Future        33S994A0F  CP3000C0G  ASIA        X                                           990608
Street Fighter III 3rd Strike: Fight for the Future        33S994A0F  CP3000H0G  MEXICO* ?          CAP-33S0A0  CAP-33S-1   CAP-33S-1    990512
Street Fighter III 3rd Strike: Fight for the Future        33S994A0F  CP3000H0G  MEXICO* ?          CAP-33S0A0  CAP-33S-2   CAP-33S-2    990608
Street Fighter III 3rd Strike: Fight for the Future        33S994A0F  CP3000U0G  USA     X          CAP-33S0A0  CAP-33S-1   CAP-33S-1    990512
Street Fighter III 3rd Strike: Fight for the Future        33S994A0F  CP3000U0G  USA     X          CAP-33S0A0  CAP-33S-2   CAP-33S-2    990608

JoJo no Kimyou na Bouken: Mirai e no Isan            1999  JJM99900F  CP300000G  JAPAN   X          CAP-JJM000  CAP-JJM-0   CAP-JJM-110  990913
JoJo no Kimyou na Bouken: Mirai e no Isan                  JJM99900F  CP300000G  JAPAN   X          CAP-JJM000  CAP-JJM-1   CAP-JJM-120  990927
JoJo's Bizarre Adventure                                   JJM999A0F  CP3000B0G  EUROPE  X          CAP-JJM0A0  CAP-JJM-0   CAP-JJM-110  990913
JoJo's Bizarre Adventure                                   JJM999A0F  CP3000B0G  EUROPE  X          CAP-JJM0A0  CAP-JJM-1   CAP-JJM-120  990927
JoJo no Kimyou na Bouken: Mirai e no Isan                  JJM99900F  CP300000G  JAPAN       X                                           990913
JoJo no Kimyou na Bouken: Mirai e no Isan                  JJM99900F  CP300000G  JAPAN       X                                           990927
JoJo no Kimyou na Bouken: Mirai e no Isan                  JJM99900F  CP300000G  JAPAN       X                              CAP-JJM-121  991015
JoJo's Bizarre Adventure                                   JJM999A0F  CP3000B0G  EUROPE      X                                           990913
JoJo's Bizarre Adventure                                   JJM999A0F  CP3000B0G  EUROPE      X                                           990927
JoJo's Bizarre Adventure                                   JJM999A0F  CP3000B0G  EUROPE      X                              CAP-JJM-121  991015

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
dead/suicided.
It is thought that when a cartridge dies it will set the decryption keys identical to the ones of SFIII-2nd Impact, so
removing the battery and changing the content of the flashROM (if it's not a 2nd Impact) will make it run as a normal
SFIII-2nd Impact cartridge (verified).
Decryption always applied when reading boot flash ROM or SH-2's On-chip cache areas. Based of the fact, what SFIII-2nd uses
encrypted boot ROM but plain not encrypted SIMMs 1&2 code&data - besides of key, static RAM inside of CPU also
should contain decryption range for SIMMs1&2, or some flag which enable or disable it.

Because the CPU in the cart is always powered by the battery, it has stealth capability that allows it to continually
monitor the situation. If the custom CPU detects any tampering (generally things such as voltage fluctuation or voltage
dropping or even removal of the cart with the power on), it immediately erases the SRAM (i.e. the decryption key)
inside the CPU which effectively kills the security cart. It is known (from decapping it) that the CPU in the security
cart contains an amount of static RAM for data storage and a SH2 core based on the Hitachi SH7010-series (SH7014)
SuperH RISC engine family of Microprocessors.

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
|        TD62064   **ADM202     |DL-2929 |   | |   |CAPCOM |  |  |       | |
|                               |IOU     |   | |   |DL-3429|  |  |       | |
|        TD62064   **ADM202     |--------|   | |   |GLL1   |  S  S       | |
|--|                            *HA16103FPJ  | |   |-------|  I  I       |-|
   |                                         | |CN5           M  M        |
   |                                         | |   |-------|  M  M        |
  |-|                        93C46           | |   |CAPCOM |  2  1        |
  | |      PS2501                            | |   |DL-2829|  |  | |-----||
  | |CN1                         **62256     | |   |CCU    |  |  | |AMD  ||
  | |      PS2501                            | |   |-------|  |  | |33C93||
  |-|             **CN3       **BT1          |-|              |  | |-----||
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
      62256      - 32k x8 SRAM (SOJ28). This is the 'SS RAM' in the test mode memory test and is connected to the custom
                   SSU chip.
      HM514260(1)- Hitachi HM514260CJ7 256k x16 DRAM (SOJ40). This is the 'Work RAM' in the test mode memory test and is
                   connected to the custom CCU chip.
      HM514260(2)- Hitachi HM514260CJ7 256k x16 DRAM (SOJ40). This is the 'Sprite RAM' in the test mode memory test
      TC5118160  - Toshiba TC5118160BJ-60 or NEC 4218160-60 1M x16 DRAM (SOJ42). This is the 'Character RAM' in the
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

** Populated at small number of 95682A-2 boards only:
      62256      - 32k x8 SRAM (SOP28)
      ADM202     - 2-channel RS-232 line driver/receiver
      CN3        - 8-pin connector
      BT1        - Battery

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
                           DL-2929 IOU SD08-1513  (QFP208) - I/O controller, next to 3.6864MHz XTAL.
                           DL-3329 SSU SD04-1536  (QFP144) - Sound chip, clocked at 21.47725MHz (42.9545/2). It has 32k
                                                             SRAM connected to it, probably also 'SS' foreground layer generator.
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
A and B cartridges also contain a FM1208S NVRAM, it is NOT used to save settings or game data, purpose is unknown.
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
      FM1208S      - RAMTRON FM1208S 4k (512bytes x8) Nonvolatile Ferroelectric RAM (not populated on type D boards)
      MACH111      - AMD MACH111 CPLD stamped 'CP3B1A' (PLCC44)
      *            - These components located on the other side of the PCB

      Note: The battery powers the CPU only. Some transistors/resistors present on the PCB and wired to the 74HC00
      switch the CPU from battery power to main power to increase the life of the battery.


Security cart resurrection info
-------------------------------

When the security cart dies the game no longer functions. The PCB can be brought back to life by doing the following
hardware modification to the security cart.....

1. Remove the custom QFP144 CPU and replace it with a standard Hitachi HD6417604 or HD6417095 SH-2 CPU
2. Remove the 29F400 TSOP48 flashROM and re-program it with the decrypted and modified main program ROM from set
   'cps3boot' in MAME. A 28F400 SOP44 flashROM can be used instead and mounted to the back side of the security cart
   PCB. Do not mount both SOP44 and TSOP48 flashROMs, use only one TSOP48 flashROM or one SOP44 flashROM.
3. Power on the PCB and using the built-in cart flashROM menu re-program the SIMMs for your chosen game using the CD
   from set 'cps3boot' in MAME.
4. That is all. Enjoy your working PCB.


Hardware registers info
----------------------

        PPU registers (read only)
        0x040C0000 - 0x040C000D

        Offset: Bits:               Desc:
        0C      ---- ---- ---- -2-- Palette DMA active   |
                ---- ---- ---- --1- Character DMA active | several parts of game code assume only 1 of these might be active at the same time
                ---- ---- ---- ---0 Sprite list DMA/copy active, see register 82 description

        PPU registers (write only)
        0x040C0000 - 0x040C00AF

        Offset: Bits:               Desc:
        00      ---- --xx xxxx xxxx Global Scroll 0 X
        02      ---- --xx xxxx xxxx Global Scroll 0 Y
        04-1F                   Global Scrolls 1-7
        20      xxxx xxxx xxxx xxxx Tilemap 0 Scroll X
        22      xxxx xxxx xxxx xxxx Tilemap 0 Scroll Y
        24      ---- -a98 76-- ---- Tilemap 0 ?? always 0
                ---- ---- ---4 3210 Tilemap 0 Width (in tiles)
        26      f--- ---- ---- ---- Tilemap 0 Enable
                -e-- ---- ---- ---- Tilemap 0 Line Scroll Enable
                --d- ---- ---- ---- Tilemap 0 Line Zoom Enable (seems unused in games, but might be enabled in jojo dev.menu BG test)
                ---c ---- ---- ---- Tilemap 0 ?? set together with Zoom
                ---- b--- ---- ---- Tilemap 0 Flip X
                ---- -a-- ---- ---- Tilemap 0 Flip Y
                ---- --98 7654 3210 Tilemap 0 ?? always 0
        28      -edc ba98 ---- ---- Tilemap 0 Line Scroll and Zoom Base address (1st word is scroll, 2nd word is zoom)
                ---- ---- -654 3210 Tilemap 0 Tiles Base address
        2A-2F   unused
        30-5F                   Tilemaps 1-3
                                                Values: 384     495 "wide"
        60      xxxx xxxx xxxx xxxx H Sync end*         42      35
        62      xxxx xxxx xxxx xxxx H Blank end         111     118
        64      xxxx xxxx xxxx xxxx H Screen end        495     613
        66      xxxx xxxx xxxx xxxx H Total end*        454     454
        68      ---- --xx xxxx xxxx H ?? Zoom Master?   0       0       +128 if flip screen, might be not zoom-related but global H scroll
        6A      xxxx xxxx xxxx xxxx H ?? Zoom Offset?   0       0
        6C      xxxx xxxx xxxx xxxx H ?? Zoom Size?     1023    1023    (511 at BIOS init)
        6E      xxxx xxxx xxxx xxxx H Zoom Scale        64      64
        70      xxxx xxxx xxxx xxxx V Sync end          3       3
        72      xxxx xxxx xxxx xxxx V Blank end         21      21
        74      xxxx xxxx xxxx xxxx V Screen end        245     245
        76      xxxx xxxx xxxx xxxx V Total end         262     262
        78      ---- --xx xxxx xxxx V ?? Zoom Master?   0       0       might be not zoom-related but global V scroll
        7A      xxxx xxxx xxxx xxxx V ?? Zoom Offset?   0       0
        7C      xxxx xxxx xxxx xxxx V ?? Zoom Size?     1023    1023    (261 at BIOS init)
        7E      xxxx xxxx xxxx xxxx V Zoom Scale        64      64
        80      ---- ---- ---- -210 Pixel clock         3       5       base clock is 42.954545MHz, 3 = /5 divider, 5 = /4 divider.
                ---- ---- ---4 3--- Flip screen X/Y (or Y/X)
                ---- ---- --5- ---- ?? always set to 1, 0 in unused 24KHz mode (pixel clock divider?)
                ---- ---- -6-- ---- ?? set to 0 by BIOS init, then set to 1 after video mode selection, 0 in unused 24KHz mode (pixel clock divider?)
                f--- ---- ---- ---- ?? always 0, but there is code which may set it
        82      ---- ---- ---- 3--0 Sprite list DMA/copy to onchip RAM ? after new list upload to sprite RAM games write here 8/9/8/9 pattern, then wait until register 0C bit 0 became 0, then write 0.
        84      ---- b--- ---- ---- ?? always set to 0x0800
        86      ---- ---- ---- 3210 Character RAM bank
        88      ---- ---- --54 3210 Gfx flash ROM bank
        8A      ---- ---- ---- ---- ?? set to 0 by BIOS init, never writen later
        8E      ---- ---- 7-5- ---- ?? set to 0x00A0 by BIOS init after Pal/Char DMA registers, never writen later (Char/Pal DMA IRQ enable ?)
        96      xxxx xxxx xxxx xxxx Character DMA Source low bits
        98      ---- ---- --54 3210 Character DMA Source high bits
                ---- ---- -6-- ---- Character DMA Start
        A0      ---- -a98 7654 3210 Palette DMA Source high bits
        A2      xxxx xxxx xxxx xxxx Palette DMA Source low bits
        A4      ---- ---- ---- ---0 Palette DMA Destination high bit
        A6      xxxx xxxx xxxx xxxx Palette DMA Destination low bits
        A8      -edc ba98 -654 3210 Palette DMA Fade low bits
        AA      ---- ---- -654 3210 Palette DMA Fade high bits
        AC      xxxx xxxx xxxx xxxx Palette DMA Length low bits
        AE      ---- ---- ---- ---0 Palette DMA Length high bit
                ---- ---- ---- --1- Palette DMA Start

        CRTC-related H/V values is last clock/line of given area, i.e. actual numbers is +1 to value, actual V Total is +2 to register value.

    (*) H Total value is same for all 15KHz modes, uses fixed clock (not affected by pixel clock modifier) -
        42.954545MHz/6 (similar to SSV) /(454+1) = 15734.25Hz /(262+2) = 59.59Hz
        unused 24KHz 512x384 mode uses H Total 293 V Total 424 (42.954545MHz/6 /(293+1) = 24350.62Hz /(424+2) = 57.16Hz)


        'SS' foreground tilemap layer generator (presumable located in 'SSU' chip) registers (write only?)
        0x05050000 - 0x05050029 area, even bytes only.

        Offset: Bits:       Desc:   Values: 384     495 "wide"
        00      xxxx xxxx   H Sync*         42      35      same as PPU
        01      xxxx xxxx   H Start L
        02      xxxx xxxx   H Start H       62      64
        03      xxxx xxxx   H Blank L
        04      xxxx xxxx   H Blank H       534     671
        05      xxxx xxxx   H Total L*
        06      xxxx xxxx   H Total H*      454     454*    same as PPU
        07      xxxx xxxx   H Scroll L
        08      xxxx xxxx   H Scroll H      -101    -107    +128 if flip screen
        09      xxxx xxxx   V Sync          3       3       same as PPU
        0a      xxxx xxxx   V Start L
        0b      xxxx xxxx   V Start H       21      21      same as PPU
        0c      xxxx xxxx   V Blank L
        0d      xxxx xxxx   V Blank H       247     247     PPU value +2
        0e      xxxx xxxx   V Total L
        0f      xxxx xxxx   V Total H       262     262     same as PPU
        10      xxxx xxxx   V Scroll L
        11      xxxx xxxx   V Scroll H      -24     -24     +288 if flip screen
        12      xxxx xxxx   Palette base
        13      ---- -210   Pixel clock     3       5       not clear how it works
        14      ---- --10   Flip screen X/Y (or Y/X?)

    (*) H Total value is same for all 15KHz modes, same as PPU.
*/

#include "emu.h"
#include "cdrom.h"
#include "machine/nvram.h"
#include "cps3.h"
#include "bus/nscsi/cd.h"
#include "machine/wd33c9x.h"
#include "screen.h"
#include "speaker.h"

#include <algorithm>

#include "sfiii2.lh"

#define DEBUG_PRINTF 0



#define DMA_XOR(a)      ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(1,2))

static constexpr u32 USER4REGION_LENGTH = 0x800000*2;
static constexpr u32 USER5REGION_LENGTH = 0x800000*10;

enum
{
	CPS3_TRANSPARENCY_NONE,
	CPS3_TRANSPARENCY_PEN,
	CPS3_TRANSPARENCY_PEN_INDEX,
	CPS3_TRANSPARENCY_PEN_INDEX_BLEND
};

inline void cps3_state::cps3_drawgfxzoom(bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx,
		u32 code,u32 color,int flipx,int flipy,int sx,int sy,
		int transparency,int transparent_color,
		int scalex, int scaley)
{
//  u8 al;

//  al = (pdrawgfx_shadow_lowpri) ? 0 : 0x80;

	if (!scalex || !scaley) return;

// todo: reimplement this optimization!!
//  if (scalex == 0x10000 && scaley == 0x10000)
//  {
//      cps3_drawgfx(dest_bmp,gfx,code,color,flipx,flipy,sx,sy,clip,transparency,transparent_color);
//      return;
//  }

	/*
	scalex and scaley are 16.16 fixed point numbers
	1<<15 : shrink to 50%
	1<<16 : uniform scale
	1<<17 : double to 200%
	*/


	/* force clip to bitmap boundary */
	rectangle myclip = clip;
	myclip &= dest_bmp.cliprect();

	/* 32-bit ONLY */
	{
		if (gfx)
		{
//          const pen_t *pal = &gfx->colortable[gfx->granularity() * (color % gfx->colors())];
			u32 palbase = (gfx->granularity() * color) & 0x1ffff;
			const pen_t *pal = &m_mame_colours[palbase];
			const u8 *source_base = gfx->get_data(code % gfx->elements());

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

				if (flipx)
				{
					x_index_base = (sprite_screen_width-1)*dx;
					dx = -dx;
				}
				else
				{
					x_index_base = 0;
				}

				if (flipy)
				{
					y_index = (sprite_screen_height-1)*dy;
					dy = -dy;
				}
				else
				{
					y_index = 0;
				}

				if (sx < myclip.left())
				{ /* clip left */
					int pixels = myclip.left()-sx;
					sx += pixels;
					x_index_base += pixels*dx;
				}
				if (sy < myclip.top())
				{ /* clip top */
					int pixels = myclip.top()-sy;
					sy += pixels;
					y_index += pixels*dy;
				}
				if (ex > myclip.right()+1)
				{ /* clip right */
					int pixels = ex-myclip.right()-1;
					ex -= pixels;
				}
				if (ey > myclip.bottom()+1)
				{ /* clip bottom */
					int pixels = ey-myclip.bottom()-1;
					ey -= pixels;
				}

				if (ex > sx)
				{ /* skip if inner loop doesn't draw anything */
					if (transparency == CPS3_TRANSPARENCY_NONE)
					{
						for (int y = sy; y < ey; y++)
						{
							u8 const *const source = source_base + (y_index>>16) * gfx->rowbytes();
							u32 *const dest = &dest_bmp.pix(y);

							int x_index = x_index_base;
							for (int x = sx; x < ex; x++)
							{
								dest[x] = pal[source[x_index>>16]];
								x_index += dx;
							}
							y_index += dy;
						}
					}
					else if (transparency == CPS3_TRANSPARENCY_PEN)
					{
						for (int y = sy; y < ey; y++)
						{
							u8 const *const source = source_base + (y_index>>16) * gfx->rowbytes();
							u32 *const dest = &dest_bmp.pix(y);

							int x_index = x_index_base;
							for (int x = sx; x < ex; x++)
							{
								int c = source[x_index>>16];
								if (c != transparent_color) dest[x] = pal[c];
								x_index += dx;
							}
							y_index += dy;
						}
					}
					else if (transparency == CPS3_TRANSPARENCY_PEN_INDEX)
					{
						for (int y = sy; y < ey; y++)
						{
							u8 const *const source = source_base + (y_index>>16) * gfx->rowbytes();
							u32 *const dest = &dest_bmp.pix(y);

							int x_index = x_index_base;
							for (int x = sx; x < ex; x++)
							{
								int c = source[x_index>>16];
								if (c != transparent_color) dest[x] = c | palbase;
								x_index += dx;
							}
							y_index += dy;
						}
					}
					else if (transparency == CPS3_TRANSPARENCY_PEN_INDEX_BLEND)
					{
						for (int y = sy; y < ey; y++)
						{
							u8 const *const source = source_base + (y_index>>16) * gfx->rowbytes();
							u32 *const dest = &dest_bmp.pix(y);

							int x_index = x_index_base;
							for (int x = sx; x < ex; x++)
							{
								int c = source[x_index>>16];
								if (c != transparent_color)
								{
									/* blending isn't 100% understood */
									// is it really ORed or bits should be replaced same as in Seta/SSV hardware ? both produce same results in games
									if (gfx->granularity() == 64)
									{
										// OK for sfiii world map spotlight
#if 1
										dest[x] |= (c & 0xf) << 13;
#else
										dest[x] = (dest[x] & 0x01fff) | ((c & 0xf) << 13);
#endif
									}
									else
									{
										// OK for jojo intro, and warzard swords, and various shadows in sf games
#if 1
										dest[x] |= ((c & 1) << 15) | ((color & 1) << 16);
#else
										dest[x] = (dest[x] & 0x07fff) | ((c & 1) << 15) | ((color & 1) << 16);
#endif
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



/* Encryption */


u16 cps3_state::rotate_left(u16 value, int n)
{
	int aux = value>>(16-n);
	return ((value<<n)|aux)%0x10000;
}

u16 cps3_state::rotxor(u16 val, u16 xorval)
{
	u16 res = val + rotate_left(val,2);

	res = rotate_left(res,4) ^ (res & (val ^ xorval));

	return res;
}

u32 cps3_state::cps3_mask(u32 address, u32 key1, u32 key2)
{
	// ignore all encryption
	if (m_altEncryption == 2)
		return 0;

	address ^= key1;

	u16 val = (address & 0xffff) ^ 0xffff;

	val = rotxor(val, key2 & 0xffff);

	val ^= (address >> 16) ^ 0xffff;

	val = rotxor(val, key2 >> 16);

	val ^= (address & 0xffff) ^ (key2 & 0xffff);

	return val | (val << 16);
}

void cps3_state::decrypt_bios()
{
	u32 *coderegion = (u32*)memregion("bios")->base();
	u32 codelength = memregion("bios")->bytes();

	for (int i = 0; i<codelength; i +=4)
	{
		u32 dword = coderegion[i/4];
		u32 xormask = cps3_mask(i, m_key1, m_key2);
		coderegion[i/4] = dword ^ xormask;
	}

	// Dump to file
	if (0)
	{
		auto filename = std::string(machine().system().name) + "_bios.dump";
		auto fp = fopen(filename.c_str(), "w+b");
		if (fp)
		{
			fwrite(coderegion, codelength, 1, fp);
			fclose(fp);
		}
	}
}


void cps3_state::init_crypt(u32 key1, u32 key2, int altEncryption)
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
		m_user4_allocated = std::make_unique<u8[]>(USER4REGION_LENGTH);
		m_user4 = m_user4_allocated.get();
	}

	if (m_user5_region)
	{
		m_user5 = m_user5_region->base();
	}
	else
	{
		m_user5_allocated = std::make_unique<u8[]>(USER5REGION_LENGTH);
		m_user5 = m_user5_allocated.get();
	}

	m_cps3sound->set_base((s8*)m_user5);

	// set strict verify
	m_maincpu->sh2drc_set_options(SH2DRC_STRICT_VERIFY);
	m_maincpu->sh2drc_add_fastram(0x02000000, 0x0207ffff, 0, &m_mainram[0]);
	m_maincpu->sh2drc_add_fastram(0x04000000, 0x0407ffff, 0, &m_spriteram[0]);
	m_maincpu->sh2drc_add_fastram(0x040C0020, 0x040C005f, 0, &m_tilemap_regs[0]);

	decrypt_bios();
}

void cps3_state::init_redearth()  { init_crypt(0x9e300ab1, 0xa175b82c, 0); }
void cps3_state::init_sfiii()     { init_crypt(0xb5fe053e, 0xfc03925a, 0); }
void cps3_state::init_sfiii2()    { init_crypt(0x00000000, 0x00000000, 1); } // sfiii2 runs off a 'dead' cart
void cps3_state::init_jojo()      { init_crypt(0x02203ee3, 0x01301972, 0); }
void cps3_state::init_sfiii3()    { init_crypt(0xa55432b4, 0x0c129981, 0); }
void cps3_state::init_jojoba()    { init_crypt(0x23323ee3, 0x03021972, 0); }
void cps3_state::init_cps3boot()  { init_crypt((u32)-1,(u32)-1,2); }



/* GFX decodes */


static const gfx_layout cps3_tiles16x16_layout =
{
	16,16,
	0x8000,
	8,
	{ STEP8(0,1) },
	{ 3*8,2*8,1*8,0*8,7*8,6*8,5*8,4*8,
		11*8,10*8,9*8,8*8,15*8,14*8,13*8,12*8 },
	{ STEP16(0,8*16) },
	8*256
};



static const gfx_layout cps3_tiles8x8_layout =
{
	8,8,
	0x200,
	4,
	{ STEP4(0,1) },
	{ 1*4,0*4,3*4,2*4,5*4,4*4,7*4,6*4 },
	{ STEP8(0,8*4) },
	64*4
};

static inline u8 get_fade(int c, int f)
{
	// bit 7 unused, explicit masked out
	// bit 6 fade enable / disable
	// bit 5 fade mode (1 = invert input values and output)
	// bit 4-0 fade value
	if (f & 0x40) // Fading enable / disable
		c = (f & 0x20) ? ((((c ^ 0x1f) * (~f & 0x1f)) >> 5) ^ 0x1f) : (c * (f & 0x1f) >> 5);
	return c;
}

void cps3_state::set_mame_colours(int colournum, u16 data, u32 fadeval)
{
	int r = (data >> 0) & 0x1f;
	int g = (data >> 5) & 0x1f;
	int b = (data >> 10) & 0x1f;

	/* is this 100% correct? */
	if (fadeval & 0x40400040)
	{
		r = get_fade(r, (fadeval & 0x7f000000)>>24);
		g = get_fade(g, (fadeval & 0x007f0000)>>16);
		b = get_fade(b, (fadeval & 0x0000007f)>>0);

		data = (data & 0x8000) | (r << 0) | (g << 5) | (b << 10);
	}

	colournum &= 0x1ffff;
	m_colourram[colournum] = data;

	m_mame_colours[colournum] = rgb_t(r << 3, g << 3, b << 3); // no color extension, VideoDAC's RGB bits 0-2 connected to GND

	if (colournum < 0x10000) m_palette->set_pen_color(colournum,m_mame_colours[colournum]/* rgb_t(r<<3,g<<3,b<<3)*/);//m_mame_colours[colournum]);
}


void cps3_state::video_start()
{
	m_char_ram = make_unique_clear<u32[]>(0x800000/4);
	m_mame_colours = make_unique_clear<u32[]>(0x20000);
	m_ss_ram = make_unique_clear<u8[]>(0x8000);
	m_spritelist = make_unique_clear<u32[]>(0x80000/4);

	m_spritelist[0] = 0x80000000;

	/* create the char set (gfx will then be updated dynamically from RAM) */
	m_gfxdecode->set_gfx(0, std::make_unique<gfx_element>(m_palette, cps3_tiles8x8_layout, &m_ss_ram[0x4000], 0, m_palette->entries() / 16, 0));

	/* create the char set (gfx will then be updated dynamically from RAM) */
	m_gfxdecode->set_gfx(1, std::make_unique<gfx_element>(m_palette, cps3_tiles16x16_layout, (u8 *)m_char_ram.get(), 0, m_palette->entries() / 64, 0));
	m_gfxdecode->gfx(1)->set_granularity(64);

	m_screenwidth = 384;

	// the renderbuffer can be twice the size of the screen, this allows us to handle framebuffer zoom values
	// between 0x00 and 0x80 (0x40 is normal, 0x80 would be 'view twice as much', 0x20 is 'view half as much')
	m_renderbuffer_bitmap.allocate(512*2,224*2);

	m_renderbuffer_clip.set(0, m_screenwidth-1, 0, 224-1);

	m_renderbuffer_bitmap.fill(0x3f, m_renderbuffer_clip);

	save_pointer(NAME(m_char_ram), 0x800000/4);
	save_pointer(NAME(m_mame_colours), 0x20000);
	save_pointer(NAME(m_ss_ram), 0x8000);
	save_pointer(NAME(m_spritelist), 0x80000/4);
}

void cps3_state::draw_tilemapsprite_line(u32 *regs, int drawline, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!(regs[1] & 0x00008000)) return;

	int scrollx =            (regs[0] & 0xffff0000) >> 16;
	int scrolly =            (regs[0] & 0x0000ffff) >> 0;
	bool linescroll_enable = (regs[1] & 0x00004000) >> 14;
	int global_flip_x =      (regs[1] & 0x00000800) >> 11; // warzard special moves
	int global_flip_y =      (regs[1] & 0x00000400) >> 10; // sfiii2 loading screens (capcom background and title logo during flashing)
	u32 linebase =           (regs[2] & 0x7f000000) >> 24;
	u32 mapbase =            (regs[2] & 0x007f0000) >> 16;

	mapbase <<= 10;
	linebase <<= 10;
	scrolly += 4;

	int line = drawline + scrolly;
	line &= 0x3ff;

	if (global_flip_y) line ^= 0x3ff;   // these probably needs compensation of our scrolly and tileline tweaks, but it's fine for sfiii2.
	int xflip_mask = (global_flip_x) ? 0x3f : 0;

	int tileline = (line / 16) + 1;
	int tilesubline = line % 16;

	if (linescroll_enable)
		scrollx += (m_spriteram[linebase + ((line + 16) & 0x3ff)] >> 16) & 0x3ff; // test case: sfiii Ryu's stage 2nd round floor

	rectangle clip(cliprect.left(), cliprect.right(), drawline, drawline);

	for (int x = cliprect.left() / 16; x < (cliprect.right() / 16) + 2; x++)
	{
		u32 const dat = m_spriteram[mapbase + ((tileline & 63) * 64) + (((x + scrollx / 16) & 63) ^ xflip_mask)];
		u32 const tileno = (dat & 0xfffe0000) >> 17;
		//u32 tilenoH =    (dat & 0x00008000) >> 15; // games put here tile number's bit 16, probably for (unreleased) mobos with expanded to 16Mbyte character RAM
		int xflip =        (dat & 0x00001000) >> 12;
		bool const yflip = (dat & 0x00000800) >> 11;
		bool const alpha = (dat & 0x00000400) >> 10; // enabled at jojo's "country town" and "in air plane" stages, but associated tile is empty - shadowing have no effect, why ?
		bool const bpp =   (dat & 0x00000200) >> 9;
		u32 const colour = (dat & 0x000001ff) >> 0;

		int trans = alpha ? CPS3_TRANSPARENCY_PEN_INDEX_BLEND : CPS3_TRANSPARENCY_PEN_INDEX;

		m_gfxdecode->gfx(1)->set_granularity(bpp ? 64 : 256);

		xflip ^= xflip_mask & 1;

		cps3_drawgfxzoom(bitmap, clip, m_gfxdecode->gfx(1), tileno, colour, xflip, yflip, (x * 16) - scrollx % 16, drawline - tilesubline, trans, 0, 0x10000, 0x10000);
	}
}

// fg layer (TODO: this could be handled with an actual tilemap)
void cps3_state::draw_fg_layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int scrolly = (-m_ss_vscroll) & 0x100; // TODO properly handle scroll value

	for (int line = cliprect.top(); line <= cliprect.bottom(); line++)
	{
		rectangle clip = cliprect;
		clip.min_y = clip.max_y = line;

		int y = line / 8;
		int offset = ((line + scrolly) / 8 * 128) & 0x1fff;

		// 'combo meter' in JoJo games uses rowscroll
		int rowscroll = m_ss_ram[((line + scrolly - 1) & 0x1ff)*2 + 0x2000];

		for (int x = 0; x < 64; x++)
		{
			u16 data = m_ss_ram[offset] | (m_ss_ram[offset + 1] << 8) ;
			u32 tile =  (data & 0x01ff) >> 0;
			int pal =   (data & 0x3e00) >> 9;
			int flipy = (data & 0x4000) >> 14;
			int flipx = (data & 0x8000) >> 15;
			pal += m_ss_pal_base << 5;

			cps3_drawgfxzoom(bitmap, clip, m_gfxdecode->gfx(0), tile, pal, flipx, flipy, (x * 8) - rowscroll, y * 8, CPS3_TRANSPARENCY_PEN, 0, 0x10000, 0x10000);
			cps3_drawgfxzoom(bitmap, clip, m_gfxdecode->gfx(0), tile, pal, flipx, flipy, 512 + (x * 8) - rowscroll, y * 8, CPS3_TRANSPARENCY_PEN, 0, 0x10000, 0x10000);

			offset += 2;
		}
	}
}

u32 cps3_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int width = ((m_ppu_crtc_zoom[1] & 0xffff0000) >> 16) - (m_ppu_crtc_zoom[0] & 0xffff);
	if (width > 0 && m_screenwidth != width)
	{
		attoseconds_t period = screen.frame_period().attoseconds();
		rectangle visarea = screen.visible_area();

		int height = ((m_ppu_crtc_zoom[5] & 0xffff0000) >> 16) - (m_ppu_crtc_zoom[4] & 0xffff);
		visarea.set(0, width - 1, 0, height - 1);
		screen.configure(width, height, visarea, period);
		m_screenwidth = width;
	}

	u32 fullscreenzoomx = m_ppu_crtc_zoom[3] & 0x000000ff;
	u32 fullscreenzoomy = m_ppu_crtc_zoom[7] & 0x000000ff;
	/* clamp at 0x80, I don't know if this is accurate */
	if (fullscreenzoomx > 0x80) fullscreenzoomx = 0x80;
	if (fullscreenzoomy > 0x80) fullscreenzoomy = 0x80;

	u32 fszx = (fullscreenzoomx << 16) / 0x40;
	u32 fszy = (fullscreenzoomy << 16) / 0x40;

	if (fullscreenzoomx == 0x40 && fullscreenzoomy == 0x40)
	{
		m_renderbuffer_clip = cliprect;
	}
	else
	{
		m_renderbuffer_clip.set(
			(cliprect.left() * fszx) >> 16, (((cliprect.right() + 1) * fszx + 0x8000) >> 16) - 1,
			(cliprect.top() * fszy) >> 16, (((cliprect.bottom() + 1) * fszy + 0x8000) >> 16) - 1);
	}
	m_renderbuffer_bitmap.fill(0, m_renderbuffer_clip);

	/* Sprites */
	//logerror("Spritelist start:\n");
	for (int i = 0x00000 / 4; i < 0x2000 / 4; i += 4)
	{
		if (m_spritelist[i + 0] & 0x80000000)
			break;

		u8 const gscroll = (m_spritelist[i + 0] & 0x70000000) >> 28;
		u32 const length = (m_spritelist[i + 0] & 0x01ff0000) >> 16; // how many entries in the sprite table
		u32 start =        (m_spritelist[i + 0] & 0x00007ff0) >> 4;

		int const xpos =   (m_spritelist[i + 1] & 0x03ff0000) >> 16;
		int const ypos =    m_spritelist[i + 1] & 0x000003ff;

		bool const whichbpp =     (m_spritelist[i + 2] & 0x40000000) >> 30; // not 100% sure if this is right, jojo title / characters
		bool const whichpal =     (m_spritelist[i + 2] & 0x20000000) >> 29;
		u8 const global_xflip =   (m_spritelist[i + 2] & 0x10000000) >> 28;
		u8 const global_yflip =   (m_spritelist[i + 2] & 0x08000000) >> 27;
		bool const global_alpha = (m_spritelist[i + 2] & 0x04000000) >> 26; // all games except warzard
		bool const global_bpp =   (m_spritelist[i + 2] & 0x02000000) >> 25;
		u32 const global_pal =    (m_spritelist[i + 2] & 0x01ff0000) >> 16;
		//int const tilemapnum =  (m_spritelist[i + 2] & 0x00000030) >> 4; // jojo and jojoba only

		int const gscrollx = (m_ppu_gscroll_buff[gscroll] & 0x03ff0000) >> 16;
		int const gscrolly = (m_ppu_gscroll_buff[gscroll] & 0x000003ff) >> 0;
		start = (start * 0x100) >> 2;

		for (int j = 0; j < (length) * 4; j += 4)
		{
			u32 const value1 = (m_spritelist[start + j + 0]);
			u32 const value2 = (m_spritelist[start + j + 1]);
			u32 const value3 = (m_spritelist[start + j + 2]);

			static const int tilestable[4] = { 8,1,2,4 };

			u32 const tileno = (value1 & 0xfffe0000) >> 17;
			//u8 unk2000 =     (value1 & 0x00002000) >> 13); //? sfiii2/3 bonus stages - score numbers and balls icons, sfiii3 staff roll texts
			u8 flipx =         (value1 & 0x00001000) >> 12;
			u8 flipy =         (value1 & 0x00000800) >> 11;
			bool const alpha = (value1 & 0x00000400) >> 10; // warzard alpha effects
			bool const bpp =   (value1 & 0x00000200) >> 9;
			u32 const pal =    (value1 & 0x000001ff);

			int xpos2 = (value2 & 0x03ff0000) >> 16;
			int ypos2 = (value2 & 0x000003ff) >> 0;

			/* these are the sizes to actually draw */
			u32 const ysizedraw2 = ((value3 & 0x7f000000) >> 24) + 1;
			u32 const xsizedraw2 = ((value3 & 0x007f0000) >> 16) + 1;
			//u8 unk =             ((value3 & 0x00000300) >> 8); // 3 - sprites, 0 - tilemaps, enable X/Y zooms ?
			s8 ysize2 =            ((value3 & 0x0000000c) >> 2);
			s8 xsize2 =            ((value3 & 0x00000003) >> 0);

			if (ysize2 == 0)
			{
				//logerror("invalid sprite ysize of 0 tiles\n");
				continue;
			}

			if (xsize2 == 0) // xsize of 0 tiles seems to be a special command to draw tilemaps
			{
				int tilemapnum = ((value3 & 0x00000030) >> 4);
				u32* regs = &m_tilemap_regs[tilemapnum * 4];

				for (int yy = 0; yy < ysizedraw2; yy++)
				{
					// positioning similar to sprites Y coord, but relative to edge, not center
					int cury_pos = ypos2 + gscrolly - yy;
					cury_pos = ~cury_pos;
					cury_pos -= 18;
					cury_pos &= 0x3ff;

					if (cury_pos >= m_renderbuffer_clip.top() && cury_pos <= m_renderbuffer_clip.bottom())
						draw_tilemapsprite_line(regs, cury_pos, m_renderbuffer_bitmap, m_renderbuffer_clip);
				}
			}
			else
			{
				ysize2 = tilestable[ysize2];
				xsize2 = tilestable[xsize2];

				u32 const xinc = (xsizedraw2 << 16) / xsize2;
				u32 const yinc = (ysizedraw2 << 16) / ysize2;

				u32 xscale = xinc / 16;
				u32 yscale = yinc / 16;

				/* Let's approximate to the nearest greater integer value
				   to avoid holes in between tiles */
				if (xscale & 0xffff)    xscale += (1<<16) / 16;
				if (yscale & 0xffff)    yscale += (1<<16) / 16;

				xsize2 -= 1;
				ysize2 -= 1;

				flipx ^= global_xflip;
				flipy ^= global_yflip;

				if (!flipx) xpos2 += (xsizedraw2 / 2);
				else xpos2 -= (xsizedraw2 / 2);

				ypos2 += (ysizedraw2 / 2);

				if (!flipx) xpos2 -= ((xsize2 + 1) * xinc) >> 16;
				else  xpos2 += (xsize2 * xinc) >> 16;

				if (flipy) ypos2 -= (ysize2 * yinc) >> 16;

				/* use the palette value from the main list or the sublists? */
				int actualpal = whichpal ? global_pal : pal;

				/* use the bpp value from the main list or the sublists? */
				m_gfxdecode->gfx(1)->set_granularity((whichbpp ? global_bpp : bpp) ? 64 : 256);

				int trans = (global_alpha || alpha) ? CPS3_TRANSPARENCY_PEN_INDEX_BLEND : CPS3_TRANSPARENCY_PEN_INDEX;

				int count = 0;
				for (int xx = 0; xx < xsize2 + 1; xx++)
				{
					int current_xpos;

					if (!flipx) current_xpos = (xpos + xpos2 + ((xx * xinc) >> 16));
					else current_xpos = (xpos + xpos2 - ((xx * xinc) >> 16));
					//current_xpos +=  machine().rand() & 0x3ff;
					current_xpos += gscrollx;
					current_xpos += 1;
					current_xpos &= 0x3ff;
					if (current_xpos & 0x200) current_xpos -= 0x400;

					for (int yy = 0; yy < ysize2 + 1; yy++)
					{
						int current_ypos;

						if (flipy) current_ypos = (ypos + ypos2 + ((yy * yinc) >> 16));
						else current_ypos = (ypos + ypos2 - ((yy * yinc) >> 16));

						current_ypos += gscrolly;
						current_ypos = 0x3ff - current_ypos;
						current_ypos -= 17;
						current_ypos &= 0x3ff;

						if (current_ypos & 0x200) current_ypos -= 0x400;

						//if ( (whichbpp) && (m_screen->frame_number() & 1)) continue;

						cps3_drawgfxzoom(m_renderbuffer_bitmap, m_renderbuffer_clip, m_gfxdecode->gfx(1), tileno + count, actualpal, 0 ^ flipx, 0 ^ flipy, current_xpos, current_ypos, trans, 0, xscale, yscale);
						count++;
					}
				}
			}
		}
	}

	if (fullscreenzoomx == 0x40 && fullscreenzoomy == 0x40)
	{
		/* copy render bitmap without zoom */
		for (u32 rendery = cliprect.top(); rendery <= cliprect.bottom(); rendery++)
		{
			u32 *const dstbitmap = &bitmap.pix(rendery);
			u32 const *const srcbitmap = &m_renderbuffer_bitmap.pix(rendery);

			for (u32 renderx = cliprect.left(); renderx <= cliprect.right(); renderx++)
			{
				dstbitmap[renderx] = m_mame_colours[srcbitmap[renderx] & 0x1ffff];
			}
		}
	}
	else
	{
		/* copy render bitmap with zoom */
		u32 srcy = cliprect.top() * fszy;
		for (u32 rendery = cliprect.top(); rendery <= cliprect.bottom(); rendery++)
		{
			u32 *const dstbitmap = &bitmap.pix(rendery);
			u32 const *const srcbitmap = &m_renderbuffer_bitmap.pix(srcy >> 16);
			u32 srcx = cliprect.left() * fszx;

			for (u32 renderx = cliprect.left(); renderx <= cliprect.right(); renderx++)
			{
				dstbitmap[renderx] = m_mame_colours[srcbitmap[srcx >> 16] & 0x1ffff];
				srcx += fszx;
			}
			srcy += fszy;
		}
	}

	draw_fg_layer(screen, bitmap, cliprect);

	return 0;
}

/*
    SSRAM (only even bytes)
          0x0000 - 0x3fff tilemap layout
          0x4000 - 0x7fff rowscroll
          0x8000 - 0xffff tile character definitions
*/

u8 cps3_state::ssram_r(offs_t offset)
{
	return m_ss_ram[offset];
}

void cps3_state::ssram_w(offs_t offset, u8 data)
{
	if (offset >= 0x4000)
		m_gfxdecode->gfx(0)->mark_dirty((offset - 0x4000)/32);

	m_ss_ram[offset] = data;
}

void cps3_state::sh2cache_ram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA( &m_sh2cache_ram[offset] );
	// store a decrypted copy
	m_sh2cache_ram_decrypted[offset] = m_sh2cache_ram[offset]^cps3_mask(offset*4+0xc0000000, m_key1, m_key2);
}

void cps3_state::cram_bank_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		// this seems to be related to accesses to the 0x04100000 region
		if (m_cram_bank != data)
		{
			m_cram_bank = data;
		//if (data & 0xfffffff0)
		//bank_w 00000000, ffff0000
		//bank_w 00000001, ffff0000
		//bank_w 00000002, ffff0000
		//bank_w 00000003, ffff0000
		//bank_w 00000004, ffff0000
		//bank_w 00000005, ffff0000
		//bank_w 00000006, ffff0000
		//bank_w 00000007, ffff0000
		// during CHARACTER RAM test..
			if (DEBUG_PRINTF) logerror("bank_w %08x, %08x\n",data,mem_mask);

		}
	}
	else
	{
		if (DEBUG_PRINTF) logerror("bank_w LSB32 %08x, %08x\n",data,mem_mask);

	}
}

u32 cps3_state::cram_data_r(offs_t offset)
{
	u32 fulloffset = (((m_cram_bank & 0x7)*0x100000)/4) + offset;

	return little_endianize_int32(m_char_ram[fulloffset]);
}

void cps3_state::cram_data_w(offs_t offset, u32 data, u32 mem_mask)
{
	u32 fulloffset = (((m_cram_bank & 0x7)*0x100000)/4) + offset;
	mem_mask = little_endianize_int32(mem_mask);
	data = little_endianize_int32(data);
	COMBINE_DATA(&m_char_ram[fulloffset]);
	m_gfxdecode->gfx(1)->mark_dirty(fulloffset/0x40);
}

void cps3_state::spritedma_w(offs_t offset, u16 data, u16 mem_mask)
{
	u16 prev = m_spritelist_dma;
	COMBINE_DATA(&m_spritelist_dma);

	// display list DMA. actual DMA probably combine coordinates and control fields from main/sub list records and gscroll registers, we just save them for further processing.
	if (!(m_dma_status & 1) && (m_spritelist_dma & 9) == 8 && (prev & 9) == 9) // 0->1
	{
		for (int i = 0; i < 0x2000/4; i += 4)
		{
			std::copy(&m_spriteram[i], &m_spriteram[i + 4], &m_spritelist[i]); // copy main list record
			u32 dat = m_spriteram[i];
			if (dat & 0x80000000)
				break;
			u32 offs =   (dat & 0x00007fff) << 2;
			u32 length = (dat & 0x01ff0000) >> 16;
			std::copy(&m_spriteram[offs], &m_spriteram[offs + length*4], &m_spritelist[offs]); // copy sublist
		}
		std::copy(&m_ppu_gscroll[0], &m_ppu_gscroll[8], &m_ppu_gscroll_buff[0]);

		m_dma_status |= 1;
		m_spritelist_dma_timer->adjust(attotime::from_usec(4)); // slight delay to skip multiple 8/9 writes. actual DMA speed is unknown.
	}
}

/* FLASH ROM ACCESS */

u32 cps3_state::gfxflash_r(offs_t offset, u32 mem_mask)
{
	u32 result = 0;
	if (m_cram_gfxflash_bank&1) offset += 0x200000/4;

	fujitsu_29f016a_device *chip0 = m_simm[2 + m_cram_gfxflash_bank/8][(m_cram_gfxflash_bank % 8) & ~1].target();
	fujitsu_29f016a_device *chip1 = m_simm[2 + m_cram_gfxflash_bank/8][(m_cram_gfxflash_bank % 8) |  1].target();
	if (chip0 == nullptr || chip1 == nullptr)
		return 0xffffffff;

	if (DEBUG_PRINTF) logerror("gfxflash_r\n");

	if (ACCESSING_BITS_24_31)   // GFX Flash 1
	{
		//logerror("read GFX flash chip %s addr %02x\n", chip0->tag(), (offset<<1));
		result |= chip0->read(offset<<1) << 24;
	}
	if (ACCESSING_BITS_16_23)   // GFX Flash 2
	{
		//logerror("read GFX flash chip %s addr %02x\n", chip1->tag(), (offset<<1));
		result |= chip1->read(offset<<1) << 16;
	}
	if (ACCESSING_BITS_8_15)    // GFX Flash 1
	{
		//logerror("read GFX flash chip %s addr %02x\n", chip0->tag(), (offset<<1)+1);
		result |= chip0->read((offset<<1)+0x1) << 8;
	}
	if (ACCESSING_BITS_0_7) // GFX Flash 2
	{
		//logerror("read GFX flash chip %s addr %02x\n", chip1->tag(), (offset<<1)+1);
		result |= chip1->read((offset<<1)+0x1) << 0;
	}

	//logerror("read GFX flash chips addr %02x returning %08x mem_mask %08x crambank %08x gfxbank %08x\n", offset*2, result,mem_mask,  m_cram_bank, m_cram_gfxflash_bank  );

	return result;
}

void cps3_state::gfxflash_w(offs_t offset, u32 data, u32 mem_mask)
{
	int command;
	if (m_cram_gfxflash_bank&1) offset += 0x200000/4;

	fujitsu_29f016a_device *chip0 = m_simm[2 + m_cram_gfxflash_bank/8][(m_cram_gfxflash_bank % 8) & ~1];
	fujitsu_29f016a_device *chip1 = m_simm[2 + m_cram_gfxflash_bank/8][(m_cram_gfxflash_bank % 8) |  1];
	if (chip0 == nullptr || chip1 == nullptr)
		return;

//  if (DEBUG_PRINTF) logerror("gfxflash_w %08x %08x %08x\n", offset *2, data, mem_mask);


	if (ACCESSING_BITS_24_31)   // GFX Flash 1
	{
		command = (data >> 24) & 0xff;
		//logerror("write to GFX flash chip %s addr %02x cmd %02x\n", chip0->tag(), (offset<<1), command);
		chip0->write((offset<<1), command);
	}
	if (ACCESSING_BITS_16_23)   // GFX Flash 2
	{
		command = (data >> 16) & 0xff;
		//logerror("write to GFX flash chip %s addr %02x cmd %02x\n", chip1->tag(), (offset<<1), command);
		chip1->write((offset<<1), command);
	}
	if (ACCESSING_BITS_8_15)    // GFX Flash 1
	{
		command = (data >> 8) & 0xff;
		//logerror("write to GFX flash chip %s addr %02x cmd %02x\n", chip0->tag(), (offset<<1)+1, command);
		chip0->write((offset<<1)+0x1, command);
	}
	if (ACCESSING_BITS_0_7) // GFX Flash 2
	{
		command = (data >> 0) & 0xff;
		//if ( ((offset<<1)+1) != 0x555) logerror("write to GFX flash chip %s addr %02x cmd %02x\n", chip1->tag(), (offset<<1)+1, command);
		chip1->write((offset<<1)+0x1, command);
	}

	/* make a copy in the linear memory region we actually use for drawing etc.  having it stored in interleaved flash roms isnt' very useful */
	{
		u32* romdata = (u32*)m_user5;

		u32 const real_offset = ((m_cram_gfxflash_bank & 0x3e) * 0x200000) + offset*4;

		u32 const newdata =((chip0->read_raw(((offset*2) & 0xfffffffe)+0)<<8) |
					(chip0->read_raw(((offset*2) & 0xfffffffe)+1)<<24) |
					(chip1->read_raw(((offset*2) & 0xfffffffe)+0)<<0)  |
					(chip1->read_raw(((offset*2) & 0xfffffffe)+1)<<16));

//      logerror("flashcrap %08x %08x %08x\n", offset *2, romdata[real_offset/4], newdata);
		romdata[real_offset/4] = newdata;
	}
}



u32 cps3_state::flashmain_r(int which, u32 offset, u32 mem_mask)
{
	u32 result = 0;

	if (m_simm[which][0] == nullptr || m_simm[which][1] == nullptr || m_simm[which][2] == nullptr || m_simm[which][3] == nullptr)
		return 0xffffffff;

	if (ACCESSING_BITS_24_31)   // Flash 1
	{
		//logerror("read flash chip %d addr %02x\n", base+0, offset*4 );
		result |= (m_simm[which][0]->read(offset)<<24);
	}
	if (ACCESSING_BITS_16_23)   // Flash 1
	{
		//logerror("read flash chip %d addr %02x\n", base+1, offset*4 );
		result |= (m_simm[which][1]->read(offset)<<16);
	}
	if (ACCESSING_BITS_8_15)    // Flash 1
	{
		//logerror("read flash chip %d addr %02x\n", base+2, offset*4 );
		result |= (m_simm[which][2]->read(offset)<<8);
	}
	if (ACCESSING_BITS_0_7) // Flash 1
	{
		//logerror("read flash chip %d addr %02x\n", base+3, offset*4 );
		result |= (m_simm[which][3]->read(offset)<<0);
	}

//  if (base==4) logerror("read flash chips addr %02x returning %08x\n", offset*4, result );

	return result;
}



u32 cps3_state::flash1_r(offs_t offset, u32 mem_mask)
{
	u32 retvalue = flashmain_r(0, offset, mem_mask);

	if (m_altEncryption) return retvalue;

	retvalue = retvalue ^ cps3_mask(0x6000000+offset*4, m_key1, m_key2);
	return retvalue;
}

u32 cps3_state::flash2_r(offs_t offset, u32 mem_mask)
{
	u32 retvalue = flashmain_r(1, offset, mem_mask);

	if (m_altEncryption) return retvalue;

	retvalue = retvalue ^ cps3_mask(0x6800000+offset*4, m_key1, m_key2);
	return retvalue;
}

void cps3_state::flashmain_w(int which, u32 offset, u32 data, u32 mem_mask)
{
	u8 command;

	if (m_simm[which][0] == nullptr || m_simm[which][1] == nullptr || m_simm[which][2] == nullptr || m_simm[which][3] == nullptr)
		return;

	if (ACCESSING_BITS_24_31)   // Flash 1
	{
		command = (data >> 24) & 0xff;
		//logerror("write to flash chip %s addr %02x cmd %02x\n", m_simm[which][0]->tag(), offset, command);
		m_simm[which][0]->write(offset, command);
	}
	if (ACCESSING_BITS_16_23)   // Flash 2
	{
		command = (data >> 16) & 0xff;
		//logerror("write to flash chip %s addr %02x cmd %02x\n", m_simm[which][1]->tag(), offset, command);
		m_simm[which][1]->write(offset, command);
	}
	if (ACCESSING_BITS_8_15)    // Flash 2
	{
		command = (data >> 8) & 0xff;
		//logerror("write to flash chip %s addr %02x cmd %02x\n", m_simm[which][2]->tag(), offset, command);
		m_simm[which][2]->write(offset, command);
	}
	if (ACCESSING_BITS_0_7) // Flash 2
	{
		command = (data >> 0) & 0xff;
		//logerror("write to flash chip %s addr %02x cmd %02x\n", m_simm[which][3]->tag(), offset, command);
		m_simm[which][3]->write(offset, command);
	}

	/* copy data into regions to execute from */
	{
		u32* romdata =  (u32*)m_user4;
		u32* romdata2 = (u32*)m_decrypted_gamerom;

		u32 real_offset = offset * 4;

		if (which==1)
		{
			romdata += 0x800000/4;
			romdata2 += 0x800000/4;
			real_offset += 0x800000;
		}

		u32 const newdata = (m_simm[which][0]->read_raw(offset)<<24) |
					(m_simm[which][1]->read_raw(offset)<<16) |
					(m_simm[which][2]->read_raw(offset)<<8) |
					(m_simm[which][3]->read_raw(offset)<<0);

		//logerror("%08x %08x %08x %08x %08x\n",offset, romdata2[offset], romdata[offset], newdata,  newdata^cps3_mask(0x6000000+real_offset, m_key1, m_key2)  );

		romdata[offset] = newdata;
		romdata2[offset] = newdata^cps3_mask(0x6000000+real_offset, m_key1, m_key2);
	}
}

void cps3_state::flash1_w(offs_t offset, u32 data, u32 mem_mask)
{
	flashmain_w(0,offset,data,mem_mask);
}

void cps3_state::flash2_w(offs_t offset, u32 data, u32 mem_mask)
{
	flashmain_w(1,offset,data,mem_mask);
}

void cps3_state::cram_gfxflash_bank_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_24_31)
	{
		//logerror("cram_gfxflash_bank_w MSB32 %08x\n",data);
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
		//logerror("cram_gfxflash_bank_LSB_w LSB32 %08x\n",data);
	}
}

u16 cps3_state::dma_status_r()
{
	return m_dma_status;
}

u16 cps3_state::dev_dipsw_r()
{
	// presumably these data came from serial interface populated on early boards
	// inverted words from 5000a00-5000a0f area ANDed with inverted words from 5000a10-5000a1f. perhaps one return DIPSW in 8 high bits, while other in 8 low bits.
	//  warzard will crash before booting if some of bits is not 0
	return 0xffff;
}

/* EEPROM access is a little odd, I think it accesses eeprom through some kind of
   additional interface, as these writes aren't normal for the type of eeprom we have */

u32 cps3_state::eeprom_r(offs_t offset, u32 mem_mask)
{
	int addr = offset*4;

	if (addr >= 0x100 && addr <= 0x17f)
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
			//if (DEBUG_PRINTF) logerror("reading %04x from eeprom\n", m_current_eeprom_read);
			return m_current_eeprom_read;
		}
	}
	else
	{
	//  if (DEBUG_PRINTF) logerror("unk read eeprom addr %04x, mask %08x\n", addr, mem_mask);
		return 0x00000000;
	}
}

void cps3_state::eeprom_w(offs_t offset, u32 data, u32 mem_mask)
{
	int addr = offset*4;

	if (addr >= 0x080 && addr <= 0x0ff)
	{
		offset -= 0x80/4;
		COMBINE_DATA(&m_eeprom[offset]);
		// write word to storage

	}
	else if (addr >= 0x180 && addr <= 0x1ff)
	{
		// write 0 before data word write, erase ? which also means above probably may only reset data bits.
	}
	else
	{
	//  if (DEBUG_PRINTF) logerror("unk write eeprom addr %04x, data %08x, mask %08x\n", addr, data, mem_mask);
	}

}

void cps3_state::outport_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		machine().bookkeeping().coin_lockout_w(0, ~data & 0x01);
		machine().bookkeeping().coin_lockout_w(1, ~data & 0x02);
		machine().bookkeeping().coin_counter_w(0, data & 0x10);
		machine().bookkeeping().coin_counter_w(1, data & 0x20);
	}
	// bits 14 and 15 some LEDs ?
}

void cps3_state::ssregs_w(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0x07:
		m_ss_hscroll = (m_ss_hscroll & 0xff00) | data;
		break;
	case 0x08:
		m_ss_hscroll = (m_ss_hscroll & 0xff) | (data << 8);
		break;
	case 0x10:
		m_ss_vscroll = (m_ss_vscroll & 0xff00) | data;
		break;
	case 0x11:
		m_ss_vscroll = (m_ss_vscroll & 0xff) | (data << 8);
		break;
	case 0x12:
		m_ss_pal_base = data;
		break;
	case 0x14:
		break;
	default:
		logerror("SS regs write %02X data %02X\n", offset, data);
		break;
	}
}

//<ElSemi> +0 X  +2 Y +4 unknown +6 enable ( & 0x8000) +8 low part tilemap base, high part linescroll base
//<ElSemi> (a word each)


void cps3_state::palettedma_w(offs_t offset, u32 data, u32 mem_mask)
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
				u16* src = (u16*)m_user5;
			//  if (DEBUG_PRINTF) logerror("CPS3 pal dma start %08x (real: %08x) dest %08x fade %08x other2 %08x (length %04x)\n", m_paldma_source, m_paldma_realsource, m_paldma_dest, m_paldma_fade, m_paldma_other2, m_paldma_length);

				u32 dmalen = m_paldma_length | ((data & 1) << 16);
				for (u32 i = 0; i < dmalen; i++)
				{
					u16 coldata = src[BYTE_XOR_BE(((m_paldma_realsource>>1)+i))];

					//if (m_paldma_fade!=0) logerror("%08x\n",m_paldma_fade);

					set_mame_colours((m_paldma_dest+i)^1, coldata, m_paldma_fade);
				}
				m_dma_status |= 4;
				m_dma_timer->adjust(attotime::from_usec(100)); // delay time is a hack, what is actual DMA speed?
			}
		}
	}
}


//static u8* current_table;




u32 cps3_state::process_byte( u8 real_byte, u32 destination, int max_length )
{
	u8* dest       = (u8*)m_char_ram.get();

	//logerror("process byte for destination %08x\n", destination);

	destination&=0x7fffff;

	if (real_byte & 0x40)
	{
		int tranfercount = 0;

		//logerror("Set RLE Mode\n");
		m_rle_length = (real_byte & 0x3f)+1;

		//logerror("RLE Operation (length %08x\n", m_rle_length );

		while (m_rle_length)
		{
			dest[((destination+tranfercount) & 0x7fffff)^3] = (m_last_normal_byte & 0x3f);
			m_gfxdecode->gfx(1)->mark_dirty(((destination+tranfercount) & 0x7fffff)/0x100);
			//logerror("RLE WRite Byte %08x, %02x\n", destination+tranfercount, real_byte);

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
		//logerror("Write Normal Data\n");
		dest[(destination & 0x7fffff)^3] = real_byte;
		m_last_normal_byte = real_byte;
		m_gfxdecode->gfx(1)->mark_dirty((destination & 0x7fffff)/0x100);
		return 1;
	}
}

void cps3_state::do_char_dma( u32 real_source, u32 real_destination, u32 real_length )
{
	u8* sourcedata = (u8*)m_user5;
	int length_remaining;

	m_last_normal_byte = 0;
	m_rle_length = 0;
	length_remaining = real_length;
	while (length_remaining)
	{
		u8 current_byte;

		current_byte = sourcedata[DMA_XOR(real_source)];
		real_source++;

		if (current_byte & 0x80)
		{
			u8 real_byte;
			u32 length_processed;
			current_byte &= 0x7f;

			real_byte = sourcedata[DMA_XOR((m_current_table_address+current_byte*2+0))];
			//if (real_byte & 0x80) return;
			length_processed = process_byte(real_byte, real_destination, length_remaining );
			length_remaining-=length_processed; // subtract the number of bytes the operation has taken
			real_destination+=length_processed; // add it onto the destination
			if (real_destination>0x7fffff) return;
			if (length_remaining<=0) return; // if we've expired, exit

			real_byte = sourcedata[DMA_XOR((m_current_table_address+current_byte*2+1))];
			//if (real_byte & 0x80) return;
			length_processed = process_byte(real_byte, real_destination, length_remaining );
			length_remaining-=length_processed; // subtract the number of bytes the operation has taken
			real_destination+=length_processed; // add it onto the destination
			if (real_destination>0x7fffff) return;
			if (length_remaining<=0) return;  // if we've expired, exit
		}
		else
		{
			u32 length_processed;
			length_processed = process_byte(current_byte, real_destination, length_remaining );
			length_remaining-=length_processed; // subtract the number of bytes the operation has taken
			real_destination+=length_processed; // add it onto the destination
			if (real_destination>0x7fffff) return;
			if (length_remaining<=0) return;  // if we've expired, exit
		}

//      length_remaining--;
	}
}

u32 cps3_state::ProcessByte8(u8 b,u32 dst_offset)
{
	u8* destRAM = (u8*)m_char_ram.get();
	int l=0;

	if (m_lastb==m_lastb2) //rle
	{
		int rle=(b+1) & 0xff;

		for (int i = 0; i<rle; ++i)
		{
			destRAM[(dst_offset & 0x7fffff)^3] = m_lastb;
			m_gfxdecode->gfx(1)->mark_dirty((dst_offset & 0x7fffff)/0x100);

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
		destRAM[(dst_offset & 0x7fffff)^3] = b;
		m_gfxdecode->gfx(1)->mark_dirty((dst_offset & 0x7fffff)/0x100);
		return 1;
	}
}

void cps3_state::do_alt_char_dma( u32 src, u32 real_dest, u32 real_length )
{
	u8* px = (u8*)m_user5;
	u32 start = real_dest;
	u32 ds = real_dest;

	m_lastb=0xfffe;
	m_lastb2=0xffff;

	while(1)
	{
		u8 ctrl=px[DMA_XOR(src)];
		++src;

		for (int i = 0; i<8; ++i)
		{
			u8 p=px[DMA_XOR(src)];

			if (ctrl & 0x80)
			{
				u8 real_byte;
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

			if ((ds-start)>=real_length)
				return;
		}
	}
}

void cps3_state::process_character_dma(u32 address)
{
	//logerror("charDMA start:\n");

	for (int i = 0; i < 0x1000; i += 3)
	{
		u32 dat1 = little_endianize_int32(m_char_ram[i + 0 + (address)]);
		u32 dat2 = little_endianize_int32(m_char_ram[i + 1 + (address)]);
		u32 dat3 = little_endianize_int32(m_char_ram[i + 2 + (address)]);

		// 0x01000000 is the end of list marker
		if (dat1 & 0x01000000) break;

		u32 dma_command      = (dat1 & 0x00e00000) >> 21;
		u32 real_length      = (((dat1 & 0x001fffff) + 1) << 3); // for command 4 here is number of 16bit words -1
		u32 real_destination = dat2 << 3;
		u32 real_source      = (dat3 << 1) - 0x400000;

		//logerror("%08x %08x %08x real_source %08x (rom %d offset %08x) real_destination %08x, real_length %08x\n", dat1, dat2, dat3, real_source, real_source/0x800000, real_source%0x800000, real_destination, real_length);

		switch (dma_command)
		{
		case 4: /* Sets a table used by the decompression routines */
			/* We should probably copy this, but a pointer to it is fine for our purposes as the data doesn't change */
			m_current_table_address = real_source;
			break;
		case 2: /* 6bpp DMA decompression
		      - this is used for the majority of sprites and backgrounds */
			do_char_dma(real_source, real_destination, real_length);
			break;
		case 3: /* 8bpp DMA decompression
		      - this is used on SFIII NG Sean's Stage ONLY */
			do_alt_char_dma(real_source, real_destination, real_length);
			break;
		case 0: // not compressed DMA
			// warzard use this at stage's start, code looks intent, not a game bug
			for (u8* dest = (u8*)m_char_ram.get(); real_length; --real_length)
			{
				dest[(real_destination & 0x7fffff) ^ 3] = m_user5[DMA_XOR(real_source)];
				m_gfxdecode->gfx(1)->mark_dirty((real_destination & 0x7fffff) / 0x100);
				real_source++;
				real_destination++;
			}
			break;
		default:
			logerror("Unknown DMA List Command Type %d\n", dma_command);
			break;
		}
	}
	m_dma_status |= 2;
	m_dma_timer->adjust(attotime::from_usec(100)); // delay time is a hack, what is actual DMA speed?
}

void cps3_state::characterdma_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (DEBUG_PRINTF) logerror("chardma_w %08x %08x %08x\n", offset, data, mem_mask);

	if (offset==0)
	{
		//COMBINE_DATA(&m_chardma_source);
		if (ACCESSING_BITS_0_7)
		{
			m_chardma_source = data & 0x0000ffff;
		}
		if (ACCESSING_BITS_24_31)
		{
			if (DEBUG_PRINTF) logerror("chardma_w accessing MSB32 of offset 0");
		}
	}
	else if (offset==1)
	{
		COMBINE_DATA(&m_chardma_other);

		if (ACCESSING_BITS_24_31)
		{
			if ((data>>16) & 0x0040)
			{
				u32 list_address;
				list_address = (m_chardma_source | ((m_chardma_other & 0x003f0000)));

				//logerror("chardma_w activated %08x %08x (address = cram %08x)\n", m_chardma_source, m_chardma_other, list_address*4 );
				process_character_dma(list_address);
			}
			else
			{
				if (DEBUG_PRINTF) logerror("chardma_w NOT activated %08x %08x\n", m_chardma_source, m_chardma_other );
			}

			if ((data>>16) & 0xff80)
				if (DEBUG_PRINTF) logerror("chardma_w unknown bits in activate command %08x %08x\n", m_chardma_source, m_chardma_other );
		}
		else
		{
			if (DEBUG_PRINTF) logerror("chardma_w LSB32 write to activate command %08x %08x\n", m_chardma_source, m_chardma_other );
		}
	}
}

u16 cps3_state::colourram_r(offs_t offset)
{
	return m_colourram[offset];
}

void cps3_state::colourram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_colourram[offset]);

	set_mame_colours(offset, m_colourram[offset], 0);
}


/* there are more unknown writes, but you get the idea */
void cps3_state::cps3_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).rom().region("bios", 0); // BIOS ROM
	map(0x02000000, 0x0207ffff).ram().share("mainram"); // Main RAM
	map(0x03000000, 0x030003ff).ram(); // 'FRAM' (sfiii and warzard memory test mode ONLY, and only odd bytes)

	map(0x04000000, 0x0407ffff).ram().share("spriteram"); // Sprite RAM
	map(0x04080000, 0x040bffff).rw(FUNC(cps3_state::colourram_r), FUNC(cps3_state::colourram_w));  // Colour RAM 0x20000 colours
	// PPU registers
	map(0x040c0000, 0x040c0007).nopr(); // ?? warzard reads this but not use values, dev/debug leftovers ?
	map(0x040c000c, 0x040c000d).r(FUNC(cps3_state::dma_status_r));
	map(0x040c0000, 0x040c001f).writeonly().share("ppu_gscroll_regs");
	map(0x040c0020, 0x040c005f).writeonly().share("ppu_tmap_regs");
	map(0x040c0060, 0x040c007f).writeonly().share("ppu_crtc_zoom");
	map(0x040c0080, 0x040c0083).w(FUNC(cps3_state::spritedma_w)).umask32(0x0000ffff);
	map(0x040c0084, 0x040c0087).w(FUNC(cps3_state::cram_bank_w));
	map(0x040c0088, 0x040c008b).w(FUNC(cps3_state::cram_gfxflash_bank_w));
	map(0x040c0094, 0x040c009b).w(FUNC(cps3_state::characterdma_w));
	map(0x040c00a0, 0x040c00af).w(FUNC(cps3_state::palettedma_w));

	map(0x040e0000, 0x040e02ff).rw(m_cps3sound, FUNC(cps3_sound_device::sound_r), FUNC(cps3_sound_device::sound_w));

	map(0x04100000, 0x041fffff).rw(FUNC(cps3_state::cram_data_r), FUNC(cps3_state::cram_data_w));
	map(0x04200000, 0x043fffff).rw(FUNC(cps3_state::gfxflash_r), FUNC(cps3_state::gfxflash_w)); // GFX Flash ROMS

	map(0x05000000, 0x05000003).portr("INPUTS");
	map(0x05000004, 0x05000007).portr("EXTRA");
	map(0x05000008, 0x05000009).w(FUNC(cps3_state::outport_w));

	map(0x05000a00, 0x05000a1f).r(FUNC(cps3_state::dev_dipsw_r));

	map(0x05001000, 0x05001203).rw(FUNC(cps3_state::eeprom_r), FUNC(cps3_state::eeprom_w));

	map(0x05040000, 0x0504ffff).rw(FUNC(cps3_state::ssram_r), FUNC(cps3_state::ssram_w)).umask32(0x00ff00ff); // 'SS' RAM (Score Screen) (text tilemap + toles)
	map(0x05050000, 0x0505002b).w(FUNC(cps3_state::ssregs_w)).umask32(0x00ff00ff);

	map(0x05100000, 0x05100003).lw32(NAME([this] (offs_t, u32) { m_maincpu->set_input_line(12, CLEAR_LINE); }));
	map(0x05110000, 0x05110003).lw32(NAME([this] (offs_t, u32) { m_maincpu->set_input_line(10, CLEAR_LINE); }));
	map(0x05120000, 0x05120003).lw32(NAME([this] (offs_t, u32) { m_maincpu->set_input_line(14, CLEAR_LINE); })); // ?? unused
	map(0x05130000, 0x05130003).lw32(NAME([this] (offs_t, u32) { m_maincpu->set_input_line(6, CLEAR_LINE); })); // ?? unused
	map(0x05140000, 0x05140003).rw("scsi:7:wd33c93", FUNC(wd33c93_device::indir_r), FUNC(wd33c93_device::indir_w)).umask32(0x00ff00ff);

	map(0x06000000, 0x067fffff).rw(FUNC(cps3_state::flash1_r), FUNC(cps3_state::flash1_w)); /* Flash ROMs simm 1 */
	map(0x06800000, 0x06ffffff).rw(FUNC(cps3_state::flash2_r), FUNC(cps3_state::flash2_w)); /* Flash ROMs simm 2 */

	map(0x07ff0048, 0x07ff004b).nopw(); // bit 0 toggles during programming
	map(0xc0000000, 0xc00003ff).ram().w(FUNC(cps3_state::sh2cache_ram_w)).share("sh2cache_ram"); /* Executes code from here */
}

void cps3_state::decrypted_opcodes_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).rom().region("bios", 0); // BIOS ROM
	map(0x06000000, 0x06ffffff).rom().share("decrypted_gamerom");
	map(0xc0000000, 0xc00003ff).rom().share("sh2cache_ram_decrypted");
}

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
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)

	PORT_MODIFY("EXTRA")
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 Stand") PORT_PLAYER(1)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P2 Stand") PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
INPUT_PORTS_END

void cps3_state::vbl_interrupt(int state)
{
	if (state)
		m_maincpu->set_input_line(12, ASSERT_LINE);
}

// Test cases: character portraits screen after character select in sfiii2, warzard attract title.
TIMER_DEVICE_CALLBACK_MEMBER(cps3_state::dma_interrupt)
{
	m_dma_status &= ~6;
	m_maincpu->set_input_line(10, ASSERT_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(cps3_state::sprite_dma_cb)
{
	m_dma_status &= ~1;
}


void cps3_state::machine_start()
{
	m_eeprom = std::make_unique<u32[]>(0x80/4);
	subdevice<nvram_device>("eeprom")->set_base(m_eeprom.get(), 0x80);

	save_item(NAME(m_cram_gfxflash_bank));
	save_item(NAME(m_cram_bank));
	save_item(NAME(m_current_eeprom_read));
	save_item(NAME(m_paldma_source));
	save_item(NAME(m_paldma_realsource));
	save_item(NAME(m_paldma_dest));
	save_item(NAME(m_paldma_fade));
	save_item(NAME(m_paldma_other2));
	save_item(NAME(m_paldma_length));
	save_item(NAME(m_chardma_source));
	save_item(NAME(m_chardma_other));
	save_item(NAME(m_current_table_address));
	save_item(NAME(m_dma_status));
	save_item(NAME(m_ppu_gscroll_buff));
	save_item(NAME(m_ss_hscroll));
	save_item(NAME(m_ss_vscroll));
	save_item(NAME(m_ss_pal_base));
	save_item(NAME(m_spritelist_dma));

	save_pointer(NAME(m_eeprom), 0x80/4);
}


void cps3_state::machine_reset()
{
	m_current_table_address = -1;
	m_dma_status = 0;
	m_spritelist_dma = 0;

	// copy data from flashroms back into user regions + decrypt into regions we execute/draw from.
	copy_from_nvram();
}


void cps3_state::device_post_load()
{
	// copy data from flashroms back into user regions + decrypt into regions we execute/draw from.
	copy_from_nvram();
	m_gfxdecode->gfx(0)->mark_all_dirty();
	m_gfxdecode->gfx(1)->mark_all_dirty();
}


// make a copy in the regions we execute code / draw gfx from
void cps3_state::copy_from_nvram()
{
	u32* romdata = (u32*)m_user4;
	u32* romdata2 = (u32*)m_decrypted_gamerom;
	/* copy + decrypt program roms which have been loaded from flashroms/nvram */
	for (u32 i = 0; i < 0x800000; i +=4)
	{
		u32 data;

		data = ((m_simm[0][0]->read_raw(i/4)<<24) | (m_simm[0][1]->read_raw(i/4)<<16) | (m_simm[0][2]->read_raw(i/4)<<8) | (m_simm[0][3]->read_raw(i/4)<<0));

		//logerror("%08x %08x %08x %08x\n",romdata[i/4],data, romdata2[i/4], data ^ cps3_mask(i+0x6000000, m_key1, m_key2));
		romdata[i/4] = data;
		romdata2[i/4] = data ^ cps3_mask(i+0x6000000, m_key1, m_key2);

	}

	romdata  += 0x800000/4;
	romdata2 += 0x800000/4;

	if (m_simm[1][0] != nullptr)
		for (u32 i = 0; i < 0x800000; i +=4)
		{
			u32 data;

			data = ((m_simm[1][0]->read_raw(i/4)<<24) | (m_simm[1][1]->read_raw(i/4)<<16) | (m_simm[1][2]->read_raw(i/4)<<8) | (m_simm[1][3]->read_raw(i/4)<<0));

			//logerror("%08x %08x %08x %08x\n",romdata[i/4],data, romdata2[i/4],  data ^ cps3_mask(i+0x6800000, m_key1, m_key2) );
			romdata[i/4] = data;
			romdata2[i/4] = data ^ cps3_mask(i+0x6800000, m_key1, m_key2);
		}

	/* copy gfx from loaded flashroms to user reigon 5, where it's used */
	{
		u32 len = USER5REGION_LENGTH;
		int flashnum = 0;
		int countoffset = 0;

		romdata = (u32*)m_user5;
		for (u32 thebase = 0; thebase < len/2; thebase += 0x200000)
		{
			//logerror("flashnums %d. %d\n",flashnum, flashnum+1);

			fujitsu_29f016a_device *flash0 = m_simm[2 + flashnum/8][flashnum % 8 + 0];
			fujitsu_29f016a_device *flash1 = m_simm[2 + flashnum/8][flashnum % 8 + 1];
			if (flash0 == nullptr || flash1 == nullptr)
				continue;
			if (flash0 != nullptr && flash1 != nullptr)
			{
				for (u32 i = 0; i < 0x200000; i +=2)
				{
					u32 dat = (flash0->read_raw(i+0)<<8) |
									(flash0->read_raw(i+1)<<24) |
									(flash1->read_raw(i+0)<<0) |
									(flash1->read_raw(i+1)<<16);

					//logerror("%08x %08x\n",romdata[countoffset],dat);
					romdata[countoffset] = dat;

					countoffset++;
				}
			}
			flashnum+=2;
		}
	}
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


	if (src < 0x80000)
	{
		data = data ^ cps3_mask(src & ~3, m_key1, m_key2);
	}
	else if (src >= 0x6000000 && src < 0x7000000)
	{
		if (!m_altEncryption) data = data ^ cps3_mask(src & ~3, m_key1, m_key2);
	}
	else
	{
		//logerror("%s :src %08x, dst %08x, returning %08x\n", machine().describe_context().c_str(), src, dst, data);
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

void cps3_state::simm_config(machine_config &config, unsigned slot, unsigned count)
{
	for (unsigned i = 0; i < count; i++)
		FUJITSU_29F016A(config, m_simm[slot][i]);
}

void cps3_state::simm1_64mbit(machine_config &config)
{
	simm_config(config, 0, 4);
}

void cps3_state::simm2_64mbit(machine_config &config)
{
	simm_config(config, 1, 4);
}

void cps3_state::simm3_128mbit(machine_config &config)
{
	simm_config(config, 2, 8);
}

void cps3_state::simm4_128mbit(machine_config &config)
{
	simm_config(config, 3, 8);
}

void cps3_state::simm5_128mbit(machine_config &config)
{
	simm_config(config, 4, 8);
}

void cps3_state::simm5_32mbit(machine_config &config)
{
	simm_config(config, 4, 2);
}

void cps3_state::simm6_128mbit(machine_config &config)
{
	simm_config(config, 5, 8);
}

void cps3_state::cps3(machine_config &config)
{
	/* basic machine hardware */
	SH7604(config, m_maincpu, 6250000*4); // external clock is 6.25 Mhz, it sets the internal multiplier to 4x (this should probably be handled in the core..)
	m_maincpu->set_addrmap(AS_PROGRAM, &cps3_state::cps3_map);
	m_maincpu->set_addrmap(AS_OPCODES, &cps3_state::decrypted_opcodes_map);
	m_maincpu->set_dma_kludge_callback(FUNC(cps3_state::dma_callback));

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:1").option_set("cdrom", NSCSI_CDROM);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("wd33c93", WD33C93A).clock(10'000'000);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(42'954'545)/5, (454+1)*6/5, 0, 384, 262+2, 0, 224); // H Total counter uses XTAL/6 clock
	screen.set_screen_update(FUNC(cps3_state::screen_update));
	screen.screen_vblank().set(FUNC(cps3_state::vbl_interrupt));
/*
    Measured clocks:
        Video DAC = 8.602MHz (384 wide mode) ~ 42.9545MHz / 5
                    10.73MHZ (496 wide mode) ~ 42.9545MHz / 4
        H = 15.73315kHz
        V = 59.59Hz
        H/V ~ 264 lines
*/

	TIMER(config, m_dma_timer).configure_generic(FUNC(cps3_state::dma_interrupt));
	TIMER(config, m_spritelist_dma_timer).configure_generic(FUNC(cps3_state::sprite_dma_cb));

	NVRAM(config, "eeprom", nvram_device::DEFAULT_ALL_0);
	PALETTE(config, m_palette).set_entries(0x10000); // actually 0x20000 ...

	GFXDECODE(config, m_gfxdecode, m_palette, gfxdecode_device::empty);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	CPS3(config, m_cps3sound, XTAL(42'954'545) / 3);
	m_cps3sound->add_route(1, "speaker", 1.0, 0);
	m_cps3sound->add_route(0, "speaker", 1.0, 1);
}


/* individual configs for each machine, depending on the SIMMs installed */
void cps3_state::redearth(machine_config &config)
{
	cps3(config);
	simm1_64mbit(config);
	simm3_128mbit(config);
	simm4_128mbit(config);
	simm5_32mbit(config);
}

void cps3_state::sfiii(machine_config &config)
{
	cps3(config);
	simm1_64mbit(config);
	simm3_128mbit(config);
	simm4_128mbit(config);
	simm5_32mbit(config);
}

void cps3_state::sfiii2(machine_config &config)
{
	cps3(config);
	simm1_64mbit(config);
	simm2_64mbit(config);
	simm3_128mbit(config);
	simm4_128mbit(config);
	simm5_128mbit(config);
}

void cps3_state::jojo(machine_config &config)
{
	cps3(config);
	simm1_64mbit(config);
	simm2_64mbit(config);
	simm3_128mbit(config);
	simm4_128mbit(config);
	simm5_32mbit(config);
}

void cps3_state::sfiii3(machine_config &config)
{
	cps3(config);
	simm1_64mbit(config);
	simm2_64mbit(config);
	simm3_128mbit(config);
	simm4_128mbit(config);
	simm5_128mbit(config);
	simm6_128mbit(config);
}

void cps3_state::jojoba(machine_config &config)
{
	cps3(config);
	simm1_64mbit(config);
	simm2_64mbit(config);
	simm3_128mbit(config);
	simm4_128mbit(config);
	simm5_128mbit(config);
}


/* CD sets - use CD BIOS roms */

ROM_START( redearth )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "redearth_euro.29f400.u2", 0x000000, 0x080000, CRC(02e0f336) SHA1(acc37e830dfeb9674f5a0fb24f4cc23217ae4ff5) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-wzd-5", 0, SHA1(034c375c3f2f68723eef530b40da909a7be7b556) )
ROM_END

ROM_START( redearthr1 )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "redearth_euro.29f400.u2", 0x000000, 0x080000, CRC(02e0f336) SHA1(acc37e830dfeb9674f5a0fb24f4cc23217ae4ff5) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-wzd-3", 0, SHA1(a6ff67093db6bc80ee5fc46e4300e0177b213a52) )
ROM_END

ROM_START( warzard )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "warzard_japan.29f400.u2", 0x000000, 0x080000, CRC(f8e2f0c6) SHA1(93d6a986f44c211fff014e55681eca4d2a2774d6) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-wzd-5", 0, SHA1(034c375c3f2f68723eef530b40da909a7be7b556) )
ROM_END

ROM_START( warzardr1 )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "warzard_japan.29f400.u2", 0x000000, 0x080000, CRC(f8e2f0c6) SHA1(93d6a986f44c211fff014e55681eca4d2a2774d6) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-wzd-3", 0, SHA1(a6ff67093db6bc80ee5fc46e4300e0177b213a52) )
ROM_END


ROM_START( sfiii )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "sfiii_euro.29f400.u2", 0x000000, 0x080000, CRC(27699ddc) SHA1(d8b525cd27e584560b129598df31fd2c5b2a682a) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-sf3-3", 0, SHA1(20aa46f8ffeb235205dc95cfd8fba42c7d102355) )
ROM_END

ROM_START( sfiiiu )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "sfiii_usa_region_b1.29f400.u2", 0x000000, 0x080000, CRC(fb172a8e) SHA1(48ebf59910f246835f7dc0c588da30f7a908072f) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-sf3-3", 0, SHA1(20aa46f8ffeb235205dc95cfd8fba42c7d102355) )
ROM_END

ROM_START( sfiiia )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "sfiii_asia_region_bd.29f400.u2", 0x000000, 0x080000,  CRC(cbd28de7) SHA1(9c15ecb73b9587d20850e62e8683930a45caa01b) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-sf3-3", 0, SHA1(20aa46f8ffeb235205dc95cfd8fba42c7d102355) )
ROM_END

ROM_START( sfiiij )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "sfiii_japan.29f400.u2", 0x000000, 0x080000, CRC(74205250) SHA1(c3e83ace7121d32da729162662ec6b5285a31211) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-sf3-3", 0, SHA1(20aa46f8ffeb235205dc95cfd8fba42c7d102355) )
ROM_END

ROM_START( sfiiih )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "sfiii_hispanic.29f400.u2", 0x000000, 0x080000, CRC(d2b3cd48) SHA1(00ebb270c24a66515c97e35331de54ff5358000e) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-sf3-3", 0, SHA1(20aa46f8ffeb235205dc95cfd8fba42c7d102355) )
ROM_END


ROM_START( sfiii2 )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "sfiii2_usa.29f400.u2", 0x000000, 0x080000, CRC(75dd72e0) SHA1(5a12d6ea6734df5de00ecee6f9ef470749d2f242) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-3ga000", 0, SHA1(a0c11a5c3057dc1ad3962aa38adf95acb3430bec) )
ROM_END

ROM_START( sfiii2j )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "sfiii2_japan.29f400.u2", 0x000000, 0x080000, CRC(faea0a3e) SHA1(a03cd63bcf52e4d57f7a598c8bc8e243694624ec) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-3ga000", 0, SHA1(a0c11a5c3057dc1ad3962aa38adf95acb3430bec) )
ROM_END

ROM_START( sfiii2h )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "sfiii2_hispanic.29f400.u2", 0x000000, 0x080000, CRC(5c799526) SHA1(5a56345b500cd7e20d5fb6cabc791655c6ff4ed2) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-3ga000", 0, SHA1(a0c11a5c3057dc1ad3962aa38adf95acb3430bec) )
ROM_END

ROM_START( jojo )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "jojo_euro.29f400.u2", 0x000000, 0x080000, CRC(513e40ec) SHA1(03b91f0fbd5be56d24feed4698c7543d4df07837) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-jjk-3", 0, SHA1(dc6e74b5e02e13f62cb8c4e234dd6061501e49c1) )
ROM_END

ROM_START( jojor1 )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "jojo_euro.29f400.u2", 0x000000, 0x080000, CRC(513e40ec) SHA1(03b91f0fbd5be56d24feed4698c7543d4df07837) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-jjk-2", 0, SHA1(ce22d10f2b35a0e00f8d83642b97842c9b2327fa) )
ROM_END

ROM_START( jojor2 )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "jojo_euro.29f400.u2", 0x000000, 0x080000, CRC(513e40ec) SHA1(03b91f0fbd5be56d24feed4698c7543d4df07837) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-jjk000", 0, SHA1(a24e174aaaf47f98312a38129645026a613cd344) )
ROM_END

ROM_START( jojou )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "jojo_usa.29f400.u2", 0x000000, 0x080000, CRC(8d40f7be) SHA1(2a4bd83db2f959c33b071e517941aa55a0f919c0) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-jjk-3", 0, SHA1(dc6e74b5e02e13f62cb8c4e234dd6061501e49c1) )
ROM_END

ROM_START( jojour1 )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "jojo_usa.29f400.u2", 0x000000, 0x080000, CRC(8d40f7be) SHA1(2a4bd83db2f959c33b071e517941aa55a0f919c0) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-jjk-2", 0, SHA1(ce22d10f2b35a0e00f8d83642b97842c9b2327fa) )
ROM_END

ROM_START( jojour2 )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "jojo_usa.29f400.u2", 0x000000, 0x080000, CRC(8d40f7be) SHA1(2a4bd83db2f959c33b071e517941aa55a0f919c0) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-jjk000", 0, SHA1(a24e174aaaf47f98312a38129645026a613cd344) )
ROM_END

ROM_START( jojoj )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "jojo_japan.29f400.u2", 0x000000, 0x080000, CRC(02778f60) SHA1(a167f9ebe030592a0cdb0c6a3c75835c6a43be4c) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-jjk-3", 0, SHA1(dc6e74b5e02e13f62cb8c4e234dd6061501e49c1) )
ROM_END

ROM_START( jojojr1 )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "jojo_japan.29f400.u2", 0x000000, 0x080000, CRC(02778f60) SHA1(a167f9ebe030592a0cdb0c6a3c75835c6a43be4c) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-jjk-2", 0, SHA1(ce22d10f2b35a0e00f8d83642b97842c9b2327fa) )
ROM_END

ROM_START( jojojr2 )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "jojo_japan.29f400.u2", 0x000000, 0x080000, CRC(02778f60) SHA1(a167f9ebe030592a0cdb0c6a3c75835c6a43be4c) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-jjk000", 0, SHA1(a24e174aaaf47f98312a38129645026a613cd344) )
ROM_END

ROM_START( jojoa )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "jojo_asia.29f400.u2", 0x000000, 0x080000, CRC(789aa72a) SHA1(afcefb963d7c103514585f4a6738b2deb5b7d27a) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-jjk-3", 0, SHA1(dc6e74b5e02e13f62cb8c4e234dd6061501e49c1) )
ROM_END

ROM_START( jojoar1 )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "jojo_asia.29f400.u2", 0x000000, 0x080000, CRC(789aa72a) SHA1(afcefb963d7c103514585f4a6738b2deb5b7d27a) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-jjk-2", 0, SHA1(ce22d10f2b35a0e00f8d83642b97842c9b2327fa) )
ROM_END

ROM_START( jojoar2 )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "jojo_asia.29f400.u2", 0x000000, 0x080000, CRC(789aa72a) SHA1(afcefb963d7c103514585f4a6738b2deb5b7d27a) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-jjk000", 0, SHA1(a24e174aaaf47f98312a38129645026a613cd344) )
ROM_END


ROM_START( sfiii3 )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "sfiii3_euro.29f400.u2", 0x000000, 0x080000, CRC(30bbf293) SHA1(f094c2eeaf4f6709060197aca371a4532346bf78) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-33s-2", 0, SHA1(5a090956fc6d68e496ac42854199059898f2fe16) )
ROM_END

ROM_START( sfiii3r1 )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "sfiii3_euro.29f400.u2", 0x000000, 0x080000, CRC(30bbf293) SHA1(f094c2eeaf4f6709060197aca371a4532346bf78) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-33s-1", 0, SHA1(92e146fea751404077919da91f4c5112742627ed) )
ROM_END

ROM_START( sfiii3u )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "sfiii3_usa.29f400.u2", 0x000000, 0x080000, CRC(ecc545c1) SHA1(e39083820aae914fd8b80c9765129bedb745ceba) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-33s-2", 0, SHA1(5a090956fc6d68e496ac42854199059898f2fe16) )
ROM_END

ROM_START( sfiii3ur1 )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "sfiii3_usa.29f400.u2", 0x000000, 0x080000, CRC(ecc545c1) SHA1(e39083820aae914fd8b80c9765129bedb745ceba) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-33s-1", 0, SHA1(92e146fea751404077919da91f4c5112742627ed) )
ROM_END

ROM_START( sfiii3j )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "sfiii3_japan.29f400.u2", 0x000000, 0x080000, CRC(63f23d1f) SHA1(58559403c325454f8c8d3eb0f569a531aa22db26) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-33s-2", 0, SHA1(5a090956fc6d68e496ac42854199059898f2fe16) )
ROM_END

ROM_START( sfiii3jr1 )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "sfiii3_japan.29f400.u2", 0x000000, 0x080000, CRC(63f23d1f) SHA1(58559403c325454f8c8d3eb0f569a531aa22db26) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-33s-1", 0, SHA1(92e146fea751404077919da91f4c5112742627ed) )
ROM_END


ROM_START( jojobar1 )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "jojoba_euro.29f400.u2", 0x000000, 0x080000, CRC(63cc8800) SHA1(f0c7e6abb205a16dab7a114e017b193521071a4b) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-jjm-1", 0, SHA1(8628d3fa555fbd5f4121082e925c1834b76c5e65) )
ROM_END

ROM_START( jojobajr1 )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "jojoba_japan.29f400.u2", 0x000000, 0x080000, CRC(3085478c) SHA1(055eab1fc42816f370a44b17fd7e87ffcb10e8b7) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-jjm-1", 0, SHA1(8628d3fa555fbd5f4121082e925c1834b76c5e65) )
ROM_END

ROM_START( jojobar2 )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "jojoba_euro.29f400.u2", 0x000000, 0x080000, CRC(63cc8800) SHA1(f0c7e6abb205a16dab7a114e017b193521071a4b) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-jjm-0", 0, SHA1(1651896d127dbf32af99175ae91227cd90675aaa) )
ROM_END

ROM_START( jojobajr2 )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "jojoba_japan.29f400.u2", 0x000000, 0x080000, CRC(3085478c) SHA1(055eab1fc42816f370a44b17fd7e87ffcb10e8b7) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "cap-jjm-0", 0, SHA1(1651896d127dbf32af99175ae91227cd90675aaa) )
ROM_END





/* NO CD sets - use NO CD BIOS roms - don't require the CD image to boot */

ROM_START( redearthn )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "redearth_asia_nocd.29f400.u2", 0x000000, 0x080000, CRC(7a4f0851) SHA1(660ca716960ec761038e5ad4de636be13b0dddd8) ) // this is a different VERSION of the bios compared to other sets, not just an alt region code

	ROM_REGION( 0x200000, "simm1.0", 0 )
	ROM_LOAD( "redearth-simm1.0", 0x00000, 0x200000, CRC(cad468f8) SHA1(b3aa4f7d3fae84e8821417ccde9528d3eda2b7a6) )
	ROM_REGION( 0x200000, "simm1.1", 0 )
	ROM_LOAD( "redearth-simm1.1", 0x00000, 0x200000, CRC(e9721d89) SHA1(5c63d10bdbce52d50b6dde14d4a0f1369383d656) )
	ROM_REGION( 0x200000, "simm1.2", 0 )
	ROM_LOAD( "redearth-simm1.2", 0x00000, 0x200000, CRC(2889ec98) SHA1(a94310eb4777f908d87e9d90969db8504b4140ff) )
	ROM_REGION( 0x200000, "simm1.3", 0 )
	ROM_LOAD( "redearth-simm1.3", 0x00000, 0x200000, CRC(5a6cd148) SHA1(d65c6e8378a91828474a16a3bbcd13c4b3b15f13) )

	ROM_REGION( 0x200000, "simm3.0", 0 )
	ROM_LOAD( "redearth-simm3.0", 0x00000, 0x200000, CRC(83350cc5) SHA1(922b1abf80a4a89f35279b66311a7369d3965bd0) )
	ROM_REGION( 0x200000, "simm3.1", 0 )
	ROM_LOAD( "redearth-simm3.1", 0x00000, 0x200000, CRC(56734de6) SHA1(75699fa6efe5bec335e4b02e15b3c45726b68fa8) )
	ROM_REGION( 0x200000, "simm3.2", 0 )
	ROM_LOAD( "redearth-simm3.2", 0x00000, 0x200000, CRC(800ea0f1) SHA1(33871ab56dc1cd24441389d53e43fb8e43b149d9) )
	ROM_REGION( 0x200000, "simm3.3", 0 )
	ROM_LOAD( "redearth-simm3.3", 0x00000, 0x200000, CRC(97e9146c) SHA1(ab7744709615081440bee72f4080d6fd5b938668) )
	ROM_REGION( 0x200000, "simm3.4", 0 )
	ROM_LOAD( "redearth-simm3.4", 0x00000, 0x200000, CRC(0cb1d648) SHA1(7042a590c2b7ec55323062127e254da3cdc790a1) )
	ROM_REGION( 0x200000, "simm3.5", 0 )
	ROM_LOAD( "redearth-simm3.5", 0x00000, 0x200000, CRC(7a1099f0) SHA1(c6a92ec86eb24485f1db530e0e78f647e8432231) )
	ROM_REGION( 0x200000, "simm3.6", 0 )
	ROM_LOAD( "redearth-simm3.6", 0x00000, 0x200000, CRC(aeff8f54) SHA1(fd760e237c2e5fb2da45e32a1c12fd3defb4c3e4) )
	ROM_REGION( 0x200000, "simm3.7", 0 )
	ROM_LOAD( "redearth-simm3.7", 0x00000, 0x200000, CRC(f770acd0) SHA1(4b3ccb6f91568f95f04ede6c574144918d131201) )

	ROM_REGION( 0x200000, "simm4.0", 0 )
	ROM_LOAD( "redearth-simm4.0", 0x00000, 0x200000, CRC(301e56f2) SHA1(4847d971bff70a2aeed4599e1201c7ec9677da60) )
	ROM_REGION( 0x200000, "simm4.1", 0 )
	ROM_LOAD( "redearth-simm4.1", 0x00000, 0x200000, CRC(2048e103) SHA1(b21f95b05cd99749bd3f25cc71b2671c2026847b) )
	ROM_REGION( 0x200000, "simm4.2", 0 )
	ROM_LOAD( "redearth-simm4.2", 0x00000, 0x200000, CRC(c9433455) SHA1(63a269d76bac332c2e991d0f6a20c35e0e88680a) )
	ROM_REGION( 0x200000, "simm4.3", 0 )
	ROM_LOAD( "redearth-simm4.3", 0x00000, 0x200000, CRC(c02171a8) SHA1(2e9228729b27a6113d9f2e42af310a834979f714) )
	ROM_REGION( 0x200000, "simm4.4", 0 )
	ROM_LOAD( "redearth-simm4.4", 0x00000, 0x200000, CRC(2ddbf276) SHA1(b232baaa8edc8db18f8a3bdcc2d38fe984a94a34) )
	ROM_REGION( 0x200000, "simm4.5", 0 )
	ROM_LOAD( "redearth-simm4.5", 0x00000, 0x200000, CRC(fea820a6) SHA1(55ee8ef95751f5a509fb126513e1b2a70a3414e5) )
	ROM_REGION( 0x200000, "simm4.6", 0 )
	ROM_LOAD( "redearth-simm4.6", 0x00000, 0x200000, CRC(c7528df1) SHA1(aa312f80c2d7759d18d1aa8d416cf932b2850824) )
	ROM_REGION( 0x200000, "simm4.7", 0 )
	ROM_LOAD( "redearth-simm4.7", 0x00000, 0x200000, CRC(2449cf3b) SHA1(c60d8042136d74e547f668ad787cae529c42eed9) )

	ROM_REGION( 0x200000, "simm5.0", 0 )
	ROM_LOAD( "redearth-simm5.0", 0x00000, 0x200000, CRC(424451b9) SHA1(250fb92254c9e7ff5bc8dbeea5872f8a771dc9bd) )
	ROM_REGION( 0x200000, "simm5.1", 0 )
	ROM_LOAD( "redearth-simm5.1", 0x00000, 0x200000, CRC(9b8cb56b) SHA1(2ff1081dc99bb7c2f1e036f4c112137c96b83d23) )
ROM_END

ROM_START( redearthnr1 )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "redearth_asia_nocd.29f400.u2", 0x000000, 0x080000, CRC(7a4f0851) SHA1(660ca716960ec761038e5ad4de636be13b0dddd8) ) // this is a different VERSION of the bios compared to other sets, not just an alt region code

	ROM_REGION( 0x200000, "simm1.0", 0 )
	ROM_LOAD( "redearthr1-simm1.0", 0x00000, 0x200000, CRC(65bac346) SHA1(6f4ba0c2cae91a37fc97bea5fc8a50aaf6ca6513) )
	ROM_REGION( 0x200000, "simm1.1", 0 )
	ROM_LOAD( "redearthr1-simm1.1", 0x00000, 0x200000, CRC(a8ec4aae) SHA1(0012cb6ba630ddd74958f7759de34706bf919338) )
	ROM_REGION( 0x200000, "simm1.2", 0 )
	ROM_LOAD( "redearthr1-simm1.2", 0x00000, 0x200000, CRC(2caf8995) SHA1(ca012b6dec0481b043edf9c7e931bd952ec74ebb) )
	ROM_REGION( 0x200000, "simm1.3", 0 )
	ROM_LOAD( "redearthr1-simm1.3", 0x00000, 0x200000, CRC(13ebc21d) SHA1(465bdea0633526a8bf07b35495a5311c8bf213d5) )

	ROM_REGION( 0x200000, "simm3.0", 0 )
	ROM_LOAD( "redearth-simm3.0", 0x00000, 0x200000, CRC(83350cc5) SHA1(922b1abf80a4a89f35279b66311a7369d3965bd0) )
	ROM_REGION( 0x200000, "simm3.1", 0 )
	ROM_LOAD( "redearth-simm3.1", 0x00000, 0x200000, CRC(56734de6) SHA1(75699fa6efe5bec335e4b02e15b3c45726b68fa8) )
	ROM_REGION( 0x200000, "simm3.2", 0 )
	ROM_LOAD( "redearth-simm3.2", 0x00000, 0x200000, CRC(800ea0f1) SHA1(33871ab56dc1cd24441389d53e43fb8e43b149d9) )
	ROM_REGION( 0x200000, "simm3.3", 0 )
	ROM_LOAD( "redearth-simm3.3", 0x00000, 0x200000, CRC(97e9146c) SHA1(ab7744709615081440bee72f4080d6fd5b938668) )
	ROM_REGION( 0x200000, "simm3.4", 0 )
	ROM_LOAD( "redearth-simm3.4", 0x00000, 0x200000, CRC(0cb1d648) SHA1(7042a590c2b7ec55323062127e254da3cdc790a1) )
	ROM_REGION( 0x200000, "simm3.5", 0 )
	ROM_LOAD( "redearth-simm3.5", 0x00000, 0x200000, CRC(7a1099f0) SHA1(c6a92ec86eb24485f1db530e0e78f647e8432231) )
	ROM_REGION( 0x200000, "simm3.6", 0 )
	ROM_LOAD( "redearth-simm3.6", 0x00000, 0x200000, CRC(aeff8f54) SHA1(fd760e237c2e5fb2da45e32a1c12fd3defb4c3e4) )
	ROM_REGION( 0x200000, "simm3.7", 0 )
	ROM_LOAD( "redearth-simm3.7", 0x00000, 0x200000, CRC(f770acd0) SHA1(4b3ccb6f91568f95f04ede6c574144918d131201) )

	ROM_REGION( 0x200000, "simm4.0", 0 )
	ROM_LOAD( "redearth-simm4.0", 0x00000, 0x200000, CRC(301e56f2) SHA1(4847d971bff70a2aeed4599e1201c7ec9677da60) )
	ROM_REGION( 0x200000, "simm4.1", 0 )
	ROM_LOAD( "redearth-simm4.1", 0x00000, 0x200000, CRC(2048e103) SHA1(b21f95b05cd99749bd3f25cc71b2671c2026847b) )
	ROM_REGION( 0x200000, "simm4.2", 0 )
	ROM_LOAD( "redearth-simm4.2", 0x00000, 0x200000, CRC(c9433455) SHA1(63a269d76bac332c2e991d0f6a20c35e0e88680a) )
	ROM_REGION( 0x200000, "simm4.3", 0 )
	ROM_LOAD( "redearth-simm4.3", 0x00000, 0x200000, CRC(c02171a8) SHA1(2e9228729b27a6113d9f2e42af310a834979f714) )
	ROM_REGION( 0x200000, "simm4.4", 0 )
	ROM_LOAD( "redearth-simm4.4", 0x00000, 0x200000, CRC(2ddbf276) SHA1(b232baaa8edc8db18f8a3bdcc2d38fe984a94a34) )
	ROM_REGION( 0x200000, "simm4.5", 0 )
	ROM_LOAD( "redearth-simm4.5", 0x00000, 0x200000, CRC(fea820a6) SHA1(55ee8ef95751f5a509fb126513e1b2a70a3414e5) )
	ROM_REGION( 0x200000, "simm4.6", 0 )
	ROM_LOAD( "redearth-simm4.6", 0x00000, 0x200000, CRC(c7528df1) SHA1(aa312f80c2d7759d18d1aa8d416cf932b2850824) )
	ROM_REGION( 0x200000, "simm4.7", 0 )
	ROM_LOAD( "redearth-simm4.7", 0x00000, 0x200000, CRC(2449cf3b) SHA1(c60d8042136d74e547f668ad787cae529c42eed9) )

	ROM_REGION( 0x200000, "simm5.0", 0 )
	ROM_LOAD( "redearth-simm5.0", 0x00000, 0x200000, CRC(424451b9) SHA1(250fb92254c9e7ff5bc8dbeea5872f8a771dc9bd) )
	ROM_REGION( 0x200000, "simm5.1", 0 )
	ROM_LOAD( "redearth-simm5.1", 0x00000, 0x200000, CRC(9b8cb56b) SHA1(2ff1081dc99bb7c2f1e036f4c112137c96b83d23) )
ROM_END


ROM_START( sfiiin )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
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
	ROM_REGION32_BE( 0x080000, "bios", 0 )
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
	ROM_REGION32_BE( 0x080000, "bios", 0 )
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
	ROM_REGION32_BE( 0x080000, "bios", 0 )
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
	ROM_REGION32_BE( 0x080000, "bios", 0 )
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
	ROM_REGION32_BE( 0x080000, "bios", 0 )
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
	ROM_REGION32_BE( 0x080000, "bios", 0 )
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

ROM_START( sfiii3na )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "sfiii3_asia_nocd.29f400.u2", 0x000000, 0x080000, CRC(a12ebcd1) SHA1(f0925a3099e3279da42d2d292dfa69e834219197) )

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
	ROM_REGION32_BE( 0x080000, "bios", 0 )
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

ROM_START( sfiii3nar1 )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "sfiii3_asia_nocd.29f400.u2", 0x000000, 0x080000, CRC(a12ebcd1) SHA1(f0925a3099e3279da42d2d292dfa69e834219197) )

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

ROM_START( jojoba )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "jojoba_euro_nocd.29f400.u2", 0x000000, 0x080000, CRC(1ee2d679) SHA1(9e129b454a376606b3f7e8aec64de425cf9c635c) )

	ROM_REGION( 0x200000, "simm1.0", 0 )
	ROM_LOAD( "jojoba-simm1.0", 0x00000, 0x200000, CRC(b3cc516d) SHA1(cac67a5bf9d01af226be377e2b1b8ed4ea7fd164) ) // sldh
	ROM_REGION( 0x200000, "simm1.1", 0 )
	ROM_LOAD( "jojoba-simm1.1", 0x00000, 0x200000, CRC(dab4bdc7) SHA1(47283071de152a0b2b96d3f3fc22537677c35d3f) ) // sldh
	ROM_REGION( 0x200000, "simm1.2", 0 )
	ROM_LOAD( "jojoba-simm1.2", 0x00000, 0x200000, CRC(a6a4bf48) SHA1(1684663252894538de4855eee336ffc031142ba9) ) // sldh
	ROM_REGION( 0x200000, "simm1.3", 0 )
	ROM_LOAD( "jojoba-simm1.3", 0x00000, 0x200000, CRC(731229ee) SHA1(2171ffa115752b0dcc1695f4a5d0557237a73db8) ) // sldh

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

ROM_START( jojoban )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "jojoba_japan_nocd.29f400.u2", 0x000000, 0x080000, CRC(4dab19f5) SHA1(ba07190e7662937fc267f07285c51e99a45c061e) )

	ROM_REGION( 0x200000, "simm1.0", 0 )
	ROM_LOAD( "jojoba-simm1.0", 0x00000, 0x200000, CRC(b3cc516d) SHA1(cac67a5bf9d01af226be377e2b1b8ed4ea7fd164) ) // sldh
	ROM_REGION( 0x200000, "simm1.1", 0 )
	ROM_LOAD( "jojoba-simm1.1", 0x00000, 0x200000, CRC(dab4bdc7) SHA1(47283071de152a0b2b96d3f3fc22537677c35d3f) ) // sldh
	ROM_REGION( 0x200000, "simm1.2", 0 )
	ROM_LOAD( "jojoba-simm1.2", 0x00000, 0x200000, CRC(a6a4bf48) SHA1(1684663252894538de4855eee336ffc031142ba9) ) // sldh
	ROM_REGION( 0x200000, "simm1.3", 0 )
	ROM_LOAD( "jojoba-simm1.3", 0x00000, 0x200000, CRC(731229ee) SHA1(2171ffa115752b0dcc1695f4a5d0557237a73db8) ) // sldh

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
	ROM_REGION32_BE( 0x080000, "bios", 0 )
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

ROM_START( jojobanr2 )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
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

ROM_START( jojobaner1 )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
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

ROM_START( jojobaner2 )
	ROM_REGION32_BE( 0x080000, "bios", 0 )
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
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "no-battery_bios_29f400_for_hd6417095_sh2.u2", 0x000000, 0x080000, CRC(cb9bd5b0) SHA1(ea7ecb3deb69f5307a62d8f0d7d8e68d49013d07))

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "unicd-cps3_for_standard_sh2_v4", 0, SHA1(099c52bd38753f0f4876243e7aa87ca482a2dcb7) )
ROM_END

ROM_START( cps3booto ) // for cart with standard SH2
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "no-battery_bios_29f400_for_hd6417095_sh2.u2", 0x000000, 0x080000, CRC(cb9bd5b0) SHA1(ea7ecb3deb69f5307a62d8f0d7d8e68d49013d07))

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "no-battery_multi-game_bootleg_cd_for_hd6417095_sh2", 0, SHA1(6057cc3ec7991c0c00a7ab9da6ac2f92c9fb1aed) )
ROM_END

ROM_START( cps3booto2 ) // for cart with standard SH2
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "no-battery_bios_29f400_for_hd6417095_sh2.u2", 0x000000, 0x080000, CRC(cb9bd5b0) SHA1(ea7ecb3deb69f5307a62d8f0d7d8e68d49013d07))

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "no-battery_multi-game_bootleg_cd_for_hd6417095_sh2_older", 0, SHA1(123f2fcb0f3dd3d6b859e82a51d0127e46763776) )
ROM_END

ROM_START( cps3bs32 ) // for cart with standard SH2
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "no-battery_bios_29f400_for_hd6417095_sh2.u2", 0x000000, 0x080000, CRC(cb9bd5b0) SHA1(ea7ecb3deb69f5307a62d8f0d7d8e68d49013d07))

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "sfiii_2nd_impact_converted_for_standard_sh2_v3", 0, SHA1(8f180d159e88042a1e819cefd39eef67f5e86e3d) )
ROM_END

ROM_START( cps3bs32a ) // for cart with standard SH2
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "no-battery_bios_29f400_for_hd6417095_sh2.u2", 0x000000, 0x080000, CRC(cb9bd5b0) SHA1(ea7ecb3deb69f5307a62d8f0d7d8e68d49013d07))

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "sfiii_2nd_impact_converted_for_standard_sh2_older", 0, SHA1(8a8e4138c3bf12435933ab9d9ace510513200843) ) // v1 or v2?
ROM_END

ROM_START( cps3boota ) // for cart with dead custom SH2 (or 2nd Impact CPU which is the same as a dead one)
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "no-battery_bios_29f400_for_dead_security_cart.u2", 0x000000, 0x080000, CRC(0fd56fb3) SHA1(5a8bffc07eb7da73cf4bca6718df72e471296bfd) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "unicd-cps3_for_custom_sh2_v5", 0, SHA1(50a5b2845d3dd3de3bce15c4f1b58500db80cabe) )
ROM_END

ROM_START( cps3bootao ) // for cart with dead custom SH2 (or 2nd Impact CPU which is the same as a dead one)
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "no-battery_bios_29f400_for_dead_security_cart.u2", 0x000000, 0x080000, CRC(0fd56fb3) SHA1(5a8bffc07eb7da73cf4bca6718df72e471296bfd) )

	DISK_REGION( "scsi:1:cdrom" )
	DISK_IMAGE_READONLY( "no-battery_multi-game_bootleg_cd_for_dead_security_cart", 0, SHA1(1ede2f1ba197ee787208358a13eae7185a5ae3b2) )
ROM_END


ROM_START( cps3bootao2 ) // for cart with dead custom SH2 (or 2nd Impact CPU which is the same as a dead one)
	ROM_REGION32_BE( 0x080000, "bios", 0 )
	ROM_LOAD( "no-battery_bios_29f400_for_dead_security_cart.u2", 0x000000, 0x080000, CRC(0fd56fb3) SHA1(5a8bffc07eb7da73cf4bca6718df72e471296bfd) )

	DISK_REGION( "scsi:1:cdrom" )
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

    u32 *rom =  (u32*)machine.root_device().memregion ( "bios" )->base();
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

    u32 *rom =  (u32*)machine.root_device().memregion ( "bios" )->base();
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

    u32 *rom =  (u32*)machine.root_device().memregion ( "bios" )->base();
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

    u32 *rom =  (u32*)machine.root_device().memregion ( "bios" )->base();
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

    u32 *rom =  (u32*)machine.root_device().memregion ( "bios" )->base();
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

    u32 *rom =  (u32*)machine.root_device().memregion ( "bios" )->base();
    rom[0x1fec8/4]^=0x00000001; // region (clear jpn)
    rom[0x1fec8/4]^=0x00000002; // region
    rom[0x1fec8/4]^=0x00000070; // DEV mode
    rom[0x1fecc/4]^=0x01000000; // nocd

*****************************************************************************************/

/* Red Earth / Warzard */

// 961121
GAME( 1996, redearth,    0,        redearth, cps3_re,   cps3_state, init_redearth, ROT0, "Capcom", "Red Earth (Europe 961121)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, warzard,     redearth, redearth, cps3_re,   cps3_state, init_redearth, ROT0, "Capcom", "Warzard (Japan 961121)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, redearthn,   redearth, redearth, cps3_re,   cps3_state, init_redearth, ROT0, "Capcom", "Red Earth (Asia 961121, NO CD)", MACHINE_SUPPORTS_SAVE )

// 961023
GAME( 1996, redearthr1,  redearth, redearth, cps3_re,   cps3_state, init_redearth, ROT0, "Capcom", "Red Earth (Europe 961023)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, warzardr1,   redearth, redearth, cps3_re,   cps3_state, init_redearth, ROT0, "Capcom", "Warzard (Japan 961023)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, redearthnr1, redearth, redearth, cps3_re,   cps3_state, init_redearth, ROT0, "Capcom", "Red Earth (Asia 961023, NO CD)", MACHINE_SUPPORTS_SAVE )

/* Street Fighter III: New Generation */

// 970403
// not dumped

// 970312
// not dumped

// 970204
GAME( 1997, sfiii,       0,        sfiii,    cps3,      cps3_state, init_sfiii,    ROT0, "Capcom", "Street Fighter III: New Generation (Europe 970204)", MACHINE_SUPPORTS_SAVE )
GAME( 1997, sfiiiu,      sfiii,    sfiii,    cps3,      cps3_state, init_sfiii,    ROT0, "Capcom", "Street Fighter III: New Generation (USA 970204)", MACHINE_SUPPORTS_SAVE )
GAME( 1997, sfiiia,      sfiii,    sfiii,    cps3,      cps3_state, init_sfiii,    ROT0, "Capcom", "Street Fighter III: New Generation (Asia 970204)", MACHINE_SUPPORTS_SAVE )
GAME( 1997, sfiiij,      sfiii,    sfiii,    cps3,      cps3_state, init_sfiii,    ROT0, "Capcom", "Street Fighter III: New Generation (Japan 970204)", MACHINE_SUPPORTS_SAVE )
GAME( 1997, sfiiih,      sfiii,    sfiii,    cps3,      cps3_state, init_sfiii,    ROT0, "Capcom", "Street Fighter III: New Generation (Hispanic 970204)", MACHINE_SUPPORTS_SAVE )
GAME( 1997, sfiiin,      sfiii,    sfiii,    cps3,      cps3_state, init_sfiii,    ROT0, "Capcom", "Street Fighter III: New Generation (Asia 970204, NO CD, BIOS set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1997, sfiiina,     sfiii,    sfiii,    cps3,      cps3_state, init_sfiii,    ROT0, "Capcom", "Street Fighter III: New Generation (Asia 970204, NO CD, BIOS set 2)", MACHINE_SUPPORTS_SAVE )

/* Street Fighter III 2nd Impact: Giant Attack */

// 971016
// not dumped

// 970930
GAMEL(1997, sfiii2,      0,        sfiii2,   cps3,      cps3_state, init_sfiii2,   ROT0, "Capcom", "Street Fighter III 2nd Impact: Giant Attack (USA 970930)", MACHINE_SUPPORTS_SAVE, layout_sfiii2 ) // layout is for widescreen support
GAMEL(1997, sfiii2j,     sfiii2,   sfiii2,   cps3,      cps3_state, init_sfiii2,   ROT0, "Capcom", "Street Fighter III 2nd Impact: Giant Attack (Japan 970930)", MACHINE_SUPPORTS_SAVE, layout_sfiii2 )
GAMEL(1997, sfiii2h,     sfiii2,   sfiii2,   cps3,      cps3_state, init_sfiii2,   ROT0, "Capcom", "Street Fighter III 2nd Impact: Giant Attack (Hispanic 970930)", MACHINE_SUPPORTS_SAVE, layout_sfiii2 )
GAMEL(1997, sfiii2n,     sfiii2,   sfiii2,   cps3,      cps3_state, init_sfiii2,   ROT0, "Capcom", "Street Fighter III 2nd Impact: Giant Attack (Asia 970930, NO CD)", MACHINE_SUPPORTS_SAVE, layout_sfiii2 )

/* JoJo's Venture / JoJo no Kimyou na Bouken */

// 990128
GAME( 1998, jojo,        0,        jojo,     cps3_jojo, cps3_state, init_jojo,     ROT0, "Capcom", "JoJo's Venture (Europe 990128)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, jojou,       jojo,     jojo,     cps3_jojo, cps3_state, init_jojo,     ROT0, "Capcom", "JoJo's Venture (USA 990128)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, jojoa,       jojo,     jojo,     cps3_jojo, cps3_state, init_jojo,     ROT0, "Capcom", "JoJo's Venture (Asia 990128)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, jojoj,       jojo,     jojo,     cps3_jojo, cps3_state, init_jojo,     ROT0, "Capcom", "JoJo no Kimyou na Bouken (Japan 990128)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, jojon,       jojo,     jojo,     cps3_jojo, cps3_state, init_jojo,     ROT0, "Capcom", "JoJo's Venture (Asia 990128, NO CD)", MACHINE_SUPPORTS_SAVE )

// 990108
GAME( 1998, jojor1,      jojo,     jojo,     cps3_jojo, cps3_state, init_jojo,     ROT0, "Capcom", "JoJo's Venture (Europe 990108)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, jojour1,     jojo,     jojo,     cps3_jojo, cps3_state, init_jojo,     ROT0, "Capcom", "JoJo's Venture (USA 990108)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, jojoar1,     jojo,     jojo,     cps3_jojo, cps3_state, init_jojo,     ROT0, "Capcom", "JoJo's Venture (Asia 990108)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, jojojr1,     jojo,     jojo,     cps3_jojo, cps3_state, init_jojo,     ROT0, "Capcom", "JoJo no Kimyou na Bouken (Japan 990108)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, jojonr1,     jojo,     jojo,     cps3_jojo, cps3_state, init_jojo,     ROT0, "Capcom", "JoJo's Venture (Asia 990108, NO CD)", MACHINE_SUPPORTS_SAVE )

// 981202
GAME( 1998, jojor2,      jojo,     jojo,     cps3_jojo, cps3_state, init_jojo,     ROT0, "Capcom", "JoJo's Venture (Europe 981202)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, jojour2,     jojo,     jojo,     cps3_jojo, cps3_state, init_jojo,     ROT0, "Capcom", "JoJo's Venture (USA 981202)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, jojoar2,     jojo,     jojo,     cps3_jojo, cps3_state, init_jojo,     ROT0, "Capcom", "JoJo's Venture (Asia 981202)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, jojojr2,     jojo,     jojo,     cps3_jojo, cps3_state, init_jojo,     ROT0, "Capcom", "JoJo no Kimyou na Bouken (Japan 981202)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, jojonr2,     jojo,     jojo,     cps3_jojo, cps3_state, init_jojo,     ROT0, "Capcom", "JoJo's Venture (Asia 981202, NO CD)", MACHINE_SUPPORTS_SAVE )

/* Street Fighter III 3rd Strike: Fight for the Future */

// 990608
GAME( 1999, sfiii3,      0,        sfiii3,   cps3,      cps3_state, init_sfiii3,   ROT0, "Capcom", "Street Fighter III 3rd Strike: Fight for the Future (Europe 990608)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, sfiii3u,     sfiii3,   sfiii3,   cps3,      cps3_state, init_sfiii3,   ROT0, "Capcom", "Street Fighter III 3rd Strike: Fight for the Future (USA 990608)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, sfiii3j,     sfiii3,   sfiii3,   cps3,      cps3_state, init_sfiii3,   ROT0, "Capcom", "Street Fighter III 3rd Strike: Fight for the Future (Japan 990608)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, sfiii3n,     sfiii3,   sfiii3,   cps3,      cps3_state, init_sfiii3,   ROT0, "Capcom", "Street Fighter III 3rd Strike: Fight for the Future (Japan 990608, NO CD)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, sfiii3na,    sfiii3,   sfiii3,   cps3,      cps3_state, init_sfiii3,   ROT0, "Capcom", "Street Fighter III 3rd Strike: Fight for the Future (Asia 990608, NO CD)", MACHINE_SUPPORTS_SAVE )

// 990512
GAME( 1999, sfiii3r1,    sfiii3,   sfiii3,   cps3,      cps3_state, init_sfiii3,   ROT0, "Capcom", "Street Fighter III 3rd Strike: Fight for the Future (Europe 990512)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, sfiii3ur1,   sfiii3,   sfiii3,   cps3,      cps3_state, init_sfiii3,   ROT0, "Capcom", "Street Fighter III 3rd Strike: Fight for the Future (USA 990512)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, sfiii3jr1,   sfiii3,   sfiii3,   cps3,      cps3_state, init_sfiii3,   ROT0, "Capcom", "Street Fighter III 3rd Strike: Fight for the Future (Japan 990512)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, sfiii3nr1,   sfiii3,   sfiii3,   cps3,      cps3_state, init_sfiii3,   ROT0, "Capcom", "Street Fighter III 3rd Strike: Fight for the Future (Japan 990512, NO CD)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, sfiii3nar1,  sfiii3,   sfiii3,   cps3,      cps3_state, init_sfiii3,   ROT0, "Capcom", "Street Fighter III 3rd Strike: Fight for the Future (Asia 990512, NO CD)", MACHINE_SUPPORTS_SAVE )

/* JoJo's Bizarre Adventure / JoJo no Kimyou na Bouken: Mirai e no Isan */

// 991015
GAME( 1999, jojoba,      0,        jojoba,   cps3_jojo, cps3_state, init_jojoba,   ROT0, "Capcom", "JoJo's Bizarre Adventure (Europe 991015, NO CD)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, jojoban,     jojoba,   jojoba,   cps3_jojo, cps3_state, init_jojoba,   ROT0, "Capcom", "JoJo no Kimyou na Bouken: Mirai e no Isan (Japan 991015, NO CD)", MACHINE_SUPPORTS_SAVE )

// 990927
GAME( 1999, jojobar1,    jojoba,   jojoba,   cps3_jojo, cps3_state, init_jojoba,   ROT0, "Capcom", "JoJo's Bizarre Adventure (Europe 990927)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, jojobajr1,   jojoba,   jojoba,   cps3_jojo, cps3_state, init_jojoba,   ROT0, "Capcom", "JoJo no Kimyou na Bouken: Mirai e no Isan (Japan 990927)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, jojobanr1,   jojoba,   jojoba,   cps3_jojo, cps3_state, init_jojoba,   ROT0, "Capcom", "JoJo no Kimyou na Bouken: Mirai e no Isan (Japan 990927, NO CD)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, jojobaner1,  jojoba,   jojoba,   cps3_jojo, cps3_state, init_jojoba,   ROT0, "Capcom", "JoJo's Bizarre Adventure (Europe 990927, NO CD)", MACHINE_SUPPORTS_SAVE )

// 990913
GAME( 1999, jojobar2,    jojoba,   jojoba,   cps3_jojo, cps3_state, init_jojoba,   ROT0, "Capcom", "JoJo's Bizarre Adventure (Europe 990913)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, jojobajr2,   jojoba,   jojoba,   cps3_jojo, cps3_state, init_jojoba,   ROT0, "Capcom", "JoJo no Kimyou na Bouken: Mirai e no Isan (Japan 990913)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, jojobanr2,   jojoba,   jojoba,   cps3_jojo, cps3_state, init_jojoba,   ROT0, "Capcom", "JoJo no Kimyou na Bouken: Mirai e no Isan (Japan 990913, NO CD)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, jojobaner2,  jojoba,   jojoba,   cps3_jojo, cps3_state, init_jojoba,   ROT0, "Capcom", "JoJo's Bizarre Adventure (Europe 990913, NO CD)", MACHINE_SUPPORTS_SAVE )

// bootlegs, hold START1 during bootup to change games

// newest revision, fixes some issues with Warzard decryption.
GAME( 1999, cps3boot,    0,        sfiii3,   cps3,      cps3_state, init_cps3boot, ROT0, "bootleg", "CPS3 Multi-game bootleg for HD6417095 type SH2 (V4)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, cps3boota,   cps3boot, sfiii3,   cps3,      cps3_state, init_sfiii2,   ROT0, "bootleg", "CPS3 Multi-game bootleg for dead security cart (V5)", MACHINE_SUPPORTS_SAVE )

GAME( 1999, cps3booto,   cps3boot, sfiii3,   cps3,      cps3_state, init_cps3boot, ROT0, "bootleg", "CPS3 Multi-game bootleg for HD6417095 type SH2 (older)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, cps3bootao,  cps3boot, sfiii3,   cps3,      cps3_state, init_sfiii2,   ROT0, "bootleg", "CPS3 Multi-game bootleg for dead security cart (older)", MACHINE_SUPPORTS_SAVE )
// this doesn't play 2nd Impact despite it being listed.  2nd Impact uses separate data/code encryption and can't be decrypted cleanly for a standard SH2.  Selecting it just flashes in a copy of 3rd Strike with the 2nd Impact loading screen
GAME( 1999, cps3booto2,  cps3boot, sfiii3,   cps3,      cps3_state, init_cps3boot, ROT0, "bootleg", "CPS3 Multi-game bootleg for HD6417095 type SH2 (oldest) (New Generation, 3rd Strike, JoJo's Venture, JoJo's Bizarre Adventure and Red Earth only)", MACHINE_SUPPORTS_SAVE )
// this does not play Red Earth or the 2 Jojo games.  New Generation and 3rd Strike have been heavily modified to work with the separate code/data encryption a dead cart / 2nd Impact cart has.  Selecting the other games will give an 'invalid CD' message.
GAME( 1999, cps3bootao2, cps3boot, sfiii3,   cps3,      cps3_state, init_sfiii2,   ROT0, "bootleg", "CPS3 Multi-game bootleg for dead security cart (oldest) (New Generation, 2nd Impact and 3rd Strike only)", MACHINE_SUPPORTS_SAVE )
// these are test bootleg CDs for running 2nd Impact on a standard SH2
GAME( 1999, cps3bs32,    cps3boot, sfiii3,   cps3,      cps3_state, init_cps3boot, ROT0, "bootleg", "Street Fighter III 2nd Impact: Giant Attack (USA 970930, bootleg for HD6417095 type SH2, V3)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, cps3bs32a,   cps3boot, sfiii3,   cps3,      cps3_state, init_cps3boot, ROT0, "bootleg", "Street Fighter III 2nd Impact: Giant Attack (USA 970930, bootleg for HD6417095 type SH2, older)", MACHINE_SUPPORTS_SAVE ) // older / buggier hack
