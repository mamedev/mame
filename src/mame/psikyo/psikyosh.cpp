// license:BSD-3-Clause
// copyright-holders:David Haywood, Paul Priest
/*----------------------------------------------------------------
   Psikyo PS3/PS5/PS5v2 SH-2 Based Systems
   driver by David Haywood (+ Paul Priest)
   thanks to Farfetch'd for information about the sprite zoom table.
------------------------------------------------------------------

Moving on from the 68020 based system used for the first Strikers
1945 game Psikyo introduced a system using Hitachi's SH-2 CPU

This driver is for the single-screen PS3/PS5/PS5v2 boards

There appear to be multiple revisions of this board

 Board PS3-V1 (Custom Chip PS6406B)
 -----------------------------------
 Sol Divide (c)1997
 Strikers 1945 II (c)1997
 Space Bomber Ver.B (c)1998
 Daraku Tenshi - The Fallen Angels (c)1998

 Board PS5 (Custom Chip PS6406B)
 -------------------------------
 Gunbird 2 (c)1998
 Strikers 1999 / Strikers 1945 III (c)1999

 The PS5 board appears to just have a different memory map to PS3
 Otherwise identical.

 Board PS5V2 (Custom Chip PS6406B)
 ---------------------------------
 Dragon Blaze (c)2000
 Tetris The Grand Master 2 (c)2000
 Tetris The Grand Master 2 Plus (c)2000 (Confirmed by Japump to be a Dragon Blaze upgraded board)
 GunBarich (c)2001 (Appears to be a Dragon Blaze upgraded board, easily replaced chips have been swapped)
 Mahjong G-Taste (c)2002

 The PS5v2 board is only different physically.

All the boards have

YMF278B-F (80 pin PQFP) & YAC513 (16 pin SOIC)
( YMF278B-F is OPL4 == OPL3 plus a sample playback engine. )

93C56 EEPROM
( 93c56 is a 93c46 with double the address space. )

To Do:
  - see notes in video file -


*-----------------------------------*
|         Tips and Tricks           |
*-----------------------------------*

Hold Button during booting to test roms (Checksum 16-bit, on Words for gfx and Bytes for sound) for:

Daraku:           PL1 Button 1 (passes, doesn't test sound)
Space Bomber:     PL1 Start (passes all, only if bit 0x40 is set. But then EEPROM resets?)
Gunbird 2:        PL1 Start (passes all, only if bit 0x40 is set. But then EEPROM resets)
Strikers 1945III: PL1 Start (passes all, only if bit 0x40 is set)
Dragon Blaze:     PL1 Start (passes all, only if bit 0x40 is set)
Gunbarich:        PL1 Start (passes all, only if bit 0x40 is set)
Mahjong G-taste   PL1 Start (passes all)

Hold PL1 Button 1 and Test Mode button to get Maintenance mode for:

Space Bomber, Strikers 1945 II, Sol Divide, Daraku
(this works for earlier Psikyo games as well)

--- Space Bomber ---

Score rankings:
DOG-1
CAT-2
BUTA-3
KAME-4
IKA-5
RABBIT-6
FROG-7
TAKO-8

--- Gunbird 2 ---

5-2-0-4-8 Maintenance Mode
5-3-5-7-3 All Data Initialised

[Aine]
5-1-0-2-4 Secret Command Enabled ["Down" on ?]
5-3-7-6-5 Secret Random Enabled
5-3-1-5-7 Secret All Disabled

--- Strikers 1945 III / S1999 ---

8-1-6-5-0 Maintenance Mode
8-1-6-1-0 All Data Initialised
1-2-3-4-5 Best Score Erased

[X-36]
0-1-9-9-9 Secret Command Enabled ["Up" on ?]
8-1-6-3-0 Secret Random Enabled
8-1-6-2-0 Secret All Disabled

--- Dragon Blaze ---

9-2-2-2-0 Maintenance Mode
9-2-2-1-0 All Data Initialised
1-2-3-4-5 Best Score Erased

--- Gunbarich ---

0-2-9-2-0 Maintenance Mode
0-2-9-1-0 All Data Initialised
1-2-3-4-5 Best Score Erased

--- Tetris The Grand Master 2 / TGM2+ ---

4-1-5-7-3 All Data Initialised
4-1-7-6-5 Best Score Erased

The following 4 are also tested for, but appear to be disabled:
1-3-5-7-9
0-2-4-6-8
4-1-3-7-3
5-0-2-1-3

--- Mahjong G-Taste ---

1-3-5-1-0 All Data Initialised
1-3-5-2-0 Maintenance Mode
1-2-3-4-5 Hangs game, needs reset

----------------------------------------------------------------*/

/*

Psikyo PS3-V1 hardware readme
-----------------------------

Strikers 1945 II
Sol Divide
The Fallen Angels / Daraku Tenshi
Space Bomber

PCB Layout
----------

PS3-V1
|--------------------------------------------------|
|HA13118         3771    PROG_L.U18                |
|    VOL  YAC513         PROG_H.U17  |-----|       |
|      JRC4741                       | SH2 |       |
|                        PROG.U16    |     |       |
|     YMF278B  SOUND.U32             |-----|       |
|                                                  |
|J                                                 |
|A                57.2727MHz   HY514260   HY514260 |
|M                                                 |
|M                 |-------|                       |
|A                 |PSIKYO |                       |
|                  |PS6406B|         4L.U10 0L.U4  |
|          62256   |       |                       |
|93C56     62256   |-------|         5L.U9  1L.U3  |
|JP5                                               |
|    6H.U37  4H.U31  2H.U20  0H.U13  6L.U8  2L.U2  |
|                                                  |
|                                   *7L.U7  3L.U1  |
|   *7H.U36  5H.U30  3H.U19  1H.U12                |
|--------------------------------------------------|
Notes:
      JP5 - hardwired jumper bank (x4) for region selection. Cut 2ND jumper from left
            for International/English region. All jumpers shorted = Japan region (default)
      SH2 - Hitachi HD6417604F28 SH-2 CPU, clock input 28.63635 [57.2727/2] (QFP144)
      YMF278B - Yamaha YMF278B OPL4 sound chip, clock input 28.63635MHz [57.2727/2] (QFP80)
      PROG_H.U17/PROG_L.U18 - 27C4096 DIP40 EPROM
      PROG.U16 - 27C802 SOP44 OTP EPROM
      All other ROMs - 16M/32M SOP44 MaskROM
      * - ROM locations not populated
      VSync - 60Hz
      HSync - 15.27kHz

ROM Usage
---------

|-------------------------------------------|-CPU ROMs----|-GFX ROMs------------------------|-Sound ROM-|
| Game                                      | U18 U17 U16 | U4  U3  U2  U1  U10 U9  U8  U7  |    U32    |
|                                           |             | U13 U12 U20 U19 U31 U30 U37 U36 |           |
|-------------------------------------------|-------------|---------------------------------|-----------|
| Sol Divide - The Sword Of Darkness        | 4M  4M  -   | -   -   -   -   32M 32M 32M -   |    32M    |
|                                           |             | -   -   -   -   32M 32M 32M -   |           |
|-------------------------------------------|-------------|---------------------------------|-----------|
| Strikers 1945 II                          | 4M  4M  -   | 32M 32M 32M 32M -   -   -   -   |    32M    |
|                                           |             | 32M 32M 32M 32M -   -   -   -   |           |
|-------------------------------------------|-------------|---------------------------------|-----------|
| The Fallen Angels (World) /               | 4M  4M  8M  | 32M 32M 32M 32M 32M 32M 16M -   |    32M    |
| Daraku Tenshi - The Fallen Angels (Japan) |             | 32M 32M 32M 32M 32M 32M 16M -   |           |
|-------------------------------------------|-------------|---------------------------------|-----------|
| Space Bomber                              | 4M  4M  -   | 32M 32M 32M 32M 32M -   -   -   |    32M    |
|                                           |             | 32M 32M 32M 32M 32M -   -   -   |           |
|-------------------------------------------|-------------|---------------------------------|-----------|
-: Unpopulated


Psikyo PS5 hardware readme
--------------------------

Gunbird 2
Strikers 1945 III / Strikers 1999

PCB Layout
----------

PS5
|--------------------------------------------------|
|HA13118      M514260      PROG_L.U16    DATA.U1   |
| VOL    3771 M514260      PROG_H.U17              |
|                                    *PROG_DATA.U2 |
|                          PAL                     |
|                                                  |
|   JRC4741                          0H.U10  0L.U3 |
|J          |-----|                                |
|A          | SH2 |      57.2727MHz  1H.U11  1L.U4 |
|M  YAC513  |     |                                |
|M          |-----|       |-------|  2H.U12  2L.U5 |
|A                        |PSIKYO |                |
|                         |PS6406B|  3H.U13  3L.U6 |
|                         |       |                |
|                         |-------| *4H.U14 *4L.U7 |
|                                                  |
|                           62256   *5H.U15 *5L.U8 |
|                           62256                  |
|                   93C56                          |
|            JP4             YMF278B    SOUND.U9   |
|--------------------------------------------------|
Notes:
      JP4 - hardwired jumper bank (x4) for region selection. Cut leftmost jumper
            for International/English region. All jumpers shorted = Japan region (default)
      SH2 - Hitachi HD6417604F28 SH-2 CPU, clock input 28.63635 [57.2727/2] (QFP144)
      YMF278B - Yamaha YMF278B OPL4 sound chip, clock input 28.63635MHz [57.2727/2] (QFP80)
      PAL - AMD PALCE 16V8H stamped 'PS5-1' (DIP20)
      PROG_H.U17/PROG_L.U16/DATA.U1 - 27C4096 DIP40 EPROM
      All other ROMs - 64M/32M SOP44 MaskROM
      * - ROM locations not populated
      VSync - 60Hz
      HSync - 15.27kHz

ROM Usage
---------

|-----------------------------|-CPU ROMs--------|-GFX ROMs----------------|-Sound ROM-|
| Game                        | U16 U17 U1  U2  | U3  U4  U5  U6  U7  U8  |    U9     |
|                             |                 | U10 U11 U12 U13 U14 U15 |           |
|-----------------------------|-----------------|-------------------------|-----------|
| Gunbird 2                   | 4M  4M  4M  -   | 64M 64M 64M 32M -   -   |    32M    |
|                             |                 | 64M 64M 64M 32M -   -   |           |
|-----------------------------|-----------------|-------------------------|-----------|
| Strikers 1945 III (World) / | 4M  4M  4M  -   | 64M 64M 64M 32M -   -   |    32M    |
| Strikers 1999 (Japan)       |                 | 64M 64M 64M 32M -   -   |           |
|-----------------------------|-----------------|-------------------------|-----------|
-: Unpopulated


Psikyo PS5V2 hardware readme
----------------------------

Dragon Blaze, Psikyo, 2000
Gunbarich, Psikyo, 2001
Tetris The Grand Master 2, Arika / Psikyo, 2000
Tetris The Grand Master 2+, Arika / Psikyo, 2000
Mahjong G-Taste, Psikyo, 2002

PCB Layout
----------

PS5V2
|----------------------------------------------------|
|HA13118  PS5-1  SM81C256  PROG_H.U21 *0H.U11 *0L.U3 |
|VOL  JRC4741    SM81C256  PROG_L.U22                |
|     YAC516  3771                     1H.U12  1L.U4 |
|              |-----| *PROG_DATA.U23                |
|              | SH2 |                 2H.U13  2L.U5 |
|              |     |                               |
|J    YMF278B  |-----|                 3H.U14  3L.U6 |
|A                          57.2727MHz               |
|M                                     4H.U15  4L.U7 |
|M    SND.U52               |-------|                |
|A                          |PSIKYO |  5H.U16  5L.U8 |
|                           |PS6406B|                |
|                           |       | *6H.U17 *6L.U9 |
|                    62256  |-------|                |
|         JP3 93C56  62256            *7H.U18 *7L.U10|
|                                                    |
|  10L.U58    9L.U41    8L.U28    7L.U19     6L.U1   |
|                                                    |
|  10H.U59    9H.U42    8H.U29    7H.U20     6H.U2   |
|----------------------------------------------------|
Notes:
      JP3     - hardwired jumper bank (x4) for region selection. Cut rightmost jumper
                for International/English region. All jumpers shorted = Japan region (default)
      SH2     - Hitachi HD6417604F28 SH-2 CPU, clock input 28.63635 [57.2727/2] (QFP144)
      YMF278B - Yamaha YMF278B OPL4 sound chip, clock input 28.63635MHz [57.2727/2] (QFP80)
      PROG_H.U21/PROG_L.U22 - 27C4096 DIP40 EPROM
      ROMs U1-U59 (at bottom of PCB) - 16M DIP42 MaskROM
      ROMs U3-U10 & U11-U18 (at side of PCB) - 16M TSOP48 Type-II surface-mounted MaskROM
      ROM U52 - 16M/32M TSOP48 Type-II surface-mounted MaskROM
      * - ROM locations not populated
      VSync - 60Hz
      HSync - 15.27kHz

ROM Usage
---------

|-------------------------|-CPU ROMs----|-GFX ROMs(TSOP48)----------------|-GFX ROMs(DIP42)-----|-Sound ROM-|
| Game                    | U21 U22 U23 | U3  U4  U5  U6  U7  U8  U9  U10 | U1  U19 U28 U41 U58 |    U52    |
|                         |             | U11 U12 U13 U14 U15 U16 U17 U18 | U2  U20 U29 U42 U59 |           |
|-------------------------|-------------|---------------------------------|---------------------|-----------|
| Dragon Blaze            | 4M  4M  -   | -   16M 16M 16M 16M 16M -   -   | 16M 16M 16M 16M 16M |    16M    |
|                         |             | -   16M 16M 16M 16M 16M -   -   | 16M 16M 16M 16M 16M |           |
|-------------------------|-------------|---------------------------------|---------------------|-----------|
| Tetris: the Absolute -  | 4M  4M  -   | -   -   -   16M 16M 16M -   -   | 16M 16M 16M 16M 16M |    32M    |
| The Grand Master 2/Plus |             | -   -   -   16M 16M 16M -   -   | 16M 16M 16M 16M 16M |           |
|-------------------------|-------------|---------------------------------|---------------------|-----------|
| Gunbarich               | 4M  4M  -   | -   *   *   *   *   *   -   -   | 16M 16M 16M 16M *   |    16M    |
|                         |             | -   *   *   *   *   *   -   -   | 16M 16M 16M 16M *   |           |
|-------------------------|-------------|---------------------------------|---------------------|-----------|
| Mahjong G-Taste         | 4M  4M  -   | -   16M 16M 16M 16M *   -   -   | *   *   *   *   *   |    32M    |
|                         |             | -   16M 16M 16M 16M *   -   -   | *   *   *   *   *   |           |
|-------------------------|-------------|---------------------------------|---------------------|-----------|
-: Unpopulated
*: Leftover from Dragon Blaze

*/

