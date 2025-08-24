// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    Data East 32 bit ARM based games:

    Captain America
    Dragon Gun
    Fighter's History
    Locked 'n Loaded
    Night Slashers
    Tattoo Assassins


    Emulation by Bryan McPhail, mish@tendril.co.uk.  Thank you to Tim,
    Avedis and Stiletto for many things including work on Fighter's
    History protection and tracking down Tattoo Assassins!

    Captain America & Fighter's History - reset with both start buttons
    held down for test mode.  Reset with player 1 start held in Fighter's
    History for 'Pattern Editor'.

    For version information:
     Captain America   - Reset with Player 1 start held
     Fighter's History - Reset with Player 1 button 1 & 2 held
     Night Slashers    - Reset with Player 1 & 2 start held
     Locked 'n Loaded  - Reset with Player 1 & 2 start held

    Tattoo Assassins is a prototype, it is thought only 25 test units
    were manufactured and distributed to test arcades before the game
    was recalled.  TA is the only game developed by Data East Pinball
    in USA, rather than Data East Corporation in Japan.

    Tattoo Assassins uses DE Pinball soundboard 520-5077-00 R

    How to calibrate guns in Locked 'n Loaded and Gun Hard:
    - keep SERVICE1 pressed (default 9) and press the test mode switch (default F2)
    - complete guns' calibration
    - exit test mode

    TODO:

    Tattoo Assassins & Night slashers use an less emulated chip (Ace/Jack) for
    special blending effects.  It's exact effect is unclear.

    Video backgrounds(intel DVI) in Dragongun?
    reference video(dragngunj): https://youtu.be/TVc0SsTVJ94

    Locked 'n Loaded (parent set) is a slightly different hardware
    revision: board # DE-0420-1 where the US set is DE-0359-2.
    The sound is _not_ hooked up correctly for this set.

    Z80 Sound cpu version games : Music tempo is unverified (it has external timer / IRQ controller?).



Locked 'n Loaded (US)
Data East Corporation (c) 1994

DE-0359-2 PCB Layout - Same PCB as used for Dragon Gun, see comment below:

------------------------------------------------------------
|     32.220MHz   28.000MHz                 8M-7    8M-3   |
|                                          MBM-05  MBM-03  |
|         NH06-0   HuC6280A                 8M-5    8M-1   |
|           YM2151                         MBM-04  MBM-02  |
--+                                                        |
--+           MBM-07                                       |
|             MAR-07                74                     |
| M6295 M6295 MBM-06                       MBM-01  NH05-0  |
| M6295                                    MBM-00  NH04-0  |
|                                      74                  |
|J                                                         |
|A                                                         |
|M                                                         |
|M                                          2M-5    2M-4   |
|A  113                                    NH03-0  NH01-0  |
|                                  101      2M-7    2M-6   |
--+                                        NH02-0  NH00-0  |
--+ DSW1   146                                             |
|A             93C45                                       |
|U                                +-------------------------+
|X                                |         DE-0406-1       |
--|       ADC0808CCN              |       AUX PCB with      |
  --------------------------------|      Gun Connectors     |
                                  --------------------------+

2M-4 through 2M-7 are empty sockets for additional program ROMs (used by dragon Gun)
Odd numbered 8M are empty sockets
AUX edge connector is a 48 pin type similar to those used on Namco System 11, 12, etc


DE-0360-4 ROM board Layout:

------------------------------------------------------------
| CN2                   TC524256BZ-10 TC524256BZ-10  MAR-17|
|                       TC524256BZ-10 TC524256BZ-10  MAR-18|
| HM65256BLSP-10        TC524256BZ-10 TC524256BZ-10  MAR-19|
| 16 of these chips     TC524256BZ-10 TC524256BZ-10  MAR-20|
| in this area                                       MAR-21|
|                                       Intel i750   MAR-22|
|         187     23.000MHz                          MAR-23|
|MBM-08                                              MAR-24|
|MBM-09             20.0000MHz                       MAR-25|
|MBM-10                                      145     MAR-26|
|MBM-11  186                                         MAR-27|
|MBM-12                                              MAR-28|
|MBM-13                                                    |
|MBM-14 PAL16L8BCN                          Intel i750     |
|MBM-15 PAL16L8BCN                                         |
| CN1                           25.000MHz      PAL16L8BCN  |
------------------------------------------------------------

CN1 = Triple row 32 pin connector
CN2 = Dual row 32 pin connector

Locked 'n Loaded appears to be a conversion of Dragon Gun (c) 1993 as
there are 12 surface mounted GFX roms and 1 surface mounted sample rom
left over from the conversion.  The roms labeled "MAR-xx" are those
from Dragon Gun.



Night Slashers
Data East, 1993

PCB Layout
----------

DE-0397-0   DEC-22VO
|-----------------------------------------------------|
| TA8205AH   Z80          |-----|              HBM-07 |
|         6164    YM2151  | 52  |                     |
|         LX02            |     | HBM-09       HBM-06 |
|     YM3012      32MHz   |-----|                     |
|  JP1   HBM-11   93C45                        HBM-05 |
|CN2     HBM-10           |-----|                     |
|  M6295(1)               | 52  |              HBM-04 |
|     M6295(2)            |     |                     |
|      |-----| |-----|    |-----|    |-----|   HBM-03 |
|J     | 104 | | 153 |               | 52  |          |
|A     |     | |     |     |-----|   |     |   HBM-02 |
|M     |-----| |-----|     | 153 |   |-----|          |
|M                         |     |           28.322MHz|
|A                         |-----|                    |
|    |-----| PAL                                      |
|    | 99  | PROM              |-----| 6164    HBM-01 |
|    |     |                   | 74  | 6164           |
|    |-----|                   |     |         HBM-00 |
|      |-----|  |-----|        |-----|                |
|      | 153 |  | 200 |        |-----| 6164           |
|      |     |  |     |        | 141 | 6164           |
|      |-----|  |-----|        |     |   PAL          |
|TEST_SW                       |-----|   PAL  |-----| |
|                                             | 156 | |
|           LH52250  LH52250   LX-01          |     | |
|  CN3      LH52250  LH52250   LX-00          |-----| |
|-----------------------------------------------------|
Notes:
       The CPU is chip 156. It's an encrypted ARM-based CPU. The CPU is running at 7.0805MHz [28.322/4]

       Z80B      - Goldstar Z8400B, running at 3.5555MHz [32/9]
       YM2151    - Yamaha YM2151 sound chip, running at 3.5555MHz [32/9]
       OKI M6295 - Oki M6295 PCM Sample chip, (1) running at 1.000MHz [32/32]. Sample rate = 1000000 / 132
                                              (2) running at 2.000MHz [32/16]. Sample rate = 2000000 / 132
       6164      - UM6164BK-20 8K x8 SRAM
       LH52250   - Sharp LH52250 32K x8 SRAM
       93C45     - 128bytes x8 Serial EEPROM
       PALs      - PAL 16L8ACN (x 2, near program ROMs, one at 3D labelled 'VM-00', one at 4D labelled 'VM-01')
                   PAL 16l8ACN (near chip 99, located at 8J, labelled 'VM-02')
       HSync     - 15.86kHz
       VSync     - 58Hz
       Custom ICs-
                   DE #            Package Type      Additional #'s (for reference of scratched-off chips on other PCB's)
                   ------------------------------------------------------------------------------------------------------
                   156 (CPU)       100 Pin PQFP      932EV 301801
                   141             160 Pin PQFP      24220F008
                   74              160 Pin PQFP      24220F009
                   99              208 Pin PQFP      L7A0967
                   52 (x3)         128 Pin PQFP      9322EV 298251 VC5259-0001
                   153 (x3)        144 Pin PQFP      L7A0888 9328
                   104             100 Pin PQFP      L7A0717 9148
                   200             100 Pin PQFP      JAPAN 9320PD027  (chip is darker black)

       Other     - There's a small push button near the JAMMA connector to access test mode.
                   All settings are via an on-board menu.

                   All pinouts conform to standard JAMMA pinout.  Joystick is 8-way with 3 buttons used.

                   JP1: 1-2 Sound Output in MONO
                        2-3 Sound Output in STEREO

                   CN2: 4 Pin connector (use when JP1 = 2-3)
                        Pin #   Signal
                         1      L-Speaker +
                         2      L-Speaker -
                         3      R-Speaker -
                         4      R-Speaker +

                   CN3: 15 Pin connector (Player 3)
                        Pin #   Signal
                         1      COIN SW3
                         2      3P PUSH 3
                         3      3P LEFT
                         4      3P RIGHT
                         5      3P UP
                         6      3P DOWN
                         7      3P PUSH 1
                         8      3P PUSH 2
                         9-13   NOT USED
                         14-15  GND


       ROMs      - MAINPRG1.1F     HN27C4096      \
                   MAINPRG2.2F     HN27C4096      /  Main Program (no ROM stickers attached, DE ROM code unknown)

                   Japanese Version
                   LX01-.2F        HN27C4096      \
                   LX00-.1F        HN27C4096      /  Main Program (Japan version)

                   MBH-00.8C   42 pin 16M MASK  (read as 27C160)    \
                   MBH-01.9C   42 pin 16M MASK  (read as 27C160)    |
                   MBH-02.14C  42 pin 16M MASK  (read as 27C160)    |
                   MBH-03.15C  40 pin 4M MASK   (read as MX27C4100) |
                   MBH-04.16C  42 pin 16M MASK  (read as 27C160)    |
                   MBH-05.17C  40 pin 4M MASK   (read as MX27C4100) | GFX
                   MBH-06.18C  32 pin 8M MASK   (read as 27C080)    |
                   MBH-07.19C  32 pin 2M MASK   (read as 27C020)    |
                   MBH-08.16E  40 pin 4M MASK   (read as MX27C2100) |
                   MBH-09.18E  40 pin 4M MASK   (read as MX27C2100) /

                   MBH-10.14L  32 pin 4M MASK   (read as 27C040)    \
                   MBH-11.16L  32 pin 4M MASK   (read as 27C040)    / Sound (Samples)

                   LX02-.17L   27C512                                 Sound Program

                   PROM.9J     Fujitsu MB7124 compatible with 82S147  Labelled 'LN-00'

Night Slashers
Data East, 1993

DE-0395-1   DEC-22VO
|-----------------------------------------------------|
| TA8205AH  6164            |-----|   2M-16M*   MBH-07|
|            02-  HuC6280A  | 52  |   MBH-09    MBH-06|
|           YM2151          |     |   2M-16M*   MBH-05|
|     YM3012      32.220MHz |-----|   MBH-08    MBH-04|
|  JP1   MBH-11   93C45     |-----|             MBH-03|
|CN2     MBH-10             | 52  |             MBH-02|
|  M6295(1)                 |     |                   |
|     M6295(2)              |-----|                   |
|      |-----|                       |-----|          |
|J     | 104 |    |-----|  |-----|   | 52  |          |
|A     |     |    | 153 |  | 113 |   |     |          |
|M     |-----|    |     |  |     |   |-----|          |
|M                |-----|  |-----|            28MHz   |
|A  |-----|                                           |
|   | 99  |  VM-02                                    |
|   |     |  LN-00             |-----| 6164    MBH-01 |
|   |-----|                    | 74  | 6164           |
|                              |     |         MBH-00 |
|      |-----|  |-----|        |-----|                |
|      | 113 |  | 200 |        |-----| 6164           |
|      |     |  |     |        | 141 | 6164           |
|      |-----|  |-----|        |     |  VE-01A        |
|TEST_SW                       |-----|  VE-00 |-----| |
|                                             | 156 | |
|  CN4      LH52250  LH52250     01-          |     | |
|  CN3      LH52250  LH52250     00-          |-----| |
|-----------------------------------------------------|

NOTE: Program EPROMs did NOT have regional letter codes, just 00, 01 & 02

Same PCB as shown below for Fighter's History, but populated with more
chips as needed. It is worth noting that Night Slashers uses the encryption
features of the 156 chip where as Fighter's History does not.

*****************************************************************************************************

Fighter's History
Data East, 1993

PCB Layout
----------

DE-0395-1   DEC-22VO
|-----------------------------------------------------|
| TA8205AH  6164                      MBF-05    2M-8M*|
|           LE02  HuC6280A            MBF-04       8M*|
|           YM2151            52*     MBF-03   4M-16M*|
|     YM3012      32.220MHz           MBF-02      16M*|
|  JP1   MBF-07   93C45                        4M-16M*|
|CN2     MBF-06                                   16M*|
|  M6295(1)                   52*                     |
|     M6295(2)                                        |
|      |-----|                       |-----|          |
|J     | 75  |   113/153*            | 52  |          |
|A     |     |             |-----|   |     |          |
|M     |-----|             | 113 |   |-----|          |
|M                         |     |            28MHz   |
|A                         |-----|                    |
|            PAL*                                     |
|      99*   KT-00             |-----| 6164    MBF-01 |
|                              | 74  | 6164           |
|                              |     |         MBF-00 |
|      |-----|  |-----|        |-----|                |
|      | 113 |  | 200 |        |-----| 6164           |
|      |     |  |     |        | 56  | 6164           |
|      |-----|  |-----|        |     |  VE-01A        |
|TEST_SW                       |-----|  VE-00 |-----| |
|                                             | 156 | |
|  CN4      LH52250  LH52250   LE01           |     | |
|  CN3*     LH52250  LH52250   LE00           |-----| |
|-----------------------------------------------------|


Very similar to the DE-0396-0 described below with the notable exceptions:
  The sound area reworked to use the HuC6280A instead of a standard Z80
  Uses the larger 113 instead of the 153 chips, however both PCBs have
    solder pads in the same locations for either chip.
  Uses the 56 instead of the 141


DE-0396-0   DEC-22VO
|-----------------------------------------------------|
| TA8205AH  Z80                       MBF-05    2M-8M*|
|           6164  YM2151              MBF-04       8M*|
|           LJ02              52*     MBF-03   4M-16M*|
|     YM3012      32.220MHz           MBF-02      16M*|
|  JP1   MBF-07   93C45                        4M-16M*|
|CN2     MBF-06                                   16M*|
|  M6295(1)                   52*                     |
|     M6295(2)                                        |
|      |-----|                       |-----|          |
|J     | 75  |   113/153*            | 52  |          |
|A     |     |             |-----|   |     |          |
|M     |-----|             | 153 |   |-----|          |
|M                         |     |            28MHz   |
|A                         |-----|                    |
|            PAL*                                     |
|      99*   KT-00             |-----| 6164    MBF-01 |
|                              | 74  | 6164           |
|                              |     |         MBF-00 |
|      |-----|  |-----|        |-----|                |
|      | 153 |  | 200 |        |-----| 6164           |
|      |     |  |     |        | 141 | 6164           |
|      |-----|  |-----|        |     |  VE-01A        |
|TEST_SW                       |-----|  VE-00 |-----| |
|                                             | 156 | |
|  CN4      LH52250  LH52250   LJ01-3         |     | |
|  CN3*     LH52250  LH52250   LJ00-3         |-----| |
|-----------------------------------------------------|

This PCB is very close to the DE-397-0 listed above

       Custom ICs-
                   DE #            Package Type      Additional #'s (for reference of scratched-off chips on other PCB's)
                   ------------------------------------------------------------------------------------------------------
                   156 (CPU)       100 Pin PQFP      9321EV 301811 (Doesn't use encryption functions of the 156)
                   141             160 Pin PQFP      24220F008
                   74              160 Pin PQFP      24220F009
                   52              128 Pin PQFP      9313EV 211771 VC5259-0001
                   153 (x2)        144 Pin PQFP      L7A0888 9312
                   75              100 Pin PQFP      L7A0680 9143
                   200             100 Pin PQFP      JAPAN 9315PP002  (chip is darker black)

NOTE: There are several unpopulated locations (denoted by *) for additional rom chips
      or more custom ICs.

***************************************************************************/

#include "emu.h"
#include "deco32.h"

#include "deco156_m.h"
#include "decocrpt.h"

#include "cpu/arm/arm.h"
#include "cpu/m6809/m6809.h"
#include "cpu/z80/z80.h"
#include "machine/input_merger.h"

#include "speaker.h"

#include <algorithm>


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void captaven_state::captaven_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x100007).r(FUNC(captaven_state::_71_r));
	map(0x100000, 0x100003).w(FUNC(captaven_state::buffer_spriteram_w<0>));
	map(0x108000, 0x108003).nopw(); // ?
	map(0x110000, 0x111fff).rw(FUNC(captaven_state::spriteram_r<0>), FUNC(captaven_state::spriteram_w<0>));
	map(0x120000, 0x127fff).ram(); // Main RAM
	map(0x128000, 0x12ffff).rw(FUNC(captaven_state::ioprot_r), FUNC(captaven_state::ioprot_w)).umask32(0x0000ffff);
	map(0x130000, 0x131fff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0x148000, 0x14800f).m(m_deco_irq, FUNC(deco_irq_device::map)).umask32(0x000000ff);
	map(0x160000, 0x167fff).ram(); // Extra work RAM
	map(0x168000, 0x168000).lr8(NAME([this] () { return m_io_dsw[0]->read(); }));
	map(0x168001, 0x168001).lr8(NAME([this] () { return m_io_dsw[1]->read(); }));
	map(0x168002, 0x168002).lr8(NAME([this] () { return m_io_dsw[2]->read(); }));
	map(0x168003, 0x168003).r(FUNC(captaven_state::captaven_soundcpu_status_r));
	map(0x178000, 0x178003).w(FUNC(captaven_state::pri_w));
	map(0x180000, 0x18001f).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf_control_dword_r), FUNC(deco16ic_device::pf_control_dword_w));
	map(0x190000, 0x191fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf1_data_dword_r), FUNC(deco16ic_device::pf1_data_dword_w));
	map(0x192000, 0x193fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf1_data_dword_r), FUNC(deco16ic_device::pf1_data_dword_w)); // Mirror address - bug in program code
	map(0x194000, 0x195fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf2_data_dword_r), FUNC(deco16ic_device::pf2_data_dword_w));
	map(0x1a0000, 0x1a3fff).ram().w(FUNC(captaven_state::pf_rowscroll_w<0>)).share("pf1_rowscroll32");
	map(0x1a4000, 0x1a5fff).ram().w(FUNC(captaven_state::pf_rowscroll_w<1>)).share("pf2_rowscroll32");
	map(0x1c0000, 0x1c001f).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf_control_dword_r), FUNC(deco16ic_device::pf_control_dword_w));
	map(0x1d0000, 0x1d1fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf1_data_dword_r), FUNC(deco16ic_device::pf1_data_dword_w));
	map(0x1d4000, 0x1d5fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf2_data_dword_r), FUNC(deco16ic_device::pf2_data_dword_w)); // unused
	map(0x1e0000, 0x1e3fff).ram().w(FUNC(captaven_state::pf_rowscroll_w<2>)).share("pf3_rowscroll32");
	map(0x1e4000, 0x1e5fff).ram().w(FUNC(captaven_state::pf_rowscroll_w<3>)).share("pf4_rowscroll32"); // unused
}

void fghthist_state::fghthist_map(address_map &map)
{
	map.unmap_value_high();
//  map(0x000000, 0x001fff).rom().w(FUNC(fghthist_state::pf1_data_w)); // wtf??
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x11ffff).ram();
	map(0x120020, 0x120021).lr16(NAME([this] () { return m_io_in[0]->read(); }));
	map(0x120024, 0x120025).lr16(NAME([this] () { return m_io_in[1]->read(); }));
	map(0x120028, 0x120028).r(FUNC(fghthist_state::eeprom_r));
	map(0x12002c, 0x12002c).w(FUNC(fghthist_state::eeprom_w));
	map(0x12002d, 0x12002d).w(FUNC(fghthist_state::volume_w));
	map(0x1201fc, 0x1201fc).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x140000, 0x140003).w(FUNC(fghthist_state::vblank_ack_w));
	map(0x168000, 0x169fff).ram().w(FUNC(fghthist_state::buffered_palette_w)).share("paletteram");
	map(0x16c008, 0x16c00b).w(FUNC(fghthist_state::palette_dma_w));
	map(0x16c010, 0x16c013).r(FUNC(fghthist_state::unk_status_r));
	map(0x178000, 0x179fff).rw(FUNC(fghthist_state::spriteram_r<0>), FUNC(fghthist_state::spriteram_w<0>));
	map(0x17c010, 0x17c013).w(FUNC(fghthist_state::buffer_spriteram_w<0>));
	map(0x17c020, 0x17c023).r(FUNC(fghthist_state::unk_status_r));
	map(0x182000, 0x183fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf1_data_dword_r), FUNC(deco16ic_device::pf1_data_dword_w));
	map(0x184000, 0x185fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf2_data_dword_r), FUNC(deco16ic_device::pf2_data_dword_w));
	map(0x192000, 0x193fff).ram().w(FUNC(fghthist_state::pf_rowscroll_w<0>)).share("pf1_rowscroll32");
	map(0x194000, 0x195fff).ram().w(FUNC(fghthist_state::pf_rowscroll_w<1>)).share("pf2_rowscroll32");
	map(0x1a0000, 0x1a001f).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf_control_dword_r), FUNC(deco16ic_device::pf_control_dword_w));
	map(0x1c2000, 0x1c3fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf1_data_dword_r), FUNC(deco16ic_device::pf1_data_dword_w));
	map(0x1c4000, 0x1c5fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf2_data_dword_r), FUNC(deco16ic_device::pf2_data_dword_w));
	map(0x1d2000, 0x1d3fff).ram().w(FUNC(fghthist_state::pf_rowscroll_w<2>)).share("pf3_rowscroll32");
	map(0x1d4000, 0x1d5fff).ram().w(FUNC(fghthist_state::pf_rowscroll_w<3>)).share("pf4_rowscroll32");
	map(0x1e0000, 0x1e001f).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf_control_dword_r), FUNC(deco16ic_device::pf_control_dword_w));
	map(0x200000, 0x207fff).rw(FUNC(fghthist_state::ioprot_r), FUNC(fghthist_state::ioprot_w)).umask32(0xffff0000).share("prot32ram"); // only maps on 16-bits
	map(0x208800, 0x208803).nopw(); // ?
}

void fghthist_state::fghthsta_memmap(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x11ffff).ram();
	map(0x140000, 0x140003).w(FUNC(fghthist_state::vblank_ack_w));
	map(0x150000, 0x150000).w(FUNC(fghthist_state::eeprom_w));
	map(0x150001, 0x150001).w(FUNC(fghthist_state::volume_w));
	map(0x168000, 0x169fff).ram().w(FUNC(fghthist_state::buffered_palette_w)).share("paletteram");
	map(0x16c008, 0x16c00b).w(FUNC(fghthist_state::palette_dma_w));
	map(0x16c010, 0x16c013).r(FUNC(fghthist_state::unk_status_r));
	map(0x178000, 0x179fff).rw(FUNC(fghthist_state::spriteram_r<0>), FUNC(fghthist_state::spriteram_w<0>));
	map(0x17c010, 0x17c013).w(FUNC(fghthist_state::buffer_spriteram_w<0>));
	map(0x17c020, 0x17c023).r(FUNC(fghthist_state::unk_status_r));
	map(0x182000, 0x183fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf1_data_dword_r), FUNC(deco16ic_device::pf1_data_dword_w));
	map(0x184000, 0x185fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf2_data_dword_r), FUNC(deco16ic_device::pf2_data_dword_w));
	map(0x192000, 0x193fff).ram().w(FUNC(fghthist_state::pf_rowscroll_w<0>)).share("pf1_rowscroll32");
	map(0x194000, 0x195fff).ram().w(FUNC(fghthist_state::pf_rowscroll_w<1>)).share("pf2_rowscroll32");
	map(0x1a0000, 0x1a001f).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf_control_dword_r), FUNC(deco16ic_device::pf_control_dword_w));
	map(0x1c2000, 0x1c3fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf1_data_dword_r), FUNC(deco16ic_device::pf1_data_dword_w));
	map(0x1c4000, 0x1c5fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf2_data_dword_r), FUNC(deco16ic_device::pf2_data_dword_w));
	map(0x1d2000, 0x1d3fff).ram().w(FUNC(fghthist_state::pf_rowscroll_w<2>)).share("pf3_rowscroll32");
	map(0x1d4000, 0x1d5fff).ram().w(FUNC(fghthist_state::pf_rowscroll_w<3>)).share("pf4_rowscroll32");
	map(0x1e0000, 0x1e001f).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf_control_dword_r), FUNC(deco16ic_device::pf_control_dword_w));
	map(0x200000, 0x207fff).rw(FUNC(fghthist_state::ioprot_r), FUNC(fghthist_state::ioprot_w)).umask32(0xffff0000).share("prot32ram"); // only maps on 16-bits
}

void dragngun_state::namcosprite_map(address_map &map)
{
	map(0x204800, 0x204fff).ram().share(m_sprite_cliptable);// (all entries set to 320x256 here)
	map(0x208000, 0x208fff).ram().share(m_sprite_spriteformat[0]);
	map(0x20c000, 0x20cfff).ram().share(m_sprite_spriteformat[1]);
	map(0x210000, 0x217fff).ram().share(m_sprite_spritetile[0]);
	map(0x218000, 0x21ffff).ram().share(m_sprite_spritetile[1]);
	map(0x220000, 0x221fff).ram().share("spriteram"); // Main spriteram
	map(0x228000, 0x2283ff).ram().share(m_sprite_indextable); // sprite index (just a table 0x00-0xff here)
	map(0x230000, 0x230003).w(FUNC(dragngun_state::spriteram_dma_w));
}

// the video drawing (especially sprite) code on this is too slow to cope with proper partial updates
// raster effects appear to need some work on it anyway?
void dragngun_state::dragngun_map(address_map &map)
{
	map(0x0000000, 0x00fffff).rom().region("maincpu", 0x000000);
	map(0x0100000, 0x011ffff).ram();
	map(0x0120000, 0x0127fff).rw(FUNC(dragngun_state::ioprot_r), FUNC(dragngun_state::ioprot_w)).umask32(0x0000ffff);
//  map(0x01204c0, 0x01204c3).w(FUNC(dragngun_state::sound_w));
	map(0x0128000, 0x012800f).m(m_deco_irq, FUNC(deco_irq_device::map)).umask32(0x000000ff);
	map(0x0130000, 0x0131fff).ram().w(FUNC(dragngun_state::buffered_palette_w)).share("paletteram");
	map(0x0138000, 0x0138003).noprw(); // Palette dma complete in bit 0x8? ack?  return 0 else tight loop
	map(0x0138008, 0x013800b).w(FUNC(dragngun_state::palette_dma_w));
//  map(0x0150000, 0x0150003).nopw(); // Unknown; Masking related?
//  map(0x0160000, 0x0160003).w(FUNC(dragngun_state::pri_w)); // priority
	map(0x0170100, 0x0170103).nopw();
	map(0x0170038, 0x017003b).nopw();
	map(0x017002C, 0x017002f).nopw();
	map(0x0170224, 0x0170227).nopw();
	map(0x0180000, 0x018001f).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf_control_dword_r), FUNC(deco16ic_device::pf_control_dword_w));
	map(0x0190000, 0x0191fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf1_data_dword_r), FUNC(deco16ic_device::pf1_data_dword_w));
	map(0x0194000, 0x0195fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf2_data_dword_r), FUNC(deco16ic_device::pf2_data_dword_w));
	map(0x01a0000, 0x01a3fff).ram().w(FUNC(dragngun_state::pf_rowscroll_w<0>)).share("pf1_rowscroll32");
	map(0x01a4000, 0x01a5fff).ram().w(FUNC(dragngun_state::pf_rowscroll_w<1>)).share("pf2_rowscroll32");
	map(0x01c0000, 0x01c001f).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf_control_dword_r), FUNC(deco16ic_device::pf_control_dword_w));
	map(0x01d0000, 0x01d1fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf1_data_dword_r), FUNC(deco16ic_device::pf1_data_dword_w));
	map(0x01d4000, 0x01d5fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf2_data_dword_r), FUNC(deco16ic_device::pf2_data_dword_w)); // unused
	map(0x01e0000, 0x01e3fff).ram().w(FUNC(dragngun_state::pf_rowscroll_w<2>)).share("pf3_rowscroll32");
	map(0x01e4000, 0x01e5fff).ram().w(FUNC(dragngun_state::pf_rowscroll_w<3>)).share("pf4_rowscroll32"); // unused

	//  0x2xxxxx Namco Zooming Sprites
	namcosprite_map(map);

	map(0x0300000, 0x03fffff).rom().region("maincpu", 0x100000);
	map(0x0400000, 0x0400000).rw("oki3", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x0410000, 0x0410003).w(FUNC(dragngun_state::volume_w));
	map(0x0418000, 0x0418003).w(FUNC(dragngun_state::speaker_switch_w));
	map(0x0420000, 0x0420000).rw(FUNC(dragngun_state::eeprom_r), FUNC(dragngun_state::eeprom_w));
	map(0x0430000, 0x043001f).w(FUNC(dragngun_state::lightgun_w));
	map(0x0438000, 0x0438003).r(FUNC(dragngun_state::lightgun_r));
	map(0x0440000, 0x0440003).portr("IN2");
	map(0x0500000, 0x0500003).w(FUNC(dragngun_state::sprite_control_w));
	// this is clearly the dvi video related area
	map(0x1000000, 0x1000007).r(FUNC(dragngun_state::unk_video_r));
	map(0x1000100, 0x1007fff).ram();
	map(0x10b0000, 0x10b01ff).ram();
	map(0x1400000, 0x1ffffff).rom().region("dvi", 0x00000); // reads from here during boss battles when the videos should be displayed at the offsets where the DVI headers are                                                                 // as a result it ends up writing what looks like pointers to the frame data in the ram area above
}

void dragngun_state::lockloadu_map(address_map &map)
{
	dragngun_map(map);
	map(0x0170000, 0x0170007).r(FUNC(dragngun_state::lockload_gun_mirror_r)); // Not on Dragongun
}

void dragngun_state::lockload_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom().region("maincpu", 0x000000);
	map(0x100000, 0x11ffff).ram();
	map(0x120000, 0x127fff).rw(FUNC(dragngun_state::ioprot_r), FUNC(dragngun_state::ioprot_w)).umask32(0x0000ffff);
	map(0x128000, 0x12800f).m(m_deco_irq, FUNC(deco_irq_device::map)).umask32(0x000000ff);
	map(0x130000, 0x131fff).ram().w(FUNC(dragngun_state::buffered_palette_w)).share("paletteram");
	map(0x138000, 0x138003).readonly().nopw(); //palette dma complete in bit 0x8? ack?  return 0 else tight loop
	map(0x138008, 0x13800b).w(FUNC(dragngun_state::palette_dma_w));
	map(0x170000, 0x170007).r(FUNC(dragngun_state::lockload_gun_mirror_r)); // Not on Dragongun
	map(0x178008, 0x17800f).w(FUNC(dragngun_state::gun_irq_ack_w)); // Gun read ACK's
	map(0x180000, 0x18001f).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf_control_dword_r), FUNC(deco16ic_device::pf_control_dword_w));
	map(0x190000, 0x191fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf1_data_dword_r), FUNC(deco16ic_device::pf1_data_dword_w));
	map(0x194000, 0x195fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf2_data_dword_r), FUNC(deco16ic_device::pf2_data_dword_w));
	map(0x1a0000, 0x1a3fff).ram().w(FUNC(dragngun_state::pf_rowscroll_w<0>)).share("pf1_rowscroll32");
	map(0x1a4000, 0x1a5fff).ram().w(FUNC(dragngun_state::pf_rowscroll_w<1>)).share("pf2_rowscroll32");
	map(0x1c0000, 0x1c001f).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf_control_dword_r), FUNC(deco16ic_device::pf_control_dword_w));
	map(0x1d0000, 0x1d1fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf1_data_dword_r), FUNC(deco16ic_device::pf1_data_dword_w));
	map(0x1d4000, 0x1d5fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf2_data_dword_r), FUNC(deco16ic_device::pf2_data_dword_w)); // unused
	map(0x1e0000, 0x1e3fff).ram().w(FUNC(dragngun_state::pf_rowscroll_w<2>)).share("pf3_rowscroll32");
	map(0x1e4000, 0x1e5fff).ram().w(FUNC(dragngun_state::pf_rowscroll_w<3>)).share("pf4_rowscroll32"); // unused

	//  0x2xxxxx Namco Zooming Sprites
	namcosprite_map(map);

	map(0x300000, 0x3fffff).rom().region("maincpu", 0x100000);
	map(0x410000, 0x410003).w(FUNC(dragngun_state::volume_w));
	map(0x420000, 0x420000).rw(FUNC(dragngun_state::eeprom_r), FUNC(dragngun_state::eeprom_w));
	map(0x440000, 0x440003).portr("IN2");
	map(0x500000, 0x500003).w(FUNC(dragngun_state::sprite_control_w));
}

