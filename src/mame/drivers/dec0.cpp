// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
// thanks-to:Richard Bush
/***************************************************************************

  Data East 16 bit games - Bryan McPhail, mish@tendril.co.uk

  This file contains drivers for:

    * Heavy Barrel                            (USA set)
    * Heavy Barrel                            (World set)
    * Bad Dudes vs. Dragonninja               (USA set)
    * Dragonninja                             (Japanese version of above)
    * Birdie Try                              (Japanese set)
    * Robocop                                 (World bootleg rom set)
    * Robocop                                 (World rev 3)
    * Robocop                                 (USA rev 1)
    * Robocop                                 (USA rev 0)
    * Hippodrome                              (USA set)
    * Fighting Fantasy                        (Japanese version of above)
    * Secret Agent                            (World rev 3)
    * Secret Agent                            (Japan rev 2)
    * Sly Spy                                 (USA rev 3)
    * Sly Spy                                 (USA rev 2)
    * Midnight Resistance                     (World set)
    * Midnight Resistance                     (USA set)
    * Midnight Resistance                     (Japanese set)
    * Boulder Dash                            (World set)
    * Bandit                                  (USA set)

    Heavy Barrel, Bad Dudes, Robocop, Birdie Try & Hippodrome use the 'MEC-M1'
motherboard and varying game boards.  Sly Spy, Midnight Resistance and
Boulder Dash use the same graphics chips but are different pcbs.

    Bandit (USA) is almost certainly a field test prototype, the software runs
    on a Heavy Barrel board including the original Heavy Barrel MCU (which is effectively
    not used).  There is also Japanese version known to run on a DE-0321-1 top board.

    There are Secret Agent (bootleg) and Robocop (bootleg) sets to add.

    Thanks to Gouky & Richard Bush for information along the way, especially
    Gouky's patch for Bad Dudes & YM3812 information!
    Thanks to JC Alexander for fix to Robocop ending!

    All games' Dip Switches (except Boulder Dash) have been verified against
Original Service Manuals and Service Mode (when available).


ToDo:
- Fix remaining graphical problems in Automat (bootleg);
- Fix remaining sound problems in Secret Agent (bootleg);
- graphics are completely broken in Secret Agent (bootleg);
- Fighting Fantasy (bootleg) doesn't move on when killing the Lamia, is the MCU involved?
- Hook up the 68705 in Midnight Resistance (bootleg) (it might not be used, leftover from the Fighting Fantasy bootleg on the same PCB?)
- Get rid of ROM patch in Hippodrome;
- background pen in Birdie Try is presumably wrong.
- Pixel clock frequency isn't verified

Bad Dudes MCU implements a command to calculate a program ROM checksum and
compare the low byte of the result to a value supplied by the host CPU, but it
doesn't work.  Here's the code in question:

0AB0: 51 50    acall   $0A50
0AB2: C3       clr     c
0AB3: 48       orl     a,r0
0AB4: 70 02    jnz     $0AB8
0AB6: 80 89    sjmp    $0A41
0AB8: 21 F0    ajmp    $09F0

The function at $0A50 reads the expected value from the host, $0A41 is the
normal command response, and $09F0 is the error response.  The orl instruction
doesn't make sense here.  However, changing it from 48 to 60 makes it an xrl
instruction which would work as expected.  The game doesn't issue this command.
The checksum function can't be used to verify that the program is good because
the expected value mod 256 has to be supplied by the host.

Bad Dudes only seems to use commands $0B (sync), $01 (reset if parameter is not
$3B), $07 (return table index if parameter matches table, otherwise reset), and
$09 (set table index to zero).  Dragonninja only seems to use commands $03 (on
startup), $07 (same function as Bad Dudes) and $09 (same function as Bad Dudes).
Most of the MCU program isn't utilised.



***************************************************************************

Data East 16 bit games (Updated 19-Feb-2021)
Hardware info by Guru

The games that use this hardware include....
Heavy Barrel
Bad Dudes vs. Dragonninja
Dragonninja
Birdie Try
Robocop
Hippodrome
Fighting Fantasy
Secret Agent
Sly Spy
Midnight Resistance
Boulder Dash

Heavy Barrel, Bad Dudes, Robocop, Birdie Try & Hippodrome/Fighting Fantasy use the 'MEC-M1'
motherboard and another plug-in game board containing all the ROMs.
Sly Spy, Midnight Resistance and Boulder Dash use the same graphics chips but are single pcbs.

PCB Layouts
-----------

Main Board:

This board is used with Heavy Barrel, Bad Dudes, Robocop, Birdie Try, Bandit & Hippodrome/Fighting Fantasy

PCB number: MEC-M1 DE-0297-3 (uses QFP custom chips)
or
MEC-M1 DE-0295-1 (uses PGA custom chips)
or
DE-0289-2 (uses PGA custom chips and MEC-M1 not written on PCB)
|------------------------------------------------------------------|
|               TMM2018                                    12MHz   |
|               TMM2018                                            |
|               TMM2018          MB7122                            |
|                                                       TC5565     |
|                                                       TC5565     |
|                                                MB7116 TMM2018    |
|          20MHz                                        TMM2018    |
|J  RCDM-I1                                                        |
|A  RCDM-I1                                            |---------| |
|M  RCDM-I1                             TMM2018        |L7B0072  | |
|M  RCDM-I1                             TMM2018        |DATAEAST | |
|A  RCDM-I1                             TMM2018        |BAC 06   | |
|   RCDM-I1    |-------------|          TMM2018        |---------| |
|    DSW2      |   68000P10  |                                     |
|    DSW1      |-------------|            |---------|  |---------| |
|                                         |L7B0073  |  |L7B0072  | |
|UPC3403      TC5565           6502A      |DATAEAST |  |DATAEAST | |
|CN4   YM3014 TC5565        TMM2018       |MXC 06   |  |BAC 06   | |
|VOL   YM3014 YM3812                      |---------|  |---------| |
|MB3730      YM2203             CN2                CN1             |
|------------------------------------------------------------------|
Notes:
      68000   - Clock input 10.000MHz [20/2]
      6502A   - Clock input 1.500MHz [12/8]
      YM2203  - Clock input 1.500MHz [12/8]
      YM3812  - Clock input 3.000MHz [12/4]
      TMM2018 - 2k x8 SRAM
      TC5565  - 8k x8 SRAM
      MB7122  - 1k x4 bipolar PROM with label "A-2" at location E17
      MB7116  - 512b x4 bipolar PROM with label "A-1" at location C12
      RCDM-I1 - Custom resistor array
      DSW1/2  - 8-position DIP switches
      L7B007x - DECO custom graphics chips (QFP160 or PGA type)
      CN1/2   - 96-pin connectors joining to ROM board
      CN4     - 6-pin cable joining to ROM board

      Measurements
      ------------
      VSync - 57.4162Hz
      HSync - 15.6172kHz
      OSC1  - 20.00006MHz (20MHz)
      XTAL1 - 11.9938MHz (12MHz)

The "6502A" (so identified on Robocop schematics) is typically a Ricoh
RP65C02A, but Rockwell R6502AP is found on some Japanese boards as well
as bootlegs.


ROM Board Type 1:

This board is used with Heavy Barrel, Bad Dudes, Bandit & Birdie Try
All ROMs are in sockets. Boards that do not use some positions
do not have the socket populated (i.e. Bad Dudes)

DE-0299-2 (earlier version DE-0293-2 or DE-0293-1 uses PGA custom chips)
|-------------------------------------------------------|
|C4558          CN2  7.8A              CN1              |
|       1.3A 2.4A 3.6A        9.12A 10.14A 11.16A 12.17A|
| M6295               I8751_31.9A                       |
|CN9                    8MHz                            |
|                                                       |
|       4.3C 5.4C 6.6C       13.12C 14.14C 15.16C 16.17C|
|  8.2C                                                 |
|                                                       |
|                 |---------|                           |
|                 |L7B0072  | TMM2018                   |
|CN5              |DATAEAST |17.12D 18.14D 19.16D 20.17D|
|                 |BAC 06   | TMM2018                   |
|                 |---------|                           |
|CN6                                                    |
|                            21.12F 22.14F 23.16F 24.17F|
|                   27.7F 28.9F                         |
|CN7        D4701                                       |
|   LA6339                                              |
|   LA6339  D4701                                       |
|CN8                29.7J 30.9J      25.15J 26.16J      |
|-------------------------------------------------------|
Notes:
      i8751     - Intel 8751 microcontroller, clock input 8.000MHz
      M6295     - Clock 1.000MHz [20/2/10], pin 7 HIGH
      L7B0072   - DECO custom graphics chip (QFP160 or PGA type). Populated on ALL PCBs even if not used.
                  If ROMs 27/28/29 & 30 are not populated this chip is not used
      TMM2018   - 2k x8 SRAM
      C4558     - NEC C4558 Dual Op Amp
      LA6339    - Sanyo LA6339 High Performance Quad Comparator
      D4701     - NEC uPD4701 X,Y 2-axis Incremental Encoder Counter
      CN1/2     - 96-pin connectors joining to Main Board
      CN5/6     - Connectors for other inputs/buttons. Not populated on Bad Dudes.
                  Used for the rotary joysticks on Heavy Barrel
      CN7/8     - Connectors for trackball inputs for player 1 and player 2. Not populated on Bad Dudes or Heavy Barrel.
                  Used for trackballs on Birdie Try
      CN9       - 6-pin cable joining to Main Board
      ROMs      - All ROM locations shown but not all are populated.
                  All DECO games use a game code on the label....
                   - Bad Dudes    = EI    \
                   - Birdie Try   = EK     |
                   - Dragon Ninja = EG     | These also change for different country/regions
                   - Heavy Barrel = EC    /  Some Heavy Barrel boards only have numbers on the stickers
                  All ROMs are 27512/27256 EPROM/maskROM
                  All ROMs have a number location on the board next to the chip and that same number is used
                  as part of the ROM label. The chips can also be referenced via the numbers & letters on the
                  side of the board. For example EI30.30 or EI30.9J. Both of these are the same chip and the
                  game code identifies the game, which in this case is Bad Dudes.

ROM usage per game
------------------
     Location  3A    4A    6A    3C    4C    6C    8A    2C    12A   14A   16A   17A   12C   14C   16C   17C   12D   14D   16D   17D   12F   14F   16F   17F   15J   16J   7F    9F    7J    9J    9A
Game           1     2     3     4     5     6     7     8     9     10    11    12    13    14    15    16    17    18    19    20    21    22    23    24    25    26    27    28    29    30    31
----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Birdie Try     EK01  EK02  EK03  EK04  EK05  EK06  EK07  EK08  EK09  EK10  EK11  EK12  EK13  EK14  EK15  EK16  EK17  EK18  EK19  EK20  EK21  EK22  EK23  EK24  EK25  EK26  -     -     -     -     EK31-1
Bad Dudes      EI01  -     EI03  EI04  -     EI06  EI07  EI08  EI09  EI10  EI11  EI12  EI13  EI14  EI15  EI16  -     EI18  -     EI20  -     EI22  -     EI24  EI25  EI26  -     EI28  -     EI30  EI31
Dragon Ninja   EG01  -     EG03  EG04  -     EG06  EG07  EG08  EG09  EG10  EG11  EG12  EG13  EG14  EG15  EG16  -     EG18  -     EG20  -     EG22  -     EG24  EG25  EG26  -     EG28  -     EG30  EG31
Heavy Barrel   EC01  EC02  EC03  EC04  EC05  EC06  EC07  EC08  EC09  EC10  EC11  EC12  EC13  EC14  EC15  EC16  EC17  EC18  EC19  EC20  EC21  EC22  EC23  EC24  EC25  EC26  EC27  EC28  EC29  EC30  EC31

Note the games can be converted by swapping all of the ROMs and MCU. For example on a Birdie Try PCB, when the
ROMs are swapped for the 'hbarrel' set from MAME, the board will run Heavy Barrel. I can confirm the MCU dump
from the 'hbarrel' MAME ROM set (HB31.9A) and 'baddudes' MAME ROM set (EI31.9A) works fine on the real PCB.


ROM Board Type 2:

This board is used with Hippodrome & Fighting Fantasy

DE-0318-4
|-------------------------------------------------------|
|               CN2                    CN1              |
|                                                       |
|       EX00.B1    EX04.C1                              |
|CN4                                                    |
|       EX01-3.B3  PZ-1.C3            EX13.F3   EX18.G3 |
|                                                       |
|       EX02-3.B4  EX05.C4            EX14.F4   EX19.G4 |
|                                                       |
|       EX03.B5    EX06.C5  EX10.D5   EX15.F5   EX20.G5 |
|                                                       |
|       M6295      EX07.C6  EX11.D6   EX16.F6   EX21.G6 |
|     4558                  EX12.D8   EX17.F8   EX22.G8 |
||--|                                 TMM2018           |
||  |       |---------|               TMM2018   EX23.G10|
||49|       |   47    |            |---------|          |
||  |       |---------|            |L7B0072  |  EX24.G11|
||  |           TMM2063            |DATAEAST |          |
||--| 21.4772MHz                   |BAC 06   |  EX25.G13|
|           |---|  EX08.C15        |---------|          |
|    C1060  | 45|                                       |
|-----------|---|---------------------------------------|
Notes:
      49        - NEC DIP40 ULA (no clock input)
      47        - Unknown SDIP52 ASIC (no clock input so not an MCU)
      45        - HuC6280 sound CPU in disguise as Data East custom chip #45. Clock input 21.4772MHz on pin 10
      M6295     - Clock 1.000MHz [20/2/10], pin 7 HIGH
      4558      - NEC C4558 Dual Op Amp
      C1060     - NEC uPC1060C 2.5V High Precision Reference Voltage Circuit
      L7B0072   - DECO custom graphics chip (QFP160)
      TMM2018   - 2k x8 SRAM
      TMM2063   - 8k x8 SRAM
      PZ-1.C3   - MMI PAL16L8 marked "PZ-1"
      CN1/2     - 96-pin connectors joining to Main Board
      CN4       - 6-pin cable joining to Main Board
      ROMs      - All ROMs are 27512/27256 EPROM/maskROM
                  All ROMs have a number location under the chip and that same number is used
                  as part of the ROM label. The chips can also be referenced via the numbers
                  & letters on the side of the board. For example EX25.G13 or EX25.25 are the
                  same chip. Game code on the ROM labels for Hippodrome/Fighting Fantasy is
                  EV, EW or EX.


ROM Board Type 3:

This board is used only with Robocop

DE-0316-4
|-------------------------------------------------------|
|               CN2                    CN1              |
|4558 PZ-1.B15                                          |
|                EP03-3.C14 EP06.E14  EP10.F14          |
|CN4                                                    |
|     EP00-3.B13            EP07.E13  EP11.F13  EP18.H13|
|M6295                                                  |
|     EP01-4.B12 EP04-3.C12 EP08.E12  EP12.F12  EP19.H12|
|                                                       |
|     EP02.B11   EP05-4.C11 EP09.E11  EP13.F11  EP20.H11|
|                           TMM2063                     |
|  PZ-0.A9                  TMM2063             EP21.H10|
|                                                       |
| 21.4772MHz              |---------| EP14.F7   EP22.H7 |
| |------|                |L7B0072  |           EP23.H6 |
| |DEC-01|                |DATAEAST | EP15.F4           |
| |      |                |BAC 06   |                   |
| |------|  |---------|   |---------| EP16.F3           |
|  2018     | DEM-01  |                                 |
|           |---------|               EP17.F2           |
|  PZ-2.A2                                              |
|-------------------------------------------------------|
Notes:
      DEM-01    - Fujitsu MB8421 Dual port SRAM in disguise as Data East custom chip DEM-01.
      DEC-01    - HuC6280 sound CPU in disguise as Data East custom chip DEC-01. Clock input 21.4772MHz on pin 10
      M6295     - Clock 1.000MHz [20/2/10], pin 7 HIGH
      4558      - NEC C4558 Dual Op Amp
      L7B0072   - DECO custom graphics chip (QFP160)
      TMM2018   - 2k x8 SRAM
      TMM2063   - 8k x8 SRAM
      PZ-*      - MMI PAL16L8 PALs
      CN1/2     - 96-pin connectors joining to Main Board
      CN4       - 6-pin cable joining to Main Board
      ROMs      - All ROMs are 27512/27256 EPROM/maskROM


Single PCB Types
----------------

Midnight Resistance
Hardware info by Guru

DE-0323-4
|--------------------------------------------------------------------------|
|   RCDM-I1          TMM2018  MB7114.22F |---------|              FH-11.21A|
|J  RCDM-I1          TMM2018             |L7B0072  |  TMM2018     FH-10.20A|
|A  RCDM-I1  FK15.18K  TB-6              |DATAEAST |  TMM2018     FH-09.18A|
|M  RCDM-I1            TB-1-1            |BAC 06   |                       |
|M  RCDM-I1  FK14.17K           |-----|  |---------|              FH-08.16A|
|A   SW1                        | 6   |  |---------|                       |
|    SW2     FK13.15K           | 8   |  |L7B0072  |              FH-07.15A|
|                               | 0   |  |DATAEAST |  TMM2018              |
|   RCDM-I1  FK12.13K  TB-5     | 0   |  |BAC 06   |  TMM2018     FH-05.13A|
|   RCDM-I1                     | 0   |  |---------|                       |
|   RCDM-I1                     | P   |                           FH05-.11A|
|   RCDM-I1            TB-2     | 1   |  |---------|                       |
|   RCDM-I1  TMM2063   TB-3     | 2   |  |L7B0072  |  TMM2063     FH04-.10A|
|   RCDM-I1  TMM2063   TB-4     |-----|  |DATAEAST |  TMM2063              |
|CN2         20MHz                       |BAC 06   |              FH-03.8A |
|                                        |---------|                       |
|CN3         24MHz     36       TMM2018               |---------| FH-02.6A |
|                               TMM2018               |L7B0073  |          |
|VOL     YM3014     1.056MHz    FH16-.5F   TMM2018    |DATAEAST | FH-01.4A |
|UPC3403          YM3812        TMM2063    TMM2018    |MXC 06   |          |
|        YM3014   YM2203          |-----|             |---------| FH-00.2A |
|MB3730  UPC3403  M6295  FH17-.1H | 45  |                                  |
|---------------------------------|-----|----------------------------------|
Notes:
      68000   - Clock input 10.000MHz [20/2]
      YM2203  - Clock input 1.500MHz [24/8/2]
      YM3812  - Clock input 3.000MHz [24/4/2]
      M6295   - Clock 1.056MHz, pin 7 HIGH
      45      - HuC6280 sound CPU in disguise as Data East custom chip 45. Clock input 6.000MHz [24/4] on pin 10
      36      - Mysterious unknown SOP28 chip (possibly protection-related?)
      UPC3403 - NEC uPC3403 quad operational amplifier
      TMM2018 - 2k x8 SRAM, equivalent to 6116
      TMM2063 - 8k x8 SRAM, equivalent to 6264
      MB7114  - 256b x4-bit bipolar PROM marked 'TB-7', compatible with 82S129
      RCDM-I1 - Custom resistor array
      SW1/SW2 - 8-position DIP switch
      CN2/CN3 - 13-pin connector for rotary joystick
      L7B007x - DECO custom graphics chips (QFP160)
                MXC 06 = Sprite Generator
                BAC 06 = Tile Generator
      TB-*    - PALs
                TB1-1 = MMI PAL16R4
                Other PALs are MMI PAL16L8
      ROMs    - FK* = 27C010 EPROM
                FH04/FH05 = 64kB Mask ROM (compatible with 27C512)
                FH17 = 27C010 EPROM
                Other FH-xx ROMs are 128kB Mask ROM (compatible with 27C010)
                Labels on some ROMs FL & FH seem to be used interchangeably on different ROM sets but the contents is the same.

      Measurements
      ------------
      VSync - 57.44530Hz
      HSync - 15.1438kHz

***************************************************************************/

#include "emu.h"
#include "includes/dec0.h"

#include "cpu/m68000/m68000.h"
#include "cpu/m6502/m6502.h"
#include "cpu/z80/z80.h"
#include "cpu/m6805/m68705.h"
#include "machine/mb8421.h"
#include "machine/upd4701.h"
#include "sound/okim6295.h"
#include "sound/ymopn.h"
#include "sound/ymopl.h"
#include "speaker.h"


/******************************************************************************/

void dec0_state::dec0_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (offset << 1)
	{
		case 0: /* Playfield & Sprite priority */
			priority_w(0, data, mem_mask);
			break;

		case 2: /* DMA flag */
			m_spriteram->copy();
			break;

		case 4: /* 6502 sound cpu */
			if (ACCESSING_BITS_0_7)
				m_soundlatch->write(data & 0xff);
			break;

		case 6: /* Intel 8751 microcontroller - Bad Dudes, Heavy Barrel, Birdie Try, Bandit only */
			dec0_i8751_write(data);
			break;

		case 8: /* Interrupt ack (VBL - IRQ 6) */
			m_maincpu->set_input_line(6, CLEAR_LINE);
			break;

		case 0xa: /* Mix Psel(?). */
			logerror("CPU #0 PC %06x: warning - write %02x to unmapped memory address %06x\n",m_maincpu->pc(),data,0x30c010+(offset<<1));
			break;

		case 0xc: /* Cblk - coin blockout.  Seems to be unused by the games */
			break;

		case 0xe: /* Reset Intel 8751? - not sure, all the games write here at startup */
			dec0_i8751_reset();
			logerror("CPU #0 PC %06x: warning - write %02x to unmapped memory address %06x\n",m_maincpu->pc(),data,0x30c010+(offset<<1));
			break;

		default:
			logerror("CPU #0 PC %06x: warning - write %02x to unmapped memory address %06x\n",m_maincpu->pc(),data,0x30c010+(offset<<1));
			break;
	}
}

void dec0_automat_state::automat_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (offset << 1)
	{
		case 0xe: /* z80 sound cpu */
			if (ACCESSING_BITS_0_7)
				m_soundlatch->write(data & 0xff);
			break;

		case 12: /* DMA flag */
			//m_spriteram->copy();
			break;
#if 0
		case 8: /* Interrupt ack (VBL - IRQ 6) */
			break;

		case 0xa: /* Mix Psel(?). */
			logerror("CPU #0 PC %06x: warning - write %02x to unmapped memory address %06x\n",m_maincpu->pc(),data,0x30c010+(offset<<1));
			break;

		case 0xc: /* Cblk - coin blockout.  Seems to be unused by the games */
			break;
#endif

		default:
			logerror("CPU #0 PC %06x: warning - write %02x to unmapped memory address %06x\n",m_maincpu->pc(),data,0x30c010+(offset<<1));
			break;
	}
}

/******************************************************************************/

void dec0_state::dec0_map(address_map &map)
{
	map(0x000000, 0x05ffff).rom();
	map(0x240000, 0x240007).w(m_tilegen[0], FUNC(deco_bac06_device::pf_control_0_w));                          /* text layer */
	map(0x240010, 0x240017).w(m_tilegen[0], FUNC(deco_bac06_device::pf_control_1_w));
	map(0x242000, 0x24207f).rw(m_tilegen[0], FUNC(deco_bac06_device::pf_colscroll_r), FUNC(deco_bac06_device::pf_colscroll_w));
	map(0x242400, 0x2427ff).rw(m_tilegen[0], FUNC(deco_bac06_device::pf_rowscroll_r), FUNC(deco_bac06_device::pf_rowscroll_w));
	map(0x242800, 0x243fff).ram();                                                     /* Robocop only */
	map(0x244000, 0x245fff).rw(m_tilegen[0], FUNC(deco_bac06_device::pf_data_r), FUNC(deco_bac06_device::pf_data_w));

	map(0x246000, 0x246007).w(m_tilegen[1], FUNC(deco_bac06_device::pf_control_0_w));                                  /* first tile layer */
	map(0x246010, 0x246017).w(m_tilegen[1], FUNC(deco_bac06_device::pf_control_1_w));
	map(0x248000, 0x24807f).rw(m_tilegen[1], FUNC(deco_bac06_device::pf_colscroll_r), FUNC(deco_bac06_device::pf_colscroll_w));
	map(0x248400, 0x2487ff).rw(m_tilegen[1], FUNC(deco_bac06_device::pf_rowscroll_r), FUNC(deco_bac06_device::pf_rowscroll_w));
	map(0x24a000, 0x24a7ff).rw(m_tilegen[1], FUNC(deco_bac06_device::pf_data_r), FUNC(deco_bac06_device::pf_data_w));

	map(0x24c000, 0x24c007).w(m_tilegen[2], FUNC(deco_bac06_device::pf_control_0_w));                              /* second tile layer */
	map(0x24c010, 0x24c017).w(m_tilegen[2], FUNC(deco_bac06_device::pf_control_1_w));
	map(0x24c800, 0x24c87f).rw(m_tilegen[2], FUNC(deco_bac06_device::pf_colscroll_r), FUNC(deco_bac06_device::pf_colscroll_w));
	map(0x24cc00, 0x24cfff).rw(m_tilegen[2], FUNC(deco_bac06_device::pf_rowscroll_r), FUNC(deco_bac06_device::pf_rowscroll_w));
	map(0x24d000, 0x24d7ff).rw(m_tilegen[2], FUNC(deco_bac06_device::pf_data_r), FUNC(deco_bac06_device::pf_data_w));

	map(0x300000, 0x300001).portr("AN0");
	map(0x300008, 0x300009).portr("AN1");
	map(0x30c000, 0x30c00b).r(FUNC(dec0_state::dec0_controls_r));
	map(0x30c010, 0x30c01f).w(FUNC(dec0_state::dec0_control_w));                                   /* Priority, sound, etc. */
	map(0x30c012, 0x30c013).nopr(); // clr.w for sprite DMA
	map(0x30c018, 0x30c019).nopr(); // clr.w for irq ack
	map(0x310000, 0x3107ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x314000, 0x3147ff).ram().w(m_palette, FUNC(palette_device::write16_ext)).share("palette_ext");

	map(0x318000, 0x31bfff).ram().share("ram");         // Bandit uses 318000/31c000 which are mirrors but exact mirror patten is unclear
	map(0x31c000, 0x31c7ff).ram().share("spriteram");

	map(0xff8000, 0xffbfff).ram().share("ram");                                 /* Main ram */
	map(0xffc000, 0xffc7ff).ram().share("spriteram");
}

void dec0_state::ffantasybl_map(address_map &map)
{
	dec0_map(map);

	map(0x0024c880, 0x0024cbff).ram(); // what is this? layer 3-related??
	map(0x00242024, 0x00242025).r(FUNC(dec0_state::ffantasybl_242024_r));
	map(0x00ff87ee, 0x00ff87ef).portr("VBLANK");
}

void dec0_state::dec0_tb_map(address_map &map)
{
	dec0_map(map);
	map(0x300010, 0x300017).r("tb0", FUNC(upd4701_device::read_xy)).umask16(0x00ff);
	map(0x300018, 0x30001f).r("tb1", FUNC(upd4701_device::read_xy)).umask16(0x00ff);
	map(0x300001, 0x300001).w("tb0", FUNC(upd4701_device::reset_x_w));
	map(0x300009, 0x300009).w("tb0", FUNC(upd4701_device::reset_y_w));
	map(0x300011, 0x300011).w("tb1", FUNC(upd4701_device::reset_x_w));
	map(0x300019, 0x300019).w("tb1", FUNC(upd4701_device::reset_y_w));
}

void dec0_state::robocop_map(address_map &map)
{
	dec0_map(map);
	map(0x180000, 0x180fff).rw("dem01", FUNC(mb8421_device::left_r), FUNC(mb8421_device::left_w)).umask16(0x00ff);
}

void dec0_state::robocop_sub_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x1f0000, 0x1f1fff).ram();                                 /* Main ram */
	map(0x1f2000, 0x1f27ff).rw("dem01", FUNC(mb8421_device::right_r), FUNC(mb8421_device::right_w));  /* Shared ram */
}

void dec0_state::hippodrm_map(address_map &map)
{
	dec0_map(map);
	map(0x180000, 0x18003f).rw(FUNC(dec0_state::hippodrm_68000_share_r), FUNC(dec0_state::hippodrm_68000_share_w));
	map(0xffc800, 0xffcfff).w(FUNC(dec0_state::sprite_mirror_w));
}

void dec0_state::hippodrm_sub_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x180000, 0x18001f).ram().share("hippodrm_shared");
	map(0x1a0000, 0x1a0007).w(m_tilegen[2], FUNC(deco_bac06_device::pf_control0_8bit_packed_w));
	map(0x1a0010, 0x1a001f).w(m_tilegen[2], FUNC(deco_bac06_device::pf_control1_8bit_swap_w));
	map(0x1a1000, 0x1a17ff).rw(m_tilegen[2], FUNC(deco_bac06_device::pf_data_8bit_swap_r), FUNC(deco_bac06_device::pf_data_8bit_swap_w));
	map(0x1d0000, 0x1d00ff).rw(FUNC(dec0_state::hippodrm_prot_r), FUNC(dec0_state::hippodrm_prot_w));
	map(0x1f0000, 0x1f1fff).ram(); /* Main ram */
}


uint16_t dec0_state::slyspy_controls_r(offs_t offset)
{
	switch (offset<<1)
	{
		case 0: /* Dip Switches */
			return ioport("DSW")->read();

		case 2: /* Player 1 & Player 2 joysticks & fire buttons */
			return ioport("INPUTS")->read();

		case 4: /* Credits */
			return ioport("SYSTEM")->read();
	}

	logerror("Unknown control read at 30c000 %d\n", offset);
	return ~0;
}

// TODO: this can be a timer access, maybe video counter returns (and used as RNG in both games)
uint16_t dec0_state::slyspy_protection_r(offs_t offset)
{
	switch (offset<<1)
	{
		/* These values are for Boulder Dash, I have no idea what they do in Sly Spy */
		case 0:     return 0;
		case 2:     return 0x13;
		case 4:     return 0;
		case 6:     return 0x2;
		// Sly Spy uses this port as RNG, for now let's do same thing as bootleg (i.e. reads 0x306028)
		// chances are that it actually ties to the main CPU xtal instead.
		// (reads at 6958 6696)
		case 0xc:   return m_ram[0x2028/2] >> 8;
	}

	logerror("%04x, Unknown protection read at 30c000 %d\n", m_maincpu->pc(), offset);
	return 0;
}


/*
    The memory map in Sly Spy can change between 4 states according to the protection!

    Default state (called by Traps 1, 3, 4, 7, C)

    240000 - 24001f = control   (Playfield 2 area)
    242000 - 24207f = colscroll
    242400 - 2425ff = rowscroll
    246000 - 2467ff = data

    248000 - 24801f = control  (Playfield 1 area)
    24c000 - 24c07f = colscroll
    24c400 - 24c4ff = rowscroll
    24e000 - 24e7ff = data

    State 1 (Called by Trap 9) uses this memory map:

    248000 = pf1 data
    24c000 = pf2 data

    State 2 (Called by Trap A) uses this memory map:

    240000 = pf2 data
    242000 = pf1 data
    24e000 = pf1 data

    State 3 (Called by Trap B) uses this memory map:

    240000 = pf1 data
    248000 = pf2 data

*/

void dec0_state::slyspy_state_w(uint16_t data)
{
	m_slyspy_state = 0;
	m_pfprotect->set_bank(m_slyspy_state);
}

uint16_t dec0_state::slyspy_state_r()
{
	m_slyspy_state = (m_slyspy_state + 1) % 4;
	m_pfprotect->set_bank(m_slyspy_state);

	return 0; /* Value doesn't mater */
}


void dec0_state::slyspy_protection_map(address_map &map)
{
	map(0x04000, 0x04001).mirror(0x30000).r(FUNC(dec0_state::slyspy_state_r)).nopw();
	map(0x0a000, 0x0a001).mirror(0x30000).w(FUNC(dec0_state::slyspy_state_w));
	// Default state (called by Traps 1, 3, 4, 7, C)
	map(0x00000, 0x00007).w(m_tilegen[1], FUNC(deco_bac06_device::pf_control_0_w));
	map(0x00010, 0x00017).w(m_tilegen[1], FUNC(deco_bac06_device::pf_control_1_w));
	map(0x02000, 0x0207f).w(m_tilegen[1], FUNC(deco_bac06_device::pf_colscroll_w));
	map(0x02400, 0x027ff).w(m_tilegen[1], FUNC(deco_bac06_device::pf_rowscroll_w));
	map(0x06000, 0x07fff).w(m_tilegen[1], FUNC(deco_bac06_device::pf_data_w));
	map(0x08000, 0x08007).w(m_tilegen[0], FUNC(deco_bac06_device::pf_control_0_w));
	map(0x08010, 0x08017).w(m_tilegen[0], FUNC(deco_bac06_device::pf_control_1_w));
	map(0x0c000, 0x0c07f).w(m_tilegen[0], FUNC(deco_bac06_device::pf_colscroll_w));
	map(0x0c400, 0x0c7ff).w(m_tilegen[0], FUNC(deco_bac06_device::pf_rowscroll_w));
	map(0x0e000, 0x0ffff).w(m_tilegen[0], FUNC(deco_bac06_device::pf_data_w));
	// State 1 (Called by Trap 9)
	map(0x18000, 0x19fff).w(m_tilegen[0], FUNC(deco_bac06_device::pf_data_w));
	map(0x1c000, 0x1dfff).w(m_tilegen[1], FUNC(deco_bac06_device::pf_data_w));
	// State 2 (Called by Trap A)
	map(0x20000, 0x21fff).w(m_tilegen[1], FUNC(deco_bac06_device::pf_data_w));
	map(0x22000, 0x23fff).w(m_tilegen[0], FUNC(deco_bac06_device::pf_data_w));
	map(0x2e000, 0x2ffff).w(m_tilegen[0], FUNC(deco_bac06_device::pf_data_w));
	// State 3 (Called by Trap B)
	map(0x30000, 0x31fff).w(m_tilegen[0], FUNC(deco_bac06_device::pf_data_w));
	map(0x38000, 0x39fff).w(m_tilegen[1], FUNC(deco_bac06_device::pf_data_w));
}

void dec0_state::slyspy_map(address_map &map)
{
	map(0x000000, 0x05ffff).rom();

	/* The location of pf1 & pf2 can change in the 240000 - 24ffff region according to protection */
	map(0x240000, 0x24ffff).m(m_pfprotect, FUNC(address_map_bank_device::amap16));

	/* Pf3 is unaffected by protection */
	map(0x300000, 0x300007).w(m_tilegen[2], FUNC(deco_bac06_device::pf_control_0_w));
	map(0x300010, 0x300017).w(m_tilegen[2], FUNC(deco_bac06_device::pf_control_1_w));
	map(0x300800, 0x30087f).rw(m_tilegen[2], FUNC(deco_bac06_device::pf_colscroll_r), FUNC(deco_bac06_device::pf_colscroll_w));
	map(0x300c00, 0x300fff).rw(m_tilegen[2], FUNC(deco_bac06_device::pf_rowscroll_r), FUNC(deco_bac06_device::pf_rowscroll_w));
	map(0x301000, 0x3017ff).rw(m_tilegen[2], FUNC(deco_bac06_device::pf_data_r), FUNC(deco_bac06_device::pf_data_w));

	map(0x304000, 0x307fff).ram().share("ram"); /* Sly Spy main ram */
	map(0x308000, 0x3087ff).ram().share("spriteram");   /* Sprites */
	map(0x310000, 0x3107ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x314001, 0x314001).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x314002, 0x314003).w(FUNC(dec0_state::priority_w));
	map(0x314008, 0x31400f).r(FUNC(dec0_state::slyspy_controls_r));
	map(0x31c000, 0x31c00f).r(FUNC(dec0_state::slyspy_protection_r)).nopw();
}


