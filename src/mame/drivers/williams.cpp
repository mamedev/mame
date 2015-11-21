// license:???
// copyright-holders:Michael Soderstrom, Marc LaFontaine, Aaron Giles
/***************************************************************************

    Williams 6809 system

    driver by Michael Soderstrom, Marc LaFontaine, Aaron Giles

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

    CC00-CFFF 1K X 4 CMOS ram battery backed up (8 bits on Sinistar)

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
    Joust 2 has an additional music/speech board that has a
    68B09E CPU, 68B21 PIA, Harris 55564-5 CVSD, and a YM2151.

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

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "cpu/m6800/m6800.h"
#include "sound/dac.h"
#include "sound/hc55516.h"
#include "machine/ticket.h"
#include "includes/williams.h"
#include "machine/nvram.h"


#define MASTER_CLOCK        (XTAL_12MHz)
#define SOUND_CLOCK         (XTAL_3_579545MHz)



/*************************************
 *
 *  Defender memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( defender_map, AS_PROGRAM, 8, williams_state )
	AM_RANGE(0x0000, 0xbfff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0xc000, 0xcfff) AM_DEVICE("bankc000", address_map_bank_device, amap8)
	AM_RANGE(0xd000, 0xdfff) AM_WRITE(defender_bank_select_w)
	AM_RANGE(0xd000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( defender_bankc000_map, AS_PROGRAM, 8, williams_state )
	AM_RANGE(0x0000, 0x000f) AM_MIRROR(0x03e0) AM_WRITEONLY AM_SHARE("paletteram")
	AM_RANGE(0x03ff, 0x03ff) AM_WRITE(williams_watchdog_reset_w)
	AM_RANGE(0x0010, 0x001f) AM_MIRROR(0x03e0) AM_WRITE(defender_video_control_w)
	AM_RANGE(0x0400, 0x04ff) AM_MIRROR(0x0300) AM_RAM_WRITE(williams_cmos_w) AM_SHARE("nvram")
	AM_RANGE(0x0800, 0x0bff) AM_READ(williams_video_counter_r)
	AM_RANGE(0x0c00, 0x0c03) AM_MIRROR(0x03e0) AM_DEVREADWRITE("pia_1", pia6821_device, read, write)
	AM_RANGE(0x0c04, 0x0c07) AM_MIRROR(0x03e0) AM_DEVREADWRITE("pia_0", pia6821_device, read, write)
	AM_RANGE(0x1000, 0x9fff) AM_ROM AM_REGION("maincpu", 0x10000)
	AM_RANGE(0xa000, 0xffff) AM_NOP
ADDRESS_MAP_END



/*************************************
 *
 *  General Williams memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( williams_map, AS_PROGRAM, 8, williams_state )
	AM_RANGE(0x0000, 0x8fff) AM_READ_BANK("bank1") AM_WRITEONLY AM_SHARE("videoram")
	AM_RANGE(0x9000, 0xbfff) AM_RAM
	AM_RANGE(0xc000, 0xc00f) AM_MIRROR(0x03f0) AM_WRITEONLY AM_SHARE("paletteram")
	AM_RANGE(0xc804, 0xc807) AM_MIRROR(0x00f0) AM_DEVREADWRITE("pia_0", pia6821_device, read, write)
	AM_RANGE(0xc80c, 0xc80f) AM_MIRROR(0x00f0) AM_DEVREADWRITE("pia_1", pia6821_device, read, write)
	AM_RANGE(0xc900, 0xc9ff) AM_WRITE(williams_vram_select_w)
	AM_RANGE(0xca00, 0xca07) AM_MIRROR(0x00f8) AM_WRITE(williams_blitter_w)
	AM_RANGE(0xcb00, 0xcbff) AM_READ(williams_video_counter_r)
	AM_RANGE(0xcbff, 0xcbff) AM_WRITE(williams_watchdog_reset_w)
	AM_RANGE(0xcc00, 0xcfff) AM_RAM_WRITE(williams_cmos_w) AM_SHARE("nvram")
	AM_RANGE(0xd000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Sinistar memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( sinistar_map, AS_PROGRAM, 8, williams_state )
	AM_RANGE(0x0000, 0x8fff) AM_READ_BANK("bank1") AM_WRITEONLY AM_SHARE("videoram")
	AM_RANGE(0x9000, 0xbfff) AM_RAM
	AM_RANGE(0xc000, 0xc00f) AM_MIRROR(0x03f0) AM_WRITEONLY AM_SHARE("paletteram")
	AM_RANGE(0xc804, 0xc807) AM_MIRROR(0x00f0) AM_DEVREADWRITE("pia_0", pia6821_device, read, write)
	AM_RANGE(0xc80c, 0xc80f) AM_MIRROR(0x00f0) AM_DEVREADWRITE("pia_1", pia6821_device, read, write)
	AM_RANGE(0xc900, 0xc9ff) AM_WRITE(sinistar_vram_select_w)
	AM_RANGE(0xca00, 0xca07) AM_MIRROR(0x00f8) AM_WRITE(williams_blitter_w)
	AM_RANGE(0xcb00, 0xcbff) AM_READ(williams_video_counter_r)
	AM_RANGE(0xcbff, 0xcbff) AM_WRITE(williams_watchdog_reset_w)
	AM_RANGE(0xcc00, 0xcfff) AM_RAM_WRITE(williams_cmos_w) AM_SHARE("nvram")
	AM_RANGE(0xd000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Blaster memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( blaster_map, AS_PROGRAM, 8, blaster_state )
	AM_RANGE(0x0000, 0x3fff) AM_READ_BANK("bank1") AM_WRITEONLY AM_SHARE("videoram")
	AM_RANGE(0x4000, 0x8fff) AM_READ_BANK("bank2") AM_WRITEONLY
	AM_RANGE(0xbb00, 0xbbff) AM_WRITEONLY AM_SHARE("blaster_pal0")
	AM_RANGE(0xbc00, 0xbcff) AM_WRITEONLY AM_SHARE("blaster_scan")
	AM_RANGE(0x9000, 0xbfff) AM_RAM
	AM_RANGE(0xc000, 0xc00f) AM_MIRROR(0x03f0) AM_WRITEONLY AM_SHARE("paletteram")
	AM_RANGE(0xc804, 0xc807) AM_MIRROR(0x00f0) AM_DEVREADWRITE("pia_0", pia6821_device, read, write)
	AM_RANGE(0xc80c, 0xc80f) AM_MIRROR(0x00f0) AM_DEVREADWRITE("pia_1", pia6821_device, read, write)
	AM_RANGE(0xc900, 0xc93f) AM_WRITE(blaster_vram_select_w)
	AM_RANGE(0xc940, 0xc97f) AM_WRITE(blaster_remap_select_w)
	AM_RANGE(0xc980, 0xc9bf) AM_WRITE(blaster_bank_select_w)
	AM_RANGE(0xc9c0, 0xc9ff) AM_WRITE(blaster_video_control_w)
	AM_RANGE(0xca00, 0xca07) AM_MIRROR(0x00f8) AM_WRITE(williams_blitter_w)
	AM_RANGE(0xcb00, 0xcbff) AM_READ(williams_video_counter_r)
	AM_RANGE(0xcbff, 0xcbff) AM_WRITE(williams_watchdog_reset_w)
	AM_RANGE(0xcc00, 0xcfff) AM_RAM_WRITE(williams_cmos_w) AM_SHARE("nvram")
	AM_RANGE(0xd000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Later Williams memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( williams2_common_map, AS_PROGRAM, 8, williams2_state )
	AM_RANGE(0x0000, 0x7fff) AM_READ_BANK("bank1")
	AM_RANGE(0x8000, 0x87ff) AM_DEVICE("bank8000", address_map_bank_device, amap8)
	AM_RANGE(0x0000, 0xbfff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0xc000, 0xc7ff) AM_RAM_WRITE(williams2_tileram_w) AM_SHARE("williams2_tile")
	AM_RANGE(0xc800, 0xc87f) AM_WRITE(williams2_bank_select_w)
	AM_RANGE(0xc880, 0xc887) AM_MIRROR(0x0078) AM_WRITE(williams_blitter_w)
	AM_RANGE(0xc900, 0xc97f) AM_WRITE(williams2_watchdog_reset_w)
	AM_RANGE(0xc980, 0xc983) AM_MIRROR(0x0070) AM_DEVREADWRITE("pia_1", pia6821_device, read, write)
	AM_RANGE(0xc984, 0xc987) AM_MIRROR(0x0070) AM_DEVREADWRITE("pia_0", pia6821_device, read, write)
	AM_RANGE(0xc98c, 0xc98f) AM_MIRROR(0x0070) AM_WRITE(williams2_7segment_w)
	AM_RANGE(0xcb00, 0xcb1f) AM_WRITE(williams2_fg_select_w)
	AM_RANGE(0xcb20, 0xcb3f) AM_WRITE(williams2_bg_select_w)
	AM_RANGE(0xcb40, 0xcb5f) AM_WRITE(williams2_xscroll_low_w)
	AM_RANGE(0xcb60, 0xcb7f) AM_WRITE(williams2_xscroll_high_w)
	AM_RANGE(0xcb80, 0xcb9f) AM_WRITE(defender_video_control_w)
	AM_RANGE(0xcba0, 0xcbbf) AM_WRITE(williams2_blit_window_enable_w)
	AM_RANGE(0xcbe0, 0xcbef) AM_READ(williams_video_counter_r)
	AM_RANGE(0xcc00, 0xcfff) AM_RAM_WRITE(williams_cmos_w) AM_SHARE("nvram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( williams2_bank8000_map, AS_PROGRAM, 8, williams2_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAMBANK("vram8000")
	AM_RANGE(0x0800, 0x0fff) AM_RAM_WRITE(williams2_paletteram_w) AM_SHARE("paletteram")
ADDRESS_MAP_END


/* mysticm and inferno: D000-DFFF is RAM */
static ADDRESS_MAP_START( williams2_d000_ram_map, AS_PROGRAM, 8, williams2_state )
	AM_IMPORT_FROM(williams2_common_map)
	AM_RANGE(0xd000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END


/* tshoot and joust2: D000-DFFF is ROM */
static ADDRESS_MAP_START( williams2_d000_rom_map, AS_PROGRAM, 8, williams2_state )
	AM_IMPORT_FROM(williams2_common_map)
	AM_RANGE(0xd000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Sound board memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( defender_sound_map, AS_PROGRAM, 8, williams_state )
	AM_RANGE(0x0000, 0x007f) AM_RAM     /* internal RAM */
	AM_RANGE(0x0400, 0x0403) AM_MIRROR(0x8000) AM_DEVREADWRITE("pia_2", pia6821_device, read, write)
	AM_RANGE(0xb000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, williams_state )
	AM_RANGE(0x0000, 0x007f) AM_RAM     /* internal RAM */
	AM_RANGE(0x0080, 0x00ff) AM_RAM     /* MC6810 RAM */
	AM_RANGE(0x0400, 0x0403) AM_MIRROR(0x8000) AM_DEVREADWRITE("pia_2", pia6821_device, read, write)
	AM_RANGE(0xb000, 0xffff) AM_ROM
ADDRESS_MAP_END

/* Same as above, but for second sound board */
static ADDRESS_MAP_START( sound_map_b, AS_PROGRAM, 8, blaster_state )
	AM_RANGE(0x0000, 0x007f) AM_RAM     /* internal RAM */
	AM_RANGE(0x0080, 0x00ff) AM_RAM     /* MC6810 RAM */
	AM_RANGE(0x0400, 0x0403) AM_MIRROR(0x8000) AM_DEVREADWRITE("pia_2b", pia6821_device, read, write)
	AM_RANGE(0xb000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Later sound board memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( williams2_sound_map, AS_PROGRAM, 8, williams2_state )
	AM_RANGE(0x0000, 0x007f) AM_RAM     /* internal RAM */
	AM_RANGE(0x0080, 0x00ff) AM_RAM     /* MC6810 RAM */
	AM_RANGE(0x2000, 0x2003) AM_MIRROR(0x1ffc) AM_DEVREADWRITE("pia_2", pia6821_device, read, write)
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Advance") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("High Score Reset") PORT_CODE(KEYCODE_9)
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE PORT_CODE(KEYCODE_F1) /* ? */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Advance") PORT_CODE(KEYCODE_F2) /* ? */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
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
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ))       /* documented as unused */
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ))       /* documented as unused */
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
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) ) // coins don't work if you change this..
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 ) // where is coin 2?
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Advance") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("High Score Reset") PORT_CODE(KEYCODE_9)
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Advance") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("High Score Reset") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( joust )
	PORT_START("IN0")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, williams_state,williams_mux_r, "INP2\0INP1")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Advance") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("High Score Reset") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("INP2") /* muxed into IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("INP1") /* muxed into IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Advance") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("High Score Reset") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( splat )
	PORT_START("IN0")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0xcf, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, williams_state,williams_mux_r, "INP2\0INP1")

	PORT_START("IN1")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, williams_state,williams_mux_r, "INP2A\0INP1A")
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Advance") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("High Score Reset") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )

	PORT_START("INP2") /* muxed into IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("INP1") /* muxed into IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY PORT_PLAYER(1)

	PORT_START("INP2A") /* muxed into IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("INP1A") /* muxed into IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY PORT_PLAYER(1)

INPUT_PORTS_END


static INPUT_PORTS_START( sinistar )
	PORT_START("IN0")
	/* pseudo analog joystick, see below */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Advance") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("High Score Reset") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("49WAYX")    /* converted by williams_49way_port_0_r() */
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("49WAYY")    /* converted by williams_49way_port_0_r() */
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Advance") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("High Score Reset") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( blaster )
	PORT_START("IN0")
	/* pseudo analog joystick, see below */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Advance") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("High Score Reset") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("49WAYX")    /* converted by williams_49way_port_0_r() */
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("49WAYY")    /* converted by williams_49way_port_0_r() */
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE
INPUT_PORTS_END


static INPUT_PORTS_START( blastkit )
	PORT_START("IN0")
	/* pseudo analog joystick, see below */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Advance") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("High Score Reset") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("49WAYX")    /* converted by williams_49way_port_0_r() */
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("49WAYY")    /* converted by williams_49way_port_0_r() */
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Advance") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("High Score Reset") PORT_CODE(KEYCODE_9)
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

	PORT_START("AN0")       /* analog */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("AN1")       /* analog */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_PLAYER(1)

	PORT_START("AN2")       /* analog */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("AN3")       /* analog */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( alienar )
	PORT_START("IN0")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0xcf, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, williams_state,williams_mux_r, "INP2\0INP1")

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Advance") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("High Score Reset") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("INP2")  /* muxed into IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)

	PORT_START("INP1")   /* muxed into IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
INPUT_PORTS_END


static INPUT_PORTS_START( lottofun )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r)

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Memory Protect") PORT_CODE(KEYCODE_F1) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Advance") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("High Score Reset") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // COIN1.5? :)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) // Sound board handshake
INPUT_PORTS_END


static INPUT_PORTS_START( mysticm )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED ) /* Key */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Advance") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("High Score Reset") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( tshoot )
	PORT_START("IN0")
	PORT_BIT( 0x3f, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, williams_state,williams_mux_r, "INP1X\0INP1Y")
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_BUTTON1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x3C, IP_ACTIVE_HIGH, IPT_UNUSED ) /* 0011-1100 output */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Advance") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("High Score Reset") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("INP1X") /* muxed into IN0 */
	PORT_BIT( 0x3F, 0x20, IPT_AD_STICK_Y ) PORT_MINMAX(0,0x3F) PORT_SENSITIVITY(25) PORT_KEYDELTA(10)

	PORT_START("INP1Y") /* muxed into IN0 */
	PORT_BIT( 0x3F, 0x20, IPT_AD_STICK_X ) PORT_MINMAX(0,0x3F) PORT_SENSITIVITY(25) PORT_KEYDELTA(10)
INPUT_PORTS_END