void tattass_state::tattass_map(address_map &map)
{
	map(0x000000, 0x0f7fff).rom();
	map(0x0f8000, 0x0fffff).rom().nopw();
	map(0x100000, 0x11ffff).ram();
	map(0x120000, 0x120003).noprw();             // ACIA (unused)
	map(0x130000, 0x130003).nopw();        // Coin port (unused?)
	map(0x140000, 0x140003).w(FUNC(tattass_state::vblank_ack_w));
	map(0x150000, 0x150003).w(FUNC(tattass_state::tattass_control_w)); // Volume port/Eprom/Priority
	map(0x162000, 0x162fff).ram();             // 'Jack' RAM!?
	map(0x163000, 0x16309f).rw(m_deco_ace, FUNC(deco_ace_device::ace_r), FUNC(deco_ace_device::ace_w)).umask32(0x0000ffff); // 'Ace' RAM
	map(0x164000, 0x164000).w(FUNC(tattass_state::tilemap_color_bank_w));
	map(0x164004, 0x164004).w(FUNC(tattass_state::sprite1_color_bank_w));
	map(0x164008, 0x164008).w(FUNC(tattass_state::sprite2_color_bank_w));
	map(0x16400c, 0x16400f).nopw();
	map(0x168000, 0x169fff).rw(m_deco_ace, FUNC(deco_ace_device::buffered_palette_r), FUNC(deco_ace_device::buffered_palette_w));
	map(0x16c000, 0x16c003).nopw();
	map(0x16c008, 0x16c00b).w(m_deco_ace, FUNC(deco_ace_device::palette_dma_w));
	map(0x170000, 0x171fff).rw(FUNC(tattass_state::spriteram_r<0>), FUNC(tattass_state::spriteram_w<0>));
	map(0x174000, 0x174003).nopw(); // Sprite DMA mode (2)
	map(0x174010, 0x174013).w(FUNC(tattass_state::buffer_spriteram_w<0>));
	map(0x174018, 0x17401b).nopw(); // Sprite 'CPU' (unused)
	map(0x178000, 0x179fff).rw(FUNC(tattass_state::spriteram_r<1>), FUNC(tattass_state::spriteram_w<1>));
	map(0x17c000, 0x17c003).nopw(); // Sprite DMA mode (2)
	map(0x17c010, 0x17c013).w(FUNC(tattass_state::buffer_spriteram_w<1>));
	map(0x17c018, 0x17c01b).nopw(); // Sprite 'CPU' (unused)
	map(0x182000, 0x183fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf1_data_dword_r), FUNC(deco16ic_device::pf1_data_dword_w));
	map(0x184000, 0x185fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf2_data_dword_r), FUNC(deco16ic_device::pf2_data_dword_w));
	map(0x192000, 0x193fff).ram().w(FUNC(tattass_state::pf_rowscroll_w<0>)).share("pf1_rowscroll32");
	map(0x194000, 0x195fff).ram().w(FUNC(tattass_state::pf_rowscroll_w<1>)).share("pf2_rowscroll32");
	map(0x1a0000, 0x1a001f).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf_control_dword_r), FUNC(deco16ic_device::pf_control_dword_w));
	map(0x1c2000, 0x1c3fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf1_data_dword_r), FUNC(deco16ic_device::pf1_data_dword_w));
	map(0x1c4000, 0x1c5fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf2_data_dword_r), FUNC(deco16ic_device::pf2_data_dword_w));
	map(0x1d2000, 0x1d3fff).ram().w(FUNC(tattass_state::pf_rowscroll_w<2>)).share("pf3_rowscroll32");
	map(0x1d4000, 0x1d5fff).ram().w(FUNC(tattass_state::pf_rowscroll_w<3>)).share("pf4_rowscroll32");
	map(0x1e0000, 0x1e001f).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf_control_dword_r), FUNC(deco16ic_device::pf_control_dword_w));
	map(0x200000, 0x207fff).rw(FUNC(tattass_state::ioprot_r), FUNC(tattass_state::ioprot_w)).umask32(0xffff0000);
	map(0x200000, 0x207fff).r(FUNC(tattass_state::nslasher_debug_r)).umask32(0x0000ffff);
}

void nslasher_state::nslasher_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x11ffff).ram();
	map(0x120000, 0x1200ff).noprw();                         // ACIA (unused)
	map(0x140000, 0x140003).w(FUNC(nslasher_state::vblank_ack_w));
	map(0x150000, 0x150000).w(FUNC(nslasher_state::eeprom_w));
	map(0x150001, 0x150001).w(FUNC(nslasher_state::volume_w));
	map(0x163000, 0x16309f).rw(m_deco_ace, FUNC(deco_ace_device::ace_r), FUNC(deco_ace_device::ace_w)).umask32(0x0000ffff); // 'Ace' RAM
	map(0x164000, 0x164000).w(FUNC(nslasher_state::tilemap_color_bank_w));
	map(0x164004, 0x164004).w(FUNC(nslasher_state::sprite1_color_bank_w));
	map(0x164008, 0x164008).w(FUNC(nslasher_state::sprite2_color_bank_w));
	map(0x16400c, 0x16400f).nopw();
	map(0x168000, 0x169fff).rw(m_deco_ace, FUNC(deco_ace_device::buffered_palette_r), FUNC(deco_ace_device::buffered_palette_w));
	map(0x16c000, 0x16c003).nopw();
	map(0x16c008, 0x16c00b).w(m_deco_ace, FUNC(deco_ace_device::palette_dma_w));
	map(0x170000, 0x171fff).rw(FUNC(nslasher_state::spriteram_r<0>), FUNC(nslasher_state::spriteram_w<0>));
	map(0x174000, 0x174003).nopw(); // Sprite DMA mode (2)
	map(0x174010, 0x174013).w(FUNC(nslasher_state::buffer_spriteram_w<0>));
	map(0x174018, 0x17401b).nopw(); // Sprite 'CPU' (unused)
	map(0x178000, 0x179fff).rw(FUNC(nslasher_state::spriteram_r<1>), FUNC(nslasher_state::spriteram_w<1>));
	map(0x17c000, 0x17c003).nopw(); // Sprite DMA mode (2)
	map(0x17c010, 0x17c013).w(FUNC(nslasher_state::buffer_spriteram_w<1>));
	map(0x17c018, 0x17c01b).nopw(); // Sprite 'CPU' (unused)
	map(0x182000, 0x183fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf1_data_dword_r), FUNC(deco16ic_device::pf1_data_dword_w));
	map(0x184000, 0x185fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf2_data_dword_r), FUNC(deco16ic_device::pf2_data_dword_w));
	map(0x192000, 0x193fff).ram().w(FUNC(nslasher_state::pf_rowscroll_w<0>)).share("pf1_rowscroll32");
	map(0x194000, 0x195fff).ram().w(FUNC(nslasher_state::pf_rowscroll_w<1>)).share("pf2_rowscroll32");
	map(0x1a0000, 0x1a001f).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf_control_dword_r), FUNC(deco16ic_device::pf_control_dword_w));
	map(0x1c2000, 0x1c3fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf1_data_dword_r), FUNC(deco16ic_device::pf1_data_dword_w));
	map(0x1c4000, 0x1c5fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf2_data_dword_r), FUNC(deco16ic_device::pf2_data_dword_w));
	map(0x1d2000, 0x1d3fff).ram().w(FUNC(nslasher_state::pf_rowscroll_w<2>)).share("pf3_rowscroll32");
	map(0x1d4000, 0x1d5fff).ram().w(FUNC(nslasher_state::pf_rowscroll_w<3>)).share("pf4_rowscroll32");
	map(0x1e0000, 0x1e001f).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf_control_dword_r), FUNC(deco16ic_device::pf_control_dword_w));
	map(0x200000, 0x207fff).rw(FUNC(nslasher_state::ioprot_r), FUNC(nslasher_state::ioprot_w)).umask32(0xffff0000);
	map(0x200000, 0x207fff).r(FUNC(nslasher_state::nslasher_debug_r)).umask32(0x0000ffff); // seems to be debug switches / code activated by this?
}

// H6280 based sound
void deco32_state::h6280_sound_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x110000, 0x110001).rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x120000, 0x120001).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x130000, 0x130001).rw(m_oki[1], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x140000, 0x140000).r(m_ioprot, FUNC(deco_146_base_device::soundlatch_r));
	map(0x1f0000, 0x1f1fff).ram();
}

void deco32_state::h6280_sound_custom_latch_map(address_map &map)
{
	h6280_sound_map(map);
	map(0x140000, 0x140000).r("soundlatch", FUNC(generic_latch_8_device::read));
}

// Z80 based sound
void deco32_state::z80_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xa000, 0xa001).rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xb000, 0xb000).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xc000, 0xc000).rw(m_oki[1], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xd000, 0xd000).r(m_ioprot, FUNC(deco_146_base_device::soundlatch_r));
}

void deco32_state::z80_sound_io(address_map &map)
{
	map(0x0000, 0xffff).rom().region("audiocpu", 0);
}

// lockload needs hi bits of OKI2 bankswitching
void dragngun_state::lockload_sound_map(address_map &map)
{
	z80_sound_map(map);
	map(0xe000, 0xe000).w(FUNC(dragngun_state::lockload_okibank_hi_w));
}

void dragngun_state::lockloadu_sound_map(address_map &map)
{
	h6280_sound_map(map);
	map(0x150000, 0x150000).w(FUNC(dragngun_state::lockload_okibank_hi_w));
}


//**************************************************************************
//  PROTECTION
//**************************************************************************

u16 deco32_state::ioprot_r(offs_t offset)
{
	const offs_t real_address = 0 + (offset * 2);
	const offs_t deco146_addr = (BIT(real_address, 14, 4) << 11) | BIT(real_address, 0, 11); // NC 31-18, 13-11
	u8 cs = 0;

	return m_ioprot->read_data(deco146_addr, cs);
}

void deco32_state::ioprot_w(offs_t offset, u16 data, u16 mem_mask)
{
	const offs_t real_address = 0 + (offset * 2);
	const offs_t deco146_addr = (BIT(real_address, 14, 4) << 11) | BIT(real_address, 0, 11); // NC 31-18, 13-11
	u8 cs = 0;

	m_ioprot->write_data(deco146_addr, data, mem_mask, cs);
}


//**************************************************************************
//  SOUND
//**************************************************************************

void deco32_state::volume_w(u8 data)
{
	// TODO: assume linear with a 0.0-1.0 dB scale for now
	const u8 raw_vol = 0xff - data;
	const float vol_output = float(raw_vol) / 255.0f;

	m_ym2151->set_output_gain(ALL_OUTPUTS, vol_output);
	m_oki[0]->set_output_gain(ALL_OUTPUTS, vol_output);
	m_oki[1]->set_output_gain(ALL_OUTPUTS, vol_output);
}

u8 captaven_state::captaven_soundcpu_status_r()
{
	// 7-------  sound cpu status (0 = busy)
	// -6543210  unknown

	return 0xff;
}

void dragngun_state::volume_w(u32 data)
{
	m_vol_main->ce_w(BIT(data, 2));
	m_vol_main->clk_w(BIT(data, 1));
	m_vol_main->di_w(BIT(data, 0));

	if (m_vol_gun.found())
	{
		m_vol_gun->ce_w(BIT(data, 2));
		m_vol_gun->clk_w(BIT(data, 1));
		m_vol_gun->di_w(BIT(data, 0));
	}
}

void dragngun_state::speaker_switch_w(u32 data)
{
	// TODO: This should switch the oki3 output between the gun speaker and the standard speakers
	m_gun_speaker_disabled = bool(BIT(data, 0));

	logerror("Gun speaker: %s\n", m_gun_speaker_disabled ? "Disabled" : "Enabled");
}

LC7535_VOLUME_CHANGED( dragngun_state::volume_main_changed )
{
	// TODO: Support loudness
	logerror("Main speaker volume: left = %d dB, right %d dB, loudness = %s\n", attenuation_left, attenuation_right, loudness ? "on" :"off");

	// convert to 0.0 - 1.0
	const float gain_l = m_vol_main->normalize(attenuation_left);
	const float gain_r = m_vol_main->normalize(attenuation_right);

	m_ym2151->set_output_gain(0, gain_l);
	m_ym2151->set_output_gain(1, gain_r); // left and right are always set to the same value
	m_oki[0]->set_output_gain(ALL_OUTPUTS, gain_l);
	m_oki[1]->set_output_gain(ALL_OUTPUTS, gain_l);

	if (m_oki[2].found() && m_gun_speaker_disabled)
		m_oki[2]->set_output_gain(ALL_OUTPUTS, gain_l);
}

LC7535_VOLUME_CHANGED( dragngun_state::volume_gun_changed )
{
	logerror("Gun speaker volume: left = %d dB, right %d dB, loudness = %s\n", attenuation_left, attenuation_right, loudness ? "on" :"off");

	if (m_oki[2].found() && !m_gun_speaker_disabled)
		m_oki[2]->set_output_gain(ALL_OUTPUTS, m_vol_gun->normalize(attenuation_left));
}

void tattass_state::tattass_sound_irq_w(int state)
{
	if (state)
	{
		u8 data = m_ioprot->soundlatch_r();
		// Swap bits 0 and 3 to correct for design error from BSMT schematic
		data = bitswap<8>(data, 7, 6, 5, 4, 0, 2, 1, 3);
		m_decobsmt->bsmt_comms_w(data);
	}
}

void deco32_state::sound_bankswitch_w(u8 data)
{
	m_oki[0]->set_rom_bank((data >> 0) & 1);
	m_oki[1]->set_rom_bank((data >> 1) & 1);
}

void dragngun_state::lockload_okibank_lo_w(u8 data)
{
	m_oki2_bank = (m_oki2_bank & 2) | ((data >> 1) & 1);
	logerror("Load OKI2 Bank Low bits: %02x, Current : %02x\n",(data >> 1) & 1, m_oki2_bank);
	m_oki[0]->set_rom_bank((data >> 0) & 1);
	m_oki[1]->set_rom_bank(m_oki2_bank);
}

void dragngun_state::lockload_okibank_hi_w(u8 data)
{
	m_oki2_bank = (m_oki2_bank & 1) | ((data & 1) << 1); // TODO : Actually value unverified
	logerror("Load OKI2 Bank Hi bits: %02x, Current : %02x\n",((data & 1) << 1), m_oki2_bank);
	m_oki[1]->set_rom_bank(m_oki2_bank);
}


//**************************************************************************
//  VIDEO
//**************************************************************************

void deco32_state::vblank_ack_w(u32 data)
{
	m_maincpu->set_input_line(ARM_IRQ_LINE, CLEAR_LINE);
}

template<int Chip>
u32 deco32_state::spriteram_r(offs_t offset)
{
	return m_spriteram16[Chip][offset] ^ 0xffff0000;
}

template<int Chip>
void deco32_state::spriteram_w(offs_t offset, u32 data, u32 mem_mask)
{
	data &= 0x0000ffff;
	mem_mask &= 0x0000ffff;
	COMBINE_DATA(&m_spriteram16[Chip][offset]);
}

template<int Chip>
void deco32_state::buffer_spriteram_w(u32 data)
{
	std::copy(&m_spriteram16[Chip][0], &m_spriteram16[Chip][0x2000/4], &m_spriteram16_buffered[Chip][0]);
}

// tattass tests these as 32-bit ram, even if only 16-bits are hooked up to the tilemap chip - does it mirror parts of the dword?
template <int TileMap> void deco32_state::pf_rowscroll_w(offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_pf_rowscroll32[TileMap][offset]); data &= 0x0000ffff; mem_mask &= 0x0000ffff; COMBINE_DATA(&m_pf_rowscroll[TileMap][offset]); }

u32 dragngun_state::unk_video_r()
{
	return machine().rand();
}

DECOSPR_PRIORITY_CB_MEMBER( captaven_state::captaven_pri_callback )
{
	if ((pri & 0x60) == 0x00)
	{
		return 0; // above everything
	}
	else if ((pri & 0x60) == 0x20)
	{
		return 0xfff0; // above the 2nd playfield
	}
	else if ((pri & 0x60) == 0x40)
	{
		return 0xfffc; // above the 1st playfield
	}
	else
	{
		return 0xfffe; // under everything
	}
}

DECOSPR_PRIORITY_CB_MEMBER( fghthist_state::fghthist_pri_callback )
{
	u32 mask = GFX_PMASK_8; // under the text layer
	if (extpri)
		mask |= GFX_PMASK_4; // under the uppermost playfield

	return mask;
}

void captaven_state::tile_callback(u32 &tile, u32 &colour, int layer, bool is_8x8)
{
	// Captain America operates this chip in 8bpp mode.
	// In 8bpp mode you appear to only get 1 layer, not 2, but you also
	// have an extra 2 tile bits, and 2 less colour bits.
	if (layer == 0)
	{
		tile |= ((colour & 0x3) << 12);
		colour >>= 2;
	}
}

DECO16IC_BANK_CB_MEMBER( captaven_state::bank_callback )
{
	return (bank & 0x20) << 9;
}

DECO16IC_BANK_CB_MEMBER( fghthist_state::bank_callback )
{
	bank = (bank & 0x10) | ((bank & 0x40) >> 1) | ((bank & 0x20) << 1);

	return bank << 8;
}

DECO16IC_BANK_CB_MEMBER( dragngun_state::bank_1_callback )
{
	return (bank & ~0xf) << 8;
}

DECO16IC_BANK_CB_MEMBER( dragngun_state::bank_2_callback )
{
	return (bank & ~0x1f) << 7;
}

DECO16IC_BANK_CB_MEMBER( nslasher_state::bank_callback )
{
	return (bank & ~0xf) << 8;
}

u16 nslasher_state::mix_callback(u16 p, u16 p2)
{
	return ((p & 0x70f) + (((p & 0x30) | (p2 & 0x0f)) << 4)) & 0x7ff;
}


//**************************************************************************
//  INPUTS
//**************************************************************************

// TODO: probably clears both player 1 and player 2
void dragngun_state::gun_irq_ack_w(u32 data)
{
	m_deco_irq->lightgun_irq_ack_w(data);
}

// TODO: improve this, Y axis not understood at all
u32 dragngun_state::lockload_gun_mirror_r(offs_t offset)
{
//logerror("%08x:Read gun %d\n",m_maincpu->pc(),offset);

	switch (offset)
	{
		case 0:
			return ((m_io_inputs->read() & 0x30) << 5) | (m_io_light_x[0]->read()) | 0xffff800;

		case 1:
			return ((m_io_inputs->read() & 0x3000) >> 3) | (m_io_light_x[1]->read()) | 0xffff800;
	}

	return ~0;
}

u32 dragngun_state::lightgun_r()
{
	// Ports 0-3 are read, but seem unused
	switch (m_lightgun_port)
	{
	case 4: return m_io_light_x[0]->read();
	case 5: return m_io_light_x[1]->read();
	case 6: return m_io_light_y[0]->read();
	case 7: return m_io_light_y[1]->read();
	}

//  logerror("Illegal lightgun port %d read \n",m_lightgun_port);
	return 0;
}

void dragngun_state::lightgun_w(offs_t offset, u32 data)
{
//  logerror("Lightgun port %d\n",m_lightgun_port);
	m_lightgun_port = offset;
}

INPUT_CHANGED_MEMBER( dragngun_state::lockload_gun_trigger )
{
	switch (param)
	{
	case 0: m_deco_irq->lightgun1_trigger_w(!newval); break;
	case 1: m_deco_irq->lightgun2_trigger_w(!newval); break;
	}
}


//**************************************************************************
//  EEPROM
//**************************************************************************

u8 deco32_state::eeprom_r()
{
	return 0xfe | m_eeprom->do_read();
}

void deco32_state::eeprom_w(u8 data)
{
	// 7-------  unknown
	// -6------  eeprom cs
	// --5-----  eeprom clk
	// ---4----  eeprom di
	// ----3---  unknown
	// -----2--  color fading effect mode (0 = without obj1, 1 = with obj1)
	// ------1-  bg2/3 joint mode (8bpp) (not used by fghthist?)
	// -------0  layer priority

	m_eeprom->clk_write(BIT(data, 5) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->di_write(BIT(data, 4));
	m_eeprom->cs_write(BIT(data, 6) ? ASSERT_LINE : CLEAR_LINE);

	pri_w(data & 0x07);
}

void dragngun_state::eeprom_w(u8 data)
{
	// 76543---  unknown
	// -----2--  eeprom cs
	// ------1-  eeprom clk
	// -------0  eeprom di

	m_eeprom->clk_write(BIT(data, 1) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->di_write(BIT(data, 0));
	m_eeprom->cs_write(BIT(data, 2) ? ASSERT_LINE : CLEAR_LINE);
}

void tattass_state::tattass_control_w(offs_t offset, u32 data, u32 mem_mask)
{
	// Eprom in low byte
	if (ACCESSING_BITS_0_7)
	{ // Byte write to low byte only (different from word writing including low byte)
		/*
		    The Tattoo Assassins eprom seems strange...  It's 1024 bytes in size, and 8 bit
		    in width, but offers a 'multiple read' mode where a bit stream can be read
		    starting at any byte boundary.

		    Multiple read mode:
		    Write 110aa000      [Read command, top two bits of address, 4 zeroes]
		    Write 00000000      [8 zeroes]
		    Write aaaaaaaa      [Bottom 8 bits of address]

		    Then bits are read back per clock, for as many bits as needed (NOT limited to byte
		    boundaries).

		    Write mode:
		    Write 000aa000      [Write command, top two bits of address, 4 zeroes]
		    Write 00000000      [8 zeroes]
		    Write aaaaaaaa      [Bottom 8 bits of address]
		    Write dddddddd      [8 data bits]

		*/
		if ((BIT(data, 6)) == 0)
		{
			if (m_buf_ptr)
			{
				logerror("Eprom reset (bit count %d): ", m_read_bit_count);
				for (int i = 0; i < m_buf_ptr; i++)
					logerror("%s", BIT(m_buffer, m_buf_ptr - 1 - i) ? "1" : "0");
				logerror("\n");

			}
			m_buf_ptr = 0;
			m_pending_command = 0;
			m_read_bit_count = 0;
		}

		// Eprom has been clocked
		if (m_last_clock == 0 && BIT(data, 5) && BIT(data, 6))
		{
			if (m_buf_ptr >= 32)
			{
				logerror("Eprom overflow!");
				m_buf_ptr = 0;
			}

			// Handle pending read
			if (m_pending_command == 1)
			{
				const int d = m_read_bit_count >> 3;
				const int m = 7 - (m_read_bit_count & 0x7);
				const int a = (m_byte_addr + d) & 0x3ff;
				const int b = m_eeprom->internal_read(a);

				m_tattass_eprom_bit = (b >> m) & 1;

				m_read_bit_count++;
				m_last_clock = BIT(data, 5);
				return;
			}

			// Handle pending write
			if (m_pending_command == 2)
			{
				m_buffer = (m_buffer << 1) | BIT(data, 4);

				m_buf_ptr++;
				if (m_buf_ptr == 32)
				{
					m_eeprom->internal_write(m_byte_addr, m_buffer & 0xff);
				}
				m_last_clock = BIT(data, 5);
				return;
			}

			m_buffer = (m_buffer << 1) | BIT(data, 4);

			m_buf_ptr++;
			if (m_buf_ptr == 24)
			{
				// Decode addr
				m_byte_addr = ((m_buffer & 0x180000) >> 11) | (m_buffer & 0x0000ff);

				// Check for read command
				if ((m_buffer & 0xc00000) == 0xc00000)
				{
					m_tattass_eprom_bit = (m_eeprom->internal_read(m_byte_addr) >> 7) & 1;
					m_read_bit_count = 1;
					m_pending_command = 1;
				}

				// Check for write command
				else if ((m_buffer & 0xc00000) == 0x0)
				{
					m_pending_command = 2;
				}
				else
				{
					logerror("Detected unknown eprom command\n");
				}
			}
		}
		else
		{
			if (!(BIT(data, 6)))
			{
				logerror("Cs set low\n");
				m_buf_ptr = 0;
			}
		}

		m_last_clock = BIT(data, 5);
	}

	// Volume in high byte
	if (ACCESSING_BITS_8_15)
	{
		//TODO:  volume attenuation == ((data >> 8) & 0xff);
		// TODO: is it really there?
	}

	// Playfield control - Only written in full word memory accesses
	pri_w(data & 0x7); // Bit 0 - layer priority toggle, Bit 1 - BG2/3 Joint mode (8bpp), Bit 2 - color fading effect mode (with/without OBJ1)?

	// Sound board reset control
	if (BIT(data, 7))
		m_decobsmt->bsmt_reset_line(CLEAR_LINE);
	else
		m_decobsmt->bsmt_reset_line(ASSERT_LINE);

	// bit 0x4 fade cancel?
	// bit 0x8 ??
	// Bit 0x100 ??
	//logerror("%08x: %08x data\n",data,mem_mask);
}

u16 tattass_state::port_b_tattass()
{
	return m_tattass_eprom_bit;
}


//**************************************************************************
//  MACHINE
//**************************************************************************

u32 fghthist_state::unk_status_r()
{
	// bit 3 needs to be 0
	return 0xfffffff7;
}

u16 nslasher_state::nslasher_debug_r()
{
	return 0xffff;
}

u32 captaven_state::_71_r()
{
	// Bit 0x80 goes high when sprite DMA is complete, and low
	// while it's in progress, we don't bother to emulate it
	return 0xffffffff;
}

void captaven_state::init_captaven()
{
	deco56_decrypt_gfx(machine(), "tiles1");
	deco56_decrypt_gfx(machine(), "tiles2");
}

extern void process_dvi_data(device_t *device,u8* dvi_data, int offset, int regionsize);

void dragngun_state::expand_sprite_data()
{
	u8 *rom = memregion("c355spr")->base();
	size_t len = memregion("c355spr")->bytes();

	for (int i = 0; i < len / 2; i++)
	{
		u8 temp = rom[i];
		rom[i + len / 2] = temp & 0x0f;
		rom[i] = (temp >> 4) & 0x0f;
	}
}



int dragngun_state::sprite_bank_callback(int sprite)
{
	// High bits of the sprite reference into the sprite control bits for banking
	switch (sprite & 0x3000) {
	default:
	case 0x0000: sprite = (sprite & 0xfff) | ((m_sprite_ctrl & 0x000f) << 12); break;
	case 0x1000: sprite = (sprite & 0xfff) | ((m_sprite_ctrl & 0x00f0) << 8); break;
	case 0x2000: sprite = (sprite & 0xfff) | ((m_sprite_ctrl & 0x0f00) << 4); break;
	case 0x3000: sprite = (sprite & 0xfff) | ((m_sprite_ctrl & 0xf000) << 0); break;
	}

	// Because of the unusual interleaved rom layout, we have to mangle the bank bits
	// even further to suit our gfx decode
	switch (sprite & 0xf000) {
	case 0x0000: sprite = 0xc000 | (sprite & 0xfff); break;
	case 0x1000: sprite = 0xd000 | (sprite & 0xfff); break;
	case 0x2000: sprite = 0xe000 | (sprite & 0xfff); break;
	case 0x3000: sprite = 0xf000 | (sprite & 0xfff); break;

	case 0xc000: sprite = 0x0000 | (sprite & 0xfff); break;
	case 0xd000: sprite = 0x1000 | (sprite & 0xfff); break;
	case 0xe000: sprite = 0x2000 | (sprite & 0xfff); break;
	case 0xf000: sprite = 0x3000 | (sprite & 0xfff); break;
	}

	return sprite;
}

void dragngun_state::dragngun_init_common()
{
	const u8 *SRC_RAM = memregion("tiles1")->base();
	u8 *DST_RAM = memregion("tiles2")->base();

	deco74_decrypt_gfx(machine(), "tiles1");
	deco74_decrypt_gfx(machine(), "tiles2");
	deco74_decrypt_gfx(machine(), "tiles3");

	std::copy(&SRC_RAM[0x00000], &SRC_RAM[0x10000], &DST_RAM[0x080000]);
	std::copy(&SRC_RAM[0x10000], &SRC_RAM[0x20000], &DST_RAM[0x110000]);

	expand_sprite_data();

#if 0
	{
		u8 *ROM = memregion("dvi")->base();

		auto fp = fopen("video.dvi", "w+b");
		if (fp)
		{
			fwrite(ROM, 0xc00000, 1, fp);
			fclose(fp);
		}
	}
#endif

	save_item(NAME(m_lightgun_port));

	// there are DVI headers at 0x000000, 0x580000, 0x800000, 0xB10000, 0xB80000
//  process_dvi_data(this,memregion("dvi")->base(),0x000000, 0x1000000);
//  process_dvi_data(this,memregion("dvi")->base(),0x580000, 0x1000000);
//  process_dvi_data(this,memregion("dvi")->base(),0x800000, 0x1000000);
//  process_dvi_data(this,memregion("dvi")->base(),0xB10000, 0x1000000);
//  process_dvi_data(this,memregion("dvi")->base(),0xB80000, 0x1000000);
}

void dragngun_state::init_dragngun()
{
	dragngun_init_common();

	u32 *ROM = (u32 *)memregion("maincpu")->base();
	ROM[0x01b32c/4] = 0xe1a00000; // bl $ee000: NOP test switch lock
}

void dragngun_state::init_dragngunj()
{
	dragngun_init_common();

	u32 *ROM = (u32 *)memregion("maincpu")->base();
	ROM[0x01a1b4/4] = 0xe1a00000; // bl $ee000: NOP test switch lock
}

void fghthist_state::init_fghthist()
{
	deco56_decrypt_gfx(machine(), "tiles1");
	deco74_decrypt_gfx(machine(), "tiles2");
}

void dragngun_state::init_lockload()
{
//  u32 *ROM = (u32 *)memregion("maincpu")->base();

	deco74_decrypt_gfx(machine(), "tiles1");
	deco74_decrypt_gfx(machine(), "tiles2");
	deco74_decrypt_gfx(machine(), "tiles3");

//  ROM[0x1fe3c0/4] = 0xe1a00000;//  NOP test switch lock
//  ROM[0x1fe3cc/4] = 0xe1a00000;//  NOP test switch lock
//  ROM[0x1fe40c/4] = 0xe1a00000;//  NOP test switch lock

	expand_sprite_data();

	save_item(NAME(m_oki2_bank));
}

void tattass_state::init_tattass()
{
	u8 *RAM = memregion("tiles1")->base();
	std::vector<u8> tmp(0x80000);

	// Reorder bitplanes to make decoding easier
	std::copy(&RAM[0x080000], &RAM[0x100000], tmp.begin());
	std::copy(&RAM[0x100000], &RAM[0x180000], &RAM[0x080000]);
	std::copy(tmp.begin(),    tmp.end(),      &RAM[0x100000]);

	RAM = memregion("tiles2")->base();
	std::copy(&RAM[0x080000], &RAM[0x100000], tmp.begin());
	std::copy(&RAM[0x100000], &RAM[0x180000], &RAM[0x080000]);
	std::copy(tmp.begin(),    tmp.end(),      &RAM[0x100000]);

	deco56_decrypt_gfx(machine(), "tiles1"); // 141
	deco56_decrypt_gfx(machine(), "tiles2"); // 141

	save_item(NAME(m_tattass_eprom_bit));
	save_item(NAME(m_last_clock));
	save_item(NAME(m_buffer));
	save_item(NAME(m_buf_ptr));
	save_item(NAME(m_pending_command));
	save_item(NAME(m_read_bit_count));
	save_item(NAME(m_byte_addr));
}

void nslasher_state::init_nslasher()
{
	u8 *RAM = memregion("tiles1")->base();
	std::vector<u8> tmp(0x80000);

	// Reorder bitplanes to make decoding easier
	std::copy(&RAM[0x080000], &RAM[0x100000], tmp.begin());
	std::copy(&RAM[0x100000], &RAM[0x180000], &RAM[0x080000]);
	std::copy(tmp.begin(),    tmp.end(),      &RAM[0x100000]);

	RAM = memregion("tiles2")->base();
	std::copy(&RAM[0x080000], &RAM[0x100000], tmp.begin());
	std::copy(&RAM[0x100000], &RAM[0x180000], &RAM[0x080000]);
	std::copy(tmp.begin(),    tmp.end(),      &RAM[0x100000]);

	deco56_decrypt_gfx(machine(), "tiles1"); // 141
	deco74_decrypt_gfx(machine(), "tiles2");

	deco156_decrypt(machine());

	// The board for Night Slashers is very close to the Fighter's History and
	// Tattoo Assassins boards, but has an encrypted ARM cpu.
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

/* Notes (2002.02.05) :

When the "Continue Coin" Dip Switch is set to "2 Start/1 Continue",
the "Coinage" Dip Switches have no effect.

START, BUTTON1 and COIN effects :

  2 players, common coin slots

STARTn starts a game for player n. It adds 100 energy points each time it is pressed
(provided there are still some credits, and energy is <= 900).

BUTTON1n selects the character for player n.

COIN1n adds credit(s)/coin(s).

  2 players, individual coin slots

NO STARTn button !

BUTTON1n starts a game for player n. It also adds 100 energy points for each credit
inserted for the player. It then selects the character for player n.

COIN1n adds 100 energy points (based on "Coinage") for player n when ingame if energy
<= 900, else adds credit(s)/coin(s) for player n.

  4 players, common coin slots

NO STARTn button !

BUTTON1n starts a game for player n. It gives 100 energy points. It then selects the
character for player n.

  4 players, individual coin slots

NO STARTn button !

BUTTON1n starts a game for player n. It also adds 100 energy points for each credit
inserted for the player. It then selects the character for player n.

COIN1n adds 100 energy points (based on "Coinage") for player n when ingame if energy
<= 900, else adds credit(s)/coin(s) for player n.

*/

static INPUT_PORTS_START( captaven )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )         PORT_DIPLOCATION("DSW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )         PORT_DIPLOCATION("DSW1:4,5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )    PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Continue Coin" )           PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, "1 Start/1 Continue" )
	PORT_DIPSETTING(    0x00, "2 Start/1 Continue" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )          PORT_DIPLOCATION("DSW2:1,2")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )     PORT_DIPLOCATION("DSW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, "Coin Slots" )              PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x10, "Common" )
	PORT_DIPSETTING(    0x00, "Individual" )
	PORT_DIPNAME( 0x20, 0x20, "Play Mode" )               PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, "2 Player" )
	PORT_DIPSETTING(    0x00, "4 Player" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )    PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	// This one isn't documented in the manual
	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC(0x01, IP_ACTIVE_LOW, "DSW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, IP_ACTIVE_LOW, "DSW3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, IP_ACTIVE_LOW, "DSW3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, IP_ACTIVE_LOW, "DSW3:4")
	PORT_DIPNAME(   0x10, 0x10, "Reset")                  PORT_DIPLOCATION("DSW3:5")
	PORT_DIPSETTING(      0x10, DEF_STR( Off ))
	PORT_DIPSETTING(      0x00, DEF_STR( On ))
	PORT_DIPNAME(   0x20, 0x20, DEF_STR( Free_Play ))     PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(      0x20, DEF_STR( Off ))
	PORT_DIPSETTING(      0x00, DEF_STR( On ))
	PORT_DIPNAME(   0x40, 0x40, "Stage Select")           PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(      0x40, DEF_STR( Off ))
	PORT_DIPSETTING(      0x00, DEF_STR( On ))
	PORT_DIPNAME(   0x80, 0x80, "Debug Mode")             PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(      0x80, DEF_STR( Off ))
	PORT_DIPSETTING(      0x00, DEF_STR( On ))
INPUT_PORTS_END

static INPUT_PORTS_START( fghthist )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )        PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 )        PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( dragngun )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("DSW")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED ) // Would be a dipswitch, but only 1 present on board
	PORT_DIPNAME( 0x0100, 0x0000, "Reset" ) // Behaves like Reset
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Stage Select" )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Debug Mode" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank)) // is this actually vblank?
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x0004, IP_ACTIVE_LOW )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) //check  //test BUTTON F2
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("LIGHT0_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_SENSITIVITY(20) PORT_KEYDELTA(25) PORT_PLAYER(1)

	PORT_START("LIGHT1_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_SENSITIVITY(20) PORT_KEYDELTA(25) PORT_PLAYER(2)

	PORT_START("LIGHT0_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(25) PORT_PLAYER(1)

	PORT_START("LIGHT1_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(25) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( lockload )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Fire") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dragngun_state::lockload_gun_trigger), 0)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Reload")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Fire") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dragngun_state::lockload_gun_trigger), 1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Reload")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("DSW")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED ) // Would be a dipswitch, but only 1 present on board
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, "Reset" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Debug Mode" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x0004, IP_ACTIVE_LOW )
	PORT_BIT( 0x00f8, IP_ACTIVE_LOW, IPT_UNUSED ) //check  //test BUTTON F2

	PORT_START("LIGHT0_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(20) PORT_KEYDELTA(25) PORT_PLAYER(1)

	PORT_START("LIGHT0_Y")
	PORT_BIT( 0xff, 49, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(20) PORT_KEYDELTA(25) PORT_PLAYER(1) PORT_MINMAX(16,82)

	PORT_START("LIGHT1_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(20) PORT_KEYDELTA(25) PORT_PLAYER(2)

	PORT_START("LIGHT1_Y")
	PORT_BIT( 0xff, 49, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(20) PORT_KEYDELTA(25) PORT_PLAYER(2) PORT_MINMAX(16,82)
INPUT_PORTS_END

static INPUT_PORTS_START( tattass )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )        PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 )        PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED ) // 'soundmask'
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( nslasher )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )        PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 )        PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED ) // 'soundmask'
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(3)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(3)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 )        PORT_PLAYER(3)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START3 )
INPUT_PORTS_END