#include "emu.h"
#include "psikyosh.h"

#include "machine/eepromser.h"
#include "machine/watchdog.h"
#include "sound/ymopl.h"
#include "speaker.h"


static GFXDECODE_START( gfx_psikyosh )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_16x16x4_packed_msb, 0x000, 0x100 ) // 4bpp tiles
	GFXDECODE_ENTRY( "gfx1", 0, gfx_16x16x8_raw,        0x000, 0x100 ) // 8bpp tiles
GFXDECODE_END

void psikyosh_state::eeprom_w(u8 data)
{
	m_eeprom->di_write(BIT(data, 5));
	m_eeprom->cs_write(BIT(data, 7));
	m_eeprom->clk_write(BIT(data, 6));

	if (data & ~0xe0)
		logerror("Unk EEPROM write %x\n", data);
}

INTERRUPT_GEN_MEMBER(psikyosh_state::interrupt)
{
	device.execute().set_input_line(4, ASSERT_LINE);
}

// VBL handler writes 0x00 on entry, 0xc0 on exit
// bit 0 controls game speed on readback, mechanism is a little weird
void psikyosh_state::irqctrl_w(u32 data)
{
	if (!(data & 0x00c00000))
	{
		m_maincpu->set_input_line(4, CLEAR_LINE);
	}
}


void psikyosh_state::vidregs_w(offs_t offset, u32 data, u32 mem_mask)
{
	u32 const old = m_vidregs[offset];
	data = COMBINE_DATA(&m_vidregs[offset]);

	if (offset == 4) /* Configure bank for gfx test */
	{
		if ((old ^ data) & 0xfff)    // Bank
			m_gfxrombank->set_entry(data & 0xfff);
	}
}

/* Mahjong Panel */
u32 psikyosh_state::mjgtaste_input_r()
{
/*
Mahjong keyboard encoder -> JAMMA adapter (SK-G001). Created to allow the use of a Mahjong panel with the existing, recycled Dragon Blaze boards.
PCB contains what looks like an MCU of some description labeled "TMIBOD 4", undumped
PCB maps keyboard lines onto JAMMA P1 controls
Normally the demultiplexing is taken care of in hardware (e.g. metro.c, ssv.c), but here the game code has to do it.

Standard Mahjong keyboard encoder
      /---------------- KEY7
     /    /------------ KEY10
    /    /   /--------- KEY6
   /    /   /   /------ KEY2
  /    /   /   /   /--- KEY3
 -|----D---C---B---A--- KEY4
 -|----H---G---F---E--- KEY5
 -|----L---K---J---I--- KEY8
 -FLIP-PON-CHI-N---M--- KEY9
 ----------RON-RCH-KAN- KEY11
 ------------------STR- KEY1

Standard Mahjong pinout - NOT JAMMA compatible
  Parts         Solder
   KEY3   1|2   KEY2
   KEY6   3|4   KEY10
   KEY7   7|8   Coin Counter
    GND   9|10  GND
         11|12
Service  13|14  Test
    RED  15|16  GREEN
   BLUE  17|18  SYNC
   SPK+  19|20  SPK- (GND)
 P1KEY4  21|22  P2KEY4
 P1KEY5  23|24  P2KEY5
 P1KEY8  25|26  P2KEY8
 P1KEY9  27|28  P2KEY9
P1KEY11  29|30  P2KEY11
 P1KEY1  31|32  P2KEY1
         33|34  Coin
         35|36
    GND  37|38  GND
    GND  39|40  GND
    +5V  41|42  +5V
    +5V  43|44  +5V
   +12V  45|46  +12V
         47|48
    +5V  49|50  +5V
    +5V  51|52  +5V
    GND  53|54  GND
    GND  55|56  GND
*/

	u32 const controls = m_controller_io->read();
	u32 value = m_inputs->read();

	if (controls)
	{
		// Clearly has ghosting, game will only recognise one key depressed at once, and keyboard can only represent keys with distinct rows and columns
		// Since the game can't accept conflicting inputs e.g. PL1 Up and 'A' or 'B' we have to
		// make the user choose the input method. Especially since in test mode both sets are usable.
		// Switch top word to either Mahjong inputs or joystick depending

		enum {
			KEY1  = 0x0400, // JAMMA P2 Button 1
			KEY2  = 0x0040, // JAMMA P2 Down
			KEY3  = 0x0080, // JAMMA P2 Up
			KEY4  = 0x8000, // JAMMA P1 Up
			KEY5  = 0x4000, // JAMMA P1 Down
			KEY6  = 0x0010, // JAMMA P2 Left
//          KEY7
			KEY8  = 0x1000, // JAMMA P1 Left
			KEY9  = 0x2000, // JAMMA P1 Right
			KEY10 = 0x0020, // JAMMA P2 Right
			KEY11 = 0x0800  // JAMMA P1 Button 1
		}; // Mahjong->JAMMA mapping specific to this game pcb

		static const u16 key_codes[] = { // treated as IP_ACTIVE_LOW, game inverts them upon reading
//          ROW (distinct pins for P1 or P2) | COLUMN (shared for P1+P2)
			KEY4 | KEY3,  // A
			KEY4 | KEY2,  // B
			KEY4 | KEY6,  // C
			KEY4 | KEY10, // D
			KEY5 | KEY3,  // E
			KEY5 | KEY2,  // F
			KEY5 | KEY6,  // G
			KEY5 | KEY10, // H
			KEY8 | KEY3,  // I
			KEY8 | KEY2,  // J
			KEY8 | KEY6,  // K
			KEY8 | KEY10, // L
			KEY9 | KEY3,  // M
			KEY9 | KEY2,  // N
			KEY11 | KEY3, // Kan
			KEY9 | KEY10, // Pon
			KEY9 | KEY6,  // Chi
			KEY11 | KEY2, // Reach/Lechi
			KEY11 | KEY6, // Ron
			KEY1 | KEY3   // Start
		}; // generic Mahjong keyboard encoder, corresponds to ordering in input port
		u32 keys = m_mahjong_io->read();
		u32 which_key = 0x1;
		int count = 0;

		// HACK: read IPT_START1 from "INPUTS" to avoid listing it twice or having two independent STARTs listed
		u32 const start_depressed = ~value & 0x01000000;
		keys |= start_depressed ? 1 << (std::size(key_codes) - 1) : 0; // and bung it in at the end

		value |= 0xFFFF0000; // set top word
		do {
			// since we can't handle multiple keys, just return the first one depressed
			if ((keys & which_key) && (count < std::size(key_codes)))
			{
				value &= ~((u32)(key_codes[count]) << 16); // mask in selected word as IP_ACTIVE_LOW
				break;
			}
			which_key <<= 1;
			count++;
		} while (which_key);
	}

	return value;
}