static INPUT_PORTS_START( inferno )
	PORT_START("IN0")
	PORT_BIT( 0xFF, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, williams_state,williams_mux_r, "INP1\0INP2")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x3C, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Advance") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("High Score Reset") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("INP1")  /* muxed into IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_PLAYER(1)

	PORT_START("INP2")  /* muxed into IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( joust2 )
	PORT_START("IN0")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, williams_state,williams_mux_r, "INP1\0INP2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Auto Up / Manual Down") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Advance") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("High Score Reset") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("INP1") /* muxed into IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INP2") /* muxed into IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
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
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8, 32*8, 36*8, 40*8, 44*8, 48*8, 52*8, 56*8, 60*8 },
	4*16*8
};


static GFXDECODE_START( williams2 )
	GFXDECODE_ENTRY( "gfx1", 0, williams2_layout, 0, 8 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( williams, williams_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, MASTER_CLOCK/3/4)
	MCFG_CPU_PROGRAM_MAP(williams_map)

	MCFG_CPU_ADD("soundcpu", M6808, SOUND_CLOCK) // internal clock divider of 4, effective frequency is 894.886kHz
	MCFG_CPU_PROGRAM_MAP(sound_map)

	MCFG_MACHINE_START_OVERRIDE(williams_state,williams)
	MCFG_MACHINE_RESET_OVERRIDE(williams_state,williams)
	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_TIMER_DRIVER_ADD("scan_timer", williams_state, williams_va11_callback)
	MCFG_TIMER_DRIVER_ADD("240_timer", williams_state, williams_count240_callback)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_SCANLINE | VIDEO_ALWAYS_UPDATE)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK*2/3, 512, 6, 298, 260, 7, 247)
	MCFG_SCREEN_UPDATE_DRIVER(williams_state, screen_update_williams)

	MCFG_VIDEO_START_OVERRIDE(williams_state,williams)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DAC_ADD("wmsdac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* pia */
	MCFG_DEVICE_ADD("pia_0", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(IOPORT("IN0"))
	MCFG_PIA_READPB_HANDLER(IOPORT("IN1"))

	MCFG_DEVICE_ADD("pia_1", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(IOPORT("IN2"))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(williams_state, williams_snd_cmd_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(williams_state, williams_main_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(williams_state, williams_main_irq))

	MCFG_DEVICE_ADD("pia_2", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(DEVWRITE8("wmsdac", dac_device, write_unsigned8))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(williams_state,williams_snd_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(williams_state,williams_snd_irq))
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( defender, williams )

	/* basic machine hardware */

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(defender_map)

	MCFG_CPU_MODIFY("soundcpu")
	MCFG_CPU_PROGRAM_MAP(defender_sound_map)

	MCFG_DEVICE_ADD("bankc000", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(defender_bankc000_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_BIG)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(16)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x1000)

	MCFG_MACHINE_START_OVERRIDE(williams_state,defender)
	MCFG_MACHINE_RESET_OVERRIDE(williams_state,defender)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(12, 304-1, 7, 247-1)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( jin, defender ) // needs a different screen size or the credit text is clipped
	/* basic machine hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, 315, 7, 247-1)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( williams_muxed, williams )

	/* basic machine hardware */

	/* pia */
	MCFG_DEVICE_MODIFY("pia_0")
	MCFG_PIA_CB2_HANDLER(WRITELINE(williams_state, williams_port_select_w))
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( spdball, williams )

	/* basic machine hardware */

	/* pia */
	MCFG_DEVICE_ADD("pia_3", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(IOPORT("IN3"))
	MCFG_PIA_READPB_HANDLER(IOPORT("IN4"))
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( lottofun, williams )

	/* basic machine hardware */

	/* pia */
	MCFG_DEVICE_MODIFY("pia_0")
	MCFG_PIA_WRITEPB_HANDLER(DEVWRITE8("ticket", ticket_dispenser_device, write))
	MCFG_PIA_CA2_HANDLER(WRITELINE(williams_state, lottofun_coin_lock_w))

	MCFG_TICKET_DISPENSER_ADD("ticket", attotime::from_msec(70), TICKET_MOTOR_ACTIVE_LOW, TICKET_STATUS_ACTIVE_HIGH)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( sinistar, williams )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sinistar_map)

	/* sound hardware */
	MCFG_SOUND_ADD("cvsd", HC55516, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	/* pia */
	MCFG_DEVICE_MODIFY("pia_0")
	MCFG_PIA_READPA_HANDLER(READ8(williams_state, williams_49way_port_0_r))

	MCFG_DEVICE_MODIFY("pia_2")
	MCFG_PIA_CA2_HANDLER(DEVWRITELINE("cvsd", hc55516_device, digit_w))
	MCFG_PIA_CB2_HANDLER(DEVWRITELINE("cvsd", hc55516_device, clock_w))
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( playball, williams )

	/* basic machine hardware */

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(6, 298-1, 8, 240-1)

	/* sound hardware */
	MCFG_SOUND_ADD("cvsd", HC55516, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	/* pia */
	MCFG_DEVICE_MODIFY("pia_1")
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(williams_state, playball_snd_cmd_w))

	MCFG_DEVICE_MODIFY("pia_2")
	MCFG_PIA_CA2_HANDLER(DEVWRITELINE("cvsd", hc55516_device, digit_w))
	MCFG_PIA_CB2_HANDLER(DEVWRITELINE("cvsd", hc55516_device, clock_w))
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED_CLASS( blastkit, williams, blaster_state )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(blaster_map)

	MCFG_MACHINE_START_OVERRIDE(blaster_state,blaster)
	MCFG_MACHINE_RESET_OVERRIDE(blaster_state,blaster)

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(blaster_state,blaster)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(blaster_state, screen_update_blaster)

	/* pia */
	MCFG_DEVICE_MODIFY("pia_0")
	MCFG_PIA_READPA_HANDLER(READ8(williams_state, williams_input_port_49way_0_5_r))
	MCFG_PIA_CB2_HANDLER(WRITELINE(williams_state, williams_port_select_w))
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( blaster, blastkit )

	/* basic machine hardware */
	MCFG_CPU_ADD("soundcpu_b", M6808, SOUND_CLOCK) // internal clock divider of 4, effective frequency is 894.886kHz
	MCFG_CPU_PROGRAM_MAP(sound_map_b)

	/* pia */
	MCFG_DEVICE_MODIFY("pia_0")
	MCFG_PIA_READPA_HANDLER(READ8(williams_state, williams_49way_port_0_r))

	MCFG_DEVICE_MODIFY("pia_1")
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(blaster_state, blaster_snd_cmd_w))

	MCFG_DEVICE_ADD("pia_2b", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(DEVWRITE8("wmsdac_b", dac_device, write_unsigned8))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(blaster_state,williams_snd_irq_b))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(blaster_state,williams_snd_irq_b))

	/* sound hardware */
	MCFG_DEVICE_REMOVE("wmsdac")
	MCFG_DEVICE_REMOVE("mono")

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_DAC_ADD("wmsdac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_DAC_ADD("wmsdac_b")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( williams2, williams2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, MASTER_CLOCK/3/4)
	MCFG_CPU_PROGRAM_MAP(williams2_d000_ram_map)

	MCFG_CPU_ADD("soundcpu", M6808, MASTER_CLOCK/3) /* yes, this is different from the older games */
	MCFG_CPU_PROGRAM_MAP(williams2_sound_map)

	MCFG_DEVICE_ADD("bank8000", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(williams2_bank8000_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_BIG)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(12)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x0800)

	MCFG_MACHINE_START_OVERRIDE(williams2_state,williams2)
	MCFG_MACHINE_RESET_OVERRIDE(williams2_state,williams2)
	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_TIMER_DRIVER_ADD("scan_timer", williams2_state, williams2_va11_callback)
	MCFG_TIMER_DRIVER_ADD("254_timer", williams2_state, williams2_endscreen_callback)

	/* video hardware */
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", williams2)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_SCANLINE | VIDEO_ALWAYS_UPDATE)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK*2/3, 512, 8, 284, 260, 8, 248)
	MCFG_SCREEN_UPDATE_DRIVER(williams2_state, screen_update_williams2)

	MCFG_VIDEO_START_OVERRIDE(williams2_state,williams2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DAC_ADD("wmsdac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* pia */
	MCFG_DEVICE_ADD("pia_0", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(IOPORT("IN0"))
	MCFG_PIA_READPB_HANDLER(IOPORT("IN1"))
	MCFG_PIA_CA2_HANDLER(WRITELINE(williams_state, williams_port_select_w))

	MCFG_DEVICE_ADD("pia_1", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(IOPORT("IN2"))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(williams2_state,williams2_snd_cmd_w))
	MCFG_PIA_CB2_HANDLER(DEVWRITELINE("pia_2", pia6821_device, ca1_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(williams_state,williams_main_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(williams_state,williams_main_irq))

	MCFG_DEVICE_ADD("pia_2", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(DEVWRITE8("pia_1", pia6821_device, portb_w))
	MCFG_PIA_WRITEPB_HANDLER(DEVWRITE8("wmsdac", dac_device, write_unsigned8))
	MCFG_PIA_CA2_HANDLER(DEVWRITELINE("pia_1", pia6821_device, cb1_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(williams_state,williams_snd_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(williams_state,williams_snd_irq))
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( mysticm, williams2 )

	/* basic machine hardware */

	/* pia */
	MCFG_DEVICE_MODIFY("pia_0")
	MCFG_PIA_CA2_HANDLER(NULL)
	MCFG_PIA_IRQA_HANDLER(WRITELINE(williams_state,williams_main_firq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(williams2_state,mysticm_main_irq))

	MCFG_DEVICE_MODIFY("pia_1")
	MCFG_PIA_IRQA_HANDLER(WRITELINE(williams2_state,mysticm_main_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(williams2_state,mysticm_main_irq))
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( tshoot, williams2 )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(williams2_d000_rom_map)

	/* pia */
	MCFG_DEVICE_MODIFY("pia_0")
	MCFG_PIA_READPA_HANDLER(READ8(williams2_state,tshoot_input_port_0_3_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(williams2_state,tshoot_lamp_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(williams2_state,tshoot_main_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(williams2_state,tshoot_main_irq))

	MCFG_DEVICE_MODIFY("pia_1")
	MCFG_PIA_IRQA_HANDLER(WRITELINE(williams2_state,tshoot_main_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(williams2_state,tshoot_main_irq))

	MCFG_DEVICE_MODIFY("pia_2")
	MCFG_PIA_CB2_HANDLER(WRITELINE(williams2_state,tshoot_maxvol_w))
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED_CLASS( joust2, williams2, joust2_state )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(williams2_d000_rom_map)

	MCFG_WILLIAMS_CVSD_SOUND_ADD("cvsd")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_MACHINE_START_OVERRIDE(joust2_state,joust2)
	MCFG_MACHINE_RESET_OVERRIDE(joust2_state,joust2)

	/* pia */
	MCFG_DEVICE_MODIFY("pia_1")
	MCFG_PIA_READPA_HANDLER(IOPORT("IN2"))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(joust2_state,joust2_snd_cmd_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(joust2_state,joust2_pia_3_cb1_w))
	MCFG_PIA_CB2_HANDLER(DEVWRITELINE("pia_2", pia6821_device, ca1_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(williams_state,williams_main_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(williams_state,williams_main_irq))
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( defender )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "defend.1",     0x0d000, 0x0800, CRC(c3e52d7e) SHA1(a57f5278ffe44248fc73f9925d107f4024ad981a) )
	ROM_LOAD( "defend.4",     0x0d800, 0x0800, CRC(9a72348b) SHA1(ed6ce796702ff32209ced3cb1ba3837dbafa526f) )
	ROM_LOAD( "defend.2",     0x0e000, 0x1000, CRC(89b75984) SHA1(a9481478da38f99efb67f0ecf82d084e14b93b42) )
	ROM_LOAD( "defend.3",     0x0f000, 0x1000, CRC(94f51e9b) SHA1(a24cfc55de56a72758c76fe2a55f1ec6c353b16f) )
	ROM_LOAD( "defend.9",     0x10000, 0x0800, CRC(6870e8a5) SHA1(67ccc194b1753a18af0c85f5e603355549c4f727) )
	ROM_LOAD( "defend.12",    0x10800, 0x0800, CRC(f1f88938) SHA1(26e48dfeefa0766837b1e762695b9532dbc8bc5e) )
	ROM_LOAD( "defend.8",     0x11000, 0x0800, CRC(b649e306) SHA1(9d7bc3c89e5a53c575946f06702c722b864b1ff0) )
	ROM_LOAD( "defend.11",    0x11800, 0x0800, CRC(9deaf6d9) SHA1(59b018ba0f3fe6eadfd387dc180ac281460358bc) )
	ROM_LOAD( "defend.7",     0x12000, 0x0800, CRC(339e092e) SHA1(2f89951dbe55d80df43df8dcf497171f73e726d3) )
	ROM_LOAD( "defend.10",    0x12800, 0x0800, CRC(a543b167) SHA1(9292b94b0d74e57e03aada4852ad1997c34122ff) )
	ROM_LOAD( "defend.6",     0x16000, 0x0800, CRC(65f4efd1) SHA1(a960fd1559ed74b81deba434391e49fc6ec389ca) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "defend.snd",   0xf800, 0x0800, CRC(fefd5b48) SHA1(ceb0d18483f0691978c604db94417e6941ad7ff2) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder.2",   0x0000, 0x0200, CRC(8dd98da5) SHA1(da979604f7a2aa8b5a6d4a5debd2e80f77569e35) )
	ROM_LOAD( "decoder.3",   0x0200, 0x0200, CRC(c3f45f70) SHA1(d19036cbc46b130548873597b44b8b70758f25c4) )
ROM_END

ROM_START( defenderg )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "defeng01.bin", 0x0d000, 0x0800, CRC(6111d74d) SHA1(2a335bdce8269f75012df44b446cb261ddd5924c) )
	ROM_LOAD( "defeng04.bin", 0x0d800, 0x0800, CRC(3cfc04ce) SHA1(8ee65c7daed4d6956d0e15ada4dc414c28376012) )
	ROM_LOAD( "defeng02.bin", 0x0e000, 0x1000, CRC(d184ab6b) SHA1(ed61a95b04f6162aedba8a72bc46005b77283955) )
	ROM_LOAD( "defeng03.bin", 0x0f000, 0x1000, CRC(788b76d7) SHA1(92987207770a870b5be61c820e9e229801f1fa7a) )
	ROM_LOAD( "defeng09.bin", 0x10000, 0x0800, CRC(f57caa62) SHA1(c8c91b96fd3bc98eddcc1503159050dae5755001) )
	ROM_LOAD( "defeng12.bin", 0x10800, 0x0800, CRC(33db686f) SHA1(34bc7fa10b7996efcc53d3a891b2983874269828) )
	ROM_LOAD( "defeng08.bin", 0x11000, 0x0800, CRC(9a9eb3d2) SHA1(306a3a24931e1aa5fcfd71e3f117cc726d0920ac) )
	ROM_LOAD( "defeng11.bin", 0x11800, 0x0800, CRC(5ca4e860) SHA1(031188c009b8fca92703a0cc0c2bb44976212ae9) )
	ROM_LOAD( "defeng07.bin", 0x12000, 0x0800, CRC(545c3326) SHA1(98199df5206c261061b0108c68ab9128fa0779eb) )
	ROM_LOAD( "defeng10.bin", 0x12800, 0x0800, CRC(941cf34e) SHA1(411dcb18b67585982672ff687a9249f4890faa1b) )
	ROM_LOAD( "defeng06.bin", 0x16000, 0x0800, CRC(3af34c05) SHA1(71f3ced06a373fa4805c856bd9fc97760787a920) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "defend.snd",   0xf800, 0x0800, CRC(fefd5b48) SHA1(ceb0d18483f0691978c604db94417e6941ad7ff2) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "decoder.1",   0x0000, 0x0200, CRC(8dd98da5) SHA1(da979604f7a2aa8b5a6d4a5debd2e80f77569e35) )
ROM_END

ROM_START( defenderb )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "wb01.bin",     0x0d000, 0x1000, CRC(0ee1019d) SHA1(a76247e825b8267abfd195c12f96348fe10d4cbc) )
	ROM_LOAD( "defeng02.bin", 0x0e000, 0x1000, CRC(d184ab6b) SHA1(ed61a95b04f6162aedba8a72bc46005b77283955) )
	ROM_LOAD( "wb03.bin",     0x0f000, 0x1000, CRC(a732d649) SHA1(b681882c02c5870ad613edc77255969a5f796422) )
	ROM_LOAD( "defeng09.bin", 0x10000, 0x0800, CRC(f57caa62) SHA1(c8c91b96fd3bc98eddcc1503159050dae5755001) )
	ROM_LOAD( "defeng12.bin", 0x10800, 0x0800, CRC(33db686f) SHA1(34bc7fa10b7996efcc53d3a891b2983874269828) )
	ROM_LOAD( "defeng08.bin", 0x11000, 0x0800, CRC(9a9eb3d2) SHA1(306a3a24931e1aa5fcfd71e3f117cc726d0920ac) )
	ROM_LOAD( "defeng11.bin", 0x11800, 0x0800, CRC(5ca4e860) SHA1(031188c009b8fca92703a0cc0c2bb44976212ae9) )
	ROM_LOAD( "defeng07.bin", 0x12000, 0x0800, CRC(545c3326) SHA1(98199df5206c261061b0108c68ab9128fa0779eb) )
	ROM_LOAD( "defeng10.bin", 0x12800, 0x0800, CRC(941cf34e) SHA1(411dcb18b67585982672ff687a9249f4890faa1b) )
	ROM_LOAD( "defeng06.bin", 0x16000, 0x0800, CRC(3af34c05) SHA1(71f3ced06a373fa4805c856bd9fc97760787a920) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "defend.snd",   0xf800, 0x0800, CRC(fefd5b48) SHA1(ceb0d18483f0691978c604db94417e6941ad7ff2) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "decoder.1",   0x0000, 0x0200, CRC(8dd98da5) SHA1(da979604f7a2aa8b5a6d4a5debd2e80f77569e35) )
ROM_END

/* the white set seems to be the source of the defcmnd & startrkd bootlegs */
ROM_START( defenderw )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "rom1.bin",     0x0d000, 0x1000, CRC(5af871e3) SHA1(f9a42619b37db2eb07d0302ac9d0ff5c1923c21d) )
	ROM_LOAD( "rom2.bin",     0x0e000, 0x1000, CRC(1126adc9) SHA1(526cf1ca3a7eefd6115d74ac9af1a50774cc258e) )
	ROM_LOAD( "rom3.bin",     0x0f000, 0x1000, CRC(4097b46b) SHA1(8f506dc59b129c9441d813062fc38747619678db) )
	ROM_LOAD( "rom9.bin",     0x10000, 0x0800, CRC(93012991) SHA1(9e06ed4a489b2ed063f83b708d3e7c6a02e45389) )
	ROM_LOAD( "rom12.bin",    0x10800, 0x0800, CRC(4bdd8dc4) SHA1(e7503e68608e8f7bb066c99e1e32c6fe060c1dd3) )
	ROM_LOAD( "rom8.bin",     0x11000, 0x0800, CRC(5227fc0b) SHA1(1e6fd398b5beef0be58667f1f0a789a76edd5eb9) )
	ROM_LOAD( "rom11.bin",    0x11800, 0x0800, CRC(d068f0c5) SHA1(d32a4232756ca05972780cb35b0add12b31e8283) )
	ROM_LOAD( "rom7.bin",     0x12000, 0x0800, CRC(fef4cb77) SHA1(96202e97f3392bc043a252e78d1c42b51c38d269) )
	ROM_LOAD( "rom10.bin",    0x12800, 0x0800, CRC(49b50b40) SHA1(91cf841271a2f7d06f81477b4a450eb4580c7ca5) ) // hand-repaired with startrkd rom, but should be good
	ROM_LOAD( "rom6.bin",     0x16000, 0x0800, CRC(43d42a1b) SHA1(b13d59940646451c00b49bbe4a41b9e2df4d7758) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "defend.snd",   0xf800, 0x0800, CRC(fefd5b48) SHA1(ceb0d18483f0691978c604db94417e6941ad7ff2) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "decoder.1",   0x0000, 0x0200, CRC(8dd98da5) SHA1(da979604f7a2aa8b5a6d4a5debd2e80f77569e35) )
ROM_END

ROM_START( defndjeu )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "15", 0x0d000, 0x1000, CRC(706a24bd) SHA1(60cef3d4f7204eff42de2c08244863e83bc842b4) )
	ROM_LOAD( "16", 0x0e000, 0x1000, CRC(03201532) SHA1(77e8c10ba0ecb6e7a7cb4229a5025c4b9ea4c73e) )
	ROM_LOAD( "17", 0x0f000, 0x1000, CRC(25287eca) SHA1(ec81181a5a0ac2adf7c0dabbec638f886c13e6ec) )
	ROM_LOAD( "21", 0x10000, 0x1000, CRC(bddb71a3) SHA1(ecba4c09a9d59fd7aa02efa240461df89159d2ec) )
	ROM_LOAD( "20", 0x11000, 0x1000, BAD_DUMP CRC(12fa0788) SHA1(7464386521c9db0153caf1ea05a353f0018651e5) )
	ROM_LOAD( "19", 0x12000, 0x1000, CRC(769f5984) SHA1(0ea49754b45bc214fd2b69846ede738994f07ee3) )
	ROM_LOAD( "18", 0x16000, 0x1000, CRC(e99d5679) SHA1(b4344a32aed6cc64284661c03993a59718289c82) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "s", 0xf800, 0x0800, CRC(cb79ae42) SHA1(d22bef68ef62aa012f1919338a33621138c2278b) )
ROM_END

ROM_START( tornado1 )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "torna1.bin", 0x0d000, 0x1000, CRC(706a24bd) SHA1(60cef3d4f7204eff42de2c08244863e83bc842b4) ) // same as defndjeu 15
	ROM_LOAD( "torna3.bin", 0x0e000, 0x1000, CRC(a79213b2) SHA1(ff9a59cb1d28af396a7b93df82a4bb825581bcce) )
	ROM_LOAD( "torna4.bin", 0x0f000, 0x1000, CRC(f96d3d26) SHA1(42a1add3c39c1376aa3292c9b85346bb480d329d) )
	ROM_LOAD( "tornc4.bin", 0x10000, 0x1000, CRC(e30f4c00) SHA1(afdf1877ae52bc027c4cd4a31b861aa50321a094) )
	ROM_LOAD( "tornb3.bin", 0x11000, 0x1000, CRC(0e3fef55) SHA1(d0350c36971b523aa490b29b64345f3777019a8d) )
	ROM_LOAD( "tornb1.bin", 0x12000, 0x1000, CRC(f2bef850) SHA1(78e50c790e3a8ebc4b65f2d827be8bd375bf4ef4) )
	ROM_LOAD( "tornb4.bin", 0x16000, 0x1000, CRC(e99d5679) SHA1(b4344a32aed6cc64284661c03993a59718289c82) ) // same as defndjeu 18

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "tornb6.bin", 0xf800, 0x0800, CRC(3685e033) SHA1(74d16675d9c16a6ea3af09cfbc20c3d6b0ab2311) )
	ROM_CONTINUE ( 0x000, 0x800 ) // cut rom instead?
ROM_END

/* I suspect this one is a bad dump */
ROM_START( tornado2 )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "tto15.bin", 0x0d000, 0x1000, BAD_DUMP CRC(910ac603) SHA1(33705b5ce4242a5865a8b4ecf27aa9e656067ea3) ) //  1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "to16.bin",  0x0e000, 0x1000, BAD_DUMP CRC(46ccd582) SHA1(4f74edb2c48273fce0531b7de54fd7eb925bf3df) )
	ROM_LOAD( "tto17.bin", 0x0f000, 0x1000, BAD_DUMP CRC(faa3613c) SHA1(79014de2021cd73525ecfe0e8d3e7da25e9ee1f3) )
	ROM_LOAD( "to21.bin",  0x10000, 0x1000, CRC(e30f4c00) SHA1(afdf1877ae52bc027c4cd4a31b861aa50321a094) ) // same as tornado1 tornc4.bin
	ROM_LOAD( "to20.bin",  0x11000, 0x1000, CRC(e90bdcb2) SHA1(591fbad03bfaf371a865d81982c03402acdf0421) ) //  1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "to19.bin",  0x12000, 0x1000, CRC(42885b4f) SHA1(759319f84ea2cec6808d744d190062a7105d7efc) )
	ROM_LOAD( "to18.bin",  0x16000, 0x1000, CRC(c15ffc03) SHA1(9093a8079b21484a999c965346accdbed91d0273) ) //  1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "to_s.bin",  0xf800, 0x0800, CRC(cb79ae42) SHA1(d22bef68ef62aa012f1919338a33621138c2278b) ) // same as defndjeu s
ROM_END

ROM_START( zero )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "zero-15", 0x0d000, 0x1000, CRC(706a24bd) SHA1(60cef3d4f7204eff42de2c08244863e83bc842b4) )
	ROM_LOAD( "zero-16", 0x0e000, 0x1000, CRC(a79213b2) SHA1(ff9a59cb1d28af396a7b93df82a4bb825581bcce) )
	ROM_LOAD( "zero-17", 0x0f000, 0x1000, CRC(25287eca) SHA1(ec81181a5a0ac2adf7c0dabbec638f886c13e6ec) )
	ROM_LOAD( "zero-21", 0x10000, 0x1000, CRC(7ca35cfd) SHA1(9afa4f6641082f73bfc3e2b800acf82dcc2bafb3) )
	ROM_LOAD( "zero-20", 0x11000, 0x1000, CRC(0757967f) SHA1(445e399f1fc834a9333549440171beb6e19a24a7) )
	ROM_LOAD( "zero-19", 0x12000, 0x1000, CRC(f2bef850) SHA1(78e50c790e3a8ebc4b65f2d827be8bd375bf4ef4) )
	ROM_LOAD( "zero-18", 0x16000, 0x1000, CRC(e99d5679) SHA1(b4344a32aed6cc64284661c03993a59718289c82) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "defend.snd",   0xf800, 0x0800, CRC(fefd5b48) SHA1(ceb0d18483f0691978c604db94417e6941ad7ff2) )
ROM_END

ROM_START( zero2 )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "15me.1a", 0x0d000, 0x1000, CRC(9323eee5) SHA1(727eaf669162a39c763203b925ab6b2cdb4626c1) )
	ROM_LOAD( "to16.3a", 0x0e000, 0x1000, CRC(a79213b2) SHA1(ff9a59cb1d28af396a7b93df82a4bb825581bcce) )
	ROM_LOAD( "17m5.4a", 0x0f000, 0x1000, CRC(16a3c0dd) SHA1(df11e4c7a62db85638d1fdda3602735b0d3f7f80) )
	ROM_LOAD( "21.4c",   0x10000, 0x1000, CRC(7ca35cfd) SHA1(9afa4f6641082f73bfc3e2b800acf82dcc2bafb3) )
	ROM_LOAD( "20m5.3b", 0x11000, 0x1000, CRC(7473955b) SHA1(b27515ca3fd2f938706415425226c4d7113eb276) )
	ROM_LOAD( "to19.1b", 0x12000, 0x1000, CRC(f2bef850) SHA1(78e50c790e3a8ebc4b65f2d827be8bd375bf4ef4) )
	ROM_LOAD( "18m5.4b", 0x16000, 0x1000, CRC(7e4afe43) SHA1(7a4f2128e48ed4d5e83885a3a72756822769fe5d) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "to4.6b",   0xf800, 0x0800, CRC(cb79ae42) SHA1(d22bef68ef62aa012f1919338a33621138c2278b) )
ROM_END

ROM_START( defcmnd )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "defcmnda.1",   0x0d000, 0x1000, CRC(68effc1d) SHA1(459fd95cdf94233e1a4302d1c166e0f7cc239579) )
	ROM_LOAD( "defcmnda.2",   0x0e000, 0x1000, CRC(1126adc9) SHA1(526cf1ca3a7eefd6115d74ac9af1a50774cc258e) )
	ROM_LOAD( "defcmnda.3",   0x0f000, 0x1000, CRC(7340209d) SHA1(d2cdab8ac4830ac027655ed7fe54314c5b87fdb3) )
	ROM_LOAD( "defcmnda.10",  0x10000, 0x0800, CRC(3dddae75) SHA1(f45fcf3e5ca9bf3edd692b4ee1e96f9f1d388522) )
	ROM_LOAD( "defcmnda.7",   0x10800, 0x0800, CRC(3f1e7cf8) SHA1(87afb4b1158e64039129bd8a9653bc61ab3e1e37) )
	ROM_LOAD( "defcmnda.9",   0x11000, 0x0800, CRC(8882e1ff) SHA1(9d54a39230acd01e0555f67ba2a3c9c6d66b59a1) )
	ROM_LOAD( "defcmnda.6",   0x11800, 0x0800, CRC(d068f0c5) SHA1(d32a4232756ca05972780cb35b0add12b31e8283) )
	ROM_LOAD( "defcmnda.8",   0x12000, 0x0800, CRC(fef4cb77) SHA1(96202e97f3392bc043a252e78d1c42b51c38d269) )
	ROM_LOAD( "defcmnda.5",   0x12800, 0x0800, CRC(49b50b40) SHA1(91cf841271a2f7d06f81477b4a450eb4580c7ca5) )
	ROM_LOAD( "defcmnda.4",   0x16000, 0x0800, CRC(43d42a1b) SHA1(b13d59940646451c00b49bbe4a41b9e2df4d7758) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "defcmnda.snd", 0xf800, 0x0800, CRC(f122d9c9) SHA1(70092fc354a2efbe7365be922fa36309b50d5c6f) )
ROM_END

ROM_START( startrkd )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "st_rom8.bin",     0x0d000, 0x1000, CRC(5af871e3) SHA1(f9a42619b37db2eb07d0302ac9d0ff5c1923c21d) )
	ROM_LOAD( "st_rom9.bin",     0x0e000, 0x1000, CRC(1126adc9) SHA1(526cf1ca3a7eefd6115d74ac9af1a50774cc258e) )
	ROM_LOAD( "st_rom10.bin",    0x0f000, 0x1000, CRC(4097b46b) SHA1(8f506dc59b129c9441d813062fc38747619678db) )
	ROM_LOAD( "st_rom6.bin",     0x10000, 0x0800, CRC(93012991) SHA1(9e06ed4a489b2ed063f83b708d3e7c6a02e45389) )
	ROM_LOAD( "st_rom5.bin",     0x10800, 0x0800, CRC(c6f0c004) SHA1(57c547b804ad3eceb33a9390bbffcfc0b63f16dd) )
	ROM_LOAD( "st_rom4.bin",     0x11000, 0x0800, CRC(b48430bf) SHA1(6572812f3a1e6eede3dff3273f16846799e79ed9) )
	ROM_LOAD( "st_rom3.bin",     0x11800, 0x0800, CRC(d068f0c5) SHA1(d32a4232756ca05972780cb35b0add12b31e8283) )
	ROM_LOAD( "st_rom2.bin",     0x12000, 0x0800, CRC(fef4cb77) SHA1(96202e97f3392bc043a252e78d1c42b51c38d269) )
	ROM_LOAD( "st_rom1.bin",     0x12800, 0x0800, CRC(d23d6cdb) SHA1(2ce9fe269598919f994339c8f8685d6a79e417d8) )
	ROM_LOAD( "st_rom7.bin",     0x16000, 0x0800, CRC(43d42a1b) SHA1(b13d59940646451c00b49bbe4a41b9e2df4d7758) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "defend.snd",   0xf800, 0x0800, CRC(fefd5b48) SHA1(ceb0d18483f0691978c604db94417e6941ad7ff2) )
ROM_END

ROM_START( defence )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "1",            0x0d000, 0x1000, CRC(ebc93622) SHA1(bd1c098e91b24409925d01aa25de013451dba8e6) )
	ROM_LOAD( "2",            0x0e000, 0x1000, CRC(2a4f4f44) SHA1(8c0519fcb631e05e967cf0953ab2749183655594) )
	ROM_LOAD( "3",            0x0f000, 0x1000, CRC(a4112f91) SHA1(aad7ae81da7c20c7f4c1ef41697c8900a0c81f8e) )
	ROM_LOAD( "0",            0x10000, 0x0800, CRC(7a1e5998) SHA1(c133f43427540b39a383db7f46298942420d138a) )
	ROM_LOAD( "7",            0x10800, 0x0800, CRC(4c2616a3) SHA1(247411e2bb6618f77df6ea74aef1743fafb491a3) )
	ROM_LOAD( "9",            0x11000, 0x0800, CRC(7b146003) SHA1(04746f1b037bf6549fd53cff8f8c37136fce099e) )
	ROM_LOAD( "6",            0x11800, 0x0800, CRC(6d748030) SHA1(060ddf95eeb1318695a25c8c082a670fcdf117e7) )
	ROM_LOAD( "8",            0x12000, 0x0800, CRC(52d5438b) SHA1(087268ca30a42c00dbeceb4df901ddf80ae50125) )
	ROM_LOAD( "5",            0x12800, 0x0800, CRC(4a270340) SHA1(317fcc3156a099dbe48a0658757a9d6c4c54b23a) )
	ROM_LOAD( "4",            0x16000, 0x0800, CRC(e13f457c) SHA1(c706babc0005dfeb3c1b880047da6ec04bce407d) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "defcmnda.snd", 0xf800, 0x0800, CRC(f122d9c9) SHA1(70092fc354a2efbe7365be922fa36309b50d5c6f) )
ROM_END

ROM_START( mayday )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "ic03-3.bin",  0x0d000, 0x1000, CRC(a1ff6e62) SHA1(c3c60ce94c6bdc4b07e45f386eff9a4aa4816953) )
	ROM_LOAD( "ic02-2.bin",  0x0e000, 0x1000, CRC(62183aea) SHA1(3843fe055ab6d3bb5a3362f57a63ce99e36cec47) )
	ROM_LOAD( "ic01-1.bin",  0x0f000, 0x1000, CRC(5dcb113f) SHA1(c41d671c336c68824771b7c4f0ffce39f1b6cd62) )
	ROM_LOAD( "ic04-4.bin",  0x10000, 0x1000, CRC(ea6a4ec8) SHA1(eaedc11968d88fd6f3c5b40c8d15d64ca6d0a1ab) )
	ROM_LOAD( "ic05-5.bin",  0x11000, 0x1000, CRC(0d797a3e) SHA1(289d2ecfebd7d71430d6624f3c9fbc91f9ef05cc) )
	ROM_LOAD( "ic06-6.bin",  0x12000, 0x1000, CRC(ee8bfcd6) SHA1(f68c44fdc18d57070aea604e261fb7b9407345a2) )
	ROM_LOAD( "ic07-7d.bin", 0x16000, 0x1000, CRC(d9c065e7) SHA1(ceeb58d1dfe14106271f17cf1c689b812216c3c0) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ic28-8.bin",  0xf800, 0x0800, CRC(fefd5b48) SHA1(ceb0d18483f0691978c604db94417e6941ad7ff2) )
ROM_END

ROM_START( maydaya )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "mayday.c",   0x0d000, 0x1000, CRC(872a2f2d) SHA1(5823e889151b34e3fa739775440c788cca0b44c6) )
	ROM_LOAD( "mayday.b",   0x0e000, 0x1000, CRC(c4ab5e22) SHA1(757fd9311cffea420b1de8f574e84c13c0aac77d) )
	ROM_LOAD( "mayday.a",   0x0f000, 0x1000, CRC(329a1318) SHA1(4aa1d05ca05f37460eccb450ae61c21d86348f02) )
	ROM_LOAD( "mayday.d",   0x10000, 0x1000, CRC(c2ae4716) SHA1(582f763eda7d7d51ed0580045d6c617246b104b7) )
	ROM_LOAD( "mayday.e",   0x11000, 0x1000, CRC(41225666) SHA1(6d9c0347ff85bf9f9ae4648976c3ee971fec0f53) )
	ROM_LOAD( "mayday.f",   0x12000, 0x1000, CRC(c39be3c0) SHA1(91ac61e20d325a3a018ffe57bd79bfdfdc5a3cbd) )
	ROM_LOAD( "mayday.g",   0x16000, 0x1000, CRC(2bd0f106) SHA1(ac74d74a54d5b464e4c82b5b46ad7d20cdb7b6d7) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ic28-8.bin", 0xf800, 0x0800, CRC(fefd5b48) SHA1(ceb0d18483f0691978c604db94417e6941ad7ff2) )
ROM_END

ROM_START( maydayb )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "ic03-3.bin",  0x0d000, 0x1000, CRC(a1ff6e62) SHA1(c3c60ce94c6bdc4b07e45f386eff9a4aa4816953) )
	ROM_LOAD( "ic02-2.bin",  0x0e000, 0x1000, CRC(62183aea) SHA1(3843fe055ab6d3bb5a3362f57a63ce99e36cec47) )
	ROM_LOAD( "ic01-1.bin",  0x0f000, 0x1000, CRC(5dcb113f) SHA1(c41d671c336c68824771b7c4f0ffce39f1b6cd62) )
	ROM_LOAD( "rom7.bin",    0x10000, 0x1000, CRC(0c3ca687) SHA1(a83f17c20f5767f092300266dd494bd0abf267bb) )
	ROM_LOAD( "ic05-5.bin",  0x11000, 0x1000, CRC(0d797a3e) SHA1(289d2ecfebd7d71430d6624f3c9fbc91f9ef05cc) )
	ROM_LOAD( "ic06-6.bin",  0x12000, 0x1000, CRC(ee8bfcd6) SHA1(f68c44fdc18d57070aea604e261fb7b9407345a2) )
	ROM_LOAD( "ic07-7d.bin", 0x16000, 0x1000, CRC(d9c065e7) SHA1(ceeb58d1dfe14106271f17cf1c689b812216c3c0) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ic28-8.bin",  0xf800, 0x0800, CRC(fefd5b48) SHA1(ceb0d18483f0691978c604db94417e6941ad7ff2) )

	ROM_REGION( 0x2000, "user1", 0 ) // what are these? alt (bad?) roms?
	ROM_LOAD( "rom11.bin",   0x0000, 0x0800, CRC(7e113979) SHA1(ac908afb6aa756fc4db1ffddbd3688aa07080693) ) // 11xxxxxxxxx = 0x00
	ROM_LOAD( "rom12.bin",   0x0800, 0x0800, CRC(a562c506) SHA1(a0bae41732f05caa80b9c13fba6ae4f01647e680) ) // 11xxxxxxxxx = 0x00
	ROM_LOAD( "rom6a.bin",   0x1000, 0x0800, CRC(8e4e981f) SHA1(685c1fca9373f4129c7c6b86f18900a1bd324019) )
	ROM_LOAD( "rom8-sos.bin",0x1800, 0x0800, CRC(6a9b383f) SHA1(10e71a3bb9492b6c34ff06760dd55c442611ca75) ) // FIXED BITS (xxxxxx1x)
ROM_END



ROM_START( batlzone )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "43-2732.rom.bin",  0x0d000, 0x1000, CRC(244334f8) SHA1(ac625a1858c372db6a748ef8aa504569aef6cad7) )
	ROM_LOAD( "42-2732.rom.bin",  0x0e000, 0x1000, CRC(62183aea) SHA1(3843fe055ab6d3bb5a3362f57a63ce99e36cec47) )
	ROM_LOAD( "41-2732.rom.bin",  0x0f000, 0x1000, CRC(a7e9093e) SHA1(d9d9641c9f8c060b2fa227b0620454067d0b0acc) )
	ROM_LOAD( "44-8532.rom.bin",  0x10000, 0x1000, CRC(bba3e626) SHA1(f2a364a25ee0cf91e25f8a20173bd9fb56cc2a72) )
	ROM_LOAD( "45-8532.rom.bin",  0x11000, 0x1000, CRC(43b3a0de) SHA1(674ff7110d07aeb09889eddb0d3dd0e7b16fe979) )
	ROM_LOAD( "46-8532.rom.bin",  0x12000, 0x1000, CRC(3df9b901) SHA1(6b2be3292b60e197154688ffd40789e937e17b4c) )
	ROM_LOAD( "47-8532.rom.bin",  0x16000, 0x1000, CRC(55a27e02) SHA1(23064ba59caed9b21e4ef6b944313e5bd2809dc5) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "48-2716.rom.bin",  0xf800, 0x0800, CRC(fefd5b48) SHA1(ceb0d18483f0691978c604db94417e6941ad7ff2) )
ROM_END


ROM_START( colony7 )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "cs03.bin",  0x0d000, 0x1000, CRC(7ee75ae5) SHA1(1d268d83c2b0c7897d9e783f5da4e1d892709ba4) )
	ROM_LOAD( "cs02.bin",  0x0e000, 0x1000, CRC(c60b08cb) SHA1(8cf91a2c2c04199b2870bb11e10fa6ffef5b877f) )
	ROM_LOAD( "cs01.bin",  0x0f000, 0x1000, CRC(1bc97436) SHA1(326692de3491925bbeea9b0e880d9133f0e6440c) )
	ROM_LOAD( "cs06.bin",  0x10000, 0x0800, CRC(318b95af) SHA1(cb276ef440436f6000a2d20252f3197a67965167) )
	ROM_LOAD( "cs04.bin",  0x10800, 0x0800, CRC(d740faee) SHA1(aad164e72ebb0de18487c5397ea33d300cf93423) )
	ROM_LOAD( "cs07.bin",  0x11000, 0x0800, CRC(0b23638b) SHA1(b577c0cefa3ea2df436ed0fa1efa8ecd04ff78b0) )
	ROM_LOAD( "cs05.bin",  0x11800, 0x0800, CRC(59e406a8) SHA1(b64081ca83b6f57ac8fb71b1f8618083f19b99de) )
	ROM_LOAD( "cs08.bin",  0x12000, 0x0800, CRC(3bfde87a) SHA1(f5047927833be97324c861aa93a8e95b457058c4) )
	ROM_RELOAD(            0x12800, 0x0800 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "cs11.bin",  0xf800, 0x0800, CRC(6032293c) SHA1(dd2c6afc1149a879d49e93d1a2fa8e1f6d0b043b) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "cs10.bin",  0x0000, 0x0200, CRC(25de5d85) SHA1(826f78c2fe847f594d261c280dd10b9e776bf4fd) )
	ROM_LOAD( "decoder.3", 0x0200, 0x0200, CRC(c3f45f70) SHA1(d19036cbc46b130548873597b44b8b70758f25c4) )
ROM_END

ROM_START( colony7a )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "cs03a.bin", 0x0d000, 0x1000, CRC(e0b0d23b) SHA1(4c50e00a71b3b2bf8d032a3cb496e5473204a8d6) )
	ROM_LOAD( "cs02a.bin", 0x0e000, 0x1000, CRC(370c6f41) SHA1(4e13a4cf9c1a3b1c354443c41549b59581d8b357) )
	ROM_LOAD( "cs01a.bin", 0x0f000, 0x1000, CRC(ba299946) SHA1(42e5d6ad0505f5a951d92165c9e2fa4e86659469) )
	ROM_LOAD( "cs06.bin",  0x10000, 0x0800, CRC(318b95af) SHA1(cb276ef440436f6000a2d20252f3197a67965167) )
	ROM_LOAD( "cs04.bin",  0x10800, 0x0800, CRC(d740faee) SHA1(aad164e72ebb0de18487c5397ea33d300cf93423) )
	ROM_LOAD( "cs07.bin",  0x11000, 0x0800, CRC(0b23638b) SHA1(b577c0cefa3ea2df436ed0fa1efa8ecd04ff78b0) )
	ROM_LOAD( "cs05.bin",  0x11800, 0x0800, CRC(59e406a8) SHA1(b64081ca83b6f57ac8fb71b1f8618083f19b99de) )
	ROM_LOAD( "cs08.bin",  0x12000, 0x0800, CRC(3bfde87a) SHA1(f5047927833be97324c861aa93a8e95b457058c4) )
	ROM_RELOAD(            0x12800, 0x0800 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "cs11.bin",  0xf800, 0x0800, CRC(6032293c) SHA1(dd2c6afc1149a879d49e93d1a2fa8e1f6d0b043b) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "cs10.bin",  0x0000, 0x0200, CRC(25de5d85) SHA1(826f78c2fe847f594d261c280dd10b9e776bf4fd) )
	ROM_LOAD( "decoder.3", 0x0200, 0x0200, CRC(c3f45f70) SHA1(d19036cbc46b130548873597b44b8b70758f25c4) )
ROM_END


ROM_START( jin )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "jin11.6c", 0x0d000, 0x1000, CRC(c4b9e93f) SHA1(5c3451fd8e108aed60c0ef1032873460aa81454e) )
	ROM_LOAD( "jin12.7c", 0x0e000, 0x1000, CRC(a8bc9fdd) SHA1(fffec12dbb0ef85f65bbfd93569c535d41773da4) )
	ROM_LOAD( "jin13.6d", 0x0f000, 0x1000, CRC(79779b85) SHA1(3c0aa595f8ee370790db89ea2ecc4868dde06a27) )
	ROM_LOAD( "jin14.4c", 0x10000, 0x1000, CRC(6a4df97e) SHA1(c1b8f180e1f935a88fb7203b77b97a614b091c9e) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "jin15.3f",   0xf800, 0x0800, CRC(fefd5b48) SHA1(ceb0d18483f0691978c604db94417e6941ad7ff2) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "jin.1a",   0x0000, 0x0200, CRC(8dd98da5) SHA1(da979604f7a2aa8b5a6d4a5debd2e80f77569e35) )
	ROM_LOAD( "jin.1l",   0x0200, 0x0200, CRC(c3f45f70) SHA1(d19036cbc46b130548873597b44b8b70758f25c4) )
ROM_END


ROM_START( stargate )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "10",         0x0d000, 0x1000, CRC(60b07ff7) SHA1(ba833f48ddfc1bd04ddb41b1d1c840d66ee7da30) )
	ROM_LOAD( "11",         0x0e000, 0x1000, CRC(7d2c5daf) SHA1(6ca39f493eb8b370154ad46ef01976d352c929e1) )
	ROM_LOAD( "12",         0x0f000, 0x1000, CRC(a0396670) SHA1(c46872550e0ca031453c6513f8f0448ecc9b5572) )
	ROM_LOAD( "01",         0x10000, 0x1000, CRC(88824d18) SHA1(f003a5a9319c4eb8991fa2aae3f10c72d6b8e81a) )
	ROM_LOAD( "02",         0x11000, 0x1000, CRC(afc614c5) SHA1(087c6da93318e8dc922d3d22e0a2af7b9759701c) )
	ROM_LOAD( "03",         0x12000, 0x1000, CRC(15077a9d) SHA1(7badb4318b208f49d7fa65e915d0aa22a1e37915) )
	ROM_LOAD( "04",         0x13000, 0x1000, CRC(a8b4bf0f) SHA1(6b4d47c2899fe9f14f9dab5928499f12078c437d) )
	ROM_LOAD( "05",         0x14000, 0x1000, CRC(2d306074) SHA1(54f871983699113e31bb756d4ca885c26c2d66b4) )
	ROM_LOAD( "06",         0x15000, 0x1000, CRC(53598dde) SHA1(54b02d944caf95283c9b6f0160e75ea8c4ccc97b) )
	ROM_LOAD( "07",         0x16000, 0x1000, CRC(23606060) SHA1(a487ffcd4920d1056b87469735f7e1002f6a2e49) )
	ROM_LOAD( "08",         0x17000, 0x1000, CRC(4ec490c7) SHA1(8726ebaf048db9608dfe365bf434ed5ca9452db7) )
	ROM_LOAD( "09",         0x18000, 0x1000, CRC(88187b64) SHA1(efacc4a6d4b2af9a236c9d520de6d605c79cc5a8) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "sg.snd",      0xf800, 0x0800, CRC(2fcf6c4d) SHA1(9c4334ac3ff15d94001b22fc367af40f9deb7d57) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder.4",   0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) )
	ROM_LOAD( "decoder.5",   0x0200, 0x0200, CRC(f921c5fe) SHA1(9cebb8bb935315101d248140d1b4503993ebdf8a) )
ROM_END


ROM_START( robotron )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "robotron.sba", 0x0d000, 0x1000, CRC(13797024) SHA1(d426a50e75dabe936de643c83a548da5e399331c) )
	ROM_LOAD( "robotron.sbb", 0x0e000, 0x1000, CRC(7e3c1b87) SHA1(f8c6cbe3688f256f41a121255fc08f575f6a4b4f) )
	ROM_LOAD( "robotron.sbc", 0x0f000, 0x1000, CRC(645d543e) SHA1(fad7cea868ebf17347c4bc5193d647bbd8f9517b) )
	ROM_LOAD( "robotron.sb1", 0x10000, 0x1000, CRC(66c7d3ef) SHA1(f6d60e26c209c1df2cc01ac07ad5559daa1b7118) )
	ROM_LOAD( "robotron.sb2", 0x11000, 0x1000, CRC(5bc6c614) SHA1(4d6e82bc29f49100f7751ccfc6a9ff35695b84b3) )
	ROM_LOAD( "robotron.sb3", 0x12000, 0x1000, CRC(e99a82be) SHA1(06a8c8dd0b4726eb7f0bb0e89c8533931d75fc1c) )
	ROM_LOAD( "robotron.sb4", 0x13000, 0x1000, CRC(afb1c561) SHA1(aaf89c19fd8f4e8750717169eb1af476aef38a5e) )
	ROM_LOAD( "robotron.sb5", 0x14000, 0x1000, CRC(62691e77) SHA1(79b4680ce19bd28882ae823f0e7b293af17cbb91) )
	ROM_LOAD( "robotron.sb6", 0x15000, 0x1000, CRC(bd2c853d) SHA1(f76ec5432a7939b33a27be1c6855e2dbe6d9fdc8) )
	ROM_LOAD( "robotron.sb7", 0x16000, 0x1000, CRC(49ac400c) SHA1(06eae5138254723819a5e93cfd9e9f3285fcddf5) )
	ROM_LOAD( "robotron.sb8", 0x17000, 0x1000, CRC(3a96e88c) SHA1(7ae38a609ed9a6f62ca003cab719740ed7651b7c) )
	ROM_LOAD( "robotron.sb9", 0x18000, 0x1000, CRC(b124367b) SHA1(fd9d75b866f0ebbb723f84889337e6814496a103) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "robotron.snd", 0xf000, 0x1000, CRC(c56c1d28) SHA1(15afefef11bfc3ab78f61ab046701db78d160ec3) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder.4",   0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) )
	ROM_LOAD( "decoder.6",   0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) )
ROM_END

ROM_START( robotronyo )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "robotron.yoa", 0x0d000, 0x1000, CRC(4a9d5f52) SHA1(d5ae801e60ed829e7ef5c54a18aefca54eae827f) )
	ROM_LOAD( "robotron.yob", 0x0e000, 0x1000, CRC(2afc5e7f) SHA1(f3405be9ad2287f3921e7dbd9c5313c91fa7f8d6) )
	ROM_LOAD( "robotron.yoc", 0x0f000, 0x1000, CRC(45da9202) SHA1(81b3b2a72a3c871e8d7b9348056622c90a20d876) )
	ROM_LOAD( "robotron.sb1", 0x10000, 0x1000, CRC(66c7d3ef) SHA1(f6d60e26c209c1df2cc01ac07ad5559daa1b7118) )
	ROM_LOAD( "robotron.sb2", 0x11000, 0x1000, CRC(5bc6c614) SHA1(4d6e82bc29f49100f7751ccfc6a9ff35695b84b3) )
	ROM_LOAD( "robotron.yo3", 0x12000, 0x1000, CRC(67a369bc) SHA1(5a912d485e686de5e3175d3fc0e5daad36f4b836) )
	ROM_LOAD( "robotron.yo4", 0x13000, 0x1000, CRC(b0de677a) SHA1(02013e00513dd74e878a01791cbcca92712e2c80) )
	ROM_LOAD( "robotron.yo5", 0x14000, 0x1000, CRC(24726007) SHA1(8b4ed881f64e3ce73ac1a9ae2c184721c1ab37cc) )
	ROM_LOAD( "robotron.yo6", 0x15000, 0x1000, CRC(028181a6) SHA1(41c4d9ece2ae8a103b7151fc4ff576796303318d) )
	ROM_LOAD( "robotron.yo7", 0x16000, 0x1000, CRC(4dfcceae) SHA1(46fe1b1162d6054eb502852d065fc2e8c694b09d) )
	ROM_LOAD( "robotron.sb8", 0x17000, 0x1000, CRC(3a96e88c) SHA1(7ae38a609ed9a6f62ca003cab719740ed7651b7c) )
	ROM_LOAD( "robotron.sb9", 0x18000, 0x1000, CRC(b124367b) SHA1(fd9d75b866f0ebbb723f84889337e6814496a103) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "robotron.snd", 0xf000, 0x1000, CRC(c56c1d28) SHA1(15afefef11bfc3ab78f61ab046701db78d160ec3) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder.4",   0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) )
	ROM_LOAD( "decoder.6",   0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) )
ROM_END


ROM_START( joust )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "3006-22.10b", 0x0d000, 0x1000, CRC(3f1c4f89) SHA1(90864a8ab944df45287bf0f68ad3a85194077a82) )
	ROM_LOAD( "3006-23.11b", 0x0e000, 0x1000, CRC(ea48b359) SHA1(6d38003d56bebeb1f5b4d2287d587342847aa195) )
	ROM_LOAD( "3006-24.12b", 0x0f000, 0x1000, CRC(c710717b) SHA1(7d01764e8251c60b3cab96f7dc6dcc1c624f9d12) )
	ROM_LOAD( "3006-13.1b",  0x10000, 0x1000, CRC(fe41b2af) SHA1(0443e00ae2eb3e66cf805562ee04309487bb0ba4) )
	ROM_LOAD( "3006-14.2b",  0x11000, 0x1000, CRC(501c143c) SHA1(5fda266d43cbbf42eeae1a078b5209d9408ab99f) )
	ROM_LOAD( "3006-15.3b",  0x12000, 0x1000, CRC(43f7161d) SHA1(686da120aa4bd4a41f3d93e8c79ebb343977851a) )
	ROM_LOAD( "3006-16.4b",  0x13000, 0x1000, CRC(db5571b6) SHA1(cb1c3285344e2cfbe0a81ab9b51758c40da8a23f) )
	ROM_LOAD( "3006-17.5b",  0x14000, 0x1000, CRC(c686bb6b) SHA1(d9cac4c46820e1a451a145864bca7a35cfab7d37) )
	ROM_LOAD( "3006-18.6b",  0x15000, 0x1000, CRC(fac5f2cf) SHA1(febaa8cf5c3a0af901cd12d0b7909f6fec3beadd) )
	ROM_LOAD( "3006-19.7b",  0x16000, 0x1000, CRC(81418240) SHA1(5ad14aa65e71c3856dcdb04c99edda92e406a3e3) )
	ROM_LOAD( "3006-20.8b",  0x17000, 0x1000, CRC(ba5359ba) SHA1(f4ee13d5a95ed3e1050a3927a3a0ccf86ed7752d) )
	ROM_LOAD( "3006-21.9b",  0x18000, 0x1000, CRC(39643147) SHA1(d95d3b746133eac9dcc9ee05eabecb797023f1a5) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "joust.snd",   0xf000, 0x1000, CRC(f1835bdd) SHA1(af7c066d2949d36b87ea8c425ca7d12f82b5c653) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder.4",   0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) )
	ROM_LOAD( "decoder.6",   0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) )
ROM_END

ROM_START( joustwr )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "joust.wra",   0x0d000, 0x1000, CRC(2039014a) SHA1(b9a76ecf01404585f833f76c54aa5a88a0215715) )
	ROM_LOAD( "joust.wgb",   0x0e000, 0x1000, CRC(ea48b359) SHA1(6d38003d56bebeb1f5b4d2287d587342847aa195) )
	ROM_LOAD( "joust.wgc",   0x0f000, 0x1000, CRC(c710717b) SHA1(7d01764e8251c60b3cab96f7dc6dcc1c624f9d12) )
	ROM_LOAD( "joust.wg1",   0x10000, 0x1000, CRC(fe41b2af) SHA1(0443e00ae2eb3e66cf805562ee04309487bb0ba4) )
	ROM_LOAD( "joust.wg2",   0x11000, 0x1000, CRC(501c143c) SHA1(5fda266d43cbbf42eeae1a078b5209d9408ab99f) )
	ROM_LOAD( "joust.wg3",   0x12000, 0x1000, CRC(43f7161d) SHA1(686da120aa4bd4a41f3d93e8c79ebb343977851a) )
	ROM_LOAD( "joust.wg4",   0x13000, 0x1000, CRC(db5571b6) SHA1(cb1c3285344e2cfbe0a81ab9b51758c40da8a23f) )
	ROM_LOAD( "joust.wg5",   0x14000, 0x1000, CRC(c686bb6b) SHA1(d9cac4c46820e1a451a145864bca7a35cfab7d37) )
	ROM_LOAD( "joust.wg6",   0x15000, 0x1000, CRC(fac5f2cf) SHA1(febaa8cf5c3a0af901cd12d0b7909f6fec3beadd) )
	ROM_LOAD( "joust.wr7",   0x16000, 0x1000, CRC(e6f439c4) SHA1(ff8f1d54f3ac91101ab9f5f115baeca4f2670186) )
	ROM_LOAD( "joust.wg8",   0x17000, 0x1000, CRC(ba5359ba) SHA1(f4ee13d5a95ed3e1050a3927a3a0ccf86ed7752d) )
	ROM_LOAD( "joust.wg9",   0x18000, 0x1000, CRC(39643147) SHA1(d95d3b746133eac9dcc9ee05eabecb797023f1a5) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "joust.snd",   0xf000, 0x1000, CRC(f1835bdd) SHA1(af7c066d2949d36b87ea8c425ca7d12f82b5c653) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder.4",   0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) )
	ROM_LOAD( "decoder.6",   0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) )
ROM_END

ROM_START( joustr )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "joust.sra",   0x0d000, 0x1000, CRC(c0c6e52a) SHA1(f14ff16195027f3e199e79e43741f0849c17fd10) )
	ROM_LOAD( "joust.srb",   0x0e000, 0x1000, CRC(ab11bcf9) SHA1(efb09e92a621d6c4d6cde2f166e8c988c64d81ae) )
	ROM_LOAD( "joust.src",   0x0f000, 0x1000, CRC(ea14574b) SHA1(7572d118b2343646054e558f0bd48e4959d84ce7) )
	ROM_LOAD( "joust.wg1",   0x10000, 0x1000, CRC(fe41b2af) SHA1(0443e00ae2eb3e66cf805562ee04309487bb0ba4) )
	ROM_LOAD( "joust.wg2",   0x11000, 0x1000, CRC(501c143c) SHA1(5fda266d43cbbf42eeae1a078b5209d9408ab99f) )
	ROM_LOAD( "joust.wg3",   0x12000, 0x1000, CRC(43f7161d) SHA1(686da120aa4bd4a41f3d93e8c79ebb343977851a) )
	ROM_LOAD( "joust.sr4",   0x13000, 0x1000, CRC(ab347170) SHA1(ad50c83fcfa958f2673cae04bd811095f9ee08c0) )
	ROM_LOAD( "joust.wg5",   0x14000, 0x1000, CRC(c686bb6b) SHA1(d9cac4c46820e1a451a145864bca7a35cfab7d37) )
	ROM_LOAD( "joust.sr6",   0x15000, 0x1000, CRC(3d9a6fac) SHA1(0c81394ae96a2fcfa4c953d38e43f3ef415fe4fc) )
	ROM_LOAD( "joust.sr7",   0x16000, 0x1000, CRC(0a70b3d1) SHA1(eb78b694aa29f777f3c7e7104e568f865930c0ec) )
	ROM_LOAD( "joust.sr8",   0x17000, 0x1000, CRC(a7f01504) SHA1(0ca3211d060befc102bda2e97d163de7fb12a6f6) )
	ROM_LOAD( "joust.sr9",   0x18000, 0x1000, CRC(978687ad) SHA1(25e651af3e3be08d6293aab427a0843e9333a629) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "joust.snd",   0xf000, 0x1000, CRC(f1835bdd) SHA1(af7c066d2949d36b87ea8c425ca7d12f82b5c653) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder.4",   0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) )
	ROM_LOAD( "decoder.6",   0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) )
ROM_END


ROM_START( bubbles )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "bubbles.10b", 0x0d000, 0x1000, CRC(26e7869b) SHA1(db428e79fc325ae3c8cab460267c27cdbc35a3bd) )
	ROM_LOAD( "bubbles.11b", 0x0e000, 0x1000, CRC(5a5b572f) SHA1(f0c3a330abf9c8cfb6007ee372409450d2a15a93) )
	ROM_LOAD( "bubbles.12b", 0x0f000, 0x1000, CRC(ce22d2e2) SHA1(be4b9800c846660ce2b2ddd75ad872dcf174979a) )
	ROM_LOAD( "bubbles.1b",  0x10000, 0x1000, CRC(8234f55c) SHA1(4d60942320c03ae50b0b17267062a321cf49e240) )
	ROM_LOAD( "bubbles.2b",  0x11000, 0x1000, CRC(4a188d6a) SHA1(2788c4a21659799e59ab82bc8d1864a3abe3b6d7) )
	ROM_LOAD( "bubbles.3b",  0x12000, 0x1000, CRC(7728f07f) SHA1(2a2c6dd8c2196dcd5e71b38554a56ee03d2aa454) )
	ROM_LOAD( "bubbles.4b",  0x13000, 0x1000, CRC(040be7f9) SHA1(de4d212cd2967b2dcd7b2c09dea2c1b06ce4c5bd) )
	ROM_LOAD( "bubbles.5b",  0x14000, 0x1000, CRC(0b5f29e0) SHA1(ae52f8c69c8b821abb458288c8ee0bc6c28fe535) )
	ROM_LOAD( "bubbles.6b",  0x15000, 0x1000, CRC(4dd0450d) SHA1(d55aa8fb8f2974ce5ba7155b01bc3e3622f202af) )
	ROM_LOAD( "bubbles.7b",  0x16000, 0x1000, CRC(e0a26ec0) SHA1(2da6213df6c15735a8bbd6750cfb1a1b6232a6f5) )
	ROM_LOAD( "bubbles.8b",  0x17000, 0x1000, CRC(4fd23d8d) SHA1(9d71caa30bc3f4151789279d21651e5a4fe4a484) )
	ROM_LOAD( "bubbles.9b",  0x18000, 0x1000, CRC(b48559fb) SHA1(551a49a12353044dbbf28dba2bd860c2d00c50bd) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "bubbles.snd",  0xf000, 0x1000, CRC(689ce2aa) SHA1(b70d2553f731f9a20ddaf9af2f93b7e9c44d4d99) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder.4",   0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) )
	ROM_LOAD( "decoder.6",   0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) )
ROM_END

ROM_START( bubblesr )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "bubblesr.10b", 0x0d000, 0x1000, CRC(8b396db0) SHA1(88cab59ce7f07dfa15d1485d12ebab96d777ca65) )
	ROM_LOAD( "bubblesr.11b", 0x0e000, 0x1000, CRC(096af43e) SHA1(994e60c1e684ae46ea791b274995d21ff5052e56) )
	ROM_LOAD( "bubblesr.12b", 0x0f000, 0x1000, CRC(5c1244ef) SHA1(25b0f359c28291894381d73f4ba3a2b991a547f0) )
	ROM_LOAD( "bubblesr.1b",  0x10000, 0x1000, CRC(dda4e782) SHA1(ad6825ebc05931942ce1042f18e18e3873083abc) )
	ROM_LOAD( "bubblesr.2b",  0x11000, 0x1000, CRC(3c8fa7f5) SHA1(fd3db6c2abab7000d586ef1a4e425329da292144) )
	ROM_LOAD( "bubblesr.3b",  0x12000, 0x1000, CRC(f869bb9c) SHA1(ce276fc33136a527eefbbf35c2bcf1f0b9858740) )
	ROM_LOAD( "bubblesr.4b",  0x13000, 0x1000, CRC(0c65eaab) SHA1(c622906cbda07421a7024955f3b9e8d173f4b6cb) )
	ROM_LOAD( "bubblesr.5b",  0x14000, 0x1000, CRC(7ece4e13) SHA1(c6ec7145c2d3bf51877c7fb995d9732b09e04cf0) )
	ROM_LOAD( "bubbles.6b",   0x15000, 0x1000, CRC(4dd0450d) SHA1(d55aa8fb8f2974ce5ba7155b01bc3e3622f202af) )
	ROM_LOAD( "bubbles.7b",   0x16000, 0x1000, CRC(e0a26ec0) SHA1(2da6213df6c15735a8bbd6750cfb1a1b6232a6f5) )
	ROM_LOAD( "bubblesr.8b",  0x17000, 0x1000, CRC(598b9bd6) SHA1(993cc3fac58310d0e617e58e3a0753002b987df1) )
	ROM_LOAD( "bubbles.9b",   0x18000, 0x1000, CRC(b48559fb) SHA1(551a49a12353044dbbf28dba2bd860c2d00c50bd) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "bubbles.snd",  0xf000, 0x1000, CRC(689ce2aa) SHA1(b70d2553f731f9a20ddaf9af2f93b7e9c44d4d99) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder.4",   0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) )
	ROM_LOAD( "decoder.6",   0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) )
ROM_END

ROM_START( bubblesp )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "bub_prot.10b", 0x0d000, 0x1000, CRC(89a565df) SHA1(1f02c17222f7303218962fada6c6f867414551cf) )
	ROM_LOAD( "bub_prot.11b", 0x0e000, 0x1000, CRC(5a0c36a7) SHA1(2b9dd9006e57ff8214ad4e6b10a4b72e736d472c) )
	ROM_LOAD( "bub_prot.12b", 0x0f000, 0x1000, CRC(2bfd3438) SHA1(2427a5614e98a9499e4d19f9d6e25f2b73896bf5) )
	ROM_LOAD( "bub_prot.1b",  0x10000, 0x1000, CRC(6466a746) SHA1(ed67d879d82ef05bcd2b655f761f84bc0cf08897) )
	ROM_LOAD( "bub_prot.2b",  0x11000, 0x1000, CRC(cca04357) SHA1(98f879675c02e7ad5532da30f663714913a059b9) )
	ROM_LOAD( "bub_prot.3b",  0x12000, 0x1000, CRC(7aaff9e5) SHA1(8b377ec5c595a4e062bdc8fb8ca99b52a6bd9298) )
	ROM_LOAD( "bub_prot.4b",  0x13000, 0x1000, CRC(4e264f01) SHA1(a6fd2d0613f78c45b3873e06efa2dd99530ed0c8) )
	ROM_LOAD( "bub_prot.5b",  0x14000, 0x1000, CRC(121b0be6) SHA1(75ed718b9e83c32390ee0fe2c34e0300ecd98a85) )
	ROM_LOAD( "bub_prot.6b",  0x15000, 0x1000, CRC(80e90b25) SHA1(92c83b4333f4f0f65638b1827ace01b02c490339) )
	ROM_LOAD( "bubbles.7b",   0x16000, 0x1000, CRC(e0a26ec0) SHA1(2da6213df6c15735a8bbd6750cfb1a1b6232a6f5) )
	ROM_LOAD( "bub_prot.8b",  0x17000, 0x1000, CRC(96fb19c8) SHA1(3b1720e5efe2adc1f633216419bdf00c7e7b817d) )
	ROM_LOAD( "bub_prot.9b",  0x18000, 0x1000, CRC(be7e1028) SHA1(430b33c8d83ee6756a3ef9298792b71066c88326) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "bubbles.snd",  0xf000, 0x1000, CRC(689ce2aa) SHA1(b70d2553f731f9a20ddaf9af2f93b7e9c44d4d99) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder.4",   0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) )
	ROM_LOAD( "decoder.6",   0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) )
ROM_END


ROM_START( splat )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "splat.10",    0x0d000, 0x1000, CRC(d1a1f632) SHA1(de4f5ba2b92c47757dfd2ca810bf8f87338223f7) )
	ROM_LOAD( "splat.11",    0x0e000, 0x1000, CRC(ca8cde95) SHA1(8e12f6d9eaf397646691ec5d02963b32973cb32e) )
	ROM_LOAD( "splat.12",    0x0f000, 0x1000, CRC(5bee3e60) SHA1(b4ee99fb6c353093faf1e088bab82fec66e785bc) )
	ROM_LOAD( "splat.01",    0x10000, 0x1000, CRC(1cf26e48) SHA1(6ba4de6cc7d1359ed450da7bae1000552373f873) )
	ROM_LOAD( "splat.02",    0x11000, 0x1000, CRC(ac0d4276) SHA1(710aba98909d5d63c4b9b08579021f9c026b3111) )
	ROM_LOAD( "splat.03",    0x12000, 0x1000, CRC(74873e59) SHA1(727c9da682fd10353f3969ef02e9f1826d8cb77a) )
	ROM_LOAD( "splat.04",    0x13000, 0x1000, CRC(70a7064e) SHA1(7e6440585462b68b62d6d571d83635bf17149f1a) )
	ROM_LOAD( "splat.05",    0x14000, 0x1000, CRC(c6895221) SHA1(6f88ba8ac72d9301760d6e2512549f70b5373c65) )
	ROM_LOAD( "splat.06",    0x15000, 0x1000, CRC(ea4ab7fd) SHA1(288a361691a7f147ff3346627a10531d613ad017) )
	ROM_LOAD( "splat.07",    0x16000, 0x1000, CRC(82fd8713) SHA1(c4d42b111a0357700ac2bf700117d75ffb3c5be5) )
	ROM_LOAD( "splat.08",    0x17000, 0x1000, CRC(7dded1b4) SHA1(73df546dd60870f63a8c3deffea2b2d13149a48b) )
	ROM_LOAD( "splat.09",    0x18000, 0x1000, CRC(71cbfe5a) SHA1(bf22bedeceffdccc340637098070b32e9c13cf68) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "splat.snd",   0xf000, 0x1000, CRC(a878d5f3) SHA1(f3347a354cb54ca228fe0971f0ae3bc778e2aecf) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder.4",   0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) )
	ROM_LOAD( "decoder.6",   0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) )
ROM_END


ROM_START( sinistar )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "sinistar.10",  0x0e000, 0x1000, CRC(3d670417) SHA1(81802622bee8dbea5c0f08019d87d941dcdbe292) )
	ROM_LOAD( "sinistar.11",  0x0f000, 0x1000, CRC(3162bc50) SHA1(2f38e572ab9c731e38dfe9bad3cc8222a775c5ea) )
	ROM_LOAD( "sinistar.01",  0x10000, 0x1000, CRC(f6f3a22c) SHA1(026d8cab07734fa294a5645edbe65a904bcbc302) )
	ROM_LOAD( "sinistar.02",  0x11000, 0x1000, CRC(cab3185c) SHA1(423d1e3b0c07333ec582529bc4d0b7baf591820a) )
	ROM_LOAD( "sinistar.03",  0x12000, 0x1000, CRC(1ce1b3cc) SHA1(5bc03d7249529d827dc60c087e074ab3e4ea7361) )
	ROM_LOAD( "sinistar.04",  0x13000, 0x1000, CRC(6da632ba) SHA1(72c0c3d5a5ca87ca4d95fcedaf834206e4633950) )
	ROM_LOAD( "sinistar.05",  0x14000, 0x1000, CRC(b662e8fc) SHA1(828a89d2ea13d8a362dae708f86bff54cb231887) )
	ROM_LOAD( "sinistar.06",  0x15000, 0x1000, CRC(2306183d) SHA1(703e29e6446856615760a4897c0f5d79cc7bdfb2) )
	ROM_LOAD( "sinistar.07",  0x16000, 0x1000, CRC(e5dd918e) SHA1(bf4e2ada6a59d246218544d822ba5355da925924) )
	ROM_LOAD( "sinistar.08",  0x17000, 0x1000, CRC(4785a787) SHA1(8c7eca656b2c23b0da41a8c7ce51a2735cab85a4) )
	ROM_LOAD( "sinistar.09",  0x18000, 0x1000, CRC(50cb63ad) SHA1(96e28e4fef98fff2649741a266fa590e0313e3b0) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "speech.ic7",   0xb000, 0x1000, CRC(e1019568) SHA1(442f4f3ccd2e1db2136d2ffb121ea442921f87ca) )
	ROM_LOAD( "speech.ic5",   0xc000, 0x1000, CRC(cf3b5ffd) SHA1(d5d51c550581c9d46ab331dd4fd32541a2ef598e) )
	ROM_LOAD( "speech.ic6",   0xd000, 0x1000, CRC(ff8d2645) SHA1(16fa2a602acbbc182dd96bab113ab18356f3daf0) )
	ROM_LOAD( "speech.ic4",   0xe000, 0x1000, CRC(4b56a626) SHA1(44430cd5c110ec751b0bfb8ae99b26d443350db1) )
	ROM_LOAD( "sinistar.snd", 0xf000, 0x1000, CRC(b82f4ddb) SHA1(c70c7dd6e88897920d7709a260f27810f66aade1) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder.4",   0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) )
	ROM_LOAD( "decoder.6",   0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) )
ROM_END

ROM_START( sinistar1 )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "sinrev1.10",   0x0e000, 0x1000, CRC(ea87a53f) SHA1(4e4bad5315a8f5740c926ee5681879919a5be37f) )
	ROM_LOAD( "sinrev1.11",   0x0f000, 0x1000, CRC(88d36e80) SHA1(bb9adaf5b73f9874e52dc2f5fd35e22f8b4fc258) )
	ROM_LOAD( "sinrev1.01",   0x10000, 0x1000, CRC(3810d7b8) SHA1(dcd690cbc958a2f97f022765315d77fb7c7d8e8b) )
	ROM_LOAD( "sinistar.02",  0x11000, 0x1000, CRC(cab3185c) SHA1(423d1e3b0c07333ec582529bc4d0b7baf591820a) )
	ROM_LOAD( "sinrev1.03",   0x12000, 0x1000, CRC(7c984ca9) SHA1(b32b7d15194051db5d29acf95b049e2eccf6d393) )
	ROM_LOAD( "sinrev1.04",   0x13000, 0x1000, CRC(cc6c4f24) SHA1(b4375544e02a19458c6fcc85edb31025c0b8eb71) )
	ROM_LOAD( "sinrev1.05",   0x14000, 0x1000, CRC(12285bfe) SHA1(6d433103332ddda2f2af23febc0b15aa93db1f31) )
	ROM_LOAD( "sinrev1.06",   0x15000, 0x1000, CRC(7a675f35) SHA1(3a7e9fdb2aef52dc29d33799694737038802b6e0) )
	ROM_LOAD( "sinrev1.07",   0x16000, 0x1000, CRC(b0463243) SHA1(95d597856a1942bd176f5f62db0d691f8f2f2932) )
	ROM_LOAD( "sinrev1.08",   0x17000, 0x1000, CRC(909040d4) SHA1(5361cc378bdace0799227e901341747dce9bb029) )
	ROM_LOAD( "sinrev1.09",   0x18000, 0x1000, CRC(cc949810) SHA1(2d2d1cccd7e43b63e424c34ab5215a412e2b9809) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "speech.ic7",   0xb000, 0x1000, CRC(e1019568) SHA1(442f4f3ccd2e1db2136d2ffb121ea442921f87ca) )
	ROM_LOAD( "speech.ic5",   0xc000, 0x1000, CRC(cf3b5ffd) SHA1(d5d51c550581c9d46ab331dd4fd32541a2ef598e) )
	ROM_LOAD( "speech.ic6",   0xd000, 0x1000, CRC(ff8d2645) SHA1(16fa2a602acbbc182dd96bab113ab18356f3daf0) )
	ROM_LOAD( "speech.ic4",   0xe000, 0x1000, CRC(4b56a626) SHA1(44430cd5c110ec751b0bfb8ae99b26d443350db1) )
	ROM_LOAD( "sinistar.snd", 0xf000, 0x1000, CRC(b82f4ddb) SHA1(c70c7dd6e88897920d7709a260f27810f66aade1) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder.4",   0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) )
	ROM_LOAD( "decoder.6",   0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) )
ROM_END

ROM_START( sinistar2 )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "sinistar.10",  0x0e000, 0x1000, CRC(3d670417) SHA1(81802622bee8dbea5c0f08019d87d941dcdbe292) )
	ROM_LOAD( "sinrev2.11",   0x0f000, 0x1000, CRC(792c8b00) SHA1(1f847ca8a67595927c36d69cead02813c2431c7b) )
	ROM_LOAD( "sinistar.01",  0x10000, 0x1000, CRC(f6f3a22c) SHA1(026d8cab07734fa294a5645edbe65a904bcbc302) )
	ROM_LOAD( "sinistar.02",  0x11000, 0x1000, CRC(cab3185c) SHA1(423d1e3b0c07333ec582529bc4d0b7baf591820a) )
	ROM_LOAD( "sinistar.03",  0x12000, 0x1000, CRC(1ce1b3cc) SHA1(5bc03d7249529d827dc60c087e074ab3e4ea7361) )
	ROM_LOAD( "sinistar.04",  0x13000, 0x1000, CRC(6da632ba) SHA1(72c0c3d5a5ca87ca4d95fcedaf834206e4633950) )
	ROM_LOAD( "sinistar.05",  0x14000, 0x1000, CRC(b662e8fc) SHA1(828a89d2ea13d8a362dae708f86bff54cb231887) )
	ROM_LOAD( "sinistar.06",  0x15000, 0x1000, CRC(2306183d) SHA1(703e29e6446856615760a4897c0f5d79cc7bdfb2) )
	ROM_LOAD( "sinistar.07",  0x16000, 0x1000, CRC(e5dd918e) SHA1(bf4e2ada6a59d246218544d822ba5355da925924) )
	ROM_LOAD( "sinrev2.08",   0x17000, 0x1000, CRC(d7ecee45) SHA1(f9552035409bce0a36ed93a677b28f8cd361f8f1) )
	ROM_LOAD( "sinistar.09",  0x18000, 0x1000, CRC(50cb63ad) SHA1(96e28e4fef98fff2649741a266fa590e0313e3b0) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "speech.ic7",   0xb000, 0x1000, CRC(e1019568) SHA1(442f4f3ccd2e1db2136d2ffb121ea442921f87ca) )
	ROM_LOAD( "speech.ic5",   0xc000, 0x1000, CRC(cf3b5ffd) SHA1(d5d51c550581c9d46ab331dd4fd32541a2ef598e) )
	ROM_LOAD( "speech.ic6",   0xd000, 0x1000, CRC(ff8d2645) SHA1(16fa2a602acbbc182dd96bab113ab18356f3daf0) )
	ROM_LOAD( "speech.ic4",   0xe000, 0x1000, CRC(4b56a626) SHA1(44430cd5c110ec751b0bfb8ae99b26d443350db1) )
	ROM_LOAD( "sinistar.snd", 0xf000, 0x1000, CRC(b82f4ddb) SHA1(c70c7dd6e88897920d7709a260f27810f66aade1) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder.4",   0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) )
	ROM_LOAD( "decoder.6",   0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) )
ROM_END


ROM_START( playball )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "playball.10", 0x0d000, 0x1000, CRC(18787b52) SHA1(621754c1eab68de12763616b7bf01948cdce0221) )
	ROM_LOAD( "playball.11", 0x0e000, 0x1000, CRC(1dd5c8f2) SHA1(17d0380ea05d9ddd17576691d0e5179ae7a71200) )
	ROM_LOAD( "playball.12", 0x0f000, 0x1000, CRC(a700597b) SHA1(5ba07409ae9315b9ee65530f61155c394bfc69ad) )
	ROM_LOAD( "playball.01", 0x10000, 0x1000, CRC(7ba8fd71) SHA1(9b77996238c67aead8c2cfc7f964f8cf9c6182eb) )
	ROM_LOAD( "playball.02", 0x11000, 0x1000, CRC(2387c3d4) SHA1(19d9da6af317595d0f3336e886154e0b8467cb3e) )
	ROM_LOAD( "playball.03", 0x12000, 0x1000, CRC(d34cc5fd) SHA1(d1f6d321c1a6a04a06813c77a3e079836a05956c) )
	ROM_LOAD( "playball.04", 0x13000, 0x1000, CRC(f68c3a8e) SHA1(f9cc7250254b9adceff883d3f6ee01c475d859ec) )
	ROM_LOAD( "playball.05", 0x14000, 0x1000, CRC(a3f20810) SHA1(678d2a5a06263cc5f74f4cb92287cf4d7a8b934f) )
	ROM_LOAD( "playball.06", 0x15000, 0x1000, CRC(f213e48e) SHA1(05b54f5121a887bc24fbe30f322277ae94474c14) )
	ROM_LOAD( "playball.07", 0x16000, 0x1000, CRC(9b5574e9) SHA1(1dddd33cd3f13694d7ba6a73e5090594c6677d5b) )
	ROM_LOAD( "playball.08", 0x17000, 0x1000, CRC(b2d2074a) SHA1(2defb2ffaca782606f792020f9c96d41abd77518) )
	ROM_LOAD( "playball.09", 0x18000, 0x1000, CRC(c4566d0f) SHA1(7848ea87d2d1693ade9129846024fbedc4145cbb) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "speech.ic4",   0xb000, 0x1000, CRC(7e4fc798) SHA1(4636ab25238503370063f51f86f37d0e49c0d3b6) )
	ROM_LOAD( "speech.ic5",   0xc000, 0x1000, CRC(ddfe860c) SHA1(f847a0a6438af5dc646b7abe994530e6d1cbb803) )
	ROM_LOAD( "speech.ic6",   0xd000, 0x1000, CRC(8bfebf87) SHA1(d6829f78e1a2aee85673a42f7f6b78679847b616) )
	ROM_LOAD( "speech.ic7",   0xe000, 0x1000, CRC(db351db6) SHA1(94d807df61b5015f5fa78a500e2a58277db95c1f) )
	ROM_LOAD( "playball.snd", 0xf000, 0x1000, CRC(f3076f9f) SHA1(436fb1a6456535cd27f85c941ff79c0465b71555) )
ROM_END


ROM_START( blaster )
	ROM_REGION( 0x54000, "maincpu", 0 )
	ROM_LOAD( "16.ic39",     0x0d000, 0x1000, CRC(54a40b21) SHA1(663c7b539e6f1f065a4ecae7bb0477c71951223f) )
	ROM_LOAD( "13.ic27",     0x0e000, 0x2000, CRC(f4dae4c8) SHA1(211dcbe085a30419d649afe10ca7c4017d909bd7) )

	ROM_LOAD( "11.ic25",     0x10000, 0x2000, CRC(6371e62f) SHA1(dc4173d2ee88757a6ac0838acaee325eadc2c4fb) )
	ROM_LOAD( "12.ic26",     0x12000, 0x2000, CRC(9804faac) SHA1(e61218fe190ad268af48d611d140d8f4cd38e4c7) )
	ROM_LOAD( "17.ic41",     0x14000, 0x1000, CRC(bf96182f) SHA1(e25a02508eecf79ea1ae5d45278a60becc6c7dcc) )

	ROM_LOAD( "15.ic38",     0x18000, 0x4000, CRC(1ad146a4) SHA1(5ab3d9618023b59bc329a9eeef986901867a639b) )
	ROM_LOAD( "8.ic20",      0x1c000, 0x4000, CRC(f110bbb0) SHA1(314dea232a3706509399348c7415f933c64cea1b) )
	ROM_LOAD( "9.ic22",      0x20000, 0x4000, CRC(5c5b0f8a) SHA1(224f89c85b2b1ca511d006180b8d994fccbdfb6b) )
	ROM_LOAD( "10.ic24",     0x24000, 0x4000, CRC(d47eb67f) SHA1(5dcde8be1a7b1927b90ffab3219dc47c5b2f20e4) )
	ROM_LOAD( "6.ic13",      0x28000, 0x4000, CRC(47fc007e) SHA1(3a80b9b7ae460e9732f7c1cdd465a5b06ded970f) )
	ROM_LOAD( "5.ic11",      0x2c000, 0x4000, CRC(15c1b94d) SHA1(5d97628541eb8933870c3ffd3646b7aaf8af6af5) )
	ROM_LOAD( "14.ic35",     0x30000, 0x4000, CRC(aea6b846) SHA1(04cb4b5eb000471a0cec377a5236ac8c83529528) )
	ROM_LOAD( "7.ic15",      0x34000, 0x4000, CRC(7a101181) SHA1(5f1581911ea7fe3e63ce1b9c50b1d3bf081dbf81) )
	ROM_LOAD( "1.ic1",       0x38000, 0x4000, CRC(8d0ea9e7) SHA1(34f8e2e99748bed29285f7e4929bb920960ab03e) )
	ROM_LOAD( "2.ic3",       0x3c000, 0x4000, CRC(03c4012c) SHA1(53f0adc91e5f1ac58b08b3a6d2de8de5a40bebab) )
	ROM_LOAD( "4.ic7",       0x40000, 0x4000, CRC(fc9d39fb) SHA1(126d43a64471bbf4b40aeda8913d50e82d254f9c) )
	ROM_LOAD( "3.ic6",       0x44000, 0x4000, CRC(253690fb) SHA1(06cb2ef95bb06b3618392e298aa690e1f75bc977) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "18.sb13",      0xf000, 0x1000, CRC(c33a3145) SHA1(6ffe2da7b70c0b576fbc1790a33eecdbb9ee3d02) )
	ROM_REGION( 0x10000, "soundcpu_b", 0 )
	ROM_LOAD( "18.sb10",      0xf000, 0x1000, CRC(c33a3145) SHA1(6ffe2da7b70c0b576fbc1790a33eecdbb9ee3d02) )

	ROM_REGION( 0x0c00, "proms", 0 )        /* color & video-decoder PROM data */
	ROM_LOAD( "4.u42",        0x0800, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) )
	ROM_LOAD( "6.u23",        0x0a00, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) )
	ROM_LOAD( "blaster.col",  0x0000, 0x0800, CRC(bac50bc4) SHA1(80a48eb97c6f02703210d00498f9669c36e64326) )
ROM_END

ROM_START( blastero )
	ROM_REGION( 0x54000, "maincpu", 0 )
	ROM_LOAD( "16.ic39",   0x0d000, 0x1000, CRC(2db032d2) SHA1(287769361639695b1c1ceae0fe6899d83b4575d5) ) // sldh
	ROM_LOAD( "13.ic27",   0x0e000, 0x2000, CRC(c99213c7) SHA1(d1c1549c053de3d862d8ef3ebca02811ed289464) ) // sldh

	ROM_LOAD( "11.ic25",   0x10000, 0x2000, CRC(bc2d7eda) SHA1(831e9ecb75b143f9770eab1939136092a29e64f7) ) // sldh
	ROM_LOAD( "12.ic26",   0x12000, 0x2000, CRC(8a215017) SHA1(ee9233134907c03f7a1221d9daa84fe047c2db94) ) // sldh
	ROM_LOAD( "17.ic41",   0x14000, 0x1000, CRC(b308f0e5) SHA1(262e25be40dff66e65a0fe34c9d013a750b90876) ) // sldh

	ROM_LOAD( "15.ic38",   0x18000, 0x4000, CRC(1ad146a4) SHA1(5ab3d9618023b59bc329a9eeef986901867a639b) )
	ROM_LOAD( "8.ic20",    0x1c000, 0x4000, CRC(f110bbb0) SHA1(314dea232a3706509399348c7415f933c64cea1b) )
	ROM_LOAD( "9.ic22",    0x20000, 0x4000, CRC(5c5b0f8a) SHA1(224f89c85b2b1ca511d006180b8d994fccbdfb6b) )
	ROM_LOAD( "10.ic24",   0x24000, 0x4000, CRC(d47eb67f) SHA1(5dcde8be1a7b1927b90ffab3219dc47c5b2f20e4) )
	ROM_LOAD( "6.ic13",    0x28000, 0x4000, CRC(47fc007e) SHA1(3a80b9b7ae460e9732f7c1cdd465a5b06ded970f) )
	ROM_LOAD( "5.ic11",    0x2c000, 0x4000, CRC(15c1b94d) SHA1(5d97628541eb8933870c3ffd3646b7aaf8af6af5) )
	ROM_LOAD( "14.ic35",   0x30000, 0x4000, CRC(aea6b846) SHA1(04cb4b5eb000471a0cec377a5236ac8c83529528) )
	ROM_LOAD( "7.ic15",    0x34000, 0x4000, CRC(a1c4db77) SHA1(7a878d44b6ca7444ecbb6c8f75e5e91de149daf3) ) // sldh
	ROM_LOAD( "1.ic1",     0x38000, 0x4000, CRC(8d0ea9e7) SHA1(34f8e2e99748bed29285f7e4929bb920960ab03e) )
	ROM_LOAD( "2.ic3",     0x3c000, 0x4000, CRC(03c4012c) SHA1(53f0adc91e5f1ac58b08b3a6d2de8de5a40bebab) )
	ROM_LOAD( "4.ic7",     0x40000, 0x4000, CRC(39d2a32c) SHA1(33707877e841ef86a11b47ffabddce7f3d2a7030) ) // sldh
	ROM_LOAD( "3.ic6",     0x44000, 0x4000, CRC(054c9f1c) SHA1(c21e3493f1ae506ab9fd28ed9ecc67d3305e9d7a) ) // sldh

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "18.sb13",      0xf000, 0x1000, CRC(c33a3145) SHA1(6ffe2da7b70c0b576fbc1790a33eecdbb9ee3d02) )
	ROM_REGION( 0x10000, "soundcpu_b", 0 )
	ROM_LOAD( "18.sb10",      0xf000, 0x1000, CRC(c33a3145) SHA1(6ffe2da7b70c0b576fbc1790a33eecdbb9ee3d02) )

	ROM_REGION( 0x0c00, "proms", 0 )        /* color & video-decoder PROM data */
	ROM_LOAD( "4.u42",        0x0800, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) )
	ROM_LOAD( "6.u23",        0x0a00, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) )
	ROM_LOAD( "blaster.col",  0x0000, 0x0800, CRC(bac50bc4) SHA1(80a48eb97c6f02703210d00498f9669c36e64326) )
ROM_END

ROM_START( blasterkit )
	ROM_REGION( 0x54000, "maincpu", 0 )
	ROM_LOAD( "blastkit.16",  0x0d000, 0x1000, CRC(414b2abf) SHA1(2bde972d225d6e93e44751f542cee584d57f7983) )
	ROM_LOAD( "blastkit.13",  0x0e000, 0x2000, CRC(9c64db76) SHA1(c14508cb2f964af93631779db3adaa960fcc7559) )

	ROM_LOAD( "blastkit.11",  0x10000, 0x2000, CRC(b7df4914) SHA1(81f7a89dfde06c160f2c8974eec701f2298ec434) )
	ROM_LOAD( "blastkit.12",  0x12000, 0x2000, CRC(8b1e26ab) SHA1(7d30800a9302f5a83792499d8df536693d01f75d) )
	ROM_LOAD( "blastkit.17",  0x14000, 0x1000, CRC(577d1e9a) SHA1(0064124a65490e0473dfb0081ec28b7ee43a04b5) )

	ROM_LOAD( "blastkit.15",  0x18000, 0x4000, CRC(1ad146a4) SHA1(5ab3d9618023b59bc329a9eeef986901867a639b) )
	ROM_LOAD( "blastkit.8",   0x1c000, 0x4000, CRC(f110bbb0) SHA1(314dea232a3706509399348c7415f933c64cea1b) )
	ROM_LOAD( "blastkit.9",   0x20000, 0x4000, CRC(5c5b0f8a) SHA1(224f89c85b2b1ca511d006180b8d994fccbdfb6b) )
	ROM_LOAD( "blastkit.10",  0x24000, 0x4000, CRC(d47eb67f) SHA1(5dcde8be1a7b1927b90ffab3219dc47c5b2f20e4) )
	ROM_LOAD( "blastkit.6",   0x28000, 0x4000, CRC(47fc007e) SHA1(3a80b9b7ae460e9732f7c1cdd465a5b06ded970f) )
	ROM_LOAD( "blastkit.5",   0x2c000, 0x4000, CRC(15c1b94d) SHA1(5d97628541eb8933870c3ffd3646b7aaf8af6af5) )
	ROM_LOAD( "blastkit.14",  0x30000, 0x4000, CRC(aea6b846) SHA1(04cb4b5eb000471a0cec377a5236ac8c83529528) )
	ROM_LOAD( "blastkit.7",   0x34000, 0x4000, CRC(6fcc2153) SHA1(00e7b6846c15400315d94e2c7d1c99b1a737c285) )
	ROM_LOAD( "blastkit.1",   0x38000, 0x4000, CRC(8d0ea9e7) SHA1(34f8e2e99748bed29285f7e4929bb920960ab03e) )
	ROM_LOAD( "blastkit.2",   0x3c000, 0x4000, CRC(03c4012c) SHA1(53f0adc91e5f1ac58b08b3a6d2de8de5a40bebab) )
	ROM_LOAD( "blastkit.4",   0x40000, 0x4000, CRC(f80e9ff5) SHA1(e232d96b6e07c7b4240fa4dd2cb9be4745a1be4b) )
	ROM_LOAD( "blastkit.3",   0x44000, 0x4000, CRC(20e851f9) SHA1(efc288ef0333812a6282f22aade8e43e9a827533) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "blastkit.18", 0xf000, 0x1000, CRC(c33a3145) SHA1(6ffe2da7b70c0b576fbc1790a33eecdbb9ee3d02) )

	ROM_REGION( 0x0c00, "proms", 0 )        /* color & video-decoder PROM data */
	ROM_LOAD( "4.u42",        0x0800, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) )
	ROM_LOAD( "6.u23",        0x0a00, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) )
	ROM_LOAD( "blaster.col",  0x0000, 0x0800, CRC(bac50bc4) SHA1(80a48eb97c6f02703210d00498f9669c36e64326) )
ROM_END


ROM_START( spdball )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "speedbal.10", 0x0d000, 0x1000, CRC(4a3add93) SHA1(6939dd6cb6751a0406f364223029eff99040f9e2) )
	ROM_LOAD( "speedbal.11", 0x0e000, 0x1000, CRC(1fbcfaa5) SHA1(fccdebbab172b141bbaec6f520b378d21c72f67a) )
	ROM_LOAD( "speedbal.12", 0x0f000, 0x1000, CRC(f3458f41) SHA1(366fb880b4dc68849d6ea7a9dab55efa9c566123) )
	ROM_LOAD( "speedbal.01", 0x10000, 0x1000, CRC(7f4801bb) SHA1(8f22396170571189b1d088d73331d6a713c76f41) )
	ROM_LOAD( "speedbal.02", 0x11000, 0x1000, CRC(5cd5e489) SHA1(83c1bce945ecbaa4a59e0023198e574d9069680c) )
	ROM_LOAD( "speedbal.03", 0x12000, 0x1000, CRC(280e11a4) SHA1(4ef321e1744955a9a54c1e4b1f88c01c01e7b7c8) )
	ROM_LOAD( "speedbal.04", 0x13000, 0x1000, CRC(3469cbbf) SHA1(70b46cf686438441484ffeca0fa1398c15c8811e) )
	ROM_LOAD( "speedbal.05", 0x14000, 0x1000, CRC(87373c89) SHA1(a3cd72f4b517d5d727059a7d911b79ced27e9f93) )
	ROM_LOAD( "speedbal.06", 0x15000, 0x1000, CRC(48779a0d) SHA1(9cdfc12d1021b5d66acd38ab61f385219be39f4f) )
	ROM_LOAD( "speedbal.07", 0x16000, 0x1000, CRC(2e5d8db6) SHA1(7a13d60267ce12a6a4b20322c2ed1f39762bc663) )
	ROM_LOAD( "speedbal.08", 0x17000, 0x1000, CRC(c173cedf) SHA1(603c4c7cdc712d54a86b59470651d00b369293d8) )
	ROM_LOAD( "speedbal.09", 0x18000, 0x1000, CRC(415f424b) SHA1(f7e59385a67319ba152488762af1b42fc62ab264) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "speedbal.snd", 0xf000, 0x1000, CRC(78de20e2) SHA1(ece6e04b1d57167faf7aaee0829e7c31eb560437) )

	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD( "mystery.rom", 0x00000, 0x1000, CRC(dcb6a070) SHA1(6a6fcddf5b46eef187dcf5d9b60e03e9375e7276) )
ROM_END


ROM_START( alienar )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "aarom10",   0x0d000, 0x1000, CRC(6feb0314) SHA1(5cf30f097bc695cbd388cb408e78394926362a7b) )
	ROM_LOAD( "aarom11",   0x0e000, 0x1000, CRC(ae3a270e) SHA1(867fff32062bc876390e8ca6bd7cedae47cd92c9) )
	ROM_LOAD( "aarom12",   0x0f000, 0x1000, CRC(6be9f09e) SHA1(98821c9b94301c5fd6e7f5d9e4bc9c1bdbab53ec) )
	ROM_LOAD( "aarom01",   0x10000, 0x1000, CRC(bb0c21be) SHA1(dbf122870adaa49cd99e2c1e9fa4b78fb74ef2c1) )
	ROM_LOAD( "aarom02",   0x11000, 0x1000, CRC(165acd37) SHA1(12466c94bcf5a98f154a639ecc2e95d5193cbab2) )
	ROM_LOAD( "aarom03",   0x12000, 0x1000, CRC(e5d51d92) SHA1(598c928499e977a30906319c97ffa1ef2b9395d1) )
	ROM_LOAD( "aarom04",   0x13000, 0x1000, CRC(24f6feb8) SHA1(c1b7d764785b4edfe80a90ffdc52a67c8dbbfea5) )
	ROM_LOAD( "aarom05",   0x14000, 0x1000, CRC(5b1ac59b) SHA1(9b312eb419e994a006fda2ae61c58c31f048bace) )
	ROM_LOAD( "aarom06",   0x15000, 0x1000, CRC(da7195a2) SHA1(ef2c2750c504176fd6a11e8463278d97cac9a5c5) )
	ROM_LOAD( "aarom07",   0x16000, 0x1000, CRC(f9812be4) SHA1(5f116345f09cd79790612672aa48ede63fc91f56) )
	ROM_LOAD( "aarom08",   0x17000, 0x1000, CRC(cd7f3a87) SHA1(59577059308931139ecc036d06953660a148d91c) )
	ROM_LOAD( "aarom09",   0x18000, 0x1000, CRC(e6ce77b4) SHA1(bd4354100067654d0ad2e590591582dbdfb845b6) )

	ROM_REGION( 0x10000, "soundcpu", ROMREGION_ERASE00 )
ROM_END


ROM_START( alienaru )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "aarom10",   0x0d000, 0x1000, CRC(6feb0314) SHA1(5cf30f097bc695cbd388cb408e78394926362a7b) )
	ROM_LOAD( "aarom11",   0x0e000, 0x1000, CRC(ae3a270e) SHA1(867fff32062bc876390e8ca6bd7cedae47cd92c9) )
	ROM_LOAD( "aarom12",   0x0f000, 0x1000, CRC(6be9f09e) SHA1(98821c9b94301c5fd6e7f5d9e4bc9c1bdbab53ec) )
	ROM_LOAD( "aarom01",   0x10000, 0x1000, CRC(bb0c21be) SHA1(dbf122870adaa49cd99e2c1e9fa4b78fb74ef2c1) )
	ROM_LOAD( "aarom02",   0x11000, 0x1000, CRC(165acd37) SHA1(12466c94bcf5a98f154a639ecc2e95d5193cbab2) )
	ROM_LOAD( "aarom03",   0x12000, 0x1000, CRC(e5d51d92) SHA1(598c928499e977a30906319c97ffa1ef2b9395d1) )
	ROM_LOAD( "aarom04",   0x13000, 0x1000, CRC(24f6feb8) SHA1(c1b7d764785b4edfe80a90ffdc52a67c8dbbfea5) )
	ROM_LOAD( "aarom05",   0x14000, 0x1000, CRC(5b1ac59b) SHA1(9b312eb419e994a006fda2ae61c58c31f048bace) )
	ROM_LOAD( "aarom06",   0x15000, 0x1000, CRC(da7195a2) SHA1(ef2c2750c504176fd6a11e8463278d97cac9a5c5) )
	ROM_LOAD( "aarom07",   0x16000, 0x1000, CRC(f9812be4) SHA1(5f116345f09cd79790612672aa48ede63fc91f56) )
	ROM_LOAD( "aarom08",   0x17000, 0x1000, CRC(cd7f3a87) SHA1(59577059308931139ecc036d06953660a148d91c) )
	ROM_LOAD( "aarom09",   0x18000, 0x1000, CRC(e6ce77b4) SHA1(bd4354100067654d0ad2e590591582dbdfb845b6) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "sg.snd",    0xf800, 0x0800, CRC(2fcf6c4d) SHA1(9c4334ac3ff15d94001b22fc367af40f9deb7d57) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "decoder.4", 0x0000, 0x0200, CRC(e6631c23) SHA1(9988723269367fb44ef83f627186a1c88cf7877e) )
	ROM_LOAD( "decoder.6", 0x0200, 0x0200, CRC(83faf25e) SHA1(30002643d08ed983a6701a7c4b5ee74a2f4a1adb) )
ROM_END


ROM_START( lottofun )
	ROM_REGION( 0x19000, "maincpu", 0 )
	ROM_LOAD( "vl7a.dat",    0x0d000, 0x1000, CRC(fb2aec2c) SHA1(73dc6a6dfe9ba51e3612b6d912bd7af1d5782296) )
	ROM_LOAD( "vl7c.dat",    0x0e000, 0x1000, CRC(9a496519) SHA1(ae98dadcb63a33c796a3e3083d4b5bc957873cbc) )
	ROM_LOAD( "vl7e.dat",    0x0f000, 0x1000, CRC(032cab4b) SHA1(87bdd0fd58b12e39efaadcf6e82744886a9292e9) )
	ROM_LOAD( "vl4e.dat",    0x10000, 0x1000, CRC(5e9af236) SHA1(6f26c9be6da6f1195a4569f003a010d3f2e0c24d) )
	ROM_LOAD( "vl4c.dat",    0x11000, 0x1000, CRC(4b134ae2) SHA1(86756e1d8de113571857818a98d347789c003339) )
	ROM_LOAD( "vl4a.dat",    0x12000, 0x1000, CRC(b2f1f95a) SHA1(89166cdf4aff5e5a8cc4ea6ba589ce095de82f57) )
	ROM_LOAD( "vl5e.dat",    0x13000, 0x1000, CRC(c8681c55) SHA1(ac63e53a958f63bd0a05f36303c1aa777aee799d) )
	ROM_LOAD( "vl5c.dat",    0x14000, 0x1000, CRC(eb9351e0) SHA1(c66477ca0b3ed95708eb478fb992833beda1a4f8) )
	ROM_LOAD( "vl5a.dat",    0x15000, 0x1000, CRC(534f2fa1) SHA1(c034aa037ef6bc7cd2ed85da7531fd8efb7083e4) )
	ROM_LOAD( "vl6e.dat",    0x16000, 0x1000, CRC(befac592) SHA1(548cb1f0bc178eeada144c443545f7545c90b6a6) )
	ROM_LOAD( "vl6c.dat",    0x17000, 0x1000, CRC(a73d7f13) SHA1(833ff14c33635b61e1bd45b2878a4f6c9e18bf82) )
	ROM_LOAD( "vl6a.dat",    0x18000, 0x1000, CRC(5730a43d) SHA1(8acadf105dc373bf2b3087ccc1667b872452c913) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "vl2532.snd",   0xf000, 0x1000, CRC(214b8a04) SHA1(45f06b44a605cca6b293b20cfea4763b469254b8) )
ROM_END


ROM_START( mysticm )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "mm02_2.a09", 0x0e000, 0x1000, CRC(3a776ea8) SHA1(1fef5f5cef5e10606c97ac9c365f000a88d51314) ) /* IC9  */
	ROM_LOAD( "mm03_2.a10", 0x0f000, 0x1000, CRC(6e247c75) SHA1(4daf5206d29b887cd1a78528fac4b0cd8ec7f39b) ) /* IC10 */

	ROM_LOAD( "mm11_1.a18", 0x10000, 0x2000, CRC(f537968e) SHA1(2660a480d0bba5fe25885453115ef1015f8bdea9) ) /* IC18 */
	ROM_LOAD( "mm09_1.a16", 0x12000, 0x2000, CRC(3bd12f6c) SHA1(7925a92c486c994e8f34c8ed52bf81a34cf44f68) ) /* IC16 */
	ROM_LOAD( "mm07_1.a14", 0x14000, 0x2000, CRC(ea2a2a68) SHA1(71855c874cd5032f47fafc67e2d1667f956cd9b5) ) /* IC14 */
	ROM_LOAD( "mm05_1.a12", 0x16000, 0x2000, CRC(b514eef3) SHA1(0f9309768c416dd98e9c02121cc750993a2923ea) ) /* IC12 */

	ROM_LOAD( "mm18_1.a26", 0x20000, 0x2000, CRC(9b391a81) SHA1(b3f34e5d468fe4a4de2d4e771e2fa08de6596f26) ) /* IC26 */
	ROM_LOAD( "mm16_1.a24", 0x22000, 0x2000, CRC(399e175d) SHA1(e17301e4159e5a6d83c3ca62c93eb70f34b948df) ) /* IC24 */
	ROM_LOAD( "mm14_1.a22", 0x24000, 0x2000, CRC(191153b1) SHA1(fcd8aa6ad6506ba51a01f777f6a3b94e9c051b1c) ) /* IC22 */

	ROM_LOAD( "mm10_1.a17", 0x30000, 0x2000, CRC(d6a37509) SHA1(4b1f52954ca208ccc040c017873777fbf7fbd1f2) ) /* IC17 */
	ROM_LOAD( "mm08_1.a15", 0x32000, 0x2000, CRC(6f1a64f2) SHA1(4183b658b257d7fe35e1d7271f76d3358df5a7a2) ) /* IC15 */
	ROM_LOAD( "mm06_1.a13", 0x34000, 0x2000, CRC(2e6795d4) SHA1(8b074f6a7a4b5a9705de498684180815581faea2) ) /* IC13 */
	ROM_LOAD( "mm04_1.a11", 0x36000, 0x2000, CRC(c222fb64) SHA1(b4c51d2b1664ef3267df1dee9e4888acf847c286) ) /* IC11 */

	ROM_LOAD( "mm17_1.a25", 0x40000, 0x2000, CRC(d36f0a96) SHA1(9830955ca7e46b5b0dba98b4d2ea325bbbebe3c7) ) /* IC25 */
	ROM_LOAD( "mm15_1.a23", 0x42000, 0x2000, CRC(cd5d99da) SHA1(41a37903503c14fb9c801c51afa2f97c83b79f8b) ) /* IC23 */
	ROM_LOAD( "mm13_1.a21", 0x44000, 0x2000, CRC(ef4b79db) SHA1(346057cb8c4593df44fb36771553e60610fe1a0c) ) /* IC21 */
	ROM_LOAD( "mm12_1.a19", 0x46000, 0x2000, CRC(a1f04bf0) SHA1(389bdb7c9e395af9275abfb20c3ab51bc12dc4db) ) /* IC19 */

	/* sound CPU */
	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "mm01_1.a08", 0x0e000, 0x2000, CRC(65339512) SHA1(144625d2905c953383bcc90cd2435d332394883f) ) /* IC8  */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "mm20_1.b57", 0x00000, 0x2000, CRC(5c0f4f46) SHA1(7dedbbeda2f34a2eac9fb14277874d9d66f468c7) ) /* IC57 */
	ROM_LOAD( "mm21_1.b58", 0x02000, 0x2000, CRC(cb90b3c5) SHA1(f28cca2c3ff23d6c9e2952a1b08ab2875655ec70) ) /* IC58 */
	ROM_LOAD( "mm19_1.b41", 0x04000, 0x2000, CRC(e274df86) SHA1(9876a487c5efa350ced31acbc39df22c8d414677) ) /* IC41 */
ROM_END

ROM_START( mysticmp )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "cpu_2732_ic9_rom2_proto6.d4",    0x0e000, 0x1000, CRC(3e4f53dd) SHA1(ebe36af7367b7f1037f5d7d55817e5580db6f10f) ) /* ic9  */ // different
	ROM_LOAD( "cpu_2732_ic10_rom3_proto6.f4",   0x0f000, 0x1000, CRC(6a25ee4b) SHA1(0668a0f3d6ddcf413d8b1f4f8f5b9a2dc9c4edc1) ) /* ic10 */ // different

	ROM_LOAD( "cpu_2764_ic18_rom11_proto5.j8",  0x10000, 0x2000, CRC(f537968e) SHA1(2660a480d0bba5fe25885453115ef1015f8bdea9) ) /* ic18 */
	ROM_LOAD( "cpu_2764_ic16_rom9_proto5.h8",   0x12000, 0x2000, CRC(3bd12f6c) SHA1(7925a92c486c994e8f34c8ed52bf81a34cf44f68) ) /* ic16 */
	ROM_LOAD( "cpu_2764_ic14_rom7_proto5.j6",   0x14000, 0x2000, CRC(ea2a2a68) SHA1(71855c874cd5032f47fafc67e2d1667f956cd9b5) ) /* ic14 */
	ROM_LOAD( "cpu_2764_ic12_rom5_proto5.h6",   0x16000, 0x2000, CRC(b514eef3) SHA1(0f9309768c416dd98e9c02121cc750993a2923ea) ) /* ic12 */

	ROM_LOAD( "cpu_2764_ic26_rom18_proto5.j10", 0x20000, 0x2000, CRC(9b391a81) SHA1(b3f34e5d468fe4a4de2d4e771e2fa08de6596f26) ) /* ic26 */
	ROM_LOAD( "cpu_2764_ic24_rom16_proto5.h10", 0x22000, 0x2000, CRC(399e175d) SHA1(e17301e4159e5a6d83c3ca62c93eb70f34b948df) ) /* ic24 */
	ROM_LOAD( "cpu_2764_ic22_rom14_proto5.j9",  0x24000, 0x2000, CRC(191153b1) SHA1(fcd8aa6ad6506ba51a01f777f6a3b94e9c051b1c) ) /* ic22 */

	ROM_LOAD( "cpu_2764_ic17_rom10_proto4.i8",  0x30000, 0x2000, CRC(d6a37509) SHA1(4b1f52954ca208ccc040c017873777fbf7fbd1f2) ) /* ic17 */
	ROM_LOAD( "cpu_2764_ic15_rom8_proto4.g8",   0x32000, 0x2000, CRC(6f1a64f2) SHA1(4183b658b257d7fe35e1d7271f76d3358df5a7a2) ) /* ic15 */
	ROM_LOAD( "cpu_2764_ic13_rom6_proto4.i6",   0x34000, 0x2000, CRC(2e6795d4) SHA1(8b074f6a7a4b5a9705de498684180815581faea2) ) /* ic13 */
	ROM_LOAD( "cpu_2764_ic11_rom4_proto4.g6",   0x36000, 0x2000, CRC(c222fb64) SHA1(b4c51d2b1664ef3267df1dee9e4888acf847c286) ) /* ic11 */

	ROM_LOAD( "cpu_2764_ic25_rom17_proto6.i10", 0x40000, 0x2000, CRC(7acc9995) SHA1(a996cbd65cf7efd1cdf9b5750b5c743c5edda4dd) ) /* ic25 */ // different
	ROM_LOAD( "cpu_2764_ic23_rom15_proto6.g10", 0x42000, 0x2000, CRC(c32d1ce5) SHA1(0df3eafbb558699382eb729a3059e99305e2e8c8) ) /* ic23 */ // different
	ROM_LOAD( "cpu_2764_ic21_rom13_proto6.i9",  0x44000, 0x2000, CRC(e387a785) SHA1(de98d503f4d2c947c701ff96628114b34da45f93) ) /* ic21 */ // different
	ROM_LOAD( "cpu_2764_ic19_rom12_proto6.g9",  0x46000, 0x2000, CRC(a1f04bf0) SHA1(389bdb7c9e395af9275abfb20c3ab51bc12dc4db) ) /* ic19 */

	/* sound cpu */
	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "cpu_2764_ic8_rom1_proto4.f0", 0x0e000, 0x2000, CRC(65339512) SHA1(144625d2905c953383bcc90cd2435d332394883f) )    /* ic8  */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "ram_2764_ic57_rom20_rev1.f9",  0x00000, 0x2000, CRC(5c0f4f46) SHA1(7dedbbeda2f34a2eac9fb14277874d9d66f468c7) )   /* ic57 */
	ROM_LOAD( "ram_2764_ic58_rom21_rev1.f10", 0x02000, 0x2000, CRC(cb90b3c5) SHA1(f28cca2c3ff23d6c9e2952a1b08ab2875655ec70) )   /* ic58 */
	ROM_LOAD( "ram_2764_ic41_rom19_rev1.d10", 0x04000, 0x2000, CRC(e274df86) SHA1(9876a487c5efa350ced31acbc39df22c8d414677) )   /* ic41 */
ROM_END




ROM_START( tshoot )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "rom18.cpu", 0x0d000, 0x1000, CRC(effc33f1) SHA1(cd1b16b4a4a46ce9d550d10b465b8cf1ab3c5273) )  /* IC55 */
	ROM_LOAD( "rom2.cpu",  0x0e000, 0x1000, CRC(fd982687) SHA1(70be1ea57ea0a1e75b1bd988492a9c0244e8b91f) )  /* IC9  */
	ROM_LOAD( "rom3.cpu",  0x0f000, 0x1000, CRC(9617054d) SHA1(8795b97a6391aa3804f68dc2d2b33866dc17f34c) )  /* IC10 */

	ROM_LOAD( "rom11.cpu", 0x10000, 0x2000, CRC(60d5fab8) SHA1(fe75e46dedb7ca153470d6a39cea0a721e5b7b39) )  /* IC18 */
	ROM_LOAD( "rom9.cpu",  0x12000, 0x2000, CRC(a4dd4a0e) SHA1(bb2f38c5ef2f3398b6ba605ffa0c30c89387bf14) )  /* IC16 */
	ROM_LOAD( "rom7.cpu",  0x14000, 0x2000, CRC(f25505e6) SHA1(d075ff89b6379ad7a47d9723ed1c21468b9d1dae) )  /* IC14 */
	ROM_LOAD( "rom5.cpu",  0x16000, 0x2000, CRC(94a7c0ed) SHA1(11f46e1ca7d79b4244ea0f60e0fba44186f1ebde) )  /* IC12 */

	ROM_LOAD( "rom17.cpu", 0x20000, 0x2000, CRC(b02d1ccd) SHA1(b08b6d9affb6f3e50a11fd9397fe4255927de3b6) )  /* IC26 */
	ROM_LOAD( "rom15.cpu", 0x22000, 0x2000, CRC(11709935) SHA1(ae25bbadbbcab9f3cba2bb4bb92d5217705b38e3) )  /* IC24 */

	ROM_LOAD( "rom10.cpu", 0x30000, 0x2000, CRC(0f32bad8) SHA1(7a2f559697d252ceec3a2f55fe51bc755e4bb65a) )  /* IC17 */
	ROM_LOAD( "rom8.cpu",  0x32000, 0x2000, CRC(e9b6cbf7) SHA1(6cd6b1e1c5e8e253e779afff8ad1ff90d6116fc9) )  /* IC15 */
	ROM_LOAD( "rom6.cpu",  0x34000, 0x2000, CRC(a49f617f) SHA1(759d25e33a09204664880329b86724805a1fe0e8) )  /* IC13 */
	ROM_LOAD( "rom4.cpu",  0x36000, 0x2000, CRC(b026dc00) SHA1(8a068997aa19e152d64db47528893046d338389c) )  /* IC11 */

	ROM_LOAD( "rom16.cpu", 0x40000, 0x2000, CRC(69ce38f8) SHA1(a2cd678e71bfa5e6a3594d8699660c7fa8b52001) )  /* IC25 */
	ROM_LOAD( "rom14.cpu", 0x42000, 0x2000, CRC(769a4ae5) SHA1(1cdfae2d889848d69f68f990714d027cfbca1853) )  /* IC23 */
	ROM_LOAD( "rom13.cpu", 0x44000, 0x2000, CRC(ec016c9b) SHA1(f2e40abd14b8b4944b792dd453ebe92eb64355ae) )  /* IC21 */
	ROM_LOAD( "rom12.cpu", 0x46000, 0x2000, CRC(98ae7afa) SHA1(6a904408419f576352bd2f895727fd17c0541ff8) )  /* IC19 */

	/* sound CPU */
	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "rom1.cpu", 0xe000, 0x2000, CRC(011a94a7) SHA1(9f54a742a87ba56b9517e33e556f57dce6eb2eab) )    /* IC8  */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "rom20.cpu", 0x00000, 0x2000, CRC(c6e1d253) SHA1(c408a29f75ba2958e229996f903400b3d95e3bd3) )  /* IC57 */
	ROM_LOAD( "rom21.cpu", 0x02000, 0x2000, CRC(9874e90f) SHA1(85282823cc862341adf9642d2d5d05972da6dff0) )  /* IC58 */
	ROM_LOAD( "rom19.cpu", 0x04000, 0x2000, CRC(b9ce4d2a) SHA1(af5332f340d3c3ae02e77923d6e8f0dd92547728) )  /* IC41 */
ROM_END


ROM_START( inferno )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "ic9.inf",  0x0e000, 0x1000, CRC(1a013185) SHA1(9079c082ec043714f9d8ea92bc81d0b93d2ce715) )   /* IC9  */
	ROM_LOAD( "ic10.inf", 0x0f000, 0x1000, CRC(dbf64a36) SHA1(54326bc527797f0a3a55764073eb40030aec1aae) )   /* IC10 */

	ROM_LOAD( "ic18.inf", 0x10000, 0x2000, CRC(95bcf7b1) SHA1(66687a3962109a25e26ae00bddd33ed973981b91) )   /* IC18 */
	ROM_LOAD( "ic16.inf", 0x12000, 0x2000, CRC(8bc4f935) SHA1(12da6faa71e5984047fa14f32af5bb865f228cb2) )   /* IC16 */
	ROM_LOAD( "ic14.inf", 0x14000, 0x2000, CRC(a70508a7) SHA1(930bb9af3b6ba9fdf3e7c32f6b5ffae9acd6cee3) )   /* IC14 */
	ROM_LOAD( "ic12.inf", 0x16000, 0x2000, CRC(7ffb87f9) SHA1(469f5ae39ad8531c4c11e9d10ab57686e7f54aef) )   /* IC12 */

	ROM_LOAD( "ic17.inf", 0x30000, 0x2000, CRC(b4684139) SHA1(c1d6ecd3dc8191250ef70e6972dad234c0d8f739) )   /* IC17 */
	ROM_LOAD( "ic15.inf", 0x32000, 0x2000, CRC(128a6ad6) SHA1(357438e50663d6cb96dabfa5110c17836584e15f) )   /* IC15 */
	ROM_LOAD( "ic13.inf", 0x34000, 0x2000, CRC(83a9e4d6) SHA1(4937e4d1c516da837213e40a1da862578c8dd272) )   /* IC13 */
	ROM_LOAD( "ic11.inf", 0x36000, 0x2000, CRC(c2e9c909) SHA1(21f0b9bf6ef3a9466ea9afde1c7efde9ed04ce5b) )   /* IC11 */

	ROM_LOAD( "ic25.inf", 0x40000, 0x2000, CRC(103a5951) SHA1(57c8caa1e9d5e245052822d08add9343fd622e04) )   /* IC25 */
	ROM_LOAD( "ic23.inf", 0x42000, 0x2000, CRC(c04749a0) SHA1(b203e8d1df556e10b4ecad4733448f889c63e261) )   /* IC23 */
	ROM_LOAD( "ic21.inf", 0x44000, 0x2000, CRC(c405f853) SHA1(6bd74d065a6043849e083c2822925b82c6fedb00) )   /* IC21 */
	ROM_LOAD( "ic19.inf", 0x46000, 0x2000, CRC(ade7645a) SHA1(bfaab1840e3171df895a2333a30b9dac214b3351) )   /* IC19 */

	/* sound CPU */
	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ic8.inf", 0x0e000, 0x2000, CRC(4e3123b8) SHA1(f453feed3ae3b6430db49eb4325f62eecfee9f5e) )    /* IC8  */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "ic57.inf", 0x00000, 0x2000, CRC(65a4ef79) SHA1(270c58901e83665bc388cd9cb92022c55e8eae50) )   /* IC57 */
	ROM_LOAD( "ic58.inf", 0x02000, 0x2000, CRC(4bb1c2a0) SHA1(9e8d214b8d1dbe4c2369e4047e165c9e692098a5) )   /* IC58 */
	ROM_LOAD( "ic41.inf", 0x04000, 0x2000, CRC(f3f7238f) SHA1(3810f1afd318ec37271c099c989b142b85d8da51) )   /* IC41 */

	ROM_REGION( 0x200, "proms", 0 ) /* not hooked up */
	ROM_LOAD( "prom1", 0x00000, 0x020, CRC(85057e40) SHA1(c34cdd24d77031450493da50d4ad02813f9b30a8) )
	ROM_LOAD( "prom2", 0x00000, 0x100, CRC(efb03024) SHA1(4c3e3de374f7959a03dcfcb8a29a372685f3b273) )
	ROM_LOAD( "prom3", 0x00000, 0x200, CRC(0ea3f7fb) SHA1(a8a2a7fbc1a3527a8e2cda71d737afaa717902f1) )
ROM_END


ROM_START( joust2 )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "cpu_2732_ic55_rom2_rev1.4c",   0x0d000, 0x1000, CRC(08b0d5bd) SHA1(b58da478aef36ae20fcfee48151d5d556e16b7b9) )
	ROM_LOAD( "cpu_2732_ic9_rom3_rev2.4d",    0x0e000, 0x1000, CRC(951175ce) SHA1(ac70df125bb438f9fccc082276df4a76ff693e16) )
	ROM_LOAD( "cpu_2732_ic10_rom4_rev2.4f",   0x0f000, 0x1000, CRC(ba6e0f6c) SHA1(431cbf38e919011d030f41008e1ad45e7e0ec38b) )

	ROM_LOAD( "cpu_2732_ic18_rom11_rev1.8j",  0x10000, 0x2000, CRC(9dc986f9) SHA1(5ce479936536ef713cdfc8fc8190d338c46d171e) )
	ROM_LOAD( "cpu_2732_ic16_rom9_rev2.8h",   0x12000, 0x2000, CRC(56e2b550) SHA1(01211d389ca384987d56c26596aa8c1adffdf8dd) )
	ROM_LOAD( "cpu_2732_ic14_rom7_rev2.6j",   0x14000, 0x2000, CRC(f3bce576) SHA1(30ee1b212879b3b55b47c9064f123fb77c8f3089) )
	ROM_LOAD( "cpu_2732_ic12_rom5_rev2.6h",   0x16000, 0x2000, CRC(5f8b4919) SHA1(1215a314c07ef4f244e862743035626cac1d9538) )

	ROM_LOAD( "cpu_2732_ic26_rom19_rev1.10j", 0x20000, 0x2000, CRC(4ef5e805) SHA1(98b93388ab4a4fa6eeceee3386fa46f5a307b8cb) )
	ROM_LOAD( "cpu_2732_ic24_rom17_rev1.10h", 0x22000, 0x2000, CRC(4861f063) SHA1(6db00cce230bf4bdfdfbfe59e0dc2d916b84d0dc) )
	ROM_LOAD( "cpu_2732_ic22_rom15_rev1.9j",  0x24000, 0x2000, CRC(421aafa8) SHA1(06187ba8fef3e89eb399d7040015212bd5f86853) )
	ROM_LOAD( "cpu_2732_ic20_rom13_rev1.9h",  0x26000, 0x2000, CRC(3432ff55) SHA1(aec0f83b92369de8a830ec298ac490a51bc29f26) )

	ROM_LOAD( "cpu_2732_ic17_rom10_rev1.8i",  0x30000, 0x2000, CRC(3e01b597) SHA1(17d09482636d6cda2f3266152396f0461121e748) )
	ROM_LOAD( "cpu_2732_ic15_rom8_rev1.8g",   0x32000, 0x2000, CRC(ff26fb29) SHA1(5ad498db71c384c1928ec965ba3cad48af428f19) )
	ROM_LOAD( "cpu_2732_ic13_rom6_rev2.6i",   0x34000, 0x2000, CRC(5f107db5) SHA1(c413a2e58853ccda602515b9668a6a620294ba49) )

	ROM_LOAD( "cpu_2732_ic25_rom18_rev1.10i", 0x40000, 0x2000, CRC(47580af5) SHA1(d2728f32f02b549c7e9691c668f0097e327a1d2d) )
	ROM_LOAD( "cpu_2732_ic23_rom16_rev1.10g", 0x42000, 0x2000, CRC(869b5942) SHA1(a3f4bab4c0db71589e9be2bbf1f94052ef2f56da) )
	ROM_LOAD( "cpu_2732_ic21_rom14_rev1.9i",  0x44000, 0x2000, CRC(0bbd867c) SHA1(f2db9fc57b6afb762715617345e8c3dcb89b6cc2) )
	ROM_LOAD( "cpu_2732_ic19_rom12_rev1.9g",  0x46000, 0x2000, CRC(b9221ed1) SHA1(428ea8f3e2fa58d875f581f5de6e0d05ed855a45) )

	/* sound CPU */
	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "cpu_2764_ic8_rom1_rev1.0f", 0x0E000, 0x2000, CRC(84517c3c) SHA1(de0b6473953783c091ddcc7aaa89fc1ec3b9d378) )

	/* sound board */
	ROM_REGION( 0x90000, "cvsd:cpu", 0 )
	ROM_LOAD( "snd_27256_rom23_rev1.u4",  0x10000, 0x8000, CRC(3af6b47d) SHA1(aff19d65a4d9c249dec6a9e04a4066fada0f8fa1) )
	ROM_RELOAD(             0x18000, 0x8000 )
	ROM_RELOAD(             0x20000, 0x8000 )
	ROM_RELOAD(             0x28000, 0x8000 )
	ROM_LOAD( "snd_27256_rom24_rev1.u19", 0x30000, 0x8000, CRC(e7f9ed2e) SHA1(6b9ef5189650f0b6b2866da7f532cdf851f02ead) )
	ROM_RELOAD(             0x38000, 0x8000 )
	ROM_RELOAD(             0x40000, 0x8000 )
	ROM_RELOAD(             0x48000, 0x8000 )
	ROM_LOAD( "snd_27256_rom25_rev1.u20", 0x50000, 0x8000, CRC(c85b29f7) SHA1(b37e1890bd0dfa0c7db19fc878450718b60c1ca0) )
	ROM_RELOAD(             0x58000, 0x8000 )
	ROM_RELOAD(             0x60000, 0x8000 )
	ROM_RELOAD(             0x68000, 0x8000 )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "vid_27128_ic57_rom20_rev1.8f", 0x00000, 0x4000, CRC(572c6b01) SHA1(651df3223c1dc42543f57a7204ae492eb15a4999) )
	ROM_LOAD( "vid_27128_ic58_rom21_rev1.9f", 0x04000, 0x4000, CRC(aa94bf05) SHA1(3412dd181e2c12dc2dd1caabfe7e737005b0ccd7) )
	ROM_LOAD( "vid_27128_ic41_rom22_rev1.9d", 0x08000, 0x4000, CRC(c41e3daa) SHA1(fafe76bebd6eaf2cd124c1030e3a58eb5a6cddc6) )

	ROM_REGION( 0x200, "proms", 0 ) /* not hooked up */
	ROM_LOAD( "vid_82s123_ic14_a-5282-10295.2b",   0x00000, 0x020, CRC(85057e40) SHA1(c34cdd24d77031450493da50d4ad02813f9b30a8) )
	ROM_LOAD( "vid_82s129_ic47_a-5282-10294.15d",  0x00000, 0x100, CRC(efb03024) SHA1(4c3e3de374f7959a03dcfcb8a29a372685f3b273) )
	ROM_LOAD( "vid_82s147a_ic60_a-5282-10292.12f", 0x00000, 0x200, CRC(0ea3f7fb) SHA1(a8a2a7fbc1a3527a8e2cda71d737afaa717902f1) )
ROM_END


ROM_START( joust2r1 )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "cpu_2732_ic55_rom2_rev1.4c",   0x0d000, 0x1000, CRC(08b0d5bd) SHA1(b58da478aef36ae20fcfee48151d5d556e16b7b9) )
	ROM_LOAD( "cpu_2732_ic9_rom3_rev1.4d",    0x0e000, 0x1000, CRC(6f319644) SHA1(1a9bc121b830277c42bac816ec26758c915b49dd) )
	ROM_LOAD( "cpu_2732_ic10_rom4_rev1.4f",   0x0f000, 0x1000, CRC(027b9f0c) SHA1(8c4631fc42ed0b87b2bb0326c48b92d73cdd2f42) )

	ROM_LOAD( "cpu_2732_ic18_rom11_rev1.8j",  0x10000, 0x2000, CRC(9dc986f9) SHA1(5ce479936536ef713cdfc8fc8190d338c46d171e) )
	ROM_LOAD( "cpu_2732_ic16_rom9_rev1.8h",   0x12000, 0x2000, CRC(0c77e22f) SHA1(db024b8d2fe79f9230c07398bcade1c75e772541) )
	ROM_LOAD( "cpu_2732_ic14_rom7_rev1.6j",   0x14000, 0x2000, CRC(fb9455ca) SHA1(8963832f2ab6f5b2f31611e768cab636672f398c) )
	ROM_LOAD( "cpu_2732_ic12_rom5_rev1.6h",   0x16000, 0x2000, CRC(31248a0d) SHA1(a27a252b353f99748aacfeb29c8bbbd8b3a833f2) )

	ROM_LOAD( "cpu_2732_ic26_rom19_rev1.10j", 0x20000, 0x2000, CRC(4ef5e805) SHA1(98b93388ab4a4fa6eeceee3386fa46f5a307b8cb) )
	ROM_LOAD( "cpu_2732_ic24_rom17_rev1.10h", 0x22000, 0x2000, CRC(4861f063) SHA1(6db00cce230bf4bdfdfbfe59e0dc2d916b84d0dc) )
	ROM_LOAD( "cpu_2732_ic22_rom15_rev1.9j",  0x24000, 0x2000, CRC(421aafa8) SHA1(06187ba8fef3e89eb399d7040015212bd5f86853) )
	ROM_LOAD( "cpu_2732_ic20_rom13_rev1.9h",  0x26000, 0x2000, CRC(3432ff55) SHA1(aec0f83b92369de8a830ec298ac490a51bc29f26) )

	ROM_LOAD( "cpu_2732_ic17_rom10_rev1.8i",  0x30000, 0x2000, CRC(3e01b597) SHA1(17d09482636d6cda2f3266152396f0461121e748) )
	ROM_LOAD( "cpu_2732_ic15_rom8_rev1.8g",   0x32000, 0x2000, CRC(ff26fb29) SHA1(5ad498db71c384c1928ec965ba3cad48af428f19) )
	ROM_LOAD( "cpu_2732_ic13_rom6_rev1.6i",   0x34000, 0x2000, CRC(6a8c87d7) SHA1(ba66cd8f23a249470c612890829d40d070bbd1e9) )

	ROM_LOAD( "cpu_2732_ic25_rom18_rev1.10i", 0x40000, 0x2000, CRC(47580af5) SHA1(d2728f32f02b549c7e9691c668f0097e327a1d2d) )
	ROM_LOAD( "cpu_2732_ic23_rom16_rev1.10g", 0x42000, 0x2000, CRC(869b5942) SHA1(a3f4bab4c0db71589e9be2bbf1f94052ef2f56da) )
	ROM_LOAD( "cpu_2732_ic21_rom14_rev1.9i",  0x44000, 0x2000, CRC(0bbd867c) SHA1(f2db9fc57b6afb762715617345e8c3dcb89b6cc2) )
	ROM_LOAD( "cpu_2732_ic19_rom12_rev1.9g",  0x46000, 0x2000, CRC(b9221ed1) SHA1(428ea8f3e2fa58d875f581f5de6e0d05ed855a45) )

	/* sound CPU */
	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "cpu_2764_ic8_rom1_rev1.0f", 0x0E000, 0x2000, CRC(84517c3c) SHA1(de0b6473953783c091ddcc7aaa89fc1ec3b9d378) )

	/* sound board */
	ROM_REGION( 0x90000, "cvsd:cpu", 0 )
	ROM_LOAD( "snd_27256_rom23_rev1.u4",  0x10000, 0x8000, CRC(3af6b47d) SHA1(aff19d65a4d9c249dec6a9e04a4066fada0f8fa1) )
	ROM_RELOAD(             0x18000, 0x8000 )
	ROM_RELOAD(             0x20000, 0x8000 )
	ROM_RELOAD(             0x28000, 0x8000 )
	ROM_LOAD( "snd_27256_rom24_rev1.u19", 0x30000, 0x8000, CRC(e7f9ed2e) SHA1(6b9ef5189650f0b6b2866da7f532cdf851f02ead) )
	ROM_RELOAD(             0x38000, 0x8000 )
	ROM_RELOAD(             0x40000, 0x8000 )
	ROM_RELOAD(             0x48000, 0x8000 )
	ROM_LOAD( "snd_27256_rom25_rev1.u20", 0x50000, 0x8000, CRC(c85b29f7) SHA1(b37e1890bd0dfa0c7db19fc878450718b60c1ca0) )
	ROM_RELOAD(             0x58000, 0x8000 )
	ROM_RELOAD(             0x60000, 0x8000 )
	ROM_RELOAD(             0x68000, 0x8000 )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "vid_27128_ic57_rom20_rev1.8f", 0x00000, 0x4000, CRC(572c6b01) SHA1(651df3223c1dc42543f57a7204ae492eb15a4999) )
	ROM_LOAD( "vid_27128_ic58_rom21_rev1.9f", 0x04000, 0x4000, CRC(aa94bf05) SHA1(3412dd181e2c12dc2dd1caabfe7e737005b0ccd7) )
	ROM_LOAD( "vid_27128_ic41_rom22_rev1.9d", 0x08000, 0x4000, CRC(c41e3daa) SHA1(fafe76bebd6eaf2cd124c1030e3a58eb5a6cddc6) )

	ROM_REGION( 0x200, "proms", 0 ) /* not hooked up */
	ROM_LOAD( "vid_82s123_ic14_a-5282-10295.2b",   0x00000, 0x020, CRC(85057e40) SHA1(c34cdd24d77031450493da50d4ad02813f9b30a8) )
	ROM_LOAD( "vid_82s129_ic47_a-5282-10294.15d",  0x00000, 0x100, CRC(efb03024) SHA1(4c3e3de374f7959a03dcfcb8a29a372685f3b273) )
	ROM_LOAD( "vid_82s147a_ic60_a-5282-10292.12f", 0x00000, 0x200, CRC(0ea3f7fb) SHA1(a8a2a7fbc1a3527a8e2cda71d737afaa717902f1) )
ROM_END



/*************************************
 *
 *  Configuration macros
 *
 *************************************/

#define CONFIGURE_BLITTER(x,c) \
	m_blitter_config = x; \
	m_blitter_clip_address = c

#define CONFIGURE_TILEMAP(x) \
	m_williams2_tilemap_config = x



/*************************************
 *
 *  Defender hardware driver init
 *
 *************************************/

DRIVER_INIT_MEMBER(williams_state,defender)
{
	CONFIGURE_BLITTER(WILLIAMS_BLITTER_NONE, 0x0000);
}


DRIVER_INIT_MEMBER(williams_state,defndjeu)
{
	UINT8 *rom = memregion("maincpu")->base();
	int i;

	CONFIGURE_BLITTER(WILLIAMS_BLITTER_NONE, 0x0000);

	/* apply simple decryption by swapping bits 0 and 7 */
	for (i = 0xd000; i < 0x19000; i++)
		rom[i] = BITSWAP8(rom[i],0,6,5,4,3,2,1,7);
}


DRIVER_INIT_MEMBER(williams_state,mayday)
{
	CONFIGURE_BLITTER(WILLIAMS_BLITTER_NONE, 0x0000);

	/* install a handler to catch protection checks */
	m_mayday_protection = m_maincpu->space(AS_PROGRAM).install_read_handler(0xa190, 0xa191, read8_delegate(FUNC(williams_state::mayday_protection_r),this));
}



/*************************************
 *
 *  Standard hardware driver init
 *
 *************************************/

DRIVER_INIT_MEMBER(williams_state,stargate)
{
	CONFIGURE_BLITTER(WILLIAMS_BLITTER_NONE, 0x0000);
}


DRIVER_INIT_MEMBER(williams_state,robotron)
{
	CONFIGURE_BLITTER(WILLIAMS_BLITTER_SC01, 0xc000);
}


DRIVER_INIT_MEMBER(williams_state,joust)
{
	CONFIGURE_BLITTER(WILLIAMS_BLITTER_SC01, 0xc000);
}


DRIVER_INIT_MEMBER(williams_state,bubbles)
{
	CONFIGURE_BLITTER(WILLIAMS_BLITTER_SC01, 0xc000);

	/* bubbles has a full 8-bit-wide CMOS */
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xcc00, 0xcfff, write8_delegate(FUNC(williams_state::bubbles_cmos_w),this));
}


DRIVER_INIT_MEMBER(williams_state,splat)
{
	CONFIGURE_BLITTER(WILLIAMS_BLITTER_SC02, 0xc000);
}


DRIVER_INIT_MEMBER(williams_state,sinistar)
{
	CONFIGURE_BLITTER(WILLIAMS_BLITTER_SC01, 0x7400);
}


DRIVER_INIT_MEMBER(williams_state,playball)
{
	CONFIGURE_BLITTER(WILLIAMS_BLITTER_SC01, 0xc000);
}


DRIVER_INIT_MEMBER(blaster_state,blaster)
{
	CONFIGURE_BLITTER(WILLIAMS_BLITTER_SC02, 0x9700);
}


DRIVER_INIT_MEMBER(williams_state,spdball)
{
	pia6821_device *pia_3 = machine().device<pia6821_device>("pia_3");

	CONFIGURE_BLITTER(WILLIAMS_BLITTER_SC01, 0xc000);

	/* add a third PIA */
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xc808, 0xc80b, read8_delegate(FUNC(pia6821_device::read), pia_3), write8_delegate(FUNC(pia6821_device::write), pia_3));

	/* install extra input handlers */
	m_maincpu->space(AS_PROGRAM).install_read_port(0xc800, 0xc800, "AN0");
	m_maincpu->space(AS_PROGRAM).install_read_port(0xc801, 0xc801, "AN1");
	m_maincpu->space(AS_PROGRAM).install_read_port(0xc802, 0xc802, "AN2");
	m_maincpu->space(AS_PROGRAM).install_read_port(0xc803, 0xc803, "AN3");
}


DRIVER_INIT_MEMBER(williams_state,alienar)
{
	CONFIGURE_BLITTER(WILLIAMS_BLITTER_SC01, 0xc000);
	m_maincpu->space(AS_PROGRAM).nop_write(0xcbff, 0xcbff);
}


DRIVER_INIT_MEMBER(williams_state,alienaru)
{
	CONFIGURE_BLITTER(WILLIAMS_BLITTER_SC01, 0xc000);
	m_maincpu->space(AS_PROGRAM).nop_write(0xcbff, 0xcbff);
}


DRIVER_INIT_MEMBER(williams_state,lottofun)
{
	CONFIGURE_BLITTER(WILLIAMS_BLITTER_SC01, 0xc000);
}



/*************************************
 *
 *  2nd gen hardware driver init
 *
 *************************************/

DRIVER_INIT_MEMBER(williams2_state,mysticm)
{
	CONFIGURE_BLITTER(WILLIAMS_BLITTER_SC02, 0x9000);
	CONFIGURE_TILEMAP(WILLIAMS_TILEMAP_MYSTICM);
}


DRIVER_INIT_MEMBER(williams2_state,tshoot)
{
	CONFIGURE_BLITTER(WILLIAMS_BLITTER_SC02, 0x9000);
	CONFIGURE_TILEMAP(WILLIAMS_TILEMAP_TSHOOT);
}


DRIVER_INIT_MEMBER(williams2_state,inferno)
{
	CONFIGURE_BLITTER(WILLIAMS_BLITTER_SC02, 0x9000);
	CONFIGURE_TILEMAP(WILLIAMS_TILEMAP_TSHOOT);
}


DRIVER_INIT_MEMBER(joust2_state,joust2)
{
	CONFIGURE_BLITTER(WILLIAMS_BLITTER_SC02, 0x9000);
	CONFIGURE_TILEMAP(WILLIAMS_TILEMAP_JOUST2);
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

/* Defender hardware games */
GAME( 1980, defender,   0,        defender,       defender, williams_state, defender, ROT0,   "Williams", "Defender (Red label)", MACHINE_SUPPORTS_SAVE ) // developers left Williams in 1981 and formed Vid Kidz
GAME( 1980, defenderg,  defender, defender,       defender, williams_state, defender, ROT0,   "Williams", "Defender (Green label)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, defenderb,  defender, defender,       defender, williams_state, defender, ROT0,   "Williams", "Defender (Blue label)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, defenderw,  defender, defender,       defender, williams_state, defender, ROT0,   "Williams", "Defender (White label)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, defndjeu,   defender, defender,       defender, williams_state, defndjeu, ROT0,   "bootleg (Jeutel)", "Defender (bootleg)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1980, tornado1,   defender, defender,       defender, williams_state, defndjeu, ROT0,   "bootleg (Jeutel)", "Tornado (set 1, Defender bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, tornado2,   defender, defender,       defender, williams_state, defndjeu, ROT0,   "bootleg (Jeutel)", "Tornado (set 2, Defender bootleg)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // bad dump?
GAME( 1980, zero,       defender, defender,       defender, williams_state, defndjeu, ROT0,   "bootleg (Jeutel)", "Zero (set 1, Defender bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, zero2,      defender, defender,       defender, williams_state, defndjeu, ROT0,   "bootleg (Amtec)", "Zero (set 2, Defender bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, defcmnd,    defender, defender,       defender, williams_state, defender, ROT0,   "bootleg", "Defense Command (Defender bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, defence,    defender, defender,       defender, williams_state, defender, ROT0,   "bootleg (Outer Limits)", "Defence Command (Defender bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, startrkd,   defender, defender,       defender, williams_state, defender, ROT0,   "bootleg", "Star Trek (Defender bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, mayday,     0,        defender,       mayday, williams_state,   mayday,   ROT0,   "Hoei", "Mayday (set 1)", MACHINE_SUPPORTS_SAVE ) // original by Hoei, which one of these 3 sets is bootleg/licensed/original is unknown
GAME( 1980, maydaya,    mayday,   defender,       mayday, williams_state,   mayday,   ROT0,   "Hoei", "Mayday (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, maydayb,    mayday,   defender,       mayday, williams_state,   mayday,   ROT0,   "Hoei", "Mayday (set 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, batlzone,   mayday,   defender,       mayday, williams_state,   mayday,   ROT0,   "bootleg (Video Game)", "Battle Zone (bootleg of Mayday)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, colony7,    0,        defender,       colony7, williams_state,  defender, ROT270, "Taito", "Colony 7 (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, colony7a,   colony7,  defender,       colony7, williams_state,  defender, ROT270, "Taito", "Colony 7 (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, jin,        0,        jin,            jin, williams_state,      defender, ROT90,  "Falcon", "Jin", MACHINE_SUPPORTS_SAVE )

/* Standard Williams hardware */
GAME( 1981, stargate,   0,        williams,       stargate, williams_state, stargate, ROT0,   "Williams / Vid Kidz", "Stargate", MACHINE_SUPPORTS_SAVE )
GAME( 1982, robotron,   0,        williams,       robotron, williams_state, robotron, ROT0,   "Williams / Vid Kidz", "Robotron: 2084 (Solid Blue label)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, robotronyo, robotron, williams,       robotron, williams_state, robotron, ROT0,   "Williams / Vid Kidz", "Robotron: 2084 (Yellow/Orange label)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, joust,      0,        williams_muxed, joust, williams_state,    joust,    ROT0,   "Williams", "Joust (White/Green label)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, joustr,     joust,    williams_muxed, joust, williams_state,    joust,    ROT0,   "Williams", "Joust (Solid Red label)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, joustwr,    joust,    williams_muxed, joust, williams_state,    joust,    ROT0,   "Williams", "Joust (White/Red label)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, bubbles,    0,        williams,       bubbles, williams_state,  bubbles,  ROT0,   "Williams", "Bubbles", MACHINE_SUPPORTS_SAVE )
GAME( 1982, bubblesr,   bubbles,  williams,       bubbles, williams_state,  bubbles,  ROT0,   "Williams", "Bubbles (Solid Red label)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, bubblesp,   bubbles,  williams,       bubbles, williams_state,  bubbles,  ROT0,   "Williams", "Bubbles (prototype version)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, splat,      0,        williams_muxed, splat, williams_state,    splat,    ROT0,   "Williams", "Splat!", MACHINE_SUPPORTS_SAVE )
GAME( 1982, sinistar,   0,        sinistar,       sinistar, williams_state, sinistar, ROT270, "Williams", "Sinistar (revision 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, sinistar1,  sinistar, sinistar,       sinistar, williams_state, sinistar, ROT270, "Williams", "Sinistar (prototype version)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, sinistar2,  sinistar, sinistar,       sinistar, williams_state, sinistar, ROT270, "Williams", "Sinistar (revision 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, playball,   0,        playball,       playball, williams_state, playball, ROT270, "Williams", "PlayBall! (prototype)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, blaster,    0,        blaster,        blaster, blaster_state,   blaster,  ROT0,   "Williams / Vid Kidz", "Blaster", MACHINE_SUPPORTS_SAVE )
GAME( 1983, blastero,   blaster,  blaster,        blaster, blaster_state,   blaster,  ROT0,   "Williams / Vid Kidz", "Blaster (location test)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, blasterkit, blaster,  blastkit,       blastkit, blaster_state,  blaster,  ROT0,   "Williams / Vid Kidz", "Blaster (conversion kit)", MACHINE_SUPPORTS_SAVE ) // mono sound
GAME( 1985, spdball,    0,        spdball,        spdball, williams_state,  spdball,  ROT0,   "Williams", "Speed Ball - Contest at Neonworld (prototype)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, alienar,    0,        williams_muxed, alienar, williams_state,  alienar,  ROT0,   "Duncan Brown", "Alien Arena", MACHINE_SUPPORTS_SAVE )
GAME( 1985, alienaru,   alienar,  williams_muxed, alienar, williams_state,  alienaru, ROT0,   "Duncan Brown", "Alien Arena (Stargate upgrade)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, lottofun,   0,        lottofun,       lottofun, williams_state, lottofun, ROT0,   "H.A.R. Management", "Lotto Fun", MACHINE_SUPPORTS_SAVE )

/* 2nd Generation Williams hardware with tilemaps */
GAME( 1983, mysticm,    0,        mysticm,        mysticm, williams2_state, mysticm,  ROT0,   "Williams", "Mystic Marathon", MACHINE_SUPPORTS_SAVE )
GAME( 1983, mysticmp,   mysticm,  mysticm,        mysticm, williams2_state, mysticm,  ROT0,   "Williams", "Mystic Marathon (prototype)", MACHINE_SUPPORTS_SAVE ) // newest roms are 'proto 6' ?
GAME( 1984, tshoot,     0,        tshoot,         tshoot, williams2_state,  tshoot,   ROT0,   "Williams", "Turkey Shoot", MACHINE_SUPPORTS_SAVE )
GAME( 1984, inferno,    0,        williams2,      inferno, williams2_state, inferno,  ROT0,   "Williams", "Inferno (Williams)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, joust2,     0,        joust2,         joust2, joust2_state,     joust2,   ROT270, "Williams", "Joust 2 - Survival of the Fittest (revision 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, joust2r1,   joust2,   joust2,         joust2, joust2_state,     joust2,   ROT270, "Williams", "Joust 2 - Survival of the Fittest (revision 1)", MACHINE_SUPPORTS_SAVE )