void dec0_state::midres_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x103fff).ram().share("ram");
	map(0x120000, 0x1207ff).ram().share("spriteram");
	map(0x140000, 0x1407ff).w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x160000, 0x160001).w(FUNC(dec0_state::priority_w));
	map(0x180000, 0x18000f).r(FUNC(dec0_state::midres_controls_r));
	map(0x180008, 0x18000f).nopw(); /* ?? watchdog ?? */
	map(0x1a0001, 0x1a0001).w(m_soundlatch, FUNC(generic_latch_8_device::write));

	map(0x200000, 0x200007).w(m_tilegen[1], FUNC(deco_bac06_device::pf_control_0_w));
	map(0x200010, 0x200017).w(m_tilegen[1], FUNC(deco_bac06_device::pf_control_1_w));
	map(0x220000, 0x2207ff).rw(m_tilegen[1], FUNC(deco_bac06_device::pf_data_r), FUNC(deco_bac06_device::pf_data_w));
	map(0x220800, 0x220fff).rw(m_tilegen[1], FUNC(deco_bac06_device::pf_data_r), FUNC(deco_bac06_device::pf_data_w)); /* mirror address used in end sequence */
	map(0x240000, 0x24007f).rw(m_tilegen[1], FUNC(deco_bac06_device::pf_colscroll_r), FUNC(deco_bac06_device::pf_colscroll_w));
	map(0x240400, 0x2407ff).rw(m_tilegen[1], FUNC(deco_bac06_device::pf_rowscroll_r), FUNC(deco_bac06_device::pf_rowscroll_w));

	map(0x280000, 0x280007).w(m_tilegen[2], FUNC(deco_bac06_device::pf_control_0_w));
	map(0x280010, 0x280017).w(m_tilegen[2], FUNC(deco_bac06_device::pf_control_1_w));
	map(0x2a0000, 0x2a07ff).rw(m_tilegen[2], FUNC(deco_bac06_device::pf_data_r), FUNC(deco_bac06_device::pf_data_w));
	map(0x2c0000, 0x2c007f).rw(m_tilegen[2], FUNC(deco_bac06_device::pf_colscroll_r), FUNC(deco_bac06_device::pf_colscroll_w));
	map(0x2c0400, 0x2c07ff).rw(m_tilegen[2], FUNC(deco_bac06_device::pf_rowscroll_r), FUNC(deco_bac06_device::pf_rowscroll_w));

	map(0x300000, 0x300007).w(m_tilegen[0], FUNC(deco_bac06_device::pf_control_0_w));
	map(0x300010, 0x300017).w(m_tilegen[0], FUNC(deco_bac06_device::pf_control_1_w));
	map(0x320000, 0x321fff).rw(m_tilegen[0], FUNC(deco_bac06_device::pf_data_r), FUNC(deco_bac06_device::pf_data_w));
	map(0x340000, 0x34007f).rw(m_tilegen[0], FUNC(deco_bac06_device::pf_colscroll_r), FUNC(deco_bac06_device::pf_colscroll_w));
	map(0x340400, 0x3407ff).rw(m_tilegen[0], FUNC(deco_bac06_device::pf_rowscroll_r), FUNC(deco_bac06_device::pf_rowscroll_w));
}

void dec0_state::midresb_map(address_map &map)
{
	midres_map(map);
	map(0x160010, 0x160011).w(FUNC(dec0_state::priority_w));
	map(0x180000, 0x18000f).r(FUNC(dec0_state::dec0_controls_r));
	map(0x180012, 0x180013).noprw();
	map(0x180015, 0x180015).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x180018, 0x180019).noprw();
	map(0x1a0000, 0x1a0001).portr("AN0");
	map(0x1a0008, 0x1a0009).portr("AN1");
}

/******************************************************************************/

void dec0_state::dec0_s_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x0800, 0x0801).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x1000, 0x1001).rw("ym2", FUNC(ym3812_device::read), FUNC(ym3812_device::write));
	map(0x3000, 0x3000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x3800, 0x3800).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x8000, 0xffff).rom();
}

/* Physical memory map (21 bits) */
void dec0_state::slyspy_s_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x080000, 0x0fffff).m(m_sndprotect, FUNC(address_map_bank_device::amap8));
	map(0x1f0000, 0x1f1fff).ram();
}

// Sly Spy sound state protection machine emulation
// similar to the video state machine
// current bank is at 0x1f0045, incremented by 1 then here is read
uint8_t dec0_state::slyspy_sound_state_r()
{
	m_slyspy_sound_state ++;
	m_slyspy_sound_state &= 3;
	m_sndprotect->set_bank(m_slyspy_sound_state);

	// returned value doesn't matter
	return 0xff;
}

uint8_t dec0_state::slyspy_sound_state_reset_r()
{
	m_slyspy_sound_state = 0;
	m_sndprotect->set_bank(m_slyspy_sound_state);

	// returned value doesn't matter
	return 0xff;
}

void dec0_state::slyspy_sound_protection_map(address_map &map)
{
	map(0x020000, 0x020001).mirror(0x180000).r(FUNC(dec0_state::slyspy_sound_state_r)); /* Protection counter */
	map(0x050000, 0x050001).mirror(0x180000).r(FUNC(dec0_state::slyspy_sound_state_reset_r));
	// state 0
	map(0x010000, 0x010001).w("ym2", FUNC(ym3812_device::write));
	map(0x030000, 0x030001).w("ym1", FUNC(ym2203_device::write));
	map(0x060000, 0x060001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x070000, 0x070001).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	// state 1
	map(0x090000, 0x090001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x0c0000, 0x0c0001).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x0e0000, 0x0e0001).w("ym1", FUNC(ym2203_device::write));
	map(0x0f0000, 0x0f0001).w("ym2", FUNC(ym3812_device::write));
	// state 2
	map(0x110000, 0x110001).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x130000, 0x130001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x140000, 0x140001).w("ym1", FUNC(ym2203_device::write));
	map(0x170000, 0x170001).w("ym2", FUNC(ym3812_device::write));
	// state 3
	map(0x190000, 0x190001).w("ym2", FUNC(ym3812_device::write));
	map(0x1c0000, 0x1c0001).w("ym1", FUNC(ym2203_device::write));
	map(0x1e0000, 0x1e0001).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x1f0000, 0x1f0001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}


void dec0_state::midres_s_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x108000, 0x108001).w("ym2", FUNC(ym3812_device::write));
	map(0x118000, 0x118001).w("ym1", FUNC(ym2203_device::write));
	map(0x130000, 0x130001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x138000, 0x138001).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x1f0000, 0x1f1fff).ram();
}



void dec0_automat_state::machine_start()
{
	m_adpcm_toggle[0] = false;
	m_adpcm_toggle[1] = false;
	save_item(NAME(m_adpcm_toggle));
	save_item(NAME(m_automat_scroll_regs));

	m_soundbank->configure_entries(0, 8, memregion("audiocpu")->base(), 0x4000);
	m_soundbank->set_entry(0);
}


/* swizzle the palette writes around so we can use the same gfx plane ordering as the originals */
uint16_t dec0_automat_state::automat_palette_r(offs_t offset)
{
	offset ^=0xf;
	return m_paletteram[offset];
}

void dec0_automat_state::automat_palette_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset ^=0xf;
	m_palette->write16(offset, data, mem_mask);
}



void dec0_automat_state::automat_map(address_map &map)
{
	map(0x000000, 0x05ffff).rom();

	map(0x240000, 0x240007).ram();         /* text layer */
	map(0x240010, 0x240017).ram();
	map(0x242000, 0x24207f).ram();
	map(0x242400, 0x2427ff).ram();
	map(0x242800, 0x243fff).ram();
	map(0x244000, 0x245fff).ram().rw(m_tilegen[0], FUNC(deco_bac06_device::pf_data_r), FUNC(deco_bac06_device::pf_data_w));

	map(0x246000, 0x246007).ram();         /* first tile layer */
	map(0x246010, 0x246017).ram();
	map(0x248000, 0x24807f).ram();
	map(0x248400, 0x2487ff).ram();
	map(0x24a000, 0x24a7ff).ram().rw(m_tilegen[1], FUNC(deco_bac06_device::pf_data_r), FUNC(deco_bac06_device::pf_data_w));

	map(0x24c000, 0x24c007).ram();         /* second tile layer */
	map(0x24c010, 0x24c017).ram();
	map(0x24c800, 0x24c87f).ram();
	map(0x24cc00, 0x24cfff).ram();
	map(0x24d000, 0x24d7ff).ram().rw(m_tilegen[2], FUNC(deco_bac06_device::pf_data_r), FUNC(deco_bac06_device::pf_data_w));

	map(0x300000, 0x300001).portr("AN0");
	map(0x300008, 0x300009).portr("AN1");
	map(0x30c000, 0x30c00b).r(FUNC(dec0_automat_state::dec0_controls_r));
	map(0x30c000, 0x30c01f).w(FUNC(dec0_automat_state::automat_control_w));            /* Priority, sound, etc. */
	map(0x310000, 0x3107ff).rw(FUNC(dec0_automat_state::automat_palette_r), FUNC(dec0_automat_state::automat_palette_w)).share("palette");
	map(0x314000, 0x3147ff).ram();

	// video regs are moved to here..
	map(0x400000, 0x400007).w(FUNC(dec0_automat_state::automat_scroll_w));
	map(0x400008, 0x400009).w(FUNC(dec0_automat_state::priority_w));

	map(0x500000, 0x500001).nopw(); // ???

	map(0xff8000, 0xffbfff).ram().share("ram");             /* Main ram */
	map(0xffc000, 0xffcfff).ram().share("spriteram");           /* Sprites */
}

void dec0_automat_state::secretab_map(address_map &map)
{
	map(0x000000, 0x05ffff).rom();
//  map(0x240000, 0x240007).w(m_tilegen[1], FUNC(deco_bac06_device::pf_control_0_w));
//  map(0x240010, 0x240017).w(m_tilegen[1], FUNC(deco_bac06_device::pf_control_1_w));
	map(0x246000, 0x247fff).rw(m_tilegen[1], FUNC(deco_bac06_device::pf_data_r), FUNC(deco_bac06_device::pf_data_w));
//  map(0x240000, 0x24007f).rw(m_tilegen[1], FUNC(deco_bac06_device::pf_colscroll_r), FUNC(deco_bac06_device::pf_colscroll_w));
//  map(0x240400, 0x2407ff).rw(m_tilegen[1], FUNC(deco_bac06_device::pf_rowscroll_r), FUNC(deco_bac06_device::pf_rowscroll_w));

//  map(0x200000, 0x300007).w(m_tilegen[0], FUNC(deco_bac06_device::pf_control_0_w));
//  map(0x300010, 0x300017).w(m_tilegen[0], FUNC(deco_bac06_device::pf_control_1_w));
	map(0x24e000, 0x24ffff).rw(m_tilegen[0], FUNC(deco_bac06_device::pf_data_r), FUNC(deco_bac06_device::pf_data_w));
//  map(0x340000, 0x34007f).rw(m_tilegen[0], FUNC(deco_bac06_device::pf_colscroll_r), FUNC(deco_bac06_device::pf_colscroll_w));
//  map(0x340400, 0x3407ff).rw(m_tilegen[0], FUNC(deco_bac06_device::pf_rowscroll_r), FUNC(deco_bac06_device::pf_rowscroll_w));

	map(0x314008, 0x31400f).r(FUNC(dec0_automat_state::slyspy_controls_r));
	map(0x314001, 0x314001).w(m_soundlatch, FUNC(generic_latch_8_device::write));

	map(0x300000, 0x300007).ram();
	map(0x300010, 0x300017).ram();
	map(0x300800, 0x30087f).ram();
	map(0x300c00, 0x300fff).ram();
	map(0x301000, 0x3017ff).rw(m_tilegen[2], FUNC(deco_bac06_device::pf_data_r), FUNC(deco_bac06_device::pf_data_w));
	map(0x301800, 0x307fff).ram().share("ram"); /* Sly Spy main ram */
	map(0x310000, 0x3107ff).rw(FUNC(dec0_automat_state::automat_palette_r), FUNC(dec0_automat_state::automat_palette_w)).share("palette");
	map(0xb08000, 0xb08fff).ram().share("spriteram"); /* Sprites */
}


void dec0_automat_state::automat_s_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("soundbank");
	map(0xc000, 0xc7ff).ram();
	map(0xc800, 0xc801).rw("2203a", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xd000, 0xd001).rw("2203b", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xd800, 0xd800).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xe000, 0xe000).w(m_adpcm_select[1], FUNC(ls157_device::ba_w));
	map(0xe800, 0xe800).w(FUNC(dec0_automat_state::sound_bankswitch_w));
	map(0xf000, 0xf000).w(m_adpcm_select[0], FUNC(ls157_device::ba_w));
}

void dec0_automat_state::secretab_s_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("soundbank");
	map(0xc000, 0xc7ff).ram();
	map(0xc800, 0xc801).rw("2203a", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xd000, 0xd001).rw("ym3812", FUNC(ym3812_device::read), FUNC(ym3812_device::write));
	map(0xd800, 0xd800).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xe000, 0xe000).w(m_adpcm_select[1], FUNC(ls157_device::ba_w));
	map(0xe800, 0xe800).w(FUNC(dec0_automat_state::sound_bankswitch_w));
	map(0xf000, 0xf000).w(m_adpcm_select[0], FUNC(ls157_device::ba_w));
}

/******************************************************************************/

static INPUT_PORTS_START( dec0 )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) /* Button 3 - only in Service Mode */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) /* Button 4 - only in Service Mode */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL /* Button 3 - only in Service Mode */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL /* Button 4 - only in Service Mode */

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON5 ) /* Button 5 - only in Service Mode */
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_COCKTAIL /* Button 5 - only in Service Mode */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END

static INPUT_PORTS_START( dec1 )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) /* Button 3 - only in Service Mode */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL /* Button 3 - only in Service Mode */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

#define DEC0_COIN_SETTING \
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2")\
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_2C ) ) \
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:3,4")\
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_2C ) )

static const ioport_value rotary_table[12] =
{
	0xfffe, 0xfffd, 0xfffb, 0xfff7,
	0xffef, 0xffdf, 0xffbf, 0xff7f,
	0xfeff, 0xfdff, 0xfbff, 0xf7ff
};

static INPUT_PORTS_START( rotary_ports )
	PORT_START("AN0")   /* player 1 12-way rotary control */
	PORT_BIT( 0xffff, 0x0000, IPT_POSITIONAL ) PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(10) PORT_KEYDELTA(1) PORT_REMAP_TABLE(rotary_table) PORT_REVERSE PORT_FULL_TURN_COUNT(12)

	PORT_START("AN1")   /* player 2 12-way rotary control */
	PORT_BIT( 0xffff, 0x0000, IPT_POSITIONAL ) PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(10) PORT_KEYDELTA(1) PORT_REMAP_TABLE(rotary_table) PORT_PLAYER(2) PORT_REVERSE PORT_FULL_TURN_COUNT(12)
INPUT_PORTS_END

static INPUT_PORTS_START( trackball_ports )
	PORT_START("track_0")
	PORT_BIT( 0x0fff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(24) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("track_1")
	PORT_BIT( 0x0fff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(24) PORT_PLAYER(1)

	PORT_START("track_2")
	PORT_BIT( 0x0fff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(24) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("track_3")
	PORT_BIT( 0x0fff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(24) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( rotary_null )
	PORT_START("AN0")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("AN1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( hbarrel )
	PORT_INCLUDE( dec0 )

	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Fire")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Bomb")
	PORT_BIT( 0x00c0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Fire")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Bomb")
	PORT_BIT( 0xc000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x0003, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_3C ) )
	PORT_SERVICE_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:8" ) // Always OFF

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "5" )
	PORT_DIPSETTING(      0x0000, "Infinite (Cheat)")
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x3000, "30k 80k 160k" )
	PORT_DIPSETTING(      0x2000, "50k 120k 190k" )
	PORT_DIPSETTING(      0x1000, "100k 200k 300k" )
	PORT_DIPSETTING(      0x0000, "150k 300k 450k" )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW1:8" ) // Always OFF

	PORT_INCLUDE( rotary_ports )
INPUT_PORTS_END

static INPUT_PORTS_START( bandit )
	PORT_INCLUDE( dec0 )

	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Fire")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Bomb")
	PORT_BIT( 0x00c0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Fire")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Bomb")
	PORT_BIT( 0xc000, IP_ACTIVE_LOW, IPT_UNUSED )

#if 0
	PORT_DIPNAME( 0x0001, 0x0001, "UNK_0" )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPNAME( 0x0002, 0x0002, "UNK_1" )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPNAME( 0x0004, 0x0004, "UNK_2" )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPNAME( 0x0008, 0x0008, "UNK_3" )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPNAME( 0x0010, 0x0010, "UNK_4" ) // Gun
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPNAME( 0x0020, 0x0020, "UNK_5" ) // Missile
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPNAME( 0x0040, 0x0040, "UNK_6" )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPNAME( 0x0080, 0x0080, "UNK_7" )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
#endif
	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x0003, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "Analog controls?" ) // ?
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPNAME( 0x0002, 0x0002, "L/R control related (keep off)" )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPNAME( 0x0004, 0x0004, "DSUNK_2" )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPNAME( 0x0008, 0x0000, "Road select (debug)" ) // Debug mode
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPNAME( 0x0020, 0x0020, "DSUNK_5" )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPNAME( 0x0040, 0x0040, "DSUNK_6" )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPNAME( 0x0080, 0x0000, "Enable enemies" )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )

	PORT_DIPNAME( 0x0100, 0x0100, "DSUNK_8" )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPNAME( 0x0200, 0x0200, "DSUNK_9" )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPNAME( 0x0400, 0x0400, "DSUNK_A" )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPNAME( 0x0800, 0x0800, "DSUNK_B" )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPNAME( 0x1000, 0x1000, "DSUNK_C" )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPNAME( 0x2000, 0x2000, "DSUNK_D" )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPNAME( 0x4000, 0x4000, "DSUNK_E" )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPNAME( 0x8000, 0x8000, "DSUNK_F" )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )

	PORT_INCLUDE( rotary_null )
	PORT_INCLUDE( trackball_ports )
INPUT_PORTS_END

static INPUT_PORTS_START( birdtry )
	PORT_INCLUDE( dec0 )

	PORT_MODIFY("INPUTS")
//  PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Shoot")
//  PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Select")
	PORT_BIT( 0x00c0, IP_ACTIVE_LOW, IPT_UNUSED )
//  PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Shoot")
//  PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Select")
	PORT_BIT( 0xc000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x0003, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	DEC0_COIN_SETTING
	PORT_SERVICE_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:8" ) // Always OFF

	PORT_DIPNAME( 0x0300, 0x0300, "Difficulty (Extend)" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Difficulty (Course)" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Timer" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "Fast" )
	PORT_DIPNAME( 0xc000, 0x0000, "Control Panel Type" ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0xc000, "Type A - Cocktail" )
	PORT_DIPSETTING(      0x8000, "Type B - Cocktail 2" )
	PORT_DIPSETTING(      0x4000, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0000, "Type C - Upright" )

	/* "Difficulty (Extend)"
	                Easy    Normal  Hard    Hardest
	(Start)         (5)     (3)     (3)     (3)
	Hole in one     +5      +5      +2      +1
	Albatross       +3      +3      +1       0
	Eagle           +2      +2      +1       0
	Birdie          +1      +1      +1       0
	Par              0       0       0       0
	Bogey           -1      -1      -1      -1
	Double bogey    -2      -2      -2      -1
	Triple bogey    -3      -3      -3      -1
	Quadruple bogey -4      -4      -4      -1
	Give up         -5      -5      -4      -2
	*/

	PORT_INCLUDE( rotary_null )
	PORT_INCLUDE( trackball_ports )
INPUT_PORTS_END

static INPUT_PORTS_START( baddudes )
	PORT_INCLUDE( dec0 )

	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Attack")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Jump")
	PORT_BIT( 0x00c0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Attack")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Jump")
	PORT_BIT( 0xc000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x0003, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	DEC0_COIN_SETTING
	PORT_SERVICE_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:8" ) // Always OFF
	/* "SW1:8"
	English "Bad Dudes" manual says "Dont Change"
	Japanese "Dragonninja" manual says "Control Panel / Off=Table / On=Upright", but maybe not work
	*/

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "5" )
	PORT_DIPSETTING(      0x0000, "Infinite (Cheat)")
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Yes ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPUNUSED_DIPLOC( 0x2000, IP_ACTIVE_LOW, "SW2:6" ) // Always OFF
	PORT_DIPUNUSED_DIPLOC( 0x4000, IP_ACTIVE_LOW, "SW2:7" ) // Always OFF
	PORT_DIPUNUSED_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW2:8" ) // Always OFF

	PORT_INCLUDE( rotary_null )
INPUT_PORTS_END

static INPUT_PORTS_START( drgninja )
	PORT_INCLUDE( baddudes )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0100, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "4" )
	PORT_DIPSETTING(      0x0000, "Infinite (Cheat)")

INPUT_PORTS_END

static INPUT_PORTS_START( robocop )
	PORT_INCLUDE( dec0 )

	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Attack")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Jump")
	PORT_BIT( 0x00c0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Attack")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Jump")
	PORT_BIT( 0xc000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x0003, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	DEC0_COIN_SETTING
	PORT_DIPUNUSED_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW1:5" ) // Always OFF
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Cocktail ) )

	PORT_DIPNAME( 0x0300, 0x0300, "Player Energy" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0100, DEF_STR( Low ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( High ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_High ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Bonus Stage Energy" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Low ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( High ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Brink Time" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "Less" )
	/* "SW2:7"
	English manual says "Always Off"
	Japanese manual says "Invulnerable Brink Time On Continue / Off=Long / On=Short"
	*/
	PORT_DIPUNUSED_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW2:8" ) // Always OFF

	PORT_INCLUDE( rotary_null )
INPUT_PORTS_END

static INPUT_PORTS_START( hippodrm )
	PORT_INCLUDE( dec0 )

	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Attack")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Jump")
	PORT_BIT( 0x00c0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Attack")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Jump")
	PORT_BIT( 0xc000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x0003, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	DEC0_COIN_SETTING
	PORT_DIPUNUSED_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW1:5" ) // Always OFF
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:8" ) // Always OFF

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0300, "2" )
	PORT_DIPSETTING(      0x0200, "3" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x3000, 0x3000, "Player & Enemy Energy" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x1000, DEF_STR( Very_Low ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Low ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( High ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Enemy Power Decrease on Continue" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, "2 Dots" )    // 2 Dots less
	PORT_DIPSETTING(      0x0000, "3 Dots" )    // 3 Dots less
	PORT_DIPUNUSED_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW2:8" ) // Always OFF

	PORT_INCLUDE( rotary_null )
INPUT_PORTS_END

static INPUT_PORTS_START( ffantasy )
	PORT_INCLUDE( hippodrm )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x4000, 0x4000, "Enemy Power Decrease on Continue" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, "2 Dots" )    // 2 Dots less
	PORT_DIPSETTING(      0x0000, DEF_STR( None ) ) // 0 Dot less
INPUT_PORTS_END

static INPUT_PORTS_START( ffantasybl )
	PORT_INCLUDE( hippodrm )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x4000, 0x4000, "Enemy Power Decrease on Continue" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, "2 Dots" )    // 2 Dots less
	PORT_DIPSETTING(      0x0000, DEF_STR( None ) ) // 0 Dot less

	PORT_START("VBLANK")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Game does not want vblank here
INPUT_PORTS_END

static INPUT_PORTS_START( slyspy )
	PORT_INCLUDE( dec1 )
	/* if you set VBLANK as ACTIVE_LOW, you obtain screwed up colors */

	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Attack")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Jump")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Attack")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Jump")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	DEC0_COIN_SETTING
	PORT_SERVICE_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Cocktail ) )

	PORT_DIPNAME( 0x0300, 0x0300, "Energy" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, "Low - 8 bars" )
	PORT_DIPSETTING(      0x0300, "Medium - 10 bars" )
	PORT_DIPSETTING(      0x0100, "High - 12 bars" )
	PORT_DIPSETTING(      0x0000, "Very High - 14 bars" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")   /* not mentioned in manual */
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x2000, IP_ACTIVE_LOW, "SW2:6" ) // Always OFF
	PORT_DIPUNUSED_DIPLOC( 0x4000, IP_ACTIVE_LOW, "SW2:7" ) // Always OFF
	PORT_DIPUNUSED_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW2:8" ) // Always OFF

	PORT_INCLUDE( rotary_null )
INPUT_PORTS_END

static INPUT_PORTS_START( midres )
	PORT_INCLUDE( dec1 )

	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Fire")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Jump")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Fire")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Jump")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	DEC0_COIN_SETTING
	PORT_DIPUNUSED_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW1:5" ) // Always OFF
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:8" ) // Always OFF

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "5" )
	PORT_DIPSETTING(      0x0000, "Infinite (Cheat)")
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x1000, IP_ACTIVE_LOW, "SW2:5" ) /* manual mentions Extra Lives - Default OFF */
	PORT_DIPUNUSED_DIPLOC( 0x2000, IP_ACTIVE_LOW, "SW2:6" ) /* manual mentions Extra Lives - Default OFF */
	/* "SW2:5,6"
	English manual says "Extra Lives / OFF OFF" ( missing "ON OFF", "OFF ON" and "ON ON" ) , but maybe not work
	Japanese manual says "Never Touch"
	*/
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Allow_Continue ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW2:8" ) // Always OFF

	PORT_INCLUDE( rotary_ports )
INPUT_PORTS_END

static INPUT_PORTS_START( midresb )
	PORT_INCLUDE( dec0 )

	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Fire")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Jump")
	PORT_BIT( 0x00c0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Fire")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Jump")
	PORT_BIT( 0xc000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x0003, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	DEC0_COIN_SETTING
	PORT_DIPUNUSED_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW1:5" ) // Always OFF
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:8" ) // Always OFF

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "5" )
	PORT_DIPSETTING(      0x0000, "Infinite (Cheat)")
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x1000, IP_ACTIVE_LOW, "SW2:5" ) /* manual mentions Extra Lives - Default OFF */
	PORT_DIPUNUSED_DIPLOC( 0x2000, IP_ACTIVE_LOW, "SW2:6" ) /* manual mentions Extra Lives - Default OFF */
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Allow_Continue ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW2:8" ) // Always OFF

	PORT_INCLUDE( rotary_ports )
INPUT_PORTS_END

static INPUT_PORTS_START( bouldash )
	PORT_INCLUDE( dec1 )

	PORT_MODIFY("INPUTS")
	// 4 way joysticks according to manual
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) // squeeze diamond
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) // escape
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED ) /* Button 3 - only in Service Mode */
//  PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL // squeeze diamond
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL // escape
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
//  PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")        /* extremely slow palette fades with ACTIVE_HIGH */

	PORT_START("DSW")
	/* Different Coinage. Just a few combinations from manual, the rest was figured out */
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Cocktail ) )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "4" )
	PORT_DIPSETTING(      0x0100, "5" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x1000, IP_ACTIVE_LOW, "SW2:5" ) // Always OFF
	PORT_DIPNAME( 0x2000, 0x2000, "Game Change Mode" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, "Part 1" )
	PORT_DIPSETTING(      0x0000, "Part 2" )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_INCLUDE( rotary_null )
INPUT_PORTS_END

/******************************************************************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 chars */
	RGN_FRAC(1,4),
	4,      /* 4 bits per pixel  */
	{ RGN_FRAC(0,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(3,4) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(1,4), RGN_FRAC(3,4), RGN_FRAC(0,4), RGN_FRAC(2,4) },
	{ STEP8(16*8,1), STEP8(0,1) },
	{ STEP16(0,8) },
	16*16
};

static const gfx_layout automat_spritelayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(1,4), RGN_FRAC(3,4), RGN_FRAC(0,4), RGN_FRAC(2,4) },
	{ STEP8(16*8+7,-1), STEP8(7,-1) },
	{ STEP16(0,8) },
	16*16
};

static const gfx_layout automat_tilelayout3 =
{
	16,16,
	0x800,
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(3,4) },
	{ 0,1,2,3,4,5,6,7,0x8000*8+0, 0x8000*8+1, 0x8000*8+2, 0x8000*8+3, 0x8000*8+4, 0x8000*8+5, 0x8000*8+6, 0x8000*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 0x4000*8+0*8, 0x4000*8+1*8, 0x4000*8+2*8, 0x4000*8+3*8, 0x4000*8+4*8, 0x4000*8+5*8, 0x4000*8+6*8, 0x4000*8+7*8 },
	8*8
};


static const gfx_layout automat_tilelayout2 =
{
	16,16,
	0x400, // RGN_FRAC(1,16) causes divide by zero?!
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(3,4) },
	{ 0,1,2,3,4,5,6,7, 0x4000*8+0, 0x4000*8+1, 0x4000*8+2, 0x4000*8+3, 0x4000*8+4, 0x4000*8+5, 0x4000*8+6, 0x4000*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 0x2000*8+0*8, 0x2000*8+1*8, 0x2000*8+2*8, 0x2000*8+3*8, 0x2000*8+4*8, 0x2000*8+5*8, 0x2000*8+6*8, 0x2000*8+7*8 },
	8*8
};


static GFXDECODE_START( gfx_dec0 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 16 )   /* Characters 8x8 */
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 512, 16 )   /* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout, 768, 16 )   /* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx4", 0, tilelayout, 256, 16 )   /* Sprites 16x16 */
GFXDECODE_END

static GFXDECODE_START( gfx_midres )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 256, 16 )   /* Characters 8x8 */
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 512, 16 )   /* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout, 768, 16 )   /* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx4", 0, tilelayout,   0, 16 )   /* Sprites 16x16 */
GFXDECODE_END

static GFXDECODE_START( gfx_automat )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 16 )   /* Characters 8x8 */
	GFXDECODE_ENTRY( "gfx2", 0, automat_tilelayout3, 512, 16 )  /* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx3", 0, automat_tilelayout2, 768, 16 )  /* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx4", 0, automat_spritelayout, 256, 16 ) /* Sprites 16x16 */
GFXDECODE_END

static GFXDECODE_START( gfx_secretab )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0x000, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0, automat_tilelayout2,     0x200, 0x10 )
	GFXDECODE_ENTRY( "gfx3", 0, automat_tilelayout2,     0x300, 0x10 )
	GFXDECODE_ENTRY( "gfx4", 0, automat_spritelayout,    0x100, 0x10 )
GFXDECODE_END

/******************************************************************************/



// DECO video CRTC, pixel clock is unverified (actually 24MHz/4?)
void dec0_state::set_screen_raw_params_data_east(machine_config &config)
{
	m_screen->set_raw(XTAL(12'000'000)/2,384,0,256,272,8,248);
}

void dec0_state::dec0_base(machine_config &config)
{
	/* video hardware */
	BUFFERED_SPRITERAM16(config, m_spriteram);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	set_screen_raw_params_data_east(config);
	//m_screen->set_refresh_hz(57.41);
	//m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(529)); /* 57.41 Hz, 529us Vblank */
	//m_screen->set_size(32*8, 32*8);
	//m_screen->set_visarea(0*8, 32*8-1, 1*8, 31*8-1);
	//screen update callback differs per game
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_dec0);
	PALETTE(config, m_palette);

	DECO_BAC06(config, m_tilegen[0], 0);
	m_tilegen[0]->set_gfx_region_wide(0, 0, 0);
	m_tilegen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO_BAC06(config, m_tilegen[1], 0);
	m_tilegen[1]->set_gfx_region_wide(0, 1, 0);
	m_tilegen[1]->set_gfxdecode_tag(m_gfxdecode);

	DECO_BAC06(config, m_tilegen[2], 0);
	m_tilegen[2]->set_gfx_region_wide(0, 2, 0);
	m_tilegen[2]->set_gfxdecode_tag(m_gfxdecode);

	DECO_MXC06(config, m_spritegen, 0);

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
}

void dec0_state::dec0(machine_config &config)
{
	dec0_base(config);

	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(20'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &dec0_state::dec0_map);
	m_maincpu->set_vblank_int("screen", FUNC(dec0_state::irq6_line_assert)); /* VBL */

	M6502(config, m_audiocpu, XTAL(12'000'000) / 8);
	m_audiocpu->set_addrmap(AS_PROGRAM, &dec0_state::dec0_s_map);

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(dec0_state,dec0)

	m_palette->set_format(palette_device::xBGR_888, 1024);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2203_device &ym1(YM2203(config, "ym1", XTAL(12'000'000) / 8));
	ym1.add_route(0, "mono", 0.81);
	ym1.add_route(1, "mono", 0.81);
	ym1.add_route(2, "mono", 0.81);
	ym1.add_route(3, "mono", 0.32);
	ym1.irq_handler().set_inputline(m_audiocpu, 0); // Schematics show both ym2203 and ym3812 can trigger IRQ, but Bandit is only game to program 2203 to do so

	ym3812_device &ym2(YM3812(config, "ym2", XTAL(12'000'000) / 4));
	ym2.irq_handler().set_inputline(m_audiocpu, 0);
	ym2.add_route(ALL_OUTPUTS, "mono", 0.72);

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(20'000'000) / 2 / 10, okim6295_device::PIN7_HIGH));
	oki.add_route(ALL_OUTPUTS, "mono", 0.72);
}


void dec0_state::dec1(machine_config &config)
{
	dec0_base(config);
	/* basic machine hardware */
	/* maincpu and audiocpu clocks and address maps differ per game */

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(dec0_state,dec0_nodma)

	m_palette->set_format(palette_device::xBGR_444, 1024);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2203_device &ym1(YM2203(config, "ym1", XTAL(12'000'000)/8)); /* verified on pcb */
	ym1.add_route(0, "mono", 0.90);
	ym1.add_route(1, "mono", 0.90);
	ym1.add_route(2, "mono", 0.90);
	ym1.add_route(3, "mono", 0.35);

	ym3812_device &ym2(YM3812(config, "ym2", XTAL(12'000'000)/4)); /* verified on pcb */
	ym2.irq_handler().set_inputline(m_audiocpu, 1);
	ym2.add_route(ALL_OUTPUTS, "mono", 0.80);

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(12'000'000)/12, okim6295_device::PIN7_HIGH)); /* verified on pcb */
	oki.add_route(ALL_OUTPUTS, "mono", 0.80);
}


void dec0_automat_state::sound_bankswitch_w(uint8_t data)
{
	m_msm[0]->reset_w(BIT(data, 3));
	m_msm[1]->reset_w(BIT(data, 4));

	m_soundbank->set_entry(data & 7);
}

WRITE_LINE_MEMBER(dec0_automat_state::msm1_vclk_cb)
{
	if (!state)
		return;

	m_adpcm_toggle[0] = !m_adpcm_toggle[0];
	m_adpcm_select[0]->select_w(m_adpcm_toggle[0]);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, m_adpcm_toggle[0]);
}

WRITE_LINE_MEMBER(dec0_automat_state::msm2_vclk_cb)
{
	if (!state)
		return;

	m_adpcm_toggle[1] = !m_adpcm_toggle[1];
	m_adpcm_select[1]->select_w(m_adpcm_toggle[1]);
}


void dec0_automat_state::automat(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 10000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &dec0_automat_state::automat_map);
	m_maincpu->set_vblank_int("screen", FUNC(dec0_state::irq6_line_hold)); /* VBL */

	Z80(config, m_audiocpu, 3000000); // ?
	m_audiocpu->set_addrmap(AS_PROGRAM, &dec0_automat_state::automat_s_map);

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(dec0_automat_state,dec0_nodma)

	BUFFERED_SPRITERAM16(config, m_spriteram);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
//  m_screen->set_refresh_hz(57.41);
//  m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(529)); /* 57.41 Hz, 529us Vblank */
	set_screen_raw_params_data_east(config);
	m_screen->set_screen_update(FUNC(dec0_automat_state::screen_update_automat));
	m_screen->set_palette(m_palette);

	DECO_BAC06(config, m_tilegen[0], 0);
	m_tilegen[0]->set_gfx_region_wide(0, 0, 0);
	m_tilegen[0]->set_gfxdecode_tag("gfxdecode");

	DECO_BAC06(config, m_tilegen[1], 0);
	m_tilegen[1]->set_gfx_region_wide(0, 1, 0);
	m_tilegen[1]->set_gfxdecode_tag("gfxdecode");

	DECO_BAC06(config, m_tilegen[2], 0);
	m_tilegen[2]->set_gfx_region_wide(0, 2, 0);
	m_tilegen[2]->set_gfxdecode_tag("gfxdecode");

	DECO_MXC06(config, m_spritegen, 0);
	m_spritegen->set_colpri_callback(FUNC(dec0_automat_state::robocop_colpri_cb));

	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 1024);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_automat);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, 0);

	ym2203_device &ym2203a(YM2203(config, "2203a", 1250000));
	ym2203a.add_route(0, "mono", 0.90);
	ym2203a.add_route(1, "mono", 0.90);
	ym2203a.add_route(2, "mono", 0.90);
	ym2203a.add_route(3, "mono", 0.35);

	ym2203_device &ym2203b(YM2203(config, "2203b", 1250000));
	ym2203b.add_route(0, "mono", 0.90);
	ym2203b.add_route(1, "mono", 0.90);
	ym2203b.add_route(2, "mono", 0.90);
	ym2203b.add_route(3, "mono", 0.35);

	LS157(config, m_adpcm_select[0], 0);
	m_adpcm_select[0]->out_callback().set("msm1", FUNC(msm5205_device::data_w));

	LS157(config, m_adpcm_select[1], 0);
	m_adpcm_select[1]->out_callback().set("msm2", FUNC(msm5205_device::data_w));

	msm5205_device &msm1(MSM5205(config, "msm1", 384000));
	msm1.vck_legacy_callback().set(FUNC(dec0_automat_state::msm1_vclk_cb));
	msm1.set_prescaler_selector(msm5205_device::S96_4B);
	msm1.add_route(ALL_OUTPUTS, "mono", 1.0);

	msm5205_device &msm2(MSM5205(config, "msm2", 384000));
	msm2.vck_legacy_callback().set(FUNC(dec0_automat_state::msm2_vclk_cb));
	msm2.set_prescaler_selector(msm5205_device::S96_4B);
	msm2.add_route(ALL_OUTPUTS, "mono", 1.0);
}

// this seems very similar to the automat bootleg
void dec0_automat_state::secretab(machine_config &config) // all clocks verified on PCB
{
	// basic machine hardware
	M68000(config, m_maincpu, 20_MHz_XTAL / 2); // verified on pcb (20MHZ OSC) 68000P12 running at 10Mhz
	m_maincpu->set_addrmap(AS_PROGRAM, &dec0_automat_state::secretab_map);
	m_maincpu->set_vblank_int("screen", FUNC(dec0_state::irq6_line_hold)); // VBL

	Z80(config, m_audiocpu, 20_MHz_XTAL / 4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &dec0_automat_state::secretab_s_map);

	// video hardware
	MCFG_VIDEO_START_OVERRIDE(dec0_automat_state,slyspy)

	BUFFERED_SPRITERAM16(config, m_spriteram);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
//  m_screen->set_refresh_hz(57.41);
//  m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(529)); // 57.41 Hz, 529us Vblank
	set_screen_raw_params_data_east(config);
	m_screen->set_screen_update(FUNC(dec0_automat_state::screen_update_secretab));
	m_screen->set_palette(m_palette);

	DECO_BAC06(config, m_tilegen[0], 0);
	m_tilegen[0]->set_gfx_region_wide(0, 0, 0);
	m_tilegen[0]->set_gfxdecode_tag("gfxdecode");

	DECO_BAC06(config, m_tilegen[1], 0);
	m_tilegen[1]->set_gfx_region_wide(0, 1, 0);
	m_tilegen[1]->set_gfxdecode_tag("gfxdecode");
	m_tilegen[1]->set_tile_callback(FUNC(dec0_automat_state::baddudes_tile_cb));

	DECO_BAC06(config, m_tilegen[2], 0);
	m_tilegen[2]->set_gfx_region_wide(0, 2, 0);
	m_tilegen[2]->set_gfxdecode_tag("gfxdecode");

	DECO_MXC06(config, m_spritegen, 0);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 1024);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_secretab);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, 0);

	ym2203_device &ym2203a(YM2203(config, "2203a", 20_MHz_XTAL / 16));
	ym2203a.add_route(0, "mono", 0.90);
	ym2203a.add_route(1, "mono", 0.90);
	ym2203a.add_route(2, "mono", 0.90);
	ym2203a.add_route(3, "mono", 0.35);

	ym3812_device &ym3812(YM3812(config, "ym3812", 20_MHz_XTAL / 8));
	ym3812.add_route(ALL_OUTPUTS, "mono", 0.80);

	LS157(config, m_adpcm_select[0], 0);
	m_adpcm_select[0]->out_callback().set("msm1", FUNC(msm5205_device::data_w));

	LS157(config, m_adpcm_select[1], 0);
	m_adpcm_select[1]->out_callback().set("msm2", FUNC(msm5205_device::data_w));

	msm5205_device &msm1(MSM5205(config, "msm1", 400_kHz_XTAL));
	msm1.vck_legacy_callback().set(FUNC(dec0_automat_state::msm1_vclk_cb));
	msm1.set_prescaler_selector(msm5205_device::S96_4B);
	msm1.add_route(ALL_OUTPUTS, "mono", 1.0);

	msm5205_device &msm2(MSM5205(config, "msm2", 400_kHz_XTAL));
	msm2.vck_legacy_callback().set(FUNC(dec0_automat_state::msm2_vclk_cb));
	msm2.set_prescaler_selector(msm5205_device::S96_4B);
	msm2.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void dec0_state::hbarrel(machine_config &config)
{
	dec0(config);

	i8751_device &mcu(I8751(config, m_mcu, XTAL(8'000'000)));
	mcu.port_in_cb<0>().set(FUNC(dec0_state::dec0_mcu_port0_r));
	mcu.port_out_cb<0>().set(FUNC(dec0_state::dec0_mcu_port0_w));
	mcu.port_out_cb<1>().set(FUNC(dec0_state::dec0_mcu_port1_w));
	mcu.port_out_cb<2>().set(FUNC(dec0_state::dec0_mcu_port2_w));
	mcu.port_out_cb<3>().set(FUNC(dec0_state::dec0_mcu_port3_w));

	/* video hardware */
	m_screen->set_screen_update(FUNC(dec0_state::screen_update_hbarrel));
	m_spritegen->set_colpri_callback(FUNC(dec0_state::hbarrel_colpri_cb));
}

void dec0_state::bandit(machine_config &config)
{
	dec0(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &dec0_state::dec0_tb_map);

	upd4701_device &tb0(UPD4701A(config, "tb0"));
	tb0.set_portx_tag("track_0");
	tb0.set_porty_tag("track_1");

	upd4701_device &tb1(UPD4701A(config, "tb1"));
	tb1.set_portx_tag("track_2");
	tb1.set_porty_tag("track_3");

	i8751_device &mcu(I8751(config, m_mcu, XTAL(8'000'000)));
	mcu.port_in_cb<0>().set(FUNC(dec0_state::dec0_mcu_port0_r));
	mcu.port_out_cb<0>().set(FUNC(dec0_state::dec0_mcu_port0_w));
	mcu.port_out_cb<1>().set(FUNC(dec0_state::dec0_mcu_port1_w));
	mcu.port_out_cb<2>().set(FUNC(dec0_state::dec0_mcu_port2_w));
	mcu.port_out_cb<3>().set(FUNC(dec0_state::dec0_mcu_port3_w));

	/* video hardware */
	m_screen->set_screen_update(FUNC(dec0_state::screen_update_hbarrel));
	m_spritegen->set_colpri_callback(FUNC(dec0_state::bandit_colpri_cb));
}

void dec0_state::baddudes(machine_config &config)
{
	dec0(config);

	i8751_device &mcu(I8751(config, m_mcu, XTAL(8'000'000)));
	mcu.port_in_cb<0>().set(FUNC(dec0_state::dec0_mcu_port0_r));
	mcu.port_out_cb<0>().set(FUNC(dec0_state::dec0_mcu_port0_w));
	mcu.port_out_cb<1>().set(FUNC(dec0_state::dec0_mcu_port1_w));
	mcu.port_out_cb<2>().set(FUNC(dec0_state::dec0_mcu_port2_w));
	mcu.port_out_cb<3>().set(FUNC(dec0_state::dec0_mcu_port3_w));

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(dec0_state,baddudes)

	m_tilegen[1]->set_tile_callback(FUNC(dec0_state::baddudes_tile_cb));
	m_tilegen[2]->set_tile_callback(FUNC(dec0_state::baddudes_tile_cb));

	m_screen->set_screen_update(FUNC(dec0_state::screen_update_baddudes));
}

void dec0_state::drgninjab(machine_config &config)
{
	dec0(config);

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(dec0_state,baddudes)

	m_tilegen[1]->set_tile_callback(FUNC(dec0_state::baddudes_tile_cb));
	m_tilegen[2]->set_tile_callback(FUNC(dec0_state::baddudes_tile_cb));

	m_screen->set_screen_update(FUNC(dec0_state::screen_update_baddudes));
}

void dec0_state::birdtry(machine_config &config)
{
	dec0(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &dec0_state::dec0_tb_map);

	// needs a tight sync with the mcu
	config.set_perfect_quantum(m_maincpu);

	i8751_device &mcu(I8751(config, m_mcu, XTAL(8'000'000)));
	mcu.port_in_cb<0>().set(FUNC(dec0_state::dec0_mcu_port0_r));
	mcu.port_out_cb<0>().set(FUNC(dec0_state::dec0_mcu_port0_w));
	mcu.port_out_cb<1>().set(FUNC(dec0_state::dec0_mcu_port1_w));
	mcu.port_out_cb<2>().set(FUNC(dec0_state::dec0_mcu_port2_w));
	mcu.port_out_cb<3>().set(FUNC(dec0_state::dec0_mcu_port3_w));

	upd4701_device &tb0(UPD4701A(config, "tb0"));
	tb0.set_portx_tag("track_0");
	tb0.set_porty_tag("track_1");

	upd4701_device &tb1(UPD4701A(config, "tb1"));
	tb1.set_portx_tag("track_2");
	tb1.set_porty_tag("track_3");

	/* video hardware */
	m_screen->set_screen_update(FUNC(dec0_state::screen_update_birdtry));
}

void dec0_state::robocop(machine_config &config)
{
	dec0(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &dec0_state::robocop_map);

	H6280(config, m_subcpu, XTAL(21'477'272) / 16);
	m_subcpu->set_addrmap(AS_PROGRAM, &dec0_state::robocop_sub_map);
	m_subcpu->add_route(ALL_OUTPUTS, "mono", 0); // internal sound unused

	mb8421_device &dem01(MB8421(config, "dem01"));
	dem01.intl_callback().set_inputline(m_maincpu, M68K_IRQ_4);
	dem01.intr_callback().set_inputline(m_subcpu, 0);

	config.set_maximum_quantum(attotime::from_hz(3000));  /* Interleave between HuC6280 & 68000 */

	/* video hardware */
	m_screen->set_screen_update(FUNC(dec0_state::screen_update_robocop));
	m_spritegen->set_colpri_callback(FUNC(dec0_state::robocop_colpri_cb));
}

void dec0_state::robocopb(machine_config &config)
{
	dec0(config);

	/* video hardware */
	m_screen->set_screen_update(FUNC(dec0_state::screen_update_robocop));
	m_spritegen->set_colpri_callback(FUNC(dec0_state::robocop_colpri_cb));
}

void dec0_state::hippodrm(machine_config &config)
{
	dec0(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &dec0_state::hippodrm_map);

	H6280(config, m_subcpu, XTAL(21'477'272) / 16);
	m_subcpu->set_addrmap(AS_PROGRAM, &dec0_state::hippodrm_sub_map);
	m_subcpu->add_route(ALL_OUTPUTS, "mono", 0); // internal sound unused

	config.set_maximum_quantum(attotime::from_hz(300));   /* Interleave between H6280 & 68000 */

	/* video hardware */
	m_screen->set_screen_update(FUNC(dec0_state::screen_update_robocop));
	m_screen->screen_vblank().set_inputline(m_subcpu, 1); /* VBL */
}

void dec0_state::ffantasybl(machine_config &config)
{
	dec0(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &dec0_state::ffantasybl_map);

//  H6280(config, m_subcpu, XTAL(21'477'272) / 16);
//  m_subcpu->set_addrmap(AS_PROGRAM, &dec0_state::hippodrm_sub_map);
//  m_subcpu->add_route(ALL_OUTPUTS, "mono", 0); // internal sound unused

//  config.set_maximum_quantum(attotime::from_hz(300));   /* Interleave between H6280 & 68000 */

	/* video hardware */
	m_screen->set_screen_update(FUNC(dec0_state::screen_update_robocop));
}

MACHINE_RESET_MEMBER(dec0_state,slyspy)
{
	// set initial memory map
	m_slyspy_state = 0;
	m_pfprotect->set_bank(m_slyspy_state);
	m_slyspy_sound_state = 0;
	m_sndprotect->set_bank(m_slyspy_sound_state);
}

void dec0_state::slyspy(machine_config &config)
{
	dec1(config);

	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(20'000'000)/2); /* verified on pcb (20MHZ OSC) 68000P12 running at 10Mhz */
	m_maincpu->set_addrmap(AS_PROGRAM, &dec0_state::slyspy_map);
	m_maincpu->set_vblank_int("screen", FUNC(dec0_state::irq6_line_hold)); /* VBL, apparently it auto-acks */

	// TODO: both games doesn't like /3 here, MT #06740
	h6280_device &audiocpu(H6280(config, m_audiocpu, XTAL(12'000'000)/2/2)); /* verified on pcb (6Mhz is XIN on pin 10 of H6280) */
	audiocpu.set_addrmap(AS_PROGRAM, &dec0_state::slyspy_s_map);
	audiocpu.add_route(ALL_OUTPUTS, "mono", 0); // internal sound unused

	ADDRESS_MAP_BANK(config, "pfprotect").set_map(&dec0_state::slyspy_protection_map).set_options(ENDIANNESS_BIG, 16, 18, 0x10000);
	ADDRESS_MAP_BANK(config, "sndprotect").set_map(&dec0_state::slyspy_sound_protection_map).set_options(ENDIANNESS_LITTLE, 8, 21, 0x80000);

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(dec0_state,slyspy)

	m_tilegen[1]->set_tile_callback(FUNC(dec0_state::baddudes_tile_cb));

	m_screen->set_screen_update(FUNC(dec0_state::screen_update_slyspy));

	MCFG_MACHINE_RESET_OVERRIDE(dec0_state,slyspy)
}

void dec0_state::midres(machine_config &config)
{
	dec1(config);

	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(20'000'000)/2); /* verified on pcb (20MHZ OSC) 68000P12 running at 10Mhz */
	m_maincpu->set_addrmap(AS_PROGRAM, &dec0_state::midres_map);
	m_maincpu->set_vblank_int("screen", FUNC(dec0_state::irq6_line_hold)); /* VBL */

	h6280_device &audiocpu(H6280(config, m_audiocpu, XTAL(24'000'000)/4/3)); /* verified on pcb (6Mhz is XIN on pin 10 of H6280, verified on pcb */
	audiocpu.set_addrmap(AS_PROGRAM, &dec0_state::midres_s_map);
	audiocpu.add_route(ALL_OUTPUTS, "mono", 0); // internal sound unused

	/* video hardware */
	m_screen->set_screen_update(FUNC(dec0_state::screen_update_robocop));
	m_spritegen->set_colpri_callback(FUNC(dec0_state::midres_colpri_cb));

	m_gfxdecode->set_info(gfx_midres);

	subdevice<okim6295_device>("oki")->set_clock(XTAL(1'056'000));
}

void dec0_state::midresb(machine_config &config)
{
	midres(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &dec0_state::midresb_map);

	M6502(config.replace(), m_audiocpu, 1500000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &dec0_state::dec0_s_map);

	M68705R3(config, m_mcu, XTAL(3'579'545));

	subdevice<ym3812_device>("ym2")->irq_handler().set_inputline(m_audiocpu, 0);
	subdevice<ym3812_device>("ym2")->add_route(ALL_OUTPUTS, "mono", 0.80);

	// bootleg doesn't seem to support row/col scroll (or enable is different)
//  m_tilegen[0]->disable_16x16();
//  m_tilegen[0]->disable_rc_scroll();

//  m_tilegen[1]->disable_8x8();
	m_tilegen[1]->disable_rc_scroll();

//  m_tilegen[2]->disable_8x8();
	m_tilegen[2]->disable_rc_scroll();
}

void dec0_state::midresbj(machine_config &config)
{
	midresb(config);
	config.device_remove("mcu");
}

/******************************************************************************/

ROM_START( hbarrel ) /* DE-0289-2 main board, DE-0293-1 sub/rom board */
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "ec04-e.3c", 0x00000, 0x10000, CRC(d01bc3db) SHA1(53c9b78ce12ab577111fd96ef793b0fc4131bec3) )
	ROM_LOAD16_BYTE( "ec01-e.3a", 0x00001, 0x10000, CRC(6756f8ae) SHA1(4edea085dedab46995b07d134b0974e365c32bfe) )
	ROM_LOAD16_BYTE( "ec05.4c",   0x20000, 0x10000, CRC(2087d570) SHA1(625a33c2f4feed56f636d318531d0996cdee9194) )
	ROM_LOAD16_BYTE( "ec02.4a",   0x20001, 0x10000, CRC(815536ae) SHA1(684f67dc92f2a3bd77effce68c50e4013e054d31) )
	ROM_LOAD16_BYTE( "ec06.6c",   0x40000, 0x10000, CRC(61ec20d8) SHA1(9cd87fb896e746dc7745c59396cf5b06a9c6fae1) )
	ROM_LOAD16_BYTE( "ec03.6a",   0x40001, 0x10000, CRC(720c6b13) SHA1(2af04de911f759b20ecec3aaf96238545c6cc987) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 Sound */
	ROM_LOAD( "ec07.8a", 0x8000, 0x8000, CRC(16a5a1aa) SHA1(27eb8c09be6b1be502bda9ae9c9ff860d2560d46) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* i8751 microcontroller */
	ROM_LOAD( "ec31-e.9a", 0x0000, 0x1000, CRC(bf93a2ec) SHA1(e62687c9ae280485835b3bc56c161618f47c56a1) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "ec25.15h", 0x00000, 0x10000, CRC(2e5732a2) SHA1(b730ce11db1876b81d2b7cde0f129bd6fbfeb771) )
	ROM_LOAD( "ec26.16h", 0x10000, 0x10000, CRC(161a2c4d) SHA1(fbfa97ecc8b4d540d38f811ebb6272b348ed37e5) )

	ROM_REGION( 0x80000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "ec18.14d", 0x00000, 0x10000, CRC(ef664373) SHA1(d66a8c685c44cc8583527297d7ea7778f0d9c8db) )
	ROM_LOAD( "ec17.12d", 0x10000, 0x10000, CRC(a4f186ac) SHA1(ee422f8479c1f21bb62d040567a9748b646e6f9f) )
	ROM_LOAD( "ec20.17d", 0x20000, 0x10000, CRC(2fc13be0) SHA1(cce46b91104c0ac4038e98131fe957e0ed2f1a88) )
	ROM_LOAD( "ec19.15d", 0x30000, 0x10000, CRC(d6b47869) SHA1(eaef6ed5505395b1b829d6a126363031ad4e851a) )
	ROM_LOAD( "ec22.14f", 0x40000, 0x10000, CRC(50d6a1ad) SHA1(e7b464f34d6f3796823de6fdcbfd79416f71a119) )
	ROM_LOAD( "ec21.12f", 0x50000, 0x10000, CRC(f01d75c5) SHA1(959f9e2461db5f08b7ab12cc3b43f33be69318c9) )
	ROM_LOAD( "ec24.17f", 0x60000, 0x10000, CRC(ae377361) SHA1(a9aa520044f5b5037a495402ef128d3d8522b20f) )
	ROM_LOAD( "ec23.15f", 0x70000, 0x10000, CRC(bbdaf771) SHA1(7b29d6d606319337562b0431b6290df15cde17e2) )

	ROM_REGION( 0x40000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "ec29.8h", 0x00000, 0x10000, CRC(5514b296) SHA1(d258134a95bb223db139780b8e7377cccbe01af0) )
	ROM_LOAD( "ec30.9h", 0x10000, 0x10000, CRC(5855e8ef) SHA1(0f09143fed7c354231a4f343d0371424d8436877) )
	ROM_LOAD( "ec27.8f", 0x20000, 0x10000, CRC(99db7b9c) SHA1(2faeb287d685c8ea72c21658777f62ff9e194a69) )
	ROM_LOAD( "ec28.9f", 0x30000, 0x10000, CRC(33ce2b1a) SHA1(ef150dd5bc22368857ba27da18a17c161bb807a4) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "ec15.16c", 0x00000, 0x10000, CRC(21816707) SHA1(859a70dfc7d8c01124a035dcd5ea554af5f4e871) )
	ROM_LOAD( "ec16.17c", 0x10000, 0x10000, CRC(a5684574) SHA1(2dfe429cd6e110645ab976dd3a2b27d54ad91e89) )
	ROM_LOAD( "ec11.16a", 0x20000, 0x10000, CRC(5c768315) SHA1(00905e59dec90bf51f1d8e2482f54ede0895d142) )
	ROM_LOAD( "ec12.17a", 0x30000, 0x10000, CRC(8b64d7a4) SHA1(4d880d97a8eabd9b0a50cba3357df4f70afdf909) )
	ROM_LOAD( "ec13.13c", 0x40000, 0x10000, CRC(56e3ed65) SHA1(e7e4a53a7a18c81af8e395a33bcd82a41482c0da) )
	ROM_LOAD( "ec14.14c", 0x50000, 0x10000, CRC(bedfe7f3) SHA1(9db9c632fbf5a98d2d21bb960cc7111f6f9410fc) )
	ROM_LOAD( "ec09.13a", 0x60000, 0x10000, CRC(26240ea0) SHA1(25732986d787afd99a045ce4587f1079f84e675b) )
	ROM_LOAD( "ec10.14a", 0x70000, 0x10000, CRC(47d95447) SHA1(d2ffe96a19cfcbddee0df07dad89bd83cba801fa) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "ec08.2c", 0x0000, 0x10000, CRC(2159a609) SHA1(cae503e446c7164a44b59886680f554a4cb1eef2) )

	ROM_REGION( 0x600, "proms", 0 ) /* PROMs */
	ROM_LOAD( "mb7116e.12c", 0x000, 0x200, CRC(86e775f8) SHA1(e8dee3d56fb5ca0fd7f9ce05a84674abb139d008) ) /* Also known to be labeled as A-1 */
	ROM_LOAD( "mb7122e.17e", 0x200, 0x400, CRC(a5cda23e) SHA1(d6c8534ae3c95b47a0701047fef67f15dd71f3fe) ) /* Also known to be labeled as A-2 */
ROM_END

/*
There are known PCBs with ROMs 01 & 04 stamped as "-1" revision
At least 1 PCB was spotted with ROM labels as "MYF HEAVY BARREL 01-2" & "MYF HEAVY BARREL 04-2"
Niether version has been dumped to verify they are indeed different versions
*/
ROM_START( hbarrelu ) /* DE-0297-1 main board, DE-0299-0 sub/rom board */
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "heavy_barrel_04.3c", 0x00000, 0x10000, CRC(4877b09e) SHA1(30c653b2f59fece881d088b675192ff2599adbe3) )
	ROM_LOAD16_BYTE( "heavy_barrel_01.3a", 0x00001, 0x10000, CRC(8b41c219) SHA1(5155095f459c29bd1fa5b3e8e2555db20a3bcfbc) )
	ROM_LOAD16_BYTE( "heavy_barrel_05.4c", 0x20000, 0x10000, CRC(2087d570) SHA1(625a33c2f4feed56f636d318531d0996cdee9194) )
	ROM_LOAD16_BYTE( "heavy_barrel_02.4a", 0x20001, 0x10000, CRC(815536ae) SHA1(684f67dc92f2a3bd77effce68c50e4013e054d31) )
	ROM_LOAD16_BYTE( "heavy_barrel_06.6c", 0x40000, 0x10000, CRC(da4e3fbc) SHA1(afc054eb5ee1d64d69fd8134d62e7c2d90f775c8) )
	ROM_LOAD16_BYTE( "heavy_barrel_03.6a", 0x40001, 0x10000, CRC(7fed7c46) SHA1(697742a18a0b01acadb0bbddc54331ab7e097bd8) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 Sound */
	ROM_LOAD( "heavy_barrel_07.8a", 0x8000, 0x8000, CRC(a127f0f7) SHA1(2cf962410936ac336e384dda2bf434a297bc940f) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* i8751 microcontroller */
	ROM_LOAD( "heavy_barrel_31.9a", 0x0000, 0x1000, CRC(239d726f) SHA1(969f38ae981ffde6053ece93cc51614d492edbbb) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "heavy_barrel_25.15h", 0x00000, 0x10000, CRC(8649762c) SHA1(84d3d82d4d011c54271ef7a0dc5857a34b61cf8a) )
	ROM_LOAD( "heavy_barrel_26.16h", 0x10000, 0x10000, CRC(f8189bbd) SHA1(b4445f50e8771af6ba4fcbc34018f6ecd379779a) )

	ROM_REGION( 0x80000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "heavy_barrel_18.14d", 0x00000, 0x10000, CRC(ef664373) SHA1(d66a8c685c44cc8583527297d7ea7778f0d9c8db) )
	ROM_LOAD( "heavy_barrel_17.12d", 0x10000, 0x10000, CRC(a4f186ac) SHA1(ee422f8479c1f21bb62d040567a9748b646e6f9f) )
	ROM_LOAD( "heavy_barrel_20.17d", 0x20000, 0x10000, CRC(2fc13be0) SHA1(cce46b91104c0ac4038e98131fe957e0ed2f1a88) )
	ROM_LOAD( "heavy_barrel_19.15d", 0x30000, 0x10000, CRC(d6b47869) SHA1(eaef6ed5505395b1b829d6a126363031ad4e851a) )
	ROM_LOAD( "heavy_barrel_22.14f", 0x40000, 0x10000, CRC(50d6a1ad) SHA1(e7b464f34d6f3796823de6fdcbfd79416f71a119) )
	ROM_LOAD( "heavy_barrel_21.12f", 0x50000, 0x10000, CRC(f01d75c5) SHA1(959f9e2461db5f08b7ab12cc3b43f33be69318c9) )
	ROM_LOAD( "heavy_barrel_24.17f", 0x60000, 0x10000, CRC(ae377361) SHA1(a9aa520044f5b5037a495402ef128d3d8522b20f) )
	ROM_LOAD( "heavy_barrel_23.15f", 0x70000, 0x10000, CRC(bbdaf771) SHA1(7b29d6d606319337562b0431b6290df15cde17e2) )

	ROM_REGION( 0x40000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "heavy_barrel_29.8h", 0x00000, 0x10000, CRC(5514b296) SHA1(d258134a95bb223db139780b8e7377cccbe01af0) )
	ROM_LOAD( "heavy_barrel_30.9h", 0x10000, 0x10000, CRC(5855e8ef) SHA1(0f09143fed7c354231a4f343d0371424d8436877) )
	ROM_LOAD( "heavy_barrel_27.8f", 0x20000, 0x10000, CRC(99db7b9c) SHA1(2faeb287d685c8ea72c21658777f62ff9e194a69) )
	ROM_LOAD( "heavy_barrel_28.9f", 0x30000, 0x10000, CRC(33ce2b1a) SHA1(ef150dd5bc22368857ba27da18a17c161bb807a4) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "heavy_barrel_15.16c", 0x00000, 0x10000, CRC(21816707) SHA1(859a70dfc7d8c01124a035dcd5ea554af5f4e871) )
	ROM_LOAD( "heavy_barrel_16.17c", 0x10000, 0x10000, CRC(a5684574) SHA1(2dfe429cd6e110645ab976dd3a2b27d54ad91e89) )
	ROM_LOAD( "heavy_barrel_11.16a", 0x20000, 0x10000, CRC(5c768315) SHA1(00905e59dec90bf51f1d8e2482f54ede0895d142) )
	ROM_LOAD( "heavy_barrel_12.17a", 0x30000, 0x10000, CRC(8b64d7a4) SHA1(4d880d97a8eabd9b0a50cba3357df4f70afdf909) )
	ROM_LOAD( "heavy_barrel_13.13c", 0x40000, 0x10000, CRC(56e3ed65) SHA1(e7e4a53a7a18c81af8e395a33bcd82a41482c0da) )
	ROM_LOAD( "heavy_barrel_14.14c", 0x50000, 0x10000, CRC(bedfe7f3) SHA1(9db9c632fbf5a98d2d21bb960cc7111f6f9410fc) )
	ROM_LOAD( "heavy_barrel_09.13a", 0x60000, 0x10000, CRC(26240ea0) SHA1(25732986d787afd99a045ce4587f1079f84e675b) )
	ROM_LOAD( "heavy_barrel_10.14a", 0x70000, 0x10000, CRC(47d95447) SHA1(d2ffe96a19cfcbddee0df07dad89bd83cba801fa) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "heavy_barrel_08.2c", 0x0000, 0x10000, CRC(645c5b68) SHA1(096ca5d7b5df752df6d2c856b3f94b29eea7c3de) )

	ROM_REGION( 0x600, "proms", 0 ) /* PROMs */
	ROM_LOAD( "mb7116e.12c", 0x000, 0x200, CRC(86e775f8) SHA1(e8dee3d56fb5ca0fd7f9ce05a84674abb139d008) ) /* Also known to be labeled as A-1 */
	ROM_LOAD( "mb7122e.17e", 0x200, 0x400, CRC(a5cda23e) SHA1(d6c8534ae3c95b47a0701047fef67f15dd71f3fe) ) /* Also known to be labeled as A-2 */
ROM_END


ROM_START( baddudes ) /* DE-0297-1 main board, DE-0299-1 sub/rom board */
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 6*64k for 68000 code, middle 0x20000 unused */
	ROM_LOAD16_BYTE( "ei04-1.3c", 0x00000, 0x10000, CRC(4bf158a7) SHA1(e034f64cec3e8596a2d86dd83462592178f19611) )
	ROM_LOAD16_BYTE( "ei01-1.3a", 0x00001, 0x10000, CRC(74f5110c) SHA1(9b8ff24e69505846a1406f5ab82b855b84a5cdf2) )
	ROM_LOAD16_BYTE( "ei06.6c",   0x40000, 0x10000, CRC(3ff8da57) SHA1(eea8125a3eac33d76d22e72b69633eaae138efe5) )
	ROM_LOAD16_BYTE( "ei03.6a",   0x40001, 0x10000, CRC(f8f2bd94) SHA1(622c66fea00cabb2cce16bf621b07d38a660708d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "ei07.8a",   0x8000, 0x8000, CRC(9fb1ef4b) SHA1(f4dd0773be93c2ad8b0faacd12939c531b5aa130) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* i8751 microcontroller */
	ROM_LOAD( "ei31.9a",   0x0000, 0x1000, CRC(2a8745d2) SHA1(f15ab17b1e7836d603135f5c66ca2e3d72f6e4a2) )

	ROM_REGION( 0x10000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "ei25.15h",  0x00000, 0x08000, CRC(bcf59a69) SHA1(486727e19c12ea55b47e2ef773d0d0471cf50083) )
	ROM_LOAD( "ei26.16h",  0x08000, 0x08000, CRC(9aff67b8) SHA1(18c3972a9f17a48897463f48be0d723ea0cf5aba) )

	ROM_REGION( 0x40000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "ei18.14d",  0x00000, 0x10000, CRC(05cfc3e5) SHA1(a0163921c77dc9706463a402c3dd45ec4341cd21) )
	ROM_LOAD( "ei20.17d",  0x10000, 0x10000, CRC(e11e988f) SHA1(0c59f0d8d1abe414c7e1ebd49d454179fed2cd00) )
	ROM_LOAD( "ei22.14f",  0x20000, 0x10000, CRC(b893d880) SHA1(99e228174677f2e3e96154f77bfa9bf0f1c0a6a5) )
	ROM_LOAD( "ei24.17f",  0x30000, 0x10000, CRC(6f226dda) SHA1(65ebb16a292c57d49c135fce7ed7537146226eb5) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "ei30.9h",   0x08000, 0x08000, CRC(982da0d1) SHA1(d819a587905624d793988f2ea726783da527d9f2) )
	ROM_CONTINUE(          0x00000, 0x08000 )   /* the two halves are swapped */
	ROM_LOAD( "ei28.9f",   0x18000, 0x08000, CRC(f01ebb3b) SHA1(1686690cb0c87d9e687b2abb4896cf285ab8378f) )
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "ei15.16c",  0x00000, 0x10000, CRC(a38a7d30) SHA1(5cb1fb97605829fc733c79a7e169fa52adc6863b) )
	ROM_LOAD( "ei16.17c",  0x10000, 0x08000, CRC(17e42633) SHA1(405f5296a741901677cca978a1b287d894eb1e54) )
	ROM_LOAD( "ei11.16a",  0x20000, 0x10000, CRC(3a77326c) SHA1(4de81752329cde6210a9c250a9f8ebe3dad9fe92) )
	ROM_LOAD( "ei12.17a",  0x30000, 0x08000, CRC(fea2a134) SHA1(525dd5f48993db1fe1e3c095442884178f75e8e0) )
	ROM_LOAD( "ei13.13c",  0x40000, 0x10000, CRC(e5ae2751) SHA1(4e4a3c68b11e9b0c8da70121b23296128063d4e9) )
	ROM_LOAD( "ei14.14c",  0x50000, 0x08000, CRC(e83c760a) SHA1(d08db381658b8b3288c5eaa9048a906126e0f712) )
	ROM_LOAD( "ei09.13a",  0x60000, 0x10000, CRC(6901e628) SHA1(1162c8cee20450780774cad54a9af40ebf0f0826) )
	ROM_LOAD( "ei10.14a",  0x70000, 0x08000, CRC(eeee8a1a) SHA1(2bf8378ff38f6a7c7cbd4cbd489de25cb1f0fe71) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "ei08.2c",   0x0000, 0x10000, CRC(3c87463e) SHA1(f17c98507b562e91e9b27599614b3249fe68ff7a) )

	ROM_REGION( 0x600, "proms", 0 ) /* PROMs */
	ROM_LOAD( "mb7116e.12c", 0x000, 0x200, CRC(86e775f8) SHA1(e8dee3d56fb5ca0fd7f9ce05a84674abb139d008) ) /* Also known to be labeled as A-1 */
	ROM_LOAD( "mb7122e.17e", 0x200, 0x400, CRC(a5cda23e) SHA1(d6c8534ae3c95b47a0701047fef67f15dd71f3fe) ) /* Also known to be labeled as A-2 */
ROM_END

ROM_START( drgninja ) /* DE-0297-0 main board, DE-0299-0 sub/rom board */
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 6*64k for 68000 code, middle 0x20000 unused */
	ROM_LOAD16_BYTE( "eg04-1.3c", 0x00000, 0x10000, CRC(41b8b3f8) SHA1(0ab143b9f7a5f857cfd2053c24fa5213ce7641e4) )
	ROM_LOAD16_BYTE( "eg01-1.3a", 0x00001, 0x10000, CRC(e08e6885) SHA1(641eaf4ef6c8bfbc39611f5f81765f7915ae9d9f) )
	ROM_LOAD16_BYTE( "eg06.6c",   0x40000, 0x10000, CRC(2b81faf7) SHA1(6d10c29f5ee06856843d83e77ba24c2b6e00a9cb) )
	ROM_LOAD16_BYTE( "eg03.6a",   0x40001, 0x10000, CRC(c52c2e9d) SHA1(399f2b7df9d558c8f33bf1a7c8048c62e0f54cec) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "eg07.8a",   0x8000, 0x8000, CRC(001d2f51) SHA1(f186671f0450ccf9201577a5caf0efc490c6645e) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* i8751 microcontroller */
	ROM_LOAD( "eg31.9a",   0x0000, 0x1000, CRC(657aab2d) SHA1(c3b3837d1208596509e09ddd8e3e58845a4e07b2) )
	/* various graphic and sound roms also differ when compared to baddudes */

	ROM_REGION( 0x10000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "eg25.15h",  0x00000, 0x08000, CRC(6791bc20) SHA1(7240b2688cda04ee9ea331472a84fbffc85b8e90) ) // different to baddudes
	ROM_LOAD( "eg26.16h",  0x08000, 0x08000, CRC(5d75fc8f) SHA1(92947dd78bfe8067fb5f645fa1ef212e48b69c70) ) // different to baddudes

	ROM_REGION( 0x40000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "eg18.14d",  0x00000, 0x10000, CRC(05cfc3e5) SHA1(a0163921c77dc9706463a402c3dd45ec4341cd21) )
	ROM_LOAD( "eg20.17d",  0x10000, 0x10000, CRC(e11e988f) SHA1(0c59f0d8d1abe414c7e1ebd49d454179fed2cd00) )
	ROM_LOAD( "eg22.14f",  0x20000, 0x10000, CRC(b893d880) SHA1(99e228174677f2e3e96154f77bfa9bf0f1c0a6a5) )
	ROM_LOAD( "eg24.17f",  0x30000, 0x10000, CRC(6f226dda) SHA1(65ebb16a292c57d49c135fce7ed7537146226eb5) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "eg30.9h",   0x08000, 0x08000, CRC(2438e67e) SHA1(5f143aeb83606a2c64d0b31bfee38156d231dcc9) )
	ROM_CONTINUE(          0x00000, 0x08000 )   /* the two halves are swapped */
	ROM_LOAD( "eg28.9f",   0x18000, 0x08000, CRC(5c692ab3) SHA1(4c58ff50833f869575f1a15c776fbf1429944fab) )
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "eg15.16c",  0x00000, 0x10000, CRC(5617d67f) SHA1(8f684de27ae79c4d35720706cdd2733af0e0a9cc) ) // different to baddudes
	ROM_LOAD( "eg16.17c",  0x10000, 0x08000, CRC(17e42633) SHA1(405f5296a741901677cca978a1b287d894eb1e54) )
	ROM_LOAD( "eg11.16a",  0x20000, 0x10000, CRC(ba83e8d8) SHA1(63092a5d0da0c9228a72a83b43a67a47b1388724) ) // different to baddudes
	ROM_LOAD( "eg12.17a",  0x30000, 0x08000, CRC(fea2a134) SHA1(525dd5f48993db1fe1e3c095442884178f75e8e0) )
	ROM_LOAD( "eg13.13c",  0x40000, 0x10000, CRC(fd91e08e) SHA1(8998f020791c8830e963096dc7b8fcb430d041d4) ) // different to baddudes
	ROM_LOAD( "eg14.14c",  0x50000, 0x08000, CRC(e83c760a) SHA1(d08db381658b8b3288c5eaa9048a906126e0f712) )
	ROM_LOAD( "eg09.13a",  0x60000, 0x10000, CRC(601b7b23) SHA1(c1c665614f1377bc47720382b25c965266a2593f) ) // different to baddudes
	ROM_LOAD( "eg10.14a",  0x70000, 0x08000, CRC(eeee8a1a) SHA1(2bf8378ff38f6a7c7cbd4cbd489de25cb1f0fe71) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "eg08.2c",   0x0000, 0x10000, CRC(92f2c916) SHA1(38b4ed81edcc2069b096591bdc5baab8b9edfa9a) ) // different to baddudes

	ROM_REGION( 0x600, "proms", 0 ) /* PROMs */
	ROM_LOAD( "mb7116e.12c", 0x000, 0x200, CRC(86e775f8) SHA1(e8dee3d56fb5ca0fd7f9ce05a84674abb139d008) ) /* Also known to be labeled as A-1 */
	ROM_LOAD( "mb7122e.17e", 0x200, 0x400, CRC(a5cda23e) SHA1(d6c8534ae3c95b47a0701047fef67f15dd71f3fe) ) /* Also known to be labeled as A-2 */
ROM_END

ROM_START( drgninjab )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 6*64k for 68000 code, middle 0x20000 unused */
	ROM_LOAD16_BYTE( "n-12.d2",  0x00000, 0x10000, CRC(5a70eb52) SHA1(26fd48ea71cd5196e3907eebcf1234f44a3d7dba) )
	ROM_LOAD16_BYTE( "n-11.a2",  0x00001, 0x10000, CRC(3887eb92) SHA1(a8650ce128927955497540d7c6fbd23516afdb24) )
	ROM_LOAD16_BYTE( "eg06.6c",  0x40000, 0x10000, CRC(2b81faf7) SHA1(6d10c29f5ee06856843d83e77ba24c2b6e00a9cb) )
	ROM_LOAD16_BYTE( "eg03.6a",  0x40001, 0x10000, CRC(c52c2e9d) SHA1(399f2b7df9d558c8f33bf1a7c8048c62e0f54cec) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "eg07.8a",   0x8000, 0x8000, CRC(001d2f51) SHA1(f186671f0450ccf9201577a5caf0efc490c6645e) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* i8751 microcontroller */
	ROM_LOAD( "i8751",     0x0000, 0x1000, NO_DUMP ) // Does this set actually have/need a MCU???

	/* various graphic and sound roms also differ when compared to baddudes */

	ROM_REGION( 0x10000, "gfx1", 0 ) /* chars */
	// ROM_LOAD( "eg25.15h",  0x00000, 0x08000, CRC(6791bc20) SHA1(7240b2688cda04ee9ea331472a84fbffc85b8e90) ) has been verified
	//  correct for the above drgninja set and dumped from an original DECO PCB with the original label intact.
	// ROM_LOAD( "eg25.bin",  0x00000, 0x08000, CRC(dd557b19) SHA1(ce1e76aeb7e147f373bb48dbc1becc1601953499) ) has the unused
	//  'Bad Dudes' logo partially erased, and a bad pixel on the left arrow character.
	ROM_LOAD( "eg25.bin",  0x00000, 0x08000, CRC(dd557b19) SHA1(ce1e76aeb7e147f373bb48dbc1becc1601953499) ) // 99.996948%
	ROM_LOAD( "eg26.16h",  0x08000, 0x08000, CRC(5d75fc8f) SHA1(92947dd78bfe8067fb5f645fa1ef212e48b69c70) ) // different to baddudes

	ROM_REGION( 0x40000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "eg18.14d",  0x00000, 0x10000, CRC(05cfc3e5) SHA1(a0163921c77dc9706463a402c3dd45ec4341cd21) )
	ROM_LOAD( "eg20.17d",  0x10000, 0x10000, CRC(e11e988f) SHA1(0c59f0d8d1abe414c7e1ebd49d454179fed2cd00) )
	ROM_LOAD( "eg22.14f",  0x20000, 0x10000, CRC(b893d880) SHA1(99e228174677f2e3e96154f77bfa9bf0f1c0a6a5) )
	ROM_LOAD( "eg24.17f",  0x30000, 0x10000, CRC(6f226dda) SHA1(65ebb16a292c57d49c135fce7ed7537146226eb5) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "eg30.9h",   0x08000, 0x08000, CRC(2438e67e) SHA1(5f143aeb83606a2c64d0b31bfee38156d231dcc9) )
	ROM_CONTINUE(          0x00000, 0x08000 )   /* the two halves are swapped */
	ROM_LOAD( "eg28.9f",   0x18000, 0x08000, CRC(5c692ab3) SHA1(4c58ff50833f869575f1a15c776fbf1429944fab) )
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "eg15.16c",  0x00000, 0x10000, CRC(5617d67f) SHA1(8f684de27ae79c4d35720706cdd2733af0e0a9cc) ) // different to baddudes
	ROM_LOAD( "eg16.17c",  0x10000, 0x08000, CRC(17e42633) SHA1(405f5296a741901677cca978a1b287d894eb1e54) )
	ROM_LOAD( "eg11.16a",  0x20000, 0x10000, CRC(ba83e8d8) SHA1(63092a5d0da0c9228a72a83b43a67a47b1388724) ) // different to baddudes
	ROM_LOAD( "eg12.17a",  0x30000, 0x08000, CRC(fea2a134) SHA1(525dd5f48993db1fe1e3c095442884178f75e8e0) )
	ROM_LOAD( "eg13.13c",  0x40000, 0x10000, CRC(fd91e08e) SHA1(8998f020791c8830e963096dc7b8fcb430d041d4) ) // different to baddudes
	ROM_LOAD( "eg14.14c",  0x50000, 0x08000, CRC(e83c760a) SHA1(d08db381658b8b3288c5eaa9048a906126e0f712) )
	ROM_LOAD( "eg09.13a",  0x60000, 0x10000, CRC(601b7b23) SHA1(c1c665614f1377bc47720382b25c965266a2593f) ) // different to baddudes
	ROM_LOAD( "eg10.14a",  0x70000, 0x08000, CRC(eeee8a1a) SHA1(2bf8378ff38f6a7c7cbd4cbd489de25cb1f0fe71) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "eg08.2c",   0x0000, 0x10000, CRC(92f2c916) SHA1(38b4ed81edcc2069b096591bdc5baab8b9edfa9a) ) // different to baddudes
ROM_END

/*

 CPUs
QTY     Type    clock   position    function
1x  SCN68000CAN64       main PCB 9h     16/32-bit Microprocessor - main
1x  UM6502      main PCB 7d     8-bit Microprocessor - sound
1x  MC3403      main PCB 14e    Quad Operational Amplifier - sound
1x  GL358       main PCB 13b    Dual Operational Amplifier - sound
1x  YM2203C         main PCB 12a    FM Operator Type-M (OPM) - sound
2x  YM3014B         main PCB 13d, 13e   D/A Converter (DAC) - sound
1x  YM3812      main PCB 9b     FM Operator Type-L II (OPL II) - sound
1x  MC68705R3P      ROMs PCB 1l     8-bit EPROM Microcomputer Unit - main (not dumped)
1x  M5205       ROMs PCB 12c    ADPCM Speech Synthesis IC - sound
1x  oscillator  24.000MHz   main PCB 2a
1x  oscillator  16.0000     main PCB 12n
1x  blu resonator   CSB-400P    ROMs PCB 12b
ROMs
QTY     Type    position    status
4x  27256   main PCB 1-4    dumped
4x  27512   main PCB 5-8    dumped
7x  27256   ROMs PCB 15,20-23,28,29     dumped
9x  27512   ROMs PCB 9-14,27-30     dumped
2x  N82S129AN   main PCB 2q,3p  not dumped yet
1x  N82S131N    main PCB 5q     not dumped yet
1x  N82S137N    main PCB 8u     not dumped yet
1x  N82S129AN   ROMs PCB 12c    not dumped yet
RAMs
QTY     Type    position
2x  HY6264  main PCB 12c,12d
14x     TMM2018     main PCB 1e,2e,5k,5l,5m,5n,5o,5p,7m,7n,8b,11s,11t,11u
2x  TMM2064     ROMs PCB 8n,8o
4x  TMM2018     ROMs PCB 8f,8g,8j,8k
PLDs
QTY     Type    position    status
Others

1x 28x2 edge connector
3x 50 pins flat cable connector from main board to roms board
1x trimmer (volume)
1x 8x2 switches DIP

*/

ROM_START( drgninjab2 )
	ROM_REGION( 0x60000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "a14.3e",  0x00000, 0x10000, CRC(c4b9f4e7) SHA1(4a8176cce8c7909aace8ece4f97b1a199617938e) ) // 99.978638%
	ROM_LOAD16_BYTE( "a11.3b",  0x00001, 0x10000, CRC(e4cc7c60) SHA1(63aeab4e20420f28a947438f2d7079c92a43d2df) ) // 99.978638%
	ROM_LOAD16_BYTE( "a12.2e",  0x40000, 0x10000, CRC(2b81faf7) SHA1(6d10c29f5ee06856843d83e77ba24c2b6e00a9cb) )
	ROM_LOAD16_BYTE( "a9.2b",   0x40001, 0x10000, CRC(c52c2e9d) SHA1(399f2b7df9d558c8f33bf1a7c8048c62e0f54cec) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "a15.7b",   0x8000, 0x8000, CRC(82007af2) SHA1(f0db1b1dab199df402a7590e56d4d5ab4baca803) ) // 99.612427%

	ROM_REGION( 0x1000, "mcu", 0 )  /* 68705 microcontroller */
	ROM_LOAD( "68705r3.0m",     0x0000, 0x1000, CRC(34bc5e7f) SHA1(7231dd7eb9b5152a287e1bcceb3c3a0b35f441af) )

	ROM_REGION( 0x10000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "a22.9m",  0x00000, 0x08000, CRC(6791bc20) SHA1(7240b2688cda04ee9ea331472a84fbffc85b8e90) )
	ROM_LOAD( "a23.9n",  0x08000, 0x08000, CRC(5d75fc8f) SHA1(92947dd78bfe8067fb5f645fa1ef212e48b69c70) )

	ROM_REGION( 0x40000, "gfx2", 0 ) /* tiles */ // identical but split into 4 roms
	ROM_LOAD( "a25.10f",  0x00000, 0x10000, CRC(05cfc3e5) SHA1(a0163921c77dc9706463a402c3dd45ec4341cd21) )
	ROM_LOAD( "a27.10h",  0x10000, 0x10000, CRC(e11e988f) SHA1(0c59f0d8d1abe414c7e1ebd49d454179fed2cd00) )
	ROM_LOAD( "a24.10e",  0x20000, 0x10000, CRC(b893d880) SHA1(99e228174677f2e3e96154f77bfa9bf0f1c0a6a5) )
	ROM_LOAD( "a26.10g",  0x30000, 0x10000, CRC(6f226dda) SHA1(65ebb16a292c57d49c135fce7ed7537146226eb5) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "a29.10k",   0x00000, 0x08000, CRC(4bf80966) SHA1(de4d83bac16f161a43678c2b2ae71f8fcac7212d) )
	ROM_LOAD( "a21.9k",    0x08000, 0x08000, CRC(b2e989fc) SHA1(492c1f3b18a4059c87254e0cba01ad9848e8b553) )
	ROM_LOAD( "a28.10j",   0x10000, 0x08000, CRC(2d38032d) SHA1(833ebff370825e5c8b8fc59fbe663b8998884353) )
	ROM_LOAD( "a20.9j",    0x18000, 0x08000, CRC(e71c0793) SHA1(e42a8192c772da1d6c93f9f9e89c553d712e18f7) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "a6.4g",   0x00000, 0x10000, CRC(5617d67f) SHA1(8f684de27ae79c4d35720706cdd2733af0e0a9cc) )
	ROM_LOAD( "a2.4c",   0x10000, 0x08000, CRC(17e42633) SHA1(405f5296a741901677cca978a1b287d894eb1e54) )
	ROM_LOAD( "a8.5g",   0x20000, 0x10000, CRC(ba83e8d8) SHA1(63092a5d0da0c9228a72a83b43a67a47b1388724) )
	ROM_LOAD( "a4.5c",   0x30000, 0x08000, CRC(fea2a134) SHA1(525dd5f48993db1fe1e3c095442884178f75e8e0) )
	ROM_LOAD( "a5.3g",   0x40000, 0x10000, CRC(fd91e08e) SHA1(8998f020791c8830e963096dc7b8fcb430d041d4) )
	ROM_LOAD( "a1.3c",   0x50000, 0x08000, CRC(e83c760a) SHA1(d08db381658b8b3288c5eaa9048a906126e0f712) )
	ROM_LOAD( "a7.4-5g", 0x60000, 0x10000, CRC(601b7b23) SHA1(c1c665614f1377bc47720382b25c965266a2593f) )
	ROM_LOAD( "a3.4-5c", 0x70000, 0x08000, CRC(eeee8a1a) SHA1(2bf8378ff38f6a7c7cbd4cbd489de25cb1f0fe71) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "a30.10b",   0x0000, 0x10000, CRC(f6806826) SHA1(e2c6a0682f34d30c63dab8715729151cc3657387) ) //  99.218750%

	ROM_REGION( 0x40000, "proms", 0 ) /* proms */
	ROM_LOAD( "n82s129an.12c", 0x0000, 0x100, CRC(78994fdb) SHA1(cd52bff11b81f19eeb2683ed94b236f1464a5ea9) )
	ROM_LOAD( "n82s129an.2q",  0x0000, 0x100, CRC(af46d1ee) SHA1(281bcc61d9d67b007c1399e228ec6baf6ab5d4ff) )
	ROM_LOAD( "n82s129an.3p",  0x0000, 0x100, CRC(9f6aa3e5) SHA1(518247d4581eee3a078269fcf0c86d182cf622cd) )
	ROM_LOAD( "n82s131n.5q",   0x0000, 0x200, CRC(86e775f8) SHA1(e8dee3d56fb5ca0fd7f9ce05a84674abb139d008) )  // == mb7116e.12c
	ROM_LOAD( "n82s137n.8u",   0x0000, 0x400, CRC(a5cda23e) SHA1(d6c8534ae3c95b47a0701047fef67f15dd71f3fe) )  // == mb7122e.17e
ROM_END


ROM_START( birdtry ) /* DE-0311-0 main board, DE-0299-2 sub/rom board */
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "ek-04-2.3c", 0x00000, 0x10000, CRC(5f0f4686) SHA1(5eea74f5626339ebd50e623029f21f1cd0f93135) )
	ROM_LOAD16_BYTE( "ek-01-2.3a", 0x00001, 0x10000, CRC(47f470db) SHA1(8fcb043d02e1c04c8517781715da4dd4ee3bb8fb) )
	ROM_LOAD16_BYTE( "ek-05-1.4c", 0x20000, 0x10000, CRC(b508cffd) SHA1(c1861a2420d99e19d889881f9164fe4ff667a1be) )
	ROM_LOAD16_BYTE( "ek-02-1.4a", 0x20001, 0x10000, CRC(0195d989) SHA1(cff48d57b2085263e12413ae19757cdcc7028282) )
	ROM_LOAD16_BYTE( "ek-06-1.6c", 0x40000, 0x10000, CRC(301d57d8) SHA1(64fd77aa2fbb235c86f0f84603e5272b4f4bba85) )
	ROM_LOAD16_BYTE( "ek-03-1.6a", 0x40001, 0x10000, CRC(73b0acc5) SHA1(76b79c9f02de2e53093ded66a1639b40cd2640e8) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 Sound */
	ROM_LOAD( "ek-07.8a", 0x8000, 0x8000, CRC(236549bc) SHA1(1f664a277b3451b7905638abdf98c7e428b2e935) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* i8751 microcontroller */
	ROM_LOAD( "ek-31-1.9a", 0x0000, 0x1000, CRC(3bf41abb) SHA1(d1833f5b59547c17f2683f4f2dced7ead3608d49) ) /* revised code / game data */

	ROM_REGION( 0x10000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "ek-25.15h", 0x00000, 0x08000, CRC(4df134ad) SHA1(f2cfa7e3fc4a2ac40897c2600c901ff75237e081) )
	ROM_LOAD( "ek-26.16h", 0x08000, 0x08000, CRC(a00d3e8e) SHA1(3ac8511d55a684a5b2bc05d8d520169447a66840) )

	ROM_REGION( 0x80000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "ek-18.14d", 0x00000, 0x10000, CRC(9886fb70) SHA1(d36c41bfe217affab7f9deec64ff3f12e3efa28c) )
	ROM_LOAD( "ek-17.12d", 0x10000, 0x10000, CRC(bed91bf7) SHA1(f0ffc557a4c216a5a2e180b4c2366e7b49630064) )
	ROM_LOAD( "ek-20.17d", 0x20000, 0x10000, CRC(45d53965) SHA1(d54d33cc82e099bcb511de8ee26cdcc64a0b8f1d) )
	ROM_LOAD( "ek-19.15d", 0x30000, 0x10000, CRC(c2949dd2) SHA1(d4317f8e0d9957feda54ee6d05aafb3f74f243d1) )
	ROM_LOAD( "ek-22.14f", 0x40000, 0x10000, CRC(7f2cc80a) SHA1(f2539515fcf0b6dc90134d399baf779c50b19c0d) )
	ROM_LOAD( "ek-21.12f", 0x50000, 0x10000, CRC(281bc793) SHA1(836fc2900b7197c886c23d9eeb1a80aed85c4d13) )
	ROM_LOAD( "ek-24.17f", 0x60000, 0x10000, CRC(2244cc75) SHA1(67c9868927319abe80a932203e8ac6595ae455b3) )
	ROM_LOAD( "ek-23.15f", 0x70000, 0x10000, CRC(d0ed0116) SHA1(a35e64ecac57585b83e830a1bf90a402c931f071) )

	ROM_REGION( 0x10000, "gfx3", ROMREGION_ERASEFF ) /* tiles */
	/* This game doesn't have the extra playfield chip, so no roms */

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "ek-15.16c", 0x00000, 0x10000, CRC(a6a041a3) SHA1(3b8d18d5821e6d354ed97a4f547f1b2bee8674f5) )
	ROM_LOAD( "ek-16.17c", 0x10000, 0x08000, CRC(784f62b0) SHA1(b68b234a5f469149d481645290a3251667bdab27) )
	ROM_LOAD( "ek-11.16a", 0x20000, 0x10000, CRC(9224a6b9) SHA1(547c22db1728a85035a682eb54ce654a98a4ba3d) )
	ROM_LOAD( "ek-12.17a", 0x30000, 0x08000, CRC(12deecfa) SHA1(22e33ccc6623957533028f720e9a746f36217ded) )
	ROM_LOAD( "ek-13.13c", 0x40000, 0x10000, CRC(1f023459) SHA1(e502edb4078168df4677a6d3aa43770eb8e49caa) )
	ROM_LOAD( "ek-14.14c", 0x50000, 0x08000, CRC(57d54943) SHA1(9639fad61919652c1564b24926845d228d016ca0) )
	ROM_LOAD( "ek-09.13a", 0x60000, 0x10000, CRC(6d2d488a) SHA1(40b21a4bc8a4641a6f80d7579e32fe9d69eb42f1) )
	ROM_LOAD( "ek-10.14a", 0x70000, 0x08000, CRC(580ba206) SHA1(8e57e4ef8c732b85e494bd6ec5da6566f27540e6) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "ek-08.2c", 0x0000, 0x10000, CRC(be3db6cb) SHA1(4e8b8e0bef3a3f36d7e641e27b5f48c8fe9a8b7f) )

	ROM_REGION( 0x600, "proms", 0 ) /* PROMs */
	ROM_LOAD( "mb7116e.12c", 0x000, 0x200, CRC(86e775f8) SHA1(e8dee3d56fb5ca0fd7f9ce05a84674abb139d008) ) /* Also known to be labeled as A-1 */
	ROM_LOAD( "mb7122e.17e", 0x200, 0x400, CRC(a5cda23e) SHA1(d6c8534ae3c95b47a0701047fef67f15dd71f3fe) ) /* Also known to be labeled as A-2 */
ROM_END

ROM_START( birdtrya ) /* DE-0311-0 main board, DE-0299-2 sub/rom board */
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "ek-04-2.3c", 0x00000, 0x10000, CRC(5f0f4686) SHA1(5eea74f5626339ebd50e623029f21f1cd0f93135) )
	ROM_LOAD16_BYTE( "ek-01-2.3a", 0x00001, 0x10000, CRC(47f470db) SHA1(8fcb043d02e1c04c8517781715da4dd4ee3bb8fb) )
	ROM_LOAD16_BYTE( "ek-05-1.4c", 0x20000, 0x10000, CRC(b508cffd) SHA1(c1861a2420d99e19d889881f9164fe4ff667a1be) )
	ROM_LOAD16_BYTE( "ek-02-1.4a", 0x20001, 0x10000, CRC(0195d989) SHA1(cff48d57b2085263e12413ae19757cdcc7028282) )
	ROM_LOAD16_BYTE( "ek-06-1.6c", 0x40000, 0x10000, CRC(301d57d8) SHA1(64fd77aa2fbb235c86f0f84603e5272b4f4bba85) )
	ROM_LOAD16_BYTE( "ek-03-1.6a", 0x40001, 0x10000, CRC(73b0acc5) SHA1(76b79c9f02de2e53093ded66a1639b40cd2640e8) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 Sound */
	ROM_LOAD( "ek-07.8a", 0x8000, 0x8000, CRC(236549bc) SHA1(1f664a277b3451b7905638abdf98c7e428b2e935) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* i8751 microcontroller */
	ROM_LOAD( "ek-31.9a", 0x0000, 0x1000, CRC(68831ae9) SHA1(0c8ef4903adbff68dccec04d8385c36904923a3c) )

	ROM_REGION( 0x10000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "ek-25.15h", 0x00000, 0x08000, CRC(4df134ad) SHA1(f2cfa7e3fc4a2ac40897c2600c901ff75237e081) )
	ROM_LOAD( "ek-26.16h", 0x08000, 0x08000, CRC(a00d3e8e) SHA1(3ac8511d55a684a5b2bc05d8d520169447a66840) )

	ROM_REGION( 0x80000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "ek-18.14d", 0x00000, 0x10000, CRC(9886fb70) SHA1(d36c41bfe217affab7f9deec64ff3f12e3efa28c) )
	ROM_LOAD( "ek-17.12d", 0x10000, 0x10000, CRC(bed91bf7) SHA1(f0ffc557a4c216a5a2e180b4c2366e7b49630064) )
	ROM_LOAD( "ek-20.17d", 0x20000, 0x10000, CRC(45d53965) SHA1(d54d33cc82e099bcb511de8ee26cdcc64a0b8f1d) )
	ROM_LOAD( "ek-19.15d", 0x30000, 0x10000, CRC(c2949dd2) SHA1(d4317f8e0d9957feda54ee6d05aafb3f74f243d1) )
	ROM_LOAD( "ek-22.14f", 0x40000, 0x10000, CRC(7f2cc80a) SHA1(f2539515fcf0b6dc90134d399baf779c50b19c0d) )
	ROM_LOAD( "ek-21.12f", 0x50000, 0x10000, CRC(281bc793) SHA1(836fc2900b7197c886c23d9eeb1a80aed85c4d13) )
	ROM_LOAD( "ek-24.17f", 0x60000, 0x10000, CRC(2244cc75) SHA1(67c9868927319abe80a932203e8ac6595ae455b3) )
	ROM_LOAD( "ek-23.15f", 0x70000, 0x10000, CRC(d0ed0116) SHA1(a35e64ecac57585b83e830a1bf90a402c931f071) )

	ROM_REGION( 0x10000, "gfx3", ROMREGION_ERASEFF ) /* tiles */
	/* This game doesn't have the extra playfield chip, so no roms */

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "ek-15.16c", 0x00000, 0x10000, CRC(a6a041a3) SHA1(3b8d18d5821e6d354ed97a4f547f1b2bee8674f5) )
	ROM_LOAD( "ek-16.17c", 0x10000, 0x08000, CRC(784f62b0) SHA1(b68b234a5f469149d481645290a3251667bdab27) )
	ROM_LOAD( "ek-11.16a", 0x20000, 0x10000, CRC(9224a6b9) SHA1(547c22db1728a85035a682eb54ce654a98a4ba3d) )
	ROM_LOAD( "ek-12.17a", 0x30000, 0x08000, CRC(12deecfa) SHA1(22e33ccc6623957533028f720e9a746f36217ded) )
	ROM_LOAD( "ek-13.13c", 0x40000, 0x10000, CRC(1f023459) SHA1(e502edb4078168df4677a6d3aa43770eb8e49caa) )
	ROM_LOAD( "ek-14.14c", 0x50000, 0x08000, CRC(57d54943) SHA1(9639fad61919652c1564b24926845d228d016ca0) )
	ROM_LOAD( "ek-09.13a", 0x60000, 0x10000, CRC(6d2d488a) SHA1(40b21a4bc8a4641a6f80d7579e32fe9d69eb42f1) )
	ROM_LOAD( "ek-10.14a", 0x70000, 0x08000, CRC(580ba206) SHA1(8e57e4ef8c732b85e494bd6ec5da6566f27540e6) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "ek-08.2c", 0x0000, 0x10000, CRC(be3db6cb) SHA1(4e8b8e0bef3a3f36d7e641e27b5f48c8fe9a8b7f) )

	ROM_REGION( 0x600, "proms", 0 ) /* PROMs */
	ROM_LOAD( "mb7116e.12c", 0x000, 0x200, CRC(86e775f8) SHA1(e8dee3d56fb5ca0fd7f9ce05a84674abb139d008) ) /* Also known to be labeled as A-1 */
	ROM_LOAD( "mb7122e.17e", 0x200, 0x400, CRC(a5cda23e) SHA1(d6c8534ae3c95b47a0701047fef67f15dd71f3fe) ) /* Also known to be labeled as A-2 */
ROM_END


ROM_START( robocop ) /* DE-0297-3 main board, DE-0316-3 sub/rom board */
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "ep05-4.11c", 0x00000, 0x10000, CRC(29c35379) SHA1(a352c2d0dff843c1e0b5cade506a8b33c2d781f1) )
	ROM_LOAD16_BYTE( "ep01-4.11b", 0x00001, 0x10000, CRC(77507c69) SHA1(843b678b4a297d6d99ea7d797dedde33e5003119) )
	ROM_LOAD16_BYTE( "ep04-3",     0x20000, 0x10000, CRC(39181778) SHA1(f91b63e541ef547d34d144c80bc0344b6acf8de0) )
	ROM_LOAD16_BYTE( "ep00-3",     0x20001, 0x10000, CRC(e128541f) SHA1(c123b6ba282b552890319d97348015361264fa3b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 Sound */
	ROM_LOAD( "ep03-3", 0x08000, 0x08000, CRC(5b164b24) SHA1(b217a2ac8b26aebd208631a13030487ed27d232e) )

	ROM_REGION( 0x200000, "sub", 0 )    /* HuC6280 CPU */
	ROM_LOAD( "en_24_mb7124e.a2", 0x01e00, 0x0200, CRC(b8e2ca98) SHA1(bd1e193c544dc17a665aa6c4d3b844775ed08b43) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "ep23", 0x00000, 0x10000, CRC(a77e4ab1) SHA1(d06cc847192b6c7f642e4ff7128e298d0aa034b2) )
	ROM_LOAD( "ep22", 0x10000, 0x10000, CRC(9fbd6903) SHA1(9ac6ac8a18c23e915e8ae3782867d10c0bd65778) )

	ROM_REGION( 0x40000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "ep20", 0x00000, 0x10000, CRC(1d8d38b8) SHA1(9add6349f8a578fb86b678cef921d6ec0cfccdad) )
	ROM_LOAD( "ep21", 0x10000, 0x10000, CRC(187929b2) SHA1(deca1f0a52584769caee1d2302617aa957c56a71) )
	ROM_LOAD( "ep18", 0x20000, 0x10000, CRC(b6580b5e) SHA1(ee216d8db89b8cb7a51a4e19bf6f17788547156b) )
	ROM_LOAD( "ep19", 0x30000, 0x10000, CRC(9bad01c7) SHA1(947c7f9d0facaea13a924274adde0e996be7b999) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "ep14", 0x00000, 0x08000, CRC(ca56ceda) SHA1(edbaa29fc166cddf071ff5e59cfcfb7eeb127d68) )
	ROM_LOAD( "ep15", 0x08000, 0x08000, CRC(a945269c) SHA1(de0b387e8699298f7682d6d7ca803a209888f7a1) )
	ROM_LOAD( "ep16", 0x10000, 0x08000, CRC(e7fa4d58) SHA1(32e3f649b4f112a4e6be00068473b82c627bc8d1) )
	ROM_LOAD( "ep17", 0x18000, 0x08000, CRC(84aae89d) SHA1(037520bd0f291f862c2211a6f35b2a8a54f10b2a) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "ep07", 0x00000, 0x10000, CRC(495d75cf) SHA1(0ffe677d53b7675073902e9bd40e4150f2cdfb1a) )
	ROM_LOAD( "ep06", 0x10000, 0x08000, CRC(a2ae32e2) SHA1(4e8182205563da9d50a831c65951645e278b03e6) )
	ROM_LOAD( "ep11", 0x20000, 0x10000, CRC(62fa425a) SHA1(be88c1a6436df8a456c405822e28c472e3e79a69) )
	ROM_LOAD( "ep10", 0x30000, 0x08000, CRC(cce3bd95) SHA1(00bbb197824d970b0e404167ca4ae53e1955ad94) )
	ROM_LOAD( "ep09", 0x40000, 0x10000, CRC(11bed656) SHA1(6a7d984a32982d9aef8ea7d8a720925036e7046e) )
	ROM_LOAD( "ep08", 0x50000, 0x08000, CRC(c45c7b4c) SHA1(70e3e475fe767eefa4cc1d6ca052271a099ff7a8) )
	ROM_LOAD( "ep13", 0x60000, 0x10000, CRC(8fca9f28) SHA1(cac85bf2b66e49e22c33c85bdb5712feef6aae7e) )
	ROM_LOAD( "ep12", 0x70000, 0x08000, CRC(3cd1d0c3) SHA1(ca3546cf51ebb10dfa4e78954f0212e8fcdb3d57) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "ep02", 0x00000, 0x10000, CRC(711ce46f) SHA1(939a8545e53776ff2180d2c7e63bc997689c088e) )

/*
MB7116E at 12C on CPU board - Removing this causes all sprites to disappear

MB7124E at 12A on ROM Board (although the board
is screened as MB7130) - Removing this causes the
display to disappear, although coining up makes
the correct text appear (and sound to play),
intro graphics appear on starting a game, then
nothing, it won't even play blind (although the
coin up sound still plays on coin insert)
*/
	ROM_REGION( 0x600, "proms", 0 ) /* PROMs */
	ROM_LOAD( "mb7116e_a-1.12c", 0x000, 0x200, CRC(86e775f8) SHA1(e8dee3d56fb5ca0fd7f9ce05a84674abb139d008) )
	ROM_LOAD( "mb7122e_a-2.17e", 0x200, 0x400, CRC(a5cda23e) SHA1(d6c8534ae3c95b47a0701047fef67f15dd71f3fe) )
ROM_END

ROM_START( robocopw ) /* DE-0297-3 main board, DE-0316-3 sub/rom board */
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "ep05-3", 0x00000, 0x10000, CRC(ba69bf84) SHA1(a9d4d94d1b936d43a610cfe02cc03bdeddb81ac6) )
	ROM_LOAD16_BYTE( "ep01-3", 0x00001, 0x10000, CRC(2a9f6e2c) SHA1(74aeb5be36619d90034d4a8139c3d043fe8d33c2) )
	ROM_LOAD16_BYTE( "ep04-3", 0x20000, 0x10000, CRC(39181778) SHA1(f91b63e541ef547d34d144c80bc0344b6acf8de0) )
	ROM_LOAD16_BYTE( "ep00-3", 0x20001, 0x10000, CRC(e128541f) SHA1(c123b6ba282b552890319d97348015361264fa3b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 Sound */
	ROM_LOAD( "ep03-3", 0x08000, 0x08000, CRC(5b164b24) SHA1(b217a2ac8b26aebd208631a13030487ed27d232e) )

	ROM_REGION( 0x200000, "sub", 0 )    /* HuC6280 CPU */
	ROM_LOAD( "en_24.a2", 0x01e00, 0x0200, CRC(b8e2ca98) SHA1(bd1e193c544dc17a665aa6c4d3b844775ed08b43) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "ep23", 0x00000, 0x10000, CRC(a77e4ab1) SHA1(d06cc847192b6c7f642e4ff7128e298d0aa034b2) )
	ROM_LOAD( "ep22", 0x10000, 0x10000, CRC(9fbd6903) SHA1(9ac6ac8a18c23e915e8ae3782867d10c0bd65778) )

	ROM_REGION( 0x40000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "ep20", 0x00000, 0x10000, CRC(1d8d38b8) SHA1(9add6349f8a578fb86b678cef921d6ec0cfccdad) )
	ROM_LOAD( "ep21", 0x10000, 0x10000, CRC(187929b2) SHA1(deca1f0a52584769caee1d2302617aa957c56a71) )
	ROM_LOAD( "ep18", 0x20000, 0x10000, CRC(b6580b5e) SHA1(ee216d8db89b8cb7a51a4e19bf6f17788547156b) )
	ROM_LOAD( "ep19", 0x30000, 0x10000, CRC(9bad01c7) SHA1(947c7f9d0facaea13a924274adde0e996be7b999) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "ep14", 0x00000, 0x08000, CRC(ca56ceda) SHA1(edbaa29fc166cddf071ff5e59cfcfb7eeb127d68) )
	ROM_LOAD( "ep15", 0x08000, 0x08000, CRC(a945269c) SHA1(de0b387e8699298f7682d6d7ca803a209888f7a1) )
	ROM_LOAD( "ep16", 0x10000, 0x08000, CRC(e7fa4d58) SHA1(32e3f649b4f112a4e6be00068473b82c627bc8d1) )
	ROM_LOAD( "ep17", 0x18000, 0x08000, CRC(84aae89d) SHA1(037520bd0f291f862c2211a6f35b2a8a54f10b2a) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "ep07", 0x00000, 0x10000, CRC(495d75cf) SHA1(0ffe677d53b7675073902e9bd40e4150f2cdfb1a) )
	ROM_LOAD( "ep06", 0x10000, 0x08000, CRC(a2ae32e2) SHA1(4e8182205563da9d50a831c65951645e278b03e6) )
	ROM_LOAD( "ep11", 0x20000, 0x10000, CRC(62fa425a) SHA1(be88c1a6436df8a456c405822e28c472e3e79a69) )
	ROM_LOAD( "ep10", 0x30000, 0x08000, CRC(cce3bd95) SHA1(00bbb197824d970b0e404167ca4ae53e1955ad94) )
	ROM_LOAD( "ep09", 0x40000, 0x10000, CRC(11bed656) SHA1(6a7d984a32982d9aef8ea7d8a720925036e7046e) )
	ROM_LOAD( "ep08", 0x50000, 0x08000, CRC(c45c7b4c) SHA1(70e3e475fe767eefa4cc1d6ca052271a099ff7a8) )
	ROM_LOAD( "ep13", 0x60000, 0x10000, CRC(8fca9f28) SHA1(cac85bf2b66e49e22c33c85bdb5712feef6aae7e) )
	ROM_LOAD( "ep12", 0x70000, 0x08000, CRC(3cd1d0c3) SHA1(ca3546cf51ebb10dfa4e78954f0212e8fcdb3d57) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "ep02", 0x00000, 0x10000, CRC(711ce46f) SHA1(939a8545e53776ff2180d2c7e63bc997689c088e) )

	ROM_REGION( 0x600, "proms", 0 ) /* PROMs */
	ROM_LOAD( "mb7116e_a-1.12c", 0x000, 0x200, CRC(86e775f8) SHA1(e8dee3d56fb5ca0fd7f9ce05a84674abb139d008) )
	ROM_LOAD( "mb7122e_a-2.17e", 0x200, 0x400, CRC(a5cda23e) SHA1(d6c8534ae3c95b47a0701047fef67f15dd71f3fe) )
ROM_END

ROM_START( robocopj ) /* DE-0297-3 main board, DE-0316-3 sub/rom board */
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "em05-1.c11", 0x00000, 0x10000, CRC(954ea8f4) SHA1(2efc6c6bf856bcd86aca439bf85ec9a5c2f89612) )
	ROM_LOAD16_BYTE( "em01-1.b12", 0x00001, 0x10000, CRC(1b87b622) SHA1(4b17e6377a77e8529b038529c12fdb2bd8a5af25) )
	ROM_LOAD16_BYTE( "ep04-3",     0x20000, 0x10000, CRC(39181778) SHA1(f91b63e541ef547d34d144c80bc0344b6acf8de0) )
	ROM_LOAD16_BYTE( "ep00-3",     0x20001, 0x10000, CRC(e128541f) SHA1(c123b6ba282b552890319d97348015361264fa3b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 Sound */
	ROM_LOAD( "ep03-3", 0x08000, 0x08000, CRC(5b164b24) SHA1(b217a2ac8b26aebd208631a13030487ed27d232e) )

	ROM_REGION( 0x200000, "sub", 0 )    /* HuC6280 CPU */
	ROM_LOAD( "en_24.a2", 0x01e00, 0x0200, CRC(b8e2ca98) SHA1(bd1e193c544dc17a665aa6c4d3b844775ed08b43) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "ep23", 0x00000, 0x10000, CRC(a77e4ab1) SHA1(d06cc847192b6c7f642e4ff7128e298d0aa034b2) )
	ROM_LOAD( "ep22", 0x10000, 0x10000, CRC(9fbd6903) SHA1(9ac6ac8a18c23e915e8ae3782867d10c0bd65778) )

	ROM_REGION( 0x40000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "ep20", 0x00000, 0x10000, CRC(1d8d38b8) SHA1(9add6349f8a578fb86b678cef921d6ec0cfccdad) )
	ROM_LOAD( "ep21", 0x10000, 0x10000, CRC(187929b2) SHA1(deca1f0a52584769caee1d2302617aa957c56a71) )
	ROM_LOAD( "ep18", 0x20000, 0x10000, CRC(b6580b5e) SHA1(ee216d8db89b8cb7a51a4e19bf6f17788547156b) )
	ROM_LOAD( "ep19", 0x30000, 0x10000, CRC(9bad01c7) SHA1(947c7f9d0facaea13a924274adde0e996be7b999) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "ep14", 0x00000, 0x08000, CRC(ca56ceda) SHA1(edbaa29fc166cddf071ff5e59cfcfb7eeb127d68) )
	ROM_LOAD( "ep15", 0x08000, 0x08000, CRC(a945269c) SHA1(de0b387e8699298f7682d6d7ca803a209888f7a1) )
	ROM_LOAD( "ep16", 0x10000, 0x08000, CRC(e7fa4d58) SHA1(32e3f649b4f112a4e6be00068473b82c627bc8d1) )
	ROM_LOAD( "ep17", 0x18000, 0x08000, CRC(84aae89d) SHA1(037520bd0f291f862c2211a6f35b2a8a54f10b2a) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "ep07", 0x00000, 0x10000, CRC(495d75cf) SHA1(0ffe677d53b7675073902e9bd40e4150f2cdfb1a) )
	ROM_LOAD( "ep06", 0x10000, 0x08000, CRC(a2ae32e2) SHA1(4e8182205563da9d50a831c65951645e278b03e6) )
	ROM_LOAD( "ep11", 0x20000, 0x10000, CRC(62fa425a) SHA1(be88c1a6436df8a456c405822e28c472e3e79a69) )
	ROM_LOAD( "ep10", 0x30000, 0x08000, CRC(cce3bd95) SHA1(00bbb197824d970b0e404167ca4ae53e1955ad94) )
	ROM_LOAD( "ep09", 0x40000, 0x10000, CRC(11bed656) SHA1(6a7d984a32982d9aef8ea7d8a720925036e7046e) )
	ROM_LOAD( "ep08", 0x50000, 0x08000, CRC(c45c7b4c) SHA1(70e3e475fe767eefa4cc1d6ca052271a099ff7a8) )
	ROM_LOAD( "ep13", 0x60000, 0x10000, CRC(8fca9f28) SHA1(cac85bf2b66e49e22c33c85bdb5712feef6aae7e) )
	ROM_LOAD( "ep12", 0x70000, 0x08000, CRC(3cd1d0c3) SHA1(ca3546cf51ebb10dfa4e78954f0212e8fcdb3d57) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "ep02", 0x00000, 0x10000, CRC(711ce46f) SHA1(939a8545e53776ff2180d2c7e63bc997689c088e) )

	ROM_REGION( 0x600, "proms", 0 ) /* PROMs */
	ROM_LOAD( "mb7116e_a-1.12c", 0x000, 0x200, CRC(86e775f8) SHA1(e8dee3d56fb5ca0fd7f9ce05a84674abb139d008) )
	ROM_LOAD( "mb7122e_a-2.17e", 0x200, 0x400, CRC(a5cda23e) SHA1(d6c8534ae3c95b47a0701047fef67f15dd71f3fe) )
ROM_END

ROM_START( robocopu ) /* DE-0297-3 main board, DE-0316-3 sub/rom board */
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "ep05-1", 0x00000, 0x10000, CRC(8de5cb3d) SHA1(66eb87aa11697d0abdd0c265aaa2048ca3c80c18) )
	ROM_LOAD16_BYTE( "ep01-1", 0x00001, 0x10000, CRC(b3c6bc02) SHA1(380dd241bebfdbdc93450b6cf562bccf8e3b8e27) )
	ROM_LOAD16_BYTE( "ep04",   0x20000, 0x10000, CRC(c38b9d18) SHA1(683bc4ce8dac62ab9ce79679ad44dc9542b814c8) )
	ROM_LOAD16_BYTE( "ep00",   0x20001, 0x10000, CRC(374c91aa) SHA1(d8bccc12278b754fe303eb75204b38126d401c3d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 Sound */
	ROM_LOAD( "ep03", 0x08000, 0x08000, CRC(1089eab8) SHA1(088c570b12b681f6751d7ae48560726464bcb79e) )

	ROM_REGION( 0x200000, "sub", 0 )    /* HuC6280 CPU */
	ROM_LOAD( "en_24.a2", 0x01e00, 0x0200, CRC(b8e2ca98) SHA1(bd1e193c544dc17a665aa6c4d3b844775ed08b43) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "ep23", 0x00000, 0x10000, CRC(a77e4ab1) SHA1(d06cc847192b6c7f642e4ff7128e298d0aa034b2) )
	ROM_LOAD( "ep22", 0x10000, 0x10000, CRC(9fbd6903) SHA1(9ac6ac8a18c23e915e8ae3782867d10c0bd65778) )

	ROM_REGION( 0x40000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "ep20", 0x00000, 0x10000, CRC(1d8d38b8) SHA1(9add6349f8a578fb86b678cef921d6ec0cfccdad) )
	ROM_LOAD( "ep21", 0x10000, 0x10000, CRC(187929b2) SHA1(deca1f0a52584769caee1d2302617aa957c56a71) )
	ROM_LOAD( "ep18", 0x20000, 0x10000, CRC(b6580b5e) SHA1(ee216d8db89b8cb7a51a4e19bf6f17788547156b) )
	ROM_LOAD( "ep19", 0x30000, 0x10000, CRC(9bad01c7) SHA1(947c7f9d0facaea13a924274adde0e996be7b999) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "ep14", 0x00000, 0x08000, CRC(ca56ceda) SHA1(edbaa29fc166cddf071ff5e59cfcfb7eeb127d68) )
	ROM_LOAD( "ep15", 0x08000, 0x08000, CRC(a945269c) SHA1(de0b387e8699298f7682d6d7ca803a209888f7a1) )
	ROM_LOAD( "ep16", 0x10000, 0x08000, CRC(e7fa4d58) SHA1(32e3f649b4f112a4e6be00068473b82c627bc8d1) )
	ROM_LOAD( "ep17", 0x18000, 0x08000, CRC(84aae89d) SHA1(037520bd0f291f862c2211a6f35b2a8a54f10b2a) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "ep07", 0x00000, 0x10000, CRC(495d75cf) SHA1(0ffe677d53b7675073902e9bd40e4150f2cdfb1a) )
	ROM_LOAD( "ep06", 0x10000, 0x08000, CRC(a2ae32e2) SHA1(4e8182205563da9d50a831c65951645e278b03e6) )
	ROM_LOAD( "ep11", 0x20000, 0x10000, CRC(62fa425a) SHA1(be88c1a6436df8a456c405822e28c472e3e79a69) )
	ROM_LOAD( "ep10", 0x30000, 0x08000, CRC(cce3bd95) SHA1(00bbb197824d970b0e404167ca4ae53e1955ad94) )
	ROM_LOAD( "ep09", 0x40000, 0x10000, CRC(11bed656) SHA1(6a7d984a32982d9aef8ea7d8a720925036e7046e) )
	ROM_LOAD( "ep08", 0x50000, 0x08000, CRC(c45c7b4c) SHA1(70e3e475fe767eefa4cc1d6ca052271a099ff7a8) )
	ROM_LOAD( "ep13", 0x60000, 0x10000, CRC(8fca9f28) SHA1(cac85bf2b66e49e22c33c85bdb5712feef6aae7e) )
	ROM_LOAD( "ep12", 0x70000, 0x08000, CRC(3cd1d0c3) SHA1(ca3546cf51ebb10dfa4e78954f0212e8fcdb3d57) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "ep02", 0x00000, 0x10000, CRC(711ce46f) SHA1(939a8545e53776ff2180d2c7e63bc997689c088e) )

	ROM_REGION( 0x600, "proms", 0 ) /* PROMs */
	ROM_LOAD( "mb7116e_a-1.12c", 0x000, 0x200, CRC(86e775f8) SHA1(e8dee3d56fb5ca0fd7f9ce05a84674abb139d008) )
	ROM_LOAD( "mb7122e_a-2.17e", 0x200, 0x400, CRC(a5cda23e) SHA1(d6c8534ae3c95b47a0701047fef67f15dd71f3fe) )
ROM_END

ROM_START( robocopu0 ) /* DE-0297-3 main board, DE-0316-3 sub/rom board */
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "ep05", 0x00000, 0x10000, CRC(c465bdd8) SHA1(a1d5435ea2664ac38db84577b97ba74304e09473) )
	ROM_LOAD16_BYTE( "ep01", 0x00001, 0x10000, CRC(1352d36e) SHA1(7bfdce66020b6c9465b768bac2ba7c9fe458242e) )
	ROM_LOAD16_BYTE( "ep04", 0x20000, 0x10000, CRC(c38b9d18) SHA1(683bc4ce8dac62ab9ce79679ad44dc9542b814c8) )
	ROM_LOAD16_BYTE( "ep00", 0x20001, 0x10000, CRC(374c91aa) SHA1(d8bccc12278b754fe303eb75204b38126d401c3d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 Sound */
	ROM_LOAD( "ep03", 0x08000, 0x08000, CRC(1089eab8) SHA1(088c570b12b681f6751d7ae48560726464bcb79e) )

	ROM_REGION( 0x200000, "sub", 0 )    /* HuC6280 CPU */
	ROM_LOAD( "en_24.a2", 0x01e00, 0x0200, CRC(b8e2ca98) SHA1(bd1e193c544dc17a665aa6c4d3b844775ed08b43) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "ep23", 0x00000, 0x10000, CRC(a77e4ab1) SHA1(d06cc847192b6c7f642e4ff7128e298d0aa034b2) )
	ROM_LOAD( "ep22", 0x10000, 0x10000, CRC(9fbd6903) SHA1(9ac6ac8a18c23e915e8ae3782867d10c0bd65778) )

	ROM_REGION( 0x40000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "ep20", 0x00000, 0x10000, CRC(1d8d38b8) SHA1(9add6349f8a578fb86b678cef921d6ec0cfccdad) )
	ROM_LOAD( "ep21", 0x10000, 0x10000, CRC(187929b2) SHA1(deca1f0a52584769caee1d2302617aa957c56a71) )
	ROM_LOAD( "ep18", 0x20000, 0x10000, CRC(b6580b5e) SHA1(ee216d8db89b8cb7a51a4e19bf6f17788547156b) )
	ROM_LOAD( "ep19", 0x30000, 0x10000, CRC(9bad01c7) SHA1(947c7f9d0facaea13a924274adde0e996be7b999) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "ep14", 0x00000, 0x08000, CRC(ca56ceda) SHA1(edbaa29fc166cddf071ff5e59cfcfb7eeb127d68) )
	ROM_LOAD( "ep15", 0x08000, 0x08000, CRC(a945269c) SHA1(de0b387e8699298f7682d6d7ca803a209888f7a1) )
	ROM_LOAD( "ep16", 0x10000, 0x08000, CRC(e7fa4d58) SHA1(32e3f649b4f112a4e6be00068473b82c627bc8d1) )
	ROM_LOAD( "ep17", 0x18000, 0x08000, CRC(84aae89d) SHA1(037520bd0f291f862c2211a6f35b2a8a54f10b2a) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "ep07", 0x00000, 0x10000, CRC(495d75cf) SHA1(0ffe677d53b7675073902e9bd40e4150f2cdfb1a) )
	ROM_LOAD( "ep06", 0x10000, 0x08000, CRC(a2ae32e2) SHA1(4e8182205563da9d50a831c65951645e278b03e6) )
	ROM_LOAD( "ep11", 0x20000, 0x10000, CRC(62fa425a) SHA1(be88c1a6436df8a456c405822e28c472e3e79a69) )
	ROM_LOAD( "ep10", 0x30000, 0x08000, CRC(cce3bd95) SHA1(00bbb197824d970b0e404167ca4ae53e1955ad94) )
	ROM_LOAD( "ep09", 0x40000, 0x10000, CRC(11bed656) SHA1(6a7d984a32982d9aef8ea7d8a720925036e7046e) )
	ROM_LOAD( "ep08", 0x50000, 0x08000, CRC(c45c7b4c) SHA1(70e3e475fe767eefa4cc1d6ca052271a099ff7a8) )
	ROM_LOAD( "ep13", 0x60000, 0x10000, CRC(8fca9f28) SHA1(cac85bf2b66e49e22c33c85bdb5712feef6aae7e) )
	ROM_LOAD( "ep12", 0x70000, 0x08000, CRC(3cd1d0c3) SHA1(ca3546cf51ebb10dfa4e78954f0212e8fcdb3d57) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "ep02", 0x00000, 0x10000, CRC(711ce46f) SHA1(939a8545e53776ff2180d2c7e63bc997689c088e) )

	ROM_REGION( 0x600, "proms", 0 ) /* PROMs */
	ROM_LOAD( "mb7116e_a-1.12c", 0x000, 0x200, CRC(86e775f8) SHA1(e8dee3d56fb5ca0fd7f9ce05a84674abb139d008) )
	ROM_LOAD( "mb7122e_a-2.17e", 0x200, 0x400, CRC(a5cda23e) SHA1(d6c8534ae3c95b47a0701047fef67f15dd71f3fe) )
ROM_END

ROM_START( robocopb )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "robop_05.rom", 0x00000, 0x10000, CRC(bcef3e9b) SHA1(0ca099ea7428f877036e6e2a6daddfd9145ed9bb) )
	ROM_LOAD16_BYTE( "robop_01.rom", 0x00001, 0x10000, CRC(c9803685) SHA1(13b3b0ebee24b4453685616e9a204b4ca6fb0053) )
	ROM_LOAD16_BYTE( "robop_04.rom", 0x20000, 0x10000, CRC(9d7b79e0) SHA1(e0d901b9b3cd62f7c947da04f7447ebfa88bf44a) )
	ROM_LOAD16_BYTE( "robop_00.rom", 0x20001, 0x10000, CRC(80ba64ab) SHA1(0688f1b483a265c7324f546d38a4a5ac5b1b9214) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 Sound */
	ROM_LOAD( "ep03-3", 0x08000, 0x08000, CRC(5b164b24) SHA1(b217a2ac8b26aebd208631a13030487ed27d232e) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "ep23", 0x00000, 0x10000, CRC(a77e4ab1) SHA1(d06cc847192b6c7f642e4ff7128e298d0aa034b2) )
	ROM_LOAD( "ep22", 0x10000, 0x10000, CRC(9fbd6903) SHA1(9ac6ac8a18c23e915e8ae3782867d10c0bd65778) )

	ROM_REGION( 0x40000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "ep20", 0x00000, 0x10000, CRC(1d8d38b8) SHA1(9add6349f8a578fb86b678cef921d6ec0cfccdad) )
	ROM_LOAD( "ep21", 0x10000, 0x10000, CRC(187929b2) SHA1(deca1f0a52584769caee1d2302617aa957c56a71) )
	ROM_LOAD( "ep18", 0x20000, 0x10000, CRC(b6580b5e) SHA1(ee216d8db89b8cb7a51a4e19bf6f17788547156b) )
	ROM_LOAD( "ep19", 0x30000, 0x10000, CRC(9bad01c7) SHA1(947c7f9d0facaea13a924274adde0e996be7b999) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "ep14", 0x00000, 0x08000, CRC(ca56ceda) SHA1(edbaa29fc166cddf071ff5e59cfcfb7eeb127d68) )
	ROM_LOAD( "ep15", 0x08000, 0x08000, CRC(a945269c) SHA1(de0b387e8699298f7682d6d7ca803a209888f7a1) )
	ROM_LOAD( "ep16", 0x10000, 0x08000, CRC(e7fa4d58) SHA1(32e3f649b4f112a4e6be00068473b82c627bc8d1) )
	ROM_LOAD( "ep17", 0x18000, 0x08000, CRC(84aae89d) SHA1(037520bd0f291f862c2211a6f35b2a8a54f10b2a) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "ep07", 0x00000, 0x10000, CRC(495d75cf) SHA1(0ffe677d53b7675073902e9bd40e4150f2cdfb1a) )
	ROM_LOAD( "ep06", 0x10000, 0x08000, CRC(a2ae32e2) SHA1(4e8182205563da9d50a831c65951645e278b03e6) )
	ROM_LOAD( "ep11", 0x20000, 0x10000, CRC(62fa425a) SHA1(be88c1a6436df8a456c405822e28c472e3e79a69) )
	ROM_LOAD( "ep10", 0x30000, 0x08000, CRC(cce3bd95) SHA1(00bbb197824d970b0e404167ca4ae53e1955ad94) )
	ROM_LOAD( "ep09", 0x40000, 0x10000, CRC(11bed656) SHA1(6a7d984a32982d9aef8ea7d8a720925036e7046e) )
	ROM_LOAD( "ep08", 0x50000, 0x08000, CRC(c45c7b4c) SHA1(70e3e475fe767eefa4cc1d6ca052271a099ff7a8) )
	ROM_LOAD( "ep13", 0x60000, 0x10000, CRC(8fca9f28) SHA1(cac85bf2b66e49e22c33c85bdb5712feef6aae7e) )
	ROM_LOAD( "ep12", 0x70000, 0x08000, CRC(3cd1d0c3) SHA1(ca3546cf51ebb10dfa4e78954f0212e8fcdb3d57) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "ep02", 0x00000, 0x10000, CRC(711ce46f) SHA1(939a8545e53776ff2180d2c7e63bc997689c088e) )
ROM_END

/*

AUTOMAT (bootleg ROBOCOP)
^^^^^^^^^^^^^^^^^^^^^^^^^

Dumped by Andrew Welburn on the evening of a day
of big snow! 02/02/09

http://www.andys-arcade.com

*************************************************
**Do not separate this text file from the roms.**
*************************************************

Take a look at the photos in the archive, the roms
should be fairly explanatory, and you should be
able to pick out the chips it uses.

The most striking thing about this bootleg apart
from the obviously changed title screen is that
the music/melody is not right, they've copied the
digital sound effects, but appear to have ripped
the music and circuit design from an earlier
capcom game, I can't work out which one, but
what an odd thing to do!

you can see a youtube video of it running here:
http://uk.youtube.com/watch?v=Y-KvbKtqzaQ

Rom 21 is full of 0's... I cleaned and re-dumped
it numerous times, but I just got 0's every time.
It contains some of the graphics for enemies on
the opening stage at the very least.

enjoy..

*/

ROM_START( automat )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "5.ic50", 0x00000, 0x10000, CRC(fb6faa74) SHA1(0af03c06193b5ba1422571b9504a7f655c608d94) )
	ROM_LOAD16_BYTE( "2.ic54", 0x00001, 0x10000, CRC(7ecf8309) SHA1(59dd50bcb528ece42a67154bcc4f432770420986) )
	ROM_LOAD16_BYTE( "4.ic51", 0x20000, 0x10000, CRC(9d7b79e0) SHA1(e0d901b9b3cd62f7c947da04f7447ebfa88bf44a) )
	ROM_LOAD16_BYTE( "3.ic53", 0x20001, 0x10000, CRC(e655f9c3) SHA1(d5e99d542303d009277ccfc245f877e4e28603c9) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 Sound */
	ROM_LOAD( "1.ic26", 0x00000, 0x10000, CRC(72ea6024) SHA1(debd30219879ec01f43cc116a6cfa17209940ecc) )
	ROM_RELOAD(        0x10000, 0x10000 ) // IC25 socket unpopulated

	ROM_REGION( 0x40000, "gfxload1", 0 ) /* chars */
	ROM_LOAD( "8.bin",  0x00000, 0x10000, CRC(dcfffc7a) SHA1(e250626473917d397381210ef536efbc93c46474) ) // y?
	ROM_LOAD( "7.bin",  0x10000, 0x10000, CRC(40218082) SHA1(6a5c83d20fe110d642d5730c52e2796655fb66b4) ) // y
	ROM_LOAD( "10.bin", 0x20000, 0x10000, CRC(957da6dd) SHA1(53490d80ef108e93f13440de13b58761b89a419a) ) // y
	ROM_LOAD( "12.bin", 0x30000, 0x10000, CRC(00cd0990) SHA1(3fc498fcee2110001e376f5ee38d7dd361bd3ee3) ) // y

	/* copy out the chars */
	ROM_REGION( 0x20000, "gfx1", ROMREGION_INVERT ) /* chars */
	ROM_COPY( "gfxload1", 0x00000, 0x00000, 0x8000 )
	ROM_COPY( "gfxload1", 0x10000, 0x08000, 0x8000 )
	ROM_COPY( "gfxload1", 0x20000, 0x10000, 0x8000 )
	ROM_COPY( "gfxload1", 0x30000, 0x18000, 0x8000 )

	ROM_REGION( 0x20000, "gfx3", ROMREGION_INVERT ) /* tiles */
	ROM_COPY( "gfxload1", 0x08000, 0x00000, 0x8000 )
	ROM_COPY( "gfxload1", 0x18000, 0x08000, 0x8000 )
	ROM_COPY( "gfxload1", 0x28000, 0x10000, 0x8000 )
	ROM_COPY( "gfxload1", 0x38000, 0x18000, 0x8000 )

	// we have to rearrange this with ROM_CONTINUE due to the way gfxdecode works */
	ROM_REGION( 0x40000, "gfx2", ROMREGION_INVERT ) /* tiles */
	ROM_LOAD( "9.bin",  0x00000, 0x2000, CRC(ccf91ce0) SHA1(c976eddcea48da6e7fbd28a4d5c48706d61cabfb) )
	ROM_CONTINUE(       0x04000, 0x2000 )
	ROM_CONTINUE(       0x08000, 0x2000 )
	ROM_CONTINUE(       0x0c000, 0x2000 )
	ROM_CONTINUE(       0x02000, 0x2000 )
	ROM_CONTINUE(       0x06000, 0x2000 )
	ROM_CONTINUE(       0x0a000, 0x2000 )
	ROM_CONTINUE(       0x0e000, 0x2000 )
	ROM_LOAD( "6.bin",  0x10000, 0x2000, CRC(5a557765) SHA1(f081323dad532fae6ec5d2875ffb1c394ac0bcf9) )
	ROM_CONTINUE(       0x14000, 0x2000 )
	ROM_CONTINUE(       0x18000, 0x2000 )
	ROM_CONTINUE(       0x1c000, 0x2000 )
	ROM_CONTINUE(       0x12000, 0x2000 )
	ROM_CONTINUE(       0x16000, 0x2000 )
	ROM_CONTINUE(       0x1a000, 0x2000 )
	ROM_CONTINUE(       0x1e000, 0x2000 )
	ROM_LOAD( "11.bin", 0x20000, 0x2000, CRC(8b196ab7) SHA1(030dc19f464db072c8dbbf043ae9334aa58510d0) )
	ROM_CONTINUE(       0x24000, 0x2000 )
	ROM_CONTINUE(       0x28000, 0x2000 )
	ROM_CONTINUE(       0x2c000, 0x2000 )
	ROM_CONTINUE(       0x22000, 0x2000 )
	ROM_CONTINUE(       0x26000, 0x2000 )
	ROM_CONTINUE(       0x2a000, 0x2000 )
	ROM_CONTINUE(       0x2e000, 0x2000 )
	ROM_LOAD( "13.bin", 0x30000, 0x2000, CRC(7f12ed0e) SHA1(9340611b85f9866d086970ed5e9c0c704616c330) )
	ROM_CONTINUE(       0x34000, 0x2000 )
	ROM_CONTINUE(       0x38000, 0x2000 )
	ROM_CONTINUE(       0x3c000, 0x2000 )
	ROM_CONTINUE(       0x32000, 0x2000 )
	ROM_CONTINUE(       0x36000, 0x2000 )
	ROM_CONTINUE(       0x3a000, 0x2000 )
	ROM_CONTINUE(       0x3e000, 0x2000 )

	// the sprite data is the same as robocop, but with the bits in each byte reversed
	// 21.bin was repaired with this knowledge as the chip was faulty
	ROM_REGION( 0x80000, "gfx4", 0) /* sprites */
	ROM_LOAD( "16.bin", 0x00000, 0x10000, CRC(e42e8675) SHA1(5b964477de8278ea330ffc2366e5fc7e10122ef8) )
	ROM_LOAD( "17.bin", 0x10000, 0x08000, CRC(9a414c56) SHA1(017eb5a238e24cd6de50afd029c239993fc61a21) )
	ROM_LOAD( "20.bin", 0x20000, 0x10000, CRC(7c62a2a1) SHA1(43a40355cdcbb17506f9634e8f12673287e79bd7) )
	ROM_LOAD( "21.bin", 0x30000, 0x08000, CRC(ae59dccd) SHA1(e4ec6e9441bd7882a14768a7b7d8e79a7781f436) )
	ROM_LOAD( "14.bin", 0x40000, 0x10000, CRC(674ad6dc) SHA1(63982b8106f771e9e79cd8dbad42cfd4aad6f16f) )
	ROM_LOAD( "15.bin", 0x50000, 0x08000, CRC(5e7dd1aa) SHA1(822232a7389708dd5fee4a874a8832e22e7a0a26) )
	ROM_LOAD( "18.bin", 0x60000, 0x10000, CRC(751e34aa) SHA1(066730a26606a74b9295fc483cb0063c32dc9a14) )
	ROM_LOAD( "19.bin", 0x70000, 0x08000, CRC(118e7fc7) SHA1(fa6d8eef9da873579e19a9bf982643e061b8ca26) )
ROM_END


ROM_START( bandit )  /* DE-0289-2 main board, DE-0293-1 sub/rom board */
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "4.4",     0x00000, 0x10000, CRC(01a29133) SHA1(6a8e9b959828f82333ca17dbc751a9fbafae6935) )
	ROM_LOAD16_BYTE( "1.1",     0x00001, 0x10000, CRC(dc61b11f) SHA1(3178a1247d2ef4d30f9c6c55b53db658214d2861) )
	ROM_LOAD16_BYTE( "5.5",     0x20000, 0x10000, CRC(7dbfa088) SHA1(7046d84f0a00f86cfa0c4d77e43118adda111001) )
	ROM_LOAD16_BYTE( "2.2",     0x20001, 0x10000, CRC(3e81e138) SHA1(b2eb57900fe110d64852a5c4c9d12c060ecb54e5) )
	ROM_LOAD16_BYTE( "6.6",     0x40000, 0x10000, CRC(b12e33cc) SHA1(b551cc09d25bb8ae198415f75ac2a150f2789849) )
	ROM_LOAD16_BYTE( "3.3",     0x40001, 0x10000, CRC(30bf52cf) SHA1(f45d3028d0e4e8c8f2c5968b52399a4e7eb9255f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 Sound */
	ROM_LOAD( "7.7",     0x8000, 0x8000, CRC(69259ca4) SHA1(ce014836c71e269569279a09abef20cf03d46e31) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* i8751 microcontroller */
	ROM_LOAD( "hb31.9a",      0x0000, 0x1000, CRC(239d726f) SHA1(969f38ae981ffde6053ece93cc51614d492edbbb) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "25.25",     0x00000, 0x10000, CRC(4047ff81) SHA1(56a82c7694e6dbbdb9b42ed134120a76f848f7a5) )
	ROM_LOAD( "26.26",     0x10000, 0x10000, CRC(3a0a2f1e) SHA1(f06b44e4a8c29ee2c0a6e8f786fbee144138ba72) )

	ROM_REGION( 0x80000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "18.18",     0x00000, 0x10000, CRC(ac38e585) SHA1(c1a6fb083c096d119351883dea121ee6358d9298) )
	ROM_LOAD( "20.20",     0x20000, 0x10000, CRC(2194f737) SHA1(fbe2f7d0d6b80bf62fb9c38f9f2a001a728f3b7c) )
	ROM_LOAD( "22.22",     0x40000, 0x10000, CRC(fcc6cb4d) SHA1(548bd8688b255cdd1eef82e4fbec3d88e1d0ab53) )
	ROM_LOAD( "24.24",     0x60000, 0x10000, CRC(aa3c33b6) SHA1(f7770daedb5c1d5dd5099f1378c5c292c68a6a12) )

	ROM_REGION( 0x40000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "29.29",     0x00000, 0x10000, CRC(32218c8a) SHA1(33e922ffd7000a03a8fa8bbe61483cd8e916ebd6) )
	ROM_LOAD( "30.30",     0x10000, 0x10000, CRC(6a5fe9a9) SHA1(a750373e9fb1a0ad81d63a19bbac7a6079d3372f) )
	ROM_LOAD( "27.27",     0x20000, 0x10000, CRC(62970304) SHA1(57606bedfd83429629593bbea50fb25db1f2d874) )
	ROM_LOAD( "28.28",     0x30000, 0x10000, CRC(e018459f) SHA1(136ce96523988173425fa791742dab62ecaef5c9) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "15.15",     0x00000, 0x10000, CRC(84c03235) SHA1(3b5a8e24dd0aba1d1530d37685aaa46a35b0249a) )
	ROM_LOAD( "16.16",     0x10000, 0x10000, CRC(eaa35477) SHA1(08763d7937a6fa76f6029974d5b909ba05c69d81) )
	ROM_LOAD( "11.11",     0x20000, 0x10000, CRC(c9e6b57f) SHA1(9c12e0e7e25d48c7679ea65c3dfeaca9fdfdcbb3) )
	ROM_LOAD( "12.12",     0x30000, 0x10000, CRC(317f7e4a) SHA1(7d20722e75d69ef4f408e1995a29338c452b63c9) )

	ROM_LOAD( "13.13",     0x40000, 0x10000, CRC(0c063bec) SHA1(e9808d20dc9e450c6a880ca1d8383246aff1ab36) )
	ROM_LOAD( "14.14",     0x50000, 0x10000, CRC(2ebf06d3) SHA1(6381f978c0e719bf80d2c16a8355a0892473720a) )
	ROM_LOAD( "9.9",       0x60000, 0x10000, CRC(046f9d58) SHA1(a3078c13d27f365f900be3b55d1a917a8cd712f8) )
	ROM_LOAD( "10.10",     0x70000, 0x10000, CRC(3d2d704e) SHA1(3dcf3dd8cc91c5629e014bc6e2ccce234cf549a5) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "8.8",     0x0000, 0x10000, CRC(b0e79b9f) SHA1(6613c41234b8305d8959d06d6b4e9127bfc5eebe) )

	ROM_REGION( 0x600, "proms", 0 ) /* PROMs */
	ROM_LOAD( "mb7116e_a-1.12c", 0x000, 0x200, CRC(86e775f8) SHA1(e8dee3d56fb5ca0fd7f9ce05a84674abb139d008) )
	ROM_LOAD( "mb7122e_a-2.17e", 0x200, 0x400, CRC(a5cda23e) SHA1(d6c8534ae3c95b47a0701047fef67f15dd71f3fe) )
ROM_END


ROM_START( hippodrm )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 4*64k for 68000 code */
	ROM_LOAD16_BYTE( "ew02",         0x00000, 0x10000, CRC(df0d7dc6) SHA1(a60197ad6f19f730e05cf6a3be9181f28d425344) )
	ROM_LOAD16_BYTE( "ew01",         0x00001, 0x10000, CRC(d5670aa7) SHA1(ea8bdff63176c2657746c2c438298685e1f44eae) )
	ROM_LOAD16_BYTE( "ew05",         0x20000, 0x10000, CRC(c76d65ec) SHA1(620990acaf2fd7f3fbfe7135a17ac0195feb8330) )
	ROM_LOAD16_BYTE( "ew00",         0x20001, 0x10000, CRC(e9b427a6) SHA1(b334992846771739d31756724138b82f897dfad5) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 sound */
	ROM_LOAD( "ew04",         0x8000, 0x8000, CRC(9871b98d) SHA1(2b6c46bc2b10a28946d6ad8251e1a156a0b99947) )

	ROM_REGION( 0x10000, "sub", 0 ) /* HuC6280 CPU */
	ROM_LOAD( "ew08",         0x00000, 0x10000, CRC(53010534) SHA1(8b996e48414bacd009e05ff49848884ecf15d967) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "ew14",         0x00000, 0x10000, CRC(71ca593d) SHA1(05ab9403c4010a21dcaa169f4c59d19c4169d9cd) )
	ROM_LOAD( "ew13",         0x10000, 0x10000, CRC(86be5fa7) SHA1(71c31ca2e92fb39a5486e80150919e13d5617855) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "ew19",         0x00000, 0x08000, CRC(6b80d7a3) SHA1(323162e7e0ce16f6244d8d98fdb2396ffef87e82) )
	ROM_LOAD( "ew18",         0x08000, 0x08000, CRC(78d3d764) SHA1(e8f77a23bd4f4d268bec7c0153fb957acd07cdee) )
	ROM_LOAD( "ew20",         0x10000, 0x08000, CRC(ce9f5de3) SHA1(b8af33f52ca3579a45b41395751697a58931f9d6) )
	ROM_LOAD( "ew21",         0x18000, 0x08000, CRC(487a7ba2) SHA1(7d52cc1517def8426355e8281440ec5e617d1121) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "ew24",         0x00000, 0x08000, CRC(4e1bc2a4) SHA1(d7d4c42fd932722436f1847929088e46d03184bd) )
	ROM_LOAD( "ew25",         0x08000, 0x08000, CRC(9eb47dfb) SHA1(bb1e8a3a47f447f3a983ea51943d3081d56ad9a4) )
	ROM_LOAD( "ew23",         0x10000, 0x08000, CRC(9ecf479e) SHA1(a8d4c1490f12e1b15d53a2a97147920dcb638378) )
	ROM_LOAD( "ew22",         0x18000, 0x08000, CRC(e55669aa) SHA1(2a9b0e85bb81ff87a108e08b28e19b7b469463e4) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "ew15",         0x00000, 0x10000, CRC(95423914) SHA1(e9e7a6bdf5aa717dc04a751709632f31762886fb) )
	ROM_LOAD( "ew16",         0x10000, 0x10000, CRC(96233177) SHA1(929a1b7fb65ab33277719b84517ff57da563f875) )
	ROM_LOAD( "ew10",         0x20000, 0x10000, CRC(4c25dfe8) SHA1(e4334de96698cd0112a8926dea131e748b6a84fc) )
	ROM_LOAD( "ew11",         0x30000, 0x10000, CRC(f2e007fc) SHA1(da30ad3725b9bc4a07dbb1afa05f145c3574c84c) )
	ROM_LOAD( "ew06",         0x40000, 0x10000, CRC(e4bb8199) SHA1(49b5b45c7cd9c44f53d83ee2a156d9e9f8a53960) )
	ROM_LOAD( "ew07",         0x50000, 0x10000, CRC(470b6989) SHA1(16b292d8a3a54048bf29f0b4f41bb6ca049b347c) )
	ROM_LOAD( "ew17",         0x60000, 0x10000, CRC(8c97c757) SHA1(36fd807da9e144dfb29c8252e9450cc37ca2604f) )
	ROM_LOAD( "ew12",         0x70000, 0x10000, CRC(a2d244bc) SHA1(ff2391efc480f36a302650691f8a7a620b86d99a) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "ew03",         0x0000, 0x10000, CRC(b606924d) SHA1(b759fcec10b333465cf5cd1b30987bf2d62186b2) )

	ROM_REGION( 0x600, "proms", 0 ) /* PROMs */
	ROM_LOAD( "mb7116e_a-1.12c", 0x000, 0x200, CRC(86e775f8) SHA1(e8dee3d56fb5ca0fd7f9ce05a84674abb139d008) )
	ROM_LOAD( "mb7122e_a-2.17e", 0x200, 0x400, CRC(a5cda23e) SHA1(d6c8534ae3c95b47a0701047fef67f15dd71f3fe) )
ROM_END

ROM_START( ffantasyj )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 4*64k for 68000 code */
	ROM_LOAD16_BYTE( "ff-02-2.bin",  0x00000, 0x10000, CRC(29fc22a7) SHA1(73cbd47c34bee22c16a69cfc6037a60dc30effe8) )
	ROM_LOAD16_BYTE( "ff-01-2.bin",  0x00001, 0x10000, CRC(9f617cb4) SHA1(447ea4e57dd6b23aaf48e5e14c7893277730c7d9) )
	ROM_LOAD16_BYTE( "ew05",         0x20000, 0x10000, CRC(c76d65ec) SHA1(620990acaf2fd7f3fbfe7135a17ac0195feb8330) )
	ROM_LOAD16_BYTE( "ew00",         0x20001, 0x10000, CRC(e9b427a6) SHA1(b334992846771739d31756724138b82f897dfad5) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 sound */
	ROM_LOAD( "ew04",         0x8000, 0x8000, CRC(9871b98d) SHA1(2b6c46bc2b10a28946d6ad8251e1a156a0b99947) )

	ROM_REGION( 0x10000, "sub", 0 ) /* HuC6280 CPU */
	ROM_LOAD( "ew08",         0x00000, 0x10000, CRC(53010534) SHA1(8b996e48414bacd009e05ff49848884ecf15d967) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "ev14",         0x00000, 0x10000, CRC(686f72c1) SHA1(41d4fc1208d779f3428990a96586f6a555c28562) )
	ROM_LOAD( "ev13",         0x10000, 0x10000, CRC(b787dcc9) SHA1(7fce9d2040bcb2483419ea1cafed538bb8aba4f9) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "ew19",         0x00000, 0x08000, CRC(6b80d7a3) SHA1(323162e7e0ce16f6244d8d98fdb2396ffef87e82) )
	ROM_LOAD( "ew18",         0x08000, 0x08000, CRC(78d3d764) SHA1(e8f77a23bd4f4d268bec7c0153fb957acd07cdee) )
	ROM_LOAD( "ew20",         0x10000, 0x08000, CRC(ce9f5de3) SHA1(b8af33f52ca3579a45b41395751697a58931f9d6) )
	ROM_LOAD( "ew21",         0x18000, 0x08000, CRC(487a7ba2) SHA1(7d52cc1517def8426355e8281440ec5e617d1121) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "ew24",         0x00000, 0x08000, CRC(4e1bc2a4) SHA1(d7d4c42fd932722436f1847929088e46d03184bd) )
	ROM_LOAD( "ew25",         0x08000, 0x08000, CRC(9eb47dfb) SHA1(bb1e8a3a47f447f3a983ea51943d3081d56ad9a4) )
	ROM_LOAD( "ew23",         0x10000, 0x08000, CRC(9ecf479e) SHA1(a8d4c1490f12e1b15d53a2a97147920dcb638378) )
	ROM_LOAD( "ew22",         0x18000, 0x08000, CRC(e55669aa) SHA1(2a9b0e85bb81ff87a108e08b28e19b7b469463e4) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "ev15",         0x00000, 0x10000, CRC(1d80f797) SHA1(1b6878155367350ff826593ea73bda5b893c1823) )
	ROM_LOAD( "ew16",         0x10000, 0x10000, CRC(96233177) SHA1(929a1b7fb65ab33277719b84517ff57da563f875) )
	ROM_LOAD( "ev10",         0x20000, 0x10000, CRC(c4e7116b) SHA1(1e665ba150e08ceb1c0d5f7b7e777f3d60997811) )
	ROM_LOAD( "ew11",         0x30000, 0x10000, CRC(f2e007fc) SHA1(da30ad3725b9bc4a07dbb1afa05f145c3574c84c) )
	ROM_LOAD( "ev06",         0x40000, 0x10000, CRC(6c794f1a) SHA1(ab7996917bea99850aef5a0890485dd27778cd99) )
	ROM_LOAD( "ew07",         0x50000, 0x10000, CRC(470b6989) SHA1(16b292d8a3a54048bf29f0b4f41bb6ca049b347c) )
	ROM_LOAD( "ev17",         0x60000, 0x10000, CRC(045509d4) SHA1(ebbd71de8e8492ff6321e3ede0d98d9ed462de01) )
	ROM_LOAD( "ew12",         0x70000, 0x10000, CRC(a2d244bc) SHA1(ff2391efc480f36a302650691f8a7a620b86d99a) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "ew03",         0x0000, 0x10000, CRC(b606924d) SHA1(b759fcec10b333465cf5cd1b30987bf2d62186b2) )

	ROM_REGION( 0x600, "proms", 0 ) /* PROMs */
	ROM_LOAD( "mb7116e_a-1.12c", 0x000, 0x200, CRC(86e775f8) SHA1(e8dee3d56fb5ca0fd7f9ce05a84674abb139d008) )
	ROM_LOAD( "mb7122e_a-2.17e", 0x200, 0x400, CRC(a5cda23e) SHA1(d6c8534ae3c95b47a0701047fef67f15dd71f3fe) )
ROM_END

// I believe 'EX' is the world (export) release, but it still shows a 'ONLY FOR USE IN JAPAN' screen
// 'EV' is original Japanese release.
// 'EW' is US release.
ROM_START( ffantasy )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 4*64k for 68000 code */
	ROM_LOAD16_BYTE( "ex02-3.4b",  0x00000, 0x10000, CRC(df0d7dc6) SHA1(a60197ad6f19f730e05cf6a3be9181f28d425344) )
	ROM_LOAD16_BYTE( "ex01-3.3b",  0x00001, 0x10000, CRC(c0fb4fe5) SHA1(f6ed0904ec19491c4ed5f5e5da16df02476ab1f3) )
	ROM_LOAD16_BYTE( "ex05-.4c",   0x20000, 0x10000, CRC(c76d65ec) SHA1(620990acaf2fd7f3fbfe7135a17ac0195feb8330) )
	ROM_LOAD16_BYTE( "ex00-.1b",   0x20001, 0x10000, CRC(e9b427a6) SHA1(b334992846771739d31756724138b82f897dfad5) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 sound */
	ROM_LOAD( "ex04-.1c",         0x8000, 0x8000, CRC(9871b98d) SHA1(2b6c46bc2b10a28946d6ad8251e1a156a0b99947) )

	ROM_REGION( 0x10000, "sub", 0 ) /* HuC6280 CPU */
	ROM_LOAD( "ew08",         0x00000, 0x10000, CRC(53010534) SHA1(8b996e48414bacd009e05ff49848884ecf15d967) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "ev14",         0x00000, 0x10000, CRC(686f72c1) SHA1(41d4fc1208d779f3428990a96586f6a555c28562) )
	ROM_LOAD( "ev13",         0x10000, 0x10000, CRC(b787dcc9) SHA1(7fce9d2040bcb2483419ea1cafed538bb8aba4f9) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "ew19",         0x00000, 0x08000, CRC(6b80d7a3) SHA1(323162e7e0ce16f6244d8d98fdb2396ffef87e82) )
	ROM_LOAD( "ew18",         0x08000, 0x08000, CRC(78d3d764) SHA1(e8f77a23bd4f4d268bec7c0153fb957acd07cdee) )
	ROM_LOAD( "ew20",         0x10000, 0x08000, CRC(ce9f5de3) SHA1(b8af33f52ca3579a45b41395751697a58931f9d6) )
	ROM_LOAD( "ew21",         0x18000, 0x08000, CRC(487a7ba2) SHA1(7d52cc1517def8426355e8281440ec5e617d1121) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "ew24",         0x00000, 0x08000, CRC(4e1bc2a4) SHA1(d7d4c42fd932722436f1847929088e46d03184bd) )
	ROM_LOAD( "ew25",         0x08000, 0x08000, CRC(9eb47dfb) SHA1(bb1e8a3a47f447f3a983ea51943d3081d56ad9a4) )
	ROM_LOAD( "ew23",         0x10000, 0x08000, CRC(9ecf479e) SHA1(a8d4c1490f12e1b15d53a2a97147920dcb638378) )
	ROM_LOAD( "ew22",         0x18000, 0x08000, CRC(e55669aa) SHA1(2a9b0e85bb81ff87a108e08b28e19b7b469463e4) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "ev15",         0x00000, 0x10000, CRC(1d80f797) SHA1(1b6878155367350ff826593ea73bda5b893c1823) )
	ROM_LOAD( "ew16",         0x10000, 0x10000, CRC(96233177) SHA1(929a1b7fb65ab33277719b84517ff57da563f875) )
	ROM_LOAD( "ev10",         0x20000, 0x10000, CRC(c4e7116b) SHA1(1e665ba150e08ceb1c0d5f7b7e777f3d60997811) )
	ROM_LOAD( "ew11",         0x30000, 0x10000, CRC(f2e007fc) SHA1(da30ad3725b9bc4a07dbb1afa05f145c3574c84c) )
	ROM_LOAD( "ev06",         0x40000, 0x10000, CRC(6c794f1a) SHA1(ab7996917bea99850aef5a0890485dd27778cd99) )
	ROM_LOAD( "ew07",         0x50000, 0x10000, CRC(470b6989) SHA1(16b292d8a3a54048bf29f0b4f41bb6ca049b347c) )
	ROM_LOAD( "ev17",         0x60000, 0x10000, CRC(045509d4) SHA1(ebbd71de8e8492ff6321e3ede0d98d9ed462de01) )
	ROM_LOAD( "ew12",         0x70000, 0x10000, CRC(a2d244bc) SHA1(ff2391efc480f36a302650691f8a7a620b86d99a) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "ew03",         0x0000, 0x10000, CRC(b606924d) SHA1(b759fcec10b333465cf5cd1b30987bf2d62186b2) )

	ROM_REGION( 0x600, "proms", 0 ) /* PROMs */
	ROM_LOAD( "mb7116e_a-1.12c", 0x000, 0x200, CRC(86e775f8) SHA1(e8dee3d56fb5ca0fd7f9ce05a84674abb139d008) )
	ROM_LOAD( "mb7122e_a-2.17e", 0x200, 0x400, CRC(a5cda23e) SHA1(d6c8534ae3c95b47a0701047fef67f15dd71f3fe) )
ROM_END

ROM_START( ffantasya )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 4*64k for 68000 code */
	ROM_LOAD16_BYTE( "ev02",         0x00000, 0x10000, CRC(797a7860) SHA1(aaab24c99e96b393d2bda435f18b0dc4003cdf09) )
	ROM_LOAD16_BYTE( "ev01",         0x00001, 0x10000, CRC(0f17184d) SHA1(c1bcd6347df9bee2d2d9ca29b22af9235493871c) )
	ROM_LOAD16_BYTE( "ew05",         0x20000, 0x10000, CRC(c76d65ec) SHA1(620990acaf2fd7f3fbfe7135a17ac0195feb8330) )
	ROM_LOAD16_BYTE( "ew00",         0x20001, 0x10000, CRC(e9b427a6) SHA1(b334992846771739d31756724138b82f897dfad5) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 sound */
	ROM_LOAD( "ew04",         0x8000, 0x8000, CRC(9871b98d) SHA1(2b6c46bc2b10a28946d6ad8251e1a156a0b99947) )

	ROM_REGION( 0x10000, "sub", 0 ) /* HuC6280 CPU */
	ROM_LOAD( "ew08",         0x00000, 0x10000, CRC(53010534) SHA1(8b996e48414bacd009e05ff49848884ecf15d967) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "ev14",         0x00000, 0x10000, CRC(686f72c1) SHA1(41d4fc1208d779f3428990a96586f6a555c28562) )
	ROM_LOAD( "ev13",         0x10000, 0x10000, CRC(b787dcc9) SHA1(7fce9d2040bcb2483419ea1cafed538bb8aba4f9) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "ew19",         0x00000, 0x08000, CRC(6b80d7a3) SHA1(323162e7e0ce16f6244d8d98fdb2396ffef87e82) )
	ROM_LOAD( "ew18",         0x08000, 0x08000, CRC(78d3d764) SHA1(e8f77a23bd4f4d268bec7c0153fb957acd07cdee) )
	ROM_LOAD( "ew20",         0x10000, 0x08000, CRC(ce9f5de3) SHA1(b8af33f52ca3579a45b41395751697a58931f9d6) )
	ROM_LOAD( "ew21",         0x18000, 0x08000, CRC(487a7ba2) SHA1(7d52cc1517def8426355e8281440ec5e617d1121) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "ew24",         0x00000, 0x08000, CRC(4e1bc2a4) SHA1(d7d4c42fd932722436f1847929088e46d03184bd) )
	ROM_LOAD( "ew25",         0x08000, 0x08000, CRC(9eb47dfb) SHA1(bb1e8a3a47f447f3a983ea51943d3081d56ad9a4) )
	ROM_LOAD( "ew23",         0x10000, 0x08000, CRC(9ecf479e) SHA1(a8d4c1490f12e1b15d53a2a97147920dcb638378) )
	ROM_LOAD( "ew22",         0x18000, 0x08000, CRC(e55669aa) SHA1(2a9b0e85bb81ff87a108e08b28e19b7b469463e4) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "ev15",         0x00000, 0x10000, CRC(1d80f797) SHA1(1b6878155367350ff826593ea73bda5b893c1823) )
	ROM_LOAD( "ew16",         0x10000, 0x10000, CRC(96233177) SHA1(929a1b7fb65ab33277719b84517ff57da563f875) )
	ROM_LOAD( "ev10",         0x20000, 0x10000, CRC(c4e7116b) SHA1(1e665ba150e08ceb1c0d5f7b7e777f3d60997811) )
	ROM_LOAD( "ew11",         0x30000, 0x10000, CRC(f2e007fc) SHA1(da30ad3725b9bc4a07dbb1afa05f145c3574c84c) )
	ROM_LOAD( "ev06",         0x40000, 0x10000, CRC(6c794f1a) SHA1(ab7996917bea99850aef5a0890485dd27778cd99) )
	ROM_LOAD( "ew07",         0x50000, 0x10000, CRC(470b6989) SHA1(16b292d8a3a54048bf29f0b4f41bb6ca049b347c) )
	ROM_LOAD( "ev17",         0x60000, 0x10000, CRC(045509d4) SHA1(ebbd71de8e8492ff6321e3ede0d98d9ed462de01) )
	ROM_LOAD( "ew12",         0x70000, 0x10000, CRC(a2d244bc) SHA1(ff2391efc480f36a302650691f8a7a620b86d99a) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "ew03",         0x0000, 0x10000, CRC(b606924d) SHA1(b759fcec10b333465cf5cd1b30987bf2d62186b2) )

	ROM_REGION( 0x600, "proms", 0 ) /* PROMs */
	ROM_LOAD( "mb7116e_a-1.12c", 0x000, 0x200, CRC(86e775f8) SHA1(e8dee3d56fb5ca0fd7f9ce05a84674abb139d008) )
	ROM_LOAD( "mb7122e_a-2.17e", 0x200, 0x400, CRC(a5cda23e) SHA1(d6c8534ae3c95b47a0701047fef67f15dd71f3fe) )
ROM_END

ROM_START( ffantasyb )  // DE-0297-3 PCB. All EX labels.
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 4*64k for 68000 code */
	ROM_LOAD16_BYTE( "ex02-2",       0x00000, 0x10000, CRC(4c26cda6) SHA1(475eb30da7020bf2b1546e3878973231aa52d522) )
	ROM_LOAD16_BYTE( "ex01",         0x00001, 0x10000, CRC(d2c4ab91) SHA1(3134e5aa9815e9ca46601c46268a91414f907fce) )
	ROM_LOAD16_BYTE( "ex05",         0x20000, 0x10000, CRC(c76d65ec) SHA1(620990acaf2fd7f3fbfe7135a17ac0195feb8330) )
	ROM_LOAD16_BYTE( "ex00",         0x20001, 0x10000, CRC(e9b427a6) SHA1(b334992846771739d31756724138b82f897dfad5) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 sound */
	ROM_LOAD( "ex04",         0x8000, 0x8000, CRC(9871b98d) SHA1(2b6c46bc2b10a28946d6ad8251e1a156a0b99947) )

	ROM_REGION( 0x10000, "sub", 0 ) /* HuC6280 CPU */
	ROM_LOAD( "ex08",         0x00000, 0x10000, CRC(53010534) SHA1(8b996e48414bacd009e05ff49848884ecf15d967) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "ex14",         0x00000, 0x10000, CRC(686f72c1) SHA1(41d4fc1208d779f3428990a96586f6a555c28562) )
	ROM_LOAD( "ex13",         0x10000, 0x10000, CRC(b787dcc9) SHA1(7fce9d2040bcb2483419ea1cafed538bb8aba4f9) )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "ex19",         0x00000, 0x08000, CRC(6b80d7a3) SHA1(323162e7e0ce16f6244d8d98fdb2396ffef87e82) )
	ROM_LOAD( "ex18",         0x08000, 0x08000, CRC(78d3d764) SHA1(e8f77a23bd4f4d268bec7c0153fb957acd07cdee) )
	ROM_LOAD( "ex20",         0x10000, 0x08000, CRC(ce9f5de3) SHA1(b8af33f52ca3579a45b41395751697a58931f9d6) )
	ROM_LOAD( "ex21",         0x18000, 0x08000, CRC(487a7ba2) SHA1(7d52cc1517def8426355e8281440ec5e617d1121) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "ex24",         0x00000, 0x08000, CRC(4e1bc2a4) SHA1(d7d4c42fd932722436f1847929088e46d03184bd) )
	ROM_LOAD( "ex25",         0x08000, 0x08000, CRC(9eb47dfb) SHA1(bb1e8a3a47f447f3a983ea51943d3081d56ad9a4) )
	ROM_LOAD( "ex23",         0x10000, 0x08000, CRC(9ecf479e) SHA1(a8d4c1490f12e1b15d53a2a97147920dcb638378) )
	ROM_LOAD( "ex22",         0x18000, 0x08000, CRC(e55669aa) SHA1(2a9b0e85bb81ff87a108e08b28e19b7b469463e4) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "ex15",         0x00000, 0x10000, CRC(95423914) SHA1(e9e7a6bdf5aa717dc04a751709632f31762886fb) )
	ROM_LOAD( "ex16",         0x10000, 0x10000, CRC(96233177) SHA1(929a1b7fb65ab33277719b84517ff57da563f875) )
	ROM_LOAD( "ex10",         0x20000, 0x10000, CRC(4c25dfe8) SHA1(e4334de96698cd0112a8926dea131e748b6a84fc) )
	ROM_LOAD( "ex11",         0x30000, 0x10000, CRC(f2e007fc) SHA1(da30ad3725b9bc4a07dbb1afa05f145c3574c84c) )
	ROM_LOAD( "ex06",         0x40000, 0x10000, CRC(e4bb8199) SHA1(49b5b45c7cd9c44f53d83ee2a156d9e9f8a53960) )
	ROM_LOAD( "ex07",         0x50000, 0x10000, CRC(470b6989) SHA1(16b292d8a3a54048bf29f0b4f41bb6ca049b347c) )
	ROM_LOAD( "ex17",         0x60000, 0x10000, CRC(8c97c757) SHA1(36fd807da9e144dfb29c8252e9450cc37ca2604f) )
	ROM_LOAD( "ex12",         0x70000, 0x10000, CRC(a2d244bc) SHA1(ff2391efc480f36a302650691f8a7a620b86d99a) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "ex03",         0x0000, 0x10000, CRC(b606924d) SHA1(b759fcec10b333465cf5cd1b30987bf2d62186b2) )

	ROM_REGION( 0x600, "proms", 0 ) /* PROMs */
	ROM_LOAD( "mb7116e_a-1.12c", 0x000, 0x200, CRC(86e775f8) SHA1(e8dee3d56fb5ca0fd7f9ce05a84674abb139d008) )
	ROM_LOAD( "mb7122e_a-2.17e", 0x200, 0x400, CRC(a5cda23e) SHA1(d6c8534ae3c95b47a0701047fef67f15dd71f3fe) )
ROM_END

/* this is probably a bootleg of an undumped original revision */
ROM_START( ffantasybl )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 4*64k for 68000 code */
	ROM_LOAD16_BYTE( "14.bin",  0x00000, 0x10000, CRC(bd42bc66) SHA1(d81a3d16ca282817f85372d1426470900a553b24) ) // 61.926270% ff-02-2.bin
	ROM_LOAD16_BYTE( "11.bin",  0x00001, 0x10000, CRC(4df38e4b) SHA1(e176afb7b63e2e1ac482662d152da2866884594e) ) // 55.798340% ff-01-2.bin
	ROM_LOAD16_BYTE( "13.bin",  0x20000, 0x10000, CRC(eecb6bed) SHA1(f5761bfc01ae207d3a321aa4ad510f6af8ad6094) ) // 86.532593% ew05
	ROM_LOAD16_BYTE( "10.bin",  0x20001, 0x10000, CRC(7cdcf418) SHA1(9653b6620dce70bd510fb63ba5c324dda581a412) ) // 85.887146% ew00

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 sound */
	ROM_LOAD( "15.bin",         0x8000, 0x8000, CRC(9871b98d) SHA1(2b6c46bc2b10a28946d6ad8251e1a156a0b99947) )

	ROM_REGION( 0x1000, "mcu", 0 )    /* 68705 MCU */ // (labeled on PCB as Z80, but it isn't!)
	ROM_LOAD( "68705u3.bin",              0x00000, 0x1000, NO_DUMP ) // nor dumped, maybe it's the same as the midresb one?

	ROM_REGION( 0x20000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "22.bin",         0x00000, 0x10000, CRC(686f72c1) SHA1(41d4fc1208d779f3428990a96586f6a555c28562) )
	ROM_LOAD( "23.bin",         0x10000, 0x10000, CRC(28e69371) SHA1(32d57aabf948388825757ab0cfe87b6550a07a9d) ) // 94.793701% ev13

	ROM_REGION( 0x20000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "25.bin",         0x00000, 0x08000, CRC(6b80d7a3) SHA1(323162e7e0ce16f6244d8d98fdb2396ffef87e82) )
	ROM_LOAD( "27.bin",         0x08000, 0x08000, CRC(78d3d764) SHA1(e8f77a23bd4f4d268bec7c0153fb957acd07cdee) )
	ROM_LOAD( "24.bin",         0x10000, 0x08000, CRC(ce9f5de3) SHA1(b8af33f52ca3579a45b41395751697a58931f9d6) )
	ROM_LOAD( "26.bin",         0x18000, 0x08000, CRC(487a7ba2) SHA1(7d52cc1517def8426355e8281440ec5e617d1121) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "29.bin",         0x00000, 0x08000, CRC(4e1bc2a4) SHA1(d7d4c42fd932722436f1847929088e46d03184bd) )
	ROM_LOAD( "21.bin",         0x08000, 0x08000, CRC(28b37d27) SHA1(c70718f8ce23f75a728dc0a7556fd7d259048b88) )
	ROM_IGNORE(0x8000) // same content, double size as original, ignore 2nd half
	ROM_LOAD( "28.bin",         0x10000, 0x08000, CRC(9ecf479e) SHA1(a8d4c1490f12e1b15d53a2a97147920dcb638378) )
	ROM_LOAD( "20.bin",         0x18000, 0x08000, CRC(b5ca8ed9) SHA1(3f44ebf7fec76154a843ee4398d4ac8690e70342) )
	ROM_IGNORE(0x8000) // same content, double size as original, ignore 2nd half

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "6.bin",         0x00000, 0x10000, CRC(95423914) SHA1(e9e7a6bdf5aa717dc04a751709632f31762886fb) )
	ROM_LOAD( "3.bin",         0x10000, 0x10000, CRC(96233177) SHA1(929a1b7fb65ab33277719b84517ff57da563f875) )
	ROM_LOAD( "8.bin",         0x20000, 0x10000, CRC(4c25dfe8) SHA1(e4334de96698cd0112a8926dea131e748b6a84fc) )
	ROM_LOAD( "4.bin",         0x30000, 0x10000, CRC(f2e007fc) SHA1(da30ad3725b9bc4a07dbb1afa05f145c3574c84c) )
	ROM_LOAD( "5.bin",         0x40000, 0x10000, CRC(bc6028c4) SHA1(6ca5bb328912df23ad3d61b596b4a35f2815ef31) ) // 99.996948% ew06 (bad dump?)
	ROM_LOAD( "1.bin",         0x50000, 0x10000, CRC(470b6989) SHA1(16b292d8a3a54048bf29f0b4f41bb6ca049b347c) )
	ROM_LOAD( "7.bin",         0x60000, 0x10000, CRC(8c97c757) SHA1(36fd807da9e144dfb29c8252e9450cc37ca2604f) )
	ROM_LOAD( "2.bin",         0x70000, 0x10000, CRC(a2d244bc) SHA1(ff2391efc480f36a302650691f8a7a620b86d99a) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "30.bin",         0x0000, 0x10000, CRC(b606924d) SHA1(b759fcec10b333465cf5cd1b30987bf2d62186b2) )
ROM_END


ROM_START( secretag ) /* DE-0322-2 PCB */
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "fb14-3.17l", 0x00000, 0x10000, CRC(9be6ac90) SHA1(1c78af9da63add7c77c8d2ce24924505481381b1) )
	ROM_LOAD16_BYTE( "fb12-3.9l",  0x00001, 0x10000, CRC(28904b6b) SHA1(c3fd42c3ba5b19c3483df3ac9e44016570762de7) )
	ROM_LOAD16_BYTE( "fb15.19l",   0x20000, 0x10000, CRC(106bb26c) SHA1(e5d05124b6dfc54e41dcf40916633caaa9a19823) )
	ROM_LOAD16_BYTE( "fb13.11l",   0x20001, 0x10000, CRC(90523413) SHA1(7ea65525f2d7c577255aa01260acc5f43d136b3c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "fb10.5h", 0x00000, 0x10000, CRC(dfd2ff25) SHA1(3dcd6d50b92b49daae4b51581abe9c95f764e848) )

	ROM_REGION( 0x10000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "fb05.11a", 0x04000, 0x04000, CRC(09802924) SHA1(d9bc5fe7f053afa15cd39400aae993866d1b0226) )
	ROM_CONTINUE(         0x00000, 0x04000 )    /* the two halves are swapped */
	ROM_LOAD( "fb04.9a",  0x0c000, 0x04000, CRC(ec25b895) SHA1(8c1d2b9a2487fd7114d37fe9dc271183c4cc1613) )
	ROM_CONTINUE(         0x08000, 0x04000 )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "fb07.17a", 0x00000, 0x10000, CRC(e932268b) SHA1(ee8ed29affa951e725cf19a5f56d3beac24420c9) )
	ROM_LOAD( "fb06.15a", 0x10000, 0x10000, CRC(c4dd38c0) SHA1(267dbbdd5df6b13662cd307c5c95fdf643d64f45) )

	ROM_REGION( 0x40000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "fb09.22a", 0x00000, 0x20000, CRC(1395e9be) SHA1(60693ac6236ffe1e0933d81771cfad32e14514c3) )
	ROM_LOAD( "fb08.21a", 0x20000, 0x20000, CRC(4d7464db) SHA1(82e2a3c3d78447985968220d52c7c1f1ff625d83) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "fb01.4a", 0x00000, 0x20000, CRC(99b0cd92) SHA1(2729e874730391b5fa93e9a28142c02c00eb5068) )
	ROM_LOAD( "fb03.7a", 0x20000, 0x20000, CRC(0e7ea74d) SHA1(22078a2856933af2d31750a4a506b993fe309e9a) )
	ROM_LOAD( "fb00.2a", 0x40000, 0x20000, CRC(f7df3fd7) SHA1(ed9e4649e0b1fcca61cf4d159b3f8a35f06102ce) )
	ROM_LOAD( "fb02.5a", 0x60000, 0x20000, CRC(84e8da9d) SHA1(41da6042f80ea3562aa350f4f466b16db29e2aca) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "fa11.11k", 0x00000, 0x20000, CRC(4e547bad) SHA1(655eda4d00f8846957ed40dcbf750fba3ce19f4e) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7114h.21k", 0x0000, 0x0100, CRC(ad26e8d4) SHA1(827337aeb8904429a1c050279240ae38aa6ce064) )    /* Priority (not used) */

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8b-ta-1.15k", 0x0000, 0x0104, CRC(79a87527) SHA1(3c6ad20d5a7c41c020e671d462c0b1e4a5dda7f8) )
	ROM_LOAD( "pal16r4a-ta-2.16k", 0x0200, 0x0104, CRC(eca31311) SHA1(a87b2721e13767f7448236d0bbb3355583fe88bb) )
	ROM_LOAD( "pal16l8a-ta-3.17k", 0x0400, 0x0104, CRC(6c324919) SHA1(83bba4634d7ab7c4ad3083c063804fd1e7c9c10b) )
	ROM_LOAD( "pal16l8a-ta-4.11m", 0x0600, 0x0104, CRC(116177fa) SHA1(f63802578b6f743e2d3a64e4805488d44361dcb2) )
ROM_END

ROM_START( secretagj ) /* DE-0322-2 PCB */
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "fc14-2.17l", 0x00000, 0x10000, CRC(e4cc767d) SHA1(0173a446f4af0b6853b497e67e229b2b3568f1d4) )
	ROM_LOAD16_BYTE( "fc12-2.9l",  0x00001, 0x10000, CRC(8a589c90) SHA1(cc97f0ed7da9b936f9a59a010bacc1b187008135) )
	ROM_LOAD16_BYTE( "fc15.19l",   0x20000, 0x10000, CRC(106bb26c) SHA1(e5d05124b6dfc54e41dcf40916633caaa9a19823) ) // == FB counterpart from World set
	ROM_LOAD16_BYTE( "fc13.11l",   0x20001, 0x10000, CRC(90523413) SHA1(7ea65525f2d7c577255aa01260acc5f43d136b3c) ) // == FB counterpart from World set

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "fc10.5h", 0x00000, 0x10000, CRC(dfd2ff25) SHA1(3dcd6d50b92b49daae4b51581abe9c95f764e848) ) // == FB counterpart from World set

	ROM_REGION( 0x10000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "fc05.11a", 0x04000, 0x04000, CRC(09802924) SHA1(d9bc5fe7f053afa15cd39400aae993866d1b0226) ) // == FB counterpart from World set
	ROM_CONTINUE(         0x00000, 0x04000 )    /* the two halves are swapped */
	ROM_LOAD( "fc04.9a",  0x0c000, 0x04000, CRC(ec25b895) SHA1(8c1d2b9a2487fd7114d37fe9dc271183c4cc1613) ) // == FB counterpart from World set
	ROM_CONTINUE(         0x08000, 0x04000 )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "fc07.17a", 0x00000, 0x10000, CRC(e932268b) SHA1(ee8ed29affa951e725cf19a5f56d3beac24420c9) ) // == FB counterpart from World set
	ROM_LOAD( "fc06.15a", 0x10000, 0x10000, CRC(c4dd38c0) SHA1(267dbbdd5df6b13662cd307c5c95fdf643d64f45) ) // == FB counterpart from World set

	ROM_REGION( 0x40000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "fc09.22a", 0x00000, 0x20000, CRC(1395e9be) SHA1(60693ac6236ffe1e0933d81771cfad32e14514c3) ) // == FB counterpart from World set
	ROM_LOAD( "fc08.21a", 0x20000, 0x20000, CRC(4d7464db) SHA1(82e2a3c3d78447985968220d52c7c1f1ff625d83) ) // == FB counterpart from World set

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "fc01.4a", 0x00000, 0x20000, CRC(99b0cd92) SHA1(2729e874730391b5fa93e9a28142c02c00eb5068) ) // == FB counterpart from World set
	ROM_LOAD( "fc03.7a", 0x20000, 0x20000, CRC(0e7ea74d) SHA1(22078a2856933af2d31750a4a506b993fe309e9a) ) // == FB counterpart from World set
	ROM_LOAD( "fc00.2a", 0x40000, 0x20000, CRC(f7df3fd7) SHA1(ed9e4649e0b1fcca61cf4d159b3f8a35f06102ce) ) // == FB counterpart from World set
	ROM_LOAD( "fc02.5a", 0x60000, 0x20000, CRC(84e8da9d) SHA1(41da6042f80ea3562aa350f4f466b16db29e2aca) ) // == FB counterpart from World set

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "fc11.11k", 0x00000, 0x20000, CRC(4e547bad) SHA1(655eda4d00f8846957ed40dcbf750fba3ce19f4e) ) // == FB counterpart from World set

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7114h.21k", 0x0000, 0x0100, CRC(ad26e8d4) SHA1(827337aeb8904429a1c050279240ae38aa6ce064) )    /* Priority (not used) */

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8b-ta-1.15k", 0x0000, 0x0104, CRC(79a87527) SHA1(3c6ad20d5a7c41c020e671d462c0b1e4a5dda7f8) )
	ROM_LOAD( "pal16r4a-ta-2.16k", 0x0200, 0x0104, CRC(eca31311) SHA1(a87b2721e13767f7448236d0bbb3355583fe88bb) )
	ROM_LOAD( "pal16l8a-ta-3.17k", 0x0400, 0x0104, CRC(6c324919) SHA1(83bba4634d7ab7c4ad3083c063804fd1e7c9c10b) )
	ROM_LOAD( "pal16l8a-ta-4.11m", 0x0600, 0x0104, CRC(116177fa) SHA1(f63802578b6f743e2d3a64e4805488d44361dcb2) )
ROM_END

ROM_START( slyspy ) /* DE-0322-3 PCB */
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "fa14-4.17l", 0x00000, 0x10000, CRC(60f16e31) SHA1(b2359fe8ecbed63b7d56c9962fab16b354a14305) )
	ROM_LOAD16_BYTE( "fa12-4.9l",  0x00001, 0x10000, CRC(b9b9fdcf) SHA1(fdd91b9bc8f0228078bb50323531808076180fe7) )
	ROM_LOAD16_BYTE( "fa15.19l",   0x20000, 0x10000, CRC(04a79266) SHA1(69d256ffb1c89721f8b1e929c581f187e047b977) )
	ROM_LOAD16_BYTE( "fa13.11l",   0x20001, 0x10000, CRC(641cc4b3) SHA1(ce0ccd14d201f411cfc02ec988b2ad4fcb0d8f5d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "fa10.5h", 0x00000, 0x10000, CRC(dfd2ff25) SHA1(3dcd6d50b92b49daae4b51581abe9c95f764e848) ) // == FB counterpart from World set

	ROM_REGION( 0x10000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "fa05.11a", 0x04000, 0x04000, CRC(09802924) SHA1(d9bc5fe7f053afa15cd39400aae993866d1b0226) ) // == FB counterpart from World set
	ROM_CONTINUE(         0x00000, 0x04000 )    /* the two halves are swapped */
	ROM_LOAD( "fa04.9a",  0x0c000, 0x04000, CRC(ec25b895) SHA1(8c1d2b9a2487fd7114d37fe9dc271183c4cc1613) ) // == FB counterpart from World set
	ROM_CONTINUE(         0x08000, 0x04000 )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "fa07.17a", 0x00000, 0x10000, CRC(e932268b) SHA1(ee8ed29affa951e725cf19a5f56d3beac24420c9) ) // == FB counterpart from World set
	ROM_LOAD( "fa06.15a", 0x10000, 0x10000, CRC(c4dd38c0) SHA1(267dbbdd5df6b13662cd307c5c95fdf643d64f45) ) // == FB counterpart from World set

	ROM_REGION( 0x40000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "fa09.22a", 0x00000, 0x20000, CRC(1395e9be) SHA1(60693ac6236ffe1e0933d81771cfad32e14514c3) ) // == FB counterpart from World set
	ROM_LOAD( "fa08.21a", 0x20000, 0x20000, CRC(4d7464db) SHA1(82e2a3c3d78447985968220d52c7c1f1ff625d83) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "fa01.4a", 0x00000, 0x20000, CRC(99b0cd92) SHA1(2729e874730391b5fa93e9a28142c02c00eb5068) ) // == FB counterpart from World set
	ROM_LOAD( "fa03.7a", 0x20000, 0x20000, CRC(0e7ea74d) SHA1(22078a2856933af2d31750a4a506b993fe309e9a) ) // == FB counterpart from World set
	ROM_LOAD( "fa00.2a", 0x40000, 0x20000, CRC(f7df3fd7) SHA1(ed9e4649e0b1fcca61cf4d159b3f8a35f06102ce) ) // == FB counterpart from World set
	ROM_LOAD( "fa02.5a", 0x60000, 0x20000, CRC(84e8da9d) SHA1(41da6042f80ea3562aa350f4f466b16db29e2aca) ) // == FB counterpart from World set

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "fa11.11k", 0x00000, 0x20000, CRC(4e547bad) SHA1(655eda4d00f8846957ed40dcbf750fba3ce19f4e) ) // == FB counterpart from World set

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7114h.21k", 0x0000, 0x0100, CRC(ad26e8d4) SHA1(827337aeb8904429a1c050279240ae38aa6ce064) )    /* Priority (not used) */

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8b-ta-1.15k", 0x0000, 0x0104, CRC(79a87527) SHA1(3c6ad20d5a7c41c020e671d462c0b1e4a5dda7f8) )
	ROM_LOAD( "pal16r4a-ta-2.16k", 0x0200, 0x0104, CRC(eca31311) SHA1(a87b2721e13767f7448236d0bbb3355583fe88bb) )
	ROM_LOAD( "pal16l8a-ta-3.17k", 0x0400, 0x0104, CRC(6c324919) SHA1(83bba4634d7ab7c4ad3083c063804fd1e7c9c10b) )
	ROM_LOAD( "pal16l8a-ta-4.11m", 0x0600, 0x0104, CRC(116177fa) SHA1(f63802578b6f743e2d3a64e4805488d44361dcb2) )
ROM_END

ROM_START( slyspy3 ) /* DE-0322-3 PCB */
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "fa14-3.17l", 0x00000, 0x10000, CRC(54353a84) SHA1(899559f17705a8222fd56e9304e9b802eac8f6db) )
	ROM_LOAD16_BYTE( "fa12-2.9l",  0x00001, 0x10000, CRC(1b534294) SHA1(cf7badea6604c47d9f3ff8a0ef326e09de1974a0) )
	ROM_LOAD16_BYTE( "fa15.19l",   0x20000, 0x10000, CRC(04a79266) SHA1(69d256ffb1c89721f8b1e929c581f187e047b977) )
	ROM_LOAD16_BYTE( "fa13.11l",   0x20001, 0x10000, CRC(641cc4b3) SHA1(ce0ccd14d201f411cfc02ec988b2ad4fcb0d8f5d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "fa10.5h", 0x00000, 0x10000, CRC(dfd2ff25) SHA1(3dcd6d50b92b49daae4b51581abe9c95f764e848) ) // == FB counterpart from World set

	ROM_REGION( 0x10000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "fa05.11a", 0x04000, 0x04000, CRC(09802924) SHA1(d9bc5fe7f053afa15cd39400aae993866d1b0226) ) // == FB counterpart from World set
	ROM_CONTINUE(         0x00000, 0x04000 )    /* the two halves are swapped */
	ROM_LOAD( "fa04.9a",  0x0c000, 0x04000, CRC(ec25b895) SHA1(8c1d2b9a2487fd7114d37fe9dc271183c4cc1613) ) // == FB counterpart from World set
	ROM_CONTINUE(         0x08000, 0x04000 )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "fa07.17a", 0x00000, 0x10000, CRC(e932268b) SHA1(ee8ed29affa951e725cf19a5f56d3beac24420c9) ) // == FB counterpart from World set
	ROM_LOAD( "fa06.15a", 0x10000, 0x10000, CRC(c4dd38c0) SHA1(267dbbdd5df6b13662cd307c5c95fdf643d64f45) ) // == FB counterpart from World set

	ROM_REGION( 0x40000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "fa09.22a", 0x00000, 0x20000, CRC(1395e9be) SHA1(60693ac6236ffe1e0933d81771cfad32e14514c3) ) // == FB counterpart from World set
	ROM_LOAD( "fa08.21a", 0x20000, 0x20000, CRC(4d7464db) SHA1(82e2a3c3d78447985968220d52c7c1f1ff625d83) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "fa01.4a", 0x00000, 0x20000, CRC(99b0cd92) SHA1(2729e874730391b5fa93e9a28142c02c00eb5068) ) // == FB counterpart from World set
	ROM_LOAD( "fa03.7a", 0x20000, 0x20000, CRC(0e7ea74d) SHA1(22078a2856933af2d31750a4a506b993fe309e9a) ) // == FB counterpart from World set
	ROM_LOAD( "fa00.2a", 0x40000, 0x20000, CRC(f7df3fd7) SHA1(ed9e4649e0b1fcca61cf4d159b3f8a35f06102ce) ) // == FB counterpart from World set
	ROM_LOAD( "fa02.5a", 0x60000, 0x20000, CRC(84e8da9d) SHA1(41da6042f80ea3562aa350f4f466b16db29e2aca) ) // == FB counterpart from World set

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "fa11.11k", 0x00000, 0x20000, CRC(4e547bad) SHA1(655eda4d00f8846957ed40dcbf750fba3ce19f4e) ) // == FB counterpart from World set

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7114h.21k", 0x0000, 0x0100, CRC(ad26e8d4) SHA1(827337aeb8904429a1c050279240ae38aa6ce064) )    /* Priority (not used) */

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8b-ta-1.15k", 0x0000, 0x0104, CRC(79a87527) SHA1(3c6ad20d5a7c41c020e671d462c0b1e4a5dda7f8) )
	ROM_LOAD( "pal16r4a-ta-2.16k", 0x0200, 0x0104, CRC(eca31311) SHA1(a87b2721e13767f7448236d0bbb3355583fe88bb) )
	ROM_LOAD( "pal16l8a-ta-3.17k", 0x0400, 0x0104, CRC(6c324919) SHA1(83bba4634d7ab7c4ad3083c063804fd1e7c9c10b) )
	ROM_LOAD( "pal16l8a-ta-4.11m", 0x0600, 0x0104, CRC(116177fa) SHA1(f63802578b6f743e2d3a64e4805488d44361dcb2) )
ROM_END

ROM_START( slyspy2 ) /* DE-0322-3 PCB */
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "fa14-2.17l", 0x00000, 0x10000, CRC(0e431e39) SHA1(ab4774966ad113e4d7004d14bfd72330d4a93a43) )
	ROM_LOAD16_BYTE( "fa12-2.9l",  0x00001, 0x10000, CRC(1b534294) SHA1(cf7badea6604c47d9f3ff8a0ef326e09de1974a0) )
	ROM_LOAD16_BYTE( "fa15.19l",   0x20000, 0x10000, CRC(04a79266) SHA1(69d256ffb1c89721f8b1e929c581f187e047b977) )
	ROM_LOAD16_BYTE( "fa13.11l",   0x20001, 0x10000, CRC(641cc4b3) SHA1(ce0ccd14d201f411cfc02ec988b2ad4fcb0d8f5d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "fa10.5h", 0x00000, 0x10000, CRC(dfd2ff25) SHA1(3dcd6d50b92b49daae4b51581abe9c95f764e848) ) // == FB counterpart from World set

	ROM_REGION( 0x10000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "fa05.11a", 0x04000, 0x04000, CRC(09802924) SHA1(d9bc5fe7f053afa15cd39400aae993866d1b0226) ) // == FB counterpart from World set
	ROM_CONTINUE(         0x00000, 0x04000 )    /* the two halves are swapped */
	ROM_LOAD( "fa04.9a",  0x0c000, 0x04000, CRC(ec25b895) SHA1(8c1d2b9a2487fd7114d37fe9dc271183c4cc1613) ) // == FB counterpart from World set
	ROM_CONTINUE(         0x08000, 0x04000 )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "fa07.17a", 0x00000, 0x10000, CRC(e932268b) SHA1(ee8ed29affa951e725cf19a5f56d3beac24420c9) ) // == FB counterpart from World set
	ROM_LOAD( "fa06.15a", 0x10000, 0x10000, CRC(c4dd38c0) SHA1(267dbbdd5df6b13662cd307c5c95fdf643d64f45) ) // == FB counterpart from World set

	ROM_REGION( 0x40000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "fa09.22a", 0x00000, 0x20000, CRC(1395e9be) SHA1(60693ac6236ffe1e0933d81771cfad32e14514c3) ) // == FB counterpart from World set
	ROM_LOAD( "fa08.21a", 0x20000, 0x20000, CRC(4d7464db) SHA1(82e2a3c3d78447985968220d52c7c1f1ff625d83) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "fa01.4a", 0x00000, 0x20000, CRC(99b0cd92) SHA1(2729e874730391b5fa93e9a28142c02c00eb5068) ) // == FB counterpart from World set
	ROM_LOAD( "fa03.7a", 0x20000, 0x20000, CRC(0e7ea74d) SHA1(22078a2856933af2d31750a4a506b993fe309e9a) ) // == FB counterpart from World set
	ROM_LOAD( "fa00.2a", 0x40000, 0x20000, CRC(f7df3fd7) SHA1(ed9e4649e0b1fcca61cf4d159b3f8a35f06102ce) ) // == FB counterpart from World set
	ROM_LOAD( "fa02.5a", 0x60000, 0x20000, CRC(84e8da9d) SHA1(41da6042f80ea3562aa350f4f466b16db29e2aca) ) // == FB counterpart from World set

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "fa11.11k", 0x00000, 0x20000, CRC(4e547bad) SHA1(655eda4d00f8846957ed40dcbf750fba3ce19f4e) ) // == FB counterpart from World set

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7114h.21k", 0x0000, 0x0100, CRC(ad26e8d4) SHA1(827337aeb8904429a1c050279240ae38aa6ce064) )    /* Priority (not used) */

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8b-ta-1.15k", 0x0000, 0x0104, CRC(79a87527) SHA1(3c6ad20d5a7c41c020e671d462c0b1e4a5dda7f8) )
	ROM_LOAD( "pal16r4a-ta-2.16k", 0x0200, 0x0104, CRC(eca31311) SHA1(a87b2721e13767f7448236d0bbb3355583fe88bb) )
	ROM_LOAD( "pal16l8a-ta-3.17k", 0x0400, 0x0104, CRC(6c324919) SHA1(83bba4634d7ab7c4ad3083c063804fd1e7c9c10b) )
	ROM_LOAD( "pal16l8a-ta-4.11m", 0x0600, 0x0104, CRC(116177fa) SHA1(f63802578b6f743e2d3a64e4805488d44361dcb2) )
ROM_END

/*

secret agent - deco (clone)

(snd)
1 x z80
1 x ym2203c
1 x ym3812
sa_01 and sa_02

(prg)
1 x 68000
from sa_03 to sa_06

(gfx)
from sa_07 to sa_14
from sa_15 to sa_22

[dump.it]

*/

ROM_START( secretab )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "sa_05.ic94",   0x00000, 0x10000, CRC(54869474) SHA1(88c1894d1b6d8dd3d37e97d566aafef9c9409d6e) ) // misnumbered; should be IC84
	ROM_LOAD16_BYTE( "sa_03.ic67",   0x00001, 0x10000, CRC(36ab1874) SHA1(baa47c466ab13ac792761531f77ee8e639d19203) )
	ROM_LOAD16_BYTE( "sa_06.ic83",   0x20000, 0x10000, CRC(8e691f23) SHA1(eb08c9539b699af124fcf87be07a33d2d5a71ada) )
	ROM_LOAD16_BYTE( "sa_04.ic66",   0x20001, 0x10000, CRC(c838b205) SHA1(8c7a453ec7a00d4f5bbf9fadba6d551909647ed8) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "sa_01.ic41",     0x00000, 0x10000, CRC(9fdc503b) SHA1(7b258e0734ca88a7d3f574d75116f0fe3b628898) )
	ROM_LOAD( "sa_02.ic40",     0x10000, 0x10000, CRC(439eb5a9) SHA1(8d6baad8a1e89279ef0a378941d3d9b49a606864) ) // both halves identical

	ROM_REGION( 0x40000, "charset", 0 )
	ROM_LOAD( "sa_08.ic105", 0x00000, 0x10000, CRC(4806b951) SHA1(a2fa5b8587132747067d7d64ccfd14129a34ef58) )
	ROM_LOAD( "sa_12.ic156", 0x10000, 0x10000, CRC(f9e2cd5f) SHA1(f2c3f6e763c6f80307e9daee533d316b05cd02c5) )
	ROM_LOAD( "sa_10.ic138", 0x20000, 0x10000, CRC(843c4679) SHA1(871f3e77aa7e628e924a40d06ddec700487e23fb) )
	ROM_LOAD( "sa_14.ic188", 0x30000, 0x10000, CRC(3dac9128) SHA1(f3a2068e90973c1f04f1bbaa209111e3f9669ee0) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_INVERT ) /* chars */
	ROM_COPY( "charset", 0x00000, 0x00000, 0x8000 )
	ROM_COPY( "charset", 0x10000, 0x08000, 0x8000 )
	ROM_COPY( "charset", 0x20000, 0x10000, 0x8000 )
	ROM_COPY( "charset", 0x30000, 0x18000, 0x8000 )

	ROM_REGION( 0x20000, "gfx2", ROMREGION_INVERT ) /* tiles */
	ROM_COPY( "charset", 0x08000, 0x00000, 0x8000 )
	ROM_COPY( "charset", 0x18000, 0x08000, 0x8000 )
	ROM_COPY( "charset", 0x28000, 0x10000, 0x8000 )
	ROM_COPY( "charset", 0x38000, 0x18000, 0x8000 )

	ROM_REGION( 0x40000, "gfx3", ROMREGION_INVERT ) /* tiles */
	ROM_LOAD( "sa_09.ic139",     0x00000, 0x10000, CRC(9e412267) SHA1(482cd6e772fa21f15db66c27acf85e8f97f7c5a5) )
	ROM_LOAD( "sa_11.ic157",     0x10000, 0x10000, CRC(e87650db) SHA1(381352428b12fd4a8cd13270009ff7602aa41a0b) )
	ROM_LOAD( "sa_07.ic106",     0x20000, 0x10000, CRC(6ad2e575) SHA1(b6b159cb36e222fe62fc10271602226f027440e4) )
	ROM_LOAD( "sa_13.ic189",     0x30000, 0x10000, CRC(e8601057) SHA1(fd73a36fb84049154248d250ffea68b1ee39a43f) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "sa_20.ic176",     0x00000, 0x10000, CRC(447e4f0b) SHA1(97db103e505a6e11eb9bdb3622e4aa3b796a9714) )
	ROM_LOAD( "sa_19.ic177",     0x10000, 0x10000, CRC(d29bc22e) SHA1(ce0935d09f7e94fa32247c86e14a74b73514b29e) )
	ROM_LOAD( "sa_16.ic180",     0x20000, 0x10000, CRC(ff72b838) SHA1(fdc48ecdd2225fc69472313f34973f6add8fb558) )
	ROM_LOAD( "sa_15.ic181",     0x30000, 0x10000, CRC(54fcbc39) SHA1(293a6799193b01424c3eac86cf90cc023aa771db) )
	ROM_LOAD( "sa_22.ic174",     0x40000, 0x10000, CRC(d234cae5) SHA1(0cd07bf087a4da19a5da29785385de9eee52d0fb) )
	ROM_LOAD( "sa_21.ic175",     0x50000, 0x10000, CRC(dc6a38df) SHA1(9043df911389d3f085299f2f2202cab356473a32) )
	ROM_LOAD( "sa_18.ic178",     0x60000, 0x10000, CRC(4f989f00) SHA1(ae7ae6e62e6a516ae3c8ebbeb5e39887c1961add) )
	ROM_LOAD( "sa_17.ic179",     0x70000, 0x10000, CRC(f61972c8) SHA1(fa9ddca3473091b4879171d8f3b302e8f2b45149) )
ROM_END


ROM_START( mastbond ) // same as secretab, but for the charset ROMs. Just a hack to change the title to Master Bond. PCB marked 19 89 N. All ROMs are 27C512.
	ROM_REGION( 0x60000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "5.ic84",   0x00000, 0x10000, CRC(54869474) SHA1(88c1894d1b6d8dd3d37e97d566aafef9c9409d6e) )
	ROM_LOAD16_BYTE( "3.ic67",   0x00001, 0x10000, CRC(36ab1874) SHA1(baa47c466ab13ac792761531f77ee8e639d19203) )
	ROM_LOAD16_BYTE( "6.ic83",   0x20000, 0x10000, CRC(8e691f23) SHA1(eb08c9539b699af124fcf87be07a33d2d5a71ada) )
	ROM_LOAD16_BYTE( "4.ic66",   0x20001, 0x10000, CRC(c838b205) SHA1(8c7a453ec7a00d4f5bbf9fadba6d551909647ed8) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "1.ic41",     0x00000, 0x10000, CRC(9fdc503b) SHA1(7b258e0734ca88a7d3f574d75116f0fe3b628898) )
	ROM_LOAD( "2.ic40",     0x10000, 0x10000, CRC(439eb5a9) SHA1(8d6baad8a1e89279ef0a378941d3d9b49a606864) ) // both halves identical

	ROM_REGION( 0x40000, "charset", 0 )
	ROM_LOAD( "8.ic105",  0x00000, 0x10000, CRC(3cfc2960) SHA1(495aad53d00cf569094a5d8084a829d0647ba9dd) )
	ROM_LOAD( "12.ic156", 0x10000, 0x10000, CRC(62476ba2) SHA1(5a025d11502f35896a40e33d7a487ed4c933135b) )
	ROM_LOAD( "10.ic138", 0x20000, 0x10000, CRC(16df8be2) SHA1(3d5e63933dc151caca56536dce67407a8bacb761) )
	ROM_LOAD( "14.ic188", 0x30000, 0x10000, CRC(f1803c03) SHA1(d6504db6d98d838025b6ba68d5b5b5e3999b1c13) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_INVERT ) // chars
	ROM_COPY( "charset", 0x00000, 0x00000, 0x8000 )
	ROM_COPY( "charset", 0x10000, 0x08000, 0x8000 )
	ROM_COPY( "charset", 0x20000, 0x10000, 0x8000 )
	ROM_COPY( "charset", 0x30000, 0x18000, 0x8000 )

	ROM_REGION( 0x20000, "gfx2", ROMREGION_INVERT ) // tiles
	ROM_COPY( "charset", 0x08000, 0x00000, 0x8000 )
	ROM_COPY( "charset", 0x18000, 0x08000, 0x8000 )
	ROM_COPY( "charset", 0x28000, 0x10000, 0x8000 )
	ROM_COPY( "charset", 0x38000, 0x18000, 0x8000 )

	ROM_REGION( 0x40000, "gfx3", ROMREGION_INVERT ) // tiles
	ROM_LOAD( "9.ic139",     0x00000, 0x10000, CRC(9e412267) SHA1(482cd6e772fa21f15db66c27acf85e8f97f7c5a5) )
	ROM_LOAD( "11.ic157",    0x10000, 0x10000, CRC(e87650db) SHA1(381352428b12fd4a8cd13270009ff7602aa41a0b) )
	ROM_LOAD( "7.ic106",     0x20000, 0x10000, CRC(6ad2e575) SHA1(b6b159cb36e222fe62fc10271602226f027440e4) )
	ROM_LOAD( "13.ic189",    0x30000, 0x10000, CRC(e8601057) SHA1(fd73a36fb84049154248d250ffea68b1ee39a43f) )

	ROM_REGION( 0x80000, "gfx4", 0 ) // sprites
	ROM_LOAD( "20.ic176",     0x00000, 0x10000, CRC(447e4f0b) SHA1(97db103e505a6e11eb9bdb3622e4aa3b796a9714) )
	ROM_LOAD( "19.ic177",     0x10000, 0x10000, CRC(d29bc22e) SHA1(ce0935d09f7e94fa32247c86e14a74b73514b29e) )
	ROM_LOAD( "16.ic180",     0x20000, 0x10000, CRC(ff72b838) SHA1(fdc48ecdd2225fc69472313f34973f6add8fb558) )
	ROM_LOAD( "15.ic181",     0x30000, 0x10000, CRC(54fcbc39) SHA1(293a6799193b01424c3eac86cf90cc023aa771db) )
	ROM_LOAD( "22.ic174",     0x40000, 0x10000, CRC(d234cae5) SHA1(0cd07bf087a4da19a5da29785385de9eee52d0fb) )
	ROM_LOAD( "21.ic175",     0x50000, 0x10000, CRC(dc6a38df) SHA1(9043df911389d3f085299f2f2202cab356473a32) )
	ROM_LOAD( "18.ic178",     0x60000, 0x10000, CRC(4f989f00) SHA1(ae7ae6e62e6a516ae3c8ebbeb5e39887c1961add) )
	ROM_LOAD( "17.ic179",     0x70000, 0x10000, CRC(f61972c8) SHA1(fa9ddca3473091b4879171d8f3b302e8f2b45149) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "gal16v8.ic43",  0x0000, 0x0117, CRC(01cdc0bf) SHA1(df47ba4b3d0cf1b3acef2c4a7ba3bd1433aa9bf3) ) // brute-forced
	ROM_LOAD( "gal16v8.ic48",  0x0200, 0x0117, CRC(cfb99386) SHA1(62f6befd34de85bbc76f3f115593ec72c7474303) ) // brute-forced
	ROM_LOAD( "gal16v8.ic141", 0x0400, 0x0117, NO_DUMP ) // registered
ROM_END


ROM_START( midres )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "fk_14.rom",    0x00000, 0x20000, CRC(de7522df) SHA1(b627a4bf2f2308ff16e55a9e49ba4eb9bd637d90) )
	ROM_LOAD16_BYTE( "fk_12.rom",    0x00001, 0x20000, CRC(3494b8c9) SHA1(6cb3f1421fe71d329c65c0a9056bcfae7229a37b) )
	ROM_LOAD16_BYTE( "fl15",         0x40000, 0x20000, CRC(1328354e) SHA1(2780a524718f351350e0fbc92a9a7ce9bdfc315e) )
	ROM_LOAD16_BYTE( "fl13",         0x40001, 0x20000, CRC(e3b3955e) SHA1(10ff430b14c1dbcce81b13251bac124ef4f9f1d9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "fl16",              0x00000, 0x10000, CRC(66360bdf) SHA1(76ecaeb396118bb2fe6c0151bb0705a3a878f7a5) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "fk_05.rom",         0x08000, 0x08000, CRC(3cdb7453) SHA1(d4b7fbf4726a375b4478922db6d936274bfa963c) )
	ROM_CONTINUE(                  0x00000, 0x08000 )   /* the two halves are swapped */
	ROM_LOAD( "fk_04.rom",         0x18000, 0x08000, CRC(325ba20c) SHA1(fecd6254cf8c3b18496039fe18ded13c2ae47ff4) )
	ROM_CONTINUE(                  0x10000, 0x08000 )

	ROM_REGION( 0x80000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "fl09",              0x00000, 0x20000, CRC(907d5910) SHA1(6f4963724987bf44007988d117a1f7276cf270d8) )
	ROM_LOAD( "fl08",              0x20000, 0x20000, CRC(a936c03c) SHA1(293e69874ce9b2dfb1d605c9f988fa736b12bbcf) )
	ROM_LOAD( "fl07",              0x40000, 0x20000, CRC(2068c45c) SHA1(943ed767a462ee39a42cd15f02d06c8a2e4556b3) )
	ROM_LOAD( "fl06",              0x60000, 0x20000, CRC(b7241ab9) SHA1(3e83f9285ff4c476f1287bf73b514eace482dccc) )

	ROM_REGION( 0x40000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "fl11",              0x00000, 0x20000, CRC(b86b73b4) SHA1(dd0e61d60574e537aa1b7f35ffdfd08434ec8208) )
	ROM_LOAD( "fl10",              0x20000, 0x20000, CRC(92245b29) SHA1(3289842bbd4bd7858846b234f08ea5737c11536d) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "fl01",              0x00000, 0x20000, CRC(2c8b35a7) SHA1(9ab1c2f014a24837ee99c4db000291f7e55aeb12) )
	ROM_LOAD( "fl03",              0x20000, 0x20000, CRC(1eefed3c) SHA1(be0ce3db211587086ae3ee8df85b7c56f831c623) )
	ROM_LOAD( "fl00",              0x40000, 0x20000, CRC(756fb801) SHA1(35510c4ddf9258d87fdee0d3a64a8de0ebd1967d) )
	ROM_LOAD( "fl02",              0x60000, 0x20000, CRC(54d2c120) SHA1(84f93bcd41d5bda8cfb39c4947fff025f53b143d) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "fl17",              0x00000, 0x20000, CRC(9029965d) SHA1(9b28dc38e86f24fa89d7971b141c9bdddc662c99) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "7114.prm",          0x0000, 0x0100, CRC(eb539ffb) SHA1(6a8c9112f289f63e8c88320c9df698b559632c3d) )   /* Priority (not used) */

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "pal16r4a-1.bin", 0x0000, 0x0104, CRC(d28fb8e0) SHA1(73cd73a075bd3ba3b3e50f3b71a4aaecce37115f) )
	ROM_LOAD( "pal16l8b-2.bin", 0x0200, 0x0104, CRC(bcb591e3) SHA1(d3ebc2a19108c9db355d3ba1512ab4cf0d9fad76) )
	ROM_LOAD( "pal16l8a-3.bin", 0x0400, 0x0104, CRC(e12972ac) SHA1(6b178c936068d9017a1444f437aea7e2ab1c6ca9) )
	ROM_LOAD( "pal16l8a-4.bin", 0x0600, 0x0104, CRC(c6437e49) SHA1(0d89855378ab5f45d55f6aa175a63458b3da52a3) )
	ROM_LOAD( "pal16l8b-5.bin", 0x0800, 0x0104, CRC(e9ee3a67) SHA1(5299f44f1141fcd57b0559b91ec7adb51b36c5c4) )
	ROM_LOAD( "pal16l8a-6.bin", 0x0a00, 0x0104, CRC(23b17abe) SHA1(ca6c47f4df63d84401ccb29d0a0e3633b09d708a) )
ROM_END

ROM_START( midres2 ) // DE-0323-4 PCB, only the first 2 main CPU ROMs differ, ROM labels weren't original so unfortunately not possible to determine version
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "mr14",         0x00000, 0x20000, CRC(ad4617a9) SHA1(cca7cf9d21050fc187a6279ac008b4bd83baf5ac) )
	ROM_LOAD16_BYTE( "mr12",         0x00001, 0x20000, CRC(c9ed677b) SHA1(7efcde8a1cf84910e9d946648a419fb9f7fd30ac) )
	ROM_LOAD16_BYTE( "mr15",         0x40000, 0x20000, CRC(1328354e) SHA1(2780a524718f351350e0fbc92a9a7ce9bdfc315e) )
	ROM_LOAD16_BYTE( "mr13",         0x40001, 0x20000, CRC(e3b3955e) SHA1(10ff430b14c1dbcce81b13251bac124ef4f9f1d9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "mr16",              0x00000, 0x10000, CRC(66360bdf) SHA1(76ecaeb396118bb2fe6c0151bb0705a3a878f7a5) )

	ROM_REGION( 0x20000, "gfx1", 0 ) // chars
	ROM_LOAD( "mr05",              0x08000, 0x08000, CRC(d75aba06) SHA1(cb3b969db3dd8e0c5c3729482f7461cde3a961f3) )
	ROM_CONTINUE(                  0x00000, 0x08000 )   // the two halves are swapped
	ROM_LOAD( "mr04",              0x18000, 0x08000, CRC(8f5bbb79) SHA1(cb10f68787606111ba5e9967bf0b0cd21269a902) )
	ROM_CONTINUE(                  0x10000, 0x08000 )

	ROM_REGION( 0x80000, "gfx2", 0 ) // tiles
	ROM_LOAD( "fl09",              0x00000, 0x20000, CRC(907d5910) SHA1(6f4963724987bf44007988d117a1f7276cf270d8) )
	ROM_LOAD( "fl08",              0x20000, 0x20000, CRC(a936c03c) SHA1(293e69874ce9b2dfb1d605c9f988fa736b12bbcf) )
	ROM_LOAD( "fl07",              0x40000, 0x20000, CRC(2068c45c) SHA1(943ed767a462ee39a42cd15f02d06c8a2e4556b3) )
	ROM_LOAD( "fl06",              0x60000, 0x20000, CRC(b7241ab9) SHA1(3e83f9285ff4c476f1287bf73b514eace482dccc) )

	ROM_REGION( 0x40000, "gfx3", 0 ) // tiles
	ROM_LOAD( "fl11",              0x00000, 0x20000, CRC(b86b73b4) SHA1(dd0e61d60574e537aa1b7f35ffdfd08434ec8208) )
	ROM_LOAD( "fl10",              0x20000, 0x20000, CRC(92245b29) SHA1(3289842bbd4bd7858846b234f08ea5737c11536d) )

	ROM_REGION( 0x80000, "gfx4", 0 ) // sprites
	ROM_LOAD( "fl01",              0x00000, 0x20000, CRC(2c8b35a7) SHA1(9ab1c2f014a24837ee99c4db000291f7e55aeb12) )
	ROM_LOAD( "fl03",              0x20000, 0x20000, CRC(1eefed3c) SHA1(be0ce3db211587086ae3ee8df85b7c56f831c623) )
	ROM_LOAD( "fl00",              0x40000, 0x20000, CRC(756fb801) SHA1(35510c4ddf9258d87fdee0d3a64a8de0ebd1967d) )
	ROM_LOAD( "fl02",              0x60000, 0x20000, CRC(54d2c120) SHA1(84f93bcd41d5bda8cfb39c4947fff025f53b143d) )

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "mr17",              0x00000, 0x20000, CRC(9029965d) SHA1(9b28dc38e86f24fa89d7971b141c9bdddc662c99) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "7114.prm",          0x0000, 0x0100, CRC(eb539ffb) SHA1(6a8c9112f289f63e8c88320c9df698b559632c3d) )   // Priority (not used)

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "pal16r4a-1.bin", 0x0000, 0x0104, CRC(d28fb8e0) SHA1(73cd73a075bd3ba3b3e50f3b71a4aaecce37115f) )
	ROM_LOAD( "pal16l8b-2.bin", 0x0200, 0x0104, CRC(bcb591e3) SHA1(d3ebc2a19108c9db355d3ba1512ab4cf0d9fad76) )
	ROM_LOAD( "pal16l8a-3.bin", 0x0400, 0x0104, CRC(e12972ac) SHA1(6b178c936068d9017a1444f437aea7e2ab1c6ca9) )
	ROM_LOAD( "pal16l8a-4.bin", 0x0600, 0x0104, CRC(c6437e49) SHA1(0d89855378ab5f45d55f6aa175a63458b3da52a3) )
	ROM_LOAD( "pal16l8b-5.bin", 0x0800, 0x0104, CRC(e9ee3a67) SHA1(5299f44f1141fcd57b0559b91ec7adb51b36c5c4) )
	ROM_LOAD( "pal16l8a-6.bin", 0x0a00, 0x0104, CRC(23b17abe) SHA1(ca6c47f4df63d84401ccb29d0a0e3633b09d708a) )
ROM_END

ROM_START( midresu )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "fl14",         0x00000, 0x20000, CRC(2f9507a2) SHA1(bfa3449c2f8d706ec9eebb41c0f089229cd30537) )
	ROM_LOAD16_BYTE( "fl12",         0x00001, 0x20000, CRC(3815ad9f) SHA1(04b05ca68a2526ef6b16a1bbbf91c36300070c6c) )
	ROM_LOAD16_BYTE( "fl15",         0x40000, 0x20000, CRC(1328354e) SHA1(2780a524718f351350e0fbc92a9a7ce9bdfc315e) )
	ROM_LOAD16_BYTE( "fl13",         0x40001, 0x20000, CRC(e3b3955e) SHA1(10ff430b14c1dbcce81b13251bac124ef4f9f1d9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "fl16",              0x00000, 0x10000, CRC(66360bdf) SHA1(76ecaeb396118bb2fe6c0151bb0705a3a878f7a5) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "fl05",              0x08000, 0x08000, CRC(d75aba06) SHA1(cb3b969db3dd8e0c5c3729482f7461cde3a961f3) )
	ROM_CONTINUE(                  0x00000, 0x08000 )   /* the two halves are swapped */
	ROM_LOAD( "fl04",              0x18000, 0x08000, CRC(8f5bbb79) SHA1(cb10f68787606111ba5e9967bf0b0cd21269a902) )
	ROM_CONTINUE(                  0x10000, 0x08000 )

	ROM_REGION( 0x80000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "fl09",              0x00000, 0x20000, CRC(907d5910) SHA1(6f4963724987bf44007988d117a1f7276cf270d8) )
	ROM_LOAD( "fl08",              0x20000, 0x20000, CRC(a936c03c) SHA1(293e69874ce9b2dfb1d605c9f988fa736b12bbcf) )
	ROM_LOAD( "fl07",              0x40000, 0x20000, CRC(2068c45c) SHA1(943ed767a462ee39a42cd15f02d06c8a2e4556b3) )
	ROM_LOAD( "fl06",              0x60000, 0x20000, CRC(b7241ab9) SHA1(3e83f9285ff4c476f1287bf73b514eace482dccc) )

	ROM_REGION( 0x40000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "fl11",              0x00000, 0x20000, CRC(b86b73b4) SHA1(dd0e61d60574e537aa1b7f35ffdfd08434ec8208) )
	ROM_LOAD( "fl10",              0x20000, 0x20000, CRC(92245b29) SHA1(3289842bbd4bd7858846b234f08ea5737c11536d) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "fl01",              0x00000, 0x20000, CRC(2c8b35a7) SHA1(9ab1c2f014a24837ee99c4db000291f7e55aeb12) )
	ROM_LOAD( "fl03",              0x20000, 0x20000, CRC(1eefed3c) SHA1(be0ce3db211587086ae3ee8df85b7c56f831c623) )
	ROM_LOAD( "fl00",              0x40000, 0x20000, CRC(756fb801) SHA1(35510c4ddf9258d87fdee0d3a64a8de0ebd1967d) )
	ROM_LOAD( "fl02",              0x60000, 0x20000, CRC(54d2c120) SHA1(84f93bcd41d5bda8cfb39c4947fff025f53b143d) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "fl17",              0x00000, 0x20000, CRC(9029965d) SHA1(9b28dc38e86f24fa89d7971b141c9bdddc662c99) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "7114.prm",          0x0000, 0x0100, CRC(eb539ffb) SHA1(6a8c9112f289f63e8c88320c9df698b559632c3d) )   /* Priority (not used) */

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "pal16r4a-1.bin", 0x0000, 0x0104, CRC(d28fb8e0) SHA1(73cd73a075bd3ba3b3e50f3b71a4aaecce37115f) )
	ROM_LOAD( "pal16l8b-2.bin", 0x0200, 0x0104, CRC(bcb591e3) SHA1(d3ebc2a19108c9db355d3ba1512ab4cf0d9fad76) )
	ROM_LOAD( "pal16l8a-3.bin", 0x0400, 0x0104, CRC(e12972ac) SHA1(6b178c936068d9017a1444f437aea7e2ab1c6ca9) )
	ROM_LOAD( "pal16l8a-4.bin", 0x0600, 0x0104, CRC(c6437e49) SHA1(0d89855378ab5f45d55f6aa175a63458b3da52a3) )
	ROM_LOAD( "pal16l8b-5.bin", 0x0800, 0x0104, CRC(e9ee3a67) SHA1(5299f44f1141fcd57b0559b91ec7adb51b36c5c4) )
	ROM_LOAD( "pal16l8a-6.bin", 0x0a00, 0x0104, CRC(23b17abe) SHA1(ca6c47f4df63d84401ccb29d0a0e3633b09d708a) )
ROM_END

ROM_START( midresj )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "fh14",         0x00000, 0x20000, CRC(6d632a51) SHA1(38f9e8fe01ec9105c1ec83d70a5f5b2c754865ca) )
	ROM_LOAD16_BYTE( "fh12",         0x00001, 0x20000, CRC(45143384) SHA1(5733439d6598a02dc0ae74b41d34b6afadd39330) )
	ROM_LOAD16_BYTE( "fl15",         0x40000, 0x20000, CRC(1328354e) SHA1(2780a524718f351350e0fbc92a9a7ce9bdfc315e) )
	ROM_LOAD16_BYTE( "fl13",         0x40001, 0x20000, CRC(e3b3955e) SHA1(10ff430b14c1dbcce81b13251bac124ef4f9f1d9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "fh16",              0x00000, 0x10000, CRC(00736f32) SHA1(292f98b5579314c866247dd0ea1346c6e160b304) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "fk_05.rom",         0x08000, 0x08000, CRC(3cdb7453) SHA1(d4b7fbf4726a375b4478922db6d936274bfa963c) )
	ROM_CONTINUE(                  0x00000, 0x08000 )   /* the two halves are swapped */
	ROM_LOAD( "fk_04.rom",         0x18000, 0x08000, CRC(325ba20c) SHA1(fecd6254cf8c3b18496039fe18ded13c2ae47ff4) )
	ROM_CONTINUE(                  0x10000, 0x08000 )

	ROM_REGION( 0x80000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "fl09",              0x00000, 0x20000, CRC(907d5910) SHA1(6f4963724987bf44007988d117a1f7276cf270d8) )
	ROM_LOAD( "fl08",              0x20000, 0x20000, CRC(a936c03c) SHA1(293e69874ce9b2dfb1d605c9f988fa736b12bbcf) )
	ROM_LOAD( "fl07",              0x40000, 0x20000, CRC(2068c45c) SHA1(943ed767a462ee39a42cd15f02d06c8a2e4556b3) )
	ROM_LOAD( "fl06",              0x60000, 0x20000, CRC(b7241ab9) SHA1(3e83f9285ff4c476f1287bf73b514eace482dccc) )

	ROM_REGION( 0x40000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "fl11",              0x00000, 0x20000, CRC(b86b73b4) SHA1(dd0e61d60574e537aa1b7f35ffdfd08434ec8208) )
	ROM_LOAD( "fl10",              0x20000, 0x20000, CRC(92245b29) SHA1(3289842bbd4bd7858846b234f08ea5737c11536d) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "fl01",              0x00000, 0x20000, CRC(2c8b35a7) SHA1(9ab1c2f014a24837ee99c4db000291f7e55aeb12) )
	ROM_LOAD( "fl03",              0x20000, 0x20000, CRC(1eefed3c) SHA1(be0ce3db211587086ae3ee8df85b7c56f831c623) )
	ROM_LOAD( "fl00",              0x40000, 0x20000, CRC(756fb801) SHA1(35510c4ddf9258d87fdee0d3a64a8de0ebd1967d) )
	ROM_LOAD( "fl02",              0x60000, 0x20000, CRC(54d2c120) SHA1(84f93bcd41d5bda8cfb39c4947fff025f53b143d) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "fh17",              0x00000, 0x20000, CRC(c7b0a24e) SHA1(8a068d7838bbdfb200c7104deb0cd5647336117a) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "7114.prm",          0x0000, 0x0100, CRC(eb539ffb) SHA1(6a8c9112f289f63e8c88320c9df698b559632c3d) )   /* Priority (not used) */

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "pal16r4a-1.bin", 0x0000, 0x0104, CRC(d28fb8e0) SHA1(73cd73a075bd3ba3b3e50f3b71a4aaecce37115f) )
	ROM_LOAD( "pal16l8b-2.bin", 0x0200, 0x0104, CRC(bcb591e3) SHA1(d3ebc2a19108c9db355d3ba1512ab4cf0d9fad76) )
	ROM_LOAD( "pal16l8a-3.bin", 0x0400, 0x0104, CRC(e12972ac) SHA1(6b178c936068d9017a1444f437aea7e2ab1c6ca9) )
	ROM_LOAD( "pal16l8a-4.bin", 0x0600, 0x0104, CRC(c6437e49) SHA1(0d89855378ab5f45d55f6aa175a63458b3da52a3) )
	ROM_LOAD( "pal16l8b-5.bin", 0x0800, 0x0104, CRC(e9ee3a67) SHA1(5299f44f1141fcd57b0559b91ec7adb51b36c5c4) )
	ROM_LOAD( "pal16l8a-6.bin", 0x0a00, 0x0104, CRC(23b17abe) SHA1(ca6c47f4df63d84401ccb29d0a0e3633b09d708a) )
ROM_END

/*
Midnight Resistance bootleg

Really nasty piece of junk, 2 huge boards.
There's also Bad Dudes running on the same
h/w (have seen one a few years ago)

basic components......
68000 @8MHz
6502
68705R3
YM2203
YM3812
M6295
XTALs 16MHz & 12MHz
2x 8 position DSWs

rom10 is missing (another PCB was found and dumped)
*/

ROM_START( midresb )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "14.bin",         0x00000, 0x10000, CRC(d9c0f06f) SHA1(23cdc3e0613fed4e3e35094884b716d2507d59c8) )
	ROM_LOAD16_BYTE( "13.bin",         0x20000, 0x10000, CRC(d1bb2cd6) SHA1(6d4afd8dd8c4c3e90de199358da27108286637e2) )
	ROM_LOAD16_BYTE( "11.bin",         0x00001, 0x10000, CRC(1909081a) SHA1(a8cfa551b55830f3cc32e52c9a855ca525e1ab3f) )
	ROM_LOAD16_BYTE( "10.bin",         0x20001, 0x10000, CRC(42ccdd0d) SHA1(ef17cc984a8d57e9c52877f4e9b78e9976f99033) )
	ROM_LOAD16_BYTE( "12.bin",         0x40000, 0x10000, CRC(1e85a68d) SHA1(9ff778d023523302f408d80f1cbd3a7c49c044b0) )
	ROM_LOAD16_BYTE( "9.bin",          0x40001, 0x10000, CRC(1587bc2a) SHA1(0ca2abccfc52b0071b0741e1498c34d765fe38da) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 sound */ // same as hippodrome / fighting fantasy...
	ROM_LOAD( "15.bin",         0x8000, 0x8000, CRC(9871b98d) SHA1(2b6c46bc2b10a28946d6ad8251e1a156a0b99947) )

	ROM_REGION( 0x1000, "mcu", 0 )    /* 68705 MCU */
	ROM_LOAD( "68705r3.bin",              0x00000, 0x1000, CRC(ad5b1c13) SHA1(3616dc5969323a54e3e171d169f76250ae4e711a) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "23.bin",             0x08000, 0x08000, CRC(d75aba06) SHA1(cb3b969db3dd8e0c5c3729482f7461cde3a961f3) )
	ROM_CONTINUE(                   0x00000, 0x08000 )  /* the two halves are swapped */
	ROM_LOAD( "24.bin",             0x18000, 0x08000, CRC(8f5bbb79) SHA1(cb10f68787606111ba5e9967bf0b0cd21269a902) )
	ROM_CONTINUE(                   0x10000, 0x08000 )

	ROM_REGION( 0x80000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "28.bin",             0x00000, 0x10000, CRC(4490ed48) SHA1(e825c6290c65b9e3fa38f961a2174836ec2324d9) )
	ROM_LOAD( "19.bin",             0x10000, 0x10000, CRC(0f94f5c1) SHA1(235f1d8a09c0bfc51454a16c41489eb45ea29e0b) )
	ROM_LOAD( "26.bin",             0x20000, 0x10000, CRC(d4994050) SHA1(1d78ad702325013c3fd0889622b969af76d749ee) )
	ROM_LOAD( "18.bin",             0x30000, 0x10000, CRC(dc85368b) SHA1(4c5b04de63e1b58d5d8615eb561fbb90d1e16011) )
	ROM_LOAD( "27.bin",             0x40000, 0x10000, CRC(06f7ac18) SHA1(402a2c05ef7bea5afaff417bc1a1e2ba24e52eaa) )
	ROM_LOAD( "20.bin",             0x50000, 0x10000, CRC(d2679020) SHA1(5ca10cd55e8a4aa670645528f6eff33fabf0c4d7) )
	ROM_LOAD( "25.bin",             0x60000, 0x10000, CRC(323cce90) SHA1(8f5c5d0cc2bc2ded75bb4d0683f2611585b5affc) )
	ROM_LOAD( "17.bin",             0x70000, 0x10000, CRC(7c94e1b4) SHA1(5b2d036f13c9ec85b46d601d8d925cfa14d204c3) )

	ROM_REGION( 0x40000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "22.bin",             0x00000, 0x10000, CRC(68d50336) SHA1(89a1b2398796ec2392f003e1c77ba914ea90c8c2) )
	ROM_LOAD( "30.bin",             0x10000, 0x10000, CRC(efe22953) SHA1(2f4b6090c2fcd45381746ccc14c8ad8948aa096b) )
	ROM_LOAD( "21.bin",             0x20000, 0x10000, CRC(3311d7b0) SHA1(d9812cd9d8b5bd38a78c4c3a92aa2a90d78525a3) )
	ROM_LOAD( "29.bin",             0x30000, 0x10000, CRC(9210b713) SHA1(1db2775359d946221b99047fb648114a908690a9) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "8.bin",             0x00000, 0x10000, CRC(3f499acb) SHA1(1a22cfeed0497ddc2d571114d9f246b3ae18ede9) )
	ROM_LOAD( "4.bin",             0x10000, 0x10000, CRC(5e7a6800) SHA1(8dd5c9005b6804a30627644053f14e4477fe0074) )
	ROM_LOAD( "6.bin",             0x20000, 0x10000, CRC(897ba6e4) SHA1(70fd9cba3922751cb317770d6effdc2fb94c1324) )
	ROM_LOAD( "2.bin",             0x30000, 0x10000, CRC(9fefb810) SHA1(863a81540261e78de5c612dea807ba29b12054d4) )
	ROM_LOAD( "7.bin",             0x40000, 0x10000, CRC(ebafe720) SHA1(b9f76d2f1b59f1d028e6156b831c5c8ada033641) )
	ROM_LOAD( "3.bin",             0x50000, 0x10000, CRC(87aab3c2) SHA1(fc5e96505f392b95a397e412f193f9aee74d58f5) )
	ROM_LOAD( "5.bin",             0x60000, 0x10000, CRC(fd0bd8d3) SHA1(d6b19869ddc2a8ed4f38ba9d613b71853f2d13c0) )
	ROM_LOAD( "1.bin",             0x70000, 0x10000, CRC(fc46d5ed) SHA1(20ddf3f67f0dfb222ad8d3fd464b892ec9c9e4f5) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "16.bin",            0x00000, 0x10000, CRC(ccf24b52) SHA1(39b2663c548b30684197284cb8e7a6ca803330c9) )
ROM_END

ROM_START( midresbj )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "14",         0x00000, 0x10000, CRC(6b3bc886) SHA1(998ef6ae89565148bcb8909f21acbec378ed5f4f) )
	ROM_LOAD16_BYTE( "11",         0x00001, 0x10000, CRC(9b6faab3) SHA1(b60e41972f52df910bfa09accd5fde7d858b55bf) )
	ROM_LOAD16_BYTE( "13",         0x20000, 0x10000, CRC(d1bb2cd6) SHA1(6d4afd8dd8c4c3e90de199358da27108286637e2) )
	ROM_LOAD16_BYTE( "10",         0x20001, 0x10000, CRC(42ccdd0d) SHA1(ef17cc984a8d57e9c52877f4e9b78e9976f99033) )
	ROM_LOAD16_BYTE( "12",         0x40000, 0x10000, CRC(258b10b2) SHA1(f0849801ab2c72bc6e929b230d0c6d41823f18ae) )
	ROM_LOAD16_BYTE( "9",          0x40001, 0x10000, CRC(dd6985d5) SHA1(bd58a1da2c5152464d7660f5b931d6257cb87c4e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 6502 sound */
	ROM_LOAD( "15",         0x0000, 0x10000, CRC(99d47166) SHA1(a9a1adfe47be8dd3e4d6f8c783447e09be1747b2) )

	ROM_REGION( 0x1000, "mcu", ROMREGION_ERASE00 )    /* 68705 MCU */
	//ROM_LOAD( "68705r3.bin",              0x00000, 0x1000, CRC(ad5b1c13) SHA1(3616dc5969323a54e3e171d169f76250ae4e711a) ) // unpopulated socket

	ROM_REGION( 0x20000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "23",             0x08000, 0x08000, CRC(d75aba06) SHA1(cb3b969db3dd8e0c5c3729482f7461cde3a961f3) )
	ROM_CONTINUE(                   0x00000, 0x08000 )  /* the two halves are swapped */
	ROM_LOAD( "24",             0x18000, 0x08000, CRC(8f5bbb79) SHA1(cb10f68787606111ba5e9967bf0b0cd21269a902) )
	ROM_CONTINUE(                   0x10000, 0x08000 )

	ROM_REGION( 0x80000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "19",             0x00000, 0x20000, CRC(fd9ba1bd) SHA1(a105a4335eeed19662c89ab0f90485f1029cf03f) )
	ROM_LOAD( "18",             0x20000, 0x20000, CRC(a936c03c) SHA1(293e69874ce9b2dfb1d605c9f988fa736b12bbcf) )
	ROM_LOAD( "20",             0x40000, 0x20000, CRC(4d8e3cf1) SHA1(db804a608f6ba9ce4cedfec2581bcbb00de3f2ba) )
	ROM_LOAD( "17",             0x60000, 0x20000, CRC(b7241ab9) SHA1(3e83f9285ff4c476f1287bf73b514eace482dccc) )

	ROM_REGION( 0x40000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "22",             0x10000, 0x10000, CRC(35a54bb4) SHA1(1869eb77a060e9df42b761b02e7fa5ecb7c414d1) )
	ROM_CONTINUE(0x00000,0x10000)
	ROM_LOAD( "21",             0x30000, 0x10000, CRC(4b9227b3) SHA1(7059f2d07fffa0468a45a42b87bf561da5e9c5a4) )
	ROM_CONTINUE(0x20000,0x10000)

	ROM_REGION( 0x80000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "4",             0x00000, 0x10000, CRC(3f499acb) SHA1(1a22cfeed0497ddc2d571114d9f246b3ae18ede9) )
	ROM_LOAD( "8",             0x10000, 0x10000, CRC(5e7a6800) SHA1(8dd5c9005b6804a30627644053f14e4477fe0074) )
	ROM_LOAD( "2",             0x20000, 0x10000, CRC(897ba6e4) SHA1(70fd9cba3922751cb317770d6effdc2fb94c1324) )
	ROM_LOAD( "6",             0x30000, 0x10000, CRC(9fefb810) SHA1(863a81540261e78de5c612dea807ba29b12054d4) )
	ROM_LOAD( "3",             0x40000, 0x10000, CRC(ebafe720) SHA1(b9f76d2f1b59f1d028e6156b831c5c8ada033641) )
	ROM_LOAD( "7",             0x50000, 0x10000, CRC(bb8cf641) SHA1(a22e47a15d38d4f33e5a2c90f3a90a16a4231d2c) ) // slight changes, check (equivalent to 3.bin in above)
	ROM_LOAD( "1",             0x60000, 0x10000, CRC(fd0bd8d3) SHA1(d6b19869ddc2a8ed4f38ba9d613b71853f2d13c0) )
	ROM_LOAD( "5",             0x70000, 0x10000, CRC(fc46d5ed) SHA1(20ddf3f67f0dfb222ad8d3fd464b892ec9c9e4f5) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "16",            0x00000, 0x10000, CRC(ccf24b52) SHA1(39b2663c548b30684197284cb8e7a6ca803330c9) )
ROM_END


ROM_START( bouldash )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "fw-15-2.17l",   0x00000, 0x10000, CRC(ca19a967) SHA1(b9dc2b1323f19b6239e550ed020943bf13de8db0) )
	ROM_LOAD16_BYTE( "fw-12-2.9l",    0x00001, 0x10000, CRC(242bdc2a) SHA1(9f2d7a5af94fae4ce4e61a2f3881a7aa10cdef3f) )
	ROM_LOAD16_BYTE( "fw-16-2.19l",   0x20000, 0x10000, CRC(b7217265) SHA1(7ffc71fffb82b1299c6d5d0d1e9e1550977d258a) )
	ROM_LOAD16_BYTE( "fw-13-2.11l",   0x20001, 0x10000, CRC(19209ef4) SHA1(36d7eb242f9558ee760b6cc69e7cf8f32d01070f) )
	ROM_LOAD16_BYTE( "fw-17-2.20l",   0x40000, 0x10000, CRC(78a632a1) SHA1(6b7b82bf59cca10ac5a71b910a218a09c5014ff6) )
	ROM_LOAD16_BYTE( "fw-14-2.13l",   0x40001, 0x10000, CRC(69b6112d) SHA1(3a8e34ae858946fc72b9ed4f932b9af64b081051) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "fn-10",      0x00000, 0x10000, CRC(c74106e7) SHA1(72213454c0ec78aa7d6843bd81d14b388ef7a48f) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "fn-04",        0x08000, 0x08000, CRC(40f5a760) SHA1(0d08b816714c08d0848dd25882a09d0a57fcc71b) )
	ROM_CONTINUE(             0x00000, 0x08000 )    /* the two halves are swapped */
	ROM_LOAD( "fn-05",        0x18000, 0x08000, CRC(824f2168) SHA1(32272a35e5faeebe41ece91fb902251707c9114b) )
	ROM_CONTINUE(             0x10000, 0x08000 )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "fn-07",        0x00000, 0x10000, CRC(eac6a3b3) SHA1(359df7335d11134ae149675080169cb53cafc19c) )
	ROM_LOAD( "fn-06",        0x10000, 0x10000, CRC(3feee292) SHA1(d0dc75afffff268e0b3b817fbc060d52418a2ca7) )

	ROM_REGION( 0x40000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "fn-09",        0x00000, 0x20000, CRC(c2b27bd2) SHA1(8452d4442af476a35d3dfc4bd4df0a7d84a3dd7c) )
	ROM_LOAD( "fn-08",        0x20000, 0x20000, CRC(5ac97178) SHA1(7d246cb17986033ae2c219f7409e3b91be0dd259) )

	ROM_REGION( 0x40000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "fn-01",        0x00000, 0x10000, CRC(9333121b) SHA1(826ed261b1af41dd5468b2244767593d48577618) )
	ROM_LOAD( "fn-03",        0x10000, 0x10000, CRC(254ba60f) SHA1(71ab5cd48ee34da1d2dd951bb243a26d7a1171ae) )
	ROM_LOAD( "fn-00",        0x20000, 0x10000, CRC(ec18d098) SHA1(3cd1a27de295a177e81c14b9e9bbfcf5793aade2) )
	ROM_LOAD( "fn-02",        0x30000, 0x10000, CRC(4f060cba) SHA1(4063183e699bb8b6059d56f4e2fec5fa0b037c23) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "fn-11",      0x00000, 0x10000, CRC(990fd8d9) SHA1(a37bd96ecd75c610d98df3320f53ae4e2b7fdefd) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "ta-16.21k",          0x0000, 0x0100, CRC(ad26e8d4) SHA1(827337aeb8904429a1c050279240ae38aa6ce064) )  /* Priority (not used) */
ROM_END

ROM_START( bouldashj )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "fn-15-1.17l",  0x00000, 0x10000, CRC(d3ef20f8) SHA1(87a32a3865bec086afee5d97c0691087a41f4870) )
	ROM_LOAD16_BYTE( "fn-12-1.9l",   0x00001, 0x10000, CRC(f4a10b45) SHA1(12c42d8abc7b21fbdef4f02d588a800cef222754) )
	ROM_LOAD16_BYTE( "fn-16-.19l",   0x20000, 0x10000, CRC(fd1806a5) SHA1(fdbc8e8048d0935ee69b2b8023b5afdfe6fd9095) )
	ROM_LOAD16_BYTE( "fn-13-.11l",   0x20001, 0x10000, CRC(d24d3681) SHA1(3f822592f7db4ba10852a57ea03fbc84271d2f77) )
	ROM_LOAD16_BYTE( "fn-17-.20l",   0x40000, 0x10000, CRC(28d48a37) SHA1(7c5ddc35e7b29e5f89073ba88cd4048699f57e55) )
	ROM_LOAD16_BYTE( "fn-14-.13l",   0x40001, 0x10000, CRC(8c61c682) SHA1(4ff2b5fc61b7887775901c968c872a2853ea6dbc) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "fn-10",      0x00000, 0x10000, CRC(c74106e7) SHA1(72213454c0ec78aa7d6843bd81d14b388ef7a48f) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "fn-04",        0x08000, 0x08000, CRC(40f5a760) SHA1(0d08b816714c08d0848dd25882a09d0a57fcc71b) )
	ROM_CONTINUE(             0x00000, 0x08000 )    /* the two halves are swapped */
	ROM_LOAD( "fn-05",        0x18000, 0x08000, CRC(824f2168) SHA1(32272a35e5faeebe41ece91fb902251707c9114b) )
	ROM_CONTINUE(             0x10000, 0x08000 )

	ROM_REGION( 0x20000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "fn-07",        0x00000, 0x10000, CRC(eac6a3b3) SHA1(359df7335d11134ae149675080169cb53cafc19c) )
	ROM_LOAD( "fn-06",        0x10000, 0x10000, CRC(3feee292) SHA1(d0dc75afffff268e0b3b817fbc060d52418a2ca7) )

	ROM_REGION( 0x40000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "fn-09",        0x00000, 0x20000, CRC(c2b27bd2) SHA1(8452d4442af476a35d3dfc4bd4df0a7d84a3dd7c) )
	ROM_LOAD( "fn-08",        0x20000, 0x20000, CRC(5ac97178) SHA1(7d246cb17986033ae2c219f7409e3b91be0dd259) )

	ROM_REGION( 0x40000, "gfx4", 0 ) /* sprites */
	ROM_LOAD( "fn-01",        0x00000, 0x10000, CRC(9333121b) SHA1(826ed261b1af41dd5468b2244767593d48577618) )
	ROM_LOAD( "fn-03",        0x10000, 0x10000, CRC(254ba60f) SHA1(71ab5cd48ee34da1d2dd951bb243a26d7a1171ae) )
	ROM_LOAD( "fn-00",        0x20000, 0x10000, CRC(ec18d098) SHA1(3cd1a27de295a177e81c14b9e9bbfcf5793aade2) )
	ROM_LOAD( "fn-02",        0x30000, 0x10000, CRC(4f060cba) SHA1(4063183e699bb8b6059d56f4e2fec5fa0b037c23) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "fn-11",      0x00000, 0x10000, CRC(990fd8d9) SHA1(a37bd96ecd75c610d98df3320f53ae4e2b7fdefd) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "ta-16.21k",          0x0000, 0x0100, CRC(ad26e8d4) SHA1(827337aeb8904429a1c050279240ae38aa6ce064) )  /* Priority (not used) */
ROM_END


uint16_t dec0_state::ffantasybl_242024_r()
{
/*
    000152: 41F9 0024 2020             lea     $242020.l, A0
    000158: 4A68 0004                  tst.w   ($4,A0)
    00015C: 6700 00A0                  beq     $1fe

    This allows us to at insert a coin...
*/

	return 0xffff;
}

/******************************************************************************/

//    YEAR, NAME,       PARENT,   MACHINE,    INPUT,      STATE/DEVICE,   INIT,        MONITOR,COMPANY,                 FULLNAME,            FLAGS
GAME( 1987, hbarrel,    0,        hbarrel,    hbarrel,    dec0_state, init_hbarrel,    ROT270, "Data East Corporation", "Heavy Barrel (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, hbarrelu,   hbarrel,  hbarrel,    hbarrel,    dec0_state, init_hbarrel,    ROT270, "Data East USA",         "Heavy Barrel (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, baddudes,   0,        baddudes,   baddudes,   dec0_state, init_hbarrel,    ROT0,   "Data East USA",         "Bad Dudes vs. Dragonninja (US revision 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, drgninja,   baddudes, baddudes,   drgninja,   dec0_state, init_hbarrel,    ROT0,   "Data East Corporation", "Dragonninja (Japan revision 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, birdtry,    0,        birdtry,    birdtry,    dec0_state, init_hbarrel,    ROT270, "Data East Corporation", "Birdie Try (Japan revision 2, revision 1 MCU)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, birdtrya,   birdtry,  birdtry,    birdtry,    dec0_state, init_hbarrel,    ROT270, "Data East Corporation", "Birdie Try (Japan revision 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, robocop,    0,        robocop,    robocop,    dec0_state, empty_init,      ROT0,   "Data East Corporation", "Robocop (World revision 4)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, robocopw,   robocop,  robocop,    robocop,    dec0_state, empty_init,      ROT0,   "Data East Corporation", "Robocop (World revision 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, robocopj,   robocop,  robocop,    robocop,    dec0_state, empty_init,      ROT0,   "Data East Corporation", "Robocop (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, robocopu,   robocop,  robocop,    robocop,    dec0_state, empty_init,      ROT0,   "Data East USA",         "Robocop (US revision 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, robocopu0,  robocop,  robocop,    robocop,    dec0_state, empty_init,      ROT0,   "Data East USA",         "Robocop (US revision 0)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, bandit,     0,        bandit,     bandit,     dec0_state, init_hbarrel,    ROT90,  "Data East USA",         "Bandit (US)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1989, hippodrm,   0,        hippodrm,   hippodrm,   dec0_state, init_hippodrm,   ROT0,   "Data East USA",         "Hippodrome (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, ffantasy,   hippodrm, hippodrm,   ffantasy,   dec0_state, init_hippodrm,   ROT0,   "Data East Corporation", "Fighting Fantasy (Japan revision 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, ffantasyj,  hippodrm, hippodrm,   ffantasy,   dec0_state, init_hippodrm,   ROT0,   "Data East Corporation", "Fighting Fantasy (Japan revision 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, ffantasya,  hippodrm, hippodrm,   ffantasy,   dec0_state, init_hippodrm,   ROT0,   "Data East Corporation", "Fighting Fantasy (Japan)", MACHINE_SUPPORTS_SAVE ) // presumably rev 1
GAME( 1989, ffantasyb,  hippodrm, hippodrm,   ffantasy,   dec0_state, init_hippodrm,   ROT0,   "Data East Corporation", "Fighting Fantasy (Japan revision ?)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, secretag,   0,        slyspy,     slyspy,     dec0_state, init_slyspy,     ROT0,   "Data East Corporation", "Secret Agent (World revision 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, secretagj,  secretag, slyspy,     slyspy,     dec0_state, init_slyspy,     ROT0,   "Data East Corporation", "Secret Agent (Japan revision 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, slyspy,     secretag, slyspy,     slyspy,     dec0_state, init_slyspy,     ROT0,   "Data East USA",         "Sly Spy (US revision 4)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, slyspy3,    secretag, slyspy,     slyspy,     dec0_state, init_slyspy,     ROT0,   "Data East USA",         "Sly Spy (US revision 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, slyspy2,    secretag, slyspy,     slyspy,     dec0_state, init_slyspy,     ROT0,   "Data East USA",         "Sly Spy (US revision 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, midres,     0,        midres,     midres,     dec0_state, empty_init,      ROT0,   "Data East Corporation", "Midnight Resistance (World, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, midres2,    midres,   midres,     midres,     dec0_state, empty_init,      ROT0,   "Data East Corporation", "Midnight Resistance (World, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, midresu,    midres,   midres,     midres,     dec0_state, empty_init,      ROT0,   "Data East USA",         "Midnight Resistance (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, midresj,    midres,   midres,     midres,     dec0_state, empty_init,      ROT0,   "Data East Corporation", "Midnight Resistance (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, bouldash,   0,        slyspy,     bouldash,   dec0_state, init_slyspy,     ROT0,   "Data East Corporation (licensed from First Star)", "Boulder Dash / Boulder Dash Part 2 (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, bouldashj,  bouldash, slyspy,     bouldash,   dec0_state, init_slyspy,     ROT0,   "Data East Corporation (licensed from First Star)", "Boulder Dash / Boulder Dash Part 2 (Japan)", MACHINE_SUPPORTS_SAVE )

// bootlegs

// more or less just an unprotected versions of the game, everything intact
GAME( 1988, robocopb,   robocop,  robocopb,   robocop,    dec0_state, empty_init,      ROT0, "bootleg", "Robocop (World bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, drgninjab,  baddudes, drgninjab,  drgninja,   dec0_state, init_drgninja,   ROT0, "bootleg", "Dragonninja (bootleg)", MACHINE_SUPPORTS_SAVE )


// this is a common bootleg board
GAME( 1989, midresb,    midres,   midresb,    midresb,    dec0_state, empty_init,      ROT0, "bootleg", "Midnight Resistance (bootleg with 68705)", MACHINE_SUPPORTS_SAVE ) // need to hook up 68705? (probably unused)
GAME( 1989, midresbj,   midres,   midresbj,   midresb,    dec0_state, empty_init,      ROT0, "bootleg", "Midnight Resistance (Joystick bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, ffantasybl, hippodrm, ffantasybl, ffantasybl, dec0_state, empty_init,      ROT0, "bootleg", "Fighting Fantasy (bootleg with 68705)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING ) // 68705 not dumped, might be the same as midresb
GAME( 1988, drgninjab2, baddudes, drgninjab,  drgninja,   dec0_state, init_drgninja,   ROT0, "bootleg", "Dragonninja (bootleg with 68705)", MACHINE_SUPPORTS_SAVE ) // is this the same board as above? (region warning hacked to World, but still shows Japanese text), 68705 dumped but not hooked up

// these are different to the above but quite similar to each other
GAME( 1988, automat,    robocop,  automat,    robocop,    dec0_automat_state, empty_init,   ROT0,   "bootleg", "Automat (bootleg of Robocop)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // sound rom / music from section z with mods for ADPCM?
GAME( 1989, secretab,   secretag, secretab,   slyspy,     dec0_automat_state, empty_init,   ROT0,   "bootleg", "Secret Agent (bootleg)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1989, mastbond,   secretag, secretab,   slyspy,     dec0_automat_state, empty_init,   ROT0,   "bootleg", "Master Bond (bootleg of Secret Agent)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