// ps3v1
void psikyosh_state::ps3v1_map(address_map &map)
{
// rom mapping
	map(0x00000000, 0x000fffff).rom(); // program ROM (1 meg)
	map(0x02000000, 0x020fffff).rom().region("maincpu", 0x100000); // data ROM
// video chip
	map(0x03000000, 0x0300ffff).ram().share("spriteram"); // sprite and backgrounds are share this area (video banks 0-1f)
	map(0x03040000, 0x03044fff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette"); // palette..
	map(0x03050000, 0x030501ff).ram().share("zoomram"); // sprite zoom lookup table
	map(0x0305ffdc, 0x0305ffdf).r("watchdog", FUNC(watchdog_timer_device::reset32_r)).w(FUNC(psikyosh_state::irqctrl_w)); // also writes to this address - might be vblank reads?
	map(0x0305ffe0, 0x0305ffff).ram().w(FUNC(psikyosh_state::vidregs_w)).share("vidregs"); //  video registers
	map(0x03060000, 0x0307ffff).bankr("gfxbank"); // data for rom tests (gfx), data is controlled by vidreg
// rom mapping
	map(0x04060000, 0x0407ffff).bankr("gfxbank"); // data for rom tests (gfx) (Mirrored?)
// sound chip
	map(0x05000000, 0x05000007).rw("ymf", FUNC(ymf278b_device::read), FUNC(ymf278b_device::write));
// inputs/eeprom
	map(0x05800000, 0x05800003).portr("INPUTS");
	map(0x05800004, 0x05800007).portr("JP4");
	map(0x05800004, 0x05800004).w(FUNC(psikyosh_state::eeprom_w));
// ram
	map(0x06000000, 0x060fffff).ram().share("ram"); // main RAM (1 meg)
}

// ps5, ps5v2
void psikyosh_state::ps5_map(address_map &map)
{
// rom mapping
	map(0x00000000, 0x000fffff).rom(); // program ROM (1 meg)
// inputs/eeprom
	map(0x03000000, 0x03000003).portr("INPUTS");
	map(0x03000004, 0x03000007).portr("JP4");
	map(0x03000004, 0x03000004).w(FUNC(psikyosh_state::eeprom_w));
// sound chip
	map(0x03100000, 0x03100007).rw("ymf", FUNC(ymf278b_device::read), FUNC(ymf278b_device::write));
// video chip
	map(0x04000000, 0x0400ffff).ram().share("spriteram"); // sprite and backgrounds are share this area (video banks 0-1f)
	map(0x04040000, 0x04044fff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0x04050000, 0x040501ff).ram().share("zoomram"); // sprite zoom lookup table
	map(0x0405ffdc, 0x0405ffdf).nopr().w(FUNC(psikyosh_state::irqctrl_w)); // also writes to this address - might be vblank reads?
	map(0x0405ffe0, 0x0405ffff).ram().w(FUNC(psikyosh_state::vidregs_w)).share("vidregs"); // video registers
	map(0x04060000, 0x0407ffff).bankr("gfxbank"); // data for rom tests (gfx), data is controlled by vidreg
// rom mapping
	map(0x05000000, 0x0507ffff).rom().region("maincpu", 0x100000); // data ROM
// ram
	map(0x06000000, 0x060fffff).ram().share("ram");
}

// mahjong
void psikyosh_state::ps5_mahjong_map(address_map &map)
{
	ps5_map(map);
	/* needs to install mahjong controls too (can select joystick in test mode tho) */
	map(0x03000000, 0x03000003).r(FUNC(psikyosh_state::mjgtaste_input_r));
}

void psikyosh_state::s1945iiibl_map(address_map &map)
{
	ps5_map(map);

	map(0x00000000, 0x001fffff).rom(); // bootleg has the data both in a separate chip and at the end of the 2 program ROMs. Why?
	map(0x04003000, 0x040031ff).ram().share("zoomram"); // sprite zoom lookup table is inside of spriteram instead of external like in the original
	map(0x04050000, 0x040501ff).unmaprw();
	map(0x04053fdc, 0x04053fdf).nopr().w(FUNC(psikyosh_state::irqctrl_w));
	map(0x04053fe0, 0x04053fff).ram().w(FUNC(psikyosh_state::vidregs_w)).share("vidregs");
	map(0x0405ffdc, 0x0405ffdf).unmaprw();
	map(0x0405ffe0, 0x0405ffff).unmaprw();
	map(0x05000000, 0x0507ffff).rom().region("maincpu", 0x200000); // data ROM
}

void psikyosh_state::s1945iiibla_map(address_map &map)
{
	ps5_map(map);

	map(0x04003000, 0x040031ff).ram().share("zoomram");
	map(0x04030000, 0x040301ff).ram(); // fails RAM test otherwise
	map(0x04053fdc, 0x04053fdf).nopr().w(FUNC(psikyosh_state::irqctrl_w));
	map(0x04053fe0, 0x04053fff).ram().w(FUNC(psikyosh_state::vidregs_w)).share("vidregs");
	map(0x0405ffdc, 0x0405ffdf).unmaprw();
	map(0x0405ffe0, 0x0405ffff).unmaprw();
}


static INPUT_PORTS_START( common )
	PORT_START("INPUTS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x00000020, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x00000040, 0x00000040, "Tilt (Enables Debug Mode)" )     /* Debug stuff. Resets EEPROM? */
	PORT_DIPSETTING(          0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
INPUT_PORTS_END


static INPUT_PORTS_START( s1945ii )
	PORT_INCLUDE( common )

	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_UNKNOWN )  /* No button 3 */
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNKNOWN )  /* No button 3 */

	PORT_START("JP4")   /* jumper pads on the PCB */
	PORT_DIPNAME( 0x01000000, 0x01000000, DEF_STR( Region ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Japan ) )
	PORT_DIPSETTING(          0x01000000, DEF_STR( World ) )
	PORT_BIT( 0x10000000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
INPUT_PORTS_END

static INPUT_PORTS_START( soldivid )
	PORT_INCLUDE( common )

	PORT_START("JP4")   /* jumper pads on the PCB */
	PORT_DIPNAME( 0x01000000, 0x01000000, DEF_STR( Region ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Japan ) )
	PORT_DIPSETTING(          0x01000000, DEF_STR( World ) )
	PORT_BIT( 0x10000000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
INPUT_PORTS_END

static INPUT_PORTS_START( soldividk )
	PORT_INCLUDE( common )

	PORT_START("JP4")   /* jumper pads on the PCB */
//  PORT_DIPNAME( 0x01000000, 0x01000000, DEF_STR( Region ) ) /* Game is hard coded to Korea */
//  PORT_DIPSETTING(          0x00000000, DEF_STR( Japan ) )
//  PORT_DIPSETTING(          0x01000000, DEF_STR( World ) )
	PORT_BIT( 0x10000000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
INPUT_PORTS_END

static INPUT_PORTS_START( daraku )
	PORT_INCLUDE( common )

	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_UNKNOWN )  /* No button 3 here */
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNKNOWN )  /* No button 3 here */

	PORT_START("JP4")   /* jumper pads on the PCB */
	PORT_DIPNAME( 0x01000000, 0x01000000, DEF_STR( Region ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Japan ) )
	PORT_DIPSETTING(          0x01000000, DEF_STR( World ) ) /* Title screen is different, English is default now */
	PORT_BIT( 0x10000000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
INPUT_PORTS_END

static INPUT_PORTS_START( sbomber )
	PORT_INCLUDE( common )
	/* If Debug is HIGH then you can perform rom test, but EEPROM resets? */

	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_UNKNOWN )  /* No button 3 */
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNKNOWN )  /* No button 3 */

	PORT_START("JP4")   /* jumper pads on the PCB */
	PORT_DIPNAME( 0x01000000, 0x01000000, DEF_STR( Region ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Japan ) )
	PORT_DIPSETTING(          0x01000000, DEF_STR( World ) )
	PORT_BIT( 0x10000000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
INPUT_PORTS_END

static INPUT_PORTS_START( gunbird2 ) /* Different Region */
	PORT_INCLUDE( common )
	/* If Debug is HIGH then you can perform rom test, but EEPROM resets */

	PORT_START("JP4")   /* jumper pads on the PCB */
	PORT_DIPNAME( 0x03000000, 0x02000000, DEF_STR( Region ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Japan ) )
	PORT_DIPSETTING(          0x01000000, "International Ver A." )
	PORT_DIPSETTING(          0x02000000, "International Ver B." )
	PORT_BIT( 0x10000000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
INPUT_PORTS_END

static INPUT_PORTS_START( s1945iii ) /* Different Region again */
	PORT_INCLUDE( common )
	/* If Debug is HIGH then you can perform rom test, EEPROM doesn't reset */

	PORT_START("JP4")   /* jumper pads on the PCB */
	PORT_DIPNAME( 0x03000000, 0x01000000, DEF_STR( Region ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Japan ) )
	PORT_DIPSETTING(          0x02000000, "International Ver A." )
	PORT_DIPSETTING(          0x01000000, "International Ver B." )
	PORT_BIT( 0x10000000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
INPUT_PORTS_END

static INPUT_PORTS_START( dragnblz )
	PORT_INCLUDE( common )
	/* If Debug is LOW then you can perform rom test and EEPROM security check is skipped, EEPROM doesn't reset */

	PORT_START("JP4")   /* jumper pads on the PCB */
	PORT_DIPNAME( 0x03000000, 0x01000000, DEF_STR( Region ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Japan ) )
	PORT_DIPSETTING(          0x02000000, "International Ver A." )
	PORT_DIPSETTING(          0x01000000, "International Ver B." )
	PORT_BIT( 0x10000000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
INPUT_PORTS_END

static INPUT_PORTS_START( gnbarich ) /* Same as S1945iii except only one button */
	PORT_INCLUDE( common )
	/* If Debug is HIGH then you can perform rom test, but EEPROM resets? */

	PORT_START("JP4")   /* jumper pads on the PCB */
	PORT_DIPNAME( 0x03000000, 0x01000000, DEF_STR( Region ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Japan ) )
	PORT_DIPSETTING(          0x02000000, "International Ver A." )
	PORT_DIPSETTING(          0x01000000, "International Ver B." )
	PORT_BIT( 0x10000000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
INPUT_PORTS_END

static INPUT_PORTS_START( tgm2 )
	PORT_INCLUDE( common )
	/* sticks should actually be PORT_4WAY according to manual */
	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)

	PORT_START("JP4")   /* jumper pads on the PCB. Checked and discarded. However, if you force word 0x6060000 to 1/2/3 you can have various effects. Disbled at compile time */
//  PORT_DIPNAME( 0x03000000, 0x01000000, DEF_STR( Region ) )
//  PORT_DIPSETTING(          0x00000000, DEF_STR( Japan ) )
//  PORT_DIPSETTING(          0x02000000, "International Ver A." )
//  PORT_DIPSETTING(          0x01000000, "International Ver B." )
	PORT_BIT( 0x10000000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
INPUT_PORTS_END

static INPUT_PORTS_START( mjgtaste )
	PORT_START("INPUTS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x00000020, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x00000040, 0x00000040, "Tilt (Enables Debug Mode)" )     /* Debug stuff. Resets EEPROM? */
	PORT_DIPSETTING(          0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_BIT( 0x00ffff80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START1 ) /* start for joystick */
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	// Make the user pick the input scheme since the game can't handle both simultaneously
	PORT_START("CONTROLLER")
	PORT_CONFNAME( 0x00000001, 0x00000001, DEF_STR ( Controller ) )
	PORT_CONFSETTING(          0x00000000, DEF_STR( Joystick ) )
	PORT_CONFSETTING(          0x00000001, "Mahjong Panel" )

	PORT_START("MAHJONG") /* artificial enumeration for mahjong encoder */
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_MAHJONG_A ) PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_MAHJONG_B ) PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_MAHJONG_C ) PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_MAHJONG_D ) PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_MAHJONG_E ) PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_HIGH, IPT_MAHJONG_F ) PORT_PLAYER(1)
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_MAHJONG_G ) PORT_PLAYER(1)
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_MAHJONG_H ) PORT_PLAYER(1)
	PORT_BIT( 0x00000100, IP_ACTIVE_HIGH, IPT_MAHJONG_I ) PORT_PLAYER(1)
	PORT_BIT( 0x00000200, IP_ACTIVE_HIGH, IPT_MAHJONG_J ) PORT_PLAYER(1)
	PORT_BIT( 0x00000400, IP_ACTIVE_HIGH, IPT_MAHJONG_K ) PORT_PLAYER(1)
	PORT_BIT( 0x00000800, IP_ACTIVE_HIGH, IPT_MAHJONG_L ) PORT_PLAYER(1)
	PORT_BIT( 0x00001000, IP_ACTIVE_HIGH, IPT_MAHJONG_M ) PORT_PLAYER(1)
	PORT_BIT( 0x00002000, IP_ACTIVE_HIGH, IPT_MAHJONG_N ) PORT_PLAYER(1)
	PORT_BIT( 0x00004000, IP_ACTIVE_HIGH, IPT_MAHJONG_KAN ) PORT_PLAYER(1)
	PORT_BIT( 0x00008000, IP_ACTIVE_HIGH, IPT_MAHJONG_PON ) PORT_PLAYER(1)
	PORT_BIT( 0x00010000, IP_ACTIVE_HIGH, IPT_MAHJONG_CHI ) PORT_PLAYER(1)
	PORT_BIT( 0x00020000, IP_ACTIVE_HIGH, IPT_MAHJONG_REACH ) PORT_PLAYER(1)
	PORT_BIT( 0x00040000, IP_ACTIVE_HIGH, IPT_MAHJONG_RON ) PORT_PLAYER(1)
//  PORT_BIT( 0x00080000, IP_ACTIVE_HIGH, IPT_START1 ) /* start on panel, hacked in from the regular one to avoid duplicates in the UI */

	PORT_START("JP4")   /* jumper pads on the PCB */
