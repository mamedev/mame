// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                          -= Newer Seta Hardware =-

                    driver by   Luca Elia (l.elia@tin.it)


CPU    :    TMP68301*
            or ColdFire + H8/3007 + PIC12C508 (for EVA2 & EVA3 PCBs)

Video  :    DX-101
            DX-102 x3

Sound  :    X1-010
            or OKI M9810 (for EVA2 & EVA3 PCBs)

OSC    :    50.00000MHz
            32.53047MHz

*   The Toshiba TMP68301 is a 68HC000 + serial I/O, parallel I/O,
    3 timers, address decoder, wait generator, interrupt controller,
    all integrated in a single chip.

-------------------------------------------------------------------------------------------
Ordered by Board        Year    Game                                    By
-------------------------------------------------------------------------------------------
P-FG01-1                1995    Guardians / Denjin Makai II             Banpresto
P0-113A                 1994    Mobile Suit Gundam EX Revue             Banpresto
P0-123A                 1996    Wakakusamonogatari Mahjong Yonshimai    Maboroshi Ware
P0-125A ; KE (Namco)    1996    Kosodate Quiz My Angel                  Namco
P0-136A ; KL (Namco)    1997    Kosodate Quiz My Angel 2                Namco
P-FG-02                 1997    Reel'N Quake                            <unknown>
P-FG-03              <unknown>  Endless Riches                          E.N.Tiger
P0-140B                 2000    Funcube                                 Namco
P0-140B                 2000    Namco Stars                             Namco
P0-142A                 1999    Puzzle De Bowling                       MOSS / Nihon System
P0-142A + extra parts   2000    Penguin Brothers                        Subsino
B0-003A (or B0-003B)    2000    Deer Hunting USA                        Sammy
B0-003A (or B0-003B)    2001    Turkey Hunting USA                      Sammy
B0-006B                 2001-2  Funcube 2 - 5                           Namco
B0-010A                 2001    Wing Shooting Championship              Sammy
B0-010A                 2002    Trophy Hunting - Bear & Moose           Sammy
-------------------------------------------------------------------------------------------

TODO:

- Proper emulation of the TMP68301 CPU, in a core file.
- Proper emulation of the ColdFire CPU, in a core file.
- Flip screen / Zooming support.
- Fix some graphics imperfections (e.g. color depth selection, "tilemap" sprites) [all done? - NS]
- I added a kludge involving a -0x10 yoffset, this fixes the lifeline in myangel.
  I didn't find a better way to do it without breaking pzlbowl's title screen.

mj4simai:
- test mode doesn't work correctly, the grid is ok but when you press a key to go to the
  next screen (input test) it stays up a second and then drops back into the game

myangel:
- some gfx at the end of the game (rays just before fireworks, and the border during
  the wedding) have wrong colors. You can see the rays red, green and yellow because
  that's how the palette is preinitialized by MAME, but the game never sets up those
  palette entries. The game selects color depth "1", whose meaning is uncertain, and
  color code 0 so I see no way to point to a different section of palette RAM.
- there are glitches in the bg horizontal scroll in the wedding sequence at the end of
  the game. It looks like "scrollx" should be delayed one frame wrt "xoffs".
- there's a 4 pixel gap at the top of the title screen since clipping was reimplemented.

myangel2:
- before each level, the background image is shown with completely wrong colors. It
  corrects itself when the level starts.

grdians:
- the map screen after the character selection needs zooming. There is a global
  zoom register that should affect the background map and the level picture but
  not the frontmost frame. This latter should use color 7ff (the last one) and
  ignore the individual color codes in the tiles data. Note: the frontmost frame
  has the shadow bit set, and has become invisible after implementing it.

pengbros:
- Zooming is used briefly.

deerhunt,wschamp:
- offset tilemap sprite during demo. In deerhunt intro, the hunter should zoom
  in to the deer. In wschamp intro the GPS unit should zoom to the high scores.

wschampb:
- dumps of the program roms matched the hand written checksum for each chip, but
  the boot screen reports NG for both roms. - Is this correct and a bug from the
  original release? Is that why the next bug fix release is v1.01? IE: such a
  a minor increase in the version number.

trophyh:
- mame hangs for around 15 seconds every now and then, at scene changes.
  This is probably due to a couple of frames with an odd or corrupt sprites list,
  taking a long time to render.

funcube series:
- Hacked to run, as they use a ColdFire CPU.
- Pay-out key causes "unknown error" after coin count reaches 0.

reelquak:
- Needs an x offset for tilemap sprites.
- There are one frame glitches in the reels scroll.

***************************************************************************/

#include "emu.h"
#include "includes/seta2.h"
#include "cpu/m68000/m68000.h"
#include "machine/tmp68301.h"
#include "cpu/h8/h83006.h"
#include "sound/okim9810.h"
#include "machine/eepromser.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/mcf5206e.h"


/***************************************************************************


                            Memory Maps - Main CPU


***************************************************************************/

WRITE16_MEMBER(seta2_state::sound_bank_w)
{
	if (ACCESSING_BITS_0_7)
	{
		UINT8 *ROM = memregion( "x1snd" )->base();
		int banks = (memregion( "x1snd" )->bytes() - 0x100000) / 0x20000;
		if (data >= banks)
		{
			logerror("CPU #0 PC %06X: invalid sound bank %04X\n",space.device().safe_pc(),data);
			data %= banks;
		}
		memcpy(ROM + offset * 0x20000, ROM + 0x100000 + data * 0x20000, 0x20000);
	}
}


/***************************************************************************
                                Guardians
***************************************************************************/

WRITE16_MEMBER(seta2_state::grdians_lockout_w)
{
	if (ACCESSING_BITS_0_7)
	{
		// initially 0, then either $25 (coin 1) or $2a (coin 2)
		coin_counter_w(machine(), 0,data & 0x01);   // or 0x04
		coin_counter_w(machine(), 1,data & 0x02);   // or 0x08
	}
//  popmessage("%04X", data & 0xffff);
}

static ADDRESS_MAP_START( grdians_map, AS_PROGRAM, 16, seta2_state )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM                             // ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM                             // RAM
	AM_RANGE(0x304000, 0x30ffff) AM_RAM                             // ? seems tile data
	AM_RANGE(0x600000, 0x600001) AM_READ_PORT("DSW1")               // DSW 1
	AM_RANGE(0x600002, 0x600003) AM_READ_PORT("DSW2")               // DSW 2
	AM_RANGE(0x700000, 0x700001) AM_READ_PORT("P1")                 // P1
	AM_RANGE(0x700002, 0x700003) AM_READ_PORT("P2")                 // P2
	AM_RANGE(0x700004, 0x700005) AM_READ_PORT("SYSTEM")             // Coins
	AM_RANGE(0x70000c, 0x70000d) AM_READ(watchdog_reset16_r)        // Watchdog
	AM_RANGE(0x800000, 0x800001) AM_WRITE(grdians_lockout_w)
	AM_RANGE(0xb00000, 0xb03fff) AM_DEVREADWRITE("x1snd", x1_010_device, word_r, word_w)   // Sound
	AM_RANGE(0xc00000, 0xc3ffff) AM_RAM AM_SHARE("spriteram")       // Sprites
	AM_RANGE(0xc40000, 0xc4ffff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0xc50000, 0xc5ffff) AM_RAM                             // cleared
	AM_RANGE(0xc60000, 0xc6003f) AM_WRITE(vregs_w) AM_SHARE("vregs")  // Video Registers
	AM_RANGE(0xe00010, 0xe0001f) AM_WRITE(sound_bank_w)       // Samples Banks
	AM_RANGE(0xfffc00, 0xffffff) AM_DEVREADWRITE("tmp68301", tmp68301_device, regs_r, regs_w)  // TMP68301 Registers
ADDRESS_MAP_END

/***************************************************************************
                        Mobile Suit Gundam EX Revue
***************************************************************************/

READ16_MEMBER(seta2_state::gundamex_eeprom_r)
{
	return ((m_eeprom->do_read() & 1)) << 3;
}

WRITE16_MEMBER(seta2_state::gundamex_eeprom_w)
{
	m_eeprom->clk_write((data & 0x2) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->di_write(data & 0x1);
	m_eeprom->cs_write((data & 0x4) ? ASSERT_LINE : CLEAR_LINE);
}

static ADDRESS_MAP_START( gundamex_map, AS_PROGRAM, 16, seta2_state )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM                             // ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM                             // RAM
	AM_RANGE(0x500000, 0x57ffff) AM_ROM                             // ROM
	AM_RANGE(0x600000, 0x600001) AM_READ_PORT("DSW1")               // DSW 1
	AM_RANGE(0x600002, 0x600003) AM_READ_PORT("DSW2")               // DSW 2
	AM_RANGE(0x700000, 0x700001) AM_READ_PORT("P1")                 // P1
	AM_RANGE(0x700002, 0x700003) AM_READ_PORT("P2")                 // P2
	AM_RANGE(0x700004, 0x700005) AM_READ_PORT("SYSTEM")             // Coins
	AM_RANGE(0x700008, 0x700009) AM_READ_PORT("IN0")                // P1
	AM_RANGE(0x70000a, 0x70000b) AM_READ_PORT("IN1")                // P2
	AM_RANGE(0x70000c, 0x70000d) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0x800000, 0x800001) AM_WRITE(grdians_lockout_w)
	AM_RANGE(0xb00000, 0xb03fff) AM_DEVREADWRITE("x1snd", x1_010_device, word_r, word_w)   // Sound
	AM_RANGE(0xc00000, 0xc3ffff) AM_RAM AM_SHARE("spriteram")   // Sprites
	AM_RANGE(0xc40000, 0xc4ffff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0xc50000, 0xc5ffff) AM_RAM                             // cleared
	AM_RANGE(0xc60000, 0xc6003f) AM_WRITE(vregs_w) AM_SHARE("vregs")  // Video Registers
	AM_RANGE(0xe00010, 0xe0001f) AM_WRITE(sound_bank_w)       // Samples Banks
	AM_RANGE(0xfffc00, 0xffffff) AM_DEVREADWRITE("tmp68301", tmp68301_device, regs_r, regs_w)  // TMP68301 Registers
ADDRESS_MAP_END


/***************************************************************************
                      Wakakusamonogatari Mahjong Yonshimai
***************************************************************************/

MACHINE_START_MEMBER(seta2_state, mj4simai)
{
	save_item(NAME(m_keyboard_row));
}

READ16_MEMBER(seta2_state::mj4simai_p1_r)
{
	switch (m_keyboard_row)
	{
		case 0x01: return ioport("P1_KEY0")->read();
		case 0x02: return ioport("P1_KEY1")->read();
		case 0x04: return ioport("P1_KEY2")->read();
		case 0x08: return ioport("P1_KEY3")->read();
		case 0x10: return ioport("P1_KEY4")->read();
		default:   logerror("p1_r with keyboard_row = %02x\n", m_keyboard_row); return 0xffff;
	}
}

READ16_MEMBER(seta2_state::mj4simai_p2_r)
{
	switch (m_keyboard_row)
	{
		case 0x01: return ioport("P2_KEY0")->read();
		case 0x02: return ioport("P2_KEY1")->read();
		case 0x04: return ioport("P2_KEY2")->read();
		case 0x08: return ioport("P2_KEY3")->read();
		case 0x10: return ioport("P2_KEY4")->read();
		default:   logerror("p2_r with keyboard_row = %02x\n", m_keyboard_row); return 0xffff;
	}
}

WRITE16_MEMBER(seta2_state::mj4simai_keyboard_w)
{
	if (ACCESSING_BITS_0_7)
		m_keyboard_row = data & 0xff;
}

static ADDRESS_MAP_START( mj4simai_map, AS_PROGRAM, 16, seta2_state )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM                             // ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM                             // RAM
	AM_RANGE(0x600000, 0x600001) AM_READ(mj4simai_p1_r)             // P1
	AM_RANGE(0x600002, 0x600003) AM_READ(mj4simai_p2_r)             // P2
	AM_RANGE(0x600004, 0x600005) AM_WRITE(mj4simai_keyboard_w)      // select keyboard row to read
	AM_RANGE(0x600006, 0x600007) AM_READ(watchdog_reset16_r)        // Watchdog
	AM_RANGE(0x600100, 0x600101) AM_READ_PORT("SYSTEM")             //
	AM_RANGE(0x600200, 0x600201) AM_WRITENOP                        // Leds? Coins?
	AM_RANGE(0x600300, 0x600301) AM_READ_PORT("DSW1")               // DSW 1
	AM_RANGE(0x600302, 0x600303) AM_READ_PORT("DSW2")               // DSW 2
	AM_RANGE(0x600300, 0x60030f) AM_WRITE(sound_bank_w)       // Samples Banks
	AM_RANGE(0xb00000, 0xb03fff) AM_DEVREADWRITE("x1snd", x1_010_device, word_r, word_w)   // Sound
	AM_RANGE(0xc00000, 0xc3ffff) AM_RAM AM_SHARE("spriteram")   // Sprites
	AM_RANGE(0xc40000, 0xc4ffff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0xc60000, 0xc6003f) AM_WRITE(vregs_w) AM_SHARE("vregs")  // Video Registers
	AM_RANGE(0xfffc00, 0xffffff) AM_DEVREADWRITE("tmp68301", tmp68301_device, regs_r, regs_w)  // TMP68301 Registers
ADDRESS_MAP_END


/***************************************************************************
                            Kosodate Quiz My Angel
***************************************************************************/

static ADDRESS_MAP_START( myangel_map, AS_PROGRAM, 16, seta2_state )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM                             // ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM                             // RAM
	AM_RANGE(0x700000, 0x700001) AM_READ_PORT("P1")                 // P1
	AM_RANGE(0x700002, 0x700003) AM_READ_PORT("P2")                 // P2
	AM_RANGE(0x700004, 0x700005) AM_READ_PORT("SYSTEM")             // Coins
	AM_RANGE(0x700006, 0x700007) AM_READ(watchdog_reset16_r)        // Watchdog
	AM_RANGE(0x700200, 0x700201) AM_WRITENOP                        // Leds? Coins?
	AM_RANGE(0x700300, 0x700301) AM_READ_PORT("DSW1")               // DSW 1
	AM_RANGE(0x700302, 0x700303) AM_READ_PORT("DSW2")               // DSW 2
	AM_RANGE(0x700310, 0x70031f) AM_WRITE(sound_bank_w)       // Samples Banks
	AM_RANGE(0xb00000, 0xb03fff) AM_DEVREADWRITE("x1snd", x1_010_device, word_r, word_w)   // Sound
	AM_RANGE(0xc00000, 0xc3ffff) AM_RAM AM_SHARE("spriteram")       // Sprites
	AM_RANGE(0xc40000, 0xc4ffff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0xc60000, 0xc6003f) AM_WRITE(vregs_w) AM_SHARE("vregs")              // Video Registers
	AM_RANGE(0xfffc00, 0xffffff) AM_DEVREADWRITE("tmp68301", tmp68301_device, regs_r, regs_w)      // TMP68301 Registers
ADDRESS_MAP_END


/***************************************************************************
                            Kosodate Quiz My Angel 2
***************************************************************************/

static ADDRESS_MAP_START( myangel2_map, AS_PROGRAM, 16, seta2_state )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM                             // ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM                             // RAM
	AM_RANGE(0x600000, 0x600001) AM_READ_PORT("P1")                 // P1
	AM_RANGE(0x600002, 0x600003) AM_READ_PORT("P2")                 // P2
	AM_RANGE(0x600004, 0x600005) AM_READ_PORT("SYSTEM")             // Coins
	AM_RANGE(0x600006, 0x600007) AM_READ(watchdog_reset16_r)        // Watchdog
	AM_RANGE(0x600200, 0x600201) AM_WRITENOP                        // Leds? Coins?
	AM_RANGE(0x600300, 0x600301) AM_READ_PORT("DSW1")               // DSW 1
	AM_RANGE(0x600302, 0x600303) AM_READ_PORT("DSW2")               // DSW 2
	AM_RANGE(0x600300, 0x60030f) AM_WRITE(sound_bank_w)       // Samples Banks
	AM_RANGE(0xb00000, 0xb03fff) AM_DEVREADWRITE("x1snd", x1_010_device, word_r, word_w)   // Sound
	AM_RANGE(0xd00000, 0xd3ffff) AM_RAM AM_SHARE("spriteram")       // Sprites
	AM_RANGE(0xd40000, 0xd4ffff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0xd60000, 0xd6003f) AM_WRITE(vregs_w) AM_SHARE("vregs")          // Video Registers
	AM_RANGE(0xfffc00, 0xffffff) AM_DEVREADWRITE("tmp68301", tmp68301_device, regs_r, regs_w)      // TMP68301 Registers
ADDRESS_MAP_END


/***************************************************************************
                                Puzzle De Bowling
***************************************************************************/

/*  The game checks for a specific value read from the ROM region.
    The offset to use is stored in RAM at address 0x20BA16 */
READ16_MEMBER(seta2_state::pzlbowl_protection_r)
{
	UINT32 address = (space.read_word(0x20ba16) << 16) | space.read_word(0x20ba18);
	return memregion("maincpu")->base()[address - 2];
}

READ16_MEMBER(seta2_state::pzlbowl_coins_r)
{
	return ioport("SYSTEM")->read() | (machine().rand() & 0x80 );
}

WRITE16_MEMBER(seta2_state::pzlbowl_coin_counter_w)
{
	if (ACCESSING_BITS_0_7)
	{
		coin_counter_w(machine(), 0,data & 0x10);
		coin_counter_w(machine(), 1,data & 0x20);
	}
}

static ADDRESS_MAP_START( pzlbowl_map, AS_PROGRAM, 16, seta2_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM                                 // ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM                                 // RAM
	AM_RANGE(0x400300, 0x400301) AM_READ_PORT("DSW1")                   // DSW 1
	AM_RANGE(0x400302, 0x400303) AM_READ_PORT("DSW2")                   // DSW 2
	AM_RANGE(0x400300, 0x40030f) AM_WRITE(sound_bank_w)           // Samples Banks
	AM_RANGE(0x500000, 0x500001) AM_READ_PORT("P1")                     // P1
	AM_RANGE(0x500002, 0x500003) AM_READ_PORT("P2")                     // P2
	AM_RANGE(0x500004, 0x500005) AM_READWRITE(pzlbowl_coins_r,pzlbowl_coin_counter_w)   // Coins + Protection?
	AM_RANGE(0x500006, 0x500007) AM_READ(watchdog_reset16_r)            // Watchdog
	AM_RANGE(0x700000, 0x700001) AM_READ(pzlbowl_protection_r)          // Protection
	AM_RANGE(0x800000, 0x83ffff) AM_RAM AM_SHARE("spriteram")       // Sprites
	AM_RANGE(0x840000, 0x84ffff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0x860000, 0x86003f) AM_WRITE(vregs_w) AM_SHARE("vregs")              // Video Registers
	AM_RANGE(0x900000, 0x903fff) AM_DEVREADWRITE("x1snd", x1_010_device, word_r, word_w)   // Sound
	AM_RANGE(0xfffc00, 0xffffff) AM_DEVREADWRITE("tmp68301", tmp68301_device, regs_r, regs_w)      // TMP68301 Registers
ADDRESS_MAP_END


/***************************************************************************
                            Penguin Bros
***************************************************************************/

static ADDRESS_MAP_START( penbros_map, AS_PROGRAM, 16, seta2_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM                             // ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM                             // RAM
	AM_RANGE(0x210000, 0x23ffff) AM_RAM                             // RAM
	AM_RANGE(0x300000, 0x30ffff) AM_RAM                             // RAM
	AM_RANGE(0x500300, 0x500301) AM_READ_PORT("DSW1")               // DSW 1
	AM_RANGE(0x500302, 0x500303) AM_READ_PORT("DSW2")               // DSW 2
	AM_RANGE(0x500300, 0x50030f) AM_WRITE(sound_bank_w)       // Samples Banks
	AM_RANGE(0x600000, 0x600001) AM_READ_PORT("P1")                 // P1
	AM_RANGE(0x600002, 0x600003) AM_READ_PORT("P2")                 // P2
	AM_RANGE(0x600004, 0x600005) AM_READ_PORT("SYSTEM")             // Coins
	AM_RANGE(0x600004, 0x600005) AM_WRITE(pzlbowl_coin_counter_w)   // Coins Counter
	AM_RANGE(0x600006, 0x600007) AM_READ(watchdog_reset16_r)        // Watchdog
	//AM_RANGE(0x700000, 0x700001) AM_READ(pzlbowl_protection_r)      // Protection
	AM_RANGE(0xb00000, 0xb3ffff) AM_RAM AM_SHARE("spriteram")       // Sprites
	AM_RANGE(0xb40000, 0xb4ffff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0xb60000, 0xb6003f) AM_WRITE(vregs_w) AM_SHARE("vregs")
	AM_RANGE(0xa00000, 0xa03fff) AM_DEVREADWRITE("x1snd", x1_010_device, word_r, word_w)   // Sound
	AM_RANGE(0xfffc00, 0xffffff) AM_DEVREADWRITE("tmp68301", tmp68301_device, regs_r, regs_w)      // TMP68301 Registers
ADDRESS_MAP_END


/***************************************************************************
                              Reel'N Quake
***************************************************************************/

WRITE16_MEMBER(seta2_state::reelquak_leds_w)
{
	if (ACCESSING_BITS_0_7)
	{
		set_led_status( machine(), 0, data & 0x0001 );  // start
		set_led_status( machine(), 1, data & 0x0002 );  // small
		set_led_status( machine(), 2, data & 0x0004 );  // bet
		set_led_status( machine(), 3, data & 0x0008 );  // big
		set_led_status( machine(), 4, data & 0x0010 );  // double up
		set_led_status( machine(), 5, data & 0x0020 );  // collect
		set_led_status( machine(), 6, data & 0x0040 );  // bet cancel
	}
	if (ACCESSING_BITS_8_15)
	{
		machine().device<ticket_dispenser_device>("ticket")->write(space, 0, (data & 0x0100) >> 1); // ticket dispenser
	}

//  popmessage("LED %04X", data);
}

WRITE16_MEMBER(seta2_state::reelquak_coin_w)
{
	if (ACCESSING_BITS_0_7)
	{
		coin_counter_w(machine(), 0, data & 0x01);  // coin in
		coin_counter_w(machine(), 1, data & 0x02);  // coin in
		coin_counter_w(machine(), 2, data & 0x04);  // pay out
		coin_counter_w(machine(), 3, data & 0x08);  // key in
		//                                data & 0x10); // Sound IRQ Ack.? 1->0
		//                                data & 0x20); // Vblank IRQ.? 1
	}
//  popmessage("COIN %04X", data & 0xffff);
}

static ADDRESS_MAP_START( reelquak_map, AS_PROGRAM, 16, seta2_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM                             // ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM                             // RAM
	AM_RANGE(0x300000, 0x303fff) AM_RAM AM_SHARE("nvram")           // NVRAM (Battery Backed)
	AM_RANGE(0x400000, 0x400001) AM_READ_PORT("P1")                 // P1
	AM_RANGE(0x400002, 0x400003) AM_READ_PORT("TICKET")             // Tickets
	AM_RANGE(0x400004, 0x400005) AM_READ_PORT("SYSTEM")             // Coins
	AM_RANGE(0x400006, 0x400007) AM_READ(watchdog_reset16_r)        // Watchdog
	AM_RANGE(0x400200, 0x400201) AM_WRITE(reelquak_coin_w)          // Coin Counters / IRQ Ack
	AM_RANGE(0x400300, 0x400301) AM_READ_PORT("DSW1")               // DSW 1
	AM_RANGE(0x400302, 0x400303) AM_READ_PORT("DSW2")               // DSW 2
	AM_RANGE(0x400300, 0x40030f) AM_WRITE(sound_bank_w)       // Samples Banks
	AM_RANGE(0xb00000, 0xb03fff) AM_DEVREADWRITE("x1snd", x1_010_device, word_r, word_w)   // Sound
	AM_RANGE(0xc00000, 0xc3ffff) AM_RAM AM_SHARE("spriteram")       // Sprites
	AM_RANGE(0xc40000, 0xc4ffff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0xc60000, 0xc6003f) AM_WRITE(vregs_w) AM_SHARE("vregs")              // Video Registers
	AM_RANGE(0xfffc00, 0xffffff) AM_DEVREADWRITE("tmp68301", tmp68301_device, regs_r, regs_w)      // TMP68301 Registers
ADDRESS_MAP_END


/***************************************************************************
                                Namco Stars
***************************************************************************/

// To be done:
static ADDRESS_MAP_START( namcostr_map, AS_PROGRAM, 16, seta2_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM                             // ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM                             // RAM
	AM_RANGE(0xc00000, 0xc3ffff) AM_RAM AM_SHARE("spriteram")       // Sprites
	AM_RANGE(0xc60000, 0xc6003f) AM_WRITE(vregs_w) AM_SHARE("vregs")  // Video Registers
	AM_RANGE(0xfffc00, 0xffffff) AM_DEVREADWRITE("tmp68301", tmp68301_device, regs_r, regs_w)  // TMP68301 Registers
ADDRESS_MAP_END


/***************************************************************************
                            Sammy Outdoor Shooting
***************************************************************************/

WRITE16_MEMBER(seta2_state::samshoot_coin_w)
{
	if (ACCESSING_BITS_0_7)
	{
		coin_counter_w(machine(), 0, data & 0x10);
		coin_counter_w(machine(), 1, data & 0x20);
		// Are these connected? They are set in I/O test
		coin_lockout_w(machine(), 0,~data & 0x40);
		coin_lockout_w(machine(), 1,~data & 0x80);
	}
//  popmessage("%04x",data);
}

static ADDRESS_MAP_START( samshoot_map, AS_PROGRAM, 16, seta2_state )
	AM_RANGE( 0x000000, 0x1fffff ) AM_ROM
	AM_RANGE( 0x200000, 0x20ffff ) AM_RAM
	AM_RANGE( 0x300000, 0x30ffff ) AM_RAM AM_SHARE("nvram")

	AM_RANGE( 0x400000, 0x400001 ) AM_READ_PORT("DSW1")             // DSW 1
	AM_RANGE( 0x400002, 0x400003 ) AM_READ_PORT("BUTTONS")          // Buttons

	AM_RANGE( 0x400300, 0x40030f ) AM_WRITE(sound_bank_w )    // Samples Banks

	AM_RANGE( 0x500000, 0x500001 ) AM_READ_PORT("GUN1")             // P1
	AM_RANGE( 0x580000, 0x580001 ) AM_READ_PORT("GUN2")             // P2

	AM_RANGE( 0x700000, 0x700001 ) AM_READ_PORT("TRIGGER")          // Trigger
	AM_RANGE( 0x700002, 0x700003 ) AM_READ_PORT("PUMP")             // Pump
	AM_RANGE( 0x700004, 0x700005 ) AM_READ_PORT("COIN") AM_WRITE(samshoot_coin_w )  // Coins
	AM_RANGE( 0x700006, 0x700007 ) AM_READ(watchdog_reset16_r ) // Watchdog?

	AM_RANGE( 0x800000, 0x83ffff ) AM_RAM AM_SHARE("spriteram") // Sprites
	AM_RANGE( 0x840000, 0x84ffff ) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")  // Palette
	AM_RANGE( 0x860000, 0x86003f ) AM_WRITE(vregs_w) AM_SHARE("vregs")    // Video Registers

	AM_RANGE( 0x900000, 0x903fff ) AM_DEVREADWRITE("x1snd", x1_010_device, word_r, word_w)   // Sound

	AM_RANGE( 0xfffc00, 0xffffff ) AM_DEVREADWRITE("tmp68301", tmp68301_device, regs_r, regs_w)    // TMP68301 Registers
ADDRESS_MAP_END


/***************************************************************************
                               Funcube series
***************************************************************************/

// Touchscreen

#define MCFG_FUNCUBE_TOUCHSCREEN_ADD( _tag, _clock ) \
	MCFG_DEVICE_ADD( _tag, FUNCUBE_TOUCHSCREEN, _clock )

#define MCFG_FUNCUBE_TOUCHSCREEN_TX_CALLBACK(_devcb) \
	devcb = &funcube_touchscreen_device::set_tx_cb(*device, DEVCB_##_devcb);

class funcube_touchscreen_device : public device_t,
									public device_serial_interface
{
public:
	funcube_touchscreen_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual ioport_constructor device_input_ports() const override;
	template<class _Object> static devcb_base &set_tx_cb(device_t &device, _Object object) { return downcast<funcube_touchscreen_device &>(device).m_tx_cb.set_callback(object); }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual void tra_complete() override;
	virtual void tra_callback() override;

private:
	devcb_write_line m_tx_cb;
	required_ioport m_x;
	required_ioport m_y;
	required_ioport m_btn;

	UINT8 m_button_state;
	int m_serial_pos;
	UINT8 m_serial[4];
};

const device_type FUNCUBE_TOUCHSCREEN = &device_creator<funcube_touchscreen_device>;

static INPUT_PORTS_START( funcube_touchscreen )
	PORT_START("touch_btn")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME( "Touch Screen" )

	PORT_START("touch_x")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_X ) PORT_MINMAX(0,0x5c+1) PORT_CROSSHAIR(X, -(1.0 * 0x05d/0x5c), -1.0/0x5c, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(5) PORT_REVERSE

	PORT_START("touch_y")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_Y ) PORT_MINMAX(0,0x46+1) PORT_CROSSHAIR(Y, -(0xf0-8.0)/0xf0*0x047/0x46, -1.0/0x46, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(5) PORT_REVERSE
INPUT_PORTS_END

funcube_touchscreen_device::funcube_touchscreen_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, FUNCUBE_TOUCHSCREEN, "Funcube Touchscreen", tag, owner, clock, "funcube_touchscrene", __FILE__),
	device_serial_interface(mconfig, *this),
	m_tx_cb(*this),
	m_x(*this, "touch_x"),
	m_y(*this, "touch_y"),
	m_btn(*this, "touch_btn")
{
}

ioport_constructor funcube_touchscreen_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(funcube_touchscreen);
}

void funcube_touchscreen_device::device_start()
{
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_tra_rate(9600);
	m_button_state = 0x00;
	emu_timer *tm = timer_alloc(0);
	tm->adjust(attotime::from_ticks(1, clock()), 0, attotime::from_ticks(1, clock()));
	m_tx_cb.resolve_safe();

	save_item(NAME(m_button_state));
	save_item(NAME(m_serial_pos));
	save_item(NAME(m_serial));
	device_serial_interface::register_save_state(machine().save(), this);
}

void funcube_touchscreen_device::device_reset()
{
	m_serial_pos = 0;
	memset(m_serial, 0, sizeof(m_serial));
	m_tx_cb(1);
}

void funcube_touchscreen_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if(id) {
		device_serial_interface::device_timer(timer, id, param, ptr);
		return;
	}

	UINT8 button_state = m_btn->read();
	if(m_button_state != button_state) {
		m_button_state = button_state;
		m_serial[0] = button_state ? 0xfe : 0xfd;
		m_serial[1] = m_x->read();
		m_serial[2] = m_y->read();
		m_serial[3] = 0xff;
		m_serial_pos = 0;
		transmit_register_setup(m_serial[m_serial_pos++]);
	}
}

void funcube_touchscreen_device::tra_complete()
{
	if(m_serial_pos != 4)
		transmit_register_setup(m_serial[m_serial_pos++]);
}

void funcube_touchscreen_device::tra_callback()
{
	m_tx_cb(transmit_register_get_data_bit());
}


// Bus conversion functions:

// RAM shared with the sub CPU
READ32_MEMBER(seta2_state::funcube_nvram_dword_r)
{
	UINT16 val = m_nvram[offset];
	return ((val & 0xff00) << 8) | (val & 0x00ff);
}

WRITE32_MEMBER(seta2_state::funcube_nvram_dword_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_nvram[offset] = (m_nvram[offset] & 0xff00) | (data & 0x000000ff);
	}
	if (ACCESSING_BITS_16_23)
	{
		m_nvram[offset] = (m_nvram[offset] & 0x00ff) | ((data & 0x00ff0000) >> 8);
	}
}

WRITE16_MEMBER(seta2_state::spriteram16_word_w)
{
	COMBINE_DATA( &m_spriteram[offset] );
}

READ16_MEMBER(seta2_state::spriteram16_word_r)
{
	return m_spriteram[offset];
}

// Main CPU


READ32_MEMBER(seta2_state::funcube_debug_r)
{
	UINT32 ret = ioport("DEBUG")->read();

	// This bits let you move the crosshair in the inputs / touch panel test with a joystick
	if (!(m_screen->frame_number() % 3))
		ret |= 0x3f;

	return ret;
}

READ32_MEMBER(seta2_state::oki_read)
{
	return m_oki->read_status() << 16;
}
WRITE32_MEMBER(seta2_state::oki_write)
{
	if (ACCESSING_BITS_0_7)
	{
		const UINT8 tmp = (data & 0x000000ff);
		m_oki->write_TMP_register(tmp);
	}
	else if (ACCESSING_BITS_16_23)
	{
		const UINT8 cmd = (data & 0x00ff0000) >> 16;
		m_oki->write_command(cmd);
	}
}

static ADDRESS_MAP_START( funcube_map, AS_PROGRAM, 32, seta2_state )
	AM_RANGE( 0x00000000, 0x0007ffff ) AM_ROM
	AM_RANGE( 0x00200000, 0x0020ffff ) AM_RAM

	AM_RANGE( 0x00400000, 0x00400003 ) AM_READ(funcube_debug_r)
	AM_RANGE( 0x00400004, 0x00400007 ) AM_READ(watchdog_reset32_r ) AM_WRITENOP

	AM_RANGE( 0x00500000, 0x00500003 ) AM_READWRITE(oki_read, oki_write)

	AM_RANGE( 0x00800000, 0x0083ffff ) AM_READWRITE16(spriteram16_word_r,  spriteram16_word_w, 0xffffffff ) AM_SHARE("spriteram")
	AM_RANGE( 0x00840000, 0x0084ffff ) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")  // Palette
	AM_RANGE( 0x00860000, 0x0086003f ) AM_WRITE16(vregs_w, 0xffffffff ) AM_SHARE("vregs")

	AM_RANGE( 0x00c00000, 0x00c002ff ) AM_READWRITE(funcube_nvram_dword_r, funcube_nvram_dword_w )

	AM_RANGE(0xf0000000, 0xf00001ff) AM_DEVREADWRITE("maincpu_onboard", mcf5206e_peripheral_device, seta2_coldfire_regs_r, seta2_coldfire_regs_w) // technically this can be moved with MBAR
	AM_RANGE(0xffffe000, 0xffffffff ) AM_RAM    // SRAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( funcube2_map, AS_PROGRAM, 32, seta2_state )
	AM_RANGE( 0x00000000, 0x0007ffff ) AM_ROM
	AM_RANGE( 0x00200000, 0x0020ffff ) AM_RAM

	AM_RANGE( 0x00500000, 0x00500003 ) AM_READ(funcube_debug_r )
	AM_RANGE( 0x00500004, 0x00500007 ) AM_READ(watchdog_reset32_r ) AM_WRITENOP

	AM_RANGE( 0x00600000, 0x00600003 ) AM_READWRITE(oki_read, oki_write)

	AM_RANGE( 0x00800000, 0x0083ffff ) AM_READWRITE16(spriteram16_word_r,  spriteram16_word_w, 0xffffffff ) AM_SHARE("spriteram")
	AM_RANGE( 0x00840000, 0x0084ffff ) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE( 0x00860000, 0x0086003f ) AM_WRITE16(vregs_w, 0xffffffff ) AM_SHARE("vregs")

	AM_RANGE( 0x00c00000, 0x00c002ff ) AM_READWRITE(funcube_nvram_dword_r, funcube_nvram_dword_w )

	AM_RANGE(0xf0000000, 0xf00001ff) AM_DEVREADWRITE("maincpu_onboard", mcf5206e_peripheral_device, seta2_coldfire_regs_r, seta2_coldfire_regs_w) // technically this can be moved with MBAR
	AM_RANGE(0xffffe000, 0xffffffff ) AM_RAM    // SRAM
ADDRESS_MAP_END

// Sub CPU

static ADDRESS_MAP_START( funcube_sub_map, AS_PROGRAM, 16, seta2_state )
	AM_RANGE( 0x000000, 0x01ffff ) AM_ROM
	AM_RANGE( 0x200000, 0x20017f ) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END




// Simulate coin drop through two sensors

#define FUNCUBE_SUB_CPU_CLOCK (XTAL_14_7456MHz)

READ16_MEMBER(seta2_state::funcube_coins_r)
{
	UINT8 ret = ioport("SWITCH")->read();
	UINT8 coin_bit0 = 1;    // active low
	UINT8 coin_bit1 = 1;

	UINT8 hopper_bit = (m_funcube_hopper_motor && !(m_screen->frame_number()%20)) ? 1 : 0;

	const UINT64 coin_total_cycles = FUNCUBE_SUB_CPU_CLOCK / (1000/20);

	if ( m_funcube_coin_start_cycles )
	{
		UINT64 elapsed = downcast<cpu_device *>(&space.device())->total_cycles() - m_funcube_coin_start_cycles;

		if ( elapsed < coin_total_cycles/2 )
			coin_bit0 = 0;
		else if ( elapsed < coin_total_cycles )
			coin_bit1 = 0;
		else
			m_funcube_coin_start_cycles = 0;
	}
	else
	{
		if (!(ret & 1))
			m_funcube_coin_start_cycles = downcast<cpu_device *>(&space.device())->total_cycles();
	}

	return (ret & ~7) | (hopper_bit << 2) | (coin_bit1 << 1) | coin_bit0;
}

void seta2_state::funcube_debug_outputs()
{
#ifdef MAME_DEBUG
//  popmessage("LED: %02x OUT: %02x", (int)*m_funcube_leds, (int)*m_funcube_outputs);
#endif
}

WRITE16_MEMBER(seta2_state::funcube_leds_w)
{
	*m_funcube_leds = data;

	set_led_status( machine(), 0, (~data) & 0x01 ); // win lamp (red)
	set_led_status( machine(), 1, (~data) & 0x02 ); // win lamp (green)

	// Set in a moving pattern: 0111 -> 1011 -> 1101 -> 1110
	set_led_status( machine(), 2, (~data) & 0x10 );
	set_led_status( machine(), 3, (~data) & 0x20 );
	set_led_status( machine(), 4, (~data) & 0x40 );
	set_led_status( machine(), 5, (~data) & 0x80 );

	funcube_debug_outputs();
}

READ16_MEMBER(seta2_state::funcube_outputs_r)
{
	// Bits 1,2,3 read
	return *m_funcube_outputs;
}

WRITE16_MEMBER(seta2_state::funcube_outputs_w)
{
	*m_funcube_outputs = data;

	// Bits 0,1,3 written

	// Bit 0: hopper motor
	m_funcube_hopper_motor = (~data) & 0x01;

	// Bit 1: high on pay out

	// Bit 3: low after coining up, blinks on pay out
	set_led_status( machine(), 6, (~data) & 0x08 );

	funcube_debug_outputs();
}

READ16_MEMBER(seta2_state::funcube_battery_r)
{
	return ioport("BATTERY")->read() ? 0x40 : 0x00;
}

// cabinet linking on sci0
static ADDRESS_MAP_START( funcube_sub_io, AS_IO, 16, seta2_state )
	AM_RANGE( h8_device::PORT_7,   h8_device::PORT_7   )    AM_READ(funcube_coins_r )
	AM_RANGE( h8_device::PORT_4,   h8_device::PORT_4   )    AM_READ(funcube_battery_r )
	AM_RANGE( h8_device::PORT_A,   h8_device::PORT_A   )    AM_READWRITE(funcube_outputs_r, funcube_outputs_w ) AM_SHARE("funcube_outputs")
	AM_RANGE( h8_device::PORT_B,   h8_device::PORT_B   )    AM_WRITE(funcube_leds_w )                           AM_SHARE("funcube_leds")
ADDRESS_MAP_END

static ADDRESS_MAP_START( funcube2_sub_io, AS_IO, 16, seta2_state )
	AM_RANGE( h8_device::PORT_7,   h8_device::PORT_7   )    AM_READ(funcube_coins_r )
	AM_RANGE( h8_device::PORT_4,   h8_device::PORT_4   )    AM_NOP  // unused
	AM_RANGE( h8_device::PORT_A,   h8_device::PORT_A   )    AM_READWRITE(funcube_outputs_r, funcube_outputs_w ) AM_SHARE("funcube_outputs")
	AM_RANGE( h8_device::PORT_B,   h8_device::PORT_B   )    AM_WRITE(funcube_leds_w )                           AM_SHARE("funcube_leds")
ADDRESS_MAP_END




/***************************************************************************

                                Input Ports

***************************************************************************/

/***************************************************************************
                        Mobile Suit Gundam EX Revue
***************************************************************************/

static INPUT_PORTS_START( gundamex )
	PORT_START("DSW1")  // $600000.w
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0006, 0x0006, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW1:4" ) /* Listed as "Unused" */
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Freeze" ) PORT_DIPLOCATION("SW1:6")  /* Listed as "Unused" */
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Show Targets" ) PORT_DIPLOCATION("SW1:7") /* Listed as "Unused" */
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")  // $600002.w
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0000, "3 Coins/5 Credits" )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Debug Mode" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")    // $700000.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")    // $700002.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")    // $700004.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN ) //jumper pad
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Language ) )          //jumper pad
	PORT_DIPSETTING(      0x0020, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Japanese ) )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")   // $700008.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")   // $70000a.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/***************************************************************************
                                Guardians
***************************************************************************/

static INPUT_PORTS_START( grdians )
	PORT_START("DSW1")  // $600000.w
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy )    )  // 0
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal )  )  // 1
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard )    )  // 2
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )  // 3
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Title" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, "Guardians" )
	PORT_DIPSETTING(      0x0000, "Denjin Makai II" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0020, "1" )
	PORT_DIPSETTING(      0x0030, "2" )
	PORT_DIPSETTING(      0x0010, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_SERVICE_DIPLOC(  0x0040, IP_ACTIVE_LOW, "SW1:7" ) /* NOTE: Test mode shows player 3 & 4 controls, but it's a two player game */
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")  // $600002.w
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(      0x0002, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x00f0, 0x00f0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(      0x0020, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0050, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x00f0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0070, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x00d0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x00b0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0090, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")    // $700000.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")    // $700002.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")    // $700004.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                      Wakakusamonogatari Mahjong Yonshimai
***************************************************************************/

static INPUT_PORTS_START( mj4simai )
	PORT_START("DSW1")  // $600300.w
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Tumo Pin" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")  // $600302.w
	PORT_DIPNAME( 0x0007, 0x0004, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0004, "0" )
	PORT_DIPSETTING(      0x0003, "1" )
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0001, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPSETTING(      0x0007, "5" )
	PORT_DIPSETTING(      0x0006, "6" )
	PORT_DIPSETTING(      0x0005, "7" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0010, 0x0000, "Select Girl" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0020, 0x0000, "Com Put" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")    // $600100.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_KEY0")   // $600000(0)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_KEY1")   // $600000(1)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("P1_KEY2")   // $600000(2)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xffe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_KEY3")   // $600000(3)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_KEY4")   // $600000(4)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0x00c0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2_KEY0")   // $600000(0)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2_KEY1")   // $600000(1)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("P2_KEY2")   // $600000(2)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0xffe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2_KEY3")   // $600000(3)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2_KEY4")   // $600000(4)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(2)
	PORT_BIT( 0x00c0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                            Kosodate Quiz My Angel
***************************************************************************/

static INPUT_PORTS_START( myangel )
	PORT_START("DSW1")  // $700300.w
	PORT_SERVICE_DIPLOC(  0x0001, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0002, "SW1:2" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW1:3" ) /* Listed as "Unused" */
	PORT_DIPNAME( 0x0008, 0x0008, "Increase Lives While Playing" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0020, "2" )
	PORT_DIPSETTING(      0x0030, "3" )
	PORT_DIPSETTING(      0x0010, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")  // $700302.w
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(      0x0002, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW2:5" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW2:6" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:7" ) /* Listed as "Unused" */
	PORT_DIPNAME( 0x0080, 0x0080, "Push Start To Freeze (Cheat)") PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1") //$700000.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2") //$700002.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM") //$700004.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                            Kosodate Quiz My Angel 2
***************************************************************************/

static INPUT_PORTS_START( myangel2 )
	PORT_START("DSW1") //$600300.w
	PORT_SERVICE_DIPLOC(  0x0001, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0002, "SW1:2" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW1:3" ) /* Listed as "Unused" */
	PORT_DIPNAME( 0x0008, 0x0008, "Increase Lives While Playing" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0020, "2" )
	PORT_DIPSETTING(      0x0030, "3" )
	PORT_DIPSETTING(      0x0010, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT(     0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2") //$600302.w
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(      0x0002, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW2:5" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW2:6" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:7" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW2:8" ) /* Listed as "Unused" */
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1") //$600000.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2") //$600002.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM") //$600004.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                                Puzzle De Bowling
***************************************************************************/

static INPUT_PORTS_START( pzlbowl )
	PORT_START("DSW1") //$400300.w
	PORT_SERVICE_DIPLOC(  0x0001, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0030, DEF_STR( Easiest ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( Easier ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Harder ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, "Winning Rounds (Player VS Player)" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x0040, "1" )
	PORT_DIPSETTING(      0x00c0, "2" )     /* This setting is not defined in the manual */
	PORT_DIPSETTING(      0x0080, "3" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2") //$400302.w
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_2C ) )
//  PORT_DIPSETTING(      0x0002, DEF_STR( 1C_1C ) )        /* This setting is not defined in the manual */
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_3C ) )
//  PORT_DIPSETTING(      0x0001, DEF_STR( 1C_3C ) )        /* This setting is not defined in the manual */
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Join In" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:7" )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Language ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Japanese ) )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1") //$500000.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2") //$500002.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM") //$500004.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)   // unused, test mode shows it
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_HIGH, IPT_SPECIAL )    // Protection?
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                            Penguin Bros
***************************************************************************/

static INPUT_PORTS_START( penbros )
	PORT_START("DSW1") //$500300.w
	PORT_SERVICE_DIPLOC(  0x0001, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW1:3" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0020, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x0080, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_2C ) )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2") //$500302.w
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x000c, "3" )
	PORT_DIPSETTING(      0x0004, "4" )
	PORT_DIPSETTING(      0x0008, "5" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0010, "150k and 500k" )
	PORT_DIPSETTING(      0x0030, "200k and 700k" )
	PORT_DIPSETTING(      0x0000, "Every 250k" )    // no extra life after the one at 1500k
	PORT_DIPSETTING(      0x0020, DEF_STR( None ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, "Winning Rounds (Player VS Player)" ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0x00c0, "2" )
	PORT_DIPSETTING(      0x0040, "3" )
	PORT_DIPSETTING(      0x0080, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1") //$600000.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* Player 1 button 3 is unused */
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2") //$600002.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* Player 2 button 3 is unused */
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM") //$600004.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)   // unused, test mode shows it
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                              Reel'N Quake
***************************************************************************/

static INPUT_PORTS_START( reelquak )
	PORT_START("DSW1")  // $400300.w
	PORT_DIPNAME( 0x0001, 0x0001, "Game Style" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Standard ) )
	PORT_DIPSETTING(      0x0000, "Redemption" )
	PORT_DIPNAME( 0x000e, 0x000e, "Key-In Credits" ) PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(      0x000c, "1 Turn / 2 Credits" )
	PORT_DIPSETTING(      0x000a, "1 Turn / 3 Credits" )
	PORT_DIPSETTING(      0x0008, "1 Turn / 5 Credits" )
	PORT_DIPSETTING(      0x000e, "1 Turn / 10 Credits" )
	PORT_DIPSETTING(      0x0006, "1 Turn / 20 Credits" )
	PORT_DIPSETTING(      0x0004, "1 Turn / 25 Credits" )
	PORT_DIPSETTING(      0x0002, "1 Turn / 50 Credits" )
	PORT_DIPSETTING(      0x0000, "1 Turn / 100 Credits" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:7,8")   // bit 7 tested according to game style
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, "1 Coin/10 Credits" )

	PORT_START("DSW2")  // $400302.w    PORT_START("DSW2")  // $400302.w
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:1")  // used
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("P1")    // $400001.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_POKER_CANCEL  )                    // bet cancel
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE   )                    // collect
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP   )                    // double up
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH   ) PORT_NAME("Big")   // big
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_BET     )                    // bet
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_GAMBLE_LOW    ) PORT_NAME("Small") // small
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL   )                    // start
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN       )

	PORT_START("TICKET")    // $400003.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SPECIAL       ) PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r)    // ticket sensor
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Knock Down")    // knock down
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE2      ) PORT_NAME("Ticket Clear")  // ticket clear
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE3      ) PORT_NAME("Ticket Resume") // ticket resume
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN  )                            // key in
	PORT_SERVICE_NO_TOGGLE(0x0080, IP_ACTIVE_LOW       )                            // test mode

	PORT_START("SYSTEM")    // $400005.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1         ) PORT_IMPULSE(5)    // coin a
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2         ) PORT_IMPULSE(5)    // coin b
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1      )                    // service
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK   )                    // diagnostic
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN       )
INPUT_PORTS_END


/***************************************************************************
                              Endless Riches
***************************************************************************/

static INPUT_PORTS_START( endrichs )
	PORT_INCLUDE(reelquak)

	PORT_MODIFY("DSW1")  // $400300.w
	PORT_DIPNAME( 0x0001, 0x0001, "Payout Style" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, "Normal Payout" )
	PORT_DIPSETTING(      0x0000, "Ticket Payout" ) // Ticket Printer?

	PORT_MODIFY("DSW2")  // $400302.w
	PORT_DIPUNUSED_DIPLOC( 0x0001, IP_ACTIVE_LOW, "SW2:1" ) // DSW2 unpopulated
	PORT_DIPUNUSED_DIPLOC( 0x0002, IP_ACTIVE_LOW, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END


/***************************************************************************
                            Sammy Outdoor Shooting
***************************************************************************/

static INPUT_PORTS_START( deerhunt )
	PORT_START("DSW1") // $400000.w
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0005, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0028, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Discount To Continue" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_BIT(     0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2") // fffd0a.w
	PORT_DIPNAME( 0x0001, 0x0001, "Vert. Flip Screen" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Horiz. Flip Screen" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Blood Color" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0020, "Red" )
	PORT_DIPSETTING(      0x0000, "Yellow" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0080, 0x0080, "Gun Type" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, "Pump Action" )
	PORT_DIPSETTING(      0x0000, "Hand Gun" )
	PORT_BIT(     0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("GUN1") // $500000
	PORT_BIT( 0x00ff, 0x0080, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1, 0, 0) PORT_MINMAX(0x0025,0x00c5) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_BIT( 0xff00, 0x8000, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1, 0, 0) PORT_MINMAX(0x0800,0xf800) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("GUN2")  // $580000.b
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )  // P2 gun, read but not used

	PORT_START("TRIGGER")   // $700000
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SPECIAL )  // trigger
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0xff3f, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PUMP")  // $700003.b
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SPECIAL )  // pump
	PORT_BIT( 0xffbf, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COIN")  // $700005.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BUTTONS")   // $400002
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 )  // trigger
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 )  // pump
	PORT_BIT( 0xfffc, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( turkhunt )
	PORT_INCLUDE(deerhunt)

	PORT_MODIFY("DSW2") // fffd0a.w
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
INPUT_PORTS_END


static INPUT_PORTS_START( wschamp )
	PORT_INCLUDE(deerhunt)

	PORT_MODIFY("DSW1") // $400000.w
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(      0x0009, "4 Coins Start, 4 Coins Continue" )
	PORT_DIPSETTING(      0x0008, "4 Coins Start, 3 Coins Continue" )
	PORT_DIPSETTING(      0x0007, "4 Coins Start, 2 Coins Continue" )
	PORT_DIPSETTING(      0x0006, "4 Coins Start, 1 Coin Continue" )
	PORT_DIPSETTING(      0x000c, "3 Coins Start, 3 Coins Continue" )
	PORT_DIPSETTING(      0x000b, "3 Coins Start, 2 Coins Continue" )
	PORT_DIPSETTING(      0x000a, "3 Coins Start, 1 Coin Continue" )
	PORT_DIPSETTING(      0x000e, "2 Coins Start, 2 Coins Continue" )
	PORT_DIPSETTING(      0x000d, "2 Coins Start, 1 Coin Continue" )
	PORT_DIPSETTING(      0x000f, "1 Coin Start, 1 Coin Continue" )
	PORT_DIPSETTING(      0x0005, "1 Coin 2 Credits, 1 Credit Start & Continue" )
	PORT_DIPSETTING(      0x0004, "1 Coin 3 Credits, 1 Credit Start & Continue" )
	PORT_DIPSETTING(      0x0003, "1 Coin 4 Credits, 1 Credit Start & Continue" )
	PORT_DIPSETTING(      0x0002, "1 Coin 5 Credits, 1 Credit Start & Continue" )
	PORT_DIPSETTING(      0x0001, "1 Coin 6 Credits, 1 Credit Start & Continue" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW1:7" )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_BIT(     0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW2") // fffd0a.w
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW2:6" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, "2" )
	PORT_DIPSETTING(      0x0000, "3" )

	PORT_MODIFY("GUN2") // $580000
	PORT_BIT( 0x00ff, 0x0080, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1, 0, 0) PORT_MINMAX(0x0025,0x00c5) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_BIT( 0xff00, 0x8000, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1, 0, 0) PORT_MINMAX(0x0800,0xf800) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_MODIFY("TRIGGER")  // $700000
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SPECIAL )  // trigger P2
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SPECIAL )  // trigger P1
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0xff1f, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("PUMP") // $700003.b
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SPECIAL )  // pump P2
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SPECIAL )  // pump P1
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT( 0xff1f, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("COIN") // $700005.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("BUTTONS")  // $400002
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 )  // trigger P1
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 )  // pump P1
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )  PORT_PLAYER(2)  // trigger P2
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )  PORT_PLAYER(2)  // pump P2
	PORT_BIT( 0xffcc, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( trophyh )
	PORT_INCLUDE(wschamp)

	PORT_MODIFY("DSW2") // fffd0a.w
	PORT_DIPNAME( 0x0020, 0x0020, "Blood Color" ) PORT_DIPLOCATION("SW2:6") /* WSChamp doesn't use Blood Color, so add it back in */
	PORT_DIPSETTING(      0x0020, "Red" )
	PORT_DIPSETTING(      0x0000, "Yellow" )
INPUT_PORTS_END


/***************************************************************************
                               Funcube series
***************************************************************************/

static INPUT_PORTS_START( funcube )
	PORT_START("SWITCH")    // c00030.l
	PORT_BIT(     0x01, IP_ACTIVE_LOW,  IPT_COIN1    ) PORT_IMPULSE(1)  // coin solenoid 1
	PORT_BIT(     0x02, IP_ACTIVE_HIGH, IPT_SPECIAL  )                  // coin solenoid 2
	PORT_BIT(     0x04, IP_ACTIVE_HIGH, IPT_SPECIAL  )                  // hopper sensor
	PORT_BIT(     0x08, IP_ACTIVE_LOW,  IPT_BUTTON2  )                  // game select
	PORT_BIT(     0x10, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT )
	PORT_BIT(     0x20, IP_ACTIVE_LOW,  IPT_SERVICE1 ) PORT_NAME( "Reset Key" )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW   )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW   )

	PORT_START("BATTERY")
	PORT_DIPNAME( 0x10, 0x10, "Battery" )
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x10, DEF_STR( On ) )

	PORT_START("DEBUG")
	// 500002.w
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(2)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(2)

	// 500000.w
	PORT_DIPNAME(    0x00010000, 0x00000000, "Debug 0" )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00010000, DEF_STR( On ) )
	PORT_DIPNAME(    0x00020000, 0x00000000, "Debug 1" )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00020000, DEF_STR( On ) )
	PORT_DIPNAME(    0x00040000, 0x00000000, "Debug 2" )    // Touch-Screen
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00040000, DEF_STR( On ) )
	PORT_DIPNAME(    0x00080000, 0x00000000, "Debug 3" )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00080000, DEF_STR( On ) )
	PORT_DIPNAME(    0x00100000, 0x00000000, "Debug 4" )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00100000, DEF_STR( On ) )
	PORT_DIPNAME(    0x00200000, 0x00000000, "Debug 5" )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00200000, DEF_STR( On ) )
	PORT_DIPNAME(    0x00400000, 0x00000000, "Debug 6" )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00400000, DEF_STR( On ) )
	PORT_DIPNAME(    0x00800000, 0x00000000, "Debug 7" )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00800000, DEF_STR( On ) )
INPUT_PORTS_END


/***************************************************************************


                            Graphics Layouts


***************************************************************************/

static const gfx_layout layout_4bpp_lo =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{   RGN_FRAC(1,4)+8,RGN_FRAC(1,4)+0,
		RGN_FRAC(0,4)+8,RGN_FRAC(0,4)+0     },
	{   STEP8(0,1)      },
	{   STEP8(0,8*2)    },
	8*8*2
};

static const gfx_layout layout_4bpp_hi =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{   RGN_FRAC(3,4)+8,RGN_FRAC(3,4)+0,
		RGN_FRAC(2,4)+8,RGN_FRAC(2,4)+0     },
	{   STEP8(0,1)      },
	{   STEP8(0,8*2)    },
	8*8*2
};

static const gfx_layout layout_6bpp =
{
	8,8,
	RGN_FRAC(1,4),
	6,
	{
		RGN_FRAC(2,4)+8,RGN_FRAC(2,4)+0,
		RGN_FRAC(1,4)+8,RGN_FRAC(1,4)+0,
		RGN_FRAC(0,4)+8,RGN_FRAC(0,4)+0     },
	{   STEP8(0,1)      },
	{   STEP8(0,8*2)    },
	8*8*2
};

static const gfx_layout layout_8bpp =
{
	8,8,
	RGN_FRAC(1,4),
	8,
	{   RGN_FRAC(3,4)+8,RGN_FRAC(3,4)+0,
		RGN_FRAC(2,4)+8,RGN_FRAC(2,4)+0,
		RGN_FRAC(1,4)+8,RGN_FRAC(1,4)+0,
		RGN_FRAC(0,4)+8,RGN_FRAC(0,4)+0     },
	{   STEP8(0,1)      },
	{   STEP8(0,8*2)    },
	8*8*2
};

static const gfx_layout layout_3bpp_lo =
{
	8,8,
	RGN_FRAC(1,4),
	3,
	{                   RGN_FRAC(1,4)+0,
		RGN_FRAC(0,4)+8,RGN_FRAC(0,4)+0     },
	{   STEP8(0,1)      },
	{   STEP8(0,8*2)    },
	8*8*2
};

static const gfx_layout layout_2bpp_hi =
{
	8,8,
	RGN_FRAC(1,4),
	2,
	{   RGN_FRAC(2,4)+8,RGN_FRAC(2,4)+0 },
	{   STEP8(0,1)      },
	{   STEP8(0,8*2)    },
	8*8*2
};

/*  Tiles are 8bpp, but the hardware is additionally able to discard
    some bitplanes and use the low 4 bits only, or the high 4 bits only */
static GFXDECODE_START( seta2 )
	GFXDECODE_ENTRY( "sprites", 0, layout_4bpp_lo, 0, 0x8000/16 )
	GFXDECODE_ENTRY( "sprites", 0, layout_4bpp_hi, 0, 0x8000/16 )
	GFXDECODE_ENTRY( "sprites", 0, layout_6bpp,    0, 0x8000/16 )   // 6bpp, but 4bpp color granularity
	GFXDECODE_ENTRY( "sprites", 0, layout_8bpp,    0, 0x8000/16 )   // 8bpp, but 4bpp color granularity
	GFXDECODE_ENTRY( "sprites", 0, layout_3bpp_lo, 0, 0x8000/16 )   // 3bpp, but 4bpp color granularity
	GFXDECODE_ENTRY( "sprites", 0, layout_2bpp_hi, 0, 0x8000/16 )   // 2bpp, but 4bpp color granularity
GFXDECODE_END

/***************************************************************************
                               Funcube series
***************************************************************************/

static const gfx_layout funcube_layout_4bpp_lo =
{
	8,8,
	RGN_FRAC(1,1),
	4,
//  { STEP4(7*8, -8) },
	{ STEP4(0*8, 8) },  // needed by funcube3 text
	{ STEP8(0, 1) },
	{ STEP8(0, 8*8) },
	8*8*8
};

static const gfx_layout funcube_layout_4bpp_hi =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(4*8, -8) },
	{ STEP8(0, 1) },
	{ STEP8(0, 8*8) },
	8*8*8
};

static const gfx_layout funcube_layout_6bpp =
{
	8,8,
	RGN_FRAC(1,1),
	6,
	{ STEP4(7*8, -8), STEP2(3*8, -8) },
	{ STEP8(0, 1) },
	{ STEP8(0, 8*8) },
	8*8*8
};

static const gfx_layout funcube_layout_8bpp =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ STEP8(7*8, -8) },
	{ STEP8(0, 1) },
	{ STEP8(0, 8*8) },
	8*8*8
};

static const gfx_layout funcube_layout_3bpp_lo =
{
	8,8,
	RGN_FRAC(1,1),
	3,
	{ 7*8,6*8,5*8 },
	{ STEP8(0, 1) },
	{ STEP8(0, 8*8) },
	8*8*8
};

static const gfx_layout funcube_layout_2bpp_hi =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ STEP2(5*8, -8) },
	{ STEP8(0, 1) },
	{ STEP8(0, 8*8) },
	8*8*8
};

/*  Tiles are 8bpp, but the hardware is additionally able to discard
    some bitplanes and use the low 4 bits only, or the high 4 bits only */
static GFXDECODE_START( funcube )
	GFXDECODE_ENTRY( "sprites", 0, funcube_layout_4bpp_lo, 0, 0x8000/16 )
	GFXDECODE_ENTRY( "sprites", 0, funcube_layout_4bpp_hi, 0, 0x8000/16 )
	GFXDECODE_ENTRY( "sprites", 0, funcube_layout_6bpp,    0, 0x8000/16 )   // 6bpp, but 4bpp color granularity
	GFXDECODE_ENTRY( "sprites", 0, funcube_layout_8bpp,    0, 0x8000/16 )   // 8bpp, but 4bpp color granularity
	GFXDECODE_ENTRY( "sprites", 0, funcube_layout_3bpp_lo, 0, 0x8000/16 )   // 3bpp, but 4bpp color granularity
	GFXDECODE_ENTRY( "sprites", 0, funcube_layout_2bpp_hi, 0, 0x8000/16 )   // 2bpp, but 4bpp color granularity
GFXDECODE_END


/***************************************************************************

                                Machine Drivers

***************************************************************************/

INTERRUPT_GEN_MEMBER(seta2_state::seta2_interrupt)
{
	/* VBlank is connected to INT0 (external interrupts pin 0) */
	m_tmp68301->external_interrupt_0();
}

INTERRUPT_GEN_MEMBER(seta2_state::samshoot_interrupt)
{
	m_tmp68301->external_interrupt_2();   // to do: hook up x1-10 interrupts
}

static MACHINE_CONFIG_START( seta2, seta2_state )
	MCFG_CPU_ADD("maincpu", M68301, XTAL_50MHz/3)   // !! TMP68301 !!
	MCFG_CPU_PROGRAM_MAP(mj4simai_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", seta2_state,  seta2_interrupt)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("tmp68301",tmp68301_device,irq_callback)

	MCFG_DEVICE_ADD("tmp68301", TMP68301, 0)

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(0x200, 0x200)
	MCFG_SCREEN_VISIBLE_AREA(0x40, 0x1c0-1, 0x80, 0x170-1)
	MCFG_SCREEN_UPDATE_DRIVER(seta2_state, screen_update)
	MCFG_SCREEN_VBLANK_DRIVER(seta2_state, screen_eof)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", seta2)
	MCFG_PALETTE_ADD("palette", 0x8000+0xf0)    // extra 0xf0 because we might draw 256-color object with 16-color granularity
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("x1snd", X1_010, XTAL_50MHz/3)   // clock?
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( mj4simai, seta2 )
	MCFG_MACHINE_START_OVERRIDE(seta2_state, mj4simai)

MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( gundamex, seta2 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(gundamex_map)

	MCFG_DEVICE_MODIFY("tmp68301")
	MCFG_TMP68301_IN_PARALLEL_CB(READ16(seta2_state, gundamex_eeprom_r))
	MCFG_TMP68301_OUT_PARALLEL_CB(WRITE16(seta2_state, gundamex_eeprom_w))

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	// video hardware
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0x00, 0x180-1, 0x100, 0x1e0-1)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( grdians, seta2 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(grdians_map)

	// video hardware
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0x80, 0x80 + 0x130 -1, 0x80, 0x80 + 0xe8 -1)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( myangel, seta2 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(myangel_map)

	// video hardware
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, 0x178-1, 0x00, 0xf0-1)

	MCFG_VIDEO_START_OVERRIDE(seta2_state,yoffset)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( myangel2, seta2 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(myangel2_map)

	// video hardware
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, 0x178-1, 0x00, 0xf0-1)

	MCFG_VIDEO_START_OVERRIDE(seta2_state,yoffset)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( pzlbowl, seta2 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(pzlbowl_map)

	// video hardware
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0x10, 0x190-1, 0x100, 0x1f0-1)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( penbros, seta2 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(penbros_map)

	// video hardware
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, 0x140-1, 0x80, 0x160-1)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( reelquak, seta2 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(reelquak_map)

	MCFG_DEVICE_MODIFY("tmp68301")
	MCFG_TMP68301_OUT_PARALLEL_CB(WRITE16(seta2_state, reelquak_leds_w))

	MCFG_NVRAM_ADD_0FILL("nvram")
	MCFG_TICKET_DISPENSER_ADD("ticket", attotime::from_msec(200), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_LOW)

	// video hardware
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0x40, 0x180-1, 0x80, 0x170-1)

	MCFG_VIDEO_START_OVERRIDE(seta2_state,xoffset)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( samshoot, seta2 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(samshoot_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(seta2_state, samshoot_interrupt, 60)

	MCFG_DEVICE_MODIFY("tmp68301")
	MCFG_TMP68301_IN_PARALLEL_CB(IOPORT("DSW2"))

	MCFG_NVRAM_ADD_0FILL("nvram")

	// video hardware
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0x40, 0x180-1, 0x40, 0x130-1)
MACHINE_CONFIG_END


/***************************************************************************
                               Funcube series
***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(seta2_state::funcube_interrupt)
{
	int scanline = param;

	if(scanline == 368)
		m_maincpu->set_input_line(1, HOLD_LINE);

	if(scanline == 0)
		m_maincpu->set_input_line(2, HOLD_LINE);
}

MACHINE_START_MEMBER(seta2_state, funcube)
{
	save_item(NAME(m_funcube_coin_start_cycles));
	save_item(NAME(m_funcube_hopper_motor));
}

MACHINE_RESET_MEMBER(seta2_state, funcube)
{
	m_funcube_coin_start_cycles = 0;
	m_funcube_hopper_motor = 0;
}

static MACHINE_CONFIG_START( funcube, seta2_state )

	MCFG_CPU_ADD("maincpu", MCF5206E, XTAL_25_447MHz)
	MCFG_CPU_PROGRAM_MAP(funcube_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", seta2_state, funcube_interrupt, "screen", 0, 1)

	MCFG_CPU_ADD("sub", H83007, FUNCUBE_SUB_CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(funcube_sub_map)
	MCFG_CPU_IO_MAP(funcube_sub_io)

	MCFG_MCF5206E_PERIPHERAL_ADD("maincpu_onboard")

	MCFG_FUNCUBE_TOUCHSCREEN_ADD("touchscreen", 200)
	MCFG_FUNCUBE_TOUCHSCREEN_TX_CALLBACK(DEVWRITELINE(":sub:sci1", h8_sci_device, rx_w))

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_MACHINE_START_OVERRIDE(seta2_state, funcube)
	MCFG_MACHINE_RESET_OVERRIDE(seta2_state, funcube)

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))  // not accurate
	MCFG_SCREEN_SIZE(0x200, 0x200)
	MCFG_SCREEN_VISIBLE_AREA(0x0+1, 0x140-1+1, 0x80, 0x170-1)
	MCFG_SCREEN_UPDATE_DRIVER(seta2_state, screen_update)
	MCFG_SCREEN_VBLANK_DRIVER(seta2_state, screen_eof)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", funcube)
	MCFG_PALETTE_ADD("palette", 0x8000+0xf0)    // extra 0xf0 because we might draw 256-color object with 16-color granularity
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	// sound hardware
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM9810_ADD("oki", XTAL_4_096MHz)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.80)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.80)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( funcube2, funcube )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(funcube2_map)

	MCFG_CPU_MODIFY("sub")
	MCFG_CPU_IO_MAP(funcube2_sub_io)

	// video hardware
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0x0, 0x140-1, 0x80, 0x170-1)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( funcube3, funcube2 )
	// video hardware
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0x0, 0x140-1, 0x80-0x40, 0x170-1-0x40)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( namcostr, seta2_state )
	MCFG_CPU_ADD("maincpu", M68301, XTAL_50MHz/3)   // !! TMP68301 !!
	MCFG_CPU_PROGRAM_MAP(namcostr_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", seta2_state,  seta2_interrupt)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("tmp68301",tmp68301_device,irq_callback)

	MCFG_DEVICE_ADD("tmp68301", TMP68301, 0)  // does this have a ticket dispenser?

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(0x200, 0x200)
	MCFG_SCREEN_VISIBLE_AREA(0x40, 0x1c0-1, 0x80, 0x170-1)
	MCFG_SCREEN_UPDATE_DRIVER(seta2_state, screen_update)
	MCFG_SCREEN_VBLANK_DRIVER(seta2_state, screen_eof)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", funcube)
	MCFG_PALETTE_ADD("palette", 0x8000+0xf0)    // extra 0xf0 because we might draw 256-color object with 16-color granularity
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	// sound hardware
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM9810_ADD("oki", XTAL_4_096MHz)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.80)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.80)
MACHINE_CONFIG_END


/***************************************************************************

                                ROMs Loading

***************************************************************************/

/***************************************************************************

FUNCUBE
EVA2E PCB

It's the same PCB as Namco Stars (P0-140B). It's a lot more complicated to dump
than the others because there are several surface mounted flash ROMs spread across
multiple daughterboards instead of simple socketed 32M DIP42 mask roms all on one PCB.

***************************************************************************/

ROM_START( funcube )
	ROM_REGION( 0x80000, "maincpu", 0 ) // XCF5206 Code
	ROM_LOAD( "fcu1_prg0-f.u08", 0x00000, 0x80000, CRC(57f4f340) SHA1(436fc66409b254aba68ae33fc994bc270ce803a6) )

	ROM_REGION( 0x20000, "sub", 0 )     // H8/3007 Code
	ROM_LOAD( "fcu_0_iopr-0b.1b", 0x00000, 0x20000, CRC(87e3690f) SHA1(1b9dc573de31543884678df2dba2d6a74d6a2496) )

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD32_BYTE( "fcu1_obj-0a.u12", 0x000000, 0x200000, CRC(908b6baf) SHA1(cb5aa8c9b16abb17d8cc16d0d3b2f690a48ee503) )
	ROM_LOAD32_BYTE( "fcu1_obj-0a.u13", 0x000001, 0x200000, CRC(8c31ca21) SHA1(e497ab1d7d30b41928a0c3db1ea7c3420376ad8c) )
	ROM_LOAD32_BYTE( "fcu1_obj-0a.u14", 0x000002, 0x200000, CRC(4298d599) SHA1(d245206bc78de5f17da85ae6063b662cf9cf67aa) )
	ROM_LOAD32_BYTE( "fcu1_obj-0a.u15", 0x000003, 0x200000, CRC(0669c78e) SHA1(0158fc4f90efa12d795b97873b8646c352864c69) )

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASE00 )
	ROM_LOAD( "fcu1_snd-0a.u40", 0x000000, 0x200000, CRC(448539bc) SHA1(9e53bd5e29d1a88bf634e58bfeebccd3a1c2d866) )
ROM_END

/***************************************************************************

              FUNCUBE (BET) Series PCB for chapters 2 through 5

PCB Number: B0-006B (also known as EVA3_A system and is non-JAMMA)
+------------------------------------------------+
|+--+ S +---+ +---+                CN5           |
||  | W |   | |   |                          CN6?|
||  | 4 | U | | U |                              |
||  |   | 4 | | 4 |     +---+                CN2?|
||  |   | 2 | | 3 |     |DX |                    |
||  |   |   | |   |     |102|                    |
||C |   |   | |   |     +---+                    |
||N |   +---+ +---+                              |
||4 |                                            |
||  |      +----------+   M1                     |
||  |  M3  |          |                        C |
||  |      |   NEC    |   M1                   N |
||  |  M3  |  DX-101  |                        3 |
||  |      |          |                          |
||  |      |          |   50MHz                  |
|+--+      +----------+                          |
| PIC  25.447MHz         +-----------+           |
|  CN7                   |    U47    |           |
|                        +-----------+           |
|          +-----------+  +---+ +---+       D    |
|          |     U3    |  |OKI| |DX |       S    |
|    M2    +-----------+  |   | |102|       W    |
|                         +---+ +---+       1    |
|                 ispLSI2032                     |
|    M1                      +---+               |
|          +----------+      |IDT|           +--+|
|          |          |  C   |   |           |  ||
| C        | ColdFire |  N   +---+           |  ||
| N  M2    | XCF5206E |  8                   |  ||
| 1        |          |        +---+         |C ||
|          |          |        |H8 |         |N ||
|    M1    +----------+        +---+      D  |9 ||
|                         14.7456MHz      S  |  ||
|                            +-----------+W  |  ||
|            SW1      BAT1   |    U49    |2  +--+|
|                            +-----------+       |
+------------------------------------------------+

   CPU: ColdFire XCF5206EFT54 (160 Pin PQFP)
        Hitachi H8/3007 (64130007F20) used for touch screen I/O
 Video: NEC DX-101 (240 Pin PQFP)
        NEC DX-102 (52 Pin PQFP x2)
 Sound: OKI MSM9810B 8-Channel Mixing ADPCM Type Voice Synthesis LSI
   OSC: 50MHz, 25.447MHz & 14.7456MHz
 Other: Lattice ispLSI2032 - stamped "EVA3A"
        BAT1 - CR2032 3Volt

ColdFire XCF5206EFT54:
  68K/ColdFire V2 core family
  8K internal SRAM
  54MHz (max) Bus Frequency
  32bit External Bus Width
  2 UART Serial Interfaces
  2 Timer Channels

PIC - PIC12C508 MCU used for security
       Labeled FC21A for Funcube 2
       Labeled FC41A for Funcube 4

Ram M1 are Toshiba TC55257DFL-70L
Ram M2 are NEC D43001GU-70L
Ram M3 are ISSI IS62C1024L-70Q
IDT - IDT 7130 64-pin TQFP High-speed 1K x 8 Dual-Port Static RAM

CN1 - Unused 64 pin double row connecter
CN2?  2x2 connecter
CN3 - Unused 50 pin double row connecter
CN4 - 96 pin triple row connecter
CN5 - 2x3 pin connecter
CN6?  3x3 connecter
CN7 - Unused 20 pin connecter
CN8 - 8 pin single row connecter
CN9 - 40 pin double row connecter

DSW1 - 8 position dipswitch
DSW2 - 2 position dipswitch
SW1  - Pushbutton
SW4  - Single position slider switch

U3  - Is a 27C4002 EPROM
U49 - Is a 27C1001 EPROM
U42, U43 & U47 are MASK ROMs read as 27C322

The same H8/3007 code "FC21 IOPR-0" at U49 is used for FUNCUBE 2,3,4 & 5

***************************************************************************/

ROM_START( funcube2 )
	ROM_REGION( 0x80000, "maincpu", 0 ) // XCF5206 Code
	ROM_LOAD( "fc21_prg-0b.u3", 0x00000, 0x80000, CRC(add1c8a6) SHA1(bf91518da659098a4bad4e756533525fcc910570) )

	ROM_REGION( 0x20000, "sub", 0 )     // H8/3007 Code
	ROM_LOAD( "fc21_iopr-0.u49", 0x00000, 0x20000, CRC(314555ef) SHA1(b17e3926c8ef7f599856c198c330d2051aae13ad) )

	ROM_REGION( 0x300, "pic", 0 )       // PIC12C508? Code
	ROM_LOAD( "fc21a.u57", 0x000, 0x300, NO_DUMP )

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD32_WORD( "fc21_obj-0.u43", 0x000000, 0x400000, CRC(08cfe6d9) SHA1(d10f362dcde01f7a9855d8f76af3084b5dd1573a) )
	ROM_LOAD32_WORD( "fc21_obj-1.u42", 0x000002, 0x400000, CRC(4c1fbc20) SHA1(ff83691c19ce3600b31c494eaec26d2ac79e0028) )

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASE00 )
	ROM_LOAD( "fc21_voi0.u47", 0x000000, 0x200000, CRC(4a49370a) SHA1(ac10e2c25626965b49475767ef5a0ec3ba9a2d01) )
ROM_END

ROM_START( funcube3 )
	ROM_REGION( 0x80000, "maincpu", 0 ) // XCF5206 Code
	ROM_LOAD( "fc31_prg-0a.u4", 0x00000, 0x80000, CRC(ed7d70dd) SHA1(4ebfca9e60ab5e8de22821f0475abf515c83ce53) )

	ROM_REGION( 0x20000, "sub", 0 )     // H8/3007 Code
	ROM_LOAD( "fc21_iopr-0.u49", 0x00000, 0x20000, CRC(314555ef) SHA1(b17e3926c8ef7f599856c198c330d2051aae13ad) )

	ROM_REGION( 0x400, "pic", 0 )       // PIC12C508? Code
	ROM_LOAD( "fc31a.u57", 0x000, 0x400, NO_DUMP )

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD32_WORD( "fc31_obj-0.u43", 0x000000, 0x400000, CRC(08c5eb6f) SHA1(016d8f3067db487ccd47188142743897c9722b1f) )
	ROM_LOAD32_WORD( "fc31_obj-1.u42", 0x000002, 0x400000, CRC(4dadc76e) SHA1(cf82296b38dc22a618fd178816316af05f2459b3) )

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASE00 )
	ROM_LOAD( "fc31_snd-0.u47", 0x000000, 0x200000, CRC(36b03769) SHA1(20e583359421e0933c781a487fe5f7220052a6d4) )
ROM_END

ROM_START( funcube4 )
	ROM_REGION( 0x80000, "maincpu", 0 ) // XCF5206 Code
	ROM_LOAD( "fc41_prg-0.u3", 0x00000, 0x80000, CRC(ef870874) SHA1(dcb8dc3f780ca135df55e4b4f3c95620597ad28f) )

	ROM_REGION( 0x20000, "sub", 0 )     // H8/3007 Code
	ROM_LOAD( "fc21_iopr-0.u49", 0x00000, 0x20000, CRC(314555ef) SHA1(b17e3926c8ef7f599856c198c330d2051aae13ad) )

	ROM_REGION( 0x300, "pic", 0 )       // PIC12C508? Code
	ROM_LOAD( "fc41a", 0x000, 0x300, NO_DUMP )

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD32_WORD( "fc41_obj-0.u43", 0x000000, 0x400000, CRC(9ff029d5) SHA1(e057f4929aa745ecaf9d4ff7e39974c82e440146) )
	ROM_LOAD32_WORD( "fc41_obj-1.u42", 0x000002, 0x400000, CRC(5ab7b087) SHA1(c600158b2358cdf947357170044dda2deacd4f37) )

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASE00 )
	ROM_LOAD( "fc41_snd0.u47", 0x000000, 0x200000, CRC(e6f7d2bc) SHA1(638c73d439eaaff8097cb0aa2684f9f7111bcade) )
ROM_END

ROM_START( funcube5 )
	ROM_REGION( 0x80000, "maincpu", 0 ) // XCF5206 Code
	ROM_LOAD( "fc51_prg-0.u4", 0x00000, 0x80000, CRC(4e34c2d8) SHA1(1ace4f6edab291e69e5c36b15193fba62f4a6773) )

	ROM_REGION( 0x20000, "sub", 0 )     // H8/3007 Code
	ROM_LOAD( "fc21_iopr-0.u49", 0x00000, 0x20000, CRC(314555ef) SHA1(b17e3926c8ef7f599856c198c330d2051aae13ad) )

	ROM_REGION( 0x300, "pic", 0 )       // PIC12C508? Code
	ROM_LOAD( "fc51a.u57", 0x000, 0x300, NO_DUMP )

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD32_WORD( "fc51_obj-0.u43", 0x000000, 0x400000, CRC(116624b3) SHA1(c0b3dbe0ea4a0808222616c3ef77b2d1194a970a) )
	ROM_LOAD32_WORD( "fc51_obj-1.u42", 0x000002, 0x400000, CRC(35c6ec61) SHA1(424c9b66a2cdd5217d8a577d0179d1228112ee5b) )

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASE00 )
	ROM_LOAD( "fc51_snd-0.u47", 0x000000, 0x200000, CRC(2a504fe1) SHA1(911ad650bf48aa78d9cb3c64284aa526ceb519ba) )
ROM_END

DRIVER_INIT_MEMBER(seta2_state,funcube)
{
	UINT32 *main_cpu = (UINT32 *) memregion("maincpu")->base();
	UINT16 *sub_cpu  = (UINT16 *) memregion("sub")->base();

	main_cpu[0x064/4] = 0x0000042a; // PIC protection?

	// Sub CPU
	sub_cpu[0x506/2] = 0x5470;  // rte -> rts
}

DRIVER_INIT_MEMBER(seta2_state,funcube2)
{
	UINT32 *main_cpu = (UINT32 *) memregion("maincpu")->base();
	UINT16 *sub_cpu  = (UINT16 *) memregion("sub")->base();

	main_cpu[0xa5c/4] = 0x4e713e3c;       // PIC protection?
	main_cpu[0xa74/4] = 0x4e713e3c;
	main_cpu[0xa8c/4] = 0x4e7141f9;

	// Sub CPU
	sub_cpu[0x4d4/2] = 0x5470;  // rte -> rts
}

DRIVER_INIT_MEMBER(seta2_state,funcube3)
{
	UINT32 *main_cpu = (UINT32 *) memregion("maincpu")->base();
	UINT16 *sub_cpu  = (UINT16 *) memregion("sub")->base();

	main_cpu[0x008bc/4] = 0x4a804e71;
	main_cpu[0x19f0c/4] = 0x4e714e71;
	main_cpu[0x19fb8/4] = 0x4e714e71;

	// Sub CPU
	sub_cpu[0x4d4/2] = 0x5470;  // rte -> rts
}

/***************************************************************************

Guardians
Banpresto, 1995

This hardware is not common Banpresto hardware. Possibly licensed
to them from another manufacturer? Or an early design that they decided
not to use for future games? Either way, this game is _extremely_ rare :-)

PCB Layout
----------

P-FG01-1
------------------------------------------------------
|        X1-010 6264          U32 CXK581000          |
|                                 CXK581000      U16 |
|                                                    |
|                                                U20 |
|    U3 U5 U2 U4 62256 CXK58257                      |
|                62256 CXK58257                  U15 |
|                                                    |
|J                                               U19 |
|A    TMP68301AF-16                                  |
|M                                               U18 |
|M                           NEC                     |
|A          NEC              DX-101              U22 |
|           DX-102                                   |
|                                                U17 |
|                   PAL   50MHz                      |
|                                                U21 |
|           DSW1(8)                                  |
|           DSW2(8)                   CXK58257 NEC   |
|                                     CXK58257 DX-102|
------------------------------------------------------

Notes:
      HSync: 15.23kHz
      VSync: 58.5Hz

***************************************************************************/

ROM_START( grdians )
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "u2.bin", 0x000000, 0x080000, CRC(36adc6f2) SHA1(544e87f88179fe1342e7a06a8948ac1828e85108) )
	ROM_LOAD16_BYTE( "u3.bin", 0x000001, 0x080000, CRC(2704f416) SHA1(9081a12cbb9927d36e1c50b52aa2c6003810ee42) )
	ROM_LOAD16_BYTE( "u4.bin", 0x100000, 0x080000, CRC(bb52447b) SHA1(61433f683210ab2bc2cf1cc4b5b7a39cc5b6493d) )
	ROM_LOAD16_BYTE( "u5.bin", 0x100001, 0x080000, CRC(9c164a3b) SHA1(6d688c7af9e7e8e8d54b2e4dfbf41f59c79242eb) )

	ROM_REGION( 0x2000000, "sprites", ROMREGION_ERASE)  // Sprites
	ROM_LOAD( "u16.bin",  0x0000000, 0x400000, CRC(6a65f265) SHA1(6cad11f718f8bbcff464d41eb4717460769237ed) )
	ROM_LOAD( "u20.bin",  0x0600000, 0x200000, CRC(a7226ab7) SHA1(408580dd35c568ffef1ebbd87359e3ec1f867020) )
	ROM_CONTINUE(         0x0400000, 0x200000 )

	ROM_LOAD( "u15.bin",  0x0800000, 0x400000, CRC(01672dcd) SHA1(f61f60e3343cc5b6ccee391ee529966a141566db) )
	ROM_LOAD( "u19.bin",  0x0e00000, 0x200000, CRC(c0c998a0) SHA1(498fb1877527ed37412537f06a2c39ff0c60f146) )
	ROM_CONTINUE(         0x0c00000, 0x200000 )

	ROM_LOAD( "u18.bin",  0x1000000, 0x400000, CRC(967babf4) SHA1(42a6311576417c44aeaceb8ba6bb3cd7794e4882) )
	ROM_LOAD( "u22.bin",  0x1600000, 0x200000, CRC(6239997a) SHA1(87b6d6f30f152f625f82fd858c1290176c7e156e) )
	ROM_CONTINUE(         0x1400000, 0x200000 )

	ROM_LOAD( "u17.bin",  0x1800000, 0x400000, CRC(0fad0629) SHA1(1bdc8e7c5e39e83d327f14a672ec81b049112da6) )
	ROM_LOAD( "u21.bin",  0x1e00000, 0x200000, CRC(6f95e466) SHA1(28482fad16a3ac9302f152d81552e6f84a44f3e4) )
	ROM_CONTINUE(         0x1c00000, 0x200000 )

	ROM_REGION( 0x200000, "x1snd", 0 )  // Samples
	// Leave 1MB empty (addressable by the chip)
	ROM_LOAD( "u32.bin", 0x100000, 0x100000, CRC(cf0f3017) SHA1(8376d3a674f71aec72f52c72758fbc53d9feb1a1) )
ROM_END

/***************************************************************************

MS Gundam Ex Revue
Banpresto, 1994

This game runs on Seta/Allumer hardware

PCB Layout
----------

P0-113A   BP949KA
|----------------------------------|
|  X1-010  6264  U28               |
|                     581001   U19 |
|     U3  U5  U2  U4  581001   U17 |
|      62256   62256           U15 |
|J                             U20 |
|A    U77  68301               U18 |
|M                     *       U16 |
|M    93C46                    U23 |
|A                             U22 |
|                              U21 |
|  DSW1            50MHz           |
|  DSW2       PAL  32.5304MHz      |
|       20MHz PAL                  |
|----------------------------------|

Notes:
      *: unknown QFP208 (has large heatsink on it). Should be similar to other known
         graphics chips used on Seta hardware of this era.
      68301 clock: 16.000MHz (?? From what OSC + divider??)
      VSync: 60Hz

***************************************************************************/

ROM_START( gundamex )
	ROM_REGION( 0x600000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE(      "ka002002.u2",  0x000000, 0x080000, CRC(e850f6d8) SHA1(026325e305676b1f8d3d9e7573920f8b70d7bccb) )
	ROM_LOAD16_BYTE(      "ka002004.u3",  0x000001, 0x080000, CRC(c0fb1208) SHA1(84b25e4c73cb8e023ee5dbf69f588be98700b43f) )
	ROM_LOAD16_BYTE(      "ka002001.u4",  0x100000, 0x080000, CRC(553ebe6b) SHA1(7fb8a159513d31a1d60520ff14e4c4d133fd3e19) )
	ROM_LOAD16_BYTE(      "ka002003.u5",  0x100001, 0x080000, CRC(946185aa) SHA1(524911c4c510d6c3e17a7ab42c7077c2fffbf06b) )
	ROM_LOAD16_WORD_SWAP( "ka001005.u77", 0x500000, 0x080000, CRC(f01d3d00) SHA1(ff12834e99a76261d619f10d186f4b329fb9cb7a) )

	ROM_REGION( 0x2000000, "sprites", ROMREGION_ERASE)  // Sprites
	ROM_LOAD( "ka001009.u16",  0x0000000, 0x200000, CRC(997d8d93) SHA1(4cb4cdb7e8208af4b14483610d9d6aa5e13acd89) )
	ROM_LOAD( "ka001010.u18",  0x0200000, 0x200000, CRC(811b67ca) SHA1(c8cfae6f54c76d63bd625ff011c872ffb75fd2e2) )
	ROM_LOAD( "ka001011.u20",  0x0400000, 0x200000, CRC(08a72700) SHA1(fb8003aa02dd249c30a757cb43b516260b41c1bf) )
	ROM_LOAD( "ka001012.u15",  0x0800000, 0x200000, CRC(b789e4a8) SHA1(400b773f24d677a9d47466fdbbe68cb6efc1ad37) )
	ROM_LOAD( "ka001013.u17",  0x0a00000, 0x200000, CRC(d8a0201f) SHA1(fe8a2407c872adde8aec8e9340b00be4f00a2872) )
	ROM_LOAD( "ka001014.u19",  0x0c00000, 0x200000, CRC(7635e026) SHA1(116a3daab14a17faca85c4a956b356aaf0fc2276) )
	ROM_LOAD( "ka001006.u21",  0x1000000, 0x200000, CRC(6aac2f2f) SHA1(fac5478ca2941a93c57f670a058ff626e537bcde) )
	ROM_LOAD( "ka001007.u22",  0x1200000, 0x200000, CRC(588f9d63) SHA1(ed5148d09d02e3bc12c50c39c5c86e6356b2dd7a) )
	ROM_LOAD( "ka001008.u23",  0x1400000, 0x200000, CRC(db55a60a) SHA1(03d118c7284ca86219891c473e2a89489710ea27) )
	ROM_FILL(                  0x1800000, 0x600000, 0x00 ) // 6bpp instead of 8bpp

	ROM_REGION( 0x300000, "x1snd", 0 )  // Samples
	// Leave 1MB empty (addressable by the chip)
	ROM_LOAD( "ka001015.u28", 0x100000, 0x200000, CRC(ada2843b) SHA1(09d06026031bc7558da511c3c0e29187ea0a0099) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "eeprom.bin", 0x0000, 0x0080, CRC(80f8e248) SHA1(1a9787811e56d95f7acbedfb00225b6e7df265eb) )
ROM_END

/***************************************************************************

Wakakusamonogatari Mahjong Yonshimai (JPN Ver.)
(c)1996 Maboroshi Ware

Board:  P0-123A

CPU:    TMP68301 (68000 core)
OSC:    50.0000MHz
        32.5304MHz

Sound:  X1-010

***************************************************************************/

ROM_START( mj4simai )
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "ll.u2",       0x000000, 0x080000, CRC(7be9c781) SHA1(d29e579706d98909933f6bed2ee292c88ed10d2c) )
	ROM_LOAD16_BYTE( "lh1.u3",      0x000001, 0x080000, CRC(82aa3f72) SHA1(a93d5dc7cdf12f852a692759d91f6f2951b6b5b5) )
	ROM_LOAD16_BYTE( "hl.u4",       0x100000, 0x080000, CRC(226063b7) SHA1(1737baffc16ff7261f887911187ece96925fa6ff) )
	ROM_LOAD16_BYTE( "hh.u5",       0x100001, 0x080000, CRC(23aaf8df) SHA1(b3d678afce4ddef32e48d690c6d07b723dd0c28f) )

	ROM_REGION( 0x2000000, "sprites", 0 )   // Sprites
	ROM_LOAD( "cha-03.u16",  0x0000000, 0x400000, CRC(d367429a) SHA1(b32c215ef85c3d0a4c5550cef4f5c4c0e7030b7c) )
	ROM_LOAD( "cha-04.u18",  0x0400000, 0x400000, CRC(7f2008c3) SHA1(e45d863540eb2381f5d7660d64cdfef87c890768) )
	ROM_LOAD( "cha-05.u15",  0x0800000, 0x400000, CRC(e94ec40a) SHA1(2685dbc5680b5f76688c6b4fbe40ae682c525bfe) )
	ROM_LOAD( "cha-06.u17",  0x0c00000, 0x400000, CRC(5cb0b3a9) SHA1(92fb82d45b4c46326d5796981f812e20a8ddb4f2) )
	ROM_LOAD( "cha-01.u21",  0x1000000, 0x400000, CRC(35f47b37) SHA1(4a8eb088890272f2a069e2c3f00fadf6421f7b0e) )
	ROM_LOAD( "cha-02.u22",  0x1400000, 0x400000, CRC(f6346860) SHA1(4eebd3fa315b97964fa39b88224f9de7622ba881) )
	ROM_FILL(                0x1800000, 0x800000, 0x00 )   // 6bpp instead of 8bpp

	ROM_REGION( 0x500000, "x1snd", 0 )  // Samples
	// Leave 1MB empty (addressable by the chip)
	ROM_LOAD( "cha-07.u32",  0x100000, 0x400000, CRC(817519ee) SHA1(ed09740cdbf61a328f7b50eb569cf498fb749416) )
ROM_END

/***************************************************************************

Kosodate Quiz My Angel (JPN Ver.)
(c)1996 Namco

Board:  KE (Namco) ; P0-125A (Seta)

CPU:    TMP68301 (68000 core)
OSC:    50.0000MHz
        32.5304MHz

Sound:  X1-010

***************************************************************************/

ROM_START( myangel )
	ROM_REGION( 0x200000, "maincpu", 0 )        // TMP68301 Code
	ROM_LOAD16_BYTE( "kq1-prge.u2", 0x000000, 0x080000, CRC(6137d4c0) SHA1(762341e11b56e4a7787a0662833b702b78aee0a9) )
	ROM_LOAD16_BYTE( "kq1-prgo.u3", 0x000001, 0x080000, CRC(4aad10d8) SHA1(a08e1c4f57c64be829e0807ae2791da947fd60aa) )
	ROM_LOAD16_BYTE( "kq1-tble.u4", 0x100000, 0x080000, CRC(e332a514) SHA1(dfd255239c80c48c9865e70681b9ddd175b8bf55) )
	ROM_LOAD16_BYTE( "kq1-tblo.u5", 0x100001, 0x080000, CRC(760cab15) SHA1(fa7ea85ec2ebfaab3111b8631ea6ea3d794d449c) )

	ROM_REGION( 0x1000000, "sprites", 0 )   // Sprites
	ROM_LOAD( "kq1-cg2.u20", 0x000000, 0x200000, CRC(80b4e8de) SHA1(c8685c4f4e3c0415ce0ec88e0288835e504cab00) )
	ROM_LOAD( "kq1-cg0.u16", 0x200000, 0x200000, CRC(f8ae9a05) SHA1(4f3b41386a48a1608aa96b911e6b74ca775260fb) )
	ROM_LOAD( "kq1-cg3.u19", 0x400000, 0x200000, CRC(9bdc35c9) SHA1(fd0a1eb3dd10705bce5462263667353632558b58) )
	ROM_LOAD( "kq1-cg1.u15", 0x600000, 0x200000, CRC(23bd7ea4) SHA1(e925bbadc33fc2586bb18283cf989ab35f28c1e9) )
	ROM_LOAD( "kq1-cg6.u22", 0x800000, 0x200000, CRC(b25acf12) SHA1(5cca35921f3b376c3cc36f5b009eb845db2e1897) )
	ROM_LOAD( "kq1-cg4.u18", 0xa00000, 0x200000, CRC(dca7f8f2) SHA1(20595c7940a28d01bdc6610b67aaaeac61ba92e2) )
	ROM_LOAD( "kq1-cg7.u21", 0xc00000, 0x200000, CRC(9f48382c) SHA1(80dfc33a55123b5d3cdb3ed97b43a527f0254d61) )
	ROM_LOAD( "kq1-cg5.u17", 0xe00000, 0x200000, CRC(a4bc4516) SHA1(0eb11fa54d16bba1b96f9dd943a68949a3bb9a2f) )

	ROM_REGION( 0x300000, "x1snd", 0 )  // Samples
	// Leave 1MB empty (addressable by the chip)
	ROM_LOAD( "kq1-snd.u32", 0x100000, 0x200000, CRC(8ca1b449) SHA1(f54096fb5400843af4879135c96760485b6cb319) )
ROM_END

/***************************************************************************

Kosodate Quiz My Angel 2 (JPN Ver.)
(c)1997 Namco

Board:  KL (Namco) ; P0-136A (Seta)

CPU:    TMP68301 (68000 core)
OSC:    50.0000MHz
        32.5304MHz

Sound:  X1-010

***************************************************************************/

ROM_START( myangel2 )
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "kqs1ezpr.u2", 0x000000, 0x080000, CRC(2469aac2) SHA1(7dade2de31252e305d24c659c4801dd4687ad1f6) )
	ROM_LOAD16_BYTE( "kqs1ozpr.u3", 0x000001, 0x080000, CRC(6336375c) SHA1(72089f77e94832e74e0512944acadeccd0dec8b0) )
	ROM_LOAD16_BYTE( "kqs1e-tb.u4", 0x100000, 0x080000, CRC(e759b4cc) SHA1(4f806a144a47935b2710f8af800ec0d771f12a18) )
	ROM_LOAD16_BYTE( "kqs1o-tb.u5", 0x100001, 0x080000, CRC(b6168737) SHA1(4c3de877c0c1dca1c43ac737a0bf231335237d3a) )

	ROM_REGION( 0x1800000, "sprites", 0 )   // Sprites
	ROM_LOAD( "kqs1-cg4.u20", 0x0000000, 0x200000, CRC(d1802241) SHA1(52c45a13d46f7ee8043e85b99d07b1765ca93dcc) )
	ROM_LOAD( "kqs1-cg0.u16", 0x0200000, 0x400000, CRC(c21a33a7) SHA1(bc6f479a8f4c716ba79a725f160ddeb95fdedbcb) )
	ROM_LOAD( "kqs1-cg5.u19", 0x0600000, 0x200000, CRC(d86cf19c) SHA1(da5a5b576ce107433605b24d8b9dcd0abd46bcde) )
	ROM_LOAD( "kqs1-cg1.u15", 0x0800000, 0x400000, CRC(dca799ba) SHA1(8379b11472c27b1945fe7fc274c7fedf756accba) )
	ROM_LOAD( "kqs1-cg6.u22", 0x0c00000, 0x200000, CRC(3f08886b) SHA1(054546ae44ffa5d0973f4ead080fe720a340e144) )
	ROM_LOAD( "kqs1-cg2.u18", 0x0e00000, 0x400000, CRC(f7f92c7e) SHA1(24a525a15fded0de6e382b346da6bd5e7b9eced5) )
	ROM_LOAD( "kqs1-cg7.u21", 0x1200000, 0x200000, CRC(2c977904) SHA1(2589447f2471cdc414266b34aff552044c680d93) )
	ROM_LOAD( "kqs1-cg3.u17", 0x1400000, 0x400000, CRC(de3b2191) SHA1(d7d6ea07b665cfd834747d3c0776b968ce03bc6a) )

	ROM_REGION( 0x500000, "x1snd", 0 )  // Samples
	// Leave 1MB empty (addressable by the chip)
	ROM_LOAD( "kqs1-snd.u32", 0x100000, 0x400000, CRC(792a6b49) SHA1(341b4e8f248b5032217733bada32e353c67e3888) )
ROM_END

/***************************************************************************

  Namco Stars

  EVA2B PCB (8829970101 P0-140B Serial Z033):

  TMP68301AF-16 CPU
  DX101
  DX102 x 2
  OKI M9810
  GAL 16V8d x 2
  MAX232
  DSW8
  Coin Battery
  Reset Button
  OSC: 25.447 MHz @ XM1, 50.000 MHz @ XM2, 32.53005 MHz @ X2
  Volume Trimmer

***************************************************************************/

ROM_START( namcostr )
	ROM_REGION( 0x80000, "maincpu", 0 ) // TMP68301 Code
	ROM_LOAD( "ns1mpr0.u08", 0x00000, 0x80000, BAD_DUMP CRC(008d23fe) SHA1(8c77a34dd0285c06809e99d20b9d8b31b81bfc68) )  // FIXED BITS (xxxxx1xxxxxxxxxx)

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD32_WORD( "ns1cha0.u39", 0x000000, 0x400000, BAD_DUMP CRC(372d1651) SHA1(355553992e5a474ae1e45bcdeb88804d5b75f802) ) // FIXED BITS (xxxxx1xxxxxxxxxx)
	ROM_LOAD32_WORD( "ns1cha1.u38", 0x000002, 0x400000, BAD_DUMP CRC(82e67809) SHA1(6b25726cd3683e1691e4d4e1628c13998f20933d) ) // FIXED BITS (xxxxx1xxxxxxxxxx)

	ROM_REGION( 0x1000000, "oki", 0 )
	ROM_LOAD( "ns1voi0.u40", 0x000000, 0x400000, BAD_DUMP CRC(fe5c2b16) SHA1(21e4423cc91e8833297d4588343237b8b3155196) )    // FIXED BITS (xxxxx1xxxxxxxxxx)
ROM_END

/***************************************************************************

                            Puzzle De Bowling (Japan)

(c)1999 MOSS / Nihon System

   CPU: Toshiba TMP68301AF-16 (100 Pin PQFP)
 Video: NEC DX-101 (240 Pin PQFP)
        NEC DX-102 (52 Pin PQFP x3)
 Sound: X1-010 (Mitsubishi M60016 Gate Array, 80 Pin PQFP)
   OSC: 50MHz & 32.53047MHz
 Other: 8 Position Dipswitch x 2
        Lattice ispLSI2032 - stamped "KUDEC"

PCB Number: P0-142A
+-----------------------------------------------------------+
|                VOL                       +------+         |
|                                          |Seta  |     M1  |
|   +---+ +---+                            |X1-010|         |
|   |   | |   |  U4*  M                    |      |  +---+  |
+-+ | U | | U |       1                    +------+  | K |  |
  | | 0 | | 0 |                     U30*             | U |  |
+-+ | 7 | | 6 |  U5*  M                              | S |  |
|   |   | |   |       1                              |   |  |
|   +---+ +---+                                      | U |  |
|                                          Lattice   | 1 |  |
|J  D D  +---+                            ispLSI2032 | 8 |  |
|A  S S  |DX |                  +-------+            +---+  |
|M  W W  |102|                  |Toshiba|     CN2           |
|M  1 2  +---+      BAT1*       |  TMP  |                   |
|A                              | 68301 |  U50*             |
|                               +-------+                   |
|C                                                          |
|o                 50MHz        +----------+     XM2*       |
|n    +---+                     |          |                |
|n    |DX |     SW1             |   NEC    |    M   M       |
|e    |102|                     |  DX-101  |    2   2       |
|c    +---+         M  M        |          |                |
|t                  1  1        |          |                |
|e                              +----------+                |
|r                                                          |
|                             +---+      +---++---++---+    |
|                             | K |      | K || K || K |    |
|     +---+                   | U |      | U || U || U |    |
+-+   |DX |                   | C |      | C || C || C |    |
  |   |102|     32.53047MHz   |   |      |   ||   ||   |    |
+-+   +---+                   | U |      | U || U || U |    |
|                             | 4 |      | 4 || 3 || 3 |    |
|                             | 0 |      | 1 || 8 || 9 |    |
|                             +---+      +---++---++---+    |
+-----------------------------------------------------------+

* Unpopulated:
  U4 & U5 RAM HM62256 equivalent
  U50 93LC46BX EEPROM
  U30 74HC00
  BAT1 CR2032 3Volt battery
  XM2 OSC

Ram M1 are NEC D43001GU-70LL
Ram M2 are LGS GM76C8128ALLFW70

KUP-U06-I03 U06 Program rom ST27C4001 (even)
KUP-U07-I03 U07 Program rom ST27C4001 (odd)

KUS-U18-I00 U18 Mask rom (Samples 23C32000 32Mbit)

KUC-U38-I00 U38 Mask rom (Graphics 23C32000 32Mbit)
KUC-U39-I00 U39 Mask rom (Graphics 23C32000 32Mbit)
KUC-U40-I00 U40 Mask rom (Graphics 23C32000 32Mbit)
KUC-U41-I00 U41 Mask rom (Graphics 23C32000 32Mbit)

***************************************************************************/

ROM_START( pzlbowl )
	ROM_REGION( 0x100000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "kup-u06.i03", 0x000000, 0x080000, CRC(314e03ac) SHA1(999398e55161dd75570d418f4c9899e3bf311cc8) )
	ROM_LOAD16_BYTE( "kup-u07.i03", 0x000001, 0x080000, CRC(a0423a04) SHA1(9539023c5c2f2bf72ee3fb6105443ffd3d61e2f8) )

	ROM_REGION( 0x1000000, "sprites", 0 )   // Sprites
	ROM_LOAD( "kuc-u38.i00", 0x000000, 0x400000, CRC(3db24172) SHA1(89c39963e15c53b799994185d0c8b2e795478939) )
	ROM_LOAD( "kuc-u39.i00", 0x400000, 0x400000, CRC(9b26619b) SHA1(ea7a0bf46641d15353217b01e761d1a148bee4e7) )
	ROM_LOAD( "kuc-u40.i00", 0x800000, 0x400000, CRC(7e49a2cf) SHA1(d24683addbc54515c33fb620ac500e6702bd9e17) )
	ROM_LOAD( "kuc-u41.i00", 0xc00000, 0x400000, CRC(2febf19b) SHA1(8081ac590c0463529777b5e4817305a1a6f6ea41) )

	ROM_REGION( 0x500000, "x1snd", 0 )  // Samples
	// Leave 1MB empty (addressable by the chip)
	ROM_LOAD( "kus-u18.i00", 0x100000, 0x400000, CRC(e2b1dfcf) SHA1(fb0b8be119531a1a27efa46ed7b86b05a37ed585) )
ROM_END

/***************************************************************************

Penguin Brothers (Japan)
(c)2000 Subsino

   CPU: Toshiba TMP68301AF-16 (100 Pin PQFP)
 Video: NEC DX-101 (240 Pin PQFP)
        NEC DX-102 (52 Pin PQFP x3)
 Sound: X1-010 (Mitsubishi M60016 Gate Array, 80 Pin PQFP)
   OSC: 50MHz, 32.53047MHz & 28MHz
 Other: 8 Position Dipswitch x 2
        Lattice ispLSI2032

PCB Number: P0-142A
+-----------------------------------------------------------+
|                VOL                       +------+         |
|                                          |Seta  |     M1  |
|   +---+ +---+                            |X1-010|         |
|   |   | |   |   M   M                    |      |  +---+  |
+-+ | U | | U |   1   1                    +------+  |   |  |
  | | 0 | | 0 |                    74HC00            |   |  |
+-+ | 7 | | 6 |   M   M                              | U |  |
|   |   | |   |   1   1                              | 1 |  |
|   +---+ +---+                                      | 8 |  |
|                                          Lattice   |   |  |
|J  D D  +---+                            ispLSI2032 |   |  |
|A  S S  |DX |                  +-------+            +---+  |
|M  W W  |102|                  |Toshiba|     CN2           |
|M  1 2  +---+      BAT1*       |  TMP  |                   |
|A                              | 68301 |  U50*             |
|                               +-------+                   |
|C                                                          |
|o                 50MHz        +----------+     28MHz      |
|n    +---+                     |          |                |
|n    |DX |     SW1             |   NEC    |    M   M       |
|e    |102|                     |  DX-101  |    2   2       |
|c    +---+         M  M        |          |                |
|t                  1  1        |          |                |
|e                              +----------+                |
|r                                                          |
|                             +---+      +---++---++---+    |
|                             |   |      |   ||   ||   |    |
|     +---+                   |   |      |   ||   ||   |    |
+-+   |DX |                   | U |      | U || U || U |    |
  |   |102|     32.53047MHz   | 4 |      | 4 || 3 || 3 |    |
+-+   +---+                   | 0 |      | 1 || 8 || 9 |    |
|                             |   |      |   ||   ||   |    |
|                             |   |      |   ||   ||   |    |
|                             +---+      +---++---++---+    |
+-----------------------------------------------------------+

Notes:  pzlbowl PCB with these extra parts:
        28MHz OSC
        2x 62256 SRAM
        74HC00

U50*  Unpopulated 93LC46BX EEPROM
BAT1* Unpopulated CR2032 3Volt battery

Ram M1 are NEC D43001GU-70LL
Ram M2 are LGS GM76C8128ALLFW70

***************************************************************************/

ROM_START( penbros )
	ROM_REGION( 0x100000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "u06.bin", 0x000000, 0x080000, CRC(7bbdffac) SHA1(d5766cb171b8d2e4c04a6bae37181fa5ada9d797) )
	ROM_LOAD16_BYTE( "u07.bin", 0x000001, 0x080000, CRC(d50cda5f) SHA1(fc66f55f2070b447c5db85c948ce40adc37512f7) )

	ROM_REGION( 0x1000000, "sprites", 0 )   // Sprites
	ROM_LOAD( "u38.bin", 0x000000, 0x400000, CRC(4247b39e) SHA1(f273931293beced312e02c870bf35e9cf0c91a8b) )
	ROM_LOAD( "u39.bin", 0x400000, 0x400000, CRC(f9f07faf) SHA1(66fc4a9ad422fb384d2c775e43619137226898fc) )
	ROM_LOAD( "u40.bin", 0x800000, 0x400000, CRC(dc9e0a96) SHA1(c2c8ccf9039ee0e179b08fdd2d37f29899349cda) )
	ROM_FILL(            0xc00000, 0x400000, 0x00 )    // 6bpp instead of 8bpp

	ROM_REGION( 0x300000, "x1snd", 0 )  // Samples
	// Leave 1MB empty (addressable by the chip)
	ROM_LOAD( "u18.bin", 0x100000, 0x200000, CRC(de4e65e2) SHA1(82d4e590c714b3e9bf0ffaf1500deb24fd315595) )
ROM_END

/***************************************************************************

Reel'N Quake!

   CPU: Toshiba TMP68301AF-16 (100 Pin PQFP)
 Video: NEC DX-101 (240 Pin PQFP, @ U10)
        NEC DX-102 (52 Pin PQFP x3, @ U28 U30 & U45)
 Sound: X1-010 (Mitsubishi M60016 Gate Array, 80 Pin PQFP @ U26)
   OSC: 50MHz & 28MHz
 Other: 8 Position Dipswitch x 2
        Push Button SW1
        3.6V Battery at BT1
        GAL 16V8 - labeled "KF-001" at U38

Memory:
M1 are TC551001BFL-70L at U42 & U43
M2 is  W2465K-70LL at U27
M3 are LH5168D-10L at U8 & U9
M4 are UT62256SC-70L at U6, U7, U13 & U14

PCB Number: P-FG-02
+-----------------------------------------------------------+
|             +------+      U  U                            |
| VOL         |Seta  |   M  5  5            +--------------+|
|             |X1-010|   2  8  7    +-+  M  |KF-001-005 U16||
|             +------+      *  *    | |  1  +--------------+|
+-+                                 |U|                     |
  |  +-+    +-+           BT1       |3|            U20*     |
+-+  | |    | |         M           |2|  M                  |
|  C |U| U  |U| U  M M  4           | |  1  +--------------+|
|J N |3| 5  |2| 4  3 3              +-+     |KF-001-006 U15||
|A 1 | | *  | | *       M                   +--------------+|
|M   +-+    +-+         4                                   |
|M C                                               U19*     |
|A N                                                        |
|  2                                        +--------------+|
|C 1                                        |KF-001-007 U18||
|o                           +----------+   +--------------+|
|n C        +-------+        |          |                   |
|n N        |Toshiba|        |   NEC    |          U22*     |
|e 2        |  TMP  |        |  DX-101  |                   |
|c 2        | 68301 |        |          |   +--------------+|
|t        U +-------+        |          |   |KF-001-008 U17||
|e C      5                  +----------+   +--------------+|
|r N      6                                                 |
|  3      *                                        U21*     |
+-+   +---+    +---+       U  50MHz 32MHz*                  |
  |   |DX | S  |DX |       3                                |
  |   |102| W  |102|       8                   +---+   28MHz|
+-+   +---+ 1  +---+                    M  M   |DX |        |
|              D D                      4  4   |102|        |
|              S S                             +---+        |
|              W W                                          |
|              2 1                                          |
+-----------------------------------------------------------+

CN1   - 7 Pin connector
CN2-1 - 3 Pin connector
CN2-2 - 3 Pin connector
CN3   - 10 Pin connector (used for extra buttons)

* Denotes not populated. U56 is unpopulated 93C45 EEPROM

    U3-U5 silkscreened 27C4001
  U57-U58 silkscreened 23C8001E
  U15-U22 silkscreened 23C32000
      U32 silkscreened 23C32000

Note:
  The PCB is silkscreened with 23C32000 which would be equal to the 27C322.
  The graphics roms dumped that way have the first half as a bad mirror
  of the second half (even <- odd, odd <- FF). They seem OK dumped as 27C160.

Program ROMs:
  Labeled roms (undumped) have been seen as KF003002 & KF003004, which
  revision do have?

V1.05 program roms have been seen with labels dated 12/17/97

Reel'N Quake! is also known to be available on the P-FG-03 PCB which is
 essentially the same layout as the P-FG-02 but with standard 8-liner
 edge connectors. Program rom labels for this (undumped) set are:

   KFP       KFP
   U02  and  U03
   C00       C00

***************************************************************************/

ROM_START( reelquak )
	ROM_REGION( 0x100000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "rq_ver1.05.u2", 0x00000, 0x80000, CRC(7740d7a4) SHA1(21c28db5d4d7eea5a2506cb51b58533eba28c2cb) ) /* Should be KF00x002, x = revision */
	ROM_LOAD16_BYTE( "rq_ver1.05.u3", 0x00001, 0x80000, CRC(8c78889e) SHA1(584ba123e9caafdbddc96a4d9b2b6f6994fa84b0) ) /* Should be KF00x004, x = revision */

	ROM_REGION( 0x800000, "sprites", 0 )    // Sprites
	ROM_LOAD( "kf-001-005_t42.u16", 0x000000, 0x200000, CRC(25e07d5c) SHA1(dd0818611f39be25dc6f0c737da4e79c6c0f9659) )
	ROM_LOAD( "kf-001-006_t43.u15", 0x200000, 0x200000, CRC(67e2ecc4) SHA1(35cdaf7fcd29e0229da104baced41fa7620dba3d) )
	ROM_LOAD( "kf-001-007_t44.u18", 0x400000, 0x200000, CRC(9daec83d) SHA1(07de144898deac5058d05466f29682d7840323b7) )
	ROM_LOAD( "kf-001-008_t45.u17", 0x600000, 0x200000, CRC(f6ef6e41) SHA1(c3e838dd4dc340f44abdf45ec0b90de24f50dda9) )

	ROM_REGION( 0x300000, "x1snd", 0 )  // Samples
	// Leave 1MB empty (addressable by the chip)
	ROM_LOAD( "kf-001-009_t46.u32", 0x100000, 0x200000, CRC(2a9641f9) SHA1(efb9df78f1877eddf29c4dae2461546adb9cea8f) )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8_kf-001.u38", 0x000, 0x117, NO_DUMP )
ROM_END

/***************************************************************************

Endless Riches
(c) 199? E.N.Tiger

   CPU: Toshiba TMP68301AF-16 (100 Pin PQFP)
 Video: NEC DX-101 (240 Pin PQFP, @ U10)
        NEC DX-102 (52 Pin PQFP x3, @ U28 U30 & U45)
 Sound: X1-010 (Mitsubishi M60016 Gate Array, 80 Pin PQFP @ U26)
   OSC: 50MHz & 28MHz
 Other: 8 Position Dipswitch x 2
        Push Button SW1
        3.6V Battery at BT1
        GAL 16V8 - labeled "KF-001" at U38

Memory:
M1 are TC551001BFL-70L at U42 & U43
M2 is  W2465K-70LL at U27
M3 are LH5168D-10L at U8 & U9
M4 are UT62256SC-70L at U6, U7, U13 & U14

PCB Number: P-FG-03
+-----+_+----------------------------------------------------+
|             +------+      U  U                             |
| VOL         |Seta  |   M  5  5            +---------------+|
|             |X1-010|   2  8  7    +-+  M  |KFC-U16-C00 U16||
|             +------+      *  *    | |  1  +---------------+|
+-+                                 |U|                      |
  |  +-+    +-+           BT1       |3|            U20*      |
+-+  | |    | |         M           |2|  M                   |
|    |U| U  |U| U  M M  4           | |  1  +---------------+|
|    |3| 5  |2| 4  3 3              +-+     |KFC-U15-C00 U15||
|8   | | *  | | *       M                   +---------------+|
|    +-+    +-+         4                                    |
|L                                                 U19*      |
|I                                                           |
|N                                          +---------------+|
|E                                          |KFC-U18-C00 U18||
|R                           +----------+   +---------------+|
|           +-------+        |          |                    |
|C C        |Toshiba|        |   NEC    |          U22*      |
|o N        |  TMP  |        |  DX-101  |                    |
|n 1        | 68301 |        |          |   +---------------+|
|n        U +-------+        |          |   |KFC-U17-C00 U17||
|e C      5                  +----------+   +---------------+|
|c N      6                                                  |
|t 2      *                                        U21*      |
|e  +---+    +---+       U  50MHz 28MHz                      |
|r    |DX | S  |DX |       3                                 |
|     |102| W  |102|       8                   +---+    OSC2*|
|     +---+ 1  +---+                    M  M   |DX |         |
+-+            D D                      4  4   |102|         |
  |            S S                             +---+         |
+-+            W W                                           |
|              2 1                                           |
+------------------------------------------------------------+

CN1   - 7 Pin connector
CN2   - 8 Pin connector

* Denotes not populated.
  U56 is unpopulated 93C45 EEPROM
  DSW2 is unpopulated

    U3-U5 silkscreened 27C4001
  U57-U58 silkscreened 23C8001E
  U15-U22 silkscreened 23C32000
      U32 silkscreened 23C32000

Note:
  8-Liner version of P-FG-02 (see Reel'N Quake! above)
  Hitting Service Mode "F2" will show Ver 1.7, but going through the diagnostic "0"
   Main Menu --> Test Mode --> Memory Test will show Version 1.20

***************************************************************************/

ROM_START( endrichs )
	ROM_REGION( 0x100000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "kfp_u02_c12.u2", 0x00000, 0x80000, CRC(462341d2) SHA1(a88215d74469513f4239853f62d4dbbffe2aa83a) )
	ROM_LOAD16_BYTE( "kfp_u03_c12.u3", 0x00001, 0x80000, CRC(2baee8d1) SHA1(f86920382c54a259adb1dee253859561746d215a) )

	ROM_REGION( 0x800000, "sprites", 0 )    // Sprites
	ROM_LOAD( "kfc-u16-c00.u16", 0x000000, 0x200000, CRC(cbfe5e0f) SHA1(6c7c8088c43231997ac47ce05cf43c78c1fdad47) )
	ROM_LOAD( "kfc-u15-c00.u15", 0x200000, 0x200000, CRC(98e4c36c) SHA1(651be122b78f225d38878ae90776f66989440590) )
	ROM_LOAD( "kfc-u18-c00.u18", 0x400000, 0x200000, CRC(561ac136) SHA1(96da493157405a5d3d72b8cc3004abd3fa3eadfa) )
	ROM_LOAD( "kfc-u17-c00.u17", 0x600000, 0x200000, CRC(34660029) SHA1(cf09b97422497d739f71e6ff8b9974fca0329928) )

	ROM_REGION( 0x300000, "x1snd", 0 )  // Samples
	// Leave 1MB empty (addressable by the chip)
	ROM_LOAD( "kfs-u32-c00.u32", 0x100000, 0x200000, CRC(e9ffbecf) SHA1(3cc9ab3f4be1a305235603a68ca1e15797fb27cb) ) // Yes, it's actually "KFS" here

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8_kf-001.u38", 0x000, 0x117, NO_DUMP )
ROM_END

/***************************************************************************

Sammy USA Outdoor Shooting Series PCB

PCB B0-003A (or B0-003B):
   Deer Hunting USA (c) 2000 Sammy USA
   Turkey Hunting USA (c) 2001 Sammy USA

PCB B0-010A:
   Wing Shooting Championship (c) 2001 Sammy USA
   Trophy Hunting - Bear & Moose (c) 2002 Sammy USA


   CPU: Toshiba TMP68301AF-16 (100 Pin PQFP)
 Video: NEC DX-101 (240 Pin PQFP)
        NEC DX-102 (52 Pin PQFP x3)
 Sound: X1-010 (Mitsubishi M60016 Gate Array, 80 Pin PQFP)
EEPROM: 93LC46BX (1K Low-power 64 x 16-bit organization serial EEPROM)
   OSC: 50MHz & 28MHz
 Other: 8 Position Dipswitch x 2
        Lattice ispLSI2032 - stamped "KW001"
        Lattice isp1016E - stamped "GUN" (2 for PCB B0-010A, used for light gun input)
        BAT1 - CR2032 3Volt

PCB Number: B0-003A (or B0-003B)
+-----------------------------------------------------------+
|             VOL                          +------+         |
|                                          |X1-010|     M1  |
|   +---+ +---+                            |M60016|         |
|   |   | |   |  M    M                    |CALRUA|  +---+  |
+-+ | U | | U |  2    1                    +------+  |   |  |
  | | 0 | | 0 |                                      |   |  |
+-+ | 7 | | 6 |  M    M                              | U |  |
|   |   | |   |  2    1                              | 1 |  |
|   +---+ +---+                                      | 8 |  |
|                                          Lattice   |   |  |
|J  D +---+  C                            ispLSI2032 |   |  |
|A  S |DX |  N   BAT1           +-------+            +---+  |
|M  W |102|  5                  |Toshiba|  D                |
|M  1 +---+                     |  TMP  |  S EEPROM       C |
|A           C                  | 68301 |  W              N |
|            N  Lattice         +-------+  2              2 |
|C           6  isp1016E                                    |
|o                              +----------+    50MHz       |
|n    +---+                     |          |                |
|n    |DX |  SW1                |   NEC    |    M   M       |
|e    |102|                     |  DX-101  |    3   3       |
|c    +---+         M  M        |          |                |
|t                  1  1        |          |                |
|e                              +----------+                |
|r                                                          |
|                             +---+      +---++---++---+    |
|                  28MHz      |   |      |   ||   ||   |    |
|     +---+                   |   |      |   ||   ||   |    |
+-+   |DX |                   | U |      | U || U || U |    |
  |   |102|                   | 4 |      | 4 || 3 || 3 |    |
+-+   +---+                   | 0 |      | 1 || 8 || 9 |    |
|                             |   |      |   ||   ||   |    |
|                             |   |      |   ||   ||   |    |
|                             +---+      +---++---++---+    |
+-----------------------------------------------------------+

PCB Number: B0-010A - This PCB is slightly revised for 2 player play
+-----------------------------------------------------------+
|             VOL                          +------+         |
|                                          |X1-010|     M1  |
|   +---+ +---+                            |M60016|         |
|   |   | |   |  M    M                    |CALRUA|  +---+  |
+-+ | U | | U |  2    1                    +------+  |   |  |
  | | 0 | | 0 |                                      |   |  |
+-+ | 7 | | 6 |  M    M                              | U |  |
|   |   | |   |  2    1                              | 1 |  |
|   +---+ +---+                                      | 8 |  |
|                                          Lattice   |   |  |
|J  D +---+  C                            ispLSI2032 |   |  |
|A  S |DX |  N   BAT1           +-------+            +---+  |
|M  W |102|  5                  |Toshiba|  D                |
|M  1 +---+                     |  TMP  |  S EEPROM       C |
|A           C                  | 68301 |  W              N |
|            N  Lattice         +-------+  2              2 |
|C           6  isp1016E                                    |
|o                              +----------+    50MHz       |
|n    +---+                     |          |                |
|n    |DX |  SW1                |   NEC    |    M   M       |
|e    |102|                     |  DX-101  |    3   3       |
|c    +---+         M  M        |          |                |
|t                  1  1        |          |                |
|e                              +----------+                |
|r                                                          |
|                             +---+      +---++---++---+    |
|                  28MHz      |   |      |   ||   ||   |    |
|     +---+              C    |   |      |   ||   ||   |    |
+-+   |DX |              N    | U |      | U || U || U |    |
  |   |102|              7    | 4 |      | 4 || 3 || 3 |    |
+-+   +---+                   | 0 |      | 1 || 8 || 9 |    |
|             Lattice    C    |   |      |   ||   ||   |    |
|             isp1016E   N    |   |      |   ||   ||   |    |
|                        8    +---+      +---++---++---+    |
+-----------------------------------------------------------+

Ram M1 are Toshiba TC55257DFL-70L
Ram M2 are NEC D43001GU-70L
Ram M3 are ISSI IS62C1024L-70Q

U06 Program rom ST27C801 (even)
U07 Program rom ST27C801 (odd)

U18 Mask rom (Samples 23C32000 32Mbit (read as 27C322))

U38 - U40 Mask roms (Graphics 23c64020 64Mbit) - 23C64020 read as 27C322 with pin11 +5v & 27C322 with pin11 GND

--------------------------------------------------------------------------

From the WSC upgrade instruction sheet:

 Wing Shooting Championship
      Game Echancement
          1/23/02

New Program chip Ver. 2.00 For Wing Shooting Championship
We are announcing NEW GAME FEATURES to enhance game play. Please refer below.

NEW FEATURES
------------

 * Easier play for the first 3 hunting spots in every state with the addition of more birds.
 * The "BEGINNER" weapon has been changed to the 5-shot PUMP SHOTGUN plus the "hit area"
    for each shot has been increased. Same as the 3-shot SEMI-AUTO SHOTGUN.
 * Player can now advance through all result screens faster by pulling gun trigger.
 * The Auto Select bird is now GOOSE (easiest target) if player fails to choose bird at start of game.

***************************************************************************/

ROM_START( deerhunt ) /* Deer Hunting USA V4.3 (11/1/2000) - The "E05" breaks version label conventions but is correct & verified */
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "as0906e05.u06", 0x000000, 0x100000, CRC(20c81f17) SHA1(d41d93d6ee88738cec55f7bf3ce6be1dbec68e09) ) /* checksum 694E printed on label */
	ROM_LOAD16_BYTE( "as0907e05.u07", 0x000001, 0x100000, CRC(1731aa2a) SHA1(cffae7a99a7f960a62ef0c4454884df17a93c1a6) ) /* checksum 5D89 printed on label */

	ROM_REGION( 0x2000000, "sprites", 0 )   // Sprites
	ROM_LOAD( "as0901m01.u38", 0x0000000, 0x800000, CRC(1d6acf8f) SHA1(6f61fe21bebb7c87e8e6c3ef3ba73b8cf327dde9) )
	ROM_LOAD( "as0902m01.u39", 0x0800000, 0x800000, CRC(c7ca2128) SHA1(86be3a3ec2f86f61acfa3d4d261faea3c27dc378) )
	ROM_LOAD( "as0903m01.u40", 0x1000000, 0x800000, CRC(e8ef81b3) SHA1(97666942ca6cca5b8ea6451314a2aaabad9e06ba) )
	ROM_LOAD( "as0904m01.u41", 0x1800000, 0x800000, CRC(d0f97fdc) SHA1(776c9d42d03a9f61155521212305e1ed696eaf47) )

	ROM_REGION( 0x500000, "x1snd", 0 )  // Samples
	// Leave 1MB empty (addressable by the chip)
	ROM_LOAD( "as0905m01.u18", 0x100000, 0x400000, CRC(8d8165bb) SHA1(aca7051613d260734ee787b4c3db552c336bd600) )
ROM_END

ROM_START( deerhunta ) /* Deer Hunting USA V4.2 (xx/x/2000) */
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "as0906e04-v4_2.u06", 0x000000, 0x100000, CRC(bb3af36f) SHA1(f04071347e8ad361bf666fcb6c0136e522f19d47) ) /* checksum 6640 printed on label */
	ROM_LOAD16_BYTE( "as0907e04-v4_2.u07", 0x000001, 0x100000, CRC(83f02117) SHA1(70fc2291bc93af3902aae88688be6a8078f7a07e) ) /* checksum 595A printed on label */

	ROM_REGION( 0x2000000, "sprites", 0 )   // Sprites
	ROM_LOAD( "as0901m01.u38", 0x0000000, 0x800000, CRC(1d6acf8f) SHA1(6f61fe21bebb7c87e8e6c3ef3ba73b8cf327dde9) )
	ROM_LOAD( "as0902m01.u39", 0x0800000, 0x800000, CRC(c7ca2128) SHA1(86be3a3ec2f86f61acfa3d4d261faea3c27dc378) )
	ROM_LOAD( "as0903m01.u40", 0x1000000, 0x800000, CRC(e8ef81b3) SHA1(97666942ca6cca5b8ea6451314a2aaabad9e06ba) )
	ROM_LOAD( "as0904m01.u41", 0x1800000, 0x800000, CRC(d0f97fdc) SHA1(776c9d42d03a9f61155521212305e1ed696eaf47) )

	ROM_REGION( 0x500000, "x1snd", 0 )  // Samples
	// Leave 1MB empty (addressable by the chip)
	ROM_LOAD( "as0905m01.u18", 0x100000, 0x400000, CRC(8d8165bb) SHA1(aca7051613d260734ee787b4c3db552c336bd600) )
ROM_END

ROM_START( deerhuntb ) /* Deer Hunting USA V4.0 (6/15/2000) */
	ROM_REGION( 0x200000, "maincpu", 0 )        // TMP68301 Code
	ROM_LOAD16_BYTE( "as0906e04.u06", 0x000000, 0x100000, CRC(07d9b64a) SHA1(f9aac644aab920bbac84b14836ee589ccd51f6db) ) /* checksum 7BBB printed on label */
	ROM_LOAD16_BYTE( "as0907e04.u07", 0x000001, 0x100000, CRC(19973d08) SHA1(da1cc02ce480a62ccaf94d0af1246a340f054b43) ) /* checksum 4C78 printed on label */

	ROM_REGION( 0x2000000, "sprites", 0 )   // Sprites
	ROM_LOAD( "as0901m01.u38", 0x0000000, 0x800000, CRC(1d6acf8f) SHA1(6f61fe21bebb7c87e8e6c3ef3ba73b8cf327dde9) )
	ROM_LOAD( "as0902m01.u39", 0x0800000, 0x800000, CRC(c7ca2128) SHA1(86be3a3ec2f86f61acfa3d4d261faea3c27dc378) )
	ROM_LOAD( "as0903m01.u40", 0x1000000, 0x800000, CRC(e8ef81b3) SHA1(97666942ca6cca5b8ea6451314a2aaabad9e06ba) )
	ROM_LOAD( "as0904m01.u41", 0x1800000, 0x800000, CRC(d0f97fdc) SHA1(776c9d42d03a9f61155521212305e1ed696eaf47) )

	ROM_REGION( 0x500000, "x1snd", 0 )  // Samples
	// Leave 1MB empty (addressable by the chip)
	ROM_LOAD( "as0905m01.u18", 0x100000, 0x400000, CRC(8d8165bb) SHA1(aca7051613d260734ee787b4c3db552c336bd600) )
ROM_END

	/* Are there versions 3.x of Deer Hunting USA with labels "AS0906 E03 U06" & "AS0907 E03 U07" ?? */

ROM_START( deerhuntc ) /* These rom labels break label conventions but is correct & verified. Version in program code is listed as 0.00 */
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "as0937e01.u06", 0x000000, 0x100000, CRC(8d74088e) SHA1(cb11ffaf4c0267cc8cbe01accc3daeed910a3af3) ) /* SUM16 = C2CD */
	ROM_LOAD16_BYTE( "as0938e01.u07", 0x000001, 0x100000, CRC(c7657889) SHA1(4cc707c8abbc0862457375a9a910d3c338859193) ) /* SUM16 = 27D7 */

	ROM_REGION( 0x2000000, "sprites", 0 )   // Sprites
	ROM_LOAD( "as0901m01.u38", 0x0000000, 0x800000, CRC(1d6acf8f) SHA1(6f61fe21bebb7c87e8e6c3ef3ba73b8cf327dde9) )
	ROM_LOAD( "as0902m01.u39", 0x0800000, 0x800000, CRC(c7ca2128) SHA1(86be3a3ec2f86f61acfa3d4d261faea3c27dc378) )
	ROM_LOAD( "as0903m01.u40", 0x1000000, 0x800000, CRC(e8ef81b3) SHA1(97666942ca6cca5b8ea6451314a2aaabad9e06ba) )
	ROM_LOAD( "as0904m01.u41", 0x1800000, 0x800000, CRC(d0f97fdc) SHA1(776c9d42d03a9f61155521212305e1ed696eaf47) )

	ROM_REGION( 0x500000, "x1snd", 0 )  // Samples
	// Leave 1MB empty (addressable by the chip)
	ROM_LOAD( "as0905m01.u18", 0x100000, 0x400000, CRC(8d8165bb) SHA1(aca7051613d260734ee787b4c3db552c336bd600) )
ROM_END

ROM_START( deerhuntd ) /* Deer Hunting USA V2.x - No version number is printed to screen but "E02" in EPROM label signifies V2 */
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "as0906e02.u06", 0x000000, 0x100000, CRC(190cca42) SHA1(aef63f5e8c71ed0156b8b0104c5d23872c119167) ) /* Version in program code is listed as 0.00 */
	ROM_LOAD16_BYTE( "as0907e02.u07", 0x000001, 0x100000, CRC(9de2b901) SHA1(d271bc54c41e30c0d9962eedd22f3ef2b7b8c9e5) ) /* Verified with two different sets of chips */

	ROM_REGION( 0x2000000, "sprites", 0 )   // Sprites
	ROM_LOAD( "as0901m01.u38", 0x0000000, 0x800000, CRC(1d6acf8f) SHA1(6f61fe21bebb7c87e8e6c3ef3ba73b8cf327dde9) )
	ROM_LOAD( "as0902m01.u39", 0x0800000, 0x800000, CRC(c7ca2128) SHA1(86be3a3ec2f86f61acfa3d4d261faea3c27dc378) )
	ROM_LOAD( "as0903m01.u40", 0x1000000, 0x800000, CRC(e8ef81b3) SHA1(97666942ca6cca5b8ea6451314a2aaabad9e06ba) )
	ROM_LOAD( "as0904m01.u41", 0x1800000, 0x800000, CRC(d0f97fdc) SHA1(776c9d42d03a9f61155521212305e1ed696eaf47) )

	ROM_REGION( 0x500000, "x1snd", 0 )  // Samples
	// Leave 1MB empty (addressable by the chip)
	ROM_LOAD( "as0905m01.u18", 0x100000, 0x400000, CRC(8d8165bb) SHA1(aca7051613d260734ee787b4c3db552c336bd600) )
ROM_END

ROM_START( deerhunte ) /* Deer Hunting USA V1.x - No version number is printed to screen but "E01" in EPROM label signifies V1 */
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "as0906e01.u06", 0x000000, 0x100000, CRC(103e3ba3) SHA1(677d912ea9ed2ee1f26cdcac1687ce8ef416a96f) ) /* Version in program code is listed as 0.00 */
	ROM_LOAD16_BYTE( "as0907e01.u07", 0x000001, 0x100000, CRC(ddeb0f97) SHA1(a2578071f3506d69057d2256685b969adc50d275) ) /* Verified with two different sets of chips */

	ROM_REGION( 0x2000000, "sprites", 0 )   // Sprites
	ROM_LOAD( "as0901m01.u38", 0x0000000, 0x800000, CRC(1d6acf8f) SHA1(6f61fe21bebb7c87e8e6c3ef3ba73b8cf327dde9) )
	ROM_LOAD( "as0902m01.u39", 0x0800000, 0x800000, CRC(c7ca2128) SHA1(86be3a3ec2f86f61acfa3d4d261faea3c27dc378) )
	ROM_LOAD( "as0903m01.u40", 0x1000000, 0x800000, CRC(e8ef81b3) SHA1(97666942ca6cca5b8ea6451314a2aaabad9e06ba) )
	ROM_LOAD( "as0904m01.u41", 0x1800000, 0x800000, CRC(d0f97fdc) SHA1(776c9d42d03a9f61155521212305e1ed696eaf47) )

	ROM_REGION( 0x500000, "x1snd", 0 )  // Samples
	// Leave 1MB empty (addressable by the chip)
	ROM_LOAD( "as0905m01.u18", 0x100000, 0x400000, CRC(8d8165bb) SHA1(aca7051613d260734ee787b4c3db552c336bd600) )
ROM_END

ROM_START( turkhunt ) /* V1.0 is currently the only known version */
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "asx906e01.u06", 0x000000, 0x100000, CRC(c96266e1) SHA1(0ca462b3b0f27198e36384eee6ea5c5d4e7e1293) ) /* checksum E510 printed on label */
	ROM_LOAD16_BYTE( "asx907e01.u07", 0x000001, 0x100000, CRC(7c67b502) SHA1(6a0e8883a115dac4095d86897e7eca2a007a1c71) ) /* checksum AB40 printed on label */

	ROM_REGION( 0x2000000, "sprites", 0 )   // Sprites
	ROM_LOAD( "asx901m01.u38", 0x0000000, 0x800000, CRC(eabd3f44) SHA1(5a1ac986d11a8b019e18761cf4ea0a6f49fbdbfc) )
	ROM_LOAD( "asx902m01.u39", 0x0800000, 0x800000, CRC(c32130c8) SHA1(70d56ebed1f51657aaee02f95ac51589733e6eb7) )
	ROM_LOAD( "asx903m01.u40", 0x1000000, 0x800000, CRC(5f86c322) SHA1(5a72adb99eea176199f172384cb051e2b045ab94) )
	ROM_LOAD( "asx904m01.u41", 0x1800000, 0x800000, CRC(c77e0b66) SHA1(0eba30e62e4bd38c198fa6cb69fb94d002ded77a) )

	ROM_REGION( 0x500000, "x1snd", 0 )  // Samples
	// Leave 1MB empty (addressable by the chip)
	ROM_LOAD( "asx905m01.u18", 0x100000, 0x400000, CRC(8d9dd9a9) SHA1(1fc2f3688d2c24c720dca7357bca6bf5f4016c53) )
ROM_END

ROM_START( wschamp ) /* Wing Shooting Championship V2.00 (01/23/2002) */
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "as1006e03.u06", 0x000000, 0x100000, CRC(0ad01677) SHA1(63e09b9f7cc8b781af1756f86caa0cc0962ae584) ) /* checksum 421E printed on label */
	ROM_LOAD16_BYTE( "as1007e03.u07", 0x000001, 0x100000, CRC(572624f0) SHA1(0c2f67daa22f4edd66a2be990dc6cd999faff0fa) ) /* checksum A48F printed on label */

	ROM_REGION( 0x2000000, "sprites", 0 )   // Sprites
	ROM_LOAD( "as1001m01.u38", 0x0000000, 0x800000, CRC(92595579) SHA1(75a7131aedb18b7103677340c3cca7c91aaca2bf) )
	ROM_LOAD( "as1002m01.u39", 0x0800000, 0x800000, CRC(16c2bb08) SHA1(63926464c8bd8db7d05905a953765e645942beb4) )
	ROM_LOAD( "as1003m01.u40", 0x1000000, 0x800000, CRC(89618858) SHA1(a8bd07f233482e8f5a256af7ff9577648eb58ef4) )
	ROM_LOAD( "as1004m01.u41", 0x1800000, 0x800000, CRC(500c0909) SHA1(73ff27d46b9285f34a50a81c21c54437f21e1939) )

	ROM_REGION( 0x500000, "x1snd", 0 )  // Samples
	// Leave 1MB empty (addressable by the chip)
	ROM_LOAD( "as1005m01.u18", 0x100000, 0x400000, CRC(e4b137b8) SHA1(4d8d15073c51f7d383282cc5755ae5b2eab6226c) )
ROM_END

ROM_START( wschampa ) /* Wing Shooting Championship V1.01 */
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "as1006e02.u06", 0x000000, 0x100000, CRC(d3d3b2b5) SHA1(2d036d795b40a4ed78bb9f7751f875cfc76276a9) ) /* checksum 31EF printed on label */
	ROM_LOAD16_BYTE( "as1007e02.u07", 0x000001, 0x100000, CRC(78ede6d9) SHA1(e6d10f52cd4c6bf97288df44911f23bb64fc012c) ) /* checksum 615E printed on label */

	ROM_REGION( 0x2000000, "sprites", 0 )   // Sprites
	ROM_LOAD( "as1001m01.u38", 0x0000000, 0x800000, CRC(92595579) SHA1(75a7131aedb18b7103677340c3cca7c91aaca2bf) )
	ROM_LOAD( "as1002m01.u39", 0x0800000, 0x800000, CRC(16c2bb08) SHA1(63926464c8bd8db7d05905a953765e645942beb4) )
	ROM_LOAD( "as1003m01.u40", 0x1000000, 0x800000, CRC(89618858) SHA1(a8bd07f233482e8f5a256af7ff9577648eb58ef4) )
	ROM_LOAD( "as1004m01.u41", 0x1800000, 0x800000, CRC(500c0909) SHA1(73ff27d46b9285f34a50a81c21c54437f21e1939) )

	ROM_REGION( 0x500000, "x1snd", 0 )  // Samples
	// Leave 1MB empty (addressable by the chip)
	ROM_LOAD( "as1005m01.u18", 0x100000, 0x400000, CRC(e4b137b8) SHA1(4d8d15073c51f7d383282cc5755ae5b2eab6226c) )
ROM_END

ROM_START( wschampb ) /* Wing Shooting Championship V1.00, dumps match listed checksum but shows as "NG" on boot screen - need to verify correct at some point if possible */
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "as10u6.u06", 0x000000, 0x100000, CRC(70a18bef) SHA1(3fb2e8a4db790dd732115d7d3d991b2d6c54feb9) ) /* checksum 3F38 & 10/26 16:00 hand written on label */
	ROM_LOAD16_BYTE( "as10u7.u07", 0x000001, 0x100000, CRC(cf23be7d) SHA1(b9130757466ff0d41d261b1c2435d36d2452df54) ) /* checksum 1537 & 10/26 16:00 hand written on label */

	ROM_REGION( 0x2000000, "sprites", 0 )   // Sprites
	ROM_LOAD( "as1001m01.u38", 0x0000000, 0x800000, CRC(92595579) SHA1(75a7131aedb18b7103677340c3cca7c91aaca2bf) )
	ROM_LOAD( "as1002m01.u39", 0x0800000, 0x800000, CRC(16c2bb08) SHA1(63926464c8bd8db7d05905a953765e645942beb4) )
	ROM_LOAD( "as1003m01.u40", 0x1000000, 0x800000, CRC(89618858) SHA1(a8bd07f233482e8f5a256af7ff9577648eb58ef4) )
	ROM_LOAD( "as1004m01.u41", 0x1800000, 0x800000, CRC(500c0909) SHA1(73ff27d46b9285f34a50a81c21c54437f21e1939) )

	ROM_REGION( 0x500000, "x1snd", 0 )  // Samples
	// Leave 1MB empty (addressable by the chip)
	ROM_LOAD( "as1005m01.u18", 0x100000, 0x400000, CRC(e4b137b8) SHA1(4d8d15073c51f7d383282cc5755ae5b2eab6226c) )
ROM_END

ROM_START( trophyh ) /* V1.0 is currently the only known version */
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "as1106e01.u06", 0x000000, 0x100000, CRC(b4950882) SHA1(2749f7ffc5b543c9f39815f0913a1d1e385b63f4) ) /* checksum D8DA printed on label */
	ROM_LOAD16_BYTE( "as1107e01.u07", 0x000001, 0x100000, CRC(19ee67cb) SHA1(e75ce66d3ff5aad46ba997c09d6514260e617f55) ) /* checksum CEEF printed on label */

	ROM_REGION( 0x2000000, "sprites", 0 )   // Sprites
	ROM_LOAD( "as1101m01.u38", 0x0000000, 0x800000, CRC(855ed675) SHA1(84ce229a9feb6331413253a5aed10b362e8102e5) )
	ROM_LOAD( "as1102m01.u39", 0x0800000, 0x800000, CRC(d186d271) SHA1(3c54438b35adfab8be91df0a633270d6db49beef) )
	ROM_LOAD( "as1103m01.u40", 0x1000000, 0x800000, CRC(adf8a54e) SHA1(bb28bf219d18082246f7964851a5c49b9c0ba7f5) )
	ROM_LOAD( "as1104m01.u41", 0x1800000, 0x800000, CRC(387882e9) SHA1(0fdd0c77dabd1066c6f3bd64e357236a76f524ab) )

	ROM_REGION( 0x500000, "x1snd", 0 )  // Samples
	// Leave 1MB empty (addressable by the chip)
	ROM_LOAD( "as1105m01.u18", 0x100000, 0x400000, CRC(633d0df8) SHA1(3401c424f5c207ef438a9269e0c0e7d482771fed) )
ROM_END


GAME( 1994, gundamex, 0,        gundamex, gundamex, driver_device, 0,        ROT0, "Banpresto",             "Mobile Suit Gundam EX Revue",                  0 )
GAME( 1995, grdians,  0,        grdians,  grdians,  driver_device, 0,        ROT0, "Winkysoft (Banpresto license)", "Guardians / Denjin Makai II",          MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, mj4simai, 0,        mj4simai, mj4simai, driver_device, 0,        ROT0, "Maboroshi Ware",        "Wakakusamonogatari Mahjong Yonshimai (Japan)", MACHINE_NO_COCKTAIL )
GAME( 1996, myangel,  0,        myangel,  myangel,  driver_device, 0,        ROT0, "MOSS / Namco",          "Kosodate Quiz My Angel (Japan)",               MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, myangel2, 0,        myangel2, myangel2, driver_device, 0,        ROT0, "MOSS / Namco",          "Kosodate Quiz My Angel 2 (Japan)",             MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, reelquak, 0,        reelquak, reelquak, driver_device, 0,        ROT0, "<unknown>",             "Reel'N Quake! (Version 1.05)",                 MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 199?, endrichs, 0,        reelquak, endrichs, driver_device, 0,        ROT0, "E.N.Tiger",             "Endless Riches (Ver 1.20)",                    MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, pzlbowl,  0,        pzlbowl,  pzlbowl,  driver_device, 0,        ROT0, "MOSS / Nihon System",   "Puzzle De Bowling (Japan)",                    MACHINE_NO_COCKTAIL )
GAME( 2000, penbros,  0,        penbros,  penbros,  driver_device, 0,        ROT0, "Subsino",               "Penguin Brothers (Japan)",                     MACHINE_NO_COCKTAIL )
GAME( 2000, namcostr, 0,        namcostr, funcube,  driver_device, 0,        ROT0, "Namco",                 "Namco Stars",                                  MACHINE_NO_COCKTAIL | MACHINE_NOT_WORKING )
GAME( 2000, deerhunt, 0,        samshoot, deerhunt, driver_device, 0,        ROT0, "Sammy USA Corporation", "Deer Hunting USA V4.3",                        MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2000, deerhunta,deerhunt, samshoot, deerhunt, driver_device, 0,        ROT0, "Sammy USA Corporation", "Deer Hunting USA V4.2",                        MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2000, deerhuntb,deerhunt, samshoot, deerhunt, driver_device, 0,        ROT0, "Sammy USA Corporation", "Deer Hunting USA V4.0",                        MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2000, deerhuntc,deerhunt, samshoot, deerhunt, driver_device, 0,        ROT0, "Sammy USA Corporation", "Deer Hunting USA V3",                          MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2000, deerhuntd,deerhunt, samshoot, deerhunt, driver_device, 0,        ROT0, "Sammy USA Corporation", "Deer Hunting USA V2",                          MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2000, deerhunte,deerhunt, samshoot, deerhunt, driver_device, 0,        ROT0, "Sammy USA Corporation", "Deer Hunting USA V1",                          MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2001, turkhunt, 0,        samshoot, turkhunt, driver_device, 0,        ROT0, "Sammy USA Corporation", "Turkey Hunting USA V1.0",                      MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2001, wschamp,  0,        samshoot, wschamp,  driver_device, 0,        ROT0, "Sammy USA Corporation", "Wing Shooting Championship V2.00",             MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2001, wschampa, wschamp,  samshoot, wschamp,  driver_device, 0,        ROT0, "Sammy USA Corporation", "Wing Shooting Championship V1.01",             MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2001, wschampb, wschamp,  samshoot, wschamp,  driver_device, 0,        ROT0, "Sammy USA Corporation", "Wing Shooting Championship V1.00",             MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2002, trophyh,  0,        samshoot, trophyh,  driver_device, 0,        ROT0, "Sammy USA Corporation", "Trophy Hunting - Bear & Moose V1.0",           MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2000, funcube,  0,        funcube,  funcube,  seta2_state,   funcube,  ROT0, "Namco",                 "Funcube (v1.5)",                               MACHINE_NO_COCKTAIL )
GAME( 2001, funcube2, 0,        funcube2, funcube,  seta2_state,   funcube2, ROT0, "Namco",                 "Funcube 2 (v1.1)",                             MACHINE_NO_COCKTAIL )
GAME( 2001, funcube3, 0,        funcube3, funcube,  seta2_state,   funcube3, ROT0, "Namco",                 "Funcube 3 (v1.1)",                             MACHINE_NO_COCKTAIL )
GAME( 2001, funcube4, 0,        funcube2, funcube,  seta2_state,   funcube2, ROT0, "Namco",                 "Funcube 4 (v1.0)",                             MACHINE_NO_COCKTAIL )
GAME( 2002, funcube5, 0,        funcube2, funcube,  seta2_state,   funcube2, ROT0, "Namco",                 "Funcube 5 (v1.0)",                             MACHINE_NO_COCKTAIL )