//**************************************************************************
//  GFXDECODE LAYOUTS
//**************************************************************************

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8*2) },
	16*8    // every char takes 8 consecutive bytes
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ STEP8(16*8*2,1), STEP8(0,1) },
	{ STEP16(0,8*2) },
	64*8
};

static const gfx_layout tilelayout_8bpp =
{
	16,16,
	RGN_FRAC(1,4),
	8,
	{ RGN_FRAC(3,4)+8, RGN_FRAC(3,4)+0, RGN_FRAC(2,4)+8, RGN_FRAC(2,4)+0, RGN_FRAC(1,4)+8, RGN_FRAC(1,4)+0, 8, 0 },
	{ STEP8(16*8*2,1), STEP8(0,1) },
	{ STEP16(0,8*2) },
	64*8
};

static const gfx_layout tilelayout_5bpp =
{
	16,16,
	RGN_FRAC(1,1),
	5,
	{ 8*0, 8*1, 8*2, 8*3, 8*4 },
	{ STEP8(16*8*5,1), STEP8(0,1) },
	{ STEP16(0,8*5) },
	16*16*5
};

static GFXDECODE_START( gfx_captaven )
	GFXDECODE_ENTRY( "tiles1", 0, charlayout,      0, 128 ) // Characters 8x8
	GFXDECODE_ENTRY( "tiles1", 0, tilelayout,      0, 128 ) // Tiles 16x16
	GFXDECODE_ENTRY( "tiles2", 0, tilelayout_8bpp, 0,   8 ) // Tiles 16x16
GFXDECODE_END

static GFXDECODE_START( gfx_captaven_spr )
	GFXDECODE_ENTRY( "sprites1", 0, tilelayout,      0,  32 ) // Sprites 16x16
GFXDECODE_END

static GFXDECODE_START( gfx_fghthist )
	GFXDECODE_ENTRY( "tiles1", 0, charlayout,    0, 128 ) // Characters 8x8
	GFXDECODE_ENTRY( "tiles1", 0, tilelayout,    0, 128 ) // Tiles 16x16
	GFXDECODE_ENTRY( "tiles2", 0, tilelayout,    0, 128 ) // Tiles 16x16
GFXDECODE_END

static GFXDECODE_START( gfx_fghthist_spr )
	GFXDECODE_ENTRY( "sprites1", 0, tilelayout, 1024,  32 ) // Sprites 16x16
GFXDECODE_END

static GFXDECODE_START( gfx_dragngun )
	GFXDECODE_ENTRY( "tiles1", 0, charlayout,      0, 64 ) // Characters 8x8
	GFXDECODE_ENTRY( "tiles2", 0, tilelayout,      0, 64 ) // Tiles 16x16
	GFXDECODE_ENTRY( "tiles3", 0, tilelayout_8bpp, 0,  8 ) // Tiles 16x16
GFXDECODE_END

static GFXDECODE_START( gfx_nslasher )
	GFXDECODE_ENTRY( "tiles1", 0, charlayout,  0x800, 128 ) // Characters 8x8
	GFXDECODE_ENTRY( "tiles1", 0, tilelayout,      0, 128 ) // Tiles 16x16
	GFXDECODE_ENTRY( "tiles2", 0, tilelayout,      0, 128 ) // Tiles 16x16
GFXDECODE_END

static GFXDECODE_START( gfx_nslasher_spr1 )
	GFXDECODE_ENTRY( "sprites1", 0, tilelayout_5bpp, 0,  16 ) // Sprites 16x16
GFXDECODE_END

static GFXDECODE_START( gfx_nslasher_spr2 )
	GFXDECODE_ENTRY( "sprites2", 0, tilelayout,      0,  16 ) // Sprites 16x16
GFXDECODE_END


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void captaven_state::captaven(machine_config &config)
{
	// basic machine hardware
	ARM(config, m_maincpu, XTAL(28'000'000)/4); // verified on pcb (Data East 101 custom)*/
	m_maincpu->set_addrmap(AS_PROGRAM, &captaven_state::captaven_map);

	h6280_device &audiocpu(H6280(config, m_audiocpu, XTAL(32'220'000)/4/3));  // pin 10 is 32mhz/4, pin 14 is High so internal divisor is 3 (verified on pcb)
	audiocpu.set_addrmap(AS_PROGRAM, &captaven_state::h6280_sound_map);
	audiocpu.add_route(ALL_OUTPUTS, "speaker", 0, 0); // internal sound unused
	audiocpu.add_route(ALL_OUTPUTS, "speaker", 0, 1);

	INPUT_MERGER_ANY_HIGH(config, "irq_merger").output_handler().set_inputline(m_maincpu, ARM_IRQ_LINE);

	DECO_IRQ(config, m_deco_irq, 0);
	m_deco_irq->set_screen_tag(m_screen);
	m_deco_irq->raster2_irq_callback().set("irq_merger", FUNC(input_merger_any_high_device::in_w<0>));
	m_deco_irq->vblank_irq_callback().set("irq_merger", FUNC(input_merger_any_high_device::in_w<1>));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(28'000'000) / 4, 442, 0, 320, 274, 8, 248);
	m_screen->set_screen_update(FUNC(captaven_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_captaven);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_888, 2048);

	DECO16IC(config, m_deco_tilegen[0], 0);
	m_deco_tilegen[0]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf1_col_bank(0x20);
	m_deco_tilegen[0]->set_pf2_col_bank(0x30);
	m_deco_tilegen[0]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf12_8x8_bank(0);
	m_deco_tilegen[0]->set_pf12_16x16_bank(1);
	m_deco_tilegen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO16IC(config, m_deco_tilegen[1], 0);    // pf3 is in 8bpp mode, pf4 is not used
	m_deco_tilegen[1]->set_pf1_size(DECO_32x32);
	m_deco_tilegen[1]->set_pf2_size(DECO_32x32);
	m_deco_tilegen[1]->set_pf1_col_bank(0x04);
	m_deco_tilegen[1]->set_pf2_col_bank(0x00);
	m_deco_tilegen[1]->set_pf1_col_mask(0x03);
	m_deco_tilegen[1]->set_pf2_col_mask(0x00);
	m_deco_tilegen[1]->set_tile_callback(FUNC(captaven_state::tile_callback));
	m_deco_tilegen[1]->set_bank1_callback(FUNC(captaven_state::bank_callback));
	// no bank2 callback
	m_deco_tilegen[1]->set_pf12_8x8_bank(0);
	m_deco_tilegen[1]->set_pf12_16x16_bank(2);
	m_deco_tilegen[1]->set_gfxdecode_tag(m_gfxdecode);

	DECO_SPRITE(config, m_sprgen[0], 0, m_palette, gfx_captaven_spr);
	m_sprgen[0]->set_pri_callback(FUNC(captaven_state::captaven_pri_callback));
	m_sprgen[0]->set_alt_format(true);

	DECO146PROT(config, m_ioprot, 0);
	m_ioprot->port_a_cb().set_ioport("IN0");
	m_ioprot->port_b_cb().set_ioport("SYSTEM");
	m_ioprot->port_c_cb().set_ioport("IN1");
	m_ioprot->soundlatch_irq_cb().set_inputline(m_audiocpu, 0);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	YM2151(config, m_ym2151, XTAL(32'220'000)/9); // verified on pcb
	m_ym2151->irq_handler().set_inputline(m_audiocpu, 1);
	m_ym2151->port_write_handler().set(FUNC(deco32_state::sound_bankswitch_w));
	m_ym2151->add_route(0, "speaker", 0.42, 0);
	m_ym2151->add_route(1, "speaker", 0.42, 1);

	OKIM6295(config, m_oki[0], XTAL(32'220'000)/32, okim6295_device::PIN7_HIGH);  // verified on pcb; pin 7 is floating to 2.5V (left unconnected), so I presume High
	m_oki[0]->add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	m_oki[0]->add_route(ALL_OUTPUTS, "speaker", 1.0, 1);

	OKIM6295(config, m_oki[1], XTAL(32'220'000)/16, okim6295_device::PIN7_HIGH); // verified on pcb; pin 7 is floating to 2.5V (left unconnected), so I presume High
	m_oki[1]->add_route(ALL_OUTPUTS, "speaker", 0.35, 0);
	m_oki[1]->add_route(ALL_OUTPUTS, "speaker", 0.35, 1);
}

// DE-0380-2
void fghthist_state::fghthist(machine_config &config)
{
	ARM(config, m_maincpu, XTAL(28'000'000) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &fghthist_state::fghthist_map);
	m_maincpu->set_vblank_int("screen", FUNC(deco32_state::irq0_line_assert));

	h6280_device &audiocpu(H6280(config, m_audiocpu, XTAL(32'220'000) / 8));
	audiocpu.set_addrmap(AS_PROGRAM, &fghthist_state::h6280_sound_custom_latch_map);
	audiocpu.add_route(ALL_OUTPUTS, "speaker", 0, 0); // internal sound unused
	audiocpu.add_route(ALL_OUTPUTS, "speaker", 0, 1);

	EEPROM_93C46_16BIT(config, m_eeprom);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(28'000'000) / 4, 442, 0, 320, 274, 8, 248);
	m_screen->set_screen_update(FUNC(fghthist_state::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_fghthist);
	PALETTE(config, m_palette).set_entries(2048);

	DECO16IC(config, m_deco_tilegen[0], 0);
	m_deco_tilegen[0]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf1_col_bank(0x00);
	m_deco_tilegen[0]->set_pf2_col_bank(0x10);
	m_deco_tilegen[0]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[0]->set_bank1_callback(FUNC(fghthist_state::bank_callback));
	m_deco_tilegen[0]->set_bank2_callback(FUNC(fghthist_state::bank_callback));
	m_deco_tilegen[0]->set_pf12_8x8_bank(0);
	m_deco_tilegen[0]->set_pf12_16x16_bank(1);
	m_deco_tilegen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO16IC(config, m_deco_tilegen[1], 0);
	m_deco_tilegen[1]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf1_col_bank(0x20);
	m_deco_tilegen[1]->set_pf2_col_bank(0x30);
	m_deco_tilegen[1]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[1]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[1]->set_bank1_callback(FUNC(fghthist_state::bank_callback));
	m_deco_tilegen[1]->set_bank2_callback(FUNC(fghthist_state::bank_callback));
	m_deco_tilegen[1]->set_pf12_8x8_bank(0);
	m_deco_tilegen[1]->set_pf12_16x16_bank(2);
	m_deco_tilegen[1]->set_gfxdecode_tag(m_gfxdecode);

	DECO_SPRITE(config, m_sprgen[0], 0, m_palette, gfx_fghthist_spr);
	m_sprgen[0]->set_pri_callback(FUNC(fghthist_state::fghthist_pri_callback));

	DECO146PROT(config, m_ioprot, 0);
	m_ioprot->port_a_cb().set_ioport("IN0");
	m_ioprot->port_b_cb().set("eeprom", FUNC(eeprom_serial_93cxx_device::do_read)).lshift(0);
	m_ioprot->port_c_cb().set_ioport("IN1");
	m_ioprot->set_interface_scramble_interleave();
	m_ioprot->set_use_magic_read_address_xor(true);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, 0);

	YM2151(config, m_ym2151, 32220000/9);
	m_ym2151->irq_handler().set_inputline(m_audiocpu, 1);
	m_ym2151->port_write_handler().set(FUNC(deco32_state::sound_bankswitch_w));
	m_ym2151->add_route(0, "speaker", 0.42, 0);
	m_ym2151->add_route(1, "speaker", 0.42, 1);

	OKIM6295(config, m_oki[0], 32220000/32, okim6295_device::PIN7_HIGH);
	m_oki[0]->add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	m_oki[0]->add_route(ALL_OUTPUTS, "speaker", 1.0, 1);

	OKIM6295(config, m_oki[1], 32220000/16, okim6295_device::PIN7_HIGH);
	m_oki[1]->add_route(ALL_OUTPUTS, "speaker", 0.35, 0);
	m_oki[1]->add_route(ALL_OUTPUTS, "speaker", 0.35, 1);
}

// DE-0395-1
void fghthist_state::fghthsta(machine_config &config)
{
	fghthist(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &fghthist_state::fghthsta_memmap);
	m_audiocpu->set_addrmap(AS_PROGRAM, &fghthist_state::h6280_sound_map);

	config.device_remove("soundlatch");

	m_ioprot->soundlatch_irq_cb().set_inputline(m_audiocpu, 0);
}

// DE-0396-0
void fghthist_state::fghthistu(machine_config &config)
{
	fghthsta(config);

	Z80(config.replace(), m_audiocpu, XTAL(32'220'000) / 9);
	m_audiocpu->set_addrmap(AS_PROGRAM, &fghthist_state::z80_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &fghthist_state::z80_sound_io);

	INPUT_MERGER_ANY_HIGH(config, "sound_irq_merger").output_handler().set_inputline(m_audiocpu, INPUT_LINE_IRQ0);

	m_ioprot->soundlatch_irq_cb().set("sound_irq_merger", FUNC(input_merger_any_high_device::in_w<0>));

	m_ym2151->irq_handler().set("sound_irq_merger", FUNC(input_merger_any_high_device::in_w<1>));
	m_ym2151->reset_routes();
	m_ym2151->add_route(0, "speaker", 0.40, 0);
	m_ym2151->add_route(1, "speaker", 0.40, 1);
}

// DE-0359-2 + Bottom board DE-0360-4
void dragngun_state::dragngun(machine_config &config)
{
	// basic machine hardware
	ARM(config, m_maincpu, XTAL(28'000'000) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &dragngun_state::dragngun_map);

	h6280_device &audiocpu(H6280(config, m_audiocpu, 32220000/8));
	audiocpu.set_addrmap(AS_PROGRAM, &dragngun_state::h6280_sound_map);
	audiocpu.add_route(ALL_OUTPUTS, "speaker", 0, 0); // internal sound unused
	audiocpu.add_route(ALL_OUTPUTS, "speaker", 0, 1);

	INPUT_MERGER_ANY_HIGH(config, "irq_merger").output_handler().set_inputline("maincpu", ARM_IRQ_LINE);

	DECO_IRQ(config, m_deco_irq, 0);
	m_deco_irq->set_screen_tag(m_screen);
	m_deco_irq->raster2_irq_callback().set("irq_merger", FUNC(input_merger_any_high_device::in_w<0>));
	m_deco_irq->vblank_irq_callback().set("irq_merger", FUNC(input_merger_any_high_device::in_w<1>));

	EEPROM_93C46_16BIT(config, m_eeprom);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(28'000'000) / 4, 442, 0, 320, 274, 8, 248);
	m_screen->set_screen_update(FUNC(dragngun_state::screen_update));
	//m_screen->set_palette(m_palette);

	BUFFERED_SPRITERAM32(config, m_spriteram);

	DECO16IC(config, m_deco_tilegen[0], 0);
	m_deco_tilegen[0]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf1_col_bank(0x20);
	m_deco_tilegen[0]->set_pf2_col_bank(0x30);
	m_deco_tilegen[0]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[0]->set_bank1_callback(FUNC(dragngun_state::bank_1_callback));
	m_deco_tilegen[0]->set_bank2_callback(FUNC(dragngun_state::bank_1_callback));
	m_deco_tilegen[0]->set_pf12_8x8_bank(0);
	m_deco_tilegen[0]->set_pf12_16x16_bank(1);
	m_deco_tilegen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO16IC(config, m_deco_tilegen[1], 0);
	m_deco_tilegen[1]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf1_col_bank(0x04);
	m_deco_tilegen[1]->set_pf2_col_bank(0x04);
	m_deco_tilegen[1]->set_pf1_col_mask(0x03);
	m_deco_tilegen[1]->set_pf2_col_mask(0x03);
	m_deco_tilegen[1]->set_bank1_callback(FUNC(dragngun_state::bank_2_callback));
	// no bank2 callback
	m_deco_tilegen[1]->set_pf12_8x8_bank(0);
	m_deco_tilegen[1]->set_pf12_16x16_bank(2);
	m_deco_tilegen[1]->set_gfxdecode_tag(m_gfxdecode);

	namco_sprites(config);

	// I750, these aren't emulated
	//I82750PB(config, m_i82750pb, XTAL(25'000'000));
	//I82750DB(config, m_i82750db, XTAL(25'000'000));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_dragngun);
	PALETTE(config, m_palette).set_entries(2048);

	DECO146PROT(config, m_ioprot, 0);
	m_ioprot->port_a_cb().set_ioport("INPUTS");
	m_ioprot->port_b_cb().set_ioport("SYSTEM");
	m_ioprot->port_c_cb().set_ioport("DSW");
	m_ioprot->soundlatch_irq_cb().set_inputline(m_audiocpu, 0);
	m_ioprot->set_interface_scramble_reverse();

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	YM2151(config, m_ym2151, 32220000/9);
	m_ym2151->irq_handler().set_inputline(m_audiocpu, 1);
	m_ym2151->port_write_handler().set(FUNC(deco32_state::sound_bankswitch_w));
	m_ym2151->add_route(0, "speaker", 0.42, 0);
	m_ym2151->add_route(1, "speaker", 0.42, 1);

	OKIM6295(config, m_oki[0], 32220000/32, okim6295_device::PIN7_HIGH);
	m_oki[0]->add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	m_oki[0]->add_route(ALL_OUTPUTS, "speaker", 1.0, 1);

	OKIM6295(config, m_oki[1], 32220000/16, okim6295_device::PIN7_HIGH);
	m_oki[1]->add_route(ALL_OUTPUTS, "speaker", 0.35, 0);
	m_oki[1]->add_route(ALL_OUTPUTS, "speaker", 0.35, 1);

	SPEAKER(config, "gun_speaker").front_center();

	OKIM6295(config, m_oki[2], 32220000/32, okim6295_device::PIN7_HIGH);
	m_oki[2]->add_route(ALL_OUTPUTS, "gun_speaker", 1.0);

	LC7535(config, m_vol_main);
	m_vol_main->select().set_constant(1);
	m_vol_main->set_volume_callback(FUNC(dragngun_state::volume_main_changed));

	LC7535(config, m_vol_gun);
	m_vol_gun->select().set_constant(0);
	m_vol_gun->set_volume_callback(FUNC(dragngun_state::volume_gun_changed));
}

void dragngun_state::lockloadu(machine_config &config)
{
	dragngun(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &dragngun_state::lockloadu_map);
	m_audiocpu->set_addrmap(AS_PROGRAM, &dragngun_state::lockloadu_sound_map);

	m_deco_irq->lightgun_irq_callback().set("irq_merger", FUNC(input_merger_any_high_device::in_w<2>));

	m_deco_tilegen[1]->set_pf1_size(DECO_32x32);
	m_deco_tilegen[1]->set_pf2_size(DECO_32x32);    // lockload definitely wants pf34 half width..

	m_ym2151->port_write_handler().set(FUNC(dragngun_state::lockload_okibank_lo_w));
}

void dragngun_state::namco_sprites(machine_config &config)
{
	NAMCO_C355SPR(config, m_sprgenzoom, 0);
	m_sprgenzoom->set_tile_callback(namco_c355spr_device::c355_obj_code2tile_delegate(&dragngun_state::sprite_bank_callback, this));
	m_sprgenzoom->set_palette(m_palette);
	m_sprgenzoom->set_colors(32);
	m_sprgenzoom->set_granularity(16);
	m_sprgenzoom->set_read_spritetile(FUNC(dragngun_state::read_spritetile));
	m_sprgenzoom->set_read_spriteformat(FUNC(dragngun_state::read_spriteformat));
	m_sprgenzoom->set_read_spritetable(FUNC(dragngun_state::read_spritetable));
	m_sprgenzoom->set_read_spritelist(FUNC(dragngun_state::read_spritelist));
	m_sprgenzoom->set_read_cliptable(FUNC(dragngun_state::read_cliptable));
	m_sprgenzoom->set_priority_callback(FUNC(dragngun_state::sprite_priority_callback));
	m_sprgenzoom->set_device_allocates_spriteram_and_bitmaps(false);
}

// DE-0420-1 + Bottom board DE-0421-0
void dragngun_state::lockload(machine_config &config)
{
	// basic machine hardware
	ARM(config, m_maincpu, XTAL(28'000'000) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &dragngun_state::lockload_map);

	INPUT_MERGER_ANY_HIGH(config, "irq_merger").output_handler().set_inputline("maincpu", ARM_IRQ_LINE);

	Z80(config, m_audiocpu, 32220000/8);
	m_audiocpu->set_addrmap(AS_PROGRAM, &dragngun_state::lockload_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &dragngun_state::z80_sound_io);

	INPUT_MERGER_ANY_HIGH(config, "sound_irq_merger").output_handler().set_inputline("audiocpu", INPUT_LINE_IRQ0);

	config.set_maximum_quantum(attotime::from_hz(6000));  // to improve main<->audio comms

	DECO_IRQ(config, m_deco_irq, 0);
	m_deco_irq->set_screen_tag(m_screen);
	m_deco_irq->lightgun1_callback().set_ioport("LIGHT0_Y");
	m_deco_irq->lightgun2_callback().set_ioport("LIGHT1_Y");
	m_deco_irq->raster2_irq_callback().set("irq_merger", FUNC(input_merger_any_high_device::in_w<0>));
	m_deco_irq->vblank_irq_callback().set("irq_merger", FUNC(input_merger_any_high_device::in_w<1>));
	m_deco_irq->lightgun_irq_callback().set("irq_merger", FUNC(input_merger_any_high_device::in_w<2>));

	EEPROM_93C46_16BIT(config, m_eeprom);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(28'000'000) / 4, 442, 0, 320, 274, 8, 248);
	m_screen->set_screen_update(FUNC(dragngun_state::screen_update));

	BUFFERED_SPRITERAM32(config, m_spriteram);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_dragngun);
	PALETTE(config, m_palette).set_entries(2048);

	DECO16IC(config, m_deco_tilegen[0], 0);
	m_deco_tilegen[0]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf1_col_bank(0x20);
	m_deco_tilegen[0]->set_pf2_col_bank(0x30);
	m_deco_tilegen[0]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[0]->set_bank1_callback(FUNC(dragngun_state::bank_1_callback));
	m_deco_tilegen[0]->set_bank2_callback(FUNC(dragngun_state::bank_1_callback));
	m_deco_tilegen[0]->set_pf12_8x8_bank(0);
	m_deco_tilegen[0]->set_pf12_16x16_bank(1);
	m_deco_tilegen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO16IC(config, m_deco_tilegen[1], 0);
	m_deco_tilegen[1]->set_pf1_size(DECO_32x32);
	m_deco_tilegen[1]->set_pf2_size(DECO_32x32);    // lockload definitely wants pf34 half width..
	m_deco_tilegen[1]->set_pf1_col_bank(0x04);
	m_deco_tilegen[1]->set_pf2_col_bank(0x04);
	m_deco_tilegen[1]->set_pf1_col_mask(0x03);
	m_deco_tilegen[1]->set_pf2_col_mask(0x03);
	m_deco_tilegen[1]->set_bank1_callback(FUNC(dragngun_state::bank_2_callback));
	// no bank2 callback
	m_deco_tilegen[1]->set_pf12_8x8_bank(0);
	m_deco_tilegen[1]->set_pf12_16x16_bank(2);
	m_deco_tilegen[1]->set_gfxdecode_tag(m_gfxdecode);

	namco_sprites(config);

	DECO146PROT(config, m_ioprot, 0);
	m_ioprot->port_a_cb().set_ioport("INPUTS");
	m_ioprot->port_b_cb().set_ioport("SYSTEM");
	m_ioprot->port_c_cb().set_ioport("DSW");
	m_ioprot->soundlatch_irq_cb().set("sound_irq_merger", FUNC(input_merger_any_high_device::in_w<0>));
	m_ioprot->set_interface_scramble_reverse();

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	YM2151(config, m_ym2151, 32220000/9);
	m_ym2151->irq_handler().set("sound_irq_merger", FUNC(input_merger_any_high_device::in_w<1>));
	m_ym2151->port_write_handler().set(FUNC(dragngun_state::lockload_okibank_lo_w));
	m_ym2151->add_route(0, "speaker", 0.42, 0);
	m_ym2151->add_route(1, "speaker", 0.42, 1);

	OKIM6295(config, m_oki[0], 32220000/32, okim6295_device::PIN7_HIGH);
	m_oki[0]->add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	m_oki[0]->add_route(ALL_OUTPUTS, "speaker", 1.0, 1);

	OKIM6295(config, m_oki[1], 32220000/16, okim6295_device::PIN7_HIGH);
	m_oki[1]->add_route(ALL_OUTPUTS, "speaker", 0.35, 0);
	m_oki[1]->add_route(ALL_OUTPUTS, "speaker", 0.35, 1);

	LC7535(config, m_vol_main);
	m_vol_main->select().set_constant(1);
	m_vol_main->set_volume_callback(FUNC(dragngun_state::volume_main_changed));
}

void tattass_state::tattass(machine_config &config)
{
	// basic machine hardware
	ARM(config, m_maincpu, 28000000/4); // unconfirmed
	m_maincpu->set_addrmap(AS_PROGRAM, &tattass_state::tattass_map);
	m_maincpu->set_vblank_int("screen", FUNC(deco32_state::irq0_line_assert));

	config.set_maximum_quantum(attotime::from_hz(6000));  // to improve main<->audio comms

	EEPROM_93C76_8BIT(config, m_eeprom);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(28'000'000) / 4, 442, 0, 320, 274, 8, 248);
	m_screen->set_screen_update(FUNC(tattass_state::screen_update_tattass));

	DECO_ACE(config, m_deco_ace, 0);

	DECO16IC(config, m_deco_tilegen[0], 0);
	m_deco_tilegen[0]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf1_col_bank(0x00);
	m_deco_tilegen[0]->set_pf2_col_bank(0x10);
	m_deco_tilegen[0]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[0]->set_bank1_callback(FUNC(tattass_state::bank_callback));
	m_deco_tilegen[0]->set_bank2_callback(FUNC(tattass_state::bank_callback));
	m_deco_tilegen[0]->set_pf12_8x8_bank(0);
	m_deco_tilegen[0]->set_pf12_16x16_bank(1);
	m_deco_tilegen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO16IC(config, m_deco_tilegen[1], 0);
	m_deco_tilegen[1]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf1_col_bank(0x20);
	m_deco_tilegen[1]->set_pf2_col_bank(0x30);
	m_deco_tilegen[1]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[1]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[1]->set_bank1_callback(FUNC(tattass_state::bank_callback));
	m_deco_tilegen[1]->set_bank2_callback(FUNC(tattass_state::bank_callback));
	m_deco_tilegen[1]->set_mix_callback(FUNC(tattass_state::mix_callback));
	m_deco_tilegen[1]->set_pf12_8x8_bank(0);
	m_deco_tilegen[1]->set_pf12_16x16_bank(2);
	m_deco_tilegen[1]->set_gfxdecode_tag(m_gfxdecode);

	DECO_SPRITE(config, m_sprgen[0], 0, m_deco_ace, gfx_nslasher_spr1);
	DECO_SPRITE(config, m_sprgen[1], 0, m_deco_ace, gfx_nslasher_spr2);

	GFXDECODE(config, m_gfxdecode, m_deco_ace, gfx_nslasher);

	DECO104PROT(config, m_ioprot, 0);
	m_ioprot->port_a_cb().set_ioport("IN0");
	m_ioprot->port_b_cb().set(FUNC(tattass_state::port_b_tattass));
	m_ioprot->port_c_cb().set_ioport("IN1");
	m_ioprot->soundlatch_irq_cb().set(FUNC(tattass_state::tattass_sound_irq_w));
	m_ioprot->set_interface_scramble_interleave();

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	DECOBSMT(config, m_decobsmt, 0);
	m_decobsmt->add_route(0, "speaker", 1.0, 0);
	m_decobsmt->add_route(1, "speaker", 1.0, 1);
}

void nslasher_state::nslasher(machine_config &config)
{
	// basic machine hardware
	ARM(config, m_maincpu, XTAL(28'322'000) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &nslasher_state::nslasher_map);
	m_maincpu->set_vblank_int("screen", FUNC(deco32_state::irq0_line_assert));

	Z80(config, m_audiocpu, 32220000/9);
	m_audiocpu->set_addrmap(AS_PROGRAM, &nslasher_state::z80_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &nslasher_state::z80_sound_io);

	INPUT_MERGER_ANY_HIGH(config, "sound_irq_merger").output_handler().set_inputline("audiocpu", INPUT_LINE_IRQ0);

	config.set_maximum_quantum(attotime::from_hz(6000));  // to improve main<->audio comms

	EEPROM_93C46_16BIT(config, m_eeprom);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(28'322'000) / 4, 442, 0, 320, 274, 8, 248);
	m_screen->set_screen_update(FUNC(nslasher_state::screen_update_nslasher));

	DECO_ACE(config, m_deco_ace, 0);

	DECO16IC(config, m_deco_tilegen[0], 0);
	m_deco_tilegen[0]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf1_col_bank(0x00);
	m_deco_tilegen[0]->set_pf2_col_bank(0x10);
	m_deco_tilegen[0]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[0]->set_bank1_callback(FUNC(nslasher_state::bank_callback));
	m_deco_tilegen[0]->set_bank2_callback(FUNC(nslasher_state::bank_callback));
	m_deco_tilegen[0]->set_pf12_8x8_bank(0);
	m_deco_tilegen[0]->set_pf12_16x16_bank(1);
	m_deco_tilegen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO16IC(config, m_deco_tilegen[1], 0);
	m_deco_tilegen[1]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf1_col_bank(0x20);
	m_deco_tilegen[1]->set_pf2_col_bank(0x30);
	m_deco_tilegen[1]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[1]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[1]->set_bank1_callback(FUNC(nslasher_state::bank_callback));
	m_deco_tilegen[1]->set_bank2_callback(FUNC(nslasher_state::bank_callback));
	m_deco_tilegen[1]->set_mix_callback(FUNC(nslasher_state::mix_callback));
	m_deco_tilegen[1]->set_pf12_8x8_bank(0);
	m_deco_tilegen[1]->set_pf12_16x16_bank(2);
	m_deco_tilegen[1]->set_gfxdecode_tag(m_gfxdecode);

	DECO_SPRITE(config, m_sprgen[0], 0, m_deco_ace, gfx_nslasher_spr1);
	DECO_SPRITE(config, m_sprgen[1], 0, m_deco_ace, gfx_nslasher_spr2);

	GFXDECODE(config, m_gfxdecode, m_deco_ace, gfx_nslasher);

	DECO104PROT(config, m_ioprot, 0);
	m_ioprot->port_a_cb().set_ioport("IN0");
	m_ioprot->port_b_cb().set("eeprom", FUNC(eeprom_serial_93cxx_device::do_read)).lshift(0);
	m_ioprot->port_c_cb().set_ioport("IN1");
	m_ioprot->soundlatch_irq_cb().set("sound_irq_merger", FUNC(input_merger_any_high_device::in_w<0>));
	m_ioprot->set_interface_scramble_interleave();

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	YM2151(config, m_ym2151, 32220000/9);
	m_ym2151->irq_handler().set("sound_irq_merger", FUNC(input_merger_any_high_device::in_w<1>));
	m_ym2151->port_write_handler().set(FUNC(deco32_state::sound_bankswitch_w));
	m_ym2151->add_route(0, "speaker", 0.40, 0);
	m_ym2151->add_route(1, "speaker", 0.40, 1);

	OKIM6295(config, m_oki[0], 32220000/32, okim6295_device::PIN7_HIGH);
	m_oki[0]->add_route(ALL_OUTPUTS, "speaker", 0.80, 0);
	m_oki[0]->add_route(ALL_OUTPUTS, "speaker", 0.80, 1);

	OKIM6295(config, m_oki[1], 32220000/16, okim6295_device::PIN7_HIGH);
	m_oki[1]->add_route(ALL_OUTPUTS, "speaker", 0.10, 0);
	m_oki[1]->add_route(ALL_OUTPUTS, "speaker", 0.10, 1);
}