//  PORT_DIPNAME( 0x03000000, 0x01000000, DEF_STR( Region ) )
//  PORT_DIPSETTING(          0x00000000, DEF_STR( Japan ) )
//  PORT_DIPSETTING(          0x02000000, "International Ver A." )
//  PORT_DIPSETTING(          0x01000000, "International Ver B." )
	PORT_BIT( 0x10000000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
INPUT_PORTS_END


void psikyosh_state::machine_start()
{
	m_gfxrombank->configure_entries(0, 0x1000, memregion("gfx1")->base(), 0x20000);
}


void psikyosh_state::psikyo3v1(machine_config &config)
{
	/* basic machine hardware */
	SH7604(config, m_maincpu, MASTER_CLOCK/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &psikyosh_state::ps3v1_map);
	m_maincpu->set_vblank_int("screen", FUNC(psikyosh_state::interrupt));

	WATCHDOG_TIMER(config, "watchdog");

	EEPROM_93C56_8BIT(config, m_eeprom).default_value(0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0, 40*8-1, 0, 28*8-1);
	m_screen->set_screen_update(FUNC(psikyosh_state::screen_update));
	m_screen->screen_vblank().set(FUNC(psikyosh_state::screen_vblank));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_psikyosh);
	PALETTE(config, m_palette).set_format(palette_device::RGBx_888, 0x5000 / 4);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ymf278b_device &ymf(YMF278B(config, "ymf", MASTER_CLOCK/2));
	ymf.irq_handler().set_inputline(m_maincpu, 12);
	ymf.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void psikyosh_state::psikyo5(machine_config &config)
{
	psikyo3v1(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &psikyosh_state::ps5_map);
}

void psikyosh_state::psikyo5_mahjong(machine_config &config)
{
	psikyo5(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &psikyosh_state::ps5_mahjong_map);
}

void psikyosh_state::psikyo5_240(machine_config &config)
{
	psikyo5(config);

	/* Measured Hsync 16.165 KHz, Vsync 61.68 Hz */
	/* Ideally this would be driven off the video register. However, it doesn't change at runtime and MAME will pick a better screen resolution if it knows upfront */
	m_screen->set_raw(MASTER_CLOCK/8, 443, 0, 40*8, 262, 0, 30*8);
}

void psikyosh_state::s1945iiibl(machine_config &config)
{
	psikyo5(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &psikyosh_state::s1945iiibl_map);

	// TODO: this uses a YMF268-F sound chip. Seems compatible with YMF278B but verify if it has differences.
}

void psikyosh_state::s1945iiibla(machine_config &config)
{
	psikyo5(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &psikyosh_state::s1945iiibla_map);

	// TODO: this uses a YMF268-F sound chip. Seems compatible with YMF278B but verify if it has differences.
}

/* PS3 */

ROM_START( soldivid )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD32_WORD_SWAP( "2-prog_l.u18", 0x000002, 0x080000, CRC(cf179b04) SHA1(343f00a81cffd44334a4db81b6b828b7cf73c1e8) )
	ROM_LOAD32_WORD_SWAP( "1-prog_h.u17", 0x000000, 0x080000, CRC(f467d1c4) SHA1(a011e6f310a54f09efa0bf4597783cd78c05ad6f) )

	ROM_REGION( 0x3800000, "gfx1", ROMREGION_ERASE00 )
	/* This Space Empty! */
	ROM_LOAD32_WORD_SWAP( "4l.u10", 0x2000000, 0x400000, CRC(9eb9f269) SHA1(4a4d90eefe62b5462f5ed5e062eea7b6b4900f85) )
	ROM_LOAD32_WORD_SWAP( "4h.u31", 0x2000002, 0x400000, CRC(7c76cfe7) SHA1(14e291e840a4afe3802fe1847615c5e806d7492a) )
	ROM_LOAD32_WORD_SWAP( "5l.u9",  0x2800000, 0x400000, CRC(c59c6858) SHA1(bd580b57e432ef42295060c5a84c8129d9b995f7) )
	ROM_LOAD32_WORD_SWAP( "5h.u30", 0x2800002, 0x400000, CRC(73bc66d0) SHA1(7988ce81ff43235a3b30ddd8fd9419530a07b6ba) )
	ROM_LOAD32_WORD_SWAP( "6l.u8",  0x3000000, 0x400000, CRC(f01b816e) SHA1(2a0d86c1c106eef539028aa9ebe49d13216a6b9c) )
	ROM_LOAD32_WORD_SWAP( "6h.u37", 0x3000002, 0x400000, CRC(fdd57361) SHA1(f58d91acde1f4e6d4f0e8dcd1b23aa5092d89916) )

	ROM_REGION( 0x400000, "ymf", 0 )
	ROM_LOAD( "sound.u32", 0x000000, 0x400000, CRC(e98f8d45) SHA1(7791c0f31d08f37c6ec65e7cecf8ef54ca73b1fd) )
ROM_END

ROM_START( soldividk )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD32_WORD_SWAP( "9-prog_lk.u18", 0x000002, 0x080000, CRC(b534029d) SHA1(a96ca55e6694c5537d489d40bc890c376bbce933) )
	ROM_LOAD32_WORD_SWAP( "8-prog_hk.u17", 0x000000, 0x080000, CRC(a48e3206) SHA1(d560286972de2a0baa5369672a0190b7bd421843) )

	ROM_REGION( 0x3800000, "gfx1", ROMREGION_ERASE00 )
	/* This Space Empty! */
	ROM_LOAD32_WORD_SWAP( "4l.u10", 0x2000000, 0x400000, CRC(9eb9f269) SHA1(4a4d90eefe62b5462f5ed5e062eea7b6b4900f85) )
	ROM_LOAD32_WORD_SWAP( "4h.u31", 0x2000002, 0x400000, CRC(7c76cfe7) SHA1(14e291e840a4afe3802fe1847615c5e806d7492a) )
	ROM_LOAD32_WORD_SWAP( "5l.u9",  0x2800000, 0x400000, CRC(c59c6858) SHA1(bd580b57e432ef42295060c5a84c8129d9b995f7) )
	ROM_LOAD32_WORD_SWAP( "5h.u30", 0x2800002, 0x400000, CRC(73bc66d0) SHA1(7988ce81ff43235a3b30ddd8fd9419530a07b6ba) )
	ROM_LOAD32_WORD_SWAP( "6l.u8",  0x3000000, 0x400000, CRC(f01b816e) SHA1(2a0d86c1c106eef539028aa9ebe49d13216a6b9c) )
	ROM_LOAD32_WORD_SWAP( "6h.u37", 0x3000002, 0x400000, CRC(fdd57361) SHA1(f58d91acde1f4e6d4f0e8dcd1b23aa5092d89916) )

	ROM_REGION( 0x400000, "ymf", 0 )
	ROM_LOAD( "sound.u32", 0x000000, 0x400000, CRC(e98f8d45) SHA1(7791c0f31d08f37c6ec65e7cecf8ef54ca73b1fd) )
ROM_END

ROM_START( s1945ii )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASE00 ) /* Code */
	ROM_LOAD32_WORD_SWAP( "2_prog_l.u18", 0x000002, 0x080000, CRC(20a911b8) SHA1(82ba7b93bd621fc45a4dc2722752077b59a0a233) )
	ROM_LOAD32_WORD_SWAP( "1_prog_h.u17", 0x000000, 0x080000, CRC(4c0fe85e) SHA1(74f810a1c3e9d629c8b190f68d73ce07b11f77b7) )

	ROM_REGION( 0x2000000, "gfx1", 0 )  /* Tiles */
	ROM_LOAD32_WORD( "0l.u4",    0x0000000, 0x400000, CRC(bfacf98d) SHA1(19954f12881e6e95e808bd1f2c2f5a425786727f) )
	ROM_LOAD32_WORD( "0h.u13",   0x0000002, 0x400000, CRC(1266f67c) SHA1(cf93423a827aa92aa54afbbecf8509d2590edc9b) )
	ROM_LOAD32_WORD( "1l.u3",    0x0800000, 0x400000, CRC(2d3332c9) SHA1(f2e54100a48061bfd589e8765f59ca051176a38b) )
	ROM_LOAD32_WORD( "1h.u12",   0x0800002, 0x400000, CRC(27b32c3e) SHA1(17a80b3c919d8a282169c019ede8a22d2079c018) )
	ROM_LOAD32_WORD( "2l.u2",    0x1000000, 0x400000, CRC(91ba6d23) SHA1(fd016a90204b2de43bb709971f7cd891f839de1a) )
	ROM_LOAD32_WORD( "2h.u20",   0x1000002, 0x400000, CRC(fabf4334) SHA1(f8ec43e083b674700f532575f0d067bd49c5aaf7) )
	ROM_LOAD32_WORD( "3l.u1",    0x1800000, 0x400000, CRC(a6c3704e) SHA1(cb9881e4235cc8e4bcca4c6ccbd8d8d8634e3624) )
	ROM_LOAD32_WORD( "3h.u19",   0x1800002, 0x400000, CRC(4cd3ca70) SHA1(5b0a6ea4fe0e821cebe6e840596f648e24dded51) )

	ROM_REGION( 0x400000, "ymf", 0 ) /* Samples */
	ROM_LOAD( "sound.u32", 0x000000, 0x400000, CRC(ba680ca7) SHA1(b645896e297aad426784aa656bff738e1b33c2a2) )

	ROM_REGION( 0x100, "eeprom", 0 )
	ROM_LOAD( "eeprom-s1945ii.bin", 0x0000, 0x0100, CRC(7ac38846) SHA1(c5f4b05a94211f3c96b8c472adbe634f2e77d753) )
ROM_END

ROM_START( daraku )
	/* main program */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_WORD_SWAP( "4_prog_l.u18", 0x000002, 0x080000, CRC(660b4609) SHA1(ee6b5606fae41881c3e671ee642baae5c03331ca) )
	ROM_LOAD32_WORD_SWAP( "3_prog_h.u17", 0x000000, 0x080000, CRC(7a9cf601) SHA1(8df464ce3fd02b30dd2ab77828594f4916375fd5) )
	ROM_LOAD16_WORD_SWAP( "prog.u16",     0x100000, 0x100000, CRC(3742e990) SHA1(dd4b8777e57245151b3d520ed1bdab207530420b) )

	ROM_REGION( 0x3400000, "gfx1", 0 )
	ROM_LOAD32_WORD( "0l.u4",  0x0000000, 0x400000, CRC(565d8427) SHA1(090ce9213c530d29e488cfb89bb39fd7169985d5) )
	ROM_LOAD32_WORD( "0h.u13", 0x0000002, 0x400000, CRC(9a602630) SHA1(ab176490b36aec7ce30d1cf20b57c02c926c59d3) )
	ROM_LOAD32_WORD( "1l.u3",  0x0800000, 0x400000, CRC(ac5ce8e1) SHA1(7df6a04ea2530cc669581474e8b8ee6f59caae1b) )
	ROM_LOAD32_WORD( "1h.u12", 0x0800002, 0x400000, CRC(b0a59f7b) SHA1(8704705aa0977f11da8bcdafae6e2531190878d0) )
	ROM_LOAD32_WORD( "2l.u2",  0x1000000, 0x400000, CRC(2daa03b2) SHA1(475badc60cbd26786242d685a3d7dbaf385862a8) )
	ROM_LOAD32_WORD( "2h.u20", 0x1000002, 0x400000, CRC(e98e185a) SHA1(124d5fcf6cfb1faf70d665b687564bf6589d17c4) )
	ROM_LOAD32_WORD( "3l.u1",  0x1800000, 0x400000, CRC(1d372aa1) SHA1(e5965a1d8919409a314dfd56482a848d6ab9f5ac) )
	ROM_LOAD32_WORD( "3h.u19", 0x1800002, 0x400000, CRC(597f3f15) SHA1(62bf74ed29732e6cc1979458745cdb53a8edddf3) )
	ROM_LOAD32_WORD( "4l.u10", 0x2000000, 0x400000, CRC(e3d58cd8) SHA1(9482d0b71f840d72b20029804cfc8dca207462de) )
	ROM_LOAD32_WORD( "4h.u31", 0x2000002, 0x400000, CRC(aebc9cd0) SHA1(c20a1f9851ace74e00f1a0746e0c9e751ccec336) )
	ROM_LOAD32_WORD( "5l.u9",  0x2800000, 0x400000, CRC(eab5a50b) SHA1(76ce96e89afc438bafb9f8caa86eb48fb7e4e154) )
	ROM_LOAD32_WORD( "5h.u30", 0x2800002, 0x400000, CRC(f157474f) SHA1(89509f0772a40829070cea708c21438ff61d1019) )
	ROM_LOAD32_WORD( "6l.u8",  0x3000000, 0x200000, CRC(9f008d1b) SHA1(9607e09bde430eefe126569a6e251114bc8f754b) )
	ROM_LOAD32_WORD( "6h.u37", 0x3000002, 0x200000, CRC(acd2d0e3) SHA1(dee96bdf3b8efde1298b73c5e7dd62abcdc101cf) )

	ROM_REGION( 0x400000, "ymf", 0 ) /* Samples */
	ROM_LOAD( "sound.u32", 0x000000, 0x400000, CRC(ef2c781d) SHA1(1313f082f6dbe4da0efaf261226085eb7325667f) )

	ROM_REGION( 0x100, "eeprom", 0 )
	ROM_LOAD( "eeprom-daraku.bin", 0x0000, 0x0100, CRC(a9715297) SHA1(fcd32b936e0d05bad4ba4969ddec24aae7768cea) )
ROM_END

ROM_START( sbomber ) /* Version B - Only shows "Version B" when set to Japan region */
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD32_WORD_SWAP( "1-b_pr_l.u18", 0x000002, 0x080000, CRC(52d12225) SHA1(0a31a5d557414e7bf51dc6f7fbdd417a20b78df1) )
	ROM_LOAD32_WORD_SWAP( "1-b_pr_h.u17", 0x000000, 0x080000, CRC(1bbd0345) SHA1(c6ccb7c97cc9e9ea298c1883d1dd5563907a7255) )

	ROM_REGION( 0x2800000, "gfx1", 0 )
	ROM_LOAD32_WORD( "0l.u4",  0x0000000, 0x400000, CRC(b7e4ac51) SHA1(70e802b6235932116496a77ee0c78a256e85aff3) )
	ROM_LOAD32_WORD( "0h.u13", 0x0000002, 0x400000, CRC(235e6c27) SHA1(c597d7b5bef4edac1474ad0024cfb33eb1257106) )
	ROM_LOAD32_WORD( "1l.u3",  0x0800000, 0x400000, CRC(3c88c48c) SHA1(d1ce4ab60ba18449bbd96e29c310e060a0bb6de6) )
	ROM_LOAD32_WORD( "1h.u12", 0x0800002, 0x400000, CRC(15626a6e) SHA1(5493e92c9724982938591d758bee7d86cf96fd19) )
	ROM_LOAD32_WORD( "2l.u2",  0x1000000, 0x400000, CRC(41e92f64) SHA1(ea121c7cb35266ed0c21af4bb958fe5d73d84977) )
	ROM_LOAD32_WORD( "2h.u20", 0x1000002, 0x400000, CRC(4ae62e84) SHA1(adc1dab2f09aa4f5665d7bb7603a9b75c978031e) )
	ROM_LOAD32_WORD( "3l.u1",  0x1800000, 0x400000, CRC(43ba5f0f) SHA1(b8f93ed055441fd06b68103c9fd62b6aa3f3da7d) )
	ROM_LOAD32_WORD( "3h.u19", 0x1800002, 0x400000, CRC(ff01bb12) SHA1(df6fab898356c02f34ee7a45fdcc265218f2f20e) )
	ROM_LOAD32_WORD( "4l.u10", 0x2000000, 0x400000, CRC(e491d593) SHA1(12a7f6c282969be342b70443b8c802a399571245) )
	ROM_LOAD32_WORD( "4h.u31", 0x2000002, 0x400000, CRC(7bdd377a) SHA1(e357c98f82b8ea3ae4fd8eae6c1ad2dfb500db9c) )

	ROM_REGION( 0x400000, "ymf", 0 ) /* Samples */
	ROM_LOAD( "sound.u32", 0x000000, 0x400000, CRC(85cbff69) SHA1(34c7f4d337111de2064f84214294b6bdc37bf16c) )

	ROM_REGION( 0x100, "eeprom", 0 )
	ROM_LOAD( "eeprom-sbomber.bin", 0x0000, 0x0100, CRC(7ac38846) SHA1(c5f4b05a94211f3c96b8c472adbe634f2e77d753) ) /* Both versions use the same default settings */
ROM_END

ROM_START( sbombera ) /* Original version */
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD32_WORD_SWAP( "2.u18", 0x000002, 0x080000, CRC(57819a26) SHA1(d7a6fc957e39adf97762ab0a35b91aa17ec026e0) )
	ROM_LOAD32_WORD_SWAP( "1.u17", 0x000000, 0x080000, CRC(c388e847) SHA1(cbf4f2e191894160bdf0290d72cf20c222aaf7a7) )

	ROM_REGION( 0x2800000, "gfx1", 0 )
	ROM_LOAD32_WORD( "0l.u4",  0x0000000, 0x400000, CRC(b7e4ac51) SHA1(70e802b6235932116496a77ee0c78a256e85aff3) )
	ROM_LOAD32_WORD( "0h.u13", 0x0000002, 0x400000, CRC(235e6c27) SHA1(c597d7b5bef4edac1474ad0024cfb33eb1257106) )
	ROM_LOAD32_WORD( "1l.u3",  0x0800000, 0x400000, CRC(3c88c48c) SHA1(d1ce4ab60ba18449bbd96e29c310e060a0bb6de6) )
	ROM_LOAD32_WORD( "1h.u12", 0x0800002, 0x400000, CRC(15626a6e) SHA1(5493e92c9724982938591d758bee7d86cf96fd19) )
	ROM_LOAD32_WORD( "2l.u2",  0x1000000, 0x400000, CRC(41e92f64) SHA1(ea121c7cb35266ed0c21af4bb958fe5d73d84977) )
	ROM_LOAD32_WORD( "2h.u20", 0x1000002, 0x400000, CRC(4ae62e84) SHA1(adc1dab2f09aa4f5665d7bb7603a9b75c978031e) )
	ROM_LOAD32_WORD( "3l.u1",  0x1800000, 0x400000, CRC(43ba5f0f) SHA1(b8f93ed055441fd06b68103c9fd62b6aa3f3da7d) )
	ROM_LOAD32_WORD( "3h.u19", 0x1800002, 0x400000, CRC(ff01bb12) SHA1(df6fab898356c02f34ee7a45fdcc265218f2f20e) )
	ROM_LOAD32_WORD( "4l.u10", 0x2000000, 0x400000, CRC(e491d593) SHA1(12a7f6c282969be342b70443b8c802a399571245) )
	ROM_LOAD32_WORD( "4h.u31", 0x2000002, 0x400000, CRC(7bdd377a) SHA1(e357c98f82b8ea3ae4fd8eae6c1ad2dfb500db9c) )

	ROM_REGION( 0x400000, "ymf", 0 ) /* Samples */
	ROM_LOAD( "sound.u32", 0x000000, 0x400000, CRC(85cbff69) SHA1(34c7f4d337111de2064f84214294b6bdc37bf16c) )

	ROM_REGION( 0x100, "eeprom", 0 )
	ROM_LOAD( "eeprom-sbomber.bin", 0x0000, 0x0100, CRC(7ac38846) SHA1(c5f4b05a94211f3c96b8c472adbe634f2e77d753) ) /* Both versions use the same default settings */
ROM_END

/* PS5 */

ROM_START( gunbird2 ) /* Internal date string shows Oct 07 16:05 */
	ROM_REGION( 0x180000, "maincpu", 0 )
	ROM_LOAD32_WORD_SWAP( "2_prog_l.u16", 0x000002, 0x080000, CRC(76f934f0) SHA1(cf197796d66f15639a6b3d5311c18da33cefd06b) )
	ROM_LOAD32_WORD_SWAP( "1_prog_h.u17", 0x000000, 0x080000, CRC(7328d8bf) SHA1(c640de1ab5b32400b2d77e0dc6e3ee0f78ab7803) )
	ROM_LOAD16_WORD_SWAP( "3_pdata.u1",   0x100000, 0x080000, CRC(a5b697e6) SHA1(947f124fa585c2cf77c6571af7559bd652897b89) )

	ROM_REGION( 0x3800000, "gfx1", 0 )
	ROM_LOAD32_WORD( "0l.u3",  0x0000000, 0x800000, CRC(5c826bc8) SHA1(74fb6b242b4c5fe5365cfcc3029ed6da4cf3a621) )
	ROM_LOAD32_WORD( "0h.u10", 0x0000002, 0x800000, CRC(3df0cb6c) SHA1(271d276fa0f63d84e458223316a9517865fc2255) )
	ROM_LOAD32_WORD( "1l.u4",  0x1000000, 0x800000, CRC(1558358d) SHA1(e3b9c3da4e9b29ffa9568b57d14fe2b600aead68) )
	ROM_LOAD32_WORD( "1h.u11", 0x1000002, 0x800000, CRC(4ee0103b) SHA1(29bbe0162dda39919fcd188ea4a6b7b5f20366ff) )
	ROM_LOAD32_WORD( "2l.u5",  0x2000000, 0x800000, CRC(e1c7a7b8) SHA1(b5f6e5d53e21928197773df7dde0e7c83f4082af) )
	ROM_LOAD32_WORD( "2h.u12", 0x2000002, 0x800000, CRC(bc8a41df) SHA1(90460b11eea778f17cf8be67430e2ab149680686) )
	ROM_LOAD32_WORD( "3l.u6",  0x3000000, 0x400000, CRC(0229d37f) SHA1(f9d98d1d2dda2d552b2a46c76b4c7fc84b1aa4c6) )
	ROM_LOAD32_WORD( "3h.u13", 0x3000002, 0x400000, CRC(f41bbf2b) SHA1(b705274e392541e2f513a4ae4bae543c03be0913) )

	ROM_REGION( 0x400000, "ymf", 0 ) /* Samples */
	ROM_LOAD( "sound.u9", 0x000000, 0x400000, CRC(f19796ab) SHA1(b978f0550ebd675e8ce9d9edcfcc3f6214e49e8b) )

	ROM_REGION( 0x100, "eeprom", 0 )
	ROM_LOAD( "eeprom-gunbird2.bin", 0x0000, 0x0100, CRC(7ac38846) SHA1(c5f4b05a94211f3c96b8c472adbe634f2e77d753) )
ROM_END

/*
This Program & Data ROM set came from a PCB in Korea.

Notable differences:
 On initial bootup, no sound of "Marion...."
 On inserting coins, no "Gunbird 2" wave, just a tone pattern
 On selecting a character to play, no character name wave, just a simple tone
 On capturing a power-up there's no "Power Up" in character voice, just a simple tone

Seems to have minimal changes, probably just the sound table.  Internal date string
shows Oct 08 17:02, while the above parent set shows Oct 07 16:05

This set can still be set Japan, International Ver A. or International Ver B. via jumper pads
*/

ROM_START( gunbird2a ) /* Internal date string shows Oct 08 17:02 - No specific Korean copyright / message, but made for the Korean market? */
	ROM_REGION( 0x180000, "maincpu", 0 )
	ROM_LOAD32_WORD_SWAP( "prog_l.u16", 0x000002, 0x080000, CRC(974f85ba) SHA1(4e19b12bd5f268088317ea231bbe7f9d2d694b2b) ) /* these roms had no labels */
	ROM_LOAD32_WORD_SWAP( "prog_h.u17", 0x000000, 0x080000, CRC(cb0cb826) SHA1(8827e9ebfedbc63dbf41c6a5c994a691a6d63fdb) )
	ROM_LOAD16_WORD_SWAP( "pdata.u1",   0x100000, 0x080000, CRC(23751839) SHA1(4762685d1f6843e8e53eae6c014e66b98fa15eb7) )

	ROM_REGION( 0x3800000, "gfx1", 0 )
	ROM_LOAD32_WORD( "0l.u3",  0x0000000, 0x800000, CRC(5c826bc8) SHA1(74fb6b242b4c5fe5365cfcc3029ed6da4cf3a621) )
	ROM_LOAD32_WORD( "0h.u10", 0x0000002, 0x800000, CRC(3df0cb6c) SHA1(271d276fa0f63d84e458223316a9517865fc2255) )
	ROM_LOAD32_WORD( "1l.u4",  0x1000000, 0x800000, CRC(1558358d) SHA1(e3b9c3da4e9b29ffa9568b57d14fe2b600aead68) )
	ROM_LOAD32_WORD( "1h.u11", 0x1000002, 0x800000, CRC(4ee0103b) SHA1(29bbe0162dda39919fcd188ea4a6b7b5f20366ff) )
	ROM_LOAD32_WORD( "2l.u5",  0x2000000, 0x800000, CRC(e1c7a7b8) SHA1(b5f6e5d53e21928197773df7dde0e7c83f4082af) )
	ROM_LOAD32_WORD( "2h.u12", 0x2000002, 0x800000, CRC(bc8a41df) SHA1(90460b11eea778f17cf8be67430e2ab149680686) )
	ROM_LOAD32_WORD( "3l.u6",  0x3000000, 0x400000, CRC(0229d37f) SHA1(f9d98d1d2dda2d552b2a46c76b4c7fc84b1aa4c6) )
	ROM_LOAD32_WORD( "3h.u13", 0x3000002, 0x400000, CRC(f41bbf2b) SHA1(b705274e392541e2f513a4ae4bae543c03be0913) )

	ROM_REGION( 0x400000, "ymf", 0 ) /* Samples */
	ROM_LOAD( "sound.u9", 0x000000, 0x400000, CRC(f19796ab) SHA1(b978f0550ebd675e8ce9d9edcfcc3f6214e49e8b) )

	ROM_REGION( 0x100, "eeprom", 0 )
	ROM_LOAD( "eeprom-gunbird2.bin", 0x0000, 0x0100, CRC(7ac38846) SHA1(c5f4b05a94211f3c96b8c472adbe634f2e77d753) )
ROM_END

ROM_START( s1945iii )
	ROM_REGION( 0x180000, "maincpu", 0 )
	ROM_LOAD32_WORD_SWAP( "2_progl.u16", 0x000002, 0x080000, CRC(5d5d385f) SHA1(67b3bcabd71cf084bcea7a59939281a8d6257059) )
	ROM_LOAD32_WORD_SWAP( "1_progh.u17", 0x000000, 0x080000, CRC(1b8a5a18) SHA1(718a176bd48e16f964fcb07c568b5227cfc0515f) )
	ROM_LOAD16_WORD_SWAP( "3_data.u1",   0x100000, 0x080000, CRC(8ff5f7d3) SHA1(420a3d7f2d5ab6a56789d36b418431f12f5f73f5) )

	ROM_REGION( 0x3800000, "gfx1", 0 )
	ROM_LOAD32_WORD( "0l.u3",  0x0000000, 0x800000, CRC(70a0d52c) SHA1(c9d9534da59123b577dc22020273b94ccdeeb67d) )
	ROM_LOAD32_WORD( "0h.u10", 0x0000002, 0x800000, CRC(4dcd22b4) SHA1(2df7a7d08df17d2a62d574fccc8ba40aaae21a13) )
	ROM_LOAD32_WORD( "1l.u4",  0x1000000, 0x800000, CRC(de1042ff) SHA1(468f6dfd5c1f2084c573b6851e314ff2826dc350) )
	ROM_LOAD32_WORD( "1h.u11", 0x1000002, 0x800000, CRC(b51a4430) SHA1(b51117591b0e351e922f9a6a7930e8b50237e54e) )
	ROM_LOAD32_WORD( "2l.u5",  0x2000000, 0x800000, CRC(23b02dca) SHA1(0249dceca02b312301a917d98fac481b6a0a9122) )
	ROM_LOAD32_WORD( "2h.u12", 0x2000002, 0x800000, CRC(9933ab04) SHA1(710e6b20e111c1898666b4466554d039309883cc) )
	ROM_LOAD32_WORD( "3l.u6",  0x3000000, 0x400000, CRC(f693438c) SHA1(d70e25a3f56aae6575c696d9b7b6d7a9d04f0104) )
	ROM_LOAD32_WORD( "3h.u13", 0x3000002, 0x400000, CRC(2d0c334f) SHA1(74d94abb34484c7b79dbb989645f53124e53e3b7) )

	ROM_REGION( 0x400000, "ymf", 0 ) /* Samples */
	ROM_LOAD( "sound.u9", 0x000000, 0x400000, CRC(c5374beb) SHA1(d13e12cbd249246d953c45bb3bfa576a0ec75595) )

	ROM_REGION( 0x100, "eeprom", 0 )
	ROM_LOAD( "eeprom-s1945iii.bin", 0x0000, 0x0100, CRC(b39f3604) SHA1(d7c66210598096fcafb20adac2f0b293755f4926) )
ROM_END

ROM_START( s1945iiibl ) // BM1045 PCB - has HD6417098 CPU + YMF268-F sound chip + 2x Altera MAX II
	ROM_REGION( 0x280000, "maincpu", 0 )
	ROM_LOAD32_WORD_SWAP( "strikers1945.u2",    0x000002, 0x100000, CRC(ef3f8857) SHA1(fa51dbcbba84f5ce183bb122bd80cface6fe431d) ) // chip type unknown, one big sticker covering this and the ROM below
	ROM_IGNORE(                                           0x100000 ) // 1xxxxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD32_WORD_SWAP( "strikers1945.u1",    0x000000, 0x100000, CRC(470d909b) SHA1(e813f9ee7fb074b35d9d82d299b81820ddbafab4) ) // chip type unknown, one big sticker covering this and the ROM above
	ROM_IGNORE(                                           0x100000 ) // 1xxxxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_WORD_SWAP( "083351_10-01-13.u3", 0x200000, 0x080000, CRC(f76eb183) SHA1(457f6f959c0885b64ec35a3046f2f0b60798a2e0) ) // M27C160, u3 also written with a marker on the label
	ROM_IGNORE(                                           0x180000 ) // 11xxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x3800000, "gfx1", 0 ) // not dumped for this bootleg, which actually has 4x 26L12811MC-12 MTPs
	ROM_LOAD32_WORD( "0l.u3",  0x0000000, 0x800000, BAD_DUMP CRC(70a0d52c) SHA1(c9d9534da59123b577dc22020273b94ccdeeb67d) )
	ROM_LOAD32_WORD( "0h.u10", 0x0000002, 0x800000, BAD_DUMP CRC(4dcd22b4) SHA1(2df7a7d08df17d2a62d574fccc8ba40aaae21a13) )
	ROM_LOAD32_WORD( "1l.u4",  0x1000000, 0x800000, BAD_DUMP CRC(de1042ff) SHA1(468f6dfd5c1f2084c573b6851e314ff2826dc350) )
	ROM_LOAD32_WORD( "1h.u11", 0x1000002, 0x800000, BAD_DUMP CRC(b51a4430) SHA1(b51117591b0e351e922f9a6a7930e8b50237e54e) )
	ROM_LOAD32_WORD( "2l.u5",  0x2000000, 0x800000, BAD_DUMP CRC(23b02dca) SHA1(0249dceca02b312301a917d98fac481b6a0a9122) )
	ROM_LOAD32_WORD( "2h.u12", 0x2000002, 0x800000, BAD_DUMP CRC(9933ab04) SHA1(710e6b20e111c1898666b4466554d039309883cc) )
	ROM_LOAD32_WORD( "3l.u6",  0x3000000, 0x400000, BAD_DUMP CRC(f693438c) SHA1(d70e25a3f56aae6575c696d9b7b6d7a9d04f0104) )
	ROM_LOAD32_WORD( "3h.u13", 0x3000002, 0x400000, BAD_DUMP CRC(2d0c334f) SHA1(74d94abb34484c7b79dbb989645f53124e53e3b7) )

	ROM_REGION( 0x400000, "ymf", 0 ) // not dumped for this bootleg, which actually has a LH28F640BFN flash
	ROM_LOAD( "sound.u9", 0x000000, 0x400000, BAD_DUMP CRC(c5374beb) SHA1(d13e12cbd249246d953c45bb3bfa576a0ec75595) )

	ROM_REGION( 0x100, "eeprom", 0 )
	ROM_LOAD( "eeprom-s1945iii.bin", 0x0000, 0x0100, CRC(b39f3604) SHA1(d7c66210598096fcafb20adac2f0b293755f4926) )
ROM_END

ROM_START( s1945iiibla ) // SK000420 PCB
	ROM_REGION( 0x180000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD32_WORD_SWAP( "u3", 0x000002, 0x080000, CRC(c111ed1a) SHA1(45125287641373f7b9184846de9b4b9262cbf602) ) // 27C240 EPROM with no lables, marked in pencil as 1, 2 & 3
	ROM_LOAD32_WORD_SWAP( "u1", 0x000000, 0x080000, CRC(f67016d0) SHA1(3b52690c450524d54a0c74639049f1b648b6ceb9) )
	ROM_LOAD16_WORD_SWAP( "u2", 0x100000, 0x080000, CRC(4922685b) SHA1(9af39abeed55459981d6dd74aaeff35293c66892) )

	ROM_REGION( 0x3800000, "gfx1", 0 ) // data verified same as original PCB
	ROM_LOAD32_WORD( "0l",  0x0000000, 0x800000, CRC(70a0d52c) SHA1(c9d9534da59123b577dc22020273b94ccdeeb67d) ) // mask ROMs with silkscreened labels
	ROM_LOAD32_WORD( "0h",  0x0000002, 0x800000, CRC(4dcd22b4) SHA1(2df7a7d08df17d2a62d574fccc8ba40aaae21a13) )
	ROM_LOAD32_WORD( "1l",  0x1000000, 0x800000, CRC(de1042ff) SHA1(468f6dfd5c1f2084c573b6851e314ff2826dc350) )
	ROM_LOAD32_WORD( "1h",  0x1000002, 0x800000, CRC(b51a4430) SHA1(b51117591b0e351e922f9a6a7930e8b50237e54e) )
	ROM_LOAD32_WORD( "2l",  0x2000000, 0x800000, CRC(23b02dca) SHA1(0249dceca02b312301a917d98fac481b6a0a9122) )
	ROM_LOAD32_WORD( "2h",  0x2000002, 0x800000, CRC(9933ab04) SHA1(710e6b20e111c1898666b4466554d039309883cc) )
	ROM_LOAD32_WORD( "3ll", 0x3000000, 0x400000, CRC(f693438c) SHA1(d70e25a3f56aae6575c696d9b7b6d7a9d04f0104) )
	ROM_LOAD32_WORD( "3hl", 0x3000002, 0x400000, CRC(2d0c334f) SHA1(74d94abb34484c7b79dbb989645f53124e53e3b7) )

	ROM_REGION( 0x400000, "ymf", 0 ) // data verified same as original PCB
	ROM_LOAD( "sound.u9", 0x000000, 0x400000, CRC(c5374beb) SHA1(d13e12cbd249246d953c45bb3bfa576a0ec75595) )

	ROM_REGION( 0x100, "eeprom", 0 )
	ROM_LOAD( "eeprom-s1945iii.bin", 0x0000, 0x0100, CRC(b39f3604) SHA1(d7c66210598096fcafb20adac2f0b293755f4926) )

	ROM_REGION( 0x0400, "pals", 0 )
	ROM_LOAD( "gal16v8d_1", 0x0000, 0x0117, CRC(efac207d) SHA1(0e1ada029a90efbc557cad1b0ec81b7904e69bd4) ) // GAL16V8D - bruteforced
	ROM_LOAD( "gal16v8d_2", 0x0200, 0x0117, CRC(72afa479) SHA1(959129da59d254c60e6e0e1061512d14a62ee2bf) ) // GAL16V8D - bruteforced

ROM_END

/* PS5v2 */

ROM_START( dragnblz )
	ROM_REGION( 0x180000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD32_WORD_SWAP( "2prog_h.u21",   0x000000, 0x080000, CRC(fc5eade8) SHA1(e5d05543641e4a3900b0d42e0d5f75734683d635) )
	ROM_LOAD32_WORD_SWAP( "1prog_l.u22",   0x000002, 0x080000, CRC(95d6fd02) SHA1(2b2830e7fa66cbd13666191762bfddc40571caec) )

	ROM_REGION( 0x2c00000, "gfx1", ROMREGION_ERASE00 )  /* Sprites */
	ROM_LOAD32_WORD( "1l.u4",  0x0400000, 0x200000, CRC(c2eb565c) SHA1(07e41b36cc03a87f28d091754fdb0d1a7316a532) )
	ROM_LOAD32_WORD( "1h.u12", 0x0400002, 0x200000, CRC(23cb46b7) SHA1(005b7cc40eea103688a64a72c219c7535970dbfb) )
	ROM_LOAD32_WORD( "2l.u5",  0x0800000, 0x200000, CRC(bc256aea) SHA1(1f1d678e8a63513a95f296b8a07d2ea485d1e53f) )
	ROM_LOAD32_WORD( "2h.u13", 0x0800002, 0x200000, CRC(b75f59ec) SHA1(a6cde94bc972e46e54c962fde49fc2174b312882) )
	ROM_LOAD32_WORD( "3l.u6",  0x0c00000, 0x200000, CRC(4284f008) SHA1(610b13304043411b3088fd4299b3cb0a4d8b0cc2) )
	ROM_LOAD32_WORD( "3h.u14", 0x0c00002, 0x200000, CRC(abe5cbbf) SHA1(c2fb1d8ea8772572c08b36496cf9fc5b91cf848b) )
	ROM_LOAD32_WORD( "4l.u7",  0x1000000, 0x200000, CRC(c9fcf2e5) SHA1(7cecdf3406da11289b54aaf58d12883ddfdc5e6b) )
	ROM_LOAD32_WORD( "4h.u15", 0x1000002, 0x200000, CRC(0ab0a12a) SHA1(1b29b6dc79e69edb56634517365d0ee8e6ea78ae) )
	ROM_LOAD32_WORD( "5l.u8",  0x1400000, 0x200000, CRC(68d03ccf) SHA1(d2bf6da5fa6e346b05872ed9616ffe51c3768f50) )
	ROM_LOAD32_WORD( "5h.u16", 0x1400002, 0x200000, CRC(5450fbca) SHA1(7a804263549cea951782a67855e69cb8cb417e98) )
	ROM_LOAD32_WORD( "6l.u1",  0x1800000, 0x200000, CRC(8b52c90b) SHA1(e1067ef252870787e46c62015e5778b4e641e68d) )
	ROM_LOAD32_WORD( "6h.u2",  0x1800002, 0x200000, CRC(7362f929) SHA1(9ced06202e3f104d30377aeef489021d26e87f73) )
	ROM_LOAD32_WORD( "7l.u19", 0x1c00000, 0x200000, CRC(b4f4d86e) SHA1(2ad786c5626c98e6943ae05688a1b66307ceac84) )
	ROM_LOAD32_WORD( "7h.u20", 0x1c00002, 0x200000, CRC(44b7b9cc) SHA1(3f8122b62ea1183d9fb3aad32d0e47bd32244f87) )
	ROM_LOAD32_WORD( "8l.u28", 0x2000000, 0x200000, CRC(cd079f89) SHA1(49c46eb36bc0458428a7fad3fe622f5ed974073b) )
	ROM_LOAD32_WORD( "8h.u29", 0x2000002, 0x200000, CRC(3edb508a) SHA1(72b07fb34a94cc127de02070604b1ff31f3d46c7) )
	ROM_LOAD32_WORD( "9l.u41", 0x2400000, 0x200000, CRC(0b53cd78) SHA1(e2071d9fe6c7be4e289b491587ab431c164e59da) )
	ROM_LOAD32_WORD( "9h.u42", 0x2400002, 0x200000, CRC(bc61998a) SHA1(75dbefe712104c64576196c27c25dbed59ae3923) )
	ROM_LOAD32_WORD( "10l.u58",0x2800000, 0x200000, CRC(a3f5c7f8) SHA1(d17478ca3e7ef46270f350ffa35d43acb05b1185) )
	ROM_LOAD32_WORD( "10h.u59",0x2800002, 0x200000, CRC(30e304c4) SHA1(1d866276bfe7f7524306a880d225aaf11ac2e5dd) )

	ROM_REGION( 0x400000, "ymf", 0 ) /* Samples */
	ROM_LOAD( "snd0.u52", 0x000000, 0x200000, CRC(7fd1b225) SHA1(6aa61021ada51393bbb34fd1aea00b8feccc8197) )

	ROM_REGION( 0x100, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "eeprom-dragnblz.u44", 0x0000, 0x0100, CRC(46e85da9) SHA1(673cf974fd23a20e6bfa7b2b234206d550011f54) )
ROM_END

/*
Starting with Gunbarich and including Mahjong G-Taste, Psikyo started to "recycle" left over Dragon Blaze PCBs.
  Psikyo would replace some of the Dragon Blaze roms with the new game roms leaving many of the surface mounted
  roms intact.  The new games don't use or access the left over roms, but the PCB needs the roms filled to function.

  The hidden rom tests in Gunbarich and Mahjong G-Teste shows the games only uses the new game roms.
*/

ROM_START( gnbarich )
	ROM_REGION( 0x180000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD32_WORD_SWAP( "2-prog_l.u21",   0x000000, 0x080000, CRC(c136cd9c) SHA1(ab66c4f5196a66a97dbb5832336a203421cf40fa) )
	ROM_LOAD32_WORD_SWAP( "1-prog_h.u22",   0x000002, 0x080000, CRC(6588fc96) SHA1(3db29fcf17e8b2aee465319b557bd3e45bc966b2) )

	ROM_REGION( 0x2c00000, "gfx1", ROMREGION_ERASE00 )  /* Sprites */
//  ROM_LOAD32_WORD( "1l.u4",  0x0400000, 0x200000, CRC(c2eb565c) SHA1(07e41b36cc03a87f28d091754fdb0d1a7316a532) ) /* From Dragon Blaze */
//  ROM_LOAD32_WORD( "1h.u12", 0x0400002, 0x200000, CRC(23cb46b7) SHA1(005b7cc40eea103688a64a72c219c7535970dbfb) ) /* From Dragon Blaze */
//  ROM_LOAD32_WORD( "2l.u5",  0x0800000, 0x200000, CRC(bc256aea) SHA1(1f1d678e8a63513a95f296b8a07d2ea485d1e53f) ) /* From Dragon Blaze */
//  ROM_LOAD32_WORD( "2h.u13", 0x0800002, 0x200000, CRC(b75f59ec) SHA1(a6cde94bc972e46e54c962fde49fc2174b312882) ) /* From Dragon Blaze */
//  ROM_LOAD32_WORD( "3l.u6",  0x0c00000, 0x200000, CRC(4284f008) SHA1(610b13304043411b3088fd4299b3cb0a4d8b0cc2) ) /* From Dragon Blaze */
//  ROM_LOAD32_WORD( "3h.u14", 0x0c00002, 0x200000, CRC(abe5cbbf) SHA1(c2fb1d8ea8772572c08b36496cf9fc5b91cf848b) ) /* From Dragon Blaze */
//  ROM_LOAD32_WORD( "4l.u7",  0x1000000, 0x200000, CRC(c9fcf2e5) SHA1(7cecdf3406da11289b54aaf58d12883ddfdc5e6b) ) /* From Dragon Blaze */
//  ROM_LOAD32_WORD( "4h.u15", 0x1000002, 0x200000, CRC(0ab0a12a) SHA1(1b29b6dc79e69edb56634517365d0ee8e6ea78ae) ) /* From Dragon Blaze */
//  ROM_LOAD32_WORD( "5l.u8",  0x1400000, 0x200000, CRC(68d03ccf) SHA1(d2bf6da5fa6e346b05872ed9616ffe51c3768f50) ) /* From Dragon Blaze */
//  ROM_LOAD32_WORD( "5h.u16", 0x1400002, 0x200000, CRC(5450fbca) SHA1(7a804263549cea951782a67855e69cb8cb417e98) ) /* From Dragon Blaze */
	ROM_LOAD32_WORD( "6l.u1",  0x1800000, 0x200000, CRC(0432e1a8) SHA1(632cb6534a19a92aa16d1dc8bb98c0c1fa17e428) )
	ROM_LOAD32_WORD( "6h.u2",  0x1800002, 0x200000, CRC(f90fa3ea) SHA1(773861c6c559f2df88e395669f27c43bd4dd6eb6) )
	ROM_LOAD32_WORD( "7l.u19", 0x1c00000, 0x200000, CRC(36bf9a58) SHA1(b546425f17f4b0b1112f0a22f9f5c695f5d97fe9) )
	ROM_LOAD32_WORD( "7h.u20", 0x1c00002, 0x200000, CRC(4b3eafd8) SHA1(8d0a4516bab2a188a66291e805c3c265774a6b72) )
	ROM_LOAD32_WORD( "8l.u28", 0x2000000, 0x200000, CRC(026754da) SHA1(66072e7584dcfea614a1e37592bda65733c9ce11) )
	ROM_LOAD32_WORD( "8h.u29", 0x2000002, 0x200000, CRC(8cd7aaa0) SHA1(83469c5407cba134ec1d22330623d8be8e0eabec) )
	ROM_LOAD32_WORD( "9l.u41", 0x2400000, 0x200000, CRC(02c066fe) SHA1(ecd5f36d9e55a341aff956bab4e7b0ae9e6cc15f) )
	ROM_LOAD32_WORD( "9h.u42", 0x2400002, 0x200000, CRC(5433385a) SHA1(138d62409cfb9e1a4eb3ca378ab8f6df45d478c0) )
//  ROM_LOAD32_WORD( "10l.u58",0x2800000, 0x200000, CRC(a3f5c7f8) SHA1(d17478ca3e7ef46270f350ffa35d43acb05b1185) ) /* From Dragon Blaze */
//  ROM_LOAD32_WORD( "10h.u59",0x2800002, 0x200000, CRC(30e304c4) SHA1(1d866276bfe7f7524306a880d225aaf11ac2e5dd) ) /* From Dragon Blaze */

	ROM_REGION( 0x400000, "ymf", 0 ) /* Samples */
	ROM_LOAD( "snd0.u52", 0x000000, 0x200000, CRC(7b10436b) SHA1(c731fcce024e286a677ca10a91761c1ee06094a5) )

	ROM_REGION( 0x100, "eeprom", 0 )
	ROM_LOAD( "eeprom-gnbarich.bin", 0x0000, 0x0100, CRC(0f5bf42f) SHA1(ad9d724327a2321f8ae256002d847213e6486b7b) )
ROM_END

ROM_START( mjgtaste )
	ROM_REGION( 0x180000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD32_WORD_SWAP( "2.u21",   0x000000, 0x080000, CRC(5f2041dc) SHA1(f3862ffdb8df0cf921ce1cb0236935731e7729a7) )
	ROM_LOAD32_WORD_SWAP( "1.u22",   0x000002, 0x080000, CRC(f5ff7876) SHA1(4c909db9c97f29fd79df6dacd29762688701b973) )

	ROM_REGION( 0x2c00000, "gfx1", ROMREGION_ERASE00 )  /* Sprites */
	ROM_LOAD32_WORD( "1l.u4",  0x0400000, 0x200000, CRC(30da42b1) SHA1(8485f2c0e7769b50b95d962afe14fa7ae74cd887) )
	ROM_LOAD32_WORD( "1h.u12", 0x0400002, 0x200000, CRC(629c1d44) SHA1(61909091328bb7b6d3e6e0bff91e14c9b4b86c2c) )
	ROM_LOAD32_WORD( "2l.u5",  0x0800000, 0x200000, CRC(1f6126ab) SHA1(e9fc70ca42798c04a4d4e1ef1113a59477c77fdc) )
	ROM_LOAD32_WORD( "2h.u13", 0x0800002, 0x200000, CRC(dba34e46) SHA1(ae26ef5f90431274764aaa72ae179c4c7572d2ad) )
	ROM_LOAD32_WORD( "3l.u6",  0x0c00000, 0x200000, CRC(1023e35e) SHA1(47a6f6f205703e6ee86b6e77b386060f4b2d87df) )
	ROM_LOAD32_WORD( "3h.u14", 0x0c00002, 0x200000, CRC(8aebec7f) SHA1(f56ab20901688665438b1efd254d5796d43c6625) )
	ROM_LOAD32_WORD( "4l.u7",  0x1000000, 0x200000, CRC(9acf018b) SHA1(9d1e34273a8cfb0ef481c9f31459960bb93b6431) )
	ROM_LOAD32_WORD( "4h.u15", 0x1000002, 0x200000, CRC(f93e154c) SHA1(f783c6ddb3727eeface1df7c1d0bc3bb28bef047) )
//  ROM_LOAD32_WORD( "5l.u8",  0x1400000, 0x200000, CRC(68d03ccf) SHA1(d2bf6da5fa6e346b05872ed9616ffe51c3768f50) ) /* From Dragon Blaze */
//  ROM_LOAD32_WORD( "5h.u16", 0x1400002, 0x200000, CRC(5450fbca) SHA1(7a804263549cea951782a67855e69cb8cb417e98) ) /* From Dragon Blaze */
//  ROM_LOAD32_WORD( "6l.u1",  0x1800000, 0x200000, CRC(8b52c90b) SHA1(e1067ef252870787e46c62015e5778b4e641e68d) ) /* From Dragon Blaze */
//  ROM_LOAD32_WORD( "6h.u2",  0x1800002, 0x200000, CRC(7362f929) SHA1(9ced06202e3f104d30377aeef489021d26e87f73) ) /* From Dragon Blaze */
//  ROM_LOAD32_WORD( "7l.u19", 0x1c00000, 0x200000, CRC(b4f4d86e) SHA1(2ad786c5626c98e6943ae05688a1b66307ceac84) ) /* From Dragon Blaze */
//  ROM_LOAD32_WORD( "7h.u20", 0x1c00002, 0x200000, CRC(44b7b9cc) SHA1(3f8122b62ea1183d9fb3aad32d0e47bd32244f87) ) /* From Dragon Blaze */
//  ROM_LOAD32_WORD( "8l.u28", 0x2000000, 0x200000, CRC(cd079f89) SHA1(49c46eb36bc0458428a7fad3fe622f5ed974073b) ) /* From Dragon Blaze */
//  ROM_LOAD32_WORD( "8h.u29", 0x2000002, 0x200000, CRC(3edb508a) SHA1(72b07fb34a94cc127de02070604b1ff31f3d46c7) ) /* From Dragon Blaze */
//  ROM_LOAD32_WORD( "9l.u41", 0x2400000, 0x200000, CRC(0b53cd78) SHA1(e2071d9fe6c7be4e289b491587ab431c164e59da) ) /* From Dragon Blaze */
//  ROM_LOAD32_WORD( "9h.u42", 0x2400002, 0x200000, CRC(bc61998a) SHA1(75dbefe712104c64576196c27c25dbed59ae3923) ) /* From Dragon Blaze */
//  ROM_LOAD32_WORD( "10l.u58",0x2800000, 0x200000, CRC(a3f5c7f8) SHA1(d17478ca3e7ef46270f350ffa35d43acb05b1185) ) /* From Dragon Blaze */
//  ROM_LOAD32_WORD( "10h.u59",0x2800002, 0x200000, CRC(30e304c4) SHA1(1d866276bfe7f7524306a880d225aaf11ac2e5dd) ) /* From Dragon Blaze */

	ROM_REGION( 0x400000, "ymf", 0 ) /* Samples */
	ROM_LOAD( "snd0.u52", 0x000000, 0x400000, CRC(0179f018) SHA1(16ae63e021230356777342ed902e02407a1a1b82) )

	ROM_REGION( 0x100, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "eeprom-mjgtaste.u44", 0x0000, 0x0100, CRC(d35586f2) SHA1(ce26a82d760f87dccfc15468ac3d24efc258648d) )
ROM_END

ROM_START( tgm2 )
	ROM_REGION( 0x180000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD32_WORD_SWAP( "2.u21",   0x000000, 0x080000, CRC(b19f6c31) SHA1(c58346c575db71262aebc3993743cb031c41e4af) )
	ROM_LOAD32_WORD_SWAP( "1.u22",   0x000002, 0x080000, CRC(c521bf24) SHA1(0ee5b9f74b6b8bcc01b2270c53f30d99e877ed64) )

	ROM_REGION( 0x3000000, "gfx1", ROMREGION_ERASE00 )  /* Sprites */
	// Lower positions not populated
	ROM_LOAD32_WORD( "81ts_3l.u6",   0x0c00000, 0x200000, CRC(d77cff9c) SHA1(93ee48c350110ebf9a80cca95c599c90a523147d) )
	ROM_LOAD32_WORD( "82ts_3h.u14",  0x0c00002, 0x200000, CRC(f012b583) SHA1(907e1c93cbfa6a0285f96c53f5ccb63e313053d7) )
	ROM_LOAD32_WORD( "83ts_4l.u7",   0x1000000, 0x200000, CRC(078cafc3) SHA1(26e47c8f0aaa461e69e9f40ee61ce4b4cc480776) )
	ROM_LOAD32_WORD( "84ts_4h.u15",  0x1000002, 0x200000, CRC(1f91446b) SHA1(81b43156c6a0f4e63dcc9e7c1e9dd54bcba38240) )
	ROM_LOAD32_WORD( "85ts_5l.u8",   0x1400000, 0x200000, CRC(40fbd259) SHA1(6b8cbfc6232e04785fd232158b3f4d56fadb0c7d) )
	ROM_LOAD32_WORD( "86ts_5h.u16",  0x1400002, 0x200000, CRC(186c935f) SHA1(0cab30c2ec4df3dc35b4c9de63d29bd0bc99afdb) )
	ROM_LOAD32_WORD( "87ts_6l.u1",   0x1800000, 0x200000, CRC(c17dc48a) SHA1(4399bfc253fb1cd4ef1081d7350c73df3a0f7441) )
	ROM_LOAD32_WORD( "88ts_6h.u2",   0x1800002, 0x200000, CRC(e4dba5da) SHA1(24db1e19f4df94ba3a22fba59e4fd065921db1c5) )
	ROM_LOAD32_WORD( "89ts_7l.u19",  0x1c00000, 0x200000, CRC(dab1b2c5) SHA1(114fd7717b97cdfd605ab7e2a354190c41ba4a82) )
	ROM_LOAD32_WORD( "90ts_7h.u20",  0x1c00002, 0x200000, CRC(aae696b3) SHA1(9ac27365719c1700f647911dc324a0e2aacea172) )
	ROM_LOAD32_WORD( "91ts_8l.u28",  0x2000000, 0x200000, CRC(e953ace1) SHA1(c6cdfd807a7a84b86378c3585aeb7c0cb066f8a1) )
	ROM_LOAD32_WORD( "92ts_8h.u29",  0x2000002, 0x200000, CRC(9da3b976) SHA1(ce1e4eb93760749200ede45703412868ca29a5e7) )
	ROM_LOAD32_WORD( "93ts_9l.u41",  0x2400000, 0x200000, CRC(233087fe) SHA1(c4adb307ce11ef558fd23b299ce7f458de581446) )
	ROM_LOAD32_WORD( "94ts_9h.u42",  0x2400002, 0x200000, CRC(9da831c7) SHA1(42698697aa85df088745b2d37ec89b01adce700f) )
	ROM_LOAD32_WORD( "95ts_10l.u58", 0x2800000, 0x200000, CRC(303a5240) SHA1(5816d1922e85bc27a2a13cdd183d9e67c7ddb2e1) )
	ROM_LOAD32_WORD( "96ts_10h.u59", 0x2800002, 0x200000, CRC(2240ebf6) SHA1(b61f93a18dd9d94fb57d95745d4df2e41a0371ff) )

	ROM_REGION( 0x400000, "ymf", 0 ) /* Samples */
	ROM_LOAD( "97ts_snd.u52", 0x000000, 0x400000, CRC(9155eca6) SHA1(f0b4f68462d8a465c39815d3b7fd9818788132ae) )

	ROM_REGION( 0x100, "eeprom", 0 ) /* Default Eeprom (contains scores etc.) */
	// might need byteswapping to reprogram actual chip
	ROM_LOAD( "tgm2.default.nv", 0x000, 0x100, CRC(50e2348c) SHA1(d17d2739c97a1d93a95dcc9f11feb1f6f228729e) )
ROM_END

ROM_START( tgm2p )
	ROM_REGION( 0x180000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD32_WORD_SWAP( "2b.u21",   0x000000, 0x080000, CRC(38bc626c) SHA1(783e8413b11f1fa08d331b09ef4ed63f62b87ead) )
	ROM_LOAD32_WORD_SWAP( "1b.u22",   0x000002, 0x080000, CRC(7599fb19) SHA1(3f7e81756470c173cc17a7e7dee91437571fd0c3) )

	ROM_REGION( 0x3000000, "gfx1", ROMREGION_ERASE00 )  /* Sprites */
	// Lower positions not populated
	ROM_LOAD32_WORD( "81ts_3l.u6",   0x0c00000, 0x200000, CRC(d77cff9c) SHA1(93ee48c350110ebf9a80cca95c599c90a523147d) )
	ROM_LOAD32_WORD( "82ts_3h.u14",  0x0c00002, 0x200000, CRC(f012b583) SHA1(907e1c93cbfa6a0285f96c53f5ccb63e313053d7) )
	ROM_LOAD32_WORD( "83ts_4l.u7",   0x1000000, 0x200000, CRC(078cafc3) SHA1(26e47c8f0aaa461e69e9f40ee61ce4b4cc480776) )
	ROM_LOAD32_WORD( "84ts_4h.u15",  0x1000002, 0x200000, CRC(1f91446b) SHA1(81b43156c6a0f4e63dcc9e7c1e9dd54bcba38240) )
	ROM_LOAD32_WORD( "85ts_5l.u8",   0x1400000, 0x200000, CRC(40fbd259) SHA1(6b8cbfc6232e04785fd232158b3f4d56fadb0c7d) )
	ROM_LOAD32_WORD( "86ts_5h.u16",  0x1400002, 0x200000, CRC(186c935f) SHA1(0cab30c2ec4df3dc35b4c9de63d29bd0bc99afdb) )
	ROM_LOAD32_WORD( "87ts_6l.u1",   0x1800000, 0x200000, CRC(c17dc48a) SHA1(4399bfc253fb1cd4ef1081d7350c73df3a0f7441) )
	ROM_LOAD32_WORD( "88ts_6h.u2",   0x1800002, 0x200000, CRC(e4dba5da) SHA1(24db1e19f4df94ba3a22fba59e4fd065921db1c5) )
	ROM_LOAD32_WORD( "89ts_7l.u19",  0x1c00000, 0x200000, CRC(dab1b2c5) SHA1(114fd7717b97cdfd605ab7e2a354190c41ba4a82) )
	ROM_LOAD32_WORD( "90ts_7h.u20",  0x1c00002, 0x200000, CRC(aae696b3) SHA1(9ac27365719c1700f647911dc324a0e2aacea172) )
	ROM_LOAD32_WORD( "91ts_8l.u28",  0x2000000, 0x200000, CRC(e953ace1) SHA1(c6cdfd807a7a84b86378c3585aeb7c0cb066f8a1) )
	ROM_LOAD32_WORD( "92ts_8h.u29",  0x2000002, 0x200000, CRC(9da3b976) SHA1(ce1e4eb93760749200ede45703412868ca29a5e7) )
	ROM_LOAD32_WORD( "93ts_9l.u41",  0x2400000, 0x200000, CRC(233087fe) SHA1(c4adb307ce11ef558fd23b299ce7f458de581446) )
	ROM_LOAD32_WORD( "94ts_9h.u42",  0x2400002, 0x200000, CRC(9da831c7) SHA1(42698697aa85df088745b2d37ec89b01adce700f) )
	ROM_LOAD32_WORD( "95ts_10l.u58", 0x2800000, 0x200000, CRC(303a5240) SHA1(5816d1922e85bc27a2a13cdd183d9e67c7ddb2e1) )
	ROM_LOAD32_WORD( "96ts_10h.u59", 0x2800002, 0x200000, CRC(2240ebf6) SHA1(b61f93a18dd9d94fb57d95745d4df2e41a0371ff) )

	ROM_REGION( 0x400000, "ymf", 0 ) /* Samples */
	ROM_LOAD( "97ts_snd.u52", 0x000000, 0x400000, CRC(9155eca6) SHA1(f0b4f68462d8a465c39815d3b7fd9818788132ae) )

	ROM_REGION( 0x100, "eeprom", 0 ) /* Default Eeprom (contains scores etc.) */
	// might need byteswapping to reprogram actual chip
	ROM_LOAD( "tgm2p.default.nv", 0x000, 0x100, CRC(b2328b40) SHA1(e6cda4d6f4e91b9f78d2ca84a5eee6c3bd03fe02) )
ROM_END

void psikyosh_state::init_ps3()
{
	m_maincpu->sh2drc_set_options(SH2DRC_FASTEST_OPTIONS);
	m_maincpu->sh2drc_add_fastram(0x03000000, 0x0300ffff, 0, &m_spriteram[0]);
	m_maincpu->sh2drc_add_fastram(0x03050000, 0x030501ff, 0, &m_zoomram[0]);
	m_maincpu->sh2drc_add_fastram(0x06000000, 0x060fffff, 0, &m_ram[0]);
}

void psikyosh_state::init_ps5()
{
	m_maincpu->sh2drc_set_options(SH2DRC_FASTEST_OPTIONS);
	m_maincpu->sh2drc_add_fastram(0x04000000, 0x0400ffff, 0, &m_spriteram[0]);
	m_maincpu->sh2drc_add_fastram(0x04050000, 0x040501ff, 0, &m_zoomram[0]);
	m_maincpu->sh2drc_add_fastram(0x06000000, 0x060fffff, 0, &m_ram[0]);
}

void psikyosh_state::init_s1945iiibl()
{
	m_maincpu->sh2drc_set_options(SH2DRC_FASTEST_OPTIONS);
	m_maincpu->sh2drc_add_fastram(0x04000000, 0x0400ffff, 0, &m_spriteram[0]);
	m_maincpu->sh2drc_add_fastram(0x04003000, 0x040031ff, 0, &m_zoomram[0]); // zoomram is in spriteram here
	m_maincpu->sh2drc_add_fastram(0x06000000, 0x060fffff, 0, &m_ram[0]);
}


//    YEAR  NAME        PARENT    MACHINE          INPUT      STATE           INIT             MONITOR COMPANY    FULLNAME                                                                 FLAGS */

/* ps3-v1 */
GAME( 1997, soldivid,   0,        psikyo3v1,       soldivid,  psikyosh_state, init_ps3,        ROT0,   "Psikyo",  "Sol Divide - The Sword Of Darkness",                                    MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1997, soldividk,  soldivid, psikyo3v1,       soldividk, psikyosh_state, init_ps3,        ROT0,   "Psikyo",  "Sol Divide - The Sword Of Darkness (Korea)",                            MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1997, s1945ii,    0,        psikyo3v1,       s1945ii,   psikyosh_state, init_ps3,        ROT270, "Psikyo",  "Strikers 1945 II",                                                      MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1998, daraku,     0,        psikyo3v1,       daraku,    psikyosh_state, init_ps3,        ROT0,   "Psikyo",  "The Fallen Angels (World) / Daraku Tenshi - The Fallen Angels (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1998, sbomber,    0,        psikyo3v1,       sbomber,   psikyosh_state, init_ps3,        ROT270, "Psikyo",  "Space Bomber (ver. B)",                                                 MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1998, sbombera,   sbomber,  psikyo3v1,       sbomber,   psikyosh_state, init_ps3,        ROT270, "Psikyo",  "Space Bomber",                                                          MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

/* ps5 */
GAME( 1998, gunbird2,   0,        psikyo5,         gunbird2,  psikyosh_state, init_ps5,        ROT270, "Psikyo",  "Gunbird 2 (set 1)",                                                     MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1998, gunbird2a,  gunbird2, psikyo5,         gunbird2,  psikyosh_state, init_ps5,        ROT270, "Psikyo",  "Gunbird 2 (set 2)",                                                     MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1999, s1945iii,   0,        psikyo5,         s1945iii,  psikyosh_state, init_ps5,        ROT270, "Psikyo",  "Strikers 1945 III (World) / Strikers 1999 (Japan)",                     MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1999, s1945iiibl, s1945iii, s1945iiibl,      s1945iii,  psikyosh_state, init_s1945iiibl, ROT270, "bootleg", "Strikers 1945 III (World) / Strikers 1999 (Japan) (bootleg, set 1)",    MACHINE_NOT_WORKING | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // sprites don't show up if MAME is started without debugger active
GAME( 1999, s1945iiibla,s1945iii, s1945iiibla,     s1945iii,  psikyosh_state, init_s1945iiibl, ROT270, "bootleg", "Strikers 1945 III (World) / Strikers 1999 (Japan) (bootleg, set 2)",    MACHINE_NOT_WORKING | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // sprites don't show up if MAME is started without debugger active

/* ps5v2 */
GAME( 2000, dragnblz,   0,        psikyo5,         dragnblz,  psikyosh_state, init_ps5,        ROT270, "Psikyo",  "Dragon Blaze",                                                          MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 2000, tgm2,       0,        psikyo5_240,     tgm2,      psikyosh_state, init_ps5,        ROT0,   "Arika",   "Tetris: The Absolute - The Grand Master 2",                             MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 2000, tgm2p,      tgm2,     psikyo5_240,     tgm2,      psikyosh_state, init_ps5,        ROT0,   "Arika",   "Tetris: The Absolute - The Grand Master 2 Plus",                        MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 2001, gnbarich,   0,        psikyo5,         gnbarich,  psikyosh_state, init_ps5,        ROT270, "Psikyo",  "Gunbarich",                                                             MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 2002, mjgtaste,   0,        psikyo5_mahjong, mjgtaste,  psikyosh_state, init_ps5,        ROT0,   "Psikyo",  "Mahjong G-Taste",                                                       MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
