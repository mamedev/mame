// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Williams 6809 system

    Games supported:
        * Defender
        * Mayday
        * Colony 7
        * Stargate
        * Robotron
        * Joust
        * Bubbles
        * Splat!
        * Sinistar
        * PlayBall!
        * Blaster
        * Mystic Marathon
        * Turkey Shoot
        * Inferno
        * Joust 2
        * Lotto Fun

****************************************************************************

    video-decoder proms added for Defender,Stargate,Robotron,Joust,Sinistar,
    Bubbles, etc. by HIGHWAYMAN, all decoder proms are 512x8.
    defender original pcb's used a single prom(decoder1).
    newer defender pcb's used 2 proms(decoder2,decoder3) this was to allow
    video inversion for the cocktail table, decoders 1 & 2 are the same code.
    white was first romset, then green/blue the only difference was the chips used,
    the final version was red, only red can run in a cocktail table.
    (red also has *much* improved enemy AI and is harder to play)
    Colony7 uses a different chip for decoder2(cs10), but decoder3 is the same.
    early stargate pcb's used decoder4, and decoder5.
    newer stargate pcb's and the other games on that hardware used decoder4 and decoder6
    this was probably just to fix a minor bug in decoder5.
    Blaster uses decoder4 and decoder6 and 2 other proms.
    if i could find more proms i would clean this up into a table - maybe later.

****************************************************************************

    Blitter (Stargate and Defender do not have blitter)
    ---------------------------------------------------

    CA00 start_blitter    Each bits has a function
          1000 0000 Do not process half the byte 4-7
          0100 0000 Do not process half the byte 0-3
          0010 0000 Shift the shape one pixel right (to display a shape on an odd pixel)
          0001 0000 Remap, if shape != 0 then pixel = mask
          0000 1000 Source  1 = take source 0 = take Mask only
          0000 0100 ?
          0000 0010 Transparent
          0000 0001
    CA01 blitter_mask     Not really a mask, more a remap color, see Blitter
    CA02 blitter_source   hi
    CA03 blitter_source   lo
    CA04 blitter_dest     hi
    CA05 blitter_dest     lo
    CA06 blitter_w_h      H  Do a XOR with 4 to have the real value (Except Splat)
    CA07 blitter_w_h      W  Do a XOR with 4 to have the real value (Except Splat)

    CB00 6 bits of the video counters bits 2-7

    CBFF watchdog

    CC00-CFFF 1K X 4 CMOS ram battery backed up (8 bits on Bubbles)

****************************************************************************

    Blaster Bubbles Joust Robotron Sinistar Splat Stargate
    ------------------------------------------------------

    0000-8FFF ROM   (for Blaster, 0000-3FFF is a bank of 12 ROMs)
    0000-97FF Video  RAM Bank switched with ROM (96FF for Blaster)
    9800-BFFF RAM
        0xBB00 Blaster only, Color 0 for each line (256 entry)
        0xBC00 Blaster only, Color 0 flags, latch color only if bit 0 = 1 (256 entry)
                             Erase background only if bit 1 = 1
    C000-CFFF I/O
    D000-FFFF ROM

    c000-C00F color_registers  (16 bytes of BBGGGRRR)

    c804 widget_pia_dataa (widget = I/O board)
    c805 widget_pia_ctrla
    c806 widget_pia_datab
    c807 widget_pia_ctrlb (CB2 select between player 1 and player 2
                          controls if Table or Joust)
          bits 5-3 = 110 = player 2
          bits 5-3 = 111 = player 1

    c80c rom_pia_dataa
    c80d rom_pia_ctrla
    c80e rom_pia_datab
          bits 0-5 = 6 bits to sound board
          bits 6-7 plus CA2 and CB2 = 4 bits to drive the LED 7 segment
                   Blaster only: bits 6-7 are for selecting the sound board
    c80f rom_pia_ctrlb

    C900 rom_enable_scr_ctrl  Switch between video ram and rom at 0000-97FF

    C940 Blaster only: Select bank in the color Prom for color remap
    C980 Blaster only: Select which ROM is at 0000-3FFF
    C9C0 Blaster only: bit 0 = enable the color 0 changing each lines
                       bit 1 = erase back each frame

****************************************************************************

    Robotron
    --------
    c804 widget_pia_dataa (widget = I/O board)
      bit 0  Move Up
      bit 1  Move Down
      bit 2  Move Left
      bit 3  Move Right
      bit 4  1 Player
      bit 5  2 Players
      bit 6  Fire Up
      bit 7  Fire Down

    c806 widget_pia_datab
      bit 0  Fire Left
      bit 1  Fire Right
      bit 2
      bit 3
      bit 4
      bit 5
      bit 6
      bit 7

    c80c rom_pia_dataa
      bit 0  Auto Up
      bit 1  Advance
      bit 2  Right Coin
      bit 3  High Score Reset
      bit 4  Left Coin
      bit 5  Center Coin
      bit 6  Slam Door Tilt
      bit 7  Hand Shake from sound board

****************************************************************************

    Joust
    -----
    c804 widget_pia_dataa (widget = I/O board)
      bit 0  Move Left   player 1/2
      bit 1  Move Right  player 1/2
      bit 2  Flap        player 1/2
      bit 3
      bit 4  2 Player
      bit 5  1 Players
      bit 6
      bit 7

    c806 widget_pia_datab
      bit 0
      bit 1
      bit 2
      bit 3
      bit 4
      bit 5
      bit 6
      bit 7

    c80c rom_pia_dataa
      bit 0  Auto Up
      bit 1  Advance
      bit 2  Right Coin
      bit 3  High Score Reset
      bit 4  Left Coin
      bit 5  Center Coin
      bit 6  Slam Door Tilt
      bit 7  Hand Shake from sound board

****************************************************************************

    Stargate
    --------
    c804 widget_pia_dataa (widget = I/O board)
      bit 0  Fire
      bit 1  Thrust
      bit 2  Smart Bomb
      bit 3  HyperSpace
      bit 4  2 Players
      bit 5  1 Player
      bit 6  Reverse
      bit 7  Down

    c806 widget_pia_datab
      bit 0  Up
      bit 1  Inviso
      bit 2
      bit 3
      bit 4
      bit 5
      bit 6
      bit 7  0 = Upright  1 = Table

    c80c rom_pia_dataa
      bit 0  Auto Up
      bit 1  Advance
      bit 2  Right Coin        (High Score Reset in schematics)
      bit 3  High Score Reset  (Left Coin in schematics)
      bit 4  Left Coin         (Center Coin in schematics)
      bit 5  Center Coin       (Right Coin in schematics)
      bit 6  Slam Door Tilt
      bit 7  Hand Shake from sound board

****************************************************************************

    Splat
    -----
    c804 widget_pia_dataa (widget = I/O board)
      bit 0  Walk Up
      bit 1  Walk Down
      bit 2  Walk Left
      bit 3  Walk Right
      bit 4  1 Player
      bit 5  2 Players
      bit 6  Throw Up
      bit 7  Throw Down

    c806 widget_pia_datab
      bit 0  Throw Left
      bit 1  Throw Right
      bit 2
      bit 3
      bit 4
      bit 5
      bit 6
      bit 7

    c80c rom_pia_dataa
      bit 0  Auto Up
      bit 1  Advance
      bit 2  Right Coin
      bit 3  High Score Reset
      bit 4  Left Coin
      bit 5  Center Coin
      bit 6  Slam Door Tilt
      bit 7  Hand Shake from sound board

****************************************************************************

    Blaster
    -------
    c804 widget_pia_dataa (widget = I/O board)
      bit 0  up/down switch a
      bit 1  up/down switch b
      bit 2  up/down switch c
      bit 3  up/down direction
      bit 4  left/right switch a
      bit 5  left/right switch b
      bit 6  left/right switch c
      bit 7  left/right direction

    c806 widget_pia_datab
      bit 0  Thrust (Panel)
      bit 1  Blast
      bit 2  Thrust (Joystick)
      bit 3
      bit 4  1 Player
      bit 5  2 Player
      bit 6
      bit 7

    c80c rom_pia_dataa
      bit 0  Auto Up
      bit 1  Advance
      bit 2  Right Coin
      bit 3  High Score Reset
      bit 4  Left Coin
      bit 5  Center Coin
      bit 6  Slam Door Tilt
      bit 7  Hand Shake from sound board

****************************************************************************

    Sinistar
    --------
    c804 widget_pia_dataa (widget = I/O board)
      bit 0  up/down switch a
      bit 1  up/down switch b
      bit 2  up/down switch c
      bit 3  up/down direction
      bit 4  left/right switch a
      bit 5  left/right switch b
      bit 6  left/right switch c
      bit 7  left/right direction

    c806 widget_pia_datab
      bit 0  Fire
      bit 1  Bomb
      bit 2
      bit 3
      bit 4  1 Player
      bit 5  2 Player
      bit 6
      bit 7

    c80c rom_pia_dataa
      bit 0  Auto Up
      bit 1  Advance
      bit 2  Right Coin
      bit 3  High Score Reset
      bit 4  Left Coin
      bit 5  Center Coin
      bit 6  Slam Door Tilt
      bit 7  Hand Shake from sound board

****************************************************************************

    Bubbles
    -------
    c804 widget_pia_dataa (widget = I/O board)
      bit 0  Up
      bit 1  Down
      bit 2  Left
      bit 3  Right
      bit 4  2 Players
      bit 5  1 Player
      bit 6
      bit 7

    c806 widget_pia_datab
      bit 0
      bit 1
      bit 2
      bit 3
      bit 4
      bit 5
      bit 6
      bit 7

    c80c rom_pia_dataa
      bit 0  Auto Up
      bit 1  Advance
      bit 2  Right Coin        (High Score Reset in schematics)
      bit 3  High Score Reset  (Left Coin in schematics)
      bit 4  Left Coin         (Center Coin in schematics)
      bit 5  Center Coin       (Right Coin in schematics)
      bit 6  Slam Door Tilt
      bit 7  Hand Shake from sound board

****************************************************************************

    Defender
    --------
    0000-9800 Video RAM
    C000-CFFF ROM (4 banks) + I/O
    d000-ffff ROM

    c000-c00f color_registers  (16 bytes of BBGGGRRR)

    C3FC      WatchDog

    C400-C4FF CMOS ram battery backed up

    C800      6 bits of the video counters bits 2-7

    cc00 pia1_dataa (widget = I/O board)
      bit 0  Auto Up
      bit 1  Advance
      bit 2  Right Coin
      bit 3  High Score Reset
      bit 4  Left Coin
      bit 5  Center Coin
      bit 6
      bit 7
    cc01 pia1_ctrla

    cc02 pia1_datab
      bit 0 \
      bit 1 |
      bit 2 |-6 bits to sound board
      bit 3 |
      bit 4 |
      bit 5 /
      bit 6 \
      bit 7 /Plus CA2 and CB2 = 4 bits to drive the LED 7 segment
    cc03 pia1_ctrlb (CB2 select between player 1 and player 2 controls if Table)

    cc04 pia2_dataa
      bit 0  Fire
      bit 1  Thrust
      bit 2  Smart Bomb
      bit 3  HyperSpace
      bit 4  2 Players
      bit 5  1 Player
      bit 6  Reverse
      bit 7  Down
    cc05 pia2_ctrla

    cc06 pia2_datab
      bit 0  Up
      bit 1
      bit 2
      bit 3
      bit 4
      bit 5
      bit 6
      bit 7
    cc07 pia2_ctrlb
      Control the IRQ

    d000 Select bank (c000-cfff)
      0 = I/O
      1 = BANK 1
      2 = BANK 2
      3 = BANK 3
      7 = BANK 4

****************************************************************************

    Mystic Marathon (1983)
    Turkey Shoot (1984)
    Inferno (1984)
    Joust2 Survival of the Fittest (1986)

    All have two boards, a large board with lots of RAM and
    three ROMs, and a smaller board with lots of ROMs,
    the CPU, the 6821 PIAs, and the two "Special Chip 2"
    custom BIT/BLT chips.
    Joust 2 has an additional D-11298 Williams System 11 background
    music/speech board (the older board version from PIN*BOT)
    that has a 68B09E CPU, 68B21 PIA, Harris 55564-5 CVSD, and a YM2151.

    Contact Michael Soderstrom (ichael@geocities.com) if you
    have any additional information or corrections.

    Memory Map:

    15 14 13 12  11 10  9  8   7  6  5  4   3  2  1  0
    --------------------------------------------------
     x  x  x  x   x  x  x  x   x  x  x  x   x  x  x  x  0000-BFFF   48K DRAM

     0  0  0  x   x  x  x  x   x  x  x  x   x  x  x  x  0000-1FFF   8K ROM
     0  0  1  x   x  x  x  x   x  x  x  x   x  x  x  x  2000-3FFF   8K ROM
     0  1  0  x   x  x  x  x   x  x  x  x   x  x  x  x  4000-5FFF   8K ROM
     0  1  1  x   x  x  x  x   x  x  x  x   x  x  x  x  6000-7FFF   8K ROM

     1  0  0  0   x  x  x  x   x  x  x  x   x  x  x  x  8000-8FFF   EN_COLOR* (PAGE3 only)

     0  x  x  x   x  x  x  x   x  x  x  x   x  x  x  x  0000-7FFF   OE_DRAM* (PAGE0 and read only) or:
     1  0  x  x   x  x  x  x   x  x  x  x   x  x  x  x  9000-BFFF   OE_DRAM* (!EN COLOR and read only)

     1  1  0  0   x  x  x  x   x  x  x  x   x  x  x  x  C000-CFFF   I/O:
     1  1  0  0   0  x  x  x   x  x  x  x   x  x  x  x  C000-C7FF   MAP_EN*
     1  1  0  0   1  0  0  0   0  x  x  x   x  x  x  x  C800-C87F   CS_PAGE
     1  1  0  0   1  0  0  0   1  x  x  x   x  x  x  x  C880-C87F   CS_INT* (blitter)
     1  1  0  0   1  0  0  1   0  x  x  x   x  x  x  x  C900-C97F   CS_WDOG* (data = 0x14)
     1  1  0  0   1  0  0  1   1  x  x  x   x  x  x  x  C980-C9FF   CS_PIA
     1  1  0  0   1  0  0  1   1  x  x  x   0  0  x  x  C980-C983   PIA IC5
     1  1  0  0   1  0  0  1   1  x  x  x   0  1  x  x  C984-C987   PIA IC6
     1  1  0  0   1  0  0  1   1  x  x  x   1  1  x  x  C98C        7 segment LED

     1  1  0  0   1  0  1  1   0  0  0  x   x  x  x  x  CB00-CB1F   CK_FG
     1  1  0  0   1  0  1  1   0  0  1  x   x  x  x  x  CB20-CB3F   CK_BG
     1  1  0  0   1  0  1  1   0  1  0  x   x  x  x  x  CB40-CB5F   CK_SCL
     1  1  0  0   1  0  1  1   0  1  1  x   x  x  x  x  CB60-CB7F   CK_SCH
     1  1  0  0   1  0  1  1   1  0  0  x   x  x  x  x  CB80-CB9F   FLIP clk
     1  1  0  0   1  0  1  1   1  0  1  x   x  x  x  x  CBA0-CBBF   DMA_WRINH clk

     1  1  0  0   1  0  1  1   1  1  1  0   x  x  x  x  CBE0-CBEF   EN VPOS*

     1  1  0  0   1  1  0  0   x  x  x  x   x  x  x  x  CC00-CCFF   1Kx4 CMOS RAM MEM_PROT protected
     1  1  0  0   1  1  x  x   x  x  x  x   x  x  x  x  CD00-CFFF             not MEM_PROT protected

     Mystic Marathon/Inferno:
     1  1  0  1   0  x  x  x   x  x  x  x   x  x  x  x  D000-D7FF   SRAM0*
     1  1  0  1   1  x  x  x   x  x  x  x   x  x  x  x  D800-DFFF   SRAM1*
     1  1  1  0   x  x  x  x   x  x  x  x   x  x  x  x  E000-EFFF   EXXX* 4K ROM
     1  1  1  1   x  x  x  x   x  x  x  x   x  x  x  x  F000-FFFF   FXXX* 4K ROM

     Turkey Shoot/Joust2:
     1  1  0  1   x  x  x  x   x  x  x  x   x  x  x  x  D000-DFFF   DXXX* 4K ROM
     1  1  1  0   x  x  x  x   x  x  x  x   x  x  x  x  E000-EFFF   EXXX* 4K ROM
     1  1  1  1   x  x  x  x   x  x  x  x   x  x  x  x  F000-FFFF   FXXX* 4K ROM

    6802/6808 Sound

     0  0  0  x   x  x  x  x   0  x  x  x   x  x  x  x  0000-007F   128 bytes RAM
     0  0  1  x   x  x  x  x   x  x  x  x   x  x  x  x  2000-3FFF   CS PIA IC4
     1  1  1  x   x  x  x  x   x  x  x  x   x  x  x  x  E000-FFFF   8K ROM

Reference videos: https://www.youtube.com/watch?v=R5OeC6Wc_yI
                  https://www.youtube.com/watch?v=3J_EZ1OXlww
                  https://www.youtube.com/watch?v=zxZ48iJShSU

***************************************************************************/

#include "emu.h"
#include "williams.h"

#include "machine/input_merger.h"
#include "machine/nvram.h"
#include "sound/dac.h"
#include "speaker.h"




/*************************************
 *
 *  Defender memory handlers
 *
 *************************************/

void defender_state::defender_main_map(address_map &map)
{
	map(0x0000, 0xbfff).ram().share(m_videoram);
	map(0xc000, 0xcfff).view(m_rom_view);
	m_rom_view[0](0xc000, 0xc00f).mirror(0x03e0).writeonly().share(m_paletteram);
	m_rom_view[0](0xc3ff, 0xc3ff).w(FUNC(defender_state::watchdog_reset_w));
	m_rom_view[0](0xc010, 0xc01f).mirror(0x03e0).w(FUNC(defender_state::video_control_w));
	m_rom_view[0](0xc400, 0xc4ff).mirror(0x0300).ram().w(FUNC(defender_state::cmos_4bit_w)).share("nvram");
	m_rom_view[0](0xc800, 0xcbff).r(FUNC(defender_state::video_counter_r));
	m_rom_view[0](0xcc00, 0xcc03).mirror(0x03e0).rw(m_pia[1], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	m_rom_view[0](0xcc04, 0xcc07).mirror(0x03e0).rw(m_pia[0], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xd000, 0xdfff).w(FUNC(defender_state::bank_select_w));
	map(0xd000, 0xffff).rom().region("maincpu", 0);
}

/*************************************
 *
 *  Mayday memory handlers
 *
 *************************************/

void mayday_state::mayday_main_map(address_map &map)
{
	defender_main_map(map);

	// install a handler to catch protection checks
	map(0xa190, 0xa191).r(FUNC(mayday_state::protection_r));
}


/*************************************
 *
 *  General Williams memory handlers
 *
 *************************************/

void williams_state::main_map(address_map &map)
{
	map(0x0000, 0xbfff).ram().share(m_videoram);
	map(0x0000, 0x8fff).view(m_rom_view);
	m_rom_view[0](0x0000, 0x8fff).rom().region("maincpu", 0x00000);
	map(0xc000, 0xc00f).mirror(0x03f0).writeonly().share(m_paletteram);
	map(0xc804, 0xc807).mirror(0x00f0).rw(m_pia[0], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xc80c, 0xc80f).mirror(0x00f0).rw(m_pia[1], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xc900, 0xc9ff).w(FUNC(williams_state::vram_select_w));
	map(0xca00, 0xca07).mirror(0x00f8).w(FUNC(williams_state::blitter_w));
	map(0xcb00, 0xcbff).r(FUNC(williams_state::video_counter_r));
	map(0xcbff, 0xcbff).w(FUNC(williams_state::watchdog_reset_w));
	map(0xcc00, 0xcfff).ram().w(FUNC(williams_state::cmos_4bit_w)).share("nvram");
	map(0xd000, 0xffff).rom();
}


void williams_state::sinistar_main_map(address_map &map)
{
	main_map(map);

	map(0xc900, 0xc9ff).w(FUNC(williams_state::sinistar_vram_select_w));

	map(0xd000, 0xdfff).ram();
	map(0xe000, 0xffff).rom();
}


void williams_state::bubbles_main_map(address_map &map)
{
	main_map(map);

	// bubbles has additional CMOS for a full 8 bits
	map(0xcc00, 0xcfff).ram().share("nvram");
}


void williams_state::spdball_main_map(address_map &map)
{
	main_map(map);

	// install extra input handlers
	map(0xc800, 0xc800).portr("AN0");
	map(0xc801, 0xc801).portr("AN1");
	map(0xc802, 0xc802).portr("AN2");
	map(0xc803, 0xc803).portr("AN3");

	// add a third PIA
	map(0xc808, 0xc80b).mirror(0x00f0).rw(m_pia[3], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
}


void williams_state::alienar_main_map(address_map &map)
{
	main_map(map);

	map(0xcbff, 0xcbff).nopw();
}



/*************************************
 *
 *  Blaster memory handlers
 *
 *************************************/

void blaster_state::blaster_main_map(address_map &map)
{
	map(0x0000, 0xbfff).ram().share(m_videoram);
	map(0x0000, 0x8fff).view(m_rom_view);
	m_rom_view[0](0x0000, 0x3fff).bankr(m_mainbank);
	m_rom_view[0](0x4000, 0x8fff).rom().region("maincpu", 0x04000);
	map(0xc000, 0xc00f).mirror(0x03f0).writeonly().share(m_paletteram);
	map(0xc804, 0xc807).mirror(0x00f0).rw(m_pia[0], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xc80c, 0xc80f).mirror(0x00f0).rw(m_pia[1], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xc900, 0xc93f).w(FUNC(blaster_state::blaster_vram_select_w));
	map(0xc940, 0xc97f).w(FUNC(blaster_state::remap_select_w));
	map(0xc980, 0xc9bf).w(FUNC(blaster_state::bank_select_w));
	map(0xc9c0, 0xc9ff).w(FUNC(blaster_state::video_control_w));
	map(0xca00, 0xca07).mirror(0x00f8).w(FUNC(blaster_state::blitter_w));
	map(0xcb00, 0xcbff).r(FUNC(blaster_state::video_counter_r));
	map(0xcbff, 0xcbff).w(FUNC(blaster_state::watchdog_reset_w));
	map(0xcc00, 0xcfff).ram().w(FUNC(blaster_state::cmos_4bit_w)).share("nvram");
	map(0xd000, 0xffff).rom();
}



/*************************************
 *
 *  Later Williams memory handlers
 *
 *************************************/

void williams2_state::common_map(address_map &map)
{
	map(0x0000, 0xbfff).ram().share(m_videoram);
	map(0x0000, 0x7fff).view(m_rom_view);
	m_rom_view[0](0x0000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0x87ff).view(m_palette_view);
	m_palette_view[0](0x8000, 0x87ff).ram().w(FUNC(williams2_state::paletteram_w)).share(m_paletteram);
	map(0xc000, 0xc7ff).ram().w(FUNC(williams2_state::tileram_w)).share(m_tileram);
	map(0xc800, 0xc87f).w(FUNC(williams2_state::bank_select_w));
	map(0xc880, 0xc887).mirror(0x0078).w(FUNC(williams2_state::blitter_w));
	map(0xc900, 0xc97f).w(FUNC(williams2_state::watchdog_reset_w));
	map(0xc980, 0xc983).mirror(0x0070).rw(m_pia[1], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xc984, 0xc987).mirror(0x0070).rw(m_pia[0], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xc98c, 0xc98f).mirror(0x0070).w(FUNC(williams2_state::segments_w));
	map(0xcb00, 0xcb1f).w(FUNC(williams2_state::fg_select_w));
	map(0xcb20, 0xcb3f).w(FUNC(williams2_state::bg_select_w));
	map(0xcb40, 0xcb5f).w(FUNC(williams2_state::xscroll_low_w));
	map(0xcb60, 0xcb7f).w(FUNC(williams2_state::xscroll_high_w));
	map(0xcb80, 0xcb9f).w(FUNC(williams2_state::video_control_w));
	map(0xcba0, 0xcbbf).w(FUNC(williams2_state::blit_window_enable_w));
	map(0xcbe0, 0xcbef).r(FUNC(williams2_state::video_counter_r));
	map(0xcc00, 0xcfff).ram().w(FUNC(williams2_state::cmos_4bit_w)).share("nvram");
}

// mysticm and inferno: D000-DFFF is RAM
void williams2_state::d000_ram_map(address_map &map)
{
	common_map(map);
	map(0xd000, 0xdfff).ram();
	map(0xe000, 0xffff).rom();
}

// tshoot and joust2: D000-DFFF is ROM
void williams2_state::d000_rom_map(address_map &map)
{
	common_map(map);
	map(0xd000, 0xffff).rom();
}



/*************************************
 *
 *  Sound board memory handlers
 *
 *************************************/

void defender_state::defender_sound_map(address_map &map)
{
	map(0x0000, 0x007f).ram(); // internal RAM
	map(0x0400, 0x0403).mirror(0x8000).rw(m_pia[2], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xb000, 0xffff).rom();
}

void defender_state::defender_sound_map_6802(address_map &map)
{
	// 6802 has its RAM declared internally
	map(0x0400, 0x0403).mirror(0x8000).rw(m_pia[2], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xb000, 0xffff).rom();
}


void williams_state::sound_map(address_map &map)
{
	map(0x0000, 0x007f).ram(); // internal RAM
	map(0x0080, 0x00ff).ram(); // MC6810 RAM
	map(0x0400, 0x0403).mirror(0x8000).rw(m_pia[2], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xb000, 0xffff).rom();
}

// Same as above, but for second sound board
void williams_state::sound2_map(address_map &map)
{
	map(0x0000, 0x007f).ram(); // internal RAM
	map(0x0080, 0x00ff).ram(); // MC6810 RAM
	map(0x0400, 0x0403).mirror(0x8000).rw(m_pia[3], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xb000, 0xffff).rom();
}



/*************************************
 *
 *  Later sound board memory handlers
 *
 *************************************/

void williams2_state::sound_map(address_map &map)
{
	map(0x0000, 0x007f).ram(); // internal RAM
	map(0x0080, 0x00ff).ram(); // MC6810 RAM
	map(0x2000, 0x2003).mirror(0x1ffc).rw(m_pia[2], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xe000, 0xffff).rom();
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( monitor_controls_mysticm )
	PORT_START("REDG")
	PORT_ADJUSTER( 80, "Monitor Gain Red" ) PORT_MINMAX(0, 250) PORT_CHANGED_MEMBER(DEVICE_SELF, mysticm_state, rgb_gain, 0)
	PORT_START("GREENG")
	PORT_ADJUSTER( 73, "Monitor Gain Green" ) PORT_MINMAX(0, 250) PORT_CHANGED_MEMBER(DEVICE_SELF, mysticm_state, rgb_gain, 1)
	PORT_START("BLUEG")
	PORT_ADJUSTER( 81, "Monitor Gain Blue" ) PORT_MINMAX(0, 250) PORT_CHANGED_MEMBER(DEVICE_SELF, mysticm_state, rgb_gain, 2)
	PORT_START("REDO")
	PORT_ADJUSTER( 73, "Monitor Offset Red" ) PORT_MINMAX(0, 200) PORT_CHANGED_MEMBER(DEVICE_SELF, mysticm_state, rgb_gain, 3)
	PORT_START("GREENO")
	PORT_ADJUSTER( 100, "Monitor Offset Green" ) PORT_MINMAX(0, 200) PORT_CHANGED_MEMBER(DEVICE_SELF, mysticm_state, rgb_gain, 4)
	PORT_START("BLUEO")
	PORT_ADJUSTER( 78, "Monitor Offset Blue" ) PORT_MINMAX(0, 200) PORT_CHANGED_MEMBER(DEVICE_SELF, mysticm_state, rgb_gain, 5)
INPUT_PORTS_END

static INPUT_PORTS_START( monitor_controls )
	PORT_START("REDG")
	PORT_ADJUSTER( 25, "Monitor Gain Red" ) PORT_MINMAX(0, 250) PORT_CHANGED_MEMBER(DEVICE_SELF, mysticm_state, rgb_gain, 0)
	PORT_START("GREENG")
	PORT_ADJUSTER( 25, "Monitor Gain Green" ) PORT_MINMAX(0, 250) PORT_CHANGED_MEMBER(DEVICE_SELF, mysticm_state, rgb_gain, 1)
	PORT_START("BLUEG")
	PORT_ADJUSTER( 25, "Monitor Gain Blue" ) PORT_MINMAX(0, 250) PORT_CHANGED_MEMBER(DEVICE_SELF, mysticm_state, rgb_gain, 2)
	PORT_START("REDO")
	PORT_ADJUSTER(100, "Monitor Offset Red" ) PORT_MINMAX(0, 200) PORT_CHANGED_MEMBER(DEVICE_SELF, mysticm_state, rgb_gain, 3)
	PORT_START("GREENO")
	PORT_ADJUSTER(100, "Monitor Offset Green" ) PORT_MINMAX(0, 200) PORT_CHANGED_MEMBER(DEVICE_SELF, mysticm_state, rgb_gain, 4)
	PORT_START("BLUEO")
	PORT_ADJUSTER(100, "Monitor Offset Blue" ) PORT_MINMAX(0, 200) PORT_CHANGED_MEMBER(DEVICE_SELF, mysticm_state, rgb_gain, 5)
INPUT_PORTS_END

static INPUT_PORTS_START( defender )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Fire")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Thrust")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Smart Bomb")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Hyperspace")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME(DEF_STR( Reverse ))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_2WAY

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_2WAY
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Advance")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MEMORY_RESET ) PORT_NAME("High Score Reset")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( mayday )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Fire")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Mayday")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Back Fire")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_8WAY

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE4 ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE // Default to Auto Up
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Advance") // ?
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Test Credit")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( colony7 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ))
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3")
	PORT_DIPNAME( 0x02, 0x00, "Bonus At" )
	PORT_DIPSETTING(    0x00, "20k/40k" )       PORT_CONDITION("IN2",0x01,NOTEQUALS,0x01)
	PORT_DIPSETTING(    0x02, "30k/50k" )       PORT_CONDITION("IN2",0x01,NOTEQUALS,0x01)
	PORT_DIPSETTING(    0x00, "30k/50k" )       PORT_CONDITION("IN2",0x01,EQUALS,0x01)
	PORT_DIPSETTING(    0x02, "40k/70k" )       PORT_CONDITION("IN2",0x01,EQUALS,0x01)
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown )) // documented as unused
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown )) // documented as unused
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( jin )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x07, "A 2C/1C B 1C/3C" )
	PORT_DIPSETTING(    0x06, "A 2C/1C B 1C/1C" )
	PORT_DIPSETTING(    0x05, "A 1C/3C B 1C/3C" )
	PORT_DIPSETTING(    0x04, "A 1C/3C B 1C/1C" )
	PORT_DIPSETTING(    0x03, "A 1C/2C B 1C/3C" )
	PORT_DIPSETTING(    0x02, "A 1C/2C B 1C/1C" )
	PORT_DIPSETTING(    0x01, "A 1C/1C B 1C/3C" )
	PORT_DIPSETTING(    0x00, "A 1C/1C B 1C/1C" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Level completed" )
	PORT_DIPSETTING(    0x60, "85%" )
	PORT_DIPSETTING(    0x40, "75%" )
	PORT_DIPSETTING(    0x20, "65%" )
	PORT_DIPSETTING(    0x00, "55%" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( stargate )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Fire")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Thrust")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Smart Bomb")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Hyperspace")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME(DEF_STR( Reverse ))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Inviso")
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Advance")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MEMORY_RESET ) PORT_NAME("High Score Reset")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( robotron )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_NAME("Move Up")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_NAME("Move Down")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT ) PORT_NAME("Move Left")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT ) PORT_NAME("Move Right")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_NAME("Fire Up")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_NAME("Fire Down")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT ) PORT_NAME("Fire Left")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_NAME("Fire Right")
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Advance")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MEMORY_RESET ) PORT_NAME("High Score Reset")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( joust )
	PORT_START("IN0")
	// 0x0f muxed from INP1/INP2
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	// 0xc0 muxed from INP1A/INP2A

	PORT_START("IN1")
	// 0x03 muxed from INP1A/INP2A
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Advance")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MEMORY_RESET ) PORT_NAME("High Score Reset")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("INP2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("INP1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("INP2A")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("INP1A")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( bubbles )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Advance")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MEMORY_RESET ) PORT_NAME("High Score Reset")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( conquest )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Fire")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Thrust")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(conquest_state, dial1_r)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(conquest_state, dial0_r)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DIAL")
	PORT_BIT( 0x0f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(10) PORT_KEYDELTA(16) PORT_REVERSE
INPUT_PORTS_END


static INPUT_PORTS_START( splat )
	PORT_START("IN0")
	// 0x0f muxed from INP1/INP2
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	// 0xc0 muxed from INP1A/INP2A

	PORT_START("IN1")
	// 0x03 muxed from INP1A/INP2A
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Advance")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MEMORY_RESET ) PORT_NAME("High Score Reset")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )

	PORT_START("INP2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("INP1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY PORT_PLAYER(1)

	PORT_START("INP2A")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("INP1A")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY PORT_PLAYER(1)
INPUT_PORTS_END


static INPUT_PORTS_START( sinistar )
	PORT_START("IN0")
	// pseudo analog joystick, see below

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Advance")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MEMORY_RESET ) PORT_NAME("High Score Reset")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("49WAYX") // converted by port_0_49way_r()
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("49WAYY") // converted by port_0_49way_r()
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE
INPUT_PORTS_END


static INPUT_PORTS_START( playball )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Advance")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MEMORY_RESET ) PORT_NAME("High Score Reset")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( blaster )
	PORT_START("IN0")
	// pseudo analog joystick, see below

	PORT_START("IN1")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Advance")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MEMORY_RESET ) PORT_NAME("High Score Reset")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("INP1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("INP2")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("49WAYX") // converted by port_0_49way_r()
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("49WAYY") // converted by port_0_49way_r()
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE
INPUT_PORTS_END


static INPUT_PORTS_START( blastkit )
	PORT_START("IN0")
	// pseudo analog joystick, see below

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Advance")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MEMORY_RESET ) PORT_NAME("High Score Reset")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("49WAYX") // converted by port_0_49way_r()
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("49WAYY") // converted by port_0_49way_r()
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( spdball )
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Advance")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MEMORY_RESET ) PORT_NAME("High Score Reset")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_START("AN0") // analog
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("AN1") // analog
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_PLAYER(1)

	PORT_START("AN2") // analog
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("AN3") // analog
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( alienar )
	PORT_START("IN0")
	// 0x0f muxed from INP1/INP2
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	// 0xc0 muxed from INP1A/INP2A

	PORT_START("IN1")
	// 0x03 muxed from INP1A/INP2A
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Advance")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MEMORY_RESET ) PORT_NAME("High Score Reset")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("INP2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)

	PORT_START("INP1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)

	PORT_START("INP2A")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)

	PORT_START("INP1A")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
INPUT_PORTS_END


static INPUT_PORTS_START( lottofun )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r)

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Memory Protect") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Advance")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MEMORY_RESET ) PORT_NAME("High Score Reset")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // COIN1.5? :)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) // Sound board handshake
INPUT_PORTS_END


static INPUT_PORTS_START( mysticm )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED ) // Key
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Advance")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MEMORY_RESET ) PORT_NAME("High Score Reset")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_INCLUDE(monitor_controls_mysticm)
INPUT_PORTS_END


template <int P>
ioport_value tshoot_state::gun_r()
{
	int data = m_gun[P]->read();
	return (data & 0x3f) ^ ((data & 0x3f) >> 1);
}

static INPUT_PORTS_START( tshoot )
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Grenade")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Gobble")
	PORT_BIT( 0x3c, IP_ACTIVE_HIGH, IPT_UNUSED ) // 0011-1100 output
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Advance")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MEMORY_RESET ) PORT_NAME("High Score Reset")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("INP1")
	PORT_BIT( 0x3f, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(tshoot_state, gun_r<0>)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_NAME("Fire")

	PORT_START("INP2")
	PORT_BIT( 0x3f, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(tshoot_state, gun_r<1>)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("GUNX")
	PORT_BIT( 0x3f, 0x20, IPT_AD_STICK_Y ) PORT_MINMAX(0,0x3f) PORT_SENSITIVITY(25) PORT_KEYDELTA(10)

	PORT_START("GUNY")
	PORT_BIT( 0x3f, 0x20, IPT_AD_STICK_X ) PORT_MINMAX(0,0x3f) PORT_SENSITIVITY(25) PORT_KEYDELTA(10)

	PORT_INCLUDE(monitor_controls)
INPUT_PORTS_END


static INPUT_PORTS_START( inferno )
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x3C, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Advance")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MEMORY_RESET ) PORT_NAME("High Score Reset")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("INP1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_PLAYER(1)

	PORT_START("INP2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_PLAYER(2)

	PORT_INCLUDE(monitor_controls)
INPUT_PORTS_END


static INPUT_PORTS_START( joust2 )
	PORT_START("IN0")
	// 0x0f muxed from INP1/INP2
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Start/Transform") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Start/Transform") PORT_PLAYER(1)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Advance")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MEMORY_RESET ) PORT_NAME("High Score Reset")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("INP1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Flap") PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INP2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Flap") PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_INCLUDE(monitor_controls)
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout williams2_layout =
{
	24, 16,
	RGN_FRAC(1,3),
	4,
	{ 0, 1, 2, 3 },
	{ 0+0*8+RGN_FRAC(0,3), 4+0*8+RGN_FRAC(0,3), 0+0*8+RGN_FRAC(1,3), 4+0*8+RGN_FRAC(1,3), 0+0*8+RGN_FRAC(2,3), 4+0*8+RGN_FRAC(2,3),
		0+1*8+RGN_FRAC(0,3), 4+1*8+RGN_FRAC(0,3), 0+1*8+RGN_FRAC(1,3), 4+1*8+RGN_FRAC(1,3), 0+1*8+RGN_FRAC(2,3), 4+1*8+RGN_FRAC(2,3),
		0+2*8+RGN_FRAC(0,3), 4+2*8+RGN_FRAC(0,3), 0+2*8+RGN_FRAC(1,3), 4+2*8+RGN_FRAC(1,3), 0+2*8+RGN_FRAC(2,3), 4+2*8+RGN_FRAC(2,3),
		0+3*8+RGN_FRAC(0,3), 4+3*8+RGN_FRAC(0,3), 0+3*8+RGN_FRAC(1,3), 4+3*8+RGN_FRAC(1,3), 0+3*8+RGN_FRAC(2,3), 4+3*8+RGN_FRAC(2,3)
	},
	{ STEP16(0,4*8) },
	4*16*8
};


static GFXDECODE_START( gfx_williams2 )
	GFXDECODE_ENTRY( "tiles", 0, williams2_layout, 0, 64 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static constexpr XTAL MASTER_CLOCK = (XTAL(12'000'000));
static constexpr XTAL SOUND_CLOCK = (XTAL(3'579'545));

void williams_state::williams_base(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, MASTER_CLOCK/3/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &williams_state::main_map);

	M6808(config, m_soundcpu, SOUND_CLOCK); // internal clock divider of 4, effective frequency is 894.886kHz
	m_soundcpu->set_addrmap(AS_PROGRAM, &williams_state::sound_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // 5101 (Defender), 5114 or 6514 (later games) + battery

	// set a timer to go off every 32 scanlines, to toggle the VA11 line and update the screen
	TIMER(config, "scan_timer").configure_scanline(FUNC(williams_state::va11_callback), "screen", 0, 32);

	// also set a timer to go off on scanline 240
	TIMER(config, "240_timer").configure_scanline(FUNC(williams_state::count240_callback), "screen", 0, 240);

	WATCHDOG_TIMER(config, m_watchdog);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_SCANLINE | VIDEO_ALWAYS_UPDATE);
	m_screen->set_raw(MASTER_CLOCK*2/3, 512, 6, 298, 260, 7, 247);
	m_screen->set_screen_update(FUNC(williams_state::screen_update));

	PALETTE(config, m_palette, FUNC(williams_state::palette_init), 256);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	MC1408(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25); // mc1408.ic6

	// pia
	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);

	INPUT_MERGER_ANY_HIGH(config, "soundirq").output_handler().set_inputline(m_soundcpu, M6808_IRQ_LINE);

	PIA6821(config, m_pia[0]);
	m_pia[0]->readpa_handler().set_ioport("IN0");
	m_pia[0]->readpb_handler().set_ioport("IN1");

	PIA6821(config, m_pia[1]);
	m_pia[1]->readpa_handler().set_ioport("IN2");
	m_pia[1]->writepb_handler().set(FUNC(williams_state::snd_cmd_w));
	m_pia[1]->irqa_handler().set("mainirq", FUNC(input_merger_any_high_device::in_w<0>));
	m_pia[1]->irqb_handler().set("mainirq", FUNC(input_merger_any_high_device::in_w<1>));

	PIA6821(config, m_pia[2]);
	m_pia[2]->writepa_handler().set("dac", FUNC(dac_byte_interface::data_w));
	m_pia[2]->irqa_handler().set("soundirq", FUNC(input_merger_any_high_device::in_w<0>));
	m_pia[2]->irqb_handler().set("soundirq", FUNC(input_merger_any_high_device::in_w<1>));
}

void williams_state::williams_b0(machine_config &config)
{
	williams_base(config);
	m_blitter_config = WILLIAMS_BLITTER_NONE;
	m_blitter_clip_address = 0x0000;
}

void williams_state::williams_b1(machine_config &config)
{
	williams_base(config);
	m_blitter_config = WILLIAMS_BLITTER_SC1;
	m_blitter_clip_address = 0xc000;
}

void williams_state::williams_b2(machine_config &config)
{
	williams_base(config);
	m_blitter_config = WILLIAMS_BLITTER_SC2;
	m_blitter_clip_address = 0xc000;
}


void defender_state::defender(machine_config &config)
{
	williams_b0(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &defender_state::defender_main_map);
	m_soundcpu->set_addrmap(AS_PROGRAM, &defender_state::defender_sound_map);

	m_screen->set_visarea(12, 304-1, 7, 247-1);
}

void defender_state::defender_6802snd(machine_config &config)
{
	defender(config);

	// Some bootlegs use a 6802 CPU for sound
	M6802(config.replace(), m_soundcpu, SOUND_CLOCK);
	m_soundcpu->set_addrmap(AS_PROGRAM, &defender_state::defender_sound_map_6802);
}

void defender_state::jin(machine_config &config)
{
	defender(config);

	// needs a different screen size or the credit text is clipped
	m_screen->set_visarea(0, 315, 7, 247-1);
}

void mayday_state::mayday(machine_config &config)
{
	defender(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &mayday_state::mayday_main_map);
}


void williams_state::williams_muxed(machine_config &config)
{
	// pia
	m_pia[0]->readpa_handler().set_ioport("IN0").mask(0x30);
	m_pia[0]->readpa_handler().append("mux_0", FUNC(ls157_device::output_r)).mask(0x0f);
	m_pia[0]->readpa_handler().append("mux_1", FUNC(ls157_device::output_r)).lshift(6).mask(0xc0);
	m_pia[0]->readpb_handler().set_ioport("IN1").mask(0xfc);
	m_pia[0]->readpb_handler().append("mux_1", FUNC(ls157_device::output_r)).rshift(2).mask(0x03);
	m_pia[0]->cb2_handler().set("mux_0", FUNC(ls157_device::select_w));
	m_pia[0]->cb2_handler().append("mux_1", FUNC(ls157_device::select_w));

	ls157_device &mux0(LS157(config, "mux_0", 0)); // IC3 on interface board (actually LS257 with OC tied low)
	mux0.a_in_callback().set_ioport("INP2");
	mux0.b_in_callback().set_ioport("INP1");

	ls157_device &mux1(LS157(config, "mux_1", 0)); // IC4 on interface board (actually LS257 with OC tied low)
	mux1.a_in_callback().set_ioport("INP2A");
	mux1.b_in_callback().set_ioport("INP1A");
}

void williams_state::joust(machine_config &config)
{
	williams_b1(config);
	williams_muxed(config);
}

void williams_state::splat(machine_config &config)
{
	williams_b2(config);
	williams_muxed(config);
}

void williams_state::alienar(machine_config &config)
{
	joust(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &williams_state::alienar_main_map);
}


void williams_state::bubbles(machine_config &config)
{
	williams_b1(config);

	// Bubbles: 8-bit nvram
	m_maincpu->set_addrmap(AS_PROGRAM, &williams_state::bubbles_main_map);
}

void williams_state::sinistar_upright(machine_config &config)
{
	// Sinistar: blitter window clip
	williams_b1(config);
	m_blitter_clip_address = 0x7400;

	m_maincpu->set_addrmap(AS_PROGRAM, &williams_state::sinistar_main_map);

	// pia
	m_pia[0]->readpa_handler().set(FUNC(williams_state::port_0_49way_r));

	m_pia[2]->ca2_handler().set("cvsd", FUNC(hc55516_device::digit_w));
	m_pia[2]->cb2_handler().set("cvsd", FUNC(hc55516_device::clock_w));

	// sound hardware
	HC55516(config, "cvsd", 0).add_route(ALL_OUTPUTS, "speaker", 0.8);
}

void williams_state::sinistar_cockpit(machine_config &config)
{
	sinistar_upright(config);

	// basic machine hardware
	auto &soundcpu_b(M6808(config, "soundcpu_b", SOUND_CLOCK)); // internal clock divider of 4, effective frequency is 894.886kHz
	soundcpu_b.set_addrmap(AS_PROGRAM, &williams_state::sound2_map);

	// additional sound hardware
	SPEAKER(config, "rspeaker").rear_center();
	MC1408(config, "rdac").add_route(ALL_OUTPUTS, "rspeaker", 0.25); // unknown DAC

	// pia
	INPUT_MERGER_ANY_HIGH(config, "soundirq_b").output_handler().set_inputline("soundcpu_b", M6808_IRQ_LINE);

	m_pia[1]->writepb_handler().set(FUNC(williams_state::cockpit_snd_cmd_w));

	PIA6821(config, m_pia[3]);
	m_pia[3]->writepa_handler().set("rdac", FUNC(dac_byte_interface::data_w));
	m_pia[3]->irqa_handler().set("soundirq_b", FUNC(input_merger_any_high_device::in_w<0>));
	m_pia[3]->irqb_handler().set("soundirq_b", FUNC(input_merger_any_high_device::in_w<1>));
}

void williams_state::playball(machine_config &config)
{
	williams_b1(config);

	// video hardware
	m_screen->set_visarea(6, 298-1, 8, 240-1);

	// sound hardware
	HC55516(config, "cvsd", 0).add_route(ALL_OUTPUTS, "speaker", 0.8);

	// pia
	m_pia[1]->writepb_handler().set(FUNC(williams_state::playball_snd_cmd_w));

	m_pia[2]->ca2_handler().set("cvsd", FUNC(hc55516_device::digit_w));
	m_pia[2]->cb2_handler().set("cvsd", FUNC(hc55516_device::clock_w));
}

void williams_state::spdball(machine_config &config)
{
	williams_b1(config);

	// Speed Ball: more input ports
	m_maincpu->set_addrmap(AS_PROGRAM, &williams_state::spdball_main_map);

	// pia
	PIA6821(config, m_pia[3]);
	m_pia[3]->readpa_handler().set_ioport("IN3");
	m_pia[3]->readpb_handler().set_ioport("IN4");
}

void williams_state::lottofun(machine_config &config)
{
	williams_b1(config);

	// pia
	m_pia[0]->writepb_handler().set("ticket", FUNC(ticket_dispenser_device::motor_w)).bit(7).invert();
	m_pia[0]->ca2_handler().set([this](int state) { machine().bookkeeping().coin_lockout_global_w(state); });

	TICKET_DISPENSER(config, "ticket", attotime::from_msec(70));
}


void blaster_state::blastkit(machine_config &config)
{
	williams_b2(config);
	m_blitter_clip_address = 0x9700;

	m_maincpu->set_addrmap(AS_PROGRAM, &blaster_state::blaster_main_map);

	// video hardware
	m_screen->set_screen_update(FUNC(blaster_state::screen_update));

	// pia
	m_pia[0]->readpa_handler().set(m_muxa, FUNC(ls157_x2_device::output_r));
	m_pia[0]->cb2_handler().set(m_muxa, FUNC(ls157_x2_device::select_w));

	// All multiplexers on Blaster interface board are really LS257 with OC tied to GND (which is equivalent to LS157)
	LS157_X2(config, m_muxa, 0);
	m_muxa->a_in_callback().set_ioport("IN3");
	m_muxa->b_in_callback().set(FUNC(blaster_state::port_0_49way_r));
}

void blaster_state::blaster(machine_config &config)
{
	blastkit(config);

	// basic machine hardware
	auto &soundcpu_b(M6808(config, "soundcpu_b", SOUND_CLOCK)); // internal clock divider of 4, effective frequency is 894.886kHz
	soundcpu_b.set_addrmap(AS_PROGRAM, &blaster_state::sound2_map);

	// pia
	m_pia[0]->readpb_handler().set("mux_b", FUNC(ls157_device::output_r)).mask(0x0f);
	m_pia[0]->readpb_handler().append_ioport("IN1").mask(0xf0);
	m_pia[0]->cb2_handler().append("mux_b", FUNC(ls157_device::select_w));

	// IC7 (for PA0-PA3) + IC5 (for PA4-PA7)
	m_muxa->a_in_callback().set(FUNC(blaster_state::port_0_49way_r));
	m_muxa->b_in_callback().set_ioport("IN3");

	ls157_device &muxb(LS157(config, "mux_b", 0)); // IC3
	muxb.a_in_callback().set_ioport("INP1");
	muxb.b_in_callback().set_ioport("INP2");

	INPUT_MERGER_ANY_HIGH(config, "soundirq_b").output_handler().set_inputline("soundcpu_b", M6808_IRQ_LINE);

	m_pia[1]->writepb_handler().set(FUNC(blaster_state::blaster_snd_cmd_w));
	m_pia[2]->writepa_handler().set("ldac", FUNC(dac_byte_interface::data_w));

	PIA6821(config, m_pia[3]);
	m_pia[3]->writepa_handler().set("rdac", FUNC(dac_byte_interface::data_w));
	m_pia[3]->irqa_handler().set("soundirq_b", FUNC(input_merger_any_high_device::in_w<0>));
	m_pia[3]->irqb_handler().set("soundirq_b", FUNC(input_merger_any_high_device::in_w<1>));

	// sound hardware
	config.device_remove("speaker");
	config.device_remove("dac");

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	MC1408(config, "ldac", 0).add_route(ALL_OUTPUTS, "lspeaker", 0.25); // unknown DAC
	MC1408(config, "rdac", 0).add_route(ALL_OUTPUTS, "rspeaker", 0.25); // unknown DAC
}


void williams2_state::williams2_base(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, MASTER_CLOCK/3/4);

	M6808(config, m_soundcpu, MASTER_CLOCK/3); // yes, this is different from the older games
	m_soundcpu->set_addrmap(AS_PROGRAM, &williams2_state::sound_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // 5114 + battery

	// set a timer to go off every 32 scanlines, to toggle the VA11 line and update the screen
	TIMER(config, "scan_timer").configure_scanline(FUNC(williams2_state::va11_callback), "screen", 0, 32);

	// also set a timer to go off on scanline 254
	TIMER(config, "254_timer").configure_scanline(FUNC(williams2_state::endscreen_callback), "screen", 8, 246);

	WATCHDOG_TIMER(config, m_watchdog);

	// video hardware
	PALETTE(config, m_palette).set_entries(1024);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_williams2);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_SCANLINE | VIDEO_ALWAYS_UPDATE);
	m_screen->set_raw(MASTER_CLOCK*2/3, 512, 8, 284, 260, 8, 248);
	m_screen->set_screen_update(FUNC(williams2_state::screen_update));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	MC1408(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // unknown DAC

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);
	INPUT_MERGER_ANY_HIGH(config, "soundirq").output_handler().set_inputline(m_soundcpu, M6808_IRQ_LINE);

	// pia
	PIA6821(config, m_pia[0]);
	m_pia[0]->readpa_handler().set_ioport("IN0");
	m_pia[0]->readpb_handler().set_ioport("IN1");

	PIA6821(config, m_pia[1]);
	m_pia[1]->readpa_handler().set_ioport("IN2");
	m_pia[1]->writepb_handler().set(FUNC(williams2_state::snd_cmd_w));
	m_pia[1]->cb2_handler().set(m_pia[2], FUNC(pia6821_device::ca1_w));
	m_pia[1]->irqa_handler().set("mainirq", FUNC(input_merger_any_high_device::in_w<0>));
	m_pia[1]->irqb_handler().set("mainirq", FUNC(input_merger_any_high_device::in_w<1>));

	PIA6821(config, m_pia[2]);
	m_pia[2]->writepa_handler().set(m_pia[1], FUNC(pia6821_device::portb_w));
	m_pia[2]->writepb_handler().set("dac", FUNC(dac_byte_interface::data_w));
	m_pia[2]->ca2_handler().set(m_pia[1], FUNC(pia6821_device::cb1_w));
	m_pia[2]->irqa_handler().set("soundirq", FUNC(input_merger_any_high_device::in_w<0>));
	m_pia[2]->irqb_handler().set("soundirq", FUNC(input_merger_any_high_device::in_w<1>));

	m_blitter_config = WILLIAMS_BLITTER_SC2;
	m_blitter_clip_address = 0x9000;
}


void williams2_state::inferno(machine_config &config)
{
	williams2_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &williams2_state::d000_ram_map);

	// pia
	m_pia[0]->readpa_handler().set("mux", FUNC(ls157_x2_device::output_r));
	m_pia[0]->ca2_handler().set("mux", FUNC(ls157_x2_device::select_w));

	m_pia[2]->set_port_a_input_overrides_output_mask(0xff);

	ls157_x2_device &mux(LS157_X2(config, "mux", 0)); // IC45 (for PA4-PA7) + IC46 (for PA0-PA3) on CPU board
	mux.a_in_callback().set_ioport("INP1");
	mux.b_in_callback().set_ioport("INP2");
}

void mysticm_state::mysticm(machine_config &config)
{
	williams2_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &mysticm_state::d000_ram_map);

	m_screen->set_raw(MASTER_CLOCK*2/3, 512, 8, 284, 256, 8, 248);
	m_screen->set_screen_update(FUNC(mysticm_state::screen_update));

	// pia
	m_pia[0]->irqa_handler().set_inputline("maincpu", M6809_FIRQ_LINE);
	m_pia[0]->irqb_handler().set("mainirq", FUNC(input_merger_any_high_device::in_w<0>));

	m_pia[1]->irqa_handler().set("mainirq", FUNC(input_merger_any_high_device::in_w<1>));
	m_pia[1]->irqb_handler().set("mainirq", FUNC(input_merger_any_high_device::in_w<2>));
}

void tshoot_state::tshoot(machine_config &config)
{
	williams2_base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &tshoot_state::d000_rom_map);

	// pia
	m_pia[0]->readpa_handler().set(m_mux, FUNC(ls157_x2_device::output_r));
	m_pia[0]->writepb_handler().set(FUNC(tshoot_state::lamp_w));
	m_pia[0]->ca2_handler().set(m_mux, FUNC(ls157_x2_device::select_w));
	m_pia[0]->irqa_handler().set("mainirq", FUNC(input_merger_any_high_device::in_w<0>));
	m_pia[0]->irqb_handler().set("mainirq", FUNC(input_merger_any_high_device::in_w<1>));

	m_pia[1]->irqa_handler().set("mainirq", FUNC(input_merger_any_high_device::in_w<2>));
	m_pia[1]->irqb_handler().set("mainirq", FUNC(input_merger_any_high_device::in_w<3>));

	m_pia[2]->cb2_handler().set(FUNC(tshoot_state::maxvol_w));

	LS157_X2(config, m_mux, 0); // U2 + U3 on interface board
	m_mux->a_in_callback().set_ioport("INP1");
	m_mux->b_in_callback().set_ioport("INP2");
}

void joust2_state::joust2(machine_config &config)
{
	williams2_base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &joust2_state::d000_rom_map);

	S11_OBG(config, m_bg).add_route(ALL_OUTPUTS, "speaker", 2.0); // D-11298-3035 'pinbot style' older BG sound board
	// Jumpers for the board: W1=? W2=open W3=present W4=open W5=open W6=open W7=present

	// pia
	m_pia[0]->readpa_handler().set_ioport("IN0").mask(0xf0);
	m_pia[0]->readpa_handler().append(m_mux, FUNC(ls157_device::output_r)).mask(0x0f);
	m_pia[0]->ca2_handler().set(m_mux, FUNC(ls157_device::select_w));

	m_pia[1]->readpa_handler().set_ioport("IN2");
	m_pia[1]->writepb_handler().set(FUNC(joust2_state::snd_cmd_w)); // this goes both to the sound cpu AND to the s11 bg cpu
	m_pia[1]->ca2_handler().set(FUNC(joust2_state::pia_s11_bg_strobe_w));
	m_pia[1]->cb2_handler().set(m_pia[2], FUNC(pia6821_device::ca1_w));
	m_pia[1]->irqa_handler().set("mainirq", FUNC(input_merger_any_high_device::in_w<0>));
	m_pia[1]->irqb_handler().set("mainirq", FUNC(input_merger_any_high_device::in_w<1>));

	// these (and ca2 above) are educated guesses, as we have no schematics for joust 2's pcb which has the 20 pin system 11 bg sound connector on it;
	// inferno, which we have schematics to, lacks this connector. All of pia[1] ca2, pia[2] cb1, and pia[2] cb2 are unconnected/grounded on inferno.
	m_bg->cb2_cb().set(m_pia[2], FUNC(pia6821_device::cb1_w));
	m_pia[2]->cb2_handler().set(m_bg, FUNC(s11_obg_device::resetq_w)); // inverted?

	LS157(config, m_mux, 0);
	m_mux->a_in_callback().set_ioport("INP1");
	m_mux->b_in_callback().set_ioport("INP2");
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( defender )
	ROM_REGION( 0x3000, "maincpu", ROMREGION_BE )
	ROM_LOAD( "defend.1",     0x0000, 0x0800, CRC(c3e52d7e) SHA1(a57f5278ffe44248fc73f9925d107f4024ad981a) )
	ROM_LOAD( "defend.4",     0x0800, 0x0800, CRC(9a72348b) SHA1(ed6ce796702ff32209ced3cb1ba3837dbafa526f) )
	ROM_LOAD( "defend.2",     0x1000, 0x1000, CRC(89b75984) SHA1(a9481478da38f99efb67f0ecf82d084e14b93b42) )
	ROM_LOAD( "defend.3",     0x2000, 0x1000, CRC(94f51e9b) SHA1(a24cfc55de56a72758c76fe2a55f1ec6c353b16f) )

	ROM_REGION( 0x7000, "banked", ROMREGION_BE )
	ROM_LOAD( "defend.9",     0x0000, 0x0800, CRC(6870e8a5) SHA1(67ccc194b1753a18af0c85f5e603355549c4f727) )
	ROM_LOAD( "defend.12",    0x0800, 0x0800, CRC(f1f88938) SHA1(26e48dfeefa0766837b1e762695b9532dbc8bc5e) )
	ROM_LOAD( "defend.8",     0x1000, 0x0800, CRC(b649e306) SHA1(9d7bc3c89e5a53c575946f06702c722b864b1ff0) )
	ROM_LOAD( "defend.11",    0x1800, 0x0800, CRC(9deaf6d9) SHA1(59b018ba0f3fe6eadfd387dc180ac281460358bc) )
	ROM_LOAD( "defend.7",     0x2000, 0x0800, CRC(339e092e) SHA1(2f89951dbe55d80df43df8dcf497171f73e726d3) )
	ROM_LOAD( "defend.10",    0x2800, 0x0800, CRC(a543b167) SHA1(9292b94b0d74e57e03aada4852ad1997c34122ff) )
	ROM_LOAD( "defend.6",     0x6000, 0x0800, CRC(65f4efd1) SHA1(a960fd1559ed74b81deba434391e49fc6ec389ca) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "video_sound_rom_1.ic12", 0xf800, 0x0800, CRC(fefd5b48) SHA1(ceb0d18483f0691978c604db94417e6941ad7ff2) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder.2",   0x0000, 0x0200, CRC(8dd98da5) SHA1(da979604f7a2aa8b5a6d4a5debd2e80f77569e35) )
	ROM_LOAD( "decoder.3",   0x0200, 0x0200, CRC(c3f45f70) SHA1(d19036cbc46b130548873597b44b8b70758f25c4) )
ROM_END

ROM_START( defenderg )
	ROM_REGION( 0x3000, "maincpu", ROMREGION_BE )
	ROM_LOAD( "defeng01.bin", 0x0000, 0x0800, CRC(6111d74d) SHA1(2a335bdce8269f75012df44b446cb261ddd5924c) )
	ROM_LOAD( "defeng04.bin", 0x0800, 0x0800, CRC(3cfc04ce) SHA1(8ee65c7daed4d6956d0e15ada4dc414c28376012) )
	ROM_LOAD( "defeng02.bin", 0x1000, 0x1000, CRC(d184ab6b) SHA1(ed61a95b04f6162aedba8a72bc46005b77283955) )
	ROM_LOAD( "defeng03.bin", 0x2000, 0x1000, CRC(788b76d7) SHA1(92987207770a870b5be61c820e9e229801f1fa7a) )

	ROM_REGION( 0x7000, "banked", ROMREGION_BE )
	ROM_LOAD( "defeng09.bin", 0x0000, 0x0800, CRC(f57caa62) SHA1(c8c91b96fd3bc98eddcc1503159050dae5755001) )
	ROM_LOAD( "defeng12.bin", 0x0800, 0x0800, CRC(33db686f) SHA1(34bc7fa10b7996efcc53d3a891b2983874269828) )
	ROM_LOAD( "defeng08.bin", 0x1000, 0x0800, CRC(9a9eb3d2) SHA1(306a3a24931e1aa5fcfd71e3f117cc726d0920ac) )
	ROM_LOAD( "defeng11.bin", 0x1800, 0x0800, CRC(5ca4e860) SHA1(031188c009b8fca92703a0cc0c2bb44976212ae9) )
	ROM_LOAD( "defeng07.bin", 0x2000, 0x0800, CRC(545c3326) SHA1(98199df5206c261061b0108c68ab9128fa0779eb) )
	ROM_LOAD( "defeng10.bin", 0x2800, 0x0800, CRC(941cf34e) SHA1(411dcb18b67585982672ff687a9249f4890faa1b) )
	ROM_LOAD( "defeng06.bin", 0x6000, 0x0800, CRC(3af34c05) SHA1(71f3ced06a373fa4805c856bd9fc97760787a920) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "video_sound_rom_1.ic12", 0xf800, 0x0800, CRC(fefd5b48) SHA1(ceb0d18483f0691978c604db94417e6941ad7ff2) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "decoder.1",   0x0000, 0x0200, CRC(8dd98da5) SHA1(da979604f7a2aa8b5a6d4a5debd2e80f77569e35) )
ROM_END

ROM_START( defenderb )
	ROM_REGION( 0x3000, "maincpu", ROMREGION_BE )
	ROM_LOAD( "wb01.bin",     0x0000, 0x1000, CRC(0ee1019d) SHA1(a76247e825b8267abfd195c12f96348fe10d4cbc) )
	ROM_LOAD( "defeng02.bin", 0x1000, 0x1000, CRC(d184ab6b) SHA1(ed61a95b04f6162aedba8a72bc46005b77283955) )
	ROM_LOAD( "wb03.bin",     0x2000, 0x1000, CRC(a732d649) SHA1(b681882c02c5870ad613edc77255969a5f796422) )

	ROM_REGION( 0x7000, "banked", ROMREGION_BE )
	ROM_LOAD( "defeng09.bin", 0x0000, 0x0800, CRC(f57caa62) SHA1(c8c91b96fd3bc98eddcc1503159050dae5755001) )
	ROM_LOAD( "defeng12.bin", 0x0800, 0x0800, CRC(33db686f) SHA1(34bc7fa10b7996efcc53d3a891b2983874269828) )
	ROM_LOAD( "defeng08.bin", 0x1000, 0x0800, CRC(9a9eb3d2) SHA1(306a3a24931e1aa5fcfd71e3f117cc726d0920ac) )
	ROM_LOAD( "defeng11.bin", 0x1800, 0x0800, CRC(5ca4e860) SHA1(031188c009b8fca92703a0cc0c2bb44976212ae9) )
	ROM_LOAD( "defeng07.bin", 0x2000, 0x0800, CRC(545c3326) SHA1(98199df5206c261061b0108c68ab9128fa0779eb) )
	ROM_LOAD( "defeng10.bin", 0x2800, 0x0800, CRC(941cf34e) SHA1(411dcb18b67585982672ff687a9249f4890faa1b) )
	ROM_LOAD( "defeng06.bin", 0x6000, 0x0800, CRC(3af34c05) SHA1(71f3ced06a373fa4805c856bd9fc97760787a920) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "video_sound_rom_1.ic12", 0xf800, 0x0800, CRC(fefd5b48) SHA1(ceb0d18483f0691978c604db94417e6941ad7ff2) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "decoder.1",   0x0000, 0x0200, CRC(8dd98da5) SHA1(da979604f7a2aa8b5a6d4a5debd2e80f77569e35) )
ROM_END

// the white set seems to be the source of the defcmnd & startrkd bootlegs
ROM_START( defenderw )
	ROM_REGION( 0x3000, "maincpu", ROMREGION_BE )
	ROM_LOAD( "rom1.bin",     0x0000, 0x1000, CRC(5af871e3) SHA1(f9a42619b37db2eb07d0302ac9d0ff5c1923c21d) )
	ROM_LOAD( "rom2.bin",     0x1000, 0x1000, CRC(1126adc9) SHA1(526cf1ca3a7eefd6115d74ac9af1a50774cc258e) )
	ROM_LOAD( "rom3.bin",     0x2000, 0x1000, CRC(4097b46b) SHA1(8f506dc59b129c9441d813062fc38747619678db) )

	ROM_REGION( 0x7000, "banked", ROMREGION_BE )
	ROM_LOAD( "rom9.bin",     0x0000, 0x0800, CRC(93012991) SHA1(9e06ed4a489b2ed063f83b708d3e7c6a02e45389) )
	ROM_LOAD( "rom12.bin",    0x0800, 0x0800, CRC(4bdd8dc4) SHA1(e7503e68608e8f7bb066c99e1e32c6fe060c1dd3) )
	ROM_LOAD( "rom8.bin",     0x1000, 0x0800, CRC(5227fc0b) SHA1(1e6fd398b5beef0be58667f1f0a789a76edd5eb9) )
	ROM_LOAD( "rom11.bin",    0x1800, 0x0800, CRC(d068f0c5) SHA1(d32a4232756ca05972780cb35b0add12b31e8283) )
	ROM_LOAD( "rom7.bin",     0x2000, 0x0800, CRC(fef4cb77) SHA1(96202e97f3392bc043a252e78d1c42b51c38d269) )
	ROM_LOAD( "rom10.bin",    0x2800, 0x0800, CRC(49b50b40) SHA1(91cf841271a2f7d06f81477b4a450eb4580c7ca5) ) // hand-repaired with startrkd rom, but should be good
	ROM_LOAD( "rom6.bin",     0x6000, 0x0800, CRC(43d42a1b) SHA1(b13d59940646451c00b49bbe4a41b9e2df4d7758) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "video_sound_rom_1.ic12", 0xf800, 0x0800, CRC(fefd5b48) SHA1(ceb0d18483f0691978c604db94417e6941ad7ff2) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "decoder.1",   0x0000, 0x0200, CRC(8dd98da5) SHA1(da979604f7a2aa8b5a6d4a5debd2e80f77569e35) )
ROM_END

ROM_START( defenderj )
	ROM_REGION( 0x3000, "maincpu", ROMREGION_BE )
	ROM_LOAD( "df1-1.e3",     0x0000, 0x1000, CRC(8c04602b) SHA1(a8ed5afd0b276cebb479b1717666eaabbf75c6a5) ) //2532
	ROM_LOAD( "df2-1.e2",     0x1000, 0x1000, CRC(89b75984) SHA1(a9481478da38f99efb67f0ecf82d084e14b93b42) ) //2532
	ROM_LOAD( "df3-1.e1",     0x2000, 0x1000, CRC(94f51e9b) SHA1(a24cfc55de56a72758c76fe2a55f1ec6c353b16f) ) //2532

	ROM_REGION( 0x7000, "banked", ROMREGION_BE )
	ROM_LOAD( "df10-1.a1",    0x0000, 0x0800, CRC(12e2bd1c) SHA1(c2fdf2fced003a0acf037aa6fab141b04c1c81bd) ) //2716
	ROM_LOAD( "df7-1.b1",     0x0800, 0x0800, CRC(f1f88938) SHA1(26e48dfeefa0766837b1e762695b9532dbc8bc5e) ) //2716
	ROM_LOAD( "df9-1.a2",     0x1000, 0x0800, CRC(b649e306) SHA1(9d7bc3c89e5a53c575946f06702c722b864b1ff0) ) //2716
	ROM_LOAD( "df6-1.b2",     0x1800, 0x0800, CRC(9deaf6d9) SHA1(59b018ba0f3fe6eadfd387dc180ac281460358bc) ) //2716
	ROM_LOAD( "df8-1.a3",     0x2000, 0x0800, CRC(339e092e) SHA1(2f89951dbe55d80df43df8dcf497171f73e726d3) ) //2716
	ROM_LOAD( "df5-1.b3",     0x2800, 0x0800, CRC(a543b167) SHA1(9292b94b0d74e57e03aada4852ad1997c34122ff) ) //2716
	ROM_LOAD( "df4-1.c1",     0x6000, 0x0800, CRC(65f4efd1) SHA1(a960fd1559ed74b81deba434391e49fc6ec389ca) ) //2716

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "dr12.i3",   0xf800, 0x0800, CRC(f122d9c9) SHA1(70092fc354a2efbe7365be922fa36309b50d5c6f) ) //2716

	ROM_REGION( 0x0400, "proms", 0 ) // Same as Colony 7 with different labels. The type is NEC B425, equivalent to 82S141 and 74S474
	ROM_LOAD( "df11.k3", 0x0000, 0x0200, CRC(25de5d85) SHA1(826f78c2fe847f594d261c280dd10b9e776bf4fd) )
	ROM_LOAD( "df12.f3", 0x0200, 0x0200, CRC(c3f45f70) SHA1(d19036cbc46b130548873597b44b8b70758f25c4) )
ROM_END

ROM_START( defndjeu )
	ROM_REGION( 0x3000, "maincpu", ROMREGION_BE )
	ROM_LOAD( "15", 0x0000, 0x1000, CRC(706a24bd) SHA1(60cef3d4f7204eff42de2c08244863e83bc842b4) )
	ROM_LOAD( "16", 0x1000, 0x1000, CRC(03201532) SHA1(77e8c10ba0ecb6e7a7cb4229a5025c4b9ea4c73e) )
	ROM_LOAD( "17", 0x2000, 0x1000, CRC(25287eca) SHA1(ec81181a5a0ac2adf7c0dabbec638f886c13e6ec) )

	ROM_REGION( 0x7000, "banked", ROMREGION_BE )
	ROM_LOAD( "21", 0x0000, 0x1000, CRC(bddb71a3) SHA1(ecba4c09a9d59fd7aa02efa240461df89159d2ec) )
	ROM_LOAD( "20", 0x1000, 0x1000, BAD_DUMP CRC(12fa0788) SHA1(7464386521c9db0153caf1ea05a353f0018651e5) )
	ROM_LOAD( "19", 0x2000, 0x1000, CRC(769f5984) SHA1(0ea49754b45bc214fd2b69846ede738994f07ee3) )
	ROM_LOAD( "18", 0x6000, 0x1000, CRC(e99d5679) SHA1(b4344a32aed6cc64284661c03993a59718289c82) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "s", 0xf800, 0x0800, CRC(cb79ae42) SHA1(d22bef68ef62aa012f1919338a33621138c2278b) )
ROM_END

ROM_START( tornado1 )
	ROM_REGION( 0x3000, "maincpu", ROMREGION_BE )
	ROM_LOAD( "torna1.bin", 0x0000, 0x1000, CRC(706a24bd) SHA1(60cef3d4f7204eff42de2c08244863e83bc842b4) ) // same as defndjeu 15
	ROM_LOAD( "torna3.bin", 0x1000, 0x1000, CRC(a79213b2) SHA1(ff9a59cb1d28af396a7b93df82a4bb825581bcce) )
	ROM_LOAD( "torna4.bin", 0x2000, 0x1000, CRC(f96d3d26) SHA1(42a1add3c39c1376aa3292c9b85346bb480d329d) )

	ROM_REGION( 0x7000, "banked", ROMREGION_BE )
	ROM_LOAD( "tornc4.bin", 0x0000, 0x1000, CRC(e30f4c00) SHA1(afdf1877ae52bc027c4cd4a31b861aa50321a094) )
	ROM_LOAD( "tornb3.bin", 0x1000, 0x1000, CRC(0e3fef55) SHA1(d0350c36971b523aa490b29b64345f3777019a8d) )
	ROM_LOAD( "tornb1.bin", 0x2000, 0x1000, CRC(f2bef850) SHA1(78e50c790e3a8ebc4b65f2d827be8bd375bf4ef4) )
	ROM_LOAD( "tornb4.bin", 0x6000, 0x1000, CRC(e99d5679) SHA1(b4344a32aed6cc64284661c03993a59718289c82) ) // same as defndjeu 18

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "tornb6.bin", 0xf800, 0x0800, CRC(3685e033) SHA1(74d16675d9c16a6ea3af09cfbc20c3d6b0ab2311) )
	ROM_CONTINUE ( 0x000, 0x800 ) // cut rom instead?
ROM_END

// I suspect this one is a bad dump
ROM_START( tornado2 )
	ROM_REGION( 0x3000, "maincpu", ROMREGION_BE )
	ROM_LOAD( "tto15.bin", 0x0000, 0x1000, BAD_DUMP CRC(910ac603) SHA1(33705b5ce4242a5865a8b4ecf27aa9e656067ea3) ) //  1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "to16.bin",  0x1000, 0x1000, BAD_DUMP CRC(46ccd582) SHA1(4f74edb2c48273fce0531b7de54fd7eb925bf3df) )
	ROM_LOAD( "tto17.bin", 0x2000, 0x1000, BAD_DUMP CRC(faa3613c) SHA1(79014de2021cd73525ecfe0e8d3e7da25e9ee1f3) )

	ROM_REGION( 0x7000, "banked", ROMREGION_BE )
	ROM_LOAD( "to21.bin",  0x0000, 0x1000, CRC(e30f4c00) SHA1(afdf1877ae52bc027c4cd4a31b861aa50321a094) ) // same as tornado1 tornc4.bin
	ROM_LOAD( "to20.bin",  0x1000, 0x1000, CRC(e90bdcb2) SHA1(591fbad03bfaf371a865d81982c03402acdf0421) ) //  1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "to19.bin",  0x2000, 0x1000, CRC(42885b4f) SHA1(759319f84ea2cec6808d744d190062a7105d7efc) )
	ROM_LOAD( "to18.bin",  0x6000, 0x1000, CRC(c15ffc03) SHA1(9093a8079b21484a999c965346accdbed91d0273) ) //  1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "to_s.bin",  0xf800, 0x0800, CRC(cb79ae42) SHA1(d22bef68ef62aa012f1919338a33621138c2278b) ) // same as defndjeu s
ROM_END

ROM_START( zero )
	ROM_REGION( 0x3000, "maincpu", ROMREGION_BE )
	ROM_LOAD( "zero-15", 0x0000, 0x1000, CRC(706a24bd) SHA1(60cef3d4f7204eff42de2c08244863e83bc842b4) )
	ROM_LOAD( "zero-16", 0x1000, 0x1000, CRC(a79213b2) SHA1(ff9a59cb1d28af396a7b93df82a4bb825581bcce) )
	ROM_LOAD( "zero-17", 0x2000, 0x1000, CRC(25287eca) SHA1(ec81181a5a0ac2adf7c0dabbec638f886c13e6ec) )

	ROM_REGION( 0x7000, "banked", ROMREGION_BE )
	ROM_LOAD( "zero-21", 0x0000, 0x1000, CRC(7ca35cfd) SHA1(9afa4f6641082f73bfc3e2b800acf82dcc2bafb3) )
	ROM_LOAD( "zero-20", 0x1000, 0x1000, CRC(0757967f) SHA1(445e399f1fc834a9333549440171beb6e19a24a7) )
	ROM_LOAD( "zero-19", 0x2000, 0x1000, CRC(f2bef850) SHA1(78e50c790e3a8ebc4b65f2d827be8bd375bf4ef4) )
	ROM_LOAD( "zero-18", 0x6000, 0x1000, CRC(e99d5679) SHA1(b4344a32aed6cc64284661c03993a59718289c82) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "video_sound_rom_1.ic12", 0xf800, 0x0800, CRC(fefd5b48) SHA1(ceb0d18483f0691978c604db94417e6941ad7ff2) )
ROM_END

ROM_START( zero2 )
	ROM_REGION( 0x3000, "maincpu", ROMREGION_BE )
	ROM_LOAD( "15me.1a", 0x0000, 0x1000, CRC(9323eee5) SHA1(727eaf669162a39c763203b925ab6b2cdb4626c1) )
	ROM_LOAD( "to16.3a", 0x1000, 0x1000, CRC(a79213b2) SHA1(ff9a59cb1d28af396a7b93df82a4bb825581bcce) )
	ROM_LOAD( "17m5.4a", 0x2000, 0x1000, CRC(16a3c0dd) SHA1(df11e4c7a62db85638d1fdda3602735b0d3f7f80) )

	ROM_REGION( 0x7000, "banked", ROMREGION_BE )
	ROM_LOAD( "21.4c",   0x0000, 0x1000, CRC(7ca35cfd) SHA1(9afa4f6641082f73bfc3e2b800acf82dcc2bafb3) )
	ROM_LOAD( "20m5.3b", 0x1000, 0x1000, CRC(7473955b) SHA1(b27515ca3fd2f938706415425226c4d7113eb276) )
	ROM_LOAD( "to19.1b", 0x2000, 0x1000, CRC(f2bef850) SHA1(78e50c790e3a8ebc4b65f2d827be8bd375bf4ef4) )
	ROM_LOAD( "18m5.4b", 0x6000, 0x1000, CRC(7e4afe43) SHA1(7a4f2128e48ed4d5e83885a3a72756822769fe5d) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "to4.6b",   0xf800, 0x0800, CRC(cb79ae42) SHA1(d22bef68ef62aa012f1919338a33621138c2278b) )
ROM_END

ROM_START( defcmnd )
	ROM_REGION( 0x3000, "maincpu", ROMREGION_BE )
	ROM_LOAD( "defcmnda.1",   0x0000, 0x1000, CRC(68effc1d) SHA1(459fd95cdf94233e1a4302d1c166e0f7cc239579) )
	ROM_LOAD( "defcmnda.2",   0x1000, 0x1000, CRC(1126adc9) SHA1(526cf1ca3a7eefd6115d74ac9af1a50774cc258e) )
	ROM_LOAD( "defcmnda.3",   0x2000, 0x1000, CRC(7340209d) SHA1(d2cdab8ac4830ac027655ed7fe54314c5b87fdb3) )

	ROM_REGION( 0x7000, "banked", ROMREGION_BE )
	ROM_LOAD( "defcmnda.10",  0x0000, 0x0800, CRC(3dddae75) SHA1(f45fcf3e5ca9bf3edd692b4ee1e96f9f1d388522) )
	ROM_LOAD( "defcmnda.7",   0x0800, 0x0800, CRC(3f1e7cf8) SHA1(87afb4b1158e64039129bd8a9653bc61ab3e1e37) )
	ROM_LOAD( "defcmnda.9",   0x1000, 0x0800, CRC(8882e1ff) SHA1(9d54a39230acd01e0555f67ba2a3c9c6d66b59a1) )
	ROM_LOAD( "defcmnda.6",   0x1800, 0x0800, CRC(d068f0c5) SHA1(d32a4232756ca05972780cb35b0add12b31e8283) )
	ROM_LOAD( "defcmnda.8",   0x2000, 0x0800, CRC(fef4cb77) SHA1(96202e97f3392bc043a252e78d1c42b51c38d269) )
	ROM_LOAD( "defcmnda.5",   0x2800, 0x0800, CRC(49b50b40) SHA1(91cf841271a2f7d06f81477b4a450eb4580c7ca5) )
	ROM_LOAD( "defcmnda.4",   0x6000, 0x0800, CRC(43d42a1b) SHA1(b13d59940646451c00b49bbe4a41b9e2df4d7758) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "defcmnda.snd", 0xf800, 0x0800, CRC(f122d9c9) SHA1(70092fc354a2efbe7365be922fa36309b50d5c6f) )
ROM_END

ROM_START( startrkd )
	ROM_REGION( 0x3000, "maincpu", ROMREGION_BE )
	ROM_LOAD( "st_rom8.bin",     0x0000, 0x1000, CRC(5af871e3) SHA1(f9a42619b37db2eb07d0302ac9d0ff5c1923c21d) )
	ROM_LOAD( "st_rom9.bin",     0x1000, 0x1000, CRC(1126adc9) SHA1(526cf1ca3a7eefd6115d74ac9af1a50774cc258e) )
	ROM_LOAD( "st_rom10.bin",    0x2000, 0x1000, CRC(4097b46b) SHA1(8f506dc59b129c9441d813062fc38747619678db) )

	ROM_REGION( 0x7000, "banked", ROMREGION_BE )
	ROM_LOAD( "st_rom6.bin",     0x0000, 0x0800, CRC(93012991) SHA1(9e06ed4a489b2ed063f83b708d3e7c6a02e45389) )
	ROM_LOAD( "st_rom5.bin",     0x0800, 0x0800, CRC(c6f0c004) SHA1(57c547b804ad3eceb33a9390bbffcfc0b63f16dd) )
	ROM_LOAD( "st_rom4.bin",     0x1000, 0x0800, CRC(b48430bf) SHA1(6572812f3a1e6eede3dff3273f16846799e79ed9) )
	ROM_LOAD( "st_rom3.bin",     0x1800, 0x0800, CRC(d068f0c5) SHA1(d32a4232756ca05972780cb35b0add12b31e8283) )
	ROM_LOAD( "st_rom2.bin",     0x2000, 0x0800, CRC(fef4cb77) SHA1(96202e97f3392bc043a252e78d1c42b51c38d269) )
	ROM_LOAD( "st_rom1.bin",     0x2800, 0x0800, CRC(d23d6cdb) SHA1(2ce9fe269598919f994339c8f8685d6a79e417d8) )
	ROM_LOAD( "st_rom7.bin",     0x6000, 0x0800, CRC(43d42a1b) SHA1(b13d59940646451c00b49bbe4a41b9e2df4d7758) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "video_sound_rom_1.ic12", 0xf800, 0x0800, CRC(fefd5b48) SHA1(ceb0d18483f0691978c604db94417e6941ad7ff2) )
ROM_END

ROM_START( defence )
	ROM_REGION( 0x3000, "maincpu", ROMREGION_BE )
	ROM_LOAD( "1",            0x0000, 0x1000, CRC(ebc93622) SHA1(bd1c098e91b24409925d01aa25de013451dba8e6) )
	ROM_LOAD( "2",            0x1000, 0x1000, CRC(2a4f4f44) SHA1(8c0519fcb631e05e967cf0953ab2749183655594) )
	ROM_LOAD( "3",            0x2000, 0x1000, CRC(a4112f91) SHA1(aad7ae81da7c20c7f4c1ef41697c8900a0c81f8e) )

	ROM_REGION( 0x7000, "banked", ROMREGION_BE )
	ROM_LOAD( "0",            0x0000, 0x0800, CRC(7a1e5998) SHA1(c133f43427540b39a383db7f46298942420d138a) )
	ROM_LOAD( "7",            0x0800, 0x0800, CRC(4c2616a3) SHA1(247411e2bb6618f77df6ea74aef1743fafb491a3) )
	ROM_LOAD( "9",            0x1000, 0x0800, CRC(7b146003) SHA1(04746f1b037bf6549fd53cff8f8c37136fce099e) )
	ROM_LOAD( "6",            0x1800, 0x0800, CRC(6d748030) SHA1(060ddf95eeb1318695a25c8c082a670fcdf117e7) )
	ROM_LOAD( "8",            0x2000, 0x0800, CRC(52d5438b) SHA1(087268ca30a42c00dbeceb4df901ddf80ae50125) )
	ROM_LOAD( "5",            0x2800, 0x0800, CRC(4a270340) SHA1(317fcc3156a099dbe48a0658757a9d6c4c54b23a) )
	ROM_LOAD( "4",            0x6000, 0x0800, CRC(e13f457c) SHA1(c706babc0005dfeb3c1b880047da6ec04bce407d) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "defcmnda.snd", 0xf800, 0x0800, CRC(f122d9c9) SHA1(70092fc354a2efbe7365be922fa36309b50d5c6f) )
ROM_END

ROM_START( defenderom )
	ROM_REGION( 0x3000, "maincpu", ROMREGION_BE )
	ROM_LOAD( "rom1.bin",  0x0000, 0x1000, CRC(8c04602b) SHA1(a8ed5afd0b276cebb479b1717666eaabbf75c6a5) )
	ROM_LOAD( "rom2.bin",  0x1000, 0x1000, CRC(89b75984) SHA1(a9481478da38f99efb67f0ecf82d084e14b93b42) )
	ROM_LOAD( "rom3.bin",  0x2000, 0x1000, CRC(94f51e9b) SHA1(a24cfc55de56a72758c76fe2a55f1ec6c353b16f) )

	ROM_REGION( 0x7000, "banked", ROMREGION_BE )
	ROM_LOAD( "rom10.bin", 0x0000, 0x0800, CRC(12e2bd1c) SHA1(c2fdf2fced003a0acf037aa6fab141b04c1c81bd) )
	ROM_LOAD( "rom7.bin",  0x0800, 0x0800, CRC(19e1ac79) SHA1(02925bbfab103304d097d778bda1b169b5f98d9c) )
	ROM_LOAD( "rom9.bin",  0x1000, 0x0800, CRC(b8ac5966) SHA1(df9ff8c6585f67dc55e54f07f6ec51158aa35ac3) )
	ROM_LOAD( "rom6.bin",  0x1800, 0x0800, BAD_DUMP CRC(9deaf6d9) SHA1(59b018ba0f3fe6eadfd387dc180ac281460358bc) ) // Bad on the dumped PCB, borrowed from 'defenseb'
	ROM_LOAD( "rom8.bin",  0x2000, 0x0800, BAD_DUMP CRC(339e092e) SHA1(2f89951dbe55d80df43df8dcf497171f73e726d3) ) // Bad on the dumped PCB, borrowed from 'defenseb'
	ROM_LOAD( "rom5.bin",  0x2800, 0x0800, CRC(871f75a0) SHA1(4ded757dbb375a703e930bd0c46281c0d8479a0c) )
	ROM_LOAD( "rom4.bin",  0x6000, 0x0800, CRC(65f4efd1) SHA1(a960fd1559ed74b81deba434391e49fc6ec389ca) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "rom12.bin", 0xf800, 0x0800, CRC(f122d9c9) SHA1(70092fc354a2efbe7365be922fa36309b50d5c6f) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "7641-1.bin", 0x0000, 0x0200, CRC(25de5d85) SHA1(826f78c2fe847f594d261c280dd10b9e776bf4fd) )
	ROM_LOAD( "7641-2.bin", 0x0200, 0x0200, CRC(c3f45f70) SHA1(d19036cbc46b130548873597b44b8b70758f25c4) )
ROM_END

// 2-PCB stack: BB10A + BB10B
ROM_START( defenseb )
	ROM_REGION( 0x3000, "maincpu", ROMREGION_BE ) // All ROMs but 2 identical to defenderj
	ROM_LOAD( "1.d9",  0x0000, 0x1000, CRC(8c04602b) SHA1(a8ed5afd0b276cebb479b1717666eaabbf75c6a5) )
	ROM_LOAD( "2.c9",  0x1000, 0x1000, CRC(89b75984) SHA1(a9481478da38f99efb67f0ecf82d084e14b93b42) )
	ROM_LOAD( "3.d8",  0x2000, 0x1000, CRC(94f51e9b) SHA1(a24cfc55de56a72758c76fe2a55f1ec6c353b16f) )

	ROM_REGION( 0x7000, "banked", ROMREGION_BE )
	ROM_LOAD( "10.c4", 0x0000, 0x0800, CRC(12e2bd1c) SHA1(c2fdf2fced003a0acf037aa6fab141b04c1c81bd) )
	ROM_LOAD( "7.d5",  0x0800, 0x0800, CRC(88881cc6) SHA1(5fd39a8596aeffc4ba279f9e680ac0ceaa2a179d) ) // unique
	ROM_LOAD( "9.d4",  0x1000, 0x0800, CRC(252605c9) SHA1(74d5a1f66b45193824f496b954d449d7acd68251) ) // unique
	ROM_LOAD( "6.c6",  0x1800, 0x0800, CRC(9deaf6d9) SHA1(59b018ba0f3fe6eadfd387dc180ac281460358bc) )
	ROM_LOAD( "8.c5",  0x2000, 0x0800, CRC(339e092e) SHA1(2f89951dbe55d80df43df8dcf497171f73e726d3) )
	ROM_LOAD( "5.d6",  0x2800, 0x0800, CRC(a543b167) SHA1(9292b94b0d74e57e03aada4852ad1997c34122ff) )
	ROM_LOAD( "4.c7",  0x6000, 0x0800, CRC(65f4efd1) SHA1(a960fd1559ed74b81deba434391e49fc6ec389ca) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "12.f3.snd", 0xf800, 0x0800, CRC(f122d9c9) SHA1(70092fc354a2efbe7365be922fa36309b50d5c6f) )

	ROM_REGION( 0x0400, "proms", 0 ) // same PROMs as colony7
	ROM_LOAD( "mmi6341.a1", 0x0000, 0x0200, CRC(25de5d85) SHA1(826f78c2fe847f594d261c280dd10b9e776bf4fd) )
	ROM_LOAD( "mmi6341.l1", 0x0200, 0x0200, CRC(c3f45f70) SHA1(d19036cbc46b130548873597b44b8b70758f25c4) )
ROM_END

// The dumps of ROMs 1, 2, 3 were bad, but the dumper observed that using the defenderb ROMs the emulation behaves identically to the PCB.
// A redump is definitely needed:
// 002-1.ic1   [1/2]      wb01.bin     [1/2]      IDENTICAL
// 002-2.ic2   [1/2]      defeng02.bin [1/2]      IDENTICAL
// 002-1.ic1   [2/2]      wb01.bin     [1/2]      IDENTICAL
// 002-2.ic2   [2/2]      defeng02.bin [1/2]      IDENTICAL
// 002-3.ic3   [2/2]      wb03.bin     [1/2]      IDENTICAL
// 002-3.ic3   [1/2]      defend.11               2.197266%
// For now we use the defenderb ones.
// PCBs: FAMARESA 590-001, 590-002, 590-003, 590-004
ROM_START( attackf )
	ROM_REGION( 0x3000, "maincpu", ROMREGION_BE )
	ROM_LOAD( "002-1.ic1",   0x0000, 0x1000, BAD_DUMP CRC(0ee1019d) SHA1(a76247e825b8267abfd195c12f96348fe10d4cbc) )
	ROM_LOAD( "002-2.ic2",   0x1000, 0x1000, BAD_DUMP CRC(d184ab6b) SHA1(ed61a95b04f6162aedba8a72bc46005b77283955) )
	ROM_LOAD( "002-3.ic3",   0x2000, 0x1000, BAD_DUMP CRC(a732d649) SHA1(b681882c02c5870ad613edc77255969a5f796422) )

	ROM_REGION( 0x7000, "banked", ROMREGION_BE )
	ROM_LOAD( "002-9.ic12",  0x0000, 0x0800, CRC(f57caa62) SHA1(c8c91b96fd3bc98eddcc1503159050dae5755001) )
	ROM_LOAD( "002-12.ic9",  0x0800, 0x0800, CRC(eb73d8a1) SHA1(f26007839a9eff6c7f77768da150fa26b8c96643) )
	ROM_LOAD( "002-8.ic11",  0x1000, 0x0800, CRC(17f7abde) SHA1(6959ed471687174a3fdc3f980ca7bd993b23d54f) )
	ROM_LOAD( "002-11.ic8",  0x1800, 0x0800, CRC(5ca4e860) SHA1(031188c009b8fca92703a0cc0c2bb44976212ae9) )
	ROM_LOAD( "002-7.ic10",  0x2000, 0x0800, CRC(545c3326) SHA1(98199df5206c261061b0108c68ab9128fa0779eb) )
	ROM_LOAD( "002-10.ic7",  0x2800, 0x0800, CRC(3940d731) SHA1(c867efa48e3ed6a6c3ddcd519aba1fe0a1712400) )
	ROM_LOAD( "002-6.ic6",   0x6000, 0x0800, CRC(3af34c05) SHA1(71f3ced06a373fa4805c856bd9fc97760787a920) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "003-13.ic12", 0xf800, 0x0800, CRC(fefd5b48) SHA1(ceb0d18483f0691978c604db94417e6941ad7ff2) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "001-14.g1",   0x0000, 0x0200, BAD_DUMP CRC(8dd98da5) SHA1(da979604f7a2aa8b5a6d4a5debd2e80f77569e35) ) // not dumped from this PCB, believed to match
ROM_END

ROM_START( galwars2 ) // 2 board stack: CPU and ROM boards
	ROM_REGION( 0x3000, "maincpu", ROMREGION_BE )
	ROM_LOAD( "9d-1-2532.bin",  0x0000, 0x1000, CRC(ebc93622) SHA1(bd1c098e91b24409925d01aa25de013451dba8e6) )
	ROM_LOAD( "9c-2-2532.bin",  0x1000, 0x1000, CRC(2a4f4f44) SHA1(8c0519fcb631e05e967cf0953ab2749183655594) )
	ROM_LOAD( "8d-3-2532.bin",  0x2000, 0x1000, CRC(a4112f91) SHA1(aad7ae81da7c20c7f4c1ef41697c8900a0c81f8e) )

	ROM_REGION( 0x7000, "banked", ROMREGION_BE )
	ROM_LOAD( "4c-10-2716.bin", 0x0000, 0x0800, CRC(7a1e5998) SHA1(c133f43427540b39a383db7f46298942420d138a) )
	ROM_LOAD( "5d-7-2716.bin",  0x0800, 0x0800, CRC(a9bdacdc) SHA1(aa6f31a127c5e744c1267705fffa659c03c38329) )
	ROM_LOAD( "4d-9-2716.bin",  0x1000, 0x0800, CRC(906dca8f) SHA1(ae77945f1628f5c60040dcc3fa650ace3bbe8720) )
	ROM_LOAD( "6c-6-2716.bin",  0x1800, 0x0800, CRC(6d748030) SHA1(060ddf95eeb1318695a25c8c082a670fcdf117e7) )
	ROM_LOAD( "5c-8-2716.bin",  0x2000, 0x0800, CRC(52d5438b) SHA1(087268ca30a42c00dbeceb4df901ddf80ae50125) )
	ROM_LOAD( "6d-5-2716.bin",  0x2800, 0x0800, CRC(4a270340) SHA1(317fcc3156a099dbe48a0658757a9d6c4c54b23a) )
	ROM_LOAD( "7c-4-2716.bin",  0x6000, 0x0800, CRC(e13f457c) SHA1(c706babc0005dfeb3c1b880047da6ec04bce407d) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "3f-11-2716.bin", 0xf800, 0x0800, CRC(f122d9c9) SHA1(70092fc354a2efbe7365be922fa36309b50d5c6f) )

	ROM_REGION( 0x1000, "user1", 0 ) // these are on the main CPU board. What are they used for?
	ROM_LOAD( "1l-13-8516.bin",   0x0000, 0x0800, CRC(7e113979) SHA1(ac908afb6aa756fc4db1ffddbd3688aa07080693) ) // 11xxxxxxxxx = 0x00, identical to rom11.bin in maydayb
	ROM_LOAD( "1a-12-8516.bin",   0x0800, 0x0800, CRC(a562c506) SHA1(a0bae41732f05caa80b9c13fba6ae4f01647e680) ) // 11xxxxxxxxx = 0x00, identical to rom12.bin in maydayb
ROM_END

ROM_START( mayday )
	ROM_REGION( 0x3000, "maincpu", ROMREGION_BE )
	ROM_LOAD( "ic03-3.bin",  0x0000, 0x1000, CRC(a1ff6e62) SHA1(c3c60ce94c6bdc4b07e45f386eff9a4aa4816953) )
	ROM_LOAD( "ic02-2.bin",  0x1000, 0x1000, CRC(62183aea) SHA1(3843fe055ab6d3bb5a3362f57a63ce99e36cec47) )
	ROM_LOAD( "ic01-1.bin",  0x2000, 0x1000, CRC(5dcb113f) SHA1(c41d671c336c68824771b7c4f0ffce39f1b6cd62) )

	ROM_REGION( 0x7000, "banked", ROMREGION_BE )
	ROM_LOAD( "ic04-4.bin",  0x0000, 0x1000, CRC(ea6a4ec8) SHA1(eaedc11968d88fd6f3c5b40c8d15d64ca6d0a1ab) )
	ROM_LOAD( "ic05-5.bin",  0x1000, 0x1000, CRC(0d797a3e) SHA1(289d2ecfebd7d71430d6624f3c9fbc91f9ef05cc) )
	ROM_LOAD( "ic06-6.bin",  0x2000, 0x1000, CRC(ee8bfcd6) SHA1(f68c44fdc18d57070aea604e261fb7b9407345a2) )
	ROM_LOAD( "ic07-7d.bin", 0x6000, 0x1000, CRC(d9c065e7) SHA1(ceeb58d1dfe14106271f17cf1c689b812216c3c0) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ic28-8.bin",  0xf800, 0x0800, CRC(fefd5b48) SHA1(ceb0d18483f0691978c604db94417e6941ad7ff2) )
ROM_END

ROM_START( maydaya )
	ROM_REGION( 0x3000, "maincpu", ROMREGION_BE )
	ROM_LOAD( "mayday.c",   0x0000, 0x1000, CRC(872a2f2d) SHA1(5823e889151b34e3fa739775440c788cca0b44c6) )
	ROM_LOAD( "mayday.b",   0x1000, 0x1000, CRC(c4ab5e22) SHA1(757fd9311cffea420b1de8f574e84c13c0aac77d) )
	ROM_LOAD( "mayday.a",   0x2000, 0x1000, CRC(329a1318) SHA1(4aa1d05ca05f37460eccb450ae61c21d86348f02) )

	ROM_REGION( 0x7000, "banked", ROMREGION_BE )
	ROM_LOAD( "mayday.d",   0x0000, 0x1000, CRC(c2ae4716) SHA1(582f763eda7d7d51ed0580045d6c617246b104b7) )
	ROM_LOAD( "mayday.e",   0x1000, 0x1000, CRC(41225666) SHA1(6d9c0347ff85bf9f9ae4648976c3ee971fec0f53) )
	ROM_LOAD( "mayday.f",   0x2000, 0x1000, CRC(c39be3c0) SHA1(91ac61e20d325a3a018ffe57bd79bfdfdc5a3cbd) )
	ROM_LOAD( "mayday.g",   0x6000, 0x1000, CRC(2bd0f106) SHA1(ac74d74a54d5b464e4c82b5b46ad7d20cdb7b6d7) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ic28-8.bin", 0xf800, 0x0800, CRC(fefd5b48) SHA1(ceb0d18483f0691978c604db94417e6941ad7ff2) )
ROM_END

ROM_START( maydayb )
	ROM_REGION( 0x3000, "maincpu", ROMREGION_BE )
	ROM_LOAD( "ic03-3.bin",  0x0000, 0x1000, CRC(a1ff6e62) SHA1(c3c60ce94c6bdc4b07e45f386eff9a4aa4816953) )
	ROM_LOAD( "ic02-2.bin",  0x1000, 0x1000, CRC(62183aea) SHA1(3843fe055ab6d3bb5a3362f57a63ce99e36cec47) )
	ROM_LOAD( "ic01-1.bin",  0x2000, 0x1000, CRC(5dcb113f) SHA1(c41d671c336c68824771b7c4f0ffce39f1b6cd62) )

	ROM_REGION( 0x7000, "banked", ROMREGION_BE )
	ROM_LOAD( "rom7.bin",    0x0000, 0x1000, CRC(0c3ca687) SHA1(a83f17c20f5767f092300266dd494bd0abf267bb) )
	ROM_LOAD( "ic05-5.bin",  0x1000, 0x1000, CRC(0d797a3e) SHA1(289d2ecfebd7d71430d6624f3c9fbc91f9ef05cc) )
	ROM_LOAD( "ic06-6.bin",  0x2000, 0x1000, CRC(ee8bfcd6) SHA1(f68c44fdc18d57070aea604e261fb7b9407345a2) )
	ROM_LOAD( "ic07-7d.bin", 0x6000, 0x1000, CRC(d9c065e7) SHA1(ceeb58d1dfe14106271f17cf1c689b812216c3c0) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ic28-8.bin",  0xf800, 0x0800, CRC(fefd5b48) SHA1(ceb0d18483f0691978c604db94417e6941ad7ff2) )

	ROM_REGION( 0x2000, "user1", 0 ) // what are these? alt (bad?) roms? rom11.bin and rom12.bin were also found on galwars2 PCB
	ROM_LOAD( "rom11.bin",   0x0000, 0x0800, CRC(7e113979) SHA1(ac908afb6aa756fc4db1ffddbd3688aa07080693) ) // 11xxxxxxxxx = 0x00
	ROM_LOAD( "rom12.bin",   0x0800, 0x0800, CRC(a562c506) SHA1(a0bae41732f05caa80b9c13fba6ae4f01647e680) ) // 11xxxxxxxxx = 0x00
	ROM_LOAD( "rom6a.bin",   0x1000, 0x0800, CRC(8e4e981f) SHA1(685c1fca9373f4129c7c6b86f18900a1bd324019) )
	ROM_LOAD( "rom8-sos.bin",0x1800, 0x0800, CRC(6a9b383f) SHA1(10e71a3bb9492b6c34ff06760dd55c442611ca75) ) // FIXED BITS (xxxxxx1x)
ROM_END

ROM_START( batlzone )
	ROM_REGION( 0x3000, "maincpu", ROMREGION_BE )
	ROM_LOAD( "43-2732.rom.bin",  0x0000, 0x1000, CRC(244334f8) SHA1(ac625a1858c372db6a748ef8aa504569aef6cad7) )
	ROM_LOAD( "42-2732.rom.bin",  0x1000, 0x1000, CRC(62183aea) SHA1(3843fe055ab6d3bb5a3362f57a63ce99e36cec47) )
	ROM_LOAD( "41-2732.rom.bin",  0x2000, 0x1000, CRC(a7e9093e) SHA1(d9d9641c9f8c060b2fa227b0620454067d0b0acc) )

	ROM_REGION( 0x7000, "banked", ROMREGION_BE )
	ROM_LOAD( "44-8532.rom.bin",  0x0000, 0x1000, CRC(bba3e626) SHA1(f2a364a25ee0cf91e25f8a20173bd9fb56cc2a72) )
	ROM_LOAD( "45-8532.rom.bin",  0x1000, 0x1000, CRC(43b3a0de) SHA1(674ff7110d07aeb09889eddb0d3dd0e7b16fe979) )
	ROM_LOAD( "46-8532.rom.bin",  0x2000, 0x1000, CRC(3df9b901) SHA1(6b2be3292b60e197154688ffd40789e937e17b4c) )
	ROM_LOAD( "47-8532.rom.bin",  0x6000, 0x1000, CRC(55a27e02) SHA1(23064ba59caed9b21e4ef6b944313e5bd2809dc5) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "48-2716.rom.bin",  0xf800, 0x0800, CRC(fefd5b48) SHA1(ceb0d18483f0691978c604db94417e6941ad7ff2) )
ROM_END


ROM_START( colony7 )
	ROM_REGION( 0x3000, "maincpu", ROMREGION_BE )
	ROM_LOAD( "cs03.bin",  0x0000, 0x1000, CRC(7ee75ae5) SHA1(1d268d83c2b0c7897d9e783f5da4e1d892709ba4) )
	ROM_LOAD( "cs02.bin",  0x1000, 0x1000, CRC(c60b08cb) SHA1(8cf91a2c2c04199b2870bb11e10fa6ffef5b877f) )
	ROM_LOAD( "cs01.bin",  0x2000, 0x1000, CRC(1bc97436) SHA1(326692de3491925bbeea9b0e880d9133f0e6440c) )

	ROM_REGION( 0x3000, "banked", ROMREGION_BE )
	ROM_LOAD( "cs06.bin",  0x0000, 0x0800, CRC(318b95af) SHA1(cb276ef440436f6000a2d20252f3197a67965167) )
	ROM_LOAD( "cs04.bin",  0x0800, 0x0800, CRC(d740faee) SHA1(aad164e72ebb0de18487c5397ea33d300cf93423) )
	ROM_LOAD( "cs07.bin",  0x1000, 0x0800, CRC(0b23638b) SHA1(b577c0cefa3ea2df436ed0fa1efa8ecd04ff78b0) )
	ROM_LOAD( "cs05.bin",  0x1800, 0x0800, CRC(59e406a8) SHA1(b64081ca83b6f57ac8fb71b1f8618083f19b99de) )
	ROM_LOAD( "cs08.bin",  0x2000, 0x0800, CRC(3bfde87a) SHA1(f5047927833be97324c861aa93a8e95b457058c4) )
	ROM_RELOAD(            0x2800, 0x0800 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "cs11.bin",  0xf800, 0x0800, CRC(6032293c) SHA1(dd2c6afc1149a879d49e93d1a2fa8e1f6d0b043b) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "cs10.bin",  0x0000, 0x0200, CRC(25de5d85) SHA1(826f78c2fe847f594d261c280dd10b9e776bf4fd) )
	ROM_LOAD( "decoder.3", 0x0200, 0x0200, CRC(c3f45f70) SHA1(d19036cbc46b130548873597b44b8b70758f25c4) )
ROM_END

ROM_START( colony7a )
	ROM_REGION( 0x3000, "maincpu", ROMREGION_BE )
	ROM_LOAD( "cs03a.bin", 0x0000, 0x1000, CRC(e0b0d23b) SHA1(4c50e00a71b3b2bf8d032a3cb496e5473204a8d6) )
	ROM_LOAD( "cs02a.bin", 0x1000, 0x1000, CRC(370c6f41) SHA1(4e13a4cf9c1a3b1c354443c41549b59581d8b357) )
	ROM_LOAD( "cs01a.bin", 0x2000, 0x1000, CRC(ba299946) SHA1(42e5d6ad0505f5a951d92165c9e2fa4e86659469) )

	ROM_REGION( 0x3000, "banked", ROMREGION_BE )
	ROM_LOAD( "cs06.bin",  0x0000, 0x0800, CRC(318b95af) SHA1(cb276ef440436f6000a2d20252f3197a67965167) )
	ROM_LOAD( "cs04.bin",  0x0800, 0x0800, CRC(d740faee) SHA1(aad164e72ebb0de18487c5397ea33d300cf93423) )
	ROM_LOAD( "cs07.bin",  0x1000, 0x0800, CRC(0b23638b) SHA1(b577c0cefa3ea2df436ed0fa1efa8ecd04ff78b0) )
	ROM_LOAD( "cs05.bin",  0x1800, 0x0800, CRC(59e406a8) SHA1(b64081ca83b6f57ac8fb71b1f8618083f19b99de) )
	ROM_LOAD( "cs08.bin",  0x2000, 0x0800, CRC(3bfde87a) SHA1(f5047927833be97324c861aa93a8e95b457058c4) )
	ROM_RELOAD(            0x2800, 0x0800 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "cs11.bin",  0xf800, 0x0800, CRC(6032293c) SHA1(dd2c6afc1149a879d49e93d1a2fa8e1f6d0b043b) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "cs10.bin",  0x0000, 0x0200, CRC(25de5d85) SHA1(826f78c2fe847f594d261c280dd10b9e776bf4fd) )
	ROM_LOAD( "decoder.3", 0x0200, 0x0200, CRC(c3f45f70) SHA1(d19036cbc46b130548873597b44b8b70758f25c4) )
ROM_END


ROM_START( jin )
	ROM_REGION( 0x3000, "maincpu", ROMREGION_BE )
	ROM_LOAD( "jin11.6c", 0x0000, 0x1000, CRC(c4b9e93f) SHA1(5c3451fd8e108aed60c0ef1032873460aa81454e) )
	ROM_LOAD( "jin12.7c", 0x1000, 0x1000, CRC(a8bc9fdd) SHA1(fffec12dbb0ef85f65bbfd93569c535d41773da4) )
	ROM_LOAD( "jin13.6d", 0x2000, 0x1000, CRC(79779b85) SHA1(3c0aa595f8ee370790db89ea2ecc4868dde06a27) )

	ROM_REGION( 0x1000, "banked", ROMREGION_BE )
	ROM_LOAD( "jin14.4c", 0x0000, 0x1000, CRC(6a4df97e) SHA1(c1b8f180e1f935a88fb7203b77b97a614b091c9e) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "jin15.3f",   0xf800, 0x0800, CRC(fefd5b48) SHA1(ceb0d18483f0691978c604db94417e6941ad7ff2) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "jin.1a",   0x0000, 0x0200, CRC(8dd98da5) SHA1(da979604f7a2aa8b5a6d4a5debd2e80f77569e35) )
	ROM_LOAD( "jin.1l",   0x0200, 0x0200, CRC(c3f45f70) SHA1(d19036cbc46b130548873597b44b8b70758f25c4) )
ROM_END


/*

Conquest: early prototype by Vid Kidz for Williams.
Despite concept similarities, it's not an early version of Sinistar.

ROM files were reconstructed from June 1982 source code, not dumped from EPROMs.
It appears to use the sound ROM from Defender.

*/
ROM_START( conquest )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "conquest_a.bin", 0x00000, 0x4000, CRC(a384f4a2) SHA1(819df35281216b8be2ba066602fc7d19a860e69e) )
	ROM_LOAD( "conquest_b.bin", 0x0e000, 0x1000, CRC(9aab5516) SHA1(a71ce8f24fd7ffda8800d1af8c164085b0e2ec0a) )
	ROM_RELOAD(                 0x0f000, 0x1000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "video_sound_rom_1.ic12", 0xf800, 0x0800, CRC(fefd5b48) SHA1(ceb0d18483f0691978c604db94417e6941ad7ff2) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder.2",   0x0000, 0x0200, CRC(8dd98da5) SHA1(da979604f7a2aa8b5a6d4a5debd2e80f77569e35) )
	ROM_LOAD( "decoder.3",   0x0200, 0x0200, CRC(c3f45f70) SHA1(d19036cbc46b130548873597b44b8b70758f25c4) )
ROM_END


/*

Stargate ROM labels are in this format:

+--------------------+
| STARGATE ROM 1-A   |   <-- Game name, ROM board number and ROM type (A is 2532, B is 2732)
| (c) 1982 WILLIAMS  |
| ELECTRONICS, INC.  |
|     3002-1         |   <-- Williams game number & ROM number
+--------------------+

+--------------------+
| Video Sound Rom 2  |
| (c) 1981 WILLIAMS  |
| ELECTRONICS, INC.  |
|         STD. 744   |
+--------------------+

Solid Yellow (Black print) 3002-1  through 3002-12 - ROM type A, 2532
Solid Yellow (Green print) 3002-13 through 3002-24 - ROM type B, 2732

             |    Black print    |    Green print
 Part Number |  ROM#     Number  |  ROM#     Number
-------------+-------------------+------------------
A-5343-09700 | ROM 1A  - 3002-1  | ROM 1B  - 3002-13
A-5343-09701 | ROM 2A  - 3002-2  | ROM 2B  - 3002-14
A-5343-09702 | ROM 3A  - 3002-3  | ROM 3B  - 3002-15
A-5343-09703 | ROM 4A  - 3002-4  | ROM 4B  - 3002-16
A-5343-09704 | ROM 5A  - 3002-5  | ROM 5B  - 3002-17
A-5343-09705 | ROM 6A  - 3002-6  | ROM 6B  - 3002-18
A-5343-09706 | ROM 7A  - 3002-7  | ROM 7B  - 3002-19
A-5343-09707 | ROM 8A  - 3002-8  | ROM 8B  - 3002-20
A-5343-09708 | ROM 9A  - 3002-9  | ROM 9B  - 3002-21
A-5343-09709 | ROM 10A - 3002-10 | ROM 10B - 3002-22
A-5343-09710 | ROM 11A - 3002-11 | ROM 11B - 3002-23
A-5343-09711 | ROM 12A - 3002-12 | ROM 12B - 3002-24

D-8729-3002 ROM Board Assembly:
+-----------------------------------------------+
|        2J3                           2J4      |
|               +---------------+               |
|               | 6821 PIA @ 1C |               |
|2              +---------------+             L |
|J    4049BP    7420N       7474     SN7425N  E |
|2      7474    74LS139N    7411PC   SN7404N  D |
|  +----------+    +----------+    +----------+ |
|  | ROM3  4A |    | ROM2  4C |    | ROM1  4E | |
|  +----------+    +----------+    +----------+ |
|  +----------+    +----------+    +----------+ |
|  | ROM6  5A |    | ROM5  5C |    | ROM4  5E | |
|  +----------+    +----------+    +----------+ |
|  +----------+    +----------+    +----------+ |
|W | ROM9  6A |  W | ROM8  6C |    | ROM7  6E | |
|2 +----------+  4 +----------+    +----------+ |
|  +----------+    +----------+    +----------+ |
|w | ROM10 7A |  W | ROM11 7C |    | ROM12 7E | |
|1 +----------+  3 +----------+    +----------+ |
|                          +------------------+ |
|                          |  2J1  connector  | |
+--------------------------+------------------+-+

Wire W2 & W4 with Zero Ohm resistors for 2532 ROMs
Wire W1 & W3 with Zero Ohm resistors for 2732 ROMs

*/
ROM_START( stargate ) // "B" ROMs labeled 3002-13 through 3002-24, identical data
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "stargate_rom_1-a_3002-1.e4",   0x00000, 0x1000, CRC(88824d18) SHA1(f003a5a9319c4eb8991fa2aae3f10c72d6b8e81a) )
	ROM_LOAD( "stargate_rom_2-a_3002-2.c4",   0x01000, 0x1000, CRC(afc614c5) SHA1(087c6da93318e8dc922d3d22e0a2af7b9759701c) )
	ROM_LOAD( "stargate_rom_3-a_3002-3.a4",   0x02000, 0x1000, CRC(15077a9d) SHA1(7badb4318b208f49d7fa65e915d0aa22a1e37915) )
	ROM_LOAD( "stargate_rom_4-a_3002-4.e5",   0x03000, 0x1000, CRC(a8b4bf0f) SHA1(6b4d47c2899fe9f14f9dab5928499f12078c437d) )
	ROM_LOAD( "stargate_rom_5-a_3002-5.c5",   0x04000, 0x1000, CRC(2d306074) SHA1(54f871983699113e31bb756d4ca885c26c2d66b4) )
	ROM_LOAD( "stargate_rom_6-a_3002-6.a5",   0x05000, 0x1000, CRC(53598dde) SHA1(54b02d944caf95283c9b6f0160e75ea8c4ccc97b) )
	ROM_LOAD( "stargate_rom_7-a_3002-7.e6",   0x06000, 0x1000, CRC(23606060) SHA1(a487ffcd4920d1056b87469735f7e1002f6a2e49) )
	ROM_LOAD( "stargate_rom_8-a_3002-8.c6",   0x07000, 0x1000, CRC(4ec490c7) SHA1(8726ebaf048db9608dfe365bf434ed5ca9452db7) )
	ROM_LOAD( "stargate_rom_9-a_3002-9.a6",   0x08000, 0x1000, CRC(88187b64) SHA1(efacc4a6d4b2af9a236c9d520de6d605c79cc5a8) )
	ROM_LOAD( "stargate_rom_10-a_3002-10.a7", 0x0d000, 0x1000, CRC(60b07ff7) SHA1(ba833f48ddfc1bd04ddb41b1d1c840d66ee7da30) )
	ROM_LOAD( "stargate_rom_11-a_3002-11.c7", 0x0e000, 0x1000, CRC(7d2c5daf) SHA1(6ca39f493eb8b370154ad46ef01976d352c929e1) )
	ROM_LOAD( "stargate_rom_12-a_3002-12.e7", 0x0f000, 0x1000, CRC(a0396670) SHA1(c46872550e0ca031453c6513f8f0448ecc9b5572) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "video_sound_rom_2_std_744.ic12", 0xf800, 0x0800, CRC(2fcf6c4d) SHA1(9c4334ac3ff15d94001b22fc367af40f9deb7d57) ) // P/N A-5342-09809

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder_rom_4.3g", 0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) ) // Universal Horizontal decoder ROM - 7641-5 BPROM - P/N A-5342-09694
	ROM_LOAD( "decoder_rom_5.3c", 0x0200, 0x0200, CRC(f921c5fe) SHA1(9cebb8bb935315101d248140d1b4503993ebdf8a) ) // Universal Vertical decoder ROM - 7641-5 BPROM - P/N A-5342-09695
ROM_END


/*

Robotron 2084 ROM labels are in this format:

+--------------------+
| 2084 ROM 1-A       |   <-- Game name, ROM board number and ROM type (A is 2532, B is 2732)
| (c) 1982 WILLIAMS  |
| ELECTRONICS, INC.  |
|     3005-1         |   <-- Williams game number & ROM number
+--------------------+

+--------------------+
| Video Sound Rom 3  |
| (c) 1981 WILLIAMS  |
| ELECTRONICS, INC.  |
|         STD. 767   |
+--------------------+

Yellow/Red (black print) 3005-1  through 3005-12 - ROM type B, 2732 (jumpers W1 & W3) (the "A" is overwitten with "B")
Yellow/Red (green print) 3005-13 through 3005-24 - ROM type B, 2732 (jumpers W1 & W3)
Solid Blue               3005-13 through 3005-24 - ROM type B, 2732 (jumpers W1 & W3)

It's been confirmed that the Yellow labeled ROMs 3005-1 through 3005-12 are itentical to yellow labeled ROMs 3005-13 through 3005-24
Yellow labels ROMs 3005-1 through 3005-12 are known to be labeled as "A" type ROMs with the A overwitten with "B"

NOTE: Blue labels and later Yellow labels with red stripe share the SAME 3005-x numbers but have different data!

           | Y/R Black | Y/R Green |Solid Blue |
ROM | Board|  "A" ROMs |  "B" ROMs |  "B" ROMs |
 ## | Loc. |  label #  |  label #  |  label #  |
----+------+-----------+-----------+-----------+
  1 |  E4  |  3005-1   |  3005-13  |  3005-13  |
  2 |  C4  |  3005-2   |  3005-14  |  3005-14  |
  3 |  A4  |  3005-3   |  3005-15  |  3005-15  |
  4 |  E5  |  3005-4   |  3005-16  |  3005-16  |
  5 |  C5  |  3005-5   |  3005-17  |  3005-17  |
  6 |  A5  |  3005-6   |  3005-18  |  3005-18  |
  7 |  E6  |  3005-7   |  3005-19  |  3005-19  |
  8 |  C6  |  3005-8   |  3005-20  |  3005-20  |
  9 |  A6  |  3005-9   |  3005-21  |  3005-21  |
 10 |  A7  |  3005-10  |  3005-22  |  3005-22  |
 11 |  C7  |  3005-11  |  3005-23  |  3005-23  |
 12 |  E7  |  3005-12  |  3005-24  |  3005-24  |
----+------+-----------+-----------+-----------+

        |      Red label or        |
        | Yellow with red stripe   |    Solid Blue labeled
 ROM #  | Part Number     Number   |  Part Number    Number
--------+--------------------------+------------------------
ROM 1B  |  A-5343-09898   3005-1   |  A-5343-09945   3005-13
ROM 2B  |  A-5343-09899   3005-2   |  A-5343-09946   3005-14
ROM 3B  |  A-5343-09900   3005-3   |  A-5343-09947   3005-15
ROM 4B  |  A-5343-09901   3005-4   |  A-5343-09948   3005-16
ROM 5B  |  A-5343-09902   3005-5   |  A-5343-09949   3005-17
ROM 6B  |  A-5343-09903   3005-6   |  A-5343-09950   3005-18
ROM 7B  |  A-5343-09904   3005-7   |  A-5343-09951   3005-19
ROM 8B  |  A-5343-09905   3005-8   |  A-5343-09952   3005-20
ROM 9B  |  A-5343-09906   3005-9   |  A-5343-09953   3005-21
ROM 10B |  A-5343-09907   3005-10  |  A-5343-09954   3005-22
ROM 11B |  A-5343-09908   3005-11  |  A-5343-09955   3005-23
ROM 12B |  A-5343-09909   3005-12  |  A-5343-09956   3005-24

Robotron 2084 Manual No. 16P-3005-101 May 1982:
  - Current Robotron games use blue-label ROMs.  Earlier games have either yellow or red-labels ROMs, which are interchangeable
      and may be mixed in the same game. DO NOT attempt to mix blue-label ROMs with red or yellow-label ROMs.

D-9144-3005 ROM Board Assembly:
+----------------------------------------------+
|       2J3                           2J4      |
|              +---------------+               |
|              | 6821 PIA @ 1B |               |
|2             +---------------+             L |
|J   4049BP    7420N       7474     SN7425N  E |
|2     7474    74LS139N    7411PC   SN7404N  D |
| +----------+    +----------+    +----------+ |
| | ROM3  4A |    | ROM2  4C |    | ROM1  4E | |
| +----------+    +----------+    +----------+ |
| +----------+    +----------+    +----------+ |
| | ROM6  5A |WW  | ROM5  5C |WW  | ROM4  5E | |
| +----------+12  +----------+34  +----------+ |
| +----------+    +----------+    +----------+ |
| | ROM9  6A |    | ROM8  6C |    | ROM7  6E | |
| +----------+    +----------+    +----------+ |
| +----------+    +----------+    +----------+ |
| | ROM10 7A |    | ROM11 7C |    | ROM12 7E | |
| +----------+    +----------+    +----------+ |
| +------------------+            +----------+ |
| | VTI 8220  VL2001 |  74LS245N  |  74154N  | |
| +------------------+            +----------+ |
| +------------------+  74LS244N               |
| | VTI 8220  VL2001 |    +------------------+ |
| +------------------+    |  2J1  connector  | |
+-------------------------+------------------+-+

Connectors:
2J1 40 pin ribbon cable connector
2J2  6 pin header (KEY pin 4)
2J3 10 pin header (KEY pin 9)
2J4  9 pin header (KEY pin 1)

LED - 7Seg LED display

Wired W1 & W3 with Zero Ohm resistors for 2732 ROMs

*/
ROM_START( robotron ) // Solid Blue labels, "B" type ROMs labeled 3005-13 through 3005-24
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2084_rom_1b_3005-13.e4",  0x00000, 0x1000, CRC(66c7d3ef) SHA1(f6d60e26c209c1df2cc01ac07ad5559daa1b7118) ) // == 2084_rom_1b_3005-1.e4
	ROM_LOAD( "2084_rom_2b_3005-14.c4",  0x01000, 0x1000, CRC(5bc6c614) SHA1(4d6e82bc29f49100f7751ccfc6a9ff35695b84b3) ) // == 2084_rom_2b_3005-2.c4
	ROM_LOAD( "2084_rom_3b_3005-15.a4",  0x02000, 0x1000, CRC(e99a82be) SHA1(06a8c8dd0b4726eb7f0bb0e89c8533931d75fc1c) )
	ROM_LOAD( "2084_rom_4b_3005-16.e5",  0x03000, 0x1000, CRC(afb1c561) SHA1(aaf89c19fd8f4e8750717169eb1af476aef38a5e) )
	ROM_LOAD( "2084_rom_5b_3005-17.c5",  0x04000, 0x1000, CRC(62691e77) SHA1(79b4680ce19bd28882ae823f0e7b293af17cbb91) )
	ROM_LOAD( "2084_rom_6b_3005-18.a5",  0x05000, 0x1000, CRC(bd2c853d) SHA1(f76ec5432a7939b33a27be1c6855e2dbe6d9fdc8) )
	ROM_LOAD( "2084_rom_7b_3005-19.e6",  0x06000, 0x1000, CRC(49ac400c) SHA1(06eae5138254723819a5e93cfd9e9f3285fcddf5) ) // == 2084_rom_7b_3005-7.e6
	ROM_LOAD( "2084_rom_8b_3005-20.c6",  0x07000, 0x1000, CRC(3a96e88c) SHA1(7ae38a609ed9a6f62ca003cab719740ed7651b7c) ) // == 2084_rom_8b_3005-8.c6
	ROM_LOAD( "2084_rom_9b_3005-21.a6",  0x08000, 0x1000, CRC(b124367b) SHA1(fd9d75b866f0ebbb723f84889337e6814496a103) ) // == 2084_rom_9b_3005-9.a6
	ROM_LOAD( "2084_rom_10b_3005-22.a7", 0x0d000, 0x1000, CRC(13797024) SHA1(d426a50e75dabe936de643c83a548da5e399331c) )
	ROM_LOAD( "2084_rom_11b_3005-23.c7", 0x0e000, 0x1000, CRC(7e3c1b87) SHA1(f8c6cbe3688f256f41a121255fc08f575f6a4b4f) )
	ROM_LOAD( "2084_rom_12b_3005-24.e7", 0x0f000, 0x1000, CRC(645d543e) SHA1(fad7cea868ebf17347c4bc5193d647bbd8f9517b) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "video_sound_rom_3_std_767.ic12", 0xf000, 0x1000, CRC(c56c1d28) SHA1(15afefef11bfc3ab78f61ab046701db78d160ec3) ) // P/N A-5342-09910

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder_rom_4.3g", 0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) ) // Universal Horizontal decoder ROM - 7641-5 BPROM - P/N A-5342-09694
	ROM_LOAD( "decoder_rom_6.3c", 0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) ) // Universal Vertical decoder ROM - 7641-5 BPROM - P/N A-5342-09821
ROM_END

ROM_START( robotronyo ) // Yellow label / Red stripe & Black print or Yellow label / Red stripe & Green print "B" type ROMs numbered 3005-13 through 3005-24
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2084_rom_1b_3005-1.e4",   0x00000, 0x1000, CRC(66c7d3ef) SHA1(f6d60e26c209c1df2cc01ac07ad5559daa1b7118) )
	ROM_LOAD( "2084_rom_2b_3005-2.c4",   0x01000, 0x1000, CRC(5bc6c614) SHA1(4d6e82bc29f49100f7751ccfc6a9ff35695b84b3) )
	ROM_LOAD( "2084_rom_3b_3005-3.a4",   0x02000, 0x1000, CRC(67a369bc) SHA1(5a912d485e686de5e3175d3fc0e5daad36f4b836) )
	ROM_LOAD( "2084_rom_4b_3005-4.e5",   0x03000, 0x1000, CRC(b0de677a) SHA1(02013e00513dd74e878a01791cbcca92712e2c80) )
	ROM_LOAD( "2084_rom_5b_3005-5.c5",   0x04000, 0x1000, CRC(24726007) SHA1(8b4ed881f64e3ce73ac1a9ae2c184721c1ab37cc) )
	ROM_LOAD( "2084_rom_6b_3005-6.a5",   0x05000, 0x1000, CRC(028181a6) SHA1(41c4d9ece2ae8a103b7151fc4ff576796303318d) )
	ROM_LOAD( "2084_rom_7b_3005-7.e6",   0x06000, 0x1000, CRC(4dfcceae) SHA1(46fe1b1162d6054eb502852d065fc2e8c694b09d) )
	ROM_LOAD( "2084_rom_8b_3005-8.c6",   0x07000, 0x1000, CRC(3a96e88c) SHA1(7ae38a609ed9a6f62ca003cab719740ed7651b7c) )
	ROM_LOAD( "2084_rom_9b_3005-9.a6",   0x08000, 0x1000, CRC(b124367b) SHA1(fd9d75b866f0ebbb723f84889337e6814496a103) )
	ROM_LOAD( "2084_rom_10b_3005-10.a7", 0x0d000, 0x1000, CRC(4a9d5f52) SHA1(d5ae801e60ed829e7ef5c54a18aefca54eae827f) ) // originally printed as "A" ROMs, the A is overwitten with "B"
	ROM_LOAD( "2084_rom_11b_3005-11.c7", 0x0e000, 0x1000, CRC(2afc5e7f) SHA1(f3405be9ad2287f3921e7dbd9c5313c91fa7f8d6) )
	ROM_LOAD( "2084_rom_12b_3005-12.e7", 0x0f000, 0x1000, CRC(45da9202) SHA1(81b3b2a72a3c871e8d7b9348056622c90a20d876) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "video_sound_rom_3_std_767.ic12", 0xf000, 0x1000, CRC(c56c1d28) SHA1(15afefef11bfc3ab78f61ab046701db78d160ec3) ) // P/N A-5342-09910

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder_rom_4.3g", 0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) ) // Universal Horizontal decoder ROM - 7641-5 BPROM - P/N A-5342-09694
	ROM_LOAD( "decoder_rom_6.3c", 0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) ) // Universal Vertical decoder ROM - 7641-5 BPROM - P/N A-5342-09821
ROM_END

ROM_START( robotronun )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "roboun11.1b",  0x00000, 0x1000, CRC(66c7d3ef) SHA1(f6d60e26c209c1df2cc01ac07ad5559daa1b7118) )
	ROM_LOAD( "roboun11.2b",  0x01000, 0x1000, CRC(5bc6c614) SHA1(4d6e82bc29f49100f7751ccfc6a9ff35695b84b3) )
	ROM_LOAD( "roboun11.3b",  0x02000, 0x1000, CRC(e99a82be) SHA1(06a8c8dd0b4726eb7f0bb0e89c8533931d75fc1c) )
	ROM_LOAD( "roboun11.4b",  0x03000, 0x1000, CRC(afb1c561) SHA1(aaf89c19fd8f4e8750717169eb1af476aef38a5e) )
	ROM_LOAD( "roboun11.5b",  0x04000, 0x1000, CRC(62691e77) SHA1(79b4680ce19bd28882ae823f0e7b293af17cbb91) )
	ROM_LOAD( "roboun11.6b",  0x05000, 0x1000, CRC(bd2c853d) SHA1(f76ec5432a7939b33a27be1c6855e2dbe6d9fdc8) )
	ROM_LOAD( "roboun11.7b",  0x06000, 0x1000, CRC(8981a43b) SHA1(8ecab99093d42cb66e177dfa7cf7e352667930ca) ) //
	ROM_LOAD( "roboun11.8b",  0x07000, 0x1000, CRC(3a96e88c) SHA1(7ae38a609ed9a6f62ca003cab719740ed7651b7c) )
	ROM_LOAD( "roboun11.9b",  0x08000, 0x1000, CRC(b124367b) SHA1(fd9d75b866f0ebbb723f84889337e6814496a103) )
	ROM_LOAD( "roboun11.10b", 0x0d000, 0x1000, CRC(13797024) SHA1(d426a50e75dabe936de643c83a548da5e399331c) )
	ROM_LOAD( "roboun11.11b", 0x0e000, 0x1000, CRC(7e3c1b87) SHA1(f8c6cbe3688f256f41a121255fc08f575f6a4b4f) )
	ROM_LOAD( "roboun11.12b", 0x0f000, 0x1000, CRC(645d543e) SHA1(fad7cea868ebf17347c4bc5193d647bbd8f9517b) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "video_sound_rom_3_std_767.ic12", 0xf000, 0x1000, CRC(c56c1d28) SHA1(15afefef11bfc3ab78f61ab046701db78d160ec3) ) // P/N A-5342-09910

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder_rom_4.3g", 0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) ) // Universal Horizontal decoder ROM - 7641-5 BPROM - P/N A-5342-09694
	ROM_LOAD( "decoder_rom_6.3c", 0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) ) // Universal Vertical decoder ROM - 7641-5 BPROM - P/N A-5342-09821
ROM_END

ROM_START( robotron87 ) // Patch by Christian Gingras in 1987 fixing 7 bugs, AKA "Shot in the corner" bug fix
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2084_rom_1b_3005-13.e4",  0x00000, 0x1000, CRC(66c7d3ef) SHA1(f6d60e26c209c1df2cc01ac07ad5559daa1b7118) )
	ROM_LOAD( "2084_rom_2b_3005-14.c4",  0x01000, 0x1000, CRC(5bc6c614) SHA1(4d6e82bc29f49100f7751ccfc6a9ff35695b84b3) )
	ROM_LOAD( "2084_rom_3b_3005-15.a4",  0x02000, 0x1000, CRC(e99a82be) SHA1(06a8c8dd0b4726eb7f0bb0e89c8533931d75fc1c) )
	ROM_LOAD( "2084_rom_4b_3005-16.e5",  0x03000, 0x1000, CRC(afb1c561) SHA1(aaf89c19fd8f4e8750717169eb1af476aef38a5e) )
	ROM_LOAD( "fixrobo_rom_5b.c5",       0x04000, 0x1000, CRC(827cb5c9) SHA1(1732d16cd88e0662f1cffce1aeda5c8aa8c31338) ) // fixes the enforcer explosion “reset” bug
	ROM_LOAD( "2084_rom_6b_3005-18.a5",  0x05000, 0x1000, CRC(bd2c853d) SHA1(f76ec5432a7939b33a27be1c6855e2dbe6d9fdc8) )
	ROM_LOAD( "2084_rom_7b_3005-19.e6",  0x06000, 0x1000, CRC(49ac400c) SHA1(06eae5138254723819a5e93cfd9e9f3285fcddf5) )
	ROM_LOAD( "2084_rom_8b_3005-20.c6",  0x07000, 0x1000, CRC(3a96e88c) SHA1(7ae38a609ed9a6f62ca003cab719740ed7651b7c) )
	ROM_LOAD( "2084_rom_9b_3005-21.a6",  0x08000, 0x1000, CRC(b124367b) SHA1(fd9d75b866f0ebbb723f84889337e6814496a103) )
	ROM_LOAD( "2084_rom_10b_3005-22.a7", 0x0d000, 0x1000, CRC(13797024) SHA1(d426a50e75dabe936de643c83a548da5e399331c) )
	ROM_LOAD( "fixrobo_rom_11b.c7",      0x0e000, 0x1000, CRC(e83a2eda) SHA1(4a62fcd2f91dfb609c3d2c300bd9e6cb60edf52e) ) //
	ROM_LOAD( "2084_rom_12b_3005-24.e7", 0x0f000, 0x1000, CRC(645d543e) SHA1(fad7cea868ebf17347c4bc5193d647bbd8f9517b) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "video_sound_rom_3_std_767.ic12", 0xf000, 0x1000, CRC(c56c1d28) SHA1(15afefef11bfc3ab78f61ab046701db78d160ec3) ) // P/N A-5342-09910

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder_rom_4.3g", 0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) ) // Universal Horizontal decoder ROM - 7641-5 BPROM - P/N A-5342-09694
	ROM_LOAD( "decoder_rom_6.3c", 0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) ) // Universal Vertical decoder ROM - 7641-5 BPROM - P/N A-5342-09821

//  ROM_REGION( 0x800, "patch", 0 ) // the bugfix was distributed as the following patches, we don't need them for emulation, but list them here for reference.
//  ROM_LOAD( "fixrobo1.pat",   0x000, 0x6d1, CRC(38f81254) SHA1(7ea140c08bfd9947a7f3e769b24d5e8351525e4f) )
//  ROM_LOAD( "fixrobo2.pat",   0x000, 0x6ae, CRC(61912101) SHA1(bb52db08301ac38268f6ae71e7002730022de1c8) )
ROM_END

ROM_START( robotron12 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2084_rom_1b_3005-13.e4",  0x00000, 0x1000, CRC(66c7d3ef) SHA1(f6d60e26c209c1df2cc01ac07ad5559daa1b7118) )
	ROM_LOAD( "2084_rom_2b_3005-14.c4",  0x01000, 0x1000, CRC(5bc6c614) SHA1(4d6e82bc29f49100f7751ccfc6a9ff35695b84b3) )
	ROM_LOAD( "wave201.a4",              0x02000, 0x1000, CRC(85eb583e) SHA1(b6c4280415515de6f56b358206dc3bd93a12bfce) ) // wave 201 patch
	ROM_LOAD( "2084_rom_4b_3005-16.e5",  0x03000, 0x1000, CRC(afb1c561) SHA1(aaf89c19fd8f4e8750717169eb1af476aef38a5e) )
	ROM_LOAD( "fixrobo_rom_5b.c5",       0x04000, 0x1000, CRC(827cb5c9) SHA1(1732d16cd88e0662f1cffce1aeda5c8aa8c31338) ) // fixes the enforcer explosion “reset” bug
	ROM_LOAD( "2084_rom_6b_3005-18.a5",  0x05000, 0x1000, CRC(bd2c853d) SHA1(f76ec5432a7939b33a27be1c6855e2dbe6d9fdc8) )
	ROM_LOAD( "2084_rom_7b_3005-19.e6",  0x06000, 0x1000, CRC(49ac400c) SHA1(06eae5138254723819a5e93cfd9e9f3285fcddf5) )
	ROM_LOAD( "2084_rom_8b_3005-20.c6",  0x07000, 0x1000, CRC(3a96e88c) SHA1(7ae38a609ed9a6f62ca003cab719740ed7651b7c) )
	ROM_LOAD( "2084_rom_9b_3005-21.a6",  0x08000, 0x1000, CRC(b124367b) SHA1(fd9d75b866f0ebbb723f84889337e6814496a103) )
	ROM_LOAD( "2084_rom_10b_3005-22.a7", 0x0d000, 0x1000, CRC(13797024) SHA1(d426a50e75dabe936de643c83a548da5e399331c) )
	ROM_LOAD( "fixrobo_rom_11b.c7",      0x0e000, 0x1000, CRC(e83a2eda) SHA1(4a62fcd2f91dfb609c3d2c300bd9e6cb60edf52e) ) //
	ROM_LOAD( "2084_rom_12b_3005-24.e7", 0x0f000, 0x1000, CRC(645d543e) SHA1(fad7cea868ebf17347c4bc5193d647bbd8f9517b) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "video_sound_rom_3_std_767.ic12", 0xf000, 0x1000, CRC(c56c1d28) SHA1(15afefef11bfc3ab78f61ab046701db78d160ec3) ) // P/N A-5342-09910

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder_rom_4.3g", 0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) ) // Universal Horizontal decoder ROM - 7641-5 BPROM - P/N A-5342-09694
	ROM_LOAD( "decoder_rom_6.3c", 0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) ) // Universal Vertical decoder ROM - 7641-5 BPROM - P/N A-5342-09821
ROM_END

ROM_START( robotrontd ) // Tie-Die version starts with a "Solid Blue label" set
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2084_rom_1b_3005-13.e4",  0x00000, 0x1000, CRC(66c7d3ef) SHA1(f6d60e26c209c1df2cc01ac07ad5559daa1b7118) ) // == 2084_rom_1b_3005-1.e4
	ROM_LOAD( "2084_rom_2b_3005-14.c4",  0x01000, 0x1000, CRC(5bc6c614) SHA1(4d6e82bc29f49100f7751ccfc6a9ff35695b84b3) ) // == 2084_rom_2b_3005-2.c4
	ROM_LOAD( "2084_rom_3b_3005-15.a4",  0x02000, 0x1000, CRC(e99a82be) SHA1(06a8c8dd0b4726eb7f0bb0e89c8533931d75fc1c) )
	ROM_LOAD( "tiedie_rom_4b.e5",        0x03000, 0x1000, CRC(e8238019) SHA1(0ce29f4bf6bdee677c8e80c2d5e66fc556ba349f) )
	ROM_LOAD( "fixrobo_rom_5b.c5",       0x04000, 0x1000, CRC(827cb5c9) SHA1(1732d16cd88e0662f1cffce1aeda5c8aa8c31338) ) // fixes the enforcer explosion “reset” bug
	ROM_LOAD( "2084_rom_6b_3005-18.a5",  0x05000, 0x1000, CRC(bd2c853d) SHA1(f76ec5432a7939b33a27be1c6855e2dbe6d9fdc8) )
	ROM_LOAD( "tiedie_rom_7b.e6",        0x06000, 0x1000, CRC(3ecf4620) SHA1(3c670a1f8df35d18451c82f220a02448bf5ef5ac) )
	ROM_LOAD( "tiedie_rom_8b.c6",        0x07000, 0x1000, CRC(752d7a46) SHA1(85dd58d14d527ca75d6c546d6271bf8ee5a82c8c) )
	ROM_LOAD( "2084_rom_9b_3005-21.a6",  0x08000, 0x1000, CRC(b124367b) SHA1(fd9d75b866f0ebbb723f84889337e6814496a103) ) // == 2084_rom_9b_3005-9.a6
	ROM_LOAD( "tiedie_rom_10b.a7",       0x0d000, 0x1000, CRC(952bea55) SHA1(80f51d8e7ec62518afad7e56a47e0756f83f813c) )
	ROM_LOAD( "tiedie_rom_11b.c7",       0x0e000, 0x1000, CRC(4c05fd3c) SHA1(0d727458454826fd8222e4022b755d686ccb065f) )
	ROM_LOAD( "2084_rom_12b_3005-24.e7", 0x0f000, 0x1000, CRC(645d543e) SHA1(fad7cea868ebf17347c4bc5193d647bbd8f9517b) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "video_sound_rom_3_std_767.ic12", 0xf000, 0x1000, CRC(c56c1d28) SHA1(15afefef11bfc3ab78f61ab046701db78d160ec3) ) // P/N A-5342-09910

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder_rom_4.3g", 0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) ) // Universal Horizontal decoder ROM - 7641-5 BPROM - P/N A-5342-09694
	ROM_LOAD( "decoder_rom_6.3c", 0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) ) // Universal Vertical decoder ROM - 7641-5 BPROM - P/N A-5342-09821
ROM_END


/*

Joust ROM labels are in this format:

+--------------------+
| JOUST ROM 1A       |   <-- Game name, ROM board number and ROM type (A is 2532, B is 2732)
| (c) 1982 WILLIAMS  |
| ELECTRONICS, INC.  |
|     3006-1         |   <-- Williams game number & ROM number
+--------------------+

+--------------------+
| Video Sound Rom 4  |
| (c) 1981 WILLIAMS  |
| ELECTRONICS, INC.  |
|         STD. 780   |
+--------------------+

Solid yellow* 3006-1  through 3006-12 - ROM type A, 2532 (jumpers W2 & W4)
Solid green^  3006-13 through 3006-24 - ROM type B, 2732 (jumpers W1 & W3)
Solid red     3006-28 through 3006-39 - ROM type A, 2532 (jumpers W2 & W4)
 --Missing--  3006-40 through 3006-51 - This set is unknown
White/green   3006-52 through 3006-63 - ROM type B, 2732 (jumpers W1 & W3)

* NOTE: Earliest examples are yellow lables with red stripe numbered 16-3006-1 through 16-3006-12
^ NOTE: Earliest examples have been mixed solid green labels and white labels with green stripe

           |Solid Yellow|Solid Green| Solid Red |White/Green|
ROM | Board|  "A" 2532  |  "B" 2732 |  "A" 2532 | "B" 2732  |
 ## | Loc. |  label #   |  label #  |  label #  |  label #  |
----+------+------------+-----------+-----------+-----------+
  1 |  E4  | 16-3006-1  |  3006-13  |  3006-28  |  3006-52  |
  2 |  C4  | 16-3006-2  |  3006-14  |  3006-29  |  3006-53  |
  3 |  A4  | 16-3006-3  |  3006-15  |  3006-30  |  3006-54  |
  4 |  E5  | 16-3006-4  |  3006-16  |  3006-31  |  3006-55  |
  5 |  C5  | 16-3006-5  |  3006-17  |  3006-32  |  3006-56  |
  6 |  A5  | 16-3006-6  |  3006-18  |  3006-33  |  3006-57  |
  7 |  E6  | 16-3006-7  |  3006-19  |  3006-34  |  3006-58  |
  8 |  C6  | 16-3006-8  |  3006-20  |  3006-35  |  3006-59  |
  9 |  A6  | 16-3006-9  |  3006-21  |  3006-36  |  3006-60  |
 10 |  A7  | 16-3006-10 |  3006-22  |  3006-37  |  3006-61  |
 11 |  C7  | 16-3006-11 |  3006-23  |  3006-38  |  3006-62  |
 12 |  E7  | 16-3006-12 |  3006-24  |  3006-39  |  3006-63  |
----+------+------------+-----------+-----------+-----------+

    Solid Yellow labeled ROMs      |      Solid Green labeled ROMs
Part Number       ROM     Number   |  Part Number       ROM     Number
-----------------------------------------------------------------------
A-5343-09961-A   ROM 1A   3006-1   |  A-5343-09961-B   ROM 1B   3006-13
A-5343-09962-A   ROM 2A   3006-2   |  A-5343-09962-B   ROM 2B   3006-14
A-5343-09963-A   ROM 3A   3006-3   |  A-5343-09963-B   ROM 3B   3006-15
A-5343-09964-A   ROM 4A   3006-4   |  A-5343-09964-B   ROM 4B   3006-16
A-5343-09965-A   ROM 5A   3006-5   |  A-5343-09965-B   ROM 5B   3006-17
A-5343-09966-A   ROM 6A   3006-6   |  A-5343-09966-B   ROM 6B   3006-18
A-5343-09967-A   ROM 7A   3006-7   |  A-5343-10150-B   ROM 7B   3006-19 <-- Revised code with a completly different part number
A-5343-09968-A   ROM 8A   3006-8   |  A-5343-09968-B   ROM 8B   3006-20
A-5343-09969-A   ROM 9A   3006-9   |  A-5343-09969-B   ROM 9B   3006-21
A-5343-09970-A   ROM 10A  3006-10  |  A-5343-10153-B   ROM 10B  3006-22 <-- Revised code with a completly different part number
A-5343-09971-A   ROM 11A  3006-11  |  A-5343-09971-B   ROM 11B  3006-23
A-5343-09972-A   ROM 12A  3006-12  |  A-5343-09972-B   ROM 12B  3006-24

Joust Manual Amendment No. 16P-3006-101-AMD-1 October 1982:
  - Current JOUST games use green-label ROMs.  Earlier games have either yellow or red-labels ROMs, which are interchangeable
      and may be mixed in the same game. DO NOT attempt to mix green-label ROMs with red or yellow label ROMs.
  - Boards with green-label ROMs should include jumper W1 and W3 only. Boards with red or yellow label ROMs subsitute jumpers W2 and W4

ROMs changed in October 1982 as Instruction Manuals 16P-3006-101-T September 1982 & 16P-3006-101 October 1982 only mention Yellow-label ROMs.
  Only the 16P-3006-101-AMD-1 October 1982 Amendment and the 16P-3006-101 Revision A December 1982 manuals mention the new green label ROMs

The "White labels with Green stripe" set (ROMs 3006-52 through 3006-63) contains the same data as the "Green label" set (ROMs 3006-13 through 3006-24).

D-9144-3006 ROM Board Assembly:
+----------------------------------------------+
|       2J3                           2J4      |
|              +---------------+               |
|              | 6821 PIA @ 1B |               |
|2             +---------------+             L |
|J   4049BP    7420N       7474     SN7425N  E |
|2     7474    74LS139N    7411PC   SN7404N  D |
| +----------+    +----------+    +----------+ |
| | ROM3  4A |    | ROM2  4C |    | ROM1  4E | |
| +----------+    +----------+    +----------+ |
| +----------+    +----------+    +----------+ |
| | ROM6  5A |WW  | ROM5  5C |WW  | ROM4  5E | |
| +----------+12  +----------+34  +----------+ |
| +----------+    +----------+    +----------+ |
| | ROM9  6A |    | ROM8  6C |    | ROM7  6E | |
| +----------+    +----------+    +----------+ |
| +----------+    +----------+    +----------+ |
| | ROM10 7A |    | ROM11 7C |    | ROM12 7E | |
| +----------+    +----------+    +----------+ |
| +------------------+            +----------+ |
| | VTI 8220  VL2001 |  74LS245N  |  74154N  | |
| +------------------+            +----------+ |
| +------------------+  74LS244N               |
| | VTI 8220  VL2001 |    +------------------+ |
| +------------------+    |  2J1  connector  | |
+-------------------------+------------------+-+

Connectors:
2J1 40 pin ribbon cable connector
2J2  6 pin header (KEY pin 4)
2J3 10 pin header (KEY pin 9)
2J4  9 pin header (KEY pin 1)

LED - 7Seg LED display

Wire W1 & W3 with Zero Ohm resistors for 2732 ROMs
Wire W2 & W4 with Zero Ohm resistors for 2532 ROMs

*/
ROM_START( joust ) // Solid green labels - contains the same data as the white label with green stripe 3006-52 through 3006-63 set
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "joust_rom_1b_3006-13.e4",  0x00000, 0x1000, CRC(fe41b2af) SHA1(0443e00ae2eb3e66cf805562ee04309487bb0ba4) ) // == joust_rom_1a_3006-1.e4
	ROM_LOAD( "joust_rom_2b_3006-14.c4",  0x01000, 0x1000, CRC(501c143c) SHA1(5fda266d43cbbf42eeae1a078b5209d9408ab99f) ) // == joust_rom_2a_3006-2.c4
	ROM_LOAD( "joust_rom_3b_3006-15.a4",  0x02000, 0x1000, CRC(43f7161d) SHA1(686da120aa4bd4a41f3d93e8c79ebb343977851a) ) // == joust_rom_3a_3006-3.a4
	ROM_LOAD( "joust_rom_4b_3006-16.e5",  0x03000, 0x1000, CRC(db5571b6) SHA1(cb1c3285344e2cfbe0a81ab9b51758c40da8a23f) ) // == joust_rom_4a_3006-4.e5
	ROM_LOAD( "joust_rom_5b_3006-17.c5",  0x04000, 0x1000, CRC(c686bb6b) SHA1(d9cac4c46820e1a451a145864bca7a35cfab7d37) ) // == joust_rom_5a_3006-5.c5
	ROM_LOAD( "joust_rom_6b_3006-18.a5",  0x05000, 0x1000, CRC(fac5f2cf) SHA1(febaa8cf5c3a0af901cd12d0b7909f6fec3beadd) ) // == joust_rom_6a_3006-6.a5
	ROM_LOAD( "joust_rom_7b_3006-19.e6",  0x06000, 0x1000, CRC(81418240) SHA1(5ad14aa65e71c3856dcdb04c99edda92e406a3e3) )
	ROM_LOAD( "joust_rom_8b_3006-20.c6",  0x07000, 0x1000, CRC(ba5359ba) SHA1(f4ee13d5a95ed3e1050a3927a3a0ccf86ed7752d) ) // == joust_rom_8a_3006-8.c6
	ROM_LOAD( "joust_rom_9b_3006-21.a6",  0x08000, 0x1000, CRC(39643147) SHA1(d95d3b746133eac9dcc9ee05eabecb797023f1a5) ) // == joust_rom_9a_3006-9.a6
	ROM_LOAD( "joust_rom_10b_3006-22.a7", 0x0d000, 0x1000, CRC(3f1c4f89) SHA1(90864a8ab944df45287bf0f68ad3a85194077a82) )
	ROM_LOAD( "joust_rom_11b_3006-23.c7", 0x0e000, 0x1000, CRC(ea48b359) SHA1(6d38003d56bebeb1f5b4d2287d587342847aa195) ) // == joust_rom_11a_3006-11.c7
	ROM_LOAD( "joust_rom_12b_3006-24.e7", 0x0f000, 0x1000, CRC(c710717b) SHA1(7d01764e8251c60b3cab96f7dc6dcc1c624f9d12) ) // == joust_rom_12a_3006-12.e7

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "video_sound_rom_4_std_780.ic12", 0xf000, 0x1000, CRC(f1835bdd) SHA1(af7c066d2949d36b87ea8c425ca7d12f82b5c653) ) // P/N A-5343-09973

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder_rom_4.3g", 0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) ) // Universal Horizontal decoder ROM - 7641-5 BPROM - P/N A-5342-09694
	ROM_LOAD( "decoder_rom_6.3c", 0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) ) // Universal Vertical decoder ROM - 7641-5 BPROM - P/N A-5342-09821
ROM_END

ROM_START( jousty ) // Solid yellow labels
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "joust_rom_1a_3006-1.e4",   0x00000, 0x1000, CRC(fe41b2af) SHA1(0443e00ae2eb3e66cf805562ee04309487bb0ba4) )
	ROM_LOAD( "joust_rom_2a_3006-2.c4",   0x01000, 0x1000, CRC(501c143c) SHA1(5fda266d43cbbf42eeae1a078b5209d9408ab99f) )
	ROM_LOAD( "joust_rom_3a_3006-3.a4",   0x02000, 0x1000, CRC(43f7161d) SHA1(686da120aa4bd4a41f3d93e8c79ebb343977851a) )
	ROM_LOAD( "joust_rom_4a_3006-4.e5",   0x03000, 0x1000, CRC(db5571b6) SHA1(cb1c3285344e2cfbe0a81ab9b51758c40da8a23f) )
	ROM_LOAD( "joust_rom_5a_3006-5.c5",   0x04000, 0x1000, CRC(c686bb6b) SHA1(d9cac4c46820e1a451a145864bca7a35cfab7d37) )
	ROM_LOAD( "joust_rom_6a_3006-6.a5",   0x05000, 0x1000, CRC(fac5f2cf) SHA1(febaa8cf5c3a0af901cd12d0b7909f6fec3beadd) )
	ROM_LOAD( "joust_rom_7a_3006-7.e6",   0x06000, 0x1000, CRC(e6f439c4) SHA1(ff8f1d54f3ac91101ab9f5f115baeca4f2670186) )
	ROM_LOAD( "joust_rom_8a_3006-8.c6",   0x07000, 0x1000, CRC(ba5359ba) SHA1(f4ee13d5a95ed3e1050a3927a3a0ccf86ed7752d) )
	ROM_LOAD( "joust_rom_9a_3006-9.a6",   0x08000, 0x1000, CRC(39643147) SHA1(d95d3b746133eac9dcc9ee05eabecb797023f1a5) )
	ROM_LOAD( "joust_rom_10a_3006-10.a7", 0x0d000, 0x1000, CRC(2039014a) SHA1(b9a76ecf01404585f833f76c54aa5a88a0215715) )
	ROM_LOAD( "joust_rom_11a_3006-11.c7", 0x0e000, 0x1000, CRC(ea48b359) SHA1(6d38003d56bebeb1f5b4d2287d587342847aa195) )
	ROM_LOAD( "joust_rom_12a_3006-12.e7", 0x0f000, 0x1000, CRC(c710717b) SHA1(7d01764e8251c60b3cab96f7dc6dcc1c624f9d12) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "video_sound_rom_4_std_780.ic12", 0xf000, 0x1000, CRC(f1835bdd) SHA1(af7c066d2949d36b87ea8c425ca7d12f82b5c653) ) // P/N A-5343-09973

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder_rom_4.3g", 0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) ) // Universal Horizontal decoder ROM - 7641-5 BPROM - P/N A-5342-09694
	ROM_LOAD( "decoder_rom_6.3c", 0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) ) // Universal Vertical decoder ROM - 7641-5 BPROM - P/N A-5342-09821
ROM_END

ROM_START( joustr ) // Solid red labels
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "joust_rom_1a_3006-28.e4",  0x00000, 0x1000, CRC(fe41b2af) SHA1(0443e00ae2eb3e66cf805562ee04309487bb0ba4) ) // == joust_rom_1a_3006-1.e4
	ROM_LOAD( "joust_rom_2a_3006-29.c4",  0x01000, 0x1000, CRC(501c143c) SHA1(5fda266d43cbbf42eeae1a078b5209d9408ab99f) ) // == joust_rom_2a_3006-2.c4
	ROM_LOAD( "joust_rom_3a_3006-30.a4",  0x02000, 0x1000, CRC(43f7161d) SHA1(686da120aa4bd4a41f3d93e8c79ebb343977851a) ) // == joust_rom_3a_3006-3.a4
	ROM_LOAD( "joust_rom_4a_3006-31.e5",  0x03000, 0x1000, CRC(ab347170) SHA1(ad50c83fcfa958f2673cae04bd811095f9ee08c0) )
	ROM_LOAD( "joust_rom_5a_3006-32.c5",  0x04000, 0x1000, CRC(c686bb6b) SHA1(d9cac4c46820e1a451a145864bca7a35cfab7d37) ) // == joust_rom_5a_3006-5.c5
	ROM_LOAD( "joust_rom_6a_3006-33.a5",  0x05000, 0x1000, CRC(3d9a6fac) SHA1(0c81394ae96a2fcfa4c953d38e43f3ef415fe4fc) )
	ROM_LOAD( "joust_rom_7a_3006-34.e6",  0x06000, 0x1000, CRC(0a70b3d1) SHA1(eb78b694aa29f777f3c7e7104e568f865930c0ec) )
	ROM_LOAD( "joust_rom_8a_3006-35.c6",  0x07000, 0x1000, CRC(a7f01504) SHA1(0ca3211d060befc102bda2e97d163de7fb12a6f6) )
	ROM_LOAD( "joust_rom_9a_3006-36.a6",  0x08000, 0x1000, CRC(978687ad) SHA1(25e651af3e3be08d6293aab427a0843e9333a629) )
	ROM_LOAD( "joust_rom_10a_3006-37.a7", 0x0d000, 0x1000, CRC(c0c6e52a) SHA1(f14ff16195027f3e199e79e43741f0849c17fd10) )
	ROM_LOAD( "joust_rom_11a_3006-38.c7", 0x0e000, 0x1000, CRC(ab11bcf9) SHA1(efb09e92a621d6c4d6cde2f166e8c988c64d81ae) )
	ROM_LOAD( "joust_rom_12a_3006-39.e7", 0x0f000, 0x1000, CRC(ea14574b) SHA1(7572d118b2343646054e558f0bd48e4959d84ce7) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "video_sound_rom_4_std_780.ic12", 0xf000, 0x1000, CRC(f1835bdd) SHA1(af7c066d2949d36b87ea8c425ca7d12f82b5c653) ) // P/N A-5343-09973

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder_rom_4.3g", 0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) ) // Universal Horizontal decoder ROM - 7641-5 BPROM - P/N A-5342-09694
	ROM_LOAD( "decoder_rom_6.3c", 0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) ) // Universal Vertical decoder ROM - 7641-5 BPROM - P/N A-5342-09821
ROM_END


/*

Bubbles ROM labels are in this format:

+--------------------+
| BUBBLES ROM 1B     |   <-- Game name, ROM board number and ROM type (B is 2732)
| (c) 1983 WILLIAMS  |
| ELECTRONICS, INC.  |
|    16-3012-1       |   <-- Williams game number & ROM number
+--------------------+

+--------------------+
| Video Sound Rom 5  |
| (c) 1982 WILLIAMS  |
| ELECTRONICS        |
|         STD. 771   |
+--------------------+

ROM | Board|  "B" ROMs  |
 ## | Loc. |  label #   |  Part Number
----+------+------------+----------------
  1 |  E4  | 16-3012-1  |  A-5343-10111-B
  2 |  C4  | 16-3012-2  |  A-5343-10112-B
  3 |  A4  | 16-3012-3  |  A-5343-10113-B
  4 |  E5  | 16-3012-4  |  A-5343-10114-B
  5 |  C5  | 16-3012-5  |  A-5343-10115-B
  6 |  A5  | 16-3012-6  |  A-5343-10116-B
  7 |  E6  | 16-3012-7  |  A-5343-10117-B
  8 |  C6  | 16-3012-8  |  A-5343-10118-B
  9 |  A6  | 16-3012-9  |  A-5343-10119-B
 10 |  A7  | 16-3012-10 |  A-5343-10120-B
 11 |  C7  | 16-3012-11 |  A-5343-10121-B
 12 |  E7  | 16-3012-12 |  A-5343-10122-B

Instruction Manual 16-3012-101 states Brown labels

Observed, but currently unverified, sets include:
  Red Label "B" ROMs numbers 16-3012-13 through 16-3012-24
  Red Label "B" ROMs numbers 16-3012-52 through 16-3012-63


D-9144-3012 ROM Board Assembly:
+----------------------------------------------+
|       2J3                           2J4      |
|              +---------------+               |
|              | 6821 PIA @ 1B |               |
|2             +---------------+             L |
|J   4049BP    7420N       7474     SN7425N  E |
|2     7474    74LS139N    7411PC   SN7404N  D |
| +----------+    +----------+    +----------+ |
| | ROM3  4A |    | ROM2  4C |    | ROM1  4E | |
| +----------+    +----------+    +----------+ |
| +----------+    +----------+    +----------+ |
| | ROM6  5A |WW  | ROM5  5C |WW  | ROM4  5E | |
| +----------+12  +----------+34  +----------+ |
| +----------+    +----------+    +----------+ |
| | ROM9  6A |    | ROM8  6C |    | ROM7  6E | |
| +----------+    +----------+    +----------+ |
| +----------+    +----------+    +----------+ |
| | ROM10 7A |    | ROM11 7C |    | ROM12 7E | |
| +----------+    +----------+    +----------+ |
| +------------------+            +----------+ |
| | VTI 8220  VL2001 |  74LS245N  |  74154N  | |
| +------------------+            +----------+ |
| +------------------+  74LS244N               |
| | VTI 8220  VL2001 |    +------------------+ |
| +------------------+    |  2J1  connector  | |
+-------------------------+------------------+-+

Connectors:
2J1 40 pin ribbon cable connector
2J2  6 pin header (KEY pin 4)
2J3 10 pin header (KEY pin 9)
2J4  9 pin header (KEY pin 1)

LED - 7Seg LED display

Wire W1 & W3 with Zero Ohm resistors for 2732 ROMs
Wire W2 & W4 with Zero Ohm resistors for 2532 ROMs


For the sound ROM:
  Instruction Manual 16-3012-101 states "ROM 13" P/N A-5342-10127 (same as Splat)
  Drawing Set 16-3012-103 states "Video Sound ROM 8"

*/
ROM_START( bubbles )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Solid red Label "B" ROMs numbers 16-3012-40 through 16-3012-51
	ROM_LOAD( "bubbles_rom_1b_16-3012-40.4e",  0x00000, 0x1000, CRC(8234f55c) SHA1(4d60942320c03ae50b0b17267062a321cf49e240) )
	ROM_LOAD( "bubbles_rom_2b_16-3012-41.4c",  0x01000, 0x1000, CRC(4a188d6a) SHA1(2788c4a21659799e59ab82bc8d1864a3abe3b6d7) )
	ROM_LOAD( "bubbles_rom_3b_16-3012-42.4a",  0x02000, 0x1000, CRC(7728f07f) SHA1(2a2c6dd8c2196dcd5e71b38554a56ee03d2aa454) )
	ROM_LOAD( "bubbles_rom_4b_16-3012-43.5e",  0x03000, 0x1000, CRC(040be7f9) SHA1(de4d212cd2967b2dcd7b2c09dea2c1b06ce4c5bd) )
	ROM_LOAD( "bubbles_rom_5b_16-3012-44.5c",  0x04000, 0x1000, CRC(0b5f29e0) SHA1(ae52f8c69c8b821abb458288c8ee0bc6c28fe535) )
	ROM_LOAD( "bubbles_rom_6b_16-3012-45.5a",  0x05000, 0x1000, CRC(4dd0450d) SHA1(d55aa8fb8f2974ce5ba7155b01bc3e3622f202af) )
	ROM_LOAD( "bubbles_rom_7b_16-3012-46.6e",  0x06000, 0x1000, CRC(e0a26ec0) SHA1(2da6213df6c15735a8bbd6750cfb1a1b6232a6f5) )
	ROM_LOAD( "bubbles_rom_8b_16-3012-47.6c",  0x07000, 0x1000, CRC(4fd23d8d) SHA1(9d71caa30bc3f4151789279d21651e5a4fe4a484) )
	ROM_LOAD( "bubbles_rom_9b_16-3012-48.6a",  0x08000, 0x1000, CRC(b48559fb) SHA1(551a49a12353044dbbf28dba2bd860c2d00c50bd) )
	ROM_LOAD( "bubbles_rom_10b_16-3012-49.a7", 0x0d000, 0x1000, CRC(26e7869b) SHA1(db428e79fc325ae3c8cab460267c27cdbc35a3bd) )
	ROM_LOAD( "bubbles_rom_11b_16-3012-50.c7", 0x0e000, 0x1000, CRC(5a5b572f) SHA1(f0c3a330abf9c8cfb6007ee372409450d2a15a93) )
	ROM_LOAD( "bubbles_rom_12b_16-3012-51.e7", 0x0f000, 0x1000, CRC(ce22d2e2) SHA1(be4b9800c846660ce2b2ddd75ad872dcf174979a) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "video_sound_rom_5_std_771.ic12",  0xf000, 0x1000, CRC(689ce2aa) SHA1(b70d2553f731f9a20ddaf9af2f93b7e9c44d4d99) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder_rom_4.3g", 0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) ) // Universal Horizontal decoder ROM - 7641-5 BPROM - P/N A-5342-09694
	ROM_LOAD( "decoder_rom_6.3c", 0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) ) // Universal Vertical decoder ROM - 7641-5 BPROM - P/N A-5342-09821
ROM_END

ROM_START( bubblesr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bubblesr.1b",  0x00000, 0x1000, CRC(dda4e782) SHA1(ad6825ebc05931942ce1042f18e18e3873083abc) )
	ROM_LOAD( "bubblesr.2b",  0x01000, 0x1000, CRC(3c8fa7f5) SHA1(fd3db6c2abab7000d586ef1a4e425329da292144) )
	ROM_LOAD( "bubblesr.3b",  0x02000, 0x1000, CRC(f869bb9c) SHA1(ce276fc33136a527eefbbf35c2bcf1f0b9858740) )
	ROM_LOAD( "bubblesr.4b",  0x03000, 0x1000, CRC(0c65eaab) SHA1(c622906cbda07421a7024955f3b9e8d173f4b6cb) )
	ROM_LOAD( "bubblesr.5b",  0x04000, 0x1000, CRC(7ece4e13) SHA1(c6ec7145c2d3bf51877c7fb995d9732b09e04cf0) )
	ROM_LOAD( "bubbles.6b",   0x05000, 0x1000, CRC(4dd0450d) SHA1(d55aa8fb8f2974ce5ba7155b01bc3e3622f202af) )
	ROM_LOAD( "bubbles.7b",   0x06000, 0x1000, CRC(e0a26ec0) SHA1(2da6213df6c15735a8bbd6750cfb1a1b6232a6f5) ) // = bub_prot.7b
	ROM_LOAD( "bubblesr.8b",  0x07000, 0x1000, CRC(598b9bd6) SHA1(993cc3fac58310d0e617e58e3a0753002b987df1) )
	ROM_LOAD( "bubbles.9b",   0x08000, 0x1000, CRC(b48559fb) SHA1(551a49a12353044dbbf28dba2bd860c2d00c50bd) )
	ROM_LOAD( "bubblesr.10b", 0x0d000, 0x1000, CRC(8b396db0) SHA1(88cab59ce7f07dfa15d1485d12ebab96d777ca65) )
	ROM_LOAD( "bubblesr.11b", 0x0e000, 0x1000, CRC(096af43e) SHA1(994e60c1e684ae46ea791b274995d21ff5052e56) )
	ROM_LOAD( "bubblesr.12b", 0x0f000, 0x1000, CRC(5c1244ef) SHA1(25b0f359c28291894381d73f4ba3a2b991a547f0) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "video_sound_rom_5_std_771.ic12",  0xf000, 0x1000, CRC(689ce2aa) SHA1(b70d2553f731f9a20ddaf9af2f93b7e9c44d4d99) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder_rom_4.3g", 0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) ) // Universal Horizontal decoder ROM - 7641-5 BPROM - P/N A-5342-09694
	ROM_LOAD( "decoder_rom_6.3c", 0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) ) // Universal Vertical decoder ROM - 7641-5 BPROM - P/N A-5342-09821
ROM_END

ROM_START( bubblesp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bub_prot.1b",  0x00000, 0x1000, CRC(6466a746) SHA1(ed67d879d82ef05bcd2b655f761f84bc0cf08897) )
	ROM_LOAD( "bub_prot.2b",  0x01000, 0x1000, CRC(cca04357) SHA1(98f879675c02e7ad5532da30f663714913a059b9) )
	ROM_LOAD( "bub_prot.3b",  0x02000, 0x1000, CRC(7aaff9e5) SHA1(8b377ec5c595a4e062bdc8fb8ca99b52a6bd9298) )
	ROM_LOAD( "bub_prot.4b",  0x03000, 0x1000, CRC(4e264f01) SHA1(a6fd2d0613f78c45b3873e06efa2dd99530ed0c8) )
	ROM_LOAD( "bub_prot.5b",  0x04000, 0x1000, CRC(121b0be6) SHA1(75ed718b9e83c32390ee0fe2c34e0300ecd98a85) )
	ROM_LOAD( "bub_prot.6b",  0x05000, 0x1000, CRC(80e90b25) SHA1(92c83b4333f4f0f65638b1827ace01b02c490339) )
	ROM_LOAD( "bub_prot.7b",  0x06000, 0x1000, CRC(e0a26ec0) SHA1(2da6213df6c15735a8bbd6750cfb1a1b6232a6f5) )
	ROM_LOAD( "bub_prot.8b",  0x07000, 0x1000, CRC(96fb19c8) SHA1(3b1720e5efe2adc1f633216419bdf00c7e7b817d) )
	ROM_LOAD( "bub_prot.9b",  0x08000, 0x1000, CRC(be7e1028) SHA1(430b33c8d83ee6756a3ef9298792b71066c88326) )
	ROM_LOAD( "bub_prot.10b", 0x0d000, 0x1000, CRC(89a565df) SHA1(1f02c17222f7303218962fada6c6f867414551cf) )
	ROM_LOAD( "bub_prot.11b", 0x0e000, 0x1000, CRC(5a0c36a7) SHA1(2b9dd9006e57ff8214ad4e6b10a4b72e736d472c) )
	ROM_LOAD( "bub_prot.12b", 0x0f000, 0x1000, CRC(2bfd3438) SHA1(2427a5614e98a9499e4d19f9d6e25f2b73896bf5) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "video_sound_rom_5_std_771.ic12",  0xf000, 0x1000, CRC(689ce2aa) SHA1(b70d2553f731f9a20ddaf9af2f93b7e9c44d4d99) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder_rom_4.3g", 0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) ) // Universal Horizontal decoder ROM - 7641-5 BPROM - P/N A-5342-09694
	ROM_LOAD( "decoder_rom_6.3c", 0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) ) // Universal Vertical decoder ROM - 7641-5 BPROM - P/N A-5342-09821
ROM_END


/*

Splat! ROM labels are in this format:

+--------------------+
| SPLAT ROM 1B       |   <-- Game name, ROM board number and ROM type (B is 2732)
| (c) 1983 WILLIAMS  |
| ELECTRONICS, INC.  |
|    16-3011-1       |   <-- Williams game number & ROM number
+--------------------+

ROM | Board|  "B" ROMs  |
 ## | Loc. |  label #   |  Part Number
----+------+------------+----------------
  1 |  E4  | 16-3011-1  |  A-5343-10071-B
  2 |  C4  | 16-3011-2  |  A-5343-10072-B
  3 |  A4  | 16-3011-3  |  A-5343-10073-B
  4 |  E5  | 16-3011-4  |  A-5343-10074-B
  5 |  C5  | 16-3011-5  |  A-5343-10075-B
  6 |  A5  | 16-3011-6  |  A-5343-10076-B
  7 |  E6  | 16-3011-7  |  A-5343-10077-B
  8 |  C6  | 16-3011-8  |  A-5343-10078-B
  9 |  A6  | 16-3011-9  |  A-5343-10079-B
 10 |  A7  | 16-3011-10 |  A-5343-10080-B
 11 |  C7  | 16-3011-11 |  A-5343-10081-B
 12 |  E7  | 16-3011-12 |  A-5343-10082-B

Uses a standard D-9144 ROM Board Assembly, see Joust or Robotron above

*/
ROM_START( splat ) // Solid Brown labels
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "splat_rom_1b_16-3011-1.e4",   0x00000, 0x1000, CRC(1cf26e48) SHA1(6ba4de6cc7d1359ed450da7bae1000552373f873) )
	ROM_LOAD( "splat_rom_2b_16-3011-2.c4",   0x01000, 0x1000, CRC(ac0d4276) SHA1(710aba98909d5d63c4b9b08579021f9c026b3111) )
	ROM_LOAD( "splat_rom_3b_16-3011-3.a4",   0x02000, 0x1000, CRC(74873e59) SHA1(727c9da682fd10353f3969ef02e9f1826d8cb77a) )
	ROM_LOAD( "splat_rom_4b_16-3011-4.e5",   0x03000, 0x1000, CRC(70a7064e) SHA1(7e6440585462b68b62d6d571d83635bf17149f1a) )
	ROM_LOAD( "splat_rom_5b_16-3011-5.c5",   0x04000, 0x1000, CRC(c6895221) SHA1(6f88ba8ac72d9301760d6e2512549f70b5373c65) )
	ROM_LOAD( "splat_rom_6b_16-3011-6.a5",   0x05000, 0x1000, CRC(ea4ab7fd) SHA1(288a361691a7f147ff3346627a10531d613ad017) )
	ROM_LOAD( "splat_rom_7b_16-3011-7.e6",   0x06000, 0x1000, CRC(82fd8713) SHA1(c4d42b111a0357700ac2bf700117d75ffb3c5be5) )
	ROM_LOAD( "splat_rom_8b_16-3011-8.c6",   0x07000, 0x1000, CRC(7dded1b4) SHA1(73df546dd60870f63a8c3deffea2b2d13149a48b) )
	ROM_LOAD( "splat_rom_9b_16-3011-9.a6",   0x08000, 0x1000, CRC(71cbfe5a) SHA1(bf22bedeceffdccc340637098070b32e9c13cf68) )
	ROM_LOAD( "splat_rom_10b_16-3011-10.a7", 0x0d000, 0x1000, CRC(d1a1f632) SHA1(de4f5ba2b92c47757dfd2ca810bf8f87338223f7) )
	ROM_LOAD( "splat_rom_11b_16-3011-11.c7", 0x0e000, 0x1000, CRC(ca8cde95) SHA1(8e12f6d9eaf397646691ec5d02963b32973cb32e) )
	ROM_LOAD( "splat_rom_12b_16-3011-12.e7", 0x0f000, 0x1000, CRC(5bee3e60) SHA1(b4ee99fb6c353093faf1e088bab82fec66e785bc) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "video_sound_rom_13_std.ic12", 0xf000, 0x1000, CRC(a878d5f3) SHA1(f3347a354cb54ca228fe0971f0ae3bc778e2aecf) ) // Instruction Manual 16-3011-101 states "ROM 13" P/N A-5342-10127

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder_rom_4.3g", 0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) ) // Universal Horizontal decoder ROM - 7641-5 BPROM - P/N A-5342-09694
	ROM_LOAD( "decoder_rom_6.3c", 0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) ) // Universal Vertical decoder ROM - 7641-5 BPROM - P/N A-5342-09821
ROM_END


/*

Sinistar

Multiple different ROM boards are known to exist:

Rev.3:
 ROM board known to come with final production rev. 2 labels with corresponding part numbers and two rev. 3
 upgrade ROMs: white labels and orange stripe:

    SINISTAR          SINISTAR
    ROM 8-B    and    ROM 11-B
    REV. 3            REV. 3

 ROM board known to come with all rev. 2 upgrade ROMs (as described below): white labels and red stripe plus the
 two rev. 3 upgrade ROMs as described above

Rev.2:
 Earlier ROM boards known to have all upgrade styled labels, white with red stripe, in the format:

  SINISTAR
  ROM  1-B
  REV. 2

Although not currently dumped, "true" rev.1 Sinistar ROMs are believed to be numbered 16-3004-23 through 16-3004-33,
  with speech ROMs 16-3004-34 through 16-3004-37  (labels believed to be solid brown)
There is known to be a "perfect" version of Sinistar, that being the original version presented to Williams by the
  dev team. The dev team thought this version had the best game play while Williams decided it was too easy (IE: it
  could be played too long on one quarter)

Sinistar's cockpit cabinet features two sound boards, one for the front speakers and another for the rear.  The rear
  sound board uses a different ROM, Video Sound ROM 10.  It adds a slight delay to some of the sound effects,
  and ignores the extra ship and bounce effects.  It has no speech ROMs.

If you disconnect the speech ROMs from the upright sound board, Video Sound ROM 9 will play two replacement sound
  effects for the Sinistar's missing audio.  Any line of dialogue will be replaced by a generic alarm noise,
  while the Sinistar roar is replaced by a loud square wave synth noise that attempts to emulate the "sini-scream".
  Video Sound Rom 10 disables this functionality so that it doesn't play placeholder sounds in place of speech.

*/
ROM_START( sinistar ) // rev. 3
	ROM_REGION( 0x10000, "maincpu", 0 ) // solid RED labels with final production part numbers
	ROM_LOAD( "sinistar_rom_1-b_16-3004-53.1d",  0x00000, 0x1000, CRC(f6f3a22c) SHA1(026d8cab07734fa294a5645edbe65a904bcbc302) )
	ROM_LOAD( "sinistar_rom_2-b_16-3004-54.1c",  0x01000, 0x1000, CRC(cab3185c) SHA1(423d1e3b0c07333ec582529bc4d0b7baf591820a) )
	ROM_LOAD( "sinistar_rom_3-b_16-3004-55.1a",  0x02000, 0x1000, CRC(1ce1b3cc) SHA1(5bc03d7249529d827dc60c087e074ab3e4ea7361) )
	ROM_LOAD( "sinistar_rom_4-b_16-3004-56.2d",  0x03000, 0x1000, CRC(6da632ba) SHA1(72c0c3d5a5ca87ca4d95fcedaf834206e4633950) )
	ROM_LOAD( "sinistar_rom_5-b_16-3004-57.2c",  0x04000, 0x1000, CRC(b662e8fc) SHA1(828a89d2ea13d8a362dae708f86bff54cb231887) )
	ROM_LOAD( "sinistar_rom_6-b_16-3004-58.2a",  0x05000, 0x1000, CRC(2306183d) SHA1(703e29e6446856615760a4897c0f5d79cc7bdfb2) )
	ROM_LOAD( "sinistar_rom_7-b_16-3004-59.3d",  0x06000, 0x1000, CRC(e5dd918e) SHA1(bf4e2ada6a59d246218544d822ba5355da925924) )
	ROM_LOAD( "sinistar_rom_8-b_16-3004-60.3c",  0x07000, 0x1000, CRC(4785a787) SHA1(8c7eca656b2c23b0da41a8c7ce51a2735cab85a4) )
	ROM_LOAD( "sinistar_rom_9-b_16-3004-61.3a",  0x08000, 0x1000, CRC(50cb63ad) SHA1(96e28e4fef98fff2649741a266fa590e0313e3b0) )
	ROM_LOAD( "sinistar_rom_10-b_16-3004-62.4c", 0x0e000, 0x1000, CRC(3d670417) SHA1(81802622bee8dbea5c0f08019d87d941dcdbe292) )
	ROM_LOAD( "sinistar_rom_11-b_16-3004-63.4a", 0x0f000, 0x1000, CRC(3162bc50) SHA1(2f38e572ab9c731e38dfe9bad3cc8222a775c5ea) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "3004_speech_ic7_r1_16-3004-52.ic7", 0xb000, 0x1000, CRC(e1019568) SHA1(442f4f3ccd2e1db2136d2ffb121ea442921f87ca) )
	ROM_LOAD( "3004_speech_ic5_r1_16-3004-50.ic5", 0xc000, 0x1000, CRC(cf3b5ffd) SHA1(d5d51c550581c9d46ab331dd4fd32541a2ef598e) )
	ROM_LOAD( "3004_speech_ic6_r1_16-3004-51.ic6", 0xd000, 0x1000, CRC(ff8d2645) SHA1(16fa2a602acbbc182dd96bab113ab18356f3daf0) )
	ROM_LOAD( "3004_speech_ic4_r1_16-3004-49.ic4", 0xe000, 0x1000, CRC(4b56a626) SHA1(44430cd5c110ec751b0bfb8ae99b26d443350db1) )
	ROM_LOAD( "video_sound_rom_9_std.808.ic12",    0xf000, 0x1000, CRC(b82f4ddb) SHA1(c70c7dd6e88897920d7709a260f27810f66aade1) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder_rom_4.3g", 0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) ) // Universal Horizontal decoder ROM - 7641-5 BPROM - P/N A-5342-09694
	ROM_LOAD( "decoder_rom_6.3c", 0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) ) // Universal Vertical decoder ROM - 7641-5 BPROM - P/N A-5342-09821
ROM_END

ROM_START( sinistarc ) // rev. 3
	ROM_REGION( 0x10000, "maincpu", 0 ) // solid RED labels with final production part numbers
	ROM_LOAD( "sinistar_rom_1-b_16-3004-53.1d",  0x00000, 0x1000, CRC(f6f3a22c) SHA1(026d8cab07734fa294a5645edbe65a904bcbc302) )
	ROM_LOAD( "sinistar_rom_2-b_16-3004-54.1c",  0x01000, 0x1000, CRC(cab3185c) SHA1(423d1e3b0c07333ec582529bc4d0b7baf591820a) )
	ROM_LOAD( "sinistar_rom_3-b_16-3004-55.1a",  0x02000, 0x1000, CRC(1ce1b3cc) SHA1(5bc03d7249529d827dc60c087e074ab3e4ea7361) )
	ROM_LOAD( "sinistar_rom_4-b_16-3004-56.2d",  0x03000, 0x1000, CRC(6da632ba) SHA1(72c0c3d5a5ca87ca4d95fcedaf834206e4633950) )
	ROM_LOAD( "sinistar_rom_5-b_16-3004-57.2c",  0x04000, 0x1000, CRC(b662e8fc) SHA1(828a89d2ea13d8a362dae708f86bff54cb231887) )
	ROM_LOAD( "sinistar_rom_6-b_16-3004-58.2a",  0x05000, 0x1000, CRC(2306183d) SHA1(703e29e6446856615760a4897c0f5d79cc7bdfb2) )
	ROM_LOAD( "sinistar_rom_7-b_16-3004-59.3d",  0x06000, 0x1000, CRC(e5dd918e) SHA1(bf4e2ada6a59d246218544d822ba5355da925924) )
	ROM_LOAD( "sinistar_rom_8-b_16-3004-60.3c",  0x07000, 0x1000, CRC(4785a787) SHA1(8c7eca656b2c23b0da41a8c7ce51a2735cab85a4) )
	ROM_LOAD( "sinistar_rom_9-b_16-3004-61.3a",  0x08000, 0x1000, CRC(50cb63ad) SHA1(96e28e4fef98fff2649741a266fa590e0313e3b0) )
	ROM_LOAD( "sinistar_rom_10-b_16-3004-62.4c", 0x0e000, 0x1000, CRC(3d670417) SHA1(81802622bee8dbea5c0f08019d87d941dcdbe292) )
	ROM_LOAD( "sinistar_rom_11-b_16-3004-63.4a", 0x0f000, 0x1000, CRC(3162bc50) SHA1(2f38e572ab9c731e38dfe9bad3cc8222a775c5ea) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "3004_speech_ic7_r1_16-3004-52.ic7", 0xb000, 0x1000, CRC(e1019568) SHA1(442f4f3ccd2e1db2136d2ffb121ea442921f87ca) )
	ROM_LOAD( "3004_speech_ic5_r1_16-3004-50.ic5", 0xc000, 0x1000, CRC(cf3b5ffd) SHA1(d5d51c550581c9d46ab331dd4fd32541a2ef598e) )
	ROM_LOAD( "3004_speech_ic6_r1_16-3004-51.ic6", 0xd000, 0x1000, CRC(ff8d2645) SHA1(16fa2a602acbbc182dd96bab113ab18356f3daf0) )
	ROM_LOAD( "3004_speech_ic4_r1_16-3004-49.ic4", 0xe000, 0x1000, CRC(4b56a626) SHA1(44430cd5c110ec751b0bfb8ae99b26d443350db1) )
	ROM_LOAD( "video_sound_rom_9_std.808.ic12",    0xf000, 0x1000, CRC(b82f4ddb) SHA1(c70c7dd6e88897920d7709a260f27810f66aade1) )

	ROM_REGION( 0x10000, "soundcpu_b", 0 ) // Stereo sound requires 2nd sound board as used in the cockpit version
	ROM_LOAD( "video_sound_rom_10_std.ic12",       0xf000, 0x1000, CRC(b5c70082) SHA1(643af087b57da3a71c68372c79c5777e0c1fbef7) ) // no speech board is connected

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder_rom_4.3g", 0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) ) // Universal Horizontal decoder ROM - 7641-5 BPROM - P/N A-5342-09694
	ROM_LOAD( "decoder_rom_6.3c", 0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) ) // Universal Vertical decoder ROM - 7641-5 BPROM - P/N A-5342-09821
ROM_END

ROM_START( sinistar2 ) // rev. 2
	ROM_REGION( 0x10000, "maincpu", 0 ) // solid RED labels with final production part numbers
	ROM_LOAD( "sinistar_rom_1-b_16-3004-38.1d",  0x00000, 0x1000, CRC(f6f3a22c) SHA1(026d8cab07734fa294a5645edbe65a904bcbc302) ) //  == rev. 3 PN 16-3004-53
	ROM_LOAD( "sinistar_rom_2-b_16-3004-39.1c",  0x01000, 0x1000, CRC(cab3185c) SHA1(423d1e3b0c07333ec582529bc4d0b7baf591820a) ) //  == rev. 3 PN 16-3004-54
	ROM_LOAD( "sinistar_rom_3-b_16-3004-40.1a",  0x02000, 0x1000, CRC(1ce1b3cc) SHA1(5bc03d7249529d827dc60c087e074ab3e4ea7361) ) //  == rev. 3 PN 16-3004-55
	ROM_LOAD( "sinistar_rom_4-b_16-3004-41.2d",  0x03000, 0x1000, CRC(6da632ba) SHA1(72c0c3d5a5ca87ca4d95fcedaf834206e4633950) ) //  == rev. 3 PN 16-3004-56
	ROM_LOAD( "sinistar_rom_5-b_16-3004-42.2c",  0x04000, 0x1000, CRC(b662e8fc) SHA1(828a89d2ea13d8a362dae708f86bff54cb231887) ) //  == rev. 3 PN 16-3004-57
	ROM_LOAD( "sinistar_rom_6-b_16-3004-43.2a",  0x05000, 0x1000, CRC(2306183d) SHA1(703e29e6446856615760a4897c0f5d79cc7bdfb2) ) //  == rev. 3 PN 16-3004-57
	ROM_LOAD( "sinistar_rom_7-b_16-3004-44.3d",  0x06000, 0x1000, CRC(e5dd918e) SHA1(bf4e2ada6a59d246218544d822ba5355da925924) ) //  == rev. 3 PN 16-3004-59
	ROM_LOAD( "sinistar_rom_8-b_16-3004-45.3c",  0x07000, 0x1000, CRC(d7ecee45) SHA1(f9552035409bce0a36ed93a677b28f8cd361f8f1) ) //  unique to rev. 2
	ROM_LOAD( "sinistar_rom_9-b_16-3004-46.3a",  0x08000, 0x1000, CRC(50cb63ad) SHA1(96e28e4fef98fff2649741a266fa590e0313e3b0) ) //  == rev. 3 PN 16-3004-61
	ROM_LOAD( "sinistar_rom_10-b_16-3004-47.4c", 0x0e000, 0x1000, CRC(3d670417) SHA1(81802622bee8dbea5c0f08019d87d941dcdbe292) ) //  == rev. 3 PN 16-3004-62
	ROM_LOAD( "sinistar_rom_11-b_16-3004-48.4a", 0x0f000, 0x1000, CRC(792c8b00) SHA1(1f847ca8a67595927c36d69cead02813c2431c7b) ) //  unique to rev. 2

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "3004_speech_ic7_r1_16-3004-52.ic7", 0xb000, 0x1000, CRC(e1019568) SHA1(442f4f3ccd2e1db2136d2ffb121ea442921f87ca) )
	ROM_LOAD( "3004_speech_ic5_r1_16-3004-50.ic5", 0xc000, 0x1000, CRC(cf3b5ffd) SHA1(d5d51c550581c9d46ab331dd4fd32541a2ef598e) )
	ROM_LOAD( "3004_speech_ic6_r1_16-3004-51.ic6", 0xd000, 0x1000, CRC(ff8d2645) SHA1(16fa2a602acbbc182dd96bab113ab18356f3daf0) )
	ROM_LOAD( "3004_speech_ic4_r1_16-3004-49.ic4", 0xe000, 0x1000, CRC(4b56a626) SHA1(44430cd5c110ec751b0bfb8ae99b26d443350db1) )
	ROM_LOAD( "video_sound_rom_9_std.808.ic12",    0xf000, 0x1000, CRC(b82f4ddb) SHA1(c70c7dd6e88897920d7709a260f27810f66aade1) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder_rom_4.3g", 0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) ) // Universal Horizontal decoder ROM - 7641-5 BPROM - P/N A-5342-09694
	ROM_LOAD( "decoder_rom_6.3c", 0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) ) // Universal Vertical decoder ROM - 7641-5 BPROM - P/N A-5342-09821
ROM_END

ROM_START( sinistarc2 ) // rev. 2
	ROM_REGION( 0x10000, "maincpu", 0 ) // solid RED labels with final production part numbers
	ROM_LOAD( "sinistar_rom_1-b_16-3004-38.1d",  0x00000, 0x1000, CRC(f6f3a22c) SHA1(026d8cab07734fa294a5645edbe65a904bcbc302) ) //  == rev. 3 PN 16-3004-53
	ROM_LOAD( "sinistar_rom_2-b_16-3004-39.1c",  0x01000, 0x1000, CRC(cab3185c) SHA1(423d1e3b0c07333ec582529bc4d0b7baf591820a) ) //  == rev. 3 PN 16-3004-54
	ROM_LOAD( "sinistar_rom_3-b_16-3004-40.1a",  0x02000, 0x1000, CRC(1ce1b3cc) SHA1(5bc03d7249529d827dc60c087e074ab3e4ea7361) ) //  == rev. 3 PN 16-3004-55
	ROM_LOAD( "sinistar_rom_4-b_16-3004-41.2d",  0x03000, 0x1000, CRC(6da632ba) SHA1(72c0c3d5a5ca87ca4d95fcedaf834206e4633950) ) //  == rev. 3 PN 16-3004-56
	ROM_LOAD( "sinistar_rom_5-b_16-3004-42.2c",  0x04000, 0x1000, CRC(b662e8fc) SHA1(828a89d2ea13d8a362dae708f86bff54cb231887) ) //  == rev. 3 PN 16-3004-57
	ROM_LOAD( "sinistar_rom_6-b_16-3004-43.2a",  0x05000, 0x1000, CRC(2306183d) SHA1(703e29e6446856615760a4897c0f5d79cc7bdfb2) ) //  == rev. 3 PN 16-3004-57
	ROM_LOAD( "sinistar_rom_7-b_16-3004-44.3d",  0x06000, 0x1000, CRC(e5dd918e) SHA1(bf4e2ada6a59d246218544d822ba5355da925924) ) //  == rev. 3 PN 16-3004-59
	ROM_LOAD( "sinistar_rom_8-b_16-3004-45.3c",  0x07000, 0x1000, CRC(d7ecee45) SHA1(f9552035409bce0a36ed93a677b28f8cd361f8f1) ) //  unique to rev. 2
	ROM_LOAD( "sinistar_rom_9-b_16-3004-46.3a",  0x08000, 0x1000, CRC(50cb63ad) SHA1(96e28e4fef98fff2649741a266fa590e0313e3b0) ) //  == rev. 3 PN 16-3004-61
	ROM_LOAD( "sinistar_rom_10-b_16-3004-47.4c", 0x0e000, 0x1000, CRC(3d670417) SHA1(81802622bee8dbea5c0f08019d87d941dcdbe292) ) //  == rev. 3 PN 16-3004-62
	ROM_LOAD( "sinistar_rom_11-b_16-3004-48.4a", 0x0f000, 0x1000, CRC(792c8b00) SHA1(1f847ca8a67595927c36d69cead02813c2431c7b) ) //  unique to rev. 2

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "3004_speech_ic7_r1_16-3004-52.ic7", 0xb000, 0x1000, CRC(e1019568) SHA1(442f4f3ccd2e1db2136d2ffb121ea442921f87ca) )
	ROM_LOAD( "3004_speech_ic5_r1_16-3004-50.ic5", 0xc000, 0x1000, CRC(cf3b5ffd) SHA1(d5d51c550581c9d46ab331dd4fd32541a2ef598e) )
	ROM_LOAD( "3004_speech_ic6_r1_16-3004-51.ic6", 0xd000, 0x1000, CRC(ff8d2645) SHA1(16fa2a602acbbc182dd96bab113ab18356f3daf0) )
	ROM_LOAD( "3004_speech_ic4_r1_16-3004-49.ic4", 0xe000, 0x1000, CRC(4b56a626) SHA1(44430cd5c110ec751b0bfb8ae99b26d443350db1) )
	ROM_LOAD( "video_sound_rom_9_std.808.ic12",    0xf000, 0x1000, CRC(b82f4ddb) SHA1(c70c7dd6e88897920d7709a260f27810f66aade1) )

	ROM_REGION( 0x10000, "soundcpu_b", 0 ) // Stereo sound requires 2nd sound board as used in the cockpit version
	ROM_LOAD( "video_sound_rom_10_std.ic12",       0xf000, 0x1000, CRC(b5c70082) SHA1(643af087b57da3a71c68372c79c5777e0c1fbef7) ) // no speech board is connected

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder_rom_4.3g", 0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) ) // Universal Horizontal decoder ROM - 7641-5 BPROM - P/N A-5342-09694
	ROM_LOAD( "decoder_rom_6.3c", 0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) ) // Universal Vertical decoder ROM - 7641-5 BPROM - P/N A-5342-09821
ROM_END

ROM_START( sinistarp ) // solid pink labels - 1982 AMOA prototype
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sinistar_rom_1-b_16-3004-12.1d",  0x00000, 0x1000, CRC(3810d7b8) SHA1(dcd690cbc958a2f97f022765315d77fb7c7d8e8b) )
	ROM_LOAD( "sinistar_rom_2-b_16-3004-13.1c",  0x01000, 0x1000, CRC(cab3185c) SHA1(423d1e3b0c07333ec582529bc4d0b7baf591820a) ) // only this one ROM remains the same through to rev. 3
	ROM_LOAD( "sinistar_rom_3-b_16-3004-14.1a",  0x02000, 0x1000, CRC(7c984ca9) SHA1(b32b7d15194051db5d29acf95b049e2eccf6d393) )
	ROM_LOAD( "sinistar_rom_4-b_16-3004-15.2d",  0x03000, 0x1000, CRC(cc6c4f24) SHA1(b4375544e02a19458c6fcc85edb31025c0b8eb71) )
	ROM_LOAD( "sinistar_rom_5-b_16-3004-16.2c",  0x04000, 0x1000, CRC(12285bfe) SHA1(6d433103332ddda2f2af23febc0b15aa93db1f31) )
	ROM_LOAD( "sinistar_rom_6-b_16-3004-17.2a",  0x05000, 0x1000, CRC(7a675f35) SHA1(3a7e9fdb2aef52dc29d33799694737038802b6e0) )
	ROM_LOAD( "sinistar_rom_7-b_16-3004-18.3d",  0x06000, 0x1000, CRC(b0463243) SHA1(95d597856a1942bd176f5f62db0d691f8f2f2932) )
	ROM_LOAD( "sinistar_rom_8-b_16-3004-19.3c",  0x07000, 0x1000, CRC(909040d4) SHA1(5361cc378bdace0799227e901341747dce9bb029) )
	ROM_LOAD( "sinistar_rom_9-b_16-3004-20.3a",  0x08000, 0x1000, CRC(cc949810) SHA1(2d2d1cccd7e43b63e424c34ab5215a412e2b9809) )
	ROM_LOAD( "sinistar_rom_10-b_16-3004-21.4c", 0x0e000, 0x1000, CRC(ea87a53f) SHA1(4e4bad5315a8f5740c926ee5681879919a5be37f) )
	ROM_LOAD( "sinistar_rom_11-b_16-3004-22.4a", 0x0f000, 0x1000, CRC(88d36e80) SHA1(bb9adaf5b73f9874e52dc2f5fd35e22f8b4fc258) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "3004_speech_ic7_r1.ic7",    0xb000, 0x1000, CRC(e1019568) SHA1(442f4f3ccd2e1db2136d2ffb121ea442921f87ca) ) // same data as later sets, but no official part number assigned yet
	ROM_LOAD( "3004_speech_ic5_r1.ic5",    0xc000, 0x1000, CRC(cf3b5ffd) SHA1(d5d51c550581c9d46ab331dd4fd32541a2ef598e) )
	ROM_LOAD( "3004_speech_ic6_r1.ic6",    0xd000, 0x1000, CRC(ff8d2645) SHA1(16fa2a602acbbc182dd96bab113ab18356f3daf0) )
	ROM_LOAD( "3004_speech_ic4_r1.ic4",    0xe000, 0x1000, CRC(4b56a626) SHA1(44430cd5c110ec751b0bfb8ae99b26d443350db1) )
	ROM_LOAD( "video_sound_rom_9_std.808", 0xf000, 0x1000, CRC(b82f4ddb) SHA1(c70c7dd6e88897920d7709a260f27810f66aade1) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder_rom_4.3g", 0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) ) // Universal Horizontal decoder ROM - 7641-5 BPROM - P/N A-5342-09694
	ROM_LOAD( "decoder_rom_6.3c", 0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) ) // Universal Vertical decoder ROM - 7641-5 BPROM - P/N A-5342-09821
ROM_END


ROM_START( playball )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "playball.01", 0x00000, 0x1000, CRC(7ba8fd71) SHA1(9b77996238c67aead8c2cfc7f964f8cf9c6182eb) )
	ROM_LOAD( "playball.02", 0x01000, 0x1000, CRC(2387c3d4) SHA1(19d9da6af317595d0f3336e886154e0b8467cb3e) )
	ROM_LOAD( "playball.03", 0x02000, 0x1000, CRC(d34cc5fd) SHA1(d1f6d321c1a6a04a06813c77a3e079836a05956c) )
	ROM_LOAD( "playball.04", 0x03000, 0x1000, CRC(f68c3a8e) SHA1(f9cc7250254b9adceff883d3f6ee01c475d859ec) )
	ROM_LOAD( "playball.05", 0x04000, 0x1000, CRC(a3f20810) SHA1(678d2a5a06263cc5f74f4cb92287cf4d7a8b934f) )
	ROM_LOAD( "playball.06", 0x05000, 0x1000, CRC(f213e48e) SHA1(05b54f5121a887bc24fbe30f322277ae94474c14) )
	ROM_LOAD( "playball.07", 0x06000, 0x1000, CRC(9b5574e9) SHA1(1dddd33cd3f13694d7ba6a73e5090594c6677d5b) )
	ROM_LOAD( "playball.08", 0x07000, 0x1000, CRC(b2d2074a) SHA1(2defb2ffaca782606f792020f9c96d41abd77518) )
	ROM_LOAD( "playball.09", 0x08000, 0x1000, CRC(c4566d0f) SHA1(7848ea87d2d1693ade9129846024fbedc4145cbb) )
	ROM_LOAD( "playball.10", 0x0d000, 0x1000, CRC(18787b52) SHA1(621754c1eab68de12763616b7bf01948cdce0221) )
	ROM_LOAD( "playball.11", 0x0e000, 0x1000, CRC(1dd5c8f2) SHA1(17d0380ea05d9ddd17576691d0e5179ae7a71200) )
	ROM_LOAD( "playball.12", 0x0f000, 0x1000, CRC(a700597b) SHA1(5ba07409ae9315b9ee65530f61155c394bfc69ad) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "speech.ic4",   0xb000, 0x1000, CRC(7e4fc798) SHA1(4636ab25238503370063f51f86f37d0e49c0d3b6) )
	ROM_LOAD( "speech.ic5",   0xc000, 0x1000, CRC(ddfe860c) SHA1(f847a0a6438af5dc646b7abe994530e6d1cbb803) )
	ROM_LOAD( "speech.ic6",   0xd000, 0x1000, CRC(8bfebf87) SHA1(d6829f78e1a2aee85673a42f7f6b78679847b616) )
	ROM_LOAD( "speech.ic7",   0xe000, 0x1000, CRC(db351db6) SHA1(94d807df61b5015f5fa78a500e2a58277db95c1f) )
	ROM_LOAD( "playball.snd", 0xf000, 0x1000, CRC(f3076f9f) SHA1(436fb1a6456535cd27f85c941ff79c0465b71555) )
ROM_END


ROM_START( blaster ) // 20 Level version - Each ROM label had an additional "PROTO5" or "PROTO6" sticker attached (verified on multiple PCBs)
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "proto6_blaster_3021_rom_11.ic25", 0x04000, 0x2000, CRC(6371e62f) SHA1(dc4173d2ee88757a6ac0838acaee325eadc2c4fb) ) // labeled:  BLASTER   (3021) ROM 11   (c) 1983 WILLIAMS   ELECTRONICS, INC.   PROTO6
	ROM_LOAD( "proto6_blaster_3021_rom_12.ic26", 0x06000, 0x2000, CRC(9804faac) SHA1(e61218fe190ad268af48d611d140d8f4cd38e4c7) ) // labeled:  BLASTER   (3021) ROM 12   (c) 1983 WILLIAMS   ELECTRONICS, INC.   PROTO6
	ROM_LOAD( "proto6_blaster_3021_rom_17.ic41", 0x08000, 0x1000, CRC(bf96182f) SHA1(e25a02508eecf79ea1ae5d45278a60becc6c7dcc) ) // labeled:  BLASTER   (3021) ROM 17   (c) 1983 WILLIAMS   ELECTRONICS, INC.   PROTO6

	ROM_LOAD( "proto6_blaster_3021_rom_16.ic39", 0x0d000, 0x1000, CRC(54a40b21) SHA1(663c7b539e6f1f065a4ecae7bb0477c71951223f) ) // labeled:  BLASTER   (3021) ROM 16   (c) 1983 WILLIAMS   ELECTRONICS, INC.   PROTO6
	ROM_LOAD( "proto6_blaster_3021_rom_13.ic27", 0x0e000, 0x2000, CRC(f4dae4c8) SHA1(211dcbe085a30419d649afe10ca7c4017d909bd7) ) // labeled:  BLASTER   (3021) ROM 13   (c) 1983 WILLIAMS   ELECTRONICS, INC.   PROTO6

	ROM_LOAD( "proto5_blaster_3021_rom_15.ic38", 0x10000, 0x4000, CRC(1ad146a4) SHA1(5ab3d9618023b59bc329a9eeef986901867a639b) ) // labeled:  BLASTER   (3021) ROM 15   (c) 1983 WILLIAMS   ELECTRONICS, INC.   PROTO5
	ROM_LOAD( "proto5_blaster_3021_rom_8.ic20",  0x14000, 0x4000, CRC(f110bbb0) SHA1(314dea232a3706509399348c7415f933c64cea1b) ) // labeled:  BLASTER   (3021) ROM 8   (c) 1983 WILLIAMS   ELECTRONICS, INC.   PROTO5
	ROM_LOAD( "proto5_blaster_3021_rom_9.ic22",  0x18000, 0x4000, CRC(5c5b0f8a) SHA1(224f89c85b2b1ca511d006180b8d994fccbdfb6b) ) // labeled:  BLASTER   (3021) ROM 9   (c) 1983 WILLIAMS   ELECTRONICS, INC.   PROTO5
	ROM_LOAD( "proto5_blaster_3021_rom_10.ic24", 0x1c000, 0x4000, CRC(d47eb67f) SHA1(5dcde8be1a7b1927b90ffab3219dc47c5b2f20e4) ) // labeled:  BLASTER   (3021) ROM 10   (c) 1983 WILLIAMS   ELECTRONICS, INC.   PROTO5
	ROM_LOAD( "proto5_blaster_3021_rom_6.ic13",  0x20000, 0x4000, CRC(47fc007e) SHA1(3a80b9b7ae460e9732f7c1cdd465a5b06ded970f) ) // labeled:  BLASTER   (3021) ROM 6   (c) 1983 WILLIAMS   ELECTRONICS, INC.   PROTO5
	ROM_LOAD( "proto5_blaster_3021_rom_5.ic11",  0x24000, 0x4000, CRC(15c1b94d) SHA1(5d97628541eb8933870c3ffd3646b7aaf8af6af5) ) // labeled:  BLASTER   (3021) ROM 5   (c) 1983 WILLIAMS   ELECTRONICS, INC.   PROTO5
	ROM_LOAD( "proto5_blaster_3021_rom_14.ic35", 0x28000, 0x4000, CRC(aea6b846) SHA1(04cb4b5eb000471a0cec377a5236ac8c83529528) ) // labeled:  BLASTER   (3021) ROM 14   (c) 1983 WILLIAMS   ELECTRONICS, INC.   PROTO5
	ROM_LOAD( "proto6_blaster_3021_rom_7.ic15",  0x2c000, 0x4000, CRC(7a101181) SHA1(5f1581911ea7fe3e63ce1b9c50b1d3bf081dbf81) ) // labeled:  BLASTER   (3021) ROM 7   (c) 1983 WILLIAMS   ELECTRONICS, INC.   PROTO6
	ROM_LOAD( "proto5_blaster_3021_rom_1.ic1",   0x30000, 0x4000, CRC(8d0ea9e7) SHA1(34f8e2e99748bed29285f7e4929bb920960ab03e) ) // labeled:  BLASTER   (3021) ROM 1   (c) 1983 WILLIAMS   ELECTRONICS, INC.   PROTO5
	ROM_LOAD( "proto5_blaster_3021_rom_2.ic3",   0x34000, 0x4000, CRC(03c4012c) SHA1(53f0adc91e5f1ac58b08b3a6d2de8de5a40bebab) ) // labeled:  BLASTER   (3021) ROM 2   (c) 1983 WILLIAMS   ELECTRONICS, INC.   PROTO5
	ROM_LOAD( "proto6_blaster_3021_rom_4.ic7",   0x38000, 0x4000, CRC(fc9d39fb) SHA1(126d43a64471bbf4b40aeda8913d50e82d254f9c) ) // labeled:  BLASTER   (3021) ROM 4   (c) 1983 WILLIAMS   ELECTRONICS, INC.   PROTO6
	ROM_LOAD( "proto6_blaster_3021_rom_3.ic6",   0x3c000, 0x4000, CRC(253690fb) SHA1(06cb2ef95bb06b3618392e298aa690e1f75bc977) ) // labeled:  BLASTER   (3021) ROM 3   (c) 1983 WILLIAMS   ELECTRONICS, INC.   PROTO6

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "proto5_blaster_3021_rom_18.sb13", 0xf000, 0x1000, CRC(c33a3145) SHA1(6ffe2da7b70c0b576fbc1790a33eecdbb9ee3d02) ) // labeled:  BLASTER   (3021) ROM 18   (c) 1983 WILLIAMS   ELECTRONICS, INC.   PROTO5

	ROM_REGION( 0x10000, "soundcpu_b", 0 )
	ROM_LOAD( "proto5_blaster_3021_rom_18.sb10", 0xf000, 0x1000, CRC(c33a3145) SHA1(6ffe2da7b70c0b576fbc1790a33eecdbb9ee3d02) ) // labeled:  BLASTER   (3021) ROM 18   (c) 1983 WILLIAMS   ELECTRONICS, INC.   PROTO5

	ROM_REGION( 0x0c00, "proms", 0 ) // color & video-decoder PROM data
	ROM_LOAD( "decoder_rom_4.ic42",       0x0800, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) ) // labeled:  Decoder Rom 4   (c) 1981 WILLIAMS   ELECTRONICS INC.   STD.729
	ROM_LOAD( "video_decoder_rom_6.ic23", 0x0a00, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) ) // labeled:  Video Decoder Rom 6   (c) 1981 WILLIAMS   ELECTRONICS INC.   STD.746
	ROM_LOAD( "blaster.col",              0x0000, 0x0800, CRC(bac50bc4) SHA1(80a48eb97c6f02703210d00498f9669c36e64326) ) // A-5282-10426 at IC49 & IC50??
ROM_END

ROM_START( blastero ) // 30 Level version
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "proto5_blaster_3021_rom_11.ic25", 0x04000, 0x2000, CRC(bc2d7eda) SHA1(831e9ecb75b143f9770eab1939136092a29e64f7) ) // assumed to be PROTO5 revision
	ROM_LOAD( "proto5_blaster_3021_rom_12.ic26", 0x06000, 0x2000, CRC(8a215017) SHA1(ee9233134907c03f7a1221d9daa84fe047c2db94) ) // assumed to be PROTO5 revision
	ROM_LOAD( "proto5_blaster_3021_rom_17.ic41", 0x08000, 0x1000, CRC(b308f0e5) SHA1(262e25be40dff66e65a0fe34c9d013a750b90876) ) // assumed to be PROTO5 revision

	ROM_LOAD( "proto5_blaster_3021_rom_16.ic39", 0x0d000, 0x1000, CRC(2db032d2) SHA1(287769361639695b1c1ceae0fe6899d83b4575d5) ) // assumed to be PROTO5 revision
	ROM_LOAD( "proto5_blaster_3021_rom_13.ic27", 0x0e000, 0x2000, CRC(c99213c7) SHA1(d1c1549c053de3d862d8ef3ebca02811ed289464) ) // assumed to be PROTO5 revision

	ROM_LOAD( "proto5_blaster_3021_rom_15.ic38", 0x10000, 0x4000, CRC(1ad146a4) SHA1(5ab3d9618023b59bc329a9eeef986901867a639b) )
	ROM_LOAD( "proto5_blaster_3021_rom_8.ic20",  0x14000, 0x4000, CRC(f110bbb0) SHA1(314dea232a3706509399348c7415f933c64cea1b) )
	ROM_LOAD( "proto5_blaster_3021_rom_9.ic22",  0x18000, 0x4000, CRC(5c5b0f8a) SHA1(224f89c85b2b1ca511d006180b8d994fccbdfb6b) )
	ROM_LOAD( "proto5_blaster_3021_rom_10.ic24", 0x1c000, 0x4000, CRC(d47eb67f) SHA1(5dcde8be1a7b1927b90ffab3219dc47c5b2f20e4) )
	ROM_LOAD( "proto5_blaster_3021_rom_6.ic13",  0x20000, 0x4000, CRC(47fc007e) SHA1(3a80b9b7ae460e9732f7c1cdd465a5b06ded970f) )
	ROM_LOAD( "proto5_blaster_3021_rom_5.ic11",  0x24000, 0x4000, CRC(15c1b94d) SHA1(5d97628541eb8933870c3ffd3646b7aaf8af6af5) )
	ROM_LOAD( "proto5_blaster_3021_rom_14.ic35", 0x28000, 0x4000, CRC(aea6b846) SHA1(04cb4b5eb000471a0cec377a5236ac8c83529528) )
	ROM_LOAD( "proto5_blaster_3021_rom_7.ic15",  0x2c000, 0x4000, CRC(a1c4db77) SHA1(7a878d44b6ca7444ecbb6c8f75e5e91de149daf3) ) // assumed to be PROTO5 revision
	ROM_LOAD( "proto5_blaster_3021_rom_1.ic1",   0x30000, 0x4000, CRC(8d0ea9e7) SHA1(34f8e2e99748bed29285f7e4929bb920960ab03e) )
	ROM_LOAD( "proto5_blaster_3021_rom_2.ic3",   0x34000, 0x4000, CRC(03c4012c) SHA1(53f0adc91e5f1ac58b08b3a6d2de8de5a40bebab) )
	ROM_LOAD( "proto5_blaster_3021_rom_4.ic7",   0x38000, 0x4000, CRC(39d2a32c) SHA1(33707877e841ef86a11b47ffabddce7f3d2a7030) ) // assumed to be PROTO5 revision
	ROM_LOAD( "proto5_blaster_3021_rom_3.ic6",   0x3c000, 0x4000, CRC(054c9f1c) SHA1(c21e3493f1ae506ab9fd28ed9ecc67d3305e9d7a) ) // assumed to be PROTO5 revision

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "proto5_blaster_3021_rom_18.sb13", 0xf000, 0x1000, CRC(c33a3145) SHA1(6ffe2da7b70c0b576fbc1790a33eecdbb9ee3d02) ) // labeled:  BLASTER   (3021) ROM 18   (c) 1983 WILLIAMS   ELECTRONICS, INC.   PROTO5

	ROM_REGION( 0x10000, "soundcpu_b", 0 )
	ROM_LOAD( "proto5_blaster_3021_rom_18.sb10", 0xf000, 0x1000, CRC(c33a3145) SHA1(6ffe2da7b70c0b576fbc1790a33eecdbb9ee3d02) ) // labeled:  BLASTER   (3021) ROM 18   (c) 1983 WILLIAMS   ELECTRONICS, INC.   PROTO5

	ROM_REGION( 0x0c00, "proms", 0 ) // color & video-decoder PROM data
	ROM_LOAD( "decoder_rom_4.ic42",       0x0800, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) ) // labeled:  Decoder Rom 4   (c) 1981 WILLIAMS   ELECTRONICS INC.   STD.729
	ROM_LOAD( "video_decoder_rom_6.ic23", 0x0a00, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) ) // labeled:  Video Decoder Rom 6   (c) 1981 WILLIAMS   ELECTRONICS INC.   STD.746
	ROM_LOAD( "blaster.col",              0x0000, 0x0800, CRC(bac50bc4) SHA1(80a48eb97c6f02703210d00498f9669c36e64326) ) // A-5282-10426 at IC49 & IC50??
ROM_END

ROM_START( blasterkit ) // 20 Level version with single sound board & mono sound
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "blastkit_rom_11.ic25", 0x04000, 0x2000, CRC(b7df4914) SHA1(81f7a89dfde06c160f2c8974eec701f2298ec434) ) // unique to this set
	ROM_LOAD( "blastkit_rom_12.ic26", 0x06000, 0x2000, CRC(8b1e26ab) SHA1(7d30800a9302f5a83792499d8df536693d01f75d) ) // unique to this set
	ROM_LOAD( "blastkit_rom_17.ic41", 0x08000, 0x1000, CRC(577d1e9a) SHA1(0064124a65490e0473dfb0081ec28b7ee43a04b5) ) // unique to this set

	ROM_LOAD( "blastkit_rom_16.ic39", 0x0d000, 0x1000, CRC(414b2abf) SHA1(2bde972d225d6e93e44751f542cee584d57f7983) ) // unique to this set
	ROM_LOAD( "blastkit_rom_13.ic27", 0x0e000, 0x2000, CRC(9c64db76) SHA1(c14508cb2f964af93631779db3adaa960fcc7559) ) // unique to this set

	ROM_LOAD( "blastkit_rom_15.ic38", 0x10000, 0x4000, CRC(1ad146a4) SHA1(5ab3d9618023b59bc329a9eeef986901867a639b) )
	ROM_LOAD( "blastkit_rom_8.ic20",  0x14000, 0x4000, CRC(f110bbb0) SHA1(314dea232a3706509399348c7415f933c64cea1b) )
	ROM_LOAD( "blastkit_rom_9.ic22",  0x18000, 0x4000, CRC(5c5b0f8a) SHA1(224f89c85b2b1ca511d006180b8d994fccbdfb6b) )
	ROM_LOAD( "blastkit_rom_10.ic24", 0x1c000, 0x4000, CRC(d47eb67f) SHA1(5dcde8be1a7b1927b90ffab3219dc47c5b2f20e4) )
	ROM_LOAD( "blastkit_rom_6.ic13",  0x20000, 0x4000, CRC(47fc007e) SHA1(3a80b9b7ae460e9732f7c1cdd465a5b06ded970f) )
	ROM_LOAD( "blastkit_rom_5.ic11",  0x24000, 0x4000, CRC(15c1b94d) SHA1(5d97628541eb8933870c3ffd3646b7aaf8af6af5) )
	ROM_LOAD( "blastkit_rom_14.ic35", 0x28000, 0x4000, CRC(aea6b846) SHA1(04cb4b5eb000471a0cec377a5236ac8c83529528) )
	ROM_LOAD( "blastkit_rom_7.ic15",  0x2c000, 0x4000, CRC(6fcc2153) SHA1(00e7b6846c15400315d94e2c7d1c99b1a737c285) ) // unique to this set
	ROM_LOAD( "blastkit_rom_1.ic1",   0x30000, 0x4000, CRC(8d0ea9e7) SHA1(34f8e2e99748bed29285f7e4929bb920960ab03e) )
	ROM_LOAD( "blastkit_rom_2.ic3",   0x34000, 0x4000, CRC(03c4012c) SHA1(53f0adc91e5f1ac58b08b3a6d2de8de5a40bebab) )
	ROM_LOAD( "blastkit_rom_4.ic7",   0x38000, 0x4000, CRC(f80e9ff5) SHA1(e232d96b6e07c7b4240fa4dd2cb9be4745a1be4b) ) // unique to this set
	ROM_LOAD( "blastkit_rom_3.ic6",   0x3c000, 0x4000, CRC(20e851f9) SHA1(efc288ef0333812a6282f22aade8e43e9a827533) ) // unique to this set

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "blastkit_rom_18", 0xf000, 0x1000, CRC(c33a3145) SHA1(6ffe2da7b70c0b576fbc1790a33eecdbb9ee3d02) )

	ROM_REGION( 0x0c00, "proms", 0 ) // color & video-decoder PROM data
	ROM_LOAD( "decoder_rom_4.ic42",       0x0800, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) ) // labeled:  Decoder Rom 4   (c) 1981 WILLIAMS   ELECTRONICS INC.   STD.729
	ROM_LOAD( "video_decoder_rom_6.ic23", 0x0a00, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) ) // labeled:  Video Decoder Rom 6   (c) 1981 WILLIAMS   ELECTRONICS INC.   STD.746
	ROM_LOAD( "blaster.col",              0x0000, 0x0800, CRC(bac50bc4) SHA1(80a48eb97c6f02703210d00498f9669c36e64326) ) // A-5282-10426 at IC49 & IC50??
ROM_END


ROM_START( spdball )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "speedbal.01", 0x00000, 0x1000, CRC(7f4801bb) SHA1(8f22396170571189b1d088d73331d6a713c76f41) )
	ROM_LOAD( "speedbal.02", 0x01000, 0x1000, CRC(5cd5e489) SHA1(83c1bce945ecbaa4a59e0023198e574d9069680c) )
	ROM_LOAD( "speedbal.03", 0x02000, 0x1000, CRC(280e11a4) SHA1(4ef321e1744955a9a54c1e4b1f88c01c01e7b7c8) )
	ROM_LOAD( "speedbal.04", 0x03000, 0x1000, CRC(3469cbbf) SHA1(70b46cf686438441484ffeca0fa1398c15c8811e) )
	ROM_LOAD( "speedbal.05", 0x04000, 0x1000, CRC(87373c89) SHA1(a3cd72f4b517d5d727059a7d911b79ced27e9f93) )
	ROM_LOAD( "speedbal.06", 0x05000, 0x1000, CRC(48779a0d) SHA1(9cdfc12d1021b5d66acd38ab61f385219be39f4f) )
	ROM_LOAD( "speedbal.07", 0x06000, 0x1000, CRC(2e5d8db6) SHA1(7a13d60267ce12a6a4b20322c2ed1f39762bc663) )
	ROM_LOAD( "speedbal.08", 0x07000, 0x1000, CRC(c173cedf) SHA1(603c4c7cdc712d54a86b59470651d00b369293d8) )
	ROM_LOAD( "speedbal.09", 0x08000, 0x1000, CRC(415f424b) SHA1(f7e59385a67319ba152488762af1b42fc62ab264) )
	ROM_LOAD( "speedbal.10", 0x0d000, 0x1000, CRC(4a3add93) SHA1(6939dd6cb6751a0406f364223029eff99040f9e2) )
	ROM_LOAD( "speedbal.11", 0x0e000, 0x1000, CRC(1fbcfaa5) SHA1(fccdebbab172b141bbaec6f520b378d21c72f67a) )
	ROM_LOAD( "speedbal.12", 0x0f000, 0x1000, CRC(f3458f41) SHA1(366fb880b4dc68849d6ea7a9dab55efa9c566123) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "speedbal.snd", 0xf000, 0x1000, CRC(78de20e2) SHA1(ece6e04b1d57167faf7aaee0829e7c31eb560437) )

	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD( "mystery.rom", 0x00000, 0x1000, CRC(dcb6a070) SHA1(6a6fcddf5b46eef187dcf5d9b60e03e9375e7276) )
ROM_END


ROM_START( alienar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "aarom01",   0x00000, 0x1000, CRC(bb0c21be) SHA1(dbf122870adaa49cd99e2c1e9fa4b78fb74ef2c1) )
	ROM_LOAD( "aarom02",   0x01000, 0x1000, CRC(165acd37) SHA1(12466c94bcf5a98f154a639ecc2e95d5193cbab2) )
	ROM_LOAD( "aarom03",   0x02000, 0x1000, CRC(e5d51d92) SHA1(598c928499e977a30906319c97ffa1ef2b9395d1) )
	ROM_LOAD( "aarom04",   0x03000, 0x1000, CRC(24f6feb8) SHA1(c1b7d764785b4edfe80a90ffdc52a67c8dbbfea5) )
	ROM_LOAD( "aarom05",   0x04000, 0x1000, CRC(5b1ac59b) SHA1(9b312eb419e994a006fda2ae61c58c31f048bace) )
	ROM_LOAD( "aarom06",   0x05000, 0x1000, CRC(da7195a2) SHA1(ef2c2750c504176fd6a11e8463278d97cac9a5c5) )
	ROM_LOAD( "aarom07",   0x06000, 0x1000, CRC(f9812be4) SHA1(5f116345f09cd79790612672aa48ede63fc91f56) )
	ROM_LOAD( "aarom08",   0x07000, 0x1000, CRC(cd7f3a87) SHA1(59577059308931139ecc036d06953660a148d91c) )
	ROM_LOAD( "aarom09",   0x08000, 0x1000, CRC(e6ce77b4) SHA1(bd4354100067654d0ad2e590591582dbdfb845b6) )
	ROM_LOAD( "aarom10",   0x0d000, 0x1000, CRC(6feb0314) SHA1(5cf30f097bc695cbd388cb408e78394926362a7b) )
	ROM_LOAD( "aarom11",   0x0e000, 0x1000, CRC(ae3a270e) SHA1(867fff32062bc876390e8ca6bd7cedae47cd92c9) )
	ROM_LOAD( "aarom12",   0x0f000, 0x1000, CRC(6be9f09e) SHA1(98821c9b94301c5fd6e7f5d9e4bc9c1bdbab53ec) )

	ROM_REGION( 0x10000, "soundcpu", ROMREGION_ERASE00 )
ROM_END


ROM_START( alienaru )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "aarom01",   0x00000, 0x1000, CRC(bb0c21be) SHA1(dbf122870adaa49cd99e2c1e9fa4b78fb74ef2c1) )
	ROM_LOAD( "aarom02",   0x01000, 0x1000, CRC(165acd37) SHA1(12466c94bcf5a98f154a639ecc2e95d5193cbab2) )
	ROM_LOAD( "aarom03",   0x02000, 0x1000, CRC(e5d51d92) SHA1(598c928499e977a30906319c97ffa1ef2b9395d1) )
	ROM_LOAD( "aarom04",   0x03000, 0x1000, CRC(24f6feb8) SHA1(c1b7d764785b4edfe80a90ffdc52a67c8dbbfea5) )
	ROM_LOAD( "aarom05",   0x04000, 0x1000, CRC(5b1ac59b) SHA1(9b312eb419e994a006fda2ae61c58c31f048bace) )
	ROM_LOAD( "aarom06",   0x05000, 0x1000, CRC(da7195a2) SHA1(ef2c2750c504176fd6a11e8463278d97cac9a5c5) )
	ROM_LOAD( "aarom07",   0x06000, 0x1000, CRC(f9812be4) SHA1(5f116345f09cd79790612672aa48ede63fc91f56) )
	ROM_LOAD( "aarom08",   0x07000, 0x1000, CRC(cd7f3a87) SHA1(59577059308931139ecc036d06953660a148d91c) )
	ROM_LOAD( "aarom09",   0x08000, 0x1000, CRC(e6ce77b4) SHA1(bd4354100067654d0ad2e590591582dbdfb845b6) )
	ROM_LOAD( "aarom10",   0x0d000, 0x1000, CRC(6feb0314) SHA1(5cf30f097bc695cbd388cb408e78394926362a7b) )
	ROM_LOAD( "aarom11",   0x0e000, 0x1000, CRC(ae3a270e) SHA1(867fff32062bc876390e8ca6bd7cedae47cd92c9) )
	ROM_LOAD( "aarom12",   0x0f000, 0x1000, CRC(6be9f09e) SHA1(98821c9b94301c5fd6e7f5d9e4bc9c1bdbab53ec) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "sg.snd",    0xf800, 0x0800, CRC(2fcf6c4d) SHA1(9c4334ac3ff15d94001b22fc367af40f9deb7d57) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder.4", 0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) )
	ROM_LOAD( "decoder.6", 0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) )
ROM_END


ROM_START( lottofun )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vl4e.dat",    0x00000, 0x1000, CRC(5e9af236) SHA1(6f26c9be6da6f1195a4569f003a010d3f2e0c24d) )
	ROM_LOAD( "vl4c.dat",    0x01000, 0x1000, CRC(4b134ae2) SHA1(86756e1d8de113571857818a98d347789c003339) )
	ROM_LOAD( "vl4a.dat",    0x02000, 0x1000, CRC(b2f1f95a) SHA1(89166cdf4aff5e5a8cc4ea6ba589ce095de82f57) )
	ROM_LOAD( "vl5e.dat",    0x03000, 0x1000, CRC(c8681c55) SHA1(ac63e53a958f63bd0a05f36303c1aa777aee799d) )
	ROM_LOAD( "vl5c.dat",    0x04000, 0x1000, CRC(eb9351e0) SHA1(c66477ca0b3ed95708eb478fb992833beda1a4f8) )
	ROM_LOAD( "vl5a.dat",    0x05000, 0x1000, CRC(534f2fa1) SHA1(c034aa037ef6bc7cd2ed85da7531fd8efb7083e4) )
	ROM_LOAD( "vl6e.dat",    0x06000, 0x1000, CRC(befac592) SHA1(548cb1f0bc178eeada144c443545f7545c90b6a6) )
	ROM_LOAD( "vl6c.dat",    0x07000, 0x1000, CRC(a73d7f13) SHA1(833ff14c33635b61e1bd45b2878a4f6c9e18bf82) )
	ROM_LOAD( "vl6a.dat",    0x08000, 0x1000, CRC(5730a43d) SHA1(8acadf105dc373bf2b3087ccc1667b872452c913) )
	ROM_LOAD( "vl7a.dat",    0x0d000, 0x1000, CRC(fb2aec2c) SHA1(73dc6a6dfe9ba51e3612b6d912bd7af1d5782296) )
	ROM_LOAD( "vl7c.dat",    0x0e000, 0x1000, CRC(9a496519) SHA1(ae98dadcb63a33c796a3e3083d4b5bc957873cbc) )
	ROM_LOAD( "vl7e.dat",    0x0f000, 0x1000, CRC(032cab4b) SHA1(87bdd0fd58b12e39efaadcf6e82744886a9292e9) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "vl2532.snd",   0xf000, 0x1000, CRC(214b8a04) SHA1(45f06b44a605cca6b293b20cfea4763b469254b8) )
ROM_END


ROM_START( mysticm )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "mm02_2.a09", 0x0e000, 0x1000, CRC(3a776ea8) SHA1(1fef5f5cef5e10606c97ac9c365f000a88d51314) ) // IC9
	ROM_LOAD( "mm03_2.a10", 0x0f000, 0x1000, CRC(6e247c75) SHA1(4daf5206d29b887cd1a78528fac4b0cd8ec7f39b) ) // IC10

	ROM_LOAD( "mm11_1.a18", 0x10000, 0x2000, CRC(f537968e) SHA1(2660a480d0bba5fe25885453115ef1015f8bdea9) ) // IC18
	ROM_LOAD( "mm09_1.a16", 0x12000, 0x2000, CRC(3bd12f6c) SHA1(7925a92c486c994e8f34c8ed52bf81a34cf44f68) ) // IC16
	ROM_LOAD( "mm07_1.a14", 0x14000, 0x2000, CRC(ea2a2a68) SHA1(71855c874cd5032f47fafc67e2d1667f956cd9b5) ) // IC14
	ROM_LOAD( "mm05_1.a12", 0x16000, 0x2000, CRC(b514eef3) SHA1(0f9309768c416dd98e9c02121cc750993a2923ea) ) // IC12

	ROM_LOAD( "mm18_1.a26", 0x18000, 0x2000, CRC(9b391a81) SHA1(b3f34e5d468fe4a4de2d4e771e2fa08de6596f26) ) // IC26
	ROM_LOAD( "mm16_1.a24", 0x1a000, 0x2000, CRC(399e175d) SHA1(e17301e4159e5a6d83c3ca62c93eb70f34b948df) ) // IC24
	ROM_LOAD( "mm14_1.a22", 0x1c000, 0x2000, CRC(191153b1) SHA1(fcd8aa6ad6506ba51a01f777f6a3b94e9c051b1c) ) // IC22

	ROM_LOAD( "mm10_1.a17", 0x20000, 0x2000, CRC(d6a37509) SHA1(4b1f52954ca208ccc040c017873777fbf7fbd1f2) ) // IC17
	ROM_LOAD( "mm08_1.a15", 0x22000, 0x2000, CRC(6f1a64f2) SHA1(4183b658b257d7fe35e1d7271f76d3358df5a7a2) ) // IC15
	ROM_LOAD( "mm06_1.a13", 0x24000, 0x2000, CRC(2e6795d4) SHA1(8b074f6a7a4b5a9705de498684180815581faea2) ) // IC13
	ROM_LOAD( "mm04_1.a11", 0x26000, 0x2000, CRC(c222fb64) SHA1(b4c51d2b1664ef3267df1dee9e4888acf847c286) ) // IC11

	ROM_LOAD( "mm17_1.a25", 0x28000, 0x2000, CRC(d36f0a96) SHA1(9830955ca7e46b5b0dba98b4d2ea325bbbebe3c7) ) // IC25
	ROM_LOAD( "mm15_1.a23", 0x2a000, 0x2000, CRC(cd5d99da) SHA1(41a37903503c14fb9c801c51afa2f97c83b79f8b) ) // IC23
	ROM_LOAD( "mm13_1.a21", 0x2c000, 0x2000, CRC(ef4b79db) SHA1(346057cb8c4593df44fb36771553e60610fe1a0c) ) // IC21
	ROM_LOAD( "mm12_1.a19", 0x2e000, 0x2000, CRC(a1f04bf0) SHA1(389bdb7c9e395af9275abfb20c3ab51bc12dc4db) ) // IC19

	// sound CPU
	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "mm01_1.a08", 0x0e000, 0x2000, CRC(65339512) SHA1(144625d2905c953383bcc90cd2435d332394883f) ) // IC8

	ROM_REGION( 0x6000, "tiles", 0 )
	ROM_LOAD( "mm20_1.b57", 0x00000, 0x2000, CRC(5c0f4f46) SHA1(7dedbbeda2f34a2eac9fb14277874d9d66f468c7) ) // IC57
	ROM_LOAD( "mm21_1.b58", 0x02000, 0x2000, CRC(cb90b3c5) SHA1(f28cca2c3ff23d6c9e2952a1b08ab2875655ec70) ) // IC58
	ROM_LOAD( "mm19_1.b41", 0x04000, 0x2000, CRC(e274df86) SHA1(9876a487c5efa350ced31acbc39df22c8d414677) ) // IC41

	ROM_REGION( 0x200, "proms", 0 ) // not hooked up
	ROM_LOAD( "ic14.bpr",   0x00000,  0x020, CRC(27a6d555) SHA1(988d55092d7d0243a867986873dfd12be67280c7) )
	ROM_LOAD( "ic47.bpr",   0x00000,  0x100, CRC(efb03024) SHA1(4c3e3de374f7959a03dcfcb8a29a372685f3b273) )
	ROM_LOAD( "ic60.bpr",   0x00000,  0x200, CRC(0ea3f7fb) SHA1(a8a2a7fbc1a3527a8e2cda71d737afaa717902f1) )
ROM_END

ROM_START( mysticmp )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "cpu_2732_ic9_rom2_proto6.d4",    0x0e000, 0x1000, CRC(3e4f53dd) SHA1(ebe36af7367b7f1037f5d7d55817e5580db6f10f) ) // ic9, different
	ROM_LOAD( "cpu_2732_ic10_rom3_proto6.f4",   0x0f000, 0x1000, CRC(6a25ee4b) SHA1(0668a0f3d6ddcf413d8b1f4f8f5b9a2dc9c4edc1) ) // ic10, different

	ROM_LOAD( "cpu_2764_ic18_rom11_proto5.j8",  0x10000, 0x2000, CRC(f537968e) SHA1(2660a480d0bba5fe25885453115ef1015f8bdea9) ) // ic18
	ROM_LOAD( "cpu_2764_ic16_rom9_proto5.h8",   0x12000, 0x2000, CRC(3bd12f6c) SHA1(7925a92c486c994e8f34c8ed52bf81a34cf44f68) ) // ic16
	ROM_LOAD( "cpu_2764_ic14_rom7_proto5.j6",   0x14000, 0x2000, CRC(ea2a2a68) SHA1(71855c874cd5032f47fafc67e2d1667f956cd9b5) ) // ic14
	ROM_LOAD( "cpu_2764_ic12_rom5_proto5.h6",   0x16000, 0x2000, CRC(b514eef3) SHA1(0f9309768c416dd98e9c02121cc750993a2923ea) ) // ic12

	ROM_LOAD( "cpu_2764_ic26_rom18_proto5.j10", 0x18000, 0x2000, CRC(9b391a81) SHA1(b3f34e5d468fe4a4de2d4e771e2fa08de6596f26) ) // ic26
	ROM_LOAD( "cpu_2764_ic24_rom16_proto5.h10", 0x1a000, 0x2000, CRC(399e175d) SHA1(e17301e4159e5a6d83c3ca62c93eb70f34b948df) ) // ic24
	ROM_LOAD( "cpu_2764_ic22_rom14_proto5.j9",  0x1c000, 0x2000, CRC(191153b1) SHA1(fcd8aa6ad6506ba51a01f777f6a3b94e9c051b1c) ) // ic22

	ROM_LOAD( "cpu_2764_ic17_rom10_proto4.i8",  0x20000, 0x2000, CRC(d6a37509) SHA1(4b1f52954ca208ccc040c017873777fbf7fbd1f2) ) // ic17
	ROM_LOAD( "cpu_2764_ic15_rom8_proto4.g8",   0x22000, 0x2000, CRC(6f1a64f2) SHA1(4183b658b257d7fe35e1d7271f76d3358df5a7a2) ) // ic15
	ROM_LOAD( "cpu_2764_ic13_rom6_proto4.i6",   0x24000, 0x2000, CRC(2e6795d4) SHA1(8b074f6a7a4b5a9705de498684180815581faea2) ) // ic13
	ROM_LOAD( "cpu_2764_ic11_rom4_proto4.g6",   0x26000, 0x2000, CRC(c222fb64) SHA1(b4c51d2b1664ef3267df1dee9e4888acf847c286) ) // ic11

	ROM_LOAD( "cpu_2764_ic25_rom17_proto6.i10", 0x28000, 0x2000, CRC(7acc9995) SHA1(a996cbd65cf7efd1cdf9b5750b5c743c5edda4dd) ) // ic25, different
	ROM_LOAD( "cpu_2764_ic23_rom15_proto6.g10", 0x2a000, 0x2000, CRC(c32d1ce5) SHA1(0df3eafbb558699382eb729a3059e99305e2e8c8) ) // ic23, different
	ROM_LOAD( "cpu_2764_ic21_rom13_proto6.i9",  0x2c000, 0x2000, CRC(e387a785) SHA1(de98d503f4d2c947c701ff96628114b34da45f93) ) // ic21, different
	ROM_LOAD( "cpu_2764_ic19_rom12_proto6.g9",  0x2e000, 0x2000, CRC(a1f04bf0) SHA1(389bdb7c9e395af9275abfb20c3ab51bc12dc4db) ) // ic19

	// sound CPU
	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "cpu_2764_ic8_rom1_proto4.f0", 0x0e000, 0x2000, CRC(65339512) SHA1(144625d2905c953383bcc90cd2435d332394883f) )    // ic8

	ROM_REGION( 0x6000, "tiles", 0 )
	ROM_LOAD( "ram_2764_ic57_rom20_rev1.f9",  0x00000, 0x2000, CRC(5c0f4f46) SHA1(7dedbbeda2f34a2eac9fb14277874d9d66f468c7) )   // ic57
	ROM_LOAD( "ram_2764_ic58_rom21_rev1.f10", 0x02000, 0x2000, CRC(cb90b3c5) SHA1(f28cca2c3ff23d6c9e2952a1b08ab2875655ec70) )   // ic58
	ROM_LOAD( "ram_2764_ic41_rom19_rev1.d10", 0x04000, 0x2000, CRC(e274df86) SHA1(9876a487c5efa350ced31acbc39df22c8d414677) )   // ic41
ROM_END


ROM_START( tshoot )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "rom18.ic55", 0x0d000, 0x1000, CRC(effc33f1) SHA1(cd1b16b4a4a46ce9d550d10b465b8cf1ab3c5273) )  // IC55
	ROM_LOAD( "rom2.ic9",   0x0e000, 0x1000, CRC(fd982687) SHA1(70be1ea57ea0a1e75b1bd988492a9c0244e8b91f) )  // IC9
	ROM_LOAD( "rom3.ic10",  0x0f000, 0x1000, CRC(9617054d) SHA1(8795b97a6391aa3804f68dc2d2b33866dc17f34c) )  // IC10

	ROM_LOAD( "rom11.ic18", 0x10000, 0x2000, CRC(60d5fab8) SHA1(fe75e46dedb7ca153470d6a39cea0a721e5b7b39) )  // IC18
	ROM_LOAD( "rom9.ic16",  0x12000, 0x2000, CRC(a4dd4a0e) SHA1(bb2f38c5ef2f3398b6ba605ffa0c30c89387bf14) )  // IC16
	ROM_LOAD( "rom7.ic14",  0x14000, 0x2000, CRC(f25505e6) SHA1(d075ff89b6379ad7a47d9723ed1c21468b9d1dae) )  // IC14
	ROM_LOAD( "rom5.ic12",  0x16000, 0x2000, CRC(94a7c0ed) SHA1(11f46e1ca7d79b4244ea0f60e0fba44186f1ebde) )  // IC12

	ROM_LOAD( "rom17.ic26", 0x18000, 0x2000, CRC(b02d1ccd) SHA1(b08b6d9affb6f3e50a11fd9397fe4255927de3b6) )  // IC26
	ROM_LOAD( "rom15.ic24", 0x1a000, 0x2000, CRC(11709935) SHA1(ae25bbadbbcab9f3cba2bb4bb92d5217705b38e3) )  // IC24

	ROM_LOAD( "rom10.ic17", 0x20000, 0x2000, CRC(0f32bad8) SHA1(7a2f559697d252ceec3a2f55fe51bc755e4bb65a) )  // IC17
	ROM_LOAD( "rom8.ic15",  0x22000, 0x2000, CRC(e9b6cbf7) SHA1(6cd6b1e1c5e8e253e779afff8ad1ff90d6116fc9) )  // IC15
	ROM_LOAD( "rom6.ic13",  0x24000, 0x2000, CRC(a49f617f) SHA1(759d25e33a09204664880329b86724805a1fe0e8) )  // IC13
	ROM_LOAD( "rom4.ic11",  0x26000, 0x2000, CRC(b026dc00) SHA1(8a068997aa19e152d64db47528893046d338389c) )  // IC11

	ROM_LOAD( "rom16.ic25", 0x28000, 0x2000, CRC(69ce38f8) SHA1(a2cd678e71bfa5e6a3594d8699660c7fa8b52001) )  // IC25
	ROM_LOAD( "rom14.ic23", 0x2a000, 0x2000, CRC(769a4ae5) SHA1(1cdfae2d889848d69f68f990714d027cfbca1853) )  // IC23
	ROM_LOAD( "rom13.ic21", 0x2c000, 0x2000, CRC(ec016c9b) SHA1(f2e40abd14b8b4944b792dd453ebe92eb64355ae) )  // IC21
	ROM_LOAD( "rom12.ic19", 0x2e000, 0x2000, CRC(98ae7afa) SHA1(6a904408419f576352bd2f895727fd17c0541ff8) )  // IC19

	// sound CPU
	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "rom1.ic8",   0xe000, 0x2000, CRC(011a94a7) SHA1(9f54a742a87ba56b9517e33e556f57dce6eb2eab) )    // IC8

	ROM_REGION( 0x6000, "tiles", 0 )
	ROM_LOAD( "rom20.ic57", 0x00000, 0x2000, CRC(c6e1d253) SHA1(c408a29f75ba2958e229996f903400b3d95e3bd3) )  // IC57
	ROM_LOAD( "rom21.ic58", 0x02000, 0x2000, CRC(9874e90f) SHA1(85282823cc862341adf9642d2d5d05972da6dff0) )  // IC58
	ROM_LOAD( "rom19.ic41", 0x04000, 0x2000, CRC(b9ce4d2a) SHA1(af5332f340d3c3ae02e77923d6e8f0dd92547728) )  // IC41

	ROM_REGION( 0x200, "proms", 0 ) // not hooked up
	ROM_LOAD( "82s123.ic14a", 0x00000, 0x020, CRC(27a6d555) SHA1(988d55092d7d0243a867986873dfd12be67280c7) )
	ROM_LOAD( "82s129.ic47",  0x00000, 0x100, CRC(efb03024) SHA1(4c3e3de374f7959a03dcfcb8a29a372685f3b273) )
	ROM_LOAD( "7649.ic60",    0x00000, 0x200, CRC(0ea3f7fb) SHA1(a8a2a7fbc1a3527a8e2cda71d737afaa717902f1) )
ROM_END


ROM_START( inferno )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "ic9.inf",  0x0e000, 0x1000, CRC(1a013185) SHA1(9079c082ec043714f9d8ea92bc81d0b93d2ce715) )   // IC9
	ROM_LOAD( "ic10.inf", 0x0f000, 0x1000, CRC(dbf64a36) SHA1(54326bc527797f0a3a55764073eb40030aec1aae) )   // IC10

	ROM_LOAD( "ic18.inf", 0x10000, 0x2000, CRC(95bcf7b1) SHA1(66687a3962109a25e26ae00bddd33ed973981b91) )   // IC18
	ROM_LOAD( "ic16.inf", 0x12000, 0x2000, CRC(8bc4f935) SHA1(12da6faa71e5984047fa14f32af5bb865f228cb2) )   // IC16
	ROM_LOAD( "ic14.inf", 0x14000, 0x2000, CRC(a70508a7) SHA1(930bb9af3b6ba9fdf3e7c32f6b5ffae9acd6cee3) )   // IC14
	ROM_LOAD( "ic12.inf", 0x16000, 0x2000, CRC(7ffb87f9) SHA1(469f5ae39ad8531c4c11e9d10ab57686e7f54aef) )   // IC12

	ROM_LOAD( "ic17.inf", 0x20000, 0x2000, CRC(b4684139) SHA1(c1d6ecd3dc8191250ef70e6972dad234c0d8f739) )   // IC17
	ROM_LOAD( "ic15.inf", 0x22000, 0x2000, CRC(128a6ad6) SHA1(357438e50663d6cb96dabfa5110c17836584e15f) )   // IC15
	ROM_LOAD( "ic13.inf", 0x24000, 0x2000, CRC(83a9e4d6) SHA1(4937e4d1c516da837213e40a1da862578c8dd272) )   // IC13
	ROM_LOAD( "ic11.inf", 0x26000, 0x2000, CRC(c2e9c909) SHA1(21f0b9bf6ef3a9466ea9afde1c7efde9ed04ce5b) )   // IC11

	ROM_LOAD( "ic25.inf", 0x28000, 0x2000, CRC(103a5951) SHA1(57c8caa1e9d5e245052822d08add9343fd622e04) )   // IC25
	ROM_LOAD( "ic23.inf", 0x2a000, 0x2000, CRC(c04749a0) SHA1(b203e8d1df556e10b4ecad4733448f889c63e261) )   // IC23
	ROM_LOAD( "ic21.inf", 0x2c000, 0x2000, CRC(c405f853) SHA1(6bd74d065a6043849e083c2822925b82c6fedb00) )   // IC21
	ROM_LOAD( "ic19.inf", 0x2e000, 0x2000, CRC(ade7645a) SHA1(bfaab1840e3171df895a2333a30b9dac214b3351) )   // IC19

	// sound CPU
	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ic8.inf", 0x0e000, 0x2000, CRC(4e3123b8) SHA1(f453feed3ae3b6430db49eb4325f62eecfee9f5e) )    // IC8

	ROM_REGION( 0x6000, "tiles", 0 )
	ROM_LOAD( "ic57.inf", 0x00000, 0x2000, CRC(65a4ef79) SHA1(270c58901e83665bc388cd9cb92022c55e8eae50) )   // IC57
	ROM_LOAD( "ic58.inf", 0x02000, 0x2000, CRC(4bb1c2a0) SHA1(9e8d214b8d1dbe4c2369e4047e165c9e692098a5) )   // IC58
	ROM_LOAD( "ic41.inf", 0x04000, 0x2000, CRC(f3f7238f) SHA1(3810f1afd318ec37271c099c989b142b85d8da51) )   // IC41

	ROM_REGION( 0x200, "proms", 0 ) // not hooked up
	ROM_LOAD( "prom1", 0x00000, 0x020, CRC(85057e40) SHA1(c34cdd24d77031450493da50d4ad02813f9b30a8) )
	ROM_LOAD( "prom2", 0x00000, 0x100, CRC(efb03024) SHA1(4c3e3de374f7959a03dcfcb8a29a372685f3b273) )
	ROM_LOAD( "prom3", 0x00000, 0x200, CRC(0ea3f7fb) SHA1(a8a2a7fbc1a3527a8e2cda71d737afaa717902f1) )
ROM_END


ROM_START( joust2 )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "cpu_2732_ic55_rom2_rev1.4c",   0x0d000, 0x1000, CRC(08b0d5bd) SHA1(b58da478aef36ae20fcfee48151d5d556e16b7b9) )
	ROM_LOAD( "cpu_2732_ic9_rom3_rev2.4d",    0x0e000, 0x1000, CRC(951175ce) SHA1(ac70df125bb438f9fccc082276df4a76ff693e16) )
	ROM_LOAD( "cpu_2732_ic10_rom4_rev2.4f",   0x0f000, 0x1000, CRC(ba6e0f6c) SHA1(431cbf38e919011d030f41008e1ad45e7e0ec38b) )

	ROM_LOAD( "cpu_2732_ic18_rom11_rev1.8j",  0x10000, 0x2000, CRC(9dc986f9) SHA1(5ce479936536ef713cdfc8fc8190d338c46d171e) )
	ROM_LOAD( "cpu_2732_ic16_rom9_rev2.8h",   0x12000, 0x2000, CRC(56e2b550) SHA1(01211d389ca384987d56c26596aa8c1adffdf8dd) )
	ROM_LOAD( "cpu_2732_ic14_rom7_rev2.6j",   0x14000, 0x2000, CRC(f3bce576) SHA1(30ee1b212879b3b55b47c9064f123fb77c8f3089) )
	ROM_LOAD( "cpu_2732_ic12_rom5_rev2.6h",   0x16000, 0x2000, CRC(5f8b4919) SHA1(1215a314c07ef4f244e862743035626cac1d9538) )

	ROM_LOAD( "cpu_2732_ic26_rom19_rev1.10j", 0x18000, 0x2000, CRC(4ef5e805) SHA1(98b93388ab4a4fa6eeceee3386fa46f5a307b8cb) )
	ROM_LOAD( "cpu_2732_ic24_rom17_rev1.10h", 0x1a000, 0x2000, CRC(4861f063) SHA1(6db00cce230bf4bdfdfbfe59e0dc2d916b84d0dc) )
	ROM_LOAD( "cpu_2732_ic22_rom15_rev1.9j",  0x1c000, 0x2000, CRC(421aafa8) SHA1(06187ba8fef3e89eb399d7040015212bd5f86853) )
	ROM_LOAD( "cpu_2732_ic20_rom13_rev1.9h",  0x1e000, 0x2000, CRC(3432ff55) SHA1(aec0f83b92369de8a830ec298ac490a51bc29f26) )

	ROM_LOAD( "cpu_2732_ic17_rom10_rev1.8i",  0x20000, 0x2000, CRC(3e01b597) SHA1(17d09482636d6cda2f3266152396f0461121e748) )
	ROM_LOAD( "cpu_2732_ic15_rom8_rev1.8g",   0x22000, 0x2000, CRC(ff26fb29) SHA1(5ad498db71c384c1928ec965ba3cad48af428f19) )
	ROM_LOAD( "cpu_2732_ic13_rom6_rev2.6i",   0x24000, 0x2000, CRC(5f107db5) SHA1(c413a2e58853ccda602515b9668a6a620294ba49) )

	ROM_LOAD( "cpu_2732_ic25_rom18_rev1.10i", 0x28000, 0x2000, CRC(47580af5) SHA1(d2728f32f02b549c7e9691c668f0097e327a1d2d) )
	ROM_LOAD( "cpu_2732_ic23_rom16_rev1.10g", 0x2a000, 0x2000, CRC(869b5942) SHA1(a3f4bab4c0db71589e9be2bbf1f94052ef2f56da) )
	ROM_LOAD( "cpu_2732_ic21_rom14_rev1.9i",  0x2c000, 0x2000, CRC(0bbd867c) SHA1(f2db9fc57b6afb762715617345e8c3dcb89b6cc2) )
	ROM_LOAD( "cpu_2732_ic19_rom12_rev1.9g",  0x2e000, 0x2000, CRC(b9221ed1) SHA1(428ea8f3e2fa58d875f581f5de6e0d05ed855a45) )

	// sound CPU
	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "cpu_2764_ic8_rom1_rev1.0f", 0x0e000, 0x2000, CRC(84517c3c) SHA1(de0b6473953783c091ddcc7aaa89fc1ec3b9d378) )

	// sound board
	ROM_REGION( 0x80000, "bg:cpu", 0 )
	ROM_LOAD( "snd_27256_rom23_rev1.u4",  0x00000, 0x8000, CRC(3af6b47d) SHA1(aff19d65a4d9c249dec6a9e04a4066fada0f8fa1) )
	ROM_RELOAD(             0x08000, 0x8000 )
	ROM_RELOAD(             0x10000, 0x8000 )
	ROM_RELOAD(             0x18000, 0x8000 )
	ROM_LOAD( "snd_27256_rom24_rev1.u19", 0x20000, 0x8000, CRC(e7f9ed2e) SHA1(6b9ef5189650f0b6b2866da7f532cdf851f02ead) )
	ROM_RELOAD(             0x28000, 0x8000 )
	ROM_RELOAD(             0x30000, 0x8000 )
	ROM_RELOAD(             0x38000, 0x8000 )
	ROM_LOAD( "snd_27256_rom25_rev1.u20", 0x40000, 0x8000, CRC(c85b29f7) SHA1(b37e1890bd0dfa0c7db19fc878450718b60c1ca0) )
	ROM_RELOAD(             0x48000, 0x8000 )
	ROM_RELOAD(             0x50000, 0x8000 )
	ROM_RELOAD(             0x58000, 0x8000 )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "vid_27128_ic57_rom20_rev1.8f", 0x00000, 0x4000, CRC(572c6b01) SHA1(651df3223c1dc42543f57a7204ae492eb15a4999) )
	ROM_LOAD( "vid_27128_ic58_rom21_rev1.9f", 0x04000, 0x4000, CRC(aa94bf05) SHA1(3412dd181e2c12dc2dd1caabfe7e737005b0ccd7) )
	ROM_LOAD( "vid_27128_ic41_rom22_rev1.9d", 0x08000, 0x4000, CRC(c41e3daa) SHA1(fafe76bebd6eaf2cd124c1030e3a58eb5a6cddc6) )

	ROM_REGION( 0x200, "proms", 0 ) // not hooked up
	ROM_LOAD( "vid_82s123_ic14_a-5282-10295.2b",   0x00000, 0x020, CRC(85057e40) SHA1(c34cdd24d77031450493da50d4ad02813f9b30a8) )
	ROM_LOAD( "vid_82s129_ic47_a-5282-10294.15d",  0x00000, 0x100, CRC(efb03024) SHA1(4c3e3de374f7959a03dcfcb8a29a372685f3b273) )
	ROM_LOAD( "vid_82s147a_ic60_a-5282-10292.12f", 0x00000, 0x200, CRC(0ea3f7fb) SHA1(a8a2a7fbc1a3527a8e2cda71d737afaa717902f1) )
ROM_END


ROM_START( joust2r1 )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "cpu_2732_ic55_rom2_rev1.4c",   0x0d000, 0x1000, CRC(08b0d5bd) SHA1(b58da478aef36ae20fcfee48151d5d556e16b7b9) )
	ROM_LOAD( "cpu_2732_ic9_rom3_rev1.4d",    0x0e000, 0x1000, CRC(6f319644) SHA1(1a9bc121b830277c42bac816ec26758c915b49dd) )
	ROM_LOAD( "cpu_2732_ic10_rom4_rev1.4f",   0x0f000, 0x1000, CRC(027b9f0c) SHA1(8c4631fc42ed0b87b2bb0326c48b92d73cdd2f42) )

	ROM_LOAD( "cpu_2732_ic18_rom11_rev1.8j",  0x10000, 0x2000, CRC(9dc986f9) SHA1(5ce479936536ef713cdfc8fc8190d338c46d171e) )
	ROM_LOAD( "cpu_2732_ic16_rom9_rev1.8h",   0x12000, 0x2000, CRC(0c77e22f) SHA1(db024b8d2fe79f9230c07398bcade1c75e772541) )
	ROM_LOAD( "cpu_2732_ic14_rom7_rev1.6j",   0x14000, 0x2000, CRC(fb9455ca) SHA1(8963832f2ab6f5b2f31611e768cab636672f398c) )
	ROM_LOAD( "cpu_2732_ic12_rom5_rev1.6h",   0x16000, 0x2000, CRC(31248a0d) SHA1(a27a252b353f99748aacfeb29c8bbbd8b3a833f2) )

	ROM_LOAD( "cpu_2732_ic26_rom19_rev1.10j", 0x18000, 0x2000, CRC(4ef5e805) SHA1(98b93388ab4a4fa6eeceee3386fa46f5a307b8cb) )
	ROM_LOAD( "cpu_2732_ic24_rom17_rev1.10h", 0x1a000, 0x2000, CRC(4861f063) SHA1(6db00cce230bf4bdfdfbfe59e0dc2d916b84d0dc) )
	ROM_LOAD( "cpu_2732_ic22_rom15_rev1.9j",  0x1c000, 0x2000, CRC(421aafa8) SHA1(06187ba8fef3e89eb399d7040015212bd5f86853) )
	ROM_LOAD( "cpu_2732_ic20_rom13_rev1.9h",  0x1e000, 0x2000, CRC(3432ff55) SHA1(aec0f83b92369de8a830ec298ac490a51bc29f26) )

	ROM_LOAD( "cpu_2732_ic17_rom10_rev1.8i",  0x20000, 0x2000, CRC(3e01b597) SHA1(17d09482636d6cda2f3266152396f0461121e748) )
	ROM_LOAD( "cpu_2732_ic15_rom8_rev1.8g",   0x22000, 0x2000, CRC(ff26fb29) SHA1(5ad498db71c384c1928ec965ba3cad48af428f19) )
	ROM_LOAD( "cpu_2732_ic13_rom6_rev1.6i",   0x24000, 0x2000, CRC(6a8c87d7) SHA1(ba66cd8f23a249470c612890829d40d070bbd1e9) )

	ROM_LOAD( "cpu_2732_ic25_rom18_rev1.10i", 0x28000, 0x2000, CRC(47580af5) SHA1(d2728f32f02b549c7e9691c668f0097e327a1d2d) )
	ROM_LOAD( "cpu_2732_ic23_rom16_rev1.10g", 0x2a000, 0x2000, CRC(869b5942) SHA1(a3f4bab4c0db71589e9be2bbf1f94052ef2f56da) )
	ROM_LOAD( "cpu_2732_ic21_rom14_rev1.9i",  0x2c000, 0x2000, CRC(0bbd867c) SHA1(f2db9fc57b6afb762715617345e8c3dcb89b6cc2) )
	ROM_LOAD( "cpu_2732_ic19_rom12_rev1.9g",  0x2e000, 0x2000, CRC(b9221ed1) SHA1(428ea8f3e2fa58d875f581f5de6e0d05ed855a45) )

	// sound CPU
	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "cpu_2764_ic8_rom1_rev1.0f", 0x0e000, 0x2000, CRC(84517c3c) SHA1(de0b6473953783c091ddcc7aaa89fc1ec3b9d378) )

	// sound board
	ROM_REGION( 0x80000, "bg:cpu", 0 )
	ROM_LOAD( "snd_27256_rom23_rev1.u4",  0x00000, 0x8000, CRC(3af6b47d) SHA1(aff19d65a4d9c249dec6a9e04a4066fada0f8fa1) )
	ROM_RELOAD(             0x08000, 0x8000 )
	ROM_RELOAD(             0x10000, 0x8000 )
	ROM_RELOAD(             0x18000, 0x8000 )
	ROM_LOAD( "snd_27256_rom24_rev1.u19", 0x20000, 0x8000, CRC(e7f9ed2e) SHA1(6b9ef5189650f0b6b2866da7f532cdf851f02ead) )
	ROM_RELOAD(             0x28000, 0x8000 )
	ROM_RELOAD(             0x30000, 0x8000 )
	ROM_RELOAD(             0x38000, 0x8000 )
	ROM_LOAD( "snd_27256_rom25_rev1.u20", 0x40000, 0x8000, CRC(c85b29f7) SHA1(b37e1890bd0dfa0c7db19fc878450718b60c1ca0) )
	ROM_RELOAD(             0x48000, 0x8000 )
	ROM_RELOAD(             0x50000, 0x8000 )
	ROM_RELOAD(             0x58000, 0x8000 )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "vid_27128_ic57_rom20_rev1.8f", 0x00000, 0x4000, CRC(572c6b01) SHA1(651df3223c1dc42543f57a7204ae492eb15a4999) )
	ROM_LOAD( "vid_27128_ic58_rom21_rev1.9f", 0x04000, 0x4000, CRC(aa94bf05) SHA1(3412dd181e2c12dc2dd1caabfe7e737005b0ccd7) )
	ROM_LOAD( "vid_27128_ic41_rom22_rev1.9d", 0x08000, 0x4000, CRC(c41e3daa) SHA1(fafe76bebd6eaf2cd124c1030e3a58eb5a6cddc6) )

	ROM_REGION( 0x200, "proms", 0 ) // not hooked up
	ROM_LOAD( "vid_82s123_ic14_a-5282-10295.2b",   0x00000, 0x020, CRC(85057e40) SHA1(c34cdd24d77031450493da50d4ad02813f9b30a8) )
	ROM_LOAD( "vid_82s129_ic47_a-5282-10294.15d",  0x00000, 0x100, CRC(efb03024) SHA1(4c3e3de374f7959a03dcfcb8a29a372685f3b273) )
	ROM_LOAD( "vid_82s147a_ic60_a-5282-10292.12f", 0x00000, 0x200, CRC(0ea3f7fb) SHA1(a8a2a7fbc1a3527a8e2cda71d737afaa717902f1) )
ROM_END



/*************************************
 *
 *  Defender hardware driver init
 *
 *************************************/

void defender_state::init_defndjeu()
{
	memory_region *rom;

	// apply simple decryption by swapping bits 0 and 7
	rom = memregion("maincpu");
	for (int i = 0; i < rom->bytes(); i++)
		rom->base()[i] = bitswap<8>(rom->base()[i], 0,6,5,4,3,2,1,7);
	rom = memregion("banked");
	for (int i = 0; i < rom->bytes(); i++)
		rom->base()[i] = bitswap<8>(rom->base()[i], 0,6,5,4,3,2,1,7);
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

// Defender hardware games

//    YEAR  NAME        PARENT    MACHINE           INPUT     CLASS            INIT           ROT     COMPANY                                 FULLNAME                                    FLAGS
GAME( 1980, defender,   0,        defender,         defender, defender_state,  empty_init,    ROT0,   "Williams",                             "Defender (Red label)",                     MACHINE_SUPPORTS_SAVE ) // developers left Williams in 1981 and formed Vid Kidz
GAME( 1980, defenderg,  defender, defender,         defender, defender_state,  empty_init,    ROT0,   "Williams",                             "Defender (Green label)",                   MACHINE_SUPPORTS_SAVE )
GAME( 1980, defenderb,  defender, defender,         defender, defender_state,  empty_init,    ROT0,   "Williams",                             "Defender (Blue label)",                    MACHINE_SUPPORTS_SAVE )
GAME( 1980, defenderw,  defender, defender,         defender, defender_state,  empty_init,    ROT0,   "Williams",                             "Defender (White label)",                   MACHINE_SUPPORTS_SAVE )
GAME( 1980, defenderj,  defender, defender,         defender, defender_state,  empty_init,    ROT0,   "Williams (Taito Corporation license)", "T.T Defender",                             MACHINE_SUPPORTS_SAVE )
GAME( 1980, defndjeu,   defender, defender,         defender, defender_state,  init_defndjeu, ROT0,   "bootleg (Jeutel)",                     "Defender (bootleg)",                       MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1980, tornado1,   defender, defender,         defender, defender_state,  init_defndjeu, ROT0,   "bootleg (Jeutel)",                     "Tornado (bootleg of Defender, set 1)",     MACHINE_SUPPORTS_SAVE )
GAME( 1980, tornado2,   defender, defender,         defender, defender_state,  init_defndjeu, ROT0,   "bootleg (Jeutel)",                     "Tornado (bootleg of Defender, set 2)",     MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // bad dump?
GAME( 1980, zero,       defender, defender,         defender, defender_state,  init_defndjeu, ROT0,   "bootleg (Jeutel)",                     "Zero (bootleg of Defender, set 1)",        MACHINE_SUPPORTS_SAVE )
GAME( 1980, zero2,      defender, defender,         defender, defender_state,  init_defndjeu, ROT0,   "bootleg (Amtec)",                      "Zero (bootleg of Defender, set 2)",        MACHINE_SUPPORTS_SAVE )
GAME( 1981, defenderom, defender, defender_6802snd, defender, defender_state,  empty_init,    ROT0,   "bootleg (Operamatic)",                 "Operacion Defender (bootleg of Defender)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, defcmnd,    defender, defender,         defender, defender_state,  empty_init,    ROT0,   "bootleg",                              "Defense Command (bootleg of Defender)",    MACHINE_SUPPORTS_SAVE )
GAME( 1981, defence,    defender, defender,         defender, defender_state,  empty_init,    ROT0,   "bootleg (Outer Limits)",               "Defence Command (bootleg of Defender)",    MACHINE_SUPPORTS_SAVE )
GAME( 198?, defenseb,   defender, defender,         defender, defender_state,  empty_init,    ROT0,   "bootleg",                              "Defense (bootleg of Defender)",            MACHINE_SUPPORTS_SAVE )
GAME( 1981, startrkd,   defender, defender,         defender, defender_state,  empty_init,    ROT0,   "bootleg",                              "Star Trek (bootleg of Defender)",          MACHINE_SUPPORTS_SAVE )
GAME( 1980, attackf,    defender, defender,         defender, defender_state,  empty_init,    ROT0,   "bootleg (Famaresa)",                   "Attack (bootleg of Defender)",             MACHINE_SUPPORTS_SAVE )
GAME( 1981, galwars2,   defender, defender_6802snd, defender, defender_state,  empty_init,    ROT0,   "bootleg (Sonic)",                      "Galaxy Wars II (bootleg of Defender)",     MACHINE_SUPPORTS_SAVE ) // Sega Sonic - Sega S.A., only displays Sonic on title screen

GAME( 1980, mayday,     0,        mayday,           mayday,   mayday_state,    empty_init,    ROT0,   "Hoei",                 "Mayday (set 1)",                  MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION ) // \  original by Hoei, which one of these 3 sets is bootleg/licensed/original is unknown
GAME( 1980, maydaya,    mayday,   mayday,           mayday,   mayday_state,    empty_init,    ROT0,   "Hoei",                 "Mayday (set 2)",                  MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION ) //  > these games have an unemulated protection chip of some sort which is hacked around in /midway/williams_m.cpp "protection_r" function
GAME( 1980, maydayb,    mayday,   mayday,           mayday,   mayday_state,    empty_init,    ROT0,   "Hoei",                 "Mayday (set 3)",                  MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION ) // /
GAME( 1980, batlzone,   mayday,   mayday,           mayday,   mayday_state,    empty_init,    ROT0,   "bootleg (Video Game)", "Battle Zone (bootleg of Mayday)", MACHINE_SUPPORTS_SAVE ) // the bootleg may or may not use the same protection chip, or some hack around it.

GAME( 1981, colony7,    0,        defender,         colony7,  defender_state,  empty_init,    ROT270, "Taito",                "Colony 7 (set 1)",                MACHINE_SUPPORTS_SAVE )
GAME( 1981, colony7a,   colony7,  defender,         colony7,  defender_state,  empty_init,    ROT270, "Taito",                "Colony 7 (set 2)",                MACHINE_SUPPORTS_SAVE )

GAME( 1982, jin,        0,        jin,              jin,      defender_state,  empty_init,    ROT90,  "Falcon",               "Jin",                             MACHINE_SUPPORTS_SAVE )


// Standard Williams hardware
GAME( 1981, stargate,   0,        williams_b0,      stargate, williams_state,  empty_init,    ROT0,   "Williams / Vid Kidz", "Stargate", MACHINE_SUPPORTS_SAVE )

GAME( 1982, conquest,   0,        williams_b1,      conquest, conquest_state,  empty_init,    ROT270, "Williams / Vid Kidz", "Conquest (prototype)", MACHINE_IS_INCOMPLETE | MACHINE_SUPPORTS_SAVE )

GAME( 1982, robotron,   0,        williams_b1,      robotron, williams_state,  empty_init,    ROT0,   "Williams / Vid Kidz",                   "Robotron: 2084 (Solid Blue label)",    MACHINE_SUPPORTS_SAVE )
GAME( 1982, robotronyo, robotron, williams_b1,      robotron, williams_state,  empty_init,    ROT0,   "Williams / Vid Kidz",                   "Robotron: 2084 (Yellow/Orange label)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, robotronun, robotron, williams_b1,      robotron, williams_state,  empty_init,    ROT0,   "Williams / Vid Kidz (Unidesa license)", "Robotron: 2084 (Unidesa license)",     MACHINE_SUPPORTS_SAVE )

// the 3 below are all noteworthy hacks of the Solid Blue set
GAME( 1987, robotron87, robotron, williams_b1,      robotron, williams_state,  empty_init,    ROT0,   "hack", "Robotron: 2084 (1987 'shot-in-the-corner' bugfix)", MACHINE_SUPPORTS_SAVE ) // fixes a reset bug.
GAME( 2012, robotron12, robotron, williams_b1,      robotron, williams_state,  empty_init,    ROT0,   "hack", "Robotron: 2084 (2012 'wave 201 start' hack)",       MACHINE_SUPPORTS_SAVE ) // includes sitc bug fix, used for competitive play.
GAME( 2015, robotrontd, robotron, williams_b1,      robotron, williams_state,  empty_init,    ROT0,   "hack", "Robotron: 2084 (2015 'tie-die V2' hack)",           MACHINE_SUPPORTS_SAVE ) // inc. sitc fix, mods by some of the original developers, see backstory here http://www.robotron2084guidebook.com/gameplay/raceto100million/robo2k14_tie-die-romset/  (I guess there's a tie-die V1 before it was released to the public?)

GAME( 1982, joust,      0,        joust,            joust,    williams_state,  empty_init,    ROT0,   "Williams", "Joust (Green label)",  MACHINE_SUPPORTS_SAVE )
GAME( 1982, joustr,     joust,    joust,            joust,    williams_state,  empty_init,    ROT0,   "Williams", "Joust (Red label)",    MACHINE_SUPPORTS_SAVE )
GAME( 1982, jousty,     joust,    joust,            joust,    williams_state,  empty_init,    ROT0,   "Williams", "Joust (Yellow label)", MACHINE_SUPPORTS_SAVE )

GAME( 1982, bubbles,    0,        bubbles,          bubbles,  williams_state,  empty_init,    ROT0,   "Williams", "Bubbles",                   MACHINE_SUPPORTS_SAVE )
GAME( 1982, bubblesr,   bubbles,  bubbles,          bubbles,  williams_state,  empty_init,    ROT0,   "Williams", "Bubbles (Solid Red label)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, bubblesp,   bubbles,  bubbles,          bubbles,  williams_state,  empty_init,    ROT0,   "Williams", "Bubbles (prototype)",       MACHINE_SUPPORTS_SAVE )

GAME( 1982, splat,      0,        splat,            splat,    williams_state,  empty_init,    ROT0,   "Williams", "Splat!", MACHINE_SUPPORTS_SAVE )

GAME( 1982, sinistar,   0,        sinistar_upright, sinistar, williams_state,  empty_init,    ROT270, "Williams", "Sinistar (revision 3, upright)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, sinistarc,  sinistar, sinistar_cockpit, sinistar, williams_state,  empty_init,    ROT270, "Williams", "Sinistar (revision 3, cockpit)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, sinistar2,  sinistar, sinistar_upright, sinistar, williams_state,  empty_init,    ROT270, "Williams", "Sinistar (revision 2, upright)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, sinistarc2, sinistar, sinistar_cockpit, sinistar, williams_state,  empty_init,    ROT270, "Williams", "Sinistar (revision 2, cockpit)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, sinistarp,  sinistar, sinistar_upright, sinistar, williams_state,  empty_init,    ROT270, "Williams", "Sinistar (AMOA-82 prototype)",   MACHINE_SUPPORTS_SAVE )

GAME( 1983, playball,   0,        playball,         playball, williams_state,  empty_init,    ROT270, "Williams", "PlayBall! (prototype)", MACHINE_SUPPORTS_SAVE )

GAME( 1983, blaster,    0,        blaster,          blaster,  blaster_state,   empty_init,    ROT0,   "Williams / Vid Kidz", "Blaster",                  MACHINE_SUPPORTS_SAVE ) // 20 levels - stereo sound
GAME( 1983, blastero,   blaster,  blaster,          blaster,  blaster_state,   empty_init,    ROT0,   "Williams / Vid Kidz", "Blaster (location test)",  MACHINE_SUPPORTS_SAVE ) // 30 levels - stereo sound
GAME( 1983, blasterkit, blaster,  blastkit,         blastkit, blaster_state,   empty_init,    ROT0,   "Williams / Vid Kidz", "Blaster (conversion kit)", MACHINE_SUPPORTS_SAVE ) // 20 levels - mono sound

GAME( 1985, spdball,    0,        spdball,          spdball,  williams_state,  empty_init,    ROT0,   "Williams", "Speed Ball - Contest at Neonworld (prototype)", MACHINE_SUPPORTS_SAVE )

GAME( 1985, alienar,    0,        alienar,          alienar,  williams_state,  empty_init,    ROT0,   "Duncan Brown", "Alien Arena",                    MACHINE_SUPPORTS_SAVE )
GAME( 1985, alienaru,   alienar,  alienar,          alienar,  williams_state,  empty_init,    ROT0,   "Duncan Brown", "Alien Arena (Stargate upgrade)", MACHINE_SUPPORTS_SAVE )

GAME( 1987, lottofun,   0,        lottofun,         lottofun, williams_state,  empty_init,    ROT0,   "HAR Management", "Lotto Fun", MACHINE_SUPPORTS_SAVE )


// 2nd Generation Williams hardware with tilemaps
GAME( 1983, mysticm,    0,        mysticm,          mysticm, mysticm_state,    empty_init,    ROT0,   "Williams", "Mystic Marathon",             MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE)
GAME( 1983, mysticmp,   mysticm,  mysticm,          mysticm, mysticm_state,    empty_init,    ROT0,   "Williams", "Mystic Marathon (prototype)", MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE ) // newest ROMs are 'proto 6' ?

GAME( 1984, tshoot,     0,        tshoot,           tshoot,  tshoot_state,     empty_init,    ROT0,   "Williams", "Turkey Shoot (prototype)", MACHINE_SUPPORTS_SAVE )

GAME( 1984, inferno,    0,        inferno,          inferno, williams2_state,  empty_init,    ROT0,   "Williams", "Inferno (Williams)", MACHINE_SUPPORTS_SAVE )

GAME( 1986, joust2,     0,        joust2,           joust2,  joust2_state,     empty_init,    ROT270, "Williams", "Joust 2 - Survival of the Fittest (revision 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, joust2r1,   joust2,   joust2,           joust2,  joust2_state,     empty_init,    ROT270, "Williams", "Joust 2 - Survival of the Fittest (revision 1)", MACHINE_SUPPORTS_SAVE )