// the US release uses a H6280 instead of a Z80, much like Lock 'n' Loaded
void nslasher_state::nslasheru(machine_config &config)
{
	nslasher(config);
	config.device_remove("audiocpu");

	h6280_device &audiocpu(H6280(config, m_audiocpu, 32220000/8));
	audiocpu.set_addrmap(AS_PROGRAM, &nslasher_state::h6280_sound_map);
	audiocpu.add_route(ALL_OUTPUTS, "speaker", 0, 0); // internal sound unused
	audiocpu.add_route(ALL_OUTPUTS, "speaker", 0, 1);

	config.device_remove("sound_irq_merger");

	m_ym2151->irq_handler().set_inputline(m_audiocpu, 1);

	m_ioprot->soundlatch_irq_cb().set_inputline("audiocpu", 0);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

// used for 5bpp gfxs
#define ROM_LOAD40_BYTE(name,offset,length,hash)        ROMX_LOAD(name, offset, length, hash, ROM_SKIP(4))
#define ROM_LOAD40_WORD(name,offset,length,hash)        ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_SKIP(3))
#define ROM_LOAD40_WORD_SWAP(name,offset,length,hash)   ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(3))

ROM_START( captaven ) // DE-0351-x PCB (x=3 or 4)
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_BYTE( "hn_00-4.1e",  0x000000, 0x20000, CRC(147fb094) SHA1(6bd759c42f4b7f9e1c3f2d3ece0b3ec72de1a982) )
	ROM_LOAD32_BYTE( "hn_01-4.1h",  0x000001, 0x20000, CRC(11ecdb95) SHA1(832b56f05ae7e15e67fbdd321da8c1cc5e7629a0) )
	ROM_LOAD32_BYTE( "hn_02-4.1k",  0x000002, 0x20000, CRC(35d2681f) SHA1(3af7d959dc4842238a7f79926adf449cb7f0b2e9) )
	ROM_LOAD32_BYTE( "hn_03-4.1m",  0x000003, 0x20000, CRC(3b59ba05) SHA1(400e868e59977e56a4fa1870321c643983ba4162) )
	ROM_LOAD32_BYTE( "man-12.3e",   0x080000, 0x20000, CRC(d6261e98) SHA1(f3707be37ca926d9a341b9253a6bb2f3de0e25f6) )
	ROM_LOAD32_BYTE( "man-13.3h",   0x080001, 0x20000, CRC(40f0764d) SHA1(a6715c4a2accacf96f41c885579f314367c70dde) )
	ROM_LOAD32_BYTE( "man-14.3k",   0x080002, 0x20000, CRC(7cb9a4bd) SHA1(0af1a7bf0fcfa3cc14b38d92f19e97ad6e5541dd) )
	ROM_LOAD32_BYTE( "man-15.3m",   0x080003, 0x20000, CRC(c7854fe8) SHA1(ffa87dcda44fa0111de6ab317b77dd2bde015890) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "hj_08.17k",  0x00000,  0x10000,  CRC(361fbd16) SHA1(c4bbaf74e09c263044be74bb2c98caf6cfcab618) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "man-00.8a",  0x000000,  0x80000,  CRC(7855a607) SHA1(fa0be080515482281e5a12fe172eeb9a21af0820) ) // Encrypted tiles

	ROM_REGION( 0x500000, "tiles2", 0 )
	ROM_LOAD( "man-05.16a", 0x000000,  0x40000,  CRC(d44d1995) SHA1(e88e1a59a4b24ad058f21538f6e9bbba94a166b4) ) // Encrypted tiles
	ROM_CONTINUE(           0x140000,  0x40000 )
	ROM_CONTINUE(           0x280000,  0x40000 )
	ROM_CONTINUE(           0x3c0000,  0x40000 )
	ROM_LOAD( "man-04.14a", 0x040000,  0x40000,  CRC(541492a1) SHA1(2e0ab12555fc46001a815e76e3a0cd21f385f82a) ) // Encrypted tiles
	ROM_CONTINUE(           0x180000,  0x40000 )
	ROM_CONTINUE(           0x2c0000,  0x40000 )
	ROM_CONTINUE(           0x400000,  0x40000 )
	ROM_LOAD( "man-03.12a", 0x080000,  0x40000,  CRC(2d9c52b2) SHA1(8f6f4fe4f1a63099f889068991b34f9432b04fd7) ) // Encrypted tiles
	ROM_CONTINUE(           0x1c0000,  0x40000 )
	ROM_CONTINUE(           0x300000,  0x40000 )
	ROM_CONTINUE(           0x440000,  0x40000 )
	ROM_LOAD( "man-02.11a", 0x0c0000,  0x40000,  CRC(07674c05) SHA1(08b33721d7eba4a1ff2e282f77eeb56535a52923) ) // Encrypted tiles
	ROM_CONTINUE(           0x200000,  0x40000 )
	ROM_CONTINUE(           0x340000,  0x40000 )
	ROM_CONTINUE(           0x480000,  0x40000 )
	ROM_LOAD( "man-01.10a", 0x100000,  0x40000,  CRC(ae714ada) SHA1(b4d5806265d422c8b837489afe93731f584e4adf) ) // Encrypted tiles
	ROM_CONTINUE(           0x240000,  0x40000 )
	ROM_CONTINUE(           0x380000,  0x40000 )
	ROM_CONTINUE(           0x4c0000,  0x40000 )

	ROM_REGION( 0x400000, "sprites1", 0 ) // Sprites
	ROM_LOAD( "man-06.17a",  0x200000,  0x100000,  CRC(a9a64297) SHA1(e4cb441207b1907461c90c32c05a461c9bd30756) )
	ROM_LOAD( "man-07.18a",  0x000000,  0x100000,  CRC(b1db200c) SHA1(970bb15e90194dd285f53594aca5dec3405e75d5) )
	ROM_LOAD( "man-08.17c",  0x300000,  0x100000,  CRC(28e98e66) SHA1(55dbbd945eada81f7dcc874fdcb0b9e62ea453f0) )
	ROM_LOAD( "man-09.21c",  0x100000,  0x100000,  CRC(1921245d) SHA1(88d3b69a38c18c83d5658d057b95974f1bd371e6) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "man-10.14k", 0x000000,  0x80000,  CRC(0132c578) SHA1(70952f39508360bab51e1151531536f0ea6bbe06) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "man-11.16k", 0x000000,  0x80000,  CRC(0dc60a4c) SHA1(4d0daa6a0272852a37f341a0cdc48baee0ad9dd8) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "ts-00.4h",  0x0000, 0x0117, CRC(ebc2908e) SHA1(dca14a55abd1d88ee09092d4122614e55c3e7f53) )
	ROM_LOAD( "ts-01.5h",  0x0200, 0x0117, CRC(c776a980) SHA1(cd4bdcfb755f561fefa4c88fab5d6d2397332aa7) )
	ROM_LOAD( "ts-02.12l", 0x0400, 0x01bf, CRC(6f26528c) SHA1(2cf869b2a789a9b0646162a61c147bcbb13c9141) )
ROM_END

ROM_START( captavena ) // DE-0351-x PCB (x=3 or 4)
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_BYTE( "hn_00.e1",    0x000000, 0x20000, CRC(12dd0c71) SHA1(77bd0e5f1b105ec70de5e76cb9c8138f02a496be) )
	ROM_LOAD32_BYTE( "hn_01.h1",    0x000001, 0x20000, CRC(ac5ea492) SHA1(e08fa2b3e3a40cba6dcdf07049d67056d59ed72a) )
	ROM_LOAD32_BYTE( "hn_02.k1",    0x000002, 0x20000, CRC(0c5e13f6) SHA1(d9ebf503db7da8663f45fe307e432545651cfc13) )
	ROM_LOAD32_BYTE( "hn_03.l1",    0x000003, 0x20000, CRC(bc050740) SHA1(bee425e76734251444c9cfa9287e1eb9383625bc) )
	ROM_LOAD32_BYTE( "man-12.3e",   0x080000, 0x20000, CRC(d6261e98) SHA1(f3707be37ca926d9a341b9253a6bb2f3de0e25f6) )
	ROM_LOAD32_BYTE( "man-13.3h",   0x080001, 0x20000, CRC(40f0764d) SHA1(a6715c4a2accacf96f41c885579f314367c70dde) )
	ROM_LOAD32_BYTE( "man-14.3k",   0x080002, 0x20000, CRC(7cb9a4bd) SHA1(0af1a7bf0fcfa3cc14b38d92f19e97ad6e5541dd) )
	ROM_LOAD32_BYTE( "man-15.3m",   0x080003, 0x20000, CRC(c7854fe8) SHA1(ffa87dcda44fa0111de6ab317b77dd2bde015890) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "hj_08.17k",  0x00000,  0x10000,  CRC(361fbd16) SHA1(c4bbaf74e09c263044be74bb2c98caf6cfcab618) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "man-00.8a",  0x000000,  0x80000,  CRC(7855a607) SHA1(fa0be080515482281e5a12fe172eeb9a21af0820) ) // Encrypted tiles

	ROM_REGION( 0x500000, "tiles2", 0 )
	ROM_LOAD( "man-05.16a", 0x000000,  0x40000,  CRC(d44d1995) SHA1(e88e1a59a4b24ad058f21538f6e9bbba94a166b4) ) // Encrypted tiles
	ROM_CONTINUE(           0x140000,  0x40000 )
	ROM_CONTINUE(           0x280000,  0x40000 )
	ROM_CONTINUE(           0x3c0000,  0x40000 )
	ROM_LOAD( "man-04.14a", 0x040000,  0x40000,  CRC(541492a1) SHA1(2e0ab12555fc46001a815e76e3a0cd21f385f82a) ) // Encrypted tiles
	ROM_CONTINUE(           0x180000,  0x40000 )
	ROM_CONTINUE(           0x2c0000,  0x40000 )
	ROM_CONTINUE(           0x400000,  0x40000 )
	ROM_LOAD( "man-03.12a", 0x080000,  0x40000,  CRC(2d9c52b2) SHA1(8f6f4fe4f1a63099f889068991b34f9432b04fd7) ) // Encrypted tiles
	ROM_CONTINUE(           0x1c0000,  0x40000 )
	ROM_CONTINUE(           0x300000,  0x40000 )
	ROM_CONTINUE(           0x440000,  0x40000 )
	ROM_LOAD( "man-02.11a", 0x0c0000,  0x40000,  CRC(07674c05) SHA1(08b33721d7eba4a1ff2e282f77eeb56535a52923) ) // Encrypted tiles
	ROM_CONTINUE(           0x200000,  0x40000 )
	ROM_CONTINUE(           0x340000,  0x40000 )
	ROM_CONTINUE(           0x480000,  0x40000 )
	ROM_LOAD( "man-01.10a", 0x100000,  0x40000,  CRC(ae714ada) SHA1(b4d5806265d422c8b837489afe93731f584e4adf) ) // Encrypted tiles
	ROM_CONTINUE(           0x240000,  0x40000 )
	ROM_CONTINUE(           0x380000,  0x40000 )
	ROM_CONTINUE(           0x4c0000,  0x40000 )

	ROM_REGION( 0x400000, "sprites1", 0 ) // Sprites
	ROM_LOAD( "man-06.17a",  0x200000,  0x100000,  CRC(a9a64297) SHA1(e4cb441207b1907461c90c32c05a461c9bd30756) )
	ROM_LOAD( "man-07.18a",  0x000000,  0x100000,  CRC(b1db200c) SHA1(970bb15e90194dd285f53594aca5dec3405e75d5) )
	ROM_LOAD( "man-08.17c",  0x300000,  0x100000,  CRC(28e98e66) SHA1(55dbbd945eada81f7dcc874fdcb0b9e62ea453f0) )
	ROM_LOAD( "man-09.21c",  0x100000,  0x100000,  CRC(1921245d) SHA1(88d3b69a38c18c83d5658d057b95974f1bd371e6) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "man-10.14k", 0x000000,  0x80000,  CRC(0132c578) SHA1(70952f39508360bab51e1151531536f0ea6bbe06) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "man-11.16k", 0x000000,  0x80000,  CRC(0dc60a4c) SHA1(4d0daa6a0272852a37f341a0cdc48baee0ad9dd8) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "ts-00.4h",  0x0000, 0x0117, CRC(ebc2908e) SHA1(dca14a55abd1d88ee09092d4122614e55c3e7f53) )
	ROM_LOAD( "ts-01.5h",  0x0200, 0x0117, CRC(c776a980) SHA1(cd4bdcfb755f561fefa4c88fab5d6d2397332aa7) )
	ROM_LOAD( "ts-02.12l", 0x0400, 0x01bf, CRC(6f26528c) SHA1(2cf869b2a789a9b0646162a61c147bcbb13c9141) )
ROM_END

ROM_START( captavene ) // DE-0351-x PCB (x=3 or 4)
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_BYTE( "hg_00-4.1e",  0x000000, 0x20000, CRC(7008d43c) SHA1(a39143e13075ebc58ecc576391f04d2649675dfb) )
	ROM_LOAD32_BYTE( "hg_01-4.1h",  0x000001, 0x20000, CRC(53dc1042) SHA1(4547ad20e5bc3b9cedae53f73f1628fa3493aafa) )
	ROM_LOAD32_BYTE( "hg_02-4.1k",  0x000002, 0x20000, CRC(9e3f9ee2) SHA1(a56a68bdac58a337be48b346b6939c3f68da8e9d) )
	ROM_LOAD32_BYTE( "hg_03-4.1m",  0x000003, 0x20000, CRC(bc050740) SHA1(bee425e76734251444c9cfa9287e1eb9383625bc) )
	ROM_LOAD32_BYTE( "man-12.3e",   0x080000, 0x20000, CRC(d6261e98) SHA1(f3707be37ca926d9a341b9253a6bb2f3de0e25f6) )
	ROM_LOAD32_BYTE( "man-13.3h",   0x080001, 0x20000, CRC(40f0764d) SHA1(a6715c4a2accacf96f41c885579f314367c70dde) )
	ROM_LOAD32_BYTE( "man-14.3k",   0x080002, 0x20000, CRC(7cb9a4bd) SHA1(0af1a7bf0fcfa3cc14b38d92f19e97ad6e5541dd) )
	ROM_LOAD32_BYTE( "man-15.3m",   0x080003, 0x20000, CRC(c7854fe8) SHA1(ffa87dcda44fa0111de6ab317b77dd2bde015890) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "hj_08.17k",  0x00000,  0x10000,  CRC(361fbd16) SHA1(c4bbaf74e09c263044be74bb2c98caf6cfcab618) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "man-00.8a",  0x000000,  0x80000,  CRC(7855a607) SHA1(fa0be080515482281e5a12fe172eeb9a21af0820) ) // Encrypted tiles

	ROM_REGION( 0x500000, "tiles2", 0 )
	ROM_LOAD( "man-05.16a", 0x000000,  0x40000,  CRC(d44d1995) SHA1(e88e1a59a4b24ad058f21538f6e9bbba94a166b4) ) // Encrypted tiles
	ROM_CONTINUE(           0x140000,  0x40000 )
	ROM_CONTINUE(           0x280000,  0x40000 )
	ROM_CONTINUE(           0x3c0000,  0x40000 )
	ROM_LOAD( "man-04.14a", 0x040000,  0x40000,  CRC(541492a1) SHA1(2e0ab12555fc46001a815e76e3a0cd21f385f82a) ) // Encrypted tiles
	ROM_CONTINUE(           0x180000,  0x40000 )
	ROM_CONTINUE(           0x2c0000,  0x40000 )
	ROM_CONTINUE(           0x400000,  0x40000 )
	ROM_LOAD( "man-03.12a", 0x080000,  0x40000,  CRC(2d9c52b2) SHA1(8f6f4fe4f1a63099f889068991b34f9432b04fd7) ) // Encrypted tiles
	ROM_CONTINUE(           0x1c0000,  0x40000 )
	ROM_CONTINUE(           0x300000,  0x40000 )
	ROM_CONTINUE(           0x440000,  0x40000 )
	ROM_LOAD( "man-02.11a", 0x0c0000,  0x40000,  CRC(07674c05) SHA1(08b33721d7eba4a1ff2e282f77eeb56535a52923) ) // Encrypted tiles
	ROM_CONTINUE(           0x200000,  0x40000 )
	ROM_CONTINUE(           0x340000,  0x40000 )
	ROM_CONTINUE(           0x480000,  0x40000 )
	ROM_LOAD( "man-01.10a", 0x100000,  0x40000,  CRC(ae714ada) SHA1(b4d5806265d422c8b837489afe93731f584e4adf) ) // Encrypted tiles
	ROM_CONTINUE(           0x240000,  0x40000 )
	ROM_CONTINUE(           0x380000,  0x40000 )
	ROM_CONTINUE(           0x4c0000,  0x40000 )

	ROM_REGION( 0x400000, "sprites1", 0 ) // Sprites
	ROM_LOAD( "man-06.17a",  0x200000,  0x100000,  CRC(a9a64297) SHA1(e4cb441207b1907461c90c32c05a461c9bd30756) )
	ROM_LOAD( "man-07.18a",  0x000000,  0x100000,  CRC(b1db200c) SHA1(970bb15e90194dd285f53594aca5dec3405e75d5) )
	ROM_LOAD( "man-08.17c",  0x300000,  0x100000,  CRC(28e98e66) SHA1(55dbbd945eada81f7dcc874fdcb0b9e62ea453f0) )
	ROM_LOAD( "man-09.21c",  0x100000,  0x100000,  CRC(1921245d) SHA1(88d3b69a38c18c83d5658d057b95974f1bd371e6) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "man-10.14k", 0x000000,  0x80000,  CRC(0132c578) SHA1(70952f39508360bab51e1151531536f0ea6bbe06) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "man-11.16k", 0x000000,  0x80000,  CRC(0dc60a4c) SHA1(4d0daa6a0272852a37f341a0cdc48baee0ad9dd8) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "ts-00.4h",  0x0000, 0x0117, CRC(ebc2908e) SHA1(dca14a55abd1d88ee09092d4122614e55c3e7f53) )
	ROM_LOAD( "ts-01.5h",  0x0200, 0x0117, CRC(c776a980) SHA1(cd4bdcfb755f561fefa4c88fab5d6d2397332aa7) )
	ROM_LOAD( "ts-02.12l", 0x0400, 0x01bf, CRC(6f26528c) SHA1(2cf869b2a789a9b0646162a61c147bcbb13c9141) )
	ROM_LOAD( "pal16r8b.14c", 0x0600, 0x0104, NO_DUMP ) // PAL is read protected
ROM_END

ROM_START( captavenu ) // DE-0351-x PCB (x=3 or 4)
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_BYTE( "hh_00-19.1e", 0x000000, 0x20000, CRC(08b870e0) SHA1(44c837e3c5dfc9764d89b0ebb3e9b7a40fe4d76f) )
	ROM_LOAD32_BYTE( "hh_01-19.1h", 0x000001, 0x20000, CRC(0dc0feca) SHA1(cb1c97aac59dabcf6c37bc1562cf2f62bca951f1) )
	ROM_LOAD32_BYTE( "hh_02-19.1k", 0x000002, 0x20000, CRC(26ef94c0) SHA1(985fae62a6a7ca7e1e64dba2db053b08206c65e7) )
	ROM_LOAD32_BYTE( "hn_03-4.1m",  0x000003, 0x20000, CRC(3b59ba05) SHA1(400e868e59977e56a4fa1870321c643983ba4162) )
	ROM_LOAD32_BYTE( "man-12.3e",   0x080000, 0x20000, CRC(d6261e98) SHA1(f3707be37ca926d9a341b9253a6bb2f3de0e25f6) )
	ROM_LOAD32_BYTE( "man-13.3h",   0x080001, 0x20000, CRC(40f0764d) SHA1(a6715c4a2accacf96f41c885579f314367c70dde) )
	ROM_LOAD32_BYTE( "man-14.3k",   0x080002, 0x20000, CRC(7cb9a4bd) SHA1(0af1a7bf0fcfa3cc14b38d92f19e97ad6e5541dd) )
	ROM_LOAD32_BYTE( "man-15.3m",   0x080003, 0x20000, CRC(c7854fe8) SHA1(ffa87dcda44fa0111de6ab317b77dd2bde015890) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "hj_08.17k",  0x00000,  0x10000,  CRC(361fbd16) SHA1(c4bbaf74e09c263044be74bb2c98caf6cfcab618) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "man-00.8a",  0x000000,  0x80000,  CRC(7855a607) SHA1(fa0be080515482281e5a12fe172eeb9a21af0820) ) // Encrypted tiles

	ROM_REGION( 0x500000, "tiles2", 0 )
	ROM_LOAD( "man-05.16a", 0x000000,  0x40000,  CRC(d44d1995) SHA1(e88e1a59a4b24ad058f21538f6e9bbba94a166b4) ) // Encrypted tiles
	ROM_CONTINUE(           0x140000,  0x40000 )
	ROM_CONTINUE(           0x280000,  0x40000 )
	ROM_CONTINUE(           0x3c0000,  0x40000 )
	ROM_LOAD( "man-04.14a", 0x040000,  0x40000,  CRC(541492a1) SHA1(2e0ab12555fc46001a815e76e3a0cd21f385f82a) ) // Encrypted tiles
	ROM_CONTINUE(           0x180000,  0x40000 )
	ROM_CONTINUE(           0x2c0000,  0x40000 )
	ROM_CONTINUE(           0x400000,  0x40000 )
	ROM_LOAD( "man-03.12a", 0x080000,  0x40000,  CRC(2d9c52b2) SHA1(8f6f4fe4f1a63099f889068991b34f9432b04fd7) ) // Encrypted tiles
	ROM_CONTINUE(           0x1c0000,  0x40000 )
	ROM_CONTINUE(           0x300000,  0x40000 )
	ROM_CONTINUE(           0x440000,  0x40000 )
	ROM_LOAD( "man-02.11a", 0x0c0000,  0x40000,  CRC(07674c05) SHA1(08b33721d7eba4a1ff2e282f77eeb56535a52923) ) // Encrypted tiles
	ROM_CONTINUE(           0x200000,  0x40000 )
	ROM_CONTINUE(           0x340000,  0x40000 )
	ROM_CONTINUE(           0x480000,  0x40000 )
	ROM_LOAD( "man-01.10a", 0x100000,  0x40000,  CRC(ae714ada) SHA1(b4d5806265d422c8b837489afe93731f584e4adf) ) // Encrypted tiles
	ROM_CONTINUE(           0x240000,  0x40000 )
	ROM_CONTINUE(           0x380000,  0x40000 )
	ROM_CONTINUE(           0x4c0000,  0x40000 )

	ROM_REGION( 0x400000, "sprites1", 0 ) // Sprites
	ROM_LOAD( "man-06.17a",  0x200000,  0x100000,  CRC(a9a64297) SHA1(e4cb441207b1907461c90c32c05a461c9bd30756) )
	ROM_LOAD( "man-07.18a",  0x000000,  0x100000,  CRC(b1db200c) SHA1(970bb15e90194dd285f53594aca5dec3405e75d5) )
	ROM_LOAD( "man-08.17c",  0x300000,  0x100000,  CRC(28e98e66) SHA1(55dbbd945eada81f7dcc874fdcb0b9e62ea453f0) )
	ROM_LOAD( "man-09.21c",  0x100000,  0x100000,  CRC(1921245d) SHA1(88d3b69a38c18c83d5658d057b95974f1bd371e6) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "man-10.14k", 0x000000,  0x80000,  CRC(0132c578) SHA1(70952f39508360bab51e1151531536f0ea6bbe06) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "man-11.16k", 0x000000,  0x80000,  CRC(0dc60a4c) SHA1(4d0daa6a0272852a37f341a0cdc48baee0ad9dd8) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "ts-00.4h",  0x0000, 0x0117, CRC(ebc2908e) SHA1(dca14a55abd1d88ee09092d4122614e55c3e7f53) )
	ROM_LOAD( "ts-01.5h",  0x0200, 0x0117, CRC(c776a980) SHA1(cd4bdcfb755f561fefa4c88fab5d6d2397332aa7) )
	ROM_LOAD( "ts-02.12l", 0x0400, 0x01bf, CRC(6f26528c) SHA1(2cf869b2a789a9b0646162a61c147bcbb13c9141) )
ROM_END

ROM_START( captavenuu ) // DE-0351-x PCB (x=3 or 4)
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_BYTE( "hh-00.1e",    0x000000, 0x20000, CRC(c34da654) SHA1(a1988a6a45991db6dee10b484049f6703b4671c9) )
	ROM_LOAD32_BYTE( "hh-01.1h",    0x000001, 0x20000, CRC(55abe63f) SHA1(98772eff3ebb5a4f243c7a77d398eb142d1505cb) )
	ROM_LOAD32_BYTE( "hh-02.1k",    0x000002, 0x20000, CRC(6096a9fb) SHA1(aa81189b9c185dc5d59f888afcb17a1e4935c241) )
	ROM_LOAD32_BYTE( "hh-03.1m",    0x000003, 0x20000, CRC(93631ded) SHA1(b4c8a6cbf586f895e637c0ed38f0842327624423) )
	ROM_LOAD32_BYTE( "man-12.3e",   0x080000, 0x20000, CRC(d6261e98) SHA1(f3707be37ca926d9a341b9253a6bb2f3de0e25f6) )
	ROM_LOAD32_BYTE( "man-13.3h",   0x080001, 0x20000, CRC(40f0764d) SHA1(a6715c4a2accacf96f41c885579f314367c70dde) )
	ROM_LOAD32_BYTE( "man-14.3k",   0x080002, 0x20000, CRC(7cb9a4bd) SHA1(0af1a7bf0fcfa3cc14b38d92f19e97ad6e5541dd) )
	ROM_LOAD32_BYTE( "man-15.3m",   0x080003, 0x20000, CRC(c7854fe8) SHA1(ffa87dcda44fa0111de6ab317b77dd2bde015890) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "hj_08.17k",  0x00000,  0x10000,  CRC(361fbd16) SHA1(c4bbaf74e09c263044be74bb2c98caf6cfcab618) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "man-00.8a",  0x000000,  0x80000,  CRC(7855a607) SHA1(fa0be080515482281e5a12fe172eeb9a21af0820) ) // Encrypted tiles

	ROM_REGION( 0x500000, "tiles2", 0 )
	ROM_LOAD( "man-05.16a", 0x000000,  0x40000,  CRC(d44d1995) SHA1(e88e1a59a4b24ad058f21538f6e9bbba94a166b4) ) // Encrypted tiles
	ROM_CONTINUE(           0x140000,  0x40000 )
	ROM_CONTINUE(           0x280000,  0x40000 )
	ROM_CONTINUE(           0x3c0000,  0x40000 )
	ROM_LOAD( "man-04.14a", 0x040000,  0x40000,  CRC(541492a1) SHA1(2e0ab12555fc46001a815e76e3a0cd21f385f82a) ) // Encrypted tiles
	ROM_CONTINUE(           0x180000,  0x40000 )
	ROM_CONTINUE(           0x2c0000,  0x40000 )
	ROM_CONTINUE(           0x400000,  0x40000 )
	ROM_LOAD( "man-03.12a", 0x080000,  0x40000,  CRC(2d9c52b2) SHA1(8f6f4fe4f1a63099f889068991b34f9432b04fd7) ) // Encrypted tiles
	ROM_CONTINUE(           0x1c0000,  0x40000 )
	ROM_CONTINUE(           0x300000,  0x40000 )
	ROM_CONTINUE(           0x440000,  0x40000 )
	ROM_LOAD( "man-02.11a", 0x0c0000,  0x40000,  CRC(07674c05) SHA1(08b33721d7eba4a1ff2e282f77eeb56535a52923) ) // Encrypted tiles
	ROM_CONTINUE(           0x200000,  0x40000 )
	ROM_CONTINUE(           0x340000,  0x40000 )
	ROM_CONTINUE(           0x480000,  0x40000 )
	ROM_LOAD( "man-01.10a", 0x100000,  0x40000,  CRC(ae714ada) SHA1(b4d5806265d422c8b837489afe93731f584e4adf) ) // Encrypted tiles
	ROM_CONTINUE(           0x240000,  0x40000 )
	ROM_CONTINUE(           0x380000,  0x40000 )
	ROM_CONTINUE(           0x4c0000,  0x40000 )

	ROM_REGION( 0x400000, "sprites1", 0 ) // Sprites
	ROM_LOAD( "man-06.17a",  0x200000,  0x100000,  CRC(a9a64297) SHA1(e4cb441207b1907461c90c32c05a461c9bd30756) )
	ROM_LOAD( "man-07.18a",  0x000000,  0x100000,  CRC(b1db200c) SHA1(970bb15e90194dd285f53594aca5dec3405e75d5) )
	ROM_LOAD( "man-08.17c",  0x300000,  0x100000,  CRC(28e98e66) SHA1(55dbbd945eada81f7dcc874fdcb0b9e62ea453f0) )
	ROM_LOAD( "man-09.21c",  0x100000,  0x100000,  CRC(1921245d) SHA1(88d3b69a38c18c83d5658d057b95974f1bd371e6) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "man-10.14k", 0x000000,  0x80000,  CRC(0132c578) SHA1(70952f39508360bab51e1151531536f0ea6bbe06) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "man-11.16k", 0x000000,  0x80000,  CRC(0dc60a4c) SHA1(4d0daa6a0272852a37f341a0cdc48baee0ad9dd8) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "ts-00.4h",  0x0000, 0x0117, CRC(ebc2908e) SHA1(dca14a55abd1d88ee09092d4122614e55c3e7f53) )
	ROM_LOAD( "ts-01.5h",  0x0200, 0x0117, CRC(c776a980) SHA1(cd4bdcfb755f561fefa4c88fab5d6d2397332aa7) )
	ROM_LOAD( "ts-02.12l", 0x0400, 0x01bf, CRC(6f26528c) SHA1(2cf869b2a789a9b0646162a61c147bcbb13c9141) )
ROM_END

ROM_START( captavenua ) // DE-0351-x PCB (x=3 or 4)
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_BYTE( "hh_00-4.2e",   0x000000, 0x20000, CRC(0e1acc05) SHA1(7eb6206efad233f9f4ee51102f9fe6b58f0719ea) )
	ROM_LOAD32_BYTE( "hh_01-4.2h",   0x000001, 0x20000, CRC(4ff0351d) SHA1(15fc2662ff0d32986c4d4d074b985ad853da34e1) )
	ROM_LOAD32_BYTE( "hh_02-4.2k",   0x000002, 0x20000, CRC(e84c0665) SHA1(d846f04315af49abeca00314b3d23e1d8c638dcd) )
	ROM_LOAD32_BYTE( "hh_03-4.2m",   0x000003, 0x20000, CRC(bc050740) SHA1(bee425e76734251444c9cfa9287e1eb9383625bc) )
	ROM_LOAD32_BYTE( "man-12.3e",   0x080000, 0x20000, CRC(d6261e98) SHA1(f3707be37ca926d9a341b9253a6bb2f3de0e25f6) )
	ROM_LOAD32_BYTE( "man-13.3h",   0x080001, 0x20000, CRC(40f0764d) SHA1(a6715c4a2accacf96f41c885579f314367c70dde) )
	ROM_LOAD32_BYTE( "man-14.3k",   0x080002, 0x20000, CRC(7cb9a4bd) SHA1(0af1a7bf0fcfa3cc14b38d92f19e97ad6e5541dd) )
	ROM_LOAD32_BYTE( "man-15.3m",   0x080003, 0x20000, CRC(c7854fe8) SHA1(ffa87dcda44fa0111de6ab317b77dd2bde015890) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "hj_08.17k",  0x00000,  0x10000,  CRC(361fbd16) SHA1(c4bbaf74e09c263044be74bb2c98caf6cfcab618) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "man-00.8a",  0x000000,  0x80000,  CRC(7855a607) SHA1(fa0be080515482281e5a12fe172eeb9a21af0820) ) // Encrypted tiles

	ROM_REGION( 0x500000, "tiles2", 0 )
	ROM_LOAD( "man-05.16a", 0x000000,  0x40000,  CRC(d44d1995) SHA1(e88e1a59a4b24ad058f21538f6e9bbba94a166b4) ) // Encrypted tiles
	ROM_CONTINUE(           0x140000,  0x40000 )
	ROM_CONTINUE(           0x280000,  0x40000 )
	ROM_CONTINUE(           0x3c0000,  0x40000 )
	ROM_LOAD( "man-04.14a", 0x040000,  0x40000,  CRC(541492a1) SHA1(2e0ab12555fc46001a815e76e3a0cd21f385f82a) ) // Encrypted tiles
	ROM_CONTINUE(           0x180000,  0x40000 )
	ROM_CONTINUE(           0x2c0000,  0x40000 )
	ROM_CONTINUE(           0x400000,  0x40000 )
	ROM_LOAD( "man-03.12a", 0x080000,  0x40000,  CRC(2d9c52b2) SHA1(8f6f4fe4f1a63099f889068991b34f9432b04fd7) ) // Encrypted tiles
	ROM_CONTINUE(           0x1c0000,  0x40000 )
	ROM_CONTINUE(           0x300000,  0x40000 )
	ROM_CONTINUE(           0x440000,  0x40000 )
	ROM_LOAD( "man-02.11a", 0x0c0000,  0x40000,  CRC(07674c05) SHA1(08b33721d7eba4a1ff2e282f77eeb56535a52923) ) // Encrypted tiles
	ROM_CONTINUE(           0x200000,  0x40000 )
	ROM_CONTINUE(           0x340000,  0x40000 )
	ROM_CONTINUE(           0x480000,  0x40000 )
	ROM_LOAD( "man-01.10a", 0x100000,  0x40000,  CRC(ae714ada) SHA1(b4d5806265d422c8b837489afe93731f584e4adf) ) // Encrypted tiles
	ROM_CONTINUE(           0x240000,  0x40000 )
	ROM_CONTINUE(           0x380000,  0x40000 )
	ROM_CONTINUE(           0x4c0000,  0x40000 )

	ROM_REGION( 0x400000, "sprites1", 0 ) // Sprites
	ROM_LOAD( "man-06.17a",  0x200000,  0x100000,  CRC(a9a64297) SHA1(e4cb441207b1907461c90c32c05a461c9bd30756) )
	ROM_LOAD( "man-07.18a",  0x000000,  0x100000,  CRC(b1db200c) SHA1(970bb15e90194dd285f53594aca5dec3405e75d5) )
	ROM_LOAD( "man-08.17c",  0x300000,  0x100000,  CRC(28e98e66) SHA1(55dbbd945eada81f7dcc874fdcb0b9e62ea453f0) )
	ROM_LOAD( "man-09.21c",  0x100000,  0x100000,  CRC(1921245d) SHA1(88d3b69a38c18c83d5658d057b95974f1bd371e6) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "man-10.14k", 0x000000,  0x80000,  CRC(0132c578) SHA1(70952f39508360bab51e1151531536f0ea6bbe06) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "man-11.16k", 0x000000,  0x80000,  CRC(0dc60a4c) SHA1(4d0daa6a0272852a37f341a0cdc48baee0ad9dd8) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "ts-00.4h",  0x0000, 0x0117, CRC(ebc2908e) SHA1(dca14a55abd1d88ee09092d4122614e55c3e7f53) )
	ROM_LOAD( "ts-01.5h",  0x0200, 0x0117, CRC(c776a980) SHA1(cd4bdcfb755f561fefa4c88fab5d6d2397332aa7) )
	ROM_LOAD( "ts-02.12l", 0x0400, 0x01bf, CRC(6f26528c) SHA1(2cf869b2a789a9b0646162a61c147bcbb13c9141) )
ROM_END

ROM_START( captavenj ) // DE-0351-x PCB (x=3 or 4)
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_BYTE( "hj_00-2.1e",  0x000000, 0x20000, CRC(10b1faaf) SHA1(9d76885200a846b4751c8d44ff591e2aff7c4148) )
	ROM_LOAD32_BYTE( "hj_01-2.1h",  0x000001, 0x20000, CRC(62c59f27) SHA1(20bbb7f3ff63a8c795686c1d56d51e90305daa77) )
	ROM_LOAD32_BYTE( "hj_02-2.1k",  0x000002, 0x20000, CRC(ce946cad) SHA1(9f1e92f5149e8a8d0236d5a7ba854ee100fd8488) )
	ROM_LOAD32_BYTE( "hj_03-2.1m",  0x000003, 0x20000, CRC(140cf9ce) SHA1(e2260ca4cea2fd7b64b8a78fd5444a7628bdafbb) )
	ROM_LOAD32_BYTE( "man-12.3e",   0x080000, 0x20000, CRC(d6261e98) SHA1(f3707be37ca926d9a341b9253a6bb2f3de0e25f6) )
	ROM_LOAD32_BYTE( "man-13.3h",   0x080001, 0x20000, CRC(40f0764d) SHA1(a6715c4a2accacf96f41c885579f314367c70dde) )
	ROM_LOAD32_BYTE( "man-14.3k",   0x080002, 0x20000, CRC(7cb9a4bd) SHA1(0af1a7bf0fcfa3cc14b38d92f19e97ad6e5541dd) )
	ROM_LOAD32_BYTE( "man-15.3m",   0x080003, 0x20000, CRC(c7854fe8) SHA1(ffa87dcda44fa0111de6ab317b77dd2bde015890) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "hj_08.17k",  0x00000,  0x10000,  CRC(361fbd16) SHA1(c4bbaf74e09c263044be74bb2c98caf6cfcab618) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "man-00.8a",  0x000000,  0x80000,  CRC(7855a607) SHA1(fa0be080515482281e5a12fe172eeb9a21af0820) ) // Encrypted tiles

	ROM_REGION( 0x500000, "tiles2", 0 )
	ROM_LOAD( "man-05.16a", 0x000000,  0x40000,  CRC(d44d1995) SHA1(e88e1a59a4b24ad058f21538f6e9bbba94a166b4) ) // Encrypted tiles
	ROM_CONTINUE(           0x140000,  0x40000 )
	ROM_CONTINUE(           0x280000,  0x40000 )
	ROM_CONTINUE(           0x3c0000,  0x40000 )
	ROM_LOAD( "man-04.14a", 0x040000,  0x40000,  CRC(541492a1) SHA1(2e0ab12555fc46001a815e76e3a0cd21f385f82a) ) // Encrypted tiles
	ROM_CONTINUE(           0x180000,  0x40000 )
	ROM_CONTINUE(           0x2c0000,  0x40000 )
	ROM_CONTINUE(           0x400000,  0x40000 )
	ROM_LOAD( "man-03.12a", 0x080000,  0x40000,  CRC(2d9c52b2) SHA1(8f6f4fe4f1a63099f889068991b34f9432b04fd7) ) // Encrypted tiles
	ROM_CONTINUE(           0x1c0000,  0x40000 )
	ROM_CONTINUE(           0x300000,  0x40000 )
	ROM_CONTINUE(           0x440000,  0x40000 )
	ROM_LOAD( "man-02.11a", 0x0c0000,  0x40000,  CRC(07674c05) SHA1(08b33721d7eba4a1ff2e282f77eeb56535a52923) ) // Encrypted tiles
	ROM_CONTINUE(           0x200000,  0x40000 )
	ROM_CONTINUE(           0x340000,  0x40000 )
	ROM_CONTINUE(           0x480000,  0x40000 )
	ROM_LOAD( "man-01.10a", 0x100000,  0x40000,  CRC(ae714ada) SHA1(b4d5806265d422c8b837489afe93731f584e4adf) ) // Encrypted tiles
	ROM_CONTINUE(           0x240000,  0x40000 )
	ROM_CONTINUE(           0x380000,  0x40000 )
	ROM_CONTINUE(           0x4c0000,  0x40000 )

	ROM_REGION( 0x400000, "sprites1", 0 ) // Sprites
	ROM_LOAD( "man-06.17a",  0x200000,  0x100000,  CRC(a9a64297) SHA1(e4cb441207b1907461c90c32c05a461c9bd30756) )
	ROM_LOAD( "man-07.18a",  0x000000,  0x100000,  CRC(b1db200c) SHA1(970bb15e90194dd285f53594aca5dec3405e75d5) )
	ROM_LOAD( "man-08.17c",  0x300000,  0x100000,  CRC(28e98e66) SHA1(55dbbd945eada81f7dcc874fdcb0b9e62ea453f0) )
	ROM_LOAD( "man-09.21c",  0x100000,  0x100000,  CRC(1921245d) SHA1(88d3b69a38c18c83d5658d057b95974f1bd371e6) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "man-10.14k", 0x000000,  0x80000,  CRC(0132c578) SHA1(70952f39508360bab51e1151531536f0ea6bbe06) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "man-11.16k", 0x000000,  0x80000,  CRC(0dc60a4c) SHA1(4d0daa6a0272852a37f341a0cdc48baee0ad9dd8) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "ts-00.4h",  0x0000, 0x0117, CRC(ebc2908e) SHA1(dca14a55abd1d88ee09092d4122614e55c3e7f53) )
	ROM_LOAD( "ts-01.5h",  0x0200, 0x0117, CRC(c776a980) SHA1(cd4bdcfb755f561fefa4c88fab5d6d2397332aa7) )
	ROM_LOAD( "ts-02.12l", 0x0400, 0x01bf, CRC(6f26528c) SHA1(2cf869b2a789a9b0646162a61c147bcbb13c9141) )
ROM_END

ROM_START( dragngun )
	ROM_REGION(0x200000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_BYTE( "kb02.a9",  0x000000, 0x40000, CRC(4fb9cfea) SHA1(e20fbae32682fc5fdc82070d2d6c73b5b7ac13f8) )
	ROM_LOAD32_BYTE( "kb06.c9",  0x000001, 0x40000, CRC(2395efec) SHA1(3c08299a6cdeebf9d3d5d367ab435eec76986194) )
	ROM_LOAD32_BYTE( "kb00.a5",  0x000002, 0x40000, CRC(1539ff35) SHA1(6c82fe01f5ebf5cdd3a914cc823499fa6a26f9a9) )
	ROM_LOAD32_BYTE( "kb04.c5",  0x000003, 0x40000, CRC(5b5c1ec2) SHA1(3c5c02b7e432cf1861e0c8db23b302dc47774a42) )
	ROM_LOAD32_BYTE( "kb03.a10", 0x100000, 0x40000, CRC(6c6a4f42) SHA1(ae96fe81f9ba587eb3194dbffa0233413d63c4c6) )
	ROM_LOAD32_BYTE( "kb07.c10", 0x100001, 0x40000, CRC(2637e8a1) SHA1(7bcd1b1f3a4e6aaa0a3b78ca77dc666948c87547) )
	ROM_LOAD32_BYTE( "kb01.a7",  0x100002, 0x40000, CRC(d780ba8d) SHA1(0e315c718c038962b6020945b48bcc632de6f5e1) )
	ROM_LOAD32_BYTE( "kb05.c7",  0x100003, 0x40000, CRC(fbad737b) SHA1(04e16abe8c4cec4f172bea29516535511db9db90) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "kb10.n25",  0x00000,  0x10000,  CRC(ec56f560) SHA1(feb9491683ba7f1000edebb568d6b3471fcc87fb) )

	ROM_REGION( 0x020000, "tiles1", 0 )
	ROM_LOAD16_BYTE( "kb08.a15",  0x00000,  0x10000,  CRC(8fe4e5f5) SHA1(922b94f8ce0c35e965259c11e95891ef4be913d4) ) // Encrypted tiles
	ROM_LOAD16_BYTE( "kb09.a17",  0x00001,  0x10000,  CRC(e9dcac3f) SHA1(0621e601ffae73bbf69623042c9c8ab0526c3de6) )

	ROM_REGION( 0x120000, "tiles2", 0 )
	ROM_LOAD( "mar-00.bin",    0x000000,  0x80000,  CRC(d0491a37) SHA1(cc0ae1e9e5f42ba30159fb79bccd2e237cd037d0) ) // Encrypted tiles
	ROM_LOAD( "mar-01.bin",    0x090000,  0x80000,  CRC(d5970365) SHA1(729baf1efbef15c9f3e1d700717f5ba4f10d3014) )

	ROM_REGION( 0x400000, "tiles3", 0 )
	ROM_LOAD( "mar-02.bin",  0x000000, 0x40000,  CRC(c6cd4baf) SHA1(350286829a330b64f463d0a9cbbfdb71eecf5188) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x100000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x200000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x300000, 0x40000 ) // 3/4
	ROM_LOAD( "mar-03.bin",  0x040000, 0x40000,  CRC(793006d7) SHA1(7d8aba2fe75917f580a3a931a7defe5939a0874e) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x140000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x240000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x340000, 0x40000 ) // 3/4
	ROM_LOAD( "mar-04.bin",  0x080000, 0x40000,  CRC(56631a2b) SHA1(0fa3d6215df8ce923c153b96f39161ba88b2dd53) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x180000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x280000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x380000, 0x40000 ) // 3/4
	ROM_LOAD( "mar-05.bin",  0x0c0000, 0x40000,  CRC(ac16e7ae) SHA1(dca32e0a677a99f47a7b8e8f105483c57382f218) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x1c0000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x2c0000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x3c0000, 0x40000 ) // 3/4

	ROM_REGION( 0x800000*2, "c355spr", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE( "mar-15.bin", 0x000000, 0x100000,  CRC(ec976b20) SHA1(c120b3c56d5e02162e41dc7f726c260d0f8d2f1a) )
	ROM_LOAD32_BYTE( "mar-13.bin", 0x000001, 0x100000,  CRC(d675821c) SHA1(ff195422d0bef62d1f9c7784bba1e6b7ab5cd211) )
	ROM_LOAD32_BYTE( "mar-11.bin", 0x000002, 0x100000,  CRC(1fc638a4) SHA1(003dcfbb65a8f32a1a030502a11432287cf8b4e0) )
	ROM_LOAD32_BYTE( "mar-09.bin", 0x000003, 0x100000,  CRC(18fec9e1) SHA1(1290a9c13b4fd7d2197b39ec616206796e3a17a8) )
	ROM_LOAD32_BYTE( "mar-16.bin", 0x400000, 0x100000,  CRC(8b329bc8) SHA1(6e34eb6e2628a01a699d20a5155afb2febc31255) )
	ROM_LOAD32_BYTE( "mar-14.bin", 0x400001, 0x100000,  CRC(22d38c71) SHA1(62273665975f3e6000fa4b01755aeb70e5dd002d) )
	ROM_LOAD32_BYTE( "mar-12.bin", 0x400002, 0x100000,  CRC(4c412512) SHA1(ccd5014bc9f9648cf5fa56bb8d54fc72a7099ca3) )
	ROM_LOAD32_BYTE( "mar-10.bin", 0x400003, 0x100000,  CRC(73126fbc) SHA1(9b9c31335e4db726863b219072c83810008f88f9) )

	// this is standard DVI data, see http://www.fileformat.info/format/dvi/egff.htm
	// there are DVI headers at 0x000000, 0x580000, 0x800000, 0xB10000, 0xB80000
	ROM_REGION32_LE( 0x1000000, "dvi", 0 ) // Video data - unused for now
	ROM_LOAD32_BYTE( "mar-17.bin",  0x000003,  0x100000,  CRC(7799ed23) SHA1(ae28ad4fa6033a3695fa83356701b3774b26e6b0) ) // 56 V / 41 A
	ROM_LOAD32_BYTE( "mar-20.bin",  0x000002,  0x100000,  CRC(fa0462f0) SHA1(1a52617ad4d7abebc0f273dd979f4cf2d6a0306b) ) // 44 D / 56 V
	ROM_LOAD32_BYTE( "mar-28.bin",  0x000001,  0x100000,  CRC(5a2ec71d) SHA1(447c404e6bb696f7eb7c61992a99b9be56f5d6b0) ) // 56 V / 53 S
	ROM_LOAD32_BYTE( "mar-25.bin",  0x000000,  0x100000,  CRC(d65d895c) SHA1(4508dfff95a7aff5109dc74622cbb4503b0b5840) ) // 49 I / 53 S
	ROM_LOAD32_BYTE( "mar-18.bin",  0x400003,  0x100000,  CRC(ded66da9) SHA1(5134cb47043cc190a35ebdbf1912166669f9c055) )
	ROM_LOAD32_BYTE( "mar-21.bin",  0x400002,  0x100000,  CRC(2d0a28ae) SHA1(d87f6f71bb76880e4d4f1eab8e0451b5c3df69a5) )
	ROM_LOAD32_BYTE( "mar-27.bin",  0x400001,  0x100000,  CRC(3fcbd10f) SHA1(70fc7b88bbe35bbae1de14364b03d0a06d541de5) )
	ROM_LOAD32_BYTE( "mar-24.bin",  0x400000,  0x100000,  CRC(5cec45c8) SHA1(f99a26afaca9d9320477e469b09e3873bc8c156f) )
	ROM_LOAD32_BYTE( "mar-19.bin",  0x800003,  0x100000,  CRC(bdd1ed20) SHA1(2435b23210b8fee4d39c30d4d3c6ea40afaa3b93) ) // 56 V / 41 A
	ROM_LOAD32_BYTE( "mar-22.bin",  0x800002,  0x100000,  CRC(c85f3559) SHA1(a5d5cf9b18c9ef6a92d7643ca1ec9052de0d4a01) ) // 44 D / 56 V
	ROM_LOAD32_BYTE( "mar-26.bin",  0x800001,  0x100000,  CRC(246a06c5) SHA1(447252be976a5059925f4ad98df8564b70198f62) ) // 56 V / 53 S
	ROM_LOAD32_BYTE( "mar-23.bin",  0x800000,  0x100000,  CRC(ba907d6a) SHA1(1fd99b66e6297c8d927c1cf723a613b4ee2e2f90) ) // 49 I / 53 S

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mar-06.n17", 0x000000, 0x80000,  CRC(3e006c6e) SHA1(55786e0fde2bf6ba9802f3f4fa8d4c21625b976a) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mar-08.n21", 0x000000, 0x80000,  CRC(b9281dfd) SHA1(449faf5d36f3b970d0a9b483e2152a5f68604a77) )

	// TODO : Japan version uses first bank of oki3, US version uses second half. it has bankswitched dynamic? or address shuffle?
	ROM_REGION(0x80000, "oki3", 0 )
	// Remove this hack if oki3 bankswitching is verified
	ROM_LOAD( "mar-07.n19", 0x40000, 0x40000,  CRC(40287d62) SHA1(c00cb08bcdae55bcddc14c38e88b0484b1bc9e3e) )
	ROM_CONTINUE(           0x00000, 0x40000 )
	//ROM_LOAD( "mar-07.n19", 0x000000, 0x80000,  CRC(40287d62) SHA1(c00cb08bcdae55bcddc14c38e88b0484b1bc9e3e) )
ROM_END

ROM_START( dragngunj )
	ROM_REGION(0x200000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_BYTE( "ka-02.a9",  0x000000, 0x40000, CRC(402a03f9) SHA1(cdd5da9e35191bd716eb6245360702adb6078a94) )
	ROM_LOAD32_BYTE( "ka-06.c9",  0x000001, 0x40000, CRC(26822853) SHA1(8a9e61c9ac9a5aa4b21f063f700acfebac8d408b) )
	ROM_LOAD32_BYTE( "ka-00.a5",  0x000002, 0x40000, CRC(cc9e321b) SHA1(591d5f13a558960dbf286ca4541be0e42b234f2f) )
	ROM_LOAD32_BYTE( "ka-04.c5",  0x000003, 0x40000, CRC(5fd9d935) SHA1(8fd87b05325fae84860bbf1e169a3946f3197307) )
	ROM_LOAD32_BYTE( "ka-03.a10", 0x100000, 0x40000, CRC(e213c859) SHA1(aa0610427bbaa22da7f44289fdf9baf37b636710) )
	ROM_LOAD32_BYTE( "ka-07.c10", 0x100001, 0x40000, CRC(f34c54eb) SHA1(6b67cdb1d2dcc272de96292254914a212ff351cd) )
	ROM_LOAD32_BYTE( "ka-01.a7",  0x100002, 0x40000, CRC(1b52364c) SHA1(151365adc26bc7d71a4d2fc73bca598d3aa09f81) )
	ROM_LOAD32_BYTE( "ka-05.c7",  0x100003, 0x40000, CRC(4c975f52) SHA1(3c6b287c77a049e3f8822ed9d545733e8ea3357b) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "ka-10.n25",  0x00000,  0x10000,  CRC(ec56f560) SHA1(feb9491683ba7f1000edebb568d6b3471fcc87fb) )

	ROM_REGION( 0x020000, "tiles1", 0 )
	ROM_LOAD16_BYTE( "ka-08.a15",  0x00000,  0x10000,  CRC(8fe4e5f5) SHA1(922b94f8ce0c35e965259c11e95891ef4be913d4) ) // Encrypted tiles
	ROM_LOAD16_BYTE( "ka-09.a17",  0x00001,  0x10000,  CRC(e9dcac3f) SHA1(0621e601ffae73bbf69623042c9c8ab0526c3de6) )

	ROM_REGION( 0x120000, "tiles2", 0 )
	ROM_LOAD( "mar-00.bin",    0x000000,  0x80000,  CRC(d0491a37) SHA1(cc0ae1e9e5f42ba30159fb79bccd2e237cd037d0) ) // Encrypted tiles
	ROM_LOAD( "mar-01.bin",    0x090000,  0x80000,  CRC(d5970365) SHA1(729baf1efbef15c9f3e1d700717f5ba4f10d3014) )

	ROM_REGION( 0x400000, "tiles3", 0 )
	ROM_LOAD( "mar-02.bin",  0x000000, 0x40000,  CRC(c6cd4baf) SHA1(350286829a330b64f463d0a9cbbfdb71eecf5188) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x100000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x200000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x300000, 0x40000 ) // 3/4
	ROM_LOAD( "mar-03.bin",  0x040000, 0x40000,  CRC(793006d7) SHA1(7d8aba2fe75917f580a3a931a7defe5939a0874e) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x140000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x240000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x340000, 0x40000 ) // 3/4
	ROM_LOAD( "mar-04.bin",  0x080000, 0x40000,  CRC(56631a2b) SHA1(0fa3d6215df8ce923c153b96f39161ba88b2dd53) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x180000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x280000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x380000, 0x40000 ) // 3/4
	ROM_LOAD( "mar-05.bin",  0x0c0000, 0x40000,  CRC(ac16e7ae) SHA1(dca32e0a677a99f47a7b8e8f105483c57382f218) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x1c0000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x2c0000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x3c0000, 0x40000 ) // 3/4

	ROM_REGION( 0x800000*2, "c355spr", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE( "mar-15.bin", 0x000000, 0x100000,  CRC(ec976b20) SHA1(c120b3c56d5e02162e41dc7f726c260d0f8d2f1a) )
	ROM_LOAD32_BYTE( "mar-13.bin", 0x000001, 0x100000,  CRC(d675821c) SHA1(ff195422d0bef62d1f9c7784bba1e6b7ab5cd211) )
	ROM_LOAD32_BYTE( "mar-11.bin", 0x000002, 0x100000,  CRC(1fc638a4) SHA1(003dcfbb65a8f32a1a030502a11432287cf8b4e0) )
	ROM_LOAD32_BYTE( "mar-09.bin", 0x000003, 0x100000,  CRC(18fec9e1) SHA1(1290a9c13b4fd7d2197b39ec616206796e3a17a8) )
	ROM_LOAD32_BYTE( "mar-16.bin", 0x400000, 0x100000,  CRC(8b329bc8) SHA1(6e34eb6e2628a01a699d20a5155afb2febc31255) )
	ROM_LOAD32_BYTE( "mar-14.bin", 0x400001, 0x100000,  CRC(22d38c71) SHA1(62273665975f3e6000fa4b01755aeb70e5dd002d) )
	ROM_LOAD32_BYTE( "mar-12.bin", 0x400002, 0x100000,  CRC(4c412512) SHA1(ccd5014bc9f9648cf5fa56bb8d54fc72a7099ca3) )
	ROM_LOAD32_BYTE( "mar-10.bin", 0x400003, 0x100000,  CRC(73126fbc) SHA1(9b9c31335e4db726863b219072c83810008f88f9) )

	ROM_REGION32_LE( 0x1000000, "dvi", 0 ) // Video data - unused for now
	ROM_LOAD32_BYTE( "mar-17.bin",  0x000003,  0x100000,  CRC(7799ed23) SHA1(ae28ad4fa6033a3695fa83356701b3774b26e6b0) ) // 56 V / 41 A
	ROM_LOAD32_BYTE( "mar-20.bin",  0x000002,  0x100000,  CRC(fa0462f0) SHA1(1a52617ad4d7abebc0f273dd979f4cf2d6a0306b) ) // 44 D / 56 V
	ROM_LOAD32_BYTE( "mar-28.bin",  0x000001,  0x100000,  CRC(5a2ec71d) SHA1(447c404e6bb696f7eb7c61992a99b9be56f5d6b0) ) // 56 V / 53 S
	ROM_LOAD32_BYTE( "mar-25.bin",  0x000000,  0x100000,  CRC(d65d895c) SHA1(4508dfff95a7aff5109dc74622cbb4503b0b5840) ) // 49 I / 53 S
	ROM_LOAD32_BYTE( "mar-18.bin",  0x400003,  0x100000,  CRC(ded66da9) SHA1(5134cb47043cc190a35ebdbf1912166669f9c055) )
	ROM_LOAD32_BYTE( "mar-21.bin",  0x400002,  0x100000,  CRC(2d0a28ae) SHA1(d87f6f71bb76880e4d4f1eab8e0451b5c3df69a5) )
	ROM_LOAD32_BYTE( "mar-27.bin",  0x400001,  0x100000,  CRC(3fcbd10f) SHA1(70fc7b88bbe35bbae1de14364b03d0a06d541de5) )
	ROM_LOAD32_BYTE( "mar-24.bin",  0x400000,  0x100000,  CRC(5cec45c8) SHA1(f99a26afaca9d9320477e469b09e3873bc8c156f) )
	ROM_LOAD32_BYTE( "mar-19.bin",  0x800003,  0x100000,  CRC(bdd1ed20) SHA1(2435b23210b8fee4d39c30d4d3c6ea40afaa3b93) ) // 56 V / 41 A
	ROM_LOAD32_BYTE( "mar-22.bin",  0x800002,  0x100000,  CRC(c85f3559) SHA1(a5d5cf9b18c9ef6a92d7643ca1ec9052de0d4a01) ) // 44 D / 56 V
	ROM_LOAD32_BYTE( "mar-26.bin",  0x800001,  0x100000,  CRC(246a06c5) SHA1(447252be976a5059925f4ad98df8564b70198f62) ) // 56 V / 53 S
	ROM_LOAD32_BYTE( "mar-23.bin",  0x800000,  0x100000,  CRC(ba907d6a) SHA1(1fd99b66e6297c8d927c1cf723a613b4ee2e2f90) ) // 49 I / 53 S

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mar-06.n17", 0x000000, 0x80000,  CRC(3e006c6e) SHA1(55786e0fde2bf6ba9802f3f4fa8d4c21625b976a) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mar-08.n21", 0x000000, 0x80000,  CRC(b9281dfd) SHA1(449faf5d36f3b970d0a9b483e2152a5f68604a77) )

	ROM_REGION(0x80000, "oki3", 0 )
	ROM_LOAD( "mar-07.n19", 0x000000, 0x80000,  CRC(40287d62) SHA1(c00cb08bcdae55bcddc14c38e88b0484b1bc9e3e) )
ROM_END

ROM_START( fghthist ) // DE-0395-1 PCB
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_WORD( "lc00-1.1f", 0x000000, 0x80000, CRC(61a76a16) SHA1(b69cd3e11cf133f1b14a017391035855a5038d46) ) // Version 43-09, Overseas
	ROM_LOAD32_WORD( "lc01-1.2f", 0x000002, 0x80000, CRC(6f2740d1) SHA1(4fa1fe4714236028ef70d42e15a58cfd25e45363) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "lc02-1.18k",  0x00000,  0x10000,  CRC(5fd2309c) SHA1(2fb7af54d5cd9bf7dd6fb4f6b82aa52b03294f1f) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mbf00-8.8a",  0x000000,  0x100000,  CRC(d3e9b580) SHA1(fc4676e0ecc6c32441ff66fa1f990cc3158237db) ) // Encrypted tiles

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mbf01-8.9a",  0x000000,  0x100000,  CRC(0c6ed2eb) SHA1(8e37ef4b1f0b6d3370a08758bfd602cb5f221282) ) // Encrypted tiles

	ROM_REGION( 0x800000, "sprites1", 0 ) // Sprites
	ROM_LOAD( "mbf02-16.16d",  0x000000,  0x200000,  CRC(c19c5953) SHA1(e6ed26f932c6c86bbd1fc4c000aa2f510c268009) )
	ROM_LOAD( "mbf03-16.17d",  0x200000,  0x200000,  CRC(37d25c75) SHA1(8219d31091b4317190618edd8acc49f97cba6a1e) )
	ROM_LOAD( "mbf04-16.18d",  0x400000,  0x200000,  CRC(f6a23fd7) SHA1(74e5559f17cd591aa25d2ed6c34ac9ed89e2e9ba) )
	ROM_LOAD( "mbf05-16.19d",  0x600000,  0x200000,  CRC(137be66d) SHA1(3fde345183ce04a7a65b4cedfd050d771df7d026) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbf06.15k",  0x000000,  0x80000,  CRC(fb513903) SHA1(7727a49ff7977f159ed36d097020edef3b5b36ba) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbf07.16k",  0x000000,  0x80000,  CRC(51d4adc7) SHA1(22106ed7a05db94adc5a783ce34529e29d24d41a) )

	ROM_REGION(512, "proms", 0 )
	ROM_LOAD( "kt-00.8j",  0,  512,  CRC(7294354b) SHA1(14fe42ad5d26d022c0fe9a46a4a9017af2296f40) ) // MB7124H type prom

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "ve-00.3d",  0x0000, 0x0117, CRC(384d316c) SHA1(61b50c695d4210c199cf6f7bbe50c8a5ecd1d21c) )
	ROM_LOAD( "ve-01a.4d", 0x0200, 0x0117, CRC(109613c8) SHA1(5991e010c1bc2a827c8ee2c85a9b40e00a3167b3) )
	// PAL16L8BCN at 8J is unpopulated
ROM_END

ROM_START( fghthista ) // DE-0380-2 PCB
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_WORD( "kx00-3.1f", 0x000000, 0x80000, CRC(fe5eaba1) SHA1(c8a3784af487a1bbd2150abf4b1c8f3ad33da8a4) ) // Version 43-07, Overseas
	ROM_LOAD32_WORD( "kx01-3.2f", 0x000002, 0x80000, CRC(3fb8d738) SHA1(2fca7a3ea483f01c97fb28a0adfa6d7980d8236c) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "kx02.18k",  0x00000,  0x10000,  CRC(5fd2309c) SHA1(2fb7af54d5cd9bf7dd6fb4f6b82aa52b03294f1f) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mbf00-8.8a",  0x000000,  0x100000,  CRC(d3e9b580) SHA1(fc4676e0ecc6c32441ff66fa1f990cc3158237db) ) // Encrypted tiles

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mbf01-8.9a",  0x000000,  0x100000,  CRC(0c6ed2eb) SHA1(8e37ef4b1f0b6d3370a08758bfd602cb5f221282) ) // Encrypted tiles

	ROM_REGION( 0x800000, "sprites1", 0 ) // Sprites
	ROM_LOAD( "mbf02-16.16d",  0x000000,  0x200000,  CRC(c19c5953) SHA1(e6ed26f932c6c86bbd1fc4c000aa2f510c268009) )
	ROM_LOAD( "mbf03-16.17d",  0x200000,  0x200000,  CRC(37d25c75) SHA1(8219d31091b4317190618edd8acc49f97cba6a1e) )
	ROM_LOAD( "mbf04-16.18d",  0x400000,  0x200000,  CRC(f6a23fd7) SHA1(74e5559f17cd591aa25d2ed6c34ac9ed89e2e9ba) )
	ROM_LOAD( "mbf05-16.19d",  0x600000,  0x200000,  CRC(137be66d) SHA1(3fde345183ce04a7a65b4cedfd050d771df7d026) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbf06.15k",  0x000000,  0x80000,  CRC(fb513903) SHA1(7727a49ff7977f159ed36d097020edef3b5b36ba) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbf07.16k",  0x000000,  0x80000,  CRC(51d4adc7) SHA1(22106ed7a05db94adc5a783ce34529e29d24d41a) )

	ROM_REGION(512, "proms", 0 )
	ROM_LOAD( "kt-00.8j",  0,  512,  CRC(7294354b) SHA1(14fe42ad5d26d022c0fe9a46a4a9017af2296f40) ) // MB7124H type prom

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "ve-00.3d",  0x0000, 0x0117, CRC(384d316c) SHA1(61b50c695d4210c199cf6f7bbe50c8a5ecd1d21c) )
	ROM_LOAD( "ve-01.4d",  0x0200, 0x0117, CRC(4ba7e6a9) SHA1(b65d696a3519e792df226f9f148c759cdb0e1e43) )
ROM_END

ROM_START( fghthistb ) // DE-0380-2 PCB
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_WORD( "kx00-2.1f", 0x000000, 0x80000, CRC(a7c36bbd) SHA1(590937818343da53a6bccbd3ea1d7102abd4f27e) ) // Version 43-05, Overseas
	ROM_LOAD32_WORD( "kx01-2.2f", 0x000002, 0x80000, CRC(bdc60bb1) SHA1(e621c5cf357f49aa62deef4da1e2227021f552ce) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "kx02.18k",  0x00000,  0x10000,  CRC(5fd2309c) SHA1(2fb7af54d5cd9bf7dd6fb4f6b82aa52b03294f1f) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mbf00-8.8a",  0x000000,  0x100000,  CRC(d3e9b580) SHA1(fc4676e0ecc6c32441ff66fa1f990cc3158237db) ) // Encrypted tiles

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mbf01-8.9a",  0x000000,  0x100000,  CRC(0c6ed2eb) SHA1(8e37ef4b1f0b6d3370a08758bfd602cb5f221282) ) // Encrypted tiles

	ROM_REGION( 0x800000, "sprites1", 0 ) // Sprites
	ROM_LOAD( "mbf02-16.16d",  0x000000,  0x200000,  CRC(c19c5953) SHA1(e6ed26f932c6c86bbd1fc4c000aa2f510c268009) )
	ROM_LOAD( "mbf03-16.17d",  0x200000,  0x200000,  CRC(37d25c75) SHA1(8219d31091b4317190618edd8acc49f97cba6a1e) )
	ROM_LOAD( "mbf04-16.18d",  0x400000,  0x200000,  CRC(f6a23fd7) SHA1(74e5559f17cd591aa25d2ed6c34ac9ed89e2e9ba) )
	ROM_LOAD( "mbf05-16.19d",  0x600000,  0x200000,  CRC(137be66d) SHA1(3fde345183ce04a7a65b4cedfd050d771df7d026) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbf06.15k",  0x000000,  0x80000,  CRC(fb513903) SHA1(7727a49ff7977f159ed36d097020edef3b5b36ba) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbf07.16k",  0x000000,  0x80000,  CRC(51d4adc7) SHA1(22106ed7a05db94adc5a783ce34529e29d24d41a) )

	ROM_REGION(512, "proms", 0 )
	ROM_LOAD( "kt-00.8j",  0,  512,  CRC(7294354b) SHA1(14fe42ad5d26d022c0fe9a46a4a9017af2296f40) ) // MB7124H type prom

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "ve-00.3d",  0x0000, 0x0117, CRC(384d316c) SHA1(61b50c695d4210c199cf6f7bbe50c8a5ecd1d21c) )
	ROM_LOAD( "ve-01.4d",  0x0200, 0x0117, CRC(4ba7e6a9) SHA1(b65d696a3519e792df226f9f148c759cdb0e1e43) )
	// PAL16L8BCN at 8J is unpopulated
ROM_END

ROM_START( fghthistu ) // DE-0396-0 PCB
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_WORD( "lj00-3.1f", 0x000000, 0x80000, CRC(17543d60) SHA1(ff206e8552587b41d075b3c99f9ad733f1c2b5e0) ) // Version 42-09, US
	ROM_LOAD32_WORD( "lj01-3.2f", 0x000002, 0x80000, CRC(e255d48f) SHA1(30444832cfed7eeb6082010eb219362adbafb826) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "lj02-.17k",  0x00000,  0x10000,  CRC(146a1063) SHA1(d16734c2443bf38add54040b9dd2628ba523638d) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mbf00-8.8a",  0x000000,  0x100000,  CRC(d3e9b580) SHA1(fc4676e0ecc6c32441ff66fa1f990cc3158237db) ) // Encrypted tiles

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mbf01-8.9a",  0x000000,  0x100000,  CRC(0c6ed2eb) SHA1(8e37ef4b1f0b6d3370a08758bfd602cb5f221282) ) // Encrypted tiles

	ROM_REGION( 0x800000, "sprites1", 0 ) // Sprites
	ROM_LOAD( "mbf02-16.16d",  0x000000,  0x200000,  CRC(c19c5953) SHA1(e6ed26f932c6c86bbd1fc4c000aa2f510c268009) )
	ROM_LOAD( "mbf03-16.17d",  0x200000,  0x200000,  CRC(37d25c75) SHA1(8219d31091b4317190618edd8acc49f97cba6a1e) )
	ROM_LOAD( "mbf04-16.18d",  0x400000,  0x200000,  CRC(f6a23fd7) SHA1(74e5559f17cd591aa25d2ed6c34ac9ed89e2e9ba) )
	ROM_LOAD( "mbf05-16.19d",  0x600000,  0x200000,  CRC(137be66d) SHA1(3fde345183ce04a7a65b4cedfd050d771df7d026) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbf06.15k",  0x000000,  0x80000,  CRC(fb513903) SHA1(7727a49ff7977f159ed36d097020edef3b5b36ba) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbf07.16k",  0x000000,  0x80000,  CRC(51d4adc7) SHA1(22106ed7a05db94adc5a783ce34529e29d24d41a) )

	ROM_REGION(512, "proms", 0 )
	ROM_LOAD( "kt-00.8j",  0,  512,  CRC(7294354b) SHA1(14fe42ad5d26d022c0fe9a46a4a9017af2296f40) ) // MB7124H type prom

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "ve-00.3d",  0x0000, 0x0117, CRC(384d316c) SHA1(61b50c695d4210c199cf6f7bbe50c8a5ecd1d21c) )
	ROM_LOAD( "ve-01.4d",  0x0200, 0x0117, CRC(4ba7e6a9) SHA1(b65d696a3519e792df226f9f148c759cdb0e1e43) )
	// PAL16L8BCN at 8J is unpopulated
ROM_END

ROM_START( fghthistua ) // DE-0395-1 PCB
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_WORD( "le00-1.1f", 0x000000, 0x80000, CRC(fccacafb) SHA1(b7236a90a09dbd5870a16aa4e4eac5ab5c098418) ) // Version 42-06, US
	ROM_LOAD32_WORD( "le01-1.2f", 0x000002, 0x80000, CRC(06a3c326) SHA1(3d8842fb69def93fc544e89fd0e56ada416157dc) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "le02.18k",  0x00000,  0x10000,  CRC(5fd2309c) SHA1(2fb7af54d5cd9bf7dd6fb4f6b82aa52b03294f1f) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mbf00-8.8a",  0x000000,  0x100000,  CRC(d3e9b580) SHA1(fc4676e0ecc6c32441ff66fa1f990cc3158237db) ) // Encrypted tiles

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mbf01-8.9a",  0x000000,  0x100000,  CRC(0c6ed2eb) SHA1(8e37ef4b1f0b6d3370a08758bfd602cb5f221282) ) // Encrypted tiles

	ROM_REGION( 0x800000, "sprites1", 0 ) // Sprites
	ROM_LOAD( "mbf02-16.16d",  0x000000,  0x200000,  CRC(c19c5953) SHA1(e6ed26f932c6c86bbd1fc4c000aa2f510c268009) )
	ROM_LOAD( "mbf03-16.17d",  0x200000,  0x200000,  CRC(37d25c75) SHA1(8219d31091b4317190618edd8acc49f97cba6a1e) )
	ROM_LOAD( "mbf04-16.18d",  0x400000,  0x200000,  CRC(f6a23fd7) SHA1(74e5559f17cd591aa25d2ed6c34ac9ed89e2e9ba) )
	ROM_LOAD( "mbf05-16.19d",  0x600000,  0x200000,  CRC(137be66d) SHA1(3fde345183ce04a7a65b4cedfd050d771df7d026) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbf06.15k",  0x000000,  0x80000,  CRC(fb513903) SHA1(7727a49ff7977f159ed36d097020edef3b5b36ba) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbf07.16k",  0x000000,  0x80000,  CRC(51d4adc7) SHA1(22106ed7a05db94adc5a783ce34529e29d24d41a) )

	ROM_REGION(512, "proms", 0 )
	ROM_LOAD( "kt-00.8j",  0,  512,  CRC(7294354b) SHA1(14fe42ad5d26d022c0fe9a46a4a9017af2296f40) ) // MB7124H type prom

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "ve-00.3d",  0x0000, 0x0117, CRC(384d316c) SHA1(61b50c695d4210c199cf6f7bbe50c8a5ecd1d21c) )
	ROM_LOAD( "ve-01a.4d", 0x0200, 0x0117, CRC(109613c8) SHA1(5991e010c1bc2a827c8ee2c85a9b40e00a3167b3) )
	// PAL16L8BCN at 8J is unpopulated
ROM_END

ROM_START( fghthistub ) // DE-0395-1 PCB
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_WORD( "le00.1f", 0x000000, 0x80000, CRC(a5c410eb) SHA1(e2b0cb2351782e1155ecc4029010beb7326fd874) ) // Version 42-05, US
	ROM_LOAD32_WORD( "le01.2f", 0x000002, 0x80000, CRC(7e148aa2) SHA1(b21e16604c4d29611f91d629deb9f041eaf41e9b) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "le02.18k",  0x00000,  0x10000,  CRC(5fd2309c) SHA1(2fb7af54d5cd9bf7dd6fb4f6b82aa52b03294f1f) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mbf00-8.8a",  0x000000,  0x100000,  CRC(d3e9b580) SHA1(fc4676e0ecc6c32441ff66fa1f990cc3158237db) ) // Encrypted tiles

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mbf01-8.9a",  0x000000,  0x100000,  CRC(0c6ed2eb) SHA1(8e37ef4b1f0b6d3370a08758bfd602cb5f221282) ) // Encrypted tiles

	ROM_REGION( 0x800000, "sprites1", 0 ) // Sprites
	ROM_LOAD( "mbf02-16.16d",  0x000000,  0x200000,  CRC(c19c5953) SHA1(e6ed26f932c6c86bbd1fc4c000aa2f510c268009) )
	ROM_LOAD( "mbf03-16.17d",  0x200000,  0x200000,  CRC(37d25c75) SHA1(8219d31091b4317190618edd8acc49f97cba6a1e) )
	ROM_LOAD( "mbf04-16.18d",  0x400000,  0x200000,  CRC(f6a23fd7) SHA1(74e5559f17cd591aa25d2ed6c34ac9ed89e2e9ba) )
	ROM_LOAD( "mbf05-16.19d",  0x600000,  0x200000,  CRC(137be66d) SHA1(3fde345183ce04a7a65b4cedfd050d771df7d026) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbf06.15k",  0x000000,  0x80000,  CRC(fb513903) SHA1(7727a49ff7977f159ed36d097020edef3b5b36ba) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbf07.16k",  0x000000,  0x80000,  CRC(51d4adc7) SHA1(22106ed7a05db94adc5a783ce34529e29d24d41a) )

	ROM_REGION(512, "proms", 0 )
	ROM_LOAD( "kt-00.8j",  0,  512,  CRC(7294354b) SHA1(14fe42ad5d26d022c0fe9a46a4a9017af2296f40) ) // MB7124H type prom

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "ve-00.3d",  0x0000, 0x0117, CRC(384d316c) SHA1(61b50c695d4210c199cf6f7bbe50c8a5ecd1d21c) )
	ROM_LOAD( "ve-01a.4d", 0x0200, 0x0117, CRC(109613c8) SHA1(5991e010c1bc2a827c8ee2c85a9b40e00a3167b3) )
	// PAL16L8BCN at 8J is unpopulated
ROM_END

ROM_START( fghthistuc ) // DE-0380-2 PCB
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_WORD( "kz00-1.1f", 0x000000, 0x80000, CRC(3a3dd15c) SHA1(689b51adf73402b12191a75061b8e709468c91bc) ) // Version 42-03, US
	ROM_LOAD32_WORD( "kz01-1.2f", 0x000002, 0x80000, CRC(86796cd6) SHA1(c397c07d7a1d03ba96ccb2fe7a0ad25b8331e945) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "kz02.18k",  0x00000,  0x10000,  CRC(5fd2309c) SHA1(2fb7af54d5cd9bf7dd6fb4f6b82aa52b03294f1f) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mbf00-8.8a",  0x000000,  0x100000,  CRC(d3e9b580) SHA1(fc4676e0ecc6c32441ff66fa1f990cc3158237db) ) // Encrypted tiles

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mbf01-8.9a",  0x000000,  0x100000,  CRC(0c6ed2eb) SHA1(8e37ef4b1f0b6d3370a08758bfd602cb5f221282) ) // Encrypted tiles

	ROM_REGION( 0x800000, "sprites1", 0 ) // Sprites
	ROM_LOAD( "mbf02-16.16d",  0x000000,  0x200000,  CRC(c19c5953) SHA1(e6ed26f932c6c86bbd1fc4c000aa2f510c268009) )
	ROM_LOAD( "mbf03-16.17d",  0x200000,  0x200000,  CRC(37d25c75) SHA1(8219d31091b4317190618edd8acc49f97cba6a1e) )
	ROM_LOAD( "mbf04-16.18d",  0x400000,  0x200000,  CRC(f6a23fd7) SHA1(74e5559f17cd591aa25d2ed6c34ac9ed89e2e9ba) )
	ROM_LOAD( "mbf05-16.19d",  0x600000,  0x200000,  CRC(137be66d) SHA1(3fde345183ce04a7a65b4cedfd050d771df7d026) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbf06.15k",  0x000000,  0x80000,  CRC(fb513903) SHA1(7727a49ff7977f159ed36d097020edef3b5b36ba) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbf07.16k",  0x000000,  0x80000,  CRC(51d4adc7) SHA1(22106ed7a05db94adc5a783ce34529e29d24d41a) )

	ROM_REGION(512, "proms", 0 )
	ROM_LOAD( "kt-00.8j",  0,  512,  CRC(7294354b) SHA1(14fe42ad5d26d022c0fe9a46a4a9017af2296f40) ) // MB7124H type prom

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "ve-00.3d",  0x0000, 0x0117, CRC(384d316c) SHA1(61b50c695d4210c199cf6f7bbe50c8a5ecd1d21c) )
	ROM_LOAD( "ve-01.4d",  0x0200, 0x0117, CRC(4ba7e6a9) SHA1(b65d696a3519e792df226f9f148c759cdb0e1e43) )
	// PAL16L8BCN at 8J is unpopulated
ROM_END

ROM_START( fghthistj ) // DE-0395-1 PCB
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_WORD( "lb00.1f", 0x000000, 0x80000, CRC(321099ad) SHA1(c5f8cedc1d349fb24b0d7b942dcda02190b1b536) ) // Version 41-07, Japan
	ROM_LOAD32_WORD( "lb01.2f", 0x000002, 0x80000, CRC(22f45755) SHA1(02ba35b557085e379be98705ca5395b677a264fd) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "lb02.18k",  0x00000,  0x10000,  CRC(5fd2309c) SHA1(2fb7af54d5cd9bf7dd6fb4f6b82aa52b03294f1f) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mbf00-8.8a",  0x000000,  0x100000,  CRC(d3e9b580) SHA1(fc4676e0ecc6c32441ff66fa1f990cc3158237db) ) // Encrypted tiles

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mbf01-8.9a",  0x000000,  0x100000,  CRC(0c6ed2eb) SHA1(8e37ef4b1f0b6d3370a08758bfd602cb5f221282) ) // Encrypted tiles

	ROM_REGION( 0x800000, "sprites1", 0 ) // Sprites
	ROM_LOAD( "mbf02-16.16d",  0x000000,  0x200000,  CRC(c19c5953) SHA1(e6ed26f932c6c86bbd1fc4c000aa2f510c268009) )
	ROM_LOAD( "mbf03-16.17d",  0x200000,  0x200000,  CRC(37d25c75) SHA1(8219d31091b4317190618edd8acc49f97cba6a1e) )
	ROM_LOAD( "mbf04-16.18d",  0x400000,  0x200000,  CRC(f6a23fd7) SHA1(74e5559f17cd591aa25d2ed6c34ac9ed89e2e9ba) )
	ROM_LOAD( "mbf05-16.19d",  0x600000,  0x200000,  CRC(137be66d) SHA1(3fde345183ce04a7a65b4cedfd050d771df7d026) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbf06.15k",  0x000000,  0x80000,  CRC(fb513903) SHA1(7727a49ff7977f159ed36d097020edef3b5b36ba) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbf07.16k",  0x000000,  0x80000,  CRC(51d4adc7) SHA1(22106ed7a05db94adc5a783ce34529e29d24d41a) )

	ROM_REGION(512, "proms", 0 )
	ROM_LOAD( "kt-00.8j",  0,  512,  CRC(7294354b) SHA1(14fe42ad5d26d022c0fe9a46a4a9017af2296f40) ) // MB7124H type prom

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "ve-00.3d",  0x0000, 0x0117, CRC(384d316c) SHA1(61b50c695d4210c199cf6f7bbe50c8a5ecd1d21c) )
	ROM_LOAD( "ve-01a.4d", 0x0200, 0x0117, CRC(109613c8) SHA1(5991e010c1bc2a827c8ee2c85a9b40e00a3167b3) )
	// PAL16L8BCN at 8J is unpopulated
ROM_END

ROM_START( fghthistja ) // DE-0380-2 PCB
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_WORD( "kw00-3.1f", 0x000000, 0x80000, CRC(ade9581a) SHA1(c1302e921f119ff9baeb52f9c338df652e64a9ee) ) // Version 41-05, Japan
	ROM_LOAD32_WORD( "kw01-3.2f", 0x000002, 0x80000, CRC(63580acf) SHA1(03372b168fe461542dd1cf64b4021d948d07e15c) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "kw02-.18k",  0x00000,  0x10000,  CRC(5fd2309c) SHA1(2fb7af54d5cd9bf7dd6fb4f6b82aa52b03294f1f) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mbf00-8.8a",  0x000000,  0x100000,  CRC(d3e9b580) SHA1(fc4676e0ecc6c32441ff66fa1f990cc3158237db) ) // Encrypted tiles

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mbf01-8.9a",  0x000000,  0x100000,  CRC(0c6ed2eb) SHA1(8e37ef4b1f0b6d3370a08758bfd602cb5f221282) ) // Encrypted tiles

	ROM_REGION( 0x800000, "sprites1", 0 ) // Sprites
	ROM_LOAD( "mbf02-16.16d",  0x000000,  0x200000,  CRC(c19c5953) SHA1(e6ed26f932c6c86bbd1fc4c000aa2f510c268009) )
	ROM_LOAD( "mbf03-16.17d",  0x200000,  0x200000,  CRC(37d25c75) SHA1(8219d31091b4317190618edd8acc49f97cba6a1e) )
	ROM_LOAD( "mbf04-16.18d",  0x400000,  0x200000,  CRC(f6a23fd7) SHA1(74e5559f17cd591aa25d2ed6c34ac9ed89e2e9ba) )
	ROM_LOAD( "mbf05-16.19d",  0x600000,  0x200000,  CRC(137be66d) SHA1(3fde345183ce04a7a65b4cedfd050d771df7d026) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbf06.15k",  0x000000,  0x80000,  CRC(fb513903) SHA1(7727a49ff7977f159ed36d097020edef3b5b36ba) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbf07.16k",  0x000000,  0x80000,  CRC(51d4adc7) SHA1(22106ed7a05db94adc5a783ce34529e29d24d41a) )

	ROM_REGION(512, "proms", 0 )
	ROM_LOAD( "kt-00.8j",  0,  512,  CRC(7294354b) SHA1(14fe42ad5d26d022c0fe9a46a4a9017af2296f40) ) // MB7124H type prom

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "ve-00.3d",  0x0000, 0x0117, CRC(384d316c) SHA1(61b50c695d4210c199cf6f7bbe50c8a5ecd1d21c) )
	ROM_LOAD( "ve-01.4d",  0x0200, 0x0117, CRC(4ba7e6a9) SHA1(b65d696a3519e792df226f9f148c759cdb0e1e43) )
	// PAL16L8BCN at 8J is unpopulated
ROM_END

ROM_START( fghthistjb ) // DE-0380-1 PCB
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_WORD( "kw00-2.1f", 0x000000, 0x80000, CRC(f4749806) SHA1(acdbd19b350d5d8670db879c446633a991e28c05) ) // Version 41-04, Japan
	ROM_LOAD32_WORD( "kw01-2.2f", 0x000002, 0x80000, CRC(7e0ee66a) SHA1(d62321eb9942bfe8629010fabeb42356cf7dd4d6) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "kw02-.18k",  0x00000,  0x10000,  CRC(5fd2309c) SHA1(2fb7af54d5cd9bf7dd6fb4f6b82aa52b03294f1f) )

	ROM_REGION( 0x100000, "tiles1", 0 )
	ROM_LOAD( "mbf00-8.8a",  0x000000,  0x100000,  CRC(d3e9b580) SHA1(fc4676e0ecc6c32441ff66fa1f990cc3158237db) ) // Encrypted tiles

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mbf01-8.9a",  0x000000,  0x100000,  CRC(0c6ed2eb) SHA1(8e37ef4b1f0b6d3370a08758bfd602cb5f221282) ) // Encrypted tiles

	ROM_REGION( 0x800000, "sprites1", 0 ) // Sprites
	ROM_LOAD( "mbf02-16.16d",  0x000000,  0x200000,  CRC(c19c5953) SHA1(e6ed26f932c6c86bbd1fc4c000aa2f510c268009) )
	ROM_LOAD( "mbf03-16.17d",  0x200000,  0x200000,  CRC(37d25c75) SHA1(8219d31091b4317190618edd8acc49f97cba6a1e) )
	ROM_LOAD( "mbf04-16.18d",  0x400000,  0x200000,  CRC(f6a23fd7) SHA1(74e5559f17cd591aa25d2ed6c34ac9ed89e2e9ba) )
	ROM_LOAD( "mbf05-16.19d",  0x600000,  0x200000,  CRC(137be66d) SHA1(3fde345183ce04a7a65b4cedfd050d771df7d026) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbf06.15k",  0x000000,  0x80000,  CRC(fb513903) SHA1(7727a49ff7977f159ed36d097020edef3b5b36ba) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbf07.16k",  0x000000,  0x80000,  CRC(51d4adc7) SHA1(22106ed7a05db94adc5a783ce34529e29d24d41a) )

	ROM_REGION(512, "proms", 0 )
	ROM_LOAD( "kt-00.8j",  0,  512,  CRC(7294354b) SHA1(14fe42ad5d26d022c0fe9a46a4a9017af2296f40) ) // MB7124H type prom

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "ve-00.3d",  0x0000, 0x0117, CRC(384d316c) SHA1(61b50c695d4210c199cf6f7bbe50c8a5ecd1d21c) )
	ROM_LOAD( "ve-01.4d",  0x0200, 0x0117, CRC(4ba7e6a9) SHA1(b65d696a3519e792df226f9f148c759cdb0e1e43) )
	// PAL16L8BCN at 8J is unpopulated
ROM_END

ROM_START( lockload ) // Board No. DE-0420-1 + Bottom board DE-0421-0 slightly different hardware, a unique PCB and not a Dragongun conversion
	ROM_REGION(0x200000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_BYTE( "nl-00-1.a6", 0x000002, 0x80000, CRC(7a39bf8d) SHA1(8b1a6407bab74b3960a243a6c04c0005a82126f1) )
	ROM_LOAD32_BYTE( "nl-01-1.a8", 0x000000, 0x80000, CRC(d23afcb7) SHA1(de7b5bc936a87cc6511d588b0bf082bbf745581c) )
	ROM_LOAD32_BYTE( "nl-02-1.d6", 0x000003, 0x80000, CRC(730e0168) SHA1(fdfa0d335c03c2c528326f90948e642f9ea43150) )
	ROM_LOAD32_BYTE( "nl-03-1.d8", 0x000001, 0x80000, CRC(51a53ece) SHA1(ee2c8858844a47fa1e83c30c06d78cf49219dc33) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "nm-06-.p22",  0x00000,  0x10000,  CRC(31d1c245) SHA1(326e35e7ebd8ea761d90e856c50d86512327f2a5) )

	ROM_REGION( 0x020000, "tiles1", 0 )
	ROM_LOAD16_BYTE( "nl-04-.a15",  0x00000,  0x10000,  CRC(f097b3d9) SHA1(5748de9a796afddd78dad7f5c184269ee533c51c) ) // Encrypted tiles
	ROM_LOAD16_BYTE( "nl-05-.a17",  0x00001,  0x10000,  CRC(448fec1e) SHA1(9a107959621cbb3688fd3ad9a8320aa5584f7d13) )

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mbm-00.d15",  0x00000, 0x80000,  CRC(b97de8ff) SHA1(59508f7135af22c2ac89db78874b1e8a68c53434) ) // Encrypted tiles
	ROM_LOAD( "mbm-01.d17",  0x80000, 0x80000,  CRC(6d4b8fa0) SHA1(56e2b9adb4d010ba2592eccba654a24141441141) )

	ROM_REGION( 0x800000, "tiles3", 0 )
	ROM_LOAD( "mbm-02.b21",  0x000000, 0x40000,  CRC(e723019f) SHA1(15361d3e6db5707a7f0ead4254463c50163c864c) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x200000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x400000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x600000, 0x40000 ) // 3/4
	ROM_CONTINUE(            0x040000, 0x40000 ) // Next block 2bpp 0/4
	ROM_CONTINUE(            0x240000, 0x40000 ) // 1/4
	ROM_CONTINUE(            0x440000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x640000, 0x40000 ) // 3/4
	ROM_LOAD( "mbm-03.b22",  0x080000, 0x40000,  CRC(e0d09894) SHA1(be2faa81cf92b6fadfb2ec4ca2173157b05071ec) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x280000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x480000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x680000, 0x40000 ) // 3/4
	ROM_CONTINUE(            0x0c0000, 0x40000 ) // Next block 2bpp 0/4
	ROM_CONTINUE(            0x2c0000, 0x40000 ) // 1/4
	ROM_CONTINUE(            0x4c0000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x6c0000, 0x40000 ) // 3/4
	ROM_LOAD( "mbm-04.e21",  0x100000, 0x40000,  CRC(9e12466f) SHA1(51eaadfaf45d02d72b61052a606f97f36b3964fd) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x300000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x500000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x700000, 0x40000 ) // 3/4
	ROM_CONTINUE(            0x140000, 0x40000 ) // Next block 2bpp 0/4
	ROM_CONTINUE(            0x340000, 0x40000 ) // 1/4
	ROM_CONTINUE(            0x540000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x740000, 0x40000 ) // 3/4
	ROM_LOAD( "mbm-05.e22",  0x180000, 0x40000,  CRC(6ff02dc0) SHA1(5862e2189a09f963d5ec58ca4aa1c06210a3c7ef) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x380000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x580000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x780000, 0x40000 ) // 3/4
	ROM_CONTINUE(            0x1c0000, 0x40000 ) // Next block 2bpp 0/4
	ROM_CONTINUE(            0x3c0000, 0x40000 ) // 1/4
	ROM_CONTINUE(            0x5c0000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x7c0000, 0x40000 ) // 3/4

	ROM_REGION( 0x800000*2, "c355spr", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE( "mbm-14.a23",  0x000000, 0x100000,  CRC(5aaaf929) SHA1(5ee30db9b83db664d77e6b5e0988ce3366460df6) )
	ROM_LOAD32_BYTE( "mbm-12.a21",  0x000001, 0x100000,  CRC(7d221d66) SHA1(25c9c20485e443969c0bf4d74c4211c3881dabcd) )
	ROM_LOAD32_BYTE( "mbm-10.a19",  0x000002, 0x100000,  CRC(232e1c91) SHA1(868d4eb4873ecc210cbb3a266cae0b6ad8f11add) )
	ROM_LOAD32_BYTE( "mbm-08.a14",  0x000003, 0x100000,  CRC(5358a43b) SHA1(778637fc63a0957338c7da3adb2555ffada177c4) )
	ROM_LOAD32_BYTE( "mbm-15.a25",  0x400000, 0x100000,  CRC(789ce7b1) SHA1(3fb390ce0620ce7a63f7f46eac1ff0eb8ed76d26) )
	ROM_LOAD32_BYTE( "mbm-13.a22",  0x400001, 0x100000,  CRC(678b9052) SHA1(ae8fc921813e53e9dbc3960e772c1c4de94c22a7) )
	ROM_LOAD32_BYTE( "mbm-11.a20",  0x400002, 0x100000,  CRC(8a2a2a9f) SHA1(d11e0ea2785e35123bc56a8f18ce22f58432b599) )
	ROM_LOAD32_BYTE( "mbm-09.a16",  0x400003, 0x100000,  CRC(2cce162f) SHA1(db5795465a36971861e8fb7436db0805717ad101) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbm-07.n19",  0x00000, 0x80000,  CRC(414f3793) SHA1(ed5f63e57390d503193fd1e9f7294ae1da6d3539) )

	ROM_REGION(0x100000, "oki2", 0 )
	ROM_LOAD( "mbm-06.n17",  0x00000, 0x100000,  CRC(f34d5999) SHA1(265b5f4e8598bcf9183bf9bd95db69b01536acb2) )
ROM_END

ROM_START( gunhard ) // Board No. DE-0420-1 + Bottom board DE-0421-0 slightly different hardware, a unique PCB and not a Dragongun conversion
	ROM_REGION(0x200000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_BYTE( "nf-00-1.a6", 0x000002, 0x80000, CRC(2c8045d4) SHA1(4c900951d56bd22e30905969b8eb687d9b4363bd) )
	ROM_LOAD32_BYTE( "nf-01-1.a8", 0x000000, 0x80000, CRC(6f160117) SHA1(05738f61890e9d6d2b25330958c0e7369f2ff4a6) )
	ROM_LOAD32_BYTE( "nf-02-1.d6", 0x000003, 0x80000, CRC(bd353948) SHA1(ddcc12b3d1c8919eb7eb961d61f6286e6b37a58e) )
	ROM_LOAD32_BYTE( "nf-03-1.d8", 0x000001, 0x80000, CRC(118a9a72) SHA1(e0b2fd21f477e531d6a04256767874f13e031a48) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "nj-06-1.p22",  0x00000,  0x10000,  CRC(31d1c245) SHA1(326e35e7ebd8ea761d90e856c50d86512327f2a5) )

	ROM_REGION( 0x020000, "tiles1", 0 )
	ROM_LOAD16_BYTE( "nf-04-1.a15",  0x00000,  0x10000,  CRC(f097b3d9) SHA1(5748de9a796afddd78dad7f5c184269ee533c51c) ) // Encrypted tiles
	ROM_LOAD16_BYTE( "nf-05-1.a17",  0x00001,  0x10000,  CRC(448fec1e) SHA1(9a107959621cbb3688fd3ad9a8320aa5584f7d13) )

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mbm-00.d15",  0x00000, 0x80000,  CRC(b97de8ff) SHA1(59508f7135af22c2ac89db78874b1e8a68c53434) ) // Encrypted tiles
	ROM_LOAD( "mbm-01.d17",  0x80000, 0x80000,  CRC(6d4b8fa0) SHA1(56e2b9adb4d010ba2592eccba654a24141441141) )

	ROM_REGION( 0x800000, "tiles3", 0 )
	ROM_LOAD( "mbm-02.b21",  0x000000, 0x40000,  CRC(e723019f) SHA1(15361d3e6db5707a7f0ead4254463c50163c864c) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x200000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x400000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x600000, 0x40000 ) // 3/4
	ROM_CONTINUE(            0x040000, 0x40000 ) // Next block 2bpp 0/4
	ROM_CONTINUE(            0x240000, 0x40000 ) // 1/4
	ROM_CONTINUE(            0x440000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x640000, 0x40000 ) // 3/4
	ROM_LOAD( "mbm-03.b22",  0x080000, 0x40000,  CRC(e0d09894) SHA1(be2faa81cf92b6fadfb2ec4ca2173157b05071ec) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x280000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x480000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x680000, 0x40000 ) // 3/4
	ROM_CONTINUE(            0x0c0000, 0x40000 ) // Next block 2bpp 0/4
	ROM_CONTINUE(            0x2c0000, 0x40000 ) // 1/4
	ROM_CONTINUE(            0x4c0000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x6c0000, 0x40000 ) // 3/4
	ROM_LOAD( "mbm-04.e21",  0x100000, 0x40000,  CRC(9e12466f) SHA1(51eaadfaf45d02d72b61052a606f97f36b3964fd) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x300000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x500000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x700000, 0x40000 ) // 3/4
	ROM_CONTINUE(            0x140000, 0x40000 ) // Next block 2bpp 0/4
	ROM_CONTINUE(            0x340000, 0x40000 ) // 1/4
	ROM_CONTINUE(            0x540000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x740000, 0x40000 ) // 3/4
	ROM_LOAD( "mbm-05.e22",  0x180000, 0x40000,  CRC(6ff02dc0) SHA1(5862e2189a09f963d5ec58ca4aa1c06210a3c7ef) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x380000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x580000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x780000, 0x40000 ) // 3/4
	ROM_CONTINUE(            0x1c0000, 0x40000 ) // Next block 2bpp 0/4
	ROM_CONTINUE(            0x3c0000, 0x40000 ) // 1/4
	ROM_CONTINUE(            0x5c0000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x7c0000, 0x40000 ) // 3/4

	ROM_REGION( 0x800000*2, "c355spr", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE( "mbm-14.a23",  0x000000, 0x100000,  CRC(5aaaf929) SHA1(5ee30db9b83db664d77e6b5e0988ce3366460df6) )
	ROM_LOAD32_BYTE( "mbm-12.a21",  0x000001, 0x100000,  CRC(7d221d66) SHA1(25c9c20485e443969c0bf4d74c4211c3881dabcd) )
	ROM_LOAD32_BYTE( "mbm-10.a19",  0x000002, 0x100000,  CRC(232e1c91) SHA1(868d4eb4873ecc210cbb3a266cae0b6ad8f11add) )
	ROM_LOAD32_BYTE( "mbm-08.a14",  0x000003, 0x100000,  CRC(5358a43b) SHA1(778637fc63a0957338c7da3adb2555ffada177c4) )
	ROM_LOAD32_BYTE( "mbm-15.a25",  0x400000, 0x100000,  CRC(789ce7b1) SHA1(3fb390ce0620ce7a63f7f46eac1ff0eb8ed76d26) )
	ROM_LOAD32_BYTE( "mbm-13.a22",  0x400001, 0x100000,  CRC(678b9052) SHA1(ae8fc921813e53e9dbc3960e772c1c4de94c22a7) )
	ROM_LOAD32_BYTE( "mbm-11.a20",  0x400002, 0x100000,  CRC(8a2a2a9f) SHA1(d11e0ea2785e35123bc56a8f18ce22f58432b599) )
	ROM_LOAD32_BYTE( "mbm-09.a16",  0x400003, 0x100000,  CRC(2cce162f) SHA1(db5795465a36971861e8fb7436db0805717ad101) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbm-07.n19",  0x00000, 0x80000,  CRC(414f3793) SHA1(ed5f63e57390d503193fd1e9f7294ae1da6d3539) )

	ROM_REGION(0x100000, "oki2", 0 )
	ROM_LOAD( "mbm-06.n17",  0x00000, 0x100000,  CRC(f34d5999) SHA1(265b5f4e8598bcf9183bf9bd95db69b01536acb2) )
ROM_END

ROM_START( lockloadu ) // Board No. DE-0359-2 + Bottom board DE-0360-4, a Dragongun conversion
	ROM_REGION(0x200000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_BYTE( "nh-00-0.b5", 0x000002, 0x80000, CRC(b8a57164) SHA1(b700a08db2ad1aa1bf0a32635ffbd5d3f08713ee) )
	ROM_LOAD32_BYTE( "nh-01-0.b8", 0x000000, 0x80000, CRC(e371ac50) SHA1(c448b54bc8962844b490994607b21b0c806d7714) )
	ROM_LOAD32_BYTE( "nh-02-0.d5", 0x000003, 0x80000, CRC(3e361e82) SHA1(b5445d44f2a775c141fdc561d5489234c39445a4) )
	ROM_LOAD32_BYTE( "nh-03-0.d8", 0x000001, 0x80000, CRC(d08ee9c3) SHA1(9a85710a11940df047e83e8d5977a23d6c67d665) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "nh-06-0.n25",  0x00000,  0x10000,  CRC(7a1af51d) SHA1(54e6b16d3f5b787d3c6eb7203d8854e6e0fb9803) )

	ROM_REGION( 0x020000, "tiles1", 0 )
	ROM_LOAD16_BYTE( "nh-04-0.b15",  0x00000,  0x10000,  CRC(f097b3d9) SHA1(5748de9a796afddd78dad7f5c184269ee533c51c) ) // Encrypted tiles
	ROM_LOAD16_BYTE( "nh-05-0.b17",  0x00001,  0x10000,  CRC(448fec1e) SHA1(9a107959621cbb3688fd3ad9a8320aa5584f7d13) )

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mbm-00.d15",  0x00000, 0x80000,  CRC(b97de8ff) SHA1(59508f7135af22c2ac89db78874b1e8a68c53434) ) // Encrypted tiles
	ROM_LOAD( "mbm-01.d17",  0x80000, 0x80000,  CRC(6d4b8fa0) SHA1(56e2b9adb4d010ba2592eccba654a24141441141) )

	ROM_REGION( 0x800000, "tiles3", 0 )
	ROM_LOAD( "mbm-02.b23",  0x000000, 0x40000,  CRC(e723019f) SHA1(15361d3e6db5707a7f0ead4254463c50163c864c) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x200000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x400000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x600000, 0x40000 ) // 3/4
	ROM_CONTINUE(            0x040000, 0x40000 ) // Next block 2bpp 0/4
	ROM_CONTINUE(            0x240000, 0x40000 ) // 1/4
	ROM_CONTINUE(            0x440000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x640000, 0x40000 ) // 3/4
	ROM_LOAD( "mbm-03.b26",  0x080000, 0x40000,  CRC(e0d09894) SHA1(be2faa81cf92b6fadfb2ec4ca2173157b05071ec) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x280000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x480000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x680000, 0x40000 ) // 3/4
	ROM_CONTINUE(            0x0c0000, 0x40000 ) // Next block 2bpp 0/4
	ROM_CONTINUE(            0x2c0000, 0x40000 ) // 1/4
	ROM_CONTINUE(            0x4c0000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x6c0000, 0x40000 ) // 3/4
	ROM_LOAD( "mbm-04.e23",  0x100000, 0x40000,  CRC(9e12466f) SHA1(51eaadfaf45d02d72b61052a606f97f36b3964fd) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x300000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x500000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x700000, 0x40000 ) // 3/4
	ROM_CONTINUE(            0x140000, 0x40000 ) // Next block 2bpp 0/4
	ROM_CONTINUE(            0x340000, 0x40000 ) // 1/4
	ROM_CONTINUE(            0x540000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x740000, 0x40000 ) // 3/4
	ROM_LOAD( "mbm-05.e26",  0x180000, 0x40000,  CRC(6ff02dc0) SHA1(5862e2189a09f963d5ec58ca4aa1c06210a3c7ef) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x380000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x580000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x780000, 0x40000 ) // 3/4
	ROM_CONTINUE(            0x1c0000, 0x40000 ) // Next block 2bpp 0/4
	ROM_CONTINUE(            0x3c0000, 0x40000 ) // 1/4
	ROM_CONTINUE(            0x5c0000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x7c0000, 0x40000 ) // 3/4

	ROM_REGION( 0x800000*2, "c355spr", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE( "mbm-14.a23",  0x000000, 0x100000,  CRC(5aaaf929) SHA1(5ee30db9b83db664d77e6b5e0988ce3366460df6) )
	ROM_LOAD32_BYTE( "mbm-12.a21",  0x000001, 0x100000,  CRC(7d221d66) SHA1(25c9c20485e443969c0bf4d74c4211c3881dabcd) )
	ROM_LOAD32_BYTE( "mbm-10.a19",  0x000002, 0x100000,  CRC(232e1c91) SHA1(868d4eb4873ecc210cbb3a266cae0b6ad8f11add) )
	ROM_LOAD32_BYTE( "mbm-08.a14",  0x000003, 0x100000,  CRC(5358a43b) SHA1(778637fc63a0957338c7da3adb2555ffada177c4) )
	ROM_LOAD32_BYTE( "mbm-15.a25",  0x400000, 0x100000,  CRC(789ce7b1) SHA1(3fb390ce0620ce7a63f7f46eac1ff0eb8ed76d26) )
	ROM_LOAD32_BYTE( "mbm-13.a22",  0x400001, 0x100000,  CRC(678b9052) SHA1(ae8fc921813e53e9dbc3960e772c1c4de94c22a7) )
	ROM_LOAD32_BYTE( "mbm-11.a20",  0x400002, 0x100000,  CRC(8a2a2a9f) SHA1(d11e0ea2785e35123bc56a8f18ce22f58432b599) )
	ROM_LOAD32_BYTE( "mbm-09.a16",  0x400003, 0x100000,  CRC(2cce162f) SHA1(db5795465a36971861e8fb7436db0805717ad101) )

	ROM_REGION32_LE( 0x1000000, "dvi", ROMREGION_ERASE00 ) // Video data - same as Dragongun, probably leftover from a conversion
//  ROM_LOAD( "mar-17.bin",  0x00000,  0x100000,  CRC(7799ed23) SHA1(ae28ad4fa6033a3695fa83356701b3774b26e6b0) )
//  ROM_LOAD( "mar-18.bin",  0x00000,  0x100000,  CRC(ded66da9) SHA1(5134cb47043cc190a35ebdbf1912166669f9c055) )
//  ROM_LOAD( "mar-19.bin",  0x00000,  0x100000,  CRC(bdd1ed20) SHA1(2435b23210b8fee4d39c30d4d3c6ea40afaa3b93) )
//  ROM_LOAD( "mar-20.bin",  0x00000,  0x100000,  CRC(fa0462f0) SHA1(1a52617ad4d7abebc0f273dd979f4cf2d6a0306b) )
//  ROM_LOAD( "mar-21.bin",  0x00000,  0x100000,  CRC(2d0a28ae) SHA1(d87f6f71bb76880e4d4f1eab8e0451b5c3df69a5) )
//  ROM_LOAD( "mar-22.bin",  0x00000,  0x100000,  CRC(c85f3559) SHA1(a5d5cf9b18c9ef6a92d7643ca1ec9052de0d4a01) )
//  ROM_LOAD( "mar-23.bin",  0x00000,  0x100000,  CRC(ba907d6a) SHA1(1fd99b66e6297c8d927c1cf723a613b4ee2e2f90) )
//  ROM_LOAD( "mar-24.bin",  0x00000,  0x100000,  CRC(5cec45c8) SHA1(f99a26afaca9d9320477e469b09e3873bc8c156f) )
//  ROM_LOAD( "mar-25.bin",  0x00000,  0x100000,  CRC(d65d895c) SHA1(4508dfff95a7aff5109dc74622cbb4503b0b5840) )
//  ROM_LOAD( "mar-26.bin",  0x00000,  0x100000,  CRC(246a06c5) SHA1(447252be976a5059925f4ad98df8564b70198f62) )
//  ROM_LOAD( "mar-27.bin",  0x00000,  0x100000,  CRC(3fcbd10f) SHA1(70fc7b88bbe35bbae1de14364b03d0a06d541de5) )
//  ROM_LOAD( "mar-28.bin",  0x00000,  0x100000,  CRC(5a2ec71d) SHA1(447c404e6bb696f7eb7c61992a99b9be56f5d6b0) )

	// not sure why the IC positions are swapped compared to Dragon Gun
	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbm-07.n21",  0x00000, 0x80000,  CRC(414f3793) SHA1(ed5f63e57390d503193fd1e9f7294ae1da6d3539) )

	ROM_REGION(0x100000, "oki2", ROMREGION_ERASE00 )
	ROM_LOAD( "mbm-06.n17",  0x00000, 0x100000,  CRC(f34d5999) SHA1(265b5f4e8598bcf9183bf9bd95db69b01536acb2) )

	ROM_REGION(0x80000, "oki3", ROMREGION_ERASE00 )
	ROM_LOAD( "mar-07.n19",  0x00000, 0x80000,  CRC(40287d62) SHA1(c00cb08bcdae55bcddc14c38e88b0484b1bc9e3e) )  // same as dragngun, unused
ROM_END

ROM_START( tattass )
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_WORD( "pp44.cpu", 0x000000, 0x80000, CRC(c3ca5b49) SHA1(c6420b0c20df1ae166b279504880ade65b1d8048) )
	ROM_LOAD32_WORD( "pp45.cpu", 0x000002, 0x80000, CRC(d3f30de0) SHA1(5a0aa0f96d29299b3b337b4b51bc84e447eb74d0) )

	ROM_REGION(0x10000, "decobsmt:soundcpu", 0 ) // Sound CPU
	ROM_LOAD( "u7.snd",  0x00000, 0x10000,  CRC(a7228077) SHA1(4d01575e51958a4d4686968008f5ed0a46380d70) )

	ROM_REGION( 0x200000, "tiles1", 0 )
	ROM_LOAD16_BYTE( "abak_b01.s02",  0x000000, 0x80000,  CRC(bc805680) SHA1(ccdbca23fc843ef82a3524020999542f43b3c618) )
	ROM_LOAD16_BYTE( "abak_b01.s13",  0x000001, 0x80000,  CRC(350effcd) SHA1(0452d95be9fc28bd00d846a2cc5828899d69601e) )
	ROM_LOAD16_BYTE( "abak_b23.s02",  0x100000, 0x80000,  CRC(91abdc21) SHA1(ba08e59bc0417e863d35ea295cf58cfe8faf57b5) )
	ROM_LOAD16_BYTE( "abak_b23.s13",  0x100001, 0x80000,  CRC(80eb50fe) SHA1(abfe1a5417ceff9d6d52372d11993bf9b1db9432) )

	ROM_REGION( 0x200000, "tiles2", 0 )
	ROM_LOAD16_BYTE( "bbak_b01.s02",  0x000000, 0x80000,  CRC(611be9a6) SHA1(86263c8beb562e0607a65aa30fbbe030a044cd75) )
	ROM_LOAD16_BYTE( "bbak_b01.s13",  0x000001, 0x80000,  CRC(097e0604) SHA1(6ae241b37b6bb15fc66679cf66f500b8f8a19f44) )
	ROM_LOAD16_BYTE( "bbak_b23.s02",  0x100000, 0x80000,  CRC(3836531a) SHA1(57bead820ac396ee0ed8fb2ac5c15929896d75bf) )
	ROM_LOAD16_BYTE( "bbak_b23.s13",  0x100001, 0x80000,  CRC(1210485a) SHA1(9edc4c96f389e231066ef164a7b2851cd7ade038) )

	ROM_REGION( 0xa00000, "sprites1", 0 )
	ROM_LOAD40_BYTE( "ob1_c0.b0",  0x000004, 0x80000,  CRC(053fecca) SHA1(319efc71042238d9012d2c3dddab9d11205decc6) )
	ROM_LOAD40_BYTE( "ob1_c1.b0",  0x000003, 0x80000,  CRC(e183e6bc) SHA1(d9cce277861967f403a882879e1baefa84696bdc) )
	ROM_LOAD40_BYTE( "ob1_c2.b0",  0x000002, 0x80000,  CRC(1314f828) SHA1(6a91543d4e70af30de287ba775c69ffb1cde719d) )
	ROM_LOAD40_BYTE( "ob1_c3.b0",  0x000001, 0x80000,  CRC(c63866df) SHA1(a897835d8a33002f1bd54f27d1a6393c4e1864b9) )
	ROM_LOAD40_BYTE( "ob1_c4.b0",  0x000000, 0x80000,  CRC(f71cdd1b) SHA1(6fbccdbe460c8ddfeed972ebe766a6f8a2d4c466) )

	ROM_LOAD40_BYTE( "ob1_c0.b1",  0x280004, 0x80000,  CRC(385434b0) SHA1(ea764bd9844e13f5b10207022135dbe07bf0258a) )
	ROM_LOAD40_BYTE( "ob1_c1.b1",  0x280003, 0x80000,  CRC(0a3ec489) SHA1(1a2e1252d6acda43019ded5a31ae60bef40e4bd9) )
	ROM_LOAD40_BYTE( "ob1_c2.b1",  0x280002, 0x80000,  CRC(52f06081) SHA1(c630f45b110b9423dfb0bf92359fdb28b75c8cf1) )
	ROM_LOAD40_BYTE( "ob1_c3.b1",  0x280001, 0x80000,  CRC(a8a5cfbe) SHA1(7afc8f7c7f3826a276e4840e4fc8b8bb645dd3bd) )
	ROM_LOAD40_BYTE( "ob1_c4.b1",  0x280000, 0x80000,  CRC(09d0acd6) SHA1(1b162f5b76852e49ae6a24db2031d66ca59d87e9) )

	ROM_LOAD40_BYTE( "ob1_c0.b2",  0x500004, 0x80000,  CRC(946e9f59) SHA1(46a0d35641b381fe553caa00451c30f1950b5dfd) )
	ROM_LOAD40_BYTE( "ob1_c1.b2",  0x500003, 0x80000,  CRC(9f66ad54) SHA1(6e6ac6edee2f2dda46e7cd85db8d79c8335c73cd) )
	ROM_LOAD40_BYTE( "ob1_c2.b2",  0x500002, 0x80000,  CRC(a8df60eb) SHA1(c971e66eec6accccaf2bdd87dde7adde79322da9) )
	ROM_LOAD40_BYTE( "ob1_c3.b2",  0x500001, 0x80000,  CRC(a1a753be) SHA1(1666a32bb69db36dba029a835592d00a21ad8c5e) )
	ROM_LOAD40_BYTE( "ob1_c4.b2",  0x500000, 0x80000,  CRC(b65b3c4b) SHA1(f636a682b506e3ce5ca07ba8fd3166158d1ab667) )

	ROM_LOAD40_BYTE( "ob1_c0.b3",  0x780004, 0x80000,  CRC(cbbbc696) SHA1(6f2383655461ac35f3178e0f7c0146cff89c8295) )
	ROM_LOAD40_BYTE( "ob1_c1.b3",  0x780003, 0x80000,  CRC(f7b1bdee) SHA1(1d505d8d4ede55246de0b5fbc6ca20f836699b60) )
	ROM_LOAD40_BYTE( "ob1_c2.b3",  0x780002, 0x80000,  CRC(97815619) SHA1(b1b694310064971aa5438671d0f9992b7e4bf277) )
	ROM_LOAD40_BYTE( "ob1_c3.b3",  0x780001, 0x80000,  CRC(fc3ccb7a) SHA1(4436fcbd830912589bd6c838eb63b7d41a2bb56e) )
	ROM_LOAD40_BYTE( "ob1_c4.b3",  0x780000, 0x80000,  CRC(dfdfd0ff) SHA1(79dc686351d41d635359936efe97c7ade305dc84) )

	ROM_REGION( 0x800000, "sprites2", 0 )
	ROM_LOAD16_BYTE( "ob2_c0.b0",  0x000000, 0x80000,  CRC(9080ebe4) SHA1(1cfabe045532e16f203fe054449149451a280f56) )
	ROM_LOAD16_BYTE( "ob2_c1.b0",  0x000001, 0x80000,  CRC(c0464970) SHA1(2bd87c9a7ed0742a8f1ee0c0de225e18a0351168) )
	ROM_LOAD16_BYTE( "ob2_c2.b0",  0x400000, 0x80000,  CRC(35a2e621) SHA1(ff7687e30c379cbcee4f80c0c737cef891509881) )
	ROM_LOAD16_BYTE( "ob2_c3.b0",  0x400001, 0x80000,  CRC(99c7cc2d) SHA1(c761e5b7f1e2afdafef36390f7141ebcb5e8f254) )
	ROM_LOAD16_BYTE( "ob2_c0.b1",  0x100000, 0x80000,  CRC(2c2c15c9) SHA1(fdc48fab6dad97d16d4e77479fa77bb320eb3767) )
	ROM_LOAD16_BYTE( "ob2_c1.b1",  0x100001, 0x80000,  CRC(d2c49a14) SHA1(49d92233d6d5f77fbbf9d31607c568efef6d94f0) )
	ROM_LOAD16_BYTE( "ob2_c2.b1",  0x500000, 0x80000,  CRC(fbe957e8) SHA1(4f0bb0e434771316bcd8796878ffd3e5cafebb2b) )
	ROM_LOAD16_BYTE( "ob2_c3.b1",  0x500001, 0x80000,  CRC(d7238829) SHA1(6fef08a518be69251852d3204413b4b8b6615be2) )
	ROM_LOAD16_BYTE( "ob2_c0.b2",  0x200000, 0x80000,  CRC(aefa1b01) SHA1(bbd4b432b36d64f80065c56559ea9675acf3151e) )
	ROM_LOAD16_BYTE( "ob2_c1.b2",  0x200001, 0x80000,  CRC(4af620ca) SHA1(f3753235b2e72f011c9b94f26a425b9a79577201) )
	ROM_LOAD16_BYTE( "ob2_c2.b2",  0x600000, 0x80000,  CRC(8e58be07) SHA1(d8a8662e800da0892d70c628de0ca27ff983006c) )
	ROM_LOAD16_BYTE( "ob2_c3.b2",  0x600001, 0x80000,  CRC(1b5188c5) SHA1(4792a36b889a2c2dfab9ec78d848d3d8bf10d20f) )
	ROM_LOAD16_BYTE( "ob2_c0.b3",  0x300000, 0x80000,  CRC(a2a5dafd) SHA1(2baadcfe9ae8fa30ae4226caa10fe3d58f8af3e0) )
	ROM_LOAD16_BYTE( "ob2_c1.b3",  0x300001, 0x80000,  CRC(6f0afd05) SHA1(6a4bf3466a77d14b3bc18377537f86108774badd) )
	ROM_LOAD16_BYTE( "ob2_c2.b3",  0x700000, 0x80000,  CRC(90fe5f4f) SHA1(2149e9eae152556c632ebd4d0b2de49e40916a77) )
	ROM_LOAD16_BYTE( "ob2_c3.b3",  0x700001, 0x80000,  CRC(e3517e6e) SHA1(68ac60570423d8f0d7cff3db1901c9c050d0be91) )

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD( "u17.snd",  0x000000, 0x80000,  CRC(49303539) SHA1(3c4b6bbdcb039e355b66f72ed5ffe0bbd363f179) )
	ROM_LOAD( "u21.snd",  0x080000, 0x80000,  CRC(0ad8bc18) SHA1(b06654e682080b3baeb7872d3c4eb3a04b22d79b) )
	ROM_LOAD( "u36.snd",  0x100000, 0x80000,  CRC(f558947b) SHA1(57c0dc2e23beb388c1cf10a175be7dabf07da458) )
	ROM_LOAD( "u37.snd",  0x180000, 0x80000,  CRC(7a3190bc) SHA1(e78b77268e5aa2879e7dcb5e8ff9e5dbed228cb1) )

	ROM_REGION( 0x400, "eeprom", 0 )
	ROM_LOAD( "eeprom-tattass.bin", 0x0000, 0x0400, CRC(7140f40c) SHA1(4fb7897933046b6adaf00b4626d5fd23d0e8a666) )
ROM_END

ROM_START( tattasso )
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_WORD( "pp44.cpu", 0x000000, 0x80000, CRC(c3ca5b49) SHA1(c6420b0c20df1ae166b279504880ade65b1d8048) )
	ROM_LOAD32_WORD( "pp45.cpu", 0x000002, 0x80000, CRC(d3f30de0) SHA1(5a0aa0f96d29299b3b337b4b51bc84e447eb74d0) )

	ROM_REGION(0x10000, "decobsmt:soundcpu", 0 ) // Sound CPU
	ROM_LOAD( "u7.snd",  0x00000, 0x10000,  CRC(6947be8a) SHA1(4ac6c3c7f54501f23c434708cea6bf327bc8cf95) )

	ROM_REGION( 0x200000, "tiles1", 0 )
	ROM_LOAD16_BYTE( "abak_b01.s02",  0x000000, 0x80000,  CRC(bc805680) SHA1(ccdbca23fc843ef82a3524020999542f43b3c618) )
	ROM_LOAD16_BYTE( "abak_b01.s13",  0x000001, 0x80000,  CRC(350effcd) SHA1(0452d95be9fc28bd00d846a2cc5828899d69601e) )
	ROM_LOAD16_BYTE( "abak_b23.s02",  0x100000, 0x80000,  CRC(91abdc21) SHA1(ba08e59bc0417e863d35ea295cf58cfe8faf57b5) )
	ROM_LOAD16_BYTE( "abak_b23.s13",  0x100001, 0x80000,  CRC(80eb50fe) SHA1(abfe1a5417ceff9d6d52372d11993bf9b1db9432) )

	ROM_REGION( 0x200000, "tiles2", 0 )
	ROM_LOAD16_BYTE( "bbak_b01.s02",  0x000000, 0x80000,  CRC(611be9a6) SHA1(86263c8beb562e0607a65aa30fbbe030a044cd75) )
	ROM_LOAD16_BYTE( "bbak_b01.s13",  0x000001, 0x80000,  CRC(097e0604) SHA1(6ae241b37b6bb15fc66679cf66f500b8f8a19f44) )
	ROM_LOAD16_BYTE( "bbak_b23.s02",  0x100000, 0x80000,  CRC(3836531a) SHA1(57bead820ac396ee0ed8fb2ac5c15929896d75bf) )
	ROM_LOAD16_BYTE( "bbak_b23.s13",  0x100001, 0x80000,  CRC(1210485a) SHA1(9edc4c96f389e231066ef164a7b2851cd7ade038) )

	ROM_REGION( 0xa00000, "sprites1", 0 )
	ROM_LOAD40_BYTE( "ob1_c0.b0",  0x000004, 0x80000,  CRC(053fecca) SHA1(319efc71042238d9012d2c3dddab9d11205decc6) )
	ROM_LOAD40_BYTE( "ob1_c1.b0",  0x000003, 0x80000,  CRC(e183e6bc) SHA1(d9cce277861967f403a882879e1baefa84696bdc) )
	ROM_LOAD40_BYTE( "ob1_c2.b0",  0x000002, 0x80000,  CRC(1314f828) SHA1(6a91543d4e70af30de287ba775c69ffb1cde719d) )
	ROM_LOAD40_BYTE( "ob1_c3.b0",  0x000001, 0x80000,  CRC(c63866df) SHA1(a897835d8a33002f1bd54f27d1a6393c4e1864b9) )
	ROM_LOAD40_BYTE( "ob1_c4.b0",  0x000000, 0x80000,  CRC(f71cdd1b) SHA1(6fbccdbe460c8ddfeed972ebe766a6f8a2d4c466) )

	ROM_LOAD40_BYTE( "ob1_c0.b1",  0x280004, 0x80000,  CRC(385434b0) SHA1(ea764bd9844e13f5b10207022135dbe07bf0258a) )
	ROM_LOAD40_BYTE( "ob1_c1.b1",  0x280003, 0x80000,  CRC(0a3ec489) SHA1(1a2e1252d6acda43019ded5a31ae60bef40e4bd9) )
	ROM_LOAD40_BYTE( "ob1_c2.b1",  0x280002, 0x80000,  CRC(52f06081) SHA1(c630f45b110b9423dfb0bf92359fdb28b75c8cf1) )
	ROM_LOAD40_BYTE( "ob1_c3.b1",  0x280001, 0x80000,  CRC(a8a5cfbe) SHA1(7afc8f7c7f3826a276e4840e4fc8b8bb645dd3bd) )
	ROM_LOAD40_BYTE( "ob1_c4.b1",  0x280000, 0x80000,  CRC(09d0acd6) SHA1(1b162f5b76852e49ae6a24db2031d66ca59d87e9) )

	ROM_LOAD40_BYTE( "ob1_c0.b2",  0x500004, 0x80000,  CRC(946e9f59) SHA1(46a0d35641b381fe553caa00451c30f1950b5dfd) )
	ROM_LOAD40_BYTE( "ob1_c1.b2",  0x500003, 0x80000,  CRC(9f66ad54) SHA1(6e6ac6edee2f2dda46e7cd85db8d79c8335c73cd) )
	ROM_LOAD40_BYTE( "ob1_c2.b2",  0x500002, 0x80000,  CRC(a8df60eb) SHA1(c971e66eec6accccaf2bdd87dde7adde79322da9) )
	ROM_LOAD40_BYTE( "ob1_c3.b2",  0x500001, 0x80000,  CRC(a1a753be) SHA1(1666a32bb69db36dba029a835592d00a21ad8c5e) )
	ROM_LOAD40_BYTE( "ob1_c4.b2",  0x500000, 0x80000,  CRC(b65b3c4b) SHA1(f636a682b506e3ce5ca07ba8fd3166158d1ab667) )

	ROM_LOAD40_BYTE( "ob1_c0.b3",  0x780004, 0x80000,  CRC(cbbbc696) SHA1(6f2383655461ac35f3178e0f7c0146cff89c8295) )
	ROM_LOAD40_BYTE( "ob1_c1.b3",  0x780003, 0x80000,  CRC(f7b1bdee) SHA1(1d505d8d4ede55246de0b5fbc6ca20f836699b60) )
	ROM_LOAD40_BYTE( "ob1_c2.b3",  0x780002, 0x80000,  CRC(97815619) SHA1(b1b694310064971aa5438671d0f9992b7e4bf277) )
	ROM_LOAD40_BYTE( "ob1_c3.b3",  0x780001, 0x80000,  CRC(fc3ccb7a) SHA1(4436fcbd830912589bd6c838eb63b7d41a2bb56e) )
	ROM_LOAD40_BYTE( "ob1_c4.b3",  0x780000, 0x80000,  CRC(dfdfd0ff) SHA1(79dc686351d41d635359936efe97c7ade305dc84) )

	ROM_REGION( 0x800000, "sprites2", 0 )
	ROM_LOAD16_BYTE( "ob2_c0.b0",  0x000000, 0x80000,  CRC(9080ebe4) SHA1(1cfabe045532e16f203fe054449149451a280f56) )
	ROM_LOAD16_BYTE( "ob2_c1.b0",  0x000001, 0x80000,  CRC(c0464970) SHA1(2bd87c9a7ed0742a8f1ee0c0de225e18a0351168) )
	ROM_LOAD16_BYTE( "ob2_c2.b0",  0x400000, 0x80000,  CRC(35a2e621) SHA1(ff7687e30c379cbcee4f80c0c737cef891509881) )
	ROM_LOAD16_BYTE( "ob2_c3.b0",  0x400001, 0x80000,  CRC(99c7cc2d) SHA1(c761e5b7f1e2afdafef36390f7141ebcb5e8f254) )
	ROM_LOAD16_BYTE( "ob2_c0.b1",  0x100000, 0x80000,  CRC(2c2c15c9) SHA1(fdc48fab6dad97d16d4e77479fa77bb320eb3767) )
	ROM_LOAD16_BYTE( "ob2_c1.b1",  0x100001, 0x80000,  CRC(d2c49a14) SHA1(49d92233d6d5f77fbbf9d31607c568efef6d94f0) )
	ROM_LOAD16_BYTE( "ob2_c2.b1",  0x500000, 0x80000,  CRC(fbe957e8) SHA1(4f0bb0e434771316bcd8796878ffd3e5cafebb2b) )
	ROM_LOAD16_BYTE( "ob2_c3.b1",  0x500001, 0x80000,  CRC(d7238829) SHA1(6fef08a518be69251852d3204413b4b8b6615be2) )
	ROM_LOAD16_BYTE( "ob2_c0.b2",  0x200000, 0x80000,  CRC(aefa1b01) SHA1(bbd4b432b36d64f80065c56559ea9675acf3151e) )
	ROM_LOAD16_BYTE( "ob2_c1.b2",  0x200001, 0x80000,  CRC(4af620ca) SHA1(f3753235b2e72f011c9b94f26a425b9a79577201) )
	ROM_LOAD16_BYTE( "ob2_c2.b2",  0x600000, 0x80000,  CRC(8e58be07) SHA1(d8a8662e800da0892d70c628de0ca27ff983006c) )
	ROM_LOAD16_BYTE( "ob2_c3.b2",  0x600001, 0x80000,  CRC(1b5188c5) SHA1(4792a36b889a2c2dfab9ec78d848d3d8bf10d20f) )
	ROM_LOAD16_BYTE( "ob2_c0.b3",  0x300000, 0x80000,  CRC(a2a5dafd) SHA1(2baadcfe9ae8fa30ae4226caa10fe3d58f8af3e0) )
	ROM_LOAD16_BYTE( "ob2_c1.b3",  0x300001, 0x80000,  CRC(6f0afd05) SHA1(6a4bf3466a77d14b3bc18377537f86108774badd) )
	ROM_LOAD16_BYTE( "ob2_c2.b3",  0x700000, 0x80000,  CRC(90fe5f4f) SHA1(2149e9eae152556c632ebd4d0b2de49e40916a77) )
	ROM_LOAD16_BYTE( "ob2_c3.b3",  0x700001, 0x80000,  CRC(e3517e6e) SHA1(68ac60570423d8f0d7cff3db1901c9c050d0be91) )

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD( "u17.snd",  0x000000, 0x80000,  CRC(b945c18d) SHA1(6556bbb4a7057df3680132f24687fa944006c784) )
	ROM_LOAD( "u21.snd",  0x080000, 0x80000,  CRC(10b2110c) SHA1(83e5938ed22da2874022e1dc8df76c72d95c448d) )
	ROM_LOAD( "u36.snd",  0x100000, 0x80000,  CRC(3b73abe2) SHA1(195096e2302e84123b23b4ccd982fb3ab9afe42c) )
	ROM_LOAD( "u37.snd",  0x180000, 0x80000,  CRC(986066b5) SHA1(9dd1a14de81733617cf51293674a8e26fc5cec68) )

	ROM_REGION( 0x400, "eeprom", 0 )
	ROM_LOAD( "eeprom-tattass.bin", 0x0000, 0x0400, CRC(7140f40c) SHA1(4fb7897933046b6adaf00b4626d5fd23d0e8a666) )
ROM_END

ROM_START( tattassa )
	ROM_REGION(0x100000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_WORD( "rev232a.000", 0x000000, 0x80000, CRC(1a357112) SHA1(d7f78f90970fd56ca1452a4c138168568b06d868) )
	ROM_LOAD32_WORD( "rev232a.001", 0x000002, 0x80000, CRC(550245d4) SHA1(c1b2b31768da9becebd907a8622d05aa68ecaa29) )

	ROM_REGION(0x10000, "decobsmt:soundcpu", 0 ) // Sound CPU
	ROM_LOAD( "u7.snd",  0x00000, 0x10000,  CRC(a7228077) SHA1(4d01575e51958a4d4686968008f5ed0a46380d70) )

	ROM_REGION( 0x200000, "tiles1", 0 )
	ROM_LOAD16_BYTE( "abak_b01.s02",  0x000000, 0x80000,  CRC(bc805680) SHA1(ccdbca23fc843ef82a3524020999542f43b3c618) )
	ROM_LOAD16_BYTE( "abak_b01.s13",  0x000001, 0x80000,  CRC(350effcd) SHA1(0452d95be9fc28bd00d846a2cc5828899d69601e) )
	ROM_LOAD16_BYTE( "abak_b23.s02",  0x100000, 0x80000,  CRC(91abdc21) SHA1(ba08e59bc0417e863d35ea295cf58cfe8faf57b5) )
	ROM_LOAD16_BYTE( "abak_b23.s13",  0x100001, 0x80000,  CRC(80eb50fe) SHA1(abfe1a5417ceff9d6d52372d11993bf9b1db9432) )

	ROM_REGION( 0x200000, "tiles2", 0 )
	ROM_LOAD16_BYTE( "bbak_b01.s02",  0x000000, 0x80000,  CRC(611be9a6) SHA1(86263c8beb562e0607a65aa30fbbe030a044cd75) )
	ROM_LOAD16_BYTE( "bbak_b01.s13",  0x000001, 0x80000,  CRC(097e0604) SHA1(6ae241b37b6bb15fc66679cf66f500b8f8a19f44) )
	ROM_LOAD16_BYTE( "bbak_b23.s02",  0x100000, 0x80000,  CRC(3836531a) SHA1(57bead820ac396ee0ed8fb2ac5c15929896d75bf) )
	ROM_LOAD16_BYTE( "bbak_b23.s13",  0x100001, 0x80000,  CRC(1210485a) SHA1(9edc4c96f389e231066ef164a7b2851cd7ade038) )

	ROM_REGION( 0xa00000, "sprites1", 0 )
	ROM_LOAD40_BYTE( "ob1_c0.b0",  0x000004, 0x80000,  CRC(053fecca) SHA1(319efc71042238d9012d2c3dddab9d11205decc6) )
	ROM_LOAD40_BYTE( "ob1_c1.b0",  0x000003, 0x80000,  CRC(e183e6bc) SHA1(d9cce277861967f403a882879e1baefa84696bdc) )
	ROM_LOAD40_BYTE( "ob1_c2.b0",  0x000002, 0x80000,  CRC(1314f828) SHA1(6a91543d4e70af30de287ba775c69ffb1cde719d) )
	ROM_LOAD40_BYTE( "ob1_c3.b0",  0x000001, 0x80000,  CRC(c63866df) SHA1(a897835d8a33002f1bd54f27d1a6393c4e1864b9) )
	ROM_LOAD40_BYTE( "ob1_c4.b0",  0x000000, 0x80000,  CRC(f71cdd1b) SHA1(6fbccdbe460c8ddfeed972ebe766a6f8a2d4c466) )

	ROM_LOAD40_BYTE( "ob1_c0.b1",  0x280004, 0x80000,  CRC(385434b0) SHA1(ea764bd9844e13f5b10207022135dbe07bf0258a) )
	ROM_LOAD40_BYTE( "ob1_c1.b1",  0x280003, 0x80000,  CRC(0a3ec489) SHA1(1a2e1252d6acda43019ded5a31ae60bef40e4bd9) )
	ROM_LOAD40_BYTE( "ob1_c2.b1",  0x280002, 0x80000,  CRC(52f06081) SHA1(c630f45b110b9423dfb0bf92359fdb28b75c8cf1) )
	ROM_LOAD40_BYTE( "ob1_c3.b1",  0x280001, 0x80000,  CRC(a8a5cfbe) SHA1(7afc8f7c7f3826a276e4840e4fc8b8bb645dd3bd) )
	ROM_LOAD40_BYTE( "ob1_c4.b1",  0x280000, 0x80000,  CRC(09d0acd6) SHA1(1b162f5b76852e49ae6a24db2031d66ca59d87e9) )

	ROM_LOAD40_BYTE( "ob1_c0.b2",  0x500004, 0x80000,  CRC(946e9f59) SHA1(46a0d35641b381fe553caa00451c30f1950b5dfd) )
	ROM_LOAD40_BYTE( "ob1_c1.b2",  0x500003, 0x80000,  CRC(9f66ad54) SHA1(6e6ac6edee2f2dda46e7cd85db8d79c8335c73cd) )
	ROM_LOAD40_BYTE( "ob1_c2.b2",  0x500002, 0x80000,  CRC(a8df60eb) SHA1(c971e66eec6accccaf2bdd87dde7adde79322da9) )
	ROM_LOAD40_BYTE( "ob1_c3.b2",  0x500001, 0x80000,  CRC(a1a753be) SHA1(1666a32bb69db36dba029a835592d00a21ad8c5e) )
	ROM_LOAD40_BYTE( "ob1_c4.b2",  0x500000, 0x80000,  CRC(b65b3c4b) SHA1(f636a682b506e3ce5ca07ba8fd3166158d1ab667) )

	ROM_LOAD40_BYTE( "ob1_c0.b3",  0x780004, 0x80000,  CRC(cbbbc696) SHA1(6f2383655461ac35f3178e0f7c0146cff89c8295) )
	ROM_LOAD40_BYTE( "ob1_c1.b3",  0x780003, 0x80000,  CRC(f7b1bdee) SHA1(1d505d8d4ede55246de0b5fbc6ca20f836699b60) )
	ROM_LOAD40_BYTE( "ob1_c2.b3",  0x780002, 0x80000,  CRC(97815619) SHA1(b1b694310064971aa5438671d0f9992b7e4bf277) )
	ROM_LOAD40_BYTE( "ob1_c3.b3",  0x780001, 0x80000,  CRC(fc3ccb7a) SHA1(4436fcbd830912589bd6c838eb63b7d41a2bb56e) )
	ROM_LOAD40_BYTE( "ob1_c4.b3",  0x780000, 0x80000,  CRC(dfdfd0ff) SHA1(79dc686351d41d635359936efe97c7ade305dc84) )

	ROM_REGION( 0x800000, "sprites2", 0 )
	ROM_LOAD16_BYTE( "ob2_c0.b0",  0x000000, 0x80000,  CRC(9080ebe4) SHA1(1cfabe045532e16f203fe054449149451a280f56) )
	ROM_LOAD16_BYTE( "ob2_c1.b0",  0x000001, 0x80000,  CRC(c0464970) SHA1(2bd87c9a7ed0742a8f1ee0c0de225e18a0351168) )
	ROM_LOAD16_BYTE( "ob2_c2.b0",  0x400000, 0x80000,  CRC(35a2e621) SHA1(ff7687e30c379cbcee4f80c0c737cef891509881) )
	ROM_LOAD16_BYTE( "ob2_c3.b0",  0x400001, 0x80000,  CRC(99c7cc2d) SHA1(c761e5b7f1e2afdafef36390f7141ebcb5e8f254) )
	ROM_LOAD16_BYTE( "ob2_c0.b1",  0x100000, 0x80000,  CRC(2c2c15c9) SHA1(fdc48fab6dad97d16d4e77479fa77bb320eb3767) )
	ROM_LOAD16_BYTE( "ob2_c1.b1",  0x100001, 0x80000,  CRC(d2c49a14) SHA1(49d92233d6d5f77fbbf9d31607c568efef6d94f0) )
	ROM_LOAD16_BYTE( "ob2_c2.b1",  0x500000, 0x80000,  CRC(fbe957e8) SHA1(4f0bb0e434771316bcd8796878ffd3e5cafebb2b) )
	ROM_LOAD16_BYTE( "ob2_c3.b1",  0x500001, 0x80000,  CRC(d7238829) SHA1(6fef08a518be69251852d3204413b4b8b6615be2) )
	ROM_LOAD16_BYTE( "ob2_c0.b2",  0x200000, 0x80000,  CRC(aefa1b01) SHA1(bbd4b432b36d64f80065c56559ea9675acf3151e) )
	ROM_LOAD16_BYTE( "ob2_c1.b2",  0x200001, 0x80000,  CRC(4af620ca) SHA1(f3753235b2e72f011c9b94f26a425b9a79577201) )
	ROM_LOAD16_BYTE( "ob2_c2.b2",  0x600000, 0x80000,  CRC(8e58be07) SHA1(d8a8662e800da0892d70c628de0ca27ff983006c) )
	ROM_LOAD16_BYTE( "ob2_c3.b2",  0x600001, 0x80000,  CRC(1b5188c5) SHA1(4792a36b889a2c2dfab9ec78d848d3d8bf10d20f) )
	ROM_LOAD16_BYTE( "ob2_c0.b3",  0x300000, 0x80000,  CRC(a2a5dafd) SHA1(2baadcfe9ae8fa30ae4226caa10fe3d58f8af3e0) )
	ROM_LOAD16_BYTE( "ob2_c1.b3",  0x300001, 0x80000,  CRC(6f0afd05) SHA1(6a4bf3466a77d14b3bc18377537f86108774badd) )
	ROM_LOAD16_BYTE( "ob2_c2.b3",  0x700000, 0x80000,  CRC(90fe5f4f) SHA1(2149e9eae152556c632ebd4d0b2de49e40916a77) )
	ROM_LOAD16_BYTE( "ob2_c3.b3",  0x700001, 0x80000,  CRC(e3517e6e) SHA1(68ac60570423d8f0d7cff3db1901c9c050d0be91) )

	ROM_REGION(0x1000000, "decobsmt:bsmt", 0 )
	ROM_LOAD( "u17.snd",  0x000000, 0x80000,  CRC(49303539) SHA1(3c4b6bbdcb039e355b66f72ed5ffe0bbd363f179) )
	ROM_LOAD( "u21.snd",  0x080000, 0x80000,  CRC(0ad8bc18) SHA1(b06654e682080b3baeb7872d3c4eb3a04b22d79b) )
	ROM_LOAD( "u36.snd",  0x100000, 0x80000,  CRC(f558947b) SHA1(57c0dc2e23beb388c1cf10a175be7dabf07da458) )
	ROM_LOAD( "u37.snd",  0x180000, 0x80000,  CRC(7a3190bc) SHA1(e78b77268e5aa2879e7dcb5e8ff9e5dbed228cb1) )

	ROM_REGION( 0x400, "eeprom", 0 )
	ROM_LOAD( "eeprom-tattass.bin", 0x0000, 0x0400, CRC(7140f40c) SHA1(4fb7897933046b6adaf00b4626d5fd23d0e8a666) )
ROM_END

ROM_START( nslasher ) // DE-0397-0 PCB
	ROM_REGION(0x100000, "maincpu", 0 ) // Encrypted ARM 32 bit code
	ROM_LOAD32_WORD( "mainprg.1f", 0x000000, 0x80000, CRC(507acbae) SHA1(329a2bb244f2f3adb8d75cab5aa2dcb129d70712) )
	ROM_LOAD32_WORD( "mainprg.2f", 0x000002, 0x80000, CRC(931fc7ee) SHA1(54eb12abfa3f332ce9b43a45ec424aaee88641a6) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "sndprg.17l",  0x00000,  0x10000,  CRC(18939e92) SHA1(50b37a78d9d2259d4b140dd17393c4e5ca92bca5) )

	ROM_REGION( 0x200000, "tiles1", 0 )
	ROM_LOAD( "mbh-00.8c",  0x000000,  0x200000,  CRC(a877f8a3) SHA1(79253525f360a73161894f31e211e4d6b38d307a) ) // Encrypted tiles

	ROM_REGION( 0x200000, "tiles2", 0 )
	ROM_LOAD( "mbh-01.9c",  0x000000,  0x200000,  CRC(1853dafc) SHA1(b1183c0db301cbed9f079c782202dbfc553b198e) ) // Encrypted tiles

	ROM_REGION( 0x640000, "sprites1", 0 ) // Sprites
	ROM_LOAD40_WORD_SWAP( "mbh-02.14c",  0x000003,  0x200000, CRC(b2f158a1) SHA1(4f8c0b324813db198fe1dad7fff4185b828b94de) )
	ROM_LOAD40_WORD_SWAP( "mbh-04.16c",  0x000001,  0x200000, CRC(eecfe06d) SHA1(2df817fe5e2ea31207b217bb03dc58979c05d0d2) )
	ROM_LOAD40_BYTE(      "mbh-06.18c",  0x000000,  0x100000, CRC(038c2127) SHA1(5bdb215305f1a419fde27a83b623a38b9328e560) )
	ROM_LOAD40_WORD_SWAP( "mbh-03.15c",  0x500003,  0x080000, CRC(787787e3) SHA1(531444e3f28aa9a7539a5a76ca94a9d6b97274c5) )
	ROM_LOAD40_WORD_SWAP( "mbh-05.17c",  0x500001,  0x080000, CRC(1d2b7c17) SHA1(ae0b8e0448a1a8180fb424fb0bc8a4302f8ff602) )
	ROM_LOAD40_BYTE(      "mbh-07.19c",  0x500000,  0x040000, CRC(bbd22323) SHA1(6ab665b2e6d04cdadc48c52e15098e978b61fe10) )

	ROM_REGION( 0x100000, "sprites2", 0 ) // Sprites
	ROM_LOAD( "mbh-08.16e",  0x000000,  0x80000,  CRC(cdd7f8cb) SHA1(910bbe8783c0ba722e9d6399b332d658fa059fdb) )
	ROM_LOAD( "mbh-09.18e",  0x080000,  0x80000,  CRC(33fa2121) SHA1(eb0e99d29b1ad9995df28e5b7cfc89d53efb53c3) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbh-10.14l", 0x000000,  0x80000,  CRC(c4d6b116) SHA1(c5685bce6a6c6a74ca600ebf766ba1007f0dc666) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbh-11.16l", 0x000000,  0x80000,  CRC(0ec40b6b) SHA1(9fef44149608ae2a00f6a75a6f77f2efcab6e78e) )

	ROM_REGION(0x200, "prom", 0 )
	ROM_LOAD( "ln-00.j7", 0x000000,  0x200, CRC(5e83eaf3) SHA1(95f5eb8e56dff6c2dce7c39a6dd458bfc38fe1cf) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "vm-00.3d",  0x0000, 0x0117, NO_DUMP ) // 16L8ACN is read protected
	ROM_LOAD( "vm-01.4d",  0x0200, 0x0117, NO_DUMP ) // 16L8ACN is read protected
	ROM_LOAD( "vm-02.8j",  0x0400, 0x0117, CRC(53692426) SHA1(b8f8cf6b1f6b637fcd1fcd62474e637f5d4a6901) )
ROM_END

ROM_START( nslasherj ) // DE-0397-0 PCB
	ROM_REGION(0x100000, "maincpu", 0 ) // Encrypted ARM 32 bit code
	ROM_LOAD32_WORD( "lx-00.1f", 0x000000, 0x80000, CRC(6ed5fb88) SHA1(84350da7939a479968a523c84e254e3ee54b8da2) )
	ROM_LOAD32_WORD( "lx-01.2f", 0x000002, 0x80000, CRC(a6df2152) SHA1(6fe7e0b2e71c5f807951dcc81a6a3cff55247961) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "sndprg.17l",  0x00000,  0x10000,  CRC(18939e92) SHA1(50b37a78d9d2259d4b140dd17393c4e5ca92bca5) )

	ROM_REGION( 0x200000, "tiles1", 0 )
	ROM_LOAD( "mbh-00.8c",  0x000000,  0x200000,  CRC(a877f8a3) SHA1(79253525f360a73161894f31e211e4d6b38d307a) ) // Encrypted tiles

	ROM_REGION( 0x200000, "tiles2", 0 )
	ROM_LOAD( "mbh-01.9c",  0x000000,  0x200000,  CRC(1853dafc) SHA1(b1183c0db301cbed9f079c782202dbfc553b198e) ) // Encrypted tiles

	ROM_REGION( 0x640000, "sprites1", 0 ) // Sprites
	ROM_LOAD40_WORD_SWAP( "mbh-02.14c",  0x000003,  0x200000, CRC(b2f158a1) SHA1(4f8c0b324813db198fe1dad7fff4185b828b94de) )
	ROM_LOAD40_WORD_SWAP( "mbh-04.16c",  0x000001,  0x200000, CRC(eecfe06d) SHA1(2df817fe5e2ea31207b217bb03dc58979c05d0d2) )
	ROM_LOAD40_BYTE(      "mbh-06.18c",  0x000000,  0x100000, CRC(038c2127) SHA1(5bdb215305f1a419fde27a83b623a38b9328e560) )
	ROM_LOAD40_WORD_SWAP( "mbh-03.15c",  0x500003,  0x080000, CRC(787787e3) SHA1(531444e3f28aa9a7539a5a76ca94a9d6b97274c5) )
	ROM_LOAD40_WORD_SWAP( "mbh-05.17c",  0x500001,  0x080000, CRC(1d2b7c17) SHA1(ae0b8e0448a1a8180fb424fb0bc8a4302f8ff602) )
	ROM_LOAD40_BYTE(      "mbh-07.19c",  0x500000,  0x040000, CRC(bbd22323) SHA1(6ab665b2e6d04cdadc48c52e15098e978b61fe10) )

	ROM_REGION( 0x100000, "sprites2", 0 ) // Sprites
	ROM_LOAD( "mbh-08.16e",  0x000000,  0x80000,  CRC(cdd7f8cb) SHA1(910bbe8783c0ba722e9d6399b332d658fa059fdb) )
	ROM_LOAD( "mbh-09.18e",  0x080000,  0x80000,  CRC(33fa2121) SHA1(eb0e99d29b1ad9995df28e5b7cfc89d53efb53c3) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbh-10.14l", 0x000000,  0x80000,  CRC(c4d6b116) SHA1(c5685bce6a6c6a74ca600ebf766ba1007f0dc666) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbh-11.16l", 0x000000,  0x80000,  CRC(0ec40b6b) SHA1(9fef44149608ae2a00f6a75a6f77f2efcab6e78e) )

	ROM_REGION(0x200, "prom", 0 )
	ROM_LOAD( "ln-00.j7", 0x000000,  0x200, CRC(5e83eaf3) SHA1(95f5eb8e56dff6c2dce7c39a6dd458bfc38fe1cf) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "vm-00.3d",  0x0000, 0x0117, NO_DUMP ) // 16L8ACN is read protected
	ROM_LOAD( "vm-01.4d",  0x0200, 0x0117, NO_DUMP ) // 16L8ACN is read protected
	ROM_LOAD( "vm-02.8j",  0x0400, 0x0117, CRC(53692426) SHA1(b8f8cf6b1f6b637fcd1fcd62474e637f5d4a6901) )
ROM_END

ROM_START( nslashers ) // DE-0397-0 PCB
	ROM_REGION(0x100000, "maincpu", 0 ) // Encrypted ARM 32 bit code
	ROM_LOAD32_WORD( "ly-00.1f", 0x000000, 0x80000, CRC(fa0646f9) SHA1(7f9633bda230a0ced59171cdc5ab40a6d56c3d34) )
	ROM_LOAD32_WORD( "ly-01.2f", 0x000002, 0x80000, CRC(ae508149) SHA1(3592949e5fb2770adb9c9daa4e38c4e75f3e2554) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "sndprg.17l",  0x00000,  0x10000,  CRC(18939e92) SHA1(50b37a78d9d2259d4b140dd17393c4e5ca92bca5) )

	ROM_REGION( 0x200000, "tiles1", 0 )
	ROM_LOAD( "mbh-00.8c",  0x000000,  0x200000,  CRC(a877f8a3) SHA1(79253525f360a73161894f31e211e4d6b38d307a) ) // Encrypted tiles

	ROM_REGION( 0x200000, "tiles2", 0 )
	ROM_LOAD( "mbh-01.9c",  0x000000,  0x200000,  CRC(1853dafc) SHA1(b1183c0db301cbed9f079c782202dbfc553b198e) ) // Encrypted tiles

	ROM_REGION( 0x640000, "sprites1", 0 ) // Sprites
	ROM_LOAD40_WORD_SWAP( "mbh-02.14c",  0x000003,  0x200000, CRC(b2f158a1) SHA1(4f8c0b324813db198fe1dad7fff4185b828b94de) )
	ROM_LOAD40_WORD_SWAP( "mbh-04.16c",  0x000001,  0x200000, CRC(eecfe06d) SHA1(2df817fe5e2ea31207b217bb03dc58979c05d0d2) )
	ROM_LOAD40_BYTE(      "mbh-06.18c",  0x000000,  0x100000, CRC(038c2127) SHA1(5bdb215305f1a419fde27a83b623a38b9328e560) )
	ROM_LOAD40_WORD_SWAP( "mbh-03.15c",  0x500003,  0x080000, CRC(787787e3) SHA1(531444e3f28aa9a7539a5a76ca94a9d6b97274c5) )
	ROM_LOAD40_WORD_SWAP( "mbh-05.17c",  0x500001,  0x080000, CRC(1d2b7c17) SHA1(ae0b8e0448a1a8180fb424fb0bc8a4302f8ff602) )
	ROM_LOAD40_BYTE(      "mbh-07.19c",  0x500000,  0x040000, CRC(bbd22323) SHA1(6ab665b2e6d04cdadc48c52e15098e978b61fe10) )

	ROM_REGION( 0x100000, "sprites2", 0 ) // Sprites
	ROM_LOAD( "mbh-08.16e",  0x000000,  0x80000,  CRC(cdd7f8cb) SHA1(910bbe8783c0ba722e9d6399b332d658fa059fdb) )
	ROM_LOAD( "mbh-09.18e",  0x080000,  0x80000,  CRC(33fa2121) SHA1(eb0e99d29b1ad9995df28e5b7cfc89d53efb53c3) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbh-10.14l", 0x000000,  0x80000,  CRC(c4d6b116) SHA1(c5685bce6a6c6a74ca600ebf766ba1007f0dc666) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbh-11.16l", 0x000000,  0x80000,  CRC(0ec40b6b) SHA1(9fef44149608ae2a00f6a75a6f77f2efcab6e78e) )

	ROM_REGION(0x200, "prom", 0 )
	ROM_LOAD( "ln-00.j7", 0x000000,  0x200, CRC(5e83eaf3) SHA1(95f5eb8e56dff6c2dce7c39a6dd458bfc38fe1cf) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "vm-00.3d",  0x0000, 0x0117, NO_DUMP ) // 16L8ACN is read protected
	ROM_LOAD( "vm-01.4d",  0x0200, 0x0117, NO_DUMP ) // 16L8ACN is read protected
	ROM_LOAD( "vm-02.8j",  0x0400, 0x0117, CRC(53692426) SHA1(b8f8cf6b1f6b637fcd1fcd62474e637f5d4a6901) )
ROM_END

ROM_START( nslasheru ) // DE-0395-1 PCB
	ROM_REGION(0x100000, "maincpu", 0 ) // Encrypted ARM 32 bit code
	ROM_LOAD32_WORD( "00.f1", 0x000000, 0x80000, CRC(944f3329) SHA1(7e7909e203b9752de3d3d798c6f84ac6ae824a07) )
	ROM_LOAD32_WORD( "01.f2", 0x000002, 0x80000, CRC(ac12d18a) SHA1(7cd4e843bf575c70c5c39a8afa78b803106f59b0) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "02.l18",  0x00000,  0x10000, CRC(5e63bd91) SHA1(a6ac3c8c50f44cf2e6cf029aef1c974d1fc16ed5) )

	ROM_REGION( 0x200000, "tiles1", 0 )
	ROM_LOAD( "mbh-00.8c",  0x000000,  0x200000,  CRC(a877f8a3) SHA1(79253525f360a73161894f31e211e4d6b38d307a) ) // Encrypted tiles

	ROM_REGION( 0x200000, "tiles2", 0 )
	ROM_LOAD( "mbh-01.9c",  0x000000,  0x200000,  CRC(1853dafc) SHA1(b1183c0db301cbed9f079c782202dbfc553b198e) ) // Encrypted tiles

	ROM_REGION( 0x640000, "sprites1", 0 ) // Sprites
	ROM_LOAD40_WORD_SWAP( "mbh-02.14c",  0x000003,  0x200000, CRC(b2f158a1) SHA1(4f8c0b324813db198fe1dad7fff4185b828b94de) )
	ROM_LOAD40_WORD_SWAP( "mbh-04.16c",  0x000001,  0x200000, CRC(eecfe06d) SHA1(2df817fe5e2ea31207b217bb03dc58979c05d0d2) )
	ROM_LOAD40_BYTE(      "mbh-06.18c",  0x000000,  0x100000, CRC(038c2127) SHA1(5bdb215305f1a419fde27a83b623a38b9328e560) )
	ROM_LOAD40_WORD_SWAP( "mbh-03.15c",  0x500003,  0x080000, CRC(787787e3) SHA1(531444e3f28aa9a7539a5a76ca94a9d6b97274c5) )
	ROM_LOAD40_WORD_SWAP( "mbh-05.17c",  0x500001,  0x080000, CRC(1d2b7c17) SHA1(ae0b8e0448a1a8180fb424fb0bc8a4302f8ff602) )
	ROM_LOAD40_BYTE(      "mbh-07.19c",  0x500000,  0x040000, CRC(bbd22323) SHA1(6ab665b2e6d04cdadc48c52e15098e978b61fe10) )

	ROM_REGION( 0x100000, "sprites2", 0 ) // Sprites
	ROM_LOAD( "mbh-08.16e",  0x000000,  0x80000,  CRC(cdd7f8cb) SHA1(910bbe8783c0ba722e9d6399b332d658fa059fdb) )
	ROM_LOAD( "mbh-09.18e",  0x080000,  0x80000,  CRC(33fa2121) SHA1(eb0e99d29b1ad9995df28e5b7cfc89d53efb53c3) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbh-10.14l", 0x000000,  0x80000,  CRC(c4d6b116) SHA1(c5685bce6a6c6a74ca600ebf766ba1007f0dc666) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbh-11.16l", 0x000000,  0x80000,  CRC(0ec40b6b) SHA1(9fef44149608ae2a00f6a75a6f77f2efcab6e78e) )

	ROM_REGION(0x200, "prom", 0 )
	ROM_LOAD( "ln-00.j7", 0x000000,  0x200, CRC(5e83eaf3) SHA1(95f5eb8e56dff6c2dce7c39a6dd458bfc38fe1cf) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "ve-00.3d",  0x0000, 0x0117, CRC(384d316c) SHA1(61b50c695d4210c199cf6f7bbe50c8a5ecd1d21c) )
	ROM_LOAD( "ve-01a.4d", 0x0200, 0x0117, CRC(109613c8) SHA1(5991e010c1bc2a827c8ee2c85a9b40e00a3167b3) )
	ROM_LOAD( "vm-02.8j",  0x0400, 0x0117, CRC(53692426) SHA1(b8f8cf6b1f6b637fcd1fcd62474e637f5d4a6901) )
ROM_END


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT    MACHINE    INPUT     CLASS           INIT            ROT   COMPANY                  FULLNAME                                              FLAGS
GAME( 1991, captaven,   0,        captaven,  captaven, captaven_state, init_captaven,  ROT0, "Data East Corporation", "Captain America and The Avengers (Asia Rev 1.4)",    MACHINE_SUPPORTS_SAVE )
GAME( 1991, captavena,  captaven, captaven,  captaven, captaven_state, init_captaven,  ROT0, "Data East Corporation", "Captain America and The Avengers (Asia Rev 1.0)",    MACHINE_SUPPORTS_SAVE )
GAME( 1991, captavene,  captaven, captaven,  captaven, captaven_state, init_captaven,  ROT0, "Data East Corporation", "Captain America and The Avengers (UK Rev 1.4)",      MACHINE_SUPPORTS_SAVE )
GAME( 1991, captavenu,  captaven, captaven,  captaven, captaven_state, init_captaven,  ROT0, "Data East Corporation", "Captain America and The Avengers (US Rev 1.9)",      MACHINE_SUPPORTS_SAVE )
GAME( 1991, captavenuu, captaven, captaven,  captaven, captaven_state, init_captaven,  ROT0, "Data East Corporation", "Captain America and The Avengers (US Rev 1.6)",      MACHINE_SUPPORTS_SAVE )
GAME( 1991, captavenua, captaven, captaven,  captaven, captaven_state, init_captaven,  ROT0, "Data East Corporation", "Captain America and The Avengers (US Rev 1.4)",      MACHINE_SUPPORTS_SAVE )
GAME( 1991, captavenj,  captaven, captaven,  captaven, captaven_state, init_captaven,  ROT0, "Data East Corporation", "Captain America and The Avengers (Japan Rev 0.2)",   MACHINE_SUPPORTS_SAVE )

// DE-0396-0 PCB sets (Z80 for sound)
GAME( 1993, fghthistu,  fghthist, fghthistu, fghthist, fghthist_state, init_fghthist,  ROT0, "Data East Corporation", "Fighter's History (US ver 42-09, DE-0396-0 PCB)",    MACHINE_SUPPORTS_SAVE )
// DE-0395-1 PCB sets (HuC6280 for sound)
GAME( 1993, fghthist,   0,        fghthsta,  fghthist, fghthist_state, init_fghthist,  ROT0, "Data East Corporation", "Fighter's History (World ver 43-09, DE-0395-1 PCB)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, fghthistua, fghthist, fghthsta,  fghthist, fghthist_state, init_fghthist,  ROT0, "Data East Corporation", "Fighter's History (US ver 42-06, DE-0395-1 PCB)",    MACHINE_SUPPORTS_SAVE )
GAME( 1993, fghthistub, fghthist, fghthsta,  fghthist, fghthist_state, init_fghthist,  ROT0, "Data East Corporation", "Fighter's History (US ver 42-05, DE-0395-1 PCB)",    MACHINE_SUPPORTS_SAVE )
GAME( 1993, fghthistj,  fghthist, fghthsta,  fghthist, fghthist_state, init_fghthist,  ROT0, "Data East Corporation", "Fighter's History (Japan ver 41-07, DE-0395-1 PCB)", MACHINE_SUPPORTS_SAVE )
// DE-0380-2 PCB sets (HuC6280 for sound)
GAME( 1993, fghthista,  fghthist, fghthist,  fghthist, fghthist_state, init_fghthist,  ROT0, "Data East Corporation", "Fighter's History (World ver 43-07, DE-0380-2 PCB)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, fghthistb,  fghthist, fghthist,  fghthist, fghthist_state, init_fghthist,  ROT0, "Data East Corporation", "Fighter's History (World ver 43-05, DE-0380-2 PCB)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, fghthistuc, fghthist, fghthist,  fghthist, fghthist_state, init_fghthist,  ROT0, "Data East Corporation", "Fighter's History (US ver 42-03, DE-0380-2 PCB)",    MACHINE_SUPPORTS_SAVE )
GAME( 1993, fghthistja, fghthist, fghthist,  fghthist, fghthist_state, init_fghthist,  ROT0, "Data East Corporation", "Fighter's History (Japan ver 41-05, DE-0380-2 PCB)", MACHINE_SUPPORTS_SAVE )
// DE-0380-1 PCB sets (HuC6280 for sound)
GAME( 1993, fghthistjb, fghthist, fghthist,  fghthist, fghthist_state, init_fghthist,  ROT0, "Data East Corporation", "Fighter's History (Japan ver 41-04, DE-0380-1 PCB)", MACHINE_SUPPORTS_SAVE )

// DE-0397-0 PCB sets (Z80 for sound)
GAME( 1994, nslasher,   0,        nslasher,  nslasher, nslasher_state, init_nslasher,  ROT0, "Data East Corporation", "Night Slashers (Korea Rev 1.3, DE-0397-0 PCB)",      MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, nslasherj,  nslasher, nslasher,  nslasher, nslasher_state, init_nslasher,  ROT0, "Data East Corporation", "Night Slashers (Japan Rev 1.2, DE-0397-0 PCB)",      MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, nslashers,  nslasher, nslasher,  nslasher, nslasher_state, init_nslasher,  ROT0, "Data East Corporation", "Night Slashers (Over Sea Rev 1.2, DE-0397-0 PCB)",   MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
// DE-0395-1 PCB sets (HuC6280 for sound)
GAME( 1994, nslasheru,  nslasher, nslasheru, nslasher, nslasher_state, init_nslasher,  ROT0, "Data East Corporation", "Night Slashers (US Rev 1.2, DE-0395-1 PCB)",         MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

GAME( 1994, tattass,    0,        tattass,   tattass,  tattass_state,  init_tattass,   ROT0, "Data East Pinball",     "Tattoo Assassins (US prototype, Mar 14 1995)",       MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, tattasso,   tattass,  tattass,   tattass,  tattass_state,  init_tattass,   ROT0, "Data East Pinball",     "Tattoo Assassins (US prototype, Mar 14 1995, older sound)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, tattassa,   tattass,  tattass,   tattass,  tattass_state,  init_tattass,   ROT0, "Data East Pinball",     "Tattoo Assassins (Asia prototype, Mar 14 1995)",     MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

// Dragon Gun / Locked 'n Loaded have very different sprite hardware
GAME( 1993, dragngun,   0,        dragngun,  dragngun, dragngun_state, init_dragngun,  ROT0, "Data East Corporation", "Dragon Gun (US)",                                    MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // OKI3 Bankswitching aren't verified
GAME( 1993, dragngunj,  dragngun, dragngun,  dragngun, dragngun_state, init_dragngunj, ROT0, "Data East Corporation", "Dragon Gun (Japan)",                                 MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // ""

GAME( 1994, lockload,   0,        lockload,  lockload, dragngun_state, init_lockload,  ROT0, "Data East Corporation", "Locked 'n Loaded (World)",                           MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // hangs during attract mode if let running for a while without coining up; shooting in the lower corners during calibration of player's 2 gun hangs the game
GAME( 1994, gunhard,    lockload, lockload,  lockload, dragngun_state, init_lockload,  ROT0, "Data East Corporation", "Gun Hard (Japan)",                                   MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // "
GAME( 1994, lockloadu,  lockload, lockloadu, lockload, dragngun_state, init_lockload,  ROT0, "Data East Corporation", "Locked 'n Loaded (US, Dragon Gun conversion)",       MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // "
